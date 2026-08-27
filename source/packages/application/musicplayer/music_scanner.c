/*
 * music_scanner.c - Local music file scanner with ID3v2 tag parsing
 *
 * Pure C implementation, no external library dependencies.
 * Parses ID3v2.3/2.4 headers for title (TIT2), artist (TPE1), album (TALB).
 *
 * Architecture (GAP-1 回退 — raw SQL removed):
 *   scanner → mpd_db_add_song() → MPD Directory/Song tree
 *   MPD tree → DatabaseSaveSqlite.cxx → SQLite  (persistence)
 *   MPD tree → mpd_db_visit_all()     → MusicList (for UI/playlist)
 *
 * The scanner no longer owns any SQLite code. All database operations
 * are delegated to mpd_db_bridge.h (C API) backed by mpd_db_bridge.cxx
 * (C++ STL) and DatabaseSaveSqlite.cxx (SQLite persistence layer).
 */

#include "music_scanner.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

/*
 * Issue #67: When USE_CMUS_ID3 is defined, the ID3 parser delegates to
 * cmus_bridge_parse_tags() for proper UTF-16→UTF-8 decoding.
 * Issue #68: CUE sheet files are detected and expanded into track entries.
 *
 * To enable: add -DUSE_CMUS_ID3 to CFLAGS and link cmus_bridge.c + cmus/*.c
 */
#ifdef USE_CMUS_ID3
#include "cmus_bridge.h"
#endif

/* ---------- Utility helpers ---------- */

static const char *get_filename(const char *path)
{
    const char *p = strrchr(path, '/');
    return p ? p + 1 : path;
}

static const char *get_extension(const char *filename)
{
    const char *p = strrchr(filename, '.');
    return p ? p : "";
}

static void str_to_lower(char *dst, const char *src, size_t len)
{
    size_t i;
    for (i = 0; i < len - 1 && src[i]; i++)
        dst[i] = tolower((unsigned char)src[i]);
    dst[i] = '\0';
}

/* Strip trailing whitespace / null padding from ID3 tag strings */
static void trim_tag(char *s)
{
    int len = strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\0'))
        len--;
    s[len] = '\0';
}

bool music_is_audio_file(const char *filename)
{
    char ext[16];
    const char *p = get_extension(filename);
    if (!p || !*p) return false;

    str_to_lower(ext, p, sizeof(ext));

    return (strcmp(ext, MUSIC_EXT_MP3)  == 0 ||
            strcmp(ext, MUSIC_EXT_WAV)  == 0 ||
            strcmp(ext, MUSIC_EXT_FLAC) == 0 ||
            strcmp(ext, MUSIC_EXT_AAC)  == 0 ||
            strcmp(ext, MUSIC_EXT_OGG)  == 0 ||
            strcmp(ext, MUSIC_EXT_WMA)  == 0 ||
            strcmp(ext, MUSIC_EXT_M4A)  == 0);
}

/* ---------- ID3v2 parser ---------- */

/*
 * ID3v2 header: 10 bytes
 *   "ID3"
 *   version (2 bytes): major, revision
 *   flags (1 byte)
 *   size (4 bytes): synchsafe integer
 *
 * Frame header (v2.3/v2.4): 10 bytes
 *   frame ID (4 bytes): e.g. "TIT2"
 *   size (4 bytes): v2.3=big-endian, v2.4=synchsafe
 *   flags (2 bytes)
 */

static uint32_t read_synchsafe(const uint8_t *buf)
{
    return ((uint32_t)buf[0] << 21) |
           ((uint32_t)buf[1] << 14) |
           ((uint32_t)buf[2] << 7)  |
           ((uint32_t)buf[3]);
}

static uint32_t read_be32(const uint8_t *buf)
{
    return ((uint32_t)buf[0] << 24) |
           ((uint32_t)buf[1] << 16) |
           ((uint32_t)buf[2] << 8)  |
           ((uint32_t)buf[3]);
}

/* Extract a text frame value. Handles encoding byte:
 *   0x00 = ISO-8859-1
 *   0x01 = UTF-16 with BOM
 *   0x02 = UTF-16BE
 *   0x03 = UTF-8
 * For simplicity, we take UTF-8 and ISO-8859-1 as-is, and handle
 * UTF-16 by skipping BOM and taking ASCII subset only. */
static void extract_text_frame(const uint8_t *data, uint32_t size,
                               char *out, size_t out_len)
{
    if (size < 1) return;

    uint8_t encoding = data[0];
    const uint8_t *text = data + 1;
    uint32_t text_len = size - 1;

    if (encoding == 0x00 || encoding == 0x03) {
        /* ISO-8859-1 or UTF-8: copy directly */
        uint32_t copy_len = (text_len < out_len - 1) ? text_len : (out_len - 1);
        memcpy(out, text, copy_len);
        out[copy_len] = '\0';
    } else if (encoding == 0x01 || encoding == 0x02) {
        /* UTF-16: skip BOM if present, extract ASCII-compatible chars */
        uint32_t offset = 0;
        if (encoding == 0x01 && text_len >= 2) {
            /* Skip BOM (FF FE or FE FF) */
            offset = 2;
        }
        size_t j = 0;
        for (uint32_t i = offset; i + 1 < text_len && j < out_len - 1; i += 2) {
            uint16_t ch;
            if (encoding == 0x01 && text_len >= 2 &&
                text[0] == 0xFF && text[1] == 0xFE) {
                /* Little-endian */
                ch = text[i] | (text[i+1] << 8);
            } else {
                /* Big-endian */
                ch = (text[i] << 8) | text[i+1];
            }
            if (ch == 0) break;
            if (ch < 0x80) {
                out[j++] = (char)ch;
            } else {
                out[j++] = '?'; /* non-ASCII placeholder */
            }
        }
        out[j] = '\0';
    }
    trim_tag(out);
}

