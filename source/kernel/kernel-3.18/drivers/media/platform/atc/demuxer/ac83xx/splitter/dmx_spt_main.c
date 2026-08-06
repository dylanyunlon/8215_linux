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
 */

/*!
 * @file dmx_spt_main.c
 *
 *
 * @par Project
 *
 * @par Description
 *
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */

#include "x_os.h"
#include "x_debug.h"

#ifdef __linux__
#include <linux/mm.h>
#include <linux/errno.h>
#include "windows.h"
#include "winutil.h"
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/dmx_event.h>
#include <media/atc/dmx_cfa_def.h>
#include <media/atc/drv_esm_if.h>
#include <media/atc/ioctl_dmx.h>
/* #include <media/atc/mm_debug.h> */
#else /* __linux__*/
#include "drv_esm_if.h"
#include "dmx_define.h"
#include "dmx_splitter.h"
#include "dmx_event.h"
#include "dmx_cfa_def.h"
#include "ioctl_dmx.h"
#include "mm_debug.h"
#endif /* __linux__*/

#include "dmx_def.h"
#include "dmx_spt_os.h"
#include "dmx_spt.h"
#include "dmx_spt_if.h"
#include "dmx_pbbuf.h"
#include "dmx_spt_main.h"
#include "dmx_spt_psr.h"
#include "dmx_spt_rsp.h"
#include "dmx_stream.h"
#include "dmx_parser.h"
#include "dmx_psr_cc.h"
#include "dmx_mem.h"
#include "dmx_gau_if.h"
#include "dmx_cpsa.h"
#include "dmx_esm.h"
#include "dmx_dump.h"
#include "dmx_inst.h"

#ifndef __linux__
#pragma warning(disable : 4127) /*disable warning C4127: conditional expression is constant*/
#endif

EXTERN DMX_ESM_INST_T g_arESMInst[];
EXTERN DMX_SPT_MAN_INFO_T g_rSptMan;

HANDLE g_hDmxManLock = NULL;

MRESULT SptUninitInsts(void)
{
	DMX_SPT_INST_T *prSpt = NULL;
	MRESULT mrRet = RET_DMX_OK;
	u32 i;

	for (i = 0; i < DMX_MAX_SPT_INST_CNT; i++) {
		prSpt = g_rSptMan.aprSptInst[i];
		if (NULL == prSpt)
			continue;

		mrRet = SplitterDeleteTask(prSpt);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, 
				TEXT("[SPT] %s -- fail in delete prSpt(0x%p) task, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, prSpt, mrRet);
			break;
		}

		DMXLOG_DEBUG(TEXT("[SPT] %s -- delete prSpt(0x%p) task success\r\n"),
			DMX_FUNC_NAME, prSpt);

		mrRet = SplitterDeleteEvent(prSpt);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, 
				TEXT("[SPT] %s -- fail in delete prSpt(0x%p) event, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, prSpt, mrRet);
			break;
		}

		mrRet = SplitterDeleteSema(prSpt);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, 
				TEXT("[SPT] %s -- fail in delete prSpt(0x%p) semaphore, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, prSpt, mrRet);
			break;
		}

		mrRet = SplitterDeleteCmdQ(prSpt);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, 
				TEXT("[SPT] %s -- fail in delete prSpt(0x%p) CmdQ, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, prSpt, mrRet);
			break;
		}
	}

	for (i = 0; i < DMX_MAX_SPT_INST_CNT; i++) {
		prSpt = g_rSptMan.aprSptInst[i];
		if (NULL == prSpt)
			continue;

		dmx_memset(prSpt, 0, sizeof(DMX_SPT_INST_T));

		/* Initialize Splitter Instance*/
		prSpt->u4SptCompId = i;
	}

	DMX_FreeMemory(g_rSptMan.aprSptInst[0]);

	for (i = 0; i < DMX_MAX_SPT_INST_CNT; i++)
		g_rSptMan.aprSptInst[i] = NULL;

	MM_RETURN(mrRet);

}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SptInit*/
/* Initialize Splitter, do the following:*/
/* 1. Allocate Splitter instances*/
/* 2. initialize the instances, including create sema, event group, message queue, thread*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SptInit(void)
{
	DMX_SPT_INST_T *prSpt = NULL;
	MRESULT mrRet  = RET_DMX_OK;
	u32	i;

	DMXLOG_DEBUG(TEXT("[SPT] %s -- fgSptInitial: %d \r\n"),
		DMX_FUNC_NAME, g_rSptMan.fgSptInitial);

	if (g_rSptMan.fgSptInitial) {
		DMXLOG_TRACE(
			TEXT("[SPT] %s line %d exit, Dmx Spt has been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OK);
	}

	smp_mb();

	DMXLOG_DEBUG(
		TEXT("[SPT] %s line %d -- alloc Splitter Instance Array\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	if (NULL == g_rSptMan.aprSptInst[0]) {
		DMX_NewMemory((sizeof(DMX_SPT_INST_T) * DMX_MAX_SPT_INST_CNT),
			g_rSptMan.aprSptInst[0]);
		smp_mb();
		if (NULL == g_rSptMan.aprSptInst[0]) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, 
				TEXT("[SPT] %s fail in alloc splitter instance memory !!\r\n"),
				DMX_FUNC_NAME);
			DMX_ASSERT(FALSE);
			mrRet = RET_DMX_NO_MEM;
			smp_mb();
			goto ERRSPTINIT;
		}
	} else {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, 
			TEXT("[SPT] %s fail for SptInstsLists has been created!!\r\n"),
			DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		mrRet = RET_DMX_ALREADY_EXIST;
		smp_mb();
		goto ERRSPTINIT;
	}
	
	DMXLOG_DEBUG(TEXT("[SPT] %s -- initialize Splitter Instance\r\n"),
		DMX_FUNC_NAME, mrRet);

	smp_mb();
	/* Wait Turn On*/
	for (i = 0; i < DMX_MAX_SPT_INST_CNT; i++) {
		DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- i=%d\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, i);
		g_rSptMan.aprSptInst[i] = g_rSptMan.aprSptInst[0] + i;
		smp_mb();

		prSpt = g_rSptMan.aprSptInst[i];

		if (NULL == prSpt)
			continue;

		smp_mb();

		dmx_memset(prSpt, 0, sizeof(DMX_SPT_INST_T));
		smp_mb();
		/* Initialize Splitter Instance*/
		prSpt->u4SptCompId = i;

		prSpt->fgDivxDRMOn		= FALSE;
		prSpt->u8DivxDRMOffset	= DMX_INVALID_UINT64;
		prSpt->u4DecLen			= 0;
		prSpt->u2FrameKeyIndex	= DMX_DIVXDRM_INVALID_FRAMEIDX;
		prSpt->i4DecryptId		= DECRYPT_PLAY_INVALID_ID;

		prSpt->u8OtherAudioPts	= INVALID_TIMESTAMP;
		prSpt->u8PureAudPts		= INVALID_TIMESTAMP;

		prSpt->u4CfaType		   = DMX_INVALID_UINT32;
		prSpt->u8RspStartPts	= INVALID_TIMESTAMP;
		prSpt->u8RspPtsDelta	= INVALID_TIMESTAMP;

		prSpt->i4DecryptId		= DECRYPT_PLAY_INVALID_ID;

		prSpt->fgExitThread		= TRUE;

		prSpt->u4CmdRdIdx		= MAX_SPT_CMD_CNT;
		prSpt->u4CmdWrIdx		= MAX_SPT_CMD_CNT;

		prSpt->fgAUCtrl = TRUE;   /*init to TRUE*/
		
	}

	smp_mb();
	g_rSptMan.fgSptInitial = TRUE;
	smp_mb();

	DMXLOG_TRACE(TEXT("[SPT] %s success\r\n"),
		DMX_FUNC_NAME, mrRet);

	MM_RETURN(mrRet);

