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


#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>

#include "pcm_ac83xx.h"
#include "micin.h"
#include "aud_pcm_dbg.h"
#include "winutil.h"
#include "strmproc.h"

#include "pcm_debug.h"
#define LOG_TAG "input"

static bool _fgCaptureDncEn = false;

void Capture_NdcEnable(bool fgEnable)
{
	PCM_DEBUG(LOG_TAG, "Capture_NdcEnable: %d -> %d .\r\n", (int)_fgCaptureDncEn, (int)fgEnable);  
    if (fgEnable != _fgCaptureDncEn)
    {
	    _fgCaptureDncEn = fgEnable;
    }
}


static u32 TransferData(struct snd_pcm_substream  *substream)
{
	u32 u4size = 0;
	u32 hw_pos = 0;
	u32 hw_bytes = 0;
	u32 u4RP = 0;
	u8 *m_lpCurrData;
	WAVE_DATA_BUF_T rMicBuf = {0};
	struct snd_pcm_runtime *runtime = substream->runtime;
	substream_data *substrm_data = runtime->private_data;
	atc_capture_stream *capture_stream = (atc_capture_stream *)substrm_data;


	if(NOERR != capture_stream->prMic->GetBuffer(capture_stream->prMic, &rMicBuf)) {
		PCM_ERROR(LOG_TAG, "TransferData: Get Mic In output buffer error\r\n");
		return 0;
	}

	if(0 == rMicBuf.u4DataSz) {
		return 0;
	}

	u4RP = rMicBuf.u4DataSz & 0xFFFFFF80;

	if(0 == u4RP) {
		return 0;
	}

	u4RP += rMicBuf.u4DataOff;

	if(u4RP >= rMicBuf.u4ChBufSz) {
		u4RP -= rMicBuf.u4ChBufSz;
	}

	hw_pos = (u32)frames_to_bytes(runtime, (snd_pcm_sframes_t)(substrm_data->last_ptr - substrm_data->hw_Base));
	m_lpCurrData = (u8 *)(substrm_data->dma_start + hw_pos);

	if((!m_lpCurrData) || (substrm_data->m_eState != SNDRV_PCM_TRIGGER_START)) {
		return 0;
	}

	while((substrm_data->m_u4MicRP != u4RP) && m_lpCurrData) {
		if(substrm_data->m_eState != SNDRV_PCM_TRIGGER_START) {
			return 0;
		}

		switch(substrm_data->m_SampleType) {
		case PCM_TYPE_M8:
			*m_lpCurrData = *(u8 *)(rMicBuf.u4Buf1 + substrm_data->m_u4MicRP + 1U);
			*m_lpCurrData = *m_lpCurrData + 128;
			m_lpCurrData++;
			u4size = 1U;
			break;

		case PCM_TYPE_S8:
			*m_lpCurrData = *(u8 *)(rMicBuf.u4Buf1 + substrm_data->m_u4MicRP + 1U);
			*m_lpCurrData = *m_lpCurrData + 128;
			m_lpCurrData++;
			*m_lpCurrData = *(u8 *)(rMicBuf.u4Buf2 + substrm_data->m_u4MicRP + 1U);
			*m_lpCurrData = *m_lpCurrData + 128;
			m_lpCurrData++;
			u4size = 2U;
			break;

		case PCM_TYPE_M16:
			*(s16 *)m_lpCurrData = *(s16 *)(rMicBuf.u4Buf1 + substrm_data->m_u4MicRP);
			m_lpCurrData += 2;
			u4size = 2U;
			break;

		case PCM_TYPE_S16:
			*(s16 *)m_lpCurrData = *(s16 *)(rMicBuf.u4Buf1 + substrm_data->m_u4MicRP);
			m_lpCurrData += 2;
			*(s16 *)m_lpCurrData = *(s16 *)(rMicBuf.u4Buf2 + substrm_data->m_u4MicRP);
			m_lpCurrData += 2;
			u4size = 4U;
			break;

		default:
			return 0;
		}

		substrm_data->m_u4MicRP += 2;

		if(substrm_data->m_u4MicRP >= rMicBuf.u4ChBufSz) {
			substrm_data->m_u4MicRP = 0;
		}

		hw_bytes += u4size;
		hw_pos += u4size;

		if(hw_pos >= substrm_data->buffer_size) {
			m_lpCurrData = (u8 *)substrm_data->dma_start;
			hw_pos = 0;
		}

		substrm_data->Used_size += u4size;

		if(substrm_data->Used_size >= substrm_data->period_size) {
			substrm_data->Used_size %= substrm_data->period_size;
			step_hwptr(runtime, hw_bytes);
			hw_bytes = 0;
			snd_pcm_period_elapsed(substream);
		}

		if(substrm_data->m_eState != SNDRV_PCM_TRIGGER_START) {
			return 0;
		}
	}

	if(substrm_data->m_eState == SNDRV_PCM_TRIGGER_START) {
		step_hwptr(runtime, hw_bytes);
	}

	capture_stream->prMic->UpdateRP(capture_stream->prMic, u4RP);

	return 1U;
}




