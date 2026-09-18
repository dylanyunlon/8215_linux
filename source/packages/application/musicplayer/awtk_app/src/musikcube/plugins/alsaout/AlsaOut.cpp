//////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2007-2016 musikcube team
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the author nor the names of other contributors may
//      be used to endorse or promote products derived from this software
//      without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////////

#include "AlsaOut.h"

#include "../sdk/constants.h"
#include "../sdk/IPreferences.h"
#include <cstdint>
#include <execinfo.h>

static musik::core::sdk::IPreferences* prefs;

#define BUFFER_COUNT 16
#define PCM_ACCESS_TYPE SND_PCM_ACCESS_RW_INTERLEAVED
#define PREF_DEVICE_ID "device_id"

/* AC83xx only supports S8/U8/S16_LE/U16_LE natively.
 * Try FLOAT first via plughw (auto-converts), fall back to S16_LE
 * with manual float->s16 conversion in WriteLoop. */
static snd_pcm_format_t s_pcmFormat = SND_PCM_FORMAT_FLOAT_LE;
static bool s_needFloatToS16 = false;

#define LOCK(x) \
    /*std::cerr << "locking " << x << "\n";*/ \
    std::unique_lock<std::recursive_mutex> lock(this->stateMutex); \
    /*std::cerr << "locked " << x << "\n";*/ \

#define WAIT() this->threadEvent.wait(lock);
#define NOTIFY() this->threadEvent.notify_all();
#define CHECK_QUIT() if (this->quit) { return; }
#define PRINT_ERROR(x) std::cerr << "AlsaOut: error! " << snd_strerror(x) << std::endl;

#define WRITE_BUFFER(handle, context, samples) \
    err = snd_pcm_writei(handle, context->buffer->BufferPointer(), samples); \
    if (err < 0) { PRINT_ERROR(err); }

static inline bool playable(snd_pcm_t* pcm) {
    if (!pcm) {
        return false;
    }

    snd_pcm_state_t state = snd_pcm_state(pcm);

    if (state == SND_PCM_STATE_RUNNING ||
        state == SND_PCM_STATE_PREPARED)
    {
        return true;
    }

    /* SND_PCM_STATE_PAUSED (4): if hardware pause was triggered by some
     * external path, unpause via snd_pcm_pause(0) so we can keep writing. */
    if (state == SND_PCM_STATE_PAUSED) {
        std::cerr << "AlsaOut: PCM in PAUSED state, unpausing hardware\n";
        int err = snd_pcm_pause(pcm, 0);
        if (err == 0) return true;
        /* If unpause fails, fall through to prepare-recovery below */
        std::cerr << "AlsaOut: snd_pcm_pause(0) failed: " << snd_strerror(err)
                  << ", trying prepare\n";
    }

    /* Auto-recover from XRUN / SETUP / failed-unpause — don't just give up */
    if (state == SND_PCM_STATE_XRUN ||
        state == SND_PCM_STATE_SETUP ||
        state == SND_PCM_STATE_SUSPENDED ||
        state == SND_PCM_STATE_PAUSED)
    {
        std::cerr << "AlsaOut: device not playable, recovering..."
                  << " (state=" << (int)state << ")\n";
        int err = snd_pcm_prepare(pcm);
        if (err == 0) {
            std::cerr << "AlsaOut: device recovered OK\n";
            return true;
        }
        std::cerr << "AlsaOut: recovery failed: " << snd_strerror(err) << "\n";
        return false;
    }

    std::cerr << "AlsaOut: invalid device state: " << (int) state << "\n";
    return false;
}

using namespace musik::core::sdk;

class AlsaDevice : public IDevice {
    public:
        AlsaDevice(const std::string& id, const std::string& name) {
            this->id = id;
            this->name = name;
        }

        virtual void Release() override {
            delete this;
        }

        virtual const char* Name() const override {
            return name.c_str();
        }

        virtual const char* Id() const override {
            return id.c_str();
        }

    private:
        std::string name, id;
};

class AlsaDeviceList : public musik::core::sdk::IDeviceList {
    public:
        virtual void Release() override {
            delete this;
        }

        virtual size_t Count() const override {
            return devices.size();
        }

