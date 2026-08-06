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

/*!
* @file dmx_gau.c
*
* @par Project
*
*
* @par Description
*
* @par Author_Name
*	  Shuhui Zhang
*
*/
#ifdef __linux__
#include <linux/mm.h>
#include <linux/errno.h>
#include "windows.h"
#include "drv_win32_if.h"
#include <media/atc/dmx_define.h>
#include <media/atc/ioctl_dmx.h>
#include <media/atc/drv_esm_if.h>
#include <media/atc/perf_timer.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "ioctl_dmx.h"
#include "drv_esm_if.h"
#include "perf_timer.h"
#include "mm_debug.h"
#endif

#include "dmx_cli.h"
#include "dmx_dump.h"
#include "dmx_gau_if.h"
#include "dmx_gau.h"
#include "dmx_esm_if.h"
#include "dmx_esm.h"
#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_spt.h"
#include "dmx_spt_if.h"
#include "dmx_psr_cc.h"
#include "dmx_psr_filter.h"

#include "dmx_pfm.h"
#include "dmx_gau_queue.h"

/* 64K */
#define DMX_SP_INIT_SELFFIFO_SZ	((u32)((u32)1 << (u32)16))
/* 64K */
#define DMX_AUD_INIT_SELFFIFO_SZ	((u32)((u32)1 << (u32)16))

#define DMX_VID_INIT_SELFFIFO_SZ	((u32)((u32)1 << (u32)19))
EXTERN DMX_CLI_MAN_T   g_rDmxCliMan;
EXTERN DMX_CLI_MAN_T   g_rDmxCliMan;
EXTERN bool	g_fgDmxGauInit;
EXTERN GAU_MANAMENT_INFO_T g_rDmxGauManager;
EXTERN AU_AUDIO	g_arLogAudioAUs[DMX_MAX_LOG_AUDIO_AU_CNT];
EXTERN PSR_STRUCT_T   g_rPSRHalStruct[DMX_DEV_CNT];
EXTERN DMX_ESM_INST_T g_arESMInst[];

static MRESULT JudgeThreshold(u32 u4Handle);

#if DMX_DISABLE_COMP_AU
static MRESULT GAU_GetVirtualAU(ES_TYPE eType, u32 u4Handle,
	ESM_IO_BUF_INFO *pEsmIOBufInfo)
{
	if (NULL == pEsmIOBufInfo) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU,
		TEXT("[GAU] %s line %d fail for invalid args\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (MAX_GAU_INSTANCE_CNT <= u4Handle) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU,
		TEXT("[GAU] %s line %d fail for u4Handle(%d) > MAX_GAU_INSTANCE_CNT(%d)\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, MAX_GAU_INSTANCE_CNT);
		pEsmIOBufInfo->u4Status = GAU_E_INVALIDARG;
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (g_rDmxGauManager.arGAUInstance[u4Handle].fgEOS) {
		pEsmIOBufInfo->u4Status = GAU_E_EOS;
		MM_RETURN(RET_DMX_NO_AU);
	}

	if (g_rDmxGauManager.arGAUInstance[u4Handle].u4AUConsumedCnt == 0) {
		pEsmIOBufInfo->u4Status = GAU_S_DISCONTINUOUS;
	} else {
		pEsmIOBufInfo->u4Status = GAU_S_OK;
		if (g_rDmxGauManager.arGAUInstance[u4Handle].u4AUConsumedCnt % 5 == 4)
			DMX_THREAD_DELAY(5);
	}

	/* convert to phisical address  */
	if (ES_V == eType) {
		pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.u8Pts =
			g_rDmxGauManager.arGAUInstance[u4Handle].u4AUConsumedCnt * (DMX_PTS_1S / 30);
	} else if (ES_A == eType) {
		pEsmIOBufInfo->rAU.rAudioAU.rAUInfo.rInfo.u8Pts =
			g_rDmxGauManager.arGAUInstance[u4Handle].u4AUConsumedCnt * (DMX_PTS_1S / 30);
	} else if (ES_SP == eType) {
		pEsmIOBufInfo->rAU.rSPStruct.u8StartPts =
			g_rDmxGauManager.arGAUInstance[u4Handle].u4AUConsumedCnt * (DMX_PTS_1S / 30);
		pEsmIOBufInfo->rAU.rSPStruct.i8Delay = (s64)(DMX_PTS_1S / 30);
	}

	++g_rDmxGauManager.arGAUInstance[u4Handle].u4AUConsumedCnt;

	MM_RETURN(RET_DMX_OK);
}
#endif /* DMX_DISABLE_COMP_AU */

void GAU_PrintLogAUInfo(void)
{
	u32 u4Idxj = 0;

	for (u4Idxj = 0; u4Idxj < (u32)MAX_GAU_INSTANCE_CNT; u4Idxj++) {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] AUDIO AU[%d] (Type: %d, DataSA: 0x%x,")
			TEXT("DataEA: 0x%x, PTS: "DMX_PTS_LOGSTR")\r\n"),
			u4Idxj,
			g_arLogAudioAUs[u4Idxj].eAuType,
			g_arLogAudioAUs[u4Idxj].ptrSAddr,
			g_arLogAudioAUs[u4Idxj].ptrEAddr,
			DMX_PTS_LOG_MS(g_arLogAudioAUs[u4Idxj].rAUInfo.rInfo.u8Pts),
			DMX_PTS_LOG_PTS(g_arLogAudioAUs[u4Idxj].rAUInfo.rInfo.u8Pts));
	}
}

MRESULT GAU_DumpInfo(void)
{
	u32	   u4StmType = ES_NONE;
	u32	   u4Idx	 = 0;

	for (u4Idx = 0; u4Idx < (u32)MAX_GAU_INSTANCE_CNT; u4Idx++) {
		if (!g_rDmxGauManager.arGAUInstance[u4Idx].fgUsed)
			break;

		if (g_rDmxGauManager.arGAUInstance[u4Idx].u4ESHandle >= MAX_ESICOUNT)
			continue;

		u4StmType = g_rDmxGauManager.arGAUInstance[u4Idx].u4StmType;

		DmxLogT(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("GAU[%d] --> ESIH: 0x%x, ESType: %s, fgGAUEnable: %d, fgEOS: %d,")
			TEXT("u4Status: %d, pvSptHdl: 0x%x\r\n"),
			u4Idx,
			g_rDmxGauManager.arGAUInstance[u4Idx].u4ESHandle,
			((u4StmType < MAX_SPT_DATA_TYPE_CNT) ? g_aszSptDataTypeName[u4StmType] : TEXT("UNKNOWN")),
			(g_rDmxGauManager.arGAUInstance[u4Idx].fgGetAUEnable ? 1 : 0),
			(g_rDmxGauManager.arGAUInstance[u4Idx].fgEOS ? 1 : 0),
			g_rDmxGauManager.arGAUInstance[u4Idx].u4Status,
			g_rDmxGauManager.arGAUInstance[u4Idx].pvSptHdl);
		DmxLogT(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("g_rDmxGauManager.arGAUInstance[%d] --> Threshold (Size: 0x%x), fgRchThreh: %d\r\n"),
			u4Idx,
			g_rDmxGauManager.arGAUInstance[u4Idx].u4Threshold,
			(g_rDmxGauManager.arGAUInstance[u4Idx].fgReachThreshold ? 1 : 0));
	}

	MM_RETURN(RET_DMX_OK);
}

bool GAU_GetEOSStatus(u32 u4Handle, bool *pfgEOS, u32 *pu4Status)
{
	if ((MAX_GAU_INSTANCE_CNT <= u4Handle) ||
		(NULL == pfgEOS) ||
		(NULL == pu4Status)) {
		return FALSE;
	}

	*pfgEOS = g_rDmxGauManager.arGAUInstance[u4Handle].fgEOS;
	*pu4Status = g_rDmxGauManager.arGAUInstance[u4Handle].u4Status;

	return TRUE;
}

void GAU_SetEOS(void *pvSptHdl, bool fgEOS, u32 u4Status)
{
	u32 u4Idx = 0;

	if (NULL == pvSptHdl) {
		for (u4Idx = 0; u4Idx < MAX_GAU_INSTANCE_CNT; ++u4Idx) {
			if (g_rDmxGauManager.arGAUInstance[u4Idx].fgUsed) {
				if (fgEOS &&
					((!g_rDmxGauManager.arGAUInstance[u4Idx].fgEOS) ||
					 (g_rDmxGauManager.arGAUInstance[u4Idx].u4Status != u4Status))) {
					DmxLogT(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
						TEXT(
"[GAU] --------->GAU_SetEOS - GAUInstIdx: %d, fgEOS = TRUE, u4Status: %d, u4StmType: %d\r\n"
),
						u4Idx, u4Status,
						g_rDmxGauManager.arGAUInstance[u4Idx].u4StmType);
					GAU_DisableThreshold();
				}
				g_rDmxGauManager.arGAUInstance[u4Idx].fgEOS = fgEOS;
				g_rDmxGauManager.arGAUInstance[u4Idx].u4Status = u4Status;
			}
		}
	} else {
		for (u4Idx = 0; u4Idx < MAX_GAU_INSTANCE_CNT; ++u4Idx) {
			if ((g_rDmxGauManager.arGAUInstance[u4Idx].fgUsed) &&
				(g_rDmxGauManager.arGAUInstance[u4Idx].pvSptHdl == pvSptHdl)) {
				if (fgEOS &&
					((!g_rDmxGauManager.arGAUInstance[u4Idx].fgEOS) ||
					 (g_rDmxGauManager.arGAUInstance[u4Idx].u4Status != u4Status))) {
					DmxLogT(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
						TEXT(
"[GAU] --------->GAU_SetEOS - pvSptHdl: 0x%x, GAUInstIdx: %d, fgEOS = TRUE, u4Status: %d, u4StmType: %d\r\n"
),
						pvSptHdl, u4Idx, u4Status,
						g_rDmxGauManager.arGAUInstance[u4Idx].u4StmType);
					GAU_DisableThreshold();
				}
				g_rDmxGauManager.arGAUInstance[u4Idx].fgEOS = fgEOS;
				g_rDmxGauManager.arGAUInstance[u4Idx].u4Status = u4Status;
			}
		}
	}
}

void GAU_Init(void)
{
	u32 u4Idx = 0;

	mm_memset(&g_rDmxGauManager, 0, sizeof(GAU_MANAMENT_INFO_T));
	smp_mb();
	for (u4Idx = 0; u4Idx < MAX_GAU_INSTANCE_CNT; ++u4Idx) {
		g_rDmxGauManager.arGAUInstance[u4Idx].u4Handle = u4Idx;
		g_rDmxGauManager.arGAUInstance[u4Idx].fgUsed = FALSE;
		g_rDmxGauManager.arGAUInstance[u4Idx].u4ESHandle = ESM_INVALID_HANDLE;
		g_rDmxGauManager.arGAUInstance[u4Idx].hAUInEG = NULL;
		g_rDmxGauManager.arGAUInstance[u4Idx].u4AUConsumedCnt = 0;
		g_rDmxGauManager.arGAUInstance[u4Idx].fgEOS = FALSE;
		g_rDmxGauManager.arGAUInstance[u4Idx].u4Status = 0;
		g_rDmxGauManager.arGAUInstance[u4Idx].pvSptHdl = NULL;
		g_rDmxGauManager.arGAUInstance[u4Idx].u4StmType = DMX_INVALID_UINT32;
		g_rDmxGauManager.arGAUInstance[u4Idx].u4Threshold = 0;
		g_rDmxGauManager.arGAUInstance[u4Idx].fgReachThreshold = TRUE;
		g_rDmxGauManager.arGAUInstance[u4Idx].fgGetAUEnable = FALSE;
		g_rDmxGauManager.arGAUInstance[u4Idx].u4StmCodec  = DMX_INVALID_UINT32;
		g_rDmxGauManager.arGAUInstance[u4Idx].pvSptHdl = NULL;
		g_rDmxGauManager.arGAUInstance[u4Idx].pvDmxInst = NULL;
		g_rDmxGauManager.arGAUInstance[u4Idx].ptrMMRsvBufBase = 0;
		g_rDmxGauManager.arGAUInstance[u4Idx].u8DecSendBufMask = 0;
		g_rDmxGauManager.arGAUInstance[u4Idx].prRelEsmIOBuf = NULL;
		g_rDmxGauManager.arGAUInstance[u4Idx].prGetEsmIOBuf = NULL;
		GAU_Q_Init(&(g_rDmxGauManager.arGAUInstance[u4Idx].rGetAUQueue), u4Idx);
	}
	smp_mb();

	mm_memset(g_arLogAudioAUs, 0, sizeof(AU_AUDIO) * DMX_MAX_LOG_AUDIO_AU_CNT);
	smp_mb();

	g_rDmxGauManager.fgReachThreshold = TRUE;
}

void GAU_Uninit(void)
{
	MRESULT mrRet = RET_DMX_OK;
	u32 u4Idx = 0;

	for (u4Idx = 0; u4Idx < MAX_GAU_INSTANCE_CNT; u4Idx++) {
		mrRet = GAU_Disconnect(u4Idx);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
				TEXT("[GAU] %s failed in GAU_Disconnect(%d), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, u4Idx, mrRet);
		}

		GAU_Q_UnInit(&(g_rDmxGauManager.arGAUInstance[u4Idx].rGetAUQueue));

  		if (NULL != g_rDmxGauManager.arGAUInstance[u4Idx].prGetEsmIOBuf)
			DMX_FreeMemory(g_rDmxGauManager.arGAUInstance[u4Idx].prGetEsmIOBuf);
		smp_mb();
		g_rDmxGauManager.arGAUInstance[u4Idx].prGetEsmIOBuf = NULL;
    
		if (NULL != g_rDmxGauManager.arGAUInstance[u4Idx].prRelEsmIOBuf)
			DMX_FreeMemory(g_rDmxGauManager.arGAUInstance[u4Idx].prRelEsmIOBuf);
		smp_mb();
		g_rDmxGauManager.arGAUInstance[u4Idx].prRelEsmIOBuf = NULL;
 		smp_mb();
	}
	smp_mb();

	mm_memset(g_arLogAudioAUs, 0, sizeof(AU_AUDIO) * DMX_MAX_LOG_AUDIO_AU_CNT);
	smp_mb();

	g_rDmxGauManager.fgReachThreshold = TRUE;
}

void GAU_func_CB(ES_CBEVENT eEvent, void *pvData, void *pvPrivate)
{
	GAU_INSTANCE_T *ins = (GAU_INSTANCE_T *)pvPrivate;

	switch (eEvent) {
	case CBE_SYNC_READPTR: {
			if (NULL == ins) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
					TEXT("[GAU] %s line %d fail for no Private Data (eEvent: %d)!!!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, eEvent);
				return;
			}
			if (SPT_DATA_A == ins->u4StmType)
				GAU_Enable(ins->u4Handle, TRUE);
		}
		break;
	case CBE_AU_IN: {
			if (NULL == ins) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
					TEXT("[GAU] %s line %d fail for no Private Data (eEvent: %d)!!!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, eEvent);
				return;
			}
			smp_mb();
			GAU_CheckThreshold(ins->u4Handle);

			if (SPT_DATA_A == ins->u4StmType)
				GAU_Enable(ins->u4Handle, TRUE);

			if (OSR_OK != x_ev_group_set_event((uintptr_t)ins->hAUInEG, GAU_EV_AU_IN, X_EV_OP_OR)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
					TEXT("[GAU] %s(u4Handle: 0x%x) fail in Set Event GAU_EV_REACH_THRESHOLD!!!\r\n"),
					DMX_FUNC_NAME, ins->u4Handle);
				return;
			}
		}
		break;

	case CBE_FIFO_FLUSH: {
			if (NULL == ins) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
					TEXT("[GAU] %s line %d fail for no Private Data (eEvent: %d)!!!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, eEvent);
				return;
			}
			DmxLogD(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
				TEXT("[GAU] %s line %d -- SEND GAU_EV_FIFO_FLUSH (StmType: %s)!!!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				((ins->u4StmType < MAX_SPT_DATA_TYPE_CNT) ?
g_aszSptDataTypeName[ins->u4StmType] : TEXT("UNKNOWN")));
			smp_mb();
			GAU_ResetEvent(ins->u4Handle, GAU_EV_AU_IN);
			smp_mb();
			GAU_Q_Flush(&(g_rDmxGauManager.arGAUInstance[ins->u4Handle].rGetAUQueue));
			if (OSR_OK != x_ev_group_set_event((uintptr_t)ins->hAUInEG, GAU_EV_FIFO_FLUSH, X_EV_OP_OR)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
					TEXT(
"[GAU] %s(u4Handle: 0x%x) fail in Set Event GAU_EV_REACH_THRESHOLD!!!\r\n"
),
					DMX_FUNC_NAME, ins->u4Handle);
				return;
			}
			ins->u4StmCodec = DMX_INVALID_UINT32;
		}
		break;

	default:
		break;
	}
}

