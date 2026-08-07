/*
 * music_player.h - Music playback controller
 *
 * Wraps libatcmediaplayer.so (MediaPlayer class) and adds:
 *   - Playlist management
 *   - Play modes: sequential, repeat-one, repeat-all, shuffle
 *   - Track navigation: next, previous
 *   - State change notifications via callback
 *
 * Reference: Android AutoMediaPlayer media-model/impl/MediaPlayerModel.java
 *
 * Copyright (c) 2026. All rights reserved.
 */

#ifndef MUSIC_PLAYER_H
#define MUSIC_PLAYER_H

#include "music_scanner.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Play mode (aligned with Android IConstant play modes) */
typedef enum {
    PLAY_MODE_SEQUENTIAL = 0,   /* play list in order, stop at end */
    PLAY_MODE_REPEAT_ALL = 1,   /* loop entire playlist */
    PLAY_MODE_REPEAT_ONE = 2,   /* loop current track */
    PLAY_MODE_SHUFFLE    = 3,   /* random order */
} PlayMode;

/* Player state */
typedef enum {
    PLAYER_STATE_IDLE    = 0,
    PLAYER_STATE_PLAYING = 1,
    PLAYER_STATE_PAUSED  = 2,
    PLAYER_STATE_STOPPED = 3,
    PLAYER_STATE_ERROR   = 4,
} PlayerState;

/*
 * Callback function types for UI notification
 * (will be called from player thread context)
 */
typedef void (*on_state_changed_fn)(PlayerState state, void *user_data);
typedef void (*on_track_changed_fn)(int index, const MusicInfo *info, void *user_data);
typedef void (*on_position_changed_fn)(int position_ms, int duration_ms, void *user_data);

/*
 * MusicPlayerContext - opaque player handle
 */
typedef struct MusicPlayerContext MusicPlayerContext;

/* --- Lifecycle --- */

/* Create player context. Returns NULL on failure. */
MusicPlayerContext *music_player_create(void);

/* Destroy player context. Stops playback and frees resources. */
void music_player_destroy(MusicPlayerContext *ctx);

/* --- Playlist --- */

/* Set the playlist from a MusicList (copies data).
 * This replaces any existing playlist. */
int music_player_set_playlist(MusicPlayerContext *ctx, const MusicList *list);

/* Get current playlist count. */
int music_player_get_playlist_count(MusicPlayerContext *ctx);

/* Get info for a track at index. Returns NULL if out of range. */
const MusicInfo *music_player_get_track_info(MusicPlayerContext *ctx, int index);

/* Get current track index. Returns -1 if nothing playing. */
int music_player_get_current_index(MusicPlayerContext *ctx);

/* --- Playback control --- */

/* Play track at index. index=-1 means resume current or start from 0. */
int music_player_play(MusicPlayerContext *ctx, int index);

/* Pause playback. */
int music_player_pause(MusicPlayerContext *ctx);

/* Resume playback. */
int music_player_resume(MusicPlayerContext *ctx);

/* Stop playback. */
int music_player_stop(MusicPlayerContext *ctx);

/* Skip to next track (respects play mode). */
int music_player_next(MusicPlayerContext *ctx);

/* Skip to previous track (respects play mode). */
int music_player_prev(MusicPlayerContext *ctx);

/* Seek to position in milliseconds. */
int music_player_seek(MusicPlayerContext *ctx, int position_ms);

/* --- Play mode --- */

/* Set play mode. */
void music_player_set_mode(MusicPlayerContext *ctx, PlayMode mode);

/* Get current play mode. */
PlayMode music_player_get_mode(MusicPlayerContext *ctx);

/* --- State query --- */

/* Get current player state. */
PlayerState music_player_get_state(MusicPlayerContext *ctx);

/* Get current playback position in milliseconds. */
int music_player_get_position(MusicPlayerContext *ctx);

/* --- Callbacks (for UI) --- */

void music_player_set_state_callback(MusicPlayerContext *ctx,
                                     on_state_changed_fn cb, void *user_data);

void music_player_set_track_callback(MusicPlayerContext *ctx,
                                     on_track_changed_fn cb, void *user_data);

void music_player_set_position_callback(MusicPlayerContext *ctx,
                                        on_position_changed_fn cb, void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* MUSIC_PLAYER_H */
