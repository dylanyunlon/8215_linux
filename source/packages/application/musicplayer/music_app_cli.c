/**
 * @file music_app_cli.c
 * @brief CLI test harness for the full music_app layer.
 *
 * Exposes ALL music_app.h interfaces as interactive CLI commands.
 * Does NOT require AWTK — uses awtk_stub.h for timer/idle simulation.
 *
 * Usage on the dev board:
 *   adb shell
 *   music_app_cli                # auto-detects USB/SD mounts
 *   music_app_cli /mnt/usb0     # manual scan path
 *
 * Commands:
 *   --- Playback ---
 *   play [N]        Play track N (1-based), or resume
 *   pause           Pause
 *   resume          Resume
 *   stop            Stop
 *   next            Next track
 *   prev            Previous track
 *   seek <ms>       Seek to position
 *   toggle          Toggle play/pause
 *
 *   --- Play Mode ---
 *   mode            Cycle play mode
 *   mode <0-3>      Set: 0=seq 1=repeat-all 2=repeat-one 3=shuffle
 *
 *   --- Playlist ---
 *   list            List current playlist (first 50)
 *   info            Show current track info
 *   count           Show playlist count
 *
 *   --- Storage Devices ---
 *   devices         List detected storage devices
 *   switch <N>      Switch to device N (0-based)
 *   rescan          Re-scan current device
 *
 *   --- Folders ---
 *   folders         List unique folders
 *   playfolder <N>  Play from folder N (0-based)
 *
 *   --- Album / Artist ---
 *   albums          List album groups
 *   artists         List artist groups
 *   playalbum <G> <T>  Play track T in album group G
 *   playartist <G> <T> Play track T in artist group G
 *
 *   --- Favorites ---
 *   fav             Toggle favorite for current track
 *   isfav           Check if current track is favorite
 *   favlist         List all favorites
 *   favcount        Show favorite count
 *
 *   --- Search ---
 *   search <kw>     Search tracks by keyword
 *
 *   --- Lyrics ---
 *   lyrics          Show lyrics for current track
 *   lrcline <ms>    Get lyrics line at position
 *
 *   --- Album Art ---
 *   art             Check album art availability
 *
 *   --- State ---
 *   state           Show full app state
 *   save            Save state to disk
 *   restore         Restore state from disk
 *   accoff          Simulate ACC OFF
 *   accon           Simulate ACC ON
 *   restorelist     Restore full device playlist
 *
 *   --- System ---
 *   help            Show this help
 *   quit / q        Exit
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "music_app.h"

/*============================================================================
 * UI event callback — prints events to stdout
 *==========================================================================*/
static const char* event_name(music_app_event_t ev) {
    switch (ev) {
        case APP_EVENT_STORAGE_MOUNTED:   return "STORAGE_MOUNTED";
        case APP_EVENT_STORAGE_UNMOUNTED: return "STORAGE_UNMOUNTED";
        case APP_EVENT_SCAN_STARTED:      return "SCAN_STARTED";
        case APP_EVENT_SCAN_FINISHED:     return "SCAN_FINISHED";
        case APP_EVENT_TRACK_CHANGED:     return "TRACK_CHANGED";
        case APP_EVENT_STATE_CHANGED:     return "STATE_CHANGED";
        case APP_EVENT_POSITION_CHANGED:  return "POSITION_CHANGED";
        case APP_EVENT_PLAYLIST_CHANGED:  return "PLAYLIST_CHANGED";
        case APP_EVENT_ERROR:             return "ERROR";
        case APP_EVENT_FAVORITE_CHANGED:  return "FAVORITE_CHANGED";
        default:                          return "UNKNOWN";
    }
}

static const char* state_name(PlayerState ps) {
    switch (ps) {
        case PLAYER_STATE_IDLE:    return "IDLE";
        case PLAYER_STATE_PLAYING: return "PLAYING";
        case PLAYER_STATE_PAUSED:  return "PAUSED";
        case PLAYER_STATE_STOPPED: return "STOPPED";
        case PLAYER_STATE_ERROR:   return "ERROR";
        default:                   return "UNKNOWN";
    }
}

static const char* mode_name(PlayMode m) {
    switch (m) {
        case PLAY_MODE_SEQUENTIAL: return "Sequential";
        case PLAY_MODE_REPEAT_ALL: return "Repeat-All";
        case PLAY_MODE_REPEAT_ONE: return "Repeat-One";
        case PLAY_MODE_SHUFFLE:    return "Shuffle";
        default:                   return "Unknown";
    }
}

