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

    /* Shuffle order */
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
        /* Track ended naturally -> auto-advance */
        if (ctx->state == PLAYER_STATE_PLAYING) {
            pthread_mutex_unlock(&ctx->mutex);
            music_player_next(ctx);
            return;
        }
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

/* --- Shuffle helper --- */

static void generate_shuffle(MusicPlayerContext *ctx)
{
    if (!ctx->shuffle_order || ctx->playlist_count <= 0) return;

    /* Fisher-Yates shuffle */
    for (int i = 0; i < ctx->playlist_count; i++)
        ctx->shuffle_order[i] = i;

    srand((unsigned)time(NULL));
    for (int i = ctx->playlist_count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = ctx->shuffle_order[i];
        ctx->shuffle_order[i] = ctx->shuffle_order[j];
        ctx->shuffle_order[j] = tmp;
    }
}

/* Map logical index to actual playlist index based on play mode */
static int get_actual_index(MusicPlayerContext *ctx, int logical_index)
{
    if (ctx->mode == PLAY_MODE_SHUFFLE && ctx->shuffle_order) {
        if (logical_index >= 0 && logical_index < ctx->playlist_count)
            return ctx->shuffle_order[logical_index];
    }
    return logical_index;
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

    memset(ctx, 0, sizeof(MusicPlayerContext));
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

    /* Stop playback */
    if (ctx->player && ctx->state == PLAYER_STATE_PLAYING) {
        ctx->player->stop();
    }

    delete ctx->player;
    free(ctx->playlist);
    free(ctx->shuffle_order);
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

    /* Copy new playlist */
    ctx->playlist_count = list->count;
    ctx->playlist = (MusicInfo *)calloc(list->count, sizeof(MusicInfo));
    ctx->shuffle_order = (int *)calloc(list->count, sizeof(int));

    if (!ctx->playlist || !ctx->shuffle_order) {
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
        if (ctx->current_index >= 0) {
            /* Resume */
            ctx->player->resume();
            ctx->state = PLAYER_STATE_PLAYING;
            pthread_mutex_unlock(&ctx->mutex);
            return 0;
        }
        index = 0;
    }

    if (index < 0 || index >= ctx->playlist_count) {
        pthread_mutex_unlock(&ctx->mutex);
        return -1;
    }

    int actual = get_actual_index(ctx, index);
    if (actual < 0 || actual >= ctx->playlist_count) {
        pthread_mutex_unlock(&ctx->mutex);
        return -1;
    }

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
        next = (next + 1) % ctx->playlist_count;
        if (next == 0) {
            /* Reshuffled at wrap-around */
            generate_shuffle(ctx);
        }
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
