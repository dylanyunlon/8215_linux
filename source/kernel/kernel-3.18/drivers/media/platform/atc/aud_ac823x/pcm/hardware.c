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

/******************************************************************************
*
*[Description
*	  hardware_init funcs for ALSA Audio driver
*
******************************************************************************/

#include "aud_oal.h"
#include "asrc.h"
#include "aud_clock.h"
#include "pcm_ac83xx.h"
#include "outhw.h"
#include "aud_pcm_dbg.h"
#include "micin.h"
#include "speechdev.h"
#include "drv_config.h"
#include "pcm_hardware.h"


u32 _u4OutProc = 0;

void Out_InterruptThread(u32 u4UsedBytes)
{
	u32 u4CurTime = (u32)1000 * (u32)jiffies / (u32)HZ;
	StreamProcess *prTmp = NULL;

	if (STATE_STARTED != snd_chip->m_pbu4State) {
		pr_err("[PCM ERR]Out_InterruptThread: m_pbu4State = %d != STATE_STARTED\r\n",
			(s32)snd_chip->m_pbu4State);
		return;
	}
	if ((u4CurTime - _u4OutProc) >= (snd_chip->m_u4IntrTime << (u32)1))
		pr_err("[PCM ERR]Out_InterruptThread: Process Interval(%d ms).\r\n",
			(s32)(u4CurTime - _u4OutProc));

	_u4OutProc = u4CurTime;

	mutex_lock(&snd_chip->m_HeadLock);	
	prTmp = snd_chip->m_prSPHead;
	while (prTmp) {
		EventInform((u32)EVT_UPDATE_PLAYED_SAMPLE, u4UsedBytes >> 1U, prTmp);
		prTmp = prTmp->m_prNext;
	}
	mutex_unlock(&snd_chip->m_HeadLock);

	u4CurTime = (u32)1000 * (u32)jiffies / (u32)HZ;
	if ((u4CurTime - _u4OutProc) >= (snd_chip->m_u4IntrTime >> (u32)1))
		pr_err("[PCM ERR]Out_InterruptThread: Process time(%d ms).\r\n",
			(s32)(u4CurTime - _u4OutProc));
	_u4OutProc = u4CurTime;
}

static s32 OutputDeviceContext_CB(u32 u4Param, u32 u4Param2)
{
	Out_InterruptThread(u4Param2 & 0xFFFFU);

	return 0;
}

s32 HardWare_Init(ac_83xx *chip)
{
	PCMFMT_T rFmt;
	s32 err = 0;

	pr_debug("[PCM]HardWare_Init: Start\r\n");
	if (chip->m_Intialized) {
		pr_err("[PCM ERR]HardWare_Init: HardWare is Initialized\r\n");
		return INVALIDSTATE;
	}

	if (chip->m_capu4State == STATE_UNINIT) {
		err = MicIn_Init();
		if (err < 0) {
			pr_err("[PCM ERR]HardWare_Init: MicIn_Init fail\r\n");
			MicIn_UnInit();
			return NORESOURCE;
		}
		chip->m_capu4State = STATE_INITED;
	}

	if ((STATE_INITED == chip->m_capu4State) || (STATE_STOPPED == chip->m_capu4State)) {
		chip->m_capu4State = STATE_STOPPED;
	}

	err = SpeechDev_Init();
	if (err < 0) {
		pr_err("[PCM ERR]HardWare_Init: SpeechDev_Init fail\r\n");
		SpeechDev_UnInit();
		return NORESOURCE;
	}

	if (chip->m_pbu4State == STATE_UNINIT) {
		err = DspMixOut_Init();
		if (err < 0) {
			pr_err("[PCM ERR]HardWare_Init: DspMixOut_Init fail\r\n");
			DspMixOut_UnInit();
			return NORESOURCE;
		}
		chip->m_pbu4State = STATE_INITED;
	}

	if (AsrcMgr_Init() < 0) {
		pr_err("[PCM ERR]HardWare_Init: AsrcMgr_Init fail\r\n");
		AsrcMgr_UnInit();
		return NORESOURCE;
	}

	rFmt.u4BW = 16;
	rFmt.u4Chn = 2;
	rFmt.u4FS = 48000;
	DspMixOut_SetFormat(&rFmt);
	if ((STATE_INITED == chip->m_pbu4State) || (STATE_STOPPED == chip->m_pbu4State)) {
		chip->m_pbu4State = STATE_STOPPED;
		AudioOut_RegISTCB(OutputDeviceContext_CB, (u32)0, chip->m_u4IntrTime);
		DspMixOut_Setup();
		_u4OutProc = (u32)1000 * (u32)jiffies / (u32)HZ;
	}

	chip->m_Intialized = true;
	pr_debug("[PCM]HardWare_Init: success!\r\n");

	return NOERR;
}

