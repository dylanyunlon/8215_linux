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



#ifndef MICIN_H
#define MICIN_H

#include "bt_cfg.h"
#include "aud_micin_hal_if.h"

#include "audiosys.h"


#define MICIN_USE_DSP_MEMORY			1U

#define MICIN_SAFE_READ_SIZE			(32U)

#define MICIN_MAX_NUM					(20U)

#define MIC2CAP_BT_START_MIC		0U
#define MIC2CAP_BT_STOP_MIC			1U
#define MIC2CAP_FS_CHANGED          2U

#define MIC_INVALID_INDEX   0xFFFFFFFFU

typedef enum {
	MICIN_START_BY_NONE = 0U,
	MICIN_START_BY_BTSCO,
	MICIN_START_BY_CAPTURE
} MICIN_START_TYPE;


typedef struct UL_DATA_FILE {
	WAVE_DATA_BUF_T m_rBuf;
	u32 m_u4WP;

	struct file *m_pfUL;
	struct file *m_pfUL2;
	u32 m_u4DataPos;
	u32 m_u4DataPos2;
} ULDataFile;

s32 ULDataFile_Init(void);
u32 ULDataFile_UnInit(void);
u32 ULDataFile_GetBuf(WAVE_DATA_BUF_T *prBuf);
s32 ULDataFile_SetRP(u32 u4RP);
u32 ULDataFile_GetWP(void);

typedef u32(*PFN_MicInCallback)(u32 u4MsgID, u32 u4Param1, u32 u4Param2, void *pvData);

typedef struct __Mic_In__ {
	u32 m_u4State;
	u32 m_u4StartTimes;
	u32 m_u4PrimaryMicIndex;

	WAVE_DATA_BUF_T m_rBuf;

    void * aOwner[MICIN_MAX_NUM];

	struct mutex m_MicLock;

	MIC_EXTPARAMS_T m_rMicCfg;
	MIC_HAL_CLS_PUB *m_prMicHal;

	bool m_fgDataFromFile;
} MicIn;

typedef struct __EXT_MIC_CONF__ {
	u32 u4ExtMicEn;
	u32 u4ExtMicI2sPin;
	u32 u4ExtMicFs;
	u32 u4ExtMicSrcBitNum;
} EXT_MIC_CONF;


extern MicIn g_rMic;

s32 MicIn_Init(void);
u32 MicIn_UnInit(void);

void MicIn_HibernationCtrl(bool fgWakeUp);

void MicIn_SetPrimaryMic(u32 u4Index);
u32 MicIn_GetPrimaryMic(void);
void MicIn_SetExtMicCfg(EXT_MIC_CONF rExtMicConf);
s32 MicIn_SetGain(u32 u4MicGain);
u32 MicIn_GetGain(void);
u32 MicIn_SetFS(u32 u4SampleRate);
u32 MicIn_GetFS(void);

bool MicIn_IsStart(void);
s32 MicIn_Start(void *owner);
s32 MicIn_Stop(void *owner);

void MicIn_InitBuffer(void);
u32 MicIn_GetBuffer(WAVE_DATA_BUF_T *prBuffer);
u32 MicIn_GetWP(void);

bool MicIn_IsDataFromFile(void);
void MicIn_SetDataFromFile(bool fgDataFromFile);
bool MicIn_EnableDataFromFile(bool fgEnable);


#endif



