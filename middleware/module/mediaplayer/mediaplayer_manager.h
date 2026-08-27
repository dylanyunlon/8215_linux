/**
 * @file mediaplayer_manager.h
 * @brief MediaPlayer 适配层内部实现（仅 C++ 层使用，禁止被对外头/应用包含）
 *
 * STL 使用边界：std::string / std::nothrow 只出现在本层与 .cpp 内；
 * 一切对外的形态都经 mediaplayer_cxx.cpp 收敛为 C ABI。
 *
 * 持有厂商 MediaPlayer C++ 对象 (libatcmediaplayer.so)，管理：
 *   - 播放器生命周期 (setup/destroy)
 *   - 播放列表 (copy-in)
 *   - 播放模式与 shuffle consumption pool
 *   - Position 轮询线程
 *   - State/Track/Position 回调分发
 *
 * 对标 Android: MediaPlayerModel.java (播放引擎) + MusicPlaylistEx.java (列表/随机)
 */
#ifndef __MEDIAPLAYER_MANAGER_H__
#define __MEDIAPLAYER_MANAGER_H__

#include <cstdint>
#include <pthread.h>

/* C headers (all have extern "C" guard) */
#include "music_scanner.h"
#include "mediaplayer_cxx.h"

/* Forward declare vendor class — full include only in .cpp */
class MediaPlayer;

/**
 * MediaPlayerManager — per-instance, not singleton.
 *
 * mp_context_t in mediaplayer_cxx.h is a typedef of this struct
 * (opaque to C callers).
 */
struct mp_context {
    /* Vendor player */
    MediaPlayer         *player;
    bool                 player_ready;

    /* Playlist (copied from MusicList) */
    MusicInfo           *playlist;
    int                  playlist_count;
    int                  current_index;

    /* Shuffle consumption pool (mirrors Android mRandomPositionList) */
    int                 *shuffle_pool;
    int                  shuffle_pool_count;
    int                 *shuffle_order;     /* legacy, kept for set_mode */

    /* State */
    mp_player_state_t    state;
    mp_play_mode_t       mode;

    /* Callbacks */
    mp_on_state_changed_fn    state_cb;
    void                     *state_cb_data;
    mp_on_track_changed_fn    track_cb;
    void                     *track_cb_data;
    mp_on_position_changed_fn position_cb;
    void                     *position_cb_data;

    /* Position polling thread */
    pthread_t            poll_thread;
    bool                 poll_running;

    pthread_mutex_t      mutex;
};

/* ---- Internal helpers (called from mediaplayer_manager.cpp) ---- */
void mgr_rebuild_shuffle_pool(mp_context *ctx);
void mgr_shuffle_pool_remove(mp_context *ctx, int track_index);
int  mgr_shuffle_pool_pick_next(mp_context *ctx);
void mgr_generate_shuffle(mp_context *ctx);

#endif /* __MEDIAPLAYER_MANAGER_H__ */
