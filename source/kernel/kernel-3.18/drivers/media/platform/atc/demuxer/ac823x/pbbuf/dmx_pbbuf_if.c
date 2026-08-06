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
 * @file dmx_pbbuf_if.c
 *
 * @par Project
 *	  MT3360
 *
 * @par Description
 *
 *
 * @par Author_Name
 *	  Shuhui Zhang
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
#include <media/atc/dmx_event.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/ose_mem.h>
#include <media/atc/ioctl_dmx.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "dmx_event.h"
#include "dmx_splitter.h"
#include "ose_mem.h"
#include "ioctl_dmx.h"
#include "mm_debug.h"
#endif /* __linux__*/

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_spt.h"
#include "dmx_dump.h"
#include "dmx_sema.h"
#include "dmx_pbbuf.h"
#include "dmx_pbbuf_if.h"
#include "dmx_psr_pbbuf.h"
#include "dmx_psr_cc.h"

#ifndef __linux__
#pragma warning(disable : 4127) /*disable warning C4127: conditional expression is constant*/
#endif /* __linux__*/

EXTERN PBBUF *g_aprPbBuf[DMX_MAX_PBBUF_INST_CNT];
EXTERN bool g_fgPbBufInit;
EXTERN u32	g_u4PbBufFlag;

/*
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *++++++++++++++Follow functions called by MSDK module.++++++++++++++++++++++
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
MRESULT PBBUF_DumpSlotData(void *hSlot, char *wszPreFix)
{
	SLOT *prSlot = (SLOT *)hSlot;

	if ((NULL == prSlot) || (NULL == wszPreFix))
		MM_RETURN(RET_DMX_PARAM_WRONG);


	if (NULL != prSlot->pcBuffer) {
		if (!DmxCreateDumpPbbufFile(prSlot->u8SrcOffset, wszPreFix)) {
#ifdef __linux__
			DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail in DmxCreateDumpPbbufFile")
			TEXT("(u8SrcOfst: %lld)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSlot->u8SrcOffset);
#else
			DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail in DmxCreateDumpPbbufFile")
			TEXT("(u8SrcOfst: %I64d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSlot->u8SrcOffset);
#endif /* #ifdef __linux__*/
			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
		}

