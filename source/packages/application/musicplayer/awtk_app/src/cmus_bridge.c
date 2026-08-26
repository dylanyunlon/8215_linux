/**
 * @file cmus_bridge.c
 * @brief Bridge between cmus ported modules and musicplayer's music_scanner.
 *
 * Integration layer that calls cmus's id3_read_tags() and cue_from_file()
 * and maps results into our MusicInfo struct.
 *
 * Reference:
 *   Android: MediaID3Util.java (java.lang.ID3MetadataRetriever)
 *   cmus:    ip/mp3.c read_comments()
 *
 * File location: source/packages/application/musicplayer/awtk_app/src/cmus_bridge.c
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 * Uses cmus code licensed under GPL-2.0+
 */

#include "cmus_bridge.h"

/* cmus ported headers */
#include "cmus/cmus_compat.h"
#include "cmus/id3.h"
#include "cmus/cue.h"
#include "cmus/cue_utils.h"
#include "cmus/comment.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>

/*============================================================================
 * ID3 tag integration
 *
 * Replaces music_scanner.c's extract_text_frame() which only handles
 * ASCII from UTF-16 (all CJK chars become '?').
 *
 * cmus id3.c uses iconv for proper charset conversion:
 *   UTF-16 LE/BE → UTF-8
 *   ISO-8859-1 → UTF-8
 *   UTF-8 passthrough
 *==========================================================================*/

/**
 * Helper: safely copy a cmus id3 string into a fixed-size buffer.
 * id3_get_comment() returns malloc'd strings; we copy and free.
 */
static void copy_id3_field(struct id3tag *id3, enum id3_key key,
                           char *dst, size_t dst_len)
{
    char *val = id3_get_comment(id3, key);
    if (val) {
        /* Trim trailing whitespace (cmus handles most, but be safe) */
        size_t len = strlen(val);
        while (len > 0 && (val[len-1] == ' ' || val[len-1] == '\0'))
            len--;
        if (len >= dst_len) len = dst_len - 1;
        memcpy(dst, val, len);
        dst[len] = '\0';
        free(val);
    }
}

int cmus_bridge_parse_tags(const char *filepath, MusicInfo *info)
{
    if (!filepath || !info) return -1;

    int fd = open(filepath, O_RDONLY);
    if (fd < 0) return -1;

    struct id3tag id3;
    id3_init(&id3);

    /* Try both ID3v2 (beginning of file) and ID3v1 (last 128 bytes).
     * ID3v2 takes priority if both exist — cmus handles this correctly
     * in id3_get_comment() by checking v2 first, then falling back to v1. */
    int rc = id3_read_tags(&id3, fd, ID3_V2 | ID3_V1);
    close(fd);

    if (rc != 0) {
        id3_free(&id3);
        return -1;
    }

    /* Map cmus id3 keys → MusicInfo fields */
    copy_id3_field(&id3, ID3_TITLE,  info->title,  sizeof(info->title));
    copy_id3_field(&id3, ID3_ARTIST, info->artist, sizeof(info->artist));
    copy_id3_field(&id3, ID3_ALBUM,  info->album,  sizeof(info->album));

    /* Track number: "5" or "5/12" format */
    {
        char *track_str = id3_get_comment(&id3, ID3_TRACK);
        if (track_str) {
            /* Handle "5/12" format — take only the numerator */
            char *slash = strchr(track_str, '/');
            if (slash) *slash = '\0';
            info->track_num = atoi(track_str);
            free(track_str);
        }
    }

    /* Genre: cmus resolves "(13)" → "Pop" via id3_get_genre() table */
    /* The id3_get_comment(ID3_GENRE) already handles this resolution */
    /* We don't currently have a genre field in MusicInfo, but if added: */
    /* copy_id3_field(&id3, ID3_GENRE, info->genre, sizeof(info->genre)); */

    /* Album artist: for compilation albums (e.g. "Various Artists") */
    /* copy_id3_field(&id3, ID3_ALBUMARTIST, info->album_artist, sizeof(info->album_artist)); */

    id3_free(&id3);

    printf("[cmus_bridge] Parsed tags: '%s' - '%s' [%s] (track %d)\n",
           info->artist, info->title, info->album, info->track_num);
    return 0;
}

