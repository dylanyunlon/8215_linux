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


#include "speechenhance.h"
#include "speechdev.h"
#include "bt_osl.h"
#include "speech_dual.h"
#include "winutil.h"
#include "aud_oal.h"

#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/mmc/host.h>
#include <linux/cpufreq.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/mmc/core.h>
#include <linux/mmc/mmc.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <mach/dma.h>
#include <linux/delay.h>
#include <asm/delay.h>

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#if CONFIG_DRV_AUD_AC83XX
#include <mach/dma.h>
#else
#include <linux/miscdevice.h>
#include "dma.h"
extern struct miscdevice pcm_aud_dev; 
#endif

static struct semaphore g_rSpeechDbgSema[DBG_FILE_NUM];

SpeechEnhance g_prSpeechEnhance;

static bool SpeechEnhance_SpeechWaitAECState(u32 u4State);
static void SpeechEnhance_InitParamFromRegistry(void);
static bool SpeechEnhance_SpeechWaitCompleted(u32 u4Event, u32 u4TimeOut);

#define STATE_SCO	STATE_STARTED

#define DEBUG_SDCARD_PRE  "/data"

#define USED_FRAME ((g_prSpeechEnhance.m_prShareMem->u4WriteIdx  >= \
	g_prSpeechEnhance.m_u4LastReadIdx) ? \
	(g_prSpeechEnhance.m_prShareMem->u4WriteIdx - g_prSpeechEnhance.m_u4LastReadIdx) : \
	((g_prSpeechEnhance.m_prShareMem->u4WriteIdx + (g_prSpeechEnhance.m_prShareMem->u4MaxFrame << 1)) - \
	g_prSpeechEnhance.m_u4LastReadIdx))
#define INCREASE_WRITE_IDX (g_prSpeechEnhance.m_prShareMem->u4WriteIdx = \
	(g_prSpeechEnhance.m_prShareMem->u4WriteIdx + 1) % \
	(g_prSpeechEnhance.m_prShareMem->u4MaxFrame << 1))
#if !ARM1_SPEECH_INTERACT_WITH_ARM2
#define INCREASE_READ_IDX  (g_prSpeechEnhance.m_prShareMem->u4ReadIdx = \
	(g_prSpeechEnhance.m_prShareMem->u4ReadIdx + 1) % \
	(g_prSpeechEnhance.m_prShareMem->u4MaxFrame << 1))
#endif
#define IS_FREE_FRAME (USED_FRAME < (g_prSpeechEnhance.m_prShareMem->u4MaxFrame-1))

static u32 _u4PTime;
static u32 _u4DLCount;
static u32 _u4DLTotal;
static u32 _u4DLMax;
static u32 _u4ULCount;
static u32 _u4ULTotal;
static u32 _u4ULMax;

static u32 WaitAECStateTimeOutCnt;

static SPH_ENH_08K_ctrl_struct Sph_Enh_ctrl1;
static SPH_ENH_08K_ctrl_struct Sph_Enh_ctrl2;
static Word16 ABF_cal_data[DMNR_PARAM_NUM] = {0};
static Word16 aec_com_rx[AEC_COM_RX] = {0};
static Word16 aec_com_tx[AEC_COM_TX] = {0};
static uWord32 Sph_Enh_ctrl_16k[AEC_NDC_PARAM_NUM] = {0};
static uWord32 Sph_Enh_ctrl_comm_16k[COMMOM_PARAM_NUM] = {0};

static Word16 DMNR_cal_data_16k[DMNR_PARAM_NUM_16K] = {0};
static Word16 Compen_filter_16k[COMPEN_FILTER_16K] = {0};


static s32 SpeechEnhance_InitDebugWaveFiles(void);
static void SpeechEnhance_UnInitDebugWaveFiles(void);

#define PRINT_INTERVAL 5000U

static u32 MsgHandler(u32 u4MsgID, u32 u4Param1, u32 u4Param2, u32 u4Param3)
{
	return SpeechEnhance_SpeechHandleAECMsg(u4MsgID, u4Param1, u4Param2, u4Param3);
}

s32 SpeechEnhance_EnableSCO(bool fgEnable)
{
	u32 u4Param = 0;

	if (STATE_UNINIT == g_prSpeechEnhance.m_u4State) {
		return INVALIDPRAM;
	}

	if (((STATE_SCO == g_prSpeechEnhance.m_u4State) && fgEnable) ||
	    ((STATE_INITED == g_prSpeechEnhance.m_u4State) && (!fgEnable))) {
		return NOERR;
	}

	if (fgEnable) {
		g_prSpeechEnhance.m_prShareMem->u4Version = 0x00000001;
		g_prSpeechEnhance.m_prShareMem->u4WriteIdx = 0;
		g_prSpeechEnhance.m_prShareMem->u4ReadIdx = 0;
		g_prSpeechEnhance.m_u4LastReadIdx = 0;
		_u4PTime = 0;
		_u4DLCount = 0;
		_u4DLTotal = 0;
		_u4DLMax = 0;
		_u4ULCount = 0;
		_u4ULMax = 0;
		_u4ULTotal = 0;

		if (g_prSpeechEnhance.m_fgDbgEnable) {
			SpeechEnhance_InitDebugWaveFiles();
		}
	}

	u4Param = ((u32)FRAME_OPT_AEC) | ((u32)FRAME_OPT_NDC);
	if (g_prSpeechEnhance.m_rSphOpt.fgDMNR) {
		u4Param |= FRAME_OPT_DMNR;
	}

	if (g_prSpeechEnhance.m_rSphOpt.fgPLC) {
		u4Param |= FRAME_OPT_PLC;
	}

	if (g_prSpeechEnhance.m_rSphOpt.u4Log) {
		u4Param |= ((g_prSpeechEnhance.m_rSphOpt.u4Log << 8U) & ((u32)OPT_OUTPUT_LOG));
	}

#if 0
#if ARM1_SPEECH_INTERACT_WITH_ARM2
	SpeechSendMessage((u32)BT_SCO_AUDIO_CONTROL, (u32)fgEnable, u4Param, (u32)0);
#endif
	if (fgEnable) {
#if ARM1_SPEECH_INTERACT_WITH_ARM2
		SpeechEnhance_SpeechWaitAECState((u32)BT_STATE_SCO);
#endif
		g_prSpeechEnhance.m_u4State = STATE_SCO;
	} else {
#if ARM1_SPEECH_INTERACT_WITH_ARM2
		SpeechEnhance_SpeechWaitAECState((u32)BT_STATE_IDLE);
#endif
		g_prSpeechEnhance.m_u4State = STATE_INITED;

		if (g_prSpeechEnhance.m_fgDbgEnable) {
			SpeechEnhance_UnInitDebugWaveFiles();
		}
	}
#endif

	return NOERR;
}

bool SpeechEnhance_IsSCOEnable(void)
{
	return (STATE_SCO == g_prSpeechEnhance.m_u4State);
}

s32 SpeechEnhance_Init(void)
{
	BT_SHARE_MEM_EX_T *prShareMem;

	pr_debug("[PCM]SpeechEnhance_Init: Enter\r\n");
	g_prSpeechEnhance.m_prShareMem = NULL;
	g_prSpeechEnhance.m_u4PhyAddr = 0;
	g_prSpeechEnhance.m_u4HwSemaphore = 0;
	g_prSpeechEnhance.m_hCompleted = NULL;
	g_prSpeechEnhance.m_u4AECEvent = 0;
	g_prSpeechEnhance.m_u4AECState = BT_STATE_UNINIT;
	g_prSpeechEnhance.m_u4LastReadIdx = 0;
	g_prSpeechEnhance.m_u4State = STATE_UNINIT;
	g_prSpeechEnhance.m_u4ActiveIdx = SPEECH_FRAME_COUNT;
	g_prSpeechEnhance.m_rSphOpt.u4Log = 0;
	g_prSpeechEnhance.m_u4DebugIdx = 0;

	if (STATE_UNINIT != g_prSpeechEnhance.m_u4State) {
		return INVALIDPRAM;
	}

	g_prSpeechEnhance.m_hCompleted = x_event_create(NULL, false, false, TEXT("SpeechenhanceCompleted"));

	if (!g_prSpeechEnhance.m_hCompleted) {
		pr_err("[PCM ERR]SpeechEnhance_Init: Create m_hCompleted Event error\r\n");
		return NORESOURCE;
	}
#if CONFIG_DRV_AUD_AC83XX
	prShareMem = (BT_SHARE_MEM_EX_T *)dma_alloc_coherent(NULL, sizeof(BT_SHARE_MEM_EX_T),
		(dma_addr_t *)&g_prSpeechEnhance.m_u4PhyAddr, GFP_KERNEL);
#else
	if(pcm_aud_dev.this_device == NULL)
	{
	    pr_err("[PCM ERR]pcm_aud_dev->this_device == NULL\r\n");
		return NORESOURCE;
	}
	prShareMem = (BT_SHARE_MEM_EX_T *)dma_alloc_coherent(pcm_aud_dev.this_device, sizeof(BT_SHARE_MEM_EX_T),
		(dma_addr_t *)&g_prSpeechEnhance.m_u4PhyAddr, GFP_KERNEL);
#endif
	if (!prShareMem) {
		pr_err("[PCM ERR]SpeechEnhance_Init: malloc prShareMem err.\r\n");
		return NORESOURCE;
	}

	memset(prShareMem, 0, sizeof(BT_SHARE_MEM_EX_T));
	g_prSpeechEnhance.m_prShareMem = prShareMem;
	prShareMem->u4MaxFrame = SPEECH_FRAME_COUNT;

#if CONFIG_DRV_AUD_AC83XX
	g_prSpeechEnhance.m_u4HwSemaphore = HSMPHE_SPEECH;
#else
        g_prSpeechEnhance.m_u4HwSemaphore = (1U << 3);

#endif

	pr_debug("[PCM]SpeechEnhance_Init: rFrame(%x) u4MaxFrame(%d)\r\n",
		(uintptr_t)prShareMem->rFrame, (s32)prShareMem->u4MaxFrame);

	//SpeechEnhance_SetARM2SpeechLog((u32)8);
	//SpeechEnhance_InitParamFromRegistry();
	//SpeechRegCallback(MsgHandler);
	//msleep(100);
#if ARM1_SPEECH_INTERACT_WITH_ARM2
	//SpeechSendMessage((uintptr_t)BT_SET_HW_RESOURCE, (uintptr_t)SHARE_MEM((uintptr_t)prShareMem,
		//g_prSpeechEnhance.m_u4PhyAddr), sizeof(BT_SHARE_MEM_EX_T), g_prSpeechEnhance.m_u4HwSemaphore);
	//pr_debug("[PCM]SpeechEnhance_Init: Wait for ARM2 HW Resource Start\r\n");
	//SpeechEnhance_SpeechWaitCompleted((u32)BT_SET_HW_RESOURCE, (u32)INFINITE);
	//pr_debug("[PCM]SpeechEnhance_Init: Wait for ARM2 HW Resource Finish\r\n");
#endif
	g_prSpeechEnhance.m_u4State = STATE_INITED;

	return NOERR;
}

