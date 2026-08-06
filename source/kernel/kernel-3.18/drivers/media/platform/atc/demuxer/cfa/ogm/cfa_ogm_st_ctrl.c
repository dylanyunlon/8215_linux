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

#include "cfa_ogm_st_ctrl.h"
#include "dmx_def.h"
#include "dmx_mem.h"
#include "cfa_macro.h"


/*-----------------------------------------------------------------------------
			macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/



static void OgmFinishPrs(const u8 *pu8FuncName, const u32 u4Line, void *pvSptHdl, CfaOgmInst *prCfaOgm);


#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
static u32 MapAacSampleRate(u32 u4SampleFreq)
{
	switch (u4SampleFreq) {
	case (u32)96000:
		return 0x0;
	case (u32)88200:
		return 0x1;
	case (u32)64000:
		return 0x2;
	case (u32)48000:
		return 0x3;
	case (u32)44100:
		return 0x4;
	case (u32)32000:
		return 0x5;
	case (u32)24000:
		return 0x6;
	case (u32)22050:
		return 0x7;
	case (u32)16000:
		return 0x8;
	case (u32)12000:
		return 0x9;
	case (u32)11025:
		return 0xa;
	case (u32)8000:
		return 0xb;
	case (u32)7350:
		return 0xc;
	default:
		return 0xff;
	}
}

static void SetAacADTSHdr(CfaOgmInst *prCfaOgm)
{
	u32 u4AudSamplePerSec = 0;
	u8 u1AudChannels = 0;
	u32 u4FrameSize = 0;

	u4AudSamplePerSec =
		MapAacSampleRate((u32) prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
			u8SamplePerUnit);
	u1AudChannels = prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].uChannel;
	u4FrameSize = (u32) prCfaOgm->rCfaOgmAACPacket.u8CurAACPacketLen + (u32)7;

	prCfaOgm->rCfaOgmAACPacket.pu1AACPacketData[0] = (u8)0xFF;
	prCfaOgm->rCfaOgmAACPacket.pu1AACPacketData[1] = (u8)0xF9;
	prCfaOgm->rCfaOgmAACPacket.pu1AACPacketData[2] =
		(u8) (((u32)1 << 6U) | ((u4AudSamplePerSec << 2U) & (u32)0x3C) | (u32)((u1AudChannels >> 2U) & (u8)0x1));
	prCfaOgm->rCfaOgmAACPacket.pu1AACPacketData[3] =
		(u8) (((u8)((u1AudChannels & (u8)0x3) << 6U)) | ((u4FrameSize >> 11U) & (u32)0x3));
	prCfaOgm->rCfaOgmAACPacket.pu1AACPacketData[4] = (u8) ((u4FrameSize >> 3U) & 0xFFU);
	prCfaOgm->rCfaOgmAACPacket.pu1AACPacketData[5] =
		(u8) (((u4FrameSize << 5U) & (u32)0xE0) | (((u32)0x7FF >> 6U) & (u32)0x1F));
	prCfaOgm->rCfaOgmAACPacket.pu1AACPacketData[6] = (u8)((u16)((u16)0x7FF << 2U) & (u16)0xFC);

}


/*transfer AAC stream header maybe 2 to 5 bytes*/
static void CfaOgmTxAACStrmHdr(void *pvSptHdl, CfaOgmInst *prCfaOgm)
{
	s32 i4Ret = 0;

	prCfaOgm->eCfaOgmCurState = CFA_OGM_ST_PACKET_TX;
	prCfaOgm->eCfaOgmCurPrsStrm = prCfaOgm->rCurPage.eCfaOgmStrmType;

	prCfaOgm->rCfaOgmAACPacket.fgTxAACStrmHdr = TRUE;
	prCfaOgm->rCfaOgmAACPacket.fgOnlyTxAACHeader = TRUE;

	i4Ret =
		(s32)Spt4CfaBuf2AFifo(pvSptHdl,
				prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].auAacHeader,
				prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].uAacHeaderLen,
				prCfaOgm->rCurPage.u4StreamNo,
				prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec);
	if (RET_DMX_OK != i4Ret) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] Spt4CfaBuf2AFifo error ret = %d\n"), i4Ret);
		OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);
	}

}


/*
* After one packet parsed,this function will be called to transfer header and data to specific FIFO
*/
static void CfaOgmTxAACPacketData(void *pvSptHdl, CfaOgmInst *prCfaOgm)
{
	MRESULT mrRet = RET_DMX_OK;
	CFA_AUDIO_INFO_T rAudInf;

	prCfaOgm->eCfaOgmCurState = CFA_OGM_ST_PACKET_TX;

	prCfaOgm->eCfaOgmCurPrsStrm = prCfaOgm->rCurPage.eCfaOgmStrmType;

	/*packet header */

	if (!prCfaOgm->rCfaOgmAACPacket.fgTxAACStrmHdr)
		CfaOgmTxAACStrmHdr(pvSptHdl, prCfaOgm);

	prCfaOgm->rCfaOgmAACPacket.fgOnlyTxAACData = TRUE;
	prCfaOgm->rCfaOgmAACPacket.fgOnlyTxAACHeader = FALSE;
	SetAacADTSHdr(prCfaOgm);

	mm_memset((void *) &rAudInf, 0, sizeof(rAudInf));
	rAudInf.u4PrsStrmId = prCfaOgm->rCurPage.u4StreamNo;
	if (prCfaOgm->rCurPage.uAudIdx >= MAX_NS_OGM_AUD)
		prCfaOgm->rCurPage.uAudIdx = MAX_NS_OGM_AUD - 1;

	rAudInf.eAudType = prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec;
	/*rAudInf.u8FileOfst = prCfaOgm->rCfaOgmAACPacket.pu1AACPacketData;*/
	rAudInf.u8Len = prCfaOgm->rCfaOgmAACPacket.u8CurAACPacketLen + 7;
	rAudInf.u8Pts = prCfaOgm->rCurPacket.u8AudStartPTS;

	mrRet = Spt4CfaBuf2AFifoAUCtrl(pvSptHdl, prCfaOgm->rCfaOgmAACPacket.pu1AACPacketData,
					&rAudInf, prCfaOgm->rCfaOgmAACPacket.u8CurAACPacketLen + 7);
	if (RET_DMX_OK != mrRet) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] Spt4CfaBuf2AFifoAUCtrl error ret = %d\n"),
			mrRet);
		OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);
	}

	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM]Audio Data To Fifo Offset : 0x%llx\n\n"),
			prCfaOgm->rCurPacket.u8AudStartOfst);
}


/*store the data of one packet*/
static void CfaOgmStoreAACPacketData(void *pvSptHdl, u64 u8TxLen, CfaOgmInst *prCfaOgm)
{
	s32 i4Ret = 0;

	prCfaOgm->eCfaOgmCurState = CFA_OGM_ST_AAC_PACKET_ANA;
	prCfaOgm->eCfaOgmCurPrsStrm = prCfaOgm->rCurPage.eCfaOgmStrmType;

	i4Ret = Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaOgm->rCurPacket.u8AudStartOfst,
				sizeof(u8) * (u8TxLen),
				(prCfaOgm->rCfaOgmAACPacket.
				pu1AACPacketData + 7 +
				prCfaOgm->rCfaOgmAACPacket.
				uLastNoEndPacketLen));
	if (RET_DMX_OK != i4Ret) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] Spt4CfaPbb2SyncBuf error ret = %d\n"),
			i4Ret);
		OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);
		return;
	}

	prCfaOgm->rCfaOgmAACPacket.u8AnaAACDataLen = u8TxLen;
	prCfaOgm->rCurPacket.uAudPacketNo++;
}


static void CfaOgmAACPacketAna(void *pvSptHdl, u64 u8TxLen, CfaOgmInst *prCfaOgm)
{
	/*if last packet is completely ended we should transfer the data */
	if ((!prCfaOgm->fgPageContainEnd)
		&& (prCfaOgm->rCurPacket.uAudPacketNo >= prCfaOgm->rCurPage.uPacketNs)) {
		CfaOgmNextState(pvSptHdl, prCfaOgm, (u64)CFA_OGM_HDR_READ_BYTES, CFA_OGM_ST_PAGE_HDR_ANA);
	} else {
		CfaOgmTxAACPacketData(pvSptHdl, prCfaOgm);
	}
}

#endif

static u8 GetLenbytes(u8 uPacketHeader)
{
	return (u8)((uPacketHeader >> 6U) | ((u8)((uPacketHeader << 1U) & 0x04U)));
}

static bool KeyFrame(u8 uPacketHeader)
{
	return ((((u8)((uPacketHeader >> 3U) & 0x01U)) == 1U) ? TRUE : FALSE);
}

static void PrePareToTxAudCmdQ(CfaOgmInst * prCfaOgm)
{
	u8 i = 0,j = 0;
	for ( ; i < prCfaOgm->u4AudNum; i++) {
		if(prCfaOgm->arAudCmdQInfo[i].u4EntryCnt > 0) {
			prCfaOgm->au8TxAudCmdQIndex[j] = i;
			j++;
		}
	}
	prCfaOgm->uTxAudCmdQNs = j;
}

static u64 OgmComputeLittleEndian(const u8 *pucPoint, u8 ucLen)
{
	u64 u8Result = 0;
	u8 ucOfst = 0;

	while (ucOfst < ucLen) {
		u8Result += ((u64)((*(pucPoint + ucOfst)) << (8U * ucOfst)));
		ucOfst++;
	}
	return u8Result;
}


static void ComputePTS(CfaOgmInst *prCfaOgm)
{
	if (prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_V) {
		prCfaOgm->rCurPacket.u8VidStartPTS =
			(prCfaOgm->rCurPacket.u8LastVidGranule) *
			(prCfaOgm->rCfaOgmVidStream.u8TimeUnit) * (u64)9 / (u64)1000;
	} else {
		if (prCfaOgm->rCurPage.uAudIdx >= MAX_NS_OGM_AUD)
			prCfaOgm->rCurPage.uAudIdx = MAX_NS_OGM_AUD - 1;
		prCfaOgm->rCurPacket.u8AudStartPTS =
			(prCfaOgm->rCurPacket.au8LastAudGranule[prCfaOgm->rCurPage.uAudIdx]) *
			(prCfaOgm->rCurPage.au8AudPtsPerGranule[prCfaOgm->rCurPage.uAudIdx]) / OGM_TIME;

		if((0 != prCfaOgm->u4DurationMs) && (prCfaOgm->rCurPacket.u8AudStartPTS / OGM_TIME_S_TO_PTS >
			prCfaOgm->u4DurationMs / 1000)) {				
			prCfaOgm->rCurPacket.u8AudStartPTS
			= DMX_INVALID_UINT64;
		} else if((DMX_INVALID_UINT64 != prCfaOgm->u8PreAudPts) &&
			(DMX_INVALID_UINT64 != prCfaOgm->rCurPacket.u8AudStartPTS)) {
			if(((prCfaOgm->rCurPacket.u8AudStartPTS > prCfaOgm->u8PreAudPts)
				&& (prCfaOgm->rCurPacket.u8AudStartPTS - prCfaOgm->u8PreAudPts
				> OGM_MAX_DIFFRENCE_TIME * OGM_TIME_S_TO_PTS))
				|| (prCfaOgm->rCurPacket.u8AudStartPTS < prCfaOgm->u8PreAudPts)) {
				prCfaOgm->rCurPacket.u8AudStartPTS = DMX_INVALID_UINT64;
			}				
		}

		prCfaOgm->u8PreAudPts = prCfaOgm->rCurPacket.u8AudStartPTS;

	}
}



static void GetIndexByStreamNo(u32 u4StreamNo, CfaOgmInst *prCfaOgm)
{
	u8 uIdx = 0;

	if (prCfaOgm->u4AudNum > MAX_NS_OGM_AUD) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] %s line %d,prCfaOgm->u4AudNum(%d) > MAX_NS_OGM_AUD(%d)!\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prCfaOgm->u4AudNum, MAX_NS_OGM_AUD);
		prCfaOgm->u4AudNum = MAX_NS_OGM_AUD;
	}

	for (uIdx = 0; uIdx < prCfaOgm->u4AudNum; uIdx++) {
		if (u4StreamNo == prCfaOgm->arCfaOgmAudStream[uIdx].u4AudStreamNo) {
			prCfaOgm->rCurPage.eCfaOgmStrmType = CFA_OGM_PRS_STRM_TYPE_A;
			prCfaOgm->rCurPage.uAudIdx = uIdx;
			return;
		}
	}

	if (u4StreamNo == prCfaOgm->rCfaOgmVidStream.u4VidStreamNo) {
		prCfaOgm->rCurPage.eCfaOgmStrmType = CFA_OGM_PRS_STRM_TYPE_V;
		prCfaOgm->rCurPage.uAudIdx = DMX_INVALID_UINT8;
		return;
	}

	prCfaOgm->rCurPage.eCfaOgmStrmType = CFA_OGM_PRS_STRM_TYPE_NONE;
	prCfaOgm->rCurPage.uAudIdx = DMX_INVALID_UINT8;
}

static MRESULT CfaOgmTxAudCmdQ(void *pvSptHdl, CfaOgmInst *prCfaOgm, u32 u4TxLen);

static BOOL CfaOgmTxAvCmdQ(void *pvSptHdl, CfaOgmInst * prCfaOgm,
		CfaOgmAnaState_E eCfaOgmSavedState,CfaOgmAnaState_E eCfaOgmStateWhenCmdQ);

/*-----------------------------------------------------------------------------
 * Name: OgmFinishPrs
 *
 * Description:
 *      After current range are processed already ,this function will be call to finish parse
 *
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: void
 *
 *-----------------------------------------------------------------------------*/
static void OgmFinishPrs(const u8 *pu8FuncName, const u32 u4Line, void *pvSptHdl, CfaOgmInst *prCfaOgm)
{
	MRESULT mrRet;
	bool fgTxCmdQ = FALSE;
	prCfaOgm->eCfaOgmCurState = CFA_OGM_ST_IDLE;
	prCfaOgm->eCfaOgmNextState = CFA_OGM_ST_IDLE;
	MMATE_CHECK_POINTER(prCfaOgm);
	MMATE_CHECK_STRUCT(prCfaOgm->rCurPacket);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmRange);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmAu);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmVidStream);

	DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
		TEXT("[CFA_OGM]%s line %d call OgmFinishPrs!\r\n"),
		pu8FuncName, u4Line);

	if (prCfaOgm->fgUseCMDQ) {
		fgTxCmdQ = CfaOgmTxAvCmdQ(pvSptHdl, prCfaOgm, CFA_OGM_ST_IDLE, CFA_OGM_ST_FINISH);
		if (fgTxCmdQ) {
			DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] %s,there are remainling audio or video data to tx by Cmdq!")
				TEXT("can not finish parse now!\n"),
				DMX_FUNC_NAME);
			return;
		}
	} else  {/*tx vorbis page is used cmdq although fgUseCMDQ is false for historical reason*/
		if (prCfaOgm->rAudCmdQInfo.u4EntryCnt > 0) {
			DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] %s,there are remainling audio data to tx by Cmdq!")
				TEXT("can not finish parse now!\n"),
				DMX_FUNC_NAME);
			CfaOgmTxAudCmdQ(pvSptHdl, prCfaOgm, 0);
			prCfaOgm->eCfaOgmCurState = CFA_OGM_ST_FINISH;
			return;
		}
	}

	if (prCfaOgm->rCfaOgmRange.fgVidRangeEn) {
		mrRet =
			Spt4CfaFinishedEx(pvSptHdl, prCfaOgm->rCfaOgmRange.u8VidEndOfst, FALSE, (u32)GAU_E_EOS);
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] Finish parse at 0x%llx , 0x%llx!\n"),
			 	prCfaOgm->rCfaOgmRange.u8VidEndOfst, prCfaOgm->u8Ca);
	} else {
		mrRet =
			Spt4CfaFinishedEx(pvSptHdl, prCfaOgm->rCfaOgmRange.u8AudEndOfst, FALSE, (u32)GAU_E_EOS);
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] Finish parse at 0x%llx ,0x%llx!\n"),
			 	prCfaOgm->rCfaOgmRange.u8AudEndOfst, prCfaOgm->u8Ca);
	}

}

static MRESULT CfaOgmTxAudCmdQ(void *pvSptHdl, CfaOgmInst *prCfaOgm, u32 u4TxLen)
{
	CFA_AUDIO_INFO_T rCfaOgmTxAudInfo = {0};
	DMX_CMDQ_TX_ENTRY_T *arCmdEntrys = prCfaOgm->arOgmInstCmdEntrys;
	u32 u4CmdEntryIdx = 0;
	MRESULT mrRet = RET_DMX_OK;
	u8 uAudIdx = 0;

	if (prCfaOgm->fgUseCMDQ) {
		uAudIdx = prCfaOgm->au8TxAudCmdQIndex[prCfaOgm->uCurCmdQIdx];
		
#if CFA_OGM_INTERNAL_DEBUG
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] %s enter, uAudIdx:%d\n"), DMX_FUNC_NAME,uAudIdx);
#endif
		if ((prCfaOgm->uTxAudCmdQNs == 0)
			|| (prCfaOgm->uCurCmdQIdx >= prCfaOgm->uTxAudCmdQNs)
			|| (uAudIdx >= prCfaOgm->u4AudNum)
			|| (prCfaOgm->arAudCmdQInfo[uAudIdx].u4EntryCnt == 0)) {
			DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] %s,line %d,some parameter is error!Finish parse!\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			
			Spt4CfaFinishedEx(pvSptHdl, prCfaOgm->rCfaOgmRange.u8AudEndOfst, FALSE, GAU_E_EOS);
			return RET_DMX_OPERATE_FORBID;
		}
		
		if (1 == prCfaOgm->uTxAudCmdQNs) {
			prCfaOgm->eCfaOgmCurState = prCfaOgm->eTmpStateWhenCmdQ;
		} else {
			if(prCfaOgm->uCurCmdQIdx < prCfaOgm->uTxAudCmdQNs - 1 ) {
				prCfaOgm->eCfaOgmCurState = CFA_OGM_ST_AVCMDQ_TX;
			} else {
			   prCfaOgm->eCfaOgmCurState = prCfaOgm->eTmpStateWhenCmdQ;
			}
		}
		prCfaOgm->uIdxForFillAU = uAudIdx;
		prCfaOgm->uCurCmdQIdx++;

		mm_memset(arCmdEntrys, 0, sizeof(DMX_CMDQ_TX_ENTRY_T) * DMX_MAX_TX_CNT_FOR_CMD_Q);
		rCfaOgmTxAudInfo.u8FileOfst = prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[0].u8FileOffset;
		rCfaOgmTxAudInfo.u8Len = prCfaOgm->arAudCmdQInfo[uAudIdx].u8TotalLen;
		rCfaOgmTxAudInfo.u8Pts = prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[0].u8Pts; 
		rCfaOgmTxAudInfo.u4PrsStrmId = prCfaOgm->arCfaOgmAudStream[uAudIdx].u4AudStreamNo;
		rCfaOgmTxAudInfo.eAudType = prCfaOgm->arCfaOgmAudStream[uAudIdx].eCfaAudCodec;
		rCfaOgmTxAudInfo.u8RealTxLen = prCfaOgm->arAudCmdQInfo[uAudIdx].u4RealTxLen;
		
		for (u4CmdEntryIdx = 0;
			(u4CmdEntryIdx < prCfaOgm->arAudCmdQInfo[uAudIdx].u4EntryCnt) && (prCfaOgm->arAudCmdQInfo[uAudIdx].u4EntryCnt <= DMX_MAX_TX_CNT_FOR_CMD_Q);
			u4CmdEntryIdx++) {
			arCmdEntrys[u4CmdEntryIdx].u4TxLen = prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[u4CmdEntryIdx].u4Len;
			if (0 == u4CmdEntryIdx) {
				arCmdEntrys[u4CmdEntryIdx].u4TxOfst = 0;
			} else {
				arCmdEntrys[u4CmdEntryIdx].u4TxOfst = (u32)(prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[u4CmdEntryIdx].u8FileOffset -
					(prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[u4CmdEntryIdx - 1].u8FileOffset + prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[u4CmdEntryIdx - 1].u4Len));
			}

			if (0 == (prCfaOgm->u4PrsFlg & CFA_OGM_PRS_STRM_TYPE_V)) {
				arCmdEntrys[u4CmdEntryIdx].fgEndAU = TRUE;
			} else {
				arCmdEntrys[u4CmdEntryIdx].fgEndAU = TRUE;
			}
		
#if CFA_OGM_DBG_AUD_CMD_Q_FLOW
			DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] %s line %d,AudIdx:%d -- ComposeCmdQ (ACmdIdx: %d, FileOfst: %lld, Len: %d, Pts: %lld ms, fgEnd: %d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, uAudIdx,
				u4CmdEntryIdx,
				prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[u4CmdEntryIdx].u8FileOffset,
				prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[u4CmdEntryIdx].u4Len,
				PTS_TO_MS(prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[u4CmdEntryIdx].u8Pts),
				(arCmdEntrys[u4CmdEntryIdx].fgEndAU ? 1 : 0));

