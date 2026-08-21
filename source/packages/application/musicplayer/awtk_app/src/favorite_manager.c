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
#include "darray.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>

/* Define dynamic array type for favorites */
DARRAY_DEFINE(FavArray, MusicInfo)

/*============================================================================
 * Module state
 *==========================================================================*/
static struct {
    FavArray            arr;       /* was: MusicInfo items[128] */
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
    for (i = s_fav.arr.count - 1; i >= 0; i--) {
        if (reverse_path_equals(s_fav.arr.items[i].filepath, filepath)) {
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
    while (fgets(line, sizeof(line), fp)) {
        char* nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        if (line[0] == '\0') continue;

        MusicInfo item;
        memset(&item, 0, sizeof(item));
        item.media_type = MEDIA_TYPE_MUSIC;
        item.uid = s_fav.arr.count;

        /* Parse: filepath\ttitle\tartist\talbum */
        char* p = line;
        char* tab;

        /* filepath */
        tab = strchr(p, '\t');
        if (tab) {
            *tab = '\0';
            snprintf(item.filepath, sizeof(item.filepath), "%s", p);
            p = tab + 1;
        } else {
            snprintf(item.filepath, sizeof(item.filepath), "%s", p);
            FavArray_push(&s_fav.arr, &item);
            continue;
        }

        /* title */
        tab = strchr(p, '\t');
        if (tab) {
            *tab = '\0';
            snprintf(item.title, sizeof(item.title), "%s", p);
            p = tab + 1;
        } else {
            snprintf(item.title, sizeof(item.title), "%s", p);
            FavArray_push(&s_fav.arr, &item);
            continue;
        }

        /* artist */
        tab = strchr(p, '\t');
        if (tab) {
            *tab = '\0';
            snprintf(item.artist, sizeof(item.artist), "%s", p);
            p = tab + 1;
        } else {
            snprintf(item.artist, sizeof(item.artist), "%s", p);
            FavArray_push(&s_fav.arr, &item);
            continue;
        }

        /* album */
        snprintf(item.album, sizeof(item.album), "%s", p);

        /* Extract filename from filepath */
        const char* slash = strrchr(item.filepath, '/');
        snprintf(item.filename, sizeof(item.filename), "%s",
                 slash ? slash + 1 : item.filepath);

        FavArray_push(&s_fav.arr, &item);
    }

    fclose(fp);
    printf("[favorite] Loaded %d favorites from DB\n", s_fav.arr.count);
}

/*============================================================================
 * Public API
 *==========================================================================*/

int favorite_init(favorite_callback_t cb) {
    if (s_fav.inited) return -1;

    memset(&s_fav, 0, sizeof(s_fav));
    FavArray_init(&s_fav.arr, 32);  /* start small, grows as needed */
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
    FavArray_destroy(&s_fav.arr);
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

    /* Soft limit warning (Android: MAX_FAVORITE_INFO_THRESHOLD) */
    if (s_fav.arr.count >= FAVORITE_SOFT_LIMIT) {
        pthread_mutex_unlock(&s_fav.mutex);
        notify(FAVORITE_OP_MAX_LIMIT, NULL, -1);
        return false;
    }

    /* Copy into list via darray push (auto-grows if needed) */
    MusicInfo copy = *info;
    copy.uid = s_fav.arr.count;
    if (FavArray_push(&s_fav.arr, &copy) != 0) {
        pthread_mutex_unlock(&s_fav.mutex);
        return false; /* OOM */
    }
    int idx = s_fav.arr.count - 1;

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

    MusicInfo removed = s_fav.arr.items[idx];

    /* Remove via darray (handles memmove internally) */
    FavArray_remove(&s_fav.arr, idx);

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
    if (!info || info->filepath[0] == '\0') return false;

    /* Issue #22 fix: Hold mutex throughout check+action to prevent TOCTOU race.
     * If two threads toggle the same track simultaneously, one might add
     * a duplicate. By holding the lock for the entire operation, the
     * check-and-modify is atomic. */
    pthread_mutex_lock(&s_fav.mutex);

    int idx = find_by_path(info->filepath);
    if (idx >= 0) {
        /* Already favorited → remove */
        MusicInfo removed = s_fav.arr.items[idx];
        FavArray_remove(&s_fav.arr, idx);
        pthread_mutex_unlock(&s_fav.mutex);
        notify(FAVORITE_OP_REMOVE, &removed, idx);
        return false;  /* Now un-favorited */
    } else {
        /* Not favorited → add */
        if (s_fav.arr.count >= FAVORITE_SOFT_LIMIT) {
            pthread_mutex_unlock(&s_fav.mutex);
            notify(FAVORITE_OP_MAX_LIMIT, NULL, -1);
            return false;
        }
        MusicInfo copy = *info;
        copy.uid = s_fav.arr.count;
        if (FavArray_push(&s_fav.arr, &copy) != 0) {
            pthread_mutex_unlock(&s_fav.mutex);
            return false;
        }
        int new_idx = s_fav.arr.count - 1;
        pthread_mutex_unlock(&s_fav.mutex);
        notify(FAVORITE_OP_ADD, info, new_idx);
        return true;   /* Now favorited */
    }
}

const MusicInfo* favorite_get_list(int* out_count) {
    if (out_count) *out_count = s_fav.arr.count;
    return s_fav.arr.items;
}

void favorite_save(void) {
    mkdir("/data/music", 0755);
    FILE* fp = fopen(FAVORITE_DB_PATH, "w");
    if (!fp) return;

    pthread_mutex_lock(&s_fav.mutex);
    int i;
    for (i = 0; i < s_fav.arr.count; i++) {
        const MusicInfo* item = &s_fav.arr.items[i];
        fprintf(fp, "%s\t%s\t%s\t%s\n",
                item->filepath, item->title, item->artist, item->album);
    }
    pthread_mutex_unlock(&s_fav.mutex);

    fclose(fp);
    printf("[favorite] Saved %d favorites\n", s_fav.arr.count);
}

int favorite_validate(const MusicList* music_list) {
    if (!music_list) return 0;

    pthread_mutex_lock(&s_fav.mutex);

    int removed_count = 0;
    int i = 0;
    while (i < s_fav.arr.count) {
        /* Check if this favorite's file exists in the scan result */
        bool found = false;
        int j;
        for (j = 0; j < music_list->count; j++) {
            if (reverse_path_equals(s_fav.arr.items[i].filepath,
                                    music_list->items[j].filepath)) {
                found = true;
                break;
            }
        }

        if (!found) {
            /* File no longer exists — remove from favorites */
            FavArray_remove(&s_fav.arr, i);
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
