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
 * @file dmx_psr_esm.c
 *
 * @par Project
 *
 * @par Description
 *
 *
 * @par Author_Name
 *		Shuhui Zhang
 *
 */
#ifdef __linux__
#include <linux/mm.h>
#include "windows.h"
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_decrypt.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "dmx_decrypt.h"
#include "mm_debug.h"
#endif				/* __linux__ */

#include "x_debug.h"

#include "dmx_def.h"

#include "dmx_esm_if.h"
#include "dmx_esm.h"
#include "dmx_psr_esm.h"
#include "dmx_parser.h"
#include "dmx_mem.h"
#include "dmx_spt.h"
#include "dmx_gau_if.h"
#include "dmx_spt_util.h"
#include "dmx_cli.h"
#include "dmx_dump.h"
#include "dmx_log.h"
#include "stc_hal.h"
#include "dmx_inst.h"

#ifndef __linux__
/*disable warning C4127: conditional expression is constant */
#pragma warning(disable : 4127)
/*disable warning C6011: Dereferencing NULL pointer */
#pragma warning(disable : 6011)
#endif

#define DMX_DBG_ESM 0

EXTERN DMX_CLI_MAN_T g_rDmxCliMan;
EXTERN DMX_SPT_MAN_INFO_T g_rSptMan;


MRESULT PSR_Filter_IsAUTableFull(PSR_FILTER *prPsrFtr, bool *pfgFull)
{
	u32 u4FreeCount = 0;
	PSR_CC *prPsrCC = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == pfgFull)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	*pfgFull = FALSE;

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);
	if (NULL == prPsrCC) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
			TEXT("[PSR] %s fail for PsrFtr(0x%x)'s pvPsrCC == NULL\r\n"),
			DMX_FUNC_NAME, prPsrFtr);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mrRet = ESM_AUTableGetFreeCount(prPsrFtr->u4ESIH, &u4FreeCount);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
			TEXT("[PSR] %s line %d fail in ESM_AUTableGetFreeCount(u4Handle: 0x%x),")
			 TEXT(" mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		MM_RETURN(mrRet);
	}

	if (SPT_DATA_V == prPsrFtr->eType) {
		if ((SplitterRspIsEnabled(prPsrCC->pvSptHdl) &&
			SplitterIsRspOffStart(prPsrCC->pvSptHdl))) {
			if (u4FreeCount >= (DMX_MAX_VID_STARTCODE_CNT / 2 +
				prPsrFtr->u4AUExtCnt)) {
				MM_RETURN(RET_DMX_OK);
			} else {
				DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPOFF,
					TEXT
					 ("[PSR] %s line %d -- Need to do Resplitter, FreeCnt: %d")
					 TEXT(" < 105 ++++++++++\r\n"), DMX_FUNC_NAME, DMX_LINE_NO,
					u4FreeCount);
			}
		} else if (DMX_IS_RW_PLAY(prPsrCC->pvSptHdl)) {
			if (prPsrFtr->u4IFrmCnt > DMX_IFRM_CNT_IN_CTRL_RW) {
				*pfgFull = TRUE;
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
					TEXT("[PSR] %s line %d -- Video AU FULL ++++++++++\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				MM_RETURN(RET_DMX_OK);
			}
		}
		if ((DMX_MAX_VID_STARTCODE_CNT + prPsrFtr->u4AUExtCnt) > u4FreeCount) {
			*pfgFull = TRUE;
			if (NULL != prPsrCC) {
				if (SplitterRspIsEnabled(prPsrCC->pvSptHdl) &&
						SplitterIsRspOffStart(prPsrCC->pvSptHdl)) {
					DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPOFF,
						TEXT
						 ("[PSR] %s line %d -- PsrFtr(Type: %d) encounter ")
						 TEXT("AUTable full in Rsp\r\n"), DMX_FUNC_NAME,
						DMX_LINE_NO, prPsrFtr->eType);
				}
			}
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
				TEXT("[PSR] %s line %d -- Video AU FULL ++++++++++\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_OK);
		}
	}

	if ((prPsrCC->fgUseCmdQ) && (prPsrCC->fgAUByCmdQEnd)) {
		if ((DMX_STM_AU_CNT_THRESHOLD + DMX_MAX_TX_CNT_FOR_CMD_Q +
				 prPsrFtr->u4AUExtCnt) > u4FreeCount) {
			if (NULL != prPsrCC) {
				if (SplitterRspIsEnabled(prPsrCC->pvSptHdl) &&
						SplitterIsRspOffStart(prPsrCC->pvSptHdl)) {
					DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPOFF,
						TEXT
						 ("[PSR] %s line %d -- PsrFtr(Type: %d) encounter ")
						 TEXT("AUTable full in Rsp\r\n"), DMX_FUNC_NAME,
						DMX_LINE_NO, prPsrFtr->eType);
				}
			}
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
				TEXT("[PSR] %s line %d -- Video AU FULL ++++++++++\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			*pfgFull = TRUE;
		}
	} else {
		if ((DMX_STM_AU_CNT_THRESHOLD + prPsrFtr->u4AUExtCnt) > u4FreeCount) {
			if (NULL != prPsrCC) {
				if (SplitterRspIsEnabled(prPsrCC->pvSptHdl) &&
						SplitterIsRspOffStart(prPsrCC->pvSptHdl)) {
					DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPOFF,
						TEXT
						 ("[PSR] %s line %d -- PsrFtr(Type: %d) encounter ")
						 TEXT("AUTable full in Rsp\r\n"), DMX_FUNC_NAME,
						DMX_LINE_NO, prPsrFtr->eType);
				}
			}
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
				TEXT("[PSR] %s line %d -- Video AU FULL ++++++++++\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			*pfgFull = TRUE;
		}
	}

	MM_RETURN(RET_DMX_OK);
}

void PSR_Filter_ESICB4AUOut(PSR_FILTER *prPsrFtr,
	PSR_CC *prPsrCC, void *pvData)
{
	DMX_INST_T *prDmxInst = NULL;
	
	if (NULL == prPsrCC) {
		DMX_ASSERT(0);
		return;
	}
	prDmxInst = (DMX_INST_T *)prPsrCC->pvDmxInst;

	if (NULL == prDmxInst) {
		DMX_ASSERT(0);
		return;
	}
	if ((NULL != pvData) && (SPT_DATA_V == prPsrFtr->u4StmType)) {
		AU_VPic *prVidAU = (AU_VPic *) pvData;
#ifdef MM_SUPPORT_DIVXHT31
		if (DMX_IS_NORMAL_PLAY(prPsrCC->pvSptHdl)) {
			if (INVALID_TIMESTAMP != prVidAU->rAUInfo.rInfo.u8Pts) {
				u32 i = 0;
				PSR_CC *prPsrCC2 = NULL;

				prPsrCC->u8BaseSTC = prVidAU->rAUInfo.rInfo.u8Pts;
				for (i = 0; i < DMX_MAX_SPT_INST_CNT; i++) {
					prPsrCC2 = (PSR_CC *) ((NULL !=
								g_rSptMan.aprSptInst[i]) ?
										 ((g_rSptMan.aprSptInst[i])->
								pvPsrCC) : NULL);
					if ((NULL == prPsrCC2) || (prPsrCC2->pvDmxInst != (void *)prDmxInst))
						continue;
					if ((NULL != prPsrCC2) && (prPsrCC != prPsrCC2)
							&& (!(prPsrCC2->fgCfaPrsEnd))) {
						prPsrCC2->u8BaseSTC = prPsrCC->u8BaseSTC;
					}
				}
			}
		}
#endif				/* MM_SUPPORT_DIVXHT31 */
		if (DMX_IS_RW_PLAY(prPsrCC->pvSptHdl)) {
			if (fgIsIType(prVidAU->rAUInfo.rInfo.u4VType) &&
			 (prPsrFtr->u4IFrmCnt > 0)) {
				prPsrFtr->u4IFrmCnt--;
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FFRW,
					TEXT("[PSR] %s IFrmCnt: %d\r\n"),
					DMX_FUNC_NAME, prPsrFtr->u4IFrmCnt);
			}
		}
	}
}

