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
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/module.h>
#include "windows.h"
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/dmx_cfa_def.h>
#include <media/atc/ioctl_dmx.h>
#include <media/atc/drv_esm_if.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "dmx_splitter.h"
#include "dmx_cfa_def.h"
#include "ioctl_dmx.h"
#include "drv_esm_if.h"
#include "mm_debug.h"
#endif /* __linux__ */

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
#include "dmx_inst.h"
#include "dmx_log.h"

#ifndef __linux__
#pragma warning(disable : 4127) /* disable warning C4127: conditional expression is constant */
#endif

EXTERN DMX_INST_LIST_T g_rDmxInstList;
EXTERN DMX_CLI_MAN_T g_rDmxCliMan;
EXTERN DMX_SPT_MAN_INFO_T g_rSptMan;


void DmxCliInit(void)
{
	mm_memset(&g_rDmxCliMan, 0, sizeof(DMX_CLI_MAN_T));
}

void DmxCliDeInit(void)
{
	mm_memset(&g_rDmxCliMan, 0, sizeof(DMX_CLI_MAN_T));
}

/*******************************************************************************
 *_CLI_DMXTurnOnLog
 * Function: Turn On Demuxer's Log
 * u4LogLevel -- Log level ID, it should be the following value:
 * 1) =1. Debug Level
 * 2) =2. Warning Level
 * 3) =3. Error Level
 * 4) =4. Fatal Level
 * 5) =none. All Level Log
 * u4OnOff -- Turn On or Off
 * 1) =0. Turn off
 * 2) =1. Turn on
*******************************************************************************/
MRESULT DmxCliTurnOnOffLog(u32 u4OnOff, u32 u4LogLevel, u32 u4Module, u32 u4ModLogLvl)
{
	DMX_INST_T *prDmxInst = NULL;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx1 = 0;
	bool fgEnable = FALSE;

	if (0 == u4OnOff)
		fgEnable = FALSE;
	else
		fgEnable = TRUE;

	if (u4ModLogLvl > 31)
		u4ModLogLvl = -1;
	else
		u4ModLogLvl = (1 << u4ModLogLvl);

	if (u4Module >= DMX_MOD_HW) {
		DmxLogEnable(fgEnable, u4LogLevel, u4Module, u4ModLogLvl);
		MM_RETURN(RET_DMX_OK);
	}

	DmxInstListLockSema();

	prDmxInst = g_rDmxInstList.prHead;
	while (NULL != prDmxInst) {
		DMXLOG_TRACE(
			(TEXT("////////////////////////////////////////////////////////////////////")
			TEXT("///////////////////////////////////////////////////////\r\n")));
		DMXLOG_TRACE(
			TEXT("[CLI]  DMX Instance(0x%x) information list as following =====================\r\n"),
			prDmxInst);
		DmxInstLockSema(prDmxInst);
		for (u4Idx1 = 0;
			 ((u4Idx1 < MAX_SPT_INST_CNT_PER_DMX) && (u4Idx1 < prDmxInst->u4SptCnt));
			 u4Idx1++) {
			prSpt = prDmxInst->parSpt[u4Idx1];
			if (NULL == prSpt)
				break;

			if (!prSpt->fgCreated)
				continue;

			SptCfaProcCliCmd(prSpt, DMX_CFA_CLI_CMD_TURN_ONOFF_LOG,
				 u4OnOff, u4LogLevel, u4ModLogLvl, NULL);
		}

		DmxInstReleaseSema(prDmxInst);
		DMXLOG_TRACE(
			(TEXT("[CLI]  ===================== DMX Instance(0x%x)")
			TEXT(" information List End =====================\r\n")),
			prDmxInst);
		DMXLOG_TRACE(
			(TEXT("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX")
			TEXT("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\r\n")));
		prDmxInst = prDmxInst->prNext;
	}

	DmxInstListReleaseSema();

	MM_RETURN(RET_DMX_OK);
}

