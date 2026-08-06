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

/*-----------------------------------------------------------------------------
include files
-----------------------------------------------------------------------------*/
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/drv_esm_if.h>
/* #include <media/atc/mm_debug.h> */

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_sema.h"

#include "cfa_if.h"
#include "cfa_macro.h"
#include "cfa_ts.h"
#include "cfa_ts_st_ctrl.h"
#include "dmx_spt_cfa.h"
#include "stc_hal.h"



/*-----------------------------------------------------------------------------
* Name: CfaTsInit
*
* Description:
*	   Init CFA TS
*
* Inputs:
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaTsInit(void *pvSptHdl, void **ppvCfaPrivData)
{
	CfaTsInst *prCfaTs = NULL;
	MRESULT mrRet = RET_DMX_OK;

	DMX_NewMemory(sizeof(CfaTsInst), prCfaTs);

	if (NULL == prCfaTs) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
            TEXT("[CFA TS]CfaTsInit(): alloc prCfaTs ERROR! \r\n"));

		MM_RETURN(RET_DMX_NO_MEM);
	}
	dmx_memset((void *)prCfaTs, 0, sizeof(CfaTsInst));
	mrRet = dmx_sema_create(&(prCfaTs->sCfaTs), DMX_SEMA_TYPE_BINARY, DMX_SEMA_STATE_UNLOCK);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
            TEXT("[CFA TS]CfaTsInit(): create semaphore FAIL!\n"));
		DMX_FreeMemory(prCfaTs);
		MM_RETURN(mrRet);
	}

	prCfaTs->pvSptHdl = pvSptHdl;
	prCfaTs->ptrMemAddr = DMX_INVALID_UINTPTR_T;
	
	DMX_NewHwMemory(CFA_TS_MAX_TS_PACKET_SIZE, prCfaTs->pucPacketBuf);
	if (NULL == prCfaTs->pucPacketBuf) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
            TEXT("[CFA TS]CfaTsInit(): new pucPacketBuf FAIL!\n"));
		dmx_sema_delete(prCfaTs->sCfaTs);
		DMX_FreeMemory(prCfaTs);
		MM_RETURN(RET_DMX_NO_MEM);
	}
#if CFG_ALLOC_CC_MEM_WHEN_INIT
	DMX_NewHwMemory(CFA_TS_MAX_PES_PACKET_LEN, prCfaTs->pucCcPacketBuf);
	if (NULL == prCfaTs->pucCcPacketBuf) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
            TEXT("[CFA TS]CfaTsInit(): new pucCcPacketBuf FAIL!\n"));
		dmx_sema_delete(prCfaTs->sCfaTs);
		DMX_FreeHwMemory((void *)prCfaTs->pucPacketBuf);
		DMX_FreeMemory(prCfaTs);

		MM_RETURN(RET_DMX_NO_MEM);
	}
	prCfaTs->u4CcPacketBufLen = CFA_TS_MAX_PES_PACKET_LEN;
#endif

	DMX_NewMemory(sizeof(DMX_CMDQ_TX_ENTRY_T) * DMX_MAX_TX_CNT_FOR_CMD_Q, prCfaTs->rVidInf.parCmdQTxEntry);
	if (NULL == prCfaTs->rVidInf.parCmdQTxEntry) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
            TEXT("[CFA TS]CfaTsInit(): alloc vid parCmdQTxEntry error! \r\n"));
		dmx_sema_delete(prCfaTs->sCfaTs);
		DMX_FreeHwMemory((void *)prCfaTs->pucPacketBuf);
#if CFG_ALLOC_CC_MEM_WHEN_INIT
		DMX_FreeHwMemory((void *)prCfaTs->pucCcPacketBuf);
		prCfaTs->u4CcPacketBufLen = 0;
#endif
		DMX_FreeMemory(prCfaTs);

		MM_RETURN(RET_DMX_NO_MEM);
	}

	DMX_NewMemory(sizeof(DMX_CMDQ_TX_ENTRY_T) * DMX_MAX_TX_CNT_FOR_CMD_Q, prCfaTs->rAudInf.parCmdQTxEntry);
	if (NULL == prCfaTs->rAudInf.parCmdQTxEntry) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
            TEXT("[CFA TS]CfaTsInit(): alloc aud parCmdQTxEntry error! \r\n"));
		dmx_sema_delete(prCfaTs->sCfaTs);
		DMX_FreeMemory(prCfaTs->rVidInf.parCmdQTxEntry);
		DMX_FreeHwMemory((void *)prCfaTs->pucPacketBuf);
#if CFG_ALLOC_CC_MEM_WHEN_INIT
		DMX_FreeHwMemory((void *)prCfaTs->pucCcPacketBuf);
		prCfaTs->u4CcPacketBufLen = 0;
#endif
		DMX_FreeMemory(prCfaTs);

		MM_RETURN(RET_DMX_NO_MEM);
	}

	DMX_NewMemory(sizeof(DMX_CMDQ_TX_ENTRY_T) * DMX_MAX_TX_CNT_FOR_CMD_Q,
		prCfaTs->rAudCmdQBakInfo.rAudInf.parCmdQTxEntry);
	if (NULL == prCfaTs->rAudCmdQBakInfo.rAudInf.parCmdQTxEntry) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
            TEXT("[CFA TS]CfaTsInit(): alloc aud parCmdQTxEntry error! \r\n"));
		dmx_sema_delete(prCfaTs->sCfaTs);
		DMX_FreeMemory(prCfaTs->rVidInf.parCmdQTxEntry);
		DMX_FreeMemory(prCfaTs->rAudInf.parCmdQTxEntry);
		DMX_FreeHwMemory((void *)prCfaTs->pucPacketBuf);
#if CFG_ALLOC_CC_MEM_WHEN_INIT
		DMX_FreeHwMemory((void *)prCfaTs->pucCcPacketBuf);
		prCfaTs->u4CcPacketBufLen = 0;
#endif
		DMX_FreeMemory(prCfaTs);

		MM_RETURN(RET_DMX_NO_MEM);
	}

	*ppvCfaPrivData = (void *)prCfaTs;
	DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
        TEXT("[CFA TS]CfaTsInit(): CfaTsInit OK!\n"));

	prCfaTs->u4CurVidPID = CFA_TS_INVALID_PID;
	prCfaTs->u4CurAudPID = CFA_TS_INVALID_PID;
	prCfaTs->u4CurCcPID  = CFA_TS_INVALID_PID;


	MM_RETURN(RET_DMX_OK);
}




/*-----------------------------------------------------------------------------
* Name: CfaTsTxDone
*
* Description:
*	   TS CFA callback for transfer done
*	   This function will be called after a transfer is complete.
*
* Inputs:
*	   [IN] handle of splitter
*	   [IN] Actual transferred data length.
		Normally this value should be equal to the u4Len in the previous transfer issue,
		unless file end is hit.
*	   [IN] pointer to CfaTsInst
*
* Outputs:
*
* Returns: s32
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaTsTxDone(void *pvSptHdl, u64 u8TxLen, void *pvPrivData, bool fgRsp)
{
	CfaTsInst *prCfaTs = NULL;
	if (NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsTxDone(): PARAM error!\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prCfaTs = (CfaTsInst *)pvPrivData;

#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaTs);
	MMATE_CHECK_STRUCT(prCfaTs->rRange);
	MMATE_CHECK_STRUCT(prCfaTs->rVidDeltaPesPtsInfo);
	MMATE_CHECK_STRUCT(prCfaTs->rAudDeltaPesPtsInfo);
	MMATE_CHECK_STRUCT(prCfaTs->rAudCmdQBakInfo);
#endif

	CfaTsTxDoneStCtrl(pvSptHdl, u8TxLen, prCfaTs);
	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: CfaTsSetStrmInf
*
* Description:
*	   Set Stream information
*
* Inputs:
*	   [IN] handle of splitter
*	   [IN] stream to set
*	   [IN] stream info
*	   [IN] pointer to CfaTsInst
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaTsSetStrmInf(void *pvSptHdl, u32 u4Strm, u32 u4Info,
							 void *pvPrivData)
{
	CfaTsInst *prCfaTs = NULL;
	if (NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsSetStrmInf(): PARAM error!\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prCfaTs = (CfaTsInst *)pvPrivData;
#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaTs);
	MMATE_CHECK_STRUCT(prCfaTs->rRange);
	MMATE_CHECK_STRUCT(prCfaTs->rVidDeltaPesPtsInfo);
	MMATE_CHECK_STRUCT(prCfaTs->rAudDeltaPesPtsInfo);
	MMATE_CHECK_STRUCT(prCfaTs->rAudCmdQBakInfo);
#endif

	switch (u4Strm) {
	case CFA_STRM_V:
		prCfaTs->u4VideoStreamID = u4Info;
		DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsSetStrmInf(): prCfaTs->u4VideoStreamID: 0x%x\n"),
			prCfaTs->u4VideoStreamID);
		break;

	case CFA_STRM_A:
		prCfaTs->u4AudioStreamID = u4Info;
		DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsSetStrmInf(): prCfaTs->u4AudioStreamID: 0x%x\n"),
			prCfaTs->u4AudioStreamID);
		break;

	case CFA_STRM_SEC:
		prCfaTs->u4SectionStreamID = u4Info;
		break;

	case CFA_STRM_SP:
		prCfaTs->u4SpStreamID = u4Info;
		break;
	case CFA_STRM_NV:
		break;

	default:   /* not support yet*/
		MM_RETURN(RET_DMX_PARAM_WRONG);
		break;
	}

	DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
		TEXT("[CFA TS]CfaTsSetStrmInf(): CfaTsSetStrmInf OK!\n"));

	MM_RETURN(RET_DMX_OK);
}

/*-----------------------------------------------------------------------------
 * Name: CfaTsEnableStrm
 *
 * Description:
 *		TS CFA sets stream to parse, may be combinations of V/A/S.
 *		splitter will ensure that pfvSetStrm() is only called in "off" or "paused" state.
 *
 * Inputs:
 *		[IN] handle of splitter
 *		[IN] streams to parse or to cancel parsing
 *		[IN] CFA_STREAM_ON:  The bits turned ON in u4StrmToPrs are the streams that FMPC
 *			  would like to parse.	CFA_STRM_OFF: The bits turned ON in u4StrmToPrs are the
 *			  streams that FMPC would like to stop parsing
 *		[IN] pointer to CfaTsInst
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaTsEnableStrm(void *pvSptHdl, u32 u4StrmToPrs, CfaStreamOp eOp,
							  void *pvPrivData)
{
	CfaTsInst *prCfaTs = NULL;
	if (NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsEnableStrm(): PARAM error!\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prCfaTs = (CfaTsInst *)pvPrivData;
#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaTs);
	MMATE_CHECK_STRUCT(prCfaTs->rRange);
	MMATE_CHECK_STRUCT(prCfaTs->rVidDeltaPesPtsInfo);
	MMATE_CHECK_STRUCT(prCfaTs->rAudDeltaPesPtsInfo);
	MMATE_CHECK_STRUCT(prCfaTs->rAudCmdQBakInfo);
#endif
	if (CFA_STREAM_ON == eOp) {	/* enable*/
		if (CFA_STRM_V & u4StrmToPrs)
			prCfaTs->u4CurPrsFlg |= CFA_TS_PRS_STRM_TYPE_V;

		if (CFA_STRM_A & u4StrmToPrs)
			prCfaTs->u4CurPrsFlg |= CFA_TS_PRS_STRM_TYPE_A;

		if (CFA_STRM_SEC & u4StrmToPrs)
			prCfaTs->u4CurPrsFlg |= CFA_TS_PRS_STRM_TYPE_SEC;

		if (CFA_STRM_SP & u4StrmToPrs)
			prCfaTs->u4CurPrsFlg |= CFA_TS_PRS_STRM_TYPE_SP;
			} else {	/* disable*/
		if (CFA_STRM_V & u4StrmToPrs)
			prCfaTs->u4CurPrsFlg &= ~((u32)CFA_TS_PRS_STRM_TYPE_V);

		if (CFA_STRM_A & u4StrmToPrs)
			prCfaTs->u4CurPrsFlg &= ~((u32)CFA_TS_PRS_STRM_TYPE_A);

		if (CFA_STRM_SEC & u4StrmToPrs)
			prCfaTs->u4CurPrsFlg &= ~((u32)CFA_TS_PRS_STRM_TYPE_SEC);

		if (CFA_STRM_SP & u4StrmToPrs)
			prCfaTs->u4CurPrsFlg &= ~((u32)CFA_TS_PRS_STRM_TYPE_SP);

	}

	DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
        TEXT("[CFA TS]CfaTsEnableStrm(): CfaTsEnableStrm OK!\n"));

	MM_RETURN(RET_DMX_OK);
}


