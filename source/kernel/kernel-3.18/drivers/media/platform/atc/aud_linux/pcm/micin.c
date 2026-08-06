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


#include <linux/fs.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/semaphore.h>

#include "winutil.h"
#include "micin.h"
#include "aud_pcm_dbg.h"
#include "bt_speech.h"
#include "speechdev.h"

#if MICIN_USE_DSP_MEMORY
#include "GpsMix_if.h"
#endif

#include "pcm_debug.h"
#define LOG_TAG "micin"

static EXT_MIC_CONF g_rExtMicConf;
MicIn g_rMic;
static ULDataFile g_ULFile;

static AUDIO_SAMPLING_T MicIn_GetFSIdx(u32 u4FS)
{
	AUDIO_SAMPLING_T eFS = FS_UNKNOWN;

	switch(u4FS) {
	case 8000U:
		eFS = FS_8K;
		break;

	case 16000U:
		eFS = FS_16K;
		break;

	case 22050U:
		eFS = FS_22K;
		break;

	case 24000U:
		eFS = FS_24K;
		break;

	case 32000U:
		eFS = FS_32K;
		break;

	case 44100U:
		eFS = FS_44K;
		break;

	case 48000U:
		eFS = FS_48K;
		break;

	case 64000U:
		eFS = FS_64K;
		break;

	case 88200U:
		eFS = FS_88K;
		break;

	case 96000U:
		eFS = FS_96K;
		break;

	case 176400U:
		eFS = FS_176K;
		break;

	case 192000U:
		eFS = FS_192K;
		break;

	default:
		eFS = FS_48K;
		break;
	}

	return eFS;
}

s32 MicIn_Init(void)
{
	s32 i4Ret = NOERR;
	u32 u4Idx = 0;

	g_rMic.m_u4State = STATE_UNINIT;
	g_rMic.m_u4PrimaryMicIndex = 0;
	g_rMic.m_u4StartTimes = 0;

	g_rMic.m_rMicCfg.eSrc = INT_MICIN;
	g_rMic.m_rMicCfg.eI2sPin = PINMUX_I2SMICIN_DEFAULT;
	g_rMic.m_rMicCfg.eFs = FS_8K;
	g_rMic.m_rMicCfg.u4SrcBitNum = 16;
	g_rMic.m_rMicCfg.eOutBitNum = LIN_16;

	g_rMic.m_rMicCfg.u4BufPhyAdr = 0;
	g_rMic.m_rMicCfg.u4BufSz = MIC_BUFFER_SIZE;

	for(u4Idx = 0; u4Idx < MICIN_MAX_NUM; u4Idx++) {
		g_rMic.aOwner[u4Idx] = NULL;
	}


	g_rMic.m_prMicHal = MicHal_New();

	if(!g_rMic.m_prMicHal) {
		PCM_ERROR(LOG_TAG, "MicIn_Init: Create MIC Hal failed!\r\n");
		i4Ret = NORESOURCE;
	} else {
		if(g_rExtMicConf.u4ExtMicEn) {
			g_rMic.m_rMicCfg.eSrc = EXT_MICIN;
			g_rMic.m_rMicCfg.eI2sPin = (AUD_PINMUX_I2SMICIN)g_rExtMicConf.u4ExtMicI2sPin;
			g_rMic.m_rMicCfg.u4SrcBitNum = g_rExtMicConf.u4ExtMicSrcBitNum;
			g_rMic.m_rMicCfg.eFs = MicIn_GetFSIdx(g_rExtMicConf.u4ExtMicFs);
			PCM_DEBUG(LOG_TAG, "MicIn_Init: Use External Mic PinMux(%d) FS_Idx(%d) BW(%d)\r\n",
				 (u32)g_rMic.m_rMicCfg.eI2sPin, (u32)g_rMic.m_rMicCfg.eFs,
				 g_rMic.m_rMicCfg.u4SrcBitNum);
		} else {
			g_rMic.m_rMicCfg.eSrc = INT_MICIN;
			g_rMic.m_rMicCfg.eFs = FS_48K; //FS_8K; //atc6048
			PCM_DEBUG(LOG_TAG, "MicIn_Init: Use Internal Mic FS_Idx(%d) BW(%d)\r\n",
				 (u32)g_rMic.m_rMicCfg.eFs, g_rMic.m_rMicCfg.u4SrcBitNum);
		}

#if (MICIN_USE_DSP_MEMORY)
		MicIn_InitBuffer();
		g_rMic.m_prMicHal->rHwIf.Setup(g_rMic.m_prMicHal, &g_rMic.m_rMicCfg);
#else
		g_rMic.m_prMicHal->rHwIf.Setup(g_rMic.m_prMicHal, &g_rMic.m_rMicCfg);
		MicIn_InitBuffer();
#endif

		g_rMic.m_u4State = STATE_INITED;
	}

	if(g_rMic.m_fgDataFromFile) {
		ULDataFile_Init();
	}

	mutex_init(&g_rMic.m_MicLock);

	return i4Ret;
}