int music_parse_id3v2(const char *filepath, MusicInfo *info)
{
#ifdef USE_CMUS_ID3
    /* Delegate to cmus full ID3 parser — supports UTF-16, ID3v1 fallback, genre table */
    return cmus_bridge_parse_tags(filepath, info);
#else
    FILE *fp = fopen(filepath, "rb");
    if (!fp) return -1;

    uint8_t header[10];
    if (fread(header, 1, 10, fp) != 10) {
        fclose(fp);
        return -1;
    }

    /* Check "ID3" magic */
    if (header[0] != 'I' || header[1] != 'D' || header[2] != '3') {
        fclose(fp);
        return -1; /* No ID3v2 tag */
    }

    uint8_t version_major = header[3]; /* 3 or 4 */
    /* uint8_t version_rev = header[4]; */
    /* uint8_t flags = header[5]; */
    uint32_t tag_size = read_synchsafe(&header[6]);

    /* Sanity check: cap tag size at 10 MB to prevent OOM from malformed files.
     * Real-world ID3 tags with embedded album art rarely exceed 5 MB. */
    if (tag_size == 0 || tag_size > 10 * 1024 * 1024) {
        fclose(fp);
        return -1;
    }

    /* Read entire tag into memory */
    uint8_t *tag_data = (uint8_t *)malloc(tag_size);
    if (!tag_data) {
        fclose(fp);
        return -1;
    }

    if (fread(tag_data, 1, tag_size, fp) != tag_size) {
        free(tag_data);
        fclose(fp);
        return -1;
    }
    fclose(fp);

    /* Parse frames */
    uint32_t pos = 0;
    while (pos + 10 <= tag_size) {
        char frame_id[5];
        memcpy(frame_id, &tag_data[pos], 4);
        frame_id[4] = '\0';

        /* Padding check: frame ID should be alphanumeric */
        if (!isupper(frame_id[0]) && !isdigit(frame_id[0])) break;

        uint32_t frame_size;
        if (version_major >= 4) {
            frame_size = read_synchsafe(&tag_data[pos + 4]);
        } else {
            frame_size = read_be32(&tag_data[pos + 4]);
        }
        /* uint16_t frame_flags = (tag_data[pos+8] << 8) | tag_data[pos+9]; */

        pos += 10; /* skip frame header */

        if (frame_size == 0 || pos + frame_size > tag_size) break;

        if (strcmp(frame_id, "TIT2") == 0) {
            extract_text_frame(&tag_data[pos], frame_size,
                               info->title, MUSIC_MAX_TAG_LEN);
        } else if (strcmp(frame_id, "TPE1") == 0) {
            extract_text_frame(&tag_data[pos], frame_size,
                               info->artist, MUSIC_MAX_TAG_LEN);
        } else if (strcmp(frame_id, "TALB") == 0) {
            extract_text_frame(&tag_data[pos], frame_size,
                               info->album, MUSIC_MAX_TAG_LEN);
        } else if (strcmp(frame_id, "TRCK") == 0) {
            char tmp[32] = {0};
            extract_text_frame(&tag_data[pos], frame_size, tmp, sizeof(tmp));
            info->track_num = atoi(tmp);
        }

        pos += frame_size;
    }

    free(tag_data);
    return 0;
#endif /* USE_CMUS_ID3 */
}

/* ---------- Directory scanner ---------- */

/* Maximum recursion depth to prevent symlink loops and excessive nesting.
 * Android MediaFilePathScan does not recurse infinitely either — it skips
 * hidden dirs and .nomedia. We add an explicit depth cap for safety. */
#define MAX_SCAN_DEPTH 20

/*
 * Ensure the list has room for at least one more item.
 * Grows by MUSIC_GROW_FACTOR (2x) via realloc, capped at MUSIC_MAX_FILES.
 * Returns 0 on success, -1 on OOM or at ceiling.
 */
static int music_list_ensure_capacity(MusicList *list)
{
    if (list->count < list->capacity) return 0;

    if (list->capacity >= MUSIC_MAX_FILES) {
        fprintf(stderr, "[MusicScanner] Safety ceiling reached: %d files\n",
                MUSIC_MAX_FILES);
        return -1;
    }

    int new_cap = list->capacity * MUSIC_GROW_FACTOR;
    if (new_cap > MUSIC_MAX_FILES) new_cap = MUSIC_MAX_FILES;

    MusicInfo *new_items = (MusicInfo *)realloc(list->items,
                                                 new_cap * sizeof(MusicInfo));
    if (!new_items) {
        fprintf(stderr, "[MusicScanner] realloc failed: %d -> %d items "
                "(%zu bytes)\n", list->capacity, new_cap,
                (size_t)new_cap * sizeof(MusicInfo));
        return -1;
    }

    /* Zero out the newly allocated portion */
    memset(&new_items[list->capacity], 0,
           (new_cap - list->capacity) * sizeof(MusicInfo));

    list->items = new_items;
    list->capacity = new_cap;
    printf("[MusicScanner] List grown: capacity now %d\n", new_cap);
    return 0;
}

/* Per-list UID counter — avoids global static which is not thread-safe
 * if two scans run on different MusicLists concurrently. */

static int scan_dir_recursive(MusicList *list, const char *dir_path,
                              const char *device_name, int depth)
{
    if (depth > MAX_SCAN_DEPTH) {
        fprintf(stderr, "[MusicScanner] Max depth reached: %s\n", dir_path);
        return 0;
    }

    DIR *dir = opendir(dir_path);
    if (!dir) {
        fprintf(stderr, "[MusicScanner] Cannot open dir: %s (%s)\n",
                dir_path, strerror(errno));
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        /* Skip . and .. */
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;

        /* Skip hidden files/dirs (Android MediaFilePathScan filters these) */
        if (entry->d_name[0] == '.')
            continue;

        /* Build full path */
        char fullpath[MUSIC_MAX_PATH_LEN];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir_path, entry->d_name);

        /* Use lstat to detect symlinks — avoid following symlink loops */
        struct stat st;
        if (lstat(fullpath, &st) != 0) continue;

        /* Skip symlinks entirely to prevent infinite loops */
        if (S_ISLNK(st.st_mode)) continue;

        if (S_ISDIR(st.st_mode)) {
            /* Skip Android-style non-scan paths (aligned with MediaFilePathScan) */
            if (strstr(fullpath, "/Android") ||
                strstr(fullpath, "/LOST.DIR") ||
                strstr(fullpath, "/System Volume Information") ||
                strstr(fullpath, "/DCIM")) {
                continue;
            }

            /* Skip dirs with .nomedia file */
            char nomedia[MUSIC_MAX_PATH_LEN];
            snprintf(nomedia, sizeof(nomedia), "%s/.nomedia", fullpath);
            struct stat nm_st;
            if (stat(nomedia, &nm_st) == 0) continue;

            /* Recurse into subdirectory */
            scan_dir_recursive(list, fullpath, device_name, depth + 1);
        } else if (S_ISREG(st.st_mode)) {
#ifdef USE_CMUS_ID3
            /* Issue #68: CUE sheet detection — expand .cue files into track entries */
            if (cmus_bridge_is_cue(entry->d_name)) {
                int added = cmus_bridge_scan_cue(fullpath, dir_path, list);
                if (added > 0)
                    printf("[MusicScanner] CUE: %d tracks from %s\n", added, entry->d_name);
                continue;
            }
#endif
            if (!music_is_audio_file(entry->d_name)) continue;

            /* Dynamic growth: ensure room for one more item */
            if (music_list_ensure_capacity(list) != 0) {
                fprintf(stderr, "[MusicScanner] Cannot grow list, "
                        "stopping at %d files\n", list->count);
                closedir(dir);
                return 0;
            }

            MusicInfo *info = &list->items[list->count];
            memset(info, 0, sizeof(MusicInfo));

            info->uid = list->count + 1; /* per-list 1-based UID */
            info->media_type = MEDIA_TYPE_MUSIC;
            info->file_size = (uint32_t)st.st_size;
            strncpy(info->filepath, fullpath, MUSIC_MAX_PATH_LEN - 1);
            strncpy(info->filename, entry->d_name, MUSIC_MAX_TAG_LEN - 1);
            strncpy(info->device_name, device_name, MUSIC_MAX_PATH_LEN - 1);

            /* Try to parse ID3v2 tags (MP3 only for now) */
            char ext[16];
            str_to_lower(ext, get_extension(entry->d_name), sizeof(ext));

            if (strcmp(ext, MUSIC_EXT_MP3) == 0) {
                if (music_parse_id3v2(fullpath, info) == 0) {
#ifdef USE_CMUS_ID3
                    info->id3_parsed = 2; /* cmus parser (full charset) */
#else
                    info->id3_parsed = 1; /* built-in ID3v2 parser */
#endif
                } else {
                    /* No ID3 tag: use filename as title */
                    strncpy(info->title, entry->d_name, MUSIC_MAX_TAG_LEN - 1);
                    /* Remove extension from title */
                    char *dot = strrchr(info->title, '.');
                    if (dot) *dot = '\0';
                }
            } else {
                /* Non-MP3: use filename as title */
                strncpy(info->title, entry->d_name, MUSIC_MAX_TAG_LEN - 1);
                char *dot = strrchr(info->title, '.');
                if (dot) *dot = '\0';
            }

            /* If title is still empty after parsing, use filename */
            if (info->title[0] == '\0') {
                strncpy(info->title, entry->d_name, MUSIC_MAX_TAG_LEN - 1);
                char *dot = strrchr(info->title, '.');
                if (dot) *dot = '\0';
            }

            /* Default artist/album if empty */
            if (info->artist[0] == '\0')
                strncpy(info->artist, "Unknown", MUSIC_MAX_TAG_LEN - 1);
            if (info->album[0] == '\0')
                strncpy(info->album, "Unknown", MUSIC_MAX_TAG_LEN - 1);

            list->count++;
            printf("[MusicScanner] [%d] %s - %s (%s) id3=%d\n",
                   info->uid, info->artist, info->title, info->filepath,
                   info->id3_parsed);
        }
    }

    closedir(dir);
    return 0;
}

