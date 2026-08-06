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



/*#pragma warning(push)*/
/*#pragma warning(disable: 4127) //disable warning C4127: conditional expression is constant*/
/*#pragma warning(disable: 4115) //disable warning C4115: named type definition in parentheses*/

#include "x_typedef.h"
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/drv_esm_if.h>
/* #include <media/atc/mm_debug.h> */

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_spt.h"
#include "cfa_macro.h"
#include "cfa_mp4.h"
#include "cfa_mp4_main.h"
#include "cfa_mp4_ana.h"
#include "cfa_mp4_util.h"
#include "cfa_mp4_state.h"
#include "mmisc.h"

/*#pragma warning(pop)*/

/*#pragma warning(disable: 4127) //disable warning C4127: conditional expression is constant*/
/*#pragma warning(disable: 4100) //disable warning C4100: unreferenced formal parameter*/

/*-----------------------------------------------------------------------------
* Name: CfaMp4Init
*
* Description:
*	   Init CFA MP4
*
* Inputs:
*
* Outputs:
*
* Returns: None
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaMp4Init(void *pvSptHdl, void **ppvCfaPrivData)
{
	CfaMp4Inst *prCfaMp4 = NULL;
	u32 u4Idx = 0;

	DMX_NewMemory(sizeof(CfaMp4Inst), prCfaMp4);

	if (NULL == prCfaMp4) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA_MP4] Alloc prCfaMp4 memory fail\r\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}
	dmx_memset(prCfaMp4, 0x00, sizeof(CfaMp4Inst));
	prCfaMp4->pu1HdrBuf = NULL;
	prCfaMp4->ptrPfrMemAddress = DMX_INVALID_UINTPTR_T;
	prCfaMp4->u4CurPrsFlg = CFA_MP4_PRS_BIT_STRM_TYPE_NONE;
	prCfaMp4->u4CurAvcEsdIndex = 0;
	prCfaMp4->u4EsdIndex = 0;
	prCfaMp4->u4EsdNums = 0;
	prCfaMp4->u1SyncBufSize = 0;
	prCfaMp4->fgTxAvcHdr = FALSE;

		/* alloc memory to save START CODE for H264 Video Codec*/
	DMX_NewHwMemory(3, prCfaMp4->pucAvcHdr);
	if (NULL == prCfaMp4->pucAvcHdr) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				 TEXT("[CFA_MP4] Alloc prCfaMp4->pucAvcHdr memory fail\r\n"));
		DMX_FreeMemory(prCfaMp4);
		MM_RETURN(RET_DMX_NO_MEM);
	}
	prCfaMp4->pucAvcHdr[0] = 0x00;
	prCfaMp4->pucAvcHdr[1] = 0x00;
	prCfaMp4->pucAvcHdr[2] = 0x01;

	/* init CFA mp4 internal parameters. */
	prCfaMp4->u4CurAudInfoId = 0;
	prCfaMp4->u4CurSubInfoId = 0;
	prCfaMp4->u4CurVidInfoId = 0;

	/* current position infomation. */
	prCfaMp4->rCurOfst.u8VidCurOfst = 0;
	prCfaMp4->rCurOfst.u8AudCurOfst = 0;
	prCfaMp4->rCurOfst.u8SubCurOfst = 0;
	prCfaMp4->u8Ea = 0;
	prCfaMp4->u8CurVPts = 0;
	prCfaMp4->u8CurVDuation = 0;

	/* Init Audio related elements */
	for (u4Idx = (u32)0; u4Idx < (u32)MAX_NS_MP4_AUD; u4Idx++) {
		dmx_memset(&(prCfaMp4->rCfaMp4AInf[u4Idx]), 0,
				sizeof(prCfaMp4->rCfaMp4AInf[u4Idx]));
		prCfaMp4->u8CurAPts[u4Idx] = 0;
		prCfaMp4->u8CurADuation[u4Idx] = 0;
		prCfaMp4->rCfaMp4AInf[u4Idx].u8CurAudTableStartSampleNo = 0;
		prCfaMp4->rCfaMp4AInf[u4Idx].u8CurAudTableEndSampleNo = 0;
		prCfaMp4->rCfaMp4AInf[u4Idx].u8CurAudTableStartChunkNo = 0;
		prCfaMp4->rCfaMp4AInf[u4Idx].fgTxAacSCDone = TRUE;
		prCfaMp4->rCfaMp4AInf[u4Idx].pucDecSpecInfo = NULL;
		prCfaMp4->rCfaMp4AInf[u4Idx].pucADTSBuf = NULL;
		prCfaMp4->rCfaMp4AInf[u4Idx].prATable = NULL;
		prCfaMp4->rCurTblPos.rTblAudPos[u4Idx].u4SttsInvaildSampleNums = 0;
		prCfaMp4->rCurTblPos.rTblAudPos[u4Idx].u4CurTableLastChunkNo = 1;
		prCfaMp4->rCurTblPos.rTblAudPos[u4Idx].u4TabelUb[STCO] = 0;
		prCfaMp4->rCurTblPos.rTblAudPos[u4Idx].u4TabelUb[STSC] = 0;
		prCfaMp4->rCurTblPos.rTblAudPos[u4Idx].u4TabelUb[STTS] = 0;
		prCfaMp4->rCurTblPos.rTblAudPos[u4Idx].u4TabelUb[STSZ] = 0;
		prCfaMp4->rCurTblPos.rTblAudPos[u4Idx].u8CurSampleNo = 1;
		prCfaMp4->rCurTblPos.rTblAudPos[u4Idx].u4SampleBufNums = 0;
		prCfaMp4->rCurTblPos.rTblAudPos[u4Idx].u4ChunkBufNums = 0;
		prCfaMp4->rCfaRange.u8AudStartChunk1stSmp[u4Idx] = 1;
		prCfaMp4->rCfaRange.u8AudPts[u4Idx] = 0;
		prCfaMp4->rCfaRange.u8AudStartChunkNo[u4Idx] = 1;
		prCfaMp4->rCfaRange.u8AudStartSampleNo[u4Idx] = 1;
		prCfaMp4->rCfaRange.u8AudEndChunkNo[u4Idx] = DMX_INVALID_UINT64;
		prCfaMp4->rCfaRange.u8AudEndSampleNo[u4Idx] = DMX_INVALID_UINT64;
		}

	/* Init Subpicture related elements */
	for (u4Idx = (u32)0; u4Idx < (u32)MAX_NS_MP4_SUB; u4Idx++) {
		dmx_memset(&(prCfaMp4->rCfaMp4SInf[u4Idx]), 0,
				sizeof(prCfaMp4->rCfaMp4SInf[u4Idx]));
		prCfaMp4->u8CurSPts[u4Idx] = 0;
		prCfaMp4->u8CurSDuation[u4Idx] = 0;
		prCfaMp4->rCfaMp4SInf[u4Idx].u8CurSubTableStartSampleNo = 0;
		prCfaMp4->rCfaMp4SInf[u4Idx].u8CurSubTableEndSampleNo = 0;
		prCfaMp4->rCfaMp4SInf[u4Idx].u8CurSubTableStartChunkNo = 0;
		prCfaMp4->rCfaMp4SInf[u4Idx].prSTable = NULL;
		prCfaMp4->rCurTblPos.rTblSubPos[u4Idx].u4SttsInvaildSampleNums = 0;
		prCfaMp4->rCurTblPos.rTblSubPos[u4Idx].u4CurTableLastChunkNo = 1;
		prCfaMp4->rCurTblPos.rTblSubPos[u4Idx].u4TabelUb[STCO] = 0;
		prCfaMp4->rCurTblPos.rTblSubPos[u4Idx].u4TabelUb[STSC] = 0;
		prCfaMp4->rCurTblPos.rTblSubPos[u4Idx].u4TabelUb[STTS] = 0;
		prCfaMp4->rCurTblPos.rTblSubPos[u4Idx].u4TabelUb[STSZ] = 0;
		prCfaMp4->rCurTblPos.rTblSubPos[u4Idx].u8CurSampleNo = 1;
		prCfaMp4->rCurTblPos.rTblSubPos[u4Idx].u4SampleBufNums = 0;
		prCfaMp4->rCurTblPos.rTblSubPos[u4Idx].u4ChunkBufNums = 0;
		prCfaMp4->rCfaRange.u8SubStartChunk1stSmp[u4Idx] = 0;
		prCfaMp4->rCfaRange.u8SubPts[u4Idx] = 0;
		prCfaMp4->rCfaRange.u8SubStartChunkNo[u4Idx] = 0;
		prCfaMp4->rCfaRange.u8SubStartSampleNo[u4Idx] = 0;
	}

	/* Init Video related elements */
	dmx_memset(&(prCfaMp4->rCfaMp4VInf), 0, sizeof(prCfaMp4->rCfaMp4VInf));
	prCfaMp4->rCfaMp4VInf.u8CurVidTableStartSampleNo = 0;
	prCfaMp4->rCfaMp4VInf.u8CurVidTableEndSampleNo = 0;
	prCfaMp4->rCfaMp4VInf.u8CurVidTableStartChunkNo = 0;
	prCfaMp4->rCfaMp4VInf.u8CurChunk1stSmpNo = 0;
	prCfaMp4->rCfaMp4VInf.u8Ofst = 0;
	prCfaMp4->rCfaMp4VInf.fgTxMp4SCDone = TRUE;
	for (u4Idx = (u32)0; u4Idx < (u32)MP4_STSD_TABLE_MAX_NUMS; u4Idx++) {
		prCfaMp4->rCfaMp4VInf.pucDecSpecInfo[u4Idx] = NULL;
		prCfaMp4->rCfaMp4VInf.pucSeqParamInfo[u4Idx] = NULL;
		prCfaMp4->rCfaMp4VInf.pucPicParamInfo[u4Idx] = NULL;
		prCfaMp4->rCfaMp4VInf.pucVPSInfo[u4Idx] = NULL;
	}
	prCfaMp4->rCfaMp4VInf.pucMpeg4CodecSC = NULL;
	prCfaMp4->rCfaMp4VInf.pucPicParamAdress = NULL;	/*for klockwork issuses*/
	prCfaMp4->rCfaMp4VInf.pucSeqParamAdress = NULL;	/*for klockwork issuses*/
	prCfaMp4->rCfaMp4VInf.puCodecSC = NULL;
	prCfaMp4->rCfaMp4VInf.pucWVc1CodecSC = NULL;
	prCfaMp4->rCfaMp4VInf.fgCodecSCDone = FALSE;
	prCfaMp4->rCurTblPos.rTblVidPos.u4SttsInvaildSampleNums = 0;
	prCfaMp4->rCurTblPos.rTblVidPos.u4CurTableLastChunkNo = 1;
	prCfaMp4->rCurTblPos.rTblVidPos.u4TabelUb[STCO] = 0;
	prCfaMp4->rCurTblPos.rTblVidPos.u4TabelUb[STSC] = 0;
	prCfaMp4->rCurTblPos.rTblVidPos.u4TabelUb[STTS] = 0;
	prCfaMp4->rCurTblPos.rTblVidPos.u4TabelUb[STSZ] = 0;
	prCfaMp4->rCurTblPos.rTblVidPos.u8CurSampleNo = 1;
	prCfaMp4->rCurTblPos.rTblVidPos.u4SampleBufNums = 0;
	prCfaMp4->rCurTblPos.rTblVidPos.u4ChunkBufNums = 0;
	prCfaMp4->fgGetDate = FALSE;
	prCfaMp4->fgPrsSeqFrameInterpolation = FALSE;
	prCfaMp4->fgPrsPreProcRange = FALSE;
	prCfaMp4->u4PrsNumBFrames = 0;
	prCfaMp4->fgHasSkipData = FALSE;
	dmx_memset(&(prCfaMp4->rSliceInf), 0, sizeof(prCfaMp4->rSliceInf));
	prCfaMp4->rCfaRange.u8VidStartChunk1stSmp = 1;
	prCfaMp4->rCfaRange.u8VidPts = 0;
	prCfaMp4->rCfaRange.u8VidStartChunkNo = 1;
	prCfaMp4->rCfaRange.u8VidStartSampleNo = 1;
	prCfaMp4->rCfaRange.u8VidEndChunkNo = DMX_INVALID_UINT64;
	prCfaMp4->rCfaRange.u8VidEndSampleNo = DMX_INVALID_UINT64;
	prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_ST_IDLE;
	prCfaMp4->eLastCfaMp4AnaSt = CFA_MP4_ANA_ST_IDLE;
	prCfaMp4->eCurPrsSampleType = CFA_MP4_PRS_BIT_STRM_TYPE_NONE;

	/*Assign cfa function pointer*/
	*ppvCfaPrivData = (void *) prCfaMp4;
	prCfaMp4->fgGetATbl = TRUE;
	DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4][%s] Exit Init\r\n"), __func__);
	MMATE_CHECK_POINTER(prCfaMp4);
	MMATE_CHECK_STRUCT(prCfaMp4->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaMp4->rCfaMp4VInf);
	MMATE_CHECK_STRUCT(prCfaMp4->rCurOfst);
	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4Uninit
*
* Description:
*	   Uninit CFA MP4
*
* Inputs:
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaMp4Uninit(void *pvSptHdl, void *pvCfaPrivData)
{
	CfaMp4Inst *prCfaMp4 = (CfaMp4Inst *) pvCfaPrivData;

	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4][%s] Enter UnInit\r\n"), __func__);

	if (NULL != prCfaMp4) {
		MMATE_CHECK_POINTER(prCfaMp4);
		MMATE_CHECK_STRUCT(prCfaMp4->rCfaRange);
		MMATE_CHECK_STRUCT(prCfaMp4->rCfaMp4VInf);
		MMATE_CHECK_STRUCT(prCfaMp4->rCurOfst);
		CfaMp4InternalFreeMem(prCfaMp4);
		DMX_FreeMemory(prCfaMp4);
	}
	DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4][%s] Exit UnInit\r\n"), __func__);
	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4SetRange
*
* Description:
*	   MP4 CFA sets demuxing range
*	   splitter will ensure that pfvSetRange is only called in "off" state.
*	   If used with MPC, the range of MPC_SCMD_SPR will be passed here
*
* Inputs:
*
* Outputs:
*
* Returns: MRESULT
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaMp4SetRange(void *pvSptHdl, void *pvRange, void *pvPrivData, bool fgIsUserMem)
{
	u32 i = 0;

	CfaMp4Inst *prCfaMp4 = (CfaMp4Inst *) pvPrivData;

	if ((NULL == pvRange) || (NULL == prCfaMp4))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4][%s] Enter SetRange\r\n"), __func__);

#ifdef MM_ATE_CHECK
		dmx_memcpy(&(prCfaMp4->rCfaRange.u8Length), &(((CfaMp4Range *) pvRange)->u8Length),
			   sizeof(CfaMp4Range) - 2 * sizeof(u32));

#else
		dmx_memcpy(&(prCfaMp4->rCfaRange), pvRange, sizeof(CfaMp4Range));

#endif
	MMATE_INIT_STRUCT(prCfaMp4->rCfaRange);
	prCfaMp4->fgSyncBuf = FALSE;
	prCfaMp4->u8Ca = CfaMp4GetRangeCa(&prCfaMp4->rCfaRange);
#if MP4_SUPPORT_FRAGMENT
	prCfaMp4->fgGetMoofData = FALSE;
	if (prCfaMp4->eCfaMoofType == TYPE_ONLY_MOOF) {
		if ((NULL != prCfaMp4->rCfaMp4AInf[0].pucDecSpecInfo) &&
			 (0 != prCfaMp4->rCfaMp4AInf[0].u4DecSpecSz)) {
			prCfaMp4->rCfaMp4AInf[0].fgTxAacSCDone = FALSE;
		}
		if (AVCODEC_ID_AAC == prCfaMp4->rCfaMp4AInf[0].eAudType) {
			prCfaMp4->rCfaMp4AInf[0].fgAddAdtsDone = FALSE;
		}
		else {
			prCfaMp4->rCfaMp4AInf[i].fgAddAdtsDone = TRUE;
		}
	} else {
#endif
	CfaMp4ParseRange(prCfaMp4);

		/*set some audio optional infomation*/
#if (LAST_MEMORY == 0)
	if (prCfaMp4->fgGetATbl) {
		MRESULT mrRet = RET_DMX_OK;

		prCfaMp4->fgGetATbl = FALSE;
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4] NOT SET LASTMEMORY \r\n"));
		mrRet = mrCfaMp4ParseTable(prCfaMp4);
		if (RET_DMX_OK != mrRet) {
			CfaMp4InternalFreeMem(prCfaMp4);
			MM_RETURN(mrRet);
		}
	} else {
#endif

	{
		for (i = (u32)0; i < (u32)MAX_NS_MP4_AUD; i++) {
			if ((NULL != prCfaMp4->rCfaMp4AInf[i].pucDecSpecInfo) &&
				 (0 != prCfaMp4->rCfaMp4AInf[i].u4DecSpecSz)) {
				prCfaMp4->rCfaMp4AInf[i].fgTxAacSCDone = FALSE;
			}
			if (AVCODEC_ID_AAC == prCfaMp4->rCfaMp4AInf[i].eAudType)
				prCfaMp4->rCfaMp4AInf[i].fgAddAdtsDone = FALSE;

			}
		prCfaMp4->rCfaMp4AInf[0].fgGetRangeOfst = TRUE;
	}
#if (LAST_MEMORY == 0)
	}
