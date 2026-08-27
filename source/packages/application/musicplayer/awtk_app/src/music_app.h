/**
 * @file music_app.h
 * @brief AWTK Music Player Application Controller.
 *
 * Integrates all sub-modules into one cohesive application:
 *   - usb_monitor:   USB/SD hotplug detection
 *   - music_scanner: File scanning + ID3 parsing
 *   - music_player:  Playback engine (libatcmediaplayer)
 *   - AWTK UI:       GUI widgets + data binding
 *
 * Architecture (matches Android LocalService / MediaService pattern):
 *
 *   ┌─────────────────────────────────────────────────────┐
 *   │  AWTK UI (widgets, event handlers)                   │
 *   │    ↕ music_app_xxx() API                             │
 *   ├─────────────────────────────────────────────────────┤
 *   │  music_app (this module) — coordinator               │
 *   │    ├─ usb_monitor  (Linux uevent netlink)           │
 *   │    ├─ music_scanner (dir scan + ID3v2 parse)        │
 *   │    ├─ music_player  (libatcmediaplayer wrapper)     │
 *   │    └─ storage_device[] (per-device state)           │
 *   └─────────────────────────────────────────────────────┘
 *
 * Reference: Android AutoMediaPlayer/app/.../local/LocalService.java
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 */

#ifndef MUSIC_APP_H
#define MUSIC_APP_H

#ifdef AWTK_STUB
#include "awtk_stub.h"
#else
#include "awtk.h"
#endif

#include "music_scanner.h"
#include "music_player.h"
#include "usb_monitor.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Storage device state (mirrors Android StorageDeviceEx)
 *==========================================================================*/
#define MAX_STORAGE_DEVICES  4

typedef struct {
    bool            mounted;
    bool            scanning;       /* file scan in progress */
    bool            scan_done;      /* file scan completed */
    bool            id3_done;       /* ID3 parse completed */
    storage_type_t  type;
    char            mount_point[STORAGE_PATH_MAX];
    MusicList*      music_list;     /* scan results */
    /* Issue #51: Scan generation counter for cancellation protection.
     * Incremented on each scan_device_async() call. The scan thread checks
     * this periodically; if it has changed (a newer scan was requested),
     * the older thread exits early.
     * Mirrors Android MediaFilePathScan.mLoadingIndex atomic counter. */
    volatile int    scan_generation;
} storage_device_state_t;

/*============================================================================
 * Playlist type (mirrors Android IPlaylistType)
 * Issue #23: Track which playlist source is active
 *==========================================================================*/
typedef enum {
    PLAYLIST_TYPE_DEVICE   = 0,  /* full device list */
    PLAYLIST_TYPE_FOLDER   = 1,  /* folder-filtered list */
    PLAYLIST_TYPE_FAVORITE = 2,  /* favorites list */
    PLAYLIST_TYPE_ALBUM    = 3,  /* album group list */
    PLAYLIST_TYPE_ARTIST   = 4,  /* artist group list */
} playlist_type_t;

/*============================================================================
 * Application play state (mirrors Android AppGlobalData)
 *==========================================================================*/
typedef struct {
    /* Current storage device index in s_devices[] */
    int                     current_device_idx;

    /* Play mode */
    PlayMode                play_mode;

    /* Issue #23: Current playlist type */
    playlist_type_t         playlist_type;

    /* Current track info (points into playlist, do NOT free) */
    const MusicInfo*        current_info;
    int                     current_position_ms;
    int                     current_duration_ms;

    /* Player state */
    PlayerState             player_state;

    /* Last played path (for resume) */
    char                    last_path[MUSIC_MAX_PATH_LEN];
    int                     last_position_ms;
} music_app_state_t;

/*============================================================================
 * UI update event IDs (posted to AWTK main loop via idle_queue)
 * Mirrors Android IMediaEvent constants
 *==========================================================================*/
