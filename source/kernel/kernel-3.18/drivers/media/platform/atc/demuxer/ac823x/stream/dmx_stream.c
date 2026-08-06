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
 * @file dmx_stream.c
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
#ifdef __linux__
#include <linux/mm.h>
#include <linux/errno.h>
#include "windows.h"
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_cfa_def.h>
#include <media/atc/ioctl_dmx.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "dmx_cfa_def.h"
#include "ioctl_dmx.h"
#include "mm_debug.h"
#endif /* __linux__*/

#include "dmx_stream.h"
#include "dmx_def.h"
#include "dmx_spt.h"
#include "dmx_spt_cfa.h"
#include "dmx_spt_main.h"
#include "dmx_spt_util.h"
#include "dmx_spt_os.h"
#include "dmx_psr_cc.h"
#include "dmx_psr_filter.h"
#include "dmx_psr_esm.h"
#include "dmx_mem.h"
#include "dmx_gau.h"
#include "dmx_gau_if.h"
#include "dmx_parser.h"
#include "dmx_cli.h"
#include "dmx_dump.h"

#ifndef __linux__
#pragma warning(disable : 4127) /*disable warning C4127: conditional expression is constant*/
#endif

EXTERN DMX_CLI_MAN_T g_rDmxCliMan;
EXTERN DMX_DUMP_MAN_T g_rDmxDumpMan;
EXTERN DMX_STM_MAN_INFO_T g_rDmxStmMan;
EXTERN AU_AUDIO g_arLogAudioAUs[DMX_MAX_LOG_AUDIO_AU_CNT];