#endif
#if MP4_SUPPORT_FRAGMENT
	}
#endif
	/* for http player, we have to restore seq header and pic param info for it's special seek flow*/
	/* add by mtk68034 20131204*/
	prCfaMp4->rCfaMp4VInf.u4NextSeqParamPos = 0;
	prCfaMp4->rCfaMp4VInf.u4NextPicParamPos = 0;
	prCfaMp4->rCfaMp4VInf.u4NextVPSPos = 0;
	dmx_memcpy(prCfaMp4->rCfaMp4VInf.u1SeqParamNum, prCfaMp4->rCfaMp4VInf.u1SeqParamNumbkp,
			MP4_STSD_TABLE_MAX_NUMS * sizeof(u8));
	dmx_memcpy(prCfaMp4->rCfaMp4VInf.u1PicParamNum, prCfaMp4->rCfaMp4VInf.u1PicParamNumbkp,
			MP4_STSD_TABLE_MAX_NUMS * sizeof(u8));
	MMATE_CHECK_STRUCT(prCfaMp4->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaMp4->rCfaMp4VInf);
	MMATE_CHECK_STRUCT(prCfaMp4->rCurOfst);
	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4EnableStrm
*
* Description:
*	   MP4 CFA sets stream to parse, may be combinations of V/A/S.
*	   splitter will ensure that pfvSetStrm() is only called in "off" or "paused" state.
*
* Inputs:
*
* Outputs:
*
* Returns: MRESULT
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaMp4EnableStrm(void *pvSptHdl, u32 u4StrmToPrs, CfaStreamOp eOp,
				void *pvPrivData)
{
	CfaMp4Inst *prCfaMp4 = (CfaMp4Inst *) pvPrivData;

	if (NULL == prCfaMp4)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4][%s] Enter EnableStrm \r\n"), __func__);
	MMATE_CHECK_POINTER(prCfaMp4);
	MMATE_CHECK_STRUCT(prCfaMp4->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaMp4->rCfaMp4VInf);
	MMATE_CHECK_STRUCT(prCfaMp4->rCurOfst);

	/*-----------------------------------------------------------------------------
	we can't get the Aud info idx when vSetStrmInf,
	since we set the audio related info later in the vConfigInfo.
	-----------------------------------------------------------------------------*/
	if (CFA_STREAM_ON == eOp) {

		/* enable */
		if (CFA_STRM_V & u4StrmToPrs) {
			prCfaMp4->u4CurPrsFlg |= CFA_MP4_PRS_BIT_STRM_TYPE_V;
			prCfaMp4->u4PrsFlg |= CFA_MP4_PRS_BIT_STRM_TYPE_V;
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					 TEXT("[CFA MP4][%s] vCfaMp4EnableStrm: Video\r\n"),
					 __func__);
		}
		if (CFA_STRM_A & u4StrmToPrs) {
			prCfaMp4->u4CurPrsFlg |= CFA_MP4_PRS_BIT_STRM_TYPE_A;
			prCfaMp4->u4PrsFlg |= CFA_MP4_PRS_BIT_STRM_TYPE_A;
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					 TEXT("[CFA MP4][%s] vCfaMp4EnableStrm: Audio\r\n"),
					 __func__);
		}
		if (CFA_STRM_SP & u4StrmToPrs) {
			prCfaMp4->u4CurPrsFlg |= CFA_MP4_PRS_BIT_STRM_TYPE_SP;
			prCfaMp4->u4PrsFlg |= CFA_MP4_PRS_BIT_STRM_TYPE_SP;
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					 TEXT("[CFA MP4][%s] vCfaMp4EnableStrm: SubPic\r\n"),
					 __func__);
		}
	} else {
		if (CFA_STRM_V & u4StrmToPrs) {
			prCfaMp4->u4CurPrsFlg &= ~((u32) CFA_MP4_PRS_BIT_STRM_TYPE_V);
			prCfaMp4->u4PrsFlg &= ~((u32) CFA_MP4_PRS_BIT_STRM_TYPE_V);
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					 TEXT("[CFA MP4][%s] vCfaMp4DisableStrm: Video\r\n"),
					 __func__);
		}
		if (CFA_STRM_A & u4StrmToPrs) {
			prCfaMp4->u4CurPrsFlg &= ~((u32) CFA_MP4_PRS_BIT_STRM_TYPE_A);
			prCfaMp4->u4PrsFlg &= ~((u32) CFA_MP4_PRS_BIT_STRM_TYPE_A);
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					 TEXT("[CFA MP4][%s] vCfaMp4DisableStrm: Audio\r\n"),
					 __func__);
		}
		if (CFA_STRM_SP & u4StrmToPrs) {
			prCfaMp4->u4CurPrsFlg &= ~((u32) CFA_MP4_PRS_BIT_STRM_TYPE_SP);
			prCfaMp4->u4PrsFlg &= ~((u32) CFA_MP4_PRS_BIT_STRM_TYPE_SP);
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					 TEXT("[CFA MP4][%s] vCfaMp4DisableStrm: SubPic\r\n"),
					 __func__);
		}
	}
	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4SetStrmInf
*
* Description:
*	   Set Stream ID
*
* Inputs:
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaMp4SetStrmInf(void *pvSptHdl, u32 u4Strm, u32 u4Info, void *pvPrivData)
{
	CfaMp4Inst *prCfaMp4 = (CfaMp4Inst *) pvPrivData;

	if (NULL == prCfaMp4)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4][%s] Enter SetStrmInf \r\n"), __func__);
	if (CFA_STRM_V == u4Strm)
		prCfaMp4->u4CurVidInfoId = u4Info;
	else if (CFA_STRM_A == u4Strm)
		prCfaMp4->u4CurAudPlayId = u4Info;
	else if (CFA_STRM_SP == u4Strm)
		prCfaMp4->u4CurSubInfoId = u4Info;
	else {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				 TEXT("[CFA MP4] Receive Unkown Stream Type: %d. \r\n"),
				 u4Strm);
	}
	MMATE_CHECK_POINTER(prCfaMp4);
	MMATE_CHECK_STRUCT(prCfaMp4->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaMp4->rCfaMp4VInf);
	MMATE_CHECK_STRUCT(prCfaMp4->rCurOfst);
	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4TurnOn
*
* Description:
*	   MP4 CFA turns on file demuxing
*	   A transfer should be issued in this function.
*
* Inputs:
*
* Outputs:
*
* Returns: MRESULT
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaMp4TurnOn(void *pvSptHdl, void *pvPrivData)
{
	CfaMp4Inst *prCfaMp4 = (CfaMp4Inst *) pvPrivData;
	TCfaMp4VInf *prMp4VInfo;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prCfaMp4)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	MMATE_CHECK_POINTER(prCfaMp4);
	MMATE_CHECK_STRUCT(prCfaMp4->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaMp4->rCfaMp4VInf);
	MMATE_CHECK_STRUCT(prCfaMp4->rCurOfst);

	DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4][%s][CFA_VERSION:2017-0307-0957] CfaMp4TurnOn, fgSyncBuf %d ,SA 0x%llx -- EA 0x%llx\n"),
		__func__, prCfaMp4->fgSyncBuf, prCfaMp4->rCfaRange.u8VidStartOffset, prCfaMp4->rCfaRange.u8VidEndOffset);
	prCfaMp4->fgFinished = FALSE;
	prCfaMp4->fgTxAvcHdr = FALSE;
	prCfaMp4->eLastCfaMp4AnaSt = CFA_MP4_ANA_ST_IDLE;
	prCfaMp4->eCurPrsSampleType = CFA_MP4_PRS_BIT_STRM_TYPE_NONE;
	prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_NEXT_STATE;
	prMp4VInfo = &prCfaMp4->rCfaMp4VInf;
	prMp4VInfo->fgTxMp4SCDone = TRUE;
	prCfaMp4->fgGetSPLen = FALSE;
	if ((CFA_VID_DIVX4 == prCfaMp4->eVidType)
		   || (CFA_VID_MPEG4 == prCfaMp4->eVidType)
		   || (CFA_VID_H265 == prCfaMp4->eVidType) || (CFA_VID_VC1 == prCfaMp4->eVidType)
		   || ((CFA_VID_MPEG2 == prCfaMp4->eVidType) && (0 != prMp4VInfo->u4DecSpecSz[0]))){
		prMp4VInfo->fgTxMp4SCDone = FALSE;
	} else if (((CFA_VID_WMV7 == prCfaMp4->eVidType) || (CFA_VID_WMV8 == prCfaMp4->eVidType) ||
		(CFA_VID_WMV9 == prCfaMp4->eVidType)) && (0 != prMp4VInfo->u4DecSpecSz[0]))
		prMp4VInfo->fgTxMp4SCDone = FALSE;
	else if (CFA_VID_H264 == prCfaMp4->eVidType)
		if ((0 != prMp4VInfo->u1SeqParamNum[0]) || (0 != prMp4VInfo->u1PicParamNum[0]))
			prMp4VInfo->fgTxMp4SCDone = FALSE;

	if (!prCfaMp4->fgSyncBuf) {
		u64 u8FileOfst = 0;

		u8FileOfst = CfaMp4GetRangeCa(&prCfaMp4->rCfaRange);

		prCfaMp4->fgSyncBuf = TRUE;
		prCfaMp4->u8Ca = u8FileOfst;
		if (prCfaMp4->rCfaRange.u8Length == 0) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4] call Spt4CfaFinishedEx\n"));
			mrRet = Spt4CfaFinishedEx(pvSptHdl, prCfaMp4->u8Ca, TRUE, (u32)GAU_E_EOS);
			MM_RETURN(mrRet);
		}

		mrRet =
			Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaMp4->u8Ca, (u64)1,
					   (u8 *) &prCfaMp4->ptrPfrMemAddress);

		if (RET_DMX_OK != mrRet) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4] Spt4CfaPbb2SyncBuf error ret = %d\n"),
					mrRet);
			MM_RETURN(mrRet);
		}
	} else
		CfaMp4PrsNextState(pvSptHdl, prCfaMp4);
	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4TxDone
*
* Description:
*	   MP4 CFA callback for transfer done
*	   This function will be called after a transfer is complete.
*
* Inputs:
*
* Outputs:
*
* Returns: MRESULT
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaMp4TxDone(void *pvSptHdl, u64 u8TxLen, void *pvPrivData, bool fgRsp)
{
	CfaMp4Inst *prCfaMp4 = (CfaMp4Inst *) pvPrivData;

	if (NULL == prCfaMp4)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	MMATE_CHECK_POINTER(prCfaMp4);
	MMATE_CHECK_STRUCT(prCfaMp4->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaMp4->rCfaMp4VInf);
	MMATE_CHECK_STRUCT(prCfaMp4->rCurOfst);
	if (fgRsp) {
		MRESULT mrRet =
			Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaMp4->u8Ca, u8TxLen,
					   (u8 *) &(prCfaMp4->ptrPfrMemAddress));
		if (RET_DMX_OK != mrRet) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4] Spt4CfaPbb2SyncBuf error ret = %d\n"),
					 mrRet);
		}
		MM_RETURN(mrRet);
	}
	CfaMp4TxDoneStCtrl(pvSptHdl, prCfaMp4);
	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4GetCurPos
*
* Description:
*	   MP4 CFA callback for when FMPC needs to know CFA's current position.
*
* Inputs:
*
* Outputs:
*
* Returns:MRESULT
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaMp4GetCurPos(void *pvSptHdl, void *pvCurPos, void *pvPrivData)
{
	CfaMp4Inst *prCfaMp4 = (CfaMp4Inst *) pvPrivData;
	u64 *pu8FileOffset = pvCurPos;

	if (NULL == prCfaMp4)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4][%s] Enter CfaMp4GetCurPos \r\n"), __func__);
	if (!prCfaMp4->fgFinished) {
		*pu8FileOffset = prCfaMp4->rCurOfst.u8VidCurOfst;
		DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4][%s]u8Ca: 0x%llx, u8VidCurOfst:0xllx, u8CurPrsVidSampleNo:0xllx. \r\n"),
			__func__, prCfaMp4->u8Ca, prCfaMp4->rCurOfst.u8VidCurOfst,
			prCfaMp4->rCfaMp4VInf.u8CurPrsVidSampleNo);
	}
	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4FillPicInfo
*
* Description:
*	   MP4 CFA callback for each picture is demuxed
*	   original related function: vMp4M4vPIsr
*
* Inputs:
*
* Outputs:
*
* Returns: TRUE - this picture should be retained in video FIFO.
*		   FALSE - this picture should be removed from video FIFO.
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaMp4FillPicInfo(void *pvSptHdl, Spt2CfaPicInfo *ptPicInfo, void *pvPrivData)
{
	CfaMp4Inst *prCfaMp4 = (CfaMp4Inst *) pvPrivData;

	if ((NULL == prCfaMp4) || (NULL == ptPicInfo))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	MMATE_CHECK_POINTER(prCfaMp4);
	MMATE_CHECK_STRUCT(prCfaMp4->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaMp4->rCfaMp4VInf);
	MMATE_CHECK_STRUCT(prCfaMp4->rCurOfst);
	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] Enter CfaMp4FillPicInfo \r\n"),
			   __func__);
	ptPicInfo->u8ThisDts = 0;

	/*DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
	TEXT("[CFA MP4][CfaMp4FillPicInfo] CurVPts: 0x%x\n"), prCfaMp4->u8CurVPts);*/
	ptPicInfo->u8Custom1 = prCfaMp4->rCfaMp4VInf.u8CurPrsVidSampleNo - 1;
	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4Configure
*
* Description:
*	   splitter will ensure that it is only called in "off" or "paused" state.
*	   save info from pvParam to pvPrivData
* Inputs:
*
* Outputs:
*
* Returns: MRESULT
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaMp4Configure(void *pvSptHdl, void *pvParam, void *pvPrivData, bool fgIsUserMem)
{
	CfaMp4ConfigInfo *prCfaMp4Cfg = NULL;
	CfaMp4ConfigInfo rCfaMp4Cfg;
	CfaMp4Inst *prCfaMp4 = (CfaMp4Inst *) pvPrivData;
	TCfaMp4VInf *prMp4VInfo = NULL;
	CfaMp4VidInfo *prCfgVInfo = NULL;
	CfaMp4AudInfo *prCfgAInfo = NULL;
	CfaMp4SubInfo *prCfgSInfo = NULL;
	u32 i = 0;
	u32 u4BufNums = 0;
	MRESULT mrRet = RET_DMX_OK;

	if (!pvParam || !pvPrivData)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	mm_memset(&rCfaMp4Cfg, 0, sizeof(CfaMp4ConfigInfo));
	prCfaMp4Cfg = &rCfaMp4Cfg;
	if (fgIsUserMem) {
		if (0 != mm_copy_from_user(prCfaMp4Cfg,
			pvParam, sizeof(CfaMp4ConfigInfo))) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA_MP4] %s line %d failed in mm_copy_from_user\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
		}
	}
	else {
		mm_memcpy(prCfaMp4Cfg,
				pvParam, sizeof(CfaMp4ConfigInfo));
	}

	MMATE_CHECK_POINTER(prCfaMp4);
	DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4][%s] Enter CfaMp4Configure \r\n"), __func__);

	/* function point  */
	prCfaMp4->fgSyncBuf = FALSE;
	prCfaMp4->fgSh263GetedHdr = FALSE;
	prCfaMp4->fgGetSPLen = FALSE;