static MRESULT DmxCliDumpStmFifoInfo(uintptr_t pvSptHdl, u32 u4SptDataType)
{
	u32	   u4FifoAvailSize = 0;
	u32	   u4FifoSize	   = 0;
	u32	   u4FifoSelfASize = 0;
	u32	   u4AvailCnt	   = 0;
	u32	   u4TotalCnt	   = 0;
	HANDLE		hStm		= NULL;
	PSR_FILTER *pPsr		   = NULL;
	PSR_FILTER *pActPsr		= NULL;
	DMX_FIFO_INFO_T *pFifo	   = NULL;
	MRESULT    mrRet		   = RET_DMX_OK;

	hStm = GetStreamByType(pvSptHdl, u4SptDataType);
	if ((NULL != hStm) &&
		(StreamIsEnabled(hStm))) {
		pPsr = (PSR_FILTER *)(((DMX_STM_INST_T *)hStm)->pvPsrFtr);

		mrRet = ESM_FifoGetInfo(pPsr->u4ESIH, (void **)&pFifo);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[CLI] %s fail in ESM_FifoGetInfo(Video), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, mrRet);
			MM_RETURN(mrRet);
		}

		mrRet = ESM_AUTableGetTotalCount(pPsr->u4ESIH, &u4TotalCnt);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[CLI] %s line %d fail in ")
				TEXT("ESM_AUTableGetTotalCount(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pPsr->u4ESIH, mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		mrRet = ESM_AUTableGetAvailCount(pPsr->u4ESIH, &u4AvailCnt);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[CLI] %s line %d fail in ")
				TEXT("ESM_AUTableGetAvailCount(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pPsr->u4ESIH, mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		mrRet = ESM_FifoGetAvailDataSize(pPsr->u4ESIH, &u4FifoAvailSize);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[CLI] %s line %d fail in ")
				TEXT("ESM_FifoGetAvailDataSize(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pPsr->u4ESIH, mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		mrRet = ESM_FifoGetSelfAvailDataSize(pPsr->u4ESIH, &u4FifoSelfASize);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[CLI] %s line %d fail in")
				TEXT(" ESM_FifoGetSelfAvailDataSize(u4Handle: 0x%x), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pPsr->u4ESIH, mrRet);
			DMX_ASSERT(FALSE);
			MM_RETURN(mrRet);
		}

		u4FifoSize = pFifo->ptrEa - pFifo->ptrSa;

		ESM_PrintFifoInfo(pPsr->u4ESIH);

		if (SPT_DATA_A == u4SptDataType) {
			DMXLOG_TRACE(
				TEXT("[CLI] %s ----------> DmxInst: 0x%x, Fifo Total Size: %d,")
				TEXT(" Fifo Data Size: %d, Self Fifo Data Size: %d\r\n"),
				g_aszSptDataTypeName[SPT_DATA_A], pPsr->pvDmxInst,
				u4FifoSize, u4FifoAvailSize, u4FifoSelfASize);
			GAU_PrintLogAUInfo();
		} else {
			DMXLOG_TRACE(
				TEXT("[CLI] %s ----------> DmxInst: 0x%x, Fifo Total Size: %d,")
				TEXT(" Fifo Data Size: %d\r\n"),
				((u4SptDataType < MAX_SPT_DATA_TYPE_CNT) ?
					g_aszSptDataTypeName[u4SptDataType] : TEXT("UNKNOWN")),
				pPsr->pvDmxInst, u4FifoSize, u4FifoAvailSize);
		}
		DMXLOG_TRACE(
			TEXT("[CLI] %s ----------> DmxInst: 0x%x, Total AU Count: %d, Avail AU Count: %d\r\n"),
			((u4SptDataType < MAX_SPT_DATA_TYPE_CNT) ?
				g_aszSptDataTypeName[u4SptDataType] : TEXT("UNKNOWN")),
			pPsr->pvDmxInst, u4TotalCnt, u4AvailCnt);

		if (NULL != pPsr->pvPsrCC) {
			pActPsr = (PSR_FILTER *)(((PSR_CC *)(pPsr->pvPsrCC))->pvActFilter);
			DMXLOG_TRACE(
				TEXT("[CLI] %s ----------> DmxInst: 0x%x, pvSptHdl: 0x%x PsrCC(0x%x)'s eTxState: %d,")
				TEXT(" eState: 0x%x, ActFilter's eType: 0x%x\r\n"),
				((u4SptDataType < MAX_SPT_DATA_TYPE_CNT) ?
					g_aszSptDataTypeName[u4SptDataType] : TEXT("UNKNOWN")),
				pPsr->pvDmxInst, ((PSR_CC *)(pPsr->pvPsrCC))->pvSptHdl, pPsr->pvPsrCC,
				((PSR_CC *)(pPsr->pvPsrCC))->eTxState,
				((PSR_CC *)(pPsr->pvPsrCC))->eState,
				((NULL != pActPsr) ? (pActPsr->eType) : SPT_DATA_UNDEFINE));
		}
	}

	MM_RETURN(RET_DMX_OK);
}