MRESULT StreamInit(void)
{
	DMX_STM_INST_T *prStm = NULL;
	u32	i;

	if (g_rDmxStmMan.fgStmInitial) {
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM, TEXT("[STM] %s exit for Stream has been intialized!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	smp_mb();/* */

	mm_memset(g_rDmxStmMan.arStmInst, 0, sizeof(g_rDmxStmMan.arStmInst));
	smp_mb();/* */

	for (i = 0; i < (u32)MAX_STREAM_INSTANCE_CNT; i++) {
		prStm = &(g_rDmxStmMan.arStmInst[i]);
		smp_mb();/* */

		prStm->u4StmUID = DMX_INVALID_UINT32;
		prStm->u4CompId = i;
		prStm->u4FifoThreshold = 0;  /*default : 0*/
		if (i < (u32)DMX_MAX_ORG_STM_CNT)
			prStm->u4StmType = STREAM_NONE;
		else if (i < (u32)(DMX_MAX_ORG_STM_CNT + DMX_MAX_DMA_STM_CNT))
			prStm->u4StmType = STREAM_DMA;
		else /*Ground filter*/
			prStm->u4StmType = STREAM_INITIAL;
	}

	smp_mb();/* */

	g_rDmxStmMan.fgStmInitial = TRUE;
	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM, TEXT("[STM] %s success!\r\n"),
		DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

MRESULT StreamUninit(void)
{
	if (!g_rDmxStmMan.fgStmInitial)
		MM_RETURN(RET_DMX_OK);

	g_rDmxStmMan.fgStmInitial = FALSE;
	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM, TEXT("[STM] %s success!\r\n"),
		DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

/****************************************************/
/* StreamCreate*/
/* 1.  Get One unused Stream Instance, Add its handle to Splitter.*/
/* 2.  Create Parser Filter for this stream, and Add it to the splitter's Parser CC*/
/* 3.  Set the type of this Parser Filter, Alloc Psr Filter Specific Info according to its type*/
/* 4.  Create ESM,	Set ESM Memory, and Register ESM Demuxer CB  for this parser filter*/
/* 5.  Connect this stream to GAU(Create ESM), Register ESM Decoder CB*/
/* @Param u4StmType -- Stream Type, eg. SPT_DATA_V, SPT_DATA_A, SPT_DATA_SP, and so on*/
/* @Param u4StmUID	 -- Stream ID in this kind of stream, eg. For Audio, if we using the first Audio*/
/*stream, u4StmUID is 0*/
/****************************************************/
MRESULT StreamCreate(void *pvDmxInst, STM_PARAM_CREATE *prParam, void **ppvHandle)
{
	GAU_CONNECT_PARAM_T rGauParam;
	DMX_STM_INST_T *prStm = NULL;
	DMX_SPT_INST_T *prSpt = NULL;
	void	*pvStm = NULL;
	void	*pvPsrFtr = NULL;
	MRESULT mrRet = RET_DMX_OK;
	u32	i;

	if ((NULL == prParam) ||
		(NULL == ppvHandle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s failed for invalid args, (prParam: 0x%x, pptrHandle: 0x%x)!\r\n"),
			DMX_FUNC_NAME, prParam, ppvHandle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*ppvHandle = NULL;
	prSpt = (DMX_SPT_INST_T *)(prParam->pvSptHdl);
	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s failed for invalid args(prParam->pvSptHdl==NULL), ")
			TEXT("prParam: 0x%x, pHandle: 0x%x)!\r\n"),
			DMX_FUNC_NAME, prParam, ppvHandle);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (prParam->u4StmType < MAX_SPT_DATA_TYPE_CNT) {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s(Type=%d, %s) enter, StreamUID: %d!\r\n"),
			DMX_FUNC_NAME, prParam->u4StmType,
			g_aszSptDataTypeName[prParam->u4StmType],
			prParam->u4StmUID);
	} else {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s(Type=%d, %s) enter, StreamUID: %d!\r\n"),
			DMX_FUNC_NAME, prParam->u4StmType,
			TEXT("UNKNOWN"),
			prParam->u4StmUID);
	}

	STREAM_LOCK(prSpt);

	switch (prParam->u4StmType) {
	case STREAM_VIDEO:
	case STREAM_AUDIO:
	case STREAM_SUBTITLE:
	case STREAM_SECTION:
		break;

	default:
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s(Type=%d, %s) fail for invaldi streamtype!\r\n"),
			DMX_FUNC_NAME, prParam->u4StmType,
			DMX_SPTDATATYPE_STR(prParam->u4StmType));
		STREAM_UNLOCK(prSpt);
		MM_RETURN(RET_DMX_PARAM_WRONG);
        break;
	}

	pvStm = GetStreamByTypes(pvDmxInst, prParam->u4StmType);
	if (NULL != pvStm) {
		prStm = (DMX_STM_INST_T *)pvStm;
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s(Type=%d, %s) fail for already has this ")
			TEXT("streamtype's stream(owned by pvSptHdl(0x%p)) in DMX driver!\r\n"),
			DMX_FUNC_NAME, prParam->u4StmType,
			DMX_SPTDATATYPE_STR(prParam->u4StmType), prStm->pvSptHdl);
		STREAM_UNLOCK(prSpt);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}

	/*1. Get handle of stream.*/
	for (i = 0; i < MAX_STREAM_INSTANCE_CNT; i++) {
		prStm = &(g_rDmxStmMan.arStmInst[i]);

		if ((STREAM_NONE == prStm->u4StmType) &&
			(DMX_INVALID_UINT32 == prStm->u4StmUID)) {
			prStm->u4StmType = prParam->u4StmType;
			prStm->u4StmUID  = prParam->u4StmUID;
			prStm->pvDmxInst  = pvDmxInst;
			pvStm = (void *)prStm;
			break;
		}
	}

	if (NULL == pvStm) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s failed for no unused Stream Instance!\r\n"),
			DMX_FUNC_NAME);
		STREAM_UNLOCK(prSpt);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}

	/* 2. Add this stream handle to splitter.*/
	mrRet = SplitterAddStmHandle(prParam->pvSptHdl, pvStm);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail in Add Stream Handle!\r\n"),
			DMX_FUNC_NAME);
		STREAM_UNLOCK(prSpt);
		MM_RETURN(mrRet);
	}

	/* 3. Create Parser Filter and connect it.*/
	mrRet = PSR_Filter_Create(pvDmxInst, prParam->u4StmType,
		prParam->u4StmUID, &pvPsrFtr);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail in  Stream create Pfr, StmType: 0x%x, ")
			TEXT("StmUID: 0x%x\r\n"),
			DMX_FUNC_NAME, prParam->u4StmType, prParam->u4StmUID);
		goto ERRSTREAMCREATE1;
	} else
		prStm->pvPsrFtr = pvPsrFtr;

	/* 4. Add Parser Filter to Parser CC*/
	mrRet = PSR_CC_AttachFilter(prSpt->pvPsrCC, pvPsrFtr);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail in Attach Pfr to PsrCC, StmType: 0x%x, ")
			TEXT("StmUID: 0x%x\r\n"),
			DMX_FUNC_NAME, prParam->u4StmType, prParam->u4StmUID);
		goto ERRSTREAMCREATE2;
	}

	prStm->pvPsrCC = prSpt->pvPsrCC;

	/* 5. Set Parser Filter type, Alloc Psr Filter Specific Info according to its type, Create ESM,*/
	/* and Set ESM Memory, and Register ESM Demuxer CB*/
	mrRet = PSR_Filter_SetType(prStm->pvPsrFtr,
		(E_SPT_DATA_TYPE_T)(prParam->u4StmType),
		prParam->u8DecSendBufMask);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail in Set Psr Filter Type, StmType: 0x%x, ")
			TEXT("StmUID: 0x%x\r\n"),
			DMX_FUNC_NAME, prParam->u4StmType, prParam->u4StmUID);
		goto ERRSTREAMCREATE3;
	}

	mm_memset(&rGauParam, 0, sizeof(rGauParam));
	rGauParam.pvSptHdl		  = prParam->pvSptHdl;
	rGauParam.u4StmType   = prParam->u4StmType;
	rGauParam.u4StmUID	  = prParam->u4StmUID;
	rGauParam.u8DecSendBufMask = prParam->u8DecSendBufMask;
	rGauParam.u4QueueElemCnt = DMX_GAU_GETAU_Q_ELEM_CNT;
	rGauParam.pu4Handle = (u32 *)&(prStm->u4GAUHandle);

	/* 6. Connect GAU(Create ESM), Register ESM Decoder CB*/
	mrRet = GAU_Connect(&rGauParam);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail in Connect GAU Instance, StmType: 0x%x,")
			TEXT(" StmUID: 0x%x\r\n"),
			DMX_FUNC_NAME, prParam->u4StmType, prParam->u4StmUID);
		goto ERRSTREAMCREATE3;
	}

	((PSR_FILTER *)pvPsrFtr)->u4GAU = prStm->u4GAUHandle;

	*ppvHandle = pvStm;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
		TEXT("[STM] %s(%s) success, prParam->u8DecSendBufMask: 0x%llx!\r\n"),
		DMX_FUNC_NAME, DMX_SPTDATATYPE_STR(prParam->u4StmType),
		prParam->u8DecSendBufMask);

	STREAM_UNLOCK(prSpt);

	MM_RETURN(RET_DMX_OK);

ERRSTREAMCREATE3:
	PSR_CC_DetachFilter(prSpt->pvPsrCC, pvPsrFtr);
ERRSTREAMCREATE2:
	SplitterDelStmHandle(prParam->pvSptHdl, pvStm);
ERRSTREAMCREATE1:
	PSR_Filter_Destroy(pvPsrFtr);
	DMX_ASSERT(FALSE);
	STREAM_UNLOCK(prSpt);

	MM_RETURN(mrRet);
}