#if MP4_SUPPORT_FRAGMENT
	prCfaMp4->u8CfaCurMoofOffset= prCfaMp4Cfg->u8MoofOffset;
	prCfaMp4->eCfaMoofType= prCfaMp4Cfg->eMoofType;
#endif
	/* video related info from playback */
	prMp4VInfo = &prCfaMp4->rCfaMp4VInf;
	prCfgVInfo = &prCfaMp4Cfg->rCfaMp4VidInfo;
	prCfaMp4->eVidType = CfaMp4GetVidCodec(prCfgVInfo->eVidType);
	if ((CFA_VID_MPEG4 == prCfaMp4->eVidType) || (CFA_VID_DIVX4 == prCfaMp4->eVidType)
		|| ((CFA_VID_MPEG2 == prCfaMp4->eVidType) && (0 != prCfgVInfo->u4DecSpecSz))) {
		DMX_NewHwMemory(prCfgVInfo->u4DecSpecSz, prMp4VInfo->pucMpeg4CodecSC);
		if (NULL == prMp4VInfo->pucMpeg4CodecSC) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA_MP4] Alloc prMp4VInfo->pucMpeg4CodecSC memory fail\n"));
			CfaMp4InternalFreeMem(prCfaMp4);
			if (!fgIsUserMem)
				DMX_FreeMemory(pvParam);
			MM_RETURN(RET_DMX_NO_MEM);
		}

		if (fgIsUserMem) {
			mm_copy_from_user(prMp4VInfo->pucMpeg4CodecSC, prCfgVInfo->pucDecSpecInfo,
						prCfgVInfo->u4DecSpecSz);
		}
		else{
			dmx_memcpy(prMp4VInfo->pucMpeg4CodecSC, prCfgVInfo->pucDecSpecInfo,
					prCfgVInfo->u4DecSpecSz);
		}
		prMp4VInfo->u4DecSpecSz[0] = prCfgVInfo->u4DecSpecSz;
		prMp4VInfo->fgTxMp4SCDone = FALSE;
	} else if (CFA_VID_VC1 == prCfaMp4->eVidType) {
		DMX_NewHwMemory(4, prMp4VInfo->puCodecSC);
		if (NULL == prMp4VInfo->puCodecSC) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA_MP4] Alloc prMp4VInfo->puCodecSC memory fail\n"));
			CfaMp4InternalFreeMem(prCfaMp4);
			if (!fgIsUserMem)
				DMX_FreeMemory(pvParam);
			MM_RETURN(RET_DMX_NO_MEM);
		}
		DMX_NewHwMemory(prCfaMp4Cfg->rCfaMp4VidInfo.u4DecSpecSz,
				   prMp4VInfo->pucWVc1CodecSC);
		if (NULL == prMp4VInfo->pucWVc1CodecSC) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA_MP4] Alloc prMp4VInfo->pucWVc1CodecSC memory fail\n"));
			CfaMp4InternalFreeMem(prCfaMp4);
			if (!fgIsUserMem)
				DMX_FreeMemory(pvParam);
			MM_RETURN(RET_DMX_NO_MEM);
		}
		prMp4VInfo->puCodecSC[0] = 0x00;
		prMp4VInfo->puCodecSC[1] = 0x00;
		prMp4VInfo->puCodecSC[2] = 0x01;
		prMp4VInfo->puCodecSC[3] = 0x0D;
		prMp4VInfo->fgTxMp4SCDone = FALSE;
		prMp4VInfo->fgCodecSCDone = FALSE;
		prMp4VInfo->u4DecSpecSz[0] = prCfaMp4Cfg->rCfaMp4VidInfo.u4DecSpecSz;

		if (fgIsUserMem) {
			mm_copy_from_user(prMp4VInfo->pucWVc1CodecSC, prCfgVInfo->pucDecSpecInfo,
				  prCfgVInfo->u4DecSpecSz);
		 }
		else{
			dmx_memcpy(prMp4VInfo->pucWVc1CodecSC, prCfgVInfo->pucDecSpecInfo,
					prCfgVInfo->u4DecSpecSz);
		}
	} else if ((CFA_VID_WMV7 == prCfaMp4->eVidType) ||
		 (CFA_VID_WMV8 == prCfaMp4->eVidType) || (CFA_VID_WMV9 == prCfaMp4->eVidType)) {
		if (prCfaMp4Cfg->rCfaMp4VidInfo.pucDecSpecInfo
			 && prCfaMp4Cfg->rCfaMp4VidInfo.u4DecSpecSz > 0) {
			DMX_NewHwMemory(prCfaMp4Cfg->rCfaMp4VidInfo.u4DecSpecSz,
					   prMp4VInfo->pucMpeg4CodecSC);
			if (NULL == prMp4VInfo->pucMpeg4CodecSC) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA_MP4] Alloc prMp4VInfo->pucMpeg4CodecSC memory fail\n"));
				CfaMp4InternalFreeMem(prCfaMp4);
				if (!fgIsUserMem)
					DMX_FreeMemory(pvParam);
				MM_RETURN(RET_DMX_NO_MEM);
			}

			if (fgIsUserMem) {
				mm_copy_from_user(prMp4VInfo->pucMpeg4CodecSC,
							prCfgVInfo->pucDecSpecInfo, prCfgVInfo->u4DecSpecSz);
			}
			else{
				dmx_memcpy(prMp4VInfo->pucMpeg4CodecSC, prCfgVInfo->pucDecSpecInfo,
						prCfgVInfo->u4DecSpecSz);
			}
			prMp4VInfo->u4DecSpecSz[0] = prCfaMp4Cfg->rCfaMp4VidInfo.u4DecSpecSz;
			prMp4VInfo->fgTxMp4SCDone = FALSE;
		} else
			prMp4VInfo->fgTxMp4SCDone = TRUE;

	} else if (CFA_VID_H264 == prCfaMp4->eVidType || CFA_VID_H265 == prCfaMp4->eVidType) {
		prMp4VInfo->u1AvcSmpDesNums = prCfgVInfo->u1AvcSmpDesNums;
		if (0 == prMp4VInfo->u1AvcSmpDesNums)
			prMp4VInfo->u1AvcSmpDesNums = 1;

		prCfaMp4->u4EsdNums = prMp4VInfo->u1AvcSmpDesNums;
		if (0xFF != prCfgVInfo->u1PayLoadLength[0])
			prCfaMp4->u1SyncBufSize = prCfgVInfo->u1PayLoadLength[0] + 1;
		else
			prCfaMp4->u1SyncBufSize = 0xFF;

		for (i = 0; i < prMp4VInfo->u1AvcSmpDesNums; i++) {
			if (prCfgVInfo->pucSeqParamInfo[i]) {
				prMp4VInfo->u1SeqParamNum[i] = prCfgVInfo->u1SeqParamNum[i];
				DMX_NewMemory(prCfgVInfo->u4SeqSize[i],
						 prMp4VInfo->pucSeqParamInfo[i]);
				if (NULL == prMp4VInfo->pucSeqParamInfo[i]) {
					DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA_MP4] Alloc prMp4VInfo->")
						TEXT("pucSeqParamInfo[i] memory fail\n"));
					if (!fgIsUserMem)
						DMX_FreeMemory(pvParam);
					MM_RETURN(RET_DMX_NO_MEM);
				}

				if (fgIsUserMem) {
					mm_copy_from_user(prMp4VInfo->pucSeqParamInfo[i],
								prCfgVInfo->pucSeqParamInfo[i],
								prCfgVInfo->u4SeqSize[i]);
				}
				else{
					dmx_memcpy(prMp4VInfo->pucSeqParamInfo[i],
							prCfgVInfo->pucSeqParamInfo[i],
							prCfgVInfo->u4SeqSize[i]);
				}
				prMp4VInfo->u4DecSpecSz[i] =
				CfaMp4GetSpecSize(prMp4VInfo->u1SeqParamNum[i],
						  prMp4VInfo->pucSeqParamInfo[i]);
			}
			if (prCfaMp4Cfg->rCfaMp4VidInfo.pucPicParamInfo[i]) {
				prMp4VInfo->u1PicParamNum[i] = prCfgVInfo->u1PicParamNum[i];
				DMX_NewMemory(prCfgVInfo->u4PicSize[i],
						 prMp4VInfo->pucPicParamInfo[i]);
				if (NULL == prMp4VInfo->pucPicParamInfo[i]) {
					DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA_MP4] Alloc prMp4VInfo->")
						TEXT("pucPicParamInfo[i] memory fail\n"));
					CfaMp4InternalFreeMem(prCfaMp4);
					if (!fgIsUserMem)
						DMX_FreeMemory(pvParam);
					MM_RETURN(RET_DMX_NO_MEM);
					}

				if (fgIsUserMem) {
					mm_copy_from_user(prMp4VInfo->pucPicParamInfo[i],
								prCfgVInfo->pucPicParamInfo[i],
								prCfgVInfo->u4PicSize[i]);
				}
				else {
					dmx_memcpy(prMp4VInfo->pucPicParamInfo[i],
							prCfgVInfo->pucPicParamInfo[i],
							prCfgVInfo->u4PicSize[i]);
				}
				prMp4VInfo->u4DecSpecSz[i] +=
				CfaMp4GetSpecSize(prMp4VInfo->u1PicParamNum[i],
						  prMp4VInfo->pucPicParamInfo[i]);
			}
			if (prCfaMp4Cfg->rCfaMp4VidInfo.pucVPSInfo[i]) {
					prMp4VInfo->u1VPSNum[i] = prCfgVInfo->u1VPSNum[i];
					DMX_NewMemory(prCfgVInfo->u4VPSSize[i],
						 prMp4VInfo->pucVPSInfo[i]);
				if (NULL == prMp4VInfo->pucVPSInfo[i]) {
					DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA_MP4] Alloc prMp4VInfo->pucVPSInfo[i] memory fail\n"));
					CfaMp4InternalFreeMem(prCfaMp4);
					if (!fgIsUserMem)
						DMX_FreeMemory(pvParam);
					MM_RETURN(RET_DMX_NO_MEM);
				}

				if (fgIsUserMem) {
					mm_copy_from_user(prMp4VInfo->pucVPSInfo[i],
								prCfgVInfo->pucVPSInfo[i],
								prCfgVInfo->u4VPSSize[i]);
				}
				else{
					dmx_memcpy(prMp4VInfo->pucVPSInfo[i], prCfgVInfo->pucVPSInfo[i],
							prCfgVInfo->u4VPSSize[i]);
				}
				prMp4VInfo->u4DecSpecSz[i] +=
				CfaMp4GetSpecSize(prMp4VInfo->u1VPSNum[i],
						  prMp4VInfo->pucVPSInfo[i]);
			}
			if ((!prCfgVInfo->pucSeqParamInfo[0])
				   && (!prCfgVInfo->pucPicParamInfo[0]) && (!prCfgVInfo->pucVPSInfo[0])) {
				prMp4VInfo->fgTxMp4SCDone = TRUE;
			} else {
				DMX_NewHwMemory(prMp4VInfo->u4DecSpecSz[i] + 4,
						 prMp4VInfo->pucDecSpecInfo[i]);
				if (NULL == prMp4VInfo->pucDecSpecInfo[i]) {
					DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA_MP4] Alloc prMp4VInfo->pucDecSpecInfo[i] memory fail\n"));
					CfaMp4InternalFreeMem(prCfaMp4);
					if (!fgIsUserMem)
						DMX_FreeMemory(pvParam);
					MM_RETURN(RET_DMX_NO_MEM);
				}
				*prMp4VInfo->pucDecSpecInfo[i] = 0x00;
				*(prMp4VInfo->pucDecSpecInfo[i] + 1) = 0x00;
				*(prMp4VInfo->pucDecSpecInfo[i] + 2) = 0x00;
				*(prMp4VInfo->pucDecSpecInfo[i] + 3) = 0x01;
				prMp4VInfo->fgTxMp4SCDone = FALSE;
			}
		}
	} else
		prMp4VInfo->fgTxMp4SCDone = TRUE;

	prMp4VInfo->u4NextSeqParamPos = 0;
	prMp4VInfo->u4NextPicParamPos = 0;
	prMp4VInfo->u4NextVPSPos = 0;
	prMp4VInfo->prVTable = NULL;
	if ((prCfgVInfo->prVTable) && (0 != prCfgVInfo->prVTable->u4EntryNs)) {
		mrRet = CfaMp4CfgStmTbl(&(prMp4VInfo->prVTable), prCfgVInfo->prVTable);
		if (RET_DMX_OK != mrRet) {
			CfaMp4InternalFreeMem(prCfaMp4);
			if (!fgIsUserMem)
				DMX_FreeMemory(pvParam);
			MM_RETURN(mrRet);
		}
		prMp4VInfo->u4SampleSize = prCfgVInfo->u4SampSz;
		prMp4VInfo->fgCO64Valid = prCfgVInfo->fgCO64Valid;
		prMp4VInfo->u4TrackID = prCfgVInfo->u4TrackID;	/*add by zhiwei chen mtk40495 for streamid*/
		if (prCfgVInfo->u4SampSz)
			u4BufNums = prMp4VInfo->prVTable[STTS].u4Allotted + 20;
		else
			u4BufNums = prMp4VInfo->prVTable[STSZ].u4Allotted + 20;

		prCfaMp4->rCurTblPos.rTblVidPos.u4SampleBufNums = u4BufNums;
		DMX_NewMemory(sizeof(TSampleInfo) * u4BufNums, prMp4VInfo->pTSampleInfo);
		if (NULL == prMp4VInfo->pTSampleInfo) {
			CfaMp4InternalFreeMem(prCfaMp4);
			if (!fgIsUserMem)
				DMX_FreeMemory(pvParam);
			MM_RETURN(RET_DMX_NO_MEM);
		}
		u4BufNums = prMp4VInfo->prVTable[STCO].u4Allotted + 20;
		prCfaMp4->rCurTblPos.rTblVidPos.u4ChunkBufNums = u4BufNums;
		DMX_NewMemory(sizeof(TChunkInfo) * u4BufNums, prMp4VInfo->pTChunkInfo);
		if (NULL == prMp4VInfo->pTChunkInfo) {
			CfaMp4InternalFreeMem(prCfaMp4);
			if (!fgIsUserMem)
				DMX_FreeMemory(pvParam);
			MM_RETURN(RET_DMX_NO_MEM);
		}
		prMp4VInfo->u4TimeScale = prCfgVInfo->u4TimeScale;
#if MP4_SUPPORT_FRAGMENT
	}else if (prCfaMp4->eCfaMoofType == TYPE_ONLY_MOOF) {
		prMp4VInfo->u4TrackID = prCfgVInfo->u4TrackID;
		prMp4VInfo->u4TimeScale = prCfgVInfo->u4TimeScale;
		prMp4VInfo->prVTable = NULL;
		DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4][%s] Enter CfaMp4Configure,prCfaMp4->eCfaMoofType == TYPE_ONLY_MOOF \r\n"), __func__);
		}else if (prCfaMp4->eCfaMoofType == TYPE_MOOV_AND_MOOF) {
		prMp4VInfo->u4TrackID = prCfgVInfo->u4TrackID;
		prMp4VInfo->u4TimeScale = prCfgVInfo->u4TimeScale;
		prMp4VInfo->prVTable = NULL;
		prCfaMp4->eCfaMoofType = TYPE_ONLY_MOOF;
		DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4][%s] Enter CfaMp4Configure,TYPE_MOOV_AND_MOOF change type to TYPE_ONLY_MOOF \r\n"), __func__);
