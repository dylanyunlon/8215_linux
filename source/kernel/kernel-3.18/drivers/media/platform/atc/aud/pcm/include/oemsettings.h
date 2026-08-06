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


#ifndef OEMSETTINGS_H
#define OEMSETTINGS_H

#define OUTCHANNELS				(2U)
#define INCHANNELS				(2U)
#define MAXCHANNELS				(2U)

#define BITSPERSAMPLE			(16U)


/* The code will use the follwing values as saturation points */
#define AUDIO_SAMPLE_MAX		(32767U)
#define AUDIO_SAMPLE_MIN		(-32768)

/* The following define the maximum attenuations for the SW volume controls in devctxt.cpp
     e.g. 100 => range is from 0dB to -100dB */
#define SPH_GAIN_RANGE			32U
#define SPH_GAIN_MAX			0U
#define STREAM_GAIN_RANGE		32
#define STREAM_GAIN_MAX			0
#define DEVICE_GAIN_RANGE		32
#define DEVICE_GAIN_MAX			0
#define MUTE_VOLUME				0x100

/* If set to 1, all gain will be mono (left volume applied to both channels) */
#define MONO_GAIN				0U


#define DEF_DATA_BITS			16U

typedef struct _PCMFMT_T {
	u32 u4FS;
	u32 u4BW;
	u32 u4Chn;
} PCMFMT_T, *PPCMFMT_T;


#define USE_BUILTIN_PATH				1U

#define ARM1_SPEECH_INTERACT_WITH_ARM2	1U

#define ENABLE_DTMF_FUNCTION			1U
#define DTMF_INFO_SENDER				1U

#define PB_DEBUG_SUPPORT				0U

#define NO_DEVICE_VOLUME				0U
#define OS_SW_MIXER						0U

#endif