MRESULT StreamDisable(void *pvStm)
{
	DMX_STM_INST_T *prStm = (DMX_STM_INST_T *)pvStm;
	DMX_CMD_INFO_T rCmd;
	void *pvSptHdl = NULL;
	DMX_STM_ENABLE_INPUTBUF_T rInBuf;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prStm) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s failed for invalid args, (pvStm: 0x%p)!\r\n"),
			DMX_FUNC_NAME, pvStm);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	pvSptHdl = StreamGetSptHandle(pvStm);
	if (NULL == pvSptHdl) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s exit, pvStm's pvSptHdl == NULL, (pvStm: 0x%p)!\r\n"),
			DMX_FUNC_NAME, pvStm);
		MM_RETURN(RET_DMX_OK);
	}

	STREAM_LOCK(pvSptHdl);

	if (!StreamIsEnabled(pvStm)) {
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(RET_DMX_OK);
	}

	#if DMX_DISABLE_VID_STM
	if (prStm->u4StmType == STREAM_VIDEO) {
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(RET_DMX_OK);
	}
	#endif /* DMX_DISABLE_VID_STM*/

	#if DMX_DISABLE_AUD_STM
	if (prStm->u4StmType == STREAM_AUDIO) {
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(RET_DMX_OK);
	}
	#endif /* DMX_DISABLE_AUD_STM*/

	switch (prStm->u4StmType) {
	case STREAM_VIDEO:
	case STREAM_AUDIO:
	case STREAM_SUBTITLE:
	case STREAM_SECTION:
		break;
	default:
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s(%s) success!\r\n"), DMX_FUNC_NAME,
			DMX_SPTDATATYPE_STR(prStm->u4StmType));
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(RET_DMX_OK);
        break;
	}

	mm_memset(&rCmd, 0, sizeof(rCmd));
	mm_memset(&rInBuf, 0, sizeof(rInBuf));

	rInBuf.fgEnable = FALSE;
	rInBuf.pvStm = pvStm;

	rCmd.eCmd		= DMX_CMD_STM_ENABLE;
	rCmd.fgASync	= FALSE;
	rCmd.pvInBuf	= &rInBuf;
	rCmd.u4InBufSz	= sizeof(rInBuf);
	rCmd.u4UsrEvts	= SPLITTER_UEV_STM_ENABLE;
	rCmd.u4WaitTime = DMX_PSR_WAIT_OFF_MAXTIME * 2;

	mrRet = SplitterSendCmd(prStm->pvSptHdl, &rCmd);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[SPT] %s fail in SplitterSendCmd(STM_ENABLE), ")
			TEXT("pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, prStm->pvSptHdl, mrRet);
		DMX_ASSERT(FALSE);
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(mrRet);
	}

	if (DMX_FAILED(rCmd.mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[SPT] %s fail in exceed cmd STM_ENABLE, ")
			TEXT("pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, prStm->pvSptHdl, rCmd.mrRet);
		DMX_ASSERT(FALSE);
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
		TEXT("[STM] %s(%s) success!\r\n"),
		DMX_FUNC_NAME, DMX_SPTDATATYPE_STR(prStm->u4StmType));

	STREAM_UNLOCK(pvSptHdl);

	MM_RETURN(RET_DMX_OK);
}


/****************************************************/
/* StreamDestroy*/
/* 1.  Disconnect this stream to GAU(Destroy ESM)*/
/* 2.  Remove its Corresponding Parser Filter from the Splitter's Parser CC*/
/* 2.  Destroy the Parser Filter of this stream*/
/****************************************************/
MRESULT StreamDestroy(void *pvSptHdl, void *pvStm)
{
	DMX_STM_INST_T *prStm = (DMX_STM_INST_T *)pvStm;
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	u32	u4StmType = SPT_DATA_UNDEFINE;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvSptHdl) ||
		(NULL == pvStm)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s failed for invalid args, (pvStm: 0x%p, pvSptHdl: 0x%p)!\r\n"),
			DMX_FUNC_NAME, pvStm, pvSptHdl);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	STREAM_LOCK(pvSptHdl);

	u4StmType = prStm->u4StmType;

	switch (u4StmType) {
	case STREAM_VIDEO:
	case STREAM_AUDIO:
	case STREAM_SUBTITLE:
	case STREAM_SECTION:
		break;

	default:
		if (u4StmType < MAX_SPT_DATA_TYPE_CNT)
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
				TEXT("[STM] %s(%s) success!\r\n"), DMX_FUNC_NAME,
				g_aszSptDataTypeName[u4StmType]);
		else
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
				TEXT("[STM] %s(%s) success!\r\n"), DMX_FUNC_NAME,
				TEXT("UNKNOWN"));
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(RET_DMX_OK);
        break;
	}

	mrRet = GAU_Disconnect((u32)prStm->u4GAUHandle);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s failed in GAU_Disconnect, (pvStm: 0x%p, pvSptHdl: 0x%p)!\r\n"),
			DMX_FUNC_NAME, pvStm, pvSptHdl);
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(mrRet);
	}

	if (StreamIsEnabled(pvStm)) {
		STREAM_UNLOCK(pvSptHdl);
		StreamDisable(pvStm);
		STREAM_LOCK(pvSptHdl);
	}

	if (NULL != prStm->pvPsrFtr) {
		if (NULL != prSpt->pvPsrCC) {
			mrRet = PSR_CC_DetachFilter(prSpt->pvPsrCC, prStm->pvPsrFtr);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
					TEXT("[STM] %s failed in PSR_CC_DetachFilter, ")
					TEXT("pvPsrCC: 0x%p, pvPsrFtr: 0x%p)!\r\n"),
					DMX_FUNC_NAME, prSpt->pvPsrCC, prStm->pvPsrFtr);
			}
		}

		/* Delete this stream handle to splitter.*/
		mrRet = SplitterDelStmHandle(pvSptHdl, pvStm);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
				TEXT("[STM] %s fail in Delete Stream Handle!\r\n"),
				DMX_FUNC_NAME);
		}

		mrRet = PSR_Filter_Destroy(prStm->pvPsrFtr);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
				TEXT("[STM] %s failed in PSR_Filter_Destroy, (pvPsrFtr: 0x%p, pvSptHdl: 0x%p)!\r\n"),
				DMX_FUNC_NAME, prStm->pvPsrFtr, pvSptHdl);
		}
	}

	if (DMX_SUCCEED(mrRet)) {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s(%s) success!\r\n"), DMX_FUNC_NAME,
			((u4StmType < MAX_SPT_DATA_TYPE_CNT) ? g_aszSptDataTypeName[u4StmType] : TEXT("UNKNOWN")));
	}

	prStm->pvSptHdl = NULL;
	prStm->pvPsrCC = NULL;
	prStm->pvPsrFtr = NULL;
	prStm->pvDmxInst = NULL;

	prStm->u4StmUID = DMX_INVALID_UINT32;
	prStm->u4StmType = STREAM_NONE;

	STREAM_UNLOCK(pvSptHdl);

	MM_RETURN(mrRet);
}

