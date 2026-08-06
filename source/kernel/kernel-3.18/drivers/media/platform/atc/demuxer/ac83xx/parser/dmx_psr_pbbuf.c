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
 * @file dmx_psr_pbbuf.c
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
#ifdef __linux__
#include <linux/mm.h>
#include "windows.h"
#endif /* __linux__*/

#include "x_debug.h"
#include "drv_def.h"

#ifdef __linux__
#include <media/atc/dmx_define.h>
#if CONFIG_DRV_HDMI_RX
#include <media/atc/x_audin.h>
#endif /* CONFIG_DRV_HDMI_RX*/
/* #include <media/atc/mm_debug.h> */

#else /*__linux__*/
#include "dmx_define.h"
#if CONFIG_DRV_HDMI_RX
#include "x_audin.h"
#endif /* CONFIG_DRV_HDMI_RX*/
#include "mm_debug.h"
#endif /* __linux__*/

#include "dmx_def.h"
#include "dmx_cli.h"
#include "dmx_dump.h"
#include "dmx_pbbuf_if.h"
#include "dmx_psr_pbbuf.h"
#include "dmx_parser.h"
#include "dmx_mem.h"
#include "dmx_psr_decrypt.h"
#include "dmx_pfm.h"

#ifndef __linux__
#pragma warning(disable : 4127) /*disable warning C4127: conditional expression is constant*/
#endif

EXTERN DMX_CLI_MAN_T g_rDmxCliMan;
EXTERN DMX_DUMP_MAN_T g_rDmxDumpMan;

/*//////////////////////////////////////////////////////////////////////////////*/
/* PSR_CC_PBBuf_Notify*/
/* When the Pbbuf get one sent slot while the parser cc wait for PBBuf,*/
/* the PBBuf manager will call this function*/
/* to inform sent slot comes in*/
/*//////////////////////////////////////////////////////////////////////////////*/
void PSR_CC_PBBuf_Notify(void *pvTag, DRV_PBBUF_NOTIFY_COND_T eReadyCond,
			 u32 u4Data)
{
	PSR_CC *prPsrCC = (PSR_CC *)pvTag;

	UNUSE_PARAMETER(u4Data);

	if (NULL == prPsrCC) {
		DMX_ASSERT(FALSE);
		return;
	}

	if (DRV_PBBUF_COND_RELEASE_ALL_SLOTS == eReadyCond) {
		/*Check CC State*/
		/* If current state is in transferring, it is wrong form release all slots*/
		if ((CCS_TX == prPsrCC->eState) &&
		    (TXS_TXING == prPsrCC->eTxState)) {
			DMX_ASSERT(FALSE);
			return;
		}
		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);

		prPsrCC->u4Flag |= CCF_PBBUF_RELEASE_NOTIFY;

		/* wake up me*/
		PSR_CC_CBSplitter(prPsrCC, E_WAKEUP_ME, NULL);
	} else if (DRV_PBBUF_COND_READY_TO_GET_READ_BUFFER == eReadyCond) {
		/* wake up me*/
		DMXLOG_DEBUG(TEXT("[PSR] %s -- PsrCC's State: 0x%x, eTxState: 0x%x\r\n"),
			    DMX_FUNC_NAME, prPsrCC->eState, prPsrCC->eTxState);

		/* If in transferring and wait for PBbuf, wake up splitter(set PTX_CALL event)*/
		if ((CCS_TX == prPsrCC->eState) &&
		    (TXS_WAIT_PBBUF == prPsrCC->eTxState))
			PSR_CC_CBSplitter(prPsrCC, E_WAKEUP_ME, NULL);
		else if ((CCS_TX == prPsrCC->eState) &&
			 (TXS_TX_JUMP == prPsrCC->eTxState))
			PSR_CC_CBSplitter(prPsrCC, E_WAKEUP_ME, NULL);
	} else
		return;
}

/*//////////////////////////////////////////////////////////////////////////////*/
/* PSR_CC_ReleasePBBuf*/
/* Release designated Pbbuf slot*/
/* 1. When the Slot' data has been read complete, we will call this function to*/
/*    release slot into pbbuf's free slot list*/
/* 2. Remove the to-Free slot's from the Parser CC slots array-arPBBuf*/
/* 3. If there is no slots in the parser cc for read, clear CCF_PBBUF_EXIST flag*/
/*//////////////////////////////////////////////////////////////////////////////*/
MRESULT PSR_CC_ReleasePBBuf(PSR_CC *prPsrCC, u32 u4PbbufIdx)
{
	DMX_READ_BUFFER *prPbbuf = NULL;
	MRESULT      mrRet    = RET_DMX_OK;

	if ((MAX_CACHE_PBBUF <= u4PbbufIdx) ||
	    (NULL == prPsrCC)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPbbuf = &(prPsrCC->arPBBuf[u4PbbufIdx]);

	if (NULL != prPbbuf->pcPlayBuffer) {
		mrRet = PBBUF_ReleaseNoUseSlot(prPsrCC->pvSptHdl, prPbbuf);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in PBBUF_ReleaseNoUseSlot, mrRet: 0x%x\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
	}

	dmx_memset(prPbbuf, 0, sizeof(DMX_READ_BUFFER));

	if ((MAX_CACHE_PBBUF - 1) != u4PbbufIdx) {
		u32 u4Idx = 0;

		for (u4Idx = u4PbbufIdx; u4Idx < (MAX_CACHE_PBBUF - 1); u4Idx++) {
			dmx_memcpy(&(prPsrCC->arPBBuf[u4Idx]), &(prPsrCC->arPBBuf[u4Idx + 1]),
				   sizeof(DMX_READ_BUFFER));
		}

		dmx_memset(&(prPsrCC->arPBBuf[MAX_CACHE_PBBUF - 1]), 0, sizeof(DMX_READ_BUFFER));
	}

	if (NULL == prPsrCC->arPBBuf[0].pcPlayBuffer)
		prPsrCC->u4Flag &= (~CCF_PBBUF_EXIST);

	MM_RETURN(RET_DMX_OK);
}

/*///////////////////////////////////////////////////////////////////////////////////////*/
/* PSR_CC_ClearAllPBBufInfo*/
/* Remove all PBbuf Slots in Parser CC, clear CCF_PBBUF_EXIST flag*/
/*///////////////////////////////////////////////////////////////////////////////////////*/
void PSR_CC_ClearAllPBBufInfo(PSR_CC *prPsrCC)
{
	u32 u4Idx = 0;

	DMXLOG_DEBUG(TEXT("[PSR] %s -- prPsrCC:0x%x, eState:0x%x, eTxState:0x%x\r\n"),
		    DMX_FUNC_NAME, prPsrCC, prPsrCC->eState, prPsrCC->eTxState);

	for (u4Idx = 0; u4Idx < MAX_CACHE_PBBUF; u4Idx++) {
		if (NULL != prPsrCC->arPBBuf[u4Idx].pcPlayBuffer)
			dmx_memset(&(prPsrCC->arPBBuf[u4Idx]), 0, sizeof(DMX_READ_BUFFER));
	}

	prPsrCC->u4Flag &= (~CCF_PBBUF_EXIST);
}

/*///////////////////////////////////////////////////////////////////////////////////////*/
/* PSR_CC_ReleaseAllPBBuf*/
/* Release all Pbbuf Slots to PBBuf free slot list, Remove all PBbuf Slots in Parser CC, clear CCF_PBBUF_EXIST flag*/
/*///////////////////////////////////////////////////////////////////////////////////////*/
void PSR_CC_ReleaseAllPBBuf(PSR_CC *prPsrCC)
{
	u32  u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PSR] %s -- prPsrCC:0x%x, eState:0x%x, eTxState:0x%x\r\n"),
		    DMX_FUNC_NAME, prPsrCC, prPsrCC->eState, prPsrCC->eTxState);

	for (u4Idx = 0; u4Idx < MAX_CACHE_PBBUF; u4Idx++) {
		if (NULL != prPsrCC->arPBBuf[u4Idx].pcPlayBuffer) {
			mrRet = PBBUF_ReleaseNoUseSlot(prPsrCC->pvSptHdl,
						       &(prPsrCC->arPBBuf[u4Idx]));

			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
						TEXT("PBBUF_ReleaseNoUseSlot, mrRet: 0x%x\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				DMX_ASSERT(FALSE);
				return;
			}

			dmx_memset(&(prPsrCC->arPBBuf[u4Idx]), 0, sizeof(DMX_READ_BUFFER));
		}
	}

	prPsrCC->u4Flag &= (~CCF_PBBUF_EXIST);
}

