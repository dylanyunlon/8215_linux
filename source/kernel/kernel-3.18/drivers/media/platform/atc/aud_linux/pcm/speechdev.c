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
#include "micin.h"
#include "aud_pcm_dbg.h"
#include "strmproc.h"
#include "dtmf.h"
#include "btoutput.h"

#include "pcm_debug.h"
#define LOG_TAG "sphdev"

#define STATE_WAITFOR_SYNC	0x04U
#define STATE_SCO			STATE_STARTED
#define DL_REF_BUF_SIZE		64000U
#define BT_VOL_MAX          0xFFFFU

u8 *g_pBTDmaArea = NULL;
SpeechDeviceContext g_prSpeechDev;
static u32 g_u4SpeechPrePbSize;
static u32 g_u4DLSilenceFrame = 0;

static void SpeechDev_PreProcess(void);
static void SpeechDev_ULPostProcess(SPEECH_FRAME_T *prFrame);
static void SpeechDev_DLPostProcess(const SPEECH_FRAME_T *prFrame);
static void SpeechDev_ReadDLRefData(SPEECH_FRAME_T *prFrame);
static void SpeechDev_ReadULData(SPEECH_FRAME_T *prFrame);
static void SpeechDev_ReadDLData(SPEECH_FRAME_T *prFrame);

u32 SpeechDev_GetSCOFS(void)
{
    return g_prSpeechEnhance.m_prShareMem->u4SampleRate;
}

void SpeechDev_Enable(bool fgEnable, bool fgTrueSph, u32 u4SampleRate)
{
    PCM_DEBUG(LOG_TAG, "SpeechDev_Enable: En(%d) TrueSph(%d) Fs(%d) (%d %d)\r\n",
        (int)fgEnable, (int)fgTrueSph, (int)u4SampleRate, (int)g_prSpeechDev.m_fgTrueSphEn, g_prSpeechDev.m_u4NDCForCapture);
        
    if (fgEnable)
    {
    	if (fgTrueSph && g_prSpeechDev.m_fgTrueSphEn)
    	{
			PCM_WARN(LOG_TAG, "SCO has been enabled already!!!\r\n");
			return;
    	}
    	if (!fgTrueSph && g_prSpeechDev.m_u4NDCForCapture)
    	{
    	    g_prSpeechDev.m_u4NDCForCapture ++;
			return;
    	}
    }
    else
    {
    	if (fgTrueSph && !g_prSpeechDev.m_fgTrueSphEn)
    	{
			PCM_WARN(LOG_TAG, "SCO is disalbed!!!\r\n");
			return;
    	}
    	if (!fgTrueSph && !g_prSpeechDev.m_u4NDCForCapture)
    	{
			PCM_WARN(LOG_TAG, "SCO for capture is disalbed!!!\r\n");
			return;
    	}
    }

    if (fgEnable)
    {
        if (SpeechDev_IsSCOEnable())
        {
            if (fgTrueSph && u4SampleRate != g_prSpeechDev.m_prVMic->GetFS(g_prSpeechDev.m_prVMic)) 
            {
                SpeechDev_EnableSCO(FALSE, 8000);
				NdcVirtualMicIn_FsChange(u4SampleRate);
                SpeechDev_EnableSCO(TRUE, u4SampleRate);
            }
        }
        else
        {
            SpeechDev_EnableSCO(TRUE, u4SampleRate);
        }
        if (fgTrueSph)
        	g_prSpeechDev.m_fgTrueSphEn = true;
        else
    	    g_prSpeechDev.m_u4NDCForCapture ++;
        
    }
    else
    {
    	if (fgTrueSph)
	    	g_prSpeechDev.m_fgTrueSphEn = false;
	    else
	    	g_prSpeechDev.m_u4NDCForCapture --;
	    if (!g_prSpeechDev.m_fgTrueSphEn && !g_prSpeechDev.m_u4NDCForCapture)
	        SpeechDev_EnableSCO(FALSE, 8000);
    }
        
}


