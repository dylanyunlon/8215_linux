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
 * @file dmx_esm.c
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
#ifdef __linux__
#include <linux/mm.h>
#include <linux/errno.h>
#include "windows.h"
#include <media/atc/x_buf_def.h>
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_cfa_def.h>
#include <media/atc/ioctl_dmx.h>
/* #include <media/atc/mm_debug.h> */
#include <media/atc/perf_timer.h>
#else
#include "x_buf_def.h"
#include "dmx_define.h"
#include "dmx_cfa_def.h"
#include "ioctl_dmx.h"
#include "mm_debug.h"
#include "perf_timer.h"
#endif /* __linux__ */

#include "dmx_def.h"
#include "dmx_esm.h"
#include "dmx_mem.h"
#include "dmx_spt.h"
#include "dmx_sema.h"
#include "dmx_esm_if.h"
#include "dmx_gau_if.h"
#include "dmx_stream.h"
#include "aud_esm.h"

#ifndef __linux__
#pragma warning(disable : 4127) /*disable warning C4127: conditional expression is constant*/
#endif

EXTERN DMX_ESM_INST_T g_arESMInst[MAX_ESICOUNT];
EXTERN HANDLE g_hESMSema;
EXTERN bool g_fgESMInit;

#if DMX_CHECK_MEM_VALIBILITY
#define DMX_DBG_ESM_INFO		   1
#else
#define DMX_DBG_ESM_INFO		   0
#endif

#define AUDIO_USE_AUDPRIMARY_FIFO  1


#if AUDIO_USE_AUDPRIMARY_FIFO

static bool	_fgAudDrvInClearStatus = FALSE;

#endif /* AUDIO_USE_AUDPRIMARY_FIFO*/


#if DMX_PFM_TEST
EXTERN PSR_PFM g_rPsrPfm;
#endif /*DMX_PFM_TEST*/

static void CheckHandle(u32 u4Handle)
{
	if (MAX_ESICOUNT <= u4Handle) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
	} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
		}
	}
}

static void CheckAUTable(u32 u4Handle)
{
	if (NULL == g_arESMInst[u4Handle].prAUTable) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for the ESIH(%d) has no AUTable\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
	}
}

static void CheckFifo(u32 u4Handle)
{	if (NULL == g_arESMInst[u4Handle].prFifo) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for no fifo (ESIH: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
	}
}

static void CheckInit(void)
{
	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
	}
}

/* **********************************************************************/
/* internal function*/
/* **********************************************************************/

static u32 ESM_AUTableCalcAUExtSz(u32 u4Handle)
{
	u32 u4AUExtSize;

	if (u4Handle >= (u32)MAX_ESICOUNT)
		return 0;

	u4AUExtSize  = 0;

	switch (g_arESMInst[u4Handle].eType) {
	case ES_A:
		if (CFA_TYPE_APE == SplitterGetCfaType(g_arESMInst[u4Handle].pvSptHdl))
			u4AUExtSize = (u32)(sizeof(AU_AUDIO_EXT_INFO_T));
		break;
	default:
		break;
	}

	return u4AUExtSize;
}

#if DMX_DBG_ESM_INFO
void ESM_DumpESMInfo(u32 u4Handle)
{
	CheckInit();
	CheckHandle(u4Handle);

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
		TEXT("[ESM]=========== DUMP ESIH(HANDLE: %d)'s INFO =============\r\n"),
		u4Handle);
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
		TEXT("[ESM] eType: %d(%s), u4FilterType: %d, u4FilterIdx: %d, Spt Handle: 0x%p\r\n"),
		g_arESMInst[u4Handle].eType,
		ESM_TYPESTR(g_arESMInst[u4Handle].eType),
		g_arESMInst[u4Handle].u4FilterType,
		g_arESMInst[u4Handle].u4FilterId,
		g_arESMInst[u4Handle].pvSptHdl);

	if (g_arESMInst[u4Handle].prFifo != NULL) {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] fifo info (ptrSa: 0x%lx, ptrEa: 0x%lx, ptrRdPtr: 0x%lx, ptrWrPtr: 0x%lx)\r\n"),
			g_arESMInst[u4Handle].prFifo->ptrSa,
			g_arESMInst[u4Handle].prFifo->ptrEa,
			g_arESMInst[u4Handle].prFifo->ptrRdPtr,
			g_arESMInst[u4Handle].prFifo->ptrWrPtr);
	} else {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM, TEXT("[ESM] No fifo info\r\n"));
	}

	if (g_arESMInst[u4Handle].prAUTable != NULL) {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			(TEXT("[ESM] AUTable Info (ptrSa: 0x%lx, u4Sz: 0x%lx, u4AUCount: 0x%lx,")
			TEXT(" u4RdIdx: 0x%lx, u4WrIdx: 0x%lx)\r\n")),
			g_arESMInst[u4Handle].prAUTable->ptrSa,
			g_arESMInst[u4Handle].prAUTable->u4Sz,
			g_arESMInst[u4Handle].prAUTable->u4AUCount,
			g_arESMInst[u4Handle].prAUTable->u4RdIdx,
			g_arESMInst[u4Handle].prAUTable->u4WrIdx);
	} else {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM, TEXT("[ESM] No AUTable info\r\n"));
	}

	if (g_arESMInst[u4Handle].prAUExtTable != NULL) {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			(TEXT("[ESM] AUExt Table Info (ptrSa: 0x%lx, u4Sz: 0x%lx, u4AUCount: 0x%lx,")
			TEXT(" u4RdIdx: 0x%lx, u4WrIdx: 0x%lx)\r\n")),
			g_arESMInst[u4Handle].prAUExtTable->ptrSa,
			g_arESMInst[u4Handle].prAUExtTable->u4Sz,
			g_arESMInst[u4Handle].prAUExtTable->u4AUCount,
			g_arESMInst[u4Handle].prAUExtTable->u4RdIdx,
			g_arESMInst[u4Handle].prAUExtTable->u4WrIdx);
	} else {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] No AUExt Table\r\n"));
	}

	if (g_arESMInst[u4Handle].pvDemuxerCBPrivate != NULL) {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] Demuxer Callback Private Data (0x%lx)\r\n"),
			g_arESMInst[u4Handle].pvDemuxerCBPrivate);
	} else {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] No Demuxer Callback Private Data\r\n"));
	}

	if (g_arESMInst[u4Handle].pvDemuxerCB != NULL) {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] Demuxer Callback Address (0x%lx)\r\n"),
			g_arESMInst[u4Handle].pvDemuxerCB);
	} else {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] No Demuxer Callback\r\n"));
	}

	if (g_arESMInst[u4Handle].pvDecoderCBPrivate != NULL) {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] Esm's Decoder Callback Private Data (0x%lx)\r\n"),
			g_arESMInst[u4Handle].pvDecoderCBPrivate);
	} else {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] No Esm's Decoder Callback Private Data\r\n"));
	}

	if (g_arESMInst[u4Handle].pvDecoderCB != NULL) {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] Esm's Decoder Callback Address (0x%lx)\r\n"),
			g_arESMInst[u4Handle].pvDecoderCB);
	} else {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] No Esm's Decoder Callback\r\n"));
	}
}
#endif /* DMX_DBG_ESM_INFO*/

#if DMX_CHECK_MEM_VALIBILITY
static bool ESM_CheckAUTableValibility(u32 u4Handle)
{
	u32 u4AUCount = 0;
	u32 u4AUSize  = 0;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	if (MAX_ESICOUNT <= u4Handle) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		return FALSE;
	} else if (!g_arESMInst[u4Handle].fgUsed) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		return FALSE;
	} else {
		/*do nothing*/
	}

	if (NULL == g_arESMInst[u4Handle].prAUTable) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for the ESIH(%d) has no AUTable\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	if (ES_NONE == g_arESMInst[u4Handle].eType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d) hasn't been set type\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	/* calculate maximum access unit count*/
	switch (g_arESMInst[u4Handle].eType) {
	case ES_V:
		u4AUSize = (u32)(sizeof(AU_VPic));
		u4AUCount = DMX_MAX_VIDEOAU_CNT;
		break;
	case ES_A:
		u4AUSize = (u32)(sizeof(AU_AUDIO));
		u4AUCount = DMX_MAX_AUDIOAU_CNT;
		break;
	case ES_SP:
		u4AUSize = (u32)(sizeof(AU_SP));
		u4AUCount = DMX_MAX_SPAU_CNT;
		break;
	case ES_SECTION:
		u4AUSize = (u32)(sizeof(AU_SECTION));
		u4AUCount = DMX_MAX_SECAU_CNT;
		break;
	default:
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(0x%x)'s Type(%d) is error\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, g_arESMInst[u4Handle].eType);
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	if ((g_arESMInst[u4Handle].prAUTable->u4AUCount != u4AUCount) &&
    (g_arESMInst[u4Handle].prAUTable->u4AUCount > 0)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(0x%x)'s u4AUCount(%d), eType(%d) is error\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle,
			g_arESMInst[u4Handle].prAUTable->u4AUCount,
			g_arESMInst[u4Handle].eType);
		DMX_ASSERT(FALSE);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		return FALSE;
	}

	if ((g_arESMInst[u4Handle].prAUTable->u4Sz != u4AUCount * u4AUSize) &&
    (g_arESMInst[u4Handle].prAUTable->u4Sz > 0) &&
    (u4AUCount > 0)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(0x%x)'s AUTableSz(%d) error, eType(%d) is error\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle,
			g_arESMInst[u4Handle].prAUTable->u4Sz,
			g_arESMInst[u4Handle].eType);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	if ((g_arESMInst[u4Handle].prAUTable->u4RdIdx >=
		g_arESMInst[u4Handle].prAUTable->u4AUCount) &&
		(g_arESMInst[u4Handle].prAUTable->u4AUCount > 0)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail, ESIH(%d), eType(%d), u4RdIdx(%d) > AUCount(%d)\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle,
			g_arESMInst[u4Handle].eType,
			g_arESMInst[u4Handle].prAUTable->u4RdIdx,
			g_arESMInst[u4Handle].prAUTable->u4AUCount);
		DMX_ASSERT(FALSE);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		return FALSE;
	}

	if ((g_arESMInst[u4Handle].prAUTable->u4WrIdx >=
		g_arESMInst[u4Handle].prAUTable->u4AUCount) &&
		(g_arESMInst[u4Handle].prAUTable->u4AUCount > 0)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail, ESIH(%d), eType(%d), u4WrIdx(%d) > AUCount(%d)\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle,
			g_arESMInst[u4Handle].eType,
			g_arESMInst[u4Handle].prAUTable->u4WrIdx,
			g_arESMInst[u4Handle].prAUTable->u4AUCount);
		DMX_ASSERT(FALSE);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		return FALSE;
	}

  if (g_arESMInst[u4Handle].prAUTable->u4AUCount > 0) {
    if ((0 == g_arESMInst[u4Handle].prAUTable->ptrSa) ||
      (ESM_INVALID_ADDRESS == g_arESMInst[u4Handle].prAUTable->ptrSa)) {
      DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
        TEXT("[ESM] %s line %d(u4Handle: 0x%x) fail for invalid AUTable Address(0x%lx)\r\n"),
        DMX_FUNC_NAME, DMX_LINE_NO, u4Handle,
        g_arESMInst[u4Handle].prAUTable->ptrSa);
#if DMX_DBG_ESM_INFO
      ESM_DumpESMInfo(u4Handle);
#endif /* DMX_DBG_ESM_INFO*/
      DMX_ASSERT(FALSE);
      MM_RETURN(RET_DMX_PARAM_WRONG);
    }
  }

	return TRUE;
}

static bool ESM_CheckFifoValibility(u32 u4Handle)
{
	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	if (MAX_ESICOUNT <= u4Handle) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		return FALSE;
	} else if (!g_arESMInst[u4Handle].fgUsed) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		return FALSE;
	} else {
		/*do nothing*/
	}

	if (ES_NONE == g_arESMInst[u4Handle].eType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d) hasn't been set type\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	if (NULL == g_arESMInst[u4Handle].prFifo) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for no fifo (ESIH: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	if ((0 == g_arESMInst[u4Handle].prFifo->ptrSa) ||
		(ESM_INVALID_ADDRESS == g_arESMInst[u4Handle].prFifo->ptrSa)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d (u4Handle: 0x%x) fail for invalid prFifo Address(0x%lx)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle,
			g_arESMInst[u4Handle].prFifo->ptrSa);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	if (g_arESMInst[u4Handle].prFifo->ptrSa >= g_arESMInst[u4Handle].prFifo->ptrEa) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d (u4Handle: 0x%x) fail for invalid prFifo Address, [0x%lx, 0x%lx)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle,
			g_arESMInst[u4Handle].prFifo->ptrSa,
			g_arESMInst[u4Handle].prFifo->ptrEa);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	if ((g_arESMInst[u4Handle].prFifo->ptrRdPtr >= g_arESMInst[u4Handle].prFifo->ptrEa) ||
		(g_arESMInst[u4Handle].prFifo->ptrRdPtr < g_arESMInst[u4Handle].prFifo->ptrSa)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail, ESIH(%d), eType(%d), ptrRdPtr(0x%lx) error, Fifo[0x%lx, 0x%lx)\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle,
			g_arESMInst[u4Handle].eType,
			g_arESMInst[u4Handle].prFifo->ptrRdPtr,
			g_arESMInst[u4Handle].prFifo->ptrSa,
			g_arESMInst[u4Handle].prFifo->ptrEa);
		DMX_ASSERT(FALSE);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		return FALSE;
	}

	if ((g_arESMInst[u4Handle].prFifo->ptrWrPtr >= g_arESMInst[u4Handle].prFifo->ptrEa) ||
		(g_arESMInst[u4Handle].prFifo->ptrWrPtr < g_arESMInst[u4Handle].prFifo->ptrSa)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail, ESIH(%d), eType(%d), ptrWrPtr(0x%lx) error, Fifo[0x%lx, 0x%lx)\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle,
			g_arESMInst[u4Handle].eType,
			g_arESMInst[u4Handle].prFifo->ptrWrPtr,
			g_arESMInst[u4Handle].prFifo->ptrSa,
			g_arESMInst[u4Handle].prFifo->ptrEa);
		DMX_ASSERT(FALSE);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		return FALSE;
	}

	return TRUE;
}
#endif /* DMX_CHECK_MEM_VALIBILITY*/

/*/ Calculate access unit acount*/
static bool ESM_AUTableCalcCount(u32 u4Handle, u32 *pu4AUSz,
	u32 *pu4AUExtSz, u32 *pu4AUCount)
{
	u32 u4AUCount = 0;
	u32 u4AUSize  = 0;

	if ((NULL == pu4AUSz) ||
		(NULL == pu4AUCount) ||
		(NULL == pu4AUExtSz) ||
		(u4Handle >= MAX_ESICOUNT))
		return FALSE;

#ifndef __linux__
#if DMX_CFG_RSPMEM_BY_MEMSIZE
	if (128 == OSE_GetChipMemSize()) {
		/* calculate maximum access unit count*/
		switch (g_arESMInst[u4Handle].eType) {
		case ES_V:
			u4AUSize = (u32)(sizeof(AU_VPic));
			u4AUCount = DMX_MAX_VIDEOAU_CNT_128M;
			break;
		case ES_A:
			u4AUSize = (u32)(sizeof(AU_AUDIO));
			u4AUCount = DMX_MAX_AUDIOAU_CNT_128M;
			break;
		case ES_SP:
			u4AUSize = (u32)(sizeof(AU_SP));
			u4AUCount = DMX_MAX_SP_ESM_AU_CNT_128M;
			break;
		case ES_SECTION:
			u4AUSize = (u32)(sizeof(AU_SECTION));
			u4AUCount = DMX_MAX_SECAU_CNT_128M;
			break;
		default:
			DMX_ASSERT(FALSE);
			return FALSE;
		}
	} else
#endif /* DMX_CFG_RSPMEM_BY_MEMSIZE*/
#endif /* __linux__ */
	{
		/* calculate maximum access unit count*/
		switch (g_arESMInst[u4Handle].eType) {
		case ES_V:
			u4AUSize = (u32)(sizeof(AU_VPic));
			u4AUCount = (u32)DMX_MAX_VIDEOAU_CNT;
			break;
		case ES_A:
			u4AUSize = (u32)(sizeof(AU_AUDIO));
			u4AUCount = (u32)DMX_MAX_AUDIOAU_CNT;
			break;
		case ES_SP:
			u4AUSize = (u32)(sizeof(AU_SP));
			u4AUCount = (u32)DMX_MAX_SPAU_CNT;
			break;
		case ES_SECTION:
			u4AUSize = (u32)(sizeof(AU_SECTION));
			u4AUCount = (u32)DMX_MAX_SECAU_CNT;
			break;
		default:
			DMX_ASSERT(FALSE);
			return FALSE;
		}
	}

	*pu4AUSz = u4AUSize;
	*pu4AUExtSz = ESM_AUTableCalcAUExtSz(u4Handle);
	*pu4AUCount = u4AUCount;

	return TRUE;
}

u32 ESM_AUTableAvailCount(u32 u4Handle)
{
	u32 u4RdIdx, u4WrIdx, u4RetLen;

	if (MAX_ESICOUNT <= u4Handle) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		return 0;
	}

	if (!g_arESMInst[u4Handle].fgUsed) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		return 0;
	}

	if (NULL == g_arESMInst[u4Handle].prAUTable) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for the ESIH(%d) has no AUTable\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		return 0;
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s AUTable FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		return 0;
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	u4WrIdx = g_arESMInst[u4Handle].prAUTable->u4WrIdx;
	if (u4WrIdx >= g_arESMInst[u4Handle].prAUTable->u4AUCount) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s u4WrIdx(%d) > u4AUCount(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4WrIdx,
			g_arESMInst[u4Handle].prAUTable->u4AUCount);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		return 0;
	}

	u4RdIdx = g_arESMInst[u4Handle].prAUTable->u4RdIdx;
	if (u4RdIdx >= g_arESMInst[u4Handle].prAUTable->u4AUCount) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s u4RdIdx(%d) > u4AUCount(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4RdIdx,
			g_arESMInst[u4Handle].prAUTable->u4AUCount);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		return 0;
	}

	if (u4RdIdx <= u4WrIdx)
		u4RetLen = u4WrIdx - u4RdIdx;
	else
		u4RetLen = u4WrIdx + g_arESMInst[u4Handle].prAUTable->u4AUCount - u4RdIdx;

	if (u4RetLen >= g_arESMInst[u4Handle].prAUTable->u4AUCount) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s u4RetLen(%d) > u4AUCount(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4RdIdx,
			g_arESMInst[u4Handle].prAUTable->u4AUCount);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		return 0;
	}

	if ((0 == u4RetLen) &&
		(NULL != g_arESMInst[u4Handle].pvDemuxerCB)) {
		g_arESMInst[u4Handle].pvDemuxerCB(CBE_AU_OUT, NULL,
			g_arESMInst[u4Handle].pvDemuxerCBPrivate);
	}

	return u4RetLen;
}

