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

/*****************************************************************************
 * @par Description
 *    CFA audio Header File
 *
 * @par Author_Name
 *    Qing Li 
*****************************************************************************/

 /*!
  * @file       cfa_audio.h
  * @author  
  * @version 1.0
  * @brief     The C file of the interface for Audio CFA
  */

#ifndef CFA_APE_H
#define CFA_APE_H

#include <media/atc/dmx_cfa_ape.h>

#include "dmx_spt_cfa.h"

/* CFA Audio invalid address */
#define CFA_AUDIO_INVALID_ADDRESS (-1)

typedef enum {
	CFA_APE_LOG_DEFAULT = (u32)1 << (u32)0,
	CFA_APE_LOG_COMMON  = (u32)1 << (u32)1,
	CFA_APE_LOG_FFRW    = (u32)1 << (u32)2
} CfaApeLogLvl_E;

typedef enum {
	CFA_APE_ANA_TX = 0x00,
	CFA_APE_ANA_HEADER = 0x01,
	CFA_APE_ANA_RW = 0x02,
	CFA_APE_ANA_IDLE = 0x03,
} ECfaApeAnaSt;


typedef struct {
#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
#endif
	u32 u4TotalFrames;	/*the total number frames (frames are used internally)*/
	u32 u4BlocksPerFrame;	/*the samples in a frame (frames are used internally)*/
	u32 u4FinalFrameBlocks;	/* the number of samples in the final frame*/
	u32 u4Channels;	/* audio channels*/
	u32 u4SampleRate;	/* audio samples per second*/
	u32 u4BitsPerSample;	/* audio bits per sample*/
	u32 u4BytesPerSample;	/* audio bytes per sample*/
	u32 u4BlockAlign;	/* audio block align (channels * bytes per sample)*/
	u32 u4TotalBlocks;	/* the total number audio blocks*/
	u32 u4AverageBitrate;	/* the kbps (i.e. 637 kpbs)*/
	u32 u4SeekTableElements;	/* the number of elements in the seek table(s)*/

	CFA_APE_FRAME_INFO_T *pFrames;
	bool fgSeekable;

	u32 u4AudioByteRate;	/*< byte / s*/
	u32 u4Duration;
	bool fgAc3Type;
#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
#endif
} CfaApeInf;


typedef struct {
#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
#endif
	CfaApePR rRange;
	bool fgAc3Type;
	UCHAR *pucPbBuf;
	bool fgRWTurnOn;
	ECfaApeAnaSt eCfaApeAnaSt;
	u32 u4DataLenth;
	u32 u4StartFrmNo;
	u32 u4CurFrmNo;
	u32 u4SkipLenth;
	u32 u4TxUnitRange;
	u64 u8CurrdataLenth;
	u64 u8CurrTxOft;
	CfaApeInf cfaApeinfo;
	s32 i4Rate;
	UCHAR *pucHeader;
	bool fgSetSeekInfo;
	u32 au4SeekInfo[2];
#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
#endif
} CfaApeInst;




#endif /* _CFA_AUDIO_H_*/