static CfaApiVidType eMapVidCodec(AVCODECID_T eCodec)
{
    switch(eCodec)
    {
        case AVCODEC_ID_MPEG1:
        case AVCODEC_ID_MPEG2:
            return CFA_VID_MPEG2;

        case AVCODEC_ID_MPEG4:
            return CFA_VID_MPEG4;

        case AVCODEC_ID_H264:
            return CFA_VID_H264;

        case AVCODEC_ID_VC1:
            return CFA_VID_VC1;

        default:
            return CFA_VID_UNKNOWN;
    }
}

static  CfaApiAudType eMapAudCodec(AVCODECID_T eCodec)
{
    switch(eCodec)
    {
        case AVCODEC_ID_MPEG:
             return CFA_AUD_DRV_FMT_MPEG;

        case AVCODEC_ID_MP3:
             return CFA_AUD_DRV_FMT_MP3;

        case AVCODEC_ID_AAC_PURE:
            return CFA_AUD_DRV_FMT_AAC;

        case AVCODEC_ID_AC3:
            return CFA_AUD_DRV_FMT_AC3;

        case AVCODEC_ID_DTS:
            return CFA_AUD_DRV_FMT_DTS;
            
        case AVCODEC_ID_PCM:
            return CFA_AUD_DRV_FMT_PCM;
            
        case AVCODEC_ID_EAC3:
            return CFA_AUD_DRV_FMT_EAC3;
            
        case AVCODEC_ID_DTSHD_NO_XLL:
            return CFA_AUD_DRV_FMT_DTSHD_PRI_NO_XLL;
            
        case AVCODEC_ID_DTSHD_XLL:
            return CFA_AUD_DRV_FMT_DTSHD_PRI_XLL;

        default:
            return CFA_AUD_DRV_FMT_UNKNOWN;
    }
}


static MRESULT CfaTsInitSectionPrivate(void **ppvPrivate, CfaTsInst *prCfaTs)
{
	CfaTsSecPrivate_T *prPrivate = NULL;

	DMX_NewMemory(sizeof(CfaTsSecPrivate_T), prPrivate);
	if (NULL == prPrivate) {
		DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
            TEXT("[CFA TS]CfaTsInitSecPrivate(): FAIL!\n"));
		*ppvPrivate = NULL;

		MM_RETURN(RET_DMX_NO_MEM);
	}
	dmx_memset(prPrivate, 0, sizeof(CfaTsSecPrivate_T));
	*ppvPrivate = (void *)prPrivate;

	DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
        TEXT("[CFA TS]CfaTsInitSecPrivate(): OK!\n"));

	MM_RETURN(RET_DMX_OK);
}


static void CfaTsUninitSectionPrivate(void *pvPrivate)
{
	CfaTsSecPrivate_T *prPrivate = NULL;

	prPrivate = (CfaTsSecPrivate_T *)pvPrivate;

	DMX_FreeMemory(prPrivate);

	DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
        TEXT("[CFA TS]CfaTsUninitSectionPrivate(): CfaTsUninitSecPrivate OK!\n"));
}



static MRESULT CfaTsInitCcPrivate(void **ppvPrivate, CfaTsInst *prCfaTs)
{
	CfaTsCcPrivate_T *prPrivate = NULL;

	DMX_NewMemory(sizeof(CfaTsCcPrivate_T), prPrivate);
	if (NULL == prPrivate) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
            TEXT("[CFA TS]CfaTsInitCcPrivate(): new private FAIL!\n"));
		*ppvPrivate = NULL;

		MM_RETURN(RET_DMX_NO_MEM);
	}
	dmx_memset(prPrivate, 0, sizeof(CfaTsCcPrivate_T));

#if !CFG_ALLOC_CC_MEM_WHEN_INIT
	DMX_NewHwMemory(CFA_TS_MAX_PES_PACKET_LEN, prPrivate->pucDataBuf);
	if (NULL == prPrivate->pucDataBuf) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsInitCcPrivate(): CfaTsInitCcPrivate new data buffer FAIL!\n"));
		DMX_FreeMemory(prPrivate);
		*ppvPrivate = NULL;

		MM_RETURN(RET_DMX_NO_MEM);
	}
	prPrivate->u4BufLen = CFA_TS_MAX_PES_PACKET_LEN;
#endif

	*ppvPrivate = (void *)prPrivate;

	DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
        TEXT("[CFA TS]CfaTsInitCcPrivate(): CfaTsInitCcPrivate OK!\n"));

	MM_RETURN(RET_DMX_OK);
}



static void CfaTsUninitCcPrivate(void *pvPrivate)
{
	CfaTsCcPrivate_T *prPrivate = NULL;

	prPrivate = (CfaTsCcPrivate_T *)pvPrivate;

#if CFG_ALLOC_CC_MEM_WHEN_INIT
	prPrivate->pucDataBuf = NULL;
	prPrivate->u4DataLen = 0;
	prPrivate->u4BufLen = 0;
#else
	if (NULL != prPrivate->pucDataBuf) {
		DMX_FreeHwMemory(prPrivate->pucDataBuf);
		prPrivate->pucDataBuf = NULL;
		prPrivate->u4DataLen = 0;
		prPrivate->u4BufLen = 0;
	}
#endif

	DMX_FreeMemory(prPrivate);

	DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
        TEXT("[CFA TS]CfaTsUninitCcPrivate(): OK!\n"));
}


static MRESULT CfaTsInitVidPrivate(void **ppvPrivate, CfaTsInst *prCfaTs)
{
	CfaTsVidPrivate_T *prPrivate = NULL;

	DMX_NewMemory(sizeof(CfaTsVidPrivate_T), prPrivate);
	if (NULL == prPrivate) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
            TEXT("[CFA TS]CfaTsInitVidPrivate(): new private FAIL!\n"));
		*ppvPrivate = NULL;

		MM_RETURN(RET_DMX_NO_MEM);
	}
	dmx_memset(prPrivate, 0, sizeof(CfaTsVidPrivate_T));

#if CFG_SUPPORT_HDCP
	if ((prCfaTs->fgHDCP) ||
		((CFA_VID_H264 != prPrivate->eVCodec) &&
		(CFA_VID_MPEG2 != prPrivate->eVCodec)))
#else
	if ((CFA_VID_H264 != prPrivate->eVCodec) &&
		(CFA_VID_MPEG2 != prPrivate->eVCodec))
#endif
		{
		DMX_NewHwMemory(CFA_TS_MAX_PES_PACKET_LEN, prPrivate->pucDataBuf);
		if (NULL == prPrivate->pucDataBuf) {
			DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
                TEXT("[CFA TS]CfaTsInitVidPrivate(): new data buffer FAIL!\n"));
			DMX_FreeMemory(prPrivate);
			*ppvPrivate = NULL;

			MM_RETURN(RET_DMX_NO_MEM);
		}
		prPrivate->u4BufLen = CFA_TS_MAX_PES_PACKET_LEN;
	}

	*ppvPrivate = (void *)prPrivate;

	DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
        TEXT("[CFA TS]CfaTsInitVidPrivate(): CfaTsInitVidPrivate OK!\n"));

	MM_RETURN(RET_DMX_OK);
}



static void CfaTsUninitVidPrivate(void *pvPrivate)
{
	CfaTsVidPrivate_T *prPrivate = NULL;

	prPrivate = (CfaTsVidPrivate_T *)pvPrivate;

	if (NULL != prPrivate->pucDataBuf) {
		DMX_FreeHwMemory(prPrivate->pucDataBuf);
		prPrivate->pucDataBuf = NULL;
		prPrivate->u4DataLen = 0;
		prPrivate->u4BufLen = 0;
	}

	DMX_FreeMemory(prPrivate);

	/*DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
	TEXT("[CFA TS] CfaTsUninitVidPrivate OK!\n"));*/
}

static MRESULT CfaTsInitAudPrivate(void **ppvPrivate, CfaTsInst *prCfaTs)
{
	CfaTsAudPrivate_T *prPrivate = NULL;

	DMX_NewMemory(sizeof(CfaTsAudPrivate_T), prPrivate);
	if (NULL == prPrivate) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
            TEXT("[CFA TS]CfaTsInitAudPrivate(): new private FAIL!\n"));
		*ppvPrivate = NULL;

		MM_RETURN(RET_DMX_NO_MEM);
	}
	dmx_memset(prPrivate, 0, sizeof(CfaTsAudPrivate_T));

	if (
#ifdef BSP_WIFI_WFD
#if CFG_SUPPORT_HDCP
	prCfaTs->fgHDCP ||
#endif
#endif
	prCfaTs->fgAudSwDec)
	{
		DMX_NewHwMemory(CFA_TS_MAX_PES_PACKET_LEN, prPrivate->pucDataBuf);
		if (NULL == prPrivate->pucDataBuf) {
			DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
                TEXT("[CFA TS]CfaTsInitAudPrivate(): new data buffer FAIL!\n"));
			DMX_FreeMemory(prPrivate);
			*ppvPrivate = NULL;

			MM_RETURN(RET_DMX_NO_MEM);
		}
		prPrivate->u4BufLen = CFA_TS_MAX_PES_PACKET_LEN;
	}

	prPrivate->fgCcError = FALSE;
	*ppvPrivate = (void *)prPrivate;

	DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
        TEXT("[CFA TS]CfaTsInitAudPrivate(): CfaTsInitAudPrivate OK!\n"));

	MM_RETURN(RET_DMX_OK);
}


static void CfaTsUninitAudPrivate(void *pvPrivate)
{
	CfaTsAudPrivate_T *prPrivate = NULL;

	prPrivate = (CfaTsAudPrivate_T *)pvPrivate;
	if (NULL != prPrivate->pucDataBuf) {
		DMX_FreeHwMemory(prPrivate->pucDataBuf);
		prPrivate->pucDataBuf = NULL;
		prPrivate->u4DataLen = 0;
		prPrivate->u4BufLen = 0;
	}
	DMX_FreeMemory(prPrivate);

	DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
        TEXT("[CFA TS]CfaTsUninitAudPrivate(): CfaTsUninitAudPrivate OK!\n"));
}

static MRESULT CfaTsCfgAud(CfaTsInst *prCfaTs, CfaTsConfigInfo_T *prCfaTsCfgInf)
{
	u32 i = 0;

	prCfaTs->u4AudStreamNb = prCfaTsCfgInf->u4AStreamNb;
	if (prCfaTs->u4AudStreamNb > CFA_TS_AUD_STREAM_NB_MAX)
		prCfaTs->u4AudStreamNb = CFA_TS_AUD_STREAM_NB_MAX;

	prCfaTs->fgAudSwDec = prCfaTsCfgInf->fgAudSwDec;

	DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
        TEXT("[CFA TS]CfaTsCfgAud(): config audio filter\n"));
	for (i = 0; i < prCfaTs->u4AudStreamNb; i++) {
		CfaTsAStreamInfo_T *prAudInf = &prCfaTsCfgInf->arAStreamInfo[i];

		if (CFA_AUD_DRV_FMT_UNKNOWN != eMapAudCodec(prAudInf->eCodec)) {
			CfaTsAudPrivate_T *prAPriv = NULL;
			CfaTsPidFilter_T *prPidFilter = NULL;
			u32 u4Pid = (u32)prAudInf->u2Pid;

			if (NULL == prCfaTs->Pids[u4Pid]) {
				DMX_NewMemory(sizeof(CfaTsPidFilter_T), prCfaTs->Pids[u4Pid]);
				if (NULL == prCfaTs->Pids[u4Pid])
					MM_RETURN(RET_DMX_NO_MEM);

				dmx_memset((void *)prCfaTs->Pids[u4Pid], 0, sizeof(CfaTsPidFilter_T));

				prPidFilter = prCfaTs->Pids[u4Pid];

				prPidFilter->u4PID = u4Pid;
				prPidFilter->ePidType = TS_PESPACKET;
				prPidFilter->fgCcOk = FALSE;
				prPidFilter->i4LastCc = -1;
				prPidFilter->u4CcErrCnt = 0;
				prPidFilter->fgUnitStart = FALSE;
				prPidFilter->fgTxDataFromUinitStart = TRUE;
				prPidFilter->pfInitPrivate = CfaTsInitAudPrivate;
				prPidFilter->pfUninitPrivate = CfaTsUninitAudPrivate;

				if ((!prCfaTs->fgAudSwDec) &&
					((DATA_SOURCE_WFD != prCfaTs->eDataSource) ||
					((DATA_SOURCE_WFD == prCfaTs->eDataSource)
#if CFG_SUPPORT_HDCP
					&& !prCfaTs->fgHDCP
#endif
				   ))) {
					prPidFilter->pfTsPktCb = CfaTsAudPesCb;
				} else if (prCfaTs->fgAudSwDec
#if CFG_SUPPORT_HDCP
					|| ((DATA_SOURCE_WFD == prCfaTs->eDataSource) && prCfaTs->fgHDCP)
#endif
					) {
					prPidFilter->pfTsPktCb = CfaTsBufAudPesCb;
				} else
				{
					prPidFilter->pfTsPktCb = NULL;
				}
			} else {
				prPidFilter = prCfaTs->Pids[u4Pid];
			}

			/*set private data*/
			if (NULL == prPidFilter->pvPrivate) {
				MRESULT mrRet = prPidFilter->pfInitPrivate(&(prPidFilter->pvPrivate), prCfaTs);

				if (RET_DMX_OK != mrRet) {
					DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
						TEXT("[CFA TS]CfaTsCfgAud(): init audio private error!\n"));
					MM_RETURN(mrRet);
				}
			}
			prAPriv = (CfaTsAudPrivate_T *)prPidFilter->pvPrivate;
			if (NULL == prAPriv)
				MM_RETURN(RET_DMX_NO_MEM);

			prAPriv->pvSptHdl = prCfaTs->pvSptHdl;
			prAPriv->prCfaTs = prCfaTs;
			prAPriv->u2PcrPid = prAudInf->u2PcrPid;

			/*get audio info*/
			prAPriv->eACodec = eMapAudCodec(prAudInf->eCodec);
			DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
                TEXT("[CFA TS]CfaTsCfgAud(): audio eACodec: %d\n"), prAPriv->eACodec);
			DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsCfgAud(): prPidFilter->u4PID: %d\n"), prPidFilter->u4PID);
		}

	}

	prCfaTs->u4CurAudPID = prCfaTsCfgInf->arAStreamInfo[0].u2Pid;

	MM_RETURN(RET_DMX_OK);
}