/****************************************************/
/* StmDisconnectPsr*/
/* Remove all Parser Filters from the Splitter's Parser CC*/
/****************************************************/
MRESULT StmDisconnectPsr(void *pvSptHdl)
{
	void	*pvPsr;
	void	**ppvStmHandles;
	u32	u4StmHandleNs;
	u32	i;
	MRESULT mrRet = RET_DMX_OK;

	/* Get its Parser CC*/
	pvPsr = SplitterGetPtxHandle(pvSptHdl);

	if (NULL == pvPsr)
		MM_RETURN(RET_DMX_OK);

	ppvStmHandles = SplitterGetStmHandles(pvSptHdl, &u4StmHandleNs);
	if (NULL == ppvStmHandles) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail in SplitterGetStmHandles, pvSptHdl: 0x%p \r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		MM_RETURN(RET_DMX_NOT_FOUND);
	}

	for (i = 0; i < u4StmHandleNs; i++) {
		if ((NULL == (DMX_STM_INST_T *)ppvStmHandles[i]) ||
			(NULL == ((DMX_STM_INST_T *)ppvStmHandles[i])->pvPsrFtr)) {
			continue;
		}
		mrRet = PSR_CC_DetachFilter(pvPsr, ((DMX_STM_INST_T *)ppvStmHandles[i])->pvPsrFtr);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
				TEXT("[STM] %s fail in PSR_CC_DetachFilter, pvSptHdl: 0x%p \r\n"),
				DMX_FUNC_NAME, pvSptHdl);
			MM_RETURN(mrRet);
		}
	}

	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
		TEXT("[STM] %s success!\r\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

/****************************************************/
/* StreamConnectSptByHandle*/
/* 1. Add the Stream handle to the splitter*/
/* 2. Create Parser Filter for this stream*/
/* 3. Add this Parser Filter to the splitter's Parser CC*/
/****************************************************/
MRESULT StreamConnectSptByHandle(void *pvStm, void *pvSptHdl)
{
	DMX_STM_INST_T *prStm = (DMX_STM_INST_T *)pvStm;
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	void	*pvPsrFtr = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (!((NULL != prStm) && (NULL != pvSptHdl))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
			DMX_FUNC_NAME, __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prStm->fgInUsing = TRUE;

	mrRet = SplitterAddStmHandle(pvSptHdl, pvStm);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail in SplitterAddStmHandle, pvStm: 0x%p Spth: 0x%x\r\n"),
			DMX_FUNC_NAME, pvStm, pvSptHdl);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	mrRet = PSR_Filter_Create(prSpt->pvDmxInst, STREAM_NONE, prStm->u4CompId, &pvPsrFtr);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail in PSR_Filter_Create, pvStm: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvStm, mrRet);
		DMX_ASSERT(FALSE);
		SplitterDelStmHandle(pvSptHdl, pvStm);
		MM_RETURN(mrRet);
	}

	mrRet = PSR_CC_AttachFilter(((DMX_SPT_INST_T *)pvSptHdl)->pvPsrCC, pvPsrFtr);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail in PSR_Filter_Create, pvStm: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvStm, mrRet);
		DMX_ASSERT(FALSE);
		PSR_Filter_Destroy((PSR_FILTER *)pvPsrFtr);
		SplitterDelStmHandle(pvSptHdl, pvStm);
		MM_RETURN(mrRet);
	}

	prStm->pvPsrFtr = pvPsrFtr;
	prStm->pvSptHdl    = pvSptHdl;
	prStm->pvPsrCC  = ((DMX_SPT_INST_T *)pvSptHdl)->pvPsrCC;
	prStm->pvDmxInst = prSpt->pvDmxInst;

	MM_RETURN(RET_DMX_OK);
}

/****************************************************/
/* StreamDisconnectSptByHandle*/
/* 1. Destroy the stream's ESM, Reset Parser Filter to 0*/
/* 2. Remove the Stream handle from the Splitter's stream handle array*/
/* 3. Set the Stream's Spt handle to NULL and fgInUsing to be FALSE*/
/****************************************************/
MRESULT StreamDisconnectSptByHandle(void *pvStm, void *pvSptHdl)
{
	DMX_STM_INST_T *prStm = (DMX_STM_INST_T *)pvStm;
	MRESULT  mrRet = RET_DMX_OK;
	void	*ptrPsr  = NULL;

	if (NULL == pvStm) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s failed for invalid args, (pvStm: 0x%p, pvSptHdl: 0x%p)!\r\n"),
			DMX_FUNC_NAME, pvStm, pvSptHdl);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	/* 1. Check if the connection is established */
	if (prStm->pvSptHdl != pvSptHdl) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail for Strm's pvSptHdl != pvSptHdl!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	/* 2. Check if already disable (Already delete filter driver?) */
	if (StreamIsEnabled(pvStm)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s failed for Stream is Enable, StmType: 0x%x!\r\n"),
			DMX_FUNC_NAME, prStm->u4StmType);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	ptrPsr = SplitterGetPtxHandle(pvSptHdl);

	if (NULL == ptrPsr) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s failed in SplitterGetPtxHandle!\r\n"),
			DMX_FUNC_NAME, prStm->u4StmType);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	mrRet = PSR_CC_DetachFilter(ptrPsr, prStm->pvPsrFtr);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail in PSR_CC_DetachFilter, pvSptHdl: 0x%p \r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		MM_RETURN(mrRet);
	}

	mrRet = PSR_Filter_Destroy(prStm->pvPsrFtr);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s failed in PSR_Filter_Destroy!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(mrRet);
	}

	/* 3. Initial Splitter Disconnect Variable */
	mrRet = SplitterDelStmHandle(pvSptHdl, pvStm);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s failed in SplitterDelStmHandle!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(mrRet);
	}

	/* 4. Initial Filter Disconnect Variable */
	prStm->pvSptHdl = NULL;
	prStm->pvDmxInst = NULL;

	/* 5. Initial Filter Connected Variable */
	prStm->fgInUsing = FALSE;

	MM_RETURN(RET_DMX_OK);
}