MusicList *music_list_create(int capacity)
{
    if (capacity <= 0)
        capacity = MUSIC_INIT_CAPACITY;
    /* No upper clamp here — realloc will grow as needed,
     * MUSIC_MAX_FILES is enforced in ensure_capacity */

    MusicList *list = (MusicList *)calloc(1, sizeof(MusicList));
    if (!list) return NULL;

    list->items = (MusicInfo *)calloc(capacity, sizeof(MusicInfo));
    if (!list->items) {
        free(list);
        return NULL;
    }

    list->capacity = capacity;
    list->count = 0;
    list->state = SCAN_IDLE;
    return list;
}

void music_list_destroy(MusicList *list)
{
    if (!list) return;
    free(list->items);
    free(list);
}

int music_scan_directory(MusicList *list, const char *dir_path)
{
    if (!list || !dir_path) return -1;

    list->state = SCAN_SCANNING;
    list->count = 0;
    strncpy(list->scan_path, dir_path, MUSIC_MAX_PATH_LEN - 1);

    printf("[MusicScanner] Start scanning: %s\n", dir_path);

    int ret = scan_dir_recursive(list, dir_path, dir_path, 0);

    list->state = (ret == 0) ? SCAN_DONE : SCAN_ERROR;
    printf("[MusicScanner] Scan complete: %d files found\n", list->count);

    return ret;
}

/*============================================================================
 * Cancellable scan — production solution for:
 *   1. 一万首歌曲怎么解决 → incremental progress callback
 *   2. 扫到一半拔U盘怎么解决 → cancel_flag + mount-point liveness check
 *==========================================================================*/

/**
 * Check if mount point is still accessible (U盘没被拔掉).
 * Cheap check: stat the mount point directory itself.
 * Returns true if still mounted, false if gone.
 */
static bool is_mount_alive(const char *mount_point) {
    struct stat st;
    if (stat(mount_point, &st) != 0) return false;
    return S_ISDIR(st.st_mode);
}

/**
 * Internal recursive scan with cancellation support.
 * Returns: 0=ok, -1=error, -2=cancelled
 */
static int scan_dir_recursive_cancellable(
        MusicList *list,
        const char *dir_path,
        const char *device_name,
        int depth,
        const char *mount_root,
        const volatile int *cancel_flag,
        int expected_gen,
        scan_progress_fn progress_cb,
        void *cb_ctx)
{
    if (depth > MAX_SCAN_DEPTH) return 0;

    if (cancel_flag && *cancel_flag != expected_gen) {
        printf("[MusicScanner] Scan cancelled (gen mismatch) at %s\n", dir_path);
        return -2;
    }
    if (!is_mount_alive(mount_root)) {
        printf("[MusicScanner] Mount gone during scan: %s\n", mount_root);
        return -2;
    }

    DIR *dir = opendir(dir_path);
    if (!dir) {
        if (errno == ENOENT || errno == EACCES || errno == EIO) {
            fprintf(stderr, "[MusicScanner] Dir inaccessible (device removed?): %s\n",
                    dir_path);
            return -2;
        }
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        /* Skip . .. and hidden */
        if (entry->d_name[0] == '.') continue;

        if (list->count > 0 && (list->count % SCAN_PROGRESS_INTERVAL) == 0) {
            if (cancel_flag && *cancel_flag != expected_gen) {
                closedir(dir);
                return -2;
            }
            if (!is_mount_alive(mount_root)) {
                closedir(dir);
                return -2;
            }
            if (progress_cb) {
                int cb_ret = progress_cb(list->count, cb_ctx);
                if (cb_ret != 0) {
                    closedir(dir);
                    return -2;
                }
            }
        }

        char fullpath[MUSIC_MAX_PATH_LEN];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (lstat(fullpath, &st) != 0) {
            if (errno == EIO || errno == ENOENT) continue;
            continue;
        }

        if (S_ISLNK(st.st_mode)) continue;

        if (S_ISDIR(st.st_mode)) {
            if (strstr(fullpath, "/Android") ||
                strstr(fullpath, "/LOST.DIR") ||
                strstr(fullpath, "/System Volume Information") ||
                strstr(fullpath, "/DCIM")) {
                continue;
            }

            char nomedia[MUSIC_MAX_PATH_LEN];
            snprintf(nomedia, sizeof(nomedia), "%s/.nomedia", fullpath);
            struct stat nm_st;
            if (stat(nomedia, &nm_st) == 0) continue;

            int ret = scan_dir_recursive_cancellable(
                list, fullpath, device_name, depth + 1,
                mount_root, cancel_flag, expected_gen,
                progress_cb, cb_ctx);
            if (ret == -2) {
                closedir(dir);
                return -2;  /* propagate cancellation up */
            }
        } else if (S_ISREG(st.st_mode)) {
#ifdef USE_CMUS_ID3
            if (cmus_bridge_is_cue(entry->d_name)) {
                int added = cmus_bridge_scan_cue(fullpath, dir_path, list);
                if (added > 0)
                    printf("[MusicScanner] CUE: %d tracks from %s\n", added, entry->d_name);
                continue;
            }
#endif
            if (!music_is_audio_file(entry->d_name)) continue;

            if (music_list_ensure_capacity(list) != 0) {
                fprintf(stderr, "[MusicScanner] Cannot grow list, "
                        "stopping at %d files\n", list->count);
                closedir(dir);
                return 0;
            }

            MusicInfo *info = &list->items[list->count];
            memset(info, 0, sizeof(MusicInfo));

            info->uid = list->count + 1;
            info->media_type = MEDIA_TYPE_MUSIC;
            info->file_size = (uint32_t)st.st_size;
            strncpy(info->filepath, fullpath, MUSIC_MAX_PATH_LEN - 1);
            strncpy(info->filename, entry->d_name, MUSIC_MAX_TAG_LEN - 1);
            strncpy(info->device_name, device_name, MUSIC_MAX_PATH_LEN - 1);

            /* ID3 parse */
            char ext[16];
            str_to_lower(ext, get_extension(entry->d_name), sizeof(ext));

            if (strcmp(ext, MUSIC_EXT_MP3) == 0) {
                if (music_parse_id3v2(fullpath, info) == 0) {
#ifdef USE_CMUS_ID3
                    info->id3_parsed = 2; /* cmus parser (full charset) */
#else
                    info->id3_parsed = 1; /* built-in ID3v2 parser */
#endif
                } else {
                    strncpy(info->title, entry->d_name, MUSIC_MAX_TAG_LEN - 1);
                    char *dot = strrchr(info->title, '.');
                    if (dot) *dot = '\0';
                }
            } else {
                strncpy(info->title, entry->d_name, MUSIC_MAX_TAG_LEN - 1);
                char *dot = strrchr(info->title, '.');
                if (dot) *dot = '\0';
            }

            if (info->title[0] == '\0') {
                strncpy(info->title, entry->d_name, MUSIC_MAX_TAG_LEN - 1);
                char *dot = strrchr(info->title, '.');
                if (dot) *dot = '\0';
            }

            if (info->artist[0] == '\0')
                strncpy(info->artist, "Unknown", MUSIC_MAX_TAG_LEN - 1);
            if (info->album[0] == '\0')
                strncpy(info->album, "Unknown", MUSIC_MAX_TAG_LEN - 1);

            list->count++;
        }
    }

    closedir(dir);
    return 0;
}

