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
 * @file dmx_spt_cfa.c
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
#endif /* __linux__*/

#ifdef __linux__
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/dmx_cfa_def.h>
/* #include <media/atc/mm_debug.h> */
#include <media/atc/drv_aud.h>
#if CONFIG_DRV_HDMI_RX
#include <media/atc/x_audin.h>
#endif /* CONFIG_DRV_HDMI_RX*/
#else
#include "dmx_define.h"
#include "dmx_splitter.h"
#include "dmx_cfa_def.h"
#include "drv_aud.h"
#if CONFIG_DRV_HDMI_RX
#include "x_audin.h"
#endif /* CONFIG_DRV_HDMI_RX*/
#include "mm_debug.h"
#endif /* __linux__*/

#include "dmx_esm_if.h"
#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_cpsa.h"
#include "dmx_spt.h"
#include "dmx_dump.h"
#include "dmx_spt_cfa.h"
#include "dmx_spt_main.h"
#include "dmx_spt_rsp.h"
#include "dmx_spt_psr.h"
#include "dmx_spt_util.h"
#include "dmx_spt_os.h"
#include "dmx_stream.h"
#include "dmx_parser.h"
#include "dmx_gau.h"
#include "dmx_gau_if.h"
#include "dmx_psr_cc.h"
#include "dmx_psr_filter.h"
#include "cfa_if.h"
#include "dmx_pfm.h"
#include "dmx_inst.h"

#ifndef __linux__
#pragma warning(disable : 4127) /*disable warning C4127: conditional expression is constant*/
#endif

#define FUNC_SPT_TO_CFA
#define FUNC_CFA_TO_SPT

#if DMX_PFM_TEST
EXTERN PSR_PFM g_rPsrPfm;
#endif /* DMX_PFM_TEST*/

EXTERN DMX_CLI_MAN_T g_rDmxCliMan;
EXTERN DMX_SPT_MAN_INFO_T g_rSptMan;
EXTERN AU_AUDIO g_arLogAudioAUs[DMX_MAX_LOG_AUDIO_AU_CNT];


#ifdef FUNC_SPT_TO_CFA
/*/////////////////////////////////////////////////////////////////////////////*/
/*///////////////Follow function called to CFA.////////////////////////////////*/
/*/////////////////////////////////////////////////////////////////////////////*/
void *SptGetCfaInterface(u32 u4CfaType)
{
	return CfaGetInterface(u4CfaType);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SptCfaInit*/
/* Call Current Cfa's Init function, set Private data of cfa*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SptCfaInit(void *pvSptHdl)
{
	CfaIntf *prCfaInterface = (CfaIntf *)SplitterGetCfaInterface(pvSptHdl);
	void	*pvCfaPrivateData = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prCfaInterface)
		MM_RETURN(RET_DMX_NO_CFA_INTERFACE);

	if (NULL == prCfaInterface->pfmrInit)
		MM_RETURN(RET_DMX_NO_IMPLEMENT);

	mrRet = prCfaInterface->pfmrInit(pvSptHdl, &pvCfaPrivateData);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in Cfa->pfmrInit, mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet, pvSptHdl);
		MM_RETURN(mrRet);
	}

	SplitterSetCfaPrivateData(pvSptHdl, pvCfaPrivateData);

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SptCfaUninit*/
/* Call Current Cfa's UnInit function, Set cfa's Private data to be NULL*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SptCfaUninit(void *pvSptHdl)
{
	CfaIntf *prCfaInterface = NULL;
	void	*pvCfaPrivateData = NULL;
	MRESULT  mrRet = RET_DMX_OK;

	if (NULL == pvSptHdl) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail for invalid args(pvSptHdl:0x%p)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prCfaInterface = (CfaIntf *)SplitterGetCfaInterface(pvSptHdl);

	if (NULL == prCfaInterface)
		MM_RETURN(RET_DMX_OK);

	pvCfaPrivateData = SplitterGetCfaPrivateData(pvSptHdl);

	if (NULL == pvCfaPrivateData)
		MM_RETURN(RET_DMX_OK);

	if (NULL == prCfaInterface->pfmrUninit) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in pfmrUninit == NULL, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		MM_RETURN(RET_DMX_NO_IMPLEMENT);
	}

	mrRet = prCfaInterface->pfmrUninit(pvSptHdl, pvCfaPrivateData);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in Cfa->pfmrUninit, pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	SplitterSetCfaPrivateData(pvSptHdl, NULL);

	MM_RETURN(RET_DMX_OK);
}


/*/////////////////////////////////////////////////////////////////////////////*/
/* SptCfaSetInquirer*/
/* Set cfa Inquire Type*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SptCfaSetInquirer(void *pvSptHdl, u32 u4InquirerTypes)
{
	CfaIntf *prCfaInterface = (CfaIntf *)SplitterGetCfaInterface(pvSptHdl);
	void	*pvCfaPrivateData = SplitterGetCfaPrivateData(pvSptHdl);
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prCfaInterface)
		MM_RETURN(RET_DMX_NO_CFA_INTERFACE);

	if (NULL == pvCfaPrivateData)
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);

	if (NULL == prCfaInterface->pfmrSetInqTypes)
		MM_RETURN(RET_DMX_NO_IMPLEMENT);

	mrRet = prCfaInterface->pfmrSetInqTypes(pvSptHdl, u4InquirerTypes, pvCfaPrivateData);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in Cfa->pfmrSetInqTypes, mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet, pvSptHdl);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SptCfaConfigure*/
/* Configure Cfa*/
/* @Param pvCfaParameter [in] Cfa Configuration Info, e.g. CfaAviConfigInfo*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SptCfaConfigure(void *pvSptHdl, const void *pvCfaParameter, bool fgIsUserMem)
{
	CfaIntf *prCfaInterface   = (CfaIntf *)SplitterGetCfaInterface(pvSptHdl);
	void	*pvCfaPrivateData = SplitterGetCfaPrivateData(pvSptHdl);
	MRESULT  mrRet = RET_DMX_OK;

	if (NULL == prCfaInterface)
		MM_RETURN(RET_DMX_NO_CFA_INTERFACE);

	if (NULL == pvCfaPrivateData)
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);

	if (NULL == prCfaInterface->pfmrConfigure)
		MM_RETURN(RET_DMX_NO_IMPLEMENT);

	mrRet = prCfaInterface->pfmrConfigure(pvSptHdl, (void *)pvCfaParameter, pvCfaPrivateData, fgIsUserMem);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in Cfa->pfmrConfigure, mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet, pvSptHdl);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SptSetCfaRange*/
/* Set Cfa Parsing Range*/
/* @Param pvCfaRange [in] Cfa Range Info, e.g. CfaAviRange*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SptCfaSetRange(void *pvSptHdl, const void *pvCfaRange, bool fgIsUserMem)
{
	CfaIntf *prCfaInterface = (CfaIntf *)SplitterGetCfaInterface(pvSptHdl);
	void	*pvCfaPrivateData = SplitterGetCfaPrivateData(pvSptHdl);
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prCfaInterface)
		MM_RETURN(RET_DMX_NO_CFA_INTERFACE);

	if (NULL == pvCfaPrivateData)
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);

	if (NULL == prCfaInterface->pfmrSetRange)
		MM_RETURN(RET_DMX_NO_IMPLEMENT);

	mrRet = prCfaInterface->pfmrSetRange(pvSptHdl, (void *)pvCfaRange, pvCfaPrivateData, fgIsUserMem);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in Cfa->pfmrSetRange, mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet, pvSptHdl);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SptCfaGetPosition*/
/* Set Cfa Current Parsing fileOffset/Position*/
/* @Param pvCfaPosition [Out] Cfa Current Parsing fileOffset/Position*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SptCfaGetPosition(void *pvSptHdl, const void *pvCfaPosition)
{
	CfaIntf *prCfaInterface = (CfaIntf *)SplitterGetCfaInterface(pvSptHdl);
	void	*pvCfaPrivateData = SplitterGetCfaPrivateData(pvSptHdl);
	MRESULT  mrRet = RET_DMX_OK;

	if (NULL == prCfaInterface)
		MM_RETURN(RET_DMX_NO_CFA_INTERFACE);

	if (NULL == pvCfaPrivateData)
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);

	if (NULL == prCfaInterface->pfmrGetCurPos)
		MM_RETURN(RET_DMX_NO_IMPLEMENT);

	mrRet = prCfaInterface->pfmrGetCurPos(pvSptHdl, (void *)pvCfaPosition, pvCfaPrivateData);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in Cfa->pfmrGetCurPos, mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet, pvSptHdl);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SptCfaGetGeneral*/
/* Get Cfa some information, according to inquire type - u4FID*/
/* @Param u4CfaFID [IN] Cfa inquire type*/
/* @Param pvCfaParameter [Out] Information*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SptCfaGetGeneral(void *pvSptHdl, u32 u4CfaFID, const void *pvCfaParameter,
			 u32 u4CfaParameterSize)
{
	CfaIntf *prCfaInterface = (CfaIntf *)SplitterGetCfaInterface(pvSptHdl);
	void	*pvCfaPrivateData = SplitterGetCfaPrivateData(pvSptHdl);
	MRESULT  mrRet = RET_DMX_OK;

	if (NULL == prCfaInterface)
		MM_RETURN(RET_DMX_NO_CFA_INTERFACE);

	if (NULL == pvCfaPrivateData)
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);

	if (NULL == prCfaInterface->pfmrGetGeneral)
		MM_RETURN(RET_DMX_NO_IMPLEMENT);

	mrRet = prCfaInterface->pfmrGetGeneral(pvSptHdl, u4CfaFID, pvCfaPrivateData,
		(void *)pvCfaParameter, u4CfaParameterSize);

	if (DMX_FAILED(mrRet)) {
		if (!MM_IS_STATE_ERROR(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in Cfa->pfmrGetGeneral,mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet, pvSptHdl);
		}

		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SptCfaSetGeneral*/
/* Set Cfa some information, according to inquire type - u4FID*/
/* @Param u4CfaFID [IN] Cfa inquire type*/
/* @Param pvCfaParameter [IN] Information*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SptCfaSetGeneral(void *pvSptHdl, u32 u4CfaFID, const void *pvCfaParameter,
			 u32 u4CfaParameterSize)
{
	CfaIntf *prCfaInterface = (CfaIntf *)SplitterGetCfaInterface(pvSptHdl);
	void	*pvCfaPrivateData = SplitterGetCfaPrivateData(pvSptHdl);
	MRESULT  mrRet = RET_DMX_OK;

	if (NULL == prCfaInterface)
		MM_RETURN(RET_DMX_NO_CFA_INTERFACE);

	if (NULL == pvCfaPrivateData)
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);

	if (NULL == prCfaInterface->pfmrSetGeneral)
		MM_RETURN(RET_DMX_NO_IMPLEMENT);

	mrRet = prCfaInterface->pfmrSetGeneral(pvSptHdl, u4CfaFID, pvCfaPrivateData,
					       (void *)pvCfaParameter, u4CfaParameterSize);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in Cfa->pfmrSetGeneral, mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet, pvSptHdl);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SptCfaEnableStream*/
/* Enable/Disable Cfa Parsing/Transferring designated type stream*/
/* @Param u4CFAStmType [IN] Stream Type, e.g. CFA_STRM_V0, CFA_STRM_V1*/
/* @Param fgEnable [IN] Enable/Disable*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SptCfaEnableStream(void *pvSptHdl, u32 u4CFAStmType, bool fgEnable)
{
	CfaIntf *prCfaInterface = (CfaIntf *)SplitterGetCfaInterface(pvSptHdl);
	void	*pvCfaPrivateData = SplitterGetCfaPrivateData(pvSptHdl);
	MRESULT  mrRet = RET_DMX_OK;

	if (NULL == prCfaInterface)
		MM_RETURN(RET_DMX_NO_CFA_INTERFACE);

	if (NULL == pvCfaPrivateData)
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);

	if (NULL == prCfaInterface->pfmrEnableStrm)
		MM_RETURN(RET_DMX_NO_IMPLEMENT);

	mrRet = prCfaInterface->pfmrEnableStrm(pvSptHdl, u4CFAStmType,
					       fgEnable ? CFA_STREAM_ON : CFA_STREAM_OFF, pvCfaPrivateData);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in Cfa->pfmrEnableStrm, mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet, pvSptHdl);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SptCfaSetTxDone*/
/* Inform Cfa Tx/DMA Done*/
/* @Param u8TotalTxLen [IN] Last Tx Length*/
/* @Param fgRsp [IN] whether has done Resplitter, if do, cfa should do sync pbbuf first,
		because resplitter may do rebuffer*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SptCfaSetTxDone(void *pvSptHdl, u64 u8TotalTxLen, bool fgRsp)
{
	CfaIntf *prCfaInterface = (CfaIntf *)SplitterGetCfaInterface(pvSptHdl);
	void	*pvCfaPrivateData = SplitterGetCfaPrivateData(pvSptHdl);
	MRESULT  mrRet = RET_DMX_OK;

	if (NULL == prCfaInterface)
		MM_RETURN(RET_DMX_NO_CFA_INTERFACE);

	if (NULL == pvCfaPrivateData)
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);

	if (NULL == prCfaInterface->pfmrTxDone)
		MM_RETURN(RET_DMX_NO_IMPLEMENT);

	if (SplitterRspIsRsping(pvSptHdl))
		DMX_ASSERT(FALSE);

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = pvSptHdl;
		DmxDumpFlow(DMX_OPER_SW_PTX_DONE, &rOperInfo);
	}

	mrRet = prCfaInterface->pfmrTxDone(pvSptHdl, u8TotalTxLen, pvCfaPrivateData, fgRsp);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in Cfa->pfmrTxDone, mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet, pvSptHdl);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SptCfaSetTurnOn*/
/* Inform Cfa Turn On*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SptCfaSetTurnOn(void *pvSptHdl)
{
	CfaIntf *prCfaInterface = (CfaIntf *)SplitterGetCfaInterface(pvSptHdl);
	void	*pvCfaPrivateData = SplitterGetCfaPrivateData(pvSptHdl);
	MRESULT  mrRet = RET_DMX_OK;

	if (NULL == prCfaInterface)
		MM_RETURN(RET_DMX_NO_CFA_INTERFACE);

	if (NULL == pvCfaPrivateData)
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);

	if (NULL == prCfaInterface->pfmrTurnOn)
		MM_RETURN(RET_DMX_NO_IMPLEMENT);

	mrRet = prCfaInterface->pfmrTurnOn(pvSptHdl, pvCfaPrivateData);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d, fail in Cfa->pfmrTurnOn, mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet, pvSptHdl);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SptCfaSetJumpInfo*/
/* Inform Cfa to Set Jump Info*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SptCfaSetJumpInfo(void *pvSptHdl, const void *pvJumpInfo)
{
	CfaIntf *prCfaInterface   = (CfaIntf *)SplitterGetCfaInterface(pvSptHdl);
	void	*pvCfaPrivateData = SplitterGetCfaPrivateData(pvSptHdl);
	MRESULT  mrRet = RET_DMX_OK;

	if (NULL == pvJumpInfo) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d failed for invalid arg\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (NULL == prCfaInterface)
		MM_RETURN(RET_DMX_NO_CFA_INTERFACE);

	if (NULL == pvCfaPrivateData)
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);

	if (NULL == prCfaInterface->pfmrSetJumpInfo)
		MM_RETURN(RET_DMX_NO_IMPLEMENT);

	mrRet = prCfaInterface->pfmrSetJumpInfo(pvSptHdl, (void *)pvJumpInfo, pvCfaPrivateData);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in Cfa->pfmrSetJumpInfo, mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet, pvSptHdl);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SptCfaGetGeneral*/
/* Get Cfa some information, according to inquire type - u4FID*/
/* @Param u4CfaFID [IN] Cfa inquire type*/
/* @Param pvCfaParameter [Out] Information*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SptCfaGetParam(void *pvSptHdl, u32 u4ParamID, const void *pvCfaParameter,
		       u32 u4CfaParameterSize)
{
	CfaIntf *prCfaInterface   = (CfaIntf *)SplitterGetCfaInterface(pvSptHdl);
	void	*pvCfaPrivateData = SplitterGetCfaPrivateData(pvSptHdl);
	MRESULT  mrRet = RET_DMX_OK;

	if (NULL == prCfaInterface)
		MM_RETURN(RET_DMX_NO_CFA_INTERFACE);

	if (NULL == pvCfaPrivateData)
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);

	if (NULL == prCfaInterface->pfmrGetParam)
		MM_RETURN(RET_DMX_NO_IMPLEMENT);

	mrRet = prCfaInterface->pfmrGetParam(pvSptHdl, u4ParamID, pvCfaPrivateData,
					     (void *)pvCfaParameter, u4CfaParameterSize);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in Cfa->pfmrGetParam, mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet, pvSptHdl);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SptCfaSetAUInfo*/
/* Information Cfa to Set AU Information, eg. frametype, pts and so on*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SptCfaSetAUInfo(void *pvSptHdl, const void *pvAUInf, const void *pvAUExtInf)
{
	CfaIntf *prCfaInterface   = (CfaIntf *)SplitterGetCfaInterface(pvSptHdl);
	void	*pvCfaPrivateData = SplitterGetCfaPrivateData(pvSptHdl);
	MRESULT  mrRet = RET_DMX_OK;

	if (NULL == pvAUInf) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d failed for invalid arg\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (NULL == prCfaInterface)
		MM_RETURN(RET_DMX_NO_CFA_INTERFACE);

	if (NULL == pvCfaPrivateData)
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);

	if (NULL == prCfaInterface->pfmrFillAUInfo)
		MM_RETURN(RET_DMX_NO_IMPLEMENT);

	mrRet = prCfaInterface->pfmrFillAUInfo(pvSptHdl, (void *)pvAUInf,
		(void *)pvAUExtInf, pvCfaPrivateData);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in Cfa->pfmrFillAUInfo, mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet, pvSptHdl);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SptCfaSetStmInfo*/
/* Set Stream Information, i.e. Cfa parse stream ID*/
/* @param u4StreamToSet [IN] StreamType -- CFA_STRM_V, CFA_STRM_A, CFA_STRM_SP*/
/* @param u4StmUID [IN] Stream UID*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SptCfaSetStmInfo(void *pvSptHdl, u32 u4StreamToSet, u32 u4StmUID)
{
	CfaIntf *prCfaInterface   = (CfaIntf *)SplitterGetCfaInterface(pvSptHdl);
	void	*pvCfaPrivateData = SplitterGetCfaPrivateData(pvSptHdl);
	MRESULT  mrRet = RET_DMX_OK;

	if (NULL == prCfaInterface)
		MM_RETURN(RET_DMX_NO_CFA_INTERFACE);

	if (NULL == pvCfaPrivateData)
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);

	if (NULL == prCfaInterface->pfmrSetStrmInf)
		MM_RETURN(RET_DMX_NO_IMPLEMENT);

	mrRet = prCfaInterface->pfmrSetStrmInf(pvSptHdl, u4StreamToSet, u4StmUID,
					       pvCfaPrivateData);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in Cfa->pfmrSetStrmInf, mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet, pvSptHdl);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SptCfaSetStmInfo*/
/* Set Stream Information, i.e. Cfa parse stream ID*/
/* @param u4StreamToSet [IN] StreamType -- CFA_STRM_V, CFA_STRM_A, CFA_STRM_SP*/
/* @param u4StmUID [IN] Stream UID*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SptCfaProcCliCmd(void *pvSptHdl,
			 E_DMX_CFA_CLI_TYPE_T eCliType,   /*/< [IN] Cfa Cli Command*/
			 u32 arg1,
			 u32 arg2,
			 u32 arg3,
			 const char *szParam)
{
	CfaIntf *prCfaInterface   = (CfaIntf *)SplitterGetCfaInterface(pvSptHdl);
	void	*pvCfaPrivateData = SplitterGetCfaPrivateData(pvSptHdl);
	MRESULT  mrRet = RET_DMX_OK;

	if (NULL == prCfaInterface)
		MM_RETURN(RET_DMX_NO_CFA_INTERFACE);

	if (NULL == pvCfaPrivateData)
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);

	if (NULL == prCfaInterface->pfmrProcCliCmd)
		MM_RETURN(RET_DMX_NO_IMPLEMENT);

	mrRet = prCfaInterface->pfmrProcCliCmd(pvSptHdl, eCliType, arg1, arg2, arg3,
		szParam, pvCfaPrivateData);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in Cfa->pfmrProcCliCmd, mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet, pvSptHdl);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