/*******************************************************************************
 *_CLI_DMXDumpFifoInfo
 * Function: Dump Demuxer's one or several stream's Fifo and AU info
 * u4StmTypeID -- Dump Stream Type ID, it should be the following value:
 * 1) =1. Video
 * 2) =2. Audio
 * 3) =3. SP/CC
 * 3) =4. Section
 * 4) =none. All Streams
*******************************************************************************/
MRESULT DmxCliDumpFifoInfo(u32 u4StmTypeID)
{
	u32	   u4Idx		   = 0;
	MRESULT    mrRet		   = RET_DMX_OK;

	if ((!g_rSptMan.fgSptInitial) ||
		(NULL == g_rSptMan.aprSptInst[0])) {
		DMXLOG_TRACE(TEXT("[CLI] %s exit, no used splitter instances\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	for (u4Idx = 0; u4Idx < DMX_MAX_SPT_INST_CNT; u4Idx++) {
		if (NULL == g_rSptMan.aprSptInst[u4Idx])
			break;

		if (!g_rSptMan.aprSptInst[u4Idx]->fgCreated)
			continue;

		switch (u4StmTypeID) {
		case 1:
			mrRet = DmxCliDumpStmFifoInfo((uintptr_t)(g_rSptMan.aprSptInst[u4Idx]),
				SPT_DATA_V);
			break;
		case 2:
			mrRet = DmxCliDumpStmFifoInfo((uintptr_t)(g_rSptMan.aprSptInst[u4Idx]),
				SPT_DATA_A);
			break;
		case 3:
			mrRet = DmxCliDumpStmFifoInfo((uintptr_t)(g_rSptMan.aprSptInst[u4Idx]),
				SPT_DATA_SP);
			break;
		case 4:
			mrRet = DmxCliDumpStmFifoInfo((uintptr_t)(g_rSptMan.aprSptInst[u4Idx]),
				SPT_DATA_SECTION);
			break;
		case ((u32)(-1)): /* ALl */
			DmxCliDumpStmFifoInfo((uintptr_t)(g_rSptMan.aprSptInst[u4Idx]),
				SPT_DATA_V);
			DmxCliDumpStmFifoInfo((uintptr_t)(g_rSptMan.aprSptInst[u4Idx]),
				SPT_DATA_A);
			DmxCliDumpStmFifoInfo((uintptr_t)(g_rSptMan.aprSptInst[u4Idx]),
				SPT_DATA_SP);
			DmxCliDumpStmFifoInfo((uintptr_t)(g_rSptMan.aprSptInst[u4Idx]),
				SPT_DATA_SECTION);
			mrRet = RET_DMX_OK;
			break;
		default:
			DMXLOG_ERROR(
				TEXT("[CLI] %s fail for error dump stream type id: %d,")
				TEXT(" (only support 1~4 and -1)\r\n"),
				DMX_FUNC_NAME, u4StmTypeID);
			MM_RETURN(RET_DMX_NO_IMPLEMENT);
		}
	}
	
	GAU_DumpInfo();
	MM_RETURN(mrRet);
}

/*******************************************************************************
 *DmxCliDumpPbbufInfo
 * Function: Dump Demuxer's Pbbufs' Info
 * No Param
*******************************************************************************/
MRESULT DmxCliDumpPbbufInfo(u32 u4DumpData)
{
	u32 u4Idx = 0;
	bool   fgDumpData = ((0 == u4DumpData) ? FALSE : TRUE);

	if ((!g_rSptMan.fgSptInitial) ||
		(NULL == g_rSptMan.aprSptInst[0])) {
		DMXLOG_TRACE(TEXT("[CLI] %s exit, no used splitter instances\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	for (u4Idx = 0; u4Idx < DMX_MAX_SPT_INST_CNT; u4Idx++) {
		if (NULL == g_rSptMan.aprSptInst[u4Idx])
			break;

		if (!g_rSptMan.aprSptInst[u4Idx]->fgCreated)
			continue;

		PBBUF_DumpInfo(g_rSptMan.aprSptInst[u4Idx], fgDumpData);
	}

	MM_RETURN(RET_DMX_OK);
}

/*******************************************************************************
 *DmxCliDumpPbbufInfo
 * Function: Dump Demuxer's Pbbufs' Info
 * No Param
*******************************************************************************/
MRESULT DmxCliDumpGAUInfo(void)
{
	MRESULT mrRet = RET_DMX_OK;


	if ((!g_rSptMan.fgSptInitial) ||
		(NULL == g_rSptMan.aprSptInst[0])) {
		DMXLOG_TRACE(TEXT("[CLI] %s exit, no used splitter instances\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	mrRet = GAU_DumpInfo();

	MM_RETURN(mrRet);
}

/*******************************************************************************
 * DmxDumpThresholdInfo
 * Function: Dump Demuxer's one or several stream's Fifo and AU info
 * u4SptDataType -- Dump Stream Type ID, it should be the following value:
 * 1) =1. Video
 * 2) =2. Audio
*******************************************************************************/
static MRESULT DmxDumpThresholdInfo(DMX_SPT_INST_T *prSpt, u32 u4SptDataType)
{
	DMX_STM_INST_T *prStm		   = NULL;

	prStm = GetStreamByType(prSpt, u4SptDataType);

	if (NULL == prStm) {
		DMXLOG_ERROR(TEXT("[CLI] %s fail for can't find the stream type: %d\r\n"),
			DMX_FUNC_NAME, u4SptDataType);
		MM_RETURN(RET_DMX_OK);
	}

	if (StreamIsEnabled(prStm)) {
		if (SPT_DATA_V == u4SptDataType) {
			DMXLOG_ERROR(TEXT("[CLI] Video --> Threshold ( ThresholdSz: %d ) \r\n"),
				prStm->u4FifoThreshold);
		} else if (SPT_DATA_A == u4SptDataType) {
			DMXLOG_ERROR(TEXT("[CLI] Audio --> Threshold ( ThresholdSz: %d ) \r\n"),
				prStm->u4FifoThreshold);
		}
	} else {
		DMXLOG_ERROR(TEXT("[CLI] %s fail, stream (type: %d) is disable\r\n"),
			DMX_FUNC_NAME, u4SptDataType);
		MM_RETURN(RET_DMX_OK);
	}

	MM_RETURN(RET_DMX_OK);
}

/*******************************************************************************
 * DmxCliDumpThresholdInfo
 * Function: Dump Demuxer's one or several stream's threshold info
 * u4StmTypeID -- Dump Stream Type ID, it should be the following value:
 * 1) =1. Video
 * 2) =2. Audio
 * 3) =none. Both Video and Audio
*******************************************************************************/
MRESULT DmxCliDumpThresholdInfo(u32 u4StmTypeID)
{
	DMX_SPT_INST_T *prSpt = NULL;
	u32		   u4Idx  = 0;

	if ((!g_rSptMan.fgSptInitial) ||
		(NULL == g_rSptMan.aprSptInst[0])) {
		DMXLOG_TRACE(TEXT("[CLI] %s exit, no used splitter instances\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	prSpt  = NULL;
	for (u4Idx = 0; u4Idx < DMX_MAX_SPT_INST_CNT; u4Idx++) {
		if (NULL == g_rSptMan.aprSptInst[u4Idx])
			break;

		if (!g_rSptMan.aprSptInst[u4Idx]->fgCreated)
			continue;

		prSpt = g_rSptMan.aprSptInst[u4Idx];
	}

	if (NULL == prSpt) {
		DMXLOG_ERROR(TEXT("[CLI] %s fail for no used splitter instance\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	if (1 == u4StmTypeID) {
		DmxDumpThresholdInfo(prSpt, SPT_DATA_V);
	} else if (2 == u4StmTypeID) {
		DmxDumpThresholdInfo(prSpt, SPT_DATA_A);
	} else {
		DmxDumpThresholdInfo(prSpt, SPT_DATA_V);
		DmxDumpThresholdInfo(prSpt, SPT_DATA_A);
	}
	

	MM_RETURN(RET_DMX_OK);
}

static	MRESULT DmxEnableThresholdInfo(u32 u4StmTypeID, u32 u4Enable)
{
	u32 u4StmType	   = 0;
	u32 u4Idx		   = 0;
	DMX_STM_INST_T *prStm  = NULL;
	DMX_SPT_INST_T *prSpt  = NULL;
	MRESULT mrRet		   = RET_DMX_OK;

	for (u4Idx = 0; u4Idx < DMX_MAX_SPT_INST_CNT; u4Idx++) {
		if (NULL == g_rSptMan.aprSptInst[u4Idx])
			break;

		if (!g_rSptMan.aprSptInst[u4Idx]->fgCreated)
			continue;

		prSpt = g_rSptMan.aprSptInst[u4Idx];
	}

	if (NULL == prSpt) {
		DMXLOG_ERROR(TEXT("[CLI] %s fail for no used splitter instance\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	if (1 == u4StmTypeID) {
		u4StmType = SPT_DATA_V;
	} else if (2 == u4StmTypeID) {
		u4StmType = SPT_DATA_A;
	} else {
		DMXLOG_ERROR(TEXT("[CLI] %s fail for unsupported param, u4StmTypeID: %d\r\n"),
			DMX_FUNC_NAME, u4StmTypeID);
		MM_RETURN(RET_DMX_OK);
	}

	prStm = GetStreamByType(prSpt, u4StmType);
	if ((NULL != prStm) &&
		(StreamIsEnabled(prStm))) {
		if (0 == u4Enable) { /* clear threshold */
			mrRet = GAU_SetThreshold((u32)(prStm->u4GAUHandle), 0);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT("[CLI] %s fail in GAU_SetThresholdInfo, strmtype: 0x%x\r\n"),
					DMX_FUNC_NAME, prStm->u4StmType);
				MM_RETURN(RET_DMX_OK);
			}
		} else { /* enable threshold */
			mrRet = GAU_SetThreshold((u32)(prStm->u4GAUHandle), prStm->u4FifoThreshold);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT("[CLI] %s fail in GAU_SetThresholdInfo, strmtype: 0x%x\r\n"),
					DMX_FUNC_NAME, prStm->u4StmType);
				MM_RETURN(RET_DMX_OK);
			}
		}
	}
	MM_RETURN(RET_DMX_OK);
}

/*******************************************************************************
 * DmxCliEnableThreshold
 * Function: Enable/Disable Demuxer's one or several stream's threshold info
 * u4StmTypeID -- Dump Stream Type ID, it should be the following value:
 * 1) =1. Video
 * 2) =2. Audio
 * u4Enabled   -- Indication whether to enable or disable threshold
*******************************************************************************/
MRESULT DmxCliEnableThreshold(u32 u4StmTypeID, u32 u4Enabled)
{
	MRESULT mrRet = RET_DMX_OK;

	if ((!g_rSptMan.fgSptInitial) ||
		(NULL == g_rSptMan.aprSptInst[0])) {
		DMXLOG_TRACE(TEXT("[CLI] %s exit, no used splitter instances\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}
	
	mrRet = DmxEnableThresholdInfo(u4StmTypeID, u4Enabled);
		

	MM_RETURN(mrRet);
}

/*******************************************************************************
 * DmxCliPrintGetAULog
 * Function: Turn on / off GetAU Log
 * u4Enabled -- Indicate whether to turn on/off GetAU Log
 * 1) =0. Turn Off
 * 2) =1. Turn On
*******************************************************************************/
MRESULT DmxCliPrintGetAULog(u32 u4StmType, u32 u4Enabled)
{
	bool fgEnable = (0 == u4Enabled) ? FALSE : TRUE;

	switch (u4StmType) {
	case 1: /* video */
		DmxLogDEnable(fgEnable, DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_V);
		break;
	case 2: /* audio */
		DmxLogDEnable(fgEnable, DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_A);
		break;
	case 3: /* cc */
		DmxLogDEnable(fgEnable, DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SP);
		break;
	case 4: /* section */
		DmxLogDEnable(fgEnable, DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SEC);
		break;
	default:
		DmxLogDEnable(fgEnable, DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_V);
		DmxLogDEnable(fgEnable, DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_A);
		DmxLogDEnable(fgEnable, DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SP);
		DmxLogDEnable(fgEnable, DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_SEC);
		break;
	}

	MM_RETURN(RET_DMX_OK);
}

/*******************************************************************************
 *_CLI_DMXDumpState
 * Function: Dump Demuxer's Splitter, Parser CC, Parser Filter' Info
 * u4DumpLevel -- Dump level ID, it should be the following value:
 * 1) =1. Splitter's Current Info
 * 2) =2. Parser CC's Current Info
 * 3) =3. Parser Filter's Current Info
 * 3) =none. All Info
*******************************************************************************/
MRESULT DmxCliDumpInstsInfo(u32 u4DumpLevel)
{
	DMX_INST_T	*prDmxInst	 = NULL;
	DMX_SPT_INST_T *prSpt	= NULL;
	PSR_CC	   *prPsrCC	= NULL;
	PSR_FILTER *prPsrFtr	= NULL;
	bool		fgDumpSpt	 = FALSE;
	bool		fgDumpPsrCC  = FALSE;
	bool		fgDumpPsrFtr = FALSE;
	u32		u4Idx1		 = 0;

	if ((!g_rSptMan.fgSptInitial) ||
		(NULL == g_rSptMan.aprSptInst[0])) {
		DMXLOG_TRACE(TEXT("[CLI] %s exit, no used splitter instances\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	switch (u4DumpLevel) {
	case 1:
		fgDumpSpt	 = TRUE;
		break;
	case 2:
		fgDumpPsrCC  = TRUE;
		break;
	case 3:
		fgDumpPsrFtr = TRUE;
		break;
	default:
		fgDumpSpt	 = TRUE;
		fgDumpPsrCC  = TRUE;
		fgDumpPsrFtr = TRUE;
		break;
	}
	
	DmxInstListLockSema();

	prDmxInst = g_rDmxInstList.prHead;
	while (NULL != prDmxInst) {
		DMXLOG_TRACE(
			(TEXT("////////////////////////////////////////////////////////////////////////////////")
			TEXT("///////////////////////////////////////////\r\n")));
		DMXLOG_TRACE(
			TEXT("[CLI]  DMX Instance(0x%x) information list as following, u4DumpLevel=%d, u4SptCnt: %d =====================\r\n"),
			prDmxInst, u4DumpLevel, prDmxInst->u4SptCnt);
		DmxInstLockSema(prDmxInst);
		for (u4Idx1 = 0;
			 ((u4Idx1 < MAX_SPT_INST_CNT_PER_DMX) && (u4Idx1 < prDmxInst->u4SptCnt));
			 u4Idx1++) {
			prSpt = prDmxInst->parSpt[u4Idx1];
			if (NULL == prSpt) {
				DMXLOG_TRACE(
					TEXT("[CLI]  DMX Instance(0x%x) 's parSpt[%u] == NULL\r\n"),
					prDmxInst, u4Idx1);
				break;
			}
			if (!prSpt->fgCreated)
				continue;

			if (fgDumpSpt)
				DmxDumpSptInfo(prSpt);

			prPsrCC = (PSR_CC *)(prSpt->pvPsrCC);

			if (NULL == prPsrCC)
				continue;

			if (fgDumpPsrCC)
				DmxDumpPsrCCInfo(prPsrCC);

			if (fgDumpPsrFtr) {
				u32 u4Idx2 = 0;

				for (u4Idx2 = 0; u4Idx2 < MAX_PSR_FILTER_PER_CC; u4Idx2++) {
					prPsrFtr = (PSR_FILTER *)(prPsrCC->apvFtr[u4Idx2]);
					if (NULL == prPsrFtr)
						break;
					DmxDumpPsrFilterInfo((uintptr_t)prPsrFtr);
				}
			}
			#if DMX_PFM_TEST
			DmxPfmPrintInfo(prSpt);
			#endif /* DMX_PFM_TEST */

			SptCfaProcCliCmd(prSpt, DMX_CFA_CLI_CMD_DUMP_INFO, 0, 0, 0, NULL);
		}

		DmxInstReleaseSema(prDmxInst);
		DMXLOG_TRACE(
			TEXT("[CLI]  ===================== DMX Instance(0x%x)")
			TEXT(" information List End =====================\r\n"),
			prDmxInst);
		DMXLOG_TRACE(
			TEXT("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX")
			TEXT("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\r\n"));
		prDmxInst = prDmxInst->prNext;
	}

	DmxInstListReleaseSema();

	PSR_HAL_DumpPidStructInfo(0);
	PSR_HAL_DumpPidStructInfo(1);
	PSR_HAL_DumpPidStructInfo(2);

	MM_RETURN(RET_DMX_OK);
}


MRESULT DmxCliDumpPidStructure(u32 u4Pidx)
{
	u8 u1Pidx = 0;

	if ((!g_rSptMan.fgSptInitial) ||
		(NULL == g_rSptMan.aprSptInst[0])) {
		DMXLOG_TRACE(TEXT("[CLI] %s exit, no used splitter instances\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	if (3 <= u4Pidx) {
		PSR_HAL_DumpPidStructInfo(0);
		PSR_HAL_DumpPidStructInfo(1);
		PSR_HAL_DumpPidStructInfo(2);
		PSR_HAL_DumpPidStructInfo(10);
	} else {
		u1Pidx = (u8)u4Pidx;

		PSR_HAL_DumpPidStructInfo(u1Pidx);
	}
	
	PSR_HAL_DumpDDIInfo();

	MM_RETURN(RET_DMX_OK);
}

MRESULT DmxCliDumpFlow(u32 u4Enable, char *wszDirname)
{
	if (0 != u4Enable) {
		if (!DmxCreateDumpFlowFile(wszDirname)) {
			DMXLOG_ERROR(
				TEXT("[CLI] ++++++++ %s fail for create dump flow files, Dir: %s\r\n"),
				DMX_FUNC_NAME, wszDirname);
			MM_RETURN(RET_DMX_OK);
		}
		g_rDmxCliMan.fgDumpFlow = TRUE;
	} else {
		g_rDmxCliMan.fgDumpFlow = FALSE;
		DmxCloseDumpFlowFile();
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT DmxCliDumpAllAUData(u32 u4StmType, u32 u4Enable, char *wszDirname)
{
	if ((0 < u4Enable) &&
		(NULL == wszDirname)) {
		DMXLOG_ERROR(TEXT("[CLI] ++++++++ %s fail for invalid args wszDirname\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	switch (u4StmType) {
	case 1:
		if (0 != u4Enable) {
			if (!DmxCreateDumpVFile(wszDirname)) {
				DMXLOG_ERROR(
					TEXT("[CLI] ++++++++ %s fail for create Video dump files, Dir: %s\r\n"),
					DMX_FUNC_NAME, wszDirname);
				MM_RETURN(RET_DMX_OK);
			}
			g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_VID] = TRUE;
		} else {
			g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_VID] = FALSE;
			DmxCloseDumpVFile();
		}
		break;
	case 2:
		if (0 != u4Enable) {
			if (!DmxCreateDumpAFile(wszDirname)) {
				DMXLOG_ERROR(
					TEXT("[CLI] ++++++++ %s fail for create Audio dump files, Dir: %s\r\n"),
					DMX_FUNC_NAME, wszDirname);
				MM_RETURN(RET_DMX_OK);
			}
			g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_AUD] = TRUE;
		} else {
			g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_AUD] = FALSE;
			DmxCloseDumpAFile();
		}
		break;
	case 3:
		if (0 != u4Enable) {
			if (!DmxCreateDumpSPFile(wszDirname)) {
				DMXLOG_ERROR(
					TEXT("[CLI] ++++++++ %s fail for create SP dump files, Dir: %s\r\n"),
					DMX_FUNC_NAME, wszDirname);
				MM_RETURN(RET_DMX_OK);
			}
			g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_SP] = TRUE;
		} else {
			g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_SP] = FALSE;
			DmxCloseDumpSPFile();
		}
		break;

	default:
		DMXLOG_ERROR(
			TEXT("[CLI] ++++++++ %s fail for unsupported to dump this stream's data\r\n"),
			DMX_FUNC_NAME);
		break;
	}

	MM_RETURN(RET_DMX_OK);
}
MRESULT DmxCliDumpMemUsage(void)
{

	if ((!g_rSptMan.fgSptInitial) ||
		(NULL == g_rSptMan.aprSptInst[0])) {
		DMXLOG_TRACE(TEXT("[CLI] %s exit, no used splitter instances\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	DMX_Dump_Mem();

	DMXLOG_TRACE(TEXT("/***********OSE_MEM status*************************/\r\n"));

	/* OSE_PrintOSEMemoryCfg(); */

	MM_RETURN(RET_DMX_OK);
}

/*******************************************************************************
 *DmxCliDumpRspInfo
 * Function: Dump Demuxer's Pbbufs' Info
 * No Param
*******************************************************************************/
MRESULT DmxCliDumpRspInfo(u32 u4Dump)
{
	u32 u4Idx = 0;
	bool   fgDump = ((0 == u4Dump) ? FALSE : TRUE);

	if ((!g_rSptMan.fgSptInitial) ||
		(NULL == g_rSptMan.aprSptInst[0])) {
		DMXLOG_TRACE(TEXT("[CLI] %s exit, no used splitter instances\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	for (u4Idx = 0; u4Idx < DMX_MAX_SPT_INST_CNT; u4Idx++) {
		if (NULL == g_rSptMan.aprSptInst[u4Idx])
			break;

		if (fgDump) {
			if (!DmxCreateDumpRspFile(g_rSptMan.aprSptInst[u4Idx]->u4SptCompId)) {
				DMXLOG_ERROR(
					TEXT("[CLI] %s fail in DmxCreateDumpRspFile(SptId: %d, pvSptHdl: 0x%x)\r\n"),
					DMX_FUNC_NAME, g_rSptMan.aprSptInst[u4Idx]->u4SptCompId,
					g_rSptMan.aprSptInst[u4Idx]);
			} else {
				g_rDmxCliMan.fgDumpRspInfo = TRUE;
			}
		} else {
			g_rDmxCliMan.fgDumpRspInfo = FALSE;
			DmxCloseDumpRspFile(g_rSptMan.aprSptInst[u4Idx]->u4SptCompId);
		}
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT DmxCliDumpRegisters(u32 u41stRegAddr, u32 u4RegCnt)
{
	if ((!g_rSptMan.fgSptInitial) ||
		(NULL == g_rSptMan.aprSptInst[0])) {
		DMXLOG_TRACE(TEXT("[CLI] %s exit, no used splitter instances\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}
	
	DMXDumpRegisters(u41stRegAddr, u4RegCnt);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DmxCliDumpAUInfo(u32 u4StmType, u32 u4StartIdx, u32 u4EndIdx)
{
	E_SPT_DATA_TYPE_T u4SptDataType = SPT_DATA_UNDEFINE;
	uintptr_t pvStm  = 0;
	PSR_FILTER *pPsr = NULL;
	u32	   u4Idx = 0;

	if ((!g_rSptMan.fgSptInitial) ||
		(NULL == g_rSptMan.aprSptInst[0])) {
		DMXLOG_TRACE(TEXT("[CLI] %s exit, no used splitter instances\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	switch (u4StmType) {
	case 1: /* video */
		u4SptDataType = SPT_DATA_V;
		break;
	case 2: /* audio */
		u4SptDataType = SPT_DATA_A;
		break;
	case 3: /* cc */
		u4SptDataType = SPT_DATA_SP;
		break;
	case 4: /* section */
		u4SptDataType = SPT_DATA_SECTION;
		break;
	default:
		MM_RETURN(RET_DMX_OK);
	}

	for (u4Idx = 0; u4Idx < DMX_MAX_SPT_INST_CNT; u4Idx++) {
		if (NULL == g_rSptMan.aprSptInst[u4Idx])
			break;

		if (!g_rSptMan.aprSptInst[u4Idx]->fgCreated)
			continue;

		pvStm = GetStreamByType(g_rSptMan.aprSptInst[u4Idx], u4SptDataType);
		if ((NULL != pvStm) &&
			(StreamIsEnabled(pvStm))) {
			pPsr = (PSR_FILTER *)(((DMX_STM_INST_T *)pvStm)->pvPsrFtr);

			ESM_PrintAUInfo(pPsr->u4ESIH, u4StartIdx, u4EndIdx);
		}
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT DmxCliPrintPerfInfo(void)
{
	#if DMX_PFM_TEST
	u32 u4InstID = 0;

	for (u4InstID = 0; u4InstID < DMX_MAX_SPT_INST_CNT; u4InstID++) {
		if ((NULL != g_rSptMan.aprSptInst[u4InstID]) &&
			(g_rSptMan.aprSptInst[u4InstID]->fgCreated)) {
			DmxPfmPrintInfo(g_rSptMan.aprSptInst[u4InstID]);
		}

	}
	#endif /* DMX_PFM_TEST */

	MM_RETURN(RET_DMX_OK);
}

#ifdef __linux__
void  _DmxCliCmd(DMX_CLI_TYPE eDmxCliType, u32 arg1, u32 arg2, u32 arg3, u32 arg4, const char **pfilename)
{
	switch (eDmxCliType) {
	case DMX_CLI_CMD_TURN_ONOFF_LOG:
		DmxCliTurnOnOffLog(arg1, arg2, arg3, arg4);
		break;
	case DMX_CLI_CMD_DUMP_FIFO_INFO:
		DmxCliDumpFifoInfo(arg1);
		break;
	case DMX_CLI_CMD_DUMP_PBBUF_INFO:
		DmxCliDumpPbbufInfo(arg1);
		break;
	case DMX_CLI_CMD_DUMP_GAU_INFO:
		DmxCliDumpGAUInfo();
		break;
	case DMX_CLI_CMD_DUMP_THRESHOLD_INFO:
		DmxCliDumpThresholdInfo(arg1);
		break;
	case DMX_CLI_CMD_ENABLE_THRESHOLD:
		DmxCliEnableThreshold(arg1, arg2);
		break;
	case DMX_CLI_CMD_PRINT_AUGET_LOG:
		DmxCliPrintGetAULog(arg1, arg2);
		break;
	case DMX_CLI_CMD_DUMP_INSTS_INFO:
		DmxCliDumpInstsInfo(arg1);
		break;
	case DMX_CLI_CMD_DUMP_HW_INFO:
		DmxCliDumpPidStructure(arg1);
		break;
	case DMX_CLI_CMD_DUMP_ALLAUDATA:
		DmxCliDumpAllAUData(arg1, arg2, (char *)pfilename);
		break;
	case DMX_CLI_CMD_DUMP_MEM_USAGE:
		DmxCliDumpMemUsage();
		break;
	case DMX_CLI_CMD_DUMP_FLOW:
		DMXLOG_TRACE(TEXT("[SPT] %s line %d DMX_CLI_CMD_DUMP_FLOW(arg1: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, arg1);
		DmxCliDumpFlow(arg1, (char *)pfilename);
		break;
	case DMX_CLI_CMD_DUMP_REGISTERS:
		DmxCliDumpRegisters(arg1, arg2);
		break;
	case DMX_CLI_CMD_DUMP_AU_INFO:
		DmxCliDumpAUInfo(arg1, arg2, arg3);
		break;
	case DMX_CLI_CMD_PRINT_PERF_INFO:
		DmxCliPrintPerfInfo();
		break;
	default:
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail for Unsupport Cli cmd(0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDmxCliType);
		break;
	}
}
EXPORT_SYMBOL(_DmxCliCmd);

#endif /* __linux__ */
