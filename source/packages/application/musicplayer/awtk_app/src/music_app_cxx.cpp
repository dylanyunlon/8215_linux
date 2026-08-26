/**
 * @file music_app_cxx.cpp
 * @brief C ABI wrapper — 所有 extern "C" 入口 try/catch 全兜底
 *
 * 遵循 middleware/module/bt/bt_cxx.cpp 范式：
 *   1. 异常绝不穿越 C 边界
 *   2. std::string -> char* 拷贝只在本层做
 *   3. 返回值只使用 music_app_cxx.h 定义的 int 错误码
 *   4. wrapper 不写业务逻辑 — 全部下沉到 MusicAppImpl
 *
 * 位置: source/packages/application/musicplayer/awtk_app/src/music_app_cxx.cpp
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 */
#include <cstdio>

#include "music_app_cxx.h"
#include "music_app_impl.h"
#include "music_app.h"  /* 获取现有 C API 中的类型定义 */

extern "C" int music_cxx_probe(void) {
    try {
        return 1;  /* 适配层版本号 */
    } catch (...) {
        return MUSIC_CXX_ERR;
    }
}

extern "C" int music_cxx_seek_forward(int step_ms) {
    try {
        if (step_ms <= 0) return MUSIC_CXX_ERR_ARGS;
        music_app_seek_forward(step_ms);
        return MUSIC_CXX_OK;
    } catch (...) {
        return MUSIC_CXX_ERR;
    }
}

extern "C" int music_cxx_seek_backward(int step_ms) {
    try {
        if (step_ms <= 0) return MUSIC_CXX_ERR_ARGS;
        music_app_seek_backward(step_ms);
        return MUSIC_CXX_OK;
    } catch (...) {
        return MUSIC_CXX_ERR;
    }
}

extern "C" int music_cxx_is_player_locked(void) {
    try {
        return music_app_is_player_locked() ? 1 : 0;
    } catch (...) {
        return MUSIC_CXX_ERR;
    }
}

extern "C" int music_cxx_get_allow_resume(void) {
    try {
        /* TODO: 等 music_app.c 增加 allow_resume_play 字段后接入 */
        return 0;
    } catch (...) {
        return MUSIC_CXX_ERR;
    }
}

extern "C" int music_cxx_set_allow_resume(int allow) {
    try {
        /* TODO: 等 music_app.c 增加 allow_resume_play 字段后接入 */
        (void)allow;
        return MUSIC_CXX_OK;
    } catch (...) {
        return MUSIC_CXX_ERR;
    }
}

extern "C" int music_cxx_play_favorites(int index) {
    try {
        if (!MusicAppImpl::instance().is_inited()) {
            return MUSIC_CXX_ERR_NOTINIT;
        }
        /* TODO: Issue #F2 — 从 favorite_manager 获取收藏列表，
         * 构建子 playlist 并设为 player 的当前列表。
         * 当前先返回 ERR 表示待实现。 */
        (void)index;
        std::fprintf(stderr, "[music_cxx] play_favorites: not yet implemented\n");
        return MUSIC_CXX_ERR;
    } catch (...) {
        return MUSIC_CXX_ERR;
    }
}

extern "C" int music_cxx_safe_play(int index) {
    try {
        /* Issue #A4: 查 C 层的 player_locked 状态
         * 对标 Android: if (mAppData.mIsMediaPlayerLocked) return false; */
        if (music_app_is_player_locked()) {
            return MUSIC_CXX_ERR_LOCKED;
        }
        music_app_play(index);
        return MUSIC_CXX_OK;
    } catch (...) {
        return MUSIC_CXX_ERR;
    }
}

extern "C" int music_cxx_safe_next(void) {
    try {
        if (music_app_is_player_locked()) {
            return MUSIC_CXX_ERR_LOCKED;
        }
        music_app_next();
        return MUSIC_CXX_OK;
    } catch (...) {
        return MUSIC_CXX_ERR;
    }
}

extern "C" int music_cxx_safe_prev(void) {
    try {
        if (music_app_is_player_locked()) {
            return MUSIC_CXX_ERR_LOCKED;
        }
        music_app_prev();
        return MUSIC_CXX_OK;
    } catch (...) {
        return MUSIC_CXX_ERR;
    }
}