s32 SpeechDev_EnableSCO(bool fgEnable, u32 u4SampleRate)
{
	if (STATE_UNINIT == g_prSpeechDev.m_u4State) {
		return INVALIDPRAM;
	}

	if ((u4SampleRate != 8000U) && (u4SampleRate != 16000U)) {
		PCM_ERROR(LOG_TAG, "SpeechDev_EnableSCO: param u4SampleRate(%d) error!\r\n",
			(s32)u4SampleRate);
		return INVALIDPRAM;
	}
	PCM_DEBUG(LOG_TAG, "SpeechDev_EnableSCO: EnableSCO(%d) start\r\n", (s32)fgEnable);
	if ((STATE_INITED == g_prSpeechDev.m_u4State) && fgEnable) {
		PCM_DEBUG(LOG_TAG, "SpeechDev_EnableSCO: SampleRate(%d)\r\n", (s32)u4SampleRate);
        g_u4DLSilenceFrame = 0;
		g_u4SpeechPrePbSize = (u32)SPEECH_PRE_PB_SIZE * u4SampleRate / 8000U;
		g_prSpeechEnhance.m_prShareMem->u4SampleRate = u4SampleRate;
		g_prSpeechEnhance.m_prShareMem->u4FrameSample = SPEECH_FRAME_SAMPLES * u4SampleRate / 8000U;
		g_prSpeechEnhance.m_prShareMem->u4FrameByte = SPEECH_FRAME_BYTES * u4SampleRate / 8000U;
		mutex_lock(&g_prSpeechDev.m_SpeechLock);
		AsrcMgr_SetSpeechFs(g_prSpeechEnhance.m_prShareMem->u4SampleRate);
		if (NOERR != CreateVirtualMicIn(VMT_NORMAL, &g_prSpeechDev.m_prVMic))
		{
			PCM_ERROR(LOG_TAG, "SpeechDev_EnableSCO: Create VirtualMicIn error!\r\n");
			return NORESOURCE;
		}
		g_prSpeechDev.m_prVMic->Setup(g_prSpeechDev.m_prVMic, g_prSpeechEnhance.m_prShareMem->u4SampleRate);
		g_prSpeechDev.m_prVMic->Start(g_prSpeechDev.m_prVMic);

		g_prSpeechDev.m_rSpeechStrm = OpenBtSpeechStream();
		if (NULL == g_prSpeechDev.m_rSpeechStrm) {
			PCM_ERROR(LOG_TAG, "SpeechDev_EnableSCO: OpenBtSpeechStream return NULL!\r\n");
			mutex_unlock(&g_prSpeechDev.m_SpeechLock);
			return NORESOURCE;
		}

		g_prSpeechDev.m_u4DLNDC = (u4SampleRate == 8000U) ?
			g_prSpeechDev.m_u4DLDelay : g_prSpeechDev.m_u4DL16KDelay;
		g_prSpeechDev.m_u4MICRP = 0;
		g_prSpeechDev.m_u4DLRP = 0;
		BtPCMHw_GetDLBuffer(&g_prSpeechDev.m_rDLBuf);
		SpeechEnhance_EnableSCO(true);
		g_prSpeechDev.m_u4State = STATE_WAITFOR_SYNC;

		/***************for debug chang g_prSpeechDev.m_u4State = STATE_WAITFOR_SYNC->STATE_SCO*/
		SpeechDev_Synchronization();
		mutex_unlock(&g_prSpeechDev.m_SpeechLock);
		if (u4SampleRate == 8000U) {
			PCM_DEBUG(LOG_TAG, "SpeechDev_EnableSCO: Speech delay %d samples\r\n",
				(s32)((g_prSpeechDev.m_u4DLDelay >> 1) + DL_ENHANCE_SAMPLE));
		} else
			PCM_DEBUG(LOG_TAG, "SpeechDev_EnableSCO: Speech delay %d samples\r\n",
				(s32)((g_prSpeechDev.m_u4DL16KDelay >> 1) + DL_16K_ENHANCE_SAMPLE));
	} else if ((STATE_INITED < g_prSpeechDev.m_u4State) && (!fgEnable)) {
		mutex_lock(&g_prSpeechDev.m_SpeechLock);
		g_prSpeechDev.m_prVMic->Stop(g_prSpeechDev.m_prVMic);
		DeleteVirtualMicIn(g_prSpeechDev.m_prVMic);
		g_prSpeechDev.m_prVMic = NULL;
		if (STATE_SCO == g_prSpeechDev.m_u4State)
		{
			BtPCMHw_Stop();
		}
		g_prSpeechDev.m_u4State = STATE_INITED;
		mutex_unlock(&g_prSpeechDev.m_SpeechLock);

		SpeechEnhance_EnableSCO(false);

		mutex_lock(&g_prSpeechDev.m_SpeechLock);
		CloseBtSpeechStream(g_prSpeechDev.m_rSpeechStrm);
		g_prSpeechDev.m_rSpeechStrm = NULL;
		mutex_unlock(&g_prSpeechDev.m_SpeechLock);
	} else {
		PCM_ERROR(LOG_TAG, "SpeechDev_EnableSCO: state error!\r\n");
		return INVALIDSTATE;
	}

	PCM_DEBUG(LOG_TAG, "SpeechDev_EnableSCO(%d) end\r\n", fgEnable);

	return NOERR;
}

bool SpeechDev_IsSCOEnable(void)
{
	return (g_prSpeechDev.m_u4State > STATE_INITED);
}

u32 SpeechDev_EventInform(u32 u4Event, u32 u4Param)
{
	if (STATE_INITED >= g_prSpeechDev.m_u4State) {
		return NOERR;
	}

	if (EVT_STRM_DATA_FINISH == u4Event) {
		StateChangeInform(g_prSpeechDev.m_rSpeechStrm);
		DataAvailableInform(g_prSpeechDev.m_rSpeechStrm);
		return NOERR;
	}

	mutex_lock(&g_prSpeechDev.m_SpeechLock);

	switch (u4Event) {
	case EVT_SPH_SYNC:
		if (STATE_WAITFOR_SYNC == g_prSpeechDev.m_u4State) {
			PCM_DEBUG(LOG_TAG, "SpeechDev_EventInform: STATE_WAITFOR_SYNC event.\r\n");
			SpeechDev_Synchronization();
		}
		break;

	case EVT_PCM_OUT_INTR:
		if (STATE_SCO == g_prSpeechDev.m_u4State) {
			SpeechDev_PreProcess();
		}
		break;

	case EVT_DL_FRAME_FINISH:
		if (STATE_SCO == g_prSpeechDev.m_u4State) {
			SpeechDev_DLPostProcess((SPEECH_FRAME_T *)u4Param);
		}
		break;

	case EVT_UL_FRAME_FINISH:
		if (STATE_SCO == g_prSpeechDev.m_u4State) {
			SpeechDev_ULPostProcess((SPEECH_FRAME_T *)u4Param);
		}
		break;

	default:
		break;
	}

	mutex_unlock(&g_prSpeechDev.m_SpeechLock);
	return NOERR;
}