s32 CopyMicDataThread(void *data)
{
	u32 u4Idx = 0;
	s32 ret;
	VIRTUAL_MIC_TYPE type = VMT_NORMAL;
	struct snd_pcm_substream *substream = NULL;
	struct snd_pcm_runtime *runtime = NULL;
	substream_data *substrm_data = (substream_data *)data;
	atc_capture_stream *capture_stream = NULL;

	if(NULL == substrm_data) {
		PCM_ERROR(LOG_TAG, "CopyMicDataThread: substrm_data is NULL!\r\n");
		goto ERROR;
	}
	substream = (struct snd_pcm_substream *)substrm_data->substream;
	
	if(NULL == substream) {
		PCM_ERROR(LOG_TAG, "CopyMicDataThread: substream is NULL!\r\n");
		goto ERROR;
	}
	
	runtime = substream->runtime;
	
	if(NULL == runtime) {
		PCM_ERROR(LOG_TAG, "CopyMicDataThread: runtime is NULL!\r\n");
		goto ERROR;
	}

	capture_stream = (atc_capture_stream *) substrm_data;
	PCM_INFO(LOG_TAG, "CopyMicDataThread: start capture(0x%x) \r\n", (u32)capture_stream);
	if (_fgCaptureDncEn)
		type = VMT_NDC;

	ret = CreateVirtualMicIn(type, &capture_stream->prMic);

	while(!capture_stream->CopyMicDataThread_exit) {
		wait_event_interruptible(capture_stream->CopyMicDataThread_wq, capture_stream->CopyMicDataThread_wq_flag);

		if(capture_stream->CopyMicDataThread_exit) {
			break;
		}

		capture_stream->CopyMicDataThread_wq_flag = 0;

		capture_stream->prMic->Setup(capture_stream->prMic, runtime->rate);
		if(NOERR != capture_stream->prMic->Start(capture_stream->prMic)) {
			PCM_ERROR(LOG_TAG, "Failed to start Virtual Mic In.\r\n");
			goto ERROR;
		}


		while(runtime->status->state == SNDRV_PCM_STATE_RUNNING) {
			msleep(5);

			if(substrm_data->m_eState != SNDRV_PCM_TRIGGER_START) {
				break;
			}

			if(capture_stream->CopyMicDataThread_exit) {
				break;
			}
		
			TransferData(substream);
		}
		if(NOERR != capture_stream->prMic->Stop(capture_stream->prMic)) {
			PCM_ERROR(LOG_TAG, "Failed to stop Virtual Mic In.\r\n");
		}

	}

	DeleteVirtualMicIn(capture_stream->prMic);
	capture_stream->prMic = NULL;

	PCM_INFO(LOG_TAG, "CopyMicDataThread: end capture(0x%x) \r\n", (u32)capture_stream);
	up(&substrm_data->rThreadExitSema);

	return (NOERR);

ERROR:
	up(&substrm_data->rThreadExitSema);
	PCM_ERROR(LOG_TAG, "CopyMicDataThread: end (Error) capture(0x%x) \r\n", (u32)capture_stream);

	return (NORESOURCE);
}