int music_scan_directory_cancellable(
        MusicList *list,
        const char *dir_path,
        const volatile int *cancel_flag,
        int expected_gen,
        scan_progress_fn progress_cb,
        void *cb_ctx)
{
    if (!list || !dir_path) return -1;

    list->state = SCAN_SCANNING;
    list->count = 0;
    strncpy(list->scan_path, dir_path, MUSIC_MAX_PATH_LEN - 1);

    printf("[MusicScanner] Start cancellable scan: %s (gen=%d)\n",
           dir_path, expected_gen);

    int ret = scan_dir_recursive_cancellable(
        list, dir_path, dir_path, 0,
        dir_path,  /* mount_root for liveness check */
        cancel_flag, expected_gen,
        progress_cb, cb_ctx);

    if (ret == -2) {
        list->state = SCAN_IDLE;
        printf("[MusicScanner] Scan cancelled at %d files\n", list->count);
        return -2;
    }

    list->state = (ret == 0) ? SCAN_DONE : SCAN_ERROR;
    printf("[MusicScanner] Scan complete: %d files found\n", list->count);
    return list->count;
}

/* ---------- Legacy flat-file database (retained for fallback) ---------- */

int music_db_save_text(const MusicList *list, const char *db_path)
{
    if (!list || !db_path) return -1;

    /* Ensure directory exists */
    char dir[MUSIC_MAX_PATH_LEN];
    strncpy(dir, db_path, MUSIC_MAX_PATH_LEN - 1);
    char *slash = strrchr(dir, '/');
    if (slash) {
        *slash = '\0';
        mkdir(dir, 0755);
    }

    FILE *fp = fopen(db_path, "w");
    if (!fp) {
        fprintf(stderr, "[MusicScanner] Cannot create text DB: %s (%s)\n",
                db_path, strerror(errno));
        return -1;
    }

    fprintf(fp, "#uid\ttype\ttitle\tartist\talbum\tduration\ttrack\tpath\tfilename\tdevice\tsize\n");

    for (int i = 0; i < list->count; i++) {
        const MusicInfo *m = &list->items[i];
        fprintf(fp, "%d\t%d\t%s\t%s\t%s\t%d\t%d\t%s\t%s\t%s\t%u\n",
                m->uid, m->media_type,
                m->title, m->artist, m->album,
                m->duration_ms, m->track_num,
                m->filepath, m->filename,
                m->device_name, m->file_size);
    }

    fclose(fp);
    printf("[MusicScanner] Text DB saved: %d records -> %s\n", list->count, db_path);
    return 0;
}

int music_db_load_text(MusicList *list, const char *db_path)
{
    if (!list || !db_path) return -1;

    FILE *fp = fopen(db_path, "r");
    if (!fp) return -1;

    char line[2048];
    list->count = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#') continue;

        if (music_list_ensure_capacity(list) != 0) {
            fprintf(stderr, "[MusicScanner] Cannot grow list during text DB load, "
                    "stopping at %d records\n", list->count);
            break;
        }

        MusicInfo *m = &list->items[list->count];
        memset(m, 0, sizeof(MusicInfo));

        int tmp_type = 0;
        int n = sscanf(line, "%d\t%d\t", &m->uid, &tmp_type);
        if (n < 2) continue;
        m->media_type = (MediaType)tmp_type;

        char *fields[11];
        int field_count = 0;
        char *p = line;
        while (field_count < 11 && p) {
            fields[field_count++] = p;
            p = strchr(p, '\t');
            if (p) *p++ = '\0';
        }

        if (field_count >= 11) {
            strncpy(m->title, fields[2], MUSIC_MAX_TAG_LEN - 1);
            strncpy(m->artist, fields[3], MUSIC_MAX_TAG_LEN - 1);
            strncpy(m->album, fields[4], MUSIC_MAX_TAG_LEN - 1);
            m->duration_ms = atoi(fields[5]);
            m->track_num = atoi(fields[6]);
            strncpy(m->filepath, fields[7], MUSIC_MAX_PATH_LEN - 1);
            strncpy(m->filename, fields[8], MUSIC_MAX_TAG_LEN - 1);
            strncpy(m->device_name, fields[9], MUSIC_MAX_PATH_LEN - 1);
            m->file_size = (uint32_t)strtoul(fields[10], NULL, 10);
            list->count++;
        }
    }

    fclose(fp);
    list->state = SCAN_DONE;
    printf("[MusicScanner] Text DB loaded: %d records from %s\n", list->count, db_path);
    return list->count;
}

/*============================================================================
 * MPD bridge: MusicList ←→ mpd_db_t
 *
 * Conditionally compiled: requires -DUSE_MPD_DB, libmpd_db_bridge.so, libsqlite3.
 * Not used by the AWTK music player app (music_app.c uses scan-to-MusicList path).
 *==========================================================================*/

#ifdef USE_MPD_DB
#include "mpd_db/mpd_db_bridge.h"