void ESM_FreeESMem(u32 u4Handle, bool fgNotify)
{
	if (!g_arESMInst[u4Handle].fgUsed)
		return;

	if (NULL != g_arESMInst[u4Handle].prFifo) {
		if (ESM_INVALID_ADDRESS != g_arESMInst[u4Handle].prFifo->ptrSa) {
			if (fgNotify && (NULL != g_arESMInst[u4Handle].pvDecoderCB)) {
				g_arESMInst[u4Handle].pvDecoderCB(CBE_FIFO_DESTROY, NULL,
				g_arESMInst[u4Handle].pvDecoderCBPrivate);
			}
		}
	}

	if (NULL != g_arESMInst[u4Handle].prSwFifo) {
		if (ESM_INVALID_ADDRESS != g_arESMInst[u4Handle].prSwFifo->ptrSa) {
			if (ES_V == g_arESMInst[u4Handle].eType)
				DMX_FreeHwMemory((void *)g_arESMInst[u4Handle].prSwFifo->ptrSa);
			else if (ES_A == g_arESMInst[u4Handle].eType)
				DMX_FreeHwMemory((void *)g_arESMInst[u4Handle].prSwFifo->ptrSa);
			else if (ES_SECTION == g_arESMInst[u4Handle].eType)
				DMX_FreeHwMemory((void *)g_arESMInst[u4Handle].prSwFifo->ptrSa);
			else
				DMX_FreeHwMemory((void *)g_arESMInst[u4Handle].prSwFifo->ptrSa);

			g_arESMInst[u4Handle].prSwFifo->ptrSa = ESM_INVALID_ADDRESS;
			g_arESMInst[u4Handle].prSwFifo->ptrEa = ESM_INVALID_ADDRESS;
			g_arESMInst[u4Handle].prSwFifo->ptrRdPtr = ESM_INVALID_ADDRESS;
			g_arESMInst[u4Handle].prSwFifo->ptrWrPtr = ESM_INVALID_ADDRESS;
		}
	}

	if (NULL != g_arESMInst[u4Handle].prHwFifo) {
		if (ESM_INVALID_ADDRESS != g_arESMInst[u4Handle].prHwFifo->ptrSa) {
			if (ES_V == g_arESMInst[u4Handle].eType) {
				DMX_FreeHwMemory((void *)g_arESMInst[u4Handle].prHwFifo->ptrSa);
			} else if (ES_A == g_arESMInst[u4Handle].eType) {
				#if AUDIO_USE_AUDPRIMARY_FIFO
				Del_MemMap((void *)(g_arESMInst[u4Handle].prHwFifo->ptrSa));
				#else
				DMX_FreeHwMemory((void *)g_arESMInst[u4Handle].prHwFifo->ptrSa);
				#endif /* AUDIO_USE_AUDPRIMARY_FIFO*/
			} else if (ES_SECTION == g_arESMInst[u4Handle].eType) {
				DMX_FreeHwMemory((void *)g_arESMInst[u4Handle].prHwFifo->ptrSa);
			} else {
				DMX_FreeHwMemory((void *)g_arESMInst[u4Handle].prHwFifo->ptrSa);
			}
			g_arESMInst[u4Handle].prHwFifo->ptrSa = ESM_INVALID_ADDRESS;
			g_arESMInst[u4Handle].prHwFifo->ptrEa = ESM_INVALID_ADDRESS;
			g_arESMInst[u4Handle].prHwFifo->ptrRdPtr = ESM_INVALID_ADDRESS;
			g_arESMInst[u4Handle].prHwFifo->ptrWrPtr = ESM_INVALID_ADDRESS;
		}
	}

	if (NULL != g_arESMInst[u4Handle].prAUTable) {
		#if DMX_CHECK_MEM_VALIBILITY
		if (!ESM_CheckAUTableValibility(u4Handle)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for ESIH(%d)'s AUTable FATAL ERROR\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			#if DMX_DBG_ESM_INFO
			ESM_DumpESMInfo(u4Handle);
			#endif /* DMX_DBG_ESM_INFO*/
			DMX_ASSERT(FALSE);
			return;
		}
		#endif /* DMX_CHECK_MEM_VALIBILITY*/

		if (ESM_INVALID_ADDRESS != g_arESMInst[u4Handle].prAUTable->ptrSa) {
			#ifdef __linux__
			DMX_FreeHwMemory(g_arESMInst[u4Handle].prAUTable->ptrSa);
			#else
			DMX_FreeHwMemory((void *)g_arESMInst[u4Handle].prAUTable->ptrSa);
			#endif
		}
		g_arESMInst[u4Handle].prAUTable->ptrSa = ESM_INVALID_ADDRESS;
		g_arESMInst[u4Handle].prAUTable->u4Sz = 0;
		g_arESMInst[u4Handle].prAUTable->u4RdIdx = ESM_INVALID_INDEX;
		g_arESMInst[u4Handle].prAUTable->u4WrIdx = ESM_INVALID_INDEX;
		g_arESMInst[u4Handle].prAUTable->u4AUCount = ESM_INVALID_COUNT;
	}

	if (NULL != g_arESMInst[u4Handle].prAUExtTable) {
		if (ESM_INVALID_ADDRESS != g_arESMInst[u4Handle].prAUExtTable->ptrSa) {
			#ifdef __linux__
			DMX_FreeHwMemory(g_arESMInst[u4Handle].prAUExtTable->ptrSa);
			#else
			DMX_FreeHwMemory((void *)(g_arESMInst[u4Handle].prAUExtTable->ptrSa));
			#endif
		}
		g_arESMInst[u4Handle].prAUExtTable->ptrSa = ESM_INVALID_ADDRESS;
		g_arESMInst[u4Handle].prAUExtTable->u4Sz = 0;
		g_arESMInst[u4Handle].prAUExtTable->u4RdIdx = ESM_INVALID_INDEX;
		g_arESMInst[u4Handle].prAUExtTable->u4WrIdx = ESM_INVALID_INDEX;
		g_arESMInst[u4Handle].prAUExtTable->u4AUCount = ESM_INVALID_COUNT;
	}
}

void ESM_FreeES(u32 u4Handle)
{
	if (!g_arESMInst[u4Handle].fgUsed)
		return;

	ESM_FreeESMem(u4Handle, TRUE);

	if (NULL != g_arESMInst[u4Handle].prSwFifo) {
		DMX_FreeMemory(g_arESMInst[u4Handle].prSwFifo);
		g_arESMInst[u4Handle].prSwFifo = NULL;
	}

	if (NULL != g_arESMInst[u4Handle].prHwFifo) {
		DMX_FreeMemory(g_arESMInst[u4Handle].prHwFifo);
		g_arESMInst[u4Handle].prHwFifo = NULL;
	}

	if (NULL != g_arESMInst[u4Handle].prAUTable) {
		DMX_FreeMemory(g_arESMInst[u4Handle].prAUTable);
		g_arESMInst[u4Handle].prAUTable = NULL;
	}

	if (NULL != g_arESMInst[u4Handle].prAUExtTable) {
		DMX_FreeMemory(g_arESMInst[u4Handle].prAUExtTable);
		g_arESMInst[u4Handle].prAUExtTable = NULL;
	}

	g_arESMInst[u4Handle].pvDecoderCB = NULL;
	g_arESMInst[u4Handle].pvDecoderCBPrivate = NULL;
	g_arESMInst[u4Handle].pvDemuxerCB = NULL;
	g_arESMInst[u4Handle].pvDemuxerCBPrivate = NULL;
	g_arESMInst[u4Handle].u4FilterType = SPT_DATA_UNDEFINE;
	g_arESMInst[u4Handle].u4FilterId = DMX_INVALID_UINT32;
	g_arESMInst[u4Handle].u8DecSendBufMask = 0;
	g_arESMInst[u4Handle].u4StmCodec = DMX_INVALID_UINT32;
	g_arESMInst[u4Handle].prFifo = NULL;
	g_arESMInst[u4Handle].pvSptHdl = NULL;
	g_arESMInst[u4Handle].eType = ES_NONE;
	g_arESMInst[u4Handle].fgUsed = FALSE;
}

/* **********************************************************************/
/* export interface function*/
/* **********************************************************************/
MRESULT ESM_SetESType(u32 u4Handle, ES_TYPE eEsIfType)
{
	if (ES_NONE == eEsIfType)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	g_arESMInst[u4Handle].eType = eEsIfType;

	MM_RETURN(RET_DMX_OK);
}

MRESULT ESM_GetESType(u32 u4Handle, ES_TYPE *pEsType)
{
	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}

	*pEsType = g_arESMInst[u4Handle].eType;

	MM_RETURN(RET_DMX_OK);
}

/*/ Increase current read index*/
MRESULT ESM_AUTableIncRdIdx(u32 u4Handle, u32 u4Value)
{
	u32 u4TgtIdx = 0;
	u32 u4AvailCount = 0;
	void   *pvData = NULL;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prAUTable) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for the ESIH(%d) has no AUTable\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_AUTABLE);
	}
	if (ES_NONE == g_arESMInst[u4Handle].eType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d) hasn't been set type\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_SET_TYPE);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s AUTable FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	u4AvailCount = ESM_AUTableAvailCount(u4Handle);
	if (u4Value > u4AvailCount) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for error params -- eType: 0x%x, Handle: 0x%x,")
			TEXT(" u4Value(0x%x) > AUTotalCnt(0x%x)\r\n"),
			DMX_FUNC_NAME, g_arESMInst[u4Handle].eType, u4Handle,
			u4Value, g_arESMInst[u4Handle].prAUTable->u4AUCount);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for error params -- u4Value: %d, u4AvailCount: %d,")
			TEXT(" u4RdIdx: %d, u4WrIdx: %d\r\n"),
			DMX_FUNC_NAME, u4Value, u4AvailCount,
			g_arESMInst[u4Handle].prAUTable->u4RdIdx,
			g_arESMInst[u4Handle].prAUTable->u4WrIdx);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	while (u4Value != 0) {
		u4TgtIdx = g_arESMInst[u4Handle].prAUTable->u4RdIdx + 1;
		if (u4TgtIdx >= g_arESMInst[u4Handle].prAUTable->u4AUCount)
			u4TgtIdx -= g_arESMInst[u4Handle].prAUTable->u4AUCount;

		if (u4TgtIdx >= g_arESMInst[u4Handle].prAUTable->u4AUCount) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for error params -- eType: 0x%x, Handle: 0x%x,")
				TEXT(" RdIdx(%d) + u4Value(0x%x) > AUTotalCnt(0x%x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, g_arESMInst[u4Handle].eType, u4Handle,
				g_arESMInst[u4Handle].prAUTable->u4RdIdx, u4Value,
				g_arESMInst[u4Handle].prAUTable->u4AUCount);
			#if DMX_DBG_ESM_INFO
			ESM_DumpESMInfo(u4Handle);
			#endif /* DMX_DBG_ESM_INFO*/
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}

		if (NULL != g_arESMInst[u4Handle].pvDemuxerCB) {
			switch (g_arESMInst[u4Handle].eType) {
			case ES_V: {
					AU_VPic *paAUTable = (AU_VPic *)(g_arESMInst[u4Handle].prAUTable->ptrSa);

					if (paAUTable != NULL) {
						pvData = &(paAUTable[g_arESMInst[u4Handle].prAUTable->u4RdIdx]);
						g_arESMInst[u4Handle].pvDemuxerCB(CBE_AU_OUT, pvData,
							g_arESMInst[u4Handle].pvDemuxerCBPrivate);
					}
				}
				break;
			case ES_A: {
					AU_AUDIO *paAUTable = (AU_AUDIO *)(g_arESMInst[u4Handle].prAUTable->ptrSa);

					if (paAUTable != NULL) {
						pvData = &(paAUTable[g_arESMInst[u4Handle].prAUTable->u4RdIdx]);
						g_arESMInst[u4Handle].pvDemuxerCB(CBE_AU_OUT, pvData,
							g_arESMInst[u4Handle].pvDemuxerCBPrivate);
					}
				}
				break;
			case ES_SP: {
					AU_SP *paAUTable = (AU_SP *)(g_arESMInst[u4Handle].prAUTable->ptrSa);

					if (paAUTable != NULL) {
						pvData = &(paAUTable[g_arESMInst[u4Handle].prAUTable->u4RdIdx]);
						g_arESMInst[u4Handle].pvDemuxerCB(CBE_AU_OUT, pvData,
							g_arESMInst[u4Handle].pvDemuxerCBPrivate);
					}
				}
				break;
			case ES_SECTION: {
					AU_SECTION *paAUTable = (AU_SECTION *)(g_arESMInst[u4Handle].prAUTable->ptrSa);

					if (paAUTable != NULL) {
						pvData = &(paAUTable[g_arESMInst[u4Handle].prAUTable->u4RdIdx]);
						g_arESMInst[u4Handle].pvDemuxerCB(CBE_AU_OUT, pvData,
							g_arESMInst[u4Handle].pvDemuxerCBPrivate);
					}
				}
				break;
			default:
				break;
			}
		}

		g_arESMInst[u4Handle].prAUTable->u4RdIdx = u4TgtIdx;
		if (NULL != g_arESMInst[u4Handle].prAUExtTable)
			g_arESMInst[u4Handle].prAUExtTable->u4RdIdx = u4TgtIdx;

		u4Value--;
	}

	MM_RETURN(RET_DMX_OK);
}

/*/ Get current read index*/
MRESULT ESM_AUTableGetRdIdx(u32 u4Handle, u32 *pu4RdIdx)
{
	u32 u4RdIdx = 0;

	if (NULL == pu4RdIdx) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*pu4RdIdx = ESM_INVALID_INDEX;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}
	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prAUTable) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for the ESIH(%d) has no AUTable\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_AUTABLE);
	}
	if (ES_NONE == g_arESMInst[u4Handle].eType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d) hasn't been set type\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_SET_TYPE);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s AUTable FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	u4RdIdx = g_arESMInst[u4Handle].prAUTable->u4RdIdx;
	DMX_ASSERT(u4RdIdx < g_arESMInst[u4Handle].prAUTable->u4AUCount);

	*pu4RdIdx = u4RdIdx;

	MM_RETURN(RET_DMX_OK);
}

/*/ Get current write index*/
MRESULT ESM_AUTableGetWrIdx(u32 u4Handle, u32 *pu4WrIdx)
{
	u32 u4WrIdx = 0;

	if (NULL == pu4WrIdx) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*pu4WrIdx = ESM_INVALID_INDEX;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}
	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prAUTable) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for the ESIH(%d) has no AUTable\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_AUTABLE);
	}
	if (ES_NONE == g_arESMInst[u4Handle].eType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d) hasn't been set type\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_SET_TYPE);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s AUTable FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	u4WrIdx = g_arESMInst[u4Handle].prAUTable->u4WrIdx;

	if (u4WrIdx >= g_arESMInst[u4Handle].prAUTable->u4AUCount) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d error u4WrIdx -- u4RdIdx: %d, u4WrIdx: %d, AUTotalCnt: %d\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			g_arESMInst[u4Handle].prAUTable->u4RdIdx,
			u4WrIdx, g_arESMInst[u4Handle].prAUTable->u4AUCount);
	}

	DMX_ASSERT(u4WrIdx < g_arESMInst[u4Handle].prAUTable->u4AUCount);

	*pu4WrIdx = u4WrIdx;

	MM_RETURN(RET_DMX_OK);
}

/*/ Set current write index*/
MRESULT ESM_AUTableSetWrIdx(u32 u4Handle, u32 u4Value)
{
	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}
	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prAUTable) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for the ESIH(%d) has no AUTable\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	if (ES_NONE == g_arESMInst[u4Handle].eType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d) hasn't been set type\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_SET_TYPE);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s AUTable FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	/* check u4Value range*/
	if (u4Value >= g_arESMInst[u4Handle].prAUTable->u4AUCount) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for error params -- eType: 0x%x, Handle: 0x%x,")
			TEXT(" u4Value(0x%x) > AUTotalCnt(0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, g_arESMInst[u4Handle].eType, u4Handle,
			u4Value, g_arESMInst[u4Handle].prAUTable->u4AUCount);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for error params -- u4Value: %d, u4RdIdx: %d, u4WrIdx: %d\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Value,
			g_arESMInst[u4Handle].prAUTable->u4RdIdx,
			g_arESMInst[u4Handle].prAUTable->u4WrIdx);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (g_arESMInst[u4Handle].prAUTable->u4WrIdx == u4Value)
		MM_RETURN(RET_DMX_OK);

#if DMX_PFM_TEST
	if ((g_arESMInst[u4Handle].prAUTable->u4WrIdx <= u4Value) &&
			(u4Value <= g_arESMInst[u4Handle].prAUTable->u4RdIdx)) { /* Wo <= Wn <= R*/
		if (g_arESMInst[u4Handle].eType == ES_V)
			g_rPsrPfm.rVideo.u8CreateAUCnt += (u64)(u4Value - g_arESMInst[u4Handle].prAUTable->u4WrIdx);
		else if (g_arESMInst[u4Handle].eType == ES_A)
			g_rPsrPfm.rAudio.u8CreateAUCnt += (u64)(u4Value - g_arESMInst[u4Handle].prAUTable->u4WrIdx);
		else if (g_arESMInst[u4Handle].eType == ES_SP)
			g_rPsrPfm.rSP.u8CreateAUCnt += (u64)(u4Value - g_arESMInst[u4Handle].prAUTable->u4WrIdx);
	} else if ((u4Value < g_arESMInst[u4Handle].prAUTable->u4RdIdx) &&
			(g_arESMInst[u4Handle].prAUTable->u4RdIdx <=
				g_arESMInst[u4Handle].prAUTable->u4WrIdx)) { /* Wn < R <= Wo*/
		if (g_arESMInst[u4Handle].eType == ES_V)
			g_rPsrPfm.rVideo.u8CreateAUCnt += (u64)(g_arESMInst[u4Handle].prAUTable->u4AUCount + u4Value
				- g_arESMInst[u4Handle].prAUTable->u4WrIdx);
		else if (g_arESMInst[u4Handle].eType == ES_A)
			g_rPsrPfm.rAudio.u8CreateAUCnt += (u64)(g_arESMInst[u4Handle].prAUTable->u4AUCount + u4Value
				- g_arESMInst[u4Handle].prAUTable->u4WrIdx);
		else if (g_arESMInst[u4Handle].eType == ES_SP)
			g_rPsrPfm.rSP.u8CreateAUCnt += (u64)(g_arESMInst[u4Handle].prAUTable->u4AUCount + u4Value
				- g_arESMInst[u4Handle].prAUTable->u4WrIdx);
	} else if ((g_arESMInst[u4Handle].prAUTable->u4WrIdx <= u4Value) &&
			(g_arESMInst[u4Handle].prAUTable->u4RdIdx <=
				g_arESMInst[u4Handle].prAUTable->u4WrIdx)) { /* R <= Wo <= Wn*/
		if (g_arESMInst[u4Handle].eType == ES_V)
			g_rPsrPfm.rVideo.u8CreateAUCnt += (u64)(u4Value - g_arESMInst[u4Handle].prAUTable->u4WrIdx);
		else if (g_arESMInst[u4Handle].eType == ES_A)
			g_rPsrPfm.rAudio.u8CreateAUCnt += (u64)(u4Value - g_arESMInst[u4Handle].prAUTable->u4WrIdx);
		else if (g_arESMInst[u4Handle].eType == ES_SP)
			g_rPsrPfm.rSP.u8CreateAUCnt += (u64)(u4Value - g_arESMInst[u4Handle].prAUTable->u4WrIdx);
	}
