/*
 * music_scanner.h - Local music file scanner with ID3v2 tag parsing
 *
 * Scans directories (USB/SD) for audio files and extracts metadata.
 * No external library dependency - parses ID3v2 headers directly.
 *
 * Reference: Android AutoMediaPlayer media-data/room/MediaInfo.java
 *
 * Copyright (c) 2026. All rights reserved.
 */

#ifndef MUSIC_SCANNER_H
#define MUSIC_SCANNER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum counts */
/* Initial capacity — grows dynamically via realloc (no hard limit) */
#define MUSIC_INIT_CAPACITY   512
#define MUSIC_GROW_FACTOR     2    /* double on each realloc */
#define MUSIC_MAX_FILES       65536  /* safety ceiling to prevent OOM on 512MB DDR */
#define MUSIC_MAX_PATH_LEN    512
#define MUSIC_MAX_TAG_LEN     256
#define MUSIC_DB_PATH         "/data/music/music.db"
#define MUSIC_DB_DIR          "/data/music"

/* Supported audio extensions */
#define MUSIC_EXT_MP3         ".mp3"
#define MUSIC_EXT_WAV         ".wav"
#define MUSIC_EXT_FLAC        ".flac"
#define MUSIC_EXT_AAC         ".aac"
#define MUSIC_EXT_OGG         ".ogg"
#define MUSIC_EXT_WMA         ".wma"
#define MUSIC_EXT_M4A         ".m4a"

/* Media type (aligned with Android MediaInfo.mediaType) */
typedef enum {
    MEDIA_TYPE_MUSIC = 0,
    MEDIA_TYPE_VIDEO = 1,
    MEDIA_TYPE_IMAGE = 2,
} MediaType;

/* Scan state */
typedef enum {
    SCAN_IDLE = 0,
    SCAN_SCANNING,
    SCAN_DONE,
    SCAN_ERROR,
} ScanState;

/*
 * MusicInfo - per-file metadata
 * Aligned with Android MediaInfo fields:
 *   title, artist, album, duration, filepath, name, deviceName
 */
typedef struct {
    int          uid;                           /* unique ID (auto-increment) */
    MediaType    media_type;                    /* always MEDIA_TYPE_MUSIC */
    char         title[MUSIC_MAX_TAG_LEN];      /* ID3 title or filename */
    char         artist[MUSIC_MAX_TAG_LEN];     /* ID3 artist */
    char         album[MUSIC_MAX_TAG_LEN];      /* ID3 album */
    int          duration_ms;                   /* duration in milliseconds */
    int          track_num;                     /* track number */
    char         filepath[MUSIC_MAX_PATH_LEN];  /* absolute path */
    char         filename[MUSIC_MAX_TAG_LEN];   /* file name only */
    char         device_name[MUSIC_MAX_PATH_LEN]; /* mount point e.g. /mnt/usb */
    char         folder_path[MUSIC_MAX_PATH_LEN]; /* parent directory path */
    uint32_t     file_size;                     /* bytes */
    int          folder_index;  /* -1 = this is a folder entry, >=0 = file */
} MusicInfo;

/*
 * MusicList - scan result container
 */
typedef struct {
    MusicInfo   *items;
    int          count;
    int          capacity;
    ScanState    state;
    char         scan_path[MUSIC_MAX_PATH_LEN];
} MusicList;

/* --- Scanner API --- */

/* Create a music list (allocates memory). Returns NULL on failure. */
MusicList *music_list_create(int capacity);

/* Destroy a music list (frees all memory). */
void music_list_destroy(MusicList *list);

/* Scan a directory recursively for audio files.
 * Populates list->items with MusicInfo entries.
 * Blocks until scan completes. Thread-safe if different lists used. */
int music_scan_directory(MusicList *list, const char *dir_path);

/* Check if a filename has a supported audio extension. */
bool music_is_audio_file(const char *filename);

/* --- ID3 parsing API --- */

/* Parse ID3v2 tags from an MP3 file. Fills title/artist/album fields.
 * Returns 0 on success, -1 if no ID3 tag found (falls back to filename). */
int music_parse_id3v2(const char *filepath, MusicInfo *info);

/* --- Simple flat-file database --- */

/* Save music list to a text-based DB file.
 * Format: one record per line, fields separated by \t */
int music_db_save(const MusicList *list, const char *db_path);

/* Load music list from DB file. Returns count loaded, -1 on error. */
int music_db_load(MusicList *list, const char *db_path);

#ifdef __cplusplus
}
#endif

#endif /* MUSIC_SCANNER_H */