#ifdef __linux__
		DMXLOG_TRACE(TEXT("[PBBUF] %s line %d -- DmxCreateDumpPbbufFile")
			TEXT("(PlayBuffer: 0x%x, SrcOfst: %lld, DataOffst: %d, Size: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			(prSlot->pcBuffer + prSlot->u4DataOffset),
			prSlot->u8SrcOffset, prSlot->u4DataOffset,
			prSlot->u4DataSize);
#else
		DMXLOG_TRACE(TEXT("[PBBUF] %s line %d -- DmxCreateDumpPbbufFile")
			TEXT("(PlayBuffer: 0x%x, SrcOfst: %I64d, DataOffst: %d, Size: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			(prSlot->pcBuffer + prSlot->u4DataOffset),
			prSlot->u8SrcOffset, prSlot->u4DataOffset,
			prSlot->u4DataSize);
#endif /* #ifdef __linux__*/

		DmxDumpPbbufData(prSlot->u8SrcOffset, (prSlot->pcBuffer + prSlot->u4DataOffset),
			prSlot->u4DataSize);

		DmxCloseDumpPbbufFile();

#ifdef __linux__
		DMXLOG_TRACE(TEXT("[PBBUF] %s line %d -- DmxCloseDumpPbbufFile")
			TEXT("(SrcOfst: %lld, DataOffst: %d, Size: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSlot->u8SrcOffset,
			 prSlot->u4DataOffset, prSlot->u4DataSize);
#else
		DMXLOG_TRACE(TEXT("[PBBUF] %s line %d -- DmxCloseDumpPbbufFile")
			TEXT("(SrcOfst: %I64d, DataOffst: %d, Size: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSlot->u8SrcOffset,
			 prSlot->u4DataOffset, prSlot->u4DataSize);
#endif /* #ifdef __linux__*/
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT PBBUF_DumpInfo(void *pvSptHdl, bool fgDumpData)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	PBBUF	*prPbBuf = (PBBUF *)SplitterGetPBBuf(pvSptHdl);
	SLOT	*prCur = NULL;
	u32	u4Idx = 0, u4ReadSlotCnt = 0;

	if (!prPbBuf) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);

	u4ReadSlotCnt = PBBUF_GetListCount(prPbBuf, PBBUF_SLOT_READING);

	DMXLOG_TRACE(TEXT("[PBBUF] Slots Cnt (Total: %d, Allocated: %d, ")
		TEXT("Sent: %d, Reading: %d, Free: %d)\r\n"),
		prPbBuf->u4SlotAmount,
		PBBUF_GetListCount(prPbBuf, PBBUF_SLOT_ALLOCATED),
		PBBUF_GetListCount(prPbBuf, PBBUF_SLOT_SEND),
		u4ReadSlotCnt,
		PBBUF_GetListCount(prPbBuf, PBBUF_SLOT_FREE));

	if (NULL != prSpt) {
		PSR_CC *prPsrCC = NULL;

		prPsrCC = (PSR_CC *)(prSpt->pvPsrCC);
		if (NULL != prPsrCC) {
			DMX_READ_BUFFER *prReadBuf = NULL;

			DMXLOG_TRACE(TEXT("[PBBUF] PsrCC->u8TxCurrOffst: 0x%x%x\r\n"),
				(u32)((prPsrCC->u8TxCurrOffset) >> 32),
				(u32)(prPsrCC->u8TxCurrOffset));

			for (u4Idx = 0; u4Idx < MAX_CACHE_PBBUF; u4Idx++) {
				prReadBuf = (DMX_READ_BUFFER *)(&(prPsrCC->arPBBuf[u4Idx]));
				DMXLOG_TRACE(TEXT("[PBBUF] No.%d ReadPbBuf -- SrcOfst: 0x%x%x,")
					TEXT(" BufSz: 0x%x, ValidDataSz: 0x%x, PlayOfst: 0x%x\r\n"),
					u4Idx,
					(u32)((prReadBuf->u8SrcOffset) >> 32),
					(u32)(prReadBuf->u8SrcOffset),
					prReadBuf->u4BufferSize,
					prReadBuf->u4PlaySize,
					prReadBuf->u4PlayOffset);
				if ((prPsrCC->u8TxCurrOffset <= prReadBuf->u8SrcOffset + prReadBuf->u4PlayOffset
					+ prReadBuf->u4PlaySize) &&
					(prReadBuf->u8SrcOffset + prReadBuf->u4PlayOffset <= prPsrCC->u8TxCurrOffset)) {
					u64 u8Ofst = prPsrCC->u8TxCurrOffset - prReadBuf->u8SrcOffset;
					u64 u8Len = (u64)(prReadBuf->u4PlaySize) - u8Ofst;

					DMXLOG_TRACE(TEXT("[PBBUF] No.%d ReadPbBuf -- ValidDataOfst:")
						TEXT(" 0x%x%x, RemainDataSz: 0x%x%x\r\n"),
						u4Idx,
						(u32)(u8Ofst >> 32),
						(u32)(u8Ofst),
						(u32)(u8Len >> 32),
						(u32)(u8Len));
				}
			}
		}
	}

	DMXLOG_TRACE(TEXT("[PBBUF] Dump Reading Slots Buffer Address: \r\n"));
	if (prPbBuf->arSlotLists[PBBUF_SLOT_READING].u4SlotCnt > 0) {
		prCur = prPbBuf->arSlotLists[PBBUF_SLOT_READING].prHeadSlot;
		while (NULL != prCur) {
#ifdef __linux__
			DMXLOG_TRACE(TEXT("[PBBUF] hBuffer: 0x%x, u8SrcOfst: %lld\r\n"),
				prCur->pcBuffer, prCur->u8SrcOffset);
#else
			DMXLOG_TRACE(TEXT("[PBBUF] hBuffer: 0x%x, u8SrcOfst: %I64d\r\n"),
				prCur->pcBuffer, prCur->u8SrcOffset);
#endif /* #ifdef __linux__*/
			if (fgDumpData)
				PBBUF_DumpSlotData(prCur, TEXT("Reading"));

			prCur = prCur->prNextSlot;
		}
	}

	DMXLOG_TRACE(TEXT("[PBBUF] Dump Sent Slots Handle: \r\n"));
	if (prPbBuf->arSlotLists[PBBUF_SLOT_SEND].u4SlotCnt > 0) {
		prCur = prPbBuf->arSlotLists[PBBUF_SLOT_SEND].prHeadSlot;
		while (NULL != prCur) {
#ifdef __linux__
			DMXLOG_TRACE(TEXT("[PBBUF] hBuffer: 0x%x, u8SrcOfst: %lld\r\n"),
				prCur->pcBuffer, prCur->u8SrcOffset);
#else
			DMXLOG_TRACE(TEXT("[PBBUF] hBuffer: 0x%x, u8SrcOfst: %I64d\r\n"),
				prCur->pcBuffer, prCur->u8SrcOffset);
#endif /* #ifdef __linux__*/
			if (fgDumpData)
				PBBUF_DumpSlotData(prCur, TEXT("Sent"));

			prCur = prCur->prNextSlot;
		}
	}

	DMXLOG_TRACE(TEXT("[PBBUF] Dump Free Slots Handle: \r\n"));
	if (prPbBuf->arSlotLists[PBBUF_SLOT_FREE].u4SlotCnt > 0) {
		prCur = prPbBuf->arSlotLists[PBBUF_SLOT_FREE].prHeadSlot;
		while (NULL != prCur) {
#ifdef __linux__
			DMXLOG_TRACE(TEXT("[PBBUF] hBuffer: 0x%x, u8SrcOfst: %lld\r\n"),
				prCur->pcBuffer, prCur->u8SrcOffset);
#else
			DMXLOG_TRACE(TEXT("[PBBUF] hBuffer: 0x%x, u8SrcOfst: %I64d\r\n"),
				prCur->pcBuffer, prCur->u8SrcOffset);
#endif /* #ifdef __linux__*/
			if (fgDumpData)
				PBBUF_DumpSlotData(prCur, TEXT("Free"));

			prCur = prCur->prNextSlot;
		}
	}

	DMXLOG_TRACE(TEXT("[PBBUF] Dump Allocate Slots Handle: \r\n"));
	if (prPbBuf->arSlotLists[PBBUF_SLOT_ALLOCATED].u4SlotCnt > 0) {
		prCur = prPbBuf->arSlotLists[PBBUF_SLOT_ALLOCATED].prHeadSlot;
		while (NULL != prCur) {
#ifdef __linux__
			DMXLOG_TRACE(TEXT("[PBBUF] hBuffer: 0x%x, u8SrcOfst: %lld\r\n"),
				prCur->pcBuffer, prCur->u8SrcOffset);
#else
			DMXLOG_TRACE(TEXT("[PBBUF] hBuffer: 0x%x, u8SrcOfst: %I64d\r\n"),
				prCur->pcBuffer, prCur->u8SrcOffset);
#endif /* #ifdef __linux__*/
			if (fgDumpData)
				PBBUF_DumpSlotData(prCur, TEXT("rALLOCATED"));

			prCur = prCur->prNextSlot;
		}
	}

	PBBUF_EXIT(prPbBuf);

	MM_RETURN(RET_DMX_OK);
}

MRESULT PBBUF_DumpInfoEx(void *pvSptHdl, bool fgDumpData)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	PBBUF	*prPbBuf = (PBBUF *)SplitterGetPBBuf(pvSptHdl);
	SLOT	*prCur = NULL;
	u32	u4Idx = 0, u4ReadSlotCnt = 0;

	if (!prPbBuf) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	u4ReadSlotCnt = PBBUF_GetListCount(prPbBuf, PBBUF_SLOT_READING);

	DMXLOG_TRACE(TEXT("[PBBUF] Slots Cnt (Total: %d, Allocated: %d, ")
		TEXT("Sent: %d, Reading: %d, Free: %d)\r\n"),
		prPbBuf->u4SlotAmount,
		PBBUF_GetListCount(prPbBuf, PBBUF_SLOT_ALLOCATED),
		PBBUF_GetListCount(prPbBuf, PBBUF_SLOT_SEND),
		u4ReadSlotCnt,
		PBBUF_GetListCount(prPbBuf, PBBUF_SLOT_FREE));

	if (NULL != prSpt) {
		PSR_CC *prPsrCC = NULL;

		prPsrCC = (PSR_CC *)(prSpt->pvPsrCC);
		if (NULL != prPsrCC) {
			DMX_READ_BUFFER *prReadBuf = NULL;

			DMXLOG_TRACE(TEXT("[PBBUF] PsrCC->u8TxCurrOffst: 0x%x%x\r\n"),
				(u32)((prPsrCC->u8TxCurrOffset) >> 32),
				(u32)(prPsrCC->u8TxCurrOffset));

			for (u4Idx = 0; u4Idx < MAX_CACHE_PBBUF; u4Idx++) {
				prReadBuf = (DMX_READ_BUFFER *)(&(prPsrCC->arPBBuf[u4Idx]));
				DMXLOG_TRACE(TEXT("[PBBUF] No.%d ReadPbBuf -- SrcOfst")
					TEXT(": 0x%x%x, BufSz: 0x%x, ValidDataSz: 0x%x, PlayOfst: 0x%x\r\n"),
					u4Idx,
					(u32)((prReadBuf->u8SrcOffset) >> 32),
					(u32)(prReadBuf->u8SrcOffset),
					prReadBuf->u4BufferSize,
					prReadBuf->u4PlaySize,
					prReadBuf->u4PlayOffset);
				if ((prPsrCC->u8TxCurrOffset <= prReadBuf->u8SrcOffset + prReadBuf->u4PlayOffset
					+ prReadBuf->u4PlaySize) && (prReadBuf->u8SrcOffset + prReadBuf->u4PlayOffset
					<= prPsrCC->u8TxCurrOffset)) {
					u64 u8Ofst = prPsrCC->u8TxCurrOffset - prReadBuf->u8SrcOffset;
					u64 u8Len = (u64)(prReadBuf->u4PlaySize) - u8Ofst;

					DMXLOG_TRACE(
						TEXT("[PBBUF] No.%d ReadPbBuf -- ValidDataOfst: ")
						TEXT("0x%x%x, RemainDataSz: 0x%x%x\r\n"),
						u4Idx,
						(u32)(u8Ofst >> 32),
						(u32)(u8Ofst),
						(u32)(u8Len >> 32),
						(u32)(u8Len));
				}
			}
		}
	}

	DMXLOG_TRACE(TEXT("[PBBUF] Dump Reading Slots Buffer Address: \r\n"));
	if (prPbBuf->arSlotLists[PBBUF_SLOT_READING].u4SlotCnt > 0) {
		prCur = prPbBuf->arSlotLists[PBBUF_SLOT_READING].prHeadSlot;
		while (NULL != prCur) {
#ifdef __linux__
			DMXLOG_TRACE(TEXT("[PBBUF] hBuffer: 0x%x, u8SrcOfst: %lld\r\n"),
				prCur->pcBuffer, prCur->u8SrcOffset);
#else
			DMXLOG_TRACE(TEXT("[PBBUF] hBuffer: 0x%x, u8SrcOfst: %I64d\r\n"),
				prCur->pcBuffer, prCur->u8SrcOffset);
#endif /* #ifdef __linux__*/
			if (fgDumpData)
				PBBUF_DumpSlotData(prCur, TEXT("Reading"));

			prCur = prCur->prNextSlot;
		}
	}

	DMXLOG_TRACE(TEXT("[PBBUF] Dump Sent Slots Handle: \r\n"));
	if (prPbBuf->arSlotLists[PBBUF_SLOT_SEND].u4SlotCnt > 0) {
		prCur = prPbBuf->arSlotLists[PBBUF_SLOT_SEND].prHeadSlot;
		while (NULL != prCur) {
#ifdef __linux__
			DMXLOG_TRACE(TEXT("[PBBUF] hBuffer: 0x%x, u8SrcOfst: %lld\r\n"),
				prCur->pcBuffer, prCur->u8SrcOffset);
#else
			DMXLOG_TRACE(TEXT("[PBBUF] hBuffer: 0x%x, u8SrcOfst: %I64d\r\n"),
				prCur->pcBuffer, prCur->u8SrcOffset);
#endif /* #ifdef __linux__*/
			if (fgDumpData)
				PBBUF_DumpSlotData(prCur, TEXT("Sent"));
			prCur = prCur->prNextSlot;
		}
	}

	DMXLOG_TRACE(TEXT("[PBBUF] Dump Free Slots Handle: \r\n"));
	if (prPbBuf->arSlotLists[PBBUF_SLOT_FREE].u4SlotCnt > 0) {
		prCur = prPbBuf->arSlotLists[PBBUF_SLOT_FREE].prHeadSlot;
		while (NULL != prCur) {
#ifdef __linux__
			DMXLOG_TRACE(TEXT("[PBBUF] hBuffer: 0x%x, u8SrcOfst: %lld\r\n"),
				prCur->pcBuffer, prCur->u8SrcOffset);
#else
			DMXLOG_TRACE(TEXT("[PBBUF] hBuffer: 0x%x, u8SrcOfst: %I64d\r\n"),
				prCur->pcBuffer, prCur->u8SrcOffset);
#endif /* #ifdef __linux__*/
			if (fgDumpData)
				PBBUF_DumpSlotData(prCur, TEXT("Free"));
			prCur = prCur->prNextSlot;
		}
	}

	DMXLOG_TRACE(TEXT("[PBBUF] Dump Allocate Slots Handle: \r\n"));
	if (prPbBuf->arSlotLists[PBBUF_SLOT_ALLOCATED].u4SlotCnt > 0) {
		prCur = prPbBuf->arSlotLists[PBBUF_SLOT_ALLOCATED].prHeadSlot;
		while (NULL != prCur) {
#ifdef __linux__
			DMXLOG_TRACE(TEXT("[PBBUF] hBuffer: 0x%x, u8SrcOfst: %lld\r\n"),
				prCur->pcBuffer, prCur->u8SrcOffset);
#else
			DMXLOG_TRACE(TEXT("[PBBUF] hBuffer: 0x%x, u8SrcOfst: %I64d\r\n"),
				prCur->pcBuffer, prCur->u8SrcOffset);
#endif /* #ifdef __linux__*/
			if (fgDumpData)
				PBBUF_DumpSlotData(prCur, TEXT("rALLOCATED"));
			prCur = prCur->prNextSlot;
		}
	}

	MM_RETURN(RET_DMX_OK);
}

/**************************************************************************************/
/* MRESULT PBBUF_Init(void)*/
/* Describe: Malloc PBBUF instance and create sema, and this function should be*/
/* call when system bootup.*/
/* Parameters: None*/
/* return: Error Code of*/
/**************************************************************************************/
MRESULT PBBUF_Init(void)
{
	u32	i = 0;
	MRESULT mrRet = RET_DMX_OK;

	if (g_fgPbBufInit)
		MM_RETURN(RET_DMX_OK);

	smp_mb();

	for (i = 0; i < DMX_MAX_PBBUF_INST_CNT; i++) {
		if (NULL != g_aprPbBuf[i]) {
			DMX_FreeMemory(g_aprPbBuf[i]);
			g_aprPbBuf[i] = NULL;
		}
		smp_mb();

		DMX_NewMemory(sizeof(PBBUF), g_aprPbBuf[i]);

		if (NULL == g_aprPbBuf[i]) {
			DMXLOG_ERROR(
				TEXT("[PBBUF] %s fail in alloc pbbuf instance(no mem, i: %d)\r\n"),
				DMX_FUNC_NAME, i);
			DMX_ASSERT(FALSE);
			mrRet = RET_DMX_NO_MEM;
			goto ERRPBBUFINIT;
		}
		smp_mb();

		dmx_memset(g_aprPbBuf[i], 0, sizeof(PBBUF));
	}
	smp_mb();

	g_fgPbBufInit = TRUE;

	MM_RETURN(RET_DMX_OK);

ERRPBBUFINIT:

	for (i = 0; i < DMX_MAX_PBBUF_INST_CNT; i++) {
		if (NULL == g_aprPbBuf[i])
			continue;

		if (NULL != g_aprPbBuf[i]) {
			DMX_FreeMemory(g_aprPbBuf[i]);
			g_aprPbBuf[i] = NULL;
		}
	}

	MM_RETURN(mrRet);
}


/*************************************************************************************/
/* MRESULT PBBUF_Uninit(void)*/
/* Describe: Malloc PBBUF instance and create sema, and this function should be*/
/* call when system shutdown.*/
/* Parameters: u2CompId  [IN] specify the target PBBUF*/
/* return: void*/
/*************************************************************************************/
MRESULT PBBUF_UnInit(void)
{
	u32 u4Idx = 0;

	if (!g_fgPbBufInit)
		MM_RETURN(RET_DMX_OK);

	smp_mb();

	for (u4Idx = 0; u4Idx < DMX_MAX_PBBUF_INST_CNT; u4Idx++) {
		if (NULL == g_aprPbBuf[u4Idx]) {
			DMX_ASSERT(FALSE);
			continue;
		}

		if (NULL != g_aprPbBuf[u4Idx]->hSemaphore) {
			DMX_ASSERT(RET_DMX_OK == dmx_sema_delete(g_aprPbBuf[u4Idx]->hSemaphore));
			g_aprPbBuf[u4Idx]->hSemaphore = NULL;
		}

//#if DMX_NEW_PBBUF_MECHANISM
		if (g_u4PbBufFlag) {
  		if (NULL != g_aprPbBuf[u4Idx]->hEvtGroup) {
  			DMX_ASSERT(OSR_OK == x_ev_group_delete((uintptr_t)g_aprPbBuf[u4Idx]->hEvtGroup));
  			g_aprPbBuf[u4Idx]->hEvtGroup = NULL;
  		}
		}
//#endif /* DMX_NEW_PBBUF_MECHANISM*/
		smp_mb();

		g_aprPbBuf[u4Idx]->fgConnected = FALSE;
		smp_mb();

		if (NULL != g_aprPbBuf[u4Idx]) {
			DMX_FreeMemory(g_aprPbBuf[u4Idx]);
			g_aprPbBuf[u4Idx] = NULL;
		}
	}
	smp_mb();

	g_fgPbBufInit = FALSE;

	MM_RETURN(RET_DMX_OK);
}

MRESULT PBBUF_Connect(void *pvSptHdl, void *pvPsr, u32 *pu4Idx, void **phPbbuf)
{
	PBBUF  *prPbBuf = NULL;
	MRESULT  mrRet = RET_DMX_OK;
	u32 i = 0;
	s32 i4Ret = 0;
	char  szNameBuf[16] = {0};

	//sprintf(szNameBuf, "PbbufEvtGrp_%p", pvSptHdl);

	if (!g_fgPbBufInit) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s fail for g_fgPbbufInit is FALSE")
			TEXT("(Pbbuf Management Not init)\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_NO_INIT);
	}
	smp_mb();

	if (NULL == phPbbuf) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s fail for invalid args(phPbbuf: %p, ")
			TEXT("pvPsr: 0x%x)\r\n"),
			DMX_FUNC_NAME, phPbbuf, pvPsr);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	smp_mb();

	*phPbbuf = NULL;

	for (i = 0; i < DMX_MAX_PBBUF_INST_CNT; i++) {
		prPbBuf = g_aprPbBuf[i];
		if ((NULL != prPbBuf) && !prPbBuf->fgConnected) {
			if (NULL != pu4Idx) {
		        *pu4Idx = i;
		    }
			if (NULL == prPbBuf->hSemaphore) {
				mrRet = dmx_sema_create(&(prPbBuf->hSemaphore),
					  DMX_SEMA_TYPE_BINARY, DMX_SEMA_STATE_UNLOCK);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[PBBUF] %s fail in create semaphore")
						TEXT("(i: %d, mrRet: 0x%x)\r\n"),
						DMX_FUNC_NAME, i, mrRet);
					DMX_ASSERT(FALSE);
					MM_RETURN(mrRet);
				}
			}
			smp_mb();

			//#if DMX_NEW_PBBUF_MECHANISM
			if (g_u4PbBufFlag) {
				DMXLOG_TRACE(TEXT("[PBBUF]-------USING NEW PBBUF MECHANISM--------\r\n"));
	  			if (NULL == prPbBuf->hEvtGroup) {
                			sprintf(szNameBuf, "PbbufEvtGrp_%d", i);
	  				i4Ret = x_ev_group_create((uintptr_t *)&(prPbBuf->hEvtGroup),
	  					(const char *)szNameBuf, DMX_PBBUF_EV_INITIAL);
	  				if (OSR_OK != i4Ret) {
	  					DMXLOG_ERROR(
	  						TEXT("[PBBUF] %s fail in x_ev_group_create(i: %d, i4Ret: 0x%x)\r\n"),
	  						DMX_FUNC_NAME, i, i4Ret);
	  					dmx_sema_delete(prPbBuf->hSemaphore);
	  					prPbBuf->hSemaphore = NULL;

	  					DMX_ASSERT(FALSE);
	  					MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	  				}
	  			}
			}
			//#endif /* DMX_NEW_PBBUF_MECHANISM*/
			smp_mb();

			prPbBuf->fgConnected = TRUE;
			prPbBuf->pvDrvOwner = pvPsr;
			prPbBuf->pvSptHdl	   = pvSptHdl;
			prPbBuf->u4CompID  = i;
			prPbBuf->fgEnable  = FALSE;

			prPbBuf->u8LastSrcOfst = DMX_INVALID_UINT64;

			smp_mb();

			*phPbbuf = (void *)prPbBuf;

			DMXLOG_DEBUG(TEXT("[PBBUF] %s line %d success, ")
				TEXT("(phPbbuf: 0x%x, *pu4Idx: 0x%x, pvSptHdl: 0x%p, pvPsr: 0x%p)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, phPbbuf, i, pvSptHdl, pvPsr);

			MM_RETURN(RET_DMX_OK);
		}
	}

	DMXLOG_ERROR(TEXT("[PBBUF] %s fail for no unused Pbbuf instance")
		TEXT("(phPbbuf: 0x%x, *pu4Idx: 0x%x, pvPsr: 0x%p)\r\n"),
		DMX_FUNC_NAME, phPbbuf, i, pvPsr);

	MM_RETURN(RET_DMX_OVER_LIMIT);
}

MRESULT PBBUF_Disconnect(void *pvSptHdl)
{
	MRESULT mrRet = RET_DMX_OK;
	PBBUF *prPbBuf = (PBBUF *)SplitterGetPBBuf(pvSptHdl);

	if (!g_fgPbBufInit) {
		DMXLOG_TRACE(
			TEXT("[PBBUF] %s line %d success, Pbbuf hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OK);
	}
	smp_mb();

	if (NULL == prPbBuf) {
		DMXLOG_TRACE(
			TEXT("[PBBUF] %s line %d success, pvSptHdl(0x%x) han't connect to any pbbuf\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		MM_RETURN(RET_DMX_OK);
	}
	smp_mb();

	mrRet = PBBUF_Disable(pvSptHdl);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s fail in PBBUF_Disable (pvSptHdl: 0x%p, mrRet: 0x%x)!\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
	}
	smp_mb();

	if (NULL != prPbBuf->hSemaphore) {
		DMX_ASSERT(RET_DMX_OK == dmx_sema_delete(prPbBuf->hSemaphore));
		prPbBuf->hSemaphore = NULL;
	}

//#if DMX_NEW_PBBUF_MECHANISM
	if (g_u4PbBufFlag) {
		if (NULL != prPbBuf->hEvtGroup) {
			DMX_ASSERT(OSR_OK == x_ev_group_delete((uintptr_t)prPbBuf->hEvtGroup));
			prPbBuf->hEvtGroup = NULL;
		}
	}
//#endif /* DMX_NEW_PBBUF_MECHANISM*/
	smp_mb();

	if (prPbBuf->fgConnected) {
		prPbBuf->fgConnected = FALSE;
		prPbBuf->pvDrvOwner	 = NULL;
		prPbBuf->pvSptHdl		 = NULL;
		prPbBuf->u4CompID	 = 0;
	}

	DMXLOG_DEBUG(TEXT("[PBBUF] %s line %d success, pvSptHdl(0x%x)\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);

	MM_RETURN(RET_DMX_OK);
}

MRESULT PBBUF_GetInfo(void *pvSptHdl, uintptr_t *pptrBufSa,
	u32 *pu4BufSz, u32 *pu4SlotSz)
{
	PBBUF *prPbBuf = (PBBUF *)SplitterGetPBBuf(pvSptHdl);

	if (!prPbBuf) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);

	if (NULL == prPbBuf->pcBufSa) {
		PBBUF_EXIT(prPbBuf);

		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	smp_mb();

	if (NULL != pptrBufSa)
		*pptrBufSa = (uintptr_t)(prPbBuf->pcBufSa);

	if (NULL != pu4BufSz)
		*pu4BufSz = prPbBuf->u4TotalSz;


	if (NULL != pu4SlotSz)
		*pu4SlotSz = prPbBuf->u4SlotSz;


	PBBUF_EXIT(prPbBuf);

	MM_RETURN(RET_DMX_OK);
}

/**************************************************************************************/
/* s32 i4PBDrvInit(u16 u2CompId, u32 u4BufTotalSz, u32 u4SlotSz, bool fgEnableSendBusy)*/
/* Describe: Initialize PBBUF parameters, create the slot entry table and initialize it.*/
/* Parameters: u2CompId		[IN] specify the target PBBUF*/
/*			   u4BufTotalSz	[IN] the total size of the buffer*/
/*			   u4SlotSz		[IN] the size of each slot*/
/* Return: S_PBBUF_OK: Initialize and create the slot table successfully.*/
/*E_PBBUF_MEM_ALLOC_FAIL: Fail to create the slot table.*/
/*************************************************************************************/
MRESULT PBBUF_Enable(
	void *pvSptHdl,	 u32 u4BufTotalSz,
	u32 u4SlotSz, u8 u1PbbufType,
	u32 u4HdrParamSz)
{
	PBBUF  *prPbBuf = (PBBUF *)SplitterGetPBBuf(pvSptHdl);
	MRESULT mrRet = RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PBBUF] %s enter!\r\n"), DMX_FUNC_NAME);

	if (!prPbBuf) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);

	dmx_memset(&(prPbBuf->rPbBufInterfaces), 0, sizeof(DMX_IPBBUF));

	switch (u1PbbufType) {
	case SPT_PBUFF_FILE:
	case SPT_PBUFF_ONE_SEG:
	case SPT_PBUFF_FULL_SEG: {
			prPbBuf->rPbBufInterfaces.pmrPbBufInitBuffer = PBBUF_SelfBuf_InitBuffer;
			prPbBuf->rPbBufInterfaces.pvPbBufDeInitBuffer = PBBUF_SelfBuf_DeInitBuffer;
			prPbBuf->rPbBufInterfaces.pmrPbBufGetAllocSlot = PBBUF_SelfBuf_GetAllocSlot;
			prPbBuf->rPbBufInterfaces.pmrPbBufCancelAllocSlot = PBBUF_SelfBuf_CancelAllocSlot;
			prPbBuf->rPbBufInterfaces.pmrPbBufSendDataSlot = PBBUF_SelfBuf_SendDataSlot;
			prPbBuf->rPbBufInterfaces.pmrPbBufReleaseAllocSlot = PBBUF_SelfBuf_ReleaseAllocSlot;
			prPbBuf->rPbBufInterfaces.pmrPbBufCleanAllSlots = PBBUF_SelfBuf_CleanAllSlots;
			prPbBuf->rPbBufInterfaces.pmrPbBufGetAvailDataSlot = PBBUF_SelfBuf_GetAvailDataSlot;
			prPbBuf->rPbBufInterfaces.pmrPbBufReleaseNoUseSlot = PBBUF_SelfBuf_ReleaseNoUseSlot;
			prPbBuf->rPbBufInterfaces.pmrPbBufCancelReadSlot = PBBUF_SelfBuf_CancelReadSlot;

			prPbBuf->rPbBufInterfaces.pmrPbBufReleaseFrmSlotToUnCon = PBBUF_SelfBuf_RelFrmSlotToUnCon;

			#if CONFIG_DRV_HDMI_RX
			prPbBuf->rPbBufInterfaces.pmrPbBufGetAudInParsingInfo = NULL;
			prPbBuf->rPbBufInterfaces.pmrAudInIsRAW = NULL;
			#endif /* CONFIG_DRV_HDMI_RX*/
		}
		break;
	case SPT_PBUFF_AUDIN: {
			prPbBuf->rPbBufInterfaces.pmrPbBufInitBuffer = PBBUF_ExtBuf_InitBuffer;
			prPbBuf->rPbBufInterfaces.pvPbBufDeInitBuffer = PBBUF_ExtBuf_DeInitBuffer;
			prPbBuf->rPbBufInterfaces.pmrPbBufGetAllocSlot = NULL;
			prPbBuf->rPbBufInterfaces.pmrPbBufCancelAllocSlot = NULL;
			prPbBuf->rPbBufInterfaces.pmrPbBufSendDataSlot = PBBUF_ExtBuf_SendDataSlot;
			prPbBuf->rPbBufInterfaces.pmrPbBufReleaseAllocSlot = NULL;
			prPbBuf->rPbBufInterfaces.pmrPbBufCleanAllSlots = PBBUF_ExtBuf_CleanAllSlots;
			prPbBuf->rPbBufInterfaces.pmrPbBufGetAvailDataSlot = PBBUF_ExtBuf_GetAvailDataSlot;
			prPbBuf->rPbBufInterfaces.pmrPbBufReleaseNoUseSlot = PBBUF_ExtBuf_ReleaseNoUseSlot;
			prPbBuf->rPbBufInterfaces.pmrPbBufCancelReadSlot = NULL;

			prPbBuf->rPbBufInterfaces.pmrPbBufReleaseFrmSlotToUnCon = PBBUF_ExtBuf_RelFrmSlotToUnCon;

			#if CONFIG_DRV_HDMI_RX
			prPbBuf->rPbBufInterfaces.pmrPbBufGetAudInParsingInfo = PBBUF_ExtBuf_GetAudInParsingInfo;
			prPbBuf->rPbBufInterfaces.pmrAudInIsRAW = PBBUF_ExtBuf_AudInIsRAW;
			#endif /*CONFIG_DRV_HDMI_RX*/
		}
		break;
	default:
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail for invalid u1PbbufType(%u)!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u1PbbufType);
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(RET_DMX_PARAM_WRONG);
        	break;
	}

	if (NULL == prPbBuf->rPbBufInterfaces.pmrPbBufInitBuffer) {
		dmx_memset(&(prPbBuf->rPbBufInterfaces), 0, sizeof(DMX_IPBBUF));
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(RET_DMX_NO_IMPLEMENT);
	}

	mrRet = prPbBuf->rPbBufInterfaces.pmrPbBufInitBuffer(prPbBuf,
		u4BufTotalSz, u4SlotSz, u4HdrParamSz, u1PbbufType);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail in rPbBufInterfaces")
			TEXT(".pmrPbBufInitBuffer, pvSptHdl(0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(mrRet);
	}

	prPbBuf->fgEnable = TRUE;

	PBBUF_EXIT(prPbBuf);

	DMXLOG_TRACE(TEXT("[PBBUF] %s Success, pvSptHdl: 0x%p, u1PbbufType: %u!\r\n"),
		DMX_FUNC_NAME, pvSptHdl, u1PbbufType);

	MM_RETURN(RET_DMX_OK);
}

MRESULT PBBUF_Disable(void *pvSptHdl)
{
	PBBUF *prPbBuf = (PBBUF *)SplitterGetPBBuf(pvSptHdl);

	if (!prPbBuf) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_OK);
	}

	PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);

	if (!prPbBuf->fgEnable) {
		DMXLOG_DEBUG(
			TEXT("[PBBUF] %s line %d exit, Pbbuf is in disable status\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(RET_DMX_OK);
	}

	if (NULL == prPbBuf->rPbBufInterfaces.pvPbBufDeInitBuffer) {
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(RET_DMX_NO_IMPLEMENT);
	}

	prPbBuf->rPbBufInterfaces.pvPbBufDeInitBuffer(prPbBuf);

	prPbBuf->fgEnable = FALSE;

	dmx_memset(&(prPbBuf->rPbBufInterfaces), 0, sizeof(DMX_IPBBUF));

	PBBUF_EXIT(prPbBuf);

	DMXLOG_TRACE(TEXT("[PBBUF] %s Success, pvSptHdl: 0x%p!\r\n"),
		DMX_FUNC_NAME, pvSptHdl);

	MM_RETURN(RET_DMX_OK);
}

/**********************************************************************/
/* MRESULT PBBUF_GetAllocSlot(uintptr_t pvSptHdl, SEND_BUFFER *prSdBuf)*/
/* Describe: Return a slot(Send Buffer) to middleware and remove it from FREE linked list*/
/*			 if slot available. PBBUF marks it as a DATA slot.*/
/* Parameters: pvSptHdl		[IN]  Splitter instance handle*/
/*			  prSendBuffer [OUT] slot pointer to an allocate buffer if available.*/
/* Return: S_PBBUF_OK		A free slot is available.*/
/*		   RET_DMX_PBBUF_BUSY	  No free slot can be available.*/
/**********************************************************************/
MRESULT PBBUF_GetAllocSlot(void *pvSptHdl, SEND_BUFFER *prSdBuf)
{
	PBBUF  *prPbBuf = (PBBUF *)SplitterGetPBBuf(pvSptHdl);
	MRESULT mrRet = RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PBBUF] -- %s enter\r\n"), DMX_FUNC_NAME);

	if (!((NULL != prPbBuf) && (NULL != pvSptHdl))) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);

	if (NULL == prPbBuf->rPbBufInterfaces.pmrPbBufGetAllocSlot) {
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(RET_DMX_OK);
	}

	mrRet = prPbBuf->rPbBufInterfaces.pmrPbBufGetAllocSlot(prPbBuf, prSdBuf);
	if (DMX_FAILED(mrRet) && (!MM_IS_STATE_ERROR(mrRet))) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail in rPbBufInterfaces")
			TEXT(".pmrPbBufGetAllocSlot, pvSptHdl(0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(mrRet);
	}

	PBBUF_EXIT(prPbBuf);

	MM_RETURN(mrRet);
}

