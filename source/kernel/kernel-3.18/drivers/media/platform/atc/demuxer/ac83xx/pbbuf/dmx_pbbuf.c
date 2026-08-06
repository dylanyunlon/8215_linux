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
 * @file dmx_pbbuf.c
 *
 * @par Project
 *
 * @par Description
 *
 *
 * @par Author_Name
 *	Shuhui Zhang
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
#include <media/atc/dmx_cfa_def.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "dmx_cfa_def.h"
#include "mm_debug.h"
#endif				/* __linux__*/

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_spt.h"
#include "dmx_psr_cc.h"
#include "dmx_pbbuf.h"
#include "dmx_psr_pbbuf.h"

#ifndef __linux__
#pragma warning(disable : 4127)	/*disable warning C4127: conditional expression is constant*/
#endif

EXTERN u32	g_u4PbBufFlag;
SLOT *PBBUF_GetSlotByHandle(const PBBUF * prPbBuf, void *pvSlot)
{
	SLOT *prSlot = NULL;
	u32 u4Idx = 0;
	bool fgSlotFound = FALSE;

	if ((NULL == pvSlot) || (NULL == prPbBuf)) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail for invalid args!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		return NULL;
	}

	prSlot = (SLOT *) pvSlot;
	for (u4Idx = 0; u4Idx < prPbBuf->u4SlotAmount; u4Idx++) {
		if (prSlot == &prPbBuf->prSLOT[u4Idx]) {
			fgSlotFound = TRUE;
			break;
		}
	}

	if (!fgSlotFound) {
		DMX_ASSERT(fgSlotFound);
		return NULL;
	}

	return prSlot;
}

u32 PBBUF_GetListCount(PBBUF *prPbBuf, E_PBBUF_SLOT_TYPE_T eSlotListType)
{
	if ((NULL == prPbBuf) || (PBBUF_SLOT_TYPE_CNT <= eSlotListType)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail for invalid args(eType: %d, prPbbuf: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eSlotListType, prPbBuf);
		return 0;
	}

	return prPbBuf->arSlotLists[eSlotListType].u4SlotCnt;
}

SLOT *PBBUF_GetSlotFromHead(PBBUF *prPbBuf, E_PBBUF_SLOT_TYPE_T eSlotType)
{
	if ((NULL == prPbBuf) || (PBBUF_SLOT_TYPE_CNT <= eSlotType)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail for invalid args(eType: %d, prPbbuf: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eSlotType, prPbBuf);
		return 0;
	}

	return prPbBuf->arSlotLists[eSlotType].prHeadSlot;
}

MRESULT PBBUF_RemoveSlot(PBBUF *prPbBuf, SLOT *prSlot)
{
	PBBUF_SLOT_LIST_INFO_T *prSlotList = NULL;

	if ((NULL == prSlot) || (NULL == prPbBuf) || (PBBUF_SLOT_TYPE_CNT <= prSlot->eType)) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail for invalid args!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prSlotList = &(prPbBuf->arSlotLists[prSlot->eType]);

	if (prSlotList->u4SlotCnt < 1) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d Remove slot while slot cnt is 0, SlotType: %d!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSlot->eType);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (NULL != prSlot->prPrevSlot)
		prSlot->prPrevSlot->prNextSlot = prSlot->prNextSlot;


	if (NULL != prSlot->prNextSlot)
		prSlot->prNextSlot->prPrevSlot = prSlot->prPrevSlot;


	if (prSlotList->prHeadSlot == prSlot)
		prSlotList->prHeadSlot = prSlot->prNextSlot;


	if (prSlotList->prTailSlot == prSlot)
		prSlotList->prTailSlot = prSlot->prPrevSlot;


	prSlot->prNextSlot = NULL;
	prSlot->prPrevSlot = NULL;

	prSlotList->u4SlotCnt--;

	MM_RETURN(RET_DMX_OK);
}