#endif /* DMX_PFM_TEST*/

	g_arESMInst[u4Handle].prAUTable->u4WrIdx = u4Value;
	if (NULL != g_arESMInst[u4Handle].prAUExtTable)
		g_arESMInst[u4Handle].prAUExtTable->u4WrIdx = u4Value;

	if (ES_A == g_arESMInst[u4Handle].eType) {
		if (!_fgAudDrvInClearStatus) {
			if (NULL != g_arESMInst[u4Handle].pvDecoderCB) {
				g_arESMInst[u4Handle].pvDecoderCB(CBE_AU_IN, NULL,
					g_arESMInst[u4Handle].pvDecoderCBPrivate);
			}
		} else {
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d -- AFifo in Audio Clear Status, so don't send CBE_AU_IN event"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		}
	} else {
		if (NULL != g_arESMInst[u4Handle].pvDecoderCB) {
			g_arESMInst[u4Handle].pvDecoderCB(CBE_AU_IN, NULL,
				g_arESMInst[u4Handle].pvDecoderCBPrivate);
		}
	}

	MM_RETURN(RET_DMX_OK);
}

/*/ Increase current write index*/
MRESULT ESM_AUTableIncWrIdx(u32 u4Handle, u32 u4Value)
{
	u32 u4TgtIdx = 0;
	u32 u4FreeCount = 0;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}
	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prAUTable) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for the ESIH(%d) has no AUTable\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	if (ES_NONE == g_arESMInst[u4Handle].eType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d) hasn't been set type\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_SET_TYPE);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s AUTable FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	u4TgtIdx = g_arESMInst[u4Handle].prAUTable->u4WrIdx + u4Value;
	if (u4TgtIdx >= g_arESMInst[u4Handle].prAUTable->u4AUCount)
		u4TgtIdx -= g_arESMInst[u4Handle].prAUTable->u4AUCount;

	if (u4TgtIdx >= g_arESMInst[u4Handle].prAUTable->u4AUCount) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for error params -- eType: 0x%x, Handle: 0x%x,")
			TEXT(" u4WrIdx(%d) + u4Value(0x%x) > AUTotalCnt(0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, g_arESMInst[u4Handle].eType, u4Handle,
			g_arESMInst[u4Handle].prAUTable->u4WrIdx,
			u4Value, g_arESMInst[u4Handle].prAUTable->u4AUCount);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for error params -- u4Value: %d, u4RdIdx: %d, u4WrIdx: %d\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Value,
			g_arESMInst[u4Handle].prAUTable->u4RdIdx,
			g_arESMInst[u4Handle].prAUTable->u4WrIdx);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	u4FreeCount = g_arESMInst[u4Handle].prAUTable->u4AUCount -
		ESM_AUTableAvailCount(u4Handle);
	if (u4Value >= u4FreeCount) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for error params -- eType: 0x%x, Handle: 0x%x,")
			TEXT(" u4Value(0x%x) > AUTotalCnt(0x%x)\r\n"),
			DMX_FUNC_NAME, g_arESMInst[u4Handle].eType, u4Handle,
			u4Value, g_arESMInst[u4Handle].prAUTable->u4AUCount);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for error params -- u4Value: %d, u4FreeCount: %d,")
			TEXT(" u4RdIdx: %d, u4WrIdx: %d\r\n"),
			DMX_FUNC_NAME, u4Value,  u4FreeCount,
			g_arESMInst[u4Handle].prAUTable->u4RdIdx,
			g_arESMInst[u4Handle].prAUTable->u4WrIdx);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

#if DMX_PFM_TEST
	if (g_arESMInst[u4Handle].eType == ES_V)
		g_rPsrPfm.rVideo.u8CreateAUCnt += u4Value;
	else if (g_arESMInst[u4Handle].eType == ES_A)
		g_rPsrPfm.rAudio.u8CreateAUCnt += u4Value;
	else if (g_arESMInst[u4Handle].eType == ES_SP)
		g_rPsrPfm.rSP.u8CreateAUCnt += u4Value;
#endif /* DMX_PFM_TEST*/

	g_arESMInst[u4Handle].prAUTable->u4WrIdx = u4TgtIdx;

	if (NULL != g_arESMInst[u4Handle].prAUExtTable)
		g_arESMInst[u4Handle].prAUExtTable->u4WrIdx = u4TgtIdx;


	if (ES_A == g_arESMInst[u4Handle].eType) {
		if (!_fgAudDrvInClearStatus) {
			if (NULL != g_arESMInst[u4Handle].pvDecoderCB) {
				g_arESMInst[u4Handle].pvDecoderCB(CBE_AU_IN, NULL,
					g_arESMInst[u4Handle].pvDecoderCBPrivate);
			}
		} else {
			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d -- AFifo in Audio Clear Status, so don't send CBE_AU_IN event"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		}
	} else {
		if (NULL != g_arESMInst[u4Handle].pvDecoderCB) {
			g_arESMInst[u4Handle].pvDecoderCB(CBE_AU_IN, NULL,
				g_arESMInst[u4Handle].pvDecoderCBPrivate);
		}
	}

	MM_RETURN(RET_DMX_OK);
}

/*/ Get total AU count*/
MRESULT ESM_AUTableGetTotalCount(u32 u4Handle, u32 *pu4TotalCnt)
{
	u32 u4TotalCount = 0;

	if (NULL == pu4TotalCnt)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	*pu4TotalCnt = ESM_INVALID_COUNT;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}
	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}

	if (NULL == g_arESMInst[u4Handle].prAUTable) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s(u4Handle: 0x%x) fail for p4AUTable\r\n"),
			DMX_FUNC_NAME, u4Handle);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (ES_NONE == g_arESMInst[u4Handle].eType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s(u4Handle: 0x%x) fail for eType han't been set\r\n"),
			DMX_FUNC_NAME, u4Handle);
		MM_RETURN(RET_DMX_NO_SET_TYPE);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s AUTable FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	u4TotalCount = g_arESMInst[u4Handle].prAUTable->u4AUCount;
	*pu4TotalCnt = u4TotalCount;

	MM_RETURN(RET_DMX_OK);
}

/*/ Get access unit count*/
MRESULT ESM_AUTableGetAvailCount(u32 u4Handle, u32 *pu4AvailCnt)
{
	u32 u4AvailCount = 0;

	if (NULL == pu4AvailCnt)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	*pu4AvailCnt = ESM_INVALID_COUNT;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s AUTable FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	if (NULL == g_arESMInst[u4Handle].prAUTable) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s(u4Handle: 0x%x) fail for p4AUTable\r\n"),
			DMX_FUNC_NAME, u4Handle);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (ES_NONE == g_arESMInst[u4Handle].eType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s(u4Handle: 0x%x) fail for eType han't been set\r\n"),
			DMX_FUNC_NAME, u4Handle);
		MM_RETURN(RET_DMX_NO_SET_TYPE);
	}

	u4AvailCount = ESM_AUTableAvailCount(u4Handle);
	*pu4AvailCnt = u4AvailCount;

	MM_RETURN(RET_DMX_OK);
}

/*/ Get free access unit count*/
MRESULT ESM_AUTableGetFreeCount(u32 u4Handle, u32 *pu4FreeCount)
{
	u32 u4FreeCount = 0;

	if (NULL == pu4FreeCount)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	*pu4FreeCount = ESM_INVALID_COUNT;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}
	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prAUTable) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for the ESIH(%d) has no AUTable\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	if (ES_NONE == g_arESMInst[u4Handle].eType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d) hasn't been set type\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_SET_TYPE);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s AUTable FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	u4FreeCount = g_arESMInst[u4Handle].prAUTable->u4AUCount -
		ESM_AUTableAvailCount(u4Handle);
	*pu4FreeCount = u4FreeCount;

	MM_RETURN(RET_DMX_OK);
}

/*/ Get access unit information*/
MRESULT ESM_AUTableGetAUInfo(u32 u4Handle, u32 u4Value, void **ppvAUInfo)
{
	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}
	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prAUTable) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for the ESIH(%d) has no AUTable\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (u4Value >= g_arESMInst[u4Handle].prAUTable->u4AUCount) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for AU's Index(%d) exceed ")
			TEXT("the max value(%d), (ESIH: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			u4Value, g_arESMInst[u4Handle].prAUTable->u4AUCount, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (ES_NONE == g_arESMInst[u4Handle].eType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d) hasn't been set type\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_SET_TYPE);
	}

	if (NULL == ppvAUInfo) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d(u4Handle: 0x%x) fail for pprAUInfo is NULL\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*ppvAUInfo = NULL;

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s AUTable FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	switch (g_arESMInst[u4Handle].eType) {
	case ES_V: {
			AU_VPic *paAUTable = NULL;

			if (ESM_INVALID_ADDRESS == g_arESMInst[u4Handle].prAUTable->ptrSa) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d(u4Handle: 0x%x) fail for invalid AUTable Address\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle,
					g_arESMInst[u4Handle].prAUTable->ptrSa);
				#if DMX_DBG_ESM_INFO
				ESM_DumpESMInfo(u4Handle);
				#endif /* DMX_DBG_ESM_INFO*/
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			paAUTable = (AU_VPic *)(g_arESMInst[u4Handle].prAUTable->ptrSa);
			if (u4Value >= g_arESMInst[u4Handle].prAUTable->u4AUCount) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d(u4Handle: 0x%x) fail for invalid")
					TEXT(" arg--u4Value(%d), u4AUCount(%d)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4Value,
					g_arESMInst[u4Handle].prAUTable->u4AUCount);
				#if DMX_DBG_ESM_INFO
				ESM_DumpESMInfo(u4Handle);
				#endif /* DMX_DBG_ESM_INFO*/
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			*ppvAUInfo = (void *)(&(paAUTable[u4Value]));
			if (((u32)(*ppvAUInfo) + sizeof(AU_VPic) > g_arESMInst[u4Handle].prAUTable->ptrSa +
				g_arESMInst[u4Handle].prAUTable->u4Sz) ||
				((u32)(*ppvAUInfo) < g_arESMInst[u4Handle].prAUTable->ptrSa)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d(u4Handle: 0x%x) fail for AU's")
					TEXT(" address error, u4Value(%d),")
					TEXT(" u4AUCount(%d), AUTable(SA(0x%lx), EA(0x%lx))\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4Value,
					g_arESMInst[u4Handle].prAUTable->u4AUCount,
					g_arESMInst[u4Handle].prAUTable->ptrSa,
					(g_arESMInst[u4Handle].prAUTable->ptrSa +
					g_arESMInst[u4Handle].prAUTable->u4Sz));
				#if DMX_DBG_ESM_INFO
				ESM_DumpESMInfo(u4Handle);
				#endif /* DMX_DBG_ESM_INFO*/
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
		}
		break;
	case ES_A: {
			AU_AUDIO *paAUTable = NULL;

			if (ESM_INVALID_ADDRESS == g_arESMInst[u4Handle].prAUTable->ptrSa) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d(u4Handle: 0x%x) fail for invalid AUTable Address\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle,
					g_arESMInst[u4Handle].prAUTable->ptrSa);
				#if DMX_DBG_ESM_INFO
				ESM_DumpESMInfo(u4Handle);
				#endif /* DMX_DBG_ESM_INFO*/
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			paAUTable = (AU_AUDIO *)(g_arESMInst[u4Handle].prAUTable->ptrSa);
			if (u4Value >= g_arESMInst[u4Handle].prAUTable->u4AUCount) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d(u4Handle: 0x%x) fail for invalid")
					TEXT(" arg--u4Value(%d), u4AUCount(%d)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4Value,
					g_arESMInst[u4Handle].prAUTable->u4AUCount);
				#if DMX_DBG_ESM_INFO
				ESM_DumpESMInfo(u4Handle);
				#endif /* DMX_DBG_ESM_INFO*/
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			*ppvAUInfo = (void *)(&(paAUTable[u4Value]));
			if (((u32)(*ppvAUInfo) + sizeof(AU_AUDIO) > g_arESMInst[u4Handle].prAUTable->ptrSa +
				g_arESMInst[u4Handle].prAUTable->u4Sz) ||
				((u32)(*ppvAUInfo) < g_arESMInst[u4Handle].prAUTable->ptrSa)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d(u4Handle: 0x%x) fail for AU's address error,")
					TEXT(" u4Value(%d), u4AUCount(%d), AUTable(SA(0x%lx), EA(0x%lx))\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4Value,
					g_arESMInst[u4Handle].prAUTable->u4AUCount,
					g_arESMInst[u4Handle].prAUTable->ptrSa,
					(g_arESMInst[u4Handle].prAUTable->ptrSa +
					g_arESMInst[u4Handle].prAUTable->u4Sz));
				#if DMX_DBG_ESM_INFO
				ESM_DumpESMInfo(u4Handle);
				#endif /* DMX_DBG_ESM_INFO*/
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
		}
		break;
	case ES_SP: {
			AU_SP *paAUTable = NULL;

			if (ESM_INVALID_ADDRESS == g_arESMInst[u4Handle].prAUTable->ptrSa) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d(u4Handle: 0x%x) fail for invalid AUTable Address\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle,
					g_arESMInst[u4Handle].prAUTable->ptrSa);
				#if DMX_DBG_ESM_INFO
				ESM_DumpESMInfo(u4Handle);
				#endif /* DMX_DBG_ESM_INFO*/
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			paAUTable = (AU_SP *)(g_arESMInst[u4Handle].prAUTable->ptrSa);
			if (u4Value >= g_arESMInst[u4Handle].prAUTable->u4AUCount) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d(u4Handle: 0x%x) fail for invalid")
					TEXT(" arg--u4Value(%d), u4AUCount(%d)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4Value,
					g_arESMInst[u4Handle].prAUTable->u4AUCount);
				#if DMX_DBG_ESM_INFO
				ESM_DumpESMInfo(u4Handle);
				#endif /* DMX_DBG_ESM_INFO*/
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			*ppvAUInfo = (void *)(&(paAUTable[u4Value]));
			if (((u32)(*ppvAUInfo) + sizeof(AU_SP) > g_arESMInst[u4Handle].prAUTable->ptrSa +
				g_arESMInst[u4Handle].prAUTable->u4Sz) ||
				((u32)(*ppvAUInfo) < g_arESMInst[u4Handle].prAUTable->ptrSa)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d(u4Handle: 0x%x) fail for AU's address error,")
					TEXT(" u4Value(%d), u4AUCount(%d), AUTable(SA(0x%lx), EA(0x%lx))\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4Value,
					g_arESMInst[u4Handle].prAUTable->u4AUCount,
					g_arESMInst[u4Handle].prAUTable->ptrSa,
					(g_arESMInst[u4Handle].prAUTable->ptrSa +
					g_arESMInst[u4Handle].prAUTable->u4Sz));
				#if DMX_DBG_ESM_INFO
				ESM_DumpESMInfo(u4Handle);
				#endif /* DMX_DBG_ESM_INFO*/
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
		}
		break;
	case ES_SECTION: {
			AU_SECTION *paAUTable = NULL;

			if (ESM_INVALID_ADDRESS == g_arESMInst[u4Handle].prAUTable->ptrSa) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d(u4Handle: 0x%x) fail for invalid AUTable Address\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle,
					g_arESMInst[u4Handle].prAUTable->ptrSa);
				#if DMX_DBG_ESM_INFO
				ESM_DumpESMInfo(u4Handle);
				#endif /* DMX_DBG_ESM_INFO*/
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			paAUTable = (AU_SECTION *)(g_arESMInst[u4Handle].prAUTable->ptrSa);
			if (u4Value >= g_arESMInst[u4Handle].prAUTable->u4AUCount) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d(u4Handle: 0x%x) fail for invalid")
					TEXT(" arg--u4Value(%d), u4AUCount(%d)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4Value,
					g_arESMInst[u4Handle].prAUTable->u4AUCount);
				#if DMX_DBG_ESM_INFO
				ESM_DumpESMInfo(u4Handle);
				#endif /* DMX_DBG_ESM_INFO*/
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			*ppvAUInfo = (void *)(&(paAUTable[u4Value]));
			if (((u32)(*ppvAUInfo) + sizeof(AU_SECTION) > g_arESMInst[u4Handle].prAUTable->ptrSa +
				g_arESMInst[u4Handle].prAUTable->u4Sz) ||
				((u32)(*ppvAUInfo) < g_arESMInst[u4Handle].prAUTable->ptrSa)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d(u4Handle: 0x%x) fail for AU's address error,")
					TEXT(" u4Value(%d), u4AUCount(%d), AUTable(SA(0x%lx), EA(0x%lx))\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4Value,
					g_arESMInst[u4Handle].prAUTable->u4AUCount,
					g_arESMInst[u4Handle].prAUTable->ptrSa,
					(g_arESMInst[u4Handle].prAUTable->ptrSa +
						g_arESMInst[u4Handle].prAUTable->u4Sz));
				#if DMX_DBG_ESM_INFO
				ESM_DumpESMInfo(u4Handle);
				#endif /* DMX_DBG_ESM_INFO*/
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
		}
		break;
	default:
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s (u4Handle: 0x%x) fail for unsupport eType(0x%x)\r\n"),
			DMX_FUNC_NAME, u4Handle, g_arESMInst[u4Handle].eType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
		break;
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT ESM_AUTableGetAUExtInfo(u32 u4Handle, u32 u4Value, void **pprAUExtInfo)
{
	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}
	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (ES_NONE == g_arESMInst[u4Handle].eType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d) hasn't been set type\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_SET_TYPE);
	}

	if (NULL == pprAUExtInfo) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d(u4Handle: 0x%x) fail for pprAUExtInfo is NULL\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*pprAUExtInfo = NULL;

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s AUTable FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	switch (g_arESMInst[u4Handle].eType) {
	case ES_V:
		break;
	case ES_A: {
			if (ESM_INVALID_ADDRESS != g_arESMInst[u4Handle].prAUExtTable->ptrSa) {
				AU_AUDIO_EXT_INFO_T *paAUExtTable =
					(AU_AUDIO_EXT_INFO_T *)g_arESMInst[u4Handle].prAUExtTable->ptrSa;
				if (u4Value >= g_arESMInst[u4Handle].prAUExtTable->u4AUCount) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
						TEXT("[ESM] %s line %d (u4Handle: 0x%x) fail for invalid")
						TEXT(" arg--u4Value(%d), u4AUCount(%d)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4Value,
						g_arESMInst[u4Handle].prAUExtTable->u4AUCount);
					DMX_ASSERT(FALSE);
					#if DMX_DBG_ESM_INFO
					ESM_DumpESMInfo(u4Handle);
					#endif /* DMX_DBG_ESM_INFO*/
					MM_RETURN(RET_DMX_PARAM_WRONG);
				}
				*pprAUExtInfo = (void *)(&(paAUExtTable[u4Value]));
			}
		}
		break;
	case ES_SP:
		break;
	case ES_SECTION:
		break;
	default:
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s (u4Handle: 0x%x) fail for unsupport eType(0x%x)\r\n"),
			DMX_FUNC_NAME, u4Handle, g_arESMInst[u4Handle].eType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
		break;
	}

	MM_RETURN(RET_DMX_OK);
}