        virtual const IDevice* At(size_t index) const override {
            return &devices.at(index);
        }

        void Add(const std::string& id, const std::string& name) {
            devices.push_back(AlsaDevice(id, name));
        }

    private:
        std::vector<AlsaDevice> devices;
};

extern "C" void SetPreferences(musik::core::sdk::IPreferences* prefs) {
    /* No preferences on embedded — always use ALSA "default" device */
    (void)prefs;
}

static std::string getDeviceId() {
    return "";  /* empty = use default device set in constructor */
}

AlsaOut::AlsaOut()
: pcmHandle(nullptr)
, device("plughw:0,0")
, channels(2)
, rate(44100)
, volume(1.0)
, quit(false)
, paused(false)
, latency(0)
, initialized(false) {
    std::cerr << "AlsaOut::AlsaOut() called" << std::endl;
    this->writeThread.reset(new std::thread(std::bind(&AlsaOut::WriteLoop, this)));
}

AlsaOut::~AlsaOut() {
    std::cerr << "AlsaOut: destructor\n";

    {
        LOCK("dtor");
        this->quit = true;
        NOTIFY();
    }

    std::cerr << "AlsaOut: joining...\n";
    this->writeThread->join();

    std::cerr << "AlsaOut: closing device...\n";
    this->CloseDevice();

    std::cerr << "AlsaOut: destroyed.\n";
}

void AlsaOut::CloseDevice() {
    LOCK("CloseDevice()");
    if (this->pcmHandle) {
        /* Print backtrace hint — who is closing the PCM? */
        void* callstack[8];
        int frames = backtrace(callstack, 8);
        std::cerr << "AlsaOut: closing PCM handle (caller stack " << frames << " frames):\n";
        backtrace_symbols_fd(callstack, frames, 2);
        snd_pcm_close(this->pcmHandle);
        this->pcmHandle = nullptr;
        this->latency = 0.0;
    }
}

musik::core::sdk::IDevice* AlsaOut::GetDefaultDevice() {
    return findDeviceById<AlsaDevice, IOutput>(this, getDeviceId());
}

bool AlsaOut::SetDefaultDevice(const char* deviceId) {
    /* No preferences on embedded — ignore */
    (void)deviceId;
    return true;
}

IDeviceList* AlsaOut::GetDeviceList() {
    AlsaDeviceList* result = new AlsaDeviceList();

    /* https://stackoverflow.com/a/6870226 */
    char** hints;
    if (snd_device_name_hint(-1, "pcm", (void***)&hints) == 0) {
        char** n = hints;
        while (*n != nullptr) {
            char *name = snd_device_name_get_hint(*n, "NAME");
            if (name) {
                std::string stdName = name;
                if (stdName != "default") {
                    result->Add(stdName, stdName);
                }
                free(name);
            }
            ++n;
        }

        snd_device_name_free_hint((void**) hints);
    }

    size_t n = result->Count();
    return result;
}

std::string AlsaOut::GetPreferredDeviceId() {
    std::string result;

    if (prefs) {
        std::string storedDeviceId = getDeviceId();

        auto deviceList = GetDeviceList();
        if (deviceList) {
            for (size_t i = 0; i < deviceList->Count(); i++) {
                if (deviceList->At(i)->Id() == storedDeviceId) {
                    result = storedDeviceId;
                    break;
                }
            }
            deviceList->Release();
        }
    }

    return result;
}