MRESULT GAU_Connect(GAU_CONNECT_PARAM_T *prParam)
{
	u32	u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;
	DMX_SPT_INST_T *prSpt = NULL;

#ifdef __linux__
	  char tbuf[32];
#endif

	if (NULL == prParam)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	if ((NULL == prParam) ||
		(NULL == prParam->pvSptHdl) ||
		(0 == prParam->u4QueueElemCnt)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s fail for invalid param(pvSptHdl: 0x%x, u4QueueElemCnt: %d)\r\n"),
			DMX_FUNC_NAME, prParam->pvSptHdl, prParam->u4QueueElemCnt);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	for (u4Idx = 0; u4Idx < MAX_GAU_INSTANCE_CNT; ++u4Idx) {
		if (!(g_rDmxGauManager.arGAUInstance[u4Idx].fgUsed))
			break;
	}
	smp_mb();

	if (u4Idx >= MAX_GAU_INSTANCE_CNT) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s exceed GAU instance count(u4StmType: %d, u4StmUID: %d)\r\n"),
			DMX_FUNC_NAME, prParam->u4StmType, prParam->u4StmUID);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}
	smp_mb();

	mrRet = GAU_Q_Create(&(g_rDmxGauManager.arGAUInstance[u4Idx].rGetAUQueue), prParam->u4QueueElemCnt);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s fail in GAU_Q_Create, u4StmUID: 0x%x, ")
			TEXT("u4StmType: 0x%x, u4QueueElemCnt: %d, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, prParam->u4StmUID, prParam->u4StmType,
			prParam->u4QueueElemCnt, mrRet);
		goto CONNECTERR;
		MM_RETURN(mrRet);
	}

	mrRet = ESM_Create(prParam->pvSptHdl,
	   prParam->u4StmType, prParam->u4StmUID,
	   prParam->u8DecSendBufMask,
		&(g_rDmxGauManager.arGAUInstance[u4Idx].u4ESHandle));
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s fail in ESM_Create, u4StmUID: 0x%x, u4StmType: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, prParam->u4StmUID, prParam->u4StmType, mrRet);
		goto CONNECTERR;
	}
	smp_mb();

	if (NULL == g_rDmxGauManager.arGAUInstance[u4Idx].prGetEsmIOBuf) {
		DMX_NewMemory(sizeof(ESM_IO_BUF_INFO),
			g_rDmxGauManager.arGAUInstance[u4Idx].prGetEsmIOBuf);
		if (NULL == g_rDmxGauManager.arGAUInstance[u4Idx].prGetEsmIOBuf) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
				TEXT("[GAU] %s line %d fail in alloc ESM_IO_BUF_INFO\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			mrRet = RET_DMX_NO_MEM;
			goto CONNECTERR;
		}
	}
	smp_mb();
	dmx_memset(g_rDmxGauManager.arGAUInstance[u4Idx].prGetEsmIOBuf, 0,
		sizeof(ESM_IO_BUF_INFO));
	smp_mb();

	if (NULL == g_rDmxGauManager.arGAUInstance[u4Idx].prRelEsmIOBuf) {
		DMX_NewMemory(sizeof(ESM_IO_BUF_INFO),
			g_rDmxGauManager.arGAUInstance[u4Idx].prRelEsmIOBuf);
		if (NULL == g_rDmxGauManager.arGAUInstance[u4Idx].prRelEsmIOBuf) {
 			DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
 				TEXT("[GAU] %s line %d fail in alloc ESM_IO_BUF_INFO\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			mrRet = RET_DMX_NO_MEM;
			goto CONNECTERR;
		}
	}
	smp_mb();
	dmx_memset(g_rDmxGauManager.arGAUInstance[u4Idx].prRelEsmIOBuf, 0,
		sizeof(ESM_IO_BUF_INFO));
	smp_mb();

#ifdef __linux__
	snprintf(tbuf, sizeof(tbuf), "_gau%d", (int)u4Idx);
	if (OSR_OK !=  x_ev_group_create((uintptr_t *)&(g_rDmxGauManager.arGAUInstance[u4Idx].hAUInEG), tbuf, 0)) {
#else
	if (OSR_OK != x_ev_group_create(&(g_rDmxGauManager.arGAUInstance[u4Idx].hAUInEG), NULL, 0)) {
#endif
		DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s fail in x_ev_group_create(Handle: %d)\r\n"),
			DMX_FUNC_NAME, u4Idx);
		mrRet = RET_DMX_OS_OPERA_FAIL;
		goto CONNECTERR;
	}
	smp_mb();

	if (0 == g_rDmxGauManager.arGAUInstance[u4Idx].rSelfFifoInfo.ptrFifoVSa) {
		u32 u4SelfFifoSz = 0;

		switch (prParam->u4StmType) {
		case SPT_DATA_A:
			if (0 != prParam->u8DecSendBufMask)
				u4SelfFifoSz = DMX_AUD_INIT_SELFFIFO_SZ;
			break;
		case SPT_DATA_SP:
			u4SelfFifoSz = DMX_SP_INIT_SELFFIFO_SZ;
			break;
		case SPT_DATA_V:
			if (0 != prParam->u8DecSendBufMask)
			u4SelfFifoSz = DMX_VID_INIT_SELFFIFO_SZ;
			break;
		default:
			break;
		}
		smp_mb();

		if (u4SelfFifoSz > 0) {
			#ifdef __linux__
			DMX_NewHwAlignMemory(u4SelfFifoSz, (1U << 12U),
			    g_rDmxGauManager.arGAUInstance[u4Idx].rSelfFifoInfo.ptrFifoVSa);
			#else
			DMX_NewHwAlignMemory(u4SelfFifoSz, (1U << 12U),
			    (void *)(g_rDmxGauManager.arGAUInstance[u4Idx].rSelfFifoInfo.ptrFifoVSa));
			#endif /* #ifdef __linux__ */
			smp_mb();

			if (0 == g_rDmxGauManager.arGAUInstance[u4Idx].rSelfFifoInfo.ptrFifoVSa) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
					TEXT("[GAU] %s line %d fail in alloc self fifo for Steam(%s)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					(((prParam->u4StmType) < MAX_SPT_DATA_TYPE_CNT) ?
					g_aszSptDataTypeName[prParam->u4StmType] : TEXT("UNKNOWN")));
				mrRet = RET_DMX_NO_MEM;
				goto CONNECTERR;
			}
			smp_mb();
			g_rDmxGauManager.arGAUInstance[u4Idx].rSelfFifoInfo.u4FifoSz = u4SelfFifoSz;
			g_rDmxGauManager.arGAUInstance[u4Idx].rSelfFifoInfo.ptrFifoPSa =
				DMX_PHYSICAL(g_rDmxGauManager.arGAUInstance[u4Idx].rSelfFifoInfo.ptrFifoVSa);
			g_rDmxGauManager.arGAUInstance[u4Idx].rSelfFifoInfo.ptrFifoPEa =
				g_rDmxGauManager.arGAUInstance[u4Idx].rSelfFifoInfo.ptrFifoPSa +
				g_rDmxGauManager.arGAUInstance[u4Idx].rSelfFifoInfo.u4FifoSz;
			g_rDmxGauManager.arGAUInstance[u4Idx].rSelfFifoInfo.ptrUserVirSA = 0;
			g_rDmxGauManager.arGAUInstance[u4Idx].rSelfFifoInfo.ptrMMRsvBufBase = 
				(uintptr_t)OSE_GetMMReservedMemStartAddr();
			if (0 == g_rDmxGauManager.arGAUInstance[u4Idx].rSelfFifoInfo.ptrMMRsvBufBase) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
					TEXT("[GAU] %s line %d fail in OSE_GetMMReservedMemStartAddr for Steam(%s)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					(((prParam->u4StmType) < MAX_SPT_DATA_TYPE_CNT) ?
					g_aszSptDataTypeName[prParam->u4StmType] : TEXT("UNKNOWN")));
				mrRet = RET_DMX_NO_MEM;
				goto CONNECTERR;
			}			
			g_rDmxGauManager.arGAUInstance[u4Idx].rSelfFifoInfo.hCallProcess = NULL;
			DmxLogD(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
				TEXT("[GAU] %s (%s) FifoVSa: 0x%x, FifoPSa: 0x%p, FifoPEa: 0x%p, FifoSz: 0x%x\r\n"),
				DMX_FUNC_NAME,
				(((prParam->u4StmType) < MAX_SPT_DATA_TYPE_CNT) ?
				g_aszSptDataTypeName[prParam->u4StmType] : TEXT("UNKNOWN")),
				g_rDmxGauManager.arGAUInstance[u4Idx].rSelfFifoInfo.ptrFifoVSa,
				g_rDmxGauManager.arGAUInstance[u4Idx].rSelfFifoInfo.ptrFifoPSa,
				g_rDmxGauManager.arGAUInstance[u4Idx].rSelfFifoInfo.ptrFifoPEa,
				g_rDmxGauManager.arGAUInstance[u4Idx].rSelfFifoInfo.u4FifoSz);
		}
	}
	smp_mb();

	g_rDmxGauManager.arGAUInstance[u4Idx].u4Handle	  = u4Idx;
	g_rDmxGauManager.arGAUInstance[u4Idx].fgUsed	  = TRUE;
	g_rDmxGauManager.arGAUInstance[u4Idx].pvSptHdl		  = prParam->pvSptHdl;
	g_rDmxGauManager.arGAUInstance[u4Idx].u4StmType   = prParam->u4StmType;

	g_rDmxGauManager.arGAUInstance[u4Idx].u8DecSendBufMask = prParam->u8DecSendBufMask;
	g_rDmxGauManager.arGAUInstance[u4Idx].u4StmCodec  = DMX_INVALID_UINT32;
	prSpt = (DMX_SPT_INST_T *)prParam->pvSptHdl;
	if (NULL != prSpt) {
		g_rDmxGauManager.arGAUInstance[u4Idx].pvDmxInst   = prSpt->pvDmxInst;
	}
	DmxLogT(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
		TEXT("[GAU] GAUInstance(%d) -- Stm(%s) Software Decode Codec Mask = "DMX_UINT64_16U_LOGSTR"\r\n"),
		u4Idx, (((prParam->u4StmType) < MAX_SPT_DATA_TYPE_CNT) ?
		g_aszSptDataTypeName[prParam->u4StmType] : TEXT("UNKNOWN")),
		DMX_UINT64_16U_LOG_H(prParam->u8DecSendBufMask),
		DMX_UINT64_16U_LOG_L(prParam->u8DecSendBufMask));

	g_rDmxGauManager.arGAUInstance[u4Idx].u4Threshold = 0;
	g_rDmxGauManager.arGAUInstance[u4Idx].fgReachThreshold = TRUE;

	g_rDmxGauManager.arGAUInstance[u4Idx].u4AUConsumedCnt = 0;

	g_rDmxGauManager.arGAUInstance[u4Idx].fgGetAUEnable = FALSE;
	smp_mb();

	g_rDmxGauManager.arGAUInstance[u4Idx].ptrMMRsvBufBase = (uintptr_t)OSE_GetMMReservedMemStartAddr();
	if (0 == g_rDmxGauManager.arGAUInstance[u4Idx].ptrMMRsvBufBase) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d fail in OSE_GetMMReservedMemStartAddr for Steam(%s)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			(((prParam->u4StmType) < MAX_SPT_DATA_TYPE_CNT) ?
		g_aszSptDataTypeName[prParam->u4StmType] : TEXT("UNKNOWN")));
		mrRet = RET_DMX_NO_MEM;
		goto CONNECTERR;
	}

	if (SPT_DATA_A == prParam->u4StmType)
		mm_memset(g_arLogAudioAUs, 0, sizeof(AU_AUDIO) * DMX_MAX_LOG_AUDIO_AU_CNT);
	smp_mb();

	ESM_RegistDecoderCB(g_rDmxGauManager.arGAUInstance[u4Idx].u4ESHandle,
		GAU_func_CB,
		&(g_rDmxGauManager.arGAUInstance[u4Idx]));
	smp_mb();

	*(prParam->pu4Handle) = u4Idx;

	MM_RETURN(RET_DMX_OK);

CONNECTERR:

	if (NULL != g_rDmxGauManager.arGAUInstance[u4Idx].hAUInEG)
		x_ev_group_delete((uintptr_t)g_rDmxGauManager.arGAUInstance[u4Idx].hAUInEG);
	g_rDmxGauManager.arGAUInstance[u4Idx].hAUInEG = NULL;

	if (ESM_INVALID_HANDLE != g_rDmxGauManager.arGAUInstance[u4Idx].u4ESHandle)
		ESM_Destroy(g_rDmxGauManager.arGAUInstance[u4Idx].u4ESHandle);
	g_rDmxGauManager.arGAUInstance[u4Idx].u4ESHandle = ESM_INVALID_HANDLE;

	GAU_Q_Release(&(g_rDmxGauManager.arGAUInstance[u4Idx].rGetAUQueue));

	MM_RETURN(mrRet);
}

