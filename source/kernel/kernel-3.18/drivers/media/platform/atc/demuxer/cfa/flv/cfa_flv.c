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
#include "windows.h"
#include <media/atc/dmx_define.h>
/* #include <media/atc/mm_debug.h> */
#include <media/atc/drv_esm_if.h>

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_spt.h"
#include "dmx_log.h"
#include "cfa_macro.h"
#include "cfa_flv.h"
#include "cfa_flv_st_ctrl.h"
#include "mmisc.h"

static void CfaFlvResetTimeMan(CfaFlvInst_T * prCfaFlv)
{
	FLV_TIMESTAMP_MAN_T *prVTimeMan = NULL;
	FLV_TIMESTAMP_MAN_T *prATimeMan = NULL;

	if (NULL == prCfaFlv) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] failed for invalid args\r\n"));
		return;
	}

	prVTimeMan = &(prCfaFlv->rVTimeStampMan);

	prVTimeMan->eState = CFA_FLV_TIMESTAMP_OK;

	prVTimeMan->fgNeedCheckTimeInc = FALSE;
	prVTimeMan->fgNeedCheckTimeDec = FALSE;
	prVTimeMan->fgNeedGetDeltaTime = TRUE;

	prVTimeMan->u4TimeIncCnt = 0;	/* pts increase count*/
	prVTimeMan->u4TimeDecCnt = 0;	/* pts increase count*/

	prVTimeMan->u4DeltaTime = DMX_INVALID_UINT32;
	prVTimeMan->u4TimeIncrement = 0;

	prVTimeMan->u4LastTime = 0;
	prVTimeMan->u4CurTime = 0;

	prVTimeMan->u8TotalTagCnt = 0;

	prVTimeMan->u4OldNormnalTime = 0;
	prVTimeMan->u4FirstAbnormalTime = 0;

	dmx_memset(prVTimeMan->au4TimeRecord, 0xFF, sizeof(u32) * CFA_FLV_MAX_PTS_RECORD_LEN);
	dmx_memset(prVTimeMan->au4Rec4CorrectTime, 0xFF,
		   sizeof(u32) * CFA_FLV_MAX_PTS_RECORD_LEN);

	prVTimeMan->fgFirstGetTime = TRUE;
	prVTimeMan->u4BaseTime = DMX_INVALID_UINT32;
	prVTimeMan->u4TimeStamp = DMX_INVALID_UINT32;
	prVTimeMan->u4SeekTime = DMX_INVALID_UINT32;

	prATimeMan = &(prCfaFlv->rATimeStampMan);

	prATimeMan->eState = CFA_FLV_TIMESTAMP_OK;

	prATimeMan->fgNeedCheckTimeInc = FALSE;
	prATimeMan->fgNeedCheckTimeDec = FALSE;
	prATimeMan->fgNeedGetDeltaTime = TRUE;

	prATimeMan->u4TimeIncCnt = 0;	/* pts increase count*/
	prATimeMan->u4TimeDecCnt = 0;	/* pts increase count*/

	prATimeMan->u4DeltaTime = DMX_INVALID_UINT32;
	prATimeMan->u4TimeIncrement = 0;

	prATimeMan->u4LastTime = 0;
	prATimeMan->u4CurTime = 0;

	prATimeMan->u8TotalTagCnt = 0;

	prATimeMan->u4OldNormnalTime = 0;
	prATimeMan->u4FirstAbnormalTime = 0;

	dmx_memset(prATimeMan->au4TimeRecord, 0xFF, sizeof(u32) * CFA_FLV_MAX_PTS_RECORD_LEN);
	dmx_memset(prATimeMan->au4Rec4CorrectTime, 0xFF,
		   sizeof(u32) * CFA_FLV_MAX_PTS_RECORD_LEN);

	prATimeMan->fgFirstGetTime = TRUE;
	prATimeMan->u4BaseTime = DMX_INVALID_UINT32;
	prATimeMan->u4TimeStamp = DMX_INVALID_UINT32;
}

/*-----------------------------------------------------------------------------
macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
function prototype
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
* Name: CfaFlvInitPara
*
* Description:
*      Init CFA FLV internal parameters
*
* Inputs:
*
* Outputs:
*
* Returns: None
*
*-----------------------------------------------------------------------------*/
static void CfaFlvInitPara(CfaFlvInst_T *prCfaFlv)
{
	if (NULL == prCfaFlv) {
		return;
	}

	prCfaFlv->eNextAnaSt = CFA_FLV_ANA_ST_IDLE;
	prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_IDLE;
	prCfaFlv->u8PrsPts = 0;
	prCfaFlv->pu1HdrBuf = NULL;
	prCfaFlv->ptrPfrMemAddress = DMX_INVALID_UINTPTR_T;

	prCfaFlv->fgNoNeedSyncPb = FALSE;
	prCfaFlv->fgRealSyncPb = FALSE;
	prCfaFlv->u4AvailDataSz = 0;
	prCfaFlv->u8PreCa = prCfaFlv->u8Ca;

	prCfaFlv->rCurPosInfo.u8AudCurOfst = 0;
	prCfaFlv->rCurPosInfo.u8IFrmCurOfst = 0;
	prCfaFlv->rCurPosInfo.u8PrsCurOfst = 0;
	prCfaFlv->rCurPosInfo.u8VidCurOfst = 0;
	prCfaFlv->u4PacketLen = prCfaFlv->rFileInfo.u4DataPacketSize;
	prCfaFlv->u8PreVPts = 0;
	prCfaFlv->ePrePicType = CFA_PIC_UNDEFINE;

	prCfaFlv->u8IFrameCnt = 0;

	prCfaFlv->fgNeedTxVidSeq = FALSE;

	prCfaFlv->u4TxLen = 0;
	prCfaFlv->eCurTxStrmType = CFA_FLV_TX_STRM_TYPE_NONE;
	prCfaFlv->u4CurTagNum = 0;
	prCfaFlv->u4TagSize = 0;
	prCfaFlv->fgTagIsValid = FALSE;
	prCfaFlv->u4PacketLen = 0;
	prCfaFlv->fgKeyFrame = FALSE;

	prCfaFlv->u4FirstUintPay = 0;
	prCfaFlv->u1FirstBytePay = 0;

	prCfaFlv->u4VideoPartTagOffset = 0;

	prCfaFlv->u4PureAudCurUnitTxSz = 0;

	prCfaFlv->u4PureAudSkipTagCnt = 0;

	prCfaFlv->fgFirstVidAU = TRUE;
	prCfaFlv->fgDisplayOrder = FALSE;

	dmx_memset(&(prCfaFlv->rAudioInfo.rATagInfo), 0, sizeof(prCfaFlv->rAudioInfo.rATagInfo));

	prCfaFlv->rVideoInfo.eFrmType = FLV_FRM_TYPE_NONE;
	dmx_memset(&(prCfaFlv->rVideoInfo.rVTagInfo), 0, sizeof(prCfaFlv->rVideoInfo.rVTagInfo));

	dmx_memset(&(prCfaFlv->rTagInfo), 0, sizeof(CfaFlvTagInfo_T));

	CfaFlvResetTimeMan(prCfaFlv);
}

