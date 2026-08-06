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
/* #include <media/atc/mm_debug.h> */

#include "cfa_rm_st_ctrl.h"
#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_spt.h"
#include "cfa_macro.h"

//#pragma warning(disable : 4100) /* disable warning C4100: unreferenced formal parameter*/
//#pragma warning(disable : 4127) /* disable warning C4127: conditional expression is constant*/


/*Description: When rm cfa finished parsing, call this function.
@return none*/
static void CfaRmFinishPrs(void *pvSptHdl, CfaRmInst_T *prCfaRm)
{
	u64 u8Ea = 0;

	if (NULL == prCfaRm)
	{
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
		TEXT("[CFA_RM_ST_CTRL] CfaRmFinishPrs:: prCfaRm is NULL.\r\n"));
		return;
	}
#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaRm);
	MMATE_CHECK_STRUCT(prCfaRm->rRange);
	MMATE_CHECK_STRUCT(prCfaRm->rPacketInfo);
#endif

	u8Ea = prCfaRm->rRange.u8VidEa;
	prCfaRm->eCurAnaSt = CFA_RM_ANA_ST_IDLE;
	DmxLogD(DMX_MOD_CFA_RM,CFA_RM_LOG_STATE,TEXT("[CFA_RM_ST_CTRL] Finished parsing for current range!\r\n"));
	if (DMX_IS_FF_PLAY(pvSptHdl))
	{
		Spt4CfaFinishedEx(pvSptHdl, u8Ea, TRUE, (u32)GAU_E_EOS);
	}
	else
	{
		Spt4CfaFinishedEx(pvSptHdl, u8Ea, FALSE, (u32)GAU_E_EOS);
	}
}

/*Description: Set picture tx mode in terms of video codec type
@Return: cfa picture tx mode. 04/10/2008*/
static CfaApiPicTxMode CfaRmSetPicTxMode(CfaRmInst_T *prCfaRm)
{
	prCfaRm->eCurPicType = CFA_PTM_EXACT_POS;

	switch (prCfaRm->eVidCodecType) {
	case CFA_VID_RV30:
		if (0x00 == (prCfaRm->u1FirstBytePay & (u8)0x18))
			prCfaRm->eCurPicType = CFA_PTM_RM_INTRAPIC; /*INTRAPIC*/
		else if ((u8)0x08 == (prCfaRm->u1FirstBytePay & (u8)0x18))
			prCfaRm->eCurPicType = CFA_PTM_RM_FORCED_INTRAPIC; /*FORCED_INTRAPIC*/
		else if ((u8)0x10 == (prCfaRm->u1FirstBytePay & (u8)0x18))
			prCfaRm->eCurPicType = CFA_PTM_RM_INTERPIC; /*INTERPIC*/
		else
			prCfaRm->eCurPicType = CFA_PTM_RM_TRUEBPIC; /*TRUEBPIC*/
		break;

	case CFA_VID_RV40:
		if (0x00 == (prCfaRm->u1FirstBytePay & (u8)0x60))
			prCfaRm->eCurPicType = CFA_PTM_RM_INTRAPIC; /*INTRAPIC*/
		else if ((u8)0x20 == (prCfaRm->u1FirstBytePay & (u8)0x60))
			prCfaRm->eCurPicType = CFA_PTM_RM_FORCED_INTRAPIC; /*FORCED_INTRAPIC*/
		else if ((u8)0x40 == (prCfaRm->u1FirstBytePay & (u8)0x60))
			prCfaRm->eCurPicType = CFA_PTM_RM_INTERPIC; /*INTERPIC*/
		else
			prCfaRm->eCurPicType = CFA_PTM_RM_TRUEBPIC;  /*TRUEBPIC*/
		break;

	default:
		break;
	}
	return prCfaRm->eCurPicType;
}

void CfaRmGetPicPts(CfaRmInst_T *prCfaRm)
{
	s32 i4TrDelta = 0;

	switch (prCfaRm->eVidCodecType) {
	case CFA_VID_RV30:
		prCfaRm->u4CurPicTr = ((prCfaRm->u4First4BytesPay & (u32)0xFFF80) >> 7);
		if (0x18 == (prCfaRm->u1FirstBytePay & (u8)0x18)) {/*TRUEBPIC */
			if (0 != prCfaRm->u4ForwardRefTr) {
				i4TrDelta = prCfaRm->u4CurPicTr - prCfaRm->u4ForwardRefTr;
				if (i4TrDelta < 0)
					i4TrDelta += (s32)0x2000;
			}

			if ((0 == prCfaRm->u8ForwardRefPts) && (0 != prCfaRm->u8BackwardRefPts))
				prCfaRm->u8PrsPts = prCfaRm->u8BackwardRefPts + (i4TrDelta * CFA_RM_SYS_CLK);
			else
				prCfaRm->u8PrsPts = prCfaRm->u8ForwardRefPts + (i4TrDelta * CFA_RM_SYS_CLK);
		} else {
			prCfaRm->u8PreVBPts = 0;
			prCfaRm->u4ForwardRefTr = prCfaRm->u4BackwardRefTr;
			prCfaRm->u4BackwardRefTr = prCfaRm->u4CurPicTr;
			prCfaRm->u8ForwardRefPts = prCfaRm->u8BackwardRefPts;
			prCfaRm->u8BackwardRefPts = prCfaRm->rPacketInfo.u4TimeStamp * CFA_RM_SYS_CLK;
		}
		break;

	case CFA_VID_RV40:
		prCfaRm->u4CurPicTr = ((prCfaRm->u4First4BytesPay & 0x7FFC0) >> 6);
		if (0x60 == (prCfaRm->u1FirstBytePay & 0x60)) {/*TRUEBPIC */
			if (0 != prCfaRm->u4ForwardRefTr) {
				i4TrDelta = prCfaRm->u4CurPicTr - prCfaRm->u4ForwardRefTr;
				if (i4TrDelta < 0)
					i4TrDelta += 0x2000;
			}

			if ((0 == prCfaRm->u8ForwardRefPts) && (0 != prCfaRm->u8BackwardRefPts))
				prCfaRm->u8PrsPts = prCfaRm->u8BackwardRefPts + (i4TrDelta * CFA_RM_SYS_CLK);
			else
				prCfaRm->u8PrsPts = prCfaRm->u8ForwardRefPts + (i4TrDelta * CFA_RM_SYS_CLK);
		} else {
			prCfaRm->u4ForwardRefTr = prCfaRm->u4BackwardRefTr;
			prCfaRm->u4BackwardRefTr = prCfaRm->u4CurPicTr;
			prCfaRm->u8ForwardRefPts = prCfaRm->u8BackwardRefPts;
			prCfaRm->u8BackwardRefPts = prCfaRm->rPacketInfo.u4TimeStamp * CFA_RM_SYS_CLK;
		}
		break;

	default:
		break;
	}
}

void CfaRmAdjustSliceInfo(CfaRmInst_T *prCfaRm)
{
	if ((0x8000 == prCfaRm->rFrameData.rFrameInfo.u2BrokenUpByUs) &&
		(prCfaRm->rSliceInf.u1TotalSliceNum > 1) &&
		(2 == prCfaRm->rSliceInf.rSliceInf[1].u1SliceElemNum)) {
		prCfaRm->rSliceInf.u1TotalSliceNum = 1;
		prCfaRm->rSliceInf.rSliceInf[0].u2SliceElemSize =
			prCfaRm->rSliceInf.rSliceInf[0].u2SliceElemSize +
			prCfaRm->rSliceInf.rSliceInf[1].u2SliceElemSize;
	}
	prCfaRm->rFrameData.rFrameInfo.u2BrokenUpByUs = 0;

	if (CFA_VID_RV30 == prCfaRm->eVidCodecType) {
		if (((prCfaRm->u1SliceFirstByte) & (u8)0x20) && ((prCfaRm->rSliceInf.u1TotalSliceNum) > (u8)1))
			prCfaRm->rSliceInf.u1TotalSliceNum--;
	} else if (CFA_VID_RV40 == prCfaRm->eVidCodecType) {
		if ((prCfaRm->u1SliceFirstByte & (u8)0x80) && (prCfaRm->rSliceInf.u1TotalSliceNum > (u8)1))
			prCfaRm->rSliceInf.u1TotalSliceNum--;
	}
		else
		{
			/*do nothing*/
		}
}

