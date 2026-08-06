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
 * @file dmx_pbbuf_self.c
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
#ifdef __linux__
#include <linux/mm.h>
#include <linux/errno.h>
#include "x_debug.h"
#include "windows.h"
#include "winutil.h"
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_event.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/ose_mem.h>
#include <media/atc/ioctl_dmx.h>
/* #include <media/atc/mm_debug.h> */
#include "mmisc.h"
#else
#include "dmx_define.h"
#include "dmx_event.h"
#include "dmx_splitter.h"
#include "ose_mem.h"
#include "ioctl_dmx.h"
#include "mm_debug.h"
#endif				/* __linux__*/

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_spt.h"
#include "dmx_dump.h"
#include "dmx_pbbuf.h"
#include "dmx_pbbuf_if.h"
#include "dmx_psr_pbbuf.h"
#include "dmx_psr_cc.h"

#ifndef __linux__
#pragma warning(disable : 4127)	/*disable warning C4127: conditional expression is constant*/
#endif				/* __linux__*/

EXTERN u32	g_u4PbBufFlag;

MRESULT PBBUF_SelfBuf_InitBuffer(void *pvPbBuf, u32 u4BufTotalSz,
	u32 u4SlotSz, u32 u4HdrParamSz, u8 u1PbbufType)
{
	PBBUF *prPbBuf = (PBBUF *) pvPbBuf;
	SLOT *prCurr = NULL, *prNext = NULL;
	u8 *pcBufferSa = NULL;
	u32 u4Idx = 0;

	DMXLOG_DEBUG(TEXT("[PBBUF] %s enter!\r\n"), DMX_FUNC_NAME);

	if ((0 == u4SlotSz) || (u4BufTotalSz < 2 * u4SlotSz)) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail for invalid args u4BufTotalSz")
			TEXT("0x%x, u4SlotSz: 0x%x !!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4BufTotalSz, u4SlotSz);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!(prPbBuf)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPbBuf->u1SptPbuffType = u1PbbufType;
	prPbBuf->pvExtInfo = NULL;

//#if !DMX_NEW_PBBUF_MECHANISM
	if (!g_u4PbBufFlag) {
  	if (DMX_MAX_PBBUF_INST_CNT <= prPbBuf->u4CompID) {
  		DMXLOG_ERROR(
  			TEXT("[PBBUF] %s line %d fail for invalid Pbbuf's CompID(%d)!\r\n"),
  			DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf->u4CompID);
  		MM_RETURN(RET_DMX_PARAM_WRONG);
  	}

  	DMXLOG_TRACE(TEXT("[PBBUF] %s ---------------->SPT_EVT_NAME_PBBUF_%d!\r\n"),
  			DMX_FUNC_NAME, prPbBuf->u4CompID);
  	switch (prPbBuf->u4CompID) {
  	case 0:
  		prPbBuf->hTxData = x_event_open(EVENT_ALL_ACCESS, FALSE, SPT_EVT_NAME_PBBUF_0);
  		break;
  	case 1:
  		prPbBuf->hTxData = x_event_open(EVENT_ALL_ACCESS, FALSE, SPT_EVT_NAME_PBBUF_1);
  		break;
  	case 2:
  		prPbBuf->hTxData = x_event_open(EVENT_ALL_ACCESS, FALSE, SPT_EVT_NAME_PBBUF_2);
  		break;
  	default:
  		break;
  	}
  	DMXLOG_TRACE(TEXT("[PBBUF] %s ---------------->SPT_EVT_NAME_PBBUF_%d success!\r\n"),
  			DMX_FUNC_NAME, prPbBuf->u4CompID);

  	if (NULL == prPbBuf->hTxData) {
  		DMXLOG_ERROR(
  			TEXT("[PBBUF] %s line %d fail in x_event_open, Pbbuf's CompID(%d)!\r\n"),
  			DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf->u4CompID);
  		DMX_ASSERT(FALSE);
  		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
  	}
	}
//#endif				/* !DMX_NEW_PBBUF_MECHANISM}*/

	DMX_NewHwAlignMemory((u4BufTotalSz * sizeof(u8)), PBBUF_ALIGN, pcBufferSa);

	DMXLOG_DEBUG(
		TEXT
		("[PBBUF] %s -- Pbbufcmpid(%d) alloc pbbuf size: 0x%x, pcBufferSa: 0x%x \r\n"),
		DMX_FUNC_NAME, prPbBuf->u4CompID, u4BufTotalSz, pcBufferSa);

	if (NULL == pcBufferSa) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail in alloc pbbuf (no mem)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_NO_MEM);
	}

	prPbBuf->pcMMRsvBufBase = (u8 *)OSE_GetMMReservedMemStartAddr();
	if (NULL == prPbBuf->pcMMRsvBufBase) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail in OSE_GetMMReservedMemStartAddr\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}

	prPbBuf->pcBufSa = pcBufferSa;
	prPbBuf->pcUsrBufSa = NULL;
	prPbBuf->hUsrCaller = NULL;
	prPbBuf->u4TotalSz = u4BufTotalSz;
	prPbBuf->u4SlotSz = u4SlotSz;
	prPbBuf->u4SlotAmount = (prPbBuf->u4TotalSz / prPbBuf->u4SlotSz);

	DMXLOG_DEBUG(TEXT("[PBBUF] %s -- prPbBuf->u4SlotAmount:0x%x \r\n"),
		DMX_FUNC_NAME, prPbBuf->u4SlotAmount);

	if (0 < u4HdrParamSz) {
		DMX_NewMemory((prPbBuf->u4SlotAmount * u4HdrParamSz), prPbBuf->pvSLotHdrs);

		if (NULL == prPbBuf->pvSLotHdrs) {
			DMXLOG_ERROR(
				TEXT("[PBBUF] %s line %d fail in alloc slots Headers (no mem)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			PBBUF_SelfBuf_DeInitBuffer(prPbBuf);
			MM_RETURN(RET_DMX_NO_MEM);
		}

		dmx_memset(prPbBuf->pvSLotHdrs, 0, prPbBuf->u4SlotAmount * u4HdrParamSz);
	}

	DMX_NewMemory((prPbBuf->u4SlotAmount * sizeof(SLOT)), prPbBuf->prSLOT);

	if (NULL == prPbBuf->prSLOT) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail in alloc slots tables (no mem)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		PBBUF_SelfBuf_DeInitBuffer(prPbBuf);
		MM_RETURN(RET_DMX_NO_MEM);
	}

	dmx_memset(prPbBuf->prSLOT, 0, prPbBuf->u4SlotAmount * sizeof(SLOT));

	/* Create double linked list for SENT/ALLOCATED/READING slots.*/
	/* Now these linked list are empty.*/
	dmx_memset(prPbBuf->arSlotLists, 0, sizeof(PBBUF_SLOT_LIST_INFO_T) * PBBUF_SLOT_TYPE_CNT);

	/* 1. Initialize pcBuffer, u4BufferSize, & u4BufferHandle info.*/
	/* 2. Create double linked list for FREE slots.*/
	prPbBuf->arSlotLists[PBBUF_SLOT_FREE].prHeadSlot = prPbBuf->prSLOT;

	for (u4Idx = 0; u4Idx < prPbBuf->u4SlotAmount; u4Idx++) {
		prCurr = prPbBuf->prSLOT + u4Idx;
		/* For the last Slot, prNext is pointed to &prPbBuf->rFREE*/
		/* Initialize the three fixed information of the (SEND_BUFFER)Slot.*/
		prCurr->pcBuffer = prPbBuf->pcBufSa + (u4Idx * prPbBuf->u4SlotSz);
		prCurr->pcUsrBuffer = NULL;
		prCurr->u4BufferSize = prPbBuf->u4SlotSz;

		prCurr->rHeader.eType = PBBUF_SLOT_NORMAL;
		prCurr->rHeader.u4ParamSz = u4HdrParamSz;
		prCurr->rHeader.pvParam = NULL;

		if (0 < u4HdrParamSz) {
			prCurr->rHeader.pvParam =
				(void *) ((u8 *) (prPbBuf->pvSLotHdrs) + (u4Idx * u4HdrParamSz));
		}
		/* Create double linked list for Free Slots.*/
		if (0 == u4Idx)
			prCurr->prPrevSlot = NULL;


		if (u4Idx == (prPbBuf->u4SlotAmount - 1)) {
			prNext = NULL;
			prPbBuf->arSlotLists[PBBUF_SLOT_FREE].prTailSlot = prCurr;
		} else {
			prNext = prPbBuf->prSLOT + (u4Idx + 1);
			prNext->prPrevSlot = prCurr;
		}

		prCurr->eType = PBBUF_SLOT_FREE;
		prCurr->prNextSlot = prNext;
	}

	prPbBuf->arSlotLists[PBBUF_SLOT_FREE].u4SlotCnt = prPbBuf->u4SlotAmount;

	MM_RETURN(RET_DMX_OK);
}