/*/ Get next access unit index*/
MRESULT ESM_AUTableGetNextAUIdx(
	u32 u4Handle,
	u32 u4AUIdx,
	u32 u4AUIncCnt,
	u32 *pu4AUIdx)
{
	u32 u4TgtIdx = 0;
	u32 u4Cnt = 0;

	if (NULL == pu4AUIdx) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*pu4AUIdx = ESM_INVALID_INDEX;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prAUTable) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for the ESIH(%d) has no AUTable\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	if (u4AUIdx >= g_arESMInst[u4Handle].prAUTable->u4AUCount) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for AU's Index(%d) exceed ")
			TEXT("the max value(%d), (ESIH: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			u4AUIdx, g_arESMInst[u4Handle].prAUTable->u4AUCount, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	if (ES_NONE == g_arESMInst[u4Handle].eType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d) hasn't been set type\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_SET_TYPE);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s AUTable FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	u4Cnt = g_arESMInst[u4Handle].prAUTable->u4AUCount;

	if (u4AUIdx > u4Cnt) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d (u4Handle: 0x%x) Error for u4Value(%d) > AUTotalCnt(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4AUIdx, u4Cnt);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	u4TgtIdx = u4AUIdx + u4AUIncCnt;
	if (u4TgtIdx >= u4Cnt)
		u4TgtIdx -= u4Cnt;

	if (u4TgtIdx >= u4Cnt) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s(u4Handle: 0x%x) fail for u4TgtIdx(%d) >= u4Cnt(%d),")
			TEXT(" u4AUIdx(%d), u4AUIncCnt(%d)\r\n"),
			DMX_FUNC_NAME, u4Handle, u4TgtIdx, u4Cnt, u4AUIdx, u4AUIncCnt);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*pu4AUIdx = u4TgtIdx;

	MM_RETURN(RET_DMX_OK);
}

/*/ Get previous access unit index*/
MRESULT ESM_AUTableGetPrevAUIdx(
	u32 u4Handle,
	u32 u4AUIdx,
	u32 u4AUIncCnt,
	u32 *pu4AUIdx)
{
	u32 u4TgtIdx = 0;

	if (NULL == pu4AUIdx) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*pu4AUIdx = ESM_INVALID_INDEX;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}

	if (NULL == g_arESMInst[u4Handle].prAUTable) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for the ESIH(%d) has no AUTable\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (u4AUIdx >= g_arESMInst[u4Handle].prAUTable->u4AUCount) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for AU's Index(%d) exceed ")
			TEXT("the max value(%d), (ESIH: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			u4AUIdx, g_arESMInst[u4Handle].prAUTable->u4AUCount, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (ES_NONE == g_arESMInst[u4Handle].eType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d) hasn't been set type\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_SET_TYPE);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s AUTable FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/


	if (u4AUIdx == g_arESMInst[u4Handle].prAUTable->u4RdIdx)
		u4TgtIdx = ESM_INVALID_INDEX;
	else if (u4AUIncCnt > u4AUIdx) {
		u4TgtIdx = u4AUIdx + g_arESMInst[u4Handle].prAUTable->u4AUCount - u4AUIncCnt;

		if (u4TgtIdx >= g_arESMInst[u4Handle].prAUTable->u4AUCount) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for u4TgtIdx(%d) >= u4AUCount(%d),")
				TEXT(" u4AUIdx(%d), u4AUInCnt(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4TgtIdx,
				g_arESMInst[u4Handle].prAUTable->u4AUCount, u4AUIdx, u4AUIncCnt);
			#if DMX_DBG_ESM_INFO
			ESM_DumpESMInfo(u4Handle);
			#endif /* DMX_DBG_ESM_INFO*/
			MM_RETURN(RET_DMX_UNEXPECT);
		}
	} else {
		u4TgtIdx = u4AUIdx - u4AUIncCnt;

		if (u4TgtIdx >= g_arESMInst[u4Handle].prAUTable->u4AUCount) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for u4AUIdx(%d) - u4AUIncCnt(%d) >= u4AUCount(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4AUIdx, u4AUIncCnt,
				g_arESMInst[u4Handle].prAUTable->u4AUCount);
			#if DMX_DBG_ESM_INFO
			ESM_DumpESMInfo(u4Handle);
			#endif /* DMX_DBG_ESM_INFO*/
			MM_RETURN(RET_DMX_UNEXPECT);
		}
	}

	*pu4AUIdx = u4TgtIdx;

	MM_RETURN(RET_DMX_OK);
}


void ESM_RegistDemuxerCB(u32 u4Handle, ESM_FUNC_CB pfnCB, void *pvPrivate)
{
	CheckInit();
	CheckHandle(u4Handle);
	CheckAUTable(u4Handle);

	g_arESMInst[u4Handle].pvDemuxerCB = pfnCB;
	g_arESMInst[u4Handle].pvDemuxerCBPrivate = pvPrivate;
}

void ESM_RegistDecoderCB(u32 u4Handle,
	ESM_FUNC_CB pfnCB, void *pvPrivate)
{
	CheckInit();
	CheckHandle(u4Handle);
	CheckAUTable(u4Handle);

	g_arESMInst[u4Handle].pvDecoderCB = pfnCB;
	g_arESMInst[u4Handle].pvDecoderCBPrivate = pvPrivate;
}

MRESULT ESM_FifoSwitch(u32 u4Handle, bool fgUseSwFifo)
{
	u32 u4FifoSize = 0;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}
	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prFifo) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for no fifo (ESIH: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (ES_NONE == g_arESMInst[u4Handle].eType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s (u4Handle: 0x%x) fail for invalid eType\r\n"),
			DMX_FUNC_NAME, u4Handle);
		MM_RETURN(RET_DMX_NO_SET_TYPE);
	}

	/*/> Free the old Fifo, AUTable, AUExtTable*/
	if (ESM_INVALID_ADDRESS == g_arESMInst[u4Handle].prHwFifo->ptrSa) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s (u4Handle: 0x%x) fail for HW Fifo hasn't been set\r\n"),
			DMX_FUNC_NAME, u4Handle);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (fgUseSwFifo) {
		if ((ESM_INVALID_ADDRESS == g_arESMInst[u4Handle].prSwFifo->ptrSa) && /* liang luo sw*/
			(g_arESMInst[u4Handle].prFifo == g_arESMInst[u4Handle].prHwFifo)) {

			u32 ptrFifoSa = ESM_INVALID_ADDRESS;

			if (ES_A != g_arESMInst[u4Handle].eType) {
				g_arESMInst[u4Handle].prSwFifo->ptrSa	=
					g_arESMInst[u4Handle].prHwFifo->ptrSa;
				g_arESMInst[u4Handle].prSwFifo->ptrEa	=
					g_arESMInst[u4Handle].prHwFifo->ptrEa;
				g_arESMInst[u4Handle].prSwFifo->ptrRdPtr =
					g_arESMInst[u4Handle].prHwFifo->ptrRdPtr;
				g_arESMInst[u4Handle].prSwFifo->ptrWrPtr =
					g_arESMInst[u4Handle].prHwFifo->ptrWrPtr;
				g_arESMInst[u4Handle].prHwFifo->ptrSa = ESM_INVALID_ADDRESS;
				g_arESMInst[u4Handle].prHwFifo->ptrEa = ESM_INVALID_ADDRESS;
				g_arESMInst[u4Handle].prHwFifo->ptrRdPtr = ESM_INVALID_ADDRESS;
				g_arESMInst[u4Handle].prHwFifo->ptrWrPtr = ESM_INVALID_ADDRESS;
			} else {
				u4FifoSize = g_arESMInst[u4Handle].prHwFifo->ptrEa -
					g_arESMInst[u4Handle].prHwFifo->ptrSa;

			#ifdef __linux__
				DMX_NewHwAlignMemory(u4FifoSize, AFIFO_ALIGN, ptrFifoSa);
			#else
				DMX_NewHwAlignMemory(u4FifoSize, AFIFO_ALIGN, (void *)ptrFifoSa);
			#endif /* #ifdef __linux__*/

				if (0 == ptrFifoSa) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
						TEXT("[ESM] %s line %d (u4Handle: 0x%x)")
						TEXT(" fail in alloc fifo(no memory)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
					ESM_FreeESMem(u4Handle, FALSE);
					MM_RETURN(RET_DMX_NO_MEM);
				}

				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d (u4Handle: 0x%x, Type: %s) --")
					TEXT(" Alloc SW Fifo success, SA: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle,
					ESM_TYPESTR(g_arESMInst[u4Handle].eType),
					ptrFifoSa);

				/* set fifo memory*/
				g_arESMInst[u4Handle].prSwFifo->ptrSa	= ptrFifoSa;
				g_arESMInst[u4Handle].prSwFifo->ptrEa	= ptrFifoSa + u4FifoSize;
				g_arESMInst[u4Handle].prSwFifo->ptrRdPtr = ptrFifoSa;
				g_arESMInst[u4Handle].prSwFifo->ptrWrPtr = ptrFifoSa;
			}
		}

		g_arESMInst[u4Handle].prFifo = g_arESMInst[u4Handle].prSwFifo;

		if (ESM_INVALID_ADDRESS == g_arESMInst[u4Handle].prFifo->ptrSa) {
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d (u4Handle: 0x%x, Type: %s) fail for prFifo's ptrSa invalid\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle,
				ESM_TYPESTR(g_arESMInst[u4Handle].eType));
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d (u4Handle: 0x%x, Type: %s) --")
			TEXT(" Set use SwFifo(SA: 0x%x, EA: 0x%x, Sz: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle,
			ESM_TYPESTR(g_arESMInst[u4Handle].eType),
			g_arESMInst[u4Handle].prFifo->ptrSa,
			g_arESMInst[u4Handle].prFifo->ptrEa,
			(g_arESMInst[u4Handle].prFifo->ptrEa - g_arESMInst[u4Handle].prFifo->ptrSa));
	} else {
		if (g_arESMInst[u4Handle].prFifo == g_arESMInst[u4Handle].prSwFifo) {
			if (ES_A != g_arESMInst[u4Handle].eType) {
				g_arESMInst[u4Handle].prHwFifo->ptrSa	=
					g_arESMInst[u4Handle].prSwFifo->ptrSa;
				g_arESMInst[u4Handle].prHwFifo->ptrEa	=
					g_arESMInst[u4Handle].prSwFifo->ptrEa;
				g_arESMInst[u4Handle].prHwFifo->ptrRdPtr =
					g_arESMInst[u4Handle].prSwFifo->ptrRdPtr;
				g_arESMInst[u4Handle].prHwFifo->ptrWrPtr =
					g_arESMInst[u4Handle].prSwFifo->ptrWrPtr;
				g_arESMInst[u4Handle].prSwFifo->ptrSa = ESM_INVALID_ADDRESS;
				g_arESMInst[u4Handle].prSwFifo->ptrEa = ESM_INVALID_ADDRESS;
				g_arESMInst[u4Handle].prSwFifo->ptrRdPtr = ESM_INVALID_ADDRESS;
				g_arESMInst[u4Handle].prSwFifo->ptrWrPtr = ESM_INVALID_ADDRESS;
			}
		}

		g_arESMInst[u4Handle].prFifo = g_arESMInst[u4Handle].prHwFifo;

		if (ESM_INVALID_ADDRESS == g_arESMInst[u4Handle].prFifo->ptrSa) {
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d (u4Handle: 0x%x, Type: %s) fail for prFifo's ptrSa invalid\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle,
				ESM_TYPESTR(g_arESMInst[u4Handle].eType));
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d (u4Handle: 0x%x, Type: %s) --")
			TEXT(" Set use HwFifo(SA: 0x%x, EA: 0x%x, Sz: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle,
			ESM_TYPESTR(g_arESMInst[u4Handle].eType),
			g_arESMInst[u4Handle].prFifo->ptrSa,
			g_arESMInst[u4Handle].prFifo->ptrEa,
			(g_arESMInst[u4Handle].prFifo->ptrEa - g_arESMInst[u4Handle].prFifo->ptrSa));
	}


	MM_RETURN(RET_DMX_OK);
}

/*/ set fifo start address*/
MRESULT ESM_FifoSetMem(u32 u4Handle, u32 u4Size)
{
	uintptr_t ptrFifoSa = 0;
	uintptr_t ptrAUTSa = 0;
	u32 u4AUSz = 0;
	uintptr_t ptrAUExtTSa = 0;
	u32 u4AUExtSz = 0;
	u32 u4AUCount = 0;
	u32 u4NewSize = u4Size;
	bool   fgNotify = TRUE;

	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
		TEXT("[ESM] %s enter\r\n"), DMX_FUNC_NAME);

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}
	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prFifo) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for no fifo (ESIH: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (0 == u4Size) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s (u4Handle: 0x%x) fail for u4Size is 0\r\n"),
			DMX_FUNC_NAME, u4Handle);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	u4NewSize = DRV_ALIGN_MASK(u4NewSize, VFIFO_ALIGN);

	if (ES_NONE == g_arESMInst[u4Handle].eType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s (u4Handle: 0x%x) fail for invalid eType\r\n"),
			DMX_FUNC_NAME, u4Handle);
		MM_RETURN(RET_DMX_NO_SET_TYPE);
	}

	/*/> Free the old Fifo, AUTable, AUExtTable*/

	if (ESM_INVALID_ADDRESS != g_arESMInst[u4Handle].prHwFifo->ptrSa)
		/* free old memory*/
		ESM_FreeESMem(u4Handle, TRUE);

	if (ESM_INVALID_ADDRESS != g_arESMInst[u4Handle].prHwFifo->ptrSa) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d (u4Handle: 0x%x) fail")
			TEXT(" for Fifo SA != 0 after FIfo has been released\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		MM_RETURN(RET_DMX_NO_SET_TYPE);
	}

	/*/> Calculate AU Element Size, AUExt Element Size, and AU Count*/

	u4AUSz	  = 0;
	u4AUExtSz = 0;
	u4AUCount = 0;
	if (!ESM_AUTableCalcCount(u4Handle, &u4AUSz, &u4AUExtSz, &u4AUCount)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s (u4Handle: 0x%x) fail in ESM_AUTableCalcCount\r\n"),
			DMX_FUNC_NAME, u4Handle);
		ESM_FreeESMem(u4Handle, FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	/*/> Allocate Fifo*/

	ptrFifoSa = 0;

	/* memory allocate*/
	switch (g_arESMInst[u4Handle].eType) {
	case ES_V:
		#ifdef __linux__
		DMX_NewHwAlignMemoryEx(u4NewSize, VFIFO_ALIGN, ptrFifoSa);
		#else
		DMX_NewHwAlignMemoryEx(u4NewSize, VFIFO_ALIGN, (void *)ptrFifoSa);
		#endif /* #ifdef __linux__*/
		break;
	case ES_A: {
		#if AUDIO_USE_AUDPRIMARY_FIFO
		  AUD_POSINFO_T rAudPos;
			mm_memset(&rAudPos, 0, sizeof(rAudPos));

			if (0 != i4AudEsm_GetAudioCodecFifoInfo(AUD_FIFO_RPIMARY, &rAudPos)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d (u4Handle: 0x%x) fail in i4AudEsm_GetAudioCodecFifoInfo\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4NewSize);
				ESM_FreeESMem(u4Handle, FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
		  }

			if (rAudPos.ptrAfifoVirSA >= rAudPos.ptrAfifoVirEA) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d (u4Handle: 0x%x) fail for invalid audio fifo SA(0x%08x), EA(0x%08x)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					rAudPos.ptrAfifoVirSA, rAudPos.ptrAfifoVirEA);
				ESM_FreeESMem(u4Handle, FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
		  }

			ptrFifoSa = rAudPos.ptrAfifoVirSA;
			/*Reduce the primary afifo size for dual playback*/
			u4NewSize = rAudPos.ptrAfifoVirEA - rAudPos.ptrAfifoVirSA;

			#if DMX_MEM_PRINT_LINE_FUNCTION
			Add_MemMap((void *)ptrFifoSa,
				(void *)rAudPos.ptrAfifoSA,
				SPT_MEM_TYPE_EXT,
				u4NewSize,
				32,
				__func__,
				__LINE__);
			#else /* DMX_MEM_PRINT_LINE_FUNCTION*/
			Add_MemMap((void *)ptrFifoSa,
				(void *)rAudPos.ptrAfifoSA,
				SPT_MEM_TYPE_EXT,
				u4NewSize);
			#endif /* DMX_MEM_PRINT_LINE_FUNCTION*/

		#else /* AUDIO_USE_AUDPRIMARY_FIFO*/

			if (u4NewSize > 2 * 1024 * 1024) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d (u4Handle: 0x%x) fail in (u4NewSize(%d) > 2MB)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4NewSize);
				ESM_FreeESMem(u4Handle, FALSE);
				MM_RETURN(RET_DMX_UNEXPECT);
			}
			#ifdef __linux__
			DMX_NewHwAlignMemory(u4NewSize, AFIFO_ALIGN, ptrFifoSa);
			#else
			DMX_NewHwAlignMemory(u4NewSize, AFIFO_ALIGN, (void *)ptrFifoSa);
			#endif /* #ifdef __linux__*/
			
		#endif /* AUDIO_USE_AUDPRIMARY_FIFO*/
		}
		break;
	case ES_SP:
		#ifdef __linux__
		DMX_NewHwAlignMemory(u4NewSize, PSR_FIFO_ALIGNMENT, ptrFifoSa);
		#else
		DMX_NewHwAlignMemory(u4NewSize, PSR_FIFO_ALIGNMENT, (void *)ptrFifoSa);
		#endif /* #ifdef __linux__*/
		break;
	case ES_SECTION:
		#ifdef __linux__
		DMX_NewHwAlignMemoryEx(u4NewSize, VFIFO_ALIGN, ptrFifoSa);
		#else
		DMX_NewHwAlignMemoryEx(u4NewSize, VFIFO_ALIGN, (void *)ptrFifoSa);
		#endif /* #ifdef __linux__*/
		break;
	default:
		ptrFifoSa = 0;
		break;
	}

	if (0 == ptrFifoSa) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d (u4Handle: 0x%x) fail in alloc fifo(no memory)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		ESM_FreeESMem(u4Handle, FALSE);
		MM_RETURN(RET_DMX_NO_MEM);
	}

	/* set fifo memory*/
	g_arESMInst[u4Handle].prHwFifo->ptrSa	= ptrFifoSa;
	g_arESMInst[u4Handle].prHwFifo->ptrEa	= ptrFifoSa + u4NewSize;
	g_arESMInst[u4Handle].prHwFifo->ptrRdPtr = g_arESMInst[u4Handle].prHwFifo->ptrSa;
	g_arESMInst[u4Handle].prHwFifo->ptrWrPtr = g_arESMInst[u4Handle].prHwFifo->ptrSa;

	g_arESMInst[u4Handle].prSwFifo->ptrSa	= ESM_INVALID_ADDRESS;
	g_arESMInst[u4Handle].prSwFifo->ptrEa	= ESM_INVALID_ADDRESS;
	g_arESMInst[u4Handle].prSwFifo->ptrRdPtr = ESM_INVALID_ADDRESS;
	g_arESMInst[u4Handle].prSwFifo->ptrWrPtr = ESM_INVALID_ADDRESS;

	/* default set prFifo pointer to the HwFifo*/
	g_arESMInst[u4Handle].prFifo = g_arESMInst[u4Handle].prHwFifo;

	/*/> Allocate AU Ext Table Memory*/

	ptrAUExtTSa = 0;
	if (0 < u4AUExtSz) {
		#ifdef __linux__
		DMX_NewHwMemory((u4AUExtSz * u4AUCount), ptrAUExtTSa);
		#else
		DMX_NewHwMemory((u4AUExtSz * u4AUCount), (void *)ptrAUExtTSa);
		#endif /* #ifdef __linux__*/
		if (0 == ptrAUExtTSa) {
			ESM_FreeESMem(u4Handle, FALSE);
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s (u4Handle: 0x%x) fail in alloc AUTable (no memory)\r\n"),
				DMX_FUNC_NAME, u4Handle);
			MM_RETURN(RET_DMX_NO_MEM);
		}
		g_arESMInst[u4Handle].prAUExtTable->ptrSa = ptrAUExtTSa;
		g_arESMInst[u4Handle].prAUExtTable->u4Sz = u4AUExtSz * u4AUCount;
		g_arESMInst[u4Handle].prAUExtTable->u4AUCount = u4AUCount;
		g_arESMInst[u4Handle].prAUExtTable->u4RdIdx = 0;
		g_arESMInst[u4Handle].prAUExtTable->u4WrIdx = 0;
		if ((ESM_INVALID_ADDRESS != g_arESMInst[u4Handle].prAUExtTable->ptrSa) &&
			(0 < g_arESMInst[u4Handle].prAUExtTable->u4Sz)) {
			dmx_memset((void *)(g_arESMInst[u4Handle].prAUExtTable->ptrSa),
				(u8)0x00, g_arESMInst[u4Handle].prAUExtTable->u4Sz);
		}
	} else {
		g_arESMInst[u4Handle].prAUExtTable->ptrSa = ESM_INVALID_ADDRESS;
		g_arESMInst[u4Handle].prAUExtTable->u4Sz = 0;
		g_arESMInst[u4Handle].prAUExtTable->u4AUCount = ESM_INVALID_COUNT;
		g_arESMInst[u4Handle].prAUExtTable->u4RdIdx = ESM_INVALID_INDEX;
		g_arESMInst[u4Handle].prAUExtTable->u4WrIdx = ESM_INVALID_INDEX;
	}

	/*/> Allocate AU Table Memory*/

	ptrAUTSa = 0;
	#ifdef __linux__
	DMX_NewHwMemory((u4AUSz * u4AUCount), ptrAUTSa);
	#else
	DMX_NewHwMemory((u4AUSz * u4AUCount), (void *)ptrAUTSa);
	#endif /* #ifdef __linux__*/

	if (0 == ptrAUTSa) {
		ESM_FreeESMem(u4Handle, FALSE);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s (u4Handle: 0x%x) fail in alloc AUTable (no memory)\r\n"),
			DMX_FUNC_NAME, u4Handle);
		MM_RETURN(RET_DMX_NO_MEM);
	}

	/* set au table memory*/
	g_arESMInst[u4Handle].prAUTable->ptrSa = ptrAUTSa;
	g_arESMInst[u4Handle].prAUTable->u4Sz = u4AUSz * u4AUCount;
	g_arESMInst[u4Handle].prAUTable->u4AUCount = u4AUCount;
	g_arESMInst[u4Handle].prAUTable->u4RdIdx = 0;
	g_arESMInst[u4Handle].prAUTable->u4WrIdx = 0;

	if ((ESM_INVALID_ADDRESS != g_arESMInst[u4Handle].prAUTable->ptrSa) &&
		(0 < g_arESMInst[u4Handle].prAUTable->u4Sz)) {
		dmx_memset((void *)(g_arESMInst[u4Handle].prAUTable->ptrSa),
			(u8)0x00, g_arESMInst[u4Handle].prAUTable->u4Sz);
	}

	if ((fgNotify) && (NULL != g_arESMInst[u4Handle].pvDecoderCB)) {
		g_arESMInst[u4Handle].pvDecoderCB(CBE_FIFO_SET, NULL,
			g_arESMInst[u4Handle].pvDecoderCBPrivate);
	}

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
		TEXT("[ESM] %s (%s) success, size: %d\r\n"),
		DMX_FUNC_NAME, ESM_TYPESTR(g_arESMInst[u4Handle].eType), u4Size);

	MM_RETURN(RET_DMX_OK);
}