s32 SpeechEnhance_UnInit(void)
{
	if (STATE_SCO == g_prSpeechEnhance.m_u4State) {
		SpeechEnhance_EnableSCO(false);
	}

	if (STATE_INITED != g_prSpeechEnhance.m_u4State) {
		return INVALIDPRAM;
	}
	if (g_prSpeechEnhance.m_prShareMem) {
		dma_free_coherent(NULL, sizeof(BT_SHARE_MEM_EX_T),
			g_prSpeechEnhance.m_prShareMem, (dma_addr_t)g_prSpeechEnhance.m_u4PhyAddr);
		g_prSpeechEnhance.m_prShareMem = NULL;
		g_prSpeechEnhance.m_u4PhyAddr = 0;
	}
	if (g_prSpeechEnhance.m_hCompleted) {

		x_event_destroy(g_prSpeechEnhance.m_hCompleted);

		g_prSpeechEnhance.m_hCompleted = NULL;
	}

	return NOERR;
}

void SpeechEnhance_HibernationCtrl(bool fgWakeUp)
{
	if (fgWakeUp)
        #if CONFIG_DRV_AUD_AC83XX
	    SpeechSendMessage((u32)BT_SET_HW_RESOURCE, (u32)SHARE_MEM((u32)g_prSpeechEnhance.m_prShareMem,
	    g_prSpeechEnhance.m_u4PhyAddr), sizeof(BT_SHARE_MEM_EX_T),(u32)HSMPHE_SPEECH);
        #else
	    SpeechSendMessage((u32)BT_SET_HW_RESOURCE, (u32)SHARE_MEM((u32)g_prSpeechEnhance.m_prShareMem,
            g_prSpeechEnhance.m_u4PhyAddr), sizeof(BT_SHARE_MEM_EX_T), (u32)(1U << 3));
        #endif
}

static void SpeechEnhance_InitParamFromRegistry(void)
{
	u32 u4Idx = 0;

	memcpy(&g_prSpeechEnhance.m_prShareMem->rSphParam, &Sph_Enh_ctrl1, sizeof(SPH_ENH_08K_ctrl_struct));
	/*pr_debug("[PCM]SpeechEnhance_InitParamFromRegistry: AEC Param = ");
	for (u4Idx = 0; u4Idx < AEC_NDC_PARAM_NUM; u4Idx++) {
		if (0 == (u4Idx % 10U)) {
			pr_debug("\r\n");
		}
		pr_debug("%d, ", (s32)g_prSpeechEnhance.m_prShareMem->rSphParam.enhance_pars[u4Idx]);
	}
	pr_debug("\r\n");*/

	memcpy(&g_prSpeechEnhance.m_prShareMem->rAecRxParam, aec_com_rx, sizeof(AEC_COM_RX_struct));
	/*pr_debug("[PCM]SpeechEnhance_InitParamFromRegistry: aec_com_rx Param = ");
	for (u4Idx = 0; u4Idx < AEC_COM_RX; u4Idx++) {
		if (0 == (u4Idx % 10U)) {
			pr_debug("\r\n");
		}
		pr_debug("%d, ", g_prSpeechEnhance.m_prShareMem->rAecRxParam.enhance_pars[u4Idx]);
	}
	pr_debug("\r\n");*/

	memcpy(&g_prSpeechEnhance.m_prShareMem->rAecTxParam, aec_com_tx, sizeof(AEC_COM_RX_struct));
	/*pr_debug("[PCM]SpeechEnhance_InitParamFromRegistry: aec_com_tx Param = ");
	for (u4Idx = 0; u4Idx < AEC_COM_TX; u4Idx++) {
		if (0 == (u4Idx % 10U)) {
			pr_debug("\r\n");
		}
		pr_debug("%d, ", g_prSpeechEnhance.m_prShareMem->rAecTxParam.enhance_pars[u4Idx]);
	}
	pr_debug("\r\n");*/

	memcpy(&g_prSpeechEnhance.m_prShareMem->rSphParam2, &Sph_Enh_ctrl2, sizeof(SPH_ENH_08K_ctrl_struct));
	/*pr_debug("[PCM]SpeechEnhance_InitParamFromRegistry: AEC Param2 =");
	for (u4Idx = 0; u4Idx < AEC_NDC_PARAM_NUM; u4Idx++) {
		if (0 == (u4Idx % 10U)) {
			pr_debug("\r\n");
		}
		pr_debug("%d, ", (s32)g_prSpeechEnhance.m_prShareMem->rSphParam2.enhance_pars[u4Idx]);
	}
	pr_debug("\r\n");*/

	memcpy(&g_prSpeechEnhance.m_prShareMem->rDmnrParam, ABF_cal_data, sizeof(DMNR_PARAM_T));
	/*pr_debug("[PCM]SpeechEnhance_InitParamFromRegistry: DMNR Param =");
	for (u4Idx = 0; u4Idx < DMNR_PARAM_NUM; u4Idx++) {
		if (0 == (u4Idx % 10U)) {
			pr_debug("\r\n");
		}
		pr_debug("%d, ", (s32)g_prSpeechEnhance.m_prShareMem->rDmnrParam.dmnrParm[u4Idx]);
	}
	pr_debug("\r\n");*/

	memcpy(g_prSpeechEnhance.m_prShareMem->Sph_Enh_ctrl_16k, Sph_Enh_ctrl_16k, sizeof(Sph_Enh_ctrl_16k));
	/*pr_debug("[PCM]SpeechEnhance_InitParamFromRegistry: 16k AEC Param =");
	for (u4Idx = 0; u4Idx < AEC_NDC_PARAM_NUM; u4Idx++) {
		if (0 == (u4Idx % 10U)) {
			pr_debug("\r\n");
		}
		pr_debug("%d, ", (s32)g_prSpeechEnhance.m_prShareMem->Sph_Enh_ctrl_16k[u4Idx]);
	}
	pr_debug("\r\n");*/

	memcpy(g_prSpeechEnhance.m_prShareMem->Sph_Enh_ctrl_comm_16k, Sph_Enh_ctrl_comm_16k, sizeof(Sph_Enh_ctrl_comm_16k));
	/*pr_debug("[PCM]SpeechEnhance_InitParamFromRegistry: 16k AEC Param =");
	for (u4Idx = 0; u4Idx < COMMOM_PARAM_NUM; u4Idx++) {
		if (0 == (u4Idx % 10U)) {
			pr_debug("\r\n");
		}
		pr_debug("%d, ", (s32)g_prSpeechEnhance.m_prShareMem->Sph_Enh_ctrl_comm_16k[u4Idx]);
	}
	pr_debug("\r\n");*/

	memcpy(g_prSpeechEnhance.m_prShareMem->DMNR_cal_data_16k, DMNR_cal_data_16k, sizeof(DMNR_cal_data_16k));
	/*pr_debug("[PCM]SpeechEnhance_InitParamFromRegistry: 16k DMNR_cal_data Param =");
	for (u4Idx = 0; u4Idx < DMNR_PARAM_NUM_16K; u4Idx++) {
		if (0 == (u4Idx % 10U)) {
			pr_debug("\r\n");
		}
		pr_debug("%d, ", (s32)g_prSpeechEnhance.m_prShareMem->DMNR_cal_data_16k[u4Idx]);
	}
	pr_debug("\r\n");*/

	memcpy(g_prSpeechEnhance.m_prShareMem->Compen_filter_16k, Compen_filter_16k, sizeof(Compen_filter_16k));
	/*pr_debug("[PCM]SpeechEnhance_InitParamFromRegistry: 16k COMPEN_FILTER Param =");
	for (u4Idx = 0; u4Idx < COMPEN_FILTER_16K; u4Idx++) {
		if (0 == (u4Idx % 10U)) {
			pr_debug("\r\n");
		}
		pr_debug("%d, ", (s32)g_prSpeechEnhance.m_prShareMem->Compen_filter_16k[u4Idx]);
	}
	pr_debug("\r\n");*/
}