s32 SpeechDev_Init(void)
{
	g_prSpeechDev.m_rSpeechStrm = NULL;

	g_prSpeechDev.m_u4State = STATE_UNINIT;
	g_prSpeechDev.m_u4DLBufSize = DL_REF_BUF_SIZE;
	g_prSpeechDev.m_u4SCOVolume = MAX_GAIN;
	g_prSpeechDev.m_i4SCOMaxGain = 0;
	g_prSpeechDev.m_fgULMute = false;
	g_prSpeechDev.m_u4MICIdx = 0;
	g_prSpeechDev.m_ai2DLRingBuffer = kmalloc(DL_REF_BUF_SIZE, GFP_KERNEL);
	if (!g_prSpeechDev.m_ai2DLRingBuffer) {
		PCM_ERROR(LOG_TAG, "SpeechDev_Init: get DLRingBuffer failed\r\n");
		return NORESOURCE;
	}
	memset(g_prSpeechDev.m_ai2DLRingBuffer, 0, DL_REF_BUF_SIZE);
	PCM_DEBUG(LOG_TAG, "SpeechDev_Init: m_ai2DLRingBuffer(0x%x).\r\n",
		(u32)g_prSpeechDev.m_ai2DLRingBuffer);

	g_pBTDmaArea = vmalloc(BT_STREAM_BUFFER_SIZE);
	if (!g_pBTDmaArea) {
		kfree(g_prSpeechDev.m_ai2DLRingBuffer);
		g_prSpeechDev.m_ai2DLRingBuffer = NULL;
		return NORESOURCE;
	}
	memset(g_pBTDmaArea, 0, BT_STREAM_BUFFER_SIZE);
	PCM_DEBUG(LOG_TAG, "SpeechDev_Init: g_pBTDmaArea(0x%x).\r\n", (u32)g_pBTDmaArea);

	mutex_init(&g_prSpeechDev.m_SpeechLock);
	g_prSpeechDev.m_fgLoopBack = false;

	if (STATE_UNINIT != g_prSpeechDev.m_u4State) {
		kfree(g_prSpeechDev.m_ai2DLRingBuffer);
		g_prSpeechDev.m_ai2DLRingBuffer = NULL;

		if (g_pBTDmaArea) {
			vfree(g_pBTDmaArea);
			g_pBTDmaArea = NULL;
		}
		return INVALIDPRAM;
	}

	BtPCMHw_Init();
	SpeechEnhance_Init();

#if (ENABLE_DTMF_FUNCTION)
	Dtmf_Init();
#endif

	SpeechDev_SetDevVolume((u32)DEFAULT_DEV_VOL, (u32)DEFAULT_DEV_VOL);
	SpeechDev_SetSCOVolume((u32)DEFAULT_BT_VOL, (u32)DEFAULT_BT_VOL);

	g_prSpeechDev.m_u4State = STATE_INITED;
	g_prSpeechDev.m_i4SCOMaxGain = 0;
	g_prSpeechDev.m_u4SCOGainRange = 12;

	PCM_DEBUG(LOG_TAG, "SpeechDev_Init: Success.\r\n");

	return NOERR;
}

u32 SpeechDev_UnInit(void)
{
	if (STATE_INITED < g_prSpeechDev.m_u4State) {
		SpeechDev_EnableSCO(false, (u32)8000);
	}

	if (STATE_INITED == g_prSpeechDev.m_u4State) {
		BtPCMHw_UnInit();
		SpeechEnhance_UnInit();
#if (ENABLE_DTMF_FUNCTION)
		Dtmf_UnInit();
#endif
	}
	g_prSpeechDev.m_u4State = STATE_UNINIT;

	kfree(g_prSpeechDev.m_ai2DLRingBuffer);
	g_prSpeechDev.m_ai2DLRingBuffer = NULL;

	if (g_pBTDmaArea) {
		vfree(g_pBTDmaArea);
		g_pBTDmaArea = NULL;
	}

	return NOERR;
}

void SpeechDev_HibernationCtrl(bool fgWakeUp)
{
	MicIn_HibernationCtrl(fgWakeUp);
	BtPCMHw_HibernationCtrl(fgWakeUp);
	SpeechEnhance_HibernationCtrl(fgWakeUp);
}

void SpeechDev_SynchronizationEx(void)
{
	mutex_lock(&g_prSpeechDev.m_SpeechLock);
	if (STATE_WAITFOR_SYNC == g_prSpeechDev.m_u4State) {
		PCM_INFO(LOG_TAG, "SynchronizationEx (%d) ms.\r\n", (s32)(1000 * jiffies / HZ));
		g_prSpeechDev.m_u4DLRef = 0;
		g_prSpeechDev.m_u4DLNDC = (g_prSpeechEnhance.m_prShareMem->u4SampleRate == 8000) ?
			g_prSpeechDev.m_u4DLDelay : g_prSpeechDev.m_u4DL16KDelay;
		g_prSpeechDev.m_u4MICRP = 0;
		g_prSpeechDev.m_u4DLRP = 0;
		g_prSpeechDev.m_u4State = STATE_SCO;
		g_prSpeechDev.m_prVMic->ResetRP(g_prSpeechDev.m_prVMic);

		BtPCMHw_Start(g_prSpeechEnhance.m_prShareMem->u4SampleRate);
		BtPCMHw_FillULData((void *)g_prSpeechDev.m_ai2DLRingBuffer, g_u4SpeechPrePbSize);
		g_prSpeechDev.m_u4DLRP = 0;
	}
	else
	{
		PCM_INFO(LOG_TAG, "SynchronizationEx should be called in STATE_WAITFOR_SYNC.\r\n");
	}
	mutex_unlock(&g_prSpeechDev.m_SpeechLock);
	
}

void SpeechDev_Synchronization(void)
{
	u32 u4Len = 0;
	u32 app_ptr_tmp = 0;
	struct snd_pcm_substream *btsubstream = g_prSpeechDev.m_rSpeechStrm;
	struct snd_pcm_runtime *btruntime = btsubstream->runtime;
	substream_data *btsubstrm_data = btruntime->private_data;

	g_prSpeechDev.m_u4State = STATE_WAITFOR_SYNC;
	x_memset(g_prSpeechDev.m_ai2DLRingBuffer, 0, g_prSpeechDev.m_u4DLBufSize);

	PCM_DEBUG(LOG_TAG, "SpeechDev_Synchronization: Copy Bt speech data to appl_buffer Start\r\n");
	app_ptr_tmp = (u32)frames_to_bytes(btruntime,
		(snd_pcm_sframes_t)(btsubstrm_data->appl_ptr - btsubstrm_data->app_Base));
	if ((app_ptr_tmp + g_u4SpeechPrePbSize) >= btsubstrm_data->buffer_size) {
		u4Len = btsubstrm_data->buffer_size - app_ptr_tmp;
		memcpy((void *)(btsubstrm_data->dma_start + app_ptr_tmp),
			(void *)g_prSpeechDev.m_ai2DLRingBuffer, u4Len);
		memcpy((void *)btsubstrm_data->dma_start,
			(void *)(g_prSpeechDev.m_ai2DLRingBuffer + u4Len), (g_u4SpeechPrePbSize - u4Len));
	} else
		memcpy((void *)(btsubstrm_data->dma_start + app_ptr_tmp),
		(void *)g_prSpeechDev.m_ai2DLRingBuffer, g_u4SpeechPrePbSize);

	step_appptr(btruntime, g_u4SpeechPrePbSize);
	StateChangeInform(btsubstream);
	PCM_DEBUG(LOG_TAG, "SpeechDev_Synchronization: end %dms\r\n", (s32)(1000 * jiffies / HZ));
}