/**********************************************************************/
/* MRESULT PBBUF_CancelAllocSlot(uintptr_t pvSptHdl)*/
/* Describe: Cancel pending request of specified ready to receive event.*/
/* Parameters: pvSptHdl  [IN] Splitter instance handle*/
/* Return: S_PBBUF_OK	always return OK*/
/**********************************************************************/
MRESULT PBBUF_CancelAllocSlot(void *pvSptHdl)
{
	PBBUF  *prPbBuf = (PBBUF *)SplitterGetPBBuf(pvSptHdl);
	MRESULT mrRet = RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PBBUF] -- %s enter\r\n"), DMX_FUNC_NAME);

	if (!prPbBuf) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);

	if (!prPbBuf->fgEnable) {
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(RET_DMX_OK);
	}

	if (NULL == prPbBuf->rPbBufInterfaces.pmrPbBufCancelAllocSlot) {
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(RET_DMX_OK);
	}

	mrRet = prPbBuf->rPbBufInterfaces.pmrPbBufCancelAllocSlot(prPbBuf);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail in rPbBufInterfaces")
			TEXT(".pmrPbBufCancelAllocSlot, pvSptHdl(0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(mrRet);
	}

	PBBUF_EXIT(prPbBuf);

	MM_RETURN(mrRet);
}