u32 MicIn_UnInit(void)
{
	u32 u4Idx = 0;

	for(u4Idx = 0; u4Idx < MICIN_MAX_NUM; u4Idx++) {
		g_rMic.aOwner[u4Idx] = NULL;
	}


	if((STATE_INITED == g_rMic.m_u4State) || (STATE_STOPPED == g_rMic.m_u4State)) {
		g_rMic.m_u4State = STATE_UNINIT;
	}

	if(g_rMic.m_prMicHal) {
		g_rMic.m_prMicHal->Delete(g_rMic.m_prMicHal);
		g_rMic.m_prMicHal = NULL;
	}

	if(g_rMic.m_fgDataFromFile) {
		ULDataFile_UnInit();
	}

	return NOERR;
}

void MicIn_HibernationCtrl(bool fgWakeUp)
{
	if(fgWakeUp) {
		if(g_rMic.m_prMicHal) {
			g_rMic.m_prMicHal->rHwIf.Setup(g_rMic.m_prMicHal, &g_rMic.m_rMicCfg);
		}
	} else {
//		MicIn_Stop(NULL);
	}
}

void MicIn_SetPrimaryMic(u32 u4Index)
{
	if(u4Index > 1U) {
		u4Index = 1U;
	}

	g_rMic.m_u4PrimaryMicIndex = u4Index;
	PCM_DEBUG(LOG_TAG, "MicIn_SetPrimaryMic: Set mic%d as primary mic!\r\n",
		 (u32)(g_rMic.m_u4PrimaryMicIndex + 1U));
}

u32 MicIn_GetPrimaryMic(void)
{
	return g_rMic.m_u4PrimaryMicIndex;
}

void MicIn_SetExtMicCfg(EXT_MIC_CONF rExtMicConf)
{
	memcpy(&g_rExtMicConf, &rExtMicConf, sizeof(EXT_MIC_CONF));
	PCM_DEBUG(LOG_TAG, "MicIn_SetExtMicCfg: En(%d) PinMux(%d) FS(%d) BW(%d)\r\n",
		 (s32)g_rExtMicConf.u4ExtMicEn, (s32)g_rExtMicConf.u4ExtMicI2sPin,
		 (s32)g_rExtMicConf.u4ExtMicFs, (s32)g_rExtMicConf.u4ExtMicSrcBitNum);
}

s32 MicIn_SetGain(u32 u4MicGain)
{
	if(u4MicGain <= 63U) {
		g_rMic.m_rMicCfg.u4MicGain = u4MicGain;
	} else {
		PCM_DEBUG(LOG_TAG, "MicIn_SetGain: gain(%d) is not between 0 and 63, so set fail!\r\n",
		       (u32)u4MicGain);
		return INVALIDPRAM;
	}

	PCM_DEBUG(LOG_TAG, "MicIn_SetGain: %d (%d dB) \r\n",
		 (u32)g_rMic.m_rMicCfg.u4MicGain, (s32)(g_rMic.m_rMicCfg.u4MicGain - 14U));

	return NOERR;
}