static void SpeechDev_ULPostProcess(SPEECH_FRAME_T *prFrame)
{
	u32 u4Len = BtPCMHw_GetULFreeLen();

	if (u4Len < g_prSpeechEnhance.m_prShareMem->u4FrameByte) {
		PCM_ERROR(LOG_TAG, "SpeechDev_ULPostProcess: Lost UL Data. u4Len: %d \r\n", (s32)u4Len);
	} else {
		BtPCMHw_FillULData((void *)prFrame->ULBuf1, g_prSpeechEnhance.m_prShareMem->u4FrameByte);
	}
}

static void SpeechDev_DLPostProcess(const SPEECH_FRAME_T *prFrame)
{
	u32 u4Len;

	if (!prFrame) {
		PCM_ERROR(LOG_TAG, "SpeechDev_DLPostProcess: Error prFrame is NULL\r\n");
		return;
	}

	if ((g_prSpeechDev.m_u4DLNDC + g_prSpeechEnhance.m_prShareMem->u4FrameByte) > g_prSpeechDev.m_u4DLBufSize) {
		u4Len = g_prSpeechDev.m_u4DLBufSize - g_prSpeechDev.m_u4DLNDC;
		if (!g_prSpeechDev.m_fgLoopBack) {
			x_memcpy((void *)(g_prSpeechDev.m_ai2DLRingBuffer + (g_prSpeechDev.m_u4DLNDC >> 1)),
				(void *)prFrame->DLBuf, u4Len);
			x_memcpy((void *)g_prSpeechDev.m_ai2DLRingBuffer, (void *)(&prFrame->DLBuf[u4Len >> 1]),
				g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Len);
		}
		g_prSpeechDev.m_u4DLNDC = g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Len;
	} else {
		if (!g_prSpeechDev.m_fgLoopBack) {
			x_memcpy((void *)(g_prSpeechDev.m_ai2DLRingBuffer + (g_prSpeechDev.m_u4DLNDC >> 1)),
				(void *)prFrame->DLBuf, g_prSpeechEnhance.m_prShareMem->u4FrameByte);
		}
		g_prSpeechDev.m_u4DLNDC += g_prSpeechEnhance.m_prShareMem->u4FrameByte;
		if (g_prSpeechDev.m_u4DLNDC >= g_prSpeechDev.m_u4DLBufSize) {
			g_prSpeechDev.m_u4DLNDC -= g_prSpeechDev.m_u4DLBufSize;
		}
	}

	{
		struct snd_pcm_substream *btsubstream = g_prSpeechDev.m_rSpeechStrm;
		struct snd_pcm_runtime *btruntime = btsubstream->runtime;
		substream_data *btsubstrm_data = btruntime->private_data;
		u32 u4Len;
		u32 app_ptr_tmp;

		app_ptr_tmp = (u32)frames_to_bytes(btruntime,
			(snd_pcm_sframes_t)(btsubstrm_data->appl_ptr - btsubstrm_data->app_Base));
		if ((app_ptr_tmp + g_prSpeechEnhance.m_prShareMem->u4FrameByte) >= btsubstrm_data->buffer_size) {
			u4Len = btsubstrm_data->buffer_size - app_ptr_tmp;
			memcpy((void *)(btsubstrm_data->dma_start + app_ptr_tmp), (void *)prFrame->DLBuf, u4Len);
			memcpy((void *)btsubstrm_data->dma_start, (void *)(&prFrame->DLBuf[u4Len >> 1]),
				(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Len));
		} else
			memcpy((void *)(btsubstrm_data->dma_start + app_ptr_tmp),
			(void *)prFrame->DLBuf, g_prSpeechEnhance.m_prShareMem->u4FrameByte);
		step_appptr(btruntime, g_prSpeechEnhance.m_prShareMem->u4FrameByte);

		if (g_prSpeechEnhance.m_fgDbgEnable) {
			u32 u4Size = (u32)MAX_DBG_FILE_BUFFER -
				(u32)g_prSpeechEnhance.m_rDbgWave[DBG_FILE_PCM_APP_PTR].pull_len;

			if (u4Size < g_prSpeechEnhance.m_prShareMem->u4FrameByte) {
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_PCM_APP_PTR].pwave_buf +
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_PCM_APP_PTR].pull_len),
					(void *)prFrame->DLBuf, u4Size);
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_PCM_APP_PTR].pwave_buf),
					(void *)(&prFrame->DLBuf[u4Size >> 1]),
					(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size));
				g_prSpeechEnhance.m_rDbgWave[DBG_FILE_PCM_APP_PTR].pull_len =
					(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size);
			} else {
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_PCM_APP_PTR].pwave_buf +
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_PCM_APP_PTR].pull_len),
					(void *)prFrame->DLBuf, g_prSpeechEnhance.m_prShareMem->u4FrameByte);
				g_prSpeechEnhance.m_rDbgWave[DBG_FILE_PCM_APP_PTR].pull_len +=
					g_prSpeechEnhance.m_prShareMem->u4FrameByte;
			}
			g_prSpeechEnhance.m_rDbgWave[DBG_FILE_PCM_APP_PTR].Thread_wq_flag = 1;
			wake_up_interruptible(&g_prSpeechEnhance.m_rDbgWave[DBG_FILE_PCM_APP_PTR].Thread_wq);
		}
	}
}

