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
 * @file dmx_splitter.c
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
#include "winutil.h"
#include <media/atc/x_dmx.h>
#include <media/atc/ioctl_dmx.h>
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/dmx_cfa_def.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "x_dmx.h"
#include "ioctl_dmx.h"
#include "dmx_define.h"
#include "dmx_splitter.h"
#include "dmx_cfa_def.h"
#include "mm_debug.h"
#endif /* __linux__*/

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_cli.h"
#include "dmx_dump.h"
#include "dmx_spt.h"
#include "dmx_esm.h"
#include "dmx_pbbuf.h"
#include "dmx_spt_os.h"
#include "dmx_spt_main.h"
#include "dmx_spt_rsp.h"
#include "dmx_spt_util.h"
#include "dmx_parser.h"
#include "dmx_psr_esm.h"
#include "dmx_spt_if.h"
#include "dmx_esm_if.h"
#include "dmx_gau_if.h"
#include "dmx_stream.h"
#include "dmx_psr_cc.h"
#include "dmx_psr_pbbuf.h"
#include "cfa_if.h"
#include "dmx_pfm.h"
#include "dmx_inst.h"

#ifndef __linux__
#pragma warning(disable : 4127) /*disable warning C4127: conditional expression is constant*/
#endif

#define DMX_MW_PAUSE_STATE 1

EXTERN DMX_CLI_MAN_T  g_rDmxCliMan;
EXTERN DMX_DUMP_MAN_T g_rDmxDumpMan;
EXTERN u32	g_u4PbBufFlag;
EXTERN bool	g_fgDmxInit;

BOOL g_fgDmxDmaTwice = FALSE;


/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterCreate*/
/* Get one Unused Spt Instance, Create its Parser CC, and Connect PBBUF*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterCreate(void *pvDmxInst, void **ppvHandle)
{
	MRESULT mrRet = RET_DMX_OK;
	DMX_INST_T *prDmxInst = (DMX_INST_T *)pvDmxInst;

	DMXLOG_TRACE(TEXT("[SPT] %s enter.\r\n"), DMX_FUNC_NAME);

	if (NULL == prDmxInst) {
		DMX_ASSERT(0);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	SPLITTER_LOCK;

	mrRet = SptCreateInst(pvDmxInst, ppvHandle);

	if (DMX_SUCCEED(mrRet)) {
		DMXLOG_TRACE(TEXT("[SPT] %s success -- Spt Insts Cnt: %d\r\n"),
			DMX_FUNC_NAME, prDmxInst->u4SptCnt);
	}

	SPLITTER_UNLOCK;

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterDestroy*/
/* Set Splitter's created flag to be false*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterDestroy(void *pvSptHdl)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	DMX_INST_T *prDmxInst = NULL;
	MRESULT mrRet = RET_DMX_OK;

	DMXLOG_TRACE(TEXT("[SPT] %s enter.\r\n"), DMX_FUNC_NAME);

	if (NULL == prSpt) {
		DMX_ASSERT(0);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	prDmxInst = (DMX_INST_T *)prSpt->pvDmxInst;

	if (NULL == prDmxInst) {
		DMX_ASSERT(0);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	SPLITTER_LOCK;

	if (prSpt->fgCreated) {
		mrRet = StmDisconnectPsr(pvSptHdl);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in StmDisconnect")
				TEXT("Psr, mrRet: 0x%x.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		} else {
			DMXLOG_DEBUG(TEXT("[SPT] %s -- StmDisconnectPsr success.\r\n"),
				DMX_FUNC_NAME);
		}

		mrRet = DeleteDmaStm(pvSptHdl);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in DeleteDmaStm, mrRet: 0x%x.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		} else {
			DMXLOG_DEBUG(TEXT("[SPT] %s -- DeleteDmaStm success.\r\n"),
				DMX_FUNC_NAME);
		}

		mrRet = DeleteGrdStm(pvSptHdl);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in DeleteGrdStm, mrRet: 0x%x.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		} else {
			DMXLOG_DEBUG(TEXT("[SPT] %s -- DeleteGrdStm success.\r\n"),
				DMX_FUNC_NAME);
		}

		mrRet = SptCfaUninit(pvSptHdl);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SptCfaUninit, mrRet: 0x%x.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		} else {
			DMXLOG_DEBUG(TEXT("[SPT] %s -- SptCfaUninit success.\r\n"),
				DMX_FUNC_NAME);
		}

		SplitterRspUnInit(pvSptHdl);

		DMXLOG_DEBUG(TEXT("[SPT] %s -- SplitterRspUnInit success\r\n"),
			DMX_FUNC_NAME);

		mrRet = SplitterDeleteTask(prSpt);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s -- fail in delete prSpt(0x%p) task, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, prSpt, mrRet);
		} else {
			DMXLOG_DEBUG(TEXT("[SPT] %s -- SplitterDeleteTask success.\r\n"),
				DMX_FUNC_NAME);
		}

		DMXLOG_DEBUG(TEXT("[SPT] %s -- delete prSpt(0x%p) task success\r\n"),
			DMX_FUNC_NAME, prSpt);

		mrRet = SplitterDeleteEvent(prSpt);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s -- fail in delete prSpt(0x%p) event, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, prSpt, mrRet);
		} else {
			DMXLOG_DEBUG(TEXT("[SPT] %s -- SplitterDeleteEvent success.\r\n"),
				DMX_FUNC_NAME);
		}

		mrRet = SplitterDestroyPsr(pvSptHdl);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterDestroyPsr, mrRet: 0x%x.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		} else {
			DMXLOG_DEBUG(TEXT("[SPT] %s -- SplitterDestroyPsr success.\r\n"),
				DMX_FUNC_NAME);
		}
		
		mrRet = PBBUF_Disable(pvSptHdl);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in PBBUF_Disable, mrRet: 0x%x.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		} else {
			DMXLOG_DEBUG(TEXT("[SPT] %s -- PBBUF_Disable success.\r\n"),
				DMX_FUNC_NAME);
		}

			mrRet = PBBUF_Disconnect(pvSptHdl);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in PBBUF_Disconnect, mrRet: 0x%x.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		} else {
			DMXLOG_DEBUG(TEXT("[SPT] %s -- PBBUF_Disconnect success.\r\n"),
				DMX_FUNC_NAME);
		}

		mrRet = SplitterDeleteSema(prSpt);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s -- fail in delete prSpt(0x%p) semaphore, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, prSpt, mrRet);
		} else {
			DMXLOG_DEBUG(TEXT("[SPT] %s -- SplitterDeleteSema success.\r\n"),
				DMX_FUNC_NAME);
		}

		mrRet = SplitterDeleteCmdQ(prSpt);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s -- fail in delete prSpt(0x%p) CmdQ, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, prSpt, mrRet);
		} else {
			DMXLOG_DEBUG(TEXT("[SPT] %s -- SplitterDeleteCmdQ success.\r\n"),
				DMX_FUNC_NAME);
		}

		prSpt->fgCreated = FALSE;
		prSpt->pvDmxInst = NULL;

		DMXLOG_DEBUG(
			TEXT("[SPT] %s line %d -- Set Splitter(u4SptCompID: %d)'s fgCreated to be FALSE\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt->u4SptCompId);
	}

	SPLITTER_UNLOCK;

	DMXLOG_TRACE(TEXT("[SPT] %s success, exit\r\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterEnable*/
/* Enable Splitter, it does the following:*/
/* 1. Alloc Pbbuf Buffer, Set Slots lists*/
/* 2. Create DMA Stream, Ground Stream*/
/* 3. Initialize Resplitter*/
/* 4. Set Splitter to Enable*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterEnable(
	void					*pvSptHdl,
	DMX_PBBUF_CONFIG_INFO_T *prPbbufCfgInfo) {
	u32 u4CfaType = DMX_INVALID_UINT32;
	MRESULT mrRet	 = RET_DMX_OK;
	u32 u4Size	 = 0;

	if ((NULL == pvSptHdl) ||
		(NULL == prPbbufCfgInfo)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args(pvSptHdl: %p, prPbbufCfgInfo: 0x%p)\r\n"),
			DMX_FUNC_NAME, pvSptHdl, prPbbufCfgInfo);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if ((0 == prPbbufCfgInfo->u4PBBufTotalSz) ||
		(0 == prPbbufCfgInfo->u4PBBufSlotSz) ||
		(prPbbufCfgInfo->u4PBBufTotalSz < prPbbufCfgInfo->u4PBBufSlotSz * 2)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail for invalid args(pvSptHdl: %p, ")
			TEXT("PBBuf: TotalSz(0x%x), SlotSz(0x%x), SptPbbufType(0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, prPbbufCfgInfo->u4PBBufTotalSz,
			prPbbufCfgInfo->u4PBBufSlotSz, prPbbufCfgInfo->eSptPbuffType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (SplitterIsEnable(pvSptHdl)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for pvSptHdl(0x%p) is already enabled.\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	SPLITTER_LOCK;

	if (!SplitterIsTaskRunning(pvSptHdl)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for pvSptHdl(0x%p)'s task thread has already exited.\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}

	u4Size = 0; {
		CfaIntf *prCfaInterface   = (CfaIntf *)SplitterGetCfaInterface(pvSptHdl);
		void	*pvCfaPrivateData = SplitterGetCfaPrivateData(pvSptHdl);

		if ((NULL != prCfaInterface) &&
			(NULL != pvCfaPrivateData) &&
			(NULL != prCfaInterface->pfmrGetParam)) {
			mrRet = SptCfaGetParam(pvSptHdl, CFA_PARAM_ID_JUMP_INFO_SIZE, &u4Size, sizeof(u32));
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT("[SPT] %s line %d fail in PBBUF_Enable, pvSptHdl(0x%p), mrRet")
					TEXT("(0x%x).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
				goto SPTENABLEERR;
			} else {
				DMXLOG_DEBUG(
					TEXT("[SPT] %s -- SptCfaGetParam(CFA_PARAM_ID_JUMP_INFO_SIZE)")
					TEXT(" success.\r\n"),
					DMX_FUNC_NAME);
			}
		}
	}

	/* PBBUF Comp ID is 0, total size is 5M, slot size is 8K*/
	DMXLOG_TRACE(TEXT("[SPT] %s -- PBBuf: TotalSz(0x%x), SlotSz(0x%x)\r\n"),
		DMX_FUNC_NAME, prPbbufCfgInfo->u4PBBufTotalSz, prPbbufCfgInfo->u4PBBufSlotSz);
	mrRet = PBBUF_Enable(pvSptHdl, prPbbufCfgInfo->u4PBBufTotalSz,
		prPbbufCfgInfo->u4PBBufSlotSz, prPbbufCfgInfo->eSptPbuffType, u4Size);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in PBBUF_Enable, pvSptHdl(0x%p), mrRet(0x%x).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		goto SPTENABLEERR;
	} else{
		DMXLOG_DEBUG(TEXT("[SPT] %s -- PBBUF_Enable success.\r\n"),
			DMX_FUNC_NAME);
	}

	u4CfaType = SplitterGetCfaType(pvSptHdl);

	if (DMX_INVALID_UINT32 == u4CfaType) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d for invalid cfa type, pvSptHdl(0x%p).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		goto SPTENABLEERR;
	} else{
		DMXLOG_DEBUG(TEXT("[SPT] %s -- Cfa Type == 0x%x\r\n"),
			DMX_FUNC_NAME, u4CfaType);
	}

	/* Create DMA Stream(It is mainly used for Sync Pbbuf), it does the following work:*/
	/* 1. Get One unused DMA Stream Instance, Add its handle to Splitter.*/
	/* 2. Create Parser Filter for this stream, and Add it to the splitter's Parser CC*/
	/* 3. Set the type(SPT_DATA_BUF) of this Parser Filter, Alloc Psr Filter Specific Info according to its type*/
	mrRet = CreateDmaStm(pvSptHdl);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in CreateDmaStm, ")
			TEXT("pvSptHdl(0x%p), mrRet(0x%x).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		goto SPTENABLEERR;
	} else{
		DMXLOG_DEBUG(TEXT("[SPT] %s -- CreateDmaStm success\r\n"),
			DMX_FUNC_NAME);
	}

	/* Create Ground Stream, it does the following work:*/
	/* 1. Get One unused Ground Stream Instance, Add its handle to Splitter.*/
	/* 2. Create Parser Filter for this stream, and Add it to the splitter's Parser CC*/
	/* 3. Set the type(SPT_DATA_GRD) of this Parser Filter*/
	mrRet = CreateGrdStm(pvSptHdl);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in CreateGrdStm, ")
			TEXT("pvSptHdl(0x%p), mrRet(0x%x).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		goto SPTENABLEERR;
	} else{
		DMXLOG_DEBUG(TEXT("[SPT] %s -- CreateGrdStm success\r\n"),
			DMX_FUNC_NAME);
	}

	mrRet = SplitterSetEnable(pvSptHdl);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterSetEnable,")
			TEXT(" pvSptHdl(0x%p), mrRet(0x%x).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		goto SPTENABLEERR;
	} else{
		DMXLOG_DEBUG(TEXT("[SPT] %s -- SplitterSetEnable success\r\n"),
			DMX_FUNC_NAME);
	}

	/*g_rDmxCliMan.fgDumpFlow = TRUE;*/

	DMXLOG_TRACE(TEXT("[SPT] %s Success, pvSptHdl: %p.\r\n"),
		DMX_FUNC_NAME, pvSptHdl);

	SPLITTER_UNLOCK;

	MM_RETURN(RET_DMX_OK);