/**********************************************************************/
/* MRESULT PBBUF_SendAllocSlot(uintptr_t pvSptHdl, SEND_BUFFER *prSdBuf, bool *pfgExitSent)*/
/* Describe: MW sends a Send Buffer with stream data, PBBUF adds this Send Buffer*/
/*			 to SENT linked list.*/
/* Parameters:	u2CompId		[IN] specify the target PBBUF*/
/*		prSendBuffer		[In] Send Buffer pointer with data information given by MW*/
/*		pfgExitSent		[OUT] Inform MW not to continue to alloc slot*/
/* Return: S_PBBUF_OK	   always return OK*/
/***********************************************************************/
MRESULT PBBUF_SendDataSlot(void *pvSptHdl, SEND_BUFFER *prSdBuf, bool *pfgExitSent)
{
	PBBUF  *prPbBuf = (PBBUF *)SplitterGetPBBuf(pvSptHdl);
	MRESULT mrRet	= RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PBBUF] -- %s enter\r\n"), DMX_FUNC_NAME);

	if (!prPbBuf) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);

	if (NULL == prPbBuf->rPbBufInterfaces.pmrPbBufSendDataSlot) {
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(RET_DMX_NO_IMPLEMENT);
	}

	mrRet = prPbBuf->rPbBufInterfaces.pmrPbBufSendDataSlot(prPbBuf, prSdBuf, pfgExitSent);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail in rPbBufInterfaces")
			TEXT(".pmrPbBufSendDataSlot, pvSptHdl(0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(mrRet);
	}

	PBBUF_EXIT(prPbBuf);

	MM_RETURN(mrRet);
}