/*-----------------------------------------------------------------------------
* Name: CfaFlvInit
*
* Description:
*      Init CFA FLV
*
* Inputs:
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/

static MRESULT CfaFlvInit(void *pvSptHdl, void **ppvCfaPrivData)
{
	CfaFlvInst_T *prCfaFlv = NULL;

	DMX_NewMemory(sizeof(CfaFlvInst_T), prCfaFlv);
	if (NULL == prCfaFlv) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] failed in Alloc prCfaFlv memory\r\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}

	dmx_memset(prCfaFlv, 0, sizeof(CfaFlvInst_T));

#ifdef MM_ATE_CHECK
	MMATE_INIT_POINTER(prCfaFlv);
	MMATE_INIT_STRUCT(prCfaFlv->rRange);
	MMATE_INIT_STRUCT(prCfaFlv->rDecoderCfgInfo);
#endif

	DMX_NewHwMemory(8, prCfaFlv->rAudioInfo.pauAudHeader);
	if (NULL == prCfaFlv->rAudioInfo.pauAudHeader) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] failed in Alloc prCfaFlv->rAudioInfo.pauAudHeader memory\r\n"));
		DMX_FreeMemory(prCfaFlv);
		MM_RETURN(RET_DMX_NO_MEM);
	}

	DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		TEXT("[CFA_FLV] %s -- AdtsHeader=0x%x\r\n"),
		DMX_FUNC_NAME, prCfaFlv->rAudioInfo.pauAudHeader);

	DMX_NewMemory(sizeof(CfaFlvAudCmdQInfo_T), prCfaFlv->prAudCmdQsInfo);
	if (NULL == prCfaFlv->prAudCmdQsInfo) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] failed in Alloc prCfaFlv->prAudCmdQsInfo memory\r\n"));
		DMX_FreeHwMemory(prCfaFlv->rAudioInfo.pauAudHeader);
		DMX_FreeMemory(prCfaFlv);
		MM_RETURN(RET_DMX_NO_MEM);
	}
	dmx_memset(prCfaFlv->prAudCmdQsInfo, 0, sizeof(CfaFlvAudCmdQInfo_T));

	DMX_NewMemory(sizeof(CfaFlvVidCmdQInfo_T), prCfaFlv->prVidCmdQsInfo);
	if (NULL == prCfaFlv->prVidCmdQsInfo) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] failed in Alloc prCfaFlv->prVidCmdQsInfo memory\r\n"));
		DMX_FreeHwMemory(prCfaFlv->rAudioInfo.pauAudHeader);
		DMX_FreeMemory(prCfaFlv->prAudCmdQsInfo);
		DMX_FreeMemory(prCfaFlv);
		MM_RETURN(RET_DMX_NO_MEM);
	}
	dmx_memset(prCfaFlv->prVidCmdQsInfo, 0, sizeof(CfaFlvVidCmdQInfo_T));

	DMX_NewMemory(sizeof(DMX_CMDQ_TX_ENTRY_T) * DMX_MAX_TX_CNT_FOR_CMD_Q,
		prCfaFlv->prCmdEntrys);
	if (NULL == prCfaFlv->prCmdEntrys) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] failed in Alloc prCfaFlv->prCmdEntrys memory\r\n"));
		DMX_FreeHwMemory(prCfaFlv->rAudioInfo.pauAudHeader);
		DMX_FreeMemory(prCfaFlv->prAudCmdQsInfo);
		DMX_FreeMemory(prCfaFlv->prVidCmdQsInfo);
		DMX_FreeMemory(prCfaFlv);
		MM_RETURN(RET_DMX_NO_MEM);
	}
	dmx_memset(prCfaFlv->prCmdEntrys, 0,
		sizeof(DMX_CMDQ_TX_ENTRY_T) * DMX_MAX_TX_CNT_FOR_CMD_Q);

	prCfaFlv->fgJumpTurnOn = FALSE;
	prCfaFlv->u4PureAudTxUnitSz = 0;
	prCfaFlv->u4PureAudCurUnitTxSz = 0;

	prCfaFlv->u4PureAudSkipTagCnt = 0;

#if CONFIG_CFA_FLV_FOR_ERR_JUMP
	prCfaFlv->u4OffsetJumpCnt = 0;
#endif				/*CONFIG_CFA_FLV_FOR_ERR_JUMP*/

	prCfaFlv->ptrPfrMemAddress = DMX_INVALID_UINTPTR_T;
	prCfaFlv->pu1HdrBuf = NULL;
	prCfaFlv->puDecoderCfgBuf = NULL;
	prCfaFlv->rAacCfgInfo.puHeader = NULL;
	prCfaFlv->u8Ca = DMX_INVALID_UINT64;
	prCfaFlv->u4CurPrsFlag = 0;
	prCfaFlv->fgNeedTxVidSeq = FALSE;
	prCfaFlv->u4FirstUintPay = 0;

	/* set initial information*/
	CfaFlvInitPara(prCfaFlv);

	*ppvCfaPrivData = (void *) prCfaFlv;

	MMATE_CHECK_POINTER(prCfaFlv);
	MMATE_CHECK_STRUCT(prCfaFlv->rRange);
	MMATE_CHECK_STRUCT(prCfaFlv->rDecoderCfgInfo);

	DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		TEXT("[CFA_FLV] %s success\r\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: CfaFlvUninit
*
* Description:
*      Uninit CFA FLV
*
* Inputs:
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaFlvUninit(void *pvSptHdl, void *pvCfaPrivData)
{
	CfaFlvInst_T *prCfaFlv = (CfaFlvInst_T *) pvCfaPrivData;

	if (NULL == prCfaFlv) {
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] %s success, exit, prCfaFlv is null!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	MMATE_CHECK_POINTER(prCfaFlv);
	MMATE_CHECK_STRUCT(prCfaFlv->rRange);
	MMATE_CHECK_STRUCT(prCfaFlv->rDecoderCfgInfo);

	if (prCfaFlv->u8IFrameCnt > 0) {
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] %s -- IFrameCnt: %lld, FileSize: %lld, SzPerIFrame: %lld\r\n"),
			DMX_FUNC_NAME, prCfaFlv->u8IFrameCnt, prCfaFlv->rRange.u8FileSz,
			((prCfaFlv->rRange.u8FileSz) / (prCfaFlv->u8IFrameCnt)));
	} else {
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] %s -- IFrameCnt: %lld, FileSize: %lld, SzPerIFrame: NO_I_FRAME\r\n"),
			DMX_FUNC_NAME, prCfaFlv->u8IFrameCnt, prCfaFlv->rRange.u8FileSz);
	}

	CfaFlvReleaseCfgBuf(prCfaFlv);

	if (prCfaFlv->rAacCfgInfo.puHeader != NULL) {
		DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] %s -- free AAC.puHeader=0x%x\r\n"),
			DMX_FUNC_NAME, prCfaFlv->rAacCfgInfo.puHeader);
		DMX_FreeHwMemory(prCfaFlv->rAacCfgInfo.puHeader);
		prCfaFlv->rAacCfgInfo.puHeader = NULL;
	}

	if (NULL != prCfaFlv->puDecoderCfgBuf) {
		DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] %s -- free puDecoderCfgBuf=0x%x\r\n"),
			DMX_FUNC_NAME, prCfaFlv->puDecoderCfgBuf);
		DMX_FreeHwMemory(prCfaFlv->puDecoderCfgBuf);
		prCfaFlv->puDecoderCfgBuf = NULL;
	}

	if (NULL != prCfaFlv->rAudioInfo.pauAudHeader) {
		DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] %s -- free AdtsHeader=0x%x\r\n"),
			DMX_FUNC_NAME, prCfaFlv->rAudioInfo.pauAudHeader);
		DMX_FreeHwMemory(prCfaFlv->rAudioInfo.pauAudHeader);
		prCfaFlv->rAudioInfo.pauAudHeader = NULL;
	}

	if (NULL != prCfaFlv->prAudCmdQsInfo) {
		DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] %s -- free prAudCmdQsInfo=0x%x\r\n"),
			DMX_FUNC_NAME, prCfaFlv->prAudCmdQsInfo);
		DMX_FreeMemory(prCfaFlv->prAudCmdQsInfo);
		prCfaFlv->prAudCmdQsInfo = NULL;
	}

	if (NULL != prCfaFlv->prVidCmdQsInfo) {
		DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] %s -- free prVidCmdQsInfo=0x%x\r\n"),
			DMX_FUNC_NAME, prCfaFlv->prVidCmdQsInfo);
		DMX_FreeMemory(prCfaFlv->prVidCmdQsInfo);
		prCfaFlv->prVidCmdQsInfo = NULL;
	}

	if (NULL != prCfaFlv->prCmdEntrys) {
		DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] %s -- free prCmdEntrys=0x%x\r\n"),
			DMX_FUNC_NAME, prCfaFlv->prCmdEntrys);
		DMX_FreeMemory(prCfaFlv->prCmdEntrys);
		prCfaFlv->prCmdEntrys = NULL;
	}

	prCfaFlv->pu1HdrBuf = NULL;
	if (NULL != prCfaFlv)
		DMX_FreeMemory(prCfaFlv);

	DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		TEXT("[CFA_FLV] %s success\r\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

/*-----------------------------------------------------------------------------
* Name: CfaFlvSetRange
*
* Description:
*      FLV CFA sets demuxing range
*      splitter will ensure that pfvSetRange is only called in "off" state.
*      If used with MPC, the range of MPC_SCMD_SPR will be passed here
*
* Inputs:
*      [IN] handle of splitter
*      [IN] pointer to CfaFlvRange
*      [IN] pointer to CfaFlvInst
*
* Outputs:
*
* Returns: s32
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaFlvSetRange(void *pvSptHdl, void *pvRange, void *pvPrivData, bool fgIsUserMem)
{
	CfaFlvInst_T *prCfaFlv = NULL;

	if ((NULL == pvPrivData) || (NULL == pvRange)) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] fail for invalid args\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prCfaFlv = (CfaFlvInst_T *) pvPrivData;
#ifdef MM_ATE_CHECK
	if (0 != mm_copy_from_user(&(prCfaFlv->rRange.fgHasVid), &(((CfaFlvRange_T *) pvRange)->fgHasVid),
		   sizeof(CfaFlvRange_T) - 2 * sizeof(u32))) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] %s line %d failed in mm_copy_from_user\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
  }
#else
	if (0 != mm_copy_from_user(&(prCfaFlv->rRange), pvRange, sizeof(CfaFlvRange_T))) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] %s line %d failed in mm_copy_from_user\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
  }
#endif

	MMATE_CHECK_POINTER(prCfaFlv);
	MMATE_CHECK_STRUCT(prCfaFlv->rRange);
	MMATE_CHECK_STRUCT(prCfaFlv->rDecoderCfgInfo);

	DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		TEXT("[CFA_FLV] %s line %d -- fgHasVid: %d, fgHasAud: %d\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		(prCfaFlv->rRange.fgHasVid ? 1 : 0), (prCfaFlv->rRange.fgHasAud ? 1 : 0));

	prCfaFlv->rCurPosInfo.u8VidCurOfst = prCfaFlv->rRange.u8VidSa;
	prCfaFlv->rCurPosInfo.u8AudCurOfst = prCfaFlv->rRange.u8AudSa;
	if ((prCfaFlv->rRange.u8AudSa == 0) && (prCfaFlv->rRange.u8VidSa == 0))
		prCfaFlv->u8Ca = prCfaFlv->rFileInfo.u8HeaderSize;
	else
		prCfaFlv->u8Ca = MIN(prCfaFlv->rRange.u8AudSa, prCfaFlv->rRange.u8VidSa);

	prCfaFlv->u8Endoffst = prCfaFlv->rRange.u8VidEa;

	prCfaFlv->u8FileSz = prCfaFlv->rRange.u8FileSz;
	prCfaFlv->fgFirstTxAud = TRUE;
	prCfaFlv->fgFirstTxVid = TRUE;

	prCfaFlv->fgJumpTurnOn = TRUE;

	prCfaFlv->rRange.u8IframeNum = 0;

#if CONFIG_CFA_FLV_FOR_ERR_JUMP
	prCfaFlv->u4OffsetJumpCnt = 0;
#endif

	DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		TEXT("[CFA_FLV] %s success -- Ca: %lld --->VidEa: %lld, SeekTime: %d ms\r\n"),
		DMX_FUNC_NAME, prCfaFlv->u8Ca, prCfaFlv->rRange.u8VidEa,
		prCfaFlv->rRange.u8SeekTime);

	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: CfaFlvEnableStrm
*
* Description:
*      FLV CFA sets stream to parse, may be combinations of V/A/S.
*      splitter will ensure that pfvSetStrm() is only called in "off" or "paused" state.
*
* Inputs:
*      [IN] handle of splitter
*      [IN] streams to parse or to cancel parsing
*      [IN] CFA_STREAM_ON:  The bits turned ON in u4StrmToPrs are the streams that FMPC
*            would like to parse.  CFA_STRM_OFF: The bits turned ON in u4StrmToPrs are the
*            streams that FMPC would like to stop parsing
*      [IN] pointer to CfaFlvInst)
*
* Outputs:
*
* Returns: s32
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaFlvEnableStrm(void *pvSptHdl, u32 u4StrmToPrs, CfaStreamOp eOp, void *pvPrivData)
{
	CfaFlvInst_T *prCfaFlv = NULL;

	if (NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] fail for invalid args, u4StrmToPrs(%d), eOp(%d)\r\n"),
			u4StrmToPrs, eOp);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prCfaFlv = (CfaFlvInst_T *) pvPrivData;

	MMATE_CHECK_POINTER(prCfaFlv);
	MMATE_CHECK_STRUCT(prCfaFlv->rRange);
	MMATE_CHECK_STRUCT(prCfaFlv->rDecoderCfgInfo);

	if (CFA_STREAM_ON == eOp) {
		/*enable*/
		if (CFA_STRM_V & u4StrmToPrs)
			prCfaFlv->u4CurPrsFlag |= CFA_FLV_PRS_BIT_STRM_TYPE_VID;

		if (CFA_STRM_A & u4StrmToPrs) {
			prCfaFlv->u4CurPrsFlag |= CFA_FLV_PRS_BIT_STRM_TYPE_AUD;
			prCfaFlv->fgFirstTxAud = TRUE;
		}
	} else {
		/*disable*/
		if (CFA_STRM_V & u4StrmToPrs)
			prCfaFlv->u4CurPrsFlag &= ~((u32) CFA_FLV_PRS_BIT_STRM_TYPE_VID);

		if (CFA_STRM_A & u4StrmToPrs)
			prCfaFlv->u4CurPrsFlag &= ~((u32) CFA_FLV_PRS_BIT_STRM_TYPE_AUD);
	}

	DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		TEXT("[CFA_FLV] %s success -- u4StrmToPrs(%d), eOp(%d), CurPrsFlag: 0x%x\r\n"),
		DMX_FUNC_NAME, u4StrmToPrs, eOp, prCfaFlv->u4CurPrsFlag);

	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: i4CfaFlvSetStrmInf
