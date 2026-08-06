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


#include <media/atc/dmx_define.h>
#include <media/atc/drv_esm_if.h>
#include <media/atc/dmx_event.h>
/* #include <media/atc/mm_debug.h> */

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_spt.h"
#include "dmx_sema.h"
#include "cfa_macro.h"
#include "cfa_mpg.h"
#include "cfa_mpg_ana.h"
#include "cfa_mpg_st_ctrl.h"
#include "mmisc.h"

/* MPG CFA set video stream information.*/
/* @return None*/
/* @note Fdmx will ensure that it is only called in "off" or "paused" state.*/
/*< [IN] handle of fdmx*/
/*< [IN] stream information*/
/*< [IN] pointer to CfaMpgInst*/
static void CfaMpgSetVidStrmNo(void *pvSptHdl, const CfaMpgStrmInf *pStrmInfo, void *pvPrivData)
{
	CfaMpgInst *prCfaMpg = (CfaMpgInst *) pvPrivData;

	prCfaMpg->ucDecVidStId = (0x000000ffU & pStrmInfo->u1StrmId);
}


/* MPG CFA set audio stream information.*/
/* @return None*/
/* @note Fdmx will ensure that it is only called in "off" or "paused" state.*/
/*< [IN] handle of fdmx*/
/*< [IN] stream information*/
/*< [IN] pointer to CfaMpgInst*/
static void CfaMpgSetAudStrmNo(void *pvSptHdl, const CfaMpgStrmInf *pStrmInfo, void *pvPrivData)
{
	CfaMpgInst *prCfaMpg = (CfaMpgInst *) pvPrivData;
	u16 u2AudioID =
		(u16) ((u16)((u16)pStrmInfo->u1StrmId << 8)) | (u16) pStrmInfo->u1SubStrmIdPriStrm1;

	prCfaMpg->u2DecAudStId = (u16)((u16)0x0000ffff & u2AudioID);
}

/* MPG CFA set sub-picture stream information.*/
/* @return None*/
/* @note Fdmx will ensure that it is only called in "off" or "paused" state,*/
/*< [IN] handle of fdmx*/
/*< [IN] stream information*/
/*< [IN] pointer to CfaMpgInst*/
static void CfaMpgSetSpStrmNo(void *pvSptHdl, const CfaMpgStrmInf *pStrmInfo, void *pvPrivData)
{
	CfaMpgInst *prCfaMpg = (CfaMpgInst *) pvPrivData;

	prCfaMpg->u8DecSpStId = ((u32)pStrmInfo->u1StrmId << 8) | (u32)pStrmInfo->u1SubStrmIdPriStrm1;
	prCfaMpg->ucDecSpStId = (0x000000ffU & pStrmInfo->u1SubStrmIdPriStrm1);
}

/*
	pvSptHdl: Provided by Fdmx.  When using API in splitter4cfa.h, CFA should pass this handle as the 1st parameter.
	pvPrivData: Provided by App in MPC_CMD_INIT as MPC2FFDescr.pvCfaPrivData.
	FMPC passes it to Fdmx which passes to CFA.
*/
static void CfaMpgVidSpecHeader(void *pvSptHdl, CfaMpgInst *prCfaMpg)
{
	MRESULT mrRet = RET_DMX_OK;

	prCfaMpg->eCurPrsPktType = CFA_MPG_PRS_BIT_STRM_TYPE_HD;
	prCfaMpg->ucPrsVidStId = CFA_MPG_PRS_VID_STRM_ID_MV;

	mrRet = Spt4CfaBuf2VFifo(pvSptHdl, prCfaMpg->rVidSpecInfo.pu1VidSpecData,
				 0, CFA_PTM_SAME_POS, CFA_VID_MPEG2,
				 prCfaMpg->rVidSpecInfo.u4VidSpecDataLen);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT
				("[CFA MPG] CfaAviRETxMPEG1CocecHeader fail in Spt4CfaBuf2VFifoAUCtrl, ret:%d\r\n"),
				mrRet);
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
		return;
	}

	prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_SEARCH_SC;
}

static MRESULT CfaMpgInit(void *pvSptHdl, void **ppvCfaPrivData)
{
	CfaMpgInst *prCfaMpg = NULL;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
		TEXT("<CFA MPG> enter CfaMpgInit!"));

	DMX_NewMemory(sizeof(CfaMpgInst), prCfaMpg);
	if (NULL == prCfaMpg) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("[CFA_MPG] Alloc prCfaMpg memory fail\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}

	dmx_memset(prCfaMpg, 0x00, sizeof(CfaMpgInst));

	/*for cmdq memory allocate*/
#if CFA_MPG_HIGH_BIT_RATE_HANDLE
	prCfaMpg->rCfaVidInf.parCmdQTxEntry = NULL;
	DMX_NewMemory(sizeof(DMX_CMDQ_TX_ENTRY_T) * DMX_MAX_TX_CNT_FOR_CMD_Q,
			  prCfaMpg->rCfaVidInf.parCmdQTxEntry);
	if (!prCfaMpg->rCfaVidInf.parCmdQTxEntry) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT
				("[CFA_MPG] Alloc prCfaMpg->rCfaVidInf.parCmdQTxEntry memory fail\n"));
		DMX_FreeMemory(prCfaMpg);
		MM_RETURN(RET_DMX_NO_MEM);
	}
#endif

	prCfaMpg->ptrPfrMemAddress = DMX_INVALID_UINTPTR_T;
	prCfaMpg->eDbgCurCfaMpgAnaSt = CFA_MPG_ANA_ST_IDLE;
	prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_IDLE;
	prCfaMpg->u8Ca = DMX_INVALID_UINT64;
	prCfaMpg->eCfaMpgSysStrmType = CFA_MPG_SYS_STRM_TYPE_NONE;
	prCfaMpg->eCurPrsPktType = CFA_MPG_PRS_BIT_STRM_TYPE_NONE;
	prCfaMpg->u2DiscSectorSz = 0;

	/* information for playback*/
	prCfaMpg->u8PckPos = (u64)-1;
	prCfaMpg->u8PicPckPos = (u64)-1;
	/* information setting by application*/
	prCfaMpg->rCfaRange.u8Sa = DMX_INVALID_UINT64;
	prCfaMpg->rCfaRange.u8Ea = DMX_INVALID_UINT64;
	prCfaMpg->u2DecAudStId = DMX_INVALID_UINT16;
	prCfaMpg->ucDecSpStId = DMX_INVALID_UINT8;
	prCfaMpg->u4QueryInfType = CFA_MPG_QUERY_INF_TYPE_NONE;
	prCfaMpg->i8PtsResetAdValue = 0;
	prCfaMpg->fgFindFirstVideoPts = TRUE;
	prCfaMpg->fgSetJumpRange = FALSE;

	MMATE_INIT_STRUCT(prCfaMpg->rCfaRange);
	mm_memset(&(prCfaMpg->rStrmInf), 0x00, sizeof(prCfaMpg->rStrmInf));

	/* create semaphor for query information lock*/
	mrRet = dmx_sema_create(&prCfaMpg->hMutex, DMX_SEMA_TYPE_MUTEX,
		DMX_SEMA_STATE_UNLOCK);
	if (DMX_FAILED(mrRet)) {
		DMX_FreeMemory(prCfaMpg->rCfaVidInf.parCmdQTxEntry);
		DMX_FreeMemory(prCfaMpg);
		MM_RETURN(mrRet);
	}
	/* Assign cfa function pointer*/
	/*prCfaMpg->pCfaDrvIntf = &_rCfaDrvIntf;*/
	*ppvCfaPrivData = (void *) prCfaMpg;

	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
		TEXT("<CFA MPG> CfaMpgInit OK!"));
	MMATE_CHECK_POINTER(prCfaMpg);
	MMATE_CHECK_STRUCT(prCfaMpg->rStrmInf);

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaMpgUninit(void *pvSptHdl, void *pvCfaPrivData)
{
	CfaMpgInst *prCfaMpg = NULL;
	void *pvPointer = NULL;

	if (!pvCfaPrivData) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("CfaMpgUninit pvCfaPrivData NULL\r\n"));
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);
	}

	prCfaMpg = (CfaMpgInst *) pvCfaPrivData;
	pvPointer = (void *) prCfaMpg;

	MMATE_CHECK_POINTER(prCfaMpg);
	MMATE_CHECK_STRUCT(prCfaMpg->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaMpg->rStrmInf);

	if (NULL != prCfaMpg->hMutex) {
		dmx_sema_delete(prCfaMpg->hMutex);
		prCfaMpg->hMutex = NULL;
	}
#if CFA_MPG_HIGH_BIT_RATE_HANDLE
	if (prCfaMpg->rCfaVidInf.parCmdQTxEntry)
		DMX_FreeMemory(prCfaMpg->rCfaVidInf.parCmdQTxEntry);
#endif
	if (prCfaMpg->rVidSpecInfo.pu1VidSpecData) {
		DMX_FreeHwMemory(prCfaMpg->rVidSpecInfo.pu1VidSpecData);
		prCfaMpg->rVidSpecInfo.pu1VidSpecData = NULL;
	}
	if (pvPointer)
		DMX_FreeMemory(pvPointer);

	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
		TEXT("<CFA MPG> CfaMpgUninit OK!"));

	MM_RETURN(RET_DMX_OK);
}

/*bool _fgCfaMpgHbTest = FALSE;*/
/* MPG CFA sets demuxing range*/
/* @return s32*/
/* @note Fdmx will ensure that pfvSetRange is only called in "off" state.*/
/*		 If used with FMPC, the range of MPC_SCMD_SPR will be passed here*/
/*< [IN] handle of fdmx*/
/*< [IN] pointer to CfaMpgRange*/
/*< [IN] pointer to CfaMpgInst*/
static MRESULT CfaMpgSetRange(void *pvSptHdl, void *pvRange, void *pvPrivData, bool fgIsUserMem)
{
	CfaMpgInst *prCfaMpg = NULL;

	if (!pvRange) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("<CFA MPG> CfaMpgSetRange: pvRange == NULL\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!pvPrivData) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("<CFA MPG> CfaMpgSetRange: pvPrivate == NULL\n"));
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);
	}

	prCfaMpg = (CfaMpgInst *) pvPrivData;
	MMATE_CHECK_POINTER(prCfaMpg);
	MMATE_CHECK_STRUCT(prCfaMpg->rStrmInf);

	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
		TEXT("<CFA MPG> CfaMpgSetRange!\n"));