static void SpeechDev_PreProcess(void)
{
	u32 u4UlLen;
	u32 u4DlLen;
	u32 u4UlWP = 0;
	u32 u4DlWP = 0;
	bool fgProcessed = true;
	u32 u4MaxLoop = 1U;

	while (fgProcessed && u4MaxLoop) {
		fgProcessed = false;
		u4MaxLoop--;
		g_prSpeechDev.m_prVMic->GetBuffer(g_prSpeechDev.m_prVMic, &g_prSpeechDev.m_rMicBuf);
		u4UlLen = g_prSpeechDev.m_rMicBuf.u4DataSz;
		g_prSpeechDev.m_u4MICRP = g_prSpeechDev.m_rMicBuf.u4DataOff;

		if (u4UlLen >= (g_prSpeechDev.m_rMicBuf.u4ChBufSz - g_prSpeechEnhance.m_prShareMem->u4FrameByte)) {
			PCM_ERROR(LOG_TAG, "UL Buffer is overflow. RP(%d), Size(%d) (%dms)\r\n",
				(s32)g_prSpeechDev.m_u4MICRP, (s32)u4UlLen, (s32)(1000 * jiffies / HZ));
		}
		u4DlLen = (g_prSpeechDev.m_u4DLNDC >= g_prSpeechDev.m_u4DLRef) ?
			(g_prSpeechDev.m_u4DLNDC - g_prSpeechDev.m_u4DLRef) :
			(g_prSpeechDev.m_u4DLNDC + g_prSpeechDev.m_u4DLBufSize - g_prSpeechDev.m_u4DLRef);

		if (8000 == g_prSpeechEnhance.m_prShareMem->u4SampleRate) {
			if ((u4UlLen >= g_prSpeechEnhance.m_prShareMem->u4FrameByte) &&
				(u4DlLen >= g_prSpeechEnhance.m_prShareMem->u4FrameByte)) {
				SPEECH_FRAME_T *prUlFrame = SpeechEnhance_GetFreeFrame();

				if (prUlFrame) {
					SpeechDev_ReadDLRefData(prUlFrame);
					SpeechDev_ReadULData(prUlFrame);
					SpeechEnhance_EnhanceUL(prUlFrame);
					fgProcessed = true;
				}
			}

			u4DlLen = BtPCMHw_GetDLWP();
			if (u4DlLen >= g_prSpeechDev.m_u4DLRP) {
				u4DlLen -= g_prSpeechDev.m_u4DLRP;
			} else {
				u4DlLen = u4DlLen + g_prSpeechDev.m_rDLBuf.u4ChBufSz - g_prSpeechDev.m_u4DLRP;
			}

			if (u4DlLen >=
				(g_prSpeechDev.m_rDLBuf.u4ChBufSz - g_prSpeechEnhance.m_prShareMem->u4FrameByte)) {
				u4DlWP = BtPCMHw_GetDLWP();
				PCM_ERROR(LOG_TAG, "DL Buffer is overflow. RP(%d), WP(%d) (%dms) \r\n",
					(s32)g_prSpeechDev.m_u4DLRP, (s32)u4DlWP, (s32)(1000 * jiffies / HZ));
			}
			if (u4DlLen >= g_prSpeechEnhance.m_prShareMem->u4FrameByte) {
				SPEECH_FRAME_T *prDlFrame = SpeechEnhance_GetFreeFrame();

				if (prDlFrame) {
					SpeechDev_ReadDLData(prDlFrame);
					SpeechEnhance_EnhanceDL(prDlFrame);
				}
			}
		} else {
			u32 u4DlLen2 = BtPCMHw_GetDLWP();

			if (u4DlLen2 >= g_prSpeechDev.m_u4DLRP) {
				u4DlLen2 -= g_prSpeechDev.m_u4DLRP;
			} else {
				u4DlLen2 = u4DlLen2 + g_prSpeechDev.m_rDLBuf.u4ChBufSz - g_prSpeechDev.m_u4DLRP;
			}

			if (u4DlLen2 >=
				(g_prSpeechDev.m_rDLBuf.u4ChBufSz - g_prSpeechEnhance.m_prShareMem->u4FrameByte)) {
				u4DlWP = BtPCMHw_GetDLWP();
				PCM_ERROR(LOG_TAG, "16K DL Buffer is overflow. ");
				PCM_ERROR(LOG_TAG, "RP(%d), WP(%d) (%dms) \r\n", (s32)g_prSpeechDev.m_u4DLRP,
					(s32)u4DlWP, (s32)(1000 * jiffies / HZ));
			}
			if ((u4UlLen >= g_prSpeechEnhance.m_prShareMem->u4FrameByte) &&
				(u4DlLen >= g_prSpeechEnhance.m_prShareMem->u4FrameByte) &&
				(u4DlLen2 >= g_prSpeechEnhance.m_prShareMem->u4FrameByte)) {
				SPEECH_FRAME_T *prUlFrame = SpeechEnhance_GetFreeFrame();

				if (prUlFrame) {
					SpeechDev_ReadDLRefData(prUlFrame);
					SpeechDev_ReadULData(prUlFrame);
					SpeechDev_ReadDLData(prUlFrame);
					SpeechEnhance_EnhanceUL(prUlFrame);
					fgProcessed = true;
				}
			}
		}
	}
}