/**
 * Feed one MusicInfo into the MPD database tree.
 * Maps MusicInfo fields to mpd_song_info_t tags:
 *   title    → MPD_TAG_TITLE
 *   artist   → MPD_TAG_ARTIST
 *   album    → MPD_TAG_ALBUM
 *   track_num → MPD_TAG_TRACK
 */
static int feed_song_to_mpd(mpd_db_t *db, const MusicInfo *m)
{
    mpd_song_info_t si;
    memset(&si, 0, sizeof(si));

    si.uri = m->filepath;

    struct stat st;
    if (stat(m->filepath, &st) == 0)
        si.mtime = st.st_mtime;

    si.duration_ms = m->duration_ms;
    si.id3_parsed = m->id3_parsed;

    char track_str[16];
    if (m->track_num > 0) {
        snprintf(track_str, sizeof(track_str), "%d", m->track_num);
    } else {
        track_str[0] = '\0';
    }

    si.tags[MPD_TAG_TITLE]  = m->title;
    si.tags[MPD_TAG_ARTIST] = m->artist;
    si.tags[MPD_TAG_ALBUM]  = m->album;
    si.tags[MPD_TAG_TRACK]  = track_str[0] ? track_str : NULL;

    return mpd_db_add_song(db, &si);
}

/**
 * Visitor callback: rebuild MusicList from MPD tree.
 * Used by music_db_load() to populate the flat list that
 * music_app.c / music_player.cpp consume.
 */
struct load_ctx {
    MusicList *list;
};

static int mpd_to_musiclist_visitor(const mpd_song_info_t *song, void *user_data)
{
    struct load_ctx *ctx = (struct load_ctx *)user_data;
    MusicList *list = ctx->list;

    if (music_list_ensure_capacity(list) != 0)
        return -1;

    MusicInfo *m = &list->items[list->count];
    memset(m, 0, sizeof(MusicInfo));

    m->uid = list->count + 1;
    m->media_type = MEDIA_TYPE_MUSIC;

    if (song->uri)
        strncpy(m->filepath, song->uri, MUSIC_MAX_PATH_LEN - 1);

    const char *fn = get_filename(m->filepath);
    if (fn)
        strncpy(m->filename, fn, MUSIC_MAX_TAG_LEN - 1);

    strncpy(m->device_name, "device1", MUSIC_MAX_PATH_LEN - 1);

    /* folder_path = filepath truncated at last '/' */
    strncpy(m->folder_path, m->filepath, MUSIC_MAX_PATH_LEN - 1);
    char *last_slash = strrchr(m->folder_path, '/');
    if (last_slash)
        *last_slash = '\0';
    else
        m->folder_path[0] = '\0';

    m->duration_ms = song->duration_ms;
    m->id3_parsed = song->id3_parsed;

    if (song->tags[MPD_TAG_TITLE] && song->tags[MPD_TAG_TITLE][0])
        strncpy(m->title, song->tags[MPD_TAG_TITLE], MUSIC_MAX_TAG_LEN - 1);
    else {
        strncpy(m->title, m->filename, MUSIC_MAX_TAG_LEN - 1);
        char *dot = strrchr(m->title, '.');
        if (dot) *dot = '\0';
    }

    if (song->tags[MPD_TAG_ARTIST] && song->tags[MPD_TAG_ARTIST][0])
        strncpy(m->artist, song->tags[MPD_TAG_ARTIST], MUSIC_MAX_TAG_LEN - 1);
    else
        strncpy(m->artist, "Unknown", MUSIC_MAX_TAG_LEN - 1);

    if (song->tags[MPD_TAG_ALBUM] && song->tags[MPD_TAG_ALBUM][0])
        strncpy(m->album, song->tags[MPD_TAG_ALBUM], MUSIC_MAX_TAG_LEN - 1);
    else
        strncpy(m->album, "Unknown", MUSIC_MAX_TAG_LEN - 1);

    if (song->tags[MPD_TAG_TRACK] && song->tags[MPD_TAG_TRACK][0])
        m->track_num = atoi(song->tags[MPD_TAG_TRACK]);

    /* file_size stays 0 if device is not mounted — that's fine,
     * UI shows "未识别到" when no device is present. */

    list->count++;
    return 0;
}

/*------------------------------------------------------------------------
 * Public API: music_db_save (via mpd_db_bridge)
 *
 * 1. Open (or create) an mpd_db
 * 2. Clear existing data
 * 3. Feed every MusicInfo into the MPD tree via mpd_db_add_song()
 * 4. mpd_db_save() → triggers DatabaseSaveSqlite → SQLite
 * 5. Close
 *----------------------------------------------------------------------*/

int music_db_save(const MusicList *list, const char *db_path)
{
    if (!list || !db_path) return -1;

    /* Ensure parent directory exists */
    char dir[MUSIC_MAX_PATH_LEN];
    strncpy(dir, db_path, MUSIC_MAX_PATH_LEN - 1);
    dir[MUSIC_MAX_PATH_LEN - 1] = '\0';
    char *sl = strrchr(dir, '/');
    if (sl) {
        *sl = '\0';
        mkdir(dir, 0755);
    }

    mpd_db_t *db = mpd_db_open(db_path);
    if (!db) {
        fprintf(stderr, "[MusicScanner] mpd_db_open failed: %s\n", db_path);
        return -1;
    }

    mpd_db_clear(db);

    int errors = 0;
    for (int i = 0; i < list->count; i++) {
        if (feed_song_to_mpd(db, &list->items[i]) != 0) {
            errors++;
            fprintf(stderr, "[MusicScanner] mpd_db_add_song failed for: %s\n",
                    list->items[i].filepath);
        }
    }

    int ret = mpd_db_save(db);
    mpd_db_close(db);

    if (ret != 0) {
        fprintf(stderr, "[MusicScanner] mpd_db_save failed: %s\n", db_path);
        return -1;
    }

    printf("[MusicScanner] MPD DB saved: %d records -> %s (%d errors)\n",
           list->count, db_path, errors);
    return 0;
}

/*------------------------------------------------------------------------
 * Public API: music_db_load (via mpd_db_bridge)
 *
 * 1. Try to open as MPD/SQLite DB
 * 2. If not a valid DB, fall back to text parser (seamless migration)
 * 3. Visit all songs in MPD tree → populate MusicList
 *----------------------------------------------------------------------*/

int music_db_load(MusicList *list, const char *db_path)
{
    if (!list || !db_path) return -1;

    /* Auto-detect: if file doesn't look like SQLite, try text fallback.
     * This enables seamless OTA migration from old text format. */
    FILE *fp = fopen(db_path, "rb");
    if (!fp) return -1;
    char magic[16];
    size_t n = fread(magic, 1, 16, fp);
    fclose(fp);
    if (n < 16 || memcmp(magic, "SQLite format 3\000", 16) != 0) {
        printf("[MusicScanner] Not a SQLite file, falling back to text: %s\n",
               db_path);
        return music_db_load_text(list, db_path);
    }

    mpd_db_t *db = mpd_db_open(db_path);
    if (!db) {
        fprintf(stderr, "[MusicScanner] mpd_db_open failed for load: %s\n",
                db_path);
        return -1;
    }

    list->count = 0;

    struct load_ctx ctx;
    ctx.list = list;

    int count = mpd_db_visit_all(db, mpd_to_musiclist_visitor, &ctx);
    mpd_db_close(db);

    if (count < 0) {
        fprintf(stderr, "[MusicScanner] mpd_db_visit_all failed\n");
        return -1;
    }

    list->state = SCAN_DONE;
    printf("[MusicScanner] MPD DB loaded: %d records from %s\n",
           list->count, db_path);
    return list->count;
}