u32 MicIn_GetGain(void)
{
	PCM_DEBUG(LOG_TAG, "MicIn_GetGain: %d (%d dB) \r\n",
		 (u32)g_rMic.m_rMicCfg.u4MicGain, (s32)(g_rMic.m_rMicCfg.u4MicGain - 14U));

	return g_rMic.m_rMicCfg.u4MicGain;
}

u32 MicIn_SetFS(u32 u4SampleRate)
{
	switch(u4SampleRate) {
	case 8000U:
		g_rMic.m_rMicCfg.eFs = FS_8K;
		break;

	case 16000U:
		g_rMic.m_rMicCfg.eFs = FS_16K;
		break;

	case 48000U:
		g_rMic.m_rMicCfg.eFs = FS_48K;
		break;

	default:
		g_rMic.m_rMicCfg.eFs = FS_8K;
		break;
	}

	PCM_DEBUG(LOG_TAG, "[PCM]MicIn_SetFS: %d  %d \r\n",
		 (u32)u4SampleRate, (u32)g_rMic.m_rMicCfg.eFs);

	return NOERR;
}

u32 MicIn_GetFS(void)
{
	u32 u4SampleRate = 8000U;

	switch(g_rMic.m_rMicCfg.eFs) {
	case FS_8K:
		u4SampleRate = 8000U;
		break;

	case FS_16K:
		u4SampleRate = 16000U;
		break;

	case FS_48K:
		u4SampleRate = 48000U;
		break;

	default:
		u4SampleRate = 8000U;
		break;
	}

	return u4SampleRate;
}

bool MicIn_IsStart(void)
{
	return (STATE_STARTED == g_rMic.m_u4State);
}

u32 MicIn_GetAvailableFs(void)
{
	return (48000U);
}


s32 MicIn_Start(void *owner)
{
	u32 u4Fs;
	u32 u4Idx ;

	ASSERT(owner);
	PCM_INFO(LOG_TAG, "MicIn_Start: owner(0x%x).\r\n", (u32)owner);
	mutex_lock(&g_rMic.m_MicLock);
    for (u4Idx = 0; u4Idx < MICIN_MAX_NUM; u4Idx++)
    {
        if (NULL == g_rMic.aOwner[u4Idx])
        {
            break;
        }
    }
    if (MICIN_MAX_NUM == u4Idx)
    {
        PCM_ERROR(LOG_TAG, "MicInHal_Start: No free MicIn array!\r\n");
        mutex_unlock(&g_rMic.m_MicLock);
        return (NORESOURCE);
    }
	g_rMic.aOwner[u4Idx] = owner;

	if (!g_rMic.m_u4StartTimes)
	{
		u4Fs = MicIn_GetAvailableFs();
		MicIn_SetFS(u4Fs);
		if((STATE_INITED == g_rMic.m_u4State) || (STATE_STOPPED == g_rMic.m_u4State))  {

			if(g_rMic.m_fgDataFromFile) {
				g_ULFile.m_u4WP = 0;
				g_ULFile.m_u4DataPos = sizeof(WaveHeader);
				g_ULFile.m_u4DataPos2 = sizeof(WaveHeader);
			}

			g_rMic.m_prMicHal->rHwIf.CfgUpd(g_rMic.m_prMicHal, &g_rMic.m_rMicCfg);
			g_rMic.m_prMicHal->rHwIf.Start(g_rMic.m_prMicHal, 0);

			g_rMic.m_u4State = STATE_STARTED;
		} else {
		    PCM_ERROR(LOG_TAG, "MicIn_Start: INVALIDSTATE\r\n");
			mutex_unlock(&g_rMic.m_MicLock);
			return INVALIDSTATE;
		}
	}
	g_rMic.m_u4StartTimes ++;
	PCM_INFO(LOG_TAG, "MicIn_Start: Times(%d) end.\r\n", g_rMic.m_u4StartTimes);

	mutex_unlock(&g_rMic.m_MicLock);

	return NOERR;
}