MRESULT PBBUF_SendDataSlotEx(void *pvSptHdl, SEND_BUFFER *prSdBuf)
{
	PBBUF  *prPbBuf = (PBBUF *)SplitterGetPBBuf(pvSptHdl);
	MRESULT mrRet	= RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PBBUF] -- %s enter\r\n"), DMX_FUNC_NAME);

	if (!prPbBuf) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);

	if (NULL == prPbBuf->rPbBufInterfaces.pmrPbBufSendDataSlot) {
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(RET_DMX_NO_IMPLEMENT);
	}

	mrRet = prPbBuf->rPbBufInterfaces.pmrPbBufSendDataSlot(prPbBuf, prSdBuf, NULL);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail in rPbBufInterfaces")
			TEXT(".pmrPbBufSendDataSlot, pvSptHdl(0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(mrRet);
	}

	PBBUF_EXIT(prPbBuf);

	MM_RETURN(mrRet);
}

/***********************************************************************/
/* MRESULT PBBUF_ReleaseAllocSlot(void *pvSptHdl, SEND_BUFFER *prSdBuf)*/
/* Describe: Release an allocated Send Buffer, and add it to the FREE linked list*/
/* Parameters:	u2CompId	[IN] specify the target PBBUF*/
/*		prSendBuffer	[In] Send Buffer pointer with data information given by MW*/
/* Return: S_PBBUF_OK	   always return OK*/
/***********************************************************************/
MRESULT PBBUF_ReleaseAllocSlot(void *pvSptHdl, SEND_BUFFER *prSdBuf)
{
	PBBUF  *prPbBuf = (PBBUF *)SplitterGetPBBuf(pvSptHdl);
	MRESULT mrRet	= RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PBBUF] -- %s enter\r\n"), DMX_FUNC_NAME);

	if (!prPbBuf) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);

	if (NULL == prPbBuf->rPbBufInterfaces.pmrPbBufReleaseAllocSlot) {
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(RET_DMX_OK);
	}

	mrRet = prPbBuf->rPbBufInterfaces.pmrPbBufReleaseAllocSlot(prPbBuf, prSdBuf);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail in rPbBufInterfaces")
			TEXT(".pmrPbBufReleaseAllocSlot, pvSptHdl(0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(mrRet);
	}

	PBBUF_EXIT(prPbBuf);

	MM_RETURN(mrRet);
}