MRESULT GAU_Disconnect(u32 u4Handle)
{
	MRESULT mrRet = RET_DMX_OK;

	if (MAX_GAU_INSTANCE_CNT <= u4Handle)
		MM_RETURN(RET_DMX_PARAM_WRONG);
	smp_mb();

	if (!(g_rDmxGauManager.arGAUInstance[u4Handle].fgUsed))
		MM_RETURN(RET_DMX_OK);
	smp_mb();

	if ((NULL != g_rDmxGauManager.arGAUInstance[u4Handle].hAUInEG) &&
		(OSR_OK != x_ev_group_delete((uintptr_t)g_rDmxGauManager.arGAUInstance[u4Handle].hAUInEG))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s fail in x_ev_group_delete, Handle: 0x%x\r\n"),
			DMX_FUNC_NAME, u4Handle);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}
	smp_mb();

	if (ESM_INVALID_HANDLE != g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle) {
		mrRet = ESM_Destroy(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle);
		if (RET_DMX_OK != mrRet) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s fail in ESM_Destroy, Handle: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, u4Handle, mrRet);
			MM_RETURN(mrRet);
		}
	}
	smp_mb();

	if (SPT_DATA_A == g_rDmxGauManager.arGAUInstance[u4Handle].u4StmType)
		mm_memset(g_arLogAudioAUs, 0, sizeof(AU_AUDIO) * DMX_MAX_LOG_AUDIO_AU_CNT);

	#if DMX_PFM_TEST
	switch (g_rDmxGauManager.arGAUInstance[u4Handle].u4StmType) {
	case SPT_DATA_V:
		DmxPfmStmGetAUEnd(SPT_DATA_V);
		DmxLogT(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d -- Video GetAU (consumems: %d ms, getcnt: %d, getsuccess: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			g_rPsrPfm.rVideo.u8GetAUTime,
			g_rPsrPfm.rVideo.u4GetAUCnt,
			g_rPsrPfm.rVideo.u4GetAURealCnt);
		DmxLogT(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d -- Video GetAU (FifoFullCnt: %d, AUTableFullCnt: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, g_rPsrPfm.rVideo.u4FifoFullCnt,
			g_rPsrPfm.rVideo.u4ESTableFullCnt);
		break;
	case SPT_DATA_A:
		DmxPfmStmGetAUEnd(SPT_DATA_A);
		DmxLogT(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d -- Audio GetAU (consumems: %d ms, getcnt: %d, getsuccess: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			g_rPsrPfm.rAudio.u8GetAUTime,
			g_rPsrPfm.rAudio.u4GetAUCnt,
			g_rPsrPfm.rAudio.u4GetAURealCnt);
		DmxLogT(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d -- Audio GetAU (FifoFullCnt: %d, AUTableFullCnt: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, g_rPsrPfm.rAudio.u4FifoFullCnt,
			g_rPsrPfm.rAudio.u4ESTableFullCnt);
		break;
	case SPT_DATA_SP:
		DmxPfmStmGetAUEnd(SPT_DATA_SP);
		DmxLogT(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d -- SP GetAU (consumems: %d ms, getcnt: %d, getsuccess: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			g_rPsrPfm.rSP.u8GetAUTime,
			g_rPsrPfm.rSP.u4GetAUCnt,
			g_rPsrPfm.rSP.u4GetAURealCnt);
		DmxLogT(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d -- SP GetAU (FifoFullCnt: %d, AUTableFullCnt: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, g_rPsrPfm.rSP.u4FifoFullCnt,
			g_rPsrPfm.rSP.u4ESTableFullCnt);
		break;
	default:
		break;
	}
	#endif /* DMX_PFM_TEST */

	smp_mb();

	mrRet = GAU_Q_Release(&(g_rDmxGauManager.arGAUInstance[u4Handle].rGetAUQueue));
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU]  %s line %d fail in GAU_Q_Release(%s), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			(((g_rDmxGauManager.arGAUInstance[u4Handle].u4StmType) < MAX_SPT_DATA_TYPE_CNT) ?
g_aszSptDataTypeName[g_rDmxGauManager.arGAUInstance[u4Handle].u4StmType] : TEXT("UNKNOWN")),
			mrRet);
	}

	g_rDmxGauManager.arGAUInstance[u4Handle].fgUsed = FALSE;
	g_rDmxGauManager.arGAUInstance[u4Handle].u4Handle = u4Handle;
	g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle = ESM_INVALID_HANDLE;
	g_rDmxGauManager.arGAUInstance[u4Handle].hAUInEG = NULL;
	g_rDmxGauManager.arGAUInstance[u4Handle].u4AUConsumedCnt = 0;
	g_rDmxGauManager.arGAUInstance[u4Handle].fgEOS = FALSE;
	g_rDmxGauManager.arGAUInstance[u4Handle].u4Status = 0;
	g_rDmxGauManager.arGAUInstance[u4Handle].pvSptHdl = NULL;
	g_rDmxGauManager.arGAUInstance[u4Handle].u4StmType = DMX_INVALID_UINT32;
	g_rDmxGauManager.arGAUInstance[u4Handle].pvDmxInst	 = NULL;
	g_rDmxGauManager.arGAUInstance[u4Handle].u4Threshold = 0;
	g_rDmxGauManager.arGAUInstance[u4Handle].fgReachThreshold = TRUE;
	g_rDmxGauManager.arGAUInstance[u4Handle].fgGetAUEnable = FALSE;
	g_rDmxGauManager.arGAUInstance[u4Handle].u4StmCodec  = DMX_INVALID_UINT32;
	g_rDmxGauManager.arGAUInstance[u4Handle].u8DecSendBufMask = 0;
	smp_mb();

	if (NULL != g_rDmxGauManager.arGAUInstance[u4Handle].prRelEsmIOBuf)
		dmx_memset(g_rDmxGauManager.arGAUInstance[u4Handle].prRelEsmIOBuf, 0,
			sizeof(ESM_IO_BUF_INFO));

	if (NULL != g_rDmxGauManager.arGAUInstance[u4Handle].prGetEsmIOBuf)
		dmx_memset(g_rDmxGauManager.arGAUInstance[u4Handle].prGetEsmIOBuf, 0,
			sizeof(ESM_IO_BUF_INFO));

	if (0 != g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrUserVirSA) {
		g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrUserVirSA = 0;
		g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.hCallProcess = NULL;
	}
	smp_mb();

	if (0 != g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa)
		DMX_FreeHwMemory((void *)(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa));
	smp_mb();

	mm_memset(&(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo), 0, sizeof(GAU_SELF_USE_FIFO_T));

	MM_RETURN(RET_DMX_OK);
}	

MRESULT GAU_GetVideoAU(
	u32			u4Handle,
	ESM_IO_BUF_INFO *pEsmIOBufInfo,
	u32			u4RdIdx,
	void			*pvAU)
{
	AU_VPic *prVPicAU = (AU_VPic *)pvAU;
	u32	u4Codec = DMX_INVALID_UINT32;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pEsmIOBufInfo) ||
		(NULL == pvAU)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_VID]) {
		DmxDumpVFrame(prVPicAU, pEsmIOBufInfo->ptrFifoSPAddr,
			pEsmIOBufInfo->ptrFifoEPAddr);
	}

	if (DMX_INVALID_UINT32 == g_rDmxGauManager.arGAUInstance[u4Handle].u4StmCodec) /* liang luo sw */ {
		mrRet = ESM_GetStmCodec(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
			&(g_rDmxGauManager.arGAUInstance[u4Handle].u4StmCodec));
		if (DMX_SUCCEED(mrRet))
			u4Codec = g_rDmxGauManager.arGAUInstance[u4Handle].u4StmCodec;
		DmxLogT(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
			TEXT("[GAU] Video Codec = %d\r\n"), u4Codec);
	} else {
		u4Codec = g_rDmxGauManager.arGAUInstance[u4Handle].u4StmCodec;
	}

	pEsmIOBufInfo->rAU.rVPicAU = *(AU_VPic *)pvAU;

	if (0 == ((AU_VPic *)pvAU)->rAUInfo.rInfo.ptrEAddr) {
		uintptr_t ptrFifoSa = 0;
		uintptr_t ptrFifoEa = 0;
		uintptr_t ptrFifoRp = 0;
		uintptr_t ptrFifoWp = 0;
		u32 u4AUTableTotalCnt = 0;
		u32 u4AUTableWrIdx = 0;
		u32 u4AUTableRdIdx = 0;

		mrRet = ESM_AUTableGetTotalCount(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
			&u4AUTableTotalCnt);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
				TEXT("[GAU] %s line %d fail in ESM_AUTableGetTotalCount(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
			MM_RETURN(mrRet);
		}
		mrRet = ESM_FifoGetWrPtr(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &ptrFifoWp);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
				TEXT("[GAU] %s line %d fail in ESM_FifoGetWrPtr(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
			MM_RETURN(mrRet);
		}

		mrRet = ESM_FifoGetRdPtr(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &ptrFifoRp);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
				TEXT("[GAU] %s line %d fail in ESM_FifoGetRdPtr(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
			MM_RETURN(mrRet);
		}
		mrRet = ESM_FifoGetSA(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &ptrFifoSa);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
				TEXT("[GAU] %s line %d fail in ESM_FifoGetSA(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
			MM_RETURN(mrRet);
		}

		mrRet = ESM_FifoGetEA(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &ptrFifoEa);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
				TEXT("[GAU] %s line %d fail in ESM_FifoGetEA(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
			MM_RETURN(mrRet);
		}

		mrRet = ESM_AUTableGetWrIdx(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &u4AUTableWrIdx);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
				TEXT("[GAU] %s line %d fail in ESM_AUTableGetWrIdx, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		}

		mrRet = ESM_AUTableGetRdIdx(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &u4AUTableRdIdx);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
				TEXT("[GAU] %s line %d fail in ESM_AUTableGetRdIdx, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		}

		DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
			TEXT("[GAU] %s line %d FATAL ERROR -- Video ptrEAddr == 0\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
			TEXT("[GAU] %s line %d FATAL ERROR -- ptrSAddr:0x%p, u4VType == 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			((AU_VPic *)pvAU)->rAUInfo.rInfo.ptrSAddr,
			((AU_VPic *)pvAU)->rAUInfo.rInfo.u4VType);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
			TEXT("[GAU] %s line %d FATAL ERROR -- FifoSa:0x%p, FifoEa:0x%p, FifoRp:0x%p, FifoWp: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			ptrFifoSa, ptrFifoEa, ptrFifoRp, ptrFifoWp);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
			TEXT("[GAU] %s line %d FATAL ERROR -- AUTable TotalCnt:0x%x, WrIdx:0x%x, RdIdx:0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			u4AUTableTotalCnt, u4AUTableWrIdx, u4AUTableRdIdx);
	}

	if ((pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrSAddr < (uintptr_t)pEsmIOBufInfo->ptrFifoSPAddr) ||
		(pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrSAddr > (uintptr_t)pEsmIOBufInfo->ptrFifoEPAddr) ||
		(pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrEAddr < (uintptr_t)pEsmIOBufInfo->ptrFifoSPAddr) ||
		(pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrEAddr > (uintptr_t)pEsmIOBufInfo->ptrFifoEPAddr)) {
		uintptr_t ptrFifoSa = 0, ptrFifoEa = 0; 
		uintptr_t ptrFifoRp = 0, ptrFifoWp = 0;
		u32 u4AUTableTotalCnt = 0, u4AUTableWrIdx = 0, u4AUTableRdIdx = 0;

		mrRet = ESM_AUTableGetTotalCount(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
			&u4AUTableTotalCnt);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
				TEXT(
"[GAU] %s line %d fail in ESM_AUTableGetTotalCount(u4Handle: 0x%x), mrRet: 0x%x\r\n"
),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
			MM_RETURN(mrRet);
		}
		mrRet = ESM_FifoGetWrPtr(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &ptrFifoWp);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
				TEXT("[GAU] %s line %d fail in ESM_FifoGetWrPtr(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
			MM_RETURN(mrRet);
		}

		mrRet = ESM_FifoGetRdPtr(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &ptrFifoRp);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
				TEXT("[GAU] %s line %d fail in ESM_FifoGetRdPtr(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
			MM_RETURN(mrRet);
		}

		mrRet = ESM_FifoGetSA(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &ptrFifoSa);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
				TEXT("[GAU] %s line %d fail in ESM_FifoGetSA(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
			MM_RETURN(mrRet);
		}

		mrRet = ESM_FifoGetEA(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &ptrFifoEa);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
				TEXT("[GAU] %s line %d fail in ESM_FifoGetEA(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
			MM_RETURN(mrRet);
		}

		mrRet = ESM_AUTableGetWrIdx(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &u4AUTableWrIdx);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
				TEXT("[GAU] %s line %d fail in ESM_AUTableGetWrIdx, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		}

		mrRet = ESM_AUTableGetRdIdx(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &u4AUTableRdIdx);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[GAU] %s line %d fail in ESM_AUTableGetRdIdx, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		}

		DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
			TEXT("[GAU] %s line %d FATAL ERROR -- u4VType (0x%x), AU's SA(0x%p) or EA(0x%p) beyond Fifo\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			((AU_VPic *)pvAU)->rAUInfo.rInfo.u4VType,
			pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrSAddr,
			pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrEAddr);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
			TEXT("[GAU] %s line %d -- FifoSa == 0x%p, FifoEa == 0x%p, FifoRp == 0x%p, FifoWp == 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			ptrFifoSa, ptrFifoEa, ptrFifoRp, ptrFifoWp);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
			TEXT("[GAU] %s line %d -- AUTable TotalCnt == 0x%x, WrIdx == 0x%x, RdIdx == 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			u4AUTableTotalCnt, u4AUTableWrIdx, u4AUTableRdIdx);
	}

	if ((DMX_INVALID_UINT32 != u4Codec) &&
		((u64)0 != (g_rDmxGauManager.arGAUInstance[u4Handle].u8DecSendBufMask & (((u64)1) << u4Codec)))) {
		if (pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrSAddr >
			pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrEAddr) {
			u32 u4SzToEnd = 0;
			u32 u4DataSz = DMX_DATASIZE(pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrSAddr,
				pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrEAddr,
				((uintptr_t)pEsmIOBufInfo->ptrFifoEPAddr - (uintptr_t)pEsmIOBufInfo->ptrFifoSPAddr));

			/* Encounter Circular */
			if (u4DataSz > g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.u4FifoSz) {
				u32 u4FifoSz = (u4DataSz + ((u32)1 << 12U)) / ((u32)1 << 12U) * ((u32)1 << 12U);

				if (0 != g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrUserVirSA) {
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrUserVirSA = 0;
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.hCallProcess = NULL;
				}

				if (0 != g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa) {
					DMX_FreeHwMemory((void *)(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa));
					mm_memset(&(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo),
						0, sizeof(GAU_SELF_USE_FIFO_T));
				}

				#ifdef __linux__
				DMX_NewHwAlignMemory(u4FifoSz, ((u32)1 << 12U), g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa);
				#else
				DMX_NewHwAlignMemory(u4FifoSz, ((u32)1 << 12U), (void *)(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa));
				#endif /* #ifdef __linux__ */
				if (0 == g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU_V,
						TEXT("[GAU] %s line %d fail in DMX_NewHwAlignMemory(Video)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
					pEsmIOBufInfo->u4Status = GAU_E_MEMORY;
					MM_RETURN(RET_DMX_NO_MEM);
				}

				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.u4FifoSz = u4FifoSz;
				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPSa =
					DMX_PHYSICAL(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa);
				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPEa =
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPSa + u4FifoSz;

				DmxLogD(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT("[GAU] %s (Video) Realloc -- FifoVSa: 0x%p, FifoPSa: 0x%p, FifoPEa: 0x%p, FifoSz: 0x%x\r\n"),
					DMX_FUNC_NAME,
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa,
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPSa,
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPEa,
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.u4FifoSz);
			}

			if (0 == g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrUserVirSA) {

				/*get the relative addr , mtk40505 */
				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrUserVirSA =
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa
					- g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrMMRsvBufBase;

				DmxLogD(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT(
					"[GAU] %s (SP/ST) 4 -- FifoVSa: 0x%x, FifoPSa: 0x%p, FifoPEa: 0x%p, FifoSz: 0x%x, UVSa: 0x%p\r\n"
					),
					DMX_FUNC_NAME,
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa,
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPSa,
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPEa,
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.u4FifoSz,
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrUserVirSA);
			}

			u4SzToEnd = pEsmIOBufInfo->ptrFifoEPAddr - pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrSAddr;
			dmx_memcpy((u8 *)(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa),
				(u8 *)(pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrSAddr), u4SzToEnd);
			dmx_memcpy((u8 *)(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa)
					+ u4SzToEnd, (u8 *)pEsmIOBufInfo->ptrFifoSPAddr, u4DataSz - u4SzToEnd);

			pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrSVAddr =
				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa;
			pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrEVAddr =
				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa + u4DataSz;

			pEsmIOBufInfo->ptrFifoSVAddr =
				(__u64)g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa;
			pEsmIOBufInfo->ptrFifoEVAddr =
				(__u64)(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa +
				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.u4FifoSz);

			pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrSAddr =
				pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrSVAddr
				- g_rDmxGauManager.arGAUInstance[u4Handle].ptrMMRsvBufBase;

			pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrEAddr =
				pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrSAddr + u4DataSz;

			pEsmIOBufInfo->ptrFifoSPAddr =
				(__u64)(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa -
				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrMMRsvBufBase);
			pEsmIOBufInfo->ptrFifoEPAddr =
				(__u64)(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa +
				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.u4FifoSz -
				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrMMRsvBufBase);
		} else {
			pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrSVAddr
					= pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrSAddr;
			pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrEVAddr
					= pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrEAddr;

			pEsmIOBufInfo->ptrFifoSVAddr = (__u64)pEsmIOBufInfo->ptrFifoSPAddr;
			pEsmIOBufInfo->ptrFifoEVAddr = (__u64)pEsmIOBufInfo->ptrFifoEPAddr;

			pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrSAddr =
				pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrSAddr
					- g_rDmxGauManager.arGAUInstance[u4Handle].ptrMMRsvBufBase;

			pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrEAddr =
				pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrEAddr
					- g_rDmxGauManager.arGAUInstance[u4Handle].ptrMMRsvBufBase;

			pEsmIOBufInfo->ptrFifoSPAddr = pEsmIOBufInfo->ptrFifoSVAddr - 
                g_rDmxGauManager.arGAUInstance[u4Handle].ptrMMRsvBufBase;
			pEsmIOBufInfo->ptrFifoEPAddr = pEsmIOBufInfo->ptrFifoEVAddr - 
				g_rDmxGauManager.arGAUInstance[u4Handle].ptrMMRsvBufBase;
		}
	} else {
		pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrSVAddr
				= pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrSAddr;
		pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrEVAddr
				= pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrEAddr;
		pEsmIOBufInfo->ptrFifoSVAddr = (__u64)pEsmIOBufInfo->ptrFifoSPAddr;
		pEsmIOBufInfo->ptrFifoEVAddr = (__u64)pEsmIOBufInfo->ptrFifoEPAddr;

		pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrSAddr
				-= pEsmIOBufInfo->ptrFifoSPAddr;
		pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrEAddr
				-= pEsmIOBufInfo->ptrFifoSPAddr;

		pEsmIOBufInfo->ptrFifoSPAddr = (__u64)DMX_PHYSICAL(pEsmIOBufInfo->ptrFifoSPAddr);
		pEsmIOBufInfo->ptrFifoEPAddr = (__u64)DMX_PHYSICAL(pEsmIOBufInfo->ptrFifoEPAddr);
	}

	if (AU_DATA == pEsmIOBufInfo->rAU.rVPicAU.eAuType) {
		pEsmIOBufInfo->u4Status = GAU_S_OK;
		if (pEsmIOBufInfo->rAU.rVPicAU.fgIBCSent) {
			DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_V,
				TEXT("[GAU] %s (Video) Discontinuous ---> eDataType: %d, AUIdx(%d), ")
				TEXT("Pts("	DMX_PTS_LOGSTR"), VType(0x%x), AUSa(0x%p), AUEa(0x%p)\r\n"),
				DMX_FUNC_NAME,
				((AU_VPic *)pvAU)->eAuType, u4RdIdx,
				DMX_PTS_LOG_MS(((AU_VPic *)pvAU)->rAUInfo.rInfo.u8Pts),
				DMX_PTS_LOG_PTS(((AU_VPic *)pvAU)->rAUInfo.rInfo.u8Pts),
				((AU_VPic *)pvAU)->rAUInfo.rInfo.u4VType,
				((AU_VPic *)pvAU)->rAUInfo.rInfo.ptrSAddr,
				((AU_VPic *)pvAU)->rAUInfo.rInfo.ptrEAddr);

			pEsmIOBufInfo->rAU.rVPicAU.fgIBCSent = FALSE;
			pEsmIOBufInfo->u4Status = GAU_S_DISCONTINUOUS;
		}
	}

	if (g_rDmxCliMan.fgDumpFlow &&
		(DMX_SUCCEED(mrRet))) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = g_rDmxGauManager.arGAUInstance[u4Handle].pvSptHdl;
		rOperInfo.unFlow.rGetAU.u4AUIdx = u4RdIdx;
		rOperInfo.unFlow.rGetAU.u4StmType = SPT_DATA_V;
		DmxDumpFlow(DMX_OPER_GETAU, &rOperInfo);
	}

	DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_V,
		TEXT("[GAU] %s (Video) Success ---> eDataType: %d, AUIdx(%d), ")
		TEXT("Pts("DMX_PTS_LOGSTR"), VType(0x%x), AUSa(0x%p), AUEa(0x%p)\r\n"),
		DMX_FUNC_NAME,
		((AU_VPic *)pvAU)->eAuType, u4RdIdx,
		DMX_PTS_LOG_MS(((AU_VPic *)pvAU)->rAUInfo.rInfo.u8Pts),
		DMX_PTS_LOG_PTS(((AU_VPic *)pvAU)->rAUInfo.rInfo.u8Pts),
		((AU_VPic *)pvAU)->rAUInfo.rInfo.u4VType,
		((AU_VPic *)pvAU)->rAUInfo.rInfo.ptrSAddr,
		((AU_VPic *)pvAU)->rAUInfo.rInfo.ptrEAddr);

	#if DMX_PFM_TEST
	g_rPsrPfm.rVideo.u4GetAURealCnt++;
	#endif /* DMX_PFM_TEST */

	MM_RETURN(mrRet);
}

MRESULT GAU_GetAudioAU(
	u32			u4Handle,
	ESM_IO_BUF_INFO *pEsmIOBufInfo,
	void			*pvExtAUInfo,
	u32			u4RdIdx,
	void			*pvAU)
{
	MRESULT mrRet = RET_DMX_OK;
	u32	u4Codec = DMX_INVALID_UINT32;

	if ((NULL == pEsmIOBufInfo) ||
		(NULL == pvAU)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (DMX_INVALID_UINT32 == g_rDmxGauManager.arGAUInstance[u4Handle].u4StmCodec) {
		mrRet = ESM_GetStmCodec(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
			&(g_rDmxGauManager.arGAUInstance[u4Handle].u4StmCodec));
		if (DMX_SUCCEED(mrRet))
			u4Codec = g_rDmxGauManager.arGAUInstance[u4Handle].u4StmCodec;
		DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_A,
			TEXT("[GAU] Audio Codec = %d\r\n"), u4Codec);
	} else {
		u4Codec = g_rDmxGauManager.arGAUInstance[u4Handle].u4StmCodec;
	}

	pEsmIOBufInfo->rAU.rAudioAU = *(AU_AUDIO *)pvAU;

	if (0 == pEsmIOBufInfo->rAU.rAudioAU.ptrSAddr) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_A,
			TEXT("[GAU] line: %d, Fail for invalid address, ptrSAddr:0x%p, ptrEAddr:0x%p.\r\n"),pEsmIOBufInfo->rAU.rAudioAU.ptrSAddr,
			pEsmIOBufInfo->rAU.rAudioAU.ptrEAddr);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if ((DMX_INVALID_UINT32 != u4Codec) &&
		((u64)0 != (g_rDmxGauManager.arGAUInstance[u4Handle].u8DecSendBufMask & (((u64)1) << u4Codec)))) {
		if (pEsmIOBufInfo->rAU.rAudioAU.ptrSAddr > pEsmIOBufInfo->rAU.rAudioAU.ptrEAddr) {

			u32 u4SzToEnd = 0;
			u32 u4DataSz = DMX_DATASIZE(pEsmIOBufInfo->rAU.rAudioAU.ptrSAddr,
				pEsmIOBufInfo->rAU.rAudioAU.ptrEAddr,
				(pEsmIOBufInfo->ptrFifoEPAddr - pEsmIOBufInfo->ptrFifoSPAddr));

			/* Encounter Circular */
			if (u4DataSz > g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.u4FifoSz) {
				u32 u4FifoSz = (u4DataSz + ((u32)1 << 12U)) / ((u32)1 << 12U) * ((u32)1 << 12U);

				if (0 != g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrUserVirSA) {
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrUserVirSA = 0;
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.hCallProcess = NULL;
				}

				if (0 != g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa) {
					DMX_FreeHwMemory((void *)(g_rDmxGauManager.arGAUInstance[u4Handle].
							rSelfFifoInfo.ptrFifoVSa));
					mm_memset(&(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo),
						0, sizeof(GAU_SELF_USE_FIFO_T));
				}

				#ifdef __linux__
				DMX_NewHwAlignMemory(u4FifoSz, ((u32)1 << 12U),
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa);
				#else
				DMX_NewHwAlignMemory(u4FifoSz, ((u32)1 << 12U),
					(void *)(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa));
				#endif /* #ifdef __linux__ */
				if (0 == g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa) {
					DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_A,
						TEXT("[GAU] %s line %d fail in DMX_NewHwAlignMemory(Audio)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
					pEsmIOBufInfo->u4Status = GAU_E_MEMORY;
					MM_RETURN(RET_DMX_NO_MEM);
				}

				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.u4FifoSz = u4FifoSz;
				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPSa =
					DMX_PHYSICAL(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa);
				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPEa =
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPSa + u4FifoSz;

				DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT(
					"[GAU] %s (Audio) Realloc -- FifoVSa: 0x%x, FifoPSa: 0x%p, FifoPEa: 0x%p, FifoSz: 0x%x\r\n"
					),
					DMX_FUNC_NAME,
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa,
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPSa,
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPEa,
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.u4FifoSz);
			}

			if (0 == g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrUserVirSA) {

				/*get the relative addr , mtk40505 */
				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrUserVirSA =
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa
						- g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrMMRsvBufBase;

				DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT(
						"[GAU] %s (SP/ST) 4 -- FifoVSa: 0x%x, FifoPSa: 0x%p, FifoPEa: 0x%p, FifoSz: 0x%x, UVSa: 0x%p\r\n"
					),
					DMX_FUNC_NAME,
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa,
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPSa,
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPEa,
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.u4FifoSz,
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrUserVirSA);
			}

			u4SzToEnd = pEsmIOBufInfo->ptrFifoEPAddr - pEsmIOBufInfo->rAU.rAudioAU.ptrSAddr;
			dmx_memcpy((u8 *)(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa),
				(u8 *)(pEsmIOBufInfo->rAU.rAudioAU.ptrSAddr), u4SzToEnd);
			dmx_memcpy((u8 *)(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa)
					+ u4SzToEnd,
				(u8 *)pEsmIOBufInfo->ptrFifoSPAddr, u4DataSz - u4SzToEnd);
			pEsmIOBufInfo->rAU.rAudioAU.ptrSAddr = g_rDmxGauManager.
					arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa;
			pEsmIOBufInfo->rAU.rAudioAU.ptrEAddr = g_rDmxGauManager.
					arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa +
				u4DataSz;

#ifdef __linux__
			pEsmIOBufInfo->rAU.rAudioAU.ptrSAddr = pEsmIOBufInfo->rAU.rAudioAU.ptrSAddr
				- g_rDmxGauManager.arGAUInstance[u4Handle].ptrMMRsvBufBase;
			pEsmIOBufInfo->rAU.rAudioAU.ptrEAddr = pEsmIOBufInfo->rAU.rAudioAU.ptrEAddr
				- g_rDmxGauManager.arGAUInstance[u4Handle].ptrMMRsvBufBase;
#else
#endif /* #ifdef __linux__ */

			pEsmIOBufInfo->ptrFifoSPAddr = g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPSa;
			pEsmIOBufInfo->ptrFifoEPAddr = g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPEa;
		} else {
#ifdef __linux__
			pEsmIOBufInfo->rAU.rAudioAU.ptrSAddr = pEsmIOBufInfo->rAU.rAudioAU.ptrSAddr
				- g_rDmxGauManager.arGAUInstance[u4Handle].ptrMMRsvBufBase;
			pEsmIOBufInfo->rAU.rAudioAU.ptrEAddr = pEsmIOBufInfo->rAU.rAudioAU.ptrEAddr
				- g_rDmxGauManager.arGAUInstance[u4Handle].ptrMMRsvBufBase;
#else
#endif /* #ifdef __linux__ */
			pEsmIOBufInfo->ptrFifoSPAddr = DMX_PHYSICAL(pEsmIOBufInfo->ptrFifoSPAddr);
			pEsmIOBufInfo->ptrFifoEPAddr = DMX_PHYSICAL(pEsmIOBufInfo->ptrFifoEPAddr);
		}
	} else {
		pEsmIOBufInfo->rAU.rAudioAU.ptrSAddr -= pEsmIOBufInfo->ptrFifoSPAddr;
		pEsmIOBufInfo->rAU.rAudioAU.ptrEAddr -= pEsmIOBufInfo->ptrFifoSPAddr;
		pEsmIOBufInfo->ptrFifoSPAddr = DMX_PHYSICAL(pEsmIOBufInfo->ptrFifoSPAddr);
		pEsmIOBufInfo->ptrFifoEPAddr = DMX_PHYSICAL(pEsmIOBufInfo->ptrFifoEPAddr);
	}

	if (NULL != pvExtAUInfo) {
		AU_AUDIO_EXT_INFO_T *prAudExtInfo = (AU_AUDIO_EXT_INFO_T *)pvExtAUInfo;

		pEsmIOBufInfo->rAUEx.rAudEx = *prAudExtInfo;
	}

	if (AU_DATA == pEsmIOBufInfo->rAU.rAudioAU.eAuType) {
		pEsmIOBufInfo->u4Status = GAU_S_OK;
		if (pEsmIOBufInfo->rAU.rAudioAU.fgSkipData) {
			DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_A,
				TEXT("[GAU] %s (Audio) Discontinuous ---> eDataType(%d), AUIdx(%d), ")
				TEXT("Pts("DMX_PTS_LOGSTR"), AUSa(0x%p), AUEa(0x%p)\r\n"),
				DMX_FUNC_NAME, ((AU_AUDIO *)pvAU)->eAuType, u4RdIdx,
				DMX_PTS_LOG_MS(((AU_AUDIO *)pvAU)->rAUInfo.rInfo.u8Pts),
				DMX_PTS_LOG_PTS(((AU_AUDIO *)pvAU)->rAUInfo.rInfo.u8Pts),
				((AU_AUDIO *)pvAU)->ptrSAddr,
				((AU_AUDIO *)pvAU)->ptrEAddr);
			pEsmIOBufInfo->rAU.rAudioAU.fgSkipData = FALSE;
			pEsmIOBufInfo->u4Status = GAU_S_DISCONTINUOUS;
		}
	}
	{
		s32 i4Idxj = 0;

		for (i4Idxj = DMX_MAX_LOG_AUDIO_AU_CNT - 1; i4Idxj > 0; i4Idxj--)
			g_arLogAudioAUs[i4Idxj] = g_arLogAudioAUs[i4Idxj - 1];
		g_arLogAudioAUs[0] = *(AU_AUDIO *)pvAU;
	}

	if ((g_rDmxCliMan.fgDumpFlow) &&
		(DMX_SUCCEED(mrRet))) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = g_rDmxGauManager.arGAUInstance[u4Handle].pvSptHdl;
		rOperInfo.unFlow.rGetAU.u4AUIdx = u4RdIdx;
		rOperInfo.unFlow.rGetAU.u4StmType = SPT_DATA_A;
		DmxDumpFlow(DMX_OPER_GETAU, &rOperInfo);
	}
	DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_A,
		TEXT("[GAU] %s (Audio) Success ---> eDataType(%d), AUIdx(%d), ")
		TEXT("Pts("DMX_PTS_LOGSTR"), AUSa(0x%p), AUEa(0x%p)\r\n"),
		DMX_FUNC_NAME, ((AU_AUDIO *)pvAU)->eAuType, u4RdIdx,
		DMX_PTS_LOG_MS(((AU_AUDIO *)pvAU)->rAUInfo.rInfo.u8Pts),
		DMX_PTS_LOG_PTS(((AU_AUDIO *)pvAU)->rAUInfo.rInfo.u8Pts),
		((AU_AUDIO *)pvAU)->ptrSAddr,
		((AU_AUDIO *)pvAU)->ptrEAddr);


	#if DMX_PFM_TEST
	g_rPsrPfm.rAudio.u4GetAURealCnt++;
	#endif /* DMX_PFM_TEST */

	MM_RETURN(mrRet);
}

