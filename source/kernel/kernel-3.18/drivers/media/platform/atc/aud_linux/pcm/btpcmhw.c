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
#include <linux/err.h>
#include <linux/semaphore.h>

#include "btpcmhw.h"
#include "speechdev.h"
#include "aud_clock.h"
#include "aud_pcm_dbg.h"
#include "pcm_debug.h"
#define LOG_TAG "btpcmhw"
#define EMPTY_DATA_CYCLE  5
static void CopyUserDataToUL(void);
static void CopyDLDataToUser(void);

BtPCMHw g_rBtPcm;

static DLDataFile g_DLFile;
static u8 *_filedata;
static u32 _filesize;


static s32 PCMDev_Thread(void *data)
{
	u32 u4Ret = 0;

	PCM_DEBUG(LOG_TAG, "PCMDev_Thread: start!\r\n");
	while (1) {
		g_rBtPcm.PCMDev_Thread_wq_flag = 0;
		u4Ret = wait_event_interruptible_timeout(g_rBtPcm.PCMDev_Thread_wq,
			g_rBtPcm.PCMDev_Thread_wq_flag, 1000U);
		if (kthread_should_stop()) {
			break;
		}

		if (u4Ret > 0) {
			BtPCMHw_InterruptThread();
		}
	}
	PCM_DEBUG(LOG_TAG, "PCMDev_Thread: end!\r\n");

	return NOERR;
}

static void BtPCMHw_CB(u32 u4IntType)
{
	if (8000 == g_rBtPcm.m_u4SampleRate) {
		g_rBtPcm.PCMDev_Thread_wq_flag = 1;
		wake_up_interruptible(&g_rBtPcm.PCMDev_Thread_wq);
	} else {
		g_rBtPcm.m_u4IntNum++;
		if ((g_rBtPcm.m_u4SampleRate / 8000) == g_rBtPcm.m_u4IntNum) {
			g_rBtPcm.PCMDev_Thread_wq_flag = 1;
			wake_up_interruptible(&g_rBtPcm.PCMDev_Thread_wq);
			g_rBtPcm.m_u4IntNum = 0;
		}
	}
}

void BtPCMHw_InterruptThread(void)
{
	if (g_rBtPcm.m_u4State == STATE_STARTED) {
		if (!g_rBtPcm.m_u4StartHw)
		SpeechDev_EventInform((u32)EVT_PCM_OUT_INTR, (u32)0);
		else
		{
			//  Copy user buffer data to PCM Tx
			CopyUserDataToUL();
			//  Copy PCM Rx data to user buffer
			CopyDLDataToUser();
		}
	}
}

