/**
 * @file music_app_impl.h
 * @brief MusicAppImpl — C++ 业务逻辑类（仅 C++ 层使用，禁止被 C 头/应用包含）
 *
 * 遵循 middleware/module/bt/bt_device_manager.h 范式：
 *   - STL（vector/unordered_map/string/mutex）只出现在本层与 .cpp 内
 *   - 一切对外形态经 music_app.h（C ABI）收敛
 *   - 异常绝不穿越 C 边界
 *
 * 对标 Android: LocalService.java + AppGlobalData.java + MediaService.java
 *
 * 新文件位置: source/packages/application/musicplayer/awtk_app/src/music_app_impl.h
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 */
#ifndef MUSIC_APP_IMPL_H
#define MUSIC_APP_IMPL_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/* 当前阶段：用 pthread 保持与 music_app.c 一致。
 * 等 lib_mw.a / lib_mw_cxx.a 链入 musicplayer_awtk 后，
 * 切换到 osal_cxx.h 的 RAII 封装 (Mutex/MutexGuard/Worker)。
 *
 * TODO: #include "osal_cxx.h" 替换以下 pthread include
 */
#include <pthread.h>

/* C headers (all have extern "C" guard) */
#include "music_scanner.h"
#include "music_player.h"
#include "usb_monitor.h"

/*============================================================================
 * Forward declarations
 *==========================================================================*/

/**
 * @brief UI callback type — same as music_app_ui_callback_t in music_app.h.
 * Redeclared here to avoid pulling entire music_app.h into C++ internals.
 */
typedef void (*ui_callback_fn)(int event, void* param);

/*============================================================================
 * MusicAppImpl — singleton, C++ business logic
 *
 * Design mirrors Android LocalService (coordinator) + AppGlobalData (state).
 * Thread model:
 *   - UI thread:     music_app_xxx() → MusicAppImpl public methods
 *   - Scan thread:   osal::Worker, accesses shared state under mutex_
 *   - Player thread: callbacks from libatcmediaplayer, dispatched via idle_queue
 *   - USB thread:    usb_monitor callback, dispatched via idle_queue
 *==========================================================================*/
class MusicAppImpl {
public:
    /** Singleton (function-local static, no cross-TU init order issues) */
    static MusicAppImpl& instance();

    /* --- Lifecycle --- */
    int  init(ui_callback_fn cb);
    void deinit();
    bool is_inited() const { return inited_; }

    /* --- Play mode (mirrors Android IMusicState.REPEAT_MODE_*) --- */
    enum PlayModeE {
        MODE_SEQUENTIAL = 0,
        MODE_REPEAT_ALL = 1,
        MODE_REPEAT_ONE = 2,
        MODE_SHUFFLE    = 3,
    };

    /* --- Playlist type (mirrors Android IPlaylistType) --- */
    enum PlaylistTypeE {
        PL_DEVICE   = 0,
        PL_FOLDER   = 1,
        PL_FAVORITE = 2,
        PL_ALBUM    = 3,
        PL_ARTIST   = 4,
    };

    /* --- Playback control (all thread-safe, callable from UI thread) --- */
    void play(int index);
    void pause();
    void resume();
    void stop();
    void next();
    void prev();
    void seek(int position_ms);
    void toggle_play_pause();

    /* Issue #A3: Fast-forward / rewind (Android onSeekRewind/onFastForward) */
    void seek_forward(int step_ms = 5000);
    void seek_backward(int step_ms = 5000);

    /* --- Play mode --- */
    void set_play_mode(PlayModeE mode);
    void cycle_play_mode();
    PlayModeE get_play_mode() const;

    /* --- Playlist access --- */
    int              get_playlist_count() const;
    const MusicInfo* get_track_info(int index) const;
    int              get_current_index() const;

    /* --- Storage device access --- */
    struct DeviceState {
        bool            mounted   = false;
        bool            scanning  = false;
        bool            scan_done = false;
        bool            id3_done  = false;
        storage_type_t  type      = STORAGE_TYPE_USB;
        std::string     mount_point;
        MusicList*      music_list = nullptr;  /* raw C list (scanner output) */
        std::atomic<int> scan_generation{0};   /* Issue #51: cancellation */
    };

    int  get_device_count() const;
    const DeviceState* get_device(int idx) const;
    void switch_device(int idx);
    void rescan();

    /* --- State persistence --- */
    void save_state();
    void restore_state();

    /* --- Favorite management (Issue #1) --- */
    bool toggle_favorite();
    bool is_favorite() const;
    int  get_favorite_count() const;
    const MusicInfo* get_favorite_list(int* out_count) const;

    /* --- Folder browsing (Issue #2) --- */
    void get_folder_list(std::vector<std::string>& out_folders) const;
    void play_folder(const std::string& folder_path);

    /* --- Search (Issue #6) --- */
    int search(const std::string& keyword,
               std::vector<const MusicInfo*>& results,
               int max_results) const;