static u32 CfaTsDelAudFilter(CfaTsInst *prCfaTs, CfaTsConfigInfo_T *prCfaTsCfgInf)
{
	u32 i = 0;
	u16 u2Pid = 0;
	u16 u2PcrPid = 0;

	for (i = 0; i < prCfaTsCfgInf->u4DelAStreamNb; i++) {
		u2Pid = prCfaTsCfgInf->arDelAStreamInfo[i].u2Pid;
		u2PcrPid = prCfaTsCfgInf->arDelAStreamInfo[i].u2PcrPid;
		DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
            TEXT("[CFA TS]CfaTsDelAudFilter(): PID: 0x%x! \r\n"), (u32)u2Pid);
		if (NULL != prCfaTs->Pids[u2Pid]) {
			if (NULL != prCfaTs->Pids[u2Pid]->pvPrivate) {
				prCfaTs->Pids[u2Pid]->pfUninitPrivate(prCfaTs->Pids[u2Pid]->pvPrivate);
				prCfaTs->Pids[u2Pid]->pvPrivate = NULL;
			}
			DMX_FreeMemory(prCfaTs->Pids[u2Pid]);

			prCfaTs->Pids[u2Pid] = NULL;
		}
	}

	prCfaTs->u4CurAudPID = CFA_TS_INVALID_PID;

	MM_RETURN(RET_DMX_OK);
}


static MRESULT CfaTsCfgVid(CfaTsInst *prCfaTs, CfaTsConfigInfo_T *prCfaTsCfgInf)
{
	CfaTsVidPrivate_T *prVPriv = NULL;
	CfaTsPidFilter_T *prPidFilter = NULL;
	u32 u4Pid = (u32)prCfaTsCfgInf->rVStreamInfo.u2Pid;

	DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
        TEXT("[CFA TS]CfaTsCfgVid(): config video filter\n"));

	if (NULL == prCfaTs->Pids[u4Pid]) {
		DMX_NewMemory(sizeof(CfaTsPidFilter_T), prCfaTs->Pids[u4Pid]);
		if (NULL == prCfaTs->Pids[u4Pid])
			MM_RETURN(RET_DMX_NO_MEM);

		dmx_memset((void *)prCfaTs->Pids[u4Pid], 0, sizeof(CfaTsPidFilter_T));
		prPidFilter = prCfaTs->Pids[u4Pid];

		prPidFilter->u4PID = u4Pid;
		prPidFilter->ePidType = TS_PESPACKET;
		prPidFilter->fgCcOk = FALSE;
		prPidFilter->i4LastCc = -1;
		prPidFilter->u4CcErrCnt = 0;
		prPidFilter->fgUnitStart = FALSE;
		prPidFilter->fgTxDataFromUinitStart = FALSE;
		prPidFilter->pfInitPrivate = CfaTsInitVidPrivate;
		prPidFilter->pfUninitPrivate = CfaTsUninitVidPrivate;

		if ((DATA_SOURCE_WFD != prCfaTs->eDataSource) ||
			((DATA_SOURCE_WFD == prCfaTs->eDataSource)
#if CFG_SUPPORT_HDCP
			&& !prCfaTs->fgHDCP
#endif
			)) {
			if ((AVCODEC_ID_H264 == prCfaTsCfgInf->rVStreamInfo.eCodec)
				|| (AVCODEC_ID_MPEG1 == prCfaTsCfgInf->rVStreamInfo.eCodec)
				|| (AVCODEC_ID_MPEG2 == prCfaTsCfgInf->rVStreamInfo.eCodec)) {
				prPidFilter->pfTsPktCb = CfaTsVidPesCb;
			} else {
				prPidFilter->pfTsPktCb = CfaTsBufVidPesCb;
			}
		} else
#if CFG_SUPPORT_HDCP
		if ((DATA_SOURCE_WFD == prCfaTs->eDataSource) && prCfaTs->fgHDCP) {
			prPidFilter->pfTsPktCb = CfaTsBufVidPesCb;
		} else
#endif
		{
			prPidFilter->pfTsPktCb = NULL;
		}
	} else {
		prPidFilter = prCfaTs->Pids[u4Pid];
	}


	/*set private data*/
	if (NULL == prPidFilter->pvPrivate) {
		MRESULT mrRet = prPidFilter->pfInitPrivate(&(prPidFilter->pvPrivate), prCfaTs);

		if (RET_DMX_OK != mrRet) {
			DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
                TEXT("[CFA TS]CfaTsCfgVid(): init video private error!\n"));
			MM_RETURN(mrRet);
		}
	}
	prVPriv = (CfaTsVidPrivate_T *)(prPidFilter->pvPrivate);
	if (NULL == prVPriv)
		MM_RETURN(RET_DMX_NO_MEM);

	prVPriv->pvSptHdl = prCfaTs->pvSptHdl;
	prVPriv->prCfaTs = prCfaTs;

	/*get video info*/
	prVPriv->eVCodec = eMapVidCodec(prCfaTsCfgInf->rVStreamInfo.eCodec);
	prVPriv->u4Fps_n = prCfaTsCfgInf->rVStreamInfo.u4Fps_n;
	prVPriv->u4Fps_d = prCfaTsCfgInf->rVStreamInfo.u4Fps_d;
	prVPriv->u4BitRate = prCfaTsCfgInf->rVStreamInfo.u4BitRate;
	prVPriv->u2PcrPid = prCfaTsCfgInf->rVStreamInfo.u2PcrPid;

	prCfaTs->u4CurVidPID = u4Pid;

	DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
        TEXT("[CFA TS]CfaTsCfgVid(): video eVCodec: %d\n"), prVPriv->eVCodec);
	DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
        TEXT("[CFA TS]CfaTsCfgVid(): video filter u4PID: %d\n"), prPidFilter->u4PID);

	MM_RETURN(RET_DMX_OK);
}

static u32 CfaTsDelVidFilter(CfaTsInst *prCfaTs, CfaTsConfigInfo_T *prCfaTsCfgInf)
{
	u16 u2Pid = 0;
	u16 u2PcrPid = 0;

	u2Pid = prCfaTsCfgInf->rDelVStreamInfo.u2Pid;
	u2PcrPid = prCfaTsCfgInf->rDelVStreamInfo.u2PcrPid;
	DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
        TEXT("[CFA TS]CfaTsDelVidFilter(): PID: 0x%x! \r\n"), (u32)u2Pid);
	if (NULL != prCfaTs->Pids[u2Pid]) {
		if (NULL != prCfaTs->Pids[u2Pid]->pvPrivate) {
			prCfaTs->Pids[u2Pid]->pfUninitPrivate(prCfaTs->Pids[u2Pid]->pvPrivate);
			prCfaTs->Pids[u2Pid]->pvPrivate = NULL;
		}
		DMX_FreeMemory(prCfaTs->Pids[u2Pid]);

		prCfaTs->Pids[u2Pid] = NULL;
	}

	prCfaTs->u4CurVidPID = CFA_TS_INVALID_PID;

	MM_RETURN(RET_DMX_OK);
}

static void CfaTsCfgSectionChekTableId(SectionInfo_T *pSecInfo, CfaTsSectionInfo_T *pCfgSecInfo, bool *pfgTableIdExsit)
{
	if (pSecInfo->u2TableId == pCfgSecInfo->u2TableId) {
		if ((pSecInfo->u2TableId >= MIN_EIT_TABLE_ID)
			&& (pSecInfo->u2TableId <= MAX_EIT_TABLE_ID)) {
			if (pSecInfo->u2ServiceId == pCfgSecInfo->u2ServiceId)
				*pfgTableIdExsit = TRUE;
		} else {
			*pfgTableIdExsit = TRUE;
		}
	}
}


static MRESULT CfaTsCfgSection(CfaTsInst *prCfaTs, CfaTsConfigInfo_T *prCfaTsCfgInf)
{
	u32 i = 0;
	CfaTsSecPrivate_T *prSecPriv = NULL;
	CfaTsPidFilter_T *prPidFilter = NULL;
	u32 u4Pid = DMX_INVALID_UINT32;
	u32 u4TableId = DMX_INVALID_UINT32;

	for (i = 0; i < prCfaTsCfgInf->u4CfgSecInfoNb; i++) {
		u4Pid = (u32)prCfaTsCfgInf->arCfgSectionInfo[i].u2Pid;
		u4TableId = (u32)prCfaTsCfgInf->arCfgSectionInfo[i].u2TableId;

		DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
            TEXT("[CFA TS]CfaTsCfgSection(): config PID filter-->PID: 0x%x\n"), u4Pid);
		DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsCfgSection(): config PID filter-->TABLE ID: 0x%x\n"),
			prCfaTsCfgInf->arCfgSectionInfo[i].u2TableId);
		DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsCfgSection(): config PID filter-->SERVICE ID: 0x%x\n"),
			prCfaTsCfgInf->arCfgSectionInfo[i].u2ServiceId);

		if ((CFA_TS_PMT_TABLE_ID == u4TableId)
			&& ((u4Pid >= CFA_TS_PMT_MIN_PID) && (u4Pid <= CFA_TS_PMT_MAX_PID)))
			u4Pid = CFA_TS_PMT_MIN_PID;

		if (u4Pid >= CFA_TS_MAX_FILTER_IDEX) {
			DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsCfgSection(): config PID filter-->PID ERROR: 0x%x\n"), u4Pid);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}

		if (NULL != prCfaTs->Pids[u4Pid]) {
			u32 u4Tmp = 0;
			bool fgTableIdExsit = FALSE;

			prPidFilter = prCfaTs->Pids[u4Pid];
			prSecPriv = (CfaTsSecPrivate_T *)(prPidFilter->pvPrivate);

			if (prSecPriv->u2TableIdNum >= MAX_TABLEID_ONE_PID) {
				DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("[CFA TS]CfaTsCfgSection():")
					TEXT("prSecPriv->u2TableIdNum >= MAX_TABLEID_ONE_PID\n"));
				break;
			}

			for (u4Tmp = 0; u4Tmp < prSecPriv->u2TableIdNum; u4Tmp++) {
				CfaTsCfgSectionChekTableId(&(prSecPriv->arSecInfo[u4Tmp]),
					&(prCfaTsCfgInf->arCfgSectionInfo[i]), &fgTableIdExsit);
			}
			if (fgTableIdExsit) {
				/*RETAILMSG(1, (L"[CFA TS] PID filter-->TABLE ID: 0x%x exist\n",
				prCfaTsCfgInf->arCfgSectionInfo[i].u2TableId));*/
				DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsCfgSection(): PID filter-->TABLE ID: 0x%x exist\n"),
				prCfaTsCfgInf->arCfgSectionInfo[i].u2TableId);
				continue;
			}
			if (prSecPriv->u2TableIdNum == MAX_TABLEID_ONE_PID)
				MM_RETURN(RET_DMX_UNEXPECT);
			prSecPriv = (CfaTsSecPrivate_T *)(prPidFilter->pvPrivate);
			prSecPriv->arSecInfo[prSecPriv->u2TableIdNum].u2TableId
				= prCfaTsCfgInf->arCfgSectionInfo[i].u2TableId;
			prSecPriv->arSecInfo[prSecPriv->u2TableIdNum].u2ServiceId
				= prCfaTsCfgInf->arCfgSectionInfo[i].u2ServiceId;
			prSecPriv->u2TableIdNum++;
			continue;
		}
		DMX_NewMemory(sizeof(CfaTsPidFilter_T), prCfaTs->Pids[u4Pid]);
		if (NULL == prCfaTs->Pids[u4Pid])
			MM_RETURN(RET_DMX_NO_MEM);
		dmx_memset((void *)prCfaTs->Pids[u4Pid], 0, sizeof(CfaTsPidFilter_T));
		prPidFilter = prCfaTs->Pids[u4Pid];

		prPidFilter->u4PID				= u4Pid;
		prPidFilter->ePidType			= TS_SECTION;
		prPidFilter->fgCcOk				= FALSE;
		prPidFilter->i4LastCc			= -1;
		prPidFilter->u4CcErrCnt = 0;
		prPidFilter->fgUnitStart		= FALSE;
		prPidFilter->fgTxDataFromUinitStart		= FALSE;
		prPidFilter->pfInitPrivate		= CfaTsInitSectionPrivate;
		prPidFilter->pfUninitPrivate	= CfaTsUninitSectionPrivate;
		prPidFilter->pfTsPktCb			= CfaTsSectionCb;

		/*set private data*/
		if (NULL == prPidFilter->pvPrivate) {
			MRESULT mrRet = prPidFilter->pfInitPrivate(&(prPidFilter->pvPrivate), prCfaTs);

			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("[CFA TS]CfaTsCfgSection(): init section private error!\n"));
				MM_RETURN(mrRet);
			}
		}
		prSecPriv = (CfaTsSecPrivate_T *)(prPidFilter->pvPrivate);
		if (NULL == prSecPriv)
			MM_RETURN(RET_DMX_NO_MEM);

		prSecPriv->pvSptHdl = prCfaTs->pvSptHdl;
		prSecPriv->prCfaTs = prCfaTs;
		prSecPriv->arSecInfo[prSecPriv->u2TableIdNum].u2TableId
			= prCfaTsCfgInf->arCfgSectionInfo[i].u2TableId;
		prSecPriv->arSecInfo[prSecPriv->u2TableIdNum].u2ServiceId
			= prCfaTsCfgInf->arCfgSectionInfo[i].u2ServiceId;
		prSecPriv->u2TableIdNum++;
	}

