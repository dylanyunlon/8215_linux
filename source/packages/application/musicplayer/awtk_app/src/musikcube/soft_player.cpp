/*
 * soft_player.cpp — Software decoding playback engine.
 *
 * Replaces MediaPlayer (硬解) with musikcube components (BSD-3-Clause):
 *   LocalFileStream → FfmpegDecoder → Buffer → AlsaOut
 *
 * The playback loop runs in a dedicated thread:
 *   1. Open file via LocalFileStream
 *   2. FfmpegDecoder reads/decodes → fills Buffer with float PCM
 *   3. AlsaOut.Play() writes Buffer to ALSA sound card
 *   4. Loop until EOF or stop requested
 *
 * Copyright (c) 2026. Portions (c) 2004-2023 musikcube team (BSD-3-Clause).
 */

#include "soft_player.h"
#include "StderrDebug.h"
#include "audio/Buffer.h"
#include "io/LocalFileStream.h"
#include "plugins/ffmpegdecoder/FfmpegDecoder.h"
#include "plugins/alsaout/AlsaOut.h"
#include "sdk/IBufferProvider.h"

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <cstring>

using namespace musik::core::sdk;
using namespace musik::core::audio;

/* ---- Wire up FfmpegDecoder's global debug pointer ---- */
extern "C" void SetDebug(IDebug* debug);

static StderrDebug s_debug;
static bool s_debug_inited = false;

static void ensure_debug() {
    if (!s_debug_inited) {
        SetDebug(&s_debug);
        s_debug_inited = true;
    }
}

/* ---- BufferProvider stub (AlsaOut calls back when done with a buffer) ---- */
class SimpleBufferProvider : public IBufferProvider {
public:
    void OnBufferProcessed(IBuffer* buffer) override {
        /* AlsaOut calls this when it's done writing a buffer to the device.
         * We don't use a buffer pool — just signal that we can submit the next one. */
        std::lock_guard<std::mutex> lock(mtx);
        pending--;
        cv.notify_all();
    }

    void WaitUntilDrained() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]{ return pending <= 0; });
    }

    void Submitted() {
        std::lock_guard<std::mutex> lock(mtx);
        pending++;
    }

    std::mutex mtx;
    std::condition_variable cv;
    int pending = 0;
};

/* ---- Internal context ---- */
struct SoftPlayerContext {
    std::atomic<SoftPlayerState> state{SOFT_STATE_IDLE};
    std::atomic<double> position{0.0};
    std::atomic<double> duration{0.0};
    std::atomic<double> seekTo{-1.0};
    std::atomic<bool> quit{false};
    std::atomic<bool> paused{false};

    soft_state_cb state_cb = nullptr;
    void* state_cb_data = nullptr;

    AlsaOut* output = nullptr;
    std::thread* playThread = nullptr;
    std::mutex mtx;
    std::condition_variable pauseCv;

    std::string currentPath;
};

static void notify_state(SoftPlayerContext* ctx, SoftPlayerState s) {
    ctx->state.store(s);
    if (ctx->state_cb) {
        ctx->state_cb(s, ctx->state_cb_data);
    }
}

/* ---- Playback thread ---- */
static void playback_thread(SoftPlayerContext* ctx, std::string filepath) {
    printf("[SoftPlayer] Thread start: %s\n", filepath.c_str());

    /* 1. Open file */
    LocalFileStream stream;
    if (!stream.Open(filepath.c_str(), OpenFlags::Read)) {
        fprintf(stderr, "[SoftPlayer] Cannot open: %s\n", filepath.c_str());
        notify_state(ctx, SOFT_STATE_ERROR);
        return;
    }

    /* 2. Create decoder */
    FfmpegDecoder decoder;
    if (!decoder.Open(&stream)) {
        fprintf(stderr, "[SoftPlayer] Decoder failed: %s\n", filepath.c_str());
        notify_state(ctx, SOFT_STATE_ERROR);
        return;
    }

    ctx->duration.store(decoder.GetDuration());
    printf("[SoftPlayer] Duration: %.1f sec\n", ctx->duration.load());

    /* 3. Decode + output loop */
    Buffer buffer;
    buffer.SetSampleRate(44100);
    buffer.SetChannels(2);
    buffer.SetSamples(4096);

    SimpleBufferProvider provider;
    bool started = false;

    notify_state(ctx, SOFT_STATE_PLAYING);

    while (!ctx->quit.load()) {
        /* Handle pause */
        if (ctx->paused.load()) {
            if (ctx->output) ctx->output->Pause();
            {
                std::unique_lock<std::mutex> lock(ctx->mtx);
                ctx->pauseCv.wait(lock, [ctx]{
                    return !ctx->paused.load() || ctx->quit.load();
                });
            }
            if (ctx->quit.load()) break;
            if (ctx->output) ctx->output->Resume();
        }

        /* Handle seek */
        double seek = ctx->seekTo.exchange(-1.0);
        if (seek >= 0.0) {
            double actual = decoder.SetPosition(seek);
            if (actual >= 0.0) {
                ctx->position.store(actual);
            }
        }

        /* Decode one buffer */
        if (!decoder.GetBuffer(&buffer)) {
            if (decoder.Exhausted()) {
                printf("[SoftPlayer] EOF reached\n");
                break; /* natural end */
            }
            /* transient decode error, try next frame */
            continue;
        }

        /* Update position */
        long sr = buffer.SampleRate();
        int ch = buffer.Channels();
        if (sr > 0 && ch > 0) {
            double bufDuration = (double)buffer.Samples() / (double)(sr * ch);
            ctx->position.store(ctx->position.load() + bufDuration);
        }

        /* Output to ALSA */
        if (ctx->output) {
            provider.Submitted();
            OutputState result = ctx->output->Play(&buffer, &provider);

            if (result == OutputState::FormatError) {
                fprintf(stderr, "[SoftPlayer] ALSA format error\n");
                notify_state(ctx, SOFT_STATE_ERROR);
                break;
            }

            /* If the output returns a positive value (BufferFull), wait */
            while ((int)result > 0 && !ctx->quit.load()) {
                std::this_thread::sleep_for(std::chrono::milliseconds((int)result));
                result = ctx->output->Play(&buffer, &provider);
            }

            if (!started) {
                started = true;
                printf("[SoftPlayer] First buffer written to ALSA\n");
            }
        }
    }

    /* Drain remaining audio */
    if (!ctx->quit.load() && ctx->output) {
        provider.WaitUntilDrained();
        ctx->output->Drain();
    }

    /* Final state */
    if (!ctx->quit.load()) {
        notify_state(ctx, SOFT_STATE_STOPPED);
    }

    printf("[SoftPlayer] Thread exit\n");
}