/*/ get fifo infomation*/
MRESULT ESM_FifoGetInfo(u32 u4Handle, void **pprFifoInfo)
{
	DMX_FIFO_INFO_T  *prFifo = NULL;

	if (NULL == pprFifoInfo)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	*pprFifoInfo = NULL;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prFifo) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for no fifo (ESIH: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	prFifo = g_arESMInst[u4Handle].prFifo;

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (!ESM_CheckFifoValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	*pprFifoInfo = (void *)(prFifo);

	MM_RETURN(RET_DMX_OK);
}

/*/ get fifo start address*/
MRESULT ESM_FifoGetSA(u32 u4Handle, uintptr_t *pptrSa)
{
	uintptr_t ptrSa = 0;

	if (NULL == pptrSa)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	*pptrSa = ESM_INVALID_ADDRESS;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}
	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prFifo) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for no fifo (ESIH: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (!ESM_CheckFifoValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	ptrSa = g_arESMInst[u4Handle].prFifo->ptrSa;
	*pptrSa = ptrSa;

	MM_RETURN(RET_DMX_OK);
}

/*/ get fifo end address*/
MRESULT ESM_FifoGetEA(u32 u4Handle, uintptr_t *pptrEa)
{
	uintptr_t ptrEa = 0;

	if (NULL == pptrEa)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	*pptrEa = ESM_INVALID_ADDRESS;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prFifo) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for no fifo (ESIH: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (!ESM_CheckFifoValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	ptrEa = g_arESMInst[u4Handle].prFifo->ptrEa;
	*pptrEa = ptrEa;

	MM_RETURN(RET_DMX_OK);
}

/*/ set fifo write pointer*/
MRESULT ESM_FifoSetWrPtr(u32 u4Handle, uintptr_t ptrValue)
{
	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prFifo) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for no fifo (ESIH: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (!ESM_CheckFifoValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	if ((ptrValue < g_arESMInst[u4Handle].prFifo->ptrSa) ||
		(ptrValue >= g_arESMInst[u4Handle].prFifo->ptrEa)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s(u4Handle: 0x%x) fail for invalid param,")
			TEXT(" ToSetWPtr(0x%x)exceed fifo size(Sa: 0x%x, Ea: 0x%x)\r\n"),
			DMX_FUNC_NAME, u4Handle, ptrValue,
			g_arESMInst[u4Handle].prFifo->ptrSa,
			g_arESMInst[u4Handle].prFifo->ptrEa);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	g_arESMInst[u4Handle].prFifo->ptrWrPtr = ptrValue;

	if (NULL != g_arESMInst[u4Handle].pvDecoderCB) {
		g_arESMInst[u4Handle].pvDecoderCB(CBE_FIFO_IN, NULL,
			g_arESMInst[u4Handle].pvDecoderCBPrivate);
	}

	MM_RETURN(RET_DMX_OK);
}

/*/ get fifo write pointer*/
MRESULT ESM_FifoGetWrPtr(u32 u4Handle, uintptr_t *pptrWrPtr)
{
	u32 ptrWrPtr = 0;

	if (NULL == pptrWrPtr)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	*pptrWrPtr = ESM_INVALID_ADDRESS;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prFifo) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for no fifo (ESIH: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (!ESM_CheckFifoValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	ptrWrPtr = g_arESMInst[u4Handle].prFifo->ptrWrPtr;

	*pptrWrPtr = ptrWrPtr;

	MM_RETURN(RET_DMX_OK);
}

MRESULT ESM_FifoIncWrPtr(u32 u4Handle, u32 u4Value)
{
	u32 u4NewWrPtr = 0;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prFifo) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for no fifo (ESIH: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (!ESM_CheckFifoValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	u4NewWrPtr = g_arESMInst[u4Handle].prFifo->ptrWrPtr + u4Value;

	if (u4NewWrPtr >= g_arESMInst[u4Handle].prFifo->ptrEa) {
		u4NewWrPtr -= (g_arESMInst[u4Handle].prFifo->ptrEa -
			g_arESMInst[u4Handle].prFifo->ptrSa);
	}

	if ((u4NewWrPtr < g_arESMInst[u4Handle].prFifo->ptrSa) ||
		(u4NewWrPtr >= g_arESMInst[u4Handle].prFifo->ptrEa)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d (u4Handle: 0x%x) fail for invalid param,")
			TEXT(" u4NewWrPtr(0x%x) exceed fifo size(Sa: 0x%x, Ea: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4NewWrPtr,
			g_arESMInst[u4Handle].prFifo->ptrSa,
			g_arESMInst[u4Handle].prFifo->ptrEa);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	g_arESMInst[u4Handle].prFifo->ptrWrPtr = u4NewWrPtr;

	if (NULL != g_arESMInst[u4Handle].pvDecoderCB) {
		g_arESMInst[u4Handle].pvDecoderCB(CBE_FIFO_IN, NULL,
			g_arESMInst[u4Handle].pvDecoderCBPrivate);
	}

	MM_RETURN(RET_DMX_OK);
}

/*/ set fifo read pointer*/
MRESULT ESM_FifoSetRdPtr(u32 u4Handle, u32 u4Value, bool fgFF)
{
	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prFifo) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for no fifo (ESIH: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (!ESM_CheckFifoValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	if ((u4Value < g_arESMInst[u4Handle].prFifo->ptrSa) ||
		(u4Value >= g_arESMInst[u4Handle].prFifo->ptrEa)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s(u4Handle: 0x%x) fail for invalid param,")
			TEXT(" u4Value: 0x%x, ptrSa: 0x%x, ptrEa: 0x%x\r\n"),
			DMX_FUNC_NAME, u4Handle, u4Value,
			g_arESMInst[u4Handle].prFifo->ptrSa,
			g_arESMInst[u4Handle].prFifo->ptrEa);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	g_arESMInst[u4Handle].prFifo->ptrRdPtr = u4Value;

	if (NULL != g_arESMInst[u4Handle].pvDemuxerCB) {
		g_arESMInst[u4Handle].pvDemuxerCB(CBE_FIFO_OUT, NULL,
			g_arESMInst[u4Handle].pvDemuxerCBPrivate);
	}

	MM_RETURN(RET_DMX_OK);
}

/*/ get fifo read pointer*/
MRESULT ESM_FifoGetRdPtr(u32 u4Handle, uintptr_t *pptrRdPtr)
{
	u32 ptrRdPtr = 0;

	if (NULL == pptrRdPtr)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	*pptrRdPtr = ESM_INVALID_ADDRESS;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prFifo) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for no fifo (ESIH: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (!ESM_CheckFifoValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	ptrRdPtr = g_arESMInst[u4Handle].prFifo->ptrRdPtr;

	*pptrRdPtr = ptrRdPtr;

	MM_RETURN(RET_DMX_OK);
}

/*/ get fifo read pointer*/
MRESULT ESM_FifoGetAvailDataSize(u32 u4Handle, u32 *pu4AvailSz)
{
	u32 u4Size = 0;
	u32 u4Rp, u4Wp;
#if AUDIO_USE_AUDPRIMARY_FIFO
	u32 u4Rp2, u4Size2;
#endif /* AUDIO_USE_AUDPRIMARY_FIFO*/

	if (NULL == pu4AvailSz)
		MM_RETURN(RET_DMX_PARAM_WRONG);
	*pu4AvailSz = ESM_INVALID_SIZE;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prFifo) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for no fifo (ESIH: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (!ESM_CheckFifoValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	u4Wp = g_arESMInst[u4Handle].prFifo->ptrWrPtr;
	u4Rp = g_arESMInst[u4Handle].prFifo->ptrRdPtr;

	u4Size = DMX_DATASIZE(u4Rp, u4Wp, (g_arESMInst[u4Handle].prFifo->ptrEa -
			g_arESMInst[u4Handle].prFifo->ptrSa));

	if (ES_A == g_arESMInst[u4Handle].eType) {
		if (DMX_INVALID_UINT32 == g_arESMInst[u4Handle].u4StmCodec) {
			MRESULT mrRet = RET_DMX_OK;

			mrRet = ESM_GetStmCodec(u4Handle, NULL);

			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d (u4Handle: 0x%x) fail in")
					TEXT(" ESM_GetStmCodec(..), mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
			}
		}

		if ((DMX_INVALID_UINT32 == g_arESMInst[u4Handle].u4StmCodec) ||
			(0 == (g_arESMInst[u4Handle].u8DecSendBufMask & (((u64)1) <<
				(g_arESMInst[u4Handle].u4StmCodec))))) {
			if (g_arESMInst[u4Handle].prFifo == g_arESMInst[u4Handle].prHwFifo) {
#if AUDIO_USE_AUDPRIMARY_FIFO
				AUD_POSINFO_T rAudPos;
				mm_memset(&rAudPos, 0, sizeof(rAudPos));

				if (0 != i4AudEsm_GetAudioCodecFifoInfo(AUD_FIFO_RPIMARY, &rAudPos)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
						TEXT("[ESM] %s line %d fail in i4AudEsm_GetAudioCodecFifoInfo\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
#if DMX_DBG_ESM_INFO
					ESM_DumpESMInfo(u4Handle);
#endif /* DMX_DBG_ESM_INFO*/
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_UNEXPECT);
				}

				u4Rp2 = rAudPos.ptrAfifoRPtr + g_arESMInst[u4Handle].prFifo->ptrSa;

				u4Size2 = DMX_DATASIZE(u4Rp2, u4Wp, (g_arESMInst[u4Handle].prFifo->ptrEa -
					g_arESMInst[u4Handle].prFifo->ptrSa));

				if (_fgAudDrvInClearStatus) {
					if ((u4Rp == u4Rp2) &&
						(u4Rp2 == g_arESMInst[u4Handle].prFifo->ptrSa)) {
						_fgAudDrvInClearStatus = FALSE;
						DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
							TEXT("[ESM] %s line %d -- Audio DSP RP==0 Now\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
						/*if (ESM_AUTableAvailCount(u4Handle) > 0) {
							g_arESMInst[u4Handle].pvDecoderCB(CBE_AU_IN, NULL,
								g_arESMInst[u4Handle].pvDecoderCBPrivate);
						}*/
						(ESM_AUTableAvailCount(u4Handle) > 0) ?
								g_arESMInst[u4Handle].pvDecoderCB(CBE_AU_IN, NULL,
								g_arESMInst[u4Handle].pvDecoderCBPrivate) : 1;
					}
				}

				if (!_fgAudDrvInClearStatus)
					u4Size = u4Size2;
#else
				if (_fgAudDrvInClearStatus) {
					if (u4Rp == g_arESMInst[u4Handle].prFifo->ptrSa) {
						_fgAudDrvInClearStatus = FALSE;
						/*if (ESM_AUTableAvailCount(u4Handle) > 0) {
							g_arESMInst[u4Handle].pvDecoderCB(CBE_AU_IN, NULL,
								g_arESMInst[u4Handle].pvDecoderCBPrivate);
						}*/
					(ESM_AUTableAvailCount(u4Handle) > 0) ?
							g_arESMInst[u4Handle].pvDecoderCB(CBE_AU_IN, NULL,
							g_arESMInst[u4Handle].pvDecoderCBPrivate) : 1;
					}
				}

#endif /* AUDIO_USE_AUDPRIMARY_FIFO*/
			}
		}

		if (g_arESMInst[u4Handle].prFifo == g_arESMInst[u4Handle].prSwFifo) {
			if (_fgAudDrvInClearStatus) {
				_fgAudDrvInClearStatus = FALSE;
				if (ESM_AUTableAvailCount(u4Handle) > 0) {
					g_arESMInst[u4Handle].pvDecoderCB(CBE_AU_IN, NULL,
						g_arESMInst[u4Handle].pvDecoderCBPrivate);
				}
			}
		}

		if (!_fgAudDrvInClearStatus) {
			g_arESMInst[u4Handle].pvDecoderCB(CBE_SYNC_READPTR, NULL,
				g_arESMInst[u4Handle].pvDecoderCBPrivate);
		}
	}

	if (u4Size > (g_arESMInst[u4Handle].prFifo->ptrEa - g_arESMInst[u4Handle].prFifo->ptrSa)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d (u4Handle: 0x%x) fail for Size(%d) > FifoSz(SA: 0x%lx, EA: 0x%lx)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4Size,
			g_arESMInst[u4Handle].prFifo->ptrSa,
			g_arESMInst[u4Handle].prFifo->ptrEa);
		ESM_PrintFifoInfo(u4Handle);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*pu4AvailSz = u4Size;

	MM_RETURN(RET_DMX_OK);
}

/*/ get fifo read pointer*/
MRESULT ESM_FifoGetSelfAvailDataSize(u32 u4Handle, u32 *pu4AvailSz)
{
	u32 u4Size = 0;
	u32 u4Rp, u4Wp;

	if (NULL == pu4AvailSz)
		MM_RETURN(RET_DMX_PARAM_WRONG);
	*pu4AvailSz = ESM_INVALID_SIZE;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prFifo) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for no fifo (ESIH: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (!ESM_CheckFifoValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	u4Wp = g_arESMInst[u4Handle].prFifo->ptrWrPtr;
	u4Rp = g_arESMInst[u4Handle].prFifo->ptrRdPtr;

	u4Size = DMX_DATASIZE(u4Rp, u4Wp, (g_arESMInst[u4Handle].prFifo->ptrEa -
			g_arESMInst[u4Handle].prFifo->ptrSa));

	if (u4Size > (g_arESMInst[u4Handle].prFifo->ptrEa - g_arESMInst[u4Handle].prFifo->ptrSa)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d (u4Handle: 0x%x) fail for Size(%d) > FifoSz(SA: 0x%lx, EA: 0x%lx)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, u4Size,
			g_arESMInst[u4Handle].prFifo->ptrSa,
			g_arESMInst[u4Handle].prFifo->ptrEa);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*pu4AvailSz = u4Size;

	MM_RETURN(RET_DMX_OK);
}

/*/ Clear fifo, it will set read/write pointer to fifo start address.*/
void ESM_FifoClear(u32 u4Handle)
{
	CheckInit();
	CheckHandle(u4Handle);
	CheckFifo(u4Handle);

	if (NULL != g_arESMInst[u4Handle].pvDecoderCB) {
		g_arESMInst[u4Handle].pvDecoderCB(CBE_FIFO_FLUSH, NULL,
			g_arESMInst[u4Handle].pvDecoderCBPrivate);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		return;
	}

	if (!ESM_CheckFifoValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		return;
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	g_arESMInst[u4Handle].prAUTable->u4RdIdx = 0;
	g_arESMInst[u4Handle].prAUTable->u4WrIdx = 0;

	if ((ESM_INVALID_ADDRESS != g_arESMInst[u4Handle].prAUTable->ptrSa) &&
		(0 < g_arESMInst[u4Handle].prAUTable->u4Sz)) {
		dmx_memset((void *)(g_arESMInst[u4Handle].prAUTable->ptrSa), (u8)0x00,
			g_arESMInst[u4Handle].prAUTable->u4Sz);
	}

	if (NULL != g_arESMInst[u4Handle].prAUExtTable) {
		g_arESMInst[u4Handle].prAUExtTable->u4RdIdx = 0;
		g_arESMInst[u4Handle].prAUExtTable->u4WrIdx = 0;
		if ((ESM_INVALID_ADDRESS != g_arESMInst[u4Handle].prAUExtTable->ptrSa) &&
			(0 < g_arESMInst[u4Handle].prAUExtTable->u4Sz)) {
			dmx_memset((void *)(g_arESMInst[u4Handle].prAUExtTable->ptrSa), (u8)0x00,
				g_arESMInst[u4Handle].prAUExtTable->u4Sz);
		}
	}

	g_arESMInst[u4Handle].prHwFifo->ptrRdPtr =
		g_arESMInst[u4Handle].prHwFifo->ptrSa;
	g_arESMInst[u4Handle].prHwFifo->ptrWrPtr =
		g_arESMInst[u4Handle].prHwFifo->ptrSa;

	g_arESMInst[u4Handle].prSwFifo->ptrRdPtr =
		g_arESMInst[u4Handle].prSwFifo->ptrSa;
	g_arESMInst[u4Handle].prSwFifo->ptrWrPtr =
		g_arESMInst[u4Handle].prSwFifo->ptrSa;

#if AUDIO_USE_AUDPRIMARY_FIFO
	if (ES_A == g_arESMInst[u4Handle].eType)
		_fgAudDrvInClearStatus = TRUE;
#endif /* AUDIO_USE_AUDPRIMARY_FIFO*/

	g_arESMInst[u4Handle].u4StmCodec = DMX_INVALID_UINT32;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
		TEXT("[ESM] %s (%s) Success\r\n"),
		DMX_FUNC_NAME, ESM_TYPESTR(g_arESMInst[u4Handle].eType));
}

void ESM_CheckFifoClearStatus(u32 u4Handle)
{
#if AUDIO_USE_AUDPRIMARY_FIFO
	CheckInit();
	CheckHandle(u4Handle);
	CheckFifo(u4Handle);

	if (ES_A == g_arESMInst[u4Handle].eType) {
		AUD_POSINFO_T rAudPos;
		mm_memset(&rAudPos, 0, sizeof(rAudPos));

		if (0 != i4AudEsm_GetAudioCodecFifoInfo(AUD_FIFO_RPIMARY, &rAudPos)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail in i4AudEsm_GetAudioCodecFifoInfo\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return;
		}

		if (0 != rAudPos.ptrAfifoRPtr)
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s (Audio) -- FATAL WARNING: Audio DSP Primary Fifo's RP(0x%x)")
				TEXT(" or WP(0x%x) hasn't been set to be 0\r\n"),
				DMX_FUNC_NAME,
				rAudPos.ptrAfifoRPtr,
				rAudPos.ptrAfifoWPtr);
	}
#endif /* AUDIO_USE_AUDPRIMARY_FIFO*/

}

MRESULT ESM_CheckAudDrvStatus(u32 u4Handle)
{
	u32 u4Rp;
#if AUDIO_USE_AUDPRIMARY_FIFO
	u32 u4Rp2;
#endif /* AUDIO_USE_AUDPRIMARY_FIFO*/

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (NULL == g_arESMInst[u4Handle].prFifo) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for no fifo (ESIH: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (!ESM_CheckFifoValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s Fifo FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/


	u4Rp = g_arESMInst[u4Handle].prFifo->ptrRdPtr;

	if (ES_A == g_arESMInst[u4Handle].eType) {
		if (DMX_INVALID_UINT32 == g_arESMInst[u4Handle].u4StmCodec) {
			MRESULT mrRet = RET_DMX_OK;

			mrRet = ESM_GetStmCodec(u4Handle, NULL);

			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d (u4Handle: 0x%x) fail in")
					TEXT(" ESM_GetStmCodec(..), mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
			}
		}

		if ((DMX_INVALID_UINT32 == g_arESMInst[u4Handle].u4StmCodec) ||
			(0 == (g_arESMInst[u4Handle].u8DecSendBufMask & (((u64)1) <<
				(g_arESMInst[u4Handle].u4StmCodec))))) {
			if (g_arESMInst[u4Handle].prFifo == g_arESMInst[u4Handle].prHwFifo) {
#if AUDIO_USE_AUDPRIMARY_FIFO
				AUD_POSINFO_T rAudPos;
				mm_memset(&rAudPos, 0, sizeof(rAudPos));

				if (0 != i4AudEsm_GetAudioCodecFifoInfo(AUD_FIFO_RPIMARY, &rAudPos)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
						TEXT("[ESM] %s fail in i4AudEsm_GetAudioCodecFifoInfo(u4Handle: %d)\r\n"),
						DMX_FUNC_NAME, u4Handle);
#if DMX_DBG_ESM_INFO
					ESM_DumpESMInfo(u4Handle);
#endif /* DMX_DBG_ESM_INFO*/
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_UNEXPECT);
				}

				u4Rp2 = rAudPos.ptrAfifoRPtr +	g_arESMInst[u4Handle].prFifo->ptrSa;
				if ((u4Rp == u4Rp2) &&
					(u4Rp2 == g_arESMInst[u4Handle].prFifo->ptrSa)) {
					_fgAudDrvInClearStatus = FALSE;
					DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
						TEXT("[ESM] %s line %d -- Audio DSP RP==0 Now\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
					g_arESMInst[u4Handle].pvDecoderCB(CBE_SYNC_READPTR, NULL,
						g_arESMInst[u4Handle].pvDecoderCBPrivate);
				}
#else
				if (u4Rp == g_arESMInst[u4Handle].prFifo->ptrSa) {
					_fgAudDrvInClearStatus = FALSE;
					DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
						TEXT("[ESM] %s line %d -- Audio DSP RP==0 Now\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
					g_arESMInst[u4Handle].pvDecoderCB(CBE_SYNC_READPTR, NULL,
						g_arESMInst[u4Handle].pvDecoderCBPrivate);
				}
#endif /* AUDIO_USE_AUDPRIMARY_FIFO*/
			}
		}

		if (g_arESMInst[u4Handle].prFifo == g_arESMInst[u4Handle].prSwFifo) {
			g_arESMInst[u4Handle].pvDecoderCB(CBE_SYNC_READPTR, NULL,
				g_arESMInst[u4Handle].pvDecoderCBPrivate);
		}
	}

	MM_RETURN(RET_DMX_OK);
}

/* **********************************************************************/
/* Export function*/
/* **********************************************************************/
MRESULT ESM_Init(void)
{
	MRESULT mrRet = RET_DMX_OK;

	if (g_fgESMInit) {
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s exit for ESM already init\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}
	smp_mb();

	/* create a semaphore*/
	if (NULL == g_hESMSema) {
		mrRet = dmx_sema_create(&g_hESMSema,
			DMX_SEMA_TYPE_MUTEX, DMX_SEMA_STATE_UNLOCK);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s fail in dmx_sema_create, ErrCode: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_GET_LASTERR);
			MM_RETURN(mrRet);
		}
	}

	smp_mb();
	{/* init elementary stream interface table*/
		u32 u4Idx = 0;

		for (u4Idx = 0; u4Idx < MAX_ESICOUNT; u4Idx++) {
			g_arESMInst[u4Idx].eType = ES_NONE;
			g_arESMInst[u4Idx].fgUsed = FALSE;
			g_arESMInst[u4Idx].prFifo = NULL;
			g_arESMInst[u4Idx].prAUTable = NULL;
			g_arESMInst[u4Idx].prAUExtTable = NULL;
			g_arESMInst[u4Idx].pvDemuxerCBPrivate = NULL;
			g_arESMInst[u4Idx].pvDemuxerCB = (ESM_FUNC_CB)NULL;
			g_arESMInst[u4Idx].pvDecoderCBPrivate = NULL;
			g_arESMInst[u4Idx].pvDecoderCB = (ESM_FUNC_CB)NULL;
			g_arESMInst[u4Idx].u4FilterType = SPT_DATA_UNDEFINE;
			g_arESMInst[u4Idx].u4FilterId = DMX_INVALID_UINT32;
			g_arESMInst[u4Idx].pvSptHdl = NULL;
			g_arESMInst[u4Idx].u2Ref = 0;
		}
	}
	smp_mb();

#if AUDIO_USE_AUDPRIMARY_FIFO

	_fgAudDrvInClearStatus = FALSE;
	smp_mb();
#endif /* AUDIO_USE_AUDPRIMARY_FIFO*/
	smp_mb();

	/* turn on init flag*/
	g_fgESMInit = TRUE;

	MM_RETURN(RET_DMX_OK);
}

void ESM_Uninit(void)
{
	u32 u4Idx = 0;

	if (!g_fgESMInit) {
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s exit for already UnInit\r\n"),
			DMX_FUNC_NAME);
		return;
	}
	smp_mb();

	for (u4Idx = 0; u4Idx < MAX_ESICOUNT; u4Idx++)
		ESM_FreeES(u4Idx);
	smp_mb();

	for (u4Idx = 0; u4Idx < MAX_ESICOUNT; u4Idx++) {
		g_arESMInst[u4Idx].eType = ES_NONE;
		g_arESMInst[u4Idx].fgUsed = FALSE;
		g_arESMInst[u4Idx].prFifo = NULL;
		g_arESMInst[u4Idx].prAUTable = NULL;
		g_arESMInst[u4Idx].prAUExtTable = NULL;
		g_arESMInst[u4Idx].pvDemuxerCBPrivate = NULL;
		g_arESMInst[u4Idx].pvDemuxerCB = (ESM_FUNC_CB)NULL;
		g_arESMInst[u4Idx].pvDecoderCBPrivate = NULL;
		g_arESMInst[u4Idx].pvDecoderCB = (ESM_FUNC_CB)NULL;
		g_arESMInst[u4Idx].u4FilterType = SPT_DATA_UNDEFINE;
		g_arESMInst[u4Idx].u4FilterId = DMX_INVALID_UINT32;
		g_arESMInst[u4Idx].pvSptHdl = NULL;
		g_arESMInst[u4Idx].u2Ref = 0;
	}
	smp_mb();

	/* destroy semaphore*/
	if (NULL != g_hESMSema) {
		dmx_sema_delete(g_hESMSema);
		g_hESMSema = NULL;
	}
	smp_mb();

#if AUDIO_USE_AUDPRIMARY_FIFO
	smp_mb();

	_fgAudDrvInClearStatus = FALSE;

#endif /* AUDIO_USE_AUDPRIMARY_FIFO*/
	smp_mb();

	/* turn off init flag*/
	g_fgESMInit = FALSE;
}

MRESULT ESM_Create(
	void * pvSptHdl,
	u32  u4FilterType,				 /*/< [IN] Filter component type*/
	u32  u4FilterId,						 /*/< [IN] Filter component ID*/
	u64  u8DecSendBufMask,				 /*/< [IN] SW Dec Mask*/

	u32 *pu4Handle						  /*/< [OUT] interface handle*/
)
{
	u32	u4Handle = ESM_INVALID_HANDLE;
	u32	u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	*pu4Handle = ESM_INVALID_HANDLE;

	/*check ESI whether exist*/
	for (u4Idx = 0; u4Idx < MAX_ESICOUNT; u4Idx++) {
		if ((g_arESMInst[u4Idx].fgUsed) &&
			(u4FilterId == g_arESMInst[u4Idx].u4FilterId) &&
			(u4FilterType == g_arESMInst[u4Idx].u4FilterType)) {
			u4Handle = u4Idx;
			break;
		}
	}

	if (ESM_INVALID_HANDLE == u4Handle) {
		/* find a empty handle*/
		for (u4Idx = 0; u4Idx < MAX_ESICOUNT; u4Idx++) {
			if (!g_arESMInst[u4Idx].fgUsed) {
				u4Handle = u4Idx;
				break;
			}
		}

		if (u4Idx >= MAX_ESICOUNT) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for no unused ESM instance\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return ESM_INVALID_INDEX;
		}

		/* allocate memory for fifo information*/
		DMX_NewMemory(sizeof(DMX_FIFO_INFO_T), g_arESMInst[u4Idx].prSwFifo);
		if (NULL == g_arESMInst[u4Idx].prSwFifo) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s fail in alloc SwfifoInfo(no memory)\r\n"),
				DMX_FUNC_NAME);
			mrRet = RET_DMX_NO_MEM;
			goto ESMCREATEERR;
		}

		/* allocate memory for fifo information*/
		DMX_NewMemory(sizeof(DMX_FIFO_INFO_T), g_arESMInst[u4Idx].prHwFifo);
		if (NULL == g_arESMInst[u4Idx].prHwFifo) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s fail in alloc HwfifoInfo(no memory)\r\n"),
				DMX_FUNC_NAME);
			mrRet = RET_DMX_NO_MEM;
			goto ESMCREATEERR;
		}

		/* allocate memory for AU table*/
		DMX_NewMemory(sizeof(DMX_AUTABLE_INFO_T), g_arESMInst[u4Idx].prAUTable);
		if (NULL == g_arESMInst[u4Idx].prAUTable) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s fail in alloc DMX_AUTABLE_INFO_T(no memory)\r\n"),
				DMX_FUNC_NAME);
			mrRet = RET_DMX_NO_MEM;
			goto ESMCREATEERR;
		}

		DMX_NewMemory(sizeof(DMX_AUEXTTABLE_INFO_T),
			g_arESMInst[u4Idx].prAUExtTable);
		if (NULL == g_arESMInst[u4Idx].prAUExtTable) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s fail in alloc DMX_AUTABLE_INFO_T(no memory)\r\n"),
				DMX_FUNC_NAME);
			mrRet = RET_DMX_NO_MEM;
			goto ESMCREATEERR;
		}

		g_arESMInst[u4Idx].fgUsed = TRUE;

		g_arESMInst[u4Idx].prHwFifo->ptrSa = ESM_INVALID_ADDRESS;
		g_arESMInst[u4Idx].prHwFifo->ptrEa = ESM_INVALID_ADDRESS;
		g_arESMInst[u4Idx].prHwFifo->ptrRdPtr = ESM_INVALID_ADDRESS;
		g_arESMInst[u4Idx].prHwFifo->ptrWrPtr = ESM_INVALID_ADDRESS;
		g_arESMInst[u4Idx].prSwFifo->ptrSa = ESM_INVALID_ADDRESS;
		g_arESMInst[u4Idx].prSwFifo->ptrEa = ESM_INVALID_ADDRESS;
		g_arESMInst[u4Idx].prSwFifo->ptrRdPtr = ESM_INVALID_ADDRESS;
		g_arESMInst[u4Idx].prSwFifo->ptrWrPtr = ESM_INVALID_ADDRESS;
		g_arESMInst[u4Idx].prFifo = g_arESMInst[u4Idx].prHwFifo;

		g_arESMInst[u4Idx].prAUTable->ptrSa = ESM_INVALID_ADDRESS;
		g_arESMInst[u4Idx].prAUTable->u4Sz = 0;
		g_arESMInst[u4Idx].prAUTable->u4RdIdx = ESM_INVALID_INDEX;
		g_arESMInst[u4Idx].prAUTable->u4WrIdx = ESM_INVALID_INDEX;
		g_arESMInst[u4Idx].prAUTable->u4AUCount = ESM_INVALID_COUNT;

		g_arESMInst[u4Idx].prAUExtTable->ptrSa = ESM_INVALID_ADDRESS;
		g_arESMInst[u4Idx].prAUExtTable->u4Sz = 0;
		g_arESMInst[u4Idx].prAUExtTable->u4RdIdx = ESM_INVALID_INDEX;
		g_arESMInst[u4Idx].prAUExtTable->u4WrIdx = ESM_INVALID_INDEX;
		g_arESMInst[u4Idx].prAUExtTable->u4AUCount = ESM_INVALID_COUNT;

		g_arESMInst[u4Idx].u4FilterType = u4FilterType;
		g_arESMInst[u4Idx].u4FilterId = u4FilterId;
		g_arESMInst[u4Idx].u4StmCodec = DMX_INVALID_UINT32;
		g_arESMInst[u4Idx].u8DecSendBufMask = u8DecSendBufMask;
		g_arESMInst[u4Idx].eType = ES_NONE;
		g_arESMInst[u4Idx].pvDecoderCB = NULL;
		g_arESMInst[u4Idx].pvDecoderCBPrivate = NULL;
		g_arESMInst[u4Idx].pvDemuxerCB = NULL;
		g_arESMInst[u4Idx].pvDemuxerCBPrivate = NULL;
		g_arESMInst[u4Idx].u2Ref = 0;
		g_arESMInst[u4Idx].pvSptHdl = pvSptHdl;
	}

	if (u4Idx >= MAX_ESICOUNT) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail in alloc DMX_AUTABLE_INFO_T(no memory)\r\n"),
			DMX_FUNC_NAME);
		mrRet = RET_DMX_NO_MEM;
		goto ESMCREATEERR;
	}

	g_arESMInst[u4Idx].u2Ref++;

	*pu4Handle = u4Handle;

	MM_RETURN(RET_DMX_OK);