MRESULT GAU_GetSPAUEx(
	u32			u4Handle,
	ESM_IO_BUF_INFO *pEsmIOBufInfo,
	u32			u4RdIdx,
	void			*pvAU)
{
	MRESULT mrRet	   = RET_DMX_OK;
	AU_SP  *pSPAU	   = NULL;
	u32	u4FifoSize = 0;

	if ((NULL == pEsmIOBufInfo) ||
		(NULL == pvAU)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if ((pEsmIOBufInfo->ptrFifoEPAddr < 0x20000000) ||
		(pEsmIOBufInfo->ptrFifoSPAddr < 0x20000000)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
			TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_FifoGetEA or ")
			TEXT("ESM_FifoGetSA, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
		DMX_ASSERT(0);
		ESM_PrintFifoInfo(u4Handle);
	}

	pSPAU		   = (AU_SP *)pvAU;

	u4FifoSize = pEsmIOBufInfo->ptrFifoEPAddr - pEsmIOBufInfo->ptrFifoSPAddr;
	DMX_ASSERT(0 < u4FifoSize); /* fifosz can't be 0 when code runs to here */
	pEsmIOBufInfo->rAU.rSPStruct.ptrAUEndAddrOfFifo =
		(pSPAU->rAUInfo.rInfo.ptrAddr + pSPAU->rAUInfo.rInfo.u4Size - pEsmIOBufInfo->ptrFifoSPAddr) % u4FifoSize;
	pEsmIOBufInfo->rAU.rSPStruct.ptrAUEndAddrOfFifo =
		pEsmIOBufInfo->ptrFifoSPAddr + pEsmIOBufInfo->rAU.rSPStruct.ptrAUEndAddrOfFifo;

	pEsmIOBufInfo->rAU.rSPStruct.u8StartPts = pSPAU->rAUInfo.rInfo.u8StartPts;
	pEsmIOBufInfo->rAU.rSPStruct.i8Delay =
		(pSPAU->rAUInfo.rInfo.u8EndPts - pSPAU->rAUInfo.rInfo.u8StartPts);
		/* only for internel, for externel, i8Delay will cal in spdmx*/

	pEsmIOBufInfo->rAU.rSPStruct.u4Size = pSPAU->rAUInfo.rInfo.u4Size;

	if (pSPAU->rAUInfo.rInfo.u4Size > g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.u4FifoSz) {
		u32 u4FifoSz = (pSPAU->rAUInfo.rInfo.u4Size + ((u32)1 << 12U)) / ((u32)1 << 12U) * ((u32)1 << 12U);

		if (0 != g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrUserVirSA) {
			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrUserVirSA = 0;
			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.hCallProcess = NULL;
		}

		if (0 != g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa) {
			DMX_FreeHwMemory((void *)(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa));
			mm_memset(&(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo),
				0, sizeof(GAU_SELF_USE_FIFO_T));
		}

		#ifdef __linux__
		DMX_NewHwAlignMemory(u4FifoSz, ((u32)1 << 12U),
				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa);
		#else
		DMX_NewHwAlignMemory(u4FifoSz, ((u32)1 << 12U),
				(void *)(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa));
		#endif /* #ifdef __linux__ */
		if (0 == g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa) {
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SP,
				TEXT("[GAU] %s line %d fail in DMX_NewHwAlignMemory(SP)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			pEsmIOBufInfo->rAU.rSPStruct.ptrUserVirSA = 0;
			pEsmIOBufInfo->u4Status = GAU_E_MEMORY;
			MM_RETURN(RET_DMX_NO_MEM);
		}

		g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.u4FifoSz = u4FifoSz;
		g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPSa =
			DMX_PHYSICAL(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa);
		g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPEa =
			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPSa + u4FifoSz;

		DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
			TEXT("[GAU] %s (SP/ST) Realloc -- FifoVSa: 0x%x, FifoPSa: 0x%p, ")
			TEXT("FifoPEa: 0x%p, FifoSz: 0x%x\r\n"),
			DMX_FUNC_NAME,
			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa,
			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPSa,
			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPEa,
			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.u4FifoSz);
	}

	if (0 == g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrUserVirSA) {
		/*get the relative addr , mtk40505 */
		g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrUserVirSA =
			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa -
			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrMMRsvBufBase;

		DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
			TEXT("[GAU] %s (SP/ST) 4 -- FifoVSa: 0x%x, FifoPSa: 0x%p, FifoPEa: 0x%p, ")
			TEXT("FifoSz: 0x%x, UVSa: 0x%p\r\n"),
			DMX_FUNC_NAME,
			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa,
			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPSa,
			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPEa,
			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.u4FifoSz,
			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrUserVirSA);
	}

	pEsmIOBufInfo->rAU.rSPStruct.ptrVirSA =
		g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa;
	pEsmIOBufInfo->rAU.rSPStruct.ptrPhySA =
		g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPSa;
	pEsmIOBufInfo->rAU.rSPStruct.ptrUserVirSA =
		g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrUserVirSA;
	DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
		TEXT("[GAU] %s (SP), VirSa: 0x%p, UserVirSA: 0x%p, AUSz: 0x%x\r\n"),
		DMX_FUNC_NAME, pEsmIOBufInfo->rAU.rSPStruct.ptrVirSA,
		pEsmIOBufInfo->rAU.rSPStruct.ptrUserVirSA, pSPAU->rAUInfo.rInfo.u4Size);

	if ((pSPAU->rAUInfo.rInfo.ptrAddr - pEsmIOBufInfo->ptrFifoSPAddr +
		pSPAU->rAUInfo.rInfo.u4Size) > u4FifoSize) {
		u32	u4SzToEnd =
			pEsmIOBufInfo->ptrFifoEPAddr - pSPAU->rAUInfo.rInfo.ptrAddr;

		if ((u4SzToEnd > u4FifoSize) ||
			(pSPAU->rAUInfo.rInfo.u4Size >
			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.u4FifoSz)) {
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SP,
				TEXT("[GAU] %s (SP) FATAL ERROR, SPSaddr: 0x%p, AUSz: 0x%08x, ")
				TEXT("FifoSPAddr: 0x%llx, FifoEPAddr: 0x%llx\r\n"),
				DMX_FUNC_NAME,
				pSPAU->rAUInfo.rInfo.ptrAddr, pSPAU->rAUInfo.rInfo.u4Size,
				pEsmIOBufInfo->ptrFifoSPAddr, pEsmIOBufInfo->ptrFifoEPAddr);
			ESM_PrintFifoInfo(u4Handle);
			DMX_ASSERT(FALSE);
		}
		dmx_memcpy((u8 *)pEsmIOBufInfo->rAU.rSPStruct.ptrVirSA,
			(u8 *)(pSPAU->rAUInfo.rInfo.ptrAddr),
			u4SzToEnd);
		dmx_memcpy(((u8 *)pEsmIOBufInfo->rAU.rSPStruct.ptrVirSA) + u4SzToEnd,
			(u8 *)pEsmIOBufInfo->ptrFifoSPAddr,
			pSPAU->rAUInfo.rInfo.u4Size - u4SzToEnd);
	} else {
		dmx_memcpy(((u8 *)pEsmIOBufInfo->rAU.rSPStruct.ptrVirSA),
			(u8 *)(pSPAU->rAUInfo.rInfo.ptrAddr),
			pSPAU->rAUInfo.rInfo.u4Size);
	}

	pEsmIOBufInfo->ptrFifoSPAddr = DMX_PHYSICAL(pEsmIOBufInfo->ptrFifoSPAddr);
	pEsmIOBufInfo->ptrFifoEPAddr = DMX_PHYSICAL(pEsmIOBufInfo->ptrFifoEPAddr);

	if (AU_DATA == pSPAU->eAuType) {
		pEsmIOBufInfo->u4Status = GAU_S_OK;
		if (pSPAU->fgIBCSent) {
			DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SP,
				TEXT("[GAU] %s (SP) Discontinuous ---> eDataType(%d), AUIdx(%d), ")
				TEXT("Pts("DMX_PTS_LOGSTR")\r\n"),
				DMX_FUNC_NAME, ((AU_SP *)pvAU)->eAuType, u4RdIdx,
				DMX_PTS_LOG_MS(((AU_SP *)pvAU)->rAUInfo.rInfo.u8StartPts),
				DMX_PTS_LOG_PTS(((AU_SP *)pvAU)->rAUInfo.rInfo.u8StartPts));
			pSPAU->fgIBCSent = FALSE;
			pEsmIOBufInfo->u4Status = GAU_S_DISCONTINUOUS;
	   }
	}

	if ((g_rDmxCliMan.fgDumpFlow) &&
		(DMX_SUCCEED(mrRet))) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = g_rDmxGauManager.arGAUInstance[u4Handle].pvSptHdl;
		rOperInfo.unFlow.rGetAU.u4AUIdx = u4RdIdx;
		rOperInfo.unFlow.rGetAU.u4StmType = SPT_DATA_SP;
		DmxDumpFlow(DMX_OPER_GETAU, &rOperInfo);
	}
	DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SP,
		TEXT("[GAU] %s (SP) Success ---> eDataType(%d), AUIdx(%d), ")
		TEXT("Pts("DMX_PTS_LOGSTR")\r\n"),
		DMX_FUNC_NAME, ((AU_SP *)pvAU)->eAuType, u4RdIdx,
		DMX_PTS_LOG_MS(((AU_SP *)pvAU)->rAUInfo.rInfo.u8StartPts),
		DMX_PTS_LOG_PTS(((AU_SP *)pvAU)->rAUInfo.rInfo.u8StartPts));

	#if DMX_PFM_TEST
	g_rPsrPfm.rSP.u4GetAURealCnt++;
	#endif /* DMX_PFM_TEST */

	MM_RETURN(mrRet);
}

MRESULT GAU_GetSPAU(
	u32			u4Handle,
	ESM_IO_BUF_INFO *pEsmIOBufInfo,
	u32			u4RdIdx,
	void			*pvAU)
{
	MRESULT mrRet	   = RET_DMX_OK;
	AU_SP  *pSPAU	   = NULL;
	u32	u4FifoSize = 0;
	uintptr_t ptrSAddr = 0;
	uintptr_t ptrEAddr = 0;
	u32	u4SzToEnd = 0;

	if ((NULL == pEsmIOBufInfo) ||
		(NULL == pvAU)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if ((pEsmIOBufInfo->ptrFifoEPAddr < 0x20000000) ||
		(pEsmIOBufInfo->ptrFifoSPAddr < 0x20000000)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
			TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_FifoGetEA or ")
			TEXT("ESM_FifoGetSA, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
		DMX_ASSERT(0);
		ESM_PrintFifoInfo(u4Handle);
	}

	pSPAU = (AU_SP *)pvAU;

	u4FifoSize = pEsmIOBufInfo->ptrFifoEPAddr - pEsmIOBufInfo->ptrFifoSPAddr;
	DMX_ASSERT(0 < u4FifoSize); /* fifosz can't be 0 when code runs to here */
	pEsmIOBufInfo->rAU.rSPStruct.ptrAUEndAddrOfFifo =
		(pSPAU->rAUInfo.rInfo.ptrAddr + pSPAU->rAUInfo.rInfo.u4Size - pEsmIOBufInfo->ptrFifoSPAddr) % u4FifoSize;
	pEsmIOBufInfo->rAU.rSPStruct.ptrAUEndAddrOfFifo =
		pEsmIOBufInfo->ptrFifoSPAddr + pEsmIOBufInfo->rAU.rSPStruct.ptrAUEndAddrOfFifo;

	ptrSAddr = pSPAU->rAUInfo.rInfo.ptrAddr;
	ptrEAddr = pEsmIOBufInfo->rAU.rSPStruct.ptrAUEndAddrOfFifo;

	pEsmIOBufInfo->rAU.rSPStruct.u8StartPts = pSPAU->rAUInfo.rInfo.u8StartPts;
	pEsmIOBufInfo->rAU.rSPStruct.i8Delay =
		(pSPAU->rAUInfo.rInfo.u8EndPts - pSPAU->rAUInfo.rInfo.u8StartPts);
		/* only for internel, for externel, i8Delay will cal in spdmx*/

	pEsmIOBufInfo->rAU.rSPStruct.u4Size = pSPAU->rAUInfo.rInfo.u4Size;

	pEsmIOBufInfo->rAU.rSPStruct.ptrVirSA =
		g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa;
	pEsmIOBufInfo->rAU.rSPStruct.ptrPhySA =
		g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPSa;

	if (ptrSAddr >= ptrEAddr) {
		if (pSPAU->rAUInfo.rInfo.u4Size > g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.u4FifoSz) {
			u32 u4FifoSz = (pSPAU->rAUInfo.rInfo.u4Size + ((u32)1 << 12U)) / ((u32)1 << 12U) * ((u32)1 << 12U);

			if (0 != g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrUserVirSA) {
				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrUserVirSA = 0;
				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.hCallProcess = NULL;
			}

			if (0 != g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa) {
				DMX_FreeHwMemory((void *)(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa));
				mm_memset(&(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo),
					0, sizeof(GAU_SELF_USE_FIFO_T));
			}

			#ifdef __linux__
			DMX_NewHwAlignMemory(u4FifoSz, ((u32)1 << 12U),
					g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa);
			#else
			DMX_NewHwAlignMemory(u4FifoSz, ((u32)1 << 12U),
					(void *)(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa));
			#endif /* #ifdef __linux__ */
			if (0 == g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa) {
				DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SP,
					TEXT("[GAU] %s line %d fail in DMX_NewHwAlignMemory(SP)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				pEsmIOBufInfo->rAU.rSPStruct.ptrUserVirSA = 0;
				pEsmIOBufInfo->u4Status = GAU_E_MEMORY;
				MM_RETURN(RET_DMX_NO_MEM);
			}

			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.u4FifoSz = u4FifoSz;
			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPSa =
				DMX_PHYSICAL(g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa);
			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPEa =
				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPSa + u4FifoSz;

			DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
				TEXT("[GAU] %s (SP/ST) Realloc -- FifoVSa: 0x%x, FifoPSa: 0x%x, ")
				TEXT("FifoPEa: 0x%x, FifoSz: 0x%x\r\n"),
				DMX_FUNC_NAME,
				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa,
				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPSa,
				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPEa,
				g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.u4FifoSz);
		}

		pEsmIOBufInfo->rAU.rSPStruct.ptrVirSA =
			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa;
		pEsmIOBufInfo->rAU.rSPStruct.ptrPhySA =
			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoPSa;
		
		u4SzToEnd =
			pEsmIOBufInfo->ptrFifoEPAddr - pSPAU->rAUInfo.rInfo.ptrAddr;

		if ((u4SzToEnd > u4FifoSize) ||
			(pSPAU->rAUInfo.rInfo.u4Size >
			g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.u4FifoSz)) {
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SP,
				TEXT("[GAU] %s (SP) FATAL ERROR, SPSaddr: 0x%08x, AUSz: 0x%08x, ")
				TEXT("FifoSPAddr: 0x%08x, FifoEPAddr: 0x%08x\r\n"),
				DMX_FUNC_NAME,
				pSPAU->rAUInfo.rInfo.ptrAddr, pSPAU->rAUInfo.rInfo.u4Size,
				pEsmIOBufInfo->ptrFifoSPAddr, pEsmIOBufInfo->ptrFifoEPAddr);
			ESM_PrintFifoInfo(u4Handle);
			DMX_ASSERT(FALSE);
		}
		dmx_memcpy((u8 *)pEsmIOBufInfo->rAU.rSPStruct.ptrVirSA,
			(u8 *)(pSPAU->rAUInfo.rInfo.ptrAddr),
			u4SzToEnd);
		dmx_memcpy((u8 *)(pEsmIOBufInfo->rAU.rSPStruct.ptrVirSA + u4SzToEnd),
			(u8 *)((uintptr_t)pEsmIOBufInfo->ptrFifoSPAddr),
			pSPAU->rAUInfo.rInfo.u4Size - u4SzToEnd);
		pEsmIOBufInfo->rAU.rSPStruct.ptrUserVirSA = pEsmIOBufInfo->rAU.rSPStruct.ptrVirSA 
				- g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrMMRsvBufBase;

	} else {
		pEsmIOBufInfo->rAU.rSPStruct.ptrUserVirSA = ptrSAddr - g_rDmxGauManager.arGAUInstance[u4Handle].ptrMMRsvBufBase;
	}

	pEsmIOBufInfo->ptrFifoSPAddr = DMX_PHYSICAL(pEsmIOBufInfo->ptrFifoSPAddr);
	pEsmIOBufInfo->ptrFifoEPAddr = DMX_PHYSICAL(pEsmIOBufInfo->ptrFifoEPAddr);

	if (AU_DATA == pSPAU->eAuType) {
		pEsmIOBufInfo->u4Status = GAU_S_OK;
		if (pSPAU->fgIBCSent) {
			DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SP,
				TEXT("[GAU] %s (SP) Discontinuous ---> eDataType(%d), AUIdx(%d), ")
				TEXT("Pts("DMX_PTS_LOGSTR")\r\n"),
				DMX_FUNC_NAME, ((AU_SP *)pvAU)->eAuType, u4RdIdx,
				DMX_PTS_LOG_MS(((AU_SP *)pvAU)->rAUInfo.rInfo.u8StartPts),
				DMX_PTS_LOG_PTS(((AU_SP *)pvAU)->rAUInfo.rInfo.u8StartPts),
				((AU_SP *)pvAU)->rAUInfo.rInfo.u8StartPts);
			pSPAU->fgIBCSent = FALSE;
			pEsmIOBufInfo->u4Status = GAU_S_DISCONTINUOUS;
	   }
	}

	if ((g_rDmxCliMan.fgDumpFlow) &&
		(DMX_SUCCEED(mrRet))) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = g_rDmxGauManager.arGAUInstance[u4Handle].pvSptHdl;
		rOperInfo.unFlow.rGetAU.u4AUIdx = u4RdIdx;
		rOperInfo.unFlow.rGetAU.u4StmType = SPT_DATA_SP;
		DmxDumpFlow(DMX_OPER_GETAU, &rOperInfo);
	}
	DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SP,
		TEXT("[GAU] %s (SP) Success ---> ptrSAddr: 0x%p, eDataType(%d), AUIdx(%d), ")
		TEXT("Pts("DMX_PTS_LOGSTR")\r\n"),
		DMX_FUNC_NAME, ptrSAddr, ((AU_SP *)pvAU)->eAuType, u4RdIdx,
		DMX_PTS_LOG_MS(((AU_SP *)pvAU)->rAUInfo.rInfo.u8StartPts),
		DMX_PTS_LOG_PTS(((AU_SP *)pvAU)->rAUInfo.rInfo.u8StartPts),
		((AU_SP *)pvAU)->rAUInfo.rInfo.u8StartPts);

	#if DMX_PFM_TEST
	g_rPsrPfm.rSP.u4GetAURealCnt++;
	#endif /* DMX_PFM_TEST */

	MM_RETURN(mrRet);
}