u32 SpeechEnhance_SpeechHandleAECMsg(u32 u4MsgID, u32 u4Param1, u32 u4Param2, u32 u4Param3)
{
	/*switch(u4MsgID & 0xFFFF)
	{
	case BT_MSG_COMPLETED:
		g_prSpeechEnhance.m_u4AECEvent = u4Param1 & 0xFFFF;
		SetEvent(g_prSpeechEnhance.m_hCompleted);
		break;

	case BT_STATE_CHANGED:
		g_prSpeechEnhance.m_u4AECState = u4Param1;
		break;

	case BT_FRAME_COMPLETED:
		#ifndef  SPEECH_USE_ARM11_AEC
		task_param = u4Param1;
		tasklet_schedule(&ARM2_AEC_tasklet);
		#else
		SpeechEnhance_PostProcess(u4Param1);
		#endif
		break;

	default:
		break;
	} */

	return BT_SUCCESS;
}

static bool SpeechEnhance_SpeechWaitAECState(u32 u4State)
{
	pr_debug("[PCM]SpeechEnhance_SpeechWaitAECState: u4State(%x)\r\n", (u32)u4State);

	WaitAECStateTimeOutCnt = 0;
	while (u4State != g_prSpeechEnhance.m_u4AECState) {
		WaitAECStateTimeOutCnt++;
		if (WaitAECStateTimeOutCnt >= 500U) {
			pr_err("[PCM ERR]SpeechWaitAECState: Wait ARM2 state timeout error!\r\n");
			break;
		}
		pr_debug("[PCM]SpeechEnhance_SpeechWaitAECState: m_u4AECState(%x)\r\n",
			(u32)g_prSpeechEnhance.m_u4AECState);
		msleep(20);
	}

	return true;
}

static bool SpeechEnhance_SpeechWaitCompleted(u32 u4Event, u32 u4TimeOut)
{
	u32 u4StartTick = (u32)1000 * (u32)jiffies / (u32)HZ;

	pr_debug("[PCM]SpeechEnhance_SpeechWaitCompleted: u4Event(%x)\r\n", (u32)u4Event);
	do {

		u32 code = x_event_wait_for_objects(1, &g_prSpeechEnhance.m_hCompleted, FALSE, u4TimeOut);

		pr_debug("[PCM]SpeechEnhance_SpeechWaitCompleted: return(%x)\r\n",
			(u32)code);
		if (WAIT_TIMEOUT == code) {
			return false;
		}

		if ((code == WAIT_OBJECT_0) && (u4Event == g_prSpeechEnhance.m_u4AECEvent)) {
			return true;
		}

		u4StartTick = (u32)1000 * (u32)jiffies / (u32)HZ - u4StartTick;
		if (u4StartTick >= u4TimeOut) {
			return false;
		}

		if (u4TimeOut != INFINITE) {
			u4TimeOut -= u4StartTick;
		}
	} while (u4TimeOut);

	return false;
}

bool SpeechEnhance_PostProcess(u32 u4Index)
{
	u32 u4RealIdx;
	SPEECH_FRAME_T *prFrame;
	u32 u4CurTime = (u32)1000 * (u32)jiffies / (u32)HZ;

	if (u4Index != g_prSpeechEnhance.m_u4LastReadIdx) {
		pr_err("[PCM ERR]SpeechEnhance_PostProcess: u4Index(%d)!=LastReadIdx(%d)\r\n",
			(s32)u4Index, (s32)g_prSpeechEnhance.m_u4LastReadIdx);
		g_prSpeechEnhance.m_u4LastReadIdx = u4Index;
	}

	u4RealIdx = u4Index % g_prSpeechEnhance.m_prShareMem->u4MaxFrame;

	prFrame = g_prSpeechEnhance.m_prShareMem->rFrame + u4RealIdx;
	g_prSpeechEnhance.m_u4LastReadIdx++;
	g_prSpeechEnhance.m_u4LastReadIdx %= g_prSpeechEnhance.m_prShareMem->u4MaxFrame << 1U;
	prFrame->u4Param1 = u4CurTime - prFrame->u4Param1;
	if (prFrame->u4Opt & FRAME_OPT_DL) {
		if (prFrame->u4Param1 >= _u4DLMax) {
			_u4DLMax = prFrame->u4Param1;
		}

		_u4DLCount++;
		_u4DLTotal += prFrame->u4Param1;
	} else {
		if (prFrame->u4Param1 >= _u4ULMax) {
			_u4ULMax = prFrame->u4Param1;
		}

		_u4ULCount++;
		_u4ULTotal += prFrame->u4Param1;
	}
#if 0
	if ((u4CurTime - _u4PTime) > PRINT_INTERVAL) {
		pr_debug("[PCM]SpeechEnhance_PostProcess: (%06d ms)\r\n", (s32)u4CurTime);
		pr_debug("DL: Count(%d) Max(%dms) Avg(%dms). UL Count(%d) Max(%dms) Avg(%dms)\r\n",
			(s32)_u4DLCount, (s32)_u4DLMax, (s32)(_u4DLCount ? _u4DLTotal / _u4DLCount : 0),
			(s32)_u4ULCount, (s32)_u4ULMax, (s32)(_u4ULCount ? _u4ULTotal / _u4ULCount : 0));
		_u4PTime = u4CurTime;
	}
#endif
	if (prFrame->u4Opt & FRAME_OPT_DL) {
		u32 u4Size = 0;

		if (g_prSpeechEnhance.m_fgDbgEnable) {
			u4Size = (u32)MAX_DBG_FILE_BUFFER -
				(u32)g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].pull_len;
			if (u4Size < g_prSpeechEnhance.m_prShareMem->u4FrameByte) {
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].pwave_buf +
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].pull_len),
					(void *)prFrame->DLBuf, u4Size);
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].pwave_buf),
					(void *)(&prFrame->DLBuf[u4Size >> 1]),
					(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size));
				g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].pull_len =
					(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size);
			} else {
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].pwave_buf +
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].pull_len),
					(void *)prFrame->DLBuf, g_prSpeechEnhance.m_prShareMem->u4FrameByte);
				g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].pull_len +=
					g_prSpeechEnhance.m_prShareMem->u4FrameByte;
			}
			g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].Thread_wq_flag = 1;
			wake_up_interruptible(&g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].Thread_wq);

			u4Size = (u32)MAX_DBG_FILE_BUFFER -
				(u32)g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].pull_len;
			if (u4Size < g_prSpeechEnhance.m_prShareMem->u4FrameByte) {
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].pwave_buf +
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].pull_len),
					(void *)prFrame->DLDelayBuf, u4Size);
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].pwave_buf),
					(void *)(&prFrame->DLDelayBuf[u4Size >> 1]),
					(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size));
				g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].pull_len =
					(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size);
			} else {
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].pwave_buf +
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].pull_len),
					(void *)prFrame->DLDelayBuf, g_prSpeechEnhance.m_prShareMem->u4FrameByte);
				g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].pull_len +=
					g_prSpeechEnhance.m_prShareMem->u4FrameByte;
			}
			g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].Thread_wq_flag = 1;
			wake_up_interruptible(&g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].Thread_wq);
		}

		SpeechDev_EventInform((u32)EVT_DL_FRAME_FINISH, (uintptr_t)prFrame);
	} else {
		u32 u4Size = 0;

#if (DTMF_INFO_SENDER)
		if (DtmfInfoSender_GetSenderEn()) {
			DtmfInfoSender_ProcessData(prFrame->ULBuf1);
		}
#endif

		if (16000 == g_prSpeechEnhance.m_prShareMem->u4SampleRate) {
			if (g_prSpeechEnhance.m_fgDbgEnable) {
				u4Size = (u32)MAX_DBG_FILE_BUFFER -
					(u32)g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].pull_len;
				if (u4Size < g_prSpeechEnhance.m_prShareMem->u4FrameByte) {
					memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].pwave_buf +
						g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].pull_len),
						(void *)prFrame->DLBuf, u4Size);
					memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].pwave_buf),
						(void *)(&prFrame->DLBuf[u4Size >> 1]),
						(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size));
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].pull_len =
						(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size);
				} else {
					memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].pwave_buf +
						g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].pull_len),
						(void *)prFrame->DLBuf, g_prSpeechEnhance.m_prShareMem->u4FrameByte);
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].pull_len +=
						g_prSpeechEnhance.m_prShareMem->u4FrameByte;
				}
				g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].Thread_wq_flag = 1;
				wake_up_interruptible(&g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].Thread_wq);

				u4Size = (u32)MAX_DBG_FILE_BUFFER -
					(u32)g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].pull_len;
				if (u4Size < g_prSpeechEnhance.m_prShareMem->u4FrameByte) {
					memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].pwave_buf +
						g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].pull_len),
						(void *)prFrame->DLDelayBuf, u4Size);
					memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].pwave_buf),
						(void *)(&prFrame->DLDelayBuf[u4Size >> 1]),
						(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size));
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].pull_len =
						(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size);
				} else {
					memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].pwave_buf +
						g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].pull_len),
						(void *)prFrame->DLDelayBuf,
						g_prSpeechEnhance.m_prShareMem->u4FrameByte);
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].pull_len +=
						g_prSpeechEnhance.m_prShareMem->u4FrameByte;
				}
				g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].Thread_wq_flag = 1;
				wake_up_interruptible(&g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].Thread_wq);
			}
			SpeechDev_EventInform((u32)EVT_DL_FRAME_FINISH, (uintptr_t)prFrame);
		}

		SpeechDev_EventInform((u32)EVT_UL_FRAME_FINISH, (uintptr_t)prFrame);
		if (g_prSpeechEnhance.m_fgDbgEnable) {
			u4Size = (u32)MAX_DBG_FILE_BUFFER -
				(u32)g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_POST].pull_len;
			if (u4Size < g_prSpeechEnhance.m_prShareMem->u4FrameByte) {
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_POST].pwave_buf +
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_POST].pull_len),
					(void *)prFrame->ULBuf1, u4Size);
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_POST].pwave_buf),
					(void *)(&prFrame->ULBuf1[u4Size >> 1]),
					(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size));
				g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_POST].pull_len =
					(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size);
			} else {
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_POST].pwave_buf +
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_POST].pull_len),
					(void *)prFrame->ULBuf1, g_prSpeechEnhance.m_prShareMem->u4FrameByte);
				g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_POST].pull_len +=
					g_prSpeechEnhance.m_prShareMem->u4FrameByte;
			}
			g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_POST].Thread_wq_flag = 1;
			wake_up_interruptible(&g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_POST].Thread_wq);

			u4Size = (u32)MAX_DBG_FILE_BUFFER -
				(u32)g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_AEC_POST].pull_len;
			if (u4Size < g_prSpeechEnhance.m_prShareMem->u4FrameByte) {
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_AEC_POST].pwave_buf +
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_AEC_POST].pull_len),
					(void *)prFrame->ULBuf2, u4Size);
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_AEC_POST].pwave_buf),
					(void *)(&prFrame->ULBuf2[u4Size >> 1]),
					(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size));
				g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_AEC_POST].pull_len =
					(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size);
			} else {
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_AEC_POST].pwave_buf +
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_AEC_POST].pull_len),
					(void *)prFrame->ULBuf2, g_prSpeechEnhance.m_prShareMem->u4FrameByte);
				g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_AEC_POST].pull_len +=
					g_prSpeechEnhance.m_prShareMem->u4FrameByte;
			}

			g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_AEC_POST].Thread_wq_flag = 1;
			wake_up_interruptible(&g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_AEC_POST].Thread_wq);
		}
	}

	return true;
}

