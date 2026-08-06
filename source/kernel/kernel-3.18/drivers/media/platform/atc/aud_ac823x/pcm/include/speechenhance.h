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


#ifndef SPEECHDENHANCE_H
#define SPEECHDENHANCE_H

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>

#include "bt_cfg.h"
#include "bt_speech.h"
#include "oemsettings.h"
#include "aud_pcm_dbg.h"
#include "pcm_audconf.h"

enum {
	DBG_FILE_DL_PRE = 0U,
	DBG_FILE_DL_NDC_POST,
	DBG_FILE_DL_POST,
	DBG_FILE_UL_PRE,
	DBG_FILE_UL_REF,
	DBG_FILE_UL_AEC_POST,
	DBG_FILE_UL_POST,
	DBG_FILE_PCM_APP_PTR,
	DBG_FILE_NUM
};

typedef struct {
	bool fgDMNR;
	bool fgPLC;
	u32 u4Log;
} SPH_OPT_T, *PSPH_OPT_T;

#define SPH_PARA_IDX_DUAL_MIC	1U


#define MAX_DBG_FILE_BUFFER (320U * 150U)

struct save_wavbuf {
	char *pwave_buf;
	char szFileName[150];
	u32 data_Len;
	u32 writed_len;
	u32 pull_len;
	u32 samplerate;
	s32					Thread_wq_flag;
	struct task_struct	*Thread_task;
	wait_queue_head_t	Thread_wq;
	u32 BtDbg_File_Idx;
	bool fgStopThread;
};

#pragma pack(push, 1)
typedef struct {
	u8	 riff[4];		   /* "RIFF" */
	u32	 filesize;		   /* File size - 8 */
	u8	 wave[4];		   /* "WAVE" */
	u8	 fmt[4];		   /* "fmt " */
	u32	 fmtsize;		   /* 0x10 */
	u16	 wFormatTag;	   /* 0x01 -> PCM */
	u16	 nChannels;		   /* Channels */
	u32	 nSamplesPerSec;   /* Sampling Rate (samples per second) */
	u32	 nAvgBytesPerSec;  /* Average Bytes per second */
	u16	 nBlockAlign;
	u16	 wBitsPerSample;   /* Bits per sample */
	u8	 data[4];		   /* "data" */
	u32	 datasize;		   /* Data Size(Byte) */
} WaveHeader;
#pragma pack(pop)

typedef struct Speech_Enhance {
	u32 m_u4State;
	BT_SHARE_MEM_EX_T *m_prShareMem;
	uintptr_t m_u4PhyAddr;
	u32 m_u4HwSemaphore;
	void* m_hCompleted;
	u32 m_u4AECEvent;

	volatile u32 m_u4AECState;
	u32 m_u4LastReadIdx;

	u32 m_u4ActiveIdx;
	SPEECH_FRAME_T *m_prActiveFrame;
	SPH_OPT_T m_rSphOpt;

	bool m_fgDbgEnable;
	u32 m_u4DebugIdx;
	struct save_wavbuf m_rDbgWave[DBG_FILE_NUM];
} SpeechEnhance;

extern SpeechEnhance g_prSpeechEnhance;

bool SpeechEnhance_PostProcess(u32 u4Index);
s32 SpeechEnhance_EnableSCO(bool fgEnable);
bool SpeechEnhance_IsSCOEnable(void);
s32 SpeechEnhance_Init(void);
s32 SpeechEnhance_UnInit(void);
void SpeechEnhance_HibernationCtrl(bool fgWakeUp);
SPEECH_FRAME_T *SpeechEnhance_GetFreeFrame(void);
s32 SpeechEnhance_EnhanceDL(SPEECH_FRAME_T *prFrame);
s32 SpeechEnhance_EnhanceUL(SPEECH_FRAME_T *prFrame);
u32 SpeechEnhance_SpeechHandleAECMsg(u32 u4MsgID, u32 u4Param1, u32 u4Param2, u32 u4Param3);

s32 SpeechEnhance_GetEnhanceParam(PCM_SPEECH_CONF *prSpeechConf);
s32 SpeechEnhance_SetEnhanceParam(const PCM_SPEECH_CONF *prSpeechConf);
s32 SpeechEnhance_GetEnhance16KParam(PCM_SPEECH_16K_CONF *prSpeechConf);
s32 SpeechEnhance_SetEnhance16KParam(PCM_SPEECH_16K_CONF *prSpeechConf);

s32 SpeechEnhance_SetDmnrParam(const PCM_SPEECH_CONF *prSpeechConf);
bool SpeechEnhance_IsDmnrEnable(void);
u32 SpeechEnhance_EnableDmnr(bool fgEnable);
s32 SpeechEnhance_SetComRxParam(const PCM_SPEECH_CONF *prSpeechConf);
s32 SpeechEnhance_SetComTxParam(const PCM_SPEECH_CONF *prSpeechConf);

s32 SpeechEnhance_SetDmnr16kParam(const PCM_SPEECH_16K_CONF *prSpeechConf);
s32 SpeechEnhance_SetFilter16kParam(const signed short *pData, u32 u4DataLen);

u32 SpeechEnhance_SetARM2SpeechLog(u32 u4logLevel);

u32 SpeechEnhance_EnablePLC(bool fgEnable);
u32 SpeechEnhance_EnableDump(bool fgEnable);


#endif