#ifdef MM_ATE_CHECK
	if (0 != mm_copy_from_user(&(prCfaMpg->rCfaRange.u8Sa), &(((CfaMpgRange *) pvRange)->u8Sa),
		  sizeof(prCfaMpg->rCfaRange) - 2 * sizeof(u32))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("<CFA MPG> %s line %d fail in mm_copy_from_user\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}
#else
	if (0 != mm_copy_from_user(&(prCfaMpg->rCfaRange), pvRange, sizeof(prCfaMpg->rCfaRange))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("<CFA MPG> %s line %d fail in mm_copy_from_user\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}
#endif

  if(prCfaMpg->rCfaRange.u8Sa > prCfaMpg->rCfaRange.u8Ea) {
      MM_RETURN(RET_DMX_PARAM_WRONG);
  }

	prCfaMpg->fgSupportHibitRt = FALSE;
	prCfaMpg->fgIsSeek = FALSE;
	prCfaMpg->i8DeltaPts = 0;
	prCfaMpg->fgIsNoBeyondMaxTx = TRUE;
	MMATE_INIT_STRUCT(prCfaMpg->rCfaRange);

#if CFA_MPG_HIGH_BIT_RATE_HANDLE
	if ((prCfaMpg->rCfaRange.fgHighBitrate) &&
		(prCfaMpg->eCfaMpgMediumType == CFA_MPG_MED_TYPE_FILE)) {
		prCfaMpg->fgSupportHibitRt = TRUE;
		DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("[CFA MPG] fgSupportHibitRt = TRUE\n"));
	} else {
		prCfaMpg->fgSupportHibitRt = FALSE;
		DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("[CFA MPG] fgSupportHibitRt = FALSE\n"));
	}
#if CFA_MPG_SUPPORT_AVC
	if ((prCfaMpg->eCfaMpgMediumType == CFA_MPG_MED_TYPE_FILE)
		&& (AVCODEC_ID_H264 == prCfaMpg->u2StreamVideoType)) {
		prCfaMpg->fgSupportHibitRt = FALSE;
	}
#endif

	Spt4CfaNotiCurStrmInf(pvSptHdl, prCfaMpg->rCfaRange.fgHighBitrate);
#endif
	if (prCfaMpg->rCfaRange.fgIsSeek) {
		prCfaMpg->fgIsSeek = TRUE;
		prCfaMpg->fgFindFirstVideoPts = FALSE;
		prCfaMpg->u8TimeCode = prCfaMpg->rCfaRange.u8SeekTime;
	} else {
		prCfaMpg->i8PtsResetAdValue = 0;
		prCfaMpg->fgFindFirstVideoPts = TRUE;
	}

	if(!(prCfaMpg->u4CurPrsFlg & CFA_MPG_PRS_BIT_STRM_TYPE_V))
	{
		prCfaMpg->fgFindFirstVideoPts = FALSE;
	}

	MM_RETURN(RET_DMX_OK);
}


/* MPG CFA sets stream to parse, may be combinations of V/A/S.*/
/* @return None*/
/* @note Fdmx will ensure that pfvEnableStrm() is only called in "off" or "paused" state.*/
/*< [IN] handle of fdmx*/
/*< [IN] streams to parse or to cancel parsing*/
/*< [IN] CFA_STREAM_ON:
	The bits turned ON in u4StrmToPrs are the streams that FMPC would like to parse.
	CFA_STRM_OFF:
	The bits turned ON in u4StrmToPrs are the streams that FMPC would like to stop parsing*/
/*< [IN] pointer to CfaMpgInst*/
static MRESULT CfaMpgEnableStrm(void *pvSptHdl, u32 u4StrmToPrs, CfaStreamOp eOp, void *pvPrivData)
{
	CfaMpgInst *prCfaMpg = NULL;

	if (!pvPrivData) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("CfaMpgEnableStrm pvPrivData NULL\r\n"));
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);
	}

	prCfaMpg = (CfaMpgInst *) pvPrivData;
	MMATE_CHECK_POINTER(prCfaMpg);
	MMATE_CHECK_STRUCT(prCfaMpg->rStrmInf);

	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
		TEXT("<CFA MPG> CfaMpgEnableStrm\n"));
	if (CFA_STREAM_ON == eOp) {
		/* enable*/
		if (CFA_STRM_V & u4StrmToPrs)
			prCfaMpg->u4CurPrsFlg |= CFA_MPG_PRS_BIT_STRM_TYPE_V;

		if (CFA_STRM_A & u4StrmToPrs)
			prCfaMpg->u4CurPrsFlg |= CFA_MPG_PRS_BIT_STRM_TYPE_A;

		if (CFA_STRM_SP & u4StrmToPrs)
			prCfaMpg->u4CurPrsFlg |= CFA_MPG_PRS_BIT_STRM_TYPE_SP0;

		if (CFA_STRM_NV & u4StrmToPrs)
			prCfaMpg->u4CurPrsFlg |= CFA_MPG_PRS_BIT_STRM_TYPE_NV;
			} else {		/* disable*/
		if (CFA_STRM_V & u4StrmToPrs)
			prCfaMpg->u4CurPrsFlg &= ~((u32) CFA_MPG_PRS_BIT_STRM_TYPE_V);

		if (CFA_STRM_A & u4StrmToPrs)
			prCfaMpg->u4CurPrsFlg &= ~((u32) CFA_MPG_PRS_BIT_STRM_TYPE_A);

		if (CFA_STRM_SP & u4StrmToPrs)
			prCfaMpg->u4CurPrsFlg &= ~((u32) CFA_MPG_PRS_BIT_STRM_TYPE_SP0);

		if (CFA_STRM_NV & u4StrmToPrs)
			prCfaMpg->u4CurPrsFlg &= ~((u32) CFA_MPG_PRS_BIT_STRM_TYPE_NV);

	}

	MM_RETURN(RET_DMX_OK);
}


/*< [IN] handle of fdmx*/
/*< [IN] stream to set*/
/*< [IN] stream info*/
/*< [IN] pointer to CfaMpgInst*/
static MRESULT CfaMpgSetStrmInf(void *pvSptHdl, u32 u4Strm, u32 u4StrmInf, void *pvPrivData)
{
	CfaMpgStrmInf streamInf;

	if (!pvPrivData) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("CfaMpgSetVidStrmNo pStrmInfo NULL || PvPrivData NULL\n"));
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);
	}

	mm_memset(&streamInf, 0, sizeof(streamInf));
	mm_memcpy(&streamInf, &u4StrmInf, sizeof(streamInf));

	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
		TEXT("<CFA MPG> CfaMpgEnableStrm\n"));

	switch (u4Strm) {
	case CFA_STRM_V:
		CfaMpgSetVidStrmNo(pvSptHdl, &streamInf, pvPrivData);
		break;

	case CFA_STRM_A:
		CfaMpgSetAudStrmNo(pvSptHdl, &streamInf, pvPrivData);
		break;

	case CFA_STRM_SP:
		CfaMpgSetSpStrmNo(pvSptHdl, &streamInf, pvPrivData);
		break;

	case CFA_STRM_NV:
		break;

	default:/* not support yet*/
		MM_RETURN(RET_DMX_PARAM_WRONG);		
		break;
	}

	MM_RETURN(RET_DMX_OK);
}


/* MPG CFA turns on file demuxing*/
/* @return None*/
/* @note A transfer should be issued in this function.*/
static MRESULT CfaMpgTurnOn(void *pvSptHdl, void *pvPrivData)
{
	CfaMpgInst *prCfaMpg = NULL;
	MRESULT mrRetVal = RET_DMX_OK;

	if (!pvPrivData) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("CfaMpgTurnOn pvPrivData NULL\r\n"));
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);
	}

	prCfaMpg = (CfaMpgInst *) pvPrivData;
	MMATE_CHECK_POINTER(prCfaMpg);
	MMATE_CHECK_STRUCT(prCfaMpg->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaMpg->rStrmInf);

	DmxLogT(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
		TEXT("<CFA MPG>[CFA_VERSION:2017-0307-1028] CfaMpgTurnOn: SA 0x%llx -- EA 0x%llx\n"),
		prCfaMpg->rCfaRange.u8Sa, prCfaMpg->rCfaRange.u8Ea);
	if (prCfaMpg->rCfaRange.u8Ea <= prCfaMpg->rCfaRange.u8Sa) {
		/*add by guoqing yang for bug1970*/
		if (DMX_IS_FF_PLAY(pvSptHdl))
			Spt4CfaFinishedEx(pvSptHdl, prCfaMpg->rCfaRange.u8Ea, TRUE, (u32)GAU_E_EOS);
		 else
			Spt4CfaFinishedEx(pvSptHdl, prCfaMpg->rCfaRange.u8Ea, FALSE, (u32)GAU_E_EOS);

		MM_RETURN(RET_DMX_OK);
	}

	if (DMX_IS_NORMAL_PLAY(pvSptHdl))
		prCfaMpg->fgSetJumpRange = FALSE;

	/* initialize data*/
	mm_memset(prCfaMpg->aucDecVstId, 0, sizeof(u8) * CFA_MPG_MAX_STRM_NS);
	mm_memset(prCfaMpg->aucDecVstType, 0, sizeof(u8) * CFA_MPG_MAX_STRM_NS);
	mm_memset(prCfaMpg->arAudInf, 0, sizeof(CFAMPG_AUD_INF) * CFA_MPG_MAX_STRM_NS);
	mm_memset(prCfaMpg->arSPInf, 0, sizeof(CFAMPG_SP_INF) * CFA_MPG_MAX_STRM_NS);
	mm_memset(prCfaMpg->au8VidLastPrsPts, 0, sizeof(u64) * CFA_MPG_MAX_STRM_NS);
	mm_memset(prCfaMpg->fgVidPtsAdjust, 0, sizeof(bool) * CFA_MPG_MAX_STRM_NS);	/* for stc reset*/
	mm_memset(prCfaMpg->i8VidPtsAdjust, 0, sizeof(u64) * CFA_MPG_MAX_STRM_NS);	/*for stc reset*/
	prCfaMpg->u8Ca = prCfaMpg->rCfaRange.u8Sa;
	prCfaMpg->u4ParsedBytes = 0;
	prCfaMpg->pucHdrBufRp = NULL;
	prCfaMpg->pucHdrBuf = NULL;
	prCfaMpg->ptrPfrMemAddress = DMX_INVALID_UINTPTR_T;
	prCfaMpg->u8CfaIssueTxLen = 0;
	prCfaMpg->u4RdyDataSz = 0;
	prCfaMpg->u4AvailSz = 0;
	prCfaMpg->u8LastSyncPbbufCa = 0;
	prCfaMpg->u4LastAvailSz = 0;
	prCfaMpg->fgExitTxDoneCtrl = FALSE;
	prCfaMpg->fgEverTx = FALSE;
	prCfaMpg->fgTxData2HdrBuf = FALSE;
	prCfaMpg->fgNoSupportAutoPause = FALSE;
	prCfaMpg->eCurPrsPktType = CFA_MPG_PRS_BIT_STRM_TYPE_NONE;
	prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_IDLE;
	prCfaMpg->eDbgCurCfaMpgAnaSt = CFA_MPG_ANA_ST_IDLE;
	prCfaMpg->u4VstNs = 0;
	prCfaMpg->u4AstNs = 0;
	prCfaMpg->u4SpstNs = 0;
	prCfaMpg->u4SubPicUNum = 0;
	prCfaMpg->fgLpcmEmphasis = TRUE;
	prCfaMpg->fgCalPtsByScr = FALSE;
	prCfaMpg->i8DeltaPts = 0;
	prCfaMpg->fgHasAud = FALSE;
	prCfaMpg->fgFillDummyAU = FALSE;