ERRSPTINIT:

	SptUninitInsts();

	DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("[SPT] %s failed, err:0x%x\r\n"),
		DMX_FUNC_NAME, mrRet);

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SptUninit*/
/* DeInitialize Splitter, do the following:*/
/* 1. Deinitialize the instances, including delete sema, event group, message queue, thread*/
/* 2. free Splitter instances*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SptUninit(void)
{
	MRESULT mrRet = RET_DMX_OK;


	DMXLOG_DEBUG(TEXT("[SPT] %s --- fgSptInitial:0x%x \r\n"),
		DMX_FUNC_NAME, g_rSptMan.fgSptInitial);

	if (!g_rSptMan.fgSptInitial)
		MM_RETURN(RET_DMX_NO_INIT);

	mrRet = SptUninitInsts();

	g_rSptMan.fgSptInitial = FALSE;


	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SptCreateInst*/
/* Get one Unused Spt Instance, Create its Parser CC, and Connect PBBUF*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SptCreateInst(void *pvDmxInst, void **ppvHandle)
{
	u32	 i;
	void *pvSpt	 = NULL;
	DMX_SPT_INST_T *prSpt = NULL;
	DMX_INST_T 	*prDmxInst = (DMX_INST_T *)pvDmxInst;
	MRESULT  mrRet = RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[SPT] %s enter.\r\n"), DMX_FUNC_NAME);

	if (NULL == ppvHandle) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[SPT] %s fail for invalid params.\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*ppvHandle = NULL;

	/* Get Splitter handle.*/
	for (i = 0; i < DMX_MAX_SPT_INST_CNT; i++) {
		if (!g_rSptMan.aprSptInst[i]->fgCreated) {
			g_rSptMan.aprSptInst[i]->fgCreated = TRUE;
			prSpt = g_rSptMan.aprSptInst[i];
			pvSpt  = (void *)prSpt;
			break;
		}
	}
	
	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, 
			TEXT("[SPT] %s fail for No more splitter instance to be created.\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}
	
	mrRet = SplitterCreateSema(prSpt);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, 
			TEXT("[SPT] %s fail in SplitterCreateSema(pvSpt:0x%p, mrRet:0x%x)\r\n"),
			DMX_FUNC_NAME, pvSpt, mrRet);
		goto ERRSPTCREATEINST;
	}

	mrRet = SplitterCreateEvent(prSpt, i);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, 
			TEXT("[SPT] %s fail in SplitterCreateEvent(pvSpt:0x%p, mrRet:0x%x)\r\n"),
			DMX_FUNC_NAME, pvSpt, mrRet);
		goto ERRSPTCREATEINST;
	}

	mrRet = SplitterCreateCmdQ(prSpt, i);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, 
			TEXT("[SPT] %s fail in SplitterCreateCmdQ(pvSpt:0x%p, mrRet:0x%x)\r\n"),
			DMX_FUNC_NAME, pvSpt, mrRet);
		goto ERRSPTCREATEINST;
	}

	mrRet = SplitterCreateTask(prSpt, i);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, 
			TEXT("[SPT] %s fail in SplitterCreateTask(pvSpt:0x%p, mrRet:0x%x)\r\n"),
			DMX_FUNC_NAME, pvSpt, mrRet);
		goto ERRSPTCREATEINST;
	}

	/* Create Parser CC and set Spt's Parser CC handle.*/
	DMXLOG_DEBUG(TEXT("[SPT] %s -- Create Parser.\r\n"), DMX_FUNC_NAME);
	mrRet = SplitterCreatePsr(pvDmxInst, pvSpt);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, 
			TEXT("[SPT] %s fail in SplitterCreatePsr(pvSpt:0x%p, mrRet:0x%x)\r\n"),
			DMX_FUNC_NAME, pvSpt, mrRet);
		goto ERRSPTCREATEINST;
	}

	/* Connect PBBuf*/
	DMXLOG_DEBUG(TEXT("[SPT] %s -- Connect PBBUFF.\r\n"), DMX_FUNC_NAME);
	mrRet = PBBUF_Connect(prSpt, prSpt->pvPsrCC, &(prSpt->u4PBBCompId),
    &(prSpt->pvPBBuf));
	if (DMX_FAILED(mrRet) || (NULL == prSpt->pvPBBuf)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[SPT] %s fail in PBBUF_Connect, pvSpt: 0x%p\r\n"),
			DMX_FUNC_NAME, pvSpt);
		goto ERRSPTCREATEINST;
	}

	prSpt->fgDivxDRMOn		= FALSE;
	prSpt->u8DivxDRMOffset	= DMX_INVALID_UINT64;
	prSpt->u4DecLen			= 0;
	prSpt->u2FrameKeyIndex	= DMX_DIVXDRM_INVALID_FRAMEIDX;
	prSpt->i4DecryptId		= DECRYPT_PLAY_INVALID_ID;

	prSpt->fgRspEnable = FALSE;
	prSpt->u4RepeatErrChkCnt = 0;
	prSpt->fgCfaPrsEnd = FALSE;
  	prSpt->u4PsrEndStatus = 0;
	prSpt->i4Rate = MM_PLAY_RATE_NORMAL;

	prSpt->fgDmaAud = TRUE;

	#if DMX_DISABLE_AUD_DMA
	prSpt->fgDmaAud = FALSE;
	#endif /*DMX_DISABLE_AUD_DMA*/

	prSpt->u8RspStartPts = INVALID_TIMESTAMP;
	prSpt->u8RspPtsDelta = 0;
	prSpt->u8RspOffset = DMX_INVALID_UINT64;
	prSpt->u8RspOffsetDelta = 0;
	prSpt->fgReRsp = FALSE;
	prSpt->u8FileEndOffset = DMX_INVALID_UINT64;

	prSpt->eSptState = SPLITTER_STATE_IDLE;
	prSpt->eSptTxState = SPLITTER_TX_STATE_NONE;

	prSpt->pvDmxInst = pvDmxInst;

	SplitterSetRspStartPts(pvSpt, INVALID_TIMESTAMP);
	SplitterSetRspStartOffset(pvSpt, DMX_INVALID_UINT64);
	SplitterSetRspOffsetDelta(pvSpt, 0);
	SplitterSetReResplitter(pvSpt, FALSE);
	SplitterSetRspTxType(pvSpt, SPT_DATA_UNDEFINE);

	DMXLOG_DEBUG(TEXT("[SPT] %s success -- SptInstsCnt: %d\r\n"),
		DMX_FUNC_NAME, prDmxInst->u4SptCnt);

	*ppvHandle = pvSpt;

	MM_RETURN(mrRet);