/*------------------------------------------------------------------------
 * Query APIs — direct SQLite queries against DatabaseSaveSqlite's songs table
 *
 * The songs table schema (from DatabaseSaveSqlite.cxx):
 *   uri, dir_path, filename, mtime, added, duration_ms,
 *   tag_artist, tag_album, tag_title, tag_track, tag_genre,
 *   tag_date, tag_composer, tag_disc, tag_comment, tag_album_artist
 *----------------------------------------------------------------------*/

#include <sqlite3.h>

static void query_row_to_musicinfo(sqlite3_stmt *stmt, MusicInfo *m)
{
    memset(m, 0, sizeof(MusicInfo));

    m->media_type = MEDIA_TYPE_MUSIC;

    const char *v;

    v = (const char *)sqlite3_column_text(stmt, 0); /* uri */
    if (v) strncpy(m->filepath, v, MUSIC_MAX_PATH_LEN - 1);

    v = (const char *)sqlite3_column_text(stmt, 1); /* filename */
    if (v) strncpy(m->filename, v, MUSIC_MAX_TAG_LEN - 1);

    m->duration_ms = sqlite3_column_int(stmt, 2); /* duration_ms */
    m->id3_parsed = sqlite3_column_int(stmt, 3); /* id3_parsed */

    v = (const char *)sqlite3_column_text(stmt, 4); /* tag_title */
    if (v && v[0])
        strncpy(m->title, v, MUSIC_MAX_TAG_LEN - 1);
    else {
        strncpy(m->title, m->filename, MUSIC_MAX_TAG_LEN - 1);
        char *dot = strrchr(m->title, '.');
        if (dot) *dot = '\0';
    }

    v = (const char *)sqlite3_column_text(stmt, 5); /* tag_artist */
    if (v && v[0])
        strncpy(m->artist, v, MUSIC_MAX_TAG_LEN - 1);
    else
        strncpy(m->artist, "Unknown", MUSIC_MAX_TAG_LEN - 1);

    v = (const char *)sqlite3_column_text(stmt, 6); /* tag_album */
    if (v && v[0])
        strncpy(m->album, v, MUSIC_MAX_TAG_LEN - 1);
    else
        strncpy(m->album, "Unknown", MUSIC_MAX_TAG_LEN - 1);

    v = (const char *)sqlite3_column_text(stmt, 7); /* tag_track */
    if (v && v[0])
        m->track_num = atoi(v);

    strncpy(m->device_name, "device1", MUSIC_MAX_PATH_LEN - 1);

    /* folder_path from filepath */
    strncpy(m->folder_path, m->filepath, MUSIC_MAX_PATH_LEN - 1);
    char *sl = strrchr(m->folder_path, '/');
    if (sl)
        *sl = '\0';
    else
        m->folder_path[0] = '\0';
}

static int _sqlite_query(const char *db_path, const char *sql,
                         const char *param, MusicList *list)
{
    if (!db_path || !list) return -1;

    sqlite3 *db = NULL;
    int rc = sqlite3_open_v2(db_path, &db, SQLITE_OPEN_READONLY, NULL);
    if (rc != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return -1;
    }

    sqlite3_stmt *stmt = NULL;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return -1;
    }

    if (param)
        sqlite3_bind_text(stmt, 1, param, -1, SQLITE_TRANSIENT);

    list->count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (music_list_ensure_capacity(list) != 0)
            break;
        MusicInfo *m = &list->items[list->count];
        m->uid = list->count + 1;
        query_row_to_musicinfo(stmt, m);
        m->uid = list->count + 1; /* re-set after memset in query_row_to_musicinfo */
        list->count++;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return list->count;
}

int music_db_query_by_device(const char *db_path, const char *device_name,
                             MusicList *list)
{
    /* Device = URI prefix match (mount point is the path prefix) */
    static const char *sql =
        "SELECT uri, filename, duration_ms, id3_parsed, tag_title, tag_artist, tag_album, tag_track "
        "FROM songs WHERE uri LIKE ? ORDER BY uri";
    char pattern[MUSIC_MAX_PATH_LEN + 4];
    snprintf(pattern, sizeof(pattern), "%s%%", device_name ? device_name : "");
    return _sqlite_query(db_path, sql, pattern, list);
}

int music_db_query_by_title(const char *db_path, const char *title,
                            MusicList *list)
{
    static const char *sql =
        "SELECT uri, filename, duration_ms, id3_parsed, tag_title, tag_artist, tag_album, tag_track "
        "FROM songs WHERE tag_title LIKE ? ORDER BY uri";
    char pattern[MUSIC_MAX_TAG_LEN + 4];
    snprintf(pattern, sizeof(pattern), "%%%s%%", title ? title : "");
    return _sqlite_query(db_path, sql, pattern, list);
}

int music_db_query_by_artist(const char *db_path, const char *artist,
                             MusicList *list)
{
    static const char *sql =
        "SELECT uri, filename, duration_ms, id3_parsed, tag_title, tag_artist, tag_album, tag_track "
        "FROM songs WHERE tag_artist LIKE ? ORDER BY uri";
    char pattern[MUSIC_MAX_TAG_LEN + 4];
    snprintf(pattern, sizeof(pattern), "%%%s%%", artist ? artist : "");
    return _sqlite_query(db_path, sql, pattern, list);
}

int music_db_query_by_album(const char *db_path, const char *album,
                            MusicList *list)
{
    static const char *sql =
        "SELECT uri, filename, duration_ms, id3_parsed, tag_title, tag_artist, tag_album, tag_track "
        "FROM songs WHERE tag_album LIKE ? ORDER BY uri";
    char pattern[MUSIC_MAX_TAG_LEN + 4];
    snprintf(pattern, sizeof(pattern), "%%%s%%", album ? album : "");
    return _sqlite_query(db_path, sql, pattern, list);
}

int music_db_query_by_filepath(const char *db_path, const char *filepath,
                               MusicList *list)
{
    static const char *sql =
        "SELECT uri, filename, duration_ms, id3_parsed, tag_title, tag_artist, tag_album, tag_track "
        "FROM songs WHERE uri = ? LIMIT 1";
    return _sqlite_query(db_path, sql, filepath, list);
}
/*============================================================================
 * Direct scan → MPD tree (requirement #1: no MusicList intermediate)
 *
 * Each discovered audio file is parsed for ID3 tags and immediately
 * fed to mpd_db_add_song(). No malloc/realloc of a flat MusicInfo array.
 * MPD's C++ STL (std::unique_ptr<Song>, IntrusiveList) manages all memory.
 *==========================================================================*/