static void SpeechDev_ReadDLRefData(SPEECH_FRAME_T *prFrame)
{
	u32 u4Len;

	if ((g_prSpeechDev.m_u4DLRef + g_prSpeechEnhance.m_prShareMem->u4FrameByte) > g_prSpeechDev.m_u4DLBufSize) {
		u4Len = g_prSpeechDev.m_u4DLBufSize - g_prSpeechDev.m_u4DLRef;
		x_memcpy((void *)prFrame->DLDelayBuf,
			(void *)&g_prSpeechDev.m_ai2DLRingBuffer[g_prSpeechDev.m_u4DLRef >> 1], u4Len);
		x_memcpy((void *)(&prFrame->DLDelayBuf[u4Len >> 1]),
			(void *)&g_prSpeechDev.m_ai2DLRingBuffer, g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Len);
		g_prSpeechDev.m_u4DLRef = g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Len;
	} else {
		x_memcpy((void *)prFrame->DLDelayBuf,
			(void *)&g_prSpeechDev.m_ai2DLRingBuffer[g_prSpeechDev.m_u4DLRef >> 1],
			g_prSpeechEnhance.m_prShareMem->u4FrameByte);
		g_prSpeechDev.m_u4DLRef += g_prSpeechEnhance.m_prShareMem->u4FrameByte;
		if (g_prSpeechDev.m_u4DLRef >= g_prSpeechDev.m_u4DLBufSize) {
			g_prSpeechDev.m_u4DLRef -= g_prSpeechDev.m_u4DLBufSize;
		}
	}
}

static void SpeechDev_ReadULData(SPEECH_FRAME_T *prFrame)
{
	u32 u4Len = 0;

	if (g_prSpeechDev.m_fgULMute) {
        x_memset((void *)prFrame->ULBuf1, 0, g_prSpeechEnhance.m_prShareMem->u4FrameByte);
        x_memset((void *)prFrame->ULBuf2, 0, g_prSpeechEnhance.m_prShareMem->u4FrameByte);
		g_prSpeechDev.m_u4MICRP = (g_prSpeechDev.m_u4MICRP + g_prSpeechEnhance.m_prShareMem->u4FrameByte) %
			g_prSpeechDev.m_rMicBuf.u4ChBufSz;
		g_prSpeechDev.m_prVMic->UpdateRP(g_prSpeechDev.m_prVMic, g_prSpeechDev.m_u4MICRP);
	} else {
		if ((g_prSpeechDev.m_u4MICRP + g_prSpeechEnhance.m_prShareMem->u4FrameByte) >
			g_prSpeechDev.m_rMicBuf.u4ChBufSz) {
			u4Len = g_prSpeechDev.m_rMicBuf.u4ChBufSz - g_prSpeechDev.m_u4MICRP;
			x_memcpy((void *)prFrame->ULBuf1,
				(void *)(g_prSpeechDev.m_rMicBuf.u4Buf1 + g_prSpeechDev.m_u4MICRP), u4Len);
			x_memcpy((void *)prFrame->ULBuf2,
				(void *)(g_prSpeechDev.m_rMicBuf.u4Buf2 + g_prSpeechDev.m_u4MICRP), u4Len);
			x_memcpy((void *)(&prFrame->ULBuf1[u4Len >> 1]), (void *)(g_prSpeechDev.m_rMicBuf.u4Buf1),
				g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Len);
			x_memcpy((void *)(&prFrame->ULBuf2[u4Len >> 1]), (void *)(g_prSpeechDev.m_rMicBuf.u4Buf2),
				g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Len);
			g_prSpeechDev.m_u4MICRP = g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Len;
			g_prSpeechDev.m_prVMic->UpdateRP(g_prSpeechDev.m_prVMic, g_prSpeechDev.m_u4MICRP);
		} else {
			x_memcpy((void *)prFrame->ULBuf1,
				(void *)(g_prSpeechDev.m_rMicBuf.u4Buf1 + g_prSpeechDev.m_u4MICRP),
				g_prSpeechEnhance.m_prShareMem->u4FrameByte);
			x_memcpy((void *)prFrame->ULBuf2,
				(void *)(g_prSpeechDev.m_rMicBuf.u4Buf2 + g_prSpeechDev.m_u4MICRP),
				g_prSpeechEnhance.m_prShareMem->u4FrameByte);
			g_prSpeechDev.m_u4MICRP += g_prSpeechEnhance.m_prShareMem->u4FrameByte;
			if (g_prSpeechDev.m_u4MICRP >= g_prSpeechDev.m_rMicBuf.u4ChBufSz) {
				g_prSpeechDev.m_u4MICRP -= g_prSpeechDev.m_rMicBuf.u4ChBufSz;
			}
			g_prSpeechDev.m_prVMic->UpdateRP(g_prSpeechDev.m_prVMic, g_prSpeechDev.m_u4MICRP);
		}
	}
}

static void SpeechDev_ReadDLData(SPEECH_FRAME_T *prFrame)
{
	u32 u4Len = 0;

    if (g_u4DLSilenceFrame < PCMRX_SILENCE_FRAME)
    {
        x_memset((void *)prFrame->DLBuf, 0, g_prSpeechEnhance.m_prShareMem->u4FrameByte);
        g_prSpeechDev.m_u4DLRP += g_prSpeechEnhance.m_prShareMem->u4FrameByte;
        if (g_prSpeechDev.m_u4DLRP >= g_prSpeechDev.m_rDLBuf.u4ChBufSz)
        {
            g_prSpeechDev.m_u4DLRP -= g_prSpeechDev.m_rDLBuf.u4ChBufSz;
        }
        BtPCMHw_UpdateDLRP(g_prSpeechDev.m_u4DLRP);
        g_u4DLSilenceFrame++;
        return;
    }
	if ((g_prSpeechDev.m_u4DLRP + g_prSpeechEnhance.m_prShareMem->u4FrameByte) > g_prSpeechDev.m_rDLBuf.u4ChBufSz) {
		u4Len = g_prSpeechDev.m_rDLBuf.u4ChBufSz - g_prSpeechDev.m_u4DLRP;
		x_memcpy((void *)prFrame->DLBuf,
			(void *)(g_prSpeechDev.m_rDLBuf.u4VirSAdr + g_prSpeechDev.m_u4DLRP), u4Len);
		x_memcpy((void *)(&prFrame->DLBuf[u4Len >> 1]),
			(void *)(g_prSpeechDev.m_rDLBuf.u4VirSAdr),
			g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Len);
		g_prSpeechDev.m_u4DLRP = g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Len;
		BtPCMHw_UpdateDLRP(g_prSpeechDev.m_u4DLRP);
	} else {
		x_memcpy((void *)prFrame->DLBuf, (void *)(g_prSpeechDev.m_rDLBuf.u4VirSAdr + g_prSpeechDev.m_u4DLRP),
			g_prSpeechEnhance.m_prShareMem->u4FrameByte);
		g_prSpeechDev.m_u4DLRP += g_prSpeechEnhance.m_prShareMem->u4FrameByte;
		if (g_prSpeechDev.m_u4DLRP >= g_prSpeechDev.m_rDLBuf.u4ChBufSz) {
			g_prSpeechDev.m_u4DLRP -= g_prSpeechDev.m_rDLBuf.u4ChBufSz;
		}
		BtPCMHw_UpdateDLRP(g_prSpeechDev.m_u4DLRP);
	}

#if (ENABLE_DTMF_FUNCTION)
	if (Dtmf_GetDtmfCtrlType()) {
		Dtmf_Process((s16 *)prFrame->DLBuf);
	}
#endif
}