SPEECH_FRAME_T  *SpeechEnhance_GetFreeFrame(void)
{
	if (!IS_FREE_FRAME) {
		pr_err("[PCM ERR]SpeechEnhance_GetFreeFrame: No Free Frame. ");
		pr_err("L-Idx(%d) R-Idx(%d) W-Idx(%d)\r\n",
			(s32)g_prSpeechEnhance.m_u4LastReadIdx, (s32)g_prSpeechEnhance.m_prShareMem->u4ReadIdx,
			(s32)g_prSpeechEnhance.m_prShareMem->u4WriteIdx);
		return NULL;
	}

	TAKE_BT_HW_SEMAPHORE();
	g_prSpeechEnhance.m_u4ActiveIdx = g_prSpeechEnhance.m_prShareMem->u4WriteIdx %
		g_prSpeechEnhance.m_prShareMem->u4MaxFrame;
	g_prSpeechEnhance.m_prActiveFrame = g_prSpeechEnhance.m_prShareMem->rFrame +
		g_prSpeechEnhance.m_u4ActiveIdx;
	RELEASE_BT_HW_SEMAPHORE();
	memset(g_prSpeechEnhance.m_prActiveFrame, 0, sizeof(SPEECH_FRAME_T));

	return g_prSpeechEnhance.m_prActiveFrame;
}

s32 SpeechEnhance_EnhanceDL(SPEECH_FRAME_T *prFrame)
{
	if (g_prSpeechEnhance.m_prActiveFrame == prFrame) {
		prFrame->u4Opt = FRAME_OPT_DL | FRAME_OPT_AEC | FRAME_OPT_NDC;
		if (g_prSpeechEnhance.m_rSphOpt.fgPLC) {
			prFrame->u4Opt |= FRAME_OPT_PLC;
		}

		if (g_prSpeechEnhance.m_fgDbgEnable) {
			u32 u4Size = 0;

			prFrame->u4Opt |= DATA_REQ_POST_NDC;
			u4Size = (u32)MAX_DBG_FILE_BUFFER -
				(u32)g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].pull_len;
			if (u4Size < g_prSpeechEnhance.m_prShareMem->u4FrameByte) {
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].pwave_buf +
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].pull_len),
					(void *)prFrame->DLBuf, u4Size);
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].pwave_buf),
					(void *)(&prFrame->DLBuf[u4Size >> 1]),
					(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size));
				g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].pull_len =
					(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size);
			} else {
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].pwave_buf +
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].pull_len),
					(void *)prFrame->DLBuf, g_prSpeechEnhance.m_prShareMem->u4FrameByte);
				g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].pull_len +=
					g_prSpeechEnhance.m_prShareMem->u4FrameByte;
			}
			g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].Thread_wq_flag = 1;
			wake_up_interruptible(&g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].Thread_wq);
		}

		prFrame->u4Param1 = (u32)1000 * (u32)jiffies / (u32)HZ;
		TAKE_BT_HW_SEMAPHORE();
		g_prSpeechEnhance.m_u4ActiveIdx = g_prSpeechEnhance.m_prShareMem->u4WriteIdx;
		INCREASE_WRITE_IDX;
		RELEASE_BT_HW_SEMAPHORE();

#if ARM1_SPEECH_INTERACT_WITH_ARM2
		SpeechSendMessage((u32)BT_WRITE_FRAME, g_prSpeechEnhance.m_u4ActiveIdx, (u32)0, (u32)0);
#else
		SpeechEnhance_PostProcess(g_prSpeechEnhance.m_u4ActiveIdx);
		TAKE_BT_HW_SEMAPHORE();
		g_prSpeechEnhance.m_prShareMem->u4ReadIdx = g_prSpeechEnhance.m_u4ActiveIdx;
		INCREASE_READ_IDX;
		RELEASE_BT_HW_SEMAPHORE();
#endif
		g_prSpeechEnhance.m_u4ActiveIdx = SPEECH_FRAME_COUNT;
		g_prSpeechEnhance.m_prActiveFrame = NULL;

		return NOERR;
	}

	return INVALIDPRAM;
}

s32 SpeechEnhance_EnhanceUL(SPEECH_FRAME_T *prFrame)
{
	u32 u4Size = 0;

	if (g_prSpeechEnhance.m_prActiveFrame == prFrame) {
		prFrame->u4Opt = FRAME_OPT_AEC | FRAME_OPT_NDC;

		if (g_prSpeechEnhance.m_fgDbgEnable) {
			prFrame->u4Opt |= DATA_REQ_POST_AEC | DATA_REQ_POST_NDC;
			u4Size = (u32)MAX_DBG_FILE_BUFFER -
				(u32)g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_PRE].pull_len;
			if (u4Size < g_prSpeechEnhance.m_prShareMem->u4FrameByte) {
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_PRE].pwave_buf +
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_PRE].pull_len),
					(void *)prFrame->ULBuf1, u4Size);
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_PRE].pwave_buf),
					(void *)(&prFrame->ULBuf1[u4Size >> 1]),
					(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size));
				g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_PRE].pull_len =
					(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size);
			} else {
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_PRE].pwave_buf +
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_PRE].pull_len),
					(void *)prFrame->ULBuf1, g_prSpeechEnhance.m_prShareMem->u4FrameByte);
				g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_PRE].pull_len +=
					g_prSpeechEnhance.m_prShareMem->u4FrameByte;
			}
			g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_PRE].Thread_wq_flag = 1;
			wake_up_interruptible(&g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_PRE].Thread_wq);

			u4Size = (u32)MAX_DBG_FILE_BUFFER -
				(u32)g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_REF].pull_len;
			if (u4Size < g_prSpeechEnhance.m_prShareMem->u4FrameByte) {
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_REF].pwave_buf +
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_REF].pull_len),
					(void *)prFrame->DLDelayBuf, u4Size);
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_REF].pwave_buf),
					(void *)(&prFrame->DLDelayBuf[u4Size >> 1]),
					(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size));
				g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_REF].pull_len =
					(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size);
			} else {
				memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_REF].pwave_buf +
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_REF].pull_len),
					(void *)prFrame->DLDelayBuf, g_prSpeechEnhance.m_prShareMem->u4FrameByte);
				g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_REF].pull_len +=
					g_prSpeechEnhance.m_prShareMem->u4FrameByte;
			}
			g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_REF].Thread_wq_flag = 1;
			wake_up_interruptible(&g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_REF].Thread_wq);

			if (16000U == g_prSpeechEnhance.m_prShareMem->u4SampleRate) {
				u4Size = (u32)MAX_DBG_FILE_BUFFER -
					(u32)g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].pull_len;
				if (u4Size < g_prSpeechEnhance.m_prShareMem->u4FrameByte) {
					memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].pwave_buf +
						g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].pull_len),
						(void *)prFrame->DLBuf, u4Size);
					memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].pwave_buf),
						(void *)(&prFrame->DLBuf[u4Size >> 1]),
						(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size));
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].pull_len =
						(g_prSpeechEnhance.m_prShareMem->u4FrameByte - u4Size);
				} else {
					memcpy((void *)(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].pwave_buf +
						g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].pull_len),
						(void *)prFrame->DLBuf, g_prSpeechEnhance.m_prShareMem->u4FrameByte);
					g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].pull_len +=
						g_prSpeechEnhance.m_prShareMem->u4FrameByte;
				}
				g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].Thread_wq_flag = 1;
				wake_up_interruptible(&g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].Thread_wq);
			}
		}

		prFrame->u4Param1 = (u32)1000 * (u32)jiffies / (u32)HZ;
		TAKE_BT_HW_SEMAPHORE();
		g_prSpeechEnhance.m_u4ActiveIdx = g_prSpeechEnhance.m_prShareMem->u4WriteIdx;
		INCREASE_WRITE_IDX;
		RELEASE_BT_HW_SEMAPHORE();