s32 HardWare_UnInit(ac_83xx *chip)
{
	if ((STATE_INITED == chip->m_pbu4State) || (STATE_STOPPED == chip->m_pbu4State)) {
		chip->m_pbu4State = STATE_UNINIT;
	}

	return NOERR;
}

s32 GetStreamProcess(struct snd_pcm_substream  *substream)
{
	ac_83xx *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	substream_data *substrm_data = runtime->private_data;
	StreamProcess *prTmp = NULL;
	
	mutex_lock(&chip->m_HeadLock);
	prTmp = chip->m_prSPHead;
    
	while (prTmp) {
		if (NOERR == AttachStream(substream, prTmp)) {
			/*Check this stream is rearly detach.*/
			break;
		}
		prTmp = prTmp->m_prNext;
	}
	if (prTmp) {
		pr_debug("[PCM]GetStreamProcess: return 0x%x.\r\n", (u32)prTmp);
		substrm_data->pSubstreamPro = prTmp;
		mutex_unlock(&chip->m_HeadLock);
		return NOERR;
	}

	prTmp = kzalloc(sizeof(*prTmp), GFP_KERNEL);
	if (!prTmp) {
		mutex_unlock(&chip->m_HeadLock);
		return NORESOURCE;
	}

	StreamProcess_Init(prTmp);
	if (NOERR != AttachStream(substream, prTmp)) {
		pr_err("[PCM ERR]GetStreamProcess: AttachStream fail\r\n");
		kfree(prTmp);
		substrm_data->pSubstreamPro = NULL;
		mutex_unlock(&chip->m_HeadLock);
		return NORESOURCE;
	}

	substrm_data->pSubstreamPro = prTmp;

	if (!(chip->m_prSPHead)) {
		chip->m_prSPHead = prTmp;
	} else {
		StreamProcess *prTmp2 = chip->m_prSPHead;

		while (prTmp2->m_prNext) {
			prTmp2 = prTmp2->m_prNext;
		}
		prTmp2->m_prNext = prTmp;
	}

	pr_debug("[PCM]GetStreamProcess: Success with index(%d)\r\n",
		(u32)prTmp->m_u4Idx);

	mutex_unlock(&chip->m_HeadLock);
	return NOERR;
}


u32 ReleaseStreamProcess(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	substream_data *substrm_data = runtime->private_data;
	StreamProcess *strmProc = substrm_data->pSubstreamPro;

	DetachStream(substream);

	mutex_lock(&snd_chip->m_HeadLock); 

	if (StrmProcIsCanDelete(strmProc)) {
		StreamProcess *prTmp = snd_chip->m_prSPHead;

		if (prTmp == strmProc) {
			snd_chip->m_prSPHead = prTmp->m_prNext;
		} else {
			StreamProcess *prPre = prTmp;

			prTmp = prTmp->m_prNext;
			while (prTmp && (prTmp != strmProc)) {
				prPre = prTmp;
				prTmp = prTmp->m_prNext;
			}
			if (prTmp) {
				prPre->m_prNext = prTmp->m_prNext;
			}
		}
		kfree(strmProc);
	}
	
	mutex_unlock(&snd_chip->m_HeadLock);

	return NOERR;
}