s32 MicIn_Stop(void *owner)
{
	s32 i4Ret = NOERR;
	u32 u4Idx;

	PCM_DEBUG(LOG_TAG, "MicIn_Stop owner(0x%x) StartTimes(%d).\r\n", (u32)owner, g_rMic.m_u4StartTimes);
	if (!g_rMic.m_u4StartTimes)
		return (INVALIDSTATE);


    for (u4Idx = 0; u4Idx < MICIN_MAX_NUM; u4Idx++)
    {
        if (owner == g_rMic.aOwner[u4Idx])
        {
            break;
        }
    }
    if (MICIN_MAX_NUM == u4Idx)
    {
        PCM_ERROR(LOG_TAG, "MicIn_Stop: can't find owner!!!\r\n");
        return (INVALIDPRAM);
    }

	mutex_lock(&g_rMic.m_MicLock);

	g_rMic.aOwner[u4Idx] = NULL;

	g_rMic.m_u4StartTimes --;

	if(!g_rMic.m_u4StartTimes) {
		if(STATE_STARTED == g_rMic.m_u4State) {

			if(g_rMic.m_fgDataFromFile) {
				g_ULFile.m_u4WP = 0;
				g_ULFile.m_u4DataPos = sizeof(WaveHeader);
				g_ULFile.m_u4DataPos2 = sizeof(WaveHeader);
			}

			g_rMic.m_prMicHal->rHwIf.Stop(g_rMic.m_prMicHal, 0);

			g_rMic.m_u4State = STATE_STOPPED;
			PCM_INFO(LOG_TAG, "MicIn_Stop: Stop MIC IN.\r\n");
		} else {
			PCM_DEBUG(LOG_TAG, "MicIn_Stop: INVALIDSTATE\r\n");
			i4Ret = INVALIDSTATE;
		}
	}

	mutex_unlock(&g_rMic.m_MicLock);


	return i4Ret;
}


void MicIn_InitBuffer(void)
{
#if (MICIN_USE_DSP_MEMORY)
	AUD_MIC_BUF_FOR_BT_INFO rAudMicBuf;

	_AudGetMicBufInfo(&rAudMicBuf);

	g_rMic.m_rMicCfg.u4BufPhyAdr = rAudMicBuf.u4TotalPhy;

	if(rAudMicBuf.u4MicBufBTSize > MIC_BUFFER_SIZE) {
		rAudMicBuf.u4MicBufBTSize = MIC_BUFFER_SIZE;
	}

	g_rMic.m_rBuf.u4ChBufSz = rAudMicBuf.u4MicBufBTSize >> 1;
	g_rMic.m_rBuf.u4Chn = 2;
	g_rMic.m_rBuf.u4Buf1 = rAudMicBuf.u4WorkBufferSA + rAudMicBuf.u4MicPageSA + rAudMicBuf.u4MicBufBTSA;
#else
	AUD_DATA_BUF_T rBuf;

	g_rMic.m_prMicHal->rHwIf.GetBuf(g_rMic.m_prMicHal, &rBuf);

	g_rMic.m_rBuf.u4ChBufSz = rBuf.u4ChBufSz;
	g_rMic.m_rBuf.u4Chn = rBuf.u4Chn;
	g_rMic.m_rBuf.u4Buf1 = rBuf.u4VirSAdr;
#endif

	g_rMic.m_rBuf.u4Buf2 = (g_rMic.m_rBuf.u4Chn == 1) ? g_rMic.m_rBuf.u4Buf1 :
			       (g_rMic.m_rBuf.u4Buf1 + g_rMic.m_rBuf.u4ChBufSz);
	g_rMic.m_rBuf.u4DataOff = 0;
	g_rMic.m_rBuf.u4DataSz = g_rMic.m_rBuf.u4ChBufSz - MICIN_SAFE_READ_SIZE;

	PCM_DEBUG(LOG_TAG, "MicIn_InitBuffer: MicIn Buf: 0x%x, 0x%x, %d, %d \r\n",
		 (u32)g_rMic.m_rBuf.u4Buf1, (u32)g_rMic.m_rBuf.u4Buf2,
		 (u32)g_rMic.m_rBuf.u4ChBufSz, (u32)g_rMic.m_rBuf.u4Chn);
}