#if ARM1_SPEECH_INTERACT_WITH_ARM2
		SpeechSendMessage(BT_WRITE_FRAME, g_prSpeechEnhance.m_u4ActiveIdx, (u32)0, (u32)0);
#else
		SpeechEnhance_PostProcess(g_prSpeechEnhance.m_u4ActiveIdx);
		TAKE_BT_HW_SEMAPHORE();
		g_prSpeechEnhance.m_prShareMem->u4ReadIdx = g_prSpeechEnhance.m_u4ActiveIdx;
		INCREASE_READ_IDX;
		RELEASE_BT_HW_SEMAPHORE();
#endif

		g_prSpeechEnhance.m_u4ActiveIdx = SPEECH_FRAME_COUNT;
		g_prSpeechEnhance.m_prActiveFrame = NULL;

		return NOERR;
	}

	return INVALIDPRAM;
}

s32 SpeechEnhance_GetEnhanceParam(PCM_SPEECH_CONF *prSpeechConf)
{
	u32 u4Idx  = 0;

	if (NULL == prSpeechConf) {
		pr_err("[PCM ERR]SpeechEnhance_GetEnhanceParam: prSpeechConf error!\r\n");
		return INVALIDPRAM;
	}
	x_memcpy(prSpeechConf->enhance_pars, g_prSpeechEnhance.m_prShareMem->rSphParam.enhance_pars,
		sizeof(prSpeechConf->enhance_pars));
	x_memcpy(prSpeechConf->enhance_pars2, g_prSpeechEnhance.m_prShareMem->rSphParam2.enhance_pars,
		sizeof(prSpeechConf->enhance_pars2));
	prSpeechConf->error_flag = g_prSpeechEnhance.m_prShareMem->rSphParam.error_flag;
	prSpeechConf->error_flag2 = g_prSpeechEnhance.m_prShareMem->rSphParam2.error_flag;

	pr_debug("[PCM]SpeechEnhance_GetEnhanceParam: AEC Param = ");
	for (u4Idx = 0; u4Idx < AEC_NDC_PARAM_NUM; u4Idx++) {
		if (0 == (u4Idx % 10U)) {
			pr_debug("\r\n");
		}
		pr_debug("%d, ", (s32)g_prSpeechEnhance.m_prShareMem->rSphParam.enhance_pars[u4Idx]);
	}
	pr_debug("\r\n");
	pr_debug("rSphParam.error_flag = %d\r\n",
		(s32)g_prSpeechEnhance.m_prShareMem->rSphParam.error_flag);

	pr_debug("[PCM]SpeechEnhance_GetEnhanceParam: AEC Param2 = ");
	for (u4Idx = 0; u4Idx < AEC_NDC_PARAM_NUM; u4Idx++) {
		if (0 == (u4Idx % 10U)) {
			pr_debug("\r\n");
		}
		pr_debug("%d, ", (s32)g_prSpeechEnhance.m_prShareMem->rSphParam2.enhance_pars[u4Idx]);
	}
	pr_debug("\r\n");
	pr_debug("rSphParam2.error_flag = %d\r\n",
		(s32)g_prSpeechEnhance.m_prShareMem->rSphParam2.error_flag);

	return NOERR;
}

s32 SpeechEnhance_SetEnhanceParam(const PCM_SPEECH_CONF *prSpeechConf)
{
	u32 u4Idx  = 0;

	if (NULL == prSpeechConf) {
		pr_err("[PCM ERR]SpeechEnhance_SetEnhanceParam: prSpeechConf error!\r\n");
		return INVALIDPRAM;
	}

	if (g_prSpeechEnhance.m_prShareMem) {
		x_memcpy(g_prSpeechEnhance.m_prShareMem->rSphParam.enhance_pars, prSpeechConf->enhance_pars,
			sizeof(g_prSpeechEnhance.m_prShareMem->rSphParam.enhance_pars));
		x_memcpy(g_prSpeechEnhance.m_prShareMem->rSphParam2.enhance_pars, prSpeechConf->enhance_pars2,
			sizeof(g_prSpeechEnhance.m_prShareMem->rSphParam2.enhance_pars));
		g_prSpeechEnhance.m_prShareMem->rSphParam.error_flag = prSpeechConf->error_flag;
		g_prSpeechEnhance.m_prShareMem->rSphParam2.error_flag = prSpeechConf->error_flag2;

		pr_debug("[PCM]SpeechEnhance_SetEnhanceParam: AEC Param = ");
		for (u4Idx = 0; u4Idx < AEC_NDC_PARAM_NUM; u4Idx++) {
			if (0 == (u4Idx % 10U)) {
				pr_debug("\r\n");
			}
			pr_debug("%d, ", (s32)g_prSpeechEnhance.m_prShareMem->rSphParam.enhance_pars[u4Idx]);
		}
		pr_debug("\r\n");
		pr_debug("rSphParam.error_flag = %d\r\n",
			(s32)g_prSpeechEnhance.m_prShareMem->rSphParam.error_flag);

		pr_debug("[PCM]SpeechEnhance_SetEnhanceParam: AEC Param2 = ");
		for (u4Idx = 0; u4Idx < AEC_NDC_PARAM_NUM; u4Idx++) {
			if (0 == (u4Idx % 10U)) {
				pr_debug("\r\n");
			}
			pr_debug("%d, ", (s32)g_prSpeechEnhance.m_prShareMem->rSphParam2.enhance_pars[u4Idx]);
		}
		pr_debug("\r\n");
		pr_debug("rSphParam2.error_flag = %d\r\n",
			(s32)g_prSpeechEnhance.m_prShareMem->rSphParam2.error_flag);
	} else {
		pr_debug("[PCM]SpeechEnhance_SetEnhanceParam:Init AEC Param = ");
		for (u4Idx = 0; u4Idx < AEC_NDC_PARAM_NUM; u4Idx++) {
			Sph_Enh_ctrl1.enhance_pars[u4Idx] = prSpeechConf->enhance_pars[u4Idx];
			if (0 == (u4Idx % 10U)) {
				pr_debug("\r\n");
			}
			pr_debug("%d, ", (s32)Sph_Enh_ctrl1.enhance_pars[u4Idx]);
		}
		pr_debug("\r\n");
		Sph_Enh_ctrl1.error_flag = prSpeechConf->error_flag;
		pr_debug("rSphParam.error_flag = %d\r\n", (s32)Sph_Enh_ctrl1.error_flag);

		pr_debug("[PCM]SpeechEnhance_SetEnhanceParam:Init AEC Param2 = ");
		for (u4Idx = 0; u4Idx < AEC_NDC_PARAM_NUM; u4Idx++) {
			Sph_Enh_ctrl2.enhance_pars[u4Idx] = prSpeechConf->enhance_pars2[u4Idx];
			if (0 == (u4Idx % 10U)) {
				pr_debug("\r\n");
			}
			pr_debug("%d, ", (s32)Sph_Enh_ctrl2.enhance_pars[u4Idx]);
		}
		pr_debug("\r\n");
		Sph_Enh_ctrl2.error_flag = prSpeechConf->error_flag2;
		pr_debug("rSphParam2.error_flag = %d\r\n", (s32)Sph_Enh_ctrl2.error_flag);
	}

	return NOERR;
}

s32 SpeechEnhance_GetEnhance16KParam(PCM_SPEECH_16K_CONF *prSpeechConf)
{
	u32 u4Idx  = 0;

	if (NULL == prSpeechConf) {
		pr_err("[PCM ERR]SpeechEnhance_GetEnhance16KParam: prSpeechConf error!\r\n");
		return INVALIDPRAM;
	}
	x_memcpy(prSpeechConf->enhance_16k_pars, g_prSpeechEnhance.m_prShareMem->Sph_Enh_ctrl_16k,
		sizeof(prSpeechConf->enhance_16k_pars));

	pr_debug("[PCM]SpeechEnhance_GetEnhance16KParam: AEC 16K Param = ");
	for (u4Idx = 0; u4Idx < AEC_NDC_PARAM_NUM; u4Idx++) {
		if (0 == (u4Idx % 10U)) {
			pr_debug("\r\n");
		}
		pr_debug("%d, ", (s32)prSpeechConf->enhance_16k_pars[u4Idx]);
	}
	pr_debug("\r\n");

	return NOERR;
}

