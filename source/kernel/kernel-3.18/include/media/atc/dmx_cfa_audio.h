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


#ifndef DMX_CFA_AUDIO_H
#define DMX_CFA_AUDIO_H

#include "x_typedef.h"
#include "mm_debug.h"
#include "mm_common.h"

#define AUDIO_UNIT_TXLEN        (100 * 1024) //KB
#define AUDIO_SWDEC_UNIT_TXLEN  (20 * 1024)


#define LP_FLAC_SEEK_POINT_LENGTH						(18)
#define LP_FLAC_SEEK_PINT_SAMPLE_NUM_BYTES		(8)
#define LP_FLAC_SEEK_PINT_FRAME_OFST_BYTES		(8)

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
										macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/
	typedef struct {
#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKStart;
#endif
		__u64 u8Sa;	/* < start file offset, 0-based */
		__u64 u8Ea;	/* < end file offset.    The byte of this offset is transferred. */
		__s32 i4Rate;
		__u32 u4SeekNum;
		__u64 u8SeekTime;
		__u32 u4Ac3FrameNo;
		__u32 u4TxUnitRange;
#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKEnd;
#endif
	} CfaAudioPR;

	typedef struct {
		CfaAudioPR rCfaAudioRange;
		__u32 u4TxUnitKeyFrmRange;
	} CfaAudioKeyFrmRange_T;

	typedef enum {
		CFA_FILE_NONE,
		CFA_FILE_FLAC,
		CFA_FILE_MP3,
		CFA_FILE_AAC,
		CFA_FILE_WAV,
		CFA_FILE_AC3
	} FILE_TYPE_E;

	typedef struct {
		__u64 u4SampleNumber;
		/*< The sampe number of the target frame> */
		__u64 u4StreamOfst;
		/*<The offset, in bytes,of the target frame with respect to beginning of the first frame> */
		__u16 u2FrameSample;
		/*<The number of samples in the target frame> */
	} LP_FLAC_STREAMMETADATA_SEEKPOINT;

	typedef struct
	{
		__u64 u8FrameOft;
		__u64 u8FrameSize;
	}LP_AC3_FRAME_INFO;

	typedef struct
	{
		LP_AC3_FRAME_INFO *prAc3FrameInfo;
		__u32 u4FrameCount;
	}LP_CFA_Ac3FrameInf;
	
	typedef struct {
		__u32 u4AudioByteRate;	/* < byte / s */
		bool fgAc3Type;
		FILE_TYPE_E eFileType;
		AVCODECID_T eAudType;
		char *pcPoints;
		__u32 u4NumPoint;
		__u32 u4SeekTableSz;
		__u64 u8FrameStartOfst;
		__u32 u4Duration;
		__u32 u4SampeRate;
		__u32	u4TxUnitRange;
		LP_CFA_Ac3FrameInf rAc3FrameInfo;
	} CfaAudioCfgInf;


#ifdef __cplusplus
}
#endif
#endif				/* DMX_CFA_AUDIO_H */