#if 1
	if (NULL != prSecPriv) {
		u32 u4Tmp;

		DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsCfgSection(): exist PID filter-->PID: 0x%x\n"), u4Pid);
		for (u4Tmp = 0; u4Tmp < prSecPriv->u2TableIdNum; u4Tmp++)
			DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsCfgSection(): exist PID filter-->TABLE ID: 0x%x\n"),
			prSecPriv->arSecInfo[u4Tmp].u2TableId);
	}
#endif

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaTsCfgDelSecFilter(CfaTsInst *prCfaTs, CfaTsConfigInfo_T *prCfaTsCfgInf)
{
	u32 i = 0;
	u16 u2Pid = DMX_INVALID_UINT16;
	u16 u2Tid = DMX_INVALID_UINT16;
	CfaTsPidFilter_T *prPidFilter = NULL;

	for (i = 0; i < prCfaTsCfgInf->u4DelSecInfoNb; i++) {
		u2Pid = prCfaTsCfgInf->arDelSectionInfo[i].u2Pid;
		u2Tid = prCfaTsCfgInf->arDelSectionInfo[i].u2TableId;
		prPidFilter =  prCfaTs->Pids[u2Pid];

		DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsCfgDelSecFilter(): delete PID filter-->PID: 0x%x\n"),
			(u32)u2Pid);
		DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsCfgDelSecFilter(): delete PID filter-->TABLE ID: 0x%x\n"),
			(u32)u2Tid);

		if ((NULL != prPidFilter) && (TS_SECTION == prPidFilter->ePidType)) {
			CfaTsSecPrivate_T *prSecPriv = (CfaTsSecPrivate_T *)(prPidFilter->pvPrivate);
			u32 u4Tmp = 0;

			if (NULL == prSecPriv)
				continue;

			for (u4Tmp = 0; u4Tmp < prSecPriv->u2TableIdNum; u4Tmp++) {
				if (u2Tid == prSecPriv->arSecInfo[u4Tmp].u2TableId) {
					u32 j;

					for (j = (u32)1; (u4Tmp+j) < (u32)(prSecPriv->u2TableIdNum); j++)
						prSecPriv->arSecInfo[u4Tmp + j - (u32)1] = prSecPriv->arSecInfo[u4Tmp + j];
					prSecPriv->u2TableIdNum--;
				}
			}
			if (prSecPriv->u2TableIdNum > 0)
				continue;
			prPidFilter->pfUninitPrivate(prPidFilter->pvPrivate);
			prPidFilter->pvPrivate = NULL;
			DMX_FreeMemory(prPidFilter);

			prCfaTs->Pids[u2Pid] = NULL;
		}
	}

	MM_RETURN(RET_DMX_OK);
}


static MRESULT CfaTsCfgCc(CfaTsInst *prCfaTs, CfaTsConfigInfo_T *prCfaTsCfgInf)
{
	CfaTsCcPrivate_T *prCcPriv = NULL;
	CfaTsPidFilter_T *prPidFilter = NULL;
	u32 u4Pid = (u32)prCfaTsCfgInf->rCcStreamInfo.u2Pid;

	DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
        TEXT("[CFA TS]CfaTsCfgCc(): config cc filter\n"));

	if (NULL == prCfaTs->Pids[u4Pid]) {
		DMX_NewMemory(sizeof(CfaTsPidFilter_T), prCfaTs->Pids[u4Pid]);
		if (NULL == prCfaTs->Pids[u4Pid])
			MM_RETURN(RET_DMX_NO_MEM);
		dmx_memset((void *)prCfaTs->Pids[u4Pid], 0, sizeof(CfaTsPidFilter_T));
		prPidFilter = prCfaTs->Pids[u4Pid];

		prPidFilter->u4PID = u4Pid;
		prPidFilter->ePidType = TS_PESPACKET;
		prPidFilter->i4LastCc = -1;
		prPidFilter->u4CcErrCnt = 0;
		prPidFilter->fgUnitStart = FALSE;
		prPidFilter->fgTxDataFromUinitStart = FALSE;
		prPidFilter->pfInitPrivate = CfaTsInitCcPrivate;
		prPidFilter->pfUninitPrivate = CfaTsUninitCcPrivate;
		prPidFilter->pfTsPktCb = CfaTsCcPesCb;
	} else {
		prPidFilter = prCfaTs->Pids[u4Pid];
	}


	/*set private data*/
	if (NULL == prPidFilter->pvPrivate) {
		MRESULT mrRet = prPidFilter->pfInitPrivate(&(prPidFilter->pvPrivate), prCfaTs);

		if (RET_DMX_OK != mrRet) {
			DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
                TEXT("[CFA TS]CfaTsCfgCc(): init cc private error!\n"));
			MM_RETURN(mrRet);
		}
	}

	prCcPriv = (CfaTsCcPrivate_T *)(prPidFilter->pvPrivate);
	if (NULL == prCcPriv)
		MM_RETURN(RET_DMX_NO_MEM);

#if CFG_ALLOC_CC_MEM_WHEN_INIT
	prCcPriv->pucDataBuf = prCfaTs->pucCcPacketBuf;
	prCcPriv->u4BufLen = prCfaTs->u4CcPacketBufLen;
#endif

	prCcPriv->pvSptHdl = prCfaTs->pvSptHdl;
	prCcPriv->prCfaTs = prCfaTs;
	prCfaTs->u4CurCcPID = u4Pid;

	MM_RETURN(RET_DMX_OK);
}


static u32 CfaTsDelCcFilter(CfaTsInst *prCfaTs, CfaTsConfigInfo_T *prCfaTsCfgInf)
{
	u32 u2Pid = 0;

	u2Pid = prCfaTsCfgInf->rDelCcStreamInfo.u2Pid;

	DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
        TEXT("[CFA TS]CfaTsDelCcFilter(): PID: 0x%x! \r\n"), (u32)u2Pid);
	if (NULL != prCfaTs->Pids[u2Pid]) {
		if (NULL != prCfaTs->Pids[u2Pid]->pvPrivate) {
			prCfaTs->Pids[u2Pid]->pfUninitPrivate(prCfaTs->Pids[u2Pid]->pvPrivate);
			prCfaTs->Pids[u2Pid]->pvPrivate = NULL;
		}
		DMX_FreeMemory(prCfaTs->Pids[u2Pid]);

		prCfaTs->Pids[u2Pid] = NULL;
	}

	prCfaTs->u4CurCcPID = CFA_TS_INVALID_PID;

	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: CfaTsConfigure
*
* Description:
*	   splitter will ensure that it is only called in "off" or "paused" state.
*
* Inputs:
*	   [IN] handle of splitter
*	   [IN] configure paramter
*	   [IN] pointer to CfaTsInst
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaTsConfigure(void *pvSptHdl, void *pvParam, void *pvPrivData, bool fgIsUserMem)
{
	CfaTsInst *prCfaTs = NULL;
	CfaTsConfigInfo_T *prCfaTsCfgInf = NULL;
	CfaTsConfigInfo_T rCfaTsCfgInf;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvPrivData) || (NULL == pvParam))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	prCfaTs = (CfaTsInst *)pvPrivData;

	mm_memset(&rCfaTsCfgInf, 0, sizeof(CfaTsConfigInfo_T));
	prCfaTsCfgInf = &rCfaTsCfgInf;
	if (fgIsUserMem) {
		if (0 != mm_copy_from_user(prCfaTsCfgInf,
			pvParam, sizeof(CfaTsConfigInfo_T))) {
			DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT,
				TEXT("[CFA_TS] %s line %d failed in mm_copy_from_user\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
		}
	} else {
		mm_memcpy(prCfaTsCfgInf,
				pvParam, sizeof(CfaTsConfigInfo_T));
	}
#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaTs);
	MMATE_CHECK_STRUCT(prCfaTs->rRange);
	MMATE_CHECK_STRUCT(prCfaTs->rVidDeltaPesPtsInfo);
	MMATE_CHECK_STRUCT(prCfaTs->rAudDeltaPesPtsInfo);
	MMATE_CHECK_STRUCT(prCfaTs->rAudCmdQBakInfo);
#endif

	dmx_sema_lock(prCfaTs->sCfaTs, DMX_SEMA_OPTION_WAIT);

	/*config data source*/
	if (prCfaTsCfgInf->fgCfgDataSource) {
		prCfaTs->eDataSource = prCfaTsCfgInf->eDataSource;
		prCfaTs->u4TsPacketSize = prCfaTsCfgInf->u4TsPktSize;
		if (DATA_SOURCE_STREAM == prCfaTs->eDataSource) {
			DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsConfigure(): It is ISDBT file!\n"));
			prCfaTs->fgIsISDBT1Seg = TRUE;
		} else {
			DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsConfigure(): fgIsISDBT1Seg: %d!\n"),
				prCfaTsCfgInf->fgIsISDBT1Seg);
			prCfaTs->fgIsISDBT1Seg = prCfaTsCfgInf->fgIsISDBT1Seg;
		}
	}

#if CFG_SUPPORT_HDCP
		if (prCfaTsCfgInf->fgConfigHdcp) {
			prCfaTs->fgHDCP = prCfaTsCfgInf->fgHdcp;
			DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsConfigure():  fgHDCP: %d\n"),
				prCfaTs->fgHDCP);
		}
