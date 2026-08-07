/*
 * music_scanner.c - Local music file scanner with ID3v2 tag parsing
 *
 * Pure C implementation, no external library dependencies.
 * Parses ID3v2.3/2.4 headers for title (TIT2), artist (TPE1), album (TALB).
 *
 * Reference: Android AutoMediaPlayer media-data module
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
}

/* ---------- Directory scanner ---------- */

static int uid_counter = 0;

static int scan_dir_recursive(MusicList *list, const char *dir_path,
                              const char *device_name)
{
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

        /* Build full path */
        char fullpath[MUSIC_MAX_PATH_LEN];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (stat(fullpath, &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            /* Recurse into subdirectory */
            scan_dir_recursive(list, fullpath, device_name);
        } else if (S_ISREG(st.st_mode)) {
            if (!music_is_audio_file(entry->d_name)) continue;
            if (list->count >= list->capacity) {
                fprintf(stderr, "[MusicScanner] Capacity reached: %d\n",
                        list->capacity);
                break;
            }

            MusicInfo *info = &list->items[list->count];
            memset(info, 0, sizeof(MusicInfo));

            info->uid = ++uid_counter;
            info->media_type = MEDIA_TYPE_MUSIC;
            info->file_size = (uint32_t)st.st_size;
            strncpy(info->filepath, fullpath, MUSIC_MAX_PATH_LEN - 1);
            strncpy(info->filename, entry->d_name, MUSIC_MAX_TAG_LEN - 1);
            strncpy(info->device_name, device_name, MUSIC_MAX_PATH_LEN - 1);

            /* Try to parse ID3v2 tags (MP3 only for now) */
            char ext[16];
            str_to_lower(ext, get_extension(entry->d_name), sizeof(ext));

            if (strcmp(ext, MUSIC_EXT_MP3) == 0) {
                if (music_parse_id3v2(fullpath, info) != 0) {
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
            printf("[MusicScanner] [%d] %s - %s (%s)\n",
                   info->uid, info->artist, info->title, info->filepath);
        }
    }

    closedir(dir);
    return 0;
}

MusicList *music_list_create(int capacity)
{
    if (capacity <= 0 || capacity > MUSIC_MAX_FILES)
        capacity = MUSIC_MAX_FILES;

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

    int ret = scan_dir_recursive(list, dir_path, dir_path);

    list->state = (ret == 0) ? SCAN_DONE : SCAN_ERROR;
    printf("[MusicScanner] Scan complete: %d files found\n", list->count);

    return ret;
}

/* ---------- Simple flat-file database ---------- */

int music_db_save(const MusicList *list, const char *db_path)
{
    if (!list || !db_path) return -1;

    /* Ensure directory exists */
    char dir[MUSIC_MAX_PATH_LEN];
    strncpy(dir, db_path, MUSIC_MAX_PATH_LEN - 1);
    char *slash = strrchr(dir, '/');
    if (slash) {
        *slash = '\0';
        /* Simple mkdir -p (one level) */
        mkdir(dir, 0755);
    }

    FILE *fp = fopen(db_path, "w");
    if (!fp) {
        fprintf(stderr, "[MusicScanner] Cannot create DB: %s (%s)\n",
                db_path, strerror(errno));
        return -1;
    }

    /* Header */
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
    printf("[MusicScanner] DB saved: %d records -> %s\n", list->count, db_path);
    return 0;
}

int music_db_load(MusicList *list, const char *db_path)
{
    if (!list || !db_path) return -1;

    FILE *fp = fopen(db_path, "r");
    if (!fp) return -1;

    char line[2048];
    list->count = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#') continue; /* skip header */
        if (list->count >= list->capacity) break;

        MusicInfo *m = &list->items[list->count];
        memset(m, 0, sizeof(MusicInfo));

        int n = sscanf(line, "%d\t%d\t", &m->uid, (int*)&m->media_type);
        if (n < 2) continue;

        /* Parse tab-separated fields manually for strings with spaces */
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
            m->file_size = (uint32_t)atol(fields[10]);
            list->count++;
        }
    }

    fclose(fp);
    list->state = SCAN_DONE;
    printf("[MusicScanner] DB loaded: %d records from %s\n", list->count, db_path);
    return list->count;
}
