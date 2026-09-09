/* GaplessTransport.cpp — from musikcube (BSD-3), adapted for embedded.
 * Changes: replaced sigslot with function callbacks, removed Gain/ReplayGain,
 * removed PluginFactory, simplified Player::Create call. */

#include "../pch.hpp"
#include "GaplessTransport.h"
#include <algorithm>

using namespace musik::core::audio;
using namespace musik::core::sdk;

static std::string TAG = "GaplessTransport";

GaplessTransport::GaplessTransport()
: volume(1.0)
, playbackState(PlaybackState::Stopped)
, activePlayer(nullptr)
, nextPlayer(nullptr)
, nextCanStart(false)
, muted(false) {
    this->output = outputs::SelectedOutput();
}

GaplessTransport::~GaplessTransport() {
    LockT lock(this->stateMutex);
    this->ResetNextPlayer();
    this->ResetActivePlayer();
}

PlaybackState GaplessTransport::GetPlaybackState() {
    LockT lock(this->stateMutex);
    return this->playbackState;
}

void GaplessTransport::PrepareNextTrack(const std::string& uri) {
    bool startNext = false;
    {
        LockT lock(this->stateMutex);

        this->ResetNextPlayer();

        if (uri.size()) {
            this->nextPlayer = Player::Create(uri, this->output, this);
            startNext = this->nextCanStart;
        }
    }

    if (startNext) {
        this->StartWithPlayer(this->nextPlayer);
    }
}

void GaplessTransport::Start(const std::string& uri) {
    musik::debug::info(TAG, "starting track at " + uri);
    Player* newPlayer = Player::Create(uri, this->output, this);
    this->StartWithPlayer(newPlayer);
}

void GaplessTransport::StartWithPlayer(Player* newPlayer) {
    if (newPlayer) {
        bool playingNext = false;

        {
            LockT lock(this->stateMutex);

            playingNext = (newPlayer == nextPlayer);
            if (newPlayer != nextPlayer) {
                this->ResetNextPlayer();
            }

            this->ResetActivePlayer();

            this->nextPlayer = nullptr;
            this->activePlayer = newPlayer;

            /* Never call output->Stop() here — the output is shared (singleton
             * AlsaOut) and the old Player's decode thread may still be inside
             * output->Play().  Stopping the output would close the PCM handle
             * underneath that thread → deadlock.
             *
             * Instead, just let the old Player's thread detect Quit and exit
             * naturally.  The new Player will reuse the same AlsaOut. */
            this->SetNextCanStart(false);
            this->output->Resume();

            newPlayer->Play();
        }
    }
}

void GaplessTransport::Stop() {
    {
        LockT lock(this->stateMutex);
        this->ResetNextPlayer();
        this->ResetActivePlayer();
    }

    /* Give the old player threads a moment to detect Quit and exit,
     * then stop the output.  The threads are detached so we can't join. */
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    this->output->Stop();
    this->SetPlaybackState(PlaybackState::Stopped);
}

std::string GaplessTransport::Uri() {
    const auto player = this->activePlayer;
    return player ? player->GetUrl() : "";
}

void GaplessTransport::StopInternal(
    bool suppressStopEvent,
    bool stopOutput,
    Player const* exclude)
{
    if (stopOutput) {
        {
            LockT lock(this->stateMutex);

            this->ResetNextPlayer();
            if (this->activePlayer != exclude) {
                this->ResetActivePlayer();
            }
        }

        this->output->Stop();
    }

    if (!suppressStopEvent) {
        this->SetPlaybackState(PlaybackState::Stopped);
    }
}

bool GaplessTransport::Pause() {
    this->output->Pause();

    if (this->activePlayer) {
        this->SetPlaybackState(PlaybackState::Paused);
        return true;
    }

    return false;
}

bool GaplessTransport::Resume() {
    this->output->Resume();

    {
        LockT lock(this->stateMutex);
        if (this->activePlayer) {
            this->activePlayer->Play();
        }
    }

    if (this->activePlayer) {
        this->SetPlaybackState(PlaybackState::Playing);
        return true;
    }

    return false;
}

double GaplessTransport::Position() {
    LockT lock(this->stateMutex);
    return this->activePlayer ? this->activePlayer->GetPosition() : 0;
}

void GaplessTransport::SetPosition(double seconds) {
    {
        LockT lock(this->stateMutex);
        if (this->activePlayer) {
            if (this->playbackState != PlaybackState::Playing) {
                this->SetPlaybackState(PlaybackState::Playing);
            }
            this->activePlayer->SetPosition(seconds);
        }
    }
}

