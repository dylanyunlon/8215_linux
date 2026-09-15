/*
 * music_player.cpp - Music playback controller
 *
 * Uses MediaPlayer (libatcmediaplayer.so) for hardware decode,
 * or GaplessTransport (musikcube engine: FFmpeg + ALSA) for software decode.
 *
 * GaplessTransport manages the Player lifecycle, dual-player gapless
 * transitions, volume, pause/resume. This file manages the playlist,
 * shuffle, repeat mode, and callbacks to the UI layer.
 */

#include "music_player.h"
#include "atcmediaplayer.h"

#ifdef USE_SOFT_PLAYER
#include "musikcube/audio/GaplessTransport.h"
#include "musikcube/sdk/constants.h"
using GaplessTransport = musik::core::audio::GaplessTransport;
using PlaybackState    = musik::core::sdk::PlaybackState;
using StreamState      = musik::core::sdk::StreamState;
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <mutex>
#include <pthread.h>
#include <unistd.h>
#include <ctime>

#define PREVIOUS_GRACE_PERIOD 2.0 /* seconds — if position > this, prev = restart */

/* --- Internal context structure --- */

struct MusicPlayerContext {
    /* Hardware decoder (ATC MediaPlayer) */
    MediaPlayer         *player;
    bool                 player_ready;

    /* Software decoder (musikcube GaplessTransport) */
    bool                 use_soft;
#ifdef USE_SOFT_PLAYER
    GaplessTransport    *transport;
#else
    void                *transport;
#endif

    /* Playlist */
    std::vector<TrackRef>  playlist;
    int                    current_index;
    int                    next_index;     /* gapless: index of PrepareNextTrack */

    /* Shuffle pool (consumption-based, Issue #19) */
    std::vector<int>       shuffle_pool;

    /* State */
    PlayerState          state;
    PlayMode             mode;

    /* Callbacks */
    on_state_changed_fn    state_cb;
    void                  *state_cb_data;
    on_track_changed_fn    track_cb;
    void                  *track_cb_data;
    on_position_changed_fn position_cb;
    void                  *position_cb_data;

    /* Position polling thread */
    pthread_t            poll_thread;
    bool                 poll_running;

    std::mutex           mtx;
};

/* --- State callback from MediaPlayer (硬解) --- */

static void media_state_callback_wrapper(int new_state, void *user_data)
{
    MusicPlayerContext *ctx = (MusicPlayerContext *)user_data;
    if (!ctx) return;

    PlayerState s;
    on_state_changed_fn cb;
    void *data;

    {
        std::lock_guard<std::mutex> lock(ctx->mtx);

        switch (new_state) {
        case MediaPlayer::PlayingState:  ctx->state = PLAYER_STATE_PLAYING; break;
        case MediaPlayer::PausedState:   ctx->state = PLAYER_STATE_PAUSED;  break;
        case MediaPlayer::StoppedState:  ctx->state = PLAYER_STATE_STOPPED; break;
        case MediaPlayer::ErrorState:    ctx->state = PLAYER_STATE_ERROR;   break;
        }

        s = ctx->state;
        cb = ctx->state_cb;
        data = ctx->state_cb_data;
    }

    if (cb) cb(s, data);
}

/* --- GaplessTransport → music_player state bridge (软解) --- */

#ifdef USE_SOFT_PLAYER
static void on_transport_playback_state(MusicPlayerContext *ctx, PlaybackState ps)
{
    if (!ctx) return;

    PlayerState s;
    on_state_changed_fn cb;
    void *data;

    {
        std::lock_guard<std::mutex> lock(ctx->mtx);

        switch (ps) {
        case PlaybackState::Playing:  s = PLAYER_STATE_PLAYING; break;
        case PlaybackState::Paused:   s = PLAYER_STATE_PAUSED;  break;
        case PlaybackState::Stopped:  s = PLAYER_STATE_STOPPED; break;
        default:                      s = PLAYER_STATE_IDLE;    break;
        }

        ctx->state = s;
        cb = ctx->state_cb;
        data = ctx->state_cb_data;
    }

    if (cb) cb(s, data);
}