static MRESULT CfaRmTxAudData2Fifo(void *pvSptHdl, CfaRmInst_T *prCfaRm)
{
	MRESULT mrResult = RET_DMX_OK;
	u64 u8Sa = 0;
	u32 u4TxLen = 0;
	u64 u8TxTotalLen = 0;
	u64 u8Pts = 0;
	CFA_AUDIO_INFO_T rCfaRmTxAudInfo = {0};

	if (NULL == prCfaRm)
	{
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	u8Pts = prCfaRm->rPacketInfo.u4TimeStamp * CFA_RM_SYS_CLK;

	if ((RM_STREAM_TYPE_AUDIO == (prCfaRm->rPacketInfo.ePacketType)) &&
		(AVCODEC_ID_AAC == (prCfaRm->rAudioInfo.rCfgInfo.eCodecID))) {
		u8Sa = prCfaRm->rPacketInfo.u8Offset;
		if (((prCfaRm->rPacketInfo.u2FramNum) > 0) &&
			((prCfaRm->rPacketInfo.u2CurFramNum) < (prCfaRm->rPacketInfo.u2FramNum))) {
			u8Sa = prCfaRm->rPacketInfo.u8Offset + prCfaRm->rPacketInfo.u4FramOffset;
			if (prCfaRm->rPacketInfo.u2CurFramNum >= 128) {
				DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
					TEXT("[CFA_RM_ST_CTRL] The current frame number in packet")
					TEXT("is bigger than  127!!\r\n"));
				CfaRmFinishPrs(pvSptHdl, prCfaRm);
				MM_RETURN(RET_DMX_OK);
			}
			u4TxLen = prCfaRm->rPacketInfo.arFramLen[prCfaRm->rPacketInfo.u2CurFramNum];
			prCfaRm->rPacketInfo.u4FramOffset +=
				prCfaRm->rPacketInfo.arFramLen[prCfaRm->rPacketInfo.u2CurFramNum];
			prCfaRm->rPacketInfo.u2CurFramNum++;

			if (prCfaRm->rPacketInfo.u2CurFramNum == prCfaRm->rPacketInfo.u2FramNum)
				prCfaRm->eCurAnaSt = CFA_RM_ANA_ST_SEARCH_HEADER;
			else
				prCfaRm->eCurAnaSt = CFA_RM_ANA_ST_TX_A_HDR;
		} else {
			u4TxLen = prCfaRm->rPacketInfo.u2Length - 12;
			prCfaRm->eCurAnaSt = CFA_RM_ANA_ST_SEARCH_HEADER;
		}
	} else {
		u8Sa = prCfaRm->rPacketInfo.u8Offset + 12;

		if (prCfaRm->rPacketInfo.u2Length > 0)
			u4TxLen = prCfaRm->rPacketInfo.u2Length - 12;
		else
			u4TxLen = 0;
	}

	if (prCfaRm->fgFirstTxAud) {
		prCfaRm->fgFirstTxAud = FALSE;
		rCfaRmTxAudInfo.fgUnitStart = FALSE;

		if (DMX_IS_RW_PLAY(pvSptHdl) && (!prCfaRm->fgHasVideo))
			rCfaRmTxAudInfo.fgUnitStart = TRUE;

	} else
		rCfaRmTxAudInfo.fgUnitStart = FALSE;

	/*transfer audio data to audio fifo.*/
	prCfaRm->eCurTxStrmType = CFA_RM_TX_STRM_TYPE_AUD;

	rCfaRmTxAudInfo.u8FileOfst = u8Sa;
	rCfaRmTxAudInfo.u8Len = (u64)u4TxLen;
	rCfaRmTxAudInfo.u8Pts = u8Pts; /*change unit in Hz, STC Clock*/
	rCfaRmTxAudInfo.u4PrsStrmId = (u32)prCfaRm->rAudioInfo.rCfgInfo.u1StrmNum;/*need modify*/
	rCfaRmTxAudInfo.eAudType = prCfaRm->rAudioInfo.eAudType;

	if (DMX_IS_RW_PLAY(pvSptHdl) && (!prCfaRm->fgHasVideo)) {
		rCfaRmTxAudInfo.fgAUCompleteByEnd = TRUE;
		rCfaRmTxAudInfo.fgUnitEnd = FALSE;
	}

	u8TxTotalLen = 0;

	if (CFA_AUD_DRV_FMT_COOK == prCfaRm->rAudioInfo.eAudType) {
		if (prCfaRm->u4PacketSum == 1) {
			u8TxTotalLen = prCfaRm->u4SuperBlockSize;
			rCfaRmTxAudInfo.fgUnitStart = TRUE;
		} else
		  rCfaRmTxAudInfo.u8Pts = (u64)(-1);
	}
	if (prCfaRm->u8Endoffst < (rCfaRmTxAudInfo.u8FileOfst + rCfaRmTxAudInfo.u8Len)) {
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
			TEXT("[CFA_RM_ST_CTRL] line %d send EOS,u8Endoffst[0x%llx],")
			TEXT("u8FileOfst[0x%llx],u8Len[0x%llx]\r\n"),
			DMX_LINE_NO, prCfaRm->u8Endoffst, rCfaRmTxAudInfo.u8FileOfst, rCfaRmTxAudInfo.u8Len);
		CfaRmFinishPrs(pvSptHdl, prCfaRm);
		MM_RETURN(RET_DMX_OK);
	}

	if (DMX_IS_RW_PLAY(pvSptHdl) && (!prCfaRm->fgHasVideo)) {
		if (prCfaRm->fgFinishRWAU) {
			DmxLogT(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM_ST_CTRL] RW----Create an AU, so Finish\r\n"));
			mrResult = Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Endoffst, FALSE, GAU_E_EOS);
			MM_RETURN(mrResult);
		}

		if (0 == prCfaRm->u4RWUnitAULen) {
			rCfaRmTxAudInfo.fgUnitEnd = TRUE;
			rCfaRmTxAudInfo.u8FileOfst = prCfaRm->u8Ca;
			rCfaRmTxAudInfo.u8Len = 0;
			rCfaRmTxAudInfo.u8TotalAULen = 0;
			prCfaRm->fgFinishRWAU = TRUE;
			DmxLogT(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM_ST_CTRL] RW----Create an AU\r\n"));
			mrResult = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rCfaRmTxAudInfo);
			MM_RETURN(mrResult);
		}

		if (prCfaRm->u4RWUnitAULen < rCfaRmTxAudInfo.u8Len)
			rCfaRmTxAudInfo.u8Len = prCfaRm->u4RWUnitAULen;

		prCfaRm->u4RWUnitAULen -= (u32)rCfaRmTxAudInfo.u8Len;
		rCfaRmTxAudInfo.u8TotalAULen = 0;
		mrResult = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rCfaRmTxAudInfo);
	} else {
		if (0 == rCfaRmTxAudInfo.u8Len) {
			DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
				TEXT("[CFA_RM_ST_CTRL] tx len is invalid(0), so skip or send eos\r\n"));
			if (((u64)CFA_RM_ENDFILE_OFFSET <= (prCfaRm->u8Endoffst)) &&
				 ((prCfaRm->u8Ca) < ((prCfaRm->u8Endoffst) - (u64)CFA_RM_ENDFILE_OFFSET))) {
				Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, TRUE, (u32)GAU_E_ERRCHUNK);
				MM_RETURN(RET_DMX_OK);
			} else {
				CfaRmFinishPrs(pvSptHdl, prCfaRm);
				MM_RETURN(RET_DMX_OK);
			}
		}
		rCfaRmTxAudInfo.u8TotalAULen = u8TxTotalLen;
		mrResult = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rCfaRmTxAudInfo);
	}
	if (mrResult != RET_DMX_OK)
	{
		if (DMX_IS_RW_PLAY(pvSptHdl))
		{
			Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, FALSE, (u32)GAU_E_ERRCHUNK);
		}
		else
		{
			Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, TRUE, GAU_E_ERRCHUNK);
		}
		MM_RETURN(mrResult);
	}
	if (CFA_VID_UNKNOWN == prCfaRm->eVidCodecType)
		Sleep(1);
	MM_RETURN(RET_DMX_OK);
}

void SetRmAacHdr(const CfaRmInst_T *prCfaRm)
{
	u32 u4Channels = 0;
	u32 u4SamplePerSec = 0;
	u32 u4FrameSize = 0;
	u8 u1Temp1 = 0;
	u8 u1Temp2 = 0;

	if (NULL == prCfaRm)
	{
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
		TEXT("[CFA_RM_ST_CTRL] SetRmAacHdr:: prCfaRm is NULL.\r\n"));
		return;
	}

	u4FrameSize = prCfaRm->rPacketInfo.u2Length - (u16)12 + (u16)7;

	if ((prCfaRm->rPacketInfo.u2FramNum > 0) &&
			(prCfaRm->rPacketInfo.u2CurFramNum < prCfaRm->rPacketInfo.u2FramNum)) {
		if (prCfaRm->rPacketInfo.u2CurFramNum >= 128)
		{
			DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT, TEXT("[CFA_RM_ST_CTRL] line %d,u2CurFramNum = %d error!\r\n"),DMX_LINE_NO,prCfaRm->rPacketInfo.u2CurFramNum);
			return;
		}
		u4FrameSize = prCfaRm->rPacketInfo.arFramLen[prCfaRm->rPacketInfo.u2CurFramNum] + (u16)7;
	}

	u1Temp1 = prCfaRm->rAacCfgInfo.puHeader[4];
	u1Temp2 = prCfaRm->rAacCfgInfo.puHeader[5];

	u4SamplePerSec = ((u1Temp1 & (u8)0x07) << 1) | ((u1Temp2 >> 7) & (u8)0x01);
	u4Channels = (u1Temp2 >> 3) & (u8)0x0F;

	if (prCfaRm->rAudioInfo.rCfgInfo.eSoundType == CFA_RM_AUDIO_TYPE_MONO)
		u4Channels = (u32)1;
	else if (prCfaRm->rAudioInfo.rCfgInfo.eSoundType == CFA_RM_AUDIO_TYPE_STEREO)/*aac is stero*/
		u4Channels = (u32)2;
	else
	{
		/*do nothing*/
	}

	prCfaRm->rAudioInfo.pauAdtsHeader[0] = 0xFF;
	prCfaRm->rAudioInfo.pauAdtsHeader[1] = 0xF1;
	prCfaRm->rAudioInfo.pauAdtsHeader[2] =
		(u8)(((u32)1 << 6) | ((u4SamplePerSec << 2) & (u32)0x3C) | ((u4Channels >> 2) & (u32)0x1));
	prCfaRm->rAudioInfo.pauAdtsHeader[3] =
		(u8)(((u4Channels & (u32)0x3) << 6) | ((u4FrameSize >> 11) & (u32)0x3));
	prCfaRm->rAudioInfo.pauAdtsHeader[4] = (u8)(((u4FrameSize >> 3) & (u32)0xFF));
	prCfaRm->rAudioInfo.pauAdtsHeader[5] =
		(u8)(((u4FrameSize << 5) & (u32)0xE0) | (((u32)0x7FF >> 6) & (u32)0x1F));
	prCfaRm->rAudioInfo.pauAdtsHeader[6] = (u8)((((u32)0x7FF << 2) & (u32)0xFC));
}