#endif // CFA_Ogm_DBG_AUD_CMD_Q_FLOW
		}
		
		if (u4CmdEntryIdx > 0) {
			if (0 == (prCfaOgm->u4PrsFlg & CFA_OGM_PRS_STRM_TYPE_V)) {
				if (DMX_IS_RW_PLAY(pvSptHdl)) {
					if(prCfaOgm->u4FRAudDataTxLen + prCfaOgm->rCurPacket.u8AudLength > prCfaOgm->u4FRAudAuLen) {
						arCmdEntrys[u4CmdEntryIdx - 1].fgEndAU = TRUE;
						DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
							TEXT("[CFA_OGM] %s line %d -- Pure Audio AU End\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
					}
				} else {
					if (prCfaOgm->arAudCmdQInfo[uAudIdx].u4EntryCnt >= DMX_MAX_TX_CNT_FOR_CMD_Q) {
						arCmdEntrys[u4CmdEntryIdx - 1].fgEndAU = TRUE;
			#if CFA_OGM_DBG_AUD_CMD_Q_FLOW
						DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
							TEXT("[CFA_Ogm] %s line %d -- Pure Audio AU End\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
			#endif // CFA_Ogm_DBG_AUD_CMD_Q_FLOW
					}
				}
			} else {
				arCmdEntrys[u4CmdEntryIdx - 1].fgEndAU = TRUE;
			}
		}
		
		rCfaOgmTxAudInfo.fgUseCmdQ = TRUE;
		rCfaOgmTxAudInfo.fgAUByCmdQEnd = TRUE;
		rCfaOgmTxAudInfo.parCmdQTxEntry = arCmdEntrys;
		rCfaOgmTxAudInfo.u2TxEntryCnt = prCfaOgm->arAudCmdQInfo[uAudIdx].u4EntryCnt;
		rCfaOgmTxAudInfo.fgAUCompleteByEnd = FALSE;
		rCfaOgmTxAudInfo.fgUnitStart = TRUE;
		rCfaOgmTxAudInfo.u8TotalAULen = prCfaOgm->arAudCmdQInfo[uAudIdx].u8TotalLen;
		prCfaOgm->arAudCmdQInfo[uAudIdx].fgIsInDma = TRUE;
#if CFA_OGM_TMP_DBG
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] %s line:%d \n"),DMX_FUNC_NAME,DMX_LINE_NO);
#endif
		mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rCfaOgmTxAudInfo);
		prCfaOgm->au8AudParsedOfst[prCfaOgm->uIdxForFillAU] = rCfaOgmTxAudInfo.u8FileOfst
			+ rCfaOgmTxAudInfo.u8Len;
		if(RET_DMX_OK != mrRet) {
			DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] Spt4CfaPbb2AFifoAUCtrl error ret = %d \n"), mrRet);
			OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
		
			return mrRet;
		}
#if CFA_OGM_TMP_DBG
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] %s line:%d \n"),DMX_FUNC_NAME,DMX_LINE_NO);
#endif	  
		return mrRet;
	}

	uAudIdx = prCfaOgm->rAudCmdQInfo.uAudIdx;

	if (prCfaOgm->rAudCmdQInfo.u4EntryCnt == 0)
		return RET_DMX_OK;

	mm_memset(arCmdEntrys, 0, sizeof(DMX_CMDQ_TX_ENTRY_T) * DMX_MAX_TX_CNT_FOR_CMD_Q);
	rCfaOgmTxAudInfo.u8FileOfst = prCfaOgm->rAudCmdQInfo.arEntrys[0].u8FileOffset;
	rCfaOgmTxAudInfo.u8Len = prCfaOgm->rAudCmdQInfo.u8TotalLen;
	rCfaOgmTxAudInfo.u8Pts = prCfaOgm->rAudCmdQInfo.arEntrys[0].u8Pts;
	rCfaOgmTxAudInfo.u4PrsStrmId = prCfaOgm->arCfaOgmAudStream[uAudIdx].u4AudStreamNo;
	rCfaOgmTxAudInfo.eAudType = prCfaOgm->arCfaOgmAudStream[uAudIdx].eCfaAudCodec;
	rCfaOgmTxAudInfo.u8RealTxLen = prCfaOgm->rAudCmdQInfo.u4RealTxLen;

	for (u4CmdEntryIdx = 0;
	 	(u4CmdEntryIdx < prCfaOgm->rAudCmdQInfo.u4EntryCnt)
	 	&& (prCfaOgm->rAudCmdQInfo.u4EntryCnt <= DMX_MAX_TX_CNT_FOR_CMD_Q); u4CmdEntryIdx++) {
		arCmdEntrys[u4CmdEntryIdx].u4TxLen =
			prCfaOgm->rAudCmdQInfo.arEntrys[u4CmdEntryIdx].u4Len;
		if (0 == u4CmdEntryIdx) {

			arCmdEntrys[u4CmdEntryIdx].u4TxOfst = 0;
		} else {
			arCmdEntrys[u4CmdEntryIdx].u4TxOfst =
			 	(u32) (prCfaOgm->rAudCmdQInfo.arEntrys[u4CmdEntryIdx].u8FileOffset -
				(prCfaOgm->rAudCmdQInfo.arEntrys[u4CmdEntryIdx - (u32)1].
				u8FileOffset +
				(u64)(prCfaOgm->rAudCmdQInfo.arEntrys[u4CmdEntryIdx - (u32)1].u4Len)));
		}
		/*arCmdEntrys[u4CmdEntryIdx].fgEndAU = FALSE;*/
    	if (0 == (prCfaOgm->u4PrsFlg & CFA_OGM_PRS_STRM_TYPE_V)) {
			arCmdEntrys[u4CmdEntryIdx].fgEndAU = TRUE;
		} else {
			arCmdEntrys[u4CmdEntryIdx].fgEndAU = TRUE;
		}

		if (u4CmdEntryIdx == prCfaOgm->rAudCmdQInfo.u4EntryCnt - 1) {
			arCmdEntrys[u4CmdEntryIdx].fgEndAU = TRUE;
		}


#if CFA_OGM_DBG_AUD_CMD_Q_FLOW
		DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_CMDQ_A,
			 TEXT("[CFA_OGM]CmdQ(ACmdIdx: %d, FileOfst: %llx,")
			 TEXT(" Len: %d)\r\n"),
			 u4CmdEntryIdx,
			 prCfaOgm->rAudCmdQInfo.arEntrys[u4CmdEntryIdx].u8FileOffset,
			 prCfaOgm->rAudCmdQInfo.arEntrys[u4CmdEntryIdx].u4Len);

		DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_CMDQ_A,
			 TEXT("[CFA_OGM] %s line %d -- ComposeCmdQ (ACmdIdx: %d, FileOfst: %lld,")
			 TEXT(" Len: %d, Pts: %lld ms, fgEnd: %d)\r\n"),
			 DMX_FUNC_NAME, DMX_LINE_NO, u4CmdEntryIdx,
			 prCfaOgm->rAudCmdQInfo.arEntrys[u4CmdEntryIdx].u8FileOffset,
			 prCfaOgm->rAudCmdQInfo.arEntrys[u4CmdEntryIdx].u4Len,
			 PTS_TO_MS(prCfaOgm->rAudCmdQInfo.arEntrys[u4CmdEntryIdx].u8Pts),
			 (arCmdEntrys[u4CmdEntryIdx].fgEndAU ? 1 : 0));
#endif				/* CFA_FLV_DBG_AUD_CMD_Q_FLOW*/
	}

	rCfaOgmTxAudInfo.fgUseCmdQ = TRUE;
	rCfaOgmTxAudInfo.fgAUByCmdQEnd = TRUE;
	rCfaOgmTxAudInfo.parCmdQTxEntry = arCmdEntrys;
	rCfaOgmTxAudInfo.u2TxEntryCnt = prCfaOgm->rAudCmdQInfo.u4EntryCnt;
	rCfaOgmTxAudInfo.fgAUCompleteByEnd = FALSE;
	rCfaOgmTxAudInfo.fgUnitStart = TRUE;
	rCfaOgmTxAudInfo.u8TotalAULen = prCfaOgm->rAudCmdQInfo.u8TotalLen;
	prCfaOgm->rAudCmdQInfo.fgIsInDma = TRUE;

	mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rCfaOgmTxAudInfo);
	if (RET_DMX_OK != mrRet) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] Spt4CfaPbb2AFifoAUCtrl error ret = %d\n"),
			mrRet);
		OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);

		return mrRet;
	}
	return mrRet;

}

static MRESULT CfaOgmTxVidCmdQ(void *pvSptHdl, CfaOgmInst *prCfaOgm, UINT32 u4TxLen,BOOL fgTrigAud)
{
	CFA_VIDEO_INFO_T rCfaOgmTxVidInfo = {0};
	DMX_CMDQ_TX_ENTRY_T  *arCmdEntrys = prCfaOgm->arOgmInstCmdEntrys;
	CfaOgmVidCmdQInfo_T *prCmdQInfo = &(prCfaOgm->rVidCmdQInfo);
	UINT32 u4CmdEntryIdx = 0;
	MRESULT mrRet = RET_DMX_OK;

	#if CFA_OGM_INTERNAL_DEBUG
	DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
		TEXT("[CFA_OGM] %s enter\n"), DMX_FUNC_NAME);
	#endif
	
	if (prCmdQInfo->u4EntryCnt == 0) {
		DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] %s line %d,prCmdQInfo->u4EntryCnt is 0! Finish parse!\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		Spt4CfaFinishedEx(pvSptHdl, prCfaOgm->rCfaOgmRange.u8VidEndOfst, FALSE, GAU_E_EOS);
		return RET_DMX_OPERATE_FORBID;
	}

	if (fgTrigAud) {
		PrePareToTxAudCmdQ(prCfaOgm);
		if (prCfaOgm->uTxAudCmdQNs > 0) {
			prCfaOgm->fgHasTxVidCmdQ = TRUE;
			prCfaOgm->eCfaOgmCurState = CFA_OGM_ST_AVCMDQ_TX;
		} else {
			prCfaOgm->eCfaOgmCurState = prCfaOgm->eTmpStateWhenCmdQ;
		}
	} else {
		prCfaOgm->eCfaOgmCurState = prCfaOgm->eTmpStateWhenCmdQ;
	}

	mm_memset(arCmdEntrys, 0, sizeof(DMX_CMDQ_TX_ENTRY_T) * DMX_MAX_TX_CNT_FOR_CMD_Q);
	rCfaOgmTxVidInfo.u8FileOfst = prCmdQInfo->arEntrys[0].u8FileOffset;
	rCfaOgmTxVidInfo.u8Len = prCmdQInfo->u8TotalLen;
	rCfaOgmTxVidInfo.u4PrsStrmId = (UINT32)prCfaOgm->rCfaOgmVidStream.u4VidStreamNo;
	rCfaOgmTxVidInfo.u8RealTxLen = prCmdQInfo->u4RealTxLen;
	rCfaOgmTxVidInfo.u8TotalAULen   = prCmdQInfo->u8TotalLen;

	rCfaOgmTxVidInfo.u8FileOfst  = prCmdQInfo->arEntrys[0].u8FileOffset;
	rCfaOgmTxVidInfo.eVidType    = prCfaOgm->rCfaOgmVidStream.eCfaVidCodec;

	for (u4CmdEntryIdx = 0;
		(u4CmdEntryIdx < prCmdQInfo->u4EntryCnt) && (prCmdQInfo->u4EntryCnt <= DMX_MAX_TX_CNT_FOR_CMD_Q);
		u4CmdEntryIdx++) {
		arCmdEntrys[u4CmdEntryIdx].u4TxLen = prCmdQInfo->arEntrys[u4CmdEntryIdx].u4Len;
		if (0 == u4CmdEntryIdx) {
		arCmdEntrys[u4CmdEntryIdx].u4TxOfst = 0;
		} else {
			arCmdEntrys[u4CmdEntryIdx].u4TxOfst = (UINT32)(prCmdQInfo->arEntrys[u4CmdEntryIdx].u8FileOffset -
			(prCmdQInfo->arEntrys[u4CmdEntryIdx - 1].u8FileOffset + prCmdQInfo->arEntrys[u4CmdEntryIdx - 1].u4Len));
		}
		arCmdEntrys[u4CmdEntryIdx].fgEndAU = FALSE;

		if (CFA_VID_DIVX3 == prCfaOgm->rCfaOgmVidStream.eCfaVidCodec) {
			if (prCmdQInfo->arEntrys[u4CmdEntryIdx].fgUnitEnd) {
				arCmdEntrys[u4CmdEntryIdx].fgEndAU = TRUE;
			}
			#if ENABLE_DMX_ADVANCED_VER
			arCmdEntrys[u4CmdEntryIdx].eTxMode = prCmdQInfo->arEntrys[u4CmdEntryIdx].eTxMode;
			#endif
		}

		#if CFA_OGM_DBG_VID_CMD_Q_FLOW
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] %s line %d -- ComposeCmdQ (VCmdIdx: %d, FileOfst: %lld, ")
			TEXT("Len: %d, Pts: %lldms, fgEnd: %d,TXMode:%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4CmdEntryIdx,
			prCfaOgm->rVidCmdQInfo.arEntrys[u4CmdEntryIdx].u8FileOffset,
			prCfaOgm->rVidCmdQInfo.arEntrys[u4CmdEntryIdx].u4Len,
			PTS_TO_MS(prCfaOgm->rVidCmdQInfo.arEntrys[u4CmdEntryIdx].u8Pts),
			(arCmdEntrys[u4CmdEntryIdx].fgEndAU ? 1 : 0),
			prCfaOgm->rVidCmdQInfo.arEntrys[u4CmdEntryIdx].eTxMode);
		#endif // CFA_Ogm_DBG_AUD_CMD_Q_FLOW
	}

	rCfaOgmTxVidInfo.fgUseCmdQ = TRUE;
	rCfaOgmTxVidInfo.parCmdQTxEntry = arCmdEntrys;
	rCfaOgmTxVidInfo.u2TxEntryCnt = prCmdQInfo->u4EntryCnt;
	rCfaOgmTxVidInfo.fgAUCompleteByEnd = FALSE;
	/*rCfaOgmTxVidInfo.fgAUByCmdQEnd = FALSE;*/
	rCfaOgmTxVidInfo.fgUnitStart = TRUE;

	if (CFA_VID_DIVX3 == prCfaOgm->rCfaOgmVidStream.eCfaVidCodec) {
		rCfaOgmTxVidInfo.fgAUCompleteByEnd = FALSE;
		#if ENABLE_DMX_ADVANCED_VER
		rCfaOgmTxVidInfo.fgAUByCmdQEnd = TRUE;
		#endif
		if (prCmdQInfo->arEntrys[0].fgUnitStart) {
			rCfaOgmTxVidInfo.fgUnitStart = TRUE;
		} else {
			rCfaOgmTxVidInfo.fgUnitStart = FALSE;
		}
		rCfaOgmTxVidInfo.fgUnitEnd = FALSE;
		//rCfaOgmTxVidInfo.u8TotalAULen = 0;
	}

	prCmdQInfo->fgIsInDma = TRUE;

	#if CFA_OGM_DBG_VID_CMD_Q_FLOW
	DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
		TEXT("[CFA_OGM] %s line %d -- DmaVid (VCmdCnt: %d, FileOfst: %lld, Len: %lld, RealTxLen: %lld), Ca: %lld, Pts: %lld ms\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, rCfaOgmTxVidInfo.u2TxEntryCnt,
		rCfaOgmTxVidInfo.u8FileOfst, rCfaOgmTxVidInfo.u8Len,
		rCfaOgmTxVidInfo.u8RealTxLen,
		prCfaOgm->u8Ca, PTS_TO_MS(prCmdQInfo->arEntrys[0].u8Pts));
	#endif // CFA_Ogm_DBG_VID_CMD_Q_FLOW

	mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rCfaOgmTxVidInfo);
	prCfaOgm->u8VidParsedOfst = rCfaOgmTxVidInfo.u8FileOfst + rCfaOgmTxVidInfo.u8Len;
	if (RET_DMX_OK != mrRet) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] Spt4CfaPbb2AFifoAUCtrl error ret = %d \n"), mrRet);
		OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);

		return mrRet;
	}

	MM_RETURN(RET_DMX_OK);
}

static BOOL CfaOgmTxAvCmdQ(void *pvSptHdl, CfaOgmInst * prCfaOgm,
		CfaOgmAnaState_E eCfaOgmSavedState,CfaOgmAnaState_E eCfaOgmStateWhenCmdQ)
{
	#if CFA_OGM_INTERNAL_DEBUG
	DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
		TEXT("[CFA_OGM] %s enter\n"), DMX_FUNC_NAME);
	#endif
	
	if (prCfaOgm->rVidCmdQInfo.u4EntryCnt > 0) {
		prCfaOgm->eCfaOgmNextState = eCfaOgmSavedState;
		prCfaOgm->eTmpStateWhenCmdQ = eCfaOgmStateWhenCmdQ;
		CfaOgmTxVidCmdQ(pvSptHdl,prCfaOgm,0,TRUE);
		return TRUE;
	}
	
	PrePareToTxAudCmdQ(prCfaOgm);
	if (prCfaOgm->uTxAudCmdQNs > 0) {
		prCfaOgm->eCfaOgmNextState = eCfaOgmSavedState;
		prCfaOgm->eTmpStateWhenCmdQ = eCfaOgmStateWhenCmdQ;
		prCfaOgm->uCurCmdQIdx = 0;
		CfaOgmTxAudCmdQ(pvSptHdl,prCfaOgm,0);
		#if CFA_OGM_TMP_DBG
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] %s line:%d \n"),DMX_FUNC_NAME,DMX_LINE_NO);
		#endif
		return TRUE;
	}

	return FALSE;
}

/* if the video stream is not support ,or if the audio serial number
*   lpe send can't be found , then the page will be skipped            */
static void CfaOgmSkipPage(void *pvSptHdl, CfaOgmInst *prCfaOgm)
{
	prCfaOgm->u8Ca = prCfaOgm->rCurPage.u8DataOfst + prCfaOgm->rCurPage.u8DataLen;

	CfaOgmNextState(pvSptHdl, prCfaOgm, (u64)CFA_OGM_HDR_READ_BYTES, CFA_OGM_ST_PAGE_HDR_ANA);
}