#endif
	/*initialize CC pid filter*/
	if (prCfaTsCfgInf->fgCfgCc) {
		mrRet = CfaTsCfgCc(prCfaTs, prCfaTsCfgInf);
		if (RET_DMX_OK != mrRet) {
			dmx_sema_unlock(prCfaTs->sCfaTs);
			if (!fgIsUserMem)
				DMX_FreeMemory(pvParam);
			MM_RETURN(mrRet);
		}
	}

	/*del cc  filter*/
	if (prCfaTsCfgInf->fgDelCcFilter)
		CfaTsDelCcFilter(prCfaTs, prCfaTsCfgInf);

	/*initialize video pid filter*/
	if ((prCfaTsCfgInf->fgCfgVid) &&
		(CFA_VID_UNKNOWN != eMapVidCodec(prCfaTsCfgInf->rVStreamInfo.eCodec))) {
		mrRet = CfaTsCfgVid(prCfaTs, prCfaTsCfgInf);
		if (RET_DMX_OK != mrRet) {
			dmx_sema_unlock(prCfaTs->sCfaTs);
			if (!fgIsUserMem)
				DMX_FreeMemory(pvParam);
			MM_RETURN(mrRet);
		}
	}

	/*del vid filter*/
	if (prCfaTsCfgInf->fgDelVidFilter)
		CfaTsDelVidFilter(prCfaTs, prCfaTsCfgInf);

	/*initialize audio pid filter*/
	if (prCfaTsCfgInf->fgCfgAud) {
		mrRet = CfaTsCfgAud(prCfaTs, prCfaTsCfgInf);
		if (RET_DMX_OK != mrRet) {
			dmx_sema_unlock(prCfaTs->sCfaTs);
			if (!fgIsUserMem)
				DMX_FreeMemory(pvParam);
			MM_RETURN(mrRet);
		}
	}

	/*del aud filter*/
	if (prCfaTsCfgInf->fgDelAudFilter)
		CfaTsDelAudFilter(prCfaTs, prCfaTsCfgInf);

	/*set current audio PID*/
	if ((prCfaTsCfgInf->fgSetCurAudPid)
		&& (prCfaTs->u4CurAudPID != prCfaTsCfgInf->u4CurAudPid)) {
		prCfaTs->u4CurAudPID = prCfaTsCfgInf->u4CurAudPid;
		prCfaTs->u4AudCmdQIndex = 0;
		prCfaTs->u8AudDataInBuf = 0;
		DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsConfigure(): Cur audio PID: 0x%x!\n"),
			prCfaTs->u4CurAudPID);
	}


	/*initialize section pid filter*/
	if (prCfaTsCfgInf->fgCfgSection) {
		mrRet = CfaTsCfgSection(prCfaTs, prCfaTsCfgInf);
		if (RET_DMX_OK != mrRet) {
			dmx_sema_unlock(prCfaTs->sCfaTs);
			if (!fgIsUserMem)
				DMX_FreeMemory(pvParam);
			MM_RETURN(mrRet);
		}
	}

	/*delete section filter*/
	if (prCfaTsCfgInf->fgDelSecFilter) {
		mrRet = CfaTsCfgDelSecFilter(prCfaTs, prCfaTsCfgInf);
		if (RET_DMX_OK != mrRet) {
			dmx_sema_unlock(prCfaTs->sCfaTs);
			if (!fgIsUserMem)
				DMX_FreeMemory(pvParam);
			MM_RETURN(mrRet);
		}
	}

	DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
		TEXT("[CFA TS]CfaTsConfigure(): CfaTsConfigure OK!\n"));
	dmx_sema_unlock(prCfaTs->sCfaTs);
	if (!fgIsUserMem)
		DMX_FreeMemory(pvParam);

	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: CfaTsTurnOn
*
* Description:
*	   TS CFA turns on file demuxing
*
* Inputs:
*
* Outputs:
*
* Returns: s32
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaTsTurnOn(void *pvSptHdl, void *pvPrivData)
{
	CfaTsInst *prCfaTs = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsTurnOn(): PARAM error!\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prCfaTs = (CfaTsInst *)pvPrivData;
#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaTs);
	MMATE_CHECK_STRUCT(prCfaTs->rRange);
	MMATE_CHECK_STRUCT(prCfaTs->rVidDeltaPesPtsInfo);
	MMATE_CHECK_STRUCT(prCfaTs->rAudDeltaPesPtsInfo);
	MMATE_CHECK_STRUCT(prCfaTs->rAudCmdQBakInfo);
#endif

	prCfaTs->fgAudSeek = TRUE;
	prCfaTs->fgVidSeek = TRUE;
	prCfaTs->u8Ca = prCfaTs->rRange.u8Sa;
	prCfaTs->u8ParseOffset = prCfaTs->rRange.u8Sa;
	prCfaTs->fgSyncPbbuf = FALSE;
	prCfaTs->u4VidCmdQIndex = 0;
	prCfaTs->u8VidDataInBuf = 0;
	prCfaTs->u4AudCmdQIndex = 0;
	prCfaTs->u8AudDataInBuf = 0;
	prCfaTs->u4PacketBufLen = 0;

	prCfaTs->u8LastRealVidPts = 0;
	prCfaTs->u8CurRealVidPts = 0;
	prCfaTs->u8DeltaVPts = 0;
	dmx_memset((void *)prCfaTs->au8DeltaVPtsRec, 0, sizeof(prCfaTs->au8DeltaVPtsRec));
	prCfaTs->u4DeltaVPtsRecIdx = 0;
	prCfaTs->u8LastVidPts = 0;
	dmx_memset((void *)prCfaTs->rVidDeltaPesPtsInfo.au8PtsRecord,
		0, CFA_TS_MAX_PTS_RECORD_LEN * sizeof(u64));
	prCfaTs->rVidDeltaPesPtsInfo.fgNeedGet = TRUE;
	prCfaTs->rVidDeltaPesPtsInfo.u4GetCnt = 0;
	prCfaTs->rVidDeltaPesPtsInfo.u8DeltaPesPts = 0;
	prCfaTs->rVidDeltaPesPtsInfo.eInfoType = VID_DELTA_PE_PTS_INFO;
	dmx_memset((void *)prCfaTs->au8Rec4CorrectVPts, 0, sizeof(prCfaTs->au8Rec4CorrectVPts));

	prCfaTs->u8PtsIncrement = 0;

	prCfaTs->u8LastPesVidPts = 0;
	prCfaTs->u8CurPesVidPts = 0;
	prCfaTs->eVPtsState = VPTS_OK;

	prCfaTs->u4PesVidPtsIncCnt = 0; /*pts increase count*/
	prCfaTs->u8OldCurPesVidPts = 0;
	prCfaTs->u4PesVidPtsDecCnt = 0; /*pts decrease count*/
	prCfaTs->u8OldLastPesVidPts = 0;
	prCfaTs->u8FirstAbnormalPesVidPts = 0;

	prCfaTs->u4LastAudTicCnt = 0;
	prCfaTs->u4CurAudTicCnt = 0;
	prCfaTs->u4LastVidTicCnt = 0;
	prCfaTs->u4CurVidTicCnt = 0;

	prCfaTs->u4NoAudTicCnt = 0;
	prCfaTs->fgNeedCheckAPtsDec = FALSE;
	prCfaTs->u4PesAudPtsDecCnt = 0;
	prCfaTs->u8OldLastPesAudPts = 0;
	prCfaTs->u8LastPesAudPts = 0;
	prCfaTs->u8CurPesAudPts = 0;
	prCfaTs->u4PesAudPtsIncCnt = 0;
	prCfaTs->fgNeedCheckAPtsInc = FALSE;
	prCfaTs->u8OldCurPesAudPts = 0;
	prCfaTs->u8LastAudPts = 0;
	prCfaTs->u8LastFinalAudPts = 0;
	prCfaTs->eAPtsState = APTS_OK;

	prCfaTs->fgPcrChange = FALSE;
	prCfaTs->fgNoSignal = FALSE;

	dmx_memset((void *)prCfaTs->rAudDeltaPesPtsInfo.au8PtsRecord,
		0, CFA_TS_MAX_PTS_RECORD_LEN * sizeof(u64));
	prCfaTs->rAudDeltaPesPtsInfo.fgNeedGet = TRUE;
	prCfaTs->rAudDeltaPesPtsInfo.u4GetCnt = 0;
	prCfaTs->rAudDeltaPesPtsInfo.u8DeltaPesPts = 0;
	prCfaTs->rAudDeltaPesPtsInfo.eInfoType = AUD_DELTA_PE_PTS_INFO;
	dmx_memset((void *)prCfaTs->au8Rec4CorrectAPts, 0, sizeof(prCfaTs->au8Rec4CorrectAPts));
	prCfaTs->u4AudPesPtsInconsecutiveCnt = 0;

	prCfaTs->fgHasRemnant = FALSE;

	prCfaTs->rAudCmdQBakInfo.fgBackup = FALSE;
	prCfaTs->rVidCmdQPtsInfo.u4EntryNb = 0;
	prCfaTs->rVidCmdQPtsInfo.u4UsedEntryCnt = 0;

	prCfaTs->fgFirstGetVidPts = TRUE;
	prCfaTs->fgFirstGetAudPts = TRUE;
	prCfaTs->u8VidBasePts = INVALID_TIMESTAMP;
	prCfaTs->u8AudBasePts = INVALID_TIMESTAMP;
	prCfaTs->rVidBufPts.eState = PTS_INVALID;

	if (DATA_SOURCE_FILE == prCfaTs->eDataSource)
		prCfaTs->u8PtsIncrement = CFA_TS_MS2TSPTS(prCfaTs->rRange.u8SeekTime);

	mrRet = CfaTsSyncPbbuf(pvSptHdl, prCfaTs, 1);/*prCfaTs->u4TsPacketSize);*/

	DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
	TEXT("[CFA TS]CfaTsTurnOn(): CfaTsTurnOn OK, u8Ca: %lld!\n"), prCfaTs->u8Ca);

	MM_RETURN(mrRet);
}


/*-----------------------------------------------------------------------------
* Name: CfaTsSetRange
*
* Description:
*	   TS CFA sets demuxing range
*
*
* Inputs:
*	   [IN] handle of splitter
*	   [IN] pointer to CfaTsRange
*	   [IN] pointer to CfaTsInst
*
* Outputs:
*
* Returns: s32
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaTsSetRange(void *pvSptHdl, void *pvRange, void *pvPrivData, bool fgIsUserMem)
{
	CfaTsInst *prCfaTs = NULL;

	if ((NULL == pvPrivData) || (NULL == pvRange))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	prCfaTs = (CfaTsInst *)pvPrivData;

#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaTs);
	MMATE_CHECK_STRUCT(prCfaTs->rRange);
	dmx_memcpy(&prCfaTs->rRange.u8Sa, &(((CfaTsRange_T *)pvRange)->u8Sa),
		sizeof(prCfaTs->rRange) - 2 * sizeof(u32));
	MMATE_INIT_STRUCT(prCfaTs->rRange);
	MMATE_CHECK_POINTER(prCfaTs);
	MMATE_CHECK_STRUCT(prCfaTs->rRange);
	MMATE_CHECK_STRUCT(prCfaTs->rVidDeltaPesPtsInfo);
	MMATE_CHECK_STRUCT(prCfaTs->rAudDeltaPesPtsInfo);
	MMATE_CHECK_STRUCT(prCfaTs->rAudCmdQBakInfo);
#else
	dmx_memcpy(&prCfaTs->rRange, pvRange, sizeof(prCfaTs->rRange));
#endif
	DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
	TEXT("[CFA TS]CfaTsSetRange(): CfaTsSetRange OK!\n"));

	MM_RETURN(RET_DMX_OK);
}




/*-----------------------------------------------------------------------------
* Name: CfaTsUninit
*
* Description:
*	   Uninit CFA TS
*
* Inputs:
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaTsUninit(void *pvSptHdl, void *pvCfaPrivData)
{
	u32 i = 0;
	CfaTsInst *prCfaTs = NULL;
	CfaTsPidFilter_T **ppPidFiler = NULL;

	if (NULL == pvCfaPrivData) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsUninit(): PARAM error!\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	
	prCfaTs = (CfaTsInst *)pvCfaPrivData;
	ppPidFiler = prCfaTs->Pids;

#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaTs);
	MMATE_CHECK_STRUCT(prCfaTs->rRange);
	MMATE_CHECK_STRUCT(prCfaTs->rVidDeltaPesPtsInfo);
	MMATE_CHECK_STRUCT(prCfaTs->rAudDeltaPesPtsInfo);
	MMATE_CHECK_STRUCT(prCfaTs->rAudCmdQBakInfo);
#endif

	/*release filter*/
	for (i = 0; i < CFA_TS_MAX_FILTER_IDEX; i++) {
		if (NULL != ppPidFiler[i]) {
			if (NULL != ppPidFiler[i]->pvPrivate) {
				ppPidFiler[i]->pfUninitPrivate(ppPidFiler[i]->pvPrivate);
				ppPidFiler[i]->pvPrivate = NULL;
			}
			DMX_FreeMemory(ppPidFiler[i]);

			ppPidFiler[i] = NULL;
		}
	}

	if (NULL != prCfaTs->rVidInf.parCmdQTxEntry) {
		DMX_FreeMemory(prCfaTs->rVidInf.parCmdQTxEntry);
		prCfaTs->rVidInf.parCmdQTxEntry = NULL;
	}
	if (NULL != prCfaTs->rAudInf.parCmdQTxEntry) {
		DMX_FreeMemory(prCfaTs->rAudInf.parCmdQTxEntry);
		prCfaTs->rAudInf.parCmdQTxEntry = NULL;
	}
	if (NULL != prCfaTs->rAudCmdQBakInfo.rAudInf.parCmdQTxEntry) {
		DMX_FreeMemory(prCfaTs->rAudCmdQBakInfo.rAudInf.parCmdQTxEntry);
		prCfaTs->rAudCmdQBakInfo.rAudInf.parCmdQTxEntry = NULL;
	}

	if (NULL != prCfaTs->pucPacketBuf) {
		DMX_FreeHwMemory((void *)prCfaTs->pucPacketBuf);
		prCfaTs->pucPacketBuf = NULL;
	}
#if CFG_ALLOC_CC_MEM_WHEN_INIT
	if (NULL != prCfaTs->pucCcPacketBuf) {
		DMX_FreeHwMemory((void *)prCfaTs->pucCcPacketBuf);
		prCfaTs->pucCcPacketBuf = NULL;
	}