static MRESULT CfaRmTxAudHdl2Fifo(void *pvSptHdl, CfaRmInst_T *prCfaRm)
{
	MRESULT mrResult = RET_DMX_OK;
	if (NULL == prCfaRm)
	{
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (((prCfaRm->rPacketInfo.ePacketType) == RM_STREAM_TYPE_AUDIO) &&
		((prCfaRm->rAudioInfo.rCfgInfo.eCodecID) == AVCODEC_ID_AAC)) {
		SetRmAacHdr(prCfaRm);
		prCfaRm->eCurAnaSt = CFA_RM_ANA_ST_TX_AUD;
		mrResult = Spt4CfaBuf2AFifo(pvSptHdl, prCfaRm->rAudioInfo.pauAdtsHeader,
			7, prCfaRm->rAudioInfo.rCfgInfo.u1StrmNum,
			prCfaRm->rAudioInfo.eAudType);

		if (mrResult != RET_DMX_OK)
		{
			CfaRmFinishPrs(pvSptHdl, prCfaRm);
			MM_RETURN(mrResult);
		}
		
	} else if (prCfaRm->rPacketInfo.ePacketType == RM_STREAM_TYPE_AUDIO)
		mrResult = CfaRmTxAudData2Fifo(pvSptHdl, prCfaRm);
	else
	{
		/*do nothing*/
	}

	MM_RETURN(mrResult);
}

static MRESULT CfaRmTxAudCfg2Fifo(void *pvSptHdl, CfaRmInst_T *prCfaRm)
{
	MRESULT mrResult = RET_DMX_OK;

	if (NULL == prCfaRm)
	{
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prCfaRm->eCurAnaSt = CFA_RM_ANA_ST_SEARCH_HEADER;

	if (((prCfaRm->rPacketInfo.ePacketType) == RM_STREAM_TYPE_AUDIO) &&
		(AVCODEC_ID_AAC == (prCfaRm->rAudioInfo.rCfgInfo.eCodecID))) {
		if (prCfaRm->fgFirstTxAud) {
			DmxLogD(DMX_MOD_CFA_RM,CFA_RM_LOG_TXDONE,
				TEXT("[CFA_RM_ST_CTRL] CfaRmTxAudCfg2Fifo:rAacCfgInfo.uHeaderLen=%d\r\n"),
				prCfaRm->rAacCfgInfo.uHeaderLen);
			prCfaRm->eCurAnaSt = CFA_RM_ANA_ST_TX_A_HDR;
			mrResult = Spt4CfaBuf2AFifo(pvSptHdl, prCfaRm->rAacCfgInfo.puHeader,
				prCfaRm->rAacCfgInfo.uHeaderLen,
				prCfaRm->rAudioInfo.rCfgInfo.u1StrmNum,
				prCfaRm->rAudioInfo.eAudType);
		} else {
			prCfaRm->eCurAnaSt = CFA_RM_ANA_ST_TX_AUD;
			mrResult = CfaRmTxAudHdl2Fifo(pvSptHdl, prCfaRm);
		}

		if (mrResult != RET_DMX_OK)
		{
			CfaRmFinishPrs(pvSptHdl, prCfaRm);
			MM_RETURN(mrResult);
		}
		
	} else if (prCfaRm->rPacketInfo.ePacketType == RM_STREAM_TYPE_AUDIO)
		mrResult = CfaRmTxAudData2Fifo(pvSptHdl, prCfaRm);
		else
		{
			/*do nothing*/
		}

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaRmTxVidData2Fifo(void *pvSptHdl, CfaRmInst_T *prCfaRm)
{
	CFA_VIDEO_INFO_T rCfaRmTxVidInfo = {0};
	MRESULT mrResult = RET_DMX_OK;
	u64 u8Sa = 0;
	u32 u4TxLen = 0;

	if (NULL == prCfaRm)
	{
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (CFA_VID_H264 == prCfaRm->eVidCodecType) {
		u8Sa = prCfaRm->rPacketInfo.u8Offset + 12;/* 4 is size len*/

		if (prCfaRm->rPacketInfo.u2Length > 0)
			u4TxLen = prCfaRm->rPacketInfo.u2Length - 12;
		else
			u4TxLen = 0;
	} else {
		u8Sa = prCfaRm->rPacketInfo.u8Offset + 12;/* 4 is size len*/

		if (prCfaRm->rPacketInfo.u2Length > 0)
			u4TxLen = prCfaRm->rPacketInfo.u2Length - 12;
		else
			u4TxLen = 0;
	}

	if (prCfaRm->fgFirstTxVid)
		prCfaRm->fgFirstTxVid = FALSE;

	/*transfer video data to video fifo.*/
	prCfaRm->eCurTxStrmType = CFA_RM_TX_STRM_TYPE_VID;

	rCfaRmTxVidInfo.u8FileOfst = u8Sa;
	rCfaRmTxVidInfo.eVidType = prCfaRm->eVidCodecType;
	rCfaRmTxVidInfo.eTxMode = CfaRmSetPicTxMode(prCfaRm);
	rCfaRmTxVidInfo.u4PrsStrmId = (u32)prCfaRm->rVideoInfo.rCfgInfo.u1StrmNum;
	rCfaRmTxVidInfo.u8Len = u4TxLen;

	CfaRmGetPicPts(prCfaRm);

	/*DMXLOG_DEBUG(
	TEXT("[CFA_RM_ST_CTRL] eTxMode=%d, TxVidSa= 0x%llx, Len= 0x%x, Time = %lld ms\r\n"),
	rCfaRmTxVidInfo.eTxMode,u8Sa, u4TxLen, (prCfaRm->u8PrsPts / CFA_RM_SYS_CLK));*/
	prCfaRm->u4Tx2VFifoLen += (u32)rCfaRmTxVidInfo.u8Len;

	if (prCfaRm->u4Tx2VFifoLen > prCfaRm->u4CurFrmTotalLen) {
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
			TEXT("[CFA_RM_ST_CTRL] can't create an AU,")
			TEXT("because u4Tx2VFifoLen is bigger than TotalLen\r\n"));
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM_ST_CTRL] finish and sent EOS\r\n"));
		CfaRmFinishPrs(pvSptHdl, prCfaRm);
		MM_RETURN(RET_DMX_OK);
	} else if (prCfaRm->u4Tx2VFifoLen == prCfaRm->u4CurFrmTotalLen) {
		DmxLogD(DMX_MOD_CFA_RM,CFA_RM_LOG_TXDONE,TEXT("[CFA_RM_ST_CTRL]--txdata---create an AU\r\n"));
	}
	else
	{
		/*do nothing*/
	}
	mrResult = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rCfaRmTxVidInfo);
	MM_RETURN(mrResult);
}

static MRESULT CfaRmPrsDmux(void *pvSptHdl, CfaRmInst_T *prCfaRm)
{
	MRESULT mrResult = RET_DMX_OK;

	if (NULL == prCfaRm)
	{
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT, TEXT("[CFA_RM_ST_CTRL] line %d,Invalid parameter!\r\n"),DMX_LINE_NO);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (RM_STREAM_TYPE_AUDIO == prCfaRm->rPacketInfo.ePacketType) {
		/*transfer audio data to fifo*/
		if (AVCODEC_ID_AAC == (prCfaRm->rAudioInfo.rCfgInfo.eCodecID))
			mrResult = CfaRmTxAudCfg2Fifo(pvSptHdl, prCfaRm);
		else
			mrResult = CfaRmTxAudData2Fifo(pvSptHdl, prCfaRm);
		if (mrResult != RET_DMX_OK)
		{
			if (DMX_IS_RW_PLAY(pvSptHdl))
			{
				Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, FALSE, (u32)GAU_E_ERRCHUNK);
			}
			else
			{
				Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, TRUE, GAU_E_ERRCHUNK);
			}
			MM_RETURN(mrResult);
		}
	} else if (RM_STREAM_TYPE_VIDEO == prCfaRm->rPacketInfo.ePacketType) {
		/*transfer video data to fifo*/
		mrResult = CfaRmTxVidData2Fifo(pvSptHdl, prCfaRm);

		if (mrResult != RET_DMX_OK)
		{
			if (DMX_IS_RW_PLAY(pvSptHdl))
			{
				Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, FALSE, (u32)GAU_E_ERRCHUNK);
			}
			else
			{
				Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, TRUE, GAU_E_ERRCHUNK);
			}
			MM_RETURN(mrResult);
		}
	} else if (RM_STREAM_TYPE_SUBPI == prCfaRm->rPacketInfo.ePacketType) /*Skip*/ {
		CfaRmSearchNextSc(pvSptHdl, prCfaRm, CFA_RM_ANA_ST_MEDIA_PACKET_HEADER, 0,
			RM_READ_FOR_PACKET_HEADER_SIZE, 0); /*RM_READ_FOR_CODEC_SIZE*/
	} else
	{
		CfaRmFinishPrs(pvSptHdl, prCfaRm);
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	
	MM_RETURN(RET_DMX_OK);
}

/* RM CFA processes CFA_RM_ANA_ST_IDLE
 @return None*/
static void CfaRmAnaStIdle(void)
{
	/*can not go here!*/
	return;
}


static void CfaRmSetPacketType(CfaRmInst_T *prCfaRm, u16 u2StreamNum)
{
	u32 u4StreamNum = 0;
	RM_STREAM_TYPE_INFO_T *prStreamTypeInfo = NULL;

	while (u4StreamNum < prCfaRm->rFileInfo.u4StreamNum) {
		prStreamTypeInfo = prCfaRm->rFileInfo.prStreamInfo + u4StreamNum;
		if (prStreamTypeInfo->u2StreamNum == u2StreamNum) {
			prCfaRm->rPacketInfo.ePacketType = prStreamTypeInfo->eStreamType;
			return;
		}
		u4StreamNum++;
	}

	prCfaRm->rPacketInfo.ePacketType = RM_STREAM_TYPE_UNKNOWN;
}

static void CfaRmTxVPartialFrame(void *pvSptHdl, CfaRmInst_T *prCfaRm,
								const CfaRmPartFramInfo_T *prFrameInfo)
{
	MRESULT mrResult = RET_DMX_OK;
	CFA_VIDEO_INFO_T rCfaRmTxVidInfo = {0};
	u32 u4TempTxVidLen = 0;

	if (NULL == prCfaRm)
	{
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT, TEXT("[CFA_RM_ST_CTRL] line %d,Invalid parameter!\r\n"),DMX_LINE_NO);
		return;
	}

	rCfaRmTxVidInfo.u8FileOfst = prFrameInfo->u4CurStartOffset;
	rCfaRmTxVidInfo.eVidType = prCfaRm->eVidCodecType;

	rCfaRmTxVidInfo.eTxMode = CfaRmSetPicTxMode(prCfaRm);
	rCfaRmTxVidInfo.u4PrsStrmId = (u32)prCfaRm->rVideoInfo.rCfgInfo.u1StrmNum;
	rCfaRmTxVidInfo.u8Len = prFrameInfo->u2PatialFramSize;

	if (1 == prFrameInfo->u1PacketNum) {
		rCfaRmTxVidInfo.fgUnitStart = TRUE;
		rCfaRmTxVidInfo.u8TotalAULen = (u64)prFrameInfo->u4EntireFramSize;
		prCfaRm->u4CurFrmTotalLen = prFrameInfo->u4EntireFramSize;
		prCfaRm->u4Tx2VFifoLen = 0;
		CfaRmGetPicPts(prCfaRm);
		if (CFA_PTM_RM_INTRAPIC == rCfaRmTxVidInfo.eTxMode)
			prCfaRm->rCurPosInfo.u8IFrmCurOfst = prCfaRm->rPacketInfo.u8Offset;

		prCfaRm->rSliceInf.u1TotalSliceNum = prFrameInfo->u1NumPackets;
	} else {
		rCfaRmTxVidInfo.fgUnitStart = FALSE;
		rCfaRmTxVidInfo.u8TotalAULen = 0;
	}

	if (prFrameInfo->u1PacketNum > 0) {
		prCfaRm->rSliceInf.rSliceInf[prFrameInfo->u1PacketNum - 1].u1SliceElemNum =
			prFrameInfo->u1PacketNum;
		prCfaRm->rSliceInf.rSliceInf[prFrameInfo->u1PacketNum - 1].u2SliceElemSize =
			prFrameInfo->u2PatialFramSize;
	} else {
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM_ST_CTRL] Packet Number is zero,need stop!\r\n"));
		CfaRmFinishPrs(pvSptHdl, prCfaRm);
		return;
	}

	CfaRmAdjustSliceInfo(prCfaRm);

	rCfaRmTxVidInfo.u2RmCurAuSliceNum = 0;
	if (prFrameInfo->u1PacketNum == prFrameInfo->u1NumPackets)
		rCfaRmTxVidInfo.u2RmCurAuSliceNum = prCfaRm->rSliceInf.u1TotalSliceNum;

	prCfaRm->eCurTxStrmType = CFA_RM_TX_STRM_TYPE_VID;

	prCfaRm->eCurAnaSt = CFA_RM_ANA_ST_SEARCH_HEADER;
	prCfaRm->u8Ca = prFrameInfo->u4CurStartOffset + prFrameInfo->u2PatialFramSize;

	/*DMXLOG_DEBUG(
	TEXT("[CFA_RM_ST_CTRL] eTxMode=%d, TxVidSa= 0x%x, TLen= 0x%x,")
	TEXT("Tpacket=0x%02x, Packet=0x%02x, Len= 0x%x, Time= %d ms\r\n"),
	rCfaRmTxVidInfo.eTxMode,prFrameInfo->u4CurStartOffset, prFrameInfo->u4EntireFramSize,
	prFrameInfo->u1NumPackets, prFrameInfo->u1PacketNum,
	prFrameInfo->u2PatialFramSize, prCfaRm->rPacketInfo.u4TimeStamp);*/

	if (prCfaRm->u8Endoffst < (rCfaRmTxVidInfo.u8FileOfst + rCfaRmTxVidInfo.u8Len)) {
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM_ST_CTRL] File maybe crash!!\r\n"));
		CfaRmFinishPrs(pvSptHdl, prCfaRm);
		return;
	}
	if (prCfaRm->u4Tx2VFifoLen < prCfaRm->u4CurFrmTotalLen) 
	{
		u4TempTxVidLen = prCfaRm->u4Tx2VFifoLen;
		prCfaRm->u4Tx2VFifoLen += prFrameInfo->u2PatialFramSize;
		if (prCfaRm->u4Tx2VFifoLen > prCfaRm->u4CurFrmTotalLen) {
			rCfaRmTxVidInfo.u8Len = prCfaRm->u4CurFrmTotalLen - u4TempTxVidLen;
			DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM_ST_CTRL] u2PatialFramSize = %d\r\n"),
				rCfaRmTxVidInfo.u8Len);
			mrResult = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rCfaRmTxVidInfo);
			return;
		}
	} 
	else if (prCfaRm->u4Tx2VFifoLen == prCfaRm->u4CurFrmTotalLen) 
	{
		DmxLogD(DMX_MOD_CFA_RM,CFA_RM_LOG_TXDONE,TEXT("[CFA_RM_ST_CTRL] ------------create an AU\r\n"));
	}
	else
	{
		/*do nothing*/
	}
	mrResult = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rCfaRmTxVidInfo);
	if (mrResult != RET_DMX_OK)
	{
		if (DMX_IS_RW_PLAY(pvSptHdl))
		{
			Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, FALSE, (u32)GAU_E_ERRCHUNK);
		}
		else
		{
			Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, TRUE, GAU_E_ERRCHUNK);
		}
		return;
	}
}


