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
 * @file dmx_psr_filter.c
 *
 * @par Project
 *
 * @par Description
 *
 *
 * @par Author_Name
 *    Shuhui Zhang
 *
 */
#include "x_debug.h"
#include "x_rtos.h"
#ifdef __linux__
#include <linux/mm.h>
#include "windows.h"
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_cfa_def.h>
#include <media/atc/drv_esm_if.h>
#include <media/atc/ioctl_dmx.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "dmx_cfa_def.h"
#include "drv_esm_if.h"
#include "ioctl_dmx.h"
#include "mm_debug.h"
#endif				/* __linux__ */

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_spt.h"
#include "dmx_cli.h"
#include "dmx_dump.h"
#include "dmx_inst.h"
#include "dmx_psr_filter.h"
#include "dmx_parser.h"
#include "dmx_psr_pbbuf.h"
#include "dmx_esm_if.h"
#include "dmx_psr_esm.h"
#include "dmx_psr_util.h"
#include "dmx_stream.h"
#include "dmx_gau_if.h"
#include "dmx_esm.h"
#include "dmx_pfm.h"
#include "stc_hal.h"

#ifndef __linux__
#pragma warning(disable : 4127)
#endif

#if DMX_PFM_TEST
EXTERN PSR_PFM g_rPsrPfm;
#endif				/* DMX_PFM_TEST */

EXTERN DMX_CLI_MAN_T g_rDmxCliMan;
EXTERN DMX_DUMP_MAN_T g_rDmxDumpMan;

static MRESULT PSR_Filter_PreChkForCmdQTx(PSR_FILTER *prPsrFtr, u32 u4FifoSpace,
					  u32 u4CurMaxTxLen);

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_InitHALStatus */
/* Initialize Video header buffer information */
/* //////////////////////////////////////////////////////////////////////////////// */
static void PSR_Filter_InitHALStatus(PSR_FILTER *prPsrFtr, PSR_HDRDET_STATUS_T *prHeaderDetect)
{
	if (!((NULL != prHeaderDetect) && (NULL != prPsrFtr))) {
		DMX_ASSERT(FALSE);
		return;
	}

	PSR_HAL_LOCK;

	prHeaderDetect->u4PTransStatus = 0;
	prHeaderDetect->u4PTransStatus1 = 0;
	prHeaderDetect->u4HdrDectQueNum = 0;
	prHeaderDetect->u4HdrDectQueData = 0;
	prHeaderDetect->eHdrDectQueType = HDQ_PICTYPE;
	prHeaderDetect->u4HdrDetBufSa = 0;
	prHeaderDetect->u4HdrDetBufEa = 0;
	prHeaderDetect->u4HdrDetBufWPtr = 0;
	prHeaderDetect->u4HdrDetBufRPtr = 0;
	prHeaderDetect->u4HdrDetBufStrPtr = 0;

	switch (prPsrFtr->eType) {
	case SPT_DATA_V:
		{
			u32 u4PhysMem = DMX_PHYSICAL_REG(prPsrFtr->ptrHdrBufAddr);	/* 32K */

			if (0 == u4PhysMem) {
				PSR_HAL_UNLOCK;
				DMX_ASSERT(FALSE);
				return;
			}
			prHeaderDetect->u4HdrDetBufSa = u4PhysMem;

			prHeaderDetect->u4HdrDetBufEa = u4PhysMem + DMX_PESHDR_WORKBUF_SIZE - 1;
			prHeaderDetect->u4HdrDetBufWPtr = u4PhysMem;
			prHeaderDetect->u4HdrDetBufRPtr = u4PhysMem;
			prHeaderDetect->u4HdrDetBufStrPtr = u4PhysMem;
		}
		break;
	case SPT_DATA_A:
	case SPT_DATA_SP:
	case SPT_DATA_SECTION:
		{
			u32 u4PhysMem = DMX_PHYSICAL_REG(prPsrFtr->ptrHdrBufAddr);	/* 32K */

			if (0 == u4PhysMem) {
				PSR_HAL_UNLOCK;
				DMX_ASSERT(FALSE);
				return;
			}
			prHeaderDetect->u4HdrDetBufSa = u4PhysMem;

			prHeaderDetect->u4HdrDetBufEa = u4PhysMem + DMX_PESHDR_WORKBUF_SIZE - 1;
			prHeaderDetect->u4HdrDetBufWPtr = u4PhysMem;
			prHeaderDetect->u4HdrDetBufRPtr = u4PhysMem;
			prHeaderDetect->u4HdrDetBufStrPtr = u4PhysMem;
		}
		break;
	default:
		break;
	}
	PSR_HAL_UNLOCK;
}

MRESULT PSR_Filter_SetVDummyInfo(PSR_FILTER *prPsrFtr, bool fgDummyAUEnd, bool fgDummyCmdAU)
{
	PSR_VFSD *prVFSD = NULL;

	if (NULL == prPsrFtr) {
		DMXLOG_ERROR(TEXT("[PSR] %s fail for invalid args!\r\n"),
			    DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (SPT_DATA_V != prPsrFtr->eType) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s fail for PsrFilter(0x%x)'s Type(%d) != Video!\r\n"),
			    DMX_FUNC_NAME, prPsrFtr, prPsrFtr->eType);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prVFSD = (PSR_VFSD *) prPsrFtr->pvFilterSpecific;

	if (NULL == prVFSD) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s fail for PSR_VFSD hasn't been allocated!\r\n"),
			    DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	prVFSD->fgDummyCmdAU = fgDummyCmdAU;
	prVFSD->fgDummyAUEnd = fgDummyAUEnd;

	MM_RETURN(RET_DMX_OK);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_AddDummyAU */
/* Condition: this function is using for Video Parser Filter */
/* Add One Dummy AU into ESM table, it do the following work: */
/* 1. If AU table is full, set Parser CC's Txstate to be TXS_WAIT_FIFO,
*Parser CC's state to be CCS_TX */
/* 2. If fgUseDummyAURealWrIdx is TRUE, get the DummyAURealWr ESM AU(prVPicAU) */
/* 3. If fgUseRealWrIdx is TRUE */
/* 1)If there is prev AU, set UseRealWrIdx to be FALSE, if  is MP4 I/P/B Frame,
*  Set its End Address, get the RealWrIdx AU(prVPicAU)*/
/* 2)Otherwise, get the current ESM WrIdx AU(prVPicAU) */
/* 4. Otherwise, get the current ESM WrIdx AU(prVPicAU) */
/* 5. If Current Data Type is Video, Set AU Information, if there are prev AU,
* set current AU's duration to be equal to last */
/* 6. Set fgUseDummyAURealWrIdx to be TURE, and set ESM Wr Idx */
/* 7. Set Parser CC state to be CCS_INIT, Parser CC tx state to be TXS_TX_OK, */
/* 8. Set Parser CC Tx Done */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_AddDummyAU(PSR_FILTER *prPsrFtr, bool fgDummyAUEnd, bool fgDummyCmdAU)
{
	PSR_CC *prPsrCC = NULL;
	PSR_VFSD *prVFSD = NULL;
	u32 u4VType = 0;
	u32 u4AUIdx = ESM_INVALID_INDEX;
	u32 u4PrevAUIdx = ESM_INVALID_INDEX;
	AU_VPic *prVPicAU = NULL;
	AU_VPic *prPrevVPicAU = NULL;
	PSR_AU rAU;
	uintptr_t ptrHWCurWp = 0;
	u32 u4TotalAUCnt = 0;
	u32 u4RdIdx = 0;
	u32 u4WrIdx = 0;
	bool fgFull = 0;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prPsrFtr) {
		DMXLOG_ERROR(TEXT("[PSR] %s fail for invalid args\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (0 == ((prPsrFtr->u4Flag) & FF_ENABLE)) {
		DMXLOG_TRACE(TEXT("[PSR] %s line %d exit, Stream has been disable, so don't add dummy au\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OK);
	}

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);
	if (NULL == prPsrCC) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s fail for PsrFtr(0x%x)'s pvPsrCC == NULL\r\n"),
			    DMX_FUNC_NAME, prPsrFtr);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (prPsrFtr->u4StmType != SPT_DATA_V) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s fail for PsrFtr(0x%x)'s StmType(%d) != SPT_DATA_V\r\n"),
			    DMX_FUNC_NAME, prPsrFtr, prPsrFtr->eType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prVFSD = (PSR_VFSD *) (prPsrFtr->pvFilterSpecific);
	prPsrCC->pvActFilter = prPsrFtr;

	if (NULL == prVFSD) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s fail for prPsrFtr(0x%x)'s prVFSD == NULL\r\n"),
			    DMX_FUNC_NAME, prPsrFtr);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mrRet = PSR_Filter_SetVDummyInfo(prPsrFtr, fgDummyAUEnd, fgDummyCmdAU);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in PSR_Filter_SetVDummyInfo")
					  TEXT("(%d, %d), pvSptHdl 0x%x, mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, (fgDummyAUEnd ? 1 : 0),
			    (fgDummyCmdAU ? 1 : 0), prPsrCC->pvSptHdl, mrRet);
		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);
		prPsrCC->eState = CCS_TX;
		MM_RETURN(mrRet);
	}

	prVFSD->fgDummyTxWakeUp = FALSE;

	mrRet = PSR_Filter_IsAUTableFull(prPsrFtr, &fgFull);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s fail in PSR_Filter_IsAUTableFull, PsrCC's")
					  TEXT(" st:0x%x, tx st:0x%x, normal wait pts:0x%llx\r\n"),
			    DMX_FUNC_NAME, prPsrCC->eState, prPsrCC->eTxState,
			    prPsrCC->u8NormalWaitPts);
		GAU_DisableThreshold();
		prVFSD->fgDummyTxWakeUp = TRUE;
		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);
		prPsrCC->eState = CCS_TX;
		MM_RETURN(mrRet);
	}

	if (fgFull) {
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
			TEXT
			("[PSR] %s -- fifo full, st:0x%x, tx st:0x%x, normal wait pts:0x%llx\r\n"),
			DMX_FUNC_NAME, prPsrCC->eState, prPsrCC->eTxState,
			prPsrCC->u8NormalWaitPts);
		prVFSD->fgDummyTxWakeUp = TRUE;
		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);
		prPsrCC->eState = CCS_TX;
		MM_RETURN(RET_DMX_OK);
	}

	mrRet = ESM_FifoGetWrPtr(prPsrFtr->u4ESIH, &ptrHWCurWp);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			    TEXT
			    ("[PSR] %s line %d fail in ESM_FifoGetWrPtr(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if ((ptrHWCurWp >= prPsrFtr->ptrESFifoEa) || (ptrHWCurWp < prPsrFtr->ptrESFifoSa)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ptrHWCurWp(0x%lx) ")
					  TEXT("exceed PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, ptrHWCurWp, prPsrFtr->ptrESFifoSa,
			    prPsrFtr->ptrESFifoEa);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	mrRet = ESM_AUTableGetTotalCount(prPsrFtr->u4ESIH, &u4TotalAUCnt);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			    TEXT
			    ("[PSR] %s line %d fail in ESM_AUTableGetTotalCount, mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	mrRet = ESM_AUTableGetWrIdx(prPsrFtr->u4ESIH, &u4WrIdx);

	DMXLOG_TRACE(TEXT("[PSR] %s line %d -- fgUseDummyAURealWrIdx: %d, ")
				  TEXT
				  ("u4DummyAURealWrIdx: %d, fgUseRealWrIdx: %d, u4RealWrIdx: %d\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO, ((prVFSD->fgUseDummyAURealWrIdx) ? 1 : 0),
		    prVFSD->u4DummyAURealWrIdx, ((prVFSD->fgUseRealWrIdx) ? 1 : 0),
		    prVFSD->u4RealWrIdx);

	if (prVFSD->fgUseDummyAURealWrIdx) {
		mrRet = ESM_AUTableGetNextAUIdx(prPsrFtr->u4ESIH,
						prVFSD->u4DummyAURealWrIdx, 1, &u4AUIdx);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				    TEXT("[PSR] %s line %d fail in ESM_AUTableGetNextAUIdx")
				     TEXT("(auidx: %d, Count: %d), mrRet: 0x%x\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prVFSD->u4DummyAURealWrIdx, 1,
				    mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
	} else {
		/* 1. If fgUseRealWrIdx is TRUE */
		/* 1)If there is prev AU, set UseRealWrIdx to be FALSE,
		 * if  is MP4 I/P/B Frame, Set its End Address, get the RealWrIdx AU(prVPicAU) */
		/* 2)Otherwise, get the current ESM WrIdx AU(prVPicAU) */
		/* 2. Otherwise, get the current ESM WrIdx AU(prVPicAU) */
		if (prVFSD->fgUseRealWrIdx) {
			u4PrevAUIdx = prVFSD->u4RealWrIdx;
			if (u4PrevAUIdx >= u4TotalAUCnt) {
				DMXLOG_ERROR(
			    TEXT("[PSR] %s line %d fail for u4PrevAUIdx(%d) > u4TotalAUCnt(%d)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u4PrevAUIdx, u4TotalAUCnt);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			mrRet =
			    ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4PrevAUIdx,
						 (void **) &prPrevVPicAU);
			/* fix klocwork bug */
			if (NULL == prPrevVPicAU) {
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_NO_AU);
			}

			u4AUIdx = prVFSD->u4RealWrIdx;

			if (0 != prPrevVPicAU->rAUInfo.rInfo.ptrSAddr) {
				/* check Current AU whether is a real picture */
				if (AU_DATA == prPrevVPicAU->eAuType) {
					if (0 != GetPicType(prPrevVPicAU->rAUInfo.rInfo.u4VType)) {
						rAU.eType = prPsrFtr->eType;
						rAU.pvAUInf = prPrevVPicAU;
						
						prPrevVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;
						
						prVFSD->fgHasSetPics = FALSE;
						
						if (prVFSD->fgCreNoHdrAUWaitPkt) {
							prPrevVPicAU->rAUInfo.rInfo.u4VType |=
									prVFSD->u4VType;
							prVFSD->fgCreNoHdrAUWaitPkt = FALSE;
						}
						
						SetVCodec(prPrevVPicAU->rAUInfo.rInfo.u4VType,
								prVFSD->eVCodeC);
						
						DMXLOG_TRACE(
							TEXT("[PSR] %s line %d --> AUIdx: %d")
							 TEXT("SA: 0x%08x, EA: 0x%08x, VType: 0x%08x\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO, prVFSD->u4RealWrIdx, 
							prPrevVPicAU->rAUInfo.rInfo.ptrSAddr, 
							prPrevVPicAU->rAUInfo.rInfo.ptrEAddr,
							prPrevVPicAU->rAUInfo.rInfo.u4VType);
						
						/* Prepare to Create a New AU Idx for Demmy AU */
						mrRet = ESM_AUTableGetNextAUIdx(prPsrFtr->u4ESIH,
										u4PrevAUIdx, 1, &u4AUIdx);
						if (DMX_FAILED(mrRet)) {
							DMXLOG_ERROR(
								TEXT("[PSR] %s line %d fail in ESM_AUTable")
								 TEXT("GetNextAUIdx(auidx: %d, Count: %d),")
								 TEXT(" mrRet: 0x%x\r\n"),
								DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx, 1,
								mrRet);
							DMX_ASSERT(FALSE);
							MM_RETURN(mrRet);
						}
					}
				}
			}
		} else {
			mrRet = ESM_AUTableGetWrIdx(prPsrFtr->u4ESIH, &u4AUIdx);
			if (DMX_FAILED(mrRet)) {
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}
		}
	}

	if (u4AUIdx >= u4TotalAUCnt) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s line %d fail for u4AUIdx(%d) > u4TotalAUCnt(%d)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx, u4TotalAUCnt);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4AUIdx, (void **) &prVPicAU);
	if (DMX_FAILED(mrRet) || (NULL == prVPicAU)) {
		DMXLOG_ERROR(
			    TEXT
			    ("[PSR] %s line %d fail in ESM_AUTableGetAUInfo(auidx: %d), mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}
	/* 1. If Current Data Type is Video, Set AU Information, if there are
	 * prev AU, set current AU's duration to be equal to last */
	/* 2. Otherwise, Assert */
	if (SPT_DATA_V == prPsrFtr->eType) {
		/* callback notify Splitter to fill AU information */
		if (fgDummyCmdAU) {
			dmx_memset(prVPicAU, 0, sizeof(AU_VPic));
			prVPicAU->eAuType = AU_CMD;
			/* Designated whether the CmdAU is the last Pic
			 * CmdAU(FALSE) or ClearVFifo CmdAU(TRUE) */
			prVPicAU->fgIBCSent = FALSE;
			prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrHWCurWp;
			prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;
			u4VType = 0;

			SetVCodec(u4VType, prVFSD->eVCodeC);
			prVPicAU->rAUInfo.rInfo.u4VType = u4VType;

			prVPicAU->rAUInfo.rInfo.u8Offset = prPsrFtr->u8TxCurrOffset;

			prVFSD->fgDummyCmdAU = FALSE;

			DMXLOG_TRACE(
				    TEXT("[PSR] %s line %d Add Dummy Cmd AU(Idx:%d) -- ")
				     TEXT("u4VType: 0x%x, SA: 0x%x, EA: 0x%x!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx, u4VType,
				    prVPicAU->rAUInfo.rInfo.ptrSAddr,
				    prVPicAU->rAUInfo.rInfo.ptrEAddr);
		} else {
			prVPicAU->eAuType = AU_DATA;
			u4VType = DUMMY_FRM;
			rAU.eType = prPsrFtr->eType;
			rAU.pvAUInf = prVPicAU;
			rAU.pvAUExtInf = NULL;
			prVPicAU->rAUInfo.rInfo.u4VType = u4VType;
			prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrHWCurWp;
			prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;

			if (0 == prVPicAU->rAUInfo.rInfo.u8Offset)
				prVPicAU->rAUInfo.rInfo.u8Offset = prPsrFtr->u8TxCurrOffset;

			prVPicAU->rAUInfo.rInfo.u4Duration = 0;
			prVPicAU->rAUInfo.rInfo.u4PrevDuration = 0;
			prVPicAU->rAUInfo.rInfo.u8PrevPTS = 0x0;
			/* Set AU Information */
			if (g_rDmxCliMan.fgDumpFlow) {
				DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

				mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
				rOperInfo.pvSptHdl = prPsrCC->pvSptHdl;
				rOperInfo.unFlow.rFillAU.u4AUIdx = u4AUIdx;
				rOperInfo.unFlow.rFillAU.u4StmType = SPT_DATA_V;
				DmxDumpFlow(DMX_OPER_FILLAU, &rOperInfo);
			}
#if !DMX_DISABLE_FILL_AUINFO
			PSR_CC_CBSplitter(prPsrCC, E_GET_AU_INFO, (void *) &rAU);
#endif

			prVPicAU->rAUInfo.rInfo.u8Pts = INVALID_TIMESTAMP;

			/* Modify Prev PTS */
			mrRet = ESM_AUTableGetPrevAUIdx(prPsrFtr->u4ESIH, u4AUIdx, 1, &u4PrevAUIdx);
			if (DMX_FAILED(mrRet)) {
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}

			if (ESM_INVALID_INDEX != u4PrevAUIdx) {
				mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4PrevAUIdx,
							     (void **) &prPrevVPicAU);
				if (DMX_FAILED(mrRet)) {
					DMX_ASSERT(FALSE);
					MM_RETURN(mrRet);
				}

				if (NULL != prPrevVPicAU) {
					if (INVALID_TIMESTAMP == prVPicAU->rAUInfo.rInfo.u8PrevPTS) {
						/* For File Playback PTS control */
						prPrevVPicAU->rAUInfo.rInfo.u8Pts =
						    INVALID_TIMESTAMP;
					}

					if (prVPicAU->rAUInfo.rInfo.u4PrevDuration != 0) {
						prPrevVPicAU->rAUInfo.rInfo.u4Duration =
						    prVPicAU->rAUInfo.rInfo.u4PrevDuration;
					}
				}
			}

			SetVCodec(u4VType, prVFSD->eVCodeC);
			prVPicAU->rAUInfo.rInfo.u4VType = u4VType;

			DMXLOG_DEBUG(TEXT("[PSR] %s line %d Add Dummy Frm AU(Idx:%d)")
						  TEXT
						  (" -- u4VType: 0x%x, SA: 0x%x, EA: 0x%x!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx, u4VType,
				    prVPicAU->rAUInfo.rInfo.ptrSAddr,
				    prVPicAU->rAUInfo.rInfo.ptrEAddr);
		}
	} else {
		/* Error Condition */
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	/* 1. Set UseDummyAURealWrIdx to be TURE, and set ESM Wr Idx */
	prVFSD->u4DummyAURealWrIdx = u4AUIdx;
	prVFSD->fgUseDummyAURealWrIdx = TRUE;
	prVFSD->fgUseRealWrIdx = FALSE;

	prPsrCC->eState = CCS_INIT;
	PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);

	if (!prVFSD->fgDummyAUEnd) {
		if (prVFSD->u4DummyAURealWrIdx >= u4TotalAUCnt) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for prVFSD->u4Dummy")
						  TEXT("AURealWrIdx(%d) > u4TotalAUCnt(%d)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prVFSD->u4DummyAURealWrIdx,
				    u4TotalAUCnt);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		mrRet = ESM_AUTableGetRdIdx(prPsrFtr->u4ESIH, &u4RdIdx);
		mrRet = ESM_AUTableSetWrIdx(prPsrFtr->u4ESIH, prVFSD->u4DummyAURealWrIdx);
		DMXLOG_TRACE(TEXT("[PSR] %s line %d -- Set ESM_AUTableSetWrIdx: ")
					  TEXT("%d, u4RdIdx: %d, PrevWrIdx: %d!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prVFSD->u4DummyAURealWrIdx, u4RdIdx,
			    u4WrIdx);
	} else {
		if (prVFSD->u4DummyAURealWrIdx >= u4TotalAUCnt) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for prVFSD->")
						  TEXT
						  ("u4DummyAURealWrIdx(%d) > u4TotalAUCnt(%d)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prVFSD->u4DummyAURealWrIdx,
				    u4TotalAUCnt);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		if (fgDummyCmdAU) {
			mrRet = ESM_AUTableGetNextAUIdx(prPsrFtr->u4ESIH,
							prVFSD->u4DummyAURealWrIdx, 0, &u4AUIdx);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ESM_AUTableGet")
							  TEXT
							  ("NextAUIdx(auidx: %d, Count: %d), mrRet: 0x%x\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, prVFSD->u4DummyAURealWrIdx, 1,
					    mrRet);
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}
		} else {
			mrRet = ESM_AUTableGetNextAUIdx(prPsrFtr->u4ESIH,
							prVFSD->u4DummyAURealWrIdx, 1, &u4AUIdx);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ESM_AUTableGet")
							  TEXT
							  ("NextAUIdx(auidx: %d, Count: %d), mrRet: 0x%x\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, prVFSD->u4DummyAURealWrIdx, 1,
					    mrRet);
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}
		}

		if (u4AUIdx >= u4TotalAUCnt) {
			DMXLOG_ERROR(
				    TEXT
				    ("[PSR] %s line %d fail for u4AUIdx(%d) > u4TotalAUCnt(%d)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx, u4TotalAUCnt);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}
		mrRet = ESM_AUTableGetRdIdx(prPsrFtr->u4ESIH, &u4RdIdx);

		mrRet = ESM_AUTableSetWrIdx(prPsrFtr->u4ESIH, u4AUIdx);
		DMXLOG_TRACE(TEXT("[PSR] %s line %d -- Set ESM_AUTableSetWrIdx")
					  TEXT("(u4AUIdx: %d), u4RdIdx: %d, u4WrIdx: %d!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx, u4RdIdx, u4WrIdx);
	}

	if (DMX_FAILED(mrRet)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	prPsrCC->fgUseCmdQ = FALSE;
	prPsrCC->fgAUByCmdQEnd = FALSE;

	PSR_CC_CBSplitter(prPsrCC, E_TX_DONE, NULL);

	MM_RETURN(RET_DMX_OK);
}


/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_Create */
/* Get one unused Parser Filter instance, initialize it */
/* /@Param u4PsrFtrType [IN] Stream Type, eg. SPT_DATA_V, SPT_DATA_A, SPT_DATA_SP, and so on */
/* /@Param u4PsrFtrId     [IN] Stream ID in this kind of stream, eg. For Audio,
* if we using the first Audio stream, u4StmUID is 0 */
/* /@Param pu4Handle     [OUT] Parser Filter handle */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_Create(void *pvDmxInst, u32 u4PsrFtrType, u32 u4PsrFtrId,
			  void **ppvHandle)
{
	u32 u4Idx;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if ((NULL == ppvHandle) || (NULL == g_rPsrMan.aprPsrFtrs[0])) {
		DMXLOG_ERROR(TEXT("[STM] %s failed for invalid args, (u4PsrFtrType: ")
					  TEXT("0x%x, u4PsrFtlId: 0x%x, ppvHandle: 0x%x\r\n"),
			    DMX_FUNC_NAME, u4PsrFtrType, u4PsrFtrId, ppvHandle);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*ppvHandle = NULL;

	/* find a empty filter slot */
	for (u4Idx = 0; u4Idx < MAX_FILTER_COUNT; u4Idx++) {
		if (0 == (g_rPsrMan.aprPsrFtrs[u4Idx]->u4Flag & FF_USED))
			break;
	}

	if (MAX_FILTER_COUNT == u4Idx) {
		DMXLOG_ERROR(TEXT("[STM] %s failed for no unsed PSR_FILTER, ")
					  TEXT
					  ("(u4PsrFtrType: 0x%x, u4PsrFtlId: 0x%x, ppvHandle: 0x%p\r\n"),
			    DMX_FUNC_NAME, u4PsrFtrType, u4PsrFtrId, ppvHandle);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}

	switch (u4PsrFtrType) {
	case SPT_DATA_V:
		{
			/* PES Header Buffer(Video) */
			DMX_NewHwAlignMemory(DMX_PESHDR_WORKBUF_SIZE, 256,
					      g_rPsrMan.aprPsrFtrs[u4Idx]->ptrHdrBufAddr);

			if (0 == g_rPsrMan.aprPsrFtrs[u4Idx]->ptrHdrBufAddr) {
				DMXLOG_ERROR(
					    TEXT
					    ("[PSR] %s line %d -- Video PES header buffer alloc fail!!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO);
				MM_RETURN(RET_DMX_NO_MEM);
			}
			smp_mb();
			dmx_memset((void *) (g_rPsrMan.aprPsrFtrs[u4Idx]->ptrHdrBufAddr), 0,
				   sizeof(DMX_PESHDR_WORKBUF_SIZE));
			break;
		}
	case SPT_DATA_A:
	case SPT_DATA_SP:
	case SPT_DATA_SECTION:
		{
			/* PES Header Buffer(Video) */
			DMX_NewHwAlignMemory(DMX_PESHDR_WORKBUF_SIZE, 256,
					      g_rPsrMan.aprPsrFtrs[u4Idx]->ptrHdrBufAddr);

			if (0 == g_rPsrMan.aprPsrFtrs[u4Idx]->ptrHdrBufAddr) {
				DMXLOG_ERROR(
					    TEXT
					    ("[PSR] %s line %d -- Audio PES header buffer alloc fail!!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO);
				MM_RETURN(RET_DMX_NO_MEM);
			}
			smp_mb();
			dmx_memset((void *) (g_rPsrMan.aprPsrFtrs[u4Idx]->ptrHdrBufAddr), 0,
				   sizeof(DMX_PESHDR_WORKBUF_SIZE));
		}
		break;
	default:
		g_rPsrMan.aprPsrFtrs[u4Idx]->ptrHdrBufAddr = 0;
		break;
	}

	g_rPsrMan.aprPsrFtrs[u4Idx]->u4Flag = FF_USED;
	g_rPsrMan.aprPsrFtrs[u4Idx]->eType = SPT_DATA_GRD;
	g_rPsrMan.aprPsrFtrs[u4Idx]->u4StmType = u4PsrFtrType;
	g_rPsrMan.aprPsrFtrs[u4Idx]->u4StmUID = u4PsrFtrId;
	g_rPsrMan.aprPsrFtrs[u4Idx]->u4ESIH = DMX_INVALID_UINT32;
	g_rPsrMan.aprPsrFtrs[u4Idx]->u4GAU = DMX_INVALID_UINT32;
	g_rPsrMan.aprPsrFtrs[u4Idx]->pvPsrCC = NULL;
	g_rPsrMan.aprPsrFtrs[u4Idx]->ptrESFifoSa = ESM_INVALID_ADDRESS;
	g_rPsrMan.aprPsrFtrs[u4Idx]->ptrESFifoEa = ESM_INVALID_ADDRESS;
	g_rPsrMan.aprPsrFtrs[u4Idx]->u4ESFifoSize = 0;
	g_rPsrMan.aprPsrFtrs[u4Idx]->pvFilterSpecific = NULL;
	g_rPsrMan.aprPsrFtrs[u4Idx]->ucHwDevId = DMX_INVALID_UINT8;
	g_rPsrMan.aprPsrFtrs[u4Idx]->ptrBkWrPtr = 0;
	g_rPsrMan.aprPsrFtrs[u4Idx]->fgAUCtrlByLen = FALSE;
	g_rPsrMan.aprPsrFtrs[u4Idx]->fgAUCtrlByEnd = FALSE;
	g_rPsrMan.aprPsrFtrs[u4Idx]->u8TxCurrOffset = 0;
	g_rPsrMan.aprPsrFtrs[u4Idx]->u8TotalAULen = 0;
	g_rPsrMan.aprPsrFtrs[u4Idx]->u8CurAULen = 0;
	g_rPsrMan.aprPsrFtrs[u4Idx]->u8WMDRMTxLen = 0;
	g_rPsrMan.aprPsrFtrs[u4Idx]->u8HdrPTS = INVALID_TIMESTAMP;
	g_rPsrMan.aprPsrFtrs[u4Idx]->u8LastPTS = INVALID_TIMESTAMP;
	g_rPsrMan.aprPsrFtrs[u4Idx]->fgAUEnd = FALSE;
	g_rPsrMan.aprPsrFtrs[u4Idx]->u4AUExtCnt = 0;
	g_rPsrMan.aprPsrFtrs[u4Idx]->fgFirstAUInRng = TRUE;
	g_rPsrMan.aprPsrFtrs[u4Idx]->u8DecSendBufMask = 0;
#ifdef MM_SUPPORT_DIVXHT31
	g_rPsrMan.aprPsrFtrs[u4Idx]->u81stPTS = INVALID_TIMESTAMP;
	g_rPsrMan.aprPsrFtrs[u4Idx]->u8PrevPTS = INVALID_TIMESTAMP;
#endif				/* MM_SUPPORT_DIVXHT31 */
	g_rPsrMan.aprPsrFtrs[u4Idx]->u4AUCntFromIFrm = 0;
	g_rPsrMan.aprPsrFtrs[u4Idx]->u4IFrmCnt = 0;
	g_rPsrMan.aprPsrFtrs[u4Idx]->pvDmxInst = pvDmxInst;

	*ppvHandle = (void *) g_rPsrMan.aprPsrFtrs[u4Idx];

	MM_RETURN(RET_DMX_OK);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_Destroy */
/* Destrory ESM, free Parser Filter Private Info, Reset Filter Info to 0 */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_Destroy(PSR_FILTER *prPsrFtr)
{
	s32 i;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrFtr)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	for (i = 0; i < MAX_FILTER_COUNT; ++i) {
		if (((PSR_FILTER *) prPsrFtr == g_rPsrMan.aprPsrFtrs[i]) &&
		    (((PSR_FILTER *) prPsrFtr)->u4Flag & FF_USED)) {
			i = MAX_FILTER_COUNT << 1;
			break;
		}
	}
	if (i == MAX_FILTER_COUNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DMXLOG_DEBUG(TEXT("[PSR] %s -- prPsrFtr:0x%x\r\n"), DMX_FUNC_NAME, prPsrFtr);

	/* destroy ESI */
	if (DMX_INVALID_UINT32 != prPsrFtr->u4ESIH)
		ESM_Destroy(prPsrFtr->u4ESIH);

	if (NULL != prPsrFtr->pvFilterSpecific) {
		switch (prPsrFtr->eType) {
		case SPT_DATA_V:
			{
				PSR_VFSD *prVFSD = (PSR_VFSD *) prPsrFtr->pvFilterSpecific;
				MRESULT mrRet = RET_DMX_OK;

				if (NULL != prVFSD) {
					prVFSD->eVCodeC = VC_UNKNOW;
					mrRet = PSR_Filter_SetVCodeC(prPsrFtr);
					if (DMX_FAILED(mrRet)) {
						DMXLOG_ERROR(
							    TEXT
							     ("[PSR] %s line %d fail in PSR_Filter_")
							     TEXT
							     ("SetVCodeC(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
							    DMX_FUNC_NAME, DMX_LINE_NO,
							    prPsrFtr->u4ESIH, mrRet);
						MM_RETURN(mrRet);
					}

					if (NULL != prVFSD->prHALStatus) {
						DMX_FreeMemory(prVFSD->prHALStatus);
						prVFSD->prHALStatus = NULL;
					}
				}

				if (0 != prPsrFtr->ptrHdrBufAddr) {
					DMX_FreeHwMemory(prPsrFtr->ptrHdrBufAddr);
					prPsrFtr->ptrHdrBufAddr = 0;
				}
			}
			break;
		case SPT_DATA_A:
			{
				PSR_AUDFSD *prAFSD = (PSR_AUDFSD *) prPsrFtr->pvFilterSpecific;

				if ((NULL != prAFSD) && (NULL != prAFSD->prHALStatus)) {
					DMX_FreeMemory(prAFSD->prHALStatus);
					prAFSD->prHALStatus = NULL;
				}

				if (0 != prPsrFtr->ptrHdrBufAddr) {
					DMX_FreeHwMemory(prPsrFtr->ptrHdrBufAddr);
					prPsrFtr->ptrHdrBufAddr = 0;
				}
			}
			break;
		case SPT_DATA_SP:
		case SPT_DATA_SECTION:
			{
				PSR_NORMALFSD *prAFSD =
				    (PSR_NORMALFSD *) prPsrFtr->pvFilterSpecific;

				if ((NULL != prAFSD) && (NULL != prAFSD->prHALStatus)) {
					DMX_FreeMemory(prAFSD->prHALStatus);
					prAFSD->prHALStatus = NULL;
				}

				if (0 != prPsrFtr->ptrHdrBufAddr) {
					DMX_FreeHwMemory(prPsrFtr->ptrHdrBufAddr);
					prPsrFtr->ptrHdrBufAddr = 0;
				}
			}
			break;
		case SPT_DATA_BUF:
			{
				PSR_DMASD *prDMASD = (PSR_DMASD *) prPsrFtr->pvFilterSpecific;

				if (0 != prDMASD->ptrPrivMemSa) {
					DMX_FreeHwMemory(prDMASD->ptrPrivMemSa);
					prDMASD->ptrPrivMemSa = 0;
				}
			}
		default:
			break;
		}

		DMX_FreeMemory(prPsrFtr->pvFilterSpecific);
		prPsrFtr->pvFilterSpecific = NULL;
	}
	/* reset memory */
	dmx_memset((void *) prPsrFtr, 0, sizeof(PSR_FILTER));
	prPsrFtr->u4ESIH = DMX_INVALID_UINT32;
	prPsrFtr->u4GAU = DMX_INVALID_UINT32;
	prPsrFtr->u8HdrPTS = INVALID_TIMESTAMP;
	prPsrFtr->u8LastPTS = INVALID_TIMESTAMP;
#ifdef MM_SUPPORT_DIVXHT31
	prPsrFtr->u81stPTS = INVALID_TIMESTAMP;
	prPsrFtr->u8PrevPTS = INVALID_TIMESTAMP;
#endif				/* MM_SUPPORT_DIVXHT31 */
	prPsrFtr->u4AUCntFromIFrm = 0;
	prPsrFtr->u4IFrmCnt = 0;
	prPsrFtr->ucHwDevId = DMX_INVALID_UINT8;
	prPsrFtr->u8DecSendBufMask = 0;

	prPsrFtr->pvDmxInst = NULL;
	MM_RETURN(RET_DMX_OK);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_Enable */
/* Enable or Disable Parser Filter, it does the following works: */
/* 1. If Enable, Set Parser Filter FF_ENABLE Flag, and Alloc VFilter Pic Header Detection Status Structure */
/* 2. If Disable, it does the following: */
/* 1) Clear VFilter Pic Header Detection Status Structure */
/* 2) If Parser CC's Current TxState is WAIT_VFIFO_PTS_THRESHOLD, and wait pts is 0,
* Wait Parser Filter is this, set Wake up it */
/* 3) If Parser CC's Current TxState is WAIT_FIFO, Acitve Filter is this, set Wake up it */
/* 4) If txing, wait txing complete, */
/* 5) If tx complete or cc idle, set Parser CC state to be CCS_INIT, set Parser CC tx
* state to be TXS_TX_OK, notify Tx done */
/* 6) Clear FF_ENABLE Flag */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_Enable(PSR_FILTER *prPsrFtr)
{
	PSR_CC *prPsrCC = NULL;
	s32 i;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	for (i = 0; i < MAX_FILTER_COUNT; ++i) {
		if (((PSR_FILTER *) prPsrFtr == g_rPsrMan.aprPsrFtrs[i]) &&
		    (((PSR_FILTER *) prPsrFtr)->u4Flag & FF_USED)) {
			i = MAX_FILTER_COUNT << 1;
			break;
		}
	}
	if (i == MAX_FILTER_COUNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);

	GAU_SetEOS(prPsrCC->pvSptHdl, FALSE, 0);
	if (prPsrFtr->u4Flag & FF_ENABLE) {
		DMXLOG_TRACE(
			    TEXT("[PSR] %s -- do nothing for ftr %x is enable already!!\r\n"),
			    DMX_FUNC_NAME, prPsrFtr);
		MM_RETURN(RET_DMX_OK);
	}

	prPsrFtr->u4Flag |= FF_ENABLE;

	switch (prPsrFtr->eType) {
	case SPT_DATA_V:
		{
			PSR_VFSD *prVFSD = (PSR_VFSD *) prPsrFtr->pvFilterSpecific;

			if (NULL == prVFSD) {
				DMXLOG_ERROR(TEXT("[PSR] %s fail for PsrFtr(0x%x)'s ")
							  TEXT
							  ("PSR_VFSD han't been initailized!!\r\n"),
					    DMX_FUNC_NAME, prPsrFtr);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_NO_INIT);
			}

			prVFSD->fgWVC1KeepHdrInfo = FALSE;
			prVFSD->fgDummyTxWakeUp = FALSE;
			prVFSD->fgUseRealWrIdx = FALSE;
			prVFSD->u4RealWrIdx = 0;
			prVFSD->u4DummyAURealWrIdx = 0;
			prVFSD->fgUseDummyAURealWrIdx = FALSE;
			prVFSD->fgHasSetPics = FALSE;
			prVFSD->fgCreNoHdrAUWaitPkt = FALSE;
			prVFSD->u4VType = 0;

			if (NULL == prVFSD->prHALStatus) {
				DMX_NewMemory(sizeof(PSR_HDRDET_STATUS_T), prVFSD->prHALStatus);
				if (NULL == prVFSD->prHALStatus) {
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_NO_MEM);
				}
			}

			dmx_memset((void *) prVFSD->prHALStatus, 0, sizeof(PSR_HDRDET_STATUS_T));

			PSR_Filter_InitHALStatus(prPsrFtr, prVFSD->prHALStatus);
			DMXLOG_DEBUG(
				    TEXT
				    ("[PSR] %s line %d -- VideoHdrBufAddr: 0x%08x, PhyMem: 0x%p\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->ptrHdrBufAddr,
				    DMX_PHYSICAL_REG(prPsrFtr->ptrHdrBufAddr));
		}
		break;
	case SPT_DATA_A:
		{
			PSR_AUDFSD *prAFSD = (PSR_AUDFSD *) prPsrFtr->pvFilterSpecific;

			if (NULL == prAFSD) {
				DMXLOG_ERROR(
					    TEXT("[PSR] %s fail for PsrFtr(0x%x)'s PSR_AUDFSD ")
					     TEXT("han't been initailized!!\r\n"),
					    DMX_FUNC_NAME, prPsrFtr);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_NO_INIT);
			}

			if (NULL == prAFSD->prHALStatus) {
				DMX_NewMemory(sizeof(PSR_HDRDET_STATUS_T), prAFSD->prHALStatus);
				if (NULL == prAFSD->prHALStatus) {
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_NO_MEM);
				}
			}

			dmx_memset((void *) prAFSD->prHALStatus, 0, sizeof(PSR_HDRDET_STATUS_T));
			prAFSD->eAudFmtType = AUD_DRV_FMT_UNKNOWN;

			PSR_Filter_InitHALStatus(prPsrFtr, prAFSD->prHALStatus);
			DMXLOG_DEBUG(
				    TEXT("%s line %d -- AudHdrBufAddr: 0x%08x, PhyMem: 0x%p\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->ptrHdrBufAddr,
				    DMX_PHYSICAL_REG(prPsrFtr->ptrHdrBufAddr));
			prPsrFtr->u8HdrPTS = INVALID_TIMESTAMP;
			prPsrFtr->u8LastPTS = INVALID_TIMESTAMP;
		}
		break;
	case SPT_DATA_SP:
	case SPT_DATA_SECTION:
		{
			PSR_NORMALFSD *prAFSD = (PSR_NORMALFSD *) prPsrFtr->pvFilterSpecific;

			if (NULL == prAFSD) {
				DMXLOG_ERROR(TEXT("[PSR] %s fail for PsrFtr(0x%x)'s ")
							  TEXT
							  ("PSR_NORMALFSD han't been initailized!!\r\n"),
					    DMX_FUNC_NAME, prPsrFtr);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_NO_INIT);
			}

			if (NULL == prAFSD->prHALStatus) {
				DMX_NewMemory(sizeof(PSR_HDRDET_STATUS_T), prAFSD->prHALStatus);
				if (NULL == prAFSD->prHALStatus) {
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_NO_MEM);
				}
			}

			dmx_memset((void *) prAFSD->prHALStatus, 0, sizeof(PSR_HDRDET_STATUS_T));

			PSR_Filter_InitHALStatus(prPsrFtr, prAFSD->prHALStatus);
			DMXLOG_DEBUG(
				    TEXT
				    ("%s line %d -- SP or Section HdrBufAddr: 0x%08x, PhyMem: 0x%p\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->ptrHdrBufAddr,
				    DMX_PHYSICAL_REG(prPsrFtr->ptrHdrBufAddr));
			prPsrFtr->u8HdrPTS = INVALID_TIMESTAMP;
			prPsrFtr->u8LastPTS = INVALID_TIMESTAMP;
		}
		break;
	default:
		prPsrFtr->u8HdrPTS = INVALID_TIMESTAMP;
		prPsrFtr->u8LastPTS = INVALID_TIMESTAMP;
		break;
	}

	prPsrFtr->fgFirstAUInRng = TRUE;
#ifdef MM_SUPPORT_DIVXHT31
	if (SPT_DATA_V == prPsrFtr->eType) {
		PSR_CC *prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);

		if (NULL != prPsrCC) {
			if (CFA_TYPE_AVI == SplitterGetCfaType(prPsrCC->pvSptHdl)) {
				if (DMX_IS_FF_PLAY(prPsrCC->pvSptHdl))
					prPsrFtr->fgFirstAUInRng = FALSE;
			}
		}
	}
#endif				/* MM_SUPPORT_DIVXHT31 */

	prPsrFtr->u4AUCntFromIFrm = 0;

	prPsrCC->pvNormalWaitFtr = 0;
	prPsrCC->pvNormalWaitOthFtr = 0;
	prPsrCC->u8NormalWaitPts = INVALID_TIMESTAMP;

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_Filter_InitSpecific(PSR_FILTER *prPsrFtr)
{
	PSR_CC *prPsrCC = NULL;
	s32 i;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	for (i = 0; i < MAX_FILTER_COUNT; ++i) {
		if (((PSR_FILTER *) prPsrFtr == g_rPsrMan.aprPsrFtrs[i]) &&
		    (((PSR_FILTER *) prPsrFtr)->u4Flag & FF_USED)) {
			i = MAX_FILTER_COUNT << 1;
			break;
		}
	}
	if (i == MAX_FILTER_COUNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);

	/* Clear VFilter Pic Header Detection Status Information */
	switch (prPsrFtr->eType) {
	case SPT_DATA_V:
		{
			PSR_VFSD *prVFSD = (PSR_VFSD *) prPsrFtr->pvFilterSpecific;

			if (NULL == prVFSD) {
				DMXLOG_ERROR(TEXT("[PSR] %s fail for PsrFtr(0x%x)'s ")
							  TEXT
							  ("PSR_VFSD han't been initailized!!\r\n"),
					    DMX_FUNC_NAME, prPsrFtr);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_NO_INIT);
			} else {
				prVFSD->fgWVC1KeepHdrInfo = FALSE;
				prVFSD->fgDummyTxWakeUp = FALSE;
				prVFSD->fgUseDummyAURealWrIdx = FALSE;
				prVFSD->u4VType = 0;
			}
		}
		break;
	default:
		break;
	}

	MM_RETURN(RET_DMX_OK);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_Enable */
/* Enable or Disable Parser Filter, it does the following works: */
/* 1. If Enable, Set Parser Filter FF_ENABLE Flag, and Alloc VFilter Pic Header Detection Status Structure */
/* 2. If Disable, it does the following: */
/* 1) Clear VFilter Pic Header Detection Status Structure */
/* 2) If Parser CC's Current TxState is WAIT_VFIFO_PTS_THRESHOLD, and wait pts is 0,
* Wait Parser Filter is this, set Wake up it */
/* 3) If Parser CC's Current TxState is WAIT_FIFO, Acitve Filter is this, set Wake up it */
/* 4) If txing, wait txing complete, */
/* 5) If tx complete or cc idle, set Parser CC state to be CCS_INIT, set Parser CC tx state to
* be TXS_TX_OK, notify Tx done */
/* 6) Clear FF_ENABLE Flag */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_Disable(PSR_FILTER *prPsrFtr)
{
	PSR_CC *prPsrCC = NULL;
	s32 i;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	for (i = 0; i < MAX_FILTER_COUNT; ++i) {
		if (((PSR_FILTER *) prPsrFtr == g_rPsrMan.aprPsrFtrs[i]) &&
		    (((PSR_FILTER *) prPsrFtr)->u4Flag & FF_USED)) {
			i = MAX_FILTER_COUNT << 1;
			break;
		}
	}
	if (i == MAX_FILTER_COUNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);

	switch (prPsrFtr->eType) {
	case SPT_DATA_V:
	case SPT_DATA_A:
	case SPT_DATA_SP:
	case SPT_DATA_SECTION:
		if (prPsrCC != NULL) {
			DMXLOG_TRACE(TEXT("[PSR] %s line %d -- 4 PsrCC's eState: %d, ")
						  TEXT
						  ("eTxState: %d, ActFilter: 0x%x, PsrFtr: 0x%x, eType: %s\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->eState, prPsrCC->eTxState,
				    prPsrCC->pvActFilter, prPsrFtr,
				    ((prPsrFtr->eType <
				      MAX_SPT_DATA_TYPE_CNT) ? g_aszSptDataTypeName[prPsrFtr->eType]
				     : TEXT("UNKNOWN")));
		}
		break;
	default:
		break;
	}

	prPsrFtr->u4Flag &= (~FF_ENABLE);
	if (NULL != prPsrCC)
		prPsrCC->u4Flag &= (~CCF_CPS_ON);

	PSR_Filter_InitSpecific(prPsrFtr);

	MM_RETURN(RET_DMX_OK);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_SetVCodeC */
/* Set Video Codec */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_SetVCodeC(PSR_FILTER *prPsrFtr)
{
	PSR_VFSD *prVFSD = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prPsrFtr) {
		DMXLOG_ERROR(TEXT("[PSR] %s fail for invalid args!\r\n"),
			    DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (SPT_DATA_V != prPsrFtr->eType) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s fail for PsrFilter(0x%x)'s Type(%d) != Video!\r\n"),
			    DMX_FUNC_NAME, prPsrFtr, prPsrFtr->eType);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prVFSD = (PSR_VFSD *) prPsrFtr->pvFilterSpecific;

	if (NULL == prVFSD) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s fail for PSR_VFSD hasn't been allocated!\r\n"),
			    DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	mrRet = PSR_HAL_SetVideoCodec(prPsrFtr->ucHwDevId, prVFSD->eVCodeC, FALSE);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			    TEXT
			    ("[PSR] %s fail in PSR_HAL_SetVideoCodec, eVCodeC: %d, mrRet: 0x%x!\r\n"),
			    DMX_FUNC_NAME, prVFSD->eVCodeC, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_TxToGround */
/* Check whether the data needed(u8TxStartOffset + u8TxLen) is in the pBbuf, */
/* if not, Set TXS_WAIT_PBBUF txstate */
/* otherwise, prPsrCC->u4TxPBBufIdx is the pbbuf index, set u8TxCurrOffset to be
* u8TxStartOffset + u8TxLen, and Set  Parser CC state to be CCS_INIT,
* Parser CC TxState to be TXS_TX_OK */
/* and notify tx done */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_TxToGround(PSR_FILTER *prPsrFtr)
{
	PSR_CC *prPsrCC = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prPsrFtr) {
		DMXLOG_ERROR(TEXT("[PSR] %s fail for invalid args!\r\n"),
			    DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrCC = (PSR_CC *) prPsrFtr->pvPsrCC;

	if (NULL == prPsrCC) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s fail for PsrFtr(0x%x)'s PsrCC == NULL!\r\n"),
			    DMX_FUNC_NAME, prPsrFtr);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DMXLOG_DEBUG(TEXT("[PSR] %s -- hdl:0x%x, St:0x%x, TxSt:0x%x, ")
				  TEXT
				  ("Flag:0x%x, TxOfst:0x%llx, TxLen:0x%llx, CurTxOfst:0x%llx\r\n"),
		    DMX_FUNC_NAME, prPsrCC,
		    prPsrCC->eState, prPsrCC->eTxState,
		    prPsrCC->u4Flag,
		    prPsrCC->u8TxStartOffset,
		    prPsrCC->u8TxLen,
		    prPsrCC->u8TxCurrOffset);

	prPsrCC->fgUseCmdQ = FALSE;
	prPsrCC->fgAUByCmdQEnd = FALSE;

	/* change state to init and tx state to tx ok */
	prPsrCC->eState = CCS_INIT;
	PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);

	/* callback spliiter notify tx done */
	PSR_CC_CBSplitter(prPsrCC, E_TX_DONE, NULL);

	MM_RETURN(mrRet);
}


/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_TriggerHALGTx */
/* TX data from Memory to Memory, it does the following tasks: */
/* If this parser filter is not enable, call txtoground function */
/* otherwise, If we can't obtain the HW resource, Set Parser CC TxState to be TXS_WAIT_HW */
/* otherwise, copy the tx data from ptrTxCurrSa to tx mem, */
/* Set Parser CC state to be TXS_WAIT_IRQ_PROC, Wakeup Parser CC to tranfer mem data */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_TriggerHALGTx(PSR_FILTER *prPsrFtr)
{
	PSR_CC *prPsrCC = NULL;
	PSR_DMASD *prDMASD = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (!(NULL != prPsrFtr)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);

	if (!(NULL != prPsrCC)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_CC);
	}

	prDMASD = (PSR_DMASD *) prPsrFtr->pvFilterSpecific;

	if (SPT_DATA_BUF != prPsrFtr->eType)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	if (0 == prDMASD->ptrTxTgtMemSa)
		MM_RETURN(RET_DMX_UNEXPECT);

	if (0 == (prPsrFtr->u4Flag & FF_ENABLE)) {
		mrRet = PSR_Filter_TxToGround(prPsrFtr);
		MM_RETURN(mrRet);
	}

	if (prPsrCC->fgCfaPrsEnd)
		MM_RETURN(RET_DMX_UNEXPECT);

	if (!PSR_HWRes_Obtain(prPsrFtr)) {
		/* PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_HW); */
		MM_RETURN(RET_DMX_OK);
	}

	{
		uintptr_t ptrTxDstSa = 0;

		if (prPsrCC->u8TxCurrOffset >= prPsrCC->u8TxStartOffset) {
			ptrTxDstSa =
			    (uintptr_t) (prPsrCC->u8TxCurrOffset - prPsrCC->u8TxStartOffset) +
			    prDMASD->ptrTxTgtMemSa;
			dmx_memcpy((void *) ptrTxDstSa, (void *) prPsrCC->ptrTxCurrSa,
				   (uintptr_t) prPsrCC->u8TxCurrLen);
		}


		prPsrCC->eTxState = TXS_WAIT_IRQ_PROC;
		/* notify splitter wake up me to handle IRQ process */
		PSR_CC_CBSplitter(prPsrCC, E_WAKEUP_ME, NULL);
	}

	MM_RETURN(RET_DMX_OK);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_TriggerHALPTx */
/* If this parser filter is DMA or Ground, set Parser CC state to be TXS_WAIT_FIFO */
/* otherwise, If this parser filter is not enable, call txtoground function, */
/* otherwise, set Parser CC txstate to be TXS_TXING, */
/* If we can't obtain the HW resource, Set Parser CC TxState to be TXS_WAIT_HW */
/* otherwise, 1) Set Video Tx Header Dect Status, if this parser filter type is SPT_DATA_V,
* we alsho should set Video Codec, and */
/* 2) Set TX Src, Dst, and triggle TX */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_TriggerHALPTx(PSR_FILTER *prPsrFtr)
{
	PSR_HALPT_INFO_T rPTInfo;
	PSR_CC *prPsrCC = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prPsrFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mm_memset(&rPTInfo, 0, sizeof(PSR_HALPT_INFO_T));

	prPsrCC = (PSR_CC *) prPsrFtr->pvPsrCC;

	if (NULL == prPsrCC) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_CC);
	}

	if ((SPT_DATA_BUF == prPsrFtr->eType) || (SPT_DATA_GRD == prPsrFtr->eType)) {
		/* Todo: if need set state here, set state only for keep old flow setting @20090613 */
		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!(prPsrFtr->u4Flag & FF_ENABLE)) {
		mrRet = PSR_Filter_TxToGround(prPsrFtr);
		MM_RETURN(mrRet);
	}

	if (prPsrCC->fgCfaPrsEnd)
		MM_RETURN(RET_DMX_UNEXPECT);

	PSR_CC_SetTxSt(prPsrCC, TXS_TXING);
	if (!PSR_HWRes_Obtain(prPsrFtr))
		MM_RETURN(RET_DMX_OK);

	DMX_ASSERT(prPsrFtr->ucHwDevId == 0);

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = prPsrCC->pvSptHdl;
		rOperInfo.unFlow.rDma.u4StmType = prPsrFtr->eType;
		if (0 != (prPsrFtr->u4Flag & FF_TX_PBBUF)) {
			rOperInfo.unFlow.rDma.u8FileOfst = prPsrCC->u8TxCurrOffset;
			rOperInfo.unFlow.rDma.pvBuf = NULL;
		} else {
			rOperInfo.unFlow.rDma.u8FileOfst = 0;
			rOperInfo.unFlow.rDma.pvBuf = (void *) (prPsrCC->ptrTxCurrSa);
		}
		rOperInfo.unFlow.rDma.u8Len = prPsrCC->u8TxCurrLen;
		DmxDumpFlow(DMX_OPER_HW_PTX, &rOperInfo);
	}

	switch (prPsrFtr->eType) {
	case SPT_DATA_V:
		{
			PSR_VFSD *prVFSD = (PSR_VFSD *) prPsrFtr->pvFilterSpecific;

			rPTInfo.eBitType = BitType_Video;
			rPTInfo.ePTMode = PTMode_DMA;

			mrRet = PSR_Filter_SetVCodeC(prPsrFtr);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
							  TEXT
							  ("PSR_Filter_SetVCodeC(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
				PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);
				MM_RETURN(mrRet);
			}

			mrRet = PSR_HAL_SetLastHdrDetStatus(prPsrFtr->ucHwDevId, BitType_Video, prVFSD->prHALStatus);
			prPsrCC->fgVidPass = TRUE;
		}
		break;

	case SPT_DATA_A:
		{
			PSR_AUDFSD *prAFSD = (PSR_AUDFSD *) prPsrFtr->pvFilterSpecific;

			rPTInfo.eBitType = BitType_Audio;
			rPTInfo.ePTMode = PTMode_DMA;

			mrRet = PSR_HAL_SetLastHdrDetStatus(prPsrFtr->ucHwDevId, BitType_Audio, prAFSD->prHALStatus);
		}
		break;

	case SPT_DATA_SP:
		{
			PSR_NORMALFSD *prSPFSD = (PSR_NORMALFSD *) prPsrFtr->pvFilterSpecific;

			rPTInfo.eBitType = BitType_SubPic0;
			rPTInfo.ePTMode = PTMode_DMA;

			mrRet = PSR_HAL_SetLastHdrDetStatus(prPsrFtr->ucHwDevId, BitType_SubPic0, prSPFSD->prHALStatus);
		}
		break;

	case SPT_DATA_SECTION:
		{
			PSR_NORMALFSD *prSECFSD = (PSR_NORMALFSD *) prPsrFtr->pvFilterSpecific;

			rPTInfo.eBitType = BitType_Section;
			rPTInfo.ePTMode = PTMode_DMA;

			mrRet = PSR_HAL_SetLastHdrDetStatus(prPsrFtr->ucHwDevId, BitType_Section, prSECFSD->prHALStatus);
		}
		break;

	default:
		DMXLOG_ERROR(TEXT("[PSR] %s fail for invalid prPsrFtr->eType: %d\r\n"),
			    DMX_FUNC_NAME, prPsrFtr->eType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	rPTInfo.ptrSrcSa = prPsrCC->ptrTxCurrSa;
	rPTInfo.u4SrcLen = (u32) (prPsrCC->u8TxCurrLen);
	rPTInfo.ptrFifoSa = prPsrFtr->ptrESFifoSa;
	rPTInfo.u4FifoSz = prPsrFtr->u4ESFifoSize;
	rPTInfo.ptrFifoRdPtr = 0;
	rPTInfo.ptrFifoWrPtr = 0;
	mrRet = ESM_FifoGetWrPtr(prPsrFtr->u4ESIH, &(rPTInfo.ptrFifoWrPtr));
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
					  TEXT
					  ("ESM_FifoGetWrPtr(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);
		MM_RETURN(mrRet);
	}

	mrRet = ESM_FifoGetRdPtr(prPsrFtr->u4ESIH, &(rPTInfo.ptrFifoRdPtr));
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
					  TEXT
					  ("ESM_FifoGetRdPtr(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);
		MM_RETURN(mrRet);
	}

	prPsrCC->ptrPTXFifoRdPtr = rPTInfo.ptrFifoRdPtr;
	prPsrCC->ptrPTXFifoWrPtr = rPTInfo.ptrFifoWrPtr;
	prPsrCC->ptrPTXSrcSa = rPTInfo.ptrSrcSa;
	prPsrCC->u4PTXSrcLen = rPTInfo.u4SrcLen;

	rPTInfo.u4GarbageSz = 0;

	rPTInfo.fgUseCmdQ = prPsrCC->fgUseCmdQ;

	if (prPsrCC->fgUseCmdQ) {
		if (NULL == prPsrCC->pvCmdQTxEntryBuffer) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for ")
						  TEXT
						  ("CmdQTxEntryBuffer is NULL, PsrFtr's Type: %d, mrRet: 0x%x\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->eType, mrRet);
			PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		rPTInfo.pvCmdQTxEntryBuffer = prPsrCC->pvCmdQTxEntryBuffer;
		rPTInfo.u2EntryRdIdx = prPsrCC->rCmdQTxInf.u2CurTxRngSIdx;
		rPTInfo.u2EntryWrIdx = prPsrCC->rCmdQTxInf.u2CurTxRngEIdx;
		rPTInfo.u4RdIdxOfst = 0;
		rPTInfo.u4RdIdxLen = prPsrCC->rCmdQTxInf.u4CurTxRngSIdxLen;
		rPTInfo.u4LastValidIdxLen = prPsrCC->rCmdQTxInf.u4CurTxRngEIdxLen;

		if (SPT_DATA_V == prPsrFtr->eType) {
			if ((rPTInfo.u2EntryWrIdx == prPsrCC->u2TxEntryCnt - 1) &&
			    (0 == prPsrCC->rCmdQTxInf.u8RmnTotalRealTxLen)) {
				DMX_CMDQ_TX_ENTRY_T *prTxEntry =
				    (DMX_CMDQ_TX_ENTRY_T *) (prPsrCC->pvCmdQTxEntryBuffer);
				u32 u4EndDataOfst = 0;
				u32 u4CmdIdx = 0;

				u4EndDataOfst += rPTInfo.u4RdIdxOfst + rPTInfo.u4RdIdxLen;
				prTxEntry += rPTInfo.u2EntryRdIdx;
				for (u4CmdIdx = rPTInfo.u2EntryRdIdx + 1;
				     u4CmdIdx < rPTInfo.u2EntryWrIdx; u4CmdIdx++) {
					prTxEntry++;
					u4EndDataOfst += prTxEntry->u4TxOfst + prTxEntry->u4TxLen;
				}

				if (rPTInfo.u2EntryRdIdx != rPTInfo.u2EntryWrIdx) {
					prTxEntry =
					    (DMX_CMDQ_TX_ENTRY_T *) (prPsrCC->pvCmdQTxEntryBuffer);
					prTxEntry += rPTInfo.u2EntryWrIdx;
					u4EndDataOfst += prTxEntry->u4TxOfst + prTxEntry->u4TxLen;
				}

				rPTInfo.u4GarbageSz = 0;

				if (rPTInfo.u4GarbageSz > 0) {
					prTxEntry->u4TxLen += rPTInfo.u4GarbageSz;
					rPTInfo.u4LastValidIdxLen += rPTInfo.u4GarbageSz;
					if (u4EndDataOfst + rPTInfo.u4GarbageSz > rPTInfo.u4SrcLen) {
						rPTInfo.u4SrcLen =
						    u4EndDataOfst + rPTInfo.u4GarbageSz;
					}
				}
			}
		}
		/* Rd index done already, can ignore wr idx */
		if (rPTInfo.u2EntryRdIdx == rPTInfo.u2EntryWrIdx) {
			rPTInfo.u2EntryWrIdx++;
			rPTInfo.u4LastValidIdxLen = rPTInfo.u4RdIdxLen;
		} else if (rPTInfo.u4LastValidIdxLen > 0) {
			/* HW will ignore wr idx, update setting for HW */
			rPTInfo.u2EntryWrIdx++;
		}
#if ENABLE_DMX_ADVANCED_VER
		{
			u16 u2EntryIdx = 0;
			u16 u2CurEntryCnt = 0;
			u32 u4InsertTimes = 0, u4InsertLen = 0;
			DMX_CMDQ_TX_ENTRY_T *prTxEntry = NULL;

			prTxEntry =
			    ((DMX_CMDQ_TX_ENTRY_T *) (prPsrCC->pvCmdQTxEntryBuffer)) +
			    rPTInfo.u2EntryRdIdx;
			if ((prTxEntry->fgInsertHdr)
			    && (0 == prPsrCC->rCmdQTxInf.u4CurTxRngSIdxOfst)
			    && (0 < prPsrCC->rCmdQTxInf.u4CurTxRngEIdxLen)) {
				u4InsertTimes++;
				if (0 == u4InsertLen) {
					u4InsertLen = prTxEntry->u4InsertHdrLen;
					prPsrCC->u4InsertHdrLen = prTxEntry->u4InsertHdrLen;
					mm_memcpy(prPsrCC->au1InsertHdrBuf, prTxEntry->au1InsertHdr,
						  prPsrCC->u4InsertHdrLen);
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_INSTBS,
						TEXT
						("[PSR] %s line %d -- InsertHdr(TRUE), HdrLen(%d)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, u4InsertLen);
				}
			}
			u2CurEntryCnt++;

			for (u2EntryIdx = rPTInfo.u2EntryRdIdx + 1;
			     (u2EntryIdx < rPTInfo.u2EntryWrIdx)
			     && (u2EntryIdx < prPsrCC->u2TxEntryCnt); u2EntryIdx++) {
				prTxEntry =
				    ((DMX_CMDQ_TX_ENTRY_T *) (prPsrCC->pvCmdQTxEntryBuffer)) +
				    u2EntryIdx;
				if (prTxEntry->fgInsertHdr) {
					if (u4InsertTimes < u2CurEntryCnt) {
						DMXLOG_ERROR(
							    TEXT
							     ("[PSR] %s line %d fail for the current")
							     TEXT
							     (" chips don't support to insert the ")
							     TEXT("header not continuously\r\n"),
							    DMX_FUNC_NAME, DMX_LINE_NO);
						MM_RETURN(RET_DMX_HW_ERROR);
					} else if (u4InsertTimes > u2CurEntryCnt) {
						DMXLOG_ERROR(
							    TEXT
							     ("[PSR] %s line %d fail for the current")
							     TEXT
							     (" chips don't support to insert the ")
							     TEXT("header not continuously\r\n"),
							    DMX_FUNC_NAME, DMX_LINE_NO);
						MM_RETURN(RET_DMX_HW_ERROR);
					} else {
						u4InsertTimes++;
					}

					if (0 == u4InsertLen) {
						u4InsertLen = prTxEntry->u4InsertHdrLen;
						prPsrCC->u4InsertHdrLen = prTxEntry->u4InsertHdrLen;
						mm_memcpy(prPsrCC->au1InsertHdrBuf,
							  prTxEntry->au1InsertHdr,
							  prPsrCC->u4InsertHdrLen);
						DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_INSTBS,
							TEXT
							("[PSR] %s line %d -- InsertHdr(TRUE), HdrLen(%d)\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO, u4InsertLen);
					}
				}
				u2CurEntryCnt++;
			}

			rPTInfo.rInstBytesInfo.fgInsertBytes = (u4InsertTimes > 0) ? TRUE : FALSE;
			rPTInfo.rInstBytesInfo.u4InsertLen = u4InsertLen;
			rPTInfo.rInstBytesInfo.u4Inserttimes = u4InsertTimes;
			rPTInfo.rInstBytesInfo.pu1InsertBuf = prPsrCC->au1InsertHdrBuf;
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_INSTBS,
				TEXT("[PSR] %s line %d -- CmdQDma, InsertHdr(%s),")
				 TEXT(" HdrLen(%d), InsertTimes(%d), HdrBuf(0x%x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				((rPTInfo.rInstBytesInfo.
				  fgInsertBytes) ? TEXT("TRUE") : TEXT("FALSE")),
				rPTInfo.rInstBytesInfo.u4InsertLen,
				rPTInfo.rInstBytesInfo.u4Inserttimes,
				rPTInfo.rInstBytesInfo.pu1InsertBuf);
		}
#endif				/* ENABLE_DMX_ADVANCED_VER */
	} else {
		if (SPT_DATA_V == prPsrFtr->eType) {
			if (rPTInfo.u4SrcLen >= 4) {
				rPTInfo.u4GarbageSz = 0;
				DMX_ASSERT(rPTInfo.u4GarbageSz <= 2);
				rPTInfo.u4SrcLen += rPTInfo.u4GarbageSz;
			}
		}
#if ENABLE_DMX_ADVANCED_VER
		rPTInfo.rInstBytesInfo.fgInsertBytes = prPsrCC->fgInsertHdr;
		rPTInfo.rInstBytesInfo.u4InsertLen = prPsrCC->u4InsertHdrLen;
		rPTInfo.rInstBytesInfo.u4Inserttimes = (prPsrCC->fgInsertHdr) ? 1 : 0;
		rPTInfo.rInstBytesInfo.pu1InsertBuf = prPsrCC->au1InsertHdrBuf;
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_INSTBS,
			TEXT("[PSR] %s -- InsertHdr(%s), HdrLen(%d),")
			 TEXT(" InsertTimes(%d), HdrBuf(0x%x)\r\n"),
			DMX_FUNC_NAME,
			((rPTInfo.rInstBytesInfo.fgInsertBytes) ? TEXT("TRUE") : TEXT("FALSE")),
			rPTInfo.rInstBytesInfo.u4InsertLen, rPTInfo.rInstBytesInfo.u4Inserttimes,
			rPTInfo.rInstBytesInfo.pu1InsertBuf);
#endif				/* ENABLE_DMX_ADVANCED_VER */
	}


	rPTInfo.rGlobalFunc.pfnCB = (DMX_HAL_FUNC_CB) PSR_HAL_GlobalCB;
	rPTInfo.rGlobalFunc.pvPrivData = (void *) prPsrCC;

	rPTInfo.rPidFunc.pfnNotify = (PFN_PVR_NOTIFY) PSR_HAL_PIDCB;
	rPTInfo.rPidFunc.pvPrivData = (void *) prPsrFtr;

	rPTInfo.eCPSMode = CPSMode_NONE;

#if DMX_DRM_DECRYPT_USE_HW
	if (DECRYPT_DIVXDRM == prPsrCC->rDecryptMan.eDecryptType) {
		rPTInfo.rDRMInfo.eMode = PVR_DRM_MODE_BYPASS;

		if ((DECRYPT_COMPLETE != prPsrCC->rDecryptMan.eStatus) &&
		    (DECRYPT_BY_HW == prPsrCC->rDecryptMan.eMethod)) {
			if (SPT_DATA_V == prPsrFtr->eType) {
				rPTInfo.rDRMInfo.u2KeyLen =
				    prPsrCC->rDecryptMan.rHWParam.rDRM.rVideo.u2KeyLen;
				rPTInfo.rDRMInfo.fgDoEncrypt = FALSE;
				rPTInfo.rDRMInfo.eMode = PVR_DRM_MODE_AES;
				mm_memcpy(rPTInfo.rDRMInfo.au1Key,
					  prPsrCC->rDecryptMan.rHWParam.rDRM.rVideo.au1Key,
					  sizeof(u8) * PVR_DMEM_MM_KEY_LEN);
				rPTInfo.rDRMInfo.fgCbc =
				    prPsrCC->rDecryptMan.rHWParam.rDRM.rVideo.fgCBC;
				rPTInfo.eDescMode = PVR_DESC_MODE_AES_ECB;
				if (prPsrCC->rDecryptMan.rHWParam.rDRM.rVideo.fgCBC) {
					rPTInfo.eDescMode = PVR_DESC_MODE_AES_CBC;
					mm_memcpy(rPTInfo.rDRMInfo.au1InitVector,
						  prPsrCC->rDecryptMan.rHWParam.rDRM.rVideo.
						  au1InitVector,
						  sizeof(u8) * PVR_DMEM_MM_IV_LEN);
				}
				rPTInfo.rDRMInfo.u4DecryptOfst =
				    prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclOft;
				rPTInfo.rDRMInfo.u4DecryptLen =
				    prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclLen;
#if DMX_PRINT_DECRYPT_KEY_LOG
				{
					u32 u4Idx = 0;

					DMXLOG_TRACE(
						  TEXT("[DRM] %s -- Frame Key: "), DMX_FUNC_NAME);
					for (u4Idx = 0; u4Idx < 16; u4Idx++) {
						DMXLOG_TRACE(
							  TEXT("0x%02x, \r\n"),
							   rPTInfo.rDRMInfo.au1Key[u4Idx]);
					}

				}
				DMXLOG_TRACE(
					    TEXT("[PSR] %s -- KeyLen: %d, eMode: %d, ")
					     TEXT("rPTInfo.eDescMode: %d, DecryptLclOft: %d,")
					     TEXT(" DecryptLclLen: %d\r\n"),
					    DMX_FUNC_NAME,
					    prPsrCC->rDecryptMan.rHWParam.rDRM.rVideo.u2KeyLen,
					    rPTInfo.rDRMInfo.eMode, rPTInfo.eDescMode,
					    rPTInfo.rDRMInfo.u4DecryptOfst,
					    rPTInfo.rDRMInfo.u4DecryptLen);
#endif				/* DMX_PRINT_DECRYPT_KEY_LOG */
			} else if (SPT_DATA_A == prPsrFtr->eType) {
				rPTInfo.rDRMInfo.u2KeyLen =
				    prPsrCC->rDecryptMan.rHWParam.rDRM.rAudio.u2KeyLen;
				rPTInfo.rDRMInfo.fgDoEncrypt = FALSE;
				rPTInfo.rDRMInfo.eMode = PVR_DRM_MODE_AES;
				mm_memcpy(rPTInfo.rDRMInfo.au1Key,
					  prPsrCC->rDecryptMan.rHWParam.rDRM.rAudio.au1Key,
					  sizeof(u8) * PVR_DMEM_MM_KEY_LEN);
				rPTInfo.rDRMInfo.fgCbc =
				    prPsrCC->rDecryptMan.rHWParam.rDRM.rAudio.fgCBC;
				rPTInfo.eDescMode = PVR_DESC_MODE_AES_ECB;
				if (prPsrCC->rDecryptMan.rHWParam.rDRM.rAudio.fgCBC) {
					rPTInfo.eDescMode = PVR_DESC_MODE_AES_CBC;
					mm_memcpy(rPTInfo.rDRMInfo.au1InitVector,
						  prPsrCC->rDecryptMan.rHWParam.rDRM.rAudio.
						  au1InitVector,
						  sizeof(u8) * PVR_DMEM_MM_IV_LEN);
				}
				rPTInfo.rDRMInfo.u4DecryptOfst =
				    prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclOft;
				rPTInfo.rDRMInfo.u4DecryptLen =
				    prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclLen;

#if DMX_PRINT_DECRYPT_KEY_LOG
				{
					u32 u4Idx = 0;

					DMXLOG_TRACE(
						  TEXT("[DRM] %s -- Audio Key: "), DMX_FUNC_NAME);
					for (u4Idx = 0; u4Idx < 16; u4Idx++) {
						DMXLOG_TRACE(
							  TEXT("0x%02x, \r\n"),
							   rPTInfo.rDRMInfo.au1Key[u4Idx]);
					}

				}
				DMXLOG_TRACE(
					    TEXT("[PSR] %s -- KeyLen: %d, eMode: %d, ")
					     TEXT("rPTInfo.eDescMode: %d, DecryptLclOft:")
					     TEXT(" %d, DecryptLclLen: %d\r\n"),
					    DMX_FUNC_NAME,
					    prPsrCC->rDecryptMan.rHWParam.rDRM.rAudio.u2KeyLen,
					    rPTInfo.rDRMInfo.eMode, rPTInfo.eDescMode,
					    rPTInfo.rDRMInfo.u4DecryptOfst,
					    rPTInfo.rDRMInfo.u4DecryptLen);
#endif				/* DMX_PRINT_DECRYPT_KEY_LOG */
			}
		}
	}
#endif				/* DMX_DRM_DECRYPT_USE_HW */

	/* ////////////////////////////////////////////////////////////////////////// */
	/* PSR_HAL_PTransfer */
	/* 1. Get the PID index according to the Bit Type to Tranfer */
	/* 2. Set Destination Fifo Info into the corresponding PID data structure */
	/* 4. Set PID's Callback function */
	/* 5. Set Command Queue and Src Buffer Info into PVR, and trigger DDI transfer */
	/* 6. Set Destination fifo and Src buffer's info into Parser Structure */
	/* ////////////////////////////////////////////////////////////////////////// */
	mrRet = PSR_HAL_PTransfer(prPsrFtr->ucHwDevId, &rPTInfo);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s fail in PSR_HAL_PTransfer, ccHdl:%x, ")
					  TEXT("St:0x%x, TxSt:0x%x, Flag:0x%x\r\n"),
			    DMX_FUNC_NAME, prPsrCC, prPsrCC->eState, prPsrCC->eTxState,
			    prPsrCC->u4Flag, prPsrCC->u8TxStartOffset, prPsrCC->u8TxLen,
			    prPsrCC->u8TxCurrOffset);

		DMXLOG_ERROR(TEXT("[PVR] %s line %d fail for BitType(%d) ")
					  TEXT
					  ("-- ESIH(FifoSA(0x%x), FifoEA(0x%x), FifoSz(0x%x))\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, rPTInfo.eBitType, prPsrFtr->ptrESFifoSa,
			    prPsrFtr->ptrESFifoEa, prPsrFtr->u4ESFifoSize);

		/* release HW access right */
		PSR_HWRes_Release(prPsrFtr);
		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_HW);
		smp_mb();
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_TX2Fifo */
/* Check Whether Fifo Full or AUTable Full, if not, set TX_FIFO_OK, and Triggle HAL to Tx data into fifo, */
/* it does the following tasks: */
/* If this parser filter is DMA or Ground, set Parser CC state to be TXS_WAIT_FIFO */
/* otherwise, If this parser filter is not enable, call txtoground function, */
/* otherwise, If current tx state is WAIT_FIFO or WAIT_VFIFO_PTS_THRESHOLD or need check fifo, */
/* check whether fifo free size is smaller than FIFO reserve size, */
/* if is, set Tx State to be TXS_WAIT_FIFO */
/* otherwise, 1) check whether AU table is full,  set Tx State to be TXS_WAIT_FIFO */
/* 2) set Tx State to be TXS_FIFO_OK. Set TX Src, Dst, and triggle TX */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_TX2Fifo(PSR_FILTER *prPsrFtr, bool fgChkFifo)
{
	PSR_CC *prPsrCC = NULL;
	bool fgFull;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prPsrFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if ((SPT_DATA_BUF == prPsrFtr->eType) ||
		(SPT_DATA_GRD == prPsrFtr->eType)) {
		/* Todo: if need set state here, set state only for keep old flow setting @20090613 */
		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!(prPsrFtr->u4Flag & FF_ENABLE)) {
		mrRet = PSR_Filter_TxToGround(prPsrFtr);
		MM_RETURN(mrRet);
	}

	prPsrCC = (PSR_CC *) prPsrFtr->pvPsrCC;

	if (NULL == prPsrCC) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_CC);
	}
	/* fifo enough?? */
	if (fgChkFifo ||
	    (TXS_WAIT_FIFO == prPsrCC->eTxState) ||
	    (TXS_WAIT_VFIFO_PTS_THRESHOLD == prPsrCC->eTxState)) {
		PSR_Filter_IsESBufFull(prPsrFtr, (u32) prPsrCC->u8TxCurrLen, &fgFull);
		if (fgFull) {
			if (SplitterRspIsEnabled(prPsrCC->pvSptHdl) &&
				SplitterIsRspOffStart(prPsrCC->pvSptHdl)) {
				prPsrCC->fgUseCmdQ = FALSE;
				prPsrCC->fgAUByCmdQEnd = FALSE;
				// check whether tx complete
				// change state to init and tx state to tx ok
				prPsrCC->eState = CCS_INIT;
				PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);
				PSR_CC_CBSplitter(prPsrCC, E_TX_DONE, NULL);
				MM_RETURN(RET_DMX_OK);
			}
#if DMX_PFM_TEST
			switch (prPsrFtr->eType) {
			case SPT_DATA_V:
				g_rPsrPfm.rVideo.u4FifoFullCnt++;
				break;
			case SPT_DATA_A:
				g_rPsrPfm.rAudio.u4FifoFullCnt++;
				break;
			case SPT_DATA_SP:
				g_rPsrPfm.rSP.u4FifoFullCnt++;
				break;
			default:
				break;
			}
#endif				/* DMX_PFM_TEST */
			PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);
			MM_RETURN(RET_DMX_FIFO_FULL);
		} else {
			mrRet = PSR_Filter_IsAUTableFull(prPsrFtr, &fgFull);
			if (DMX_FAILED(mrRet)) {
				PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);
				MM_RETURN(mrRet);
			}

			if (fgFull) {
				if (SplitterRspIsEnabled(prPsrCC->pvSptHdl) &&
					SplitterIsRspOffStart(prPsrCC->pvSptHdl)) {
					prPsrCC->fgUseCmdQ = FALSE;
					prPsrCC->fgAUByCmdQEnd = FALSE;
					// check whether tx complete
					// change state to init and tx state to tx ok
					prPsrCC->eState = CCS_INIT;
					PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);
					PSR_CC_CBSplitter(prPsrCC, E_TX_DONE, NULL);
					MM_RETURN(RET_DMX_OK);
				}
#if DMX_PFM_TEST
				switch (prPsrFtr->eType) {
				case SPT_DATA_V:
					g_rPsrPfm.rVideo.u4ESTableFullCnt++;
					break;
				case SPT_DATA_A:
					g_rPsrPfm.rAudio.u4ESTableFullCnt++;
					break;
				case SPT_DATA_SP:
					g_rPsrPfm.rSP.u4ESTableFullCnt++;
					break;
				default:
					break;
				}
#endif				/* DMX_PFM_TEST */
				PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);
				MM_RETURN(RET_DMX_FIFO_FULL);
			} else {
				PSR_CC_SetTxSt(prPsrCC, TXS_FIFO_OK);
			}
		}
	}

	mrRet = PSR_Filter_TriggerHALPTx(prPsrFtr);

	MM_RETURN(mrRet);
}


/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_TX2Mem */
/* TX data from Memory to Memory, it does the following tasks: */
/* If this parser filter is not enable, call txtoground function */
/* otherwise, If we can't obtain the HW resource, Set Parser CC TxState to be TXS_WAIT_HW */
/* otherwise, copy the tx data from ptrTxCurrSa to tx mem, */
/* Set Parser CC state to be TXS_WAIT_IRQ_PROC, Wakeup Parser CC to tranfer mem data */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_TX2Mem(PSR_FILTER *prPsrFtr)
{
	MRESULT mrRet = RET_DMX_OK;

	mrRet = PSR_Filter_TriggerHALGTx(prPsrFtr);

	MM_RETURN(mrRet);
}


/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_TxPbbuf */
/* Tx Pbbuf data to FIFO/Memory, it does the following works: */
/* 1. If this parser filter is not enable, call txtoground function */
/* 2. If this parser filter is lock, change Parser CC to init, txstate
* to TXS_TX_OK, Notify Splitter TX Done */
/* 3. If Parser CC txstate is TXS_WAIT_PBBUF, and the tx offset
* isn't in the pbbuf, return(continue to wait pbbuf) */
/* 4. Call Psr_Filter_TxDecide to decode tx len and check whether fifo is full */
/* 5. If Parser Filter flag is FF_TX_TO_FIFO, call tx2Fifo function to TX data into FIFO */
/* 6. if not, set Parser CC txstate to be TXS_WAIT_HW, call TX2Mem
* function to tx data into designated memory */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_TxPbbuf(PSR_FILTER *prPsrFtr, u64 u8Offset)
{
	MRESULT mrRet = RET_DMX_OK;
	PSR_CC *prPsrCC = NULL;

	if (NULL == prPsrFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrCC = (PSR_CC *) prPsrFtr->pvPsrCC;

	if (NULL == prPsrCC) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_CC);
	}

	if (!(prPsrFtr->u4Flag & FF_ENABLE)) {
		mrRet = PSR_Filter_TxToGround(prPsrFtr);
		MM_RETURN(mrRet);
	}

	if (prPsrFtr->u4Flag & FF_LOCK) {
		prPsrCC->fgUseCmdQ = FALSE;
		prPsrCC->fgAUByCmdQEnd = FALSE;
		/* change state to init and tx state to tx ok */
		prPsrCC->eState = CCS_INIT;
		PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);

		/* callback spliiter notify tx done */
		PSR_CC_CBSplitter(prPsrCC, E_TX_DONE, NULL);
		MM_RETURN(mrRet);
	}
	/* Check Whether Tx data is in PbBuf, if in ,
	 * change eTxState into TXS_PBBUF_OK, and add PBBUF_EXIST Flag */
	if (TXS_WAIT_PBBUF == prPsrCC->eTxState) {
		E_PBBUF_CONTINUITY_TYPE_T eContinuity = PBBUF_CONTINUITY_UNKNOWN;
		bool fgOffsetIn = FALSE;

		mrRet = PSR_CC_CheckPBBuf(prPsrCC, u8Offset, &fgOffsetIn, &eContinuity, FALSE);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s fail for in PSR_CC_CheckPBBuf,")
				  TEXT(" pvPsrCC: 0x%p, mrRet: 0x%x!\r\n"),
			    DMX_FUNC_NAME, prPsrCC, mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		if (PBBUF_UNCONTINUOUS == eContinuity) {
			/* /TODO: */
			DMXLOG_DEBUG(TEXT("[PSR] %s line %d PSR_CC_CheckPBBuf")
			  TEXT(" (Uncontinuous), u4TxPbbufIdx: %d!\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u4TxPBBufIdx);
			PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
			MM_RETURN(RET_DMX_NEED_JUMP);
		} else if (!fgOffsetIn) {
			DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- Wait PBBuf data!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_OK);
		}
	}
	/* Check Whether the Tx Data Len exceed the free space of fifo,
	 * set the could tx len in the fifo to prPsrCC->u8TxCurrLen */
	mrRet = PSR_Filter_TXDecide(prPsrFtr);

	if (DMX_FAILED(mrRet)) {
		if (RET_DMX_FIFO_FULL == mrRet) {
			GAU_DisableThreshold();
			MM_RETURN(RET_DMX_OK);
		} else if (RET_DMX_ERR_DATA == mrRet) {
			GAU_DisableThreshold();
		}
		MM_RETURN(mrRet);
	}
	/* Tiggle to Tx Data Into FIFO or Mem */
	if (prPsrFtr->u4Flag & FF_TX_TO_FIFO) {
		/* Triggle to TX data into FIFO */
		mrRet = PSR_Filter_TX2Fifo(prPsrFtr, TRUE);
	} else {
		/* Triggle to TX data into Mem */
		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_HW);
		mrRet = PSR_Filter_TX2Mem(prPsrFtr);
	}

	if (DMX_FAILED(mrRet)) {
		if (RET_DMX_FIFO_FULL == mrRet) {
			GAU_DisableThreshold();
			MM_RETURN(RET_DMX_OK);
		} else if (RET_DMX_ERR_DATA == mrRet) {
			GAU_DisableThreshold();
		}
		MM_RETURN(mrRet);
	}

	MM_RETURN(mrRet);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_TxMem */
/* Tx data from memory to FIFO/Memory, it does the following works: */
/* 1. If this parser filter is not enable, call txtoground function */
/* 2. Call Psr_Filter_TxDecide to decode tx len and check whether fifo is full */
/* 3. If Parser Filter flag is FF_TX_TO_FIFO, call tx2Fifo function to TX data into FIFO */
/* 4. if not, set Parser CC txstate to be TXS_WAIT_HW, call TX2Mem function to
* tx data into designated memory */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_TxMem(PSR_FILTER *prPsrFtr)
{
	MRESULT mrRet = RET_DMX_OK;
	PSR_CC *prPsrCC = NULL;

	if (NULL == prPsrFtr) {
		DMXLOG_DEBUG(
			    TEXT("[PSR_FTR]PSR_Filter_TxMem fail for invalid args!\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!(NULL != prPsrFtr)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrCC = (PSR_CC *) prPsrFtr->pvPsrCC;

	if (!(NULL != prPsrCC)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_CC);
	}

	if (0 == (prPsrFtr->u4Flag & FF_ENABLE)) {
		mrRet = PSR_Filter_TxToGround(prPsrFtr);
		MM_RETURN(mrRet);
	}
	/* Check Whether the Tx Data Len exceed the free space of fifo, set the
	 * could tx len in the fifo to prPsrCC->u8TxCurrLen */
	mrRet = PSR_Filter_TXDecide(prPsrFtr);
	if (DMX_FAILED(mrRet)) {
		if (RET_DMX_FIFO_FULL == mrRet) {
			GAU_DisableThreshold();
			MM_RETURN(RET_DMX_OK);
		} else if (RET_DMX_ERR_DATA == mrRet) {
			GAU_DisableThreshold();
		}
		MM_RETURN(mrRet);
	}
	/* Tiggle to Tx Data Into FIFO or Mem */
	if (prPsrFtr->u4Flag & FF_TX_TO_FIFO) {
		/* Tiggle to Tx Data Into FIFO */
		mrRet = PSR_Filter_TX2Fifo(prPsrFtr, TRUE);
	} else {
		/* Tiggle to Tx Data Into Mem */
		PSR_CC_SetTxSt(prPsrFtr->pvPsrCC, TXS_WAIT_HW);
		mrRet = PSR_Filter_TX2Mem(prPsrFtr);
	}

	if (DMX_FAILED(mrRet)) {
		if (RET_DMX_FIFO_FULL == mrRet) {
			GAU_DisableThreshold();
			MM_RETURN(RET_DMX_OK);
		} else if (RET_DMX_ERR_DATA == mrRet) {
			GAU_DisableThreshold();
		}
		MM_RETURN(mrRet);
	}

	MM_RETURN(mrRet);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_Tx4HdrParsing */
/* Tx data from memory to FIFO/Pbbuf, it does the following works: */
/* 1. If this parser filter is not enable, return OK */
/* 2. Check whether the tx start offset is in the pbbuf, if not,
* set Parser CC TxState to be TXS_WAIT_PBBUF */
/* 3. Check whether the tx end offset is in the pbbuf */
/* 1) if not, set Parser CC state to be wait pbbuf, return */
/* 2) otherwise, set u8TxCurrOffset to be tx end offset, set tx
* end address and remained size in buffer to PSR_DMASD */
/* set Parser CC state to be CCS_INIT, Parser CC txstate to
* be TXS_TX_OK, and then notify splitter TX Done */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_Tx4HdrParsing(PSR_FILTER *prPsrFtr)
{
	bool fgPBBufExist = FALSE;
	PSR_CC *prPsrCC;
	u32 u4PbbufIdx;
	u64 u8Offset;
	u32 u4Len;
	uintptr_t ptrTgtSa = 0;
	u32 u4TgtSz = 0;
	PSR_DMASD *prDMASD = NULL;
	MRESULT mrRet = RET_DMX_OK;
	E_PBBUF_CONTINUITY_TYPE_T eContinuity = PBBUF_CONTINUITY_UNKNOWN;

	if (!(NULL != prPsrFtr)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prDMASD = (PSR_DMASD *) (prPsrFtr->pvFilterSpecific);
	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);

	if (!(NULL != prPsrCC)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_CC);
	}
	if (!(NULL != prDMASD)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	u8Offset = prPsrCC->u8TxStartOffset;
	u4Len = (u32) prPsrCC->u8TxLen;

	if (0 == (prPsrFtr->u4Flag & FF_ENABLE))
		MM_RETURN(RET_DMX_OK);

	/* Check whether the tx start offset is in the pbbuf,
	 * if not, set Parser CC TxState to be TXS_WAIT_PBBUF */
	fgPBBufExist = FALSE;
	eContinuity = PBBUF_CONTINUITY_UNKNOWN;

	mrRet = PSR_CC_CheckPBBuf(prPsrCC, u8Offset, &fgPBBufExist, &eContinuity, TRUE);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s fail for in PSR_CC")
					  TEXT("_CheckPBBuf, pvPsrCC: 0x%p, mrRet: 0x%x!\r\n"),
			    DMX_FUNC_NAME, prPsrCC, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (PBBUF_UNCONTINUOUS == eContinuity) {
		DMXLOG_DEBUG(TEXT("[PSR] %s line %d PSR_CC_CheckPBBuf")
					  TEXT(" (Uncontinuous), u4TxPbbufIdx: %d!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u4TxPBBufIdx);
		PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
		MM_RETURN(RET_DMX_NEED_JUMP);
	} else if (!fgPBBufExist) {
		DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- Wait PBBuf data!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);
		MM_RETURN(RET_DMX_OK);
	}

	fgPBBufExist = FALSE;
	eContinuity = PBBUF_CONTINUITY_UNKNOWN;

	/* Check whether the Offset + Len - 1 in the slot, the purpose of this */
	/* process is to ensure the data will be decrypted if Offset + Len - 1 */
	/* is in the encrypt data */
	PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);

	mrRet =
	    PSR_CC_CheckPBBuf(prPsrCC, (u8Offset + u4Len - 1), &fgPBBufExist, &eContinuity, TRUE);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for in ")
					  TEXT
					  ("PSR_CC_CheckPBBuf, pvPsrCC: 0x%p, mrRet: 0x%x!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (PBBUF_UNCONTINUOUS == eContinuity) {
		DMXLOG_DEBUG(TEXT("[PSR] %s line %d PSR_CC_CheckPBBuf")
					  TEXT(" (Uncontinuous), u4TxPbbufIdx: %d!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u4TxPBBufIdx);
		PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
		MM_RETURN(RET_DMX_NEED_JUMP);
	} else if (!fgPBBufExist) {
		DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- Wait PBBuf data!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);
		MM_RETURN(RET_DMX_OK);
	}
	/* Check whether the tx start offset is in the pbbuf,
	 * if not, set Parser CC TxState to be TXS_WAIT_PBBUF */
	fgPBBufExist = FALSE;
	eContinuity = PBBUF_CONTINUITY_UNKNOWN;

	PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);

	mrRet = PSR_CC_CheckPBBuf(prPsrCC, u8Offset, &fgPBBufExist, &eContinuity, TRUE);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for in ")
					  TEXT
					  ("PSR_CC_CheckPBBuf, pvPsrCC: 0x%p, mrRet: 0x%x!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (PBBUF_UNCONTINUOUS == eContinuity) {
		DMXLOG_DEBUG(TEXT("[PSR] %s line %d PSR_CC_CheckPBBuf ")
					  TEXT("(Uncontinuous), u4TxPbbufIdx: %d!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u4TxPBBufIdx);
		PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
		MM_RETURN(RET_DMX_NEED_JUMP);
	} else if (!fgPBBufExist) {
		DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- Wait PBBuf data!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);
		MM_RETURN(RET_DMX_OK);
	}

	u4PbbufIdx = prPsrCC->u4TxPBBufIdx;
	if (PSR_CC_IsOffsetInPbbuf(prPsrCC, u4PbbufIdx, (u8Offset + u4Len - 1))) {
		mrRet = PSR_CC_GetWaitTxBufSa(prPsrCC, &ptrTgtSa);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
						  TEXT
						  ("PSR_CC_GetWaitTxBufSa, mrRet: 0x%x, PsrCC: 0x%p!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prPsrCC);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
		DMXLOG_DEBUG(TEXT("[PSR] %s line %d PSR_CC_GetWaitTxBufSa, 0x%p!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, ptrTgtSa);
		/* calculate remained data size in pbbuf after tx data(Pbbuf End offset - tx end offset) */
		u4TgtSz = (u32) (prPsrCC->arPBBuf[u4PbbufIdx].u8SrcOffset +
				    prPsrCC->arPBBuf[u4PbbufIdx].u4PlaySize - (u8Offset + u4Len));
	} else {
		/* Sync Pbbuf Accross Slots */
		u32 u4PbbufIdx2;
		uintptr_t pvPBBufSa;
		u32 u4CpyLen;
		u64 u8NextPbbufOffset =
		    prPsrCC->arPBBuf[u4PbbufIdx].u8SrcOffset +
		    prPsrCC->arPBBuf[u4PbbufIdx].u4PlaySize;

		fgPBBufExist = FALSE;
		eContinuity = PBBUF_CONTINUITY_UNKNOWN;

		/* make the next pbbuf slot is obtained. u4PbbufIdx2 is its index */
		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);
		mrRet =
		    PSR_CC_CheckPBBuf(prPsrCC, u8NextPbbufOffset, &fgPBBufExist, &eContinuity,
				      TRUE);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for in ")
						  TEXT
						  ("PSR_CC_CheckPBBuf, pvPsrCC: 0x%p, mrRet: 0x%x!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC, mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		if (PBBUF_UNCONTINUOUS == eContinuity) {
			/* /TODO: */
			DMXLOG_DEBUG(TEXT("[PSR] %s line %d PSR_CC_CheckPBBuf")
						  TEXT(" (Uncontinuous), u4TxPbbufIdx: %d!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u4TxPBBufIdx);
			PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
			MM_RETURN(RET_DMX_NEED_JUMP);
		} else if (!fgPBBufExist) {
			DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- Wait PBBuf data!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
			PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);
			MM_RETURN(RET_DMX_OK);
		}

		u4PbbufIdx2 = prPsrCC->u4TxPBBufIdx;

		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);

		fgPBBufExist = FALSE;
		eContinuity = PBBUF_CONTINUITY_UNKNOWN;

		mrRet = PSR_CC_CheckPBBuf(prPsrCC, u8Offset, &fgPBBufExist, &eContinuity, TRUE);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for in ")
						  TEXT
						  ("PSR_CC_CheckPBBuf, pvPsrCC: 0x%p, mrRet: 0x%x!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC, mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		if (PBBUF_UNCONTINUOUS == eContinuity) {
			DMXLOG_DEBUG(TEXT("[PSR] %s line %d PSR_CC_CheckPBBuf ")
						  TEXT("(Uncontinuous), u4TxPbbufIdx: %d!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u4TxPBBufIdx);
			PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
			MM_RETURN(RET_DMX_NEED_JUMP);
		} else if (!fgPBBufExist) {
			DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- Wait PBBuf data!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
			PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);
			MM_RETURN(RET_DMX_OK);
		}

		u4PbbufIdx = prPsrCC->u4TxPBBufIdx;

		mrRet = PSR_CC_GetWaitTxBufSa(prPsrCC, &pvPBBufSa);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
						  TEXT
						  ("PSR_CC_GetWaitTxBufSa, mrRet: 0x%x, PsrCC: 0x%p!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prPsrCC);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
		DMXLOG_DEBUG(TEXT("[PSR] %s line %d PSR_CC_GetWaitTxBufSa, 0x%p!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, pvPBBufSa);

		if (prPsrCC->arPBBuf[u4PbbufIdx2].pcPlayBuffer !=
		    (prPsrCC->arPBBuf[u4PbbufIdx].pcPlayBuffer +
		     prPsrCC->arPBBuf[u4PbbufIdx].u4PlaySize)) {
			/* This case is that: */
			/* u4PbbufIdx's corresponding slot is the end slot of the pbbuf's slots list */
			/* u4PbbufIdx2's corresponding slot is the head slot of the pbbuf's slots list */
			/* Cfa doesn't know the pbbuf is the ring buffer,
			 * so we need to copy the data into one buffer for cfa to check */
			/* need to do SPT_DATA_BUF */
			if ((0 != prDMASD->ptrPrivMemSa) && (prDMASD->u4PrivMemSz < u4Len)) {
				DMX_FreeHwMemory(prDMASD->ptrPrivMemSa);
				prDMASD->ptrPrivMemSa = 0;
			}

			if (0 == prDMASD->ptrPrivMemSa) {
#ifdef __linux__
				DMX_NewHwMemory(u4Len, prDMASD->ptrPrivMemSa);
#else
				DMX_NewHwMemory(u4Len, (void *) (prDMASD->ptrPrivMemSa));
#endif				/* #ifdef __linux__ */
				if (0 == prDMASD->ptrPrivMemSa) {
					DMX_ASSERT(FALSE);
					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);
					PSR_CC_CBSplitter(prPsrCC, E_WAKEUP_ME, NULL);
					MM_RETURN(RET_DMX_NO_MEM);
				}
				prDMASD->u4PrivMemSz = u4Len;
			}

			u4CpyLen = (u32) (prPsrCC->arPBBuf[u4PbbufIdx].u8SrcOffset +
					     prPsrCC->arPBBuf[u4PbbufIdx].u4PlaySize - u8Offset);

			if (u4CpyLen > u4Len) {
				DMX_ASSERT(FALSE);
				DMXLOG_ERROR(
					    TEXT("[PSR] %s line %d -- u4CpyLen(0x%lx) ")
					     TEXT("> PrivMemSz(0x%lx), u8SrcOffset(0x%llx),")
					     TEXT(" u4PlaySize(0x%lx), u8Offset(0x%llx)\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, u4CpyLen,
					    prDMASD->u4PrivMemSz,
					    prPsrCC->arPBBuf[u4PbbufIdx].u8SrcOffset,
					    prPsrCC->arPBBuf[u4PbbufIdx].u4PlaySize, u8Offset);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			dmx_memcpy((void *) prDMASD->ptrPrivMemSa, (void *) pvPBBufSa, u4CpyLen);

			if (u4Len > u4CpyLen) {
				dmx_memcpy((void *) (prDMASD->ptrPrivMemSa + u4CpyLen),
					   (void *) prPsrCC->arPBBuf[u4PbbufIdx2].pcPlayBuffer,
					   (u4Len - u4CpyLen));
			}
			ptrTgtSa = prDMASD->ptrPrivMemSa;
			DMXLOG_DEBUG(TEXT("[PSR] %s line %d PSR_CC_GetWaitTxBufSa, 0x%p!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, ptrTgtSa);
		} else {
			ptrTgtSa = pvPBBufSa;
			DMXLOG_DEBUG(TEXT("[PSR] %s line %d PSR_CC_GetWaitTxBufSa 2, 0x%p!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, ptrTgtSa);
		}
		u4TgtSz = 0;
	}

	prPsrFtr->u8TxCurrOffset = u8Offset + u4Len;
	prPsrCC->u8TxCurrOffset = prPsrFtr->u8TxCurrOffset;
	prPsrCC->eState = CCS_INIT;
	PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);

	if (NULL != prDMASD->pptrTgtHdrPrsSa)
		*(prDMASD->pptrTgtHdrPrsSa) = ptrTgtSa;

	if (NULL != prDMASD->pu4AvailSz)
		*(prDMASD->pu4AvailSz) = u4TgtSz;

	/* callback spliiter notify tx done */
	PSR_CC_CBSplitter(prPsrCC, E_TX_DONE, NULL);

	MM_RETURN(RET_DMX_OK);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_IRQ_Proc */
/* If this parser filter is not enable,  assert */
/* otherwise, if Parser Filter Lock, release HW Res, change Parser
* CC state to CCS_INIT, txstate to TXS_TX_OK, notify splitter TX done */
/* otherwise, if Parser CC state is Abort, release HW Res, change Parser
* CC state to CCS_INIT, txstate to TXS_TX_OK, notify splitter abort done */
/* otherwise: */
/* 1) call PSR_V/A/SPFilter_UpdateESIInfo to create AU, */
/* 2) release HW res */
/* 3) update u8TXCurOffset/u4MemOffset */
/* 4) If tx PbBuf: */
/* A). Change Parser CC txstate to WAIT_PBBUF */
/* B). If Parser CC state is CCS_PAUSE, Notify Splitter Pause Done */
/* C)  else, if tx end, set Parser CC state to CCS_INIT, txstate to
* TXS_TX_OK, notify Splitter TX done */
/* else, if Parser CC state is CCS_TX, notify Splitter Wake up */
/* 5) If tx mem: */
/* A). change Parser CC txstate to TXS_TX_OK */
/* B). if Parser CC state is CCS_PAUSE, notify splitter Pause Done */
/* C). else, if tx end, change parser CC state to CCS_INIT, notify Splitter TX Done */
/* else, tx next sub range memory */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_IRQ_Proc(PSR_FILTER *prPsrFtr)
{
	MRESULT mrRet = RET_DMX_OK;
	PSR_CC *prPsrCC = NULL;

	if (NULL == prPsrFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);

	if (NULL == prPsrCC) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_CC);
	}

	if (0 == (prPsrFtr->u4Flag & FF_ENABLE)) {
		DMXLOG_ERROR(TEXT("[PSR] %s fail for PsrFilter is disable\r\n"),
			    DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (prPsrFtr->u4Flag & FF_LOCK) {
		prPsrCC->fgUseCmdQ = FALSE;
		prPsrCC->fgAUByCmdQEnd = FALSE;

		/* release HW access right */
		PSR_HWRes_Release(prPsrFtr);

		/* change state to init and tx state to tx ok */
		prPsrCC->eState = CCS_INIT;
		PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);

		/* callback spliiter notify tx done */
		PSR_CC_CBSplitter(prPsrCC, E_TX_DONE, NULL);
		MM_RETURN(mrRet);
	}

	if (CCS_ABORT == prPsrCC->eState) {
		/* release HW access right */
		PSR_HWRes_Release(prPsrFtr);

		/* change state to init, Tx State to Tx OK */
		prPsrCC->eState = CCS_INIT;
		PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);
		prPsrCC->u8TxCurrLen = 0;
		prPsrCC->ptrTxCurrSa = 0;

#if ENABLE_DMX_ADVANCED_VER
		prPsrCC->fgInsertHdr = FALSE;
		prPsrCC->u4InsertHdrLen = 0;
		dmx_memset(prPsrCC->au1InsertHdrBuf, 0, sizeof(prPsrCC->au1InsertHdrBuf));
#endif				/* ENABLE_DMX_ADVANCED_VER */

		/* callback spliiter notify abort done */
		PSR_CC_CBSplitter(prPsrCC, E_ABORT_DONE, NULL);
	} else {
		if (DMX_INVALID_UINT8 == prPsrFtr->ucHwDevId) {
			DMX_ASSERT(FALSE);
			DMXLOG_ERROR(
				    TEXT("[PSR] %s line %d fail for prPsrFtr->ucHwDecId")
				     TEXT(" = 0xFF, FtrType: %d\r\nS"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->eType);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		switch (prPsrFtr->eType) {
		case SPT_DATA_V:
#if DMX_PFM_TEST
			DmxPfmStmComposeAUStart(prPsrFtr->eType);
#endif				/* DMX_PFM_TEST */
			mrRet = PSR_VFilter_UpdateESIInfo(prPsrFtr);
#if DMX_PFM_TEST
			DmxPfmStmComposeAUEnd(prPsrFtr->eType);
#endif				/* DMX_PFM_TEST */
			break;
		case SPT_DATA_A:
#if DMX_PFM_TEST
			DmxPfmStmComposeAUStart(prPsrFtr->eType);
#endif				/* DMX_PFM_TEST */
			mrRet = PSR_AFilter_UpdateESIInfo(prPsrFtr);
#if DMX_PFM_TEST
			DmxPfmStmComposeAUEnd(prPsrFtr->eType);
#endif				/* DMX_PFM_TEST */
			break;
		case SPT_DATA_SP:
#if DMX_PFM_TEST
			DmxPfmStmComposeAUStart(prPsrFtr->eType);
#endif				/* DMX_PFM_TEST */
			mrRet = PSR_SPFilter_UpdateESIInfo(prPsrFtr);
#if DMX_PFM_TEST
			DmxPfmStmComposeAUEnd(prPsrFtr->eType);
#endif				/* DMX_PFM_TEST */
			break;
		case SPT_DATA_BUF:
			mrRet = PSR_DMA_UpdateESIInfo(prPsrFtr);
			break;
		case SPT_DATA_SECTION:
			mrRet = PSR_SectionFilter_UpdateESIInfo(prPsrFtr);
			break;
		default:
			break;
		}

		if (DMX_FAILED(mrRet)) {
			if (CCS_PAUSE == prPsrCC->eState) {
				PSR_HWRes_Release(prPsrFtr);
				prPsrCC->eState = CCS_INIT;
				PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);
				/* callback spliiter notify pause done */
				PSR_CC_CBSplitter(prPsrCC, E_PAUSE_DONE, NULL);
			} else {
				if (RET_DMX_NEED_JUMP == mrRet) {
					PSR_HWRes_Release(prPsrFtr);
					/* to make psr pause success */
					PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
					MM_RETURN(mrRet);
				}
			}
			PSR_HWRes_Release(prPsrFtr);
			/* to make psr pause success */
			PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);
			MM_RETURN(mrRet);
		}

		prPsrCC->fgTxMem2Fifo = FALSE;

		/* update current offset */
		if (prPsrFtr->u4Flag & FF_TX_PBBUF) {
			prPsrCC->u8TxCurrOffset += prPsrCC->u8TxCurrLen;
			prPsrFtr->u8TxCurrOffset = prPsrCC->u8TxCurrOffset;
		} else {
			prPsrCC->u4MemOffset += (u32) prPsrCC->u8TxCurrLen;
		}
		prPsrCC->u8TxCurrLen = 0;
		prPsrCC->ptrTxCurrSa = 0;

#if ENABLE_DMX_ADVANCED_VER
		prPsrCC->fgInsertHdr = FALSE;
		prPsrCC->u4InsertHdrLen = 0;
		dmx_memset(prPsrCC->au1InsertHdrBuf, 0, sizeof(prPsrCC->au1InsertHdrBuf));
#endif				/* ENABLE_DMX_ADVANCED_VER */

		DMXLOG_DEBUG(TEXT("[PSR] %s -- u8TxCurOfst: 0x%llx, ")
					  TEXT("u4TxStartOfst: 0x%llx, u8TxLen: 0x%llx\r\n"),
			    DMX_FUNC_NAME, prPsrCC->u8TxCurrOffset, prPsrCC->u8TxStartOffset,
			    prPsrCC->u8TxLen);

		/* release HW access right */
		PSR_HWRes_Release(prPsrFtr);

		if (prPsrFtr->u4Flag & FF_TX_PBBUF) {
			/* change state to wait pbbuf */
			PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);

			/* check pause state */
			if (CCS_PAUSE == prPsrCC->eState) {
				/* callback spliiter notify pause done */
				PSR_CC_CBSplitter(prPsrCC, E_PAUSE_DONE, NULL);
			} else if (prPsrCC->u8TxCurrOffset == (prPsrCC->u8TxStartOffset +
							       prPsrCC->u8TxLen)) {
				prPsrCC->fgUseCmdQ = FALSE;
				prPsrCC->fgAUByCmdQEnd = FALSE;
				/* check whether tx complete */
				/* change state to init and tx state to tx ok */
				prPsrCC->eState = CCS_INIT;
				PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);
				PSR_CC_CBSplitter(prPsrCC, E_TX_DONE, NULL);
			} else if (CCS_TX == prPsrCC->eState) {
				PSR_CC_CBSplitter(prPsrCC, E_WAKEUP_ME, NULL);
			}
		} else {
			/* tx memeory */

			PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);

			/* check pause state */
			if (CCS_PAUSE == prPsrCC->eState) {
				/* callback spliiter notify pause done */
				PSR_CC_CBSplitter(prPsrCC, E_PAUSE_DONE, NULL);
			} else if (prPsrCC->u4SrcMemLen == prPsrCC->u4MemOffset) {
				prPsrCC->fgUseCmdQ = FALSE;
				prPsrCC->fgAUByCmdQEnd = FALSE;

				/* change state to init and tx state to tx ok */
				prPsrCC->eState = CCS_INIT;
				PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);

				/* callback spliiter notify tx done */
				PSR_CC_CBSplitter(prPsrCC, E_TX_DONE, NULL);
			} else {
				/* tx next sub range */
				mrRet = PSR_Filter_TxMem(prPsrFtr);
			}
		}
	}

	MM_RETURN(mrRet);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_SetType */
/* If the Parser Filter's type isn't Ground, if etype is equal to this
* parser filter's type, return OK. otherwise, forbidden operation. */
/* If the parser filter's type is ground, */
/* Set Parser Filter type, Alloc Psr Filter Specific Info according to its type, */
/* Create ESM, and Set ESM Memory, and Register ESM Demuxer CB */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_SetType(PSR_FILTER *prPsrFtr, E_SPT_DATA_TYPE_T eType, u64 u8DecSendBufMask)
{
	MRESULT mrRet = RET_DMX_OK;
	PSR_CC *prPsrCC = NULL;
	ES_TYPE eESType;
	s32 i;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	for (i = 0; i < MAX_FILTER_COUNT; ++i) {
		if (((PSR_FILTER *) prPsrFtr == g_rPsrMan.aprPsrFtrs[i]) &&
		    (((PSR_FILTER *) prPsrFtr)->u4Flag & FF_USED)) {
			i = MAX_FILTER_COUNT << 1;
			break;
		}
	}
	if (i == MAX_FILTER_COUNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);
	if (NULL == prPsrCC) {
		DMXLOG_ERROR(TEXT("[PSR] % fail for no corresponding PsrCC\r\n"),
			    DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	smp_mb();

	if (SPT_DATA_GRD != prPsrFtr->eType) {
		if (eType == prPsrFtr->eType)
			MM_RETURN(RET_DMX_OK);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrFtr->eType = eType;

	if (SPT_DATA_BUF == eType) {
		DMX_NewMemory(sizeof(PSR_DMASD), prPsrFtr->pvFilterSpecific);
		if (NULL == prPsrFtr->pvFilterSpecific) {
			DMXLOG_ERROR(
				    TEXT("[PSR] % fail in alloc PSR_DMASD (no Memory) \r\n"),
				    DMX_FUNC_NAME);
			MM_RETURN(RET_DMX_NO_MEM);
		}
		dmx_memset(prPsrFtr->pvFilterSpecific, 0, sizeof(PSR_DMASD));
		MM_RETURN(RET_DMX_OK);
	}

	if (SPT_DATA_GRD == eType)
		MM_RETURN(RET_DMX_OK);

	switch (eType) {
	case SPT_DATA_V:
		if (NULL == prPsrFtr->pvFilterSpecific) {
			DMX_NewMemory(sizeof(PSR_VFSD), prPsrFtr->pvFilterSpecific);
			if (NULL == prPsrFtr->pvFilterSpecific) {
				DMXLOG_ERROR(
					    TEXT("[PSR] % fail in alloc PSR_VFSD (no Memory) \r\n"),
					    DMX_FUNC_NAME);
				MM_RETURN(RET_DMX_NO_MEM);
			}
			dmx_memset(prPsrFtr->pvFilterSpecific, 0, sizeof(PSR_VFSD));
		}
		break;
	case SPT_DATA_A:
		if (NULL == prPsrFtr->pvFilterSpecific) {
			DMX_NewMemory(sizeof(PSR_AUDFSD), prPsrFtr->pvFilterSpecific);
			if (NULL == prPsrFtr->pvFilterSpecific) {
				DMXLOG_ERROR(
					    TEXT
					    ("[PSR] % fail in alloc PSR_AUDFSD (no Memory) \r\n"),
					    DMX_FUNC_NAME);
				MM_RETURN(RET_DMX_NO_MEM);
			}
			dmx_memset(prPsrFtr->pvFilterSpecific, 0, sizeof(PSR_AUDFSD));
		}
		break;
	case SPT_DATA_SP:
	case SPT_DATA_SECTION:
		if (NULL == prPsrFtr->pvFilterSpecific) {
			DMX_NewMemory(sizeof(PSR_NORMALFSD), prPsrFtr->pvFilterSpecific);
			if (NULL == prPsrFtr->pvFilterSpecific) {
				DMXLOG_ERROR(
					    TEXT
					    ("[PSR] % fail in alloc PSR_NORMALFSD (no Memory) \r\n"),
					    DMX_FUNC_NAME);
				MM_RETURN(RET_DMX_NO_MEM);
			}
			dmx_memset(prPsrFtr->pvFilterSpecific, 0, sizeof(PSR_NORMALFSD));
		}
		break;
	default:
		break;
	}

	if (DMX_INVALID_UINT32 != prPsrFtr->u4ESIH) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s fail for prPsrFtr->u4ESIH has already created\r\n"),
			    DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_ALREADY_EXIST);
	}

	prPsrFtr->u8DecSendBufMask = u8DecSendBufMask;

	/* create ESI */
	mrRet = ESM_Create(prPsrCC->pvSptHdl, prPsrFtr->u4StmType,
			   prPsrFtr->u4StmUID, prPsrFtr->u8DecSendBufMask, &(prPsrFtr->u4ESIH));
	if (DMX_FAILED(mrRet))
		MM_RETURN(mrRet);

	switch (eType) {
	case SPT_DATA_V:
		eESType = ES_V;
		break;

	case SPT_DATA_A:
		eESType = ES_A;
		break;

	case SPT_DATA_SP:
		eESType = ES_SP;
		break;

	case SPT_DATA_SECTION:
		eESType = ES_SECTION;
		break;

	default:
		DMXLOG_ERROR(TEXT("[Psr] %s failed for invalid ESType: 0x%x\r\n"),
			    DMX_FUNC_NAME, eType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	/* Set ES type */
	mrRet = ESM_SetESType(prPsrFtr->u4ESIH, eESType);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s fail in ESM_SetESType \r\n"),
			    DMX_FUNC_NAME);
		MM_RETURN(mrRet);
	}
	/* Register data out callback function */
	ESM_RegistDemuxerCB(prPsrFtr->u4ESIH, PSR_Filter_ESICB, (void *) prPsrFtr);

	MM_RETURN(RET_DMX_OK);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_SetStreamInfo */
/* Set Parser Filte Stream ID */
/* @Param u4StreamUID [IN]  Stream User ID */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_SetStreamInfo(PSR_FILTER *prPsrFtr, u32 u4StreamUID)
{
	s32 i;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	for (i = 0; i < MAX_FILTER_COUNT; ++i) {
		if (((PSR_FILTER *) prPsrFtr == g_rPsrMan.aprPsrFtrs[i]) &&
		    (((PSR_FILTER *) prPsrFtr)->u4Flag & FF_USED)) {
			i = MAX_FILTER_COUNT << 1;
			break;
		}
	}
	if (i == MAX_FILTER_COUNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrFtr->u4StmUID = u4StreamUID;

	MM_RETURN(RET_DMX_OK);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_Flush */
/* If Parser CC state is Txing, Wait IRQ Proc or Abort,
* and Parser CC's active filter is this filter, error(assert) */
/* If Parser Filter is enable, this operation is forbidden */
/* If this filter is video filter, reset its private data, and set HdrPts to be invalid */
/* Clear ESM Fifo */
/* Reset Parser Filter, if this parser filter is Parser
* CC's active filter, update txcuroffset to be tx end offset */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_Flush(PSR_FILTER *prPsrFtr)
{
	PSR_CC *prPsrCC = NULL;
	s32 i;


	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	for (i = 0; i < MAX_FILTER_COUNT; ++i) {
		if (((PSR_FILTER *) prPsrFtr == g_rPsrMan.aprPsrFtrs[i]) &&
		    (((PSR_FILTER *) prPsrFtr)->u4Flag & FF_USED) &&
		    (((PSR_FILTER *) prPsrFtr)->u4ESIH != DMX_INVALID_UINT32)) {
			i = MAX_FILTER_COUNT << 1;
			break;
		}
	}
	if (i == MAX_FILTER_COUNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrCC = (PSR_CC *) prPsrFtr->pvPsrCC;
	if (prPsrCC == NULL)
		MM_RETURN(RET_DMX_NO_CC);

	if ((TXS_TXING == prPsrCC->eTxState) ||
	    (TXS_WAIT_IRQ_PROC == prPsrCC->eTxState) ||
	    (CCS_ABORT == prPsrCC->eState)) {
		if (prPsrCC->pvActFilter == prPsrFtr)
			MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (prPsrFtr->u4Flag & FF_ENABLE)
		MM_RETURN(RET_DMX_ERR_STATE);

	if (SPT_DATA_V == prPsrFtr->eType) {
		PSR_VFSD *prVFSD = (PSR_VFSD *) (prPsrFtr->pvFilterSpecific);

		if (NULL != prVFSD) {
			prVFSD->fgUseRealWrIdx = FALSE;
			prVFSD->u4RealWrIdx = 0;
			prVFSD->u4DummyAURealWrIdx = 0;
			prVFSD->fgUseDummyAURealWrIdx = FALSE;
			prVFSD->fgHasSetPics = FALSE;
			prVFSD->fgCreNoHdrAUWaitPkt = FALSE;
			prVFSD->u4VType = 0;
			dmx_memset(&(prVFSD->rPicDetResult), 0, sizeof(prVFSD->rPicDetResult));
		}
	}

	ESM_FifoClear(prPsrFtr->u4ESIH);

	prPsrFtr->u8HdrPTS = INVALID_TIMESTAMP;
	prPsrFtr->u8LastPTS = INVALID_TIMESTAMP;
#ifdef MM_SUPPORT_DIVXHT31
	prPsrFtr->u8PrevPTS = INVALID_TIMESTAMP;
	prPsrFtr->u81stPTS = INVALID_TIMESTAMP;
#endif				/* MM_SUPPORT_DIVXHT31 */
	prPsrFtr->u4AUCntFromIFrm = 0;
	prPsrFtr->u4IFrmCnt = 0;

	prPsrFtr->u8TxCurrOffset = 0;
	prPsrFtr->ptrBkWrPtr = 0;
	prPsrFtr->fgAUCtrlByLen = FALSE;
	prPsrFtr->fgAUCtrlByEnd = FALSE;
	prPsrFtr->u8TotalAULen = 0;
	prPsrFtr->u8CurAULen = 0;
	prPsrFtr->u8WMDRMTxLen = 0;
	prPsrFtr->fgAUEnd = FALSE;

	prPsrFtr->fgFirstAUInRng = TRUE;

	DMXLOG_TRACE(
		TEXT("[PSR] %s -- prPsrCC->eTxState: %d, pvActFilter: %p, prPsrFtr: %p\r\n"),
		DMX_FUNC_NAME, prPsrCC->eTxState,
		prPsrCC->pvActFilter, prPsrFtr);
	
	if (((TXS_WAIT_FIFO == prPsrCC->eTxState) ||
			(TXS_WAIT_VFIFO_PTS_THRESHOLD == prPsrCC->eTxState)) &&
			(prPsrCC->pvActFilter == (void *)prPsrFtr)) {
		prPsrCC->eTxState = TXS_FIFO_OK;
		PSR_CC_CBSplitter(prPsrCC, E_WAKEUP_ME, NULL);
	}

	if ((SPT_DATA_BUF != prPsrFtr->eType) && (prPsrFtr == prPsrCC->pvActFilter)) {
		/* Ignore partial uncompleted tx data */
		if (prPsrCC->u8TxCurrOffset != (prPsrCC->u8TxStartOffset + prPsrCC->u8TxLen)) {
			DMXLOG_DEBUG(
				    TEXT("[PSR] %s -- curofst:0x%llx, sa:0x%llx, len:0x%llx\r\n"),
				    DMX_FUNC_NAME, prPsrCC->u8TxCurrOffset,
				    prPsrCC->u8TxStartOffset, prPsrCC->u8TxLen);

			prPsrCC->u8TxCurrOffset = prPsrCC->u8TxStartOffset + prPsrCC->u8TxLen;
		}
	}

	DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- ESIH: 0x%x, Type: %s\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH,
		    ((prPsrFtr->eType <
		      MAX_SPT_DATA_TYPE_CNT) ? g_aszSptDataTypeName[prPsrFtr->eType] :
		     TEXT("UNKNOWN")));

	MM_RETURN(RET_DMX_OK);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_LockFtr */
/* Lock/Unlock Parser Filter */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_LockFtr(void *pvPsrFtr, bool fgLock)
{
	PSR_FILTER *pPstFtr = (PSR_FILTER *) pvPsrFtr;
	s32 i;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == pPstFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	for (i = 0; i < MAX_FILTER_COUNT; ++i) {
		if (((PSR_FILTER *) pPstFtr == g_rPsrMan.aprPsrFtrs[i]) &&
		    (((PSR_FILTER *) pPstFtr)->u4Flag & FF_USED)) {
			i = MAX_FILTER_COUNT << 1;
			break;
		}
	}
	if (i == MAX_FILTER_COUNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (fgLock) {
		if (pPstFtr->u4Flag & FF_LOCK)
			MM_RETURN(RET_DMX_OK);
		pPstFtr->u4Flag |= FF_LOCK;
	} else {
		if (0 == (pPstFtr->u4Flag & FF_LOCK))
			MM_RETURN(RET_DMX_OK);

		pPstFtr->u4Flag &= (~FF_LOCK);
	}

	MM_RETURN(RET_DMX_OK);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_IsFifoFull */
/* check whether the parser filter 's ESM fifo is
* full(free space < Fifo reserved size) or AU table is full */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_IsFifoFull(PSR_FILTER *prPsrFtr, bool *pfgFull, u64 u8TxLen)
{
	MRESULT mrRet = RET_DMX_OK;
	u32 u4Len = (u32) u8TxLen;
	bool fgFifoFull = FALSE, fgAUTableFull = FALSE;

	if (NULL == pfgFull)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	*pfgFull = FALSE;

	mrRet = PSR_Filter_IsESBufFull(prPsrFtr, u4Len, &fgFifoFull);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s fail in PSR_Filter_IsESBufFull, mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, mrRet);
		MM_RETURN(mrRet);
	}

	prPsrFtr->u4AUExtCnt = 1;
	mrRet = PSR_Filter_IsAUTableFull(prPsrFtr, &fgAUTableFull);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s fail in PSR_Filter_IsAUTableFull, mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, mrRet);
		MM_RETURN(mrRet);
	}
	prPsrFtr->u4AUExtCnt = 0;

	if (fgFifoFull)
		*pfgFull = TRUE;
	else if (fgAUTableFull)
		*pfgFull = TRUE;

	MM_RETURN(RET_DMX_OK);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_IsFifoFull */
/* check whether the parser filter 's ESM fifo is
* full(free space < Fifo reserved size) or AU table is full */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_SwitchFifo(PSR_FILTER *prPsrFtr)
{
	MRESULT mrRet = RET_DMX_OK;
	u32 u4Codec = 0;
	bool fgUseSWFifo = FALSE;

	if (NULL == prPsrFtr)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	switch (prPsrFtr->eType) {
	case SPT_DATA_V:
		{
			PSR_VFSD *prVFSD = (PSR_VFSD *) (prPsrFtr->pvFilterSpecific);

			if (NULL == prVFSD) {
				DMXLOG_ERROR(
					    TEXT
					    ("[PSR] %s fail for PsrFtr(Video)'s prVFSD == NULL\r\n"),
					    DMX_FUNC_NAME);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			u4Codec = (u32) (prVFSD->eVCodeC);
			DMXLOG_TRACE(
				    TEXT("[PSR] %s -- VFSD->eVCodec: %d, u8DecSendBufMask: 0x%llx\r\n"),
				    DMX_FUNC_NAME, u4Codec, prPsrFtr->u8DecSendBufMask);

			fgUseSWFifo = FALSE;
			if ((DMX_INVALID_UINT32 != u4Codec) &&
			    ((u64) 0 != (prPsrFtr->u8DecSendBufMask & (((u64) 1) << u4Codec)))) {
				fgUseSWFifo = TRUE;
			}
		}
		break;
	case SPT_DATA_A:
		{
			PSR_AUDFSD *prAFSD = (PSR_AUDFSD *) (prPsrFtr->pvFilterSpecific);

			if (NULL == prAFSD) {
				DMXLOG_ERROR(
					    TEXT
					    ("[PSR] %s fail for PsrFtr(Audio)'s prAFSD == NULL\r\n"),
					    DMX_FUNC_NAME);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			u4Codec = (u32) (prAFSD->eAudFmtType);

			DMXLOG_TRACE(
				    TEXT
				    ("[PSR] %s -- AFSD->eAudFmtType: %d, u8DecSendBufMask: 0x%llx\r\n"),
				    DMX_FUNC_NAME, u4Codec, prPsrFtr->u8DecSendBufMask);

			fgUseSWFifo = FALSE;
			if ((DMX_INVALID_UINT32 != u4Codec) &&
			    ((u64) 0 != (prPsrFtr->u8DecSendBufMask & (((u64) 1) << u4Codec)))) {
				fgUseSWFifo = TRUE;
			}
		}
		break;
	default:
		break;
	}

	mrRet = ESM_FifoSwitch(prPsrFtr->u4ESIH, fgUseSWFifo);

	
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s fail in ESM_FifoSwitch(ESIH: 0x%x)\r\n"),
			    DMX_FUNC_NAME, prPsrFtr->u4ESIH);
		MM_RETURN(mrRet);
	}
	/* prPsrFtr->u4ESFifoSize = u4Size; */
	/* reget FIFO size from ESM, because ESM will adjust FIFO size for Decoder alignment request */
	mrRet = ESM_FifoGetSA(prPsrFtr->u4ESIH, &(prPsrFtr->ptrESFifoSa));
	if (DMX_FAILED(mrRet))
		MM_RETURN(mrRet);

	mrRet = ESM_FifoGetEA(prPsrFtr->u4ESIH, &(prPsrFtr->ptrESFifoEa));
	if (DMX_FAILED(mrRet))
		MM_RETURN(mrRet);

	prPsrFtr->u4ESFifoSize = prPsrFtr->ptrESFifoEa - prPsrFtr->ptrESFifoSa;

	DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- ESIH: 0x%x, ")
				  TEXT("Type: %s, Fifo(Sa: 0x%x, Ea: 0x%x, Sz: 0x%x)\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH,
		    ((prPsrFtr->eType <
		      MAX_SPT_DATA_TYPE_CNT) ? g_aszSptDataTypeName[prPsrFtr->eType] :
		     TEXT("UNKNOWN")), prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa,
		    prPsrFtr->u4ESFifoSize);

	MM_RETURN(RET_DMX_OK);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_GetCurrentOffset */
/* Get Parser Filter's u8TxCurrOffset */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_GetCurrentOffset(PSR_FILTER *prPsrFtr, u64 *pu8Offset)
{
	if ((NULL == pu8Offset) || (NULL == prPsrFtr)) {
		DMXLOG_ERROR(TEXT("[PSR] %s fail for invalid args\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*pu8Offset = prPsrFtr->u8TxCurrOffset;

	MM_RETURN(RET_DMX_OK);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_SetVideoCodeC */
/* Set Video Parser Filter's private data's codec */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_SetCodeC(PSR_FILTER *prPsrFtr, u32 u4Codec)
{
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prPsrFtr) {
		DMXLOG_ERROR(TEXT("[PSR] %s fail for invalid args(u4Codec: %d)\r\n"),
			    DMX_FUNC_NAME, u4Codec);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	switch (prPsrFtr->eType) {
	case SPT_DATA_V:
		{
			PSR_VFSD *prVFSD = (PSR_VFSD *) (prPsrFtr->pvFilterSpecific);

			if (NULL == prVFSD) {
				DMXLOG_ERROR(
					    TEXT
					    ("[PSR] %s fail for PsrFtr(Video)'s prVFSD == NULL\r\n"),
					    DMX_FUNC_NAME);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (prVFSD->eVCodeC != (VCodeC) u4Codec) {
				prVFSD->eVCodeC = (VCodeC) u4Codec;
				mrRet = PSR_Filter_SwitchFifo(prPsrFtr);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
								  TEXT
								  ("PSR_Filter_SwitchFifo(prPsrFtr: 0x%x, Video)\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr);
					MM_RETURN(mrRet);
				}
			}
		}
		break;
	case SPT_DATA_A:
		{
			PSR_AUDFSD *prAFSD = (PSR_AUDFSD *) (prPsrFtr->pvFilterSpecific);

			if (NULL == prAFSD) {
				DMXLOG_ERROR(
					    TEXT
					    ("[PSR] %s fail for PsrFtr(Audio)'s prAFSD == NULL\r\n"),
					    DMX_FUNC_NAME);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (prAFSD->eAudFmtType != (AUD_DRV_FMT_T) u4Codec) {
				prAFSD->eAudFmtType = (AUD_DRV_FMT_T) u4Codec;
				mrRet = PSR_Filter_SwitchFifo(prPsrFtr);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
								  TEXT
								  ("PSR_Filter_SwitchFifo(prPsrFtr: 0x%x, Audio)\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr);
					MM_RETURN(mrRet);
				}
			}
		}
		break;
	default:
		break;
	}

	MM_RETURN(RET_DMX_OK);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_GetCodeC */
/* Get Parser Filter's private data's codec */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_GetCodeC(PSR_FILTER *prPsrFtr, u32 *pu4Codec)
{
	if ((NULL == prPsrFtr) || (NULL == pu4Codec)) {
		DMXLOG_ERROR(TEXT("[PSR] %s fail for invalid args)\r\n"),
			    DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*pu4Codec = DMX_INVALID_UINT32;

	switch (prPsrFtr->eType) {
	case SPT_DATA_V:
		{
			PSR_VFSD *prVFSD = (PSR_VFSD *) (prPsrFtr->pvFilterSpecific);

			if (NULL == prVFSD) {
				DMXLOG_ERROR(
					    TEXT
					    ("[PSR] %s fail for PsrFtr(Video)'s prVFSD == NULL\r\n"),
					    DMX_FUNC_NAME);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			if (VC_UNKNOW != prVFSD->eVCodeC)
				*pu4Codec = (u32) (prVFSD->eVCodeC);
		}
		break;
	case SPT_DATA_A:
		{
			PSR_AUDFSD *prAFSD = (PSR_AUDFSD *) (prPsrFtr->pvFilterSpecific);

			if (NULL == prAFSD) {
				DMXLOG_ERROR(
					    TEXT
					    ("[PSR] %s fail for PsrFtr(Audio)'s prAFSD == NULL\r\n"),
					    DMX_FUNC_NAME);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			if (AUD_DRV_FMT_UNKNOWN != prAFSD->eAudFmtType)
				*pu4Codec = (u32) (prAFSD->eAudFmtType);
		}
		break;
	default:
		break;
	}

	MM_RETURN(RET_DMX_OK);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_TxPBBuf2FifoWithAUCtrl */
/* 1) If Parser CC's state isn't CCS_INIT or isn't tx OK, Change Parser CC's state to CCS_INIT */
/* 2) If Parser CC's state is CCS_INIT or TX_OK state, do the following tasks: */
/* A) set Parser CC's tx start offset txlen, txcurrent offset, */
/* B) set Parser CC's active filter to be this */
/* C) set Parser CC's txstate to be WAIT PBBUF, state to be CCS_TX */
/* D) If using Cmd Queue, copy command queue entrys into parser cc,
* and set parse cc's cmdqueue info and prevcmdqueue info */
/* E) Set TX_PBBUF and TX_TO_FIFO flag */
/* F) If need create AU, set fgAUCtrlByLen and total tx AU len */
/* H) If parser filter is disable, call tx to ground, otherwise,
* call tx pbbuf function to tx data from pbbuf to fifo */
/* @Param  prPsrFtr          [IN] Filter handle */
/* @Param  u8Offset        [IN] Start offset, unit by content, maybe LBA or bytes */
/* @Param  prAUCtrlInfo  [IN] AU Ctrl Info */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_TxPBBuf2FifoWithAUCtrl(PSR_FILTER *prPsrFtr, u64 u8Offset,
					  PSR_AUCtrlInfo *prAUCtrlInfo)
{
	PSR_CC *prPsrCC;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prPsrFtr) || (NULL == prAUCtrlInfo)) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s fail for PsrFilter or prAUCtrlInfo is NULL!!\r\n"),
			    DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (0 == prAUCtrlInfo->u8Len) {
		DMXLOG_ERROR(TEXT("[PSR] %s -- error for tx len is 0!!\r\n"),
			    DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if ((prAUCtrlInfo->fgCreateAU) && (prAUCtrlInfo->u8TotalAULen < prAUCtrlInfo->u8Len)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for total len:0x%llx ")
					  TEXT("is little than len 0x%llx, line: %d!!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prAUCtrlInfo->u8TotalAULen,
			    prAUCtrlInfo->u8Len);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	/* check CC status and Tx state */
	prPsrCC = prPsrFtr->pvPsrCC;
	if (NULL == prPsrCC) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s line %d fail for invalid PsrCC handle!!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_NO_CC);
	}

	PSR_CC_LOCK(prPsrCC->rLock);

	if ((prPsrCC->eState != CCS_INIT) ||
	    ((prPsrCC->eState != CCS_TX) && (prPsrCC->eTxState != TXS_TX_OK))) {
		prPsrCC->eState = CCS_INIT;
	}

	if ((CCS_INIT == prPsrCC->eState) ||
	    ((CCS_TX == prPsrCC->eState) && (TXS_TX_OK == prPsrCC->eTxState))) {
		PSR_VFSD *prVFSD = (PSR_VFSD *) prPsrFtr->pvFilterSpecific;

		prPsrCC->u8TxStartOffset = u8Offset;
		prPsrCC->u8TxLen = prAUCtrlInfo->u8Len;
		prPsrCC->u8TxCurrOffset = u8Offset;
		prPsrCC->pvActFilter = (void *)prPsrFtr;

		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);
		prPsrCC->eState = CCS_TX;

		prPsrCC->fgUseCmdQ = prAUCtrlInfo->fgUseCmdQ;

#if ENABLE_DMX_ADVANCED_VER
		prPsrCC->fgInsertHdr = prAUCtrlInfo->fgInsertHdr;
		prPsrCC->u4InsertHdrLen = prAUCtrlInfo->u4InsertHdrLen;
		if (NULL != prAUCtrlInfo->pu1InsertHdrBuf) {
			dmx_memcpy(prPsrCC->au1InsertHdrBuf, prAUCtrlInfo->pu1InsertHdrBuf,
				   prPsrCC->u4InsertHdrLen);
		}
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_INSTBS,
			TEXT("[SPT] %s -- InsertHdr(%s), HdrLen(%d), HdrBuf(0x%x)\r\n"),
			DMX_FUNC_NAME, ((prPsrCC->fgInsertHdr) ? TEXT("TRUE") : TEXT("FALSE")),
			prPsrCC->u4InsertHdrLen, prPsrCC->au1InsertHdrBuf);
#endif				/* ENABLE_DMX_ADVANCED_VER */

		if (prPsrCC->fgUseCmdQ) {
			DMX_CMDQ_TX_ENTRY_T *prTxEntry = NULL;
			PSR_CMDQ_TX_INF *prPsrCmdQTxInf = NULL;

			prPsrCC->fgAUByCmdQEnd = prAUCtrlInfo->fgAUByCmdQEnd;

			if (0 == prPsrCC->pvCmdQTxEntryBuffer) {
				DMXLOG_ERROR(
					    TEXT
					    ("[PSR] %s line %d fail for CMDQ entry is NULL\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO);
				PSR_CC_UNLOCK(prPsrCC->rLock);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			if (0 == prAUCtrlInfo->u2TxEntryCnt) {
				DMXLOG_ERROR(
					    TEXT
					    ("[PSR] %s line %d fail for CMDQ entry cnt == 0 \r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO);
				PSR_CC_UNLOCK(prPsrCC->rLock);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			if (prAUCtrlInfo->u2TxEntryCnt > DMX_MAX_TX_CNT_FOR_CMD_Q) {
				DMXLOG_ERROR(
					    TEXT("[PSR] %s line %d fail for CMDQ entry ")
					     TEXT("cnt(%d) > DMX_MAX_TX_CNT_FOR_CMD_Q(%d)\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, prAUCtrlInfo->u2TxEntryCnt,
					    DMX_MAX_TX_CNT_FOR_CMD_Q);
				PSR_CC_UNLOCK(prPsrCC->rLock);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			prPsrCC->u2TxEntryCnt = prAUCtrlInfo->u2TxEntryCnt;

			dmx_memcpy((void *) prPsrCC->pvCmdQTxEntryBuffer,
				   (void *) (prAUCtrlInfo->parCmdQTxEntry),
				   (sizeof(DMX_CMDQ_TX_ENTRY_T) * prAUCtrlInfo->u2TxEntryCnt));

			prPsrCC->fgChkedAndWaitTx = FALSE;
			prPsrCC->fgCurTotalCmdQTxStarted = FALSE;

			/* Get 1st entry */
			prTxEntry = (DMX_CMDQ_TX_ENTRY_T *) (prPsrCC->pvCmdQTxEntryBuffer);

			prPsrCmdQTxInf = &(prPsrCC->rCmdQTxInf);
			mm_memset(prPsrCmdQTxInf, 0, sizeof(prPsrCC->rCmdQTxInf));
			prPsrCmdQTxInf->u8RmnTotalRealTxLen = prAUCtrlInfo->u8RealTxLen;
			prPsrCmdQTxInf->u2CurTxRngSIdx = 0;
			prPsrCmdQTxInf->u4CurTxRngSIdxOfst = 0;
			prPsrCmdQTxInf->u4CurTxRngSIdxLen = 0;
			prPsrCmdQTxInf->u2CurTxRngEIdx = 0;	/* No any tx currenttly, so... */
			prPsrCmdQTxInf->u4CurTxRngEIdxOfst = 0;
			prPsrCmdQTxInf->u4CurTxRngEIdxLen = 0;
			prPsrCmdQTxInf->u4CurTxRngEIdxRmnLen = prTxEntry->u4TxLen;

			prPsrCmdQTxInf = &(prPsrCC->rCmdQPrevTxInf);
			mm_memset(prPsrCmdQTxInf, 0, sizeof(prPsrCC->rCmdQPrevTxInf));
			prPsrCmdQTxInf->u8RmnTotalRealTxLen = prAUCtrlInfo->u8RealTxLen;
			prPsrCmdQTxInf->u2CurTxRngSIdx = 0;
			prPsrCmdQTxInf->u4CurTxRngSIdxOfst = 0;
			prPsrCmdQTxInf->u4CurTxRngSIdxLen = 0;
			prPsrCmdQTxInf->u2CurTxRngEIdx = 0;
			prPsrCmdQTxInf->u4CurTxRngEIdxOfst = 0;
			prPsrCmdQTxInf->u4CurTxRngEIdxLen = 0;
			prPsrCmdQTxInf->u4CurTxRngEIdxRmnLen = prTxEntry->u4TxLen;
		}

		prPsrFtr->u4Flag |= (FF_TX_PBBUF | FF_TX_TO_FIFO);

		if (prAUCtrlInfo->fgCreateAU) {
			prPsrFtr->fgAUCtrlByLen = TRUE;
			prPsrFtr->fgAUCtrlByEnd = FALSE;
			prPsrFtr->u8TotalAULen = prAUCtrlInfo->u8TotalAULen;
			prPsrFtr->u8CurAULen = 0;
			prPsrFtr->u8WMDRMTxLen = 0;

			if (SPT_DATA_V == prPsrFtr->eType) {
				if (PSR_IsNonHdrVideoType(prVFSD->eVCodeC)
#if ENABLE_DMX_ADVANCED_VER
				    && (!prPsrCC->fgUseCmdQ)
#endif				/* ENABLE_DMX_ADVANCED_VER */
				    ) {
					DMXLOG_DEBUG(
						    TEXT("[PSR] %s line %d -- CreateAU=TRUE ")
						     TEXT("prVFSD->u4VType: 0x%x, prAUCtrlInfo->u4Vtype: 0x%x!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO, prVFSD->u4VType,
						    prAUCtrlInfo->u4Vtype);
					prVFSD->u4VType = prAUCtrlInfo->u4Vtype;
				}
			}
		} else {
			if (SPT_DATA_V == prPsrFtr->eType) {
				if (PSR_IsNonHdrVideoType(prVFSD->eVCodeC)
#if ENABLE_DMX_ADVANCED_VER
				    && (!prPsrCC->fgUseCmdQ)
#endif				/* ENABLE_DMX_ADVANCED_VER */
				    ) {
					DMXLOG_DEBUG(
						    TEXT("[PSR] %s line %d -- CreateAU=FALSE ")
						     TEXT("prVFSD->u4VType: 0x%x, prAUCtrlInfo->u4Vtype: 0x%x!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO, prVFSD->u4VType,
						    prAUCtrlInfo->u4Vtype);
					if (0 == prVFSD->u4VType)
						prVFSD->u4VType = prAUCtrlInfo->u4Vtype;
				}

				prVFSD->fgQueryWVC1Mode = FALSE;
				if (prAUCtrlInfo->fgQueryWVC1Mode)
					prVFSD->fgQueryWVC1Mode = TRUE;
			}
		}

		smp_mb();

		if ((0 == (prPsrFtr->u4Flag & FF_ENABLE)) || (SPT_DATA_GRD == prPsrFtr->eType))
			mrRet = PSR_Filter_TxToGround(prPsrFtr);
		else
			mrRet = PSR_Filter_TxPbbuf(prPsrFtr, u8Offset);
	} else {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d -- fail for error ")
					  TEXT("PsrCC state --> State:0x%x, txSt:0x%x!!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->eState, prPsrCC->eTxState);
		mrRet = RET_DMX_ERR_STATE;
	}

	PSR_CC_UNLOCK(prPsrCC->rLock);

	MM_RETURN(mrRet);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_TxPBBuf2FifoWithAUEnd */
/* If Parser CC's state is CCS_INIT or TX_OK state, do the following tasks: */
/* A) set Parser CC's tx start offset txlen, txcurrent offset, */
/* B) set Parser CC's active filter to be this */
/* C) Set TX_PBBUF and TX_TO_FIFO flag */
/* D) If need create AU, set fgAUCtrlByEnd */
/* E) set fgAUEnd flag */
/* F) If parser filter is disable, call tx to ground, */
/* otherwise, if fgAuEnd and data type is Audio, */
/* otherwise, call tx pbbuf function to tx data from pbbuf to fifo */
/* @Param  prPsrFtr          [IN] Filter handle */
/* @Param  u8Offset        [IN] Start offset, unit by content, maybe LBA or bytes */
/* @Param  u8Len            [IN] Length, unit: bytes */
/* @Param  u4Codec        [IN] Codec */
/* @Param  prExtInf         [IN] Extra information notify by CFA */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_TxPBBuf2FifoWithAUEnd(PSR_FILTER *prPsrFtr, u64 u8Offset,
					 u64 u8Len, EXT_INFO_T *prExtInf)
{
	PSR_CC *prPsrCC;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prPsrFtr) || (NULL == prExtInf)) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s fail for PsrFilter or prExtInf is NULL!!\r\n"),
			    DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (SPT_DATA_GRD != prPsrFtr->eType) {
		s32 i;

		if (NULL == prPsrFtr) {
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}

		for (i = 0; i < MAX_FILTER_COUNT; ++i) {
			if (((PSR_FILTER *) prPsrFtr == g_rPsrMan.aprPsrFtrs[i]) &&
			    (((PSR_FILTER *) prPsrFtr)->u4Flag & FF_USED) &&
			    (((PSR_FILTER *) prPsrFtr)->u4ESIH != DMX_INVALID_UINT32)) {
				i = MAX_FILTER_COUNT << 1;
				break;
			}
		}
		if (i == MAX_FILTER_COUNT) {
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}

	}
	/* only allow the last AU_End notify is a virtual transfer. */
	if ((0 == u8Len) && (!prExtInf->fgAUEnd)) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s fail for u8Len is 0 but fgAUEnd is FALSE!!\r\n"),
			    DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	/* check CC status and Tx state */
	prPsrCC = prPsrFtr->pvPsrCC;
	if (NULL == prPsrCC) {
		DMXLOG_ERROR(TEXT("[PSR] %s fail for PsrCC is NULL!!\r\n"),
			    DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_NO_CC);
	}

	PSR_CC_LOCK(prPsrCC->rLock);

	DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- PsrCC State, st:0x%x, txSt:0x%x!!\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->eState, prPsrCC->eTxState);

	if ((CCS_INIT == prPsrCC->eState) ||
	    ((CCS_TX == prPsrCC->eState) && (TXS_TX_OK == prPsrCC->eTxState))) {
		prPsrCC->u8TxStartOffset = u8Offset;
		prPsrCC->u8TxLen = u8Len;
		prPsrCC->u8TxCurrOffset = u8Offset;
		prPsrCC->pvActFilter = prPsrFtr;

		prPsrFtr->u4Flag |= (FF_TX_PBBUF | FF_TX_TO_FIFO);
		if (prExtInf->fgCreateAU) {
			prPsrFtr->fgAUCtrlByEnd = TRUE;
			prPsrFtr->fgAUCtrlByLen = FALSE;
			prPsrFtr->u8TotalAULen = 0;
			prPsrFtr->u8CurAULen = 0;
			prPsrFtr->u8WMDRMTxLen = 0;
			DMXLOG_DEBUG(
				    TEXT("[PSR] %s line %d -- fgCreateAU == TRUE!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);

			if (SPT_DATA_V == prPsrFtr->eType) {
				PSR_VFSD *prVFSD = (PSR_VFSD *) prPsrFtr->pvFilterSpecific;

				if (PSR_IsNonHdrVideoType(prVFSD->eVCodeC)
#if ENABLE_DMX_ADVANCED_VER
				    && (!prPsrCC->fgUseCmdQ)
#endif				/* ENABLE_DMX_ADVANCED_VER */
				    ) {
					prVFSD->u4VType = prExtInf->u4Vtype;
				}
			}
		}

		prPsrFtr->fgAUEnd = prExtInf->fgAUEnd;

		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);

		prPsrCC->eState = CCS_TX;
		smp_mb();

		DMXLOG_DEBUG(
			    TEXT("[PSR] %s line %d -- fgAUEnd == %d, PsrFtr's eType = %d!!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, ((prPsrFtr->fgAUEnd) ? 1 : 0),
			    prPsrFtr->eType);

		if ((0 == (prPsrFtr->u4Flag & FF_ENABLE)) || (SPT_DATA_GRD == prPsrFtr->eType)) {
			mrRet = PSR_Filter_TxToGround(prPsrFtr);
		} else if (prPsrFtr->fgAUEnd && (SPT_DATA_A == prPsrFtr->eType)) {
			if (prPsrCC->fgCfaPrsEnd)
				MM_RETURN(RET_DMX_UNEXPECT);

			if (!PSR_HWRes_Obtain(prPsrFtr)) {
				PSR_CC_UNLOCK(prPsrCC->rLock);
				MM_RETURN(RET_DMX_OK);
			}
			mrRet = PSR_AFilter_UpdateESIInfo(prPsrFtr);

			/* release HW access right */
			PSR_HWRes_Release(prPsrFtr);
		} else {
			mrRet = PSR_Filter_TxPbbuf(prPsrFtr, u8Offset);
		}
	} else {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for error PsrCC State,")
					  TEXT(" st:0x%x, txSt:0x%x, line: %d!!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->eState, prPsrCC->eTxState);
		mrRet = RET_DMX_ERR_STATE;
	}

	PSR_CC_UNLOCK(prPsrCC->rLock);

	MM_RETURN(mrRet);
}


/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_TxMemory2Fifo */
/* Tx Memory data into FIFO */
/* 1) If Parser CC's state isn't CCS_INIT or isn't tx OK, Change Parser CC's state to CCS_INIT */
/* 2) If Parser CC's state is CCS_INIT or TX_OK state, do the following tasks: */
/* A) Set Source Memory SA, Len, Offset, */
/* B) change Parser CC's Active filter to be this, */
/* C) Set TX_TO_FIFO flag, parser cc's state to be CCS_TX */
/* D) If parser filter is disable or ground type, call tx to ground, otherwise,
* call tx mem function to tx data from memory to fifo */
/* @Param prPsrFtr   [IN] Filter handle */
/* @Param pvSrcSa [IN] Source memory start address */
/* @Param u8Len     [IN] length, unit: bytes */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_TxMemory2Fifo(PSR_FILTER *prPsrFtr,
	void *pvSrcSa, u32 u4Len)
{
	MRESULT mrRet = RET_DMX_OK;
	PSR_CC *prPsrCC;
	s32 i;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	for (i = 0; i < MAX_FILTER_COUNT; ++i) {
		if (((PSR_FILTER *) prPsrFtr == g_rPsrMan.aprPsrFtrs[i]) &&
		    (((PSR_FILTER *) prPsrFtr)->u4Flag & FF_USED)) {
			i = MAX_FILTER_COUNT << 1;
			break;
		}
	}
	if (i == MAX_FILTER_COUNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrCC = prPsrFtr->pvPsrCC;
	if (NULL == prPsrCC) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for PsrCC is NULL!!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_NO_CC);
	}

	PSR_CC_LOCK(prPsrCC->rLock);

	if ((prPsrCC->eState != CCS_INIT) ||
	    ((prPsrCC->eState != CCS_TX) && (prPsrCC->eTxState != TXS_TX_OK))) {
		DMXLOG_TRACE(TEXT("[PSR] %s line %d change PsrCC state to be ")
					  TEXT("CCS_INIT, eState: 0x%x, eTxState: 0x%x!!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->eState, prPsrCC->eTxState);
		prPsrCC->eState = CCS_INIT;
	}

	if ((CCS_INIT == prPsrCC->eState) ||
	    ((CCS_TX == prPsrCC->eState) && (TXS_TX_OK == prPsrCC->eTxState))) {
		/* backup information */
		prPsrCC->ptrSrcMemSa = (uintptr_t)pvSrcSa;
		prPsrCC->u4SrcMemLen = u4Len;
		prPsrCC->u4MemOffset = 0;
		prPsrFtr->u4Flag &= (~FF_TX_PBBUF);
		prPsrFtr->u4Flag |= FF_TX_TO_FIFO;
		prPsrFtr->fgAUCtrlByLen = FALSE;
		prPsrCC->pvActFilter = prPsrFtr;
		prPsrCC->eState = CCS_TX;

		prPsrCC->fgTxMem2Fifo = TRUE;

		smp_mb();

		if ((0 == (prPsrFtr->u4Flag & FF_ENABLE)) || (SPT_DATA_GRD == prPsrFtr->eType))
			mrRet = PSR_Filter_TxToGround(prPsrFtr);
		else
			mrRet = PSR_Filter_TxMem(prPsrFtr);
	} else {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for error PsrCC state, ")
					  TEXT("st:0x%x, txSt:0x%x, line: %d!!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->eState, prPsrCC->eTxState);
		mrRet = RET_DMX_ERR_STATE;
	}

	PSR_CC_UNLOCK(prPsrCC->rLock);

	MM_RETURN(mrRet);
}


/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_TxMemory2FifoWithAUCtrl */
/* If the designated parser filter isn't ground, forbidden operation */
/* If it is ground type, do the following tasks: */
/* 1) If Parser CC's state isn't CCS_INIT or isn't tx OK, Change Parser CC's state to CCS_INIT */
/* 2) If Parser CC's state is CCS_INIT or TX_OK state, do the following tasks: */
/* A) Set Source Memory SA, Len, Offset, */
/* B) change Parser CC's Active filter to be this, */
/* C) Set TX_TO_FIFO flag, parser cc's state to be CCS_TX */
/* D) If parser filter is disable or ground type, call tx to ground, otherwise,
* call tx mem function to tx data from memory to fifo */
/* F) If need create AU, set fgAUCtrlByLen and total tx AU len */
/* H) If parser filter is disable, call tx to ground, otherwise,
* call tx pbbuf function to tx data from pbbuf to fifo */
/* @Param prPsrFtr   [IN] Filter handle */
/* @Param pvSrcSa [IN] Source memory start address */
/* @Param prAUCtrlInfo     [IN] AU Ctrl Info */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_TxMemory2FifoWithAUCtrl(PSR_FILTER *prPsrFtr,
	void *pvSrcSa, PSR_AUCtrlInfo *prAUCtrlInfo)
{
	MRESULT mrRet = RET_DMX_OK;
	PSR_CC *prPsrCC = NULL;
	u32 u4Len = (u32) prAUCtrlInfo->u8Len;
	PSR_VFSD *prVFSD = (PSR_VFSD *) prPsrFtr->pvFilterSpecific;
	s32 i;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	for (i = 0; i < MAX_FILTER_COUNT; ++i) {
		if (((PSR_FILTER *) prPsrFtr == g_rPsrMan.aprPsrFtrs[i]) &&
		    (((PSR_FILTER *) prPsrFtr)->u4Flag & FF_USED)) {
			i = MAX_FILTER_COUNT << 1;
			break;
		}
	}
	if (i == MAX_FILTER_COUNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (0 == u4Len) {
		DMXLOG_ERROR(TEXT("[PSR] %s fail for u8Len is 0!!\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if ((prAUCtrlInfo->fgCreateAU) && (prAUCtrlInfo->u8TotalAULen < u4Len)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for total len:0x%llx ")
					  TEXT("is little than len 0x%llx, line: %d!!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prAUCtrlInfo->u8TotalAULen, u4Len);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrCC = prPsrFtr->pvPsrCC;
	if (NULL == prPsrCC) {
		DMXLOG_ERROR(TEXT("[PSR] %s fail for PsrCC is NULL!!\r\n"),
			    DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_NO_CC);
	}

	PSR_CC_LOCK(prPsrCC->rLock);

	if ((CCS_INIT == prPsrCC->eState) ||
	    ((CCS_TX == prPsrCC->eState) && (TXS_TX_OK == prPsrCC->eTxState))) {
		/* backup information */
		prPsrCC->ptrSrcMemSa = (uintptr_t)pvSrcSa;
		prPsrCC->u4SrcMemLen = u4Len;
		prPsrCC->u8TxLen = prAUCtrlInfo->u8Len;
		prPsrCC->u4MemOffset = 0;
		prPsrFtr->u4Flag &= (~FF_TX_PBBUF);
		prPsrFtr->u4Flag |= FF_TX_TO_FIFO;
		prPsrFtr->fgAUCtrlByLen = FALSE;
		prPsrCC->pvActFilter = prPsrFtr;
		prPsrCC->eState = CCS_TX;

#if ENABLE_DMX_ADVANCED_VER
		prPsrCC->fgInsertHdr = prAUCtrlInfo->fgInsertHdr;
		prPsrCC->u4InsertHdrLen = prAUCtrlInfo->u4InsertHdrLen;
		if (NULL != prAUCtrlInfo->pu1InsertHdrBuf) {
			dmx_memcpy(prPsrCC->au1InsertHdrBuf, prAUCtrlInfo->pu1InsertHdrBuf,
				   prPsrCC->u4InsertHdrLen);
		}
#endif				/* ENABLE_DMX_ADVANCED_VER */

		if (prAUCtrlInfo->fgCreateAU) {
			prPsrFtr->fgAUCtrlByLen = TRUE;
			prPsrFtr->fgAUCtrlByEnd = FALSE;
			prPsrFtr->u8TotalAULen = prAUCtrlInfo->u8TotalAULen;
			prPsrFtr->u8CurAULen = 0;
			prPsrFtr->u8WMDRMTxLen = 0;

			if (SPT_DATA_V == prPsrFtr->eType) {
				if (PSR_IsNonHdrVideoType(prVFSD->eVCodeC)
#if ENABLE_DMX_ADVANCED_VER
				    && (!prPsrCC->fgUseCmdQ)
#endif				/* ENABLE_DMX_ADVANCED_VER */
				    ) {
					prVFSD->u4VType = prAUCtrlInfo->u4Vtype;
				}
			}
		}

		if (SPT_DATA_V == prPsrFtr->eType) {
			if (PSR_IsNonHdrVideoType(prVFSD->eVCodeC)
#if ENABLE_DMX_ADVANCED_VER
			    && (!prPsrCC->fgUseCmdQ)
#endif				/* ENABLE_DMX_ADVANCED_VER */
			    ) {
				if (0 == prVFSD->u4VType)
					prVFSD->u4VType = prAUCtrlInfo->u4Vtype;
			}

			prVFSD->fgQueryWVC1Mode = FALSE;
			if (prAUCtrlInfo->fgQueryWVC1Mode)
				prVFSD->fgQueryWVC1Mode = TRUE;
		}

		smp_mb();

		if ((0 == (prPsrFtr->u4Flag & FF_ENABLE)) || (SPT_DATA_GRD == prPsrFtr->eType))
			mrRet = PSR_Filter_TxToGround(prPsrFtr);
		else
			mrRet = PSR_Filter_TxMem(prPsrFtr);
	} else {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for error PsrCC ")
					  TEXT("state, st:0x%x, txSt:0x%x, line: %d!!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->eState, prPsrCC->eTxState);
		mrRet = RET_DMX_ERR_STATE;
	}

	PSR_CC_UNLOCK(prPsrCC->rLock);

	MM_RETURN(mrRet);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_DMAPBBuf4HdrParsing */
/* If the parser filter is not DMA or is not enable, exit */
/* If Parser CC's state is CCS_INIT or CCS_TX, do the following tasks: */
/* A) Set Tx start offset and tx len, current offset */
/* B) Set TX PBBUF flag, clear TX_TO_FIFO flag */
/* C) Set Destination Memory Start Address */
/* D) Set Parser CC state to be CCS_TX, TxState to be TXS_WAIT_PBBUF */
/* E) Call PSR_Filter_Tx4HdrParsing to Wait Data txed into PBBUF */
/* @Param  prPsrFtr    [IN] Filter handle */
/* @Param  u8Offset  [IN] start offset, unit by content, maybe LBA or bytes */
/* @Param  u4Len     [IN] length, unit: bytes */
/* @Param  ptrTgtSa  [IN] target memory start address */
/* @Param  pu4AvailSz [IN/Out] Available pbbuf size from target memory sa in current slot */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_DMAPBBuf4HdrParsing(PSR_FILTER *prPsrFtr, u64 u8Offset,
				       u32 u4Len, uintptr_t *pptrTgtSa, u32 *pu4AvailSz)
{
	MRESULT mrRet = RET_DMX_OK;
	PSR_CC *prPsrCC;

	if ((NULL == pptrTgtSa) || (NULL == prPsrFtr)) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s fail for pptrTgtSa is 0 or prPsrFtr is NULL!!\r\n"),
			    DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*pptrTgtSa = 0;

	if (SPT_DATA_BUF != prPsrFtr->eType) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s fail for PsrFilter's eType(%d) is not BUF!!\r\n"),
			    DMX_FUNC_NAME, prPsrFtr->eType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (0 == (prPsrFtr->u4Flag & FF_ENABLE)) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s fail for PsrFilter's Flag not include FF_ENABLE!!\r\n"),
			    DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	prPsrCC = (PSR_CC *) prPsrFtr->pvPsrCC;
	if (NULL == prPsrCC) {
		DMXLOG_ERROR(TEXT("[PSR] %s fail for PsrCC is NULL!!\r\n"),
			    DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_NO_CC);
	}

	PSR_CC_LOCK(prPsrCC->rLock);

	if ((CCS_INIT == prPsrCC->eState) || (CCS_TX == prPsrCC->eState)) {
		PSR_DMASD *prDMASD = (PSR_DMASD *) prPsrFtr->pvFilterSpecific;

		prPsrCC->u8TxStartOffset = u8Offset;
		prPsrCC->u8TxLen = u4Len;
		prPsrCC->u8TxCurrOffset = u8Offset;
		prPsrCC->pvActFilter = prPsrFtr;
		prPsrFtr->u4Flag &= (~FF_TX_TO_FIFO);
		prPsrFtr->u4Flag |= FF_TX_PBBUF;
		prPsrCC->eState = CCS_TX;

		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);
		prDMASD->fgHdrParsing = TRUE;
		prDMASD->pptrTgtHdrPrsSa = pptrTgtSa;
		prDMASD->pu4AvailSz = pu4AvailSz;

		smp_mb();
		mrRet = PSR_Filter_Tx4HdrParsing(prPsrFtr);
	} else {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for error ")
					  TEXT("PsrCC state, st:0x%x, txSt:0x%x, line: %d!!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->eState, prPsrCC->eTxState);
		mrRet = RET_DMX_ERR_STATE;
	}

	PSR_CC_UNLOCK(prPsrCC->rLock);

	MM_RETURN(mrRet);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_DMASubCode */
/* Do nothing */
/* @Param  prPsrFtr          [IN] Filter handle */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_DMASubCode(PSR_FILTER *prPsrFtr)
{
	PSR_CC *prPsrCC = NULL;
	MRESULT mrRet = RET_DMX_OK;
	s32 i;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	for (i = 0; i < MAX_FILTER_COUNT; ++i) {
		if (((PSR_FILTER *) prPsrFtr == g_rPsrMan.aprPsrFtrs[i]) &&
		    (((PSR_FILTER *) prPsrFtr)->u4Flag & FF_USED)) {
			i = MAX_FILTER_COUNT << 1;
			break;
		}
	}
	if (i == MAX_FILTER_COUNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrCC = prPsrFtr->pvPsrCC;
	if (NULL != prPsrCC) {
		if (prPsrCC->fgCfaPrsEnd)
			MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (!PSR_HWRes_Obtain(prPsrFtr))
		MM_RETURN(RET_DMX_OK);

	DMX_ASSERT(prPsrFtr->ucHwDevId == 0);

	PSR_HWRes_Release(prPsrFtr);

	MM_RETURN(mrRet);
}

#if ENABLE_DMX_ADVANCED_VER
MRESULT PSR_Filter_PreChkForCmdQTx(PSR_FILTER *prPsrFtr, u32 u4FifoSpace, u32 u4CurMaxTxLen)
{
	DMX_CMDQ_TX_ENTRY_T *prTxEntry = NULL;
	PSR_CMDQ_TX_INF *prCmdQTxInf = NULL;
	PSR_CC *prPsrCC = NULL;
	u64 u8RealAdvLen = 0;
	u64 u8RealTxLen = 0;
	u16 u2Idx = 0;
	bool fgInsertHdr = FALSE;
	s32 i;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	for (i = 0; i < MAX_FILTER_COUNT; ++i) {
		if (((PSR_FILTER *) prPsrFtr == g_rPsrMan.aprPsrFtrs[i]) &&
		    (((PSR_FILTER *) prPsrFtr)->u4Flag & FF_USED)) {
			i = MAX_FILTER_COUNT << 1;
			break;
		}
	}
	if (i == MAX_FILTER_COUNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);
	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_NO_CC);

	if (!prPsrCC->fgUseCmdQ) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (0 == prPsrCC->pvCmdQTxEntryBuffer) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	/* Maybe check already, but delay by some case */
	/* Todo: Can judge again for new free fifo space */
	if (prPsrCC->fgChkedAndWaitTx) {
		DMXLOG_DEBUG(
			    TEXT
			    ("[PSR] %s -- chk already, tx ofst: 0x%08x%08x, tx len: 0x%08x%08x\r\n"),
			    DMX_FUNC_NAME, (u32) (prPsrCC->u8TxStartOffset >> 32),
			    (u32) prPsrCC->u8TxStartOffset, (u32) (prPsrCC->u8TxLen >> 32),
			    (u32) prPsrCC->u8TxLen);

		/* Check again, using old cmd q tx info */
		dmx_memcpy((void *) &(prPsrCC->rCmdQTxInf),
			   (void *) &(prPsrCC->rCmdQPrevTxInf), sizeof(PSR_CMDQ_TX_INF));
	}

	prCmdQTxInf = &(prPsrCC->rCmdQTxInf);

	if (0 == u4FifoSpace) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for u4FifoSpace == 0\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prCmdQTxInf->u2CurTxRngEIdx,
			    prPsrCC->u2TxEntryCnt);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}

	if ((u16) (prCmdQTxInf->u2CurTxRngEIdx) >= prPsrCC->u2TxEntryCnt) {
		DMXLOG_ERROR(
			    TEXT
			    ("[PSR] %s line %d fail for u2CurTxRngEIdx(%d) > u2TxEntryCnt(%d)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prCmdQTxInf->u2CurTxRngEIdx,
			    prPsrCC->u2TxEntryCnt);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}
	/* Exist enough pb buffer data ?? */
	/* Actually, cfa ensure already: */
	/* 1) all tx in one slot */
	/* 2) only used for video stream currently */
	if ((prPsrCC->u8TxCurrOffset + prPsrCC->u8TxCurrLen) >
	    (prPsrCC->u8TxStartOffset + prPsrCC->u8TxLen)) {
		/* Todo: cfa avoid running to here, need normal proc: divide tx, update info */
#ifdef __linux__
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for ")
					  TEXT("TxCurrOffset(%lld) + TxCurrLen(%lld) > ")
					  TEXT("TxStartOffset(%lld) + TxLen(%lld)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u8TxCurrOffset,
			    prPsrCC->u8TxCurrLen, prPsrCC->u8TxStartOffset, prPsrCC->u8TxLen);
#else
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for TxCurrOffset(%I64d)")
					  TEXT
					  (" + TxCurrLen(%I64d) > TxStartOffset(%I64d) + TxLen(%I64d)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u8TxCurrOffset,
			    prPsrCC->u8TxCurrLen, prPsrCC->u8TxStartOffset, prPsrCC->u8TxLen);
#endif				/* #ifdef __linux__ */
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	/* 1st entry */
	prTxEntry = (DMX_CMDQ_TX_ENTRY_T *) (prPsrCC->pvCmdQTxEntryBuffer);

	if ((!prPsrCC->fgCurTotalCmdQTxStarted) && (prTxEntry->u4TxOfst > 0)) {
		DMXLOG_ERROR(TEXT("[PSR] %s fail for First Entry's TxOfst Error!!\r\n"),
			    DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (0 == prCmdQTxInf->u4CurTxRngEIdxRmnLen) {
		/* Todo: cfa avoid running to here, need normal proc: divide tx, update info */
		DMXLOG_TRACE(
			    TEXT("[PSR] %s line %d -- SIdx(%d), SIdxOfst(%d), SIdxLen(%d)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prCmdQTxInf->u2CurTxRngSIdx,
			    prCmdQTxInf->u4CurTxRngSIdxOfst, prCmdQTxInf->u4CurTxRngSIdxLen);
		DMXLOG_TRACE(TEXT("[PSR] %s line %d -- EIdx(%d), EIdxOfst(%d), ")
					  TEXT
					  ("EIdxLen(%d), EIdxRmnLen(%d), RmnTotalRealTxLen(0x%08x%08x)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prCmdQTxInf->u2CurTxRngEIdx,
			    prCmdQTxInf->u4CurTxRngEIdxOfst, prCmdQTxInf->u4CurTxRngEIdxLen,
			    prCmdQTxInf->u4CurTxRngEIdxRmnLen,
			    (u32) (prCmdQTxInf->u8RmnTotalRealTxLen >> 32),
			    (u32) (prCmdQTxInf->u8RmnTotalRealTxLen));
#ifdef __linux__
		DMXLOG_TRACE(TEXT("[PSR] %s line %d -- TxCurrOffset(%lld), ")
					  TEXT
					  ("TxCurrLen(%lld), TxStartOffset(%lld), TxLen(%lld)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u8TxCurrOffset,
			    prPsrCC->u8TxCurrLen, prPsrCC->u8TxStartOffset, prPsrCC->u8TxLen);
#else
		DMXLOG_TRACE(TEXT("[PSR] %s line %d -- TxCurrOffset(%I64d),")
					  TEXT
					  (" TxCurrLen(%I64d), TxStartOffset(%I64d), TxLen(%I64d)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u8TxCurrOffset,
			    prPsrCC->u8TxCurrLen, prPsrCC->u8TxStartOffset, prPsrCC->u8TxLen);
#endif				/* #ifdef __linux__ */
		DMX_ASSERT(FALSE);
	}

	prCmdQTxInf->u2CurTxRngSIdx = prCmdQTxInf->u2CurTxRngEIdx;
	prCmdQTxInf->u4CurTxRngSIdxOfst =
	    prCmdQTxInf->u4CurTxRngEIdxOfst + prCmdQTxInf->u4CurTxRngEIdxLen;

	u2Idx = 0;
	u8RealTxLen = prCmdQTxInf->u4CurTxRngEIdxRmnLen;
	u8RealAdvLen = prCmdQTxInf->u4CurTxRngEIdxRmnLen;

	fgInsertHdr = FALSE;

	prTxEntry =
	    ((DMX_CMDQ_TX_ENTRY_T *) (prPsrCC->pvCmdQTxEntryBuffer)) + prCmdQTxInf->u2CurTxRngSIdx;
    /**
	* the first cmdentry's remain data size is exceed the fifo remained size of the curmaxtxlen
	**/
	if ((u8RealTxLen > (u64) u4FifoSpace) || (u8RealTxLen > (u64) u4CurMaxTxLen)) {
		u4FifoSpace = (u4FifoSpace >= u4CurMaxTxLen) ? u4CurMaxTxLen : u4FifoSpace;
		prCmdQTxInf->u4CurTxRngSIdxLen = u4FifoSpace;
		prCmdQTxInf->u4CurTxRngEIdxOfst += prCmdQTxInf->u4CurTxRngEIdxLen;
		prCmdQTxInf->u4CurTxRngEIdxLen = u4FifoSpace;

		prCmdQTxInf->u4CurTxRngEIdxRmnLen -= u4FifoSpace;

		prCmdQTxInf->u8RmnTotalRealTxLen -= u4FifoSpace;

		/* Need adjust transfer length */
		prPsrCC->u8TxCurrLen = u4FifoSpace;
		prPsrCC->fgChkedAndWaitTx = TRUE;
		prPsrCC->fgCurTotalCmdQTxStarted = TRUE;

		MM_RETURN(RET_DMX_OK);
	}

	prCmdQTxInf->u4CurTxRngSIdxLen = prCmdQTxInf->u4CurTxRngEIdxRmnLen;
	fgInsertHdr = FALSE;
	if ((prTxEntry->fgInsertHdr) && (prCmdQTxInf->u4CurTxRngSIdxLen == prTxEntry->u4TxLen))
		fgInsertHdr = TRUE;

	if (0 == prCmdQTxInf->u4CurTxRngSIdxOfst)
		u8RealAdvLen += prTxEntry->u4TxOfst;

	for (u2Idx = prCmdQTxInf->u2CurTxRngEIdx + 1; u2Idx < prPsrCC->u2TxEntryCnt; u2Idx++) {
		prTxEntry = ((DMX_CMDQ_TX_ENTRY_T *) (prPsrCC->pvCmdQTxEntryBuffer)) + u2Idx;
		/**
		  * the second cmdentry of the remained cmdq is need to txhdr,
		  * so we must tx the first cmdentry.
		  * and then tx the second cmdentry in the next cmdq dma.
		  **/
		if ((u2Idx == prCmdQTxInf->u2CurTxRngEIdx + 1) &&
		    (prTxEntry->fgInsertHdr) && (!fgInsertHdr)) {
			prCmdQTxInf->u2CurTxRngEIdx = u2Idx;
			prCmdQTxInf->u4CurTxRngEIdxOfst = 0;
			prCmdQTxInf->u4CurTxRngEIdxLen = 0;
			prCmdQTxInf->u4CurTxRngEIdxRmnLen = prTxEntry->u4TxLen;

			prCmdQTxInf->u8RmnTotalRealTxLen -= prCmdQTxInf->u4CurTxRngSIdxLen;

			prPsrCC->u8TxCurrLen = prCmdQTxInf->u4CurTxRngSIdxLen;
			prPsrCC->fgChkedAndWaitTx = TRUE;
			prPsrCC->fgCurTotalCmdQTxStarted = TRUE;

			MM_RETURN(RET_DMX_OK);
		}

		/**
		* This cmdentry's accross the pbbuf slot, so we must divide this cmdq into two cmdq.
		**/
		if (u8RealAdvLen + prTxEntry->u4TxOfst > u4CurMaxTxLen) {
			prCmdQTxInf->u2CurTxRngEIdx = u2Idx;
			prCmdQTxInf->u4CurTxRngEIdxOfst = 0;
			prCmdQTxInf->u4CurTxRngEIdxLen = 0;
			prCmdQTxInf->u4CurTxRngEIdxRmnLen = prTxEntry->u4TxLen;

			prCmdQTxInf->u8RmnTotalRealTxLen -= u8RealTxLen;

			prPsrCC->u8TxCurrLen = u4CurMaxTxLen;
			prPsrCC->fgChkedAndWaitTx = TRUE;
			prPsrCC->fgCurTotalCmdQTxStarted = TRUE;
			MM_RETURN(RET_DMX_OK);
		}

		if (!prTxEntry->fgInsertHdr) {
			fgInsertHdr = FALSE;
		} else if (!fgInsertHdr) {
			/**
			* the second cmdentry of the remained cmdq is need to txhdr,
			* so we must tx the first cmdentry.
			* and then tx the second cmdentry in the next cmdq dma.
			**/
			prCmdQTxInf->u2CurTxRngEIdx = u2Idx;
			prCmdQTxInf->u4CurTxRngEIdxOfst = 0;
			prCmdQTxInf->u4CurTxRngEIdxLen = 0;
			prCmdQTxInf->u4CurTxRngEIdxRmnLen = prTxEntry->u4TxLen;

			prCmdQTxInf->u8RmnTotalRealTxLen -= prCmdQTxInf->u4CurTxRngSIdxLen;

			prPsrCC->u8TxCurrLen = prCmdQTxInf->u4CurTxRngSIdxLen;
			prPsrCC->fgChkedAndWaitTx = TRUE;
			prPsrCC->fgCurTotalCmdQTxStarted = TRUE;
			MM_RETURN(RET_DMX_OK);
		}

		u8RealAdvLen += prTxEntry->u4TxOfst;

		if ((u8RealAdvLen + prTxEntry->u4TxLen > u4CurMaxTxLen) ||
		    (u8RealTxLen + prTxEntry->u4TxLen > u4FifoSpace)) {
			u64 u8TxLen = ((u64) u4FifoSpace - u8RealTxLen);

			if (u8TxLen > ((u64) u4CurMaxTxLen - u8RealAdvLen))
				u8TxLen = ((u64) u4CurMaxTxLen - u8RealAdvLen);

			u8RealTxLen += u8TxLen;
			u8RealAdvLen += u8TxLen;

			prCmdQTxInf->u2CurTxRngEIdx = u2Idx;
			prCmdQTxInf->u4CurTxRngEIdxOfst = 0;
			prCmdQTxInf->u4CurTxRngEIdxLen = (u32) u8TxLen;
			prCmdQTxInf->u4CurTxRngEIdxRmnLen =
			    prTxEntry->u4TxLen - prCmdQTxInf->u4CurTxRngEIdxLen;
			prCmdQTxInf->u8RmnTotalRealTxLen -= u8RealTxLen;
			/* Need adjust transfer length */
			prPsrCC->u8TxCurrLen = u8RealAdvLen;
			prPsrCC->fgChkedAndWaitTx = TRUE;
			prPsrCC->fgCurTotalCmdQTxStarted = TRUE;
			MM_RETURN(RET_DMX_OK);
		}

		u8RealTxLen += prTxEntry->u4TxLen;
		u8RealAdvLen += prTxEntry->u4TxLen;
	}

	prCmdQTxInf->u2CurTxRngEIdx = (prPsrCC->u2TxEntryCnt - 1);
	if (prCmdQTxInf->u2CurTxRngSIdx != prCmdQTxInf->u2CurTxRngEIdx) {
		prTxEntry =
		    ((DMX_CMDQ_TX_ENTRY_T *) (prPsrCC->pvCmdQTxEntryBuffer)) +
		    prCmdQTxInf->u2CurTxRngEIdx;
		prCmdQTxInf->u4CurTxRngEIdxOfst = 0;
		prCmdQTxInf->u4CurTxRngEIdxLen = prTxEntry->u4TxLen;
	} else {
		/* Use start idx entry, no use end idx entry currently */
		prCmdQTxInf->u4CurTxRngEIdxOfst = prCmdQTxInf->u4CurTxRngSIdxOfst;
		prCmdQTxInf->u4CurTxRngEIdxLen = prCmdQTxInf->u4CurTxRngSIdxLen;
	}

	/* Need adjust transfer length */
	prPsrCC->u8TxCurrLen = u8RealAdvLen;

	prCmdQTxInf->u4CurTxRngEIdxRmnLen = 0;
	prCmdQTxInf->u8RmnTotalRealTxLen = 0;

	prPsrCC->fgChkedAndWaitTx = TRUE;
	prPsrCC->fgCurTotalCmdQTxStarted = TRUE;

	{
		/* Command Number */
		u16 u2CmdQEntryCnt =
		    prCmdQTxInf->u2CurTxRngEIdx - prCmdQTxInf->u2CurTxRngSIdx + 1;
		u16 u2Idx = 0;
		u32 u4CmdQSrcDataSz;

		if (u2CmdQEntryCnt > DMX_MAX_TX_CNT_FOR_CMD_Q) {
			PSR_HAL_UNLOCK;
			DMXLOG_ERROR(
				    TEXT("[PSR] %s line %d fail for CmdQEntryCnt(%d) ")
				     TEXT("> DMX_MAX_TX_CNT_FOR_CMD_Q(%d)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u2CmdQEntryCnt,
				    DMX_MAX_TX_CNT_FOR_CMD_Q);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_OVER_LIMIT);
		}

		if ((u2CmdQEntryCnt > 1) && (NULL == prTxEntry)) {
			PSR_HAL_UNLOCK;
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for ")
						  TEXT
						  ("CmdQEntryCnt(%d) > 1, but no CmdEntry\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u2CmdQEntryCnt);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}

		u4CmdQSrcDataSz = 0;

		prTxEntry =
		    ((DMX_CMDQ_TX_ENTRY_T *) (prPsrCC->pvCmdQTxEntryBuffer)) +
		    prCmdQTxInf->u2CurTxRngSIdx;

		u4CmdQSrcDataSz += prCmdQTxInf->u4CurTxRngSIdxLen;

		for (u2Idx = prCmdQTxInf->u2CurTxRngSIdx + 1; u2Idx < prCmdQTxInf->u2CurTxRngEIdx;
		     u2Idx++) {
			prTxEntry =
			    ((DMX_CMDQ_TX_ENTRY_T *) (prPsrCC->pvCmdQTxEntryBuffer)) + u2Idx;
			u4CmdQSrcDataSz += prTxEntry->u4TxOfst + prTxEntry->u4TxLen;
		}

		if (prCmdQTxInf->u2CurTxRngSIdx != prCmdQTxInf->u2CurTxRngEIdx)
			u4CmdQSrcDataSz += prCmdQTxInf->u4CurTxRngEIdxLen;

		if (u4CmdQSrcDataSz > prPsrCC->u8TxCurrLen) {
			DMXLOG_ERROR(
				    TEXT("[PSR] %s line %d fail for CmdQEntrys(Cnt:%d)'s")
				     TEXT(" total data Sz(%d) > u8TxCurrLen(%lld)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u2CmdQEntryCnt, u4CmdQSrcDataSz,
				    prPsrCC->u8TxCurrLen);
			/* Todo: cfa avoid running to here, need normal proc: divide tx, update info */
			DMXLOG_TRACE(
				    TEXT("[PSR] %s line %d -- SIdx(%d), fgInsertHdr(%d),")
				     TEXT(" SIdxOfst(%d), SIdxLen(%d)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prCmdQTxInf->u2CurTxRngSIdx,
				    ((prTxEntry->fgInsertHdr) ? 1 : 0),
				    prCmdQTxInf->u4CurTxRngSIdxOfst,
				    prCmdQTxInf->u4CurTxRngSIdxLen);
			prTxEntry =
			    ((DMX_CMDQ_TX_ENTRY_T *) (prPsrCC->pvCmdQTxEntryBuffer)) +
			    prCmdQTxInf->u2CurTxRngSIdx;
			for (u2Idx = prCmdQTxInf->u2CurTxRngSIdx + 1;
			     u2Idx < prCmdQTxInf->u2CurTxRngEIdx; u2Idx++, prTxEntry++) {
				u4CmdQSrcDataSz += prTxEntry->u4TxOfst + prTxEntry->u4TxLen;
				DMXLOG_TRACE(TEXT("[PSR] %s line %d -- Idx(%d), ")
							  TEXT
							  ("fgInsertHdr(%d), TxOfst(%d), TxLen(%d)\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, u2Idx,
					    ((prTxEntry->fgInsertHdr) ? 1 : 0), prTxEntry->u4TxOfst,
					    prTxEntry->u4TxLen);
			}
			prTxEntry =
			    ((DMX_CMDQ_TX_ENTRY_T *) (prPsrCC->pvCmdQTxEntryBuffer)) +
			    prCmdQTxInf->u2CurTxRngEIdx;
			DMXLOG_TRACE(TEXT("[PSR] %s line %d -- EIdx(%d), ")
						  TEXT
						  ("fgInsertHdr(%d), EIdxOfst(%d), EIdxLen(%d),")
						  TEXT
						  (" EIdxRmnLen(%d), RmnTotalRealTxLen(0x%08x%08x)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prCmdQTxInf->u2CurTxRngEIdx,
				    ((prTxEntry->fgInsertHdr) ? 1 : 0),
				    prCmdQTxInf->u4CurTxRngEIdxOfst, prCmdQTxInf->u4CurTxRngEIdxLen,
				    prCmdQTxInf->u4CurTxRngEIdxRmnLen,
				    (u32) (prCmdQTxInf->u8RmnTotalRealTxLen >> 32),
				    (u32) (prCmdQTxInf->u8RmnTotalRealTxLen));
			DMX_ASSERT(FALSE);
		}
	}

	MM_RETURN(RET_DMX_OK);
}

#else
/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_PreChkForCmdQTx */
/* Check Cmd Q current time need to tx entrys, change start tx idx and end tx idx */
/* @Param  prPsrFtr            [IN] Filter handle */
/* @Param  u4FifoSpace     [IN] Fifo Current remained free size */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_PreChkForCmdQTx(PSR_FILTER *prPsrFtr, u32 u4FifoSpace, u32 u4CurMaxTxLen)
{
	DMX_CMDQ_TX_ENTRY_T *prTxEntry = NULL;
	PSR_CMDQ_TX_INF *prCmdQTxInf = NULL;
	PSR_CC *prPsrCC = NULL;
	MRESULT mrRet = RET_DMX_OK;
	s32 i;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	for (i = 0; i < MAX_FILTER_COUNT; ++i) {
		if (((PSR_FILTER *) prPsrFtr == g_rPsrMan.aprPsrFtrs[i]) &&
		    (((PSR_FILTER *) prPsrFtr)->u4Flag & FF_USED)) {
			i = MAX_FILTER_COUNT << 1;
			break;
		}
	}
	if (i == MAX_FILTER_COUNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	UNUSE_PARAMETER(u4CurMaxTxLen);

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);
	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_NO_CC);

	if (!prPsrCC->fgUseCmdQ) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (NULL == prPsrCC->pvCmdQTxEntryBuffer) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	/* Maybe check already, but delay by some case */
	/* Todo: Can judge again for new free fifo space */
	if (prPsrCC->fgChkedAndWaitTx) {
		DMXLOG_DEBUG(TEXT("[PSR] %s -- chk already, ")
					  TEXT("tx ofst: 0x%08x%08x, tx len: 0x%08x%08x\r\n"),
			    DMX_FUNC_NAME, (u32) (prPsrCC->u8TxStartOffset >> 32),
			    (u32) prPsrCC->u8TxStartOffset, (u32) (prPsrCC->u8TxLen >> 32),
			    (u32) prPsrCC->u8TxLen);

		/* Check again, using old cmd q tx info */
		dmx_memcpy((void *) &(prPsrCC->rCmdQTxInf),
			   (void *) &(prPsrCC->rCmdQPrevTxInf), sizeof(PSR_CMDQ_TX_INF));
	}

	prCmdQTxInf = &(prPsrCC->rCmdQTxInf);

	if (0 == u4FifoSpace) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for u4FifoSpace == 0\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prCmdQTxInf->u2CurTxRngEIdx,
			    prPsrCC->u2TxEntryCnt);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}

	if ((u16) (prCmdQTxInf->u2CurTxRngEIdx) >= prPsrCC->u2TxEntryCnt) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for ")
					  TEXT("u2CurTxRngEIdx(%d) > u2TxEntryCnt(%d)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prCmdQTxInf->u2CurTxRngEIdx,
			    prPsrCC->u2TxEntryCnt);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}
	/* Exist enough pb buffer data ?? */
	/* Actually, cfa ensure already: */
	/* 1) all tx in one slot */
	/* 2) only used for video stream currently */
	if ((prPsrCC->u8TxCurrOffset + prPsrCC->u8TxCurrLen) >
	    (prPsrCC->u8TxStartOffset + prPsrCC->u8TxLen)) {
		/* Todo: cfa avoid running to here, need normal proc: divide tx, update info */
#ifdef __linux__
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for TxCurrOffset(%lld)")
					  TEXT
					  (" + TxCurrLen(%lld) > TxStartOffset(%lld) + TxLen(%lld)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u8TxCurrOffset,
			    prPsrCC->u8TxCurrLen, prPsrCC->u8TxStartOffset, prPsrCC->u8TxLen);
#else
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for TxCurrOffset(%I64d)")
					  TEXT
					  (" + TxCurrLen(%I64d) > TxStartOffset(%I64d) + TxLen(%I64d)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u8TxCurrOffset,
			    prPsrCC->u8TxCurrLen, prPsrCC->u8TxStartOffset, prPsrCC->u8TxLen);
#endif				/* #ifdef __linux__ */
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	/* 1st entry */
	prTxEntry = (DMX_CMDQ_TX_ENTRY_T *) (prPsrCC->pvCmdQTxEntryBuffer);

	if ((!prPsrCC->fgCurTotalCmdQTxStarted) && (prTxEntry->u4TxOfst > 0)) {
		DMXLOG_ERROR(TEXT("[PSR] %s fail for First Entry's TxOfst Error!!\r\n"),
			    DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	/* Exist enough fifo space ?? */
	if (prCmdQTxInf->u8RmnTotalRealTxLen <= u4FifoSpace) {
		prCmdQTxInf->u2CurTxRngSIdx = prCmdQTxInf->u2CurTxRngEIdx;
		prCmdQTxInf->u4CurTxRngSIdxOfst =
		    prCmdQTxInf->u4CurTxRngEIdxOfst + prCmdQTxInf->u4CurTxRngEIdxLen;
		prCmdQTxInf->u4CurTxRngSIdxLen = prCmdQTxInf->u4CurTxRngEIdxRmnLen;

		prCmdQTxInf->u2CurTxRngEIdx = (prPsrCC->u2TxEntryCnt - 1);

		if (prCmdQTxInf->u2CurTxRngSIdx != prCmdQTxInf->u2CurTxRngEIdx) {
			prTxEntry += (prPsrCC->u2TxEntryCnt - 1);

			prCmdQTxInf->u4CurTxRngEIdxOfst = 0;
			prCmdQTxInf->u4CurTxRngEIdxLen = prTxEntry->u4TxLen;
		} else {
			/* Use start idx entry, no use end idx entry currently */
			prCmdQTxInf->u4CurTxRngEIdxOfst = prCmdQTxInf->u4CurTxRngSIdxOfst;
			prCmdQTxInf->u4CurTxRngEIdxLen = prCmdQTxInf->u4CurTxRngSIdxLen;
		}

		prCmdQTxInf->u4CurTxRngEIdxRmnLen = 0;

		prCmdQTxInf->u8RmnTotalRealTxLen = 0;
	} else {
		/* If unfinished tx, ensure that next tx start from address of one entry tx data */
		u64 u8RealAdvLen = 0;

		prCmdQTxInf->u2CurTxRngSIdx = prCmdQTxInf->u2CurTxRngEIdx;
		prCmdQTxInf->u4CurTxRngSIdxOfst =
		    prCmdQTxInf->u4CurTxRngEIdxOfst + prCmdQTxInf->u4CurTxRngEIdxLen;

		/* Notice: if one entry can tx into fifo completely, */
		/* need set infomation using next entry!! */
		if (prCmdQTxInf->u4CurTxRngEIdxRmnLen <= u4FifoSpace) {
			u16 u2Idx = 0;
			u64 u8RealTxLen = prCmdQTxInf->u4CurTxRngEIdxRmnLen;

			u8RealAdvLen = prCmdQTxInf->u4CurTxRngEIdxRmnLen;

			prCmdQTxInf->u4CurTxRngSIdxLen = prCmdQTxInf->u4CurTxRngEIdxRmnLen;

			/* Not reach last entry */
			if (prCmdQTxInf->u2CurTxRngEIdx >= (prPsrCC->u2TxEntryCnt - 1)) {
				/* Cannot goto here because: */
				/* (prPsrCC->u8RmnTotalRealTxLen > u4FifoSpace) */
				DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for ")
							  TEXT
							  ("u2CurTxRngEIdx(%d) >= u2TxEntryCnt(%d) - 1!!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, prCmdQTxInf->u2CurTxRngEIdx,
					    prPsrCC->u2TxEntryCnt);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			prTxEntry += (prCmdQTxInf->u2CurTxRngEIdx + 1);

			u2Idx = prCmdQTxInf->u2CurTxRngEIdx + 1;

			for (; u2Idx < prPsrCC->u2TxEntryCnt; u2Idx++) {
				u8RealAdvLen += prTxEntry->u4TxOfst;

				/* If equal u4FifoSpace, get next entry continuously */
				if ((u8RealTxLen + prTxEntry->u4TxLen) > u4FifoSpace) {
					/* Must goto here because of enter condition */
					break;
				}

				u8RealTxLen += prTxEntry->u4TxLen;
				u8RealAdvLen += prTxEntry->u4TxLen;

				prTxEntry++;
			}

			if (u2Idx == prPsrCC->u2TxEntryCnt) {
				/* Cannot goto here, because: */
				/* (prPsrCC->u8RmnTotalRealTxLen > u4FifoSpace) */
				DMXLOG_ERROR(
					    TEXT
					    ("[PSR] %s line %d fail for u2Idx == u2TxEntryCnt(%d)!!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u2TxEntryCnt);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			if (u8RealTxLen > u4FifoSpace) {
				/* Cannot goto here, because: */
				/* (prPsrCC->u8RmnTotalRealTxLen > u4FifoSpace) */
#ifdef __linux__
				DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for ")
							  TEXT
							  ("u8RealTxLen(%lld) > u4FifoSpace(%d)!!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, u8RealTxLen, u4FifoSpace);
#else
				DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for ")
							  TEXT
							  ("u8RealTxLen(%I64d) > u4FifoSpace(%d)!!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, u8RealTxLen, u4FifoSpace);
#endif				/* #ifdef __linux__ */
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			prCmdQTxInf->u2CurTxRngEIdx = u2Idx;
			prCmdQTxInf->u4CurTxRngEIdxOfst = 0;
			prCmdQTxInf->u4CurTxRngEIdxLen = (u32) (u4FifoSpace - u8RealTxLen);

			u8RealAdvLen += prCmdQTxInf->u4CurTxRngEIdxLen;

			/* prTxEntry now pointer to the u4CurTxRngEIdx entry */
			if (prTxEntry->u4TxLen < (u4FifoSpace - u8RealTxLen)) {
				/* Cannot goto here, because: */
				/* (prPsrCC->u8RmnTotalRealTxLen > u4FifoSpace) */
#ifdef __linux__
				DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for ")
							  TEXT
							  ("prTxEntry->u4TxLen(%lld) < u4FifoSpace(%d)")
							  TEXT(" - u8RealTxLen(%lld)!!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, u8RealTxLen, u4FifoSpace,
					    u8RealTxLen);
#else
				DMXLOG_ERROR(
					    TEXT
					     ("[PSR] %s line %d fail for prTxEntry->u4TxLen(%I64d)")
					     TEXT(" < u4FifoSpace(%d) - u8RealTxLen(%I64d)!!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, u8RealTxLen, u4FifoSpace,
					    u8RealTxLen);
#endif				/* #ifdef __linux__ */
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			prCmdQTxInf->u4CurTxRngEIdxRmnLen =
			    (u32) (prTxEntry->u4TxLen - (u4FifoSpace - u8RealTxLen));
		} else {
			prCmdQTxInf->u4CurTxRngSIdxLen = u4FifoSpace;

			prCmdQTxInf->u4CurTxRngEIdxOfst += prCmdQTxInf->u4CurTxRngEIdxLen;
			prCmdQTxInf->u4CurTxRngEIdxLen = u4FifoSpace;

			prCmdQTxInf->u4CurTxRngEIdxRmnLen -= u4FifoSpace;

			if (prCmdQTxInf->u8RmnTotalRealTxLen < u4FifoSpace) {
				DMXLOG_ERROR(
					    TEXT("[PSR] %s line %d -- remain total tx ")
					     TEXT("len:0x%08x%08x < fifo space:0x%x, error!!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO,
					    (u32) (prCmdQTxInf->u8RmnTotalRealTxLen >> 32),
					    prCmdQTxInf->u8RmnTotalRealTxLen, u4FifoSpace);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			u8RealAdvLen = u4FifoSpace;
		}

		prCmdQTxInf->u8RmnTotalRealTxLen -= u4FifoSpace;

		/* Need adjust transfer length */
		prPsrCC->u8TxCurrLen = u8RealAdvLen;
	}

	prPsrCC->fgChkedAndWaitTx = TRUE;
	prPsrCC->fgCurTotalCmdQTxStarted = TRUE;

	MM_RETURN(mrRet);
}
#endif				/* ENABLE_DMX_ADVANCED_VER */

#ifdef MM_SUPPORT_DIVXHT31
MRESULT PSR_Filter_TxDecideV(PSR_FILTER *prPsrFtr, PSR_CC *prPsrCC)
{
	MRESULT mrRet = RET_DMX_OK;
	u32 u4FifoData = 0, u4FifoSize = 0, u4FifoUsage = 0;
	u32 u4AUCount = 0;
	s32 i4Rate = 1;
	PSR_FILTER *prVPsrFtr = NULL;
	PSR_FILTER *prAPsrFtr = NULL;
	PSR_FILTER *prSPPsrFtr = NULL;
	DMX_INST_T *prDmxInst = NULL;
	u64 u8SyncCurSTC = INVALID_TIMESTAMP;

	prDmxInst = (DMX_INST_T *)prPsrFtr->pvDmxInst;

	if (NULL == prDmxInst) {
		DMX_ASSERT(0);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	u4FifoSize = prPsrFtr->u4ESFifoSize;

	mrRet = ESM_FifoGetAvailDataSize(prPsrFtr->u4ESIH, &u4FifoData);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d (ESIH: 0x%x) fail ")
					  TEXT("in ESM_FifoGetAvailDataSize, mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (0 < u4FifoSize) {
		u4FifoUsage = u4FifoData * 100 / u4FifoSize;
	} else {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s line %d - u4FifoSize: %d, u4FifoData: %d\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u4FifoSize, u4FifoData);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_OPERATE_FORBID);
	}

	mrRet = ESM_AUTableGetAvailCount(prPsrFtr->u4ESIH, &u4AUCount);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ESM_AUTableGetAvailCount")
					  TEXT("(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}
	/* 1) If current Parser CC is FIFO Hold, check whether the remained AU count is <= 1 , */
	/* if is, clear FIFOHOLD flag. */
	/* otherwise, if Fifo Usage <= Min V FIFO Usage, clear FIFOHOLD Flag */
	/* otherwise, Set FIFOHOLD Flag, set Parser CC state to be TXS_WAIT_FIFO */
	/* 2) If current Parser CC is not FIFO Hold, check whether Fifo Usage >= Max V FIFO Usage, */
	/* if is , Set FIFOHOLD Flag, and set Parser CC state to be TXS_WAIT_FIFO */
	if (prPsrCC->u4Flag & CCF_FIFOHOLD) {
		if (0 == u4AUCount) {
			if (u4FifoUsage >= V_FIFO_USAGE_MAX) {
				prPsrCC->u4Flag |= CCF_FIFOHOLD;
				PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);

				if (prDmxInst->u4SptCnt > 1)
					PSR_HWRes_Release(prPsrFtr);

				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
					TEXT("[PSR] %s line %d -- V Fifo Full(Usage: %d),")
					 TEXT(" but has no Video AU, we should sent EOS ++++++++++!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4FifoUsage);

				MM_RETURN(RET_DMX_ERR_DATA);
			} else {
				prPsrCC->u4Flag &= (~CCF_FIFOHOLD);
			}
		} else if (u4AUCount <= 1) {
			prPsrCC->u4Flag &= (~CCF_FIFOHOLD);
		} else {
			if ((SplitterRspIsEnabled(prPsrCC->pvSptHdl) &&
			     SplitterIsRspOffStart(prPsrCC->pvSptHdl))) {
				DMXLOG_DEBUG(
					    TEXT
					    ("[PSR] %s line %d -- RspOffStart, Clear CCF_FIFOHOLD\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO);
				prPsrCC->u4Flag &= (~CCF_FIFOHOLD);
			} else if (u4FifoUsage <= V_FIFO_USAGE_MIN) {
				prPsrCC->u4Flag &= (~CCF_FIFOHOLD);
			} else {
				prPsrCC->u4Flag |= CCF_FIFOHOLD;
				PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);

				if (prDmxInst->u4SptCnt > 1)
					PSR_HWRes_Release(prPsrFtr);

				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
					TEXT("[PSR] %s Line: %d -- Wait V fifo++++++++++!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
#if DMX_PFM_TEST
				g_rPsrPfm.rVideo.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */
				MM_RETURN(RET_DMX_FIFO_FULL);
			}
		}
	} else {
		/* else of if(prPsrCC->u4Flag & CCF_FIFOHOLD) */
		if (0 == u4AUCount) {
			if (u4FifoUsage >= V_FIFO_USAGE_MAX) {
				prPsrCC->u4Flag |= CCF_FIFOHOLD;
				PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);

				if (prDmxInst->u4SptCnt > 1)
					PSR_HWRes_Release(prPsrFtr);

				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
					TEXT("[PSR] %s line %d -- V Fifo Full(Usage: %d),")
					 TEXT
					 (" but has no Video AU, we should sent EOS ++++++++++!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4FifoUsage);

				MM_RETURN(RET_DMX_ERR_DATA);
			} else {
				prPsrCC->u4Flag &= (~CCF_FIFOHOLD);
			}
		}

		if (u4FifoUsage >= V_FIFO_USAGE_MAX) {
			prPsrCC->u4Flag |= CCF_FIFOHOLD;
			PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);

			if (prDmxInst->u4SptCnt > 1)
				PSR_HWRes_Release(prPsrFtr);

			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
				TEXT("[PSR] %s Line: %d -- Wait V fifo++++++++++!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
#if DMX_PFM_TEST
			g_rPsrPfm.rVideo.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */
			MM_RETURN(RET_DMX_FIFO_FULL);
		}
	}

	if ((prDmxInst->u4SptCnt <= 1) && (DMX_IS_NORMAL_PLAY(prPsrCC->pvSptHdl)))
		MM_RETURN(RET_DMX_OK);

	if ((SplitterRspIsEnabled(prPsrCC->pvSptHdl) && SplitterIsRspOffStart(prPsrCC->pvSptHdl)))
		MM_RETURN(RET_DMX_OK);

	STC_HalGetTime(0, &u8SyncCurSTC);
	if (u8SyncCurSTC > 0x01ffffffffl)
		u8SyncCurSTC = 0;

	if (!DMX_IS_NORMAL_PLAY(prPsrCC->pvSptHdl)) {
		if ((INVALID_TIMESTAMP == prPsrCC->u8BaseSTC) ||
		    (prPsrCC->u8BaseSTC > u8SyncCurSTC)) {
			prPsrCC->u8BaseSTC = u8SyncCurSTC;
#ifdef __linux__
			DMXLOG_TRACE(TEXT("[PSR] %s Line %d --- (%s) Rate(%d)")
						  TEXT(" set prPsrCC->u8BaseSTC(%lldms)!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO,
				    ((prPsrFtr->eType <
				      MAX_SPT_DATA_TYPE_CNT) ? g_aszSptDataTypeName[prPsrFtr->eType]
				     : TEXT("UNKNOWN")), SplitterGetPlayRate(prPsrCC->pvSptHdl),
				    DMX_PTS_LOG_MS(prPsrCC->u8BaseSTC));
#else
			DMXLOG_TRACE(TEXT("[PSR] %s Line %d --- (%s) Rate(%d) ")
						  TEXT("set prPsrCC->u8BaseSTC(%I64dms)!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO,
				    ((prPsrFtr->eType <
				      MAX_SPT_DATA_TYPE_CNT) ? g_aszSptDataTypeName[prPsrFtr->eType]
				     : TEXT("UNKNOWN")), SplitterGetPlayRate(prPsrCC->pvSptHdl),
				    DMX_PTS_LOG_MS(prPsrCC->u8BaseSTC));
#endif
		}
	}

	if (u4AUCount <= DMX_STM_AU_CNT_THRESHOLD) {
		prPsrCC->u4Flag &= (~CCF_TIMEHOLD);
		MM_RETURN(RET_DMX_OK);
	}

	i4Rate = SplitterGetPlayRate(prPsrCC->pvSptHdl);
	prSPPsrFtr = (PSR_FILTER *) GetStreamPsrFtrByTypes(prPsrCC->pvDmxInst, SPT_DATA_SP);
	prAPsrFtr = (PSR_FILTER *) GetStreamPsrFtrByTypes(prPsrCC->pvDmxInst, SPT_DATA_A);

	if (MM_IS_NORMAL_PLAY(i4Rate)) {
		if (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS) {
			if ((NULL != prAPsrFtr) && (NULL != prAPsrFtr->pvPsrCC)) {
				if (prAPsrFtr->pvPsrCC != prPsrFtr->pvPsrCC) {
					PSR_CC *prAPsrCC = (PSR_CC *) (prAPsrFtr->pvPsrCC);

					/* Video and Audio are transferred by two different spt instances */
					if ((INVALID_TIMESTAMP == prAPsrFtr->u8LastPTS) &&
					    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS)) {
						prAPsrFtr->u8LastPTS = prPsrFtr->u8LastPTS - 1;
					} else if ((!prAPsrCC->fgCfaPrsEnd) &&
						   (INVALID_TIMESTAMP != prAPsrFtr->u8LastPTS) &&
						   (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS) &&
						   (prAPsrFtr->u8LastPTS +
						    DMX_ASYNC_PTS_DELTA <
						    prPsrFtr->u8LastPTS)) {
						PSR_CC_SetTxSt(prPsrCC,
							       TXS_WAIT_VFIFO_PTS_THRESHOLD);
						prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
						prPsrCC->pvNormalWaitFtr = prPsrFtr;
						prPsrCC->pvNormalWaitOthFtr = prAPsrFtr;
						/* Add for Check not sent EOS while playing*
						 * video in SD Card(mtk40144) */
#ifdef __linux__
						DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
							TEXT("[PSR] %s Line %d ++++++++++ ")
							 TEXT
							 ("TXS_WAIT_VFIFO_PTS_THRESHOLD(V), APts: ")
							 TEXT
							 ("%lld ms(%lld), VPts: %lld ms(%lld) ++++++++++!\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							((prAPsrFtr->u8LastPTS !=
							  INVALID_TIMESTAMP) ?
							 DMX_PTS_LOG_MS(prAPsrFtr->u8LastPTS)
							 : INVALID_TIMESTAMP), prAPsrFtr->u8LastPTS,
							((prPsrFtr->u8LastPTS !=
							  INVALID_TIMESTAMP) ?
							 DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS)
							 : INVALID_TIMESTAMP), prPsrFtr->u8LastPTS);
#else
						DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
							TEXT("[PSR] %s Line %d ++++++++++ ")
							 TEXT
							 ("TXS_WAIT_VFIFO_PTS_THRESHOLD(V), APts:")
							 TEXT
							 (" %I64d ms(%I64d), VPts: %I64d ms(%I64d) ")
							 TEXT("++++++++++!\r\n"), DMX_FUNC_NAME,
							DMX_LINE_NO,
							((prAPsrFtr->u8LastPTS !=
							  INVALID_TIMESTAMP) ?
							 DMX_PTS_LOG_MS(prAPsrFtr->u8LastPTS)
							 : INVALID_TIMESTAMP), prAPsrFtr->u8LastPTS,
							((prPsrFtr->u8LastPTS !=
							  INVALID_TIMESTAMP) ?
							 DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS)
							 : INVALID_TIMESTAMP), prPsrFtr->u8LastPTS);
#endif				/* #ifdef __linux__ */
						PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
						g_rPsrPfm.rVideo.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */

						MM_RETURN(RET_DMX_FIFO_FULL);
					}
				} else
				    if ((prPsrCC->u8BaseSTC + DMX_VSYNC_PTS_DELTA <=
					 prPsrFtr->u8LastPTS)
					&& (INVALID_TIMESTAMP != prAPsrFtr->u8LastPTS)
					&& (prPsrCC->u8BaseSTC + DMX_ASYNC_PTS_DELTA <=
					    prAPsrFtr->u8LastPTS)) {
					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
					prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
					prPsrCC->pvNormalWaitFtr = prPsrFtr;
					prPsrCC->pvNormalWaitOthFtr = NULL;

					/* Add for Check not sent EOS while playing video in SD Card(mtk40144) */
#ifdef __linux__
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT("[PSR] %s Line %d ++++++++++ ")
						 TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(V),")
						 TEXT(" VPts: %lld ms(%lld) ++++++++++!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP), prPsrFtr->u8LastPTS);
#else
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT("[PSR] %s Line %d ++++++++++ ")
						TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(V), ")
						TEXT("VPts: %I64d ms(%I64d) ++++++++++!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP), prPsrFtr->u8LastPTS);
#endif				/* #ifdef __linux__ */

					PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
					g_rPsrPfm.rVideo.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */

					MM_RETURN(RET_DMX_FIFO_FULL);
				}
			}
		}
	} else if (MM_IS_FF_PLAY(i4Rate)) {
		if ((INVALID_TIMESTAMP != prPsrFtr->u8PrevPTS) &&
		    (u8SyncCurSTC >= prPsrCC->u8BaseSTC)) {
			u64 u8ThresholdSTCL =
			    DMX_VSYNC_PTS_DELTA * DMX_ABS_VAL(i4Rate) / 2;
			u64 u8ThresholdSTCH =
			    DMX_VSYNC_PTS_DELTA * DMX_ABS_VAL(i4Rate);
			switch (i4Rate) {
			case MM_PLAY_RATE_FF_2X:
				u8ThresholdSTCL = DMX_VSYNC_PTS_DELTA;
				u8ThresholdSTCH =
				    DMX_VSYNC_PTS_DELTA * DMX_ABS_VAL(i4Rate);
				break;
			case MM_PLAY_RATE_FF_4X:
				u8ThresholdSTCL = DMX_VSYNC_PTS_DELTA * 2;
				u8ThresholdSTCH = DMX_VSYNC_PTS_DELTA * 3;
				break;
			case MM_PLAY_RATE_FF_8X:
				u8ThresholdSTCL = DMX_VSYNC_PTS_DELTA * 3;
				u8ThresholdSTCH = DMX_VSYNC_PTS_DELTA * 4;
				break;
			case MM_PLAY_RATE_FF_16X:
				u8ThresholdSTCL = DMX_VSYNC_PTS_DELTA * 5;
				u8ThresholdSTCH = DMX_VSYNC_PTS_DELTA * 10;
				break;
			case MM_PLAY_RATE_FF_32X:
				u8ThresholdSTCL = DMX_VSYNC_PTS_DELTA * 10;
				u8ThresholdSTCH = DMX_VSYNC_PTS_DELTA * 20;
				break;
			default:
				break;
			}

			if (prPsrCC->u4Flag & CCF_TIMEHOLD) {
				if (prPsrFtr->u8PrevPTS - prPsrFtr->u81stPTS <
				    (u8SyncCurSTC - prPsrCC->u8BaseSTC) + u8ThresholdSTCL) {
					prPsrCC->u4Flag &= (~CCF_TIMEHOLD);
				} else {
					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
					prPsrCC->u8NormalWaitPts = prPsrFtr->u8PrevPTS;
					prPsrCC->pvNormalWaitFtr = prPsrFtr;
					prPsrCC->pvNormalWaitOthFtr = prVPsrFtr;

#ifdef __linux__
					DMXLOG_DEBUG(
						    TEXT("[PSR] %s Line %d --- (Video) Wait, ")
						     TEXT
						     ("STC(%lld ms), BaseSTC(%lld ms), u8PrevPTS(%lld ms),")
						     TEXT(" u81stPts(%lld ms)!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    DMX_PTS_LOG_MS(u8SyncCurSTC),
						    DMX_PTS_LOG_MS(prPsrCC->u8BaseSTC),
						    DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS),
						    DMX_PTS_LOG_MS(prPsrFtr->u81stPTS));
#else
					DMXLOG_DEBUG(
						    TEXT("[PSR] %s Line %d --- (Video) Wait,")
						     TEXT(" STC(%I64d ms), BaseSTC(%I64dms), ")
						     TEXT
						     ("u8PrevPTS(%I64d), u81stPts(%I64d)!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    DMX_PTS_LOG_MS(u8SyncCurSTC),
						    DMX_PTS_LOG_MS(prPsrCC->u8BaseSTC),
						    DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS),
						    DMX_PTS_LOG_MS(prPsrFtr->u81stPTS));
#endif				/* __linux__ */

					/* Add for Check not sent EOS while playing video in SD Card(mtk40144) */
#ifdef __linux__
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT("[PSR] %s Line %d ++++++++++ ")
						 TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(SP), ")
						 TEXT
						 ("VPts: %lld ms(%lld), SPPts: %lld ms(%lld)!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prVPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prVPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP), prVPsrFtr->u8LastPTS,
						((prPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP), prPsrFtr->u8LastPTS);
#else
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT("[PSR] %s Line %d ++++++++++ ")
						TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(SP), ")
						TEXT
						("VPts: %I64d ms(%I64d), SPPts: %I64d ms(%I64d)!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prVPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prVPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP), prVPsrFtr->u8LastPTS,
						((prPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP), prPsrFtr->u8LastPTS);
#endif				/* #ifdef __linux__ */
					prPsrCC->u4Flag |= CCF_TIMEHOLD;
					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);

					PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
					g_rPsrPfm.rSP.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */
					MM_RETURN(RET_DMX_FIFO_FULL);
				}
			} else {
				if (prPsrFtr->u8PrevPTS - prPsrFtr->u81stPTS >
				    (u8SyncCurSTC - prPsrCC->u8BaseSTC) + u8ThresholdSTCH) {
					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
					prPsrCC->u8NormalWaitPts = prPsrFtr->u8PrevPTS;
					prPsrCC->pvNormalWaitFtr = prPsrFtr;
					prPsrCC->pvNormalWaitOthFtr = prVPsrFtr;

					/* Add for Check not sent EOS while playing video in SD Card(mtk40144) */
#ifdef __linux__
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT("[PSR] %s Line %d ++++++++++ ")
						TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(SP), ")
						TEXT
						("VPts: %lld ms(%lld), SPPts: %lld ms(%lld)!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prVPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prVPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP), prVPsrFtr->u8LastPTS,
						((prPsrFtr->u8PrevPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS) :
						 INVALID_TIMESTAMP), prPsrFtr->u8PrevPTS);
#else
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT("[PSR] %s Line %d ++++++++++ ")
						 TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(SP), ")
						 TEXT
						 ("VPts: %I64d ms(%I64d), SPPts: %I64d ms(%I64d)!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prVPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prVPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP), prVPsrFtr->u8LastPTS,
						((prPsrFtr->u8PrevPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS) :
						 INVALID_TIMESTAMP), prPsrFtr->u8PrevPTS);
#endif				/* #ifdef __linux__ */

					prPsrCC->u4Flag |= CCF_TIMEHOLD;
					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);

					PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
					g_rPsrPfm.rSP.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */
					MM_RETURN(RET_DMX_FIFO_FULL);
				}
			}
		}

		if (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS) {
			if ((NULL != prSPPsrFtr) &&
			    (NULL != prSPPsrFtr->pvPsrCC) &&
			    (prSPPsrFtr->pvPsrCC != prPsrFtr->pvPsrCC)) {
				PSR_CC *prSPPsrCC = (PSR_CC *) (prSPPsrFtr->pvPsrCC);
				/* Video and SP are transferred by two different spt instances */
				if ((INVALID_TIMESTAMP == prSPPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS)) {
					prSPPsrFtr->u8LastPTS = prPsrFtr->u8LastPTS - 1;
				}
				if ((!prSPPsrCC->fgCfaPrsEnd) &&
				    (INVALID_TIMESTAMP != prSPPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS) &&
				    (prSPPsrFtr->u8LastPTS +
				     DMX_SPSYNC_PTS_DELTA * ((u32) i4Rate) <
				     prPsrFtr->u8LastPTS)) {

#ifdef __linux__
					DMXLOG_DEBUG(
						    TEXT("[PSR] %s Line %d --- (Video) Wait, ")
						     TEXT
						     ("SP'sLastPts(%lld ms), Video's u8LastPTS(%lld)!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    prSPPsrFtr->u8LastPTS, prPsrFtr->u8LastPTS);
#else
					DMXLOG_DEBUG(
						    TEXT("[PSR] %s Line %d --- (Video) Wait, ")
						     TEXT
						     ("SP'sLastPts(%I64d ms), Video's u8LastPTS(%I64d)!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    prSPPsrFtr->u8LastPTS, prPsrFtr->u8LastPTS);
#endif				/* __linux__ */

					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
					prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
					prPsrCC->pvNormalWaitFtr = prPsrFtr;
					prPsrCC->pvNormalWaitOthFtr = prSPPsrCC;

					/* Add for Check not sent EOS while playing video in SD Card(mtk40144) */
#ifdef __linux__
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT
						 ("[PSR] %s Line %d ++++++++++ TXS_WAIT_VFIFO_PTS_THRESHOLD(V),")
						 TEXT
						 (" SPPts: %lld ms(%lld), VPts: %lld ms(%lld)!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prSPPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prSPPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP), prSPPsrFtr->u8LastPTS,
						((prPsrFtr->u8PrevPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS) :
						 INVALID_TIMESTAMP), prPsrFtr->u8PrevPTS);
#else
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT
						 ("[PSR] %s Line %d ++++++++++ TXS_WAIT_VFIFO_PTS_THRESHOLD(V),")
						 TEXT
						 (" SPPts: %I64d ms(%I64d), VPts: %I64d ms(%I64d) ++++++++++!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prSPPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prSPPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP), prSPPsrFtr->u8LastPTS,
						((prPsrFtr->u8PrevPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS) :
						 INVALID_TIMESTAMP), prPsrFtr->u8PrevPTS);
#endif				/* #ifdef __linux__ */

					PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
					g_rPsrPfm.rVideo.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */

					MM_RETURN(RET_DMX_FIFO_FULL);
				}
			}
		}
	} else if (MM_IS_RW_PLAY(i4Rate)) {
		if (INVALID_TIMESTAMP != prPsrFtr->u8PrevPTS) {
			if (u8SyncCurSTC >= prPsrCC->u8BaseSTC) {
				u64 u8ThresholdSTC =
				    DMX_VSYNC_PTS_DELTA * DMX_ABS_VAL(i4Rate);
				switch (i4Rate) {
				case MM_PLAY_RATE_FF_2X:
					u8ThresholdSTC = DMX_VSYNC_PTS_DELTA;
					break;
				case MM_PLAY_RATE_FF_4X:
					u8ThresholdSTC = DMX_VSYNC_PTS_DELTA * 2;
					break;
				case MM_PLAY_RATE_FF_8X:
					u8ThresholdSTC = DMX_VSYNC_PTS_DELTA * 3;
					break;
				case MM_PLAY_RATE_FF_16X:
					u8ThresholdSTC = DMX_VSYNC_PTS_DELTA * 5;
					break;
				case MM_PLAY_RATE_FF_32X:
					u8ThresholdSTC = DMX_VSYNC_PTS_DELTA * 10;
					break;
				default:
					break;
				}

				if (prPsrFtr->u81stPTS - prPsrFtr->u8PrevPTS >
				    (u8SyncCurSTC - prPsrCC->u8BaseSTC) + u8ThresholdSTC) {
					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
					prPsrCC->u8NormalWaitPts = prPsrFtr->u8PrevPTS;
					prPsrCC->pvNormalWaitFtr = prPsrFtr;
					prPsrCC->pvNormalWaitOthFtr = prVPsrFtr;

#ifdef __linux__
					DMXLOG_DEBUG(
						    TEXT
						     ("[PSR] %s Line %d --- (Video) Wait, STC(%lld ms),")
						     TEXT(" BaseSTC(%lld ms), u8PrevPTS(%lld ms), ")
						     TEXT("u81stPts(%lld ms)!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    DMX_PTS_LOG_MS(u8SyncCurSTC),
						    DMX_PTS_LOG_MS(prPsrCC->u8BaseSTC),
						    DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS),
						    DMX_PTS_LOG_MS(prPsrFtr->u81stPTS));
#else
					DMXLOG_DEBUG(
						    TEXT
						     ("[PSR] %s Line %d --- (Video) Wait, STC(%I64d ms),")
						     TEXT(" BaseSTC(%I64dms), u8PrevPTS(%I64d), ")
						     TEXT("u81stPts(%I64d)!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    DMX_PTS_LOG_MS(u8SyncCurSTC),
						    DMX_PTS_LOG_MS(prPsrCC->u8BaseSTC),
						    DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS),
						    DMX_PTS_LOG_MS(prPsrFtr->u81stPTS));
#endif				/* __linux__ */

					/* Add for Check not sent EOS while playing video in SD Card(mtk40144) */
#ifdef __linux__
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT
						 ("[PSR] %s Line %d ++++++++++ TXS_WAIT_VFIFO_PTS_THRESHOLD(SP),")
						 TEXT
						 (" VPts: %lld ms(%lld), SPPts: %lld ms(%lld)!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prVPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prVPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP), prVPsrFtr->u8LastPTS,
						((prPsrFtr->u8PrevPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS) :
						 INVALID_TIMESTAMP), prPsrFtr->u8PrevPTS);
#else
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT
						 ("[PSR] %s Line %d ++++++++++ TXS_WAIT_VFIFO_PTS_THRESHOLD(SP),")
						 TEXT
						 (" VPts: %I64d ms(%I64d), SPPts: %I64d ms(%I64d) ++++++++++!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prVPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prVPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP), prVPsrFtr->u8LastPTS,
						((prPsrFtr->u8PrevPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS) :
						 INVALID_TIMESTAMP), prPsrFtr->u8PrevPTS);
#endif				/* #ifdef __linux__ */

					PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
					g_rPsrPfm.rSP.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */
					MM_RETURN(RET_DMX_FIFO_FULL);
				} else {
#ifdef __linux__
					DMXLOG_DEBUG(
						    TEXT
						     ("[PSR] %s Line %d --- (Video) STC(%lld ms),")
						     TEXT(" BaseSTC(%lld ms), u8PrevPTS(%lld ms), ")
						     TEXT("u81stPts(%lld ms)!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    DMX_PTS_LOG_MS(u8SyncCurSTC),
						    DMX_PTS_LOG_MS(prPsrCC->u8BaseSTC),
						    DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS),
						    DMX_PTS_LOG_MS(prPsrFtr->u81stPTS));
#else
					DMXLOG_DEBUG(
						    TEXT
						     ("[PSR] %s Line %d --- (Video) STC(%I64d ms),")
						     TEXT
						     (" BaseSTC(%I64dms), u8PrevPTS(%I64d), u81stPts(%I64d)!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    DMX_PTS_LOG_MS(u8SyncCurSTC),
						    DMX_PTS_LOG_MS(prPsrCC->u8BaseSTC),
						    DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS),
						    DMX_PTS_LOG_MS(prPsrFtr->u81stPTS));
#endif				/* __linux__ */
				}
			}
		}

		if ((NULL != prSPPsrFtr) &&
		    (NULL != prSPPsrFtr->pvPsrCC) &&
		    (prSPPsrFtr->pvPsrCC != prPsrFtr->pvPsrCC) &&
		    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS)) {
			PSR_CC *prSPPsrCC = (PSR_CC *) (prSPPsrFtr->pvPsrCC);
			/* Video and SP are transferred by two different spt instances */
			if (INVALID_TIMESTAMP == prSPPsrFtr->u8LastPTS)
				prSPPsrFtr->u8LastPTS = prPsrFtr->u8LastPTS + 1;

			if ((!prSPPsrCC->fgCfaPrsEnd) &&
			    (INVALID_TIMESTAMP != prSPPsrFtr->u8LastPTS) &&
			    (prSPPsrFtr->u8LastPTS >
			     prPsrFtr->u8LastPTS +
			     DMX_SPSYNC_PTS_DELTA * ((u32) i4Rate))) {
				PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
				prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
				prPsrCC->pvNormalWaitFtr = prPsrFtr;
				prPsrCC->pvNormalWaitOthFtr = prSPPsrCC;

#ifdef __linux__
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
					TEXT
					 ("[PSR] %s Line %d ++++++++++ TXS_WAIT_VFIFO_PTS_THRESHOLD(V),")
					 TEXT(" SPPts: %lld ms(%lld), VPts: %lld ms(%lld)!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					((prSPPsrFtr->u8LastPTS !=
					  INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prSPPsrFtr->u8LastPTS)
					 : INVALID_TIMESTAMP), prSPPsrFtr->u8LastPTS,
					((prPsrFtr->u8PrevPTS !=
					  INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS) :
					 INVALID_TIMESTAMP), prPsrFtr->u8PrevPTS);
#else
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
					TEXT
					 ("[PSR] %s Line %d ++++++++++ TXS_WAIT_VFIFO_PTS_THRESHOLD(V),")
					 TEXT
					 (" SPPts: %I64d ms(%I64d), VPts: %I64d ms(%I64d) ++++++++++!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					((prSPPsrFtr->u8LastPTS !=
					  INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prSPPsrFtr->u8LastPTS)
					 : INVALID_TIMESTAMP), prSPPsrFtr->u8LastPTS,
					((prPsrFtr->u8PrevPTS !=
					  INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS) :
					 INVALID_TIMESTAMP), prPsrFtr->u8PrevPTS);
#endif				/* #ifdef __linux__ */

				PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
				g_rPsrPfm.rVideo.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */

				MM_RETURN(RET_DMX_FIFO_FULL);
			}
		}
	}

	if ((!MM_IS_NORMAL_PLAY(i4Rate)) &&
	    (1 == prPsrCC->u4PsrFtrCnt) &&
	    (u4AUCount * DMX_PTS_1S / 60 >=
	     DMX_VSYNC_PTS_DELTA * ((u32) DMX_ABS_VAL(i4Rate)))) {
		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
		prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
		prPsrCC->pvNormalWaitFtr = prPsrFtr;
		prPsrCC->pvNormalWaitOthFtr = NULL;
		/* Add for Check not sent EOS while playing video in SD Card(mtk40144) */
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
			TEXT("[PSR] %s Line %d ++++++++++ ")
			 TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(V), u4AUCount: %d!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4AUCount);
		PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
		g_rPsrPfm.rVideo.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */

		MM_RETURN(RET_DMX_FIFO_FULL);
	}

	prPsrCC->u8NormalWaitPts = INVALID_TIMESTAMP;
	prPsrCC->pvNormalWaitFtr = NULL;
	prPsrCC->pvNormalWaitOthFtr = NULL;

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_Filter_TxDecideA(PSR_FILTER *prPsrFtr, PSR_CC *prPsrCC)
{
	MRESULT mrRet = RET_DMX_OK;
	u32 u4FifoData = 0, u4FifoSize = 0, u4FifoUsage = 0;
	u32 u4AUCount = 0;
	s32 i4Rate = 1;
	PSR_FILTER *prVPsrFtr = NULL;
	DMX_INST_T *prDmxInst = NULL;
	u64 u8SyncCurSTC = INVALID_TIMESTAMP;

	prDmxInst = (DMX_INST_T *)prPsrFtr->pvDmxInst;

	if (NULL == prDmxInst) {
		DMX_ASSERT(0);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	/* Add for sony http stream, no hold tx, @2009/08/24 */
	if (SPT_DATA_A != prPsrFtr->eType) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d (ESIH: 0x%x) ")
					  TEXT("fail in PsrFtr's eType(%d) != Audio\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, prPsrFtr->eType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	u4FifoSize = prPsrFtr->u4ESFifoSize;

	mrRet = ESM_FifoGetAvailDataSize(prPsrFtr->u4ESIH, &u4FifoData);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d (ESIH: 0x%x) fail ")
					  TEXT("in ESM_FifoGetAvailDataSize, mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (0 < u4FifoSize) {
		u4FifoUsage = u4FifoData * 100 / u4FifoSize;
	} else {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s line %d - u4FifoSize: %d, u4FifoData: %d\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u4FifoSize, u4FifoData);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_OPERATE_FORBID);
	}

	mrRet = ESM_AUTableGetAvailCount(prPsrFtr->u4ESIH, &u4AUCount);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ESM_AUTableGetAvailCount")
					  TEXT("(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if ((prDmxInst->u4SptCnt <= 1) && (DMX_IS_NORMAL_PLAY(prPsrCC->pvSptHdl)))
		MM_RETURN(RET_DMX_OK);

	if ((SplitterRspIsEnabled(prPsrCC->pvSptHdl) && SplitterIsRspOffStart(prPsrCC->pvSptHdl)))
		MM_RETURN(RET_DMX_OK);

	STC_HalGetTime(0, &u8SyncCurSTC);
	if (u8SyncCurSTC > 0x01ffffffffl)
		u8SyncCurSTC = 0;

	if (!DMX_IS_NORMAL_PLAY(prPsrCC->pvSptHdl)) {
		if ((INVALID_TIMESTAMP == prPsrCC->u8BaseSTC) ||
		    (prPsrCC->u8BaseSTC > u8SyncCurSTC)) {
			prPsrCC->u8BaseSTC = u8SyncCurSTC;
#ifdef __linux__
			DMXLOG_TRACE(TEXT("[PSR] %s Line %d --- (%s) Rate(%d)")
						  TEXT(" set prPsrCC->u8BaseSTC(%lld ms)!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO,
				    ((prPsrFtr->eType <
				      MAX_SPT_DATA_TYPE_CNT) ? g_aszSptDataTypeName[prPsrFtr->eType]
				     : TEXT("UNKNOWN")), SplitterGetPlayRate(prPsrCC->pvSptHdl),
				    DMX_PTS_LOG_MS(prPsrCC->u8BaseSTC));
#else
			DMXLOG_TRACE(TEXT("[PSR] %s Line %d --- (%s) Rate(%d) ")
						  TEXT("set prPsrCC->u8BaseSTC(%I64d ms)!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO,
				    ((prPsrFtr->eType <
				      MAX_SPT_DATA_TYPE_CNT) ? g_aszSptDataTypeName[prPsrFtr->eType]
				     : TEXT("UNKNOWN")), SplitterGetPlayRate(prPsrCC->pvSptHdl),
				    DMX_PTS_LOG_MS(prPsrCC->u8BaseSTC));
#endif				/* __linux__ */
		}
	}
#if CONFIG_DRV_HIGH_BITRATE_SPECIAL_PROC
	if (prPsrCC->fgNeedHighBitRateProc)
		MM_RETURN(RET_DMX_OK);
#endif

	i4Rate = SplitterGetPlayRate(prPsrCC->pvSptHdl);

	if (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS) {
		prVPsrFtr = (PSR_FILTER *) GetStreamPsrFtrByTypes(prPsrCC->pvDmxInst, SPT_DATA_V);

		if (MM_IS_NORMAL_PLAY(i4Rate)) {
			PSR_CC *prVPsrCC =
			    (PSR_CC *) ((NULL != prVPsrFtr) ? (prVPsrFtr->pvPsrCC) : NULL);
			if ((NULL != prVPsrCC) && (!(prVPsrCC->fgCfaPrsEnd))
			    && (prVPsrCC != prPsrFtr->pvPsrCC)) {

				if ((INVALID_TIMESTAMP == prVPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS)) {
					prVPsrFtr->u8LastPTS = prPsrFtr->u8LastPTS - 1;
				}

				if ((INVALID_TIMESTAMP != prVPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrCC->u8BaseSTC) &&
				    ((prVPsrFtr->u8LastPTS + DMX_VSYNC_PTS_DELTA <
				      prPsrFtr->u8LastPTS)
				     || (prPsrCC->u8BaseSTC + DMX_ASYNC_PTS_DELTA <=
					 prPsrFtr->u8LastPTS))) {
					if (!((u4AUCount < AUD_AU_CNT_THRESHOLD)
					      && (u4FifoUsage < AUD_FIFO_USAGE_RATE_THRESHOLD))) {
						PSR_CC_SetTxSt(prPsrCC,
							       TXS_WAIT_VFIFO_PTS_THRESHOLD);
						prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
						prPsrCC->pvNormalWaitFtr = prPsrFtr;
						prPsrCC->pvNormalWaitOthFtr = prVPsrFtr;

#ifdef __linux__
						DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
							TEXT("[PSR] %s Line %d ++++++++++ ")
							 TEXT
							 ("TXS_WAIT_VFIFO_PTS_THRESHOLD(A), VPts:")
							 TEXT
							 (" %lld ms(%lld), APts: %lld ms(%lld)!\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							((prVPsrFtr->u8LastPTS !=
							  INVALID_TIMESTAMP) ?
							 DMX_PTS_LOG_MS(prVPsrFtr->u8LastPTS)
							 : INVALID_TIMESTAMP), prVPsrFtr->u8LastPTS,
							((prPsrFtr->u8PrevPTS !=
							  INVALID_TIMESTAMP) ?
							 DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS)
							 : INVALID_TIMESTAMP), prPsrFtr->u8PrevPTS);
#else
						DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
							TEXT("[PSR] %s Line %d ++++++++++ ")
							 TEXT
							 ("TXS_WAIT_VFIFO_PTS_THRESHOLD(A), VPts: ")
							 TEXT
							 ("%I64d ms(%I64d), APts: %I64d ms(%I64d) ++++++++++!\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							((prVPsrFtr->u8LastPTS !=
							  INVALID_TIMESTAMP) ?
							 DMX_PTS_LOG_MS(prVPsrFtr->u8LastPTS)
							 : INVALID_TIMESTAMP), prVPsrFtr->u8LastPTS,
							((prPsrFtr->u8PrevPTS !=
							  INVALID_TIMESTAMP) ?
							 DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS)
							 : INVALID_TIMESTAMP), prPsrFtr->u8PrevPTS);
#endif				/* #ifdef __linux__ */

						PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
						g_rPsrPfm.rAudio.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */

						MM_RETURN(RET_DMX_FIFO_FULL);
					}
				}
			}
		}
	}

	prPsrCC->u8NormalWaitPts = INVALID_TIMESTAMP;
	prPsrCC->pvNormalWaitFtr = NULL;
	prPsrCC->pvNormalWaitOthFtr = NULL;

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_Filter_TxDecideSP(PSR_FILTER *prPsrFtr, PSR_CC *prPsrCC)
{
	MRESULT mrRet = RET_DMX_OK;
	u32 u4FifoData = 0, u4FifoSize = 0, u4FifoUsage = 0;
	u32 u4AUCount = 0;
	s32 i4Rate = 1;
	PSR_FILTER *prVPsrFtr = NULL;
	PSR_FILTER *prSPPsrFtr = NULL;
	u32 u4WrIdx = ESM_INVALID_INDEX;
	PSR_CC *prVPsrCC = NULL;
	DMX_INST_T *prDmxInst = NULL;
	u64 u8SyncCurSTC = INVALID_TIMESTAMP;

	prDmxInst = (DMX_INST_T *)prPsrFtr->pvDmxInst;

	if (NULL == prDmxInst) {
		DMX_ASSERT(0);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	/* Add for sony http stream, no hold tx, @2009/08/24 */
	if (SPT_DATA_SP != prPsrFtr->eType) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d (ESIH: 0x%x) ")
					  TEXT("fail in PsrFtr's eType(%d) != Audio\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, prPsrFtr->eType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	u4FifoSize = prPsrFtr->u4ESFifoSize;

	mrRet = ESM_FifoGetAvailDataSize(prPsrFtr->u4ESIH, &u4FifoData);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d (ESIH: 0x%x) fail ")
					  TEXT("in ESM_FifoGetAvailDataSize, mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (0 < u4FifoSize) {
		u4FifoUsage = u4FifoData * 100 / u4FifoSize;
	} else {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s line %d - u4FifoSize: %d, u4FifoData: %d\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u4FifoSize, u4FifoData);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_OPERATE_FORBID);
	}

	mrRet = ESM_AUTableGetAvailCount(prPsrFtr->u4ESIH, &u4AUCount);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ESM_AUTableGetAvail")
					  TEXT("Count(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}
	mrRet = ESM_AUTableGetWrIdx(prPsrFtr->u4ESIH, &u4WrIdx);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ESM_AUTableGetAvail")
					  TEXT("Count(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	STC_HalGetTime(0, &u8SyncCurSTC);
	if (u8SyncCurSTC > 0x01ffffffffl)
		u8SyncCurSTC = 0;

	if ((prDmxInst->u4SptCnt <= 1) && (DMX_IS_NORMAL_PLAY(prPsrCC->pvSptHdl)))
		MM_RETURN(RET_DMX_OK);

	if ((SplitterRspIsEnabled(prPsrCC->pvSptHdl) && SplitterIsRspOffStart(prPsrCC->pvSptHdl)))
		MM_RETURN(RET_DMX_OK);

	if (!DMX_IS_NORMAL_PLAY(prPsrCC->pvSptHdl)) {
		if ((INVALID_TIMESTAMP == prPsrCC->u8BaseSTC) ||
		    (prPsrCC->u8BaseSTC > u8SyncCurSTC)) {
			prPsrCC->u8BaseSTC = u8SyncCurSTC;
			DMXLOG_TRACE(TEXT("[PSR] %s Line %d --- (%s) Rate(%d)")
				    TEXT(" set prPsrCC->u8BaseSTC(%I64dms), u4AUCount(%d)!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO,
				    ((prPsrFtr->eType <
				      MAX_SPT_DATA_TYPE_CNT) ? g_aszSptDataTypeName[prPsrFtr->eType]
				     : TEXT("UNKNOWN")), SplitterGetPlayRate(prPsrCC->pvSptHdl),
				    DMX_PTS_LOG_MS(prPsrCC->u8BaseSTC));
		}
	}
#if CONFIG_DRV_HIGH_BITRATE_SPECIAL_PROC
	if (prPsrCC->fgNeedHighBitRateProc)
		MM_RETURN(RET_DMX_OK);
#endif

	i4Rate = SplitterGetPlayRate(prPsrCC->pvSptHdl);

	prVPsrFtr = (PSR_FILTER *) GetStreamPsrFtrByTypes(prPsrCC->pvDmxInst, SPT_DATA_V);

	/* prAPsrFtr = (PSR_FILTER *)GetStreamPsrFtrByTypes(prPsrCC->pvDmxInst, SPT_DATA_A); */
	if ((NULL != prVPsrFtr) &&
	    (NULL != prVPsrFtr->pvPsrCC) && (prVPsrFtr->pvPsrCC != prPsrFtr->pvPsrCC)) {
		prVPsrCC = (PSR_CC *) (prVPsrFtr->pvPsrCC);
	}

	if (MM_IS_NORMAL_PLAY(i4Rate)) {
		if (u4AUCount <= 1)
			MM_RETURN(RET_DMX_OK);

		if (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS) {
			if ((NULL != prVPsrFtr) &&
			    (NULL != prVPsrFtr->pvPsrCC) &&
			    (prVPsrFtr->pvPsrCC != prPsrFtr->pvPsrCC)) {
				if ((INVALID_TIMESTAMP == prVPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS)) {
					prVPsrFtr->u8LastPTS = prPsrFtr->u8LastPTS - 1;
				}

				if ((!prVPsrCC->fgCfaPrsEnd) &&
				    (INVALID_TIMESTAMP != prVPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrCC->u8BaseSTC) &&
				    ((prVPsrFtr->u8LastPTS + DMX_SPSYNC_PTS_DELTA <
				      prPsrFtr->u8LastPTS)
				     || (prPsrCC->u8BaseSTC + DMX_SPSYNC_PTS_DELTA <=
					 prPsrFtr->u8LastPTS))) {
					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
					prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
					prPsrCC->pvNormalWaitFtr = prPsrFtr;
					prPsrCC->pvNormalWaitOthFtr = prVPsrFtr;

#ifdef __linux__
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT
						 ("[PSR] %s Line %d ++++++++++ TXS_WAIT_VFIFO_PTS")
						 TEXT
						 ("_THRESHOLD(SP), VPts: %lld ms(%lld), SPPts: %lld ms(%lld)!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prVPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prVPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP), prVPsrFtr->u8LastPTS,
						((prPsrFtr->u8PrevPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS) :
						 INVALID_TIMESTAMP), prPsrFtr->u8PrevPTS);
#else
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT("[PSR] %s Line %d ++++++++++ ")
						 TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(SP), VPts: ")
						 TEXT
						 ("%I64d ms(%I64d), SPPts: %I64d ms(%I64d) ++++++++++!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prVPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prVPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP), prVPsrFtr->u8LastPTS,
						((prPsrFtr->u8PrevPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS) :
						 INVALID_TIMESTAMP), prPsrFtr->u8PrevPTS);
#endif				/* #ifdef __linux__ */

					PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
					g_rPsrPfm.rSP.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */
					MM_RETURN(RET_DMX_FIFO_FULL);
				} else if ((u4AUCount < SP_AU_CNT_THRESHOLD) &&
					   (u4FifoUsage < SP_FIFO_USAGE_RATE_THRESHOLD)) {
					DMXLOG_DEBUG(
						    TEXT("[PSR] TxDecide, too few sp, ")
						     TEXT("au:0x%x, usg:0x%x,  no hold!\r\n"),
						    u4AUCount, u4FifoUsage);
				}
			}
		}
	} else if (MM_IS_FF_PLAY(i4Rate)) {
		if (u4AUCount <= 1)
			MM_RETURN(RET_DMX_OK);

		if (INVALID_TIMESTAMP != prPsrFtr->u8PrevPTS) {
			if (u8SyncCurSTC >= prPsrCC->u8BaseSTC) {
				if (prPsrFtr->u8PrevPTS - prPsrFtr->u81stPTS >
				    (u8SyncCurSTC - prPsrCC->u8BaseSTC) +
				    DMX_VSYNC_PTS_DELTA * DMX_ABS_VAL(i4Rate)) {
					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
					prPsrCC->u8NormalWaitPts = prPsrFtr->u8PrevPTS;
					prPsrCC->pvNormalWaitFtr = prPsrFtr;
					prPsrCC->pvNormalWaitOthFtr = prVPsrFtr;

#ifdef __linux__
					DMXLOG_DEBUG(
						    TEXT
						     ("[PSR] %s Line %d --- (SP) STC(%lld ms), ")
						     TEXT("BaseSTC(%lld ms), u8PrevPTS(%lld ms),")
						     TEXT(" u81stPts(%lld ms)!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    DMX_PTS_LOG_MS(u8SyncCurSTC),
						    DMX_PTS_LOG_MS(prPsrCC->u8BaseSTC),
						    DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS),
						    DMX_PTS_LOG_MS(prPsrFtr->u81stPTS));
#else
					DMXLOG_DEBUG(
						    TEXT
						     ("[PSR] %s Line %d --- (SP) STC(%I64d ms),")
						     TEXT(" BaseSTC(%I64dms), u8PrevPTS(%I64d ms),")
						     TEXT(" u81stPts(%I64d ms)!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    DMX_PTS_LOG_MS(u8SyncCurSTC),
						    DMX_PTS_LOG_MS(prPsrCC->u8BaseSTC),
						    DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS),
						    DMX_PTS_LOG_MS(prPsrFtr->u81stPTS));
#endif				/* #ifdef __linux__ */

#if DMX_PRINT_FIFO_FULL_LOG
					/* Add for Check not sent EOS while playing video in SD Card(mtk40144) */
#ifdef __linux__
					DMXLOG_ERROR(
						    TEXT("[PSR] %s Line %d ++++++++++ ")
						     TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(SP), ")
						     TEXT("VPts: %lld, SPPts: %lld!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    prVPsrFtr->u8PrevPTS, prPsrFtr->u8PrevPTS);
#else
					DMXLOG_ERROR(
						    TEXT("[PSR] %s Line %d ++++++++++ ")
						     TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(SP), ")
						     TEXT("VPts: %I64d, SPPts: %I64d!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    prVPsrFtr->u8PrevPTS, prPsrFtr->u8PrevPTS);
#endif				/* #ifdef __linux__ */
#endif
					PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
					g_rPsrPfm.rSP.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */
					MM_RETURN(RET_DMX_FIFO_FULL);
				}
			}
		}
		if (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS) {
			if ((NULL != prVPsrFtr) &&
			    (NULL != prVPsrFtr->pvPsrCC) &&
			    (prVPsrFtr->pvPsrCC != prPsrFtr->pvPsrCC)) {
				prVPsrCC = (PSR_CC *) (prVPsrFtr->pvPsrCC);
				if ((INVALID_TIMESTAMP == prVPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS)) {
					prVPsrFtr->u8LastPTS = prPsrFtr->u8LastPTS - 1;
				}

				if ((!prVPsrCC->fgCfaPrsEnd) &&
				    (INVALID_TIMESTAMP != prVPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS) &&
				    (prVPsrFtr->u8LastPTS +
				     DMX_VSYNC_PTS_DELTA * ((u32) i4Rate) <
				     prPsrFtr->u8LastPTS)) {
#ifdef __linux__
					DMXLOG_DEBUG(
						    TEXT
						     ("[PSR] %s Line %d --- (SP) Wait, STC(%lld ms),")
						     TEXT
						     (" BaseSTC(%lldms), u8PrevPTS(%lld), u81stPts(%lld)!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    DMX_PTS_LOG_MS(u8SyncCurSTC),
						    DMX_PTS_LOG_MS(prPsrCC->u8BaseSTC),
						    DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS),
						    DMX_PTS_LOG_MS(prPsrFtr->u81stPTS));
#else
					DMXLOG_DEBUG(
						    TEXT("[PSR] %s Line %d --- (SP) Wait, ")
						     TEXT("STC(%I64d ms), BaseSTC(%I64dms), ")
						     TEXT
						     ("u8PrevPTS(%I64d), u81stPts(%I64d)!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    DMX_PTS_LOG_MS(u8SyncCurSTC),
						    DMX_PTS_LOG_MS(prPsrCC->u8BaseSTC),
						    DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS),
						    DMX_PTS_LOG_MS(prPsrFtr->u81stPTS));
#endif				/* #ifdef __linux__ */

					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
					prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
					prPsrCC->pvNormalWaitFtr = prPsrFtr;
					prPsrCC->pvNormalWaitOthFtr = prVPsrFtr;

#ifdef __linux__
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT("[PSR] %s Line %d ++++++++++ ")
						 TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(SP), VPts: ")
						 TEXT("%lld ms(%lld), SPPts: %lld ms(%lld)!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prVPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prVPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP), prVPsrFtr->u8LastPTS,
						((prPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP), prPsrFtr->u8LastPTS);
#else
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT("[PSR] %s Line %d ++++++++++ ")
						 TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(SP), VPts: ")
						 TEXT
						 ("%I64d ms(%I64d), SPPts: %I64d ms(%I64d) ++++++++++!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prVPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prVPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP), prVPsrFtr->u8LastPTS,
						((prPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP), prPsrFtr->u8LastPTS);
#endif				/* #ifdef __linux__ */

					PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
					g_rPsrPfm.rSP.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */
					MM_RETURN(RET_DMX_FIFO_FULL);
				}
			}
		}
	} else if (MM_IS_RW_PLAY(i4Rate)) {
		if (u4AUCount <= 1)
			MM_RETURN(RET_DMX_OK);

		if (INVALID_TIMESTAMP != prPsrFtr->u8PrevPTS) {
			if (u8SyncCurSTC >= prPsrCC->u8BaseSTC) {
				if (prPsrFtr->u81stPTS - prPsrFtr->u8PrevPTS >
				    (u8SyncCurSTC - prPsrCC->u8BaseSTC) +
				    DMX_VSYNC_PTS_DELTA * DMX_ABS_VAL(i4Rate)) {
					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
					prPsrCC->u8NormalWaitPts = prPsrFtr->u8PrevPTS;
					prPsrCC->pvNormalWaitFtr = prPsrFtr;
					prPsrCC->pvNormalWaitOthFtr = prVPsrFtr;

#ifdef __linux__
					DMXLOG_DEBUG(
						    TEXT("[PSR] %s Line %d --- (SP) STC(%lld ms),")
						     TEXT(" BaseSTC(%lld ms), u8PrevPTS(%lld ms), ")
						     TEXT("u81stPts(%lld ms)!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    DMX_PTS_LOG_MS(u8SyncCurSTC),
						    DMX_PTS_LOG_MS(prPsrCC->u8BaseSTC),
						    DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS),
						    DMX_PTS_LOG_MS(prPsrFtr->u81stPTS));
#else
					DMXLOG_DEBUG(
						    TEXT
						     ("[PSR] %s Line %d --- (SP) STC(%I64d ms),")
						     TEXT
						     (" BaseSTC(%I64dms), u8PrevPTS(%I64d), u81stPts(%I64d)!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    DMX_PTS_LOG_MS(u8SyncCurSTC),
						    DMX_PTS_LOG_MS(prPsrCC->u8BaseSTC),
						    DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS),
						    DMX_PTS_LOG_MS(prPsrFtr->u81stPTS));
#endif				/* #ifdef __linux__ */


#ifdef __linux__
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT("[PSR] %s Line %d ++++++++++ ")
						 TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(SP), VPts: ")
						 TEXT("%lld ms(%lld), SPPts: %lld ms(%lld)!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prVPsrFtr->u8PrevPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prVPsrFtr->u8PrevPTS) :
						 INVALID_TIMESTAMP), prVPsrFtr->u8PrevPTS,
						((prPsrFtr->u8PrevPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS) :
						 INVALID_TIMESTAMP), prPsrFtr->u8PrevPTS);
#else
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT("[PSR] %s Line %d ++++++++++ ")
						 TEXT
						 ("TXS_WAIT_VFIFO_PTS_THRESHOLD(SP), VPts: %I64d")
						 TEXT
						 (" ms(%I64d), SPPts: %I64d ms(%I64d) ++++++++++!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prVPsrFtr->u8PrevPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prVPsrFtr->u8PrevPTS) :
						 INVALID_TIMESTAMP), prVPsrFtr->u8PrevPTS,
						((prPsrFtr->u8PrevPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prPsrFtr->u8PrevPTS) :
						 INVALID_TIMESTAMP), prPsrFtr->u8PrevPTS);
#endif				/* #ifdef __linux__ */

					PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
					g_rPsrPfm.rSP.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */
					MM_RETURN(RET_DMX_FIFO_FULL);
				}
			}
		}

		if ((NULL != prVPsrFtr) &&
		    (NULL != prVPsrFtr->pvPsrCC) &&
		    (prVPsrFtr->pvPsrCC != prPsrFtr->pvPsrCC) &&
		    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS)) {
			prVPsrCC = (PSR_CC *) (prVPsrFtr->pvPsrCC);
			if (INVALID_TIMESTAMP == prVPsrFtr->u8LastPTS)
				prVPsrFtr->u8LastPTS = prPsrFtr->u8LastPTS + 1;

			if ((!prVPsrCC->fgCfaPrsEnd) &&
			    (INVALID_TIMESTAMP != prVPsrFtr->u8LastPTS) &&
			    (prVPsrFtr->u8LastPTS >
			     prPsrFtr->u8LastPTS * ((u32) DMX_ABS_VAL(i4Rate)) +
			     DMX_SPSYNC_PTS_DELTA)) {
				PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
				prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
				prPsrCC->pvNormalWaitFtr = prPsrFtr;
				prPsrCC->pvNormalWaitOthFtr = prVPsrFtr;

#ifdef __linux__
				DMXLOG_DEBUG(TEXT("[PSR] %s Line %d --- (SP) Wait, ")
							  TEXT
							  ("SP'sLastPts(%lld ms), Video's u8LastPTS(%lld ms)!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, prSPPsrFtr->u8LastPTS,
					    prPsrFtr->u8LastPTS);
#else
				DMXLOG_DEBUG(TEXT("[PSR] %s Line %d --- (SP) Wait, ")
							  TEXT
							  ("SP'sLastPts(%I64d ms), Video's u8LastPTS(%I64d)!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, prSPPsrFtr->u8LastPTS,
					    prPsrFtr->u8LastPTS);
#endif				/* #ifdef __linux__ */

#ifdef __linux__
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
					TEXT("[PSR] %s Line %d ++++++++++ ")
					 TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(SP), VPts: ")
					 TEXT("%lld ms(%lld), SPPts: %lld ms(%lld)!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					((prVPsrFtr->u8LastPTS !=
					  INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prVPsrFtr->u8LastPTS)
					 : INVALID_TIMESTAMP), prVPsrFtr->u8LastPTS,
					((prPsrFtr->u8LastPTS !=
					  INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS) :
					 INVALID_TIMESTAMP), prPsrFtr->u8LastPTS);
#else
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
					TEXT("[PSR] %s Line %d ++++++++++ ")
					 TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(SP), VPts: ")
					 TEXT
					 ("%I64d ms(%I64d), SPPts: %I64d ms(%I64d) ++++++++++!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					((prVPsrFtr->u8LastPTS !=
					  INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prVPsrFtr->u8LastPTS)
					 : INVALID_TIMESTAMP), prVPsrFtr->u8LastPTS,
					((prPsrFtr->u8LastPTS !=
					  INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS) :
					 INVALID_TIMESTAMP), prPsrFtr->u8LastPTS);
#endif				/* #ifdef __linux__ */

				PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
				g_rPsrPfm.rSP.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */
				MM_RETURN(RET_DMX_FIFO_FULL);
			}
		}
	}

	if ((!MM_IS_NORMAL_PLAY(i4Rate)) &&
	    (1 == prPsrCC->u4PsrFtrCnt) &&
	    (u4AUCount * DMX_PTS_1S >=
	     DMX_SPSYNC_PTS_DELTA * ((u32) DMX_ABS_VAL(i4Rate)))) {
		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
		prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
		prPsrCC->pvNormalWaitFtr = prPsrFtr;
		prPsrCC->pvNormalWaitOthFtr = NULL;

		/* Add for Check not sent EOS while playing video in SD Card(mtk40144) */
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
			TEXT("[PSR] %s Line %d ++++++++++ ")
			 TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(V), u4AUCount: %d!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4AUCount);

		PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
		g_rPsrPfm.rSP.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */
		MM_RETURN(RET_DMX_FIFO_FULL);
	}

	prPsrCC->u8NormalWaitPts = INVALID_TIMESTAMP;
	prPsrCC->pvNormalWaitFtr = NULL;
	prPsrCC->pvNormalWaitOthFtr = NULL;

	MM_RETURN(RET_DMX_OK);
}

#else				/* MM_SUPROT_DIVXHT31 */

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_TX2Mem */
/* 1) If current Parser CC is FIFO Hold, check whether the remained AU count is <= 1 , */
/* if is, clear FIFOHOLD flag. */
/* otherwise, if Fifo Usage <= Min V FIFO Usage, clear FIFOHOLD Flag */
/* otherwise, Set FIFOHOLD Flag, set Parser CC state to be TXS_WAIT_FIFO */
/* 2) If current Parser CC is not FIFO Hold, check whether Fifo Usage >= Max V FIFO Usage, */
/* if is , Set FIFOHOLD Flag, and set Parser CC state to be TXS_WAIT_FIFO */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_TxDecideV(PSR_FILTER *prPsrFtr, PSR_CC *prPsrCC)
{
	u32 u4AUCount = 0;
	u32 u4FifoFree = 0;
	u32 u4FifoSize = 0;
	u32 u4FifoUsage = 100;
	s32 i4Rate = 1;
	DMX_INST_T *prDmxInst = NULL;
	MRESULT mrRet = RET_DMX_OK;

	prDmxInst = (DMX_INST_T *)prPsrFtr->pvDmxInst;

	if (NULL == prDmxInst) {
		DMX_ASSERT(0);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (!((NULL != prPsrFtr) && (NULL != prPsrCC))) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mrRet = ESM_AUTableGetAvailCount(prPsrFtr->u4ESIH, &u4AUCount);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
					  TEXT
					  ("ESM_AUTableGetAvailCount(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	u4FifoSize = prPsrFtr->u4ESFifoSize;

	mrRet = ESM_FifoGetAvailDataSize(prPsrFtr->u4ESIH, &u4FifoFree);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d (ESIH: 0x%x) fail in ")
					  TEXT("ESM_FifoGetAvailDataSize, mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		MM_RETURN(mrRet);
	}

	if (0 < u4FifoSize) {
		u4FifoUsage = u4FifoFree * 100 / u4FifoSize;
	} else {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s line %d - u4FifoSize: %d, u4FifoFree: %d\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u4FifoSize, u4FifoFree);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_OPERATE_FORBID);
	}

	/* DMXLOG_ERROR(TEXT("[PSR] [vdieo]:  Fifo usage: %d\r\n"), u4FifoUsage); */
	/* 1) If current Parser CC is FIFO Hold, check whether the remained AU count is <= 1 , */
	/* if is, clear FIFOHOLD flag. */
	/* otherwise, if Fifo Usage <= Min V FIFO Usage, clear FIFOHOLD Flag */
	/* otherwise, Set FIFOHOLD Flag, set Parser CC state to be TXS_WAIT_FIFO */
	/* 2) If current Parser CC is not FIFO Hold, check whether Fifo Usage >= Max V FIFO Usage, */
	/* if is , Set FIFOHOLD Flag, and set Parser CC state to be TXS_WAIT_FIFO */
	if (prPsrCC->u4Flag & CCF_FIFOHOLD) {
		if (0 == u4AUCount) {
			if (u4FifoUsage >= V_FIFO_USAGE_MAX) {
				prPsrCC->u4Flag |= CCF_FIFOHOLD;
				PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);

				if (prDmxInst->u4SptCnt > 1)
					PSR_HWRes_Release(prPsrFtr);

				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
					TEXT("[PSR] %s line %d -- V Fifo Full(Usage: %d),")
					 TEXT
					 (" but has no Video AU, we should sent EOS ++++++++++!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4FifoUsage);

				MM_RETURN(RET_DMX_ERR_DATA);
			} else {
				prPsrCC->u4Flag &= (~CCF_FIFOHOLD);
			}
		} else if (u4AUCount <= 1) {
			prPsrCC->u4Flag &= (~CCF_FIFOHOLD);
		} else {
			if ((SplitterRspIsEnabled(prPsrCC->pvSptHdl) &&
			     SplitterIsRspOffStart(prPsrCC->pvSptHdl))) {
				DMXLOG_DEBUG(
					    TEXT
					    ("[PSR] %s line %d -- RspOffStart, Clear CCF_FIFOHOLD\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO);
				prPsrCC->u4Flag &= (~CCF_FIFOHOLD);
			} else if (u4FifoUsage <= V_FIFO_USAGE_MIN) {
				prPsrCC->u4Flag &= (~CCF_FIFOHOLD);
			} else {
				prPsrCC->u4Flag |= CCF_FIFOHOLD;
				PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);

				if (prDmxInst->u4SptCnt > 1)
					PSR_HWRes_Release(prPsrFtr);

				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
					TEXT("[PSR] %s Line: %d -- Wait V fifo++++++++++!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);

#if DMX_PFM_TEST
				g_rPsrPfm.rVideo.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */
				MM_RETURN(RET_DMX_FIFO_FULL);
			}
		}
	} else {
		/* else of if(prPsrCC->u4Flag & CCF_FIFOHOLD) */
		if (0 == u4AUCount) {
			if (u4FifoUsage >= V_FIFO_USAGE_MAX) {
				prPsrCC->u4Flag |= CCF_FIFOHOLD;
				PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);

				if (prDmxInst->u4SptCnt > 1)
					PSR_HWRes_Release(prPsrFtr);

				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
					TEXT("[PSR] %s line %d -- V Fifo Full(Usage: %d),")
					 TEXT
					 (" but has no Video AU, we should sent EOS ++++++++++!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4FifoUsage);

				MM_RETURN(RET_DMX_ERR_DATA);
			} else {
				prPsrCC->u4Flag &= (~CCF_FIFOHOLD);
			}
		}

		if (u4FifoUsage >= V_FIFO_USAGE_MAX) {
			prPsrCC->u4Flag |= CCF_FIFOHOLD;
			PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);

			if (prDmxInst->u4SptCnt > 1)
				PSR_HWRes_Release(prPsrFtr);

			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
				TEXT("[PSR] %s Line: %d -- Wait V fifo++++++++++!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);

#if DMX_PFM_TEST
			g_rPsrPfm.rVideo.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */
			MM_RETURN(RET_DMX_FIFO_FULL);
		}
	}

	if (prDmxInst->u4SptCnt <= 1)
		MM_RETURN(RET_DMX_OK);

	if (u4AUCount <= DMX_STM_AU_CNT_THRESHOLD)
		MM_RETURN(RET_DMX_OK);

	if ((SplitterRspIsEnabled(prPsrCC->pvSptHdl) && SplitterIsRspOffStart(prPsrCC->pvSptHdl)))
		MM_RETURN(RET_DMX_OK);

	i4Rate = SplitterGetPlayRate(prPsrCC->pvSptHdl);

	if (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS) {
		PSR_FILTER *prSPPsrFtr =
		    (PSR_FILTER *) GetStreamPsrFtrByTypes(prPsrCC->pvDmxInst, SPT_DATA_SP);
		PSR_FILTER *prAPsrFtr =
		    (PSR_FILTER *) GetStreamPsrFtrByTypes(prPsrCC->pvDmxInst, SPT_DATA_A);

		if (MM_IS_NORMAL_PLAY(i4Rate)) {
			u64 u8SyncCurSTC = DMX_INVALID_UINT64;

			STC_HalGetTime(0, &u8SyncCurSTC);

			if (u8SyncCurSTC + DMX_VSYNC_PTS_DELTA <= prPsrFtr->u8LastPTS) {
				PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
				prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
				prPsrCC->pvNormalWaitFtr = prPsrFtr;
				prPsrCC->pvNormalWaitOthFtr = NULL;

				/* Add for Check not sent EOS while playing video in SD Card(mtk40144) */
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
					TEXT("[PSR] %s Line %d ++++++++++ ")
					 TEXT("PTS_THRESHOLD(V), VPts: %lldms(%lld)")
					 TEXT("u8SyncCurSTC: %lldms(%lld)++++++++++!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					((prPsrFtr->u8LastPTS != INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS) :
					 INVALID_TIMESTAMP),
				 	((prPsrFtr->u8LastPTS != INVALID_TIMESTAMP) ? DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS) :
					 INVALID_TIMESTAMP),
			   	((u8SyncCurSTC != INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(u8SyncCurSTC) :
					 INVALID_TIMESTAMP),
				 	((u8SyncCurSTC != INVALID_TIMESTAMP) ? DMX_PTS_LOG_PTS(u8SyncCurSTC) :
					 INVALID_TIMESTAMP));

				PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
				g_rPsrPfm.rVideo.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */

				MM_RETURN(RET_DMX_FIFO_FULL);
			}

			if ((NULL != prAPsrFtr) &&
			    (NULL != prAPsrFtr->pvPsrCC) &&
			    (prAPsrFtr->pvPsrCC != prPsrFtr->pvPsrCC)) {
				PSR_CC *prAPsrCC = (PSR_CC *) (prAPsrFtr->pvPsrCC);

				/* Video and Audio are transferred by two different spt instances */
				if ((INVALID_TIMESTAMP == prAPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS)) {
					prAPsrFtr->u8LastPTS = prPsrFtr->u8LastPTS - 1;
				} else if ((!prAPsrCC->fgCfaPrsEnd) &&
					   (INVALID_TIMESTAMP != prAPsrFtr->u8LastPTS) &&
					   (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS) &&
					   (prAPsrFtr->u8LastPTS +
					    DMX_ASYNC_PTS_DELTA <
					    prPsrFtr->u8LastPTS)) {
					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
					prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
					prPsrCC->pvNormalWaitFtr = prPsrFtr;
					prPsrCC->pvNormalWaitOthFtr = prAPsrFtr;

					/* Add for Check not sent EOS while playing video in SD Card(mtk40144) */
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT("[PSR] %s Line %d ++++++++++ ")
						 TEXT("PTS_THRESHOLD(V), APts: %lldms(%lld)")
						 TEXT("VPts: %lldms(%lld) ++++++++++!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prAPsrFtr->u8LastPTS !=
					  INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prAPsrFtr->u8LastPTS) :
					 INVALID_TIMESTAMP),
				 	((prAPsrFtr->u8LastPTS !=
					  INVALID_TIMESTAMP) ? DMX_PTS_LOG_PTS(prAPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP),
						((prPsrFtr->u8LastPTS !=
							 INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP),
						((prPsrFtr->u8LastPTS !=
							 INVALID_TIMESTAMP) ? DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP));

					PSR_HWRes_Release(prPsrFtr);

#if DMX_PFM_TEST
					g_rPsrPfm.rVideo.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */

					MM_RETURN(RET_DMX_FIFO_FULL);
				}
			}
		} else if (MM_IS_FF_PLAY(i4Rate)) {
			if ((NULL != prSPPsrFtr) &&
			    (NULL != prSPPsrFtr->pvPsrCC) &&
			    (prSPPsrFtr->pvPsrCC != prPsrFtr->pvPsrCC)) {
				PSR_CC *prSPPsrCC = (PSR_CC *) (prSPPsrFtr->pvPsrCC);
				/* Video and SP are transferred by two different spt instances */
				if ((INVALID_TIMESTAMP == prSPPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS)) {
					prSPPsrFtr->u8LastPTS = prPsrFtr->u8LastPTS - 1;
				}
				if ((!prSPPsrCC->fgCfaPrsEnd) &&
				    (INVALID_TIMESTAMP != prSPPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS) &&
				    (prSPPsrFtr->u8LastPTS +
				     DMX_SPSYNC_PTS_DELTA * ((u32) i4Rate) <
				     prPsrFtr->u8LastPTS)) {
					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
					prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
					prPsrCC->pvNormalWaitFtr = prPsrFtr;
					prPsrCC->pvNormalWaitOthFtr = prSPPsrCC;

					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT("[PSR] %s Line %d ++++++++++ ")
						 TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(V), SPPts: "DMX_PTS_LOGSTR)
						 TEXT("VPts: "DMX_PTS_LOGSTR " ++++++++++!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prSPPsrFtr->u8LastPTS !=
							INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prSPPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP),
						((prSPPsrFtr->u8LastPTS !=
							INVALID_TIMESTAMP) ? DMX_PTS_LOG_PTS(prSPPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP),
						((prPsrFtr->u8LastPTS !=
						 INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP),
						((prPsrFtr->u8LastPTS !=
						 INVALID_TIMESTAMP) ? DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP));

					PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
					g_rPsrPfm.rVideo.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */

					MM_RETURN(RET_DMX_FIFO_FULL);
				}
			}
		} else if (MM_IS_RW_PLAY(i4Rate)) {
			if ((NULL != prSPPsrFtr) &&
			    (NULL != prSPPsrFtr->pvPsrCC) &&
			    (prSPPsrFtr->pvPsrCC != prPsrFtr->pvPsrCC)) {
				PSR_CC *prSPPsrCC = (PSR_CC *) (prSPPsrFtr->pvPsrCC);
				/* Video and SP are transferred by two different spt instances */
				if ((INVALID_TIMESTAMP == prSPPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS)) {
					prSPPsrFtr->u8LastPTS = prPsrFtr->u8LastPTS + 1;
				}
#ifdef MM_SUPROT_DIVXHT31
				if (CFA_TYPE_AVI == SplitterGetCfaType(prPsrCC->pvSptHdl))
					) {
					s32 i4Rate =
					    DMX_ABS_VAL(SplitterGetPlayRate(prPsrCC->pvSptHdl));
					mrRet =
					    ESM_AUTableGetAvailCount(prPsrFtr->u4ESIH, &u4AUCount);
					if (DMX_FAILED(mrRet)) {
						DMXLOG_ERROR(
							    TEXT("[PSR] %s line %d fail in ")
							     TEXT
							     ("ESM_AUTableGetAvailCount(u4Handle: 0x%x),")
							     TEXT(" mrRet: 0x%x\r\n"),
							    DMX_FUNC_NAME, DMX_LINE_NO,
							    prPsrFtr->u4ESIH, mrRet);
						DMX_ASSERT(FALSE);
						MM_RETURN(mrRet);
					}

					if (i4Rate < 1)
						i4Rate = 1;

					if (u4AUCount > 10) {
						PSR_CC_SetTxSt(prPsrCC,
							       TXS_WAIT_VFIFO_PTS_THRESHOLD);
						prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
						prPsrCC->pvNormalWaitFtr = prPsrFtr;
						prPsrCC->pvNormalWaitOthFtr = prSPPsrCC;

						DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
							TEXT("[PSR] %s Line %d ++++++++++ ")
							 TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(V), SPPts: "DMX_PTS_LOGSTR)
							 TEXT("VPts: "DMX_PTS_LOGSTR " ++++++++++!\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							((prSPPsrFtr->u8LastPTS !=
								INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prSPPsrFtr->u8LastPTS) :
							 INVALID_TIMESTAMP),
							((prSPPsrFtr->u8LastPTS !=
								INVALID_TIMESTAMP) ? DMX_PTS_LOG_PTS(prSPPsrFtr->u8LastPTS) :
							 INVALID_TIMESTAMP),
							((prPsrFtr->u8LastPTS !=
							 INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS) :
							 INVALID_TIMESTAMP),
							((prPsrFtr->u8LastPTS !=
							 INVALID_TIMESTAMP) ? DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS) :
							INVALID_TIMESTAMP));								

						PSR_HWRes_Release(prPsrFtr);
						MM_RETURN(RET_DMX_FIFO_FULL);
					}
					}
#endif				/* MM_SUPROT_DIVXHT31 */

				if ((!prSPPsrCC->fgCfaPrsEnd) &&
				    (INVALID_TIMESTAMP != prSPPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS) &&
				    (prSPPsrFtr->u8LastPTS >
				     prPsrFtr->u8LastPTS +
				     DMX_SPSYNC_PTS_DELTA * ((u32) i4Rate))) {
					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
					prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
					prPsrCC->pvNormalWaitFtr = prPsrFtr;
					prPsrCC->pvNormalWaitOthFtr = prSPPsrCC;

					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT("[PSR] %s Line %d ++++++++++ ")
						 TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(V), SPPts: "DMX_PTS_LOGSTR)
						 TEXT("VPts: "DMX_PTS_LOGSTR " ++++++++++!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prSPPsrFtr->u8LastPTS !=
							INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prSPPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP),
						((prSPPsrFtr->u8LastPTS !=
							INVALID_TIMESTAMP) ? DMX_PTS_LOG_PTS(prSPPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP),
						((prPsrFtr->u8LastPTS !=
						 INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP),
						((prPsrFtr->u8LastPTS !=
						 INVALID_TIMESTAMP) ? DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP));

					PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
					g_rPsrPfm.rVideo.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */

					MM_RETURN(RET_DMX_FIFO_FULL);
				}
			}
		}
	}

	if ((!MM_IS_NORMAL_PLAY(i4Rate)) &&
	    (1 == prPsrCC->u4PsrFtrCnt) &&
	    (u4AUCount * DMX_PTS_1S / 60 >=
	     DMX_VSYNC_PTS_DELTA * ((u32) DMX_ABS_VAL(i4Rate)))) {
		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
		prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
		prPsrCC->pvNormalWaitFtr = prPsrFtr;
		prPsrCC->pvNormalWaitOthFtr = NULL;
		/* Add for Check not sent EOS while playing video in SD Card(mtk40144) */
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
			TEXT("[PSR] %s Line %d ++++++++++ ")
			 TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(V), u4AUCount: %d!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4AUCount);
		PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
		g_rPsrPfm.rVideo.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */

		MM_RETURN(RET_DMX_FIFO_FULL);
	}

	prPsrCC->u8NormalWaitPts = INVALID_TIMESTAMP;
	prPsrCC->pvNormalWaitFtr = NULL;
	prPsrCC->pvNormalWaitOthFtr = NULL;

	MM_RETURN(RET_DMX_OK);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_TxDecideA */
/* 1) If current Parser CC is FIFO Hold, check whether the remained AU count is <= 1 , */
/* if is, clear FIFOHOLD flag. */
/* otherwise, if Fifo Usage <= Min V FIFO Usage, clear FIFOHOLD Flag */
/* otherwise, Set FIFOHOLD Flag, set Parser CC state to be TXS_WAIT_FIFO */
/* 2) If current Parser CC is not FIFO Hold, check whether Fifo Usage >= Max V FIFO Usage, */
/* if is , Set FIFOHOLD Flag, and set Parser CC state to be TXS_WAIT_FIFO */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_TxDecideA(PSR_FILTER *prPsrFtr, PSR_CC *prPsrCC)
{
	u32 u4AUCount = 0;
	u32 u4FifoData = 0;
	u32 u4FifoSize = 0;
	u32 u4FifoUsage = 100;
	u32 u4FifoAvailData = 0;
	s32 i4Rate = 0;
	DMX_INST_T *prDmxInst = NULL;
	MRESULT mrRet = RET_DMX_OK;

	prDmxInst = (DMX_INST_T *)prPsrFtr->pvDmxInst;

	if (NULL == prDmxInst) {
		DMX_ASSERT(0);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	
	if (!((NULL != prPsrFtr) && (NULL != prPsrCC))) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	/* Add for sony http stream, no hold tx, @2009/08/24 */
	if (SPT_DATA_A != prPsrFtr->eType) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d (ESIH: 0x%x) ")
					  TEXT("fail in PsrFtr's eType(%d) != Audio\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, prPsrFtr->eType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	u4FifoSize = prPsrFtr->u4ESFifoSize;

	mrRet = ESM_FifoGetAvailDataSize(prPsrFtr->u4ESIH, &u4FifoAvailData);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d (ESIH: 0x%x) fail in ")
					  TEXT("ESM_FifoGetAvailDataSize, mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		MM_RETURN(mrRet);
	}

	if (0 < u4FifoSize) {
		u4FifoUsage = u4FifoAvailData * 100 / u4FifoSize;
	} else {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s line %d - u4FifoSize: %d, u4FifoAvailData: %d\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u4FifoSize, u4FifoAvailData);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_OPERATE_FORBID);
	}
	/* Pure Audio */
	if (1 == DmxInstGetStreamCnt(prPsrFtr->pvDmxInst)) {

		/* 1) If current Parser CC is FIFO Hold, check whether the remained AU count is <= 1 , */
		/* if is, clear FIFOHOLD flag. */
		/* otherwise, if Fifo Usage <= Min V FIFO Usage, clear FIFOHOLD Flag */
		/* otherwise, Set FIFOHOLD Flag, set Parser CC state to be TXS_WAIT_FIFO */
		/* 2) If current Parser CC is not FIFO Hold, check whether Fifo Usage >= Max V FIFO Usage, */
		/* if is , Set FIFOHOLD Flag, and set Parser CC state to be TXS_WAIT_FIFO */
		if (prPsrCC->u4Flag & CCF_FIFOHOLD) {
			if ((SplitterRspIsEnabled(prPsrCC->pvSptHdl) &&
			     SplitterIsRspOffStart(prPsrCC->pvSptHdl))) {
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
					TEXT
					("[PSR] %s line %d -- RspOffStart, Clear CCF_FIFOHOLD\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				prPsrCC->u4Flag &= (~CCF_FIFOHOLD);
			} else if (DMX_IS_NORMAL_PLAY(prPsrCC->pvSptHdl)) {
				if (u4FifoUsage <= PURE_AUDIO_A_FIFO_USAGE_MIN) {
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT("[PSR] %s line %d -- u4FifoUsage(%d) < ")
						 TEXT
						 ("PURE_AUDIO_A_FIFO_USAGE_MIN(%d), Clear CCF_FIFOHOLD\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, u4FifoUsage,
						PURE_AUDIO_A_FIFO_USAGE_MIN);
					prPsrCC->u4Flag &= (~CCF_FIFOHOLD);
				} else {
					prPsrCC->u4Flag |= CCF_FIFOHOLD;
					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);

					if (prDmxInst->u4SptCnt > 1)
						PSR_HWRes_Release(prPsrFtr);

					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
						TEXT("[PSR] %s Line: %d -- u4FifoUsage(%d),")
						 TEXT(" Wait A fifo++++++++++!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, u4FifoUsage);
#if DMX_PFM_TEST
					g_rPsrPfm.rVideo.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */
					MM_RETURN(RET_DMX_FIFO_FULL);
				}
			} else {
				prPsrCC->u4Flag &= (~CCF_FIFOHOLD);
			}
		} else {
			/* else of if(prPsrCC->u4Flag & CCF_FIFOHOLD) */
			if (DMX_IS_NORMAL_PLAY(prPsrCC->pvSptHdl) &&
			    (u4FifoUsage >= PURE_AUDIO_A_FIFO_USAGE_MAX)) {
				prPsrCC->u4Flag |= CCF_FIFOHOLD;
				PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);

				if (prDmxInst->u4SptCnt > 1)
					PSR_HWRes_Release(prPsrFtr);

				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
					TEXT("[PSR] %s Line: %d -- u4FifoUsage(%d)")
					 TEXT(" Wait A fifo++++++++++!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4FifoUsage);
#if DMX_PFM_TEST
				g_rPsrPfm.rVideo.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */
				MM_RETURN(RET_DMX_FIFO_FULL);
			}
		}
	}
	/* ////////////////////////////////////////////////////////////////////////// */
	/* 1. Video & Audio & SP are transferred by the same spt inst */
	/* 2. Pure Audio */
	/* ////////////////////////////////////////////////////////////////////////// */

	if (prDmxInst->u4SptCnt <= 1)
		MM_RETURN(RET_DMX_OK);
	/* ////////////////////////////////////////////////////////////////////////// */
	/* Two or three Spt Instance */
	/* ////////////////////////////////////////////////////////////////////////// */
	u4FifoSize = prPsrFtr->u4ESFifoSize;

	mrRet = ESM_FifoGetAvailDataSize(prPsrFtr->u4ESIH, &u4FifoData);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d (ESIH: 0x%x) fail in ")
					  TEXT("ESM_FifoGetAvailDataSize, mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (0 < u4FifoSize) {
		u4FifoUsage = u4FifoData * 100 / u4FifoSize;
	} else {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s line %d - u4FifoSize: %d, u4FifoData: %d\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u4FifoSize, u4FifoData);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_OPERATE_FORBID);
	}

	mrRet = ESM_AUTableGetAvailCount(prPsrFtr->u4ESIH, &u4AUCount);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ESM_AUTableGetAvailCount")
					  TEXT("(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if ((SplitterRspIsEnabled(prPsrCC->pvSptHdl) && SplitterIsRspOffStart(prPsrCC->pvSptHdl)))
		MM_RETURN(RET_DMX_OK);

	i4Rate = SplitterGetPlayRate(prPsrCC->pvSptHdl);

	if (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS) {
		PSR_FILTER *prVPsrFtr =
		    (PSR_FILTER *) GetStreamPsrFtrByTypes(prPsrCC->pvDmxInst, SPT_DATA_V);

		if (MM_IS_NORMAL_PLAY(i4Rate)) {
			PSR_CC *prVPsrCC =
			    (PSR_CC *) ((NULL != prVPsrFtr) ? (prVPsrFtr->pvPsrCC) : NULL);
			if ((NULL != prVPsrCC) && (!(prVPsrCC->fgCfaPrsEnd))
			    && (prVPsrCC != prPsrFtr->pvPsrCC)) {
				u64 u8SyncCurSTC = DMX_INVALID_UINT64;

				STC_HalGetTime(0, &u8SyncCurSTC);



				if ((INVALID_TIMESTAMP == prVPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS)) {
					prVPsrFtr->u8LastPTS = prPsrFtr->u8LastPTS - 1;
				}

				if ((INVALID_TIMESTAMP != prVPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS) &&
				    ((prVPsrFtr->u8LastPTS + DMX_VSYNC_PTS_DELTA < prPsrFtr->u8LastPTS)
				     || (u8SyncCurSTC + DMX_ASYNC_PTS_DELTA <= prPsrFtr->u8LastPTS))) {
					if ((u4AUCount < AUD_AU_CNT_THRESHOLD) &&
							 (u4FifoUsage < AUD_FIFO_USAGE_RATE_THRESHOLD)) {
					} else {
						DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
							TEXT("[PSR] %s Line %d ++++++++++ ")
							 TEXT("PTS_THRESHOLD(A), AuCnt: %d, USage: %d, VPts: %lldms(%lld)")
							 TEXT("APts: %lldms(%lld)")
							 TEXT("u8SyncCurSTC: %lldms(%lld) ++++++++++!\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							 u4AUCount, u4FifoUsage,
							((prVPsrFtr->u8LastPTS !=
								INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prVPsrFtr->u8LastPTS) :
							 INVALID_TIMESTAMP),
							((prVPsrFtr->u8LastPTS !=
								INVALID_TIMESTAMP) ? DMX_PTS_LOG_PTS(prVPsrFtr->u8LastPTS) :
							 INVALID_TIMESTAMP),
							((prPsrFtr->u8LastPTS !=
							 INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS) :
							INVALID_TIMESTAMP),
							((prPsrFtr->u8LastPTS !=
							 INVALID_TIMESTAMP) ? DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS) :
							INVALID_TIMESTAMP),
						 ((u8SyncCurSTC !=
							 INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(u8SyncCurSTC) :
							INVALID_TIMESTAMP),
						 ((u8SyncCurSTC !=
							 INVALID_TIMESTAMP) ? DMX_PTS_LOG_PTS(u8SyncCurSTC) :
							INVALID_TIMESTAMP));					
						PSR_CC_SetTxSt(prPsrCC,
										 TXS_WAIT_VFIFO_PTS_THRESHOLD);
						prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
						prPsrCC->pvNormalWaitFtr = prPsrFtr;
						prPsrCC->pvNormalWaitOthFtr = prVPsrFtr;

						PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
						g_rPsrPfm.rAudio.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */

						MM_RETURN(RET_DMX_FIFO_FULL);
					}
				}
			}
		}
	}

	prPsrCC->u8NormalWaitPts = INVALID_TIMESTAMP;
	prPsrCC->pvNormalWaitFtr = NULL;
	prPsrCC->pvNormalWaitOthFtr = NULL;

	MM_RETURN(RET_DMX_OK);
}

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_TxDecideSP */
/* 1) If current Parser CC is FIFO Hold, check whether the remained AU count is <= 1 , */
/* if is, clear FIFOHOLD flag. */
/* otherwise, if Fifo Usage <= Min V FIFO Usage, clear FIFOHOLD Flag */
/* otherwise, Set FIFOHOLD Flag, set Parser CC state to be TXS_WAIT_FIFO */
/* 2) If current Parser CC is not FIFO Hold, check whether Fifo Usage >= Max V FIFO Usage, */
/* if is , Set FIFOHOLD Flag, and set Parser CC state to be TXS_WAIT_FIFO */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_TxDecideSP(PSR_FILTER *prPsrFtr, PSR_CC *prPsrCC)
{
	u32 u4AUCount = 0;
	u32 u4FifoData = 0;
	u32 u4FifoSize = 0;
	u32 u4FifoUsage = 100;
	s32 i4Rate = 1;
	DMX_INST_T *prDmxInst = NULL;
	MRESULT mrRet = RET_DMX_OK;

	prDmxInst = (DMX_INST_T *)prPsrFtr->pvDmxInst;

	if (NULL == prDmxInst) {
		DMX_ASSERT(0);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	
	if (!((NULL != prPsrFtr) && (NULL != prPsrCC))) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	/* Add for sony http stream, no hold tx, @2009/08/24 */
	if (SPT_DATA_SP != prPsrFtr->eType) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d (ESIH: 0x%x) ")
					  TEXT("fail in PsrFtr's eType(%d) != Audio\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, prPsrFtr->eType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	/* ////////////////////////////////////////////////////////////////////////// */
	/* Video & SP( & Audio) are transferred by the same spt instance */
	/* ////////////////////////////////////////////////////////////////////////// */
	if (prDmxInst->u4SptCnt <= 1)
		MM_RETURN(RET_DMX_OK);

	u4FifoSize = prPsrFtr->u4ESFifoSize;

	mrRet = ESM_FifoGetAvailDataSize(prPsrFtr->u4ESIH, &u4FifoData);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d (ESIH: 0x%x) fail in ")
					  TEXT("ESM_FifoGetAvailDataSize, mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (0 < u4FifoSize) {
		u4FifoUsage = u4FifoData * 100 / u4FifoSize;
	} else {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s line %d - u4FifoSize: %d, u4FifoData: %d\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u4FifoSize, u4FifoData);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_OPERATE_FORBID);
	}

	mrRet = ESM_AUTableGetAvailCount(prPsrFtr->u4ESIH, &u4AUCount);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ESM_AUTableGetAvailCount")
					  TEXT("(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (u4AUCount <= DMX_STM_AU_CNT_THRESHOLD)
		MM_RETURN(RET_DMX_OK);

	if ((SplitterRspIsEnabled(prPsrCC->pvSptHdl) && SplitterIsRspOffStart(prPsrCC->pvSptHdl)))
		MM_RETURN(RET_DMX_OK);
	/* ////////////////////////////////////////////////////////////////////////// */
	/* Multi Spt Instances */
	/* ////////////////////////////////////////////////////////////////////////// */
	i4Rate = SplitterGetPlayRate(prPsrCC->pvSptHdl);

	if (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS) {
		PSR_FILTER *prVPsrFtr =
		    (PSR_FILTER *) GetStreamPsrFtrByTypes(prPsrCC->pvDmxInst, SPT_DATA_V);
		/* PSR_FILTER *prAPsrFtr = (PSR_FILTER *)GetStreamPsrFtrByTypes
		 * (prPsrCC->pvDmxInst, SPT_DATA_A); */

		if (MM_IS_NORMAL_PLAY(i4Rate)) {
			u64 u8SyncCurSTC = DMX_INVALID_UINT64;

			STC_HalGetTime(0, &u8SyncCurSTC);

			if ((NULL != prVPsrFtr) &&
			    (NULL != prVPsrFtr->pvPsrCC) &&
			    (prVPsrFtr->pvPsrCC != prPsrFtr->pvPsrCC)) {
				PSR_CC *prVPsrCC = (PSR_CC *) (prVPsrFtr->pvPsrCC);

				if ((INVALID_TIMESTAMP == prVPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS)) {
					prVPsrFtr->u8LastPTS = prPsrFtr->u8LastPTS - 1;
				}

				if ((u4AUCount < SP_AU_CNT_THRESHOLD) &&
				    (u4FifoUsage < SP_FIFO_USAGE_RATE_THRESHOLD)) {
					DMXLOG_DEBUG(
						    TEXT("[PSR] TxDecide, too few sp, ")
						     TEXT("au:0x%x, usg:0x%x,  no hold!\r\n"),
						    u4AUCount, u4FifoUsage);
				} else if ((!prVPsrCC->fgCfaPrsEnd)
					   && (INVALID_TIMESTAMP != prVPsrFtr->u8LastPTS)
					   && (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS)
					   &&
					   ((prVPsrFtr->u8LastPTS +
					     DMX_SPSYNC_PTS_DELTA <
					     prPsrFtr->u8LastPTS)
					    || (u8SyncCurSTC + DMX_SPSYNC_PTS_DELTA <=
						prPsrFtr->u8LastPTS))) {
					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
					prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
					prPsrCC->pvNormalWaitFtr = prPsrFtr;
					prPsrCC->pvNormalWaitOthFtr = prVPsrFtr;

					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT("[PSR] %s Line %d ++++++++++ ")
						 TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(SP), VPts: "DMX_PTS_LOGSTR)
						 TEXT("SPPts: "DMX_PTS_LOGSTR " ++++++++++!\r\n"),
						 DMX_FUNC_NAME, DMX_LINE_NO,
						((prVPsrFtr->u8LastPTS !=
							INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prVPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP),
						((prVPsrFtr->u8LastPTS !=
							INVALID_TIMESTAMP) ? DMX_PTS_LOG_PTS(prVPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP),
						((prPsrFtr->u8LastPTS !=
						 INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP),
						((prPsrFtr->u8LastPTS !=
						 INVALID_TIMESTAMP) ? DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP));

					PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
					g_rPsrPfm.rSP.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */
					MM_RETURN(RET_DMX_FIFO_FULL);
				}
			}
		} else if (MM_IS_FF_PLAY(i4Rate)) {
			if ((NULL != prVPsrFtr) &&
			    (NULL != prVPsrFtr->pvPsrCC) &&
			    (prVPsrFtr->pvPsrCC != prPsrFtr->pvPsrCC)) {
				PSR_CC *prVPsrCC = (PSR_CC *) (prVPsrFtr->pvPsrCC);

				if ((INVALID_TIMESTAMP == prVPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS)) {
					prVPsrFtr->u8LastPTS = prPsrFtr->u8LastPTS - 1;
				}

				if ((!prVPsrCC->fgCfaPrsEnd) &&
				    (INVALID_TIMESTAMP != prVPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS) &&
				    (prVPsrFtr->u8LastPTS +
				     DMX_VSYNC_PTS_DELTA * ((u32) i4Rate) <
				     prPsrFtr->u8LastPTS)) {
					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
					prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
					prPsrCC->pvNormalWaitFtr = prPsrFtr;
					prPsrCC->pvNormalWaitOthFtr = prVPsrFtr;

					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT("[PSR] %s Line %d ++++++++++ ")
						 TEXT("TXS_WAIT_VFIFO_PTS_THRESHOLD(SP), VPts: "DMX_PTS_LOGSTR)
						 TEXT("SPPts: "DMX_PTS_LOGSTR " ++++++++++!\r\n"),
						 DMX_FUNC_NAME, DMX_LINE_NO,
						((prVPsrFtr->u8LastPTS !=
							INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prVPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP),
						((prVPsrFtr->u8LastPTS !=
							INVALID_TIMESTAMP) ? DMX_PTS_LOG_PTS(prVPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP),
						((prPsrFtr->u8LastPTS !=
						 INVALID_TIMESTAMP) ? DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP),
						((prPsrFtr->u8LastPTS !=
						 INVALID_TIMESTAMP) ? DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP));

					PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
					g_rPsrPfm.rSP.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */
					MM_RETURN(RET_DMX_FIFO_FULL);
				}
			}
		} else if (MM_IS_RW_PLAY(i4Rate)) {
			if ((NULL != prVPsrFtr) &&
			    (NULL != prVPsrFtr->pvPsrCC) &&
			    (prVPsrFtr->pvPsrCC != prPsrFtr->pvPsrCC)) {
				PSR_CC *prVPsrCC = (PSR_CC *) (prVPsrFtr->pvPsrCC);

				if ((INVALID_TIMESTAMP == prVPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS)) {
					prVPsrFtr->u8LastPTS = prPsrFtr->u8LastPTS + 1;
				}

				if ((!prVPsrCC->fgCfaPrsEnd) &&
				    (INVALID_TIMESTAMP != prVPsrFtr->u8LastPTS) &&
				    (INVALID_TIMESTAMP != prPsrFtr->u8LastPTS) &&
				    (prVPsrFtr->u8LastPTS >
				     prPsrFtr->u8LastPTS * ((u32) DMX_ABS_VAL(i4Rate)) +
				     DMX_SPSYNC_PTS_DELTA)) {
					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
					prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
					prPsrCC->pvNormalWaitFtr = prPsrFtr;
					prPsrCC->pvNormalWaitOthFtr = prVPsrFtr;

					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT("[PSR] %s Line %d ++++++++++ TXS_WAIT_VFIFO")
						 TEXT("_PTS_THRESHOLD(SP), VPts: " DMX_PTS_LOGSTR)
						 TEXT("SPPts: " DMX_PTS_LOGSTR " ++++++++++!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prVPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prVPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP),
						((prVPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_PTS(prVPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP),
						((prPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP),
						((prPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP));


					PSR_HWRes_Release(prPsrFtr);

#if DMX_PFM_TEST
					g_rPsrPfm.rSP.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */
					MM_RETURN(RET_DMX_FIFO_FULL);
				}
			}
#ifdef MM_SUPROT_DIVXHT31
			if (CFA_TYPE_AVI == SplitterGetCfaType(prPsrCC->pvSptHdl))
				) {
				s32 i4Rate = DMX_ABS_VAL(SplitterGetPlayRate(prPsrCC->pvSptHdl));

				if (i4Rate < 1)
					i4Rate = 1;

				if (u4AUCount > 10) {
					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
					prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
					prPsrCC->pvNormalWaitFtr = prPsrFtr;
					prPsrCC->pvNormalWaitOthFtr = prVPsrFtr;

					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
						TEXT("[PSR] %s Line %d ++++++++++ TXS_WAIT_VFIFO")
						 TEXT("_PTS_THRESHOLD(SP), VPts: " DMX_PTS_LOGSTR)
						 TEXT("SPPts: " DMX_PTS_LOGSTR " ++++++++++!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((prVPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prVPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP),
						((prVPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_PTS(prVPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP),
						((prPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP),
						((prPsrFtr->u8LastPTS !=
						  INVALID_TIMESTAMP) ?
						 DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS) :
						 INVALID_TIMESTAMP));

					PSR_HWRes_Release(prPsrFtr);
					MM_RETURN(RET_DMX_FIFO_FULL);
				}
				}
#endif				/* MM_SUPROT_DIVXHT31 */

		}
	}

	if ((!MM_IS_NORMAL_PLAY(i4Rate)) &&
	    (1 == prPsrCC->u4PsrFtrCnt) &&
	    (u4AUCount * DMX_PTS_1S >=
	     DMX_SPSYNC_PTS_DELTA * ((u32) DMX_ABS_VAL(i4Rate)))) {
		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_VFIFO_PTS_THRESHOLD);
		prPsrCC->u8NormalWaitPts = prPsrFtr->u8LastPTS;
		prPsrCC->pvNormalWaitFtr = prPsrFtr;
		prPsrCC->pvNormalWaitOthFtr = NULL;
		/* Add for Check not sent EOS while playing video in SD Card(mtk40144) */
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
			TEXT("[PSR] %s Line %d ++++++++++ TXS_WAIT_VFIFO")
			 TEXT("_PTS_THRESHOLD(V), u4AUCount: %d!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4AUCount);
		PSR_HWRes_Release(prPsrFtr);
#if DMX_PFM_TEST
		g_rPsrPfm.rSP.u4FifoFullCnt++;
#endif				/* DMX_PFM_TEST */
		MM_RETURN(RET_DMX_FIFO_FULL);
	}

	prPsrCC->u8NormalWaitPts = INVALID_TIMESTAMP;
	prPsrCC->pvNormalWaitFtr = NULL;
	prPsrCC->pvNormalWaitOthFtr = NULL;

	MM_RETURN(RET_DMX_OK);
}

#endif				/* MM_SUPPORT_DIVXHT31 */

/* //////////////////////////////////////////////////////////////////////////////// */
/* PSR_Filter_TXDecide */
/* If this parser filter is not enable, call txtoground function */
/* otherwise, If we can't obtain the HW resource, Set Parser CC TxState to be TXS_WAIT_HW */
/* otherwise, copy the tx data from ptrTxCurrSa to tx mem, */
/* Set Parser CC state to be TXS_WAIT_IRQ_PROC, Wakeup Parser CC to tranfer mem data */
/* //////////////////////////////////////////////////////////////////////////////// */
MRESULT PSR_Filter_TXDecide(PSR_FILTER *prPsrFtr)
{
	PSR_CC *prPsrCC = NULL;
	DMX_INST_T *prDmxInst = NULL;
	MRESULT mrRet = RET_DMX_OK;
	u32 u4Size;

	if (NULL == prPsrFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrCC = (PSR_CC *) prPsrFtr->pvPsrCC;

	if (!(NULL != prPsrCC)) {
		DMXLOG_ERROR(TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
			      TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_CC);
	}

	prDmxInst = (DMX_INST_T *)prPsrFtr->pvDmxInst;
	
	/* check data in enable flag */
	if (!(prPsrCC->u4Flag & CCF_DATAIN_ENABLE))
		MM_RETURN(RET_DMX_UNEXPECT);
#if DMX_SUPPORT_DIVXDRM

	/* Is this has some problem? mtk40144 */
	/* Tx source is pbbbuf */
	if (prPsrFtr->u4Flag & FF_TX_PBBUF) {
		mrRet = PSR_CC_GetWaitTxBufInfo(prPsrCC, prPsrCC->u8TxCurrOffset,
						&(prPsrCC->ptrTxCurrSa), &(prPsrCC->u8TxCurrLen));
		if (DMX_FAILED(mrRet)) {
			if (!MM_IS_STATE_ERROR(mrRet)) {
				DMXLOG_ERROR(
					    TEXT("[PSR] %s line %d fail in PSR_CC_Get")
					     TEXT("WaitTxBufSa, mrRet: 0x%x, PsrCC: 0x%p!!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prPsrCC);
				DMX_ASSERT(FALSE);
			}
			MM_RETURN(mrRet);
		}
	} else {
		mrRet = PSR_CC_GetWaitTxBufSa(prPsrCC, &(prPsrCC->ptrTxCurrSa));
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in PSR_CC_Get")
						  TEXT
						  ("WaitTxBufSa, mrRet: 0x%x, PsrCC: 0x%p!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prPsrCC);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
		/* If the data cross two slot, u8TxCurrLen is the tx data end
		 * offset - slot's start offset, this means u8TxCurrLen < u8TxLen */
		mrRet = PSR_CC_GetWaitTxBufSize(prPsrCC, &prPsrCC->u8TxCurrLen);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in PSR_CC_Get")
						  TEXT
						  ("WaitTxBufSize, mrRet: 0x%x, PsrCC: 0x%p!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prPsrCC);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
	}

#else				/* DMX_SUPPORT_DIVXDRM */

	/* Is this has some problem? mtk40144 */
	mrRet = PSR_CC_GetWaitTxBufSa(prPsrCC, &(prPsrCC->ptrTxCurrSa));
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in PSR_CC_GetWait")
					  TEXT("TxBufSa, mrRet: 0x%x, PsrCC: 0x%p!!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prPsrCC);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}
	/* If the data cross two slot, u8TxCurrLen is the tx data end offset -
	 * slot's start offset, this means u8TxCurrLen < u8TxLen */
	mrRet = PSR_CC_GetWaitTxBufSize(prPsrCC, &prPsrCC->u8TxCurrLen);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in PSR_CC_GetWait")
					  TEXT("TxBufSize, mrRet: 0x%x, PsrCC: 0x%p!!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prPsrCC);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

#endif				/* DMX_SUPPORT_DIVXDRM */

	/* Check whether there are remain data to tx */

	if ((prPsrCC->u8TxCurrOffset + prPsrCC->u8TxCurrLen) ==
	    (prPsrCC->u8TxStartOffset + prPsrCC->u8TxLen)) {
		prPsrCC->fgHaveSubsequentData = FALSE;
	} else {
		prPsrCC->fgHaveSubsequentData = TRUE;
#if 0
		if ((SplitterRspIsEnabled(prPsrCC->pvSptHdl) &&
		     SplitterIsRspOffStart(prPsrCC->pvSptHdl) &&
		     (prPsrCC->u8TxCurrLen < prPsrCC->u8TxLen))) {
			DMXLOG_TRACE(TEXT("[PSR] %s line %d -- Split Packet\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
		}
#endif
	}

#if ENABLE_DMX_ADVANCED_VER
	if (prPsrCC->fgUseCmdQ) {
		if (prPsrCC->u8TxLen > DMX_CMDQ_TX_MAX_SIZE) {
			DMXLOG_WARN(TEXT("[PSR] %s line %d fail for CmdQ Tx Data")
						 TEXT
						 (" Size(%d) > DMX_CMDQ_TX_MAX_SIZE(%d)!!\r\n"),
				   DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u8TxLen,
				   DMX_CMDQ_TX_MAX_SIZE);
		}
	}
#else				/* ENABLE_DMX_ADVANCED_VER */
	/* Check whether the Cmd Queue Cross PbBuf Slots */
	if (prPsrCC->fgUseCmdQ) {
		if ((prPsrCC->u8TxCurrLen < prPsrCC->u8TxLen) &&
		    (prPsrCC->u8TxCurrOffset == prPsrCC->u8TxStartOffset)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d CmdQ TX ")
						  TEXT("Crossing Pbbuf Slot, error!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
#ifdef __linux__
			DMXLOG_ERROR(TEXT("[PSR] %s line %d -- TxCurrLen(%lld), ")
						  TEXT("TxLen(%lld), TxCurrOffset(%lld), ")
						  TEXT("TxStartOffset(%lld)!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u8TxCurrLen,
				    prPsrCC->u8TxLen, prPsrCC->u8TxCurrOffset,
				    prPsrCC->u8TxStartOffset);
#else
			DMXLOG_ERROR(TEXT("[PSR] %s line %d -- TxCurrLen(%I64d),")
						  TEXT(" TxLen(%I64d), TxCurrOffset(%I64d), ")
						  TEXT("TxStartOffset(%I64d)!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u8TxCurrLen,
				    prPsrCC->u8TxLen, prPsrCC->u8TxCurrOffset,
				    prPsrCC->u8TxStartOffset);
#endif				/* #ifdef __linux__ */
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
			if (prPsrCC->u8TxLen > DMX_CMDQ_TX_MAX_SIZE) {
				DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- CmdQ Tx ")
							  TEXT("Video Data Size(%d) > DMX_CMDQ")
							  TEXT("_TX_MAX_SIZE(%d)!!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u8TxLen,
					    DMX_CMDQ_TX_MAX_SIZE);
				if (prPsrCC->u8TxLen > DMX_CMDQ_TX_MAX_SIZE) {
					DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for ")
								  TEXT
								  ("CmdQ Tx Video Data Size(%d) >")
								  TEXT
								  (" DMX_CMDQ_TX_MAX_SIZE(%d)!!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u8TxLen,
						    DMX_CMDQ_TX_MAX_SIZE);
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_PARAM_WRONG);
				}
			}
		}
	}
#endif				/* ENABLE_DMX_ADVANCED_VER */

	if ((0 == prPsrCC->ptrTxCurrSa) || (0 == prPsrCC->u8TxCurrLen)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for ")
					  TEXT("TxCurSa and TxCurrLen == 0!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);

		MM_RETURN(RET_DMX_UNEXPECT);
	}
	/* PSR_Filter_TxToGround do the following tasks: */
	/* Check whether the data needed(u8TxStartOffset
	 * + u8TxLen) is in the pBbuf, */
	/* if not, Set TXS_WAIT_PBBUF txstate */
	/* otherwise, prPsrCC->u4TxPBBufIdx is the pbbuf index,
	 * set u8TxCurrOffset to be u8TxStartOffset + u8TxLen, and Set
	 * Parser CC state to be CCS_INIT, Parser CC TxState to be TXS_TX_OK */
	/* and notify tx done */
	if (!(prPsrFtr->u4Flag & FF_ENABLE)) {
		mrRet = PSR_Filter_TxToGround(prPsrFtr);
		MM_RETURN(mrRet);
	}
	/* If the data is not needed to tx to fifo */
	if (!(prPsrFtr->u4Flag & FF_TX_TO_FIFO))
		MM_RETURN(mrRet);

	mrRet = PSR_Filter_GetFifoFreeSpace(prPsrFtr, &u4Size);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s(u4Handle: 0x%x) line %d fail in")
					     TEXT(" PSR_Filter_GetFifoFreeSpace, mrRet: 0x%x\r\n"),
					    DMX_FUNC_NAME, prPsrFtr->u4ESIH, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}

	if (u4Size <= PSR_RESERVE_FIFO_SPACE) {
		/* DMXLOG_DEBUG(TEXT("[PSR][TXDecide] Wait fifo!\r\n")); */
#if 0
		if ((SplitterRspIsEnabled(prPsrCC->pvSptHdl) && SplitterIsRspOffStart(prPsrCC->pvSptHdl))) {
			DMXLOG_TRACE(
				    TEXT
				    ("[PSR] %s line %d -- FreeSize < PSR_RESERVE_FIFO_SPACE, Wait Fifo\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
		}
#endif
		if (NULL != prPsrCC) {
			if (SplitterRspIsEnabled(prPsrCC->pvSptHdl) &&
			    SplitterIsRspOffStart(prPsrCC->pvSptHdl)) {
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
					TEXT("[PSR] %s line %d -- PsrFtr(Type: %d)")
					 TEXT(" encounter Fifo full in Rsp\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->eType);
				MM_RETURN(RET_DMX_OK);
			}
		}

		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);

		if (SPT_DATA_V == prPsrFtr->eType) {
			u32 u4AvailCnt = 0;

			mrRet = ESM_AUTableGetAvailCount(prPsrFtr->u4ESIH, &u4AvailCnt);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					    TEXT("[PSR] %s(u4Handle: 0x%x) line %d fail in")
					     TEXT(" ESM_AUTableGetAvailCount, mrRet: 0x%x\r\n"),
					    DMX_FUNC_NAME, prPsrFtr->u4ESIH, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}

			if (0 == u4AvailCnt) {
				if (prDmxInst->u4SptCnt > 1)
					PSR_HWRes_Release(prPsrFtr);
				/* sent EOS while playing video in SD Card(mtk40144) */
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
					TEXT("[PSR] %s line %d -- VFifo Full, but has ")
					 TEXT("no Video AU and available size is no more")
					 TEXT(" than 6k, we should sent EOS!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				MM_RETURN(RET_DMX_ERR_DATA);
			}
		}
#if DMX_PFM_TEST
		switch (prPsrFtr->eType) {
		case SPT_DATA_V:
			g_rPsrPfm.rVideo.u4FifoFullCnt++;
			break;
		case SPT_DATA_A:
			g_rPsrPfm.rAudio.u4FifoFullCnt++;
			break;
		case SPT_DATA_SP:
			g_rPsrPfm.rSP.u4FifoFullCnt++;
			break;
		default:
			break;
		}
#endif				/* DMX_PFM_TEST */

		/* Add for Check not sent EOS while playing video in SD Card(mtk40144) */
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
			TEXT("[PSR] %s line %d -- Fifo Full ++++++++++!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);

		MM_RETURN(RET_DMX_FIFO_FULL);
	}

	if (SPT_DATA_V == prPsrFtr->eType) {
		mrRet = PSR_Filter_TxDecideV(prPsrFtr, prPsrCC);
		if (DMX_FAILED(mrRet))
			MM_RETURN(mrRet);
	} else if (SPT_DATA_A == prPsrFtr->eType) {
#if CONFIG_DRV_HIGH_BITRATE_SPECIAL_PROC
		/* If no 2 pipe, bitrate is too high */
		if (!prPsrCC->fgNeedHighBitRateProc) {
			mrRet = PSR_Filter_TxDecideA(prPsrFtr, prPsrCC);
			if (DMX_FAILED(mrRet))
				MM_RETURN(mrRet);
		}
#else
		mrRet = PSR_Filter_TxDecideA(prPsrFtr, prPsrCC);
		if (DMX_FAILED(mrRet))
			MM_RETURN(mrRet);
#endif
	} else if (SPT_DATA_SP == prPsrFtr->eType) {
#if CONFIG_DRV_HIGH_BITRATE_SPECIAL_PROC
		/* If no 2 pipe, bitrate is too high */
		if (!prPsrCC->fgNeedHighBitRateProc) {
			mrRet = PSR_Filter_TxDecideSP(prPsrFtr, prPsrCC);
			if (DMX_FAILED(mrRet))
				MM_RETURN(mrRet);
		}
#else
		mrRet = PSR_Filter_TxDecideSP(prPsrFtr, prPsrCC);
		if (DMX_FAILED(mrRet))
			MM_RETURN(mrRet);
#endif
	} else {
		/* don't care */
	}

	u4Size = u4Size - PSR_RESERVE_FIFO_SPACE;

	if (prPsrCC->fgUseCmdQ) {
		if (u4Size > DMX_CMDQ_TX_MAX_SIZE) {
			DMXLOG_WARN(TEXT("[PSR] %s line %d u4Size (%d)")
						 TEXT(" > DMX_CMDQ_TX_MAX_SIZE(%d)!!\r\n"),
				   DMX_FUNC_NAME, DMX_LINE_NO, u4Size, DMX_CMDQ_TX_MAX_SIZE);
		}
	}

	if (u4Size > DMX_CMDQ_TX_MAX_SIZE)
		u4Size = DMX_CMDQ_TX_MAX_SIZE;

	if (prPsrCC->fgUseCmdQ) {
		mrRet = PSR_Filter_PreChkForCmdQTx(prPsrFtr, u4Size, prPsrCC->u8TxCurrLen);
		if (DMX_FAILED(mrRet))
			MM_RETURN(mrRet);
	} else {
		/* If the free size in Fifo is smaller than the len needed to tx,
		 * change the curtxlen to be the free space size */
		/* the remained data to tx will tx next time */
		if (u4Size < prPsrCC->u8TxCurrLen)
			prPsrCC->u8TxCurrLen = u4Size;
	}

	DMX_ASSERT(prPsrCC->u8TxCurrLen > 0);

	if (prPsrCC->fgUseCmdQ) {
		if (prPsrCC->u8TxCurrLen > DMX_CMDQ_TX_MAX_SIZE) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d Current CmdQ ")
						  TEXT
						  ("Tx Data Len (%d) > DMX_CMDQ_TX_MAX_SIZE(%d)!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u8TxCurrLen,
				    DMX_CMDQ_TX_MAX_SIZE);
		}
	} else {
		if (prPsrCC->u8TxCurrLen > DMX_CMDQ_TX_MAX_SIZE)
			prPsrCC->u8TxCurrLen = DMX_CMDQ_TX_MAX_SIZE;
	}

#if DMX_DRM_DECRYPT_USE_HW
	mrRet = PSR_CC_CalcHWDecryptOfst(prPsrCC, prPsrFtr);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in PSR_CC_Calc")
					  TEXT("HWDecryptOfst(PsrCC: 0x%p), mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}
#endif				/* DMX_DRM_DECRYPT_USE_HW */

	MM_RETURN(mrRet);
}

MRESULT PSR_Filter_MoveFifoWp2AUIdx(PSR_FILTER *prPsrFtr, u32 u4AUIdx)
{
	MRESULT mrRet = RET_DMX_OK;
	u32 u4PrevAUIdx = DMX_INVALID_UINT32;
	void *pvAUInfo = NULL;
	void *pvPrevAUInfo = NULL;
	uintptr_t ptrAUEAddr = DMX_INVALID_UINTPTR_T;
	s32 i;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	for (i = 0; i < MAX_FILTER_COUNT; ++i) {
		if (((PSR_FILTER *) prPsrFtr == g_rPsrMan.aprPsrFtrs[i]) &&
		    (((PSR_FILTER *) prPsrFtr)->u4Flag & FF_USED)) {
			i = MAX_FILTER_COUNT << 1;
			break;
		}
	}
	if (i == MAX_FILTER_COUNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- u4AUIdx: 0x%x, eType: %s\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx,
		    ((prPsrFtr->eType <
		      MAX_SPT_DATA_TYPE_CNT) ? g_aszSptDataTypeName[prPsrFtr->eType] :
		     TEXT("UNKNOWN")));

	if (ESM_INVALID_INDEX == u4AUIdx) {
		DMXLOG_ERROR(TEXT("[PSR] %s fail for invalid AUIdx: %d\r\n"),
			    DMX_FUNC_NAME, u4AUIdx);
		MM_RETURN(mrRet);
	}

	mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4AUIdx, &pvAUInfo);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ESM_AUTableGetAUInfo")
					  TEXT(" (u4AUIdx: %d, eType: %s)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx,
			    ((prPsrFtr->eType <
			      MAX_SPT_DATA_TYPE_CNT) ? g_aszSptDataTypeName[prPsrFtr->eType] :
			     TEXT("UNKNOWN")));
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	mrRet = ESM_AUTableGetPrevAUIdx(prPsrFtr->u4ESIH, u4AUIdx, 1, &u4PrevAUIdx);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ESM_AUTableGet")
					  TEXT("PrevAUIdx (u4AUIdx: %d, eType: %s)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx,
			    ((prPsrFtr->eType <
			      MAX_SPT_DATA_TYPE_CNT) ? g_aszSptDataTypeName[prPsrFtr->eType] :
			     TEXT("UNKNOWN")));
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- (u4AUIdx: ")
				  TEXT("0x%x, u4PrevAUIdx: 0x%x), eType: %s\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx, u4PrevAUIdx,
		    ((prPsrFtr->eType <
		      MAX_SPT_DATA_TYPE_CNT) ? g_aszSptDataTypeName[prPsrFtr->eType] :
		     TEXT("UNKNOWN")));

	if (ESM_INVALID_INDEX != u4PrevAUIdx) {
		mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4PrevAUIdx, &pvPrevAUInfo);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ESM_AUTable")
						  TEXT
						  ("GetAUInfo(u4PrevAUIdx: %d, eType: %s)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u4PrevAUIdx,
				    ((prPsrFtr->eType <
				      MAX_SPT_DATA_TYPE_CNT) ? g_aszSptDataTypeName[prPsrFtr->eType]
				     : TEXT("UNKNOWN")));
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		switch (prPsrFtr->eType) {
		case SPT_DATA_V:
			{
				AU_VPic *prPrevVAUInfo = (AU_VPic *) pvPrevAUInfo;

				ptrAUEAddr = prPrevVAUInfo->rAUInfo.rInfo.ptrEAddr;
				if (0 != ptrAUEAddr)
					dmx_memset(pvAUInfo, 0, sizeof(AU_VPic));
			}
			break;
		case SPT_DATA_A:
			{
				AU_AUDIO *prPrevAAUInfo = (AU_AUDIO *) pvPrevAUInfo;

				ptrAUEAddr = prPrevAAUInfo->ptrEAddr;
				if (0 != ptrAUEAddr)
					dmx_memset(pvAUInfo, 0, sizeof(AU_AUDIO));
			}
			break;
		case SPT_DATA_SP:
			{
				AU_SP *prPrevSPAUInfo = (AU_SP *) pvPrevAUInfo;
				DMX_FIFO_INFO_T *prFifoInfo = NULL;

				ptrAUEAddr = prPrevSPAUInfo->rAUInfo.rInfo.ptrAddr +
				    prPrevSPAUInfo->rAUInfo.rInfo.u4Size;
				mrRet = ESM_FifoGetInfo(prPsrFtr->u4ESIH, (void **) (&prFifoInfo));
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(
						    TEXT
						    ("[PSR] %s line %d fail in ESM_FifoGetInfo(eType: %s)\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    ((prPsrFtr->eType <
						      MAX_SPT_DATA_TYPE_CNT) ?
						     g_aszSptDataTypeName[prPsrFtr->eType] :
						     TEXT("UNKNOWN")));
					DMX_ASSERT(FALSE);
					MM_RETURN(mrRet);
				}

				if (NULL == prFifoInfo) {
					DMXLOG_ERROR(
						    TEXT("[PSR] %s line %d fail for the ")
						     TEXT("prFifoInfo is NULL which is obtained ")
						     TEXT("by ESM_FifoGetInfo(eType: %s)\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    ((prPsrFtr->eType <
						      MAX_SPT_DATA_TYPE_CNT) ?
						     g_aszSptDataTypeName[prPsrFtr->eType] :
						     TEXT("UNKNOWN")));
					DMX_ASSERT(FALSE);
					MM_RETURN(mrRet);
				}

				if (ptrAUEAddr >= prFifoInfo->ptrEa) {
					ptrAUEAddr =
					    ptrAUEAddr - (prFifoInfo->ptrEa - prFifoInfo->ptrSa);
				}
				if (0 != ptrAUEAddr)
					dmx_memset(pvAUInfo, 0, sizeof(AU_SP));
			}
			break;
		case SPT_DATA_SECTION:
			{
				AU_SECTION *prPrevSctAUInfo = (AU_SECTION *) pvPrevAUInfo;

				ptrAUEAddr = prPrevSctAUInfo->ptrEAddr;
				if (0 != ptrAUEAddr)
					dmx_memset(pvAUInfo, 0, sizeof(AU_SECTION));
			}
			break;
		default:
			MM_RETURN(RET_DMX_OK);
		}
	}

	if ((ESM_INVALID_INDEX == u4PrevAUIdx) || (0 == ptrAUEAddr)) {
		switch (prPsrFtr->eType) {
		case SPT_DATA_V:
			{
				AU_VPic *prVAUInfo = (AU_VPic *) pvAUInfo;

				ptrAUEAddr = prVAUInfo->rAUInfo.rInfo.ptrSAddr;
				dmx_memset(pvAUInfo, 0, sizeof(AU_VPic));
			}
			break;
		case SPT_DATA_A:
			{
				AU_AUDIO *prAAUInfo = (AU_AUDIO *) pvAUInfo;

				ptrAUEAddr = prAAUInfo->ptrSAddr;
				dmx_memset(pvAUInfo, 0, sizeof(AU_AUDIO));
			}
			break;
		case SPT_DATA_SP:
			{
				AU_SP *prSPAUInfo = (AU_SP *) pvAUInfo;

				ptrAUEAddr = prSPAUInfo->rAUInfo.rInfo.ptrAddr;
				dmx_memset(pvAUInfo, 0, sizeof(AU_SP));
			}
			break;
		case SPT_DATA_SECTION:
			{
				AU_SECTION *prSctAUInfo = (AU_SECTION *) pvAUInfo;

				ptrAUEAddr = prSctAUInfo->ptrSAddr;
				dmx_memset(pvAUInfo, 0, sizeof(AU_SECTION));
			}
			break;
		default:
			MM_RETURN(RET_DMX_OK);
		}
	}

	DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- u4AUIdx: 0x%x,")
				  TEXT(" u4PrevAUIdx: 0x%x, ptrAUEAddr: 0x%x, eType: %s\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx, u4PrevAUIdx, ptrAUEAddr,
		    ((prPsrFtr->eType <
		      MAX_SPT_DATA_TYPE_CNT) ? g_aszSptDataTypeName[prPsrFtr->eType] :
		     TEXT("UNKNOWN")));

	/* no au has been composed */
	if ((0 == ptrAUEAddr) && (ESM_INVALID_INDEX == u4PrevAUIdx) && (0 == u4AUIdx)) {
		ptrAUEAddr = prPsrFtr->ptrESFifoSa;
		DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- set ptrAUEAddr = 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, ptrAUEAddr);
	}

	mrRet = ESM_AUTableSetWrIdx(prPsrFtr->u4ESIH, u4AUIdx);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ESM_AUTableSetWrIdx")
					  TEXT(" (u4PrevAUIdx: %d, u4AUIdx: %d, eType: %s)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u4PrevAUIdx, u4AUIdx,
			    ((prPsrFtr->eType <
			      MAX_SPT_DATA_TYPE_CNT) ? g_aszSptDataTypeName[prPsrFtr->eType] :
			     TEXT("UNKNOWN")));
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if ((prPsrFtr->ptrESFifoSa <= ptrAUEAddr) && (ptrAUEAddr <= prPsrFtr->ptrESFifoEa)) {
		mrRet = ESM_FifoSetWrPtr(prPsrFtr->u4ESIH, ptrAUEAddr);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ESM_FifoSetWrPtr ")
						  TEXT
						  ("(u4PrevAUIdx: %d, u4AUIdx: %d, u4WrPtr: 0x%x, eType: %s)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u4PrevAUIdx, u4AUIdx, ptrAUEAddr,
				    ((prPsrFtr->eType <
				      MAX_SPT_DATA_TYPE_CNT) ? g_aszSptDataTypeName[prPsrFtr->eType]
				     : TEXT("UNKNOWN")));
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
	}

	switch (prPsrFtr->eType) {
	case SPT_DATA_V:
		{
			PSR_VFSD *prVFSD = (PSR_VFSD *) (prPsrFtr->pvFilterSpecific);

			if (NULL != prVFSD) {
				prVFSD->u4RealWrIdx = u4AUIdx;
				prVFSD->fgUseRealWrIdx = TRUE;
				prVFSD->u4DummyAURealWrIdx = 0;
				prVFSD->fgUseDummyAURealWrIdx = FALSE;
				prVFSD->fgHasSetPics = FALSE;
				prVFSD->u4VType = 0;
				prVFSD->prHALStatus->u4PTransStatus = 0;
				prVFSD->prHALStatus->u4PTransStatus1 = 0;
				prVFSD->prHALStatus->u4HdrDectQueNum = 0;
				prVFSD->prHALStatus->eHdrDectQueType = HDQ_PICTYPE;
				prVFSD->prHALStatus->u4HdrDectQueData = 0;
				dmx_memset(&(prVFSD->rPicDetResult), 0,
					   sizeof(prVFSD->rPicDetResult));
			}
		}
		break;
	case SPT_DATA_A:
		{
			PSR_AUDFSD *prAFSD = (PSR_AUDFSD *) (prPsrFtr->pvFilterSpecific);

			if (NULL != prAFSD) {
				prAFSD->prHALStatus->u4PTransStatus = 0;
				prAFSD->prHALStatus->u4PTransStatus1 = 0;
				prAFSD->prHALStatus->u4HdrDectQueNum = 0;
				prAFSD->prHALStatus->eHdrDectQueType = HDQ_PICTYPE;
				prAFSD->prHALStatus->u4HdrDectQueData = 0;
			}
		}
		break;
	case SPT_DATA_SP:
	case SPT_DATA_SECTION:
		{
			PSR_NORMALFSD *prAFSD = (PSR_NORMALFSD *) (prPsrFtr->pvFilterSpecific);

			if (NULL != prAFSD) {
				prAFSD->prHALStatus->u4PTransStatus = 0;
				prAFSD->prHALStatus->u4PTransStatus1 = 0;
				prAFSD->prHALStatus->u4HdrDectQueNum = 0;
				prAFSD->prHALStatus->eHdrDectQueType = HDQ_PICTYPE;
				prAFSD->prHALStatus->u4HdrDectQueData = 0;
			}
		}
		break;
	default:
		break;
	}

	prPsrFtr->u8HdrPTS = INVALID_TIMESTAMP;
	prPsrFtr->u8LastPTS = INVALID_TIMESTAMP;

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_Filter_Reset4Enable(PSR_FILTER *prPsrFtr)
{
	MRESULT mrRet = RET_DMX_OK;

	mrRet = PSR_Filter_Disable(prPsrFtr);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
					  TEXT("PSR_Filter_Enable (eType: %s, FALSE)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO,
			    ((prPsrFtr->eType <
			      MAX_SPT_DATA_TYPE_CNT) ?
			     g_aszSptDataTypeName[prPsrFtr->eType] : TEXT("UNKNOWN")));
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}
	switch (prPsrFtr->eType) {
	case SPT_DATA_V:
	case SPT_DATA_A:
	case SPT_DATA_SP:
	case SPT_DATA_SECTION:
		{
			u32 u4AUIdx = ESM_INVALID_INDEX;

			mrRet = ESM_AUTableGetWrIdx(prPsrFtr->u4ESIH, &u4AUIdx);
			if (DMX_SUCCEED(mrRet) && (ESM_INVALID_INDEX != u4AUIdx)) {
				DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- PSR_Filter_")
							  TEXT
							  ("MoveFifoWp2AUIdx(u4AUIdx: 0x%x), eType: %s\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx,
					    ((prPsrFtr->eType <
					      MAX_SPT_DATA_TYPE_CNT) ?
					     g_aszSptDataTypeName[prPsrFtr->eType] :
					     TEXT("UNKNOWN")));

				mrRet = PSR_Filter_MoveFifoWp2AUIdx(prPsrFtr, u4AUIdx);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(
						    TEXT("[PSR] %s line %d fail in PSR_")
						     TEXT("Filter_MoveFifoWp2AUIdx ")
						     TEXT("(eType: %s, FALSE)\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    ((prPsrFtr->eType <
						      MAX_SPT_DATA_TYPE_CNT) ?
						     g_aszSptDataTypeName[prPsrFtr->eType] :
						     TEXT("UNKNOWN")));
					DMX_ASSERT(FALSE);
					MM_RETURN(mrRet);
				}
			}
		}
		break;
	default:
		break;
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_Filter_Reset(PSR_FILTER *prPsrFtr)
{
	MRESULT mrRet = RET_DMX_OK;
	u32 u4Flag = 0;
	s32 i;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	for (i = 0; i < MAX_FILTER_COUNT; ++i) {
		if (((PSR_FILTER *) prPsrFtr == g_rPsrMan.aprPsrFtrs[i]) &&
		    (((PSR_FILTER *) prPsrFtr)->u4Flag & FF_USED)) {
			i = MAX_FILTER_COUNT << 1;
			break;
		}
	}
	if (i == MAX_FILTER_COUNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	u4Flag = prPsrFtr->u4Flag;

	if (0 != (u4Flag & FF_USED)) {
		bool fgEnable = (0 != (u4Flag & FF_ENABLE)) ? TRUE : FALSE;

		if (fgEnable) {
			mrRet = PSR_Filter_Reset4Enable(prPsrFtr);
			if (DMX_FAILED(mrRet))
				MM_RETURN(mrRet);
		}

		prPsrFtr->fgFirstAUInRng = TRUE;
#ifdef MM_SUPPORT_DIVXHT31
		if (SPT_DATA_V == prPsrFtr->eType) {
			PSR_CC *prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);

			if (NULL != prPsrCC) {
				if (CFA_TYPE_AVI == SplitterGetCfaType(prPsrCC->pvSptHdl)) {
					if (DMX_IS_FF_PLAY(prPsrCC->pvSptHdl))
						prPsrFtr->fgFirstAUInRng = FALSE;
				}
			}
		}
#endif				/* MM_SUPPORT_DIVXHT31 */

		prPsrFtr->u8TxCurrOffset = 0;
		prPsrFtr->ucHwDevId = DMX_INVALID_UINT8;
		prPsrFtr->fgAUCtrlByLen = FALSE;
		prPsrFtr->fgAUCtrlByEnd = FALSE;
		prPsrFtr->u8TotalAULen = 0;
		prPsrFtr->u8CurAULen = 0;
		prPsrFtr->u8WMDRMTxLen = 0;
		prPsrFtr->u8HdrPTS = INVALID_TIMESTAMP;
		prPsrFtr->u8LastPTS = INVALID_TIMESTAMP;
		prPsrFtr->fgAUEnd = FALSE;
		prPsrFtr->u4AUExtCnt = 0;
		prPsrFtr->u4Flag = FF_USED;

		if (SPT_DATA_BUF == prPsrFtr->eType) {
			PSR_DMASD *prDMASD = (PSR_DMASD *) (prPsrFtr->pvFilterSpecific);

			if (NULL != prDMASD) {
				if (0 != prDMASD->ptrPrivMemSa) {
					DMX_FreeHwMemory(prDMASD->ptrPrivMemSa);
					prDMASD->ptrPrivMemSa = 0;
				}
				dmx_memset(prPsrFtr->pvFilterSpecific, 0, sizeof(PSR_DMASD));
			}
		}

		if (fgEnable) {
			mrRet = PSR_Filter_Enable(prPsrFtr);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
							  TEXT
							  ("PSR_Filter_Enable (eType: %s, TRUE)\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO,
					    ((prPsrFtr->eType <
					      MAX_SPT_DATA_TYPE_CNT) ?
					     g_aszSptDataTypeName[prPsrFtr->eType] :
					     TEXT("UNKNOWN")));
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}
		}
	} else {
		DMXLOG_ERROR(TEXT("[PSR] %s -- prPsrFtr(eType: %d,")
					  TEXT(" flag: 0x%x) is an unused parser filter \r\n"),
			    DMX_FUNC_NAME, prPsrFtr->eType, u4Flag);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	MM_RETURN(RET_DMX_OK);
}