static const char* playlist_type_name(playlist_type_t t) {
    switch (t) {
        case PLAYLIST_TYPE_DEVICE:   return "DEVICE (all)";
        case PLAYLIST_TYPE_FOLDER:   return "FOLDER";
        case PLAYLIST_TYPE_FAVORITE: return "FAVORITE";
        case PLAYLIST_TYPE_ALBUM:    return "ALBUM";
        case PLAYLIST_TYPE_ARTIST:   return "ARTIST";
        default:                     return "UNKNOWN";
    }
}

static void format_time(char* buf, int buf_len, int ms) {
    if (ms < 0) ms = 0;
    int sec = ms / 1000;
    int min = sec / 60;
    sec %= 60;
    snprintf(buf, buf_len, "%02d:%02d", min, sec);
}

/* Suppress position_changed spam — only print every ~5 seconds */
static int s_last_printed_pos_sec = -1;

static void ui_callback(music_app_event_t event, void* param) {
    int int_param = param ? *(int*)param : 0;

    if (event == APP_EVENT_POSITION_CHANGED) {
        /* Only print position every 5 seconds to avoid spam */
        int cur_sec = int_param / 5000;
        if (cur_sec == s_last_printed_pos_sec) return;
        s_last_printed_pos_sec = cur_sec;

        char buf[16];
        format_time(buf, sizeof(buf), int_param);
        const music_app_state_t* st = music_app_get_state();
        char dur_buf[16];
        format_time(dur_buf, sizeof(dur_buf), st->current_duration_ms);
        printf("\r  >> Position: %s / %s", buf, dur_buf);
        fflush(stdout);
        return;
    }

    printf("\n  [EVENT] %s", event_name(event));

    switch (event) {
        case APP_EVENT_TRACK_CHANGED: {
            const music_app_state_t* st = music_app_get_state();
            if (st->current_info) {
                printf(" → [%d/%d] %s - %s",
                       music_app_get_current_index() + 1,
                       music_app_get_playlist_count(),
                       st->current_info->artist,
                       st->current_info->title);
            }
            break;
        }
        case APP_EVENT_STATE_CHANGED:
            printf(" → %s", state_name((PlayerState)int_param));
            break;
        case APP_EVENT_SCAN_FINISHED:
            printf(" → %d tracks", music_app_get_playlist_count());
            break;
        case APP_EVENT_STORAGE_MOUNTED:
        case APP_EVENT_STORAGE_UNMOUNTED: {
            const storage_device_state_t* dev = music_app_get_device(int_param);
            if (dev) printf(" → %s", dev->mount_point);
            break;
        }
        default:
            if (int_param) printf(" (%d)", int_param);
            break;
    }
    printf("\n");
    fflush(stdout);
}

/*============================================================================
 * Helpers
 *==========================================================================*/

static void cmd_help(void) {
    printf("\n");
    printf("=== Music App CLI Test Tool ===\n");
    printf("\n");
    printf("--- Playback ---\n");
    printf("  play [N]        Play track N (1-based), or resume\n");
    printf("  pause           Pause\n");
    printf("  resume          Resume\n");
    printf("  stop            Stop\n");
    printf("  next / n        Next track\n");
    printf("  prev / b        Previous track\n");
    printf("  seek <ms>       Seek to position (milliseconds)\n");
    printf("  toggle          Toggle play/pause\n");
    printf("\n");
    printf("--- Play Mode ---\n");
    printf("  mode            Cycle play mode\n");
    printf("  mode <0-3>      Set: 0=seq 1=repeat-all 2=repeat-one 3=shuffle\n");
    printf("\n");
    printf("--- Playlist ---\n");
    printf("  list [N]        List playlist (default first 50, or N items)\n");
    printf("  info / i        Current track info\n");
    printf("  count           Playlist count\n");
    printf("\n");
    printf("--- Storage ---\n");
    printf("  devices         List detected storage devices\n");
    printf("  switch <N>      Switch to device N (0-based)\n");
    printf("  rescan          Re-scan current device\n");
    printf("\n");
    printf("--- Folders ---\n");
    printf("  folders         List unique folders\n");
    printf("  playfolder <N>  Play from folder N (0-based)\n");
    printf("\n");
    printf("--- Album / Artist ---\n");
    printf("  albums          List album groups\n");
    printf("  artists         List artist groups\n");
    printf("  playalbum <G> <T>   Play track T (0-based) in album group G\n");
    printf("  playartist <G> <T>  Play track T (0-based) in artist group G\n");
    printf("\n");
    printf("--- Favorites ---\n");
    printf("  fav             Toggle favorite for current track\n");
    printf("  isfav           Check if current track is favorite\n");
    printf("  favlist         List all favorites\n");
    printf("  favcount        Favorite count\n");
    printf("\n");
    printf("--- Search ---\n");
    printf("  search <kw>     Search tracks by keyword\n");
    printf("\n");
    printf("--- Lyrics ---\n");
    printf("  lyrics          Show lyrics for current track\n");
    printf("  lrcline <ms>    Get lyrics line index at position\n");
    printf("\n");
    printf("--- Album Art ---\n");
    printf("  art             Check album art for current track\n");
    printf("\n");
    printf("--- State ---\n");
    printf("  state           Show full app state\n");
    printf("  save            Save state to disk\n");
    printf("  restore         Restore state from disk\n");
    printf("  accoff          Simulate ACC OFF\n");
    printf("  accon           Simulate ACC ON\n");
    printf("  restorelist     Restore full device playlist\n");
    printf("\n");
    printf("--- System ---\n");
    printf("  help / h        Show this help\n");
    printf("  quit / q        Exit\n");
    printf("\n");
}

