/*
 * music_player.cpp - Music playback controller
 *
 * Uses MediaPlayer class from libatcmediaplayer.so for actual playback.
 * Manages playlist, play modes, track navigation.
 *
 * Note: This file is C++ because MediaPlayer is a C++ class.
 *       The external API is C-compatible (extern "C").
 */

#include "music_player.h"
#include "atcmediaplayer.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <mutex>
#include <pthread.h>
#include <unistd.h>
#include <ctime>

/* --- Internal context structure --- */

struct MusicPlayerContext {
    /* The underlying ATC media player */
    MediaPlayer         *player;
    bool                 player_ready;

    /* Playlist — lightweight refs only (filepath + uid).
     * std::vector manages memory automatically — no calloc/free.
     * 10000 songs × 516 bytes ≈ 5 MB (was 25 MB with MusicInfo). */
    std::vector<TrackRef>  playlist;
    int                    current_index;

    /* Issue #19: Shuffle consumption pool (mirrors Android mRandomPositionList).
     * Pool contains indices not yet played. Each next() picks randomly from
     * the pool and removes it, guaranteeing every track plays exactly once. */
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

    std::mutex           mtx;  /* replaces pthread_mutex_t */
};

/* --- State callback from MediaPlayer --- */

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

/* --- Shuffle helpers (Issue #19: consumption pool with std::vector) --- */

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

    /* Swap with last and pop — O(1) removal */
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
        usleep(500000); /* 500ms polling interval */

        on_position_changed_fn cb = nullptr;
        void *data = nullptr;
        int pos = 0;

        {
            std::lock_guard<std::mutex> lock(ctx->mtx);
            if (ctx->state == PLAYER_STATE_PLAYING && ctx->player && ctx->position_cb) {
                pos = (int)ctx->player->getPosition();
                cb = ctx->position_cb;
                data = ctx->position_cb_data;
            }
        }

        if (cb) cb(pos, -1, data);
    }
    return NULL;
}

/* --- Public API implementation --- */

extern "C" {

MusicPlayerContext *music_player_create(void)
{
    MusicPlayerContext *ctx = new (std::nothrow) MusicPlayerContext();
    if (!ctx) return NULL;

    /* vectors are default-constructed (empty). Init POD fields only. */
    ctx->player = NULL;
    ctx->player_ready = false;
    ctx->current_index = -1;
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

    ctx->player = new (std::nothrow) MediaPlayer();
    if (!ctx->player) {
        delete ctx;
        return NULL;
    }

    if (!ctx->player->setup()) {
        fprintf(stderr, "[MusicPlayer] MediaPlayer setup failed\n");
        delete ctx->player;
        delete ctx;
        return NULL;
    }

    ctx->player->setStateCallback(media_state_callback_wrapper, ctx);
    ctx->player_ready = true;

    /* Start position polling thread */
    ctx->poll_running = true;
    pthread_create(&ctx->poll_thread, NULL, position_poll_func, ctx);

    printf("[MusicPlayer] Created successfully\n");
    return ctx;
}

void music_player_destroy(MusicPlayerContext *ctx)
{
    if (!ctx) return;

    /* Stop polling */
    ctx->poll_running = false;
    pthread_join(ctx->poll_thread, NULL);

    /* Stop playback */
    if (ctx->player && (ctx->state == PLAYER_STATE_PLAYING ||
                        ctx->state == PLAYER_STATE_PAUSED)) {
        ctx->player->stop();
    }

    delete ctx->player;
    /* vectors and std::mutex are cleaned up by ~MusicPlayerContext() */
    delete ctx;

    printf("[MusicPlayer] Destroyed\n");
}

int music_player_set_playlist_refs(MusicPlayerContext *ctx,
                                   const TrackRef *refs, int count)
{
    if (!ctx || !refs || count <= 0) return -1;

    std::lock_guard<std::mutex> lock(ctx->mtx);

    /* Stop current playback */
    if (ctx->state == PLAYER_STATE_PLAYING && ctx->player) {
        ctx->player->stop();
    }

    /* Replace playlist — vector handles alloc/dealloc */
    ctx->playlist.assign(refs, refs + count);
    ctx->current_index = -1;
    ctx->state = PLAYER_STATE_IDLE;

    generate_shuffle(ctx);

    printf("[MusicPlayer] Playlist set: %d tracks, %.1f KB\n",
           count, (double)ctx->playlist.size() * sizeof(TrackRef) / 1024.0);
    return 0;
}

int music_player_set_playlist(MusicPlayerContext *ctx, const MusicList *list)
{
    if (!ctx || !list || list->count <= 0) return -1;

    /* Convert MusicList → vector<TrackRef>, then delegate.
     * For sub-playlists (folder/album/group) where count is small. */
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
    /* DEPRECATED: playlist now stores TrackRef, not MusicInfo.
     * Always returns NULL. Use music_app_get_track_info() which queries
     * SQLite on-demand, or music_player_get_track_ref() for filepath. */
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
            ctx->player->resume();
            ctx->state = PLAYER_STATE_PLAYING;

            PlayerState s = ctx->state;
            on_state_changed_fn cb = ctx->state_cb;
            void *data = ctx->state_cb_data;
            lock.unlock();
            if (cb) cb(s, data);
            return 0;
        }
        index = (ctx->current_index >= 0) ? ctx->current_index : 0;
    }

    if (index < 0 || index >= (int)ctx->playlist.size()) {
        return -1;
    }

    int actual = index;

    if (ctx->state == PLAYER_STATE_PLAYING) {
        ctx->player->stop();
    }

    ctx->current_index = index;
    const TrackRef &ref = ctx->playlist[actual];

    printf("[MusicPlayer] Playing [%d/%d]: %s\n",
           index + 1, (int)ctx->playlist.size(), ref.filepath);

    std::string path(ref.filepath);
    ctx->player->play(path);
    ctx->state = PLAYER_STATE_PLAYING;

    on_track_changed_fn cb = ctx->track_cb;
    void *data = ctx->track_cb_data;

    lock.unlock();

    if (cb) cb(actual, NULL, data);

    return 0;
}

int music_player_pause(MusicPlayerContext *ctx)
{
    if (!ctx || !ctx->player) return -1;
    std::lock_guard<std::mutex> lock(ctx->mtx);
    if (ctx->state == PLAYER_STATE_PLAYING) {
        ctx->player->pause();
        ctx->state = PLAYER_STATE_PAUSED;
    }
    return 0;
}

int music_player_resume(MusicPlayerContext *ctx)
{
    if (!ctx || !ctx->player) return -1;
    std::lock_guard<std::mutex> lock(ctx->mtx);
    if (ctx->state == PLAYER_STATE_PAUSED) {
        ctx->player->resume();
        ctx->state = PLAYER_STATE_PLAYING;
    }
    return 0;
}

int music_player_stop(MusicPlayerContext *ctx)
{
    if (!ctx || !ctx->player) return -1;
    std::lock_guard<std::mutex> lock(ctx->mtx);
    ctx->player->stop();
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
                ctx->player->stop();
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
    if (!ctx || !ctx->player) return -1;
    std::lock_guard<std::mutex> lock(ctx->mtx);
    ctx->player->seek((double)position_ms);
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
    if (!ctx || !ctx->player) return 0;
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
