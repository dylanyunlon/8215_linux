/**
 * @file music_app_cxx.h
 * @brief C++ 适配层对外 C ABI — 遵循 middleware/module/bt/bt_cxx.h 范式
 *
 * 铁律（逐条照搬 bt_cxx.h）：
 *   - 只出现 C 类型；STL/C++ 类/引用/异常禁止出现在此
 *   - 错误码一律 int 返回
 *   - 此头文件可被任何 .c 文件安全 #include
 *
 * 本文件补充 music_app.h 中缺失的 C++ 增强功能：
 *   - Issue #A3: 快进/快退
 *   - Issue #A4: 播放锁查询
 *   - Issue #F7: 播放恢复状态查询
 *
 * 位置: source/packages/application/musicplayer/awtk_app/src/music_app_cxx.h
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 */
#ifndef MUSIC_APP_CXX_H
#define MUSIC_APP_CXX_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 错误码 */
enum {
    MUSIC_CXX_OK           =  0,
    MUSIC_CXX_ERR          = -1,
    MUSIC_CXX_ERR_ARGS     = -2,
    MUSIC_CXX_ERR_NOTINIT  = -3,
    MUSIC_CXX_ERR_LOCKED   = -4,  /**< player 正在 preparing，操作被拦截 */
};

/** 链接闭环探测（验证 C++ 适配层是否已链入） */
int music_cxx_probe(void);

/**
 * Issue #A3: 快进（前进 step_ms 毫秒）
 * 对标 Android: LocalService.onFastForward() — SEEK_STEP=5000ms
 */
int music_cxx_seek_forward(int step_ms);

/**
 * Issue #A3: 快退（后退 step_ms 毫秒）
 * 对标 Android: LocalService.onSeekRewind() — SEEK_STEP=5000ms
 */
int music_cxx_seek_backward(int step_ms);

/**
 * Issue #A4: 查询播放锁状态
 * true = player 正在 preparing / switching track，此时禁止新的播放指令
 * 对标 Android: mAppData.mIsMediaPlayerLocked
 */
int music_cxx_is_player_locked(void);

/**
 * Issue #F7: 查询是否应该恢复播放
 * 对标 Android: mAppData.mAllowResumePlay
 */
int music_cxx_get_allow_resume(void);

/**
 * Issue #F7: 设置恢复播放标志
 */
int music_cxx_set_allow_resume(int allow);

/**
 * Issue #F2: 播放收藏列表（将收藏列表设为当前 playlist）
 * 对标 Android: tryUpdateMusicPlaylist(IPlaylistType.FAVORITE_LIST, ...)
 * @param index  要播放的收藏列表中的索引，-1 从头开始
 */
int music_cxx_play_favorites(int index);

/**
 * Issue #A3: 带播放锁检查的 play（preparing 期间拦截）
 * 对标 Android: onLocalMusicPlayControl 的 mIsMediaPlayerLocked 门控
 */
int music_cxx_safe_play(int index);
int music_cxx_safe_next(void);
int music_cxx_safe_prev(void);

#ifdef __cplusplus
}
#endif

#endif /* MUSIC_APP_CXX_H */