static void CfaRmTxVWholeFrame(void *pvSptHdl, CfaRmInst_T *prCfaRm,
								CfaRmPartFramInfo_T *prFrameInfo)
{
	MRESULT mrResult = RET_DMX_OK;
	CFA_VIDEO_INFO_T rCfaRmTxVidInfo = {0};

	if (NULL == prCfaRm)
	{
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT, TEXT("[CFA_RM_ST_CTRL] line %d,Invalid parameter!\r\n"),DMX_LINE_NO);
		return;
	}

	mm_memset(&rCfaRmTxVidInfo, 0, sizeof(CFA_VIDEO_INFO_T));

	prCfaRm->eCurTxStrmType = CFA_RM_TX_STRM_TYPE_VID;


	rCfaRmTxVidInfo.u8FileOfst = prFrameInfo->u4CurStartOffset;
	rCfaRmTxVidInfo.eVidType = prCfaRm->eVidCodecType;

	rCfaRmTxVidInfo.eTxMode = CfaRmSetPicTxMode(prCfaRm);
	rCfaRmTxVidInfo.u4PrsStrmId = (u32)prCfaRm->rVideoInfo.rCfgInfo.u1StrmNum;
	rCfaRmTxVidInfo.u8Len = prFrameInfo->u2PatialFramSize;
	rCfaRmTxVidInfo.u8TotalAULen = prFrameInfo->u2PatialFramSize;
	rCfaRmTxVidInfo.fgUnitStart = TRUE;

	prCfaRm->u4CurFrmTotalLen = prFrameInfo->u2PatialFramSize;
	prCfaRm->u4Tx2VFifoLen	= 0;

	prCfaRm->eCurAnaSt = CFA_RM_ANA_ST_SEARCH_HEADER;
	prCfaRm->u8Ca = prFrameInfo->u4CurStartOffset + prFrameInfo->u2PatialFramSize;

	prCfaRm->rSliceInf.rSliceInf[0].u1SliceElemNum = 1;
	prCfaRm->rSliceInf.rSliceInf[0].u2SliceElemSize = prFrameInfo->u2PatialFramSize;
	prCfaRm->rSliceInf.u1TotalSliceNum = 1;

	rCfaRmTxVidInfo.u2RmCurAuSliceNum = 1;
	DmxLogD(DMX_MOD_CFA_RM,CFA_RM_LOG_TXDONE,TEXT("[CFA_RM_ST_CTRL] Whole frm, au slice num = %d\r\n"),
		rCfaRmTxVidInfo.u2RmCurAuSliceNum);

	if (CFA_PTM_RM_INTRAPIC == rCfaRmTxVidInfo.eTxMode)
		prCfaRm->rCurPosInfo.u8IFrmCurOfst = prCfaRm->rPacketInfo.u8Offset;

	CfaRmGetPicPts(prCfaRm);
	CfaRmAdjustSliceInfo(prCfaRm);
	prCfaRm->u4Tx2VFifoLen += prFrameInfo->u2PatialFramSize;

	if (prCfaRm->u4Tx2VFifoLen > prCfaRm->u4CurFrmTotalLen) 
	{
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
			TEXT("[CFA_RM_ST_CTRL] wholeframe-can't create an AU,")
			TEXT("because u4Tx2VFifoLen is bigger than TotalLen\r\n"));
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM_ST_CTRL] wholeframe-finish and sent EOS\r\n"));
		CfaRmFinishPrs(pvSptHdl, prCfaRm);
		return;
	} 
	else if (prCfaRm->u4Tx2VFifoLen == prCfaRm->u4CurFrmTotalLen) 
	{
		DmxLogD(DMX_MOD_CFA_RM,CFA_RM_LOG_TXDONE,TEXT("[CFA_RM_ST_CTRL] ----wholeframe----create an AU\r\n"));
	}
	else
	{
		/*do nothing*/
	}
	mrResult = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rCfaRmTxVidInfo);
	if (mrResult != RET_DMX_OK)
	{
		if (DMX_IS_RW_PLAY(pvSptHdl))
		{
			Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, FALSE, (u32)GAU_E_ERRCHUNK);
		}
		else
		{
			Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, TRUE, GAU_E_ERRCHUNK);
		}
		return;
	}
}

