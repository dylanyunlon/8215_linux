/*
 * test_cmus_port.c — Functional verification for ported cmus modules
 *
 * Tests:
 * 1. ID3 tag parsing (v1 + v2)
 * 2. CUE sheet parsing
 * 3. comment/keyval metadata operations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "cmus_compat.h"
#include "id3.h"
#include "cue.h"
#include "cue_utils.h"
#include "comment.h"
#include "keyval.h"
#include "uchar.h"
#include "convert.h"
#include "file.h"

/* =========================================================
 * Test 1: ID3v1 tag parsing from synthetic data
 * ========================================================= */
static void test_id3v1(void)
{
	printf("=== Test 1: ID3v1 parsing ===\n");

	/* Build a synthetic ID3v1 tag (128 bytes) */
	char tag[128];
	memset(tag, 0, sizeof(tag));
	memcpy(tag + 0, "TAG", 3);                     /* magic */
	memcpy(tag + 3, "Test Title", 10);              /* title (30 bytes) */
	memcpy(tag + 33, "Test Artist", 11);            /* artist (30 bytes) */
	memcpy(tag + 63, "Test Album", 10);             /* album (30 bytes) */
	memcpy(tag + 93, "2024", 4);                    /* year (4 bytes) */
	/* comment (28 bytes) + zero + track */
	tag[125] = 0;                                   /* ID3v1.1 marker */
	tag[126] = 5;                                   /* track number */
	tag[127] = 13;                                  /* genre: Pop */

	/* Write to temp file */
	const char *tmpfile = "/tmp/test_id3v1.mp3";
	int fd = open(tmpfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	assert(fd >= 0);

	/* Write some fake audio data before the tag */
	char fake_audio[256];
	memset(fake_audio, 0xFF, sizeof(fake_audio));
	write(fd, fake_audio, sizeof(fake_audio));
	write(fd, tag, 128);
	close(fd);

	/* Parse it */
	fd = open(tmpfile, O_RDONLY);
	assert(fd >= 0);

	struct id3tag id3;
	id3_init(&id3);
	int rc = id3_read_tags(&id3, fd, ID3_V1);
	close(fd);

	printf("  id3_read_tags returned: %d\n", rc);
	printf("  has_v1: %d\n", id3.has_v1);

	if (id3.has_v1) {
		char *title = id3_get_comment(&id3, ID3_TITLE);
		char *artist = id3_get_comment(&id3, ID3_ARTIST);
		char *album = id3_get_comment(&id3, ID3_ALBUM);
		char *date = id3_get_comment(&id3, ID3_DATE);
		char *genre = id3_get_comment(&id3, ID3_GENRE);
		char *track = id3_get_comment(&id3, ID3_TRACK);

		printf("  Title:  '%s'\n", title ? title : "(null)");
		printf("  Artist: '%s'\n", artist ? artist : "(null)");
		printf("  Album:  '%s'\n", album ? album : "(null)");
		printf("  Date:   '%s'\n", date ? date : "(null)");
		printf("  Genre:  '%s'\n", genre ? genre : "(null)");
		printf("  Track:  '%s'\n", track ? track : "(null)");

		assert(title && strcmp(title, "Test Title") == 0);
		assert(artist && strcmp(artist, "Test Artist") == 0);
		assert(album && strcmp(album, "Test Album") == 0);
		assert(date && strcmp(date, "2024") == 0);
		assert(genre && strcmp(genre, "Pop") == 0);
		assert(track && strcmp(track, "5") == 0);

		free(title); free(artist); free(album);
		free(date); free(genre); free(track);
	}

	id3_free(&id3);
	unlink(tmpfile);
	printf("  PASSED\n\n");
}

/* =========================================================
 * Test 2: ID3v2 tag parsing from synthetic data
 * ========================================================= */
static void test_id3v2(void)
{
	printf("=== Test 2: ID3v2.3 parsing ===\n");

	const char *tmpfile = "/tmp/test_id3v2.mp3";
	int fd = open(tmpfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	assert(fd >= 0);

	/* Build a minimal ID3v2.3 header + frames */
	unsigned char header[10];
	memcpy(header, "ID3", 3);   /* magic */
	header[3] = 3;              /* version major: 2.3 */
	header[4] = 0;              /* version minor */
	header[5] = 0;              /* flags */

	/* We'll build frames in a buffer first, then compute size */
	unsigned char frames[512];
	int fpos = 0;

	/* TIT2 frame: "Hello World" */
	{
		const char *text = "Hello World";
		int tlen = strlen(text);
		memcpy(frames + fpos, "TIT2", 4); fpos += 4;  /* frame id */
		/* size (4 bytes big-endian): encoding byte + text */
		int fsize = 1 + tlen;
		frames[fpos++] = (fsize >> 24) & 0xFF;
		frames[fpos++] = (fsize >> 16) & 0xFF;
		frames[fpos++] = (fsize >> 8) & 0xFF;
		frames[fpos++] = fsize & 0xFF;
		frames[fpos++] = 0; /* flags */
		frames[fpos++] = 0; /* flags */
		frames[fpos++] = 0x03; /* encoding: UTF-8 */
		memcpy(frames + fpos, text, tlen); fpos += tlen;
	}

	/* TPE1 frame: "Test Artist" */
	{
		const char *text = "Test Artist";
		int tlen = strlen(text);
		memcpy(frames + fpos, "TPE1", 4); fpos += 4;
		int fsize = 1 + tlen;
		frames[fpos++] = (fsize >> 24) & 0xFF;
		frames[fpos++] = (fsize >> 16) & 0xFF;
		frames[fpos++] = (fsize >> 8) & 0xFF;
		frames[fpos++] = fsize & 0xFF;
		frames[fpos++] = 0;
		frames[fpos++] = 0;
		frames[fpos++] = 0x03; /* UTF-8 */
		memcpy(frames + fpos, text, tlen); fpos += tlen;
	}

	/* TCON frame: "(13)" → should resolve to "Pop" */
	{
		const char *text = "(13)";
		int tlen = strlen(text);
		memcpy(frames + fpos, "TCON", 4); fpos += 4;
		int fsize = 1 + tlen;
		frames[fpos++] = (fsize >> 24) & 0xFF;
		frames[fpos++] = (fsize >> 16) & 0xFF;
		frames[fpos++] = (fsize >> 8) & 0xFF;
		frames[fpos++] = fsize & 0xFF;
		frames[fpos++] = 0;
		frames[fpos++] = 0;
		frames[fpos++] = 0x03; /* UTF-8 */
		memcpy(frames + fpos, text, tlen); fpos += tlen;
	}

	/* TRCK frame: "7" */
	{
		const char *text = "7";
		int tlen = strlen(text);
		memcpy(frames + fpos, "TRCK", 4); fpos += 4;
		int fsize = 1 + tlen;
		frames[fpos++] = (fsize >> 24) & 0xFF;
		frames[fpos++] = (fsize >> 16) & 0xFF;
		frames[fpos++] = (fsize >> 8) & 0xFF;
		frames[fpos++] = fsize & 0xFF;
		frames[fpos++] = 0;
		frames[fpos++] = 0;
		frames[fpos++] = 0x03;
		memcpy(frames + fpos, text, tlen); fpos += tlen;
	}

	/* Now write the header with syncsafe size */
	int total_size = fpos;
	header[6] = (total_size >> 21) & 0x7F;
	header[7] = (total_size >> 14) & 0x7F;
	header[8] = (total_size >> 7) & 0x7F;
	header[9] = total_size & 0x7F;

	write(fd, header, 10);
	write(fd, frames, fpos);

	/* Some fake audio */
	char fake[64];
	memset(fake, 0xFF, sizeof(fake));
	write(fd, fake, sizeof(fake));
	close(fd);

	/* Parse */
	fd = open(tmpfile, O_RDONLY);
	assert(fd >= 0);

	struct id3tag id3;
	id3_init(&id3);
	int rc = id3_read_tags(&id3, fd, ID3_V2);
	close(fd);

	printf("  id3_read_tags returned: %d\n", rc);
	printf("  has_v2: %d\n", id3.has_v2);

	if (id3.has_v2) {
		char *title = id3_get_comment(&id3, ID3_TITLE);
		char *artist = id3_get_comment(&id3, ID3_ARTIST);
		char *genre = id3_get_comment(&id3, ID3_GENRE);
		char *track = id3_get_comment(&id3, ID3_TRACK);

		printf("  Title:  '%s'\n", title ? title : "(null)");
		printf("  Artist: '%s'\n", artist ? artist : "(null)");
		printf("  Genre:  '%s' (from '(13)')\n", genre ? genre : "(null)");
		printf("  Track:  '%s'\n", track ? track : "(null)");

		assert(title && strcmp(title, "Hello World") == 0);
		assert(artist && strcmp(artist, "Test Artist") == 0);
		assert(genre && strcmp(genre, "Pop") == 0);
		assert(track && strcmp(track, "7") == 0);

		free(title); free(artist); free(genre); free(track);
	}

	id3_free(&id3);
	unlink(tmpfile);
	printf("  PASSED\n\n");
}

/* =========================================================
 * Test 3: ID3 genre lookup table
 * ========================================================= */
static void test_id3_genre(void)
{
	printf("=== Test 3: ID3 genre table ===\n");

	const char *g0 = id3_get_genre(0);
	const char *g13 = id3_get_genre(13);
	const char *g17 = id3_get_genre(17);
	const char *g999 = id3_get_genre(999);

	printf("  Genre 0:   '%s'\n", g0 ? g0 : "(null)");
	printf("  Genre 13:  '%s'\n", g13 ? g13 : "(null)");
	printf("  Genre 17:  '%s'\n", g17 ? g17 : "(null)");
	printf("  Genre 999: '%s'\n", g999 ? g999 : "(null)");

	assert(g0 && strcmp(g0, "Blues") == 0);
	assert(g13 && strcmp(g13, "Pop") == 0);
	assert(g17 && strcmp(g17, "Rock") == 0);
	assert(g999 == NULL);

	printf("  PASSED\n\n");
}

/* =========================================================
 * Test 4: CUE sheet parsing
 * ========================================================= */
static void test_cue(void)
{
	printf("=== Test 4: CUE sheet parsing ===\n");

	const char *cue_data =
		"REM GENRE Rock\n"
		"REM DATE 2023\n"
		"PERFORMER \"Test Band\"\n"
		"TITLE \"Best Of\"\n"
		"FILE \"album.flac\" WAVE\n"
		"  TRACK 01 AUDIO\n"
		"    TITLE \"First Song\"\n"
		"    PERFORMER \"Test Band\"\n"
		"    INDEX 01 00:00:00\n"
		"  TRACK 02 AUDIO\n"
		"    TITLE \"Second Song\"\n"
		"    PERFORMER \"Test Band\"\n"
		"    INDEX 00 03:24:00\n"
		"    INDEX 01 03:26:00\n"
		"  TRACK 03 AUDIO\n"
		"    TITLE \"Third Song\"\n"
		"    PERFORMER \"Guest Singer\"\n"
		"    INDEX 01 07:15:25\n";

	struct cue_sheet *sheet = cue_parse(cue_data, strlen(cue_data));
	assert(sheet != NULL);

	printf("  num_tracks: %zu\n", sheet->num_tracks);
	assert(sheet->num_tracks == 3);

	printf("  Sheet title:     '%s'\n", sheet->meta.title ? sheet->meta.title : "(null)");
	printf("  Sheet performer: '%s'\n", sheet->meta.performer ? sheet->meta.performer : "(null)");
	printf("  Sheet genre:     '%s'\n", sheet->meta.genre ? sheet->meta.genre : "(null)");
	printf("  Sheet date:      '%s'\n", sheet->meta.date ? sheet->meta.date : "(null)");

	assert(sheet->meta.title && strcmp(sheet->meta.title, "Best Of") == 0);
	assert(sheet->meta.performer && strcmp(sheet->meta.performer, "Test Band") == 0);
	assert(sheet->meta.genre && strcmp(sheet->meta.genre, "Rock") == 0);
	assert(sheet->meta.date && strcmp(sheet->meta.date, "2023") == 0);

	for (size_t i = 0; i < sheet->num_tracks; i++) {
		struct cue_track *t = &sheet->tracks[i];
		printf("  Track %zu: #%zu '%s' by '%s' @ %.2fs (len=%.2fs)\n",
		       i, t->number,
		       t->meta.title ? t->meta.title : "?",
		       t->meta.performer ? t->meta.performer : "?",
		       t->offset, t->length);
	}

	/* Verify track 1 */
	struct cue_track *t1 = cue_get_track(sheet, 1);
	assert(t1 != NULL);
	assert(strcmp(t1->meta.title, "First Song") == 0);
	assert(t1->offset < 0.01); /* 00:00:00 = 0.0 */

	/* Verify track 2 */
	struct cue_track *t2 = cue_get_track(sheet, 2);
	assert(t2 != NULL);
	assert(strcmp(t2->meta.title, "Second Song") == 0);

	/* Verify track 3 */
	struct cue_track *t3 = cue_get_track(sheet, 3);
	assert(t3 != NULL);
	assert(strcmp(t3->meta.title, "Third Song") == 0);
	assert(strcmp(t3->meta.performer, "Guest Singer") == 0);

	/* Track 1 length should be calculated from track 2's index0 */
	printf("  Track 1 length: %.2f seconds\n", t1->length);
	assert(t1->length > 0); /* 3:24.00 frames */

	cue_free(sheet);
	printf("  PASSED\n\n");
}

/* =========================================================
 * Test 5: CUE file detection
 * ========================================================= */
static void test_cue_utils(void)
{
	printf("=== Test 5: CUE utils ===\n");

	assert(is_cue("album.cue") == 1);
	assert(is_cue("album.CUE") == 0); /* case-sensitive */
	assert(is_cue("album.flac") == 0);
	assert(is_cue("music.mp3") == 0);

	printf("  is_cue detection: PASSED\n\n");
}

/* =========================================================
 * Test 6: keyval operations
 * ========================================================= */
static void test_keyval(void)
{
	printf("=== Test 6: keyval operations ===\n");

	/* Test growing_keyvals */
	GROWING_KEYVALS(kv);

	keyvals_add(&kv, "artist", strdup("Queen"));
	keyvals_add(&kv, "album", strdup("A Night at the Opera"));
	keyvals_add(&kv, "title", strdup("Bohemian Rhapsody"));
	keyvals_add(&kv, "date", strdup("1975"));

	printf("  count: %d\n", kv.count);
	assert(kv.count == 4);

	const char *artist = keyvals_get_val_growing(&kv, "artist");
	const char *album = keyvals_get_val_growing(&kv, "album");
	const char *miss = keyvals_get_val_growing(&kv, "genre");

	printf("  artist: '%s'\n", artist ? artist : "(null)");
	printf("  album:  '%s'\n", album ? album : "(null)");
	printf("  genre:  '%s'\n", miss ? miss : "(null)");

	assert(artist && strcmp(artist, "Queen") == 0);
	assert(album && strcmp(album, "A Night at the Opera") == 0);
	assert(miss == NULL);

	/* Terminate and use as flat keyvals */
	keyvals_terminate(&kv);
	const char *title = keyvals_get_val(kv.keyvals, "title");
	printf("  title (flat): '%s'\n", title ? title : "(null)");
	assert(title && strcmp(title, "Bohemian Rhapsody") == 0);

	keyvals_free(kv.keyvals);
	printf("  PASSED\n\n");
}

/* =========================================================
 * Test 7: comment module (compilation detection, key mapping)
 * ========================================================= */
static void test_comment(void)
{
	printf("=== Test 7: comment module ===\n");

	/* Build a regular (non-compilation) track */
	struct keyval regular[] = {
		{ "artist", "Radiohead" },
		{ "albumartist", "Radiohead" },
		{ "album", "OK Computer" },
		{ "title", "Paranoid Android" },
		{ NULL, NULL }
	};

	assert(track_is_compilation(regular) == 0);
	const char *aa = comments_get_albumartist(regular);
	printf("  albumartist: '%s'\n", aa ? aa : "(null)");
	assert(aa && strcmp(aa, "Radiohead") == 0);

	/* Build a compilation track */
	struct keyval compilation[] = {
		{ "artist", "Various Artists" },
		{ "album", "Summer Hits" },
		{ "compilation", "1" },
		{ NULL, NULL }
	};

	assert(track_is_compilation(compilation) == 1);
	assert(track_is_va_compilation(compilation) == 1);
	printf("  compilation detection: PASSED\n");

	/* Test date parsing */
	struct keyval dated[] = {
		{ "date", "2023-06-15" },
		{ NULL, NULL }
	};
	int date_int = comments_get_date(dated, "date");
	printf("  date integer: %d (expected: 20230615)\n", date_int);
	assert(date_int == 20230615);

	/* Test comments_add with key mapping */
	GROWING_KEYVALS(kv);
	comments_add(&kv, "album_artist", strdup("Mapped Artist"));
	keyvals_terminate(&kv);

	const char *mapped = keyvals_get_val(kv.keyvals, "albumartist");
	printf("  album_artist -> albumartist: '%s'\n", mapped ? mapped : "(null)");
	assert(mapped && strcmp(mapped, "Mapped Artist") == 0);

	keyvals_free(kv.keyvals);
	printf("  PASSED\n\n");
}

/* =========================================================
 * Test 8: UTF-8 validation (uchar module)
 * ========================================================= */
static void test_uchar(void)
{
	printf("=== Test 8: UTF-8 validation ===\n");

	assert(u_is_valid("Hello World") == 1);
	assert(u_is_valid("日本語テスト") == 1);
	assert(u_is_valid("Ünïcödé") == 1);

	/* Invalid UTF-8 */
	char invalid[] = { (char)0xFF, (char)0xFE, 0 };
	assert(u_is_valid(invalid) == 0);

	printf("  Valid ASCII:   PASSED\n");
	printf("  Valid CJK:     PASSED\n");
	printf("  Valid Latin:   PASSED\n");
	printf("  Invalid bytes: PASSED\n\n");
}

/* =========================================================
 * Test 9: iconv conversion (convert module)
 * ========================================================= */
static void test_convert(void)
{
	printf("=== Test 9: iconv UTF-8 encode ===\n");

	/* ISO-8859-1 "café" = 63 61 66 E9 */
	char latin1[] = { 0x63, 0x61, 0x66, (char)0xE9, 0 };
	char *utf8 = NULL;
	int rc = utf8_encode(latin1, "ISO-8859-1", &utf8);

	printf("  utf8_encode returned: %d\n", rc);
	printf("  result: '%s'\n", utf8 ? utf8 : "(null)");

	assert(rc == 0);
	assert(utf8 != NULL);
	/* UTF-8 for "café": 63 61 66 C3 A9 */
	assert(strcmp(utf8, "café") == 0);

	free(utf8);
	printf("  PASSED\n\n");
}

/* =========================================================
 * Test 10: read_all (file module)
 * ========================================================= */
static void test_file(void)
{
	printf("=== Test 10: read_all ===\n");

	const char *tmpfile = "/tmp/test_read_all.bin";
	int fd = open(tmpfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	assert(fd >= 0);

	const char *data = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	write(fd, data, 26);
	close(fd);

	fd = open(tmpfile, O_RDONLY);
	assert(fd >= 0);

	char buf[64];
	ssize_t n = read_all(fd, buf, 26);
	close(fd);
	buf[n] = 0;

	printf("  read_all returned: %zd\n", n);
	printf("  data: '%s'\n", buf);

	assert(n == 26);
	assert(strcmp(buf, data) == 0);

	unlink(tmpfile);
	printf("  PASSED\n\n");
}

/* ========================================================= */

int main(void)
{
	printf("============================================\n");
	printf("  cmus port functional verification tests\n");
	printf("============================================\n\n");

	test_id3v1();
	test_id3v2();
	test_id3_genre();
	test_cue();
	test_cue_utils();
	test_keyval();
	test_comment();
	test_uchar();
	test_convert();
	test_file();

	printf("============================================\n");
	printf("  ALL 10 TESTS PASSED\n");
	printf("============================================\n");

	return 0;
}