struct scan_to_mpd_ctx {
    mpd_db_t *db;
    const char *device_name;
    int count;
    int errors;
};

static int scan_dir_to_mpd_recursive(struct scan_to_mpd_ctx *ctx,
                                      const char *dir_path, int depth)
{
    if (depth > MAX_SCAN_DEPTH) return 0;

    DIR *dir = opendir(dir_path);
    if (!dir) return -1;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char fullpath[MUSIC_MAX_PATH_LEN];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (lstat(fullpath, &st) != 0) continue;
        if (S_ISLNK(st.st_mode)) continue;

        if (S_ISDIR(st.st_mode)) {
            if (strstr(fullpath, "/Android") ||
                strstr(fullpath, "/LOST.DIR") ||
                strstr(fullpath, "/System Volume Information") ||
                strstr(fullpath, "/DCIM")) {
                continue;
            }

            char nomedia[MUSIC_MAX_PATH_LEN];
            snprintf(nomedia, sizeof(nomedia), "%s/.nomedia", fullpath);
            struct stat nm_st;
            if (stat(nomedia, &nm_st) == 0) continue;

            scan_dir_to_mpd_recursive(ctx, fullpath, depth + 1);
        } else if (S_ISREG(st.st_mode)) {
            if (!music_is_audio_file(entry->d_name)) continue;

            /* Parse ID3 into a temporary MusicInfo on the stack — no heap alloc */
            MusicInfo info;
            memset(&info, 0, sizeof(info));
            strncpy(info.filepath, fullpath, MUSIC_MAX_PATH_LEN - 1);
            strncpy(info.filename, entry->d_name, MUSIC_MAX_TAG_LEN - 1);
            info.file_size = (uint32_t)st.st_size;

            char ext[16];
            str_to_lower(ext, get_extension(entry->d_name), sizeof(ext));

            if (strcmp(ext, MUSIC_EXT_MP3) == 0) {
                if (music_parse_id3v2(fullpath, &info) == 0) {
#ifdef USE_CMUS_ID3
                    info.id3_parsed = 2;
#else
                    info.id3_parsed = 1;
#endif
                } else {
                    strncpy(info.title, entry->d_name, MUSIC_MAX_TAG_LEN - 1);
                    char *dot = strrchr(info.title, '.');
                    if (dot) *dot = '\0';
                }
            } else {
                strncpy(info.title, entry->d_name, MUSIC_MAX_TAG_LEN - 1);
                char *dot = strrchr(info.title, '.');
                if (dot) *dot = '\0';
            }

            if (info.title[0] == '\0') {
                strncpy(info.title, entry->d_name, MUSIC_MAX_TAG_LEN - 1);
                char *dot = strrchr(info.title, '.');
                if (dot) *dot = '\0';
            }
            if (info.artist[0] == '\0')
                strncpy(info.artist, "Unknown", MUSIC_MAX_TAG_LEN - 1);
            if (info.album[0] == '\0')
                strncpy(info.album, "Unknown", MUSIC_MAX_TAG_LEN - 1);

            /* Feed directly to MPD tree — no MusicList intermediate */
            mpd_song_info_t si;
            memset(&si, 0, sizeof(si));
            si.uri = info.filepath;
            si.mtime = st.st_mtime;
            si.duration_ms = info.duration_ms;
            si.id3_parsed = info.id3_parsed;

            char track_str[16];
            if (info.track_num > 0) {
                snprintf(track_str, sizeof(track_str), "%d", info.track_num);
                si.tags[MPD_TAG_TRACK] = track_str;
            }
            si.tags[MPD_TAG_TITLE]  = info.title;
            si.tags[MPD_TAG_ARTIST] = info.artist;
            si.tags[MPD_TAG_ALBUM]  = info.album;

            if (mpd_db_add_song(ctx->db, &si) == 0) {
                ctx->count++;
                printf("[MusicScanner] [%d] %s - %s (%s) id3=%d\n",
                       ctx->count, info.artist, info.title, info.filepath,
                       info.id3_parsed);
            } else {
                ctx->errors++;
            }
        }
    }

    closedir(dir);
    return 0;
}

int music_scan_to_mpd_db(mpd_db_t *db, const char *dir_path)
{
    if (!db || !dir_path) return -1;

    printf("[MusicScanner] Direct scan → MPD tree: %s\n", dir_path);

    struct scan_to_mpd_ctx ctx;
    ctx.db = db;
    ctx.device_name = dir_path;
    ctx.count = 0;
    ctx.errors = 0;

    int ret = scan_dir_to_mpd_recursive(&ctx, dir_path, 0);
    if (ret != 0) return -1;

    printf("[MusicScanner] Scan complete: %d songs added, %d errors\n",
           ctx.count, ctx.errors);
    return ctx.count;
}

/*--- Cancellable version ---*/

static int scan_dir_to_mpd_cancellable_recursive(
        struct scan_to_mpd_ctx *ctx,
        const char *dir_path, int depth,
        const char *mount_root,
        const volatile int *cancel_flag,
        int expected_gen,
        scan_progress_fn progress_cb,
        void *cb_ctx)
{
    if (depth > MAX_SCAN_DEPTH) return 0;

    if (cancel_flag && *cancel_flag != expected_gen) return -2;
    if (!is_mount_alive(mount_root)) return -2;

    DIR *dir = opendir(dir_path);
    if (!dir) {
        if (errno == ENOENT || errno == EACCES || errno == EIO) return -2;
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        if (ctx->count > 0 && (ctx->count % SCAN_PROGRESS_INTERVAL) == 0) {
            if (cancel_flag && *cancel_flag != expected_gen) {
                closedir(dir); return -2;
            }
            if (!is_mount_alive(mount_root)) {
                closedir(dir); return -2;
            }
            if (progress_cb && progress_cb(ctx->count, cb_ctx) != 0) {
                closedir(dir); return -2;
            }
        }

        char fullpath[MUSIC_MAX_PATH_LEN];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (lstat(fullpath, &st) != 0) continue;
        if (S_ISLNK(st.st_mode)) continue;

        if (S_ISDIR(st.st_mode)) {
            if (strstr(fullpath, "/Android") ||
                strstr(fullpath, "/LOST.DIR") ||
                strstr(fullpath, "/System Volume Information") ||
                strstr(fullpath, "/DCIM")) {
                continue;
            }
            char nomedia[MUSIC_MAX_PATH_LEN];
            snprintf(nomedia, sizeof(nomedia), "%s/.nomedia", fullpath);
            struct stat nm_st;
            if (stat(nomedia, &nm_st) == 0) continue;

            int ret = scan_dir_to_mpd_cancellable_recursive(
                ctx, fullpath, depth + 1,
                mount_root, cancel_flag, expected_gen,
                progress_cb, cb_ctx);
            if (ret == -2) { closedir(dir); return -2; }
        } else if (S_ISREG(st.st_mode)) {
            if (!music_is_audio_file(entry->d_name)) continue;

            MusicInfo info;
            memset(&info, 0, sizeof(info));
            strncpy(info.filepath, fullpath, MUSIC_MAX_PATH_LEN - 1);
            strncpy(info.filename, entry->d_name, MUSIC_MAX_TAG_LEN - 1);
            info.file_size = (uint32_t)st.st_size;

            char ext[16];
            str_to_lower(ext, get_extension(entry->d_name), sizeof(ext));

            if (strcmp(ext, MUSIC_EXT_MP3) == 0) {
                if (music_parse_id3v2(fullpath, &info) == 0) {
#ifdef USE_CMUS_ID3
                    info.id3_parsed = 2;
#else
                    info.id3_parsed = 1;
#endif
                } else {
                    strncpy(info.title, entry->d_name, MUSIC_MAX_TAG_LEN - 1);
                    char *dot = strrchr(info.title, '.'); if (dot) *dot = '\0';
                }
            } else {
                strncpy(info.title, entry->d_name, MUSIC_MAX_TAG_LEN - 1);
                char *dot = strrchr(info.title, '.'); if (dot) *dot = '\0';
            }

            if (info.title[0] == '\0') {
                strncpy(info.title, entry->d_name, MUSIC_MAX_TAG_LEN - 1);
                char *dot = strrchr(info.title, '.'); if (dot) *dot = '\0';
            }
            if (info.artist[0] == '\0')
                strncpy(info.artist, "Unknown", MUSIC_MAX_TAG_LEN - 1);
            if (info.album[0] == '\0')
                strncpy(info.album, "Unknown", MUSIC_MAX_TAG_LEN - 1);

            mpd_song_info_t si;
            memset(&si, 0, sizeof(si));
            si.uri = info.filepath;
            si.mtime = st.st_mtime;
            si.duration_ms = info.duration_ms;
            si.id3_parsed = info.id3_parsed;

            char track_str[16];
            if (info.track_num > 0) {
                snprintf(track_str, sizeof(track_str), "%d", info.track_num);
                si.tags[MPD_TAG_TRACK] = track_str;
            }
            si.tags[MPD_TAG_TITLE]  = info.title;
            si.tags[MPD_TAG_ARTIST] = info.artist;
            si.tags[MPD_TAG_ALBUM]  = info.album;

            if (mpd_db_add_song(ctx->db, &si) == 0)
                ctx->count++;
            else
                ctx->errors++;
        }
    }

    closedir(dir);
    return 0;
}

