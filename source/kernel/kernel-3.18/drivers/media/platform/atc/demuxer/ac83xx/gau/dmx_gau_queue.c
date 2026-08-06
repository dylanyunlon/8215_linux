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
 * @file dmx_gau_queue.c
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

#include "x_os.h"
#include "x_typedef.h"
#include "drv_win32_if.h"
#include "windows.h"
#ifdef __linux__
#include <linux/mm.h>
#include <linux/errno.h>
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
#endif /* __linux__ */

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_esm_if.h"
#include "dmx_esm.h"
#include "dmx_sema.h"
#include "dmx_gau_queue.h"

static MRESULT GAU_Q_Reset(GAU_Q_T *prQueue);
static MRESULT GAU_Q_ReleaseElem(GAU_Q_T *prQueue, GAU_Q_ELEM_T *prFreeElem);


void GAU_Q_Init(GAU_Q_T *prQueue, u32 u4Handle)
{
	if (NULL == prQueue)
		return;

	mm_memset(prQueue, 0, sizeof(GAU_Q_T));
	prQueue->eStatus = GAU_Q_STATUS_NONE;
	prQueue->u4Handle = u4Handle;
	prQueue->hSema = NULL;
	prQueue->u4MaxCnt = 0;
	prQueue->prElemTable = NULL;
}

void GAU_Q_UnInit(GAU_Q_T *prQueue)
{
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prQueue)
		return;

	mrRet = GAU_Q_Release(prQueue);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s fail in GAU_Q_Release(u4Handle: %d, mrRet: 0x%x)\r\n"),
			DMX_FUNC_NAME, prQueue->u4Handle, mrRet);
	}

	mm_memset(prQueue, 0, sizeof(GAU_Q_T));
	prQueue->eStatus = GAU_Q_STATUS_NONE;
	prQueue->u4Handle = DMX_INVALID_UINT32;
	prQueue->hSema = NULL;
	prQueue->u4MaxCnt = 0;
	prQueue->prElemTable = NULL;

}

MRESULT GAU_Q_Create(GAU_Q_T *prQueue, u32 u4QueueElemCnt)
{
	u32 u4Idx = 0;
	GAU_Q_ELEM_T *prElem = NULL;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
		TEXT("[GAU] %s enter\r\n"),
		DMX_FUNC_NAME);

	if (NULL == prQueue)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	if (0 == u4QueueElemCnt)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	switch (prQueue->eStatus) {
	case GAU_Q_STATUS_INFREE:
		DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] WARNING -- %s line %d -> u4Handle(0x%x) Perhaps ")
			TEXT("the MW has some error, the ReleaseAU and GetAU isn't ")
			TEXT("in pair.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prQueue->u4Handle);
		break;
	case GAU_Q_STATUS_INUSE:
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] WARNING -- %s line %d fail for GAU_Q Status is INUSE,")
			TEXT(" error, Handle: 0x%x.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prQueue->u4Handle);
		MM_RETURN(RET_DMX_ERR_STATE);
	case GAU_Q_STATUS_IDLE:
		break;
	case GAU_Q_STATUS_NONE:
		break;
	default:
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] WARNING -- %s line %d fail for invalid GAU Q ")
			TEXT("Status(%d), Handle: 0x%x.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prQueue->eStatus, prQueue->u4Handle);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	if (u4QueueElemCnt > prQueue->u4MaxCnt) {
		MRESULT mrRet = RET_DMX_OK;

		mrRet = GAU_Q_Reset(prQueue);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
				TEXT("[GAU] %s fail in GAU_Q_Reset(u4Handle: %d,")
				TEXT(" mrRet: 0x%x)\r\n"),
				DMX_FUNC_NAME, prQueue->u4Handle, mrRet);
			MM_RETURN(mrRet);
		}
		DMX_NewMemory(u4QueueElemCnt * sizeof(GAU_Q_ELEM_T),
			prQueue->prElemTable);
	}

	if (NULL == prQueue->prElemTable)
		MM_RETURN(RET_DMX_NO_MEM);

	dmx_memset(prQueue->prElemTable, 0, u4QueueElemCnt * sizeof(GAU_Q_ELEM_T));

	if (NULL == prQueue->hSema) {
		mrRet = dmx_sema_create(&(prQueue->hSema),
			X_SEMA_TYPE_BINARY, DMX_SEMA_STATE_UNLOCK);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
				TEXT("[GAU] %s fail in dmx_sema_create(u4Idx: %d, ")
				TEXT("i4Ret: 0x%x)\r\n"),
				DMX_FUNC_NAME, u4Idx, mrRet);
			GAU_Q_Reset(prQueue);
			MM_RETURN(mrRet);
		}
	}

	prQueue->u4MaxCnt = u4QueueElemCnt;

	for (u4Idx = 0; u4Idx + 1 < u4QueueElemCnt; u4Idx++) {
		prElem = &(prQueue->prElemTable[u4Idx]);
		prElem->fgInUsing = FALSE;
		prElem->u4AuIdx = ESM_INVALID_INDEX;
		prElem->prNext = &(prQueue->prElemTable[u4Idx + 1]);
		prElem->prNext->prPrev = prElem;
	}

	prQueue->prElemTable[u4QueueElemCnt - 1].fgInUsing = FALSE;
	prQueue->prElemTable[u4QueueElemCnt - 1].u4AuIdx = ESM_INVALID_INDEX;
	prQueue->prElemTable[u4QueueElemCnt - 1].prNext = NULL;

	prQueue->prElemTable[0].prPrev = NULL;

	prQueue->rFreeList.prHead = &(prQueue->prElemTable[0]);
	prQueue->rFreeList.prTail = &(prQueue->prElemTable[u4QueueElemCnt - 1]);
	prQueue->rFreeList.u4Cnt  = u4QueueElemCnt;

	prQueue->rInUseList.prHead = NULL;
	prQueue->rInUseList.prTail = NULL;
	prQueue->rInUseList.u4Cnt  = 0;

	prQueue->eStatus = GAU_Q_STATUS_IDLE;

	DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
		TEXT("[GAU] %s success(u4Handle: %d)\r\n"),
		DMX_FUNC_NAME, prQueue->u4Handle);

	MM_RETURN(RET_DMX_OK);
}