void PSR_Filter_ESICB4RspOff(PSR_FILTER *prPsrFtr, PSR_CC *prPsrCC)
{
	u32 u4AvailCnt = 0;

	switch (prPsrFtr->eType) {
	case SPT_DATA_V:{
			if (RET_DMX_OK == ESM_AUTableGetAvailCount(prPsrFtr->u4ESIH,
				&u4AvailCnt)) {
				if (u4AvailCnt <= DMX_STM_AU_CNT_THRESHOLD) {
					if ((SplitterRspIsEnabled(prPsrCC->pvSptHdl) &&
							 SplitterIsRspOffStart(prPsrCC->pvSptHdl))) {
						DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPOFF,
							TEXT
							 ("[PSR] %s line %d -- RspOffStart -- Wake")
							 TEXT(" Up Now (PsrType: %d)\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							prPsrFtr->eType);
					}
					PSR_CC_CBSplitter((HANDLE) prPsrCC, E_WAKEUP_ME, NULL);
					break;
				}
			}
		}
		break;
	case SPT_DATA_SP:{
			if (RET_DMX_OK == ESM_AUTableGetAvailCount(prPsrFtr->u4ESIH,
				&u4AvailCnt)) {
				if ((SplitterRspIsEnabled(prPsrCC->pvSptHdl) &&
						 SplitterIsRspOffStart(prPsrCC->pvSptHdl))) {
					DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPOFF,
						TEXT("[PSR] %s line %d -- RspOffStart -- Wake Up")
						 TEXT(" Now (PsrType: %d)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->eType);
				}
				if (u4AvailCnt <= DMX_STM_AU_CNT_THRESHOLD) {
					PSR_CC_CBSplitter((HANDLE) prPsrCC, E_WAKEUP_ME, NULL);
					break;
				}
			}
		}
		break;
	case SPT_DATA_A:{
			u32 u4AvailDataSz = 0;

			if ((MM_IS_NORMAL_PLAY(SplitterGetPlayRate(prPsrCC->pvSptHdl))) &&
					(RET_DMX_OK == ESM_FifoGetAvailDataSize(prPsrFtr->u4ESIH,
										&u4AvailDataSz))
					&& (0 < prPsrFtr->u4ESFifoSize)) {
				if (u4AvailDataSz * 100 / prPsrFtr->u4ESFifoSize <
						AUD_FIFO_USAGE_RATE_THRESHOLD / 3) {
					if ((SplitterRspIsEnabled(prPsrCC->pvSptHdl)
							 && SplitterIsRspOffStart(prPsrCC->pvSptHdl))) {
						DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPOFF,
							TEXT
							 ("[PSR] %s line %d -- RspOffStart -- Wake")
							 TEXT(" Up Now (PsrType: %d)\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							prPsrFtr->eType);
					}
					PSR_CC_CBSplitter((HANDLE) prPsrCC, E_WAKEUP_ME, NULL);
					break;
				}
			}
		}
		break;
	default:
		break;

	}
}

void PSR_Filter_ESICBWakeupOtherPsr(PSR_FILTER *prPsrFtr,
	PSR_CC *prPsrCC)
{
	if (!prPsrCC->fgCfaPrsEnd) {
		bool fgEOS = TRUE;
		u32 u4Status = 0;

		GAU_GetEOSStatus(prPsrFtr->u4GAU, &fgEOS, &u4Status);
		if (!(fgEOS && ((GAU_E_ERRDATA == u4Status) ||
			(GAU_E_ERRCHUNK == u4Status))))
			PSR_CC_CBSplitter(prPsrCC, E_WAKEUP_ME, NULL);
	} else {
		if ((SplitterRspIsEnabled(prPsrCC->pvSptHdl) &&
			SplitterIsRspOffStart(prPsrCC->pvSptHdl))) {
			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPOFF,
				TEXT("[PSR] %s line %d -- RspOffStart -- Wake Up Now ")
				 TEXT("(PsrType: %d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->eType);
		}
		if (prPsrFtr == prPsrCC->pvActFilter) {
			if (SPT_DATA_V == prPsrFtr->eType) {
				/* Tx Video Data from Pbbuf Into FIFO */
				PSR_VFSD *prVFSD = (PSR_VFSD *) prPsrFtr->pvFilterSpecific;

				if ((NULL != prVFSD) && (prVFSD->fgDummyTxWakeUp))
					PSR_CC_CBSplitter(prPsrCC, E_WAKEUP_ME, NULL);
			} else {
				PSR_CC_CBSplitter(prPsrCC, E_WAKEUP_ME, NULL);
			}
		}
	}
}

bool PSR_Filter_ESICBCheckPsrEnd(PSR_FILTER *prPsrFtr)
{
	s32 i = 0;

	DMX_SPT_INST_T *prSptInst = NULL;
	PSR_CC *prPsrCC2 = NULL;
	DMX_INST_T *prDmxInst = NULL;

	if (NULL == prPsrFtr) {
		DMX_ASSERT(0);
		return FALSE;
	}
	prDmxInst = (DMX_INST_T *)prPsrFtr->pvDmxInst;

	if (NULL == prDmxInst) {
		DMX_ASSERT(0);
		return FALSE;
	}
	
	for (i = 0; i < DMX_MAX_SPT_INST_CNT; i++) {
		prSptInst = g_rSptMan.aprSptInst[i];
		if ((NULL == prSptInst) ||
			(NULL == prSptInst->pvDmxInst) ||
			(prSptInst->pvDmxInst != (void *)prDmxInst))
			continue;
		if ((NULL != prSptInst->pvPsrCC) && 
				(prSptInst->pvDmxInst == prPsrFtr->pvDmxInst) &&
				(prPsrFtr->pvPsrCC != prSptInst->pvPsrCC)) {
			prPsrCC2 = (PSR_CC *) (prSptInst->pvPsrCC);
			if (prPsrCC2->fgCfaPrsEnd)
				break;
		}
	}
	if (i >= MAX_SPT_INST_CNT_PER_DMX)
		return FALSE;

	return TRUE;
}

void PSR_Filter_ESICB(ES_CBEVENT eEvent, void *pvData, void *pvPrivate)
{
	PSR_CC *prPsrCC;
	PSR_FILTER *prPsrFtr = (PSR_FILTER *) pvPrivate;
	DMX_INST_T *prDmxInst = NULL;
	s32 i = 0;

	UNUSE_PARAMETER(pvData);

	if (NULL == prPsrFtr) {
		DMX_ASSERT(FALSE);
		return;
	}

	prDmxInst = (DMX_INST_T *)prPsrFtr->pvDmxInst;

	if (NULL == prDmxInst) {
		DMX_ASSERT(FALSE);
		return;
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
		return;
	}

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);
	if (NULL == prPsrCC)
		return;

	if ((CBE_FIFO_OUT != eEvent) && (CBE_AU_OUT != eEvent))
		return;

	PSR_CC_LOCK(prPsrCC->rLock);

	if (CBE_AU_OUT == eEvent) {
		if (pvData != NULL)
			PSR_Filter_ESICB4AUOut(prPsrFtr, prPsrCC, pvData);
		else if (prPsrFtr->eType == SPT_DATA_A) {
			if (1 < prDmxInst->u4SptCnt) {
				if (!PSR_Filter_ESICBCheckPsrEnd(prPsrFtr)) {
					PSR_CC_UNLOCK(prPsrCC->rLock);
					return;
				}
		  }
		}
	}

	if ((CBE_FIFO_OUT == eEvent) &&
			(prPsrCC->u4AVStmFlags != prPsrCC->u4AVStmPlayFlags) &&
			((SPT_DATA_V == prPsrFtr->u4StmType) ||
			 (SPT_DATA_A == prPsrFtr->u4StmType))) {
		prPsrCC->u4AVStmPlayFlags |= (u32) (1 << (prPsrFtr->u4StmType));
		if (prPsrCC->u4AVStmFlags == prPsrCC->u4AVStmPlayFlags) {
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[PSR_CC] %s --- prPsrCC->u4AVStmFlags: 0x%x, prPsrCC->")
				 TEXT("u4AVStmPlayFlags: 0x%x\r\n"),
				DMX_FUNC_NAME, prPsrCC->u4AVStmFlags, prPsrCC->u4AVStmPlayFlags);
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("[PSR] %s line %d -- Set Spt(0x%x)'s Repeat Error Chunk ")
				 TEXT("Count to be 0, eEvent: %d\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->pvSptHdl, eEvent);
			SplitterSetRepeatErrChkCnt(prPsrCC->pvSptHdl, 0);
		}
	}

	if ((!(prPsrFtr->u4Flag & FF_USED)) || (!(prPsrFtr->u4Flag & FF_ENABLE))) {
		PSR_CC_UNLOCK(prPsrCC->rLock);
		return;
	}

	/*add check for Rsp wait PTS */
	/*check in every ESI, for audio slide show */
	if ((prPsrFtr != prPsrCC->pvActFilter) &&
			(TXS_WAIT_VFIFO_PTS_THRESHOLD != prPsrCC->eTxState)) {
		PSR_CC_UNLOCK(prPsrCC->rLock);
		return;
	}

	if ((CCS_TX != prPsrCC->eState) ||
			((TXS_PBBUF_OK != prPsrCC->eTxState) &&
			 (TXS_WAIT_FIFO != prPsrCC->eTxState) &&
			 (TXS_WAIT_VFIFO_PTS_THRESHOLD != prPsrCC->eTxState) &&
			 (TXS_TXING != prPsrCC->eTxState))) {
		if (prPsrCC->fgCfaPrsEnd) {
			u32 u4CheckTxState = (u32) (1 << TXS_WAIT_VFIFO_PTS_THRESHOLD);

			if (PsrWakeupOtherPsrCC(prPsrFtr, prPsrCC, u4CheckTxState)) {
				PSR_CC_UNLOCK(prPsrCC->rLock);
				return;
			}
		} else if ((1 < prDmxInst->u4SptCnt) &&
				 (prPsrCC->eState == CCS_TX) &&
				 ((prPsrCC->eTxState == TXS_WAIT_FIFO) ||
					(prPsrCC->eTxState == TXS_WAIT_VFIFO_PTS_THRESHOLD))) {
			PSR_Filter_ESICB4RspOff(prPsrFtr, prPsrCC);
		}
		PSR_CC_UNLOCK(prPsrCC->rLock);
		return;
	}

	if (TXS_TXING != prPsrCC->eTxState) {
		/* wake up */
		if ((prPsrCC->eState == CCS_TX) &&
				((prPsrCC->eTxState == TXS_WAIT_FIFO) ||
				 (prPsrCC->eTxState == TXS_WAIT_VFIFO_PTS_THRESHOLD))) {
			/* For avoid two ESI CB wake up waiting tx @20090613 */
			/* 2 pipe, timing problem, 23033 */
			if (prPsrCC->fgWakingWaitTx) {
				PSR_CC_UNLOCK(prPsrCC->rLock);
				return;
			}

			prPsrCC->fgWakingWaitTx = TRUE;

			/* Add End */
			PSR_Filter_ESICBWakeupOtherPsr(prPsrFtr, prPsrCC);
		}
	}

	PSR_CC_UNLOCK(prPsrCC->rLock);
}

MRESULT PSR_Filter_GetFifoFreeSpace(PSR_FILTER *prPsrFtr, u32 *pu4AvailSize)
{
	u32 u4Size = 0;
	MRESULT mrRet = RET_DMX_OK;

	mrRet = ESM_FifoGetAvailDataSize(prPsrFtr->u4ESIH, &u4Size);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[PSR] %s line %d (ESIH: 0x%x) fail in ESM_FifoGetAvailDataSize,")
			 TEXT(" mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		MM_RETURN(mrRet);
	}

	if (DMX_INVALID_UINT32 == u4Size) {
		DMX_ASSERT(FALSE);
		*pu4AvailSize = 0;
		MM_RETURN(RET_DMX_OK);
	}

	*pu4AvailSize = prPsrFtr->u4ESFifoSize - u4Size;
	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_Filter_IsESBufFull(PSR_FILTER *prPsrFtr,
	u32 u4Len, bool *pfgFull)
{
	u32 u4Size;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pfgFull) || (NULL == prPsrFtr)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
			TEXT("[PSR] %s fail for invalid args\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DMX_ASSERT(pfgFull);

	*pfgFull = FALSE;

	mrRet = PSR_Filter_GetFifoFreeSpace(prPsrFtr, &u4Size);
	if (DMX_FAILED(mrRet))
	{
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
			TEXT("[PSR] %s fail in PSR_Filter_GetFifoFreeSpace.\r\n"), DMX_FUNC_NAME);
		MM_RETURN(mrRet);
	}
	
	if (u4Size < (u4Len + PSR_RESERVE_FIFO_SPACE)) {
		PSR_CC *prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);

		if (NULL != prPsrCC) {
			if (SplitterRspIsEnabled(prPsrCC->pvSptHdl) &&
					SplitterIsRspOffStart(prPsrCC->pvSptHdl)) {
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL,
					TEXT
					 ("[PSR] %s line %d -- PsrFtr(Type: %d) encounter Fifo ")
					 TEXT("full in Rsp\r\n"), DMX_FUNC_NAME, DMX_LINE_NO,
					prPsrFtr->eType);
			}
		}
		*pfgFull = TRUE;
	}

	MM_RETURN(RET_DMX_OK);
}


MRESULT PSR_Filter_SetESBufSize(PSR_FILTER *prPsrFtr, u32 u4Size)
{
	MRESULT mrRet = RET_DMX_OK;
	s32 i = 0;

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

	if (0 == u4Size)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	if ((SPT_DATA_BUF == prPsrFtr->eType) || (SPT_DATA_GRD == prPsrFtr->eType))
		MM_RETURN(RET_DMX_OK);

	/* set FIFO size */
	DMXLOG_DEBUG(TEXT("[PSR] Call ESM_FifoSetMem start.\r\n"));
	mrRet = ESM_FifoSetMem(prPsrFtr->u4ESIH, u4Size);
	DMXLOG_DEBUG(TEXT("[PSR] Call ESM_FifoSetMem end.\r\n"));
	if (DMX_FAILED(mrRet))
		MM_RETURN(mrRet);

	/* prPsrFtr->u4ESFifoSize = u4Size; */
	/* reget FIFO size from ESM, because ESM will adjust FIFO size for Decoder */
	/* alignment request */
	mrRet = ESM_FifoGetSA(prPsrFtr->u4ESIH, &(prPsrFtr->ptrESFifoSa));
	if (DMX_FAILED(mrRet))
		MM_RETURN(mrRet);

	mrRet = ESM_FifoGetEA(prPsrFtr->u4ESIH, &(prPsrFtr->ptrESFifoEa));
	if (DMX_FAILED(mrRet))
		MM_RETURN(mrRet);

	prPsrFtr->u4ESFifoSize = prPsrFtr->ptrESFifoEa - prPsrFtr->ptrESFifoSa;

	DMXLOG_TRACE(TEXT("[PSR] %s line %d -- ESIH: 0x%x, Type: %s, ")
					TEXT("Fifo(Sa: 0x%x, Ea: 0x%x, Sz: 0x%x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH,
				((prPsrFtr->eType < MAX_SPT_DATA_TYPE_CNT) ?
				 g_aszSptDataTypeName[prPsrFtr->eType] : TEXT("UNKNOWN")),
				prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa, prPsrFtr->u4ESFifoSize);

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_Filter_GetFifoAvailSize(PSR_FILTER *prPsrFtr, u32 *pu4Size)
{
	u32 u4Size = 0;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pu4Size) || (NULL == prPsrFtr))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	*pu4Size = 0;

	mrRet = ESM_FifoGetAvailDataSize(prPsrFtr->u4ESIH, &u4Size);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d (ESIH: 0x%x) fail in ")
						TEXT("ESM_FifoGetAvailDataSize, mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		MM_RETURN(mrRet);
	}

	if (DMX_INVALID_UINT32 == u4Size) {
		DMXLOG_ERROR(
					TEXT("[PSR] %s line %d (ESIH: 0x%x) fail for fifo size == -1\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	*pu4Size = u4Size;

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_AFilter_UpdateESIInfo(PSR_FILTER *prPsrFtr)
{
	MRESULT mrRet = RET_DMX_OK;
	PSR_CC *prPsrCC = NULL;
	uintptr_t ptrHWCurWp = 0;
	uintptr_t ptrESICurWp = 0;
	u64 u8TxSize = 0;
	u32 u4CurAUWrIdx = 0;
	AU_AUDIO *prAudAU = NULL;
	AU_AUDIO_EXT_INFO_T *prAudExtAU = NULL;
	u32 u4TotalAUCnt = 0;
	PSR_AUDFSD *prAFSD = NULL;

	PSR_AU rAU;
#if CONFIG_AUDIO_SUPPORT_CMDQ_MULTI_PTS
	u64 u8Offset;
	u32 u4CurPicAdvLen;
#endif

	DMX_ASSERT(NULL != prPsrFtr);

	mm_memset(&rAU, (u8) 0x00, sizeof(PSR_AU));

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);

	DMX_ASSERT(NULL != prPsrCC);

	prAFSD = (PSR_AUDFSD *) prPsrFtr->pvFilterSpecific;
	DMX_ASSERT(NULL != prAFSD);

	/* get current fifo write pointer */
	mrRet = ESM_FifoGetWrPtr(prPsrFtr->u4ESIH, &ptrESICurWp);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ESM_FifoGetWrPtr")
						TEXT("(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}
	if ((ptrESICurWp >= prPsrFtr->ptrESFifoEa) ||
		(ptrESICurWp < prPsrFtr->ptrESFifoSa)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ptrESICurWp(0x%lx)")
						TEXT(" exceed PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, ptrESICurWp, prPsrFtr->ptrESFifoSa,
					prPsrFtr->ptrESFifoEa);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	/* get HW current write pointer */
	mrRet = PSR_HAL_GetWPtr(prPsrFtr->ucHwDevId, BitType_Audio, &ptrHWCurWp);
	if (DMX_FAILED(mrRet)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}
	if ((ptrHWCurWp >= prPsrFtr->ptrESFifoEa) ||
		(ptrHWCurWp < prPsrFtr->ptrESFifoSa)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ptrHWCurWp(0x%lx)")
						TEXT(" exceed PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, ptrHWCurWp, prPsrFtr->ptrESFifoSa,
					prPsrFtr->ptrESFifoEa);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	/* get picture header detect status */
	if (NULL == prAFSD->prHALStatus) {
		DMX_NewMemory(sizeof(PSR_HDRDET_STATUS_T), prAFSD->prHALStatus);
		if (NULL == prAFSD->prHALStatus)
			MM_RETURN(RET_DMX_NO_MEM);
	}

	/* get tx result */
	dmx_memset((void *) prAFSD->prHALStatus, 0x00, sizeof(PSR_HDRDET_STATUS_T));
	mrRet = PSR_HAL_GetHdrDetResult(prPsrFtr, NULL, prAFSD->prHALStatus);
	if (DMX_FAILED(mrRet)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}
#if DMX_DISABLE_COMP_AUDIOAU
	mrRet = ESM_FifoSetWrPtr(prPsrFtr->u4ESIH, ptrHWCurWp);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ESM_FifoSetWrPtr")
						TEXT("(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		MM_RETURN(mrRet);
	}

	mrRet = ESM_FifoSetRdPtr(prPsrFtr->u4ESIH, ptrHWCurWp, FALSE);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ESM_FifoSetRdPtr")
						TEXT("(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
#endif				/* DMX_DISABLE_COMP_AUDIOAU */

	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
		TEXT("[PSR] %s line %d enter -- ESICurWp(0x%x), HWCurWp(0x%x)")
		 TEXT(", fgAUCtrlByEnd(%d), fgAUEnd(%d), ByLen(%d)\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, ptrESICurWp, ptrHWCurWp,
		(prPsrFtr->fgAUCtrlByEnd ? 1 : 0),
		(prPsrFtr->fgAUEnd ? 1 : 0), (prPsrFtr->fgAUCtrlByLen ? 1 : 0));

#ifndef __linux__
#if 0				/*mtk40144 */
	if (ptrHWCurWp > ptrESICurWp) {
		CacheRangeFlush((void *) ptrESICurWp, (ptrHWCurWp - ptrESICurWp),
			CACHE_SYNC_DISCARD);
	} else if (ptrHWCurWp < ptrESICurWp) {
		CacheRangeFlush((void *) ptrESICurWp,
				(prPsrFtr->ptrESFifoEa - ptrESICurWp), CACHE_SYNC_DISCARD);
		CacheRangeFlush((void *) (prPsrFtr->ptrESFifoSa),
				(ptrHWCurWp - prPsrFtr->ptrESFifoSa), CACHE_SYNC_DISCARD);
	}
#endif
#endif				/* end of #ifndef __linux__ */

	u8TxSize = prPsrCC->u8TxCurrLen;

	if (!((prPsrFtr->fgAUCtrlByEnd) && (prPsrFtr->fgAUEnd)))
		prPsrFtr->ptrPsrHwCurWPtr = ptrHWCurWp;

	/* because each transfer range of subpicture is a unit, we need check tx complete */
	if (prPsrFtr->u4Flag & FF_TX_PBBUF) {
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
			TEXT("[PSR] %s line %d -- (PBBUF) u8TxStartOffset(0x%llx), ")
			 TEXT("u8TxLen(0x%llx), u8TxCurrOffset(0x%llx), ")
			 TEXT("u8CurAULen(0x%llx)\r\n"),
			 DMX_FUNC_NAME,	DMX_LINE_NO, prPsrCC->u8TxStartOffset,
			 prPsrCC->u8TxLen, prPsrCC->u8TxCurrOffset, prPsrFtr->u8CurAULen);

		/* a partial transfer, because fifo size isn't big enough */
		if ((prPsrCC->u8TxStartOffset + prPsrCC->u8TxLen) !=
				(prPsrCC->u8TxCurrOffset + u8TxSize)) {
			if (0 == prPsrFtr->ptrBkWrPtr)
				prPsrFtr->ptrBkWrPtr = ptrESICurWp;

			/* update ESI FIFO write pointer */
			if ((prPsrFtr->fgAUCtrlByEnd) && (prPsrFtr->fgAUEnd)) {
				if ((prPsrFtr->ptrPsrHwCurWPtr >= prPsrFtr->ptrESFifoEa) ||
						(prPsrFtr->ptrPsrHwCurWPtr < prPsrFtr->ptrESFifoSa)) {
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
						TEXT("[PSR] %s line %d fail in prPsrFtr->ptrPsrHwCurWPtr(0x%lx) ")
						 TEXT("exceed PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
						 DMX_FUNC_NAME,	DMX_LINE_NO, prPsrFtr->ptrPsrHwCurWPtr,
						prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_UNEXPECT);
				}
				mrRet = ESM_FifoSetWrPtr(prPsrFtr->u4ESIH,
					prPsrFtr->ptrPsrHwCurWPtr);
			} else {
				mrRet = ESM_FifoSetWrPtr(prPsrFtr->u4ESIH, ptrHWCurWp);
			}

			if (DMX_FAILED(mrRet)) {
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}

			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
				TEXT("[PSR] %s line %d exit -- ptrBkWrPtr(0x%x), ")
				 TEXT("ptrPsrHwCurWPtr(0x%x), CurRngSIdx(%d), CurRngSIdxLen(%d)\r\n"),
				 DMX_FUNC_NAME,	DMX_LINE_NO, prPsrFtr->ptrBkWrPtr,
				 prPsrFtr->ptrPsrHwCurWPtr, prPsrCC->rCmdQTxInf.u2CurTxRngSIdx,
				prPsrCC->rCmdQTxInf.u4CurTxRngSIdxLen);

			MM_RETURN(RET_DMX_OK);
		}
	}

	/* Update ESI FIFO write pointer */
	if ((prPsrFtr->fgAUCtrlByEnd) && (prPsrFtr->fgAUEnd)) {
		if ((prPsrFtr->ptrPsrHwCurWPtr >= prPsrFtr->ptrESFifoEa) ||
				(prPsrFtr->ptrPsrHwCurWPtr < prPsrFtr->ptrESFifoSa)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
				TEXT("[PSR] %s line %d fail in prPsrFtr->ptrPsrHwCurWPtr(0x%lx)")
				TEXT(" exceed PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->ptrPsrHwCurWPtr,
				prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}
		mrRet = ESM_FifoSetWrPtr(prPsrFtr->u4ESIH, prPsrFtr->ptrPsrHwCurWPtr);
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
			TEXT("[PSR] %s line %d -- ESM_FifoSetWrPtr(ptrPsrHwCurWPtr: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->ptrPsrHwCurWPtr);
	} else {
		mrRet = ESM_FifoSetWrPtr(prPsrFtr->u4ESIH, ptrHWCurWp);
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
			TEXT("[PSR] %s line %d -- ESM_FifoSetWrPtr(ptrHWCurWp: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, ptrHWCurWp);
	}

	if (DMX_FAILED(mrRet)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	mrRet = ESM_AUTableGetWrIdx(prPsrFtr->u4ESIH, &u4CurAUWrIdx);
	if (DMX_FAILED(mrRet)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	mrRet = ESM_AUTableGetTotalCount(prPsrFtr->u4ESIH, &u4TotalAUCnt);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
			TEXT("[PSR] %s line %d fail in ESM_AUTableGetTo")
			TEXT("talCount, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (u4CurAUWrIdx >= u4TotalAUCnt) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
			TEXT("[PSR] %s line %d fail for u4CurAUWrIdx")
			TEXT("(%d) > u4TotalAUCnt(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx, u4TotalAUCnt);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4CurAUWrIdx,
		(void **) &prAudAU);
	if (DMX_FAILED(mrRet)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	/*fix klocwork bug */
	if (NULL == prAudAU) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
			TEXT("[PSR] %s line %d fail for prAudAU = NULL!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	mrRet = ESM_AUTableGetAUExtInfo(prPsrFtr->u4ESIH,
		u4CurAUWrIdx, (void **) &prAudExtAU);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
			TEXT("[PSR] %s line %d fail in ESM_AUTableGetAU")
			TEXT("ExtInfo, mrRet: 0x%x!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (prPsrCC->fgUseCmdQ && prPsrCC->fgAUByCmdQEnd) {
		u16 u2Index = prPsrCC->rCmdQTxInf.u2CurTxRngSIdx;
		u32 u4TxLen = 0;
		u32 u4CurPicAdvLen = 0;
		u32 u4BkpCurPicAdvLen = ptrHWCurWp - ptrESICurWp;
		DMX_CMDQ_TX_ENTRY_T *prTxEntry = NULL;
		uintptr_t ptrCurStartWp = ptrESICurWp;
		uintptr_t ptrCurEndWp = ptrESICurWp;

		if (NULL == prPsrCC->pvCmdQTxEntryBuffer) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
				TEXT("[PSR] update AUDIO ESI, cmd q buffer ")
					TEXT("is null, fatal error!!\r\n"));
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		u4CurPicAdvLen = 0;

		if (0 != prPsrFtr->ptrBkWrPtr) {
			ptrCurStartWp = prPsrFtr->ptrBkWrPtr;
		}

		prTxEntry = (DMX_CMDQ_TX_ENTRY_T *)(prPsrCC->pvCmdQTxEntryBuffer) +
			u2Index;

		for (; u2Index <= prPsrCC->rCmdQTxInf.u2CurTxRngEIdx; u2Index++) {
			if (u2Index == prPsrCC->rCmdQTxInf.u2CurTxRngSIdx) {
#if ENABLE_DMX_ADVANCED_VER
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
					TEXT("[PSR] %s line %d -- Idx(%d), CurStartWp(0x%p), CurEndWp")
					TEXT("(0x%p), InsertHdr(%d), SIdxOfst(0x%08x), SIdxLen(0x%x)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u2Index, ptrCurStartWp,
					ptrCurEndWp, ((prTxEntry->fgInsertHdr) ? 1 : 0),
					prPsrCC->rCmdQTxInf.u4CurTxRngSIdxOfst,
					prPsrCC->rCmdQTxInf.u4CurTxRngSIdxLen);
				if ((prTxEntry->fgInsertHdr) &&
						(0 == prPsrCC->rCmdQTxInf.u4CurTxRngSIdxOfst) &&
						(0 < prPsrCC->rCmdQTxInf.u4CurTxRngSIdxLen)) {
					u4TxLen += prTxEntry->u4InsertHdrLen;
					ptrCurEndWp += prTxEntry->u4InsertHdrLen;
					prPsrFtr->u8CurAULen += prTxEntry->u4InsertHdrLen;
				}
#endif				/* ENABLE_DMX_ADVANCED_VER */

				u4TxLen += prPsrCC->rCmdQTxInf.u4CurTxRngSIdxLen;
				ptrCurEndWp += prPsrCC->rCmdQTxInf.u4CurTxRngSIdxLen;
				u4CurPicAdvLen += prPsrCC->rCmdQTxInf.u4CurTxRngSIdxLen;
				prPsrFtr->u8CurAULen += prPsrCC->rCmdQTxInf.u4CurTxRngSIdxLen;
			} else if (u2Index == prPsrCC->rCmdQTxInf.u2CurTxRngEIdx) {
#if ENABLE_DMX_ADVANCED_VER
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
					TEXT("[PSR] %s line %d -- Idx(%d), CurStartWp(0x%p), CurEndWp")
					 TEXT("(0x%p), InsertHdr(%d), EIdxOfst(0x%08x), EIdxLen(0x%x)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u2Index, ptrCurStartWp,
					ptrCurEndWp, ((prTxEntry->fgInsertHdr) ? 1 : 0),
					prPsrCC->rCmdQTxInf.u4CurTxRngEIdxOfst,
					prPsrCC->rCmdQTxInf.u4CurTxRngEIdxLen);

				if ((prTxEntry->fgInsertHdr) &&
						(0 == prPsrCC->rCmdQTxInf.u4CurTxRngEIdxOfst) &&
						(0 < prPsrCC->rCmdQTxInf.u4CurTxRngEIdxLen)) {
					u4TxLen += prTxEntry->u4InsertHdrLen;
					ptrCurEndWp += prTxEntry->u4InsertHdrLen;
					prPsrFtr->u8CurAULen += prTxEntry->u4InsertHdrLen;
				}
#endif				/* ENABLE_DMX_ADVANCED_VER */
				u4TxLen += prPsrCC->rCmdQTxInf.u4CurTxRngEIdxLen;
				ptrCurEndWp += prPsrCC->rCmdQTxInf.u4CurTxRngEIdxLen;
				u4CurPicAdvLen +=
						prPsrCC->rCmdQTxInf.u4CurTxRngEIdxLen + prTxEntry->u4TxOfst;
				prPsrFtr->u8CurAULen += prPsrCC->rCmdQTxInf.u4CurTxRngEIdxLen;
			} else {
#if ENABLE_DMX_ADVANCED_VER
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
					TEXT("[PSR] %s line %d -- Idx(%d), CurStartWp(0x%p), CurEndWp")
					 TEXT("(0x%p), InsertHdr(%d), TxOfst(0x%08x), TxLen(0x%x)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u2Index, ptrCurStartWp,
					ptrCurEndWp, ((prTxEntry->fgInsertHdr) ? 1 : 0),
					prTxEntry->u4TxOfst, prTxEntry->u4TxLen);
				if (prTxEntry->fgInsertHdr) {
					u4TxLen += prTxEntry->u4InsertHdrLen;
					ptrCurEndWp += prTxEntry->u4InsertHdrLen;
					prPsrFtr->u8CurAULen += prTxEntry->u4InsertHdrLen;
				}
#endif				/* ENABLE_DMX_ADVANCED_VER */
				u4TxLen += prTxEntry->u4TxLen;
				ptrCurEndWp += prTxEntry->u4TxLen;
				u4CurPicAdvLen += prTxEntry->u4TxLen + prTxEntry->u4TxOfst;
				prPsrFtr->u8CurAULen += prTxEntry->u4TxLen;
			}

			if (u4TxLen > u4BkpCurPicAdvLen)
				break;

			if (prTxEntry->fgEndAU) {
				mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH,
									 u4CurAUWrIdx, (void **) &prAudAU);
				if (DMX_FAILED(mrRet)) {
					DMX_ASSERT(FALSE);
					MM_RETURN(mrRet);
				}

				/* update AU table item */
				prAudAU->eAuType = AU_DATA;
				/* callback notify Splitter to fill AU information */
				rAU.eType = prPsrFtr->eType;
				rAU.pvAUInf = prAudAU;
				rAU.pvAUExtInf = prAudExtAU;

				/*prAudAU->rAUInfo.rInfo.u8Offset = u8Offset; */

				if (g_rDmxCliMan.fgDumpFlow) {
					DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

					mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
					rOperInfo.pvSptHdl = prPsrCC->pvSptHdl;
					rOperInfo.unFlow.rFillAU.u4AUIdx = u4CurAUWrIdx;
					rOperInfo.unFlow.rFillAU.u4StmType = SPT_DATA_A;
					DmxDumpFlow(DMX_OPER_FILLAU, &rOperInfo);
				}

				PSR_CC_CBSplitter(prPsrCC, E_GET_AU_INFO, (void *) &rAU);

				/* callback ask splitter fill information */
				if (0 == prAudAU->ptrSAddr) {
					prAudAU->ptrSAddr = ptrCurStartWp;
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
						TEXT("[PSR] %s line %d -- ByCmdQ  Set prAudAU(AUIdx: %d)")
						 TEXT("->ptrSAddr = ptrCurStartWp(0x%p)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx,
						ptrCurStartWp);
				}

				if (INVALID_TIMESTAMP != prAudAU->rAUInfo.rInfo.u8Pts) {
					prPsrFtr->u8LastPTS = prAudAU->rAUInfo.rInfo.u8Pts;
#ifdef MM_SUPPORT_DIVXHT31
					prPsrFtr->u8PrevPTS = prPsrFtr->u8LastPTS;
					if (INVALID_TIMESTAMP == prPsrFtr->u81stPTS) {
						DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
							TEXT("[PSR] %s Line %d --- (Audio) set 1stPts to be ")
							TEXT("last PTS: "DMX_PTS_LOGSTR"\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS),
							DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS));
						prPsrFtr->u81stPTS = prPsrFtr->u8LastPTS;
					}
#endif				/* MM_SUPPORT_DIVXHT31 */
				}

				if (prPsrFtr->fgFirstAUInRng) {
					/* Use the flag to designated whether the au is the first au in the */
					/* cfa range, TRUE:FIRST */
					prAudAU->fgSkipData = TRUE;
					prPsrFtr->fgFirstAUInRng = FALSE;
				}

				/*Add End Addr Info */
				PSR_HAL_GetWPtr(prPsrFtr->ucHwDevId, BitType_Audio, &ptrHWCurWp);

				if (ptrCurEndWp >= prPsrFtr->ptrESFifoEa)
					ptrCurEndWp -= prPsrFtr->u4ESFifoSize;

				prAudAU->ptrEAddr = ptrCurEndWp;

				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
					TEXT("[PSR] %s line %d -- ByCmdQ  CreateAU (AUIdx: %d, Sa: ")
					TEXT("0x%x, Ea: 0x%x, Pts: "DMX_PTS_LOGSTR"\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					u4CurAUWrIdx, prAudAU->ptrSAddr, prAudAU->ptrEAddr,
					DMX_PTS_LOG_MS(prAudAU->rAUInfo.rInfo.u8Pts),
					DMX_PTS_LOG_PTS(prAudAU->rAUInfo.rInfo.u8Pts));

				if (g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_AUD]) {
					DmxDumpASample(prAudAU, prPsrFtr->ptrESFifoSa,
									 prPsrFtr->ptrESFifoEa, prPsrFtr->u4StmUID,
									 FALSE);
				}

				/* update ESI AU write index */
				mrRet = ESM_AUTableIncWrIdx(prPsrFtr->u4ESIH, 1);
				if (DMX_FAILED(mrRet))
					MM_RETURN(mrRet);

				mrRet = ESM_AUTableGetWrIdx(prPsrFtr->u4ESIH, &u4CurAUWrIdx);

				if (DMX_FAILED(mrRet))
					MM_RETURN(mrRet);

				mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH,
									 u4CurAUWrIdx, (void **) &prAudAU);
				if (DMX_FAILED(mrRet)) {
					DMX_ASSERT(FALSE);
					MM_RETURN(mrRet);
				}

				ptrCurStartWp = ptrCurEndWp;

				prPsrFtr->ptrBkWrPtr = 0;

			} else {
				mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH,
									 u4CurAUWrIdx, (void **) &prAudAU);
				if (DMX_FAILED(mrRet)) {
					DMX_ASSERT(FALSE);
					MM_RETURN(mrRet);
				}

				if (0 == prAudAU->ptrSAddr) {
					prAudAU->ptrSAddr = ptrCurStartWp;
					prPsrFtr->ptrBkWrPtr = ptrCurStartWp;

					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
						TEXT("[PSR] %s line %d -- ByCmdQ  Set prAudAU(AUIdx: %d)")
						 TEXT("->ptrSAddr = ptrCurStartWp(0x%p)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx,
						ptrCurStartWp);
				}
			}

			prTxEntry++;
		}

		prPsrFtr->fgAUCtrlByLen = FALSE;
		prPsrFtr->u8TotalAULen = 0;
		prPsrFtr->u8CurAULen = 0;

		MM_RETURN(RET_DMX_OK);
	} else if (prPsrFtr->fgAUCtrlByLen) {
		if (0 == prPsrFtr->u8CurAULen) {
			if (0 == prPsrFtr->ptrBkWrPtr) {
				if ((ptrESICurWp >= prPsrFtr->ptrESFifoEa) ||
						(ptrESICurWp < prPsrFtr->ptrESFifoSa)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
						TEXT("[PSR] %s line %d fail in ptrESICurWp")
						 TEXT("(0x%lx) exceed PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, ptrESICurWp,
						prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_UNEXPECT);
				}
				if (NULL == prAudAU) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
						TEXT("[PSR] %s line %d fail for (prAudAU")
						 TEXT(" == NULL), u4CurAUWrIdx(%d)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx);
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_UNEXPECT);
				}

				prAudAU->ptrSAddr = ptrESICurWp;
				prPsrFtr->ptrBkWrPtr = ptrESICurWp;
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
					TEXT("[PSR] %s line %d -- ByLen	u8CurAULen=0, ")
					 TEXT("Set prAudAU->ptrSAddr = u4ESICurWP(0x%x)\r\n"),
					 DMX_FUNC_NAME,	DMX_LINE_NO, ptrESICurWp);
			} else {
				if ((prPsrFtr->ptrBkWrPtr >= prPsrFtr->ptrESFifoEa) ||
						(prPsrFtr->ptrBkWrPtr < prPsrFtr->ptrESFifoSa)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
						TEXT("[PSR] %s line %d fail in prPsrFtr->ptrBkWrPtr(0x%lx) ")
						 TEXT("exceed PsrFtr's Fifo Range[0x%lx,")
						 TEXT(" 0x%lx)\r\n"), DMX_FUNC_NAME,
						DMX_LINE_NO, prPsrFtr->ptrBkWrPtr,
						prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_UNEXPECT);
				}
				prAudAU->ptrSAddr = prPsrFtr->ptrBkWrPtr;
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
					TEXT("[PSR] %s line %d -- ByLen u8CurAULen=0, ")
					 TEXT("Set prAudAU->ptrSAddr = ptrBkWrPtr(0x%x)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->ptrBkWrPtr);
			}
		}

		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
			TEXT("[PSR] %s line %d -- ByLen --> u8CurAULen(0x%llx),")
			 TEXT(" u8TotalAULen(0x%llx)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			prPsrFtr->u8CurAULen, prPsrFtr->u8TotalAULen);

		prPsrFtr->u8CurAULen += prPsrCC->u8TxLen;

		if (prPsrFtr->u8CurAULen >= prPsrFtr->u8TotalAULen) {
			if (prPsrFtr->u8CurAULen > prPsrFtr->u8TotalAULen) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
					TEXT("[PSR] %s line %d fail for CurAULen("DMX_UINT64_10U_LOGSTR")")
						TEXT("> TotalAULen("DMX_UINT64_10U_LOGSTR"), lasttxlen(")
						TEXT(DMX_UINT64_10U_LOGSTR")\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					DMX_UINT64_10U_LOG(prPsrFtr->u8CurAULen),
					DMX_UINT64_10U_LOG(prPsrFtr->u8TotalAULen),
					DMX_UINT64_10U_LOG(prPsrCC->u8TxLen));
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			if (NULL == prAudAU) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
					TEXT("[PSR] %s line %d fail for (prAudAU== NULL), ")
					TEXT("u4CurAUWrIdx(%d)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			/* update AU table item */
			prAudAU->eAuType = AU_DATA;
			/* callback notify Splitter to fill AU information */
			rAU.eType = prPsrFtr->eType;
			rAU.pvAUInf = prAudAU;
			rAU.pvAUExtInf = prAudExtAU;
			/* callback ask splitter fill information */

			if (g_rDmxCliMan.fgDumpFlow) {
				DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

				mm_memset(&rOperInfo, (u8) 0x00,
						sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
				rOperInfo.pvSptHdl = prPsrCC->pvSptHdl;
				rOperInfo.unFlow.rFillAU.u4AUIdx = u4CurAUWrIdx;
				rOperInfo.unFlow.rFillAU.u4StmType = SPT_DATA_A;
				DmxDumpFlow(DMX_OPER_FILLAU, &rOperInfo);
			}
#if !DMX_DISABLE_FILL_AUINFO
			PSR_CC_CBSplitter(prPsrCC, E_GET_AU_INFO, (void *) &rAU);
#endif

			/*Add End Addr Info */
			PSR_HAL_GetWPtr(prPsrFtr->ucHwDevId, BitType_Audio, &ptrHWCurWp);

			if ((ptrHWCurWp >= prPsrFtr->ptrESFifoEa) ||
					(ptrHWCurWp < prPsrFtr->ptrESFifoSa)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
					TEXT("[PSR] %s line %d fail in ptrHWCurWp(0x%lx) ")
					TEXT("exceed PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, ptrHWCurWp,
					prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}
			prAudAU->ptrEAddr = ptrHWCurWp;

			if (INVALID_TIMESTAMP != prAudAU->rAUInfo.rInfo.u8Pts) {
				prPsrFtr->u8LastPTS = prAudAU->rAUInfo.rInfo.u8Pts;
#ifdef MM_SUPPORT_DIVXHT31
				prPsrFtr->u8PrevPTS = prAudAU->rAUInfo.rInfo.u8Pts;
				if (INVALID_TIMESTAMP == prPsrFtr->u81stPTS) {
					DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
						TEXT("[PSR] %s Line %d --- (Audio) set 1stPts to be ")
						 TEXT(DMX_PTS_LOGSTR"\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS),
						DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS));
					prPsrFtr->u81stPTS = prPsrFtr->u8LastPTS;
				}
#endif				/*	MM_SUPPORT_DIVXHT31 */
			}

			if (prPsrFtr->fgFirstAUInRng) {
				/* Use the flag to designated whether the au is the first au in the cfa */
				/* range, TRUE:FIRST */
				prAudAU->fgSkipData = TRUE;
				prPsrFtr->fgFirstAUInRng = FALSE;
			}

			if (g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_AUD]) {
				DmxDumpASample(prAudAU, prPsrFtr->ptrESFifoSa,
								 prPsrFtr->ptrESFifoEa, prPsrFtr->u4StmUID, FALSE);
			}

			prPsrFtr->ptrBkWrPtr = 0;

			mrRet = ESM_AUTableIncWrIdx(prPsrFtr->u4ESIH, 1);
			if (DMX_FAILED(mrRet)) {
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}
			prPsrFtr->fgAUCtrlByLen = FALSE;
			prPsrFtr->u8TotalAULen = 0;
			prPsrFtr->u8CurAULen = 0;
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
				TEXT("[PSR] %s line %d -- ByLen --> CreateAU(WrIdx: %d), SA: 0x%x,")
				 TEXT(" EA: 0x%x\r\n"), DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx,
				prAudAU->ptrSAddr, prAudAU->ptrEAddr);
		}

		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
			TEXT("[PSR] %s line %d -- ByLen exit, CurAU(WrIdx: %d), ")
			TEXT("SA: 0x%x, EA: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx, prAudAU->ptrSAddr,
			prAudAU->ptrEAddr);

		MM_RETURN(RET_DMX_OK);
	} else if (prPsrFtr->fgAUCtrlByEnd) {
		/* SACD DST type, it's 1st AU unit */
		if (0 == prPsrFtr->u8CurAULen) {
			/* Keep AU start address for turn back to write info */
			if (0 == prPsrFtr->ptrBkWrPtr) {
				if ((ptrESICurWp >= prPsrFtr->ptrESFifoEa) ||
						(ptrESICurWp < prPsrFtr->ptrESFifoSa)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
						TEXT("[PSR] %s line %d fail in ptrESICurWp")
						 TEXT("(0x%lx) exceed PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, ptrESICurWp,
						prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_UNEXPECT);
				}
				prAudAU->ptrSAddr = ptrESICurWp;
				prPsrFtr->ptrBkWrPtr = ptrESICurWp;
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
					TEXT("[PSR] %s line %d -- ByEnd	u8CurAULen=0, ")
					 TEXT("Set prAudAU->ptrSAddr = u4ESICurWP(0x%x)\r\n"),
					 DMX_FUNC_NAME, DMX_LINE_NO, ptrESICurWp);
			} else {
				if ((prPsrFtr->ptrBkWrPtr >= prPsrFtr->ptrESFifoEa) ||
						(prPsrFtr->ptrBkWrPtr < prPsrFtr->ptrESFifoSa)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
						TEXT("[PSR] %s line %d fail in prPsrFtr->ptrBkWrPtr(0x%lx) ")
						 TEXT("exceed PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->ptrBkWrPtr,
						prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_UNEXPECT);
				}
				prAudAU->ptrSAddr = prPsrFtr->ptrBkWrPtr;
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
					TEXT("[PSR] %s line %d -- ByEnd u8CurAULen=0, ")
					 TEXT("Set prAudAU->ptrSAddr = ptrBkWrPtr(0x%x)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->ptrBkWrPtr);
			}
		}

		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
			TEXT("[PSR] %s line %d -- ByEnd --> u8CurAULen(0x%llx), ")
			 TEXT("u8TotalAULen(0x%llx)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			prPsrFtr->u8CurAULen, prPsrFtr->u8TotalAULen);

		/* accumulate AU transfer length */
		prPsrFtr->u8CurAULen += prPsrCC->u8TxLen;

		/* End notify (virtual tx) */
		if (prPsrFtr->fgAUEnd) {
			if (NULL == prAudAU) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
					TEXT("[PSR] %s line %d fail for (prAudAU== NULL), ")
					TEXT(" u4CurAUWrIdx(%d)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			/* update AU table item */
			prAudAU->eAuType = AU_DATA;
			/* callback notify Splitter to fill AU information */
			rAU.eType = prPsrFtr->eType;
			rAU.pvAUInf = prAudAU;
			rAU.pvAUExtInf = prAudExtAU;
			/* callback ask splitter fill information */
			if (g_rDmxCliMan.fgDumpFlow) {
				DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

				mm_memset(&rOperInfo, (u8) 0x00,
						sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
				rOperInfo.pvSptHdl = prPsrCC->pvSptHdl;
				rOperInfo.unFlow.rFillAU.u4AUIdx = u4CurAUWrIdx;
				rOperInfo.unFlow.rFillAU.u4StmType = SPT_DATA_A;
				DmxDumpFlow(DMX_OPER_FILLAU, &rOperInfo);
			}

			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
				TEXT("[PSR] %s line %d -- PSR_CC_CBSplitter(E_GET_AU_INFO)!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);

#if !DMX_DISABLE_FILL_AUINFO
			PSR_CC_CBSplitter(prPsrCC, E_GET_AU_INFO, (void *) &rAU);
#endif

			/*Add End Addr Info */
			/*PSR_HAL_GetWPtr(BitType_Audio, &ptrHWCurWp); */
			ptrHWCurWp = prPsrFtr->ptrPsrHwCurWPtr;
			if ((ptrHWCurWp >= prPsrFtr->ptrESFifoEa) ||
					(ptrHWCurWp < prPsrFtr->ptrESFifoSa)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
					TEXT("[PSR] %s line %d fail in ptrHWCurWp(0x%lx) ")
					TEXT("exceed PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, ptrHWCurWp,
					prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			prAudAU->ptrEAddr = ptrHWCurWp;

			prAudAU->rAUInfo.rInfo.eAudType = AUD_SACD;

			if (INVALID_TIMESTAMP != prAudAU->rAUInfo.rInfo.u8Pts) {
				prPsrFtr->u8LastPTS = prAudAU->rAUInfo.rInfo.u8Pts;
#ifdef MM_SUPPORT_DIVXHT31
				prPsrFtr->u8PrevPTS = prAudAU->rAUInfo.rInfo.u8Pts;
				if (INVALID_TIMESTAMP == prPsrFtr->u81stPTS) {
					DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
								TEXT("[PSR] %s Line %d --- (Audio) ")
								 TEXT("set 1stPts to be " DMX_PTS_LOGSTR "\r\n"),
								DMX_FUNC_NAME, DMX_LINE_NO,
								DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS),
								DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS));
					prPsrFtr->u81stPTS = prPsrFtr->u8LastPTS;
				}
#endif				/*	MM_SUPPORT_DIVXHT31 */
			}

			/* SACD AU end */

			prPsrFtr->fgAUCtrlByEnd = FALSE;
			prPsrFtr->fgAUEnd = FALSE;
			prPsrFtr->u8CurAULen = 0;

			if (INVALID_TIMESTAMP != prAudAU->rAUInfo.rInfo.u8Pts) {
				prPsrFtr->u8LastPTS = prAudAU->rAUInfo.rInfo.u8Pts;
#ifdef MM_SUPPORT_DIVXHT31
				prPsrFtr->u8PrevPTS = prAudAU->rAUInfo.rInfo.u8Pts;
				if (INVALID_TIMESTAMP == prPsrFtr->u81stPTS) {
					DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
						TEXT("[PSR] %s Line %d --- (Audio) ")
						 TEXT("set 1stPts to be "DMX_PTS_LOGSTR"\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS),
						DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS));
					prPsrFtr->u81stPTS = prPsrFtr->u8LastPTS;
				}
#endif				/* MM_SUPPORT_DIVXHT31 */
			}

			if (prPsrFtr->fgFirstAUInRng) {
				/* Use the flag to designated whether the au is the first au in the cfa */
				/* range, TRUE:FIRST */
				prAudAU->fgSkipData = TRUE;
				prPsrFtr->fgFirstAUInRng = FALSE;
			}

			if (g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_AUD]) {
				DmxDumpASample(prAudAU, prPsrFtr->ptrESFifoSa,
								 prPsrFtr->ptrESFifoEa, prPsrFtr->u4StmUID, FALSE);
			}

			prPsrFtr->ptrBkWrPtr = 0;

			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
				TEXT("[PSR] %s line %d -- ByEnd -->CreateAU(WrIdx: %d), ")
				TEXT("SA: 0x%x, EA: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				u4CurAUWrIdx, prAudAU->ptrSAddr, prAudAU->ptrEAddr);

			/* Move to next AU */
			mrRet = ESM_AUTableIncWrIdx(prPsrFtr->u4ESIH, 1);
			if (DMX_FAILED(mrRet)) {
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}

			prPsrCC->fgUseCmdQ = FALSE;
			prPsrCC->fgAUByCmdQEnd = FALSE;

			/* change state to init and tx state to tx ok */
			prPsrCC->eState = CCS_INIT;
			PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);

			PSR_CC_CBSplitter(prPsrCC, E_TX_DONE, NULL);
		}

		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
			TEXT("[PSR] %s line %d -- ByEnd exit, CurAU(WrIdx: %d), ")
			TEXT("SA: 0x%x, EA: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			u4CurAUWrIdx, prAudAU->ptrSAddr, prAudAU->ptrEAddr);

		MM_RETURN(RET_DMX_OK);
	} else {
		if (!prPsrCC->fgTxMem2Fifo) {
			if (NULL == prAudAU) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
					TEXT("[PSR] %s line %d fail for (prAudAU == NULL), ")
					TEXT(" u4CurAUWrIdx(%d)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			/* update AU table item */
			prAudAU->eAuType = AU_DATA;
			/* callback notify Splitter to fill AU information */
			rAU.eType = prPsrFtr->eType;
			rAU.pvAUInf = prAudAU;
			rAU.pvAUExtInf = prAudExtAU;

			/* callback ask splitter fill information */
			if (g_rDmxCliMan.fgDumpFlow) {
				DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

				mm_memset(&rOperInfo, (u8) 0x00,
						sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
				rOperInfo.pvSptHdl = prPsrCC->pvSptHdl;
				rOperInfo.unFlow.rFillAU.u4AUIdx = u4CurAUWrIdx;
				rOperInfo.unFlow.rFillAU.u4StmType = SPT_DATA_A;
				DmxDumpFlow(DMX_OPER_FILLAU, &rOperInfo);
			}
#if !DMX_DISABLE_FILL_AUINFO
			PSR_CC_CBSplitter(prPsrCC, E_GET_AU_INFO, (void *) &rAU);
#endif

			if (0 == prPsrFtr->ptrBkWrPtr) {
				if ((ptrESICurWp >= prPsrFtr->ptrESFifoEa) ||
					(ptrESICurWp < prPsrFtr->ptrESFifoSa)) {	/*CFA parsing wrong */
					DMX_ASSERT(FALSE);
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
						TEXT("[PSR] %s line %d fail for ptrESICurWp(0x%x) ")
						TEXT("exceed limit, FifoSa(0x%x), FifoEa(0x%x)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, ptrESICurWp,
						prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
					MM_RETURN(RET_DMX_OVER_LIMIT);
				}

				prAudAU->ptrSAddr = ptrESICurWp;
				prPsrFtr->ptrBkWrPtr = ptrESICurWp;
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
					TEXT("[PSR] %s line %d -- fgTxMem2Fifo(FALSE) ")
					 TEXT("Set prAudAU->ptrSAddr = u4ESICurWP(0x%x)\r\n"),
					 DMX_FUNC_NAME, DMX_LINE_NO, ptrESICurWp);
			} else {
				if ((prPsrFtr->ptrBkWrPtr >= prPsrFtr->ptrESFifoEa) ||
					(prPsrFtr->ptrBkWrPtr < prPsrFtr->ptrESFifoSa)) {	/*CFA parsing wrong */
					DMX_ASSERT(FALSE);
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
						TEXT("[PSR] %s line %d fail for prPsrFtr->ptrBkWrPtr(0x%x) ")
						TEXT("exceed limit, FifoSa(0x%x), FifoEa(0x%x)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						prPsrFtr->ptrBkWrPtr, prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
					MM_RETURN(RET_DMX_OVER_LIMIT);
				}
				prAudAU->ptrSAddr = prPsrFtr->ptrBkWrPtr;
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
					TEXT("[PSR] %s line %d -- fgTxMem2Fifo(FALSE) ")
					TEXT("Set prAudAU->ptrSAddr = ptrBkWrPtr(0x%x)\r\n"),
					DMX_FUNC_NAME,	DMX_LINE_NO, prPsrFtr->ptrBkWrPtr);
			}

			/*Add End Addr Info */
			PSR_HAL_GetWPtr(prPsrFtr->ucHwDevId, BitType_Audio, &ptrHWCurWp);
			if ((ptrHWCurWp >= prPsrFtr->ptrESFifoEa) ||
				(ptrHWCurWp < prPsrFtr->ptrESFifoSa)) {	/*CFA parsing wrong */
				DMX_ASSERT(FALSE);
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
					TEXT("[PSR] %s line %d fail for ptrHWCurWp(0x%x) ")
					TEXT("exceed limit, FifoSa(0x%x), FifoEa(0x%x)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, ptrHWCurWp,
					prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
				MM_RETURN(RET_DMX_OVER_LIMIT);
			}
			prAudAU->ptrEAddr = ptrHWCurWp;

			if (INVALID_TIMESTAMP != prAudAU->rAUInfo.rInfo.u8Pts) {
				prPsrFtr->u8LastPTS = prAudAU->rAUInfo.rInfo.u8Pts;
#ifdef MM_SUPPORT_DIVXHT31
				prPsrFtr->u8PrevPTS = prAudAU->rAUInfo.rInfo.u8Pts;
				if (INVALID_TIMESTAMP == prPsrFtr->u81stPTS) {
					DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
						TEXT("[PSR] %s Line %d --- (Audio) ")
						TEXT("set 1stPts to be "DMX_PTS_LOGSTR"\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS),
						DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS));
					prPsrFtr->u81stPTS = prPsrFtr->u8LastPTS;
				}
#endif				/* MM_SUPPORT_DIVXHT31 */
			}

			if (prPsrFtr->fgFirstAUInRng) {
				/* Use the flag to designated whether the au is the first au in the cfa */
				/* range, TRUE:FIRST */
				prAudAU->fgSkipData = TRUE;
				prPsrFtr->fgFirstAUInRng = FALSE;
			}

			if (g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_AUD]) {
				DmxDumpASample(prAudAU, prPsrFtr->ptrESFifoSa,
						 prPsrFtr->ptrESFifoEa, prPsrFtr->u4StmUID, FALSE);
			}

			prPsrFtr->ptrBkWrPtr = 0;

			/* update ESI AU write index */
			mrRet = ESM_AUTableIncWrIdx(prPsrFtr->u4ESIH, 1);
			if (DMX_FAILED(mrRet))
				MM_RETURN(mrRet);

			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
				TEXT("[PSR] %s line %d -- fgTxMem2Fifo(FALSE) exit, ")
				TEXT("CurAU(WrIdx: %d), SA: 0x%x, EA: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx,
				prAudAU->ptrSAddr, prAudAU->ptrEAddr);

			MM_RETURN(RET_DMX_OK);
		} else {
			if (NULL == prAudAU) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
					TEXT("[PSR] %s line %d fail for (prAudAU == NULL), ")
					TEXT("u4CurAUWrIdx(%d)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			if (0 == prPsrFtr->ptrBkWrPtr) {
				if ((ptrESICurWp >= prPsrFtr->ptrESFifoEa) ||
					(ptrESICurWp < prPsrFtr->ptrESFifoSa)) {	/*CFA parsing wrong */
					DMX_ASSERT(FALSE);
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
						TEXT("[PSR] %s line %d fail for ptrESICurWp(0x%x) ")
						TEXT("exceed limit, FifoSa(0x%x), FifoEa(0x%x)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, ptrESICurWp,
						prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
					MM_RETURN(RET_DMX_OVER_LIMIT);
				}

				prAudAU->ptrSAddr = ptrESICurWp;
				prPsrFtr->ptrBkWrPtr = ptrESICurWp;
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
					TEXT("[PSR] %s line %d -- fgTxMem2Fifo	Set ")
					TEXT("prAudAU->ptrSAddr = u4ESICurWP(0x%x)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, ptrESICurWp);
			} else {
				prAudAU->ptrSAddr = prPsrFtr->ptrBkWrPtr;
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
					TEXT("[PSR] %s line %d -- fgTxMem2Fifo	Set ")
					TEXT("prAudAU->ptrSAddr = ptrBkWrPtr(0x%x)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->ptrBkWrPtr);
			}

			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
				TEXT("[PSR] %s line %d -- fgTxMem2Fifo exit, ")
				TEXT("CurAU(WrIdx: %d), SA: 0x%x, EA: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx,
				prAudAU->ptrSAddr, prAudAU->ptrEAddr);

			MM_RETURN(RET_DMX_OK);
		}
	}

	/*reset backup write pointer */
	prPsrFtr->ptrBkWrPtr = 0;

	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_A,
		TEXT("[PSR] %s line %d exit\r\n"), DMX_FUNC_NAME, DMX_LINE_NO);


	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_SectionFilter_UpdateESIInfo(PSR_FILTER *prPsrFtr)
{
	MRESULT mrRet = RET_DMX_OK;
	PSR_CC *prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);
	uintptr_t ptrHWCurWp;
	uintptr_t ptrESICurWp;
	u64 u8TxSize;
	u32 u4CurAUWrIdx;
	AU_SECTION *prSectionAU = NULL;
	PSR_NORMALFSD *prSECFSD = NULL;
	PSR_AU rAU;

	mm_memset(&rAU, (u8) 0x00, sizeof(PSR_AU));

	/* get current fifo write pointer */
	mrRet = ESM_FifoGetWrPtr(prPsrFtr->u4ESIH, &ptrESICurWp);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ESM_FifoGetWrPtr")
						TEXT("(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}
	/* get HW current write pointer */
	mrRet = PSR_HAL_GetWPtr(prPsrFtr->ucHwDevId, BitType_Section, &ptrHWCurWp);
	if (DMX_FAILED(mrRet)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	prSECFSD = (PSR_NORMALFSD *) prPsrFtr->pvFilterSpecific;
	DMX_ASSERT(NULL != prSECFSD);

	/* get PES header detect status */
	if (NULL == prSECFSD->prHALStatus) {
		DMX_NewMemory(sizeof(PSR_HDRDET_STATUS_T), prSECFSD->prHALStatus);
		if (NULL == prSECFSD->prHALStatus)
			MM_RETURN(RET_DMX_NO_MEM);
	}

	/* get tx result */
	dmx_memset((void *) prSECFSD->prHALStatus, 0x00, sizeof(PSR_HDRDET_STATUS_T));
	mrRet = PSR_HAL_GetHdrDetResult(prPsrFtr, NULL, prSECFSD->prHALStatus);
	if (DMX_FAILED(mrRet)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}
#if DMX_DISABLE_COMP_OTHAU
	mrRet = ESM_FifoSetWrPtr(prPsrFtr->u4ESIH, ptrHWCurWp);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ESM_FifoSetWrPtr")
						TEXT("(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		MM_RETURN(mrRet);
	}

	mrRet = ESM_FifoSetRdPtr(prPsrFtr->u4ESIH, ptrHWCurWp, FALSE);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ESM_FifoSetRdPtr")
						TEXT("(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
#endif				/* DMX_DISABLE_COMP_OTHAU */

	u8TxSize = prPsrCC->u8TxCurrLen;

	if (!(prPsrFtr->fgAUCtrlByEnd && prPsrFtr->fgAUEnd))
		prPsrFtr->ptrPsrHwCurWPtr = ptrHWCurWp;

	/* because each transfer range of subpicture is a unit, we need check tx complete */
	if (prPsrFtr->u4Flag & FF_TX_PBBUF) {
		/* a partial transfer, because fifo size isn't big enough */
		if ((prPsrCC->u8TxStartOffset + prPsrCC->u8TxLen) !=
				(prPsrCC->u8TxCurrOffset + u8TxSize)) {
			if (0 == prPsrFtr->ptrBkWrPtr)
				prPsrFtr->ptrBkWrPtr = ptrESICurWp;

			/* update ESI FIFO write pointer */
			if ((prPsrFtr->fgAUCtrlByEnd) && (prPsrFtr->fgAUEnd))
				mrRet =
						ESM_FifoSetWrPtr(prPsrFtr->u4ESIH, prPsrFtr->ptrPsrHwCurWPtr);
			else
				mrRet = ESM_FifoSetWrPtr(prPsrFtr->u4ESIH, ptrHWCurWp);

			if (DMX_FAILED(mrRet)) {
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}

			MM_RETURN(RET_DMX_OK);
		}
	}

	/* Update ESI FIFO write pointer */
	if (prPsrFtr->fgAUCtrlByEnd && prPsrFtr->fgAUEnd)
		mrRet = ESM_FifoSetWrPtr(prPsrFtr->u4ESIH, prPsrFtr->ptrPsrHwCurWPtr);
	else
		mrRet = ESM_FifoSetWrPtr(prPsrFtr->u4ESIH, ptrHWCurWp);

	if (DMX_FAILED(mrRet)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	mrRet = ESM_AUTableGetWrIdx(prPsrFtr->u4ESIH, &u4CurAUWrIdx);
	if (DMX_FAILED(mrRet)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4CurAUWrIdx, (void **) &prSectionAU);
	if (DMX_FAILED(mrRet)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}
	/*fix klocwork bug */
	if (NULL == prSectionAU) {
		DMXLOG_ERROR(TEXT("[PSR] prSectionAU = NULL Failed!\r\n"));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (prPsrFtr->fgAUCtrlByLen) {
		if (0 == prPsrFtr->u8CurAULen) {
			if (0 == prPsrFtr->ptrBkWrPtr)
				prSectionAU->ptrSAddr = ptrESICurWp;
			else
				prSectionAU->ptrSAddr = prPsrFtr->ptrBkWrPtr;
		}
		prPsrFtr->u8CurAULen += prPsrCC->u8TxLen;
		if (prPsrFtr->u8CurAULen >= prPsrFtr->u8TotalAULen) {
			if (prPsrFtr->u8CurAULen > prPsrFtr->u8TotalAULen) {
				DMX_ASSERT(FALSE);
#ifdef __linux__
				DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for CurAULen")
								TEXT
								("(%lld) > TotalAULen(%lld), lasttxlen(%lld)\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u8CurAULen,
							prPsrFtr->u8TotalAULen, prPsrCC->u8TxLen);
#else
				DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for CurAULen")
								TEXT
								("(%I64d) > TotalAULen(%I64d), lasttxlen(%I64d)\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u8CurAULen,
							prPsrFtr->u8TotalAULen, prPsrCC->u8TxLen);
#endif				/* #ifdef __linux_ */
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			/* update AU table item */
			prSectionAU->eAuType = AU_DATA;
			/* callback notify Splitter to fill AU information */
			rAU.eType = prPsrFtr->eType;
			rAU.pvAUInf = prSectionAU;
			rAU.pvAUExtInf = NULL;

			/* callback ask splitter fill information */
			if (g_rDmxCliMan.fgDumpFlow) {
				DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

				mm_memset(&rOperInfo, (u8) 0x00,
						sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
				rOperInfo.pvSptHdl = prPsrCC->pvSptHdl;
				rOperInfo.unFlow.rFillAU.u4AUIdx = u4CurAUWrIdx;
				rOperInfo.unFlow.rFillAU.u4StmType = SPT_DATA_SECTION;
				DmxDumpFlow(DMX_OPER_FILLAU, &rOperInfo);
			}
#if !DMX_DISABLE_FILL_AUINFO
			PSR_CC_CBSplitter(prPsrCC, E_GET_AU_INFO, (void *) &rAU);
#endif

			/*Add End Addr Info */
			PSR_HAL_GetWPtr(prPsrFtr->ucHwDevId, BitType_Section, &ptrHWCurWp);
			prSectionAU->ptrEAddr = ptrHWCurWp;
			prSectionAU->u4Size = (u32) (prPsrCC->u8TxLen);

			mrRet = ESM_AUTableIncWrIdx(prPsrFtr->u4ESIH, 1);
			if (DMX_FAILED(mrRet)) {
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}
			prPsrFtr->fgAUCtrlByLen = FALSE;
			prPsrFtr->u8TotalAULen = 0;
			prPsrFtr->u8CurAULen = 0;
		}
	} else if (prPsrFtr->fgAUCtrlByEnd) {
		/* SACD DST type, it's 1st AU unit */

		if (0 == prPsrFtr->u8CurAULen) {
			/* Keep AU start address for turn back to write info */
			if (0 == prPsrFtr->ptrBkWrPtr)
				prSectionAU->ptrSAddr = ptrESICurWp;
			else
				prSectionAU->ptrSAddr = prPsrFtr->ptrBkWrPtr;
		}

		/* accumulate AU transfer length */
		prPsrFtr->u8CurAULen += prPsrCC->u8TxLen;

		/* End notify (virtual tx) */
		if (prPsrFtr->fgAUEnd) {
			/* update AU table item */
			prSectionAU->eAuType = AU_DATA;
			/* callback notify Splitter to fill AU information */
			rAU.eType = prPsrFtr->eType;
			rAU.pvAUInf = prSectionAU;
			rAU.pvAUExtInf = NULL;

			/* callback ask splitter fill information */
			if (g_rDmxCliMan.fgDumpFlow) {
				DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

				mm_memset(&rOperInfo, (u8) 0x00,
						sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
				rOperInfo.pvSptHdl = prPsrCC->pvSptHdl;
				rOperInfo.unFlow.rFillAU.u4AUIdx = u4CurAUWrIdx;
				rOperInfo.unFlow.rFillAU.u4StmType = SPT_DATA_SECTION;
				DmxDumpFlow(DMX_OPER_FILLAU, &rOperInfo);
			}
#if !DMX_DISABLE_FILL_AUINFO
			PSR_CC_CBSplitter(prPsrCC, E_GET_AU_INFO, (void *) &rAU);
#endif

			/*Add End Addr Info */
			ptrHWCurWp = prPsrFtr->ptrPsrHwCurWPtr;
			prSectionAU->ptrEAddr = ptrHWCurWp;
			prSectionAU->u4Size = (u32) (prPsrCC->u8TxLen);

			prPsrFtr->fgAUCtrlByEnd = FALSE;
			prPsrFtr->fgAUEnd = FALSE;
			prPsrFtr->u8CurAULen = 0;

			prPsrCC->fgUseCmdQ = FALSE;
			prPsrCC->fgAUByCmdQEnd = FALSE;

			/* change state to init and tx state to tx ok */
			prPsrCC->eState = CCS_INIT;
			PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);

			PSR_CC_CBSplitter(prPsrCC, E_TX_DONE, NULL);

			/* Move to next AU */
			mrRet = ESM_AUTableIncWrIdx(prPsrFtr->u4ESIH, 1);
			if (DMX_FAILED(mrRet)) {
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}
		}
	} else {
		if (!prPsrCC->fgTxMem2Fifo) {
			/* update AU table item */
			prSectionAU->eAuType = AU_DATA;
			/* callback notify Splitter to fill AU information */
			rAU.eType = prPsrFtr->eType;
			rAU.pvAUInf = prSectionAU;
			rAU.pvAUExtInf = NULL;

			/* callback ask splitter fill information */
			if (g_rDmxCliMan.fgDumpFlow) {
				DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

				mm_memset(&rOperInfo, (u8) 0x00,
						sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
				rOperInfo.pvSptHdl = prPsrCC->pvSptHdl;
				rOperInfo.unFlow.rFillAU.u4AUIdx = u4CurAUWrIdx;
				rOperInfo.unFlow.rFillAU.u4StmType = SPT_DATA_SECTION;
				DmxDumpFlow(DMX_OPER_FILLAU, &rOperInfo);
			}
#if !DMX_DISABLE_FILL_AUINFO
			PSR_CC_CBSplitter(prPsrCC, E_GET_AU_INFO, (void *) &rAU);
#endif

			if (0 == prPsrFtr->ptrBkWrPtr)
				prSectionAU->ptrSAddr = ptrESICurWp;
			else
				prSectionAU->ptrSAddr = prPsrFtr->ptrBkWrPtr;

			/*Add End Addr Info */
			PSR_HAL_GetWPtr(prPsrFtr->ucHwDevId, BitType_Section, &ptrHWCurWp);
			prSectionAU->ptrEAddr = ptrHWCurWp;
			prSectionAU->u4Size = (u32) (prPsrCC->u8TxLen);

			/* update ESI AU write index */
			mrRet = ESM_AUTableIncWrIdx(prPsrFtr->u4ESIH, 1);
			if (DMX_FAILED(mrRet))
				MM_RETURN(mrRet);
		}
	}

	/*reset backup write pointer */
	prPsrFtr->ptrBkWrPtr = 0;
	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_DMA_UpdateESIInfo(PSR_FILTER *prPsrFtr)
{
	PSR_DMASD *prDMASD = NULL;
	PSR_CC *prPsrCC = NULL;

	DMX_ASSERT(NULL != prPsrFtr);

	prDMASD = (PSR_DMASD *) prPsrFtr->pvFilterSpecific;
	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);
	DMX_ASSERT(NULL != prDMASD);
	DMX_ASSERT(NULL != prPsrCC);

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_SPFilter_UpdateESIInfo(PSR_FILTER *prPsrFtr)
{
	MRESULT mrRet = RET_DMX_OK;
	u32 u4CurAUWrIdx;
	AU_SP *prSPAU = NULL;
	PSR_CC *prPsrCC = NULL;
	PSR_AU rAU;
	uintptr_t ptrESICurWp, ptrHWCurWp;
	u64 u8TxSize;
	PSR_NORMALFSD *prSPFSD = NULL;

	DMX_ASSERT(NULL != prPsrFtr);

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);

	mm_memset(&rAU, (u8) 0x00, sizeof(PSR_AU));

	/* get current fifo write pointer */
	mrRet = ESM_FifoGetWrPtr(prPsrFtr->u4ESIH, &ptrESICurWp);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_SP,
			TEXT("[PSR] %s line %d fail in ESM_FifoGetWrPtr")
			TEXT("(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}
	if ((ptrESICurWp >= prPsrFtr->ptrESFifoEa) ||
		(ptrESICurWp < prPsrFtr->ptrESFifoSa)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_SP,
			TEXT("[PSR] %s line %d fail in ptrESICurWp(0x%lx)")
			TEXT(" exceed PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, ptrESICurWp, prPsrFtr->ptrESFifoSa,
			prPsrFtr->ptrESFifoEa);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}


	/* get HW current write pointer */
	mrRet = PSR_HAL_GetWPtr(prPsrFtr->ucHwDevId, BitType_SubPic0, &ptrHWCurWp);
	if (DMX_FAILED(mrRet)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if ((ptrHWCurWp >= prPsrFtr->ptrESFifoEa) ||
		(ptrHWCurWp < prPsrFtr->ptrESFifoSa)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_SP,
			TEXT("[PSR] %s line %d fail in ptrHWCurWp(0x%lx)")
			TEXT(" exceed PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, ptrHWCurWp, prPsrFtr->ptrESFifoSa,
			prPsrFtr->ptrESFifoEa);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	prSPFSD = (PSR_NORMALFSD *) prPsrFtr->pvFilterSpecific;
	DMX_ASSERT(NULL != prSPFSD);

	/* get PES header detect status */
	if (NULL == prSPFSD->prHALStatus) {
		DMX_NewMemory(sizeof(PSR_HDRDET_STATUS_T), prSPFSD->prHALStatus);
		if (NULL == prSPFSD->prHALStatus)
			MM_RETURN(RET_DMX_NO_MEM);
	}
	/* get tx result */
	dmx_memset((void *) prSPFSD->prHALStatus, 0x00, sizeof(PSR_HDRDET_STATUS_T));
	mrRet = PSR_HAL_GetHdrDetResult(prPsrFtr, NULL, prSPFSD->prHALStatus);
	if (DMX_FAILED(mrRet)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}
#if DMX_DISABLE_COMP_SPAU
	mrRet = ESM_FifoSetWrPtr(prPsrFtr->u4ESIH, ptrHWCurWp);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_SP,
			TEXT("[PSR] %s line %d fail in ESM_FifoSetWrPtr")
			TEXT("(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		MM_RETURN(mrRet);
	}

	mrRet = ESM_FifoSetRdPtr(prPsrFtr->u4ESIH, ptrHWCurWp, FALSE);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_SP,
			TEXT("[PSR] %s line %d fail in ESM_FifoSetRdPtr")
			TEXT("(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
#endif	/* DMX_DISABLE_COMP_SPAU */

#ifndef __linux__
#if 0				/*mtk40144 */
	if (ptrHWCurWp > ptrESICurWp) {
		CacheRangeFlush((void *) ptrESICurWp,
			(ptrHWCurWp - ptrESICurWp), CACHE_SYNC_DISCARD);
	} else if (ptrHWCurWp < ptrESICurWp) {
		CacheRangeFlush((void *) ptrESICurWp,
				(prPsrFtr->ptrESFifoEa - ptrESICurWp), CACHE_SYNC_DISCARD);
		CacheRangeFlush((void *) (prPsrFtr->ptrESFifoSa),
				(ptrHWCurWp - prPsrFtr->ptrESFifoSa), CACHE_SYNC_DISCARD);
	}
#endif
#endif				/* end of #ifndef __linux__ */

	u8TxSize = prPsrCC->u8TxCurrLen;

	/* because each transfer range of subpicture is a unit, we need check tx complete */
	if (prPsrFtr->u4Flag & FF_TX_PBBUF) {
		if ((prPsrCC->u8TxStartOffset + prPsrCC->u8TxLen) !=
				(prPsrCC->u8TxCurrOffset + u8TxSize)) {
			if (0 == prPsrFtr->ptrBkWrPtr)
				prPsrFtr->ptrBkWrPtr = ptrESICurWp;

			/* update ESI FIFO write pointer */
			mrRet = ESM_FifoSetWrPtr(prPsrFtr->u4ESIH, ptrHWCurWp);
			if (DMX_FAILED(mrRet)) {
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}

			MM_RETURN(RET_DMX_OK);
		}
	}

	/* update ESI FIFO write pointer */
	mrRet = ESM_FifoSetWrPtr(prPsrFtr->u4ESIH, ptrHWCurWp);
	if (DMX_FAILED(mrRet)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	/* Get ESI information */
	mrRet = ESM_AUTableGetWrIdx(prPsrFtr->u4ESIH, &u4CurAUWrIdx);
	if (DMX_FAILED(mrRet)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}
	mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4CurAUWrIdx, (void **) &prSPAU);
	if (DMX_FAILED(mrRet)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	/*fix klocwork bug */
	if (NULL == prSPAU) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_SP,
			TEXT("[PSR] %s line %d failed for prSPAU = NULL!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (prPsrFtr->fgAUCtrlByLen) {
		if (0 == prPsrFtr->u8CurAULen) {
			if (0 == prPsrFtr->ptrBkWrPtr) {
				if ((ptrESICurWp >= prPsrFtr->ptrESFifoEa) ||
						(ptrESICurWp < prPsrFtr->ptrESFifoSa)) {
					DMXLOG_ERROR(
								TEXT
								 ("[PSR] %s line %d fail in ptrESICurWp(0x%lx) exceed ")
								 TEXT("PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
								DMX_FUNC_NAME, DMX_LINE_NO, ptrESICurWp,
								prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_UNEXPECT);
				}

				prSPAU->rAUInfo.rInfo.ptrAddr = ptrESICurWp;
				prPsrFtr->ptrBkWrPtr = ptrESICurWp;
			} else {
				if ((prPsrFtr->ptrBkWrPtr >= prPsrFtr->ptrESFifoEa) ||
						(prPsrFtr->ptrBkWrPtr < prPsrFtr->ptrESFifoSa)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_SP,
						TEXT("[PSR] %s line %d fail in prPsrFtr->ptrBkWrPtr(0x%lx) ")
						TEXT("exceed PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
						DMX_FUNC_NAME,	DMX_LINE_NO, prPsrFtr->ptrBkWrPtr,
						prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_UNEXPECT);
				}
				prSPAU->rAUInfo.rInfo.ptrAddr = prPsrFtr->ptrBkWrPtr;
			}
			prSPAU->rAUInfo.rInfo.u8Offset = prPsrCC->u8TxStartOffset;
		}

		prPsrFtr->u8CurAULen += prPsrCC->u8TxLen;
		if (prPsrFtr->u8CurAULen >= prPsrFtr->u8TotalAULen) {
			if (prPsrFtr->u8CurAULen > prPsrFtr->u8TotalAULen) {
				DMX_ASSERT(FALSE);
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_SP,
					TEXT("[PSR] %s line %d fail for CurAULen("DMX_UINT64_10U_LOGSTR)
					TEXT(") > TotalAULen("DMX_UINT64_10U_LOGSTR)
					TEXT("), lasttxlen("DMX_UINT64_10U_LOGSTR")\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u8CurAULen,
					prPsrFtr->u8TotalAULen, prPsrCC->u8TxLen);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			prSPAU->rAUInfo.rInfo.u4Size = (u32) prPsrFtr->u8TotalAULen;
			/* Write ESI AU information */
			prSPAU->eAuType = AU_DATA;
			/* Callback ask splitter fill information */
			rAU.eType = prPsrFtr->eType;
			rAU.pvAUInf = prSPAU;
			rAU.pvAUExtInf = NULL;

			if (g_rDmxCliMan.fgDumpFlow) {
				DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

				mm_memset(&rOperInfo, (u8) 0x00,
						sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
				rOperInfo.pvSptHdl = prPsrCC->pvSptHdl;
				rOperInfo.unFlow.rFillAU.u4AUIdx = u4CurAUWrIdx;
				rOperInfo.unFlow.rFillAU.u4StmType = SPT_DATA_SP;
				DmxDumpFlow(DMX_OPER_FILLAU, &rOperInfo);
			}
#if !DMX_DISABLE_FILL_AUINFO
			PSR_CC_CBSplitter(prPsrCC, E_GET_AU_INFO, (void *) &rAU);
#endif

			if (0 == prSPAU->rAUInfo.rInfo.u4Size) {
				DMX_ASSERT(FALSE);
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_SP,
					TEXT("[PSR] %s line %d fail for AUSz == 0,")
					 TEXT(" CurAULen("DMX_UINT64_10U_LOGSTR)
					 TEXT("), TotalAULen("DMX_UINT64_10U_LOGSTR)
					 TEXT("), lasttxlen("DMX_UINT64_10U_LOGSTR")\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u8CurAULen,
					prPsrFtr->u8TotalAULen, prPsrCC->u8TxLen);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			if (INVALID_TIMESTAMP != prSPAU->rAUInfo.rInfo.u8StartPts) {
				prPsrFtr->u8LastPTS = prSPAU->rAUInfo.rInfo.u8StartPts;
#ifdef MM_SUPPORT_DIVXHT31
				prPsrFtr->u8PrevPTS = prPsrFtr->u8LastPTS;
				if (INVALID_TIMESTAMP == prPsrFtr->u81stPTS) {
					DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_SP,
						TEXT("[PSR] %s Line %d --- (SP) set")
						 TEXT(" 1stPts to be "DMX_PTS_LOGSTR"\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS),
						DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS);
					prPsrFtr->u81stPTS = prPsrFtr->u8LastPTS;
				}
#endif				/* MM_SUPPORT_DIVXHT31 */
			}

			if (prPsrFtr->fgFirstAUInRng) {
				/* Use the flag to designated whether the au is the first au in the cfa range */
				prSPAU->fgIBCSent = TRUE;
				prPsrFtr->fgFirstAUInRng = FALSE;
			}

			if (g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_SP]) {
				DmxDumpSPSample(prSPAU, prPsrFtr->ptrESFifoSa,
						prPsrFtr->ptrESFifoEa, prPsrFtr->u4StmUID, FALSE);
			}

			prPsrFtr->ptrBkWrPtr = 0;

			/* update ESI AU write index */
			mrRet = ESM_AUTableIncWrIdx(prPsrFtr->u4ESIH, 1);
			if (DMX_FAILED(mrRet)) {
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}
			prPsrFtr->fgAUCtrlByLen = FALSE;
			prPsrFtr->u8TotalAULen = 0;
			prPsrFtr->u8CurAULen = 0;
		}

		MM_RETURN(RET_DMX_OK);
	} else {
		/* Write ESI AU information */
		prSPAU->eAuType = AU_DATA;
		/* Callback ask splitter fill information */
		rAU.eType = prPsrFtr->eType;
		rAU.pvAUInf = prSPAU;
		rAU.pvAUExtInf = NULL;

		if (g_rDmxCliMan.fgDumpFlow) {
			DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

			mm_memset(&rOperInfo, 0x00, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
			rOperInfo.pvSptHdl = prPsrCC->pvSptHdl;
			rOperInfo.unFlow.rFillAU.u4AUIdx = u4CurAUWrIdx;
			rOperInfo.unFlow.rFillAU.u4StmType = SPT_DATA_SP;
			DmxDumpFlow(DMX_OPER_FILLAU, &rOperInfo);
		}
#if !DMX_DISABLE_FILL_AUINFO
		PSR_CC_CBSplitter(prPsrCC, E_GET_AU_INFO, (void *) &rAU);
#endif

		if (0 == prPsrFtr->ptrBkWrPtr) {
			if ((ptrESICurWp >= prPsrFtr->ptrESFifoEa) ||
					(ptrESICurWp < prPsrFtr->ptrESFifoSa)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_SP,
					TEXT("[PSR] %s line %d fail in ptrESICurWp(0x%lx) ")
					 TEXT("exceed PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, ptrESICurWp,
					prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}
			prSPAU->rAUInfo.rInfo.ptrAddr = ptrESICurWp;
			prPsrFtr->ptrBkWrPtr = ptrESICurWp;
		} else {
			if ((prPsrFtr->ptrBkWrPtr >= prPsrFtr->ptrESFifoEa) ||
					(prPsrFtr->ptrBkWrPtr < prPsrFtr->ptrESFifoSa)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_SP,
					TEXT("[PSR] %s line %d fail in prPsrFtr->ptrBkWrPtr(0x%lx) ")
					TEXT("exceed PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->ptrBkWrPtr,
					prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}
			prSPAU->rAUInfo.rInfo.ptrAddr = prPsrFtr->ptrBkWrPtr;
		}

		prSPAU->rAUInfo.rInfo.u8Offset = prPsrCC->u8TxStartOffset;
		prSPAU->rAUInfo.rInfo.u4Size = (u32) prPsrCC->u8TxLen;

		if (INVALID_TIMESTAMP != prSPAU->rAUInfo.rInfo.u8StartPts) {
			prPsrFtr->u8LastPTS = prSPAU->rAUInfo.rInfo.u8StartPts;
#ifdef MM_SUPPORT_DIVXHT31
			prPsrFtr->u8PrevPTS = prPsrFtr->u8LastPTS;
			if (INVALID_TIMESTAMP == prPsrFtr->u81stPTS) {
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_SP,
					TEXT("[PSR] %s Line %d --- (SP) set")
					 TEXT(" 1stPts to be "DMX_PTS_LOGSTR"\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS),
					DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS);
				prPsrFtr->u81stPTS = prPsrFtr->u8LastPTS;
			}
#endif				/* MM_SUPPORT_DIVXHT31 */
		}

		if (prPsrFtr->fgFirstAUInRng) {
			/* Use the flag to designated whether the au is the first au in the cfa range */
			prSPAU->fgIBCSent = TRUE;
			prPsrFtr->fgFirstAUInRng = FALSE;
		}

		if (g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_SP]) {
			DmxDumpSPSample(prSPAU, prPsrFtr->ptrESFifoSa,
					prPsrFtr->ptrESFifoEa, prPsrFtr->u4StmUID, FALSE);
		}

		prPsrFtr->ptrBkWrPtr = 0;

		/* update ESI AU write index */
		mrRet = ESM_AUTableIncWrIdx(prPsrFtr->u4ESIH, 1);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		MM_RETURN(RET_DMX_OK);
	}

	/*reset backup write pointer */
	prPsrFtr->ptrBkWrPtr = 0;

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_VFilter_UpdateESIInfo_NoHdrDect(PSR_FILTER *prPsrFtr)
{
	u32 u4CurAUWrIdx;
	AU_VPic *prVPicAU = NULL;
	AU_VPic *prPrevVPicAU = NULL;
	PSR_CC *prPsrCC = NULL;
	DMX_FIFO_INFO_T *prFifo = NULL;
	PSR_AU rAU;
	uintptr_t ptrCurFIFOWp = 0, ptrHWCurWp = 0;
	u32 u4TotalAUCnt = 0;
	u64 u8TxSize;
	PSR_VFSD *prVFSD = NULL;
	MRESULT mrRet = RET_DMX_OK;

	DMX_ASSERT(NULL != prPsrFtr);

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);
	DMX_ASSERT(NULL != prPsrCC);

	prVFSD = (PSR_VFSD *) prPsrFtr->pvFilterSpecific;
	DMX_ASSERT(NULL != prVFSD);

	/* get HW current write pointer */
	mrRet = PSR_HAL_GetWPtr(prPsrFtr->ucHwDevId, BitType_Video, &ptrHWCurWp);
	if (DMX_FAILED(mrRet))
		MM_RETURN(mrRet);

	if ((ptrHWCurWp >= prPsrFtr->ptrESFifoEa) ||
		(ptrHWCurWp < prPsrFtr->ptrESFifoSa)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail in ptrHWCurWp(0x%lx)")
			TEXT(" exceed PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, ptrHWCurWp, prPsrFtr->ptrESFifoSa,
			prPsrFtr->ptrESFifoEa);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	/* get current Fifo write pointer */
	mrRet = ESM_FifoGetWrPtr(prPsrFtr->u4ESIH, &ptrCurFIFOWp);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail in ESM_FifoGetWrPtr")
			TEXT("(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if ((ptrCurFIFOWp >= prPsrFtr->ptrESFifoEa) ||
		(ptrCurFIFOWp < prPsrFtr->ptrESFifoSa)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail in ptrCurFIFOWp(0x%lx)")
			TEXT(" exceed PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, ptrCurFIFOWp, prPsrFtr->ptrESFifoSa,
			prPsrFtr->ptrESFifoEa);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	mrRet = ESM_FifoGetInfo(prPsrFtr->u4ESIH, (void **) &prFifo);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail in ESM_FifoGetInfo")
			TEXT("(u4Handle: 0x%x, ..), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		MM_RETURN(mrRet);
	}

	if ((prFifo->ptrEa - prFifo->ptrSa != prPsrFtr->u4ESFifoSize) ||
			(prFifo->ptrSa != prPsrFtr->ptrESFifoSa) ||
			(prFifo->ptrEa != prPsrFtr->ptrESFifoEa)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail in PsrFtr's FifoInfo")
			TEXT("(Sa: 0x%x, Ea: 0x%x, Sz: 0x%x) != ESMInfo")
			TEXT("(Sa: 0x%x, Ea: 0x%x, Sz: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			prFifo->ptrSa, prFifo->ptrEa, (prFifo->ptrEa - prFifo->ptrSa),
			prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa, prPsrFtr->u4ESFifoSize);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	mrRet = ESM_FifoSetWrPtr(prPsrFtr->u4ESIH, ptrHWCurWp);
	if (DMX_FAILED(mrRet))
		MM_RETURN(mrRet);

#if DMX_DISABLE_COMP_NONHDRAU
	mrRet = ESM_FifoSetRdPtr(prPsrFtr->u4ESIH, ptrHWCurWp, FALSE);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail in ESM_FifoSetRdPtr")
			TEXT("(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
#endif				/* DMX_DISABLE_COMP_NONHDRAU */

	/* Get ESI information */
	/* If dummy AU, */
	if (prVFSD->fgUseDummyAURealWrIdx) {
		mrRet = ESM_AUTableGetNextAUIdx(prPsrFtr->u4ESIH,
			prVFSD->u4DummyAURealWrIdx, 1, &u4CurAUWrIdx);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail in ESM_AUTable")
				TEXT("GetNextAUIdx(auidx: %d, Count: %d), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prVFSD->u4DummyAURealWrIdx, 1,
				mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
		prVFSD->fgUseDummyAURealWrIdx = FALSE;
	} else {
		if ((prVFSD->fgUseRealWrIdx) &&
			(DMX_INVALID_UINT32 != prVFSD->u4RealWrIdx)) {
			mrRet = ESM_AUTableGetNextAUIdx(prPsrFtr->u4ESIH, prVFSD->u4RealWrIdx,
							0, &u4CurAUWrIdx);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] %s line %d fail in ESM_AU")
					TEXT("TableGetNextAUIdx(auidx: %d, Count: %d), mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prVFSD->u4RealWrIdx, 0x00, mrRet);
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}
		} else {
			mrRet = ESM_AUTableGetWrIdx(prPsrFtr->u4ESIH, &u4CurAUWrIdx);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] %s line %d fail in ESM_AU")
					TEXT("TableGetWrIdx(auidx: %d), mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx, mrRet);
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}
		}
	}

	mrRet = ESM_AUTableGetTotalCount(prPsrFtr->u4ESIH, &u4TotalAUCnt);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail in ESM_AUTableGet")
			TEXT("TotalCount, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (u4CurAUWrIdx >= u4TotalAUCnt) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail for u4CurAUWrIdx(%d)")
			TEXT(" > u4TotalAUCnt(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx, u4TotalAUCnt);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4CurAUWrIdx,
		(void **) &prVPicAU);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d failed in ESM_AUTableGet")
			TEXT("AUInfo(AuIdx: %d), mrRet: 0x%x!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx, mrRet);
		MM_RETURN(mrRet);
	}
	/*fix klocwork bug */
	if (NULL == prVPicAU) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail for prVPicAU = NULL!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	/* get transfer size */
	u8TxSize = prPsrCC->u8TxCurrLen;

	/* because each transfer range of DIVX311 is a picture, we need check tx complete */
	if ((prPsrFtr->u4Flag & FF_TX_PBBUF) &&
			((prPsrCC->u8TxStartOffset + prPsrCC->u8TxLen) !=
			 (prPsrCC->u8TxCurrOffset + u8TxSize))) {
		if ((prPsrFtr->fgAUCtrlByLen) && (0 == prPsrFtr->u8CurAULen))
			prVPicAU->rAUInfo.rInfo.u8Offset = prPsrCC->u8TxStartOffset;

		prVFSD->fgCreNoHdrAUWaitPkt = TRUE;
	}
#if ENABLE_DMX_ADVANCED_VER
	else if (prPsrCC->fgUseCmdQ && prPsrCC->fgAUByCmdQEnd) {
		u16 u2Index = prPsrCC->rCmdQTxInf.u2CurTxRngSIdx;
		u32 u4TxLen = 0;
		u32 u4CurPicAdvLen = 0;
		u32 u4BkpCurPicAdvLen = ptrHWCurWp - ptrCurFIFOWp;
		DMX_CMDQ_TX_ENTRY_T *prTxEntry = NULL;
		uintptr_t ptrCurStartWp = ptrCurFIFOWp;
		uintptr_t ptrCurEndWp = ptrCurFIFOWp;
		u32 u4VType = 0;

		if (NULL == prPsrCC->pvCmdQTxEntryBuffer) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] update AUDIO ESI, cmd q buffer is null, ")
				TEXT("fatal error!!\r\n"));
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		u4CurPicAdvLen = 0;

		prTxEntry = (DMX_CMDQ_TX_ENTRY_T *) (prPsrCC->pvCmdQTxEntryBuffer) +
			u2Index;

		for (; u2Index <= prPsrCC->rCmdQTxInf.u2CurTxRngEIdx; u2Index++) {
			prVFSD->fgCreNoHdrAUWaitPkt = TRUE;

			u4VType = Spt4CfaGetPitureType(prTxEntry->eTxMode);

			if ((SEQ_HDR == u4VType) ||
					(0 == GetPicType(prVPicAU->rAUInfo.rInfo.u4VType))) {
				/*sequence header has been add to the AU behind by modify AU SAaddr */
				if (0 == prVPicAU->rAUInfo.rInfo.ptrSAddr) {
					prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrCurEndWp;
					prVPicAU->rAUInfo.rInfo.ptrSeqHdrSa = ptrCurEndWp;
					prVPicAU->rAUInfo.rInfo.u4SeqHdrLen =
						(u32) prPsrCC->u8TxLen;	/*Keep Seq Hdr Length */
				}
				prVPicAU->rAUInfo.rInfo.u4VType |= u4VType;
			}

			if (u2Index == prPsrCC->rCmdQTxInf.u2CurTxRngSIdx) {
				if ((0 == prPsrCC->rCmdQTxInf.u4CurTxRngSIdxOfst) &&
						(0 < prPsrCC->rCmdQTxInf.u4CurTxRngSIdxLen)) {
					if (0 == prVPicAU->rAUInfo.rInfo.u8Offset) {
						prVPicAU->rAUInfo.rInfo.u8Offset =
								prPsrCC->u8TxCurrOffset + u4CurPicAdvLen;
					}
					if (prTxEntry->fgInsertHdr) {
						u4TxLen += prTxEntry->u4InsertHdrLen;
						ptrCurEndWp += prTxEntry->u4InsertHdrLen;
						prPsrFtr->u8CurAULen += prTxEntry->u4InsertHdrLen;
					}
				}
				u4TxLen += prPsrCC->rCmdQTxInf.u4CurTxRngSIdxLen;
				ptrCurEndWp += prPsrCC->rCmdQTxInf.u4CurTxRngSIdxLen;
				u4CurPicAdvLen += prPsrCC->rCmdQTxInf.u4CurTxRngSIdxLen;
				prPsrFtr->u8CurAULen += prPsrCC->rCmdQTxInf.u4CurTxRngSIdxLen;
			} else if (u2Index == prPsrCC->rCmdQTxInf.u2CurTxRngEIdx) {
				if (0 == prVPicAU->rAUInfo.rInfo.u8Offset) {
					prVPicAU->rAUInfo.rInfo.u8Offset =
							prPsrCC->u8TxCurrOffset + u4CurPicAdvLen +
							prPsrCC->rCmdQTxInf.u4CurTxRngEIdxOfst +
							prTxEntry->u4TxOfst;
				}
				if ((0 == prPsrCC->rCmdQTxInf.u4CurTxRngEIdxOfst) &&
						(0 < prPsrCC->rCmdQTxInf.u4CurTxRngEIdxLen) &&
						(prTxEntry->fgInsertHdr)) {
					u4TxLen += prTxEntry->u4InsertHdrLen;
					ptrCurEndWp += prTxEntry->u4InsertHdrLen;
					prPsrFtr->u8CurAULen += prTxEntry->u4InsertHdrLen;
				}

				u4TxLen += prPsrCC->rCmdQTxInf.u4CurTxRngEIdxLen;
				ptrCurEndWp += prPsrCC->rCmdQTxInf.u4CurTxRngEIdxLen;
				u4CurPicAdvLen += prPsrCC->rCmdQTxInf.u4CurTxRngEIdxLen
						+ prTxEntry->u4TxOfst;
				prPsrFtr->u8CurAULen += prPsrCC->rCmdQTxInf.u4CurTxRngEIdxLen;
			} else {
				if (prTxEntry->fgInsertHdr) {
					u4TxLen += prTxEntry->u4InsertHdrLen;
					ptrCurEndWp += prTxEntry->u4InsertHdrLen;
					prPsrFtr->u8CurAULen += prTxEntry->u4InsertHdrLen;
				}
				if (0 == prVPicAU->rAUInfo.rInfo.u8Offset) {
					prVPicAU->rAUInfo.rInfo.u8Offset = prPsrCC->u8TxCurrOffset
							+ u4CurPicAdvLen + prTxEntry->u4TxOfst;
				}

				u4TxLen += prTxEntry->u4TxLen;
				ptrCurEndWp += prTxEntry->u4TxLen;
				u4CurPicAdvLen += prTxEntry->u4TxLen + prTxEntry->u4TxOfst;
				prPsrFtr->u8CurAULen += prTxEntry->u4TxLen;
			}

			if (u4TxLen > u4BkpCurPicAdvLen)
				break;

			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR][ESM] %s line %d -- CmdIdx(%d), eTxMode(%d),")
				TEXT(" Vtype: 0x%x!!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u2Index, prTxEntry->eTxMode,
				prVPicAU->rAUInfo.rInfo.u4VType);

			if (prTxEntry->fgEndAU) {
				mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4CurAUWrIdx,
									 (void **) &prVPicAU);
				if (DMX_FAILED(mrRet)) {
					DMX_ASSERT(FALSE);
					MM_RETURN(mrRet);
				}

				prVFSD->fgCreNoHdrAUWaitPkt = FALSE;

				/* update AU table item */
				prVPicAU->eAuType = AU_DATA;
				/* callback notify Splitter to fill AU information */
				rAU.eType = prPsrFtr->eType;
				rAU.pvAUInf = prVPicAU;
				rAU.pvAUExtInf = NULL;

				if (0 == prVPicAU->rAUInfo.rInfo.u8Offset) {
					prVPicAU->rAUInfo.rInfo.u8Offset =
							prPsrCC->u8TxCurrOffset + u4CurPicAdvLen;
				}

				if (g_rDmxCliMan.fgDumpFlow) {
					DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

					mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
					rOperInfo.pvSptHdl = prPsrCC->pvSptHdl;
					rOperInfo.unFlow.rFillAU.u4AUIdx = u4CurAUWrIdx;
					rOperInfo.unFlow.rFillAU.u4StmType = SPT_DATA_V;
					DmxDumpFlow(DMX_OPER_FILLAU, &rOperInfo);
				}

				PSR_CC_CBSplitter(prPsrCC, E_GET_AU_INFO, (void *) &rAU);

				/* callback ask splitter fill information */
				if (0 == prVPicAU->rAUInfo.rInfo.ptrSAddr) {
					prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrCurStartWp;
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("[PSR] %s line %d -- ByCmdQ  Set prVPicAU(AUIdx: %d)'s")
						 TEXT(" ptrSAddr = u4CurStartWp(0x%x)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx,
						ptrCurStartWp);
				}

				if (INVALID_TIMESTAMP != prVPicAU->rAUInfo.rInfo.u8Pts) {
					prPsrFtr->u8LastPTS = prVPicAU->rAUInfo.rInfo.u8Pts;
#ifdef MM_SUPPORT_DIVXHT31
					prPsrFtr->u8PrevPTS = prPsrFtr->u8LastPTS;
					if (INVALID_TIMESTAMP == prPsrFtr->u81stPTS) {
						DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
							TEXT("[PSR] %s Line %d --- (Video)")
							TEXT(" set 1stPts to be "DMX_PTS_LOGSTR"\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS),
							DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS));
						prPsrFtr->u81stPTS = prPsrFtr->u8LastPTS;
					}
#endif				/* MM_SUPPORT_DIVXHT31 */
				}

				if (prPsrFtr->fgFirstAUInRng) {
					/* Use the flag to designated whether the au is the first au in the cfa */
					/* range, TRUE:FIRST */
					prVPicAU->fgIBCSent = TRUE;
					prPsrFtr->fgFirstAUInRng = FALSE;
				}

				/*Add End Addr Info */
				PSR_HAL_GetWPtr(prPsrFtr->ucHwDevId, BitType_Video, &ptrHWCurWp);

				if (ptrCurEndWp >= prPsrFtr->ptrESFifoEa)
					ptrCurEndWp -= prPsrFtr->u4ESFifoSize;

				/*Init Previous PTS */
				prVPicAU->rAUInfo.rInfo.u4Duration = 0x0;
				prVPicAU->rAUInfo.rInfo.u4PrevDuration = 0x0;
				prVPicAU->rAUInfo.rInfo.u8PrevPTS = 0x0;

				SetVCodec(prVPicAU->rAUInfo.rInfo.u4VType, prVFSD->eVCodeC);

				prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrCurEndWp;

				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] %s line %d -- NoHdrDect: ByCmdQ	Create AU(Idx:%d)")
					 TEXT(" -- VType: 0x%x, SA: 0x%x, EA: 0x%x")
					 TEXT(", Offset: " DMX_UINT64_10U_LOGSTR)
					 TEXT(", PTS: "DMX_PTS_LOGSTR"\r\n"),
					 DMX_FUNC_NAME, DMX_LINE_NO,
					u4CurAUWrIdx, prVPicAU->rAUInfo.rInfo.u4VType,
					prVPicAU->rAUInfo.rInfo.ptrSAddr,
					prVPicAU->rAUInfo.rInfo.ptrEAddr,
					DMX_UINT64_10U_LOG(prVPicAU->rAUInfo.rInfo.u8Offset),
					DMX_PTS_LOG_MS(prVPicAU->rAUInfo.rInfo.u8Pts),
					DMX_PTS_LOG_PTS(prVPicAU->rAUInfo.rInfo.u8Pts));

				/* update ESI AU write index */
				mrRet = ESM_AUTableIncWrIdx(prPsrFtr->u4ESIH, 1);
				if (DMX_FAILED(mrRet))
					MM_RETURN(mrRet);

				mrRet = ESM_AUTableGetWrIdx(prPsrFtr->u4ESIH, &u4CurAUWrIdx);

				if (DMX_FAILED(mrRet))
					MM_RETURN(mrRet);

				mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4CurAUWrIdx,
									 (void **) &prVPicAU);
				if (DMX_FAILED(mrRet)) {
					DMX_ASSERT(FALSE);
					MM_RETURN(mrRet);
				}

				ptrCurStartWp = ptrCurEndWp;

				prPsrFtr->ptrBkWrPtr = 0;

			} else {
				mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4CurAUWrIdx,
									 (void **) &prVPicAU);
				if (DMX_FAILED(mrRet)) {
					DMX_ASSERT(FALSE);
					MM_RETURN(mrRet);
				}

				if (0 == prVPicAU->rAUInfo.rInfo.ptrSAddr) {
					prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrCurStartWp;
					prPsrFtr->ptrBkWrPtr = ptrCurStartWp;

					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("[PSR] %s line %d -- ByCmdQ  Set prVPicAU(AUIdx: %d)'s")
						 TEXT(" SAddr = u4CurStartWp(0x%x)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx,
						ptrCurStartWp);
				}
			}

			prTxEntry++;
		}

		prPsrFtr->fgAUCtrlByLen = FALSE;
		prPsrFtr->u8TotalAULen = 0;
		prPsrFtr->u8CurAULen = 0;

		prVFSD->fgUseRealWrIdx = FALSE;
		prVFSD->u4VType = 0;

		MM_RETURN(RET_DMX_OK);
	}
#endif				/* ENABLE_DMX_ADVANCED_VER */
	else if (prPsrFtr->fgAUCtrlByLen) {
		/*Init Add AU Flag */
		prVFSD->fgCreNoHdrAUWaitPkt = TRUE;

		if (0 == prPsrFtr->u8CurAULen)
			prVPicAU->rAUInfo.rInfo.u8Offset = prPsrCC->u8TxStartOffset;

		prPsrFtr->u8CurAULen += prPsrCC->u8TxLen;
		if (prPsrFtr->u8CurAULen >= prPsrFtr->u8TotalAULen) {
			if (prPsrFtr->u8CurAULen > prPsrFtr->u8TotalAULen) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] %s line %d fail for CurAULen("DMX_UINT64_10U_LOGSTR)
					 TEXT(") > TotalAULen("DMX_UINT64_10U_LOGSTR)
					 TEXT("), lasttxlen("DMX_UINT64_10U_LOGSTR")\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					DMX_UINT64_10U_LOG(prPsrFtr->u8CurAULen),
					DMX_UINT64_10U_LOG(prPsrFtr->u8TotalAULen),
					DMX_UINT64_10U_LOG(prPsrCC->u8TxLen));
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			prVFSD->fgCreNoHdrAUWaitPkt = FALSE;

			prPsrFtr->fgAUCtrlByLen = FALSE;
			prPsrFtr->u8TotalAULen = 0;
			prPsrFtr->u8CurAULen = 0;
		}
	} else if (prPsrFtr->fgAUCtrlByEnd) {
		if (0 == prPsrFtr->u8CurAULen)
			prVPicAU->rAUInfo.rInfo.u8Offset = prPsrCC->u8TxStartOffset;

		prPsrFtr->u8CurAULen += prPsrCC->u8TxLen;
		prVFSD->fgCreNoHdrAUWaitPkt = TRUE;
		if (prPsrFtr->fgAUEnd) {
			prVFSD->fgCreNoHdrAUWaitPkt = FALSE;
			prPsrFtr->fgAUEnd = FALSE;
			prPsrFtr->fgAUCtrlByEnd = FALSE;
			prPsrFtr->u8CurAULen = 0;
		}
	} else {
		prVFSD->fgCreNoHdrAUWaitPkt = FALSE;
		if (0 == prVPicAU->rAUInfo.rInfo.u8Offset)
			prVPicAU->rAUInfo.rInfo.u8Offset = prPsrCC->u8TxStartOffset;

		if (prPsrCC->fgTxMem2Fifo) {
			prVFSD->fgCreNoHdrAUWaitPkt = TRUE;

			/* Write ESI AU information */
			prVPicAU->eAuType = AU_DATA;

			/* Callback ask splitter fill information */
			mm_memset(&rAU, 0, sizeof(PSR_AU));
			rAU.eType = prPsrFtr->eType;
			rAU.pvAUInf = prVPicAU;
			rAU.pvAUExtInf = NULL;

			/*need to add next picture type */
			prVPicAU->rAUInfo.rInfo.u4VType |= prVFSD->u4VType;

			/*In wmv3, sequence header should add to the AU behind */
			if (SEQ_HDR == DMX_GET_PICTYPE(prVPicAU->rAUInfo.rInfo.u4VType)) {
				/*sequence header has been add to the AU behind by modify AU SAaddr */
				if (0 == prVPicAU->rAUInfo.rInfo.ptrSAddr) {
					prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrCurFIFOWp;
					prVPicAU->rAUInfo.rInfo.ptrSeqHdrSa = ptrCurFIFOWp;
					prVPicAU->rAUInfo.rInfo.u4SeqHdrLen =
						(u32) prPsrCC->u8TxLen;	/*Keep Seq Hdr Length */
				}
				prVFSD->u4VType = 0;
				/*Don't add AU Index, wait next picture */
				MM_RETURN(RET_DMX_OK);
			}
			/*Init Previous PTS */
			prVPicAU->rAUInfo.rInfo.u4Duration = 0x0;
			prVPicAU->rAUInfo.rInfo.u4PrevDuration = 0x0;
			prVPicAU->rAUInfo.rInfo.u8PrevPTS = 0x0;
		}
	}

	if (!prVFSD->fgCreNoHdrAUWaitPkt) {
		/* Write ESI AU information */
		prVPicAU->eAuType = AU_DATA;

		/* Callback ask splitter fill information */
		mm_memset(&rAU, 0, sizeof(PSR_AU));
		rAU.eType = prPsrFtr->eType;
		rAU.pvAUInf = prVPicAU;
		rAU.pvAUExtInf = NULL;

		/*need to add next picture type */
		prVPicAU->rAUInfo.rInfo.u4VType |= prVFSD->u4VType;

		/*In wmv3, sequence header should add to the AU behind */
		if (SEQ_HDR == DMX_GET_PICTYPE(prVPicAU->rAUInfo.rInfo.u4VType)) {
			/*sequence header has been add to the AU behind by modify AU SAaddr */
			if (0 == prVPicAU->rAUInfo.rInfo.ptrSAddr) {
				prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrCurFIFOWp;
				prVPicAU->rAUInfo.rInfo.ptrSeqHdrSa = ptrCurFIFOWp;
				prVPicAU->rAUInfo.rInfo.u4SeqHdrLen =
					(u32) prPsrCC->u8TxLen;	/*Keep Seq Hdr Length */
			}

			prVFSD->u4VType = 0;

			/*Don't add AU Index, wait next picture */
			MM_RETURN(RET_DMX_OK);
		}

		/*Init Previous PTS */
		prVPicAU->rAUInfo.rInfo.u4Duration = 0x0;
		prVPicAU->rAUInfo.rInfo.u4PrevDuration = 0x0;
		prVPicAU->rAUInfo.rInfo.u8PrevPTS = 0x0;

		if (g_rDmxCliMan.fgDumpFlow) {
			DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

			mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
			rOperInfo.pvSptHdl = prPsrCC->pvSptHdl;
			rOperInfo.unFlow.rFillAU.u4AUIdx = u4CurAUWrIdx;
			rOperInfo.unFlow.rFillAU.u4StmType = SPT_DATA_V;
			DmxDumpFlow(DMX_OPER_FILLAU, &rOperInfo);
		}
#if !DMX_DISABLE_FILL_AUINFO
		PSR_CC_CBSplitter(prPsrCC, E_GET_AU_INFO, (void *) &rAU);
#endif

		SetVCodec(prVPicAU->rAUInfo.rInfo.u4VType, prVFSD->eVCodeC);
		if (0 == prVPicAU->rAUInfo.rInfo.ptrSAddr)
			prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrCurFIFOWp;

		prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;

		if (prPsrFtr->fgFirstAUInRng) {
			/* Use the flag to designated whether the au is the first au in the cfa range */
			prVPicAU->fgIBCSent = TRUE;
			prPsrFtr->fgFirstAUInRng = FALSE;
		}
		{
			u32 u4PrevAUIdx = ESM_INVALID_INDEX;

			mrRet = ESM_AUTableGetPrevAUIdx(prPsrFtr->u4ESIH, u4CurAUWrIdx, 1,
							&u4PrevAUIdx);
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

				if (NULL == prPrevVPicAU) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("[PSR] %s line %d fail for prPrevVPicAU = NULL!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_UNEXPECT);
				}

				if (INVALID_TIMESTAMP == prVPicAU->rAUInfo.rInfo.u8PrevPTS) {
					/* For File Playback PTS control */
					prPrevVPicAU->rAUInfo.rInfo.u8Pts = INVALID_TIMESTAMP;
				}

				if (0x0 != prVPicAU->rAUInfo.rInfo.u4PrevDuration) {
					prPrevVPicAU->rAUInfo.rInfo.u4Duration =
							prVPicAU->rAUInfo.rInfo.u4PrevDuration;
				}
			}
		}

		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("NoHdrDect: Create AU(Idx:%d) -- VType: 0x%x, SA: 0x%x, EA: 0x%x")
			 TEXT(", Offset: "DMX_UINT64_10U_LOGSTR)
			 TEXT(", PTS:"DMX_PTS_LOGSTR"\r\n"),
			u4CurAUWrIdx,
			prVPicAU->rAUInfo.rInfo.u4VType,
			prVPicAU->rAUInfo.rInfo.ptrSAddr,
			prVPicAU->rAUInfo.rInfo.ptrEAddr,
			DMX_UINT64_10U_LOG(prVPicAU->rAUInfo.rInfo.u8Offset),
			DMX_PTS_LOG_MS(prVPicAU->rAUInfo.rInfo.u8Pts),
			DMX_PTS_LOG_PTS(prVPicAU->rAUInfo.rInfo.u8Pts));

		if (INVALID_TIMESTAMP != prVPicAU->rAUInfo.rInfo.u8Pts) {
			prPsrFtr->u8LastPTS = prVPicAU->rAUInfo.rInfo.u8Pts;
#ifdef MM_SUPPORT_DIVXHT31
			prPsrFtr->u8PrevPTS = prPsrFtr->u8LastPTS;
			if (INVALID_TIMESTAMP == prPsrFtr->u81stPTS) {
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] %s Line %d --- (Video)")
					TEXT(" set 1stPts to be "DMX_PTS_LOGSTR"\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS),
					DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS));
				prPsrFtr->u81stPTS = prPsrFtr->u8LastPTS;
			}
#endif				/*	MM_SUPPORT_DIVXHT31 */
		}

		if (MM_IS_RW_PLAY(SplitterGetPlayRate(prPsrCC->pvSptHdl))) {
			if (fgIsIType(prVPicAU->rAUInfo.rInfo.u4VType)) {
				if (0 == prPsrFtr->u4AUCntFromIFrm)
					prPsrFtr->u4AUCntFromIFrm = 1;
				else
					prPsrFtr->u4AUCntFromIFrm++;

				prPsrFtr->u4IFrmCnt++;
			} else if (0 < prPsrFtr->u4AUCntFromIFrm) {
				prPsrFtr->u4AUCntFromIFrm++;
			}

			if (prPsrFtr->u4AUCntFromIFrm > MAX_VID_AU_CNT_IN_CTRL_RW) {
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] %s -- RW Ctrl Need JUMP!\r\n"),
					DMX_FUNC_NAME);
				mrRet = ESM_AUTableIncWrIdx(prPsrFtr->u4ESIH, 1);
				if (DMX_FAILED(mrRet))
					MM_RETURN(mrRet);

				prVFSD->fgUseDummyAURealWrIdx = FALSE;
				prVFSD->fgUseRealWrIdx = FALSE;
				prVFSD->u4VType = 0;
				PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
				MM_RETURN(RET_DMX_NEED_JUMP);
			}
		}
#ifdef MM_SUPPORT_DIVXHT31
		if ((prPsrCC != NULL) &&
			(CFA_TYPE_AVI == SplitterGetCfaType(prPsrCC->pvSptHdl))) {
			s32 i4Rate = SplitterGetPlayRate(prPsrCC->pvSptHdl);

			if ((i4Rate >= MM_PLAY_RATE_FF_8X) || (MM_IS_RW_PLAY(i4Rate))) {
				if (fgIsIType(prVPicAU->rAUInfo.rInfo.u4VType)) {
					if (0 == prPsrFtr->u4AUCntFromIFrm)
						prPsrFtr->u4AUCntFromIFrm = 1;
					else
						prPsrFtr->u4AUCntFromIFrm++;

				} else if (0 < prPsrFtr->u4AUCntFromIFrm) {
					prPsrFtr->u4AUCntFromIFrm++;
				}

				if (prPsrFtr->u4AUCntFromIFrm > MAX_VID_AU_CNT_IN_CTRL_RW) {
					DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("[PSR] %s -- DivXHT31 Need JUMP!\r\n"),
						DMX_FUNC_NAME);
					mrRet = ESM_AUTableIncWrIdx(prPsrFtr->u4ESIH, 1);
					if (DMX_FAILED(mrRet))
						MM_RETURN(mrRet);

					prVFSD->fgUseDummyAURealWrIdx = FALSE;
					prVFSD->fgUseRealWrIdx = FALSE;
					prVFSD->u4VType = 0;
					PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
					MM_RETURN(RET_DMX_NEED_JUMP);
				}
			}
		}
#endif				/* MM_SUPPORT_DIVXHT31 */

		/* Update ESI AU write index */
		mrRet = ESM_AUTableIncWrIdx(prPsrFtr->u4ESIH, 1);
		if (DMX_FAILED(mrRet))
			MM_RETURN(mrRet);

		prVFSD->fgUseRealWrIdx = FALSE;
		prVFSD->u4VType = 0;
	} else {
		if (0 == prVPicAU->rAUInfo.rInfo.ptrSAddr)
			prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrCurFIFOWp;

		prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;
		if (u4CurAUWrIdx >= u4TotalAUCnt) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for u4CurAUWrIdx(%d) > ")
				TEXT("u4TotalAUCnt(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx, u4TotalAUCnt);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		prVFSD->u4RealWrIdx = u4CurAUWrIdx;
		prVFSD->fgUseRealWrIdx = TRUE;
		/* get AU */
		mrRet = ESM_AUTableSetWrIdx(prPsrFtr->u4ESIH, u4CurAUWrIdx);
		if (RET_DMX_OK != mrRet) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_VFilter_SetFileOfstInAU(PSR_FILTER *prPsrFtr,
	AU_VPic *prVPicAU, uintptr_t ptrCurFIFOWp, u32 u4PicIdx)
{
	PSR_VFSD *prVFSD = NULL;
	PSR_CC *prPsrCC = NULL;
	s32  i4CurPicAdvLen = 0;
	u64 u8Offset = 0;

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);
	DMX_ASSERT(NULL != prPsrCC);

	prVFSD = (PSR_VFSD *) prPsrFtr->pvFilterSpecific;
	DMX_ASSERT(NULL != prVFSD);

	/* Calculate offset */
	if (prVFSD->rPicDetResult.ptrPicAddr[u4PicIdx] >= ptrCurFIFOWp) {
		i4CurPicAdvLen = (prVFSD->rPicDetResult.ptrPicAddr[u4PicIdx] -
					ptrCurFIFOWp);
	} else {
		i4CurPicAdvLen = ((prVFSD->rPicDetResult.ptrPicAddr[u4PicIdx] +
					 prPsrFtr->u4ESFifoSize) - ptrCurFIFOWp);
	}

	if ((prPsrCC->fgUseCmdQ) &&
			(NULL != prPsrCC->pvCmdQTxEntryBuffer)) {
		u16 u2Index = prPsrCC->rCmdQTxInf.u2CurTxRngSIdx + 1;
		u32 u4TxLen = prPsrCC->rCmdQTxInf.u4CurTxRngSIdxLen;
		s32  i4BkpCurPicAdvLen = i4CurPicAdvLen;
		DMX_CMDQ_TX_ENTRY_T *prTxEntry = NULL;

#if ENABLE_DMX_ADVANCED_VER
		prTxEntry = (DMX_CMDQ_TX_ENTRY_T *)(prPsrCC->pvCmdQTxEntryBuffer) +
				prPsrCC->rCmdQTxInf.u2CurTxRngSIdx;
		if ((0 == prPsrCC->rCmdQTxInf.u4CurTxRngSIdxOfst) &&
				(0 < prPsrCC->rCmdQTxInf.u4CurTxRngSIdxLen)) {
			if (prTxEntry->fgInsertHdr)
				u4TxLen += prTxEntry->u4InsertHdrLen;
		}
#endif /* ENABLE_DMX_ADVANCED_VER */

		if (u4TxLen < i4BkpCurPicAdvLen) {
			i4CurPicAdvLen = 0;
			prTxEntry = (DMX_CMDQ_TX_ENTRY_T *)(prPsrCC->pvCmdQTxEntryBuffer) +
				u2Index;
			for (; u2Index < prPsrCC->rCmdQTxInf.u2CurTxRngEIdx; u2Index++) {
#if ENABLE_DMX_ADVANCED_VER
				if (prTxEntry->fgInsertHdr) {
					if (u4TxLen + prTxEntry->u4InsertHdrLen >= i4BkpCurPicAdvLen) {
						i4CurPicAdvLen += prTxEntry->u4TxOfst;
						break;
					}	else if (u4TxLen + prTxEntry->u4InsertHdrLen +
						prTxEntry->u4TxLen >= i4BkpCurPicAdvLen) {
						i4CurPicAdvLen += prTxEntry->u4TxOfst +
							(i4BkpCurPicAdvLen - u4TxLen - prTxEntry->u4InsertHdrLen);
						u4TxLen += i4BkpCurPicAdvLen - u4TxLen
							- prTxEntry->u4InsertHdrLen;
						break;
					}
					u4TxLen += prTxEntry->u4InsertHdrLen;
				} else {
#endif /* ENABLE_DMX_ADVANCED_VER */
					if (u4TxLen + prTxEntry->u4TxLen >= i4BkpCurPicAdvLen) {
						i4CurPicAdvLen += prTxEntry->u4TxOfst +=
							(i4BkpCurPicAdvLen - u4TxLen);
						u4TxLen += (i4BkpCurPicAdvLen - u4TxLen);
						break;
					}
#if ENABLE_DMX_ADVANCED_VER
				}
#endif /* ENABLE_DMX_ADVANCED_VER */
				u4TxLen += prTxEntry->u4TxLen;
				i4CurPicAdvLen += prTxEntry->u4TxOfst + prTxEntry->u4TxLen;

				prTxEntry++;
			}

			if (prPsrCC->rCmdQTxInf.u2CurTxRngSIdx !=
				prPsrCC->rCmdQTxInf.u2CurTxRngEIdx) {
				prTxEntry = (DMX_CMDQ_TX_ENTRY_T *)(prPsrCC->pvCmdQTxEntryBuffer) +
						prPsrCC->rCmdQTxInf.u2CurTxRngEIdx;

#if ENABLE_DMX_ADVANCED_VER
				if (prTxEntry->fgInsertHdr)	{
					if (u4TxLen + prTxEntry->u4InsertHdrLen >= i4BkpCurPicAdvLen)
						i4CurPicAdvLen += prTxEntry->u4TxOfst;
					else if (u4TxLen + prTxEntry->u4InsertHdrLen +
						prPsrCC->rCmdQTxInf.u4CurTxRngEIdxLen >= i4BkpCurPicAdvLen) {
							i4CurPicAdvLen += prTxEntry->u4TxOfst;
							i4CurPicAdvLen += i4BkpCurPicAdvLen - u4TxLen
								- prTxEntry->u4InsertHdrLen;
							u4TxLen += i4BkpCurPicAdvLen - u4TxLen
								- prTxEntry->u4InsertHdrLen;
					} else {
							u4TxLen += prTxEntry->u4InsertHdrLen +
								prPsrCC->rCmdQTxInf.u4CurTxRngEIdxLen;
					}
				}	else {
#endif /* ENABLE_DMX_ADVANCED_VER */
					if (u4TxLen + prPsrCC->rCmdQTxInf.u4CurTxRngEIdxLen
						>= i4BkpCurPicAdvLen) {
							i4CurPicAdvLen += prTxEntry->u4TxOfst;
							i4CurPicAdvLen += (i4BkpCurPicAdvLen - u4TxLen);
							u4TxLen += (i4BkpCurPicAdvLen - u4TxLen);
					} else {
							i4CurPicAdvLen += prTxEntry->u4TxOfst;
							u4TxLen += prPsrCC->rCmdQTxInf.u4CurTxRngEIdxLen;
					}
#if ENABLE_DMX_ADVANCED_VER
				}
#endif /* ENABLE_DMX_ADVANCED_VER */
			}
		} else {
#if ENABLE_DMX_ADVANCED_VER
			prTxEntry = (DMX_CMDQ_TX_ENTRY_T *)(prPsrCC->pvCmdQTxEntryBuffer) +
					prPsrCC->rCmdQTxInf.u2CurTxRngSIdx;
			if ((0 == prPsrCC->rCmdQTxInf.u4CurTxRngSIdxOfst) &&
					(0 < prPsrCC->rCmdQTxInf.u4CurTxRngSIdxLen)) {
				if (prTxEntry->fgInsertHdr) {
					if (prTxEntry->u4InsertHdrLen >= i4BkpCurPicAdvLen)
						i4CurPicAdvLen = 0;
					else
						i4CurPicAdvLen = i4BkpCurPicAdvLen - prTxEntry->u4InsertHdrLen;
				}
			} else if (i4BkpCurPicAdvLen >= 0) {
					i4CurPicAdvLen = i4BkpCurPicAdvLen;
			} else {
					i4CurPicAdvLen = 0;
			}
#endif /* ENABLE_DMX_ADVANCED_VER */
		}
	}

	if (VC_H265 == prVFSD->eVCodeC) {
		if (0 == prVPicAU->rAUInfo.rInfo.ptrSAddr) {
			u8Offset = prPsrCC->u8TxCurrOffset + i4CurPicAdvLen;
			prVPicAU->rAUInfo.rInfo.u8Offset = u8Offset;
	  }
	} else {
		u8Offset = prPsrCC->u8TxCurrOffset + i4CurPicAdvLen;
		prVPicAU->rAUInfo.rInfo.u8Offset = u8Offset;
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_VFilter_UpdateESIInfo_MPEG2(PSR_FILTER *prPsrFtr,
	u32 u4CurAUWrIdx, AU_VPic *prVPicAU,
	uintptr_t ptrHWCurWp, uintptr_t ptrCurFIFOWp)
{
	PSR_AU rAU;
	PSR_CC *prPsrCC = NULL;
	PSR_VFSD *prVFSD = NULL;
	AU_VPic *prPrevVPicAU = NULL;
	u32 u4AUIdx;
	u32 u4Idx;
	u32 u4TotalAUCnt = 0;
	u32 u4VType = 0;
	MRESULT mrRet = RET_DMX_OK;

	DMX_ASSERT(NULL != prPsrFtr);

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);
	DMX_ASSERT(NULL != prPsrCC);

	prVFSD = (PSR_VFSD *) prPsrFtr->pvFilterSpecific;
	DMX_ASSERT(NULL != prVFSD);

	mrRet = ESM_AUTableGetTotalCount(prPsrFtr->u4ESIH, &u4TotalAUCnt);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail in ESM_AUTable")
			TEXT("GetTotalCount, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	u4Idx = 0;
	while (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount) {
		if (u4Idx >= DMX_MAX_VID_STARTCODE_CNT) {
			DMX_ASSERT(FALSE);
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for PicCount(%d)")
				 TEXT(" > MaxStartCodeCnt(%d)\r\n"), DMX_FUNC_NAME,
				DMX_LINE_NO, u4Idx, DMX_MAX_VID_STARTCODE_CNT);
			MM_RETURN(RET_DMX_OVER_LIMIT);
		}

		if ((prVFSD->rPicDetResult.ptrPicAddr[u4Idx] >= prPsrFtr->ptrESFifoEa) ||
				(prVFSD->rPicDetResult.ptrPicAddr[u4Idx] < prPsrFtr->ptrESFifoSa)) {
			DMX_ASSERT(FALSE);
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for PicDetResultIdx(%d)'s ")
				TEXT("PicAddr(0x%lx) exceed fifo Range[0x%lx, 0x%lx)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
				prVFSD->rPicDetResult.ptrPicAddr[u4Idx],
				prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		/* get AU */
		u4AUIdx = u4CurAUWrIdx;
		if (u4CurAUWrIdx >= u4TotalAUCnt) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for u4CurAUWrIdx")
				 TEXT("(%d) > u4TotalAUCnt(%d)\r\n"), DMX_FUNC_NAME,
				DMX_LINE_NO, u4CurAUWrIdx, u4TotalAUCnt);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4AUIdx,
			(void **) &prVPicAU);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for in ESM_AUTableGetAUInfo!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		/*fix klocwork bug */
		if (NULL == prVPicAU) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for prVPicAU = NULL!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		if (AU_DATA == prVPicAU->eAuType)
			u4VType = prVPicAU->rAUInfo.rInfo.u4VType;
		else
			u4VType = 0;

		if (0 == DMX_GET_PICTYPE(u4VType)) {
			if (PSR_HDR_MP2_SeqEnd == prVFSD->rPicDetResult.u1PicType[u4Idx]) {
				/* may be encounter the following cause : */
				/* seqend1 seqend2 seqend3 frame */
				/* seqend2 to seqend3 should be skip, so do this. */
				u4Idx++;
				if (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount) {
					prVPicAU->rAUInfo.rInfo.ptrSAddr =
							prVFSD->rPicDetResult.ptrPicAddr[u4Idx];
				} else {
					prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrHWCurWp;
				}
				continue;
			}

			mrRet = PSR_VFilter_SetFileOfstInAU(prPsrFtr, prVPicAU,
				ptrCurFIFOWp, u4Idx);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] %s line %d fail in PSR_VFilter_SetFileOfstInAU!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				MM_RETURN(mrRet);
			}

			/* Fill AU */
			if (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount) {
				prVPicAU->rAUInfo.rInfo.ptrSAddr =
						prVFSD->rPicDetResult.ptrPicAddr[u4Idx];
			} else {
				prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrHWCurWp;
			}
		}

		switch (prVFSD->rPicDetResult.u1PicType[u4Idx]) {
			/* check whether Sequence header, GOP header, Sequence End */
		case PSR_HDR_MP2_SeqHdr:
			if (!prVFSD->fgHasSetPics) {
				u4VType |= SEQ_HDR;
				prVPicAU->rAUInfo.rInfo.u4VType |= SEQ_HDR;
				/* write ESI AU information */
				prVPicAU->eAuType = AU_DATA;

				u4Idx++;
				continue;
			}
			break;

		case PSR_HDR_MP2_GOP:{
				u4VType |= GOP_HDR;
				prVPicAU->rAUInfo.rInfo.u4VType |= GOP_HDR;
				/* write ESI AU information */
				prVPicAU->eAuType = AU_DATA;

				u4Idx++;
				continue;
			}
			break;

		case PSR_HDR_MP2_SeqEnd:{
				if (!prVFSD->fgHasSetPics) {
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("MPEG2: SEQ_END AU(Idx:%d) -- but no Picture\r\n"),
						u4AUIdx);
					u4Idx++;
					continue;
				} else {
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("MPEG2: SEQ_END AU(Idx:%d)\r\n"),
						u4AUIdx);
					u4VType |= SEQ_END;
					prVPicAU->rAUInfo.rInfo.u4VType |= SEQ_END;
					u4Idx++;
					break;
				}
			}
			break;
		case PSR_HDR_MP2_IPic:
		case PSR_HDR_MP2_PPic:
		case PSR_HDR_MP2_BPic:
			if (!prVFSD->fgHasSetPics) {
				/* actual  setup AU from here..... */
				u4VType |= prVFSD->rPicDetResult.u1PicType[u4Idx];
			}
			break;
		case DUMMY_TYPE:
			if (!prVFSD->fgHasSetPics)
				u4VType |= DUMMY_FRM;
			break;

		default:{
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] Idx(%d) -- UpdateMPEG2ESI,")
				TEXT(" error, pic type:0x%x\r\n"), u4Idx,
				prVFSD->rPicDetResult.u1PicType[u4Idx]);
				u4Idx++;
				continue;
			}
			break;
		}	/* end switch */

		if (!prVFSD->fgHasSetPics) {	/*before getting a picture */
			u32 u4PrevAUIdx = DMX_INVALID_UINT32;
			/* write ESI AU information */
			prVPicAU->eAuType = AU_DATA;

			/* callback notify Splitter to fill AU information */
			mm_memset(&rAU, 0, sizeof(PSR_AU));
			rAU.eType = prPsrFtr->eType;
			rAU.pvAUInf = prVPicAU;
			rAU.pvAUExtInf = NULL;

			/*Init Previous PTS */
			prVPicAU->rAUInfo.rInfo.u4Duration = 0;
			prVPicAU->rAUInfo.rInfo.u4PrevDuration = 0;
			prVPicAU->rAUInfo.rInfo.u8PrevPTS = 0;

			if (g_rDmxCliMan.fgDumpFlow) {
				DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

				mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
				rOperInfo.pvSptHdl = prPsrCC->pvSptHdl;
				rOperInfo.unFlow.rFillAU.u4AUIdx = u4AUIdx;
				rOperInfo.unFlow.rFillAU.u4StmType = SPT_DATA_V;
				DmxDumpFlow(DMX_OPER_FILLAU, &rOperInfo);
			}

			SetVCodec(u4VType, prVFSD->eVCodeC);
			prVPicAU->rAUInfo.rInfo.u4VType |= u4VType;

#if !DMX_DISABLE_FILL_AUINFO
			PSR_CC_CBSplitter(prPsrCC, E_GET_AU_INFO, (void *) &rAU);
#endif

			if (INVALID_TIMESTAMP != prVPicAU->rAUInfo.rInfo.u8Pts) {
				prPsrFtr->u8LastPTS = prVPicAU->rAUInfo.rInfo.u8Pts;
#ifdef MM_SUPPORT_DIVXHT31
				prPsrFtr->u8PrevPTS = prPsrFtr->u8LastPTS;
				if (INVALID_TIMESTAMP == prPsrFtr->u81stPTS) {
					DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("[PSR] %s Line %d --- (Video) set 1stPts ")
						TEXT("to be "DMX_PTS_LOGSTR"\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS),
						DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS));
					prPsrFtr->u81stPTS = prPsrFtr->u8LastPTS;
				}
#endif				/* MM_SUPPORT_DIVXHT31 */
			}

			if (u4VType & (SEQ_HDR | GOP_HDR)) {
				if (INVALID_TIMESTAMP != prVPicAU->rAUInfo.rInfo.u8Pts)
					prPsrFtr->u8HdrPTS = prVPicAU->rAUInfo.rInfo.u8Pts;
				else
					prVPicAU->rAUInfo.rInfo.u8Pts = prPsrFtr->u8HdrPTS;

			} else {
				prPsrFtr->u8HdrPTS = INVALID_TIMESTAMP;
			}
			mrRet =
					ESM_AUTableGetPrevAUIdx(prPsrFtr->u4ESIH, u4AUIdx, 1,
								&u4PrevAUIdx);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] %s line %d fail in ESM_AUTableGetPrevAUIdx")
					TEXT("(auidx: %d, Count: %d), mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx, 1, mrRet);
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}

			if (DMX_INVALID_UINT32 != u4PrevAUIdx) {
				mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH,
						 u4PrevAUIdx, (void **) &prPrevVPicAU);
				if (DMX_FAILED(mrRet)) {
					DMX_ASSERT(FALSE);
					MM_RETURN(mrRet);
				}
				/*fix klocwork bug */
				if (NULL == prPrevVPicAU) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("[PSR] %s line %d fail for prPrevVPicAU = NULL!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_UNEXPECT);
				}

				if (INVALID_TIMESTAMP ==
						prVPicAU->rAUInfo.rInfo.u8PrevPTS) {
					/*For File Playback PTS control */
					prPrevVPicAU->rAUInfo.rInfo.u8Pts =
							INVALID_TIMESTAMP;
				}

				if (prVPicAU->rAUInfo.rInfo.u4PrevDuration != 0) {
					prPrevVPicAU->rAUInfo.rInfo.u4Duration =
							prVPicAU->rAUInfo.rInfo.u4PrevDuration;
				}
			}
			SetVCodec(u4VType, prVFSD->eVCodeC);
			prVPicAU->rAUInfo.rInfo.u4VType |= u4VType;
			prVFSD->fgHasSetPics = TRUE;
			u4Idx++;
			continue;
		}

		prVFSD->fgHasSetPics = FALSE;
		/* Actual setting the end address of the prev AU */
		if (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount) {
			prVPicAU->rAUInfo.rInfo.ptrEAddr =
					prVFSD->rPicDetResult.ptrPicAddr[u4Idx];
		} else {
			prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;
		}

		if (prPsrFtr->fgFirstAUInRng) {
			/* Use the flag to designated whether the au is the first au in the cfa range */
			prVPicAU->fgIBCSent = TRUE;
			prPsrFtr->fgFirstAUInRng = FALSE;
		}

		if (MM_IS_RW_PLAY(SplitterGetPlayRate(prPsrCC->pvSptHdl))) {
			if (fgIsIType(prVPicAU->rAUInfo.rInfo.u4VType)) {
				if (0 == prPsrFtr->u4AUCntFromIFrm)
					prPsrFtr->u4AUCntFromIFrm = 1;
				else
					prPsrFtr->u4AUCntFromIFrm++;

				prPsrFtr->u4IFrmCnt++;
			} else if (0 < prPsrFtr->u4AUCntFromIFrm) {
				prPsrFtr->u4AUCntFromIFrm++;
			}

			if (prPsrFtr->u4AUCntFromIFrm > MAX_VID_AU_CNT_IN_CTRL_RW) {
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] %s -- RW Ctrl Need JUMP!\r\n"),
					DMX_FUNC_NAME);
				mrRet = ESM_AUTableIncWrIdx(prPsrFtr->u4ESIH, 1);
				if (DMX_FAILED(mrRet))
					MM_RETURN(mrRet);

				prVFSD->fgUseDummyAURealWrIdx = FALSE;
				prVFSD->fgUseRealWrIdx = FALSE;
				prVFSD->u4VType = 0;
				PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
				MM_RETURN(RET_DMX_NEED_JUMP);
			}
		}

		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("MPEG2: Create AU(Idx:%d) -- VType: 0x%x, SA: 0x%p")
			 TEXT(", EA: 0x%p, Offset: "DMX_UINT64_10U_LOGSTR)
			 TEXT(", Pts: "DMX_PTS_LOGSTR"\r\n"),
			u4AUIdx,
			prVPicAU->rAUInfo.rInfo.u4VType,
			(void *)prVPicAU->rAUInfo.rInfo.ptrSAddr,
			(void *)prVPicAU->rAUInfo.rInfo.ptrEAddr,
			DMX_UINT64_10U_LOG(prVPicAU->rAUInfo.rInfo.u8Offset),
			DMX_PTS_LOG_MS(prVPicAU->rAUInfo.rInfo.u8Pts),
			DMX_PTS_LOG_PTS(prVPicAU->rAUInfo.rInfo.u8Pts));

		mrRet = ESM_AUTableGetNextAUIdx(prPsrFtr->u4ESIH, u4AUIdx, 1, &u4AUIdx);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail in ESM_AUTableGetNextAUIdx")
				TEXT("(auidx: %d, Count: %d), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx, 1, mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		mrRet =	ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4AUIdx,
			(void **) &prVPicAU);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		/*fix klocwork bug */
		if (NULL == prVPicAU) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for prVPicAU = NULL!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		dmx_memset(prVPicAU, 0, sizeof(AU_VPic));

		if (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount)
			prVPicAU->rAUInfo.rInfo.ptrSAddr =
					prVFSD->rPicDetResult.ptrPicAddr[u4Idx];
		else
			prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrHWCurWp;

		prVPicAU->eAuType = AU_DATA;

		u4CurAUWrIdx = u4AUIdx;

		u4VType = 0;

	}
	/* Update ESI AU write index */
	/* Because we need confirm all data had into VFifo if Decoder get a picture, */
	/* so that, the last picture information maybe filled into AU table, but can not be */
	/* get by decoder.  */
	/* We need to update write index to real write index when we got sequence end or destory. */
	/* please delete these bleve notes up */
	if (u4Idx >= prVFSD->rPicDetResult.u1PicInfoCount) {
		u4AUIdx = u4CurAUWrIdx;
		if (u4AUIdx >= u4TotalAUCnt) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for u4AUIdx")
				TEXT("(%d) > u4TotalAUCnt(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx, u4TotalAUCnt);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		mrRet =
				ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4AUIdx, (void **) &prVPicAU);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		/*fix klocwork bug */
		if (NULL == prVPicAU) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d failed for prVPicAU = NULL!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		if (0 != prVPicAU->rAUInfo.rInfo.u4VType) {
			prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;
		} else {
			if (0 == prVPicAU->rAUInfo.rInfo.ptrSAddr)
				prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrHWCurWp;
			if (0 == prVPicAU->rAUInfo.rInfo.ptrEAddr)
				prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;
		}

		if (prPsrFtr->fgFirstAUInRng) {
			/* Use the flag to designated whether the au is the first au in the cfa range */
			prVPicAU->fgIBCSent = TRUE;
			prPsrFtr->fgFirstAUInRng = FALSE;
		}

		prVFSD->u4RealWrIdx = u4AUIdx;
		prVFSD->fgUseRealWrIdx = TRUE;
		/* get AU */
		mrRet = ESM_AUTableSetWrIdx(prPsrFtr->u4ESIH, u4AUIdx);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
	} else {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d failed for u4Idx(%d) ")
			TEXT("!= prVFSD->rPicDetResult.u1PicInfoCount(%d)!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
			prVFSD->rPicDetResult.u1PicInfoCount);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_VFilter_UpdateESIInfo_H264(PSR_FILTER *prPsrFtr,
	u32 u4CurAUWrIdx, AU_VPic *prVPicAU,
	uintptr_t ptrHWCurWp, uintptr_t ptrCurFIFOWp)
{
	PSR_AU rAU;
	PSR_CC *prPsrCC = NULL;
	PSR_VFSD *prVFSD = NULL;
	AU_VPic *prPrevVPicAU = NULL;
	u32 u4AUIdx;
	u32 u4Idx;
	u32 u4EndVIdx = DMX_INVALID_UINT32;
	u32 u4TotalAUCnt = 0;
	u32 u4VType = 0;
	MRESULT mrRet = RET_DMX_OK;

	DMX_ASSERT(NULL != prPsrFtr);

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);
	DMX_ASSERT(NULL != prPsrCC);

	prVFSD = (PSR_VFSD *) prPsrFtr->pvFilterSpecific;
	DMX_ASSERT(NULL != prVFSD);

	mrRet = ESM_AUTableGetTotalCount(prPsrFtr->u4ESIH, &u4TotalAUCnt);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail in ESM_AUTableGetTotalCount, ")
			TEXT("mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (u4CurAUWrIdx >= u4TotalAUCnt) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail for u4CurAUWrIdx(%d)")
			TEXT(" > u4TotalAUCnt(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx, u4TotalAUCnt);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
		TEXT("[ESM][H264] %s line %d -- u1PicInfoCount: %d, u4CurAUWrIdx: %d\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		prVFSD->rPicDetResult.u1PicInfoCount, u4CurAUWrIdx);

	u4Idx = 0;

	while (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount) {
		if (u4Idx >= DMX_MAX_VID_STARTCODE_CNT) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		if ((prVFSD->rPicDetResult.ptrPicAddr[u4Idx] >= prPsrFtr->ptrESFifoEa) ||
				(prVFSD->rPicDetResult.ptrPicAddr[u4Idx] < prPsrFtr->ptrESFifoSa)) {
			DMX_ASSERT(FALSE);
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for PicDetResultIdx(%d)'s PicAddr(0x%lx) ")
				TEXT("exceed fifo Range[0x%lx, 0x%lx)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
				prVFSD->rPicDetResult.ptrPicAddr[u4Idx],
				prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		/* get AU */
		u4AUIdx = u4CurAUWrIdx;
		if (u4CurAUWrIdx >= u4TotalAUCnt) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for u4CurAUWrIdx(%d) > ")
				TEXT("u4TotalAUCnt(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx, u4TotalAUCnt);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4AUIdx,
							 (void **) &prVPicAU);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		/*fix klocwork bug */
		if (NULL == prVPicAU) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for prVPicAU = NULL!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		if (AU_DATA == prVPicAU->eAuType)
			u4VType = prVPicAU->rAUInfo.rInfo.u4VType;
		else
			u4VType = 0;

		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[ESM][H264] %s line %d -- PicIdx[%d], PicType: %u\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
			prVFSD->rPicDetResult.u1PicType[u4Idx]);

		if (0 == DMX_GET_PICTYPE(u4VType)) {
			if ((PSR_HDR_AVC_SeqEnd == prVFSD->rPicDetResult.u1PicType[u4Idx])
					|| (PSR_HDR_AVC_FILTER ==
				prVFSD->rPicDetResult.u1PicType[u4Idx])
					|| (PSR_HDR_AVC_STMEND ==
				prVFSD->rPicDetResult.u1PicType[u4Idx])) {
				/* may be encounter the following cause : */
				/* seqend1 seqend2 seqend3 frame */
				/* seqend2 to seqend3 should be skip, so do this. */
				u4Idx++;
				if (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount) {
					prVPicAU->rAUInfo.rInfo.ptrSAddr =
							prVFSD->rPicDetResult.ptrPicAddr[u4Idx];
				} else {
					prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrHWCurWp;
				}
				continue;
			}

			mrRet = PSR_VFilter_SetFileOfstInAU(prPsrFtr, prVPicAU,
				ptrCurFIFOWp, u4Idx);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] %s line %d fail in PSR_VFilter_SetFileOfstInAU!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				MM_RETURN(mrRet);
			}

			if (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount) {
				prVPicAU->rAUInfo.rInfo.ptrSAddr =
						prVFSD->rPicDetResult.ptrPicAddr[u4Idx];
			} else {
				prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrHWCurWp;
			}
		}

		switch (prVFSD->rPicDetResult.u1PicType[u4Idx]) {
		case PSR_HDR_AVC_SeqPar:{
				/*SPS, 0x08 --> 1 */
				if (!prVFSD->fgHasSetPics) {
					u4VType |= SEQ_PS;
					prVPicAU->rAUInfo.rInfo.u4VType |= SEQ_PS;
					u4Idx++;
					continue;
				}
			}
			break;

		case PSR_HDR_AVC_PicParam:{
				/*PPS, 0x10 --> E0 --> 3 */
				if (!prVFSD->fgHasSetPics) {
					u4VType |= PIC_PS;
					prVPicAU->rAUInfo.rInfo.u4VType |= PIC_PS;
					u4Idx++;
					continue;
				}
			}
			break;

		case PSR_HDR_AVC_SEI:{
				/*SEI, 0x04 --> 0 */
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[ESM][H264] %s line %d -- PicIdx[%d], SEI\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Idx);
				if (!prVFSD->fgHasSetPics) {
					/* Modify according to the h264 composing au spec */
					u4VType |= SEI;
					prVPicAU->rAUInfo.rInfo.u4VType |= SEI;
					u4Idx++;
					continue;
				}
			}
			break;

		case PSR_HDR_AVC_AUD:{
				/* 0x11 --> E0 --> 3 */
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[ESM][H264] %s line %d -- PicIdx[%d], AUD\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Idx);
				if (!prVFSD->fgHasSetPics) {
					u4Idx++;
					continue;
				}
			}
			break;

		case PSR_HDR_AVC_SeqEnd:{
				/*SEQ_END, 0x1E --> 2 */
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[ESM][H264] %s line %d -- PicIdx[%d], SEQ_END\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Idx);
				if (!prVFSD->fgHasSetPics) {
					u4Idx++;
					continue;
				} else {
					u4VType |= SEQ_END;
					prVPicAU->rAUInfo.rInfo.u4VType |= SEQ_END;
					u4Idx++;
					break;
				}
			}
			break;

		case PSR_HDR_AVC_FILTER:
		case PSR_HDR_AVC_STMEND:{
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[ESM][H264] %s line %d -- PicIdx[%d], AVC_FILTER ")
					TEXT("or AVC_STMEND\r\n"), DMX_FUNC_NAME,
					DMX_LINE_NO, u4Idx);
				if (!prVFSD->fgHasSetPics) {
					u4Idx++;
					continue;
				} else {
					u4Idx++;
					break;
				}
			}
			break;
		case PSR_HDR_AVC_IDR:
		case PSR_HDR_AVC_NONIDR:{
				/* IDR And NON_IDR, 0x02 --> E1 --> 4, 0x01 --> E1 --> 4 */
				/*Parsing Picture Type */
				u8 b;
				u32 u4PicType = 0;
				u8 *pb =
						(u8 *) (prVFSD->rPicDetResult.ptrPicAddr[u4Idx]) + 3;

				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[ESM][H264] %s line %d -- PicIdx[%d], IDR or NON_IDR\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Idx);

				if ((uintptr_t) pb >= prPsrFtr->ptrESFifoEa)
					pb -= prPsrFtr->u4ESFifoSize;

				b = *pb;

				if (PSR_HDR_AVC_IDR ==
						prVFSD->rPicDetResult.u1PicType[u4Idx]) {
					u4PicType |= IDR_PIC;
				} else {
					if ((b & 0x60) != 0)
						u4PicType |= REF_PIC;
				}

				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[TsSpt_ESM]: Idx(%d), IDR or Non-IDR: 0x%x, ")
					TEXT(" pb: 0x%x, pb+1 : 0x%x!\r\n"),
					u4Idx, prVFSD->rPicDetResult.u1PicType[u4Idx],
					*pb, *(pb + 1));

				pb++;

				if ((uintptr_t) pb >= prPsrFtr->ptrESFifoEa)
					pb -= prPsrFtr->u4ESFifoSize;

				b = *pb;

				if ((b & 0x80) != 0) {	/* first_mb_in_slice */
					if ((b & 0x40) == 0x40)
						u4PicType |= P_SLICE;
					else if ((b & 0x70) == 0x20)
						u4PicType |= B_SLICE;
					else if ((b & 0x70) == 0x30)
						u4PicType |= I_SLICE;
					else if ((b & 0x7C) == 0x10)
						u4PicType |= SP_SLICE;
					else if ((b & 0x7C) == 0x14)
						u4PicType |= SI_SLICE;
					else if ((b & 0x7C) == 0x18)
						u4PicType |= P_ALL_SLICE;
					else if ((b & 0x7C) == 0x1C)
						u4PicType |= B_ALL_SLICE;
					else if ((b & 0x7F) == 0x08)
						u4PicType |= I_ALL_SLICE;
					else if ((b & 0x7F) == 0x09)
						u4PicType |= SP_ALL_SLICE;
					else if ((b & 0x7F) == 0x0A)
						u4PicType |= SI_ALL_SLICE;
					if (!prVFSD->fgHasSetPics)
						u4VType |= u4PicType;
				} else {
					u4VType |= (u4PicType | MULTISLICE_PIC);
					u4Idx++;
					continue;
				}
			}
			break;

		case PSR_HDR_AVC_MVC:	/*Not Used */
			{
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[ESM][H264] %s line %d -- PicIdx[%d], AVC_MVC\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Idx);

				/*Skip This Information Now. (Only for MVC) */
				u4Idx++;
				continue;
			}
			break;

		default:{
				/* for 127293, error handle, no assert for error data */
				/* (maybe disc/loader problem) */
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] Idx(%d) -- UpdateH264ESI,")
					TEXT(" error, pic type:0x%x\r\n"), u4Idx,
					prVFSD->rPicDetResult.u1PicType[u4Idx]);
				u4Idx++;
				continue;
			}
			break;

		}

		if (!prVFSD->fgHasSetPics) {
			u32 u4PrevAUIdx = ESM_INVALID_INDEX;
			/* write ESI AU information */
			prVPicAU->eAuType = AU_DATA;

			/* callback notify Splitter to fill AU information */
			mm_memset(&rAU, 0, sizeof(PSR_AU));
			rAU.eType = prPsrFtr->eType;
			rAU.pvAUInf = prVPicAU;
			rAU.pvAUExtInf = NULL;

			/*Init Previous PTS */
			prVPicAU->rAUInfo.rInfo.u4Duration = 0;
			prVPicAU->rAUInfo.rInfo.u4PrevDuration = 0;
			prVPicAU->rAUInfo.rInfo.u8PrevPTS = 0;

			SetVCodec(u4VType, prVFSD->eVCodeC);

			prVPicAU->rAUInfo.rInfo.u4VType |= u4VType;

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

			if (INVALID_TIMESTAMP != prVPicAU->rAUInfo.rInfo.u8Pts) {
				prPsrFtr->u8LastPTS = prVPicAU->rAUInfo.rInfo.u8Pts;
#ifdef MM_SUPPORT_DIVXHT31
				prPsrFtr->u8PrevPTS = prPsrFtr->u8LastPTS;
				if (INVALID_TIMESTAMP == prPsrFtr->u81stPTS) {
					DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("[PSR] %s Line %d --- (Video)")
						 TEXT(" set 1stPts to be "DMX_PTS_LOGSTR"\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS),
						DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS));
					prPsrFtr->u81stPTS = prPsrFtr->u8LastPTS;
				}
#endif				/* MM_SUPPORT_DIVXHT31 */
			}

			mrRet =
					ESM_AUTableGetPrevAUIdx(prPsrFtr->u4ESIH, u4AUIdx, 1,
								&u4PrevAUIdx);
			if (DMX_FAILED(mrRet)) {
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}

			if (ESM_INVALID_INDEX != u4PrevAUIdx) {
				mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4PrevAUIdx,
									 (void **) &prPrevVPicAU);
				if (DMX_FAILED(mrRet)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("[PSR] %s line %d -- ESM_AUTableGetAUInfo Failed!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
					DMX_ASSERT(FALSE);
					MM_RETURN(mrRet);
				}
				/*fix klocwork bug */
				if (NULL == prPrevVPicAU) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("[PSR] %s line %d -- prPrevVPicAU = NULL Failed!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_UNEXPECT);
				}

				if (INVALID_TIMESTAMP == prVPicAU->rAUInfo.rInfo.u8PrevPTS) {
					/*For File Playback PTS control */
					prPrevVPicAU->rAUInfo.rInfo.u8Pts =
							INVALID_TIMESTAMP;
				}

				if (prVPicAU->rAUInfo.rInfo.u4PrevDuration != 0) {
					prPrevVPicAU->rAUInfo.rInfo.u4Duration =
							prVPicAU->rAUInfo.rInfo.u4PrevDuration;
				}
			}

			prVFSD->fgHasSetPics = TRUE;

			u4Idx++;

			continue;
		}

		prVFSD->fgHasSetPics = FALSE;
		if (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount)
			prVPicAU->rAUInfo.rInfo.ptrEAddr =
					prVFSD->rPicDetResult.ptrPicAddr[u4Idx];
		else
			prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;

		if (prPsrFtr->fgFirstAUInRng) {
			/* Use the flag to designated whether the au is the first au in the cfa range */
			prVPicAU->fgIBCSent = TRUE;
			prPsrFtr->fgFirstAUInRng = FALSE;
		}

		if (MM_IS_RW_PLAY(SplitterGetPlayRate(prPsrCC->pvSptHdl))) {
			if (fgIsIType(prVPicAU->rAUInfo.rInfo.u4VType)) {
				if (0 == prPsrFtr->u4AUCntFromIFrm)
					prPsrFtr->u4AUCntFromIFrm = 1;
				else
					prPsrFtr->u4AUCntFromIFrm++;

				prPsrFtr->u4IFrmCnt++;
			} else if (0 < prPsrFtr->u4AUCntFromIFrm) {
				prPsrFtr->u4AUCntFromIFrm++;
			}

			if (prPsrFtr->u4AUCntFromIFrm > MAX_VID_AU_CNT_IN_CTRL_RW) {
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] %s -- RW Ctrl Need JUMP!\r\n"),
					DMX_FUNC_NAME);
				mrRet = ESM_AUTableIncWrIdx(prPsrFtr->u4ESIH, 1);
				if (DMX_FAILED(mrRet))
					MM_RETURN(mrRet);
				prVFSD->fgUseDummyAURealWrIdx = FALSE;
				prVFSD->fgUseRealWrIdx = FALSE;
				prVFSD->u4VType = 0;
				PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
				MM_RETURN(RET_DMX_NEED_JUMP);
			}
		}

		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("H264: Create AU(Idx:%d) -- VType: 0x%x, SA: 0x%x")
			 TEXT(", EA: 0x%x, Offset: "DMX_UINT64_10U_LOGSTR)
			 TEXT(", Pts: "DMX_PTS_LOGSTR"\r\n"),
			u4AUIdx,
			prVPicAU->rAUInfo.rInfo.u4VType,
			prVPicAU->rAUInfo.rInfo.ptrSAddr,
			prVPicAU->rAUInfo.rInfo.ptrEAddr,
			DMX_UINT64_10U_LOG(prVPicAU->rAUInfo.rInfo.u8Offset),
			DMX_PTS_LOG_MS(prVPicAU->rAUInfo.rInfo.u8Pts),
			DMX_PTS_LOG_PTS(prVPicAU->rAUInfo.rInfo.u8Pts));

		mrRet = ESM_AUTableGetNextAUIdx(prPsrFtr->u4ESIH, u4AUIdx, 1, &u4AUIdx);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail in ESM_AUTableGetNextAUIdx")
				TEXT("(AUIdx: %d, Count: %d), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx, 1, mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		if (u4AUIdx >= u4TotalAUCnt) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for u4AUIdx(%d)")
				TEXT(" > u4TotalAUCnt(%d)\r\n"), DMX_FUNC_NAME,
				DMX_LINE_NO, u4AUIdx, u4TotalAUCnt);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		mrRet =
				ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4AUIdx, (void **) &prVPicAU);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		dmx_memset(prVPicAU, 0, sizeof(AU_VPic));

		if (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount)
			prVPicAU->rAUInfo.rInfo.ptrSAddr =
					prVFSD->rPicDetResult.ptrPicAddr[u4Idx];
		else
			prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrHWCurWp;

		prVPicAU->eAuType = AU_DATA;

		u4EndVIdx = u4Idx;

		u4CurAUWrIdx = u4AUIdx;

		u4VType = 0;
	}

	if (u4Idx == prVFSD->rPicDetResult.u1PicInfoCount) {
		u4AUIdx = u4CurAUWrIdx;

		if (u4AUIdx >= u4TotalAUCnt) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for u4AUIdx(%d) > u4TotalAUCnt(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx, u4TotalAUCnt);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		mrRet =
				ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4AUIdx, (void **) &prVPicAU);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		/*fix klocwork bug */
		if (NULL == prVPicAU) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d failed for prVPicAU = NULL!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		prVPicAU->eAuType = AU_DATA;

		SetVCodec(u4VType, prVFSD->eVCodeC);

		prVPicAU->rAUInfo.rInfo.u4VType |= u4VType;

		if (0 != prVPicAU->rAUInfo.rInfo.u4VType) {
			prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;
		} else {
			if (0 == prVPicAU->rAUInfo.rInfo.ptrSAddr)
				prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrHWCurWp;

			if (0 == prVPicAU->rAUInfo.rInfo.ptrEAddr)
				prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;
		}

		if (prPsrFtr->fgFirstAUInRng) {
			/* Use the flag to designated whether the au is the first au in the cfa range */
			prVPicAU->fgIBCSent = TRUE;
			prPsrFtr->fgFirstAUInRng = FALSE;
		}

		prVFSD->u4RealWrIdx = u4AUIdx;

		prVFSD->fgUseRealWrIdx = TRUE;
		/* get AU */
		mrRet = ESM_AUTableSetWrIdx(prPsrFtr->u4ESIH, u4AUIdx);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
	} else {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d failed for u4Idx(%d) ")
			TEXT("!= prVFSD->rPicDetResult.u1PicInfoCount(%d)!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
			prVFSD->rPicDetResult.u1PicInfoCount);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
		TEXT("[PSR] %s line %d exit -- fgUseRealWrIdx: %d, u4RealWrIdx: 0x%x!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		(prVFSD->fgUseRealWrIdx ? 1 : 0), prVFSD->u4RealWrIdx);

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_VFilter_UpdateESIInfo_VC1(PSR_FILTER *prPsrFtr,
	u32 u4CurAUWrIdx, AU_VPic *prVPicAU,
	uintptr_t ptrHWCurWp, uintptr_t ptrCurFIFOWp)
{
	PSR_AU rAU;
	PSR_CC *prPsrCC = NULL;
	PSR_VFSD *prVFSD = NULL;
	AU_VPic *prPrevVPicAU = NULL;
	u32 u4AUIdx;
	u32 u4Idx;
	u32 u4TotalAUCnt = 0;
	u32 u4VType = 0;
	MRESULT mrRet = RET_DMX_OK;

	DMX_ASSERT(NULL != prPsrFtr);

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);
	DMX_ASSERT(NULL != prPsrCC);

	prVFSD = (PSR_VFSD *) prPsrFtr->pvFilterSpecific;
	DMX_ASSERT(NULL != prVFSD);

	mrRet = ESM_AUTableGetTotalCount(prPsrFtr->u4ESIH, &u4TotalAUCnt);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail in ESM_AUTableGetTotalCount")
			TEXT(", mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (u4CurAUWrIdx >= u4TotalAUCnt) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail for u4CurAUWrIdx(%d)")
			TEXT(" > u4TotalAUCnt(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx, u4TotalAUCnt);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	u4Idx = 0;
	while (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount) {
		if (u4Idx >= DMX_MAX_VID_STARTCODE_CNT) {
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_OVER_LIMIT);
		}

		if ((prVFSD->rPicDetResult.ptrPicAddr[u4Idx] >= prPsrFtr->ptrESFifoEa) ||
				(prVFSD->rPicDetResult.ptrPicAddr[u4Idx] < prPsrFtr->ptrESFifoSa)) {
			DMX_ASSERT(FALSE);
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for PicDetResultIdx(%d)'s ")
				TEXT("PicAddr(0x%lx) exceed fifo Range[0x%lx, 0x%lx)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
				prVFSD->rPicDetResult.ptrPicAddr[u4Idx],
				prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		/* get AU */
		u4AUIdx = u4CurAUWrIdx;
		if (u4AUIdx >= u4TotalAUCnt) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for u4AUIdx(%d)")
				TEXT(" > u4TotalAUCnt(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx, u4TotalAUCnt);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		mrRet =
				ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4AUIdx, (void **) &prVPicAU);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		/*fix klocwork bug */
		if (NULL == prVPicAU) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for prVPicAU = NULL!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		if (AU_DATA == prVPicAU->eAuType)
			u4VType = prVPicAU->rAUInfo.rInfo.u4VType;
		else
			u4VType = 0;

		if (0 == DMX_GET_PICTYPE(u4VType)) {
			if (PSR_HDR_WMV_SeqEnd == prVFSD->rPicDetResult.u1PicType[u4Idx]) {
				/* may be encounter the following cause : */
				/* seqend1 seqend2 seqend3 frame */
				/* seqend1 to seqend3 should be skip, so do this. */
				u4Idx++;
				continue;
			}

			mrRet = PSR_VFilter_SetFileOfstInAU(prPsrFtr, prVPicAU,
				ptrCurFIFOWp, u4Idx);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] %s line %d fail in PSR_VFilter_SetFileOfstInAU!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				MM_RETURN(mrRet);
			}

			prVPicAU->eAuType = AU_DATA;
			prVPicAU->rAUInfo.rInfo.u4VType = 0;

			/* Fill AU */
			if (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount)
				prVPicAU->rAUInfo.rInfo.ptrSAddr =
						prVFSD->rPicDetResult.ptrPicAddr[u4Idx];
			else
				prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrHWCurWp;
		}

		switch (prVFSD->rPicDetResult.u1PicType[u4Idx]) {
		case PSR_HDR_WMV_SeqHdr:
			if (!prVFSD->fgHasSetPics) {
				u8 *pb9;

				u4VType |= SEQ_HDR;
				prVPicAU->rAUInfo.rInfo.u4VType |= u4VType;

				pb9 = (u8 *) (prVFSD->rPicDetResult.ptrPicAddr[u4Idx]) + 9;
				if ((uintptr_t) pb9 >= prPsrFtr->ptrESFifoEa)
					pb9 -= prPsrFtr->u4ESFifoSize;
				prVFSD->fgWMVInterlace = ((*pb9 & 0x40) != 0);
				u4Idx++;
				continue;
			}
			break;
		case PSR_HDR_WMV_EntryPoint:
			/*if (!prVFSD->fgHasSetPics) */
			{
				u4VType |= ENTRY_PTR;
				prVPicAU->rAUInfo.rInfo.u4VType |= u4VType;
				u4Idx++;
				continue;
			}
			break;
		case PSR_HDR_WMV_SeqEnd:{
				u4VType |= SEQ_END;
				prVPicAU->rAUInfo.rInfo.u4VType = u4VType;
				u4Idx++;
				continue;
			}
			break;
		case PSR_HDR_WMV_FIELD:
			if (!prVFSD->fgHasSetPics) {
				u4VType |= prVFSD->u4WMVSecondFieldPicType;
				prVPicAU->rAUInfo.rInfo.u4VType |= u4VType;

				prVFSD->u4WMVSecondFieldPicType = 0;
			}
			break;
		case PSR_HDR_WMV_FRAME:
			if (!prVFSD->fgHasSetPics) {
				u8 uSetVTypeFlag = 1;
				u8 b;
				u8 *pb4;

				pb4 = (u8 *) (prVFSD->rPicDetResult.ptrPicAddr[u4Idx]) + 4;
				if ((uintptr_t) pb4 >= prPsrFtr->ptrESFifoEa)
					pb4 -= prPsrFtr->u4ESFifoSize;

				b = *pb4;

				if (prVFSD->fgWMVInterlace) {
					if ((b & 0x80) == 0x00)
						b <<= 1;
					else if ((b & 0xC0) == 0x80)
						b <<= 2;
					else {
						static const u32 au4PictureType[4] = {
							IVOP, PVOP, BVOP, BIVOP
						};
						u4VType |= au4PictureType[(b >> 4) & 3];

						prVFSD->u4WMVSecondFieldPicType =
								au4PictureType[((b >> 3) & 1) |
									 ((b >> 4) & 2)];
						uSetVTypeFlag = 0;
					}
				}

				if (uSetVTypeFlag) {
					if ((b & 0x80) == 0x00)
						u4VType |= PVOP;
					else if ((b & 0xC0) == 0x80)
						u4VType |= BVOP;
					else if ((b & 0xE0) == 0xC0)
						u4VType |= IVOP;
					else if ((b & 0xF0) == 0xE0)
						u4VType |= BIVOP;
					else if ((b & 0xF0) == 0xF0)
						u4VType |= SKIPFRAME;
				}

				prVPicAU->rAUInfo.rInfo.u4VType |= u4VType;

			}
			break;
		case PSR_HDR_WMV_USRDAAT:{
				u4Idx++;
				continue;
			}
			break;
		case PSR_HDR_WMV_SLICE:{
				u16 u2SliceAddr;
				u8 *ucBuf;

				prVPicAU->rAUInfo.rInfo.u4VType |= MULTISLICE_PIC;
				ucBuf =
						(u8 *) (prVFSD->rPicDetResult.ptrPicAddr[u4Idx] + 2);
				if ((uintptr_t) ucBuf >= prPsrFtr->ptrESFifoEa)
					ucBuf -= prPsrFtr->u4ESFifoSize;

				u2SliceAddr = (ucBuf[0] << 1) | ((ucBuf[1] & 0x80) >> 7);
				if (u2SliceAddr < 32)
					prVPicAU->rAUInfo.rInfo.u4WMVSliceAddr[0] |=
							(0x1 << u2SliceAddr);
				else if (u2SliceAddr < 64)
					prVPicAU->rAUInfo.rInfo.u4WMVSliceAddr[1] |=
							(0x1 << (u2SliceAddr - 32));
				else
					prVPicAU->rAUInfo.rInfo.u4WMVSliceAddr[2] |=
							(0x1 << (u2SliceAddr - 64));

				u4Idx++;
				continue;
			}
			break;
		default:{
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] Idx(%d) -- UpdateVC1ESI, error, pic type:0x%x\r\n"),
				u4Idx, prVFSD->rPicDetResult.u1PicType[u4Idx]);
				u4Idx++;
				continue;
			}
			break;
		}

		if (!prVFSD->fgHasSetPics) {	/*before getting a Picture */
			u32 u4PrevAUIdx = ESM_INVALID_INDEX;
			/* write ESI AU information */
			prVPicAU->eAuType = AU_DATA;

			/* callback notify Splitter to fill AU information */
			mm_memset(&rAU, 0, sizeof(PSR_AU));
			rAU.eType = prPsrFtr->eType;
			rAU.pvAUInf = prVPicAU;
			rAU.pvAUExtInf = NULL;

			/*Init Previous PTS */
			prVPicAU->rAUInfo.rInfo.u4Duration = 0;
			prVPicAU->rAUInfo.rInfo.u4PrevDuration = 0;
			prVPicAU->rAUInfo.rInfo.u8PrevPTS = 0x0;

			SetVCodec(u4VType, prVFSD->eVCodeC);
			prVPicAU->rAUInfo.rInfo.u4VType |= u4VType;

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

			if (INVALID_TIMESTAMP != prVPicAU->rAUInfo.rInfo.u8Pts) {
				prPsrFtr->u8LastPTS = prVPicAU->rAUInfo.rInfo.u8Pts;
#ifdef MM_SUPPORT_DIVXHT31
				prPsrFtr->u8PrevPTS = prPsrFtr->u8LastPTS;
				if (INVALID_TIMESTAMP == prPsrFtr->u81stPTS) {
					DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("[PSR] %s Line %d --- (Video)")
						 TEXT(" set 1stPts to be "DMX_PTS_LOGSTR"\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS),
						DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS));
					prPsrFtr->u81stPTS = prPsrFtr->u8LastPTS;
				}
#endif				/* MM_SUPPORT_DIVXHT31 */
			}

			mrRet =
					ESM_AUTableGetPrevAUIdx(prPsrFtr->u4ESIH, u4AUIdx, 1,
								&u4PrevAUIdx);
			if (DMX_FAILED(mrRet)) {
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}

			if (ESM_INVALID_INDEX != u4PrevAUIdx) {
				mrRet =
						ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH,
							 u4PrevAUIdx,
							 (void **) &prPrevVPicAU);
				if (DMX_FAILED(mrRet)) {
					DMX_ASSERT(FALSE);
					MM_RETURN(mrRet);
				}
				/*fix klocwork bug */
				if (NULL == prPrevVPicAU) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("[PSR] %s line %d failed for prPrevVPicAU = NULL!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_UNEXPECT);
				}

				if (INVALID_TIMESTAMP ==
						prVPicAU->rAUInfo.rInfo.u8PrevPTS) {
					/*For File Playback PTS control */
					prPrevVPicAU->rAUInfo.rInfo.u8Pts =
							INVALID_TIMESTAMP;
				}
				if (prVPicAU->rAUInfo.rInfo.u4PrevDuration != 0) {
					prPrevVPicAU->rAUInfo.rInfo.u4Duration =
							prVPicAU->rAUInfo.rInfo.u4PrevDuration;
				}
			}

			prVFSD->fgHasSetPics = TRUE;
			u4Idx++;
			continue;
		}
		/*PrePare for the nest AU...... */
		prVFSD->fgHasSetPics = FALSE;
		/* Actual setting the end address of the prev AU */
		if (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount)
			prVPicAU->rAUInfo.rInfo.ptrEAddr =
					prVFSD->rPicDetResult.ptrPicAddr[u4Idx];
		else
			prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;

		if (prPsrFtr->fgFirstAUInRng) {
			/* Use the flag to designated whether the au is the first au in the cfa range */
			prVPicAU->fgIBCSent = TRUE;
			prPsrFtr->fgFirstAUInRng = FALSE;
		}

		if (MM_IS_RW_PLAY(SplitterGetPlayRate(prPsrCC->pvSptHdl))) {
			if (fgIsIType(prVPicAU->rAUInfo.rInfo.u4VType)) {
				if (0 == prPsrFtr->u4AUCntFromIFrm)
					prPsrFtr->u4AUCntFromIFrm = 1;
				else
					prPsrFtr->u4AUCntFromIFrm++;

				prPsrFtr->u4IFrmCnt++;
			} else if (0 < prPsrFtr->u4AUCntFromIFrm) {
				prPsrFtr->u4AUCntFromIFrm++;
			}

			if (prPsrFtr->u4AUCntFromIFrm > MAX_VID_AU_CNT_IN_CTRL_RW) {
				DMXLOG_TRACE(
							TEXT("[PSR] %s -- RW Ctrl Need JUMP!\r\n"),
							DMX_FUNC_NAME);
				mrRet = ESM_AUTableIncWrIdx(prPsrFtr->u4ESIH, 1);
				if (DMX_FAILED(mrRet))
					MM_RETURN(mrRet);

				prVFSD->fgUseDummyAURealWrIdx = FALSE;
				prVFSD->fgUseRealWrIdx = FALSE;
				prVFSD->u4VType = 0;
				PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
				MM_RETURN(RET_DMX_NEED_JUMP);
			}
		}

		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("VC1: Create AU(Idx:%d) -- VType: 0x%x, SA: 0x%x")
			 TEXT(", EA: 0x%x, Offset: "DMX_UINT64_10U_LOGSTR)
			 TEXT(", Pts: "DMX_PTS_LOGSTR"\r\n"),
			u4AUIdx,
			prVPicAU->rAUInfo.rInfo.u4VType,
			prVPicAU->rAUInfo.rInfo.ptrSAddr,
			prVPicAU->rAUInfo.rInfo.ptrEAddr,
			DMX_UINT64_10U_LOG(prVPicAU->rAUInfo.rInfo.u8Offset),
			DMX_PTS_LOG_MS(prVPicAU->rAUInfo.rInfo.u8Pts),
			DMX_PTS_LOG_PTS(prVPicAU->rAUInfo.rInfo.u8Pts));

		mrRet = ESM_AUTableGetNextAUIdx(prPsrFtr->u4ESIH, u4AUIdx, 1, &u4AUIdx);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail in ESM_AUTableGetNextAUIdx")
				TEXT("(AUIdx: %d, Count: %d), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx, 1, mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		if (u4AUIdx >= u4TotalAUCnt) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for u4AUIdx(%d)")
				 TEXT(" > u4TotalAUCnt(%d)\r\n"), DMX_FUNC_NAME,
				DMX_LINE_NO, u4AUIdx, u4TotalAUCnt);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		mrRet =
				ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4AUIdx, (void **) &prVPicAU);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		/*fix klocwork bug */
		if (NULL == prVPicAU) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for prVPicAU = NULL!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		dmx_memset(prVPicAU, 0, sizeof(AU_VPic));

		if (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount)
			prVPicAU->rAUInfo.rInfo.ptrSAddr =
					prVFSD->rPicDetResult.ptrPicAddr[u4Idx];
		else
			prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrHWCurWp;

		prVPicAU->eAuType = AU_DATA;

		u4CurAUWrIdx = u4AUIdx;

		u4VType = 0;
	}
	/* Update ESI AU write index */
	/* Because we need confirm all data had into VFifo if Decoder get a picture, */
	/* so that, the last picture information maybe filled into AU table, but can not be get by decoder. */
	/* We need to update write index to real write index when we got sequence end or destory. */
	if (u4Idx == prVFSD->rPicDetResult.u1PicInfoCount) {
		u4AUIdx = u4CurAUWrIdx;
		mrRet =
				ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4AUIdx, (void **) &prVPicAU);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		/*fix klocwork bug */
		if (NULL == prVPicAU) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d failed for prVPicAU = NULL!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		if (0 != prVPicAU->rAUInfo.rInfo.u4VType) {
			prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;
		} else {
			if (0 == prVPicAU->rAUInfo.rInfo.ptrSAddr)
				prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrHWCurWp;

			if (0 == prVPicAU->rAUInfo.rInfo.ptrEAddr)
				prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;
		}

		if (prPsrFtr->fgFirstAUInRng) {
			/* Use the flag to designated whether the au is the first au in the cfa range */
			prVPicAU->fgIBCSent = TRUE;
			prPsrFtr->fgFirstAUInRng = FALSE;
		}

		prVFSD->u4RealWrIdx = u4AUIdx;
		prVFSD->fgUseRealWrIdx = TRUE;
		/* get AU */
		mrRet = ESM_AUTableSetWrIdx(prPsrFtr->u4ESIH, u4AUIdx);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
	} else {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d failed for u4Idx(%d")
			TEXT(" != prVFSD->rPicDetResult.u1PicInfoCount(%d)!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
			prVFSD->rPicDetResult.u1PicInfoCount);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_VFilter_UpdateESIInfo_MPEG4(PSR_FILTER *prPsrFtr,
	u32 u4CurAUWrIdx, AU_VPic *prVPicAU,
	uintptr_t ptrHWCurWp, uintptr_t ptrCurFIFOWp)
{
	PSR_AU rAU;
	PSR_CC *prPsrCC = NULL;
	PSR_VFSD *prVFSD = NULL;
	AU_VPic *prPrevVPicAU = NULL;
	u32 u4AUIdx;
	u32 u4Idx;
	u32 u4TotalAUCnt = 0;
	u32 u4VType = 0;
	MRESULT mrRet = RET_DMX_OK;

	DMX_ASSERT(NULL != prPsrFtr);

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);
	DMX_ASSERT(NULL != prPsrCC);

	prVFSD = (PSR_VFSD *) prPsrFtr->pvFilterSpecific;
	DMX_ASSERT(NULL != prVFSD);

	mrRet = ESM_AUTableGetTotalCount(prPsrFtr->u4ESIH, &u4TotalAUCnt);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail in ESM_AUTableGetTotalCount")
			TEXT(", mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (u4CurAUWrIdx >= u4TotalAUCnt) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail for u4CurAUWrIdx(%d)")
			TEXT(" > u4TotalAUCnt(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx, u4TotalAUCnt);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
		TEXT("[ESM][MPEG4] %s line %d -- u1PicInfoCount: %d, u4CurAUWrIdx: %d\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		prVFSD->rPicDetResult.u1PicInfoCount, u4CurAUWrIdx);

	u4Idx = 0;
	while (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount) {
		if (u4Idx >= DMX_MAX_VID_STARTCODE_CNT) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		if ((prVFSD->rPicDetResult.ptrPicAddr[u4Idx] >= prPsrFtr->ptrESFifoEa) ||
				(prVFSD->rPicDetResult.ptrPicAddr[u4Idx] < prPsrFtr->ptrESFifoSa)) {
			DMX_ASSERT(FALSE);
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for PicDetResultIdx(%d)'s ")
				TEXT("PicAddr(0x%lx) exceed fifo Range[0x%lx, 0x%lx)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
				prVFSD->rPicDetResult.ptrPicAddr[u4Idx],
				prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		if (u4CurAUWrIdx >= u4TotalAUCnt) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for u4CurAUWrIdx")
				TEXT("(%d) > u4TotalAUCnt(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx, u4TotalAUCnt);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		/* get AU */
		u4AUIdx = u4CurAUWrIdx;
		mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4AUIdx,
			(void **) &prVPicAU);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		/*fix klocwork bug */
		if (NULL == prVPicAU) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for prVPicAU = NULL!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		if (AU_DATA == prVPicAU->eAuType)
			u4VType = prVPicAU->rAUInfo.rInfo.u4VType;
		else
			u4VType = 0;

		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[ESM][MPEG4] %s line %d -- PicIdx[%d], PicType: %u\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
			prVFSD->rPicDetResult.u1PicType[u4Idx]);

		if (0 == DMX_GET_PICTYPE(u4VType)) {
			mrRet = PSR_VFilter_SetFileOfstInAU(prPsrFtr, prVPicAU,
				ptrCurFIFOWp, u4Idx);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] %s line %d fail in PSR_VFilter_SetFileOfstInAU!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				MM_RETURN(mrRet);
			}

			/* fill SAddr */
			if (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount) {
				prVPicAU->rAUInfo.rInfo.ptrSAddr =
						prVFSD->rPicDetResult.ptrPicAddr[u4Idx];
			} else {
				prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrHWCurWp;
			}
		}

		/* check whether visual_object_start_code (0x00 00 01 B5) */
		/* video_object_layer_start_code (0x00 00 01 20~2F) */
		/* group_of_vop_start_code (0x00 00 01 B3) */

		switch (prVFSD->rPicDetResult.u1PicType[u4Idx]) {
		case PSR_HDR_MP4_VISOBJ:
		case PSR_HDR_MP4_VIDOBJLAY:
		case PSR_HDR_MP4_VISOBJSEQ:
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[ESM][MPEG4] %s line %d -- PicIdx[%d], VISOBJ | VIDOBJLAY")
				 TEXT(" | VISOBJSEQ\r\n"), DMX_FUNC_NAME, DMX_LINE_NO,
				u4Idx);
			if (!prVFSD->fgHasSetPics) {
				u4VType |= SEQ_HDR;
				prVPicAU->rAUInfo.rInfo.u4VType |= SEQ_HDR;
				u4Idx++;
				continue;
			}
			break;
		case PSR_HDR_MP4_GOVOP:
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[ESM][MPEG4] %s line %d -- PicIdx[%d], GOVOP\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Idx);
			if (!prVFSD->fgHasSetPics) {
				u4VType |= GOP_HDR;
				prVPicAU->rAUInfo.rInfo.u4VType |= GOP_HDR;
				u4Idx++;
				continue;
			}
			break;
		case PSR_HDR_MP4_IVOP:
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[ESM][MPEG4] %s line %d -- PicIdx[%d], IVOP\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Idx);
			if (!prVFSD->fgHasSetPics)
				u4VType |= I_VOP;
			break;
		case PSR_HDR_MP4_PVOP:
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[ESM][MPEG4] %s line %d -- PicIdx[%d], PVOP\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Idx);
			if (!prVFSD->fgHasSetPics)
				u4VType |= P_VOP;
			break;
		case PSR_HDR_MP4_BVOP:
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[ESM][MPEG4] %s line %d -- PicIdx[%d], BVOP\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Idx);
			if (!prVFSD->fgHasSetPics)
				u4VType |= B_VOP;
			break;
		case PSR_HDR_MP4_SVOP:
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[ESM][MPEG4] %s line %d -- PicIdx[%d], SVOP\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Idx);
			if (!prVFSD->fgHasSetPics)
				u4VType |= S_VOP;
			break;
		case PSR_HDR_MP4_SHIVOP:
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[ESM][MPEG4] %s line %d -- PicIdx[%d], SHIVOP\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Idx);
			if (!prVFSD->fgHasSetPics)
				u4VType |= SH_I_VOP;
			break;
		case PSR_HDR_MP4_SHPVOP:
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[ESM][MPEG4] %s line %d -- PicIdx[%d], SHPVOP\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Idx);
			if (!prVFSD->fgHasSetPics)
				u4VType |= SH_P_VOP;
			break;
		default:{
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[ESM][MPEG4] Idx(%d) -- UpdateMPEG4ESI, error, ")
				TEXT("pic type:0x%x\r\n"),
				u4Idx, prVFSD->rPicDetResult.u1PicType[u4Idx]);
				u4Idx++;
				continue;
			}
			break;
		}

		if (!prVFSD->fgHasSetPics) {
			u32 u4PrevAUIdx = ESM_INVALID_INDEX;

			if (NULL == prVPicAU) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] %s line %d fail for prVPicAU = NULL!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}
			/* write ESI AU information */
			prVPicAU->eAuType = AU_DATA;

			/* callback notify Splitter to fill AU information */
			mm_memset(&rAU, 0, sizeof(PSR_AU));
			rAU.eType = prPsrFtr->eType;
			rAU.pvAUInf = prVPicAU;
			rAU.pvAUExtInf = NULL;

			/*Init Previous PTS */
			prVPicAU->rAUInfo.rInfo.u4Duration = 0;
			prVPicAU->rAUInfo.rInfo.u4PrevDuration = 0;
			prVPicAU->rAUInfo.rInfo.u8PrevPTS = 0x0;

			SetVCodec(u4VType, prVFSD->eVCodeC);
			prVPicAU->rAUInfo.rInfo.u4VType |= u4VType;

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

			if (INVALID_TIMESTAMP != prVPicAU->rAUInfo.rInfo.u8Pts) {
				prPsrFtr->u8LastPTS = prVPicAU->rAUInfo.rInfo.u8Pts;
#ifdef MM_SUPPORT_DIVXHT31
				prPsrFtr->u8PrevPTS = prPsrFtr->u8LastPTS;
				if (INVALID_TIMESTAMP == prPsrFtr->u81stPTS) {
					DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("[PSR] %s Line %d --- (Video)")
						 TEXT(" set 1stPts to be "DMX_PTS_LOGSTR"\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS),
						DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS));
					prPsrFtr->u81stPTS = prPsrFtr->u8LastPTS;
				}