/*-----------------------------------------------------------------------------
 * Name: vCfaOgmAnaStIdle
 *
 * Description:
 *      OGM CFA processes CFA_OGM_ANA_ST_IDLE state
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static void CfaOgmAnaStIdle(void *pvSptHdl, u64 u8TxLen, const CfaOgmInst *prCfaOgm)
{

	/* do nothing */
}

/*it will be called after CfaOgmTxPacketData*/
static void CfaOgmTxPacket(void *pvSptHdl, u64 u8TxLen, CfaOgmInst *prCfaOgm)
{
	prCfaOgm->u8HdrLen = 0;

	if (prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_V) {
		prCfaOgm->u8Ca =
		    prCfaOgm->rCurPacket.u8VidStartOfst + prCfaOgm->rCurPacket.u8VidLength;

		if (prCfaOgm->rCurPacket.uVidPacketNo >= prCfaOgm->rCurPage.uPacketNs) {
			if (prCfaOgm->u8Ca >= prCfaOgm->rCfaOgmRange.u8VidEndOfst) {
				OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);
				return;
			}
			CfaOgmNextState(pvSptHdl, prCfaOgm, (u64)CFA_OGM_HDR_READ_BYTES,
					CFA_OGM_ST_PAGE_HDR_ANA);
		} else {
			CfaOgmNextState(pvSptHdl, prCfaOgm, (u64)CFA_OGM_PACKET_HDR_READ,
					CFA_OGM_ST_PACKET_HDR_ANA);
		}

		return;
	}

	if ((CFA_AUD_DRV_FMT_VORBIS ==
	 	prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec)
	 	&& (prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
	 	fgNeedTxVorbisHeaderData)) {
		prCfaOgm->u8Ca -= u8TxLen;
		prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
			fgNeedTxVorbisHeaderData = FALSE;
		CfaOgmNextState(pvSptHdl, prCfaOgm, prCfaOgm->rCurPage.uSegmentNs,
				CFA_OGM_ST_PAGE_LACING_ANA);

		return;
	}
#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
	if (CFA_AUD_DRV_FMT_AAC ==
	 	prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec) {
		if ((prCfaOgm->rCfaOgmAACPacket.fgOnlyTxAACHeader)
			&& (!prCfaOgm->rCfaOgmAACPacket.fgOnlyTxAACData)) {
			prCfaOgm->rCurPacket.u8AudLength = 0;

			prCfaOgm->u8Ca =
			 	prCfaOgm->rCurPacket.u8AudStartOfst +
			 	prCfaOgm->rCurPacket.u8AudLength;
			if (prCfaOgm->rCurPage.uAudIdx >= MAX_NS_OGM_AUD) {
				prCfaOgm->rCurPage.uAudIdx = MAX_NS_OGM_AUD - 1;
			}

			CfaOgmNextState(pvSptHdl, prCfaOgm, (u64)prCfaOgm->rCurPage.uSegmentNs,
					CFA_OGM_ST_PAGE_LACING_ANA);
			return;
		} else if ((!prCfaOgm->rCfaOgmAACPacket.fgOnlyTxAACHeader)
			&& (prCfaOgm->rCfaOgmAACPacket.fgOnlyTxAACData)) {
			prCfaOgm->rCurPacket.u8AudLength =
			prCfaOgm->rCfaOgmAACPacket.u8AnaAACDataLen;
			dmx_memset((u8 *) prCfaOgm->rCfaOgmAACPacket.pu1AACPacketData, 0,
				sizeof(u8) * (CFA_OGM_AAC_PACKET_MAX_LEN));
		} else {
			/*do nothing*/
		}
		/*if this is the last completely packet , we should go to next page, else go to next packet */
	}
#endif
	prCfaOgm->u8Ca =
		prCfaOgm->rCurPacket.u8AudStartOfst + prCfaOgm->rCurPacket.u8AudLength;
	if (prCfaOgm->rCurPage.uAudIdx >= MAX_NS_OGM_AUD) {
		prCfaOgm->rCurPage.uAudIdx = MAX_NS_OGM_AUD - 1;
	}

	if (prCfaOgm->rCurPage.uAudIdx >= MAX_NS_OGM_AUD)
		prCfaOgm->rCurPage.uAudIdx = MAX_NS_OGM_AUD - 1;

	if ((prCfaOgm->rCurPacket.uAudPacketNo >= prCfaOgm->rCurPage.uPacketNs)
		|| (prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec ==
		CFA_AUD_DRV_FMT_VORBIS)) {
		if (prCfaOgm->u8Ca >= prCfaOgm->rCfaOgmRange.u8AudEndOfst) {
			OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);
			return;
		}
		CfaOgmNextState(pvSptHdl, prCfaOgm, (u64)CFA_OGM_HDR_READ_BYTES,
				CFA_OGM_ST_PAGE_HDR_ANA);
	} else {
		CfaOgmNextState(pvSptHdl, prCfaOgm, (u64)CFA_OGM_PACKET_HDR_READ,
				CFA_OGM_ST_PACKET_HDR_ANA);
	}
}

static void CfaOgmTxVorbisHeader(void *pvSptHdl, CfaOgmInst *prCfaOgm)
{
	s32 i4Ret = 0;

	prCfaOgm->eCfaOgmCurState = CFA_OGM_ST_PACKET_TX;
	prCfaOgm->eCfaOgmCurPrsStrm = CFA_OGM_PRS_STRM_TYPE_A;


	i4Ret =
		(s32)Spt4CfaBuf2AFifo(pvSptHdl,
				prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
				pu1VorbisHeaderData,
				prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
				u4VorbisHeaderSize, prCfaOgm->rCurPage.u4StreamNo,
				prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec);
	if (RET_DMX_OK != i4Ret) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM]CfaOgmTxVorbisHeader Spt4CfaBuf2AFifo error ret = %d\n"),
			i4Ret);
		OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);
	}
}


static void CfaOgmAnaPageHeader(void *pvSptHdl, u64 u8TxLen, CfaOgmInst *prCfaOgm)
{
	prCfaOgm->puOgmHdr = (u8 *) (prCfaOgm->ptrMemAddr);	/*the end of file*/

	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
		TEXT("[CFA_OGM] %s enter, PageOffset:0x%llx\n"), __func__, prCfaOgm->u8Ca - CFA_OGM_HDR_READ_BYTES);

	if (NULL == prCfaOgm->puOgmHdr) {
		DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] %s line %d,prCfaOgm->puOgmHdr is NULL! Finish parse!\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);
		return;
	}

	if (!(((u8)('O') == prCfaOgm->puOgmHdr[0]) && ((u8)('g') == prCfaOgm->puOgmHdr[1]) &&
		((u8)('g') == prCfaOgm->puOgmHdr[2]) && ((u8)('S') == prCfaOgm->puOgmHdr[3]))) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] Can not find OGM start code: 0x%llx, Ca:0x%llx\n"),
			prCfaOgm->u8Ca - CFA_OGM_HDR_READ_BYTES, prCfaOgm->u8Ca);
		OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);
		return;
	}

	prCfaOgm->u8PageOfst = prCfaOgm->u8Ca - CFA_OGM_HDR_READ_BYTES;

	dmx_memset((void *) (prCfaOgm->rCurPage.au4PacketLen), 0,
		   (prCfaOgm->rCurPage.uPacketNs) * sizeof(u32));

	prCfaOgm->rCurPage.u8DataLen = 0;
	prCfaOgm->rCurPage.u8DataOfst = DMX_INVALID_UINT64;
	prCfaOgm->rCurPage.u8StartOfst = DMX_INVALID_UINT64;
	prCfaOgm->rCurPage.uPacketNs = 0;
	prCfaOgm->rCurPacket.uAudPacketNo = 0;
	prCfaOgm->rCurPacket.uVidPacketNo = 0;
	prCfaOgm->rCurPage.u8StartOfst = prCfaOgm->u8Ca - CFA_OGM_HDR_READ_BYTES;



	if ((prCfaOgm->puOgmHdr[CFA_OGM_HDR_TYPE]) & 0x01U)
		prCfaOgm->rCurPage.fgFreshPacket = FALSE;
	else
		prCfaOgm->rCurPage.fgFreshPacket = TRUE;

	prCfaOgm->rCurPage.u4StreamNo =
	    (u32) OgmComputeLittleEndian(&(prCfaOgm->puOgmHdr[CFA_OGM_HDR_STREAM_NO]),
					    CFA_OGM_STREAM_NO_LEN);
	prCfaOgm->rCurPage.uSegmentNs =
	    (u8) OgmComputeLittleEndian(&(prCfaOgm->puOgmHdr[CFA_OGM_HDR_SEGMENT]),
					   CFA_OGM_SEGMENT_LEN);
	GetIndexByStreamNo(prCfaOgm->rCurPage.u4StreamNo, prCfaOgm);


	prCfaOgm->rCurPage.u8GranulPosition =
	    OgmComputeLittleEndian(&(prCfaOgm->puOgmHdr[CFA_OGM_HDR_GRANUL_POSITION]),
				   CFA_OGM_GRANULE_POSITION_LEN);

	if (prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_A) {
		if(prCfaOgm->rCurPage.uAudIdx >= MAX_NS_OGM_AUD) {
			prCfaOgm->rCurPage.uAudIdx = MAX_NS_OGM_AUD - 1;
		}

		if (CFA_AUD_DRV_FMT_VORBIS ==
			prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec) {
			prCfaOgm->rCurPage.u8LastAudGranulePosition =
			prCfaOgm->rCurPage.u8ThisAudGranulePosition;
			prCfaOgm->rCurPage.u8ThisAudGranulePosition =
			prCfaOgm->rCurPage.u8GranulPosition;
		}		/*case of vorbis codec are not validated */
	}

	switch (prCfaOgm->rCurPage.eCfaOgmStrmType) {
	case CFA_OGM_PRS_STRM_TYPE_V:
		if (prCfaOgm->rCfaOgmRange.u8VidStartOfst == prCfaOgm->rCurPage.u8StartOfst) {
			prCfaOgm->rCurPage.fgFirstVidPage = TRUE;
			prCfaOgm->rCurPage.u8VidPtsPerGranule =
				(prCfaOgm->rCfaOgmVidStream.u8TimeUnit) * ((u64)9 )/ ((u64)1000);
			prCfaOgm->rCurPage.fgParseVideo = TRUE;
			prCfaOgm->rCurPacket.u8LastVidGranule =
				prCfaOgm->rCfaOgmRange.i8VidLastGranule;
		} else if (prCfaOgm->rCfaOgmRange.u8VidStartOfst > prCfaOgm->rCurPage.u8StartOfst) {
			prCfaOgm->rCurPage.fgParseVideo = FALSE;
		} else {
			prCfaOgm->rCurPage.fgFirstVidPage = FALSE;
			prCfaOgm->rCurPage.fgParseVideo = TRUE;
		}


		break;

	case CFA_OGM_PRS_STRM_TYPE_A:
		if (prCfaOgm->rCfaOgmRange.u8AudStartOfst == prCfaOgm->rCurPage.u8StartOfst) {
			prCfaOgm->rCurPage.fgFirstAudPage = TRUE;
			prCfaOgm->rCurPage.fgParseAudio = TRUE;


			if ((CFA_AUD_DRV_FMT_VORBIS !=
				prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec)
				&& (CFA_AUD_DRV_FMT_FLAC !=
				prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec)) {
				if (prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
					u8SamplePerUnit > 0) {
					prCfaOgm->rCurPage.au8AudPtsPerGranule[prCfaOgm->rCurPage.
									uAudIdx] =
						((prCfaOgm->
						arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
						u8TimeUnit) * CFA_STC_CLK) /
						(prCfaOgm->
						arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
						u8SamplePerUnit);
				}
			} else {
				if (prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
					u4AudSampRate > 0) {
					prCfaOgm->rCurPage.au8AudPtsPerGranule[prCfaOgm->rCurPage.
										uAudIdx] =
						1024 * CFA_STC_CLK /
						(prCfaOgm->
						arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
						u4AudSampRate);
				}
			}

			prCfaOgm->rCurPacket.au8LastAudGranule[prCfaOgm->rCurPage.uAudIdx] =
				prCfaOgm->rCfaOgmRange.i8AudLastGranule;

		} else if (prCfaOgm->rCfaOgmRange.u8AudStartOfst > prCfaOgm->rCurPage.u8StartOfst) {
			prCfaOgm->rCurPage.fgParseAudio = FALSE;
		} else {
			prCfaOgm->rCurPage.fgParseAudio = TRUE;
			prCfaOgm->rCurPage.fgFirstAudPage = FALSE;
		}


		if ((prCfaOgm->rCurPage.fgParseAudio)
			&& (!prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].fgFirst)) {
			if ((prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec !=
				CFA_AUD_DRV_FMT_VORBIS)
				&& (prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
				eCfaAudCodec != CFA_AUD_DRV_FMT_FLAC)) {
				if (prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
					u8SamplePerUnit > 0) {
					prCfaOgm->rCurPage.au8AudPtsPerGranule[prCfaOgm->rCurPage.
										uAudIdx] =
						((prCfaOgm->
						arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
						u8TimeUnit) * CFA_STC_CLK) /
						(prCfaOgm->
						arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
						u8SamplePerUnit);
				}
			} else {
				if (prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
					u4AudSampRate > 0) {
					prCfaOgm->rCurPage.au8AudPtsPerGranule[prCfaOgm->rCurPage.
									uAudIdx] =
						1024 * CFA_STC_CLK /
						(prCfaOgm->
						arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
						u4AudSampRate);
				}
			}

		}

		if ((prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec ==
			CFA_AUD_DRV_FMT_VORBIS)
			&& (prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
			fgNeedTxVorbisHeaderData)) {
			DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_SRCH_HDR,
				TEXT
				("[CFA_OGM] CfaOgmAnaPageHeader tx vorbis first page, auido index: %d!\n"),
				prCfaOgm->rCurPage.uAudIdx);

			prCfaOgm->rCurPage.u8DataOfst =
				prCfaOgm->rCurPage.u8StartOfst + CFA_OGM_HDR_READ_BYTES +
				prCfaOgm->rCurPage.uSegmentNs;

			CfaOgmTxVorbisHeader(pvSptHdl, prCfaOgm);

			return;
		}
#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
		if ((CFA_AUD_DRV_FMT_AAC ==
			prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec)
			&& (!prCfaOgm->rCfaOgmAACPacket.fgTxAACStrmHdr)) {
			prCfaOgm->rCurPage.u8DataOfst =
			prCfaOgm->rCurPage.u8StartOfst + CFA_OGM_HDR_READ_BYTES +
			prCfaOgm->rCurPage.uSegmentNs;
			/*prCfaOgm->rCfaOgmAACPacket.u8CurPacketSa =  prCfaOgm->rCurPage.u8StartOfst;//07/16/09*/
			/*prCfaOgm->u8Ca = prCfaOgm->rCurPage.u8DataOfst;//need*/
			/*prCfaOgm->rCurPacket.u8AudStartOfst = prCfaOgm->u8Ca;//13/07/09/delete need*/
			/*prCfaOgm->rCurPacket.u8AudStartOfst = prCfaOgm->rCurPage.u8DataOfst;////07/16/09*/
			prCfaOgm->rCurPacket.u8AudStartOfst = prCfaOgm->u8Ca;	/*07/16/09*/
			CfaOgmTxAACStrmHdr(pvSptHdl, prCfaOgm);
			return;
		}
#endif
		break;

	case CFA_OGM_PRS_STRM_TYPE_SP:
		break;

	default:
		break;
	}

	prCfaOgm->rCurPage.u8DataOfst = prCfaOgm->rCurPage.u8StartOfst + CFA_OGM_HDR_READ_BYTES +
		prCfaOgm->rCurPage.uSegmentNs;
	CfaOgmNextState(pvSptHdl, prCfaOgm, prCfaOgm->rCurPage.uSegmentNs, CFA_OGM_ST_PAGE_LACING_ANA);
}

