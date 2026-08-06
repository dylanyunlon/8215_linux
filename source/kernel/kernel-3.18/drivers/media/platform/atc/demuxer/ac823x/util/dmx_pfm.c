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
 * @file dmx_spt_cli.c
 *
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
#include <linux/errno.h>
#include "windows.h"
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/dmx_cfa_def.h>
#include <media/atc/ioctl_dmx.h>
/* #include <media/atc/mm_debug.h> */
#else	/*  */
#include "dmx_define.h"
#include "dmx_splitter.h"
#include "dmx_cfa_def.h"
#include "ioctl_dmx.h"
#include "mm_debug.h"
#endif	/* __linux__*/

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_spt.h"
#include "dmx_spt_os.h"
#include "dmx_cli.h"
#include "dmx_dump.h"
#include "dmx_esm.h"
#include "dmx_pbbuf.h"
#include "dmx_parser.h"
#include "dmx_pbbuf.h"
#include "dmx_psr_cc.h"
#include "dmx_psr_pbbuf.h"
#include "dmx_spt_main.h"
#include "dmx_spt_rsp.h"
#include "dmx_spt_util.h"
#include "dmx_psr_esm.h"
#include "dmx_esm_if.h"
#include "dmx_gau_if.h"
#include "dmx_gau.h"
#include "dmx_stream.h"
#include "cfa_if.h"
#include "dmx_pfm.h"

#ifndef __linux__
/*disable warning C4127: conditional expression is constant*/
#pragma warning(disable : 4127)
#endif	/* __linux__ */

#if DMX_PFM_TEST
EXTERN PSR_PFM g_rPsrPfm;
EXTERN DMX_STM_MAN_INFO_T g_rDmxStmMan;
EXTERN DMX_SPT_MAN_INFO_T g_rSptMan;

PSR_PFM g_rPsrPfm;

u32 GetTickCountEx(void)
{
	return ((u32)(1000*jiffies/HZ));
}

void DmxPfmInit(void)
{
	mm_memset(&g_rPsrPfm, 0, sizeof(g_rPsrPfm));
}

void DmxPfmInstStart(void *pvSptHdl)
{
	DMX_STM_INST_T *prStmInst = NULL;
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	u32 u4InstID = 0;
	u32 i;

	if (NULL == prSpt) 
		return;
		
	for (u4InstID = 0; u4InstID < DMX_MAX_SPT_INST_CNT; u4InstID++) {
		if (g_rSptMan.aprSptInst[u4InstID] == pvSptHdl)
			break;
	}
	
	if (u4InstID >= DMX_MAX_SPT_INST_CNT)
		return;
	
	g_rPsrPfm.rInst[u4InstID].u4StartTick = GetTickCountEx();

	g_rPsrPfm.rInst[u4InstID].u4EndTick = g_rPsrPfm.rInst[u4InstID].u4StartTick;
	g_rPsrPfm.rInst[u4InstID].eCurOper = PSR_PFM_OPER_NONE;
	mm_memset(&(g_rPsrPfm.rInst[u4InstID].rCfa), 0,
		sizeof(g_rPsrPfm.rInst[u4InstID].rCfa));
	mm_memset(&(g_rPsrPfm.rInst[u4InstID].rSyncPb), 0,
		sizeof(g_rPsrPfm.rInst[u4InstID].rSyncPb));
	mm_memset(&(g_rPsrPfm.rInst[u4InstID].rSyncPb), 0,
		sizeof(g_rPsrPfm.rInst[u4InstID].rSyncPb));
	for (i = 0; i < MAX_STREAM_INSTANCE_CNT; i++) {
		prStmInst = &(g_rDmxStmMan.arStmInst[i]);
		if (prStmInst->pvSptHdl == pvSptHdl) {
			switch (prStmInst->u4StmType) {
			case SPT_DATA_V:
				mm_memset(&(g_rPsrPfm.rVideo), 0, sizeof(g_rPsrPfm.rVideo));
				mm_memset(&(g_rPsrPfm.rDecryptV), 0,
					sizeof(g_rPsrPfm.rDecryptV));
				break;
			case SPT_DATA_A:
				mm_memset(&(g_rPsrPfm.rAudio), 0, sizeof(g_rPsrPfm.rAudio));
				mm_memset(&(g_rPsrPfm.rDecryptA), 0,
					sizeof(g_rPsrPfm.rDecryptA));
				break;
			case SPT_DATA_SP:
				mm_memset(&(g_rPsrPfm.rSP), 0, sizeof(g_rPsrPfm.rSP));
				break;
			default:
				break;
			}
		}
	}
}

