/**
 * @file music_app.c
 * @brief AWTK Music Player Application Controller implementation.
 *
 * This is the "brain" of the music player, coordinating:
 *   1. USB hotplug → trigger scan
 *   2. Scan complete → load playlist
 *   3. Playback events → update UI
 *   4. State persistence → last-memory resume
 *
 * Reference: Android LocalService.java (~2600 lines, collapsed here to ~500)
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 */

#include "music_app.h"
#include "favorite_manager.h"

#define _GNU_SOURCE  /* for strcasestr */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>

/*============================================================================
 * Config paths (mirrors Android Preferences / IConstant)
 *==========================================================================*/
#define STATE_FILE_DIR   "/data/music"
#define STATE_FILE_PATH  "/data/music/last_state.cfg"

/*============================================================================
 * Module state
 *==========================================================================*/
static struct {
    music_app_state_t       state;
    MusicPlayerContext*     player;
    storage_device_state_t  devices[MAX_STORAGE_DEVICES];
    int                     device_count;
    music_app_ui_callback_t ui_cb;
    pthread_mutex_t         mutex;
    bool                    inited;

    /* [GAP-6] Prev/next debounce (Android ControlHandler 500ms filter) */
    uint64_t                last_prev_next_ms;

    /* [GAP-7] Error file auto-skip counter (Android mFileNotExistCount) */
    int                     error_count;

    /* [GAP-9] Delayed auto-next timer ID */
    uint32_t                auto_next_timer_id;

    /* [GAP-11] Periodic state save timer ID */
    uint32_t                save_timer_id;

    /* Issue #2: Folder list cache for folder browsing */
    char*                   folder_paths[MUSIC_MAX_FILES]; /* unique folder strings */
    int                     folder_count;

    /* Issue #3: Album/Artist classification cache */
    music_group_t           album_groups[MUSIC_MAX_GROUPS];
    int                     album_group_count;
    music_group_t           artist_groups[MUSIC_MAX_GROUPS];
    int                     artist_group_count;

    /* Issue #7: Album art cache for current track */
    uint8_t*                album_art_data;
    int                     album_art_size;
    char                    album_art_path[MUSIC_MAX_PATH_LEN]; /* filepath of cached art */

    /* Issue #8: LRC lyrics cache for current track */
    lrc_data_t              lyrics;
    char                    lyrics_path[MUSIC_MAX_PATH_LEN]; /* filepath of cached lyrics */
} s_app;

/*============================================================================
 * Forward declarations
 *==========================================================================*/
static void on_storage_event(const storage_device_info_t* info, void* user_data);
static void on_player_state(PlayerState state, void* user_data);
static void on_player_track(int index, const MusicInfo* info, void* user_data);
static void on_player_position(int position_ms, int duration_ms, void* user_data);
static void scan_device_async(int dev_idx);
static void build_folder_cache(void);
static void build_classification_cache(void);
static void load_lyrics_for_current(void);
static void load_album_art_for_current(void);

/*============================================================================
 * AWTK main-thread dispatch helpers
 *
 * Callbacks from USB monitor / player threads use idle_queue to
 * deliver events to the AWTK main loop, ensuring thread safety
 * for widget updates.
 *==========================================================================*/

typedef struct {
    music_app_event_t event;
    int               int_param;
} ui_event_data_t;

static ret_t ui_event_dispatch(const idle_info_t* idle) {
    ui_event_data_t* data = (ui_event_data_t*)idle->ctx;
    if (data && s_app.ui_cb) {
        s_app.ui_cb(data->event, &data->int_param);
    }
    free(data);
    return RET_REMOVE;
}

static void post_ui_event(music_app_event_t event, int param) {
    ui_event_data_t* data = (ui_event_data_t*)calloc(1, sizeof(ui_event_data_t));
    if (data) {
        data->event = event;
        data->int_param = param;
        idle_queue(ui_event_dispatch, data);
    }
}

/*============================================================================
 * Storage device management
 *==========================================================================*/

static int find_device_by_mount(const char* mp) {
    int i;
    for (i = 0; i < s_app.device_count; i++) {
        if (strcmp(s_app.devices[i].mount_point, mp) == 0) {
            return i;
        }
    }
    return -1;
}

static int add_device(const storage_device_info_t* info) {
    if (s_app.device_count >= MAX_STORAGE_DEVICES) {
        fprintf(stderr, "[music_app] Max storage devices reached\n");
        return -1;
    }

    int idx = s_app.device_count;
    storage_device_state_t* dev = &s_app.devices[idx];

    memset(dev, 0, sizeof(*dev));
    dev->mounted = true;
    dev->type = info->type;
    snprintf(dev->mount_point, sizeof(dev->mount_point), "%s", info->mount_point);
    dev->music_list = music_list_create(MUSIC_MAX_FILES);

    s_app.device_count++;
    return idx;
}

static void remove_device(int idx) {
    if (idx < 0 || idx >= s_app.device_count) return;

    storage_device_state_t* dev = &s_app.devices[idx];
    if (dev->music_list) {
        music_list_destroy(dev->music_list);
        dev->music_list = NULL;
    }

    /* Shift remaining devices down */
    int i;
    for (i = idx; i < s_app.device_count - 1; i++) {
        s_app.devices[i] = s_app.devices[i + 1];
    }
    s_app.device_count--;

    /* Fix current device index */
    if (s_app.state.current_device_idx >= s_app.device_count) {
        s_app.state.current_device_idx = s_app.device_count - 1;
    }
    if (s_app.state.current_device_idx < 0) {
        s_app.state.current_device_idx = 0;
    }
}

/*============================================================================
 * USB monitor callback (called from monitor thread)
 *==========================================================================*/