static VOID CfaOgmTxVorbisPage(void *pvSptHdl, CfaOgmInst * prCfaOgm)
{
	if (prCfaOgm->fgUseCMDQ) {
		MRESULT mrRet = RET_DMX_OK;
		CFA_AUDIO_INFO_T rAudInf;
		u8 uAudIdx = 0;
		bool fgTxDataInstantly = FALSE;
		bool fgTxDataFirst = TRUE;
		u32 u4Cnt = 0;
		
#if CFA_OGM_INTERNAL_DEBUG
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] %s enter \n"), DMX_FUNC_NAME);
#endif
		mm_memset((VOID *)&rAudInf, 0 ,sizeof(rAudInf) );
		
		if (NULL == prCfaOgm) {
			DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] %s line %d,prCfaOgm is NULL! Finish parse!\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			Spt4CfaFinishedEx(pvSptHdl, 0, FALSE, GAU_E_EOS);
			return;
		}

		rAudInf.u4PrsStrmId = prCfaOgm->rCurPage.u4StreamNo;
		//prCfaOgm->rCurPage.uAudIdx = uAudIdx;
		if (prCfaOgm->rCurPage.uAudIdx >= MAX_NS_OGM_AUD) {
			prCfaOgm->rCurPage.uAudIdx = MAX_NS_OGM_AUD - 1;
		}
		
		uAudIdx = prCfaOgm->rCurPage.uAudIdx;
		
		if (DMX_IS_RW_PLAY(pvSptHdl) && (0 == (prCfaOgm->u4PrsFlg & CFA_OGM_PRS_STRM_TYPE_V))
			&& (prCfaOgm->rCurPage.uAudIdx != prCfaOgm->uCurAIndex)) {
			prCfaOgm->rCurPacket.u8AudStartOfst = prCfaOgm->rCurPage.u8DataOfst;
			prCfaOgm->rCurPacket.u8AudLength = prCfaOgm->rCurPage.u8DataLen;
			//CfaOgmTxPacket(pvSptHdl,prCfaOgm->rCurPacket.u8AudLength,prCfaOgm);
			prCfaOgm->eCfaOgmCurState = CFA_OGM_ST_PACKET_TX;
			prCfaOgm->fgNoNeedSyncPb = TRUE;  
			return;
		}
		
		rAudInf.eAudType = prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec;
		rAudInf.u8FileOfst = prCfaOgm->rCurPage.u8StartOfst;
		rAudInf.u8Len = prCfaOgm->rCurPage.u8DataLen + (prCfaOgm->rCurPage.u8DataOfst - prCfaOgm->rCurPage.u8StartOfst);
		if (prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].u4AudSampRate > 0) {
			rAudInf.u8Pts = prCfaOgm->rCurPage.u8LastAudGranulePosition * CFA_STC_CLK / (prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].u4AudSampRate);
			prCfaOgm->rCurPacket.u8AudStartPTS = rAudInf.u8Pts;
		}
		if(rAudInf.u8FileOfst < prCfaOgm->au8AudParsedOfst[prCfaOgm->rCurPage.uAudIdx])
			fgTxDataFirst = FALSE;
		
		if (rAudInf.u8FileOfst + rAudInf.u8Len > prCfaOgm->u8AvalOfst) {
			bool fgTxCmdQ = FALSE;
		
			fgTxCmdQ = CfaOgmTxAvCmdQ(pvSptHdl, prCfaOgm, CFA_OGM_ST_IDLE,CFA_OGM_ST_VORBIS_PAGE_TX);
			if (fgTxCmdQ) {
				return;
			}
		
			fgTxDataInstantly = TRUE;
		}
		else if ((prCfaOgm->arAudCmdQInfo[uAudIdx].u4EntryCnt >= DMX_MAX_TX_CNT_FOR_CMD_Q)
			|| (prCfaOgm->arAudCmdQInfo[uAudIdx].u8TotalLen + rAudInf.u8Len > DMX_VID_TX_MAX_SIZE)
			|| (DMX_IS_RW_PLAY(pvSptHdl) && (0 == (prCfaOgm->u4PrsFlg & CFA_OGM_PRS_STRM_TYPE_V)) 
				 && (prCfaOgm->u4FRAudDataTxLen + prCfaOgm->rCurPacket.u8AudLength > prCfaOgm->u4FRAudAuLen))) {
			PrePareToTxAudCmdQ(prCfaOgm);
			if (prCfaOgm->uTxAudCmdQNs > 0) {
				prCfaOgm->eTmpStateWhenCmdQ = CFA_OGM_ST_VORBIS_PAGE_TX;
				prCfaOgm->uCurCmdQIdx = 0;
				CfaOgmTxAudCmdQ(pvSptHdl,prCfaOgm,0);
		#if CFA_OGM_TMP_DBG
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,TEXT("[CFA_OGM] %s line:%d \n"),
					DMX_FUNC_NAME,DMX_LINE_NO);
		#endif
				return;
			}
			fgTxDataInstantly = TRUE;
		}
		
		if (fgTxDataInstantly) {
			 if (fgTxDataFirst) {
				if (DMX_IS_RW_PLAY(pvSptHdl) && (0 == (prCfaOgm->u4PrsFlg & CFA_OGM_PRS_STRM_TYPE_V))) {
					if (prCfaOgm->fgRWFinish) {
						DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
							TEXT("[CFA_OGM] CfaOgmTxVorbisPage(): u4FRAudDataTxLen: %d, u4FRAudAuLen: %d\r\n"), prCfaOgm->u4FRAudDataTxLen, prCfaOgm->u4FRAudAuLen);
						OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
						return;
					}
			
					if (0 == prCfaOgm->u4FRAudDataTxLen) {
						rAudInf.fgUnitStart = TRUE;
					} else {
						rAudInf.fgUnitStart = FALSE;
					}
			
					prCfaOgm->u4FRAudDataTxLen += (u32)rAudInf.u8Len;
			
					if (prCfaOgm->u4FRAudDataTxLen > prCfaOgm->u4FRAudAuLen) {
						rAudInf.u8Len = rAudInf.u8Len - (u64)(prCfaOgm->u4FRAudDataTxLen - prCfaOgm->u4FRAudAuLen);
						prCfaOgm->u4FRAudDataTxLen = 0;
						prCfaOgm->fgRWFinish = TRUE;
					}
			
					rAudInf.u8TotalAULen = prCfaOgm->u4FRAudAuLen;
					mrRet= Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rAudInf);
					prCfaOgm->au8AudParsedOfst[prCfaOgm->rCurPage.uAudIdx]
						= rAudInf.u8FileOfst + rAudInf.u8Len;		
				} else {
					rAudInf.u8TotalAULen = 0;
					mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl,&rAudInf);
					prCfaOgm->au8AudParsedOfst[prCfaOgm->rCurPage.uAudIdx]
						= rAudInf.u8FileOfst + rAudInf.u8Len;
					Sleep(1);
				}
				if (RET_DMX_OK != mrRet) {
					DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
						TEXT("[CFA_OGM]CfaOgmTxVorbisPage(): Spt4CfaPbb2AFifoAUCtrl error ret = %d \n"), mrRet);
					OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
					DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
						TEXT("[CFA_OGM]Splitter transfer audio data error!\n"));
					return;
				 }
			 }
			 prCfaOgm->rCurPacket.u8AudStartOfst = prCfaOgm->rCurPage.u8DataOfst;
			 prCfaOgm->rCurPacket.u8AudLength = prCfaOgm->rCurPage.u8DataLen;		
			 prCfaOgm->eCfaOgmCurState = CFA_OGM_ST_PACKET_TX;

			 if (!fgTxDataFirst)
			 	prCfaOgm->fgNoNeedSyncPb = TRUE;
			 return;
		}

		if (fgTxDataFirst) {
			//Add CmdQ Entry
			u4Cnt = prCfaOgm->arAudCmdQInfo[uAudIdx].u4EntryCnt;
			if (u4Cnt == 0) {
				prCfaOgm->arCmdQStartPage[uAudIdx] = prCfaOgm->rCurPage;
				prCfaOgm->arCmdQStartPacket[uAudIdx] = prCfaOgm->rCurPacket;
			}
			
			prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[u4Cnt].fgUnitStart = TRUE;
			prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[u4Cnt].u4Len = rAudInf.u8Len;
			prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[u4Cnt].u8FileOffset = rAudInf.u8FileOfst;
			prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[u4Cnt].u8Pts = rAudInf.u8Pts;
			prCfaOgm->arAudCmdQInfo[uAudIdx].u8TotalLen = rAudInf.u8FileOfst + rAudInf.u8Len
				- prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[0].u8FileOffset;
			prCfaOgm->arAudCmdQInfo[uAudIdx].u4EntryCnt++;
			prCfaOgm->arAudCmdQInfo[uAudIdx].u4RealTxLen += rAudInf.u8Len;
			
			if (DMX_IS_RW_PLAY(pvSptHdl) && (0 == (prCfaOgm->u4PrsFlg & CFA_OGM_PRS_STRM_TYPE_V))) {
				if (prCfaOgm->fgRWFinish) {
					DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
						TEXT("[CFA_OGM] CfaOgmTxPacketData() 1: u4FRAudDataTxLen: %d, u4FRAudAuLen: %d\r\n"), prCfaOgm->u4FRAudDataTxLen, prCfaOgm->u4FRAudAuLen);
					OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
					return;
				}
			
				if (0 == prCfaOgm->u4FRAudDataTxLen) {
					prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[u4Cnt].fgUnitStart = TRUE;
				} else {
					prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[u4Cnt].fgUnitStart = FALSE;
				}
			
				prCfaOgm->u4FRAudDataTxLen += (u32)rAudInf.u8Len;
			
				if (prCfaOgm->u4FRAudDataTxLen > prCfaOgm->u4FRAudAuLen) {
					prCfaOgm->fgRWFinish = TRUE;
					prCfaOgm->u4FRAudDataTxLen = 0;
				}
			}
		}
		prCfaOgm->rCurPacket.u8AudStartOfst = prCfaOgm->rCurPage.u8DataOfst;
		prCfaOgm->rCurPacket.u8AudLength = prCfaOgm->rCurPage.u8DataLen;
		prCfaOgm->eCfaOgmCurState = CFA_OGM_ST_PACKET_TX;
		prCfaOgm->fgNoNeedSyncPb = TRUE;	
	} else {
		MRESULT mrRet = RET_DMX_OK;
		CFA_AUDIO_INFO_T rAudInf;

		mm_memset((VOID *)&rAudInf, 0 ,sizeof(rAudInf));

		if (NULL == prCfaOgm) {
			DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] %s line %d,prCfaOgm is NULL! Finish parse!\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			Spt4CfaFinishedEx(pvSptHdl, 0, FALSE, GAU_E_EOS);
			return;
		}

		rAudInf.u4PrsStrmId = prCfaOgm->rCurPage.u4StreamNo;

		if (prCfaOgm->rCurPage.uAudIdx >= MAX_NS_OGM_AUD) {
		prCfaOgm->rCurPage.uAudIdx = MAX_NS_OGM_AUD - 1;
		}

		rAudInf.eAudType = prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec;
		rAudInf.u8FileOfst = prCfaOgm->rCurPage.u8StartOfst;
		rAudInf.u8Len = prCfaOgm->rCurPage.u8DataLen + (prCfaOgm->rCurPage.u8DataOfst - prCfaOgm->rCurPage.u8StartOfst);
		if (prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].u4AudSampRate > 0) {
			rAudInf.u8Pts = prCfaOgm->rCurPage.u8LastAudGranulePosition * CFA_STC_CLK / (prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].u4AudSampRate);
			prCfaOgm->rCurPacket.u8AudStartPTS = rAudInf.u8Pts;
		}

		if ((prCfaOgm->u4AudNum > 1) || 
			(DMX_IS_RW_PLAY(pvSptHdl) && (0 == (prCfaOgm->u4PrsFlg & CFA_OGM_PRS_STRM_TYPE_V)))||
			(prCfaOgm->fgCrossSlot) || 
			((rAudInf.u8FileOfst + rAudInf.u8Len > prCfaOgm->u8AvalOfst) && (0 == prCfaOgm->rAudCmdQInfo.u4EntryCnt))) {
			if (DMX_IS_RW_PLAY(pvSptHdl) && (0 == (prCfaOgm->u4PrsFlg & CFA_OGM_PRS_STRM_TYPE_V))) {
				if (prCfaOgm->fgRWFinish) {
					DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
						TEXT("[CFA_OGM] CfaOgmTxVorbisPage(): u4FRAudDataTxLen: %d, u4FRAudAuLen: %d\r\n"), prCfaOgm->u4FRAudDataTxLen, prCfaOgm->u4FRAudAuLen);
					OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
					return;
				}

				if (0 == prCfaOgm->u4FRAudDataTxLen) {
					rAudInf.fgUnitStart = TRUE;
				} else {
					rAudInf.fgUnitStart = FALSE;
				}

				prCfaOgm->u4FRAudDataTxLen += (u32)rAudInf.u8Len;

				if (prCfaOgm->u4FRAudDataTxLen > prCfaOgm->u4FRAudAuLen) {
				rAudInf.u8Len = rAudInf.u8Len - (u64)(prCfaOgm->u4FRAudDataTxLen - prCfaOgm->u4FRAudAuLen);
					prCfaOgm->u4FRAudDataTxLen = 0;
					prCfaOgm->fgRWFinish = TRUE;
				}

				//RETAILMSG(1, (L"Spt4CfaPbb2AFifoAUCtrl  ......... u4FRAudAuLen: %d\n", prCfaOgm->u4FRAudAuLen));
				rAudInf.u8TotalAULen = prCfaOgm->u4FRAudAuLen;
				mrRet= Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rAudInf);

				} else {
				rAudInf.u8TotalAULen = 0;
				mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl,&rAudInf);
				Sleep(1);
			}
			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("[CFA_OGM]CfaOgmTxVorbisPage(): Spt4CfaPbb2AFifoAUCtrl error ret = %d \n"), mrRet);
				OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("[CFA_OGM]Splitter transfer audio data error!\n"));
				return;
			}
	         
			prCfaOgm->rCurPacket.u8AudStartOfst = prCfaOgm->rCurPage.u8DataOfst;
			prCfaOgm->rCurPacket.u8AudLength = prCfaOgm->rCurPage.u8DataLen;
	        
			prCfaOgm->eCfaOgmCurState = CFA_OGM_ST_PACKET_TX;
			if (prCfaOgm->fgCrossSlot == TRUE) {
				prCfaOgm->fgCrossSlot = FALSE;
			}
			return;
		} else if (rAudInf.u8FileOfst + rAudInf.u8Len > prCfaOgm->u8AvalOfst 
			|| prCfaOgm->rAudCmdQInfo.u4EntryCnt >= DMX_MAX_TX_CNT_FOR_CMD_Q) {
			CfaOgmTxAudCmdQ(pvSptHdl,prCfaOgm,0);
			prCfaOgm->eCfaOgmCurState = CFA_OGM_ST_VORBIS_PAGE_TX;
			if(rAudInf.u8FileOfst + rAudInf.u8Len > prCfaOgm->u8AvalOfst) {
				prCfaOgm->fgCrossSlot = TRUE;
			}
			return;
		} else  {
			/*Add CmdQ Entry*/
			u32 u4Cnt = prCfaOgm->rAudCmdQInfo.u4EntryCnt;
			if (u4Cnt == 0) {
				prCfaOgm->rCmdQStartPage = prCfaOgm->rCurPage;
				prCfaOgm->rCmdQStartPacket = prCfaOgm->rCurPacket;
			}

			prCfaOgm->rAudCmdQInfo.uAudIdx = prCfaOgm->rCurPage.uAudIdx;
			prCfaOgm->rAudCmdQInfo.arEntrys[u4Cnt].fgUnitStart = TRUE;
			prCfaOgm->rAudCmdQInfo.arEntrys[u4Cnt].u4Len = rAudInf.u8Len;
			prCfaOgm->rAudCmdQInfo.arEntrys[u4Cnt].u8FileOffset = rAudInf.u8FileOfst;
			prCfaOgm->rAudCmdQInfo.arEntrys[u4Cnt].u8Pts = rAudInf.u8Pts;
			prCfaOgm->rAudCmdQInfo.u8TotalLen = rAudInf.u8FileOfst + rAudInf.u8Len
				- prCfaOgm->rAudCmdQInfo.arEntrys[0].u8FileOffset;
			prCfaOgm->rAudCmdQInfo.u4EntryCnt++;
			prCfaOgm->rAudCmdQInfo.u4RealTxLen += rAudInf.u8Len;

			prCfaOgm->rCurPacket.u8AudStartOfst = prCfaOgm->rCurPage.u8DataOfst;
			prCfaOgm->rCurPacket.u8AudLength = prCfaOgm->rCurPage.u8DataLen;

			prCfaOgm->eCfaOgmCurState = CFA_OGM_ST_PACKET_TX;
			prCfaOgm->fgNoNeedSyncPb = TRUE;
		}
	}

}

/*-----------------------------------------------------------------------------
 * Name: CfaOgmTxPacketData
 *
 * Description:
 *      After one packet parsed,this function will be called to transfer data to specific FIFO
 *
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: void
 *
 *-----------------------------------------------------------------------------*/
static void CfaOgmTxPacketData(void *pvSptHdl, CfaOgmInst *prCfaOgm)
{
	if (prCfaOgm->fgUseCMDQ) {
		MRESULT mrRet = RET_DMX_OK;
	
		CFA_AUDIO_INFO_T rAudInf;
		CFA_VIDEO_INFO_T rVidInf;
		u8 uAudIdx = prCfaOgm->rCurPage.uAudIdx;
		bool fgTxDataInstantly = FALSE;
		bool fgTxDataFirst = TRUE;
		u32 u4Cnt = 0; 
	
#if CFA_OGM_TMP_DBG
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] %s enter,type:%d, audlen:%d,vidlen:%d \n"),DMX_FUNC_NAME,
			prCfaOgm->rCurPage.eCfaOgmStrmType, prCfaOgm->rCurPacket.u8AudLength, prCfaOgm->rCurPacket.u8VidLength);
#endif
		mm_memset((VOID *)&rAudInf, 0 ,sizeof(rAudInf) );
		mm_memset((VOID *)&rVidInf, 0 ,sizeof(rVidInf) );

		if (NULL == prCfaOgm) {
			DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] %s line %d,prCfaOgm is NULL! Finish parse!\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			Spt4CfaFinishedEx(pvSptHdl, 0, FALSE, GAU_E_EOS);
			return;
		}

		prCfaOgm->eCfaOgmCurState = CFA_OGM_ST_PACKET_TX;
		
		prCfaOgm->eCfaOgmCurPrsStrm = prCfaOgm->rCurPage.eCfaOgmStrmType;
	
		if (prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_A) {
			prCfaOgm->rCurPacket.uAudPacketNo++;
			if (prCfaOgm->rCurPacket.u8AudLength == 0) {
	
#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
				if (CFA_AUD_DRV_FMT_AAC == prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec) {
					CfaOgmTxAACPacketData(pvSptHdl, prCfaOgm);
					return;
				}
#endif
				CfaOgmNextState(pvSptHdl,prCfaOgm,CFA_OGM_PACKET_HDR_READ,CFA_OGM_ST_PACKET_HDR_ANA);
				return;
			}
		} else {
			prCfaOgm->rCurPacket.uVidPacketNo++;
			if (prCfaOgm->rCurPacket.u8VidLength == 0) {
				CfaOgmNextState(pvSptHdl,prCfaOgm,CFA_OGM_PACKET_HDR_READ,CFA_OGM_ST_PACKET_HDR_ANA);
				return;
			}
		}
	
		switch(prCfaOgm->rCurPage.eCfaOgmStrmType) {
		case  CFA_OGM_PRS_STRM_TYPE_V:
#if CFA_OGM_TMP_DBG
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] %s line:%d, Vid.u8FileOfst:0x%llx, Vid.u8Len:%llx,ca:0x%llx\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prCfaOgm->rCurPacket.u8VidStartOfst, prCfaOgm->rCurPacket.u8VidLength, prCfaOgm->u8Ca);
#endif
		//mtk40301 fix bug 122452
		if (  ((prCfaOgm->rCurPacket.u8VidStartOfst + prCfaOgm->rCurPacket.u8VidLength) > prCfaOgm->rCfaOgmRange.u8VidEndOfst)
			|| ((prCfaOgm->rCurPacket.u8VidStartOfst + prCfaOgm->rCurPacket.u8VidLength < prCfaOgm->rCfaOgmRange.u8VidEndOfst)
			&&  (prCfaOgm->rCurPacket.u8VidStartOfst + prCfaOgm->rCurPacket.u8VidLength + 28 >= prCfaOgm->rCfaOgmRange.u8VidEndOfst))) {
			prCfaOgm->rCurPacket.u8VidLength = prCfaOgm->rCfaOgmRange.u8VidEndOfst - prCfaOgm->rCurPacket.u8VidStartOfst;
			#if CFA_OGM_TMP_DBG
			DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] %s line:%d, rVidInf.u8FileOfst:0x%llx, rVidInf.u8Len:%llx,ca:0x%llx\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prCfaOgm->rCurPacket.u8VidStartOfst, prCfaOgm->rCurPacket.u8VidLength, prCfaOgm->u8Ca);
			#endif
		}

		rVidInf.eVidType = prCfaOgm->rCfaOgmVidStream.eCfaVidCodec;
		rVidInf.u8FileOfst = prCfaOgm->rCurPacket.u8VidStartOfst;
		rVidInf.u8Len = prCfaOgm->rCurPacket.u8VidLength;
		rVidInf.u4PrsStrmId = prCfaOgm->rCurPage.u4StreamNo;