/*///////////////////////////////////////////////////////////////////////////////////////*/
/* PSR_CC_GetPBBufSlot*/
/* 1. If the Parser CC's to-read pbbuf slots cache is full, Release the first Pbbuf Slot*/
/* 2. Get the Sent Slot from the Pbbuf's sent slot list,*/
/* @Return   if no sent slot, return MAX_CACHE_PBBUF, otherwise, return the index in the Parser CC Pbbuf Slot Array*/
/*///////////////////////////////////////////////////////////////////////////////////////*/
MRESULT PSR_CC_GetPBBufSlot(PSR_CC *prPsrCC, u32 *pu4Idx)
{
	DMX_READ_BUFFER *prPbbuf = NULL;
	u32  u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pu4Idx) ||
	    (NULL == prPsrCC))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	DMXLOG_DEBUG(TEXT("[PSR] %s enter\r\n"), DMX_FUNC_NAME);

	/* check full, at least release one slot.*/
	if (NULL != prPsrCC->arPBBuf[MAX_CACHE_PBBUF - 1].pcPlayBuffer) {
		mrRet = PSR_CC_ReleasePBBuf(prPsrCC, 0);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
					TEXT("PSR_CC_ReleasePBBuf(0), mrRet: 0x%x\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
	}

	for (u4Idx = 0; u4Idx < MAX_CACHE_PBBUF; u4Idx++) {
		if (NULL == prPsrCC->arPBBuf[u4Idx].pcPlayBuffer) {
			while (TRUE) {
				prPbbuf = &(prPsrCC->arPBBuf[u4Idx]);
				mrRet = PBBUF_GetAvailDataSlot(prPsrCC->pvSptHdl, prPbbuf);

				if (RET_DMX_PBBUF_BUSY == mrRet) {
					DMXLOG_DEBUG(TEXT("[PSR] %s fail in ")
							TEXT("PBBUF_GetAvailDataSlot, mrRet = BUSY\r\n"),
						    DMX_FUNC_NAME);
					*pu4Idx = MAX_CACHE_PBBUF;
					MM_RETURN(RET_DMX_OK);
				} else if (DMX_FAILED(mrRet)) {
					DMXLOG_DEBUG(TEXT("[PSR] %s fail in ")
							TEXT("PBBUF_GetAvailDataSlot, mrRet = 0x%x\r\n"),
						    DMX_FUNC_NAME, mrRet);
					DMX_ASSERT(FALSE);
					MM_RETURN(mrRet);
				}

				if (NULL == prPbbuf->pcPlayBuffer) {
					DMXLOG_DEBUG(TEXT("[PSR] %s fail for ")
							TEXT("prPbbuf->pcPlayBuffer == NULL, mrRet = 0x%x\r\n"),
						    DMX_FUNC_NAME, mrRet);
					dmx_memset((void *)prPbbuf, 0, sizeof(DMX_READ_BUFFER));
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_UNEXPECT);
				}

				break;
			}

			prPsrCC->u4Flag |= CCF_PBBUF_EXIST;
			*pu4Idx = u4Idx;
			MM_RETURN(RET_DMX_OK);
		}
	}

	DMXLOG_DEBUG(TEXT("[PSR] %s -- Cannot get buf, Ofst:0x%08x%08x\r\n"),
		    DMX_FUNC_NAME, (u32)(prPsrCC->u8TxCurrOffset>>32),
		    (u32)prPsrCC->u8TxCurrOffset);

	*pu4Idx = MAX_CACHE_PBBUF;
	MM_RETURN(RET_DMX_OK);
}