s32 BtPCMHw_Init(void)
{
	s32 i4Ret = NOERR;

	_filedata = NULL;
	_filesize = 0;

	g_rBtPcm.PCMDev_Thread_task = kthread_create(PCMDev_Thread, NULL, "PCMDev_Thread");
	if (IS_ERR(g_rBtPcm.PCMDev_Thread_task)) {
		PCM_ERROR(LOG_TAG, "BtPCMHw_Init: Create PCMDev_Thread_task ERR!\r\n");
		i4Ret = (s32)PTR_ERR(g_rBtPcm.PCMDev_Thread_task);
		g_rBtPcm.PCMDev_Thread_task = NULL;
		return i4Ret;
	}
	init_waitqueue_head(&g_rBtPcm.PCMDev_Thread_wq);
	g_rBtPcm.PCMDev_Thread_wq_flag = 0;
	set_user_nice(g_rBtPcm.PCMDev_Thread_task, PCM_TASK_THREAD_NICE_PRIORITY);
    wake_up_process(g_rBtPcm.PCMDev_Thread_task);

	g_rBtPcm.m_u4StartHw = 0;
	g_rBtPcm.m_u4State = STATE_UNINIT;
	g_rBtPcm.m_u4SampleRate = 8000;
	g_rBtPcm.m_u4IntNum = 0;
	g_rBtPcm.m_rPcmCfg.eHwMode = PCM_NORMAL_MODE;
	g_rBtPcm.m_rPcmCfg.eSyncMode = PCM_LONG_MODE;
	g_rBtPcm.m_rPcmCfg.eSyncCycle = PCM_CLK_CYCLE_32;
	g_rBtPcm.m_rPcmCfg.u4SampleRate = 8000;

	g_rBtPcm.m_rPcmCfg.rIntCfg.PFN_ISR_CB = BtPCMHw_CB;
	g_rBtPcm.m_rPcmCfg.rIntCfg.u4IntSz = SPEECH_FRAME_SAMPLES;
	g_rBtPcm.m_rPcmCfg.rIntCfg.u4IntBurstTime = SPEECH_FRAME_SAMPLES >> 2;

	g_rBtPcm.m_rPcmCfg.u4BufPhyAdr = 0;
	g_rBtPcm.m_rPcmCfg.u4RxBufSz = PCM_BUFFER_SIZE >> 1;
	g_rBtPcm.m_rPcmCfg.u4TxBufSz = PCM_BUFFER_SIZE >> 1;

	g_rBtPcm.m_prPcmHal = PcmHal_New();
	if (g_rBtPcm.m_prPcmHal) {
		g_rBtPcm.m_prPcmHal->rRxHwIf.Setup(g_rBtPcm.m_prPcmHal, &g_rBtPcm.m_rPcmCfg);
		g_rBtPcm.m_prPcmHal->rRxHwIf.GetBuf(g_rBtPcm.m_prPcmHal, &g_rBtPcm.m_rRxBuf);
		g_rBtPcm.m_prPcmHal->rTxHwIf.GetBuf(g_rBtPcm.m_prPcmHal, &g_rBtPcm.m_rTxBuf);
		PCM_DEBUG(LOG_TAG, "BtPCMHw_Init: RX Buf: 0x%x, 0x%x, 0x%x; TX Buf: 0x%x, 0x%x, 0x%x\r\n",
			(u32)g_rBtPcm.m_rRxBuf.u4PhySddr, (u32)g_rBtPcm.m_rRxBuf.u4VirSAdr,
			(u32)g_rBtPcm.m_rRxBuf.u4ChBufSz, (u32)g_rBtPcm.m_rTxBuf.u4PhySddr,
			(u32)g_rBtPcm.m_rTxBuf.u4VirSAdr, (u32)g_rBtPcm.m_rTxBuf.u4ChBufSz);
	} else {
		PCM_ERROR(LOG_TAG, "BtPCMHw_Init: Can not alloc PCM Hal! \r\n");
		i4Ret = NORESOURCE;
		return i4Ret;
	}
	g_rBtPcm.m_u4State = STATE_INITED;

	if (g_rBtPcm.m_fgDataFromFile) {
		DLDataFile_Init();
	}

	return i4Ret;
}

u32 BtPCMHw_UnInit(void)
{
	if (g_rBtPcm.m_prPcmHal) {
		g_rBtPcm.m_prPcmHal->Delete(g_rBtPcm.m_prPcmHal);
		g_rBtPcm.m_prPcmHal = NULL;
	}

	g_rBtPcm.PCMDev_Thread_wq_flag = 1;
	wake_up_interruptible(&g_rBtPcm.PCMDev_Thread_wq);
	if (g_rBtPcm.PCMDev_Thread_task) {
		kthread_stop(g_rBtPcm.PCMDev_Thread_task);
		g_rBtPcm.PCMDev_Thread_task = NULL;
		PCM_DEBUG(LOG_TAG, "BtPCMHw_UnInit: PCMDev_Thread stop!\r\n");
	}

	if ((STATE_INITED == g_rBtPcm.m_u4State) || (STATE_STOPPED == g_rBtPcm.m_u4State)) {
		g_rBtPcm.m_u4State = STATE_UNINIT;
	}

	if (g_rBtPcm.m_fgDataFromFile) {
		DLDataFile_UnInit();
	}

	g_rBtPcm.m_u4SampleRate = 8000;
	g_rBtPcm.m_u4IntNum = 0;

	return NOERR;
}