static void CfaRmDataPacketHeader(void *pvSptHdl, CfaRmInst_T *prCfaRm, u64 u8TxLen)
{
	u16 u2ObjectVversion = 0;
	u32 u4ParsedLen = 0;
	u16 u2PacketHeaderBitSize = 0;
	u16 u2FrameNum = 0;
	u8 uFlag = 0;
	MRESULT mrResult = RET_DMX_OK;

	prCfaRm->u4TxLen = (u32)u8TxLen;

	while (prCfaRm->u4TxLen >= (u4ParsedLen + (u32)RM_READ_FOR_PACKET_HEADER_SIZE)) {
		/*need modify,RM_READ_FOR_CODEC_SIZE*/
		if (prCfaRm->u4StrmErrCnt >= CFA_STREAM_NUM_ERR_CNT) {
			DmxLogD(DMX_MOD_CFA_RM,CFA_RM_LOG_STATE,
				TEXT("[CFA_RM_ST_CTRL] The number of error stream is too much, so finish!\r\n"),
				u2ObjectVversion);
			prCfaRm->u4StrmErrCnt = 0;
			Sleep((u32)10);
		}

		LOADB_WORD(prCfaRm->pu1HdrBuf + u4ParsedLen, u2ObjectVversion);

		LOADB_WORD(prCfaRm->pu1HdrBuf + u4ParsedLen + 2, prCfaRm->rPacketInfo.u2Length);
		LOADB_WORD(prCfaRm->pu1HdrBuf + u4ParsedLen + 4, prCfaRm->rPacketInfo.u2StreamNum);
		LOADB_DWRD(prCfaRm->pu1HdrBuf + u4ParsedLen + 6, prCfaRm->rPacketInfo.u4TimeStamp);

		if (((0 != u2ObjectVversion) && ((u32)1 != u2ObjectVversion)) ||
			(0 == (prCfaRm->rPacketInfo.u2Length))) {
			DmxLogT(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
				TEXT("[CFA_RM_ST_CTRL] The data maybe invalid,u2ObjectVversion=%d,")
				TEXT("packet length=%d!,so skip or send eos\r\n"),
				u2ObjectVversion, prCfaRm->rPacketInfo.u2Length);

			if (((u64)CFA_RM_ENDFILE_OFFSET <= prCfaRm->u8Endoffst) &&
				 ((prCfaRm->u8Ca) < ((prCfaRm->u8Endoffst) - (u64)CFA_RM_ENDFILE_OFFSET))) {
				if (DMX_IS_RW_PLAY(pvSptHdl))
				{
					DmxLogT(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
						TEXT("[CFA_RM_ST_CTRL] Should not be errchunk when FR.\r\n"));
					Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, FALSE, (u32)GAU_E_ERRCHUNK);
				}
				else
				{
					Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, TRUE, (u32)GAU_E_ERRCHUNK);
				}
				return;
			}
			/*if the dummy frame is near to the end of file(1MB),
			then we consider that the file is finished*/
			CfaRmFinishPrs(pvSptHdl, prCfaRm);
			return;
		}

		prCfaRm->rPacketInfo.u8Offset = prCfaRm->u8Ca + u4ParsedLen;
		u4ParsedLen += (u32)10;
		prCfaRm->rPacketInfo.u4FramOffset = 10;

		if (0 == u2ObjectVversion) {
			LOAD_BYTE(prCfaRm->pu1HdrBuf + u4ParsedLen + 1, uFlag);
			u4ParsedLen += (u32)2;
			prCfaRm->rPacketInfo.u4FramOffset += (u32)2;
		} else {
			u4ParsedLen += (u32)3;
			prCfaRm->rPacketInfo.u4FramOffset += (u32)3;
		}

		CfaRmSetPacketType(prCfaRm, prCfaRm->rPacketInfo.u2StreamNum);

		if ((!prCfaRm->fgHasVideo) &&
			((u64)(prCfaRm->rPacketInfo.u4TimeStamp) < (prCfaRm->rRange.u8targetTime)) &&
			(((prCfaRm->rRange.u8targetTime) - (u64)(prCfaRm->rPacketInfo.u4TimeStamp)) > (u64)CFA_RM_TIME_THRESHOLD)) {
			DmxLogD(DMX_MOD_CFA_RM,CFA_RM_LOG_STATE,
				TEXT("[CFA_RM_ST_CTRL] skip:packet timeStamp[%dms] < target time[%lldms]\r\n"),
				prCfaRm->rPacketInfo.u4TimeStamp, prCfaRm->rRange.u8targetTime);

			prCfaRm->u8Ca += (u64)prCfaRm->rPacketInfo.u2Length;
			CfaRmSearchNextSc(pvSptHdl, prCfaRm, CFA_RM_ANA_ST_MEDIA_PACKET_HEADER, (u64)0,
				(u64)RM_READ_FOR_PACKET_HEADER_SIZE, (u32)0); /*RM_READ_FOR_CODEC_SIZE*/
			return;
		}

		if ((CfaRmToPlay(prCfaRm->u4CurPrsFlag, CFA_RM_PRS_BIT_STRM_TYPE_VID)) &&
			(RM_STREAM_TYPE_VIDEO == prCfaRm->rPacketInfo.ePacketType) &&
			(prCfaRm->rVideoInfo.rCfgInfo.u1StrmNum == prCfaRm->rPacketInfo.u2StreamNum)) {
			prCfaRm->u4StrmErrCnt = 0;
			if (prCfaRm->rRange.u8RealVidSa > prCfaRm->rPacketInfo.u8Offset) {
				prCfaRm->u8Ca += (u64)prCfaRm->rPacketInfo.u2Length;
				CfaRmSearchNextSc(pvSptHdl, prCfaRm, CFA_RM_ANA_ST_MEDIA_PACKET_HEADER, 0,
					RM_READ_FOR_PACKET_HEADER_SIZE, 0); /*RM_READ_FOR_CODEC_SIZE*/
				return;
			}
		} else if ((CfaRmToPlay(prCfaRm->u4CurPrsFlag, CFA_RM_PRS_BIT_STRM_TYPE_AUD)) &&
			(RM_STREAM_TYPE_AUDIO == prCfaRm->rPacketInfo.ePacketType) &&
			(prCfaRm->rAudioInfo.rCfgInfo.u1StrmNum == prCfaRm->rPacketInfo.u2StreamNum)) {
			prCfaRm->u4StrmErrCnt = 0;
			if (((prCfaRm->rRange.u8AudSa) > (prCfaRm->rPacketInfo.u8Offset)) ||
				(!(prCfaRm->rRange.fgEnableAud))) {
				prCfaRm->u8Ca += (u64)prCfaRm->rPacketInfo.u2Length;
				CfaRmSearchNextSc(pvSptHdl, prCfaRm, CFA_RM_ANA_ST_MEDIA_PACKET_HEADER, 0,
					RM_READ_FOR_PACKET_HEADER_SIZE, 0); /*RM_READ_FOR_CODEC_SIZE*/
				return;
			}
		} else {
			prCfaRm->u4StrmErrCnt++;
			prCfaRm->u8Ca += prCfaRm->rPacketInfo.u2Length;
			CfaRmSearchNextSc(pvSptHdl, prCfaRm, CFA_RM_ANA_ST_MEDIA_PACKET_HEADER, 0,
				RM_READ_FOR_PACKET_HEADER_SIZE, 0); /*RM_READ_FOR_CODEC_SIZE*/
			return;
		}

		if ((RM_STREAM_TYPE_AUDIO == (prCfaRm->rPacketInfo.ePacketType)) &&
			(AVCODEC_ID_AAC == (prCfaRm->rAudioInfo.rCfgInfo.eCodecID))) {
			LOADB_WORD(prCfaRm->pu1HdrBuf + u4ParsedLen, u2PacketHeaderBitSize);

			prCfaRm->rPacketInfo.u4FramOffset += (u32)2;
			if (u2PacketHeaderBitSize/(u16)16 > (u16)128) {
				DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
					TEXT("[CFA_RM_ST_CTRL] The number of frames in the packet(%d)")
					TEXT("is larger than 128,so skip or send eos\r\n"),
					u2PacketHeaderBitSize/16);
				if (((u64)CFA_RM_ENDFILE_OFFSET <= (prCfaRm->u8Endoffst)) &&
					 ((prCfaRm->u8Ca) < ((prCfaRm->u8Endoffst) - (u64)CFA_RM_ENDFILE_OFFSET))) {
					if (DMX_IS_RW_PLAY(pvSptHdl))
					{
						Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, FALSE, (u32)GAU_E_ERRCHUNK);
					}
					else
					{
						Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, TRUE, (u32)GAU_E_ERRCHUNK);
					}
					return;
				}
				{
					CfaRmFinishPrs(pvSptHdl, prCfaRm);
					return;
				}
			}
			for (u2FrameNum = 0; u2FrameNum < (u2PacketHeaderBitSize / 16); u2FrameNum++) {
				u4ParsedLen += (u32)2;
				prCfaRm->rPacketInfo.u4FramOffset += (u32)2;
				LOADB_WORD(prCfaRm->pu1HdrBuf + u4ParsedLen,
							prCfaRm->rPacketInfo.arFramLen[u2FrameNum]);
			}
			prCfaRm->rPacketInfo.u2FramNum = u2FrameNum;
			prCfaRm->rPacketInfo.u2CurFramNum = 0;
		} else if (RM_STREAM_TYPE_AUDIO == prCfaRm->rPacketInfo.ePacketType) {
			/*cook,sipt,atrc need deinterleave */
			dmx_memset(&prCfaRm->rAudioPacket, 0, sizeof(RM_Audio_Packet_T));

			/*mtk40504, fix CNB4979*/
			/*mtk10132, because bug CNB00005347, remove recover method.*/
			if (prCfaRm->rPacketInfo.u2Length <= prCfaRm->rPacketInfo.u4FramOffset) {
				DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
					TEXT("[CFA_RM_ST_CTRL] call CfaRmFinishPrs when ")
					TEXT("u2Length<rPacketInfo.u4FramOffset !\r\n"));
				CfaRmFinishPrs(pvSptHdl, prCfaRm);
				return;
			}
			prCfaRm->rAudioPacket.u2PacketSize =
				(u16)(prCfaRm->rPacketInfo.u2Length - prCfaRm->rPacketInfo.u4FramOffset);
			prCfaRm->rAudioPacket.u4PacketOffset =
				(u32)prCfaRm->rPacketInfo.u8Offset + prCfaRm->rPacketInfo.u4FramOffset;
			prCfaRm->u8Ca = prCfaRm->rAudioPacket.u4PacketOffset;
			prCfaRm->u4PacketSum++;

			if ((0 == u2ObjectVversion) && (uFlag & 0x02)) {
				prCfaRm->u1FrameIdxInPacket = 0;
				prCfaRm->u4PacketSum = 1;
				DmxLogD(DMX_MOD_CFA_RM,CFA_RM_LOG_STATE,TEXT("[CFA_RM_ST_CTRL] ---SuperBlock start!---\r\n"));
			}

			if (prCfaRm->u4PacketSum == 1) {
				prCfaRm->u8AudPts = prCfaRm->rPacketInfo.u4TimeStamp;
				prCfaRm->u8AudPts *= CFA_RM_SYS_CLK;
			}

		#if CFA_RM_DEINTERLEAVE_AUDIO
			/*DMXLOG_DEBUG(
			(TEXT("[CFA_RM_ST_CTRL] Audio,PacketOffset=0x%llx, DataOffset=0x%llx,")
			TEXT("DataLen=%d, Time=%d ms!\r\n")),
			prCfaRm->rPacketInfo.u8Offset, prCfaRm->u8Ca,
			prCfaRm->rAudioPacket.u2PacketSize, prCfaRm->rPacketInfo.u4TimeStamp);*/
			CfaRmSearchNextSc(pvSptHdl, prCfaRm, CFA_RM_ANA_ST_CBR_AUDIO_DATA, 0,
				prCfaRm->rAudioPacket.u2PacketSize, 0);
			return;
		#else
			prCfaRm->u8Ca = (u32)prCfaRm->rPacketInfo.u8Offset;
		#endif
		} else if (RM_STREAM_TYPE_VIDEO == prCfaRm->rPacketInfo.ePacketType) {
			CfaRmPartFramInfo_T rPatialFrameInfo = {0};

			LOADB_WORD(prCfaRm->pu1HdrBuf + u4ParsedLen, u2PacketHeaderBitSize);/*for get packet type*/

			rPatialFrameInfo.u1FramType = (u8)(u2PacketHeaderBitSize >> 14) & 0x03;
			u4ParsedLen += 2;
			prCfaRm->rPacketInfo.u4FramOffset += 2;

			switch (rPatialFrameInfo.u1FramType) {
			case 0:
			case 2: {
				rPatialFrameInfo.u1NumPackets = (u8)(u2PacketHeaderBitSize >> 7) & 0x7F;
				rPatialFrameInfo.u1PacketNum = (u8)(u2PacketHeaderBitSize) & 0x7F;

				LOADB_WORD(prCfaRm->pu1HdrBuf + u4ParsedLen, u2PacketHeaderBitSize);
				prCfaRm->rPacketInfo.u4FramOffset += 2;

				rPatialFrameInfo.u2BrokenUpByUs = (u2PacketHeaderBitSize & 0x8000);

				if (0x4000 == (u2PacketHeaderBitSize & 0x4000)) { /*entire_frame_size_flag */
					rPatialFrameInfo.u4EntireFramSize = u2PacketHeaderBitSize & 0x3FFF;
					u4ParsedLen += 2;
				} else {
					LOADB_DWRD(prCfaRm->pu1HdrBuf + u4ParsedLen, rPatialFrameInfo.u4EntireFramSize);
					rPatialFrameInfo.u4EntireFramSize &= 0x3FFFFFFF;
					u4ParsedLen += 4;
					prCfaRm->rPacketInfo.u4FramOffset += 2;
				}

				LOADB_WORD(prCfaRm->pu1HdrBuf + u4ParsedLen, u2PacketHeaderBitSize);
				prCfaRm->rPacketInfo.u4FramOffset += 2;

				if (0x4000 == (u2PacketHeaderBitSize & 0x4000)) {
					/*partial_frame_offset_flag or last partial_frame_size */
					rPatialFrameInfo.u4PartFrameOffset = u2PacketHeaderBitSize & 0x3FFF;
					u4ParsedLen += 2;
				} else {
					LOADB_DWRD(prCfaRm->pu1HdrBuf + u4ParsedLen,
								rPatialFrameInfo.u4PartFrameOffset);
					rPatialFrameInfo.u4PartFrameOffset &= 0x3FFFFFFF;
					u4ParsedLen += 4;
					prCfaRm->rPacketInfo.u4FramOffset += 2;
				}

				LOAD_BYTE(prCfaRm->pu1HdrBuf + u4ParsedLen, rPatialFrameInfo.u1SequenceNum);
				u4ParsedLen += 1;
				if (1 == rPatialFrameInfo.u1PacketNum) {
					LOAD_BYTE(prCfaRm->pu1HdrBuf + u4ParsedLen, prCfaRm->u1FirstBytePay);
					LOADB_DWRD(prCfaRm->pu1HdrBuf + u4ParsedLen, prCfaRm->u4First4BytesPay);
				}
				LOAD_BYTE(prCfaRm->pu1HdrBuf + u4ParsedLen, prCfaRm->u1SliceFirstByte);
				prCfaRm->rPacketInfo.u4FramOffset += 1;
				rPatialFrameInfo.u4CurStartOffset =
					(u32)prCfaRm->rPacketInfo.u8Offset + prCfaRm->rPacketInfo.u4FramOffset;
				rPatialFrameInfo.u2PatialFramSize =
					prCfaRm->rPacketInfo.u2Length - (u16)(prCfaRm->rPacketInfo.u4FramOffset);

				if (0 == rPatialFrameInfo.u1FramType) /* Partial Frame */ {
					prCfaRm->u8Ca += u4ParsedLen;
					dmx_memcpy(&prCfaRm->rFrameData.rFrameInfo, &rPatialFrameInfo,
								sizeof(CfaRmPartFramInfo_T));
					CfaRmTxVPartialFrame(pvSptHdl, prCfaRm,  &rPatialFrameInfo);
					return;
				}
				/* Last Partial Frame */
				{
					/*save last patial frame data,first, need read data,maybe contain multi frame*/
					prCfaRm->rFrameData.u4FrameSize = rPatialFrameInfo.u4EntireFramSize;
					prCfaRm->rFrameData.u1NumPackets = rPatialFrameInfo.u1NumPackets;
					dmx_memcpy(&prCfaRm->rFrameData.rFrameInfo, &rPatialFrameInfo,
								sizeof(CfaRmPartFramInfo_T));
					if ((rPatialFrameInfo.u4PartFrameOffset + rPatialFrameInfo.u2PatialFramSize) <
						CFA_RM_VIDEO_BUFFER_SIZE) {
						prCfaRm->u8Ca = rPatialFrameInfo.u4CurStartOffset;
						CfaRmSearchNextSc(pvSptHdl, prCfaRm, CFA_RM_ANA_ST_LAST_PATIAL_FRAME,
							0, rPatialFrameInfo.u2PatialFramSize, 0);
						return;
					}
				}

				break;
			}

			case 1: { /* Whole Frame */
				LOAD_BYTE(prCfaRm->pu1HdrBuf + u4ParsedLen, prCfaRm->u1FirstBytePay);
				LOADB_DWRD(prCfaRm->pu1HdrBuf + u4ParsedLen, prCfaRm->u4First4BytesPay);
				prCfaRm->u1SliceFirstByte = prCfaRm->u1FirstBytePay;
				rPatialFrameInfo.u1SequenceNum = (u8)u2PacketHeaderBitSize;/*8 bits*/
				rPatialFrameInfo.u4CurStartOffset =
										(u32)prCfaRm->rPacketInfo.u8Offset +
										prCfaRm->rPacketInfo.u4FramOffset;
				rPatialFrameInfo.u2PatialFramSize =
									prCfaRm->rPacketInfo.u2Length -
									(u16)(prCfaRm->rPacketInfo.u4FramOffset);
				prCfaRm->u8Ca += u4ParsedLen;
				CfaRmTxVWholeFrame(pvSptHdl, prCfaRm, &rPatialFrameInfo);
				return;
			}

			default: { /*3 is Multiple Frames */
				u4ParsedLen -= 2;
				prCfaRm->rPacketInfo.u4FramOffset -= 2;
				prCfaRm->u8Ca += u4ParsedLen;
				prCfaRm->rMultipleFrame.u4ParseredSize = 0;
				prCfaRm->rMultipleFrame.u4PacketOffset =
					(u32)prCfaRm->rPacketInfo.u8Offset + prCfaRm->rPacketInfo.u4FramOffset;
				prCfaRm->rMultipleFrame.u4PacketSize =
					prCfaRm->rPacketInfo.u2Length - (u16)(prCfaRm->rPacketInfo.u4FramOffset);
				CfaRmSearchNextSc(pvSptHdl, prCfaRm, CFA_RM_ANA_ST_MULTIPLE_FRAME, 0,
									prCfaRm->rMultipleFrame.u4PacketSize, 0);
				return;
			}
		}
	}
	else
	{
		/*do nothing*/
	}
	prCfaRm->u8Ca += prCfaRm->rPacketInfo.u2Length;
	prCfaRm->eCurAnaSt = CFA_RM_ANA_ST_SEARCH_HEADER;
	mrResult = CfaRmPrsDmux(pvSptHdl, prCfaRm);
	if (RET_DMX_OK != mrResult)
	{
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT, TEXT("[CFA_RM_ST_CTRL] line %d, call CfaRmPrsDmux error!\r\n"),DMX_LINE_NO);
		return;
	}
  }
}