*
* Description:
*      Set Stream information
*
* Inputs:
*      [IN] handle of splitter
*      [IN] stream to set
*      [IN] stream info
*      [IN] pointer to CfaFlvInst
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaFlvSetStrmInf(void *pvSptHdl, u32 u4StrmType, u32 u4Info, void *pvPrivData)
{
	CfaFlvInst_T *prCfaFlv = NULL;

	if (NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] fail for invalid args, u4StrmType(%d)\r\n"),
			u4StrmType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prCfaFlv = (CfaFlvInst_T *) pvPrivData;

	MMATE_CHECK_POINTER(prCfaFlv);
	MMATE_CHECK_STRUCT(prCfaFlv->rRange);
	MMATE_CHECK_STRUCT(prCfaFlv->rDecoderCfgInfo);

	if (CFA_STRM_V == u4StrmType)
		prCfaFlv->rVideoInfo.rCfgInfo.u1StrmNum = (u8) ((u32)0x000000FF & u4Info);
	else if (CFA_STRM_A == u4StrmType)
		prCfaFlv->rAudioInfo.rCfgInfo.u1StrmNum = (u8) ((u32)0x000000FF & u4Info);
	else {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] fail for unsupported stream type(%d), only support A and V\r\n"),
			u4StrmType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		TEXT("[CFA_FLV] %s success\r\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: i4CfaFlvTurnOn
*
* Description:
*      FLV CFA turns on file demuxing
*      A transfer should be issued in this function.
*
* Inputs:
*
* Outputs:
*
* Returns: s32
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaFlvTurnOn(void *pvSptHdl, void *pvPrivData)
{
	CfaFlvInst_T *prCfaFlv = NULL;
	u64 u8Sa = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		TEXT("[CFA_FLV][CFA_VERSION:2016-0705-1502] %s line %d enter!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	if (NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] fail for invalid args\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prCfaFlv = (CfaFlvInst_T *) pvPrivData;

	MMATE_CHECK_POINTER(prCfaFlv);
	MMATE_CHECK_STRUCT(prCfaFlv->rRange);
	MMATE_CHECK_STRUCT(prCfaFlv->rDecoderCfgInfo);

	CfaFlvInitPara(prCfaFlv);

	if (fgIsCfaStmToPlay(prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_AUD) &&
	    (!fgIsCfaStmToPlay(prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_VID))) {
		prCfaFlv->u8Ca = prCfaFlv->rRange.u8AudSa;
		DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] Range has only audio info!!\r\n"));
	} else if (fgIsCfaStmToPlay(prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_VID) &&
		   (!fgIsCfaStmToPlay(prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_AUD))) {
		prCfaFlv->u8Ca = prCfaFlv->rRange.u8VidSa;
		DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] Range has only video info!!\r\n"));
	} else if (fgIsCfaStmToPlay(prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_VID) &&
		   fgIsCfaStmToPlay(prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_AUD)) {
		prCfaFlv->u8Ca = ((prCfaFlv->rRange.u8VidSa) > (prCfaFlv->rRange.u8AudSa))
		    ? (prCfaFlv->rRange.u8AudSa) : (prCfaFlv->rRange.u8VidSa);
		if ((prCfaFlv->rRange.u8VidSa) > (prCfaFlv->rRange.u8AudSa)) {
			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("[CFA_FLV] Range has all video and audio,")
				TEXT(" offset of audio before video, AudSa: 0x%x!\r\n"),
				prCfaFlv->rRange.u8AudSa);
		} else {
			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("[CFA_FLV] Range has all video and audio,")
				TEXT(" offset of video before audio, VidSa: 0x%x!\r\n"),
				prCfaFlv->rRange.u8VidSa);
		}
	} else {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] fail for Both Video And Audio are disable in SetRange!\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (DMX_INVALID_UINT64 == prCfaFlv->u8Ca) {
		u8Sa = CfaFlvGetTxSa(prCfaFlv);
		if (DMX_INVALID_UINT64 == u8Sa) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("[CFA_FLV] fail for FlVInst's u8Ca == -1\r\n"));
			MM_RETURN(RET_DMX_ERR_STATE);
		}
	}

	prCfaFlv->u8PreCa = prCfaFlv->u8Ca;
	if (NULL != prCfaFlv->prAudCmdQsInfo)
		dmx_memset(prCfaFlv->prAudCmdQsInfo, 0, sizeof(CfaFlvAudCmdQInfo_T));

	if (NULL != prCfaFlv->prVidCmdQsInfo)
		dmx_memset(prCfaFlv->prVidCmdQsInfo, 0, sizeof(CfaFlvVidCmdQInfo_T));
	prCfaFlv->u4PureAudCurUnitTxSz = 0;
	if (DMX_IS_RW_PLAY(pvSptHdl) && (!prCfaFlv->rRange.fgHasVid) && (!prCfaFlv->fgJumpTurnOn)) {
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] %s line %d -- Spt4CfaFinishedEx, Pure Audio FF or RW\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		mrRet = Spt4CfaFinishedEx(pvSptHdl, prCfaFlv->u8Ca, FALSE, (u32)GAU_E_EOS);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("[CFA_FLV] fail in Spt4CfaFinishedEx, mrRet: 0x%x.\r\n"),
				mrRet);
			MM_RETURN(RET_DMX_ERR_STATE);
		}
		MM_RETURN(mrRet);
	}
	DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		TEXT("[CFA_FLV] %s line %d -- u8Ca: %lld\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prCfaFlv->u8Ca);

	if ((AVCODEC_ID_H265 == prCfaFlv->rVideoInfo.rCfgInfo.eCodecID) &&
		(VCODEC_VERSION_NONE == prCfaFlv->rVideoInfo.rCfgInfo.rVersion) &&
		(NULL != prCfaFlv->rVideoInfo.rCfgInfo.puSeqHdr)) {
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] %s enter CFA_FLV_ANA_ST_TX_PARTTAG_H265_HDR\r\n"), DMX_FUNC_NAME);
		CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_TX_PARTTAG_H265_HDR, (u64)0,
			(u64)FLV_READ_FOR_CODEC_SIZE);
	} else{
		CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER, (u64)0,
			(u64)FLV_READ_FOR_CODEC_SIZE);
	}

	DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		TEXT("[CFA_FLV] %s success\r\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

/* FLV CFA callback for transfer done*/
/* @return None*/
/* @note This function will be called after a transfer is complete.*/
/*< [IN] handle of splitter */
/*< [IN] Actual transferred data length.  Normally this value should be equal to the u4Len
* in the previous transfer issue, unless file end is hit. */
/*< [IN] pointer to CfaFlvInst */
static MRESULT CfaFlvTxDone(void *pvSptHdl, u64 u8TxLen, void *pvPrivData, bool fgRsp)
{
	CfaFlvInst_T *prCfaFlv = NULL;
	MRESULT mrRet = RET_DMX_OK;

	/* Because this function is called so frequently, and the caller should*/
	/* ensure all the params are valid, so we only add assert here.*/
    if(NULL == pvPrivData)
    {
        DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] pvPrivData is NULL! \r\n"));
    	MM_RETURN(RET_DMX_PARAM_WRONG);
    }


	prCfaFlv = (CfaFlvInst_T *) pvPrivData;

	MMATE_CHECK_POINTER(prCfaFlv);
	MMATE_CHECK_STRUCT(prCfaFlv->rRange);
	MMATE_CHECK_STRUCT(prCfaFlv->rDecoderCfgInfo);

	if (fgRsp) {
		prCfaFlv->fgRealSyncPb = TRUE;
		prCfaFlv->fgNoNeedSyncPb = FALSE;
		prCfaFlv->u4AvailDataSz = 0;

		mrRet = Spt4CfaPbb2SyncBufEx(pvSptHdl, prCfaFlv->u8Ca, u8TxLen,
						(u8 *) &(prCfaFlv->ptrPfrMemAddress),
						&(prCfaFlv->u4AvailDataSz));

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("[CFA_FLV] fail in Spt4CfaPbb2SyncBuf, mrRet: 0x%x.\r\n"),
				mrRet);
		}

		MM_RETURN(mrRet);
	}
	/*need rewrite*/
	CfaFlvTxDoneStCtrl(pvSptHdl, u8TxLen, prCfaFlv);

	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: i4CfaFlvGetCurPos
*
* Description:
*      FLV CFA callback for when FMPC needs to know CFA's current position.
*
* Inputs:
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaFlvGetCurPos(void *pvSptHdl, void *pvCurPos, void *pvPrivData)
{
	CfaFlvInst_T *prCfaFlv = NULL;
	u64 *ppu8 = pvCurPos;

	if ((NULL == pvPrivData) || (NULL == pvCurPos)) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] fail for invalid args\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prCfaFlv = (CfaFlvInst_T *) pvPrivData;

	MMATE_CHECK_POINTER(prCfaFlv);
	MMATE_CHECK_STRUCT(prCfaFlv->rRange);
	MMATE_CHECK_STRUCT(prCfaFlv->rDecoderCfgInfo);

	DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		TEXT("[CFA_FLV] %s -- u8Ca: 0x%x\r\n"),
		DMX_FUNC_NAME, (u32) prCfaFlv->u8Ca);

	*ppu8 = prCfaFlv->u8Ca;

	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: fgCfaFlvFillPicInfo