SPTENABLEERR:
	SplitterRspUnInit(pvSptHdl);
	DeleteGrdStm(pvSptHdl);
	DeleteDmaStm(pvSptHdl);
	PBBUF_Disable(pvSptHdl);
	StmDisconnectPsr(pvSptHdl);
	SplitterDestroyPsr(pvSptHdl);
	SptCfaUninit(pvSptHdl);

	SPLITTER_UNLOCK;

	DMXLOG_TRACE(TEXT("[SPT] %s fail, pvSptHdl: %p, mrRet:0x%x.\r\n"),
		DMX_FUNC_NAME, pvSptHdl, mrRet);

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterDisable*/
/* Disable Splitter, it does the following:*/
/* 1. Disable the designated splitter instance*/
/* 2. Remove all Parser Filters from the Splitter's Parser CC*/
/* 3. Delete DMA Stream, Ground Stream*/
/* 4. Destory PBBUF*/
/* 5. Call Current Cfa's UnInit function, Set cfa's Private data to be NULL*/
/* 6. Unitialize Resplitter*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterDisable(void *pvSptHdl)
{
	u32 u4SptState = SPLITTER_STATE_NONE;
	MRESULT mrRet = RET_DMX_OK;

	if (!SplitterIsEnable(pvSptHdl))
		MM_RETURN(RET_DMX_OK);

	SPLITTER_LOCK;

	if (g_rDmxCliMan.fgDumpFlow) {
		DmxCloseDumpFlowFile();
		g_rDmxDumpMan.rFlowInfo.u4FlowDumpCnt++;
	}

	u4SptState = SplitterGetState(pvSptHdl);

	if (SPLITTER_STATE_IDLE != u4SptState) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for Can not do disable op ")
			TEXT("under non-idle(%d) state, pvSptHdl(0x%p)\r\n"),
			DMX_FUNC_NAME, u4SptState, pvSptHdl);
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	mrRet = SplitterSetDisable(pvSptHdl);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in SplitterSetDisable, mrRet: 0x%x.\r\n"),
			DMX_FUNC_NAME, mrRet);
		SPLITTER_UNLOCK;
		MM_RETURN(mrRet);
	}

	DMXLOG_TRACE(TEXT("[SPT] %s Success, pvSptHdl: %p.\r\n"),
		DMX_FUNC_NAME, pvSptHdl);
	SPLITTER_UNLOCK;

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterSetRate*/
/* What? mtk40144*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterSetRate(void *pvSptHdl, s32 i4Rate, bool fgDmaAud)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	if (NULL == pvSptHdl) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args(pvSptHdl: %p, Rate: %d).\r\n"),
			DMX_FUNC_NAME, pvSptHdl, i4Rate);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	SPLITTER_LOCK;
	if (NULL != GetStreamByType(pvSptHdl, SPT_DATA_A)) {
		prSpt->fgDmaAud = fgDmaAud;
		DMXLOG_TRACE(
			TEXT("[SPT] %s change rate from %d to %d, fgDmaAud: %d, pvSptHdl: %p.\r\n"),
			DMX_FUNC_NAME, prSpt->i4Rate, i4Rate, (fgDmaAud ? 1 : 0), pvSptHdl);
	} else{
		DMXLOG_TRACE(
			TEXT("[SPT] %s change rate from %d to %d, pvSptHdl: %p.\r\n"),
			DMX_FUNC_NAME, prSpt->i4Rate, i4Rate, pvSptHdl);
	}

	prSpt->i4Rate = i4Rate;

	#if DMX_DISABLE_AUD_DMA
	prSpt->fgDmaAud = FALSE;
	#endif /* DMX_DISABLE_AUD_DMA*/

	SPLITTER_UNLOCK;

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterSetLastMem*/
/* Set Splitter's Parser CC's last mem flag*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterSetLastMem(void *pvSptHdl, bool fgLastMem)
{
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == pvSptHdl) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args(pvSptHdl: %p, ")
			TEXT("fgLastMem: %d).\r\n"),
			DMX_FUNC_NAME, pvSptHdl, (fgLastMem ? 1 : 0));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mrRet = SplitterSetPsrLastMem(pvSptHdl, fgLastMem);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterSetPsrLastMem")
			TEXT(" (pvSptHdl: %p, fgLastMem: %d).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, (fgLastMem ? 1 : 0));
	}

	MM_RETURN(mrRet);
}



/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterSetCfaType*/
/* 1. Clear previous CFA setting*/
/* 2. Get the Cfa interface group pointer by the designated cfa type*/
/* 3. Set The Splitter's Cfa type and interface*/
/* 4. Call the corresponding cfa's init function, set private data of cfa*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterSetCfaType(void *pvSptHdl, u32 u4CfaType)
{
	void	*pvCfaInterface = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == pvSptHdl) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args (pvSptHdl: %p, ")
			TEXT("u4CfaType: 0x%x)\r\n"),
			DMX_FUNC_NAME, pvSptHdl, u4CfaType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	SPLITTER_LOCK;
	/* Get Cfa Interface Group by the Cfa Type*/
	pvCfaInterface = SptGetCfaInterface(u4CfaType);

	/* We must clear previous CFA setting here */
	/* Call Current Cfa's UnInit function, Set cfa's Private data to be NULL*/
	mrRet = SptCfaUninit(pvSptHdl);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in SptCfaUninit, pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
		SPLITTER_UNLOCK;
		MM_RETURN(mrRet);
	}

	/* Due to the interface will not set by LPE or MPC, we check the interface pointer and set it */
	if (NULL == pvCfaInterface) {
		((DMX_SPT_INST_T *)pvSptHdl)->u4CfaType = DMX_INVALID_UINT32;
		((DMX_SPT_INST_T *)pvSptHdl)->pvCfaInterface = NULL;
		DMXLOG_ERROR(TEXT("[SPT] %s fail for pvCfaInterface == NULL,")
			TEXT(" pvSptHdl: %p, CfaType: 0x%x \r\n"),
			DMX_FUNC_NAME, pvSptHdl, u4CfaType);
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_NO_CFA_INTERFACE);
	}

	((DMX_SPT_INST_T *)pvSptHdl)->u4CfaType = u4CfaType;
	((DMX_SPT_INST_T *)pvSptHdl)->pvCfaInterface = pvCfaInterface;

	/* The cfa private data will set here */
	/* Call Current Cfa's Init function, set Private data of cfa*/
	mrRet = SptCfaInit(pvSptHdl);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail in SptCfaInit pvSptHdl: %p\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		SPLITTER_UNLOCK;
		MM_RETURN(mrRet);
	}

	DMXLOG_TRACE(TEXT("[SPT] %s Success, pvSptHdl: %p, CfaType: 0x%x.\r\n"),
		DMX_FUNC_NAME, pvSptHdl, u4CfaType);
	SPLITTER_UNLOCK;

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterSetCfaConfigure*/
/* Configure Cfa using the Cfa Configuration Info*/
/* @Param pvCfaParameter [in] Cfa Configuration Info. e.g. CfaAviConfigInfo*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterSetCfaConfigure(void *pvSptHdl, void *pvCfaParameter, bool fgIsUserMem)
{
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvSptHdl) ||
		(NULL == pvCfaParameter)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args (pvSptHdl: %p, ")
			TEXT("CfaParam: 0x%x)\r\n"),
			DMX_FUNC_NAME, pvSptHdl, pvCfaParameter);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterIsEnable(pvSptHdl)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for Splitter is disable, pvSptHdl: %p\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	SPLITTER_LOCK;

	/*TODO: Check it after Parser Call back ready and Pause cmd execute simultaneously*/
	mrRet = SptCfaConfigure(pvSptHdl, pvCfaParameter, fgIsUserMem);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in SptCfaConfigure, pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
		SPLITTER_UNLOCK;
		MM_RETURN(mrRet);
	}

	SPLITTER_UNLOCK;

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterSetCfaRange*/
/* Set Cfa Range Information*/
/* @Param pvCfaRange [in] Cfa Range Info. e.g. CfaAviRange*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterSetCfaRange(void *pvSptHdl, void *pvCfaRange, bool fgIsUserMem)
{
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvSptHdl) ||
		(NULL == pvCfaRange)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args (pvSptHdl: %p, pvCfaRange: 0x%x)\r\n"),
			DMX_FUNC_NAME, pvSptHdl, pvCfaRange);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterIsEnable(pvSptHdl)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for Splitter is disable, pvSptHdl: %p\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	SPLITTER_LOCK;

	/*TODO: Check it after Parser Call back ready and Pause cmd execute simultaneously*/
	mrRet = SptCfaSetRange(pvSptHdl, pvCfaRange, fgIsUserMem);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in SptSetCfaRange, pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);

		SPLITTER_UNLOCK;

		MM_RETURN(mrRet);
	}

	SPLITTER_UNLOCK;

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterSetCfaInquire*/
/* Set Cfa Inquire Type*/
/* @Param u4InquirerTypes [in]	Cfa Inquire Type*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterSetCfaInquire(void *pvSptHdl, u32 u4InquirerTypes)
{
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == pvSptHdl) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args(pvSptHdl: %p,")
			TEXT(" u4InquirerTypes: 0x%x)\r\n"),
			DMX_FUNC_NAME, pvSptHdl, u4InquirerTypes);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterIsEnable(pvSptHdl)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for Splitter is disable, pvSptHdl: %p\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	SPLITTER_LOCK;

	mrRet = SptCfaSetInquirer(pvSptHdl, u4InquirerTypes);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in SptCfaSetInquirer, pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
		SPLITTER_UNLOCK;
		MM_RETURN(mrRet);
	}

	SPLITTER_UNLOCK;

	MM_RETURN(mrRet);
}


MRESULT SplitterGetCfaPosition(void *pvSptHdl, void *pvCfaPosition)
{
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvSptHdl) || (NULL == pvCfaPosition)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args(pvSptHdl: %p,")
			TEXT(" pvCfaPosition: 0x%x)\r\n"),
			DMX_FUNC_NAME, pvSptHdl, pvCfaPosition);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterIsEnable(pvSptHdl)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for Splitter is disable, pvSptHdl: %p\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	SPLITTER_LOCK;
	mrRet = SptCfaGetPosition(pvSptHdl, pvCfaPosition);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in SptCfaGetPosition, pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
		SPLITTER_UNLOCK;
		MM_RETURN(mrRet);
	}
	SPLITTER_UNLOCK;

	MM_RETURN(mrRet);
}


MRESULT SplitterGetCfaGeneral(void *pvSptHdl, u32 u4CfaFID, void *pvCfaParameter,
						  u32 u4CfaParameterSize)
{
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvSptHdl) ||
		(NULL == pvCfaParameter)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args(pvSptHdl: %p,")
			TEXT(" u4CfaFID: 0x%x, pvCfaParameter: 0x%x)\r\n"),
			DMX_FUNC_NAME, pvSptHdl, u4CfaFID, pvCfaParameter);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	SPLITTER_LOCK;
	mrRet = SptCfaGetGeneral(pvSptHdl, u4CfaFID, pvCfaParameter, u4CfaParameterSize);
	if (DMX_FAILED(mrRet) && (!MM_IS_STATE_ERROR(mrRet))) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in SptCfaGetGeneral, pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
		SPLITTER_UNLOCK;
		MM_RETURN(mrRet);
	}
	SPLITTER_UNLOCK;

	MM_RETURN(mrRet);
}


MRESULT SplitterSetCfaGeneral(void *pvSptHdl, u32 u4CfaFID, void *pvCfaParameter,
						 u32 u4CfaParameterSize) {
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvSptHdl) ||
		(NULL == pvCfaParameter)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args(pvSptHdl: %p,")
			TEXT(" u4CfaFID: 0x%x, pvCfaParameter: 0x%x)\r\n"),
			DMX_FUNC_NAME, pvSptHdl, u4CfaFID, pvCfaParameter);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	SPLITTER_LOCK;
	mrRet = SptCfaSetGeneral(pvSptHdl, u4CfaFID, pvCfaParameter, u4CfaParameterSize);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in SptCfaSetGeneral, pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
		SPLITTER_UNLOCK;
		MM_RETURN(mrRet);
	}
	SPLITTER_UNLOCK;

	MM_RETURN(mrRet);
}

/*//////////////////////////////////////////////////////////////////////////////*/
/* SplitterGetStmFifoFullness*/
/* Get the available data size of the designated data type stream*/
/*//////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterGetStmFifoFullness(
	void *pvSptHdl,
	u32 u4SptDataType,
	u32 *pu4FullNess) {
	void	*pvStm = NULL, *pvPsrFtr = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pu4FullNess) ||
		(NULL == pvSptHdl)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args, pvSptHdl: %p, pu4FullNess: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, pu4FullNess);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if ((SPT_DATA_V != u4SptDataType) &&
		(SPT_DATA_A != u4SptDataType)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for unsupport sptdatatype(%d)\r\n"),
			DMX_FUNC_NAME, u4SptDataType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterIsEnable(pvSptHdl)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for Splitter is disable, pvSptHdl: %p\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	SPLITTER_LOCK;
	/* Get the handle of the stream whose stream type is equal to the designated type*/
	/* and is the stream of the designated splitter*/
	pvStm = GetStreamByType(pvSptHdl, u4SptDataType);
	if ((NULL == pvStm) ||
		(!StreamIsEnabled(pvStm))) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for no Designated Stm or Stm ")
			TEXT("is disabled (pvSptHdl: %p, pvStm: 0x%p, SptDataType: 0x%x)\r\n"),
			DMX_FUNC_NAME, pvSptHdl, pvStm, u4SptDataType);
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_NOT_FOUND);
	}

	pvPsrFtr = GetPsrFtrFromStm(pvStm);

	mrRet = PSR_Filter_GetFifoAvailSize(pvPsrFtr, pu4FullNess);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail in PSR_Filter_GetFifoAvailSize")
			TEXT(" (pvSptHdl: %p, SptDataType:0x%x, pvPsrFtr: 0x%p)\r\n"),
			DMX_FUNC_NAME, pvSptHdl, u4SptDataType, pvPsrFtr);
		SPLITTER_UNLOCK;
		MM_RETURN(mrRet);
	}
	SPLITTER_UNLOCK;

	MM_RETURN(RET_DMX_OK);
}