void DmxPfmInstStop(void *pvSptHdl)
{
	u32 u4InstID = 0;
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	if (NULL == prSpt) 
		return;
	
	for (u4InstID = 0; u4InstID < DMX_MAX_SPT_INST_CNT; u4InstID++) {
		if (g_rSptMan.aprSptInst[u4InstID] == pvSptHdl)
			break;
	}
	
	if (u4InstID >= DMX_MAX_SPT_INST_CNT)
		return;

	g_rPsrPfm.rInst[u4InstID].u4EndTick = GetTickCountEx();
	g_rPsrPfm.rInst[u4InstID].u4TotalTime = g_rPsrPfm.rInst[u4InstID].u4EndTick
		-g_rPsrPfm.rInst[u4InstID].u4StartTick;


}

void DmxPfmCfaStart(void *pvSptHdl)
{
	u32 u4InstID = 0;
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	if (NULL == prSpt) 
		return;
	
	for (u4InstID = 0; u4InstID < DMX_MAX_SPT_INST_CNT; u4InstID++) {
		if (g_rSptMan.aprSptInst[u4InstID] == pvSptHdl)
			break;
	}
	
	if (u4InstID >= DMX_MAX_SPT_INST_CNT)
		return;

	g_rPsrPfm.rInst[u4InstID].rCfa.u4CfaStartTick = GetTickCountEx();

	g_rPsrPfm.rInst[u4InstID].rCfa.u4CfaEndTick =
		g_rPsrPfm.rInst[u4InstID].rCfa.u4CfaStartTick;
}

void DmxPfmCfaEnd(void *pvSptHdl)
{
	u32 u4InstID = 0;
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	if (NULL == prSpt) 
		return;
	
	for (u4InstID = 0; u4InstID < DMX_MAX_SPT_INST_CNT; u4InstID++) {
		if (g_rSptMan.aprSptInst[u4InstID] == pvSptHdl)
			break;
	}
	
	if (u4InstID >= DMX_MAX_SPT_INST_CNT)
		return;

	g_rPsrPfm.rInst[u4InstID].rCfa.u4CfaEndTick = GetTickCountEx();
	g_rPsrPfm.rInst[u4InstID].rCfa.u4CfaRunTime +=
		g_rPsrPfm.rInst[u4InstID].rCfa.u4CfaEndTick -
		g_rPsrPfm.rInst[u4InstID].rCfa.u4CfaStartTick;

	g_rPsrPfm.rInst[u4InstID].rCfa.u8CfaRunCnt++;
}

void DmxPfmSyncPbbufStart(void *pvSptHdl)
{
	u32 u4InstID = 0;
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	if (NULL == prSpt) 
		return;
	
	for (u4InstID = 0; u4InstID < DMX_MAX_SPT_INST_CNT; u4InstID++) {
		if (g_rSptMan.aprSptInst[u4InstID] == pvSptHdl)
			break;
	}
	
	if (u4InstID >= DMX_MAX_SPT_INST_CNT)
		return;


	g_rPsrPfm.rInst[u4InstID].rSyncPb.u4StartSyncPbTick = GetTickCountEx();

	g_rPsrPfm.rInst[u4InstID].rSyncPb.u4EndSyncPbTick =
		g_rPsrPfm.rInst[u4InstID].rSyncPb.u4StartSyncPbTick;
		g_rPsrPfm.rInst[u4InstID].eCurOper = PSR_PFM_OPER_SYNCPB;
}