*
* Description:
*      FLV CFA callback for each picture is demuxed
*      original related function: vFlvM4vPIsr
*
* Inputs:
*
* Outputs:
*
* Returns: TRUE - this picture should be retained in video FIFO.
*          FALSE - this picture should be removed from video FIFO.
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaFlvFillPicInfo(void *pvSptHdl, Spt2CfaPicInfo *ptPicInfo, void *pvPrivData)
{
	CfaFlvInst_T *prCfaFlv = NULL;

    if((NULL == ptPicInfo) || (NULL == pvPrivData))
    {
    	DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] fail for invalid args\r\n"));
        MM_RETURN(RET_DMX_PARAM_WRONG);
    }

	prCfaFlv = (CfaFlvInst_T *) pvPrivData;

	/*ptPicInfo->u8ThisPts = prCfaFlv->u8PrsPts;*/
	/*prCfaFlv->u8PrsPts = DMX_INVALID_UINT64;*/

	MM_RETURN(RET_DMX_OK);
}

/*-----------------------------------------------------------------------------
* Name: i4CfaFlvSetVidType
*
* Description:
*      CFA FLV sets video type for transfering video data to Video FIFO with the codec info
*      by playback module setting.
* Inputs:
*
* Outputs:
*
* Returns: s32
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaFlvSetVidType(CfaFlvInst_T *prCfaFlv)
{
	if (NULL == prCfaFlv) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] fail for invalid args\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	switch (prCfaFlv->rVideoInfo.rCfgInfo.eCodecID) {
	case AVCODEC_ID_UNKNOWN:
		prCfaFlv->eVidCodecType = CFA_VID_UNKNOWN;
		break;
	case AVCODEC_ID_MJPEG:
		prCfaFlv->eVidCodecType = CFA_VID_UNKNOWN;
		break;
	case AVCODEC_ID_SORENSON:
		prCfaFlv->eVidCodecType = CFA_VID_H263_SORENSON;
		break;
	case AVCODEC_ID_SCRV:
		prCfaFlv->eVidCodecType = CFA_VID_UNKNOWN;
		break;

	case AVCODEC_ID_VP6:
		if(VCODEC_VERSION_NONE == prCfaFlv->rVideoInfo.rCfgInfo.rVersion)
			prCfaFlv->eVidCodecType = CFA_VID_VP6;
		else
			prCfaFlv->eVidCodecType = CFA_VID_VP6A;
		break;
	case AVCODEC_ID_H264:
		prCfaFlv->eVidCodecType = CFA_VID_H264;
		break;
	case AVCODEC_ID_H265:
		if(VCODEC_VERSION_NONE == prCfaFlv->rVideoInfo.rCfgInfo.rVersion)
			prCfaFlv->eVidCodecType = CFA_VID_H265;
		else
			prCfaFlv->eVidCodecType = CFA_VID_UNKNOWN;
		break;

	default:
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] fail for invalid Video Codec ID(0x%x)\r\n"),
			prCfaFlv->rVideoInfo.rCfgInfo.eCodecID);
		MM_RETURN(RET_DMX_PARAM_WRONG);
		break;
	}

	MM_RETURN(RET_DMX_OK);
}

static CfaApiAudType CfaFlvGetAudType(AVCODECID_T eInAudType)
{
	switch (eInAudType) {
	case AVCODEC_ID_PCM:
		return CFA_AUD_DRV_FMT_PCM;

	case AVCODEC_ID_MP3:
		return CFA_AUD_DRV_FMT_MP3;

	case AVCODEC_ID_AAC:
		return CFA_AUD_DRV_FMT_AAC;

	default:
		return CFA_AUD_DRV_FMT_UNKNOWN;
	}
}


/*-----------------------------------------------------------------------------
* Name: i4CfaFlvConfigure
*
* Description:
*      splitter will ensure that it is only called in "off" or "paused" state.
*
* Inputs:
*      [IN] handle of splitter
*      [IN] configure paramter
*      [IN] pointer to CfaFlvInst
*
* Outputs:
*
* Returns: s32
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaFlvConfigure(void *pvSptHdl, void *pvParam, void *pvPrivData, bool fgIsUserMem)
{
	CfaFlvInst_T *prCfaFlv = NULL;
	CfaFlvCfgInfo_T *prCfaFlvCfgInfo = NULL;
	CfaFlvVidInfo_T *prCfaFlvVidInfo = NULL;
	CfaFlvAudInfo_T *prCfaFlvAudInfo = NULL;
	CfaFlvCfgInfo_T rCfaFlvCfgInfo;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvPrivData) || (NULL == pvParam)) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] fail for invalid args\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prCfaFlv = (CfaFlvInst_T *) pvPrivData;
	prCfaFlvCfgInfo = &rCfaFlvCfgInfo;

	if (fgIsUserMem) {
		if (0 != mm_copy_from_user(prCfaFlvCfgInfo,
			pvParam, sizeof(CfaFlvCfgInfo_T))) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("[CFA_FLV] %s line %d failed in mm_copy_from_user\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
		}
	}
	else {
		mm_memcpy(prCfaFlvCfgInfo,
				pvParam, sizeof(CfaFlvCfgInfo_T));
	}

	/*config video info*/
	prCfaFlvVidInfo = &prCfaFlv->rVideoInfo;
	mm_memcpy(&(prCfaFlvVidInfo->rCfgInfo),
		&(prCfaFlvCfgInfo->rCfaFlvCfgVidInfo), sizeof(CfaFlvCfgVidInfo_T));

	/*set video type for transfering video data to video FIFO.*/
	mrRet = CfaFlvSetVidType(prCfaFlv);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] fail in CfaFlvSetVidType, mrRet: 0x%x\r\n"),
			mrRet);
		if (!fgIsUserMem)
			DMX_FreeMemory(pvParam);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		TEXT("[CFA_FLV] %s -- Video CodecID: %d, VidType: %d\r\n"),
		DMX_FUNC_NAME, prCfaFlv->rVideoInfo.rCfgInfo.eCodecID, prCfaFlv->eVidCodecType);

	if (NULL != prCfaFlv->puDecoderCfgBuf) {
		DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] %s -- free prCfaFlv->puDecoderCfgBuf=0x%x\r\n"),
			DMX_FUNC_NAME, prCfaFlv->puDecoderCfgBuf);
		DMX_FreeHwMemory(prCfaFlv->puDecoderCfgBuf);
		prCfaFlv->puDecoderCfgBuf = NULL;
	}

	prCfaFlv->u4DecoderCfgBufLen = (u32) prCfaFlv->rVideoInfo.rCfgInfo.u1SeqHdrLen;
	if (0 != prCfaFlv->u4DecoderCfgBufLen) {
		DMX_NewHwMemory(prCfaFlv->u4DecoderCfgBufLen, prCfaFlv->puDecoderCfgBuf);
		if (NULL == prCfaFlv->puDecoderCfgBuf) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("[CFA_FLV] fail in Alloc prCfaFlv->puDecoderCfgBuf memory fail\r\n"));
			if (!fgIsUserMem)
				DMX_FreeMemory(pvParam);
			MM_RETURN(RET_DMX_NO_MEM);
		}

		DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] %s -- alloc puDecoderCfgBuf=0x%x success\r\n"),
			DMX_FUNC_NAME, prCfaFlv->puDecoderCfgBuf);
		if (fgIsUserMem) {
			if (0 != mm_copy_from_user(prCfaFlv->puDecoderCfgBuf,
				prCfaFlv->rVideoInfo.rCfgInfo.puSeqHdr,
				prCfaFlv->u4DecoderCfgBufLen)) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					TEXT("[CFA_FLV] %s line %d failed in mm_copy_from_user\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
			}
		}
		else {
			dmx_memcpy(prCfaFlv->puDecoderCfgBuf,
					prCfaFlv->rVideoInfo.rCfgInfo.puSeqHdr,
					prCfaFlv->u4DecoderCfgBufLen);
		}
	}
	/*config audio info*/
	prCfaFlvAudInfo = &prCfaFlv->rAudioInfo;
	mm_memcpy(&(prCfaFlvAudInfo->rCfgInfo), &(prCfaFlvCfgInfo->rCfaFlvCfgAudInfo),
		sizeof(CfaFlvCfgAudInfo_T));

	prCfaFlvAudInfo->eAudType = CfaFlvGetAudType(prCfaFlvAudInfo->rCfgInfo.eCodecID);

	if (NULL != prCfaFlv->rAacCfgInfo.puHeader) {
		DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] %s -- free prCfaFlv->rAacCfgInfo.puHeader=0x%x\r\n"),
			DMX_FUNC_NAME, prCfaFlv->rAacCfgInfo.puHeader);
		DMX_FreeHwMemory(prCfaFlv->rAacCfgInfo.puHeader);
		prCfaFlv->rAacCfgInfo.puHeader = NULL;
	}

	prCfaFlv->rAacCfgInfo.uHeaderLen = prCfaFlv->rAudioInfo.rCfgInfo.rAacInfo.uHeaderLen;

	if (0 != prCfaFlv->rAacCfgInfo.uHeaderLen) {
		DMX_NewHwMemory(prCfaFlv->rAacCfgInfo.uHeaderLen, prCfaFlv->rAacCfgInfo.puHeader);
		if (NULL == prCfaFlv->rAacCfgInfo.puHeader) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("[CFA_FLV] fail in Alloc prCfaFlv->rAacCfgInfo.puHeader memory fail\r\n"));
			MM_RETURN(RET_DMX_NO_MEM);
		}

		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] %s -- alloc rAacCfaInfo.puHeader=0x%x HeaderLen=%dsuccess\r\n"),
			DMX_FUNC_NAME, prCfaFlv->rAacCfgInfo.puHeader,
			prCfaFlv->rAacCfgInfo.uHeaderLen);

		if (fgIsUserMem) {
			if (0 != mm_copy_from_user((prCfaFlv->rAacCfgInfo.puHeader),
				(prCfaFlv->rAudioInfo.rCfgInfo.rAacInfo.puHeader),
				(prCfaFlv->rAacCfgInfo.uHeaderLen))) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					TEXT("[CFA_FLV] %s line %d failed in mm_copy_from_user\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
			}
		}
		else {
			dmx_memcpy((prCfaFlv->rAacCfgInfo.puHeader),
					(prCfaFlv->rAudioInfo.rCfgInfo.rAacInfo.puHeader),
					(prCfaFlv->rAacCfgInfo.uHeaderLen));
		}
	}

	DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		TEXT("[CFA_FLV] %s -- prCfaFlv->rAacCfgInfo.puHeader=0x%x, uHeaderLen=0x%x\r\n"),
		DMX_FUNC_NAME, prCfaFlv->rAacCfgInfo.puHeader,
		prCfaFlv->rAacCfgInfo.uHeaderLen);

	/*config others info*/
	prCfaFlv->rFileInfo.u8FileSize = prCfaFlvCfgInfo->rCfaFlvCfgFileInfo.u8FileSize;
	prCfaFlv->rFileInfo.u8HeaderSize = prCfaFlvCfgInfo->rCfaFlvCfgFileInfo.u8HeaderSize;

	prCfaFlv->u4PayloadLenFieldSz = 0;
	if ((AVCODEC_ID_H264 == prCfaFlv->rVideoInfo.rCfgInfo.eCodecID) ||
		(VCODEC_VERSION_H265_HM91 == prCfaFlv->rVideoInfo.rCfgInfo.rVersion)) {
		prCfaFlv->u4PayloadLenFieldSz =
		    (u32) (prCfaFlv->rVideoInfo.rCfgInfo.u1PayloadLenFieldSz);

		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] %s - AVC/MPEG4_H265 Slice Size Field Len: %d\r\n"),
			DMX_FUNC_NAME, prCfaFlv->u4PayloadLenFieldSz);

		if (0 == prCfaFlv->u4PayloadLenFieldSz) {
			prCfaFlv->u4PayloadLenFieldSz = 4;
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("[CFA_FLV] %s - AVC/MPEG4_H265 Slice Size Field Len = 0, Change it to be 4\r\n"),
				DMX_FUNC_NAME);
		}
	}

	DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		TEXT("[CFA_FLV] %s -- FileSize:%llx\r\n"),
		DMX_FUNC_NAME, prCfaFlv->rFileInfo.u8FileSize);
	DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		TEXT("[CFA_FLV] %s -- Audio --> eCodecID:%d, eSoundType:%d\r\n"),
		DMX_FUNC_NAME, prCfaFlvAudInfo->rCfgInfo.eCodecID,
		prCfaFlvAudInfo->rCfgInfo.eSoundType);
	DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		TEXT("[CFA_FLV] %s -- Video --> eFrmType:%d, eCodecID:%d, u1FrameRate:%u\r\n"),
		DMX_FUNC_NAME, prCfaFlvVidInfo->eFrmType, prCfaFlvVidInfo->rCfgInfo.eCodecID,
		prCfaFlvVidInfo->rCfgInfo.u1FrameRate);

	if (!fgIsUserMem)
		DMX_FreeMemory(pvParam);

	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: i4CfaFlvSetInqTypes