/****************************************************/
/* StreamGetFifoFullness*/
/* Get the Available data size of the Stream's FIFO*/
/****************************************************/
MRESULT StreamGetFifoFullness(void *pvStm, u32 *pu4Size)
{
	void	*pvSptHdl = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvStm) ||
		(NULL == pu4Size)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s failed for invalid args (pvStm: 0x%p, pu4Size: 0x%x)!\r\n"),
			DMX_FUNC_NAME, pvStm, pu4Size);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	pvSptHdl = StreamGetSptHandle(pvStm);
	if (NULL == pvSptHdl) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s exit, pvStm's pvSptHdl == NULL, (pvStm: 0x%p)!\r\n"),
			DMX_FUNC_NAME, pvStm);
		MM_RETURN(RET_DMX_OK);
	}

	STREAM_LOCK(pvSptHdl);
	if (!StreamIsEnabled(pvStm)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s failed for Stream is disabled!\r\n"),
			DMX_FUNC_NAME);
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	mrRet = PSR_Filter_GetFifoAvailSize(((DMX_STM_INST_T *)pvStm)->pvPsrFtr, pu4Size);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s failed in PSR_Filter_GetFifoAvailSize, u4StmType: 0x%x!\r\n"),
			DMX_FUNC_NAME, ((DMX_STM_INST_T *)pvStm)->u4StmType);
	}
	STREAM_UNLOCK(pvSptHdl);

	MM_RETURN(mrRet);
}


/****************************************************/
/* StreamEnable*/
/* 1. Set the Stream to be in Use*/
/* 2. Set the stream idx for the cfa*/
/* 3. Enable the stream in cfa, i.e. add the parser flag to cfa's u4PsrFlags*/
/* 4. Enable Parser Filter of the stream, it does these:*/
/*	   1) Set the Parser Filter's flag to be FF_ENABLE*/
/*	   2) If it is Video Parser Filter, Initialize its private data(pvFilterSpecific --PSR_VFSD),*/
/*initialize PES Header Buffer*/
/****************************************************/
MRESULT StreamEnable(void *pvStm)
{
	DMX_STM_INST_T *prStm = (DMX_STM_INST_T *)pvStm;
	DMX_CMD_INFO_T rCmd;
	DMX_STM_ENABLE_INPUTBUF_T rInBuf;
	void	*pvSptHdl = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prStm) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail for invalid args!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	pvSptHdl = StreamGetSptHandle(pvStm);
	if (NULL == pvSptHdl) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail for pvStm's pvSptHdl == NULL, (pvStm: 0x%p)!\r\n"),
			DMX_FUNC_NAME, pvStm);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	STREAM_LOCK(pvSptHdl);

	if (StreamIsEnabled(pvStm)) {
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(RET_DMX_OK);
	}

	if (STREAM_NONE == prStm->u4StmType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s line %d fail for invalid StmType: 0x%x!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prStm->u4StmType);
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(RET_DMX_NO_SET_TYPE);
	}

	#if DMX_DISABLE_VID_STM
	if (prStm->u4StmType == STREAM_VIDEO) {
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(RET_DMX_OK);
	}
	#endif /* DMX_DISABLE_VID_STM*/

	#if DMX_DISABLE_AUD_STM
	if (prStm->u4StmType == STREAM_AUDIO) {
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(RET_DMX_OK);
	}
	#endif /* DMX_DISABLE_AUD_STM*/

	prStm->fgInUsing = TRUE;

	switch (prStm->u4StmType) {
	case STREAM_VIDEO:
	case STREAM_AUDIO:
	case STREAM_SUBTITLE:
	case STREAM_SECTION:
		break;
	default:
		if (prStm->u4StmType < MAX_SPT_DATA_TYPE_CNT)
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
				TEXT("[STM] %s(%s) success!\r\n"), DMX_FUNC_NAME,
				g_aszSptDataTypeName[prStm->u4StmType]);
		else
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
				TEXT("[STM] %s(%s) success!\r\n"), DMX_FUNC_NAME,
				TEXT("UNKNOWN"));
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(RET_DMX_OK);
        break;
	}

	mm_memset(&rCmd, 0, sizeof(rCmd));

	mm_memset(&rInBuf, 0, sizeof(rInBuf));

	rInBuf.fgEnable = TRUE;
	rInBuf.pvStm = pvStm;

	rCmd.eCmd		= DMX_CMD_STM_ENABLE;
	rCmd.fgASync	= FALSE;
	rCmd.pvInBuf	= &rInBuf;
	rCmd.u4InBufSz	= sizeof(rInBuf);
	rCmd.u4UsrEvts	= SPLITTER_UEV_STM_ENABLE;
	rCmd.u4WaitTime = DMX_PSR_WAIT_OFF_MAXTIME;

	mrRet = SplitterSendCmd(prStm->pvSptHdl, &rCmd);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[SPT] %s fail in SplitterSendCmd(STM_ENABLE), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, prStm->pvSptHdl, mrRet);
				STREAM_UNLOCK(pvSptHdl);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (DMX_FAILED(rCmd.mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[SPT] %s fail in exceed cmd STM_ENABLE, pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, prStm->pvSptHdl, rCmd.mrRet);
		STREAM_UNLOCK(pvSptHdl);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
		TEXT("[STM] %s(%s(Type: %d)) success!\r\n"), DMX_FUNC_NAME,
		((prStm->u4StmType < MAX_SPT_DATA_TYPE_CNT) ? g_aszSptDataTypeName[prStm->u4StmType] : TEXT("UNKNOWN")),
		prStm->u4StmType);

	STREAM_UNLOCK(pvSptHdl);

	MM_RETURN(RET_DMX_OK);
}