void DmxPfmStmSWDmaStart(void *pvSptHdl, E_SPT_DATA_TYPE_T eDataType)
{
	u32 u4InstID = 0;
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	if (NULL == prSpt) 
		return;
	
	for (u4InstID = 0; u4InstID < DMX_MAX_SPT_INST_CNT; u4InstID++) {
		if (g_rSptMan.aprSptInst[u4InstID] == pvSptHdl)
			break;
	}
	
	if (u4InstID >= DMX_MAX_SPT_INST_CNT)
		return;


	switch (eDataType) {
	case SPT_DATA_V:
		g_rPsrPfm.rInst[u4InstID].eCurOper = PSR_PFM_OPER_DMA_V;
		g_rPsrPfm.rVideo.u4StartPb2FifoTick = GetTickCountEx();
	    g_rPsrPfm.rVideo.u4EndPb2FifoTick = g_rPsrPfm.rVideo.u4StartPb2FifoTick;
		break;
	case SPT_DATA_A:
		g_rPsrPfm.rInst[u4InstID].eCurOper = PSR_PFM_OPER_DMA_A;
		g_rPsrPfm.rAudio.u4StartPb2FifoTick = GetTickCountEx();
		g_rPsrPfm.rAudio.u4EndPb2FifoTick = g_rPsrPfm.rAudio.u4StartPb2FifoTick;
		break;
	case SPT_DATA_SP:
		g_rPsrPfm.rInst[u4InstID].eCurOper = PSR_PFM_OPER_DMA_SP;
		g_rPsrPfm.rSP.u4StartPb2FifoTick = GetTickCountEx();
		g_rPsrPfm.rSP.u4EndPb2FifoTick = g_rPsrPfm.rSP.u4StartPb2FifoTick;
		break;
	default:
		break;
	}
}

void DmxPfmPtxDoneEnd(void *pvSptHdl)
{
	u32 u4InstID = 0;
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	if (NULL == prSpt) 
		return;
	
	for (u4InstID = 0; u4InstID < DMX_MAX_SPT_INST_CNT; u4InstID++) {
		if (g_rSptMan.aprSptInst[u4InstID] == pvSptHdl)
			break;
	}
	
	if (u4InstID >= DMX_MAX_SPT_INST_CNT)
		return;


	switch (g_rPsrPfm.rInst[u4InstID].eCurOper) {
	case PSR_PFM_OPER_NONE:
		break;
	case PSR_PFM_OPER_DMA_V:

		g_rPsrPfm.rVideo.u4EndPb2FifoTick = GetTickCountEx();
		g_rPsrPfm.rVideo.u8Pb2FifoTime +=
		    (u64) (g_rPsrPfm.rVideo.u4EndPb2FifoTick -
			      g_rPsrPfm.rVideo.u4StartPb2FifoTick);

		g_rPsrPfm.rVideo.u8Pb2FifoCnt++;
		break;
	case PSR_PFM_OPER_DMA_A:
		g_rPsrPfm.rAudio.u4EndPb2FifoTick = GetTickCountEx();
		g_rPsrPfm.rAudio.u8Pb2FifoTime +=
			(u64) (g_rPsrPfm.rAudio.u4EndPb2FifoTick -
			     g_rPsrPfm.rAudio.u4StartPb2FifoTick);

		g_rPsrPfm.rAudio.u8Pb2FifoCnt++;
		break;
	case PSR_PFM_OPER_DMA_SP:
		g_rPsrPfm.rSP.u4EndPb2FifoTick = GetTickCountEx();
		g_rPsrPfm.rSP.u8Pb2FifoTime +=
		   (u64) (g_rPsrPfm.rSP.u4EndPb2FifoTick -
		   g_rPsrPfm.rSP.u4StartPb2FifoTick);

		g_rPsrPfm.rSP.u8Pb2FifoCnt++;
		break;
	case PSR_PFM_OPER_SYNCPB:
		g_rPsrPfm.rInst[u4InstID].rSyncPb.u4EndSyncPbTick = GetTickCountEx();
		g_rPsrPfm.rInst[u4InstID].rSyncPb.u8SyncPbbufTime +=
		   (u64) (g_rPsrPfm.rInst[u4InstID].rSyncPb.u4EndSyncPbTick -
			      g_rPsrPfm.rInst[u4InstID].rSyncPb.u4StartSyncPbTick);

		g_rPsrPfm.rInst[u4InstID].rSyncPb.u8SyncPbbufCnt++;
		break;
	default:
		break;
	}
	g_rPsrPfm.rInst[u4InstID].eCurOper = PSR_PFM_OPER_NONE;
}