#if CFA_OGM_TMP_DBG
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] %s line:%d, rVidInf.u8FileOfst:0x%llx, rVidInf.u8Len:%llx,ca:0x%llx\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, rVidInf.u8FileOfst, rVidInf.u8Len, prCfaOgm->u8Ca);
#endif

		if (rVidInf.u8FileOfst < prCfaOgm->u8VidParsedOfst)
			fgTxDataFirst = FALSE;

		if ((rVidInf.u8FileOfst + rVidInf.u8Len > prCfaOgm->u8AvalOfst)) {
			bool fgTxCmdQ = FALSE;
  #if CFA_OGM_TMP_DBG
			  DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("[CFA_OGM] %s line:%d, rVidInf.u8FileOfst:0x%llx, rVidInf.u8Len:%llx,prCfaOgm->u8AvalOfst:0x%llx\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, rVidInf.u8FileOfst, rVidInf.u8Len, prCfaOgm->u8AvalOfst);
  #endif	
			fgTxCmdQ = CfaOgmTxAvCmdQ(pvSptHdl,prCfaOgm,CFA_OGM_ST_IDLE,CFA_OGM_ST_PACKET_DATA_TX);
			if (fgTxCmdQ) {
				prCfaOgm->rCurPacket.uVidPacketNo--;
				return;
			}
	
			fgTxDataInstantly = TRUE;
		} else if ((prCfaOgm->rVidCmdQInfo.u4EntryCnt >= DMX_MAX_TX_CNT_FOR_CMD_Q)
				  || (prCfaOgm->rVidCmdQInfo.u8TotalLen + rVidInf.u8Len > DMX_VID_TX_MAX_SIZE)) {
			 if (prCfaOgm->rVidCmdQInfo.u4EntryCnt > 0) {
				prCfaOgm->eTmpStateWhenCmdQ = CFA_OGM_ST_PACKET_DATA_TX;
				prCfaOgm->rCurPacket.uVidPacketNo--;
				CfaOgmTxVidCmdQ(pvSptHdl,prCfaOgm,0,FALSE);
				return;
			 }
			 fgTxDataInstantly = TRUE;
		}

		if (fgTxDataInstantly) {
			if (fgTxDataFirst) {
	 #if CONFIG_CFA_OGM_DIVX3_AU_SUPPORT
				 rVidInf.fgAUCompleteByEnd = TRUE;
		
				 if ((!prCfaOgm->fgPageContainEnd) &&
					(prCfaOgm->rCurPacket.uVidPacketNo == prCfaOgm->rCurPage.uPacketNs)) {
					rVidInf.fgUnitEnd = FALSE;
				 } else {
					  rVidInf.fgUnitEnd = TRUE;
				 }
		
				 if (prCfaOgm->fgUnitStart) {
					rVidInf.fgUnitStart = TRUE;
				 }
	#endif
		
				 if (prCfaOgm->rCfaOgmVidStream.eCfaVidCodec == CFA_VID_DIVX3) {
					rVidInf.eTxMode = prCfaOgm->eCfaOgmTxMode;
				 }
		
	  #if CFA_OGM_TMP_DBG
				  DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
						TEXT("[CFA_OGM] %s line:%d \n"),DMX_FUNC_NAME,DMX_LINE_NO);
	  #endif
		
				 mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl,&rVidInf);
	  			 prCfaOgm->u8VidParsedOfst = rVidInf.u8FileOfst + rVidInf.u8Len;
				 if (RET_DMX_OK != mrRet) {
					DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
						TEXT("[CFA_OGM] line %d Spt4CfaPbb2VFifoAUCtrl error ret = %d \n"), 
						DMX_LINE_NO, mrRet);
					OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
					DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
							TEXT("[CFA_OGM]Splitter transfer video data error!\n"));
					return;
				 }
		
#if CONFIG_CFA_OGM_DIVX3_AU_SUPPORT
				prCfaOgm->fgUnitStart = FALSE;
#endif
				return;
			}	
			prCfaOgm->fgNoNeedSyncPb = TRUE;
			return;	
		}

		if (fgTxDataFirst) {
			/*Add Video CmdQ Entry*/
			u4Cnt = prCfaOgm->rVidCmdQInfo.u4EntryCnt;
			if (0 == u4Cnt) {
				prCfaOgm->rVidCmdQStartPage = prCfaOgm->rCurPage;
				prCfaOgm->rVidCmdQStartPacket = prCfaOgm->rCurPacket;
			}
			if(CFA_VID_DIVX3 == prCfaOgm->rCfaOgmVidStream.eCfaVidCodec) {
				if ((!prCfaOgm->fgPageContainEnd) &&
					(prCfaOgm->rCurPacket.uVidPacketNo == prCfaOgm->rCurPage.uPacketNs)) {
					prCfaOgm->rVidCmdQInfo.arEntrys[u4Cnt].fgUnitEnd = FALSE;
				} else {
					prCfaOgm->rVidCmdQInfo.arEntrys[u4Cnt].fgUnitEnd = TRUE;
				}
		
				if (prCfaOgm->fgUnitStart) {
					prCfaOgm->rVidCmdQInfo.arEntrys[u4Cnt].fgUnitStart = TRUE;
				}
				prCfaOgm->rVidCmdQInfo.arEntrys[u4Cnt].eTxMode = prCfaOgm->eCfaOgmTxMode;
			}
			prCfaOgm->rVidCmdQInfo.arEntrys[u4Cnt].u4Len = rVidInf.u8Len;
			prCfaOgm->rVidCmdQInfo.arEntrys[u4Cnt].u8FileOffset = rVidInf.u8FileOfst;
			prCfaOgm->rVidCmdQInfo.arEntrys[u4Cnt].u8Pts = prCfaOgm->rCurPacket.u8VidStartPTS;
			prCfaOgm->rVidCmdQInfo.u8TotalLen = rVidInf.u8FileOfst + rVidInf.u8Len - prCfaOgm->rVidCmdQInfo.arEntrys[0].u8FileOffset;
			prCfaOgm->rVidCmdQInfo.u4EntryCnt++;
			prCfaOgm->rVidCmdQInfo.u4RealTxLen += rVidInf.u8Len;
		
			if (CFA_VID_DIVX3 == prCfaOgm->rCfaOgmVidStream.eCfaVidCodec) {
				prCfaOgm->fgUnitStart = FALSE;
			}
		}
	
		prCfaOgm->fgNoNeedSyncPb = TRUE;
		
		break;
	
		case CFA_OGM_PRS_STRM_TYPE_A:
	
			if (DMX_IS_RW_PLAY(pvSptHdl) && (0 == (prCfaOgm->u4PrsFlg & CFA_OGM_PRS_STRM_TYPE_V))
				&& (prCfaOgm->rCurPage.uAudIdx != prCfaOgm->uCurAIndex)) {
				CfaOgmTxPacket(pvSptHdl,prCfaOgm->rCurPacket.u8AudLength,prCfaOgm);
				return;
			}
	
			//mtk40301 fix bug 122452
			if ( (prCfaOgm->rCurPacket.u8AudStartOfst + prCfaOgm->rCurPacket.u8AudLength > prCfaOgm->rCfaOgmRange.u8AudEndOfst)
				|| ((prCfaOgm->rCurPacket.u8AudStartOfst + prCfaOgm->rCurPacket.u8AudLength < prCfaOgm->rCfaOgmRange.u8AudEndOfst)
				&& (prCfaOgm->rCurPacket.u8AudStartOfst + prCfaOgm->rCurPacket.u8AudLength + 28 >= prCfaOgm->rCfaOgmRange.u8AudEndOfst))) {
				prCfaOgm->rCurPacket.u8AudLength = prCfaOgm->rCfaOgmRange.u8AudEndOfst - prCfaOgm->rCurPacket.u8AudStartOfst;
			}
	
			 rAudInf.u4PrsStrmId = prCfaOgm->rCurPage.u4StreamNo;
			 if (prCfaOgm->rCurPage.uAudIdx >= MAX_NS_OGM_AUD) {
				prCfaOgm->rCurPage.uAudIdx = MAX_NS_OGM_AUD - 1;
			 }
			 rAudInf.eAudType = prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec;
			 rAudInf.u8FileOfst = prCfaOgm->rCurPacket.u8AudStartOfst;
			 rAudInf.u8Len = prCfaOgm->rCurPacket.u8AudLength;
			 rAudInf.u8Pts = prCfaOgm->rCurPacket.u8AudStartPTS;
			 rAudInf.ptrFromFileAddress = prCfaOgm->ptrMemAddr;
#if CFA_OGM_TMP_DBG
			DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] %s line:%d, rAudInf.u8FileOfst:0x%llx, rAudInf.u8Len:%llx,ca:0x%llx\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, rAudInf.u8FileOfst, rAudInf.u8Len, prCfaOgm->u8Ca);
#endif

			 if (rAudInf.u8FileOfst < prCfaOgm->au8AudParsedOfst[prCfaOgm->rCurPage.uAudIdx]) {
				fgTxDataFirst = FALSE;
			 }
 #if CFA_OGM_TMP_DBG
			 DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] %s AudStartOfst:%lld,len:%lld \n"),
				DMX_FUNC_NAME,rAudInf.u8FileOfst,rAudInf.u8Len);
 #endif  
	
			if (prCfaOgm->rCurPacket.u8AudStartOfst + prCfaOgm->rCurPacket.u8AudLength > prCfaOgm->u8AvalOfst) {
				bool fgTxCmdQ = FALSE;
			
    #if CFA_OGM_TMP_DBG
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("[CFA_OGM] %s line:%d, AudStartOfst:0x%llx, AudLength:0x%llx, AvalOfst:0x%llx\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prCfaOgm->rCurPacket.u8AudStartOfst,
					prCfaOgm->rCurPacket.u8AudLength, prCfaOgm->u8AvalOfst);
    #endif
				
				fgTxCmdQ = CfaOgmTxAvCmdQ(pvSptHdl, prCfaOgm, CFA_OGM_ST_PACKET_TX,CFA_OGM_ST_PACKET_DATA_TX);
				if (fgTxCmdQ) {
					prCfaOgm->rCurPacket.uAudPacketNo--;
					return;
				}
	
				fgTxDataInstantly = TRUE;
			} else if ((prCfaOgm->arAudCmdQInfo[uAudIdx].u4EntryCnt >= DMX_MAX_TX_CNT_FOR_CMD_Q)
					|| (prCfaOgm->arAudCmdQInfo[uAudIdx].u8TotalLen + rAudInf.u8Len > DMX_VID_TX_MAX_SIZE)
					|| (DMX_IS_RW_PLAY(pvSptHdl) && (0 == (prCfaOgm->u4PrsFlg & CFA_OGM_PRS_STRM_TYPE_V)) 
						&& (prCfaOgm->u4FRAudDataTxLen + prCfaOgm->rCurPacket.u8AudLength > prCfaOgm->u4FRAudAuLen))) {
    #if CFA_OGM_TMP_DBG
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("[CFA_OGM] %s line:%d \n"),DMX_FUNC_NAME,DMX_LINE_NO);
    #endif
				PrePareToTxAudCmdQ(prCfaOgm);
				if(prCfaOgm->uTxAudCmdQNs > 0)
				{
					prCfaOgm->eCfaOgmNextState = CFA_OGM_ST_PACKET_TX;
					prCfaOgm->eTmpStateWhenCmdQ = CFA_OGM_ST_PACKET_DATA_TX;
					prCfaOgm->uCurCmdQIdx = 0;
					prCfaOgm->rCurPacket.uAudPacketNo--;
					CfaOgmTxAudCmdQ(pvSptHdl,prCfaOgm,0);
        #if CFA_OGM_TMP_DBG
					DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
						TEXT("[CFA_OGM] %s line:%d \n"),DMX_FUNC_NAME,DMX_LINE_NO);
        #endif
					return;
				}
				fgTxDataInstantly = TRUE;
			}
	
			if (fgTxDataInstantly) {
				if (fgTxDataFirst) {
	     #if CFA_OGM_TMP_DBG
					 DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
						TEXT("[CFA_OGM] %s line:%d \n"),DMX_FUNC_NAME,DMX_LINE_NO);
	     #endif
					 if (rAudInf.eAudType == CFA_AUD_DRV_FMT_PCM) {
	    #if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
						if(NULL == prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].auPcmHeader) {
							DMX_NewHwMemory((u32)rAudInf.u8Len, prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].auPcmHeader);
							if (NULL == prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].auPcmHeader) {
								DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
									TEXT("[CFA_OGM] alloc memory fail \n"));
								OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
								return;
							}
		
							dmx_memcpy(prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].auPcmHeader,
								(void *)(rAudInf.ptrFromFileAddress), (u32)rAudInf.u8Len);
							prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].uPcmHeaderLen = 
								(u32)rAudInf.u8Len;
							prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].u8FileOfst = rAudInf.u8FileOfst;
						}
	    #endif
		
						if (DMX_IS_RW_PLAY(pvSptHdl) && (0 == (prCfaOgm->u4PrsFlg & CFA_OGM_PRS_STRM_TYPE_V))) {
							if (prCfaOgm->fgRWFinish) {
								DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
									TEXT("[CFA_OGM] CfaOgmTxPacketData(): u4FRAudDataTxLen: %d, u4FRAudAuLen: %d\r\n"), prCfaOgm->u4FRAudDataTxLen, prCfaOgm->u4FRAudAuLen);
								OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
								return;
							}
		
							if (0 == prCfaOgm->u4FRAudDataTxLen) {
								rAudInf.fgUnitStart = TRUE;
							} else {
								rAudInf.fgUnitStart = FALSE;
							}
		
							prCfaOgm->u4FRAudDataTxLen += (u32)rAudInf.u8Len;
		
							if (prCfaOgm->u4FRAudDataTxLen > prCfaOgm->u4FRAudAuLen) {
								prCfaOgm->fgRWFinish = TRUE;
								rAudInf.u8Len = rAudInf.u8Len - (u64)(prCfaOgm->u4FRAudDataTxLen - prCfaOgm->u4FRAudAuLen);
								prCfaOgm->u4FRAudDataTxLen = 0;
							}
							rAudInf.u8TotalAULen = prCfaOgm->u4FRAudAuLen;
							mrRet= Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rAudInf);
							prCfaOgm->au8AudParsedOfst[prCfaOgm->rCurPage.uAudIdx]
								= rAudInf.u8FileOfst + rAudInf.u8Len;
						} else {
							rAudInf.u8TotalAULen = 0;
							mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl,&rAudInf);
							prCfaOgm->au8AudParsedOfst[prCfaOgm->rCurPage.uAudIdx]
								= rAudInf.u8FileOfst + rAudInf.u8Len;
							Sleep(1);
						}
						if (RET_DMX_OK != mrRet) {
							DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,TEXT("[CFA_OGM] Spt4CfaPbb2AFifoAUCtrl error ret = %d \n"), mrRet);
							OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
		
							return;
						}
					} else {
						if (prCfaOgm->rCurPacket.u8AudStartOfst >= prCfaOgm->rCfaOgmRange.u8AudRangeOfst) {
		
							if (DMX_IS_RW_PLAY(pvSptHdl) && (0 == (prCfaOgm->u4PrsFlg & CFA_OGM_PRS_STRM_TYPE_V))) {
								if (prCfaOgm->fgRWFinish) {
									DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
										TEXT("[CFA_OGM] CfaOgmTxPacketData() 1: u4FRAudDataTxLen: %d, u4FRAudAuLen: %d\r\n"), prCfaOgm->u4FRAudDataTxLen, prCfaOgm->u4FRAudAuLen);
									OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
									return;
								}
		
								if (0 == prCfaOgm->u4FRAudDataTxLen) {
									rAudInf.fgUnitStart = TRUE;
								} else {
									rAudInf.fgUnitStart = FALSE;
								}
		
								prCfaOgm->u4FRAudDataTxLen += (u32)rAudInf.u8Len;
		
								if (prCfaOgm->u4FRAudDataTxLen > prCfaOgm->u4FRAudAuLen) {
									rAudInf.u8Len = rAudInf.u8Len - (u64)(prCfaOgm->u4FRAudDataTxLen - prCfaOgm->u4FRAudAuLen);
									prCfaOgm->fgRWFinish = TRUE;
									prCfaOgm->u4FRAudDataTxLen = 0;
								}
								
								rAudInf.u8TotalAULen = prCfaOgm->u4FRAudAuLen;
								mrRet= Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rAudInf);
								prCfaOgm->au8AudParsedOfst[prCfaOgm->rCurPage.uAudIdx]
									= rAudInf.u8FileOfst + rAudInf.u8Len;
							} else {
								rAudInf.u8TotalAULen = 0;
								mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl,&rAudInf);
								prCfaOgm->au8AudParsedOfst[prCfaOgm->rCurPage.uAudIdx]
									= rAudInf.u8FileOfst + rAudInf.u8Len;
								Sleep(1);
							}
		
							if (RET_DMX_OK != mrRet) {
								 DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
									TEXT("[CFA_OGM] Spt4CfaPbb2AFifoAUCtrl error ret = %d \n"), mrRet);
								 OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
								 return;
							}
						} else {
							CfaOgmTxPacket(pvSptHdl,prCfaOgm->rCurPacket.u8AudLength,prCfaOgm);
						}
					}
				} else {
					prCfaOgm->fgNoNeedSyncPb = TRUE;
				}
				return;
			}
	
 #if CFA_OGM_TMP_DBG
			 DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] %s line:%d \n"),DMX_FUNC_NAME,DMX_LINE_NO);
 #endif
 			 if (fgTxDataFirst) {
				 //Add CmdQ Entry
				 u4Cnt = prCfaOgm->arAudCmdQInfo[uAudIdx].u4EntryCnt;
				 if (u4Cnt == 0) {
					 prCfaOgm->arCmdQStartPage[uAudIdx] = prCfaOgm->rCurPage;
					 prCfaOgm->arCmdQStartPacket[uAudIdx] = prCfaOgm->rCurPacket;
				 }
				 prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[u4Cnt].fgUnitStart = TRUE;
				 prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[u4Cnt].u4Len = rAudInf.u8Len;
				 prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[u4Cnt].u8FileOffset = rAudInf.u8FileOfst;
				 prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[u4Cnt].u8Pts = rAudInf.u8Pts;
				 prCfaOgm->arAudCmdQInfo[uAudIdx].u8TotalLen = rAudInf.u8FileOfst + rAudInf.u8Len
					 - prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[0].u8FileOffset;
				 prCfaOgm->arAudCmdQInfo[uAudIdx].u4EntryCnt++;
				 prCfaOgm->arAudCmdQInfo[uAudIdx].u4RealTxLen += rAudInf.u8Len;
				 
				if (DMX_IS_RW_PLAY(pvSptHdl) && (0 == (prCfaOgm->u4PrsFlg & CFA_OGM_PRS_STRM_TYPE_V))) {
					if (prCfaOgm->fgRWFinish) {
						DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
							TEXT("[CFA_OGM] CfaOgmTxPacketData() 1: u4FRAudDataTxLen: %d, u4FRAudAuLen: %d\r\n"), prCfaOgm->u4FRAudDataTxLen, prCfaOgm->u4FRAudAuLen);
						OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
						return;
					}
		
					if (0 == prCfaOgm->u4FRAudDataTxLen) {
						prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[u4Cnt].fgUnitStart = TRUE;
					} else {
						prCfaOgm->arAudCmdQInfo[uAudIdx].arEntrys[u4Cnt].fgUnitStart = FALSE;
					}
		
					prCfaOgm->u4FRAudDataTxLen += (u32)rAudInf.u8Len;
		
					if (prCfaOgm->u4FRAudDataTxLen > prCfaOgm->u4FRAudAuLen) {
						prCfaOgm->fgRWFinish = TRUE;
						prCfaOgm->u4FRAudDataTxLen = 0;
					}
				}
				 
				if (rAudInf.eAudType == CFA_AUD_DRV_FMT_PCM) {
#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
					if(NULL == prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].auPcmHeader) {
						DMX_NewHwMemory((u32)rAudInf.u8Len, prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].auPcmHeader);
						if (NULL == prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].auPcmHeader) {
							DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
								TEXT("[CFA_OGM] alloc memory fail \n"));
							OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
							return;
						}
		
						dmx_memcpy(prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].auPcmHeader,
							(void *)(rAudInf.ptrFromFileAddress), (u32)rAudInf.u8Len);
						prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].uPcmHeaderLen = (u32)rAudInf.u8Len;
						prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].u8FileOfst = rAudInf.u8FileOfst;
					}