static void on_storage_event(const storage_device_info_t* info, void* user_data) {
    (void)user_data;

    pthread_mutex_lock(&s_app.mutex);

    if (info->event == STORAGE_EVENT_MOUNTED) {
        int idx = find_device_by_mount(info->mount_point);
        if (idx < 0) {
            idx = add_device(info);
        }
        if (idx >= 0) {
            s_app.devices[idx].mounted = true;
            printf("[music_app] Storage mounted: %s (idx=%d)\n",
                   info->mount_point, idx);

            /* Auto-switch to newly inserted device */
            s_app.state.current_device_idx = idx;

            pthread_mutex_unlock(&s_app.mutex);

            post_ui_event(APP_EVENT_STORAGE_MOUNTED, idx);
            scan_device_async(idx);
            return;
        }
    } else if (info->event == STORAGE_EVENT_UNMOUNTED ||
               info->event == STORAGE_EVENT_EJECT) {
        int idx = find_device_by_mount(info->mount_point);
        if (idx >= 0) {
            printf("[music_app] Storage unmounted: %s (idx=%d)\n",
                   info->mount_point, idx);

            /* [GAP-2] Only stop if current track is ON the unmounted device.
             * Android: checks mCurrentMediaInfo.mFilePath.contains(strPath) */
            if (idx == s_app.state.current_device_idx) {
                bool should_stop = true;
                if (s_app.state.current_info != NULL) {
                    /* Check if the current file path starts with the unmounted mount point */
                    if (strncmp(s_app.state.current_info->filepath,
                                info->mount_point,
                                strlen(info->mount_point)) != 0) {
                        should_stop = false; /* Playing from a different partition */
                    }
                }
                if (should_stop) {
                    pthread_mutex_unlock(&s_app.mutex);
                    music_app_stop();
                    pthread_mutex_lock(&s_app.mutex);
                }
            }

            remove_device(idx);
            pthread_mutex_unlock(&s_app.mutex);

            post_ui_event(APP_EVENT_STORAGE_UNMOUNTED, idx);
            return;
        }
    }

    pthread_mutex_unlock(&s_app.mutex);
}

/*============================================================================
 * Async scanning (runs in a background thread)
 *
 * Mirrors Android RemoteService's file scan + ID3 parse pipeline.
 *==========================================================================*/
typedef struct {
    int dev_idx;
} scan_task_t;

static void* scan_thread_func(void* arg) {
    scan_task_t* task = (scan_task_t*)arg;
    int dev_idx = task->dev_idx;
    free(task);

    pthread_mutex_lock(&s_app.mutex);
    if (dev_idx < 0 || dev_idx >= s_app.device_count) {
        pthread_mutex_unlock(&s_app.mutex);
        return NULL;
    }

    storage_device_state_t* dev = &s_app.devices[dev_idx];
    dev->scanning = true;
    dev->scan_done = false;
    dev->id3_done = false;

    char path[STORAGE_PATH_MAX];
    snprintf(path, sizeof(path), "%s", dev->mount_point);
    MusicList* list = dev->music_list;
    pthread_mutex_unlock(&s_app.mutex);

    post_ui_event(APP_EVENT_SCAN_STARTED, dev_idx);

    printf("[music_app] Scanning %s ...\n", path);
    music_scan_directory(list, path);

    /* Save to cache DB */
    char db_path[MUSIC_MAX_PATH_LEN];
    snprintf(db_path, sizeof(db_path), "%s/music_%d.db", STATE_FILE_DIR, dev_idx);
    mkdir(STATE_FILE_DIR, 0755);
    music_db_save(list, db_path);

    pthread_mutex_lock(&s_app.mutex);
    if (dev_idx < s_app.device_count) {
        dev = &s_app.devices[dev_idx];
        dev->scanning = false;
        dev->scan_done = true;
        dev->id3_done = true; /* ID3 is parsed during scan */
    }
    pthread_mutex_unlock(&s_app.mutex);

    printf("[music_app] Scan complete: %d files in %s\n", list->count, path);

    /* Issue #2: Rebuild folder cache after scan */
    build_folder_cache();

    /* Issue #3: Build album/artist classification */
    build_classification_cache();

    /* Issue #1: Validate favorites against scan result — remove stale entries */
    favorite_validate(list);

    /* Load playlist if this is the current device */
    pthread_mutex_lock(&s_app.mutex);
    bool is_current = (dev_idx == s_app.state.current_device_idx);
    pthread_mutex_unlock(&s_app.mutex);

    if (is_current && list->count > 0 && s_app.player) {
        music_player_set_playlist(s_app.player, list);
        post_ui_event(APP_EVENT_PLAYLIST_CHANGED, dev_idx);

        /* [GAP-12] Try to resume last playing track at remembered position.
         * Android: getLastMediaInfoPosition() + readMediaTime() + seekToTime() */
        if (s_app.state.last_path[0] != '\0') {
            int resume_idx = -1;
            int i;
            for (i = 0; i < list->count; i++) {
                if (strcmp(list->items[i].filepath, s_app.state.last_path) == 0) {
                    resume_idx = i;
                    break;
                }
            }
            if (resume_idx >= 0) {
                printf("[music_app] Resuming: track %d, position %d ms\n",
                       resume_idx, s_app.state.last_position_ms);
                music_player_play(s_app.player, resume_idx);
                if (s_app.state.last_position_ms > 0) {
                    music_player_seek(s_app.player, s_app.state.last_position_ms);
                }
                /* Clear last_path so we don't re-seek on next scan */
                s_app.state.last_path[0] = '\0';
                s_app.state.last_position_ms = 0;
            }
        }
    }

    post_ui_event(APP_EVENT_SCAN_FINISHED, dev_idx);
    return NULL;
}

static void scan_device_async(int dev_idx) {
    scan_task_t* task = (scan_task_t*)calloc(1, sizeof(scan_task_t));
    if (!task) return;

    task->dev_idx = dev_idx;

    pthread_t t;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&t, &attr, scan_thread_func, task);
    pthread_attr_destroy(&attr);
}

/*============================================================================
 * [GAP-9] Delayed auto-next timer callback
 * Android: H0.sendEmptyMessageDelayed(MSG_GOTO_NEXT_MEDIA, 1000)
 *==========================================================================*/
static ret_t delayed_auto_next_cb(const timer_info_t* timer) {
    (void)timer;
    s_app.auto_next_timer_id = TK_INVALID_ID;
    if (s_app.player) {
        music_player_next(s_app.player);
    }
    return RET_REMOVE;
}

/*============================================================================
 * [GAP-11] Periodic state save timer (every 30 seconds)
 * Android: writeCurrentMediaTime called on pause/stop/focus-loss
 *==========================================================================*/
static ret_t periodic_save_cb(const timer_info_t* timer) {
    (void)timer;
    music_app_save_state();
    return RET_REPEAT;
}

