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

#ifndef BTPCMHW_H
#define BTPCMHW_H

#include "bt_cfg.h"
#include "aud_pcm_hal_if.h"

#define PCMRX_SILENCE_FRAME     2 // 2 * 20ms
#define PCMTX_SILENCE_SIZE		24000U
#define PCMTX_FADEIN_SIZE		8000U
#define PCMTX_FADEIN_STEP		(0x10000U / PCMTX_FADEIN_SIZE)


typedef struct DL_DATA_FILE {
	AUD_DATA_BUF_T m_rBuf;
	u32 m_u4WP;
	u32 m_u4DataPos;
	struct file *m_pfDL;
} DLDataFile;

s32 DLDataFile_Init(void);
u32 DLDataFile_UnInit(void);
s32 DLDataFile_Read(void);
u32 DLDataFile_GetBuf(AUD_DATA_BUF_T *prBuf);
u32 DLDataFile_SetRP(u32 u4RP);
u32 DLDataFile_GetWP(void);


typedef struct BT_PCM_HW {
	u32 m_u4State;

	u32 m_u4DLRP;
	u32 m_u4ULWP;

	u32 m_u4SampleRate;
	u32 m_u4IntNum;

	u32 m_u4TxFillSize;
	s32  m_i4TxFadeInVol;

	AUD_DATA_BUF_T m_rTxBuf;
	AUD_DATA_BUF_T m_rRxBuf;

	PCM_EXTPARAMS_T m_rPcmCfg;
	PPCM_HAL_CLS_PUB m_prPcmHal;

	bool m_fgDataFromFile;
	bool resetMicP;

	s32					PCMDev_Thread_wq_flag;
	struct task_struct	*PCMDev_Thread_task;
	wait_queue_head_t	PCMDev_Thread_wq;
} BtPCMHw;

extern BtPCMHw g_rBtPcm;


void BtPCMHw_InterruptThread(void);

s32 BtPCMHw_Init(void);
u32 BtPCMHw_UnInit(void);

void BtPCMHw_HibernationCtrl(bool fgWakeUp);

bool BtPCMHw_IsStart(void);

s32 BtPCMHw_Start(u32 u4SampleRate);
s32 BtPCMHw_Stop(void);

u32 BtPCMHw_GetDLBuffer(AUD_DATA_BUF_T *prBuffer);
u32 BtPCMHw_GetDLWP(void);
void BtPCMHw_UpdateDLRP(u32 u4RP);
u32 BtPCMHw_GetDLRP(void);

u32 BtPCMHw_GetULFreeLen(void);
u32 BtPCMHw_FillULData(void *pvData, u32 u4DataSize);

void BtPCMHw_ULDataFadeIn(s16 *pi2Data, u32 u4DataSize);

bool BtPCMHw_EnableDataFromFile(bool fgEnable);
bool BtPCMHw_IsDataFromFile(void);
void BtPCMHw_SetDataFromFile(bool fgDataFromFile);

bool BtPCMHw_IsLoopback(void);
bool BtPCMHw_EnableLoopback(bool fgEnable);


#endif