#endif
	} else {
		prCfaMp4->u4CurPrsFlg &= ~((u32) CFA_MP4_PRS_BIT_STRM_TYPE_V);
		prMp4VInfo->prVTable = NULL;
	}

	/* audio related info from playback */
	prCfgAInfo = &prCfaMp4Cfg->rCfaMp4AudInfo;
	CFA_MP4_LIMIT_A_NUM(prCfgAInfo->u2AudStrmNum);
	prCfaMp4->u2AudStrmNums = prCfgAInfo->u2AudStrmNum;
	for (i = 0; i < prCfgAInfo->u2AudStrmNum; i++) {
		TCfaMp4AInf *prAInfo = &prCfaMp4->rCfaMp4AInf[i];

		prAInfo->prATable = NULL;
#if MP4_SUPPORT_FRAGMENT
		if ((prCfgAInfo->prATable[i] && prCfgAInfo->prATable[i]->u4EntryNs != 0) ||
			(prCfaMp4->eCfaMoofType == TYPE_ONLY_MOOF))
#else
		if (prCfgAInfo->prATable[i] && prCfgAInfo->prATable[i]->u4EntryNs != 0)
#endif
		{
#if MP4_SUPPORT_FRAGMENT
			if ((prCfaMp4->eCfaMoofType == TYPE_ONLY_MOOV) ||
				(prCfaMp4->eCfaMoofType == TYPE_MOOV_AND_MOOF)) {
#endif
			mrRet = CfaMp4CfgStmTbl(&(prAInfo->prATable), prCfgAInfo->prATable[i]);
			if (RET_DMX_OK != mrRet) {
				CfaMp4InternalFreeMem(prCfaMp4);
				if (!fgIsUserMem)
					DMX_FreeMemory(pvParam);
				MM_RETURN(mrRet);
			}
			prAInfo->u4SampleSize = prCfgAInfo->u4SampSz[i];
			prAInfo->fgCO64Valid = prCfgAInfo->fgCO64Valid[i];
#if MP4_SUPPORT_FRAGMENT
			}
#endif
			prAInfo->u4TimeScale = prCfgAInfo->u4TimeScale[i];
			prAInfo->eAudType = prCfgAInfo->eAudType[i];
			prAInfo->u4TrackID = prCfgAInfo->u4TrackID[i];
			prAInfo->eCfaAudType = CfaMp4GetAudType(prCfgAInfo->eAudType[i]);
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA_MP4] Idx(%d) -- AudType: %d, CfaAudType: %d\r\n"),
					 i, prAInfo->eAudType, prAInfo->eCfaAudType);
			if (AVCODEC_ID_AAC_PURE== prAInfo->eAudType)
				prAInfo->eAudType = AVCODEC_ID_AAC;

			if (AVCODEC_ID_PCM == prAInfo->eAudType)
				prCfaMp4->fgLpcmSet = TRUE;
			else
				prCfaMp4->fgLpcmSet = FALSE;

			if (AVCODEC_ID_AAC == prAInfo->eAudType) {
				prAInfo->fgSyncIV = FALSE;
				DMX_NewHwMemory(8, prAInfo->pucADTSBuf);
				if (NULL == prAInfo->pucADTSBuf) {
					CfaMp4InternalFreeMem(prCfaMp4);
					if (!fgIsUserMem)
						DMX_FreeMemory(pvParam);
					MM_RETURN(RET_DMX_NO_MEM);
				}
				prAInfo->u4AudSamplePerSec = prCfgAInfo->u4AudSamplePerSec[i];
				prAInfo->u2AudChannels = prCfgAInfo->u2AudChannels[i];
				if (prCfgAInfo->pucDecSpecInfo[i] && prCfgAInfo->u4DecSpecSz[i]) {
					u8 *pucSize = (u8 *) &(prCfgAInfo->u4DecSpecSz[i]);

					prAInfo->fgTxAacSCDone = FALSE;
					DMX_NewHwMemory(prCfgAInfo->u4DecSpecSz[i] + 8,
							   prAInfo->pucDecSpecInfo);
					if (NULL == prAInfo->pucDecSpecInfo) {
						CfaMp4InternalFreeMem(prCfaMp4);
						if (!fgIsUserMem)
							DMX_FreeMemory(pvParam);
						MM_RETURN(RET_DMX_NO_MEM);
					}
					prAInfo->pucDecSpecInfo[0] = pucSize[3];
					prAInfo->pucDecSpecInfo[1] = pucSize[2];
					prAInfo->pucDecSpecInfo[2] = pucSize[1];
					prAInfo->pucDecSpecInfo[3] = pucSize[0];

					if (fgIsUserMem) {
						mm_copy_from_user(prAInfo->pucDecSpecInfo + 4,
								prCfgAInfo->pucDecSpecInfo[i],
								prCfgAInfo->u4DecSpecSz[i]);
					}
					else {
						dmx_memcpy(prAInfo->pucDecSpecInfo + 4,
								prCfgAInfo->pucDecSpecInfo[i],
								prCfgAInfo->u4DecSpecSz[i]);
					}
						prAInfo->u4DecSpecSz = prCfgAInfo->u4DecSpecSz[i];
				} else
					prAInfo->fgTxAacSCDone = TRUE;

			} else
				prAInfo->fgTxAacSCDone = TRUE;
#if MP4_SUPPORT_FRAGMENT
			if ((prCfaMp4->eCfaMoofType == TYPE_ONLY_MOOV) ||
				(prCfaMp4->eCfaMoofType == TYPE_MOOV_AND_MOOF)) {
#endif
			if (0 != prCfgAInfo->u4SampSz[i])
				u4BufNums = prAInfo->prATable[STTS].u4Allotted + 20;
			else
				u4BufNums = prAInfo->prATable[STSZ].u4Allotted + 20;

			prCfaMp4->rCurTblPos.rTblAudPos[i].u4SampleBufNums = u4BufNums;
			DMX_NewMemory(sizeof(TSampleInfo) * u4BufNums, prAInfo->pTSampleInfo);
			if (NULL == prAInfo->pTSampleInfo) {
				CfaMp4InternalFreeMem(prCfaMp4);
				if (!fgIsUserMem)
					DMX_FreeMemory(pvParam);
				MM_RETURN(RET_DMX_NO_MEM);
			}
			u4BufNums = prAInfo->prATable[STCO].u4Allotted + 20;
			prCfaMp4->rCurTblPos.rTblAudPos[i].u4ChunkBufNums = u4BufNums;
			DMX_NewMemory(sizeof(TChunkInfo) * u4BufNums, prAInfo->pTChunkInfo);
			if (NULL == prAInfo->pTChunkInfo) {
				CfaMp4InternalFreeMem(prCfaMp4);
				if (!fgIsUserMem)
					DMX_FreeMemory(pvParam);
				MM_RETURN(RET_DMX_NO_MEM);
			}
#if MP4_SUPPORT_FRAGMENT
			}
#endif

#if SPECIAL_LPCM_SUPPORT	/*Li Lu*/
				prCfaMp4->rCfaMp4AInf[i].u2AudChannels =
				prCfaMp4Cfg->rCfaMp4AudInfo.u2AudChannels[i];

#endif
		}
	}
#if MP4_SUPPORT_FRAGMENT
	if ((prCfaMp4->eCfaMoofType == TYPE_ONLY_MOOV) ||
		(prCfaMp4->eCfaMoofType == TYPE_MOOV_AND_MOOF)) {
#endif

	/* Subpicture related info from playback */
	prCfgSInfo = &prCfaMp4Cfg->rCfaMp4SubInfo;
	CFA_MP4_LIMIT_S_NUM(prCfgSInfo->u2SubStrmNum);
	prCfaMp4->u2SubStrmNums = prCfgSInfo->u2SubStrmNum;
	for (i = 0; i < prCfgSInfo->u2SubStrmNum; i++) {
		TCfaMp4SInf *prSInfo = &prCfaMp4->rCfaMp4SInf[i];

		prSInfo->prSTable = NULL;
		if (prCfgSInfo->prSTable[i] && (0 != prCfgSInfo->prSTable[i]->u4EntryNs)) {
			mrRet = CfaMp4CfgStmTbl(&(prSInfo->prSTable), prCfgSInfo->prSTable[i]);
			if (RET_DMX_OK != mrRet) {
				CfaMp4InternalFreeMem(prCfaMp4);
				if (!fgIsUserMem)
					DMX_FreeMemory(pvParam);
				MM_RETURN(mrRet);
			}
			if (prCfgSInfo->prMFSTable[i]) {
				mrRet =
					CfaMp4CfgStmTbl(&(prSInfo->prMFSTable),
							prCfgSInfo->prMFSTable[i]);
				if (RET_DMX_OK != mrRet) {
					CfaMp4InternalFreeMem(prCfaMp4);
					if (!fgIsUserMem)
						DMX_FreeMemory(pvParam);
					MM_RETURN(mrRet);
				}
			}
			prSInfo->fgPG = prCfgSInfo->fgPG[i];
			prSInfo->u4TimeScale = prCfgSInfo->u4TimeScale[i];
			prSInfo->u4SampleSize = prCfgSInfo->u4SampSz[i];
			prSInfo->fgCO64Valid = prCfgSInfo->fgCO64Valid[i];
			prSInfo->u4TrackID = prCfgSInfo->u4TrackID[i];
			prSInfo->eSubType = prCfgSInfo->eSubType[i];
			if (prCfgSInfo->u4SampSz[i])
				u4BufNums = prSInfo->prSTable[STTS].u4Allotted + 20;
			else
				u4BufNums = prSInfo->prSTable[STSZ].u4Allotted + 20;

			prCfaMp4->rCurTblPos.rTblSubPos[i].u4SampleBufNums = u4BufNums;
			DMX_NewMemory(sizeof(TSampleInfo) * u4BufNums, prSInfo->pTSampleInfo);
			if (NULL == prSInfo->pTSampleInfo) {
				CfaMp4InternalFreeMem(prCfaMp4);
				if (!fgIsUserMem)
					DMX_FreeMemory(pvParam);
				MM_RETURN(RET_DMX_NO_MEM);
			}
			u4BufNums = prSInfo->prSTable[STCO].u4Allotted + 20;
			prCfaMp4->rCurTblPos.rTblSubPos[i].u4ChunkBufNums = u4BufNums;
			DMX_NewMemory(sizeof(TChunkInfo) * u4BufNums, prSInfo->pTChunkInfo);
			if (NULL == prSInfo->pTChunkInfo) {
				CfaMp4InternalFreeMem(prCfaMp4);
				if (!fgIsUserMem)
					DMX_FreeMemory(pvParam);
				MM_RETURN(RET_DMX_NO_MEM);
			}
		}
	}
#if MP4_SUPPORT_FRAGMENT
	}
#endif

#if (LAST_MEMORY == 1)
#if MP4_SUPPORT_FRAGMENT

	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4][%s] MP4_SUPPORT_FRAGMENT SET LAST MEMORY\n"), __func__);
	if ((prCfaMp4->eCfaMoofType == TYPE_ONLY_MOOV) ||
		(prCfaMp4->eCfaMoofType == TYPE_MOOV_AND_MOOF))

#endif
	{
	prMp4VInfo->u8CurPrsVidChunkNo = 1;
	prMp4VInfo->u8CurPrsVidSampleNo = 1;
	prCfaMp4->rCurTblPos.rTblVidPos.u4CurTableLastChunkNo = 1;
	prCfaMp4->rCurTblPos.rTblVidPos.u8CurSampleNo = 1;
	prCfaMp4->rCfaRange.u8VidStartChunk1stSmp = 1;
	prCfaMp4->rCfaRange.u8VidStartChunkNo = 1;
	prCfaMp4->rCfaRange.u8VidStartSampleNo = 1;
	prCfaMp4->rCfaRange.u8VidEndChunkNo = DMX_INVALID_UINT64 - 1;
	prCfaMp4->rCfaRange.u8VidEndSampleNo = DMX_INVALID_UINT64 - 1;
	prCfaMp4->rCfaRange.u4PrsFlag |= CFA_MP4_PRS_BIT_STRM_TYPE_V;
	prCfaMp4->rCfaRange.u4PrsFlag |= CFA_MP4_PRS_BIT_STRM_TYPE_A;
	prCfaMp4->rCfaRange.u4PrsFlag |= CFA_MP4_PRS_BIT_STRM_TYPE_SP;
	for (i = 0; i < prCfgAInfo->u2AudStrmNum; i++) {
		prCfaMp4->rCfaMp4AInf[i].u8CurPrsAudChunkNo = 1;
		prCfaMp4->rCfaMp4AInf[i].u8CurPrsAudSampleNo = 1;
		prCfaMp4->rCurTblPos.rTblAudPos[i].u4CurTableLastChunkNo = 1;
		prCfaMp4->rCurTblPos.rTblAudPos[i].u8CurSampleNo = 1;
		prCfaMp4->rCfaRange.u8AudStartChunk1stSmp[i] = 1;
		prCfaMp4->rCfaRange.u8AudStartChunkNo[i] = 1;
		prCfaMp4->rCfaRange.u8AudStartSampleNo[i] = 1;
		prCfaMp4->rCfaRange.u8AudEndChunkNo[i] = DMX_INVALID_UINT64 - 1;
		prCfaMp4->rCfaRange.u8AudEndSampleNo[i] = DMX_INVALID_UINT64 - 1;
	}
	for (i = 0; i < prCfgSInfo->u2SubStrmNum; i++) {
		prCfaMp4->rCfaMp4SInf[i].u8CurPrsSubChunkNo = 1;
		prCfaMp4->rCfaMp4SInf[i].u8CurPrsSubSampleNo = 1;
		prCfaMp4->rCurTblPos.rTblSubPos[i].u4CurTableLastChunkNo = 1;
		prCfaMp4->rCurTblPos.rTblSubPos[i].u8CurSampleNo = 1;
		prCfaMp4->rCfaRange.u8SubStartChunk1stSmp[i] = 1;
		prCfaMp4->rCfaRange.u8SubStartChunkNo[i] = 1;
		prCfaMp4->rCfaRange.u8SubStartSampleNo[i] = 1;
		prCfaMp4->rCfaRange.u8SubEndChunkNo[i] = DMX_INVALID_UINT64 - 1;
		prCfaMp4->rCfaRange.u8SubEndSampleNo[i] = DMX_INVALID_UINT64 - 1;
	}
	DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4][%s]  SET LAST MEMORY\n"), __func__);
	mrRet = mrCfaMp4ParseTable(prCfaMp4);
	if (RET_DMX_OK != mrRet) {
		CfaMp4InternalFreeMem(prCfaMp4);
		if (!fgIsUserMem)
			DMX_FreeMemory(pvParam);
		MM_RETURN(mrRet);
	}
	}
#endif
		/*for http player, we have to restore seq header and pic param info for it's special seek flow*/
		/* add by mtk68034 20131204*/
	dmx_memcpy(prCfaMp4->rCfaMp4VInf.u1SeqParamNumbkp, prCfaMp4->rCfaMp4VInf.u1SeqParamNum,
			   MP4_STSD_TABLE_MAX_NUMS * sizeof(u8));
	dmx_memcpy(prCfaMp4->rCfaMp4VInf.u1PicParamNumbkp, prCfaMp4->rCfaMp4VInf.u1PicParamNum,
			MP4_STSD_TABLE_MAX_NUMS * sizeof(u8));
	MMATE_CHECK_POINTER(prCfaMp4);
	MMATE_CHECK_STRUCT(prCfaMp4->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaMp4->rCfaMp4VInf);
	MMATE_CHECK_STRUCT(prCfaMp4->rCurOfst);
	if (!fgIsUserMem)
		DMX_FreeMemory(pvParam);
	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4FillAUInfo
