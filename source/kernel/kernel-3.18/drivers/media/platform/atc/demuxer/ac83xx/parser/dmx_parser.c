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
 * @file dmx_parser.c
 *
 * @par Project
 *
 *
 * @par Description
 *	  Demuxer Parser
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
#include <media/atc/ioctl_dmx.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "ioctl_dmx.h"
#include "mm_debug.h"
#endif				/* __linux__ */

#include "x_rtos.h"
#include "x_debug.h"
#include "u_os.h"
#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_sema.h"
#include "dmx_stream.h"
#include "dmx_parser.h"
#include "dmx_spt_os.h"
#include "stc_hal.h"
#include "dmx_inst.h"

#ifndef __linux__
#pragma warning(disable : 4127)	/*disable warning C4127: conditional expression is constant */
#endif

EXTERN DMX_PSR_MAN_INFO_T g_rPsrMan;
EXTERN DMX_SPT_MAN_INFO_T g_rSptMan;
EXTERN PSR_STRUCT_T g_rPSRHalStruct[DMX_DEV_CNT];

bool g_fgPSRInit = FALSE;

#ifdef PSR_ENABLE_SEMA
HANDLE g_hPSRSema = NULL;
#endif				/* PSR_ENABLE_SEMA */

#define PSRHWRESQUELOCKINIT(mrRet)	do { \
	mrRet = dmx_sema_create(&(g_rPsrMan.hPSRHWResQueLock), DMX_SEMA_TYPE_BINARY, \
		DMX_SEMA_STATE_UNLOCK); \
	if (DMX_FAILED(mrRet)) {							 \
		DMXLOG_ERROR(TEXT("[DECRYPT] %s line %d, fail in create semaphore, mrRet: 0x%x\r\n"), \
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet); \
	} \
} while (0)
#define PSRHWRESQUELOCK()	 \
	dmx_sema_lock(g_rPsrMan.hPSRHWResQueLock, DMX_SEMA_OPTION_WAIT)

#define PSRHWRESQUEUNLOCK()   \
	dmx_sema_unlock(g_rPsrMan.hPSRHWResQueLock)

#define PSRHWRESQUELOCKDEINIT(mrRet)	do { \
	dmx_sema_delete(g_rPsrMan.hPSRHWResQueLock); \
	g_rPsrMan.hPSRHWResQueLock = NULL;\
} while (0)

/* **********************************************************************/
/* Initial All Parser CC Instances*/
/* **********************************************************************/
static MRESULT PsrCCInit(void)
{
	u32 u4Idx;
	PSR_CC *prPsrCC = NULL;

	if (NULL == g_rPsrMan.aprPsrCCs[0]) {
		DMX_NewMemory((sizeof(PSR_CC) * MAX_PSR_CC_CNT), g_rPsrMan.aprPsrCCs[0]);

		smp_mb();
		if (NULL == g_rPsrMan.aprPsrCCs[0]) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d -- instance count:%d, ")
						  TEXT("alloc mem fail !!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, MAX_PSR_CC_CNT);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_NO_MEM);
		}

		smp_mb();
		/* Done later... */
		dmx_memset((void *) (g_rPsrMan.aprPsrCCs[0]), 0, sizeof(PSR_CC) * MAX_PSR_CC_CNT);

		DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- instance sa:0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, g_rPsrMan.aprPsrCCs[0]);

		smp_mb();
		for (u4Idx = 0; u4Idx < MAX_PSR_CC_CNT; u4Idx++) {
			g_rPsrMan.aprPsrCCs[u4Idx] = g_rPsrMan.aprPsrCCs[0] + u4Idx;
			prPsrCC = (PSR_CC *) (g_rPsrMan.aprPsrCCs[u4Idx]);
			prPsrCC->u4Idx = DMX_INVALID_UINT32;
		}
	} else {
		DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- parser cc(0x%x) insts has ")
					  TEXT("already exists!!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, g_rPsrMan.aprPsrCCs[0]);
	}

	MM_RETURN(RET_DMX_OK);
}