/*//////////////////////////////////////////////////////////////////////////////*/
/* PSR_CC_IsOffsetInPbbuf*/
/* Check whether the designated offset is in the designated PBBuf*/
/*//////////////////////////////////////////////////////////////////////////////*/
bool PSR_CC_IsOffsetInPbbuf(PSR_CC *prPsrCC, u32 u4PbbufIdx, u64 u8Offset)
{
	DMX_READ_BUFFER *prPbbuf = NULL;

	if ((MAX_CACHE_PBBUF <= u4PbbufIdx) ||
	    (NULL == prPsrCC)) {
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	prPbbuf = &prPsrCC->arPBBuf[u4PbbufIdx];

	if (NULL == prPbbuf)
		return FALSE;

	if (0 != prPbbuf->u4PlayOffset) {
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	if ((u8Offset >= (prPbbuf->u8SrcOffset + prPbbuf->u4PlayOffset)) &&
	    (u8Offset < (prPbbuf->u8SrcOffset + prPbbuf->u4PlaySize)))
		return TRUE;

	if (prPbbuf->u8SrcOffset > u8Offset)
		return FALSE;

	return FALSE;
}

/*///////////////////////////////////////////////////////////////////////////////////////*/
/* PSR_CC_GetWaitTxBufSa*/
/* 1. If Current Tx Source is PBBUF, Get the Tx Data SA in PBbuf*/
/* 2. If Current Tx Source is Memory, Get the Tx Data SA in Memory*/
/*///////////////////////////////////////////////////////////////////////////////////////*/
MRESULT PSR_CC_GetWaitTxBufSa(PSR_CC *prPsrCC, uintptr_t *pptrSa)
{
	PSR_FILTER *pPsrFtr = NULL;
	uintptr_t ptrSa = 0;

	if (!((NULL != prPsrCC) && (NULL != pptrSa))) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	pPsrFtr = (PSR_FILTER *)prPsrCC->pvActFilter;

	if (NULL == pPsrFtr) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail ")
				TEXT("in PsrCC(0x%x)'s ActFilter is NULL\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	/* check source is memory or pbbuf*/
	if (pPsrFtr->u4Flag & FF_TX_PBBUF) { /* Tx source is pbbbuf*/
		if (prPsrCC->u4Flag & CCF_PBBUF_EXIST) {
			DMX_READ_BUFFER *pbbuf = &prPsrCC->arPBBuf[prPsrCC->u4TxPBBufIdx];

			ptrSa = (uintptr_t)pbbuf->pcPlayBuffer;

			if (prPsrCC->u8TxCurrOffset > pbbuf->u8SrcOffset)
				ptrSa += (u32)(prPsrCC->u8TxCurrOffset - pbbuf->u8SrcOffset);
		} else {
			/* this should not happen*/
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for")
					TEXT(" PsrCC(0x%x)'s u4Flag & CCF_PBBUF_EXIT is FALSE\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_ERR_STATE);
		}
	} else /* Tx source is memory*/
		ptrSa = prPsrCC->ptrSrcMemSa + prPsrCC->u4MemOffset;

	*pptrSa = ptrSa;

	MM_RETURN(RET_DMX_OK);
}

/*///////////////////////////////////////////////////////////////////////////////////////*/
/* PSR_CC_GetPBBufStatus*/
/* Get the info of the First PBBuf slot in Parser CC PbBuf cache, include srcofst, datasize, and slotbufsize*/
/*///////////////////////////////////////////////////////////////////////////////////////*/
void PSR_CC_GetPBBufStatus(PSR_CC *prPsrCC, PSR_PBBUFInfo *prPBBufInfo)
{
	if (!((NULL != prPsrCC) && (NULL != prPBBufInfo))) {
		DMX_ASSERT(FALSE);
		return;
	}

	prPBBufInfo->fgPbbufReleaseNotify = (prPsrCC->u4Flag & CCF_PBBUF_RELEASE_NOTIFY);

	if (prPsrCC->u4Flag & CCF_PBBUF_EXIST) {
		prPBBufInfo->u8CurSlotSrcOfst = prPsrCC->arPBBuf[0].u8SrcOffset;
		prPBBufInfo->u4CurSlotDataSize = prPsrCC->arPBBuf[0].u4DataSize;
		prPBBufInfo->u4SlotBufSize = prPsrCC->arPBBuf[0].u4BufferSize;
	} else {
		prPBBufInfo->u8CurSlotSrcOfst = 0;
		prPBBufInfo->u4CurSlotDataSize = 0;
		prPBBufInfo->u4SlotBufSize = 0;
	}
}

/*///////////////////////////////////////////////////////////////////////////////////////*/
/* PSR_CC_GetCurPbbufStartOffset*/
/* Get Parser CC's Current Reading PBBUF Slot's SrcOffset*/
/*///////////////////////////////////////////////////////////////////////////////////////*/
MRESULT PSR_CC_GetCurPbbufStartOffset(PSR_CC *prPsrCC, u64 *pu8Offset)
{
	s32 i;

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
		DMXLOG_ERROR(TEXT("[PSR] %s line %d failed for invalid args\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*pu8Offset = 0;

	if (prPsrCC->u4Flag & CCF_PBBUF_EXIST)
		*pu8Offset = prPsrCC->arPBBuf[0].u8SrcOffset;

	MM_RETURN(RET_DMX_OK);
}

/*//////////////////////////////////////////////////////////////////////////////*/
/* PSR_CC_IsPbbufUnCon*/
/* Check whether the Pbbuf unContinuous*/
/*//////////////////////////////////////////////////////////////////////////////*/
bool PSR_CC_IsPbbufUnCon(PSR_CC *prPsrCC, u32 u4PbbufIdx)
{
	DMX_READ_BUFFER *prPbbuf = NULL;

	if ((MAX_CACHE_PBBUF <= u4PbbufIdx) ||
	    (NULL == prPsrCC)) {
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	prPbbuf = &prPsrCC->arPBBuf[u4PbbufIdx];

	if (NULL == prPbbuf)
		return FALSE;

	if (0 != prPbbuf->u4PlayOffset) {
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	if (PBBUF_SLOT_NORMAL != prPbbuf->rHeader.eType)
		return TRUE;

	return FALSE;
}

#if DMX_SUPPORT_DIVXDRM

MRESULT PSR_CC_GetWaitTxBufInfo(PSR_CC *prPsrCC, u64 u8Offset,
				uintptr_t *pptrSa, u64 *pu8Sz)
{
	DMX_READ_BUFFER *prPbbuf = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prPsrCC) ||
	    (NULL == pptrSa)   ||
	    (NULL == pu8Sz)   ||
	    (NULL == prPsrCC->hActFilter)) {
		DMXLOG_ERROR(TEXT("[DECRYPT] %s line %d fail for invalid args\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if ((0 == (prPsrCC->u4Flag & CCF_CPS_ON)) ||
	    (0 == prPsrCC->rDecryptMan.u4DecryptLen)) {
		mrRet = PSR_CC_GetWaitTxBufSa(prPsrCC, pptrSa);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail ")
					TEXT("in PSR_CC_GetWaitTxBufSa, mrRet: 0x%x, PsrCC: 0x%x!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prPsrCC);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		mrRet = PSR_CC_GetWaitTxBufSize(prPsrCC, pu8Sz);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail ")
					TEXT("in PSR_CC_GetWaitTxBufSize, mrRet: 0x%x, PsrCC: 0x%x!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prPsrCC);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		if (0 == *pu8Sz) {
#ifdef __linux__
			DMXLOG_DEBUG(
				    TEXT("[PSR] %s line %d err-- TxCurrOffset(%lld),")
				    TEXT(" TxCurrLen(%lld), TxStartOffset(%lld), ")
				    TEXT("TxLen(%lld), DecryptStOfst(%lld), DecLen(%d)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO,
				    prPsrCC->u8TxCurrOffset, prPsrCC->u8TxCurrLen,
				    prPsrCC->u8TxStartOffset, prPsrCC->u8TxLen,
				    prPsrCC->rDecryptMan.u8DecryptStOft,
				    prPsrCC->rDecryptMan.u4DecryptLen);

			if (prPsrCC->u4TxPBBufIdx < MAX_CACHE_PBBUF) {
				DMX_READ_BUFFER *pbbuf = &(prPsrCC->arPBBuf[prPsrCC->u4TxPBBufIdx]);

				DMXLOG_TRACE(
					    TEXT("[PSR] %s line %d err-- PsrCC's state: %d,")
					    TEXT(" eTxState: %d, TxPBBufIdx(%d), u8SrcOffset(%lld),")
					    TEXT(" u4DataSize(%d)\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO,
					    prPsrCC->eState, prPsrCC->eTxState,
					    prPsrCC->u4TxPBBufIdx,
					    pbbuf->u8SrcOffset, pbbuf->u4DataSize);
			}

#else
			DMXLOG_DEBUG(
				    TEXT("[PSR] %s line %d err-- TxCurrOffset(%I64d),")
				    TEXT(" TxCurrLen(%I64d), TxStartOffset(%I64d), TxLen(%I64d),")
				    TEXT(" DecryptStOfst(%I64d), DecLen(%d)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO,
				    prPsrCC->u8TxCurrOffset, prPsrCC->u8TxCurrLen,
				    prPsrCC->u8TxStartOffset, prPsrCC->u8TxLen,
				    prPsrCC->rDecryptMan.u8DecryptStOft,
				    prPsrCC->rDecryptMan.u4DecryptLen);

			if (prPsrCC->u4TxPBBufIdx < MAX_CACHE_PBBUF) {
				DMX_READ_BUFFER *pbbuf = &(prPsrCC->arPBBuf[prPsrCC->u4TxPBBufIdx]);

				DMXLOG_TRACE(
					    TEXT("[PSR] %s line %d err-- PsrCC's state: %d,")
					    TEXT(" eTxState: %d, TxPBBufIdx(%d), u8SrcOffset(%I64d),")
					    TEXT(" u4DataSize(%d)\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO,
					    prPsrCC->eState, prPsrCC->eTxState,
					    prPsrCC->u4TxPBBufIdx,
					    pbbuf->u8SrcOffset, pbbuf->u4DataSize);
			}

#endif /* #ifdef __linux__*/
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		MM_RETURN(RET_DMX_OK);
	}

#if DMX_DRM_DECRYPT_USE_HW

	if ((DECRYPT_COMPLETE == prPsrCC->rDecryptMan.eStatus) &&
	    (DECRYPT_BY_HW == prPsrCC->rDecryptMan.eMethod)) {
		/* the slot contains the whole encrypted data*/
		mrRet = PSR_CC_GetWaitTxBufSa(prPsrCC, pptrSa);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail ")
					TEXT("in PSR_CC_GetWaitTxBufSa, mrRet: 0x%x, PsrCC: 0x%x!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prPsrCC);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		mrRet = PSR_CC_GetWaitTxBufSize(prPsrCC, pu8Sz);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail ")
					TEXT("in PSR_CC_GetWaitTxBufSize, mrRet: 0x%x, PsrCC: 0x%x!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prPsrCC);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
		prPsrCC->rDecryptMan.rHWParam.rDRM.eInSlotType =  DECRYPT_NOT_IN_SLOT;
	}

#endif /* DMX_DRM_DECRYPT_USE_HW*/

	prPbbuf = (DMX_READ_BUFFER *)(&(prPsrCC->arPBBuf[prPsrCC->u4TxPBBufIdx]));
#ifdef __linux__
	DMXLOG_DEBUG(
		    TEXT("[PSR] %s line %d -- TxCurrOffset(%lld), ")
		    TEXT("TxCurrLen(%lld), TxStartOffset(%lld), TxLen(%lld), ")
		    TEXT("DecryptStOfst(%lld), DecLen(%d)\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO,
		    prPsrCC->u8TxCurrOffset, prPsrCC->u8TxCurrLen,
		    prPsrCC->u8TxStartOffset, prPsrCC->u8TxLen,
		    prPsrCC->rDecryptMan.u8DecryptStOft,
		    prPsrCC->rDecryptMan.u4DecryptLen);
#else
	DMXLOG_DEBUG(
		    TEXT("[PSR] %s line %d -- TxCurrOffset(%I64d), ")
		    TEXT("TxCurrLen(%I64d), TxStartOffset(%I64d), TxLen(%I64d),")
		    TEXT(" DecryptStOfst(%I64d), DecLen(%d)\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO,
		    prPsrCC->u8TxCurrOffset, prPsrCC->u8TxCurrLen,
		    prPsrCC->u8TxStartOffset, prPsrCC->u8TxLen,
		    prPsrCC->rDecryptMan.u8DecryptStOft,
		    prPsrCC->rDecryptMan.u4DecryptLen);
#endif /* #ifdef __linux__*/

	if ((prPbbuf->u8SrcOffset <= prPsrCC->rDecryptMan.u8DecryptStOft) &&
	    (prPsrCC->rDecryptMan.u8DecryptStOft + prPsrCC->rDecryptMan.u4DecryptLen <= prPbbuf->u8SrcOffset +
	     prPbbuf->u4DataSize)) {
		/* the slot contained the whole encrypted data*/
		if ((DECRYPT_COMPLETE != prPsrCC->rDecryptMan.eStatus) &&
		    (DECRYPT_BY_SW == prPsrCC->rDecryptMan.eMethod)) {
			u8 *pu1FrameData = (u8 *)(prPbbuf->pcPlayBuffer);

			pu1FrameData += (u32)(prPsrCC->rDecryptMan.u8DecryptStOft - prPbbuf->u8SrcOffset);

#if DMX_PFM_TEST

			if (SPT_DATA_V == prPsrCC->rDecryptMan.eDataType)
				g_rPsrPfm.rDecryptV.u8DecryptPb2FifoCnt++;
			else
				g_rPsrPfm.rDecryptA.u8DecryptPb2FifoCnt++;

#endif /* DMX_PFM_TEST*/

#if DMX_PRINT_DECRYPT_KEY_LOG
#ifdef __linux__
			DMXLOG_TRACE(
				    TEXT("[PSR] %s line %d -- PSR_Decrypt_DecryptData(InSlot, %s),")
				    TEXT(" DecryptStOfst(%lld), DecryptLen(%d)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO,
				    ((prPsrCC->rDecryptMan.eDataType < MAX_SPT_DATA_TYPE_CNT) ?
				    g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
				     TEXT("UNKNOWN")),
				    prPsrCC->rDecryptMan.u8DecryptStOft,
				    prPsrCC->rDecryptMan.u4DecryptLen);
#else
			DMXLOG_TRACE(
				    TEXT("[PSR] %s line %d -- PSR_Decrypt_DecryptData(InSlot, %s),")
				    TEXT(" DecryptStOfst(%I64d), DecryptLen(%d)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO,
				    ((prPsrCC->rDecryptMan.eDataType < MAX_SPT_DATA_TYPE_CNT) ?
				    g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
				     TEXT("UNKNOWN")),
				    prPsrCC->rDecryptMan.u8DecryptStOft,
				    prPsrCC->rDecryptMan.u4DecryptLen);
#endif /* #ifdef __linux__*/
#endif /* DMX_PRINT_DECRYPT_KEY_LOG*/

			if (g_rDmxCliMan.fgDumpFlow) {
				DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

				mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
				rOperInfo.pvSptHdl = prPsrCC->pvSptHdl;
				rOperInfo.unFlow.rDecrypt.u4StmType = prPsrCC->rDecryptMan.eDataType;
				rOperInfo.unFlow.rDecrypt.u8FileOfst =  prPsrCC->rDecryptMan.u8DecryptStOft;
				rOperInfo.unFlow.rDecrypt.u4Len =  prPsrCC->rDecryptMan.u4DecryptLen;
				DmxDumpFlow(DMX_OPER_DECRYPT, &rOperInfo);
			}

			if ((0 != (((u32)pu1FrameData) % sizeof(u32))) ||
			    (0 != ((prPsrCC->rDecryptMan.u4DecryptLen) % (prPsrCC->rDecryptMan.u4AlignSize)))) {
				if (prPsrCC->rDecryptMan.u4TxMemSize < prPsrCC->rDecryptMan.u4DecryptLen) {
					DMX_FreeHwMemory(prPsrCC->rDecryptMan.ptrTxMemAddr);
					prPsrCC->rDecryptMan.ptrTxMemAddr = 0;
					prPsrCC->rDecryptMan.u4TxMemSize = 0;
					prPsrCC->rDecryptMan.ptrTxMemWPtr = 0;
					prPsrCC->rDecryptMan.ptrTxMemRPtr = 0;
#ifdef __linux__
					DMX_NewHwAlignMemory(prPsrCC->rDecryptMan.u4DecryptLen,
								DMX_DECRYPT_DIVXDRM_DECRYPTION_ALIGNSIZE,
							    prPsrCC->rDecryptMan.ptrTxMemAddr);
#else
					DMX_NewHwAlignMemory(prPsrCC->rDecryptMan.u4DecryptLen,
								DMX_DECRYPT_DIVXDRM_DECRYPTION_ALIGNSIZE,
							    (void *)(prPsrCC->rDecryptMan.ptrTxMemAddr));
#endif

					if (0 == prPsrCC->rDecryptMan.ptrTxMemAddr) {
						DMXLOG_ERROR(TEXT("[PSR] %s line %d fail ")
								TEXT("in alloc DIVXDRM Working Buffer, ")
								TEXT("PsrCC: 0x%x\r\n"),
							    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC);
						MM_RETURN(RET_DMX_NO_MEM);
					}

					prPsrCC->rDecryptMan.u4TxMemSize = prPsrCC->rDecryptMan.u4DecryptLen;
					dmx_memset((void *)(prPsrCC->rDecryptMan.ptrTxMemAddr), 0,
							prPsrCC->rDecryptMan.u4TxMemSize);
				}

				dmx_memcpy((void *)(prPsrCC->rDecryptMan.ptrTxMemAddr), pu1FrameData,
						prPsrCC->rDecryptMan.u4DecryptLen);
				mrRet = PSR_Decrypt_DecryptData(prPsrCC, prPsrCC->rDecryptMan.eDataType,
								(u8 *)(prPsrCC->rDecryptMan.ptrTxMemAddr),
								prPsrCC->rDecryptMan.u4DecryptLen);

				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(
						    TEXT("[DECRYPT] %s line %d fail in ")
						    TEXT("PSR_Decrypt_DecryptData(%s, u4DecryptLen:")
						    TEXT(" %d), mrRet: 0x%x\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    ((prPsrCC->rDecryptMan.eDataType < MAX_SPT_DATA_TYPE_CNT) ?
						    g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
						     TEXT("UNKNOWN")),
						    prPsrCC->rDecryptMan.u4DecryptLen, mrRet);
					MM_RETURN(RET_DMX_PARAM_WRONG);
				}

				dmx_memcpy(pu1FrameData, (void *)(prPsrCC->rDecryptMan.ptrTxMemAddr),
					   prPsrCC->rDecryptMan.u4DecryptLen);
			} else {
				mrRet = PSR_Decrypt_DecryptData(prPsrCC, prPsrCC->rDecryptMan.eDataType,
								pu1FrameData, prPsrCC->rDecryptMan.u4DecryptLen);

				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(
						    TEXT("[DECRYPT] %s line %d fail in ")
						    TEXT("PSR_Decrypt_DecryptData(%s), u4DecryptLen: %d),")
						    TEXT(" mrRet: 0x%x\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    ((prPsrCC->rDecryptMan.eDataType < MAX_SPT_DATA_TYPE_CNT) ?
						    g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
						     TEXT("UNKNOWN")),
						    prPsrCC->rDecryptMan.u4DecryptLen, mrRet);
					MM_RETURN(RET_DMX_PARAM_WRONG);
				}
			}
		}

		mrRet = PSR_CC_GetWaitTxBufSa(prPsrCC, pptrSa);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in")
					TEXT(" PSR_CC_GetWaitTxBufSa, mrRet: 0x%x, PsrCC: 0x%x!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prPsrCC);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		mrRet = PSR_CC_GetWaitTxBufSize(prPsrCC, pu8Sz);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in")
					TEXT(" PSR_CC_GetWaitTxBufSize, mrRet: 0x%x, PsrCC: 0x%x!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prPsrCC);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
#if DMX_DRM_DECRYPT_USE_HW
		prPsrCC->rDecryptMan.rHWParam.rDRM.eInSlotType =  DECRYPT_WHOLE_IN_SLOT;
#endif /* DMX_DRM_DECRYPT_USE_HW*/
	} else if (prPbbuf->u8SrcOffset + prPbbuf->u4DataSize <= prPsrCC->rDecryptMan.u8DecryptStOft) {
		/* Encrypted data locates in the next pbbuf slot, it has been decrypted and doesn't accross slot*/

		mrRet = PSR_CC_GetWaitTxBufSa(prPsrCC, pptrSa);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
					TEXT("PSR_CC_GetWaitTxBufSa, mrRet: 0x%x, PsrCC: 0x%x!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prPsrCC);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		mrRet = PSR_CC_GetWaitTxBufSize(prPsrCC, pu8Sz);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
					TEXT("PSR_CC_GetWaitTxBufSize, mrRet: 0x%x, PsrCC: 0x%x!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prPsrCC);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}
#if DMX_DRM_DECRYPT_USE_HW
		prPsrCC->rDecryptMan.rHWParam.rDRM.eInSlotType =  DECRYPT_NOT_IN_SLOT;
#endif /* DMX_DRM_DECRYPT_USE_HW*/
	} else {
		/* Encrypted data accrosses slot*/

		u64 u8TxEndOffset = 0;
		u64 u8TxStOffset = 0;
		u64 u8TxSize = 0;

		if (u8Offset < prPsrCC->rDecryptMan.u8DecryptStOft) {
			/* DMA Data before encrypted data*/
			if (!(prPsrCC->u4Flag & CCF_PBBUF_EXIST)) {
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			mrRet = PSR_CC_GetWaitTxBufSa(prPsrCC, pptrSa);

			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
						TEXT("PSR_CC_GetWaitTxBufSa, mrRet: 0x%x, PsrCC: 0x%x!!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prPsrCC);
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}

			/* if the tx data cross two slots, u8TxOffset is
			* the first slot end, otherwise, is tx end offset*/
			u8TxEndOffset = DMX_MIN((prPsrCC->rDecryptMan.u8DecryptStOft),
						(prPbbuf->u8SrcOffset + prPbbuf->u4PlaySize));

			u8TxStOffset = DMX_MAX(prPsrCC->u8TxCurrOffset, prPbbuf->u8SrcOffset);

			if (u8TxStOffset >= u8TxEndOffset) {
#ifdef __linux__
				DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for tx start ")
						TEXT("offset(%lld) >= end offset(%lld), PsrCC: 0x%x!!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO,
					    u8TxStOffset, u8TxEndOffset, prPsrCC);
#else
				DMXLOG_ERROR(
					    TEXT("[PSR] %s line %d fail for tx start offset(%I64d) ")
					    TEXT(">= end offset(%I64d), PsrCC: 0x%x!!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO,
					    u8TxStOffset, u8TxEndOffset, prPsrCC);
#endif /* #ifdef __linux__*/
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			u8TxSize = u8TxEndOffset - u8TxStOffset;
#if DMX_DRM_DECRYPT_USE_HW
			prPsrCC->rDecryptMan.rHWParam.rDRM.eInSlotType = DECRYPT_NOT_IN_SLOT;
#endif /* DMX_DRM_DECRYPT_USE_HW*/
			*pu8Sz = u8TxSize;
		} else if (prPsrCC->rDecryptMan.u8DecryptStOft + prPsrCC->rDecryptMan.u4DecryptLen < u8Offset) {
			/* DMA Data after the encrypted data*/
			mrRet = PSR_CC_GetWaitTxBufSa(prPsrCC, pptrSa);

			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
						TEXT("PSR_CC_GetWaitTxBufSa, mrRet: 0x%x, PsrCC: 0x%x!!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prPsrCC);
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}

			mrRet = PSR_CC_GetWaitTxBufSize(prPsrCC, pu8Sz);

			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
						TEXT("PSR_CC_GetWaitTxBufSize, mrRet: 0x%x, PsrCC: 0x%x!!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prPsrCC);
				DMX_ASSERT(FALSE);
				MM_RETURN(mrRet);
			}
		} else if (prPsrCC->rDecryptMan.u8DecryptStOft + prPsrCC->rDecryptMan.u4DecryptLen == u8Offset) {
			/* Start DMA encrypted data that is accrossing slot*/
			if ((DECRYPT_COMPLETE != prPsrCC->rDecryptMan.eStatus) &&
			    (DECRYPT_BY_SW == prPsrCC->rDecryptMan.eMethod)) {
				u8 *pu1FrameData = (u8 *)(prPsrCC->rDecryptMan.ptrTxMemAddr);

#if DMX_PRINT_DECRYPT_KEY_LOG
#ifdef __linux__
				DMXLOG_TRACE(
					    TEXT("[PSR] %s line %d -- PSR_Decrypt_DecryptData")
					    TEXT("(AccrossSlot, %s), DecryptStOfst(%lld), ptrTxMemWPtr(%d),")
					    TEXT(" DecryptLen(%d)\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO,
					    ((prPsrCC->rDecryptMan.eDataType < MAX_SPT_DATA_TYPE_CNT) ?
					    g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
					     TEXT("UNKNOWN")),
					    prPsrCC->rDecryptMan.u8DecryptStOft,
					    prPsrCC->rDecryptMan.ptrTxMemWPtr,
					    prPsrCC->rDecryptMan.u4DecryptLen);
#else
				DMXLOG_TRACE(
					    TEXT("[PSR] %s line %d -- PSR_Decrypt_DecryptData")
					    TEXT("(AccrossSlot, %s), DecryptStOfst(%I64d), ptrTxMemWPtr(%d),")
					    TEXT(" DecryptLen(%d)\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO,
					    ((prPsrCC->rDecryptMan.eDataType < MAX_SPT_DATA_TYPE_CNT) ?
					    g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
					     TEXT("UNKNOWN")),
					    prPsrCC->rDecryptMan.u8DecryptStOft,
					    prPsrCC->rDecryptMan.ptrTxMemWPtr,
					    prPsrCC->rDecryptMan.u4DecryptLen);
#endif /* #ifdef __linux__*/
#endif /* DMX_PRINT_DECRYPT_KEY_LOG*/

				mrRet = PSR_Decrypt_DecryptData(prPsrCC, prPsrCC->rDecryptMan.eDataType,
								pu1FrameData, prPsrCC->rDecryptMan.u4DecryptLen);

				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(
						    TEXT("[DECRYPT] %s line %d fail in ")
						    TEXT("PSR_Decrypt_DecryptData(%s, u4DecryptLen:")
						    TEXT(" %d), mrRet: 0x%x\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    ((prPsrCC->rDecryptMan.eDataType < MAX_SPT_DATA_TYPE_CNT) ?
						    g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
						     TEXT("UNKNOWN")),
						    prPsrCC->rDecryptMan.u4DecryptLen, mrRet);
					MM_RETURN(RET_DMX_PARAM_WRONG);
				}
			}

#if DMX_PRINT_DECRYPT_KEY_LOG
#ifdef __linux__
			DMXLOG_TRACE(
				    TEXT("[PSR] %s line %d -- Start to DMA Decrypt Data(AccrossSlot, %s), ")
				    TEXT("DecryptStOfst(%lld), ptrTxMemWPtr(%d), DecryptLen(%d), TxCurOfst(%lld)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO,
				    ((prPsrCC->rDecryptMan.eDataType < MAX_SPT_DATA_TYPE_CNT) ?
				    g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
				     TEXT("UNKNOWN")),
				    prPsrCC->rDecryptMan.u8DecryptStOft,
				    prPsrCC->rDecryptMan.ptrTxMemWPtr,
				    prPsrCC->rDecryptMan.u4DecryptLen,
				    prPsrCC->u8TxCurrOffset);
#else
			DMXLOG_TRACE(
				    TEXT("[PSR] %s line %d -- Start to DMA Decrypt ")
				    TEXT("Data(AccrossSlot, %s), DecryptStOfst(%I64d), ptrTxMemWPtr(%d),")
				    TEXT(" DecryptLen(%d), TxCurOfst(%I64d)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO,
				    ((prPsrCC->rDecryptMan.eDataType < MAX_SPT_DATA_TYPE_CNT) ?
				    g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
				     TEXT("UNKNOWN")),
				    prPsrCC->rDecryptMan.u8DecryptStOft,
				    prPsrCC->rDecryptMan.ptrTxMemWPtr,
				    prPsrCC->rDecryptMan.u4DecryptLen,
				    prPsrCC->u8TxCurrOffset);
#endif /* #ifdef __linux__*/
#endif /*  DMX_PRINT_DECRYPT_KEY_LOG*/

			if (prPsrCC->rDecryptMan.ptrTxMemRPtr > 0) {
				/* encrypted data has been dmaed, so DMA Data after encrypted data*/
				mrRet = PSR_CC_GetWaitTxBufSa(prPsrCC, pptrSa);

				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
							TEXT("PSR_CC_GetWaitTxBufSa, mrRet: 0x%x, PsrCC: 0x%x!!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prPsrCC);
					DMX_ASSERT(FALSE);
					MM_RETURN(mrRet);
				}

				mrRet = PSR_CC_GetWaitTxBufSize(prPsrCC, pu8Sz);

				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
							TEXT("PSR_CC_GetWaitTxBufSize, mrRet: 0x%x, PsrCC: 0x%x!!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO, mrRet, prPsrCC);
					DMX_ASSERT(FALSE);
					MM_RETURN(mrRet);
				}
#if DMX_DRM_DECRYPT_USE_HW
				prPsrCC->rDecryptMan.rHWParam.rDRM.eInSlotType = DECRYPT_NOT_IN_SLOT;
#endif /* DMX_DRM_DECRYPT_USE_HW*/
			} else {
				/* Start DMA encrypted data that is accrossing slot*/
				/* change the CurTxOfset to be start offset of decrypted data*/
				prPsrCC->u8TxCurrOffset = prPsrCC->rDecryptMan.u8DecryptStOft;

				*pptrSa = prPsrCC->rDecryptMan.ptrTxMemAddr;
				*pu8Sz = (u64)(prPsrCC->rDecryptMan.u4DecryptLen);
				prPsrCC->rDecryptMan.ptrTxMemRPtr = prPsrCC->rDecryptMan.u4DecryptLen;
#if DMX_DRM_DECRYPT_USE_HW
				prPsrCC->rDecryptMan.rHWParam.rDRM.eInSlotType = DECRYPT_ACCROSS_SLOTS;
#endif /* DMX_DRM_DECRYPT_USE_HW*/
			}

			MM_RETURN(RET_DMX_OK);
		} else if ((prPsrCC->rDecryptMan.u8DecryptStOft <= u8Offset) &&
			   (u8Offset < prPsrCC->rDecryptMan.u8DecryptStOft + prPsrCC->rDecryptMan.u4DecryptLen)) {
			/* Encrypted Data hasn't been decrypted, may be the whole*/
			/* encrypted data hasn't been put into the temp buffer*/
			if (DECRYPT_COMPLETE != prPsrCC->rDecryptMan.eStatus) {
				bool fgDecryptEndOftIn = FALSE;
				E_PBBUF_CONTINUITY_TYPE_T ePbbufCon = PBBUF_CONTINUOUS;

				mrRet = PSR_Decrypt_PbbufCheck(prPsrCC, prPsrCC->u4TxPBBufIdx,
							       &fgDecryptEndOftIn, &ePbbufCon, TRUE);

				if (DMX_SUCCEED(mrRet)) {
					if (PBBUF_UNCONTINUOUS == ePbbufCon) {
						DMXLOG_TRACE(TEXT("[PSR] %s line %d -- ")
								TEXT("PSR_Decrypt_PbbufCheck  (Uncontinuous),")
								TEXT(" u4TxPBBufJumpIdx: %d!\r\n"),
							    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->u4TxPBBufJumpIdx);
						PSR_CC_SetTxSt(prPsrCC, TXS_TX_JUMP);
						MM_RETURN(RET_DMX_NEED_JUMP);
					} else if (!fgDecryptEndOftIn) {
						PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);
						MM_RETURN(RET_DMX_PBBUF_BUSY);
					} else {
						prPsrCC->u4TxPBBufIdx = 0;
#if DMX_PFM_TEST
						DmxPfmStmIncDecryptPb2Fifo(prPsrCC->rDecryptMan.eDataType);
#endif /* DMX_PFM_TEST*/

						DMX_ASSERT(DECRYPT_UNCOMPLETE != prPsrCC->rDecryptMan.eStatus);

#if DMX_PRINT_DECRYPT_KEY_LOG
#ifdef __linux__
						DMXLOG_TRACE(
							    TEXT("[PSR] %s line %d -- Start to DMA Decrypt")
							    TEXT(" Data(AccrossSlot, %s), DecryptStOfst(%lld), ")
							    TEXT("ptrTxMemWPtr(%d), DecryptLen(%d), ")
							    TEXT("TxCurOfst(%lld)\r\n"),
							    DMX_FUNC_NAME, DMX_LINE_NO,
							    ((prPsrCC->rDecryptMan.eDataType < MAX_SPT_DATA_TYPE_CNT) ?
							    g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
							     TEXT("UNKNOWN")),
							    prPsrCC->rDecryptMan.u8DecryptStOft,
							    prPsrCC->rDecryptMan.ptrTxMemWPtr,
							    prPsrCC->rDecryptMan.u4DecryptLen,
							    prPsrCC->u8TxCurrOffset);
#else
						DMXLOG_TRACE(
							    TEXT("[PSR] %s line %d -- Start to DMA")
							    TEXT(" Decrypt Data(AccrossSlot, %s),")
							    TEXT(" DecryptStOfst(%I64d), ptrTxMemWPtr(%d), ")
							    TEXT("DecryptLen(%d), TxCurOfst(%I64d)\r\n"),
							    DMX_FUNC_NAME, DMX_LINE_NO,
							    ((prPsrCC->rDecryptMan.eDataType < MAX_SPT_DATA_TYPE_CNT) ?
							    g_aszSptDataTypeName[prPsrCC->rDecryptMan.eDataType] :
							     TEXT("UNKNOWN")),
							    prPsrCC->rDecryptMan.u8DecryptStOft,
							    prPsrCC->rDecryptMan.ptrTxMemWPtr,
							    prPsrCC->rDecryptMan.u4DecryptLen,
							    prPsrCC->u8TxCurrOffset);
#endif /* #ifdef __linux__*/
#endif /* DMX_PRINT_DECRYPT_KEY_LOG*/

						/* change the CurTxOfset to be start offset of decrypted data*/
						prPsrCC->u8TxCurrOffset = prPsrCC->rDecryptMan.u8DecryptStOft;

						*pptrSa = prPsrCC->rDecryptMan.ptrTxMemAddr;
						*pu8Sz = (u64)(prPsrCC->rDecryptMan.u4DecryptLen);
						prPsrCC->rDecryptMan.ptrTxMemRPtr = prPsrCC->rDecryptMan.u4DecryptLen;
#if DMX_DRM_DECRYPT_USE_HW
						prPsrCC->rDecryptMan.rHWParam.rDRM.eInSlotType = DECRYPT_ACCROSS_SLOTS;
#endif /* DMX_DRM_DECRYPT_USE_HW*/
						MM_RETURN(RET_DMX_OK);
					}
				} else if (RET_DMX_PBBUF_BUSY == mrRet) {
					DMX_ASSERT(!fgDecryptEndOftIn);
					PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);
					MM_RETURN(RET_DMX_PBBUF_BUSY);
				} else {
					DMXLOG_ERROR(TEXT("[PSR] %s line %d fail in ")
							TEXT("PSR_Decrypt_PbbufCheck\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO);
					MM_RETURN(mrRet);
				}
			} else {
				if (DECRYPT_BY_SW == prPsrCC->rDecryptMan.eMethod) {
					/* Encrypted Data has been decrypted, so this is in dma the decrypted
					* data in temp buffer now*/
					u32 u4AlreadyTxSz = (u32)(u8Offset - prPsrCC->rDecryptMan.u8DecryptStOft);
					*pptrSa = prPsrCC->rDecryptMan.ptrTxMemAddr + u4AlreadyTxSz;
					*pu8Sz = (u64)(prPsrCC->rDecryptMan.u4DecryptLen - u4AlreadyTxSz);
					prPsrCC->rDecryptMan.ptrTxMemRPtr = prPsrCC->rDecryptMan.u4DecryptLen;
					MM_RETURN(RET_DMX_OK);
				} else {
					/* Encrypted Data has been decrypted, so this is in dma the
					* decrypted data in temp buffer now*/
					DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for the encrypted")
							TEXT(" data should be transferred once\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO);
					MM_RETURN(RET_DMX_UNEXPECT);
				}
			}
		} else {
			DMXLOG_ERROR(
				    TEXT("[PSR] %s line %d fail for encounter unexpect situation, ")
				    TEXT("I think this process shouldn't be entered\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_UNEXPECT);
		}
	}

	MM_RETURN(mrRet);
}

#endif /* DMX_SUPPORT_DIVXDRM*/

#if CONFIG_DRV_HDMI_RX
MRESULT PSR_CC_IsAudinRaw(void *pvPsrCC, bool *pfgIsAudRaw)
{
	PSR_CC *prPsrCC = (PSR_CC *)pvPsrCC;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvPsrCC) ||
	    (NULL == pfgIsAudRaw))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	mrRet = PBBUF_AudInIsRAW(prPsrCC->pvSptHdl, pfgIsAudRaw);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in ")
				TEXT("PBBUF_AudInIsRAW (pvSptHdl 0x%x), mrRet: 0x%x!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(mrRet);
}

MRESULT PSR_CC_GetAudInParsingInfo(void *pvPsrCC, AUDIN_PARSING_INFO_T *prPsrInfo)
{
	PSR_CC *prPsrCC = (PSR_CC *)pvPsrCC;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prPsrCC) ||
	    (NULL == prPsrInfo))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	mrRet = PBBUF_GetAudInParsingInfo(prPsrCC->pvSptHdl, prPsrInfo);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in ")
				TEXT("PBBUF_GetAudInParsingInfo (pvSptHdl 0x%x), mrRet: 0x%x!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}
#endif /* CONFIG_DRV_HDMI_RX*/



