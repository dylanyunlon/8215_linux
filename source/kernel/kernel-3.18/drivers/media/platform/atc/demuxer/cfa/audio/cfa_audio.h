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
  * @author  Qing Li 
  * @version 1.0
  * @brief     The C file of the interface for Audio CFA
  */

#ifndef CFA_AUDIO_H
#define CFA_AUDIO_H

#include <media/atc/dmx_cfa_audio.h>

#include "dmx_spt_cfa.h"

/* CFA Audio invalid address */
#define CFA_AUDIO_INVALID_ADDRESS (-1)

typedef enum {
	CFA_AUDIO_LOG_DEFAULT = (u32)1 << (u32)0,
	CFA_AUDIO_LOG_COMMON  = (u32)1 << (u32)1,
	CFA_AUDIO_LOG_FFRW    = (u32)1 << (u32)2
} CfaAudioLogLvl_E;

typedef enum {
	CFA_AC3_ANA_TX = 0x00,
	CFA_AC3_ANA_SYNC = 0x01
} ECfaAc3AnaSt;


typedef enum {
	CFA_AUDIO_ANA_TX,
	CFA_AUDIO_ANA_JUMP
} ECfaAudioAnaSt;

typedef struct
{
    __u64 u8FrameOft;
    __u64 u8FrameSize;
}LP_CFA_AC3_FRAME_INFO_T;

typedef struct
{
    LP_CFA_AC3_FRAME_INFO_T *prAc3FrameInfo;
    __u32 u4FrameCount;
}CfaAc3FrameInf;


typedef struct {
#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
#endif
	CfaAudioPR rRange;
	bool fgFinished;
	bool fgAc3Type;
	u8 *pucPbBuf;
	u32 u4PfrMemAddress;	/* memory address got from Pfr, using sync DMA */
	bool fgTxData2Buf;
	ECfaAc3AnaSt eCfaAc3AnaSt;
	u32 u4TxUnitRange;
	u64 u8CurrTxOft;
	s32 i4Rate;
	u32 u4SeekNum;
	u64 u8PrevTxOft;
	u32 u4FlacUnitLen;
	ECfaAudioAnaSt eCfaAudioAnaSt;
	FILE_TYPE_E eFileType;
	CfaApiAudType eAudType;
	bool fgJumpTurnOn;
	char *pcPoints;
	u32 u4NumPoint;
	u64 u8FrameStartOfst;
	u32 u4CurSeekNum;
	u32 u4Duration;
	u64 u8SeekTime;
	u32 u4SampeRate;
	u64 u8StepTime;
	CfaAc3FrameInf rAc3FrameInfo;
    __u32 u4Ac3CurFrameNo;
#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
#endif
} CfaAudioInst;


#endif				/* _CFA_AUDIO_H_ */