s32 SpeechEnhance_SetEnhance16KParam(PCM_SPEECH_16K_CONF *prSpeechConf)
{
	u32 u4Idx  = 0;

	if (NULL == prSpeechConf) {
		pr_err("[PCM ERR]SpeechEnhance_SetEnhance16KParam: prSpeechConf error!\r\n");
		return INVALIDPRAM;
	}

	if (g_prSpeechEnhance.m_prShareMem) {
		x_memcpy(g_prSpeechEnhance.m_prShareMem->Sph_Enh_ctrl_16k, prSpeechConf->enhance_16k_pars,
			sizeof(g_prSpeechEnhance.m_prShareMem->Sph_Enh_ctrl_16k));
		
		x_memcpy(g_prSpeechEnhance.m_prShareMem->Sph_Enh_ctrl_comm_16k, prSpeechConf->enhance_16k_comm_pars,
			sizeof(g_prSpeechEnhance.m_prShareMem->Sph_Enh_ctrl_comm_16k));
		pr_debug("[PCM]SpeechEnhance_SetEnhance16KParam: AEC 16K Param = ");
		for (u4Idx = 0; u4Idx < AEC_NDC_PARAM_NUM; u4Idx++) {
			if (0 == (u4Idx % 10U)) {
				pr_debug("\r\n");
			}
			pr_debug("%d, ", (s32)prSpeechConf->enhance_16k_pars[u4Idx]);
		}
		pr_debug("\r\n");

		pr_debug("[PCM]SpeechEnhance_SetEnhance16KParam: AEC 16K common Param = ");
		for (u4Idx = 0; u4Idx < COMMOM_PARAM_NUM; u4Idx++) {
			if (0 == (u4Idx % 10U)) {
				pr_debug("\r\n");
			}
			pr_debug("%d, ", (s32)prSpeechConf->enhance_16k_comm_pars[u4Idx]);
		}
		pr_debug("\r\n");
	} else {
		pr_debug("[PCM]SpeechEnhance_SetEnhance16KParam:Init AEC 16K Param = ");
		for (u4Idx = 0; u4Idx < AEC_NDC_PARAM_NUM; u4Idx++) {
			Sph_Enh_ctrl_16k[u4Idx] = prSpeechConf->enhance_16k_pars[u4Idx];
			if (0 == (u4Idx % 10U)) {
				pr_debug("\r\n");
			}
			pr_debug("%d, ", (s32)Sph_Enh_ctrl_16k[u4Idx]);
		}
		pr_debug("\r\n");

		pr_debug("[PCM]SpeechEnhance_SetEnhance16KParam:Init AEC 16K common Param = ");
		for (u4Idx = 0; u4Idx < COMMOM_PARAM_NUM; u4Idx++) {
			Sph_Enh_ctrl_comm_16k[u4Idx] = prSpeechConf->enhance_16k_comm_pars[u4Idx];
			if (0 == (u4Idx % 10U)) {
				pr_debug("\r\n");
			}
			pr_debug("%d, ", (s32)Sph_Enh_ctrl_comm_16k[u4Idx]);
		}
		pr_debug("\r\n");
	}

	return NOERR;
}

s32 SpeechEnhance_SetDmnrParam(const PCM_SPEECH_CONF *prSpeechConf)
{
	u32 u4Idx  = 0;

	if (NULL == prSpeechConf) {
		pr_err("[PCM ERR]SpeechEnhance_SetDmnrParam: prSpeechConf error!\r\n");
		return INVALIDPRAM;
	}

	if (g_prSpeechEnhance.m_prShareMem) {
		pr_debug("[PCM]SpeechEnhance_SetDmnrParam: DMNR Param =");
		for (u4Idx = 0; u4Idx < DMNR_PARAM_NUM; u4Idx++) {
			g_prSpeechEnhance.m_prShareMem->rDmnrParam.dmnrParm[u4Idx] =
				prSpeechConf->ABF_cal_data[u4Idx];
			if (0 == (u4Idx % 10U)) {
				pr_debug("\r\n");
			}
			pr_debug("%d, ", (s32)g_prSpeechEnhance.m_prShareMem->rDmnrParam.dmnrParm[u4Idx]);
		}
		pr_debug("\r\n");
	} else {
		pr_debug("[PCM]SpeechEnhance_SetDmnrParam:Init DMNR Param =");
		for (u4Idx = 0; u4Idx < DMNR_PARAM_NUM; u4Idx++) {
			ABF_cal_data[u4Idx] = prSpeechConf->ABF_cal_data[u4Idx];
			if (0 == (u4Idx % 10U)) {
				pr_debug("\r\n");
			}
			pr_debug("%d, ", (s32)ABF_cal_data[u4Idx]);
		}
		pr_debug("\r\n");
	}

	return NOERR;
}

u32 SpeechEnhance_EnableDmnr(bool fgEnable)
{
	g_prSpeechEnhance.m_rSphOpt.fgDMNR = fgEnable;
	pr_debug("SpeechEnhance_EnableDmnr: fgEnable(%d)", (s32)g_prSpeechEnhance.m_rSphOpt.fgDMNR);

	return NOERR;
}

bool SpeechEnhance_IsDmnrEnable(void)
{
	return g_prSpeechEnhance.m_rSphOpt.fgDMNR;
}

s32 SpeechEnhance_SetComRxParam(const PCM_SPEECH_CONF *prSpeechConf)
{
	u32 u4Idx  = 0;

	if (NULL == prSpeechConf) {
		pr_err("[PCM ERR]SpeechEnhance_SetComRxParam: prSpeechConf error!\r\n");
		return INVALIDPRAM;
	}

	if (g_prSpeechEnhance.m_prShareMem) {
		pr_debug("[PCM]SpeechEnhance_SetComRxParam: Param = ");
		for (u4Idx = 0; u4Idx < AEC_COM_RX; u4Idx++) {
			g_prSpeechEnhance.m_prShareMem->rAecRxParam.enhance_pars[u4Idx] =
				prSpeechConf->aec_com_rx[u4Idx];
			if (0 == (u4Idx % 10U)) {
				pr_debug("\r\n");
			}
			pr_debug("%d, ", g_prSpeechEnhance.m_prShareMem->rAecRxParam.enhance_pars[u4Idx]);
		}
		pr_debug("\r\n");
	} else {
		pr_debug("[PCM]SpeechEnhance_SetComRxParam:Init Param = ");
		for (u4Idx = 0; u4Idx < AEC_COM_RX; u4Idx++) {
			aec_com_rx[u4Idx] = prSpeechConf->aec_com_rx[u4Idx];
			if (0 == (u4Idx % 10U)) {
				pr_debug("\r\n");
			}
			pr_debug("%d, ", aec_com_rx[u4Idx]);
		}
		pr_debug("\r\n");
	}
	return NOERR;
}

s32 SpeechEnhance_SetComTxParam(const PCM_SPEECH_CONF *prSpeechConf)
{
	u32 u4Idx  = 0;

	if (NULL == prSpeechConf) {
		pr_err("[PCM ERR]SpeechEnhance_SetComTxParam: prSpeechConf error!\r\n");
		return INVALIDPRAM;
	}

	if (g_prSpeechEnhance.m_prShareMem) {
		pr_debug("[PCM]SpeechEnhance_SetComTxParam: Param = ");
		for (u4Idx = 0; u4Idx < AEC_COM_TX; u4Idx++) {
			g_prSpeechEnhance.m_prShareMem->rAecTxParam.enhance_pars[u4Idx] =
				prSpeechConf->aec_com_tx[u4Idx];
			if (0 == (u4Idx % 10U)) {
				pr_debug("\r\n");
			}
			pr_debug("%d, ", g_prSpeechEnhance.m_prShareMem->rAecTxParam.enhance_pars[u4Idx]);
		}
		pr_debug("\r\n");
	} else {
		pr_debug("[PCM]SpeechEnhance_SetComTxParam:Init Param = ");
		for (u4Idx = 0; u4Idx < AEC_COM_TX; u4Idx++) {
			aec_com_tx[u4Idx] = prSpeechConf->aec_com_tx[u4Idx];
			if (0 == (u4Idx % 10U)) {
				pr_debug("\r\n");
			}
			pr_debug("%d, ", aec_com_tx[u4Idx]);
		}
		pr_debug("\r\n");
	}

	return NOERR;
}

s32 SpeechEnhance_SetDmnr16kParam(const PCM_SPEECH_16K_CONF *prSpeechConf)
{
	u32 u4Idx  = 0;

	if (NULL == prSpeechConf) {
		pr_err("[PCM ERR]SpeechEnhance_SetDmnr16kParam: prSpeechConf error!\r\n");
		return INVALIDPRAM;
	}
	if (g_prSpeechEnhance.m_prShareMem) {
		pr_debug("[PCM]SpeechEnhance_SetDmnr16kParam: 16k DMNR_cal_data Param =");
		for (u4Idx = 0; u4Idx < DMNR_PARAM_NUM_16K; u4Idx++) {
			g_prSpeechEnhance.m_prShareMem->DMNR_cal_data_16k[u4Idx] =
				prSpeechConf->dmnr_16k_pars[u4Idx];
			if (0 == (u4Idx % 10U)) {
				pr_debug("\r\n");
			}
			pr_debug("%d, ", (s32)g_prSpeechEnhance.m_prShareMem->DMNR_cal_data_16k[u4Idx]);
		}
		pr_debug("\r\n");
	} else {
		pr_debug("[PCM]SpeechEnhance_SetDmnr16kParam:Init 16k DMNR_cal_data Param =");
		for (u4Idx = 0; u4Idx < DMNR_PARAM_NUM_16K; u4Idx++) {
			DMNR_cal_data_16k[u4Idx] = prSpeechConf->dmnr_16k_pars[u4Idx];
			if (0 == (u4Idx % 10U)) {
				pr_debug("\r\n");
			}
			pr_debug("%d, ", (s32)DMNR_cal_data_16k[u4Idx]);
		}
		pr_debug("\r\n");
	}

	return NOERR;
}