/*============================================================================
 * Player callbacks (called from player thread)
 *
 * Issue #15/27 fix: AWTK timer_add/timer_remove are NOT thread-safe.
 * All timer operations must be dispatched to the AWTK main thread via
 * idle_queue(). We post a state-change idle that handles timers + events
 * on the main thread.
 *==========================================================================*/

/* Idle handler: processes player state changes on AWTK main thread */
static ret_t player_state_idle_handler(const idle_info_t* idle) {
    PlayerState state = (PlayerState)(intptr_t)idle->ctx;

    /* [GAP-7] Error handling: auto-skip or rescan */
    if (state == PLAYER_STATE_ERROR) {
        s_app.error_count++;
        if (s_app.error_count >= 3) {
            printf("[music_app] 3 consecutive errors, triggering rescan\n");
            s_app.error_count = 0;
            post_ui_event(APP_EVENT_ERROR, 0);
            /* Cancel pending auto-next (now safe — we're on main thread) */
            if (s_app.auto_next_timer_id != TK_INVALID_ID) {
                timer_remove(s_app.auto_next_timer_id);
                s_app.auto_next_timer_id = TK_INVALID_ID;
            }
            music_app_rescan();
        } else {
            printf("[music_app] Play error, auto-skip to next (err_count=%d)\n",
                   s_app.error_count);
            post_ui_event(APP_EVENT_ERROR, 0);
            if (s_app.auto_next_timer_id != TK_INVALID_ID) {
                timer_remove(s_app.auto_next_timer_id);
                s_app.auto_next_timer_id = TK_INVALID_ID;
            }
            s_app.auto_next_timer_id = timer_add(delayed_auto_next_cb, NULL, 500);
        }
        return RET_REMOVE;
    }

    /* Reset error counter on successful play */
    if (state == PLAYER_STATE_PLAYING) {
        s_app.error_count = 0;
    }

    /* [GAP-8+9] Natural track completion → delayed auto-next */
    if (state == PLAYER_STATE_STOPPED) {
        pthread_mutex_lock(&s_app.mutex);
        s_app.state.last_position_ms = 0;
        s_app.state.current_position_ms = 0;
        pthread_mutex_unlock(&s_app.mutex);

        if (s_app.auto_next_timer_id != TK_INVALID_ID) {
            timer_remove(s_app.auto_next_timer_id);
            s_app.auto_next_timer_id = TK_INVALID_ID;
        }
        s_app.auto_next_timer_id = timer_add(delayed_auto_next_cb, NULL, 1000);
    }

    /* [GAP-11] Save state on pause/stop */
    if (state == PLAYER_STATE_PAUSED || state == PLAYER_STATE_STOPPED) {
        music_app_save_state();
    }

    post_ui_event(APP_EVENT_STATE_CHANGED, (int)state);
    return RET_REMOVE;
}

static void on_player_state(PlayerState state, void* user_data) {
    (void)user_data;
    pthread_mutex_lock(&s_app.mutex);
    s_app.state.player_state = state;
    pthread_mutex_unlock(&s_app.mutex);

    /* Issue #15/27 fix: dispatch all timer/UI work to AWTK main thread.
     * We pass the PlayerState as the ctx pointer (cast to intptr_t). */
    idle_queue(player_state_idle_handler, (void*)(intptr_t)state);
}

static void on_player_track(int index, const MusicInfo* info, void* user_data) {
    (void)user_data;
    pthread_mutex_lock(&s_app.mutex);
    s_app.state.current_info = info;
    pthread_mutex_unlock(&s_app.mutex);

    /* Issue #7,#8: Pre-load album art and lyrics for new track */
    load_album_art_for_current();
    load_lyrics_for_current();

    post_ui_event(APP_EVENT_TRACK_CHANGED, index);
}

static void on_player_position(int position_ms, int duration_ms, void* user_data) {
    (void)user_data;
    pthread_mutex_lock(&s_app.mutex);
    s_app.state.current_position_ms = position_ms;
    if (duration_ms > 0) {
        s_app.state.current_duration_ms = duration_ms;
    }
    pthread_mutex_unlock(&s_app.mutex);
    post_ui_event(APP_EVENT_POSITION_CHANGED, position_ms);
}

/*============================================================================
 * Public API
 *==========================================================================*/

int music_app_init(music_app_ui_callback_t ui_cb) {
    if (s_app.inited) {
        fprintf(stderr, "[music_app] Already initialized\n");
        return -1;
    }

    /* Issue #25 fix: Do NOT memset the entire struct — that would corrupt
     * a previously-destroyed mutex on reinit. Initialize each field explicitly.
     * This also avoids zeroing any field that might be referenced by a
     * lingering thread from a prior deinit cycle. */
    pthread_mutex_init(&s_app.mutex, NULL);
    memset(&s_app.state, 0, sizeof(s_app.state));
    s_app.player = NULL;
    memset(s_app.devices, 0, sizeof(s_app.devices));
    s_app.device_count = 0;
    s_app.ui_cb = ui_cb;
    s_app.state.play_mode = PLAY_MODE_REPEAT_ALL;
    s_app.state.current_device_idx = -1;
    s_app.auto_next_timer_id = TK_INVALID_ID;
    s_app.save_timer_id = TK_INVALID_ID;
    s_app.error_count = 0;
    s_app.last_prev_next_ms = 0;
    s_app.folder_count = 0;
    s_app.album_group_count = 0;
    s_app.artist_group_count = 0;
    s_app.album_art_data = NULL;
    s_app.album_art_size = 0;
    s_app.album_art_path[0] = '\0';
    memset(&s_app.lyrics, 0, sizeof(s_app.lyrics));
    s_app.lyrics_path[0] = '\0';
    /* Issue #23: Initialize playlist type tracking */
    s_app.state.playlist_type = PLAYLIST_TYPE_DEVICE;

    /* Create player */
    s_app.player = music_player_create();
    if (!s_app.player) {
        fprintf(stderr, "[music_app] Failed to create player\n");
        /* Non-fatal: app can still scan and display, just not play */
    } else {
        music_player_set_state_callback(s_app.player, on_player_state, NULL);
        music_player_set_track_callback(s_app.player, on_player_track, NULL);
        music_player_set_position_callback(s_app.player, on_player_position, NULL);
        music_player_set_mode(s_app.player, s_app.state.play_mode);
    }

    /* Start USB monitor */
    usb_monitor_start(on_storage_event, NULL);

    /* Discover already-mounted devices */
    usb_monitor_scan_existing(on_storage_event, NULL);

    /* Initialize favorite manager (Issue #1) */
    favorite_init(NULL);

    /* Restore last play state */
    music_app_restore_state();

    /* [GAP-11] Start periodic state save (every 30 seconds)
     * Android: writeCurrentMediaTime on timer + pause/stop */
    s_app.save_timer_id = timer_add(periodic_save_cb, NULL, 30000);

    s_app.inited = true;
    printf("[music_app] Initialized, %d devices found\n", s_app.device_count);
    return 0;
}