static MRESULT GAU_Q_Reset(GAU_Q_T *prQueue)
{
	if (NULL == prQueue)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	switch (prQueue->eStatus) {
	case GAU_Q_STATUS_NONE:
		DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s success(u4Handle: %d), Status is NONE\r\n"),
			DMX_FUNC_NAME, prQueue->u4Handle);
		MM_RETURN(RET_DMX_OK);
	case GAU_Q_STATUS_IDLE:
	case GAU_Q_STATUS_INFREE:
		break;
	default:
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d fail for  prQueue->eStatus(%d) err, Handle: %d\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO, prQueue->eStatus, prQueue->u4Handle);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	if (NULL != prQueue->prElemTable)
		DMX_FreeMemory(prQueue->prElemTable);

	prQueue->prElemTable = NULL;
	mm_memset(&(prQueue->rInUseList), 0, sizeof(prQueue->rInUseList));
	mm_memset(&(prQueue->rFreeList), 0, sizeof(prQueue->rFreeList));

	prQueue->u4MaxCnt = 0;
	prQueue->eStatus = GAU_Q_STATUS_IDLE;

	DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
		TEXT("[GAU] %s success(u4Handle: %d)\r\n"),
		DMX_FUNC_NAME, prQueue->u4Handle);

	MM_RETURN(RET_DMX_OK);
}

MRESULT GAU_Q_Release(GAU_Q_T *prQueue)
{
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prQueue)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	if (GAU_Q_STATUS_NONE == prQueue->eStatus)
		MM_RETURN(RET_DMX_OK);

	dmx_sema_lock(prQueue->hSema, DMX_SEMA_OPTION_WAIT);

	if (prQueue->rInUseList.u4Cnt > 0) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] WARNING: %s line %d -- Queue's InUseCount(%d) > 0, Handle: %d\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO, prQueue->rInUseList.u4Cnt, prQueue->u4Handle);
		prQueue->eStatus = GAU_Q_STATUS_INFREE;
		dmx_sema_unlock(prQueue->hSema);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	prQueue->eStatus = GAU_Q_STATUS_INFREE;

	mrRet = GAU_Q_Reset(prQueue);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s fail in GAU_Q_Reset(u4Handle: %d, mrRet: 0x%x)\r\n"),
			DMX_FUNC_NAME, prQueue->u4Handle, mrRet);
		dmx_sema_unlock(prQueue->hSema);
		MM_RETURN(mrRet);
	}

	dmx_sema_unlock(prQueue->hSema);
	mrRet = dmx_sema_delete(prQueue->hSema);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s fail in delete semaphore(u4Handle: %d, mrRet: 0x%x)\r\n"),
			DMX_FUNC_NAME, prQueue->u4Handle, mrRet);
	}

	prQueue->hSema = NULL;

	DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
		TEXT("[GAU] %s success(u4Handle: %d)\r\n"),
		DMX_FUNC_NAME, prQueue->u4Handle);

	MM_RETURN(RET_DMX_OK);
}