void BtPCMHw_HibernationCtrl(bool fgWakeUp)
{
	if (fgWakeUp) {
		if (g_rBtPcm.m_prPcmHal) {
			g_rBtPcm.m_prPcmHal->rRxHwIf.Setup(g_rBtPcm.m_prPcmHal, &g_rBtPcm.m_rPcmCfg);
		}
	} else {
		BtPCMHw_Stop();
	}
}

bool BtPCMHw_IsStart(void)
{
	return (STATE_UNINIT == g_rBtPcm.m_u4State);
}

s32 BtPCMHw_StartEx(u32 u4SampleRate, u32 u4Hw)
{
	printk("[PCM] BT Voice Call SampleRate(%d)\n", u4SampleRate);
	if (!g_rBtPcm.m_u4StartHw)
	{
		g_rBtPcm.m_u4StartHw = u4Hw;
		return BtPCMHw_Start(u4SampleRate);
	}
	else
	{
		g_rBtPcm.m_u4StartHw |= u4Hw;
		if (u4SampleRate != g_rBtPcm.m_u4SampleRate)
		{
			pr_warn("[PCM] BtPCMHw_StartEx u4SampleRate(%d) Hw(0x%x)\r\n", u4SampleRate, u4Hw);
		}
	}
	return NOERR;
}

s32 BtPCMHw_StopEx(u32 u4Hw)
{
	g_rBtPcm.m_u4StartHw &= ~u4Hw;
	if (!g_rBtPcm.m_u4StartHw)
	{
		return BtPCMHw_Stop();
	}
	return NOERR;
}


s32 BtPCMHw_Start(u32 u4SampleRate)
{
	s32 i4Ret = NOERR;

	if ((STATE_INITED == g_rBtPcm.m_u4State) || (STATE_STOPPED == g_rBtPcm.m_u4State)) {
		g_rBtPcm.m_u4DLRP = 0;
		if (g_rBtPcm.m_fgDataFromFile) {
			g_DLFile.m_u4WP = 0;
			g_DLFile.m_u4DataPos = 0;
			DLDataFile_Read();
		}

		/* for do fade in */
		g_rBtPcm.m_u4TxFillSize = 0;
		g_rBtPcm.m_i4TxFadeInVol = 0;

		g_rBtPcm.m_u4DLRP = 0;
		g_rBtPcm.m_u4ULWP = 0;

		g_rBtPcm.m_u4SampleRate = u4SampleRate;
		g_rBtPcm.m_rPcmCfg.u4SampleRate = u4SampleRate;
		g_rBtPcm.m_prPcmHal->PcmCfgUpd(g_rBtPcm.m_prPcmHal, &g_rBtPcm.m_rPcmCfg);
		g_rBtPcm.m_prPcmHal->PcmCtrl(g_rBtPcm.m_prPcmHal, true);
		g_rBtPcm.m_prPcmHal->rTxHwIf.Start(g_rBtPcm.m_prPcmHal, 0);
		g_rBtPcm.m_prPcmHal->rRxHwIf.Start(g_rBtPcm.m_prPcmHal, 0);

		g_rBtPcm.m_u4State = STATE_STARTED;
		BtPCMHw_GetDLBuffer(&g_rBtPcm.m_rRxBuf);
	} else if (STATE_UNINIT == g_rBtPcm.m_u4State) {
		i4Ret = INVALIDSTATE;
		PCM_ERROR(LOG_TAG, "BtPCMHw_Start: m_u4State error with UNINIT\r\n");
	} else {
		PCM_ERROR(LOG_TAG, "BtPCMHw_Start: m_u4State error with STARTED\r\n");
	}

	return i4Ret;
}

s32 BtPCMHw_Stop(void)
{
	s32 i4Ret = NOERR;

	if (STATE_STARTED == g_rBtPcm.m_u4State) {
		g_rBtPcm.m_prPcmHal->rTxHwIf.Stop(g_rBtPcm.m_prPcmHal, 0);
		g_rBtPcm.m_prPcmHal->rRxHwIf.Stop(g_rBtPcm.m_prPcmHal, 0);
		g_rBtPcm.m_prPcmHal->PcmCtrl(g_rBtPcm.m_prPcmHal, false);
		g_rBtPcm.m_u4State = STATE_STOPPED;
	} else {
		i4Ret = INVALIDSTATE;
		PCM_ERROR(LOG_TAG, "BtPCMHw_Stop: m_u4State(%d) error\r\n", g_rBtPcm.m_u4State);
	}

	return i4Ret;
}

