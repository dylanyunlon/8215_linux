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



#ifndef DMX_CFA_APE_H
#define DMX_CFA_APE_H

#include "x_typedef.h"
#include "mm_debug.h"

#define CFA_APE_UNIT_RANGE_SIZE		(50 * 1024)

/* Old C header file */
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
		__s32 i4Rate;	/* FF/RW */
		__u32 u4UnitTxDataSz;
		bool fgSetSeekInfo;
		__u32 au4SeekInfo[2];
		__u32 u4TxUnitRange;
#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKEnd;
#endif
	} CfaApePR;

	typedef struct {
		__u64 u8Pos;
		__u64 u8Pts;
		__u32 u4Blocks;
		__u32 u4Size;
		__u32 u4Skip;
	} CFA_APE_FRAME_INFO_T;


	typedef struct {
#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKStart;
#endif
		__u32 u4TotalFrames;	/* the total number frames (frames are used internally) */
		__u32 u4BlocksPerFrame;	/* the samples in a frame (frames are used internally) */
		__u32 u4FinalFrameBlocks;	/* the number of samples in the final frame */
		__u32 u4Channels;	/* audio channels */
		__u32 u4SampleRate;	/* audio samples per second */
		__u32 u4BitsPerSample;	/* audio bits per sample */
		__u32 u4BytesPerSample;	/* audio bytes per sample */
		__u32 u4BlockAlign;	/* audio block align (channels * bytes per sample) */
		__u32 u4TotalBlocks;	/* the total number audio blocks */
		__u32 u4AverageBitrate;	/* the kbps (i.e. 637 kpbs) */
		__u32 u4SeekTableElements;	/* the number of elements in the seek table(s) */
		__u8 *pSeekByteTable;

		CFA_APE_FRAME_INFO_T *pFrames;
		bool fgSeekable;

		__u32 u4AudioByteRate;	/* < byte / s */
		__u32 u4Duration;
		bool fgAc3Type;
#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKEnd;
#endif
	} CfaApeCfgInf;

/* Old C header file */
#ifdef __cplusplus
}
#endif
#endif				/* DMX_CFA_AUDIO_H */
