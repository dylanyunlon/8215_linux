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

#ifndef _RX_VIDEO_TIMING_H_
#define _RX_VIDEO_TIMING_H_

#include "vga_table.h"

#define HDTV_SEARCH_START   1
#define HDTV_SEARCH_END     (51-1)

#define HDMI_SEARCH_START       (51)
#define HDMI_SEARCH_END     (60-1)

#define DVI_SEARCH_START        (51)
#define DVI_SEARCH_END      (51+60-1)

#define MAX_TIMING_FORMAT   (60)

enum {
	MODE_NOSIGNAL = 0,        /* No signal */
	MODE_525I_OVERSAMPLE = 1,
	MODE_625I_OVERSAMPLE,
	MODE_480P_OVERSAMPLE,
	MODE_576P_OVERSAMPLE,
	MODE_720p_50,
	MODE_720p_60,
	MODE_1080i_48,
	MODE_1080i_50,
	MODE_1080i,
	MODE_1080p_24,
	MODE_1080p_25,
	MODE_1080p_30,
	MODE_1080p_50,
	MODE_1080p_60,
	MODE_525I,
	MODE_625I,
	MODE_480P,
	MODE_576P,
	MODE_720p_24,
	MODE_720p_25,
	MODE_720p_30,
	MODE_240P,
	MODE_540P,
	MODE_288P,
	MODE_480P_24,
	MODE_480P_30,
	MODE_576P_25,
	MODE_3D_720p_50,
	MODE_3D_720p_60,
	MODE_3D_1080p_24,
	MODE_3D_1080I_60_FRAMEPACKING,
	MODE_3D_1080I_50_FRAMEPACKING,
	MODE_3D_1080P60HZ,
	MODE_3D_1080P50HZ,
	MODE_3D_1080P30HZ,
	MODE_3D_1080P25HZ,
	MODE_3D_720P30HZ,
	MODE_3D_720P25HZ,
	MODE_3D_720P24HZ,
	MODE_3D_576P50HZ,
	MODE_3D_576I50HZ,
	MODE_3D_480P60HZ,
	MODE_3D_480I60HZ,
	MODE_REVERSE1,
	MODE_REVERSE2,
	MODE_HDMI_640_480P = 46,
	MODE_2160P_30HZ,
	MODE_2160P_25HZ,
	MODE_2160P_24HZ,
	MODE_2161P_24HZ,

	MODE_MAX,
	MODE_DE_MODE = 251,
	MODE_NODISPLAY = 252,
	MODE_NOSUPPORT = 253,      /* Signal out of range */
	MODE_WAIT = 254
};


enum {
	B2R_CLK_MODE_27,
	B2R_CLK_MODE_74,
	B2R_CLK_MODE_148,
	B2R_CLK_MODE_MAX
};

#define fgIsUserModeTiming(bMode) (((bMode) >= 1) && ((bMode) < 100))
#define fgIsVgaTiming(bMode)   \
	((((bMode) >= DVI_SEARCH_START) && ((bMode) <= DVI_SEARCH_END)) \
	|| ((bMode) == MODE_DE_MODE) || fgIsUserModeTiming(bMode))
#define fgIsVideoTiming(bMode) (((bMode) >= HDTV_SEARCH_START) && ((bMode) <= HDTV_SEARCH_END))
#define fgIsValidTiming(bMode) (fgIsVgaTiming(bMode) || fgIsVideoTiming(bMode) || fgIsUserModeTiming(bMode))

#if 0
/* extern UINT8 _bHdtvTiming; */

#define fgIsOversampleTiming() ((((_bHdtvTiming) >= MODE_525I_OVERSAMPLE) && ((_bHdtvTiming) <= MODE_625I_OVERSAMPLE)) \
					|| (((_bHdtvTiming) >= MODE_480P_OVERSAMPLE) && \
					((_bHdtvTiming) <= MODE_576P_OVERSAMPLE)))

#define fgIsPScanTiming() ((_bHdtvTiming == MODE_576P) || (_bHdtvTiming == MODE_576P_OVERSAMPLE) || \
					(_bHdtvTiming == MODE_480P) || (_bHdtvTiming == MODE_480P_OVERSAMPLE) || \
					(_bHdtvTiming == MODE_240P) || (_bHdtvTiming == MODE_540P))
#define fgIsVBISupportTiming() ((((_bHdtvTiming) >= MODE_525I_OVERSAMPLE) && ((_bHdtvTiming) <= MODE_576P_OVERSAMPLE)) \
					|| (((_bHdtvTiming) >= MODE_525I) && ((_bHdtvTiming) <= MODE_576P)) \
					|| (((_bHdtvTiming) >= MODE_480P_24) && ((_bHdtvTiming) <= MODE_576P_25)) \
					)
#define fgIsVBISupportCCTiming() ((_bHdtvTiming == MODE_525I_OVERSAMPLE) || (_bHdtvTiming == MODE_525I))
#endif

#define PROGRESSIVE 0           /* Progressive Mode */
#define INTERLACE 1             /* Interlace Mode */

#endif