u32 SpeechDev_EnableULMute(bool fgEnable)
{
	g_prSpeechDev.m_fgULMute = fgEnable ? true : false;
	PCM_DEBUG(LOG_TAG, "SpeechDev_EnableULMute: fgEnable = %d\r\n", (s32)fgEnable);

	return NOERR;
}

s32 SpeechDev_GetSCOVolume(u32 *pu4LVolume, u32 *pu4RVolume)
{
	if ((NULL == pu4LVolume) || (NULL == pu4RVolume)) {
		PCM_ERROR(LOG_TAG, "SpeechDev_GetSCOVolume: pu4LVolume or pu4RVolume is NULL!\r\n");
		return INVALIDPRAM;
	}
	*pu4LVolume = (g_prSpeechDev.m_u4SCOVolume & BT_VOL_MAX);
	*pu4RVolume = g_prSpeechDev.m_u4SCOVolume >> 16;
	PCM_DEBUG(LOG_TAG, "SpeechDev_GetSCOVolume: u4LVolume = %d, u4RVolume = %d\r\n",
		(s32)*pu4LVolume, (s32)*pu4RVolume);

	return NOERR;
}

s32 SpeechDev_SetSCOVolume(u32 u4LVolume, u32 u4RVolume)
{
	u4LVolume = (u4LVolume > BT_VOL_MAX) ? BT_VOL_MAX : u4LVolume;
	u4RVolume = (u4RVolume > BT_VOL_MAX) ? BT_VOL_MAX : u4RVolume;
	PCM_DEBUG(LOG_TAG, "SpeechDev_SetSCOVolume: u4LVolume = %d, u4RVolume = %d\r\n",
		(s32)u4LVolume, (s32)u4RVolume);
	g_prSpeechDev.m_u4SCOVolume = (u4RVolume << 16) | (u4LVolume);
	BtOutput_SetGain(g_prSpeechDev.m_u4SCOVolume);
	PCM_DEBUG(LOG_TAG, "SpeechDev_SetSCOVolume: m_u4SCOVolume = 0x%08x\r\n",
		(s32)g_prSpeechDev.m_u4SCOVolume);

	return NOERR;
}

s32 SpeechDev_GetDevVolume(u32 *pu4LVolume, u32 *pu4RVolume)
{
	if ((NULL == pu4LVolume) || (NULL == pu4RVolume)) {
		PCM_ERROR(LOG_TAG, "SpeechDev_GetDevVolume: pu4LVolume or pu4RVolume is NULL!\r\n");
		return INVALIDPRAM;
	}
	*pu4LVolume = (g_prSpeechDev.m_u4DevVolume & 0xFF00U) >> 8;
	*pu4RVolume = g_prSpeechDev.m_u4DevVolume >> 24;
	PCM_DEBUG(LOG_TAG, "SpeechDev_GetDevVolume: u4LVolume = %d, u4RVolume = %d\r\n",
		(s32)*pu4LVolume, (s32)*pu4RVolume);

	return NOERR;
}

s32 SpeechDev_SetDevVolume(u32 u4LVolume, u32 u4RVolume)
{
	u4LVolume = (u4LVolume > 255U) ? 255U : u4LVolume;
	u4RVolume = (u4RVolume > 255U) ? 255U : u4RVolume;
	PCM_DEBUG(LOG_TAG, "SpeechDev_SetDevVolume: u4LVolume = %d, u4RVolume = %d\r\n",
		(s32)u4LVolume, (s32)u4RVolume);
	g_prSpeechDev.m_u4DevVolume = (u4RVolume << 24) | (u4LVolume << 8);
	DeviceSPH_SetGain(g_prSpeechDev.m_u4DevVolume);
	PCM_DEBUG(LOG_TAG, "SpeechDev_SetDevVolume: m_u4DevVolume = 0x%08x\r\n",
		(s32)g_prSpeechDev.m_u4DevVolume);

	return NOERR;
}

s32 SpeechDev_GetSCOMaxGain(void)
{
	return g_prSpeechDev.m_i4SCOMaxGain;
}

u32 SpeechDev_SetSCOMaxGain(s32 i4MaxGain)
{
	g_prSpeechDev.m_i4SCOMaxGain = i4MaxGain;

	return NOERR;
}

u32 SpeechDev_GetSCOGainRange(void)
{
	return g_prSpeechDev.m_u4SCOGainRange;
}

u32 SpeechDev_SetSCOGainRange(u32 u4GainRange)
{
	g_prSpeechDev.m_u4SCOGainRange = u4GainRange;

	return NOERR;
}

s32 SpeechDev_GetEnhanceParam(PCM_SPEECH_CONF *prSpeechConf)
{
	if (NULL == prSpeechConf) {
		PCM_ERROR(LOG_TAG, "SpeechDev_GetEnhanceParam: prSpeechConf error!\r\n");
		return INVALIDPRAM;
	}
	SpeechEnhance_GetEnhanceParam(prSpeechConf);

	return NOERR;
}

