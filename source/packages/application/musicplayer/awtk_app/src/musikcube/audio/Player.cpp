/* Player.cpp — from musikcube (BSD-3), adapted for embedded build.
 * Removed: FFT/Visualizer, MixPoints, ReplayGain, kiss_fft.
 * Kept: decode thread loop, backpressure (pendingBufferCount),
 *       seek with flush, state machine, drain on finish. */

#include "../pch.hpp"
#include "Stream.h"
#include "Player.h"

#include <algorithm>
#include <cmath>

#define MAX_PREBUFFER_QUEUE_COUNT 8

using namespace musik::core::audio;
using namespace musik::core::sdk;

using std::min;
using std::max;

static std::string TAG = "Player";
using Listener = Player::EventListener;
using ListenerList = std::list<Listener*>;

namespace musik { namespace core { namespace audio {
    void playerThreadLoop(Player* player);
} } }

Player* Player::Create(
    const std::string& url,
    std::shared_ptr<IOutput> output,
    EventListener* listener)
{
    return new Player(url, output, listener);
}

Player::Player(
    const std::string& url,
    std::shared_ptr<IOutput> output,
    EventListener* listener)
: internalState(Player::Idle)
, streamState(StreamState::Buffering)
, stream(Stream::Create())
, url(url)
, currentPosition(0)
, output(output)
, notifiedStarted(false)
, seekToPosition(-1)
, pendingBufferCount(0) {
    musik::debug::info(TAG, "new instance created");

    if (!this->output) {
        throw std::runtime_error("output cannot be null!");
    }

    if (listener) {
        listeners.push_back(listener);
    }

    this->thread = new std::thread(
        std::bind(&musik::core::audio::playerThreadLoop, this));
}

Player::~Player() {
}

void Player::Play() {
    std::unique_lock<std::mutex> lock(this->queueMutex);

    if (this->internalState != Player::Quit) {
        this->internalState = Player::Playing;
        this->writeToOutputCondition.notify_all();
    }
}

void Player::Destroy() {
    {
        std::unique_lock<std::mutex> lock(this->queueMutex);

        if (this->internalState == Player::Quit && !this->thread) {
            return;
        }

        /* Set Quit BEFORE Interrupt. Otherwise there's a race:
         * Interrupt causes stream EOF → thread exits loop with finished=true
         * → checks Exited() → Quit not set yet → Drain() → closes shared PCM
         * → new Player is dead. */
        this->internalState = Player::Quit;
        this->writeToOutputCondition.notify_all();
        this->thread->detach();
        delete this->thread;
        this->thread = nullptr;
    }

    if (this->stream) {
        this->stream->Interrupt();
    }
}

void Player::Detach(EventListener* listener) {
    if (listener) {
        std::unique_lock<std::mutex> lock(this->listenerMutex);
        this->listeners.remove_if([listener](EventListener* compare) {
            return (listener == compare);
        });
    }
}

void Player::Attach(EventListener* listener) {
    this->Detach(listener);

    if (listener) {
        std::unique_lock<std::mutex> lock(this->listenerMutex);
        this->listeners.push_back(listener);
    }
}

ListenerList Player::Listeners() {
    std::unique_lock<std::mutex> lock(this->listenerMutex);
    return ListenerList(this->listeners);
}

double Player::GetPosition() {
    double seek = this->seekToPosition.load();
    double current = this->currentPosition.load();
    const double latency = this->output ? this->output->Latency() : 0.0;
    return std::max(0.0, round((seek >= 0 ? seek : current) - latency));
}

double Player::GetDuration() {
    return this->stream ? this->stream->GetDuration() : -1.0f;
}

void Player::SetPosition(double seconds) {
    std::unique_lock<std::mutex> queueLock(this->queueMutex);

    if (this->stream) {
        auto duration = this->stream->GetDuration();
        if (duration > 0.0f) {
            seconds = std::min(duration, seconds);
        }
    }

    this->seekToPosition.store(std::max(0.0, seconds));
}

int Player::State() {
    std::unique_lock<std::mutex> lock(this->queueMutex);
    return this->internalState;
}

/* ================================================================
 * THE DECODE THREAD — this is the core playback loop.
 *
 * Key design: backpressure via pendingBufferCount.
 *   - Each buffer sent to output increments pendingBufferCount.
 *   - OnBufferProcessed (called by AlsaOut after writei) decrements it.
 *   - When output's internal queue is full, Play() returns a wait time,
 *     and we sleep via condition_variable wait_for.
 *   - This naturally throttles decode speed to match playback speed.
 * ================================================================ */