typedef enum {
    APP_EVENT_STORAGE_MOUNTED       = 1,
    APP_EVENT_STORAGE_UNMOUNTED     = 2,
    APP_EVENT_SCAN_STARTED          = 3,
    APP_EVENT_SCAN_FINISHED         = 4,
    APP_EVENT_TRACK_CHANGED         = 5,
    APP_EVENT_STATE_CHANGED         = 6,
    APP_EVENT_POSITION_CHANGED      = 7,
    APP_EVENT_PLAYLIST_CHANGED      = 8,
    APP_EVENT_ERROR                 = 9,
    APP_EVENT_FAVORITE_CHANGED      = 10,  /* Issue #1: favorite add/remove */
} music_app_event_t;

/**
 * @brief UI callback — called on AWTK main thread.
 * @param event  Event type
 * @param param  Event-specific param (may be NULL)
 */
typedef void (*music_app_ui_callback_t)(music_app_event_t event,
                                        void* param);

/*============================================================================
 * Public API
 *==========================================================================*/

/**
 * @brief Initialize the music application.
 * Must be called once after AWTK tk_init().
 * @param ui_cb  Callback for UI thread notifications.
 * @return 0 on success.
 */
int music_app_init(music_app_ui_callback_t ui_cb);

/**
 * @brief Shutdown and release all resources.
 */
void music_app_deinit(void);

/**
 * @brief Get global app state (read-only outside of callbacks).
 */
const music_app_state_t* music_app_get_state(void);

/* --- Playback control (thread-safe, can be called from UI) --- */
void music_app_play(int index);          /* -1 = resume/start */
void music_app_pause(void);
void music_app_resume(void);
void music_app_stop(void);
void music_app_next(void);
void music_app_prev(void);
void music_app_seek(int position_ms);
void music_app_toggle_play_pause(void);

/* --- Issue #A3: Fast-forward / rewind (Android onSeekRewind/onFastForward) --- */
#define MUSIC_APP_SEEK_STEP_MS  5000  /* 5 seconds, matches Android SEEK_STEP */
void music_app_seek_forward(int step_ms);
void music_app_seek_backward(int step_ms);

/* --- Play mode --- */
void music_app_set_play_mode(PlayMode mode);
void music_app_cycle_play_mode(void);

/* --- Playlist access --- */
int              music_app_get_playlist_count(void);
const MusicInfo* music_app_get_track_info(int index);
int              music_app_get_current_index(void);

/* --- Storage device access --- */
int  music_app_get_device_count(void);
const storage_device_state_t* music_app_get_device(int idx);
void music_app_switch_device(int idx);

/* --- Issue #A4: Player lock query (Android mIsMediaPlayerLocked) --- */
bool music_app_is_player_locked(void);

/* --- Issue #A4: Safe play/next/prev with player lock check --- */
/* Returns 0 on success, -1 if player is locked (preparing). */
int music_app_safe_play(int index);
int music_app_safe_next(void);
int music_app_safe_prev(void);

/* --- Force re-scan current device --- */
void music_app_rescan(void);

/* --- Last-memory persistence --- */
void music_app_save_state(void);
void music_app_restore_state(void);

/* --- Favorite management (Issue #1, mirrors Android FavoriteManager) --- */
bool music_app_toggle_favorite(void);
bool music_app_is_favorite(void);
int  music_app_get_favorite_count(void);
const MusicInfo* music_app_get_favorite_list(int* out_count);

/* --- Folder browsing (Issue #2, mirrors Android FolderListLayout) --- */

/**
 * @brief Get unique folder list from current device's scanned files.
 * @param out_folders  Receives an array of folder path strings.
 *                     Caller must NOT free (points into internal data).
 * @param out_count    Receives the number of folders.
 */
void music_app_get_folder_list(const char*** out_folders, int* out_count);

/**
 * @brief Filter playlist to only songs in the given folder.
 *        Starts playing the first song in that folder.
 * @param folder_path  The folder to play from.
 */
void music_app_play_folder(const char* folder_path);

/* --- Search (Issue #6) --- */

