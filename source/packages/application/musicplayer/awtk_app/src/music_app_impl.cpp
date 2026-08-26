/**
 * @file music_app_impl.cpp
 * @brief MusicAppImpl 最小实现 — 代理到现有 music_app.h C API
 *
 * 当前阶段：薄代理层，所有调用转发到 music_app_xxx() C 函数。
 * 后续阶段：逐步将 music_app.c 的逻辑迁移到这里，用 STL 替代 darray。
 *
 * 遵循 bt_device_manager.cpp 范式：
 *   - 内部用 C++ (STL/RAII)
 *   - 对外通过 music_app_cxx.cpp 的 extern "C" wrapper 暴露
 *
 * 位置: source/packages/application/musicplayer/awtk_app/src/music_app_impl.cpp
 */

#include "music_app_impl.h"
#include "music_app.h"

#include <cstdio>
#include <cstring>
#include <algorithm>

/* ======================================================================== */
/* Singleton                                                                 */
/* ======================================================================== */

MusicAppImpl& MusicAppImpl::instance() {
    static MusicAppImpl s_instance;
    return s_instance;
}

/* ======================================================================== */
/* Lifecycle — 代理到 music_app_init / music_app_deinit                      */
/* ======================================================================== */

int MusicAppImpl::init(ui_callback_fn cb) {
    if (inited_) return -1;
    ui_cb_ = cb;
    inited_ = true;
    /* 实际初始化仍由 music_app_init() 完成 (main.c 调用) */
    return 0;
}

void MusicAppImpl::deinit() {
    inited_ = false;
}

/* ======================================================================== */
/* Playback control — 代理到 music_app_xxx()                                */
/* ======================================================================== */

void MusicAppImpl::play(int index)           { music_app_play(index); }
void MusicAppImpl::pause()                   { music_app_pause(); }
void MusicAppImpl::resume()                  { music_app_resume(); }
void MusicAppImpl::stop()                    { music_app_stop(); }
void MusicAppImpl::seek(int position_ms)     { music_app_seek(position_ms); }
void MusicAppImpl::toggle_play_pause()       { music_app_toggle_play_pause(); }

void MusicAppImpl::next() {
    player_locked_ = true;
    music_app_next();
}

void MusicAppImpl::prev() {
    player_locked_ = true;
    music_app_prev();
}

/* Issue #A3: Fast-forward / rewind */
void MusicAppImpl::seek_forward(int step_ms) {
    music_app_seek_forward(step_ms);
}

void MusicAppImpl::seek_backward(int step_ms) {
    music_app_seek_backward(step_ms);
}

/* ======================================================================== */
/* Play mode                                                                 */
/* ======================================================================== */

void MusicAppImpl::set_play_mode(PlayModeE mode) {
    music_app_set_play_mode(static_cast<PlayMode>(mode));
}

void MusicAppImpl::cycle_play_mode() {
    music_app_cycle_play_mode();
}

MusicAppImpl::PlayModeE MusicAppImpl::get_play_mode() const {
    const music_app_state_t* st = music_app_get_state();
    return static_cast<PlayModeE>(st->play_mode);
}

/* ======================================================================== */
/* Playlist access                                                           */
/* ======================================================================== */

int MusicAppImpl::get_playlist_count() const {
    return music_app_get_playlist_count();
}

const MusicInfo* MusicAppImpl::get_track_info(int index) const {
    return music_app_get_track_info(index);
}

int MusicAppImpl::get_current_index() const {
    return music_app_get_current_index();
}

/* ======================================================================== */
/* Favorite — 代理到 music_app_xxx()                                        */
/* ======================================================================== */

bool MusicAppImpl::toggle_favorite()      { return music_app_toggle_favorite(); }
bool MusicAppImpl::is_favorite() const    { return music_app_is_favorite(); }
int  MusicAppImpl::get_favorite_count() const { return music_app_get_favorite_count(); }

const MusicInfo* MusicAppImpl::get_favorite_list(int* out_count) const {
    return music_app_get_favorite_list(out_count);
}

/* ======================================================================== */
/* ACC lifecycle                                                             */
/* ======================================================================== */

void MusicAppImpl::on_acc_off() { music_app_on_acc_off(); }
void MusicAppImpl::on_acc_on()  { music_app_on_acc_on(); }

/* ======================================================================== */
/* Playlist restore                                                          */
/* ======================================================================== */

void MusicAppImpl::restore_full_playlist() { music_app_restore_full_playlist(); }