#endif
				}
 			}
			prCfaOgm->fgNoNeedSyncPb = TRUE;			
			break;
	
		default:
		break;
	
		}
	} else {
		MRESULT mrRet = RET_DMX_OK;

		CFA_AUDIO_INFO_T rAudInf;
		CFA_VIDEO_INFO_T rVidInf;

		mm_memset((void *) &rAudInf, 0, sizeof(rAudInf));
		mm_memset((void *) &rVidInf, 0, sizeof(rVidInf));

		prCfaOgm->eCfaOgmCurState = CFA_OGM_ST_PACKET_TX;

		prCfaOgm->eCfaOgmCurPrsStrm = prCfaOgm->rCurPage.eCfaOgmStrmType;

		if (prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_A) {
			prCfaOgm->rCurPacket.uAudPacketNo++;
			if (prCfaOgm->rCurPacket.u8AudLength == 0) {

#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
				if (CFA_AUD_DRV_FMT_AAC ==
					prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec) {
					/*if this packet is AAC*/
					CfaOgmTxAACPacketData(pvSptHdl, prCfaOgm);
					return;
				}
#endif
				CfaOgmNextState(pvSptHdl, prCfaOgm, (u64)CFA_OGM_PACKET_HDR_READ,
						CFA_OGM_ST_PACKET_HDR_ANA);
				return;
			}
		} else {
			prCfaOgm->rCurPacket.uVidPacketNo++;
			if (prCfaOgm->rCurPacket.u8VidLength == 0) {
				CfaOgmNextState(pvSptHdl, prCfaOgm, (u64)CFA_OGM_PACKET_HDR_READ,
						CFA_OGM_ST_PACKET_HDR_ANA);
				return;
			}
		}

		switch (prCfaOgm->rCurPage.eCfaOgmStrmType) {
		case CFA_OGM_PRS_STRM_TYPE_V:

			/*mtk40301 fix bug 122452*/
			if (((prCfaOgm->rCurPacket.u8VidStartOfst + prCfaOgm->rCurPacket.u8VidLength) >
				prCfaOgm->rCfaOgmRange.u8VidEndOfst)
				||
				((prCfaOgm->rCurPacket.u8VidStartOfst + prCfaOgm->rCurPacket.u8VidLength <
				prCfaOgm->rCfaOgmRange.u8VidEndOfst)
				&& (prCfaOgm->rCurPacket.u8VidStartOfst + prCfaOgm->rCurPacket.u8VidLength +
				28 >= prCfaOgm->rCfaOgmRange.u8VidEndOfst))) {
				prCfaOgm->rCurPacket.u8VidLength =
					prCfaOgm->rCfaOgmRange.u8VidEndOfst -
					prCfaOgm->rCurPacket.u8VidStartOfst;
			}

			rVidInf.eVidType = prCfaOgm->rCfaOgmVidStream.eCfaVidCodec;
			rVidInf.u8FileOfst = prCfaOgm->rCurPacket.u8VidStartOfst;
			rVidInf.u8Len = prCfaOgm->rCurPacket.u8VidLength;
			rVidInf.u4PrsStrmId = prCfaOgm->rCurPage.u4StreamNo;

#if CONFIG_CFA_OGM_DIVX3_AU_SUPPORT
			rVidInf.fgAUCompleteByEnd = TRUE;

			if ((prCfaOgm->fgPageContainEnd == FALSE) &&
				(prCfaOgm->rCurPacket.uVidPacketNo == prCfaOgm->rCurPage.uPacketNs))
				rVidInf.fgUnitEnd = FALSE;
			else
				rVidInf.fgUnitEnd = TRUE;

			if (prCfaOgm->fgUnitStart == TRUE)
				rVidInf.fgUnitStart = TRUE;
#endif

			if (prCfaOgm->rCfaOgmVidStream.eCfaVidCodec == CFA_VID_DIVX3)
				rVidInf.eTxMode = prCfaOgm->eCfaOgmTxMode;

			mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rVidInf);
			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("[CFA_OGM] Spt4CfaPbb2VFifoAUCtrl error ret = %d\n"),
					mrRet);
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("[CFA_OGM]Splitter transfer video data error!\n"));
				OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);
				return;
			}
#if CONFIG_CFA_OGM_DIVX3_AU_SUPPORT
			prCfaOgm->fgUnitStart = FALSE;
#endif

			break;

		case CFA_OGM_PRS_STRM_TYPE_A:

			/*mtk40301 fix bug 122452*/
			if ((prCfaOgm->rCurPacket.u8AudStartOfst + prCfaOgm->rCurPacket.u8AudLength >
				prCfaOgm->rCfaOgmRange.u8AudEndOfst)
				||
				((prCfaOgm->rCurPacket.u8AudStartOfst + prCfaOgm->rCurPacket.u8AudLength <
				prCfaOgm->rCfaOgmRange.u8AudEndOfst)
				&& (prCfaOgm->rCurPacket.u8AudStartOfst + prCfaOgm->rCurPacket.u8AudLength +
				28 >= prCfaOgm->rCfaOgmRange.u8AudEndOfst))){
				prCfaOgm->rCurPacket.u8AudLength =
					prCfaOgm->rCfaOgmRange.u8AudEndOfst -
					prCfaOgm->rCurPacket.u8AudStartOfst;
			}

			rAudInf.u4PrsStrmId = prCfaOgm->rCurPage.u4StreamNo;
			if (prCfaOgm->rCurPage.uAudIdx >= MAX_NS_OGM_AUD) {
			prCfaOgm->rCurPage.uAudIdx = MAX_NS_OGM_AUD - 1;
			}

			rAudInf.eAudType =
				prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec;
			rAudInf.u8FileOfst = prCfaOgm->rCurPacket.u8AudStartOfst;
			rAudInf.u8Len = prCfaOgm->rCurPacket.u8AudLength;
			rAudInf.u8Pts = prCfaOgm->rCurPacket.u8AudStartPTS;
			rAudInf.ptrFromFileAddress = prCfaOgm->ptrMemAddr;

			if (rAudInf.eAudType == CFA_AUD_DRV_FMT_PCM) {
#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
				if (NULL ==
					prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].auPcmHeader) {
					DMX_NewHwMemory((u32) rAudInf.u8Len,
							prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].auPcmHeader);
					if (NULL ==
						prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
						auPcmHeader) {
						DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
						TEXT("[CFA_OGM] alloc memory fail\n"));
						OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);
						return;
					}

					dmx_memcpy(prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].auPcmHeader,
						(void *)(rAudInf.ptrFromFileAddress),
						(u32) rAudInf.u8Len);
					prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].uPcmHeaderLen = 
						(u32) rAudInf.u8Len;
					prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].u8FileOfst =
						rAudInf.u8FileOfst;
				}
#endif

				if (DMX_IS_RW_PLAY(pvSptHdl)
					&& (0 == (prCfaOgm->u4PrsFlg & CFA_OGM_PRS_STRM_TYPE_V))) {
					if (prCfaOgm->fgRWFinish) {
						DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
							TEXT("[CFA_OGM] CfaOgmTxPacketData():")
							TEXT(" u4FRAudDataTxLen: %d, u4FRAudAuLen: %d\r\n"),
							prCfaOgm->u4FRAudDataTxLen,
							prCfaOgm->u4FRAudAuLen);
						OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);
						return;
					}

					if (0 == prCfaOgm->u4FRAudDataTxLen)
						rAudInf.fgUnitStart = TRUE;
					else
						rAudInf.fgUnitStart = FALSE;

					prCfaOgm->u4FRAudDataTxLen += (u32) rAudInf.u8Len;

					if (prCfaOgm->u4FRAudDataTxLen > prCfaOgm->u4FRAudAuLen) {
						prCfaOgm->fgRWFinish = TRUE;
						rAudInf.u8Len =
							rAudInf.u8Len - (u64) (prCfaOgm->u4FRAudDataTxLen -
							prCfaOgm->u4FRAudAuLen);
						prCfaOgm->u4FRAudDataTxLen = 0;
					}
					rAudInf.u8TotalAULen = prCfaOgm->u4FRAudAuLen;
					mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rAudInf);
				} else {
					rAudInf.u8TotalAULen = 0;
					mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rAudInf);
					Sleep((u32)1);
				}
				if (RET_DMX_OK != mrRet) {
					DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
						TEXT("[CFA_OGM] Spt4CfaPbb2AFifoAUCtrl error ret = %d\n"),
						mrRet);
					OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);

					return;
				}
			} else {
				if (prCfaOgm->rCurPacket.u8AudStartOfst >=
					prCfaOgm->rCfaOgmRange.u8AudRangeOfst) {

					if (DMX_IS_RW_PLAY(pvSptHdl)
						&& (0 == (prCfaOgm->u4PrsFlg & CFA_OGM_PRS_STRM_TYPE_V))) {
						if (prCfaOgm->fgRWFinish) {
							DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
								TEXT("[CFA_OGM] CfaOgmTxPacketData() 1: ")
									TEXT("u4FRAudDataTxLen: %d, u4FRAudAuLen: %d\r\n"),
									prCfaOgm->u4FRAudDataTxLen,
									prCfaOgm->u4FRAudAuLen);
							OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);
							return;
						}

						if (0 == prCfaOgm->u4FRAudDataTxLen)
							rAudInf.fgUnitStart = TRUE;
						else
							rAudInf.fgUnitStart = FALSE;

						prCfaOgm->u4FRAudDataTxLen += (u32) rAudInf.u8Len;

						if (prCfaOgm->u4FRAudDataTxLen > prCfaOgm->u4FRAudAuLen) {
							rAudInf.u8Len =
								rAudInf.u8Len -
								(u64) (prCfaOgm->u4FRAudDataTxLen -
								prCfaOgm->u4FRAudAuLen);
							prCfaOgm->fgRWFinish = TRUE;
							prCfaOgm->u4FRAudDataTxLen = 0;
						}

						rAudInf.u8TotalAULen = prCfaOgm->u4FRAudAuLen;
						mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rAudInf);
					} else {
						rAudInf.u8TotalAULen = 0;
						mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rAudInf);
						Sleep((u32)1);
					}

					if (RET_DMX_OK != mrRet) {
						DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
							TEXT("[CFA_OGM] Spt4CfaPbb2AFifoAUCtrl error ret = %d\n"),
							mrRet);
						OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);
						return;
					}
				} else {
					CfaOgmTxPacket(pvSptHdl, prCfaOgm->rCurPacket.u8AudLength, prCfaOgm);
				}
			}
			break;

		default:
			break;

		}
	}
}



static void CfaOgmAnaLacing(void *pvSptHdl, u64 u8TxLen, CfaOgmInst *prCfaOgm)
{
	u32 u4Index = 0;
	u8 uSkip = 0;
	u8 uPacketNo = 0;

	prCfaOgm->rCurPage.puSegmentTable = (u8 *) (prCfaOgm->ptrMemAddr);

	if (NULL == prCfaOgm->puOgmHdr) {
		DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] %s line %d,prCfaOgm->puOgmHdr is NULL! Finish parse!\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		Spt4CfaFinishedEx(pvSptHdl, 0, FALSE, GAU_E_EOS);
		return;
	}

	prCfaOgm->rCurPage.uPacketNs = 1;	/*any page will has one packet(maybe not complete) at least */

#if CONFIG_CFA_OGM_DIVX3_AU_SUPPORT
	/*check the size of the last segment , if it is equal as 0xff, the last packet is not end*/
	if (prCfaOgm->rCurPage.puSegmentTable[prCfaOgm->rCurPage.uSegmentNs - 1] ==
		DMX_INVALID_UINT8) {
		prCfaOgm->fgPageContainEnd = FALSE;
	} else {
		prCfaOgm->fgPageContainEnd = TRUE;
	}
#endif

	/*get data offset and length and packet numbers for this whole page */
	for (u4Index = (u32)0, uPacketNo = (u8)0; u4Index < (u32)prCfaOgm->rCurPage.uSegmentNs; u4Index++) {
		prCfaOgm->rCurPage.u8DataLen += (prCfaOgm->rCurPage.puSegmentTable[u4Index]);
		prCfaOgm->rCurPage.au4PacketLen[uPacketNo] +=
			(prCfaOgm->rCurPage.puSegmentTable[u4Index]);

		if ((prCfaOgm->rCurPage.puSegmentTable[u4Index] < DMX_INVALID_UINT8)
			&& (u4Index != (u32) (prCfaOgm->rCurPage.uSegmentNs - 1))) {
			uPacketNo++;
			prCfaOgm->rCurPage.uPacketNs++;
		}
	}

	/*for the case of first page lacing of range are all FF */
	if ((prCfaOgm->rCurPage.uPacketNs == 1) && (prCfaOgm->rCurPage.fgFirstAudPage == TRUE) &&
		(prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_A)
		&& (prCfaOgm->rCurPage.fgFreshPacket == FALSE)) {
		prCfaOgm->rCfaOgmRange.u8AudStartOfst =
			prCfaOgm->rCurPage.u8DataOfst + prCfaOgm->rCurPage.u8DataLen;
		CfaOgmSkipPage(pvSptHdl, prCfaOgm);
		return;
	}

	/*if audio or video are not need to parse,this page will be safely skipped */
	if ((((prCfaOgm->u4PrsFlg & CFA_OGM_PRS_STRM_TYPE_V) == 0) ||
		(prCfaOgm->rCurPage.fgParseVideo == FALSE)) &&
		(prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_V)) {
		CfaOgmSkipPage(pvSptHdl, prCfaOgm);
		return;
	}

	if ((((prCfaOgm->u4PrsFlg & CFA_OGM_PRS_STRM_TYPE_A) == 0) ||
		(prCfaOgm->rCurPage.fgParseAudio == FALSE)) &&
		(prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_A)) {
		CfaOgmSkipPage(pvSptHdl, prCfaOgm);
		return;
	}

	/*if this is an unknown stream,it will also be skipped */
	if (prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_NONE) {
		CfaOgmSkipPage(pvSptHdl, prCfaOgm);
		return;
	}

	/*for timesearch,LPE give no audio lastgranule,so cfa get by first page and also skip this page */

	if (prCfaOgm->rCurPage.fgParseAudio == TRUE) {
		if (prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_A) {
			if(prCfaOgm->rCurPage.uAudIdx >= MAX_NS_OGM_AUD)
			{
				prCfaOgm->rCurPage.uAudIdx = MAX_NS_OGM_AUD - 1;
			}

			if (prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec ==
				CFA_AUD_DRV_FMT_VORBIS) {
				if ((prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
					fgFirst == FALSE)
					&& ((prCfaOgm->rCfaOgmRange.i8VidLastGranule != 0))) {
					prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
						fgFirst = TRUE;
					prCfaOgm->rCurPacket.au8LastAudGranule[prCfaOgm->rCurPage.
									uAudIdx] =
					    prCfaOgm->rCurPage.u8GranulPosition;

					if (prCfaOgm->fgPageContainEnd == TRUE) {
						CfaOgmSkipPage(pvSptHdl, prCfaOgm);
						return;
					}

					if ((prCfaOgm->rCurPage.uPacketNs == 1)
						&& (prCfaOgm->rCurPage.fgFreshPacket ==
						FALSE)) {
						prCfaOgm->rCfaOgmRange.u8AudStartOfst =
							prCfaOgm->rCurPage.u8DataOfst +
							prCfaOgm->rCurPage.u8DataLen;
						prCfaOgm->rCfaOgmRange.uAudPacketStartNo =
							1;
						prCfaOgm->rCfaOgmRange.i8AudLastGranule =
							prCfaOgm->rCurPacket.
							au8LastAudGranule[prCfaOgm->rCurPage.
							uAudIdx];
						CfaOgmSkipPage(pvSptHdl, prCfaOgm);
						return;
					}

					prCfaOgm->rCurPage.fgFirstAudPage = TRUE;

					if (prCfaOgm->rCurPage.fgFreshPacket == FALSE) {
						prCfaOgm->rCfaOgmRange.uAudPacketStartNo =
							prCfaOgm->rCurPage.uPacketNs - 1;
					} else {
						prCfaOgm->rCfaOgmRange.uAudPacketStartNo =
							prCfaOgm->rCurPage.uPacketNs;
					}
				}
			} else {
				if ((prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
					fgFirst == FALSE)
					&& ((prCfaOgm->rCfaOgmRange.i8VidLastGranule != 0)
					|| (prCfaOgm->rCfaOgmRange.u8SeekTime != 0))) {
					prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
						fgFirst = TRUE;
					prCfaOgm->rCurPacket.au8LastAudGranule[prCfaOgm->rCurPage.
										uAudIdx] =
						prCfaOgm->rCurPage.u8GranulPosition;

					if (prCfaOgm->fgPageContainEnd == TRUE) {
						CfaOgmSkipPage(pvSptHdl, prCfaOgm);
						return;
					}
					if ((prCfaOgm->rCurPage.uPacketNs == 1)
						&& (prCfaOgm->rCurPage.fgFreshPacket ==
						FALSE)) {
						prCfaOgm->rCfaOgmRange.u8AudStartOfst =
							prCfaOgm->rCurPage.u8DataOfst +
							prCfaOgm->rCurPage.u8DataLen;
						prCfaOgm->rCfaOgmRange.uAudPacketStartNo =
							1;
						prCfaOgm->rCfaOgmRange.i8AudLastGranule =
							prCfaOgm->rCurPacket.
							au8LastAudGranule[prCfaOgm->rCurPage.
									uAudIdx];
						CfaOgmSkipPage(pvSptHdl, prCfaOgm);
						return;
					}

					prCfaOgm->rCurPage.fgFirstAudPage = TRUE;

					if (prCfaOgm->rCurPage.fgFreshPacket == FALSE) {
						prCfaOgm->rCfaOgmRange.uAudPacketStartNo =
							prCfaOgm->rCurPage.uPacketNs - 1;
					} else {
						prCfaOgm->rCfaOgmRange.uAudPacketStartNo =
							prCfaOgm->rCurPage.uPacketNs;
					}
				}
			}

		}
	}

	/*vorbis page could be tranfer at once */
	if (prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_A) {
		if (prCfaOgm->rCurPage.uAudIdx >= MAX_NS_OGM_AUD) {
			prCfaOgm->rCurPage.uAudIdx = MAX_NS_OGM_AUD - 1;
		}

		if (prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec ==
			CFA_AUD_DRV_FMT_VORBIS) {
			CfaOgmTxVorbisPage(pvSptHdl, prCfaOgm);
			return;
		}
	}

	/*get data offset and length by lacing for the case of first page of range */
	if ((prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_V)
		&& (prCfaOgm->rCurPage.fgFirstVidPage == TRUE)) {
		uSkip = (u8)1;
		if (prCfaOgm->rCurPage.fgFreshPacket == FALSE)
			prCfaOgm->rCfaOgmRange.uVidPacketStartNo++;
		for (u4Index = 0; u4Index < prCfaOgm->rCurPage.uSegmentNs; u4Index++) {

			if (uSkip < prCfaOgm->rCfaOgmRange.uVidPacketStartNo) {
				prCfaOgm->rCurPage.u8DataOfst +=
					(prCfaOgm->rCurPage.puSegmentTable[u4Index]);
				prCfaOgm->rCurPage.u8DataLen -=
					(prCfaOgm->rCurPage.puSegmentTable[u4Index]);
			} else {
				break;
			}
			if (prCfaOgm->rCurPage.puSegmentTable[u4Index] < DMX_INVALID_UINT8)
				uSkip++;
		}
		prCfaOgm->u8Ca = prCfaOgm->rCurPage.u8DataOfst;
		prCfaOgm->rCurPacket.uVidPacketNo = uSkip - (u8)1;
		CfaOgmNextState(pvSptHdl, prCfaOgm, (u64)CFA_OGM_PACKET_HDR_READ, CFA_OGM_ST_PACKET_HDR_ANA);
		return;
	}

	if ((prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_A)
		&& (prCfaOgm->rCurPage.fgFirstAudPage == TRUE)) {
		uSkip = (u8)1;
		if (!(prCfaOgm->rCurPage.fgFreshPacket))
			prCfaOgm->rCfaOgmRange.uAudPacketStartNo++;
		for (u4Index = 0; u4Index < prCfaOgm->rCurPage.uSegmentNs; u4Index++) {
			if (uSkip < prCfaOgm->rCfaOgmRange.uAudPacketStartNo) {
				prCfaOgm->rCurPage.u8DataOfst +=
					(prCfaOgm->rCurPage.puSegmentTable[u4Index]);
				prCfaOgm->rCurPage.u8DataLen -=
					(prCfaOgm->rCurPage.puSegmentTable[u4Index]);
			} else {
				break;
			}
			if (prCfaOgm->rCurPage.puSegmentTable[u4Index] < DMX_INVALID_UINT8)
				uSkip++;
		}
		prCfaOgm->u8Ca = prCfaOgm->rCurPage.u8DataOfst;
		prCfaOgm->rCurPacket.uAudPacketNo = uSkip - (u8)1;
		CfaOgmNextState(pvSptHdl, prCfaOgm, (u64)CFA_OGM_PACKET_HDR_READ, CFA_OGM_ST_PACKET_HDR_ANA);
		return;
	}

	/*if this page have a part of packet lasts from last page,it will
	* be transfer to specific FIFO when not first page of range      */
	if (prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_V) {
		if ((prCfaOgm->rCurPage.fgFreshPacket == FALSE)
		    && (prCfaOgm->rCurPage.fgFirstVidPage == FALSE)) {
			prCfaOgm->rCurPacket.u8VidStartOfst = prCfaOgm->u8Ca;
			prCfaOgm->rCurPacket.u8VidLength = prCfaOgm->rCurPage.au4PacketLen[0];
#if !CONFIG_CFA_OGM_DIVX3_AU_SUPPORT
			if (prCfaOgm->rCfaOgmVidStream.eCfaVidCodec == CFA_VID_DIVX3)
				prCfaOgm->eCfaOgmTxMode = CFA_PTM_DUMMY;
#endif
			CfaOgmTxPacketData(pvSptHdl, prCfaOgm);
			return;
		} else if ((prCfaOgm->rCurPage.fgFreshPacket == FALSE)
			   && (prCfaOgm->rCurPage.fgFirstVidPage == TRUE)) {
			prCfaOgm->u8Ca += prCfaOgm->rCurPage.au4PacketLen[0];
			if (prCfaOgm->rCurPage.uPacketNs == 0) {
				CfaOgmNextState(pvSptHdl, prCfaOgm, (u64)CFA_OGM_HDR_READ_BYTES,
						CFA_OGM_ST_PAGE_HDR_ANA);
				return;
			}

			CfaOgmNextState(pvSptHdl, prCfaOgm, (u64)CFA_OGM_PACKET_HDR_READ,
					CFA_OGM_ST_PACKET_HDR_ANA);
			return;
		} else {
			/*do nothing*/
		}
	} else {		/*audio */
		if ((prCfaOgm->rCurPage.fgFreshPacket == FALSE)
		    && (prCfaOgm->rCurPage.fgFirstAudPage == FALSE)) {
			prCfaOgm->rCurPacket.u8AudStartOfst = prCfaOgm->u8Ca;
			prCfaOgm->rCurPacket.u8AudLength = prCfaOgm->rCurPage.au4PacketLen[0];
			prCfaOgm->rCurPacket.u8AudStartPTS = DMX_INVALID_UINT64;
			ComputePTS(prCfaOgm);

#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
			/* mtk40301  the third type of page*/
			if (CFA_AUD_DRV_FMT_AAC ==
				prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec) {
				/*if this packet is AAC*/
				ComputePTS(prCfaOgm);
				/*callback AAC packet analyze  //store the lenth and data of this packet */
				if (prCfaOgm->rCurPacket.u8AudLength == 0) {
					CfaOgmTxAACPacketData(pvSptHdl, prCfaOgm);
					return;
				}

				prCfaOgm->rCfaOgmAACPacket.uLastNoEndPacketLen =
				    prCfaOgm->rCfaOgmAACPacket.u8CurAACPacketLen;
				prCfaOgm->rCfaOgmAACPacket.u8CurAACPacketLen +=
				    prCfaOgm->rCurPacket.u8AudLength;
				CfaOgmStoreAACPacketData(pvSptHdl,
							 prCfaOgm->rCurPacket.u8AudLength,
							 prCfaOgm);
				return;
			}
			CfaOgmTxPacketData(pvSptHdl, prCfaOgm);
			return;
#else
			CfaOgmTxPacketData(pvSptHdl, prCfaOgm);
			return;
#endif
		} else if ((prCfaOgm->rCurPage.fgFreshPacket == FALSE)
			   && (prCfaOgm->rCurPage.fgFirstAudPage == TRUE)) {
			prCfaOgm->u8Ca += (u64)prCfaOgm->rCurPage.au4PacketLen[0];
			if (prCfaOgm->rCurPage.uPacketNs == 0) {
				CfaOgmNextState(pvSptHdl, prCfaOgm, (u64)CFA_OGM_HDR_READ_BYTES,
						CFA_OGM_ST_PAGE_HDR_ANA);
				return;
			}

			CfaOgmNextState(pvSptHdl, prCfaOgm, (u64)CFA_OGM_PACKET_HDR_READ,
					CFA_OGM_ST_PACKET_HDR_ANA);
			return;
		} else {
			/*do nothing*/
		}
	}

	CfaOgmNextState(pvSptHdl, prCfaOgm, (u64)CFA_OGM_PACKET_HDR_READ, CFA_OGM_ST_PACKET_HDR_ANA);
}