void DmxPfmStmHwDmaStart(E_SPT_DATA_TYPE_T eDataType)
{
	switch (eDataType) {
	case SPT_DATA_V:
		g_rPsrPfm.rVideo.u4HWStartTxTick = GetTickCountEx();
		g_rPsrPfm.rVideo.u4HWEndTxTick = g_rPsrPfm.rVideo.u4HWStartTxTick;
		break;
	case SPT_DATA_A:
		g_rPsrPfm.rAudio.u4HWStartTxTick = GetTickCountEx();
		g_rPsrPfm.rAudio.u4HWEndTxTick = g_rPsrPfm.rAudio.u4HWStartTxTick;
		break;
	case SPT_DATA_SP:
		g_rPsrPfm.rSP.u4HWStartTxTick = GetTickCountEx();
		g_rPsrPfm.rSP.u4HWEndTxTick = g_rPsrPfm.rSP.u4HWStartTxTick;
		break;
	default:
		break;
	}
}

void DmxPfmStmHwDmaEnd(E_SPT_DATA_TYPE_T eDataType)
{
	switch (eDataType) {
	case SPT_DATA_V:
	    g_rPsrPfm.rVideo.u4HWEndTxTick = GetTickCountEx();
		g_rPsrPfm.rVideo.u8HWTxTime +=
		    (u64) (g_rPsrPfm.rVideo.u4HWEndTxTick -
		    g_rPsrPfm.rVideo.u4HWStartTxTick);

		g_rPsrPfm.rVideo.u8HWTxCompleteCnt++;
		break;
	case SPT_DATA_A:
		g_rPsrPfm.rAudio.u4HWEndTxTick = GetTickCountEx();
		g_rPsrPfm.rAudio.u8HWTxTime +=
		    (u64) (g_rPsrPfm.rAudio.u4HWEndTxTick -
		    g_rPsrPfm.rAudio.u4HWStartTxTick);

		g_rPsrPfm.rAudio.u8HWTxCompleteCnt++;
		break;
	case SPT_DATA_SP:
		g_rPsrPfm.rSP.u4HWEndTxTick = GetTickCountEx();
		g_rPsrPfm.rSP.u8HWTxTime +=
		    (u64) (g_rPsrPfm.rSP.u4HWEndTxTick - g_rPsrPfm.rSP.u4HWStartTxTick);

		g_rPsrPfm.rSP.u8HWTxCompleteCnt++;
		break;
	default:
		break;
	}
}

void DmxPfmStmComposeAUStart(E_SPT_DATA_TYPE_T eDataType)
{
	switch (eDataType) {
	case SPT_DATA_V:
		g_rPsrPfm.rVideo.u4StartPsrEsmTick = GetTickCountEx();
		g_rPsrPfm.rVideo.u4EndPsrEsmTick = g_rPsrPfm.rVideo.u4StartPsrEsmTick;
		break;
	case SPT_DATA_A:

	    g_rPsrPfm.rAudio.u4StartPsrEsmTick = GetTickCountEx();
	    g_rPsrPfm.rAudio.u4EndPsrEsmTick = g_rPsrPfm.rAudio.u4StartPsrEsmTick;
		break;
	case SPT_DATA_SP:
	    g_rPsrPfm.rSP.u4StartPsrEsmTick = GetTickCountEx();
	    g_rPsrPfm.rSP.u4EndPsrEsmTick = g_rPsrPfm.rSP.u4StartPsrEsmTick;
		break;
	default:
		break;
	}
}