s32 SpeechDev_SetEnhanceParam(const PCM_SPEECH_CONF *prSpeechConf)
{
	if (NULL == prSpeechConf) {
		PCM_ERROR(LOG_TAG, "SpeechDev_SetEnhanceParam: prSpeechConf error!\r\n");
		return INVALIDPRAM;
	}
	SpeechEnhance_SetEnhanceParam(prSpeechConf);

	return NOERR;
}

s32 SpeechDev_GetEnhance16KParam(PCM_SPEECH_16K_CONF *prSpeechConf)
{
	if (NULL == prSpeechConf) {
		PCM_ERROR(LOG_TAG, "SpeechDev_Get16KEnhanceParam: prSpeechConf error!\r\n");
		return INVALIDPRAM;
	}
	SpeechEnhance_GetEnhance16KParam(prSpeechConf);

	return NOERR;
}

s32 SpeechDev_SetEnhance16KParam(PCM_SPEECH_16K_CONF *prSpeechConf)
{
	if (NULL == prSpeechConf) {
		PCM_ERROR(LOG_TAG, "SpeechDev_SetEnhance16KParam: prSpeechConf error!\r\n");
		return INVALIDPRAM;
	}
	SpeechEnhance_SetEnhance16KParam(prSpeechConf);

	return NOERR;
}

s32 SpeechDev_SetDmnrParam(const PCM_SPEECH_CONF *prSpeechConf)
{
	if (NULL == prSpeechConf) {
		PCM_ERROR(LOG_TAG, "SpeechDev_SetDmnrParam: prSpeechConf error!\r\n");
		return INVALIDPRAM;
	}
	SpeechEnhance_SetDmnrParam(prSpeechConf);

	return NOERR;
}


u32 SpeechDev_EnableDmnr(bool fgEnable)
{
	return SpeechEnhance_EnableDmnr(fgEnable);
}

u32 SpeechDev_IsDmnrEnable(void)
{
	return SpeechEnhance_IsDmnrEnable();
}

s32 SpeechDev_SetComRxParam(const PCM_SPEECH_CONF *prSpeechConf)
{
	if (NULL == prSpeechConf) {
		PCM_ERROR(LOG_TAG, "SpeechDev_SetComRxParam: prSpeechConf error!\r\n");
		return INVALIDPRAM;
	}
	SpeechEnhance_SetComRxParam(prSpeechConf);

	return NOERR;
}


s32 SpeechDev_SetComTxParam(const PCM_SPEECH_CONF *prSpeechConf)
{
	if (NULL == prSpeechConf) {
		PCM_ERROR(LOG_TAG, "SpeechDev_SetComTxParam: prSpeechConf error!\r\n");
		return INVALIDPRAM;
	}
	SpeechEnhance_SetComTxParam(prSpeechConf);

	return NOERR;
}

s32 SpeechDev_SetDmnr16kParam(const PCM_SPEECH_16K_CONF *prSpeechConf)
{
	if (NULL == prSpeechConf) {
		PCM_ERROR(LOG_TAG, "SpeechDev_SetEnhanceParam: prSpeechConf error!\r\n");
		return INVALIDPRAM;
	}
	SpeechEnhance_SetDmnr16kParam(prSpeechConf);

	return NOERR;
}

s32 SpeechDev_SetFilter16kParam(const signed short *pData, u32 u4DataLen)
{
	if (NULL == pData) {
		PCM_ERROR(LOG_TAG, "SpeechDev_SetFilter16kParam: prSpeechConf error!\r\n");
		return INVALIDPRAM;
	}
	SpeechEnhance_SetFilter16kParam(pData, u4DataLen);

	return NOERR;
}

u32 SpeechDev_SetARM2SpeechLog(u32 u4logLevel)
{
	return SpeechEnhance_SetARM2SpeechLog(u4logLevel);
}

bool SpeechDev_EnableLoopback(bool fgEnable)
{
	g_prSpeechDev.m_fgLoopBack = fgEnable;

	return true;
}

s32 SpeechDev_SetDLDelay(u32 u4Samples)
{
	PCM_DEBUG(LOG_TAG, "SpeechDev_SetDLDelay: Set Delay %d Samples\r\n", (s32)u4Samples);
	if (u4Samples < DL_ENHANCE_SAMPLE) {
		PCM_ERROR(LOG_TAG, "SpeechDev_SetDLDelay: Delay should be larger than %d\r\n",
			DL_ENHANCE_SAMPLE - 1);
		return INVALIDPRAM;
	}
	g_prSpeechDev.m_u4DLDelay = (u4Samples - DL_ENHANCE_SAMPLE) << 1;

	return NOERR;
}

u32 SpeechDev_GetDLDelay(void)
{
	return ((g_prSpeechDev.m_u4DLDelay >> 1U) + (u32)DL_ENHANCE_SAMPLE);
}

s32 SpeechDev_SetDL16KDelay(u32 u4Samples)
{
	PCM_DEBUG(LOG_TAG, "SpeechDev_SetDL16KDelay: Set Delay %d Samples\r\n", (s32)u4Samples);
	if (u4Samples < DL_16K_ENHANCE_SAMPLE) {
		PCM_ERROR(LOG_TAG, "SpeechDev_SetDL16KDelay: Delay should be larger than %d\r\n",
			DL_16K_ENHANCE_SAMPLE - 1);
		return INVALIDPRAM;
	}
	g_prSpeechDev.m_u4DL16KDelay = (u4Samples - DL_16K_ENHANCE_SAMPLE) << 1;

	return NOERR;
}

u32 SpeechDev_GetDL16KDelay(void)
{
	return ((g_prSpeechDev.m_u4DL16KDelay >> 1U) + (u32)DL_16K_ENHANCE_SAMPLE);
}

u32 SpeechDev_EnablePLC(bool fgEnable)
{
	return SpeechEnhance_EnablePLC(fgEnable);
}

u32 SpeechDev_EnableDump(bool fgEnable)
{
	return SpeechEnhance_EnableDump(fgEnable);
}