ERRSPTCREATEINST:
	SplitterDestroyPsr(pvSpt);
	SplitterDeleteTask(prSpt);
	SplitterDeleteEvent(prSpt);
	SplitterDeleteSema(prSpt);
	SplitterDeleteCmdQ(prSpt);

	prSpt->fgCreated = FALSE;

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SptHandleFromCompID*/
/* Get the splitter handle by its index*/
/* @param u4SptCompId [in]	index in the splitter instances array*/
/*/////////////////////////////////////////////////////////////////////////////*/
void *SptHandleFromCompID(u32 u4SptCompId)
{
	u32 i;

	for (i = 0; i < DMX_MAX_SPT_INST_CNT; i++) {
		if ((NULL != g_rSptMan.aprSptInst[i]) &&
			(g_rSptMan.aprSptInst[i]->u4SptCompId == u4SptCompId))
			return ((void *)g_rSptMan.aprSptInst[i]);
	}

	return 0;
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterSetAudioMaxDuration*/
/* Set the audio max duration*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterSetAudioMaxDuration(void *pvSptHdl, u8 ucAudSec)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[SPT] %s fail for Invalid Args\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prSpt->ucAudMaxDuration = ucAudSec;
	DMXLOG_DEBUG(TEXT("[SPT] %s -- ucAudSec: 0x%x, prSpt: 0x%p \r\n"),
		DMX_FUNC_NAME, ucAudSec, prSpt);

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterSetEnable*/
/* Enable the designated splitter, do the following:*/
/* 1. Create Psr Off Event*/
/* 2. Set Splitter enable, stop flag, add spt enable cmd to the cmd history array*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterSetEnable(void *pvSptHdl)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[SPT] %s fail for Invalid Args\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (DMX_MAX_SPT_INST_CNT <= prSpt->u4SptCompId) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, 
			TEXT("[SPT] %s fail for invalid Splitter's SptCompId(%d)!\r\n"),
			DMX_FUNC_NAME, prSpt->u4SptCompId);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prSpt->fgEnable = TRUE;

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterSetDisable*/
/* Disable the designated splitter, do the following:*/
/* 1. Set Splitter enable flag to be false*/
/* 2  Add spt disable cmd to the cmd history array*/
/* 3. Close Psr Off Event*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterSetDisable(void *pvSptHdl)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[SPT] %s fail for Invalid Args\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DMXLOG_DEBUG(TEXT("[SPT] %s -- prSpt:0x%p."), DMX_FUNC_NAME, prSpt);

	prSpt->fgEnable = FALSE;

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterGet2pipeVSptInst*/
/* Get the handle of the splitter instance which contain the video stream*/
/*/////////////////////////////////////////////////////////////////////////////*/
DMX_SPT_INST_T *SplitterGet2pipeVSptInst(void)
{
	DMX_SPT_INST_T *prSpt = NULL;
	u32	i, j;

	/*/1. search V-pipe*/
	for (i = 0; i < DMX_MAX_SPT_INST_CNT; i++) {
		prSpt = g_rSptMan.aprSptInst[i];
		if ((NULL == prSpt) || (!prSpt->fgEnable))
			continue;

		for (j = 0; j < prSpt->u4StmHandleNs; j++) {
			if (prSpt->pvStmHandles[j] == GetStreamByType(prSpt, SPT_DATA_V))
				return prSpt;
		}
	}
	
	return NULL;
}

MRESULT Splitter4PsrTxDoneEvent(DMX_SPT_INST_T *prSpt)
{
	void *pvStmHandle = GetStreamByType(prSpt, prSpt->u4PtxToStreamType);
	MRESULT mrRet  = RET_DMX_OK;

	if ((NULL != pvStmHandle) && (!StreamIsEnabled(pvStmHandle) || (SplitterRspIsEnabled(prSpt) &&
                 SplitterIsRspOffStart(prSpt)))) {
		if ((SPT_DATA_A == prSpt->u4PtxToStreamType) ||
			(SPT_DATA_SP == prSpt->u4PtxToStreamType)) {
			PSR_AU	   *prPsrAu = (PSR_AU *)prSpt->pvTempPsrAu;
			AU_AUDIO   *prTmpAu = (AU_AUDIO *)prSpt->pvTempAu;

			if (NULL != prTmpAu) {
				dmx_memset(prTmpAu, 0, sizeof(AU_AUDIO));
				prTmpAu->eAuType = AU_DATA;
			}

			if (NULL != prPsrAu) {
				dmx_memset(prPsrAu, 0, sizeof(PSR_AU));
				prPsrAu->pvAUExtInf = NULL;
				prPsrAu->pvAUInf = prTmpAu;
				prPsrAu->eType = prSpt->u4PtxToStreamType;
				mrRet = SplitterSetPsrAuTable(prSpt, prPsrAu);
				if (DMX_FAILED(mrRet))
					MM_RETURN(mrRet);
			}
		}
	}
	DMXLOG_DEBUG(
		TEXT("[SPT] ----- %s line %d -- set SPLITTER_EV_PTX_DONE\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
	mrRet = SplitterSendNfy(prSpt, DMX_SPT_NTY_TX_END);

	MM_RETURN(mrRet);
}


/*/////////////////////////////////////////////////////////////////////////////*/
/* Splitter4PsrEvent*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT Splitter4PsrEvent(PSR_CB_EVENT eEvent, void *pvEventData, void *pvSptHdl)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	MRESULT mrRet  = RET_DMX_OK;

	if (NULL == pvSptHdl) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, 
			TEXT("[SPT] %s fail for invalid args (pvSptHdl: 0x%p, eEvent: 0x%x)\r\n"),
			DMX_FUNC_NAME, pvSptHdl, eEvent);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	switch (eEvent) {
	case E_WAKEUP_ME:
		mrRet = SplitterSendNfy(prSpt, DMX_SPT_NTY_TX_CONTINUE);
		break;

	case E_TX_DONE:
		mrRet = Splitter4PsrTxDoneEvent(prSpt);
		break;

	case E_PAUSE_DONE:
		DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- E_PAUSE_DONE, Splitter")
			TEXT("(0x%p)'s state: %d, eTxState: %d\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt, prSpt->eSptState, prSpt->eSptTxState);
		 mrRet = SplitterSendNfy(prSpt, DMX_SPT_NTY_TX_PAUSE);
		break;

	case E_ABORT_DONE:
		SplitterSetPtxNotBusy(prSpt);

		mrRet = PSR_CC_Enable(prSpt->pvPsrCC, FALSE);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, 
				TEXT("[SPT] %s line %d fail in PSR_CC_Enable(prSpt(0x%p), FALSE)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
			MM_RETURN(mrRet);
		}

		DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- Splitter(0x%p)'s state: %d\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt, SplitterGetState(prSpt));

		mrRet = SplitterChangeState(prSpt, SPLITTER_STATE_IDLE, SPLITTER_TX_STATE_NONE);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[SPT] %s line %d fail in SplitterChange")
				TEXT("State(prSpt(0x%p), IDLE, TX_NONE)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
			MM_RETURN(mrRet);
		}

		mrRet = SplitterSendNfy(prSpt, DMX_SPT_NTY_TX_ABORT);
		break;

	case E_GET_AU_INFO:
		{
			mrRet = SplitterSetPsrAuTable(prSpt, pvEventData);
			{
				PSR_AU	*prAuData = (PSR_AU *)pvEventData;

				if ((NULL != prAuData) && (SPT_DATA_A == prAuData->eType)) {
					AU_AUDIO *prAudAU = (AU_AUDIO *)(prAuData->pvAUInf);

					DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
						TEXT("[SPT] %s line %d -- AudAU PTS: " DMX_PTS_LOGSTR "\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						DMX_PTS_LOG_MS(prAudAU->rAUInfo.rInfo.u8Pts),
						DMX_PTS_LOG_PTS(prAudAU->rAUInfo.rInfo.u8Pts));
				}
			}
		}
		break;

	case E_TX_JUMP:
		smp_mb();
		mrRet = SplitterSendNfy(prSpt, DMX_SPT_NTY_TX_JUMP);
		break;

	default:
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, 
			TEXT("[SPT] %s line %d -- get err event.0x%x, prSpt: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eEvent, prSpt);
		MM_RETURN(RET_DMX_NO_IMPLEMENT);
	}

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterCreatePsr*/
/* Create Parser CC for the designated splitter, and set the pbbuf handle to the Parser CC*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterCreatePsr(void *pvDmxInst, void *pvSptHdl)
{
	void *pvPsrCC = NULL;
	MRESULT mrRet  = RET_DMX_OK;

	if (NULL == pvSptHdl) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[SPT] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mrRet = PSR_CC_Create(pvDmxInst, pvSptHdl, ((DMX_SPT_INST_T *)pvSptHdl)->pvPBBuf, &pvPsrCC);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, 
			TEXT("[SPT] %s fail for in PSR_CC_Create (pvSptHdl: 0x%p, mrRet: 0x%x)\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	SplitterSetPtxHandle(pvSptHdl, pvPsrCC);

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterDestroyPsr*/
/* Destroy Parser CC for the designated splitter*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterDestroyPsr(void *pvSptHdl)
{
	MRESULT mrRet = RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[SPT] %s -- pvSptHdl:0x%p."), DMX_FUNC_NAME, pvSptHdl);

	if (NULL == pvSptHdl) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[SPT] %s fail for invalid args (pvSptHdl: 0x%p)\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (NULL != ((DMX_SPT_INST_T *)pvSptHdl)->pvPsrCC) {
		mrRet = PSR_CC_Destroy(((DMX_SPT_INST_T *)pvSptHdl)->pvPsrCC);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, 
				TEXT("[SPT] %s fail for in PSR_CC_Destroy (pvSptHdl: 0x%p)\r\n"),
				DMX_FUNC_NAME, pvSptHdl);
			MM_RETURN(mrRet);
		}
	}

	SplitterSetPtxHandle(pvSptHdl, NULL);

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterSetPsrLastMem*/
/* Set Splitter's Parser CC's last mem flag*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterSetPsrLastMem(void *pvSptHdl, bool fgLastMem)
{
	MRESULT mrRet = PSR_CC_SetLastMemState(((DMX_SPT_INST_T *)pvSptHdl)->pvPsrCC, fgLastMem);

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterSetPtxData*/
/* Set Tx Data info into the splitter*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterSetPtxData(
	void *pvSptHdl,
	DMX_SPT_DMA2FIFO_INFO_T *prInf,
	void *pvStm,
	bool fgNeedRspLog)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, TEXT("[SPT] %s fail for invalid args (prSpt: 0x%p)\r\n"),
			DMX_FUNC_NAME, prSpt);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!prSpt->fgPtxBusy) {
		prSpt->fgPtxBusy			= TRUE;
		prSpt->u8PtxFromFileOffset	= prInf->u8FromFileOfst;
		prSpt->pvPtxFromDramAddress = prInf->pvFromAddress;
		prSpt->u8PtxLen				= prInf->u8TxLen;
		prSpt->u4PtxToStreamType	= prInf->u4TxStreamType;
		prSpt->pvPtxToDramAddress	= prInf->pvToAddress;
		prSpt->u4PtxVideoCodec		= prInf->u4TxVideoCodec;
		prSpt->u4PtxPictureMode		= prInf->u4TxPictureMode;
		prSpt->u8PtxPtsEa			= prInf->u8PtsEa;

		/* only Rsplog can enter*/
		if (SplitterRspIsEnabled(prSpt) && (SplitterRspIsLoging(prSpt)) &&
			fgNeedRspLog && (NULL != pvStm))
			SplitterRspSetLogTx(prSpt, prInf);

		MM_RETURN(RET_DMX_OK);
	}

	DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, 
		TEXT("[SPT] %s fail for Splitter has already started PTX data(prSpt: 0x%p)\r\n"),
		DMX_FUNC_NAME, prSpt);

	MM_RETURN(RET_DMX_PTX_BUSY);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterGetStmHandles*/
/* Get the splitter's stream handles array's pointer*/
/*/////////////////////////////////////////////////////////////////////////////*/
void **SplitterGetStmHandles(void *pvSptHdl, u32 *pu4StmHandleNs)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	if ((NULL == prSpt) ||
		(NULL == pu4StmHandleNs)) {
		DMX_ASSERT(FALSE);
		return NULL;
	}

	if (NULL != prSpt) {
		*pu4StmHandleNs = prSpt->u4StmHandleNs;
		return prSpt->pvStmHandles;
	}

	DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, TEXT("[SPT] %s fail for invalid args, prSpt: 0x%p\r\n"),
		DMX_FUNC_NAME, prSpt);
	*pu4StmHandleNs = 0;
	return NULL;
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterGetStmHandles*/
/* Add the stream to the splitter's stream handles array*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterAddStmHandle(void *pvSptHdl, void *pvStmHandle)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	u32 i;

	if ((NULL == prSpt) || (NULL == pvStmHandle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, 
			TEXT("[SPT] %s failed for invalid args(prSpt: 0x%p, pvStmHandle: 0x%p)!\r\n"),
			DMX_FUNC_NAME, prSpt, pvStmHandle);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	for (i = 0; i < MAX_SPT_STM_CONNECTED; i++) {
		if (prSpt->pvStmHandles[i] == pvStmHandle)
			MM_RETURN(RET_DMX_ALREADY_EXIST);
	}

	if (prSpt->u4StmHandleNs < MAX_SPT_STM_CONNECTED) {
		prSpt->pvStmHandles[prSpt->u4StmHandleNs++] = pvStmHandle;
		((DMX_STM_INST_T *)pvStmHandle)->pvSptHdl = pvSptHdl;

		MM_RETURN(RET_DMX_OK);
	}

	DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, TEXT("[SPT] %s failed for Splitter(pvSptHdl: 0x%p)'s Stream")
		TEXT(" Count(%d) already excceed max value!\r\n"),
		DMX_FUNC_NAME, pvSptHdl, prSpt->u4StmHandleNs);

	MM_RETURN(RET_DMX_OVER_LIMIT);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterDelStmHandle*/