static void PsrCCUnInit(void)
{
	if (NULL != g_rPsrMan.aprPsrCCs[0])
		DMX_FreeMemory(g_rPsrMan.aprPsrCCs[0]);

	smp_mb();

	g_rPsrMan.aprPsrCCs[0] = NULL;
}


/* **********************************************************************/
/* Initial All Parser Filter Instances*/
/* Each Parser CC has MAX_PSR_FILTER_PER_CC count of Parser Filters*/
/* **********************************************************************/
static MRESULT PsrFtrInit(void)
{
	u32 u4Idx;

	if (NULL == g_rPsrMan.aprPsrFtrs[0]) {
		DMX_NewMemory((sizeof(PSR_FILTER) * MAX_FILTER_COUNT), g_rPsrMan.aprPsrFtrs[0]);

		smp_mb();
		if (NULL == g_rPsrMan.aprPsrFtrs[0]) {
			DMXLOG_ERROR(TEXT("[PSR] %s line %d -- filter count:%d, ")
						  TEXT("alloc mem fail !!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, MAX_FILTER_COUNT);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_NO_MEM);
		}

		smp_mb();
		DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- alloc filters mem sa:0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, g_rPsrMan.aprPsrFtrs[0]);

		dmx_memset((void *) (g_rPsrMan.aprPsrFtrs[0]), 0,
			   sizeof(PSR_FILTER) * MAX_FILTER_COUNT);
		smp_mb();

		for (u4Idx = 0; u4Idx < MAX_FILTER_COUNT; u4Idx++)
			g_rPsrMan.aprPsrFtrs[u4Idx] = g_rPsrMan.aprPsrFtrs[0] + u4Idx;
	} else {
		DMXLOG_DEBUG(
			    TEXT
			    ("[PSR] %s line %d -- aprPsrFtrs(0x%x) insts has already exists!!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, g_rPsrMan.aprPsrFtrs[0]);
	}

	MM_RETURN(RET_DMX_OK);
}

static void PsrFtrUnInit(void)
{
	if (NULL != g_rPsrMan.aprPsrFtrs[0])
		DMX_FreeMemory(g_rPsrMan.aprPsrFtrs[0]);
	smp_mb();
	g_rPsrMan.aprPsrFtrs[0] = NULL;
}

/* **********************************************************************/
/* Initial All Parser HW Resource Queues*/
/* Each Parser CC has MAX_PSR_FILTER_PER_CC count of Parser HW Resource Queues*/
/* **********************************************************************/
static MRESULT PsrHwResQueueInit(void)
{
	MRESULT mrRet = RET_DMX_OK;

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d, fail in PSRHWRESQUELOCKINIT!!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(mrRet);
	}

	if (NULL == g_rPsrMan.rPSRHWResQueInfo.ppvQueue) {
		DMX_NewMemory((sizeof(void *) * MAX_FILTER_COUNT),
			      g_rPsrMan.rPSRHWResQueInfo.ppvQueue);
		smp_mb();
		if (NULL == g_rPsrMan.rPSRHWResQueInfo.ppvQueue) {
			DMXLOG_ERROR(
				    TEXT("[PSR] %s line %d, fail in alloc memory -- HW res cnt: %d!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, MAX_FILTER_COUNT);

			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_NO_MEM);
		}

		smp_mb();
		dmx_memset((void *) (g_rPsrMan.rPSRHWResQueInfo.ppvQueue), 0,
			   (sizeof(void *) * MAX_FILTER_COUNT));
	} else {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s line %d, fail -- HW Resource queue already exist!!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_ALREADY_EXIST);
	}

	smp_mb();
	g_rPsrMan.rPSRHWResQueInfo.u4WrPtr = 0;
	g_rPsrMan.rPSRHWResQueInfo.u4RdPtr = 0;

	DMXLOG_DEBUG(TEXT("[PSR] %s success!!\r\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

static void PsrHwResQueueUnInit(void)
{
	if (NULL != g_rPsrMan.rPSRHWResQueInfo.ppvQueue)
		DMX_FreeMemory(g_rPsrMan.rPSRHWResQueInfo.ppvQueue);

	g_rPsrMan.rPSRHWResQueInfo.ppvQueue = NULL;

	g_rPsrMan.rPSRHWResQueInfo.u4WrPtr = 0;

	g_rPsrMan.rPSRHWResQueInfo.u4RdPtr = 0;
}

MRESULT PsrHwResQueueEnQueue(void *pvQueueNode)
{
	PSR_HAL_RES_QUEUE_INFO_T *prPsrHwResQueInfo = NULL;

	prPsrHwResQueInfo = &(g_rPsrMan.rPSRHWResQueInfo);
	if (NULL == prPsrHwResQueInfo->ppvQueue) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d, fail for HW Resource Queue hasn't been")
					  TEXT(" initialized!!\r\n"), DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (prPsrHwResQueInfo->u4WrPtr >= MAX_FILTER_COUNT)
		prPsrHwResQueInfo->u4WrPtr = 0;

	prPsrHwResQueInfo->ppvQueue[prPsrHwResQueInfo->u4WrPtr] = pvQueueNode;

	prPsrHwResQueInfo->u4WrPtr++;

	if (prPsrHwResQueInfo->u4WrPtr >= MAX_FILTER_COUNT)
		prPsrHwResQueInfo->u4WrPtr = 0;

	MM_RETURN(RET_DMX_OK);
}

MRESULT PsrHwResQueueDeQueue(void **ppvQueueNode)
{
	PSR_HAL_RES_QUEUE_INFO_T *prPsrHwResQueInfo = NULL;

	if (NULL == ppvQueueNode) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d, fail for invalid args!!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrHwResQueInfo = &(g_rPsrMan.rPSRHWResQueInfo);
	if (NULL == prPsrHwResQueInfo->ppvQueue) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d, fail for HW Resource Queue hasn't been ")
					  TEXT("initialized!!\r\n"), DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (prPsrHwResQueInfo->u4RdPtr >= MAX_FILTER_COUNT)
		prPsrHwResQueInfo->u4RdPtr = 0;

	if (prPsrHwResQueInfo->u4WrPtr != prPsrHwResQueInfo->u4RdPtr) {
		/* Pop up the next Wait_HW Element, and wake up it */
		*ppvQueueNode = prPsrHwResQueInfo->ppvQueue[prPsrHwResQueInfo->u4RdPtr];
		prPsrHwResQueInfo->u4RdPtr++;
		if (prPsrHwResQueInfo->u4RdPtr >= MAX_FILTER_COUNT)
			prPsrHwResQueInfo->u4RdPtr = 0;
	}

	MM_RETURN(RET_DMX_OK);
}

/* ***********************************************************************************/
/* Initialize Parser CC, Parser Filter, Parser HWRes Qeue, Initial PVR, Create PES header buffer for Video*/
/* **************************************************************************************/
MRESULT ParserInit(void)
{
	MRESULT mrRet = RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- g_fgPSRInit:0x%x\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO, g_fgPSRInit);

	if (g_fgPSRInit)
		MM_RETURN(RET_DMX_OK);

	smp_mb();

	mrRet = PsrCCInit();
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s line %d -- fail in PsrCCInit, mrRet: 0x%x!!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		goto ERRPSRINIT;
	}
	smp_mb();

	mrRet = PsrFtrInit();
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s line %d -- fail in PsrFtrInit, mrRet: 0x%x!!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		goto ERRPSRINIT;
	}
	smp_mb();

	mrRet = PsrHwResQueueInit();
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d -- fail in PsrHwResQueueInit, ")
					  TEXT("mrRet: 0x%x!!\r\n"), DMX_FUNC_NAME, DMX_LINE_NO,
			    mrRet);
		goto ERRPSRINIT;
	}
	smp_mb();

	PSR_LOCK_INIT(mrRet);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			    TEXT("[PSR] %s line %d -- Failed to create sema PsrSemaLock\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		goto ERRPSRINIT;
	}

	smp_mb();

	g_fgPSRInit = TRUE;
	DMXLOG_DEBUG(TEXT("[Psr] %s success\r\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);

ERRPSRINIT:

	PSR_LOCK_UNINIT(mrRet);

	PsrCCUnInit();

	PsrFtrUnInit();

	PsrHwResQueueUnInit();

	MM_RETURN(mrRet);
}

/* **********************************************************************/
/* DeInitialize Parser CC, Parser Filter, Parser HWRes Qeue, Initial PVR*/
/* **********************************************************************/
MRESULT ParserUninit(void)
{
	MRESULT mrRet = RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[Psr] %s -- g_fgPSRInit:0x%x\r\n"),
		    DMX_FUNC_NAME, g_fgPSRInit);

	if (!g_fgPSRInit)
		MM_RETURN(RET_DMX_OK);

	smp_mb();

	PSR_LOCK_UNINIT(mrRet);

	PSR_HAL_Uninit();

	DMXLOG_DEBUG(TEXT("[Psr] %s, cc mem sa:0x%x\r\n"),
		    DMX_FUNC_NAME, g_rPsrMan.aprPsrCCs[0]);
	DMXLOG_DEBUG(TEXT("[Psr] %s, filters mem sa:0x%x\r\n"),
		    DMX_FUNC_NAME, g_rPsrMan.aprPsrFtrs[0]);

	if (NULL != g_rPsrMan.aprPsrCCs[0])
		DMX_FreeMemory(g_rPsrMan.aprPsrCCs[0]);

	g_rPsrMan.aprPsrCCs[0] = NULL;

	if (NULL != g_rPsrMan.aprPsrFtrs[0])
		DMX_FreeMemory(g_rPsrMan.aprPsrFtrs[0]);

	g_rPsrMan.aprPsrFtrs[0] = NULL;

	PsrHwResQueueUnInit();

	smp_mb();

	/* turn off init flag */
	g_fgPSRInit = FALSE;

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_CC_CBSplitter(PSR_CC *prPsrCC, PSR_CB_EVENT eEvent, void *pvData)
{
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prPsrCC) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d, fail for invalid args\r\n"), DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (E_TX_DONE == eEvent)
		prPsrCC->fgUseCmdQ = FALSE;

	mrRet = Splitter4PsrEvent(eEvent, pvData, prPsrCC->pvSptHdl);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d, fail in Splitter4PsrEvent, pvSptHdl: 0x%p,")
					  TEXT(" prPsrCC: 0x%p, mrRet: 0x%x\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->pvSptHdl, prPsrCC, mrRet);
	}

	MM_RETURN(mrRet);
}

bool PSR_HWRes_Obtain(PSR_FILTER *prPsrFtr)
{
	PSR_CC *prPsrCC = NULL;
	MRESULT mrRet = RET_DMX_OK;

	PSR_HAL_LOCK;

	prPsrCC = ((PSR_CC *) (prPsrFtr->pvPsrCC));

	if (DMX_INVALID_UINT8 != prPsrFtr->ucHwDevId) {
		PSR_HAL_UNLOCK;
		DMXLOG_TRACE(TEXT("[PSR] %s line %d fail -- PsrType: %d\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->eType);
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	prPsrFtr->ucHwDevId = PSR_HAL_HWRes_Obtain(prPsrFtr->pvPsrCC);
	smp_mb();

	if (DMX_INVALID_UINT8 == prPsrFtr->ucHwDevId) {
		mrRet = PsrHwResQueueEnQueue((void *)prPsrFtr);
		if (DMX_FAILED(mrRet)) {
			PSR_HAL_UNLOCK;
			DMX_ASSERT(FALSE);
			return FALSE;
		}

		PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_HW);

		PSR_HAL_UNLOCK;
		return FALSE;
	}

	PSR_HAL_UNLOCK;
	DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- PsrType: %d\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->eType);
	return TRUE;
}

#define ParserFtrGetLastPts(hPsrFtr)  \
	((NULL != (hPsrFtr)) ? (((PSR_FILTER *)(hPsrFtr))->u8LastPTS) : INVALID_TIMESTAMP)

bool PsrWakeupFromHwResQueue(u32 u4TxState, void **ppvDmxInst)
{
	void *pvHwResNode = NULL;
	PSR_CC *prPsrCC = NULL;
	PSR_FILTER *pWaitPsrFtr = NULL;
	MRESULT mrRet = RET_DMX_OK;

	while (TRUE) {
		pvHwResNode = NULL;
		mrRet = PsrHwResQueueDeQueue(&pvHwResNode);
		if (DMX_FAILED(mrRet))
			break;

		if (NULL == pvHwResNode)
			break;

		pWaitPsrFtr = (PSR_FILTER *) pvHwResNode;
		if ((NULL != pWaitPsrFtr) && (NULL != pWaitPsrFtr->pvPsrCC)) {
			prPsrCC = (PSR_CC *) (pWaitPsrFtr->pvPsrCC);
			if ((!(prPsrCC->fgCfaPrsEnd))
			    && (u4TxState & ((u32) (1 << (prPsrCC->eTxState)))) != 0) {
				smp_mb();
				*ppvDmxInst = prPsrCC->pvDmxInst;
				PSR_CC_CBSplitter(prPsrCC, E_WAKEUP_ME, NULL);
				return TRUE;
			}
		} else {
			break;
		}
	}

	return FALSE;
}

bool PsrWakeupOtherPsrCC(PSR_FILTER *prPsrFtr,
	PSR_CC *prPsrCC, u32 u4CheckTxState)
{
	u32 i = 0;
	DMX_SPT_INST_T *prSptInst = NULL;
	DMX_INST_T *prDmxInst = NULL;
	PSR_CC *prPsrCC2 = NULL;

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
		if ((prSptInst->pvDmxInst != prDmxInst) || (NULL == prSptInst))
			continue;
		if ((NULL != prSptInst->pvPsrCC) && (prPsrFtr->pvPsrCC != prSptInst->pvPsrCC)) {
			prPsrCC2 = (PSR_CC *) (prSptInst->pvPsrCC);
			if ((!(prPsrCC2->fgCfaPrsEnd))
			    && (u4CheckTxState & ((u32) (1 << (prPsrCC->eTxState)))) != 0) {
				DMXLOG_DEBUG(TEXT("[PSR] %s line %d -- ")
							  TEXT("eType: %d, PsrCC's eTxState: %d,")
							  TEXT(" PsrCC2's eTxState: %d\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO,
					    prPsrFtr->eType,
					    ((NULL == prPsrCC) ? -1 : prPsrCC->eTxState),
					    ((NULL == prPsrCC2) ? -1 : prPsrCC2->eTxState));

				PSR_CC_CBSplitter((HANDLE) prPsrCC2, E_WAKEUP_ME, NULL);
				return TRUE;
			}
		}
	}

	return FALSE;
}

bool PsrWakeupOnePsrCC(u32 u4CheckTxState, void *pvDmxInst)
{
	u32 i = 0;
	DMX_SPT_INST_T *prSptInst = NULL;
	PSR_CC *prPsrCC = NULL;
	DMX_INST_T *prDmxInst = (DMX_INST_T *)pvDmxInst;

	if (NULL ==  prDmxInst) {
		DMX_ASSERT(0);
		return FALSE;
	}

	for (i = 0; i < DMX_MAX_SPT_INST_CNT; i++) {
		prSptInst = g_rSptMan.aprSptInst[i];
		if ((NULL ==  prSptInst) || (pvDmxInst != prSptInst->pvDmxInst))
			continue;
		if ((NULL != prSptInst->pvPsrCC)) {
			prPsrCC = (PSR_CC *) (prSptInst->pvPsrCC);
			DMXLOG_TRACE(TEXT("[PSR] %s line %d -- PsrCC's eTxState")
						  TEXT(": %d\r\n"), DMX_FUNC_NAME, DMX_LINE_NO,
				    prPsrCC->eTxState);

			if (TXS_WAIT_HW == prPsrCC->eTxState) {
				DMXLOG_TRACE(TEXT("[PSR] %s line %d -- Wake up ")
							  TEXT("PsrCC(0x%x), pvSptHdl(0x%x)\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC, prPsrCC->pvSptHdl);
				PSR_CC_CBSplitter((HANDLE) prPsrCC, E_WAKEUP_ME, NULL);
				return TRUE;
			}
		}
	}

	return FALSE;
}

void PSR_HWRes_Release(PSR_FILTER *prPsrFtr)
{
	PSR_CC *prPsrCC = NULL;
	DMX_INST_T *prDmxInst = NULL;
	void *pvDmxInstTemp = 0;
	u32 u4checkTxState = 0;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prPsrFtr) {
		DMXLOG_ERROR(TEXT("[PSR] %s line %d fail for invalid args\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		return;
	}

	prDmxInst = (DMX_INST_T *)prPsrFtr->pvDmxInst;
	if (NULL == prDmxInst) {
		DMX_ASSERT(0);
		return;
	}
	
	PSR_HAL_LOCK;

	prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);

	if (DMX_INVALID_UINT8 != prPsrFtr->ucHwDevId) {
		DMXLOG_DEBUG(TEXT("[PSR] %s line %d PsrType: %d\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->eType);
		mrRet = PSR_HAL_HWRes_Release(prPsrFtr->ucHwDevId);
		prPsrFtr->ucHwDevId = DMX_INVALID_UINT8;
		smp_mb();
		if ((RET_DMX_SUSPEND_OK == mrRet) && (NULL != prPsrCC)) {
			DMXLOG_TRACE(
				    TEXT("[PSR] %s line %d -- Set Splitter(0x%x)'s Usr Event")
				     TEXT(" (SPLITTER_UEV_SUSPEND_OK)\r\n"), DMX_FUNC_NAME,
				    DMX_LINE_NO, prPsrCC->pvSptHdl);
			mrRet = SplitterSetUsrEvent(prPsrCC->pvSptHdl, SPLITTER_UEV_SUSPEND_OK);
			if (DMX_FAILED(mrRet)) {
				PSR_HAL_UNLOCK;
				DMXLOG_ERROR(
						TEXT("[PSR] %s line %d failed in Set Splitter(0x%x)'s")
					     TEXT(" Usr Event (SPLITTER_UEV_SUSPEND_OK)\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->pvSptHdl);
				return;
			}
			return;
		}
	}

	u4checkTxState = (u32) ((1 << TXS_PBBUF_OK) | (1 << TXS_FIFO_OK) | (1 << TXS_WAIT_HW));

	if (PsrWakeupFromHwResQueue(u4checkTxState, &pvDmxInstTemp)) {
		if (pvDmxInstTemp != prDmxInst)
		{
			DMX_ASSERT(0);
		}
		PSR_HAL_UNLOCK;
		return;
	}

	if (prDmxInst->u4SptCnt > 1) {
		prPsrCC = (PSR_CC *) (prPsrFtr->pvPsrCC);

		if (NULL != prPsrCC) {
			u4checkTxState = (u32) ((1 << TXS_PBBUF_OK) |
						   (1 << TXS_FIFO_OK) |
						   (1 << TXS_WAIT_HW) | (1 << TXS_WAIT_IRQ_PROC));

			if (PsrWakeupOtherPsrCC(prPsrFtr, prPsrCC, u4checkTxState)) {
				PSR_HAL_UNLOCK;
				return;
			}
		}
	}
	PSR_HAL_UNLOCK;
}

MRESULT ParserSetPowerState(DMX_PM_STATE ePowerState, void **ppvSptHdl)
{
	MRESULT mrRet = RET_DMX_OK;
	u32 u4checkTxState = 0;
	u32 i;
	void *pvDmxInst = NULL;

	DMX_PM_STATE eCurPowerState = D0;

	eCurPowerState = PSR_HAL_GetPowerState();
	switch (ePowerState) {
	case D0:		/* Power Up */
	case D1:
	case D2:
		/* Resume (if D4 --> D0) */
		if ((D3 == eCurPowerState) || (D4 == eCurPowerState)) {
			if (PSR_HAL_IsHWResOccupied(DDI_PVR_DMA_PATH_ID)) {
				DMXLOG_TRACE(TEXT("[PVR] %s line %d fail for invalid ")
							  TEXT
							  ("state, HW should not be occupied\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO);
			} else {
				VCodeC eVideoCodec = VC_UNKNOW;

				DMXLOG_TRACE(TEXT("[PVR] %s -- POWER_ON\r\n"),
					    DMX_FUNC_NAME);

				mrRet = PSR_HAL_SetPowerState(ePowerState);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(
						    TEXT("[PVR] %s line %d failed in ")
						     TEXT("PSR_HAL_SetPowerState, mrRet: 0x%x\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				}

				for (i = 0; i < DMX_DEV_CNT; i++){
					eVideoCodec = PSR_HAL_GetVideoCodec(i);

					mrRet = PSR_HAL_SetVideoCodec(i, eVideoCodec, TRUE);
					if (DMX_FAILED(mrRet)) {
						DMXLOG_ERROR(
							    TEXT("[PSR] %s line %d, fail in PSR_HAL_SetVideoCodec,")
							     TEXT(" eVCodeC: %d, mrRet: 0x%x!\r\n"),
							    DMX_FUNC_NAME, DMX_LINE_NO, eVideoCodec, mrRet);
					}
				}
				mrRet = PSR_SetAudioDataAftSuspend();
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(
						    TEXT("[PSR] %s line %d, fail in PSR_SetAudioDataAftSuspend,")
						     TEXT(" eVCodeC: %d, mrRet: 0x%x!\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO, eVideoCodec, mrRet);
				}

				u4checkTxState = (u32) (1 << TXS_WAIT_HW);

				if (PsrWakeupFromHwResQueue(u4checkTxState, &pvDmxInst))
					MM_RETURN(RET_DMX_OK);

				if (PsrWakeupOnePsrCC(u4checkTxState, pvDmxInst))
					MM_RETURN(RET_DMX_OK);
			}
		} else {
			DMXLOG_TRACE(
				    TEXT("[PVR] %s -- POWER_ON, Already Power On Now!\r\n"),
				    DMX_FUNC_NAME);
		}
		break;

	case D3:
	case D4:		/* Power Down */
		if ((D0 == eCurPowerState) || (D1 == eCurPowerState) || (D2 == eCurPowerState)) {
			DMXLOG_TRACE(TEXT("[PVR] %s -- POWER_DOWN\r\n"), DMX_FUNC_NAME);
			if (PSR_HAL_IsHWResOccupied(DDI_PVR_DMA_PATH_ID)) {
				HANDLE pvPsrCC = NULL;

				PSR_HAL_SetSuspend(DDI_PVR_DMA_PATH_ID, TRUE, &pvPsrCC);
				if ((NULL != pvPsrCC) && (NULL != ppvSptHdl)) {
					*ppvSptHdl = ((PSR_CC *) pvPsrCC)->pvSptHdl;
					DMXLOG_TRACE(
						    TEXT("[PSR] %s -- Demuxer HW is in occupied,")
						     TEXT(" Set NEEDSUSPEND, PsrCC: 0x%x\r\n"),
						    DMX_FUNC_NAME, ((PSR_CC *) pvPsrCC)->pvSptHdl);
				}
				MM_RETURN(RET_DMX_SUSPEND_NEED_WAIT);
			} else {
				mrRet = PSR_HAL_SetPowerState(ePowerState);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(
						    TEXT("[PVR] %s line %d failed in ")
						     TEXT
						     ("PSR_HAL_SetPowerState, mrRet: 0x%x\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				}
			}
		} else {
			DMXLOG_TRACE(
				    TEXT("[PVR] %s -- POWER_DOWN, Already Power Down Now!\r\n"),
				    DMX_FUNC_NAME);
		}
		break;
	default:
		break;
	}

	MM_RETURN(mrRet);
}

DMX_PM_STATE ParserGetPowerState(void)
{

	DMX_PM_STATE ePowerState;

	ePowerState = PSR_HAL_GetPowerState();

	return ePowerState;
}

MRESULT PSR_SetAudioDataAftSuspend(void)
{
	MM_RETURN(RET_DMX_OK);
}