void AlsaOut::InitDevice() {
    int err, dir;
    unsigned int rate = (unsigned int) this->rate;

    std::string preferredDeviceId = this->GetPreferredDeviceId();
    bool preferredOk = false;

    if (preferredDeviceId.size() > 0) {
        if ((err = snd_pcm_open(&this->pcmHandle, preferredDeviceId.c_str(), SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
            std::cerr << "AlsaOut: cannot opened preferred device id " << preferredDeviceId << ": " << snd_strerror(err) << std::endl;
        }
        else {
            preferredOk = true;
        }
    }

    if (!preferredOk && (err = snd_pcm_open(&this->pcmHandle, this->device.c_str(), SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        std::cerr << "AlsaOut: cannot open audio device 'default' :" << snd_strerror(err) << std::endl;
        goto error;
    }

    if ((err = snd_pcm_hw_params_malloc(&hardware)) < 0) {
        std::cerr << "AlsaOut: cannot allocate hardware parameter structure " << snd_strerror(err) << std::endl;
        goto error;
    }

    if ((err = snd_pcm_hw_params_any(pcmHandle, hardware)) < 0) {
        std::cerr << "AlsaOut: cannot initialize hardware parameter structure " << snd_strerror(err) << std::endl;
        goto error;
    }

    if ((err = snd_pcm_hw_params_set_access(pcmHandle, hardware, PCM_ACCESS_TYPE)) < 0) {
        std::cerr << "AlsaOut: cannot set access type " << snd_strerror(err) << std::endl;
        goto error;
    }

    /* Try FLOAT_LE first (musikcube internal format). If the device rejects
     * it (AC83xx only supports S16_LE), fall back to S16_LE and convert
     * float->s16 in WriteLoop before snd_pcm_writei. */
    s_pcmFormat = SND_PCM_FORMAT_FLOAT_LE;
    s_needFloatToS16 = false;
    if ((err = snd_pcm_hw_params_set_format(pcmHandle, hardware, SND_PCM_FORMAT_FLOAT_LE)) < 0) {
        std::cerr << "AlsaOut: FLOAT_LE not supported, trying S16_LE\n";
        if ((err = snd_pcm_hw_params_set_format(pcmHandle, hardware, SND_PCM_FORMAT_S16_LE)) < 0) {
            std::cerr << "AlsaOut: cannot set sample format " << snd_strerror(err) << std::endl;
            goto error;
        }
        s_pcmFormat = SND_PCM_FORMAT_S16_LE;
        s_needFloatToS16 = true;
        std::cerr << "AlsaOut: using S16_LE with float->s16 conversion\n";
    }

    if ((err = snd_pcm_hw_params_set_rate_near(pcmHandle, hardware, &rate, 0)) < 0) {
        std::cerr << "AlsaOut: cannot set sample rate " << snd_strerror(err) << std::endl;
        goto error;
    }

    if ((err = snd_pcm_hw_params_set_channels(pcmHandle, hardware, this->channels)) < 0) {
        std::cerr << "AlsaOut: cannot set channel count " << snd_strerror(err) << std::endl;
        goto error;
    }

    if ((err = snd_pcm_hw_params(pcmHandle, hardware)) < 0) {
        std::cerr << "AlsaOut: cannot set parameters " << snd_strerror(err) << std::endl;
        goto error;
    }

    snd_pcm_hw_params_free(hardware);

    if ((err = snd_pcm_prepare (pcmHandle)) < 0) {
        std::cerr << "AlsaOut: cannot prepare audio interface for use " << snd_strerror(err) << std::endl;
        goto error;
    }

    snd_pcm_nonblock(pcmHandle, 0); /* operate in blocking mode for simplicity */

    std::cerr << "AlsaOut: device seems to be prepared for use!\n";
    this->initialized = true;

    return;

error:
    this->CloseDevice();
}

void AlsaOut::Release() {
    delete this;
}

double AlsaOut::Latency() {
    if (latency <= 0.0f) {
        LOCK("latency_calc");

        if (this->pcmHandle && this->rate && this->channels) {
            snd_pcm_uframes_t bufferSize = 0, periodSize = 0;
            snd_pcm_get_params(this->pcmHandle, &bufferSize, &periodSize);

            if (bufferSize) {
                size_t sampleSize = s_needFloatToS16 ? sizeof(int16_t) : sizeof(float);
                this->latency =
                (double) bufferSize /
                (double) (this->rate * this->channels * sampleSize);
            }
        }
    }

    return this->latency;
}

void AlsaOut::Stop() {
    std::list<std::shared_ptr<BufferContext> > toNotify;

    {
        LOCK("stop");

        std::swap(this->buffers, toNotify);

        if (this->pcmHandle) {
            snd_pcm_drop(this->pcmHandle);
            this->CloseDevice();
        }
    }

    auto it = toNotify.begin();
    while (it != toNotify.end()) {
        ((*it)->provider)->OnBufferProcessed((*it)->buffer);
        ++it;
    }
}

void AlsaOut::Pause() {
    LOCK("pause");
    this->paused = true;
    /* Do NOT call snd_pcm_pause() here.
     *
     * Root cause of the deadlock (2026-09 field bug):
     *   snd_pcm_pause(handle,1) stops the hardware from draining the ring
     *   buffer.  WriteLoop runs in blocking mode (snd_pcm_nonblock=0), so
     *   the next snd_pcm_writei() blocks forever once the buffer is full.
     *   WriteLoop holds stateMutex while inside writei, so Resume() can
     *   never acquire the mutex to call snd_pcm_pause(handle,0) → deadlock.
     *
     * Fix: set the paused flag only. WriteLoop checks this flag BEFORE
     * calling writei and waits on the condition variable instead.
     * Resume() clears the flag and notifies, so WriteLoop wakes up and
     * continues writing.  The PCM stream stays RUNNING during "pause" —
     * we just stop feeding it new samples; ALSA drains what's in the ring
     * buffer (≤ 0.5s of latency audio) then the device goes silent via
     * underrun, which we recover from on resume.  This is simpler and
     * deadlock-free. */
    NOTIFY();
    std::cerr << "AlsaOut: paused (flag-based)\n";
}

void AlsaOut::Resume() {
    LOCK("resume");
    if (this->paused) {
        this->paused = false;
        /* If the PCM went into XRUN while we were paused (ring buffer
         * drained), playable() will recover it automatically via
         * snd_pcm_prepare() on the next WriteLoop iteration. */
        NOTIFY();
        std::cerr << "AlsaOut: resumed\n";
    }
}

void AlsaOut::SetVolume(double volume) {
    LOCK("set volume");
    this->volume = volume;
}

double AlsaOut::GetVolume() {
    return this->volume;
}

void AlsaOut::WriteLoop() {
    {
        LOCK("thread: init");
        while (!quit && !initialized) {
            WAIT();
        }
    }

    {
        while (!quit) {
            std::shared_ptr<BufferContext> next;

            {
                LOCK("thread: waiting for buffer");

                /* Wait until: not quit, not paused, device playable, and
                 * at least one buffer is queued.  Checking `paused` HERE
                 * (before we pop a buffer and enter writei) is the fix for
                 * the deadlock: we never call snd_pcm_writei while paused,
                 * so we never block inside writei holding stateMutex. */
                while (!quit && (this->paused || !playable(this->pcmHandle) || !this->buffers.size())) {
                    WAIT();
                }

                CHECK_QUIT();

                next = this->buffers.front();
                this->buffers.pop_front();
            }

            int err = 0;

            if (next) {
                size_t samples = next->buffer->Samples();
                size_t channels = next->buffer->Channels();
                size_t samplesPerChannel = samples / channels;
                float volume = (float) this->volume;

                /* software volume */
                if (volume != 1.0f) {
                    float *buffer = next->buffer->BufferPointer();
                    for (size_t i = 0; i < samples; i++) {
                        (*buffer) *= volume;
                        ++buffer;
                    }
                }

                {
                    LOCK("WRITE_BUFFER()");

                    /* Re-check paused under lock — if Pause() was called
                     * between the pop above and this lock acquisition,
                     * skip the write entirely; the buffer will be returned
                     * to the provider below and the loop will re-check. */
                    if (this->paused) {
                        /* Don't write — just fall through to OnBufferProcessed */
                    }
                    else if (this->pcmHandle) {
                        if (s_needFloatToS16) {
                            /* Convert float [-1.0, 1.0] -> S16_LE [-32768, 32767] */
                            float* src = next->buffer->BufferPointer();
                            std::vector<int16_t> s16buf(samples);
                            for (size_t i = 0; i < samples; i++) {
                                float s = src[i];
                                if (s > 1.0f) s = 1.0f;
                                if (s < -1.0f) s = -1.0f;
                                s16buf[i] = (int16_t)(s * 32767.0f);
                            }
                            err = snd_pcm_writei(this->pcmHandle, s16buf.data(), samplesPerChannel);
                            if (err < 0) { PRINT_ERROR(err); }
                        } else {
                            WRITE_BUFFER(this->pcmHandle, next, samplesPerChannel);
                        }

                        if (err == -EINTR || err == -EPIPE || err == -ESTRPIPE) {
                            if (!snd_pcm_recover(this->pcmHandle, err, 1)) {
                                /* try one more time */
                                if (s_needFloatToS16) {
                                    float* src = next->buffer->BufferPointer();
                                    std::vector<int16_t> s16buf(samples);
                                    for (size_t i = 0; i < samples; i++) {
                                        float s = src[i];
                                        if (s > 1.0f) s = 1.0f;
                                        if (s < -1.0f) s = -1.0f;
                                        s16buf[i] = (int16_t)(s * 32767.0f);
                                    }
                                    err = snd_pcm_writei(this->pcmHandle, s16buf.data(), samplesPerChannel);
                                    if (err < 0) { PRINT_ERROR(err); }
                                } else {
                                    WRITE_BUFFER(this->pcmHandle, next, samplesPerChannel);
                                }
                            }
                        }

                        if (err > 0 && err < (int) samplesPerChannel) {
                            std::cerr << "AlsaOut: short write. expected=" << samplesPerChannel << ", actual=" << err << std::endl;
                        }
                    }
                }

                next->provider->OnBufferProcessed(next->buffer);
            }
        }
    }

    std::cerr << "AlsaOut: thread finished\n";
}

OutputState AlsaOut::Play(IBuffer *buffer, IBufferProvider* provider) {
    this->SetFormat(buffer);

    {
        LOCK("play");

        /* Accept buffers even while paused — they queue up and WriteLoop
         * will drain them when resumed.  Returning InvalidState here was
         * part of the deadlock: the Player decode thread would spin-retry
         * forever, and once WriteLoop got stuck in writei holding the
         * mutex the entire player froze.
         *
         * We still cap the queue at BUFFER_COUNT so the decode thread
         * doesn't run ahead unboundedly during a long pause. */
        if (this->CountBuffersWithProvider(provider) >= BUFFER_COUNT) {
            return OutputState::BufferFull;
        }

        std::shared_ptr<BufferContext> context(new BufferContext());
        context->buffer = buffer;
        context->provider = provider;

        this->buffers.push_back(context);

        if (!this->paused) {
            if (!playable(this->pcmHandle)) {
                std::cerr << "AlsaOut: sanity check -- stream not playable. adding buffer to queue anyway\n";
            }
            else {
                NOTIFY();
            }
        }
        /* When paused, don't NOTIFY — WriteLoop is waiting on the paused
         * flag anyway.  The buffers will be consumed after Resume(). */
    }

    return OutputState::BufferWritten;
}

void AlsaOut::Drain() {
    LOCK("drain");

    if (this->pcmHandle) {
        std::cerr << "draining...\n";
        snd_pcm_drain(this->pcmHandle);
        std::cerr << "drained\n";
    }
}

void AlsaOut::SetFormat(IBuffer *buffer) {
    LOCK("set format");

    if (this->channels != buffer->Channels() ||
        this->rate != buffer->SampleRate() ||
        this->pcmHandle == nullptr)
    {
        this->channels = buffer->Channels();
        this->rate = buffer->SampleRate();

        this->CloseDevice();

        this->InitDevice();

        if (this->pcmHandle) {
            int err = snd_pcm_set_params(
                this->pcmHandle,
                s_pcmFormat,
                PCM_ACCESS_TYPE,
                this->channels,
                this->rate,
                1, /* allow resampling */
                500000); /* 0.5s latency */

            if (err > 0) {
                std::cerr << "AlsaOut: set format error: " << snd_strerror(err) << std::endl;
            }
            else {
                this->SetVolume(this->volume);
            }
        }

        std::cerr << "AlsaOut: device format initialized from buffer\n";
    }
}

size_t AlsaOut::CountBuffersWithProvider(IBufferProvider* provider) {
    LOCK("count");

    size_t count = 0;
    auto it = this->buffers.begin();
    while (it != this->buffers.end()) {
        if ((*it)->provider == provider) {
            ++count;
        }
        ++it;
    }

    return count;
}