MRESULT GAU_GetSectionAU(
	u32			u4Handle,
	ESM_IO_BUF_INFO *pEsmIOBufInfo,
	u32			u4RdIdx,
	void			*pvAU)
{
	AU_SECTION *pSAU = (AU_SECTION *)pvAU;
	u32 u4Size = 0;

	pEsmIOBufInfo->rAU.rSectionAU.ptrEAddr = pSAU->ptrEAddr;
	pEsmIOBufInfo->rAU.rSectionAU.ptrSAddr = pSAU->ptrSAddr;
	u4Size = pSAU->ptrEAddr - pSAU->ptrSAddr;

	if (pSAU->u4Size <= u4Size)
		u4Size = pSAU->u4Size;

	#ifdef __linux__
	DMX_NewHwAlignMemory(u4Size, ((u32)1<<12U), pEsmIOBufInfo->rAU.rSectionAU.ptrVirSA);
	#else
	DMX_NewHwAlignMemory(u4Size, ((u32)1<<12U), (void *)(pEsmIOBufInfo->rAU.rSectionAU.ptrVirSA));
	#endif /* #ifdef __linux__*/
	if (0 == pEsmIOBufInfo->rAU.rSectionAU.ptrVirSA) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SEC,
			TEXT("[GAU] %s line %d fail in DMX_NewHwAlignMemory (SECTION)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		pEsmIOBufInfo->u4Status = GAU_E_MEMORY;
		MM_RETURN(RET_DMX_NO_MEM);
	}

	dmx_memcpy(((u8 *)pEsmIOBufInfo->rAU.rSectionAU.ptrVirSA),
		(u8 *)(pSAU->ptrSAddr), u4Size);

	pEsmIOBufInfo->rAU.rSectionAU.u4Size = u4Size;

	pEsmIOBufInfo->ptrFifoSPAddr = (__u64)(DMX_PHYSICAL(pEsmIOBufInfo->ptrFifoSPAddr));
	pEsmIOBufInfo->ptrFifoEPAddr = (__u64)(DMX_PHYSICAL(pEsmIOBufInfo->ptrFifoEPAddr));

	/*get the relative addr , mtk40505*/
	pEsmIOBufInfo->rAU.rSectionAU.ptrUserVirSA = 
	  pEsmIOBufInfo->rAU.rSectionAU.ptrVirSA - 
							- g_rDmxGauManager.arGAUInstance[u4Handle].ptrMMRsvBufBase;

	MM_RETURN(RET_DMX_OK);
}

MRESULT GAU_GetSectionAUEx(u32 u4Handle, void *pvIOBuf, u32 u4BuffSize)
{
	void	*pvAU = NULL;
	ES_TYPE eType = ES_NONE;
	EV_GRP_EVENT_T ev = 0;
	s32	i4WaitResult = 0;
	u32	u4AvailCount = 0;
	uintptr_t	ptrAuEa = 0;
	u32	u4RdIdx = ESM_INVALID_INDEX;
	MRESULT mrRet  = RET_DMX_OK;

	if (MAX_GAU_INSTANCE_CNT <= u4Handle) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SEC,
			TEXT("[GAU] %s line %d fail for HANDLE(%d) > MAX_GAU_INSTANCE_CNT(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, MAX_GAU_INSTANCE_CNT);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!(g_rDmxGauManager.arGAUInstance[u4Handle].fgUsed)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SEC,
			TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail for the GAU handle hasn't been connected\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		return 0;
	}

	if (!(g_rDmxGauManager.arGAUInstance[u4Handle].fgGetAUEnable)) {
		x_ev_group_wait_event_timeout(
			(uintptr_t)g_rDmxGauManager.arGAUInstance[u4Handle].hAUInEG,
			GAU_EV_DISABLE_GETAU, &ev, X_EV_OP_OR_CONSUME, 1);
		return 0;
	}

	mrRet = ESM_AUTableGetAvailCount(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
		&u4AvailCount);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SEC,
			TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_AUTableGetAvailCount, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
		MM_RETURN(mrRet);
	}
	if ((u4AvailCount < (u32)1) && (!(g_rDmxGauManager.arGAUInstance[u4Handle].fgEOS))) {
		mrRet = ESM_GetESType(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &eType);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_GetESType\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			return 0;
		}

		i4WaitResult =	x_ev_group_wait_event_timeout((uintptr_t)g_rDmxGauManager.arGAUInstance[u4Handle].hAUInEG,
			(GAU_EV_AU_IN		  |
			 GAU_EV_FIFO_FLUSH	  |
			 GAU_EV_DISABLE_GETAU),
			 &ev, X_EV_OP_OR_CONSUME, 500);
		if (OSR_FAIL == i4WaitResult) {
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SEC,
				TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in wait AU_IN|FIFO_FLUSH events\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			return 0;
		} else if (OSR_TIMEOUT == i4WaitResult) {
			return 0;
		} /* wait ok, but Fifo flushed */ else if (0 != (ev & GAU_EV_FIFO_FLUSH)) {
			DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SEC,
				TEXT("[GAU] %s line %d (u4Handle: 0x%x) -- FIFO_FLUSH\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			return 0;
		} else if (0 != (ev & GAU_EV_DISABLE_GETAU)) {
			DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SEC,
				TEXT("[GAU] %s line %d (u4Handle: 0x%x) -- DISABLE_GETAU\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			return 0;
		} else {
			/*do nothing*/
		}
		mrRet = ESM_AUTableGetAvailCount(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
			&u4AvailCount);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SEC,
				TEXT(
					"[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_AUTableGetAvailCount, mrRet: 0x%x\r\n"
				),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
			return 0;
		}
	}

	if (u4AvailCount < 1)
		return 0;

	mrRet = ESM_AUTableGetRdIdx(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &u4RdIdx);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SEC,
			TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_AUTableGetRdIdx, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
		return 0;
	}

	mrRet = ESM_AUTableGetAUInfo(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
		u4RdIdx, &(pvAU));
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SEC,
			TEXT(
				"[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_AUTableGetAUInfo(u4RdIdx: %d), mrRet: 0x%x\r\n"
			),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4RdIdx, mrRet);
		return 0;
	}

	mrRet = ESM_GetESType(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &eType);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SEC,
			TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_GetESType, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
		return 0;
	}
	/* convert to phisical address */
	if (ES_SECTION == eType) {
		AU_SECTION *pSectionAU = (AU_SECTION *)pvAU;

		ptrAuEa = pSectionAU->ptrEAddr;

		if (u4BuffSize > pSectionAU->u4Size)
			u4BuffSize = pSectionAU->u4Size;
		dmx_memcpy((u8 *)pvIOBuf, (u8 *)(pSectionAU->ptrSAddr), u4BuffSize);
	} else {
		DMX_ASSERT(FALSE); /* never go here */
		return 0;
	}

	++g_rDmxGauManager.arGAUInstance[u4Handle].u4AUConsumedCnt;

	mrRet = ESM_FifoSetRdPtr(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
		ptrAuEa, FALSE);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SEC,
			TEXT(
				"[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_FifoSetRdPtr, ptrAuEa: 0x%p, eType: 0x%x, u4RdIdx: 0x%x, i4Ret: 0x%x\r\n"
			),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, ptrAuEa, eType, u4RdIdx, mrRet);
		return 0;
	}

	mrRet = ESM_AUTableIncRdIdx(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, 1);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SEC,
			TEXT(
				"[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_AUTableIncRdIdx(u4RdIdx: %d), mrRet: 0x%x\r\n"
			),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4RdIdx, mrRet);
		return 0;
	}

	return u4BuffSize;
}
ESM_IO_BUF_INFO *GAU_Get_GEsmIOBuf(u32 u4Handle)
{
	if (MAX_GAU_INSTANCE_CNT <= u4Handle) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
			TEXT("[GAU] %s line %d fail for HANDLE(%d) > MAX_GAU_INSTANCE_CNT(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, MAX_GAU_INSTANCE_CNT);
		return NULL;
	}

	if (!(g_rDmxGauManager.arGAUInstance[u4Handle].fgUsed)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
			TEXT("[GAU] %s line %d fail for HANDLE(%d) GAU hasn't been connected\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		return NULL;
	}

	return g_rDmxGauManager.arGAUInstance[u4Handle].prGetEsmIOBuf;
}

ESM_IO_BUF_INFO *GAU_Get_REsmIOBuf(u32 u4Handle)
{
	if (MAX_GAU_INSTANCE_CNT <= u4Handle) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
			TEXT("[GAU] %s line %d fail for HANDLE(%d) > MAX_GAU_INSTANCE_CNT(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, MAX_GAU_INSTANCE_CNT);
		return NULL;
	}

	if (!(g_rDmxGauManager.arGAUInstance[u4Handle].fgUsed)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
			TEXT("[GAU] %s line %d fail for HANDLE(%d) GAU hasn't been connected\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		return NULL;
	}

	return g_rDmxGauManager.arGAUInstance[u4Handle].prRelEsmIOBuf;
}

MRESULT GAU_GetAU(u32 u4Handle, void *pvIOBuf)
{
	ESM_IO_BUF_INFO *pEsmIOBufInfo = (ESM_IO_BUF_INFO *)pvIOBuf;
	void   *pvAU = NULL;
	void   *pvExtAU = NULL;
	ES_TYPE eType = ES_NONE;
	EV_GRP_EVENT_T ev = 0;
	s32	i4WaitResult = 0;
	u32	u4AvailCount = 0;
	u32	u4RdIdx = ESM_INVALID_INDEX;
	MRESULT mrRet	= RET_DMX_OK;

	if (NULL == pEsmIOBufInfo) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
			TEXT("[GAU] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (MAX_GAU_INSTANCE_CNT <= u4Handle) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
			TEXT("[GAU] %s line %d fail for HANDLE(%d) > MAX_GAU_INSTANCE_CNT(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, MAX_GAU_INSTANCE_CNT);
		pEsmIOBufInfo->u4Status = GAU_E_INVALIDARG;
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	pEsmIOBufInfo->u4Status = GAU_S_OK;

	if (!(g_rDmxGauManager.arGAUInstance[u4Handle].fgUsed)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
			TEXT("[GAU] %s line %d fail for HANDLE(%d) GAU hasn't been connected\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		pEsmIOBufInfo->u4Status = GAU_E_INVALIDARG;
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (!(g_rDmxGauManager.arGAUInstance[u4Handle].fgGetAUEnable)) {
		x_ev_group_wait_event_timeout(
			(uintptr_t)g_rDmxGauManager.arGAUInstance[u4Handle].hAUInEG,
			GAU_EV_DISABLE_GETAU, &ev, X_EV_OP_OR_CONSUME, 10);

		if (SPT_DATA_A == g_rDmxGauManager.arGAUInstance[u4Handle].u4StmType)
			ESM_CheckAudDrvStatus(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle);

		pEsmIOBufInfo->u4Status = GAU_E_TIMEOUT;

		MM_RETURN(RET_DMX_NO_AU);
	}

	#if DMX_PFM_TEST
	switch (g_rDmxGauManager.arGAUInstance[u4Handle].u4StmType) {
	case SPT_DATA_V:
		DmxPfmStmGetAUStart(SPT_DATA_V);
		g_rPsrPfm.rVideo.u4GetAUCnt++;
		break;
	case SPT_DATA_A:
		DmxPfmStmGetAUStart(SPT_DATA_A);
		g_rPsrPfm.rAudio.u4GetAUCnt++;
		break;
	case SPT_DATA_SP:
		DmxPfmStmGetAUStart(SPT_DATA_SP);
		g_rPsrPfm.rSP.u4GetAUCnt++;
		break;
	default:
		break;
	}
	#endif /* DMX_PFM_TEST */

#if DMX_DISABLE_CFA
	if (g_rDmxGauManager.arGAUInstance[u4Handle].u4AUConsumedCnt >= 1000) {
		pEsmIOBufInfo->u4Status = GAU_E_EOS;
		MM_RETURN(RET_DMX_NO_AU);
	} else {
		mrRet = ESM_GetESType(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &eType);
		if (DMX_FAILED(mrRet)) {
			pEsmIOBufInfo->ptrFifoSPAddr = GAU_E_FAIL;
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
				TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_GetESType\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		if (g_rDmxGauManager.arGAUInstance[u4Handle].u4AUConsumedCnt == 0)
			pEsmIOBufInfo->u4Status = GAU_S_DISCONTINUOUS;
		else
			pEsmIOBufInfo->u4Status = GAU_S_OK;

		/* convert to phisical address */
		if (ES_V == eType) {
			pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.u8Pts =
				g_rDmxGauManager.arGAUInstance[u4Handle].u4AUConsumedCnt * (DMX_PTS_1S / 30);
		} else if (ES_A == eType) {
			pEsmIOBufInfo->rAU.rAudioAU.rAUInfo.rInfo.u8Pts =
				g_rDmxGauManager.arGAUInstance[u4Handle].u4AUConsumedCnt * (DMX_PTS_1S / 30);
		} else if (ES_SP == eType) {
			pEsmIOBufInfo->rAU.rSPStruct.u8StartPts =
				g_rDmxGauManager.arGAUInstance[u4Handle].u4AUConsumedCnt * (DMX_PTS_1S / 30);
			pEsmIOBufInfo->rAU.rSPStruct.i8Delay = (s64)(DMX_PTS_1S / 30);
		}
	}

	++g_rDmxGauManager.arGAUInstance[u4Handle].u4AUConsumedCnt;

	MM_RETURN(RET_DMX_OK);

#elif (DMX_DISABLE_DMA_DATA || DMX_DISABLE_COMP_AU)

	mrRet = ESM_GetESType(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &eType);
	if (DMX_FAILED(mrRet)) {
		pEsmIOBufInfo->ptrFifoSPAddr = GAU_E_FAIL;
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
			TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_GetESType\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	switch (eType) {
	case ES_V: {
			VCodeC eVCodec = g_rPSRHalStruct.eVideoCodec;
			#if DMX_DISABLE_COMP_MPEG2AU
			if (VC_MPEG2 == eVCodec) {
				mrRet = GAU_GetVirtualAU(eType, u4Handle, pEsmIOBufInfo);
				MM_RETURN(mrRet);
			}
			#endif /* DMX_DISABLE_COMP_MPEG2AU */

			#if DMX_DISABLE_COMP_MPEG4AU
			if ((VC_MPEG4 == eVCodec) ||
				(VC_DIVX4 == eVCodec) ||
				(VC_DIVX6 == eVCodec) ||
				(VC_H263  == eVCodec)) {
				mrRet = GAU_GetVirtualAU(eType, u4Handle, pEsmIOBufInfo);
				MM_RETURN(mrRet);
			}
			#endif /* DMX_DISABLE_COMP_MPEG4AU */

			#if DMX_DISABLE_COMP_AVCAU
			if (VC_H264 == eVCodec) {
				mrRet = GAU_GetVirtualAU(eType, u4Handle, pEsmIOBufInfo);
				MM_RETURN(mrRet);
			}
			#endif /* DMX_DISABLE_COMP_AVCAU */

			#if DMX_DISABLE_COMP_VC1AU
			if (VC_VC1 == eVCodec) {
				mrRet = GAU_GetVirtualAU(eType, u4Handle, pEsmIOBufInfo);
				MM_RETURN(mrRet);
			}
			#endif /* DMX_DISABLE_COMP_VC1AU */
		}
		break;
	#if DMX_DISABLE_COMP_AUDIOAU
	case ES_A:
		mrRet = GAU_GetVirtualAU(eType, u4Handle, pEsmIOBufInfo);
		MM_RETURN(mrRet);
		break;
	#endif
	#if DMX_DISABLE_COMP_SPAU
	case ES_SP:
		mrRet = GAU_GetVirtualAU(eType, u4Handle, pEsmIOBufInfo);
		MM_RETURN(mrRet);
		break;
	#endif
	#if DMX_DISABLE_COMP_OTHAU
	case ES_SECTION:
		mrRet = GAU_GetVirtualAU(eType, u4Handle, pEsmIOBufInfo);
		MM_RETURN(mrRet);
		break;
	#endif
	default:
		break;
	}
#else /* DMX_DISABLE_CFA */

#endif /* DMX_DISABLE_CFA */

	if (!g_rDmxGauManager.arGAUInstance[u4Handle].fgReachThreshold) {
		i4WaitResult = x_ev_group_wait_event_timeout(
			(uintptr_t)g_rDmxGauManager.arGAUInstance[u4Handle].hAUInEG,
			(GAU_EV_REACH_THRESHOLD |
			 GAU_EV_FIFO_FLUSH		|
			 GAU_EV_SKIP_THRESHOLD	|
			 GAU_EV_DISABLE_GETAU),
			&ev,
			X_EV_OP_OR_CONSUME,
#if DMX_GAU_GETAU_WAIT_THRESHOLD_EVT
			pEsmIOBufInfo->u4TimeWait);
#else
			10);
#endif /* DMX_GAU_GETAU_WAIT_THRESHOLD_EVT */
		if (OSR_OK != i4WaitResult) {
			pEsmIOBufInfo->u4Status = GAU_E_THRESHOLD;
			if (OSR_TIMEOUT != i4WaitResult) {
				DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT(
						"[GAU] %s line %d (u4Handle: 0x%x) exit in wait GAU_EV_REACH_THRESHOLD, i4Ret: 0x%x\r\n"
					),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, i4WaitResult);
			}
			MM_RETURN(RET_DMX_NO_REACH_THRESHOLD);
		} else {
			if (0 != (ev & GAU_EV_FIFO_FLUSH)) {
				DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT("[GAU] %s (u4Handle: 0x%x) Fifo Flush NOW!!!\r\n"),
					DMX_FUNC_NAME, u4Handle);
				pEsmIOBufInfo->u4Status = GAU_E_FIFOFLUSH;
				MM_RETURN(RET_DMX_NO_AU);
			} else if (0 != (ev & GAU_EV_REACH_THRESHOLD)) {
				DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT("[GAU] %s (u4Handle: 0x%x) REACH THRESHOLD NOW!!!\r\n"),
					DMX_FUNC_NAME, u4Handle);
				g_rDmxGauManager.arGAUInstance[u4Handle].fgReachThreshold = TRUE;
			} else if (0 != (ev & GAU_EV_SKIP_THRESHOLD)) {
				DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT("[GAU] %s (u4Handle: 0x%x) SKIP THRESHOLD NOW!!!\r\n"),
					DMX_FUNC_NAME, u4Handle);
			} else if (0 != (ev & GAU_EV_DISABLE_GETAU)) {
				DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT("[GAU] %s line %d (u4Handle: 0x%x) DISABLE GET AU NOW!!!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
				pEsmIOBufInfo->u4Status = GAU_E_TIMEOUT;
				Sleep(1);
				MM_RETURN(RET_DMX_NO_AU);
			} else {
				DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT("[GAU] %s (u4Handle: 0x%x) -- unknown signal event, ev: 0x%x!!!\r\n"),
					DMX_FUNC_NAME, u4Handle);
				pEsmIOBufInfo->u4Status = GAU_E_FAIL;
				MM_RETURN(RET_DMX_NO_AU);
			}
		}
	}

	while (TRUE) {
#if DMX_GAU_GETAU_WAIT_THRESHOLD_EVT
		while (TRUE) {
			mrRet = ESM_AUTableGetAvailCount(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
				&u4AvailCount);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT(
						"[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_AUTableGetAvailCount, mrRet: 0x%x\r\n"
					),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
				MM_RETURN(mrRet);
			}

			if ((u4AvailCount < 1) &&
				(!(g_rDmxGauManager.arGAUInstance[u4Handle].fgEOS))) {
				i4WaitResult =	x_ev_group_wait_event_timeout(
					(uintptr_t)g_rDmxGauManager.arGAUInstance[u4Handle].hAUInEG,
					(GAU_EV_AU_IN		|
					 GAU_EV_FIFO_FLUSH	|
					 GAU_EV_DISABLE_GETAU),
					&ev, X_EV_OP_OR_CONSUME,
					pEsmIOBufInfo->u4TimeWait);
				if (OSR_TIMEOUT == i4WaitResult) {
					pEsmIOBufInfo->u4Status = GAU_E_TIMEOUT;
					MM_RETURN(RET_DMX_NO_AU);
				} else if (OSR_OK != i4WaitResult) {
					DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
						TEXT(
							"[GAU] %s line %d (u4Handle: 0x%x) fail in wait AU_IN|FIFO_FLUSH events\r\n"
						),
						DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
					pEsmIOBufInfo->u4Status = GAU_E_FAIL;
					MM_RETURN(RET_DMX_OS_OPERA_FAIL);
				} else if ((0 != (ev & GAU_EV_FIFO_FLUSH))) /* wait ok, but Fifo flushed */ {
					DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
						TEXT("[GAU] %s line %d (u4Handle: 0x%x) -- FIFO_FLUSH\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
					pEsmIOBufInfo->u4Status = GAU_E_FIFOFLUSH;
					MM_RETURN(RET_DMX_NO_AU);
				} else if (0 != (ev & GAU_EV_DISABLE_GETAU)) {
					DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
						TEXT("[GAU] %s line %d (u4Handle: 0x%x) DISABLE GET AU NOW!!!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
					pEsmIOBufInfo->u4Status = GAU_E_TIMEOUT;
					Sleep(1);
					MM_RETURN(RET_DMX_NO_AU);
				}

				mrRet = ESM_AUTableGetAvailCount(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
						&u4AvailCount);
				if (DMX_FAILED(mrRet)) {
					pEsmIOBufInfo->u4Status = GAU_E_FAIL;
					DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
						TEXT(
							"[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_AUTableGetAvailCount, mrRet: 0x%x\r\n"
						),
						DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
					MM_RETURN(mrRet);
				}
			}

			if (u4AvailCount < 1) {
				if (g_rDmxGauManager.arGAUInstance[u4Handle].fgEOS) {
					mrRet = ESM_AUTableGetAvailCount(
							g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
							&u4AvailCount);
					if (DMX_FAILED(mrRet)) {
						pEsmIOBufInfo->u4Status = GAU_E_FAIL;
						DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
							TEXT(
								"[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_AUTableGetAvailCount, mrRet: 0x%x\r\n"
							),
							DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
						MM_RETURN(mrRet);
					} else if (u4AvailCount < 1) {
						#if DMX_PFM_TEST
						DmxPfmStmGetAUEnd(g_rDmxGauManager.arGAUInstance[u4Handle].u4StmType);
						#endif /* DMX_PFM_TEST*/
						pEsmIOBufInfo->u4Status =
								g_rDmxGauManager.arGAUInstance[u4Handle].u4Status;
						DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
							TEXT(
								"[GAU] %s line %d (u4Handle: 0x%x) encounter EOS, u4Status: 0x%x, u4StmType: %d\r\n"
							),
							DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, pEsmIOBufInfo->u4Status,
							g_rDmxGauManager.arGAUInstance[u4Handle].u4StmType);
						MM_RETURN(RET_DMX_NO_AU);
					}
				}
			} else {
				break;
			}
		}

#else /* DMX_GAU_GETAU_WAIT_THRESHOLD_EVT */

		mrRet = ESM_AUTableGetAvailCount(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
			&u4AvailCount);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT(
					"[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_AUTableGetAvailCount, mrRet: 0x%x\r\n"
				),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
			MM_RETURN(mrRet);
		}
		if ((u4AvailCount < 1) &&
			(!(g_rDmxGauManager.arGAUInstance[u4Handle].fgEOS))) {
			i4WaitResult =	x_ev_group_wait_event_timeout((uintptr_t)g_rDmxGauManager.arGAUInstance[u4Handle].hAUInEG,
				(GAU_EV_AU_IN |
				 GAU_EV_FIFO_FLUSH |
				 GAU_EV_DISABLE_GETAU),
				 &ev, X_EV_OP_OR_CONSUME,
				 pEsmIOBufInfo->u4TimeWait);
			if (OSR_FAIL == i4WaitResult) {
				DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT(
						"[GAU] %s line %d (u4Handle: 0x%x) fail in wait AU_IN|FIFO_FLUSH events\r\n"
					),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
				pEsmIOBufInfo->u4Status = GAU_E_FAIL;
				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
			} else if (OSR_TIMEOUT == i4WaitResult) {
				pEsmIOBufInfo->u4Status = GAU_E_TIMEOUT;
				MM_RETURN(RET_DMX_NO_AU);
			} /* wait ok, but Fifo flushed */ else if (((ev & GAU_EV_FIFO_FLUSH) != 0)) {
				DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT("[GAU] %s line %d (u4Handle: 0x%x) -- FIFO_FLUSH\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
				pEsmIOBufInfo->u4Status = GAU_E_FIFOFLUSH;
				MM_RETURN(RET_DMX_NO_AU);
			} else if (0 != (ev & GAU_EV_DISABLE_GETAU)) {
				DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT("[GAU] %s line %d (u4Handle: 0x%x) DISABLE GET AU NOW!!!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
				pEsmIOBufInfo->u4Status = GAU_E_TIMEOUT;
				Sleep(1);
				MM_RETURN(RET_DMX_NO_AU);
			} else {
				/*do nothing*/
			}

			mrRet = ESM_AUTableGetAvailCount(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
				&u4AvailCount);
			if (DMX_FAILED(mrRet)) {
				pEsmIOBufInfo->u4Status = GAU_E_FAIL;
				DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT(
						"[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_AUTableGetAvailCount, mrRet: 0x%x\r\n"
					),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
				MM_RETURN(mrRet);
			}
		}

		if (u4AvailCount < 1) {
			pEsmIOBufInfo->u4Status = GAU_E_TIMEOUT;
			if (g_rDmxGauManager.arGAUInstance[u4Handle].fgEOS) {
				mrRet = ESM_AUTableGetAvailCount(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
						&u4AvailCount);
				if (DMX_FAILED(mrRet)) {
					pEsmIOBufInfo->u4Status = GAU_E_FAIL;
					DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
						TEXT(
							"[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_AUTableGetAvailCount, mrRet: 0x%x\r\n"
						),
						DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
					MM_RETURN(mrRet);
				} else if (u4AvailCount < 1) {
					pEsmIOBufInfo->u4Status = g_rDmxGauManager.arGAUInstance[u4Handle].u4Status;
					#if DMX_PFM_TEST
					DmxPfmStmGetAUEnd(g_rDmxGauManager.arGAUInstance[u4Handle].u4StmType);
					#endif /* DMX_PFM_TEST*/
					DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
						TEXT(
							"[GAU] %s line %d (u4Handle: 0x%x) encounter EOS, u4Status: 0x%x, u4StmType: %d\r\n"
						),
						DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, pEsmIOBufInfo->u4Status,
						g_rDmxGauManager.arGAUInstance[u4Handle].u4StmType);
				} else {
					/*do nothing*/
				}
			}
			MM_RETURN(RET_DMX_NO_AU);
		}
#endif /* DMX_GAU_GETAU_WAIT_THRESHOLD_EVT*/

		mrRet = ESM_FifoGetSA(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
			&(pEsmIOBufInfo->ptrFifoSPAddr));
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
				TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_FifoGetSA, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
			pEsmIOBufInfo->u4Status = GAU_E_FAIL;
			MM_RETURN(mrRet);
		}
		mrRet = ESM_FifoGetEA(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
			&(pEsmIOBufInfo->ptrFifoEPAddr));
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
				TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_FifoGetEA, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
			pEsmIOBufInfo->u4Status = GAU_E_FAIL;
			MM_RETURN(mrRet);
		}

		if ((pEsmIOBufInfo->ptrFifoEPAddr < 0x20000000) ||
			(pEsmIOBufInfo->ptrFifoSPAddr < 0x20000000)) {
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
				TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_FifoGetEA ")
				TEXT("or ESM_FifoGetSA, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
			DMX_ASSERT(0);
			ESM_PrintFifoInfo(u4Handle);
		}

		mrRet = ESM_AUTableGetRdIdx(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &u4RdIdx);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
				TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_AUTableGetRdIdx\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			pEsmIOBufInfo->u4Status = GAU_E_FAIL;
			MM_RETURN(mrRet);
		}

		if (ESM_INVALID_INDEX == u4RdIdx) {
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
				TEXT("[GAU] GAU_GetAU(u4Handle: 0x%x) fail in ESM_AUTableGetRdIdx\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			pEsmIOBufInfo->u4Status = GAU_E_FAIL;
			MM_RETURN(RET_DMX_NO_AU);
		}

		mrRet = GAU_Q_GetAU(&(g_rDmxGauManager.arGAUInstance[u4Handle].rGetAUQueue), u4RdIdx);
		if (DMX_FAILED(mrRet)) {
			DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
				TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in GAU_Q_GetAU\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			pEsmIOBufInfo->u4Status = GAU_E_TIMEOUT;
			MM_RETURN(mrRet);
		}

#if DMX_DISABLE_GET_REALAU
		{
			uintptr_t	ptrAuEa = 0;
			AU_TYPE eAuType = AU_DATA;
			bool	fgSendIBC = FALSE;

			mrRet = ESM_GetESType(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &eType);
			if (DMX_FAILED(mrRet)) {
				pEsmIOBufInfo->ptrFifoSPAddr = GAU_E_FAIL;
				DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_GetESType\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			mrRet = ESM_AUTableGetAUInfo(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
					u4RdIdx, &(pvAU));
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT(
						"[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_AUTableGetAUInfo(u4RdIdx: 0x%x).\r\n"
					),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4RdIdx);
				/*DMX_ASSERT(FALSE);*/
				pEsmIOBufInfo->u4Status = GAU_E_FAIL;
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			if (NULL == pvAU) {
				DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT(
						"[GAU] %s line %d (u4Handle: 0x%x) fail, prAU == NULL, (u4RdIdx: 0x%x).\r\n"
					),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4RdIdx);
				/*DMX_ASSERT(FALSE);*/
				pEsmIOBufInfo->u4Status = GAU_E_FAIL;
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			if (ES_V == eType) {
				ptrAuEa = ((AU_VPic *)pvAU)->rAUInfo.rInfo.ptrEAddr;
			} else if (ES_A == eType) {
				ptrAuEa = ((AU_AUDIO *)pvAU)->ptrEAddr;
			} else if (ES_SP == eType) {
				if (NULL == pEsmIOBufInfo) {
					DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
						TEXT(
							"[GAU] %s line %d(SP, u4Handle: 0x%x) is fail, pvIOBuf == NULL.\r\n"
						),
						DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
					pEsmIOBufInfo->u4Status = GAU_E_FAIL;
					MM_RETURN(RET_DMX_PARAM_WRONG);
				}

				if (pEsmIOBufInfo->ptrFifoEPAddr > pEsmIOBufInfo->ptrFifoSPAddr) {
					u32 u4FifoSize = pEsmIOBufInfo->ptrFifoEPAddr - pEsmIOBufInfo->ptrFifoSPAddr;

					pEsmIOBufInfo->rAU.rSPStruct.ptrAUEndAddrOfFifo =
						(((AU_SP *)pvAU)->rAUInfo.rInfo.ptrAddr
							+ ((AU_SP *)pvAU)->rAUInfo.rInfo.u4Size
							- pEsmIOBufInfo->ptrFifoSPAddr) % u4FifoSize;
					pEsmIOBufInfo->rAU.rSPStruct.ptrAUEndAddrOfFifo =
						pEsmIOBufInfo->ptrFifoSPAddr
							+ pEsmIOBufInfo->rAU.rSPStruct.ptrAUEndAddrOfFifo;
					ptrAuEa = pEsmIOBufInfo->rAU.rSPStruct.ptrAUEndAddrOfFifo;
				} else {
					DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
						TEXT(
							"[GAU] %s line %d (Section) is fail for SP's FifoEA(0x%p) <= FifoSA(0x%p)\r\n"
						),
						DMX_FUNC_NAME, DMX_LINE_NO,
						pEsmIOBufInfo->ptrFifoEPAddr,
						pEsmIOBufInfo->ptrFifoSPAddr);
					pEsmIOBufInfo->u4Status = GAU_E_FAIL;
					MM_RETURN(RET_DMX_UNEXPECT);
				}
			} else {
				DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT("[GAU] %s line %d (Section) is fail for invalid eType(0x%x)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, eType);
				pEsmIOBufInfo->u4Status = GAU_E_FAIL;
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			mrRet = ESM_FifoSetRdPtr(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
				ptrAuEa, FALSE);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT(
						"[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_FifoSetRdPtr, ptrAuEa: 0x%p, eType: 0x%x, u4RdIdx: 0x%x, i4Ret: 0x%x\r\n"
					),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, ptrAuEa, eType, u4RdIdx, mrRet);
				pEsmIOBufInfo->u4Status = GAU_E_FAIL;
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			if (ES_V == eType) {
				eAuType = ((AU_VPic *)pvAU)->eAuType;
				fgSendIBC = ((AU_VPic *)pvAU)->fgIBCSent;
				dmx_memset(pvAU, 0, sizeof(AU_VPic));
			} else if (ES_A == eType) {
				eAuType = ((AU_AUDIO *)pvAU)->eAuType;
				dmx_memset(pvAU, 0, sizeof(AU_AUDIO));
			} else if (ES_SP == eType) {
				eAuType = ((AU_SP *)pvAU)->eAuType;
				dmx_memset(pvAU, 0, sizeof(AU_SP));
			} else if (ES_SECTION == eType) {
				eAuType = ((AU_SECTION *)pvAU)->eAuType;
				dmx_memset(pvAU, 0, sizeof(AU_SECTION));
			}

			mrRet = ESM_AUTableIncRdIdx(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, 1);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
					TEXT(
						"[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_AUTableIncRdIdx(u4RdIdx: 0x%x, eType: 0x%x, i4Ret: 0x%x)\r\n"
					),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4RdIdx, eType, mrRet);
				/*DMX_ASSERT(FALSE);*/
				pEsmIOBufInfo->u4Status = GAU_E_FAIL;
				MM_RETURN(RET_DMX_OK);
			}

			if (ES_A == eType) {
				if (AU_CMD == eAuType) {
					DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
						TEXT(
							"[GAU] %s (u4Handle: 0x%x, Audio) -->	GAU_SetEvent(GAU_EV_AUCMD_RELEASE)!\r\n"
						),
						DMX_FUNC_NAME, u4Handle);
					mrRet = GAU_SetEvent(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
						GAU_EV_AUCMD_RELEASE);
					if (DMX_FAILED(mrRet)) {
						DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
							TEXT(
								"[GAU] %s line %d (u4Handle: 0x%x, Audio) fail in GAU_GetEvent(GAU_EV_AUCMD_RELEASE)!\r\n"
							),
							DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
					}
				}
			} else if (ES_V == eType) {
				if ((AU_CMD == eAuType) &&
					fgSendIBC) {
					DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
						TEXT(
							"[GAU] %s (u4Handle: 0x%x, Video) -->	GAU_SetEvent(GAU_EV_AUCMD_RELEASE)!\r\n"
						),
						DMX_FUNC_NAME, u4Handle);
					mrRet = GAU_SetEvent(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
						GAU_EV_AUCMD_RELEASE);
					if (DMX_FAILED(mrRet)) {
						DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
							TEXT(
								"[GAU] %s line %d (u4Handle: 0x%x, Video) fail in GAU_GetEvent(GAU_EV_AUCMD_RELEASE)!\r\n"
							),
							DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
					}
				}
			} else {
			}

			if (g_rDmxGauManager.arGAUInstance[u4Handle].u4AUConsumedCnt == 0)
				pEsmIOBufInfo->u4Status = GAU_S_DISCONTINUOUS;
			else
				pEsmIOBufInfo->u4Status = GAU_S_OK;

			/* convert to phisical address*/
			if (ES_V == eType) {
				pEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.u8Pts =
					g_rDmxGauManager.arGAUInstance[u4Handle].u4AUConsumedCnt * (DMX_PTS_1S / 30);
			} else if (ES_A == eType) {
				pEsmIOBufInfo->rAU.rAudioAU.rAUInfo.rInfo.u8Pts =
					g_rDmxGauManager.arGAUInstance[u4Handle].u4AUConsumedCnt * (DMX_PTS_1S / 30);
			} else if (ES_SP == eType) {
				pEsmIOBufInfo->rAU.rSPStruct.u8StartPts =
					g_rDmxGauManager.arGAUInstance[u4Handle].u4AUConsumedCnt * (DMX_PTS_1S / 30);
				pEsmIOBufInfo->rAU.rSPStruct.i8Delay = (s64)(DMX_PTS_1S / 30);
			}
		}

		++g_rDmxGauManager.arGAUInstance[u4Handle].u4AUConsumedCnt;

		MM_RETURN(RET_DMX_OK);
