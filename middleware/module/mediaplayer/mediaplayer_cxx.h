/**
 * @file mediaplayer_cxx.h
 * @brief MediaPlayer C++ 适配层对外唯一 C ABI
 *
 * 本头文件铁律（照搬 bt_cxx.h）：
 *   - 只出现 C 类型；STL/C++ 类/引用/异常禁止出现在此
 *   - 错误码一律 int 返回
 *   - 此头文件可被任何 .c 文件安全 #include
 *
 * 完整链路：
 *   C 门面 (music_app.c) -> 本头文件 -> C++ (MediaPlayerManager 操作厂商 MediaPlayer 类)
 *
 * 对标关系：
 *   厂商 libatcmediaplayer.so (C++ MediaPlayer 类)
 *     ↕
 *   mediaplayer_manager.cpp (C++ 内部: new/delete/play/pause/seek)
 *     ↕
 *   mediaplayer_cxx.cpp (extern "C" wrapper, try/catch 兜底)
 *     ↕
 *   本头文件 (纯 C ABI) -> music_app.c / music_ui.c
 */
#ifndef __MEDIAPLAYER_CXX_H__
#define __MEDIAPLAYER_CXX_H__

#include <stdint.h>
#include <stdbool.h>
#include "music_scanner.h"  /* MusicInfo, MusicList types */

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Error codes (aligned with bt_cxx.h pattern) ---- */
enum {
    MP_CXX_OK           =  0,
    MP_CXX_ERR          = -1,  /**< 未分类错误（含 C++ 异常兜底） */
    MP_CXX_ERR_ARGS     = -2,  /**< 参数非法 */
    MP_CXX_ERR_NOTINIT  = -3,  /**< 尚未 init 或已 deinit */
    MP_CXX_ERR_LOCKED   = -4,  /**< player 正在 preparing，操作被拦截 */
};

/* ---- Play mode (aligned with Android IConstant) ---- */
typedef enum {
    MP_MODE_SEQUENTIAL = 0,
    MP_MODE_REPEAT_ALL = 1,
    MP_MODE_REPEAT_ONE = 2,
    MP_MODE_SHUFFLE    = 3,
} mp_play_mode_t;

/* ---- Player state ---- */
typedef enum {
    MP_STATE_IDLE    = 0,
    MP_STATE_PLAYING = 1,
    MP_STATE_PAUSED  = 2,
    MP_STATE_STOPPED = 3,
    MP_STATE_ERROR   = 4,
} mp_player_state_t;

/* ---- Callbacks (called from player thread context) ---- */
typedef void (*mp_on_state_changed_fn)(mp_player_state_t state, void *user_data);
typedef void (*mp_on_track_changed_fn)(int index, const MusicInfo *info, void *user_data);
typedef void (*mp_on_position_changed_fn)(int position_ms, int duration_ms, void *user_data);

/* ---- Opaque handle ---- */
typedef struct mp_context mp_context_t;

/* ---- Link probe (bt_cxx pattern: verify C++ layer is linked) ---- */
int mp_cxx_probe(void);

/* ---- Lifecycle ---- */
mp_context_t *mp_create(void);
void          mp_destroy(mp_context_t *ctx);

/* ---- Playlist ---- */
int              mp_set_playlist(mp_context_t *ctx, const MusicList *list);
int              mp_get_playlist_count(mp_context_t *ctx);
const MusicInfo *mp_get_track_info(mp_context_t *ctx, int index);
int              mp_get_current_index(mp_context_t *ctx);

/* ---- Playback control ---- */
int  mp_play(mp_context_t *ctx, int index);   /**< index=-1: resume or start from 0 */
int  mp_pause(mp_context_t *ctx);
int  mp_resume(mp_context_t *ctx);
int  mp_stop(mp_context_t *ctx);
int  mp_next(mp_context_t *ctx);
int  mp_prev(mp_context_t *ctx);
int  mp_seek(mp_context_t *ctx, int position_ms);

/* ---- Play mode ---- */
void            mp_set_mode(mp_context_t *ctx, mp_play_mode_t mode);
mp_play_mode_t  mp_get_mode(mp_context_t *ctx);

/* ---- State query ---- */
mp_player_state_t mp_get_state(mp_context_t *ctx);
int               mp_get_position(mp_context_t *ctx);

/* ---- Callbacks ---- */
void mp_set_state_callback(mp_context_t *ctx, mp_on_state_changed_fn cb, void *user_data);
void mp_set_track_callback(mp_context_t *ctx, mp_on_track_changed_fn cb, void *user_data);
void mp_set_position_callback(mp_context_t *ctx, mp_on_position_changed_fn cb, void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* __MEDIAPLAYER_CXX_H__ */