s32 CopyRfDataThread(void *data)
{
	s32 ret = 0;
	struct snd_pcm_substream *substream = NULL;
	struct snd_pcm_runtime *runtime = NULL;
	atc_ref_stream *ref_stream = NULL;
	substream_data *substrm_data = (substream_data *)data;
	ref_stream = (atc_ref_stream*)substrm_data;

	if(NULL == substrm_data) {
		pr_err("[PCM ERR]CopyRfDataThread: substrm_data is NULL!\r\n");
		goto ERROR;
	}
	
	pr_debug("[PCM]CopyRfDataThread: start ref(0x%x) \r\n", (u32)ref_stream);
	if(substrm_data->m_SampleRate == SAMPLE_RATE_8000|| substrm_data->m_SampleRate == SAMPLE_RATE_16000)
	{
	while(true) {
		pr_debug("[PCM]CopyRfDataThread: wait wq \r\n");
		wait_event_interruptible(ref_stream->refDataThread_wq, ref_stream->refDataThread_wq_flag);
		ref_stream->refDataThread_wq_flag = 0;

		if(substrm_data->m_eState == SNDRV_PCM_TRIGGER_STOP && substrm_data->m_eOpen_status == 0){
			pr_debug("stop SpeechDev_EnableBT\n");
			SpeechDev_EnableBT(false, substrm_data->m_SampleRate);
			break;
		}else{
			pr_debug("start SpeechDev_EnableBT\n");
			SpeechDev_EnableBT(true, substrm_data->m_SampleRate);
		}
	}
	}
	else
	{
		VIRTUAL_MIC_TYPE type = VMT_NORMAL;
		substream = (struct snd_pcm_substream *)substrm_data->substream;
	
		if(NULL == substream) {
			PCM_ERROR(LOG_TAG, "CopyMicDataThread: substream is NULL!\r\n");
			goto ERROR;
		}
		
		runtime = substream->runtime;
		runtime->rate = SAMPLE_RATE_48000;
		
		if(NULL == runtime) {
			PCM_ERROR(LOG_TAG, "CopyMicDataThread: runtime is NULL!\r\n");
			goto ERROR;
		}
		pr_debug("QK VoIP create mic runtime->rate:%d\n",runtime->rate);
		ret = CreateVirtualMicIn(type, &ref_stream->prMic);
		ref_stream->prMic->Setup(ref_stream->prMic, runtime->rate);
		if(NOERR != ref_stream->prMic->Start(ref_stream->prMic)) {
			PCM_ERROR(LOG_TAG, "Failed to start Virtual Mic In.\r\n");
			goto ERROR;
		}

		while(!ref_stream->refDataThread_exit) {
			pr_debug("QK wait the mic thread\n");
			wait_event_interruptible(ref_stream->refDataThread_wq, ref_stream->refDataThread_wq_flag);
			ref_stream->refDataThread_wq_flag = 0;
			pr_debug("QK wait end the mic thread\n");

			while(runtime->status->state == SNDRV_PCM_STATE_RUNNING) {
				if(substrm_data->m_eState != SNDRV_PCM_TRIGGER_START) {
					break;
				}

				if(ref_stream->refDataThread_exit) {
					break;
				}
				TransferData(substream);
				msleep(3);
			}
		}

		if(NOERR != ref_stream->prMic->Stop(ref_stream->prMic)) {
			PCM_ERROR(LOG_TAG, "Failed to stop Virtual Mic In.\r\n");
		}
		DeleteVirtualMicIn(ref_stream->prMic);
		ref_stream->prMic = NULL;
			
	}
	pr_info("[PCM]CopyRfDataThread: exit\r\n");
	up(&substrm_data->rThreadExitSema);

	return (NOERR);

ERROR:
	up(&substrm_data->rThreadExitSema);
	pr_err("[PCM ERR]CopyRfDataThread: end (Error) capture(0x%x) \r\n", (u32)ref_stream);

	return (NORESOURCE);
}