bool BtPCMHw_IsLoopback(void)
{
	return (g_rBtPcm.m_rPcmCfg.eHwMode == PCM_LOOP_MODE);
}

bool BtPCMHw_EnableLoopback(bool fgEnable)
{
	PCM_DEBUG(LOG_TAG, "BtPCMHw_EnableLoopback: fgEnable(%d) \r\n", (u32)fgEnable);

	if (fgEnable) {
		g_rBtPcm.m_rPcmCfg.eHwMode = PCM_LOOP_MODE;
	} else {
		g_rBtPcm.m_rPcmCfg.eHwMode = PCM_NORMAL_MODE;
	}

	return true;
}

u32 BtPCMHw_GetDLBuffer(AUD_DATA_BUF_T *prBuffer)
{
	if (g_rBtPcm.m_fgDataFromFile) {
		DLDataFile_GetBuf(prBuffer);
	} else {
		g_rBtPcm.m_prPcmHal->rRxHwIf.GetBuf(g_rBtPcm.m_prPcmHal, prBuffer);
		prBuffer->u4DataOff = 0;
		prBuffer->u4DataSize = prBuffer->u4ChBufSz;
	}

	return NOERR;
}

u32 BtPCMHw_GetDLWP(void)
{
	u32 u4WP = 0;

	if (g_rBtPcm.m_fgDataFromFile) {
		u4WP = DLDataFile_GetWP();
	} else {
		u4WP = g_rBtPcm.m_prPcmHal->rRxHwIf.GetPoint(g_rBtPcm.m_prPcmHal);
	}

	return u4WP;
}

void BtPCMHw_UpdateDLRP(u32 u4RP)
{
	g_rBtPcm.m_u4DLRP = u4RP;
	if (g_rBtPcm.m_fgDataFromFile) {
		DLDataFile_SetRP(u4RP);
	}
}

u32 BtPCMHw_GetDLRP(void)
{
	return g_rBtPcm.m_u4DLRP;
}

u32 BtPCMHw_GetULFreeLen(void)
{
	u32 u4Len = 0;
	u32 u4ULRP = g_rBtPcm.m_prPcmHal->rTxHwIf.GetPoint(g_rBtPcm.m_prPcmHal);

	if (u4ULRP > g_rBtPcm.m_u4ULWP) {
		u4Len = u4ULRP - g_rBtPcm.m_u4ULWP;
	} else {
		u4Len = g_rBtPcm.m_rTxBuf.u4ChBufSz + u4ULRP - g_rBtPcm.m_u4ULWP;
	}

	if (u4Len > g_rBtPcm.m_rPcmCfg.rIntCfg.u4IntSz) {
		u4Len -= g_rBtPcm.m_rPcmCfg.rIntCfg.u4IntSz;
	} else {
		u4Len = 0;
	}

	u4Len &= 0xFFFFFFF0;

	return u4Len;
}

u32 BtPCMHw_FillULData(void *pvData, u32 u4DataSize)
{
	u32 u4Size = u4DataSize;
	u32 u4RollbackSize = 0;

	if (u4DataSize & 0xFU) {
		PCM_ERROR(LOG_TAG, "BtPCMHw_FillULData: Fill data size isn't 16 bytes alignment\r\n");
	}

	BtPCMHw_ULDataFadeIn((s16 *)pvData, u4DataSize);
	NdcVirtualMicIn_CopyData(pvData, u4DataSize);
	if (u4Size + g_rBtPcm.m_u4ULWP >= g_rBtPcm.m_rTxBuf.u4ChBufSz) {
		u4Size = g_rBtPcm.m_rTxBuf.u4ChBufSz - g_rBtPcm.m_u4ULWP;
		u4RollbackSize = u4DataSize - u4Size;
	}

	x_memcpy((void *)(g_rBtPcm.m_rTxBuf.u4VirSAdr + g_rBtPcm.m_u4ULWP), pvData, u4Size);

	if (u4RollbackSize) {
		pvData = (void *)((u32)pvData + u4Size);
		x_memcpy((void *)g_rBtPcm.m_rTxBuf.u4VirSAdr, pvData, u4RollbackSize);
	}
	g_rBtPcm.m_u4ULWP = (g_rBtPcm.m_u4ULWP + u4DataSize) % g_rBtPcm.m_rTxBuf.u4ChBufSz;
	g_rBtPcm.m_prPcmHal->rTxHwIf.SetPoint(g_rBtPcm.m_prPcmHal, g_rBtPcm.m_u4ULWP);

	return u4DataSize;
}