/*//////////////////////////////////////////////////////////////*/
/* SplitterSetParserOn*/
/* If current state is splitter idle or tx rebuf,  set the event -- turn on*/
/*//////////////////////////////////////////////////////////////*/
MRESULT SplitterSetParserOn(DMX_PSR_ON_PARAM_T *prParam)
{
	DMX_STM_CNT_INFO_T *prStmsCnt = NULL;
	DMX_CMD_INFO_T	rCmd;
	void	*pvSptHdl = NULL;
	MRESULT  mrRet		= RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[SPT] %s enter, prParam: 0x%x\r\n"),
		DMX_FUNC_NAME, prParam);

	if (NULL == prParam) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	pvSptHdl = prParam->pvSptHdl;
	prStmsCnt = &(prParam->rStmsCnt);

	if (NULL == pvSptHdl) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args, pvSptHdl: %p, prStmsCnt: 0x%p\r\n"),
			DMX_FUNC_NAME, pvSptHdl, prStmsCnt);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterIsEnable(pvSptHdl)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for Splitter is disable, pvSptHdl: %p\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		DMX_ASSERT(FALSE);
		SplitterSetEOSForError(pvSptHdl, RET_DMX_UNEXPECT);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	SPLITTER_LOCK;

	if (!SplitterIsTaskRunning(pvSptHdl)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for pvSptHdl(0x%p)'s task thread has already exited.\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		SplitterSetEOSForError(pvSptHdl, RET_DMX_OS_OPERA_FAIL);
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}

	/* Rsp init total memory allocation */
	/* if rsplitter initialize fail, the file can also play, so we don't return errcode*/
	mrRet = SplitterRspInit(pvSptHdl, prStmsCnt);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in SplitterRspInit, pvSptHdl(0x%p), mrRet(0x%x).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
	}

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = pvSptHdl;
		DmxDumpFlow(DMX_OPER_PSR_ON, &rOperInfo);
	} 

	{
		DMX_STM_INST_T *prAStm = GetStreamByType(pvSptHdl, SPT_DATA_A);

		if (NULL != prAStm) {
			PSR_FILTER *prPsrFtr = (PSR_FILTER *)(prAStm->pvPsrFtr);

			if ((NULL != prPsrFtr) &&
				(DMX_INVALID_UINT32 != prPsrFtr->u4ESIH)) {
				DMXLOG_TRACE(TEXT("[SPT] %s line %d -- ESM_CheckFifoClearStatus(%d)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH);
				ESM_CheckFifoClearStatus(prPsrFtr->u4ESIH);
			}
		}
	}

#if DMX_PFM_TEST
	DmxPfmInstStart(pvSptHdl);
#endif /* DMX_PFM_TEST*/

	smp_mb();

#ifdef __linux__
	DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- SplitterSetDecryptId(%d)\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prParam->i4DecryptId);
	SplitterSetDecryptId(pvSptHdl, prParam->i4DecryptId);
#else
	/* CE default set i4DecryptID to be DECRYPT_PLAY_INVALID_ID in SplitterCreateInst*/
#endif /* __linux__*/

	SPLITTER_UNLOCK;

	DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- SplitterResetAllEvents(prSpt: 0x%p)\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
	mrRet = SplitterResetAllEvents(pvSptHdl);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterResetAll")
			TEXT("Events, (pvSptHdl: %p, mrRet:0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		DMX_ASSERT(FALSE);
		SplitterSetEOSForError(pvSptHdl, RET_DMX_OS_OPERA_FAIL);
		MM_RETURN(mrRet);
	}

	mm_memset(&rCmd, 0, sizeof(rCmd));

	rCmd.eCmd		= DMX_CMD_PTX_ON;
	rCmd.fgASync	= FALSE;
	rCmd.u4UsrEvts	= SPLITTER_UEV_PTX_ON;
	rCmd.u4WaitTime = DMX_PSR_WAIT_OFF_MAXTIME;

	mrRet = SplitterSendCmd(pvSptHdl, &rCmd);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail in SplitterSendCmd(TURN_ON),")
			TEXT(" pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
		DMX_ASSERT(FALSE);
		SplitterSetEOSForError(pvSptHdl, RET_DMX_OS_OPERA_FAIL);
		MM_RETURN(mrRet);
	}

	if (DMX_FAILED(rCmd.mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in exceed cmd TURN_ON, pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, rCmd.mrRet);
		DMX_ASSERT(FALSE);
		SplitterSetEOSForError(pvSptHdl, RET_DMX_OS_OPERA_FAIL);
		MM_RETURN(mrRet);
	}

	DMXLOG_DEBUG(
		TEXT("[SPT] %s success, exit, pvSptHdl: %p, State: 0x%x\r\n"),
		DMX_FUNC_NAME, pvSptHdl, SplitterGetState(pvSptHdl));

	MM_RETURN(RET_DMX_OK);
}

