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


#ifndef ASRC_H
#define ASRC_H

#include "audiosys.h"
#include "aud_if_hw_asrc.h"


#define ASRC_IBUF_CH_SZ		(3840U * 2U) /* 2bytes(16 bits), need 48 alignment */
#define ASRC_OBUF_CH_SZ		(7680U * 2U)


/******************************************
*
*	ASRC
*
******************************************/
typedef struct _ASRC_ {
	PASRC_CHS_CLS_PUB m_prAsrcChs;

	WAVE_DATA_BUF_T m_rIBuf;
	u32 m_u4IWP;

	WAVE_DATA_BUF_T m_rOBuf;
	u32 m_u4ORP;

	u32 m_u4InToOutScale;

	u32 m_u4Idx;
} Asrc;


s32 Asrc_Init(u32 u4ChIdx);
s32 Asrc_UnInit(u32 u4ChIdx);

s32 Asrc_InterruptThread(u32 u4IntType);

s32 Asrc_Setup(u32 u4ChIdx, PASRC_CHS_CLS_PUB pvAsrcChs, PASRC_CHS_FMT_T prFmt);

s32 Asrc_Start(u32 u4ChIdx);
s32 Asrc_Stop(u32 u4ChIdx);

s32 Asrc_SetIWP(u32 u4ChIdx, u32 u4WP);
s32 Asrc_SetORP(u32 u4ChIdx, u32 u4RP);
u32 Asrc_GetIRP(u32 u4ChIdx);
u32 Asrc_GetOWP(u32 u4ChIdx);

s32 Asrc_GetIDataSz(u32 u4ChIdx, u32 *pu4Size);
s32 Asrc_GetODataSz(u32 u4ChIdx, u32 *pu4Size);

s32 Asrc_GetIBuf(u32 u4ChIdx, PWAVE_DATA_BUF_T prBuf);
s32 Asrc_GetOBuf(u32 u4ChIdx, PWAVE_DATA_BUF_T prBuf);

PASRC_CHS_CLS_PUB Asrc_GetAsrcChSet(u32 u4Idx);


/******************************************
*
*	ASRC Manager
*
******************************************/

typedef struct _ASRC_MGR_ {
	PASRC_MGR_CLS_PUB m_prAsrcMgr;
	u32 m_u4SpeechPalette;
	u32 m_u4SpeechFs;
} AsrcMgr;


s32 AsrcMgr_Init(void);
s32 AsrcMgr_UnInit(void);

s32 AsrcMgr_SetSpeechFs(u32 u4SpeechFs);

s32 AsrcMgr_AllocASRC(PASRC_CHS_FMT_T prChCfg, bool fgSpeech, u32 *pu4Idx);

s32 Asrc_HibernationCtrl(bool fgWakeUp);

#endif