MRESULT GAU_Q_Flush(GAU_Q_T *prQueue)
{
	GAU_Q_ELEM_T *prElem = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prQueue)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	if (GAU_Q_STATUS_NONE == prQueue->eStatus)
		MM_RETURN(RET_DMX_OK);

	dmx_sema_lock(prQueue->hSema, DMX_SEMA_OPTION_WAIT);

	prElem = prQueue->rInUseList.prHead;

	while (NULL != prElem) {
		mrRet = GAU_Q_ReleaseElem(prQueue, prElem);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
				TEXT("[GAU] %s line %d fail in GAU_Q_ReleaseElem, Handle: %d\r\n"),
			  DMX_FUNC_NAME, DMX_LINE_NO, prQueue->u4Handle);
			dmx_sema_unlock(prQueue->hSema);
			MM_RETURN(mrRet);
		}
		prElem = prQueue->rInUseList.prHead;
	}

	dmx_sema_unlock(prQueue->hSema);

	DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT, TEXT("[GAU] %s success(u4Handle: %d)\r\n"),
		DMX_FUNC_NAME, prQueue->u4Handle);

	MM_RETURN(RET_DMX_OK);
}

static MRESULT GAU_Q_ReleaseElem(GAU_Q_T *prQueue, GAU_Q_ELEM_T *prFreeElem)
{
	GAU_Q_ELEM_T *prElem = NULL;

	if ((NULL == prQueue) ||
	   (NULL == prFreeElem))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	if (prQueue->rInUseList.u4Cnt <= 0) {
		DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d -- Queue's InUseCount == 0, No need to free the elem, Handle: %d\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO, prQueue->u4Handle);
		MM_RETURN(RET_DMX_OK);
	}

	prElem = prQueue->rInUseList.prHead;
	while (prElem != NULL) {
		if ((prElem == prFreeElem) &&
			(prElem->fgInUsing)) {
			break;
		}
		prElem = prElem->prNext;
	}

	if (NULL == prElem) {
		DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT(
				"[GAU] %s line %d fail for Queue(Handle: %d) has no Elem whose address == prFreeElem(0x%x)\r\n"
			),
		  DMX_FUNC_NAME, DMX_LINE_NO, prQueue->u4Handle, prFreeElem);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (NULL != prElem->prPrev)
		prElem->prPrev->prNext = prElem->prNext;
	if (NULL != prElem->prNext)
		prElem->prNext->prPrev = prElem->prPrev;

	if (prElem == prQueue->rInUseList.prTail)
		prQueue->rInUseList.prTail = prElem->prPrev;
	if (prElem == prQueue->rInUseList.prHead)
		prQueue->rInUseList.prHead = prElem->prNext;
	prQueue->rInUseList.u4Cnt -= 1;

	prElem->prNext = NULL;
	prElem->prPrev = NULL;

	if (NULL != prQueue->rFreeList.prTail) {
		prQueue->rFreeList.prTail->prNext = prElem;
		prElem->prPrev = prQueue->rFreeList.prTail;
		prQueue->rFreeList.prTail = prElem;
	} else {
		prQueue->rFreeList.prTail = prElem;
	}

	if (NULL == prQueue->rFreeList.prHead)
		prQueue->rFreeList.prHead = prElem;

	prQueue->rFreeList.u4Cnt += 1;

	MM_RETURN(RET_DMX_OK);
}

GAU_Q_ELEM_T *GAU_Q_GetElem(GAU_Q_T *prQueue)
{
	GAU_Q_ELEM_T *prElem = NULL;

	if (NULL == prQueue) {
		DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d fail for invalid param\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO);
		return NULL;
	}

	switch (prQueue->eStatus) {
	case GAU_Q_STATUS_INUSE:
		break;
	default:
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d fail for prQueue->eStatus(%d) error, Handle: %d\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO, prQueue->eStatus, prQueue->u4Handle);
		return NULL;
	}

	if (prQueue->rInUseList.u4Cnt >= prQueue->u4MaxCnt) {
		DmxLogD(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d fail for Queue's InUseElemCnt(%d) >= MAXCnt(%d), Handle: %d\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO, prQueue->rInUseList.u4Cnt,
		  prQueue->u4MaxCnt, prQueue->u4Handle);
		return NULL;
	}

	if ((0 == prQueue->rFreeList.u4Cnt) ||
		(NULL == prQueue->rFreeList.prHead)) {
		DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT(
				"[GAU] %s line %d fail for Queue has No free elem, (FreeList(elemcnt: %d) is NULL), Handle: %d\r\n"
			),
		  DMX_FUNC_NAME, DMX_LINE_NO, prQueue->rFreeList.u4Cnt, prQueue->u4Handle);
		return NULL;
	}

	prElem = prQueue->rFreeList.prHead;
	prQueue->rFreeList.prHead = prElem->prNext;
	if (NULL != prElem->prNext)
		prElem->prNext->prPrev = NULL;
	if (prElem == prQueue->rFreeList.prTail) {
		prQueue->rFreeList.prTail = prElem->prPrev;
		if (prQueue->rFreeList.prTail != NULL)
			prQueue->rFreeList.prTail->prNext = NULL;
	}
	prQueue->rFreeList.u4Cnt -= 1;

	prElem->prNext = NULL;
	prElem->prPrev = NULL;
	if (NULL != prQueue->rInUseList.prTail)
		prQueue->rInUseList.prTail->prNext = prElem;
	prElem->prPrev = prQueue->rInUseList.prTail;
	prQueue->rInUseList.prTail = prElem;
	if (NULL == prQueue->rInUseList.prHead)
		prQueue->rInUseList.prHead = prElem;
	prQueue->rInUseList.u4Cnt += 1;

	return prElem;
}