static void on_transport_stream_state(MusicPlayerContext *ctx, StreamState ss, const std::string& uri)
{
    if (!ctx) return;

    /*
     * musikcube PlaybackService::ProcessMessage pattern:
     *
     * Playing  → if nextIndex is set and URI matches, promote nextIndex
     *            to current index, fire OnTrackChanged (updates UI info).
     * AlmostDone → prepare the next track for gapless.
     * Finished → auto-advance (only when no gapless next was started).
     */
    if (ss == StreamState::Playing) {
        /* Gapless transition: nextPlayer just started playing.
         * Update current_index so UI shows the new track info. */
        on_track_changed_fn tcb = nullptr;
        void *tdata = nullptr;
        int new_idx = -1;

        {
            std::lock_guard<std::mutex> lock(ctx->mtx);
            if (ctx->next_index >= 0 &&
                ctx->next_index < (int)ctx->playlist.size())
            {
                /* Verify URI matches — same check musikcube does to guard
                 * against rapid skip races */
                if (uri == ctx->playlist[ctx->next_index].filepath) {
                    ctx->current_index = ctx->next_index;
                    ctx->next_index = -1;
                    new_idx = ctx->current_index;
                    tcb = ctx->track_cb;
                    tdata = ctx->track_cb_data;
                }
            }
        }

        if (tcb && new_idx >= 0) {
            printf("[MusicPlayer] Gapless transition → [%d/%d]: %s\n",
                   new_idx + 1, (int)ctx->playlist.size(), uri.c_str());
            tcb(new_idx, NULL, tdata);
        }
    }
    else if (ss == StreamState::Finished) {
        /* Don't hold the lock across music_player_next — it takes the lock too */
        printf("[MusicPlayer] Track finished: %s, auto-next\n", uri.c_str());
        music_player_next(ctx);
    }
    /* When decoder is almost done, prepare the next track for gapless */
    else if (ss == StreamState::AlmostDone) {
        std::lock_guard<std::mutex> lock(ctx->mtx);

        int count = (int)ctx->playlist.size();
        if (count <= 0) return;

        int next = -1;
        switch (ctx->mode) {
        case PLAY_MODE_REPEAT_ONE:
            next = ctx->current_index;
            break;
        case PLAY_MODE_SEQUENTIAL:
            next = ctx->current_index + 1;
            if (next >= count) next = -1; /* will stop */
            break;
        case PLAY_MODE_REPEAT_ALL:
            next = (ctx->current_index + 1) % count;
            break;
        case PLAY_MODE_SHUFFLE:
            /* can't predict shuffle next here, just let it stop
             * and auto-next in Finished will handle it */
            return;
        }

        if (next >= 0 && next < count && ctx->transport) {
            ctx->next_index = next;
            printf("[MusicPlayer] Preparing next track [%d]: %s\n",
                   next, ctx->playlist[next].filepath);
            ctx->transport->PrepareNextTrack(std::string(ctx->playlist[next].filepath));
        }
    }
}
#endif

/* --- Shuffle helpers (Issue #19: consumption pool) --- */

static void rebuild_shuffle_pool(MusicPlayerContext *ctx)
{
    int n = (int)ctx->playlist.size();
    ctx->shuffle_pool.resize(n);
    for (int i = 0; i < n; i++)
        ctx->shuffle_pool[i] = i;
}

static int shuffle_pool_pick_next(MusicPlayerContext *ctx)
{
    if (ctx->shuffle_pool.empty()) {
        rebuild_shuffle_pool(ctx);
    }
    if (ctx->shuffle_pool.empty()) return 0;

    int pick = rand() % (int)ctx->shuffle_pool.size();
    int result = ctx->shuffle_pool[pick];

    ctx->shuffle_pool[pick] = ctx->shuffle_pool.back();
    ctx->shuffle_pool.pop_back();

    return result;
}