#if CFA_MPG_SEAMLESS_2ND_RANGE_SUPPORT
	/*Add by pingzhao, for BDP00018159, 2008/12/30*/
	if (!prCfaMpg->rCfaRange.fgSeamless2ndRange) {
#endif
		prCfaMpg->ucAPS = DMX_INVALID_UINT8;
		prCfaMpg->ucCGMS = DMX_INVALID_UINT8;
		prCfaMpg->ucAnalogSrc = DMX_INVALID_UINT8;
#if CFA_MPG_SEAMLESS_2ND_RANGE_SUPPORT
	}
#endif
	prCfaMpg->ucAspectRatio = DMX_INVALID_UINT8;
	prCfaMpg->fgFillAU = FALSE;
	prCfaMpg->u2PrevPktLen = 0;
	prCfaMpg->u2PktLen = 0;
	prCfaMpg->fgExistSCR = FALSE;
	prCfaMpg->fgExistDts = FALSE;
	prCfaMpg->fgExistPts = FALSE;
	prCfaMpg->u4PESHdrDataLen = 0;
	prCfaMpg->u2SkipLen = 0;
	prCfaMpg->fgFindIFrame = FALSE;
	prCfaMpg->fgIFrameEnd = FALSE;

	prCfaMpg->u8SCR = DMX_INVALID_UINT64;
	prCfaMpg->u8PrsPts = INVALID_TIMESTAMP;
	prCfaMpg->u8PrsPrevPts = INVALID_TIMESTAMP;
	prCfaMpg->u8PrsDts = INVALID_TIMESTAMP;
	prCfaMpg->u8LastPts = INVALID_TIMESTAMP;
	prCfaMpg->u8LastPtsAddr = prCfaMpg->u8Ca;

	if (prCfaMpg->eCfaMpgMediumType == CFA_MPG_MED_TYPE_FILE) {
		prCfaMpg->rCfaRange.u8VidSa = (u64) (CFA_MPG_INVALID_RANGE_START_ADDRESS);
		prCfaMpg->rCfaRange.u8AudSa = (u64) (CFA_MPG_INVALID_RANGE_START_ADDRESS);
		prCfaMpg->rCfaRange.u8SPSa = (u64) (CFA_MPG_INVALID_RANGE_START_ADDRESS);
	}

	if (prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_NONE) {
		prCfaMpg->i8StcOffset = 0;
		prCfaMpg->u2VOB_ID = 0;
	} else if (prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_VALUE) {
		prCfaMpg->i8StcOffset = prCfaMpg->rCfaRange.i8AdjustValue;
	} else {
		/*do nothing*/
	}

		if ((prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_NONE) ||
		(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_VALUE) ||
		(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_KEEP_CURRENT)) {
#if CFA_MPG_KEEP_VOB_PTM
			if (!prCfaMpg->rCfaRange.fgKeepVobPtm)
#endif
				{
				prCfaMpg->u4VOBU_V_S_PTM = 0;
				prCfaMpg->u4VOBU_V_E_PTM = 0;
				prCfaMpg->u4VOB_V_S_PTM = 0;
				prCfaMpg->u4VOB_V_E_PTM = 0;
			}
		}

	prCfaMpg->fgIsCSSDecOn = FALSE;
	prCfaMpg->fgIsCPRMDecOn = FALSE;
	prCfaMpg->u2CSSJumpLen = 0;

	prCfaMpg->u4HLI_S_PTM = DMX_INVALID_UINT32;
	prCfaMpg->u4HLI_E_PTM = DMX_INVALID_UINT32;
	prCfaMpg->u8PicPckPos = DMX_INVALID_UINT64;
	prCfaMpg->u2PrsAudStId = DMX_INVALID_UINT16;
	prCfaMpg->ucPrsSpStId = DMX_INVALID_UINT8;
	prCfaMpg->ucPrsVidStId = DMX_INVALID_UINT8;
	prCfaMpg->u4CurQueryInfType = prCfaMpg->u4QueryInfType;
	prCfaMpg->ucAudBound = DMX_INVALID_UINT8;
	prCfaMpg->ucVidBound = DMX_INVALID_UINT8;

#if CFA_MPG_HIGH_BIT_RATE_HANDLE
	prCfaMpg->fgTxData2FIFO = FALSE;
	prCfaMpg->u4PtsArrayIndex = 0;
	prCfaMpg->fgIsNoBeyondMaxTx = TRUE;
	if (prCfaMpg->u4CurQueryInfType == 0)
		prCfaMpg->u8HiBiBuSize = CFA_MPG_TX_HIBI_BUFSIZE;

	prCfaMpg->u8DataInBuf = 0;
	/*for CMD q*/
	prCfaMpg->u2CmdQIndex = 0;
	prCfaMpg->u8PreTxEndOff = 0;
	prCfaMpg->u8FirstOffset = 0;
	prCfaMpg->u8LatTxAvaLen = 0;
	prCfaMpg->fgTxByPbbuf = FALSE;
	prCfaMpg->fgCMDQTx = FALSE;
	prCfaMpg->fgCMDQFirstFill = FALSE;
#endif
	prCfaMpg->u4BitRate = 0;
	/*prCfaMpg->u8PckPos can use to detect error data*/
	prCfaMpg->u8PckPos = prCfaMpg->u8Ca;
	/*for demux error handle*/
	prCfaMpg->fgDealDxEr = TRUE;
	if ((!prCfaMpg->fgIsSeek) &&
		(CFA_MPG_QUERY_INF_TYPE_NONE == prCfaMpg->u4CurQueryInfType) &&
		(CFA_MPG_MED_TYPE_FILE == prCfaMpg->eCfaMpgMediumType) &&
		(prCfaMpg->rCfaRange.u8Ea - prCfaMpg->rCfaRange.u8Sa > CFA_MPG_DEMUX_MAX_LEN)) {
		prCfaMpg->fgDemuxError = FALSE;
	}

	if (CFA_MPG_SYS_STRM_TYPE_VIDEO_SEQUENCE == prCfaMpg->eCfaMpgSysStrmType) {
		/* MPEG video stream only*/
		if (!(CFA_MPG_PRS_BIT_STRM_TYPE_V & prCfaMpg->u4CurPrsFlg)) {
			DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("<CFA MPG> MPEG video stream only,but video is not enable,")
				TEXT("call finish!\r\n"));
			Spt4CfaFinishedEx(pvSptHdl, prCfaMpg->rCfaRange.u8Sa, TRUE, (u32)GAU_E_EOS);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}

		prCfaMpg->u8PrsPts = INVALID_TIMESTAMP;
		prCfaMpg->u8PrsDts = INVALID_TIMESTAMP;
		prCfaMpg->eCurPrsPktType = CFA_MPG_PRS_BIT_STRM_TYPE_V;
		prCfaMpg->ucPrsVidStId = prCfaMpg->ucDecVidStId;
		prCfaMpg->u4RdyDataSz = 0;
		prCfaMpg->pucHdrBufRp = NULL;


		if ((prCfaMpg->rCfaRange.u8Ea - prCfaMpg->u8Ca) > (100)) {
			/*add this code for nothing but drive splitter*/
					mrRetVal =
				Spt4CfaPbb2SyncBufEx(pvSptHdl, prCfaMpg->u8Ca, 100,
						 (u8 *) &prCfaMpg->ptrPfrMemAddress,
						 &prCfaMpg->u4AvailSz);
			prCfaMpg->u8CfaIssueTxLen = 100;
			if (mrRetVal != RET_DMX_OK) {
				DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
						TEXT("CfaMpgTurnOn(): sync pbbuf fail!\n"));
				MM_RETURN(mrRetVal);
			}
		} else {
			CfaMpgTxNextStrmDataToFifo(pvSptHdl, prCfaMpg, prCfaMpg->u8Ca,
						   prCfaMpg->rCfaRange.u8Ea -
						   prCfaMpg->rCfaRange.u8Sa + 1);
		}

		prCfaMpg->fgExitTxDoneCtrl = FALSE;
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_MPGV;
	} else {		/* MPEG stream*/
		prCfaMpg->u4RdyDataSz = 0;
		prCfaMpg->pucHdrBufRp = NULL;
#if CFA_MPG_HIGH_BIT_RATE_HANDLE
		prCfaMpg->u4AvailSz = 0;
#endif
		CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
		prCfaMpg->fgExitTxDoneCtrl = FALSE;

		if ((FALSE == prCfaMpg->fgSetJumpRange) &&
			(prCfaMpg->rVidSpecInfo.u4VidSpecDataLen > 0)) {
			prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_VID_HEADER;
			DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					TEXT("CfaMpgTurnOn---CFA_MPG_ANA_ST_VID_HEADER\n"));
		} else {
			prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_SEARCH_SC;
			DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("CfaMpgTurnOn---CFA_MPG_ANA_ST_SEARCH_SC\n"));
		}
	}

	MM_RETURN(RET_DMX_OK);
}


/* MPG CFA callback for transfer done*/
/* @return None*/
/* @note This function will be called after a transfer is complete.*/
/*< [IN] handle of fdmx*/
/*< [IN] Actual transferred data length.  Normally this value should be equal to the u4Len in the*/
/*		   previous transfer issue, unless file end is hit.*/
/*< [IN] pointer to CfaMpgInst*/
static MRESULT CfaMpgTxDone(void *pvSptHdl, u64 u8TxLen, void *pvPrivData, bool fgRsp)
{
	CfaMpgInst *prCfaMpg = NULL;
	MRESULT mrRetVal = RET_DMX_OK;

	if (!pvPrivData) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("CfaMpgTxDone pvPrivData NULL\r\n"));
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);
	}

	prCfaMpg = (CfaMpgInst *) pvPrivData;
	MMATE_CHECK_POINTER(prCfaMpg);
	MMATE_CHECK_STRUCT(prCfaMpg->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaMpg->rStrmInf);

	if (fgRsp) {
		mrRetVal =
			Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaMpg->u8Ca, u8TxLen,
					   (u8 *) &(prCfaMpg->ptrPfrMemAddress));
		MM_RETURN(mrRetVal);
	}

	switch (prCfaMpg->eCurCfaMpgAnaSt) {
	case CFA_MPG_ANA_ST_VID_HEADER:
		DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("CfaMpgTxDone---CFA_MPG_ANA_ST_VID_HEADER\n"));
		CfaMpgVidSpecHeader(pvSptHdl, prCfaMpg);
		break;
	default:
		DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("CfaMpgTxDone---other\n"));
		CfaMpgTxDoneStCtrl(pvSptHdl, u8TxLen, prCfaMpg);
		break;
	}

	MM_RETURN(RET_DMX_OK);
}