void BtPCMHw_ULDataFadeIn(s16 *pi2Data, u32 u4DataSize)
{
	u32 u4Size = u4DataSize;
	u32 u4SilenceSize = g_prSpeechEnhance.m_prShareMem->u4SampleRate * (u32)3;
	u32 u4FadeInSize = g_prSpeechEnhance.m_prShareMem->u4SampleRate;
	u32 u4FadeInStep = 0x10000U / u4FadeInSize;

	if (g_rBtPcm.m_u4TxFillSize < (u4SilenceSize + u4FadeInSize)) {
		while ((g_rBtPcm.m_u4TxFillSize < (u4SilenceSize + u4FadeInSize)) && (u4Size > 0)) {
			if (g_rBtPcm.m_u4TxFillSize <= u4SilenceSize) {
				*pi2Data = 0;
				pi2Data++;
			} else {
				*pi2Data = (s16)(((s32)(*pi2Data) * g_rBtPcm.m_i4TxFadeInVol) >> 16);
				pi2Data++;
				g_rBtPcm.m_i4TxFadeInVol += u4FadeInStep;
			}
			u4Size -= 2U;
			g_rBtPcm.m_u4TxFillSize += 2U;
		}
	}
	g_rBtPcm.m_u4TxFillSize += u4Size;
}

bool BtPCMHw_IsDataFromFile(void)
{
	return g_rBtPcm.m_fgDataFromFile;
}

void BtPCMHw_SetDataFromFile(bool fgDataFromFile)
{
	g_rBtPcm.m_fgDataFromFile = fgDataFromFile;
	PCM_DEBUG(LOG_TAG, "BtPCMHw_SetDataFromFile: fgDataFromFile(%d)\r\n",
		(u32)g_rBtPcm.m_fgDataFromFile);
}

bool BtPCMHw_EnableDataFromFile(bool fgEnable)
{
	bool fgRet = false;

	if (STATE_STARTED != g_rBtPcm.m_u4State) {
		if (g_rBtPcm.m_fgDataFromFile != fgEnable) {
			PCM_DEBUG(LOG_TAG, "BtPCMHw_EnableDataFromFile: fgEnable(%d) \r\n", (s32)fgEnable);
			if (fgEnable) {
				if (NOERR == DLDataFile_Init()) {
					g_rBtPcm.m_fgDataFromFile = true;
				}
			} else {
				DLDataFile_UnInit();
				g_rBtPcm.m_fgDataFromFile = false;
			}
		}
		fgRet = true;
	}

	return fgRet;
}

s32 DLDataFile_Load(void)
{
	loff_t t_cur_pos = 0;
	mm_segment_t fs;
	int32_t u4size = 0;

	fs = get_fs();
	set_fs(KERNEL_DS);
	g_DLFile.m_pfDL = filp_open(DL_FILE, O_RDONLY, 0);
	if (IS_ERR(g_DLFile.m_pfDL)) {
		PCM_ERROR(LOG_TAG, "DLDataFile_Load: filp_open err(%d)!\r\n", (u32)g_DLFile.m_pfDL);
		set_fs(fs);
		return NORESOURCE;
	}
	u4size = vfs_llseek(g_DLFile.m_pfDL, 0, SEEK_END) - sizeof(WaveHeader);
	_filedata = vmalloc(u4size);
	if (NULL == _filedata) {
		return NORESOURCE;
	}

	t_cur_pos = vfs_llseek(g_DLFile.m_pfDL, sizeof(WaveHeader), SEEK_SET);
	u4size = vfs_read(g_DLFile.m_pfDL, (void *)(_filedata), u4size, &t_cur_pos);
	if (u4size < 0) {
		PCM_ERROR(LOG_TAG, "DLDataFile_Load: vfs_read err(%i)!\r\n", u4size);
		filp_close(g_DLFile.m_pfDL, NULL);
		set_fs(fs);
		return NORESOURCE;
	}
	_filesize = (u32)u4size;

	filp_close(g_DLFile.m_pfDL, NULL);
	set_fs(fs);

	return NOERR;
}


