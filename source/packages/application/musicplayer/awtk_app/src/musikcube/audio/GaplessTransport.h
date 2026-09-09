/* GaplessTransport.h — from musikcube (BSD-3), adapted for embedded.
 * Removed: sigslot, ITransport interface, Gain/ReplayGain, MasterTransport.
 * Kept: dual Player (active + next), gapless transition, volume, pause/resume.
 *
 * This is the "transport layer" — it owns the Player instances and the
 * shared ALSA output. music_player.cpp talks to this instead of creating
 * Player objects directly. */
#pragma once

#include "../config.h"
#include "Player.h"
#include "Outputs.h"
#include "../sdk/IOutput.h"
#include "../sdk/constants.h"

#include <string>
#include <mutex>
#include <memory>
#include <functional>

namespace musik { namespace core { namespace audio {

    class GaplessTransport : private Player::EventListener {
    public:
        /* Callback types — replaces sigslot signals */
        using StreamStateCb   = std::function<void(musik::core::sdk::StreamState, const std::string&)>;
        using PlaybackStateCb = std::function<void(musik::core::sdk::PlaybackState)>;

        GaplessTransport();
        ~GaplessTransport();

        /* Playback control */
        void Start(const std::string& uri);
        void PrepareNextTrack(const std::string& uri);
        void Stop();
        bool Pause();
        bool Resume();

        /* Position / duration */
        double Position();
        void SetPosition(double seconds);
        double GetDuration();

        /* Volume */
        double Volume() noexcept;
        void SetVolume(double volume);
        bool IsMuted() noexcept;
        void SetMuted(bool muted);

        /* State */
        musik::core::sdk::PlaybackState GetPlaybackState();
        std::string Uri();

        /* Callback setters */
        void SetStreamStateCb(StreamStateCb cb)   { streamStateCb = cb; }
        void SetPlaybackStateCb(PlaybackStateCb cb) { playbackStateCb = cb; }

    private:
        using LockT = std::unique_lock<std::recursive_mutex>;

        void StartWithPlayer(Player* player);
        void StopInternal(bool suppressStopEvent, bool stopOutput, Player const* exclude = nullptr);
        void SetNextCanStart(bool nextCanStart);
        void RaiseStreamEvent(musik::core::sdk::StreamState type, Player const* player);
        void SetPlaybackState(musik::core::sdk::PlaybackState state);

        /* Player::EventListener overrides */
        void OnPlayerBuffered(Player* player) override;
        void OnPlayerStarted(Player* player) override;
        void OnPlayerStreamEof(Player* player) override;
        void OnPlayerFinished(Player* player) override;
        void OnPlayerOpenFailed(Player* player) override;
        void OnPlayerDestroying(Player* player) override;

        void ResetActivePlayer();
        void ResetNextPlayer();

        musik::core::sdk::PlaybackState playbackState;
        std::recursive_mutex stateMutex;
        std::shared_ptr<musik::core::sdk::IOutput> output;
        Player* activePlayer;
        Player* nextPlayer;
        double volume;
        bool nextCanStart;
        bool muted;

        StreamStateCb   streamStateCb;
        PlaybackStateCb playbackStateCb;
    };

} } }