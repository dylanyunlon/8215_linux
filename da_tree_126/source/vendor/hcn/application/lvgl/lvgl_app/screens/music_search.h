/**
 * @file music_search.h
 * @brief Track search for the music screen: case-insensitive substring over title / artist / album / filename,
 *        plus pinyin-initial matching (see music_pinyin.h).  Scans the current device's FULL list, independent of
 *        whichever sub-playlist (folder/album/artist) is currently playing.
 */
#ifndef MUSIC_SEARCH_H
#define MUSIC_SEARCH_H

#include <stdbool.h>
#include <stddef.h>
#include "music_scanner.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MUSIC_SEARCH_MAX 200

/** Lowercase [a-z0-9] only, everything else dropped (spaces, punctuation, non-ASCII).  Returns length. */
int music_search_normalize(const char *in, char *out, size_t out_size);

/** `raw` = what the user typed (trimmed by the caller or not), `query` = music_search_normalize(raw). */
bool music_search_track_matches(const MusicInfo *mi, const char *raw, const char *query);

/** Fills out[] with pointers into the device list (valid until the next rescan / unmount).
 *  Returns the number of matches (<= max); *truncated is set when more existed.  Empty input returns 0. */
int music_search_run(const char *raw, const MusicInfo **out, int max, bool *truncated);

#ifdef __cplusplus
}
#endif
#endif /* MUSIC_SEARCH_H */