#endif /* DMX_DISABLE_GET_REALAU*/

		mrRet = ESM_AUTableGetAUInfo(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
			u4RdIdx, &(pvAU));
		if (DMX_FAILED(mrRet)) {
			pEsmIOBufInfo->ptrFifoSPAddr = GAU_E_FAIL;
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
				TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_AUTableGetAUInfo\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			MM_RETURN(mrRet);
		}

		if (NULL == pvAU) {
			pEsmIOBufInfo->ptrFifoSPAddr = GAU_E_FAIL;
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
				TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail for pvAU == NULL, RdIdx: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4RdIdx);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		mrRet = ESM_AUTableGetAUExtInfo(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
			u4RdIdx, &(pvExtAU));
		if (DMX_FAILED(mrRet)) {
			pEsmIOBufInfo->ptrFifoSPAddr = GAU_E_FAIL;
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
				TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_AUTableGetAUExtInfo\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			MM_RETURN(mrRet);
		}
		mrRet = ESM_AUTableIncRdIdx(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, 1);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
				TEXT(
					"[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_AUTableIncRdIdx(u4RdIdx: 0x%x, eType: 0x%x, i4Ret: 0x%x)\r\n"
				),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4RdIdx, eType, mrRet);
			/*DMX_ASSERT(FALSE);*/
			pEsmIOBufInfo->u4Status = GAU_E_FAIL;
			MM_RETURN(RET_DMX_NO_AU);
		}

		mrRet = ESM_GetESType(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &eType);
		if (DMX_FAILED(mrRet)) {
			pEsmIOBufInfo->ptrFifoSPAddr = GAU_E_FAIL;
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
				TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_GetESType\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		/* convert to phisical address*/
		if (ES_V == eType) {
			mrRet = GAU_GetVideoAU(u4Handle, pEsmIOBufInfo, u4RdIdx, pvAU);
		} else if (ES_A == eType) {
			mrRet = GAU_GetAudioAU(u4Handle, pEsmIOBufInfo, pvExtAU, u4RdIdx, pvAU);
		} else if (ES_SP == eType) {
			mrRet = GAU_GetSPAU(u4Handle, pEsmIOBufInfo, u4RdIdx, pvAU);
		} else if (ES_SECTION == eType) {
			mrRet = GAU_GetSectionAU(u4Handle, pEsmIOBufInfo, u4RdIdx, pvAU);
		} else {
			pEsmIOBufInfo->u4Status = GAU_E_FAIL;
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU,
				TEXT("[GAU] %s line %d fail in error eType : 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, eType);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		++g_rDmxGauManager.arGAUInstance[u4Handle].u4AUConsumedCnt;

#if 1 /* DMX_GAU_GETAU_WAIT_THRESHOLD_EVT*/
		if ((RET_DMX_NO_AU == mrRet) &&
			(GAU_E_TIMEOUT == pEsmIOBufInfo->u4Status)) {
			continue;
		}
#endif /* DMX_GAU_GETAU_WAIT_THRESHOLD_EVT*/

		break;
	}

	MM_RETURN(mrRet);
}

MRESULT GAU_ReleaseAU(u32 u4Handle, void *pvIOBuf, bool fgFF)
{
	ESM_IO_BUF_INFO *prEsmIOBufInfo = (ESM_IO_BUF_INFO *)pvIOBuf;
	void	*pvAU  = NULL;
	GAU_Q_ELEM_T *prElem = NULL;
	uintptr_t	ptrAuEa = 0;
	u32	u4RdIdx = ESM_INVALID_INDEX;
	ES_TYPE eType = ES_NONE;
	AU_TYPE eAuType = AU_DATA;
	MRESULT mrRet	= RET_DMX_OK;
	bool	fgSendIBC = FALSE;
	u32 u4ESMHandle = 0;
	uintptr_t ptrRelAUAddrSa = DMX_INVALID_UINTPTR_T;
	uintptr_t ptrQueAUAddrSa = DMX_INVALID_UINTPTR_T;

	if (MAX_GAU_INSTANCE_CNT <= u4Handle) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_RELAU,
			TEXT("[GAU] %s line %d fail for HANDLE(%d) > MAX_GAU_INSTANCE_CNT(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, MAX_GAU_INSTANCE_CNT);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

#if (DMX_DISABLE_CFA || DMX_DISABLE_DMA_DATA || DMX_DISABLE_GET_REALAU)
	MM_RETURN(RET_DMX_OK);
#elif DMX_DISABLE_COMP_AU
	mrRet = ESM_GetESType(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &eType);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_RELAU,
			TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_GetESType\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		MM_RETURN(mrRet);
	}
	#if DMX_DISABLE_COMP_AUDIOAU
	if (ES_V == eType) {
		VCodeC eVCodec = g_rPSRHalStruct.eVideoCodec;
		#if DMX_DISABLE_COMP_MPEG2AU
		if (VC_MPEG2 == eVCodec)
			MM_RETURN(RET_DMX_OK);
		#endif /* DMX_DISABLE_COMP_MPEG2AU*/

		#if DMX_DISABLE_COMP_MPEG4AU
		if ((VC_MPEG4 == eVCodec) ||
			(VC_DIVX4 == eVCodec) ||
			(VC_DIVX6 == eVCodec) ||
			(VC_H263  == eVCodec)) {
			MM_RETURN(RET_DMX_OK);
		}
		#endif /* DMX_DISABLE_COMP_MPEG4AU*/

		#if DMX_DISABLE_COMP_AVCAU
		if (VC_H264 == eVCodec)
			MM_RETURN(RET_DMX_OK);
		#endif /* DMX_DISABLE_COMP_AVCAU*/

		#if DMX_DISABLE_COMP_VC1AU
		if (VC_VC1 == eVCodec)
			MM_RETURN(RET_DMX_OK);
		#endif /* DMX_DISABLE_COMP_VC1AU*/
	}
	#endif /* DMX_DISABLE_COMP_AUDIOAU*/

	#if DMX_DISABLE_COMP_AUDIOAU
	if (ES_A == eType)
		MM_RETURN(RET_DMX_OK);
	#endif /* DMX_DISABLE_COMP_AUDIOAU*/

	#if DMX_DISABLE_COMP_SPAU
	if (ES_SP == eType)
		MM_RETURN(RET_DMX_OK);
	#endif /* DMX_DISABLE_COMP_SPAU*/

	#if DMX_DISABLE_COMP_OTHAU
	if (ES_SECTION == eType)
		MM_RETURN(RET_DMX_OK);
	#endif /* DMX_DISABLE_COMP_OTHAU*/
#endif

	prElem = g_rDmxGauManager.arGAUInstance[u4Handle].rGetAUQueue.rInUseList.prHead;
	while (NULL != prElem) {
		u4RdIdx = GAU_Q_GetInUseAUIdx(&(g_rDmxGauManager.arGAUInstance[u4Handle].rGetAUQueue), prElem);

		if (ESM_INVALID_INDEX == u4RdIdx) {
			DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_RELAU,
				TEXT("[GAU] %s line %d (u4Handle: 0x%x) u4RdIdx is invalid in ESM_AUTableGetRdIdx\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			/*DMX_ASSERT(FALSE);*/
			MM_RETURN(RET_DMX_OK);
		}

		mrRet = ESM_AUTableGetAUInfo(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
			u4RdIdx, &(pvAU));
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_RELAU,
				TEXT(
					"[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_AUTableGetAUInfo(u4RdIdx: 0x%x).\r\n"
				),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4RdIdx);
			/*DMX_ASSERT(FALSE);*/
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		if (NULL == pvAU) {
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_RELAU,
				TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail, prAU == NULL, (u4RdIdx: 0x%x).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4RdIdx);
			/*DMX_ASSERT(FALSE);*/
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		mrRet = ESM_GetESType(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &eType);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_RELAU,
				TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_GetESType\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			MM_RETURN(mrRet);
		}
		if (ES_V == eType) {
			if (NULL == prEsmIOBufInfo) {
				DMXLOG_ERROR(TEXT("[GAU] %s line %d(VIDEO, u4Handle: 0x%x)")
					TEXT("is fail, pvIOBuf == NULL.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			ptrAuEa = ((AU_VPic *)pvAU)->rAUInfo.rInfo.ptrEAddr;
			ptrQueAUAddrSa = ((AU_VPic *)pvAU)->rAUInfo.rInfo.ptrSAddr;
			ptrRelAUAddrSa = prEsmIOBufInfo->rAU.rVPicAU.rAUInfo.rInfo.ptrSAddr + prEsmIOBufInfo->ptrFifoSVAddr;
			DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_RELAU_V,
				TEXT("[GAU] %s (Video) ---> eDataType: %d, AUIdx(%d), ")
				TEXT("Pts("DMX_PTS_LOGSTR"), VType(0x%x), AUSa(0x%x), AUEa(0x%x)\r\n"),
				DMX_FUNC_NAME,
				((AU_VPic *)pvAU)->eAuType, u4RdIdx,
				DMX_PTS_LOG_MS(((AU_VPic *)pvAU)->rAUInfo.rInfo.u8Pts),
				DMX_PTS_LOG_PTS(((AU_VPic *)pvAU)->rAUInfo.rInfo.u8Pts),
				((AU_VPic *)pvAU)->rAUInfo.rInfo.u4VType,
				((AU_VPic *)pvAU)->rAUInfo.rInfo.ptrSAddr,
				((AU_VPic *)pvAU)->rAUInfo.rInfo.ptrEAddr);
		} else if (ES_A == eType) {
			if (NULL == prEsmIOBufInfo) {
				DMXLOG_ERROR(TEXT("[GAU] %s line %d(AUDIO, u4Handle: 0x%x)")
					TEXT("is fail, pvIOBuf == NULL.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			ptrAuEa = ((AU_AUDIO *)pvAU)->ptrEAddr;
			ptrQueAUAddrSa = ((AU_AUDIO*)pvAU)->ptrSAddr;
			if (ptrQueAUAddrSa > ((AU_AUDIO*)pvAU)->ptrEAddr) {
				ptrQueAUAddrSa = g_rDmxGauManager.arGAUInstance[u4Handle].rSelfFifoInfo.ptrFifoVSa;
			}

			ptrRelAUAddrSa = prEsmIOBufInfo->rAU.rAudioAU.ptrSAddr + g_rDmxGauManager.arGAUInstance[u4Handle].ptrMMRsvBufBase;
			DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_RELAU_A,
				TEXT("[GAU] %s (Audio) ---> eDataType(%d), AUIdx(%d), ")
				TEXT("Pts("	DMX_PTS_LOGSTR "), AUSa(0x%x), AUEa(0x%x)\r\n"),
				DMX_FUNC_NAME, ((AU_AUDIO *)pvAU)->eAuType, u4RdIdx,
				DMX_PTS_LOG_MS(((AU_AUDIO *)pvAU)->rAUInfo.rInfo.u8Pts),
				DMX_PTS_LOG_PTS(((AU_AUDIO *)pvAU)->rAUInfo.rInfo.u8Pts),
				((AU_AUDIO *)pvAU)->ptrSAddr,
				((AU_AUDIO *)pvAU)->ptrEAddr);
		} else if (ES_SP == eType) {
			if (NULL == prEsmIOBufInfo) {
				DMXLOG_ERROR(TEXT("[GAU] %s line %d(SP, u4Handle: 0x%x)")
					TEXT("is fail, pvIOBuf == NULL.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_RELAU_SP,
				TEXT("[GAU] %s (SP) ---> eDataType(%d), AUIdx(%d), ")
				TEXT("Pts(" DMX_PTS_LOGSTR "), SA(0x%08x), Size(0x%08x)\r\n"),
				DMX_FUNC_NAME, ((AU_SP *)pvAU)->eAuType, u4RdIdx,
				DMX_PTS_LOG_MS(((AU_SP *)pvAU)->rAUInfo.rInfo.u8StartPts),
				DMX_PTS_LOG_PTS(((AU_SP *)pvAU)->rAUInfo.rInfo.u8StartPts),
				((AU_SP *)pvAU)->rAUInfo.rInfo.u8StartPts,
				((AU_SP *)pvAU)->rAUInfo.rInfo.ptrAddr,
				((AU_SP *)pvAU)->rAUInfo.rInfo.u4Size);
			ptrQueAUAddrSa = ((AU_SP*)pvAU)->rAUInfo.rInfo.ptrAddr;
			if (ptrQueAUAddrSa > prEsmIOBufInfo->rAU.rSPStruct.ptrAUEndAddrOfFifo) {
				ptrQueAUAddrSa = prEsmIOBufInfo->rAU.rSPStruct.ptrVirSA;
			}
			ptrRelAUAddrSa = prEsmIOBufInfo->rAU.rSPStruct.ptrUserVirSA +
				g_rDmxGauManager.arGAUInstance[u4Handle].ptrMMRsvBufBase;

			ptrAuEa = prEsmIOBufInfo->rAU.rSPStruct.ptrAUEndAddrOfFifo;
			prEsmIOBufInfo->rAU.rSPStruct.ptrUserVirSA = 0;
		} else if (ES_SECTION == eType) {
			if (NULL == prEsmIOBufInfo) {
				DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_RELAU,
					TEXT("[GAU] %s line %d (Section) fail for invalid args, pvIOBuf == NULL.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			ptrAuEa = prEsmIOBufInfo->rAU.rSectionAU.ptrEAddr;
			DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_RELAU_SEC,
				TEXT("[GAU] %s (SP) ---> eDataType(%d), AUIdx(%d), SA(0x%08x), EA(0x%08x)\r\n"),
				DMX_FUNC_NAME, ((AU_SP *)pvAU)->eAuType, u4RdIdx,
				prEsmIOBufInfo->rAU.rSectionAU.ptrSAddr,
				prEsmIOBufInfo->rAU.rSectionAU.ptrEAddr);

			prEsmIOBufInfo->rAU.rSectionAU.ptrUserVirSA = 0;
			DMX_FreeHwMemory((void *)(prEsmIOBufInfo->rAU.rSectionAU.ptrVirSA));
			prEsmIOBufInfo->rAU.rSectionAU.ptrVirSA = 0;
		} else {
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_RELAU,
				TEXT("[GAU] %s line %d (Section) is fail for invalid eType(0x%x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, eType);
			MM_RETURN(RET_DMX_UNEXPECT);
		}
		if (ptrRelAUAddrSa != ptrQueAUAddrSa) {
			DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_RELAU,
				TEXT("[GAU] %s line %d (u4Handle: 0x%x) u4RdIdx is invalid in ESM_AUTableGetRdIdx\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		} else {
			break;
		}
		prElem = prElem->prNext;
	}

	if (NULL == prElem) {
		DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_RELAU,
				TEXT("[GAU] %s line %d (u4Handle: 0x%x) No AU can be released.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		MM_RETURN(RET_DMX_OK);
	}

	mrRet = ESM_FifoSetRdPtr(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
		ptrAuEa, fgFF);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_RELAU,
			TEXT(
				"[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_FifoSetRdPtr, ptrAuEa: 0x%p, eType: 0x%x, u4RdIdx: 0x%x, i4Ret: 0x%x\r\n"
			),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, ptrAuEa, eType, u4RdIdx, mrRet);
		/*DMX_ASSERT(FALSE);*/
		/*MM_RETURN(RET_DMX_UNEXPECT);*/
		MM_RETURN(RET_DMX_OK);
	}

	if (ES_V == eType) {
		eAuType = ((AU_VPic *)pvAU)->eAuType;
		fgSendIBC = ((AU_VPic *)pvAU)->fgIBCSent;
		dmx_memset(pvAU, 0, sizeof(AU_VPic));
	} else if (ES_A == eType) {
		eAuType = ((AU_AUDIO *)pvAU)->eAuType;
		dmx_memset(pvAU, 0, sizeof(AU_AUDIO));
	} else if (ES_SP == eType) {
		eAuType = ((AU_SP *)pvAU)->eAuType;
		dmx_memset(pvAU, 0, sizeof(AU_SP));
	} else if (ES_SECTION == eType) {
		eAuType = ((AU_SECTION *)pvAU)->eAuType;
		dmx_memset(pvAU, 0, sizeof(AU_SECTION));
	} else {
		/*do nothing*/
	}

	if (ES_A == eType) {
		if (AU_CMD == eAuType) {
			DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_RELAU_A,
				TEXT(
					"[GAU] %s (u4Handle: 0x%x, Audio) --> GAU_SetEvent(GAU_EV_AUCMD_RELEASE)!\r\n"
				),
				DMX_FUNC_NAME, u4Handle);
			mrRet = GAU_SetEvent(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
				GAU_EV_AUCMD_RELEASE);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_RELAU_A,
					TEXT(
						"[GAU] %s line %d (u4Handle: 0x%x, Audio) fail in GAU_GetEvent(GAU_EV_AUCMD_RELEASE)!\r\n"
					),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			}
		}
	} else if (ES_V == eType) {
		if ((AU_CMD == eAuType) &&
			fgSendIBC) {
			DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_RELAU_V,
				TEXT(
					"[GAU] %s (u4Handle: 0x%x, Video) --> GAU_SetEvent(GAU_EV_AUCMD_RELEASE)!\r\n"
				),
				DMX_FUNC_NAME, u4Handle);
			mrRet = GAU_SetEvent(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
				GAU_EV_AUCMD_RELEASE);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_RELAU_V,
					TEXT(
						"[GAU] %s line %d (u4Handle: 0x%x, Video) fail in GAU_GetEvent(GAU_EV_AUCMD_RELEASE)!\r\n"
					),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			}
		}
	} else {
		/*do nothing*/
	}

	mrRet = GAU_Q_ReleaseAU(&(g_rDmxGauManager.arGAUInstance[u4Handle].rGetAUQueue), u4RdIdx);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_RELAU,
			TEXT(
				"[GAU] %s line %d (u4Handle: 0x%x) fail in GAU_Q_ReleaseAU, eType: 0x%x, u4RdIdx: 0x%x, i4Ret: 0x%x\r\n"
			),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, eType, u4RdIdx, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(mrRet);
}

MRESULT GAU_GetEvent(u32 u4Handle, EV_GRP_EVENT_T u4WaitOnEvent,
					EV_GRP_EVENT_T *pu4ReceiveEvent, u32 u4TimeOut)
{
	s32 i4Ret = OSR_OK;

	if (MAX_GAU_INSTANCE_CNT <= u4Handle) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s failed for u4Handle(%d) > MAX_GAU_INSTANCE_CNT(%d)\r\n"),
			DMX_FUNC_NAME, u4Handle, MAX_GAU_INSTANCE_CNT);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (NULL == g_rDmxGauManager.arGAUInstance[u4Handle].hAUInEG) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT(
				"[GAU] %s failed for the GAU instance's Event group hasn't been created (u4Handle: 0x%x)\r\n"
			),
			DMX_FUNC_NAME, u4Handle);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}

	i4Ret = x_ev_group_wait_event_timeout(
		(uintptr_t)g_rDmxGauManager.arGAUInstance[u4Handle].hAUInEG,
		u4WaitOnEvent, pu4ReceiveEvent,
		X_EV_OP_OR_CONSUME, u4TimeOut);

	if (OSR_OK == i4Ret)
		MM_RETURN(RET_DMX_OK);
	DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
		TEXT("[GAU] %s failed in x_ev_group_wait_event_timeout(u4Handle: 0x%x, u4TimeOut: %d)\r\n"),
		DMX_FUNC_NAME, u4Handle, u4TimeOut);

	MM_RETURN(RET_DMX_OS_OPERA_FAIL);
}