MRESULT GAU_Q_GetAU(GAU_Q_T *prQueue, u32 u4AUIdx)
{
	GAU_Q_ELEM_T *prElem = NULL;

	if (NULL == prQueue)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	dmx_sema_lock(prQueue->hSema, DMX_SEMA_OPTION_WAIT);

	switch (prQueue->eStatus) {
	case GAU_Q_STATUS_NONE:
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d fail for prQueue->eStatus(%d) is NONE, Handle: %d\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO, prQueue->eStatus, prQueue->u4Handle);
		dmx_sema_unlock(prQueue->hSema);
		MM_RETURN(RET_DMX_ERR_STATE);
	case GAU_Q_STATUS_INFREE:
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d fail for prQueue->eStatus(%d) is INFREE, Handle: %d\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO, prQueue->eStatus, prQueue->u4Handle);
		dmx_sema_unlock(prQueue->hSema);
		MM_RETURN(RET_DMX_NO_AU);
	default:
		break;
	}

	prQueue->eStatus = GAU_Q_STATUS_INUSE;

	prElem = GAU_Q_GetElem(prQueue);
	if (NULL == prElem) {
		dmx_sema_unlock(prQueue->hSema);
		MM_RETURN(RET_DMX_NO_AU);
	}

	prElem->u4AuIdx = u4AUIdx;
	prElem->fgInUsing = TRUE;

	dmx_sema_unlock(prQueue->hSema);

	MM_RETURN(RET_DMX_OK);
}

MRESULT GAU_Q_ReleaseAU(GAU_Q_T *prQueue, u32 u4AUIdx)
{
	GAU_Q_ELEM_T *prElem = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prQueue) ||
	   (ESM_INVALID_INDEX == u4AUIdx)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	dmx_sema_lock(prQueue->hSema, DMX_SEMA_OPTION_WAIT);

	switch (prQueue->eStatus) {
	case GAU_Q_STATUS_NONE:
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d fail for prQueue->eStatus(%d) is NONE, Handle: %d\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO, prQueue->eStatus, prQueue->u4Handle);
		dmx_sema_unlock(prQueue->hSema);
		MM_RETURN(RET_DMX_ERR_STATE);
	case GAU_Q_STATUS_INUSE:
	case GAU_Q_STATUS_INFREE:
		break;
	default:
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d fail for prQueue->eStatus(%d) error, Handle: %d\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO, prQueue->eStatus, prQueue->u4Handle);
		dmx_sema_unlock(prQueue->hSema);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	prElem = prQueue->rInUseList.prHead;
	while (NULL != prElem) {
		if (prElem->u4AuIdx == u4AUIdx) {
			break;
		}
		prElem = prElem->prNext;
	}

	if (NULL == prElem) {
		DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d fail for No InUse AU in Queue, Handle: %d\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO, prQueue->u4Handle);
		dmx_sema_unlock(prQueue->hSema);
		MM_RETURN(RET_DMX_OK);
	}

	mrRet = GAU_Q_ReleaseElem(prQueue, prElem);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d fail in GAU_Q_ReleaseElem(AUIdx: %d), Handle: %d\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx, prQueue->u4Handle);
		dmx_sema_unlock(prQueue->hSema);
		MM_RETURN(mrRet);
	}

	if (GAU_Q_STATUS_INFREE == prQueue->eStatus) {
		if (0 == prQueue->rInUseList.u4Cnt) {
			mrRet = GAU_Q_Reset(prQueue);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
					TEXT("[GAU] %s fail in GAU_Q_Reset(u4Handle: %d, mrRet: 0x%x)\r\n"),
					DMX_FUNC_NAME, prQueue->u4Handle, mrRet);
				dmx_sema_unlock(prQueue->hSema);
				MM_RETURN(mrRet);
			}
			dmx_sema_unlock(prQueue->hSema);
			mrRet = dmx_sema_delete(prQueue->hSema);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
					TEXT("[GAU] %s fail in delete semaphore(u4Handle: %d, mrRet: 0x%x)\r\n"),
					DMX_FUNC_NAME, prQueue->u4Handle, mrRet);
			}

			MM_RETURN(RET_DMX_OK);
		}
	}

	dmx_sema_unlock(prQueue->hSema);

	MM_RETURN(RET_DMX_OK);
}