/* Analyze the header of a packet*/
static void CfaOgmAnaPacket(void *pvSptHdl, u64 u8TxLen, CfaOgmInst *prCfaOgm)
{
	u8 uLenBytes = 0;

	if ((prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_A)
	    && (prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec ==
		CFA_AUD_DRV_FMT_FLAC)) {
        if (prCfaOgm->rCurPage.uAudIdx >= MAX_NS_OGM_AUD) {
			prCfaOgm->rCurPage.uAudIdx = MAX_NS_OGM_AUD - 1;
		}

		prCfaOgm->rCurPacket.au8LastAudGranule[prCfaOgm->rCurPage.uAudIdx] +=
		    prCfaOgm->rCurPacket.au8CurAudGranule[prCfaOgm->rCurPage.uAudIdx];
		prCfaOgm->rCurPacket.au8CurAudGranule[prCfaOgm->rCurPage.uAudIdx] =
		    prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].u8SamplePerUnit;
		prCfaOgm->rCurPacket.u8AudStartOfst = prCfaOgm->u8Ca - 1;
        if (prCfaOgm->rCurPacket.uAudPacketNo >= DMX_INVALID_UINT8) {
			DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] %s line %d,rCurPacket.uAudPacketNo(%d) >= 0xff! Finish parse!\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prCfaOgm->rCurPacket.uAudPacketNo);
			OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);
			return;
        }

		prCfaOgm->rCurPacket.u8AudLength =
		    prCfaOgm->rCurPage.au4PacketLen[prCfaOgm->rCurPacket.uAudPacketNo];

		ComputePTS(prCfaOgm);

		if (!prCfaOgm->fgIfNotifyPTS) {
			prCfaOgm->fgIfNotifyPTS = TRUE;
			if (prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_A)
				Spt4CfaPTSNotify(pvSptHdl, prCfaOgm->rCurPacket.u8AudStartPTS);
		}
#if CONFIG_CFA_OGM_DIVX3_AU_SUPPORT
		prCfaOgm->fgUnitStart = TRUE;
#endif
		CfaOgmTxPacketData(pvSptHdl, prCfaOgm);
	} else {
		prCfaOgm->rCurPacket.uPacketHdr = *((u8 *) prCfaOgm->ptrMemAddr);

		uLenBytes = GetLenbytes(prCfaOgm->rCurPacket.uPacketHdr);


		/*mtk40301 fix bug 122405 06/25/2009*/
		if (prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_V)
			prCfaOgm->rCurPacket.fgKeyFrame = KeyFrame(prCfaOgm->rCurPacket.uPacketHdr);

		if (uLenBytes == 0) {
			if (prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_V) {
				prCfaOgm->rCurPacket.u4VidPacketTotalNo++;
				prCfaOgm->rCurPacket.u8LastVidGranule +=
				    prCfaOgm->rCurPacket.u8CurVidGranule;
				prCfaOgm->rCurPacket.u8CurVidGranule =
				    prCfaOgm->rCfaOgmVidStream.u8DefaultFrame;
				prCfaOgm->rCurPacket.u8VidStartOfst = prCfaOgm->u8Ca;
				if(prCfaOgm->rCurPacket.uVidPacketNo >= DMX_INVALID_UINT8)
				{
					DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
						TEXT("[CFA_OGM] %s line %d,rCurPacket.uVidPacketNo(%d) >= 0xff! Finish parse!\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prCfaOgm->rCurPacket.uVidPacketNo);
					OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);
					return;
				}

				prCfaOgm->rCurPacket.u8VidLength =
				    prCfaOgm->rCurPage.au4PacketLen[prCfaOgm->rCurPacket.
								    uVidPacketNo] - 1;
				/*Fill AU Data*/
				prCfaOgm->rCfaOgmAu.u8LastVidGranule =
				    prCfaOgm->rCurPacket.u8LastVidGranule;
				prCfaOgm->rCfaOgmAu.u8StartOfst = prCfaOgm->rCurPage.u8StartOfst;
				if (prCfaOgm->rCurPage.fgFreshPacket) {
					prCfaOgm->rCfaOgmAu.uVidPacketNo =
					    prCfaOgm->rCurPacket.uVidPacketNo + 1;
				} else {
					prCfaOgm->rCfaOgmAu.uVidPacketNo =
					    prCfaOgm->rCurPacket.uVidPacketNo;
				}
			} else {	/*audio */

				if(prCfaOgm->rCurPage.uAudIdx >= MAX_NS_OGM_AUD)
				{
					prCfaOgm->rCurPage.uAudIdx = MAX_NS_OGM_AUD - 1;
				}

				prCfaOgm->rCurPacket.au8LastAudGranule[prCfaOgm->rCurPage.
								       uAudIdx] +=
				    prCfaOgm->rCurPacket.au8CurAudGranule[prCfaOgm->rCurPage.
									  uAudIdx];
				prCfaOgm->rCurPacket.au8CurAudGranule[prCfaOgm->rCurPage.uAudIdx] =
				    prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
				    u8DefaultSample;
				prCfaOgm->rCurPacket.u8AudStartOfst = prCfaOgm->u8Ca;
				if(prCfaOgm->rCurPacket.uAudPacketNo >= DMX_INVALID_UINT8)
				{
					DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
						TEXT("[CFA_OGM] %s line %d,rCurPacket.uAudPacketNo(%d) >= 0xff! Finish parse!\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prCfaOgm->rCurPacket.uAudPacketNo);
					OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);
					return;
				}

				prCfaOgm->rCurPacket.u8AudLength =
				    prCfaOgm->rCurPage.au4PacketLen[prCfaOgm->rCurPacket.
								    uAudPacketNo] - 1;
			}

			ComputePTS(prCfaOgm);

			if (!prCfaOgm->fgIfNotifyPTS) {
				prCfaOgm->fgIfNotifyPTS = TRUE;
				if (prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_V) {
					Spt4CfaPTSNotify(pvSptHdl, prCfaOgm->rCurPacket.u8VidStartPTS);
				} else if (prCfaOgm->rCurPage.eCfaOgmStrmType ==
					   CFA_OGM_PRS_STRM_TYPE_A) {
					Spt4CfaPTSNotify(pvSptHdl, prCfaOgm->rCurPacket.u8AudStartPTS);
				} else {
					/*do nothing*/
				}
			}


			if ((prCfaOgm->rCfaOgmVidStream.eCfaVidCodec == CFA_VID_DIVX3)
			    && (prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_V)
			    && (prCfaOgm->rCurPacket.u8VidLength > 0)) {
				CfaOgmNextState(pvSptHdl, prCfaOgm, (u64)1, CFA_OGM_ST_DIVX3_PACKET_ANA);
				return;
			}
#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
			if ((prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_A)
			    && (prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].
				eCfaAudCodec == CFA_AUD_DRV_FMT_AAC)) {
				/*initial parameters about AAC packet */
				prCfaOgm->rCfaOgmAACPacket.uLastNoEndPacketLen = 0;
				dmx_memset((u8 *) (prCfaOgm->rCfaOgmAACPacket.pu1AACPacketData),
					   0, (sizeof(u8) * (CFA_OGM_AAC_PACKET_MAX_LEN)));

				prCfaOgm->rCfaOgmAACPacket.u8CurAACPacketLen =
				    prCfaOgm->rCurPacket.u8AudLength;
				CfaOgmStoreAACPacketData(pvSptHdl, prCfaOgm->rCurPacket.u8AudLength,
							 prCfaOgm);
				return;
			}
#endif

#if CONFIG_CFA_OGM_DIVX3_AU_SUPPORT
			prCfaOgm->fgUnitStart = TRUE;
#endif
			CfaOgmTxPacketData(pvSptHdl, prCfaOgm);
		}  else {/*lenbytes == 0; */
			CfaOgmNextState(pvSptHdl, prCfaOgm, (u64) uLenBytes,
					CFA_OGM_ST_LENBYTES_ANA);
			return;
		}
	}
}


/* Analyze Packet lenbytes and data, Get some infomation*/
static void CfaOgmGetPacketGranul(void *pvSptHdl, u64 u8TxLen, CfaOgmInst *prCfaOgm)
{

	if (prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_V) {

		prCfaOgm->rCurPacket.u4VidPacketTotalNo++;
		prCfaOgm->rCurPacket.u8LastVidGranule += prCfaOgm->rCurPacket.u8CurVidGranule;
		prCfaOgm->rCurPacket.u8CurVidGranule =
		    OgmComputeLittleEndian((u8 *) (prCfaOgm->ptrMemAddr), (u8) u8TxLen);

		/* if frame duration larger than 1000ms set to default duration. */
		if (prCfaOgm->rCurPacket.u8CurVidGranule * prCfaOgm->rCfaOgmVidStream.u8TimeUnit *
		    (u64)1000 / (u64)OGM_TIME > (u64)1000) {
			prCfaOgm->rCurPacket.u8CurVidGranule =
			    prCfaOgm->rCfaOgmVidStream.u8DefaultFrame;
		}

		prCfaOgm->rCurPacket.u8VidStartOfst = prCfaOgm->u8Ca;
		if (prCfaOgm->rCurPacket.uVidPacketNo >= DMX_INVALID_UINT8)
			prCfaOgm->rCurPacket.uVidPacketNo = (u8)DMX_INVALID_UINT8 - (u8)1;
		prCfaOgm->rCurPacket.u8VidLength =
		    (u64)(prCfaOgm->rCurPage.au4PacketLen[prCfaOgm->rCurPacket.uVidPacketNo]) - (u64)1 -
		    u8TxLen;

		/*Fill AU Data*/
		prCfaOgm->rCfaOgmAu.u8LastVidGranule = prCfaOgm->rCurPacket.u8LastVidGranule;
		prCfaOgm->rCfaOgmAu.u8StartOfst = prCfaOgm->rCurPage.u8StartOfst;
		if (prCfaOgm->rCurPage.fgFreshPacket)
			prCfaOgm->rCfaOgmAu.uVidPacketNo = prCfaOgm->rCurPacket.uVidPacketNo + 1;
		else
			prCfaOgm->rCfaOgmAu.uVidPacketNo = prCfaOgm->rCurPacket.uVidPacketNo;
	} else {		/*audio */

		if (prCfaOgm->rCurPage.uAudIdx >= MAX_NS_OGM_AUD)
			prCfaOgm->rCurPage.uAudIdx = MAX_NS_OGM_AUD - 1;
		prCfaOgm->rCurPacket.au8LastAudGranule[prCfaOgm->rCurPage.uAudIdx] +=
		    prCfaOgm->rCurPacket.au8CurAudGranule[prCfaOgm->rCurPage.uAudIdx];
		prCfaOgm->rCurPacket.au8CurAudGranule[prCfaOgm->rCurPage.uAudIdx] =
		    OgmComputeLittleEndian((u8 *) (prCfaOgm->ptrMemAddr), (u8) u8TxLen);
		prCfaOgm->rCurPacket.u8AudStartOfst = prCfaOgm->u8Ca;
		if (prCfaOgm->rCurPacket.uAudPacketNo >= DMX_INVALID_UINT8)
			prCfaOgm->rCurPacket.uAudPacketNo = (u8)DMX_INVALID_UINT8 - (u8)1;
		prCfaOgm->rCurPacket.u8AudLength =
		    (u64)(prCfaOgm->rCurPage.au4PacketLen[prCfaOgm->rCurPacket.uAudPacketNo]) - (u64)1 -
		    u8TxLen;
	}

	ComputePTS(prCfaOgm);

	if (prCfaOgm->fgIfNotifyPTS == FALSE) {
		prCfaOgm->fgIfNotifyPTS = TRUE;
		if (prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_V)
			Spt4CfaPTSNotify(pvSptHdl, prCfaOgm->rCurPacket.u8VidStartPTS);
		else if (prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_A)
			Spt4CfaPTSNotify(pvSptHdl, prCfaOgm->rCurPacket.u8AudStartPTS);
		else {
			/*do nothing*/
		}
	}

	if ((prCfaOgm->rCfaOgmVidStream.eCfaVidCodec == CFA_VID_DIVX3)
	    && (prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_V)) {
		CfaOgmNextState(pvSptHdl, prCfaOgm, (u64)1, CFA_OGM_ST_DIVX3_PACKET_ANA);
		return;
	}
#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
	if ((prCfaOgm->rCurPage.eCfaOgmStrmType == CFA_OGM_PRS_STRM_TYPE_A)
	    && (prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec ==
		CFA_AUD_DRV_FMT_AAC)) {
		/*initial parameters about AAC packet */
		prCfaOgm->rCfaOgmAACPacket.uLastNoEndPacketLen = 0;
		dmx_memset((u8 *) (prCfaOgm->rCfaOgmAACPacket.pu1AACPacketData), 0,
			   (sizeof(u8) * (CFA_OGM_AAC_PACKET_MAX_LEN)));

		prCfaOgm->rCfaOgmAACPacket.u8CurAACPacketLen = prCfaOgm->rCurPacket.u8AudLength;
		CfaOgmStoreAACPacketData(pvSptHdl, prCfaOgm->rCurPacket.u8AudLength, prCfaOgm);
		return;
	}
	CfaOgmTxPacketData(pvSptHdl, prCfaOgm);
	return;
#else
	CfaOgmTxPacketData(pvSptHdl, prCfaOgm);
	return;
#endif
}


static void CfaOgmGetDivx3Type(void *pvSptHdl, u64 u8TxLen, CfaOgmInst *prCfaOgm)
{
	u8 ucFlag = *((u8 *) (prCfaOgm->ptrMemAddr));

	prCfaOgm->eCfaOgmTxMode = CFA_PTM_ONE_PIC_DX3_I;

	if (ucFlag & (u8)CFA_OGM_DIVX3_P_FRM)
		prCfaOgm->eCfaOgmTxMode = CFA_PTM_ONE_PIC_DX3_P;
	else
		prCfaOgm->eCfaOgmTxMode = CFA_PTM_ONE_PIC_DX3_I;

#if CONFIG_CFA_OGM_DIVX3_AU_SUPPORT
	prCfaOgm->fgUnitStart = TRUE;
#endif

	CfaOgmTxPacketData(pvSptHdl, prCfaOgm);
}