s32 SpeechEnhance_SetFilter16kParam(const signed short *pData, u32 u4DataLen)
{
	u32 u4Idx  = 0;

	if (NULL == pData) {
		pr_err("[PCM ERR]SpeechEnhance_SetFilter16kParam: pData error!\r\n");
		return INVALIDPRAM;
	}
	if (u4DataLen != sizeof(g_prSpeechEnhance.m_prShareMem->Compen_filter_16k)) {
		pr_err("[PCM ERR]SpeechEnhance_SetFilter16kParam: size error!\r\n");
		return INVALIDPRAM;
	}

	if (g_prSpeechEnhance.m_prShareMem) {
		memcpy(g_prSpeechEnhance.m_prShareMem->Compen_filter_16k, pData, u4DataLen);
		pr_debug("[PCM]SpeechEnhance_SetFilter16kParam: 16k COMPEN_FILTER Param =");
		for (u4Idx = 0; u4Idx < COMPEN_FILTER_16K; u4Idx++) {
			if (0 == (u4Idx % 10U)) {
				pr_debug("\r\n");
			}
			pr_debug("%d, ", (s32)g_prSpeechEnhance.m_prShareMem->Compen_filter_16k[u4Idx]);
		}
		pr_debug("\r\n");
	} else {
		memcpy(Compen_filter_16k, pData, u4DataLen);
		pr_debug("[PCM]SpeechEnhance_SetFilter16kParam:Init 16k COMPEN_FILTER Param =");
		for (u4Idx = 0; u4Idx < COMPEN_FILTER_16K; u4Idx++) {
			if (0 == (u4Idx % 10U)) {
				pr_debug("\r\n");
			}
			pr_debug("%d, ", (s32)Compen_filter_16k[u4Idx]);
		}
		pr_debug("\r\n");
	}

	return NOERR;
}

u32 SpeechEnhance_SetARM2SpeechLog(u32 u4logLevel)
{
	g_prSpeechEnhance.m_rSphOpt.u4Log = 0;
	if (u4logLevel < ZONE_INFO) {
		g_prSpeechEnhance.m_rSphOpt.u4Log |= 0x0;
	} else if (u4logLevel == ZONE_INFO) {
		g_prSpeechEnhance.m_rSphOpt.u4Log |= 0x1U;
	} else if (u4logLevel <= ZONE_DBG) {
		g_prSpeechEnhance.m_rSphOpt.u4Log |= 0x7U;
	} else {
		g_prSpeechEnhance.m_rSphOpt.u4Log |= 0xFU;
	}
	pr_debug("[PCM]SpeechEnhance_SetARM2SpeechLog: u4Log = 0x%02x\r\n",
		(u32)g_prSpeechEnhance.m_rSphOpt.u4Log);

	return NOERR;
}

u32 SpeechEnhance_EnablePLC(bool fgEnable)
{
	g_prSpeechEnhance.m_rSphOpt.fgPLC = fgEnable;
	pr_debug("[PCM]SpeechEnhance_EnablePLC: m_fgPLC = %d\r\n",
		(u32)g_prSpeechEnhance.m_rSphOpt.fgPLC);

	return NOERR;
}


u32 SpeechEnhance_EnableDump(bool fgEnable)
{
	g_prSpeechEnhance.m_fgDbgEnable = fgEnable;
	pr_debug("[PCM]SpeechEnhance_EnableDump: fgEnable(%d)\r\n\r\n",
		(s32)g_prSpeechEnhance.m_fgDbgEnable);

	return NOERR;
}


void WriteWaveFileHeader(const char *filename, const WaveHeader *prWaveHdr)
{
	struct file *fd = NULL;
	mm_segment_t fs;
	int32_t u4Size = 0;
	loff_t t_cur_pos = 0;

	fs = get_fs();
	set_fs(KERNEL_DS);
	fd = filp_open(filename, O_RDWR | O_CREAT, 0);
	if (IS_ERR(fd)) {
		pr_err("[PCM ERR]WriteWaveFileHeader: filp_open err\r\n");
		set_fs(fs);
		return;
	}
	t_cur_pos = vfs_llseek(fd, (loff_t)0, SEEK_SET);
	u4Size = vfs_write(fd, (char *)prWaveHdr, sizeof(WaveHeader), &t_cur_pos);
	if (sizeof(WaveHeader) != u4Size) {
		pr_err("[PCM ERR]WriteWaveFileHeader: vfs write1 err %i\r\n", u4Size);
	}
	filp_close(fd, NULL);
	set_fs(fs);
}

static s32 BtDbg_File_Thread(void *data)
{
	struct save_wavbuf *BtDbg_wavbuf = (struct save_wavbuf *)data;
	struct file *fd = NULL;
	WaveHeader m_rWaveHdr;
	u32 data_pos = 0;
	loff_t t_cur_pos = 0;
    int id = BtDbg_wavbuf->BtDbg_File_Idx;
	WriteWaveFileHeader(BtDbg_wavbuf->szFileName, &m_rWaveHdr);
	data_pos = sizeof(WaveHeader);
	while (1) {
		int32_t u4Size = 0;
		int32_t u4Sizetmp = 0;
		mm_segment_t fs;

		wait_event_interruptible(BtDbg_wavbuf->Thread_wq, BtDbg_wavbuf->Thread_wq_flag);
		if (BtDbg_wavbuf->fgStopThread) {
			break;
		}

		BtDbg_wavbuf->Thread_wq_flag = 0;
		BtDbg_wavbuf->data_Len += g_prSpeechEnhance.m_prShareMem->u4FrameByte;
		fs = get_fs();
		set_fs(KERNEL_DS);
		fd = filp_open(BtDbg_wavbuf->szFileName, O_RDWR | O_CREAT, 0);
		if (IS_ERR(fd)) {
			pr_err("[PCM ERR]BtDbg_File_Thread: filp_open err.filename(%s).\r\n", BtDbg_wavbuf->szFileName);
			set_fs(fs);
			continue;
		}

		t_cur_pos = vfs_llseek(fd, (loff_t)data_pos, SEEK_SET);
        if ((BtDbg_wavbuf->writed_len + BtDbg_wavbuf->data_Len) > MAX_DBG_FILE_BUFFER)
        {
            down(&(g_rSpeechDbgSema[id]));
            if (BtDbg_wavbuf->pwave_buf != NULL)
            {
			u4Sizetmp = MAX_DBG_FILE_BUFFER - BtDbg_wavbuf->writed_len;
			u4Size = vfs_write(fd, (BtDbg_wavbuf->pwave_buf + BtDbg_wavbuf->writed_len),
				u4Sizetmp, &t_cur_pos);
			if (u4Size < 0) {
				pr_err("[PCM ERR]BtDbg_File_Thread: vfs write1 err %i\r\n", u4Size);
				filp_close(fd, NULL);
				set_fs(fs);
                up(&(g_rSpeechDbgSema[id]));
				continue;
			}
			BtDbg_wavbuf->data_Len -= u4Size;
			data_pos += u4Size;
			BtDbg_wavbuf->writed_len = 0;

			t_cur_pos = vfs_llseek(fd, (loff_t)data_pos, SEEK_SET);
			u4Size = vfs_write(fd, (BtDbg_wavbuf->pwave_buf), BtDbg_wavbuf->data_Len, &t_cur_pos);
			if (u4Size < 0) {
				pr_err("[PCM ERR]BtDbg_File_Thread: vfs write2 err %i\r\n", u4Size);
				filp_close(fd, NULL);
				set_fs(fs);
                up(&(g_rSpeechDbgSema[id]));
				continue;
			}
			BtDbg_wavbuf->data_Len -= u4Size;
			data_pos += u4Size;
			BtDbg_wavbuf->writed_len += u4Size;
            }
            up(&(g_rSpeechDbgSema[id]));
        }
        else
        {
            down(&(g_rSpeechDbgSema[id]));
            if (BtDbg_wavbuf->pwave_buf != NULL)
            {
                u4Size = vfs_write(fd, (BtDbg_wavbuf->pwave_buf + BtDbg_wavbuf->writed_len), BtDbg_wavbuf->data_Len, &t_cur_pos);
                if (u4Size < 0)
                {
                    pr_err("[PCM]BtDbg_File_Thread: vfs write3 err %i\r\n",u4Size);
				    filp_close(fd, NULL);
				    set_fs(fs);
                    up(&(g_rSpeechDbgSema[id]));
				    continue;
			    }
			BtDbg_wavbuf->data_Len -= u4Size;
			data_pos += u4Size;
			BtDbg_wavbuf->writed_len += u4Size;
		}
            up(&(g_rSpeechDbgSema[id]));
        }
        if (BtDbg_wavbuf->writed_len >= MAX_DBG_FILE_BUFFER)
        {
			BtDbg_wavbuf->writed_len = 0;
		}
		filp_close(fd, NULL);
		set_fs(fs);
	}

	m_rWaveHdr.datasize = data_pos - sizeof(WaveHeader);
	m_rWaveHdr.filesize = data_pos;
	m_rWaveHdr.nSamplesPerSec = (u32)(BtDbg_wavbuf->samplerate);
	m_rWaveHdr.nChannels = 1;
	m_rWaveHdr.wBitsPerSample = 16;
	m_rWaveHdr.riff[0] = (u8)'R';
	m_rWaveHdr.riff[1] = (u8)'I';
	m_rWaveHdr.riff[2] = (u8)'F';
	m_rWaveHdr.riff[3] = (u8)'F';

	m_rWaveHdr.wave[0] = (u8)'W';
	m_rWaveHdr.wave[1] = (u8)'A';
	m_rWaveHdr.wave[2] = (u8)'V';
	m_rWaveHdr.wave[3] = (u8)'E';

	m_rWaveHdr.fmt[0] = (u8)'f';
	m_rWaveHdr.fmt[1] = (u8)'m';
	m_rWaveHdr.fmt[2] = (u8)'t';
	m_rWaveHdr.fmt[3] = (u8)' ';
	m_rWaveHdr.fmtsize = 0x10;
	m_rWaveHdr.wFormatTag = 0x01;
	m_rWaveHdr.nAvgBytesPerSec = m_rWaveHdr.nSamplesPerSec * m_rWaveHdr.nChannels * (m_rWaveHdr.wBitsPerSample / 8);
	m_rWaveHdr.nBlockAlign = m_rWaveHdr.wBitsPerSample / 8 * m_rWaveHdr.nChannels;
	m_rWaveHdr.data[0] = (u8)'d';
	m_rWaveHdr.data[1] = (u8)'a';
	m_rWaveHdr.data[2] = (u8)'t';
	m_rWaveHdr.data[3] = (u8)'a';

	WriteWaveFileHeader(BtDbg_wavbuf->szFileName, &m_rWaveHdr);

	return 0;
}