*
* Description:
*
*
* Inputs:
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaMp4FillAUInfo(void *pvSptHdl, void *pvAUInfo, void *pvAUExtInfo,
				void *pvPrivData)
{
	CfaMp4Inst *prCfaMp4 = (CfaMp4Inst *) pvPrivData;
	PicInfo *pPicInfo = &((AU_VPic *) pvAUInfo)->rAUInfo.rInfo;
	AudInfo *pAudInfo = &((AU_AUDIO *) pvAUInfo)->rAUInfo.rInfo;
	SPicInfo *pSubInfo = &((AU_SP *) pvAUInfo)->rAUInfo.rInfo;

	if (NULL == prCfaMp4)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	prCfaMp4 = (CfaMp4Inst *) pvPrivData;
	MMATE_CHECK_POINTER(prCfaMp4);
	MMATE_CHECK_STRUCT(prCfaMp4->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaMp4->rCfaMp4VInf);
	MMATE_CHECK_STRUCT(prCfaMp4->rCurOfst);
	switch (prCfaMp4->eCurPrsSampleType) {
	case CFA_MP4_PRS_BIT_STRM_TYPE_V:

		pPicInfo->u8Dts = INVALID_TIMESTAMP;
		pPicInfo->u8Pts = prCfaMp4->u8Vpts;
		pPicInfo->u4Duration = (u32) prCfaMp4->u8VDuration;
		if ((CFA_VID_H264 == prCfaMp4->eVidType) || (CFA_VID_H265 == prCfaMp4->eVidType)) {
			pPicInfo->u4EsdIndex = prCfaMp4->u4EsdIndex;
			pPicInfo->u4EsdNums = prCfaMp4->u4EsdNums;
		}
		if ((CFA_VID_RV30 == prCfaMp4->eVidType) || (CFA_VID_RV40 == prCfaMp4->eVidType)) {
			int i = 0;
			((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u4RMSliceNum =
				prCfaMp4->rSliceInf.u1TotalSliceNum;
			for (i = 0; i < prCfaMp4->rSliceInf.u1TotalSliceNum; i++) {
				((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.auRM4SliceSize[i] =
					prCfaMp4->rSliceInf.rSliceInf[i].u2SliceElemSize;
			}
			dmx_memset(&(prCfaMp4->rSliceInf), 0, sizeof(prCfaMp4->rSliceInf));
		}
		if (fgIsBType(pPicInfo->u4VType)) {
			if (CFA_PIC_P == prCfaMp4->ePrePicType)
				pPicInfo->u8PrevPTS = INVALID_TIMESTAMP;
		}
		if (fgIsIType(pPicInfo->u4VType))
			prCfaMp4->ePrePicType = CFA_PIC_I;
		else if (fgIsPType(pPicInfo->u4VType))
			prCfaMp4->ePrePicType = CFA_PIC_P;
		else if (fgIsBType(pPicInfo->u4VType))
			prCfaMp4->ePrePicType = CFA_PIC_B;
		else
			prCfaMp4->ePrePicType = CFA_PIC_UNDEFINE;

		DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] _CurVid(PTS): 0x%llx\r\n"),
				 __func__, prCfaMp4->u8Vpts);
		break;
	case CFA_MP4_PRS_BIT_STRM_TYPE_A:
		pAudInfo->u8Pts = prCfaMp4->u8Apts[prCfaMp4->u4CurAudInfoId];
		DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s]_CurAud(PTS): 0x%llx\r\n"),
				 __func__, prCfaMp4->u8Apts[prCfaMp4->u4CurAudInfoId]);
		break;
	case CFA_MP4_PRS_BIT_STRM_TYPE_SP:
		pSubInfo->u8StartPts = prCfaMp4->u8Spts[prCfaMp4->u4CurSubInfoId];
		pSubInfo->u8EndPts = INVALID_TIMESTAMP;
		if (prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].fgPG)
			pSubInfo->u8Dts = prCfaMp4->u8Spts[prCfaMp4->u4CurSubInfoId];
		else
			pSubInfo->u8Dts = INVALID_TIMESTAMP;

		if (prCfaMp4->u8Spts[prCfaMp4->u4CurSubInfoId] >= prCfaMp4->rCfaRange.u8VidPts) {
		} else {
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4][%s] Subtitle is High Light !\n"),
					 __func__);
		}
		DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] _CurSub(PTS): 0x%llx\n"),
				 __func__, prCfaMp4->u8Spts[prCfaMp4->u4CurSubInfoId]);
		break;
	case CFA_MP4_PRS_BIT_STRM_TYPE_NONE:
		pAudInfo->u8Pts = INVALID_TIMESTAMP;
		break;
	default:
		break;
	}
	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4TxAudHDRInfo
*
* Description:
*
*
* Inputs:
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaMp4TxAudHDRInfo(void *pvSptHdl, u32 u4TxUID, void *pvPrivData)
{
	CfaMp4Inst *prCfaMp4 = NULL;
	MRESULT mrRet = RET_DMX_OK;
	u32 i = 0;
	u32 u4AudioId = 0;

	/*modify for audio change id by zhiwei chen 2011.4.25*/
	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4][%s] ----TxAudHDRInfo u4TxUID:%d!\n"),
			__func__, u4TxUID);
	prCfaMp4 = (CfaMp4Inst *) pvPrivData;
	if (NULL == prCfaMp4)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	for (i = 0; i < prCfaMp4->u2AudStrmNums; i++)
		if (prCfaMp4->rCfaMp4AInf[i].u4TrackID == u4TxUID)
			u4AudioId = i;


	if ((DMX_INVALID_UINT32 == u4TxUID) &&
		   (AVCODEC_ID_AAC == prCfaMp4->rCfaMp4AInf[u4AudioId].eAudType)) {
		if (prCfaMp4->rCfaMp4AInf[u4AudioId].pucDecSpecInfo)
			prCfaMp4->rCfaMp4AInf[u4AudioId].fgTxAacSCDone = FALSE;

		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	if ((prCfaMp4->u4CurAudPlayId != u4TxUID) ||
		   (AVCODEC_ID_AAC != prCfaMp4->rCfaMp4AInf[u4AudioId].eAudType) ||
		   (!prCfaMp4->rCfaMp4AInf[u4AudioId].pucDecSpecInfo)) {
		MM_RETURN(RET_DMX_UNSUPPORT);
	}
	if (prCfaMp4->rCfaMp4AInf[u4AudioId].u4TrackID == u4TxUID
		&& AVCODEC_ID_AAC == prCfaMp4->rCfaMp4AInf[u4AudioId].eAudType) {
		if (prCfaMp4->rCfaMp4AInf[u4AudioId].pucDecSpecInfo) {
			mrRet =
				Spt4CfaBuf2AFifo(pvSptHdl, prCfaMp4->rCfaMp4AInf[u4AudioId].pucDecSpecInfo,
						prCfaMp4->rCfaMp4AInf[u4AudioId].u4DecSpecSz + 4,
						prCfaMp4->rCfaMp4AInf[u4AudioId].u4TrackID,
						prCfaMp4->rCfaMp4AInf[u4AudioId].eCfaAudType);
			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4] Spt4CfaBuf2AFifo error ret = %d\n"),
					mrRet);
			}
			MM_RETURN(mrRet);
		}
		MM_RETURN(RET_DMX_UNSUPPORT);
	}
	MM_RETURN(RET_DMX_UNSUPPORT);
}


/**
* < [IN] input splitter Handle
* < [IN] CFA function id, set or get id, it shall be defined by CFA and LPE
* < [IN] input CFA private data
* < [IN] The parameter of this FID, it shall be defined by CFA and LPE
* < [IN] The size of this parameter (of this FID), it shall be defined by CFA and LPE
*/
static MRESULT CfaMp4SetGeneral(void *pvSptHdl, u32 u4CfaFID,
				void *pvPrivData, void *pvCfaParameter,
				u32 u4CfaParameterSize)
{
	CfaMp4Inst *prCfaMp4 = (CfaMp4Inst *) pvPrivData;

	if (!pvPrivData || !pvCfaParameter)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	if (CFA_GENERAL_BADINT == u4CfaFID) {
		dmx_memcpy(&(prCfaMp4->rAvInterleaveChkParm), pvCfaParameter,
				sizeof(Mp4BadIntlvdCheck));
	}
	MM_RETURN(RET_DMX_OK);
}


/**
* < [IN] input splitter Handle
* < [IN] CFA function id, set or get id, it shall be defined by CFA and LPE
* < [IN] input CFA private data
* < [OUT] The parameter of this FID, it shall be defined by CFA and LPE
* < [IN] The size of this parameter (of this FID), it shall be defined by CFA and LPE
*/
static MRESULT CfaMp4GetGeneral(void *pvSptHdl, u32 u4CfaFID,
				void *pvPrivData, void *pvCfaParameter,
				u32 u4CfaParameterSize)
{
	CfaMp4Inst *prCfaMp4 = (CfaMp4Inst *) pvPrivData;
	bool fgBadInterLeave = FALSE;

	if (!pvPrivData || !pvCfaParameter)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	if (CFA_GENERAL_BADINT == u4CfaFID) {
		fgBadInterLeave = BadInterleaved(pvSptHdl, &(prCfaMp4->rAvInterleaveChkParm));
		mm_memcpy(pvCfaParameter, (void *) &fgBadInterLeave, u4CfaParameterSize);
		MM_RETURN(RET_DMX_OK);
	} else {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
}


/*-----------------------------------------------------------------------------
 * Name: CfaMP4SetJumpRange
 *
 * Description:
 *		Fro Support 8/16/32 fast forward and fast backward, to Reset CFA All state.
 *
 * Inputs:
 *		[IN] handle of splitter
 *		[IN] pointer to CfaMP4KeyFrameRange
 *		[IN] pointer to CfaMp4Inst
 *
 * Outputs:
 *
 * Returns: s32

 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaMP4SetJumpRange(void *pvSptHdl, void *pvJmpRange, void *pvPrivData)
{
	CfaMp4Inst *prCfaMp4 = NULL;
	CfaMP4KeyFrameRange *prCfaMP4KeyFrameRange = NULL;
	MRESULT mrResult = RET_DMX_OK;

	if ((NULL == pvPrivData) || (NULL == pvJmpRange))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	prCfaMp4 = (CfaMp4Inst *) pvPrivData;
	prCfaMP4KeyFrameRange = (CfaMP4KeyFrameRange *) pvJmpRange;
	MMATE_CHECK_POINTER(prCfaMp4);
	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] u8VidStartOffset[0x%x]\r\n"), __func__,
			   prCfaMP4KeyFrameRange->rCfaRangeInfo.u8VidStartOffset);
	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4][%s] u8AudStartOffset[0x%x]\r\n"), __func__,
			 prCfaMP4KeyFrameRange->rCfaRangeInfo.u8AudStartOffset[0]);
	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4][%s] u8SubStartOffset[0x%x]\r\n"), __func__,
			 prCfaMP4KeyFrameRange->rCfaRangeInfo.u8SubStartOffset[0]);
	mrResult =
		CfaMp4SetRange(pvSptHdl, (void *) (&(prCfaMP4KeyFrameRange->rCfaRangeInfo)), pvPrivData, TRUE);
	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaMP4GetParamSize(void *pvSptHdl, u32 u4ParamID,
					 void *pvPrivData, void *pvCfaParam, u32 u4CfaParamSz)
{
	MRESULT mrResult = RET_DMX_OK;

	switch (u4ParamID) {
	case CFA_PARAM_ID_JUMP_INFO_SIZE: {
		if ((NULL == pvCfaParam) || (u4CfaParamSz) < sizeof(u32)) {
			mrResult = RET_DMX_PARAM_WRONG;
		} else {
			u32 *pu4Tmp = (u32 *) pvCfaParam;
			*pu4Tmp = sizeof(CfaMP4KeyFrameRange);
		}
		break;
	}
	default:
		mrResult = RET_DMX_PARAM_WRONG;
		break;
	}
	MM_RETURN(mrResult);
}

static MRESULT CfaMp4ProcCliCmd(void *pvSptHdl, E_DMX_CFA_CLI_TYPE_T eCliType, /*< [IN] Cfa Cli Command*/
				u32 arg1,
				u32 arg2, u32 arg3, const char *szParam, void *pvPrivData)
{
	MRESULT mrRet = RET_DMX_OK;
	CfaMp4Inst *prCfaMp4 = NULL;

	prCfaMp4 = (CfaMp4Inst *) pvPrivData;

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

			DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("CfaMp4ProcCliCmd -- fgEnable: %d, Loglvl: %d, ModLogLvl: 0x%08x \r\n"),
				arg1, arg2, arg3);

			DmxLogEnable(fgEnable, arg2, DMX_MOD_CFA_MP4, arg3);
		}
		break;
	case DMX_CFA_CLI_CMD_DUMP_INFO:
		{
			DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("Cfa MP4 Instance(handle is 0x%x)")
				TEXT(" Info list as follow: \r\n"),
				prCfaMp4);
			DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("Current Analyse State is %d, ")
				TEXT("Current Parsing Flag is %d \r\n"),
				prCfaMp4->eCurCfaMp4AnaSt, prCfaMp4->u4CurPrsFlg);
			DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("Current Analyse Position is 0x%08x%08x, "),
				(u32) ((prCfaMp4->u8Ca) >> 32), (u32) (prCfaMp4->u8Ca));
		}
		break;
	default:
		break;
	}

	MM_RETURN(mrRet);
}

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
typedef struct {
	bool fgValid;
	bool fgIsCo64;
	__u32 u4EntryNs;
	compat_caddr_t pvEntry;
	__u64 u8FileOffset; /* point to "number of entries" field */
	__u32 u4Allotted;
	__u32 u4EntryLb;
	__u32 u4EntryUb;
	__u32 u4LastSmp;
	bool fgIndexErr;
} MP4_STBL_INFO32;

typedef struct {
	AVCODECID_T eVidType;
	bool fgCO64Valid;
	__u32 u4IPMPID;
	__u32 u4TrackID;	/* add by zhiwei chen mtk40495		  for streamid	  2011.3.17 */
	__u32 u4TimeScale;
	__u64 u8TimeDuration;
	compat_caddr_t pucDecSpecInfo;
	__u32 u4DecSpecSz;

	__u32 u4PicSize[MP4_STSD_TABLE_MAX_NUMS];
	__u32 u4SeqSize[MP4_STSD_TABLE_MAX_NUMS];
	__u32 u4VPSSize[MP4_STSD_TABLE_MAX_NUMS];
	__u8 u1PayLoadLength[MP4_STSD_TABLE_MAX_NUMS];
	__u8 u1VPSNum[MP4_STSD_TABLE_MAX_NUMS];
	compat_caddr_t pucVPSInfo[MP4_STSD_TABLE_MAX_NUMS];
	__u8 u1SeqParamNum[MP4_STSD_TABLE_MAX_NUMS];
	__u8 u1SeqParamNumbkp[MP4_STSD_TABLE_MAX_NUMS];
	compat_caddr_t pucSeqParamInfo[MP4_STSD_TABLE_MAX_NUMS];
	__u8 u1PicParamNum[MP4_STSD_TABLE_MAX_NUMS];
	__u8 u1PicParamNumbkp[MP4_STSD_TABLE_MAX_NUMS];
	compat_caddr_t pucPicParamInfo[MP4_STSD_TABLE_MAX_NUMS];

	__u32 u4SampSz;
	__u8 u1AvcSmpDesNums;
	compat_caddr_t prVTable;
}CfaMp4VidInfo32;

typedef struct {
	__u16 u2AudStrmNum;
	AVCODECID_T eAudType[MAX_NS_MP4_AUD];
	__u32 u4IPMPID[MAX_NS_MP4_AUD];
	__u32 u4TrackID[MAX_NS_MP4_AUD];	/*	streamid */
	bool fgCO64Valid[MAX_NS_MP4_AUD];
	__u16 u2AudChannels[MAX_NS_MP4_AUD];
	compat_caddr_t pucDecSpecInfo[MAX_NS_MP4_AUD];
	__u32 u4DecSpecSz[MAX_NS_MP4_AUD];
	__u32 u4AudBitRate[MAX_NS_MP4_AUD];
	__u32 u4AudSamplePerSec[MAX_NS_MP4_AUD];
	__u32 u4TimeScale[MAX_NS_MP4_AUD];
	__u64 u8TimeDuration[MAX_NS_MP4_AUD];
	__u32 u4SampSz[MAX_NS_MP4_AUD];
	compat_caddr_t prATable[MAX_NS_MP4_AUD];
} CfaMp4AudInfo32;

typedef struct {
	__u16 u2SubStrmNum;
	bool fgPG[MAX_NS_MP4_SUB];
	__u32 u4IPMPID[MAX_NS_MP4_SUB];
	__u32 u4TrackID[MAX_NS_MP4_SUB];	/* streamid  */
	bool fgCO64Valid[MAX_NS_MP4_SUB];
	__u32 u4TimeScale[MAX_NS_MP4_SUB];
	__u64 u8TimeDuration[MAX_NS_MP4_SUB];
	__u32 u4SampSz[MAX_NS_MP4_SUB];
	MP4_SUB_TYPE eSubType[MAX_NS_MP4_SUB];
	compat_caddr_t prSTable[MAX_NS_MP4_SUB];
	compat_caddr_t prMFSTable[MAX_NS_MP4_SUB];
} CfaMp4SubInfo32;

typedef struct {
	/* V info */
	CfaMp4VidInfo32 rCfaMp4VidInfo;

	/* A info */
	CfaMp4AudInfo32 rCfaMp4AudInfo;

	/* subtitle info */
	CfaMp4SubInfo32 rCfaMp4SubInfo;

#if MP4_SUPPORT_FRAGMENT
	MP4_FRAGMENT_TYPE eMoofType;
	__u64 u8MoofOffset;
#endif

} CfaMp4ConfigInfo32;