u32 MicIn_GetBuffer(WAVE_DATA_BUF_T *prBuffer)
{
	if(g_rMic.m_fgDataFromFile) {
		ULDataFile_GetBuf(prBuffer);
	} else {
		x_memcpy(prBuffer, &g_rMic.m_rBuf, sizeof(WAVE_DATA_BUF_T));

		if(1 == g_rMic.m_u4PrimaryMicIndex) {
			prBuffer->u4Buf2 = g_rMic.m_rBuf.u4Buf1;
			prBuffer->u4Buf1 = g_rMic.m_rBuf.u4Buf2;
		}
	}

	return NOERR;
}

void MicIn_UpdateRP(u32 u4RP)
{
}


u32 MicIn_GetWP(void)
{
	u32 u4WP = 0;

	if(g_rMic.m_fgDataFromFile) {
		u4WP = ULDataFile_GetWP();
	} else {
		u4WP = g_rMic.m_prMicHal->rHwIf.GetPoint(g_rMic.m_prMicHal);
	}

	return u4WP;
}



bool MicIn_IsDataFromFile(void)
{
	return g_rMic.m_fgDataFromFile;
}

void MicIn_SetDataFromFile(bool fgDataFromFile)
{
	g_rMic.m_fgDataFromFile = fgDataFromFile;
	PCM_DEBUG(LOG_TAG, "MicIn_SetDataFromFile: fgDataFromFile(%d)\r\n",
		 (u32)g_rMic.m_fgDataFromFile);
}

bool MicIn_EnableDataFromFile(bool fgEnable)
{
	bool fgRet = false;

	if(STATE_STARTED != g_rMic.m_u4State) {
		if(g_rMic.m_fgDataFromFile != fgEnable) {
			PCM_DEBUG(LOG_TAG, "MicIn_EnableDataFromFile: fgEnable(%d)\r\n",
			       (u32)fgEnable);

			if(fgEnable) {
				if(NOERR == ULDataFile_Init()) {
					g_rMic.m_fgDataFromFile = true;
				}
			} else {
				ULDataFile_UnInit();
				g_rMic.m_fgDataFromFile = false;
			}
		}

		fgRet = true;
	}

	return fgRet;
}


s32 ULDataFile_Init(void)
{
	g_ULFile.m_rBuf.u4Buf1 = 0;
	g_ULFile.m_rBuf.u4Chn = 2U;
	g_ULFile.m_rBuf.u4ChBufSz = SPEECH_FRAME_BYTES * 2U * 10U;
	g_ULFile.m_u4DataPos = 0;
	g_ULFile.m_u4DataPos2 = 0;
	g_ULFile.m_u4WP = 0;

	if(!g_ULFile.m_rBuf.u4Buf1) {
		g_ULFile.m_rBuf.u4Buf1 = (u32)kmalloc(g_ULFile.m_rBuf.u4ChBufSz * g_ULFile.m_rBuf.u4Chn, GFP_KERNEL);

		if(0 == g_ULFile.m_rBuf.u4Buf1) {
			PCM_ERROR(LOG_TAG, "ULDataFile_Init: kmalloc buffer error!\r\n");
			return NORESOURCE;
		}

		memset((void *)g_ULFile.m_rBuf.u4Buf1, 0, g_ULFile.m_rBuf.u4ChBufSz * g_ULFile.m_rBuf.u4Chn);
		g_ULFile.m_rBuf.u4Buf2 = g_ULFile.m_rBuf.u4Buf1 + g_ULFile.m_rBuf.u4ChBufSz;
	}

	return NOERR;
}

u32 ULDataFile_UnInit(void)
{
	g_ULFile.m_u4WP = 0;
	g_ULFile.m_u4DataPos = 0;
	g_ULFile.m_u4DataPos2 = 0;

	if(g_ULFile.m_rBuf.u4Buf1) {
		kfree((void *)g_ULFile.m_rBuf.u4Buf1);
		g_ULFile.m_rBuf.u4Buf1 = 0;
		g_ULFile.m_rBuf.u4Buf2 = 0;
	}

	return NOERR;
}