#endif

	dmx_sema_delete(prCfaTs->sCfaTs);

	DMX_FreeMemory(prCfaTs);
	prCfaTs = NULL;

	DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
        TEXT("[CFA TS]CfaTsUninit(): CfaTsUninit OK!\n"));

	MM_RETURN(RET_DMX_OK);
}

static void GetVideoDeltaPts(CfaTsInst *prCfaTs, bool *pfgFrameRateChange,
	PicInfo *prPicInfo)
{
	prCfaTs->u8LastRealVidPts = prCfaTs->u8CurRealVidPts;
	prCfaTs->u8CurRealVidPts = prPicInfo->u8Pts;

	if ((prCfaTs->u8CurRealVidPts > 0) &&
		(prCfaTs->u8LastRealVidPts > 0) &&
		(prCfaTs->u8CurRealVidPts > prCfaTs->u8LastRealVidPts) &&
		(prCfaTs->u4VidFrameCnt > 0)) {
		prCfaTs->u4DeltaVPtsRecIdx
			= prCfaTs->u4DeltaVPtsRecIdx % CFA_TS_MAX_DELTA_PTS_REC_LEN;
		prCfaTs->au8DeltaVPtsRec[prCfaTs->u4DeltaVPtsRecIdx]
			= (prCfaTs->u8CurRealVidPts - prCfaTs->u8LastRealVidPts)
			/ prCfaTs->u4VidFrameCnt;

		if (prCfaTs->u8DeltaVPts
			!= prCfaTs->au8DeltaVPtsRec[prCfaTs->u4DeltaVPtsRecIdx]) {
			u32 i = 0;

			for (i = 0; i < CFA_TS_MAX_DELTA_PTS_REC_LEN - 1; i++) {
				if (prCfaTs->au8DeltaVPtsRec[i]
					!= prCfaTs->au8DeltaVPtsRec[i + 1])
					break;
			}

			if (i == (CFA_TS_MAX_DELTA_PTS_REC_LEN - 1)) {
				*pfgFrameRateChange = TRUE;
				DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsFillAUInfo():")
				TEXT("Old DeltaVPts: %lld, new DeltaVPts: %lld\n"),
				prCfaTs->u8DeltaVPts, prCfaTs->au8DeltaVPtsRec[0]);
			}
		}

		prCfaTs->u4DeltaVPtsRecIdx += 1;
	}

	prCfaTs->u4VidFrameCnt = 0;
}


static void DumpAndFixAbnormPts(CfaTsInst *prCfaTs, u64 u8RefPts,
	PicInfo *prPicInfo, bool fgRefPtsFromStc)
{
	if (prPicInfo->u8Pts > prCfaTs->u8LastVidPts) {
		if ((prPicInfo->u8Pts - prCfaTs->u8LastVidPts) > PTS_INTERVAL) {
			DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsFillAUInfo(): ----------------->")
				TEXT("abnormal big VPTS: 0x%08x%08x,")
				TEXT("u8LastVidPts: 0x%08x%08x\n"),
				(u32)(prPicInfo->u8Pts >> 32),
				(u32)prPicInfo->u8Pts,
				(u32)(prCfaTs->u8LastVidPts >> 32),
				(u32)(prCfaTs->u8LastVidPts));

			if (prPicInfo->u8Pts > (u8RefPts + MAX_GAP_PTS_BETWEEN_AV)) {
				DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("[CFA TS]CfaTsFillAUInfo():")
					TEXT("ajust video pts, u8RefPts:")
					TEXT("0x%08x%08x, fgRefPtsFromStc: %d\n"),
					(u32)(u8RefPts >> 32),
					(u32)u8RefPts,
					(u32)fgRefPtsFromStc);
				prPicInfo->u8Pts = u8RefPts;
			}
		}
	} else {
		DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsFillAUInfo(): ----------------->")
			TEXT("abnormal small VPTS: 0x%08x%08x,")
			TEXT("u8LastVidPts: 0x%08x%08x\n"),
			(u32)(prPicInfo->u8Pts >> 32),
			(u32)prPicInfo->u8Pts,
			(u32)(prCfaTs->u8LastVidPts >> 32),
			(u32)(prCfaTs->u8LastVidPts));
	}
}

/*-----------------------------------------------------------------------------
* Name: CfaTsFillAUInfo
*
* Description:
*	   TS CFA sets AU table information
*
* Inputs:
*	   [IN] handle of Splitter
*	   [IN/OUT] AU info
*	   [IN] pointer to CfaTsInst
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaTsFillAUInfo(void *pvSptHdl, void *pvAUInfo, void *pvAUExtInfo, void *pvPrivData)
{
	CfaTsInst *prCfaTs = NULL;
	u64 u8PtsBase = 0;
	u64 u8RefPts = 0;
	bool fgRefPtsFromStc = FALSE; /*for debug only*/

	if ((NULL == pvAUInfo) || (NULL == pvPrivData)){
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsFillAUInfo(): PARAM error!\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	prCfaTs = (CfaTsInst *)pvPrivData;
#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaTs);
	MMATE_CHECK_STRUCT(prCfaTs->rRange);
	MMATE_CHECK_STRUCT(prCfaTs->rVidDeltaPesPtsInfo);
	MMATE_CHECK_STRUCT(prCfaTs->rAudDeltaPesPtsInfo);
	MMATE_CHECK_STRUCT(prCfaTs->rAudCmdQBakInfo);
#endif

	if (-1 != prCfaTs->u8LastFinalAudPts) {
		u8RefPts = prCfaTs->u8LastFinalAudPts;
		fgRefPtsFromStc = FALSE;
	} else {
		STC_HalGetTime(0, &u8RefPts);
		fgRefPtsFromStc = TRUE;
	}


	switch (prCfaTs->eCurPrsStm) {
	case CFA_TS_PRS_STRM_TYPE_V:
	{
		u32 u4Temp = 0;
		bool fgFrameRateChange = FALSE;
		PicInfo *prPicInfo = NULL;
		CmdQPtsInfo_t *pCmdQPtsInfo = NULL;

		CfaTsVidPrivate_T *prVidPriv
			= (CfaTsVidPrivate_T *)prCfaTs->Pids[prCfaTs->u4CurVidPID]->pvPrivate;


		if ((INVALID_TIMESTAMP != prCfaTs->u8AudBasePts)
			&& (prCfaTs->u4NoAudTicCnt <= NO_AUD_TICK_COUNT))
			u8PtsBase = prCfaTs->u8AudBasePts;
		else
			u8PtsBase = prCfaTs->u8VidBasePts;

		prPicInfo = &(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo);
		prPicInfo->eDiscType = DT_DATADISC;

#if CFG_SUPPORT_HDCP
		if (((DATA_SOURCE_WFD == prCfaTs->eDataSource) && prCfaTs->fgHDCP) ||
			((CFA_VID_H264 != prVidPriv->eVCodec)
			&& (CFA_VID_MPEG2 != prVidPriv->eVCodec)))
#else
		if ((CFA_VID_H264 != prVidPriv->eVCodec)
			&& (CFA_VID_MPEG2 != prVidPriv->eVCodec))
#endif
		{
			prPicInfo->u8Pts = INVALID_TIMESTAMP;
			if (PTS_VALID == prCfaTs->rVidBufPts.eState) {
				prPicInfo->u8Pts = prCfaTs->rVidBufPts.u8Pts;
				prCfaTs->rVidBufPts.eState = PTS_INVALID;

				/*compute correct pts*/
				if (prPicInfo->u8Pts != INVALID_TIMESTAMP) {
					if (prPicInfo->u8Pts >= u8PtsBase) {
						prPicInfo->u8Pts
							= prPicInfo->u8Pts - u8PtsBase + prCfaTs->u8PtsIncrement;
					} else {
						DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
						TEXT("[CFA TS]CfaTsFillAUInfo():")
						TEXT("prCfaTs->rVidBufPts.u8Pts < u8PtsBase\n"));
						prPicInfo->u8Pts = INVALID_TIMESTAMP;
					}
				}
			}
		} else {
			/*get raw pts*/
			prPicInfo->u8Pts = INVALID_TIMESTAMP;

		#ifdef BSP_WIFI_WFD
			if (DATA_SOURCE_WFD == prCfaTs->eDataSource) {
				pCmdQPtsInfo = &prCfaTs->rVidCmdQPtsInfoBak;
				for (u4Temp = 0; u4Temp < pCmdQPtsInfo->u4EntryNb; u4Temp++) {
					if ((pCmdQPtsInfo->aruEntryInfo[u4Temp].u8FileOffset
						<= prPicInfo->u8Offset)
						&& (PTS_VALID == pCmdQPtsInfo->aruEntryInfo[u4Temp].eState)) {
						/*DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
						    TEXT("[CFA TS]CfaTsFillAUInfo():
							pCmdQPtsInfo back u8Pts:
							0x%08x%08x,u8FileOffset: 0x%08x%08x\n"),*/
						/*	(u32)(pCmdQPtsInfo->aruEntryInfo[u4Temp].u8Pts >> 32),
							(u32)pCmdQPtsInfo->aruEntryInfo[u4Temp].u8Pts,*/
						/*	(u32)(pCmdQPtsInfo->aruEntryInfo[u4Temp].u8FileOffset >> 32),
							(u32)pCmdQPtsInfo->aruEntryInfo[u4Temp].u8FileOffset);*/


						prPicInfo->u8Pts = pCmdQPtsInfo->aruEntryInfo[u4Temp].u8Pts;

						pCmdQPtsInfo->u4UsedEntryCnt += 1;
						pCmdQPtsInfo->aruEntryInfo[u4Temp].eState = PTS_INVALID;
						pCmdQPtsInfo->aruEntryInfo[u4Temp].u8FileOffset = 0;
					}
				}

				pCmdQPtsInfo = &prCfaTs->rVidCmdQPtsInfo;

				for (u4Temp = 0; u4Temp < pCmdQPtsInfo->u4EntryNb; u4Temp++) {
					if ((pCmdQPtsInfo->aruEntryInfo[u4Temp].u8FileOffset <= prPicInfo->u8Offset)
						&& (PTS_VALID == pCmdQPtsInfo->aruEntryInfo[u4Temp].eState)) {
						/* DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
							TEXT("[CFA TS]CfaTsFillAUInfo():
							pCmdQPtsInfo u8Pts: 0x%08x%08x,
							u8FileOffset: 0x%08x%08x\n"),*/
						/* (u32)(pCmdQPtsInfo->aruEntryInfo[u4Temp].u8Pts >> 32),
							(u32)pCmdQPtsInfo->aruEntryInfo[u4Temp].u8Pts,*/
						/*(u32)(pCmdQPtsInfo->aruEntryInfo[u4Temp].u8FileOffset >> 32),
							(u32)pCmdQPtsInfo->aruEntryInfo[u4Temp].u8FileOffset);*/
						prPicInfo->u8Pts = pCmdQPtsInfo->aruEntryInfo[u4Temp].u8Pts;
						pCmdQPtsInfo->u4UsedEntryCnt += 1;
						pCmdQPtsInfo->aruEntryInfo[u4Temp].eState = PTS_INVALID;
						pCmdQPtsInfo->aruEntryInfo[u4Temp].u8FileOffset = 0;
					}
				}
				prCfaTs->fgFillAu = TRUE;

				/*DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsFillAUInfo():
					prPicInfo u8Pts: 0x%08x%08x,u8Offset: 0x%08x%08x\n"),*/
				/*(u32)(prPicInfo->u8Pts >> 32), (u32)prPicInfo->u8Pts,*/
				/*(u32)(prPicInfo->u8Offset >> 32), (u32)prPicInfo->u8Offset);*/
			} else
		#endif
			{
				if ((prCfaTs->rVidCmdQPtsInfoBak.u4EntryNb > 0)
					&& (prCfaTs->rVidCmdQPtsInfoBak.u4UsedEntryCnt < prCfaTs->rVidCmdQPtsInfoBak.u4EntryNb)) {
					pCmdQPtsInfo = &prCfaTs->rVidCmdQPtsInfoBak;
				}
				else {
					pCmdQPtsInfo = &prCfaTs->rVidCmdQPtsInfo;
				}

				for (u4Temp = 0; u4Temp < pCmdQPtsInfo->u4EntryNb; u4Temp++) {
					if ((pCmdQPtsInfo->aruEntryInfo[u4Temp].u8FileOffset
						<= prPicInfo->u8Offset)
						&& (PTS_VALID == pCmdQPtsInfo->aruEntryInfo[u4Temp].eState)) {
						prPicInfo->u8Pts = pCmdQPtsInfo->aruEntryInfo[u4Temp].u8Pts;
						pCmdQPtsInfo->u4UsedEntryCnt += 1;
						pCmdQPtsInfo->aruEntryInfo[u4Temp].eState = PTS_INVALID;
						pCmdQPtsInfo->aruEntryInfo[u4Temp].u8FileOffset = 0;
					}
				}
			}

			/* Get delta video PTS (frame duration)*/
			prCfaTs->u4VidFrameCnt += 1;
			if (-1 != prPicInfo->u8Pts)
				GetVideoDeltaPts(prCfaTs, &fgFrameRateChange, prPicInfo);

			if ((0 == prCfaTs->u8DeltaVPts) || fgFrameRateChange) {
				prCfaTs->u8DeltaVPts = prCfaTs->au8DeltaVPtsRec[0];
				fgFrameRateChange = FALSE;

				if ((prCfaTs->u8DeltaVPts > (4 * CFA_TS_DEFUALT_VID_FRM_DURATION))
					|| (0 == prCfaTs->u8DeltaVPts)) {
					DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("[CFA TS]CfaTsFillAUInfo(): abnormal prCfaTs->u8DeltaVPts: %lld\r\n"),
						(prCfaTs->u8DeltaVPts)*(1000/9));
					prCfaTs->u8DeltaVPts = CFA_TS_DEFUALT_VID_FRM_DURATION;
					/*default 30fps*/
				}
			}

			/*compute correct pts*/
			if (prPicInfo->u8Pts != INVALID_TIMESTAMP) {
				if (prPicInfo->u8Pts >= u8PtsBase) {
					prPicInfo->u8Pts = prPicInfo->u8Pts - u8PtsBase + prCfaTs->u8PtsIncrement;
				} else {
					DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
						TEXT("[CFA TS]CfaTsFillAUInfo(): prPicInfo->u8Pts < u8PtsBase \r\n"));
					prPicInfo->u8Pts = INVALID_TIMESTAMP;
				}
			}

			if (prCfaTs->fgIsISDBT1Seg) {
				/* FIX_PTS_BY_CFA*/
				if ((-1 != prCfaTs->u8LastVidPts) && (prCfaTs->u8DeltaVPts > 0)
					&& (prPicInfo->u8Pts == INVALID_TIMESTAMP))
					prPicInfo->u8Pts = prCfaTs->u8LastVidPts + prCfaTs->u8DeltaVPts;

				/*dump and fix abnorm pts*/
				if ((-1 != prPicInfo->u8Pts)
					&& (-1 != prCfaTs->u8LastVidPts)
					&& (prPicInfo->u8Pts > 0)
					&& (0 != prCfaTs->u8LastVidPts))
					DumpAndFixAbnormPts(prCfaTs, u8RefPts, prPicInfo, fgRefPtsFromStc);
			}
		}

		prCfaTs->u8LastVidPts = prPicInfo->u8Pts;

		break;
	}

	case CFA_TS_PRS_STRM_TYPE_A:
	{
		AudInfo *prAudInfo;

		/*CfaTsAudPrivate_T *prAudPriv
			= (CfaTsAudPrivate_T *)prCfaTs->Pids[prCfaTs->u4CurAudPID]->pvPrivate;*/

		u8PtsBase = prCfaTs->u8AudBasePts;

		prAudInfo = &(((AU_AUDIO *)pvAUInfo)->rAUInfo.rInfo);

		if (
#if CFG_SUPPORT_HDCP
			((DATA_SOURCE_WFD == prCfaTs->eDataSource) && prCfaTs->fgHDCP) ||
#endif
			(prCfaTs->fgAudSwDec)) {
			prAudInfo->u8Pts = INVALID_TIMESTAMP;
			if (PTS_VALID == prCfaTs->rAudBufPts.eState) {
				prAudInfo->u8Pts = prCfaTs->rAudBufPts.u8Pts;
				prCfaTs->rAudBufPts.eState = PTS_INVALID;
			}
		} else
		{
			if (prCfaTs->rAudCmdQPtsInfo.u4EntryNb > 0) {
				if (PTS_VALID == prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[0].eState) {
					prAudInfo->u8Pts = prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[0].u8Pts;
					prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[0].eState = PTS_INVALID;
				} else {
					if (PTS_VALID == prCfaTs->eACmdQFirstPtsEntryState){
                    }
					prAudInfo->u8Pts = INVALID_TIMESTAMP;
				}
			} else {
				DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("[CFA TS]CfaTsFillAUInfo(): this audio au has no pts, PTS = -1;\n"));
				prAudInfo->u8Pts = INVALID_TIMESTAMP;
			}

			if (prCfaTs->rAudCmdQBakInfo.fgBackup)
				CfaTsResumeBak(prCfaTs);
		}

		if (prAudInfo->u8Pts != INVALID_TIMESTAMP) {
			/*compute final audio PTS*/
			if (prAudInfo->u8Pts >= u8PtsBase) {
				prAudInfo->u8Pts = prAudInfo->u8Pts - u8PtsBase + prCfaTs->u8PtsIncrement;
				/*for the first time	   50		50	0 = 0*/
			} else {
				DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("[CFA TS]CfaTsFillAUInfo(): prAudInfo->u8Pts < u8PtsBase, PTS = -1;\n"));
				prAudInfo->u8Pts = INVALID_TIMESTAMP;
			}
		}

		/* Dump and fix abnormal pts*/
		if ((INVALID_TIMESTAMP != prAudInfo->u8Pts)
			&& (INVALID_TIMESTAMP != prCfaTs->u8LastFinalAudPts)
			&& (prAudInfo->u8Pts > 0)
			&& (prCfaTs->u8LastFinalAudPts > 0)) {
			if (prAudInfo->u8Pts > prCfaTs->u8LastFinalAudPts) {
				if	((prAudInfo->u8Pts - prCfaTs->u8LastFinalAudPts) > PTS_INTERVAL) {
					DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
						TEXT("[CFA TS]CfaTsFillAUInfo(): ----------------->")
						TEXT("abnormal big APTS: 0x%08x%08x, u8LastFinalAudPts: 0x%08x%08x\n"),
						(u32)(prAudInfo->u8Pts >> 32), (u32)prAudInfo->u8Pts
						, (u32)(prCfaTs->u8LastFinalAudPts >> 32)
						, (u32)(prCfaTs->u8LastFinalAudPts));
				}
			} else {
				DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("[CFA TS]CfaTsFillAUInfo(): ----------------->")
					TEXT("abnormal small APTS: 0x%08x%08x, u8LastFinalAudPts: 0x%08x%08x\n"),
					(u32)(prAudInfo->u8Pts >> 32), (u32)prAudInfo->u8Pts
					, (u32)(prCfaTs->u8LastFinalAudPts >> 32)
					, (u32)(prCfaTs->u8LastFinalAudPts));
			}
		}
		prCfaTs->u8LastFinalAudPts = prAudInfo->u8Pts;

