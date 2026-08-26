/**
 * @file cmus_bridge.h
 * @brief Bridge between cmus ported modules and musicplayer's music_scanner.
 *
 * Provides two integration points:
 *   1. cmus_bridge_parse_tags() — replaces music_scanner.c's extract_text_frame()
 *      with cmus id3.c's full ID3v1/v2 parser (UTF-16, genre table, etc.)
 *   2. cmus_bridge_scan_cue()  — CUE sheet parsing for split-track playback
 *
 * File location: source/packages/application/musicplayer/awtk_app/src/cmus_bridge.h
 *
 * Copyright (C) AutoChips Inc. All rights reserved.
 * Uses cmus code licensed under GPL-2.0+
 */

#ifndef CMUS_BRIDGE_H
#define CMUS_BRIDGE_H

#include "music_scanner.h"  /* MusicInfo, MusicList */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Parse ID3 tags using cmus's full parser.
 *
 * Replaces music_scanner.c's simplistic extract_text_frame() which:
 *   - Cannot decode UTF-16 (replaces non-ASCII with '?')
 *   - Only supports ID3v2.3/v2.4, not ID3v1/v2.2
 *   - Has no genre table (genre field is always empty)
 *
 * cmus id3.c supports:
 *   - ID3v1 + ID3v2.2/v2.3/v2.4 (full version coverage)
 *   - UTF-16 LE/BE → UTF-8 via iconv (中日韩歌名正确显示)
 *   - 148-genre mapping table
 *   - TXXX custom frames (ReplayGain, MusicBrainz)
 *   - COMM comment frames
 *   - albumartist (TPE2), composer (TCOM), conductor (TPE3)
 *
 * @param filepath  Audio file path (MP3)
 * @param info      MusicInfo to populate (title, artist, album, etc.)
 * @return 0 on success, -1 if no tags found
 */
int cmus_bridge_parse_tags(const char *filepath, MusicInfo *info);

/**
 * @brief Scan a .cue file and add individual tracks to the music list.
 *
 * CUE sheets describe split-track layouts for whole-CD rips:
 *   FILE "album.flac" WAVE
 *     TRACK 01 AUDIO
 *       TITLE "Track One"
 *       PERFORMER "Artist"
 *       INDEX 01 00:00:00
 *     TRACK 02 AUDIO
 *       TITLE "Track Two"
 *       INDEX 01 04:32:15
 *
 * Each track becomes a separate MusicInfo entry with cue_offset/cue_length
 * metadata (for the player to seek to the right position).
 *
 * @param cue_path  Path to the .cue file
 * @param dir_path  Directory containing the .cue file
 * @param list      Music list to append tracks to
 * @return Number of tracks added, or -1 on error
 */
int cmus_bridge_scan_cue(const char *cue_path, const char *dir_path,
                         MusicList *list);

/**
 * @brief Check if a filename is a .cue file.
 * @return true if extension is ".cue" (case insensitive)
 */
int cmus_bridge_is_cue(const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* CMUS_BRIDGE_H */