/****************************************************/
/* StreamSetUID*/
/* Set Stream ID to the Stream's Parser Filter*/
/****************************************************/
MRESULT StreamSetUID(void *pvStm, u32 u4StreamUID)
{
	DMX_STM_INST_T *prStm = (DMX_STM_INST_T *)pvStm;
	void	*pvSptHdl = NULL;
	MRESULT  mrRet	= RET_DMX_OK;

	if (NULL == prStm) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail for invalid args!\r\n"),
			DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	pvSptHdl = StreamGetSptHandle(pvStm);
	if (NULL == pvSptHdl) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s exit, pvStm's pvSptHdl == NULL, (pvStm: 0x%p)!\r\n"),
			DMX_FUNC_NAME, pvStm);
		MM_RETURN(RET_DMX_OK);
	}

	STREAM_LOCK(pvSptHdl);
	if (StreamIsEnabled(pvStm)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s failed for Stream is enable!\r\n"),
			DMX_FUNC_NAME);
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	mrRet = PSR_Filter_SetStreamInfo(prStm->pvPsrFtr, u4StreamUID);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail in PSR_Filter_SetStreamInfo, StmUID: 0x%x!\r\n"),
			DMX_FUNC_NAME, u4StreamUID);
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(mrRet);
	}

	/* We will set CFA UID until the filter enable, to avoid the CFA type is not set*/
	/* This is due to the NV filter issue.*/
	prStm->u4StmUID = u4StreamUID;
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
		TEXT("[STM] %s(%s, StmUID: 0x%x) success!\r\n"),
		DMX_FUNC_NAME,
		((prStm->u4StmType < MAX_SPT_DATA_TYPE_CNT) ? g_aszSptDataTypeName[prStm->u4StmType] : TEXT("UNKNOWN")),
		u4StreamUID);
	STREAM_UNLOCK(pvSptHdl);

	MM_RETURN(RET_DMX_OK);
}

/****************************************************/
/* StreamSetFifoInfo*/
/* 1. Ajust or Create FIFO for the stream according to the fifo size*/
/* 2. Set the FIFO's SA, EA, Sz to the Parser Filter*/
/* 3. Set Fifo SZ to the stream*/
/****************************************************/
MRESULT StreamSetFifoInfo(void *pvStm, u32 u4FifoSz)
{
	DMX_STM_INST_T *prStm = (DMX_STM_INST_T *)pvStm;
	void	*pvSptHdl = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prStm) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail for invalid args!\r\n"),
			DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	pvSptHdl = StreamGetSptHandle(pvStm);
	if (NULL == pvSptHdl) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s exit, pvStm's pvSptHdl == NULL, (pvStm: 0x%p)!\r\n"),
			DMX_FUNC_NAME, pvStm);
		MM_RETURN(RET_DMX_OK);
	}

	STREAM_LOCK(pvSptHdl);
	if (StreamIsEnabled(pvStm)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail for Stream is already Enabled\r\n"),
			DMX_FUNC_NAME);
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	mrRet = PSR_Filter_SetESBufSize(prStm->pvPsrFtr, u4FifoSz);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail in PSR_Filter_SetType, pvStm : 0x%x, u4FifoSz: 0x%x\r\n"),
			DMX_FUNC_NAME, pvStm, u4FifoSz);
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(mrRet);
	}

	prStm->u4FifoSize = u4FifoSz;
	STREAM_UNLOCK(pvSptHdl);

	MM_RETURN(RET_DMX_OK);
}

/****************************************************/
/* StreamSetFifoThreshold*/
/* Set the Threshold for the Stream's FIFO, the threshold is used for that:*/
/* After the stream begin to run, If the fifo data size < the threshold, MW can't get the AU from ESM table.*/
/* When the fifo data size >= threshold, GAU will set the threshold to be 0, and then MW will get the AU success.*/
/****************************************************/
MRESULT StreamSetFifoThreshold(void *pvStm, u32 u4FifoThreshold)
{
	DMX_STM_INST_T *prStm = (DMX_STM_INST_T *)pvStm;
	void	*pvSptHdl = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prStm) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail for invalid args!\r\n"),
			DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	pvSptHdl = StreamGetSptHandle(pvStm);
	if (NULL == pvSptHdl) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s exit, pvStm's pvSptHdl == NULL, (pvStm: 0x%p)!\r\n"),
			DMX_FUNC_NAME, pvStm);
		MM_RETURN(RET_DMX_OK);
	}

	STREAM_LOCK(pvSptHdl);

	if (prStm->u4FifoSize > u4FifoThreshold) {
		prStm->u4FifoThreshold = u4FifoThreshold;

		mrRet = GAU_SetThreshold((u32)prStm->u4GAUHandle, prStm->u4FifoThreshold);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
				TEXT("[STM] %s fail in GAU_SetThreshold, FifoThreshold: 0x%x\r\n"),
				DMX_FUNC_NAME, u4FifoThreshold);
			STREAM_UNLOCK(pvSptHdl);
			MM_RETURN(mrRet);
		}
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(RET_DMX_OK);
	}

	DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
		TEXT("[STM] %s fail for Threshold(0x%x) >= FifoSize(0x%x)\r\n"),
		DMX_FUNC_NAME, u4FifoThreshold, prStm->u4FifoSize);
	STREAM_UNLOCK(pvSptHdl);

	MM_RETURN(RET_DMX_PARAM_WRONG);
}

/****************************************************/
/* StreamSetFlush*/
/* Clear ESM FIFO Data and AU Table*/
/****************************************************/
MRESULT StreamSetFlush(void *pvStm)
{
	DMX_STM_INST_T *prStmInst = NULL;
	void	*pvSptHdl = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == pvStm) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail for invalid args!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	pvSptHdl = StreamGetSptHandle(pvStm);
	if (NULL == pvSptHdl) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s exit, pvStm's pvSptHdl == NULL, (pvStm: 0x%p)!\r\n"),
			DMX_FUNC_NAME, pvStm);
		MM_RETURN(RET_DMX_OK);
	}

	STREAM_LOCK(pvSptHdl);
	if (StreamIsEnabled(pvStm)) {/* We can not flush if is enabled */
		STREAM_UNLOCK(pvSptHdl);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail for Stream is already Enabled\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	prStmInst = (DMX_STM_INST_T *)pvStm;

	mrRet = PSR_Filter_Flush(prStmInst->pvPsrFtr);
	if (DMX_FAILED(mrRet)) {
		STREAM_UNLOCK(pvSptHdl);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail in PSR_Filter_Flush, strmtype: 0x%x\r\n"),
			DMX_FUNC_NAME, prStmInst->u4StmType);
		MM_RETURN(mrRet);
	}
	if (prStmInst->u4StmType < MAX_SPT_DATA_TYPE_CNT)
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s(%s) success!\r\n"),
			DMX_FUNC_NAME,
			g_aszSptDataTypeName[prStmInst->u4StmType]);
	else
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s(%s) success!\r\n"),
			DMX_FUNC_NAME,
			TEXT("UNKNOWN"));

	STREAM_UNLOCK(pvSptHdl);

	MM_RETURN(mrRet);
}