MRESULT PBBUF_CleanAllSlots(void *pvSptHdl)
{
	PBBUF  *prPbBuf = (PBBUF *)SplitterGetPBBuf(pvSptHdl);
	MRESULT mrRet	= RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PBBUF] -- %s enter\r\n"), DMX_FUNC_NAME);

	if (!prPbBuf) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);

	if (!prPbBuf->fgEnable) {
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(RET_DMX_OK);
	}

	if (NULL == prPbBuf->rPbBufInterfaces.pmrPbBufCleanAllSlots) {
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(RET_DMX_NO_IMPLEMENT);
	}

	mrRet = prPbBuf->rPbBufInterfaces.pmrPbBufCleanAllSlots(prPbBuf);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail in rPbBufInterfaces")
			TEXT(".pmrPbBufCleanAllSlots, pvSptHdl(0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(mrRet);
	}

	PBBUF_EXIT(prPbBuf);

	MM_RETURN(mrRet);
}

/*
 *===========================================================================
 *++++++++++++++Follow functions called by demuxer module.+++++++++++++++++++
 *===========================================================================
 */
MRESULT PBBUF_GetAvailDataSlot(void *pvSptHdl, DMX_READ_BUFFER *prRdBuf)
{
	PBBUF  *prPbBuf = (PBBUF *)SplitterGetPBBuf(pvSptHdl);
	MRESULT mrRet	= RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PBBUF] -- %s enter\r\n"), DMX_FUNC_NAME);

	if (!prPbBuf) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);

	if (NULL == prPbBuf->rPbBufInterfaces.pmrPbBufGetAvailDataSlot) {
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(RET_DMX_NO_IMPLEMENT);
	}

	mrRet = prPbBuf->rPbBufInterfaces.pmrPbBufGetAvailDataSlot(prPbBuf, prRdBuf);
	if (DMX_FAILED(mrRet) && (!MM_IS_STATE_ERROR(mrRet))) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail in rPbBufInterfaces")
			TEXT(".pmrPbBufGetAvailDataSlot, pvSptHdl(0x%x), mrRet(0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(mrRet);
	}

	PBBUF_EXIT(prPbBuf);

	MM_RETURN(mrRet);
}