#endif				/* MM_SUPPORT_DIVXHT31 */
			}
			mrRet =
					ESM_AUTableGetPrevAUIdx(prPsrFtr->u4ESIH, u4AUIdx, 1,
								&u4PrevAUIdx);
			if (DMX_FAILED(mrRet)) {
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}

			if (ESM_INVALID_INDEX != u4PrevAUIdx) {
				mrRet =
						ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH,
							 u4PrevAUIdx,
							 (void **) &prPrevVPicAU);
				if (DMX_FAILED(mrRet)) {
					DMX_ASSERT(FALSE);
					MM_RETURN(mrRet);
				}
				/*fix klocwork bug */
				if (NULL == prPrevVPicAU) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("[PSR] %s line %d prPrevVPicAU = NULL Failed!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_UNEXPECT);
				}

				if (INVALID_TIMESTAMP ==
						prVPicAU->rAUInfo.rInfo.u8PrevPTS) {
					/*For File Playback PTS control */
					prPrevVPicAU->rAUInfo.rInfo.u8Pts =
							INVALID_TIMESTAMP;
				}
				if (prVPicAU->rAUInfo.rInfo.u4PrevDuration != 0) {
					prPrevVPicAU->rAUInfo.rInfo.u4Duration =
							prVPicAU->rAUInfo.rInfo.u4PrevDuration;
				}
			}

			prVFSD->fgHasSetPics = TRUE;
			u4Idx++;
			continue;
		}

		prVFSD->fgHasSetPics = FALSE;

		/* Actual setting the end address of the prev AU */
		if (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount)
			prVPicAU->rAUInfo.rInfo.ptrEAddr =
					prVFSD->rPicDetResult.ptrPicAddr[u4Idx];
		else
			prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;

		if (prPsrFtr->fgFirstAUInRng) {
			/* Use the flag to designated whether the au is the first au in the cfa range */
			prVPicAU->fgIBCSent = TRUE;
			prPsrFtr->fgFirstAUInRng = FALSE;
		}

		if (MM_IS_RW_PLAY(SplitterGetPlayRate(prPsrCC->pvSptHdl))) {
			if (fgIsIType(prVPicAU->rAUInfo.rInfo.u4VType)) {
				if (0 == prPsrFtr->u4AUCntFromIFrm)
					prPsrFtr->u4AUCntFromIFrm = 1;
				else
					prPsrFtr->u4AUCntFromIFrm++;

				prPsrFtr->u4IFrmCnt++;
			} else if (0 < prPsrFtr->u4AUCntFromIFrm) {
				prPsrFtr->u4AUCntFromIFrm++;
			}

			if (prPsrFtr->u4AUCntFromIFrm > MAX_VID_AU_CNT_IN_CTRL_RW) {
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("[PSR] %s -- RW Ctrl Need JUMP!\r\n"),
						DMX_FUNC_NAME);
				mrRet = ESM_AUTableIncWrIdx(prPsrFtr->u4ESIH, 1);
				if (DMX_FAILED(mrRet))
					MM_RETURN(mrRet);

				prVFSD->fgUseDummyAURealWrIdx = FALSE;
				prVFSD->fgUseRealWrIdx = FALSE;
				prVFSD->u4VType = 0;
				PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
				MM_RETURN(RET_DMX_NEED_JUMP);
			}
		}