ESMCREATEERR:

	if (NULL != g_arESMInst[u4Idx].prSwFifo) {
		DMX_FreeMemory(g_arESMInst[u4Idx].prSwFifo);
		g_arESMInst[u4Idx].prSwFifo = NULL;
	}

	if (NULL != g_arESMInst[u4Idx].prHwFifo) {
		DMX_FreeMemory(g_arESMInst[u4Idx].prHwFifo);
		g_arESMInst[u4Idx].prHwFifo = NULL;
	}

	g_arESMInst[u4Idx].prFifo = NULL;

	if (NULL != g_arESMInst[u4Idx].prAUTable) {
		DMX_FreeMemory(g_arESMInst[u4Idx].prAUTable);
		g_arESMInst[u4Idx].prAUTable = NULL;
	}

	if (NULL != g_arESMInst[u4Idx].prAUExtTable) {
		DMX_FreeMemory(g_arESMInst[u4Idx].prAUExtTable);
		g_arESMInst[u4Idx].prAUExtTable = NULL;
	}

	MM_RETURN(mrRet);
}

MRESULT ESM_Destroy(u32 u4Handle)
{
	if (MAX_ESICOUNT <= u4Handle)
		MM_RETURN(RET_DMX_OK);

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (g_arESMInst[u4Handle].u2Ref > 0) {
		g_arESMInst[u4Handle].u2Ref--;
		if (0 == g_arESMInst[u4Handle].u2Ref)
			ESM_FreeES(u4Handle);
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT ESM_GetStmCodec(u32 u4Handle, u32 *pu4Codec)
{
	HANDLE hStm = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (!g_fgESMInit) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESM hasn't been initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	if (MAX_ESICOUNT <= u4Handle) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for (u4Handle: %d > MaxCount)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		} else {
		if (!g_arESMInst[u4Handle].fgUsed) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for the ESIH(%d) is disable\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	if (ES_NONE == g_arESMInst[u4Handle].eType) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d) hasn't been set type\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_SET_TYPE);
	}

	#if DMX_CHECK_MEM_VALIBILITY
	if (!ESM_CheckAUTableValibility(u4Handle)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s line %d fail for ESIH(%d)'s AUTable FATAL ERROR\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
		#if DMX_DBG_ESM_INFO
		ESM_DumpESMInfo(u4Handle);
		#endif /* DMX_DBG_ESM_INFO*/
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	if (DMX_INVALID_UINT32 == g_arESMInst[u4Handle].u4StmCodec) {
		hStm = GetStreamByType(g_arESMInst[u4Handle].pvSptHdl,
			g_arESMInst[u4Handle].u4FilterType);

		mrRet = StreamGetCodec(hStm, &(g_arESMInst[u4Handle].u4StmCodec));

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d fail for ESIH(%d)'s AUTable FATAL ERROR\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
			MM_RETURN(mrRet);
		}
	}

	if (NULL != pu4Codec)
		*pu4Codec = g_arESMInst[u4Handle].u4StmCodec;

	MM_RETURN(RET_DMX_OK);
}

