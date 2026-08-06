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

#ifndef MM_COMMON_H
#define MM_COMMON_H

//#include "windows.h"

#ifdef __cplusplus
extern "C" {
#endif

// *********************************************************************
// Invalid timestamp
// *********************************************************************
/*! \name Invalid timestamp
* @{
*/
#define INVALID_TIMESTAMP   ((__u64)(-1))
#define INVALID_DURATION    ((__u32)(-1))
/*! @} */

typedef enum
{
	VCODEC_VERSION_NONE = 0,

	//codec RV version definition here 
	VCODEC_VERSION_RV_10 = 1,
	VCODEC_VERSION_RV_13 = 2,
	VCODEC_VERSION_RV_20 = 3,
	VCODEC_VERSION_RV_30 = 4,
	VCODEC_VERSION_RV_40 = 5,

	//codec H265 version definition here
	VCODEC_VERSION_H265_HM62 = 6,
	VCODEC_VERSION_H265_HM91 = 7,
	VCODEC_VERSION_H265_HM10 = 8,
	VCODEC_VERSION_H265_HMV  = 9,

	//codec SCRV version definition here
	VCODEC_VERSION_SCRV_2 = 10,

	//codec VP6 version definition here
	VCODEC_VERSION_VP6_WITH_ALPHA = 11
	//add other codec version from here
} VCODECVERSION_T;

typedef enum{
	PROFILE_NONE = 0,

	//H264 Profile Definition here
	PROFILE_H264_BASELINE = 1,
	PROFILE_H264_MAIN = 2,
	PROFILE_H264_EXTENDED = 3,
	PROFILE_H264_HIGH = 4,
	PROFILE_H264_HIGH10 = 5,
	PROFILE_H264_HIGH422 = 6,
	PROFILE_H264_HIGH444 = 7,

	PROFILE_RESERVED = 8
}VID_PROFILE_E;

typedef enum{
	LEVEL_NONE = 0,

	//H264 Level Definition here
	LEVEL_H264_L1  = 1, // L1
	LEVEL_H264_L1b = 2, // L1b
	LEVEL_H264_L11 = 3, // L1.1
	LEVEL_H264_L12 = 4, // L1.2
	LEVEL_H264_L13 = 5, // L1.3
	LEVEL_H264_L2  = 6, // L2
	LEVEL_H264_L21 = 7, // L2.1
	LEVEL_H264_L22 = 8, // L2.2
	LEVEL_H264_L3  = 9, // L3
	LEVEL_H264_L31 = 10,// L3.1
	LEVEL_H264_L32 = 11,// L3.2
	LEVEL_H264_L4  = 12,// L4
	LEVEL_H264_L41 = 13,// L4.1
	LEVEL_H264_L42 = 14,// L4.2
	LEVEL_H264_L5  = 15,// L5
	LEVEL_H264_L51 = 16,// L5.1

	LEVEL_RESERVED = 17
}VID_LEVEL_E;

typedef enum
{
	AVCODEC_ID_NONE = 0,
	AVCODEC_ID_UNKNOWN = 1,

	AVCODEC_ID_MPEG1 = 100,
	AVCODEC_ID_MPEG2,
	AVCODEC_ID_MPEG4,
	AVCODEC_ID_MJPEG,
	AVCODEC_ID_H263,
	AVCODEC_ID_SORENSON, /*SH263*/
	AVCODEC_ID_H264,
	AVCODEC_ID_H265,
	AVCODEC_ID_DIVX3,
	AVCODEC_ID_DIVX4,
	AVCODEC_ID_DIVX5,
	AVCODEC_ID_DIVX6,
	AVCODEC_ID_WMV1,
	AVCODEC_ID_WMV2,
	AVCODEC_ID_WMV3,
	AVCODEC_ID_VC1,
	AVCODEC_ID_VP6,
	AVCODEC_ID_VP8,
	AVCODEC_ID_VP9,
	AVCODEC_ID_RV,
	AVCODEC_ID_SCRV,

	AVCODEC_ID_AC3 = 200,
	AVCODEC_ID_EAC3,
	AVCODEC_ID_MPEG,
	AVCODEC_ID_MP3,
	AVCODEC_ID_DTS,
	AVCODEC_ID_DTSCD,
	AVCODEC_ID_DTSHD_NO_XLL,
	AVCODEC_ID_DTSHD_XLL,
	AVCODEC_ID_WMA,
	AVCODEC_ID_AAC,
	AVCODEC_ID_AAC_PURE,
	AVCODEC_ID_VORBIS,
	AVCODEC_ID_PCM,
	AVCODEC_ID_APE,
	AVCODEC_ID_FLAC,
	AVCODEC_ID_RA_COOK,
	AVCODEC_ID_RA_LPCJ,
	AVCODEC_ID_RA_28_8,
	AVCODEC_ID_RA_DNET,
	AVCODEC_ID_RA_SIPR,
	AVCODEC_ID_RA_RALF,
	AVCODEC_ID_RA_ATRC,
	AVCODEC_ID_HDMI_PCM,
	AVCODEC_ID_AMR,
	AVCODEC_ID_AWB,
	AVCODEC_ID_NELLYMOSER,
	AVCODEC_ID_SPEEX,

	AVCODEC_ID_TEXT_SRT = 300,
	AVCODEC_ID_TEXT,
	AVCODEC_ID_SUB,
	AVCODEC_ID_XSUB,
	AVCODEC_ID_XSUB_PLUS,
	AVCODEC_ID_CC,
} AVCODECID_T;

typedef enum {
	MM_PLAY_RATE_RW_32X = (__s32) - 32,
	MM_PLAY_RATE_RW_16X = (__s32) - 16,
	MM_PLAY_RATE_RW_8X = (__s32) - 8,
	MM_PLAY_RATE_RW_4X = (__s32) - 4,
	MM_PLAY_RATE_RW_2X = (__s32) - 2,
	MM_PLAY_RATE_NORMAL = (__s32) 1,
	MM_PLAY_RATE_FF_2X = (__s32) 2,
	MM_PLAY_RATE_FF_4X = (__s32) 4,
	MM_PLAY_RATE_FF_8X = (__s32) 8,
	MM_PLAY_RATE_FF_16X = (__s32) 16,
	MM_PLAY_RATE_FF_32X = (__s32) 32,
} E_MM_PLAYR_RATE_T;

/* Fast Forward or Rewind Playback */
#define MM_IS_FFRW_PLAY(i4Rate)   (((i4Rate) >= (__s32)MM_PLAY_RATE_FF_2X) || \
	((i4Rate) <= (__s32)MM_PLAY_RATE_RW_2X))

/* Fast Forward Playback */
#define MM_IS_FF_PLAY(i4Rate)		((i4Rate) >= (__s32)MM_PLAY_RATE_FF_2X)

/* Fast Rewind Playback */
#define MM_IS_RW_PLAY(i4Rate)		((i4Rate) <= (__s32)MM_PLAY_RATE_RW_2X)

/* Normal Playback */
#define MM_IS_NORMAL_PLAY(i4Rate)	((__s32)MM_PLAY_RATE_NORMAL == (i4Rate))

#define MM_DIVXHT31_USE_NEW_FFRW_MECHANISM	0

#define AHW_LIMIT		_T("[ACODEC][Chip limitation]")
#define ACONT_LIMIT 	_T("[ACODEC][Container limitation]")
#define VHW_LIMIT		_T("[VCODEC][Chip limitation]")
#define VCONT_LIMIT 	_T("[VCODEC][Container limitation]")

#ifdef MM_UNSUSED
#undef MM_UNSUSED
#endif
#define MM_UNSUSED(x) (void)x  //for build warning: unused parameter

#ifdef __cplusplus
}
#endif	/*
 */

#endif	/* MM_COMMON_H */