static void cmd_state(void) {
    const music_app_state_t* st = music_app_get_state();
    printf("\n=== App State ===\n");
    printf("  Player state   : %s\n", state_name(st->player_state));
    printf("  Play mode      : %s\n", mode_name(st->play_mode));
    printf("  Playlist type  : %s\n", playlist_type_name(st->playlist_type));
    printf("  Current device : %d / %d\n",
           st->current_device_idx, music_app_get_device_count());
    printf("  Current index  : %d / %d\n",
           music_app_get_current_index() + 1,
           music_app_get_playlist_count());
    if (st->current_info) {
        printf("  Current track  : %s - %s\n",
               st->current_info->artist, st->current_info->title);
        printf("  File           : %s\n", st->current_info->filepath);
    }
    char pos_buf[16], dur_buf[16];
    format_time(pos_buf, sizeof(pos_buf), st->current_position_ms);
    format_time(dur_buf, sizeof(dur_buf), st->current_duration_ms);
    printf("  Position       : %s / %s\n", pos_buf, dur_buf);
    printf("  Last path      : %s\n",
           st->last_path[0] ? st->last_path : "(none)");
    printf("  Favorites      : %d\n", music_app_get_favorite_count());
}

static void cmd_list(int max_items) {
    int count = music_app_get_playlist_count();
    int cur = music_app_get_current_index();
    int show = (count < max_items) ? count : max_items;

    printf("\n=== Playlist (%d tracks, showing %d) ===\n", count, show);
    for (int i = 0; i < show; i++) {
        const MusicInfo* info = music_app_get_track_info(i);
        if (!info) continue;
        printf("  %s[%3d] %-30.30s %-20.20s %s\n",
               (i == cur) ? ">>" : "  ",
               i + 1, info->title, info->artist, info->album);
    }
    if (count > show) {
        printf("  ... (%d more)\n", count - show);
    }
}

static void cmd_info(void) {
    const music_app_state_t* st = music_app_get_state();
    int idx = music_app_get_current_index();
    int total = music_app_get_playlist_count();

    if (!st->current_info) {
        printf("  No track playing.\n");
        return;
    }

    const MusicInfo* m = st->current_info;
    printf("\n=== Current Track [%d/%d] ===\n", idx + 1, total);
    printf("  Title    : %s\n", m->title);
    printf("  Artist   : %s\n", m->artist);
    printf("  Album    : %s\n", m->album);
    printf("  File     : %s\n", m->filepath);
    printf("  Filename : %s\n", m->filename);
    printf("  Size     : %u bytes\n", m->file_size);
    printf("  Track #  : %d\n", m->track_num);
    printf("  State    : %s\n", state_name(st->player_state));
    printf("  Mode     : %s\n", mode_name(st->play_mode));
    printf("  Favorite : %s\n", music_app_is_favorite() ? "YES" : "no");

    char pos[16], dur[16];
    format_time(pos, sizeof(pos), st->current_position_ms);
    format_time(dur, sizeof(dur), st->current_duration_ms);
    printf("  Position : %s / %s\n", pos, dur);
}

/*============================================================================
 * Signal handler
 *==========================================================================*/
static volatile int s_quit = 0;
static void sig_handler(int sig) {
    (void)sig;
    s_quit = 1;
}

/*============================================================================
 * Main
 *==========================================================================*/
