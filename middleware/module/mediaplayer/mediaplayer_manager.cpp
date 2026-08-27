/**
 * @file mediaplayer_manager.cpp
 * @brief MediaPlayer C++ 业务逻辑实现
 *
 * 这是整个中间件中唯一 #include "atcmediaplayer.h" 的文件。
 * 厂商的 MediaPlayer C++ 类被封锁在本文件内部，不穿透到任何头文件。
 *
 * 从 music_player.cpp 迁移而来，逻辑完全不变，只是：
 *   1. struct 名从 MusicPlayerContext 改为 mp_context (mediaplayer_manager.h 定义)
 *   2. enum 名从 PLAYER_STATE_*/PLAY_MODE_* 改为 MP_STATE_*/MP_MODE_*
 *   3. 不再自带 extern "C" 导出 — 由 mediaplayer_cxx.cpp 统一导出
 *
 * 对标: Android MediaPlayerModel.java + MusicPlaylistEx.java
 */

#include "mediaplayer_manager.h"
#include "atcmediaplayer.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <string>
#include <ctime>
#include <unistd.h>

/* ======================================================================
 * State callback from vendor MediaPlayer
 * ====================================================================== */

static void media_state_callback(int new_state, void *user_data)
{
    mp_context *ctx = static_cast<mp_context *>(user_data);
    if (!ctx) return;

    pthread_mutex_lock(&ctx->mutex);

    switch (new_state) {
    case MediaPlayer::PlayingState:
        ctx->state = MP_STATE_PLAYING;
        break;
    case MediaPlayer::PausedState:
        ctx->state = MP_STATE_PAUSED;
        break;
    case MediaPlayer::StoppedState:
        ctx->state = MP_STATE_STOPPED;
        break;
    case MediaPlayer::ErrorState:
        ctx->state = MP_STATE_ERROR;
        break;
    }

    mp_player_state_t s = ctx->state;
    mp_on_state_changed_fn cb = ctx->state_cb;
    void *data = ctx->state_cb_data;

    pthread_mutex_unlock(&ctx->mutex);

    if (cb) cb(s, data);
}

/* ======================================================================
 * Shuffle helpers (mirrors Android MusicPlaylistEx)
 * ====================================================================== */

void mgr_rebuild_shuffle_pool(mp_context *ctx)
{
    if (!ctx->shuffle_pool || ctx->playlist_count <= 0) return;
    for (int i = 0; i < ctx->playlist_count; i++)
        ctx->shuffle_pool[i] = i;
    ctx->shuffle_pool_count = ctx->playlist_count;
}

void mgr_shuffle_pool_remove(mp_context *ctx, int track_index)
{
    for (int i = 0; i < ctx->shuffle_pool_count; i++) {
        if (ctx->shuffle_pool[i] == track_index) {
            ctx->shuffle_pool[i] = ctx->shuffle_pool[ctx->shuffle_pool_count - 1];
            ctx->shuffle_pool_count--;
            return;
        }
    }
}

int mgr_shuffle_pool_pick_next(mp_context *ctx)
{
    if (ctx->shuffle_pool_count <= 0)
        mgr_rebuild_shuffle_pool(ctx);
    if (ctx->shuffle_pool_count <= 0) return 0;

    int pick = rand() % ctx->shuffle_pool_count;
    int result = ctx->shuffle_pool[pick];
    ctx->shuffle_pool[pick] = ctx->shuffle_pool[ctx->shuffle_pool_count - 1];
    ctx->shuffle_pool_count--;
    return result;
}

void mgr_generate_shuffle(mp_context *ctx)
{
    srand(static_cast<unsigned>(time(NULL)));
    mgr_rebuild_shuffle_pool(ctx);
}

/* ======================================================================
 * Position polling thread
 * ====================================================================== */

static void *position_poll_func(void *arg)
{
    mp_context *ctx = static_cast<mp_context *>(arg);

    while (ctx->poll_running) {
        usleep(500000);

        pthread_mutex_lock(&ctx->mutex);
        if (ctx->state == MP_STATE_PLAYING && ctx->player && ctx->position_cb) {
            double pos = ctx->player->getPosition();
            mp_on_position_changed_fn cb = ctx->position_cb;
            void *data = ctx->position_cb_data;
            pthread_mutex_unlock(&ctx->mutex);
            cb(static_cast<int>(pos), -1, data);
        } else {
            pthread_mutex_unlock(&ctx->mutex);
        }
    }
    return NULL;
}