MRESULT GAU_ResetEvent(u32 u4Handle, EV_GRP_EVENT_T u4WaitOnEvent)
{
	EV_GRP_EVENT_T u4Events = 0;
	s32 i4Ret = OSR_OK;

	if (MAX_GAU_INSTANCE_CNT <= u4Handle) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s failed for u4Handle(%d) > MAX_GAU_INSTANCE_CNT(%d)\r\n"),
			DMX_FUNC_NAME, u4Handle, MAX_GAU_INSTANCE_CNT);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (NULL == g_rDmxGauManager.arGAUInstance[u4Handle].hAUInEG) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT(
				"[GAU] %s failed for the GAU instance's Event group hasn't been created (u4Handle: 0x%x)\r\n"
			),
			DMX_FUNC_NAME, u4Handle);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}

	while (1) {
		i4Ret = x_ev_group_wait_event_timeout(
			(uintptr_t)g_rDmxGauManager.arGAUInstance[u4Handle].hAUInEG,
			u4WaitOnEvent, &u4Events,
			X_EV_OP_OR_CONSUME, 0);

		if (OSR_OK != i4Ret)
			break;
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT GAU_SetEvent(u32 u4Handle, EV_GRP_EVENT_T u4SetEvent)
{
	if (MAX_GAU_INSTANCE_CNT <= u4Handle) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s failed for u4Handle(%d) > MAX_GAU_INSTANCE_CNT(%d)\r\n"),
			DMX_FUNC_NAME, u4Handle, MAX_GAU_INSTANCE_CNT);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (NULL == g_rDmxGauManager.arGAUInstance[u4Handle].hAUInEG) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT(
				"[GAU] %s failed for the GAU instance's Event group hasn't been created (u4Handle: 0x%x)\r\n"
			),
			DMX_FUNC_NAME, u4Handle);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}

	if (x_ev_group_set_event((uintptr_t)g_rDmxGauManager.arGAUInstance[u4Handle].hAUInEG,
		u4SetEvent, X_EV_OP_OR) == OSR_OK) {
		MM_RETURN(RET_DMX_OK);
	}

	DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
		TEXT("[GAU] %s failed in x_ev_group_set_event(u4Handle: %d, u4SetEvent: 0x%x)\r\n"),
		DMX_FUNC_NAME, u4Handle, u4SetEvent);

	MM_RETURN(RET_DMX_OS_OPERA_FAIL);
}

MRESULT GAU_Enable(u32 u4Handle, bool fgEnable)
{
	if (MAX_GAU_INSTANCE_CNT <= u4Handle)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	if (g_rDmxGauManager.arGAUInstance[u4Handle].fgGetAUEnable != fgEnable) {
		if (!fgEnable) {
			g_rDmxGauManager.arGAUInstance[u4Handle].fgGetAUEnable = fgEnable;
			DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
				TEXT("[GAU] GAU_ENABLE(u4Handle: %d) -- fgEnable: %s\r\n"),
				u4Handle, (fgEnable ? "TRUE" : "FALSE"));
			if (OSR_OK != x_ev_group_set_event((uintptr_t)g_rDmxGauManager.arGAUInstance[u4Handle].hAUInEG,
				GAU_EV_DISABLE_GETAU, X_EV_OP_OR)) {
				DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
					TEXT("[GAU] %s(u4Handle: 0x%x) fail in Set Event GAU_EV_DISABLE_GETAU!!!\r\n"),
					DMX_FUNC_NAME, u4Handle);
				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
			}
		} else {
			EV_GRP_EVENT_T ev = 0;

			x_ev_group_wait_event_timeout(
				(uintptr_t)g_rDmxGauManager.arGAUInstance[u4Handle].hAUInEG,
				GAU_EV_DISABLE_GETAU, &ev, X_EV_OP_OR_CONSUME, 0);
			g_rDmxGauManager.arGAUInstance[u4Handle].fgGetAUEnable = fgEnable;
			DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
				TEXT("[GAU] GAU_ENABLE(u4Handle: %d) -- fgEnable: %s\r\n"),
				u4Handle, (fgEnable ? "TRUE" : "FALSE"));
		}
	}

	MM_RETURN(RET_DMX_OK);
}