static void generate_shuffle(MusicPlayerContext *ctx)
{
    srand((unsigned)time(NULL));
    rebuild_shuffle_pool(ctx);
}

/* --- Position polling thread --- */

static void *position_poll_func(void *arg)
{
    MusicPlayerContext *ctx = (MusicPlayerContext *)arg;

    while (ctx->poll_running) {
        usleep(500000); /* 500ms */

        on_position_changed_fn cb = nullptr;
        void *data = nullptr;
        int pos = 0;
        int dur = -1;

        {
            std::lock_guard<std::mutex> lock(ctx->mtx);
            if (ctx->state == PLAYER_STATE_PLAYING && ctx->player_ready && ctx->position_cb) {
                if (ctx->use_soft) {
#ifdef USE_SOFT_PLAYER
                    if (ctx->transport) {
                        pos = (int)(ctx->transport->Position() * 1000.0);
                        double d = ctx->transport->GetDuration();
                        if (d > 0.0) dur = (int)(d * 1000.0);
                    }
#endif
                } else if (ctx->player) {
                    pos = (int)ctx->player->getPosition();
                }
                cb = ctx->position_cb;
                data = ctx->position_cb_data;
            }
        }

        if (cb) cb(pos, dur, data);
    }
    return NULL;
}

/* --- Public API implementation --- */

