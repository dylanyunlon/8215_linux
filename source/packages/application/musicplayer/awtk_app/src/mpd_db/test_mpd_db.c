/*
 * test_mpd_db.c — Pure C test for MPD database bridge
 * Compile: gcc -c test_mpd_db.c && g++ -o test_mpd_db test_mpd_db.o -L. -lmpd_db -lfmt -lpthread
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>

#include "mpd_db_bridge.h"

static int print_visitor(const mpd_song_info_t *song, void *user_data)
{
    int *count = (int *)user_data;
    (*count)++;
    printf("  [%d] uri='%s'", *count, song->uri);
    if (song->tags[MPD_TAG_TITLE])
        printf(" title='%s'", song->tags[MPD_TAG_TITLE]);
    if (song->tags[MPD_TAG_ARTIST])
        printf(" artist='%s'", song->tags[MPD_TAG_ARTIST]);
    if (song->tags[MPD_TAG_ALBUM])
        printf(" album='%s'", song->tags[MPD_TAG_ALBUM]);
    if (song->tags[MPD_TAG_GENRE])
        printf(" genre='%s'", song->tags[MPD_TAG_GENRE]);
    if (song->duration_ms >= 0)
        printf(" dur=%dms", song->duration_ms);
    printf("\n");
    return 0; /* continue */
}