/* Remove the stream from the splitter's stream handles array*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterDelStmHandle(void *pvSptHdl, void *pvStmHandle)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	u32	i;

	if (!((NULL != prSpt) && (NULL != pvStmHandle))) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (0 == prSpt->u4StmHandleNs) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, TEXT("[SPT] %s line %d failed for Splitter(prSpt:")
			TEXT(" 0x%p)'s Stream Count already is 0!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt);
		MM_RETURN(RET_DMX_OPERATE_FORBID);
	}

	for (i = 0; i < MAX_SPT_STM_CONNECTED; i++) {
		if (prSpt->pvStmHandles[i] == pvStmHandle) {
			prSpt->pvStmHandles[i] = NULL;
			prSpt->u4StmHandleNs--;
			MM_RETURN(RET_DMX_OK);
		}
	}

	DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, TEXT("[SPT] %s line %d failed for can't find the steam")
		TEXT("(pvStmHandle: 0x%p) in (pvSptHdl: 0x%p)!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, pvStmHandle, pvSptHdl);

	MM_RETURN(RET_DMX_NOT_FOUND);
}

/*//////////////////////////////////////////////////////////////////////////////*/
/* SplitterIstPureAudioPIPE*/
/* Check whether the Splitter is only used for tx audio or not*/
/*//////////////////////////////////////////////////////////////////////////////*/
bool SplitterIstPureAudioPIPE(void *pvSptHdl)
{
	if ((NULL != GetStreamByType(pvSptHdl, SPT_DATA_A)) &&
		(NULL == GetStreamByType(pvSptHdl, SPT_DATA_V))) {
		return TRUE;
	}

	return FALSE;
}

