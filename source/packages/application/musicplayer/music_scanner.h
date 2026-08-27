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
    int          id3_parsed;    /* 0=not parsed, 1=ID3v2(software), 2=cmus(hardware-assisted) */
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

/**
 * Cancellable scan with incremental progress callback.
 *
 * Solves two production problems:
 *   1. "一万首歌怎么办" — callback fires every N files so UI can show progress
 *   2. "扫到一半拔U盘"  — cancel_flag checked every iteration + mount check
 *
 * @param list          Output list (will be cleared first)
 * @param dir_path      Root scan path (e.g. "/mnt/usb0")
 * @param cancel_flag   Pointer to volatile int; set non-zero to abort scan.
 *                      Typically points to storage_device_state_t.scan_generation
 *                      — if it changes, scan was superseded.
 * @param expected_gen  The scan_generation value at dispatch time.
 *                      Scan aborts if *cancel_flag != expected_gen.
 * @param progress_cb   Called every SCAN_PROGRESS_INTERVAL files (may be NULL).
 *                      Receives current file count. Return non-zero to abort.
 * @param cb_ctx        Opaque context passed to progress_cb.
 * @return  Number of files found, or -1 on error, -2 on cancelled.
 */
#define SCAN_PROGRESS_INTERVAL  50  /* report every 50 files */

typedef int (*scan_progress_fn)(int current_count, void *ctx);

int music_scan_directory_cancellable(
        MusicList *list,
        const char *dir_path,
        const volatile int *cancel_flag,
        int expected_gen,
        scan_progress_fn progress_cb,
        void *cb_ctx);

/* Check if a filename has a supported audio extension. */
bool music_is_audio_file(const char *filename);

/* --- ID3 parsing API --- */

/* Parse ID3v2 tags from an MP3 file. Fills title/artist/album fields.
 * Returns 0 on success, -1 if no ID3 tag found (falls back to filename). */
int music_parse_id3v2(const char *filepath, MusicInfo *info);

/* --- Flat-file database (legacy, retained for fallback) --- */

/* Save music list to a text-based DB file.
 * Format: one record per line, fields separated by \t
 * DEPRECATED: Use music_db_save() which now routes to SQLite internally.
 *             This function is kept only for emergency fallback. */
int music_db_save_text(const MusicList *list, const char *db_path);

/* Load music list from text DB file.
 * DEPRECATED: Use music_db_load() which now routes to SQLite internally. */
int music_db_load_text(MusicList *list, const char *db_path);

/* --- Database (via MPD bridge + SQLite backend) --- */

/**
 * Save music list to database via MPD bridge.
 *
 * Data path: MusicList → mpd_db_add_song() → MPD tree
 *                      → mpd_db_save() → DatabaseSaveSqlite → SQLite
 *
 * - Single transaction for atomicity (10000 songs ~200ms)
 * - WAL journal mode for crash safety
 * - Prepared statements with bind (title containing TAB/quotes is safe)
 * - Creates the DB file and table if they don't exist
 *
 * The function signature is kept identical to the previous raw-SQL
 * version so that all existing call sites (music_app.c:412) work
 * without changes.
 *
 * @param list     Music list to persist
 * @param db_path  Database file path (e.g. "/data/music/music_0.db")
 * @return 0 on success, -1 on error
 */
int music_db_save(const MusicList *list, const char *db_path);

/**
 * Load music list from database via MPD bridge.
 * Falls back to text DB if the file is not a valid SQLite database
 * (enables seamless migration from old text format).
 *
 * @param list     Output list (will be cleared first)
 * @param db_path  SQLite file path
 * @return Number of records loaded, -1 on error
 */
int music_db_load(MusicList *list, const char *db_path);

/**
 * Query songs by device name (mount point prefix).
 * Mirrors Android MediaInfoDao.findByDeviceName().
 *
 * @param db_path      SQLite file path
 * @param device_name  Device prefix, e.g. "/mnt/usb0"
 * @param list         Output list (will be cleared first)
 * @return Number of matches, -1 on error
 */
int music_db_query_by_device(const char *db_path, const char *device_name,
                             MusicList *list);

/**
 * Query songs by title (substring match, case-insensitive).
 * Mirrors Android MediaInfoDao.findByTitle().
 */
int music_db_query_by_title(const char *db_path, const char *title,
                            MusicList *list);

/**
 * Query songs by artist (substring match, case-insensitive).
 * Mirrors Android MediaInfoDao.findByArtist().
 */
int music_db_query_by_artist(const char *db_path, const char *artist,
                             MusicList *list);

/**
 * Query songs by album (substring match, case-insensitive).
 * Mirrors Android MediaInfoDao.findByAlbum().
 */
int music_db_query_by_album(const char *db_path, const char *album,
                            MusicList *list);

/**
 * Query songs by filepath (exact match).
 * Mirrors Android MediaInfoDao.findByFilePath().
 */
int music_db_query_by_filepath(const char *db_path, const char *filepath,
                               MusicList *list);

/* --- Direct scan → MPD tree (no MusicList intermediate) --- */

/**
 * Scan directory and feed results directly into MPD database tree.
 * No MusicList intermediate array — each discovered song is immediately
 * passed to mpd_db_add_song(). Memory is managed by MPD's C++ STL internals.
 *
 * @param db           MPD database handle (from mpd_db_open)
 * @param dir_path     Root scan path (e.g. "/mnt/usb0")
 * @return Number of songs added, -1 on error
 */
int music_scan_to_mpd_db(mpd_db_t *db, const char *dir_path);

/**
 * Cancellable version of music_scan_to_mpd_db.
 * Same semantics as music_scan_directory_cancellable but feeds MPD tree directly.
 */
int music_scan_to_mpd_db_cancellable(
        mpd_db_t *db,
        const char *dir_path,
        const volatile int *cancel_flag,
        int expected_gen,
        scan_progress_fn progress_cb,
        void *cb_ctx);

/* --- Page-based list loading (no full MusicList malloc) --- */

#define MUSIC_PAGE_SIZE  20  /* default songs per page */

/**
 * Load one page of songs from MPD database into a MusicList.
 *
 * The MusicList should be pre-allocated with capacity = page_size.
 * Only this page's songs are in memory; previous pages are not retained.
 *
 * @param db         MPD database handle (already opened)
 * @param page       0-based page index
 * @param page_size  songs per page
 * @param list       Output list (count will be set to actual songs on this page)
 * @return songs on this page (0 = past end), -1 on error
 */
int music_db_load_page(mpd_db_t *db, int page, int page_size, MusicList *list);

/**
 * Get total song count in database (for calculating total pages).
 */
int music_db_get_total_count(mpd_db_t *db);

#ifdef __cplusplus
}
#endif

#endif /* MUSIC_SCANNER_H */