/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/
#ifndef _X_AUD_EXT_H_
#define _X_AUD_EXT_H_

#include "x_common.h"
#include "x_aud_dec.h"


// these definitions are for HTS model with amplifier controller or external DSP


typedef enum
{
    AUD_DEC_CH_UNKNOWN = 0,
    AUD_DEC_CH_L_R,               // L/R (2/0/0)
    AUD_DEC_CH_L_R_LFE,           // L/R/LFE (2/0/0 + lfe)
    AUD_DEC_CH_L_R_C,             // L/R/C (3/0/0)
    AUD_DEC_CH_L_R_C_LFE,         // L/R/C/LFE (3/0/0 + lfe)
    AUD_DEC_CH_L_R_S,             // L/R/S (2/1/0)
    AUD_DEC_CH_L_R_LFE_S,         // L/R/S/LFE (2/1/0 + lfe)
    AUD_DEC_CH_L_R_C_S,           // L/R/C/S (3/1/0)
    AUD_DEC_CH_L_R_C_LFE_S,       // L/R/C/S/LFE (3/1/0 + lfe)
    AUD_DEC_CH_L_R_LS_RS,         // L/R/Ls/Rs (2/2/0)
    AUD_DEC_CH_L_R_LS_RS_LFE,     // L/R/Ls/Rs/LFE (2/2/0 + lfe)
    AUD_DEC_CH_L_R_LS_RS_C,       // L/R/C/Ls/Rs (3/2/0)
    AUD_DEC_CH_L_R_C_LFE_LS_RS,   // L/R/C/Ls/Rs/LFE (3/2/0 + lfe)
    AUD_DEC_CH_L_R_LS_RS_SB,      // L/R/Ls/Rs/Sb (2/2/1)
    AUD_DEC_CH_L_R_LFE_LS_RS_SB,          // L/R/Ls/Rs/Sb/LFE (2/2/1 + lfe)
    AUD_DEC_CH_L_R_C_LS_RS_SB,            // L/R/C/Ls/Rs/Sb (3/2/1)
    AUD_DEC_CH_L_R_C_LFE_LS_RS_SB,        // L/R/C/Ls/Rs/Sb/LFE (3/2/1 + lfe)
    AUD_DEC_CH_L_R_LS_RS_LSB_RSB,         // L/R/Ls/Rs/Lsb/Rsb (2/2/2)
    AUD_DEC_CH_L_R_LFE_LS_RS_LSB_RSB,     // L/R/Ls/Rs/Lsb/Rsb/LFE (2/2/2 + lfe)
    AUD_DEC_CH_L_R_C_LS_RS_LSB_RSB,       // L/R/C/Ls/Rs/Lsb/Rsb (3/2/2)
    AUD_DEC_CH_L_R_C_LFE_LS_RS_LSB_RSB,   // L/R/C/Ls/Rs/Lsb/Rsb/LFE (3/2/2 + lfe)
    AUD_DEC_CH_C,                 // C (1/0/0)
    AUD_DEC_CH_C_LFE,             // C/LFE (1/0/0 + lfe)
    AUD_DEC_CH_DUAL_MONO,         // L+R (Dual Mono)
} AUD_DEC_CHANNEL_T;

typedef enum
{
    AUD_DEC_MIX_LEVEL_UNKNOWN = 0,
    AUD_DEC_MIX_LEVEL_707,  // 0.707 (-3.0dB)
    AUD_DEC_MIX_LEVEL_595,  // 0.595 (-4.5dB)
    AUD_DEC_MIX_LEVEL_500,  // 0.500 (-6.0dB)
    AUD_DEC_MIX_LEVEL_0
} AUD_DEC_MIX_LEVEL_T;

typedef enum
{
    AUD_DEC_STATE_UNKNOWN = 0,

    AUD_DEC_STATE_BEFORE_DECODING,

    AUD_DEC_STATE_STARTING,  // include "header changing"
    AUD_DEC_STATE_STOPPING
} AUD_DEC_STATE_T;

typedef enum
{
    AUD_DEC_ENCMODE_NOT_INDICATED = 0,
    AUD_DEC_ENCMODE_NOT_ENCODED,
    AUD_DEC_ENCMODE_ENCODED,
    AUD_DEC_ENCMODE_RESERVED
} AUD_DEC_ENCMODE_T;

typedef enum
{
    AUD_DEC_POST_DECODE_OFF,
    AUD_DEC_POST_DECODE_PROLOGIC,
    AUD_DEC_POST_DECODE_PLII_MOVIE,
    AUD_DEC_POST_DECODE_NEO6_MOVIE,
    AUD_DEC_POST_DECODE_NEO6_MUSIC
} AUD_DEC_POST_DECODE_INFO_T;

// Audio information needed by amplifier or external DSP
typedef struct _AUDIO_INFORMATION
{
    AUD_DEC_STATE_T     eState;
    bool                fgFullInfo;
    AUD_DEC_FMT_T       eStreamType;
    _u32              u4SampleRate;          // sampling rate (after downsampling)
    _u32              u4OrgSampleRate;       // original sampling rate
    AUD_DEC_CHANNEL_T   eInputChannel;
    AUD_DEC_CHANNEL_T   eOutputChannel;
    AUD_DEC_ENCMODE_T   eDolbySurroundMode;
    AUD_DEC_ENCMODE_T   eDolbySurroundExMode;
    AUD_DEC_MIX_LEVEL_T eCenterMixLevel;
    AUD_DEC_MIX_LEVEL_T eSurroundMixLevel;
    _u32              u4TotalAttMch;
    _u32              u4TotalAtt2ch;
} AUDIO_INFORMATION_T;

// Q: Shall we get the audio channel output status from the output channel assignment
//    from audio information notification?
typedef struct _AUDIO_OUTPUT_STATUS
{
    bool  fgFront;
    bool  fgCenter;
    bool  fgSubwoofer;
    bool  fgSurround;
    bool  fgSurroundBack;
} AUDIO_OUTPUT_STATUS_T;

#endif /* _X_AUD_EXT_H_ */