*
* Description:
*      FLV CFA sets information query types
*      splitter will ensure that it is only called in "off" or "paused" state.
*
* Inputs:
*      [IN] handle of splitter
*      [IN] information type for FLV CFA
*      [IN] pointer to CfaFlvInst
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaFlvSetInqTypes(void *pvSptHdl, u32 u4InfTypes, void *pvPrivData)
{
	MM_RETURN(RET_DMX_OK);
}


/*< [IN] input splitter Handle*/
/*< [IN] CFA function id, set or get id, it shall be defined by CFA and LPE*/
/*< [IN] input CFA private data*/
/*< [OUT] The parameter of this FID, it shall be defined by CFA and LPE*/
/*< [IN] The size of this parameter (of this FID), it shall be defined by CFA and LPE*/
static MRESULT CfaFlvGetGeneral(void *pvSptHdl, u32 u4CfaFID, void *pvPrivData,
				void *pvCfaParameter, u32 u4CfaParameterSize)
{
	MM_RETURN(RET_DMX_OK);
}

/*-----------------------------------------------------------------------------
* Name: i4CfaFlvFillAUInfo
*
* Description:
*      FLV CFA callback for each AU is demuxed
*
*
* Inputs:
*      [IN] input splitter Handle
*      [IN/OUT] AU info,
*      [IN] input CFA private data
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
static MRESULT CfaFlvFillAUInfo(void *pvSptHdl, void *pvAUInfo, void *pvAUExtInfo, void *pvPrivData)
{
	u64 u8RealPTS = 0;
	u64 u8Offset = 0;
	CfaFlvInst_T *prCfaFlv = NULL;

	/* Because this function is called so frequently, and the caller should*/
	/* ensure all the params are valid, so we only add assert here.*/
    if((NULL == pvAUInfo) || (NULL == pvPrivData))
    {
    	DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] fail for invalid args\r\n"));
        MM_RETURN(RET_DMX_PARAM_WRONG);
    }

	prCfaFlv = (CfaFlvInst_T *) pvPrivData;

	MMATE_CHECK_POINTER(prCfaFlv);
	MMATE_CHECK_STRUCT(prCfaFlv->rRange);
	MMATE_CHECK_STRUCT(prCfaFlv->rDecoderCfgInfo);

	switch (prCfaFlv->eCurTxStrmType) {
	case CFA_FLV_TX_STRM_TYPE_VID:
		if ((prCfaFlv->prVidCmdQsInfo->fgIsInDma) &&
		    (prCfaFlv->prVidCmdQsInfo->u4EntryCnt > 0)) {
			u32 u4Idx = 0;
			bool fgInUnit = FALSE;

			for (u4Idx = 0; u4Idx < prCfaFlv->prVidCmdQsInfo->u4EntryCnt; u4Idx++) {
				if (prCfaFlv->prVidCmdQsInfo->arEntrys[u4Idx].fgUnitStart) {
					if (!fgInUnit) {
						fgInUnit = TRUE;
						prCfaFlv->prVidCmdQsInfo->arEntrys[u4Idx].
						    fgUnitStart = FALSE;
						u8RealPTS =
						    PTS_TO_MS(prCfaFlv->prVidCmdQsInfo->
							      arEntrys[u4Idx].u8Pts);
						break;
					}
				}
			}

			if (!fgInUnit)
				u8RealPTS = (u64) prCfaFlv->rVTimeStampMan.u4TimeStamp;
		} else {
			u8RealPTS = (u64) prCfaFlv->rVTimeStampMan.u4TimeStamp;
		}

		/* error handle: for some files, payload PTS may be earlier than preroll time*/
		u8Offset = prCfaFlv->rVideoInfo.rVTagInfo.u8Offset;
		((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.eDiscType = DT_FLV;

#if CFA_FLV_CHECK_PIC_TYPE
		if (fgIsBType(((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u4VType)) {
			if (CFA_PIC_B != prCfaFlv->ePrePicType) {
				/*Reset previous I/P frame pts as Invalid.*/
				((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8PrevPTS =
				    INVALID_TIMESTAMP;
			}
		}

		((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Pts = MS_TO_PTS(u8RealPTS);

		prCfaFlv->u8PreVPts = MS_TO_PTS(u8RealPTS);
		if (fgIsIType(((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u4VType)) {
			prCfaFlv->ePrePicType = CFA_PIC_I;
			prCfaFlv->rCurPosInfo.u8IFrmCurOfst = u8Offset;

			prCfaFlv->u8IFrameCnt++;
		} else if (fgIsPType(((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u4VType)) {
			prCfaFlv->ePrePicType = CFA_PIC_P;
		} else if (fgIsBType(((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u4VType)) {
			prCfaFlv->ePrePicType = CFA_PIC_B;
		} else {
			prCfaFlv->ePrePicType = CFA_PIC_UNDEFINE;
		}
		((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u4Duration = 0;
#else
		((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Pts = MS_TO_PTS(u8RealPTS);
#endif

		prCfaFlv->fgFirstVidAU = FALSE;

#if CFA_FLV_DBG_VID_PTS
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] Video -----> PTS = %lld ms,")
			TEXT(" TimeStamp = %d ms, CmdQEntryCnt: %d, u8RealPTS: %lld\r\n"),
			PTS_TO_MS((((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Pts)),
			prCfaFlv->rVTimeStampMan.u4TimeStamp,
			prCfaFlv->prVidCmdQsInfo->u4EntryCnt, u8RealPTS);
#endif				/* CFA_FLV_DBG_VID_PTS*/
		break;

	case CFA_FLV_TX_STRM_TYPE_AUD:
		/* error handle: for some files, payload PTS may be earlier than preroll time*/
		if ((prCfaFlv->prAudCmdQsInfo->fgIsInDma) &&
		    (prCfaFlv->prAudCmdQsInfo->u4EntryCnt > 0)) {
			u32 u4CmdIdx = 0;

			for (u4CmdIdx = 0; u4CmdIdx < prCfaFlv->prAudCmdQsInfo->u4EntryCnt;
			     u4CmdIdx++) {
				if (prCfaFlv->prAudCmdQsInfo->arEntrys[u4CmdIdx].fgUnitStart) {
					prCfaFlv->prAudCmdQsInfo->arEntrys[u4CmdIdx].fgUnitStart =
					    FALSE;
					((AU_AUDIO *) pvAUInfo)->rAUInfo.rInfo.u8Pts =
					    prCfaFlv->prAudCmdQsInfo->arEntrys[u4CmdIdx].u8Pts;
#if CFA_FLV_DBG_AUD_PTS
					DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						(TEXT("[CFA_FLV] 1 Audio line %d ")
						TEXT("----->PTS = %lld ms, TimeStamp = %d ms\r\n")),
						DMX_LINE_NO,
						PTS_TO_MS(((AU_AUDIO *) pvAUInfo)->rAUInfo.rInfo.u8Pts),
							prCfaFlv->rATimeStampMan.u4TimeStamp);
#endif				/*CFA_FLV_DBG_AUD_PTS*/

					break;
				}
			}

			if (u4CmdIdx >= prCfaFlv->prAudCmdQsInfo->u4EntryCnt) {
				u8RealPTS = prCfaFlv->rATimeStampMan.u4TimeStamp;
				((AU_AUDIO *) pvAUInfo)->rAUInfo.rInfo.u8Pts = MS_TO_PTS(u8RealPTS);
#if CFA_FLV_DBG_AUD_PTS
				DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					TEXT("[CFA_FLV] 2 Audio line %d ----->PTS = %lld ms, TimeStamp = %d ms\r\n"),
					DMX_LINE_NO,
					PTS_TO_MS(((AU_AUDIO *) pvAUInfo)->rAUInfo.rInfo.u8Pts),
						prCfaFlv->rATimeStampMan.u4TimeStamp);
#endif				/* CFA_FLV_DBG_AUD_PTS*/
			}
		} else {
			u8RealPTS = prCfaFlv->rATimeStampMan.u4TimeStamp;

#if 0
			if (u8RealPTS > (u64) prCfaFlv->rAudioInfo.rATagInfo.u4TimeStamp) {
				ASSERT((u8RealPTS -
					(u64) prCfaFlv->rAudioInfo.rATagInfo.u4TimeStamp) <
				       1000);
			} else {
				ASSERT(((u64) prCfaFlv->rAudioInfo.rATagInfo.u4TimeStamp -
					u8RealPTS) < 1000);
			}
#endif

			((AU_AUDIO *) pvAUInfo)->rAUInfo.rInfo.u8Pts = MS_TO_PTS(u8RealPTS);
#if CFA_FLV_DBG_AUD_PTS
			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("[CFA_FLV] 3 Audio line %d ----->PTS = %lld ms, TimeStamp = %d ms\r\n"),
				DMX_LINE_NO,
				PTS_TO_MS(((AU_AUDIO *) pvAUInfo)->rAUInfo.rInfo.u8Pts),
				prCfaFlv->rATimeStampMan.u4TimeStamp);
#endif				/*CFA_FLV_DBG_AUD_PTS*/
		}
		break;

	case CFA_FLV_TX_STRM_TYPE_NONE:
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] Current tx no vid & aud data to FIFO\r\n"));
		break;

	default:
        DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] prCfaFlv->eCurTxStrmType is error!\r\n"));
        MM_RETURN(RET_DMX_ERR_STATE);
		break;
	}

	MM_RETURN(RET_DMX_OK);
}

/*-----------------------------------------------------------------------------
* Name: i4CfaFlvTxAudHDRInfo
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
static MRESULT CfaFlvTxAudHdrInfo(void *pvSptHdl, u32 u4TxUID, void *pvPrivData)
{
	CfaFlvInst_T *prCfaFlv = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] fail for invalid args\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prCfaFlv = (CfaFlvInst_T *) pvPrivData;

	if (AVCODEC_ID_AAC != prCfaFlv->rAudioInfo.rCfgInfo.eCodecID)
		MM_RETURN(RET_DMX_UNSUPPORT);

	if ((DMX_INVALID_UINT32 == u4TxUID) || (prCfaFlv->rAudioInfo.rCfgInfo.u1StrmNum != u4TxUID)) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] fail for invalid u4TxUID(0x%x), AudStrmUID(0x%x)\r\n"),
			u4TxUID, prCfaFlv->rAudioInfo.rCfgInfo.u1StrmNum);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!prCfaFlv->fgFirstTxAud)
		MM_RETURN(RET_DMX_OK);

	mrRet = Spt4CfaBuf2AFifo(pvSptHdl, prCfaFlv->rAacCfgInfo.puHeader,
				prCfaFlv->rAacCfgInfo.uHeaderLen, u4TxUID,
				prCfaFlv->rAudioInfo.eAudType);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] fail in Spt4CfaBuf2AFifo, mrRet: 0x%x\r\n"),
			mrRet);
	}

	MM_RETURN(mrRet);
}

/*-----------------------------------------------------------------------------
 * Name: CfaAvi_SetJumpRange
 *
 * Description:
 *      Fro Support 8/16/32 fast forward and fast backward, to Reset CFA All state.
 *
 * Inputs:
 *      [IN] handle of splitter
 *      [IN] pointer to CfaAviKeyFrameRange
 *      [IN] pointer to CfaAviInst
 *
 * Outputs:
 *
 * Returns: s32

 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaFlvSetJumpRange(void *pvSptHdl, void *pvJmpRange, void *pvPrivData)
{
	CfaFlvKeyFrmRange_T *prKeyFrmRange = NULL;
	CfaFlvRange_T *prCfaRange = NULL;
	CfaFlvInst_T *prCfaInst = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvPrivData) || (NULL == pvJmpRange)) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_FFRW,
			TEXT("[CFA_FLV] fail for invalid args\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prCfaInst = (CfaFlvInst_T *) pvPrivData;

	if (fgIsCfaStmToPlay(prCfaInst->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_AUD) &&
	    (!fgIsCfaStmToPlay(prCfaInst->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_VID)) &&
	    (!prCfaInst->rRange.fgHasVid)) {
		if (0 == prCfaInst->u4PureAudTxUnitSz) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_FFRW,
				TEXT("[CFA_FLV] fail for Pure Audio")
				TEXT(" Tx Unit Size is 0 in Fast Forward or Rewind\r\n"));
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}

	prKeyFrmRange = (CfaFlvKeyFrmRange_T *) pvJmpRange;

	prCfaRange = &(prKeyFrmRange->rRange);

	prCfaInst->u4PureAudTxUnitSz = prKeyFrmRange->u4PureAudTxUnitSz;

#ifdef MM_ATE_CHECK
	mm_memcpy(&(prCfaInst->rRange.fgHasVid), &(((CfaFlvRange_T *)prCfaRange)->fgHasVid),
			 sizeof(CfaFlvRange_T) - 2 * sizeof(u32));
#else
	mm_memcpy(&(prCfaInst->rRange), prCfaRange, sizeof(CfaFlvRange_T));
#endif

	DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_FFRW,
		TEXT("[CFA_FLV] %s -- Jump VidSa: 0x%08x%08x, VidEa: 0x%08x%08x,")
		TEXT(" AudSa: 0x%08x%08x, AudEa: 0x%08x%08x, PureAudUnitSz: %d\r\n"),
		DMX_FUNC_NAME, (u32) ((prCfaRange->u8VidSa) >> 32),
		(u32) (prCfaRange->u8VidSa), (u32) ((prCfaRange->u8VidEa) >> 32),
		(u32) (prCfaRange->u8VidEa), (u32) ((prCfaRange->u8AudSa) >> 32),
		(u32) (prCfaRange->u8AudSa), (u32) ((prCfaRange->u8AudEa) >> 32),
		(u32) (prCfaRange->u8AudEa), prCfaInst->u4PureAudTxUnitSz);


	MMATE_CHECK_POINTER(prCfaInst);
	MMATE_CHECK_STRUCT(prCfaInst->rRange);
	MMATE_CHECK_STRUCT(prCfaInst->rDecoderCfgInfo);

	DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		TEXT("[CFA_FLV] %s line %d -- fgHasVid: %d, fgHasAud: %d\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		(prCfaInst->rRange.fgHasVid ? 1 : 0), (prCfaInst->rRange.fgHasAud ? 1 : 0));

	prCfaInst->rCurPosInfo.u8VidCurOfst = prCfaInst->rRange.u8VidSa;
	prCfaInst->rCurPosInfo.u8AudCurOfst = prCfaInst->rRange.u8AudSa;
	if ((prCfaInst->rRange.u8AudSa == 0) && (prCfaInst->rRange.u8VidSa == 0))
		prCfaInst->u8Ca = prCfaInst->rFileInfo.u8HeaderSize;
	else
		prCfaInst->u8Ca = MIN(prCfaInst->rRange.u8AudSa, prCfaInst->rRange.u8VidSa);

	prCfaInst->u8Endoffst = prCfaInst->rRange.u8VidEa;

	prCfaInst->u8FileSz = prCfaInst->rRange.u8FileSz;
	prCfaInst->fgFirstTxAud = TRUE;
	prCfaInst->fgFirstTxVid = TRUE;

	prCfaInst->fgJumpTurnOn = TRUE;

	prCfaInst->rRange.u8IframeNum = 0;

#if CONFIG_CFA_FLV_FOR_ERR_JUMP
	prCfaInst->u4OffsetJumpCnt = 0;
#endif

	DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		TEXT("[CFA_FLV] %s success -- Ca: %lld --->VidEa: %lld, SeekTime: %d ms\r\n"),
		DMX_FUNC_NAME, prCfaInst->u8Ca, prCfaInst->rRange.u8VidEa,
		prCfaInst->rRange.u8SeekTime);

	MM_RETURN(mrRet);
}

static MRESULT CfaFlvGetParamSize(void *pvSptHdl, u32 u4ParamID,
				  void *pvPrivData, void *pvCfaParam, u32 u4CfaParamSz)
{
	MRESULT mrRet = RET_DMX_OK;

	switch (u4ParamID) {
	case CFA_PARAM_ID_JUMP_INFO_SIZE:

		if ((NULL == pvCfaParam) || ((u4CfaParamSz) < sizeof(u32))) {
			mrRet = RET_DMX_PARAM_WRONG;
		} else {
			u32 *pu4Tmp = (u32 *) pvCfaParam;
			*pu4Tmp = sizeof(CfaFlvKeyFrmRange_T);
		}

		break;

	default:
		mrRet = RET_DMX_PARAM_WRONG;
		break;
	}

	MM_RETURN(mrRet);
}

static MRESULT CfaFlvProcCliCmd(void *pvSptHdl, E_DMX_CFA_CLI_TYPE_T eCliType, /*< [IN] Cfa Cli Command*/
				u32 arg1,
				u32 arg2, u32 arg3, const char *szParam, void *pvPrivData)
{
	MRESULT mrRet = RET_DMX_OK;
	CfaFlvInst_T *prCfaFlv = NULL;

	prCfaFlv = (CfaFlvInst_T *) pvPrivData;

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

			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("CfaFlvProcCliCmd -- fgEnable: %d, Loglvl: %d, ModLogLvl: 0x%08x \r\n"),
				arg1, arg2, arg3);

			DmxLogEnable(fgEnable, arg2, DMX_MOD_CFA_FLV, arg3);
		}
		break;
	case DMX_CFA_CLI_CMD_DUMP_INFO:
		{
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("Cfa FLV Instance(handle is 0x%x)")
				TEXT(" Info list as follow: \r\n"),
				prCfaFlv);
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("Current Analyse State is %d, Next Analyse")
				TEXT(" State is %d, Current Parsing Flag is %d \r\n"),
				prCfaFlv->eCurAnaSt, prCfaFlv->eNextAnaSt, prCfaFlv->u4CurPrsFlag);
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("Current Analyse Position is 0x%08x%08x, ")
				TEXT("Previous Analyse Position is 0x%08x%08x\r\n"),
				(u32) ((prCfaFlv->u8Ca) >> 32), (u32) (prCfaFlv->u8Ca),
				(u32) ((prCfaFlv->u8PreCa) >> 32), (u32) (prCfaFlv->u8PreCa));
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("First Tx Video Flag is %d, First Tx Audio Flag is %d\r\n"),
				((prCfaFlv->fgFirstTxVid) ? 1 : 0),
				((prCfaFlv->fgFirstTxAud) ? 1 : 0));
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
	__u8 uObjectProfile;
	__u8 uSampFreqIdx;
	__u8 uChannels;
	__u8 uHeaderLen;
	compat_caddr_t puHeader;
} AacCfgInfo_T32;

typedef struct {
	__u8 u1StrmNum;	/* CFA_FLV_AUDIO_STREAM_ID */
	AVCODECID_T eCodecID;
	CfaFlvAudioType_E eSoundType;	/* the number of audio channel */
	__u32 u4SoundRate;	/* Samples per second */
	__u16 u2SoundSize;	/* Size of each sample */
	AacCfgInfo_T32 rAacInfo;
} CfaFlvCfgAudInfo_T32;

typedef struct {
	__u8 u1StrmNum;	/* CFA_FLV_VIDEO_STREAM_ID */
	__u8 u1FrameRate;
	__u8 u1PrevTagSzAdd;
	AVCODECID_T eCodecID;
	VCODECVERSION_T rVersion;
	__u8 u1PayloadLenFieldSz;	/* Payload/Slice Length Field Size Per AVC Slice or Advance HEVC Slice */
	__u8 u1SeqHdrLen;
	compat_caddr_t puSeqHdr;
} CfaFlvCfgVidInfo_T32;

typedef struct {
	CfaFlvCfgFileInfo_T rCfaFlvCfgFileInfo;
	CfaFlvCfgAudInfo_T32 rCfaFlvCfgAudInfo;
	CfaFlvCfgVidInfo_T32 rCfaFlvCfgVidInfo;
} CfaFlvCfgInfo_T32;

typedef struct {
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKStart;
#endif
	bool fgHasVid;
	bool fgHasAud;

	__u64 u8VidSa;
	__u64 u8VidEa;

	__u32 u4AV1stTime;	/* > ms */

	CfaFlvSkipType_E eSkipMode;
	__u32 u4SkipPacketCount;
	__u64 u8DispPicPTS;

	__u64 u8AudSa;
	__u64 u8AudEa;
	__u64 u8SeekTime;	/* > ms */

	__u64 u8FileSz;

	__u8 u8IframeNum;	/* > get I_Frame count number after setrange */
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKEnd;
#endif
} CfaFlvRange_T32;

typedef struct {
	/* Information of Current Key Frame */
	__u64 u8StartPts;	/* The Start Pts of the Key Frame                                */
	__u64 u8StartOfst;	/* The Start file offset of the Key Frame  */
	__u64 u8CurKeyFrmMaxSz;	/* The max size of current key Frame                     */
	__u32 u4PureAudTxUnitSz;	/* One Audio AU's Max Size in FLV Pure Audio File's FF & RW */
	CfaFlvRange_T32 rRange;
} CfaFlvKeyFrmRange_T32;

static long CfaFlvCompatConfigCalcSz(CfaFlvCfgInfo_T32 __user *usr_ptr32, __u32 *pu4OutSz)
{
	__u32 u4TotalSz = 0;
	__u8 uHeaderLen = 0;

	if ((NULL == usr_ptr32) || (NULL == pu4OutSz)) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	u4TotalSz += CFA_ALIGN_SZ((__u32)sizeof(CfaFlvCfgInfo_T), sizeof(uintptr_t));

	if (0 != get_user(uHeaderLen,	&(usr_ptr32->rCfaFlvCfgAudInfo.rAacInfo.uHeaderLen))) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("%s line %d fail in get_user(rCfaFlvCfgAudInfo.rAacInfo.uHeaderLen)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	u4TotalSz += CFA_ALIGN_SZ((__u32)uHeaderLen, sizeof(uintptr_t));
	
	if (0 != get_user(uHeaderLen,	&(usr_ptr32->rCfaFlvCfgVidInfo.u1SeqHdrLen))) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("%s line %d fail in get_user(rCfaFlvCfgVidInfo.u1SeqHdrLen)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	
	u4TotalSz += CFA_ALIGN_SZ((__u32)uHeaderLen, sizeof(uintptr_t));

	*pu4OutSz = u4TotalSz;

	return 0;
}

static long CfaFlvCompatConfig(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaFlvCfgInfo_T __user *usr_ptr = NULL;
	CfaFlvCfgInfo_T32 __user *usr_ptr32 = (CfaFlvCfgInfo_T32 __user *)prInfo->usr_ptr32;
	__u32 u4TotalSz = 0;
	__u32 u4UseSz = 0;
	__u8 __user*pu1UsrBufAddr = NULL;
	__u8 __user*pu1NextBufAddr = NULL;
	compat_caddr_t compatAachdr = 0;
	__u8 __user *aac_hdr = NULL;
	compat_caddr_t compatSeqHdr = 0;
	long ret = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaFlvCfgInfo_T32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	if (0 != CfaFlvCompatConfigCalcSz(usr_ptr32, &u4TotalSz)) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaFlvCompatConfigCalcSz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	//pu1UsrBufAddr = (__u8 __user *)compat_alloc_user_space(u4TotalSz);
	DMX_NewMemory(u4TotalSz,pu1UsrBufAddr);

	if (NULL == pu1UsrBufAddr) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(pu1UsrBufAddr, 0, u4TotalSz);

	usr_ptr = (CfaFlvCfgInfo_T __user *)pu1UsrBufAddr;

	pu1NextBufAddr = pu1UsrBufAddr + CFA_ALIGN_SZ((__u32)sizeof(CfaFlvCfgInfo_T), sizeof(uintptr_t));
	u4UseSz += CFA_ALIGN_SZ((__u32)sizeof(CfaFlvCfgInfo_T), sizeof(uintptr_t));
	if (u4UseSz > u4TotalSz)
	{
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("%s line %d fail in u4UseSz > u4TotalSz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -ENOMEM;
	}

	if (copy_from_user(&(usr_ptr->rCfaFlvCfgFileInfo),
				&(usr_ptr32->rCfaFlvCfgFileInfo), sizeof(CfaFlvCfgFileInfo_T))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->rCfaFlvCfgAudInfo.u1StrmNum),
		&(usr_ptr32->rCfaFlvCfgAudInfo.u1StrmNum), sizeof(__u8))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rCfaFlvCfgAudInfo.eCodecID),
		&(usr_ptr32->rCfaFlvCfgAudInfo.eCodecID), sizeof(AVCODECID_T))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		TEXT("%s line %d -- Audio Codec ID: %d.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr->rCfaFlvCfgAudInfo.eCodecID);
	if (copy_from_user(&(usr_ptr->rCfaFlvCfgAudInfo.eSoundType),
		&(usr_ptr32->rCfaFlvCfgAudInfo.eSoundType), sizeof(CfaFlvAudioType_E))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rCfaFlvCfgAudInfo.u4SoundRate),
		&(usr_ptr32->rCfaFlvCfgAudInfo.u4SoundRate), sizeof(__u32))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rCfaFlvCfgAudInfo.u2SoundSize),
		&(usr_ptr32->rCfaFlvCfgAudInfo.u2SoundSize), sizeof(__u16))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rCfaFlvCfgAudInfo.rAacInfo.uObjectProfile),
			&(usr_ptr32->rCfaFlvCfgAudInfo.rAacInfo.uObjectProfile), sizeof(__u8))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rCfaFlvCfgAudInfo.rAacInfo.uSampFreqIdx),
			&(usr_ptr32->rCfaFlvCfgAudInfo.rAacInfo.uSampFreqIdx), sizeof(__u8))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rCfaFlvCfgAudInfo.rAacInfo.uChannels),
			&(usr_ptr32->rCfaFlvCfgAudInfo.rAacInfo.uChannels), sizeof(__u8))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rCfaFlvCfgAudInfo.rAacInfo.uHeaderLen),
			&(usr_ptr32->rCfaFlvCfgAudInfo.rAacInfo.uHeaderLen), sizeof(__u8))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if ((0 < usr_ptr32->rCfaFlvCfgAudInfo.rAacInfo.uHeaderLen) &&
			(0 == usr_ptr32->rCfaFlvCfgAudInfo.rAacInfo.puHeader)) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("%s line %d fail for no aac header, but header len(%d) > 0.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr->rCfaFlvCfgAudInfo.rAacInfo.uHeaderLen);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}

	if ((0 != usr_ptr32->rCfaFlvCfgAudInfo.rAacInfo.puHeader) &&
			(0 < usr_ptr32->rCfaFlvCfgAudInfo.rAacInfo.uHeaderLen)) {
		usr_ptr->rCfaFlvCfgAudInfo.rAacInfo.puHeader = pu1NextBufAddr;
		pu1NextBufAddr += CFA_ALIGN_SZ((__u32)usr_ptr->rCfaFlvCfgAudInfo.rAacInfo.uHeaderLen, sizeof(uintptr_t));
		u4UseSz += CFA_ALIGN_SZ((__u32)usr_ptr->rCfaFlvCfgAudInfo.rAacInfo.uHeaderLen, sizeof(uintptr_t));
		if (u4UseSz > u4TotalSz)
		{
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("%s line %d fail in u4UseSz > u4TotalSz.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -ENOMEM;
		}

		if (NULL == usr_ptr->rCfaFlvCfgAudInfo.rAacInfo.puHeader) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("%s line %d fail in alloc compat user space.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -ENOMEM;
		}
		mm_memset(usr_ptr->rCfaFlvCfgAudInfo.rAacInfo.puHeader, 0,
			sizeof(__u8) * usr_ptr->rCfaFlvCfgAudInfo.rAacInfo.uHeaderLen);

		if (get_user(compatAachdr, &(usr_ptr32->rCfaFlvCfgAudInfo.rAacInfo.puHeader))) {
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}
		if (0 == compatAachdr) {
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}
		aac_hdr = compat_ptr(compatAachdr);
		if (!access_ok(VERIFY_READ, aac_hdr, 
			sizeof(__u8) * usr_ptr->rCfaFlvCfgAudInfo.rAacInfo.uHeaderLen)) {
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}

		if (copy_from_user((__u8 __user *)usr_ptr->rCfaFlvCfgAudInfo.rAacInfo.puHeader,
			aac_hdr, sizeof(__u8) * usr_ptr->rCfaFlvCfgAudInfo.rAacInfo.uHeaderLen)) {
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}
	}

	if (copy_from_user(&(usr_ptr->rCfaFlvCfgVidInfo.u1StrmNum),
		&(usr_ptr32->rCfaFlvCfgVidInfo.u1StrmNum), sizeof(__u8))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rCfaFlvCfgVidInfo.u1FrameRate),
		&(usr_ptr32->rCfaFlvCfgVidInfo.u1FrameRate), sizeof(__u8))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rCfaFlvCfgVidInfo.u1FrameRate),
		&(usr_ptr32->rCfaFlvCfgVidInfo.u1FrameRate), sizeof(__u8))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rCfaFlvCfgVidInfo.eCodecID),
		&(usr_ptr32->rCfaFlvCfgVidInfo.eCodecID), sizeof(AVCODECID_T))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rCfaFlvCfgVidInfo.rVersion),
		&(usr_ptr32->rCfaFlvCfgVidInfo.rVersion), sizeof(VCODECVERSION_T))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rCfaFlvCfgVidInfo.u1PayloadLenFieldSz),
		&(usr_ptr32->rCfaFlvCfgVidInfo.u1PayloadLenFieldSz), sizeof(__u8))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rCfaFlvCfgVidInfo.u1SeqHdrLen),
		&(usr_ptr32->rCfaFlvCfgVidInfo.u1SeqHdrLen), sizeof(__u8))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if ((0 < usr_ptr32->rCfaFlvCfgVidInfo.u1SeqHdrLen) &&
		(0 == usr_ptr32->rCfaFlvCfgVidInfo.puSeqHdr)) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("%s line %d fail for no aac header, but header len(%d) > 0.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr->rCfaFlvCfgVidInfo.u1SeqHdrLen);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}

	if (0 != usr_ptr32->rCfaFlvCfgVidInfo.puSeqHdr) {
		__u8 *pu1SeqHdr = NULL;

		usr_ptr->rCfaFlvCfgVidInfo.puSeqHdr = pu1NextBufAddr;
		pu1NextBufAddr += CFA_ALIGN_SZ((__u32)usr_ptr->rCfaFlvCfgVidInfo.u1SeqHdrLen, sizeof(uintptr_t));
		u4UseSz += CFA_ALIGN_SZ((__u32)usr_ptr->rCfaFlvCfgVidInfo.u1SeqHdrLen, sizeof(uintptr_t));
		if (u4UseSz > u4TotalSz)
		{
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("%s line %d fail in u4UseSz > u4TotalSz.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -ENOMEM;
		}

		if (NULL == usr_ptr->rCfaFlvCfgVidInfo.puSeqHdr) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("%s line %d fail in alloc compat user space.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -ENOMEM;
		}
		mm_memset(usr_ptr->rCfaFlvCfgVidInfo.puSeqHdr, 0,
			sizeof(__u8) * usr_ptr->rCfaFlvCfgVidInfo.u1SeqHdrLen);

		if (get_user(compatSeqHdr, &(usr_ptr32->rCfaFlvCfgVidInfo.puSeqHdr))) {
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}
		if (0 == compatSeqHdr) {
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}

		pu1SeqHdr = compat_ptr(compatSeqHdr);
		if (!access_ok(VERIFY_READ, pu1SeqHdr, 
			sizeof(__u8) * usr_ptr->rCfaFlvCfgVidInfo.u1SeqHdrLen)) {
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}

		if (copy_from_user((__u8 __user *)usr_ptr->rCfaFlvCfgVidInfo.puSeqHdr,
				pu1SeqHdr, sizeof(__u8) * usr_ptr->rCfaFlvCfgVidInfo.u1SeqHdrLen)) {
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}
	}

	*pfgIsUserMem = FALSE;
	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaFlvCfgInfo_T);

	return 0;
}