/*		RETAILMSG(1, (L"[CFA TS]CfaTsFillAUInfo(): APTS: 0x%08x%08x,
		u8PtsBase: 0x%08x%08x, u8PtsIncrement: 0x%08x%08x\n",
		(u32)(prAudInfo->u8Pts >> 32), (u32)(prAudInfo->u8Pts),
		(u32)(u8PtsBase >> 32), (u32)(u8PtsBase),
		(u32)(prCfaTs->u8PtsIncrement >> 32), (u32)(prCfaTs->u8PtsIncrement)));
*/
		break;
	}

	case CFA_TS_PRS_STRM_TYPE_SP:
	{
		SPicInfo *pSPInfo = &((AU_SP *)pvAUInfo)->rAUInfo.rInfo;

		CfaTsCcPrivate_T *prCcPriv
			= (CfaTsCcPrivate_T *)prCfaTs->Pids[prCfaTs->u4CurCcPID]->pvPrivate;
		u64 u8RefPts = 0;
		bool fgRefPtsFromStc = FALSE; /*for debug only*/

		pSPInfo->u8StartPts = prCcPriv->rPesHeader.u8Pts;

		/*get base pts*/
		if (prCfaTs->u8AudBasePts > 0)
			u8PtsBase = prCfaTs->u8AudBasePts;
		else
			u8PtsBase = prCfaTs->u8VidBasePts;

		/*compute pts*/
		if (INVALID_TIMESTAMP != pSPInfo->u8StartPts) {
			if (pSPInfo->u8StartPts >= u8PtsBase) {
				pSPInfo->u8StartPts
					= pSPInfo->u8StartPts - u8PtsBase + prCfaTs->u8PtsIncrement;
			} else
				pSPInfo->u8StartPts = 0;

		} else
			pSPInfo->u8StartPts = 0;

		/*check if the pts is too large*/
		if (INVALID_TIMESTAMP != prCfaTs->u8LastFinalAudPts) {
			u8RefPts = prCfaTs->u8LastFinalAudPts;
			fgRefPtsFromStc = FALSE;
		} else {
			STC_HalGetTime(0, &u8RefPts);
			fgRefPtsFromStc = TRUE;
		}

		if ((pSPInfo->u8StartPts > (u8RefPts + PTS_INTERVAL))
			|| (u8RefPts > (pSPInfo->u8StartPts + PTS_INTERVAL))) {
			DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsFillAUInfo(): abnormal cc pts,")
				TEXT("pSPInfo->u8StartPts: 0x%08x%08x, u8RefPts: 0x%08x%08x\n"),
				(u32)(pSPInfo->u8StartPts >> 32), (u32)pSPInfo->u8StartPts,
				(u32)(u8RefPts >> 32), (u32)u8RefPts);

			if (INVALID_TIMESTAMP != prCfaTs->u8LastVidPts)
				pSPInfo->u8StartPts = prCfaTs->u8LastVidPts;
			else
				pSPInfo->u8StartPts = u8RefPts;

			if (fgRefPtsFromStc) {
				DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsFillAUInfo(): u8RefPts come from stc\n"));
			} else {
				DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("[CFA TS]CfaTsFillAUInfo(): u8RefPts come from last audio pts\n"));
			}
		}
/*
		RETAILMSG(1, (L"[CFA TS]CfaTsFillAUInfo():
			CPTS: 0x%08x%08x, u8PtsBase: 0x%08x%08x, u8PtsIncrement: 0x%08x%08x\n",
			(u32)(pSPInfo->u8StartPts >> 32), (u32)(pSPInfo->u8StartPts),
			(u32)(u8PtsBase >> 32), (u32)(u8PtsBase),
			(u32)(prCfaTs->u8PtsIncrement >> 32), (u32)(prCfaTs->u8PtsIncrement)));
*/
		 break;
	}

	default:
		DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
            TEXT("[CFA TS]CfaTsFillAUInfo(): Fill AU Error!\n"));
		break;
	}

	MM_RETURN(RET_DMX_OK);
}





/*-----------------------------------------------------------------------------
 * Name: CfaTsGetCurPos
 *
 * Description:
 *		TS CFA callback for when FMPC needs to know CFA's current position.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaTsGetCurPos(void *pvSptHdl, void *pvCurPos, void *pvPrivData)
{
	CfaTsInst *prCfaTs = NULL;
	u64 *pvu8 = NULL;

	if (NULL == pvCurPos)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
        TEXT("[CFA OGM] vCfaOgmGetCurPos\n"));
	if (NULL == pvPrivData)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	pvu8 = (u64 *)pvCurPos;

	prCfaTs = (CfaTsInst *)pvPrivData;
	*pvu8 = prCfaTs->u8Ca;

	MM_RETURN(RET_DMX_OK);
}

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
/* audio stream info */
typedef struct {
	__u16 u2Pid;
	__u16 u2PcrPid;
	AVCODECID_T eCodec;
} CfaTsAStreamInfo_T32;

/* video stream info */
typedef struct {
	__u16 u2Pid;
	__u16 u2PcrPid;
	__u32 u4Fps_n;
	__u32 u4Fps_d;
	__u32 u4BitRate;
	AVCODECID_T eCodec;
} CfaTsVStreamInfo_T32;