void CfaRmLastPatialFrame(void *pvSptHdl, CfaRmInst_T *prCfaRm)
{
	MRESULT mrResult = RET_DMX_OK;
	u32 u4LastPartialFrameSize = 0;
	CfaRmPartFramInfo_T *prFrameInfo = {0};
	CFA_VIDEO_INFO_T rCfaRmTxVidInfo = {0};
	u32 u4TempTxVidLen = 0;

	prFrameInfo = &prCfaRm->rFrameData.rFrameInfo;
	u4LastPartialFrameSize = prFrameInfo->u4PartFrameOffset;
	prFrameInfo->u4PartFrameOffset = prFrameInfo->u4EntireFramSize - prFrameInfo->u4PartFrameOffset;

	prCfaRm->u8Ca += u4LastPartialFrameSize;
	if (prFrameInfo->u1PacketNum == prCfaRm->rFrameData.u1NumPackets) {
		/*need use sequence_number? need not judge ? */
		/*transfer data*/
		prCfaRm->eCurTxStrmType = CFA_RM_TX_STRM_TYPE_VID;

		rCfaRmTxVidInfo.u8FileOfst = 0;
		rCfaRmTxVidInfo.eVidType = prCfaRm->eVidCodecType;

		rCfaRmTxVidInfo.eTxMode = CfaRmSetPicTxMode(prCfaRm);
		rCfaRmTxVidInfo.u4PrsStrmId = (u32)prCfaRm->rVideoInfo.rCfgInfo.u1StrmNum;
		rCfaRmTxVidInfo.u8Len = prCfaRm->rFrameData.u4FrameSize;
		rCfaRmTxVidInfo.u8TotalAULen = prFrameInfo->u4EntireFramSize;

		if (u4LastPartialFrameSize < prFrameInfo->u2PatialFramSize) {
			prCfaRm->eCurAnaSt = CFA_RM_ANA_ST_MULTIPLE_FRAME;
			prCfaRm->rMultipleFrame.u4ParseredSize = u4LastPartialFrameSize;
			prCfaRm->rMultipleFrame.u4PacketOffset = (u32)prCfaRm->u8Ca - u4LastPartialFrameSize;
			prCfaRm->rMultipleFrame.u4PacketSize = prFrameInfo->u2PatialFramSize;
		} else
			prCfaRm->eCurAnaSt = CFA_RM_ANA_ST_SEARCH_HEADER;

		rCfaRmTxVidInfo.u8Len = u4LastPartialFrameSize;/*prFrameInfo->u2PatialFramSize;*/
		if (prFrameInfo->u1PacketNum > 0) {
			if ((prFrameInfo->u1PacketNum - 1) >= RM_VID_SLICE_MAX_NUM)
			{
				DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM_ST_CTRL] line %d,u1PacketNum error!"),DMX_LINE_NO);
				CfaRmFinishPrs(pvSptHdl, prCfaRm);
				return;
			}
			prCfaRm->rSliceInf.rSliceInf[prFrameInfo->u1PacketNum - 1].u1SliceElemNum =
				prFrameInfo->u1PacketNum;
			prCfaRm->rSliceInf.rSliceInf[prFrameInfo->u1PacketNum - 1].u2SliceElemSize =
				(u16)rCfaRmTxVidInfo.u8Len;
			if (prFrameInfo->u1PacketNum == 1)
				prCfaRm->rSliceInf.u1TotalSliceNum = prFrameInfo->u1NumPackets;
		} else {
			DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM_ST_CTRL] Packet Number is zero,need stop!\r\n"));
			CfaRmFinishPrs(pvSptHdl, prCfaRm);
			return;
		}
		CfaRmAdjustSliceInfo(prCfaRm);
		/*DMXLOG_DEBUG(
		TEXT("[CFA_RM_ST_CTRL]	eTxMode=%d, TxVidSa= 0x%x, Len= 0x%llx, Time = %d ms\r\n"),
		rCfaRmTxVidInfo.eTxMode, prFrameInfo->u4CurStartOffset,
		rCfaRmTxVidInfo.u8Len, prCfaRm->rPacketInfo.u4TimeStamp);*/

		rCfaRmTxVidInfo.fgUnitStart = FALSE;
		rCfaRmTxVidInfo.u8TotalAULen = 0;
		rCfaRmTxVidInfo.u8FileOfst = prFrameInfo->u4CurStartOffset;

		if (prCfaRm->u8Endoffst < (rCfaRmTxVidInfo.u8FileOfst + rCfaRmTxVidInfo.u8Len)) {
			DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
				TEXT("[CFA_RM_ST_CTRL] line %d send EOS,u8Endoffst[0x%llx],")
				TEXT("u8FileOfst[0x%llx],u8Len[0x%llx]\r\n"),
				DMX_LINE_NO, prCfaRm->u8Endoffst, rCfaRmTxVidInfo.u8FileOfst, rCfaRmTxVidInfo.u8Len);
			CfaRmFinishPrs(pvSptHdl, prCfaRm);
			return;
		}

		rCfaRmTxVidInfo.u2RmCurAuSliceNum = prCfaRm->rSliceInf.u1TotalSliceNum;
		if (prCfaRm->u4Tx2VFifoLen < prCfaRm->u4CurFrmTotalLen) 
		{
			u4TempTxVidLen = prCfaRm->u4Tx2VFifoLen;
			prCfaRm->u4Tx2VFifoLen += (u32)rCfaRmTxVidInfo.u8Len;
			if (prCfaRm->u4Tx2VFifoLen > prCfaRm->u4CurFrmTotalLen)
			{
				rCfaRmTxVidInfo.u8Len = prCfaRm->u4CurFrmTotalLen - u4TempTxVidLen;
				DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM_ST_CTRL] u8Len = %d\r\n"),
					rCfaRmTxVidInfo.u8Len);
				mrResult = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rCfaRmTxVidInfo);
				return;
			}
		} 
		else if (prCfaRm->u4Tx2VFifoLen == prCfaRm->u4CurFrmTotalLen) 
		{
			DmxLogD(DMX_MOD_CFA_RM,CFA_RM_LOG_STATE,TEXT("[CFA_RM_ST_CTRL] create an AU\r\n"));
		}
		else
		{
			/*do nothing*/
		}
		mrResult = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rCfaRmTxVidInfo);

		if (mrResult != RET_DMX_OK)
		{
			DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT, TEXT("[CFA_RM_ST_CTRL] line %d, call Spt4CfaPbb2VFifoAUCtrl() failed!\r\n"),DMX_LINE_NO);
			CfaRmFinishPrs(pvSptHdl, prCfaRm);
			return;
		}
	}
	else
	{
		prCfaRm->eCurAnaSt = CFA_RM_ANA_ST_SEARCH_HEADER;
		CfaRmSearchNextSc(pvSptHdl, prCfaRm, CFA_RM_ANA_ST_MEDIA_PACKET_HEADER, 0,
							RM_READ_FOR_PACKET_HEADER_SIZE, 0);
		/*RM_READ_FOR_CODEC_SIZE*/
	}
}


void CfaRmMultiFrame(void *pvSptHdl, CfaRmInst_T *prCfaRm)
{
	u16 u2Temp = 0;
	MRESULT mrResult = RET_DMX_OK;
	u32 u4ParseredSize = 0;
	RM_V_Frame_T rVFrame = {0};
	CFA_VIDEO_INFO_T rCfaRmTxVidInfo = {0};

	u4ParseredSize = prCfaRm->rMultipleFrame.u4ParseredSize;
	prCfaRm->rPacketInfo.u4FramOffset += 1;  /*skip multiple_frame_type*/
	u4ParseredSize += 1;  /*skip multiple_frame_type*/

	LOADB_WORD(prCfaRm->pu1HdrBuf + u4ParseredSize, u2Temp);
	prCfaRm->rPacketInfo.u4FramOffset += 2;

	if (0x4000 == (u2Temp & 0x4000)) {/*frame_size_flag */
		rVFrame.u4FrameSize = u2Temp & 0x3FFF;
		u4ParseredSize += 2;
	} else {
		LOADB_DWRD(prCfaRm->pu1HdrBuf + u4ParseredSize, rVFrame.u4FrameSize);
		rVFrame.u4FrameSize &= 0x3FFFFFFF;
		u4ParseredSize += 4;
		prCfaRm->rPacketInfo.u4FramOffset += 2;
	}

	LOADB_WORD(prCfaRm->pu1HdrBuf + u4ParseredSize, u2Temp);
	prCfaRm->rPacketInfo.u4FramOffset += 2;

	if (0x4000 == (u2Temp & 0x4000)) { /*timestamp_flag */
		rVFrame.u4TimeStamp = u2Temp & 0x3FFF;
		u4ParseredSize += 2;
	} else {
		LOADB_DWRD(prCfaRm->pu1HdrBuf + u4ParseredSize, rVFrame.u4TimeStamp);
		rVFrame.u4TimeStamp &= 0x3FFFFFFF;
		u4ParseredSize += 4;
		prCfaRm->rPacketInfo.u4FramOffset += 2;
	}

	LOAD_BYTE(prCfaRm->pu1HdrBuf + u4ParseredSize, rVFrame.u1SequenceNum);
	u4ParseredSize += 1;

	LOAD_BYTE(prCfaRm->pu1HdrBuf + u4ParseredSize, prCfaRm->u1FirstBytePay);
	LOADB_DWRD(prCfaRm->pu1HdrBuf + u4ParseredSize, prCfaRm->u4First4BytesPay);
	prCfaRm->u1SliceFirstByte = prCfaRm->u1FirstBytePay;

	prCfaRm->rPacketInfo.u4TimeStamp = rVFrame.u4TimeStamp;

	dmx_memcpy(&prCfaRm->rMultipleFrame.rFrame, &rVFrame, sizeof(RM_V_Frame_T));

	prCfaRm->rMultipleFrame.u4ParseredSize = u4ParseredSize;

	rCfaRmTxVidInfo.u8FileOfst = prCfaRm->rMultipleFrame.u4PacketOffset + u4ParseredSize;
	rCfaRmTxVidInfo.eVidType = prCfaRm->eVidCodecType;

	rCfaRmTxVidInfo.eTxMode = CfaRmSetPicTxMode(prCfaRm);
	rCfaRmTxVidInfo.u4PrsStrmId = (u32)prCfaRm->rVideoInfo.rCfgInfo.u1StrmNum;
	rCfaRmTxVidInfo.u8Len = rVFrame.u4FrameSize;
	rCfaRmTxVidInfo.u8TotalAULen = rVFrame.u4FrameSize;
	rCfaRmTxVidInfo.fgUnitStart = TRUE;
	prCfaRm->u4CurFrmTotalLen = rVFrame.u4FrameSize;
	prCfaRm->u4Tx2VFifoLen	= 0;

	prCfaRm->eCurTxStrmType = CFA_RM_TX_STRM_TYPE_VID;
	prCfaRm->u8Ca = prCfaRm->rMultipleFrame.u4PacketOffset + u4ParseredSize;
	if ((rVFrame.u4FrameSize + u4ParseredSize) >= prCfaRm->rMultipleFrame.u4PacketSize)
		prCfaRm->eCurAnaSt = CFA_RM_ANA_ST_SEARCH_HEADER;


	if (CFA_PTM_RM_INTRAPIC == rCfaRmTxVidInfo.eTxMode)
		prCfaRm->rCurPosInfo.u8IFrmCurOfst = prCfaRm->rPacketInfo.u8Offset;

	prCfaRm->rSliceInf.rSliceInf[0].u1SliceElemNum = 1;
	prCfaRm->rSliceInf.rSliceInf[0].u2SliceElemSize = (u16)rCfaRmTxVidInfo.u8Len;
	prCfaRm->rSliceInf.u1TotalSliceNum = 1;

	rCfaRmTxVidInfo.u2RmCurAuSliceNum = 1;

	CfaRmGetPicPts(prCfaRm);
	CfaRmAdjustSliceInfo(prCfaRm);
	prCfaRm->u8Ca += rVFrame.u4FrameSize;
	prCfaRm->rMultipleFrame.u4ParseredSize += rVFrame.u4FrameSize;

	if (prCfaRm->u8Endoffst < (rCfaRmTxVidInfo.u8FileOfst + rCfaRmTxVidInfo.u8Len)) {
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
			TEXT("[CFA_RM_ST_CTRL] line %d send EOS,u8Endoffst[0x%llx],")
			TEXT("u8FileOfst[0x%llx],u8Len[0x%llx]\r\n"),
			DMX_LINE_NO, prCfaRm->u8Endoffst, rCfaRmTxVidInfo.u8FileOfst, rCfaRmTxVidInfo.u8Len);
		CfaRmFinishPrs(pvSptHdl, prCfaRm);
		return;
	}
	prCfaRm->u4Tx2VFifoLen += (u32)rCfaRmTxVidInfo.u8Len;

	if (prCfaRm->u4Tx2VFifoLen > prCfaRm->u4CurFrmTotalLen) {
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
			TEXT("[CFA_RM_ST_CTRL] multiframe-can't create an AU,")
			TEXT("because u4Tx2VFifoLen is bigger than TotalLen\r\n"));
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM_ST_CTRL] multiframe-finish and sent EOS\r\n"));
		CfaRmFinishPrs(pvSptHdl, prCfaRm);
		return;
	} else if (prCfaRm->u4Tx2VFifoLen == prCfaRm->u4CurFrmTotalLen) {
		DmxLogD(DMX_MOD_CFA_RM,CFA_RM_LOG_STATE,TEXT("[CFA_RM_ST_CTRL] multiframe-------create an AU\r\n"));
	}
		else
		{
			/*do nothing*/
		}
	mrResult = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rCfaRmTxVidInfo);

	if (mrResult != RET_DMX_OK)
	{
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT, TEXT("[CFA_RM_ST_CTRL] line %d, call Spt4CfaPbb2VFifoAUCtrl() failed!\r\n"),DMX_LINE_NO);
		if (DMX_IS_RW_PLAY(pvSptHdl))
		{
			Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, FALSE, (u32)GAU_E_ERRCHUNK);
		}
		else
		{
			Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, TRUE, GAU_E_ERRCHUNK);
		}
		return;
	}
}