/* ======================================================================
 * Lifecycle
 * ====================================================================== */

mp_context *mgr_create(void)
{
    mp_context *ctx = new (std::nothrow) mp_context();
    if (!ctx) return NULL;

    ctx->player = NULL;
    ctx->player_ready = false;
    ctx->playlist = NULL;
    ctx->playlist_count = 0;
    ctx->current_index = -1;
    ctx->shuffle_order = NULL;
    ctx->shuffle_pool = NULL;
    ctx->shuffle_pool_count = 0;
    ctx->state = MP_STATE_IDLE;
    ctx->mode = MP_MODE_SEQUENTIAL;
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
        std::fprintf(stderr, "[MediaPlayer] setup failed\n");
        delete ctx->player;
        delete ctx;
        return NULL;
    }

    ctx->player->setStateCallback(media_state_callback, ctx);
    ctx->player_ready = true;

    ctx->poll_running = true;
    pthread_create(&ctx->poll_thread, NULL, position_poll_func, ctx);

    std::printf("[MediaPlayer] Created successfully\n");
    return ctx;
}

void mgr_destroy(mp_context *ctx)
{
    if (!ctx) return;

    ctx->poll_running = false;
    pthread_join(ctx->poll_thread, NULL);

    if (ctx->player && (ctx->state == MP_STATE_PLAYING ||
                        ctx->state == MP_STATE_PAUSED)) {
        ctx->player->stop();
    }

    delete ctx->player;
    std::free(ctx->playlist);
    std::free(ctx->shuffle_order);
    std::free(ctx->shuffle_pool);
    pthread_mutex_destroy(&ctx->mutex);
    delete ctx;

    std::printf("[MediaPlayer] Destroyed\n");
}

/* ======================================================================
 * Playlist
 * ====================================================================== */

int mgr_set_playlist(mp_context *ctx, const MusicList *list)
{
    if (!ctx || !list) return -1;

    pthread_mutex_lock(&ctx->mutex);

    if (ctx->state == MP_STATE_PLAYING && ctx->player)
        ctx->player->stop();

    std::free(ctx->playlist);
    std::free(ctx->shuffle_order);
    std::free(ctx->shuffle_pool);

    ctx->playlist_count = list->count;
    ctx->playlist = static_cast<MusicInfo *>(std::calloc(list->count, sizeof(MusicInfo)));
    ctx->shuffle_order = static_cast<int *>(std::calloc(list->count, sizeof(int)));
    ctx->shuffle_pool = static_cast<int *>(std::calloc(list->count, sizeof(int)));
    ctx->shuffle_pool_count = 0;

    if (!ctx->playlist || !ctx->shuffle_order || !ctx->shuffle_pool) {
        ctx->playlist_count = 0;
        pthread_mutex_unlock(&ctx->mutex);
        return -1;
    }

    std::memcpy(ctx->playlist, list->items, list->count * sizeof(MusicInfo));
    ctx->current_index = -1;
    ctx->state = MP_STATE_IDLE;

    mgr_generate_shuffle(ctx);

    pthread_mutex_unlock(&ctx->mutex);

    std::printf("[MediaPlayer] Playlist set: %d tracks\n", list->count);
    return 0;
}

/* ======================================================================
 * Playback control
 * ====================================================================== */