void DmxPfmStmComposeAUEnd(E_SPT_DATA_TYPE_T eDataType)
{
	switch (eDataType) {
	case SPT_DATA_V:
		g_rPsrPfm.rVideo.u4EndPsrEsmTick = GetTickCountEx();
		g_rPsrPfm.rVideo.u8PsrEsmTime +=
		    (u64) (g_rPsrPfm.rVideo.u4EndPsrEsmTick -
			      g_rPsrPfm.rVideo.u4StartPsrEsmTick);

		g_rPsrPfm.rVideo.u8PsrEsmCnt++;
		break;
	case SPT_DATA_A:
		g_rPsrPfm.rAudio.u4EndPsrEsmTick = GetTickCountEx();
		g_rPsrPfm.rAudio.u8PsrEsmTime +=
		    (u64) (g_rPsrPfm.rAudio.u4EndPsrEsmTick -
			      g_rPsrPfm.rAudio.u4StartPsrEsmTick);

		g_rPsrPfm.rAudio.u8PsrEsmCnt++;
		break;
	case SPT_DATA_SP:
		g_rPsrPfm.rSP.u4EndPsrEsmTick = GetTickCountEx();
		g_rPsrPfm.rSP.u8PsrEsmTime +=
		    (u64) (g_rPsrPfm.rSP.u4EndPsrEsmTick - g_rPsrPfm.rSP.u4StartPsrEsmTick);

		g_rPsrPfm.rSP.u8PsrEsmCnt++;
		break;
	default:
		break;
	}
}

void DmxPfmStmGetAUStart(E_SPT_DATA_TYPE_T eDataType)
{
	switch (eDataType) {
	case SPT_DATA_V:
		if (0 == g_rPsrPfm.rVideo.u4GetAUStartTick)
			g_rPsrPfm.rVideo.u4GetAUStartTick = GetTickCountEx();

		g_rPsrPfm.rVideo.u4GetAUEndTick = g_rPsrPfm.rVideo.u4GetAUStartTick;
		break;
	case SPT_DATA_A:
		if (0 == g_rPsrPfm.rAudio.u4GetAUStartTick)
			g_rPsrPfm.rAudio.u4GetAUStartTick = GetTickCountEx();

		g_rPsrPfm.rAudio.u4GetAUEndTick = g_rPsrPfm.rAudio.u4GetAUStartTick;
		break;
	case SPT_DATA_SP:
		if (0 == g_rPsrPfm.rSP.u4GetAUStartTick)
			g_rPsrPfm.rSP.u4GetAUStartTick = GetTickCountEx();

		g_rPsrPfm.rSP.u4GetAUEndTick = g_rPsrPfm.rSP.u4GetAUStartTick;
		break;
	default:
		break;
	}
}

void DmxPfmStmGetAUEnd(E_SPT_DATA_TYPE_T eDataType)
{
	switch (eDataType) {
	case SPT_DATA_V:
		if (0 == g_rPsrPfm.rVideo.u4GetAUEndTick)
			g_rPsrPfm.rVideo.u4GetAUEndTick = GetTickCountEx();
		g_rPsrPfm.rVideo.u8GetAUTime +=
			(u64) (g_rPsrPfm.rVideo.u4GetAUEndTick - g_rPsrPfm.rVideo.u4GetAUStartTick);

		break;
	case SPT_DATA_A:
		if (0 == g_rPsrPfm.rAudio.u4GetAUEndTick)
			g_rPsrPfm.rAudio.u4GetAUEndTick = GetTickCountEx();
		g_rPsrPfm.rAudio.u8GetAUTime +=
		    (u64) (g_rPsrPfm.rAudio.u4GetAUEndTick -
		    g_rPsrPfm.rAudio.u4GetAUStartTick);

		break;
	case SPT_DATA_SP:
		if (0 == g_rPsrPfm.rSP.u4GetAUEndTick)
			g_rPsrPfm.rSP.u4GetAUEndTick = GetTickCountEx();
		g_rPsrPfm.rSP.u8GetAUTime +=
		    (u64) (g_rPsrPfm.rSP.u4GetAUEndTick - g_rPsrPfm.rSP.u4GetAUStartTick);

		break;
	default:
		break;
	}
}