/* Must be compatible with MPC2Mp4PR */
typedef struct {
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKStart;
#endif
	__u64 u8Length;
	bool fgFirstRange;
	/* video */
	__u64 u8VidStartChunkNo;
	__u64 u8VidStartChunk1stSmp;
	__u64 u8VidStartSampleNo;
	__u64 u8VidStartOffset;
	__u64 u8VidEndOffset;

	__u64 u8VidEndChunkNo;
	__u64 u8VidEndSampleNo;
	__u64 u8VidPts;

	/* audio */
	__u64 u8AudStartChunkNo[MAX_NS_MP4_AUD];
	__u64 u8AudStartChunk1stSmp[MAX_NS_MP4_AUD];
	__u64 u8AudStartSampleNo[MAX_NS_MP4_AUD];
	__u64 u8AudStartOffset[MAX_NS_MP4_AUD];
	__u64 u8AudEndOffset[MAX_NS_MP4_AUD];
	__u64 u8AudEndChunkNo[MAX_NS_MP4_AUD];
	__u64 u8AudEndSampleNo[MAX_NS_MP4_AUD];
	__u64 u8AudPts[MAX_NS_MP4_AUD];

	/* subpicture */
	__u64 u8SubStartChunkNo[MAX_NS_MP4_SUB];
	__u64 u8SubStartChunk1stSmp[MAX_NS_MP4_SUB];
	__u64 u8SubStartSampleNo[MAX_NS_MP4_SUB];
	__u64 u8SubStartOffset[MAX_NS_MP4_SUB];
	__u64 u8SubEndOffset[MAX_NS_MP4_SUB];
	__u64 u8SubEndChunkNo[MAX_NS_MP4_SUB];
	__u64 u8SubEndSampleNo[MAX_NS_MP4_SUB];
	__u64 u8SubPts[MAX_NS_MP4_SUB];

	__u32 u4PrsFlag;
	__u64 u8StartPts;
	__u64 u8SeekPts;	/* add by zhiwei chen */

	__u64 u8MoovSkipSize;	/* for some file into at begin of moov box ,need skip it */
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKEnd;
#endif
} CfaMp4Range32;

typedef struct {
	CfaMp4Range32 rCfaRangeInfo;
} CfaMP4KeyFrameRange32;

static long CfaMp4CompatSTBLInfo(MP4_STBL_INFO __user **pusr_ptr,
	MP4_STBL_INFO32 __user *pucTmpTable, __u8 **ppu1NextUserBuf, __u8 *pu1UsrBufAddr, __u32 u4TotalSz)
{

	int i = 0;
	__u32 u4EntrySize = 0;
	__u32 u4Offset = 0;
	__u32 u4Size = 0;

	MP4_STBL_INFO __user *usr_ptr = NULL;

	*pusr_ptr = NULL;

	usr_ptr = *ppu1NextUserBuf;

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}

	u4Size= CFA_ALIGN_SZ(sizeof(MP4_STBL_INFO) * MAX_MP4_SMP_TBL, sizeof(uintptr_t));
	u4Offset = (__u32)(*ppu1NextUserBuf - pu1UsrBufAddr) + u4Size;
	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("%s line %d u4Offset:%d.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, u4Offset);
	if (u4Offset > u4TotalSz)
	{
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in u4Offset > u4TotalSz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}

	mm_memset(usr_ptr, 0, sizeof(MP4_STBL_INFO) * MAX_MP4_SMP_TBL);
	*ppu1NextUserBuf += u4Size;

	if (!access_ok(VERIFY_READ, pucTmpTable, sizeof(MP4_STBL_INFO) * MAX_MP4_SMP_TBL))
	{
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d error.\r\n"),	DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	for (i = 0; i < MAX_MP4_SMP_TBL; i++) {
		if (copy_from_user(&(usr_ptr[i].fgValid), &(pucTmpTable[i].fgValid), sizeof(bool)))
		{
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("%s line %d error.\r\n"),	DMX_FUNC_NAME, DMX_LINE_NO);
			return -EFAULT;
		}

		if (copy_from_user(&(usr_ptr[i].fgIsCo64), &(pucTmpTable[i].fgIsCo64), sizeof(bool)))
		{
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("%s line %d error.\r\n"),	DMX_FUNC_NAME, DMX_LINE_NO);
			return -EFAULT;
		}

		if (copy_from_user(&(usr_ptr[i].u4EntryNs), &(pucTmpTable[i].u4EntryNs), sizeof(__u32)))
		{
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("%s line %d error.\r\n"),	DMX_FUNC_NAME, DMX_LINE_NO);
			return -EFAULT;
		}

		if (STTS == i) {
			u4EntrySize = pucTmpTable[i].u4EntryNs * ENTRY_SIZE_STTS;
		}

		else if (STSC == i) {
			u4EntrySize = pucTmpTable[i].u4EntryNs * ENTRY_SIZE_STSC;
		}

		else if (STSZ == i)
			u4EntrySize = pucTmpTable[i].u4EntryNs * ENTRY_SIZE_STSZ;


		else if (STCO == i) {
			if (pucTmpTable[i].fgIsCo64)
				u4EntrySize = pucTmpTable[i].u4EntryNs * ENTRY_SIZE_CO64;


			else if (!pucTmpTable[i].fgIsCo64)
				u4EntrySize = pucTmpTable[i].u4EntryNs * ENTRY_SIZE_STCO;

		}
		else if (STSD == i) {
			u4EntrySize = pucTmpTable[i].u4EntryNs * ENTRY_SIZE_STSD;
		}

		if (u4EntrySize && pucTmpTable[i].pvEntry) {
			void *pucTmpEntry = NULL;
			compat_caddr_t compatEntry = 0;

			usr_ptr[i].pvEntry =  *ppu1NextUserBuf;

			if (NULL == usr_ptr[i].pvEntry) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail in alloc compat user space.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -ENOMEM;
			}

			u4Size= CFA_ALIGN_SZ((__u32)(sizeof(__u8) * u4EntrySize), sizeof(uintptr_t));
			u4Offset = (__u32)(*ppu1NextUserBuf - pu1UsrBufAddr) + u4Size;
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("%s line %d u4Offset:%d.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Offset);
			if (u4Offset > u4TotalSz)
			{
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail in u4Offset > u4TotalSz.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -ENOMEM;
			}

			mm_memset(usr_ptr[i].pvEntry, 0, sizeof(__u8) * u4EntrySize);
			*ppu1NextUserBuf += u4Size;

			if (get_user(compatEntry, &(pucTmpTable[i].pvEntry)))
			{
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d error.\r\n"),	DMX_FUNC_NAME, DMX_LINE_NO);
				return -EFAULT;
			}

			if (0 == compatEntry)
				return -EFAULT;

			pucTmpEntry = compat_ptr(compatEntry);

			if (!access_ok(VERIFY_READ, pucTmpEntry, u4EntrySize ))
				return -EFAULT;

			if (copy_from_user((void __user *)usr_ptr[i].pvEntry,
					pucTmpEntry, u4EntrySize ))
			{
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d usr_ptr[i].pvEntry:%p ,pucTmpEntry:%p.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,(void __user *)usr_ptr[i].pvEntry,pucTmpEntry);
				return -EFAULT;
			}
		}

		if (copy_from_user(&(usr_ptr[i].u8FileOffset), &(pucTmpTable[i].u8FileOffset), sizeof(__u64)))
			return -EFAULT;
		if (copy_from_user(&(usr_ptr[i].u4Allotted), &(pucTmpTable[i].u4Allotted), sizeof(__u32)))
			return -EFAULT;
		if (copy_from_user(&(usr_ptr[i].u4EntryLb), &(pucTmpTable[i].u4EntryLb), sizeof(__u32)))
			return -EFAULT;
		if (copy_from_user(&(usr_ptr[i].u4EntryUb), &(pucTmpTable[i].u4EntryUb), sizeof(__u32)))
			return -EFAULT;
		if (copy_from_user(&(usr_ptr[i].u4LastSmp), &(pucTmpTable[i].u4LastSmp), sizeof(__u32)))
			return -EFAULT;
		if (copy_from_user(&(usr_ptr[i].fgIndexErr), &(pucTmpTable[i].fgIndexErr), sizeof(bool)))
			return -EFAULT;
	}

	*pusr_ptr = usr_ptr;

	return 0;
}
static long CfaMp4CompatVidInfo(CfaMp4VidInfo __user *usr_ptr,
	CfaMp4VidInfo32 __user *usr_ptr32, __u8 **ppu1NextUserBuf, __u8 *pu1UsrBufAddr, __u32 u4TotalSz)
{
	int index = 0;
	long ret = 0;
	__u32 u4Offset = 0;
	__u32 u4Size = 0;

	if (copy_from_user(&(usr_ptr->eVidType), &(usr_ptr32->eVidType), sizeof(AVCODECID_T)))
		return -EFAULT;

	if (copy_from_user(&(usr_ptr->fgCO64Valid), &(usr_ptr32->fgCO64Valid), sizeof(bool)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4IPMPID), &(usr_ptr32->u4IPMPID), sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4TrackID), &(usr_ptr32->u4TrackID), sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4TimeScale), &(usr_ptr32->u4TimeScale), sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8TimeDuration), &(usr_ptr32->u8TimeDuration), sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4DecSpecSz), &(usr_ptr32->u4DecSpecSz), sizeof(__u32)))
		return -EFAULT;

	if (0 != usr_ptr32->pucDecSpecInfo) {
		__u8 *pucTmpDecSpecInfo = NULL;
		compat_caddr_t compatDecSpecInfo = 0;

		usr_ptr->pucDecSpecInfo =  *ppu1NextUserBuf;

		if (NULL == usr_ptr->pucDecSpecInfo) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("%s line %d fail in alloc compat user space.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return -ENOMEM;
		}

		u4Size = CFA_ALIGN_SZ((__u32)(sizeof(__u8) * usr_ptr->u4DecSpecSz), sizeof(uintptr_t));
		u4Offset = (__u32)(*ppu1NextUserBuf - pu1UsrBufAddr) + u4Size;
		DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d u4Offset:%d.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Offset);
		if (u4Offset > u4TotalSz)
		{
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("%s line %d fail in u4Offset > u4TotalSz.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return -ENOMEM;
		}

		mm_memset(usr_ptr->pucDecSpecInfo, 0, sizeof(__u8) * usr_ptr->u4DecSpecSz);
		*ppu1NextUserBuf += u4Size;

		if (get_user(compatDecSpecInfo, &(usr_ptr32->pucDecSpecInfo)))
			return -EFAULT;

		if (0 == compatDecSpecInfo)
			return -EFAULT;

		pucTmpDecSpecInfo = compat_ptr(compatDecSpecInfo);

		if (!access_ok(VERIFY_READ, pucTmpDecSpecInfo, sizeof(__u8) * usr_ptr->u4DecSpecSz))
			return -EFAULT;

		if (copy_from_user((__u8 __user *)usr_ptr->pucDecSpecInfo,
				pucTmpDecSpecInfo, sizeof(__u8) * usr_ptr->u4DecSpecSz))
			return -EFAULT;
	}

	if (copy_from_user(usr_ptr->u4PicSize, usr_ptr32->u4PicSize, sizeof(__u32) * MP4_STSD_TABLE_MAX_NUMS))
		return -EFAULT;
	if (copy_from_user(usr_ptr->u4SeqSize, usr_ptr32->u4SeqSize, sizeof(__u32) * MP4_STSD_TABLE_MAX_NUMS))
		return -EFAULT;
	if (copy_from_user(usr_ptr->u4VPSSize, usr_ptr32->u4VPSSize, sizeof(__u32) * MP4_STSD_TABLE_MAX_NUMS))
		return -EFAULT;
	if (copy_from_user(usr_ptr->u1PayLoadLength, usr_ptr32->u1PayLoadLength, sizeof(__u8) * MP4_STSD_TABLE_MAX_NUMS))
		return -EFAULT;
	if (copy_from_user(usr_ptr->u1VPSNum, usr_ptr32->u1VPSNum, sizeof(__u8) * MP4_STSD_TABLE_MAX_NUMS))
		return -EFAULT;

	for(index = 0; index < usr_ptr32->u1AvcSmpDesNums; index++)
	{
		if (0 != usr_ptr32->pucVPSInfo[index]) {
			__u8 *pucTmpVPSInfo = NULL;
			compat_caddr_t compatVPSInfo = 0;

			usr_ptr->pucVPSInfo[index] =  *ppu1NextUserBuf;

			if (NULL == (usr_ptr->pucVPSInfo[index])) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail in alloc compat user space.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -ENOMEM;
			}

			u4Size = CFA_ALIGN_SZ((__u32)(sizeof(__u8) * usr_ptr->u4VPSSize[index]), sizeof(uintptr_t));
			u4Offset = (__u32)(*ppu1NextUserBuf - pu1UsrBufAddr) + u4Size;
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("%s line %d u4Offset:%d.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Offset);
			if (u4Offset > u4TotalSz)
			{
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail in u4Offset > u4TotalSz.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -ENOMEM;
			}

			mm_memset(usr_ptr->pucVPSInfo[index], 0, sizeof(__u8) * (usr_ptr->u4VPSSize[index]));
			*ppu1NextUserBuf += u4Size;

			if (get_user(compatVPSInfo, &(usr_ptr32->pucVPSInfo[index])))
				return -EFAULT;

			if (0 == compatVPSInfo)
				return -EFAULT;

			pucTmpVPSInfo = compat_ptr(compatVPSInfo);

			if (!access_ok(VERIFY_READ, pucTmpVPSInfo, sizeof(__u8) * (usr_ptr->u4VPSSize[index])))
				return -EFAULT;

			if (copy_from_user((__u8 __user *)(usr_ptr->pucVPSInfo[index]),
					pucTmpVPSInfo, sizeof(__u8) * (usr_ptr->u4VPSSize[index])))
				return -EFAULT;
		}
	}

	if (copy_from_user(usr_ptr->u1SeqParamNum, usr_ptr32->u1SeqParamNum, sizeof(__u8) * MP4_STSD_TABLE_MAX_NUMS))
		return -EFAULT;
	if (copy_from_user(usr_ptr->u1SeqParamNumbkp, usr_ptr32->u1SeqParamNumbkp, sizeof(__u8) * MP4_STSD_TABLE_MAX_NUMS))
		return -EFAULT;

	for(index = 0; index < usr_ptr32->u1AvcSmpDesNums; index++)
	{
		if (0 != usr_ptr32->pucSeqParamInfo[index]) {
			__u8 *pucTmpSeqParamInfo = NULL;
			compat_caddr_t compatSeqParamInfo = 0;

			usr_ptr->pucSeqParamInfo[index] =  *ppu1NextUserBuf;

			if (NULL == (usr_ptr->pucSeqParamInfo[index])) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail in alloc compat user space.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -ENOMEM;
			}

			u4Size = CFA_ALIGN_SZ((__u32)(sizeof(__u8) * usr_ptr->u4SeqSize[index]), sizeof(uintptr_t));
			u4Offset = (__u32)(*ppu1NextUserBuf - pu1UsrBufAddr) + u4Size;
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("%s line %d u4Offset:%d.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Offset);
			if (u4Offset > u4TotalSz)
			{
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail in u4Offset > u4TotalSz.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -ENOMEM;
			}

			mm_memset(usr_ptr->pucSeqParamInfo[index], 0, sizeof(__u8) * (usr_ptr->u4SeqSize[index]));
			*ppu1NextUserBuf += u4Size;

			if (get_user(compatSeqParamInfo, &(usr_ptr32->pucSeqParamInfo[index])))
				return -EFAULT;

			if (0 == compatSeqParamInfo)
				return -EFAULT;

			 pucTmpSeqParamInfo = compat_ptr(compatSeqParamInfo);

			if (!access_ok(VERIFY_READ, pucTmpSeqParamInfo, sizeof(__u8) * (usr_ptr->u4SeqSize[index])))
				return -EFAULT;

			if (copy_from_user((__u8 __user *)(usr_ptr->pucSeqParamInfo[index]),
					pucTmpSeqParamInfo, sizeof(__u8) * (usr_ptr->u4SeqSize[index])))
				return -EFAULT;
		}
	}

	if (copy_from_user(usr_ptr->u1PicParamNum, usr_ptr32->u1PicParamNum, sizeof(__u8) * MP4_STSD_TABLE_MAX_NUMS))
		return -EFAULT;
	if (copy_from_user(usr_ptr->u1PicParamNumbkp, usr_ptr32->u1PicParamNumbkp, sizeof(__u8) * MP4_STSD_TABLE_MAX_NUMS))
		return -EFAULT;

	for(index = 0; index < usr_ptr32->u1AvcSmpDesNums; index++)
	{
		if (0 != usr_ptr32->pucPicParamInfo[index]) {
			__u8 *pucTmpPicParamInfo = NULL;
			compat_caddr_t compatPicParamInfo = 0;

			usr_ptr->pucPicParamInfo[index] =  *ppu1NextUserBuf;

			if (NULL == (usr_ptr->pucPicParamInfo[index])) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail in alloc compat user space.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -ENOMEM;
			}

			u4Size= CFA_ALIGN_SZ((__u32)(sizeof(__u8) * usr_ptr->u4PicSize[index]), sizeof(uintptr_t));
			u4Offset = (__u32)(*ppu1NextUserBuf - pu1UsrBufAddr) + u4Size;
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("%s line %d u4Offset:%d.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Offset);
			if (u4Offset > u4TotalSz)
			{
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail in u4Offset > u4TotalSz.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -ENOMEM;
			}

			mm_memset(usr_ptr->pucPicParamInfo[index], 0, sizeof(__u8) * (usr_ptr->u4PicSize[index]));
			*ppu1NextUserBuf += u4Size;

			if (get_user(compatPicParamInfo, &(usr_ptr32->pucPicParamInfo[index])))
				return -EFAULT;

			if (0 == compatPicParamInfo)
				return -EFAULT;

			pucTmpPicParamInfo = compat_ptr(compatPicParamInfo);

			if (!access_ok(VERIFY_READ, pucTmpPicParamInfo, sizeof(__u8) * (usr_ptr->u4PicSize[index])))
				return -EFAULT;

			if (copy_from_user((__u8 __user *)(usr_ptr->pucPicParamInfo[index]),
					pucTmpPicParamInfo, sizeof(__u8) * (usr_ptr->u4PicSize[index])))
				return -EFAULT;
		}
	}
	if (copy_from_user(&(usr_ptr->u4SampSz), &(usr_ptr32->u4SampSz), sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u1AvcSmpDesNums), &(usr_ptr32->u1AvcSmpDesNums), sizeof(__u8)))
		return -EFAULT;
	if (0 != usr_ptr32->prVTable) {
		compat_caddr_t compatVTable = 0;
		MP4_STBL_INFO32 __user *pucTmpVTable = NULL;
		if (get_user(compatVTable, &(usr_ptr32->prVTable)))
			return -EFAULT;
		if (0 == compatVTable)
			return -EFAULT;

		pucTmpVTable = compat_ptr(compatVTable);

		ret = CfaMp4CompatSTBLInfo(&(usr_ptr->prVTable), pucTmpVTable, ppu1NextUserBuf, pu1UsrBufAddr, u4TotalSz);
		if (0 != ret) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("%s line %d fail in CfaMp4CompatSTBLInfo.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return ret;
		}
	}

	return 0;
}
static long CfaMp4CompatAudInfo(CfaMp4AudInfo __user *usr_ptr,
	CfaMp4AudInfo32 __user *usr_ptr32, __u8 **ppu1NextUserBuf, __u8 *pu1UsrBufAddr, __u32 u4TotalSz)
{
	int index = 0;
	long ret = 0;
	__u32 u4Offset = 0;
	__u32 u4Size = 0;

	if (copy_from_user(&(usr_ptr->u2AudStrmNum), &(usr_ptr32->u2AudStrmNum), sizeof(__u16)))
		return -EFAULT;

	if (copy_from_user(usr_ptr->eAudType,
			usr_ptr32->eAudType, sizeof(AVCODECID_T) * MAX_NS_MP4_AUD))
		return -EFAULT;
	if (copy_from_user(usr_ptr->u4IPMPID,
			usr_ptr32->u4IPMPID, sizeof(__u32) * MAX_NS_MP4_AUD))
		return -EFAULT;
	if (copy_from_user(usr_ptr->u4TrackID,
			usr_ptr32->u4TrackID, sizeof(__u32) * MAX_NS_MP4_AUD))
		return -EFAULT;
	if (copy_from_user(usr_ptr->fgCO64Valid,
			usr_ptr32->fgCO64Valid, sizeof(bool) * MAX_NS_MP4_AUD))
		return -EFAULT;
	if (copy_from_user(usr_ptr->u2AudChannels,
			usr_ptr32->u2AudChannels, sizeof(__u16) * MAX_NS_MP4_AUD))
		return -EFAULT;
	if (copy_from_user(usr_ptr->u4DecSpecSz,
			usr_ptr32->u4DecSpecSz, sizeof(__u32) * MAX_NS_MP4_AUD))
		return -EFAULT;
	for(index = 0; index < usr_ptr32->u2AudStrmNum; index++)
	{
		if (0 != (usr_ptr32->pucDecSpecInfo[index])) {
			__u8 *pucTmpDecSpecInfo = NULL;
			compat_caddr_t compatDecSpecInfo = 0;

			usr_ptr->pucDecSpecInfo[index] =  *ppu1NextUserBuf;

			if (NULL == (usr_ptr->pucDecSpecInfo[index])) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail in alloc compat user space.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -ENOMEM;
			}

			u4Size= CFA_ALIGN_SZ((__u32)(sizeof(__u8) * usr_ptr->u4DecSpecSz[index]), sizeof(uintptr_t));
			u4Offset = (__u32)(*ppu1NextUserBuf - pu1UsrBufAddr) + u4Size;
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("%s line %d u4Offset:%d.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4Offset);
			if (u4Offset > u4TotalSz)
			{
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail in u4Offset > u4TotalSz.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -ENOMEM;
			}

			mm_memset(usr_ptr->pucDecSpecInfo[index], 0, sizeof(__u8) * (usr_ptr->u4DecSpecSz[index]));
			*ppu1NextUserBuf += u4Size;

			if (get_user(compatDecSpecInfo, &(usr_ptr32->pucDecSpecInfo[index])))
				return -EFAULT;

			if (0 == compatDecSpecInfo)
				return -EFAULT;

			pucTmpDecSpecInfo = compat_ptr(compatDecSpecInfo);

			if (!access_ok(VERIFY_READ, pucTmpDecSpecInfo, sizeof(__u8) * (usr_ptr->u4DecSpecSz[index])))
				return -EFAULT;

			if (copy_from_user((__u8 __user *)(usr_ptr->pucDecSpecInfo[index]),
					pucTmpDecSpecInfo, sizeof(__u8) * (usr_ptr->u4DecSpecSz[index])))
				return -EFAULT;
		}

		if (0 != usr_ptr32->prATable[index] ) {
			compat_caddr_t compatATable = 0;
			MP4_STBL_INFO32 __user *pucTmpATable = NULL;
			if (get_user(compatATable, &(usr_ptr32->prATable[index])))
				return -EFAULT;
			if (0 == compatATable)
				return -EFAULT;

			pucTmpATable = compat_ptr(compatATable);

			ret = CfaMp4CompatSTBLInfo(&(usr_ptr->prATable[index]), pucTmpATable, ppu1NextUserBuf, pu1UsrBufAddr, u4TotalSz);
			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaMp4CompatSTBLInfo.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return ret;
			}
		}
	}
	if (copy_from_user(usr_ptr->u4AudBitRate,
			usr_ptr32->u4AudBitRate, sizeof(__u32) * MAX_NS_MP4_AUD))
		return -EFAULT;
	if (copy_from_user(usr_ptr->u4AudSamplePerSec,
			usr_ptr32->u4AudSamplePerSec, sizeof(__u32) * MAX_NS_MP4_AUD))
		return -EFAULT;
	if (copy_from_user(usr_ptr->u4TimeScale,
			usr_ptr32->u4TimeScale, sizeof(__u32) * MAX_NS_MP4_AUD))
		return -EFAULT;
	if (copy_from_user(usr_ptr->u8TimeDuration,
			usr_ptr32->u8TimeDuration, sizeof(__u64) * MAX_NS_MP4_AUD))
		return -EFAULT;
	if (copy_from_user(usr_ptr->u4SampSz,
			usr_ptr32->u4SampSz, sizeof(__u32) * MAX_NS_MP4_AUD))
		return -EFAULT;

	return 0;
}