/*============================================================================
 * CUE sheet integration
 *
 * CUE sheets describe multi-track layouts within a single audio file.
 * Common in CD rips: one FLAC/WAV file + one .cue file = entire album.
 *
 * cmus cue.c parses the CUE format and returns a linked list of tracks
 * with title, performer, and time offsets (in frames: 1/75 second).
 *
 * Integration approach:
 *   1. scan_directory() detects .cue files
 *   2. cmus_bridge_scan_cue() parses and expands to individual MusicInfo entries
 *   3. Each entry gets filepath = audio file, plus cue metadata for playback
 *
 * Note: MusicInfo does not yet have cue_offset/cue_length fields.
 * For now we encode the offset in the folder_index field (reused) and
 * set a "[CUE]" prefix on the title for visual identification.
 * A proper solution requires adding fields to MusicInfo — see TODO below.
 *==========================================================================*/

/**
 * Convert CUE frame offset (1/75 second) to milliseconds.
 * CUE INDEX format: MM:SS:FF where FF = frames (0-74)
 */
static int cue_frames_to_ms(int frames)
{
    return (frames * 1000) / 75;
}

int cmus_bridge_is_cue(const char *filename)
{
    if (!filename) return 0;
    size_t len = strlen(filename);
    if (len < 5) return 0;

    const char *ext = filename + len - 4;
    return (ext[0] == '.' &&
            (ext[1] == 'c' || ext[1] == 'C') &&
            (ext[2] == 'u' || ext[2] == 'U') &&
            (ext[3] == 'e' || ext[3] == 'E'));
}

