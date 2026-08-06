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
 * @file dmx_psr_cc.c
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
#include "windows.h"
#include "winutil.h"
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_decrypt.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "dmx_decrypt.h"
#include "mm_debug.h"
#endif /* __linux__*/

#include "x_debug.h"
#include "x_rtos.h"

#include "dmx_def.h"
#include "dmx_dump.h"
#include "dmx_psr_cc.h"
#include "dmx_psr_filter.h"
#include "dmx_parser.h"
#include "dmx_psr_pbbuf.h"
#include "dmx_esm_if.h"
#include "dmx_pbbuf_if.h"
#include "dmx_mem.h"
#include "dmx_gau.h"
#include "dmx_spt.h"
#include "dmx_cpsa.h"
#include "dmx_spt_main.h"
#include "dmx_psr_decrypt.h"
#include "dmx_pfm.h"
#include "dmx_spt_os.h"

#ifndef __linux__
#pragma warning(disable : 4127) /*disable warning C4127: conditional expression is constant*/
#endif

#if DMX_PFM_TEST
EXTERN PSR_PFM g_rPsrPfm;
#endif /* DMX_PFM_TEST*/

EXTERN DMX_CLI_MAN_T g_rDmxCliMan;

/**/
/* PSR_CC_GetWaitTxBufSize*/
/* Get Tx data size, it does the following tasks:*/
/* 1. If current tx data is from memory, return tx src memory remained data size*/
/* 2. If current tx data is from pbbuf, check whether pbbuf exist:*/
/*	   1) if not exists, return 0*/
/*	   2) otherwise, if the needed tx data is across slot, the returned txsize is the len*/
/* from the current offset to the current pbbuf end address.*/
/*@Param  prPsrCC			 [IN] Parser CC handle*/
/**/
MRESULT PSR_CC_GetWaitTxBufSize(PSR_CC *prPsrCC, u64 *pu8Sz)
{
	PSR_FILTER	*prPsrFtr  = NULL;
	DMX_READ_BUFFER *pPbbuf    = NULL;
	u64		u8TxSize   = 0;
	u64		u8TxOffset = 0;

	if (!(NULL != prPsrCC)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	/* check source is memory or pbbuf*/
	prPsrFtr = (PSR_FILTER *)prPsrCC->pvActFilter;

	if (!(NULL != prPsrFtr)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	/* If tx source is memory, return remained data size*/
	if (!(prPsrFtr->u4Flag & FF_TX_PBBUF)) {
		*pu8Sz = prPsrCC->u4SrcMemLen - prPsrCC->u4MemOffset;
		MM_RETURN(RET_DMX_OK);
	}

	if (!(prPsrCC->u4Flag & CCF_PBBUF_EXIST)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	pPbbuf = &(prPsrCC->arPBBuf[prPsrCC->u4TxPBBufIdx]);
	/* if the tx data cross two slots, u8TxOffset is the first slot end, otherwise, is tx end offset*/
	u8TxOffset = DMX_MIN((prPsrCC->u8TxStartOffset + prPsrCC->u8TxLen),
					 (pPbbuf->u8SrcOffset + pPbbuf->u4PlaySize));

	u8TxSize = u8TxOffset - DMX_MAX(prPsrCC->u8TxCurrOffset, pPbbuf->u8SrcOffset);

	*pu8Sz = u8TxSize;

	MM_RETURN(RET_DMX_OK);
}

/**/
/* PSR_CC_CheckPBBuf*/
/* Condition: Parser CCis enabled, its state is CCS_TX, txstate is TXS_WAIT_PBBUF*/
/* Function: Get the pbbuf which contained the data corresponding to the designated fileoffset*/
/* 1. check whether the offset is in the pbbuf*/
/* 1) if in, set the txpbbufidx to be the pbbuf and set CCF_PBBUF_EXIST flag, change txstate to*/
/* be TXS_TX_OK*/
/* 2) if not in, get the needed pbbuf from the pbbuf's sent slots list, if can't get it, return FALSE,*/
/* otherwise, do 1)*/
/*@Param  prPsrCC			 [IN] Parser CC handle*/
/*@Param  u8Offset			  [IN] Designated checked file offset*/
/**/
MRESULT PSR_CC_CheckPBBuf(
	PSR_CC *prPsrCC,
	u64 u8Offset,
	bool   *pfgOffsetIn,
	E_PBBUF_CONTINUITY_TYPE_T *pePbbufCon,
	bool   fgSyncPb)
{
	MRESULT mrRet = RET_DMX_OK;
	u32	u4PbbufIdx = 0;
	u64	u8LastSrcOfst = DMX_INVALID_UINT64;
	bool	fgPbbufIn  = FALSE;

	if ((NULL == prPsrCC) ||
		(NULL == pfgOffsetIn) ||
		(NULL == pePbbufCon)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*pfgOffsetIn = FALSE;
	*pePbbufCon  = PBBUF_CONTINUOUS;

	/* check state and TX state*/
	if (CCS_TX != prPsrCC->eState)
		MM_RETURN(RET_DMX_OK);

	if ((TXS_WAIT_PBBUF != prPsrCC->eTxState) &&
		(TXS_TX_JUMP != prPsrCC->eTxState)) {
		MM_RETURN(RET_DMX_OK);
	}

	/* check data in enable flag*/
	if (!(prPsrCC->u4Flag & CCF_DATAIN_ENABLE))
		MM_RETURN(RET_DMX_OK);

	u8LastSrcOfst = DMX_INVALID_UINT64;

	for (u4PbbufIdx = 0; u4PbbufIdx < MAX_CACHE_PBBUF; u4PbbufIdx++) {
		if (NULL == prPsrCC->arPBBuf[u4PbbufIdx].pcPlayBuffer) {
			break;
		} else if (PSR_CC_IsOffsetInPbbuf(prPsrCC, u4PbbufIdx, u8Offset)) {
			fgPbbufIn = TRUE;
			prPsrCC->u4TxPBBufIdx = u4PbbufIdx;
			*pfgOffsetIn = TRUE;
			*pePbbufCon  = PBBUF_CONTINUOUS;
			break;
		}
		if (DMX_INVALID_UINT64 != u8LastSrcOfst) {
			if (u8LastSrcOfst != prPsrCC->arPBBuf[u4PbbufIdx].u8SrcOffset) {
				prPsrCC->u4TxPBBufJumpIdx = u4PbbufIdx;
				*pfgOffsetIn = FALSE;
				*pePbbufCon  = PBBUF_UNCONTINUOUS;
				MM_RETURN(RET_DMX_OK);
			}
		}

		u8LastSrcOfst = prPsrCC->arPBBuf[u4PbbufIdx].u8SrcOffset +
			prPsrCC->arPBBuf[u4PbbufIdx].u4DataSize;
	}

	while (!fgPbbufIn) {
		mrRet = PSR_CC_GetPBBufSlot(prPsrCC, &u4PbbufIdx);
		if (DMX_FAILED(mrRet)) {
			SplitterSetEOSForError(prPsrCC->pvSptHdl, mrRet);
			MM_RETURN(mrRet);
		}

		if (MAX_CACHE_PBBUF == u4PbbufIdx) {
			*pfgOffsetIn = FALSE;
			*pePbbufCon  = PBBUF_CONTINUOUS;
			MM_RETURN(RET_DMX_OK);
		}

		if (PSR_CC_IsPbbufUnCon(prPsrCC, u4PbbufIdx)) {
			prPsrCC->u4TxPBBufJumpIdx = u4PbbufIdx;
			*pfgOffsetIn = FALSE;
			*pePbbufCon  = PBBUF_UNCONTINUOUS;
			MM_RETURN(RET_DMX_OK);
		} else if (PSR_CC_IsOffsetInPbbuf(prPsrCC, u4PbbufIdx, u8Offset)) {
			prPsrCC->u4TxPBBufIdx = u4PbbufIdx;
			*pfgOffsetIn = TRUE;
			*pePbbufCon  = PBBUF_CONTINUOUS;
			fgPbbufIn = TRUE;
		}
	}

	*pfgOffsetIn = TRUE;
	*pePbbufCon  = PBBUF_CONTINUOUS;

	if ((DMX_INVALID_UINT64 != prPsrCC->rDecryptMan.u8DecryptStOft) &&
		(0 < prPsrCC->rDecryptMan.u4DecryptLen)) {
		if (fgSyncPb &&
			(prPsrCC->rDecryptMan.u8DecryptStOft <= u8Offset) &&
			(u8Offset < prPsrCC->rDecryptMan.u8DecryptStOft +
			prPsrCC->rDecryptMan.u4DecryptLen)) {
			if (DECRYPT_UNCOMPLETE == prPsrCC->rDecryptMan.eStatus) {
				bool fgDecryptEndOftIn = FALSE;
				E_PBBUF_CONTINUITY_TYPE_T ePbbufCon = PBBUF_CONTINUOUS;

				prPsrCC->rDecryptMan.eMethod = DECRYPT_BY_SW;

				mrRet = PSR_Decrypt_PbbufCheck(prPsrCC, prPsrCC->u4TxPBBufIdx,
					&fgDecryptEndOftIn, &ePbbufCon, TRUE);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[PSR] %s line %d -- fail in ")
						TEXT("PSR_Decrypt_PbbufCheck, mrRet: 0x%x\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
					MM_RETURN(mrRet);
				}

				if (PBBUF_CONTINUOUS == ePbbufCon) {
					if (!fgDecryptEndOftIn) {
						*pfgOffsetIn = FALSE;
						*pePbbufCon  = PBBUF_CONTINUOUS;
						MM_RETURN(RET_DMX_OK);
					}
#if DMX_PFM_TEST
					if (fgSyncPb)
						DmxPfmStmIncDecryptSyncPbCnt(prPsrCC->rDecryptMan.eDataType);
#endif
				} else {
					*pfgOffsetIn = TRUE;
					*pePbbufCon  = ePbbufCon;
				}
			}
		} else {
			mrRet = PSR_Decrypt_PbbufCheck(prPsrCC, prPsrCC->u4TxPBBufIdx, NULL, NULL,
				FALSE);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[PSR] %s line %d -- fail in ")
					TEXT("PSR_Decrypt_PbbufCheck, mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}
		}
	}

	/* update flag*/
	prPsrCC->u4Flag |= CCF_PBBUF_EXIST;
	/* change TX state*/
	PSR_CC_SetTxSt(prPsrCC, TXS_PBBUF_OK);

	MM_RETURN(RET_DMX_OK);
}

/**/
/* PSR_CC_TX_SubLoop*/
/* Condition: Sync Pbbuf/Tx data to fifo/memory*/
/* Function:  tx data sub loop, including tx data to fifo/memory or Sync Pbbuf*/
/*@Param  prPsrCC			 [IN] Parser CC handle*/
/**/
MRESULT PSR_CC_TX_SubLoop(PSR_CC *prPsrCC)
{
	PSR_FILTER *prPsrFtr = NULL;
	MRESULT		mrRet	= RET_DMX_OK;

	if (!(NULL != prPsrCC)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	prPsrFtr = (PSR_FILTER *)prPsrCC->pvActFilter;
	switch (prPsrCC->eTxState) {
	case TXS_WAIT_VFIFO_PTS_THRESHOLD:
	case TXS_WAIT_PBBUF:
	case TXS_PBBUF_OK:
	case TXS_WAIT_FIFO:
		if (!(NULL != prPsrFtr)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
		if (prPsrFtr->u4Flag & FF_TX_PBBUF) /* Tx Data From PBbuf to Fifo or Sync Pbbuf*/ {
			if (SPT_DATA_GRD == prPsrFtr->eType) { /* Tx to Ground, Ground Stream/PsrFilter*/
				mrRet = PSR_Filter_TxToGround(prPsrFtr);
			} else if (SPT_DATA_BUF == prPsrFtr->eType) {
				/* Sync Pbbuf, DMA Stream/PsrFilter (TXS_WAIT_PBBUF == prPsrCC->eTxState)*/
				PSR_DMASD *prDMASD = (PSR_DMASD *)prPsrFtr->pvFilterSpecific;

				if (prDMASD->fgHdrParsing)
					mrRet = PSR_Filter_Tx4HdrParsing(prPsrFtr);
				else
					mrRet = PSR_Filter_TxPbbuf(prPsrFtr, prPsrCC->u8TxCurrOffset);

			} else if (SPT_DATA_V == prPsrFtr->eType) { /* Tx Video Data from Pbbuf Into FIFO*/
				PSR_VFSD *prVFSD = (PSR_VFSD *)prPsrFtr->pvFilterSpecific;

				if (prVFSD->fgDummyTxWakeUp) {
					prVFSD->fgDummyTxWakeUp = FALSE;
					mrRet = PSR_Filter_AddDummyAU(prPsrFtr,
						prVFSD->fgDummyAUEnd, prVFSD->fgDummyCmdAU);
				} else {
					mrRet = PSR_Filter_TxPbbuf(prPsrFtr, prPsrCC->u8TxCurrOffset);
				}
			} else { /* Tx Audio/Other type Data from Pbbuf Into FIFO*/
				mrRet = PSR_Filter_TxPbbuf(prPsrFtr, prPsrCC->u8TxCurrOffset);
			}
		} else /* Tx Data From Mem to Fifo*/ {
			mrRet = PSR_Filter_TxMem(prPsrFtr);
		}
		break;

	case TXS_FIFO_OK:
		if (!(NULL != prPsrFtr)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
		mrRet = PSR_Filter_TX2Fifo(prPsrFtr, FALSE);
		break;

	case TXS_WAIT_HW:
		if (!(NULL != prPsrFtr)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
		if (prPsrFtr->u4Flag & FF_TX_TO_FIFO)
			mrRet = PSR_Filter_TriggerHALPTx(prPsrFtr);
		else
			mrRet = PSR_Filter_TriggerHALGTx(prPsrFtr);

		break;

	case TXS_WAIT_IRQ_PROC:
		if (!(NULL != prPsrFtr)) {
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
		mrRet = PSR_Filter_IRQ_Proc(prPsrFtr);
		break;

	case TXS_TX_JUMP: {
			/*/TODO:*/
			mrRet = SplitterChangeState(prPsrCC->pvSptHdl, SPLITTER_STATE_RUNING,
				SPLITTER_TX_STATE_JUMP);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[PSR_CC] %s line %d fail in ")
					TEXT("SplitterChangeState(pvSptHdl: 0x%x, RUNING, JUMP)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->pvSptHdl);
				MM_RETURN(mrRet);
			}
			PSR_CC_CBSplitter(prPsrCC, E_TX_JUMP, NULL);
			break;
		}
		break;
	case TXS_TX_OK:
	case TXS_TXING:
	case TXS_WAIT_DECRYPT:
	default:
		DMXLOG_DEBUG(TEXT("[PSR_CC] %s May Get Default State.0x%x\r\n"),
			DMX_FUNC_NAME, prPsrCC->eTxState);
		break;
	}

	MM_RETURN(mrRet);
}


MRESULT PSR_CC_Abort_SubLoop(PSR_CC *prPsrCC)
{
	MRESULT mrRet = RET_DMX_OK;

	if (TXS_WAIT_IRQ_PROC == prPsrCC->eTxState) {
		mrRet = PSR_Filter_IRQ_Proc(prPsrCC->pvActFilter);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}


MRESULT PSR_CC_Pause_SubLoop(PSR_CC *prPsrCC)
{
	MRESULT mrRet = RET_DMX_OK;

	if (TXS_WAIT_IRQ_PROC == prPsrCC->eTxState) {
		mrRet = PSR_Filter_IRQ_Proc(prPsrCC->pvActFilter);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

/**/
/* PSR_CC_Create*/
/* Get one unused Parser CC instance, reset it, alloc Cmd Queue Entry Array for it,*/
/* set splitter handle to PVR*/
/*@Param pvSptHdl		[IN] Splitter handle*/
/*@Param pvPBBuf   [IN] PBBuf handle*/
/*@Param ppvPsrCC [IN] Parser CC handle*/
/**/
MRESULT PSR_CC_Create(void *pvDmxInst, void *pvSptHdl, void *pvPBBuf, void **ppvPsrCC)
{
	PSR_CC	*prPsrCC = NULL;
	u32	u4Idx	 = 0;
	u32	u4FHIdx  = 0;
	MRESULT mrRet	 = RET_DMX_OK;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == ppvPsrCC) {
		DMXLOG_ERROR(TEXT("[PSR_CC] %s fail for invalid args (pvSptHdl: 0x%x,")
			TEXT(" pvPBBuf: 0x%x, ppvPsrCC: 0x%x)\r\n"),
			DMX_FUNC_NAME, pvSptHdl, pvPBBuf, ppvPsrCC);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*ppvPsrCC = NULL;

	PSR_LOCK;

	/* Done in i4PSR_Init, close here, or assert!!*/
	/* Todo: If need do here, maybe move g_rPsrMan.aprPsrFtrs operation in related parser filter module ???*/

	/* find a empty cc slot*/
	for (u4Idx = 0; u4Idx < MAX_PSR_CC_CNT; u4Idx++) {
		if (0 == (g_rPsrMan.aprPsrCCs[u4Idx]->u4Flag & CCF_USED)) {
			prPsrCC = g_rPsrMan.aprPsrCCs[u4Idx];
			prPsrCC->u4Idx = u4Idx;
			break;
		}
	}

	if (MAX_PSR_CC_CNT == u4Idx) {
		DMXLOG_ERROR(TEXT("[PSR_CC] %s fail for no unused PSR CC ")
			TEXT("Instance(pvSptHdl: 0x%x)\r\n"), DMX_FUNC_NAME, pvSptHdl);
		PSR_UNLOCK;
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}

	PSR_CC_LOCK_INIT(prPsrCC->rLock, mrRet);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR_CC] %s line %d fail in PSR_CC_LOCK_INIT,")
			TEXT(" mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		PSR_UNLOCK;
		MM_RETURN(mrRet);
	}

	smp_mb();

	DMX_NewMemory((sizeof(DMX_CMDQ_TX_ENTRY_T) * DMX_MAX_TX_CNT_FOR_CMD_Q),
		prPsrCC->pvCmdQTxEntryBuffer);
	if (NULL == prPsrCC->pvCmdQTxEntryBuffer) {
		DMXLOG_ERROR(TEXT("[PSR_CC] %s fail in dmx_mem_alloc(NO MEM),")
			TEXT(" (pvSptHdl: 0x%x)\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		PSR_UNLOCK;
		MM_RETURN(RET_DMX_NO_MEM);
	}

	prPsrCC->u4Flag					= CCF_USED;
	prPsrCC->pvPBBuf					= pvPBBuf;
	prPsrCC->pvSptHdl					= pvSptHdl;
	prPsrCC->eState					= CCS_INIT;
	prPsrCC->pvActFilter				= NULL;
	prPsrCC->ptrSrcMemSa				= 0;
	prPsrCC->u4SrcMemLen			= 0;
	prPsrCC->fgWakingWaitTx			= FALSE;

	prPsrCC->fgTxMem2Fifo			= FALSE;

	prPsrCC->fgLastMem				= FALSE;

	prPsrCC->fgNeedHighBitRateProc	= FALSE;

	prPsrCC->pvHwData				= NULL;

	prPsrCC->u4AVStmFlags			= 0;

	prPsrCC->u4AVStmPlayFlags		= 0;

	prPsrCC->u4TxPBBufJumpIdx		= MAX_CACHE_PBBUF;

	for (u4FHIdx = 0; u4FHIdx < MAX_PSR_FILTER_PER_CC; u4FHIdx++)
		prPsrCC->apvFtr[u4FHIdx] = NULL;

	prPsrCC->u4PsrFtrCnt = 0;

#if ENABLE_DMX_ADVANCED_VER
	prPsrCC->fgInsertHdr = FALSE;
	prPsrCC->u4InsertHdrLen = 0;
	dmx_memset(prPsrCC->au1InsertHdrBuf, 0, sizeof(prPsrCC->au1InsertHdrBuf));
#endif /* ENABLE_DMX_ADVANCED_VER*/

	prPsrCC->pvDmxInst				= pvDmxInst;

	dmx_memset(&(prPsrCC->rDecryptMan), 0, sizeof(PSR_DECRYPT_MAN_T));

	smp_mb();

	mrRet = PSR_CC_SetDecryptType(prPsrCC, DECRYPT_NONE);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR_CC] %s line %d fail in ")
			TEXT("PSR_CC_SetDecryptType(DECRYPT_NONE), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		prPsrCC->u4Flag &= ~CCF_USED;
		DMX_FreeMemory(prPsrCC->pvCmdQTxEntryBuffer);
		PSR_UNLOCK;
		MM_RETURN(mrRet);
	}

	*ppvPsrCC = (void *)prPsrCC;
	PSR_UNLOCK;

	MM_RETURN(RET_DMX_OK);
}

/**/
/* PSR_CC_Destroy*/
/* Release ALL pbbuf slots, Reset parser CC*/
/*@Param pvSptHdl		[IN] Splitter handle*/
/*@Param pvPBBuf   [IN] PBBuf handle*/
/*@Param ppvPsrCC [IN] Parser CC handle*/
/**/
MRESULT PSR_CC_Destroy(void *pvPsrCC)
{
	PSR_CC	*prPsrCC = (PSR_CC *)pvPsrCC;
	u32	u4FHIdx = 0;
	MRESULT mrRet = RET_DMX_OK;
	s32 i = 0;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	for (i = 0; i < MAX_PSR_CC_CNT; ++i) {
		if ((prPsrCC == g_rPsrMan.aprPsrCCs[i]) &&
			(prPsrCC->u4Flag & CCF_USED)) {
			i = MAX_PSR_CC_CNT << 1;
			break;
		}
	}
	if (i == MAX_PSR_CC_CNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PSR_LOCK;

	DMXLOG_DEBUG(TEXT("[PSR_CC] %s line %d -- PsrCC 0x%x, St:0x%x, TxSt:0x%x,")
		TEXT(" Flag:0x%x, TxOfst:0x%llx, TxLen:0x%llx, CurTxOfst:0x%llx\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		prPsrCC, prPsrCC->eState, prPsrCC->eTxState, prPsrCC->u4Flag,
		prPsrCC->u8TxStartOffset, prPsrCC->u8TxLen, prPsrCC->u8TxCurrOffset);

	/* check whether has filter attach*/
	for (u4FHIdx = 0; u4FHIdx < MAX_PSR_FILTER_PER_CC; u4FHIdx++) {
		if (NULL != prPsrCC->apvFtr[u4FHIdx]) {
			PSR_UNLOCK;
			MM_RETURN(RET_DMX_OPERATE_FORBID);
		}
	}

	if (prPsrCC->u4Flag & CCF_PBBUF_EXIST)
		PSR_CC_ReleaseAllPBBuf(pvPsrCC);

	PBBUF_CancelReadSlot(prPsrCC->pvSptHdl);

	if (prPsrCC->pvCmdQTxEntryBuffer != NULL) {
		DMX_FreeMemory(prPsrCC->pvCmdQTxEntryBuffer);
		prPsrCC->pvCmdQTxEntryBuffer = NULL;
	}

	prPsrCC->u4Flag = 0;
	{
		u32 u4Idx = 0;
		bool fgCCEnableFlag = FALSE;

		/*Check CC Enable Handle*/
		for (u4Idx = 0; u4Idx < MAX_PSR_CC_CNT; u4Idx++) {
			if (prPsrCC->u4Flag & CCF_USED) {
				DMXLOG_TRACE(TEXT("[PSR_CC] %s line %d -- Check CC's Flag,")
					TEXT(" u4Idx(%d)\r\n"), DMX_FUNC_NAME, DMX_LINE_NO, u4Idx);
				fgCCEnableFlag = TRUE;
			}
		}
	}

	mrRet = PSR_Decrypt_Release(prPsrCC);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR_CC] %s line %d fail in PSR_Decrypt_Release,")
			TEXT(" mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
	}


	mrRet = PSR_CC_SetDecryptType(prPsrCC, DECRYPT_NONE);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR_CC] %s line %d fail in ")
			TEXT("PSR_CC_SetDecryptType(DECRYPT_NONE), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
	}

	PSR_CC_LOCK_UNINIT(prPsrCC->rLock);

	/* reset memory*/
	dmx_memset((void *)prPsrCC, 0, sizeof(PSR_CC));

	prPsrCC->u4Idx = DMX_INVALID_UINT32;
	prPsrCC->pvDmxInst = NULL;

	PSR_UNLOCK;

	MM_RETURN(RET_DMX_OK);
}

/*
*/
/* PSR_CC_SetLastMemState*/
/* Set Parser CC LastMem Flag*/
/*@Param pvPsrCC	[IN] Parser CC handle*/
/*@Param fgLastMem	[IN] indicate last mem*/
/**/
MRESULT PSR_CC_SetLastMemState(void *pvPsrCC, bool fgLastMem)
{
	PSR_CC *prPsrCC = (PSR_CC *)pvPsrCC;

	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	prPsrCC->fgLastMem = fgLastMem;

	MM_RETURN(RET_DMX_OK);
}

/**/
/* PSR_CC_MainLoop*/
/* This function is the main function for tx data to fifo, sync pbbuf.*/
/* If Parser CC received RELEASE_PBBUF notify, release all pbbuf.*/
/* Otherwise, call corresponding subloop function*/
/*@Param pvPsrCC	[IN] Parser CC handle*/
/**/
MRESULT PSR_CC_MainLoop(void *pvPsrCC)
{
	MRESULT mrRet = RET_DMX_OK;
	PSR_CC	*prPsrCC = (PSR_CC *)pvPsrCC;
	s32 i = 0;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	for (i = 0; i < MAX_PSR_CC_CNT; ++i) {
		if ((prPsrCC == g_rPsrMan.aprPsrCCs[i]) &&
			(prPsrCC->u4Flag & CCF_USED)) {
			i = MAX_PSR_CC_CNT << 1;
			break;
		}
	}
	if (i == MAX_PSR_CC_CNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PSR_CC_LOCK(prPsrCC->rLock);
	if (prPsrCC->u4Flag & CCF_PBBUF_RELEASE_NOTIFY) {
		PSR_CC_ReleaseAllPBBuf(prPsrCC);
		prPsrCC->u4Flag &= (~CCF_PBBUF_RELEASE_NOTIFY);
		prPsrCC->fgWakingWaitTx = FALSE;
		PSR_CC_UNLOCK(prPsrCC->rLock);
		MM_RETURN(mrRet);
	}

	switch (prPsrCC->eState) {
	case CCS_TX:
		mrRet = PSR_CC_TX_SubLoop(prPsrCC);
		break;

	case CCS_ABORT:
		mrRet = PSR_CC_Abort_SubLoop(prPsrCC);
		break;

	case CCS_PAUSE:
		mrRet = PSR_CC_Pause_SubLoop(prPsrCC);
		break;

	default:
		DMXLOG_DEBUG(TEXT("[PSR_CC] %s Get default State.0x%x\r\n"),
			DMX_FUNC_NAME, prPsrCC->eState);
		break;
	}

	prPsrCC->fgWakingWaitTx = FALSE;

	PSR_CC_UNLOCK(prPsrCC->rLock);

	MM_RETURN(mrRet);
}


/**/
/* PSR_CC_PauseTx*/
/* when the parser CC 's state isn't CCS_TX, this operation is forbidden.*/
/* set its state to be CCS_PAUSE*/
/* if Parser CC's txstate is not txing or WAIT_IRQ_PROC, notify Splitter Pause Done*/
/* If Parser CC received RELEASE_PBBUF notify, release all pbbuf.*/
/*@Param prPsrCC		[IN] Parser CC handle*/
/**/
MRESULT PSR_CC_PauseTx(PSR_CC *prPsrCC, bool *pfgNeedToPause)
{
	MRESULT mrRet = RET_DMX_OK;
	s32 i = 0;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	for (i = 0; i < MAX_PSR_CC_CNT; ++i) {
		if ((prPsrCC == g_rPsrMan.aprPsrCCs[i]) &&
			(prPsrCC->u4Flag & CCF_USED)) {
			i = MAX_PSR_CC_CNT << 1;
			break;
		}
	}
	if (i == MAX_PSR_CC_CNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PSR_CC_LOCK(prPsrCC->rLock);
	DMXLOG_DEBUG(TEXT("[PSR_CC] %s line %d -- CCHdr:0x%x, eSt:0x%x, ")
		TEXT("eTxSt:0x%x, eType:0x%x, TxS:0x%llx, TxL:0x%llx, TxC:0x%llx\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		prPsrCC, prPsrCC->eState, prPsrCC->eTxState,
		((NULL != (PSR_FILTER *)prPsrCC->pvActFilter) ?
		((PSR_FILTER *)prPsrCC->pvActFilter)->eType : -1),
		prPsrCC->u8TxStartOffset, prPsrCC->u8TxLen, prPsrCC->u8TxCurrOffset);

	if (NULL == pfgNeedToPause) {
		PSR_CC_UNLOCK(prPsrCC->rLock);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*pfgNeedToPause = TRUE;

	if (CCS_TX != prPsrCC->eState) {
		*pfgNeedToPause = FALSE;
		PSR_CC_UNLOCK(prPsrCC->rLock);
		MM_RETURN(RET_DMX_OK);
	}

	prPsrCC->eState = CCS_PAUSE;

	if ((TXS_TXING != prPsrCC->eTxState) &&
		(TXS_WAIT_IRQ_PROC != prPsrCC->eTxState)) {
		/* Not to check fake tx condition if pause, 080508*/
		mrRet = PSR_CC_CBSplitter(prPsrCC, E_PAUSE_DONE, NULL);
	}

	DMXLOG_DEBUG(TEXT("[PSR_CC] %s line %d -- CCHdr:0x%x, eSt:0x%x, ")
		TEXT("eTxSt:0x%x, eType:0x%x, TxS:0x%llx, TxL:0x%llx, TxC:0x%llx\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		prPsrCC, prPsrCC->eState, prPsrCC->eTxState,
		((NULL != (PSR_FILTER *)prPsrCC->pvActFilter) ?
		((PSR_FILTER *)prPsrCC->pvActFilter)->eType : -1),
		prPsrCC->u8TxStartOffset, prPsrCC->u8TxLen, prPsrCC->u8TxCurrOffset);
	PSR_CC_UNLOCK(prPsrCC->rLock);

	MM_RETURN(mrRet);
}


/**/
/* PSR_CC_ResumeTx*/
/* 1. when the parser CC 's state isn't CCS_PAUSE, this operation is forbidden.*/
/* 2. When the Parser CC's active parser filter is Ground, call PSR_Filter_TxToGround*/
/* 3. When the Parser CC's active parser filter is not DMA(sync pbbuf), Check whether TX end:*/
/*	   1) if is, set Parser CC's state to be CCS_INIT, txstate to be TXS_TX_OK, notify Splitter TX done*/
/*	   2) if not, set Parser CC's state to be CCS_TX, set txstate to be TXS_WAIT_PBBUF, call PSR_Filter_TxPbbuf*/
/* 4. When the Parser CC's active parser filter is DMA(sync pbbuf), set Parser CC's state to be CCS_TX,*/
/*	   txstate to be TXS_WAIT_PBBUF, call Tx4HdrParsing or TXPbbuf:*/
/*@Param prPsrCC		[IN] Parser CC handle*/
/**/
MRESULT PSR_CC_ResumeTx(PSR_CC *prPsrCC)
{
	PSR_FILTER *prPsrFtr = NULL;
	void	*hPsrFtr = NULL;
	MRESULT    mrRet = RET_DMX_OK;
	s32 i = 0;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	for (i = 0; i < MAX_PSR_CC_CNT; ++i) {
		if ((prPsrCC == g_rPsrMan.aprPsrCCs[i]) &&
			(prPsrCC->u4Flag & CCF_USED)) {
			i = MAX_PSR_CC_CNT << 1;
			break;
		}
	}
	if (i == MAX_PSR_CC_CNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PSR_CC_LOCK(prPsrCC->rLock);
	hPsrFtr = prPsrCC->pvActFilter;
	prPsrFtr = (PSR_FILTER *)hPsrFtr;
	DMX_ASSERT(NULL != prPsrFtr);
	if (CCS_PAUSE != prPsrCC->eState) {
		PSR_CC_UNLOCK(prPsrCC->rLock);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	if (prPsrFtr->eType == SPT_DATA_GRD) {
		mrRet = PSR_Filter_TxToGround(hPsrFtr);
		PSR_CC_UNLOCK(prPsrCC->rLock);
		MM_RETURN(mrRet);
	}

	DMXLOG_DEBUG(TEXT("[PSR_CC] %s line %d -- CCHdr:0x%x, eSt:0x%x, ")
		TEXT("eTxSt:0x%x, eType:0x%x, TxS:0x%llx, TxL:0x%llx, TxC:0x%llx\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		prPsrCC, prPsrCC->eState, prPsrCC->eTxState, prPsrFtr->eType,
		prPsrCC->u8TxStartOffset, prPsrCC->u8TxLen, prPsrCC->u8TxCurrOffset);

	if (prPsrFtr->eType != SPT_DATA_BUF) {
		if (prPsrCC->u8TxCurrOffset == (prPsrCC->u8TxStartOffset + prPsrCC->u8TxLen)) {
			prPsrCC->fgUseCmdQ = FALSE;
			prPsrCC->fgAUByCmdQEnd = FALSE;
			/* check whether tx complete*/
			/* change state to init and tx state to tx ok*/
			prPsrCC->eState = CCS_INIT;
			PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);
			PSR_CC_CBSplitter(prPsrCC, E_TX_DONE, NULL);
		} else {
			prPsrCC->eState = CCS_TX;
			PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);
			mrRet = PSR_Filter_TxPbbuf(prPsrFtr, prPsrCC->u8TxCurrOffset);
		}
	} else {
		PSR_DMASD *prDMASD = (PSR_DMASD *)prPsrFtr->pvFilterSpecific;

		if (NULL == prDMASD) {
			DMXLOG_ERROR(TEXT("[PSR_CC] %s fail for PSR_DMASD has't been")
				TEXT(" allocated\r\n"), DMX_FUNC_NAME);
			PSR_CC_UNLOCK(prPsrCC->rLock);
			MM_RETURN(RET_DMX_NO_INIT);
		}

		prPsrCC->eState = CCS_TX;
		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);

		if (prDMASD->fgHdrParsing)
			mrRet = PSR_Filter_Tx4HdrParsing(prPsrFtr);
		else
			mrRet = PSR_Filter_TxPbbuf(prPsrCC->pvActFilter, prPsrCC->u8TxCurrOffset);

	}

	DMXLOG_DEBUG(TEXT("[PSR_CC] %s line %d -- CCHdr:0x%x, eSt:0x%x, eTxSt:0x%x,")
		TEXT(" eType:0x%x, TxS:0x%llx, TxL:0x%llx, TxC:0x%llx\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		prPsrCC, prPsrCC->eState, prPsrCC->eTxState, prPsrFtr->eType,
		prPsrCC->u8TxStartOffset, prPsrCC->u8TxLen, prPsrCC->u8TxCurrOffset);

	PSR_CC_UNLOCK(prPsrCC->rLock);

	MM_RETURN(mrRet);
}


/**/
/* PSR_CC_AbortTx*/
/* 1. if the parser CC 's state is TXing or Wait IRQ Proc, set Parser CC's state to be CCS_ABORT*/
/* 2. otherwise, change Parser CC's state to be CCS_INIT, txstate to be TXS_TX_OK, notify Splitter Abort Done*/
/*@Param prPsrCC		[IN] Parser CC handle*/
/**/
MRESULT PSR_CC_AbortTx(PSR_CC *prPsrCC)
{
	s32 i = 0;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	for (i = 0; i < MAX_PSR_CC_CNT; ++i) {
		if ((prPsrCC == g_rPsrMan.aprPsrCCs[i]) &&
			(prPsrCC->u4Flag & CCF_USED)) {
			i = MAX_PSR_CC_CNT << 1;
			break;
		}
	}
	if (i == MAX_PSR_CC_CNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PSR_CC_LOCK(prPsrCC->rLock);
	DMXLOG_DEBUG(TEXT("[PSR] %s enter, ParserCC's eState: 0x%x,")
		TEXT(" eTxState: 0x%x\r\n"),
		DMX_FUNC_NAME, prPsrCC->eState, prPsrCC->eTxState);

	prPsrCC->fgNeedHighBitRateProc = FALSE;

	if ((TXS_TXING == prPsrCC->eTxState) ||
		(TXS_WAIT_IRQ_PROC == prPsrCC->eTxState)) {
		prPsrCC->eState = CCS_ABORT;
	} else {
		/* change state to init*/
		prPsrCC->eState = CCS_INIT;
		PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);
		prPsrCC->u8NormalWaitPts = INVALID_TIMESTAMP;
		prPsrCC->fgWakingWaitTx = FALSE;
		PSR_CC_CBSplitter(prPsrCC, E_ABORT_DONE, NULL);
	}
	PSR_CC_UNLOCK(prPsrCC->rLock);

	MM_RETURN(RET_DMX_OK);
}

/**/
/* PSR_CC_Enable*/
/* 1. If enable, Parser CC, set CCF_DATAIN_ENABLE flag, Notify Splitter Wakeup*/
/* 2. otherwise, Release ALL Pbbufs, clear CCF_DATAIN_ENABLE flag.*/
/*@Param prPsrCC		 [IN] Parser driver control center handle*/
/*@Param fgEnable		  [IN] TRUE: turn on parser driver. FALSE: turn off parser driver*/
/**/
MRESULT PSR_CC_Enable(PSR_CC *prPsrCC, bool fgEnable)
{
	s32 i = 0;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	for (i = 0; i < MAX_PSR_CC_CNT; ++i) {
		if ((prPsrCC == g_rPsrMan.aprPsrCCs[i]) &&
			(prPsrCC->u4Flag & CCF_USED)) {
			i = MAX_PSR_CC_CNT << 1;
			break;
		}
	}
	if (i == MAX_PSR_CC_CNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PSR_CC_LOCK(prPsrCC->rLock);
	if (fgEnable) {
		prPsrCC->fgVidPass	= FALSE;
		prPsrCC->u4AVStmPlayFlags = 0;
		prPsrCC->u4Flag |= CCF_DATAIN_ENABLE;
		prPsrCC->u4Flag &= (~CCF_TIMEHOLD);
		#ifdef MM_SUPPORT_DIVXHT31
		prPsrCC->u8BaseSTC = INVALID_TIMESTAMP;
		#endif /* MM_SUPPORT_DIVXHT31*/
		PSR_CC_CBSplitter(prPsrCC, E_WAKEUP_ME, NULL);
	} else /* disable*/ {
		if (prPsrCC->u4Flag & CCF_PBBUF_EXIST)
			PSR_CC_ReleaseAllPBBuf(prPsrCC);

		prPsrCC->u4Flag &= (~CCF_DATAIN_ENABLE);
		prPsrCC->u4Flag &= (~CCF_TIMEHOLD);
		prPsrCC->u4Flag &= (~CCF_FIFOHOLD);

		prPsrCC->u4AVStmPlayFlags = 0;
	}
	PSR_CC_UNLOCK(prPsrCC->rLock);

	MM_RETURN(RET_DMX_OK);
}


/**/
/* PSR_CC_GetCurrentOffset*/
/* Get Current Tx File Offset*/
/**/
MRESULT PSR_CC_GetCurrentOffset(PSR_CC *prPsrCC, u64 *pu8Offset)
{
	s32 i = 0;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	for (i = 0; i < MAX_PSR_CC_CNT; ++i) {
		if ((prPsrCC == g_rPsrMan.aprPsrCCs[i]) &&
			(prPsrCC->u4Flag & CCF_USED)) {
			i = MAX_PSR_CC_CNT << 1;
			break;
		}
	}
	if (i == MAX_PSR_CC_CNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (NULL == pu8Offset) {
		DMXLOG_ERROR(TEXT("[PSR_CC] %s fail for invalid args,")
			TEXT(" (prPsrCC: 0x%x, pu8Offset: 0x%x)\r\n"),
			DMX_FUNC_NAME, prPsrCC, pu8Offset);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*pu8Offset = 0;

	*pu8Offset = prPsrCC->u8TxCurrOffset;

	MM_RETURN(RET_DMX_OK);
}

/**/
/* PSR_CC_AttachFilter*/
/* Add Parser Filter to Parser CC*/
/**/
MRESULT PSR_CC_AttachFilter(PSR_CC *prPsrCC, void *hPsrFtr)
{
	u32 u4StmType = 0;
	u32 u4Idx;
	s32 i = 0;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	for (i = 0; i < MAX_PSR_CC_CNT; ++i) {
		if ((prPsrCC == g_rPsrMan.aprPsrCCs[i]) &&
			(prPsrCC->u4Flag & CCF_USED)) {
			i = MAX_PSR_CC_CNT << 1;
			break;
		}
	}
	if (i == MAX_PSR_CC_CNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (NULL == hPsrFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	for (i = 0; i < MAX_FILTER_COUNT; ++i) {
		if (((PSR_FILTER *)hPsrFtr == g_rPsrMan.aprPsrFtrs[i]) &&
			(((PSR_FILTER *)hPsrFtr)->u4Flag & FF_USED)) {
			i = MAX_FILTER_COUNT << 1;
			break;
		}
	}
	if (i == MAX_FILTER_COUNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PSR_CC_LOCK(prPsrCC->rLock);
	for (u4Idx = 0; u4Idx < MAX_PSR_FILTER_PER_CC; u4Idx++) {
		if (hPsrFtr == prPsrCC->apvFtr[u4Idx]) {
			DMXLOG_ERROR(TEXT("[PSR_CC] %s fail for the PsrFtr:0x%x ")
				TEXT("already exists\r\n"),
				DMX_FUNC_NAME, hPsrFtr);
			PSR_CC_UNLOCK(prPsrCC->rLock);
			MM_RETURN(RET_DMX_ALREADY_EXIST);
		}
	}

	/* save filter handle into cc*/
	for (u4Idx = 0; u4Idx < MAX_PSR_FILTER_PER_CC; u4Idx++) {
		if (NULL == prPsrCC->apvFtr[u4Idx]) {
			prPsrCC->apvFtr[u4Idx] = hPsrFtr;
			prPsrCC->u4PsrFtrCnt++;
			if (prPsrCC->u4PsrFtrCnt > MAX_PSR_FILTER_PER_CC) {
				DMXLOG_ERROR(TEXT("[PSR_CC] %s fail for PsrCC(0x%x)'s ")
					TEXT("existed filter Count > Limit(%d)\r\n"),
					DMX_FUNC_NAME, prPsrCC, MAX_PSR_FILTER_PER_CC);
				PSR_CC_UNLOCK(prPsrCC->rLock);
				MM_RETURN(RET_DMX_OVER_LIMIT);
			}
			break;
		}
	}

	if (MAX_PSR_FILTER_PER_CC == u4Idx) {
		DMXLOG_ERROR(TEXT("[PSR_CC] %s fail for can't find the free ")
			TEXT("handle to set the PsrFtr:0x%x\r\n"),
			DMX_FUNC_NAME, hPsrFtr);
		PSR_CC_UNLOCK(prPsrCC->rLock);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}

	((PSR_FILTER *)hPsrFtr)->pvPsrCC = prPsrCC;

	u4StmType = ((PSR_FILTER *)hPsrFtr)->u4StmType;

	if ((SPT_DATA_V == u4StmType) ||
		(SPT_DATA_A == u4StmType)) {
		prPsrCC->u4AVStmFlags |= (u32)(1 << u4StmType);
	}

	DMXLOG_DEBUG(TEXT("[PSR_CC] %s --- prPsrCC->u4AVStmFlags: 0x%x, ")
		TEXT("prPsrCC->u4AVStmPlayFlags: 0x%x\r\n"),
		DMX_FUNC_NAME, prPsrCC->u4AVStmFlags, prPsrCC->u4AVStmPlayFlags);

	PSR_CC_UNLOCK(prPsrCC->rLock);

	MM_RETURN(RET_DMX_OK);
}

/**/
/* PSR_CC_DetachFilter*/
/* Remove the designated Parser Filter from Parser CC*/
/**/
MRESULT PSR_CC_DetachFilter(PSR_CC *prPsrCC, void *hPsrFtr)
{
	u32 u4StmType = 0;
	u32 u4Idx = 0;
	s32 i = 0;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	for (i = 0; i < MAX_PSR_CC_CNT; ++i) {
		if ((prPsrCC == g_rPsrMan.aprPsrCCs[i]) &&
			(prPsrCC->u4Flag & CCF_USED)) {
			i = MAX_PSR_CC_CNT << 1;
			break;
		}
	}
	if (i == MAX_PSR_CC_CNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (NULL == hPsrFtr) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	for (i = 0; i < MAX_FILTER_COUNT; ++i) {
		if (((PSR_FILTER *)hPsrFtr == g_rPsrMan.aprPsrFtrs[i]) &&
			(((PSR_FILTER *)hPsrFtr)->u4Flag & FF_USED)) {
			i = MAX_FILTER_COUNT << 1;
			break;
		}
	}
	if (i == MAX_FILTER_COUNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PSR_CC_LOCK(prPsrCC->rLock);
	DMXLOG_DEBUG(TEXT("[PSR_CC] %s, u4CCHdl:0x%x, u4FtrHdl:0x%x, CC mem:0x%x,")
		TEXT(" Ftrs mem:0x%x\r\n"),
		DMX_FUNC_NAME, prPsrCC, hPsrFtr,
		g_rPsrMan.aprPsrCCs[0], g_rPsrMan.aprPsrFtrs[0]);

	/* remove filter from CC*/
	for (u4Idx = 0; u4Idx < MAX_PSR_FILTER_PER_CC; u4Idx++) {
		if (hPsrFtr == prPsrCC->apvFtr[u4Idx]) {
			prPsrCC->apvFtr[u4Idx] = NULL;
			if (0 == prPsrCC->u4PsrFtrCnt) {
				DMXLOG_ERROR(TEXT("[PSR_CC] %s fail for PsrCC(0x%x)'s existed")
					TEXT(" filter Count already is 0\r\n"),
					DMX_FUNC_NAME, prPsrCC, MAX_PSR_FILTER_PER_CC);
				PSR_CC_UNLOCK(prPsrCC->rLock);
				MM_RETURN(RET_DMX_OVER_LIMIT);
			}
			prPsrCC->u4PsrFtrCnt--;
			break;
		}
	}

	if (MAX_PSR_FILTER_PER_CC == u4Idx) {
		DMXLOG_DEBUG(TEXT("[PSR_CC] %s fail for can't find the designated")
			TEXT(" PsrFtr:0x%x\r\n"),
			DMX_FUNC_NAME, hPsrFtr);
		PSR_CC_UNLOCK(prPsrCC->rLock);
		MM_RETURN(RET_DMX_OK);
	}

	u4StmType = ((PSR_FILTER *)hPsrFtr)->u4StmType;

	if ((SPT_DATA_V == u4StmType) ||
		(SPT_DATA_A == u4StmType)) {
		prPsrCC->u4AVStmFlags &= ~((u32)(1 << u4StmType));
	}

	((PSR_FILTER *)hPsrFtr)->pvPsrCC = NULL;

	PSR_CC_UNLOCK(prPsrCC->rLock);

	MM_RETURN(RET_DMX_OK);
}

/**/
/* PSR_CC_SetTxSt*/
/* Change Parser CC's txstate*/
/**/
MRESULT PSR_CC_SetTxSt(PSR_CC *prPsrCC, PSR_TX_STATE eTxSt)
{
	s32 i = 0;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	for (i = 0; i < MAX_PSR_CC_CNT; ++i) {
		if ((prPsrCC == g_rPsrMan.aprPsrCCs[i]) &&
			(prPsrCC->u4Flag & CCF_USED)) {
			i = MAX_PSR_CC_CNT << 1;
			break;
		}
	}
	if (i == MAX_PSR_CC_CNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrCC->fgWakingWaitTx = FALSE;
	prPsrCC->eTxState = eTxSt;
	smp_mb();

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_CC_SetState(PSR_CC *prPsrCC, PSR_CC_STATE eState)
{
	s32 i = 0;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	for (i = 0; i < MAX_PSR_CC_CNT; ++i) {
		if ((prPsrCC == g_rPsrMan.aprPsrCCs[i]) &&
			(prPsrCC->u4Flag & CCF_USED)) {
			i = MAX_PSR_CC_CNT << 1;
			break;
		}
	}
	if (i == MAX_PSR_CC_CNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrCC->eState = eState;
	smp_mb();

	MM_RETURN(RET_DMX_OK);
}

/**/
/* PSR_CC_IsPause*/
/* Check whether Parser CC's state is CCS_PAUSE*/
/**/
bool PSR_CC_IsPause(PSR_CC *prPsrCC)
{
	bool fgRet = FALSE;

	PSR_CC_LOCK(prPsrCC->rLock);
	if ((!g_fgPSRInit) ||
		(NULL == prPsrCC) ||
		(0 == (prPsrCC->u4Flag & CCF_USED))) {
		PSR_CC_UNLOCK(prPsrCC->rLock);
		return FALSE;
	}

	fgRet = (CCS_PAUSE == prPsrCC->eState);
	PSR_CC_UNLOCK(prPsrCC->rLock);

	return fgRet;
}

MRESULT PSR_CC_GetWMVParsingMode(PSR_CC *prPsrCC, u8 *pucWMVParsingMode)
{
	PSR_FILTER *prPsrFtr = NULL;
	s32 i = 0;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	for (i = 0; i < MAX_PSR_CC_CNT; ++i) {
		if ((prPsrCC == g_rPsrMan.aprPsrCCs[i]) &&
			(prPsrCC->u4Flag & CCF_USED)) {
			i = MAX_PSR_CC_CNT << 1;
			break;
		}
	}
	if (i == MAX_PSR_CC_CNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrFtr = (PSR_FILTER *)prPsrCC->pvActFilter;

	if ((NULL == pucWMVParsingMode) ||
		(NULL == prPsrFtr)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*pucWMVParsingMode = 0;

	if (SPT_DATA_V == prPsrFtr->eType) {
		PSR_VFSD *prVFSD = (PSR_VFSD *)prPsrFtr->pvFilterSpecific;

		if (prVFSD != NULL)
			*pucWMVParsingMode = prVFSD->uWVC1Mode;

	} else {
		DMXLOG_ERROR(TEXT("[PSR] %s fail for PsrFtr's Type(%d) is not Video\r\n"),
			DMX_FUNC_NAME, prPsrFtr->eType);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	MM_RETURN(RET_DMX_OK);
}

/**/
/* PSR_CC_NotiCfaPrsEnd*/
/* Set Parser CC's cfa parse end flag*/
/**/
MRESULT PSR_CC_NotiCfaPrsEnd(PSR_CC *prPsrCC, bool fgCfaPrsEnd)
{
	s32 i = 0;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	for (i = 0; i < MAX_PSR_CC_CNT; ++i) {
		if ((prPsrCC == g_rPsrMan.aprPsrCCs[i]) &&
			(prPsrCC->u4Flag & CCF_USED)) {
			i = MAX_PSR_CC_CNT << 1;
			break;
		}
	}
	if (i == MAX_PSR_CC_CNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DMXLOG_DEBUG(TEXT("[PSR_CC] %s, PsrCC:0x%x, PsrCC State:0x%x, ")
		TEXT("fgCfaPrsEnd:%s\r\n"),
		DMX_FUNC_NAME, prPsrCC, prPsrCC->eState,
		fgCfaPrsEnd ? L"TRUE":L"FALSE");

	prPsrCC->fgCfaPrsEnd = fgCfaPrsEnd;

	if (fgCfaPrsEnd &&
		(TXS_TX_JUMP == prPsrCC->eTxState)) {
		PSR_CC_SetState(prPsrCC, CCS_INIT);
		PSR_CC_SetTxSt(prPsrCC, TXS_TX_OK);
	}

	MM_RETURN(RET_DMX_OK);
}

#if CONFIG_DRV_HIGH_BITRATE_SPECIAL_PROC
/**/
/* PSR_CC_NotiCurStrmInf*/
/* Notify Splitter's parser cc whether need to do high bitrate process -- Change fgNeedHighbitRateProc flag*/
/**/
MRESULT PSR_CC_NotiCurStrmInf(PSR_CC *prPsrCC, bool fgHighBitrate)
{
	s32 i = 0;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	for (i = 0; i < MAX_PSR_CC_CNT; ++i) {
		if ((prPsrCC == g_rPsrMan.aprPsrCCs[i]) &&
			(prPsrCC->u4Flag & CCF_USED)) {
			i = MAX_PSR_CC_CNT << 1;
			break;
		}
	}
	if (i == MAX_PSR_CC_CNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PSR_CC_LOCK(prPsrCC->rLock);
	if (fgHighBitrate)
		prPsrCC->fgNeedHighBitRateProc = TRUE;
	else
		prPsrCC->fgNeedHighBitRateProc = FALSE;

	PSR_CC_UNLOCK(prPsrCC->rLock);

	MM_RETURN(RET_DMX_OK);
}
#endif

/**/
/* PSR_CC_GetActiveFilterType*/
/* Get the Parser CC's active Parser Filter's data type*/
/**/
MRESULT PSR_CC_GetActiveFilterType(PSR_CC *prPsrCC, u8 *pucActFilterType)
{
	s32 i = 0;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	for (i = 0; i < MAX_PSR_CC_CNT; ++i) {
		if ((prPsrCC == g_rPsrMan.aprPsrCCs[i]) &&
			(prPsrCC->u4Flag & CCF_USED)) {
			i = MAX_PSR_CC_CNT << 1;
			break;
		}
	}
	if (i == MAX_PSR_CC_CNT)
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);


	if (NULL == pucActFilterType)
		MM_RETURN(RET_DMX_PARAM_WRONG);


	*pucActFilterType = 0;

	if (NULL != prPsrCC->pvActFilter)
		*pucActFilterType = ((PSR_FILTER *)(prPsrCC->pvActFilter))->eType;

	MM_RETURN(RET_DMX_OK);
}

void PSR_CC_DumpCurrentState(PSR_CC *prPsrCC)
{
	PSR_FILTER *prPsrFtr = NULL;
	u32 u4Idx = 0;

	if (NULL == prPsrCC) {
		DMXLOG_ERROR(TEXT("[CLI] %s fail for invalid param --> prPsrCC")
			TEXT(" == NULL\r\n"), DMX_FUNC_NAME);
		return;
	}

	DmxDumpPsrCCInfo(prPsrCC);

	for (u4Idx = 0; u4Idx < MAX_PSR_FILTER_PER_CC; u4Idx++) {
		prPsrFtr = (PSR_FILTER *)(prPsrCC->apvFtr[u4Idx]);
		if (NULL == prPsrFtr)
			continue;

		DmxDumpPsrFilterInfo((void *)prPsrFtr);
	}
}

MRESULT PSR_CC_RelPbbuf2UnCon(
	PSR_CC *prPsrCC, u32 u4TxPbbufIdx)
{
	MRESULT mrRet = RET_DMX_OK;
	u64	u8SrcOffset = DMX_INVALID_UINT64;

	if ((NULL == prPsrCC) ||
		(MAX_CACHE_PBBUF <= u4TxPbbufIdx)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	u8SrcOffset = prPsrCC->arPBBuf[u4TxPbbufIdx].u8SrcOffset;

	while ((u8SrcOffset != prPsrCC->arPBBuf[0].u8SrcOffset) &&
		(NULL != prPsrCC->arPBBuf[0].pcPlayBuffer)) {
		mrRet = PSR_CC_ReleasePBBuf(prPsrCC, 0);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in PSR_CC_ReleasePBBuf(0),")
				TEXT(" mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
	}

	if (NULL != prPsrCC->arPBBuf[0].pcPlayBuffer) {
		/* update flag*/
		prPsrCC->u4Flag |= CCF_PBBUF_EXIST;
		/* change TX state*/
		PSR_CC_SetTxSt(prPsrCC, TXS_PBBUF_OK);

		MM_RETURN(RET_DMX_OK);
	}

	MM_RETURN(RET_DMX_UNEXPECT);
}

MRESULT PSR_CC_RelPbbufAcrossSlot2UnCon(
	PSR_CC *prPsrCC, u32 u4TxPbbufIdx, bool *pfgExistUnCon)
{
	DMX_READ_BUFFER *prPbbuf = NULL;
	u64	u8SrcOffset = 0;
	MRESULT mrRet = RET_DMX_OK;
	u32	u4Idx = 0, u4EndRelIdx = 0;

	if ((NULL == prPsrCC) ||
		(NULL == pfgExistUnCon) ||
		(MAX_CACHE_PBBUF <= u4TxPbbufIdx)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	for (u4Idx = 0; u4Idx < u4TxPbbufIdx; u4Idx++) {
		mrRet = PSR_CC_ReleasePBBuf(prPsrCC, 0);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in PSR_CC_ReleasePBBuf(0),")
				TEXT(" mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
	}

	prPbbuf = &(prPsrCC->arPBBuf[0]);

	if (NULL == prPbbuf->pcPlayBuffer) {
		*pfgExistUnCon = FALSE;
		prPsrCC->u4TxPBBufJumpIdx = MAX_CACHE_PBBUF;
		prPsrCC->u4TxPBBufIdx	  = MAX_CACHE_PBBUF;

		MM_RETURN(RET_DMX_OK);
	}

	u8SrcOffset = prPbbuf->u8SrcOffset;

	for (u4EndRelIdx = 0; u4EndRelIdx < MAX_CACHE_PBBUF - 1; u4EndRelIdx++) {
		if (NULL != prPsrCC->arPBBuf[u4EndRelIdx + 1].pcPlayBuffer) {
			if (prPbbuf->u8SrcOffset + prPbbuf->u4DataSize !=
				prPsrCC->arPBBuf[u4EndRelIdx + 1].u8SrcOffset) {
				break;
			}
		} else {
			break;
		}
	}

	for (u4Idx = 0; u4Idx < u4EndRelIdx; u4Idx++) {
		mrRet = PSR_CC_ReleasePBBuf(prPsrCC, 0);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in PSR_CC_ReleasePBBuf(0),")
				TEXT(" mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
	}

	if (NULL != prPsrCC->arPBBuf[1].pcPlayBuffer) {
		*pfgExistUnCon = TRUE;
		prPsrCC->u4TxPBBufJumpIdx = 1;
	} else {
		*pfgExistUnCon = FALSE;
		prPsrCC->u4TxPBBufJumpIdx = MAX_CACHE_PBBUF;
		prPsrCC->u4TxPBBufIdx	  = MAX_CACHE_PBBUF;

		#ifdef __linux__
		DMXLOG_TRACE(TEXT("[PSR] %s line %d -- PBBUF_ReleaseFrmSlotToUnCon,")
			TEXT(" u8SrcOffset: %lld, DataSz: %d\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->arPBBuf[0].u8SrcOffset,
			prPsrCC->arPBBuf[0].u4DataSize);
		#else
		DMXLOG_TRACE(TEXT("[PSR] %s line %d -- PBBUF_ReleaseFrmSlotToUnCon,")
				TEXT(" u8SrcOffset: %I64d, DataSz: %d\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->arPBBuf[0].u8SrcOffset,
			prPsrCC->arPBBuf[0].u4DataSize);
		#endif /* #ifdef __linux__*/

		mrRet = PBBUF_ReleaseFrmSlotToUnCon(prPsrCC->pvSptHdl,
			&(prPsrCC->arPBBuf[0]), pfgExistUnCon);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
				TEXT("PBBUF_ReleaseFrmSlotToUnCon, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		dmx_memset(&(prPsrCC->arPBBuf[0]), 0, sizeof(DMX_READ_BUFFER));

		/*PBBUF_DumpInfo(prPsrCC->pvSptHdl, FALSE);*/
	}

	MM_RETURN(mrRet);
}

MRESULT PSR_CC_Reset4NonConPbbufSlot(PSR_CC *prPsrCC)
{
	PSR_FILTER *prPsrFtr = NULL;
	MRESULT mrRet = RET_DMX_OK;
	u32	u4Idx = 0;

	if (NULL == prPsrCC) {
		DMXLOG_ERROR(TEXT("[CLI] %s fail for invalid param --> prPsrCC == NULL\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrCC->pvActFilter = NULL;

	prPsrCC->u8TxStartOffset = 0;
	prPsrCC->u8TxLen = 0;
	prPsrCC->ptrTxCurrSa = 0;
	prPsrCC->u8TxCurrOffset = 0;
	prPsrCC->u8TxCurrLen = 0;
	prPsrCC->fgHaveSubsequentData = FALSE;
	prPsrCC->ptrSrcMemSa = 0;
	prPsrCC->u4SrcMemLen = 0;
	prPsrCC->u4MemOffset = 0;

	if (prPsrCC->u4TxPBBufJumpIdx < MAX_CACHE_PBBUF) {
		mrRet = PSR_CC_RelPbbuf2UnCon(prPsrCC, prPsrCC->u4TxPBBufJumpIdx);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in PSR_CC_RelPbbuf2UnCon")
				TEXT("(TxPBBufJumpIdx: %d), PsrCC: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u4TxPBBufJumpIdx, prPsrCC);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
	}

	prPsrCC->u4TxPBBufJumpIdx = MAX_CACHE_PBBUF;
	prPsrCC->u4TxPBBufIdx = 0;

	prPsrCC->eState = CCS_INIT;
	prPsrCC->eTxState = TXS_TX_OK;
	prPsrCC->u4Flag |= (CCF_DATAIN_ENABLE | CCF_USED);

	prPsrCC->u4TxPBBufIdx = 0;

	prPsrCC->u8NormalWaitPts = INVALID_TIMESTAMP;
	prPsrCC->fgWakingWaitTx = FALSE;
	prPsrCC->fgVidPass = FALSE;
	prPsrCC->fgTxMem2Fifo = FALSE;
	prPsrCC->ptrPTXFifoRdPtr = 0;
	prPsrCC->ptrPTXFifoWrPtr = 0;
	prPsrCC->ptrPTXSrcSa = 0;
	prPsrCC->u4PTXSrcLen = 0;
	prPsrCC->fgLastMem = FALSE;

#if ENABLE_DMX_ADVANCED_VER
	prPsrCC->fgInsertHdr = FALSE;
	prPsrCC->u4InsertHdrLen = 0;
	dmx_memset(prPsrCC->au1InsertHdrBuf, 0, sizeof(prPsrCC->au1InsertHdrBuf));
#endif /* ENABLE_DMX_ADVANCED_VER*/

	prPsrCC->fgUseCmdQ = FALSE;
	prPsrCC->u2TxEntryCnt = 0;
	prPsrCC->fgCurTotalCmdQTxStarted = FALSE;
	prPsrCC->fgChkedAndWaitTx = FALSE;
	dmx_memset(&(prPsrCC->rCmdQPrevTxInf), 0, sizeof(PSR_CMDQ_TX_INF));
	dmx_memset(&(prPsrCC->rCmdQTxInf), 0, sizeof(PSR_CMDQ_TX_INF));

	prPsrCC->pvHwData = NULL;

	/* save filter handle into cc*/
	for (u4Idx = 0; u4Idx < MAX_PSR_FILTER_PER_CC; u4Idx++) {
		if (NULL != prPsrCC->apvFtr[u4Idx]) {
			prPsrFtr = (PSR_FILTER *)(prPsrCC->apvFtr[u4Idx]);
			mrRet = PSR_Filter_Reset(prPsrFtr);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
					TEXT("PSR_Filter_Reset(eType: %d), PsrCC: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->eType, prPsrCC);
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}
		}
	}

	mrRet = PSR_Decrypt_Reset(prPsrCC);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in PSR_Decrypt_Reset,")
			TEXT(" PsrCC: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	MM_RETURN(mrRet);
}

MRESULT PSR_CC_SetDecryptType(PSR_CC *prPsrCC, E_DECRYPT_TYPE_T eDecryptType)
{
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prPsrCC) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if ((eDecryptType == prPsrCC->rDecryptMan.eDecryptType) &&
		(DECRYPT_NONE != eDecryptType)) {
		MM_RETURN(RET_DMX_OK);
	}

	mrRet = PSR_Decrypt_Create(prPsrCC, eDecryptType);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in PSR_Decrypt_Create")
			TEXT("(eDecryptType: %d), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

#if DMX_SUPPORT_DIVXDRM

MRESULT PSR_CC_EnableDivxDRMDecrypt(PSR_CC *prPsrCC, PSR_DivxDRMInfo *prPsrDRMInf)
{
	PSR_DECRYPT_DIVXDRM_PRIVDATA_T *prPrivData = NULL;
	MRESULT mrRet = RET_DMX_OK;
	s32 i = 0;

	if (!g_fgPSRInit) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	for (i = 0; i < MAX_PSR_CC_CNT; ++i) {
		if ((prPsrCC == g_rPsrMan.aprPsrCCs[i]) &&
			(prPsrCC->u4Flag & CCF_USED)) {
			i = MAX_PSR_CC_CNT << 1;
			break;
		}
	}
	if (i == MAX_PSR_CC_CNT) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (NULL == prPsrDRMInf) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PSR_CC_LOCK(prPsrCC->rLock);

	if (DECRYPT_DIVXDRM != prPsrCC->rDecryptMan.eDecryptType) {
		PSR_CC_UNLOCK(prPsrCC->rLock);
		MM_RETURN(RET_DMX_OK);
	}

	if (NULL == prPsrCC->rDecryptMan.pvPrivData) {
		DMX_NewMemory(sizeof(PSR_DECRYPT_DIVXDRM_PRIVDATA_T), prPsrCC->rDecryptMan.pvPrivData);
		if (NULL == prPsrCC->rDecryptMan.pvPrivData) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in alloc DIVXDRM Priv")
				TEXT(" data, PsrCC: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC);
			PSR_CC_UNLOCK(prPsrCC->rLock);
			MM_RETURN(RET_DMX_NO_MEM);
		}
		dmx_memset(prPsrCC->rDecryptMan.pvPrivData, 0,
			sizeof(PSR_DECRYPT_DIVXDRM_PRIVDATA_T));
	}

	prPrivData = (PSR_DECRYPT_DIVXDRM_PRIVDATA_T *)(prPsrCC->rDecryptMan.pvPrivData);

	if (prPsrDRMInf->fgTurnOn) {
		if (prPsrDRMInf->u8DecryptStOfst == prPsrCC->rDecryptMan.u8DecryptStOft) {
			DMXLOG_TRACE(TEXT("[PSR] %s line %d -- Repeat TURN_ON Decrypt, Ofst:")
				TEXT(" 0x%llx, Len: %d, FrameKeyIdx: 0x%02x, PsrCC: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPsrDRMInf->u8DecryptStOfst,
				prPsrDRMInf->u4DecryptLen, prPsrDRMInf->u2FrameKeyIdx, prPsrCC);
			PSR_CC_UNLOCK(prPsrCC->rLock);
			MM_RETURN(RET_DMX_OK);
		}

		prPsrCC->u4Flag |= CCF_CPS_ON;

		prPsrCC->rDecryptMan.u4DecryptLen  = prPsrDRMInf->u4DecryptLen;

		prPrivData->u2FrameKeyIdx = prPsrDRMInf->u2FrameKeyIdx;

		prPsrCC->rDecryptMan.u4AlignSize = DMX_DECRYPT_DIVXDRM_DECRYPTION_ALIGNSIZE;

		if (0 == prPsrCC->rDecryptMan.u4TxMemAddr) {
			if (DMX_MAX_DECRYPT_BUFFER_LEN < prPsrDRMInf->u4DecryptLen)
				prPsrCC->rDecryptMan.u4TxMemSize = prPsrDRMInf->u4DecryptLen;
			else
				prPsrCC->rDecryptMan.u4TxMemSize = DMX_MAX_DECRYPT_BUFFER_LEN;

#ifdef __linux__
			DMX_NewHwAlignMemory(prPsrCC->rDecryptMan.u4TxMemSize,
				DMX_DECRYPT_DIVXDRM_DECRYPTION_ALIGNSIZE,
				prPsrCC->rDecryptMan.u4TxMemAddr);
#else
			DMX_NewHwAlignMemory(prPsrCC->rDecryptMan.u4TxMemSize,
				DMX_DECRYPT_DIVXDRM_DECRYPTION_ALIGNSIZE,
				(void *)(prPsrCC->rDecryptMan.u4TxMemAddr));
#endif /* __linux__*/
			if (0 == prPsrCC->rDecryptMan.u4TxMemAddr) {
				DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in alloc DIVXDRM")
					TEXT(" Working Buffer, PsrCC: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC);
				PSR_CC_UNLOCK(prPsrCC->rLock);
				MM_RETURN(RET_DMX_NO_MEM);
			}

			dmx_memset((void *)(prPsrCC->rDecryptMan.u4TxMemAddr), 0,
				prPsrCC->rDecryptMan.u4TxMemSize);
		} else if (prPsrCC->rDecryptMan.u4TxMemSize < prPsrDRMInf->u4DecryptLen) {
			DMX_FreeHwMemory((void *)(prPsrCC->rDecryptMan.u4TxMemAddr));
			prPsrCC->rDecryptMan.u4TxMemAddr = 0;
			prPsrCC->rDecryptMan.u4TxMemSize = 0;
			prPsrCC->rDecryptMan.u4TxMemWPtr = 0;
#ifdef __linux__
			DMX_NewHwAlignMemory(prPsrDRMInf->u4DecryptLen,
				DMX_DECRYPT_DIVXDRM_DECRYPTION_ALIGNSIZE,
				prPsrCC->rDecryptMan.u4TxMemAddr);
#else
			DMX_NewHwAlignMemory(prPsrDRMInf->u4DecryptLen,
				DMX_DECRYPT_DIVXDRM_DECRYPTION_ALIGNSIZE,
				(void *)(prPsrCC->rDecryptMan.u4TxMemAddr));
#endif /* __linux__*/
			if (0 == prPsrCC->rDecryptMan.u4TxMemAddr) {
				DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in alloc DIVXDRM")
					TEXT(" Working Buffer, PsrCC: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC);
				PSR_CC_UNLOCK(prPsrCC->rLock);
				MM_RETURN(RET_DMX_NO_MEM);
			}
			prPsrCC->rDecryptMan.u4TxMemSize = prPsrDRMInf->u4DecryptLen;
			dmx_memset((void *)(prPsrCC->rDecryptMan.u4TxMemAddr), 0,
				prPsrCC->rDecryptMan.u4TxMemSize);
		} else {

			dmx_memset((void *)(prPsrCC->rDecryptMan.u4TxMemAddr), 0,
				prPsrCC->rDecryptMan.u4TxMemSize);
		}

		prPsrCC->rDecryptMan.u4TxMemWPtr = 0;
		prPsrCC->rDecryptMan.u4TxMemRPtr = 0;

		prPsrCC->rDecryptMan.eStatus = DECRYPT_UNCOMPLETE;

#if DMX_DRM_DECRYPT_USE_HW
		prPsrCC->rDecryptMan.eMethod = DECRYPT_BY_HW;
#else  /* DMX_DRM_DECRYPT_USE_HW*/
		prPsrCC->rDecryptMan.eMethod = DECRYPT_BY_SW;
#endif /* DMX_DRM_DECRYPT_USE_HW*/

#if DMX_DRM_DECRYPT_USE_HW
		prPsrCC->rDecryptMan.rHWParam.rDRM.eInSlotType = DECRYPT_NOT_IN_SLOT;
		prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclLen = 0;
		prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclOft = 0;
		prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptTotalLen = 0;
#endif /* DMX_DRM_DECRYPT_USE_HW*/

		prPsrCC->rDecryptMan.u8DecryptStOft = prPsrDRMInf->u8DecryptStOfst;

		if (DMX_DIVXDRM_INVALID_FRAMEIDX == prPsrDRMInf->u2FrameKeyIdx) {
			prPsrCC->rDecryptMan.eDataType = SPT_DATA_A;
#if DMX_PRINT_DECRYPT_KEY_LOG
			DMXLOG_TRACE(TEXT("[PSR] (TURN_ON Decrypt) Auido -- Ofst: 0x%llx,")
				TEXT(" Len: %d, FrameKeyIdx: 0x%02x, PsrCC: 0x%x\r\n"),
				prPsrDRMInf->u8DecryptStOfst, prPsrDRMInf->u4DecryptLen,
				prPsrDRMInf->u2FrameKeyIdx, prPsrCC);
#endif /* DMX_PRINT_DECRYPT_KEY_LOG*/
		} else {
			prPsrCC->rDecryptMan.eDataType = SPT_DATA_V;

#if DMX_DRM_DECRYPT_USE_HW
#if DMX_PRINT_DECRYPT_KEY_LOG
			DMXLOG_TRACE(TEXT("[PSR] (TURN_ON Decrypt) Video -- Ofst: 0x%llx,")
				TEXT(" Len: %d, FrameKeyIdx: 0x%02x, PsrCC: 0x%x\r\n"),
				prPsrDRMInf->u8DecryptStOfst, prPsrDRMInf->u4DecryptLen,
				prPsrDRMInf->u2FrameKeyIdx, prPsrCC);
			DMXLOG_TRACE(TEXT("[PSR] %s line %d -- Reset rDecryptMan.")
				TEXT("rHWParam(eDataType: %d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->rDecryptMan.eDataType);
#endif /* DMX_PRINT_DECRYPT_KEY_LOG*/

			dmx_memset(&(prPsrCC->rDecryptMan.rHWParam.rDRM.rVideo), 0, sizeof(PSR_DRM_AES_INFO_T));
			mrRet = PSR_Decrypt_GetKeyInfo(prPsrCC, SPT_DATA_V);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[DECRYPT] %s line %d fail in ")
					TEXT("PSR_Decrypt_GetKeyInfo(%s, u4DecryptLen: %d), change to")
					TEXT(" sw decrypt\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					((prPsrCC->rDecryptMan.eDataType < MAX_SPT_DATA_TYPE_CNT) ?
					g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] : TEXT("UNKNOWN")),
					prPsrCC->rDecryptMan.u4DecryptLen);
				prPsrCC->rDecryptMan.eMethod = DECRYPT_BY_SW;
				mrRet = RET_DMX_OK;
			}
#endif /* DMX_DRM_DECRYPT_USE_HW*/
		}

		if (g_rDmxCliMan.fgDumpFlow) {
			DMX_DUMP_FLOW_OPER_INFO_T rOperInfo1;

			mm_memset(&rOperInfo1, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
			rOperInfo1.pvSptHdl = prPsrCC->pvSptHdl;
			rOperInfo1.unFlow.rDecryptSetting.eDataType = prPsrCC->rDecryptMan.eDataType;
			rOperInfo1.unFlow.rDecryptSetting.u8DecryptOfst = prPsrDRMInf->u8DecryptStOfst;
			rOperInfo1.unFlow.rDecryptSetting.u4DecryptLen	= prPsrDRMInf->u4DecryptLen;
			rOperInfo1.unFlow.rDecryptSetting.u2FrameKeyIdx = prPsrDRMInf->u2FrameKeyIdx;
			DmxDumpFlow(DMX_OPER_SET_DECRYPT_INFO, &rOperInfo1);
		}

		mrRet = PSR_Decrypt_PbbufCheck(prPsrCC, prPsrCC->u4TxPBBufIdx,
			NULL, NULL, FALSE);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in PSR_Decrypt_InitCheck,")
				TEXT(" mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			PSR_CC_UNLOCK(prPsrCC->rLock);
			MM_RETURN(mrRet);
		}
	} else {
#if DMX_PRINT_DECRYPT_KEY_LOG
		DMXLOG_TRACE(TEXT("[PSR] (TURN OFF Decrypt), PsrCC: 0x%x\r\n"), prPsrCC);
#endif /* DMX_PRINT_DECRYPT_KEY_LOG*/

		if (g_rDmxCliMan.fgDumpFlow) {
			DMX_DUMP_FLOW_OPER_INFO_T rOperInfo1;

			mm_memset(&rOperInfo1, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
			rOperInfo1.pvSptHdl = prPsrCC->pvSptHdl;
			rOperInfo1.unFlow.rDecryptSetting.eDataType = prPsrCC->rDecryptMan.eDataType;
			rOperInfo1.unFlow.rDecryptSetting.u8DecryptOfst = DMX_INVALID_UINT64;
			rOperInfo1.unFlow.rDecryptSetting.u4DecryptLen	= 0;
			rOperInfo1.unFlow.rDecryptSetting.u2FrameKeyIdx =
				DMX_DIVXDRM_INVALID_FRAMEIDX;
			DmxDumpFlow(DMX_OPER_SET_DECRYPT_INFO, &rOperInfo1);
		}

		prPsrCC->u4Flag &= (~CCF_CPS_ON);
		prPrivData->u2FrameKeyIdx	= DMX_DIVXDRM_INVALID_FRAMEIDX;
		prPsrCC->rDecryptMan.u8DecryptStOft   = DMX_INVALID_UINT64;
		prPsrCC->rDecryptMan.u4DecryptLen	  = 0;
		prPsrCC->rDecryptMan.eDataType = SPT_DATA_UNDEFINE;
		prPsrCC->rDecryptMan.u4TxMemWPtr = 0;
		prPsrCC->rDecryptMan.u4TxMemRPtr = 0;
		prPsrCC->rDecryptMan.eStatus = DECRYPT_UNCOMPLETE;
		prPsrCC->rDecryptMan.eMethod = DECRYPT_BY_NONE;
#if DMX_DRM_DECRYPT_USE_HW
		prPsrCC->rDecryptMan.rHWParam.rDRM.eInSlotType = DECRYPT_NOT_IN_SLOT;
		prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclLen = 0;
		prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclOft = 0;
		prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptTotalLen = 0;
#endif /* DMX_DRM_DECRYPT_USE_HW*/
	}

	PSR_CC_UNLOCK(prPsrCC->rLock);

	MM_RETURN(RET_DMX_OK);
}

#if DMX_DRM_DECRYPT_USE_HW

MRESULT PSR_CC_CalcHwDecryptOfstAcrossSlot(PSR_CC *prPsrCC)
{
	/* if the current tx size < encrypted data size, we make the tx*/
	/*proc to do in the next time*/
	if ((u32)(prPsrCC->u8TxCurrLen) < prPsrCC->rDecryptMan.u4DecryptLen) {
#if DMX_PRINT_DECRYPT_KEY_LOG
	#ifdef __linux__
		DMXLOG_TRACE(TEXT("[PSR] CalcOfstLen %s -- Accross Slot, but")
			TEXT(" u8TxCurrLen(%lld) < u4DecryptLen(%d), so set to WAIT_FIFO\r\n"),
			((prPsrCC->rDecryptMan.eDataType < MAX_SPT_DATA_TYPE_CNT) ?
			g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] : TEXT("UNKNOWN")),
			prPsrCC->u8TxCurrLen, prPsrCC->rDecryptMan.u4DecryptLen);
	#else
		DMXLOG_TRACE(TEXT("[PSR] CalcOfstLen %s -- Accross Slot, but")
			TEXT(" u8TxCurrLen(%I64d) < u4DecryptLen(%d), so set to ")
			TEXT("WAIT_FIFO\r\n"),
			((prPsrCC->rDecryptMan.eDataType < MAX_SPT_DATA_TYPE_CNT) ?
			g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] : TEXT("UNKNOWN")),
			prPsrCC->u8TxCurrLen, prPsrCC->rDecryptMan.u4DecryptLen);
	#endif /* #ifdef __linux__*/
#endif /* DMX_PRINT_DECRYPT_KEY_LOG*/
		prPsrCC->u8TxCurrLen = 0;
		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_FIFO);
		MM_RETURN(RET_DMX_OK);
	}

	prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclOft = 0;
	prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclLen =
		prPsrCC->rDecryptMan.u4DecryptLen;

#if DMX_PRINT_DECRYPT_KEY_LOG
	DMXLOG_TRACE(TEXT("[PSR] CalcOfstLen %s -- Accross Slot ")
		TEXT("(u4DecryptLclOft: %d , u4DecryptLclLen: %d)\r\n"),
		((prPsrCC->rDecryptMan.eDataType < MAX_SPT_DATA_TYPE_CNT) ?
		g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
		TEXT("UNKNOWN")),
		prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclOft,
		prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclLen);
#endif /* DMX_PRINT_DECRYPT_KEY_LOG*/

	prPsrCC->rDecryptMan.eStatus = DECRYPT_DECRYPTING;

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_CC_CalcHwDecryptOfstInSlot(PSR_CC *prPsrCC,
	PSR_FILTER *prPsrFtr)
{
	MRESULT mrRet = RET_DMX_OK;

	if (prPsrCC->u8TxCurrOffset <= prPsrCC->rDecryptMan.u8DecryptStOft) {
		if (prPsrCC->u8TxCurrOffset + prPsrCC->u8TxCurrLen <=
			prPsrCC->rDecryptMan.u8DecryptStOft) {
			prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclOft = 0;
			prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclLen = 0;
#if DMX_PRINT_DECRYPT_KEY_LOG
		#ifdef __linux__
			DMXLOG_TRACE(TEXT("[PSR]CalcOfstLen %s--(CurOfst:")
				TEXT(" %lld + CurLen: %lld < DecryptStOft: %lld)\r\n"),
				((prPsrCC->rDecryptMan.eDataType < MAX_SPT_DATA_TYPE_CNT) ?
				g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
				TEXT("UNKNOWN")),
				prPsrCC->u8TxCurrOffset, prPsrCC->u8TxCurrLen,
				(prPsrCC->rDecryptMan.u8DecryptStOft));
		#else
			DMXLOG_TRACE(TEXT("[PSR]CalcOfstLen %s--(CurOfst:")
				TEXT(" %I64d + CurLen: %I64d < DecryptStOft: %I64d)\r\n"),
				((prPsrCC->rDecryptMan.eDataType < MAX_SPT_DATA_TYPE_CNT) ?
				g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
				TEXT("UNKNOWN")),
				prPsrCC->u8TxCurrOffset, prPsrCC->u8TxCurrLen,
				(prPsrCC->rDecryptMan.u8DecryptStOft));
		#endif /* #ifdef __linux__*/
#endif /* DMX_PRINT_DECRYPT_KEY_LOG*/
		} else {
			prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclOft =
				(u32)(prPsrCC->rDecryptMan.u8DecryptStOft -
				prPsrCC->u8TxCurrOffset);

			if (prPsrCC->u8TxCurrOffset + prPsrCC->u8TxCurrLen <=
				prPsrCC->rDecryptMan.u8DecryptStOft +
				prPsrCC->rDecryptMan.u4DecryptLen) {
				prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclLen =
					(u32)(prPsrCC->u8TxCurrOffset +
					prPsrCC->u8TxCurrLen - prPsrCC->rDecryptMan.u8DecryptStOft);
#if DMX_PRINT_DECRYPT_KEY_LOG
			#ifdef __linux__
				DMXLOG_TRACE(TEXT("[PSR]CalcOfstLen %s--")
					TEXT("(CurOfst:%lld <DecryptStOft:%lld && CurOfst +")
					TEXT("CurLen:%lld<=DecryptStOft+DecryptLen:%lld)\r\n"),
					((prPsrCC->rDecryptMan.eDataType <
					MAX_SPT_DATA_TYPE_CNT) ?
					g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
					TEXT("UNKNOWN")),
					prPsrCC->u8TxCurrOffset,
					(prPsrCC->rDecryptMan.u8DecryptStOft),
					(prPsrCC->u8TxCurrOffset + prPsrCC->u8TxCurrLen),
					(prPsrCC->rDecryptMan.u8DecryptStOft +
					prPsrCC->rDecryptMan.u4DecryptLen));
			#else
				DMXLOG_TRACE(TEXT("[PSR]CalcOfstLen%s--")
					TEXT("(CurOfst:%I64d<DecryptStOft:%I64d&&CurOfst+")
					TEXT("CurLen:%I64d<=DecryptStOft+DecryptLen:")
					TEXT("%I64d)\r\n"),
					((prPsrCC->rDecryptMan.eDataType <
					MAX_SPT_DATA_TYPE_CNT) ?
					g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
					TEXT("UNKNOWN")),
					prPsrCC->u8TxCurrOffset,
					(prPsrCC->rDecryptMan.u8DecryptStOft),
					(prPsrCC->u8TxCurrOffset + prPsrCC->u8TxCurrLen),
					(prPsrCC->rDecryptMan.u8DecryptStOft +
					prPsrCC->rDecryptMan.u4DecryptLen));
			#endif /* #ifdef __linux__*/
#endif /* DMX_PRINT_DECRYPT_KEY_LOG*/
				/* This is the first part of the AES packet*/
			} else {
				prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclLen =
					prPsrCC->rDecryptMan.u4DecryptLen;
#if DMX_PRINT_DECRYPT_KEY_LOG
			#ifdef __linux__
				DMXLOG_TRACE(TEXT("[PSR] CalcOfstLen %s--")
					TEXT("(CurOfst: %lld<DecryptStOft:%lld&&DecryptStOft")
					TEXT("+DecryptLen:%lld<CurOfst +CurLen:%lld)\r\n"),
					((prPsrCC->rDecryptMan.eDataType <
					MAX_SPT_DATA_TYPE_CNT) ?
					g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
					TEXT("UNKNOWN")),
					prPsrCC->u8TxCurrOffset,
					(prPsrCC->rDecryptMan.u8DecryptStOft),
					(prPsrCC->rDecryptMan.u8DecryptStOft +
					prPsrCC->rDecryptMan.u4DecryptLen),
					(prPsrCC->u8TxCurrOffset + prPsrCC->u8TxCurrLen));
			#else
				DMXLOG_TRACE(TEXT("[PSR] CalcOfstLen %s -- ")
					TEXT("(CurOfst:%I64d<DecryptStOft:%I64d &&DecryptStOft")
					TEXT(" + DecryptLen:%I64d<CurOfst+CurLen:%I64d)\r\n"),
					((prPsrCC->rDecryptMan.eDataType <
					MAX_SPT_DATA_TYPE_CNT) ?
					g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
					TEXT("UNKNOWN")),
					prPsrCC->u8TxCurrOffset,
					(prPsrCC->rDecryptMan.u8DecryptStOft),
					(prPsrCC->rDecryptMan.u8DecryptStOft +
					prPsrCC->rDecryptMan.u4DecryptLen),
					(prPsrCC->u8TxCurrOffset + prPsrCC->u8TxCurrLen));
			#endif /* #ifdef __linux__*/
#endif /* DMX_PRINT_DECRYPT_KEY_LOG*/
			}
		}
	} else {
		/* prPsrCC->rDecryptMan.u8DecryptStOft < prPsrCC->u8TxCurrOffset*/
		if (prPsrCC->rDecryptMan.u8DecryptStOft +
			prPsrCC->rDecryptMan.u4DecryptLen <= prPsrCC->u8TxCurrOffset) {
			prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclOft = 0;
			prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclLen = 0;
#if DMX_PRINT_DECRYPT_KEY_LOG
		#ifdef __linux__
			DMXLOG_TRACE(TEXT("[PSR] CalcOfstLen %s -- ")
				TEXT("(DecryptStOft+DecryptLen:%lld<=CurOfst:%lld)\r\n"),
				((prPsrCC->rDecryptMan.eDataType <
				MAX_SPT_DATA_TYPE_CNT) ?
				g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
				TEXT("UNKNOWN")),
				(prPsrCC->rDecryptMan.u8DecryptStOft +
				prPsrCC->rDecryptMan.u4DecryptLen),
				prPsrCC->u8TxCurrOffset);
		#else
			DMXLOG_TRACE(TEXT("[PSR] CalcOfstLen %s -- ")
				TEXT("(DecryptStOft+DecryptLen:%I64d<=CurOfst:%I64d)\r\n"),
				((prPsrCC->rDecryptMan.eDataType <
				MAX_SPT_DATA_TYPE_CNT) ?
				g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
				TEXT("UNKNOWN")),
				(prPsrCC->rDecryptMan.u8DecryptStOft +
				prPsrCC->rDecryptMan.u4DecryptLen),
				prPsrCC->u8TxCurrOffset);
		#endif /* #ifdef __linux__*/
#endif /* DMX_PRINT_DECRYPT_KEY_LOG*/
		} else {
			prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclOft = 0;
			if (prPsrCC->u8TxCurrOffset + prPsrCC->u8TxCurrLen <=
				prPsrCC->rDecryptMan.u8DecryptStOft +
				prPsrCC->rDecryptMan.u4DecryptLen) {
				prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclLen =
					(u32)(prPsrCC->u8TxCurrLen);
#if DMX_PRINT_DECRYPT_KEY_LOG
			#ifdef __linux__
				DMXLOG_TRACE(TEXT("[PSR]CalcOfstLen %s--")
					TEXT("(DecryptStOft<CurOfst:%lld&&CurOfst+CurLen:")
					TEXT("%lld <= DecryptStOft+DecryptLen:%lld)\r\n"),
					((prPsrCC->rDecryptMan.eDataType <
					MAX_SPT_DATA_TYPE_CNT) ?
					g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
					TEXT("UNKNOWN")),
					(prPsrCC->rDecryptMan.u8DecryptStOft),
					prPsrCC->u8TxCurrOffset,
					(prPsrCC->u8TxCurrOffset + prPsrCC->u8TxCurrLen),
					(prPsrCC->rDecryptMan.u8DecryptStOft +
					prPsrCC->rDecryptMan.u4DecryptLen));
			#else
				DMXLOG_TRACE(TEXT("[PSR]CalcOfstLen %s--")
					TEXT("(DecryptStOf <CurOfst:%I64d&&CurOfst+CurLen:")
					TEXT(" %I64d<=DecryptStOft+DecryptLen:%I64d)\r\n"),
					((prPsrCC->rDecryptMan.eDataType <
					MAX_SPT_DATA_TYPE_CNT) ?
					g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
					TEXT("UNKNOWN")),
					(prPsrCC->rDecryptMan.u8DecryptStOft),
					prPsrCC->u8TxCurrOffset,
					(prPsrCC->u8TxCurrOffset + prPsrCC->u8TxCurrLen),
					(prPsrCC->rDecryptMan.u8DecryptStOft +
					prPsrCC->rDecryptMan.u4DecryptLen));
			#endif /* #ifdef __linux__*/
#endif /* DMX_PRINT_DECRYPT_KEY_LOG*/
				/* This is the middle part of the AES packet*/
			} else {
				prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclLen =
					(u32)(prPsrCC->rDecryptMan.u8DecryptStOft +
					prPsrCC->rDecryptMan.u4DecryptLen -
					prPsrCC->u8TxCurrOffset);
#if DMX_PRINT_DECRYPT_KEY_LOG
			#ifdef __linux__
				DMXLOG_TRACE(TEXT("[PSR]CalcOfstLen %s--")
					TEXT("(DecryptStOft<CurOfst:%lld&&DecryptStOft+")
					TEXT("DecryptLen:%lld<=CurOfst+CurLen:%lld)\r\n"),
					((prPsrCC->rDecryptMan.eDataType <
					MAX_SPT_DATA_TYPE_CNT) ?
					g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
					TEXT("UNKNOWN")),
					(prPsrCC->rDecryptMan.u8DecryptStOft),
					prPsrCC->u8TxCurrOffset,
					(prPsrCC->rDecryptMan.u8DecryptStOft +
					prPsrCC->rDecryptMan.u4DecryptLen),
					(prPsrCC->u8TxCurrOffset + prPsrCC->u8TxCurrLen));
			#else
				DMXLOG_TRACE(TEXT("[PSR]CalcOfstLen %s--")
					TEXT("(DecryptStOft<CurOfst:%I64d&&DecryptStOft+")
					TEXT("DecryptLen:%I64d<=CurOfst+CurLen:%I64d)\r\n"),
					((prPsrCC->rDecryptMan.eDataType <
					MAX_SPT_DATA_TYPE_CNT) ?
					g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
					TEXT("UNKNOWN")),
					(prPsrCC->rDecryptMan.u8DecryptStOft),
					prPsrCC->u8TxCurrOffset,
					(prPsrCC->rDecryptMan.u8DecryptStOft +
					prPsrCC->rDecryptMan.u4DecryptLen),
					(prPsrCC->u8TxCurrOffset + prPsrCC->u8TxCurrLen));
			#endif /* #ifdef __linux__*/
#endif /* DMX_PRINT_DECRYPT_KEY_LOG*/
				/* This is the end part of the AES packet*/
			}
		}
	}

	prPsrCC->rDecryptMan.eStatus = DECRYPT_DECRYPTING;

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_CC_CalcHWDecryptOfst(PSR_CC *prPsrCC, PSR_FILTER *prPsrFtr)
{
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prPsrCC) ||
		(NULL == prPsrFtr)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if ((0 != (prPsrCC->u4Flag & CCF_CPS_ON)) &&
		(0 < prPsrCC->rDecryptMan.u4DecryptLen) &&
		(prPsrFtr->u4Flag & FF_TX_PBBUF) &&
		(DECRYPT_BY_HW == prPsrCC->rDecryptMan.eMethod)) {
		prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclOft = 0;
		prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclLen = 0;

		if (DECRYPT_COMPLETE != prPsrCC->rDecryptMan.eStatus) {
			if (DECRYPT_ACCROSS_SLOTS == prPsrCC->rDecryptMan.rHWParam.rDRM.eInSlotType) {
				mrRet = PSR_CC_CalcHwDecryptOfstAcrossSlot(prPsrCC);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[PSR] %s fail in calculate hw ")
						TEXT("decrypt offset for accross slot\r\n"),
						DMX_FUNC_NAME);
			  }
				MM_RETURN(mrRet);
			} else {
				mrRet = PSR_CC_CalcHwDecryptOfstInSlot(prPsrCC, prPsrFtr);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[PSR] %s fail in calculate hw ")
						TEXT("decrypt offset for in slot\r\n"),
						DMX_FUNC_NAME);
			  }
				MM_RETURN(mrRet);
			}
		}
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_CC_CheckHWDecryptStatus(PSR_CC *prPsrCC, PSR_FILTER *prPsrFtr)
{
	if ((NULL == prPsrCC) ||
		(NULL == prPsrFtr)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if ((0 != (prPsrCC->u4Flag & CCF_CPS_ON)) &&
		(0 < prPsrCC->rDecryptMan.u4DecryptLen) &&
		(DECRYPT_BY_HW == prPsrCC->rDecryptMan.eMethod) &&
		(DECRYPT_COMPLETE != prPsrCC->rDecryptMan.eStatus)) {
		prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptTotalLen +=
			prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclLen;
		if (prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptTotalLen >=
			prPsrCC->rDecryptMan.u4DecryptLen) {
#if DMX_PRINT_DECRYPT_KEY_LOG
			DMXLOG_TRACE(TEXT("[PSR] CheckDecryptStatus Video (Complete) -- ")
				TEXT("(u4DecryptTotalLen: %d >= u4DecryptLen: %d, u4DecryptLclLen: %d)\r\n"),
				prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptTotalLen,
				prPsrCC->rDecryptMan.u4DecryptLen,
				prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclLen);
#endif /* DMX_PRINT_DECRYPT_KEY_LOG*/
			prPsrCC->rDecryptMan.eStatus = DECRYPT_COMPLETE;
		} else {
#if DMX_PRINT_DECRYPT_KEY_LOG
			DMXLOG_TRACE(TEXT("[PSR] CheckDecryptStatus Video (Decrypting)")
				TEXT(" -- (u4DecryptTotalLen: %d < u4DecryptLen: %d, u4DecryptLclLen: %d)\r\n"),
				prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptTotalLen,
				prPsrCC->rDecryptMan.u4DecryptLen,
				prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclLen);
#endif /* DMX_PRINT_DECRYPT_KEY_LOG*/
		}
	}

	MM_RETURN(RET_DMX_OK);
}

#endif /* DMX_DRM_DECRYPT_USE_HW*/

#endif /* #ifdef ENABLE_DIVXDRM*/