/******************************************************************************/
/* u32 PBBUF_AddSlotToTail(PBBUF *prPbBuf, SLOT *prSlot,*/
/*	  E_PBBUF_SLOT_TYPE_T eSlotListType)*/
/* Describe: Add a slot to the tail of the linked list*/
/* Parameters:	prLIST  [IN] the target linked list*/
/*		prSlot  [IN] the slot to be added to the linked list*/
/* return: void*/
/******************************************************************************/
MRESULT PBBUF_AddSlotToTail(PBBUF *prPbBuf, SLOT *prSlot, E_PBBUF_SLOT_TYPE_T eSlotListType)
{
	PBBUF_SLOT_LIST_INFO_T *prSlotList = NULL;

	if ((NULL == prSlot) || (NULL == prPbBuf) || (PBBUF_SLOT_TYPE_CNT <= eSlotListType)) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line fail for invalid args!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prSlotList = &(prPbBuf->arSlotLists[eSlotListType]);

	if (NULL != prSlotList->prTailSlot) {
		prSlotList->prTailSlot->prNextSlot = prSlot;
		prSlot->prPrevSlot = prSlotList->prTailSlot;
		prSlot->prNextSlot = NULL;
	} else {
		if (NULL != prSlotList->prHeadSlot) {
			DMXLOG_ERROR(
				TEXT("[PBBUF] %s line %d fail for Tail=NULL but Head!=NULL")
				TEXT(", SlotType: %d, ListType: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSlot->eType, eSlotListType);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_UNEXPECT);
		}
		prSlot->prPrevSlot = NULL;
		prSlot->prNextSlot = NULL;
	}

	prSlotList->prTailSlot = prSlot;

	if (NULL == prSlotList->prHeadSlot) {
		prSlotList->prHeadSlot = prSlot;
		prSlot->prPrevSlot = NULL;
		prSlot->prNextSlot = NULL;
	}

	prSlot->eType = eSlotListType;

	prSlotList->u4SlotCnt++;

	MM_RETURN(RET_DMX_OK);
}

MRESULT PBBUF_ReadyToGetAllocSlot(PBBUF *prPbBuf)
{
	if (NULL == prPbBuf) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail for invalid args!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (PBBUFChkNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_READY_TO_ALLOCATE_BUFFER)) {
		if ((prPbBuf->arSlotLists[PBBUF_SLOT_FREE].u4SlotCnt > 0) &&
			(NULL != prPbBuf->arSlotLists[PBBUF_SLOT_FREE].prHeadSlot)) {
			/* The FREE linked list is not empty.*/
//#if DMX_NEW_PBBUF_MECHANISM
			if (g_u4PbBufFlag) {
  			s32 i4Ret = OSR_OK;

  			PBBUFClrNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_READY_TO_ALLOCATE_BUFFER);
  			if (0 == prPbBuf->hEvtGroup) {
  				DMXLOG_ERROR(
  					TEXT("[PBBUF] %s line %d fail for prPbBuf(0x%x)'s hEvtGroup ")
  					TEXT("hasn't been create!\r\n"),
  					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);
  				MM_RETURN(RET_DMX_NO_INIT);
  			}
#if DMX_PRINT_PBBUF_DEBUG_LOG
  			DMXLOG_TRACE(
  				TEXT
  				("[GAU] %s -- prPbBuf(0x%x) Set Event EV_FREE_SLOT_IN!!!\r\n"),
  				DMX_FUNC_NAME, prPbBuf);
#endif				/* DMX_PRINT_PBBUF_DEBUG_LOG*/

  			i4Ret = x_ev_group_set_event(prPbBuf->hEvtGroup,
  					DMX_PBBUF_EV_FREE_SLOT_IN, X_EV_OP_OR);
  			if (OSR_OK != i4Ret) {
  				DMXLOG_ERROR(
  					TEXT
  					("[GAU] %s line %d failed in prPbBuf(0x%x) Set Event EV_FREE_SLOT_IN!!!\r\n"),
  					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);
  				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
  			}
			} else {
//#else				/* MX_NEW_PBBUF_MECHANISM*/
  			PBBUFClrNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_READY_TO_ALLOCATE_BUFFER);
  			if (NULL == prPbBuf->hTxData) {
  				if (SPT_PBUFF_FILE == prPbBuf->u1SptPbuffType) {
  					DMXLOG_ERROR(
  						TEXT
  						("[PBBUF] %s line %d fail for hTxData == NULL (prPbBuf: 0x%x)\r\n"),
  						DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);
  					MM_RETURN(RET_DMX_NO_INIT);
  				}
  				MM_RETURN(RET_DMX_OK);
  			}

  			DMXLOG_DEBUG(
  				TEXT("[PBBUF] %s line %d -- x_event_set(TxData), pvSptHdl(0x%x) \r\n"),
  				DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf->pvSptHdl);
  			if (!x_event_set(prPbBuf->hTxData)) {
  				DMXLOG_ERROR(
  					TEXT
  					("[PBBUF] %s line %d fail in set txdata event (prPbBuf: 0x%x), err: 0x%x\r\n"),
  					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, DMX_GET_LASTERR);
  				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
  			}
			}
//#endif				/* MX_NEW_PBBUF_MECHANISM*/
		}
	}

	MM_RETURN(RET_DMX_OK);
}

void PBBUF_ReadyToGetReadBuffer(PBBUF *prPbBuf, SLOT *prSlot)
{
	if (NULL == prPbBuf) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail for invalid args!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return;
	}

	if (PBBUFChkNfyMask(prPbBuf->u4DrvNfyMask, (u32)DRV_PBBUF_COND_READY_TO_GET_READ_BUFFER)) {
		if ((prPbBuf->arSlotLists[PBBUF_SLOT_SEND].u4SlotCnt > 0) &&
			(NULL != prPbBuf->arSlotLists[PBBUF_SLOT_SEND].prHeadSlot)) {
			/* Then SENT linked list is not empty.*/
			PBBUFClrNfyMask(prPbBuf->u4DrvNfyMask,
					DRV_PBBUF_COND_READY_TO_GET_READ_BUFFER);
			/* Add for Check not sent EOS while playing video in SD Card(mtk40144)*/
			DMXLOG_DEBUG(
				TEXT("[Pbbuf] +++++++++++ Pbbuf Send List Empty -> ")
				TEXT("Not Empty (hPsr: 0x%p) +++++++++++ \r\n"),
				prPbBuf->pvDrvOwner);
			PSR_CC_PBBuf_Notify(prPbBuf->pvDrvOwner,
						DRV_PBBUF_COND_READY_TO_GET_READ_BUFFER, 0);
		}
	}
}