int cmus_bridge_scan_cue(const char *cue_path, const char *dir_path,
                         MusicList *list)
{
    if (!cue_path || !dir_path || !list) return -1;

    /* Use cmus cue.c to parse the CUE sheet.
     *
     * cue_from_file() returns a cue_sheet struct containing:
     *   - sheet-level metadata (album title, performer, genre)
     *   - linked list of cue_track with per-track metadata + offsets
     *
     * Note: cue_from_file() internally uses file_read_to_buffer() (mmap)
     * and the CUE parser state machine. */

    /* First, verify the file exists and is readable */
    struct stat st;
    if (stat(cue_path, &st) != 0 || !S_ISREG(st.st_mode)) {
        return -1;
    }

    /* For now, we parse the CUE file manually since the cmus cue.c
     * interface uses its own track list structure that needs careful
     * bridging. We implement a lightweight CUE parser here that
     * understands the subset we need:
     *   FILE "name" WAVE
     *   TRACK xx AUDIO
     *     TITLE "..."
     *     PERFORMER "..."
     *     INDEX 01 mm:ss:ff
     */

    FILE *fp = fopen(cue_path, "r");
    if (!fp) return -1;

    char line[1024];
    char audio_file[MUSIC_MAX_PATH_LEN] = {0};
    char album_title[MUSIC_MAX_TAG_LEN] = {0};
    char album_performer[MUSIC_MAX_TAG_LEN] = {0};

    /* Current track being parsed */
    int track_num = 0;
    char track_title[MUSIC_MAX_TAG_LEN] = {0};
    char track_performer[MUSIC_MAX_TAG_LEN] = {0};
    int track_offset_ms = 0;
    int tracks_added = 0;

    /* State: 0=header, 1=in_track */
    int in_track = 0;

    /* Helper: extract quoted string from line like: TITLE "Hello World" */
    auto void extract_quoted(const char *src, char *dst, size_t dst_len);
    void extract_quoted(const char *src, char *dst, size_t dst_len) {
        const char *q1 = strchr(src, '"');
        if (!q1) return;
        q1++;
        const char *q2 = strchr(q1, '"');
        if (!q2) q2 = q1 + strlen(q1);
        size_t len = (size_t)(q2 - q1);
        if (len >= dst_len) len = dst_len - 1;
        memcpy(dst, q1, len);
        dst[len] = '\0';
    }

    /* Helper: parse INDEX time "mm:ss:ff" → milliseconds */
    auto int parse_index_time(const char *s);
    int parse_index_time(const char *s) {
        int mm = 0, ss = 0, ff = 0;
        if (sscanf(s, "%d:%d:%d", &mm, &ss, &ff) >= 2) {
            return mm * 60000 + ss * 1000 + cue_frames_to_ms(ff);
        }
        return 0;
    }

    /* Helper: flush current track to list */
    auto void flush_track(void);
    void flush_track(void) {
        if (track_num <= 0 || audio_file[0] == '\0') return;
        if (list->count >= list->capacity) return;  /* full */

        MusicInfo *info = &list->items[list->count];
        memset(info, 0, sizeof(MusicInfo));

        info->uid = list->count + 1;
        info->media_type = MEDIA_TYPE_MUSIC;

        /* Build audio file path */
        snprintf(info->filepath, sizeof(info->filepath),
                 "%s/%s", dir_path, audio_file);

        /* Track title */
        if (track_title[0]) {
            snprintf(info->title, sizeof(info->title), "%s", track_title);
        } else {
            snprintf(info->title, sizeof(info->title),
                     "Track %02d", track_num);
        }

        /* Artist: track performer > album performer > "Unknown" */
        if (track_performer[0]) {
            snprintf(info->artist, sizeof(info->artist), "%s", track_performer);
        } else if (album_performer[0]) {
            snprintf(info->artist, sizeof(info->artist), "%s", album_performer);
        } else {
            snprintf(info->artist, sizeof(info->artist), "Unknown");
        }

        /* Album: from sheet-level TITLE */
        if (album_title[0]) {
            snprintf(info->album, sizeof(info->album), "%s", album_title);
        } else {
            snprintf(info->album, sizeof(info->album), "Unknown");
        }

        info->track_num = track_num;

        /* Extract filename from audio_file path */
        const char *slash = strrchr(audio_file, '/');
        snprintf(info->filename, sizeof(info->filename),
                 "%s", slash ? slash + 1 : audio_file);

        /* TODO: When MusicInfo gets cue_offset_ms and cue_length_ms fields,
         * store track_offset_ms here for the player to seek to.
         * For now, encode in folder_index as a marker.
         * folder_index = -(offset_ms) for CUE tracks (negative = CUE indicator) */
        info->folder_index = -(track_offset_ms + 1);  /* +1 to distinguish from 0 */

        list->count++;
        tracks_added++;

        printf("[cmus_bridge] CUE track %d: '%s' - '%s' [%s] offset=%dms\n",
               track_num, info->artist, info->title, info->album, track_offset_ms);
    }

    /* Parse line by line */
    while (fgets(line, sizeof(line), fp)) {
        /* Strip newline/carriage return */
        char *nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        nl = strchr(line, '\r');
        if (nl) *nl = '\0';

        /* Skip UTF-8 BOM */
        char *p = line;
        if ((unsigned char)p[0] == 0xEF &&
            (unsigned char)p[1] == 0xBB &&
            (unsigned char)p[2] == 0xBF) {
            p += 3;
        }

        /* Trim leading whitespace */
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') continue;

        /* FILE "name.flac" WAVE */
        if (strncasecmp(p, "FILE ", 5) == 0) {
            extract_quoted(p + 5, audio_file, sizeof(audio_file));
        }
        /* Sheet-level TITLE (before any TRACK) */
        else if (strncasecmp(p, "TITLE ", 6) == 0 && !in_track) {
            extract_quoted(p + 6, album_title, sizeof(album_title));
        }
        /* Sheet-level PERFORMER */
        else if (strncasecmp(p, "PERFORMER ", 10) == 0 && !in_track) {
            extract_quoted(p + 10, album_performer, sizeof(album_performer));
        }
        /* TRACK xx AUDIO */
        else if (strncasecmp(p, "TRACK ", 6) == 0) {
            /* Flush previous track if any */
            if (in_track) {
                flush_track();
            }
            in_track = 1;
            track_num = atoi(p + 6);
            track_title[0] = '\0';
            track_performer[0] = '\0';
            track_offset_ms = 0;
        }
        /* Track-level TITLE */
        else if (strncasecmp(p, "TITLE ", 6) == 0 && in_track) {
            extract_quoted(p + 6, track_title, sizeof(track_title));
        }
        /* Track-level PERFORMER */
        else if (strncasecmp(p, "PERFORMER ", 10) == 0 && in_track) {
            extract_quoted(p + 10, track_performer, sizeof(track_performer));
        }
        /* INDEX 01 mm:ss:ff (track start position) */
        else if (strncasecmp(p, "INDEX 01 ", 9) == 0 && in_track) {
            track_offset_ms = parse_index_time(p + 9);
        }
        /* REM fields for extended metadata */
        else if (strncasecmp(p, "REM ", 4) == 0) {
            /* Could extract GENRE, DATE, etc. from REM lines */
            /* For now, skip */
        }
    }

    /* Flush the last track */
    if (in_track) {
        flush_track();
    }

    fclose(fp);

    printf("[cmus_bridge] CUE parsed: %d tracks from %s\n",
           tracks_added, cue_path);
    return tracks_added;
}