MRESULT PBBUF_ReleaseNoUseSlot(void *pvSptHdl, DMX_READ_BUFFER *prRdBuf)
{
	PBBUF  *prPbBuf = (PBBUF *)SplitterGetPBBuf(pvSptHdl);
	MRESULT mrRet	= RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PBBUF] -- %s enter\r\n"), DMX_FUNC_NAME);

	if (!prPbBuf) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);

	if (NULL == prPbBuf->rPbBufInterfaces.pmrPbBufReleaseNoUseSlot) {
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(RET_DMX_NO_IMPLEMENT);
	}

	mrRet = prPbBuf->rPbBufInterfaces.pmrPbBufReleaseNoUseSlot(prPbBuf, prRdBuf);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail in rPbBufInterfaces")
			TEXT(".pmrPbBufReleaseNoUseSlot, pvSptHdl(0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(mrRet);
	}

	PBBUF_EXIT(prPbBuf);

	MM_RETURN(mrRet);
}

MRESULT PBBUF_CancelReadSlot(void *pvSptHdl)
{
	PBBUF  *prPbBuf = (PBBUF *)SplitterGetPBBuf(pvSptHdl);
	MRESULT mrRet	= RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PBBUF] -- %s enter\r\n"), DMX_FUNC_NAME);

	if (!prPbBuf) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);

	if (!prPbBuf->fgEnable) {
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(RET_DMX_OK);
	}

	if (NULL == prPbBuf->rPbBufInterfaces.pmrPbBufCancelReadSlot) {
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(RET_DMX_OK);
	}

	mrRet = prPbBuf->rPbBufInterfaces.pmrPbBufCancelReadSlot(prPbBuf);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail in rPbBufInterfaces")
			TEXT(".pmrPbBufCancelReadSlot, pvSptHdl(0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(mrRet);
	}

	PBBUF_EXIT(prPbBuf);

	MM_RETURN(mrRet);
}