void DmxPfmStmIncDecryptSyncPbCnt(E_SPT_DATA_TYPE_T eDataType)
{
	if (SPT_DATA_V == eDataType)
		g_rPsrPfm.rDecryptV.u8DecryptInSyncCnt++;

	else if (SPT_DATA_A == eDataType)
		g_rPsrPfm.rDecryptA.u8DecryptInSyncCnt++;
}

void DmxPfmStmIncDecryptPb2Fifo(E_SPT_DATA_TYPE_T eDataType)
{
	if (SPT_DATA_V == eDataType)
		g_rPsrPfm.rDecryptV.u8DecryptPb2FifoCnt++;

	else if (SPT_DATA_A == eDataType)
		g_rPsrPfm.rDecryptA.u8DecryptPb2FifoCnt++;
}

void DmxPfmPrintInfo(void *pvSptHdl)
{
	DMX_STM_INST_T *prStmInst = NULL;
	u32 u4InstID = 0;
	u32 i;
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	if (NULL == prSpt) 
		return;
	
	for (u4InstID = 0; u4InstID < DMX_MAX_SPT_INST_CNT; u4InstID++) {
		if (g_rSptMan.aprSptInst[u4InstID] == pvSptHdl)
			break;
	}
	
	if (u4InstID >= DMX_MAX_SPT_INST_CNT)
		return;

	DmxPfmInstStop(pvSptHdl);

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_PFM,
		TEXT("[DMX_PFM] ============================ pvSptHdl: 0x%x, SptID: %d ")
		TEXT("BEGIN ======================================\r\n"),
		pvSptHdl, u4InstID);

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_PFM,
		TEXT("[DMX_PFM] TotalTime: "DMX_UINT64_10U_LOGSTR"ms\r\n"),
		DMX_UINT64_10U_LOG(g_rPsrPfm.rInst[u4InstID].u4TotalTime));
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_PFM,
		TEXT("[DMX_PFM] SyncPbbuf -- SyncCnt: "DMX_UINT64_10U_LOGSTR)
		TEXT(", Time: "DMX_UINT64_10U_LOGSTR" ms\r\n"),
		DMX_UINT64_10U_LOG(g_rPsrPfm.rInst[u4InstID].rSyncPb.u8SyncPbbufCnt),
		DMX_UINT64_10U_LOG(g_rPsrPfm.rInst[u4InstID].rSyncPb.u8SyncPbbufTime));
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_PFM,
		TEXT("[DMX_PFM] Cfa -- RunCnt: "DMX_UINT64_10U_LOGSTR)
		TEXT(", Time: "DMX_UINT64_10U_LOGSTR" ms\r\n"),
		DMX_UINT64_10U_LOG(g_rPsrPfm.rInst[u4InstID].rCfa.u8CfaRunCnt),
		DMX_UINT64_10U_LOG(g_rPsrPfm.rInst[u4InstID].rCfa.u4CfaRunTime));

	for (i = 0; i < MAX_STREAM_INSTANCE_CNT; i++) {
		prStmInst = &(g_rDmxStmMan.arStmInst[i]);
		if (prStmInst->pvSptHdl == pvSptHdl) {
			switch (prStmInst->u4StmType) {
			case SPT_DATA_V:
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_PFM,
				    TEXT("[DMX_PFM] Video -- ")
				    TEXT("Pb2FifoCnt: "DMX_UINT64_10U_LOGSTR)
				    TEXT("("DMX_UINT64_10U_LOGSTR" ms), ")
				    TEXT(" HWTxCnt: "DMX_UINT64_10U_LOGSTR)
				    TEXT("("DMX_UINT64_10U_LOGSTR" ms), ")
				    TEXT(", PsrEsmCnt: "DMX_UINT64_10U_LOGSTR)
				    TEXT("("DMX_UINT64_10U_LOGSTR" ms), ")
				    TEXT(", CreateAUCnt: "DMX_UINT64_10U_LOGSTR"\r\n"),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rVideo.u8Pb2FifoCnt),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rVideo.u8Pb2FifoTime),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rVideo.u8HWTxCompleteCnt),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rVideo.u8HWTxTime),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rVideo.u8PsrEsmCnt),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rVideo.u8PsrEsmTime),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rVideo.u8CreateAUCnt));
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_PFM,
					TEXT("[DMX_PFM] DivxDRM -- DecryptV")
					TEXT(", DecryptCnt: "DMX_UINT64_10U_LOGSTR)
					TEXT(", DecryptPb2FifoCnt: "DMX_UINT64_10U_LOGSTR)
					TEXT(", DecryptTime: "DMX_UINT64_10U_LOGSTR" ms\r\n"),
				    DMX_UINT64_10U_LOG(g_rPsrPfm.rDecryptV.u8DecryptCnt),
				    DMX_UINT64_10U_LOG(g_rPsrPfm.rDecryptV.u8DecryptPb2FifoCnt),
				    DMX_UINT64_10U_LOG(g_rPsrPfm.rDecryptV.u8DecryptTime));
				break;
			case SPT_DATA_A:
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_PFM,
				    TEXT("[DMX_PFM] Audio -- ")
				    TEXT("Pb2FifoCnt: "DMX_UINT64_10U_LOGSTR)
				    TEXT("("DMX_UINT64_10U_LOGSTR" ms), ")
				    TEXT(" HWTxCnt: "DMX_UINT64_10U_LOGSTR)
				    TEXT("("DMX_UINT64_10U_LOGSTR" ms), ")
				    TEXT(", PsrEsmCnt: "DMX_UINT64_10U_LOGSTR)
				    TEXT("("DMX_UINT64_10U_LOGSTR" ms), ")
				    TEXT(", CreateAUCnt: "DMX_UINT64_10U_LOGSTR"\r\n"),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rAudio.u8Pb2FifoCnt),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rAudio.u8Pb2FifoTime),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rAudio.u8HWTxCompleteCnt),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rAudio.u8HWTxTime),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rAudio.u8PsrEsmCnt),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rAudio.u8PsrEsmTime),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rAudio.u8CreateAUCnt));
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_PFM,
					TEXT("[DMX_PFM] DivxDRM -- DecryptA")
					TEXT(", DecryptCnt: "DMX_UINT64_10U_LOGSTR)
					TEXT(", DecryptPb2FifoCnt: "DMX_UINT64_10U_LOGSTR)
					TEXT(", DecryptTime: "DMX_UINT64_10U_LOGSTR" ms\r\n"),
				    DMX_UINT64_10U_LOG(g_rPsrPfm.rDecryptA.u8DecryptCnt),
				    DMX_UINT64_10U_LOG(g_rPsrPfm.rDecryptA.u8DecryptPb2FifoCnt),
				    DMX_UINT64_10U_LOG(g_rPsrPfm.rDecryptA.u8DecryptTime));
				break;
			case SPT_DATA_SP:
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_PFM,
				    TEXT("[DMX_PFM] SP -- ")
				    TEXT("Pb2FifoCnt: "DMX_UINT64_10U_LOGSTR)
				    TEXT("("DMX_UINT64_10U_LOGSTR" ms), ")
				    TEXT(" HWTxCnt: "DMX_UINT64_10U_LOGSTR)
				    TEXT("("DMX_UINT64_10U_LOGSTR" ms), ")
				    TEXT(", PsrEsmCnt: "DMX_UINT64_10U_LOGSTR)
				    TEXT("("DMX_UINT64_10U_LOGSTR" ms), ")
				    TEXT(", CreateAUCnt: "DMX_UINT64_10U_LOGSTR"\r\n"),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rSP.u8Pb2FifoCnt),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rSP.u8Pb2FifoTime),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rSP.u8HWTxCompleteCnt),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rSP.u8HWTxTime),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rSP.u8PsrEsmCnt),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rSP.u8PsrEsmTime),
					DMX_UINT64_10U_LOG(g_rPsrPfm.rSP.u8CreateAUCnt));
				break;
			default:
				break;
			}
		}
	}
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_PFM,
		TEXT("[DMX_PFM] ============================ pvSptHdl: 0x%x, SptID: %d ")
		TEXT("END ======================================\r\n"),
		pvSptHdl, u4InstID);
}


#endif	/* DMX_PFM_TEST*/