#if CFA_RM_COOK_XOR
void CfaRmCookBufCopy(const CfaRmInst_T *prCfaRm, u8 *pu_to, const u8 *pu_from,
							u32 u4Len, u32 u4LastFrameOfst) {
	u32 u4CpyLen = 0;
	u32 u4CurFrameOfst = u4LastFrameOfst;

	for (u4CpyLen = 0; u4CpyLen < u4Len; u4CpyLen++) {
		switch (u4CurFrameOfst & (u32)0x3) {
		case 0:
			*(pu_to + u4CpyLen) = *(pu_from + u4CpyLen) ^ (u32)0x37;
			break;

		case (u32)1:
			*(pu_to + u4CpyLen) = *(pu_from + u4CpyLen) ^ (u32)0xC5;
			break;

		case (u32)2:
			*(pu_to + u4CpyLen) = *(pu_from + u4CpyLen) ^ (u32)0x11;
			break;

		case (u32)3:
			*(pu_to + u4CpyLen) = *(pu_from + u4CpyLen) ^ (u32)0xF2;
			break;

		default:
			break;
		}

		u4CurFrameOfst++;

		if ((CFA_RM_AUDIO_TYPE_STEREO == prCfaRm->rAudioInfo.rCfgInfo.eSoundType) &&
			(0 == prCfaRm->rAudioInfo.rCfgInfo.u2CplQBits)) {
			if ((u16)u4CurFrameOfst == (prCfaRm->rAudioInfo.rCfgInfo.u2FrameSize / 2))
				u4CurFrameOfst = 0;
		} else {
			if (u4CurFrameOfst == prCfaRm->rAudioInfo.rCfgInfo.u2FrameSize)
				u4CurFrameOfst = 0;
		}
	}
}

void CfaRmCookBufCopy2(CfaRmInst_T *prCfaRm, u8 *pu_to,
								const u8 *pu_from, u32 u4Len)
{
	u32 u4CpyLen = 0;
	u32 u4Temp = (u32)pu_from;

	if (u4Temp % (u32)4) {
		CfaRmCookBufCopy(prCfaRm, pu_to, pu_from, u4Len, 0);
	} else {
		for (u4CpyLen = 0; u4CpyLen < u4Len/(u32)4; u4CpyLen++) {
			*((u32 *)((u32 *)pu_to + u4CpyLen)) =
				*((u32 *)((u32 *)pu_from + u4CpyLen)) ^ 0xF211C537;
		}
	}
}

bool CfaRmDWORDAligned(const CfaRmInst_T *prCfaRm)
{
	if ((prCfaRm->rAudioInfo.rCfgInfo.u2FrameSize & (u16)0x3))
		return FALSE;

	return TRUE;
}
#endif

#if CFA_RM_DEINTERLEAVE_AUDIO
void CfaRmDeInterleaveAudio(void *pvSptHdl, CfaRmInst_T *prCfaRm)
{
	MRESULT mrResult = RET_DMX_OK;
	u16 u2Loop = 0;
	u16 u2FrameSize = 0;
	u16 u2FrameNumPerSupPacket = 0;
	u16 u2OffsetInHdrBuf = 0;
	u32 u4OffsetInAudBuf = 0;
	#if CFA_RM_COOK_XOR
	bool fgIsDWORDAligned = FALSE;
	#endif
	CFA_AUDIO_INFO_T rTxAudInfo = {0};
	RM_Audio_Packet_T *prAPacketInfo = &prCfaRm->rAudioPacket;

	u2FrameSize = prCfaRm->rAudioInfo.rCfgInfo.u2FrameSize;

	if (0 == u2FrameSize)
	{
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT, TEXT("[CFA_RM_ST_CTRL] line %d, u2FrameSize is zero!\r\n"),DMX_LINE_NO);

		return;
	}
	u2FrameNumPerSupPacket = (u16)(prCfaRm->u4SuperBlockSize / u2FrameSize);

	if (u2FrameNumPerSupPacket > RM_MAX_FRAME_NUM_IN_SUP_BLOCK) {
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
			TEXT("[CFA_RM_ST_CTRL] WARNING:CfaRmDeInterleaveAudio::")
			TEXT("u2FrameNumPerSupPacket[%d] is larger than RM_MAX_FRAME_NUM_IN_SUP_BLOCK\r\n"),
			u2FrameNumPerSupPacket);
		u2FrameNumPerSupPacket = RM_MAX_FRAME_NUM_IN_SUP_BLOCK;
	}

	prCfaRm->u8Ca += prAPacketInfo->u2PacketSize;
	prCfaRm->eCurAnaSt = CFA_RM_ANA_ST_SEARCH_HEADER;

	while (prAPacketInfo->u2ParseredSize < prAPacketInfo->u2PacketSize) { /*save to buf */
		for (u2Loop = 0; u2Loop < u2FrameNumPerSupPacket; u2Loop++) {
			if (prCfaRm->u1FrameIdxInPacket == prCfaRm->u2RmPattern[u2Loop])
				break;
		}
		if (u2Loop >= u2FrameNumPerSupPacket) {
			DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
				TEXT("[CFA_RM_ST_CTRL] u2Loop is larger than frame num per super packet\r\n"));
		}

		u2OffsetInHdrBuf = prAPacketInfo->u1TxFrameIdx * u2FrameSize;
		u4OffsetInAudBuf = u2Loop * u2FrameSize;

		#if CFA_RM_COOK_XOR
		fgIsDWORDAligned = CfaRmDWORDAligned(prCfaRm);
		if (fgIsDWORDAligned) {
			CfaRmCookBufCopy2(prCfaRm, prCfaRm->puAudioBuf + u4OffsetInAudBuf,
							prCfaRm->pu1HdrBuf + u2OffsetInHdrBuf, u2FrameSize);
		} else {
			CfaRmCookBufCopy(prCfaRm, prCfaRm->puAudioBuf + u4OffsetInAudBuf,
							prCfaRm->pu1HdrBuf + u2OffsetInHdrBuf, u2FrameSize, 0);
		}
		#else
		dmx_memcpy(prCfaRm->puAudioBuf + u4OffsetInAudBuf,
					prCfaRm->pu1HdrBuf + u2OffsetInHdrBuf, u2FrameSize);
		#endif
		prAPacketInfo->u2ParseredSize = prAPacketInfo->u2ParseredSize + u2FrameSize;
		prCfaRm->u1FrameIdxInPacket++;
		prAPacketInfo->u1TxFrameIdx++;
		if (prCfaRm->u1FrameIdxInPacket == u2FrameNumPerSupPacket) { /*tx data */
			rTxAudInfo.u8FileOfst = 0;
			rTxAudInfo.u8Len = (u64)u2FrameSize * u2FrameNumPerSupPacket;
			rTxAudInfo.u8Pts = DMX_INVALID_UINT64; /*change unit in Hz, STC Clock*/
			rTxAudInfo.u4PrsStrmId = (u32)prCfaRm->rAudioInfo.rCfgInfo.u1StrmNum;/*need modify*/
			rTxAudInfo.eAudType = prCfaRm->rAudioInfo.eAudType;

			rTxAudInfo.fgUnitStart = FALSE;
			prCfaRm->eCurTxStrmType = CFA_RM_TX_STRM_TYPE_AUD;

			prCfaRm->u1FrameIdxInPacket = 0;
			prCfaRm->u4PacketSum = 0;

			DmxLogD(DMX_MOD_CFA_RM,CFA_RM_LOG_SET,TEXT("[CFA_RM_ST_CTRL] Audio Len= 0x%llx, Time=%d ms\r\n"),
				rTxAudInfo.u8Len, prCfaRm->rPacketInfo.u4TimeStamp);

			mrResult = Spt4CfaBuf2AFifoAUCtrl(pvSptHdl, prCfaRm->puAudioBuf, &rTxAudInfo, 0);
			if (mrResult != RET_DMX_OK)
			{
				DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT, TEXT("[CFA_RM_ST_CTRL] line %d, call Spt4CfaBuf2AFifoAUCtrl() failed!\r\n"),DMX_LINE_NO);
				if (DMX_IS_RW_PLAY(pvSptHdl))
				{
					Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, FALSE, (u32)GAU_E_ERRCHUNK);
				}
				else
				{
					Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, TRUE, GAU_E_ERRCHUNK);
				}
				return;
			}

			if (CFA_VID_UNKNOWN == prCfaRm->eVidCodecType)
				Sleep(1);
			return;
		}
	}
	CfaRmSearchNextSc(pvSptHdl, prCfaRm, CFA_RM_ANA_ST_MEDIA_PACKET_HEADER, 0,
					RM_READ_FOR_PACKET_HEADER_SIZE, 0); /*RM_READ_FOR_CODEC_SIZE*/
}
#endif