s32 DLDataFile_Init(void)
{
	g_DLFile.m_u4WP = 0;
	g_DLFile.m_rBuf.u4VirSAdr = 0;
	g_DLFile.m_rBuf.u4ChBufSz = SPEECH_FRAME_BYTES * 2U * 20U;
	g_DLFile.m_rBuf.u4Chn = 1;
	g_DLFile.m_u4DataPos = 0;

	g_DLFile.m_rBuf.u4VirSAdr = (u32)kmalloc(g_DLFile.m_rBuf.u4ChBufSz * g_DLFile.m_rBuf.u4Chn, GFP_KERNEL);
	if (0 == g_DLFile.m_rBuf.u4VirSAdr) {
		PCM_ERROR(LOG_TAG, "DLDataFile_Init: kmalloc buffer error!\r\n");
		return NORESOURCE;
	}
	memset((void *)g_DLFile.m_rBuf.u4VirSAdr, 0, g_DLFile.m_rBuf.u4ChBufSz * g_DLFile.m_rBuf.u4Chn);
	DLDataFile_Load();

	return NOERR;
}

u32 DLDataFile_UnInit(void)
{
	g_DLFile.m_u4WP = 0;
	if (g_DLFile.m_rBuf.u4VirSAdr) {
		kfree((void *)g_DLFile.m_rBuf.u4VirSAdr);
		g_DLFile.m_rBuf.u4VirSAdr = 0;
	}
	if (_filedata) {
		vfree(_filedata);
		_filedata = NULL;
		_filesize = 0;
	}

	return NOERR;
}

s32 DLDataFile_Read(void)
{
	if (!g_DLFile.m_rBuf.u4VirSAdr) {
		PCM_ERROR(LOG_TAG, "DLDataFile_Read: u4VirSAdr is 0!\r\n");
		return NORESOURCE;
	}

	if ((NULL != _filedata) && (_filesize >= (g_prSpeechEnhance.m_prShareMem->u4FrameByte * (u32)10))) {
		if ((g_DLFile.m_u4DataPos + g_prSpeechEnhance.m_prShareMem->u4FrameByte * (u32)10) > _filesize) {
			g_DLFile.m_u4DataPos = 0;
		}
		x_memcpy((void *)(g_DLFile.m_rBuf.u4VirSAdr + g_DLFile.m_u4WP),
			_filedata + g_DLFile.m_u4DataPos, g_prSpeechEnhance.m_prShareMem->u4FrameByte * (u32)10);
	}

	g_DLFile.m_u4WP += g_prSpeechEnhance.m_prShareMem->u4FrameByte * 10;
	if (g_DLFile.m_u4WP >= g_DLFile.m_rBuf.u4ChBufSz) {
		g_DLFile.m_u4WP -= g_DLFile.m_rBuf.u4ChBufSz;
	}

	g_DLFile.m_u4DataPos += g_prSpeechEnhance.m_prShareMem->u4FrameByte * 10;

	return NOERR;
}

u32 DLDataFile_GetBuf(AUD_DATA_BUF_T *prBuf)
{
	prBuf->u4VirSAdr = g_DLFile.m_rBuf.u4VirSAdr;
	prBuf->u4ChBufSz = g_DLFile.m_rBuf.u4ChBufSz;
	prBuf->u4Chn = g_DLFile.m_rBuf.u4Chn;
	prBuf->u4DataOff = 0;
	prBuf->u4DataSize = 0;

	return NOERR;
}