/* MPG CFA callback for getting current position*/
/* @return None*/
static MRESULT CfaMpgGetCurPos(void *pvSptHdl, void *pvCurPos, void *pvPrivData)
{
	CfaMpgInst *prCfaMpg = NULL;
	u64 *pvu8 = NULL;

	if (NULL == pvCurPos)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	if (NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("CfaMpgGetCurPos pvCurPos NULL\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	pvu8 = (u64 *) pvCurPos;

	prCfaMpg = (CfaMpgInst *) pvPrivData;
	*pvu8 = prCfaMpg->u8Ca;

	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("<CFA_MPG>Enter CfaMpgGetCurPos u8Ca 0x%llx\r\n"),
			prCfaMpg->u8Ca);

	MM_RETURN(RET_DMX_OK);
}


/* MPG CFA callback for each picture is demuxed*/
/* @return TRUE - this picture should be retained in video FIFO.*/
/*			FALSE - this picture should be removed from video FIFO.*/
/*< [IN] handle of Fdmx*/
/*< [IN/OUT] Picture info, @see Spt2CfaPicInfo*/
/*< [IN] pointer to CfaMpgInst*/
static MRESULT CfaMpgFillPicInfo(void *pvSptHdl, Spt2CfaPicInfo *ptPicInfo, void *pvPrivData)
{
	CfaMpgInst *prCfaMpg = NULL;

	if (!pvPrivData) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("<CFA MPG> CfaMpgFillPicInfo:pvPrivData NULL\n"));
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);
	}

	if (!ptPicInfo) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("<CFA MPG> CfaMpgFillPicInfo: ptPicInfo NULL\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
		TEXT("<CFA MPG> CfaMpgFillPicInfo\n"));

	prCfaMpg = (CfaMpgInst *) pvPrivData;
	MMATE_CHECK_POINTER(prCfaMpg);
	MMATE_CHECK_STRUCT(prCfaMpg->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaMpg->rStrmInf);

	ptPicInfo->u8ThisDts = prCfaMpg->u8PrsDts;
	ptPicInfo->u8ThisPts = prCfaMpg->u8PrsPts;
	ptPicInfo->u8Custom1 = prCfaMpg->u8PicPckPos;

	prCfaMpg->u8PrsDts = INVALID_TIMESTAMP;
	prCfaMpg->u8PrsPts = INVALID_TIMESTAMP;

	MM_RETURN(RET_DMX_OK);
}


/* MPG CFA callback for transfer done*/
/* @return None*/
/* @note Fdmx will ensure that it is only called in "off" or "paused" state.*/
/*< [IN] handle of fdmx*/
/*< [IN] configure paramter*/
/*< [IN] pointer to CfaMpgInst*/
static MRESULT CfaMpgConfigure(void *pvSptHdl, void *pvParam, void *pvPrivData, bool fgIsUserMem)
{
	CfaMpgInst *prCfaMpg = NULL;
	CfaMpgCfg  rMpgCfg;
	CfaMpgCfg *prConfig = NULL;
	u32 u4VideoFrameRate = 0;

	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
		TEXT("<CFA MPG> CfaMpgConfigure\n"));

	if (!pvPrivData) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("CfaMpgSetRange:pvPrivData NULL\n"));
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);
	}
	if (!pvParam) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("CfaMpgSetRange: pvParam NULL\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mm_memset(&rMpgCfg, 0, sizeof(rMpgCfg));

	if (fgIsUserMem) {
		if (0 != mm_copy_from_user(&rMpgCfg, pvParam, sizeof(CfaMpgCfg))) {
			DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("%s line %d fail in mm_copy_from_user\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
		}
	}
	else {
		mm_memcpy(&rMpgCfg, pvParam, sizeof(CfaMpgCfg));
	}

	prCfaMpg = (CfaMpgInst *) pvPrivData;
	prConfig = (CfaMpgCfg *) &rMpgCfg;

	MMATE_CHECK_POINTER(prCfaMpg);

	if (TRUE == prConfig->fgMpgVidStrmOnly)
		prCfaMpg->eCfaMpgSysStrmType = CFA_MPG_SYS_STRM_TYPE_VIDEO_SEQUENCE;
	 else
		prCfaMpg->eCfaMpgSysStrmType = CFA_MPG_SYS_STRM_TYPE_NONE;

	if (NULL != prConfig->pu1VidSpecData) {
		prCfaMpg->rVidSpecInfo.pu1VidSpecData = NULL;
		prCfaMpg->rVidSpecInfo.u4VidSpecDataLen = prConfig->u4VidSpecDataLen;
		prCfaMpg->rVidSpecInfo.u8VidSpecDataOfst = prConfig->u8VidSpecDataOfst;

		DMX_NewHwMemory(prCfaMpg->rVidSpecInfo.u4VidSpecDataLen * sizeof(u8),
				prCfaMpg->rVidSpecInfo.pu1VidSpecData);
		if (NULL == prCfaMpg->rVidSpecInfo.pu1VidSpecData) {
			DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					TEXT("<MPG> Allocate spec header info memory failed\n"));
			if (!fgIsUserMem)
				DMX_FreeMemory(pvParam);
			MM_RETURN(RET_DMX_NO_MEM);
		}

		dmx_memset(prCfaMpg->rVidSpecInfo.pu1VidSpecData, 0,
			   prCfaMpg->rVidSpecInfo.u4VidSpecDataLen);
		if (fgIsUserMem) {
			if (0 != mm_copy_from_user(prCfaMpg->rVidSpecInfo.pu1VidSpecData,
					prConfig->pu1VidSpecData,
					prCfaMpg->rVidSpecInfo.u4VidSpecDataLen)) {
				DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					TEXT("%s line %d fail in mm_copy_from_user(pu1VidSpecData: 0x%p)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prConfig->pu1VidSpecData);
				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
			}
		}
		else {
			dmx_memcpy(prCfaMpg->rVidSpecInfo.pu1VidSpecData,
				prConfig->pu1VidSpecData,
				prCfaMpg->rVidSpecInfo.u4VidSpecDataLen);
		}
		DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("<Wrtang>CfaMpgConfigure---5 byte(%x-%x-%x-%x-%x)\n"),
				prCfaMpg->rVidSpecInfo.pu1VidSpecData[0],
				prCfaMpg->rVidSpecInfo.pu1VidSpecData[1],
				prCfaMpg->rVidSpecInfo.pu1VidSpecData[2],
				prCfaMpg->rVidSpecInfo.pu1VidSpecData[3],
				prCfaMpg->rVidSpecInfo.pu1VidSpecData[4]);

	} else {
		prCfaMpg->rVidSpecInfo.pu1VidSpecData = NULL;
		prCfaMpg->rVidSpecInfo.u4VidSpecDataLen = 0;
		prCfaMpg->rVidSpecInfo.u8VidSpecDataOfst = 0;
	}

	u4VideoFrameRate = (u32)(prConfig->u4VideoFps_n / prConfig->u4VideoFps_d);
	if (u4VideoFrameRate > (u32)60)
		u4VideoFrameRate = (u32)60;
	 else if (u4VideoFrameRate == 0)
		u4VideoFrameRate = (u32)30;
	 else {
		/*do nothing*/
	 }

	prCfaMpg->u4VideoFrameDuration = (u32)CFA_MPG_PTS_1S / u4VideoFrameRate;

	prCfaMpg->u2DiscCpsType = prConfig->u2DiscCpsType;
	switch (prConfig->u4CfaMpgMediumType) {
	case CFA_MPG_MED_CFG_TYPE_FILE:
		prCfaMpg->eCfaMpgMediumType = CFA_MPG_MED_TYPE_FILE;
		prCfaMpg->u2StreamVideoType = prConfig->u2VideoType;
		break;
	default:
		prCfaMpg->eCfaMpgMediumType = CFA_MPG_MED_TYPE_NONE;
		break;
	}

	prCfaMpg->u2DiscSectorSz = prConfig->u2DiscSectorSz;

	if (!fgIsUserMem)
		DMX_FreeMemory(pvParam);

	MM_RETURN(RET_DMX_OK);
}


/* MPG CFA sets information query types*/
/* @return None*/
/* @note Fdmx will ensure that it is only called in "off" or "paused" state.*/
/*< [IN] handle of fdmx*/
/*< [IN] information type for MPG CFA*/
/*< [IN] pointer to CfaMpgInst*/
static MRESULT CfaMpgSetInqTypes(void *pvSptHdl, u32 u4InfTypes, void *pvPrivData)
{
	CfaMpgInst *prCfaMpg = NULL;

	if (!pvPrivData) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("CfaMpgSetInqTypes pvPrivData NULL\r\n"));
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);
	}

	prCfaMpg = (CfaMpgInst *) pvPrivData;

	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
		TEXT("<CFA MPG> CfaMpgSetInqTypes\n"));


	if (CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_SC_INF & u4InfTypes)
		prCfaMpg->u4QueryInfType |= CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_SC_INF;

	if (CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_PTS_INF & u4InfTypes)
		prCfaMpg->u4QueryInfType |= CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_PTS_INF;

	if (CFA_MPG_QUERY_INF_TYPE_STRM_INF & u4InfTypes)
		prCfaMpg->u4QueryInfType |= CFA_MPG_QUERY_INF_TYPE_STRM_INF;

	if (CFA_MPG_QUERY_INF_TYPE_AUTO_PAUSE_INF & u4InfTypes)
		prCfaMpg->u4QueryInfType |= CFA_MPG_QUERY_INF_TYPE_AUTO_PAUSE_INF;

	if (CFA_MPG_QUERY_INF_TYPE_MUX_RATE_INF & u4InfTypes)
		prCfaMpg->u4QueryInfType |= CFA_MPG_QUERY_INF_TYPE_MUX_RATE_INF;

	if (CFA_MPG_QUERY_INF_TYPE_LAST_MPEG_PTS_INF & u4InfTypes)
		prCfaMpg->u4QueryInfType |= CFA_MPG_QUERY_INF_TYPE_LAST_MPEG_PTS_INF;

	if (CFA_MPG_QUERY_INF_TYPE_NONE == u4InfTypes)
		prCfaMpg->u4QueryInfType = CFA_MPG_QUERY_INF_TYPE_NONE;

	MM_RETURN(RET_DMX_OK);
}