int music_scan_to_mpd_db_cancellable(
        mpd_db_t *db,
        const char *dir_path,
        const volatile int *cancel_flag,
        int expected_gen,
        scan_progress_fn progress_cb,
        void *cb_ctx)
{
    if (!db || !dir_path) return -1;

    printf("[MusicScanner] Direct cancellable scan → MPD tree: %s (gen=%d)\n",
           dir_path, expected_gen);

    struct scan_to_mpd_ctx ctx;
    ctx.db = db;
    ctx.device_name = dir_path;
    ctx.count = 0;
    ctx.errors = 0;

    int ret = scan_dir_to_mpd_cancellable_recursive(
        &ctx, dir_path, 0,
        dir_path, cancel_flag, expected_gen,
        progress_cb, cb_ctx);

    if (ret == -2) {
        printf("[MusicScanner] Scan cancelled at %d songs\n", ctx.count);
        return -2;
    }

    printf("[MusicScanner] Scan complete: %d songs, %d errors\n",
           ctx.count, ctx.errors);
    return ctx.count;
}

/*============================================================================
 * Page-based loading: MPD tree → MusicList (one page at a time)
 *
 * 翻页设计:
 *   - UI 请求第 N 页 → music_db_load_page(db, N, 20, list)
 *   - MPD 树内部 skip 前 N*20 首, 取 20 首通过 visitor 回调
 *   - MusicList 只持有当前页的 20 首 (capacity=page_size)
 *   - 翻到下一页时, list->count 重置为 0, 复用同一个 MusicList
 *
 * 内存占用: 20 * sizeof(MusicInfo) ≈ 50KB, 而非 10000 * 2.5KB = 25MB
 *==========================================================================*/

static int page_visitor(const mpd_song_info_t *song, void *user_data)
{
    MusicList *list = (MusicList *)user_data;

    if (list->count >= list->capacity)
        return -1; /* page full, stop */

    MusicInfo *m = &list->items[list->count];
    memset(m, 0, sizeof(MusicInfo));

    m->uid = list->count + 1;
    m->media_type = MEDIA_TYPE_MUSIC;

    if (song->uri)
        strncpy(m->filepath, song->uri, MUSIC_MAX_PATH_LEN - 1);

    const char *fn = get_filename(m->filepath);
    if (fn)
        strncpy(m->filename, fn, MUSIC_MAX_TAG_LEN - 1);

    strncpy(m->device_name, "device1", MUSIC_MAX_PATH_LEN - 1);

    /* folder_path = filepath truncated at last '/' */
    strncpy(m->folder_path, m->filepath, MUSIC_MAX_PATH_LEN - 1);
    char *last_slash = strrchr(m->folder_path, '/');
    if (last_slash) *last_slash = '\0';
    else m->folder_path[0] = '\0';

    m->duration_ms = song->duration_ms;
    m->id3_parsed = song->id3_parsed;

    if (song->tags[MPD_TAG_TITLE] && song->tags[MPD_TAG_TITLE][0])
        strncpy(m->title, song->tags[MPD_TAG_TITLE], MUSIC_MAX_TAG_LEN - 1);
    else {
        strncpy(m->title, m->filename, MUSIC_MAX_TAG_LEN - 1);
        char *dot = strrchr(m->title, '.');
        if (dot) *dot = '\0';
    }

    if (song->tags[MPD_TAG_ARTIST] && song->tags[MPD_TAG_ARTIST][0])
        strncpy(m->artist, song->tags[MPD_TAG_ARTIST], MUSIC_MAX_TAG_LEN - 1);
    else
        strncpy(m->artist, "Unknown", MUSIC_MAX_TAG_LEN - 1);

    if (song->tags[MPD_TAG_ALBUM] && song->tags[MPD_TAG_ALBUM][0])
        strncpy(m->album, song->tags[MPD_TAG_ALBUM], MUSIC_MAX_TAG_LEN - 1);
    else
        strncpy(m->album, "Unknown", MUSIC_MAX_TAG_LEN - 1);

    if (song->tags[MPD_TAG_TRACK] && song->tags[MPD_TAG_TRACK][0])
        m->track_num = atoi(song->tags[MPD_TAG_TRACK]);

    list->count++;
    return 0;
}

int music_db_load_page(mpd_db_t *db, int page, int page_size, MusicList *list)
{
    if (!db || !list || page < 0 || page_size <= 0) return -1;

    /* Reset count but keep the existing items buffer */
    list->count = 0;

    int visited = mpd_db_visit_page(db, page, page_size, page_visitor, list);

    list->state = SCAN_DONE;
    printf("[MusicScanner] Page %d loaded: %d songs (page_size=%d)\n",
           page, list->count, page_size);
    return visited;
}

int music_db_get_total_count(mpd_db_t *db)
{
    if (!db) return 0;
    return mpd_db_song_count(db);
}

#endif /* USE_MPD_DB */