#endif /* #ifdef FUNC_SPT_TO_CFA*/

#ifdef FUNC_CFA_TO_SPT

/*/////////////////////////////////////////////////////////////////////////////*/
/* Spt4CfaPbb2SyncBuf*/
/* Sync Pbbuf*/
/* @Param u8FileOfst [IN] Sync FileOfst*/
/* @Param u8Len [IN] Sync Len*/
/* @Param pucBuf [OUT] After Splitter done Pbbuf Sync,	Set the address of the data  in Pb buffer*/
/*									 corresponding to the sync fileofstto pucBuf*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT Spt4CfaPbb2SyncBuf(void *pvSptHdl, u64 u8FileOfst, u64 u8Len, u8 *pucBuf)
{
	DMAInfo rDma;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvSptHdl) ||
	    (0 == u8Len))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	mm_memset(&rDma, 0, sizeof(DMAInfo));

	rDma.u8FromFileOfst = u8FileOfst;
	rDma.u8TxLen		= u8Len;
	rDma.fgSync			= TRUE;
	rDma.pucToAddress	= pucBuf;
	rDma.pu4AvailSize	= NULL;

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = pvSptHdl;
		rOperInfo.unFlow.rSync.u8FileOfst = u8FileOfst;
		rOperInfo.unFlow.rSync.u8Len = u8Len;
		DmxDumpFlow(DMX_OPER_SYNC, &rOperInfo);
	}

#if DMX_PFM_TEST
	DmxPfmSyncPbbufStart(pvSptHdl);
#endif /* DMX_PFM_TEST*/

	mrRet = SplitterPbb2Buf(pvSptHdl, &rDma);

	if (DMX_FAILED(mrRet)) {
		SplitterSetEOSForError(pvSptHdl, mrRet);

		if (MM_IS_STATE_ERROR(mrRet)) {
			MM_RETURN(RET_DMX_OK);
		} else {
#ifdef __linux__
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterPbb2Buf(u8Len: %lld, u8FileOfst:")
				TEXT(" %lld), mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u8Len, u8FileOfst, mrRet, pvSptHdl);
#else
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterPbb2Buf(u8Len: %I64d, u8FileOfst:")
				TEXT(" %I64d), mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u8Len, u8FileOfst, mrRet, pvSptHdl);
#endif /* #ifdef __linux__*/
		}
	}

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* Spt4CfaPbb2SyncBufEx*/
/* Sync Pbbuf*/
/* @Param u8FileOfst [IN] Sync FileOfst*/
/* @Param u8Len [IN] Sync Len*/
/* @Param pucBuf [OUT] After Splitter done Pbbuf Sync,	Set the address of the data  in Pb buffer*/
/*									 corresponding to the sync fileofstto pucBuf*/
/* @Param pu4AvailSize [OUT] Current Avaliable Data Size in Pbbuf after Sync*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT Spt4CfaPbb2SyncBufEx(void *pvSptHdl, u64 u8FileOfst, u64 u8Len,
			     u8 *pucBuf, u32 *pu4AvailSize)
{
	DMAInfo rDma;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvSptHdl) ||
	    (0 == u8Len))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	mm_memset(&rDma, 0, sizeof(DMAInfo));

	rDma.u8FromFileOfst = u8FileOfst;
	rDma.u8TxLen		= u8Len;
	rDma.fgSync			= TRUE;
	rDma.pucToAddress	= pucBuf;
	rDma.pu4AvailSize	= pu4AvailSize;

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = pvSptHdl;
		rOperInfo.unFlow.rSync.u8FileOfst = u8FileOfst;
		rOperInfo.unFlow.rSync.u8Len = u8Len;
		DmxDumpFlow(DMX_OPER_SYNCEX, &rOperInfo);
	}

#if DMX_PFM_TEST
	DmxPfmSyncPbbufStart(pvSptHdl);
#endif /* DMX_PFM_TEST*/

	mrRet = SplitterPbb2Buf(pvSptHdl, &rDma);

	if (DMX_FAILED(mrRet)) {
		SplitterSetEOSForError(pvSptHdl, mrRet);

		if (MM_IS_STATE_ERROR(mrRet)) {
			MM_RETURN(RET_DMX_OK);
		} else {
#ifdef __linux__
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterPbb2Buf(u8Len: %lld, u8FileOfst:")
				TEXT(" %lld), mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u8Len, u8FileOfst, mrRet, pvSptHdl);
#else
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterPbb2Buf(u8Len: %I64d, u8FileOfst:")
				TEXT(" %I64d), mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u8Len, u8FileOfst, mrRet, pvSptHdl);
#endif /* #ifdef __linux__*/
		}
	}

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* Spt4CfaBuf2VFifo*/
/* Tx Designate Memory data into Video FIFO, but don't compose one AU*/
/* @Param pucSrc [IN] TX Src Memory VSA*/
/* @Param u8Position [IN] Corresponding  File Ofst*/
/* @Param eTxMode [IN] To Tx Data's Picture Type*/
/* @Param eVidType [IN] Video Codec*/
/* @Param u8Len [IN] Tx Len*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT Spt4CfaBuf2VFifo(void *pvSptHdl, u8 *pucSrc, u64 u8Position,
			 CfaApiPicTxMode eTxMode, CfaApiVidType eVidType, u64 u8Len)
{
	DMX_SPT_DMA2FIFO_INFO_T rInf;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvSptHdl) ||
	    (0 == u8Len) ||
	    (NULL == pucSrc))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	mm_memset(&rInf, 0, sizeof(DMX_SPT_DMA2FIFO_INFO_T));

	rInf.u8FromFileOfst = u8Position;
	rInf.pvFromAddress	= pucSrc;
	rInf.u8TxLen		= u8Len;
	rInf.u4TxStreamType = SPT_DATA_V;
	rInf.pvToAddress	= NULL;
	rInf.u4TxVideoCodec = eVidType;
	rInf.u4TxPictureMode = Spt4CfaGetPitureType(eTxMode);
	rInf.u8PtsSa		= INVALID_TIMESTAMP;
	rInf.u8PtsEa		= INVALID_TIMESTAMP;

	if (g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_VID])
		DmxDumpVDmaInfo(0, pucSrc, u8Len);

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = pvSptHdl;
		rOperInfo.unFlow.rDma.u4StmType = SPT_DATA_V;
		rOperInfo.unFlow.rDma.u8FileOfst = 0;
		rOperInfo.unFlow.rDma.pvBuf = pucSrc;
		rOperInfo.unFlow.rDma.u8Len = u8Len;
		DmxDumpFlow(DMX_OPER_SW_START_PTX, &rOperInfo);
	}

#if DMX_PFM_TEST
	DmxPfmStmSWDmaStart(pvSptHdl, SPT_DATA_V);
#endif /* DMX_PFM_TEST*/

	mrRet = SplitterBuf2Fifo(pvSptHdl, &rInf);

	if (DMX_FAILED(mrRet)) {
		SplitterSetEOSForError(pvSptHdl, mrRet);

		if (!MM_IS_STATE_ERROR(mrRet)) {
#ifdef __linux__
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterBuf2Fifo(u8Len: %lld, PictureMode:")
				TEXT(" 0x%x, VidType: %d), mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u8Len, rInf.u4TxPictureMode, eVidType, mrRet, pvSptHdl);
#else
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterBuf2Fifo(u8Len: %I64d,PictureMode: ")
				TEXT(" 0x%x, VidType: %d),mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u8Len, rInf.u4TxPictureMode, eVidType, mrRet, pvSptHdl);
#endif /* #ifdef __linux__*/
		} else
			MM_RETURN(RET_DMX_OK);
	}

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* Spt4CfaBuf2VFifoAUCtrl*/
/* Tx Designate Memory data into Video FIFO, and compose one AU*/
/* @Param pucSrc [IN] TX Src Memory VSA*/
/* @Param prVidInf [IN] Cfa Tx Video Info, including FileOfst, TxLen, VideoType, StreamUID, whether is dummy AU*/
/*								   whether it is the AU start, compose AU length*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT Spt4CfaBuf2VFifoAUCtrl(void *pvSptHdl, u8 *pucSrc,
			       CFA_VIDEO_INFO_T  *prVidInf)
{
	DMX_SPT_DMA2FIFO_INFO_T rInf;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvSptHdl) ||
	    (NULL == prVidInf) ||
	    (NULL == pucSrc))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	mm_memset(&rInf, 0, sizeof(DMX_SPT_DMA2FIFO_INFO_T));

	rInf.u8FromFileOfst = prVidInf->u8FileOfst;
	rInf.pvFromAddress	= pucSrc;
	rInf.u8TxLen		= prVidInf->u8Len;
	rInf.u4TxStreamType = SPT_DATA_V;
	rInf.pvToAddress	= NULL;
	rInf.u4TxVideoCodec = prVidInf->eVidType;
	rInf.u4TxPictureMode = Spt4CfaGetPitureType(prVidInf->eTxMode);
	rInf.u8PtsSa		= INVALID_TIMESTAMP;
	rInf.u8PtsEa		= INVALID_TIMESTAMP;

	rInf.u4TxUID		= prVidInf->u4PrsStrmId;
	rInf.fgDummyUnit	= prVidInf->fgDummyAU;
	rInf.fgCreateAU		= prVidInf->fgUnitStart;
	rInf.u8TotalAULen	= prVidInf->u8TotalAULen;

	if ((CFA_VID_VC1 == (prVidInf->eVidType)) &&
	    (prVidInf->fgQueryWVC1Mode))
		rInf.rExInf.rWVC1.fgQueryWVC1Mode = prVidInf->fgQueryWVC1Mode;

	if (g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_VID])
		DmxDumpVDmaInfo(0, pucSrc, prVidInf->u8Len);

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = pvSptHdl;
		rOperInfo.unFlow.rDma.u4StmType = SPT_DATA_V;
		rOperInfo.unFlow.rDma.u8FileOfst = 0;
		rOperInfo.unFlow.rDma.pvBuf = pucSrc;
		rOperInfo.unFlow.rDma.u8Len = prVidInf->u8Len;
		DmxDumpFlow(DMX_OPER_SW_START_PTX, &rOperInfo);
	}

#if DMX_PFM_TEST
	DmxPfmStmSWDmaStart(pvSptHdl, SPT_DATA_V);
#endif /* DMX_PFM_TEST*/

	mrRet = SplitterBuf2FifoAUCtrl(pvSptHdl, &rInf);

	if (DMX_FAILED(mrRet)) {
		SplitterSetEOSForError(pvSptHdl, mrRet);

		if (!MM_IS_STATE_ERROR(mrRet)) {
#ifdef __linux__
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterBuf2FifoAUCtrl(u8Len: %lld,")
				TEXT("Offset: %lld),mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prVidInf->u8Len, prVidInf->u8FileOfst, mrRet, pvSptHdl);
#else
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterBuf2FifoAUCtrl(u8Len: %I64d,")
				TEXT(" Offset: %I64d),mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prVidInf->u8Len, prVidInf->u8FileOfst, mrRet, pvSptHdl);
#endif /* #ifdef __linux__*/
		} else
			MM_RETURN(RET_DMX_OK);
	}

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* Spt4CfaBuf2AFifo*/
/* Tx Designate Memory data into Audio FIFO, but don't compose one AU*/
/* @Param pucSrc [IN] TX Src Memory VSA*/
/* @Param u8Position [IN] Corresponding  File Ofst*/
/* @Param u8Len [IN] Tx Len*/
/* @Param u8TxUID [IN] TX Stream's UID*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT Spt4CfaBuf2AFifo(void *pvSptHdl, u8 *pucSrc,
			 u64 u8Len, u32 u4TxUID, CfaApiAudType eAudType)
{
	DMX_SPT_DMA2FIFO_INFO_T rInf;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvSptHdl) ||
	    (0 == u8Len) ||
	    (NULL == pucSrc))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	DMXLOG_DEBUG(TEXT("[SPT] %s -- enter, pvSptHdl: 0x%p\r\n"),
		    DMX_FUNC_NAME, pvSptHdl);

	mm_memset(&rInf, 0, sizeof(DMX_SPT_DMA2FIFO_INFO_T));

	rInf.u8FromFileOfst = 0;
	rInf.pvFromAddress	= pucSrc;
	rInf.u8TxLen		= u8Len;
	rInf.u4TxUID		= u4TxUID;
	rInf.u4TxStreamType = SPT_DATA_A;
	rInf.pvToAddress	= NULL;
	rInf.u4TxAudioCodec = Spt4CfaGetAudioCodec(eAudType);
	rInf.u4TxVideoCodec = VC_UNKNOW;
	rInf.u4TxPictureMode = 0;
	rInf.u8PtsSa		= INVALID_TIMESTAMP;
	rInf.u8PtsEa		= INVALID_TIMESTAMP;


	if (g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_AUD])
		DmxDumpADmaInfo(pvSptHdl, 0, pucSrc, u8Len, rInf.u4TxUID);

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = pvSptHdl;
		rOperInfo.unFlow.rDma.u4StmType = SPT_DATA_A;
		rOperInfo.unFlow.rDma.u8FileOfst = 0;
		rOperInfo.unFlow.rDma.pvBuf = pucSrc;
		rOperInfo.unFlow.rDma.u8Len = u8Len;
		DmxDumpFlow(DMX_OPER_SW_START_PTX, &rOperInfo);
	}

#if DMX_PFM_TEST
	DmxPfmStmSWDmaStart(pvSptHdl, SPT_DATA_A);
#endif /* DMX_PFM_TEST*/

	mrRet = SplitterBuf2Fifo(pvSptHdl, &rInf);

	if (DMX_FAILED(mrRet)) {
		SplitterSetEOSForError(pvSptHdl, mrRet);

		if (!MM_IS_STATE_ERROR(mrRet)) {
#ifdef __linux__
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterBuf2Fifo(u8Len: %lld, u4TxUID:")
				TEXT(" 0x%x),mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u8Len, u4TxUID, mrRet, pvSptHdl);
#else
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterBuf2Fifo(u8Len: %I64d, u4TxUID:")
				TEXT(" 0x%x), mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u8Len, u4TxUID, mrRet, pvSptHdl);
#endif /* #ifdef __linux__*/
		} else
			MM_RETURN(RET_DMX_OK);
	}

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* Spt4CfaBuf2AFifoAUCtrl*/
/* Tx Designate Memory data into Audio FIFO, and compose one AU*/
/* @Param pucSrc [IN] TX Src Memory VSA*/
/* @Param prAudInf [IN] Cfa Tx Audio Info, including FileOfst, TxLen, Pts, StreamUID, whether is dummy AU*/
/*									Audio Code, whether it is the AU End, AU start*/
/* @Param u8TotalAULen [IN] Total AU Len*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT Spt4CfaBuf2AFifoAUCtrl(void *pvSptHdl, u8 *pucSrc,
			       CFA_AUDIO_INFO_T *prAudInf,
			       u64 u8TotalAULen)
{
	DMX_SPT_DMA2FIFO_INFO_T rInf;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvSptHdl) ||
	    (NULL == prAudInf) ||
	    (NULL == pucSrc))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	DMXLOG_DEBUG(TEXT("[SPT] %s -- enter, pvSptHdl: 0x%p\r\n"),
		DMX_FUNC_NAME, pvSptHdl);

	mm_memset(&rInf, 0, sizeof(DMX_SPT_DMA2FIFO_INFO_T));

	rInf.u8FromFileOfst = prAudInf->u8FileOfst;
	rInf.pvFromAddress	= pucSrc;
	rInf.u8TxLen		= prAudInf->u8Len;
	rInf.u4TxStreamType = SPT_DATA_A;
	rInf.pvToAddress	= NULL;
	rInf.u4TxVideoCodec = VC_UNKNOW;
	rInf.u4TxPictureMode = 0;
	rInf.u8PtsSa		= prAudInf->u8Pts;
	rInf.u8PtsEa		= (u64)INVALID_TIMESTAMP;

	rInf.u4TxUID		= prAudInf->u4PrsStrmId;
	rInf.fgDummyUnit	= prAudInf->fgDummyAU;

	rInf.u4TxAudioCodec = Spt4CfaGetAudioCodec(prAudInf->eAudType);
	rInf.fgAUByEnd		= prAudInf->fgAUCompleteByEnd;
	rInf.fgUnitEnd		= prAudInf->fgUnitEnd;
	rInf.fgCreateAU		= prAudInf->fgUnitStart;
	rInf.u8TotalAULen	= u8TotalAULen;

	if (g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_AUD])
		DmxDumpADmaInfo(pvSptHdl, 0, pucSrc, prAudInf->u8Len, rInf.u4TxUID);

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = pvSptHdl;
		rOperInfo.unFlow.rDma.u4StmType = SPT_DATA_A;
		rOperInfo.unFlow.rDma.u8FileOfst = 0;
		rOperInfo.unFlow.rDma.pvBuf = pucSrc;
		rOperInfo.unFlow.rDma.u8Len = prAudInf->u8Len;
		DmxDumpFlow(DMX_OPER_SW_START_PTX, &rOperInfo);
	}

#if DMX_PFM_TEST
	DmxPfmStmSWDmaStart(pvSptHdl, SPT_DATA_A);
#endif /* DMX_PFM_TEST*/

	mrRet = SplitterBuf2FifoAUCtrl(pvSptHdl, &rInf);

	if (DMX_FAILED(mrRet)) {
		SplitterSetEOSForError(pvSptHdl, mrRet);

		if (!MM_IS_STATE_ERROR(mrRet)) {
#ifdef __linux__
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterBuf2FifoAUCtrl(u8Len: %lld,")
				TEXT(" Offset: %lld),mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prAudInf->u8Len, prAudInf->u8FileOfst, mrRet, pvSptHdl);
#else
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterBuf2FifoAUCtrl(u8Len: %I64d,")
				TEXT(" Offset: %I64d),mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prAudInf->u8Len, prAudInf->u8FileOfst, mrRet, pvSptHdl);
#endif /* #ifdef __linux__*/
		} else
			MM_RETURN(RET_DMX_OK);
	}

	MM_RETURN(mrRet);
}

MRESULT Spt4CfaBuf2SectionFifo(void *pvSptHdl, u8 *pucSrc, u64 u8Position,
			       u64 u8Len, u32 u4TxUID)
{
	DMX_SPT_DMA2FIFO_INFO_T rInf;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvSptHdl) ||
	    (0 == u8Len) ||
	    (NULL == pucSrc))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	mm_memset(&rInf, 0, sizeof(DMX_SPT_DMA2FIFO_INFO_T));

	rInf.u8FromFileOfst = u8Position;
	rInf.pvFromAddress	= pucSrc;
	rInf.u8TxLen		= u8Len;
	rInf.u4TxUID		= u4TxUID;
	rInf.u4TxStreamType = SPT_DATA_SECTION;
	rInf.pvToAddress	= NULL;
	rInf.u4TxVideoCodec = VC_UNKNOW;
	rInf.u4TxPictureMode = 0;
	rInf.u8PtsSa		= INVALID_TIMESTAMP;
	rInf.u8PtsEa		= INVALID_TIMESTAMP;

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = pvSptHdl;
		rOperInfo.unFlow.rDma.u4StmType = SPT_DATA_SECTION;
		rOperInfo.unFlow.rDma.u8FileOfst = 0;
		rOperInfo.unFlow.rDma.pvBuf = pucSrc;
		rOperInfo.unFlow.rDma.u8Len = u8Len;
		DmxDumpFlow(DMX_OPER_SW_START_PTX, &rOperInfo);
	}

	mrRet = SplitterBuf2Fifo(pvSptHdl, &rInf);

	if (DMX_FAILED(mrRet)) {
		SplitterSetEOSForError(pvSptHdl, mrRet);

		if (!MM_IS_STATE_ERROR(mrRet)) {
#ifdef __linux__
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterBuf2Fifo(u8Len: %lld, u4TxUID:")
				TEXT(" 0x%x),mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u8Len, u4TxUID, mrRet, pvSptHdl);
#else
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterBuf2Fifo(u8Len: %I64d,")
				TEXT(" u4TxUID: 0x%x), mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u8Len, u4TxUID, mrRet, pvSptHdl);
#endif /* #ifdef __linux__*/
		} else
			MM_RETURN(RET_DMX_OK);
	}

	MM_RETURN(mrRet);
}

MRESULT Spt4CfaBuf2SectionFifoAUCtrl(
	void			*pvSptHdl,
	u8			   *pucSrc,
	CFA_SECTION_INFO_T *prSectionInf,
	u64			   u8TotalAULen)
{
	DMX_SPT_DMA2FIFO_INFO_T rInf;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvSptHdl) ||
	    (NULL == pucSrc) ||
	    (NULL == prSectionInf))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	mm_memset(&rInf, 0, sizeof(DMX_SPT_DMA2FIFO_INFO_T));

	rInf.u8FromFileOfst = prSectionInf->u8FileOfst;
	rInf.pvFromAddress	= pucSrc;
	rInf.u8TxLen		= prSectionInf->u8Len;
	rInf.u4TxStreamType = SPT_DATA_SECTION;
	rInf.pvToAddress	= NULL;
	rInf.u4TxVideoCodec = VC_UNKNOW;
	rInf.u4TxPictureMode = 0;
	rInf.u4TxUID		= prSectionInf->u4PrsStrmId;
	rInf.fgDummyUnit	= prSectionInf->fgDummyAU;

	rInf.fgAUByEnd		= prSectionInf->fgAUCompleteByEnd;
	rInf.fgUnitEnd		= prSectionInf->fgUnitEnd;
	rInf.fgCreateAU		= prSectionInf->fgUnitStart;
	rInf.u8TotalAULen	= u8TotalAULen;

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = pvSptHdl;
		rOperInfo.unFlow.rDma.u4StmType = SPT_DATA_SECTION;
		rOperInfo.unFlow.rDma.u8FileOfst = 0;
		rOperInfo.unFlow.rDma.pvBuf = pucSrc;
		rOperInfo.unFlow.rDma.u8Len = rInf.u8TxLen;
		DmxDumpFlow(DMX_OPER_SW_START_PTX, &rOperInfo);
	}

	mrRet = SplitterBuf2FifoAUCtrl(pvSptHdl, &rInf);

	if (DMX_FAILED(mrRet)) {
		SplitterSetEOSForError(pvSptHdl, mrRet);

		if (!MM_IS_STATE_ERROR(mrRet)) {
#ifdef __linux__
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterBuf2FifoAUCtrl(u8Len: %lld,")
				TEXT(" u4TxUID: 0x%x),mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSectionInf->u8Len, rInf.u4TxUID, mrRet, pvSptHdl);
#else
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterBuf2FifoAUCtrl(u8Len: %I64d,")
				TEXT(" u4TxUID: 0x%x),mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSectionInf->u8Len, rInf.u4TxUID, mrRet, pvSptHdl);
#endif /* #ifdef __linux__*/
		} else
			MM_RETURN(RET_DMX_OK);
	}

	MM_RETURN(mrRet);
}

MRESULT Spt4CfaBuf2SpFifo(void *pvSptHdl, u8 *pucSrc, u64 u8Position,
			  u64 u8Len, u32 u4TxUID)
{
	DMX_SPT_DMA2FIFO_INFO_T rInf;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvSptHdl) ||
	    (0 == u8Len) ||
	    (NULL == pucSrc))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	mm_memset(&rInf, 0, sizeof(DMX_SPT_DMA2FIFO_INFO_T));

	rInf.u8FromFileOfst = u8Position;
	rInf.pvFromAddress	= pucSrc;
	rInf.u8TxLen		= u8Len;
	rInf.u4TxUID		= u4TxUID;
	rInf.u4TxStreamType = SPT_DATA_SP;
	rInf.pvToAddress	= NULL;
	rInf.u4TxVideoCodec = VC_UNKNOW;
	rInf.u4TxPictureMode = 0;
	rInf.u8PtsSa		= INVALID_TIMESTAMP;
	rInf.u8PtsEa		= INVALID_TIMESTAMP;

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = pvSptHdl;
		rOperInfo.unFlow.rDma.u4StmType = SPT_DATA_SP;
		rOperInfo.unFlow.rDma.u8FileOfst = 0;
		rOperInfo.unFlow.rDma.pvBuf = pucSrc;
		rOperInfo.unFlow.rDma.u8Len = rInf.u8TxLen;
		DmxDumpFlow(DMX_OPER_SW_START_PTX, &rOperInfo);
	}

#if DMX_PFM_TEST
	DmxPfmStmSWDmaStart(pvSptHdl, SPT_DATA_SP);
#endif /* DMX_PFM_TEST*/

	mrRet = SplitterBuf2Fifo(pvSptHdl, &rInf);

	if (DMX_FAILED(mrRet)) {
		SplitterSetEOSForError(pvSptHdl, mrRet);

		if (!MM_IS_STATE_ERROR(mrRet)) {
#ifdef __linux__
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterBuf2Fifo(u8Len: %lld, u4TxUID:")
				TEXT(" 0x%x), mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u8Len, rInf.u4TxUID, mrRet, pvSptHdl);
#else
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterBuf2Fifo(u8Len: %I64d, u4TxUID:")
				TEXT(" 0x%x),mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u8Len, rInf.u4TxUID, mrRet, pvSptHdl);
#endif /* #ifdef __linux__*/
		} else
			MM_RETURN(RET_DMX_OK);
	}

	MM_RETURN(mrRet);
}

MRESULT Spt4CfaBuf2SpFifoAUCtrl(void *pvSptHdl, u8 *pucSrc,
				CFA_SUBPIC_INFO_T *prSubPicInf,
				u64 u8TotalAULen)
{
	DMX_SPT_DMA2FIFO_INFO_T rInf;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prSubPicInf) || (NULL == pucSrc))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	mm_memset(&rInf, 0, sizeof(DMX_SPT_DMA2FIFO_INFO_T));

	rInf.u8FromFileOfst = prSubPicInf->u8FileOfst;
	rInf.pvFromAddress	= pucSrc;
	rInf.u8TxLen		= prSubPicInf->u8Len;
	rInf.u4TxStreamType = SPT_DATA_SP;
	rInf.pvToAddress	= NULL;
	rInf.u4TxVideoCodec = VC_UNKNOW;
	rInf.u4TxPictureMode = 0;
	rInf.u4TxUID		= prSubPicInf->u4PrsStrmId;
	rInf.fgDummyUnit	= prSubPicInf->fgDummyAU;

	rInf.u8PtsSa		= prSubPicInf->u8Pts;
	rInf.u8PtsEa		= prSubPicInf->u8EndPts;
	rInf.fgCreateAU	= prSubPicInf->fgUnitStart;

	rInf.u8TotalAULen	= u8TotalAULen;

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = pvSptHdl;
		rOperInfo.unFlow.rDma.u4StmType = SPT_DATA_SP;
		rOperInfo.unFlow.rDma.u8FileOfst = 0;
		rOperInfo.unFlow.rDma.pvBuf = pucSrc;
		rOperInfo.unFlow.rDma.u8Len = rInf.u8TxLen;
		DmxDumpFlow(DMX_OPER_SW_START_PTX, &rOperInfo);
	}

#if DMX_PFM_TEST
	DmxPfmStmSWDmaStart(pvSptHdl, SPT_DATA_SP);
#endif /* DMX_PFM_TEST*/

	mrRet = SplitterBuf2FifoAUCtrl(pvSptHdl, &rInf);

	if (DMX_FAILED(mrRet)) {
		SplitterSetEOSForError(pvSptHdl, mrRet);

		if (!MM_IS_STATE_ERROR(mrRet)) {
#ifdef __linux__
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterBuf2FifoAUCtrl(u8Len: %lld,")
				TEXT(" u4TxUID: 0x%x),mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSubPicInf->u8Len, rInf.u4TxUID, mrRet, pvSptHdl);
#else
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterBuf2FifoAUCtrl(u8Len:")
				TEXT(" %I64d,u4TxUID: 0x%x), mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prSubPicInf->u8Len, rInf.u4TxUID, mrRet, pvSptHdl);
#endif /* #ifdef __linux__*/
		} else
			MM_RETURN(RET_DMX_OK);
	}

	MM_RETURN(mrRet);
}

MRESULT Spt4CfaPbb2SectionFifoAUCtrl(void *pvSptHdl, CFA_SECTION_INFO_T *prSectionInf)
{
	DMX_SPT_DMA2FIFO_INFO_T rInf;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prSectionInf) || (NULL == pvSptHdl))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	mm_memset(&rInf, 0, sizeof(DMX_SPT_DMA2FIFO_INFO_T));

	rInf.u8FromFileOfst = prSectionInf->u8FileOfst;
	rInf.pvFromAddress	= NULL;
	rInf.u8TxLen		= prSectionInf->u8Len;
	rInf.u4TxStreamType = SPT_DATA_SECTION;
	rInf.pvToAddress	= NULL;

	rInf.u8PtsSa		= INVALID_TIMESTAMP;
	rInf.u8PtsEa		= INVALID_TIMESTAMP;
	rInf.u4TxUID		= prSectionInf->u4PrsStrmId;
	rInf.fgDummyUnit	= prSectionInf->fgDummyAU;
	rInf.fgCreateAU		= prSectionInf->fgUnitStart;
	rInf.fgUnitEnd		= prSectionInf->fgUnitEnd;
	rInf.fgAUByEnd		= prSectionInf->fgAUCompleteByEnd;
	rInf.u8TotalAULen	= prSectionInf->u8TotalAULen;
	rInf.u4PackCnt		= prSectionInf->u4PackCnt;

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = pvSptHdl;
		rOperInfo.unFlow.rDma.u4StmType = SPT_DATA_SP;
		rOperInfo.unFlow.rDma.u8FileOfst = rInf.u8FromFileOfst;
		rOperInfo.unFlow.rDma.pvBuf = NULL;
		rOperInfo.unFlow.rDma.u8Len = rInf.u8TxLen;
		DmxDumpFlow(DMX_OPER_SW_START_PTX, &rOperInfo);
	}

	if (prSectionInf->fgUseCmdQ) {
		rInf.fgUseCmdQ = prSectionInf->fgUseCmdQ;
		rInf.fgAUByCmdQEnd = FALSE;

		if ((prSectionInf->fgUseCmdQ) &&
		    (rInf.u2TxEntryCnt > DMX_MAX_TX_CNT_FOR_CMD_Q)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail for CmdQ Entry Cnt(%d) > MAXCNT(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, rInf.u2TxEntryCnt, DMX_MAX_TX_CNT_FOR_CMD_Q);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_OVER_LIMIT);
		}

		rInf.u2TxEntryCnt = prSectionInf->u2TxEntryCnt;
		rInf.u8RealTxLen = prSectionInf->u8RealTxLen;
		rInf.parCmdQTxEntry = prSectionInf->parCmdQTxEntry;
	}

	mrRet = SplitterPbb2Fifo(pvSptHdl, &rInf);

	if (DMX_FAILED(mrRet)) {
		SplitterSetEOSForError(pvSptHdl, mrRet);

		if (!MM_IS_STATE_ERROR(mrRet)) {
#ifdef __linux__
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterPbb2Fifo(u8Len: %lld, Offset:")
				TEXT(" %lld),mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSectionInf->u8Len,
				prSectionInf->u8FileOfst, mrRet, pvSptHdl);
#else
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterPbb2Fifo(u8Len: %I64d, Offset:")
				TEXT(" %I64d),mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSectionInf->u8Len,
				prSectionInf->u8FileOfst, mrRet, pvSptHdl);
#endif /* #ifdef __linux__*/
		} else
			MM_RETURN(RET_DMX_OK);
	}

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* Spt4CfaBuf2VFifoAUCtrl*/
/* Tx Designate Pbbuf's data into Video FIFO, and compose one AU*/
/* @Param prVidInf [IN] Cfa Tx Video Info, including FileOfst, TxLen, VideoType, StreamUID, whether is dummy AU*/
/*								   whether it is the AU start, compose AU length*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT Spt4CfaPbb2VFifoAUCtrl(void *pvSptHdl, CFA_VIDEO_INFO_T *prVidInf)
{
	DMX_SPT_DMA2FIFO_INFO_T rInf;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prVidInf) ||
	    (NULL == pvSptHdl))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	mm_memset(&rInf, 0, sizeof(DMX_SPT_DMA2FIFO_INFO_T));

	rInf.u8FromFileOfst = prVidInf->u8FileOfst;
	rInf.pvFromAddress	= NULL;
	rInf.u8TxLen		= prVidInf->u8Len;
	rInf.u4TxStreamType = SPT_DATA_V;
	rInf.pvToAddress	= NULL;
	rInf.u4TxVideoCodec = prVidInf->eVidType;

#if ENABLE_DMX_ADVANCED_VER
#else

	if (rInf.u4TxVideoCodec == CFA_VID_DIVX3) {
		if ((prVidInf->eTxMode != CFA_PTM_ONE_PIC_DX3_I) &&
		    (prVidInf->eTxMode != CFA_PTM_ONE_PIC_DX3_P) &&
		    (prVidInf->eTxMode != CFA_PTM_DUMMY)) {
			DMXLOG_ERROR(
				TEXT("[Spt] %s line %d fail for CFA_VID_DIVX3, eTxMode(0x%x) Err!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prVidInf->eTxMode);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}

	else if (rInf.u4TxVideoCodec == CFA_VID_H263_SORENSON) {
		if ((prVidInf->eTxMode != CFA_PTM_H263_SORENSON_I) &&
		    (prVidInf->eTxMode != CFA_PTM_H263_SORENSON_P) &&
		    (prVidInf->eTxMode != CFA_PTM_DUMMY)) {
			DMXLOG_ERROR(
				TEXT("[Spt] %s line %d fail for CFA_VID_H263_SORENSON, eTxMode(0x%x) Err!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prVidInf->eTxMode);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}

	else if (rInf.u4TxVideoCodec == CFA_VID_MJPEG) {
		if ((prVidInf->eTxMode != CFA_PTM_MJPEG_I) &&
		    (prVidInf->eTxMode != CFA_PTM_DUMMY)) {
			DMXLOG_ERROR(
			TEXT("[Spt] %s line %d fail for CFA_VID_MJPEG, eTxMode(0x%x) Err!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prVidInf->eTxMode);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	} else if (rInf.u4TxVideoCodec == CFA_VID_VP6) {
		if ((prVidInf->eTxMode != CFA_PTM_ONE_PIC_VP6_I) &&
		    (prVidInf->eTxMode != CFA_PTM_ONE_PIC_VP6_P) &&
		    (prVidInf->eTxMode != CFA_PTM_DUMMY)) {
			DMXLOG_ERROR(
				TEXT("[Spt] %s line %d fail for CFA_VID_VP6, eTxMode(0x%x) Err!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prVidInf->eTxMode);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	} else if (rInf.u4TxVideoCodec == CFA_VID_VP6A) {
		if ((prVidInf->eTxMode != CFA_PTM_ONE_PIC_VP6_I) &&
		    (prVidInf->eTxMode != CFA_PTM_ONE_PIC_VP6_P) &&
		    (prVidInf->eTxMode != CFA_PTM_DUMMY)) {
			DMXLOG_ERROR(
				TEXT("[Spt] %s line %d fail for CFA_VID_VP6A, eTxMode(0x%x) Err!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prVidInf->eTxMode);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	} else if (rInf.u4TxVideoCodec == CFA_VID_VP8) {
		if ((prVidInf->eTxMode != CFA_PTM_ONE_PIC_VP8_I) &&
		    (prVidInf->eTxMode != CFA_PTM_ONE_PIC_VP8_P) &&
		    (prVidInf->eTxMode != CFA_PTM_DUMMY)) {
			DMXLOG_ERROR(
				TEXT("[Spt] %s line %d fail for CFA_VID_VP8, eTxMode(0x%x) Err!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prVidInf->eTxMode);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}

#endif /* ENABLE_DMX_ADVANCED_VER*/

	rInf.u4TxPictureMode = Spt4CfaGetPitureType(prVidInf->eTxMode);

	rInf.u8PtsSa		= INVALID_TIMESTAMP;
	rInf.u8PtsEa		= INVALID_TIMESTAMP;
	rInf.u4TxUID		= prVidInf->u4PrsStrmId;
	rInf.fgDummyUnit	= prVidInf->fgDummyAU;
	rInf.fgDummyAUEnd	= prVidInf->fgDummyAUEnd;
	rInf.fgDummyCmdAU	= prVidInf->fgDummyCmdAU;
	rInf.fgCreateAU		= prVidInf->fgUnitStart;
	rInf.fgUnitEnd		= prVidInf->fgUnitEnd;
	rInf.fgAUByEnd		= prVidInf->fgAUCompleteByEnd;
	rInf.u8TotalAULen	= prVidInf->u8TotalAULen;

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo.pvSptHdl = pvSptHdl;
		rOperInfo.unFlow.rDma.u4StmType = SPT_DATA_V;
		rOperInfo.unFlow.rDma.u8FileOfst = rInf.u8FromFileOfst;
		rOperInfo.unFlow.rDma.pvBuf = NULL;
		rOperInfo.unFlow.rDma.u8Len = rInf.u8TxLen;
		DmxDumpFlow(DMX_OPER_SW_START_PTX, &rOperInfo);
	}

	if (((prVidInf->eVidType) == CFA_VID_VC1) && (prVidInf->fgQueryWVC1Mode))
		rInf.rExInf.rWVC1.fgQueryWVC1Mode = prVidInf->fgQueryWVC1Mode;

#if ENABLE_DMX_ADVANCED_VER
	rInf.fgAUByCmdQEnd = prVidInf->fgAUByCmdQEnd;
#else

	if ((prVidInf->fgUseCmdQ) &&
	    (CFA_VID_MPEG2 != prVidInf->eVidType) &&
	    (CFA_VID_H264 != prVidInf->eVidType) &&
	    (CFA_VID_H265 != prVidInf->eVidType)) {
		DMXLOG_ERROR(
			TEXT("[Spt] %s line %d fail for CMDQ unsupport video codec(0x%x)!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prVidInf->eVidType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	rInf.fgAUByCmdQEnd = FALSE;
#endif /* ENABLE_DMX_ADVANCED_VER*/

	rInf.fgUseCmdQ = prVidInf->fgUseCmdQ;
	rInf.u2TxEntryCnt = prVidInf->u2TxEntryCnt;

	if ((prVidInf->fgUseCmdQ) &&
	    (rInf.u2TxEntryCnt > DMX_MAX_TX_CNT_FOR_CMD_Q)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail for CmdQ Entry Cnt(%d) > MAXCNT(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, rInf.u2TxEntryCnt,
			DMX_MAX_TX_CNT_FOR_CMD_Q);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}

	rInf.u8RealTxLen = prVidInf->u8RealTxLen;
	rInf.parCmdQTxEntry = prVidInf->parCmdQTxEntry;

	if (prVidInf->fgUseCmdQ) {
		u16 u2CmdIdx = 0;
		u64 u8EndCmdOfst = prVidInf->u8FileOfst;

		for (u2CmdIdx = 0; u2CmdIdx < rInf.u2TxEntryCnt - 1; u2CmdIdx++) {
			u8EndCmdOfst += prVidInf->parCmdQTxEntry[u2CmdIdx].u4TxOfst;
			u8EndCmdOfst += prVidInf->parCmdQTxEntry[u2CmdIdx].u4TxLen;
#if ENABLE_DMX_ADVANCED_VER
			{
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_INSTBS,
					TEXT("[SPT] %s -- CmdQDma Video, InsertHdr(%s),")
					TEXT("HdrLen(%d), HdrBuf(0x%x)\r\n"),
					DMX_FUNC_NAME,
					((prVidInf->parCmdQTxEntry[u2CmdIdx].fgInsertHdr) ?
						TEXT("TRUE") : TEXT("FALSE")),
					prVidInf->parCmdQTxEntry[u2CmdIdx].u4InsertHdrLen,
					prVidInf->parCmdQTxEntry[u2CmdIdx].au1InsertHdr);
			}
#endif /* ENABLE_DMX_ADVANCED_VER*/
		}

		u8EndCmdOfst += prVidInf->parCmdQTxEntry[rInf.u2TxEntryCnt - 1].u4TxOfst;
#ifdef __linux__
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DMAPBBUF_V,
			TEXT("[Spt] %s -- CmdQCnt: 0x%04x, SrcOfst: %lld, Len: %lld,")
			TEXT("(EndCmd's Ofst: %lld, Len: %d)!\r\n"),
			DMX_FUNC_NAME, rInf.u2TxEntryCnt,  prVidInf->u8FileOfst,
			prVidInf->u8Len, u8EndCmdOfst,
			prVidInf->parCmdQTxEntry[rInf.u2TxEntryCnt - 1].u4TxLen);
#else
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DMAPBBUF_V,
			TEXT("[Spt] %s -- CmdQCnt: 0x%04x, SrcOfst: %I64d, Len: %I64d,")
			TEXT("(EndCmd's Ofst: %I64d, Len: %d)!\r\n"),
			DMX_FUNC_NAME, rInf.u2TxEntryCnt,  prVidInf->u8FileOfst,
			prVidInf->u8Len, u8EndCmdOfst,
			prVidInf->parCmdQTxEntry[rInf.u2TxEntryCnt - 1].u4TxLen);
#endif /* #ifdef __linux__*/
	} else {

#if ENABLE_DMX_ADVANCED_VER
		rInf.fgInsertHdr = prVidInf->fgInsertHdr;
		rInf.u4InsertHdrLen = prVidInf->u4InsertHdrLen;
		rInf.pu1InsertHdrBuf = prVidInf->pu1InsertHdrBuf;

		if (rInf.fgInsertHdr) {
			if (rInf.u4InsertHdrLen > DMX_MAX_INST_BYTES_CNT) {
				DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail for Insert Bytes Count(%d) should be <= %d\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, rInf.u4InsertHdrLen, DMX_MAX_INST_BYTES_CNT);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_OVER_LIMIT);
			}
		}

		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_INSTBS,
			TEXT("[SPT] %s line %d -- Video InsertHdr(%s),HdrLen(%d), HdrBuf(0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, ((rInf.fgInsertHdr) ? TEXT("TRUE") : TEXT("FALSE")),
			rInf.u4InsertHdrLen, rInf.pu1InsertHdrBuf);
#endif /* ENABLE_DMX_ADVANCED_VER*/

#ifdef __linux__
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DMAPBBUF_V,
			TEXT("[Spt] %s -- CmdQCnt: 0,SrcOfst: %lld, Len: %lld!\r\n"),
			DMX_FUNC_NAME, prVidInf->u8FileOfst, prVidInf->u8Len);
#else
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DMAPBBUF_V,
			TEXT("[Spt] %s -- CmdQCnt: 0,SrcOfst: %I64d, Len: %I64d!\r\n"),
			DMX_FUNC_NAME, prVidInf->u8FileOfst, prVidInf->u8Len);
#endif /* #ifdef __linux__*/
	}


	if (g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_VID])
		DmxDumpVDmaInfo(prVidInf->u8FileOfst, NULL, prVidInf->u8Len);

#if DMX_PFM_TEST
	DmxPfmStmSWDmaStart(pvSptHdl, SPT_DATA_V);
#endif /* DMX_PFM_TEST*/

#ifdef __linux__
	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DMAPBBUF_V,
		TEXT("[SPT] %s line %d -- Ofst: %lld, Len: %lld,TotalAULen: %lld,")
		TEXT("TxLen: %lld, pvSptHdl: 0x%p\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, rInf.u8FromFileOfst, rInf.u8TxLen,
		rInf.u8TotalAULen, rInf.u8TxLen, pvSptHdl);
#else
	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DMAPBBUF_V,
		TEXT("[SPT] %s line %d -- Ofst: %I64d, Len: %I64d, TotalAULen: %I64d,")
		TEXT("TxLen: %I64d, pvSptHdl: 0x%p\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, rInf.u8FromFileOfst, rInf.u8TxLen,
		rInf.u8TotalAULen, rInf.u8TxLen, pvSptHdl);
#endif /* #ifdef __linux__*/

	mrRet = SplitterPbb2Fifo(pvSptHdl, &rInf);

	if (DMX_FAILED(mrRet)) {
		SplitterSetEOSForError(pvSptHdl, mrRet);

		if (!MM_IS_STATE_ERROR(mrRet)) {
#ifdef __linux__
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterPbb2Fifo(u8Len: %lld, Offset:")
				TEXT(" %lld, CMDQ: %d),mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prVidInf->u8Len, prVidInf->u8FileOfst,
				    (prVidInf->fgUseCmdQ ? 1 : 0), mrRet, pvSptHdl);
#else
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterPbb2Fifo(u8Len: %I64d, Offset:")
				TEXT(" %I64d, CMDQ: %d),mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prVidInf->u8Len, prVidInf->u8FileOfst,
				    (prVidInf->fgUseCmdQ ? 1 : 0), mrRet, pvSptHdl);
#endif /* #ifdef __linux__*/
		} else {
			MM_RETURN(RET_DMX_OK);
		}
	}

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* Spt4CfaPbb2AFifoAUCtrl*/
/* Tx Designate Pbbuf's data into Audio FIFO, and compose one AU*/
/* @Param prAudInf [IN] Cfa Tx Audio Info, including FileOfst,
TxLen, Pts, StreamUID, whether is dummy AU*/
/*									Audio Code, whether it is the AU End, AU start*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT Spt4CfaPbb2AFifoAUCtrl(void *pvSptHdl, CFA_AUDIO_INFO_T *prAudInf)
{
	DMX_SPT_DMA2FIFO_INFO_T rInf;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prAudInf) ||
	    (NULL == pvSptHdl))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	mm_memset(&rInf, 0, sizeof(DMX_SPT_DMA2FIFO_INFO_T));

	rInf.u8FromFileOfst = prAudInf->u8FileOfst;
	rInf.pvFromAddress	= NULL;
	rInf.u8TxLen		= prAudInf->u8Len;
	rInf.u4TxStreamType = SPT_DATA_A;
	rInf.pvToAddress	= NULL;
	rInf.u4TxVideoCodec = VC_UNKNOW;
	rInf.u4TxPictureMode = 0;
	rInf.u8PtsSa		= prAudInf->u8Pts;
	rInf.u8PtsEa		= INVALID_TIMESTAMP;
	rInf.u4TxUID		= prAudInf->u4PrsStrmId;
	rInf.fgDummyUnit	= prAudInf->fgDummyAU;
	rInf.u4TxAudioCodec = Spt4CfaGetAudioCodec(prAudInf->eAudType);
	rInf.fgAUByEnd		= prAudInf->fgAUCompleteByEnd;
	rInf.fgUnitEnd		= prAudInf->fgUnitEnd;
	rInf.fgCreateAU		= prAudInf->fgUnitStart;
	rInf.u8TotalAULen	= prAudInf->u8TotalAULen;

	if ((AUD_DRV_FMT_SACD == rInf.u4TxAudioCodec) &&
	    (rInf.fgAUByEnd) &&
	    (prAudInf->fgUnitStart))
		rInf.rExInf.rSACD.fgDST = prAudInf->rExInf.rSACD.fgDST;

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));

		rOperInfo.pvSptHdl = pvSptHdl;
		rOperInfo.unFlow.rDma.u4StmType = SPT_DATA_A;
		rOperInfo.unFlow.rDma.u8FileOfst = rInf.u8FromFileOfst;
		rOperInfo.unFlow.rDma.pvBuf = NULL;
		rOperInfo.unFlow.rDma.u8Len = rInf.u8TxLen;
		DmxDumpFlow(DMX_OPER_SW_START_PTX, &rOperInfo);
	}

	if (prAudInf->fgUseCmdQ) {
		rInf.fgUseCmdQ = prAudInf->fgUseCmdQ;
		rInf.fgAUByCmdQEnd = prAudInf->fgAUByCmdQEnd;

		if (rInf.u2TxEntryCnt > DMX_MAX_TX_CNT_FOR_CMD_Q) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail for CmdQ Entry Cnt(%d) > MAXCNT(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, rInf.u2TxEntryCnt, DMX_MAX_TX_CNT_FOR_CMD_Q);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_OVER_LIMIT);
		}

		rInf.u2TxEntryCnt = prAudInf->u2TxEntryCnt;
		rInf.u8RealTxLen = prAudInf->u8RealTxLen;
		rInf.parCmdQTxEntry = prAudInf->parCmdQTxEntry;
#if ENABLE_DMX_ADVANCED_VER
		{
			u16 u2CmdIdx = 0;

			for (u2CmdIdx = 0; u2CmdIdx <= rInf.u2TxEntryCnt - 1; u2CmdIdx++) {
				DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_INSTBS,
					TEXT("[SPT] %s -- CmdQDma Audio, EntryIdx(%d) InsertHdr(%s),")
					TEXT("HdrLen(%d), u4TxLen(%d)\r\n"),
					DMX_FUNC_NAME, u2CmdIdx,
					((prAudInf->parCmdQTxEntry[u2CmdIdx].fgInsertHdr) ?
						TEXT("TRUE") : TEXT("FALSE")),
					prAudInf->parCmdQTxEntry[u2CmdIdx].u4InsertHdrLen,
					prAudInf->parCmdQTxEntry[u2CmdIdx].u4TxLen);
			}
		}
#endif /* ENABLE_DMX_ADVANCED_VER*/
	} else {
#if ENABLE_DMX_ADVANCED_VER
		rInf.fgInsertHdr = prAudInf->fgInsertHdr;
		rInf.pu1InsertHdrBuf = prAudInf->pu1InsertHdrBuf;
		rInf.u4InsertHdrLen = prAudInf->u4InsertHdrLen;

		if (rInf.fgInsertHdr) {
			if (rInf.u4InsertHdrLen > DMX_MAX_INST_BYTES_CNT) {
				DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail for Insert Bytes Count(%d) should be <= %d\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, rInf.u4InsertHdrLen, DMX_MAX_INST_BYTES_CNT);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_OVER_LIMIT);
			}
		}

		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_INSTBS,
		TEXT("[SPT] %s -- Audio InsertHdr(%s), HdrLen(%d),HdrBuf(0x%x)\r\n"),
		DMX_FUNC_NAME, ((rInf.fgInsertHdr) ? TEXT("TRUE") : TEXT("FALSE")),
		rInf.u4InsertHdrLen, rInf.pu1InsertHdrBuf);
#endif /* ENABLE_DMX_ADVANCED_VER*/
	}

	if (g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_AUD])
		DmxDumpADmaInfo(pvSptHdl, prAudInf->u8FileOfst, NULL, prAudInf->u8Len, rInf.u4TxUID);

#if DMX_PFM_TEST
	DmxPfmStmSWDmaStart(pvSptHdl, SPT_DATA_A);
#endif /* DMX_PFM_TEST*/

#ifdef __linux__
	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DMAPBBUF_A,
		TEXT("[SPT] %s -- u4TxUID: %d,TotalAULen: %lld, TxLen: %lld, ")
		TEXT("fgCreateAU: %d,CompleteByEnd: %d, fgUnitEnd: %d, pvSptHdl: 0x%p\r\n"),
		DMX_FUNC_NAME, rInf.u4TxUID,
		rInf.u8TotalAULen, rInf.u8TxLen,
		((rInf.fgCreateAU) ? 1 : 0),
		((prAudInf->fgAUCompleteByEnd) ? 1 : 0),
		((prAudInf->fgUnitEnd) ? 1 : 0), pvSptHdl);
	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DMAPBBUF_A,
		TEXT("[SPT] %s line %d -- Ofst: %lld, Len: %lld\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, rInf.u8FromFileOfst, rInf.u8TxLen);
#else
	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DMAPBBUF_A,
		TEXT("[SPT] %s -- u4TxUID: %d,TotalAULen: %I64d, TxLen: %I64d,")
		TEXT(" fgCreateAU: %d,CompleteByEnd: %d, fgUnitEnd: %d, pvSptHdl: 0x%p\r\n"),
		DMX_FUNC_NAME, rInf.u4TxUID,
		rInf.u8TotalAULen, rInf.u8TxLen,
		((rInf.fgCreateAU) ? 1 : 0),
		((prAudInf->fgAUCompleteByEnd) ? 1 : 0),
		((prAudInf->fgUnitEnd) ? 1 : 0), pvSptHdl);
	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DMAPBBUF_A,
		TEXT("[SPT] %s line %d -- Ofst: %I64d, Len: %I64d\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, rInf.u8FromFileOfst, rInf.u8TxLen);
#endif /* #ifdef __linux__*/

	mrRet = SplitterPbb2Fifo(pvSptHdl, &rInf);

	if (DMX_FAILED(mrRet)) {
		SplitterSetEOSForError(pvSptHdl, mrRet);

		if (!MM_IS_STATE_ERROR(mrRet)) {
#ifdef __linux__
			DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in SplitterPbb2Fifo(u8Len: %lld, Offset: ")
			TEXT("%lld, CMDQ: %d),mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prAudInf->u8Len, prAudInf->u8FileOfst,
			(prAudInf->fgUseCmdQ ? 1 : 0), mrRet, pvSptHdl);
#else
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterPbb2Fifo(u8Len: %I64d, ")
				TEXT("Offset: %I64d, CMDQ: %d),mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prAudInf->u8Len, prAudInf->u8FileOfst,
				    (prAudInf->fgUseCmdQ ? 1 : 0), mrRet, pvSptHdl);
#endif /* #ifdef __linux__*/
		} else
			MM_RETURN(RET_DMX_OK);
	}

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* Spt4CfaPbb2SpFifoAUCtrl*/
/* Tx Designate Pbbuf's data into SP FIFO, and compose one AU*/
/* @Param prSubPicInf [IN] Cfa Tx SP Info, including FileOfst, TxLen, Pts, EndPts, StreamUID,*/
/*										whether is dummy AU,
whether it is the AU End, AU start*/
/* @Param u8TotalAULen [IN] Total AU Len*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT Spt4CfaPbb2SpFifoAUCtrl(void *pvSptHdl, CFA_SUBPIC_INFO_T *prSubPicInf,
				u64 u8TotalAULen)
{
	DMX_SPT_DMA2FIFO_INFO_T rInf;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prSubPicInf) ||
	    (NULL == pvSptHdl))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	mm_memset(&rInf, 0, sizeof(DMX_SPT_DMA2FIFO_INFO_T));

	rInf.u8FromFileOfst = prSubPicInf->u8FileOfst;
	rInf.pvFromAddress	= NULL;
	rInf.u8TxLen		= prSubPicInf->u8Len;
	rInf.u4TxStreamType = SPT_DATA_SP;
	rInf.pvToAddress	= NULL;
	rInf.u4TxVideoCodec = VC_UNKNOW;
	rInf.u4TxPictureMode = 0;
	rInf.u8PtsSa		= prSubPicInf->u8Pts;
	rInf.u8PtsEa		= prSubPicInf->u8EndPts;
	rInf.u4TxUID		= prSubPicInf->u4PrsStrmId;
	rInf.fgDummyUnit	= prSubPicInf->fgDummyAU;
	rInf.fgCreateAU		= prSubPicInf->fgUnitStart;
	rInf.u8TotalAULen	= u8TotalAULen;

	if (g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_SP]) {
		DmxDumpSPDmaInfo(pvSptHdl, prSubPicInf->u8FileOfst, NULL,
				 prSubPicInf->u8Len, rInf.u4TxUID);
	}

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));

		rOperInfo.pvSptHdl = pvSptHdl;
		rOperInfo.unFlow.rDma.u4StmType = SPT_DATA_SP;
		rOperInfo.unFlow.rDma.u8FileOfst = rInf.u8FromFileOfst;
		rOperInfo.unFlow.rDma.pvBuf = NULL;
		rOperInfo.unFlow.rDma.u8Len = rInf.u8TxLen;
		DmxDumpFlow(DMX_OPER_SW_START_PTX, &rOperInfo);
	}

#if DMX_PFM_TEST
	DmxPfmStmSWDmaStart(pvSptHdl, SPT_DATA_SP);
#endif /* DMX_PFM_TEST*/

	mrRet = SplitterPbb2Fifo(pvSptHdl, &rInf);

	if (DMX_FAILED(mrRet)) {
		SplitterSetEOSForError(pvSptHdl, mrRet);

		if (!MM_IS_STATE_ERROR(mrRet)) {
#ifdef __linux__
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterPbb2Fifo(u8Len: %lld, Offset: %lld)")
				TEXT(",mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSubPicInf->u8Len, prSubPicInf->u8FileOfst,
				mrRet, pvSptHdl);
#else
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterPbb2Fifo(u8Len: %I64d,")
				TEXT(" Offset: %I64d),mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSubPicInf->u8Len, prSubPicInf->u8FileOfst,
				    mrRet, pvSptHdl);
#endif /* #ifdef __linux__*/
		} else
			MM_RETURN(RET_DMX_OK);
	}

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* Spt4CfaPbb2AFifo*/
/* Tx PBBuf's data into Audio FIFO, but don't compose one AU*/
/* @Param pucSrc [IN] TX Src Memory VSA*/
/* @Param u8Position [IN] Corresponding  File Ofst*/
/* @Param u8Len [IN] Tx Len*/
/* @Param u8Pts [IN]*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT Spt4CfaPbb2AFifo(void *pvSptHdl, u64 u8FileOfst, u64 u8Pts,
			 u64 u8Len, CfaApiAudType eAudType)
{
	DMX_SPT_DMA2FIFO_INFO_T rInf;
	MRESULT mrRet = RET_DMX_OK;

	if ((0 == u8Len) ||
	    (NULL == pvSptHdl))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	DMXLOG_DEBUG(TEXT("[SPT] %s -- enter, pvSptHdl: 0x%p\r\n"),
		    DMX_FUNC_NAME, pvSptHdl);

	mm_memset(&rInf, 0, sizeof(DMX_SPT_DMA2FIFO_INFO_T));

	rInf.u8FromFileOfst = u8FileOfst;
	rInf.pvFromAddress	= NULL;
	rInf.u8TxLen		= u8Len;
	rInf.u4TxStreamType = SPT_DATA_A;
	rInf.pvToAddress	= NULL;
	rInf.u4TxVideoCodec = VC_UNKNOW;
	rInf.u4TxAudioCodec = Spt4CfaGetAudioCodec(eAudType);
	rInf.u4TxPictureMode = 0;
	rInf.u8PtsSa		= u8Pts;
	rInf.u8PtsEa		= INVALID_TIMESTAMP;
	rInf.u4TxUID		= GetStmUIDByType(pvSptHdl, rInf.u4TxStreamType);


	if (g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_AUD])
		DmxDumpADmaInfo(pvSptHdl, u8FileOfst, NULL, u8Len, rInf.u4TxUID);

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

		mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));

		rOperInfo.pvSptHdl = pvSptHdl;
		rOperInfo.unFlow.rDma.u4StmType = SPT_DATA_A;
		rOperInfo.unFlow.rDma.u8FileOfst = rInf.u8FromFileOfst;
		rOperInfo.unFlow.rDma.pvBuf = NULL;
		rOperInfo.unFlow.rDma.u8Len = rInf.u8TxLen;
		DmxDumpFlow(DMX_OPER_SW_START_PTX, &rOperInfo);
	}

#if DMX_PFM_TEST
	DmxPfmStmSWDmaStart(pvSptHdl, SPT_DATA_A);
#endif /* DMX_PFM_TEST*/

	mrRet = SplitterPbb2Fifo(pvSptHdl, &rInf);

	if (DMX_FAILED(mrRet)) {
		SplitterSetEOSForError(pvSptHdl, mrRet);

		if (!MM_IS_STATE_ERROR(mrRet)) {
#ifdef __linux__
			DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in SplitterPbb2Fifo(u8Len: %lld, ")
			TEXT("Offset: %lld),mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u8Len, u8FileOfst, mrRet, pvSptHdl);
#else
			DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in SplitterPbb2Fifo(u8Len: %I64d, Offset:")
			TEXT(" %I64d),mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u8Len, u8FileOfst, mrRet, pvSptHdl);
#endif /* #ifdef __linux__*/
		} else
			MM_RETURN(RET_DMX_OK);
	}

	MM_RETURN(mrRet);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* Spt4CfaFinishedEx*/
/* If Cfa Parser end, check whether there are rmain data need to tx to fifo, if is, do tx*/
/* And then inform Splitter Cfa parse end*/
/* fgCompulsory [IN] Indicate whether to do finished process compulsorily or not*/
/*					 if false, we will check jump status in RW*/
/* u4Status		[IN] Finish Status, if fgCompulsory = FALSE, its value must be GAU_E_EOS*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT Spt4CfaFinishedEx(void *pvSptHdl, u64 u8Offset, bool fgCompulsory, u32 u4Status)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	PSR_CC	*prPsrCC = NULL;
	DMX_INST_T *prDmxInst = NULL;
	u64	u8FileOfst = 0;
	MRESULT mrRet  = RET_DMX_OK;

	if (NULL == pvSptHdl) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args\r\n"),
			    DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrCC = (PSR_CC *)(prSpt->pvPsrCC);

	if (NULL == prPsrCC) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail for pvSptHdl(0x%p)'s prPsrCC = NULL\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	prDmxInst = (DMX_INST_T *)prSpt->pvDmxInst;
	if (NULL == prDmxInst) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	
	PSR_CC_LOCK(prPsrCC->rLock);

	if ((!fgCompulsory) && (!(DMX_IS_NORMAL_PLAY(pvSptHdl)))) {
		prPsrCC->u4TxPBBufJumpIdx = MAX_CACHE_PBBUF;
		PSR_CC_SetState(prSpt->pvPsrCC, CCS_TX);
		PSR_CC_SetTxSt(prSpt->pvPsrCC, TXS_TX_JUMP);
		SplitterSetEOSForError((void *)prSpt, RET_DMX_NEED_JUMP);
		PSR_CC_UNLOCK(prPsrCC->rLock);
		MM_RETURN(RET_DMX_OK);
	}

	mrRet = PSR_CC_GetCurrentOffset(prPsrCC, &u8FileOfst);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in PSR_CC_GetCurrentOffset, pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
	}

	mrRet = SplitterSetCfaPsrEnd(pvSptHdl, TRUE, u4Status);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in SplitterSetCfaPsrEnd (pvSptHdl: 0x%p, TRUE)\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
	}

	mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_IDLE, SPLITTER_TX_STATE_NONE);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in SplitterChangeState(IDLE, TX_NONE),")
			TEXT(" pvSptHdl: 0x%p,mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
		PSR_CC_UNLOCK(prPsrCC->rLock);
		SplitterSetEOSForError(pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	mrRet = SplitterPsrSetEOS(pvSptHdl, u4Status);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in SplitterPsrSetEOS, pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
		PSR_CC_UNLOCK(prPsrCC->rLock);
		MM_RETURN(mrRet);
	}

	DMXLOG_TRACE(
		TEXT("[SPT] %s success -- PsrCC's Current Offset: ")
		TEXT("0x%llx,Cfa Range Current Offset: 0x%llx\r\n"),
		DMX_FUNC_NAME, u8FileOfst - 1, u8Offset);

	PSR_CC_UNLOCK(prPsrCC->rLock);

	PSR_HAL_LOCK;

	if (prDmxInst->u4SptCnt > 1) {
		DMX_SPT_INST_T *prSptInst = NULL;
		PSR_CC *prPsrCC2 = NULL;
		u32 i = 0;

		for (i = 0; i < DMX_MAX_SPT_INST_CNT; i++) {
			prSptInst = g_rSptMan.aprSptInst[i];

			if (prDmxInst != prSptInst->pvDmxInst)
				continue;

			if ((NULL != prSptInst)  &&
			    (pvSptHdl != prSptInst) &&
			    (NULL != prSptInst->pvPsrCC)) {
				prPsrCC2 = (PSR_CC *)(prSptInst->pvPsrCC);

				if (!(prPsrCC2->fgCfaPrsEnd)) {
					switch (prPsrCC2->eTxState) {
					case TXS_PBBUF_OK:
					case TXS_FIFO_OK:
					case TXS_WAIT_HW:
					case TXS_WAIT_IRQ_PROC:
					case TXS_WAIT_FIFO:
					case TXS_WAIT_VFIFO_PTS_THRESHOLD: {
						DMXLOG_DEBUG(
							TEXT("[PSR] %s line %d -- PsrCC's eTxState: %d,")
							TEXT("PsrCC2's eTxState: %d\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							((NULL == prPsrCC) ? -1 : prPsrCC->eTxState),
							((NULL == prPsrCC2) ? -1 : prPsrCC2->eTxState));

						PSR_CC_CBSplitter(prPsrCC2, E_WAKEUP_ME, NULL);
						PSR_HAL_UNLOCK;
						MM_RETURN(RET_DMX_OK);
					}
					break;

					default:
						break;
					}
				}
			} else
				continue;
		}
	}

	PSR_HAL_UNLOCK;

	MM_RETURN(RET_DMX_OK);
}

#if CONFIG_DRV_HIGH_BITRATE_SPECIAL_PROC
/*/////////////////////////////////////////////////////////////////////////////*/
/* Spt4CfaNotiCurStrmInf*/
/* Notify Splitter's parser cc whether need to do high bitrate process*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT Spt4CfaNotiCurStrmInf(void *pvSptHdl, bool fgHighBitrate)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	MRESULT mrRet  = RET_DMX_OK;

	if (NULL == pvSptHdl) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args\r\n"),
			    DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mrRet = PSR_CC_NotiCurStrmInf(prSpt->pvPsrCC, fgHighBitrate);

	MM_RETURN(mrRet);
}
#endif

/*/////////////////////////////////////////////////////////////////////////////*/
/* Spt4CfaGetWMVParsingMode*/
/* Get WMV VC1 Mode*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT Spt4CfaGetWMVParsingMode(void *pvSptHdl, u8 *pucWMVParsingMode)
{
	MRESULT mrRet  = RET_DMX_OK;

	if (NULL == pvSptHdl) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mrRet = PSR_CC_GetWMVParsingMode(((DMX_SPT_INST_T *)pvSptHdl)->pvPsrCC, pucWMVParsingMode);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in PSR_CC_GetWMVParsingMode, mrRet: 0x%x, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, mrRet, pvSptHdl);
	}

	MM_RETURN(mrRet);
}

bool Spt4CfaSetESInputType(void *pvSptHdl, ES_INPUT_TYPE_T eESType)
{
	UNUSE_PARAMETER(pvSptHdl);
	return _PVR_SetInputType((PVR_INPUT_TYPE_T)eESType);
}

static MRESULT Spt4CfaClearStmGAUEvents(void *pvSptHdl, E_SPT_DATA_TYPE_T eDataType)
{
	DMX_STM_INST_T *prStm = NULL;
	MRESULT mrRet = RET_DMX_OK;

	prStm = (DMX_STM_INST_T *)GetStreamByType(pvSptHdl, (u32)eDataType);

	if (NULL == prStm)
		MM_RETURN(RET_DMX_OK);

	DMXLOG_DEBUG(
			TEXT("[SPT] %s(%s) begin to clear")
			TEXT("all invalid GAU events!\r\n"),
		    DMX_FUNC_NAME, ((eDataType < MAX_SPT_DATA_TYPE_CNT) ?
				g_aszSptDataTypeName[eDataType] : TEXT("UNKNOWN")));

	mrRet = GAU_ResetEvent((u32)(prStm->u4GAUHandle),
			    (GAU_EV_AU_IN			|
				GAU_EV_AUCMD_RELEASE	|
				GAU_EV_REACH_THRESHOLD	|
				GAU_EV_SKIP_THRESHOLD	|
				GAU_EV_DISABLE_GETAU	|
				GAU_EV_FIFO_FLUSH));

	if (DMX_FAILED(mrRet)) {
		DMXLOG_DEBUG(
			TEXT("[SPT] %s(%s) fail to clear all invalid GAU events!\r\n"),
			DMX_FUNC_NAME,
			((eDataType < MAX_SPT_DATA_TYPE_CNT) ?
				g_aszSptDataTypeName[eDataType] : TEXT("UNKNOWN")));
	} else {
		DMXLOG_DEBUG(
			TEXT("[SPT] %s(%s) succeed to clear all invalid GAU events!\r\n"),
			DMX_FUNC_NAME,
			((eDataType < MAX_SPT_DATA_TYPE_CNT) ?
				g_aszSptDataTypeName[eDataType] : TEXT("UNKNOWN")));
	}

	GAU_ClearThreshold((u32)(prStm->u4GAUHandle));

	MM_RETURN(mrRet);
}

MRESULT Spt4CfaClearAllGAUEvents(void *pvSptHdl)
{
	MRESULT mrRet = RET_DMX_OK;
	E_SPT_DATA_TYPE_T eDataType = SPT_DATA_UNDEFINE;

	eDataType = SPT_DATA_V;
	mrRet = Spt4CfaClearStmGAUEvents(pvSptHdl, eDataType);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_DEBUG(
			TEXT("[SPT] %s(%s) fail to clear all invalid GAU events!\r\n"),
			DMX_FUNC_NAME, g_aszSptDataTypeName[SPT_DATA_V]);
	} else {
		DMXLOG_DEBUG(
			TEXT("[SPT] %s(%s) succeed to clear all invalid GAU events!\r\n"),
			DMX_FUNC_NAME, g_aszSptDataTypeName[SPT_DATA_V]);
	}

	eDataType = SPT_DATA_A;
	mrRet = Spt4CfaClearStmGAUEvents(pvSptHdl, eDataType);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_DEBUG(
			TEXT("[SPT] %s(%s) fail to clear all invalid GAU events!\r\n"),
			DMX_FUNC_NAME, g_aszSptDataTypeName[SPT_DATA_A]);
	} else {
		DMXLOG_DEBUG(
		TEXT("[SPT] %s(%s) succeed to clear all invalid GAU events!\r\n"),
			    DMX_FUNC_NAME, g_aszSptDataTypeName[SPT_DATA_A]);
	}

	eDataType = SPT_DATA_SP;
	mrRet = Spt4CfaClearStmGAUEvents(pvSptHdl, eDataType);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_DEBUG(
			TEXT("[SPT] %s(%s) fail to clear all invalid GAU events!\r\n"),
			DMX_FUNC_NAME, g_aszSptDataTypeName[SPT_DATA_SP]);
	} else {
		DMXLOG_DEBUG(
			TEXT("[SPT] %s(%s) succeed to clear all invalid GAU events!\r\n"),
			DMX_FUNC_NAME, g_aszSptDataTypeName[SPT_DATA_SP]);
	}

	eDataType = SPT_DATA_SECTION;
	mrRet = Spt4CfaClearStmGAUEvents(pvSptHdl, eDataType);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_DEBUG(
			TEXT("[SPT] %s(%s) fail to clear all invalid GAU events!\r\n"),
			DMX_FUNC_NAME, g_aszSptDataTypeName[SPT_DATA_SECTION]);
	} else {
		DMXLOG_DEBUG(
			TEXT("[SPT] %s(%s) succeed to clear all invalid GAU events!\r\n"),
			DMX_FUNC_NAME, g_aszSptDataTypeName[SPT_DATA_SECTION]);
	}

	MM_RETURN(mrRet);
}

static MRESULT Spt4CfaClearStmData(void *pvStmHdl, u32 u4SptDataType)
{
	PSR_FILTER	   *pPsr   = NULL;
	DMX_STM_INST_T		  *prStm  = NULL;
	MRESULT			mrRet		= RET_DMX_OK;
	u32			u4AUCnt	= 0, u4AvailDataSz = 0;

	if (NULL == pvStmHdl)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	switch (u4SptDataType) {
	case SPT_DATA_V:
		break;

	case SPT_DATA_A:
		break;

	case SPT_DATA_SP:
		break;

	case SPT_DATA_SECTION:
		break;

	default:
		MM_RETURN(RET_DMX_OK);
        break;
	}

	prStm = (DMX_STM_INST_T *)pvStmHdl;

	if (StreamIsEnabled(prStm)) {
		pPsr = (PSR_FILTER *)(prStm->pvPsrFtr);

		mrRet = ESM_FifoGetAvailDataSize(pPsr->u4ESIH, &u4AvailDataSz);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in ESM_FifoGetAvailDataSize")
				TEXT("(u4Handle: 0x%x),mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pPsr->u4ESIH, mrRet);
			MM_RETURN(mrRet);
		}

		mrRet = ESM_AUTableGetAvailCount(pPsr->u4ESIH, &u4AUCnt);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in ESM_AUTableGetAvailCount")
				TEXT("(u4Handle: 0x%x),mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pPsr->u4ESIH, mrRet);
			MM_RETURN(mrRet);
		}

		DMXLOG_TRACE(
			TEXT("[SPT] %s(%s) Before Flush, Avail AU Count: %d,")
			TEXT(" Avail Data Sz: %d\r\n"),
			DMX_FUNC_NAME,
			((u4SptDataType < MAX_SPT_DATA_TYPE_CNT) ?
				g_aszSptDataTypeName[u4SptDataType] : TEXT("UNKNOWN")),
			    u4AUCnt, u4AvailDataSz);

		mrRet = PSR_Filter_Disable(prStm->pvPsrFtr);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s(%s) fail in PSR_Filter_Disable\r\n"),
				DMX_FUNC_NAME,
				((u4SptDataType < MAX_SPT_DATA_TYPE_CNT) ?
					g_aszSptDataTypeName[u4SptDataType] : TEXT("UNKNOWN")));
			goto ENABLDMYDEC;
		}

		mrRet = PSR_Filter_Flush(prStm->pvPsrFtr);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s(%s) fail in PSR_Filter_Flush, strmtype: 0x%x\r\n"),
				DMX_FUNC_NAME,
				((u4SptDataType < MAX_SPT_DATA_TYPE_CNT) ?
					g_aszSptDataTypeName[u4SptDataType] : TEXT("UNKNOWN")),
				prStm->u4StmType);
			goto ENABLDMYDEC;
		}

		mrRet = PSR_Filter_Enable(prStm->pvPsrFtr);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s(%s) fail in PSR_Filter_Enable, strmtype: 0x%x\r\n"),
				DMX_FUNC_NAME,
				((u4SptDataType < MAX_SPT_DATA_TYPE_CNT) ?
					g_aszSptDataTypeName[u4SptDataType] : TEXT("UNKNOWN")),
				prStm->u4StmType);
			goto ENABLDMYDEC;
		}

		mrRet = SplitterCreateCmdAU(prStm, u4SptDataType);

		if (DMX_SUCCEED(mrRet)) {
			EV_GRP_EVENT_T u4Event = 0;

			mrRet = GAU_SetSkipThreshold((u32)(prStm->u4GAUHandle));

			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT("[SPT] %s line %d (%s) fail in GAU_SetSkipThreshold!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					((u4SptDataType < MAX_SPT_DATA_TYPE_CNT) ?
					g_aszSptDataTypeName[u4SptDataType] : TEXT("UNKNOWN")));
			} else {
				mrRet = GAU_GetEvent((u32)(prStm->u4GAUHandle),
						     GAU_EV_AUCMD_RELEASE, &u4Event,
						     DMX_WAIT_INFINITE);

				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(
						TEXT("[SPT] %s line %d (%s) fail in GAU_GetEvent")
						TEXT("(GAU_EV_AUCMD_RELEASE)!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((u4SptDataType < MAX_SPT_DATA_TYPE_CNT) ?
							g_aszSptDataTypeName[u4SptDataType] : TEXT("UNKNOWN")));
				} else {
					DMXLOG_ERROR(
						TEXT("[SPT] %s line %d (%s) Get the event(GAU_EV_AUCMD_RELEASE)!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						((u4SptDataType < MAX_SPT_DATA_TYPE_CNT) ?
							g_aszSptDataTypeName[u4SptDataType] : TEXT("UNKNOWN")));
				}
			}

			mm_memset(g_arLogAudioAUs, 0, sizeof(AU_AUDIO) * DMX_MAX_LOG_AUDIO_AU_CNT);
		}

ENABLDMYDEC:
		mrRet = ESM_AUTableGetAvailCount(pPsr->u4ESIH, &u4AUCnt);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d (u4Handle: 0x%x) fail in ")
				TEXT("ESM_AUTableGetAvailCount,mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pPsr->u4ESIH, mrRet);
		}

		mrRet = ESM_FifoGetAvailDataSize(pPsr->u4ESIH, &u4AvailDataSz);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d (u4Handle: 0x%x) fail in ")
				TEXT("ESM_FifoGetAvailDataSize, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pPsr->u4ESIH, mrRet);
		}

		/* make GAU_GetAU do real fact*/
		DMXLOG_TRACE(
			TEXT("[SPT] %s(%s) success, Avail AU Count: %d, Avail Data Sz: %d\r\n"),
			DMX_FUNC_NAME,
			((u4SptDataType < MAX_SPT_DATA_TYPE_CNT) ?
				g_aszSptDataTypeName[u4SptDataType] : TEXT("UNKNOWN")),
			u4AUCnt, u4AvailDataSz);

	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT Spt4CfaTriggleAUCmdRelease(void *pvSptHdl, E_SPT_DATA_TYPE_T eDataType)
{
	DMX_STM_INST_T *prStm = NULL;
	MRESULT mrRet = RET_DMX_OK;

	prStm = (DMX_STM_INST_T *)GetStreamByType(pvSptHdl, (u32)eDataType);

	if (NULL == prStm)
		MM_RETURN(RET_DMX_OK);

	mrRet = GAU_SetEvent((u32)(prStm->u4GAUHandle), GAU_EV_AUCMD_RELEASE);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s(%s) fail in GAU_SetEvent(GAU_EV_AUCMD_RELEASE)!\r\n"),
			DMX_FUNC_NAME,
			((eDataType < MAX_SPT_DATA_TYPE_CNT) ?
				g_aszSptDataTypeName[eDataType] : TEXT("UNKNOWN")));
	}

	MM_RETURN(mrRet);
}

MRESULT Spt4CfaClearAllStmData(void *pvSptHdl)
{
	DMX_SPT_INST_T *prSpt  = (DMX_SPT_INST_T *)pvSptHdl;
	u32		   mrRet   = RET_DMX_OK;
	void *hVStm = NULL, *hAStm = NULL, *hSPStm = NULL, *hSectStm = NULL;
	DMX_STM_INST_T		  *prVStm = NULL, *prAStm = NULL, *prSPStm = NULL, *prSectStm = NULL;

	DMXLOG_TRACE(
		TEXT("[SPT] %s enter pvSptHdl:0x%p, State:0x%x\r\n"),
		DMX_FUNC_NAME, pvSptHdl, SplitterGetState(pvSptHdl));

	if (NULL == prSpt) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterIsEnable(pvSptHdl)) {
		DMXLOG_TRACE(
			TEXT("[SPT] %s success for Splitter is disable.\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	if (SPLITTER_STATE_IDLE == SplitterGetState(pvSptHdl)) {
		DMXLOG_TRACE(
			TEXT("[SPT] %s success for current state is idle\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	if (!GAU_IsReachThreshold()) {
		DMXLOG_TRACE(
			TEXT("[SPT] %s will clear all strm's fifo while threshold enabled!\r\n"),
			DMX_FUNC_NAME);
		hVStm = GetStreamByType(pvSptHdl, SPT_DATA_V);
		hAStm = GetStreamByType(pvSptHdl, SPT_DATA_A);
		hSPStm = GetStreamByType(pvSptHdl, SPT_DATA_SP);
		hSectStm = GetStreamByType(pvSptHdl, SPT_DATA_SECTION);

		prVStm = (DMX_STM_INST_T *)hVStm;
		prAStm = (DMX_STM_INST_T *)hAStm;
		prSPStm = (DMX_STM_INST_T *)hSPStm;
		prSectStm = (DMX_STM_INST_T *)hSectStm;

		Spt4CfaClearStmData(hVStm, SPT_DATA_V);
		Spt4CfaClearStmData(hAStm, SPT_DATA_A);

		DMXLOG_TRACE(TEXT("**** %s ENABLE THRESHOLD ****\r\n"),
			DMX_FUNC_NAME);

		/* If signal is 0, and all fifo are empty, make threshold to operate.*/
		mrRet = GAU_SetThreshold((u32)(prVStm->u4GAUHandle), prVStm->u4FifoThreshold);
		if ((NULL != prVStm) && (RET_DMX_OK != mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s(%s) fail in GAU_SetThresholdInfo, strmtype: 0x%x\r\n"),
				DMX_FUNC_NAME, g_aszSptDataTypeName[SPT_DATA_V], prVStm->u4StmType);
		}

		/* If signal is 0, and all fifo are empty, make threshold to operate.*/
		mrRet = GAU_SetThreshold((u32)(prAStm->u4GAUHandle), prAStm->u4FifoThreshold);
		if ((NULL != prAStm) && (RET_DMX_OK != mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s(%s) fail in GAU_SetThresholdInfo, strmtype: 0x%x\r\n"),
				    DMX_FUNC_NAME, g_aszSptDataTypeName[SPT_DATA_A], prAStm->u4StmType);
		}

		/* If signal is 0, and all fifo are empty, make threshold to operate.*/
		mrRet = GAU_SetThreshold((u32)(prSPStm->u4GAUHandle), prSPStm->u4FifoThreshold);
		if ((NULL != prSPStm) && (RET_DMX_OK != mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s(%s) fail in GAU_SetThresholdInfo, strmtype: 0x%x\r\n"),
				    DMX_FUNC_NAME, g_aszSptDataTypeName[SPT_DATA_SP], prSPStm->u4StmType);
		}

		/* If signal is 0, and all fifo are empty, make threshold to operate.*/
		mrRet = GAU_SetThreshold((u32)(prSectStm->u4GAUHandle), prSectStm->u4FifoThreshold);
		if ((NULL != prSectStm) && (RET_DMX_OK != mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s(%s) fail in GAU_SetThresholdInfo,")
				TEXT(" strmtype: 0x%x\r\n"),
				    DMX_FUNC_NAME, g_aszSptDataTypeName[SPT_DATA_SECTION],
				    prSectStm->u4StmType);
		}
	}

	MM_RETURN(mrRet);
}

bool	Spt4CfaIsNonHdrDectVCodec(CfaApiVidType eVidType)
{
	switch (eVidType) {
	case CFA_VID_RV30:
	case CFA_VID_RV40:
	case CFA_VID_WMV7:
	case CFA_VID_WMV8:
	case CFA_VID_WMV9:
	case CFA_VID_DIVX3:
	case CFA_VID_MJPEG:
	case CFA_VID_H263_SORENSON:
	case CFA_VID_VP6:
	case CFA_VID_VP6A:
	case CFA_VID_VP8:
		return true;

	default:
		return false;
	}
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* The following functions should be removed*/
/*/////////////////////////////////////////////////////////////////////////////*/

MRESULT Spt4CfaSubPicFound(void *pvSptHdl, u8  ucSpStId)
{
	UNUSE_PARAMETER(pvSptHdl);
	UNUSE_PARAMETER(ucSpStId);
	MM_RETURN(RET_DMX_OK);
}

MRESULT Spt4CfaAutoPause(void *pvSptHdl, void *pvAutoPausePar)
{
	UNUSE_PARAMETER(pvSptHdl);
	UNUSE_PARAMETER(pvAutoPausePar);

	MM_RETURN(RET_DMX_OK);
}

MRESULT Spt4CfaPbb2Skip(void *pvSptHdl, u64 u8FileOfst, u64 u8Len)
{
	UNUSE_PARAMETER(u8FileOfst);
	UNUSE_PARAMETER(u8Len);
	UNUSE_PARAMETER(pvSptHdl);

	MM_RETURN(RET_DMX_OK);
}

MRESULT Spt4CfaInqInfNotify(void *pvSptHdl, u32 u4InfType)
{
	UNUSE_PARAMETER(pvSptHdl);
	UNUSE_PARAMETER(u4InfType);
	MM_RETURN(RET_DMX_OK);
}

MRESULT Spt4CfaPTSNotify(void *pvSptHdl, u64 u8CfaPTS)
{
	UNUSE_PARAMETER(pvSptHdl);
	UNUSE_PARAMETER(u8CfaPTS);
	MM_RETURN(RET_DMX_OK);
}

MRESULT Spt4CfaSetLpcmEmphasis(void *pvSptHdl, bool fgEmp)
{
	UNUSE_PARAMETER(pvSptHdl);
	UNUSE_PARAMETER(fgEmp);
	MM_RETURN(RET_DMX_OK);
}

MRESULT Spt4CfaTurnCPS(void *pvSptHdl, CFA_CPS_INFO_T *prCPSInf)
{
	UNUSE_PARAMETER(pvSptHdl);
	UNUSE_PARAMETER(prCPSInf);
	MM_RETURN(RET_DMX_OK);
}

MRESULT Spt4CfaSetCPSInfo(void *pvSptHdl, void *pvPathPar, CFA_CPS_INFO_PARAMS *prCPS)
{
	UNUSE_PARAMETER(pvSptHdl);
	UNUSE_PARAMETER(pvPathPar);
	UNUSE_PARAMETER(prCPS);

	MM_RETURN(RET_DMX_OK);
}

MRESULT Spt4CfaTurnDivxDRM(void *pvSptHdl, CFA_DIVXDRM_INFO_T *prDivxDRMInf)
{
#if DMX_SUPPORT_DIVXDRM
	MRESULT mrRet = RET_DMX_OK;

	if (prDivxDRMInf->fgOn) {
		mrRet = SplitterSetDecryptType(pvSptHdl, DECRYPT_DIVXDRM);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SplitterSetDecryptType,")
				TEXT(" pvSptHdl(0x%x), mrRet(0x%x).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
			SplitterSetEOSForError(pvSptHdl, mrRet);

			if (MM_IS_STATE_ERROR(mrRet))
				MM_RETURN(RET_DMX_OK);
			else {
				DMXLOG_ERROR(
					TEXT("[SPT] %s fail in SplitterSetPtxDivxDRMInf")
					TEXT(" (pvSptHdl 0x%x), mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, pvSptHdl, mrRet);
					MM_RETURN(mrRet);
			}
		} else {
			DMXLOG_DEBUG(
			TEXT("[SPT] %s -- SplitterSetDecryptType success\r\n"),
				    DMX_FUNC_NAME);
		}

		if (DMX_DIVXDRM_INVALID_FRAMEIDX == prDivxDRMInf->u2FrameKeyIdx) {
			PSR_CC *prPsrCC = (PSR_CC *)SplitterGetPtxHandle(pvSptHdl);

			if (NULL != prPsrCC) {
				PSR_DECRYPT_DIVXDRM_PRIVDATA_T *prPrivData =
					(PSR_DECRYPT_DIVXDRM_PRIVDATA_T *)prPsrCC->rDecryptMan.pvPrivData;

				if ((NULL != prPrivData) &&
				    (DMX_INVALID_UINT32 != prPrivData->u4ProtectOffset)) {
					/* this means the audio data doesn't need to do decryption*/
					prDivxDRMInf->u8DecryptStOfst += prPrivData->u4ProtectOffset;
					prDivxDRMInf->u4DecryptLen = prPrivData->u4ProtectSize;
				} else {
					/* Turn Off Decryption*/
					prDivxDRMInf->fgOn = FALSE;
					prDivxDRMInf->u8DecryptStOfst = DMX_INVALID_UINT64;
					prDivxDRMInf->u4DecryptLen = 0;
				}

#if DMX_PRINT_DECRYPT_KEY_LOG
#ifdef __linux__
				DMXLOG_TRACE(
					TEXT("[SPT] %s (TurnAOnDecrypt)-- DecryptOfst: %lld,")
					TEXT(" DecryptLen: %d\r\n"),
					DMX_FUNC_NAME, prDivxDRMInf->u8DecryptStOfst, prDivxDRMInf->u4DecryptLen);
#else
				DMXLOG_TRACE(
					TEXT("[SPT] %s (TurnAOnDecrypt)-- DecryptOfst: %I64d, ")
					TEXT("DecryptLen: %d\r\n"),
					DMX_FUNC_NAME, prDivxDRMInf->u8DecryptStOfst,
					prDivxDRMInf->u4DecryptLen);
#endif /* #ifdef __linux__*/
#endif
			}
		} else {
#if DMX_PRINT_DECRYPT_KEY_LOG
#ifdef __linux__
			DMXLOG_TRACE(
				TEXT("[SPT] %s (TurnVOnDecrypt)-- DecryptOfst: %lld,")
				TEXT(" DecryptLen: %d\r\n"),
				DMX_FUNC_NAME, prDivxDRMInf->u8DecryptStOfst, prDivxDRMInf->u4DecryptLen);
#else
			DMXLOG_TRACE(
				TEXT("[SPT] %s (TurnVOnDecrypt)-- DecryptOfst: %I64d, ")
				TEXT("DecryptLen: %d\r\n")),
				DMX_FUNC_NAME, prDivxDRMInf->u8DecryptStOfst, prDivxDRMInf->u4DecryptLen);
#endif /* #ifdef __linux__*/
#endif
		}

		mrRet = SplitterSetPtxDivxDRMInf(pvSptHdl, prDivxDRMInf->u8DecryptStOfst,
						 prDivxDRMInf->u4DecryptLen, prDivxDRMInf->u2FrameKeyIdx);
	} else {
		mrRet = SplitterSetPtxDivxDRMInf(pvSptHdl, DMX_INVALID_UINT64, 0,
						 DMX_DIVXDRM_INVALID_FRAMEIDX);
	}

	if (DMX_FAILED(mrRet)) {
		SplitterSetEOSForError(pvSptHdl, mrRet);

		if (MM_IS_STATE_ERROR(mrRet))
			MM_RETURN(RET_DMX_OK);
		else {
			DMXLOG_ERROR(
				TEXT("[SPT] %s fail in SplitterSetPtxDivxDRMInf (pvSptHdl 0x%x),")
				TEXT(" mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, pvSptHdl, mrRet);
			MM_RETURN(mrRet);
		}
	}

	mrRet = SplitterPsrTurnDivxDRM(pvSptHdl, prDivxDRMInf);

	if (DMX_FAILED(mrRet)) {
		SplitterSetEOSForError(pvSptHdl, mrRet);

		if (MM_IS_STATE_ERROR(mrRet))
			MM_RETURN(RET_DMX_OK);
		else {
			DMXLOG_ERROR(
				TEXT("[SPT] %s fail in SplitterPsrTurnDivxDRM (pvSptHdl 0x%x),")
				TEXT(" mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, pvSptHdl, mrRet);
			MM_RETURN(mrRet);
		}
	}

#else
	UNUSE_PARAMETER(pvSptHdl);
	UNUSE_PARAMETER(prDivxDRMInf);
#endif /* #ifdef ENABLE_DIVXDRM*/

	MM_RETURN(RET_DMX_OK);
}

MRESULT Spt4CfaSCRGapCB(void *pvSptHdl, u64 u8ScrGap)
{
	UNUSE_PARAMETER(pvSptHdl);
	UNUSE_PARAMETER(u8ScrGap);

	MM_RETURN(RET_DMX_OK);
}

MRESULT Spt4CfaSCRNotify(void *pvSptHdl, u64 u8CfaSCR)
{
	UNUSE_PARAMETER(pvSptHdl);
	UNUSE_PARAMETER(u8CfaSCR);
	MM_RETURN(RET_DMX_OK);
}

u32	Spt4CfaGetStreamFifoSize(void *pvSptHdl, E_SPT_DATA_TYPE_T eType)
{
	DMX_STM_INST_T *prStm = NULL;
	PSR_FILTER *prPsrFtr = NULL;

	prStm = (DMX_STM_INST_T *)GetStreamByType(pvSptHdl, eType);

	if ((NULL == prStm) ||
	    (NULL == prStm->pvPsrFtr))
		return 0;

	prPsrFtr = (PSR_FILTER *)(prStm->pvPsrFtr);

	return prPsrFtr->u4ESFifoSize;
}

#if CONFIG_DRV_HDMI_RX
bool Spt4CfaAudInIsRAW(void *pvSptHdl)
{
	MRESULT mrRet = RET_DMX_OK;
	bool	fgIsAudInRaw = FALSE;

	mrRet = SplitterPsrAudInIsRaw(pvSptHdl, &fgIsAudInRaw);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in Splitter2PsrAudInIsRaw (pvSptHdl 0x%x),")
			TEXT(" mrRet: 0x%x!\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
		return FALSE;
	}

	return fgIsAudInRaw;
}

MRESULT Spt4CfaGetAudInParsingInfo(void *pvSptHdl, AUDIN_PARSING_INFO_T *prPsrInfo)
{
	MRESULT mrRet = RET_DMX_OK;

	mrRet = SplitterPsrGetAudInParsingInfo(pvSptHdl, prPsrInfo);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in Splitter2PsrGetAudInParsingInfo")
			TEXT(" (pvSptHdl 0x%x),mrRet: 0x%x!\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}
#endif /* CONFIG_DRV_HDMI_RX*/

#endif/* #ifdef FUNC_CFA_TO_SPT*/



