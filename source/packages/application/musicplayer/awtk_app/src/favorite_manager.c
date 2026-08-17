/**
 * @file favorite_manager.c
 * @brief Music favorite list manager implementation.
 *
 * Mirrors Android FavoriteManager.java behavior:
 *   - reverseEquals for fast path comparison
 *   - 128-item cap with OPERATE_MAX_LIMIT callback
 *   - Persistent storage as tab-separated text file
 *
 * File location: source/packages/application/musicplayer/awtk_app/src/favorite_manager.c
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 */

#include "favorite_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>

/*============================================================================
 * Module state
 *==========================================================================*/
static struct {
    MusicInfo           items[FAVORITE_MAX_COUNT];
    int                 count;
    favorite_callback_t callback;
    pthread_mutex_t     mutex;
    bool                inited;
} s_fav;

/*============================================================================
 * Internal helpers
 *==========================================================================*/

/**
 * Reverse string comparison — matches Android MiscUtils.reverseEquals().
 * Compares from the end of the string, which is faster for filepaths
 * that share long common prefixes (e.g. /mnt/usb0/Music/...).
 */
static bool reverse_path_equals(const char* a, const char* b) {
    if (a == NULL || b == NULL) return false;
    int la = (int)strlen(a);
    int lb = (int)strlen(b);
    if (la != lb) return false;
    int i;
    for (i = la - 1; i >= 0; i--) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

static int find_by_path(const char* filepath) {
    if (filepath == NULL || filepath[0] == '\0') return -1;
    int i;
    for (i = s_fav.count - 1; i >= 0; i--) {
        if (reverse_path_equals(s_fav.items[i].filepath, filepath)) {
            return i;
        }
    }
    return -1;
}

static void notify(favorite_op_t op, const MusicInfo* info, int index) {
    if (s_fav.callback) {
        s_fav.callback(op, info, index);
    }
}

/*============================================================================
 * Persistence — simple TSV format
 * Each line: filepath\ttitle\tartist\talbum
 *==========================================================================*/

static void load_from_db(void) {
    FILE* fp = fopen(FAVORITE_DB_PATH, "r");
    if (!fp) return;

    char line[2048];
    while (fgets(line, sizeof(line), fp) && s_fav.count < FAVORITE_MAX_COUNT) {
        char* nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        if (line[0] == '\0') continue;

        MusicInfo* item = &s_fav.items[s_fav.count];
        memset(item, 0, sizeof(*item));
        item->media_type = MEDIA_TYPE_MUSIC;
        item->uid = s_fav.count;

        /* Parse: filepath\ttitle\tartist\talbum */
        char* p = line;
        char* tab;

        /* filepath */
        tab = strchr(p, '\t');
        if (tab) {
            *tab = '\0';
            snprintf(item->filepath, sizeof(item->filepath), "%s", p);
            p = tab + 1;
        } else {
            snprintf(item->filepath, sizeof(item->filepath), "%s", p);
            s_fav.count++;
            continue;
        }

        /* title */
        tab = strchr(p, '\t');
        if (tab) {
            *tab = '\0';
            snprintf(item->title, sizeof(item->title), "%s", p);
            p = tab + 1;
        } else {
            snprintf(item->title, sizeof(item->title), "%s", p);
            s_fav.count++;
            continue;
        }

        /* artist */
        tab = strchr(p, '\t');
        if (tab) {
            *tab = '\0';
            snprintf(item->artist, sizeof(item->artist), "%s", p);
            p = tab + 1;
        } else {
            snprintf(item->artist, sizeof(item->artist), "%s", p);
            s_fav.count++;
            continue;
        }

        /* album */
        snprintf(item->album, sizeof(item->album), "%s", p);

        /* Extract filename from filepath */
        const char* slash = strrchr(item->filepath, '/');
        snprintf(item->filename, sizeof(item->filename), "%s",
                 slash ? slash + 1 : item->filepath);

        s_fav.count++;
    }

    fclose(fp);
    printf("[favorite] Loaded %d favorites from DB\n", s_fav.count);
}

/*============================================================================
 * Public API
 *==========================================================================*/

int favorite_init(favorite_callback_t cb) {
    if (s_fav.inited) return -1;

    memset(&s_fav, 0, sizeof(s_fav));
    pthread_mutex_init(&s_fav.mutex, NULL);
    s_fav.callback = cb;

    load_from_db();

    s_fav.inited = true;
    notify(FAVORITE_OP_INITED, NULL, -1);
    return 0;
}

void favorite_deinit(void) {
    if (!s_fav.inited) return;
    favorite_save();
    pthread_mutex_destroy(&s_fav.mutex);
    memset(&s_fav, 0, sizeof(s_fav));
}

bool favorite_add(const MusicInfo* info) {
    if (!info || info->filepath[0] == '\0') return false;

    pthread_mutex_lock(&s_fav.mutex);

    /* Already exists? */
    if (find_by_path(info->filepath) >= 0) {
        pthread_mutex_unlock(&s_fav.mutex);
        return false;
    }

    /* At limit? (Android: MAX_FAVORITE_INFO_THRESHOLD) */
    if (s_fav.count >= FAVORITE_MAX_COUNT) {
        pthread_mutex_unlock(&s_fav.mutex);
        notify(FAVORITE_OP_MAX_LIMIT, NULL, -1);
        return false;
    }

    /* Copy into list */
    int idx = s_fav.count;
    s_fav.items[idx] = *info;
    s_fav.items[idx].uid = idx;
    s_fav.count++;

    pthread_mutex_unlock(&s_fav.mutex);

    notify(FAVORITE_OP_ADD, info, idx);
    return true;
}

bool favorite_remove(const char* filepath) {
    if (!filepath) return false;

    pthread_mutex_lock(&s_fav.mutex);

    int idx = find_by_path(filepath);
    if (idx < 0) {
        pthread_mutex_unlock(&s_fav.mutex);
        return false;
    }

    MusicInfo removed = s_fav.items[idx];

    /* Shift remaining items down */
    int i;
    for (i = idx; i < s_fav.count - 1; i++) {
        s_fav.items[i] = s_fav.items[i + 1];
    }
    s_fav.count--;

    pthread_mutex_unlock(&s_fav.mutex);

    notify(FAVORITE_OP_REMOVE, &removed, idx);
    return true;
}

bool favorite_contains(const char* filepath) {
    if (!filepath) return false;

    pthread_mutex_lock(&s_fav.mutex);
    bool found = (find_by_path(filepath) >= 0);
    pthread_mutex_unlock(&s_fav.mutex);
    return found;
}

bool favorite_toggle(const MusicInfo* info) {
    if (!info) return false;

    if (favorite_contains(info->filepath)) {
        favorite_remove(info->filepath);
        return false;  /* Now un-favorited */
    } else {
        favorite_add(info);
        return true;   /* Now favorited */
    }
}

const MusicInfo* favorite_get_list(int* out_count) {
    if (out_count) *out_count = s_fav.count;
    return s_fav.items;
}

void favorite_save(void) {
    mkdir("/data/music", 0755);
    FILE* fp = fopen(FAVORITE_DB_PATH, "w");
    if (!fp) return;

    pthread_mutex_lock(&s_fav.mutex);
    int i;
    for (i = 0; i < s_fav.count; i++) {
        const MusicInfo* item = &s_fav.items[i];
        fprintf(fp, "%s\t%s\t%s\t%s\n",
                item->filepath, item->title, item->artist, item->album);
    }
    pthread_mutex_unlock(&s_fav.mutex);

    fclose(fp);
    printf("[favorite] Saved %d favorites\n", s_fav.count);
}

int favorite_validate(const MusicList* music_list) {
    if (!music_list) return 0;

    pthread_mutex_lock(&s_fav.mutex);

    int removed_count = 0;
    int i = 0;
    while (i < s_fav.count) {
        /* Check if this favorite's file exists in the scan result */
        bool found = false;
        int j;
        for (j = 0; j < music_list->count; j++) {
            if (reverse_path_equals(s_fav.items[i].filepath,
                                    music_list->items[j].filepath)) {
                found = true;
                break;
            }
        }

        if (!found) {
            /* File no longer exists — remove from favorites */
            int k;
            for (k = i; k < s_fav.count - 1; k++) {
                s_fav.items[k] = s_fav.items[k + 1];
            }
            s_fav.count--;
            removed_count++;
            /* Don't increment i — re-check the item that shifted into this slot */
        } else {
            i++;
        }
    }

    pthread_mutex_unlock(&s_fav.mutex);

    if (removed_count > 0) {
        printf("[favorite] Validated: removed %d stale entries\n", removed_count);
        notify(FAVORITE_OP_UPDATE, NULL, -1);
    }
    return removed_count;
}