MRESULT PBBUF_ReleaseFrmSlotToUnCon(void *pvSptHdl, DMX_READ_BUFFER *prRdBuf, bool *pfgExistUnCon)
{
	PBBUF  *prPbBuf = (PBBUF *)SplitterGetPBBuf(pvSptHdl);
	MRESULT mrRet	= RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PBBUF] -- %s enter\r\n"), DMX_FUNC_NAME);

	if (!prPbBuf) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);

	if (NULL == prPbBuf->rPbBufInterfaces.pmrPbBufReleaseFrmSlotToUnCon) {
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(RET_DMX_OK);
	}

	mrRet = prPbBuf->rPbBufInterfaces.pmrPbBufReleaseFrmSlotToUnCon(prPbBuf, prRdBuf, pfgExistUnCon);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail in rPbBufInterfaces")
			TEXT(".pmrPbBufCancelReadSlot, pvSptHdl(0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(mrRet);
	}

	PBBUF_EXIT(prPbBuf);

	MM_RETURN(mrRet);
}

#if CONFIG_DRV_HDMI_RX

MRESULT PBBUF_GetAudInParsingInfo(void *pvSptHdl, AUDIN_PARSING_INFO_T *prAudinPsringInfo)
{
	PBBUF  *prPbBuf = (PBBUF *)SplitterGetPBBuf(pvSptHdl);
	MRESULT mrRet	= RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PBBUF] -- %s enter\r\n"), DMX_FUNC_NAME);

	if (!prPbBuf) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);

	if (NULL != prAudinPsringInfo)
		mm_memset(prAudinPsringInfo, 0, sizeof(AUDIN_PARSING_INFO_T));


	if (NULL == prPbBuf->rPbBufInterfaces.pmrPbBufGetAudInParsingInfo) {
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(RET_DMX_OK);
	}

	mrRet = prPbBuf->rPbBufInterfaces.pmrPbBufGetAudInParsingInfo(prPbBuf, prAudinPsringInfo);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail in rPbBufInterfaces")
			TEXT(".pmrPbBufGetAudInParsingInfo, pvSptHdl(0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(mrRet);
	}

	PBBUF_EXIT(prPbBuf);

	MM_RETURN(mrRet);
}

MRESULT PBBUF_AudInIsRAW(void *pvSptHdl, bool *pfgAudInIsRaw)
{
	PBBUF  *prPbBuf = (PBBUF *)SplitterGetPBBuf(pvSptHdl);
	MRESULT mrRet	= RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PBBUF] %s enter\r\n"), DMX_FUNC_NAME);

	if (!prPbBuf) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);

	if (NULL != pfgAudInIsRaw)
		*pfgAudInIsRaw = FALSE;


	if (NULL == prPbBuf->rPbBufInterfaces.pmrAudInIsRAW) {
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(RET_DMX_OK);
	}

	mrRet = prPbBuf->rPbBufInterfaces.pmrAudInIsRAW(prPbBuf, pfgAudInIsRaw);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail in rPbBufInterfaces")
			TEXT(".pmrPbBufGetAudInParsingInfo, pvSptHdl(0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		PBBUF_EXIT(prPbBuf);
		MM_RETURN(mrRet);
	}

	PBBUF_EXIT(prPbBuf);

	MM_RETURN(mrRet);
}

#endif /* CONFIG_DRV_HDMI_RX*/