void musik::core::audio::playerThreadLoop(Player* player) {
    IBuffer* buffer = nullptr;

    if (player->stream->OpenStream(player->url, player->output.get())) {
        for (Listener* l : player->Listeners()) {
            player->streamState = StreamState::Buffered;
            l->OnPlayerBuffered(player);
        }

        /* wait until Play() is called (transitions from Idle to Playing) */
        {
            std::unique_lock<std::mutex> lock(player->queueMutex);
            while (player->internalState == Player::Idle) {
                player->writeToOutputCondition.wait(lock);
            }
        }

        bool finished = false;

        while (!finished && !player->Exited()) {
            /* === SEEK === */
            double seek = player->seekToPosition.load();

            if (seek != -1.0) {
                player->output->Stop();
                player->output->Resume();

                if (buffer) {
                    player->OnBufferProcessed(buffer);
                    buffer = nullptr;
                }

                player->currentPosition.store(seek);

                {
                    std::unique_lock<std::mutex> lock(player->queueMutex);
                    while (player->pendingBufferCount > 0) {
                        player->writeToOutputCondition.wait(lock);
                    }
                }

                player->stream->SetPosition(seek);
                player->seekToPosition.exchange(-1.0);
            }

            /* === DECODE === */
            if (!buffer) {
                std::unique_lock<std::mutex> lock(player->queueMutex);
                buffer = player->stream->GetNextProcessedOutputBuffer();
                if (buffer) {
                    ++player->pendingBufferCount;
                }
            }

            /* === OUTPUT === */
            if (buffer) {
                OutputState playResult = player->output->Play(buffer, player);

                if (playResult == OutputState::BufferWritten) {
                    buffer = nullptr;
                }
                else {
                    int sleepMs = 1000;

                    if ((int) playResult >= 0) {
                        sleepMs = std::max(
                            (int)(player->output->Latency() * 250.0),
                            (int) playResult);
                    }

                    std::unique_lock<std::mutex> lock(player->queueMutex);
                    player->writeToOutputCondition.wait_for(
                        lock, std::chrono::milliseconds(sleepMs));
                }
            }
            else {
                if (player->stream->Eof()) {
                    finished = true;
                }
                else {
                    std::unique_lock<std::mutex> lock(player->queueMutex);
                    player->writeToOutputCondition.wait_for(
                        lock, std::chrono::milliseconds(10));
                }
            }
        }

        /* stream ended naturally (not stopped by user) */
        if (!player->Exited()) {
            for (Listener* l : player->Listeners()) {
                player->streamState = StreamState::AlmostDone;
                l->OnPlayerStreamEof(player);
            }
        }
    }
    else {
        /* stream failed to open */
        if (!player->Exited()) {
            for (Listener* l : player->Listeners()) {
                player->streamState = StreamState::OpenFailed;
                l->OnPlayerOpenFailed(player);
            }
        }
    }

    /* release any unreturned buffer */
    if (buffer) {
        player->OnBufferProcessed(buffer);
        buffer = nullptr;
    }

    /* wait for all pending buffers to drain */
    {
        std::unique_lock<std::mutex> lock(player->queueMutex);
        while (player->pendingBufferCount > 0) {
            player->writeToOutputCondition.wait(lock);
        }
    }

    /* Only drain ALSA if this track ended naturally (EOF).
     * If we were Destroy()'d (user skipped track), do NOT drain —
     * the output is shared (singleton AlsaOut) and the new Player
     * is already using it.  Draining here would close the PCM handle
     * underneath the new Player's decode thread. */
    if (!player->Exited()) {
        player->output->Drain();
    }

    if (!player->Exited()) {
        for (Listener* l : player->Listeners()) {
            player->streamState = StreamState::Finished;
            l->OnPlayerFinished(player);
        }
    }

    player->internalState = Player::Quit;

    for (Listener* l : player->Listeners()) {
        player->streamState = StreamState::Stopped;
        l->OnPlayerDestroying(player);
    }

    player->Destroy();

    delete player;
}

bool Player::Exited() {
    std::unique_lock<std::mutex> lock(this->queueMutex);
    return (this->internalState == Player::Quit);
}

void Player::OnBufferProcessed(IBuffer* buffer) {
    bool started = false;

    {
        std::unique_lock<std::mutex> lock(this->queueMutex);

        --pendingBufferCount;
        this->stream->OnBufferProcessedByPlayer((Buffer*)buffer);

        if (this->seekToPosition.load() == -1) {
            this->currentPosition.store(((Buffer*)buffer)->Position());
        }

        if (!this->notifiedStarted) {
            this->streamState = StreamState::Playing;
            this->notifiedStarted = true;
            started = true;
        }
    }

    if (started) {
        for (Listener* l : this->Listeners()) {
            if (!this->Exited()) {
                l->OnPlayerStarted(this);
            }
        }
    }

    /* wake the decode thread — a buffer slot is now free */
    this->writeToOutputCondition.notify_all();
}