u32 GAU_Q_GetFstInUseAUIdx(GAU_Q_T *prQueue)
{
	GAU_Q_ELEM_T *prElem = NULL;
	u32 u4AUIdx = ESM_INVALID_INDEX;

	if (NULL == prQueue)
		return ESM_INVALID_INDEX;

	dmx_sema_lock(prQueue->hSema, DMX_SEMA_OPTION_WAIT);

	prElem = prQueue->rInUseList.prHead;

	if (NULL == prElem) {
		DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d -- No InUse AU in Queue, Handle: %d\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO, prQueue->u4Handle);
		dmx_sema_unlock(prQueue->hSema);
		return ESM_INVALID_INDEX;
	}

	u4AUIdx = prElem->u4AuIdx;

	dmx_sema_unlock(prQueue->hSema);

	return u4AUIdx;
}

u32 GAU_Q_GetInUseAUIdx(GAU_Q_T *prQueue, GAU_Q_ELEM_T *prElem)
{
	u32 u4AUIdx = ESM_INVALID_INDEX;

	if (NULL == prQueue)
		return ESM_INVALID_INDEX;

	if (NULL == prElem) {
		DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d -- No InUse AU in Queue, Handle: %d\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO, prQueue->u4Handle);
		return ESM_INVALID_INDEX;
	}

	dmx_sema_lock(prQueue->hSema, DMX_SEMA_OPTION_WAIT);

	u4AUIdx = prElem->u4AuIdx;

	dmx_sema_unlock(prQueue->hSema);

	return u4AUIdx;
}


bool GAU_Q_IsFstInUseAUIdx(GAU_Q_T *prQueue, u32 u4AUIdx)
{
	GAU_Q_ELEM_T *prElem = NULL;

	if ((NULL == prQueue) ||
	   (ESM_INVALID_INDEX == u4AUIdx))
		return FALSE;

	dmx_sema_lock(prQueue->hSema, DMX_SEMA_OPTION_WAIT);

	prElem = prQueue->rInUseList.prHead;

	if (NULL == prElem) {
		DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d -- No InUse AU in Queue, Handle: %d\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO, prQueue->u4Handle);
		dmx_sema_unlock(prQueue->hSema);
		return FALSE;
	}

	if (prElem->u4AuIdx != u4AUIdx) {
		DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d -- UsingFstAU's AUIdx(%d), AUIdx(%d), Handle: %d\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO, prElem->u4AuIdx, u4AUIdx, prQueue->u4Handle);
		dmx_sema_unlock(prQueue->hSema);
		return FALSE;
	}

	dmx_sema_unlock(prQueue->hSema);

	return TRUE;
}

MRESULT GAU_Q_ReleaseTailInUseAU(GAU_Q_T *prQueue)
{
	MRESULT mrRet = RET_DMX_OK;
	GAU_Q_ELEM_T *prElem = NULL;

	if (NULL == prQueue)
		return ESM_INVALID_INDEX;

	dmx_sema_lock(prQueue->hSema, DMX_SEMA_OPTION_WAIT);

	prElem = prQueue->rInUseList.prTail;

	if (NULL == prElem) {
		DmxLogT(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d -- No InUse AU in Queue, Handle: %d\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO, prQueue->u4Handle);
		dmx_sema_unlock(prQueue->hSema);
		MM_RETURN(mrRet);
	}

	mrRet = GAU_Q_ReleaseElem(prQueue, prElem);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_DEFAULT,
			TEXT("[GAU] %s line %d fail in GAU_Q_ReleaseElem, Handle: %d\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO, prQueue->u4Handle);
		dmx_sema_unlock(prQueue->hSema);
		MM_RETURN(mrRet);
	}

	dmx_sema_unlock(prQueue->hSema);

	MM_RETURN(RET_DMX_OK);
}

