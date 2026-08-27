/**
 * @file mediaplayer_cxx.cpp
 * @brief C ABI wrapper — 所有 extern "C" 入口 try/catch 全兜底
 *
 * 边界纪律（照搬 bt_cxx.cpp）：
 *   1. 异常绝不穿越 C 边界
 *   2. 返回值只使用 mediaplayer_cxx.h 定义的 int 错误码
 *   3. wrapper 不写业务逻辑 — 全部下沉到 mediaplayer_manager
 */

#include "mediaplayer_cxx.h"
#include "mediaplayer_manager.h"

#include <cstdio>

/* Forward declarations of mgr_* functions (defined in mediaplayer_manager.cpp) */
extern mp_context *mgr_create(void);
extern void        mgr_destroy(mp_context *ctx);
extern int         mgr_set_playlist(mp_context *ctx, const MusicList *list);
extern int         mgr_play(mp_context *ctx, int index);
extern int         mgr_pause(mp_context *ctx);
extern int         mgr_resume(mp_context *ctx);
extern int         mgr_stop(mp_context *ctx);
extern int         mgr_next(mp_context *ctx);
extern int         mgr_prev(mp_context *ctx);
extern int         mgr_seek(mp_context *ctx, int position_ms);
extern void        mgr_set_mode(mp_context *ctx, mp_play_mode_t mode);
extern int         mgr_get_position(mp_context *ctx);

/* ---- Link probe ---- */
extern "C" int mp_cxx_probe(void) {
    try {
        return 1;
    } catch (...) {
        return MP_CXX_ERR;
    }
}

/* ---- Lifecycle ---- */
extern "C" mp_context_t *mp_create(void) {
    try {
        return mgr_create();
    } catch (...) {
        std::fprintf(stderr, "[mp_cxx] mp_create: C++ exception caught\n");
        return NULL;
    }
}

extern "C" void mp_destroy(mp_context_t *ctx) {
    try {
        mgr_destroy(ctx);
    } catch (...) {
        std::fprintf(stderr, "[mp_cxx] mp_destroy: C++ exception caught\n");
    }
}

/* ---- Playlist ---- */
extern "C" int mp_set_playlist(mp_context_t *ctx, const MusicList *list) {
    try {
        if (!ctx || !list) return MP_CXX_ERR_ARGS;
        return mgr_set_playlist(ctx, list);
    } catch (...) {
        return MP_CXX_ERR;
    }
}

extern "C" int mp_get_playlist_count(mp_context_t *ctx) {
    try {
        return ctx ? ctx->playlist_count : 0;
    } catch (...) {
        return MP_CXX_ERR;
    }
}

extern "C" const MusicInfo *mp_get_track_info(mp_context_t *ctx, int index) {
    try {
        if (!ctx || index < 0 || index >= ctx->playlist_count) return NULL;
        return &ctx->playlist[index];
    } catch (...) {
        return NULL;
    }
}

extern "C" int mp_get_current_index(mp_context_t *ctx) {
    try {
        return ctx ? ctx->current_index : -1;
    } catch (...) {
        return -1;
    }
}

/* ---- Playback control ---- */
extern "C" int mp_play(mp_context_t *ctx, int index) {
    try {
        if (!ctx) return MP_CXX_ERR_ARGS;
        return mgr_play(ctx, index);
    } catch (...) {
        return MP_CXX_ERR;
    }
}

extern "C" int mp_pause(mp_context_t *ctx) {
    try {
        return mgr_pause(ctx);
    } catch (...) {
        return MP_CXX_ERR;
    }
}

extern "C" int mp_resume(mp_context_t *ctx) {
    try {
        return mgr_resume(ctx);
    } catch (...) {
        return MP_CXX_ERR;
    }
}

extern "C" int mp_stop(mp_context_t *ctx) {
    try {
        return mgr_stop(ctx);
    } catch (...) {
        return MP_CXX_ERR;
    }
}

extern "C" int mp_next(mp_context_t *ctx) {
    try {
        return mgr_next(ctx);
    } catch (...) {
        return MP_CXX_ERR;
    }
}

extern "C" int mp_prev(mp_context_t *ctx) {
    try {
        return mgr_prev(ctx);
    } catch (...) {
        return MP_CXX_ERR;
    }
}

extern "C" int mp_seek(mp_context_t *ctx, int position_ms) {
    try {
        if (!ctx) return MP_CXX_ERR_ARGS;
        return mgr_seek(ctx, position_ms);
    } catch (...) {
        return MP_CXX_ERR;
    }
}

/* ---- Play mode ---- */
extern "C" void mp_set_mode(mp_context_t *ctx, mp_play_mode_t mode) {
    try {
        mgr_set_mode(ctx, mode);
    } catch (...) {
        /* void return — swallow */
    }
}

extern "C" mp_play_mode_t mp_get_mode(mp_context_t *ctx) {
    try {
        return ctx ? ctx->mode : MP_MODE_SEQUENTIAL;
    } catch (...) {
        return MP_MODE_SEQUENTIAL;
    }
}

/* ---- State query ---- */
extern "C" mp_player_state_t mp_get_state(mp_context_t *ctx) {
    try {
        return ctx ? ctx->state : MP_STATE_IDLE;
    } catch (...) {
        return MP_STATE_IDLE;
    }
}

extern "C" int mp_get_position(mp_context_t *ctx) {
    try {
        return mgr_get_position(ctx);
    } catch (...) {
        return 0;
    }
}

/* ---- Callbacks ---- */
extern "C" void mp_set_state_callback(mp_context_t *ctx,
                                      mp_on_state_changed_fn cb, void *user_data) {
    try {
        if (!ctx) return;
        pthread_mutex_lock(&ctx->mutex);
        ctx->state_cb = cb;
        ctx->state_cb_data = user_data;
        pthread_mutex_unlock(&ctx->mutex);
    } catch (...) {
        /* swallow */
    }
}

extern "C" void mp_set_track_callback(mp_context_t *ctx,
                                      mp_on_track_changed_fn cb, void *user_data) {
    try {
        if (!ctx) return;
        pthread_mutex_lock(&ctx->mutex);
        ctx->track_cb = cb;
        ctx->track_cb_data = user_data;
        pthread_mutex_unlock(&ctx->mutex);
    } catch (...) {
        /* swallow */
    }
}

extern "C" void mp_set_position_callback(mp_context_t *ctx,
                                         mp_on_position_changed_fn cb, void *user_data) {
    try {
        if (!ctx) return;
        pthread_mutex_lock(&ctx->mutex);
        ctx->position_cb = cb;
        ctx->position_cb_data = user_data;
        pthread_mutex_unlock(&ctx->mutex);
    } catch (...) {
        /* swallow */
    }
}
