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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

/* --- Internal context structure --- */

struct MusicPlayerContext {
    /* The underlying ATC media player */
    MediaPlayer         *player;
    bool                 player_ready;

    /* Playlist (copied from MusicList) */
    MusicInfo           *playlist;
    int                  playlist_count;
    int                  current_index;

    /* Issue #19: Shuffle consumption pool (mirrors Android mRandomPositionList).
     * Pool contains indices not yet played. Each next() picks randomly from
     * the pool and removes it, guaranteeing every track plays exactly once. */
    int                 *shuffle_pool;
    int                  shuffle_pool_count;

    /* Legacy shuffle_order kept for compatibility (used by set_mode) */
    int                 *shuffle_order;

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

    pthread_mutex_t      mutex;
};

/* --- State callback from MediaPlayer --- */

static void media_state_callback(int new_state, void *user_data)
{
    MusicPlayerContext *ctx = (MusicPlayerContext *)user_data;
    if (!ctx) return;

    pthread_mutex_lock(&ctx->mutex);

    switch (new_state) {
    case MediaPlayer::PlayingState:
        ctx->state = PLAYER_STATE_PLAYING;
        break;
    case MediaPlayer::PausedState:
        ctx->state = PLAYER_STATE_PAUSED;
        break;
    case MediaPlayer::StoppedState:
        /* Issue #26 fix: Do NOT call music_player_next() here.
         * Auto-next is now handled exclusively by music_app.c's
         * on_player_state() callback, which posts a delayed timer on the
         * AWTK main thread. Calling next() here caused a "double-jump" bug:
         * once from this callback, once from the delayed timer.
         *
         * We just set the state and let the callback chain handle it:
         *   MediaPlayer → media_state_callback → on_player_state (user cb)
         *   → idle_queue → player_state_idle_handler → timer_add(1000ms)
         *   → delayed_auto_next_cb → music_player_next()
         */
        ctx->state = PLAYER_STATE_STOPPED;
        break;
    case MediaPlayer::ErrorState:
        ctx->state = PLAYER_STATE_ERROR;
        break;
    }

    PlayerState s = ctx->state;
    on_state_changed_fn cb = ctx->state_cb;
    void *data = ctx->state_cb_data;

    pthread_mutex_unlock(&ctx->mutex);

    if (cb) cb(s, data);
}

/* --- Shuffle helper (Issue #19: consumption pool) --- */

/**
 * Rebuild the shuffle consumption pool with all track indices.
 * Mirrors Android MusicPlaylistEx.updateRandomPositionList().
 */
static void rebuild_shuffle_pool(MusicPlayerContext *ctx)
{
    if (!ctx->shuffle_pool || ctx->playlist_count <= 0) return;

    for (int i = 0; i < ctx->playlist_count; i++)
        ctx->shuffle_pool[i] = i;
    ctx->shuffle_pool_count = ctx->playlist_count;
}

/**
 * Remove a specific index from the shuffle pool.
 * Mirrors Android MusicPlaylistEx.removeFromRandomPositionList().
 */
static void shuffle_pool_remove(MusicPlayerContext *ctx, int track_index)
{
    for (int i = 0; i < ctx->shuffle_pool_count; i++) {
        if (ctx->shuffle_pool[i] == track_index) {
            /* Swap with last element and shrink */
            ctx->shuffle_pool[i] = ctx->shuffle_pool[ctx->shuffle_pool_count - 1];
            ctx->shuffle_pool_count--;
            return;
        }
    }
}

/**
 * Pick a random index from the shuffle pool without replacement.
 * Mirrors Android MusicPlaylistEx.getNextRandomPosition().
 * If pool is empty, rebuilds it (new cycle — all tracks played once).
 */
static int shuffle_pool_pick_next(MusicPlayerContext *ctx)
{
    if (ctx->shuffle_pool_count <= 0) {
        rebuild_shuffle_pool(ctx);
    }

    if (ctx->shuffle_pool_count <= 0) return 0;

    int pick = rand() % ctx->shuffle_pool_count;
    int result = ctx->shuffle_pool[pick];

    /* Remove picked item (swap with last) */
    ctx->shuffle_pool[pick] = ctx->shuffle_pool[ctx->shuffle_pool_count - 1];
    ctx->shuffle_pool_count--;

    return result;
}

/* Legacy generate_shuffle — kept for set_mode compatibility, rebuilds pool */
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

        pthread_mutex_lock(&ctx->mutex);
        if (ctx->state == PLAYER_STATE_PLAYING && ctx->player && ctx->position_cb) {
            double pos = ctx->player->getPosition();
            on_position_changed_fn cb = ctx->position_cb;
            void *data = ctx->position_cb_data;
            pthread_mutex_unlock(&ctx->mutex);

            /* duration not easily available from MediaPlayer API,
             * pass -1 for now; UI can use MusicInfo.duration_ms */
            cb((int)pos, -1, data);
        } else {
            pthread_mutex_unlock(&ctx->mutex);
        }
    }
    return NULL;
}