    /* --- Album/Artist classification (Issue #3 + #F4) ---
     * C++ containers: O(1) lookup vs old O(n) linear scan */
    struct MusicGroup {
        std::string                    key;
        std::vector<const MusicInfo*>  items;
    };

    void get_album_list(std::vector<MusicGroup>& out) const;
    void get_artist_list(std::vector<MusicGroup>& out) const;
    void play_group(const std::string& key, bool is_artist, int index);

    /* --- LRC lyrics (Issue #8) --- */
    struct LrcLine {
        int         time_ms;
        std::string text;
    };
    const std::vector<LrcLine>& get_lyrics() const;
    int get_lyrics_line(int time_ms) const;

    /* --- Album art (Issue #7) --- */
    int get_album_art(const uint8_t** out_data, int* out_size) const;

    /* --- ACC lifecycle (Issue #32) --- */
    void on_acc_off();
    void on_acc_on();

    /* --- Playlist restore (Issue #36) --- */
    void restore_full_playlist();

    /* --- Issue #A4: Player lock (防止 preparing 期间并发操作) --- */
    bool is_player_locked() const { return player_locked_; }

    /* --- Issue #F7: Resume play state machine --- */
    bool allow_resume_play() const { return allow_resume_play_; }
    void set_allow_resume_play(bool v) { allow_resume_play_ = v; }

    /* --- Read-only state access (for UI) --- */
    struct AppState {
        int             current_device_idx = -1;
        PlayModeE       play_mode     = MODE_REPEAT_ALL;
        PlaylistTypeE   playlist_type = PL_DEVICE;
        const MusicInfo* current_info = nullptr;
        int             current_position_ms = 0;
        int             current_duration_ms = 0;
        PlayerState     player_state  = PLAYER_STATE_IDLE;
        std::string     last_path;
        int             last_position_ms = 0;
    };

    const AppState& state() const { return state_; }

private:
    MusicAppImpl() = default;
    ~MusicAppImpl() = default;
    MusicAppImpl(const MusicAppImpl&) = delete;
    MusicAppImpl& operator=(const MusicAppImpl&) = delete;

    /* --- Internal helpers --- */
    void post_ui_event(int event, int param);
    int  find_device_by_mount(const std::string& mp) const;
    int  add_device(const storage_device_info_t* info);
    void remove_device(int idx);
    void scan_device_async(int dev_idx);
    void build_folder_cache();
    void build_classification_cache();
    void load_lyrics_for_current();
    void load_album_art_for_current();

    /* --- Callbacks (registered with C modules) --- */
    static void on_storage_event_cb(const storage_device_info_t* info, void* ud);
    static void on_player_state_cb(PlayerState state, void* ud);
    static void on_player_track_cb(int index, const MusicInfo* info, void* ud);
    static void on_player_position_cb(int pos_ms, int dur_ms, void* ud);

    /* --- State --- */
    AppState             state_;
    MusicPlayerContext*  player_ = nullptr;
    ui_callback_fn       ui_cb_  = nullptr;
    bool                 inited_ = false;

    /* Issue #A4: Player lock — true between play command and PLAYING state */
    std::atomic<bool>    player_locked_{false};

    /* Issue #F7: Resume play state machine */
    std::atomic<bool>    allow_resume_play_{false};

    /* [GAP-6] Prev/next debounce */
    uint64_t             last_prev_next_ms_ = 0;

    /* [GAP-7] Error auto-skip counter */
    int                  error_count_ = 0;

    /* Storage devices (C++ vector replaces C fixed array) */
    static constexpr int kMaxDevices = 4;
    std::vector<DeviceState> devices_;

    /* Issue #2: Folder cache — sorted unique list (replaces darray+qsort) */
    std::vector<std::string> folder_cache_;

    /* Issue #3 + #F4: Classification — O(1) lookup via unordered_map */
    std::unordered_map<std::string, std::vector<const MusicInfo*>> album_map_;
    std::unordered_map<std::string, std::vector<const MusicInfo*>> artist_map_;
    /* Materialized lists for get_album_list/get_artist_list (stable pointers) */
    mutable std::vector<MusicGroup> album_groups_cache_;
    mutable std::vector<MusicGroup> artist_groups_cache_;
    mutable bool album_cache_dirty_  = true;
    mutable bool artist_cache_dirty_ = true;

    /* Issue #7: Album art cache */
    std::vector<uint8_t> album_art_data_;
    std::string          album_art_path_;

    /* Issue #8: LRC lyrics cache */
    std::vector<LrcLine> lyrics_;
    std::string          lyrics_path_;

    /* Thread safety — pthread mutex (same as music_app.c)
     * TODO: 切换到 osal::Mutex + MutexGuard (RAII) when lib_mw linked */
    mutable pthread_mutex_t  mutex_;

    /* AWTK timer IDs (must only be accessed from main thread) */
    uint32_t auto_next_timer_id_ = 0;  /* TK_INVALID_ID */
    uint32_t save_timer_id_      = 0;
};

#endif /* MUSIC_APP_IMPL_H */
