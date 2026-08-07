/*
 * music_main.cpp - Command-line music player for testing
 *
 * Usage:
 *   musicplayer_test <directory>
 *   e.g. musicplayer_test /mnt/usb
 *
 * Commands (stdin):
 *   p        - play / resume
 *   s        - pause
 *   t        - stop
 *   n        - next
 *   b        - previous (back)
 *   m        - cycle play mode
 *   l        - list all tracks
 *   i        - show current track info
 *   1-999    - play track number
 *   q        - quit
 */

#include "music_scanner.h"
#include "music_player.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* --- Callbacks --- */

static void on_state_changed(PlayerState state, void *user_data)
{
    (void)user_data;
    const char *s = "UNKNOWN";
    switch (state) {
    case PLAYER_STATE_IDLE:    s = "IDLE"; break;
    case PLAYER_STATE_PLAYING: s = "PLAYING"; break;
    case PLAYER_STATE_PAUSED:  s = "PAUSED"; break;
    case PLAYER_STATE_STOPPED: s = "STOPPED"; break;
    case PLAYER_STATE_ERROR:   s = "ERROR"; break;
    }
    printf("\n>> State: %s\n", s);
}

static void on_track_changed(int index, const MusicInfo *info, void *user_data)
{
    (void)user_data;
    printf("\n>> Now Playing [%d]: %s - %s (%s)\n",
           index + 1, info->artist, info->title, info->album);
}

static void on_position_changed(int position_ms, int duration_ms, void *user_data)
{
    (void)user_data;
    (void)duration_ms;
    int sec = position_ms / 1000;
    int min = sec / 60;
    sec %= 60;
    printf("\r  [%02d:%02d]", min, sec);
    fflush(stdout);
}

static const char *mode_name(PlayMode m)
{
    switch (m) {
    case PLAY_MODE_SEQUENTIAL: return "Sequential";
    case PLAY_MODE_REPEAT_ALL: return "Repeat All";
    case PLAY_MODE_REPEAT_ONE: return "Repeat One";
    case PLAY_MODE_SHUFFLE:    return "Shuffle";
    }
    return "Unknown";
}

static void print_help(void)
{
    printf("\n--- Music Player Test ---\n");
    printf("  p  - Play / Resume\n");
    printf("  s  - Pause\n");
    printf("  t  - Stop\n");
    printf("  n  - Next\n");
    printf("  b  - Previous\n");
    printf("  m  - Cycle play mode\n");
    printf("  l  - List tracks\n");
    printf("  i  - Current track info\n");
    printf("  1-999 - Play track #\n");
    printf("  q  - Quit\n");
    printf("-------------------------\n");
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <music_directory>\n", argv[0]);
        printf("  e.g. %s /mnt/usb\n", argv[0]);
        return 1;
    }

    const char *scan_dir = argv[1];

    /* 1. Scan for music files */
    printf("=== Scanning: %s ===\n", scan_dir);
    MusicList *list = music_list_create(MUSIC_MAX_FILES);
    if (!list) {
        fprintf(stderr, "Failed to create music list\n");
        return 1;
    }

    int ret = music_scan_directory(list, scan_dir);
    if (ret != 0 || list->count == 0) {
        printf("No music files found in %s\n", scan_dir);
        music_list_destroy(list);
        return 1;
    }

    /* Save scan results to DB */
    music_db_save(list, MUSIC_DB_PATH);

    /* 2. Create player */
    MusicPlayerContext *player = music_player_create();
    if (!player) {
        fprintf(stderr, "Failed to create music player\n");
        music_list_destroy(list);
        return 1;
    }

    /* Set callbacks */
    music_player_set_state_callback(player, on_state_changed, NULL);
    music_player_set_track_callback(player, on_track_changed, NULL);
    music_player_set_position_callback(player, on_position_changed, NULL);

    /* Load playlist */
    music_player_set_playlist(player, list);

    print_help();

    /* 3. Command loop */
    char cmd[32];
    while (1) {
        printf("\n> ");
        fflush(stdout);
        if (!fgets(cmd, sizeof(cmd), stdin)) break;

        /* Strip newline */
        char *nl = strchr(cmd, '\n');
        if (nl) *nl = '\0';

        if (strlen(cmd) == 0) continue;

        if (cmd[0] == 'q') {
            break;
        } else if (cmd[0] == 'p') {
            if (music_player_get_state(player) == PLAYER_STATE_PAUSED)
                music_player_resume(player);
            else
                music_player_play(player, -1);
        } else if (cmd[0] == 's') {
            music_player_pause(player);
        } else if (cmd[0] == 't') {
            music_player_stop(player);
        } else if (cmd[0] == 'n') {
            music_player_next(player);
        } else if (cmd[0] == 'b') {
            music_player_prev(player);
        } else if (cmd[0] == 'm') {
            PlayMode cur = music_player_get_mode(player);
            PlayMode next = (PlayMode)((cur + 1) % 4);
            music_player_set_mode(player, next);
            printf("Play mode: %s\n", mode_name(next));
        } else if (cmd[0] == 'l') {
            printf("\n=== Playlist (%d tracks) ===\n", list->count);
            for (int i = 0; i < list->count; i++) {
                const MusicInfo *m = &list->items[i];
                int cur = music_player_get_current_index(player);
                printf("  %s[%3d] %-30s %-20s %s\n",
                       (i == cur) ? ">> " : "   ",
                       i + 1, m->title, m->artist, m->album);
            }
        } else if (cmd[0] == 'i') {
            int idx = music_player_get_current_index(player);
            if (idx >= 0 && idx < list->count) {
                const MusicInfo *m = &list->items[idx];
                printf("  Title:  %s\n", m->title);
                printf("  Artist: %s\n", m->artist);
                printf("  Album:  %s\n", m->album);
                printf("  File:   %s\n", m->filepath);
                printf("  Mode:   %s\n", mode_name(music_player_get_mode(player)));
            } else {
                printf("No track playing\n");
            }
        } else if (cmd[0] >= '1' && cmd[0] <= '9') {
            int num = atoi(cmd);
            if (num > 0 && num <= list->count) {
                music_player_play(player, num - 1);
            } else {
                printf("Invalid track number (1-%d)\n", list->count);
            }
        } else {
            print_help();
        }
    }

    /* 4. Cleanup */
    music_player_destroy(player);
    music_list_destroy(list);
    printf("Bye.\n");
    return 0;
}