static long CfaMp4CompatSubInfo(CfaMp4SubInfo __user *usr_ptr,
	CfaMp4SubInfo32 __user *usr_ptr32, __u8 **ppu1NextUserBuf, __u8 *pu1UsrBufAddr, __u32 u4TotalSz)
{
	int index = 0;
	long ret = 0;
	__u32 u4Offset = 0;
	__u32 u4Size = 0;

	if (copy_from_user(&(usr_ptr->u2SubStrmNum), &(usr_ptr32->u2SubStrmNum), sizeof(__u16)))
		return -EFAULT;

	if (copy_from_user(usr_ptr->fgPG,
			usr_ptr32->fgPG, sizeof(bool) * MAX_NS_MP4_SUB))
		return -EFAULT;
	if (copy_from_user(usr_ptr->u4IPMPID,
			usr_ptr32->u4IPMPID, sizeof(__u32) * MAX_NS_MP4_SUB))
		return -EFAULT;
	if (copy_from_user(usr_ptr->u4TrackID,
			usr_ptr32->u4TrackID, sizeof(__u32) * MAX_NS_MP4_SUB))
		return -EFAULT;
	if (copy_from_user(usr_ptr->fgCO64Valid,
			usr_ptr32->fgCO64Valid, sizeof(bool) * MAX_NS_MP4_SUB))
		return -EFAULT;
	if (copy_from_user(usr_ptr->u4TimeScale,
			usr_ptr32->u4TimeScale, sizeof(__u32) * MAX_NS_MP4_SUB))
		return -EFAULT;
	if (copy_from_user(usr_ptr->u8TimeDuration,
			usr_ptr32->u8TimeDuration, sizeof(__u64) * MAX_NS_MP4_SUB))
		return -EFAULT;
	if (copy_from_user(usr_ptr->u4SampSz,
			usr_ptr32->u4SampSz, sizeof(__u32) * MAX_NS_MP4_SUB))
		return -EFAULT;
	if (copy_from_user(usr_ptr->eSubType,
			usr_ptr32->eSubType, sizeof(MP4_SUB_TYPE) * MAX_NS_MP4_SUB))
		return -EFAULT;
	for(index = 0; index < usr_ptr32->u2SubStrmNum; index++)
	{
		if (0 != usr_ptr32->prSTable[index] ) {
			compat_caddr_t compatSTable = 0;
			MP4_STBL_INFO32 __user *pucTmpSTable = NULL;
			if (get_user(compatSTable, &(usr_ptr32->prSTable[index])))
				return -EFAULT;
			if (0 == compatSTable)
				return -EFAULT;

			pucTmpSTable = compat_ptr(compatSTable);

			ret = CfaMp4CompatSTBLInfo(&(usr_ptr->prSTable[index]), pucTmpSTable, ppu1NextUserBuf, pu1UsrBufAddr, u4TotalSz);

			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaMp4CompatSTBLInfo.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return ret;
			}
		}
		if (0 != usr_ptr32->prMFSTable[index]) {
			compat_caddr_t compatMFSTable = 0;
			MP4_STBL_INFO32 __user *pucTmpMFSTable = NULL;
			if (get_user(compatMFSTable, &(usr_ptr32->prMFSTable[index])))
				return -EFAULT;
			if (0 == compatMFSTable)
				return -EFAULT;

			pucTmpMFSTable = compat_ptr(compatMFSTable);

			ret = CfaMp4CompatSTBLInfo(&(usr_ptr->prMFSTable[index]), pucTmpMFSTable, ppu1NextUserBuf, pu1UsrBufAddr, u4TotalSz);

			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaMp4CompatSTBLInfo.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return ret;
			}
		}
	}

	return 0;
}
static long CfaMp4CompatSTBLCalcSz(MP4_STBL_INFO32 __user *usr_ptr32,
	__u32 *pu4OutSz)
{
	__u32 u4TotalSz = 0;
	__u32 u4HeaderLen = 0;
	__u32 i = 0;
	long ret = 0;

	if ((NULL == usr_ptr32) || (NULL == pu4OutSz)) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	u4TotalSz += CFA_ALIGN_SZ((__u32)sizeof(MP4_STBL_INFO) * MAX_MP4_SMP_TBL, sizeof(uintptr_t));

	for (i = 0; i < MAX_MP4_SMP_TBL; i++) {
		ret = get_user(u4HeaderLen, &(usr_ptr32[i].u4EntryNs));
		if (ret != 0) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in get_user(usr_ptr32[i].u4EntryNs).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, i);
			return -EFAULT;
		}
		if (STTS == i) {
			u4HeaderLen = u4HeaderLen * ENTRY_SIZE_STTS;
		}

		else if (STSC == i) {
			u4HeaderLen = u4HeaderLen * ENTRY_SIZE_STSC;
		}

		else if (STSZ == i) {
			u4HeaderLen = u4HeaderLen * ENTRY_SIZE_STSZ;
		}

		else if (STCO == i) {
			if (usr_ptr32[i].fgIsCo64){
				u4HeaderLen = u4HeaderLen * ENTRY_SIZE_CO64;
			}
			else if (!usr_ptr32[i].fgIsCo64){
				u4HeaderLen = u4HeaderLen * ENTRY_SIZE_STCO;
			}
		}
		else if (STSD == i) {
			u4HeaderLen = u4HeaderLen * ENTRY_SIZE_STSD;
		}
		if (u4HeaderLen && usr_ptr32[i].pvEntry) {
			u4TotalSz += CFA_ALIGN_SZ((__u32)(sizeof(__u8) * u4HeaderLen), sizeof(uintptr_t));
		}
	}
	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d exit, (0x%08x).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,u4TotalSz);
	*pu4OutSz = u4TotalSz;
	return 0;
}