/*//////////////////////////////////////////////////////////////*/
/* SplitterSetParserOff*/
/* 1. If current state is txing, tx check, tx pause, tx rebuf,	set txrebuf false and abort tx, wait the */
/*state change to idle.*/
/* 2. If current state is idle, release all pbbuf, set ptx off event, clear Rsp log, set rsp threshold to be 0*/
/*//////////////////////////////////////////////////////////////*/
MRESULT SplitterSetParserOff(void *pvSptHdl)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	DMX_INST_T *prDmxInst = NULL;
	DMX_CMD_INFO_T	rCmd;
	MRESULT  mrRet		= RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[SPT] %s enter, pvSptHdl: %p\r\n"),
		DMX_FUNC_NAME, pvSptHdl);

	if (NULL == pvSptHdl) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args, pvSptHdl: %p\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prDmxInst = (DMX_INST_T *)prSpt->pvDmxInst;

	if (NULL == prDmxInst) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterIsEnable(pvSptHdl)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for Splitter is disable, pvSptHdl: %p\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	if (!SplitterIsTaskRunning(pvSptHdl)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for pvSptHdl(0x%p)'s task thread has already exited.\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}

	mm_memset(&rCmd, 0, sizeof(rCmd));

	rCmd.eCmd = DMX_CMD_PTX_OFF;
	rCmd.fgASync = FALSE;
	rCmd.u4UsrEvts = SPLITTER_UEV_PTX_OFF;
	rCmd.u4WaitTime = DMX_PSR_WAIT_OFF_MAXTIME;

	mrRet = SplitterSendCmd(prSpt, &rCmd);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in SplitterSendCmd(RSP_OFF), pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (DMX_FAILED(rCmd.mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in exceed cmd RSP_OFF, pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, rCmd.mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	DMXLOG_DEBUG(
		TEXT("[SPT] %s line %d -- Get SplitterUsrEvent(SPLITTER_UEV_PTX_OFF) success")
		TEXT(", pvSptHdl: %p\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prSpt);

	SPLITTER_LOCK;

	/* Release All PBBUF.*/
	mrRet = PBBUF_CleanAllSlots(pvSptHdl);
	if (DMX_FAILED(mrRet)) {
		SPLITTER_UNLOCK;
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in PBBUF_CleanAllSlots,")
			TEXT(" pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	mrRet = PSR_CC_Enable(prSpt->pvPsrCC, FALSE);
	if (DMX_FAILED(mrRet)) {
		SPLITTER_UNLOCK;
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in PSR_CC_Enable, ")
			TEXT("pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	mrRet = SplitterRspClearLog(pvSptHdl);
	if (DMX_FAILED(mrRet)) {
		SPLITTER_UNLOCK;
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterRspClearLog,")
			TEXT(" pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	prDmxInst->fgPsrOff = TRUE;

	mrRet = Spt4CfaClearAllGAUEvents(pvSptHdl);
	if (DMX_FAILED(mrRet)) {
		SplitterSetEOSForError(pvSptHdl, mrRet);
		SPLITTER_UNLOCK;
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in Spt4CfaClearAllGAUEvents, pvSptHdl: %p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		MM_RETURN(mrRet);
	}

	SplitterRspUnInit(pvSptHdl);

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = pvSptHdl;
		DmxDumpFlow(DMX_OPER_PSR_OFF, &rOperInfo);
	}

	#if DMX_PFM_TEST
	DmxPfmPrintInfo(pvSptHdl);
	#endif /* DMX_PFM_TEST*/

	DMXLOG_DEBUG(
	TEXT("[SPT] %s success, exit, pvSptHdl: %p, State: 0x%x\r\n"),
		DMX_FUNC_NAME, pvSptHdl, SplitterGetState(pvSptHdl));

	SPLITTER_UNLOCK;

	MM_RETURN(RET_DMX_OK);
}

/*//////////////////////////////////////////////////////////////*/
/* SplitterSetParserPause*/
/* 1. If current state is txing, tx pause, return ok.*/
/* 2. Otherwise, send Splitter Command Pause msg.*/
/* 3. Wait UEV_PTX_PAUSE or UEV_PTX_TO_PAUSE_NG_FINISH event.*/
/* 4. If PTX_PAUSE event, Set RspWaitPsrResume flag to be false*/
/*//////////////////////////////////////////////////////////////*/
MRESULT SplitterSetParserPause(void *pvSptHdl)
{
	DMX_CMD_INFO_T	rCmd;
	MRESULT mrRet = RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[SPT] %s enter, pvSptHdl: %p\r\n"),
		DMX_FUNC_NAME, pvSptHdl);

	if (NULL == pvSptHdl) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args, pvSptHdl: %p\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterIsEnable(pvSptHdl)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for Splitter is disable, pvSptHdl: %p\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	SPLITTER_LOCK;
	if (!SplitterIsTaskRunning(pvSptHdl)) {
		DMXLOG_TRACE(
			TEXT("[SPT] %s exit for pvSptHdl(0x%p)'s task thread has already exited.\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		if (g_rDmxCliMan.fgDumpFlow) {
			DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

			mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
			rOperInfo.pvSptHdl = pvSptHdl;
			DmxDumpFlow(DMX_OPER_PSR_PAUSE, &rOperInfo);
		}
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_OK);
	}

	SPLITTER_UNLOCK;

//#if DMX_NEW_PBBUF_MECHANISM
	/* Release All PBBUF.*/
	if (g_u4PbBufFlag) {
  	mrRet = PBBUF_CancelAllocSlot(pvSptHdl);
  	if (DMX_FAILED(mrRet)) {
  		DMXLOG_ERROR(
  			TEXT("[SPT] %s line %d fail in PBBUF_CancelAllocSlot, pvSptHdl: %p, ")
  			TEXT("mrRet: 0x%x\r\n"),
  			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		}
	}
//#endif

	if (NULL != GetStreamByType(pvSptHdl, SPT_DATA_A))
		Spt4CfaTriggleAUCmdRelease(pvSptHdl, SPT_DATA_A);

	if (NULL != GetStreamByType(pvSptHdl, SPT_DATA_V))
		Spt4CfaTriggleAUCmdRelease(pvSptHdl, SPT_DATA_V);

	mm_memset(&rCmd, 0, sizeof(rCmd));

	rCmd.eCmd = DMX_CMD_PTX_PAUSE;
	rCmd.fgASync = FALSE;
	rCmd.u4UsrEvts = SPLITTER_UEV_PTX_PAUSE;
	rCmd.u4WaitTime = DMX_PSR_WAIT_OFF_MAXTIME;

	mrRet = SplitterSendCmd(pvSptHdl, &rCmd);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail in SplitterSendCmd(DMX_SPT_")
			TEXT("CMD_PAUSE), pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (DMX_FAILED(rCmd.mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail in exceed cmd DMX_SPT_CMD_PAUSE")
			TEXT(", pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, rCmd.mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = pvSptHdl;
		DmxDumpFlow(DMX_OPER_PSR_PAUSE, &rOperInfo);
	}

	DMXLOG_DEBUG(TEXT("[SPT] %s success, exit (pvSptHdl: %p, State:0x%x)\r\n"),
		DMX_FUNC_NAME, pvSptHdl, SplitterGetState(pvSptHdl));

	MM_RETURN(RET_DMX_OK);
}

/*//////////////////////////////////////////////////////////////*/
/* SplitterGetFileOfst*/
/* Get current parsing file offset*/
/*//////////////////////////////////////////////////////////////*/
MRESULT SplitterGetFileOfst(void *pvSptHdl, u64 *pu8FileOfst)
{
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvSptHdl) ||
		(NULL == pu8FileOfst)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args (pvSptHdl: %p,")
			TEXT(" pu8FileOfst: 0x%x)\r\n"),
			DMX_FUNC_NAME, pvSptHdl, pu8FileOfst);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterIsEnable(pvSptHdl)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for Splitter is disable, pvSptHdl: %p\r\n"),
			DMX_FUNC_NAME, pvSptHdl);

		MM_RETURN(RET_DMX_ERR_STATE);
	}

	SPLITTER_LOCK;
	mrRet = PSR_CC_GetCurrentOffset(((DMX_SPT_INST_T *)pvSptHdl)->pvPsrCC, pu8FileOfst);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s failed in PSR_CC_GetCurrentOffset(pvSptHdl: %p)\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		SPLITTER_UNLOCK;
		MM_RETURN(mrRet);
	}
	SPLITTER_UNLOCK;

	MM_RETURN(mrRet);
}

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* SplitterSetParserRspOff*/
/* Turn Off Resplitter logging proccess, and tell MW whether support resplitter*/
/*@Param ucRspTxType		[IN]  Indicate whether Resplitter has done Rebuf function before turn on resplitter*/
/*@Param ucRspTxUid		[IN]  Indicate whether Respllitter doesn't need to record Sp*/
/*@Param ucRspMode		[IN]	Resplitter Mode(0: Pts, 1: offset, 2: index) - MW only set it to be 0*/
/*@Param fgCurPbPause	[IN]  Indicate whether Respllitter doesn't need to record Sp*/
/*@Param ucRspTxRet		[OUT]  Indicate whether splitter support Resplitter or not*/
/*@Param ucState			[IN]  Indicate whether Respllitter doesn't need to record Sp*/
/*//////////////////////////////////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterSetParserRspOff(
	  void *pvSptHdl,
	  u8  ucRspTxType,
	  u8  ucRspMode,
	  u8 *pu1RspTxRet,
	  u8  ucState)
{
	DMX_CMD_INFO_T	rCmd;
	DMX_SPT_RSPOFF_CMDINBUF_T rRspOffInBuf;
	DMX_SPT_INST_T *prSpt	  = NULL;
	DMX_INST_T *prDmxInst = NULL;
	PSR_CC		   *prPsrCC    = NULL;
	u64		   u8CurPbbufStartOffset = 0;
	u32		   i		  = 0;
	u32		   u4StmType  = STREAM_NONE;
	void		*hTmpStm	  = NULL;
	u32		   u4WaitMaxCnt = 0;
	u8		   u1RspTxRet	= 0;
	bool		   fgFifoFull	= FALSE;
	MRESULT			mrRet		= RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[SPT] ++++++++++++ %s enter +++++++++++++\r\n"),
		DMX_FUNC_NAME);

	prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	if (NULL == pu1RspTxRet) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args, pu1RspTxRet == NULL\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (NULL == prSpt) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args, (pvSptHdl: %p)\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		*pu1RspTxRet = 0;  /* unsupport resplitter*/
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prDmxInst = (DMX_INST_T *)prSpt->pvDmxInst;

	if (NULL == prDmxInst) {
		DMX_ASSERT(0);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	
	prPsrCC = (PSR_CC *)(prSpt->pvPsrCC);
	if (NULL == prPsrCC) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for Splitter(0x%p) has no PSR_CC\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		*pu1RspTxRet = 0;  /* unsupport resplitter*/
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterIsEnable(pvSptHdl)) {
		DMXLOG_TRACE(
			TEXT("[SPT] %s exit for Splitter is disable (pvSptHdl: %p)\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		*pu1RspTxRet = 0; /* unsupport resplitter*/
		MM_RETURN(RET_DMX_OK);
	}

	SPLITTER_LOCK;

	if (!SplitterIsTaskRunning(pvSptHdl)) {
		DMXLOG_TRACE(
			TEXT("[SPT] %s exit for pvSptHdl(0x%x)'s task thread has already exited.\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		*pu1RspTxRet = 0; /* unsupport resplitter*/
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_OK);
	}

	if (!SplitterRspIsEnabled(prSpt)) {
		*pu1RspTxRet = 0; /* support resplitter*/
		DMXLOG_TRACE(
			TEXT("[SPT] %s line %d -- RSP Disabled, So don't support to do ")
			TEXT("resplitter ========!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_OK);
	}

	*pu1RspTxRet = 1; /* support resplitter*/

#if !DMX_SUPPORT_RSP_IN_FFRW
	if (MM_IS_FFRW_PLAY(SplitterGetPlayRate(pvSptHdl))) {
		/* don't support resplitter SP in fast forward/rewind*/
		DMXLOG_TRACE(TEXT("[SPT] %s exit for pvSptHdl(0x%x) don't support")
			TEXT(" to switch SP/Audio in Fast Forward/Rewind.\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		*pu1RspTxRet = 0; /* unsupport resplitter*/
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_OK);
	}
#endif /* !DMX_SUPPORT_RSP_IN_FFRW*/

	switch (ucRspTxType) {
	case (u8)SPT_DATA_SP: {
			if (NULL == GetStreamByType(pvSptHdl, SPT_DATA_SP)) {
				DMXLOG_TRACE(TEXT("[SPT] %s exit, unsupport to switch SP,")
					TEXT(" because that the Spt(0x%p) inst has no SP stream.\r\n"),
					DMX_FUNC_NAME, pvSptHdl);
				*pu1RspTxRet = 0; /* unsupport resplitter*/
				SPLITTER_UNLOCK;
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (MM_IS_FFRW_PLAY(SplitterGetPlayRate(pvSptHdl))) {
				/* don't support switch sp while video & sp are transferred by the same splitter instance*/
				if (NULL != GetStreamByType(pvSptHdl, SPT_DATA_V)) {
					DMXLOG_TRACE(TEXT("[SPT] %s exit, unsupport to switch")
						TEXT(" SP while Video & SP are transferred by the same splitter")
						TEXT("(pvSptHdl: %p) inst in FF/RW.\r\n"),
						DMX_FUNC_NAME, pvSptHdl);
					*pu1RspTxRet = 0; /* unsupport resplitter*/
					SPLITTER_UNLOCK;
					MM_RETURN(RET_DMX_OK);
				}
			}
		}
		break;
	case (u8)SPT_DATA_A: {
			void *hAStm = NULL;

			hAStm = GetStreamByType(pvSptHdl, SPT_DATA_A);
			if (NULL == hAStm) {
				DMXLOG_TRACE(TEXT("[SPT] %s exit, unsupport to switch audio")
					TEXT(", because that the Spt(0x%p) inst has no audio stream.\r\n"),
					DMX_FUNC_NAME, pvSptHdl);
				*pu1RspTxRet = 0; /* unsupport resplitter*/
				SPLITTER_UNLOCK;
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (MM_IS_FFRW_PLAY(SplitterGetPlayRate(pvSptHdl))) {
				/* Audio virtual switch if the file has video stream in FF/RW(Fast Forward/Rewind)*/
				if (NULL != GetStreamByTypes(prSpt->pvDmxInst, SPT_DATA_V)) {
					DMXLOG_TRACE(TEXT("[SPT] %s -- To Do Switch Audio in ")
						TEXT("fast forward or rewind, pvSptHdl: %p.\r\n"),
						DMX_FUNC_NAME, pvSptHdl);
					SPLITTER_UNLOCK;
					mrRet = StreamDisable(hAStm);
					if (DMX_FAILED(mrRet)) {
						DMXLOG_ERROR(TEXT("[SPT] %s exit, Can't to switch ")
							TEXT("Audio, Because StreamDisable(Audio) fail, pvSptHdl: %p.\r\n"),
							DMX_FUNC_NAME, pvSptHdl);
						*pu1RspTxRet = 0; /* unsupport resplitter*/
						MM_RETURN(RET_DMX_OK);
					}

					SPLITTER_LOCK;
					SplitterSetRspTxType(pvSptHdl, SPT_DATA_A);
					SplitterSetRspMode(pvSptHdl, ucRspMode);
					*pu1RspTxRet = 1; /* support resplitter*/
					SPLITTER_UNLOCK;
					MM_RETURN(RET_DMX_OK);
				}
			}

#if DMX_RSP_SUPPORT_PURE_AUDIO
			if (MM_IS_FFRW_PLAY(SplitterGetPlayRate(pvSptHdl))) {
				if (NULL == GetStreamByTypes(prSpt->pvDmxInst, SPT_DATA_V)) {
					*pu1RspTxRet = 0; /* unsupport resplitter*/
					DMXLOG_TRACE(TEXT("[SPT] %s exit, unsupport to do switch")
						TEXT(" audio for Pure Audio file in Fast Rewind\n"),
						DMX_FUNC_NAME);
					SPLITTER_UNLOCK;
					MM_RETURN(RET_DMX_OK);
				}
			}
#else  /* DMX_RSP_SUPPORT_PURE_AUDIO*/
			/* Pure audio file doesn't support resplitter audio*/
			if (NULL == GetStreamByTypes(prSpt->pvDmxInst, SPT_DATA_V)) {
				*pu1RspTxRet = 0; /* unsupport resplitter*/
				DMXLOG_ERROR(TEXT("[SPT] %s exit, unsupport to do switch")
					TEXT(" audio for Pure Audio file in Fast Forward/Rewind\n"),
					DMX_FUNC_NAME);
				SPLITTER_UNLOCK;
				MM_RETURN(RET_DMX_OK);
			}
#endif /* DMX_RSP_SUPPORT_PURE_AUDIO*/
		}
		break;
	default:
		*pu1RspTxRet = 0; /* unsupport resplitter*/
		DMXLOG_ERROR(TEXT("[SPT] %s exit, unsupport to do switch")
			TEXT("other type(%u) streams\r\n"),
			DMX_FUNC_NAME, ucRspTxType);
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_OK);
	}

	if ((ucRspTxType == SplitterGetRspTxType(pvSptHdl)) &&
		(ucRspMode == SplitterGetRspMode(pvSptHdl))) {
		SplitterSetReResplitter(pvSptHdl, TRUE);
	} else{
		SPLITTER_UNLOCK;
		/*it can not do rsp when pre rsp can not end, so wait unit pre rsp end.*/
		/* I think that MSDK should create one msg queue, each time only handle the last message*/
		/* to decreate the do-resplitter times in dmx*/
		u4WaitMaxCnt = 0;

		while ((SplitterRspIsEnabled(prSpt)) &&
			   (SplitterRspIsRsping(prSpt)) &&
			   (u4WaitMaxCnt < DMX_RSP_WAIT_FINISH_MAXTIME)) {
		#ifdef __linux__
			DMX_THREAD_DELAY(DMX_RSP_WAIT_FINISH_MINTIME);
		#else
			Sleep(DMX_RSP_WAIT_FINISH_MINTIME);
		#endif /* #ifdef __linux__*/
			u4WaitMaxCnt = u4WaitMaxCnt + DMX_RSP_WAIT_FINISH_MINTIME;
		}

		if (u4WaitMaxCnt >= DMX_RSP_WAIT_FINISH_MAXTIME) {
			DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in wait Last Resplitter")
				TEXT(" Complete(pvSptHdl: %p, State: 0x%x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, SplitterGetState(pvSptHdl));
			*pu1RspTxRet = 0; /* unsupport resplitter*/
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_OK);
		}

		SPLITTER_LOCK;
		SplitterSetReResplitter(pvSptHdl, FALSE);
	}

	for (i = 0; i < prSpt->u4StmHandleNs; i++) {
		/* Only check the enabled filters*/
		hTmpStm = prSpt->pvStmHandles[i];
		if (!StreamIsEnabled(hTmpStm))
			continue;

		/* It must be the common V/A/SP stream*/
		u4StmType = StreamGetStreamType(hTmpStm);
		if ((STREAM_DMA == u4StmType)	||
			(STREAM_INITIAL == u4StmType) ||
			(STREAM_NONE == u4StmType)) {
			continue;
		}

		/* check whether the parser filter 's ESM fifo is full(free space < Fifo reserved size) or */
		/*AU table is full*/
		mrRet = PSR_Filter_IsFifoFull(((DMX_STM_INST_T *)hTmpStm)->pvPsrFtr,
			&fgFifoFull, SplitterGetPtxLen(prSpt));
		if (DMX_FAILED(mrRet)) {
			fgFifoFull = FALSE;
			break;
		}

		/* Once any one fifo is full, notify!*/
		if (fgFifoFull) {
			GAU_DisableThreshold();
			break;
		}
	}

	/* If not badinterleave, the v, a, sp, section stream are dma one by one,*/
	/* If the mw is in pause state, it doesn't get any au,*/
	/* So if the fifo full, we can't dma any sp/aud data into its fifo,*/
	/* we will tell mw not support resplitter now.*/
	if ((DMX_MW_PAUSE_STATE == ucState) &&
		(!prDmxInst->fgPsrOff) &&
		fgFifoFull) {
		if ((SPT_DATA_A == ucRspTxType) &&
			(NULL != GetStreamByType(pvSptHdl, SPT_DATA_V))) {
			DMXLOG_TRACE(TEXT("[SPT] %s line %d -- Unsupport RSP for")
				TEXT(" MW's state is PAUSE and fifo not ready!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			SplitterSetReResplitter(pvSptHdl, FALSE);
			*pu1RspTxRet = 0;
			SPLITTER_UNLOCK;
			MM_RETURN(RET_DMX_OK);
		} else if ((SPT_DATA_SP == ucRspTxType) &&
			(NULL != GetStreamByType(pvSptHdl, SPT_DATA_V))) {
			DMXLOG_TRACE(TEXT("[SPT] %s line %d -- Unsupport RSP for ")
				TEXT("MW's state is PAUSE and fifo not ready!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			SplitterSetReResplitter(pvSptHdl, FALSE);
			*pu1RspTxRet = 0;
			SPLITTER_UNLOCK;
			MM_RETURN(RET_DMX_OK);
		} 
        else {
            //do nothing
		}
	}

	/* If the MW is in pause state, and fgPsrOff is TRUE(PSROFF), the mw has*/
	/* called the demuxer to stop parsing, so we will tell mw not support*/
	/* resplitter now.*/
	if ((DMX_MW_PAUSE_STATE == ucState) &&
		(prDmxInst->fgPsrOff)) {
		DMXLOG_TRACE(TEXT("[SPT] %s line %d -- Unsupport RSP for MW's")
			TEXT(" state is PAUSE and Splitter has PsrOff!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		SplitterSetReResplitter(pvSptHdl, FALSE);
		*pu1RspTxRet = 0;
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_OK);
	}

	DMXLOG_TRACE(TEXT("[SPT] ---------->fgCfaPrsEnd: %d, SptState:")
		TEXT(" 0x%x, pvSptHdl: %p, MWState: %d\r\n"),
		(SplitterIsCfaPsrEnd(prSpt) ? 1 : 0), SplitterGetState(pvSptHdl), pvSptHdl, ucState);

	SPLITTER_UNLOCK;

	mm_memset(&rCmd, 0, sizeof(rCmd));
	mm_memset(&rRspOffInBuf, 0, sizeof(rRspOffInBuf));

	rRspOffInBuf.ucRspMode = ucRspMode;
	rRspOffInBuf.ucRspTxType = ucRspTxType;

	rCmd.eCmd	 = DMX_CMD_RSP_OFF;
	rCmd.pvInBuf	= &rRspOffInBuf;
	rCmd.u4InBufSz	= sizeof(rRspOffInBuf);
	rCmd.pvOutBuf	= &u1RspTxRet;
	rCmd.u4OutBufSz = sizeof(u8);
	rCmd.fgASync	= FALSE;
	rCmd.u4UsrEvts	= SPLITTER_UEV_RSP_OFF;
	rCmd.u4WaitTime = DMX_RSP_WAIT_MAXTIME;

	mrRet = SplitterSendCmd(pvSptHdl, &rCmd);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail in SplitterSendCmd(RSP_OFF),")
			TEXT(" pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
		DmxCliDumpInstsInfo(-1);
		DmxCliDumpFifoInfo(-1);
		SplitterSetReResplitter(pvSptHdl, FALSE);
		SplitterSetRspOffStart(pvSptHdl, FALSE);
		*pu1RspTxRet = 0;
		MM_RETURN(mrRet);
	}

	if (DMX_FAILED(rCmd.mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail in exceed cmd RSP_OFF, pvSptHdl:")
			TEXT(" 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, rCmd.mrRet);
		DmxCliDumpInstsInfo(-1);
		DmxCliDumpFifoInfo(-1);
		SplitterSetReResplitter(pvSptHdl, FALSE);
		SplitterSetRspOffStart(pvSptHdl, FALSE);
		*pu1RspTxRet = 0;
		MM_RETURN(mrRet);
	}

	*pu1RspTxRet = u1RspTxRet;

	if (0 == *pu1RspTxRet) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail for *pu1RspTxRet == 0, pvSptHdl: %p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		DmxCliDumpInstsInfo(-1);
		DmxCliDumpFifoInfo(-1);
		SplitterSetReResplitter(pvSptHdl, FALSE);
		SplitterSetRspOffStart(pvSptHdl, FALSE);
		*pu1RspTxRet = 0;
		MM_RETURN(mrRet);
	}

	SPLITTER_LOCK;

	/* We need to log offset sa, due to after parser disable, this information can not be get */
	mrRet = PSR_CC_GetCurPbbufStartOffset(prSpt->pvPsrCC, &u8CurPbbufStartOffset);
	if (DMX_FAILED(mrRet)) {
		SplitterSetReResplitter(pvSptHdl, FALSE);
		SplitterSetRspOffStart(pvSptHdl, FALSE);
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in PSR_CC_GetCurPbbuf")
			TEXT("StartOffset, pvSptHdl: %p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		*pu1RspTxRet = 0;
		SPLITTER_UNLOCK;
		MM_RETURN(mrRet);
	}

	#ifdef __linux__
	DMXLOG_TRACE(TEXT("[SPT] %s line %d --- pvSptHdl: %p, SplitterSet")
		TEXT("PBBOffsetSa(CurPbbufStartOfst: %lld)\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, u8CurPbbufStartOffset);
	#else
	DMXLOG_TRACE(TEXT("[SPT] %s line %d --- pvSptHdl: %p, SplitterSet")
		TEXT("PBBOffsetSa(CurPbbufStartOfst: %I64d)\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, u8CurPbbufStartOffset);
	#endif /* #ifdef __linux__*/

	mrRet = SplitterSetPBBOffsetSa(pvSptHdl, u8CurPbbufStartOffset);
	if (DMX_FAILED(mrRet)) {
		SplitterSetReResplitter(pvSptHdl, FALSE);
		SplitterSetRspOffStart(pvSptHdl, FALSE);
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in SplitterSetPBBOffsetSa, pvSptHdl: %p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		*pu1RspTxRet = 0;
		SPLITTER_UNLOCK;
		MM_RETURN(mrRet);
	}

	/* Set Resplitter Tx Stream Type*/
	SplitterSetRspTxType(pvSptHdl, ucRspTxType);

	SplitterSetRspMode(pvSptHdl, ucRspMode);

	SPLITTER_UNLOCK;

	DMXLOG_TRACE(TEXT("[SPT] ++++++++++++ %s success, exit (pvSptHdl: %p,")
		TEXT(" State:0x%x, ReRsp: %s) +++++++++++++\r\n"),
		DMX_FUNC_NAME, pvSptHdl, SplitterGetState(pvSptHdl),
		(SplitterIsReResplitter(pvSptHdl) ? "TRUE" : "FALSE"));

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterGetRebufferRange*/
/* Condition: Splitter 's State is IDLE or TX_RSPOFF*/
/* Get the Rsplitter Start FileOffset by the PTS(u8RspStartPts + u8RspDelta) and Pbbuf's current*/
/*start file offset*/
/* Obtained whether need to do Rebuffer*/
/* @Param u8RspDelta			[IN] Rsp Delta Pts*/
/* @Param u8RspStartPts		[IN]	Rsp Start Pts*/
/* @Param pu8RspStartOffset   [OUT]    do Resplitter's start fileoffset which is obtained by the */
/* PTS(u8RspStartPts + u8RspDelta)*/
/* @Param pu8PbbStartOffset   [OUT]    Current Pbbuf's Start fileoffset*/
/* @Param fgRebuff				   [OUT]	Indicates whether need to do Rebuf for Resplitter*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterGetRebufferRange(
	void *pvSptHdl,
	u64 u8RspDelta,
	u64 u8RspStartPts,
	u64 *pu8RspStartOffset,
	u64 *pu8PbbStartOffset,
	bool   *fgRebuff)
{
	DMX_CMD_INFO_T	rCmd;
	DMX_SPT_GETREBUFRANGE_INBUF_T  rCmdInBuf;
	DMX_SPT_GETREBUFRANGE_OUTBUF_T rCmdOutBuf;
	DMX_SPT_INST_T *prSpt	  = NULL;
	MRESULT mrRet = RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[SPT] ++++++++++++ %s enter +++++++++++++\r\n"),
		DMX_FUNC_NAME);

	prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	if ((NULL == prSpt) ||
		(NULL == pu8RspStartOffset) ||
		(NULL == pu8PbbStartOffset) ||
		(NULL == fgRebuff)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args (pvSptHdl: %p)\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterIsEnable(pvSptHdl)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for Splitter(pvSptHdl: %p) is disable\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	SPLITTER_LOCK;

	if (!SplitterRspIsEnabled(pvSptHdl)) {

		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for Resplitter is disabled, (pvSptHdl: %p)\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	if ((SPT_DATA_A != SplitterGetRspTxType(pvSptHdl)) &&
		(SPT_DATA_SP != SplitterGetRspTxType(pvSptHdl))) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s -- unsupport to switch, SplitterGetRspTxType(pvSptHdl) = ")
			TEXT("0x%x (not Audio/SP), pvSptHdl: %p .\r\n"),
			DMX_FUNC_NAME, SplitterGetRspTxType(pvSptHdl), pvSptHdl);
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_UNEXPECT);
	}

#if DMX_SUPPORT_RSP_IN_FFRW
	if (SPT_DATA_SP == SplitterGetRspTxType(pvSptHdl)) {
		if (MM_IS_FFRW_PLAY(SplitterGetPlayRate(pvSptHdl))) {
			/* don't support resplitter sp while video & sp are transferred by the same splitter instance*/
			if (NULL != GetStreamByType(pvSptHdl, SPT_DATA_V)) {
				DMXLOG_ERROR(
					TEXT("[SPT] %s fail for unsupport to switch SP while Video & ")
					TEXT("SP are transferred in the Same Spt(0x%p) inst in FF/RW.\r\n"),
					DMX_FUNC_NAME, pvSptHdl);
				SPLITTER_UNLOCK;
				MM_RETURN(RET_DMX_UNEXPECT);
			} else {
				DMXLOG_ERROR(TEXT("[SPT] %s fail for Separated SP insts")
					TEXT(" should do stop->start to do switch process, pvSptHdl(0x%p).\r\n"),
					DMX_FUNC_NAME, pvSptHdl);
				SPLITTER_UNLOCK;
				MM_RETURN(RET_DMX_UNEXPECT);
			}
		}
	} else if (SPT_DATA_A == SplitterGetRspTxType(pvSptHdl)) {
#if DMX_RSP_SUPPORT_PURE_AUDIO
		if (MM_IS_FFRW_PLAY(SplitterGetPlayRate(pvSptHdl))) {
			if (NULL == GetStreamByTypes(prSpt->pvDmxInst, SPT_DATA_V)) {
				DMXLOG_ERROR(TEXT("[SPT] %s fail for unsupport to switch")
					TEXT(" Audio for Pure Audio file in Fast Rewind, pvSptHdl(0x%p)\n"),
					DMX_FUNC_NAME, pvSptHdl);
				SPLITTER_UNLOCK;
				MM_RETURN(RET_DMX_UNEXPECT);
			} else { /* Virtual Switch Audio*/
				*fgRebuff = FALSE;
				DMXLOG_TRACE(TEXT("[SPT] %s exit, virtual switch Audio")
					TEXT(" in Fast Forward/Rewind, pvSptHdl(0x%p)\n"),
					DMX_FUNC_NAME, pvSptHdl);
				SPLITTER_UNLOCK;
				MM_RETURN(RET_DMX_OK);
			}
		}
#else
		/* Pure audio file doesn't support resplitter audio*/
		if (NULL == GetStreamByTypes(prSpt->pvDmxInst, SPT_DATA_V)) {
			DMXLOG_ERROR(TEXT("[SPT] %s fail for unsupport to switch")
				TEXT(" Audio for Pure Audio file in Fast Forward/Rewind, pvSptHdl(0x%p)\n"),
				DMX_FUNC_NAME, pvSptHdl);
			SPLITTER_UNLOCK;
			MM_RETURN(RET_DMX_UNEXPECT);
		} else if (MM_IS_FFRW_PLAY(SplitterGetPlayRate(pvSptHdl))) { /* Virtual Switch Audio*/
			*fgRebuff = FALSE;
			DMXLOG_TRACE(TEXT("[SPT] %s exit, virtual switch Audio in")
				TEXT(" Fast Forward/Rewind, pvSptHdl(0x%p)\n"),
				DMX_FUNC_NAME, pvSptHdl);
			SPLITTER_UNLOCK;
			MM_RETURN(RET_DMX_OK);
		} else {
		    //do nothing
		}
#endif /* DMX_RSP_SUPPORT_PURE_AUDIO*/
	} else {
	    //do nothing
	}
#else /* DMX_SUPPORT_RSP_IN_FFRW*/

	/* FF/RW(Fast Forward/Rewind) don't support rsplitter*/
	if (MM_IS_FFRW_PLAY(SplitterGetPlayRate(pvSptHdl))) {
		DMXLOG_TRACE(TEXT("[SPT] %s fail for unsupport to switch ")
			TEXT("SP/Audio in Fast Forward/Rewind, pvSptHdl(0x%p)\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_UNEXPECT);
	}

#endif /* DMX_SUPPORT_RSP_IN_FFRW*/

	if (!SplitterIsTaskRunning(pvSptHdl)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for pvSptHdl(0x%p)'s task thread has already exited.\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	SPLITTER_UNLOCK;

	mm_memset(&rCmdInBuf, 0, sizeof(rCmdInBuf));

	mm_memset(&rCmdOutBuf, 0, sizeof(rCmdOutBuf));

	mm_memset(&rCmd, 0, sizeof(rCmd));

	rCmdInBuf.u8RspDelta = u8RspDelta;
	rCmdInBuf.u8RspStartPts = u8RspStartPts;
	rCmdInBuf.u8RspStartOffset = *pu8RspStartOffset;
	rCmdOutBuf.fgNeedRebuf = FALSE;

	rCmd.eCmd	 = DMX_CMD_RSP_REBUF;
	rCmd.pvInBuf	= &rCmdInBuf;
	rCmd.u4InBufSz	= sizeof(rCmdInBuf);
	rCmd.pvOutBuf	= &rCmdOutBuf;
	rCmd.u4OutBufSz = sizeof(rCmdOutBuf);
	rCmd.fgASync	= FALSE;
	rCmd.u4UsrEvts	= SPLITTER_UEV_RSP_REBUF;
	rCmd.u4WaitTime = DMX_RSP_WAIT_MAXTIME;

	mrRet = SplitterSendCmd(pvSptHdl, &rCmd);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in SplitterSendCmd(RSP_OFF), pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
		*fgRebuff = FALSE;
		MM_RETURN(mrRet);
	}

	if (DMX_FAILED(rCmd.mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in exceed cmd RSP_OFF, pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, rCmd.mrRet);
		*fgRebuff = FALSE;
		MM_RETURN(mrRet);
	}

	*fgRebuff = rCmdOutBuf.fgNeedRebuf;
	*pu8RspStartOffset = rCmdOutBuf.u8RspStartOffset;
	*pu8PbbStartOffset = rCmdOutBuf.u8PbbStartOffset;

	DMXLOG_TRACE(
		TEXT("[SPT] ++++++++++++ %s line %d success, exit +++++++++++++\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/* SplitterSetParserRspOn*/
/* Turn On Resplitter logging proccess*/
/*@Param fgRebuf		 [IN]  Indicate whether Resplitter has done Rebuf function before turn on resplitter*/
/*//////////////////////////////////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterSetParserRspOn(void *pvSptHdl, bool fgRebuf)
{
	DMX_CMD_INFO_T	rCmd;
	DMX_SPT_INST_T *prSpt	  = NULL;
	MRESULT mrRet = RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[SPT] ++++++++++++ %s enter +++++++++++++\r\n"),
		DMX_FUNC_NAME);

	prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	if (NULL == prSpt) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args, (pvSptHdl: %p, fgRebuf: %d)\r\n"),
			DMX_FUNC_NAME, pvSptHdl, (fgRebuf ? 1 : 0));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterIsEnable(pvSptHdl)) {
		/*i4Splitter2PsrSetRspVidThreshold(u4SptHandle, 0);*/
		DMXLOG_ERROR(
		TEXT("[SPT] %s fail for Splitter is disable (pvSptHdl: %p, fgRebuf: %d)\r\n"),
			DMX_FUNC_NAME, pvSptHdl, (fgRebuf ? 1 : 0));
		SplitterSetEOSForError(pvSptHdl, RET_DMX_UNEXPECT);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	SPLITTER_LOCK;

	if (!SplitterRspIsEnabled(pvSptHdl)) {

		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for Resplitter is disabled, (pvSptHdl: %p)\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	if ((SPT_DATA_A != SplitterGetRspTxType(pvSptHdl)) &&
		(SPT_DATA_SP != SplitterGetRspTxType(pvSptHdl))) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s -- unsupport to switch, SplitterGetRspTxType(pvSptHdl) = ")
			TEXT("0x%x (not Audio/SP), pvSptHdl: %p .\r\n"),
			DMX_FUNC_NAME, SplitterGetRspTxType(pvSptHdl), pvSptHdl);
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_UNEXPECT);
	}

#if DMX_SUPPORT_RSP_IN_FFRW
	if (SPT_DATA_SP == SplitterGetRspTxType(pvSptHdl)) {
		if (MM_IS_FFRW_PLAY(SplitterGetPlayRate(pvSptHdl))) {
			/* don't support resplitter sp while video & sp are transferred by the same splitter instance*/
			if (NULL != GetStreamByType(pvSptHdl, SPT_DATA_V)) {
				DMXLOG_ERROR(
					TEXT("[SPT] %s fail for unsupport to switch SP while Video &")
					TEXT(" SP are transferred in the same spt inst in FF/RW.\r\n"),
					DMX_FUNC_NAME);
				SPLITTER_UNLOCK;
				MM_RETURN(RET_DMX_UNEXPECT);
			} else {
				DMXLOG_ERROR(
					TEXT("[SPT] %s fail for separated SP insts should do stop->start")
					TEXT(" to do switch process.\r\n"),
					DMX_FUNC_NAME);
				SPLITTER_UNLOCK;
				MM_RETURN(RET_DMX_UNEXPECT);
			}
		}
	} else if (SPT_DATA_A == SplitterGetRspTxType(pvSptHdl)) {
#if DMX_RSP_SUPPORT_PURE_AUDIO
		if (MM_IS_FFRW_PLAY(SplitterGetPlayRate(pvSptHdl))) {
			if (NULL == GetStreamByTypes(prSpt->pvDmxInst, SPT_DATA_V)) {
				DMXLOG_ERROR(
					TEXT("[SPT] %s fail for unsupport to switch Audio for Pure ")
					TEXT("Audio file in Fast Rewind\n"),
					DMX_FUNC_NAME);
				SPLITTER_UNLOCK;
				MM_RETURN(RET_DMX_UNEXPECT);
			} else {/* Virtual Switch Audio*/
				DMXLOG_TRACE(TEXT("[SPT] %s exit, virtual switch Audio")
					TEXT(" in Fast Forward/Rewind, pvSptHdl(0x%x)\n"),
					DMX_FUNC_NAME, pvSptHdl);
				SPLITTER_UNLOCK;
				MM_RETURN(RET_DMX_OK);
			}
		}
#else  /* DMX_RSP_SUPPORT_PURE_AUDIO*/
		/* Pure audio file doesn't support resplitter audio*/
		if (NULL == GetStreamByTypes(prSpt->pvDmxInst, SPT_DATA_V)) {
			DMXLOG_ERROR(TEXT("[SPT] %s fail for unsupport to switch")
				TEXT(" Audio for Pure Audio file in Fast Forward/Rewind\n"),
				DMX_FUNC_NAME);
			SPLITTER_UNLOCK;
			MM_RETURN(RET_DMX_UNEXPECT);
		} else if (MM_IS_FFRW_PLAY(SplitterGetPlayRate(pvSptHdl))) {
			/* Virtual Switch Audio*/
			DMXLOG_TRACE(TEXT("[SPT] %s exit, virtual switch Audio in")
				TEXT(" Fast Forward/Rewind, pvSptHdl(0x%x)\n"),
				DMX_FUNC_NAME, pvSptHdl);
			SPLITTER_UNLOCK;
			MM_RETURN(RET_DMX_OK);
		} else {
		    //do nothing
		}
#endif /* DMX_RSP_SUPPORT_PURE_AUDIO*/
	} else {
	    //do nothing
	}

#else /* DMX_SUPPORT_RSP_IN_FFRW*/

	/* FF/RW(Fast Forward/Rewind) don't support rsplitter*/
	if (MM_IS_FFRW_PLAY(SplitterGetPlayRate(pvSptHdl))) {
		DMXLOG_TRACE(TEXT("[SPT] %s fail for unsupport to switch SP/Audio")
			TEXT(" in Fast Forward/Rewind\r\n"),
			DMX_FUNC_NAME);
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_UNEXPECT);
	}

#endif /* DMX_SUPPORT_RSP_IN_FFRW*/

	if (!SplitterIsTaskRunning(pvSptHdl)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for pvSptHdl(0x%x)'s task thread has already exited.\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	SPLITTER_UNLOCK;

	mm_memset(&rCmd, 0, sizeof(rCmd));

	rCmd.eCmd	 = DMX_CMD_RSP_ON;
	rCmd.pvInBuf	= &fgRebuf;
	rCmd.u4InBufSz	= sizeof(bool);
	rCmd.fgASync	= FALSE;
	rCmd.u4UsrEvts	= SPLITTER_UEV_RSP_ON;
	rCmd.u4WaitTime = DMX_RSP_WAIT_MAXTIME;

	mrRet = SplitterSendCmd(pvSptHdl, &rCmd);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in SplitterSendCmd(RSP_OFF), pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	if (DMX_FAILED(rCmd.mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in exceed cmd RSP_OFF, pvSptHdl: %p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, rCmd.mrRet);
		MM_RETURN(rCmd.mrRet);
	}

	DMXLOG_TRACE(
		TEXT("[SPT] ++++++++++++ %s success, exit (pvSptHdl: %p, State:0x%x) +++++++++++++\r\n"),
		DMX_FUNC_NAME, pvSptHdl, SplitterGetState(pvSptHdl));

	MM_RETURN(RET_DMX_OK);
}

MRESULT SplitterGetStmEmptyInfo(PSR_FILTER *prPsrFilter, bool *pfgEmpty)
{
	u32	   u4FifoAvailSize	= 0;
	u32	   u4FifoSelfASize	= 0;
	u32	   u4FifoSize		= 0;
	u32	   u4TotalCnt		= 0;
	u32	   u4AvailCnt		= 0;
	DMX_FIFO_INFO_T   *pFifo	= NULL;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prPsrFilter) ||
		(NULL == pfgEmpty)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	SPLITTER_LOCK;

	mrRet = ESM_FifoGetInfo(prPsrFilter->u4ESIH, (void **)&pFifo);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in %s ESM_FifoGetInfo(ESIH: 0x%x), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			((prPsrFilter->eType < MAX_SPT_DATA_TYPE_CNT) ?
			g_aszSptDataTypeName[prPsrFilter->eType] : TEXT("UNKNOWN")),
			prPsrFilter->u4ESIH, mrRet);
		SPLITTER_UNLOCK;
		MM_RETURN(mrRet);
	}

	if (NULL == pFifo) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail for %s fifo info is NULL(ESIH: 0x%x), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			((prPsrFilter->eType < MAX_SPT_DATA_TYPE_CNT) ?
			g_aszSptDataTypeName[prPsrFilter->eType] : TEXT("UNKNOWN")),
			prPsrFilter->u4ESIH, RET_DMX_UNEXPECT);
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	mrRet = ESM_AUTableGetTotalCount(prPsrFilter->u4ESIH, &u4TotalCnt);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in %s ESM_AUTableGetTotalCount(ESIH: 0x%x),")
			TEXT(" mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			((prPsrFilter->eType < MAX_SPT_DATA_TYPE_CNT) ?
			g_aszSptDataTypeName[prPsrFilter->eType] : TEXT("UNKNOWN")),
			prPsrFilter->u4ESIH, mrRet);
		SPLITTER_UNLOCK;
		MM_RETURN(mrRet);
	}

	mrRet = ESM_AUTableGetAvailCount(prPsrFilter->u4ESIH, &u4AvailCnt);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in %s ESM_AUTableGet")
			TEXT("AvailCount(ESIH: 0x%x), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			((prPsrFilter->eType < MAX_SPT_DATA_TYPE_CNT) ?
			g_aszSptDataTypeName[prPsrFilter->eType] : TEXT("UNKNOWN")),
			prPsrFilter->u4ESIH, mrRet);
		SPLITTER_UNLOCK;
		MM_RETURN(mrRet);
	}

	mrRet = ESM_FifoGetAvailDataSize(prPsrFilter->u4ESIH, &u4FifoAvailSize);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in %s ESM_FifoGet")
			TEXT("AvailDataSize(ESIH: 0x%x), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			((prPsrFilter->eType < MAX_SPT_DATA_TYPE_CNT) ?
			g_aszSptDataTypeName[prPsrFilter->eType] : TEXT("UNKNOWN")),
			prPsrFilter->u4ESIH, mrRet);
		SPLITTER_UNLOCK;
		MM_RETURN(mrRet);
	}

	u4FifoSize = pFifo->ptrEa - pFifo->ptrSa;

	if (SPT_DATA_A == prPsrFilter->eType) {
		mrRet = ESM_FifoGetSelfAvailDataSize(prPsrFilter->u4ESIH, &u4FifoSelfASize);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in %s ESM_FifoGet")
				TEXT("SelfAvailDataSize(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				((prPsrFilter->eType < MAX_SPT_DATA_TYPE_CNT) ?
				g_aszSptDataTypeName[prPsrFilter->eType] : TEXT("UNKNOWN")),
				prPsrFilter->u4ESIH, mrRet);
			SPLITTER_UNLOCK;
			MM_RETURN(mrRet);
		}

		DMXLOG_ERROR(
			TEXT("[SPT] %s ----------> Fifo Total Size: %d, Fifo Data Size: %d\r\n"),
			((prPsrFilter->eType < MAX_SPT_DATA_TYPE_CNT) ?
			g_aszSptDataTypeName[prPsrFilter->eType] : TEXT("UNKNOWN")),
			u4FifoSize, u4FifoAvailSize);
	} else{
		DMXLOG_ERROR(
			TEXT("[SPT] %s ----------> Fifo Total Size: %d, Fifo Data Size: %d, ")
			TEXT("Self Fifo Data Size: %d\r\n"),
			((prPsrFilter->eType < MAX_SPT_DATA_TYPE_CNT) ?
			g_aszSptDataTypeName[prPsrFilter->eType] : TEXT("UNKNOWN")),
			u4FifoSize, u4FifoAvailSize, u4FifoSelfASize);
	}

	DMXLOG_ERROR(
		TEXT("[SPT] %s ----------> Total AU Count: %d, Avail AU Count: %d\r\n"),
		((prPsrFilter->eType < MAX_SPT_DATA_TYPE_CNT) ?
		g_aszSptDataTypeName[prPsrFilter->eType] : TEXT("UNKNOWN")),
		u4TotalCnt, u4AvailCnt);

	*pfgEmpty = FALSE;

	if ((0 == u4AvailCnt) || (0 == u4FifoAvailSize)) {
		if (SPT_DATA_A == prPsrFilter->eType) {
			if (u4FifoAvailSize < DMX_AFIFO_EMP_DATA_MINSZ)
				*pfgEmpty = TRUE;
		} else {
			*pfgEmpty = TRUE;
		}
	} else{
		*pfgEmpty = FALSE;
	}
	SPLITTER_UNLOCK;

	MM_RETURN(RET_DMX_OK);
}

MRESULT Splitter_CheckFifo(
	SPT_PARAM_FIFO_USAGE * prFifoUsage,
	SPT_PARAM_FIFO_USAGE_OUTPUT *prOutInfo)
{
	DMX_STM_INST_T	  *prVStm		= NULL;
	DMX_STM_INST_T	  *prAStm		= NULL;
	DMX_STM_INST_T	  *prSctStm		= NULL;
	PSR_FILTER *prPsrFilter    = NULL;
	bool	   fgVideoEmpty    = FALSE;
	bool	   fgAudioEmpty    = FALSE;
	bool	   fgSectionEmpty  = FALSE;
	bool	   fgCCEmpty	   = FALSE;
	MRESULT    mrRet		   = RET_DMX_OK;

	if ((NULL == prFifoUsage) || (NULL == prOutInfo) || (NULL == prFifoUsage->pvSptHdl))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	SPLITTER_LOCK;

	prOutInfo->fgEmpty = FALSE;
	prOutInfo->fgThresholdEnabled = !(GAU_IsReachThreshold());

	if (prOutInfo->fgThresholdEnabled) {
		DMXLOG_TRACE(
			TEXT("[SPT] %s --> Not Reach Theshold, so we tell the MW no data\r\n"),
			DMX_FUNC_NAME);
		prOutInfo->fgEmpty = TRUE;
		SPLITTER_UNLOCK;
		MM_RETURN(RET_DMX_OK);
	} else{
		DMXLOG_TRACE(TEXT("[SPT] %s --> has been reached threshold\r\n"),
			DMX_FUNC_NAME);
	}

	/* Check Video*/
	fgVideoEmpty = TRUE;

	prVStm = GetStreamByType(prFifoUsage->pvSptHdl, SPT_DATA_V);

	if ((NULL != prVStm) &&
		(StreamIsEnabled(prVStm))) {
		prPsrFilter = (PSR_FILTER *)(prVStm->pvPsrFtr);

		mrRet = SplitterGetStmEmptyInfo(prPsrFilter, &fgVideoEmpty);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s fail in SplitterGetStmEmptyInfo(Video, ESIH: 0x%x),")
				TEXT(" mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, prPsrFilter->u4ESIH, mrRet);
		}
	}

	/* Check Audio*/
	fgAudioEmpty = TRUE;

	prAStm = GetStreamByType(prFifoUsage->pvSptHdl, SPT_DATA_A);

	if ((NULL != prAStm) &&
		(StreamIsEnabled(prAStm))) {
		prPsrFilter = (PSR_FILTER *)(prAStm->pvPsrFtr);

		mrRet = SplitterGetStmEmptyInfo(prPsrFilter, &fgAudioEmpty);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s fail in SplitterGetStmEmptyInfo")
				TEXT("(Audio, ESIH: 0x%x), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, prPsrFilter->u4ESIH, mrRet);
		}
	}

	/* Check Section*/
	fgSectionEmpty = TRUE;

	prSctStm = GetStreamByType(prFifoUsage->pvSptHdl, SPT_DATA_SECTION);
	if ((NULL != prSctStm) &&
		(StreamIsEnabled(prSctStm))) {
		prPsrFilter = (PSR_FILTER *)(prSctStm->pvPsrFtr);

		mrRet = SplitterGetStmEmptyInfo(prPsrFilter, &fgSectionEmpty);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s fail in SplitterGetStmEmptyInfo")
				TEXT("(Section, ESIH: 0x%x), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, prPsrFilter->u4ESIH, mrRet);
		}
	}

	/* Check CC*/
	fgCCEmpty = TRUE;

	prSctStm = GetStreamByType(prFifoUsage->pvSptHdl, SPT_DATA_SP);
	if ((NULL != prSctStm) &&
		(StreamIsEnabled(prSctStm))) {
		prPsrFilter = (PSR_FILTER *)(prSctStm->pvPsrFtr);

		mrRet = SplitterGetStmEmptyInfo(prPsrFilter, &fgCCEmpty);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s fail in SplitterGetStmEmptyInfo")
				TEXT("(CC, ESIH: 0x%x), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, prPsrFilter->u4ESIH, mrRet);
		}
	}

	if (fgVideoEmpty &&
		fgAudioEmpty &&
		fgSectionEmpty) {
		prOutInfo->fgEmpty = TRUE;

		DMXLOG_TRACE(TEXT("*** %s ENABLE THRESHOLD ***\r\n"),
			DMX_FUNC_NAME);
		/* If signal is 0, and all fifo are empty, make threshold to operate.*/
		mrRet = GAU_SetThreshold((u32)(prVStm->u4GAUHandle), prVStm->u4FifoThreshold);
		if ((NULL != prVStm) && (RET_DMX_OK != mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s fail in GAU_SetThreshold, strmtype: 0x%x\r\n"),
				DMX_FUNC_NAME, prVStm->u4StmType);
			SPLITTER_UNLOCK;
			MM_RETURN(mrRet);
		}
		if ((NULL != prAStm) && (RET_DMX_OK != mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s fail in GAU_SetThreshold, strmtype: 0x%x\r\n"),
				DMX_FUNC_NAME, prAStm->u4StmType);
			SPLITTER_UNLOCK;
			MM_RETURN(mrRet);
		}
		if ((NULL != prSctStm) && (RET_DMX_OK != mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s fail in GAU_SetThreshold, strmtype: 0x%x\r\n"),
				DMX_FUNC_NAME, prSctStm->u4StmType);
			SPLITTER_UNLOCK;
			MM_RETURN(mrRet);
		}
	}

	SPLITTER_UNLOCK;

	MM_RETURN(RET_DMX_OK);
}


MRESULT SplitterSetPowerState(DMX_PM_STATE PowerState)
{
	MRESULT mrRet = RET_DMX_OK;
	void	*pvSptHdl = 0;
	u32	u4Count = 0;
	EV_GRP_EVENT_T eEvts = 0;
	DMX_PM_STATE eCurPowerState = D0;

	DMXLOG_DEBUG(TEXT("[PVR] %s line %d enter -- to set Power State: %d\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, PowerState);

	u4Count = DmxGetInstCount();
	if (0 == u4Count) {
		if (VALID_DX(PowerState)) {
			SPLITTER_LOCK;
			eCurPowerState = PSR_HAL_GetPowerState();
			switch (PowerState) {
			case D0:				// Power Up
			case D1:
			case D2:
				// Resume (if D4 --> D0)
				if ((D3 == eCurPowerState) ||
					(D4 == eCurPowerState)) {
					g_fgDmxDmaTwice = TRUE;
					SPLITTER_UNLOCK;
					MMLOG_TRACE(LOG_MOD_DMX, TEXT("[SPT] %s exit -- no inuse dmx instance, dmx hw hasn't been power up, so don't need to set power up/down.\r\n"),
						DMX_FUNC_NAME);
					MM_RETURN(RET_DMX_OK);
				} else {
					SPLITTER_UNLOCK;
					MMLOG_ERROR(LOG_MOD_DMX, TEXT("[SPT] %s fail for invalid powerstate to set(%d)\r\n"),
						DMX_FUNC_NAME, PowerState);
					MM_RETURN(RET_DMX_OK);
				}
				break;
			case D3:
			case D4:		// Power Down
				if ((D0 == eCurPowerState) ||
					(D1 == eCurPowerState) ||
					(D2 == eCurPowerState)) {		
					g_fgDmxDmaTwice = FALSE;
					break;
				} else {
					  SPLITTER_UNLOCK;
					  MMLOG_ERROR(LOG_MOD_DMX, TEXT("[SPT] %s fail for invalid powerstate to set(%d)\r\n"),
						  DMX_FUNC_NAME, PowerState);
					  MM_RETURN(RET_DMX_OK);
				}
				break;
			default:
				break;
			}
			SPLITTER_UNLOCK;
		} else {
			MMLOG_ERROR(LOG_MOD_DMX, TEXT("[SPT] %s fail for invalid powerstate to set(%d)\r\n"),
				DMX_FUNC_NAME, PowerState);
			MM_RETURN(RET_DMX_OK);
		}
	} else {
		if (VALID_DX(PowerState)) {
			SPLITTER_LOCK;
			eCurPowerState = PSR_HAL_GetPowerState();
			switch (PowerState) {
			case D0:				// Power Up
			case D1:
			case D2:
				// Resume (if D4 --> D0)
				if ((D3 == eCurPowerState) ||
					(D4 == eCurPowerState)) {
					g_fgDmxDmaTwice = TRUE;
				} else {
					SPLITTER_UNLOCK;
					MMLOG_ERROR(LOG_MOD_DMX, TEXT("[SPT] %s fail for invalid powerstate to set(%d)\r\n"),
						DMX_FUNC_NAME, PowerState);
					MM_RETURN(RET_DMX_OK);
				}
				break;
			case D3:
			case D4:		// Power Down
				if ((D0 == eCurPowerState) ||
					(D1 == eCurPowerState) ||
					(D2 == eCurPowerState)) {		
					g_fgDmxDmaTwice = FALSE;
					break;
				} else {
					SPLITTER_UNLOCK;
					MMLOG_ERROR(LOG_MOD_DMX, TEXT("[SPT] %s fail for invalid powerstate to set(%d)\r\n"),
						DMX_FUNC_NAME, PowerState);
					MM_RETURN(RET_DMX_OK);
				}
				break;
			default:
				break;
			}
			SPLITTER_UNLOCK;
		} else {
			MMLOG_ERROR(LOG_MOD_DMX, TEXT("[SPT] %s fail for invalid powerstate to set(%d)\r\n"),
					DMX_FUNC_NAME, PowerState);
			MM_RETURN(RET_DMX_OK);
		}		
	}

	SPLITTER_LOCK;
	if (VALID_DX(PowerState)) {
		DMXLOG_TRACE(TEXT("[PVR] %s line %d -- ParserSetPowerState(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, PowerState);
		mrRet = ParserSetPowerState(PowerState, &pvSptHdl);
		if ((RET_DMX_SUSPEND_NEED_WAIT == mrRet) && (NULL != pvSptHdl)) {
			DMXLOG_TRACE(
				TEXT("[PVR] %s -- Wait Splitter(0x%x)'s Usr Event: SPLITTER_UEV_")
				TEXT("SUSPEND_OK\r\n"),
				DMX_FUNC_NAME, pvSptHdl);
			mrRet = SplitterGetUsrEvent(pvSptHdl, SPLITTER_UEV_SUSPEND_OK, &eEvts, -1);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT("[PVR] %s fail in Get Usr Event: SPLITTER_UEV_SUSPEND_OK\r\n"),
					DMX_FUNC_NAME);
				SPLITTER_UNLOCK;
				SplitterSetEOSForError(pvSptHdl, mrRet);
				MM_RETURN(mrRet);
			}
			DMXLOG_TRACE(
				TEXT("[PVR] %s -- Get Usr Event: SPLITTER_UEV_SUSPEND_OK\r\n"),
				DMX_FUNC_NAME);
		}
	} else{
		SPLITTER_UNLOCK;
		DMXLOG_ERROR(TEXT("[PVR] %s fail for invalid Power State\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	SPLITTER_UNLOCK;
	DMXLOG_TRACE(
		TEXT("[PVR] %s line %d success -- Current Power State: %d\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, PowerState);

	MM_RETURN(mrRet);
}
DMX_PM_STATE SplitterGetPowerState(void)
{
	DMX_PM_STATE eState;
	bool  fgInit = TRUE;

	SPLITTER_LOCK;
	if (!g_fgDmxInit) {
		fgInit = FALSE;
	}

	if (!fgInit) {
        MMLOG_TRACE(LOG_MOD_DMX, TEXT("[SPT] %s line %d exit, splitter hasn't been inited.\r\n"),
            DMX_FUNC_NAME, DMX_LINE_NO);
        eState = D4;
    } else {
		eState = ParserGetPowerState();
	}
	SPLITTER_UNLOCK;

	return eState;
}

MRESULT SplitterHandleCliCmd(DMX_CLI_CFG *prCliCfg)
{
	MRESULT mrRet = RET_DMX_OK;

	switch (prCliCfg->eDmxCliType) {
	case DMX_CLI_CMD_TURN_ONOFF_LOG:
		mrRet = DmxCliTurnOnOffLog(prCliCfg->u4arg1, prCliCfg->u4arg2,
		  prCliCfg->u4arg3, prCliCfg->u4arg4);
		break;
	case DMX_CLI_CMD_DUMP_FIFO_INFO:
		mrRet = DmxCliDumpFifoInfo(prCliCfg->u4arg1);
		break;
	case DMX_CLI_CMD_DUMP_PBBUF_INFO:
		mrRet = DmxCliDumpPbbufInfo(prCliCfg->u4arg1);
		break;
	case DMX_CLI_CMD_DUMP_GAU_INFO:
		mrRet = DmxCliDumpGAUInfo();
		break;
	case DMX_CLI_CMD_DUMP_THRESHOLD_INFO:
		mrRet = DmxCliDumpThresholdInfo(prCliCfg->u4arg1);
		break;
	case DMX_CLI_CMD_ENABLE_THRESHOLD:
		mrRet = DmxCliEnableThreshold(prCliCfg->u4arg1, prCliCfg->u4arg2);
		break;
	case DMX_CLI_CMD_PRINT_AUGET_LOG:
		mrRet = DmxCliPrintGetAULog(prCliCfg->u4arg1, prCliCfg->u4arg2);
		break;
	case DMX_CLI_CMD_DUMP_INSTS_INFO:
		mrRet = DmxCliDumpInstsInfo(prCliCfg->u4arg1);
		break;
	case DMX_CLI_CMD_DUMP_HW_INFO:
		mrRet = DmxCliDumpPidStructure(prCliCfg->u4arg1);
		break;
	case DMX_CLI_CMD_DUMP_ALLAUDATA:
		mrRet = DmxCliDumpAllAUData(prCliCfg->u4arg1,
			prCliCfg->u4arg2, (char *)prCliCfg->ptParam);
		break;
	case DMX_CLI_CMD_DUMP_MEM_USAGE:
		mrRet = DmxCliDumpMemUsage();
		break;
	case DMX_CLI_CMD_DUMP_FLOW:
		DMXLOG_TRACE(TEXT("[SPT] %s line %d DMX_CLI_CMD_DUMP_FLOW(arg1: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prCliCfg->u4arg1);
		mrRet = DmxCliDumpFlow(prCliCfg->u4arg1, (char *)prCliCfg->ptParam);
		break;
	case DMX_CLI_CMD_DUMP_AU_INFO:
		mrRet = DmxCliDumpAUInfo(prCliCfg->u4arg1, prCliCfg->u4arg2, prCliCfg->u4arg3);
		break;
	case DMX_CLI_CMD_PRINT_PERF_INFO:
		mrRet = DmxCliPrintPerfInfo();
		break;
	default:
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail for Unsupport Cli cmd(0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prCliCfg->eDmxCliType);
		mrRet = RET_DMX_NO_IMPLEMENT;
		break;
	}

	MM_RETURN(mrRet);
}