u32 DLDataFile_SetRP(u32 u4RP)
{
	if (u4RP == g_DLFile.m_u4WP) {
		DLDataFile_Read();
	}

	return NOERR;
}

u32 DLDataFile_GetWP(void)
{
	return g_DLFile.m_u4WP;
}

extern int  buf2Capture(void *data, unsigned char * buffer, int ByteSize);
extern int  playback2Buf(void *data, unsigned char * buffer, int ByteSize);
extern int  getPlaybackAvail(void);

static void CopyUserDataToUL(void)
{
	u32 u4Size = 0;
	u32 u4RSize;
	u32 u4ULRP = g_rBtPcm.m_prPcmHal->rTxHwIf.GetPoint(g_rBtPcm.m_prPcmHal);
    
	u32 size = SPEECH_FRAME_BYTES * g_rBtPcm.m_u4SampleRate / 8000;
    u32 avail = getPlaybackAvail();
    
    if (avail > (SPEECH_FRAME_SAMPLES * EMPTY_DATA_CYCLE * g_rBtPcm.m_u4SampleRate / 8000))
        size = size + SPEECH_FRAME_BYTES;
    else if (size > avail)
            return;
    
	if((size + g_rBtPcm.m_u4ULWP) > g_rBtPcm.m_rTxBuf.u4ChBufSz)
	{
		int len = g_rBtPcm.m_rTxBuf.u4ChBufSz - g_rBtPcm.m_u4ULWP;
		int len2 = size - len;
		if (playback2Buf(NULL, (unsigned char *)(g_rBtPcm.m_rTxBuf.u4VirSAdr + g_rBtPcm.m_u4ULWP), len) < 0) {
			return;
		}
		if (playback2Buf(NULL, (unsigned char *)(g_rBtPcm.m_rTxBuf.u4VirSAdr), len2) < 0) {
			return;
		}
		g_rBtPcm.m_u4ULWP += size;
	}
	else
	{
		if (playback2Buf(NULL, (unsigned char *)(g_rBtPcm.m_rTxBuf.u4VirSAdr + g_rBtPcm.m_u4ULWP), size) < 0) {
			return;
		}
		g_rBtPcm.m_u4ULWP += size;	
	}

	if (g_rBtPcm.m_u4ULWP >= g_rBtPcm.m_rTxBuf.u4ChBufSz)
		g_rBtPcm.m_u4ULWP -= g_rBtPcm.m_rTxBuf.u4ChBufSz;

	g_rBtPcm.m_prPcmHal->rTxHwIf.SetPoint(g_rBtPcm.m_prPcmHal, g_rBtPcm.m_u4ULWP);
}


static void CopyDLDataToUser(void)
{
	u32 u4Size = 0;
	u32 u4RSize;
	u32 u4DLRP = g_rBtPcm.m_u4DLRP;
	u32 u4DLWP = BtPCMHw_GetDLWP();

	if (g_rBtPcm.m_u4DLRP < u4DLWP) {
		u4Size =  u4DLWP - g_rBtPcm.m_u4DLRP;
		u4RSize =  buf2Capture(NULL, (unsigned char *)(g_rBtPcm.m_rRxBuf.u4VirSAdr + g_rBtPcm.m_u4DLRP), u4Size);
		u4DLRP += u4RSize;
	} else {
		u4Size = g_rBtPcm.m_rTxBuf.u4ChBufSz - g_rBtPcm.m_u4DLRP;
		u4RSize =  buf2Capture(NULL, (unsigned char *)(g_rBtPcm.m_rRxBuf.u4VirSAdr + g_rBtPcm.m_u4DLRP), u4Size);
		u4DLRP += u4RSize;
		if (u4RSize == u4Size)
		{
			u4RSize =  buf2Capture(NULL, (unsigned char *)(g_rBtPcm.m_rRxBuf.u4VirSAdr), u4DLWP);
			u4DLRP = u4RSize;
		}
	}
	if (u4DLRP >= g_rBtPcm.m_rRxBuf.u4ChBufSz)
		u4DLRP -= g_rBtPcm.m_rRxBuf.u4ChBufSz;

	BtPCMHw_UpdateDLRP(u4DLRP);

}

