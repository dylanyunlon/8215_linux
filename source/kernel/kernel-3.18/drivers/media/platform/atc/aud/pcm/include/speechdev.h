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


#ifndef SPEECHDEV_H
#define SPEECHDEV_H

#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/semaphore.h>
#include <sound/pcm.h>

#include "bt_cfg.h"
#include "btpcmhw.h"
#include "speechenhance.h"
#include "bt_speech.h"

#include "pcm_ac83xx.h"
#include "audiosys.h"
#include "dtmf.h"
#include "btpcmhw.h"


#define DL_ENHANCE_SAMPLE		145U
#define DL_16K_ENHANCE_SAMPLE	120U
#define SPEECH_PRE_PB_SIZE		(160U * 2U * 6U)
#define DEFAULT_DEV_VOL			255U
#define DEFAULT_BT_VOL			255U
#define DEFAULT_MIC_VOL			37U

typedef struct Speech_Device_Context {
	BtPCMHw m_rPCMHw;
	SpeechEnhance m_rEnhance;
	WAVE_DATA_BUF_T m_rMicBuf;
	u32 m_u4MICRP;
	u32 m_u4MICIdx;
	PVirtualMicIn m_prVMic;

	bool m_fgLoopBack;

	AUD_DATA_BUF_T m_rDLBuf;
	u32 m_u4DLRP;

	s16 *m_ai2DLRingBuffer;
	u32 m_u4DLRef;
	u32 m_u4DLNDC;
	u32 m_u4DLBufSize;

	bool m_fgULMute;

	u32 m_u4State;
	u32 m_u4DLDelay;
	u32 m_u4DL16KDelay;

	u32 m_u4SCOVolume;
	u32 m_u4DevVolume;
	s32  m_i4SCOMaxGain;
	u32 m_u4SCOGainRange;

        bool m_fgTrueSphEn;
        u32 m_u4NDCForCapture;
	u32 m_DropDLCnt;

	struct snd_pcm_substream *m_rSpeechStrm;

	struct mutex m_SpeechLock;
	struct semaphore m_refLock;
    struct semaphore m_micLock;
        struct semaphore m_DLend;
} SpeechDeviceContext;


extern u8 *g_pBTDmaArea;

void SpeechDev_Enable(bool fgEnable, bool fgIsSph, u32 u4SampleRate);
s32 SpeechDev_EnableSCO(bool fgEnable, u32 u4SampleRate);
u32 SpeechDev_GetSCOFS(void);
bool SpeechDev_IsSCOEnable(void);
u32 SpeechDev_EventInform(u32 u4Event, u32 u4Param);
s32 SpeechDev_Init(void);
u32 SpeechDev_UnInit(void);
void   SpeechDev_HibernationCtrl(bool fgWakeUp);
s32 SpeechDev_SetDLDelay(u32 u4Samples);
u32 SpeechDev_GetDLDelay(void);
s32 SpeechDev_SetDL16KDelay(u32 u4Samples);
u32 SpeechDev_GetDL16KDelay(void);
u32 SpeechDev_EnableULMute(bool fgEnable);

s32 SpeechDev_GetSCOVolume(u32 *pu4LVolume, u32 *pu4RVolume);
s32 SpeechDev_SetSCOVolume(u32 u4LVolume, u32 u4RVolume);
s32 SpeechDev_GetDevVolume(u32 *pu4LVolume, u32 *pu4RVolume);
s32 SpeechDev_SetDevVolume(u32 u4LVolume, u32 u4RVolume);
s32  SpeechDev_GetSCOMaxGain(void);
u32 SpeechDev_SetSCOMaxGain(s32 i4MaxGain);
u32 SpeechDev_GetSCOGainRange(void);
u32 SpeechDev_SetSCOGainRange(u32 u4GainRange);

s32 SpeechDev_GetEnhanceParam(PCM_SPEECH_CONF *prSpeechConf);
s32 SpeechDev_SetEnhanceParam(const PCM_SPEECH_CONF *prSpeechConf);
s32 SpeechDev_GetEnhance16KParam(PCM_SPEECH_16K_CONF *prSpeechConf);
s32 SpeechDev_SetEnhance16KParam(PCM_SPEECH_16K_CONF *prSpeechConf);

s32 SpeechDev_SetDmnrParam(const PCM_SPEECH_CONF *prSpeechConf);
u32 SpeechDev_IsDmnrEnable(void);
u32 SpeechDev_EnableDmnr(bool fgEnable);
s32 SpeechDev_SetComRxParam(const PCM_SPEECH_CONF *prSpeechConf);
s32 SpeechDev_SetComTxParam(const PCM_SPEECH_CONF *prSpeechConf);

s32 SpeechDev_SetDmnr16kParam(const PCM_SPEECH_16K_CONF *prSpeechConf);
s32 SpeechDev_SetFilter16kParam(const signed short *pData, u32 u4DataLen);

u32 SpeechDev_SetARM2SpeechLog(u32 u4logLevel);

u32 SpeechDev_EnablePLC(bool fgEnable);

u32 SpeechDev_EnableDump(bool fgEnable);

bool SpeechDev_EnableLoopback(bool fgEnable);
void SpeechDev_Synchronization(void);
void SpeechDev_SynchronizationEx(void);
void SpeechDev_SynchronizationEx_user(void);

s32 SpeechDev_EnableBT(bool fgEnable, u32 u4SampleRate);
void SpeechDev_SetSubstream(struct snd_pcm_substream* Apb_substream);
void SpeechDev_SetCaptureSubStream(struct snd_pcm_substream* substream);

int SpeechDev_ULByPass(u32 dataLen);


#endif


