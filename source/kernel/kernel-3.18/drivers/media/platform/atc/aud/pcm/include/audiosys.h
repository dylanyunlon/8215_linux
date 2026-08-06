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


#ifndef AUDIOSYS_H
#define AUDIOSYS_H

#include "windows.h"

#include "aud_debug.h"

#ifdef __cplusplus
extern "C" {
#endif


#define GPSO_BUFFER_SIZE			(4800U * 2U * 2U) /* 100ms 48KHz, 16 bist stereo data */

#define PCM_BUFFER_SIZE				(8000U * 2U * 2U) /* 2000ms 8KHz 16 bit mono data */

#define MIC_BUFFER_SIZE				(8000U * 2U * 2U) /* 1000ms 8KHz, 16 bit stereo data */

#define BT_STREAM_BUFFER_SIZE		(64U * 1024U)

#define EVT_UPDATE_PLAYED_SAMPLE	0U
#define EVT_GPS_OUT_INTR			1U
#define EVT_PCM_OUT_INTR			2U
#define EVT_DL_FRAME_FINISH			3U
#define EVT_UL_FRAME_FINISH			4U
#define EVT_STRM_DATA_FINISH		5U
#define EVT_SRC_DATA_FINISH			6U
#define EVT_SPH_SYNC				7U

#define STATE_UNINIT				0U
#define STATE_INITED				1U
#define STATE_STOPPED				2U
#define STATE_STARTED				3U


#define VOL_DETECT_MUTE				0U
#define VOL_DETECT_UNMUTE			1U

#define VOL_DETECT_FRONT			0U
#define VOL_DETECT_REAR				1U
#define VOL_DETECT_WAVEFORM			2U


typedef struct {
	u32 u4Buf1;			/* Virtual start address of buffer */
	u32 u4Buf2;

	u32 u4ChBufSz;		/* Channel Buffer size (in byte)  */
	u32 u4Chn;			/* channel number */

	u32 u4DataOff;		/* Data start offset (in byte) for WP */
	u32 u4DataSz;		/* Data size of every channels (in byte) */
} WAVE_DATA_BUF_T, *PWAVE_DATA_BUF_T;



#define NOERR			0
#define INVALIDPRAM		-1
#define INVALIDSTATE	-2
#define NORESOURCE		-3

#ifdef __cplusplus
}
#endif

#endif