double GaplessTransport::GetDuration() {
    LockT lock(this->stateMutex);
    return this->activePlayer ? this->activePlayer->GetDuration() : -1.0f;
}

bool GaplessTransport::IsMuted() noexcept {
    return this->muted;
}

void GaplessTransport::SetMuted(bool muted) {
    if (this->muted != muted) {
        this->muted = muted;
        this->output->SetVolume(muted ? 0.0f : this->volume);
    }
}

double GaplessTransport::Volume() noexcept {
    return this->volume;
}

void GaplessTransport::SetVolume(double volume) {
    double oldVolume = this->volume;
    volume = std::max(0.0, std::min(1.0, volume));
    this->volume = volume;
    this->output->SetVolume(this->volume);
    if (oldVolume != this->volume) {
        this->SetMuted(false);
    }
}

void GaplessTransport::SetNextCanStart(bool nextCanStart) {
    LockT lock(this->stateMutex);
    this->nextCanStart = nextCanStart;
}

/* ================================================================
 * Player::EventListener callbacks — called from the decode thread.
 * These manage the active/next Player lifecycle and fire state
 * events back up to music_player.cpp.
 * ================================================================ */

void GaplessTransport::OnPlayerBuffered(Player* player) {
    if (player == this->activePlayer) {
        this->RaiseStreamEvent(StreamState::Buffered, player);
    }
}

void GaplessTransport::OnPlayerStarted(Player* player) {
    this->RaiseStreamEvent(StreamState::Playing, player);
    this->SetPlaybackState(PlaybackState::Playing);
}

void GaplessTransport::OnPlayerStreamEof(Player* player) {
    /* The current track's decoder has finished. If a next track
     * has been prepared, start it now for gapless playback. */
    this->SetNextCanStart(true);

    {
        LockT lock(this->stateMutex);
        if (this->nextPlayer) {
            this->StartWithPlayer(this->nextPlayer);
        }
    }

    this->RaiseStreamEvent(StreamState::AlmostDone, player);
}

void GaplessTransport::OnPlayerFinished(Player* player) {
    /* All buffers have been drained by ALSA. */
    this->RaiseStreamEvent(StreamState::Finished, player);

    bool stopped = false;

    {
        LockT lock(this->stateMutex);

        bool startedNext = false;
        const bool playerIsActive = (player == this->activePlayer);

        if (playerIsActive && this->nextPlayer) {
            this->StartWithPlayer(this->nextPlayer);
            startedNext = true;
        }

        if (!startedNext) {
            stopped = playerIsActive;
        }
    }

    if (stopped) {
        /* don't stop output immediately — let trailing samples finish */
        this->StopInternal(false, false);
    }
}

void GaplessTransport::OnPlayerOpenFailed(Player* player) {
    bool raiseEvents = false;
    {
        LockT lock(this->stateMutex);
        if (player == this->activePlayer) {
            this->ResetActivePlayer();
            this->ResetNextPlayer();
            raiseEvents = true;
        }
        else if (player == this->nextPlayer) {
            this->ResetNextPlayer();
        }
    }
    if (raiseEvents) {
        this->RaiseStreamEvent(StreamState::OpenFailed, player);
        this->SetPlaybackState(PlaybackState::Stopped);
    }
}

void GaplessTransport::OnPlayerDestroying(Player* player) {
    LockT lock(this->stateMutex);
    if (player == this->activePlayer) {
        this->activePlayer = nullptr;
    }
}

void GaplessTransport::SetPlaybackState(PlaybackState state) {
    bool changed = false;

    {
        LockT lock(this->stateMutex);
        changed = (this->playbackState != state);
        this->playbackState = state;
    }

    if (changed && this->playbackStateCb) {
        this->playbackStateCb(state);
    }
}

void GaplessTransport::RaiseStreamEvent(StreamState type, Player const* player) {
    bool isActive = false;
    {
        LockT lock(this->stateMutex);
        isActive = (player == activePlayer);
    }

    if (isActive && this->streamStateCb) {
        this->streamStateCb(type, player->GetUrl());
    }
}

void GaplessTransport::ResetNextPlayer() {
    if (this->nextPlayer) {
        this->nextPlayer->Detach(this);
        this->nextPlayer->Destroy();
        this->nextPlayer = nullptr;
    }
}

void GaplessTransport::ResetActivePlayer() {
    if (this->activePlayer) {
        this->activePlayer->Detach(this);
        this->activePlayer->Destroy();
        this->activePlayer = nullptr;
    }
}