#ifdef MM_SUPPORT_DIVXHT31
		if ((prPsrCC != NULL) &&
				(CFA_TYPE_AVI == SplitterGetCfaType(prPsrCC->pvSptHdl))) {
			s32 i4Rate = SplitterGetPlayRate(prPsrCC->pvSptHdl);

			if ((i4Rate >= MM_PLAY_RATE_FF_8X) || (MM_IS_RW_PLAY(i4Rate))) {
				if (fgIsIType(prVPicAU->rAUInfo.rInfo.u4VType)) {
					if (0 == prPsrFtr->u4AUCntFromIFrm)
						prPsrFtr->u4AUCntFromIFrm = 1;
					else
						prPsrFtr->u4AUCntFromIFrm++;
				} else if (0 < prPsrFtr->u4AUCntFromIFrm) {
					prPsrFtr->u4AUCntFromIFrm++;
				}

				if (prPsrFtr->u4AUCntFromIFrm > MAX_VID_AU_CNT_IN_CTRL_RW) {
					DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("[PSR] %s -- DivXHT31 Need JUMP!\r\n"),
						DMX_FUNC_NAME);
					mrRet = ESM_AUTableGetNextAUIdx(prPsrFtr->u4ESIH,
									u4AUIdx, 1, &u4AUIdx);
					if (DMX_FAILED(mrRet)) {
						DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
							TEXT("[PSR] %s line %d fail in ")
							TEXT(" ESM_AUTableGetNextAUIdx(auidx: %d, Count")
							TEXT(": %d), mrRet: 0x%x\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							u4AUIdx, 1, mrRet);
						DMX_ASSERT(FALSE);
						MM_RETURN(mrRet);
					}

					mrRet = ESM_AUTableSetWrIdx(prPsrFtr->u4ESIH, u4AUIdx);
					if (DMX_FAILED(mrRet)) {
						DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
							TEXT("[PSR] %s line %d fail in ")
							TEXT("ESM_AUTableSetWrIdx(auidx: %d, Count: %d)")
							TEXT(", mrRet: 0x%x\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							u4AUIdx, 1, mrRet);
						DMX_ASSERT(FALSE);
						MM_RETURN(mrRet);
					}

					prVFSD->fgUseRealWrIdx = FALSE;

					PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
					MM_RETURN(RET_DMX_NEED_JUMP);
				}
			}
		}
#endif				/* MM_SUPPORT_DIVXHT31 */

		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("MPEG4: Create AU(Idx:%d) -- VType: 0x%x, SA: 0x%x")
			 TEXT(", EA: 0x%x, Offset: "DMX_UINT64_10U_LOGSTR)
			 TEXT(", Pts: "DMX_PTS_LOGSTR"\r\n"),
			u4AUIdx,
			prVPicAU->rAUInfo.rInfo.u4VType,
			prVPicAU->rAUInfo.rInfo.ptrSAddr,
			prVPicAU->rAUInfo.rInfo.ptrEAddr,
			DMX_UINT64_10U_LOG(prVPicAU->rAUInfo.rInfo.u8Offset),
			DMX_PTS_LOG_MS(prVPicAU->rAUInfo.rInfo.u8Pts),
			DMX_PTS_LOG_PTS(prVPicAU->rAUInfo.rInfo.u8Pts));

		/*Prepare for next AU */
		mrRet = ESM_AUTableGetNextAUIdx(prPsrFtr->u4ESIH, u4AUIdx, 1, &u4AUIdx);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail in ESM_AUTable")
				TEXT("GetNextAUIdx(auidx: %d, Count: %d), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx, 1, mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		if (u4AUIdx >= u4TotalAUCnt) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for u4AUIdx(%d) > ")
				 TEXT("u4TotalAUCnt(%d)\r\n"), DMX_FUNC_NAME,
				DMX_LINE_NO, u4AUIdx, u4TotalAUCnt);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		mrRet =
				ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4AUIdx, (void **) &prVPicAU);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		/*fix klocwork bug */
		if (NULL == prVPicAU) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for prVPicAU = NULL!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		dmx_memset(prVPicAU, 0, sizeof(AU_VPic));

		if (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount)
			prVPicAU->rAUInfo.rInfo.ptrSAddr =
					prVFSD->rPicDetResult.ptrPicAddr[u4Idx];
		else
			prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrHWCurWp;

		prVPicAU->eAuType = AU_DATA;

		u4CurAUWrIdx = u4AUIdx;

		u4VType = 0;
	}

	/* Update ESI AU write index */
	/* Because we need confirm all data had into VFifo if Decoder get a picture, */
	/* so that, the last picture information maybe filled into AU table, but can not be */
	/* get by decoder.  */
	/* We need to update write index to real write index when we got sequence end or destory. */
	if (u4Idx == prVFSD->rPicDetResult.u1PicInfoCount) {
		u4AUIdx = u4CurAUWrIdx;

		if (u4AUIdx >= u4TotalAUCnt) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for u4AUIdx(%d)")
				TEXT(" > u4TotalAUCnt(%d)\r\n"), DMX_FUNC_NAME,
				DMX_LINE_NO, u4AUIdx, u4TotalAUCnt);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4AUIdx,
			(void **) &prVPicAU);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		/*fix klocwork bug */
		if (NULL == prVPicAU) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d failed for prVPicAU = NULL!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		if (0 != prVPicAU->rAUInfo.rInfo.u4VType) {
			prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;
		} else {
			if (0 == prVPicAU->rAUInfo.rInfo.ptrSAddr)
				prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrHWCurWp;
			if (0 == prVPicAU->rAUInfo.rInfo.ptrEAddr)
				prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;
		}

		if (prPsrFtr->fgFirstAUInRng) {
			/* Use the flag to designated whether the au is the first au in the cfa range */
			prVPicAU->fgIBCSent = TRUE;
			prPsrFtr->fgFirstAUInRng = FALSE;
		}

		prVFSD->u4RealWrIdx = u4AUIdx;
		prVFSD->fgUseRealWrIdx = TRUE;
		/* get AU */
		mrRet = ESM_AUTableSetWrIdx(prPsrFtr->u4ESIH, u4AUIdx);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
	} else {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d failed for u4Idx(%d) ")
			TEXT("!= prVFSD->rPicDetResult.u1PicInfoCount(%d)!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
			prVFSD->rPicDetResult.u1PicInfoCount);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_VFilter_UpdateESIInfo_H265(PSR_FILTER *prPsrFtr,
	u32 u4CurAUWrIdx, AU_VPic *prVPicAU,
	uintptr_t ptrHWCurWp, uintptr_t ptrCurFIFOWp)
{
	PSR_AU rAU;
	PSR_CC *prPsrCC = NULL;
	PSR_VFSD *prVFSD = NULL;
	AU_VPic *prPrevVPicAU = NULL;
	u32 u4AUIdx;
	u32 u4Idx;
	u32 u4EndVIdx = DMX_INVALID_UINT32;
	u32 u4TotalAUCnt = 0;
	u32 u4VType = 0;
	MRESULT mrRet = RET_DMX_OK;

	DMX_ASSERT(NULL != prPsrFtr);

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);
	DMX_ASSERT(NULL != prPsrCC);

	prVFSD = (PSR_VFSD *) (prPsrFtr->pvFilterSpecific);
	DMX_ASSERT(NULL != prVFSD);

	mrRet = ESM_AUTableGetTotalCount(prPsrFtr->u4ESIH, &u4TotalAUCnt);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail in ESM_AUTableGetTotalCount")
			TEXT(", mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (u4CurAUWrIdx >= u4TotalAUCnt) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail for u4CurAUWrIdx(%d)")
			TEXT(" > u4TotalAUCnt(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx, u4TotalAUCnt);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
		TEXT("[ESM][H265] %s line %d -- u1PicInfoCount: %d, u4CurAUWrIdx: %d\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		prVFSD->rPicDetResult.u1PicInfoCount, u4CurAUWrIdx);

	u4Idx = 0;

	while (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount) {
		if (u4Idx >= DMX_MAX_VID_STARTCODE_CNT) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		if ((prVFSD->rPicDetResult.ptrPicAddr[u4Idx] >= prPsrFtr->ptrESFifoEa) ||
				(prVFSD->rPicDetResult.ptrPicAddr[u4Idx] < prPsrFtr->ptrESFifoSa)) {
			DMX_ASSERT(FALSE);
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for PicDetResultIdx(%d)'s ")
				TEXT("PicAddr(0x%lx) exceed fifo Range[0x%lx, 0x%lx)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
				prVFSD->rPicDetResult.ptrPicAddr[u4Idx],
				prPsrFtr->ptrESFifoSa, prPsrFtr->ptrESFifoEa);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		if (u4CurAUWrIdx >= u4TotalAUCnt) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for u4CurAUWrIdx(%d)")
				TEXT(" > u4TotalAUCnt(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx, u4TotalAUCnt);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		/* get AU */
		u4AUIdx = u4CurAUWrIdx;
		mrRet =
				ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4AUIdx, (void **) &prVPicAU);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		/*fix klocwork bug */
		if (NULL == prVPicAU) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for prVPicAU = NULL!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		if (AU_DATA == prVPicAU->eAuType)
			u4VType = prVPicAU->rAUInfo.rInfo.u4VType;
		else
			u4VType = 0;

		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[ESM][H265] %s line %d -- PicIdx[%d], PicType: %u, Addr: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
			prVFSD->rPicDetResult.u1PicType[u4Idx],
			prVFSD->rPicDetResult.ptrPicAddr[u4Idx]);

		if (0 == DMX_GET_PICTYPE(u4VType)) {
			if ((PSR_HDR_H265_EOS == prVFSD->rPicDetResult.u1PicType[u4Idx]) ||
					(PSR_HDR_H265_EOB == prVFSD->rPicDetResult.u1PicType[u4Idx])) {
				/* may be encounter the following cause : */
				/* seqend1 seqend2 seqend3 frame */
				/* seqend2 to seqend3 should be skip, so do this. */
				u4Idx++;
				if (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount) {
					prVPicAU->rAUInfo.rInfo.ptrSAddr =
							prVFSD->rPicDetResult.ptrPicAddr[u4Idx];
				} else {
					prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrHWCurWp;
				}
				continue;
			}
			mrRet = PSR_VFilter_SetFileOfstInAU(prPsrFtr, prVPicAU,
				ptrCurFIFOWp, u4Idx);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] %s line %d fail in PSR_VFilter_SetFileOfstInAU!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				MM_RETURN(mrRet);
			}

			if (0 == prVPicAU->rAUInfo.rInfo.ptrSAddr) {
				/* fill SAddr */
				if (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount) {
					prVPicAU->rAUInfo.rInfo.ptrSAddr =
							prVFSD->rPicDetResult.ptrPicAddr[u4Idx];
				} else {
					prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrHWCurWp;
				}
			}
		}

		switch (prVFSD->rPicDetResult.u1PicType[u4Idx]) {
		case PSR_HDR_H265_VPS:	/* VPS */
			{
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[ESM][H265] %s line %d -- PicIdx[%d], Addr: 0x%x, VPS\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
					prVFSD->rPicDetResult.ptrPicAddr[u4Idx]);
				if (!prVFSD->fgHasSetPics) {
					u4Idx++;
					continue;
				}
			}
			break;
		case PSR_HDR_H265_SPS:
			/*SPS*/ {
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[ESM][H265] %s line %d -- PicIdx[%d], Addr: 0x%x, SPS\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
					prVFSD->rPicDetResult.ptrPicAddr[u4Idx]);

				if (!prVFSD->fgHasSetPics) {
					u4VType |= SEQ_PS;
					prVPicAU->rAUInfo.rInfo.u4VType |= SEQ_PS;
					u4Idx++;
					continue;
				}
			}
			break;

		case PSR_HDR_H265_PPS:
			/*PPS*/ {
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[ESM][H265] %s line %d -- PicIdx[%d], Addr: 0x%x, PPS\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
					prVFSD->rPicDetResult.ptrPicAddr[u4Idx]);
				if (!prVFSD->fgHasSetPics) {
					u4VType |= PIC_PS;
					prVPicAU->rAUInfo.rInfo.u4VType |= PIC_PS;
					u4Idx++;
					continue;
				}
			}
			break;

		case PSR_HDR_H265_AUD:	/* AUD */
			{
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[ESM][H265] %s line %d -- PicIdx[%d], Addr: 0x%x, AUD\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
					prVFSD->rPicDetResult.ptrPicAddr[u4Idx]);
				if (!prVFSD->fgHasSetPics) {
					u4Idx++;
					continue;
				}
			}
			break;

		case PSR_HDR_H265_PREFIX_SEI:	/*Prefix SEI */
			{
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[ESM][H265] %s line %d -- PicIdx[%d], Addr: 0x%x, ")
					 TEXT("PREFIX_SEI\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
					prVFSD->rPicDetResult.ptrPicAddr[u4Idx]);
				if (!prVFSD->fgHasSetPics) {
					/* Modify according to the h265 composing au spec */
					u4VType |= SEI;
					prVPicAU->rAUInfo.rInfo.u4VType |= SEI;
					u4Idx++;
					continue;
				}
			}
			break;
		case PSR_HDR_H265_EOS:
			/*EOS*/ {
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[ESM][H265] %s line %d -- PicIdx[%d], Addr: 0x%x, ")
					TEXT("EOS_NUT/SEQ_END\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
					prVFSD->rPicDetResult.ptrPicAddr[u4Idx]);
				if (!prVFSD->fgHasSetPics) {
					u4VType |= SEQ_END;
					u4Idx++;
					continue;
				} else {
					u4VType |= SEQ_END;
					u4Idx++;
					break;
				}
			}
			break;
		case PSR_HDR_H265_EOB:
			/*EOB*/ {
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[ESM][H265] %s line %d -- PicIdx[%d], Addr: 0x%x, ")
					TEXT("BITSTREAM_END(EOB_NUT)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
					prVFSD->rPicDetResult.ptrPicAddr[u4Idx]);
				if (!prVFSD->fgHasSetPics) {
					u4Idx++;
					continue;
				} else {
					u4Idx++;
					break;
				}
			}
			break;

		case PSR_HDR_H265_RSVNVCL_41_44:	/* RSV_NVCL 41~44 */
			{
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[ESM][H265] %s line %d -- PicIdx[%d], Addr: 0x%x, ")
					TEXT("RSV_NVCL_41_44\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
					prVFSD->rPicDetResult.ptrPicAddr[u4Idx]);
				if (!prVFSD->fgHasSetPics) {
					u4Idx++;
					continue;
				}
			}

			break;

		case PSR_HDR_H265_UNSPEC_48_55:	/* UNSPEC 48~55 */
			{
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[ESM][H265] %s line %d -- PicIdx[%d], Addr: 0x%x, ")
					 TEXT("UNSPEC_48_55\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
					prVFSD->rPicDetResult.ptrPicAddr[u4Idx]);
				if (!prVFSD->fgHasSetPics) {
					u4Idx++;
					continue;
				}
			}

			break;

		case PSR_HDR_H265_VCL:{
				/*Parsing Picture Type */
				u8 b;
				u32 u4PicType = 0;
				u8 *pb =
						(u8 *) (prVFSD->rPicDetResult.ptrPicAddr[u4Idx]) + 5;

				if ((uintptr_t) pb >= prPsrFtr->ptrESFifoEa)
					pb -= prPsrFtr->u4ESFifoSize;

				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[ESM][H265] Idx(%d), VCL, Addr: 0x%x, pb: 0x%02x!\r\n"),
					u4Idx, prVFSD->rPicDetResult.ptrPicAddr[u4Idx], *pb);

				b = *pb;

				u4PicType |= I_SLICE;

				if ((b & 0x80) != 0) {	/* first_slice_segment_in_pic_flag */
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("[ESM][H265] %s line %d -- PicIdx[%d], VCL, b: ")
						TEXT("0x%02x, FIRST_SLICE\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, u4Idx, b);
					if (!prVFSD->fgHasSetPics)
						u4VType |= u4PicType;
				} else {
					DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("[ESM][H265] %s line %d -- PicIdx[%d], VCL, b: 0x%02x\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, u4Idx, b);
					u4VType |= u4PicType;
					u4Idx++;
					continue;
				}
			}
			break;

		default:{
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] Idx(%d) -- Undefined H265")
					 TEXT(" PicType, error, pic type:0x%x\r\n"),
					u4Idx, prVFSD->rPicDetResult.u1PicType[u4Idx]);
				u4Idx++;
				continue;
			}
			break;

		}

		if (!prVFSD->fgHasSetPics) {
			u32 u4PrevAUIdx = ESM_INVALID_INDEX;

			if (NULL == prVPicAU) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] %s line %d fail for prVPicAU = NULL!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			/* write ESI AU information */
			prVPicAU->eAuType = AU_DATA;

			/* callback notify Splitter to fill AU information */
			mm_memset(&rAU, 0, sizeof(PSR_AU));
			rAU.eType = prPsrFtr->eType;
			rAU.pvAUInf = prVPicAU;
			rAU.pvAUExtInf = NULL;

			/*Init Previous PTS */
			prVPicAU->rAUInfo.rInfo.u4Duration = 0;
			prVPicAU->rAUInfo.rInfo.u4PrevDuration = 0;
			prVPicAU->rAUInfo.rInfo.u8PrevPTS = 0;

			SetVCodec(u4VType, prVFSD->eVCodeC);

			prVPicAU->rAUInfo.rInfo.u4VType |= u4VType;

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

			if (INVALID_TIMESTAMP != prVPicAU->rAUInfo.rInfo.u8Pts) {
				prPsrFtr->u8LastPTS = prVPicAU->rAUInfo.rInfo.u8Pts;
#ifdef MM_SUPPORT_DIVXHT31
				prPsrFtr->u8PrevPTS = prPsrFtr->u8LastPTS;
				if (INVALID_TIMESTAMP == prPsrFtr->u81stPTS) {
					DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("[PSR] %s Line %d --- (Video)")
						 TEXT(" set 1stPts to be "DMX_PTS_LOGSTR"\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						DMX_PTS_LOG_MS(prPsrFtr->u8LastPTS),
						DMX_PTS_LOG_PTS(prPsrFtr->u8LastPTS));
					prPsrFtr->u81stPTS = prPsrFtr->u8LastPTS;
				}
#endif				/* MM_SUPPORT_DIVXHT31 */
			}

			mrRet = ESM_AUTableGetPrevAUIdx(prPsrFtr->u4ESIH, u4AUIdx, 1,
							&u4PrevAUIdx);
			if (DMX_FAILED(mrRet)) {
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}

			if (ESM_INVALID_INDEX != u4PrevAUIdx) {
				mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4PrevAUIdx,
									 (void **) &prPrevVPicAU);
				if (DMX_FAILED(mrRet)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("[PSR] %s line %d -- ESM_AUTableGetAUInfo Failed!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
					DMX_ASSERT(FALSE);
					MM_RETURN(mrRet);
				}
				/*fix klocwork bug */
				if (NULL == prPrevVPicAU) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
						TEXT("[PSR] %s line %d -- prPrevVPicAU = NULL Failed!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_UNEXPECT);
				}

				if (INVALID_TIMESTAMP == prVPicAU->rAUInfo.rInfo.u8PrevPTS) {
					/*For File Playback PTS control */
					prPrevVPicAU->rAUInfo.rInfo.u8Pts =
							INVALID_TIMESTAMP;
				}

				if (prVPicAU->rAUInfo.rInfo.u4PrevDuration != 0) {
					prPrevVPicAU->rAUInfo.rInfo.u4Duration =
							prVPicAU->rAUInfo.rInfo.u4PrevDuration;
				}
			}

			prVFSD->fgHasSetPics = TRUE;

			u4Idx++;

			continue;
		}

		prVFSD->fgHasSetPics = FALSE;
		if (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount)
			prVPicAU->rAUInfo.rInfo.ptrEAddr =
					prVFSD->rPicDetResult.ptrPicAddr[u4Idx];
		else
			prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;

		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("H265: line %d -- Set AUIdx(%d)'s EA=0x%x\r\n"),
			DMX_LINE_NO, u4AUIdx, prVPicAU->rAUInfo.rInfo.ptrEAddr);

		if (prPsrFtr->fgFirstAUInRng) {
			/* Use the flag to designated whether the au is the first au in the cfa range */
			prVPicAU->fgIBCSent = TRUE;
			prPsrFtr->fgFirstAUInRng = FALSE;
		}

		if (MM_IS_RW_PLAY(SplitterGetPlayRate(prPsrCC->pvSptHdl))) {
			if (fgIsIType(prVPicAU->rAUInfo.rInfo.u4VType)) {
				if (0 == prPsrFtr->u4AUCntFromIFrm)
					prPsrFtr->u4AUCntFromIFrm = 1;
				else
					prPsrFtr->u4AUCntFromIFrm++;

				prPsrFtr->u4IFrmCnt++;
			} else if (0 < prPsrFtr->u4AUCntFromIFrm) {
				prPsrFtr->u4AUCntFromIFrm++;
			}

			if (prPsrFtr->u4AUCntFromIFrm > MAX_VID_AU_CNT_IN_CTRL_RW) {
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] %s -- RW Ctrl Need JUMP!\r\n"),
					DMX_FUNC_NAME);
				mrRet = ESM_AUTableIncWrIdx(prPsrFtr->u4ESIH, 1);
				if (DMX_FAILED(mrRet))
					MM_RETURN(mrRet);

				prVFSD->fgUseDummyAURealWrIdx = FALSE;
				prVFSD->fgUseRealWrIdx = FALSE;
				prVFSD->u4VType = 0;
				PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
				MM_RETURN(RET_DMX_NEED_JUMP);
			}
		}

		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("H265: Create AU(Idx:%d) -- VType: 0x%x, SA: 0x%x")
			 TEXT(", EA: 0x%x, Offset: "DMX_UINT64_10U_LOGSTR)
			 TEXT(", Pts: "DMX_PTS_LOGSTR"\r\n"),
			u4AUIdx,
			prVPicAU->rAUInfo.rInfo.u4VType,
			prVPicAU->rAUInfo.rInfo.ptrSAddr,
			prVPicAU->rAUInfo.rInfo.ptrEAddr,
			DMX_UINT64_10U_LOG(prVPicAU->rAUInfo.rInfo.u8Offset),
			DMX_PTS_LOG_MS(prVPicAU->rAUInfo.rInfo.u8Pts),
			DMX_PTS_LOG_PTS(prVPicAU->rAUInfo.rInfo.u8Pts));

		mrRet = ESM_AUTableGetNextAUIdx(prPsrFtr->u4ESIH, u4AUIdx, 1, &u4AUIdx);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail in ESM_AUTableGetNextAUIdx")
				 TEXT("(AUIdx: %d, Count: %d), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx, 1, mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		if (u4AUIdx >= u4TotalAUCnt) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for u4AUIdx(%d)")
				 TEXT(" > u4TotalAUCnt(%d)\r\n"), DMX_FUNC_NAME,
				DMX_LINE_NO, u4AUIdx, u4TotalAUCnt);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		mrRet =
				ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4AUIdx, (void **) &prVPicAU);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		dmx_memset(prVPicAU, 0, sizeof(AU_VPic));

		if (u4Idx < prVFSD->rPicDetResult.u1PicInfoCount)
			prVPicAU->rAUInfo.rInfo.ptrSAddr =
					prVFSD->rPicDetResult.ptrPicAddr[u4Idx];
		else
			prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrHWCurWp;

		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("H265: line %d -- Set AUIdx(%d)'s SA=0x%x\r\n"),
			DMX_LINE_NO, u4AUIdx, prVPicAU->rAUInfo.rInfo.ptrSAddr);

		prVFSD->fgHasSetPics = FALSE;

		prVPicAU->eAuType = AU_DATA;

		u4EndVIdx = u4Idx;

		u4CurAUWrIdx = u4AUIdx;

		u4VType = 0;
	}

	if (u4Idx == prVFSD->rPicDetResult.u1PicInfoCount) {
		u4AUIdx = u4CurAUWrIdx;

		if (u4AUIdx >= u4TotalAUCnt) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail for u4AUIdx(%d)")
				 TEXT(" > u4TotalAUCnt(%d)\r\n"), DMX_FUNC_NAME,
				DMX_LINE_NO, u4AUIdx, u4TotalAUCnt);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		mrRet =
				ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4AUIdx, (void **) &prVPicAU);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		/*fix klocwork bug */
		if (NULL == prVPicAU) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d failed for prVPicAU = NULL!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		prVPicAU->eAuType = AU_DATA;

		SetVCodec(u4VType, prVFSD->eVCodeC);

		prVPicAU->rAUInfo.rInfo.u4VType |= u4VType;

		if (0 != prVPicAU->rAUInfo.rInfo.u4VType) {
			prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("H265: line %d -- u4VType(0x%x) != 0, ")
				TEXT("Set AUIdx(%d)'s EA=0x%x\r\n"),
				DMX_LINE_NO, prVPicAU->rAUInfo.rInfo.u4VType, u4AUIdx,
				prVPicAU->rAUInfo.rInfo.ptrEAddr);
		} else {
			if (0 == prVPicAU->rAUInfo.rInfo.ptrSAddr) {
				prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrHWCurWp;
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("H265: line %d -- u4VType== 0. Set AUIdx(%d)'s SA=0x%x\r\n"),
					DMX_LINE_NO, u4AUIdx,
					prVPicAU->rAUInfo.rInfo.ptrSAddr);
			}
			if (0 == prVPicAU->rAUInfo.rInfo.ptrEAddr) {
				prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("H265: line %d -- u4VType== 0. Set AUIdx(%d)'s EA=0x%x\r\n"),
					DMX_LINE_NO, u4AUIdx,
					prVPicAU->rAUInfo.rInfo.ptrEAddr);
			}
		}

		if (prPsrFtr->fgFirstAUInRng) {
			/* Use the flag to designated whether the au is the first au in the cfa range */
			prVPicAU->fgIBCSent = TRUE;
			prPsrFtr->fgFirstAUInRng = FALSE;
		}

		prVFSD->u4RealWrIdx = u4AUIdx;

		prVFSD->fgUseRealWrIdx = TRUE;

		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d -- fgUseRealWrIdx: %d, u4RealWrIdx: 0x%x!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, (prVFSD->fgUseRealWrIdx ? 1 : 0),
			prVFSD->u4RealWrIdx);
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d -- AUIdx(%d)'s ptrSAddr: 0x%08x, ptrEAddr: ")
			 TEXT("0x%08x!\r\n"), DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx,
			prVPicAU->rAUInfo.rInfo.ptrSAddr, prVPicAU->rAUInfo.rInfo.ptrEAddr);

		/* get AU */
		mrRet = ESM_AUTableSetWrIdx(prPsrFtr->u4ESIH, u4AUIdx);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
	} else {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d failed for u4Idx(%d) ")
			TEXT("!= prVFSD->rPicDetResult.u1PicInfoCount(%d)!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Idx,
			prVFSD->rPicDetResult.u1PicInfoCount);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
		TEXT("[PSR] %s line %d exit -- fgUseRealWrIdx: %d, ")
		TEXT("u4RealWrIdx: 0x%x, exit!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, (prVFSD->fgUseRealWrIdx ? 1 : 0),
		prVFSD->u4RealWrIdx);

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_VFilter_UpdateESIInfo_HdrDect(PSR_FILTER *prPsrFtr)
{
	PSR_CC *prPsrCC = NULL;
	u32 u4VType;
	u32 u4CurAUWrIdx;
	u32 u4AUIdx;
	AU_VPic *prVPicAU = NULL;
	PSR_VFSD *prVFSD = NULL;
	u32 u4GarbageSz = 0;
	uintptr_t ptrCurFIFOWp = 0, ptrHWCurWp = 0;
	u32 u4TotalAUCnt = 0;
	MRESULT mrRet = RET_DMX_OK;

	DMX_ASSERT(NULL != prPsrFtr);

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);
	DMX_ASSERT(NULL != prPsrCC);

	prVFSD = (PSR_VFSD *) prPsrFtr->pvFilterSpecific;
	DMX_ASSERT(NULL != prVFSD);

	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
		TEXT("[PSR] %s enter\r\n"), DMX_FUNC_NAME);

	/* get HW current write pointer */
	mrRet = PSR_HAL_GetWPtr(prPsrFtr->ucHwDevId, BitType_Video, &ptrHWCurWp);
	if (DMX_FAILED(mrRet))
		MM_RETURN(mrRet);

	if ((ptrHWCurWp >= prPsrFtr->ptrESFifoEa) ||
		(ptrHWCurWp < prPsrFtr->ptrESFifoSa)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail in ptrHWCurWp(0x%lx)")
			TEXT(" exceed PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, ptrHWCurWp, prPsrFtr->ptrESFifoSa,
			prPsrFtr->ptrESFifoEa);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	mrRet = ESM_FifoGetWrPtr(prPsrFtr->u4ESIH, &ptrCurFIFOWp);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail in ESM_FifoGetWrPtr")
			TEXT("(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if ((ptrCurFIFOWp >= prPsrFtr->ptrESFifoEa) ||
		(ptrCurFIFOWp < prPsrFtr->ptrESFifoSa)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail in ptrCurFIFOWp(0x%lx)")
			TEXT(" exceed PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, ptrCurFIFOWp, prPsrFtr->ptrESFifoSa,
			prPsrFtr->ptrESFifoEa);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	/* get picture header detect status */
	if (NULL == prVFSD->prHALStatus) {
		DMX_NewMemory(sizeof(PSR_HDRDET_STATUS_T), prVFSD->prHALStatus);
		if (NULL == prVFSD->prHALStatus)
			MM_RETURN(RET_DMX_NO_MEM);
	}

	/* get tx result */
	dmx_memset((void *) prVFSD->prHALStatus, 0x00, sizeof(PSR_HDRDET_STATUS_T));
	mrRet = PSR_HAL_GetHdrDetResult(prPsrFtr,
		&(prVFSD->rPicDetResult), prVFSD->prHALStatus);
	if (DMX_FAILED(mrRet)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
		TEXT("[PSR] %s line %d -- PSR_HAL_GetGarbageSz\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	u4GarbageSz = PSR_HAL_GetGarbageSz(prPsrFtr->ucHwDevId);

	if (u4GarbageSz > 0) {
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d -- PSR_HAL_GetGarbageSz(%d)")
			TEXT(", ptrHWCurWp: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4GarbageSz, ptrHWCurWp);
		if (ptrHWCurWp >= u4GarbageSz) {
			ptrHWCurWp -= u4GarbageSz;
			if ((prPsrCC->fgUseCmdQ) &&
					(1 <= prPsrCC->u2TxEntryCnt) &&
					(prPsrCC->u2TxEntryCnt <= DMX_MAX_TX_CNT_FOR_CMD_Q) &&
					(NULL != prPsrCC->pvCmdQTxEntryBuffer)) {
				DMX_CMDQ_TX_ENTRY_T *prTxEntry =
						(DMX_CMDQ_TX_ENTRY_T *) (prPsrCC->pvCmdQTxEntryBuffer);
				prTxEntry = prTxEntry + prPsrCC->u2TxEntryCnt - 1;
				if (prTxEntry->u4TxLen > u4GarbageSz)
					prTxEntry->u4TxLen -= u4GarbageSz;
			}
		}

		if (ptrHWCurWp < prPsrFtr->ptrESFifoSa) {
			ptrHWCurWp += prPsrFtr->u4ESFifoSize;
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d -- PSR_HAL_GetGarbageSz, ptrHWCurWp: 0x%x, ")
				TEXT(" u4ESFifoSize: 0x%x, ptrESFifoSa: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, ptrHWCurWp, prPsrFtr->u4ESFifoSize,
				prPsrFtr->ptrESFifoSa);
		} else {
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d -- PSR_HAL_GetGarbageSz, ptrHWCurWp: 0x%x, ")
				TEXT("u4ESFifoSize: 0x%x, ptrESFifoSa: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, ptrHWCurWp, prPsrFtr->u4ESFifoSize,
				prPsrFtr->ptrESFifoSa);
		}
	}

	if ((ptrHWCurWp >= prPsrFtr->ptrESFifoEa) ||
		(ptrHWCurWp < prPsrFtr->ptrESFifoSa)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail in ptrCurFIFOWp(0x%lx)")
			TEXT(" exceed PsrFtr's Fifo Range[0x%lx, 0x%lx)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, ptrHWCurWp, prPsrFtr->ptrESFifoSa,
			prPsrFtr->ptrESFifoEa);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	mrRet = ESM_FifoSetWrPtr(prPsrFtr->u4ESIH, ptrHWCurWp);
	if (DMX_FAILED(mrRet))
		MM_RETURN(mrRet);

#if DMX_DISABLE_COMP_MPEG2AU
	mrRet = ESM_FifoSetRdPtr(prPsrFtr->u4ESIH, ptrHWCurWp, FALSE);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail in ESM_FifoSetRdPtr")
			TEXT("(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
#endif				/* DMX_DISABLE_COMP_MPEG2AU */

	/* If dummy AU, */
	if (prVFSD->fgUseDummyAURealWrIdx) {
		mrRet = ESM_AUTableGetNextAUIdx(prPsrFtr->u4ESIH,
			prVFSD->u4DummyAURealWrIdx, 1, &u4CurAUWrIdx);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d fail in ESM_AUTable")
				TEXT("GetNextAUIdx(auidx: %d, Count: %d), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prVFSD->u4DummyAURealWrIdx, 1,
				mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
		prVFSD->fgUseDummyAURealWrIdx = FALSE;
		u4VType = 0;
	} else {
		if ((prVFSD->fgUseRealWrIdx) &&
			(DMX_INVALID_UINT32 != prVFSD->u4RealWrIdx)) {
			mrRet = ESM_AUTableGetNextAUIdx(prPsrFtr->u4ESIH, prVFSD->u4RealWrIdx,
							0, &u4CurAUWrIdx);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] %s line %d fail in ESM_AUTable")
					TEXT("GetNextAUIdx(auidx: %d, Count: %d), mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prVFSD->u4RealWrIdx, 0x00,
					mrRet);
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}
		} else {
			mrRet = ESM_AUTableGetWrIdx(prPsrFtr->u4ESIH, &u4CurAUWrIdx);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
					TEXT("[PSR] %s line %d fail in ESM_AUTable")
					 TEXT("GetWrIdx(auidx: %d), mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx, mrRet);
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}
		}
	}

	mrRet = ESM_AUTableGetTotalCount(prPsrFtr->u4ESIH, &u4TotalAUCnt);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail in ESM_AUTable")
			TEXT("GetTotalCount, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (u4CurAUWrIdx >= u4TotalAUCnt) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d fail for u4CurAUWrIdx(%d)")
			TEXT(" > u4TotalAUCnt(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4CurAUWrIdx, u4TotalAUCnt);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (prVFSD->rPicDetResult.u1PicInfoCount > 0) {
		switch (prVFSD->eVCodeC) {
		case VC_MPEG2:{
				mrRet = PSR_VFilter_UpdateESIInfo_MPEG2(prPsrFtr, u4CurAUWrIdx,
					prVPicAU, ptrHWCurWp, ptrCurFIFOWp);
			}
			break;
		case VC_H264:{
				mrRet = PSR_VFilter_UpdateESIInfo_H264(prPsrFtr, u4CurAUWrIdx,
					prVPicAU, ptrHWCurWp, ptrCurFIFOWp);
			}
			break;
		case VC_VC1:{
				mrRet = PSR_VFilter_UpdateESIInfo_VC1(prPsrFtr, u4CurAUWrIdx,
					prVPicAU, ptrHWCurWp, ptrCurFIFOWp);
			}
			break;
		case VC_MPEG4:
		case VC_DIVX4:
		case VC_DIVX6:
		case VC_H263:{
				mrRet = PSR_VFilter_UpdateESIInfo_MPEG4(prPsrFtr, u4CurAUWrIdx,
					prVPicAU, ptrHWCurWp, ptrCurFIFOWp);
			}
			break;
		case VC_H265:{
				mrRet = PSR_VFilter_UpdateESIInfo_H265(prPsrFtr, u4CurAUWrIdx,
					prVPicAU, ptrHWCurWp, ptrCurFIFOWp);
			}
			break;
		default:{
				mrRet = RET_DMX_UNEXPECT;
			}
			break;
		}

		MM_RETURN(mrRet);
	} else {
		u4AUIdx = u4CurAUWrIdx;
		mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4AUIdx,
			(void **) &prVPicAU);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		/*fix klocwork bug */
		if (NULL == prVPicAU) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
				TEXT("[PSR] %s line %d failed for prVPicAU = NULL!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		prVPicAU->eAuType = AU_DATA;

		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d -- fgUseRealWrIdx: %d, u4RealWrIdx: 0x%x!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, (prVFSD->fgUseRealWrIdx ? 1 : 0),
			prVFSD->u4RealWrIdx);
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d -- ptrSAddr: 0x%08x, ptrEAddr: 0x%08x!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			prVPicAU->rAUInfo.rInfo.ptrSAddr, prVPicAU->rAUInfo.rInfo.ptrEAddr);

		SetVCodec(prVPicAU->rAUInfo.rInfo.u4VType, prVFSD->eVCodeC);

		prVPicAU->rAUInfo.rInfo.ptrEAddr = ptrHWCurWp;

		if (0 == prVPicAU->rAUInfo.rInfo.ptrSAddr)
			prVPicAU->rAUInfo.rInfo.ptrSAddr = ptrCurFIFOWp;

		prVFSD->u4RealWrIdx = u4AUIdx;

		prVFSD->fgUseRealWrIdx = TRUE;

		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d -- 2 fgUseRealWrIdx: %d, u4RealWrIdx: 0x%x!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, (prVFSD->fgUseRealWrIdx ? 1 : 0),
			prVFSD->u4RealWrIdx);
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_COMPAU_V,
			TEXT("[PSR] %s line %d -- 2 ptrSAddr: 0x%08x, ptrEAddr: 0x%08x!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			prVPicAU->rAUInfo.rInfo.ptrSAddr, prVPicAU->rAUInfo.rInfo.ptrEAddr);

		/* get AU */
		mrRet = ESM_AUTableSetWrIdx(prPsrFtr->u4ESIH, u4AUIdx);
		if (DMX_FAILED(mrRet)) {
			DMX_ASSERT(FALSE);
			return mrRet;
		}
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_VFilter_UpdateESIInfo(PSR_FILTER *prPsrFtr)
{
	PSR_VFSD *prVFSD = NULL;
	MRESULT mrRet = RET_DMX_OK;

	DMX_ASSERT(NULL != prPsrFtr);

	prVFSD = (PSR_VFSD *) prPsrFtr->pvFilterSpecific;

	DMX_ASSERT(NULL != prVFSD);

	switch (prVFSD->eVCodeC) {
	case VC_MPEG2:
	case VC_H264:
	case VC_VC1:
	case VC_MPEG4:
	case VC_DIVX4:
	case VC_DIVX6:
	case VC_H263:
	case VC_H265:
		mrRet = PSR_VFilter_UpdateESIInfo_HdrDect(prPsrFtr);
		break;
	case VC_RV30:
	case VC_RV40:
	case VC_WMV1:
	case VC_WMV2:
	case VC_WMV3:
	case VC_DIVX3:
	case VC_MJPEG:
	case VC_H263_SORENSON:
	case VC_VP6:
	case VC_VP6A:
	case VC_VP8:
		mrRet = PSR_VFilter_UpdateESIInfo_NoHdrDect(prPsrFtr);
		break;
	default:
		mrRet = RET_DMX_UNEXPECT;
		break;
	}

	MM_RETURN(mrRet);
}
