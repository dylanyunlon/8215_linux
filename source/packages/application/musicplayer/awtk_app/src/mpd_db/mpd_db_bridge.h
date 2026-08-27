/*
 * mpd_db_bridge.h — Pure C interface to MPD's SimpleDatabase
 */
#ifndef MPD_DB_BRIDGE_H
#define MPD_DB_BRIDGE_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MPD_DB_TYPE_DEFINED
#define MPD_DB_TYPE_DEFINED
typedef struct mpd_db mpd_db_t;
#endif

typedef enum {
    MPD_TAG_ARTIST = 0, MPD_TAG_ARTIST_SORT, MPD_TAG_ALBUM, MPD_TAG_ALBUM_SORT,
    MPD_TAG_ALBUM_ARTIST, MPD_TAG_ALBUM_ARTIST_SORT, MPD_TAG_TITLE, MPD_TAG_TITLE_SORT,
    MPD_TAG_TRACK, MPD_TAG_NAME, MPD_TAG_GENRE, MPD_TAG_MOOD, MPD_TAG_DATE,
    MPD_TAG_ORIGINAL_DATE, MPD_TAG_COMPOSER, MPD_TAG_COMPOSER_SORT,
    MPD_TAG_PERFORMER, MPD_TAG_CONDUCTOR, MPD_TAG_WORK, MPD_TAG_MOVEMENT,
    MPD_TAG_MOVEMENTNUMBER, MPD_TAG_SHOWMOVEMENT, MPD_TAG_ENSEMBLE,
    MPD_TAG_LOCATION, MPD_TAG_GROUPING, MPD_TAG_COMMENT, MPD_TAG_DISC,
    MPD_TAG_DISC_SUBTITLE, MPD_TAG_LABEL, MPD_TAG_COUNT
} mpd_tag_type_t;

typedef struct {
    const char *uri;
    time_t mtime;
    int duration_ms;
    int id3_parsed;    /* 0=not parsed, 1=ID3v2(software), 2=cmus(hardware-assisted) */
    const char *tags[MPD_TAG_COUNT];
} mpd_song_info_t;

typedef int (*mpd_song_visitor_fn)(const mpd_song_info_t *song, void *user_data);

mpd_db_t *mpd_db_open(const char *db_path);
void mpd_db_close(mpd_db_t *db);
int mpd_db_save(mpd_db_t *db);
int mpd_db_add_song(mpd_db_t *db, const mpd_song_info_t *info);
int mpd_db_remove_song(mpd_db_t *db, const char *uri);
int mpd_db_visit_all(mpd_db_t *db, mpd_song_visitor_fn visitor, void *user_data);
int mpd_db_visit_directory(mpd_db_t *db, const char *dir_uri, mpd_song_visitor_fn visitor, void *user_data);

/**
 * Page-based visit — skip first (page * page_size) songs, then visit at most page_size.
 *
 * @param db         MPD database handle
 * @param page       0-based page index
 * @param page_size  songs per page (e.g. 20)
 * @param visitor    callback for each song on this page
 * @param user_data  opaque context
 * @return number of songs visited on this page (0 = past end), -1 on error
 */
int mpd_db_visit_page(mpd_db_t *db, int page, int page_size,
                      mpd_song_visitor_fn visitor, void *user_data);

int mpd_db_song_count(mpd_db_t *db);
time_t mpd_db_get_mtime(mpd_db_t *db);
void mpd_db_clear(mpd_db_t *db);

#ifdef __cplusplus
}
#endif
#endif