int main(int argc, char* argv[]) {
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    printf("=== Music App CLI Test Tool ===\n");
    printf("Type 'help' for commands.\n\n");

    /* Initialize the full music app stack */
    int ret = music_app_init(ui_callback);
    if (ret != 0) {
        fprintf(stderr, "music_app_init failed (%d)\n", ret);
        /* Continue anyway — scanner/favorites still work */
    }

    /* If a directory was specified, trigger a manual scan.
     * Otherwise rely on usb_monitor auto-detection. */
    if (argc >= 2) {
        printf("Manual scan path: %s\n", argv[1]);
        printf("(Auto-detection also runs in background)\n");
        /* The usb_monitor_scan_existing() in music_app_init() should
         * already have picked up mounted devices. If the user specified
         * a path, it might not be a USB/SD mount — we can't easily
         * inject it. Just let them know. */
        printf("Hint: If '%s' is not auto-detected, create a test:\n", argv[1]);
        printf("  mount /dev/sda1 /mnt/usb0 && music_app_cli\n\n");
    }

    /* Wait a moment for scan threads to start */
    usleep(500000);

    /* Command loop */
    char line[1024];
    while (!s_quit) {
        printf("\nmusic> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) break;

        /* Strip newline */
        char* nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        if (line[0] == '\0') continue;

        /* Parse command and args */
        char cmd[64] = {0};
        char arg1[256] = {0};
        char arg2[64] = {0};
        sscanf(line, "%63s %255s %63s", cmd, arg1, arg2);

        /* === Playback === */
        if (strcmp(cmd, "play") == 0 || strcmp(cmd, "p") == 0) {
            if (arg1[0]) {
                int n = atoi(arg1);
                if (n > 0) {
                    printf("  Playing track %d...\n", n);
                    music_app_play(n - 1);
                } else {
                    printf("  Invalid track number.\n");
                }
            } else {
                printf("  Play/resume...\n");
                music_app_play(-1);
            }
        }
        else if (strcmp(cmd, "pause") == 0 || strcmp(cmd, "s") == 0) {
            music_app_pause();
            printf("  Paused.\n");
        }
        else if (strcmp(cmd, "resume") == 0) {
            music_app_resume();
            printf("  Resumed.\n");
        }
        else if (strcmp(cmd, "stop") == 0 || strcmp(cmd, "t") == 0) {
            music_app_stop();
            printf("  Stopped.\n");
        }
        else if (strcmp(cmd, "next") == 0 || strcmp(cmd, "n") == 0) {
            music_app_next();
        }
        else if (strcmp(cmd, "prev") == 0 || strcmp(cmd, "b") == 0) {
            music_app_prev();
        }
        else if (strcmp(cmd, "seek") == 0) {
            if (arg1[0]) {
                int ms = atoi(arg1);
                music_app_seek(ms);
                printf("  Seek to %d ms.\n", ms);
            } else {
                printf("  Usage: seek <milliseconds>\n");
            }
        }
        else if (strcmp(cmd, "toggle") == 0) {
            music_app_toggle_play_pause();
        }
        /* === Play Mode === */
        else if (strcmp(cmd, "mode") == 0) {
            if (arg1[0]) {
                int m = atoi(arg1);
                if (m >= 0 && m <= 3) {
                    music_app_set_play_mode((PlayMode)m);
                    printf("  Mode set to: %s\n", mode_name((PlayMode)m));
                } else {
                    printf("  Invalid mode (0-3).\n");
                }
            } else {
                music_app_cycle_play_mode();
                const music_app_state_t* st = music_app_get_state();
                printf("  Mode cycled to: %s\n", mode_name(st->play_mode));
            }
        }
        /* === Playlist === */
        else if (strcmp(cmd, "list") == 0 || strcmp(cmd, "l") == 0) {
            int max = arg1[0] ? atoi(arg1) : 50;
            if (max <= 0) max = 50;
            cmd_list(max);
        }
        else if (strcmp(cmd, "info") == 0 || strcmp(cmd, "i") == 0) {
            cmd_info();
        }
        else if (strcmp(cmd, "count") == 0) {
            printf("  Playlist: %d tracks\n", music_app_get_playlist_count());
        }
        /* === Storage Devices === */
        else if (strcmp(cmd, "devices") == 0) {
            int dc = music_app_get_device_count();
            const music_app_state_t* st = music_app_get_state();
            printf("\n=== Storage Devices (%d) ===\n", dc);
            for (int i = 0; i < dc; i++) {
                const storage_device_state_t* dev = music_app_get_device(i);
                if (!dev) continue;
                printf("  %s[%d] %s  mounted=%d scanning=%d scan_done=%d files=%d\n",
                       (i == st->current_device_idx) ? ">>" : "  ",
                       i, dev->mount_point,
                       dev->mounted, dev->scanning, dev->scan_done,
                       dev->music_list ? dev->music_list->count : 0);
            }
        }
        else if (strcmp(cmd, "switch") == 0) {
            if (arg1[0]) {
                int idx = atoi(arg1);
                printf("  Switching to device %d...\n", idx);
                music_app_switch_device(idx);
            } else {
                printf("  Usage: switch <device_index>\n");
            }
        }
        else if (strcmp(cmd, "rescan") == 0) {
            printf("  Rescanning...\n");
            music_app_rescan();
        }
        /* === Folders === */
        else if (strcmp(cmd, "folders") == 0) {
            const char** folders = NULL;
            int fc = 0;
            music_app_get_folder_list(&folders, &fc);
            printf("\n=== Folders (%d) ===\n", fc);
            for (int i = 0; i < fc; i++) {
                printf("  [%d] %s\n", i, folders[i]);
            }
        }
        else if (strcmp(cmd, "playfolder") == 0) {
            if (arg1[0]) {
                int idx = atoi(arg1);
                const char** folders = NULL;
                int fc = 0;
                music_app_get_folder_list(&folders, &fc);
                if (idx >= 0 && idx < fc) {
                    printf("  Playing folder: %s\n", folders[idx]);
                    music_app_play_folder(folders[idx]);
                } else {
                    printf("  Invalid folder index (0-%d).\n", fc - 1);
                }
            } else {
                printf("  Usage: playfolder <folder_index>\n");
            }
        }
        /* === Albums / Artists === */
        else if (strcmp(cmd, "albums") == 0) {
            const music_group_t* groups = NULL;
            int gc = 0;
            music_app_get_album_list(&groups, &gc);
            printf("\n=== Albums (%d) ===\n", gc);
            for (int i = 0; i < gc; i++) {
                printf("  [%d] \"%s\" (%d tracks)\n",
                       i, groups[i].key, groups[i].count);
            }
        }
        else if (strcmp(cmd, "artists") == 0) {
            const music_group_t* groups = NULL;
            int gc = 0;
            music_app_get_artist_list(&groups, &gc);
            printf("\n=== Artists (%d) ===\n", gc);
            for (int i = 0; i < gc; i++) {
                printf("  [%d] \"%s\" (%d tracks)\n",
                       i, groups[i].key, groups[i].count);
            }
        }
        else if (strcmp(cmd, "playalbum") == 0) {
            if (arg1[0] && arg2[0]) {
                int gi = atoi(arg1);
                int ti = atoi(arg2);
                const music_group_t* groups = NULL;
                int gc = 0;
                music_app_get_album_list(&groups, &gc);
                if (gi >= 0 && gi < gc) {
                    printf("  Playing album \"%s\" track %d...\n",
                           groups[gi].key, ti);
                    music_app_play_group(&groups[gi], ti);
                } else {
                    printf("  Invalid album group index.\n");
                }
            } else {
                printf("  Usage: playalbum <group_idx> <track_idx>\n");
            }
        }
        else if (strcmp(cmd, "playartist") == 0) {
            if (arg1[0] && arg2[0]) {
                int gi = atoi(arg1);
                int ti = atoi(arg2);
                const music_group_t* groups = NULL;
                int gc = 0;
                music_app_get_artist_list(&groups, &gc);
                if (gi >= 0 && gi < gc) {
                    printf("  Playing artist \"%s\" track %d...\n",
                           groups[gi].key, ti);
                    music_app_play_group(&groups[gi], ti);
                } else {
                    printf("  Invalid artist group index.\n");
                }
            } else {
                printf("  Usage: playartist <group_idx> <track_idx>\n");
            }
        }
        /* === Favorites === */
        else if (strcmp(cmd, "fav") == 0) {
            bool result = music_app_toggle_favorite();
            printf("  Favorite toggled → %s\n",
                   result ? "ADDED" : "REMOVED");
        }
        else if (strcmp(cmd, "isfav") == 0) {
            printf("  Current track is %sfavorite.\n",
                   music_app_is_favorite() ? "" : "NOT ");
        }
        else if (strcmp(cmd, "favlist") == 0) {
            int fc = 0;
            const MusicInfo* flist = music_app_get_favorite_list(&fc);
            printf("\n=== Favorites (%d) ===\n", fc);
            for (int i = 0; i < fc; i++) {
                printf("  [%d] %s - %s\n",
                       i + 1, flist[i].artist, flist[i].title);
            }
        }
        else if (strcmp(cmd, "favcount") == 0) {
            printf("  Favorites: %d\n", music_app_get_favorite_count());
        }
        /* === Search === */
        else if (strcmp(cmd, "search") == 0) {
            if (arg1[0]) {
                /* Reconstruct full keyword from line (in case of spaces) */
                const char* kw = line + 7; /* skip "search " */
                while (*kw == ' ') kw++;

                const MusicInfo* results[100];
                int found = music_app_search(kw, results, 100);
                printf("\n=== Search \"%s\" → %d results ===\n", kw, found);
                for (int i = 0; i < found; i++) {
                    printf("  [%d] %s - %s (%s)\n",
                           i + 1, results[i]->artist,
                           results[i]->title, results[i]->album);
                }
            } else {
                printf("  Usage: search <keyword>\n");
            }
        }
        /* === Lyrics === */
        else if (strcmp(cmd, "lyrics") == 0) {
            const lrc_data_t* lrc = music_app_get_lyrics();
            if (!lrc || lrc->count == 0) {
                printf("  No lyrics found for current track.\n");
            } else {
                printf("\n=== Lyrics (%d lines) ===\n", lrc->count);
                int show = lrc->count < 30 ? lrc->count : 30;
                for (int i = 0; i < show; i++) {
                    char t[16];
                    format_time(t, sizeof(t), lrc->lines[i].time_ms);
                    printf("  [%s] %s\n", t, lrc->lines[i].text);
                }
                if (lrc->count > show) {
                    printf("  ... (%d more lines)\n", lrc->count - show);
                }
            }
        }
        else if (strcmp(cmd, "lrcline") == 0) {
            if (arg1[0]) {
                int ms = atoi(arg1);
                int line_idx = music_app_get_lyrics_line(ms);
                if (line_idx >= 0) {
                    const lrc_data_t* lrc = music_app_get_lyrics();
                    if (lrc && line_idx < lrc->count) {
                        printf("  Line %d: %s\n",
                               line_idx, lrc->lines[line_idx].text);
                    }
                } else {
                    printf("  No lyrics line at %d ms.\n", ms);
                }
            } else {
                printf("  Usage: lrcline <milliseconds>\n");
            }
        }
        /* === Album Art === */
        else if (strcmp(cmd, "art") == 0) {
            const uint8_t* data = NULL;
            int size = 0;
            int r = music_app_get_album_art(&data, &size);
            if (r == 0 && data && size > 0) {
                /* Detect format from magic bytes */
                const char* fmt = "unknown";
                if (size >= 3 && data[0] == 0xFF && data[1] == 0xD8)
                    fmt = "JPEG";
                else if (size >= 8 && data[0] == 0x89 && data[1] == 'P')
                    fmt = "PNG";
                printf("  Album art: %d bytes, format: %s\n", size, fmt);
                printf("  To save: art > /tmp/cover.jpg (not implemented in CLI)\n");
            } else {
                printf("  No album art found.\n");
            }
        }
        /* === State === */
        else if (strcmp(cmd, "state") == 0) {
            cmd_state();
        }
        else if (strcmp(cmd, "save") == 0) {
            music_app_save_state();
            printf("  State saved.\n");
        }
        else if (strcmp(cmd, "restore") == 0) {
            music_app_restore_state();
            printf("  State restored.\n");
        }
        else if (strcmp(cmd, "accoff") == 0) {
            printf("  Simulating ACC OFF...\n");
            music_app_on_acc_off();
            printf("  ACC OFF done.\n");
        }
        else if (strcmp(cmd, "accon") == 0) {
            printf("  Simulating ACC ON...\n");
            music_app_on_acc_on();
            printf("  ACC ON done.\n");
        }
        else if (strcmp(cmd, "restorelist") == 0) {
            music_app_restore_full_playlist();
            printf("  Full playlist restored.\n");
        }
        /* === System === */
        else if (strcmp(cmd, "help") == 0 || strcmp(cmd, "h") == 0) {
            cmd_help();
        }
        else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "q") == 0) {
            break;
        }
        else {
            printf("  Unknown command: '%s'. Type 'help' for usage.\n", cmd);
        }
    }

    printf("\nShutting down...\n");
    music_app_deinit();
    printf("Bye.\n");
    return 0;
}