/* configure info */
typedef struct {
	bool fgCfgDataSource;
	E_TS_CHANNEL_TYPE_T eChannelType;
	TsUseType eUseType;
	TsDataFromType eDataFromType;
	DATA_SOURCE eDataSource;
	__u32 u4TsPktSize;	/* TS packet size */
	bool fgIsISDBT1Seg;
	bool fgCfgVid;
	CfaTsVStreamInfo_T32 rVStreamInfo;
	bool fgDelVidFilter;
	CfaTsVStreamInfo_T32 rDelVStreamInfo;
	bool fgCfgPcrPid;
	__u16 u2PcrPid;
	bool fgDelPcrPidFilter;
	__u16 u2DelPcrPid;
	bool fgCfgPcrBase;
	__u64 u8PcrBase;
	bool fgStartRecord;
	__u16 au2RecFileName[CFA_TS_FILE_NAME_MAX_LEN + 1];
	bool fgStopRecord;
	bool fgCfgCc;
	CfaTsCcInfo_T rCcStreamInfo;
	bool fgDelCcFilter;
	CfaTsCcInfo_T rDelCcStreamInfo;
	bool fgCfgAud;
	__u32 u4AStreamNb;	/* audio stream number */
	CfaTsAStreamInfo_T32 arAStreamInfo[CFA_TS_AUD_STREAM_NB_MAX];
	bool fgDelAudFilter;
	__u32 u4DelAStreamNb;	/* audio stream number */
	CfaTsAStreamInfo_T32 arDelAStreamInfo[CFA_TS_AUD_STREAM_NB_MAX];
	bool fgSetCurAudPid;	/* for audio track switch */
	__u32 u4CurAudPid;
	bool fgCfgSection;	/* configure section filter */
	__u32 u4CfgSecInfoNb;
	/* for configure section */
	CfaTsSectionInfo_T arCfgSectionInfo[CFA_TS_MAX_SECINFO_NB];
	bool fgDelSecFilter;	/* delete section filter */
	__u32 u4DelSecInfoNb;
	/* for delete section filter */
	CfaTsSectionInfo_T arDelSectionInfo[CFA_TS_MAX_SECINFO_NB];
	bool fgConfigHdcp;
	bool fgHdcp;
	bool fgAudSwDec;
} CfaTsConfigInfo_T32;

typedef struct {
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKStart;
#endif	/* 
 */
	__u64 u8Sa;	/* start address of file */
	__u64 u8Ea;	/* end address of file */
	bool fgIsSeek;
	__u64 u8SeekTime;	/* unit is ms */
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKEnd;
#endif	/* 
 */
} CfaTsRange_T32;
static long CfaTsCompatAStmInfo(CfaTsAStreamInfo_T __user *usr_ptr,CfaTsAStreamInfo_T32 __user *usr_ptr32)
{
	if (copy_from_user(&(usr_ptr->u2Pid), &(usr_ptr32->u2Pid), sizeof(__u16)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u2PcrPid), &(usr_ptr32->u2PcrPid), sizeof(__u16)))
			return -EFAULT;
	if (copy_from_user(&(usr_ptr->eCodec), &(usr_ptr32->eCodec), sizeof(AVCODECID_T)))
			return -EFAULT;

	return 0;
}
static long CfaTsCompatVStmInfo(CfaTsVStreamInfo_T __user *usr_ptr,CfaTsVStreamInfo_T32 __user *usr_ptr32)
{
	if (copy_from_user(&(usr_ptr->u2Pid), &(usr_ptr32->u2Pid), sizeof(__u16)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u2PcrPid), &(usr_ptr32->u2PcrPid), sizeof(__u16)))
			return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4Fps_n), &(usr_ptr32->u4Fps_n), sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4Fps_d), &(usr_ptr32->u4Fps_d), sizeof(__u32)))
			return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4BitRate), &(usr_ptr32->u4BitRate), sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->eCodec), &(usr_ptr32->eCodec), sizeof(AVCODECID_T)))
			return -EFAULT;

	return 0;
}

static long CfaTsCompatConfig(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaTsConfigInfo_T __user *usr_ptr = NULL;
	CfaTsConfigInfo_T32 __user *usr_ptr32 = (CfaTsConfigInfo_T32 __user *)prInfo->usr_ptr32;
	long ret = 0;
	int index = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaTsConfigInfo_T32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	//usr_ptr = (CfaTsConfigInfo_T *)compat_alloc_user_space(sizeof(CfaTsConfigInfo_T));
	DMX_NewMemory(sizeof(CfaTsConfigInfo_T),usr_ptr);

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaTsConfigInfo_T));
	if (copy_from_user(&(usr_ptr->fgCfgDataSource), &(usr_ptr32->fgCfgDataSource), sizeof(bool))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->eChannelType), &(usr_ptr32->eChannelType), sizeof(E_TS_CHANNEL_TYPE_T))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT; 
	}
	if (copy_from_user(&(usr_ptr->eUseType), &(usr_ptr32->eUseType), sizeof(TsUseType))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->eDataFromType), &(usr_ptr32->eDataFromType), sizeof(TsDataFromType))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->eDataSource), &(usr_ptr32->eDataSource), sizeof(DATA_SOURCE))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->u4TsPktSize), &(usr_ptr32->u4TsPktSize), sizeof(__u32))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT; 
	}
	if (copy_from_user(&(usr_ptr->fgIsISDBT1Seg), &(usr_ptr32->fgIsISDBT1Seg), sizeof(bool))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT; 
	}
	if (copy_from_user(&(usr_ptr->fgCfgVid), &(usr_ptr32->fgCfgVid), sizeof(bool))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	
	ret = CfaTsCompatVStmInfo(&(usr_ptr->rVStreamInfo),&(usr_ptr32->rVStreamInfo));
	
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaTsCompatVStmInfo.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(usr_ptr);
		return ret;
	}

	if (copy_from_user(&(usr_ptr->fgDelVidFilter), &(usr_ptr32->fgDelVidFilter), sizeof(bool))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}

	ret = CfaTsCompatVStmInfo(&(usr_ptr->rDelVStreamInfo),&(usr_ptr32->rDelVStreamInfo));
	
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaTsCompatVStmInfo.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(usr_ptr);
		return ret;
	}

	if (copy_from_user(&(usr_ptr->fgCfgPcrPid), &(usr_ptr32->fgCfgPcrPid), sizeof(bool))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->u2PcrPid), &(usr_ptr32->u2PcrPid), sizeof(__u16))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->fgDelPcrPidFilter), &(usr_ptr32->fgDelPcrPidFilter), sizeof(bool))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->u2DelPcrPid), &(usr_ptr32->u2DelPcrPid), sizeof(__u16))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->fgCfgPcrBase), &(usr_ptr32->fgCfgPcrBase), sizeof(bool))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->u8PcrBase), &(usr_ptr32->u8PcrBase), sizeof(__u64))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->fgStartRecord), &(usr_ptr32->fgStartRecord), sizeof(bool))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}

	if (copy_from_user(usr_ptr->au2RecFileName, 
			usr_ptr32->au2RecFileName, sizeof(__u16) * (CFA_TS_FILE_NAME_MAX_LEN + 1))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->fgStopRecord), &(usr_ptr32->fgStopRecord), sizeof(bool))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->fgCfgCc), &(usr_ptr32->fgCfgCc), sizeof(bool))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT; 
	}
	if (copy_from_user(&(usr_ptr->rCcStreamInfo), &(usr_ptr32->rCcStreamInfo), sizeof(CfaTsCcInfo_T))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->fgDelCcFilter), &(usr_ptr32->fgDelCcFilter), sizeof(bool))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rDelCcStreamInfo), &(usr_ptr32->rDelCcStreamInfo),sizeof(CfaTsCcInfo_T))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->fgCfgAud), &(usr_ptr32->fgCfgAud), sizeof(bool))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->u4AStreamNb), &(usr_ptr32->u4AStreamNb), sizeof(__u32))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	
	for(index = 0; index < CFA_TS_AUD_STREAM_NB_MAX; index++)
	{
		ret = CfaTsCompatAStmInfo(&(usr_ptr->arAStreamInfo[index]), &(usr_ptr32->arAStreamInfo[index]));
	
		if (0 != ret) {
			DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT,
				TEXT("%s line %d fail in CfaTsCompatAStmInfo.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(usr_ptr);
			return ret;
		}
	}
	if (copy_from_user(&(usr_ptr->fgDelAudFilter), &(usr_ptr32->fgDelAudFilter), sizeof(bool))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->u4DelAStreamNb), &(usr_ptr32->u4DelAStreamNb), sizeof(__u32))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	for(index = 0; index < CFA_TS_AUD_STREAM_NB_MAX; index++)
	{
		ret = CfaTsCompatAStmInfo(&(usr_ptr->arDelAStreamInfo[index]),&(usr_ptr32->arDelAStreamInfo[index]));
	
		if (0 != ret) {
			DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT,
				TEXT("%s line %d fail in CfaTsCompatAStmInfo.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(usr_ptr);
			return ret;
		}
	}
	if (copy_from_user(&(usr_ptr->fgSetCurAudPid), &(usr_ptr32->fgSetCurAudPid), sizeof(bool))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->u4CurAudPid), &(usr_ptr32->u4CurAudPid), sizeof(__u32))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->fgCfgSection), &(usr_ptr32->fgCfgSection), sizeof(bool))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->u4CfgSecInfoNb), &(usr_ptr32->u4CfgSecInfoNb), sizeof(__u32))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}

	if (copy_from_user(usr_ptr->arCfgSectionInfo,
			usr_ptr32->arCfgSectionInfo, sizeof(CfaTsSectionInfo_T) * CFA_TS_MAX_SECINFO_NB)) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->fgDelSecFilter), &(usr_ptr32->fgDelSecFilter), sizeof(bool))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->u4DelSecInfoNb), &(usr_ptr32->u4DelSecInfoNb), sizeof(__u32))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}

	if (copy_from_user(usr_ptr->arDelSectionInfo,
			usr_ptr32->arDelSectionInfo, sizeof(CfaTsSectionInfo_T) * CFA_TS_MAX_SECINFO_NB)) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->fgConfigHdcp), &(usr_ptr32->fgConfigHdcp), sizeof(bool))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT; 
	}
	if (copy_from_user(&(usr_ptr->fgHdcp), &(usr_ptr32->fgHdcp), sizeof(bool))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->fgAudSwDec), &(usr_ptr32->fgAudSwDec), sizeof(bool))) {
		DMX_FreeMemory(usr_ptr);
		return -EFAULT;
	}

	*pfgIsUserMem = FALSE;
	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaTsConfigInfo_T);

	return 0;

	
}
static long CfaTsCompatRange(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaTsRange_T __user *usr_ptr = NULL;
	CfaTsRange_T32 __user *usr_ptr32 = (CfaTsRange_T32 __user *)prInfo->usr_ptr32;
	long ret = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaTsRange_T32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	usr_ptr = (CfaTsRange_T *)compat_alloc_user_space(sizeof(CfaTsRange_T));

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaTsRange_T));

	#ifdef MM_ATE_CHECK
	if (copy_in_user(&(usr_ptr->u4MMATECHKStart), &(usr_ptr32->u4MMATECHKStart), sizeof(__u32)))
		return -EFAULT;
	#endif
	
	if (copy_in_user(&(usr_ptr->u8Sa), &(usr_ptr32->u8Sa), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8Ea), &(usr_ptr32->u8Ea), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->fgIsSeek), &(usr_ptr32->fgIsSeek), sizeof(bool)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8SeekTime), &(usr_ptr32->u8SeekTime), sizeof(__u64)))
		return -EFAULT;

	#ifdef MM_ATE_CHECK
	if (copy_in_user(&(usr_ptr->u4MMATECHKEnd), &(usr_ptr32->u4MMATECHKEnd), sizeof(__u32)))
		return -EFAULT;
	#endif

	*pfgIsUserMem = TRUE;
	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaTsRange_T);

	return 0;
}

static int CfaTsProcCompat(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	long ret = 0;

	if ((NULL == prInfo) || (NULL == pfgIsUserMem)) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT,
					TEXT("%s line %d fail for invalid parameter.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	}
	switch (prInfo->type) {
		case CFA_CONFIG:
			if (prInfo->is_get) {
				DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT,
					TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EPERM;
			}
			ret = CfaTsCompatConfig(prInfo, pfgIsUserMem);
			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaTsCompatConfig.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return ret;
			}
			break;
		case CFA_RANGE:
			if (prInfo->is_get) {
				DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT,
					TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EPERM;
			}
			ret = CfaTsCompatRange(prInfo, pfgIsUserMem);
			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaTsCompatRange.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return ret;
			}
			break;
		case CFA_GEN_INFO:
			DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT,
				TEXT("%s line %d fail for don't support get info for cfa Ts.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return -EPERM;
		case CFA_JUMP_INFO:
			DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT,
				TEXT("%s line %d fail for don't support get jump range for cfa Ts.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			break;
		default:
			break;
	}

	return 0;
}

#endif
/* TS CFA interface */
CfaIntf _rTsCfaIntf = {
	&CfaTsInit,
	&CfaTsUninit,
	&CfaTsSetRange,
	&CfaTsEnableStrm,
	&CfaTsSetStrmInf,
	&CfaTsTurnOn,
	&CfaTsTxDone,
	&CfaTsGetCurPos,
	NULL,
	&CfaTsConfigure,
	NULL,
	NULL,
	NULL,
	NULL,
	&CfaTsFillAUInfo,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
#ifdef CONFIG_COMPAT
	,&CfaTsProcCompat
#endif
};

/*-----------------------------------------------------------------------------
* Name: pvCfaTsGetInterface
*
* Description:
*	   Start of Public Function
*
* Inputs:
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
void *CfaTsGetInterface(void)
{
	DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
        TEXT("[CFA TS]CfaTsGetInterface(): Get Interface!\n"));

	return ((void *)(&_rTsCfaIntf));
}