extern "C" {

MusicPlayerContext *music_player_create(void)
{
    MusicPlayerContext *ctx = new (std::nothrow) MusicPlayerContext();
    if (!ctx) return NULL;

    ctx->player = NULL;
    ctx->player_ready = false;
    ctx->use_soft = false;
    ctx->transport = nullptr;
    ctx->current_index = -1;
    ctx->next_index = -1;
    ctx->state = PLAYER_STATE_IDLE;
    ctx->mode = PLAY_MODE_SEQUENTIAL;
    ctx->state_cb = NULL;
    ctx->state_cb_data = NULL;
    ctx->track_cb = NULL;
    ctx->track_cb_data = NULL;
    ctx->position_cb = NULL;
    ctx->position_cb_data = NULL;
    ctx->poll_running = false;
    ctx->poll_thread = 0;

    /* Try hardware decoder first */
    ctx->player = new (std::nothrow) MediaPlayer();
    if (ctx->player && ctx->player->setup()) {
        ctx->player->setStateCallback(media_state_callback_wrapper, ctx);
        ctx->player_ready = true;
        ctx->use_soft = false;
        printf("[MusicPlayer] Created with hardware decoder (MediaPlayer)\n");
    } else {
        fprintf(stderr, "[MusicPlayer] MediaPlayer setup failed, falling back to soft decoder\n");
        ctx->player = NULL;

#ifdef USE_SOFT_PLAYER
        ctx->transport = new GaplessTransport();
        if (!ctx->transport) {
            fprintf(stderr, "[MusicPlayer] GaplessTransport creation failed!\n");
            delete ctx;
            return NULL;
        }

        /* Wire up callbacks from transport → this context */
        ctx->transport->SetPlaybackStateCb(
            [ctx](PlaybackState ps) { on_transport_playback_state(ctx, ps); });
        ctx->transport->SetStreamStateCb(
            [ctx](StreamState ss, const std::string& uri) { on_transport_stream_state(ctx, ss, uri); });

        ctx->use_soft = true;
        ctx->player_ready = true;
        printf("[MusicPlayer] Created with GaplessTransport (musikcube + FFmpeg + ALSA)\n");
#else
        ctx->use_soft = false;
        ctx->player_ready = false;
        printf("[MusicPlayer] No soft decoder available, UI-only mode\n");
#endif
    }

    /* Start position polling thread */
    ctx->poll_running = true;
    pthread_create(&ctx->poll_thread, NULL, position_poll_func, ctx);

    return ctx;
}

void music_player_destroy(MusicPlayerContext *ctx)
{
    if (!ctx) return;

    ctx->poll_running = false;
    if (ctx->poll_thread) {
        pthread_join(ctx->poll_thread, NULL);
    }

    if (ctx->use_soft) {
#ifdef USE_SOFT_PLAYER
        if (ctx->transport) {
            ctx->transport->Stop();
            delete ctx->transport;
            ctx->transport = nullptr;
        }
#endif
    } else if (ctx->player) {
        if (ctx->state == PLAYER_STATE_PLAYING ||
            ctx->state == PLAYER_STATE_PAUSED) {
            ctx->player->stop();
        }
        delete ctx->player;
    }

    delete ctx;
    printf("[MusicPlayer] Destroyed\n");
}

int music_player_set_playlist_refs(MusicPlayerContext *ctx,
                                   const TrackRef *refs, int count)
{
    if (!ctx || !refs || count <= 0) return -1;

    std::lock_guard<std::mutex> lock(ctx->mtx);

    /*
     * musikcube pattern (HotSwap / CopyFrom):
     *   1. remember the playing track's identity (uid)
     *   2. replace the playlist
     *   3. IndexOf(playingId) to recover position in new list
     *   4. if not found → NO_POSITION
     *
     * Our uid comes from MusicInfo.uid assigned during scan.
     * For the incremental scan case the same file keeps the same uid,
     * so the lookup will always succeed.
     */
    int playing_uid = -1;
    if (ctx->current_index >= 0 &&
        ctx->current_index < (int)ctx->playlist.size()) {
        playing_uid = ctx->playlist[ctx->current_index].uid;
    }

    ctx->playlist.assign(refs, refs + count);

    /* IndexOf(playing_uid) — linear scan, same as musikcube TrackList::IndexOf */
    int restored = -1;
    if (playing_uid >= 0) {
        for (int i = 0; i < count; i++) {
            if (refs[i].uid == playing_uid) {
                restored = i;
                break;
            }
        }
    }

    if (restored >= 0) {
        /* HotSwap succeeded — keep current_index and state intact */
        ctx->current_index = restored;
        /* state stays PLAYING / PAUSED — don't touch */
        printf("[MusicPlayer] Playlist set: %d tracks, %.1f KB (hot idx=%d)\n",
               count, (double)ctx->playlist.size() * sizeof(TrackRef) / 1024.0,
               restored);
    } else {
        /* Fresh playlist or track disappeared — reset like CopyFrom fallback */
        ctx->current_index = -1;
        ctx->state = PLAYER_STATE_IDLE;
        printf("[MusicPlayer] Playlist set: %d tracks, %.1f KB\n",
               count, (double)ctx->playlist.size() * sizeof(TrackRef) / 1024.0);
    }

    generate_shuffle(ctx);

    return 0;
}

int music_player_set_playlist(MusicPlayerContext *ctx, const MusicList *list)
{
    if (!ctx || !list || list->count <= 0) return -1;

    std::vector<TrackRef> refs(list->count);
    for (int i = 0; i < list->count; i++) {
        refs[i].uid = list->items[i].uid;
        strncpy(refs[i].filepath, list->items[i].filepath,
                sizeof(refs[i].filepath) - 1);
        refs[i].filepath[sizeof(refs[i].filepath) - 1] = '\0';
    }

    return music_player_set_playlist_refs(ctx, refs.data(), (int)refs.size());
}

int music_player_get_playlist_count(MusicPlayerContext *ctx)
{
    return ctx ? (int)ctx->playlist.size() : 0;
}

const TrackRef *music_player_get_track_ref(MusicPlayerContext *ctx, int index)
{
    if (!ctx || index < 0 || index >= (int)ctx->playlist.size()) return NULL;
    return &ctx->playlist[index];
}

const MusicInfo *music_player_get_track_info(MusicPlayerContext *ctx, int index)
{
    (void)ctx; (void)index;
    return NULL;
}

int music_player_get_current_index(MusicPlayerContext *ctx)
{
    return ctx ? ctx->current_index : -1;
}

int music_player_play(MusicPlayerContext *ctx, int index)
{
    if (!ctx || !ctx->player_ready) return -1;

    std::unique_lock<std::mutex> lock(ctx->mtx);

    if (index == -1) {
        if (ctx->current_index >= 0 && ctx->state == PLAYER_STATE_PAUSED) {
            if (ctx->use_soft) {
#ifdef USE_SOFT_PLAYER
                if (ctx->transport) ctx->transport->Resume();
#endif
            } else {
                ctx->player->resume();
            }
            ctx->state = PLAYER_STATE_PLAYING;

            PlayerState s = ctx->state;
            on_state_changed_fn scb = ctx->state_cb;
            void *sdata = ctx->state_cb_data;
            lock.unlock();
            if (scb) scb(s, sdata);
            return 0;
        }
        index = (ctx->current_index >= 0) ? ctx->current_index : 0;
    }

    if (index < 0 || index >= (int)ctx->playlist.size()) {
        return -1;
    }

    ctx->current_index = index;
    ctx->next_index = -1;  /* reset — Start() supersedes any pending gapless */
    const TrackRef &ref = ctx->playlist[index];

    printf("[MusicPlayer] Playing [%d/%d]: %s (%s)\n",
           index + 1, (int)ctx->playlist.size(), ref.filepath,
           ctx->use_soft ? "gapless" : "hw");

    if (ctx->use_soft) {
#ifdef USE_SOFT_PLAYER
        if (ctx->transport) {
            ctx->transport->Start(std::string(ref.filepath));
        }
#endif
    } else {
        std::string path(ref.filepath);
        ctx->player->play(path);
    }
    ctx->state = PLAYER_STATE_PLAYING;

    on_track_changed_fn tcb = ctx->track_cb;
    void *tdata = ctx->track_cb_data;
    on_state_changed_fn scb = ctx->state_cb;
    void *sdata = ctx->state_cb_data;

    lock.unlock();

    /* Fire state callback so player_locked gets cleared immediately */
    if (scb) scb(PLAYER_STATE_PLAYING, sdata);
    if (tcb) tcb(index, NULL, tdata);

    return 0;
}

int music_player_pause(MusicPlayerContext *ctx)
{
    if (!ctx || !ctx->player_ready) return -1;
    std::lock_guard<std::mutex> lock(ctx->mtx);
    if (ctx->state == PLAYER_STATE_PLAYING) {
        if (ctx->use_soft) {
#ifdef USE_SOFT_PLAYER
            if (ctx->transport) ctx->transport->Pause();
#endif
        } else {
            ctx->player->pause();
        }
        ctx->state = PLAYER_STATE_PAUSED;
    }
    return 0;
}

int music_player_resume(MusicPlayerContext *ctx)
{
    if (!ctx || !ctx->player_ready) return -1;
    std::lock_guard<std::mutex> lock(ctx->mtx);
    if (ctx->state == PLAYER_STATE_PAUSED) {
        if (ctx->use_soft) {
#ifdef USE_SOFT_PLAYER
            if (ctx->transport) ctx->transport->Resume();
#endif
        } else {
            ctx->player->resume();
        }
        ctx->state = PLAYER_STATE_PLAYING;
    }
    return 0;
}

int music_player_stop(MusicPlayerContext *ctx)
{
    if (!ctx || !ctx->player_ready) return -1;
    std::lock_guard<std::mutex> lock(ctx->mtx);
    if (ctx->use_soft) {
#ifdef USE_SOFT_PLAYER
        if (ctx->transport) ctx->transport->Stop();
#endif
    } else {
        ctx->player->stop();
    }
    ctx->state = PLAYER_STATE_STOPPED;
    return 0;
}

int music_player_next(MusicPlayerContext *ctx)
{
    if (!ctx || ctx->playlist.empty()) return -1;

    int next;
    {
        std::lock_guard<std::mutex> lock(ctx->mtx);
        next = ctx->current_index;
        int count = (int)ctx->playlist.size();

        switch (ctx->mode) {
        case PLAY_MODE_REPEAT_ONE:
            break;
        case PLAY_MODE_SEQUENTIAL:
            next++;
            if (next >= count) {
                if (ctx->use_soft) {
#ifdef USE_SOFT_PLAYER
                    if (ctx->transport) ctx->transport->Stop();
#endif
                } else if (ctx->player) {
                    ctx->player->stop();
                }
                ctx->state = PLAYER_STATE_STOPPED;
                return 0;
            }
            break;
        case PLAY_MODE_REPEAT_ALL:
            next = (next + 1) % count;
            break;
        case PLAY_MODE_SHUFFLE:
            next = shuffle_pool_pick_next(ctx);
            break;
        }
    }

    return music_player_play(ctx, next);
}

int music_player_prev(MusicPlayerContext *ctx)
{
    if (!ctx || ctx->playlist.empty()) return -1;

    /* Grace period: if playing for > 2 seconds, restart current track
     * instead of going to the previous one (like musikcube/Spotify) */
    if (ctx->use_soft) {
#ifdef USE_SOFT_PLAYER
        if (ctx->transport && ctx->transport->Position() > PREVIOUS_GRACE_PERIOD) {
            return music_player_play(ctx, ctx->current_index);
        }
#endif
    }

    int prev;
    {
        std::lock_guard<std::mutex> lock(ctx->mtx);
        prev = ctx->current_index - 1;
        if (prev < 0) prev = (int)ctx->playlist.size() - 1;
    }

    return music_player_play(ctx, prev);
}

int music_player_seek(MusicPlayerContext *ctx, int position_ms)
{
    if (!ctx || !ctx->player_ready) return -1;
    std::lock_guard<std::mutex> lock(ctx->mtx);
    if (ctx->use_soft) {
#ifdef USE_SOFT_PLAYER
        if (ctx->transport) {
            ctx->transport->SetPosition((double)position_ms / 1000.0);
        }
#endif
    } else {
        ctx->player->seek((double)position_ms);
    }
    return 0;
}

void music_player_set_mode(MusicPlayerContext *ctx, PlayMode mode)
{
    if (!ctx) return;
    std::lock_guard<std::mutex> lock(ctx->mtx);
    ctx->mode = mode;
    if (mode == PLAY_MODE_SHUFFLE)
        generate_shuffle(ctx);
}

PlayMode music_player_get_mode(MusicPlayerContext *ctx)
{
    return ctx ? ctx->mode : PLAY_MODE_SEQUENTIAL;
}

PlayerState music_player_get_state(MusicPlayerContext *ctx)
{
    return ctx ? ctx->state : PLAYER_STATE_IDLE;
}

int music_player_get_position(MusicPlayerContext *ctx)
{
    if (!ctx || !ctx->player_ready) return 0;
    if (ctx->use_soft) {
#ifdef USE_SOFT_PLAYER
        return ctx->transport ? (int)(ctx->transport->Position() * 1000.0) : 0;
#else
        return 0;
#endif
    }
    return (int)ctx->player->getPosition();
}

void music_player_set_state_callback(MusicPlayerContext *ctx,
                                     on_state_changed_fn cb, void *user_data)
{
    if (!ctx) return;
    std::lock_guard<std::mutex> lock(ctx->mtx);
    ctx->state_cb = cb;
    ctx->state_cb_data = user_data;
}

void music_player_set_track_callback(MusicPlayerContext *ctx,
                                     on_track_changed_fn cb, void *user_data)
{
    if (!ctx) return;
    std::lock_guard<std::mutex> lock(ctx->mtx);
    ctx->track_cb = cb;
    ctx->track_cb_data = user_data;
}

void music_player_set_position_callback(MusicPlayerContext *ctx,
                                        on_position_changed_fn cb, void *user_data)
{
    if (!ctx) return;
    std::lock_guard<std::mutex> lock(ctx->mtx);
    ctx->position_cb = cb;
    ctx->position_cb_data = user_data;
}

} /* extern "C" */