void music_app_deinit(void) {
    if (!s_app.inited) return;

    music_app_save_state();

    /* Clean up timers */
    if (s_app.auto_next_timer_id != TK_INVALID_ID) {
        timer_remove(s_app.auto_next_timer_id);
        s_app.auto_next_timer_id = TK_INVALID_ID;
    }
    if (s_app.save_timer_id != TK_INVALID_ID) {
        timer_remove(s_app.save_timer_id);
        s_app.save_timer_id = TK_INVALID_ID;
    }

    usb_monitor_stop();

    if (s_app.player) {
        music_player_destroy(s_app.player);
        s_app.player = NULL;
    }

    /* Cleanup favorite manager (Issue #1) */
    favorite_deinit();

    /* Cleanup album art cache (Issue #7) */
    if (s_app.album_art_data) {
        free(s_app.album_art_data);
        s_app.album_art_data = NULL;
    }

    /* Cleanup lyrics cache (Issue #8) */
    if (s_app.lyrics.lines) {
        free(s_app.lyrics.lines);
        s_app.lyrics.lines = NULL;
    }

    /* Cleanup folder path cache (Issue #2) */
    {
        int fi;
        for (fi = 0; fi < s_app.folder_count; fi++) {
            free(s_app.folder_paths[fi]);
            s_app.folder_paths[fi] = NULL;
        }
        s_app.folder_count = 0;
    }

    int i;
    for (i = 0; i < s_app.device_count; i++) {
        if (s_app.devices[i].music_list) {
            music_list_destroy(s_app.devices[i].music_list);
        }
    }

    pthread_mutex_destroy(&s_app.mutex);
    memset(&s_app, 0, sizeof(s_app));

    printf("[music_app] Deinitialized\n");
}

const music_app_state_t* music_app_get_state(void) {
    return &s_app.state;
}

void music_app_play(int index) {
    if (s_app.player) {
        music_player_play(s_app.player, index);
    }
}

void music_app_pause(void) {
    if (s_app.player) {
        music_player_pause(s_app.player);
    }
}

void music_app_resume(void) {
    if (s_app.player) {
        music_player_resume(s_app.player);
    }
}

void music_app_stop(void) {
    if (s_app.player) {
        music_player_stop(s_app.player);
    }
}

/* [GAP-6] Prev/next debounce — Android ControlHandler 500ms filter
 * Vehicle rotary knobs can fire 5-10 events in 100ms */
#define PREV_NEXT_DEBOUNCE_MS 500

void music_app_next(void) {
    uint64_t now = timer_manager()->get_elapsed_ms
        ? timer_manager()->get_elapsed_ms(timer_manager()) : 0;
    if (now > 0 && (now - s_app.last_prev_next_ms) < PREV_NEXT_DEBOUNCE_MS) {
        return; /* Debounce: ignore rapid repeated presses */
    }
    s_app.last_prev_next_ms = now;
    if (s_app.player) {
        music_player_next(s_app.player);
    }
}

void music_app_prev(void) {
    uint64_t now = timer_manager()->get_elapsed_ms
        ? timer_manager()->get_elapsed_ms(timer_manager()) : 0;
    if (now > 0 && (now - s_app.last_prev_next_ms) < PREV_NEXT_DEBOUNCE_MS) {
        return;
    }
    s_app.last_prev_next_ms = now;
    if (s_app.player) {
        music_player_prev(s_app.player);
    }
}

void music_app_seek(int position_ms) {
    if (s_app.player) {
        music_player_seek(s_app.player, position_ms);
    }
}

void music_app_toggle_play_pause(void) {
    if (!s_app.player) return;
    PlayerState st = music_player_get_state(s_app.player);
    if (st == PLAYER_STATE_PLAYING) {
        music_app_pause();
    } else if (st == PLAYER_STATE_PAUSED) {
        music_app_resume();
    } else {
        music_app_play(-1);
    }
}

void music_app_set_play_mode(PlayMode mode) {
    pthread_mutex_lock(&s_app.mutex);
    s_app.state.play_mode = mode;
    pthread_mutex_unlock(&s_app.mutex);
    if (s_app.player) {
        music_player_set_mode(s_app.player, mode);
    }
}

void music_app_cycle_play_mode(void) {
    PlayMode cur = s_app.state.play_mode;
    PlayMode next = (PlayMode)((cur + 1) % 4);
    music_app_set_play_mode(next);
}

int music_app_get_playlist_count(void) {
    return s_app.player ? music_player_get_playlist_count(s_app.player) : 0;
}

const MusicInfo* music_app_get_track_info(int index) {
    return s_app.player ? music_player_get_track_info(s_app.player, index) : NULL;
}

int music_app_get_current_index(void) {
    return s_app.player ? music_player_get_current_index(s_app.player) : -1;
}

int music_app_get_device_count(void) {
    return s_app.device_count;
}

const storage_device_state_t* music_app_get_device(int idx) {
    if (idx >= 0 && idx < s_app.device_count) {
        return &s_app.devices[idx];
    }
    return NULL;
}

void music_app_switch_device(int idx) {
    if (idx < 0 || idx >= s_app.device_count) return;

    pthread_mutex_lock(&s_app.mutex);
    s_app.state.current_device_idx = idx;
    storage_device_state_t* dev = &s_app.devices[idx];
    pthread_mutex_unlock(&s_app.mutex);

    /* Stop current playback */
    music_app_stop();

    /* If scan done, load playlist */
    if (dev->scan_done && dev->music_list && dev->music_list->count > 0) {
        music_player_set_playlist(s_app.player, dev->music_list);
        post_ui_event(APP_EVENT_PLAYLIST_CHANGED, idx);
    } else if (!dev->scanning) {
        /* Trigger scan if not already running */
        scan_device_async(idx);
    }
}