int main(void)
{
    const char *db_path = "/tmp/test_mpd.db";

    printf("============================================\n");
    printf("  MPD Database Bridge Test\n");
    printf("============================================\n\n");

    /* Test 1: Open/Create */
    printf("=== Test 1: Open database ===\n");
    mpd_db_t *db = mpd_db_open(db_path);
    assert(db != NULL);
    printf("  Opened: %s\n", db_path);
    printf("  Initial song count: %d\n", mpd_db_song_count(db));
    printf("  PASSED\n\n");

    /* Test 2: Add songs */
    printf("=== Test 2: Add songs ===\n");
    {
        mpd_song_info_t info;
        memset(&info, 0, sizeof(info));

        /* Song 1 */
        info.uri = "USB1/Rock/track01.mp3";
        info.mtime = time(NULL);
        info.duration_ms = 245000;
        info.tags[MPD_TAG_TITLE] = "Bohemian Rhapsody";
        info.tags[MPD_TAG_ARTIST] = "Queen";
        info.tags[MPD_TAG_ALBUM] = "A Night at the Opera";
        info.tags[MPD_TAG_GENRE] = "Rock";
        info.tags[MPD_TAG_TRACK] = "1";
        info.tags[MPD_TAG_DATE] = "1975";
        assert(mpd_db_add_song(db, &info) == 0);

        /* Song 2 */
        memset(&info, 0, sizeof(info));
        info.uri = "USB1/Rock/track02.mp3";
        info.mtime = time(NULL);
        info.duration_ms = 180000;
        info.tags[MPD_TAG_TITLE] = "Stairway to Heaven";
        info.tags[MPD_TAG_ARTIST] = "Led Zeppelin";
        info.tags[MPD_TAG_ALBUM] = "Led Zeppelin IV";
        info.tags[MPD_TAG_GENRE] = "Rock";
        assert(mpd_db_add_song(db, &info) == 0);

        /* Song 3 (different directory) */
        memset(&info, 0, sizeof(info));
        info.uri = "USB1/Pop/track01.mp3";
        info.mtime = time(NULL);
        info.duration_ms = 210000;
        info.tags[MPD_TAG_TITLE] = "Billie Jean";
        info.tags[MPD_TAG_ARTIST] = "Michael Jackson";
        info.tags[MPD_TAG_ALBUM] = "Thriller";
        info.tags[MPD_TAG_GENRE] = "Pop";
        assert(mpd_db_add_song(db, &info) == 0);

        /* Song 4 (root directory) */
        memset(&info, 0, sizeof(info));
        info.uri = "loose_track.flac";
        info.mtime = time(NULL);
        info.duration_ms = 300000;
        info.tags[MPD_TAG_TITLE] = "Loose Track";
        info.tags[MPD_TAG_ARTIST] = "Unknown";
        assert(mpd_db_add_song(db, &info) == 0);
    }
    printf("  Added 4 songs\n");
    printf("  Song count: %d\n", mpd_db_song_count(db));
    assert(mpd_db_song_count(db) == 4);
    printf("  PASSED\n\n");

    /* Test 3: Visit all */
    printf("=== Test 3: Visit all songs ===\n");
    {
        int count = 0;
        int visited = mpd_db_visit_all(db, print_visitor, &count);
        printf("  Visited: %d\n", visited);
        assert(visited == 4);
    }
    printf("  PASSED\n\n");

    /* Test 4: Visit by directory */
    printf("=== Test 4: Visit USB1/Rock/ only ===\n");
    {
        int count = 0;
        int visited = mpd_db_visit_directory(db, "USB1/Rock", print_visitor, &count);
        printf("  Visited: %d (expected 2)\n", visited);
        assert(visited == 2);
    }
    printf("  PASSED\n\n");

    /* Test 5: Save to disk */
    printf("=== Test 5: Save database ===\n");
    assert(mpd_db_save(db) == 0);
    printf("  Saved to: %s\n", db_path);
    printf("  PASSED\n\n");

    /* Test 6: Close and reopen */
    printf("=== Test 6: Close and reopen ===\n");
    mpd_db_close(db);
    db = mpd_db_open(db_path);
    assert(db != NULL);
    printf("  Reopened, song count: %d\n", mpd_db_song_count(db));
    assert(mpd_db_song_count(db) == 4);
    printf("  PASSED\n\n");

    /* Test 7: Visit after reload */
    printf("=== Test 7: Visit all after reload ===\n");
    {
        int count = 0;
        int visited = mpd_db_visit_all(db, print_visitor, &count);
        assert(visited == 4);
    }
    printf("  PASSED\n\n");

    /* Test 8: Incremental update (same mtime = skip) */
    printf("=== Test 8: Incremental update ===\n");
    {
        mpd_song_info_t info;
        memset(&info, 0, sizeof(info));
        info.uri = "USB1/Rock/track01.mp3";
        /* Use an mtime that matches what's already stored */
        info.mtime = time(NULL); /* Different mtime, so it will update */
        info.duration_ms = 999000;
        info.tags[MPD_TAG_TITLE] = "Updated Title";
        assert(mpd_db_add_song(db, &info) == 0);
        printf("  Updated track01 (different mtime)\n");
        printf("  Song count still: %d\n", mpd_db_song_count(db));
        assert(mpd_db_song_count(db) == 4);
    }
    printf("  PASSED\n\n");

    /* Test 9: Remove song */
    printf("=== Test 9: Remove song ===\n");
    assert(mpd_db_remove_song(db, "loose_track.flac") == 0);
    printf("  Removed loose_track.flac\n");
    printf("  Song count: %d\n", mpd_db_song_count(db));
    assert(mpd_db_song_count(db) == 3);
    assert(mpd_db_remove_song(db, "nonexistent.mp3") == -1);
    printf("  Nonexistent remove correctly returned -1\n");
    printf("  PASSED\n\n");

    /* Test 10: Bulk add (simulate large library) */
    printf("=== Test 10: Bulk add 1000 songs ===\n");
    {
        char uri[256], title[64];
        mpd_song_info_t info;
        for (int i = 0; i < 1000; i++) {
            memset(&info, 0, sizeof(info));
            snprintf(uri, sizeof(uri), "USB1/Bulk/album%03d/track%02d.mp3",
                     i / 10, i % 10);
            snprintf(title, sizeof(title), "Bulk Song %d", i);
            info.uri = uri;
            info.mtime = time(NULL);
            info.duration_ms = 180000 + (i * 100);
            info.tags[MPD_TAG_TITLE] = title;
            info.tags[MPD_TAG_ARTIST] = "Bulk Artist";
            info.tags[MPD_TAG_ALBUM] = "Bulk Album";
            assert(mpd_db_add_song(db, &info) == 0);
        }
        printf("  Added 1000 songs\n");
        printf("  Total song count: %d\n", mpd_db_song_count(db));
        assert(mpd_db_song_count(db) == 1003); /* 3 remaining + 1000 bulk */
    }

    /* Save and verify */
    assert(mpd_db_save(db) == 0);
    mpd_db_close(db);
    db = mpd_db_open(db_path);
    assert(db != NULL);
    printf("  After save/reload: %d songs\n", mpd_db_song_count(db));
    assert(mpd_db_song_count(db) == 1003);
    printf("  PASSED\n\n");

    /* Cleanup */
    mpd_db_close(db);
    unlink(db_path);

    printf("============================================\n");
    printf("  ALL 10 TESTS PASSED\n");
    printf("============================================\n");

    return 0;
}
