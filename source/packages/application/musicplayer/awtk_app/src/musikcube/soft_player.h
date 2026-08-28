/*
 * soft_player.h — Software decoding player engine (replaces MediaPlayer硬解).
 *
 * Architecture (musikcube "拿来主义"):
 *
 *   filepath → LocalFileStream (IDataStream)
 *            → FfmpegDecoder   (IDecoder)    — avformat/avcodec 软解码
 *            → Buffer          (IBuffer)     — float PCM 缓冲
 *            → AlsaOut         (IOutput)     — snd_pcm_writei 输出声卡
 *
 * All components from musikcube (BSD-3-Clause), adapted for embedded Linux.
 * This file exposes a simple C interface consumed by music_player.cpp.
 *
 * Copyright (c) 2026. Portions (c) 2004-2023 musikcube team (BSD-3-Clause).
 */

#ifndef SOFT_PLAYER_H
#define SOFT_PLAYER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SoftPlayerContext SoftPlayerContext;

typedef enum {
    SOFT_STATE_IDLE    = 0,
    SOFT_STATE_PLAYING = 1,
    SOFT_STATE_PAUSED  = 2,
    SOFT_STATE_STOPPED = 3,
    SOFT_STATE_ERROR   = 4,
} SoftPlayerState;

typedef void (*soft_state_cb)(SoftPlayerState state, void* user_data);

/* Lifecycle */
SoftPlayerContext* soft_player_create(void);
void               soft_player_destroy(SoftPlayerContext* ctx);

/* Playback */
int    soft_player_play(SoftPlayerContext* ctx, const char* filepath);
int    soft_player_stop(SoftPlayerContext* ctx);
int    soft_player_pause(SoftPlayerContext* ctx);
int    soft_player_resume(SoftPlayerContext* ctx);
int    soft_player_seek(SoftPlayerContext* ctx, double seconds);

/* State query */
SoftPlayerState soft_player_get_state(SoftPlayerContext* ctx);
double          soft_player_get_position(SoftPlayerContext* ctx);
double          soft_player_get_duration(SoftPlayerContext* ctx);
void            soft_player_set_volume(SoftPlayerContext* ctx, double vol);

/* Callback */
void soft_player_set_state_callback(SoftPlayerContext* ctx, soft_state_cb cb, void* user_data);

#ifdef __cplusplus
}
#endif

#endif /* SOFT_PLAYER_H */