void music_app_rescan(void) {
    int idx = s_app.state.current_device_idx;
    if (idx >= 0 && idx < s_app.device_count) {
        music_app_stop();
        scan_device_async(idx);
    }
}

/*============================================================================
 * State persistence (mirrors Android Preferences read/write)
 *==========================================================================*/

void music_app_save_state(void) {
    mkdir(STATE_FILE_DIR, 0755);
    FILE* fp = fopen(STATE_FILE_PATH, "w");
    if (!fp) return;

    pthread_mutex_lock(&s_app.mutex);
    fprintf(fp, "play_mode=%d\n", (int)s_app.state.play_mode);
    fprintf(fp, "device_idx=%d\n", s_app.state.current_device_idx);

    if (s_app.state.current_info) {
        fprintf(fp, "last_path=%s\n", s_app.state.current_info->filepath);
        fprintf(fp, "last_position=%d\n", s_app.state.current_position_ms);
    } else if (s_app.state.last_path[0]) {
        fprintf(fp, "last_path=%s\n", s_app.state.last_path);
        fprintf(fp, "last_position=%d\n", s_app.state.last_position_ms);
    }
    pthread_mutex_unlock(&s_app.mutex);

    fclose(fp);
    printf("[music_app] State saved\n");
}

void music_app_restore_state(void) {
    FILE* fp = fopen(STATE_FILE_PATH, "r");
    if (!fp) return;

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        char* nl = strchr(line, '\n');
        if (nl) *nl = '\0';

        if (strncmp(line, "play_mode=", 10) == 0) {
            int m = atoi(line + 10);
            if (m >= 0 && m <= 3) {
                s_app.state.play_mode = (PlayMode)m;
                if (s_app.player) {
                    music_player_set_mode(s_app.player, s_app.state.play_mode);
                }
            }
        } else if (strncmp(line, "device_idx=", 11) == 0) {
            /* device_idx is informational; actual device is discovered at runtime */
        } else if (strncmp(line, "last_path=", 10) == 0) {
            snprintf(s_app.state.last_path, sizeof(s_app.state.last_path),
                     "%s", line + 10);
        } else if (strncmp(line, "last_position=", 14) == 0) {
            s_app.state.last_position_ms = atoi(line + 14);
        }
    }
    fclose(fp);
    printf("[music_app] State restored: mode=%d last=%s\n",
           s_app.state.play_mode, s_app.state.last_path);
}

/*============================================================================
 * Issue #1: Favorite management API
 * Mirrors Android FavoriteManager add/remove/toggle
 *==========================================================================*/

bool music_app_toggle_favorite(void) {
    const music_app_state_t* st = music_app_get_state();
    if (!st->current_info) return false;

    bool result = favorite_toggle(st->current_info);
    post_ui_event(APP_EVENT_FAVORITE_CHANGED, result ? 1 : 0);
    favorite_save();
    return result;
}

bool music_app_is_favorite(void) {
    const music_app_state_t* st = music_app_get_state();
    if (!st->current_info) return false;
    return favorite_contains(st->current_info->filepath);
}

int music_app_get_favorite_count(void) {
    int count = 0;
    favorite_get_list(&count);
    return count;
}

const MusicInfo* music_app_get_favorite_list(int* out_count) {
    return favorite_get_list(out_count);
}

/*============================================================================
 * Issue #2: Folder browsing API
 * Mirrors Android FolderListLayout + MediaFilePathScan
 *==========================================================================*/

/**
 * Build unique folder list from the current device's scanned files.
 * Extracts dirname(filepath) for each file and deduplicates.
 */
static void build_folder_cache(void) {
    /* Issue #14 fix: protect shared folder cache with mutex since this
     * runs on scan thread while UI thread may call get_folder_list(). */
    pthread_mutex_lock(&s_app.mutex);

    /* Free old cache */
    int i;
    for (i = 0; i < s_app.folder_count; i++) {
        free(s_app.folder_paths[i]);
        s_app.folder_paths[i] = NULL;
    }
    s_app.folder_count = 0;

    int dev_idx = s_app.state.current_device_idx;
    if (dev_idx < 0 || dev_idx >= s_app.device_count) {
        pthread_mutex_unlock(&s_app.mutex);
        return;
    }

    storage_device_state_t* dev = &s_app.devices[dev_idx];
    if (!dev->music_list) {
        pthread_mutex_unlock(&s_app.mutex);
        return;
    }

    MusicList* list = dev->music_list;
    for (i = 0; i < list->count && s_app.folder_count < MUSIC_MAX_FILES; i++) {
        /* Extract directory from filepath */
        char dir[MUSIC_MAX_PATH_LEN];
        snprintf(dir, sizeof(dir), "%s", list->items[i].filepath);
        char* slash = strrchr(dir, '/');
        if (slash && slash != dir) {
            *slash = '\0';
        } else {
            continue;
        }

        /* Also populate folder_path in the MusicInfo */
        snprintf(list->items[i].folder_path, sizeof(list->items[i].folder_path),
                 "%s", dir);
        list->items[i].folder_index = i; /* file entry, not a folder */

        /* Check for duplicate */
        bool exists = false;
        int j;
        for (j = 0; j < s_app.folder_count; j++) {
            if (strcmp(s_app.folder_paths[j], dir) == 0) {
                exists = true;
                break;
            }
        }

        if (!exists) {
            s_app.folder_paths[s_app.folder_count] = strdup(dir);
            s_app.folder_count++;
        }
    }

    printf("[music_app] Built folder cache: %d unique folders\n", s_app.folder_count);
    pthread_mutex_unlock(&s_app.mutex);
}

void music_app_get_folder_list(const char*** out_folders, int* out_count) {
    if (s_app.folder_count == 0) {
        build_folder_cache();
    }
    if (out_folders) *out_folders = (const char**)s_app.folder_paths;
    if (out_count) *out_count = s_app.folder_count;
}