/****************************************************************************************
** GAU_CheckFifo
** Function: print fifo usage information
** Unused.
***************************************************************************************/
MRESULT GAU_CheckFifo(u32 u4Handle, void *pvIOBuf)
{
	ESM_IO_FIFO_USAGE  *pEsmIOBufInfo = (ESM_IO_FIFO_USAGE *)pvIOBuf;
	DMX_FIFO_INFO_T    *pFifo = NULL;
	ES_TYPE	eType = ES_NONE;
	MRESULT	mrRet = RET_DMX_OK;

	if ((NULL == pEsmIOBufInfo) ||
		(MAX_GAU_INSTANCE_CNT <= u4Handle)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s (u4Handle: 0x%x) fail for invalid args\r\n"),
			DMX_FUNC_NAME, u4Handle);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	mrRet = ESM_FifoGetInfo(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
		(void **)&pFifo);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s (u4Handle: 0x%x) fail in ESM_FifoGetInfo, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, u4Handle, mrRet);
		MM_RETURN(mrRet);
	}

	if (NULL == pFifo) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s (u4Handle: 0x%x) fail for no corresponding fifo, StmType: %d\r\n"),
			DMX_FUNC_NAME, u4Handle, g_rDmxGauManager.arGAUInstance[u4Handle].u4StmType);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	mrRet = ESM_FifoGetAvailDataSize(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle,
		&(pEsmIOBufInfo->u4FifoAvailSize));
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_FifoGetAvailDataSize, mrRet: 0x%x\r\n"),
		   DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
		MM_RETURN(mrRet);
	}

	pEsmIOBufInfo->u4FifoSize = pFifo->ptrEa - pFifo->ptrSa;

	if (0 == pEsmIOBufInfo->u4FifoSize) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d(u4Handle: 0x%x) exit for fifo size is 0\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		pEsmIOBufInfo->u4FifoUsage = 0;
		MM_RETURN(RET_DMX_OK);
	}

	pEsmIOBufInfo->u4FifoUsage =
		pEsmIOBufInfo->u4FifoAvailSize * 100 / pEsmIOBufInfo->u4FifoSize;

	mrRet = ESM_GetESType(g_rDmxGauManager.arGAUInstance[u4Handle].u4ESHandle, &eType);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d(u4Handle: 0x%x) fail in ESM_GetESType, i4Ret: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
		MM_RETURN(mrRet);
	}

	pEsmIOBufInfo->eType = eType;

	MM_RETURN(RET_DMX_OK);
}

MRESULT GAU_SetThreshold(u32 u4Handle, u32 u4Threshold)
{
	void *pvDmxInst = 0;
	u32 u4Idx = 0;

	if (MAX_GAU_INSTANCE_CNT <= u4Handle) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
			TEXT("[GAU] %s failed for u4Handle(%d) > MAX_GAU_INSTANCE_CNT(%d)\r\n"),
			DMX_FUNC_NAME, u4Handle, MAX_GAU_INSTANCE_CNT);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!g_rDmxGauManager.arGAUInstance[u4Handle].fgUsed) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
			TEXT("[GAU] %s failed for GAU instance(%d)'s fgUsed is FALSE: 0x%x\r\n"),
			DMX_FUNC_NAME, u4Handle);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	pvDmxInst = g_rDmxGauManager.arGAUInstance[u4Handle].pvDmxInst;
	if (0 == pvDmxInst) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
			TEXT("[GAU] %s(Handle: 0x%x) failed for GAU instance(%d)'s pvDmxInst error\r\n"),
			DMX_FUNC_NAME, u4Handle);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	/* Section GAU instance doesn't support threshold mechanism*/
	if (SPT_DATA_SECTION == g_rDmxGauManager.arGAUInstance[u4Handle].u4StmType)
		MM_RETURN(RET_DMX_OK);

	g_rDmxGauManager.arGAUInstance[u4Handle].u4Threshold = u4Threshold;

	if (u4Threshold > 0) {
		DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
			TEXT("[GAU] %s (Handle: %d, Threshold: %d)\r\n"),
			DMX_FUNC_NAME, u4Handle, u4Threshold);
		g_rDmxGauManager.fgReachThreshold = FALSE;
		/* if anyone's threshold is not 0, we set all the threshold flag to be false
		 to make all the upper get-au module to wait reach threshold */
		for (u4Idx = 0; u4Idx < MAX_GAU_INSTANCE_CNT; u4Idx++) {
			if (!g_rDmxGauManager.arGAUInstance[u4Idx].fgUsed)
				continue;
			if (g_rDmxGauManager.arGAUInstance[u4Idx].pvDmxInst != pvDmxInst)
				continue;

			/* section doesn't support threshold, so skip it*/
			if ((DMX_INVALID_UINT32 == g_rDmxGauManager.arGAUInstance[u4Idx].u4StmType) ||
				(SPT_DATA_SECTION == g_rDmxGauManager.arGAUInstance[u4Idx].u4StmType))
				continue;
			g_rDmxGauManager.arGAUInstance[u4Idx].fgReachThreshold = FALSE;
		}
	} else {
		bool   fgAllZero = TRUE;

		for (u4Idx = 0; u4Idx < MAX_GAU_INSTANCE_CNT; u4Idx++) {
			if (!g_rDmxGauManager.arGAUInstance[u4Idx].fgUsed)
				continue;
			if (g_rDmxGauManager.arGAUInstance[u4Idx].pvDmxInst != pvDmxInst)
				continue;

			/* section doesn't support threshold, so skip it*/
			if ((DMX_INVALID_UINT32 == g_rDmxGauManager.arGAUInstance[u4Idx].u4StmType) ||
				(SPT_DATA_SECTION == g_rDmxGauManager.arGAUInstance[u4Idx].u4StmType)) {
				continue;
			}

			if (g_rDmxGauManager.arGAUInstance[u4Idx].u4Threshold > 0) {
				fgAllZero = FALSE;
				break;
			}
		}
		if (fgAllZero) {
			/* if all the thresholds are 0, we set all the threshold flag to be true
			 to make all the upper get-au module not to wait any threshold*/
			for (u4Idx = 0; u4Idx < MAX_GAU_INSTANCE_CNT; u4Idx++) {
				if (!g_rDmxGauManager.arGAUInstance[u4Idx].fgUsed)
					continue;
				if (g_rDmxGauManager.arGAUInstance[u4Idx].pvDmxInst != pvDmxInst)
					continue;
				/* section doesn't support threshold, so skip it*/
				if ((DMX_INVALID_UINT32 == g_rDmxGauManager.arGAUInstance[u4Idx].u4StmType) ||
					(SPT_DATA_SECTION == g_rDmxGauManager.arGAUInstance[u4Idx].u4StmType))
					continue;
				g_rDmxGauManager.arGAUInstance[u4Idx].fgReachThreshold = TRUE;
			}
			g_rDmxGauManager.fgReachThreshold = TRUE;
		} else {
			/* if anyone's threshold is not 0, we set all the threshold flag to be false
			 to make all the upper get-au module to wait reach threshold*/
			for (u4Idx = 0; u4Idx < MAX_GAU_INSTANCE_CNT; u4Idx++) {
				if (!g_rDmxGauManager.arGAUInstance[u4Idx].fgUsed)
					continue;
				if (g_rDmxGauManager.arGAUInstance[u4Idx].pvDmxInst != pvDmxInst)
					continue;
				/* section doesn't support threshold, so skip it*/
				if ((DMX_INVALID_UINT32 == g_rDmxGauManager.arGAUInstance[u4Idx].u4StmType) ||
					(SPT_DATA_SECTION == g_rDmxGauManager.arGAUInstance[u4Idx].u4StmType))
					continue;
				g_rDmxGauManager.arGAUInstance[u4Idx].fgReachThreshold = FALSE;
			}
			g_rDmxGauManager.fgReachThreshold = FALSE;
		}
	}

	DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
		TEXT("[GAU] %s (Handle: %d, Threshold: %d) success\r\n"),
		DMX_FUNC_NAME, u4Handle, u4Threshold);

	MM_RETURN(RET_DMX_OK);
}

bool GAU_IsReachThreshold(void)
{
	return g_rDmxGauManager.fgReachThreshold;
}

MRESULT GAU_SetSkipThreshold(u32 u4Handle)
{
	MRESULT mrRet = RET_DMX_OK;

	if (MAX_GAU_INSTANCE_CNT <= u4Handle) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
			TEXT("[GAU] %s failed for u4Handle(%d) > MAX_GAU_INSTANCE_CNT(%d)\r\n"),
			DMX_FUNC_NAME, u4Handle, MAX_GAU_INSTANCE_CNT);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!g_rDmxGauManager.arGAUInstance[u4Handle].fgUsed) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
			TEXT("[GAU] %s(u4Handle: %d) fail for GAU instance unused!\r\n"),
			DMX_FUNC_NAME, u4Handle);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!g_rDmxGauManager.arGAUInstance[u4Handle].fgReachThreshold) {
		mrRet = GAU_SetEvent(u4Handle, GAU_EV_SKIP_THRESHOLD);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
				TEXT("[GAU] %s(u4Handle: %d) fail in GAU_SetEvent(GAU_EV_SKIP_THRESHOLD)!\r\n"),
				DMX_FUNC_NAME, u4Handle);
			MM_RETURN(mrRet);
		}
	}

	MM_RETURN(RET_DMX_OK);
}

void GAU_DisableThreshold(void)
{
	if (!g_rDmxGauManager.fgReachThreshold) {
		u32 u4Idx = 0;

		for (u4Idx = 0; u4Idx < MAX_GAU_INSTANCE_CNT; ++u4Idx) {
			if (!(g_rDmxGauManager.arGAUInstance[u4Idx].fgUsed))
				break;
			g_rDmxGauManager.arGAUInstance[u4Idx].u4Threshold = 0;
		}

		g_rDmxGauManager.fgReachThreshold = TRUE;

		DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
			TEXT("[GAU] %s -- REACH THRESHOLD NOW!!!\r\n"),
			DMX_FUNC_NAME);

		for (u4Idx = 0; u4Idx < MAX_GAU_INSTANCE_CNT; u4Idx++) {
			if (!g_rDmxGauManager.arGAUInstance[u4Idx].fgUsed)
				continue;

			if ((DMX_INVALID_UINT32 == g_rDmxGauManager.arGAUInstance[u4Idx].u4StmType) ||
				(SPT_DATA_SECTION == g_rDmxGauManager.arGAUInstance[u4Idx].u4StmType))
				continue;

			if (OSR_OK != x_ev_group_set_event((uintptr_t)g_rDmxGauManager.arGAUInstance[u4Idx].hAUInEG,
					GAU_EV_REACH_THRESHOLD, X_EV_OP_OR)) {
				DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
					TEXT("[GAU] %s(u4Idx: 0x%x) fail in Set Event GAU_EV_REACH_THRESHOLD!!!\r\n"),
					DMX_FUNC_NAME, u4Idx);
				return;
			}
			DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
				TEXT("[GAU] %s(u4Idx: 0x%x) -- Set Event GAU_EV_REACH_THRESHOLD!!!\r\n"),
				DMX_FUNC_NAME, u4Idx);
		}
	}
}

void GAU_CheckThreshold(u32 u4Handle)
{
	if (!g_rDmxGauManager.fgReachThreshold) {
		if (MAX_GAU_INSTANCE_CNT <= u4Handle)
			return;

		if (RET_DMX_OK == JudgeThreshold(u4Handle)) {
			u32 u4Idx = 0;

			g_rDmxGauManager.fgReachThreshold = TRUE;

			DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
				TEXT("[GAU] %s (u4Handle: 0x%x) REACH THRESHOLD NOW!!!\r\n"),
				DMX_FUNC_NAME, u4Handle);

			for (u4Idx = 0; u4Idx < MAX_GAU_INSTANCE_CNT; u4Idx++) {
				if (!g_rDmxGauManager.arGAUInstance[u4Idx].fgUsed)
					continue;

				if ((DMX_INVALID_UINT32 == g_rDmxGauManager.arGAUInstance[u4Idx].u4StmType) ||
					(SPT_DATA_SECTION == g_rDmxGauManager.arGAUInstance[u4Idx].u4StmType))
					continue;

				if (OSR_OK != x_ev_group_set_event((uintptr_t)g_rDmxGauManager.arGAUInstance[u4Idx].hAUInEG,
						GAU_EV_REACH_THRESHOLD, X_EV_OP_OR)) {
					DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
						TEXT(
							"[GAU] %s(u4Idx: 0x%x) fail in Set Event GAU_EV_REACH_THRESHOLD!!!\r\n"
						),
						DMX_FUNC_NAME, u4Idx);
					return;
				}
				DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
					TEXT("[GAU] %s(u4Idx: 0x%x) -- Set Event GAU_EV_REACH_THRESHOLD!!!\r\n"),
					DMX_FUNC_NAME, u4Idx);
			}
		}
	}
}

void GAU_ClearThreshold(u32 u4Handle)
{
	u32 u4Idx = 0;

	if (MAX_GAU_INSTANCE_CNT <= u4Handle)
		return;

	if (!(g_rDmxGauManager.arGAUInstance[u4Handle].fgUsed))
		return;

	g_rDmxGauManager.arGAUInstance[u4Handle].fgReachThreshold = TRUE;
	g_rDmxGauManager.arGAUInstance[u4Handle].u4Threshold = 0;

	for (u4Idx = 0; u4Idx < MAX_GAU_INSTANCE_CNT; u4Idx++) {
		if ((!g_rDmxGauManager.arGAUInstance[u4Idx].fgUsed) ||
			(g_rDmxGauManager.arGAUInstance[u4Idx].fgReachThreshold))
			continue;
	}

	if (u4Idx >= MAX_GAU_INSTANCE_CNT)
		g_rDmxGauManager.fgReachThreshold = TRUE;
}

static MRESULT JudgeThreshold(u32 u4Handle)
{
	u32	u4Idx	  = 0;
	u32	u4FifoSz  = 0;
	u32	u4AvailSz = 0;
	bool	fgAllThreshodZero = TRUE;
	bool	fgOneThresholdOK = FALSE;
	u32	u4ThresholdCnt = 0;
	void *pvDmxInst = 0;
	MRESULT mrSubRet  = RET_DMX_UNEXPECT;
	MRESULT mrRet	  = RET_DMX_NO_REACH_THRESHOLD;

	if (!(g_rDmxGauManager.arGAUInstance[u4Handle].fgUsed)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
			TEXT("[GAU] %s line %d (u4Handle: 0x%x) exit for GAU(Handle: %d) is not in used\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		return RET_DMX_OK;
	}
	pvDmxInst = g_rDmxGauManager.arGAUInstance[u4Handle].pvDmxInst;
	if (0 == pvDmxInst) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
			TEXT("[GAU] %s(Handle: 0x%x) failed for GAU instance(%d)'s pvDmxInst error\r\n"),
			DMX_FUNC_NAME, u4Handle);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	/* Get Threshold Cnt;*/
	for (u4Idx = 0; u4Idx < MAX_GAU_INSTANCE_CNT; ++u4Idx) {
		if (!(g_rDmxGauManager.arGAUInstance[u4Idx].fgUsed))
			continue;
		if (g_rDmxGauManager.arGAUInstance[u4Idx].pvDmxInst != pvDmxInst)
			continue;

		if (0 != g_rDmxGauManager.arGAUInstance[u4Idx].u4Threshold) {
			fgAllThreshodZero = FALSE;
			u4ThresholdCnt++;
			continue;
		}
	}

	if (!fgAllThreshodZero) {
		if (!fgOneThresholdOK) {
			uintptr_t ptrFifoSa = 0, ptrFifoEa = 0;
			u32 u4ThresholdOKCnt = 0;

			for (u4Idx = 0; u4Idx < MAX_GAU_INSTANCE_CNT; ++u4Idx) {
				if (!(g_rDmxGauManager.arGAUInstance[u4Idx].fgUsed))
					continue;
				if (g_rDmxGauManager.arGAUInstance[u4Idx].pvDmxInst != pvDmxInst)
					continue;
				if (0 == g_rDmxGauManager.arGAUInstance[u4Idx].u4Threshold)
					continue;

				if (g_rDmxGauManager.arGAUInstance[u4Idx].fgEOS) {
					fgOneThresholdOK = TRUE;
					DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
						TEXT(
							"[GAU] %s line %d Encounter EOS  (u4Idx: 0x%x, u4Status: 0x%x, u4StmType: %d)\r\n"
						),
						DMX_FUNC_NAME, DMX_LINE_NO,
						u4Idx, g_rDmxGauManager.arGAUInstance[u4Idx].u4Status,
						g_rDmxGauManager.arGAUInstance[u4Idx].u4StmType);
					mrRet = RET_DMX_REACH_EOS;
					break;
				} 

				mrSubRet = ESM_FifoGetSA(g_rDmxGauManager.arGAUInstance[u4Idx].u4ESHandle, &ptrFifoSa);
				if (DMX_FAILED(mrSubRet)) {
					DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
						TEXT("[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_FifoGetEA, mrSubRet: 0x%x\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, u4Idx, mrSubRet);
					continue;
				}

				mrSubRet = ESM_FifoGetEA(g_rDmxGauManager.arGAUInstance[u4Idx].u4ESHandle, &ptrFifoEa);
				if (DMX_FAILED(mrSubRet)) {
					DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
						TEXT(
							"[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_FifoGetEA, mrSubRet: 0x%x\r\n"
						),
						DMX_FUNC_NAME, DMX_LINE_NO, u4Idx, mrSubRet);
					continue;
				}

				u4FifoSz = (u32)(ptrFifoEa - ptrFifoSa);

				mrSubRet = ESM_FifoGetAvailDataSize(g_rDmxGauManager.arGAUInstance[u4Idx].u4ESHandle,
					&u4AvailSz);
				if (DMX_FAILED(mrSubRet)) {
					DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
						TEXT(
							"[GAU] %s line %d (u4Handle: 0x%x) fail in ESM_FifoGetEA, mrSubRet: 0x%x\r\n"
						),
						DMX_FUNC_NAME, DMX_LINE_NO, u4Idx, mrSubRet);
					continue;
				}

				if ((g_rDmxGauManager.arGAUInstance[u4Idx].u4Threshold > 0) &&
					(u4AvailSz >= g_rDmxGauManager.arGAUInstance[u4Idx].u4Threshold)) {
					u4ThresholdOKCnt++;
					DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_THRESHOLD,
						TEXT(
							"[GAU] %s line %d Reach threshold (u4Idx: 0x%x, u4Threshold: 0x%x)\r\n"
						),
						DMX_FUNC_NAME, DMX_LINE_NO,
						u4Idx, g_rDmxGauManager.arGAUInstance[u4Idx].u4Threshold);
					fgOneThresholdOK = TRUE;
					break;
				} else if ((0 < u4FifoSz) &&
						((u4AvailSz * 100 / u4FifoSz >= 90) ||
						 (u4FifoSz - u4AvailSz <= PSR_RESERVE_FIFO_SPACE))) {
					u4ThresholdOKCnt++;
					DmxLogT(DMX_MOD_OTH, DMX_MOD_GAU_LOGLVL_THRESHOLD,
						TEXT(
							"[GAU] %s line %d Reach threshold(fifo full) (u4Idx: 0x%x, Threshold: 0x%x)\r\n"
						),
						DMX_FUNC_NAME, DMX_LINE_NO,
						u4Idx, g_rDmxGauManager.arGAUInstance[u4Idx].u4Threshold);
					fgOneThresholdOK = TRUE;
					break;
				} else {
					/*do nothing*/
				}
			}
		}
	}

	if ((!fgAllThreshodZero) &&
		fgOneThresholdOK) {
		for (u4Idx = 0; u4Idx < MAX_GAU_INSTANCE_CNT; ++u4Idx) {
			if (!(g_rDmxGauManager.arGAUInstance[u4Idx].fgUsed))
				continue;
			if (g_rDmxGauManager.arGAUInstance[u4Idx].pvDmxInst != pvDmxInst)
				continue;
			g_rDmxGauManager.arGAUInstance[u4Idx].u4Threshold = 0;
		}
		fgAllThreshodZero = TRUE;
	}

	if (fgAllThreshodZero ||
		fgOneThresholdOK) {
		g_rDmxGauManager.fgReachThreshold = TRUE;
		mrRet = RET_DMX_OK;
	}

	MM_RETURN(mrRet);
}

