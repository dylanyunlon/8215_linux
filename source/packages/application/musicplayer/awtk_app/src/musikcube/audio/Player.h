/* Player.h — from musikcube (BSD-3), adapted for embedded build.
 * Removed: FFT/Visualizer, sigslot, MixPoints, ReplayGain.
 * Kept: decode thread, backpressure, seek, state machine. */
#pragma once

#include "../config.h"
#include "IStream.h"
#include "../sdk/constants.h"
#include "../sdk/IOutput.h"
#include "../sdk/IBufferProvider.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <list>
#include <string>

namespace musik { namespace core { namespace audio {

    class Player : public musik::core::sdk::IBufferProvider {
        public:
            struct EventListener {
                virtual ~EventListener() { }
                virtual void OnPlayerBuffered(Player* player) { }
                virtual void OnPlayerStarted(Player* player) { }
                virtual void OnPlayerStreamEof(Player* player) { }
                virtual void OnPlayerFinished(Player* player) { }
                virtual void OnPlayerOpenFailed(Player* player) { }
                virtual void OnPlayerDestroying(Player* player) { }
            };

            static Player* Create(
                const std::string& url,
                std::shared_ptr<musik::core::sdk::IOutput> output,
                EventListener* listener);

            virtual void OnBufferProcessed(musik::core::sdk::IBuffer* buffer);

            void Detach(EventListener* listener);
            void Attach(EventListener* listener);

            void Play();
            void Destroy();

            double GetPosition();
            void SetPosition(double seconds);
            double GetDuration();

            std::string GetUrl() const { return this->url; }

            musik::core::sdk::StreamState GetStreamState() noexcept {
                return this->streamState;
            }

        private:
            friend void playerThreadLoop(Player* player);

            Player(
                const std::string& url,
                std::shared_ptr<musik::core::sdk::IOutput> output,
                EventListener* listener);

            virtual ~Player();

            using ListenerList = std::list<EventListener*>;
            using OutputPtr = std::shared_ptr<musik::core::sdk::IOutput>;

            typedef enum {
                Idle = 0,
                Playing = 1,
                Quit = 2
            } InternalState;

            bool Exited();
            int State();
            ListenerList Listeners();

            std::thread* thread;

            OutputPtr output;
            IStreamPtr stream;
            ListenerList listeners;

            std::string url;

            std::mutex queueMutex, listenerMutex;
            std::condition_variable writeToOutputCondition;

            double volume;
            std::atomic<double> currentPosition;
            std::atomic<double> seekToPosition;
            std::atomic<musik::core::sdk::StreamState> streamState;
            std::atomic<int> internalState;
            bool notifiedStarted;
            int pendingBufferCount;
    };

} } }