void music_app_play_folder(const char* folder_path) {
    if (!folder_path || !s_app.player) return;

    int dev_idx = s_app.state.current_device_idx;
    if (dev_idx < 0 || dev_idx >= s_app.device_count) return;

    storage_device_state_t* dev = &s_app.devices[dev_idx];
    if (!dev->music_list) return;

    /* Issue #16 fix: Build a sub-playlist containing only songs in this folder,
     * then set it as the player's current playlist. This way next/prev stays
     * within the folder. Mirrors Android updatePlaylist(FOLDER_LIST, list). */
    MusicList* src = dev->music_list;
    int folder_len = (int)strlen(folder_path);

    /* Create temporary list for folder songs */
    MusicList* folder_list = music_list_create(src->count);
    if (!folder_list) return;

    int i;
    for (i = 0; i < src->count; i++) {
        if (strncmp(src->items[i].filepath, folder_path, folder_len) == 0
            && src->items[i].filepath[folder_len] == '/') {
            const char* rest = src->items[i].filepath + folder_len + 1;
            if (strchr(rest, '/') == NULL) {
                /* Direct child of this folder */
                if (folder_list->count < folder_list->capacity) {
                    folder_list->items[folder_list->count] = src->items[i];
                    folder_list->count++;
                }
            }
        }
    }

    if (folder_list->count == 0) {
        printf("[music_app] No files found in folder: %s\n", folder_path);
        music_list_destroy(folder_list);
        return;
    }

    /* Set the folder sub-list as the player's playlist */
    music_player_set_playlist(s_app.player, folder_list);

    /* Issue #23: Track playlist type */
    pthread_mutex_lock(&s_app.mutex);
    s_app.state.playlist_type = PLAYLIST_TYPE_FOLDER;
    pthread_mutex_unlock(&s_app.mutex);

    post_ui_event(APP_EVENT_PLAYLIST_CHANGED, dev_idx);

    /* Start playing the first song */
    music_app_play(0);

    /* folder_list data was copied by set_playlist; we can destroy the container */
    music_list_destroy(folder_list);
}

/*============================================================================
 * Issue #6: Search API
 * Mirrors Android MusicSearchFragment keyword matching
 *==========================================================================*/

/**
 * Case-insensitive substring search (portable).
 */
static bool str_contains_ci(const char* haystack, const char* needle) {
    if (!haystack || !needle) return false;
    if (needle[0] == '\0') return true;

    int hlen = (int)strlen(haystack);
    int nlen = (int)strlen(needle);
    if (nlen > hlen) return false;

    int i, j;
    for (i = 0; i <= hlen - nlen; i++) {
        bool match = true;
        for (j = 0; j < nlen; j++) {
            if (tolower((unsigned char)haystack[i + j]) !=
                tolower((unsigned char)needle[j])) {
                match = false;
                break;
            }
        }
        if (match) return true;
    }
    return false;
}

int music_app_search(const char* keyword, const MusicInfo** results, int max_results) {
    if (!keyword || !results || max_results <= 0) return 0;

    int dev_idx = s_app.state.current_device_idx;
    if (dev_idx < 0 || dev_idx >= s_app.device_count) return 0;

    storage_device_state_t* dev = &s_app.devices[dev_idx];
    if (!dev->music_list) return 0;

    MusicList* list = dev->music_list;
    int found = 0;
    int i;
    for (i = 0; i < list->count && found < max_results; i++) {
        const MusicInfo* info = &list->items[i];
        if (str_contains_ci(info->title, keyword) ||
            str_contains_ci(info->artist, keyword) ||
            str_contains_ci(info->album, keyword) ||
            str_contains_ci(info->filename, keyword)) {
            results[found++] = info;
        }
    }
    return found;
}

/*============================================================================
 * Issue #3: Album/Artist classification
 * Mirrors Android MediaService.classifyMediaInfoList()
 *==========================================================================*/

/**
 * Find or create a group by key in the given group array.
 */
static music_group_t* find_or_create_group(music_group_t* groups, int* count,
                                           const char* key, int max_groups) {
    int i;
    /* Empty key → use placeholder */
    const char* effective_key = (key && key[0]) ? key : "Unknown";

    for (i = 0; i < *count; i++) {
        if (strcmp(groups[i].key, effective_key) == 0) {
            return &groups[i];
        }
    }

    if (*count >= max_groups) return NULL;

    music_group_t* g = &groups[*count];
    memset(g, 0, sizeof(*g));
    snprintf(g->key, sizeof(g->key), "%s", effective_key);
    g->count = 0;
    (*count)++;
    return g;
}

static void build_classification_cache(void) {
    /* Issue #14 fix: protect shared classification cache with mutex since
     * this runs on scan thread while UI thread may call get_album/artist_list(). */
    pthread_mutex_lock(&s_app.mutex);

    s_app.album_group_count = 0;
    s_app.artist_group_count = 0;

    int dev_idx = s_app.state.current_device_idx;
    if (dev_idx < 0 || dev_idx >= s_app.device_count) {
        pthread_mutex_unlock(&s_app.mutex);
        return;
    }

    storage_device_state_t* dev = &s_app.devices[dev_idx];
    if (!dev->music_list) {
        pthread_mutex_unlock(&s_app.mutex);
        return;
    }

    MusicList* list = dev->music_list;
    int i;
    for (i = 0; i < list->count; i++) {
        const MusicInfo* info = &list->items[i];

        /* Album grouping */
        music_group_t* ag = find_or_create_group(
            s_app.album_groups, &s_app.album_group_count,
            info->album, MUSIC_MAX_GROUPS);
        if (ag && ag->count < MUSIC_MAX_FILES) {
            ag->items[ag->count++] = info;
        }

        /* Artist grouping */
        music_group_t* rg = find_or_create_group(
            s_app.artist_groups, &s_app.artist_group_count,
            info->artist, MUSIC_MAX_GROUPS);
        if (rg && rg->count < MUSIC_MAX_FILES) {
            rg->items[rg->count++] = info;
        }
    }

    printf("[music_app] Classification: %d albums, %d artists\n",
           s_app.album_group_count, s_app.artist_group_count);
    pthread_mutex_unlock(&s_app.mutex);
}

void music_app_get_album_list(const music_group_t** out_groups, int* out_count) {
    if (s_app.album_group_count == 0) {
        build_classification_cache();
    }
    if (out_groups) *out_groups = s_app.album_groups;
    if (out_count) *out_count = s_app.album_group_count;
}

void music_app_get_artist_list(const music_group_t** out_groups, int* out_count) {
    if (s_app.artist_group_count == 0) {
        build_classification_cache();
    }
    if (out_groups) *out_groups = s_app.artist_groups;
    if (out_count) *out_count = s_app.artist_group_count;
}