/* RM CFA state control for transfer done
@return None
@note  This function will be called after a transfer is complete.*/
void CfaRmTxDoneStCtrl(void *pvSptHdl, u64 u8TxLen, CfaRmInst_T *prCfaRm)
{
	MRESULT mrResult = RET_DMX_OK;
	if (NULL == prCfaRm)
	{
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT, TEXT("[CFA_RM_ST_CTRL] line %d,Invalid parameter!\r\n"),DMX_LINE_NO);
		return;
	}

	/* check if Tx done results from Tx data to header buffer*/
	if (TRUE == prCfaRm->fgTxData2HdrBuf) {
		prCfaRm->pu1HdrBuf = (u8 *)prCfaRm->ptrPfrMemAddress;
		if (NULL == prCfaRm->pu1HdrBuf)
		{
			DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT, TEXT("[CFA_RM_ST_CTRL] line %d,pu1HdrBuf is NULL!\r\n"),DMX_LINE_NO);
			return;
		}
		prCfaRm->fgTxData2HdrBuf = FALSE;
	}

	switch (prCfaRm->eCurAnaSt) {
	case CFA_RM_ANA_ST_IDLE:
		CfaRmAnaStIdle();
		break;

	case CFA_RM_ANA_ST_MEDIA_PACKET_HEADER:
		CfaRmDataPacketHeader(pvSptHdl, prCfaRm, u8TxLen);
		break;

	case CFA_RM_ANA_ST_SEARCH_HEADER:
		CfaRmSearchNextSc(pvSptHdl, prCfaRm, CFA_RM_ANA_ST_MEDIA_PACKET_HEADER, 0,
						RM_READ_FOR_PACKET_HEADER_SIZE, 0); /*RM_READ_FOR_CODEC_SIZE*/
		break;

	case CFA_RM_ANA_ST_TX_AUD:
		prCfaRm->eCurAnaSt = CFA_RM_ANA_ST_SEARCH_HEADER;
		mrResult = CfaRmTxAudData2Fifo(pvSptHdl, prCfaRm);
		if (mrResult != RET_DMX_OK)
		{
			DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT, TEXT("[CFA_RM_ST_CTRL] line %d, call CfaRmTxAudData2Fifo() failed!\r\n"),DMX_LINE_NO);
			if (DMX_IS_RW_PLAY(pvSptHdl))
			{
				Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, FALSE, (u32)GAU_E_ERRCHUNK);
			}
			else
			{
				Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, TRUE, GAU_E_ERRCHUNK);
			}
			return;
		}
		break;

	case CFA_RM_ANA_ST_TX_VIDEO:
		prCfaRm->eCurAnaSt = CFA_RM_ANA_ST_SEARCH_HEADER;
		mrResult = CfaRmTxVidData2Fifo(pvSptHdl, prCfaRm);
		if (mrResult != RET_DMX_OK)
		{
			DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT, TEXT("[CFA_RM_ST_CTRL] line %d, call CfaRmTxVidData2Fifo() failed!\r\n"),DMX_LINE_NO);
			if (DMX_IS_RW_PLAY(pvSptHdl))
			{
				Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, FALSE, (u32)GAU_E_ERRCHUNK);
			}
			else
			{
				Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, TRUE, GAU_E_ERRCHUNK);
			}
			return;
		}
		break;

	case CFA_RM_ANA_ST_TX_A_HDR:
		prCfaRm->eCurAnaSt = CFA_RM_ANA_ST_TX_AUD;
		mrResult = CfaRmTxAudHdl2Fifo(pvSptHdl, prCfaRm);
		if (mrResult != RET_DMX_OK)
		{
			DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT, TEXT("[CFA_RM_ST_CTRL] line %d, call CfaRmTxAudHdl2Fifo() failed!\r\n"),DMX_LINE_NO);
			if (DMX_IS_RW_PLAY(pvSptHdl))
			{
				Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, FALSE, (u32)GAU_E_ERRCHUNK);
			}
			else
			{
				Spt4CfaFinishedEx(pvSptHdl, prCfaRm->u8Ca, TRUE, GAU_E_ERRCHUNK);
			}
			return;
		}
		break;

	case CFA_RM_ANA_ST_LAST_PATIAL_FRAME:
		CfaRmLastPatialFrame(pvSptHdl, prCfaRm);
		break;

	case CFA_RM_ANA_ST_MULTIPLE_FRAME:
		/*analyze multiple frame data*/
		CfaRmMultiFrame(pvSptHdl, prCfaRm);
		break;

	#if CFA_RM_DEINTERLEAVE_AUDIO
	case CFA_RM_ANA_ST_CBR_AUDIO_DATA:
		/*save audio data to bug,need deinterleave*/
		CfaRmDeInterleaveAudio(pvSptHdl, prCfaRm);
		break;
	#endif
	default:
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT, TEXT("[CFA_RM_ST_CTRL] line %d, eCurAnaSt error!\r\n"),DMX_LINE_NO);
		CfaRmFinishPrs(pvSptHdl, prCfaRm);
		return;
	}

	if (mrResult != RET_DMX_OK)
	{
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT, TEXT("[CFA_RM_ST_CTRL] line %d, mrResult error!\r\n"),DMX_LINE_NO);
		CfaRmFinishPrs(pvSptHdl, prCfaRm);
		return;
	}
}

/*-----------------------------------------------------------------------------
 * Name: CfaRmGetTxSa
 *
 * Description:
 *		RM CFA get available transfer start address
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: available transfer start address
 *
 *-----------------------------------------------------------------------------*/
u64 CfaRmGetTxSa(CfaRmInst_T *prCfaRm)
{
	u64 u8StartAddr = DMX_INVALID_UINT64;

	if (NULL == prCfaRm)
	{
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT, TEXT("[CFA_RM_ST_CTRL] line %d, prCfaRm is NULL!\r\n"),DMX_LINE_NO);
		return 0;
	}

	if (prCfaRm->u8Ca != DMX_INVALID_UINT64)
		return prCfaRm->u8Ca;

	if ((prCfaRm->u4CurPrsFlag & CFA_RM_PRS_BIT_STRM_TYPE_VID) &&
		(prCfaRm->rRange.u8VidSa != DMX_INVALID_UINT64))
		u8StartAddr = prCfaRm->rRange.u8VidSa;

	if ((prCfaRm->u4CurPrsFlag & CFA_RM_PRS_BIT_STRM_TYPE_AUD) &&
		(prCfaRm->rRange.u8AudSa != DMX_INVALID_UINT64))
		u8StartAddr = MIN(u8StartAddr, prCfaRm->rRange.u8AudSa);

	/* error handling only. */
	if (u8StartAddr == DMX_INVALID_UINT64)
		u8StartAddr = 0;

	prCfaRm->u8Ca = u8StartAddr;

	return u8StartAddr;
}

/*-----------------------------------------------------------------------------
 * Name: fgCfaRmIncPrsPos
 *
 * Description:
 *		RM CFA increases current parsing position
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: TRUE - meet range end, FALSE - not meet range end
 *
 *-----------------------------------------------------------------------------*/
static bool CfaRmIncPrsPos(CfaRmInst_T *prCfaRm)
{
	bool fgAdoFinished = FALSE;
	bool fgVdoFinished = FALSE;

	CfaRmRange_T *prCfaRange = &(prCfaRm->rRange);

	if (prCfaRm->u8Ca + 1 >= prCfaRm->u8Endoffst)
		return TRUE;

	/*Todo: need check if all audio is finished*/
	if ((CfaRmToPlay(prCfaRm->u4CurPrsFlag, CFA_RM_PRS_BIT_STRM_TYPE_AUD)) &&
		(prCfaRange->u8AudSa != DMX_INVALID_UINT64)) {
		/*
		for kernel code style
		*/
	} else if ((prCfaRm->u4CfaPrsFlag == CFA_RM_PRS_BIT_STRM_TYPE_AUD) &&
	   (prCfaRange->u8AudSa != DMX_INVALID_UINT64)) {
		/*pure audio ?*/
		/**/
	} else
		fgAdoFinished = TRUE;

	if ((CfaRmToPlay(prCfaRm->u4CurPrsFlag, CFA_RM_PRS_BIT_STRM_TYPE_VID)) &&
		(prCfaRange->u8VidSa != DMX_INVALID_UINT64)) {
		if (prCfaRm->rPacketInfo.u8Offset > prCfaRange->u8VidEa)/*need modify */
			fgVdoFinished = TRUE;
	} else
		fgVdoFinished = TRUE;

	return fgAdoFinished&fgVdoFinished;
}
/*****************************************************************
///Descripiton: Start to search next tag header and get parsing payload info.
///@return None
//<[IN] handle of splitter
//<[IN] pointer to CfaRmInst
//<[IN] next analyze state
//<[IN] previous transfer file length before search next start code
//<[IN] the data length we want to read
//<[IN] header buffer ofst. to put the data from this ofst
*******************************************************************/
EXTERN void CfaRmSearchNextSc(void *pvSptHdl, CfaRmInst_T *prCfaRm, CfaRmAnaSt_E eNextAnaSt,
							u64 u8PreLen, u64 u8ReadLen, u32 u4DestOft)
{
	MRESULT mrResult = RET_DMX_OK;
	u64 u8Sa = DMX_INVALID_UINT64;
	u64 u8RangEndOffset = 0;

	if (NULL == prCfaRm)
	{
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT, TEXT("[CFA_RM_ST_CTRL] line %d,Invalid parameter!\r\n"),DMX_LINE_NO);
		return;
	}

	u8RangEndOffset = prCfaRm->u8Endoffst;

	/* check if all data are parsed */
	if (TRUE == CfaRmIncPrsPos(prCfaRm)) {
		/* finish current parsing */
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM_ST_CTRL] SCode call FinishPrs: %x\r\n"), 0x01);
		CfaRmFinishPrs(pvSptHdl, prCfaRm);
	} else {
		u8Sa = CfaRmGetTxSa(prCfaRm);
		if (DMX_INVALID_UINT64 == u8Sa)
		{
			DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT, TEXT("[CFA_RM_ST_CTRL] line %d, u8Sa error!\r\n"),DMX_LINE_NO);
			CfaRmFinishPrs(pvSptHdl, prCfaRm);
		}

		prCfaRm->eCurAnaSt = eNextAnaSt;
		/*[CJ070307] Check if this transfer is within IO read session range */
		if (u8RangEndOffset <= (u8Sa + u8ReadLen + u4DestOft)) {
			DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
				TEXT("[CFA_RM_ST_CTRL] u8RangEndOffset <= (u8Sa + u8ReadLen + u4DestOft)!\r\n"));
			/* Fix CR CNB00005351
			When left data size less than u8ReadLen, if we dec u8ReadLen, CfaRmDataPacketHeader will do
			noting and demuxer will hang. So we filish.*/
			CfaRmFinishPrs(pvSptHdl, prCfaRm);
			return;
		}
		mrResult = Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaRm->u8Ca, u8ReadLen, (u8 *)&(prCfaRm->ptrPfrMemAddress));
		if (RET_DMX_OK != mrResult) {
			DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM_ST_CTRL] Spt4CfaPbb2SyncBuf no return ok!\r\n"));
			DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM_ST_CTRL] SCode call FinishPrs: %x\r\n"), 0x02);
			CfaRmFinishPrs(pvSptHdl, prCfaRm);
		}
		prCfaRm->fgTxData2HdrBuf = TRUE;
	}
}