static long CfaFlvCompatRange(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaFlvRange_T __user *usr_ptr = NULL;
	CfaFlvRange_T32 __user *usr_ptr32 = (CfaFlvRange_T32 __user *)prInfo->usr_ptr32;
	long ret = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaFlvRange_T32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	usr_ptr = (CfaFlvRange_T *)compat_alloc_user_space(sizeof(CfaFlvRange_T));

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaFlvRange_T));

	if (copy_in_user(&(usr_ptr->fgHasVid), &(usr_ptr32->fgHasVid), sizeof(bool)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->fgHasAud), &(usr_ptr32->fgHasAud), sizeof(bool)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8VidSa), &(usr_ptr32->u8VidSa), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8VidEa), &(usr_ptr32->u8VidEa), sizeof(__u64)))
		return -EFAULT;
  
	if (copy_in_user(&(usr_ptr->u4AV1stTime), &(usr_ptr32->u4AV1stTime), sizeof(__u32)))
		return -EFAULT;
  
	if (copy_in_user(&(usr_ptr->eSkipMode), &(usr_ptr32->eSkipMode), sizeof(CfaFlvSkipType_E)))
		return -EFAULT;
  
	if (copy_in_user(&(usr_ptr->u4SkipPacketCount), &(usr_ptr32->u4SkipPacketCount), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8DispPicPTS), &(usr_ptr32->u8DispPicPTS), sizeof(__u64)))
		return -EFAULT; 

	if (copy_in_user(&(usr_ptr->u8AudSa), &(usr_ptr32->u8AudSa), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8AudEa), &(usr_ptr32->u8AudEa), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8SeekTime), &(usr_ptr32->u8SeekTime), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8FileSz), &(usr_ptr32->u8FileSz), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8IframeNum), &(usr_ptr32->u8IframeNum), sizeof(__u8)))
		return -EFAULT;

	*pfgIsUserMem = TRUE;
	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaFlvRange_T);

	return 0;
}