void ESM_GetAudioFifoInfo(u32 u4Handle, DMX_FIFO_INFO_T *prFifoInfo,
	DMX_FIFO_INFO_T *prDSPFifoInfo)
{
	CheckInit();
	CheckHandle(u4Handle);
	CheckFifo(u4Handle);

	if (ES_A == g_arESMInst[u4Handle].eType) {
#if AUDIO_USE_AUDPRIMARY_FIFO
		if (NULL != prDSPFifoInfo) {
			AUD_POSINFO_T rAudPos;
			mm_memset(&rAudPos, 0, sizeof(rAudPos));

			if (0 != i4AudEsm_GetAudioCodecFifoInfo(AUD_FIFO_RPIMARY, &rAudPos)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d (u4Handle: 0x%x) fail in i4AudEsm_GetAudioCodecFifoInfo\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Handle);
				return;
		  }

			if (rAudPos.ptrAfifoVirSA >= rAudPos.ptrAfifoVirEA) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d (u4Handle: 0x%x) fail for invalid audio fifo SA(0x%08x), EA(0x%08x)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					rAudPos.ptrAfifoVirSA, rAudPos.ptrAfifoVirEA);
				return;
		  }

			prDSPFifoInfo->ptrSa = g_arESMInst[u4Handle].prFifo->ptrSa;
			prDSPFifoInfo->ptrEa = g_arESMInst[u4Handle].prFifo->ptrEa;
			
			prFifoInfo->ptrRdPtr = rAudPos.ptrAfifoRPtr + g_arESMInst[u4Handle].prFifo->ptrSa;
			prFifoInfo->ptrWrPtr = g_arESMInst[u4Handle].prFifo->ptrWrPtr;
		}
#endif /* AUDIO_USE_AUDPRIMARY_FIFO*/
		if (NULL != prFifoInfo) {
			prFifoInfo->ptrSa = g_arESMInst[u4Handle].prFifo->ptrSa;
			prFifoInfo->ptrEa = g_arESMInst[u4Handle].prFifo->ptrEa;
			prFifoInfo->ptrRdPtr = g_arESMInst[u4Handle].prFifo->ptrRdPtr;
			prFifoInfo->ptrWrPtr = g_arESMInst[u4Handle].prFifo->ptrWrPtr;
		}
	 }
}

void _ESM_PrintAUChkSum_V(u32 u4Handle, u32 u4AuIdx)
{
	AU_VPic *paAUTable;
	PicInfo *prInfo;
	u32 u4TempStartIdx = 0;
	u32 u4ChkSumData = 0;

	if ((MAX_ESICOUNT <= u4Handle) ||
		(!g_arESMInst[u4Handle].fgUsed) ||
		(NULL == g_arESMInst[u4Handle].prAUTable) ||
		(NULL == g_arESMInst[u4Handle].prFifo)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid handle\r\n"),
			DMX_FUNC_NAME);
		return;
	}
	paAUTable = (AU_VPic *)g_arESMInst[u4Handle].prAUTable->ptrSa;

	if ((NULL != paAUTable) &&
		(AU_DATA == paAUTable[u4AuIdx].eAuType)) {
		prInfo = &(paAUTable[u4AuIdx].rAUInfo.rInfo);
		if (prInfo) {
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] AU Idx: %d, type: data\r\n"),
				u4AuIdx);

			u4TempStartIdx = prInfo->ptrSAddr;
			while (u4TempStartIdx != prInfo->ptrEAddr) {
				u4TempStartIdx++;
				if (u4TempStartIdx > g_arESMInst[u4Handle].prFifo->ptrEa) {
					u4TempStartIdx = u4TempStartIdx -
						g_arESMInst[u4Handle].prFifo->ptrEa +
						g_arESMInst[u4Handle].prFifo->ptrSa;
				}
				u4ChkSumData += *((u8 *)(u4TempStartIdx));
			}
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] AU CheckSum: 0x%x\r\n"),
				u4ChkSumData);
		} else {
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] expired data , idx : %d\r\n"),
				u4AuIdx);
		}
	} else
		return;
}

void _ESM_PrintAUChkSum_A(
	u32 u4Handle,				  /*/< [IN] interface handle*/

	u32 u4AuIdx				  /*/< [IN] AU index*/
)
{
	AU_AUDIO *paAUTable;
	u32 u4TempStartIdx = 0;
	u32 u4ChkSumData = 0;

	if ((MAX_ESICOUNT <= u4Handle) ||
		(!g_arESMInst[u4Handle].fgUsed) ||
		(NULL == g_arESMInst[u4Handle].prAUTable) ||
		(NULL == g_arESMInst[u4Handle].prFifo)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid handle\r\n"),
			DMX_FUNC_NAME);
		return;
	}
	paAUTable = (AU_AUDIO *)g_arESMInst[u4Handle].prAUTable->ptrSa;

	if ((NULL != paAUTable) &&
		(AU_DATA == paAUTable[u4AuIdx].eAuType)) {
		paAUTable = &(paAUTable[u4AuIdx]);
		if (paAUTable) {
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] AU Idx: %d, type: data\r\n"),
				u4AuIdx);
			u4TempStartIdx = paAUTable->ptrSAddr;
			while (u4TempStartIdx != paAUTable->ptrEAddr) {
				u4TempStartIdx++;
				if (u4TempStartIdx > g_arESMInst[u4Handle].prFifo->ptrEa) {
					u4TempStartIdx = u4TempStartIdx -
						g_arESMInst[u4Handle].prFifo->ptrEa +
						g_arESMInst[u4Handle].prFifo->ptrSa;
				}
				u4ChkSumData += *((u8 *)(u4TempStartIdx));
			}
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] AU CheckSum: 0x%x\r\n"),
				u4ChkSumData);
		} else {
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] expired data , idx : %d\r\n"),
				u4AuIdx);
		}
	} else
		return;
}


void ESM_PrintESIStatus(
	u32 u4Handle					/*/< [IN] interface handle*/
)
{
	DMX_FIFO_INFO_T    *prFifo = NULL;
	DMX_AUTABLE_INFO_T *prAUTable = NULL;
	u32	u4AvailCount = 0;
	u32	u4AUFull = 0;
	u32	u4FifoSz = 0;
	u32	u4FifoAvail = 0;
	u32	u4FifoFull = 0;
	MRESULT mrRet = RET_DMX_OK;

	if ((MAX_ESICOUNT <= u4Handle) ||
		(!g_arESMInst[u4Handle].fgUsed) ||
		(NULL == g_arESMInst[u4Handle].prAUTable) ||
		(NULL == g_arESMInst[u4Handle].prFifo)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid handle\r\n"),
			DMX_FUNC_NAME);
		return;
	}

	prFifo = g_arESMInst[u4Handle].prFifo;
	prAUTable = g_arESMInst[u4Handle].prAUTable;

	u4AvailCount = ESM_AUTableAvailCount(u4Handle);

	if (prAUTable->u4AUCount > 0)
		u4AUFull = (u4AvailCount * 100) / prAUTable->u4AUCount;
	else {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s(u4ESIH: 0x%x)'s u4AUCount = 0\r\n"),
			DMX_FUNC_NAME, u4Handle);
	}

	u4FifoSz = prFifo->ptrEa - prFifo->ptrSa;

	if (u4FifoSz > 0) {
		mrRet = ESM_FifoGetAvailDataSize(u4Handle, &u4FifoAvail);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[PSR] %s line %d fail in")
				TEXT(" ESM_AUTableGetFreeCount(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Handle, mrRet);
			return;
		}
		u4FifoFull = (u4FifoAvail * 100) / u4FifoSz;
	} else {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s (u4ESIH: 0x%x)'s u4FifoSz = 0\r\n"),
			DMX_FUNC_NAME, u4Handle);
	}

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
		TEXT("[ESM] ES Type: %d, Filter type: %d, Id: %d\r\n"),
		(u32)g_arESMInst[u4Handle].eType,
		(u32)g_arESMInst[u4Handle].u4FilterType,
		(u32)g_arESMInst[u4Handle].u4FilterId);
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
		TEXT("[ESM] Fifo %d/%d (%d%%) Rp: 0x%x, Wp: 0x%x, Sa: 0x%x, Ea: 0x%x\r\n"),
		u4FifoAvail, u4FifoSz, u4FifoFull,
		prFifo->ptrRdPtr, prFifo->ptrWrPtr,
		prFifo->ptrSa, prFifo->ptrEa);
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
		TEXT("[ESM] AU %d/%d (%d%%) RIdx: %d, WIdx: %d\r\n"),
		u4AvailCount, prAUTable->u4AUCount, u4AUFull,
		prAUTable->u4RdIdx, prAUTable->u4WrIdx);
}

void _ESM_PrintAUInfo_V(
	u32 u4Handle,				   /*/< [IN] interface handle*/
	u32 u4StartIdx,				   /*/< [IN] start index*/

	u32 u4EndIdx
)
{
	AU_VPic *paAUTable = NULL;
	PicInfo *prInfo = NULL;
	u32	u4Idx = 0;
	/*char szMsg[256];*/

	if ((MAX_ESICOUNT <= u4Handle) ||
		(!g_arESMInst[u4Handle].fgUsed) ||
		(NULL == g_arESMInst[u4Handle].prAUTable) ||
		(NULL == g_arESMInst[u4Handle].prFifo)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid handle\r\n"),
			DMX_FUNC_NAME);
		return;
	}

	if (u4StartIdx > u4EndIdx) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid u4StartIdx(%d), u4EndIdx(%d), must u4StartIdx <= u4EndIdx\r\n"),
			DMX_FUNC_NAME, u4StartIdx, u4EndIdx);
		return;
	}

	if ((u4StartIdx >= g_arESMInst[u4Handle].prAUTable->u4AUCount) ||
		(u4EndIdx >= g_arESMInst[u4Handle].prAUTable->u4AUCount)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid u4StartIdx(%d), u4EndIdx(%d), must < AUCount(%d)\r\n"),
			DMX_FUNC_NAME, u4StartIdx, u4EndIdx,
			g_arESMInst[u4Handle].prAUTable->u4AUCount);
		return;
	}

	paAUTable = (AU_VPic *)g_arESMInst[u4Handle].prAUTable->ptrSa;
	if (NULL == paAUTable) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid AUTable SA\r\n"),
			DMX_FUNC_NAME);
		return;
	}

	for (u4Idx = u4StartIdx; u4Idx <= u4EndIdx; u4Idx++) {
		if (AU_DATA == paAUTable[u4Idx].eAuType) {
			prInfo = &(paAUTable[u4Idx].rAUInfo.rInfo);
			if (prInfo != NULL) {
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] Video AUIdx[%d] -- AU_DATA, VType: 0x%x, ptrSAddr: 0x%x,")
					TEXT(" ptrEAddr: 0x%x, u8Offset: %I64d, PTS: %I64d ms\r\n"),
					u4Idx, prInfo->u4VType, prInfo->ptrSAddr, prInfo->ptrEAddr,
					prInfo->u8Offset, PTS_TO_MS(prInfo->u8Pts));
			} else
				return;
		} else if (AU_CMD == paAUTable[u4Idx].eAuType) {
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] Video AUIdx[%d] -- AU_CMD\r\n"),
				u4Idx);
		} else {
			/*do nothing*/
		}
	}
}

void _ESM_PrintAUInfo_A(
	u32 u4Handle,				   /*/< [IN] interface handle*/
	u32 u4StartIdx,				   /*/< [IN] start index*/

	u32 u4EndIdx
)
{
	u32	 u4Idx = 0;
	AU_AUDIO *paAUTable = NULL;

	if ((MAX_ESICOUNT <= u4Handle) ||
		(!g_arESMInst[u4Handle].fgUsed) ||
		(NULL == g_arESMInst[u4Handle].prAUTable) ||
		(NULL == g_arESMInst[u4Handle].prFifo)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid handle\r\n"),
			DMX_FUNC_NAME);
		return;
	}

	if (u4StartIdx > u4EndIdx) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid u4StartIdx(%d), u4EndIdx(%d), must u4StartIdx <= u4EndIdx\r\n"),
			DMX_FUNC_NAME, u4StartIdx, u4EndIdx);
		return;
	}

	if ((u4StartIdx >= g_arESMInst[u4Handle].prAUTable->u4AUCount) ||
		(u4EndIdx >= g_arESMInst[u4Handle].prAUTable->u4AUCount)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid u4StartIdx(%d), u4EndIdx(%d), must < AUCount(%d)\r\n"),
			DMX_FUNC_NAME, u4StartIdx, u4EndIdx,
			g_arESMInst[u4Handle].prAUTable->u4AUCount);
		return;
	}

	paAUTable = (AU_AUDIO *)g_arESMInst[u4Handle].prAUTable->ptrSa;
	if (NULL == paAUTable) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid AUTable SA\r\n"),
			DMX_FUNC_NAME);
		return;
	}

	for (u4Idx = u4StartIdx; u4Idx <= u4EndIdx; u4Idx++) {
		if (AU_DATA == paAUTable[u4Idx].eAuType) {
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] Audio AUIdx[%d] -- AU_DATA, ptrSAddr: 0x%x,")
				TEXT(" ptrEAddr: 0x%x, PTS: %I64d ms\r\n"),
				u4Idx, paAUTable[u4Idx].ptrSAddr, paAUTable[u4Idx].ptrEAddr,
				PTS_TO_MS(paAUTable[u4Idx].rAUInfo.rInfo.u8Pts));
		} else if (AU_CMD == paAUTable[u4Idx].eAuType) {
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] Audio AUIdx[%d] -- AU_CMD\r\n"),
				u4Idx);
		} else {
			/*do nothing*/
		}
	}
}

void _ESM_PrintAUInfo_SP(
	u32 u4Handle,				/*/< [IN] interface handle*/
	u32 u4StartIdx,				/*/< [IN] start index*/

	u32 u4EndIdx					/*/< [IN] end index*/
)
{
	u32	 u4Idx = 0;
	AU_SP	 *paAUTable = NULL;
	SPicInfo *prInfo = NULL;

	if ((MAX_ESICOUNT <= u4Handle) ||
		(!g_arESMInst[u4Handle].fgUsed) ||
		(NULL == g_arESMInst[u4Handle].prAUTable) ||
		(NULL == g_arESMInst[u4Handle].prFifo)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid handle\r\n"),
			DMX_FUNC_NAME);
		return;
	}

	if (u4StartIdx > u4EndIdx) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid u4StartIdx(%d), u4EndIdx(%d), must u4StartIdx <= u4EndIdx\r\n"),
			DMX_FUNC_NAME, u4StartIdx, u4EndIdx);
		return;
	}

	if ((u4StartIdx >= g_arESMInst[u4Handle].prAUTable->u4AUCount) ||
		(u4EndIdx >= g_arESMInst[u4Handle].prAUTable->u4AUCount)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid u4StartIdx(%d), u4EndIdx(%d), must < AUCount(%d)\r\n"),
			DMX_FUNC_NAME, u4StartIdx, u4EndIdx,
			g_arESMInst[u4Handle].prAUTable->u4AUCount);
		return;
	}

	paAUTable = (AU_SP *)g_arESMInst[u4Handle].prAUTable->ptrSa;
	if (NULL == paAUTable) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid AUTable SA\r\n"),
			DMX_FUNC_NAME);
		return;
	}

	for (u4Idx = u4StartIdx; u4Idx <= u4EndIdx; u4Idx++) {
		if (AU_DATA == paAUTable[u4Idx].eAuType) {
			prInfo = &(paAUTable[u4Idx].rAUInfo.rInfo);

			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] SP AUIdx[%d] -- AU_DATA, ptrSAddr: 0x%x, u4Size: 0x%x, PTS: %I64d ms\r\n"),
				u4Idx, prInfo->ptrAddr, prInfo->u4Size, prInfo->u8Offset,
				PTS_TO_MS(prInfo->u8StartPts));
		} else if (AU_CMD == paAUTable[u4Idx].eAuType) {
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] SP AUIdx[%d] -- AU_CMD\r\n"),
				u4Idx);
		} else {
			/*do nothing*/
		}
	}
}