/*< [IN] input splitter Handle*/
/*< [IN] CFA function id, set or get id, it shall be defined by CFA and LPE*/
/*< [IN] input CFA private data*/
/*< [OUT] The parameter of this FID, it shall be defined by CFA and LPE*/
/*< [IN] The size of this parameter (of this FID), it shall be defined by CFA and LPE*/
static MRESULT CfaMpgGetGeneral(void *pvSptHdl, u32 u4CfaFID, void *pvPrivData,
				void *pvCfaParameter, u32 u4CfaParameterSize)
{
	CfaMpgInst *prCfaMpg = NULL;

	if (!pvCfaParameter) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("CfaMpgGetGeneral pvCfaParameter NULL\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!pvPrivData) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("CfaMpgGetGeneral pvPrivData NULL\r\n"));
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);
	}

	prCfaMpg = (CfaMpgInst *) pvPrivData;
	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
		TEXT("<CFA MPG> CfaMpgGetGeneral\n"));

	switch (u4CfaFID) {
	case CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_SC_INF:
		if(u4CfaParameterSize != sizeof(CfaMpgQIFirstMpgScInf))
		{
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}

		VERIFY(RET_DMX_OK == dmx_sema_lock(prCfaMpg->hMutex,
			DMX_SEMA_OPTION_WAIT));
		mm_memcpy(pvCfaParameter, &prCfaMpg->rFirstMpgScInf,
			  sizeof(prCfaMpg->rFirstMpgScInf));
		u4CfaParameterSize = sizeof(CfaMpgQIFirstMpgScInf);
		VERIFY(RET_DMX_OK == dmx_sema_unlock(prCfaMpg->hMutex));
		break;
	case CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_PTS_INF:
		if(u4CfaParameterSize != sizeof(CfaMpgQIFirstMpgPtsInf))
		{
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}

		VERIFY(RET_DMX_OK == dmx_sema_lock(prCfaMpg->hMutex,
			DMX_SEMA_OPTION_WAIT));
		mm_memcpy(pvCfaParameter, &prCfaMpg->rFirstMpgPtsInf,
			  sizeof(prCfaMpg->rFirstMpgPtsInf));
		u4CfaParameterSize = sizeof(CfaMpgQIFirstMpgPtsInf);
		VERIFY(RET_DMX_OK == dmx_sema_unlock(prCfaMpg->hMutex));
		break;
	case CFA_MPG_QUERY_INF_TYPE_STRM_INF:
		if(u4CfaParameterSize != sizeof(CfaMpgQIStrmInf))
		{
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}

		VERIFY(RET_DMX_OK == dmx_sema_lock(prCfaMpg->hMutex,
			DMX_SEMA_OPTION_WAIT));
#ifdef MM_ATE_CHECK
		mm_memcpy(&(((CfaMpgQIStrmInf *) pvCfaParameter)->u4AudStrmNs),
			  &prCfaMpg->rStrmInf.u4AudStrmNs,
			  sizeof(prCfaMpg->rStrmInf) - 2 * sizeof(u32));
#else
		mm_memcpy(pvCfaParameter, &prCfaMpg->rStrmInf,
			sizeof(prCfaMpg->rStrmInf));
#endif
		u4CfaParameterSize = sizeof(CfaMpgQIStrmInf);
		VERIFY(RET_DMX_OK == dmx_sema_unlock(prCfaMpg->hMutex));
		break;
	case CFA_MPG_QUERY_INF_TYPE_AUTO_PAUSE_INF:	/* No This Data.*/
		if(u4CfaParameterSize != sizeof(CfaMpgQIAutoPauseInf))
		{
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}

		VERIFY(RET_DMX_OK == dmx_sema_lock(prCfaMpg->hMutex,
			DMX_SEMA_OPTION_WAIT));
		mm_memcpy(pvCfaParameter, &prCfaMpg->rAutoPauseInf,
			  sizeof(prCfaMpg->rAutoPauseInf));
		u4CfaParameterSize = sizeof(CfaMpgQIAutoPauseInf);
		VERIFY(RET_DMX_OK == dmx_sema_unlock(prCfaMpg->hMutex));
		break;
	case CFA_MPG_QUERY_INF_TYPE_MUX_RATE_INF:
		if(u4CfaParameterSize != sizeof(CfaMpgQIMuxRateInf_T))
		{
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}

		VERIFY(RET_DMX_OK == dmx_sema_lock(prCfaMpg->hMutex,
			DMX_SEMA_OPTION_WAIT));
		mm_memcpy(pvCfaParameter, &prCfaMpg->rMuxRateInf,
			sizeof(prCfaMpg->rMuxRateInf));
		u4CfaParameterSize = sizeof(CfaMpgQIMuxRateInf_T);
		VERIFY(RET_DMX_OK == dmx_sema_unlock(prCfaMpg->hMutex));
		break;
	case CFA_MPG_QUERY_INF_TYPE_LAST_MPEG_PTS_INF:
		if(u4CfaParameterSize != sizeof(CfaMpgQIFirstMpgPtsInf))
		{
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}

		VERIFY(RET_DMX_OK == dmx_sema_lock(prCfaMpg->hMutex,
			DMX_SEMA_OPTION_WAIT));
		mm_memcpy(pvCfaParameter, &prCfaMpg->rLastMpgPtsInf,
			  sizeof(prCfaMpg->rLastMpgPtsInf));
		u4CfaParameterSize = sizeof(CfaMpgQIFirstMpgPtsInf);
		VERIFY(RET_DMX_OK == dmx_sema_unlock(prCfaMpg->hMutex));
		break;
	default:
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	MM_RETURN(RET_DMX_OK);
}


/*< [IN] input splitter Handle*/
/*< [IN] CFA function id, set or get id, it shall be defined by CFA and LPE*/
/*< [IN] input CFA private data*/
/*< [OUT] The parameter of this FID, it shall be defined by CFA and LPE*/
/*< [IN] The size of this parameter (of this FID), it shall be defined by CFA and LPE*/
static MRESULT CfaMpgSetGeneral(void *pvSptHdl, u32 u4CfaFID, void *pvPrivData,
				void *pvCfaParameter, u32 u4CfaParameterSize)
{
	CfaMpgInst *prCfaMpg = NULL;

	if (!pvPrivData) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("CfaMpgSetGeneral pvPrivData NULL\r\n"));
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);
	}

	prCfaMpg = (CfaMpgInst *) pvPrivData;
	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
		TEXT("<CFA MPG> CfaMpgSetGeneral\n"));

	MM_RETURN(RET_DMX_OK);
}


/* MPG CFA callback for each subpicture is demuxed*/
/* @return TRUE - this subpicture should be retained in SP FIFO.*/
/*	   FALSE - this subpicture should be removed from SP FIFO.*/
/*< [IN] handle of Fdmx*/
/*< [IN/OUT] SubPicture info, @see Spt2CfaPicInfo*/
/*< [IN] pointer to CfaMpgInst*/
static MRESULT CfaMpgFillSubPicInfo(void *pvSptHdl, Spt2CfaSubPicInfo *ptSubPicInfo,
					void *pvPrivData)
{
	CfaMpgInst *prCfaMpg = NULL;

	if (!ptSubPicInfo) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("<CFA MPG> CfaMpgFillSubPicInfo: ptSubPicInfo NULL\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}


	if (!pvPrivData) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("<CFA MPG> CfaMpgFillSubPicInfo: pvPrivData NULL\n"));
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);
	}

	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
		TEXT("<CFA MPG> CfaMpgFillSubPicInfo\n"));
	prCfaMpg = (CfaMpgInst *) pvPrivData;

	ptSubPicInfo->u8StartPts = prCfaMpg->u8PrsPts;
	ptSubPicInfo->u8EndPts = INVALID_TIMESTAMP;
	ptSubPicInfo->u8Dts = prCfaMpg->u8PrsDts;
	ptSubPicInfo->u8CusInf1 = prCfaMpg->u8PicPckPos;

	prCfaMpg->u8PrsDts = INVALID_TIMESTAMP;
	prCfaMpg->u8PrsPts = INVALID_TIMESTAMP;

	MM_RETURN(RET_DMX_OK);
}


