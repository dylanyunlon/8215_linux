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


#ifndef STRMPROC_H
#define STRMPROC_H

#include <windows.h>
#include "oemsettings.h"
#include "asrc.h"
#include <sound/pcm.h>

#define STREAM_PROCESS_NUM			4U
#define PCM_MODE_VOIP      3


#if (MONO_GAIN)
#define MAX_GAIN					0xFFFF
#else
#define MAX_GAIN					0xFFFFFFFF
#endif

#define BUF_STATE_NO_FILL			0U
#define BUF_STATE_HAS_DATA			1U
#define BUF_STATE_EMPTY				2U

#define STREAM_PROC_MSG_STOP		0U
#define STREAM_PROC_MSG_START		1U


enum {
	MUTE_NO_MUTE_CH =			0x0U,
	MUTE_RIGHT_CH =				0x1U,
	MUTE_LEFT_CH =				0x2U,
	MUTE_LEFT_AND_RIGHT_CH =	0x3U,
};

typedef struct _STRMPROC_ {
	PCMFMT_T m_rInFmt;
	PCMFMT_T m_rOutFmt;
	u32 m_u4Remain;
	u32 m_u4SampleShift;
	u32 m_u4InSampleShift;
	u32 m_u4MiniBytes;
	u32 m_u4BytesMask;
	Asrc *m_prAsrc;
	struct snd_pcm_substream *m_prStream;
	u32 m_u4State;
	u32 m_u4OState;
	AUD_DATA_BUF_T	m_rOutBuf;
	u32 m_u4OutLen;
	u32 m_u4MiniOutLen;
	u32 m_u4OutWP;
	u32 m_u4ProcOutTime;
	u32 m_u4ProcInTime;
	u32 m_u4MaxInData;
	u32 m_u4MaxOutData;
	u32 m_u4InputTotal;
	u32 m_u4OutputTotal;
	u32 m_u4Idx;

	// For debug
	u32 m_u4UnderrunCount;

	struct mutex m_ProcLock;
	struct _STRMPROC_ *m_prNext;

	u32 m_u4BufState;
} StreamProcess;

typedef struct _MIXER_DATA_T {
	u32 u4Buffer;
	u32 u4Size;
	u32 u4Used;
	u32 u4BitsPerSample;
	u32 u4Chn;
} MIXER_DATA_T, *PMIXER_DATA_T;

typedef struct _STRMPROC_MSG_T {
	u32 u4MsgID;
	u32 u4StrmProcID;
} StrmProc_MSG_T;

s32 BtOutput_SetGain(s32 dwGain);
s32 DeviceSPH_SetGain(s32 dwGain);

void StreamProcess_Init(StreamProcess *strmProc);
s32 AttachStream(struct snd_pcm_substream *substream, StreamProcess *strmProc);
s32 DetachStream(struct snd_pcm_substream *substream);
s32 PrepareStream(struct snd_pcm_substream *substream);
bool StrmProcIsCanDelete(const StreamProcess *strmProc);
u32 StateChangeInform(const struct snd_pcm_substream *substream);
s32 EventInform(u32 u4Event, u32 u4Param, StreamProcess *strmProc);
s32 DataAvailableInform(struct snd_pcm_substream *substream);
s32 StrmProcStop(u32 u4StrmProcIdx);
s32 StrmProcDSPMixCh(u32 u4DspMixCh);
u32 SetRefSubStream(struct snd_pcm_substream *substream);
u32 SetCapSubStream(struct snd_pcm_substream *substream);
void ContinueCopyDataToRefWhenDlStop(void);


#endif