void _ESM_PrintAUDataInfo_V(
	u32 u4Handle,				/*/< [IN] interface handle*/
	u32 u4AuIdx,					/*/< [IN] AU index*/
	u32 u4StartOffset,				/*/< [IN] start offset*/

	u32 u4Length					/*/< [IN] data length*/
)
{
	AU_VPic *paAUTable = NULL;
	PicInfo *prInfo = NULL;
	u32	u4TempStartIdx = 0;

	if ((MAX_ESICOUNT <= u4Handle) ||
		(!g_arESMInst[u4Handle].fgUsed) ||
		(NULL == g_arESMInst[u4Handle].prAUTable) ||
		(NULL == g_arESMInst[u4Handle].prFifo)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		return;
	}

	paAUTable = (AU_VPic *)g_arESMInst[u4Handle].prAUTable->ptrSa;
	if (NULL == paAUTable) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid AUTable SA\r\n"),
			DMX_FUNC_NAME);
		return;
	}

	if (AU_DATA == paAUTable[u4AuIdx].eAuType) {
		prInfo = &(paAUTable[u4AuIdx].rAUInfo.rInfo);
		if (prInfo) {
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] AU Idx: %d, type: data\r\n"), u4AuIdx);
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] AU data info from offset %d, length %d----\r\n"),
				u4StartOffset, u4Length);
			u4TempStartIdx = prInfo->ptrSAddr + u4StartOffset;
			if (u4TempStartIdx > g_arESMInst[u4Handle].prFifo->ptrEa) {
				u4TempStartIdx = u4TempStartIdx -
					g_arESMInst[u4Handle].prFifo->ptrEa +
					g_arESMInst[u4Handle].prFifo->ptrSa;
			}
			while (u4Length > 0) {
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("0x%2x "), *((u8 *)u4TempStartIdx));
				u4TempStartIdx++;
				u4Length--;
				if (u4TempStartIdx > g_arESMInst[u4Handle].prFifo->ptrEa) {
					u4TempStartIdx = u4TempStartIdx -
						g_arESMInst[u4Handle].prFifo->ptrEa +
						g_arESMInst[u4Handle].prFifo->ptrSa;
				}
			}
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM, TEXT("\r\n"));
		} else
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] expired data , idx : %d\r\n"), u4AuIdx);
	} else {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] IBC CMD! idx : %d\r\n"), u4AuIdx);
		return;
	}

}

void _ESM_PrintAUDataInfo_A(
	u32 u4Handle,				/*/< [IN] interface handle*/
	u32 u4AuIdx,					/*/< [IN] AU index*/
	u32 u4StartOffset,				/*/< [IN] start offset*/

	u32 u4Length					/*/< [IN] data length*/
)
{
	AU_AUDIO *paAUTable = NULL;
	u32	 u4TempStartIdx = 0;

	if ((MAX_ESICOUNT <= u4Handle) ||
		(!g_arESMInst[u4Handle].fgUsed) ||
		(NULL == g_arESMInst[u4Handle].prAUTable) ||
		(NULL == g_arESMInst[u4Handle].prFifo)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		return;
	}

	paAUTable = (AU_AUDIO *)g_arESMInst[u4Handle].prAUTable->ptrSa;
	if (NULL == paAUTable) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid AUTable SA\r\n"),
			DMX_FUNC_NAME);
		return;
	}

	if (AU_DATA == paAUTable[u4AuIdx].eAuType) {
		paAUTable = &(paAUTable[u4AuIdx]);
		if (paAUTable) {
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] AU Idx: %d, type: data\r\n"),
				u4AuIdx);
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] AU data info from offset %d, length %d----\r\n"),
				u4StartOffset, u4Length);
			u4TempStartIdx = paAUTable->ptrSAddr + u4StartOffset;
			if (u4TempStartIdx > g_arESMInst[u4Handle].prFifo->ptrEa) {
				u4TempStartIdx = u4TempStartIdx -
					g_arESMInst[u4Handle].prFifo->ptrEa +
					g_arESMInst[u4Handle].prFifo->ptrSa;
			}
			while (u4Length > 0) {
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("0x%2x "), *((u8 *)u4TempStartIdx));
				u4TempStartIdx++;
				u4Length--;
				if (u4TempStartIdx > g_arESMInst[u4Handle].prFifo->ptrEa) {
					u4TempStartIdx = u4TempStartIdx -
						g_arESMInst[u4Handle].prFifo->ptrEa +
						g_arESMInst[u4Handle].prFifo->ptrSa;
				}
			}
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM, TEXT("\n"));
		} else
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] expired data , idx : %d\r\n"), u4AuIdx);
	} else {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] IBC CMD! idx : %d\r\n"), u4AuIdx);
		return;
	}

}

void _ESM_PrintAUDataInfo_SP(
	u32 u4Handle,				/*/< [IN] interface handle*/
	u32 u4AuIdx,				   /*/< [IN] AU index*/
	u32 u4StartOffset,				   /*/< [IN] start offset*/

	u32 u4Length				  /*/< [IN] data length*/
)
{
	AU_SP	 *paAUTable = NULL;
	SPicInfo *prInfo = NULL;
	u32	 u4TempStartIdx = 0;

	if ((MAX_ESICOUNT <= u4Handle) ||
		(!g_arESMInst[u4Handle].fgUsed) ||
		(NULL == g_arESMInst[u4Handle].prAUTable) ||
		(NULL == g_arESMInst[u4Handle].prFifo)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		return;
	}

	paAUTable = (AU_SP *)g_arESMInst[u4Handle].prAUTable->ptrSa;
	if (NULL == paAUTable) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid AUTable SA\r\n"),
			DMX_FUNC_NAME);
		return;
	}

	if (AU_DATA == paAUTable[u4AuIdx].eAuType) {
		prInfo = &(paAUTable[u4AuIdx].rAUInfo.rInfo);
		if (prInfo) {
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] AU Idx: %d, type: data\r\n"), u4AuIdx);
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] AU data info from offset %d, length %d----\r\n"),
				u4StartOffset, u4Length);
			u4TempStartIdx = prInfo->ptrAddr + u4StartOffset;
			if (u4TempStartIdx > g_arESMInst[u4Handle].prFifo->ptrEa) {
				u4TempStartIdx = u4TempStartIdx -
					g_arESMInst[u4Handle].prFifo->ptrEa +
					g_arESMInst[u4Handle].prFifo->ptrSa;
			}
			while (u4Length > 0) {
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("0x%2x "), *((u8 *)u4TempStartIdx));
				u4TempStartIdx++;
				u4Length--;
				if (u4TempStartIdx > g_arESMInst[u4Handle].prFifo->ptrEa) {
					u4TempStartIdx = u4TempStartIdx -
						g_arESMInst[u4Handle].prFifo->ptrEa +
						g_arESMInst[u4Handle].prFifo->ptrSa;
				}
			}
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM, TEXT("\r\n"));
		} else
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] expired data , idx : %d\r\n"), u4AuIdx);
	} else {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] IBC CMD! idx : %d\r\n"), u4AuIdx);
		return;
	}

}

void ESM_PrintAUInfo(
	u32 u4Handle,				/*/< [IN] interface handle*/
	u32 u4StartIdx,				/*/< [IN] start index*/

	u32 u4EndIdx					/*/< [IN] end index*/
)
{
	u32 u4SIdx, u4EIdx;

	if ((MAX_ESICOUNT <= u4Handle) ||
		(!g_arESMInst[u4Handle].fgUsed) ||
		(NULL == g_arESMInst[u4Handle].prAUTable)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		return;
	}

	if ((MAX_ESICOUNT <= u4Handle) ||
		(!g_arESMInst[u4Handle].fgUsed) ||
		(NULL == g_arESMInst[u4Handle].prAUTable) ||
		(NULL == g_arESMInst[u4Handle].prFifo)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid handle\r\n"),
			DMX_FUNC_NAME);
		return;
	}

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
		TEXT("[ESM] Stream %s ESHandle %d\r\n"),
		ESM_TYPESTR(g_arESMInst[u4Handle].eType), u4Handle);

	if (u4StartIdx > u4EndIdx) {
		if ((g_arESMInst[u4Handle].prAUTable->u4RdIdx < u4StartIdx) &&
			(g_arESMInst[u4Handle].prAUTable->u4RdIdx > u4EndIdx) &&
			(g_arESMInst[u4Handle].prAUTable->u4WrIdx < u4StartIdx) &&
			(g_arESMInst[u4Handle].prAUTable->u4WrIdx > u4EndIdx)) {
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d -- [u4StartIdx(%d) ~ u4EndIdx(%d)] not in Valid %s")
				TEXT(" AU Range 's Idx Range [RdIdx(%d) ~ WrIdx(%d)]\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4StartIdx, u4EndIdx,
				ESM_TYPESTR(g_arESMInst[u4Handle].eType),
				g_arESMInst[u4Handle].prAUTable->u4RdIdx,
				g_arESMInst[u4Handle].prAUTable->u4WrIdx);
		} else {
			if (u4StartIdx < g_arESMInst[u4Handle].prAUTable->u4RdIdx)
				u4SIdx = g_arESMInst[u4Handle].prAUTable->u4RdIdx;
			else
				u4SIdx = u4StartIdx;
			if (u4EndIdx < g_arESMInst[u4Handle].prAUTable->u4WrIdx)
				u4EIdx = u4EndIdx;
			else
				u4EIdx = g_arESMInst[u4Handle].prAUTable->u4WrIdx;

			if (u4SIdx > u4EIdx) {
				if (ES_V == g_arESMInst[u4Handle].eType) {
					_ESM_PrintAUInfo_V(u4Handle, u4SIdx,
						(g_arESMInst[u4Handle].prAUTable->u4AUCount - 1));
					_ESM_PrintAUInfo_V(u4Handle, 0, u4EIdx);
				} else if (ES_A == g_arESMInst[u4Handle].eType) {
					_ESM_PrintAUInfo_A(u4Handle, u4SIdx,
						(g_arESMInst[u4Handle].prAUTable->u4AUCount - 1));
					_ESM_PrintAUInfo_A(u4Handle, 0, u4EIdx);
				} else if (ES_SP == g_arESMInst[u4Handle].eType) {
					_ESM_PrintAUInfo_SP(u4Handle, u4SIdx,
						(g_arESMInst[u4Handle].prAUTable->u4AUCount - 1));
					_ESM_PrintAUInfo_SP(u4Handle, 0, u4EIdx);
				} else {
					DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
						TEXT("[ESM] invalid Estype(%d)\r\n"),
						g_arESMInst[u4Handle].eType);
				}
			} else {
				if (ES_V == g_arESMInst[u4Handle].eType)
					_ESM_PrintAUInfo_V(u4Handle, u4SIdx, u4EIdx);
				else if (ES_A == g_arESMInst[u4Handle].eType)
					_ESM_PrintAUInfo_A(u4Handle, u4SIdx, u4EIdx);
				else if (ES_SP == g_arESMInst[u4Handle].eType)
					_ESM_PrintAUInfo_SP(u4Handle, u4SIdx, u4EIdx);
				else {
					DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
						TEXT("[ESM] invalid Estype(%d)\r\n"),
						g_arESMInst[u4Handle].eType);
				}
			}
		}
	} else {
		if (g_arESMInst[u4Handle].prAUTable->u4RdIdx <= u4StartIdx)
			u4SIdx = u4StartIdx;
		else {
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d -- %s AU [u4StartIdx(%d) ~ RdIdx(%d)) has been released\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				ESM_TYPESTR(g_arESMInst[u4Handle].eType),
				u4StartIdx, g_arESMInst[u4Handle].prAUTable->u4RdIdx);
			u4SIdx = g_arESMInst[u4Handle].prAUTable->u4RdIdx;
		}
		if (u4EndIdx < g_arESMInst[u4Handle].prAUTable->u4WrIdx)
			u4EIdx = u4EndIdx;
		else {
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s line %d -- %s AU [u4WrIdx(%d) ~ u4EndIdx(%d)) hasn't been created\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				ESM_TYPESTR(g_arESMInst[u4Handle].eType),
				g_arESMInst[u4Handle].prAUTable->u4WrIdx, u4EndIdx);
			if (0 < g_arESMInst[u4Handle].prAUTable->u4WrIdx)
				u4EIdx = g_arESMInst[u4Handle].prAUTable->u4WrIdx - 1;
			else
				u4EIdx = g_arESMInst[u4Handle].prAUTable->u4AUCount - 1;
		}

		if (u4SIdx <= u4EIdx) {
			if (ES_V == g_arESMInst[u4Handle].eType)
				_ESM_PrintAUInfo_V(u4Handle, u4SIdx, u4EIdx);
			else if (ES_A == g_arESMInst[u4Handle].eType)
				_ESM_PrintAUInfo_A(u4Handle, u4SIdx, u4EIdx);
			else if (ES_SP == g_arESMInst[u4Handle].eType)
				_ESM_PrintAUInfo_SP(u4Handle, u4SIdx, u4EIdx);
			else
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] invalid Estype(%d)\r\n"),
					g_arESMInst[u4Handle].eType);
		}
	}
}

void ESM_PrintAUDataInfo(
	u32 u4Handle,					/*/< [IN] interface handle*/
	u32 u4AuIdx,				   /*/< [IN] AU index*/
	u32 u4StarOffset,				  /*/< [IN] start offset*/

	u32 u4Length					/*/< [IN] data length*/
)
{
	if ((MAX_ESICOUNT <= u4Handle) ||
		(!g_arESMInst[u4Handle].fgUsed)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		return;
	}

	if (ES_V == g_arESMInst[u4Handle].eType)
		_ESM_PrintAUDataInfo_V(u4Handle, u4AuIdx, u4StarOffset, u4Length);
	else if (ES_A == g_arESMInst[u4Handle].eType)
		_ESM_PrintAUDataInfo_A(u4Handle, u4AuIdx, u4StarOffset, u4Length);
	else if (ES_SP == g_arESMInst[u4Handle].eType)
		_ESM_PrintAUDataInfo_SP(u4Handle, u4AuIdx, u4StarOffset, u4Length);
	else
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] invalid Estype(%d)\r\n"),
			g_arESMInst[u4Handle].eType);
}

void ESM_PrintAUDataInfoEx(
	u32 u4Handle,				/*/< [IN] interface handle*/
	u32 u4StartIdx,				/*/< [IN] start index*/

	u32 u4EndIdx					/*/< [IN] end index*/
)
{
	u32 u4PrintDataLen = (u32)20;
	u32 u4TempCount = 0;

	if ((MAX_ESICOUNT <= u4Handle) ||
		(!g_arESMInst[u4Handle].fgUsed)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		return;
	}

	if (ES_V == g_arESMInst[u4Handle].eType) {
		for (u4TempCount = u4StartIdx; u4TempCount < u4EndIdx; u4TempCount++)
			_ESM_PrintAUDataInfo_V(u4Handle, u4TempCount, 0, u4PrintDataLen);
	} else if (ES_A == g_arESMInst[u4Handle].eType) {
		for (u4TempCount = u4StartIdx; u4TempCount < u4EndIdx; u4TempCount++)
			_ESM_PrintAUDataInfo_A(u4Handle, u4TempCount, 0, u4PrintDataLen);
	} else if (ES_SP == g_arESMInst[u4Handle].eType) {
		for (u4TempCount = u4StartIdx; u4TempCount < u4EndIdx; u4TempCount++)
			_ESM_PrintAUDataInfo_SP(u4Handle, u4TempCount, 0, u4PrintDataLen);
	} else
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] invalid Estype(%d)\r\n"),
			g_arESMInst[u4Handle].eType);
}

void ESM_PrintFifoInfo(u32 u4Handle)
{
	if (MAX_ESICOUNT <= u4Handle) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for invalid args -- u4Handle: %d\r\n"),
			DMX_FUNC_NAME, u4Handle);
		return;
	}

	if (!g_arESMInst[u4Handle].fgUsed) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for the ESM instance(u4Handle: %d) unused\r\n"),
			DMX_FUNC_NAME, u4Handle);
		return;
	}

	if ((NULL == g_arESMInst[u4Handle].prAUTable) ||
		(NULL == g_arESMInst[u4Handle].prFifo)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s fail for AUTable or FifoInfo struct not init (u4Handle: %d)\r\n"),
			DMX_FUNC_NAME, u4Handle);
		return;
	}

	if (ES_A == g_arESMInst[u4Handle].eType) {
#if AUDIO_USE_AUDPRIMARY_FIFO
		AUD_POSINFO_T rAudPos;
		mm_memset(&rAudPos, 0, sizeof(rAudPos));

		if (0 != i4AudEsm_GetAudioCodecFifoInfo(AUD_FIFO_RPIMARY, &rAudPos)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
				TEXT("[ESM] %s fail in i4AudEsm_GetAudioCodecFifoInfo(u4Handle: %d)\r\n"),
				DMX_FUNC_NAME, u4Handle);
			return;
		}

		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] Audio(DSP) -> WP[0]: 0x%x(+Sa= 0x%x), RP[0]: 0x%x(+Sa= 0x%x), FifoSA: 0x%x\r\n"),
			rAudPos.ptrAfifoWPtr, (rAudPos.ptrAfifoWPtr + g_arESMInst[u4Handle].prFifo->ptrSa),
			rAudPos.ptrAfifoRPtr, (rAudPos.ptrAfifoRPtr + g_arESMInst[u4Handle].prFifo->ptrSa),
			g_arESMInst[u4Handle].prFifo->ptrSa);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] Audio(ESM) -> FifoWP: 0x%x, FifoRP: 0x%x, FifoSA: 0x%x, fgClearFifo: %d\r\n"),
			g_arESMInst[u4Handle].prFifo->ptrWrPtr,
			g_arESMInst[u4Handle].prFifo->ptrRdPtr,
			g_arESMInst[u4Handle].prFifo->ptrSa,
			(_fgAudDrvInClearStatus ? 1 : 0));
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] Audio(ESM) -> RdRdIdx: 0x%x, WrIdx: 0x%x\r\n"),
			g_arESMInst[u4Handle].prAUTable->u4RdIdx,
			g_arESMInst[u4Handle].prAUTable->u4WrIdx);
#else
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
		TEXT("[ESM] Audio(ESM) -> FifoWP: 0x%x, FifoRP: 0x%x, FifoSA: 0x%x\r\n"),
			g_arESMInst[u4Handle].prFifo->ptrWrPtr,
			g_arESMInst[u4Handle].prFifo->ptrRdPtr,
			g_arESMInst[u4Handle].prFifo->ptrSa);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] Audio(ESM) -> RdRdIdx: 0x%x, WrIdx: 0x%x\r\n"),
			g_arESMInst[u4Handle].prAUTable->u4RdIdx,
			g_arESMInst[u4Handle].prAUTable->u4WrIdx);
#endif
	} else if ((ES_V == g_arESMInst[u4Handle].eType) ||
			 (ES_SP == g_arESMInst[u4Handle].eType) ||
			 (ES_SECTION == g_arESMInst[u4Handle].eType)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s(ESM) -> FifoWP: 0x%x, FifoRP: 0x%x, FifoSA: 0x%x\r\n"),
			ESM_TYPESTR(g_arESMInst[u4Handle].eType),
			g_arESMInst[u4Handle].prFifo->ptrWrPtr,
			g_arESMInst[u4Handle].prFifo->ptrRdPtr,
			g_arESMInst[u4Handle].prFifo->ptrSa);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
			TEXT("[ESM] %s(ESM) -> RdRdIdx: 0x%x, WrIdx: 0x%x\r\n"),
			ESM_TYPESTR(g_arESMInst[u4Handle].eType),
			g_arESMInst[u4Handle].prAUTable->u4RdIdx,
			g_arESMInst[u4Handle].prAUTable->u4WrIdx);
	} else {
		/*do nothing*/
	}
}