/* MPG CFA callback for each AU is demuxed*/
/* @return TRUE*/
/*< [IN] handle of Fdmx*/
/*< [IN/OUT] AU info, @see Spt2CfaPicInfo*/
/*< [IN] pointer to CfaMpgInst*/
static MRESULT CfaMpgFillAUInfo(void *pvSptHdl, void *pvAUInfo, void *pvAUExtInfo, void *pvPrivData)
{
	CfaMpgInst *prCfaMpg = NULL;
	u64 u8PTS = INVALID_TIMESTAMP;
	u64 u8DTS = INVALID_TIMESTAMP;
	u32 u4Temp = 0, u4Index = 0;

	if (!pvAUInfo) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("<CFA MPG> CfaMpgFillAUInfo: pvAUInfo NULL\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!pvPrivData) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("<CFA MPG> CfaMpgFillAUInfo:pvPrivData NULL\n"));
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);
	}

	prCfaMpg = (CfaMpgInst *) pvPrivData;
	MMATE_CHECK_POINTER(prCfaMpg);
	MMATE_CHECK_STRUCT(prCfaMpg->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaMpg->rStrmInf);

	if ((prCfaMpg->fgCMDQTx) && (prCfaMpg->fgSupportHibitRt) && (!prCfaMpg->fgCMDQFirstFill)) {
		prCfaMpg->eCMDQCurPrsPktType = prCfaMpg->eCurPrsPktType;
		prCfaMpg->eCurPrsPktType = CFA_MPG_PRS_BIT_STRM_TYPE_V;
		prCfaMpg->fgCMDQFirstFill = TRUE;
	}

	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
		TEXT("<CFA MPG> eCurPrsPktType %d\r\n"), prCfaMpg->eCurPrsPktType);
	switch (prCfaMpg->eCurPrsPktType) {
	case CFA_MPG_PRS_BIT_STRM_TYPE_V:
		((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Dts = prCfaMpg->u8PrsDts;
		((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Pts = prCfaMpg->u8PrsPts;
		DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("Fill PTS %lld\r\n"), prCfaMpg->u8PrsPts);
		if (prCfaMpg->eCfaMpgMediumType == CFA_MPG_MED_TYPE_FILE) {
			((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.eDiscType = DT_DATADISC;
			prCfaMpg->ptrLastVideoAUSAddr =
				((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.ptrSAddr;
		} else if (prCfaMpg->eCfaMpgMediumType == CFA_MPG_MED_TYPE_NONE) {
			((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.eDiscType = DT_UNKNOW;
		} else {
			/*do nothing*/
		}

		if ((prCfaMpg->ucVidBound != 0xff) && (prCfaMpg->ucVidBound > 0) &&
			(prCfaMpg->ucPrsVidStId == prCfaMpg->ucDecVidStId) &&
			(CFA_MPG_PRS_BIT_STRM_TYPE_V & prCfaMpg->u4CurPrsFlg)) {
			Spt4CfaPTSNotify(pvSptHdl, prCfaMpg->u8PrsPts);
		}

		if (prCfaMpg->u8PrsPts != INVALID_TIMESTAMP) {
			u64 u8Gap = 0;

			if (prCfaMpg->u8PrsPts > prCfaMpg->u8SCR)
				u8Gap = prCfaMpg->u8PrsPts - prCfaMpg->u8SCR;
			 else
				u8Gap = prCfaMpg->u8SCR - prCfaMpg->u8PrsPts;

			if (u8Gap > CFA_STC_CLK)
				prCfaMpg->u8SCR = prCfaMpg->u8PrsPts;
		}
		if (prCfaMpg->ptrLastVideoAUSAddr !=
			((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.ptrSAddr) {
			prCfaMpg->fgCalPtsByScr = TRUE;
		}
		if ((prCfaMpg->u8PrsPts != INVALID_TIMESTAMP) && (prCfaMpg->fgCalPtsByScr))
			prCfaMpg->fgCalPtsByScr = FALSE;

		if ((((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u4VType & (u32)0x000000FF) > 0) {
			if ((prCfaMpg->fgFindIFrame)
				&& ((((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u4VType & (u32)0x000000FF)
				> 1)) {
				prCfaMpg->fgIFrameEnd = TRUE;
			}
#if CFA_MPG_HIGH_BIT_RATE_HANDLE
			if ((prCfaMpg->fgSupportHibitRt) && (prCfaMpg->u4PtsArrayIndex > 0)) {
				for (u4Temp = 0; u4Temp < prCfaMpg->u4PtsArrayIndex; u4Temp++) {
					if (prCfaMpg->au8FileOffSet[u4Temp] <=
						((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Offset) {
						((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Pts =
							prCfaMpg->au8FileOffSetPts[u4Temp];
					}
				}
			}
#endif
			if (fgIsIType(((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u4VType))
				prCfaMpg->fgFindIFrame = TRUE;
		}
		prCfaMpg->ptrLastVideoAUSAddr =
			((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.ptrSAddr;
		if ((prCfaMpg->fgIsSeek)
			&& (((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Pts != INVALID_TIMESTAMP)
			&& (!prCfaMpg->fgHasAud)) {
			prCfaMpg->fgIsSeek = FALSE;
			prCfaMpg->i8DeltaPts =
				(prCfaMpg->u8TimeCode / 10000) * 90 -
				((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Pts;

			DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					(TEXT("<CFA_MPG> Video CfaMpgFillAUInfo:")
					TEXT("TimeCode %lld, PTS %lld, i8DeltaPts %lld\n")),
					(prCfaMpg->u8TimeCode / 10000) * 90,
					((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Pts,
					prCfaMpg->i8DeltaPts);
		}
		((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u4Duration =
			prCfaMpg->u4VideoFrameDuration;
		if ((INVALID_TIMESTAMP != ((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Pts) &&
			((prCfaMpg->i8DeltaPts < -CFA_MPG_PTS_1S)
			 || (prCfaMpg->i8DeltaPts > CFA_MPG_PTS_1S))) {
			if ((prCfaMpg->i8DeltaPts < 0) &&
				(abs(prCfaMpg->i8DeltaPts) > ((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts)){
				DmxLogT(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					TEXT("<CFA MPG> Video CfaMpgFillAUInfo abs(prCfaMpg->i8DeltaPts)\n"));
				break;
			}
			else
			{
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts += prCfaMpg->i8DeltaPts;
			}
		}
		if (fgIsIType(((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u4VType)
			&& (prCfaMpg->fgSetJumpRange)
			&& (INVALID_TIMESTAMP == ((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Pts)) {
			/*only jump need adjust pts*/
			((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Pts =
				prCfaMpg->rCfaRange.u8SeekTime * CFA_MPG_PTS_1S / 10000000;
			prCfaMpg->fgSetJumpRange = FALSE;
			DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					TEXT("<CFA MPG> CfaMpgFillAUInfo: PTS %lld\n"),
					((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Pts);
		}

		DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("<CFA MPG> Video CfaMpgFillAUInfo: PTS %llds\n"),
				((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Pts/90000);

		break;

	case CFA_MPG_PRS_BIT_STRM_TYPE_A:
		prCfaMpg->fgHasAud = TRUE;
		for (u4Index = 0; u4Index < prCfaMpg->u4AstNs; u4Index++) {
			if (prCfaMpg->u2PrsAudStId == prCfaMpg->arAudInf[u4Index].u2DecAstId)
				u8PTS = prCfaMpg->arAudInf[u4Index].u8AudPrsPts;
			}

		/* Fix BDP00113945*/
		if ((prCfaMpg->eCfaMpgMediumType == CFA_MPG_MED_TYPE_FILE) &&
			(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_NONE) &&
			(prCfaMpg->ucAudBound != 0xff) &&
			(prCfaMpg->u4VOBU_V_S_PTM != 0) && (prCfaMpg->u4VOBU_V_E_PTM != 0))	{
			/*Fix BDP117381,if audio pts - stcoffset < vobu start PTM - 4s,
			set the pts to invalid, @pingzhao, 2008/11/3 */
			if (((u8PTS - prCfaMpg->i8StcOffset + 4 * CFA_STC_CLK) <
				prCfaMpg->u4VOBU_V_S_PTM)
				|| ((u8PTS - prCfaMpg->i8StcOffset) > prCfaMpg->u4VOBU_V_E_PTM)) {
				u8PTS = INVALID_TIMESTAMP;
			}
		}
		((AU_AUDIO *) pvAUInfo)->rAUInfo.rInfo.u8Pts = u8PTS;

		if ((prCfaMpg->ucAudBound != 0xff) && (prCfaMpg->ucAudBound > 0) &&
			(prCfaMpg->u2PrsAudStId == prCfaMpg->u2DecAudStId) &&
			(CFA_MPG_PRS_BIT_STRM_TYPE_A & prCfaMpg->u4CurPrsFlg)) {
			Spt4CfaPTSNotify(pvSptHdl, u8PTS);
		}

		if (u8PTS != INVALID_TIMESTAMP) {
			u64 u8Gap = 0;

			if (u8PTS > prCfaMpg->u8SCR)
				u8Gap = u8PTS - prCfaMpg->u8SCR;
			 else
				u8Gap = prCfaMpg->u8SCR - u8PTS;

			if (u8Gap > CFA_STC_CLK)
				prCfaMpg->u8SCR = u8PTS;
		}
		if ((prCfaMpg->fgIsSeek)
			&& (((AU_AUDIO *) pvAUInfo)->rAUInfo.rInfo.u8Pts != INVALID_TIMESTAMP)) {
			prCfaMpg->fgIsSeek = FALSE;
			prCfaMpg->i8DeltaPts =
				(prCfaMpg->u8TimeCode / 10000) * 90 -
				((AU_AUDIO *) pvAUInfo)->rAUInfo.rInfo.u8Pts;

			DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					TEXT("<CFA_MPG> Audio CfaMpgFillAUInfo: TimeCode %lld,")
					TEXT("PTS %lld, i8DeltaPts %lld\n"),
					(prCfaMpg->u8TimeCode / 10000) * 90,
					((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Pts,
					prCfaMpg->i8DeltaPts);
		}
		if ((INVALID_TIMESTAMP != ((AU_AUDIO *) pvAUInfo)->rAUInfo.rInfo.u8Pts) &&
			((prCfaMpg->i8DeltaPts < -CFA_MPG_PTS_1S)
			 || (prCfaMpg->i8DeltaPts > CFA_MPG_PTS_1S))) {
			((AU_AUDIO *) pvAUInfo)->rAUInfo.rInfo.u8Pts += prCfaMpg->i8DeltaPts;
		}
		DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("<CFA MPG> Audio CfaMpgFillAUInfo: PTS %llds, PckPos 0x%llx\n"),
				((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Pts/90000, prCfaMpg->u8PckPos);

		break;

	case CFA_MPG_PRS_BIT_STRM_TYPE_SP0:
		for (u4Index = 0; u4Index < prCfaMpg->u4SpstNs; u4Index++) {
			if (prCfaMpg->ucPrsSpStId == prCfaMpg->arSPInf[u4Index].ucDecSpstId) {
				u8PTS = prCfaMpg->arSPInf[u4Index].u8SpuPrsPts;
				u8DTS = prCfaMpg->arSPInf[u4Index].u8SpuPrsDts;
				u4Temp = u4Index;
			}
		}

		((AU_SP *) pvAUInfo)->rAUInfo.rInfo.u8StartPts = u8PTS;
		if (prCfaMpg->arSPInf[u4Temp].u2SpuTotalSz !=
			((AU_SP *) pvAUInfo)->rAUInfo.rInfo.u4Size) {
			((AU_SP *) pvAUInfo)->rAUInfo.rInfo.u8StartPts = INVALID_TIMESTAMP;
		}

		((AU_SP *) pvAUInfo)->rAUInfo.rInfo.u8EndPts = INVALID_TIMESTAMP;
		((AU_SP *) pvAUInfo)->rAUInfo.rInfo.u8Dts = u8DTS;
		if (u8PTS != INVALID_TIMESTAMP) {
			u64 u8Gap = 0;

			if (u8PTS > prCfaMpg->u8SCR)
				u8Gap = u8PTS - prCfaMpg->u8SCR;
			 else
				u8Gap = prCfaMpg->u8SCR - u8PTS;

			if (u8Gap > CFA_STC_CLK)
				prCfaMpg->u8SCR = u8PTS;
		}
		break;

	case CFA_MPG_PRS_BIT_STRM_TYPE_NV:
		/*Should parse end ptm to ese for frm acc(Current NV Pack)*/

		if (prCfaMpg->u4VOBU_V_S_PTM) {
			u64 u8Gap = 0;

			if (prCfaMpg->u4VOBU_V_S_PTM > prCfaMpg->u8SCR)
				u8Gap = prCfaMpg->u4VOBU_V_S_PTM - prCfaMpg->u8SCR;
			 else
				u8Gap = prCfaMpg->u8SCR - prCfaMpg->u4VOBU_V_S_PTM;

			if (u8Gap > CFA_STC_CLK)
				prCfaMpg->u8SCR = prCfaMpg->u4VOBU_V_S_PTM;
		}
		break;
	case CFA_MPG_PRS_BIT_STRM_TYPE_HD:
		DmxLogT(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("<Wrtang>CFAFillAUInfo---HD\n"));
		break;
	default:
		MM_RETURN(RET_DMX_PARAM_WRONG);
		break;
	}

	prCfaMpg->fgFillAU = TRUE;
	/*After fill AU, Pts/Dts reset*/
	prCfaMpg->u8PrsDts = INVALID_TIMESTAMP;
	prCfaMpg->u8PrsPts = INVALID_TIMESTAMP;
	MMATE_CHECK_POINTER(prCfaMpg);
	MMATE_CHECK_STRUCT(prCfaMpg->rCfaRange);

	MM_RETURN(RET_DMX_OK);
}



static MRESULT CfaMpgTxAudHDRInfo(void *pvSptHdl, u32 u4TxUID, void *pvPrivData)
{
	MM_RETURN(RET_DMX_UNSUPPORT);
}

/* Splitter notify Rebuf*/
/* @return TRUE*/
/*< [IN] handle of Fdmx*/
/*< [IN] flag to indicate Rebuf*/
/*< [IN] pointer to CfaMpgInst*/
static MRESULT CfaMpgRebuf(void *pvSptHdl, bool fgRebuf, void *pvPrivData)
{
	CfaMpgInst *prCfaMpg = NULL;

	if (!pvPrivData) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("CfaMpgRebuf pvPrivData NULL\r\n"));
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);
	}

	prCfaMpg = (CfaMpgInst *) pvPrivData;
	MMATE_CHECK_POINTER(prCfaMpg);

	if (fgRebuf) {
		prCfaMpg->u4RdyDataSz = 0;
		prCfaMpg->pucHdrBufRp = NULL;
	}

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaMpgSetJumpRange(void *pvSptHdl, void *pvJmpRange, void *pvPrivData)
{
	CfaMpgInst *prCfaMpg = NULL;
	CfaMpgKeyFrameRange *prCfaMpgKeyFrameRange = NULL;

	if (!pvPrivData) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("CfaMpgSetJumpRange pvPrivData NULL\r\n"));
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);
	}
	if (!pvJmpRange) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("CfaMpgSetJumpRange pvJmpRange NULL\r\n"));
		MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);
	}

	prCfaMpg = (CfaMpgInst *) pvPrivData;
	prCfaMpgKeyFrameRange = (CfaMpgKeyFrameRange *) pvJmpRange;

	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("[CFA_MPG] CfaMpgSetJumpRange::SA-EA[0x%llx -0x%llx]\r\n"),
			prCfaMpgKeyFrameRange->rCfaMpgRange.u8Sa,
			prCfaMpgKeyFrameRange->rCfaMpgRange.u8Ea);

	prCfaMpg->fgSetJumpRange = TRUE;
	CfaMpgSetRange(pvSptHdl, (void *) (&(prCfaMpgKeyFrameRange->rCfaMpgRange)), pvPrivData, TRUE);

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaMpgGetParamSize(void *pvSptHdl, u32 u4ParamID,
				  void *pvPrivData, void *pvCfaParam, u32 u4CfaParamSz)
{
	MRESULT mrResult = RET_DMX_OK;

	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
		TEXT("[CFA MPG] Entry CfaMkvGetParamSize!\r\n"));

	switch (u4ParamID) {
	case CFA_PARAM_ID_JUMP_INFO_SIZE:
		{
			if ((NULL == pvCfaParam) || ((u4CfaParamSz) < sizeof(u32))) {
				mrResult = RET_DMX_PARAM_WRONG;
			} else {
				u32 *pu4Tmp = (u32 *) pvCfaParam;
				*pu4Tmp = sizeof(CfaMpgKeyFrameRange);
				DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
						TEXT("[CFA_MPG] CfaMpgKeyFrameRange is %d(Bytes)!\r\n"),
						*pu4Tmp);
			}
			break;
		}

	default:
		mrResult = RET_DMX_PARAM_WRONG;
		break;
	}

	MM_RETURN(mrResult);
}

static MRESULT CfaMpgProcCliCmd(void *pvSptHdl, E_DMX_CFA_CLI_TYPE_T eCliType, /*< [IN] Cfa Cli Command*/
				u32 arg1,
				u32 arg2, u32 arg3, const char *szParam, void *pvPrivData)
{
	MRESULT mrRet = RET_DMX_OK;
	CfaMpgInst *prCfaMpg = NULL;

	prCfaMpg = (CfaMpgInst *) pvPrivData;

	switch (eCliType) {
	case DMX_CFA_CLI_CMD_TURN_ONOFF_LOG:
		{
			bool fgEnable = TRUE;
		/**
		* arg1: u4OnOff
		* arg2: LogLevel(T, E, W, D)
		* arg3: Module Log Level
		**/
			if (0 == arg1)
				fgEnable = FALSE;

			DmxLogT(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("CfaMpgProcCliCmd -- fgEnable: %d, Loglvl: %d, ModLogLvl: 0x%08x \r\n"),
				arg1, arg2, arg3);

			DmxLogEnable(fgEnable, arg2, DMX_MOD_CFA_MPG, arg3);
		}
		break;
	case DMX_CFA_CLI_CMD_DUMP_INFO:
		{
			DmxLogT(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("Cfa MPG Instance(handle is 0x%x)")
				TEXT(" Info list as follow: \r\n"),
				prCfaMpg);
			DmxLogT(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("Current Analyse State is %d, \r\n"),
				prCfaMpg->eCurCfaMpgAnaSt);
			DmxLogT(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("Current Analyse Position is 0x%08x%08x, "),
				(u32) ((prCfaMpg->u8Ca) >> 32), (u32) (prCfaMpg->u8Ca));
		}
		break;
	default:
		break;
	}

	MM_RETURN(mrRet);
}

#ifdef CONFIG_COMPAT
#include <linux/compat.h>

typedef struct{
	/* < Flag indicates if the stream is only MPEG vide stream only */
	bool fgMpgVidStrmOnly;
	/* <  MPG CFA medium type */
	__u32 u4CfaMpgMediumType;
	/* < flag indicates if "don't" support auto pause, only for VCD, default: 0 is supported */
	bool fgNoSupportVCDAutoPause;
	__u16 u2DiscType;	/* < for disc type, -r/-rw */
	__u16 u2DiscCpsType;	/* < for disc cprm information. */
	__u8 ucAstMode;
	__u16 u2VideoType;	/* < for CinemaNow only, config video type by LPE */
	__u16 u2AudioType;	/* < for CinemaNow only, config audio type by LPE */
	__u16 u2DiscSectorSz;	/* <    MPG CFA sector size */

	__u32 u4VideoFps_n;
	__u32 u4VideoFps_d;

	bool fgHighBitrate;
	__u64 u8VidSpecDataOfst;
	__u32 u4VidSpecDataLen;
	compat_caddr_t pu1VidSpecData;
} CfaMpgCfg32;

#if CFA_MPG_DVD_AUDIO_DEFINE_SUPPORT
	typedef struct {
		MpgParseMode eMpgParseMode;	/* < cfa parse mode setting */
		__u16 u2ParseSec;	/* < parse data length(measured in second) */
		__u16 u2SkipSec;	/* < skip data length(measured in second) */
	} MpgRangMode32;
#endif

typedef struct {
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKStart;
#endif
	__u64 u8Sa;	/* < start file offset (value range is __u32) */
	__u64 u8Ea;	/* < end file offset (value range is __u32) */
	/* < Video start file offset (value range is __u32), default: 0xFFFFFFFFFFFFFFFF, ignore. */
	__u64 u8VidSa;
	/* < Audio start file offset (value range is __u32), default: 0xFFFFFFFFFFFFFFFF, ignore. */
	__u64 u8AudSa;
	__u64 u8SPSa;	/* < start file offset (value range is __u32), default: 0xFFFFFFFFFFFFFFFF, ignore. */
#if CFA_MPG_DVD_AUDIO_DEFINE_SUPPORT
	MpgRangMode32 rMpgRangMode;	/* < mpeg cfa lpcm/pcm parse mode */
	MpgDVDAType eMpgDVDAType;	/* <dvd audio type */
#endif
#if (CFA_MPG_SUPPORT_DVD_VOBU_STILL_AUTOPAUSE)
	bool fgVobuStill;	/* < Vobu still case, normal title: 0, Vobu still: 1 */
#endif

	/* PTS adjustment */
	MpgPtsAdjustType ePtsAdjustType;
	__s64 i8AdjustValue;	/* for MPG_PTS_ADJUST_BY_VALUE */
	__u8 u1ApRatFlag;

	/* for -VR AspRatio */
	__u8 u1PassAspRatFlag;
	__u8 ucAspRatioFromLpe;

	/* Range extension */
	MpgRangeEx eExType;
	MpgRangeExParm rExParm;

#if CFA_MPG_SEAMLESS_2ND_RANGE_SUPPORT
	/* Add by pingzhao,  for BDP00018159, 2008/12/30 */
	bool fgSeamless2ndRange;
#endif

	/* add for +vr error handle, parse end after find first i frame */
	bool fgParseEndAfIFrame;

#if CFA_MPG_KEEP_VOB_PTM
	bool fgKeepVobPtm;
#endif

	bool fgHighBitrate; /* CFA_MPG_HIGH_BIT_RATE_HANDLE */

	bool fgIsSeek;
	__u64 u8SeekTime;
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKEnd;
#endif
} CfaMpgRange32;

typedef struct  {
	CfaMpgRange32 rCfaMpgRange;
} CfaMpgKeyFrameRange32;

static long CfaMpgCompatConfigCalcSz(CfaMpgCfg32 __user *usr_ptr32,
	__u32 *pu4OutSz)
{
	__u32 u4TotalSz = 0;
	__u32 u4HeaderLen = 0;
	__u32 i = 0;

	if ((NULL == usr_ptr32) || (NULL == pu4OutSz)) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	u4TotalSz += CFA_ALIGN_SZ((__u32)sizeof(CfaMpgCfg), sizeof(uintptr_t));

	if (0 != get_user(u4HeaderLen,	&(usr_ptr32->u4VidSpecDataLen))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in get_user(u4VidSpecDataLen)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	u4TotalSz += CFA_ALIGN_SZ((__u32)u4HeaderLen, sizeof(uintptr_t));
		
	*pu4OutSz = u4TotalSz;

	return 0;
}

static long CfaMpgCompatConfig(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaMpgCfg __user *usr_ptr = NULL;
	CfaMpgCfg32 __user *usr_ptr32 = (CfaMpgCfg32 __user *)prInfo->usr_ptr32;
	__u8 __user *pu1UsrBufAddr = NULL;
	__u8 __user *pu1NextBufAddr = NULL;
	__u32 u4TotalSz = 0;
	__u32 u4UseSz = 0;
	compat_caddr_t compatVidSpecData = 0;
	long ret = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaMpgCfg32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz(0x%08x/0x%08x).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, sizeof(CfaMpgCfg32), prInfo->buf_sz);
		return -EINVAL;
	}

	if (0 != CfaMpgCompatConfigCalcSz(usr_ptr32, &u4TotalSz)) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaFlvCompatConfigCalcSz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	//pu1UsrBufAddr = (__u8 __user *)compat_alloc_user_space(u4TotalSz);
	DMX_NewMemory(u4TotalSz,pu1UsrBufAddr);

	if (NULL == pu1UsrBufAddr) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(pu1UsrBufAddr, 0, u4TotalSz);

	usr_ptr = (CfaMpgCfg __user *)pu1UsrBufAddr;

	pu1NextBufAddr = pu1UsrBufAddr + CFA_ALIGN_SZ((__u32)sizeof(CfaMpgCfg), sizeof(uintptr_t));
	u4UseSz += CFA_ALIGN_SZ((__u32)sizeof(CfaMpgCfg), sizeof(uintptr_t));
	if (u4UseSz > u4TotalSz)
	{
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in u4UseSz > u4TotalSz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -ENOMEM;
	}
	if (copy_from_user(&(usr_ptr->fgMpgVidStrmOnly), &(usr_ptr32->fgMpgVidStrmOnly), sizeof(bool))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(fgMpgVidStrmOnly).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u4CfaMpgMediumType), &(usr_ptr32->u4CfaMpgMediumType), sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4CfaMpgMediumType).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->fgNoSupportVCDAutoPause), &(usr_ptr32->fgNoSupportVCDAutoPause), sizeof(bool))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(fgNoSupportVCDAutoPause).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u2DiscType), &(usr_ptr32->u2DiscType), sizeof(__u16))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u2DiscType).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u2DiscCpsType), &(usr_ptr32->u2DiscCpsType), sizeof(__u16))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u2DiscCpsType).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
  
	if (copy_from_user(&(usr_ptr->ucAstMode), &(usr_ptr32->ucAstMode), sizeof(__u8))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(ucAstMode).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
  
	if (copy_from_user(&(usr_ptr->u2VideoType), &(usr_ptr32->u2VideoType), sizeof(__u16))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u2VideoType).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u2AudioType), &(usr_ptr32->u2AudioType), sizeof(__u16))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u2AudioType).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u2DiscSectorSz), &(usr_ptr32->u2DiscSectorSz), sizeof(__u16))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u2DiscSectorSz).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u4VideoFps_n), &(usr_ptr32->u4VideoFps_n), sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4VideoFps_n).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->u4VideoFps_d), &(usr_ptr32->u4VideoFps_d), sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4VideoFps_d).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->fgHighBitrate), &(usr_ptr32->fgHighBitrate), sizeof(bool))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(fgHighBitrate).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
  
	if (copy_from_user(&(usr_ptr->u8VidSpecDataOfst), &(usr_ptr32->u8VidSpecDataOfst), sizeof(__u64))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8VidSpecDataOfst).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
  
	if (copy_from_user(&(usr_ptr->u4VidSpecDataLen), &(usr_ptr32->u4VidSpecDataLen), sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4VidSpecDataLen).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
  
	if ((0 < usr_ptr32->u4VidSpecDataLen) && (0 == usr_ptr32->pu1VidSpecData)) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail for no Video SpecData, but header len(%d) > 0.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr->u4VidSpecDataLen);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}

	if (0 != usr_ptr32->pu1VidSpecData) {
		void *pvVidSpecData = NULL;

		usr_ptr->pu1VidSpecData =  (__force void *)pu1NextBufAddr;

		if (NULL == usr_ptr->pu1VidSpecData) {
			DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("%s line %d fail in alloc compat user space(pu1VidSpecData).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -ENOMEM;
		}
		u4UseSz += CFA_ALIGN_SZ((__u32)usr_ptr->u4VidSpecDataLen, sizeof(uintptr_t));
		if (u4UseSz > u4TotalSz)
		{
			DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("%s line %d fail in u4UseSz > u4TotalSz.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -ENOMEM;
		}
		mm_memset(usr_ptr->pu1VidSpecData, 0, usr_ptr->u4VidSpecDataLen);
		pu1NextBufAddr = pu1NextBufAddr + CFA_ALIGN_SZ((__u32)usr_ptr->u4VidSpecDataLen, sizeof(uintptr_t));

		if (get_user(compatVidSpecData, &(usr_ptr32->pu1VidSpecData))) {
			DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_in_user(pu1VidSpecData).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}

    if (0 == compatVidSpecData) {
			DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_in_user(compatVidSpecData).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}

    pvVidSpecData = compat_ptr(compatVidSpecData);

    if (!access_ok(VERIFY_READ, pvVidSpecData, usr_ptr->u4VidSpecDataLen)) {
			DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("%s line %d fail in access_ok(pvVidSpecData).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}

	if (copy_from_user((void __user *)usr_ptr->pu1VidSpecData,
			pvVidSpecData, usr_ptr->u4VidSpecDataLen)) {
			DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_in_user(pu1VidSpecData).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}
	}

	*pfgIsUserMem = FALSE;
	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaMpgCfg);

	return 0;
}

static long CfaMpgCompatRangeInfo(CfaMpgRange __user *usr_ptr,CfaMpgRange32 __user *usr_ptr32)
{

	#ifdef MM_ATE_CHECK
	if (copy_in_user(&(usr_ptr->u4MMATECHKStart), &(usr_ptr32->u4MMATECHKStart), sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4MMATECHKStart).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	#endif

	if (copy_in_user(&(usr_ptr->u8Sa), &(usr_ptr32->u8Sa), sizeof(__u64))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(usr_ptr->u8Ea), &(usr_ptr32->u8Ea), sizeof(__u64))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(usr_ptr->u8VidSa), &(usr_ptr32->u8VidSa), sizeof(__u64))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(usr_ptr->u8AudSa), &(usr_ptr32->u8AudSa), sizeof(__u64))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(usr_ptr->u8SPSa), &(usr_ptr32->u8SPSa), sizeof(__u64))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	
	#if CFA_MPG_DVD_AUDIO_DEFINE_SUPPORT
	if (copy_in_user(&(usr_ptr->rMpgRangMode.eMpgParseMode),
		&(usr_ptr32->rMpgRangMode.eMpgParseMode), sizeof(MpgParseMode))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(usr_ptr->rMpgRangMode.u2ParseSec),
		&(usr_ptr32->rMpgRangMode.u2ParseSec), sizeof(__u16))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(usr_ptr->rMpgRangMode.u2SkipSec),
		&(usr_ptr32->rMpgRangMode.u2SkipSec), sizeof(__u16))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(usr_ptr->eMpgDVDAType),
		&(usr_ptr32->eMpgDVDAType), sizeof(MpgDVDAType))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	#endif

	#if CFA_MPG_SUPPORT_DVD_VOBU_STILL_AUTOPAUSE
	if (copy_in_user(&(usr_ptr->fgVobuStill), &(usr_ptr32->fgVobuStill), sizeof(bool))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	#endif

	if (copy_in_user(&(usr_ptr->ePtsAdjustType), &(usr_ptr32->ePtsAdjustType),
		sizeof(MpgPtsAdjustType))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(usr_ptr->i8AdjustValue), &(usr_ptr32->i8AdjustValue), sizeof(__s64))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(usr_ptr->u1ApRatFlag), &(usr_ptr32->u1ApRatFlag), sizeof(__u8))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(usr_ptr->u1PassAspRatFlag), &(usr_ptr32->u1PassAspRatFlag), sizeof(__u8))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  	if (copy_in_user(&(usr_ptr->ucAspRatioFromLpe),
			&(usr_ptr32->ucAspRatioFromLpe), sizeof(__u8))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(usr_ptr->eExType), &(usr_ptr32->eExType), sizeof(MpgRangeEx))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(usr_ptr->rExParm), &(usr_ptr32->rExParm), sizeof(MpgRangeExParm))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	
	#if CFA_MPG_SEAMLESS_2ND_RANGE_SUPPORT
	if (copy_in_user(&(usr_ptr->fgSeamless2ndRange),
		&(usr_ptr32->fgSeamless2ndRange), sizeof(bool))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	#endif

	if (copy_in_user(&(usr_ptr->fgParseEndAfIFrame),
		&(usr_ptr32->fgParseEndAfIFrame), sizeof(bool))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	#if CFA_MPG_KEEP_VOB_PTM
	if (copy_in_user(&(usr_ptr->fgKeepVobPtm), &(usr_ptr32->fgKeepVobPtm), sizeof(bool))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	#endif
	
	if (copy_in_user(&(usr_ptr->fgHighBitrate), &(usr_ptr32->fgHighBitrate), sizeof(bool))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(usr_ptr->fgIsSeek), &(usr_ptr32->fgIsSeek), sizeof(bool))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(usr_ptr->u8SeekTime), &(usr_ptr32->u8SeekTime), sizeof(__u64))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	#ifdef MM_ATE_CHECK
	if (copy_in_user(&(usr_ptr->u4MMATECHKEnd), &(usr_ptr32->u4MMATECHKEnd), sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Sa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	#endif

	return 0;
}
static long CfaMpgCompatRange(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaMpgRange __user *usr_ptr = NULL;
	CfaMpgRange32 __user *usr_ptr32 = (CfaMpgRange32 __user *)prInfo->usr_ptr32;
	long ret = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaMpgRange32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	usr_ptr = (CfaMpgRange *)compat_alloc_user_space(sizeof(CfaMpgRange));

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaMpgRange));

	ret = CfaMpgCompatRangeInfo(usr_ptr, usr_ptr32);

	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaMpgCompatRangeInfo.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}

	*pfgIsUserMem = TRUE;
	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaMpgRange);

	return 0;
}

static long CfaMpgCompatJumpRange(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaMpgKeyFrameRange __user *usr_ptr = NULL;
	CfaMpgKeyFrameRange32 __user *usr_ptr32 = (CfaMpgKeyFrameRange32 __user *)prInfo->usr_ptr32;
	long ret = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaMpgKeyFrameRange32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	usr_ptr = (CfaMpgKeyFrameRange *)compat_alloc_user_space(sizeof(CfaMpgKeyFrameRange));

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaMpgKeyFrameRange));

	ret = CfaMpgCompatRangeInfo(&(usr_ptr->rCfaMpgRange), &(usr_ptr32->rCfaMpgRange));

	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaMpgCompatRangeInfo.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}

	*pfgIsUserMem = TRUE;
	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaMpgKeyFrameRange);

	return 0;
}

static int CfaMpgProcCompat(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	long ret = 0;

	if ((NULL == prInfo) || (NULL == pfgIsUserMem)) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					TEXT("%s line %d fail for invalid parameter.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	}
	switch (prInfo->type) {
		case CFA_CONFIG:
			if (prInfo->is_get) {
				DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EPERM;
			}
			ret = CfaMpgCompatConfig(prInfo, pfgIsUserMem);
			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaMpgCompatConfig.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return ret;
			}
			break;
		case CFA_RANGE:
			if (prInfo->is_get) {
				DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EPERM;
			}
			ret = CfaMpgCompatRange(prInfo, pfgIsUserMem);
			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaMpgCompatRange.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return ret;
			}
			break;
		case CFA_GEN_INFO:
			DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("%s line %d fail for don't support get info for cfa Mpg.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return -EPERM;
		case CFA_JUMP_INFO:
			if (prInfo->is_get) {
				DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EPERM;
			}
			ret = CfaMpgCompatJumpRange(prInfo, pfgIsUserMem);
			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaMpgCompatJumpRange.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return ret;
			}
			break;
		default:
			break;
	}

	return 0;
}
#endif

/* MPG CFA interface*/
const CfaIntf _rMpgCfaIntf = {
	&CfaMpgInit,
	&CfaMpgUninit,
	&CfaMpgSetRange,
	&CfaMpgEnableStrm,
	&CfaMpgSetStrmInf,
	&CfaMpgTurnOn,
	&CfaMpgTxDone,
	&CfaMpgGetCurPos,
	&CfaMpgFillPicInfo,
	&CfaMpgConfigure,
	&CfaMpgSetInqTypes,
	&CfaMpgGetGeneral,
	&CfaMpgSetGeneral,
	&CfaMpgFillSubPicInfo,
	&CfaMpgFillAUInfo,
	&CfaMpgTxAudHDRInfo,
	&CfaMpgRebuf,
	&CfaMpgSetJumpRange,
	&CfaMpgGetParamSize,
	&CfaMpgProcCliCmd
#ifdef CONFIG_COMPAT
	,&CfaMpgProcCompat
#endif
};

/* ----------------------- Start of Public Function ------------------------ */
void *CfaMpgGetInterface(void)
{
	return (void *) &_rMpgCfaIntf;
}