void music_app_play_group(const music_group_t* group, int index) {
    if (!group || index < 0 || index >= group->count) return;
    if (!s_app.player) return;

    /* Issue #17 fix: Build a sub-playlist from the group's items, then set it
     * as the player's playlist. This way next/prev stays within the album/artist.
     * Mirrors Android behavior where clicking a song in an album plays that
     * album as the current playlist. */
    MusicList* group_list = music_list_create(group->count);
    if (!group_list) return;

    int i;
    for (i = 0; i < group->count; i++) {
        if (group->items[i] && group_list->count < group_list->capacity) {
            group_list->items[group_list->count] = *(group->items[i]);
            group_list->count++;
        }
    }

    if (group_list->count == 0) {
        music_list_destroy(group_list);
        return;
    }

    music_player_set_playlist(s_app.player, group_list);

    /* Issue #23: Determine playlist type from group context.
     * Caller knows whether this is album or artist, but we infer from
     * album_groups vs artist_groups membership. Default to ALBUM. */
    pthread_mutex_lock(&s_app.mutex);
    /* Check if this group is in the artist_groups array */
    bool is_artist = false;
    for (i = 0; i < s_app.artist_group_count; i++) {
        if (&s_app.artist_groups[i] == group) {
            is_artist = true;
            break;
        }
    }
    s_app.state.playlist_type = is_artist ? PLAYLIST_TYPE_ARTIST : PLAYLIST_TYPE_ALBUM;
    pthread_mutex_unlock(&s_app.mutex);

    int dev_idx = s_app.state.current_device_idx;
    post_ui_event(APP_EVENT_PLAYLIST_CHANGED, dev_idx);

    /* Play the selected track within the group */
    music_app_play(index);

    music_list_destroy(group_list);
}

/*============================================================================
 * Issue #8: LRC lyrics parser
 * Mirrors Android LyricsManager + LyricsRow
 *
 * LRC format: [mm:ss.xx] lyrics text
 * Example:    [00:12.50] Hello world
 *==========================================================================*/

/**
 * Parse a single LRC time tag "[mm:ss.xx]" and return milliseconds.
 * Returns -1 on parse failure.
 */
static int parse_lrc_time(const char* tag) {
    /* tag points to the character after '[' */
    int min = 0, sec = 0, ms = 0;

    /* Try [mm:ss.xx] or [mm:ss.xxx] or [mm:ss:xx] */
    if (sscanf(tag, "%d:%d.%d", &min, &sec, &ms) >= 2) {
        /* ms might be 2 or 3 digits; normalize to milliseconds */
        if (ms < 100) ms *= 10; /* e.g. ".05" → 50ms, ".5" → 500ms */
        return min * 60000 + sec * 1000 + ms;
    }
    if (sscanf(tag, "%d:%d:%d", &min, &sec, &ms) >= 2) {
        if (ms < 100) ms *= 10;
        return min * 60000 + sec * 1000 + ms;
    }
    return -1;
}

static void lrc_data_clear(lrc_data_t* lrc) {
    if (lrc->lines) {
        free(lrc->lines);
        lrc->lines = NULL;
    }
    lrc->count = 0;
    lrc->capacity = 0;
}

static void lrc_data_add(lrc_data_t* lrc, int time_ms, const char* text) {
    if (lrc->count >= lrc->capacity) {
        int new_cap = lrc->capacity == 0 ? 64 : lrc->capacity * 2;
        lrc_line_t* new_lines = (lrc_line_t*)realloc(lrc->lines,
                                                      new_cap * sizeof(lrc_line_t));
        if (!new_lines) return;
        lrc->lines = new_lines;
        lrc->capacity = new_cap;
    }

    lrc_line_t* line = &lrc->lines[lrc->count];
    line->time_ms = time_ms;
    snprintf(line->text, sizeof(line->text), "%s", text ? text : "");
    lrc->count++;
}

/* Sort comparator for lrc lines by time */
static int lrc_cmp(const void* a, const void* b) {
    const lrc_line_t* la = (const lrc_line_t*)a;
    const lrc_line_t* lb = (const lrc_line_t*)b;
    return la->time_ms - lb->time_ms;
}

/**
 * Parse a .lrc file into lrc_data_t.
 * Handles multiple time tags per line: [00:01.00][00:05.00] text
 */
static int parse_lrc_file(const char* lrc_path, lrc_data_t* out) {
    FILE* fp = fopen(lrc_path, "r");
    if (!fp) return -1;

    lrc_data_clear(out);

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        char* nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        char* cr = strchr(line, '\r');
        if (cr) *cr = '\0';

        if (line[0] == '\0') continue;

        /* Extract all time tags and the text content */
        const char* p = line;
        int times[16];
        int time_count = 0;

        while (*p == '[' && time_count < 16) {
            const char* close = strchr(p, ']');
            if (!close) break;

            char tag[32];
            int tag_len = (int)(close - p - 1);
            if (tag_len <= 0 || tag_len >= (int)sizeof(tag)) {
                /* Skip non-time tags like [ti:Title] [ar:Artist] */
                p = close + 1;
                continue;
            }

            memcpy(tag, p + 1, tag_len);
            tag[tag_len] = '\0';

            int t = parse_lrc_time(tag);
            if (t >= 0) {
                times[time_count++] = t;
            }

            p = close + 1;
        }

        /* p now points to the lyrics text */
        const char* text = p;

        /* Add an entry for each time tag */
        int ti;
        for (ti = 0; ti < time_count; ti++) {
            lrc_data_add(out, times[ti], text);
        }
    }

    fclose(fp);

    /* Sort by time */
    if (out->count > 1) {
        qsort(out->lines, out->count, sizeof(lrc_line_t), lrc_cmp);
    }

    printf("[music_app] Parsed LRC: %d lines from %s\n", out->count, lrc_path);
    return out->count > 0 ? 0 : -1;
}