/*//////////////////////////////////////////////////////////////////////////////*/
/* SplitterSetPBBOffsetSa*/
/* PBBuf Start Offse, We use it to save current PBBuf offset, Rsp need this,*/
/* but after parser off, we can not set it*/
/*//////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterSetPBBOffsetSa(void *pvSptHdl, u64 u8PBBOffsetSa)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, TEXT("[SPT] %s failed for invalid args!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prSpt->u8PBBOffsetSa = u8PBBOffsetSa;

	MM_RETURN(RET_DMX_OK);
}

MRESULT SplitterSetFileEndOffset(void *pvSptHdl, u64 u8FileEndOffset)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, TEXT("[SPT] %s failed for invalid args!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prSpt->u8FileEndOffset = u8FileEndOffset;

	if ((NULL != prSpt) && (NULL != prSpt->pvPsrCC)) {
		PSR_CC *prPsrCC = (PSR_CC *)(prSpt->pvPsrCC);
		u32 u4Idx = 0;

		prPsrCC->u8FileEndOffset = u8FileEndOffset;
		for (u4Idx = 0; u4Idx < MAX_PSR_FILTER_PER_CC; u4Idx++) {
			PSR_FILTER *prPsrFtr = (PSR_FILTER *)(prPsrCC->apvFtr[u4Idx]);

			if (NULL != prPsrFtr) {
				prPsrFtr->u8FileEndOffset = u8FileEndOffset;
			}
		}
		if ((CCS_TX == prPsrCC->eState) &&
			(TXS_WAIT_PBBUF == prPsrCC->eTxState))
			PSR_CC_CBSplitter(prPsrCC, E_WAKEUP_ME, NULL);
	}

	MM_RETURN(RET_DMX_OK);
}

void SplitterSetEOSForHwError(void *pvSptHdl)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	MRESULT mrRet = RET_DMX_OK;

	/* release HW access right*/
	if ((NULL != prSpt) && (NULL != prSpt->pvPsrCC)) {
		PSR_CC *prPsrCC = (PSR_CC *)(prSpt->pvPsrCC);
		u32 u4Idx = 0;

		for (u4Idx = 0; u4Idx < MAX_PSR_FILTER_PER_CC; u4Idx++) {
			PSR_FILTER *prPsrFtr = (PSR_FILTER *)(prPsrCC->apvFtr[u4Idx]);

			if (NULL != prPsrFtr) {
				if (DMX_INVALID_UINT8 != prPsrFtr->ucHwDevId) {
					PSR_HWRes_Release(prPsrFtr);
					break;
				}
			}
		}
		mrRet = SplitterPsrSetEOS(pvSptHdl, GAU_E_FAIL);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, TEXT("[SPT] %s line %d failed in ")
				TEXT("SplitterPsrSetEOS(pvSptHdl: 0x%p, GAU_E_FAIL)!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		}
		PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);
		PSR_CC_SetState(prPsrCC, CCS_INIT);
	}
}

