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


#include "speechdev.h"
#include "aud_pcm_dbg.h"
#include "strmproc.h"
#include "audiosys.h"
#include "btoutput.h"
#include "speechdev.h"
#include "pcm_hardware.h"

#include "pcm_debug.h"
#define LOG_TAG "btoutput"

#define MAX_BOUNDARY ((u32)0x40000000L)

extern SpeechDeviceContext g_prSpeechDev;

struct snd_pcm_substream *OpenBtSpeechStream(void)
{
	struct snd_pcm_substream *btsubstream;
	struct snd_pcm_runtime *btruntime;
	substream_data *btsubstrm_data;

	btsubstream = kzalloc(sizeof(*btsubstream), GFP_KERNEL);
	if (btsubstream == NULL) {
		return NULL;
	}

	btsubstream->number = 3;
	btsubstream->stream = SNDRV_PCM_STREAM_PLAYBACK;
	btsubstream->private_data = snd_chip;

	btruntime = kzalloc(sizeof(*btruntime), GFP_KERNEL);
	if (btruntime == NULL) {
		return NULL;
	}
	btsubstream->runtime = btruntime;

	memset(g_pBTDmaArea, 0, BT_STREAM_BUFFER_SIZE);
	btsubstrm_data = kzalloc(sizeof(*btsubstrm_data), GFP_KERNEL);
	if (btsubstrm_data == NULL) {
		PCM_ERROR(LOG_TAG, "OpenBtSpeechStream: Allocate btsubstrm_data error!\r\n");
		return NULL;
	}
	memset(btsubstrm_data, 0, sizeof(*btsubstrm_data));
	btsubstrm_data->substream = btsubstream;
	btruntime->private_data = btsubstrm_data;
	btruntime->channels = 1;
	btruntime->rate = g_prSpeechEnhance.m_prShareMem->u4SampleRate;
	btruntime->format = SNDRV_PCM_FORMAT_S16_LE;
	btruntime->frame_bits = 16;

	btsubstrm_data->period_size = BT_STREAM_BUFFER_SIZE / 4U;
	btsubstrm_data->buffer_size = BT_STREAM_BUFFER_SIZE;
	btsubstrm_data->dma_size = BT_STREAM_BUFFER_SIZE;
	btsubstrm_data->dma_start = (u32)g_pBTDmaArea;
	btsubstrm_data->dma_shift = 1;
	btsubstrm_data->last_ptr = 0;
	btsubstrm_data->Used_size = 0;
	btsubstrm_data->appl_ptr = 0;
	btsubstrm_data->app_Base = 0;
	btsubstrm_data->hw_Base = 0;
	btsubstrm_data->IsBtSpeech = 1;
	btsubstrm_data->boundary = MAX_BOUNDARY;

	if (NOERR != GetStreamProcess(btsubstream)) {
		PCM_ERROR(LOG_TAG, "OpenBtSpeechStream: GetStreamProcess err.\r\n");
		return NULL;
	}

	if (NOERR != PrepareStream(btsubstream)) {
		PCM_ERROR(LOG_TAG, "OpenBtSpeechStream: PrepareStream err.\r\n");
		return NULL;
	}

	btsubstrm_data->m_eState = SNDRV_PCM_TRIGGER_START;
	StateChangeInform(btsubstream);
	DataAvailableInform(btsubstream);

	PCM_DEBUG(LOG_TAG, "OpenBtSpeechStream: success!\r\n");

	return btsubstream;
}

s32 CloseBtSpeechStream(struct snd_pcm_substream *substream)
{
	u32 u4Count = 0;
	struct snd_pcm_substream *btsubstream = substream;
	struct snd_pcm_runtime *btruntime = NULL;
	substream_data *btsubstrm_data = NULL;
	StreamProcess *pStrmProc = NULL;

	if (NULL == btsubstream) {
		PCM_ERROR(LOG_TAG, "CloseBtSpeechStream: btsubstream is NULL!\r\n");
		return INVALIDPRAM;
	}

	btruntime = btsubstream->runtime;
	if (NULL == btruntime) {
		PCM_ERROR(LOG_TAG, "CloseBtSpeechStream: btruntime is NULL!\r\n");
		return INVALIDPRAM;
	}

	btsubstrm_data = btruntime->private_data;
	if (NULL == btsubstrm_data) {
		PCM_ERROR(LOG_TAG, "CloseBtSpeechStream: btsubstrm_data is NULL!\r\n");
		return INVALIDPRAM;
	}

	pStrmProc = btsubstrm_data->pSubstreamPro;
	if (NULL == pStrmProc) {
		PCM_ERROR(LOG_TAG, "CloseBtSpeechStream: pStrmProc is NULL!\r\n");
		return INVALIDPRAM;
	}

	btsubstrm_data->m_eState = SNDRV_PCM_TRIGGER_STOP;
	StateChangeInform(btsubstream);

	mutex_unlock(&g_prSpeechDev.m_SpeechLock); // Relealse SpeechLock for SpeechDev_SynchronizationEx
	ReleaseStreamProcess(btsubstream);
	mutex_lock(&g_prSpeechDev.m_SpeechLock);  // Lock Again

	memset(btsubstrm_data, 0, sizeof(*btsubstrm_data));
	if (btsubstrm_data) {
		PCM_DEBUG(LOG_TAG, "CloseBtSpeechStream: kfree btsubstrm_data!\r\n");
		kfree(btsubstrm_data);
		btruntime->private_data = NULL;
	} else {
		return INVALIDPRAM;
	}

	if (btruntime) {
		PCM_DEBUG(LOG_TAG, "CloseBtSpeechStream: kfree btruntime!\r\n");
		kfree(btruntime);
		btsubstream->runtime = NULL;
	} else {
		return INVALIDPRAM;
	}

	if (btsubstream) {
		PCM_DEBUG(LOG_TAG, "CloseBtSpeechStream: kfree btsubstream!\r\n");
		kfree(btsubstream);
		btsubstream = NULL;
	} else {
		return INVALIDPRAM;
	}


	return NOERR;
}