MRESULT StreamGetCodec(void *pvStm, u32 *pu4Codec)
{
	DMX_STM_INST_T *prStm = (DMX_STM_INST_T *)pvStm;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prStm) ||
		(NULL == pu4Codec)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail for invalid args!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*pu4Codec = DMX_INVALID_UINT32;

	if (!StreamIsEnabled(pvStm))
		MM_RETURN(RET_DMX_OK);

	mrRet = PSR_Filter_GetCodeC(prStm->pvPsrFtr, pu4Codec);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail in PSR_Filter_GetCodeC(hPsr: 0x%x, ..)!\r\n"),
			DMX_FUNC_NAME, prStm->pvPsrFtr);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

/****************************************************/
/* GetFreeStreamByType*/
/* Get the Unused Stream whose stream type is equal to the designated type*/
/****************************************************/
void *GetFreeStreamByType(u32 u4StmType)
{
	DMX_STM_INST_T *prStm = NULL;
	u32	i;

	for (i = 0; i < (u32)MAX_STREAM_INSTANCE_CNT; i++) {
		prStm = &(g_rDmxStmMan.arStmInst[i]);

		if ((prStm->u4StmType == u4StmType) &&
			(!prStm->fgInUsing)) {
			return ((void *)prStm);
		}
	}

	return NULL;
}

/****************************************************/
/* GetStreamByType*/
/* Get the handle of the stream whose stream type is equal to the designated type*/
/* and is the stream of the designated splitter*/
/****************************************************/
void *GetStreamByType(void *pvSptHdl, u32 u4StreamType)
{
	DMX_STM_INST_T *prStmInst = NULL;
	u32 i;

	for (i = 0; i < (u32)MAX_STREAM_INSTANCE_CNT; i++) {
		prStmInst = &(g_rDmxStmMan.arStmInst[i]);

		if ((prStmInst->pvSptHdl == pvSptHdl) &&
			(prStmInst->u4StmType == u4StreamType)) {
			return ((void *)prStmInst);
		}
	}

	return NULL;
}

void *GetStreamByTypes(void *pvDmxInst, u32 u4StreamType)
{
	DMX_STM_INST_T *prStmInst = NULL;
	u32 i;

	for (i = 0; i < (u32)MAX_STREAM_INSTANCE_CNT; i++) {
		prStmInst = &(g_rDmxStmMan.arStmInst[i]);

		if (((prStmInst->u4StmType) == u4StreamType) &&
      ((prStmInst->pvDmxInst) == pvDmxInst) &&
			(prStmInst->fgInUsing)) {
			return ((void *)prStmInst);
		}
	}

	return NULL;
}

void *GetStreamPsrFtrByTypes(void *pvDmxInst, u32 u4StreamType)
{
	DMX_STM_INST_T *prStmInst = NULL;
	u32 i;

	for (i = 0; i < (u32)MAX_STREAM_INSTANCE_CNT; i++) {
		prStmInst = &(g_rDmxStmMan.arStmInst[i]);

		if (((prStmInst->u4StmType) == u4StreamType) &&
      ((prStmInst->pvDmxInst) == pvDmxInst) &&
			(prStmInst->fgInUsing)) {
			return prStmInst->pvPsrFtr;
		}
	}

	return NULL;
}

void *GetStreamPsrCCByTypes(void *pvDmxInst, u32 u4StreamType)
{
	DMX_STM_INST_T *prStmInst = NULL;
	u32 i;

	for (i = 0; i < (u32)MAX_STREAM_INSTANCE_CNT; i++) {
		prStmInst = &(g_rDmxStmMan.arStmInst[i]);

		if (((prStmInst->u4StmType) == u4StreamType) &&
      ((prStmInst->pvDmxInst) == pvDmxInst) &&
			(prStmInst->fgInUsing)) {
			return prStmInst->pvPsrCC;
		}
	}

	return NULL;
}

/****************************************************/
/* GetStmUIDByType*/
/* Get the UID of the stream who is in use, and stream type is equal to the designated type*/
/* and is the stream of the designated splitter*/
/****************************************************/
u32 GetStmUIDByType(void *pvSptHdl, u32 u4StreamType)
{
	DMX_STM_INST_T *prStm = NULL;
	u32 i;

	for (i = 0; i < (u32)MAX_STREAM_INSTANCE_CNT; i++) {
		prStm = &(g_rDmxStmMan.arStmInst[i]);

		if (((prStm->pvSptHdl) == pvSptHdl)				  &&
			((prStm->u4StmType) == u4StreamType) &&
			(prStm->fgInUsing)) {
			return prStm->u4StmUID;
		}
	}

	return DMX_INVALID_UINT32;
}

/****************************************************/
/* GetStreamCntFromType*/
/* Get the count -1 of the stream who is in use, and stream type is equal to the designated type*/
/* and is the stream of the designated splitter*/
/****************************************************/
u32 GetStreamCntFromType(void *pvDmxInst, void *pvStm)
{
	DMX_STM_INST_T *prStm = (DMX_STM_INST_T *)pvStm;
	u32	u4Ftr4SptNo = 0;
	u32	i;
	u32	u4StreamType;
	void	*pvSptHdl;

	if (NULL == pvStm) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail for invalid args!\r\n"),
			DMX_FUNC_NAME);
		return MAX_STREAM_INSTANCE_CNT;
	}

	u4StreamType = prStm->u4StmType;
	pvSptHdl = prStm->pvSptHdl;
	for (i = 0; i < (u32)MAX_STREAM_INSTANCE_CNT; i++) {
		prStm = &(g_rDmxStmMan.arStmInst[i]);

		if ((prStm->u4StmType == u4StreamType) &&
			(prStm->pvSptHdl == pvSptHdl) &&
			(prStm->pvDmxInst == pvDmxInst) &&
			(prStm->fgInUsing)) {
			u4Ftr4SptNo++;
		}
	}

	return (u4Ftr4SptNo - 1);
}