static long CfaMp4CompatConfigCalcSz(CfaMp4ConfigInfo32 __user *usr_ptr32,
	__u32 *pu4OutSz)
{
	__u32 u4TotalSz = 0;
	__u32 u4HeaderLen = 0;
	__u32 i = 0;
	long ret = 0;

	if ((NULL == usr_ptr32) || (NULL == pu4OutSz)) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	u4TotalSz += CFA_ALIGN_SZ((__u32)sizeof(CfaMp4ConfigInfo), sizeof(uintptr_t));

	if(NULL != usr_ptr32->rCfaMp4VidInfo.pucDecSpecInfo){
		if (0 != get_user(u4HeaderLen,	&(usr_ptr32->rCfaMp4VidInfo.u4DecSpecSz))) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("%s line %d fail in get_user(rCfaMp4VidInfo.u4DecSpecSz)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return -EFAULT;
		}

		u4TotalSz += CFA_ALIGN_SZ((__u32)u4HeaderLen, sizeof(uintptr_t));
	}

	for (i = 0; i < usr_ptr32->rCfaMp4VidInfo.u1AvcSmpDesNums; i++)
	{
		if (0 != usr_ptr32->rCfaMp4VidInfo.pucPicParamInfo[i]) {
			ret = get_user(u4HeaderLen, &(usr_ptr32->rCfaMp4VidInfo.u4PicSize[i]));
			if (ret != 0) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("%s line %d fail in get_user(rCfaMp4VidInfo.u4PicSize[i]).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, i);
				return -EFAULT;
			}
			u4TotalSz += CFA_ALIGN_SZ(u4HeaderLen, sizeof(uintptr_t));
		}

		if (0 != usr_ptr32->rCfaMp4VidInfo.pucSeqParamInfo[i]) {
			ret = get_user(u4HeaderLen, &(usr_ptr32->rCfaMp4VidInfo.u4SeqSize[i]));
			if (ret != 0) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("%s line %d fail in get_user(rCfaMp4VidInfo.u4SeqSize[i]).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, i);
				return -EFAULT;
			}
			u4TotalSz += CFA_ALIGN_SZ(u4HeaderLen, sizeof(uintptr_t));
		}

		if (0 != usr_ptr32->rCfaMp4VidInfo.pucVPSInfo[i]) {
			ret = get_user(u4HeaderLen, &(usr_ptr32->rCfaMp4VidInfo.u4VPSSize[i]));
			if (ret != 0) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("%s line %d fail in get_user(rCfaMp4VidInfo.u4VPSSize[i]).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, i);
				return -EFAULT;
			}
			u4TotalSz += CFA_ALIGN_SZ(u4HeaderLen, sizeof(uintptr_t));
		}
	}

	if (0 != usr_ptr32->rCfaMp4VidInfo.prVTable){ 
		compat_caddr_t compatVTable = 0;
		MP4_STBL_INFO32 __user *pucTmpVTable = NULL;
		if (get_user(compatVTable, &(usr_ptr32->rCfaMp4VidInfo.prVTable)))
			return -EFAULT;
		if (0 == compatVTable)
			return -EFAULT;

		pucTmpVTable = (MP4_STBL_INFO32 __user *)compat_ptr(compatVTable);
		if(0 != CfaMp4CompatSTBLCalcSz(pucTmpVTable, &u4HeaderLen)) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("%s line %d fail in CfaMp4CompatSTBLCalcSz.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return -EFAULT;
		}

		u4TotalSz += u4HeaderLen;//CFA_ALIGN_SZ(u4HeaderLen, sizeof(uintptr_t));
	}

	for (i = 0; i < usr_ptr32->rCfaMp4AudInfo.u2AudStrmNum; i++)
	{
		if (0 != usr_ptr32->rCfaMp4AudInfo.pucDecSpecInfo[i]){
			ret = get_user(u4HeaderLen, &(usr_ptr32->rCfaMp4AudInfo.u4DecSpecSz[i]));
			if (ret != 0) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("%s line %d fail in get_user(rCfaMp4AudInfo.u4DecSpecSz[i]).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, i);
				return -EFAULT;
			}
			u4TotalSz += CFA_ALIGN_SZ(u4HeaderLen, sizeof(uintptr_t));
		}
		if (0 != usr_ptr32->rCfaMp4AudInfo.prATable[i]){
			compat_caddr_t compatVTable = 0;
			MP4_STBL_INFO32 __user *pucTmpVTable = NULL;
			if (get_user(compatVTable, &(usr_ptr32->rCfaMp4AudInfo.prATable[i])))
				return -EFAULT;
			if (0 == compatVTable)
				return -EFAULT;

			pucTmpVTable = (MP4_STBL_INFO32 __user *)compat_ptr(compatVTable);
			if(0 != CfaMp4CompatSTBLCalcSz(pucTmpVTable,&u4HeaderLen)) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaMp4CompatSTBLCalcSz.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EFAULT;
			}

			u4TotalSz += u4HeaderLen;
		}
	}

	for (i = 0; i < usr_ptr32->rCfaMp4SubInfo.u2SubStrmNum; i++)
	{
		if (0 != usr_ptr32->rCfaMp4SubInfo.prSTable[i]){
			compat_caddr_t compatVTable = 0;
			MP4_STBL_INFO32 __user *pucTmpVTable = NULL;
			if (get_user(compatVTable, &(usr_ptr32->rCfaMp4SubInfo.prSTable[i])))
				return -EFAULT;
			if (0 == compatVTable)
				return -EFAULT;

			pucTmpVTable = (MP4_STBL_INFO32 __user *)compat_ptr(compatVTable);
			if (0 != CfaMp4CompatSTBLCalcSz(pucTmpVTable,&u4HeaderLen)) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaMp4CompatSTBLCalcSz.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EFAULT;
			}

			u4TotalSz += u4HeaderLen;
		}
		if (0 != usr_ptr32->rCfaMp4SubInfo.prMFSTable[i]){
			compat_caddr_t compatVTable = 0;
			MP4_STBL_INFO32 __user *pucTmpVTable = NULL;
			if (get_user(compatVTable, &(usr_ptr32->rCfaMp4SubInfo.prMFSTable[i])))
				return -EFAULT;
			if (0 == compatVTable)
				return -EFAULT;

			pucTmpVTable = (MP4_STBL_INFO32 __user *)compat_ptr(compatVTable);
			if (0 != CfaMp4CompatSTBLCalcSz(pucTmpVTable, &u4HeaderLen)) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaMp4CompatSTBLCalcSz.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EFAULT;
			}

			u4TotalSz += u4HeaderLen;//CFA_ALIGN_SZ(u4HeaderLen, sizeof(uintptr_t));
		}
	}
	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d u4TotalSz: 0x%x.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,u4TotalSz);
	*pu4OutSz = u4TotalSz;

	return 0;
}

static long CfaMp4CompatConfig(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaMp4ConfigInfo __user *usr_ptr = NULL;
	CfaMp4ConfigInfo32 __user *usr_ptr32 = (CfaMp4ConfigInfo32 __user *)prInfo->usr_ptr32;
	long ret = 0;
	__u8 __user *pu1UsrBufAddr = NULL;
	__u8 __user *pu1NextUsrBufAddr = NULL;
	__u32 u4TotalSz = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	if (sizeof(CfaMp4ConfigInfo32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz(0x%08x/0x%08x).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prInfo->buf_sz, sizeof(CfaMp4ConfigInfo32));
		return -EINVAL;
	}

	if (0 != CfaMp4CompatConfigCalcSz(usr_ptr32, &u4TotalSz)) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaMp4CompatConfigCalcSz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	//pu1UsrBufAddr = (__u8 __user *)compat_alloc_user_space(u4TotalSz);
	//u4TotalSz is large, so alloc from kernel space
	DMX_NewMemory(u4TotalSz,pu1UsrBufAddr);

	if (NULL == pu1UsrBufAddr) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(pu1UsrBufAddr, 0, u4TotalSz);

	usr_ptr = (CfaMp4ConfigInfo __user *)pu1UsrBufAddr;

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -ENOMEM;
	}

	pu1NextUsrBufAddr = pu1UsrBufAddr + CFA_ALIGN_SZ(sizeof(CfaMp4ConfigInfo), sizeof(uintptr_t));

	ret = CfaMp4CompatVidInfo(&(usr_ptr->rCfaMp4VidInfo),&(usr_ptr32->rCfaMp4VidInfo),&pu1NextUsrBufAddr, pu1UsrBufAddr, u4TotalSz);

	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaMp4CompatVidInfo.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return ret;
	}

	ret = CfaMp4CompatAudInfo(&(usr_ptr->rCfaMp4AudInfo),&(usr_ptr32->rCfaMp4AudInfo),&pu1NextUsrBufAddr,pu1UsrBufAddr,u4TotalSz);

	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaMp4CompatAudInfo.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return ret;
	}

	ret = CfaMp4CompatSubInfo(&(usr_ptr->rCfaMp4SubInfo), &(usr_ptr32->rCfaMp4SubInfo),&pu1NextUsrBufAddr,pu1UsrBufAddr,u4TotalSz);

	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaMp4CompatSubInfo.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return ret;
	}
#if MP4_SUPPORT_FRAGMENT
	if (copy_from_user(&(usr_ptr->eMoofType), &(usr_ptr32->eMoofType), sizeof(MP4_FRAGMENT_TYPE)))
		return -EFAULT;

	if (copy_from_user(&(usr_ptr->u8MoofOffset), &(usr_ptr32->u8MoofOffset), sizeof(__u64)))
		return -EFAULT;
#endif

	*pfgIsUserMem = FALSE;

	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaMp4ConfigInfo);
	DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("%s line %d exit, u4TotalSz:(0x%08x/ 0x%08x).\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,u4TotalSz, (__u32)(pu1NextUsrBufAddr - pu1UsrBufAddr));

	return 0;
}
static long CfaMp4CompatRangeInfo(CfaMp4Range __user *usr_ptr,CfaMp4Range32 __user *usr_ptr32)
{

	#ifdef MM_ATE_CHECK
	if (copy_in_user(&(usr_ptr->u4MMATECHKStart), &(usr_ptr32->u4MMATECHKStart), sizeof(__u32)))
		return -EFAULT;
	#endif

	if (copy_in_user(&(usr_ptr->u8Length), &(usr_ptr32->u8Length), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->fgFirstRange), &(usr_ptr32->fgFirstRange), sizeof(bool)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8VidStartChunkNo), &(usr_ptr32->u8VidStartChunkNo), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8VidStartChunk1stSmp), &(usr_ptr32->u8VidStartChunk1stSmp), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8VidStartSampleNo), &(usr_ptr32->u8VidStartSampleNo), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8VidStartOffset), &(usr_ptr32->u8VidStartOffset), sizeof(__u64)))
		return -EFAULT;


	if (copy_in_user(&(usr_ptr->u8VidEndOffset), &(usr_ptr32->u8VidEndOffset), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8VidEndChunkNo), &(usr_ptr32->u8VidEndChunkNo), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8VidEndSampleNo), &(usr_ptr32->u8VidEndSampleNo), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8VidPts), &(usr_ptr32->u8VidPts), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(usr_ptr->u8AudStartChunkNo, 
			usr_ptr32->u8AudStartChunkNo, sizeof(__u64) * MAX_NS_MP4_AUD))
	{
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail .\r\n"),DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	if (copy_in_user(usr_ptr->u8AudStartChunk1stSmp, 
			usr_ptr32->u8AudStartChunk1stSmp, sizeof(__u64) * MAX_NS_MP4_AUD))
	{
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail .\r\n"),DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(usr_ptr->u8AudStartSampleNo, 
			usr_ptr32->u8AudStartSampleNo, sizeof(__u64) * MAX_NS_MP4_AUD))
	{
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail .\r\n"),DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(usr_ptr->u8AudStartOffset, 
			usr_ptr32->u8AudStartOffset, sizeof(__u64) * MAX_NS_MP4_AUD))
	{
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail .\r\n"),DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	if (copy_in_user(usr_ptr->u8AudEndOffset, 
			usr_ptr32->u8AudEndOffset, sizeof(__u64) * MAX_NS_MP4_AUD))
	{
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail .\r\n"),DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	if (copy_in_user(usr_ptr->u8AudEndChunkNo, 
			usr_ptr32->u8AudEndChunkNo, sizeof(__u64) * MAX_NS_MP4_AUD))
	{
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail .\r\n"),DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	if (copy_in_user(usr_ptr->u8AudEndSampleNo, 
			usr_ptr32->u8AudEndSampleNo, sizeof(__u64) * MAX_NS_MP4_AUD))
	{
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail .\r\n"),DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	if (copy_in_user(usr_ptr->u8AudPts, 
			usr_ptr32->u8AudPts, sizeof(__u64) * MAX_NS_MP4_AUD))
	{
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail .\r\n"),DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	if (copy_in_user(usr_ptr->u8SubStartChunkNo, 
			usr_ptr32->u8SubStartChunkNo, sizeof(__u64) * MAX_NS_MP4_SUB))
	{
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail .\r\n"),DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	if (copy_in_user(usr_ptr->u8SubStartChunk1stSmp, 
			usr_ptr32->u8SubStartChunk1stSmp, sizeof(__u64) * MAX_NS_MP4_SUB))
	{
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail .\r\n"),DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	if (copy_in_user(usr_ptr->u8SubStartSampleNo, 
			usr_ptr32->u8SubStartSampleNo, sizeof(__u64) * MAX_NS_MP4_SUB))
	{
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail .\r\n"),DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	if (copy_in_user(usr_ptr->u8SubStartOffset, 
			usr_ptr32->u8SubStartOffset, sizeof(__u64) * MAX_NS_MP4_SUB))
	{
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail .\r\n"),DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	if (copy_in_user(usr_ptr->u8SubEndOffset, 
			usr_ptr32->u8SubEndOffset, sizeof(__u64) * MAX_NS_MP4_SUB))
	{
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail .\r\n"),DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	if (copy_in_user(usr_ptr->u8SubEndChunkNo, 
			usr_ptr32->u8SubEndChunkNo, sizeof(__u64) * MAX_NS_MP4_SUB))
	{
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail .\r\n"),DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	if (copy_in_user(usr_ptr->u8SubEndSampleNo, 
			usr_ptr32->u8SubEndSampleNo, sizeof(__u64) * MAX_NS_MP4_SUB))
	{
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail .\r\n"),DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	if (copy_in_user(usr_ptr->u8SubPts, 
			usr_ptr32->u8SubPts, sizeof(__u64) * MAX_NS_MP4_SUB))
	{
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail .\r\n"),DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	if (copy_in_user(&(usr_ptr->u4PrsFlag), &(usr_ptr32->u4PrsFlag), sizeof(__u32)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8StartPts), &(usr_ptr32->u8StartPts), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8SeekPts), &(usr_ptr32->u8SeekPts), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8MoovSkipSize), &(usr_ptr32->u8MoovSkipSize), sizeof(__u64)))
		return -EFAULT;

	#ifdef MM_ATE_CHECK
	if (copy_in_user(&(usr_ptr->u4MMATECHKEnd), &(usr_ptr32->u4MMATECHKEnd), sizeof(__u32)))
		return -EFAULT;
	#endif

	return 0;
}

static long CfaMp4CompatRange(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaMp4Range __user *usr_ptr = NULL;
	CfaMp4Range32 __user *usr_ptr32 = (CfaMp4Range32 __user *)prInfo->usr_ptr32;
	long ret = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaMp4Range32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	usr_ptr = (CfaMp4Range *)compat_alloc_user_space(sizeof(CfaMp4Range));

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaMp4Range));

	ret = CfaMp4CompatRangeInfo(usr_ptr,usr_ptr32);

	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaMp4CompatRangeInfo.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}

	*pfgIsUserMem = TRUE;
	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaMp4Range);

	return 0;
}

static long CfaMp4CompatJumpRange(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaMP4KeyFrameRange __user *usr_ptr = NULL;
	CfaMP4KeyFrameRange32 __user *usr_ptr32 = (CfaMP4KeyFrameRange32 __user *)prInfo->usr_ptr32;
	long ret = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaMP4KeyFrameRange32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	usr_ptr = (CfaMP4KeyFrameRange *)compat_alloc_user_space(sizeof(CfaMP4KeyFrameRange));

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaMP4KeyFrameRange));

	ret = CfaMp4CompatRangeInfo(&(usr_ptr->rCfaRangeInfo), &(usr_ptr32->rCfaRangeInfo));

	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaMp4CompatRangeInfo.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}
	DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4]%s line %d ok.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	*pfgIsUserMem = TRUE;
	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaMP4KeyFrameRange);

	return 0;
}

static int CfaMp4ProcCompat(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	long ret = 0;

	if ((NULL == prInfo) || (NULL == pfgIsUserMem)) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail for invalid parameter.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	}
	switch (prInfo->type) {
		case CFA_CONFIG:
			if (prInfo->is_get) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EPERM;
			}
			ret = CfaMp4CompatConfig(prInfo, pfgIsUserMem);
			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaMp4CompatConfig.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return ret;
			}
			break;
		case CFA_RANGE:
			if (prInfo->is_get) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EPERM;
			}
			ret = CfaMp4CompatRange(prInfo, pfgIsUserMem);
			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaMp4CompatRange.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return ret;
			}
			break;
		case CFA_GEN_INFO:
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("%s line %d fail for don't support get info for cfa Mp4.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return -EPERM;
		case CFA_JUMP_INFO:
			if (prInfo->is_get) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EPERM;
			}
			ret = CfaMp4CompatJumpRange(prInfo, pfgIsUserMem);
			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaMp4CompatJumpRange.\r\n"),
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

/* MP4 CFA interface */
CfaIntf _rMp4CfaIntf = {
	&CfaMp4Init, 
	&CfaMp4Uninit, 
	&CfaMp4SetRange, 
	&CfaMp4EnableStrm, 
	&CfaMp4SetStrmInf,
	&CfaMp4TurnOn, 
	&CfaMp4TxDone, 
	&CfaMp4GetCurPos, 
	&CfaMp4FillPicInfo,
	&CfaMp4Configure, 
	NULL, 
	&CfaMp4GetGeneral, 
	&CfaMp4SetGeneral, 
	NULL,
	&CfaMp4FillAUInfo, 
	&CfaMp4TxAudHDRInfo, 
	NULL, 
	&CfaMP4SetJumpRange,
	&CfaMP4GetParamSize, 
	&CfaMp4ProcCliCmd
#ifdef CONFIG_COMPAT
	,&CfaMp4ProcCompat
#endif
};

/*-----------------------------------------------------------------------------
* Name: pvCfaMp4GetInterface
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
void *CfaMp4GetInterface(void)
{
	return ((void *) &_rMp4CfaIntf);
}