/*-----------------------------------------------------------------------------
 * Name: CfaOgmTxDoneStCtrl
 *
 * Description:
 *      OGM CFA state control for transfer done
 *      This function will be called after a transfer is complete.
 *
 * Inputs: -
 *
 * Outputs: -
 *
 * Returns: None
 *-----------------------------------------------------------------------------*/
void CfaOgmTxDoneStCtrl(void *pvSptHdl, u64 u8TxLen, CfaOgmInst *prCfaOgm)
{
	if (prCfaOgm->fgUseCMDQ) {
	   do {
#if CFA_OGM_INTERNAL_DEBUG
			DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] %s enter, prCfaOgm->u8Ca:0X%llx,CurState:%d, u8TxLen:%lld, fgNoNeedSync:%d\n"),
				DMX_FUNC_NAME, prCfaOgm->u8Ca, prCfaOgm->eCfaOgmCurState, u8TxLen, prCfaOgm->fgNoNeedSyncPb);
#endif
			
			prCfaOgm->fgNoNeedSyncPb = FALSE;
			if (prCfaOgm->fgRealSyncPb) {
				if (0 == prCfaOgm->ptrMemAddr) {
					DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
						TEXT("[CFA_FLV] %s line %d failed for prCfaOgm->ptrMemAddr is 0\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
					Spt4CfaFinishedEx(pvSptHdl, prCfaOgm->u8Ca, FALSE, GAU_E_EOS);
					return;
				}
				prCfaOgm->ptrLastReadMemAddr = prCfaOgm->ptrMemAddr;
			    prCfaOgm->u8AvalOfst = prCfaOgm->u8Ca + prCfaOgm->u4TxLen + prCfaOgm->u4AvalSize;
				prCfaOgm->fgRealSyncPb = FALSE;			
			} else {
				u8TxLen = prCfaOgm->u4TxLen;
			}

#if  CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
			if (  ( CFA_OGM_ST_PACKET_TX == prCfaOgm->eCfaOgmCurState)
				&&( CFA_OGM_PRS_STRM_TYPE_A == prCfaOgm->eCfaOgmCurPrsStrm)
				&&( CFA_AUD_DRV_FMT_AAC == prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec)) {
				u8TxLen = 0;
			}
#endif
	
			if (CFA_OGM_ST_AVCMDQ_TX == prCfaOgm->eCfaOgmCurState ||
				CFA_OGM_ST_PACKET_DATA_TX == prCfaOgm->eCfaOgmCurState ||
				CFA_OGM_ST_FINISH == prCfaOgm->eCfaOgmCurState) {
				u8TxLen = 0;
			}
			
			if (prCfaOgm->arAudCmdQInfo[prCfaOgm->uIdxForFillAU].fgIsInDma) {
				dmx_memset(&prCfaOgm->arAudCmdQInfo[prCfaOgm->uIdxForFillAU], 0, sizeof(prCfaOgm->arAudCmdQInfo[prCfaOgm->uIdxForFillAU]));
				if (prCfaOgm->uCurCmdQIdx >= prCfaOgm->uTxAudCmdQNs) {
					dmx_memset((VOID *)(prCfaOgm->arCmdQStartPage),0,sizeof(prCfaOgm->arCmdQStartPage[0])*MAX_NS_OGM_AUD);
					dmx_memset((VOID *)(prCfaOgm->arCmdQStartPacket),0,sizeof(prCfaOgm->arCmdQStartPacket[0])*MAX_NS_OGM_AUD);
					dmx_memset((VOID *)(prCfaOgm->au8TxAudCmdQIndex),0,sizeof(u8)*MAX_NS_OGM_AUD);
					prCfaOgm->uCurCmdQIdx = 0;
					prCfaOgm->uTxAudCmdQNs = 0;
				}
			}
			if(prCfaOgm->rVidCmdQInfo.fgIsInDma) {
				dmx_memset(&prCfaOgm->rVidCmdQInfo,0,sizeof(prCfaOgm->rVidCmdQInfo));
			}

			DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] %s line %d, prCfaOgm->u8Ca:0X%llx,u8TxLen:%lld, CA+LEN:0X%llx,state:%d\n"),
				DMX_FUNC_NAME, __LINE__, prCfaOgm->u8Ca,u8TxLen, prCfaOgm->u8Ca+u8TxLen, prCfaOgm->eCfaOgmCurState);
			prCfaOgm->u8Ca += u8TxLen;

			switch (prCfaOgm->eCfaOgmCurState) {
				case CFA_OGM_ST_IDLE:
					CfaOgmAnaStIdle(pvSptHdl, u8TxLen, prCfaOgm);
					break;
	
				case CFA_OGM_ST_PAGE_HDR_ANA:
					CfaOgmAnaPageHeader(pvSptHdl,u8TxLen,prCfaOgm);
					break;
	
				case CFA_OGM_ST_PAGE_LACING_ANA:
				   CfaOgmAnaLacing(pvSptHdl,u8TxLen,prCfaOgm);
				   break;
	
			   case CFA_OGM_ST_PACKET_HDR_ANA:
				   CfaOgmAnaPacket(pvSptHdl,u8TxLen,prCfaOgm);
				   break;
	
			   case CFA_OGM_ST_PACKET_TX:
				   CfaOgmTxPacket(pvSptHdl,u8TxLen,prCfaOgm);
				   break;
	
			   case CFA_OGM_ST_LENBYTES_ANA:
				   CfaOgmGetPacketGranul(pvSptHdl,u8TxLen,prCfaOgm);
       #if CFA_OGM_TMP_DBG
				   DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
						TEXT("[CFA_OGM] %s line:%d ,fgNoNeedSyncPb:%d\n"),DMX_FUNC_NAME,DMX_LINE_NO,prCfaOgm->fgNoNeedSyncPb);
       #endif
				   break;
	
			   case CFA_OGM_ST_DIVX3_PACKET_ANA:
				   CfaOgmGetDivx3Type(pvSptHdl,u8TxLen,prCfaOgm);
				   break;
	
#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
			   case CFA_OGM_ST_AAC_PACKET_ANA:
				   CfaOgmAACPacketAna(pvSptHdl, u8TxLen, prCfaOgm);
				   break;
#endif
			   case CFA_OGM_ST_NEXT_STATE:
					prCfaOgm->u8Ca -= u8TxLen;
					CfaOgmNextState(pvSptHdl,prCfaOgm,u8TxLen,prCfaOgm->eCfaOgmNextState);
					break;
	
			   case CFA_OGM_ST_VORBIS_PAGE_TX:
					CfaOgmTxVorbisPage(pvSptHdl,prCfaOgm);
					break;
	
			   case CFA_OGM_ST_PACKET_DATA_TX:
        #if CFA_OGM_TMP_DBG
					DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
						TEXT("[CFA_OGM] %s line:%d \n"),DMX_FUNC_NAME,DMX_LINE_NO);
        #endif
					CfaOgmTxPacketData(pvSptHdl,prCfaOgm);
					break;
	
			   case CFA_OGM_ST_AVCMDQ_TX:
					CfaOgmTxAudCmdQ(pvSptHdl,prCfaOgm,0);
					break;
	
			   case CFA_OGM_ST_FINISH:
					OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
					break;
					
				default:
					DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
						TEXT("[CFA_OGM] %s line %d,prCfaOgm->eCfaOgmCurState is error! Finish parse!\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
					OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
					break;
			}
		}while(prCfaOgm->fgNoNeedSyncPb);
	} else {
		do {
			prCfaOgm->fgNoNeedSyncPb = FALSE;
			if (prCfaOgm->fgRealSyncPb) {
				if (0 == prCfaOgm->ptrMemAddr) {
					DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
						    TEXT
						    ("[CFA_FLV] %s line %d failed for prCfaOgm->ptrMemAddr is 0\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO);
					Spt4CfaFinishedEx(pvSptHdl, prCfaOgm->u8Ca, TRUE, (u32)GAU_E_FAIL);
					return;
				}

				prCfaOgm->fgRealSyncPb = FALSE;
			} else {
				u8TxLen = prCfaOgm->u4TxLen;
			}
#if  CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
			if ((CFA_OGM_ST_PACKET_TX == prCfaOgm->eCfaOgmCurState)
			    && (CFA_OGM_PRS_STRM_TYPE_A == prCfaOgm->eCfaOgmCurPrsStrm)
			    && (CFA_AUD_DRV_FMT_AAC ==
				prCfaOgm->arCfaOgmAudStream[prCfaOgm->rCurPage.uAudIdx].eCfaAudCodec)) {
				u8TxLen = 0;
			}
#endif
			if (prCfaOgm->rAudCmdQInfo.fgIsInDma)
				dmx_memset(&prCfaOgm->rAudCmdQInfo, 0, sizeof(CfaOgmAudCmdQInfo_T));
			
			prCfaOgm->u8Ca += u8TxLen;

			switch (prCfaOgm->eCfaOgmCurState) {
			case CFA_OGM_ST_IDLE:
				CfaOgmAnaStIdle(pvSptHdl, u8TxLen, prCfaOgm);
				break;

			case CFA_OGM_ST_PAGE_HDR_ANA:
				CfaOgmAnaPageHeader(pvSptHdl, u8TxLen, prCfaOgm);
				break;

			case CFA_OGM_ST_PAGE_LACING_ANA:
				CfaOgmAnaLacing(pvSptHdl, u8TxLen, prCfaOgm);
				break;

			case CFA_OGM_ST_PACKET_HDR_ANA:
				CfaOgmAnaPacket(pvSptHdl, u8TxLen, prCfaOgm);
				break;

			case CFA_OGM_ST_PACKET_TX:
				CfaOgmTxPacket(pvSptHdl, u8TxLen, prCfaOgm);
				break;

			case CFA_OGM_ST_LENBYTES_ANA:
				CfaOgmGetPacketGranul(pvSptHdl, u8TxLen, prCfaOgm);
				break;

			case CFA_OGM_ST_DIVX3_PACKET_ANA:
				CfaOgmGetDivx3Type(pvSptHdl, u8TxLen, prCfaOgm);
				break;

#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
			case CFA_OGM_ST_AAC_PACKET_ANA:
				CfaOgmAACPacketAna(pvSptHdl, u8TxLen, prCfaOgm);
				break;
#endif
			case CFA_OGM_ST_NEXT_STATE:
				prCfaOgm->u8Ca -= u8TxLen;
				CfaOgmNextState(pvSptHdl, prCfaOgm, u8TxLen, prCfaOgm->eCfaOgmNextState);
				break;

			case CFA_OGM_ST_VORBIS_PAGE_TX:
				CfaOgmTxVorbisPage(pvSptHdl, prCfaOgm);
				break;

			case CFA_OGM_ST_FINISH:
				 OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
				 break;

			default:
				DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("[CFA_OGM] %s line %d,prCfaOgm->eCfaOgmCurState is error! Finish parse!\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
				break;
			}
		} while (prCfaOgm->fgNoNeedSyncPb);
	}
}


void CfaOgmNextState(void *pvSptHdl, CfaOgmInst *prCfaOgm, u64 u8ReadLen,
		     CfaOgmAnaState_E eCfaOgmNextState)
{
	if (prCfaOgm->fgUseCMDQ) {
	   MRESULT mrRet = RET_DMX_OK;
#if CFA_OGM_INTERNAL_DEBUG
	   DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
		   TEXT("[CFA_OGM] %s enter,CA:0x%llx,readlen:0x%llx\r\n"),
		   DMX_FUNC_NAME, prCfaOgm->u8Ca, u8ReadLen);
#endif
	   if ((prCfaOgm->u8Ca) >= ((prCfaOgm->rCfaOgmRange.u8AudEndOfst > prCfaOgm->rCfaOgmRange.u8VidEndOfst) ?
												(prCfaOgm->rCfaOgmRange.u8AudEndOfst) : (prCfaOgm->rCfaOgmRange.u8VidEndOfst))) {
			DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM]%s,prCfaOgm->u8Ca(%lld) >= EndOfst(%lld) \n"),
				DMX_FUNC_NAME,prCfaOgm->u8Ca,prCfaOgm->rCfaOgmRange.u8VidEndOfst);
			OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
	   } else {
		   prCfaOgm->eCfaOgmCurState = eCfaOgmNextState;
	
#if CONFIG_CFA_OGM_NEW_SYNCBUF
		   if ((prCfaOgm->fgIfRebuf) && ((prCfaOgm->u8Ca + u8ReadLen) <= prCfaOgm->u8AvalOfst)) {
			   prCfaOgm->ptrMemAddr	= (uintptr_t)(prCfaOgm->ptrLastReadMemAddr + (prCfaOgm->u8Ca - prCfaOgm->u8LastReadOfst));
			   prCfaOgm->fgNoNeedSyncPb = TRUE;
			   prCfaOgm->u8LastReadOfst = prCfaOgm->u8Ca;
			   prCfaOgm->ptrLastReadMemAddr = prCfaOgm->ptrMemAddr;
			   prCfaOgm->u4TxLen = u8ReadLen;
   #if CFA_OGM_TMP_DBG
			   DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("[CFA_OGM] %s line:%d \n"),DMX_FUNC_NAME,DMX_LINE_NO);
   #endif
			   return;
		   } else {
			   bool fgTxCmdQ = FALSE;
	
			   fgTxCmdQ = CfaOgmTxAvCmdQ(pvSptHdl, prCfaOgm, eCfaOgmNextState, CFA_OGM_ST_NEXT_STATE);
			   if (fgTxCmdQ) {
				   prCfaOgm->u4TxLen = (u32)u8ReadLen;
				   return;
			   }
	
   #if CFA_OGM_TMP_DBG
			   DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("[CFA_OGM] %s line:%d \n"),DMX_FUNC_NAME,DMX_LINE_NO);
   #endif
			   prCfaOgm->fgRealSyncPb = TRUE;
			   prCfaOgm->u4AvalSize = 0;
			   prCfaOgm->u4TxLen = u8ReadLen;
			   prCfaOgm->u8LastReadOfst = prCfaOgm->u8Ca;

			   prCfaOgm->u8RspCa = prCfaOgm->u8Ca;
			   prCfaOgm->u8RspTxLen = prCfaOgm->u4TxLen;
			   prCfaOgm->eRspState = prCfaOgm->eCfaOgmCurState;
			   prCfaOgm->rRspPacket = prCfaOgm->rCurPacket;
			   prCfaOgm->rRspPage = prCfaOgm->rCurPage;
				
			   	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("[CFA_OGM] %s line:%d,RspCa:0x%llx, RspTxLen:0x%llx,eRspState:0x%d\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prCfaOgm->u8RspCa, prCfaOgm->u8RspTxLen, prCfaOgm->eRspState);
			   mrRet = Spt4CfaPbb2SyncBufEx(pvSptHdl, prCfaOgm->u8Ca,u8ReadLen,(UCHAR *)&(prCfaOgm->ptrMemAddr),&(prCfaOgm->u4AvalSize));
			   
			   if (RET_DMX_OK != mrRet) {
				   DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
						TEXT("[CFA_OGM] Spt4CfaPbb2SyncBufEx error ret = %d \n"), mrRet);
				   OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
				   return ;
			   }
		   }
#else
		   mrRet = Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaOgm->u8Ca,u8ReadLen,(UCHAR *)&(prCfaOgm->ptrMemAddr));
		   if (RET_DMX_OK != mrRet) {
			   DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("[CFA_OGM] Spt4CfaPbb2SyncBuf error ret = %d \n"), mrRet);
			   OgmFinishPrs(__func__, __LINE__, pvSptHdl,prCfaOgm);
			   return ;
		   }
#endif
	   }
	} else {
		MRESULT mrRet = RET_DMX_OK;

		if ((prCfaOgm->u8Ca) >=
		    ((prCfaOgm->rCfaOgmRange.u8AudEndOfst >
		      prCfaOgm->rCfaOgmRange.u8VidEndOfst) ? (prCfaOgm->rCfaOgmRange.
							      u8AudEndOfst) : (prCfaOgm->rCfaOgmRange.
									       u8VidEndOfst))) {
			OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);
		} else {
			prCfaOgm->eCfaOgmCurState = eCfaOgmNextState;

#if CONFIG_CFA_OGM_NEW_SYNCBUF
			if (((prCfaOgm->u8Ca + u8ReadLen) < prCfaOgm->u8AvalOfst)
			    && (prCfaOgm->fgIfRebuf == TRUE)) {
				prCfaOgm->ptrMemAddr =
				    (uintptr_t) (prCfaOgm->ptrLastReadMemAddr +
					      (prCfaOgm->u8Ca - prCfaOgm->u8LastReadOfst));
				prCfaOgm->fgNoNeedSyncPb = TRUE;
				prCfaOgm->u8LastReadOfst = prCfaOgm->u8Ca;
				prCfaOgm->ptrLastReadMemAddr = prCfaOgm->ptrMemAddr;
				prCfaOgm->u4TxLen = u8ReadLen;

				return;
			}

			if (prCfaOgm->rAudCmdQInfo.u4EntryCnt > 0) {
				prCfaOgm->eCfaOgmNextState = eCfaOgmNextState;
				prCfaOgm->eCfaOgmCurState = CFA_OGM_ST_NEXT_STATE;
				prCfaOgm->u4TxLen = (u32) u8ReadLen;
				CfaOgmTxAudCmdQ(pvSptHdl, prCfaOgm, (u32)0);
				return;
			}

			prCfaOgm->fgRealSyncPb = TRUE;
			prCfaOgm->fgCrossSlot = TRUE;
			prCfaOgm->u4AvalSize = 0;
			prCfaOgm->u4TxLen = u8ReadLen;
			prCfaOgm->u8LastReadOfst = prCfaOgm->u8Ca;
			mrRet =
			    Spt4CfaPbb2SyncBufEx(pvSptHdl, prCfaOgm->u8Ca, u8ReadLen,
						 (u8 *) &(prCfaOgm->ptrMemAddr),
						 &(prCfaOgm->u4AvalSize));

			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("[CFA_OGM] Spt4CfaPbb2SyncBufEx error ret = %d\n"),
					mrRet);
				OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);
				return;
			}
			prCfaOgm->ptrLastReadMemAddr = prCfaOgm->ptrMemAddr;
			prCfaOgm->u8AvalOfst = prCfaOgm->u8Ca + u8ReadLen + prCfaOgm->u4AvalSize;
#else
			mrRet =
			    Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaOgm->u8Ca, u8ReadLen,
					       (u8 *) &(prCfaOgm->ptrMemAddr));
			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("[CFA_OGM] Spt4CfaPbb2SyncBuf error ret = %d\n"), mrRet);
				OgmFinishPrs(__func__, __LINE__, pvSptHdl, prCfaOgm);
				return;
			}
#endif

		}
	}
}



/*-----------------------------------------------------------------------------
 * Name: ucCfaOgmGetAudIndex
 *
 * Description:
 *      Get the index of audio info by audio stream ID.
 *      For multi-channel.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: the index of audio info
 *
 *-----------------------------------------------------------------------------*/
u8 CfaOgmGetAudIndex(CfaOgmInst *prCfaOgm, u32 u4StrmID)
{
	u8 uIdx = 0;

    if (prCfaOgm->u4AudNum > MAX_NS_OGM_AUD) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] %s line %d,prCfaOgm->u4AudNum(%d) > MAX_NS_OGM_AUD(%d)!\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prCfaOgm->u4AudNum, MAX_NS_OGM_AUD);
		prCfaOgm->u4AudNum = MAX_NS_OGM_AUD;
    }

	for (uIdx = 0; uIdx < prCfaOgm->u4AudNum; uIdx++) {
		if (prCfaOgm->arCfaOgmAudStream[uIdx].u4AudStreamNo == u4StrmID)
			return uIdx;
	}

	return DMX_INVALID_UINT8;
}