/****************************************************/
/* Create DMA Stream, it does the following work:*/
/* 1. Get One unused DMA Stream Instance, Add its handle to Splitter.*/
/* 2. Create Parser Filter for this stream, and Add it to the splitter's Parser CC*/
/* 3. Set the type(SPT_DATA_BUF) of this Parser Filter, Alloc Psr Filter Specific Info according to its type*/
/****************************************************/
MRESULT CreateDmaStm(void *pvSptHdl)
{
	void	*pvStm = GetFreeStreamByType(STREAM_DMA);
	DMX_STM_INST_T *prStm = (DMX_STM_INST_T *)pvStm;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
		TEXT("[STM] %s enter!\r\n"), DMX_FUNC_NAME);

	if (NULL == pvSptHdl) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail for invalid args!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if ((NULL == pvStm) ||
		(NULL == prStm)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail for no unsed DMA Stream Instance!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}

	STREAM_LOCK(pvSptHdl);

	/* Simulate the connection Step */
	mrRet = StreamConnectSptByHandle(pvStm, pvSptHdl);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail in StreamConnectSptByHandle, mrRet: 0x%x!\r\n"),
			DMX_FUNC_NAME, mrRet);
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(mrRet);
	}

	mrRet = PSR_Filter_SetType(prStm->pvPsrFtr, STREAM_DMA, 0);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail PSR_Filter_SetType, pvStm: 0x%p, strmtype: 0x%x\r\n"),
			DMX_FUNC_NAME, pvStm, STREAM_DMA);
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(mrRet);
	}

	prStm->u4StmType = STREAM_DMA;

	mrRet = PSR_Filter_Enable(prStm->pvPsrFtr);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[SPT] %s fail in PSR_Filter_Enable: pvStm: 0x%p, StmType: 0x%x!\r\n"),
			DMX_FUNC_NAME, prStm, prStm->u4StmType);
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(mrRet);
	}

	prStm->fgEnable = TRUE;
	STREAM_UNLOCK(pvSptHdl);

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
		TEXT("[STM] %s Success!\r\n"), DMX_FUNC_NAME);

	MM_RETURN(mrRet);
}

/****************************************************/
/* Create Ground Stream, it does the following work:*/
/* 1. Get One unused Ground Stream Instance, Add its handle to Splitter.*/
/* 2. Create Parser Filter for this stream, and Add it to the splitter's Parser CC*/
/* 3. Set the type(SPT_DATA_GRD) of this Parser Filter*/
/****************************************************/
MRESULT CreateGrdStm(void *pvSptHdl)
{
	void	*pvStm = GetFreeStreamByType(STREAM_INITIAL);
	DMX_STM_INST_T *prStm = (DMX_STM_INST_T *)pvStm;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
		TEXT("[STM] %s enter!\r\n"), DMX_FUNC_NAME);

	if (NULL == pvSptHdl) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail for invalid args!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if ((NULL == pvStm) ||
		(NULL == prStm)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail for no unsed Ground Stream Instance!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}

	STREAM_LOCK(pvSptHdl);
	/* Simulate the connection Step */
	mrRet = StreamConnectSptByHandle(pvStm, pvSptHdl);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail in StreamConnectSptByHandle, mrRet: 0x%x!\r\n"),
			DMX_FUNC_NAME, mrRet);
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(mrRet);
	}

	mrRet = PSR_Filter_SetType(prStm->pvPsrFtr, STREAM_INITIAL, 0);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[STM] %s fail PSR_Filter_SetType, pvStm: 0x%p, strmtype: 0x%x\r\n"),
			DMX_FUNC_NAME, pvStm, STREAM_INITIAL);
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(mrRet);
	}

	prStm->u4StmType = STREAM_INITIAL;

	mrRet = PSR_Filter_Enable(prStm->pvPsrFtr);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[SPT] %s fail in PSR_Filter_Enable: pvStm: 0x%p, StmType: 0x%x!\r\n"),
			DMX_FUNC_NAME, prStm, prStm->u4StmType);
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(mrRet);
	}

	prStm->fgEnable = TRUE;
	STREAM_UNLOCK(pvSptHdl);

	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
		TEXT("[STM] %s Success!\r\n"), DMX_FUNC_NAME);

	MM_RETURN(mrRet);
}


MRESULT DeleteDmaStm(void *pvSptHdl)
{
	void	*pvStm = GetStreamByType(pvSptHdl, SPT_DATA_BUF);
	DMX_STM_INST_T *prStm = (DMX_STM_INST_T *)pvStm;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvSptHdl) ||
		(NULL == prStm)) {
		MM_RETURN(RET_DMX_OK);
	}

	STREAM_LOCK(pvSptHdl);
	mrRet = PSR_Filter_Disable(prStm->pvPsrFtr);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[SPT] %s fail in PSR_Filter_Disable: pvStm: 0x%p, StmType: 0x%x!\r\n"),
			DMX_FUNC_NAME, prStm, prStm->u4StmType);
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(mrRet);
	}
	prStm->fgEnable = FALSE;

	/* Simulate the Dis-connection Step */
	mrRet = StreamDisconnectSptByHandle(pvStm, pvSptHdl);
	STREAM_UNLOCK(pvSptHdl);

	MM_RETURN(mrRet);
}


MRESULT DeleteGrdStm(void *pvSptHdl)
{
	void	*pvStm = GetStreamByType(pvSptHdl, SPT_DATA_GRD);
	DMX_STM_INST_T *prStm = (DMX_STM_INST_T *)pvStm;
	MRESULT mrRet = RET_DMX_OK;

	/* Set Disable */
	if ((NULL == pvSptHdl) ||
		(NULL == prStm)) {
		MM_RETURN(RET_DMX_OK);
	}

	STREAM_LOCK(pvSptHdl);
	mrRet = PSR_Filter_Disable(prStm->pvPsrFtr);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_STM,
			TEXT("[SPT] %s fail in PSR_Filter_Disable: pvStm: 0x%p, StmType: 0x%x!\r\n"),
			DMX_FUNC_NAME, prStm, prStm->u4StmType);
		STREAM_UNLOCK(pvSptHdl);
		MM_RETURN(mrRet);
	}
	prStm->fgEnable = FALSE;

	/* Simulate the Dis-connection Step */
	mrRet = StreamDisconnectSptByHandle(pvStm, pvSptHdl);
	STREAM_UNLOCK(pvSptHdl);

	MM_RETURN(mrRet);
}