static void load_lyrics_for_current(void) {
    const music_app_state_t* st = music_app_get_state();
    if (!st->current_info) return;

    const char* filepath = st->current_info->filepath;

    /* Already loaded for this track? */
    if (strcmp(s_app.lyrics_path, filepath) == 0 && s_app.lyrics.count > 0) {
        return;
    }

    lrc_data_clear(&s_app.lyrics);
    s_app.lyrics_path[0] = '\0';

    /* Build .lrc path: replace extension with .lrc */
    char lrc_path[MUSIC_MAX_PATH_LEN];
    snprintf(lrc_path, sizeof(lrc_path), "%s", filepath);
    char* dot = strrchr(lrc_path, '.');
    if (dot) {
        strcpy(dot, ".lrc");
    } else {
        snprintf(lrc_path + strlen(lrc_path),
                 sizeof(lrc_path) - strlen(lrc_path), ".lrc");
    }

    /* Try to parse */
    if (parse_lrc_file(lrc_path, &s_app.lyrics) == 0) {
        snprintf(s_app.lyrics_path, sizeof(s_app.lyrics_path), "%s", filepath);
    }
}

const lrc_data_t* music_app_get_lyrics(void) {
    load_lyrics_for_current();
    return s_app.lyrics.count > 0 ? &s_app.lyrics : NULL;
}

int music_app_get_lyrics_line(int time_ms) {
    if (s_app.lyrics.count <= 0) return -1;

    /* Issue #24 fix: if time_ms is before the first lyric line (e.g. during
     * intro/prelude), return -1 to indicate "no lyric line active yet".
     * Android LyricsManager does not highlight any line during the intro. */
    if (time_ms < s_app.lyrics.lines[0].time_ms) return -1;

    /* Binary search for the last line whose time <= time_ms */
    int lo = 0, hi = s_app.lyrics.count - 1, result = 0;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (s_app.lyrics.lines[mid].time_ms <= time_ms) {
            result = mid;
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    return result;
}

/*============================================================================
 * Issue #7: Album art (APIC frame) extraction from ID3v2
 * Mirrors Android BitmapCache.loadNativeImage()
 *
 * ID3v2 APIC frame layout:
 *   [1 byte]  encoding
 *   [string]  MIME type (null-terminated)
 *   [1 byte]  picture type (03 = cover front)
 *   [string]  description (null-terminated)
 *   [data]    image data (JPEG or PNG)
 *==========================================================================*/

static uint32_t read_be32_art(const uint8_t* p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  | (uint32_t)p[3];
}

static uint32_t read_synchsafe_art(const uint8_t* p) {
    return ((uint32_t)(p[0] & 0x7F) << 21) |
           ((uint32_t)(p[1] & 0x7F) << 14) |
           ((uint32_t)(p[2] & 0x7F) << 7)  |
           (uint32_t)(p[3] & 0x7F);
}

static int extract_apic_from_file(const char* filepath,
                                  uint8_t** out_data, int* out_size) {
    FILE* fp = fopen(filepath, "rb");
    if (!fp) return -1;

    uint8_t header[10];
    if (fread(header, 1, 10, fp) != 10) { fclose(fp); return -1; }

    if (header[0] != 'I' || header[1] != 'D' || header[2] != '3') {
        fclose(fp);
        return -1;
    }

    uint8_t version_major = header[3];
    uint32_t tag_size = read_synchsafe_art(&header[6]);

    if (tag_size == 0 || tag_size > 10 * 1024 * 1024) {
        fclose(fp);
        return -1;
    }

    uint8_t* tag_data = (uint8_t*)malloc(tag_size);
    if (!tag_data) { fclose(fp); return -1; }

    if (fread(tag_data, 1, tag_size, fp) != tag_size) {
        free(tag_data);
        fclose(fp);
        return -1;
    }
    fclose(fp);

    /* Scan for APIC frame */
    uint32_t pos = 0;
    while (pos + 10 <= tag_size) {
        char frame_id[5];
        memcpy(frame_id, &tag_data[pos], 4);
        frame_id[4] = '\0';

        if (!isupper((unsigned char)frame_id[0]) &&
            !isdigit((unsigned char)frame_id[0])) break;

        uint32_t frame_size;
        if (version_major >= 4) {
            frame_size = read_synchsafe_art(&tag_data[pos + 4]);
        } else {
            frame_size = read_be32_art(&tag_data[pos + 4]);
        }

        pos += 10;
        if (frame_size == 0 || pos + frame_size > tag_size) break;

        if (strcmp(frame_id, "APIC") == 0) {
            /* Parse APIC frame */
            uint32_t fpos = pos;
            /* uint8_t encoding = tag_data[fpos]; */
            fpos++; /* skip encoding byte */

            /* Skip MIME type (null-terminated string) */
            while (fpos < pos + frame_size && tag_data[fpos] != 0) fpos++;
            fpos++; /* skip null terminator */

            /* Skip picture type byte */
            fpos++;

            /* Skip description (null-terminated) */
            while (fpos < pos + frame_size && tag_data[fpos] != 0) fpos++;
            fpos++; /* skip null terminator */

            /* Remaining is image data */
            int img_size = (int)(pos + frame_size - fpos);
            if (img_size > 0) {
                *out_data = (uint8_t*)malloc(img_size);
                if (*out_data) {
                    memcpy(*out_data, &tag_data[fpos], img_size);
                    *out_size = img_size;
                    free(tag_data);
                    return 0;
                }
            }
        }

        pos += frame_size;
    }

    free(tag_data);
    return -1;
}

static void load_album_art_for_current(void) {
    const music_app_state_t* st = music_app_get_state();
    if (!st->current_info) return;

    const char* filepath = st->current_info->filepath;

    /* Already loaded for this track? */
    if (strcmp(s_app.album_art_path, filepath) == 0) {
        return;
    }

    /* Free old data */
    if (s_app.album_art_data) {
        free(s_app.album_art_data);
        s_app.album_art_data = NULL;
        s_app.album_art_size = 0;
    }
    s_app.album_art_path[0] = '\0';

    uint8_t* data = NULL;
    int size = 0;
    if (extract_apic_from_file(filepath, &data, &size) == 0) {
        s_app.album_art_data = data;
        s_app.album_art_size = size;
        snprintf(s_app.album_art_path, sizeof(s_app.album_art_path),
                 "%s", filepath);
        printf("[music_app] Album art: %d bytes from %s\n", size, filepath);
    }
}

int music_app_get_album_art(const uint8_t** out_data, int* out_size) {
    load_album_art_for_current();
    if (s_app.album_art_data && s_app.album_art_size > 0) {
        if (out_data) *out_data = s_app.album_art_data;
        if (out_size) *out_size = s_app.album_art_size;
        return 0;
    }
    return -1;
}