int mgr_play(mp_context *ctx, int index)
{
    if (!ctx || !ctx->player_ready) return -1;

    pthread_mutex_lock(&ctx->mutex);

    if (index == -1) {
        if (ctx->current_index >= 0 && ctx->state == MP_STATE_PAUSED) {
            ctx->player->resume();
            ctx->state = MP_STATE_PLAYING;

            mp_player_state_t s = ctx->state;
            mp_on_state_changed_fn cb = ctx->state_cb;
            void *data = ctx->state_cb_data;
            pthread_mutex_unlock(&ctx->mutex);
            if (cb) cb(s, data);
            return 0;
        }
        index = (ctx->current_index >= 0) ? ctx->current_index : 0;
    }

    if (index < 0 || index >= ctx->playlist_count) {
        pthread_mutex_unlock(&ctx->mutex);
        return -1;
    }

    if (ctx->state == MP_STATE_PLAYING)
        ctx->player->stop();

    ctx->current_index = index;
    const MusicInfo *info = &ctx->playlist[index];

    std::printf("[MediaPlayer] Playing [%d/%d]: %s - %s\n",
                index + 1, ctx->playlist_count, info->artist, info->title);

    std::string path(info->filepath);
    ctx->player->play(path);
    ctx->state = MP_STATE_PLAYING;

    mp_on_track_changed_fn cb = ctx->track_cb;
    void *data = ctx->track_cb_data;

    pthread_mutex_unlock(&ctx->mutex);

    if (cb) cb(index, info, data);
    return 0;
}

int mgr_pause(mp_context *ctx)
{
    if (!ctx || !ctx->player) return -1;
    pthread_mutex_lock(&ctx->mutex);
    if (ctx->state == MP_STATE_PLAYING) {
        ctx->player->pause();
        ctx->state = MP_STATE_PAUSED;
    }
    pthread_mutex_unlock(&ctx->mutex);
    return 0;
}

int mgr_resume(mp_context *ctx)
{
    if (!ctx || !ctx->player) return -1;
    pthread_mutex_lock(&ctx->mutex);
    if (ctx->state == MP_STATE_PAUSED) {
        ctx->player->resume();
        ctx->state = MP_STATE_PLAYING;
    }
    pthread_mutex_unlock(&ctx->mutex);
    return 0;
}

int mgr_stop(mp_context *ctx)
{
    if (!ctx || !ctx->player) return -1;
    pthread_mutex_lock(&ctx->mutex);
    ctx->player->stop();
    ctx->state = MP_STATE_STOPPED;
    pthread_mutex_unlock(&ctx->mutex);
    return 0;
}

int mgr_next(mp_context *ctx)
{
    if (!ctx || ctx->playlist_count <= 0) return -1;

    pthread_mutex_lock(&ctx->mutex);
    int next = ctx->current_index;

    switch (ctx->mode) {
    case MP_MODE_REPEAT_ONE:
        break;
    case MP_MODE_SEQUENTIAL:
        next++;
        if (next >= ctx->playlist_count) {
            ctx->player->stop();
            ctx->state = MP_STATE_STOPPED;
            pthread_mutex_unlock(&ctx->mutex);
            return 0;
        }
        break;
    case MP_MODE_REPEAT_ALL:
        next = (next + 1) % ctx->playlist_count;
        break;
    case MP_MODE_SHUFFLE:
        next = mgr_shuffle_pool_pick_next(ctx);
        break;
    }

    pthread_mutex_unlock(&ctx->mutex);
    return mgr_play(ctx, next);
}

int mgr_prev(mp_context *ctx)
{
    if (!ctx || ctx->playlist_count <= 0) return -1;
    pthread_mutex_lock(&ctx->mutex);
    int prev = ctx->current_index - 1;
    if (prev < 0) prev = ctx->playlist_count - 1;
    pthread_mutex_unlock(&ctx->mutex);
    return mgr_play(ctx, prev);
}

int mgr_seek(mp_context *ctx, int position_ms)
{
    if (!ctx || !ctx->player) return -1;
    pthread_mutex_lock(&ctx->mutex);
    ctx->player->seek(static_cast<double>(position_ms));
    pthread_mutex_unlock(&ctx->mutex);
    return 0;
}

/* ======================================================================
 * Mode / State / Position query
 * ====================================================================== */

void mgr_set_mode(mp_context *ctx, mp_play_mode_t mode)
{
    if (!ctx) return;
    pthread_mutex_lock(&ctx->mutex);
    ctx->mode = mode;
    if (mode == MP_MODE_SHUFFLE)
        mgr_generate_shuffle(ctx);
    pthread_mutex_unlock(&ctx->mutex);
}

int mgr_get_position(mp_context *ctx)
{
    if (!ctx || !ctx->player) return 0;
    return static_cast<int>(ctx->player->getPosition());
}