void SplitterSetEOSForDefaultError(void *pvSptHdl)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	MRESULT mrRet = RET_DMX_OK;

	/* release HW access right*/
	if ((NULL != prSpt) && (NULL != prSpt->pvPsrCC)) {
		PSR_CC *prPsrCC = (PSR_CC *)(prSpt->pvPsrCC);
		u32 u4Idx = 0;

		for (u4Idx = 0; u4Idx < MAX_PSR_FILTER_PER_CC; u4Idx++) {
			PSR_FILTER *prPsrFtr = (PSR_FILTER *)(prPsrCC->apvFtr[u4Idx]);

			if (NULL != prPsrFtr) {
				if (DMX_INVALID_UINT8 != prPsrFtr->ucHwDevId) {
					PSR_HWRes_Release(prPsrFtr);
					break;
				}
			}
		}
	}

	mrRet = SplitterPsrSetEOS(pvSptHdl, GAU_E_FAIL);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, TEXT("[SPT] %s line %d failed in Splitter")
			TEXT("PsrSetEOS(pvSptHdl: 0x%p, GAU_E_FAIL)!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
	}
}

void SplitterSetEOSForError(void *pvSptHdl, MRESULT mrRet)
{
	switch (mrRet) {
	case RET_DMX_OK:
		break;
	case RET_DMX_ERR_DATA:
		{
			/*DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;*/
			if (NULL != pvSptHdl)
				GAU_DisableThreshold();
#if 1
			/* if the user want to stop the playing process while encounter file's data error*/
			/* we should send GAU_E_ERRDATA*/
			mrRet = SplitterPsrSetEOS(pvSptHdl, GAU_E_ERRDATA);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, TEXT("[SPT] %s line %d failed in Splitter")
					TEXT("PsrSetEOS(pvSptHdl: 0x%p, Status: 0x%x)!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, GAU_E_ERRDATA);
			}
#else
			/* if the user want to skip 5s while encounter file's data error*/
			/* we should send GAU_E_ERRCHUNK*/
			mrRet = SplitterPsrSetEOS(pvSptHdl, GAU_E_ERRCHUNK);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, (TEXT("[SPT] %s line %d failed in ")
					TEXT("SplitterPsrSetEOS(pvSptHdl: 0x%x, Status: 0x%x)!\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, GAU_E_ERRCHUNK);
			}
#endif
		}
		break;
	case RET_DMX_NEED_JUMP:
		{
			DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;

			if ((NULL != prSpt) && ((CFA_TYPE_AUDIN == SplitterGetCfaType(pvSptHdl)) ||
				MM_IS_FFRW_PLAY(prSpt->i4Rate))) {
				PSR_CC *prPsrCC = (PSR_CC *)prSpt->pvPsrCC;

				mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_JUMP);
				if (DMX_FAILED(mrRet)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, TEXT("[SPT] %s fail in SplitterChange")
						TEXT("State(RUNING, JUMP), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
						DMX_FUNC_NAME, pvSptHdl, mrRet);
					SplitterSetEOSForError(pvSptHdl, mrRet);
					return;
				}
				PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
				PSR_CC_CBSplitter(prPsrCC, E_TX_JUMP, NULL);
			} else {
				mrRet = SplitterPsrSetEOS(pvSptHdl, GAU_E_FAIL);
				if (DMX_FAILED(mrRet)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, TEXT("[SPT] %s line %d failed in ")
						TEXT("SplitterPsrSetEOS(pvSptHdl: 0x%p, GAU_E_FAIL)!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
				}
			}
		}
		break;
	case RET_DMX_FIFO_FULL:
	case RET_DMX_PBBUF_BUSY:
	case RET_DMX_UNSUPPORT:
	case RET_DMX_NO_REACH_THRESHOLD:
	case RET_DMX_REACH_EOS:
	case RET_DMX_NO_AU:
		break;
	case RET_DMX_HW_ERROR:
		/*DmxDumpClocksRegisters();*/
		SplitterSetEOSForHwError(pvSptHdl);
		break;
	default:
		SplitterSetEOSForDefaultError(pvSptHdl);
		break;
	}
}

/*/////////////////////////////////////////////////////////////////////////////////*/
/* SplitterGetPureAudioSTC*/
/* If the Splitter is only used for tx audio, Get the PTS of current Rd idx's Audio AU*/
/* otherwise, return INVLAID_TIMESTAMP*/
/*/////////////////////////////////////////////////////////////////////////////////*/
u64 SplitterGetPureAudioSTC(void *pvSptHdl)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	u64	 u8STC = (u64)INVALID_TIMESTAMP;

	if (NULL == prSpt)
		return u8STC;

	if (SplitterIstPureAudioPIPE(prSpt)) {
		/*for pure audio case, not connect sync-ctrl, need get STC from audio driver*/
		/*Since audio driver do not have interface, we just get it from the current audio AU*/
		AU_AUDIO *auAudio = NULL;
		u32	  u4AuIdx = 0;
		u32	  u4RdIdx, u4WdIdx;

		/*search AUDIO ESITable item index*/
		for (u4AuIdx = 0; u4AuIdx < MAX_ESICOUNT; u4AuIdx++) {
			if (ES_A == g_arESMInst[u4AuIdx].eType)
				break;
		}

		if (u4AuIdx >= MAX_ESICOUNT)
			return (u64)INVALID_TIMESTAMP;

		if (NULL == g_arESMInst[u4AuIdx].prAUTable)
			return (u64)INVALID_TIMESTAMP;

		auAudio = (AU_AUDIO *)(g_arESMInst[u4AuIdx].prAUTable->ptrSa);

		if (NULL == auAudio)
			return (u64)INVALID_TIMESTAMP;

		u4RdIdx = g_arESMInst[u4AuIdx].prAUTable->u4RdIdx;
		u4WdIdx = g_arESMInst[u4AuIdx].prAUTable->u4WrIdx;

		if (u4RdIdx != u4WdIdx) {
			u8STC  = auAudio[u4RdIdx].rAUInfo.rInfo.u8Pts;
			prSpt->u8PureAudPts = u8STC;
		} else /*use the previous AU pts as the STC*/ {
			/*Maybe already flush fifo, use previous one as the STC*/
			if ((u64)INVALID_TIMESTAMP != prSpt->u8PureAudPts)
				u8STC = prSpt->u8PureAudPts;
		}
	}

	return u8STC;
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterSetCfaPsrEnd*/
/* Set Splitter's CfaParseEnd Flag, and information Parser CC Cfa whether parse end,*/
/*i.e. Set Parser CC CfaParseEnd Flag*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterSetCfaPsrEnd(void *pvSptHdl, bool fgCfaPrsEnd, u32 u4Status)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	MRESULT mrRet  = RET_DMX_OK;

	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, TEXT("[SPT] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prSpt->fgCfaPrsEnd = fgCfaPrsEnd;
	
	prSpt->u4PsrEndStatus = u4Status;

	mrRet = PSR_CC_NotiCfaPrsEnd(prSpt->pvPsrCC, fgCfaPrsEnd);

	MM_RETURN(mrRet);
}

E_DECRYPT_TYPE_T SplitterGetDecryptType(void *pvSptHdl)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, TEXT("[SPT] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		return DECRYPT_NONE;
	}

	if (NULL != prSpt->pvPsrCC)
		return (((PSR_CC *)(prSpt->pvPsrCC))->rDecryptMan.eDecryptType);

	return DECRYPT_NONE;
}

MRESULT SplitterSetDecryptType(void *pvSptHdl, E_DECRYPT_TYPE_T eDecryptType)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	MRESULT mrRet  = RET_DMX_OK;

	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, TEXT("[SPT] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mrRet = PSR_CC_SetDecryptType(prSpt->pvPsrCC, eDecryptType);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT, TEXT("[SPT] %s line %d failed in PSR_CC_SetDecrypt")
			TEXT("Type(eDecryptType: %d), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(mrRet);
	}

	MM_RETURN(mrRet);
}