void PBBUF_SelfBuf_DeInitBuffer(void *pvPbBuf)
{
	PBBUF *prPbBuf = (PBBUF *) pvPbBuf;

	if (!(prPbBuf)) {
		DMX_ASSERT(FALSE);
		return;
	}


	if (NULL != prPbBuf->pcBufSa) {
#ifndef __linux__
		if (NULL != prPbBuf->pcUsrBufSa) {
			DMXLOG_DEBUG(
				TEXT
				("[PBBUF] %s VirtualFreeEx, Caller: 0x%x, Sa: 0x%x, Sz: 0x%x, VirSa: 0x%x\r\n"),
				DMX_FUNC_NAME, prPbBuf->hUsrCaller, prPbBuf->pcBufSa,
				prPbBuf->u4TotalSz, prPbBuf->pcUsrBufSa);
			if (!VirtualFreeEx
				(prPbBuf->hUsrCaller, (void *) (prPbBuf->pcUsrBufSa), 0, MEM_RELEASE)) {
				DMXLOG_ERROR(
					TEXT("[PBBUF] %s line %d fail in VirtualFreeEx, Caller: 0x%x,")
					TEXT(" Sa: 0x%x, Sz: 0x%x, VirSa: 0x%x, Err: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf->hUsrCaller, prPbBuf->pcBufSa,
					prPbBuf->u4TotalSz, prPbBuf->pcUsrBufSa,
					DMX_GET_LASTERR);
			}
		}
#endif				/* __linux__*/

		if (prPbBuf->u4CompID > DMX_MAX_PBBUF_INST_CNT) {
			DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail for Pbbuf Component")
				TEXT(" ID(%d) > Max(%d) 0x%x, u4SlotSz: 0x%x !!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf->u4CompID,
				DMX_MAX_PBBUF_INST_CNT);
			return;
		}

		DMXLOG_DEBUG(
			TEXT("[PBBUF] %s -- PbbufCompId(%d) free pbbuff pcBufSa: 0x%x \r\n"),
			DMX_FUNC_NAME, prPbBuf->u4CompID, prPbBuf->pcBufSa);

		DMX_FreeHwMemory(prPbBuf->pcBufSa);

		prPbBuf->hUsrCaller = NULL;
		prPbBuf->pcUsrBufSa = NULL;
		prPbBuf->pcBufSa = NULL;
	}

	if (NULL != prPbBuf->prSLOT) {
		DMX_FreeMemory(prPbBuf->prSLOT);
		prPbBuf->prSLOT = NULL;
	}

	if (NULL != prPbBuf->pvSLotHdrs) {
		DMX_FreeMemory(prPbBuf->pvSLotHdrs);
		prPbBuf->pvSLotHdrs = NULL;
	}

	dmx_memset(prPbBuf->arSlotLists, 0, sizeof(PBBUF_SLOT_LIST_INFO_T) * PBBUF_SLOT_TYPE_CNT);

//#if !DMX_NEW_PBBUF_MECHANISM
	if (!g_u4PbBufFlag) {
  	if (NULL != prPbBuf->hTxData) {
  		DMXLOG_DEBUG(TEXT("[PBBUF] %s ---------------Close TxData Event!\r\n"),
  				DMX_FUNC_NAME);
  		x_event_destroy(prPbBuf->hTxData);

  		prPbBuf->hTxData = NULL;
  	}
	}
//#endif				/* !DMX_NEW_PBBUF_MECHANISM*/


	prPbBuf->pcMMRsvBufBase = NULL;
	prPbBuf->hUsrCaller = NULL;
	prPbBuf->pcUsrBufSa = NULL;
	prPbBuf->pcBufSa = NULL;
	prPbBuf->u4TotalSz = 0;
	prPbBuf->u4SlotSz = 0;
	prPbBuf->u4SlotAmount = 0;
	prPbBuf->u4SlotAmount = 0;
	prPbBuf->u4MwNfyMask = 0;
	prPbBuf->u4DrvNfyMask = 0;
	prPbBuf->u1SptPbuffType = SPT_PBUFF_UNKNOWN;
}

MRESULT PBBUF_SelfBuf_GetAllocSlot(void *pvPbBuf, SEND_BUFFER *prSdBuf)
{
	PBBUF *prPbBuf = (PBBUF *) pvPbBuf;
	SLOT *prSlot = NULL;
	MRESULT mrRet = RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PBBUF] -- %s enter\r\n"), DMX_FUNC_NAME);

	if (!(prPbBuf)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (PBBUFChkNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_CANCEL_ALLOCATE_BUFFER)) {
		DMXLOG_DEBUG(
			TEXT("[GAU] %s -- Clean PBBUF_COND_CANCEL_ALLOCATE_BUFFER!!!\r\n"),
			DMX_FUNC_NAME);
		PBBUFClrNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_CANCEL_ALLOCATE_BUFFER);
		MM_RETURN(RET_DMX_PBBUF_BUSY);
	}

	if (NULL == prPbBuf->pcUsrBufSa) {
		u32 i = 0;

		/*get the relative addr*/
		prPbBuf->pcUsrBufSa = (u8 *) (prPbBuf->pcBufSa - prPbBuf->pcMMRsvBufBase);
		mrRet = E_DMX_OK;
		for (i = 0; i < prPbBuf->u4SlotAmount; i++) {
			prSlot = prPbBuf->prSLOT + i;
			prSlot->pcUsrBuffer = prPbBuf->pcUsrBufSa + i * prPbBuf->u4SlotSz;
		}
	}
	/* Get a slot from FREE linked list.*/
	prSlot = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_FREE);

	if (NULL != prSlot) {
		/* Remove the slot from FREE linked list.*/
		mrRet = PBBUF_RemoveSlot(prPbBuf, prSlot);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT
				("[PBBUF] %s line %d fail in PBBUF_RemoveSlot, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			MM_RETURN(mrRet);
		}
		/* Add the slot to the Allocated Linked list.*/
		mrRet = PBBUF_AddSlotToTail(prPbBuf, prSlot, PBBUF_SLOT_ALLOCATED);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT
				("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			MM_RETURN(mrRet);
		}

		prSdBuf->pcBuffer = prSlot->pcUsrBuffer;
		prSdBuf->u4BufferSize = prSlot->u4BufferSize;
		prSdBuf->pvBuffer = (void *)prSlot;

		DMXLOG_DEBUG(TEXT("[PBBUF] -- %s line %d, Success\r\n"), DMX_FUNC_NAME,
			DMX_LINE_NO);

		prSlot->rHeader.eType = PBBUF_SLOT_NORMAL;
		prSdBuf->rHeader.eType = PBBUF_SLOT_NORMAL;

		MM_RETURN(RET_DMX_OK);
	}
	{
//#if DMX_NEW_PBBUF_MECHANISM
		if (g_u4PbBufFlag) {
  		EV_GRP_EVENT_T u4WaitEvtsRes = 0;
  		s32 i4Ret = 0;

  		/* No free slot available.*/
  		PBBUFSetNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_READY_TO_ALLOCATE_BUFFER);

#if DMX_PRINT_PBBUF_DEBUG_LOG
  		DMXLOG_TRACE(
  			TEXT
  			("[PBBUF] %s wait pbbuf EV_FREE_SLOT_IN | EV_STOP_ALLOC events\r\n"),
  			DMX_FUNC_NAME);
#endif			/* DMX_PRINT_PBBUF_DEBUG_LOG*/
		PBBUF_EXIT(prPbBuf);
  		i4Ret = x_ev_group_wait_event_timeout(prPbBuf->hEvtGroup,
  			(DMX_PBBUF_EV_FREE_SLOT_IN | DMX_PBBUF_EV_STOP_ALLOC),
  			&u4WaitEvtsRes, X_EV_OP_OR_CONSUME, DMX_PBBUF_WAIT_MAXTIME);

		if (OSR_TIMEOUT == i4Ret) {
			DMXLOG_DEBUG(
  				TEXT("[PBBUF] %s line %d wait pbbuf EV_FREE_SLOT_IN")
  				TEXT(" | EV_STOP_ALLOC events timeout.\r\n"),
  				DMX_FUNC_NAME, DMX_LINE_NO);
			PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);
  			PBBUFClrNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_READY_TO_ALLOCATE_BUFFER);
  			MM_RETURN(RET_DMX_TIMEOUT);
		} else if (OSR_OK != i4Ret) {
  			DMXLOG_ERROR(
  				TEXT("[PBBUF] %s line %d fail in wait pbbuf EV_FREE_SLOT_IN")
  				TEXT(" | EV_STOP_ALLOC events\r\n"),
  				DMX_FUNC_NAME, DMX_LINE_NO);
			PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);
  			PBBUFClrNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_READY_TO_ALLOCATE_BUFFER);
  			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
  		}
		
		PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);
  		PBBUFClrNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_READY_TO_ALLOCATE_BUFFER);

  		if (DMX_PBBUF_EV_FREE_SLOT_IN & u4WaitEvtsRes) {
#if DMX_PRINT_PBBUF_DEBUG_LOG
  			DMXLOG_TRACE(
  				TEXT("[PBBUF] %s line %d Get pbbuf EV_FREE_SLOT_IN event\r\n"),
  				DMX_FUNC_NAME, DMX_LINE_NO);
#endif				/* DMX_PRINT_PBBUF_DEBUG_LOG*/
  			/* Get a slot from FREE linked list.*/
  			prSlot = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_FREE);
  			if (NULL == prSlot) {
  				DMXLOG_ERROR(
  					TEXT
  					("[PBBUF] %s line %d fail in PBBUFGetSlotFromHead get free slot\r\n"),
  					DMX_FUNC_NAME, DMX_LINE_NO);

  				MM_RETURN(RET_DMX_UNEXPECT);
  			}
  			/* Remove the slot from FREE linked list.*/
  			mrRet = PBBUF_RemoveSlot(prPbBuf, prSlot);
  			if (DMX_FAILED(mrRet)) {
  				DMXLOG_ERROR(
  					TEXT
  					("[PBBUF] %s line %d fail in PBBUF_RemoveSlot, mrRet: 0x%x\r\n"),
  					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
  				MM_RETURN(mrRet);
  			}
  			/* Add the slot to the Allocated Linked list.*/
  			mrRet = PBBUF_AddSlotToTail(prPbBuf, prSlot, PBBUF_SLOT_ALLOCATED);
  			if (DMX_FAILED(mrRet)) {
  				DMXLOG_ERROR(
  					TEXT
  					("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail, mrRet: 0x%x\r\n"),
  					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
  				MM_RETURN(mrRet);
  			}
  			prSdBuf->pcBuffer = prSlot->pcUsrBuffer;
  			prSdBuf->u4BufferSize = prSlot->u4BufferSize;
  			prSdBuf->pvBuffer = (void *)prSlot;
  			DMXLOG_DEBUG(TEXT("[PBBUF] -- %s , Success\r\n"),
  				DMX_FUNC_NAME);
  		} else if (DMX_PBBUF_EV_STOP_ALLOC & u4WaitEvtsRes) {
#if DMX_PRINT_PBBUF_DEBUG_LOG
  			DMXLOG_TRACE(
  				TEXT("[PBBUF] %s line %d Get pbbuf EV_STOP_ALLOC event\r\n"),
  				DMX_FUNC_NAME, DMX_LINE_NO);
#endif				/*DMX_PRINT_PBBUF_DEBUG_LOG*/
  			MM_RETURN(RET_DMX_PBBUF_BUSY);
  		} else {
  			DMXLOG_ERROR(
  				TEXT("[PBBUF] %s line %d fail in wait pbbuf EV_FREE_SLOT_IN")
  				TEXT(" | EV_STOP_ALLOC events\r\n"),
  				DMX_FUNC_NAME, DMX_LINE_NO);
  			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
  		}
		} else {
//#else
  		/*No free slot available.*/
  		DMXLOG_DEBUG(TEXT("[PBBUF] %s line %d -- NO FREE SLOT Available\r\n"),
  			DMX_FUNC_NAME, DMX_LINE_NO);
  		PBBUFSetNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_READY_TO_ALLOCATE_BUFFER);
  		mrRet = RET_DMX_PBBUF_BUSY;
		}