static long CfaFlvCompatJumpRange(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaFlvKeyFrmRange_T __user *usr_ptr = NULL;
	CfaFlvKeyFrmRange_T32 __user *usr_ptr32 = (CfaFlvRange_T32 __user *)prInfo->usr_ptr32;
	long ret = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaFlvKeyFrmRange_T32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	usr_ptr = (CfaFlvKeyFrmRange_T *)compat_alloc_user_space(sizeof(CfaFlvKeyFrmRange_T));

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaFlvKeyFrmRange_T));

	if (copy_in_user(&(usr_ptr->u8StartPts), &(usr_ptr32->u8StartPts), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8StartOfst), &(usr_ptr32->u8StartOfst), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8CurKeyFrmMaxSz), &(usr_ptr32->u8CurKeyFrmMaxSz), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u4PureAudTxUnitSz),
			&(usr_ptr32->u4PureAudTxUnitSz), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->rRange.fgHasVid), &(usr_ptr32->rRange.fgHasVid), sizeof(bool)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->rRange.fgHasAud), &(usr_ptr32->rRange.fgHasAud), sizeof(bool)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->rRange.u8VidSa), &(usr_ptr32->rRange.u8VidSa), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->rRange.u8VidEa), &(usr_ptr32->rRange.u8VidEa), sizeof(__u64)))
		return -EFAULT;
  
	if (copy_in_user(&(usr_ptr->rRange.u4AV1stTime), &(usr_ptr32->rRange.u4AV1stTime), sizeof(__u32)))
		return -EFAULT;
  
	if (copy_in_user(&(usr_ptr->rRange.eSkipMode),
			&(usr_ptr32->rRange.eSkipMode), sizeof(CfaFlvSkipType_E)))
		return -EFAULT;
  
	if (copy_in_user(&(usr_ptr->rRange.u4SkipPacketCount),
			&(usr_ptr32->rRange.u4SkipPacketCount), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->rRange.u8DispPicPTS),
			&(usr_ptr32->rRange.u8DispPicPTS), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->rRange.u8AudSa),
			&(usr_ptr32->rRange.u8AudSa), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->rRange.u8AudEa),
			&(usr_ptr32->rRange.u8AudEa), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->rRange.u8SeekTime),
			&(usr_ptr32->rRange.u8SeekTime), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->rRange.u8FileSz),
			&(usr_ptr32->rRange.u8FileSz), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->rRange.u8IframeNum),
			&(usr_ptr32->rRange.u8IframeNum), sizeof(__u8)))
		return -EFAULT;

	*pfgIsUserMem = TRUE;
	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaFlvKeyFrmRange_T);

	return 0;
}