/**
 * @brief Search tracks by keyword (matches title/artist/album/filename).
 * @param keyword    Search string (case-insensitive).
 * @param results    Caller-provided array to receive matching MusicInfo pointers.
 * @param max_results  Max entries to fill.
 * @return Number of matches found.
 */
int music_app_search(const char* keyword, const MusicInfo** results, int max_results);

/* --- Album/Artist classification (Issue #3, mirrors Android classifyMediaInfoList) --- */

/** Max number of unique albums or artists */
#include "darray.h"

/* Dynamic array of const MusicInfo pointers (used inside groups) */
DARRAY_DEFINE(MusicPtrArray, const MusicInfo*)

/**
 * @brief A group of tracks sharing the same album or artist key.
 *        Mirrors Android MusicKeyInfo.
 */
typedef struct {
    char            key[MUSIC_MAX_TAG_LEN]; /* album or artist name */
    MusicPtrArray   items;                  /* was: const MusicInfo* items[2000] */
} music_group_t;

/* Dynamic array of music_group_t (replaces album_groups[256]) */
DARRAY_DEFINE(GroupArray, music_group_t)

/* Dynamic array of char* string pointers (replaces folder_paths[2000]) */
DARRAY_DEFINE(StrPtrArray, char*)

/**
 * @brief Get album groups from current device.
 * @param out_groups  Receives pointer to internal array.
 * @param out_count   Number of groups.
 */
void music_app_get_album_list(const music_group_t** out_groups, int* out_count);

/**
 * @brief Get artist groups from current device.
 */
void music_app_get_artist_list(const music_group_t** out_groups, int* out_count);

/**
 * @brief Play a track from within a group (album or artist).
 * @param group       The group to play from.
 * @param index       Index within the group.
 */
void music_app_play_group(const music_group_t* group, int index);

/* --- LRC lyrics (Issue #8, mirrors Android LyricsManager) --- */

/** A single LRC lyrics line */
typedef struct {
    int  time_ms;       /* timestamp in milliseconds */
    char text[256];     /* lyrics text content */
} lrc_line_t;

/** Parsed LRC result */
typedef struct {
    lrc_line_t* lines;
    int         count;
    int         capacity;
} lrc_data_t;

/**
 * @brief Load LRC lyrics for the currently playing track.
 *        Searches for a .lrc file with the same name as the audio file.
 * @return Pointer to internal lrc_data (NULL if no lyrics found).
 *         Valid until next call or track change.
 */
const lrc_data_t* music_app_get_lyrics(void);

/**
 * @brief Find the lyrics line index for a given playback position.
 * @param time_ms  Current playback position in ms.
 * @return Line index (0-based), or -1 if no lyrics.
 */
int music_app_get_lyrics_line(int time_ms);

/* --- Album art / APIC (Issue #7) --- */

/**
 * @brief Extract album art (APIC frame) from current track's ID3 tag.
 * @param out_data  Receives pointer to JPEG/PNG data (internal buffer).
 * @param out_size  Receives data size in bytes.
 * @return 0 on success, -1 if no album art found.
 */
int music_app_get_album_art(const uint8_t** out_data, int* out_size);

/* --- ACC lifecycle (Issue #32, mirrors Android LocalService ACC handling) --- */

/**
 * @brief Handle ACC OFF event — save state and stop playback.
 * Should be called when the vehicle ignition is turned off.
 */
void music_app_on_acc_off(void);

/**
 * @brief Handle ACC ON event — restore state and optionally resume.
 * Should be called when the vehicle ignition is turned on.
 */
void music_app_on_acc_on(void);

/* --- Playlist restore (Issue #36, mirrors Android MusicPlaylistEx.mFirstPlaylistEx) --- */

/**
 * @brief Restore the full device playlist after folder/album/artist sub-list playback.
 * This resets the player's playlist to the complete scan result of the current device.
 */
void music_app_restore_full_playlist(void);

#ifdef __cplusplus
}
#endif

#endif /* MUSIC_APP_H */