MRESULT PBBUF_BufferCleaned(PBBUF *prPbBuf)
{
	SLOT *prSlot = NULL;
	u32 u4Count = 0;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prPbBuf) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d exit, prPbBuf = NULL!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OK);
	}

	if (PBBUFChkNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_BUFFER_CLEANED)) {
		u4Count = PBBUF_GetListCount(prPbBuf, PBBUF_SLOT_READING);
		if (u4Count > 0) {
			DMXLOG_TRACE(
				TEXT("[PBBUF] %s exit, ReadingSlotCnt(%d), SendSlotCnt(%d), ")
				TEXT("AllocSltCnt(%d), FreeSlotCnt(%d), SlotAmount(%d)!\r\n"),
				DMX_FUNC_NAME, u4Count, PBBUF_GetListCount(prPbBuf,
					PBBUF_SLOT_SEND),
				PBBUF_GetListCount(prPbBuf, PBBUF_SLOT_ALLOCATED),
				PBBUF_GetListCount(prPbBuf, PBBUF_SLOT_FREE),
				prPbBuf->u4SlotAmount);
			MM_RETURN(RET_DMX_OK);
		}

		prSlot = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_READING);
		while (NULL != prSlot) {
			mrRet = PBBUF_RemoveSlot(prPbBuf, prSlot);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT("[PBBUF] %s line %d fail in PBBUF_RemoveSlot(prPbBuf(0x%x),")
					TEXT(" prSlot(0x%x), PBBUF_SLOT_READING), mrRet = 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prSlot, mrRet);
				MM_RETURN(mrRet);
			}
			prSlot->rHeader.eType = PBBUF_SLOT_NORMAL;
			mrRet = PBBUF_AddSlotToTail(prPbBuf, prSlot, PBBUF_SLOT_FREE);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail(prPbBuf(0x%x),")
					TEXT(" prSlot(0x%x), PBBUF_SLOT_FREE), mrRet = 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prSlot, mrRet);
				MM_RETURN(mrRet);
			}
			prSlot = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_READING);
		}

		prSlot = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_SEND);
		while (NULL != prSlot) {
			mrRet = PBBUF_RemoveSlot(prPbBuf, prSlot);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT("[PBBUF] %s line %d fail in PBBUF_RemoveSlot(prPbBuf(0x%x),")
					TEXT(" prSlot(0x%x), PBBUF_SLOT_SEND), mrRet = 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prSlot, mrRet);
				MM_RETURN(mrRet);
			}
			prSlot->rHeader.eType = PBBUF_SLOT_NORMAL;
			mrRet = PBBUF_AddSlotToTail(prPbBuf, prSlot, PBBUF_SLOT_FREE);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail(prPbBuf(0x%x),")
					TEXT(" prSlot(0x%x), PBBUF_SLOT_FREE), mrRet = 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prSlot, mrRet);
				MM_RETURN(mrRet);
			}
			prSlot = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_SEND);
		}

		prSlot = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_ALLOCATED);
		while (NULL != prSlot) {
			mrRet = PBBUF_RemoveSlot(prPbBuf, prSlot);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT("[PBBUF] %s line %d fail in PBBUF_RemoveSlot(prPbBuf(0x%x),")
					TEXT(" prSlot(0x%x), PBBUF_SLOT_ALLOCATED), mrRet = 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prSlot, mrRet);
				MM_RETURN(mrRet);
			}
			prSlot->rHeader.eType = PBBUF_SLOT_NORMAL;
			mrRet = PBBUF_AddSlotToTail(prPbBuf, prSlot, PBBUF_SLOT_FREE);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail(prPbBuf(0x%x),")
					TEXT(" prSlot(0x%x), PBBUF_SLOT_FREE), mrRet = 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prSlot, mrRet);
				MM_RETURN(mrRet);
			}
			prSlot = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_ALLOCATED);
		}

		PBBUFClrNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_CANCEL_ALLOCATE_BUFFER);

		prPbBuf->u8LastSrcOfst = DMX_INVALID_UINT64;

#if DMX_PRINT_PBBUF_DEBUG_LOG
		DMXLOG_TRACE(
			TEXT("[GAU] %s -- Clean PBBUF_COND_CANCEL_ALLOCATE_BUFFER!!!\r\n"),
			DMX_FUNC_NAME);
#endif			/* DMX_PRINT_PBBUF_DEBUG_LOG*/

		PBBUFClrNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_BUFFER_CLEANED);

		DMXLOG_TRACE(
			TEXT("[GAU] %s -- All Slots has been Cleaned, FreeSlotCnt: %d!!!\r\n"),
			DMX_FUNC_NAME, PBBUF_GetListCount(prPbBuf, PBBUF_SLOT_FREE));
	}

	MM_RETURN(mrRet);
}