static s32 SpeechEnhance_InitDebugWaveFiles(void)
{
	u32 u4Idx = 0;
	s32 err = 0;
	struct task_struct *Thread_task = NULL;

	memset(g_prSpeechEnhance.m_rDbgWave, 0, sizeof(g_prSpeechEnhance.m_rDbgWave));
	sprintf(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].szFileName,
		DEBUG_SDCARD_PRE "/%05d_fe.wav", g_prSpeechEnhance.m_u4DebugIdx);
	pr_debug("[PCM]SpeechEnhance_InitDebugWaveFiles: Create BtDbgFile(%s)\r\n",
		g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_PRE].szFileName);

	sprintf(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].szFileName,
		DEBUG_SDCARD_PRE "/%05d_fe_aec.wav", g_prSpeechEnhance.m_u4DebugIdx);
	pr_debug("[PCM]SpeechEnhance_InitDebugWaveFiles: Create BtDbgFile(%s)\r\n",
		g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_POST].szFileName);

	sprintf(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].szFileName,
		DEBUG_SDCARD_PRE "/%05d_fe_ndc.wav", g_prSpeechEnhance.m_u4DebugIdx);
	pr_debug("[PCM]SpeechEnhance_InitDebugWaveFiles: Create BtDbgFile(%s)\r\n",
		g_prSpeechEnhance.m_rDbgWave[DBG_FILE_DL_NDC_POST].szFileName);

	sprintf(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_PRE].szFileName,
		DEBUG_SDCARD_PRE "/%05d_ne_mic1.wav", g_prSpeechEnhance.m_u4DebugIdx);
	pr_debug("[PCM]SpeechEnhance_InitDebugWaveFiles: Create BtDbgFile(%s)\r\n",
		g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_PRE].szFileName);

	sprintf(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_REF].szFileName,
		DEBUG_SDCARD_PRE "/%05d_ne_fe.wav", g_prSpeechEnhance.m_u4DebugIdx);
	pr_debug("[PCM]SpeechEnhance_InitDebugWaveFiles: Create BtDbgFile(%s)\r\n",
		g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_REF].szFileName);

	sprintf(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_AEC_POST].szFileName,
		DEBUG_SDCARD_PRE "/%05d_ne_aec.wav", g_prSpeechEnhance.m_u4DebugIdx);
	pr_debug("[PCM]SpeechEnhance_InitDebugWaveFiles: Create BtDbgFile(%s)\r\n",
		g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_AEC_POST].szFileName);

	sprintf(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_POST].szFileName,
		DEBUG_SDCARD_PRE "/%05d_ne_ndc.wav", g_prSpeechEnhance.m_u4DebugIdx);
	pr_debug("[PCM]SpeechEnhance_InitDebugWaveFiles: Create BtDbgFile(%s)\r\n",
		g_prSpeechEnhance.m_rDbgWave[DBG_FILE_UL_POST].szFileName);

	sprintf(g_prSpeechEnhance.m_rDbgWave[DBG_FILE_PCM_APP_PTR].szFileName,
		DEBUG_SDCARD_PRE "/%05d_pcm_app_ptr.wav",g_prSpeechEnhance.m_u4DebugIdx);
	pr_debug("[PCM]SpeechEnhance_InitDebugWaveFiles: Create BtDbgFile(%s)\r\n",
		g_prSpeechEnhance.m_rDbgWave[DBG_FILE_PCM_APP_PTR].szFileName);

	for (u4Idx = 0; u4Idx < DBG_FILE_NUM; u4Idx++) {
        sema_init(&g_rSpeechDbgSema[u4Idx], 1);
		Thread_task = kthread_create(BtDbg_File_Thread,
			(void *)&g_prSpeechEnhance.m_rDbgWave[u4Idx], "BtDbg_File_Thread");
		if (IS_ERR(Thread_task)) {
			pr_err("[PCM ERR]InitDebugWaveFiles: Create BtDbg_File_Thread %d ERR!\r\n", u4Idx);
			err = (s32)PTR_ERR(Thread_task);
			Thread_task = NULL;
			goto ERROR;
		}
		g_prSpeechEnhance.m_rDbgWave[u4Idx].pwave_buf = kmalloc(MAX_DBG_FILE_BUFFER, GFP_KERNEL);
		if (!g_prSpeechEnhance.m_rDbgWave[u4Idx].pwave_buf) {
			err = NORESOURCE;
			goto ERROR;
		}
		g_prSpeechEnhance.m_rDbgWave[u4Idx].BtDbg_File_Idx = u4Idx;
		g_prSpeechEnhance.m_rDbgWave[u4Idx].Thread_task = Thread_task;
		init_waitqueue_head(&g_prSpeechEnhance.m_rDbgWave[u4Idx].Thread_wq);
		g_prSpeechEnhance.m_rDbgWave[u4Idx].Thread_wq_flag = 0;
        wake_up_process(g_prSpeechEnhance.m_rDbgWave[u4Idx].Thread_task);
		g_prSpeechEnhance.m_rDbgWave[u4Idx].fgStopThread = false;
		g_prSpeechEnhance.m_rDbgWave[u4Idx].samplerate = g_prSpeechEnhance.m_prShareMem->u4SampleRate;
	}

	g_prSpeechEnhance.m_u4DebugIdx++;

	return NOERR;

ERROR:
	for (u4Idx = 0; u4Idx < DBG_FILE_NUM; u4Idx++) {
		g_prSpeechEnhance.m_rDbgWave[u4Idx].fgStopThread = true;
		g_prSpeechEnhance.m_rDbgWave[u4Idx].Thread_wq_flag = 1;
        //wake_up_interruptible(&g_prSpeechEnhance.m_rDbgWave[u4Idx].Thread_wq);
        if (g_prSpeechEnhance.m_rDbgWave[u4Idx].Thread_wq.task_list.prev != NULL &&
            g_prSpeechEnhance.m_rDbgWave[u4Idx].Thread_wq.task_list.next != NULL )
        {
			wake_up_interruptible(&g_prSpeechEnhance.m_rDbgWave[u4Idx].Thread_wq);
        }
		g_prSpeechEnhance.m_rDbgWave[u4Idx].Thread_task = NULL;
        if (g_prSpeechEnhance.m_rDbgWave[u4Idx].pwave_buf)
        {
            down(&(g_rSpeechDbgSema[u4Idx]));
			kfree(g_prSpeechEnhance.m_rDbgWave[u4Idx].pwave_buf);
			g_prSpeechEnhance.m_rDbgWave[u4Idx].pwave_buf = NULL;
            up(&(g_rSpeechDbgSema[u4Idx]));
        }
	}

	return err;
}

static void SpeechEnhance_UnInitDebugWaveFiles(void)
{
	u32 u4Idx = 0;

	for (u4Idx = 0; u4Idx < DBG_FILE_NUM; u4Idx++) {
		g_prSpeechEnhance.m_rDbgWave[u4Idx].fgStopThread = true;
		g_prSpeechEnhance.m_rDbgWave[u4Idx].Thread_wq_flag = 1;
        if (g_prSpeechEnhance.m_rDbgWave[u4Idx].Thread_wq.task_list.prev != NULL &&
            g_prSpeechEnhance.m_rDbgWave[u4Idx].Thread_wq.task_list.next != NULL )
        {
			wake_up_interruptible(&g_prSpeechEnhance.m_rDbgWave[u4Idx].Thread_wq);
        }
        //wake_up_interruptible(&g_prSpeechEnhance.m_rDbgWave[u4Idx].Thread_wq);
        /*if (g_prSpeechEnhance.m_rDbgWave[u4Ret].Thread_task)
        {
            kthread_stop(g_prSpeechEnhance.m_rDbgWave[u4Ret].Thread_task);
            g_prSpeechEnhance.m_rDbgWave[u4Ret].Thread_task = NULL;
            
        }*/
		g_prSpeechEnhance.m_rDbgWave[u4Idx].Thread_task = NULL;
        if (g_prSpeechEnhance.m_rDbgWave[u4Idx].pwave_buf)
        {
            down(&(g_rSpeechDbgSema[u4Idx]));
			kfree(g_prSpeechEnhance.m_rDbgWave[u4Idx].pwave_buf);
			g_prSpeechEnhance.m_rDbgWave[u4Idx].pwave_buf = NULL;
            up(&(g_rSpeechDbgSema[u4Idx]));
        }
        
	}
}