/* --- Public API implementation --- */

extern "C" {

MusicPlayerContext *music_player_create(void)
{
    MusicPlayerContext *ctx = new (std::nothrow) MusicPlayerContext();
    if (!ctx) return NULL;

    /* Zero-initialize all POD members safely. The struct is POD-like,
     * but using memset after new is technically UB in C++ if there are
     * non-trivial members. We explicitly init each field instead. */
    ctx->player = NULL;
    ctx->player_ready = false;
    ctx->playlist = NULL;
    ctx->playlist_count = 0;
    ctx->current_index = -1;
    ctx->shuffle_order = NULL;
    ctx->shuffle_pool = NULL;
    ctx->shuffle_pool_count = 0;
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

    pthread_mutex_init(&ctx->mutex, NULL);

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

    ctx->player->setStateCallback(media_state_callback, ctx);
    ctx->player_ready = true;
    ctx->current_index = -1;
    ctx->state = PLAYER_STATE_IDLE;
    ctx->mode = PLAY_MODE_SEQUENTIAL;

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

    /* Stop playback regardless of state */
    if (ctx->player && (ctx->state == PLAYER_STATE_PLAYING ||
                        ctx->state == PLAYER_STATE_PAUSED)) {
        ctx->player->stop();
    }

    delete ctx->player;
    free(ctx->playlist);
    free(ctx->shuffle_order);
    free(ctx->shuffle_pool);
    pthread_mutex_destroy(&ctx->mutex);
    delete ctx;

    printf("[MusicPlayer] Destroyed\n");
}

int music_player_set_playlist(MusicPlayerContext *ctx, const MusicList *list)
{
    if (!ctx || !list) return -1;

    pthread_mutex_lock(&ctx->mutex);

    /* Stop current playback */
    if (ctx->state == PLAYER_STATE_PLAYING && ctx->player) {
        ctx->player->stop();
    }

    /* Free old playlist */
    free(ctx->playlist);
    free(ctx->shuffle_order);
    free(ctx->shuffle_pool);

    /* Copy new playlist */
    ctx->playlist_count = list->count;
    ctx->playlist = (MusicInfo *)calloc(list->count, sizeof(MusicInfo));
    ctx->shuffle_order = (int *)calloc(list->count, sizeof(int));
    ctx->shuffle_pool = (int *)calloc(list->count, sizeof(int));
    ctx->shuffle_pool_count = 0;

    if (!ctx->playlist || !ctx->shuffle_order || !ctx->shuffle_pool) {
        ctx->playlist_count = 0;
        pthread_mutex_unlock(&ctx->mutex);
        return -1;
    }

    memcpy(ctx->playlist, list->items, list->count * sizeof(MusicInfo));
    ctx->current_index = -1;
    ctx->state = PLAYER_STATE_IDLE;

    generate_shuffle(ctx);

    pthread_mutex_unlock(&ctx->mutex);

    printf("[MusicPlayer] Playlist set: %d tracks\n", list->count);
    return 0;
}

int music_player_get_playlist_count(MusicPlayerContext *ctx)
{
    return ctx ? ctx->playlist_count : 0;
}

const MusicInfo *music_player_get_track_info(MusicPlayerContext *ctx, int index)
{
    if (!ctx || index < 0 || index >= ctx->playlist_count) return NULL;
    return &ctx->playlist[index];
}

int music_player_get_current_index(MusicPlayerContext *ctx)
{
    return ctx ? ctx->current_index : -1;
}

int music_player_play(MusicPlayerContext *ctx, int index)
{
    if (!ctx || !ctx->player_ready) return -1;

    pthread_mutex_lock(&ctx->mutex);

    if (index == -1) {
        if (ctx->current_index >= 0 && ctx->state == PLAYER_STATE_PAUSED) {
            /* Resume from pause */
            ctx->player->resume();
            ctx->state = PLAYER_STATE_PLAYING;

            PlayerState s = ctx->state;
            on_state_changed_fn cb = ctx->state_cb;
            void *data = ctx->state_cb_data;
            pthread_mutex_unlock(&ctx->mutex);
            if (cb) cb(s, data);
            return 0;
        }
        /* If stopped or idle, (re)start from current or track 0 */
        index = (ctx->current_index >= 0) ? ctx->current_index : 0;
    }

    if (index < 0 || index >= ctx->playlist_count) {
        pthread_mutex_unlock(&ctx->mutex);
        return -1;
    }

    /* Issue #18 fix: Use the index directly — do NOT apply shuffle mapping.
     * In Android (MusicPlaylistEx), shuffle only affects adjustPlayPosition()
     * (auto-next/prev), NOT user-initiated play. When a user clicks a song
     * in the list, they expect THAT song to play, not a random one.
     *
     * The shuffle_order is only consumed by music_player_next/prev. */
    int actual = index;

    /* Stop current if playing */
    if (ctx->state == PLAYER_STATE_PLAYING) {
        ctx->player->stop();
    }

    ctx->current_index = index;
    const MusicInfo *info = &ctx->playlist[actual];

    printf("[MusicPlayer] Playing [%d/%d]: %s - %s\n",
           index + 1, ctx->playlist_count, info->artist, info->title);

    /* Use MediaPlayer to play the file */
    std::string path(info->filepath);
    ctx->player->play(path);
    ctx->state = PLAYER_STATE_PLAYING;

    /* Notify track change */
    on_track_changed_fn cb = ctx->track_cb;
    void *data = ctx->track_cb_data;

    pthread_mutex_unlock(&ctx->mutex);

    if (cb) cb(actual, info, data);

    return 0;
}

int music_player_pause(MusicPlayerContext *ctx)
{
    if (!ctx || !ctx->player) return -1;

    pthread_mutex_lock(&ctx->mutex);
    if (ctx->state == PLAYER_STATE_PLAYING) {
        ctx->player->pause();
        ctx->state = PLAYER_STATE_PAUSED;
    }
    pthread_mutex_unlock(&ctx->mutex);
    return 0;
}

int music_player_resume(MusicPlayerContext *ctx)
{
    if (!ctx || !ctx->player) return -1;

    pthread_mutex_lock(&ctx->mutex);
    if (ctx->state == PLAYER_STATE_PAUSED) {
        ctx->player->resume();
        ctx->state = PLAYER_STATE_PLAYING;
    }
    pthread_mutex_unlock(&ctx->mutex);
    return 0;
}

int music_player_stop(MusicPlayerContext *ctx)
{
    if (!ctx || !ctx->player) return -1;

    pthread_mutex_lock(&ctx->mutex);
    ctx->player->stop();
    ctx->state = PLAYER_STATE_STOPPED;
    pthread_mutex_unlock(&ctx->mutex);
    return 0;
}

int music_player_next(MusicPlayerContext *ctx)
{
    if (!ctx || ctx->playlist_count <= 0) return -1;

    pthread_mutex_lock(&ctx->mutex);

    int next = ctx->current_index;

    switch (ctx->mode) {
    case PLAY_MODE_REPEAT_ONE:
        /* Stay on same track */
        break;
    case PLAY_MODE_SEQUENTIAL:
        next++;
        if (next >= ctx->playlist_count) {
            /* End of list */
            ctx->player->stop();
            ctx->state = PLAYER_STATE_STOPPED;
            pthread_mutex_unlock(&ctx->mutex);
            return 0;
        }
        break;
    case PLAY_MODE_REPEAT_ALL:
        next = (next + 1) % ctx->playlist_count;
        break;
    case PLAY_MODE_SHUFFLE:
        /* Issue #19 fix: Use consumption pool (Android mRandomPositionList).
         * Pick a random unplayed track. Pool auto-refills when exhausted. */
        next = shuffle_pool_pick_next(ctx);
        break;
    }

    pthread_mutex_unlock(&ctx->mutex);
    return music_player_play(ctx, next);
}

int music_player_prev(MusicPlayerContext *ctx)
{
    if (!ctx || ctx->playlist_count <= 0) return -1;

    pthread_mutex_lock(&ctx->mutex);
    int prev = ctx->current_index - 1;
    if (prev < 0) prev = ctx->playlist_count - 1;
    pthread_mutex_unlock(&ctx->mutex);

    return music_player_play(ctx, prev);
}

int music_player_seek(MusicPlayerContext *ctx, int position_ms)
{
    if (!ctx || !ctx->player) return -1;

    pthread_mutex_lock(&ctx->mutex);
    ctx->player->seek((double)position_ms);
    pthread_mutex_unlock(&ctx->mutex);
    return 0;
}

void music_player_set_mode(MusicPlayerContext *ctx, PlayMode mode)
{
    if (!ctx) return;
    pthread_mutex_lock(&ctx->mutex);
    ctx->mode = mode;
    if (mode == PLAY_MODE_SHUFFLE)
        generate_shuffle(ctx);
    pthread_mutex_unlock(&ctx->mutex);
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
    pthread_mutex_lock(&ctx->mutex);
    ctx->state_cb = cb;
    ctx->state_cb_data = user_data;
    pthread_mutex_unlock(&ctx->mutex);
}

void music_player_set_track_callback(MusicPlayerContext *ctx,
                                     on_track_changed_fn cb, void *user_data)
{
    if (!ctx) return;
    pthread_mutex_lock(&ctx->mutex);
    ctx->track_cb = cb;
    ctx->track_cb_data = user_data;
    pthread_mutex_unlock(&ctx->mutex);
}

void music_player_set_position_callback(MusicPlayerContext *ctx,
                                        on_position_changed_fn cb, void *user_data)
{
    if (!ctx) return;
    pthread_mutex_lock(&ctx->mutex);
    ctx->position_cb = cb;
    ctx->position_cb_data = user_data;
    pthread_mutex_unlock(&ctx->mutex);
}

} /* extern "C" */