static int CfaFlvProcCompat(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	long ret = 0;

	if ((NULL == prInfo) || (NULL == pfgIsUserMem)) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					TEXT("%s line %d fail for invalid parameter.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	}
	switch (prInfo->type) {
		case CFA_CONFIG:
			if (prInfo->is_get) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EPERM;
			}
			ret = CfaFlvCompatConfig(prInfo, pfgIsUserMem);
			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaFlvCompatConfig.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return ret;
			}
			break;
		case CFA_RANGE:
			if (prInfo->is_get) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EPERM;
			}
			ret = CfaFlvCompatRange(prInfo, pfgIsUserMem);
			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaFlvCompatRange.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return ret;
			}
			break;
		case CFA_GEN_INFO:
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("%s line %d fail for don't support get info for cfa flv.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return -EPERM;
		case CFA_JUMP_INFO:
			if (prInfo->is_get) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EPERM;
			}
			ret = CfaFlvCompatJumpRange(prInfo, pfgIsUserMem);
			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaFlvCompatJumpRange.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return ret;
			}
			break;
		default:
			break;
	}

	return 0;
}

#endif /* CONFIG_COMPAT */

/* FLV CFA interface */
CfaIntf _rFlvCfaIntf = {
	&CfaFlvInit,
	&CfaFlvUninit,
	&CfaFlvSetRange,
	&CfaFlvEnableStrm,
	&CfaFlvSetStrmInf,
	&CfaFlvTurnOn,
	&CfaFlvTxDone,
	&CfaFlvGetCurPos,
	&CfaFlvFillPicInfo,
	&CfaFlvConfigure,
	&CfaFlvSetInqTypes,
	&CfaFlvGetGeneral,
	NULL,
	NULL,
	&CfaFlvFillAUInfo,
	&CfaFlvTxAudHdrInfo,
	NULL,
	&CfaFlvSetJumpRange,
	&CfaFlvGetParamSize,
	&CfaFlvProcCliCmd
#ifdef CONFIG_COMPAT
	,&CfaFlvProcCompat
#endif
};


/*-----------------------------------------------------------------------------
* Name: pvCfaFlvGetInterface
*
* Description:
*      Start of Public Function
*
* Inputs:
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
void *CfaFlvGetInterface(void)
{
	return (void *) &_rFlvCfaIntf;
}