/* ---- Public API (extern "C") ---- */

extern "C" {

SoftPlayerContext* soft_player_create(void) {
    ensure_debug();

    SoftPlayerContext* ctx = new (std::nothrow) SoftPlayerContext();
    if (!ctx) return nullptr;

    ctx->output = new AlsaOut();
    printf("[SoftPlayer] Created (ALSA output)\n");
    return ctx;
}

void soft_player_destroy(SoftPlayerContext* ctx) {
    if (!ctx) return;

    soft_player_stop(ctx);

    if (ctx->output) {
        ctx->output->Release();
        ctx->output = nullptr;
    }

    delete ctx;
    printf("[SoftPlayer] Destroyed\n");
}

int soft_player_play(SoftPlayerContext* ctx, const char* filepath) {
    if (!ctx || !filepath) return -1;

    /* Stop current playback first */
    soft_player_stop(ctx);

    ctx->quit.store(false);
    ctx->paused.store(false);
    ctx->position.store(0.0);
    ctx->seekTo.store(-1.0);
    ctx->currentPath = filepath;

    ctx->playThread = new std::thread(playback_thread, ctx, std::string(filepath));
    return 0;
}

int soft_player_stop(SoftPlayerContext* ctx) {
    if (!ctx) return -1;

    ctx->quit.store(true);
    ctx->paused.store(false);
    ctx->pauseCv.notify_all();

    if (ctx->playThread) {
        if (ctx->playThread->joinable()) {
            ctx->playThread->join();
        }
        delete ctx->playThread;
        ctx->playThread = nullptr;
    }

    if (ctx->output) {
        ctx->output->Stop();
    }

    ctx->state.store(SOFT_STATE_STOPPED);
    return 0;
}

int soft_player_pause(SoftPlayerContext* ctx) {
    if (!ctx) return -1;
    ctx->paused.store(true);
    notify_state(ctx, SOFT_STATE_PAUSED);
    return 0;
}

int soft_player_resume(SoftPlayerContext* ctx) {
    if (!ctx) return -1;
    ctx->paused.store(false);
    ctx->pauseCv.notify_all();
    notify_state(ctx, SOFT_STATE_PLAYING);
    return 0;
}

int soft_player_seek(SoftPlayerContext* ctx, double seconds) {
    if (!ctx) return -1;
    ctx->seekTo.store(seconds);
    return 0;
}

SoftPlayerState soft_player_get_state(SoftPlayerContext* ctx) {
    return ctx ? ctx->state.load() : SOFT_STATE_IDLE;
}

double soft_player_get_position(SoftPlayerContext* ctx) {
    return ctx ? ctx->position.load() : 0.0;
}

double soft_player_get_duration(SoftPlayerContext* ctx) {
    return ctx ? ctx->duration.load() : 0.0;
}

void soft_player_set_volume(SoftPlayerContext* ctx, double vol) {
    if (ctx && ctx->output) {
        ctx->output->SetVolume(vol);
    }
}

void soft_player_set_state_callback(SoftPlayerContext* ctx, soft_state_cb cb, void* user_data) {
    if (!ctx) return;
    ctx->state_cb = cb;
    ctx->state_cb_data = user_data;
}

} /* extern "C" */