//#endif				/* DMX_NEW_PBBUF_MECHANISM*/
	}

	MM_RETURN(mrRet);
}

MRESULT PBBUF_SelfBuf_CancelAllocSlot(void *pvPbBuf)
{
	PBBUF *prPbBuf = (PBBUF *) pvPbBuf;

	if (!(prPbBuf)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

#if DMX_PRINT_PBBUF_DEBUG_LOG
	DMXLOG_TRACE(TEXT("[GAU] %s -- u4MwNfyMask: 0x%x!!!\r\n"),
		DMX_FUNC_NAME, prPbBuf->u4MwNfyMask);
#endif		/* DMX_PRINT_PBBUF_DEBUG_LOG*/

	if (PBBUFChkNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_READY_TO_ALLOCATE_BUFFER)) {
//#if DMX_NEW_PBBUF_MECHANISM
		if (g_u4PbBufFlag) {
  		s32 i4Ret = OSR_OK;

  		PBBUFClrNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_READY_TO_ALLOCATE_BUFFER);
  		if (NULL_HANDLE == prPbBuf->hEvtGroup) {
  			DMXLOG_ERROR(
  				TEXT
  				("[PBBUF] %s line %d fail for prPbBuf(0x%x)'s hEvtGroup hasn't been create!\r\n"),
  				DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);
  			MM_RETURN(RET_DMX_NO_INIT);
  		}
#if DMX_PRINT_PBBUF_DEBUG_LOG
  		DMXLOG_TRACE(TEXT("[GAU] %s Set Event EV_STOP_ALLOC!!!\r\n"),
  			DMX_FUNC_NAME, prPbBuf);
#endif				/* DMX_PRINT_PBBUF_DEBUG_LOG*/

  		i4Ret = x_ev_group_set_event(prPbBuf->hEvtGroup,
  			DMX_PBBUF_EV_STOP_ALLOC, X_EV_OP_OR);
  		if (OSR_OK != i4Ret) {
  			DMXLOG_ERROR(
  				TEXT
  				("[GAU] %s line %d failed in prPbBuf(0x%x) Set Event EV_STOP_ALLOC!!!\r\n"),
  				DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);
  			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
  		}
		} else {
  //#else				/* MX_NEW_PBBUF_MECHANISM*/
  		PBBUFClrNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_READY_TO_ALLOCATE_BUFFER);
		}
  //#endif				/* MX_NEW_PBBUF_MECHANISM*/
	} else {
		PBBUFSetNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_CANCEL_ALLOCATE_BUFFER);
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT PBBUF_SelfBuf_SendDataSlot(void *pvPbBuf, SEND_BUFFER *prSdBuf, bool *pfgExitSent)
{
	PBBUF *prPbBuf = (PBBUF *)pvPbBuf;
	SLOT *prSlot = NULL;
	MRESULT mrRet = RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PBBUF] -- %s enter\r\n"), DMX_FUNC_NAME);

	if ((NULL == prSdBuf) || (NULL == prSdBuf->pvBuffer)) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail for invalid args:")
			TEXT("(pvPbBuf: 0x%p, prSdBuf: 0x%p)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvPbBuf, prSdBuf);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!(prPbBuf)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prSlot = PBBUF_GetSlotByHandle(prPbBuf, prSdBuf->pvBuffer);

	if (NULL == prSlot) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail for Can't find the slot whose handle")
			TEXT(" == 0x%x (pvPbBuf: 0x%x, prPbBuf: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSdBuf->pvBuffer, pvPbBuf, prPbBuf);
		MM_RETURN(RET_DMX_NOT_FOUND);
	}

	if ((PBBUF_SLOT_END == prSdBuf->rHeader.eType) && (DMX_IS_NORMAL_PLAY(prPbBuf->pvSptHdl))) {
		DMXLOG_TRACE(
			TEXT
			("[PBBUF] %s line %d -- End Slot is sent while normal play, so exit\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);

		/* Remove from allocated linked list.*/
		mrRet = PBBUF_RemoveSlot(prPbBuf, prSlot);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT
				("[PBBUF] %s line %d fail in PBBUF_RemoveSlot, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			MM_RETURN(mrRet);
		}

		prSlot->rHeader.eType = PBBUF_SLOT_NORMAL;

		/* Add the allocated slot to FREE linked list.*/
		mrRet = PBBUF_AddSlotToTail(prPbBuf, prSlot, PBBUF_SLOT_FREE);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT
				("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);

			MM_RETURN(mrRet);
		}

		mrRet = PBBUF_ReadyToGetAllocSlot(prPbBuf);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT
				("[PBBUF] %s line %d fail in PBBUF_ReadyToGetAllocSlot, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			MM_RETURN(mrRet);
		}

		MM_RETURN(RET_DMX_OK);
	}

	if (PBBUF_SLOT_ALLOCATED != prSlot->eType) {
		/* the slot perhaps has been added to the other link list, such as free link list.*/
		if (NULL != pfgExitSent) {
			*pfgExitSent = TRUE;
			/*DMXLOG_TRACE(TEXT("[PBBUF] -- %s line %d Dump Pbbuf Info*/
			/* before Set fgExitSend to be TRUE\r\n"), DMX_FUNC_NAME, DMX_LINE_NO);*/
			/*PBBUF_DumpInfoEx(pvSptHdl, FALSE);*/
			DMXLOG_DEBUG(TEXT("[PBBUF] -- %s: fgExitSent = TRUE\r\n"),
				DMX_FUNC_NAME);
		}
	} else {
		if (PBBUF_SLOT_NORMAL != prSdBuf->rHeader.eType) {
			prPbBuf->u8LastSrcOfst = DMX_INVALID_UINT64;
			DMXLOG_DEBUG(
				TEXT
				("[PBBUF] %s -- Find Uncon, LastSrcOfst: %I64d, SdBuf's SrcOfst: %I64d\r\n"),
				DMX_FUNC_NAME, prPbBuf->u8LastSrcOfst, prSdBuf->u8SrcOffset);
		}

		if (DMX_INVALID_UINT64 != prPbBuf->u8LastSrcOfst) {
			DMXLOG_DEBUG(
				TEXT
				("[PBBUF] %s -- LastSrcOfst: %I64d, SdBuf's SrcOfst: %I64d, DataSize: %d\r\n"),
				DMX_FUNC_NAME, prPbBuf->u8LastSrcOfst, prSdBuf->u8SrcOffset,
				prSdBuf->u4DataSize);
			if (prPbBuf->u8LastSrcOfst == prSdBuf->u8SrcOffset) {
				prPbBuf->u8LastSrcOfst = prSdBuf->u8SrcOffset + prSdBuf->u4DataSize;

				/* Remove from Allocated linked list.*/
				mrRet = PBBUF_RemoveSlot(prPbBuf, prSlot);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(
						TEXT("[PBBUF] %s line %d fail in PBBUF_RemoveSlot")
						TEXT(", mrRet: 0x%x\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
					MM_RETURN(mrRet);
				}

				prSlot->rHeader.eType = PBBUF_SLOT_NORMAL;

				/* Add a sent slot to FREE linked list.*/
				mrRet = PBBUF_AddSlotToTail(prPbBuf, prSlot, PBBUF_SLOT_FREE);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(
						TEXT("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail,")
						TEXT(" mrRet: 0x%x\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
					MM_RETURN(mrRet);
				}

				if (NULL != pfgExitSent)
					*pfgExitSent = FALSE;


				DMXLOG_DEBUG(
					TEXT("[PBBUF] %s -- u4MwNfyMask: 0x%x\r\n"),
					DMX_FUNC_NAME, prPbBuf->u4MwNfyMask);

				mrRet = PBBUF_ReadyToGetAllocSlot(prPbBuf);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(
						TEXT("[PBBUF] %s line %d fail in PBBUF_ReadyToGetAllocSlot")
						TEXT(", mrRet: 0x%x\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
					MM_RETURN(mrRet);
				}

				MM_RETURN(RET_DMX_OK);
			}
		}
		/* Copy prSendBuffer info to the target slot.*/
		prSlot->u4DataOffset = prSdBuf->u4DataOffset;
		prSlot->u4DataSize = prSdBuf->u4DataSize;
		prSlot->u4PlayOffset = prSdBuf->u4PlayOffset;
		prSlot->u4PlaySize = prSdBuf->u4PlaySize;
		prSlot->u8SrcOffset = prSdBuf->u8SrcOffset;

		prSlot->rHeader.eType = PBBUF_SLOT_NORMAL;

		if (PBBUF_SLOT_NORMAL != prSdBuf->rHeader.eType) {
			prSlot->rHeader.eType = prSdBuf->rHeader.eType;

			if (prSlot->rHeader.u4ParamSz != prSdBuf->rHeader.u4ParamSz) {
				DMXLOG_ERROR(
					TEXT("[PBBUF] %s line %d fail for SendBuf's rHeader.u4ParamSz")
					TEXT("(%d) != SLot's rHeader.u4ParamSz(%d)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSdBuf->rHeader.u4ParamSz,
					prSlot->rHeader.u4ParamSz);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (NULL != prSdBuf->rHeader.pvParam) {
#ifndef __linux__
				void *pvUnMarshalledMem = prSdBuf->rHeader.pvParam;
				void *pvDstMarshallMem = NULL;
				HRESULT hr = S_OK;

				hr = CeOpenCallerBuffer(&pvDstMarshallMem,
					pvUnMarshalledMem,
					prSdBuf->rHeader.u4ParamSz, ARG_I_PTR,
					FALSE);

				if (FAILED(hr)) {
					DMXLOG_ERROR(
						TEXT("[PBBUF] %s line %d failed in CeOpenCallerBuffer")
						TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d)")
						TEXT(", Err: 0x%x\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						prSdBuf->rHeader.pvParam, pvUnMarshalledMem,
						prSdBuf->rHeader.u4ParamSz, DMX_GET_LASTERR);
					MM_RETURN(RET_DMX_OS_OPERA_FAIL);
				}

				dmx_memcpy(prSlot->rHeader.pvParam, pvDstMarshallMem,
					   prSdBuf->rHeader.u4ParamSz);

				hr = CeCloseCallerBuffer(pvDstMarshallMem,
					pvUnMarshalledMem,
					prSdBuf->rHeader.u4ParamSz, ARG_I_PTR);

				if (FAILED(hr)) {
					DMXLOG_ERROR(
						TEXT("[PBBUF] %s line %d failed in CeCloseCallerBuffer")
						TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), ")
						TEXT("Err: 0x%x\r\n"), DMX_FUNC_NAME, DMX_LINE_NO,
						prSlot->rHeader.pvParam, pvUnMarshalledMem,
						prSdBuf->rHeader.u4ParamSz, DMX_GET_LASTERR);
					MM_RETURN(RET_DMX_OS_OPERA_FAIL);
				}
#else
				if (0 !=
					mm_copy_from_user(prSlot->rHeader.pvParam,
							  prSdBuf->rHeader.pvParam,
							  prSdBuf->rHeader.u4ParamSz)) {
					DMXLOG_ERROR(
						TEXT("[PBBUF] %s line %d failed in copy_from_user")
						TEXT("(Slot->rHdr.pvParam: 0x%x, SdBuf's rHdr.pvParam")
						TEXT(": 0x%x, Sz: %d), Err: 0x%x\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						prSlot->rHeader.pvParam,
						prSdBuf->rHeader.pvParam,
						prSdBuf->rHeader.u4ParamSz, DMX_GET_LASTERR);
					MM_RETURN(RET_DMX_OS_OPERA_FAIL);
				}
#endif				/* __linux__*/
			}
		}
		/* Remove from ALLOCATED linked list.*/
		mrRet = PBBUF_RemoveSlot(prPbBuf, prSlot);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT
				("[PBBUF] %s line %d fail in PBBUF_RemoveSlot, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			MM_RETURN(mrRet);
		}
		/* Add a sent slot to SENT linked list.*/
		mrRet = PBBUF_AddSlotToTail(prPbBuf, prSlot, PBBUF_SLOT_SEND);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT
				("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			MM_RETURN(mrRet);
		}

		PBBUF_ReadyToGetReadBuffer(prPbBuf, prSlot);
		if (NULL != pfgExitSent)
			*pfgExitSent = FALSE;

	}

	DMXLOG_DEBUG(
		TEXT("[PBBUF] %s line %d exit, Success, SrcOfst: %I64d, DataSz: %d\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prSlot->u8SrcOffset, prSlot->u4DataSize);

	MM_RETURN(RET_DMX_OK);
}

MRESULT PBBUF_SelfBuf_ReleaseAllocSlot(void *pvPbBuf, SEND_BUFFER *prSdBuf)
{
	PBBUF *prPbBuf = (PBBUF *)pvPbBuf;
	SLOT *prSlot = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prSdBuf) || (NULL == prSdBuf->pvBuffer)) {
		DMXLOG_ERROR(
			TEXT
			("[PBBUF] %s line %d fail for invalid args: (pvPbBuf: 0x%p, prSdBuf: 0x%p)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvPbBuf, prSdBuf);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!(prPbBuf)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prSlot = PBBUF_GetSlotByHandle(prPbBuf, prSdBuf->pvBuffer);

	if (NULL == prSlot) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail for Can't find the slot whose handle == 0x%p ")
			TEXT("(pvPbBuf: 0x%p, prPbBuf: 0x%p)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSdBuf->pvBuffer, pvPbBuf, prPbBuf);
		MM_RETURN(RET_DMX_NOT_FOUND);
	}
	/* Remove from allocated linked list.*/
	mrRet = PBBUF_RemoveSlot(prPbBuf, prSlot);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail in PBBUF_RemoveSlot, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}

	prSlot->rHeader.eType = PBBUF_SLOT_NORMAL;

	/* Add the allocated slot to FREE linked list.*/
	mrRet = PBBUF_AddSlotToTail(prPbBuf, prSlot, PBBUF_SLOT_FREE);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}

	mrRet = PBBUF_ReadyToGetAllocSlot(prPbBuf);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT
			("[PBBUF] %s line %d fail in PBBUF_ReadyToGetAllocSlot, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT PBBUF_SelfBuf_CleanAllSlots(void *pvPbBuf)
{
	PBBUF *prPbBuf = (PBBUF *)pvPbBuf;
	SLOT *prCurr = NULL;
	SLOT *prNext = NULL;
	u32 u4Idx = 0;

	if (!(prPbBuf)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

#if 1

	prPbBuf->arSlotLists[PBBUF_SLOT_FREE].prHeadSlot = prPbBuf->prSLOT;

	for (u4Idx = 0; u4Idx < prPbBuf->u4SlotAmount; u4Idx++) {
		prCurr = prPbBuf->prSLOT + u4Idx;

		prCurr->rHeader.eType = PBBUF_SLOT_NORMAL;

		/* Create double linked list for Free Slots.*/
		if (u4Idx == (prPbBuf->u4SlotAmount - 1)) {
			prNext = NULL;
			prPbBuf->arSlotLists[PBBUF_SLOT_FREE].prTailSlot = prCurr;
		} else {
			prNext = prPbBuf->prSLOT + (u4Idx + 1);
			prNext->prPrevSlot = prCurr;
		}

		prCurr->prNextSlot = prNext;
		prCurr->eType = PBBUF_SLOT_FREE;
	}

	if (NULL != prPbBuf->arSlotLists[PBBUF_SLOT_FREE].prHeadSlot)
		prPbBuf->arSlotLists[PBBUF_SLOT_FREE].prHeadSlot->prPrevSlot = NULL;


	prPbBuf->arSlotLists[PBBUF_SLOT_FREE].u4SlotCnt = prPbBuf->u4SlotAmount;

	prPbBuf->u8LastSrcOfst = DMX_INVALID_UINT64;

	dmx_memset(&(prPbBuf->arSlotLists[PBBUF_SLOT_ALLOCATED]), 0,
		   sizeof(PBBUF_SLOT_LIST_INFO_T));
	dmx_memset(&(prPbBuf->arSlotLists[PBBUF_SLOT_READING]), 0, sizeof(PBBUF_SLOT_LIST_INFO_T));
	dmx_memset(&(prPbBuf->arSlotLists[PBBUF_SLOT_SEND]), 0, sizeof(PBBUF_SLOT_LIST_INFO_T));

#if DMX_PRINT_PBBUF_DEBUG_LOG
	DMXLOG_TRACE(TEXT("[GAU] %s -- Clean PBBUF_COND_CANCEL_ALLOCATE_BUFFER!!!\r\n"),
		DMX_FUNC_NAME);
#endif				/* DMX_PRINT_PBBUF_DEBUG_LOG*/

	PBBUFClrNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_CANCEL_ALLOCATE_BUFFER);

#else

	if (0 == PBBUF_GetListCount(prPbBuf, PBBUF_SLOT_READING)) {
		MRESULT mrRet = RET_DMX_OK;

		prCurr = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_SEND);
		while (NULL != prCurr) {
			mrRet = PBBUF_RemoveSlot(prPbBuf, prCurr);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					(TEXT("[PBBUF] %s line %d fail in PBBUF_RemoveSlot(prPbBuf(0x%x)")
					TEXT(", prCurr(0x%x), PBBUF_SLOT_SEND), mrRet = 0x%x\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prCurr, mrRet);
				MM_RETURN(mrRet);
			}
			prCurr->rHeader.eType = PBBUF_SLOT_NORMAL;
			mrRet = PBBUF_AddSlotToTail(prPbBuf, prCurr, PBBUF_SLOT_FREE);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					(TEXT("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail(prPbBuf(0x%x)")
					TEXT(", prCurr(0x%x), PBBUF_SLOT_FREE), mrRet = 0x%x\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prCurr, mrRet);
				MM_RETURN(mrRet);
			}
		}

		prCurr = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_ALLOCATED);
		while (NULL != prCurr) {
			mrRet = PBBUF_RemoveSlot(prPbBuf, prCurr);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					(TEXT("[PBBUF] %s line %d fail in PBBUF_RemoveSlot(prPbBuf(0x%x)")
					TEXT(", prCurr(0x%x), PBBUF_SLOT_ALLOCATED), mrRet = 0x%x\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prCurr, mrRet);
				MM_RETURN(mrRet);
			}
			prCurr->rHeader.eType = PBBUF_SLOT_NORMAL;
			mrRet = PBBUF_AddSlotToTail(prPbBuf, prCurr, PBBUF_SLOT_FREE);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					(TEXT("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail(prPbBuf(0x%x)")
					TEXT(", prCurr(0x%x), PBBUF_SLOT_FREE), mrRet = 0x%x\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prCurr, mrRet);
				MM_RETURN(mrRet);
			}
		}

		prPbBuf->u8LastSrcOfst = DMX_INVALID_UINT64;

#if DMX_PRINT_PBBUF_DEBUG_LOG
		DMXLOG_TRACE(
			TEXT("[GAU] %s -- Clean PBBUF_COND_CANCEL_ALLOCATE_BUFFER!!!\r\n"),
			DMX_FUNC_NAME);
#endif				/* DMX_PRINT_PBBUF_DEBUG_LOG*/

		PBBUFClrNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_CANCEL_ALLOCATE_BUFFER);
	} else {
		PSR_CC_PBBuf_Notify(prPbBuf->ptrDrvOwner, DRV_PBBUF_COND_RELEASE_ALL_SLOTS, 0);
		PBBUFSetNfyMask(&prPbBuf->u4MwNfyMask, PBBUF_COND_BUFFER_CLEANED);
	}
#endif

	MM_RETURN(RET_DMX_OK);
}

MRESULT PBBUF_SelfBuf_GetAvailDataSlot(void *pvPbBuf, DMX_READ_BUFFER *prRdBuf)
{
	PBBUF *prPbBuf = (PBBUF *)pvPbBuf;
	SLOT *prSlot = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (!(prPbBuf)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prSlot = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_SEND);
	if (NULL == prSlot) {
		DMXLOG_DEBUG(TEXT("[PBBUF] +++++++++++ Pbbuf Send List")
		TEXT("Empty (hPsr: 0x%x) +++++++++++ \r\n"), prPbBuf->pvDrvOwner);
		PBBUFSetNfyMask(prPbBuf->u4DrvNfyMask, DRV_PBBUF_COND_READY_TO_GET_READ_BUFFER);

		MM_RETURN(RET_DMX_PBBUF_BUSY);
	}

	prRdBuf->fgFollowedByIbc = FALSE;
	prRdBuf->pcPlayBuffer = prSlot->pcBuffer + prSlot->u4DataOffset;
	prRdBuf->u4DataSize = prSlot->u4DataSize;
	prRdBuf->u4PlayOffset = prSlot->u4PlayOffset;
	prRdBuf->u4PlaySize = prSlot->u4PlaySize;
	prRdBuf->pvSlot = (void *)prSlot;
	prRdBuf->u8SrcOffset = prSlot->u8SrcOffset;
	prRdBuf->u4BufferSize = prSlot->u4BufferSize;

	dmx_memcpy(&(prRdBuf->rHeader), &(prSlot->rHeader), sizeof(PBBUF_SLOT_HEADER_INFO_T));

	/* Remove the slot from SENT linked list.*/
	mrRet = PBBUF_RemoveSlot(prPbBuf, prSlot);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail in PBBUF_RemoveSlot, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}
	/*Add it to READING linked list.*/
	mrRet = PBBUF_AddSlotToTail(prPbBuf, prSlot, PBBUF_SLOT_READING);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT PBBUF_SelfBuf_ReleaseNoUseSlot(void *pvPbBuf, DMX_READ_BUFFER *prRdBuf)
{
	PBBUF *prPbBuf = (PBBUF *)pvPbBuf;
	SLOT *prSlot = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prRdBuf) || (NULL == prRdBuf->pvSlot)) {
		DMXLOG_ERROR(
			TEXT
			("[PBBUF] %s line %d fail for invalid args (pvPbBuf: 0x%p, prRdBuf: 0x%p)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvPbBuf, prRdBuf);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!(prPbBuf)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prSlot = PBBUF_GetSlotByHandle(prPbBuf, prRdBuf->pvSlot);

	if (NULL == prSlot) {
		DMXLOG_ERROR(
			TEXT
			("[PBBUF] %s line %d fail for can't find the reading slot whose handle = ")
			TEXT("0x%p, (pvPbBuf: 0x%p, prPbBuf: 0x%p)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prRdBuf->pvSlot, pvPbBuf, prPbBuf);
		MM_RETURN(RET_DMX_NOT_FOUND);
	}
	/* Remove from READING linked list. Add the slot to FREE linked list.*/
	mrRet = PBBUF_RemoveSlot(prPbBuf, prSlot);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail in PBBUF_RemoveSlot, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}

	prSlot->rHeader.eType = PBBUF_SLOT_NORMAL;

	mrRet = PBBUF_AddSlotToTail(prPbBuf, prSlot, PBBUF_SLOT_FREE);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}

	mrRet = PBBUF_ReadyToGetAllocSlot(prPbBuf);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT
			("[PBBUF] %s line %d fail in PBBUF_ReadyToGetAllocSlot, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);

		MM_RETURN(mrRet);
	}

	mrRet = PBBUF_BufferCleaned(prPbBuf);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail in PBBUF_BufferCleaned, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);

		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT PBBUF_SelfBuf_CancelReadSlot(void *pvPbBuf)
{
	PBBUF *prPbBuf = (PBBUF *)pvPbBuf;

	if (!(prPbBuf)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PBBUFClrNfyMask(prPbBuf->u4DrvNfyMask, DRV_PBBUF_COND_READY_TO_GET_READ_BUFFER);

	MM_RETURN(RET_DMX_OK);
}

MRESULT PBBUF_SelfBuf_RelFrmSlotToUnCon(void *pvPbBuf, DMX_READ_BUFFER *prRdBuf,
					bool *pfgExistUnCon)
{
	PBBUF *prPbBuf = (PBBUF *)pvPbBuf;
	SLOT *prSlot = NULL;
	MRESULT mrRet = RET_DMX_OK;
	u64 u8SrcOfst = DMX_INVALID_UINT64;
	bool fgFindUnCon = FALSE;

	if ((NULL == prRdBuf) || (NULL == prRdBuf->pvSlot)) {
		DMXLOG_ERROR(
			TEXT
			("[PBBUF] %s line %d fail for invalid args (pvPbBuf: 0x%p, prRdBuf: 0x%p)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvPbBuf, prRdBuf);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!(prPbBuf)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}


	prSlot = PBBUF_GetSlotByHandle(prPbBuf, prRdBuf->pvSlot);

	if (NULL == prSlot) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail for can't find the reading slot whose handle")
			TEXT(" = 0x%p, (pvPbBuf: 0x%p, prPbBuf: 0x%p)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prRdBuf->pvSlot, pvPbBuf, prPbBuf);
		MM_RETURN(RET_DMX_NOT_FOUND);
	}

	u8SrcOfst = prSlot->u8SrcOffset + prSlot->u4DataSize;

	/*Remove from READING linked list. Add the slot to FREE linked list.*/
	mrRet = PBBUF_RemoveSlot(prPbBuf, prSlot);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail in PBBUF_RemoveSlot, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}

	prSlot->rHeader.eType = PBBUF_SLOT_NORMAL;

	mrRet = PBBUF_AddSlotToTail(prPbBuf, prSlot, PBBUF_SLOT_FREE);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}

	while (TRUE) {
		prSlot = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_SEND);

		if (NULL != prSlot) {
			if (prSlot->u8SrcOffset != u8SrcOfst) {
				fgFindUnCon = TRUE;
				break;
			}

			u8SrcOfst = prSlot->u8SrcOffset + prSlot->u4DataSize;

			/* Remove the slot from Sent linked list.*/
			mrRet = PBBUF_RemoveSlot(prPbBuf, prSlot);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT
					("[PBBUF] %s line %d fail in PBBUF_RemoveSlot, mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}
			/*Add the slot to the Allocated Linked list.*/
			mrRet = PBBUF_AddSlotToTail(prPbBuf, prSlot, PBBUF_SLOT_FREE);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT
					("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail, mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}
		} else {
			break;
		}
	}

	if (!fgFindUnCon) {
		DMXLOG_DEBUG(TEXT("[PBBUF] %s line %d -- LastSrcOfst: %I64d\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u8SrcOfst);
		prPbBuf->u8LastSrcOfst = u8SrcOfst;
		PBBUFSetNfyMask(prPbBuf->u4DrvNfyMask, DRV_PBBUF_COND_READY_TO_GET_READ_BUFFER);
	} else {
		prPbBuf->u8LastSrcOfst = DMX_INVALID_UINT64;
	}

	if (NULL != pfgExistUnCon)
		*pfgExistUnCon = fgFindUnCon;

	mrRet = PBBUF_ReadyToGetAllocSlot(prPbBuf);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT
			("[PBBUF] %s line %d fail in PBBUF_ReadyToGetAllocSlot, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}
	/*DMXLOG_TRACE(TEXT("[PBBUF] -- %s line %d Dump Pbbuf Info Before*/
	/* Exit\r\n"), DMX_FUNC_NAME, DMX_LINE_NO);*/
	/*PBBUF_DumpInfoEx(pvSptHdl, FALSE);*/
	DMXLOG_DEBUG(TEXT("[PBBUF] -- %s line %d Exit\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	MM_RETURN(RET_DMX_OK);
}