u32 ULDataFile_GetBuf(WAVE_DATA_BUF_T *prBuf)
{
	prBuf->u4Buf1 = g_ULFile.m_rBuf.u4Buf1;
	prBuf->u4Buf2 = g_ULFile.m_rBuf.u4Buf2;
	prBuf->u4ChBufSz = g_ULFile.m_rBuf.u4ChBufSz;
	prBuf->u4Chn = g_ULFile.m_rBuf.u4Chn;
	prBuf->u4DataOff = 0;
	prBuf->u4DataSz = 0;

	return NOERR;
}

u32 ULDataFile_GetWP(void)
{
	return g_ULFile.m_u4WP;
}

s32 ULDataFile_SetRP(u32 u4RP)
{
	if(u4RP == g_ULFile.m_u4WP) {
		loff_t t_cur_pos = 0;
		mm_segment_t fs;
		int32_t u4size = 0;

		if(!g_ULFile.m_rBuf.u4Buf1) {
		    PCM_ERROR(LOG_TAG, "ULDataFile_SetRP: u4Buf1 is 0!\r\n");
			return NORESOURCE;
		}

		fs = get_fs();
		set_fs(KERNEL_DS);
		g_ULFile.m_pfUL = filp_open(UL_FILE, O_RDONLY, 0);

		if(IS_ERR(g_ULFile.m_pfUL)) {
			PCM_ERROR(LOG_TAG, "ULDataFile_SetRP: filp_open err(%d)!\r\n", (u32)g_ULFile.m_pfUL);
			set_fs(fs);
			return NORESOURCE;
		}

		t_cur_pos = vfs_llseek(g_ULFile.m_pfUL, (loff_t)g_ULFile.m_u4DataPos, SEEK_SET);
		u4size = vfs_read(g_ULFile.m_pfUL, (void *)(g_ULFile.m_rBuf.u4Buf1 + g_ULFile.m_u4WP),
				  g_prSpeechEnhance.m_prShareMem->u4FrameByte, &t_cur_pos);

		if(u4size < 0) {
			PCM_ERROR(LOG_TAG, "ULDataFile_SetRP: vfs_read err(%i)!\r\n", u4size);
			filp_close(g_ULFile.m_pfUL, NULL);
			set_fs(fs);
			return NORESOURCE;
		}

		g_ULFile.m_u4DataPos += g_prSpeechEnhance.m_prShareMem->u4FrameByte;
		filp_close(g_ULFile.m_pfUL, NULL);

		g_ULFile.m_pfUL2 = filp_open(UL2_FILE, O_RDONLY, 0);

		if(IS_ERR(g_ULFile.m_pfUL2)) {
			PCM_ERROR(LOG_TAG, "ULDataFile_SetRP: filp_open UL2 err(%d)!\r\n", (u32)g_ULFile.m_pfUL2);
			set_fs(fs);
			return NORESOURCE;
		}

		t_cur_pos = vfs_llseek(g_ULFile.m_pfUL2, (loff_t)g_ULFile.m_u4DataPos2, SEEK_SET);
		u4size = vfs_read(g_ULFile.m_pfUL2, (void *)(g_ULFile.m_rBuf.u4Buf2 + g_ULFile.m_u4WP),
				  g_prSpeechEnhance.m_prShareMem->u4FrameByte, &t_cur_pos);

		if(u4size < 0) {
			PCM_ERROR(LOG_TAG, "ULDataFile_SetRP: vfs_read UL2 err(%i)!\r\n", u4size);
			filp_close(g_ULFile.m_pfUL2, NULL);
			set_fs(fs);
			return NORESOURCE;
		}

		g_ULFile.m_u4DataPos2 += g_prSpeechEnhance.m_prShareMem->u4FrameByte;
		filp_close(g_ULFile.m_pfUL2, NULL);
		set_fs(fs);

		g_ULFile.m_u4WP = (g_ULFile.m_u4WP + g_prSpeechEnhance.m_prShareMem->u4FrameByte) %
				  g_ULFile.m_rBuf.u4ChBufSz;
	}

	return NOERR;
}




