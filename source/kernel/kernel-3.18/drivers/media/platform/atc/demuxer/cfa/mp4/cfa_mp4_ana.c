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
/*#pragma warning(disable: 4127) *//*disable warning C4127: conditional expression is constant*/
/*#pragma warning(disable: 4115) *//*disable warning C4115: named type definition in parentheses*/

#include <media/atc/dmx_define.h>
/* #include <media/atc/mm_debug.h> */

#include "dmx_def.h"
#include "cfa_mp4_util.h"
#include "cfa_mp4_ana.h"
#include "cfa_mp4_state.h"
#include "cfa_macro.h"
#include "dmx_mem.h"
#include "dmx_spt.h"
/*#pragma warning(pop)*/

#define dwCfaMP4FourCC(a, b, c, d)	(((d)<<24)|((c)<<16)|((b)<<8)|(a))
#define fgCfaMP4Is4cc(addr, a, b, c, d) (*((u32 *)(addr)) == dwCfaMP4FourCC(a, b, c, d))
#define SUBTXT_LENGTH_HEADER 2
/*-----------------------------------------------------------------------------
* Name: vCfaMp4AnaPrsVRange
*
* Description:
*	   prs video range to demux index table,once prs only one chunk,prs  all sample in cur chunk
*
* Inputs:
*
* Outputs:
*
* Returns: the index of audio info
*
*-----------------------------------------------------------------------------*/
void CfaMp4AnaPrsVRange(void *pvSptHdl, CfaMp4Inst *prCfaMp4)
{
	u64 u8CurSampleNo = 0;
	u32 u4CurChunkSampleNums = 0;
	TCfaMp4VInf *prVInfo = &prCfaMp4->rCfaMp4VInf;

	if (prVInfo->u8CurPrsVidChunkNo == prCfaMp4->rCfaRange.u8VidStartChunkNo) {
		u8CurSampleNo =
			CfaMp4PrsStscForChunkToSample(prCfaMp4, Cfa_Mp4_Vid_Track,
						   prVInfo->u8CurPrsVidChunkNo,
						   &u4CurChunkSampleNums);
		prVInfo->u4NeedPrsSampleNums =
			u4CurChunkSampleNums - (u32) (prVInfo->u8CurPrsVidSampleNo - u8CurSampleNo);
	} else {
		if (prVInfo->u8CurPrsVidChunkNo == prCfaMp4->rCfaRange.u8VidEndChunkNo) {
			u8CurSampleNo =
				CfaMp4PrsStscForChunkToSample(prCfaMp4, Cfa_Mp4_Vid_Track,
							   prVInfo->u8CurPrsVidChunkNo,
							   &u4CurChunkSampleNums);
			prVInfo->u4NeedPrsSampleNums =
				(u32)(prCfaMp4->rCfaRange.u8VidEndSampleNo - u8CurSampleNo + 1);
		} else {
			u8CurSampleNo =
				CfaMp4PrsStscForChunkToSample(prCfaMp4, Cfa_Mp4_Vid_Track,
							   prVInfo->u8CurPrsVidChunkNo,
							   &u4CurChunkSampleNums);
			prVInfo->u4NeedPrsSampleNums = u4CurChunkSampleNums;		
		}
	}
	prVInfo->u8CurChunk1stSmpNo = u8CurSampleNo;
	if (TRUE == prVInfo->fgGetRangeOfst) {
		prCfaMp4->rCurOfst.u8VidCurOfst = prCfaMp4->rCfaRange.u8VidStartOffset;
		prVInfo->fgGetRangeOfst = FALSE;
	} else {
		prCfaMp4->rCurOfst.u8VidCurOfst =
			CfaMp4PrsStcoGetVideoFileOfst(prCfaMp4, Cfa_Mp4_Vid_Track,
						   prVInfo->u8CurPrsVidChunkNo,
						   prVInfo->u8CurPrsVidSampleNo);
	}
	prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_GET_V_PTS_TO_FIFO;
	CfaMp4AnaGetVPts2Fifo(pvSptHdl, prCfaMp4);
}


/*-----------------------------------------------------------------------------
* Name: vCfaMp4AnaPrsARange
*
* Description:
*	   prs audio range to demux index table,once prs only one chunk,prs  all sample in cur chunk
*
* Inputs:
*
* Outputs:
*
* Returns: the index of audio info
*
*-----------------------------------------------------------------------------*/
void CfaMp4AnaPrsARange(void *pvSptHdl, CfaMp4Inst *prCfaMp4)
{
	u64 u8CurSampleNo = 0;
	u32 u4CurChunkSampleNums = 0;

		/*DMXLOG_ERROR(TEXT("[CFA MP4]  PrsARange\n"));*/
	if (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudChunkNo ==
		prCfaMp4->rCfaRange.u8AudStartChunkNo[prCfaMp4->u4CurAudInfoId]) {
		u8CurSampleNo =
			CfaMp4PrsStscForChunkToSample(prCfaMp4, Cfa_Mp4_Aud_Track,
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudChunkNo,
			&u4CurChunkSampleNums);
		prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4NeedPrsSampleNums =
			u4CurChunkSampleNums -
			(u32) (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudSampleNo -
				  u8CurSampleNo);
	} else {
		if (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudChunkNo ==
			prCfaMp4->rCfaRange.u8AudEndChunkNo[prCfaMp4->u4CurAudInfoId]) {
			u8CurSampleNo =
				CfaMp4PrsStscForChunkToSample(prCfaMp4, Cfa_Mp4_Aud_Track,
				prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudChunkNo,
				&u4CurChunkSampleNums);
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4NeedPrsSampleNums =
				(u32) (prCfaMp4->rCfaRange.u8AudEndSampleNo[prCfaMp4->u4CurAudInfoId]
				- u8CurSampleNo + 1);
		}

		else {
			u8CurSampleNo =
				CfaMp4PrsStscForChunkToSample(prCfaMp4, Cfa_Mp4_Aud_Track,
				prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudChunkNo,
				&u4CurChunkSampleNums);
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4NeedPrsSampleNums =
				u4CurChunkSampleNums;
		}
	}
	if (TRUE == prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].fgGetRangeOfst) {
		prCfaMp4->rCurOfst.u8AudCurOfst =
			prCfaMp4->rCfaRange.u8AudStartOffset[prCfaMp4->u4CurAudInfoId];
		prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].fgGetRangeOfst = FALSE;
		}

	else {
		prCfaMp4->rCurOfst.u8AudCurOfst =
			CfaMp4PrsStcoGetVideoFileOfst(prCfaMp4, Cfa_Mp4_Aud_Track,
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudChunkNo,
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudSampleNo);
	}
	prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_GET_A_PTS_TO_FIFO;
	CfaMp4AnaGetAPts2Fifo(pvSptHdl, prCfaMp4);
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4AnaPrsSRange
*
* Description:
*	   prs subtitle range to demux index table,once prs only one chunk,prs	all sample in cur chunk
*
* Inputs:
*
* Outputs:
*
* Returns: the index of audio info
*
*-----------------------------------------------------------------------------*/
void CfaMp4AnaPrsSRange(void *pvSptHdl, CfaMp4Inst *prCfaMp4)
{
	u64 u8CurSampleNo = 0;
	u32 u4CurChunkSampleNums = 0;

	/*MLOG_DEBUG(TEXT("[CFA MP4] ___CurSub(PrsChunkNo): 0x%llx\n"),
	prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurPrsSubChunkNo);*/
	/*MLOG_DEBUG(TEXT("[CFA MP4] ___CurSub(PrsSmpNo): 0x%llx\n"),
	prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurPrsSubSampleNo);*/
	/*MLOG_DEBUG(TEXT("[CFA MP4] ___CurSub(InfoID): 0x%lx\n"),
	prCfaMp4->u4CurSubInfoId);*/
	if (prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurPrsSubChunkNo ==
		   prCfaMp4->rCfaRange.u8SubStartChunkNo[prCfaMp4->u4CurSubInfoId]) {
		u8CurSampleNo =
			CfaMp4PrsStscForChunkToSample(prCfaMp4, Cfa_Mp4_Sub_Track,
			prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurPrsSubChunkNo,
			&u4CurChunkSampleNums);
		prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u4NeedPrsSampleNums =
			u4CurChunkSampleNums -
			(u32) (prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurPrsSubSampleNo -
				  u8CurSampleNo);
	}

	else {
		if (prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurPrsSubChunkNo ==
			prCfaMp4->rCfaRange.u8SubEndChunkNo[prCfaMp4->u4CurSubInfoId]) {
			u8CurSampleNo =
				CfaMp4PrsStscForChunkToSample(prCfaMp4, Cfa_Mp4_Sub_Track,
				prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurPrsSubChunkNo,
				&u4CurChunkSampleNums);
			prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u4NeedPrsSampleNums =
				(u32) (prCfaMp4->rCfaRange.u8SubEndSampleNo[prCfaMp4->u4CurSubInfoId]
				- u8CurSampleNo + 1);
		}

		else {
			u8CurSampleNo =
				CfaMp4PrsStscForChunkToSample(prCfaMp4, Cfa_Mp4_Sub_Track,
				prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurPrsSubChunkNo,
				&u4CurChunkSampleNums);
			prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u4NeedPrsSampleNums =
				u4CurChunkSampleNums;
		}
	}
	if (TRUE == prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].fgGetRangeOfst) {
		prCfaMp4->rCurOfst.u8SubCurOfst =
			prCfaMp4->rCfaRange.u8SubStartOffset[prCfaMp4->u4CurSubInfoId];
		prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].fgGetRangeOfst = FALSE;
	}

	else {
		prCfaMp4->rCurOfst.u8SubCurOfst =
			CfaMp4PrsStcoGetVideoFileOfst(prCfaMp4, Cfa_Mp4_Sub_Track,
			prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurPrsSubChunkNo,
			prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurPrsSubSampleNo);
	}

	prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_GET_S_PTS_TO_FIFO;
	CfaMp4AnaGetSPts2Fifo(pvSptHdl, prCfaMp4);
}


/*-----------------------------------------------------------------------------
* Name: vCfaMp4AnaGetAPts2Fifo
*
* Description: get the pts of cur prs audio sample ,add the adts header to sample and
throw the sample to afifo,

*
* Inputs:
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
void CfaMp4AnaGetAPts2Fifo(void *pvSptHdl, CfaMp4Inst *prCfaMp4)
{
	u64 u8FileOffset = 0;
	u64 u8Temp = 0;
	u32 u4TempSampleSize = 0;
	MRESULT mrRet = RET_DMX_OK;
	u32 i = 0;
	TSampleInfo *pTemp = NULL;

#if MP4_SUPPORT_FRAGMENT
	if (prCfaMp4->eCfaMoofType == TYPE_ONLY_MOOF) {
		DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4][%s] line %d ,enter,prCfaMp4->eAudType %d!!\n"),
			__func__,__LINE__,prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].eAudType);
	} else
#endif
	pTemp = prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pTSampleInfo;
	if (!prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].fgTxAacSCDone
		   && !prCfaMp4->fgFinished
		   && prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pucDecSpecInfo) {
		DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4][%s]  TxAudHDRInfo !\n"), __func__);
		prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].fgTxAacSCDone = TRUE;
		prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_GET_A_PTS_TO_FIFO;
		prCfaMp4->eCurPrsSampleType = CFA_MP4_PRS_BIT_STRM_TYPE_A;
		mrRet =
			Spt4CfaBuf2AFifo(pvSptHdl,
					 prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pucDecSpecInfo,
					 prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4DecSpecSz +
					 4, prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4TrackID,
					 prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].eCfaAudType);
		if (RET_DMX_OK != mrRet) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4] Spt4CfaBuf2AFifo error ret = %d\n"),
					mrRet);
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
			return;
		}
	}

	else if ((FALSE == prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].fgAddAdtsDone) &&
		 (FALSE == prCfaMp4->fgFinished) &&
		 (FALSE == prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].fgSyncIV)) {
		u32 u4AudSamplePerSec =
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4AudSamplePerSec;
		u32 u4AudChannels =
			(u32)prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u2AudChannels;
		prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].fgAddAdtsDone = TRUE;
		*prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pucADTSBuf = 0xFF;
#if MP4_SUPPORT_FRAGMENT
		if (prCfaMp4->eCfaMoofType != TYPE_ONLY_MOOF) {
#endif
		u8Temp =
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudSampleNo -
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurAudTableStartSampleNo;
		u4TempSampleSize = (pTemp + u8Temp)->u4SampleSize + 7;
		if (0 == prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4SampleSize) {
			u4TempSampleSize = (pTemp + u8Temp)->u4SampleSize + 7;
		}

		else {
			u4TempSampleSize =
				prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4SampleSize + 7;
		}
#if MP4_SUPPORT_FRAGMENT
		} else {
			u4TempSampleSize =
				prCfaMp4->rAudInf.u8Len + 7;
		}
#endif
		*(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pucADTSBuf + 1) = 0xF9;
		*(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pucADTSBuf + 2) =
			(u8) ((1U << 6U) | ((u4AudSamplePerSec << 2U) & (u32)0x3C) |
				 ((u4AudChannels >> 2U) & (u32)0x1));
		*(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pucADTSBuf + 3) =
			(u8) (((u4AudChannels & (u32)0x3) << 6U) | ((u4TempSampleSize >> 11U) & (u32)0x3));
		*(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pucADTSBuf + 4) =
			(u8) ((u4TempSampleSize >> 3U) & (u32)0xFF);
		*(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pucADTSBuf + 5) =
			(u8) ((u4TempSampleSize << 5U) & (u32)0xE0) | (((u32)0x7FF >> 6U) & (u32)0x1F);
		*(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pucADTSBuf + 6) =
			((0x7FF << 2) & 0xFC);
		prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_GET_A_PTS_TO_FIFO;
		prCfaMp4->eCurPrsSampleType = CFA_MP4_PRS_BIT_STRM_TYPE_A;
		mrRet =
			Spt4CfaBuf2AFifo(pvSptHdl,
					  prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pucADTSBuf,
					  7, prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4TrackID,
					  prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].eCfaAudType);
		if (RET_DMX_OK != mrRet) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4] Spt4CfaBuf2AFifo error ret = %d\n"),
					mrRet);
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
			return;
		}
	}

	else if (AVCODEC_ID_PCM != prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].eAudType) {
		u8FileOffset = prCfaMp4->rCurOfst.u8AudCurOfst;

		if (AVCODEC_ID_AAC == prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].eAudType)
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].fgAddAdtsDone = FALSE;
		else
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].fgAddAdtsDone = TRUE;

#if MP4_SUPPORT_FRAGMENT
		if (prCfaMp4->eCfaMoofType != TYPE_ONLY_MOOF) {
#endif

		prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4NeedPrsSampleNums--;
		u8Temp =
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudSampleNo -
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurAudTableStartSampleNo;
		if (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudSampleNo ==
			(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurAudTableEndSampleNo - 1)) {
			prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_RELOAD_TABLE;
		}

		else {
			prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_GET_A_PTS_TO_FIFO;
		}
		if (0 == prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4NeedPrsSampleNums) {
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudChunkNo++;
			if (CFA_MP4_ANA_RELOAD_TABLE != prCfaMp4->eCurCfaMp4AnaSt)
				prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_NEXT_STATE;
		}
		if (!(prCfaMp4->u4PrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_V)) {
			for (i = 0; i < prCfaMp4->u2AudStrmNums; i++) {
				if (prCfaMp4->rCfaMp4AInf[i].u4TrackID == prCfaMp4->u4CurAudPlayId) {
					if (!(prCfaMp4->rCfaRange.u8AudEndChunkNo[i])
						&& !(prCfaMp4->rCfaRange.u8AudEndSampleNo[i])) {
						DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
								TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
						CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
						return;
					}
				}

				else {
					break;
				}
			}
		}
		if (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudSampleNo >=
			 prCfaMp4->rCfaRange.u8AudEndSampleNo[prCfaMp4->u4CurAudInfoId]) {
			if (prCfaMp4->u4PrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_V) {
				prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_NEXT_STATE;
			} else {
				if (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4TrackID ==
					prCfaMp4->u4CurAudPlayId) {
#if MP4_SUPPORT_FRAGMENT
					if (prCfaMp4->eCfaMoofType == TYPE_MOOV_AND_MOOF) {
						DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
							TEXT("[CFA MP4][%s] line %d,Moov data parse over,type transfore TYPE_ONLY_MOOF!\n"),__func__,__LINE__);
						prCfaMp4->eCfaMoofType = TYPE_ONLY_MOOF;
						mrRet = Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaMp4->u8CfaCurMoofOffset, 8,
									   (u8 *) &prCfaMp4->ptrPfrMemAddress);
						if (RET_DMX_OK != mrRet) {
							DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
								TEXT("[CFA MP4] Spt4CfaPbb2SyncBuf error ret = %d\n"),
								mrRet);
							CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
							return;
						}
						prCfaMp4->fgGetMoofData = FALSE;
						prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_MOOF_HEADER;
						if (prCfaMp4->u4PrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_V) {
							prCfaMp4->u4CurPrsFlg |= CFA_MP4_PRS_BIT_STRM_TYPE_V;
						}
						if (prCfaMp4->u4PrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_A) {
							prCfaMp4->u4CurPrsFlg |= CFA_MP4_PRS_BIT_STRM_TYPE_A;
						}
						DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
							TEXT("[CFA MP4][%s] line %d,prCfaMp4->u8CfaCurMoofOffset 0x%llx!\n"),
							__func__,__LINE__,prCfaMp4->u8CfaCurMoofOffset);
						return;
					}
#endif
					DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
							TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
					CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
					return;
				}

				else {
					prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_NEXT_STATE;
				}
			}
		}
		if (0 == prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4SampleSize) {
			prCfaMp4->rCurOfst.u8AudCurOfst += (pTemp + u8Temp)->u4SampleSize;
		}

		else {
			prCfaMp4->rCurOfst.u8AudCurOfst +=
				prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4SampleSize;
		}
		if (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudSampleNo ==
			   prCfaMp4->rCfaRange.u8AudStartSampleNo[prCfaMp4->u4CurAudInfoId]) {
			prCfaMp4->u8Apts[prCfaMp4->u4CurAudInfoId] =
				prCfaMp4->rCfaRange.u8AudPts[prCfaMp4->u4CurAudInfoId];
		}

		else {
			prCfaMp4->u8Apts[prCfaMp4->u4CurAudInfoId] =
				prCfaMp4->u8CurAPts[prCfaMp4->u4CurAudInfoId];
		}
		prCfaMp4->u8CurAPts[prCfaMp4->u4CurAudInfoId] =
			CfaMp4PrsSttsGetPts(prCfaMp4, Cfa_Mp4_Aud_Track,
					prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudSampleNo
#if SPECIAL_LPCM_SUPPORT
					, 1
#endif
			);
		prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudSampleNo++;
		if (0 == prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4SampleSize) {
			prCfaMp4->rAudInf.u8Len = (pTemp + u8Temp)->u4SampleSize;
		}

		else {
			prCfaMp4->rAudInf.u8Len =
				prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4SampleSize;
		}
#if MP4_SUPPORT_FRAGMENT
		} else {
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] line %d , Audio TYPE_ONLY_MOOF!\n"),__func__,__LINE__);
			prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_MOOF_TRUN;

			if (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunSampleNo >=
					prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunSampleCount ) {
				mrRet = CfaMp4MoofInit(&(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo));
				if (mrRet != RET_DMX_OK) {
					DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA MP4]CfaMp4MoofInit audio fail,call CfaMp4FinishPrs!\n"));
					CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
					return;
				}
				if ((prCfaMp4->u4CurPrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_V) &&
					(prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.fgTrunBufValid == TRUE)) {
					prCfaMp4->eCurStreamType = CFA_MP4_VIDEO;
				} else {
					prCfaMp4->eCurStreamType = CFA_MP4_UNKNOWN;
					prCfaMp4->u8CfaCurMoofOffset = prCfaMp4->u8CfaNextMoofOffset;
					prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_NEXT_STATE;//CFA_MP4_ANA_PRS_MOOF_HEADER
				}
			}
		}
#endif

		if (FALSE == prCfaMp4->fgFinished)
			Spt4CfaPTSNotify(pvSptHdl, prCfaMp4->u8Apts[prCfaMp4->u4CurAudInfoId]);

		prCfaMp4->rAudInf.u8FileOfst = u8FileOffset;

		prCfaMp4->rAudInf.u8Pts = prCfaMp4->u8Apts[prCfaMp4->u4CurAudInfoId];
		prCfaMp4->rAudInf.fgUnitStart = FALSE;
		prCfaMp4->rAudInf.u4PrsStrmId = prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4TrackID;
		/*add by zhiwei chen for streamid 2011.3.17*/
		prCfaMp4->rAudInf.eAudType = prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].eCfaAudType;
		/*add by mtk68014*/
		prCfaMp4->eCurPrsSampleType = CFA_MP4_PRS_BIT_STRM_TYPE_A;
		DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				   TEXT("[CFA MP4][%s]	CurAud(PrsSmpNo): 0x%llx, CurAud(PrsSmpOfst): 0x%llx,")
				   TEXT("CurAud(PrsSmpLength): 0x%llx\r\n"),
				   __func__,
				   prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudSampleNo -
				   1, prCfaMp4->rAudInf.u8FileOfst, prCfaMp4->rAudInf.u8Len);
		if (FALSE == prCfaMp4->fgFinished) {
			if ((CfaMp4GetRangeEa(prCfaMp4) <
				  (prCfaMp4->rAudInf.u8FileOfst +
				   prCfaMp4->rAudInf.u8Len)) || (prCfaMp4->rAudInf.u8FileOfst <
								prCfaMp4->u8Ca)
								|| (0 == prCfaMp4->rAudInf.u8Len)) {
				DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
			} else if (prCfaMp4->rAudInf.u8Pts < prCfaMp4->rCfaRange.u8SeekPts) {
				prCfaMp4->u8Ca =
					prCfaMp4->rAudInf.u8FileOfst + prCfaMp4->rAudInf.u8Len;
				prCfaMp4->fgHasSkipData = TRUE;
				DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4]%s line %d, Audio Data PTS:%lld(1/90000s) < SeekTime:%lld(1/90000s)")
					TEXT("skip this audio data!\r\n"),
					__func__, __LINE__, prCfaMp4->rAudInf.u8Pts, prCfaMp4->rCfaRange.u8SeekPts);
			} else {
				prCfaMp4->u8Ca =
					prCfaMp4->rAudInf.u8FileOfst + prCfaMp4->rAudInf.u8Len;
				prCfaMp4->rAudInf.u8TotalAULen = 0;
				mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &prCfaMp4->rAudInf);
				if (RET_DMX_OK != mrRet) {
					DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA MP4] Spt4CfaPbb2AFifoAUCtrl error ret = %d\n"),
						mrRet);
					CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
					return;
				}
				if (!(prCfaMp4->u4PrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_V))
				{
					Sleep(1);/*for bug 112296*/
				}

			}
		}
	}

	else {
		CfaMp4LpcmChunk2Fifo(pvSptHdl, prCfaMp4);
	}
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4AnaGetSPts2Fifo
*
* Description: get the pts of cur prs subtitle sample , and throw the sample to sfifo,

*
* Inputs:
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
void CfaMp4AnaGetSPts2Fifo(void *pvSptHdl, CfaMp4Inst *prCfaMp4)
{
	MRESULT mrRet = RET_DMX_OK;
	u64 u8FileOffset = 0;
	u64 u8Temp = 0;
	u16 u2Length = 0;
	TSampleInfo *pTemp = NULL;
	u8 pucHdrBuf[SUBTXT_LENGTH_HEADER] = { 0 };

	if (!prCfaMp4->fgGetSPLen) {
		mrRet = Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaMp4->rCurOfst.u8SubCurOfst, (u64)SUBTXT_LENGTH_HEADER,
			(u8 *) &prCfaMp4->ptrPfrMemAddress);

		if (RET_DMX_OK != mrRet) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4] Spt4CfaPbb2SyncBuf error ret = %d\n"),
				mrRet);
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
		}

		prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_GET_S_PTS_TO_FIFO;
		prCfaMp4->fgGetSPLen = TRUE;
		return;
	}

	pTemp = prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].pTSampleInfo;
	u8FileOffset = prCfaMp4->rCurOfst.u8SubCurOfst;
	prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u4NeedPrsSampleNums--;
	u8Temp =
		prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurPrsSubSampleNo -
		prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurSubTableStartSampleNo;
	if (0 == prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u4SampleSize) {
		prCfaMp4->rCurOfst.u8SubCurOfst += (pTemp + u8Temp)->u4SampleSize;
	}

	else {
		prCfaMp4->rCurOfst.u8SubCurOfst +=
			prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u4SampleSize;
	}
	if (prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurPrsSubSampleNo ==
		(prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurSubTableEndSampleNo - 1)) {
		prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_RELOAD_TABLE;
	}

	else {
		prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_GET_S_PTS_TO_FIFO;
	}
	if (0 == prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u4NeedPrsSampleNums) {
		prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurPrsSubChunkNo++;
		if (CFA_MP4_ANA_RELOAD_TABLE != prCfaMp4->eCurCfaMp4AnaSt)
			prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_NEXT_STATE;

	}
	if (prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurPrsSubSampleNo ==
		 prCfaMp4->rCfaRange.u8SubEndSampleNo[prCfaMp4->u4CurSubInfoId]) {
		prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_NEXT_STATE;
	}
	if (prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurPrsSubSampleNo ==
		 prCfaMp4->rCfaRange.u8SubStartSampleNo[prCfaMp4->u4CurSubInfoId]) {
		prCfaMp4->u8Spts[prCfaMp4->u4CurSubInfoId] =
			prCfaMp4->rCfaRange.u8SubPts[prCfaMp4->u4CurSubInfoId];
	}

	else {
		prCfaMp4->u8Spts[prCfaMp4->u4CurSubInfoId] =
			prCfaMp4->u8CurSPts[prCfaMp4->u4CurSubInfoId];
	}
	prCfaMp4->u8CurSPts[prCfaMp4->u4CurSubInfoId] =
		CfaMp4PrsSttsGetPts(prCfaMp4, Cfa_Mp4_Sub_Track,
				prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].
				u8CurPrsSubSampleNo
#if SPECIAL_LPCM_SUPPORT
				, 1
#endif
		);
	prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurPrsSubSampleNo++;
	prCfaMp4->rSubInf.u8FileOfst = u8FileOffset;
	prCfaMp4->rSubInf.fgUnitStart = FALSE;
	prCfaMp4->rSubInf.u8Pts = prCfaMp4->u8Spts[prCfaMp4->u4CurSubInfoId];
	prCfaMp4->rSubInf.u8EndPts = INVALID_TIMESTAMP;
	prCfaMp4->rSubInf.u4SpuPos = 0;
	prCfaMp4->rSubInf.u8Len = (pTemp + u8Temp)->u4SampleSize;
	prCfaMp4->rSubInf.u4PrsStrmId =
		prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u4TrackID;
	/*dd by guoqing yang for sb sream id*/
	prCfaMp4->eCurPrsSampleType = CFA_MP4_PRS_BIT_STRM_TYPE_SP;
	if ((prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].eSubType == MP4_SUB_TEXT) &&
		(prCfaMp4->rSubInf.u8Len > SUBTXT_LENGTH_HEADER)) {
		mm_memcpy((void *) (pucHdrBuf), (void *) (u8 *) prCfaMp4->ptrPfrMemAddress,
			 sizeof(pucHdrBuf));

		LOADB_WORD(pucHdrBuf, u2Length);

		DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4][%s] u8Len 0x%llx,u2Length %x,u8FileOffset 0x%llx\n"),
			__func__, prCfaMp4->rSubInf.u8Len,u2Length,u8FileOffset);

		if ((u16)(prCfaMp4->rSubInf.u8Len - SUBTXT_LENGTH_HEADER) == u2Length) {
			prCfaMp4->rSubInf.u8FileOfst = u8FileOffset + SUBTXT_LENGTH_HEADER;
			prCfaMp4->rSubInf.u8Len -= SUBTXT_LENGTH_HEADER;
		} else {
			DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4][%s] SubTxt No Length Region!\n"), __func__);
		}

	}
	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4][%s] u8FileOfst 0x%llx, u8Pts 0x%llx,u8Len 0x%llx,u4PrsStrmId %d\n"),
		__func__,prCfaMp4->rSubInf.u8FileOfst,prCfaMp4->rSubInf.u8Pts,
		prCfaMp4->rSubInf.u8Len,prCfaMp4->rSubInf.u4PrsStrmId);

	if (FALSE == prCfaMp4->fgFinished) {
		if ((CfaMp4GetRangeEa(prCfaMp4) <
			  (prCfaMp4->rSubInf.u8FileOfst +
			   prCfaMp4->rSubInf.u8Len)) || (prCfaMp4->rSubInf.u8FileOfst < prCfaMp4->u8Ca)
			 || (0 == prCfaMp4->rSubInf.u8Len)) {
			DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
		}

		else {
			prCfaMp4->u8Ca = prCfaMp4->rSubInf.u8FileOfst + prCfaMp4->rSubInf.u8Len;
			mrRet = Spt4CfaPbb2SpFifoAUCtrl(pvSptHdl, &prCfaMp4->rSubInf, 0);
			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4] Spt4CfaPbb2SpFifoAUCtrl error ret = %d\n"),
					mrRet);
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
				return;
			}
			prCfaMp4->fgGetSPLen = FALSE;
		}
	}
}


/*-----------------------------------------------------------------------------
* Name: GetMp4VPictType
*
* Description: get the Pictype,

*
* Inputs:
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
void GetMp4VPictType(CfaMp4Inst *prCfaMp4)
{
	u8 u1Temp1 = 0;
	u8 u1Temp2 = 0;
	u32 u4PictSizeOft = 0;	/*kip frm type,codecid,start code,version,temporalreferance*/
	u8 pucHdrBuf[20] = { 0 };

	mm_memcpy((void *) (pucHdrBuf), (void *) (u8 *) prCfaMp4->ptrPfrMemAddress,
			 sizeof(pucHdrBuf));

	switch (prCfaMp4->eVidType) {
	case CFA_VID_H263_SORENSON: {
			u4PictSizeOft = (u32)3;
			LOAD_BYTE(pucHdrBuf + u4PictSizeOft, u1Temp1);
			u1Temp1 &= (u8)0x03;
			u1Temp1 = u1Temp1 << 1;
			LOAD_BYTE(pucHdrBuf + u4PictSizeOft + 1, u1Temp2);
			u1Temp2 &= (u8)0x80;
			u1Temp2 = u1Temp2 >> 7;
			u1Temp1 = u1Temp1 + u1Temp2;
			if (0 == u1Temp1)
				u4PictSizeOft += (u32)3;


			else if (1 == u1Temp1)
				u4PictSizeOft += 5;


			else
				u4PictSizeOft += 1;

			LOAD_BYTE(pucHdrBuf + u4PictSizeOft, u1Temp2);
			u1Temp2 &= (u8)0x60;
			if ((u8)0 == u1Temp2)
				prCfaMp4->rVidInf.eTxMode = CFA_PTM_H263_SORENSON_I;
			else if (0x20 == u1Temp2)
				prCfaMp4->rVidInf.eTxMode = CFA_PTM_H263_SORENSON_P;
			else if (0x40 == u1Temp2)
				prCfaMp4->rVidInf.eTxMode = CFA_PTM_H263_SORENSON_P;	/*eed modify*/
			else
				prCfaMp4->rVidInf.eTxMode = CFA_PTM_H263_SORENSON_P;

			prCfaMp4->fgSh263GetedHdr = FALSE;
		}
		break;
	case CFA_VID_VP6: {
			LOAD_BYTE(pucHdrBuf, u1Temp1);
			u1Temp1 &= (u8)0x80;
			if ((u8)0 == u1Temp1)
				prCfaMp4->rVidInf.eTxMode = CFA_PTM_ONE_PIC_VP6_I;
			else
				prCfaMp4->rVidInf.eTxMode = CFA_PTM_ONE_PIC_VP6_P;

			prCfaMp4->fgSh263GetedHdr = FALSE;
		}
		break;
	case CFA_VID_VP8: {
			LOAD_BYTE(pucHdrBuf, u1Temp1);
			u1Temp1 &= (u8)0x01;
			if ((u8)0 == u1Temp1)
				prCfaMp4->rVidInf.eTxMode = CFA_PTM_ONE_PIC_VP8_I;
			else
				prCfaMp4->rVidInf.eTxMode = CFA_PTM_ONE_PIC_VP8_P;

			prCfaMp4->fgSh263GetedHdr = FALSE;
		}
		break;
	case CFA_VID_DIVX3: {
			LOAD_BYTE(pucHdrBuf, u1Temp1);
			u1Temp1 &= (u8)0x40;
			if (0 == u1Temp1)
				prCfaMp4->rVidInf.eTxMode = CFA_PTM_ONE_PIC_DX3_I;
			else
				prCfaMp4->rVidInf.eTxMode = CFA_PTM_ONE_PIC_DX3_P;

			prCfaMp4->fgSh263GetedHdr = FALSE;
		}
		break;
	case CFA_VID_RV40: {
			u32 u4Offset = 0;
			u32 TempSize = 0;
			u8 u1SliceElemNum = 0;
			u8 u1TotalSliceNum = (prCfaMp4->pu1HdrBuf[0]) + 1;
			u8 u1FrmType = prCfaMp4->pu1HdrBuf[0];

			prCfaMp4->rSliceInf.u1TotalSliceNum = u1TotalSliceNum;
			u4Offset++;

			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA][mp4]u1TotalSliceNum:0x%x	\r\n"),
					 prCfaMp4->rSliceInf.u1TotalSliceNum);
			for (u1SliceElemNum = 0; u1SliceElemNum < u1TotalSliceNum;
				  u1SliceElemNum++) {
				prCfaMp4->rSliceInf.rSliceInf[u1SliceElemNum].u1SliceElemNum =
					u1SliceElemNum + 1;
				u4Offset += 4;
				LOADL_DWRD(prCfaMp4->pu1HdrBuf + u4Offset, TempSize);
				u4Offset += 4;
				prCfaMp4->rSliceInf.rSliceInf[u1SliceElemNum].u2SliceElemSize =
					TempSize;
				if (u1SliceElemNum) {
					prCfaMp4->rSliceInf.rSliceInf[u1SliceElemNum - 1].u2SliceElemSize =
					TempSize -
						prCfaMp4->rSliceInf.rSliceInf[u1SliceElemNum - 1].u2SliceElemSize;
					DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA][mp4]u1SliceElemNum[%d]:0x%x, u2SliceElemSize[%d]:0x%x")
						TEXT("\r\n"),
					u1SliceElemNum - 1,
					prCfaMp4->rSliceInf.rSliceInf[u1SliceElemNum - 1].u1SliceElemNum,
					u1SliceElemNum - 1,
					prCfaMp4->rSliceInf.rSliceInf[u1SliceElemNum - 1].u2SliceElemSize);
				}
			}
			prCfaMp4->rSliceInf.rSliceInf[prCfaMp4->rSliceInf.u1TotalSliceNum -
							   1].u2SliceElemSize =
				prCfaMp4->rVidInf.u8Len - TempSize - u4Offset;
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					 TEXT("[CFA][mp4]u1SliceElemNum[%d]:0x%x, u2SliceElemSize[%d]:0x%x	\r\n"),
					 u1TotalSliceNum - 1,
					 prCfaMp4->rSliceInf.rSliceInf[u1TotalSliceNum - 1].u1SliceElemNum,
					 u1TotalSliceNum - 1,
					 prCfaMp4->rSliceInf.rSliceInf[u1TotalSliceNum - 1].u2SliceElemSize);
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					 TEXT("[CFA][mp4]u8Len:0x%llx, u8Ca:0x%llx, u4Offset:0x%x,")
					 TEXT("TempSize:0x%x.  \r\n"),
					 prCfaMp4->rVidInf.u8Len, prCfaMp4->u8Ca, u4Offset,
					 TempSize);
			prCfaMp4->rVidInf.u8FileOfst += u4Offset;
			prCfaMp4->u8Ca = prCfaMp4->rVidInf.u8FileOfst;
			prCfaMp4->rVidInf.u8Len -= u4Offset;
			u1FrmType = prCfaMp4->pu1HdrBuf[u4Offset];
			if (0x00 == (u1FrmType & 0x60))
				prCfaMp4->rVidInf.eTxMode = CFA_PTM_RM_INTRAPIC;
			else if (0x20 == (u1FrmType & 0x60))
				prCfaMp4->rVidInf.eTxMode = CFA_PTM_RM_FORCED_INTRAPIC;
			else if (0x40 == (u1FrmType & 0x60))
				prCfaMp4->rVidInf.eTxMode = CFA_PTM_RM_INTERPIC;
			else
				prCfaMp4->rVidInf.eTxMode = CFA_PTM_RM_TRUEBPIC;

		}
		break;
	case CFA_VID_MJPEG: {
			prCfaMp4->rVidInf.eTxMode = CFA_PTM_MJPEG_I;
		}
		break;
	case CFA_VID_WMV7:
		if (0x01 == ((prCfaMp4->pu1HdrBuf[0]) >> 6))/* frame*/
			prCfaMp4->rVidInf.eTxMode = (CFA_PTM_WMV_P);
		else
			prCfaMp4->rVidInf.eTxMode = (CFA_PTM_WMV_I);

		break;
	case CFA_VID_WMV8:
		if (0x01 == ((prCfaMp4->pu1HdrBuf[0]) >> 7))
			prCfaMp4->rVidInf.eTxMode = (CFA_PTM_WMV_P);
		else
			prCfaMp4->rVidInf.eTxMode = (CFA_PTM_WMV_I);

		break;
	case CFA_VID_WMV9:
		u4PictSizeOft = 8;
		if (prCfaMp4->fgPrsSeqFrameInterpolation)
			u4PictSizeOft--;

		u4PictSizeOft -= 2;
		if (prCfaMp4->fgPrsPreProcRange)
			u4PictSizeOft--;

		if ((prCfaMp4->pu1HdrBuf[0]) & (1 << --u4PictSizeOft)) {
			prCfaMp4->rVidInf.eTxMode = (CFA_PTM_WMV_P);
		} else {
			if (prCfaMp4->u4PrsNumBFrames == 0) {
				prCfaMp4->rVidInf.eTxMode = (CFA_PTM_WMV_I);
			} else {
				if ((prCfaMp4->pu1HdrBuf[0]) & (1 << --u4PictSizeOft))
					prCfaMp4->rVidInf.eTxMode = (CFA_PTM_WMV_I);
				else
					prCfaMp4->rVidInf.eTxMode = (CFA_PTM_WMV_B);
			}
		}
		break;
	default:
		break;
	}
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4AnaGetVPts2Fifo
*
* Description: get the pts of cur prs audio sample , and throw the sample to vfifo,

*
* Inputs:
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
void CfaMp4AnaGetVPts2Fifo(void *pvSptHdl, CfaMp4Inst *prCfaMp4)
{
	u64 u8FileOffset = 0;
	u64 u8Temp = 0;
	TCfaMp4VInf *prVInfo = &prCfaMp4->rCfaMp4VInf;
	MRESULT mrRet = RET_DMX_OK;

#if MP4_SUPPORT_FRAGMENT
	if (prCfaMp4->eCfaMoofType == TYPE_ONLY_MOOF) {
		DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4][%s] line %d ,prCfaMp4->eCfaMoofType == TYPE_ONLY_MOOF;prCfaMp4->eVidType %d!\n"),__func__,__LINE__,prCfaMp4->eVidType);
	} else {
#else
	if ((NULL == prCfaMp4) || (NULL == prVInfo->pTSampleInfo))
		return;
#endif
#if MP4_SUPPORT_FRAGMENT
	}
#endif

	if (!prVInfo->fgTxMp4SCDone && !prCfaMp4->fgFinished) {
		if ((CFA_VID_MPEG4 == prCfaMp4->eVidType) || (CFA_VID_DIVX4 == prCfaMp4->eVidType)
			|| (CFA_VID_MPEG2 == prCfaMp4->eVidType)) {
			prVInfo->fgTxMp4SCDone = TRUE;
			prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_GET_V_PTS_TO_FIFO;
			mrRet =
				Spt4CfaBuf2VFifo(pvSptHdl, prVInfo->pucMpeg4CodecSC, 0, CFA_PTM_SAME_POS,
						 prCfaMp4->eVidType, prVInfo->u4DecSpecSz[0]);
			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4] Spt4CfaBuf2VFifo error ret = %d\n"),
					mrRet);
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
				return;
			}
		} else if (CFA_VID_VC1 == prCfaMp4->eVidType) {
			prCfaMp4->rCfaMp4VInf.fgTxMp4SCDone = TRUE;
			prCfaMp4->rCfaMp4VInf.fgCodecSCDone = FALSE;
			prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_GET_V_PTS_TO_FIFO;
			mrRet =
				Spt4CfaBuf2VFifo(pvSptHdl, prCfaMp4->rCfaMp4VInf.pucWVc1CodecSC, 0,
						 CFA_PTM_SAME_POS, prCfaMp4->eVidType,
						 prVInfo->u4DecSpecSz[0]);
			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4] Spt4CfaBuf2VFifo error ret = %d\n"),
					mrRet);
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
				return;
			}
		} else if ((CFA_VID_WMV7 == prCfaMp4->eVidType || CFA_VID_WMV8 == prCfaMp4->eVidType
			|| CFA_VID_WMV9 == prCfaMp4->eVidType) && (NULL !=
			prCfaMp4->rCfaMp4VInf.pucMpeg4CodecSC)) {
			prCfaMp4->rCfaMp4VInf.fgTxMp4SCDone = TRUE;
			prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_GET_V_PTS_TO_FIFO;
			if (prCfaMp4->rCfaMp4VInf.u4DecSpecSz[0] >= 4) {
				prCfaMp4->fgPrsSeqFrameInterpolation =
					(prCfaMp4->rCfaMp4VInf.pucMpeg4CodecSC[3] & (u8)0x02) >> 1U;
				prCfaMp4->fgPrsPreProcRange =
					(prCfaMp4->rCfaMp4VInf.pucMpeg4CodecSC[3] & (u8)0x80) >> 7U;
				prCfaMp4->u4PrsNumBFrames =
					(prCfaMp4->rCfaMp4VInf.pucMpeg4CodecSC[3] & (u8)0x70) >> 4U;
				DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4]  vCfaMp4AnaGetVPts2Fifo fgPrsSeqFrameInterpolation[%x]")
					TEXT("gPrsPreProcRange[%d] u4PrsNumBFrames[%d]\n"),
					prCfaMp4->fgPrsSeqFrameInterpolation,
					prCfaMp4->fgPrsPreProcRange,
					prCfaMp4->u4PrsNumBFrames);
				DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4]  vCfaMp4AnaGetVPts2Fifo pucMpeg4CodecSC[%x,%x,%x,%x]\n"),
					prCfaMp4->rCfaMp4VInf.pucMpeg4CodecSC[0],
					prCfaMp4->rCfaMp4VInf.pucMpeg4CodecSC[1],
					prCfaMp4->rCfaMp4VInf.pucMpeg4CodecSC[2],
					prCfaMp4->rCfaMp4VInf.pucMpeg4CodecSC[3]);
			}
			prCfaMp4->rVidInf.u4PrsStrmId = prCfaMp4->u4CurVidInfoId;
			prCfaMp4->rVidInf.eTxMode = CFA_PTM_WMV_SEQHDR;
			prCfaMp4->rVidInf.eVidType = prCfaMp4->eVidType;
			prCfaMp4->rVidInf.fgUnitStart = TRUE;
			prCfaMp4->rVidInf.u8Len = prCfaMp4->rCfaMp4VInf.u4DecSpecSz[0];
			prCfaMp4->rVidInf.u8TotalAULen = prCfaMp4->rCfaMp4VInf.u4DecSpecSz[0];
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4]	vCfaMp4AnaGetVPts2Fifo eVidType: WMV u4DecSpecSz[%d], eVidType: %d\n"),
			prCfaMp4->rCfaMp4VInf.u4DecSpecSz[0], prCfaMp4->eVidType);
			mrRet =
				Spt4CfaBuf2VFifoAUCtrl(pvSptHdl, prCfaMp4->rCfaMp4VInf.pucMpeg4CodecSC,
						   &(prCfaMp4->rVidInf));
			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4] Spt4CfaBuf2VFifo error ret = %d\n"),
					mrRet);
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
				return;
			}
		} else {		  /*.264*/
			u32 u4Len = 0;

			if (0 != prVInfo->u1VPSNum[prCfaMp4->u4CurAvcEsdIndex]) {
				u8 *ptemp =
					(u8 *) prVInfo->pucVPSInfo[prCfaMp4->u4CurAvcEsdIndex] +
					prVInfo->u4NextVPSPos;
				prVInfo->u1VPSNum[prCfaMp4->u4CurAvcEsdIndex]--;
				LOADB_WORD(ptemp, u4Len);
				ptemp += 2;
				dmx_memcpy((prVInfo->pucDecSpecInfo[prCfaMp4->u4CurAvcEsdIndex] +
						 4), ptemp, u4Len);
				ptemp += u4Len;
				prVInfo->u4NextVPSPos =
					ptemp - prVInfo->pucVPSInfo[prCfaMp4->u4CurAvcEsdIndex];
			} else if (0 != prVInfo->u1SeqParamNum[prCfaMp4->u4CurAvcEsdIndex]) {
				u8 *ptemp =
					(u8 *) prVInfo->pucSeqParamInfo[prCfaMp4->u4CurAvcEsdIndex] +
					prVInfo->u4NextSeqParamPos;
				prVInfo->u1SeqParamNum[prCfaMp4->u4CurAvcEsdIndex]--;
				LOADB_WORD(ptemp, u4Len);
				ptemp += 2;
				dmx_memcpy((prVInfo->pucDecSpecInfo[prCfaMp4->u4CurAvcEsdIndex] +
						   4), ptemp, u4Len);
				ptemp += u4Len;
				prVInfo->u4NextSeqParamPos =
					ptemp - prVInfo->pucSeqParamInfo[prCfaMp4->u4CurAvcEsdIndex];
			} else if (0 != prVInfo->u1PicParamNum[prCfaMp4->u4CurAvcEsdIndex]) {
				u8 *ptemp =
					(u8 *) prVInfo->pucPicParamInfo[prCfaMp4->u4CurAvcEsdIndex] +
					prVInfo->u4NextPicParamPos;
				prVInfo->u1PicParamNum[prCfaMp4->u4CurAvcEsdIndex]--;
				LOADB_WORD(ptemp, u4Len);
				ptemp += 2;
				dmx_memcpy((prVInfo->pucDecSpecInfo[prCfaMp4->u4CurAvcEsdIndex] +
						 4), ptemp, u4Len);
				ptemp += u4Len;
				prVInfo->u4NextPicParamPos =
					ptemp - prVInfo->pucPicParamInfo[prCfaMp4->u4CurAvcEsdIndex];
			}
			mrRet = Spt4CfaBuf2VFifo(pvSptHdl,
				prVInfo->pucDecSpecInfo[prCfaMp4->u4CurAvcEsdIndex],
				0, CFA_PTM_SAME_POS, prCfaMp4->eVidType, u4Len + 4);
			if ((0 == prVInfo->u1SeqParamNum[prCfaMp4->u4CurAvcEsdIndex])
				 && (0 == prVInfo->u1PicParamNum[prCfaMp4->u4CurAvcEsdIndex])
				 && (0 == prVInfo->u1VPSNum[prCfaMp4->u4CurAvcEsdIndex])) {
				prCfaMp4->u4CurAvcEsdIndex++;
				if (prCfaMp4->u4CurAvcEsdIndex == prVInfo->u1AvcSmpDesNums) {
					prVInfo->fgTxMp4SCDone = TRUE;
					prCfaMp4->u4CurAvcEsdIndex = 0;
				}
			}
			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4] Spt4CfaBuf2VFifo error ret = %d\n"),
					mrRet);
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
				return;
			}
		}
	} else if ((CFA_VID_VC1 == prCfaMp4->eVidType)
		&& (FALSE == prCfaMp4->fgFinished)
		&& (FALSE == prCfaMp4->rCfaMp4VInf.fgCodecSCDone)) {
		prCfaMp4->fgGetDate = FALSE;
		prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_TX_VC1_TO_FIFO;
		CfaMp4TxWVC1Payload2Fifo(pvSptHdl, prCfaMp4);
	} else {
		if ((!prCfaMp4->fgFinished) &&
			 ((CFA_VID_H263_SORENSON == prCfaMp4->eVidType)
			 || (CFA_VID_VP6 == prCfaMp4->eVidType)
			 || (CFA_VID_VP8 == prCfaMp4->eVidType)
			 || (CFA_VID_DIVX3 == prCfaMp4->eVidType)) && (!prCfaMp4->fgSh263GetedHdr)) {
			prCfaMp4->fgSh263GetedHdr = TRUE;
			mrRet =
				Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaMp4->rCurOfst.u8VidCurOfst, 20,
						   (u8 *) &prCfaMp4->ptrPfrMemAddress);
			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4] Spt4CfaPbb2SyncBuf error ret = %d\n"),
					mrRet);
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
			}
			return;
		}
		u8FileOffset = prCfaMp4->rCurOfst.u8VidCurOfst;
#if MP4_SUPPORT_FRAGMENT
		if (prCfaMp4->eCfaMoofType != TYPE_ONLY_MOOF) {
#endif
		prVInfo->u4NeedPrsSampleNums--;
		u8Temp = prVInfo->u8CurPrsVidSampleNo - prVInfo->u8CurVidTableStartSampleNo;
		if (prVInfo->u8CurPrsVidSampleNo == prVInfo->u8CurVidTableEndSampleNo - 1)
			prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_RELOAD_TABLE;
		else
			prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_GET_V_PTS_TO_FIFO;

		if (0 == prVInfo->u4NeedPrsSampleNums) {
			prVInfo->u8CurPrsVidChunkNo++;
			if (CFA_MP4_ANA_RELOAD_TABLE != prCfaMp4->eCurCfaMp4AnaSt)
				prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_NEXT_STATE;
		}
		if (prVInfo->u8CurPrsVidSampleNo == prCfaMp4->rCfaRange.u8VidEndSampleNo) {
			prCfaMp4->u4CurPrsFlg &= ~((u32) CFA_MP4_PRS_BIT_STRM_TYPE_V);
			prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_NEXT_STATE;
		}
		if (0 == prVInfo->u4SampleSize) {
			prCfaMp4->rCurOfst.u8VidCurOfst +=
				prVInfo->pTSampleInfo[u8Temp].u4SampleSize;
		} else
			prCfaMp4->rCurOfst.u8VidCurOfst += prVInfo->u4SampleSize;

		if (prVInfo->u8CurPrsVidSampleNo == prCfaMp4->rCfaRange.u8VidStartSampleNo)
			prCfaMp4->u8Vpts = prCfaMp4->rCfaRange.u8VidPts;
		else
			prCfaMp4->u8Vpts = prCfaMp4->u8CurVPts;

		prCfaMp4->u8CurVPts =
			CfaMp4PrsSttsGetPts(prCfaMp4, Cfa_Mp4_Vid_Track,
					prVInfo->u8CurPrsVidSampleNo
#if SPECIAL_LPCM_SUPPORT
					, 1
#endif
			);
		prVInfo->u8CurPrsVidSampleNo++;

		if (0 == prVInfo->u4SampleSize)
			prCfaMp4->rVidInf.u8Len = prVInfo->pTSampleInfo[u8Temp].u4SampleSize;
		else
			prCfaMp4->rVidInf.u8Len = prVInfo->u4SampleSize;

#if MP4_SUPPORT_FRAGMENT
		} else {
			prCfaMp4->rVidInf.u8Len = prVInfo->u4SampleSize;
		}
#endif
		if (!prCfaMp4->fgFinished)
			Spt4CfaPTSNotify(pvSptHdl, prCfaMp4->u8Vpts);

		prCfaMp4->rVidInf.u8FileOfst = u8FileOffset;
		prVInfo->u8Ofst = u8FileOffset;
		prCfaMp4->rVidInf.eTxMode = CFA_PTM_EXACT_POS;
		prCfaMp4->rVidInf.eVidType = prCfaMp4->eVidType;

		prCfaMp4->rVidInf.u4PrsStrmId = prCfaMp4->u4CurVidInfoId;
		prCfaMp4->eCurPrsSampleType = CFA_MP4_PRS_BIT_STRM_TYPE_V;

			/*MX_ASSERT(prCfaMp4->rVidInf.u8Len);*/
		if ((0 == prCfaMp4->rVidInf.u8Len) && (!prCfaMp4->fgFinished)) {
			DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
			return;
		}

		if (!prCfaMp4->fgFinished &&
			 ((CFA_VID_H265 == prCfaMp4->eVidType)
			  || (CFA_VID_H264 == prCfaMp4->eVidType))
			 && (0xFF != prCfaMp4->u1SyncBufSize)) {
			prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_TX_AVC_TO_FIFO;
			prCfaMp4->fgSyncBuf = FALSE;
			CfaMp4TxAvcPayload2Fifo(pvSptHdl, prCfaMp4);
		} else if (((CFA_VID_WMV7 == prCfaMp4->eVidType)
			|| (CFA_VID_WMV8 == prCfaMp4->eVidType)
			|| (CFA_VID_WMV9 == prCfaMp4->eVidType)
			|| (CFA_VID_RV40 == prCfaMp4->eVidType))
			&& (FALSE == prCfaMp4->fgFinished)) {
			prCfaMp4->eLastCfaMp4AnaSt = prCfaMp4->eCurCfaMp4AnaSt;
			prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_TX_WMV_TO_FIFO;
			prCfaMp4->fgGetDate = FALSE;
			CfaMp4TxwmvPayload2Fifo(pvSptHdl, prCfaMp4);
		} else if (!prCfaMp4->fgFinished) {
			if ((CfaMp4GetRangeEa(prCfaMp4) <
				  (prCfaMp4->rVidInf.u8FileOfst +
				   prCfaMp4->rVidInf.u8Len)) || (prCfaMp4->rVidInf.u8FileOfst <
								  prCfaMp4->u8Ca)) {
				DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
			} else {
				GetMp4VPictType(prCfaMp4);
				prCfaMp4->u8Ca =
					prCfaMp4->rVidInf.u8FileOfst + prCfaMp4->rVidInf.u8Len;
				if (CFA_VID_VC1 == prCfaMp4->eVidType) {
					prCfaMp4->rVidInf.eTxMode = CFA_PTM_SAME_POS;
					prCfaMp4->rVidInf.fgQueryWVC1Mode = FALSE;
					prCfaMp4->rCfaMp4VInf.fgCodecSCDone = FALSE;
				}
				prCfaMp4->rVidInf.fgDummyAU = FALSE;

					/*MLOG_ERROR(TEXT("[CFA MP4][%s]-------video------
					prCfaMp4->u8Vpts:
					0x%llx\n"), __FUNCTION__, prCfaMp4->u8Vpts);*/
					mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &(prCfaMp4->rVidInf));
				if (RET_DMX_OK != mrRet) {
					DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA MP4] Spt4CfaPbb2VFifoAUCtrl error ret = %d\n"),
						mrRet);
					CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
					return;
				}
#if MP4_SUPPORT_FRAGMENT
				if (prCfaMp4->eCfaMoofType == TYPE_ONLY_MOOF) {
					DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA MP4][%s] line %d ,TYPE_ONLY_MOOF!\n"),__func__,__LINE__);
					prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_MOOF_TRUN;

					if (prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleNo >=
							prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleCount ) {
						mrRet = CfaMp4MoofInit(&(prCfaMp4->rCfaMp4VInf.TCfaMoofInfo));
						if (mrRet != RET_DMX_OK) {
							DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
								TEXT("[CFA MP4]CfaMp4MoofInit video fail,call CfaMp4FinishPrs!\n"));
							CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
							return;
						}
						if ((prCfaMp4->u4CurPrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_A) &&
							(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.fgTrunBufValid == TRUE)) {
							prCfaMp4->eCurStreamType = CFA_MP4_AUDIO;
						} else {
							prCfaMp4->eCurStreamType = CFA_MP4_UNKNOWN;
							prCfaMp4->u8CfaCurMoofOffset = prCfaMp4->u8CfaNextMoofOffset;
							prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_NEXT_STATE;//CFA_MP4_ANA_PRS_MOOF_HEADER
						}
					}
				}
#endif
			}
		}
	}
}

/*-----------------------------------------------------------------------------
* Name: vCfaMp4FinishPrs
*
* Description:
*	   finished this prs
*
* Inputs:
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
void CfaMp4FinishPrs(void *pvSptHdl, CfaMp4Inst *prCfaMp4)
{
	if (prCfaMp4->fgFinished) {
		prCfaMp4->u4CurPrsFlg = CFA_MP4_PRS_BIT_STRM_TYPE_NONE;
		return;
	}
	prCfaMp4->u8Ea = CfaMp4GetRangeEa(prCfaMp4);
	MMATE_CHECK_POINTER(prCfaMp4);
	MMATE_CHECK_STRUCT(prCfaMp4->rCfaRange);
	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s]u8Ca:0x%llx, u8Ea:0x%llx.\r\n"),
			   __func__, prCfaMp4->u8Ca, prCfaMp4->u8Ea);
	prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_ST_IDLE;
	if (!(DMX_IS_RW_PLAY(pvSptHdl)))
		Spt4CfaFinishedEx(pvSptHdl, prCfaMp4->u8Ea, TRUE, GAU_E_EOS);


	else
		Spt4CfaFinishedEx(pvSptHdl, prCfaMp4->u8Ea, FALSE, GAU_E_EOS);

}


/*-----------------------------------------------------------------------------
 * Name: vCfaMp4TxWVC1Payload2Fifo
 *
 * Description:

 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
void CfaMp4TxWVC1Payload2Fifo(void *pvSptHdl, CfaMp4Inst *prCfaMp4)
{
	MRESULT mrRet = RET_MSDKC_OK;

	if (FALSE == prCfaMp4->fgGetDate) {
		prCfaMp4->fgGetDate = TRUE;
		prCfaMp4->rVidInf.u8FileOfst = prCfaMp4->rCurOfst.u8VidCurOfst;
		if (CfaMp4GetRangeEa(prCfaMp4) <
			 (prCfaMp4->rVidInf.u8FileOfst + prCfaMp4->rVidInf.u8Len)) {
			prCfaMp4->fgGetDate = FALSE;
			DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
		}

		else {
			mrRet = Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaMp4->rVidInf.u8FileOfst, 4,
				(u8 *) &prCfaMp4->ptrPfrMemAddress);
			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4] Spt4CfaPbb2SyncBuf error ret = %d\n"),
					mrRet);
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
				return;
			}
		}
	}

	else {
		prCfaMp4->fgGetDate = FALSE;
		prCfaMp4->pu1HdrBuf = (u8 *) prCfaMp4->ptrPfrMemAddress;
		prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_GET_V_PTS_TO_FIFO;
		prCfaMp4->rCfaMp4VInf.fgCodecSCDone = TRUE;
		if ((fgCfaMP4Is4cc(prCfaMp4->pu1HdrBuf, 0x0, 0x0, 0x1, 0xD)) ||
			 (fgCfaMP4Is4cc(prCfaMp4->pu1HdrBuf, 0x0, 0x0, 0x1, 0xE)) ||
			 (fgCfaMP4Is4cc(prCfaMp4->pu1HdrBuf, 0x0, 0x0, 0x1, 0xF))) {
			CfaMp4AnaGetVPts2Fifo(pvSptHdl, prCfaMp4);
		}

		else {
			Spt4CfaBuf2VFifo(pvSptHdl, prCfaMp4->rCfaMp4VInf.puCodecSC, 0,
				CFA_PTM_WMV_SEQHDR, prCfaMp4->eVidType, 4);
		}
	}
}


/*-----------------------------------------------------------------------------
 * Name: vCfaMp4TxwmvPayload2Fifo
 *
 * Description:

 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
void CfaMp4TxwmvPayload2Fifo(void *pvSptHdl, CfaMp4Inst *prCfaMp4)
{
	MRESULT mrRet = RET_DMX_OK;

	if (FALSE == prCfaMp4->fgGetDate) {
		DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				 TEXT("[CFA]CfaMp4TxwmvPayload2Fifo: u8VidCurOfst:0x%llx,")
				 TEXT("u8FileOfst:0x%llx, u8Len:0x%llx.\r\n"),
				 prCfaMp4->rCurOfst.u8VidCurOfst, prCfaMp4->rVidInf.u8FileOfst,
				 prCfaMp4->rVidInf.u8Len);
		prCfaMp4->fgGetDate = TRUE;
		if (CfaMp4GetRangeEa(prCfaMp4) <
			(prCfaMp4->rVidInf.u8FileOfst + prCfaMp4->rVidInf.u8Len)) {
			prCfaMp4->fgGetDate = FALSE;
			DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
		} else {
			mrRet = Spt4CfaPbb2SyncBuf(pvSptHdl,
				/*rCfaMp4->rCurOfst.u8VidCurOfst,*/
						prCfaMp4->rVidInf.u8FileOfst, prCfaMp4->rVidInf.u8Len,
						(u8 *) &prCfaMp4->ptrPfrMemAddress);
			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4] Spt4CfaPbb2SyncBuf error ret = %d\n"),
					mrRet);
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
				return;
			}
		}
	} else {
		prCfaMp4->fgGetDate = FALSE;
		prCfaMp4->pu1HdrBuf = (u8 *) prCfaMp4->ptrPfrMemAddress;
		GetMp4VPictType(prCfaMp4);
		prCfaMp4->eCurCfaMp4AnaSt = prCfaMp4->eLastCfaMp4AnaSt;
		if (CFA_VID_RV40 == prCfaMp4->eVidType) {

			prCfaMp4->rVidInf.u2RmCurAuSliceNum =
				prCfaMp4->rSliceInf.u1TotalSliceNum;

			prCfaMp4->rVidInf.eVidType = prCfaMp4->eVidType;
			prCfaMp4->rVidInf.fgUnitStart = TRUE;
			prCfaMp4->rVidInf.u8TotalAULen = prCfaMp4->rVidInf.u8Len;
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT, TEXT("[CFA MP4]current slice num:%d\r\n"),
					 prCfaMp4->rVidInf.u2RmCurAuSliceNum);
		}
		if ((CFA_VID_WMV7 == prCfaMp4->eVidType) ||
			   (CFA_VID_WMV8 == prCfaMp4->eVidType) ||
			   (CFA_VID_WMV9 == prCfaMp4->eVidType)) {
			if (0) {	/*RUE == prCfaMp4Inst->rVidInf.fgUnitStart)*/
				prCfaMp4->rVidInf.fgUnitStart = FALSE;
				DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						 TEXT("[CFA MP4]  vCfaMp4TxwmvPayload2Fifo u4DecSpecSz[%d]")
						 TEXT("sampleLen[%llx]firstAUlength[%d]\r\n"),
						 prCfaMp4->rCfaMp4VInf.u4DecSpecSz[0],
						 prCfaMp4->rVidInf.u8Len,
						 prCfaMp4->rVidInf.u8TotalAULen);
			} else {
				prCfaMp4->rVidInf.fgUnitStart = TRUE;
				prCfaMp4->rVidInf.u8TotalAULen = prCfaMp4->rVidInf.u8Len;
			}
		}
		if (prCfaMp4->fgFinished == FALSE) {
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					 TEXT("[CFA]wwww TMode:0x%llx-0x%x-0x%llx-0x%llx\r\n"),
					 prCfaMp4->rCfaMp4VInf.u8CurPrsVidSampleNo,
					 prCfaMp4->rVidInf.eTxMode, prCfaMp4->rVidInf.u8FileOfst,
					 prCfaMp4->rVidInf.u8Len);
			if ((CfaMp4GetRangeEa(prCfaMp4) <
				  (prCfaMp4->rVidInf.u8FileOfst +
				   prCfaMp4->rVidInf.u8Len)) || (prCfaMp4->rVidInf.u8FileOfst <
								  prCfaMp4->u8Ca)) {
				DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
			} else {
				if (CFA_VID_RV30 == prCfaMp4->eVidType)
					prCfaMp4->u8Ca += prCfaMp4->rVidInf.u8Len;
				else {
					prCfaMp4->u8Ca =
						prCfaMp4->rVidInf.u8FileOfst + prCfaMp4->rVidInf.u8Len;
				}
				if(RET_DMX_OK != Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &(prCfaMp4->rVidInf)))
				{
					DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT, TEXT("[CFA]CfaMp4TxwmvPayload2Fifo Spt4CfaPbb2VFifoAUCtrl fail!\r\n"));
					CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
				}
#if MP4_SUPPORT_FRAGMENT
				if (prCfaMp4->eCfaMoofType == TYPE_ONLY_MOOF) {
					DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA MP4][%s] line %d ,TYPE_ONLY_MOOF!\n"),__func__,__LINE__);
					prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_MOOF_TRUN;

					if (prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleNo >=
							prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleCount ) {
						mrRet = CfaMp4MoofInit(&(prCfaMp4->rCfaMp4VInf.TCfaMoofInfo));
						if (mrRet != RET_DMX_OK) {
							DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
								TEXT("[CFA MP4]CfaMp4MoofInit video fail,call CfaMp4FinishPrs!\n"));
							CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
							return;
						}
						if ((prCfaMp4->u4CurPrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_A) &&
							(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.fgTrunBufValid == TRUE)) {
							prCfaMp4->eCurStreamType = CFA_MP4_AUDIO;
						} else {
							prCfaMp4->eCurStreamType = CFA_MP4_UNKNOWN;
							prCfaMp4->u8CfaCurMoofOffset = prCfaMp4->u8CfaNextMoofOffset;
							prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_NEXT_STATE;//CFA_MP4_ANA_PRS_MOOF_HEADER
						}
					}
				}
#endif
				DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						 TEXT("[CFA]TMode:0x%llx-0x%x-0x%llx-0x%llx fgUnitStart[%d]")
						 TEXT("u8TotalAULen[%lld]\r\n"),
						 prCfaMp4->rCfaMp4VInf.u8CurPrsVidSampleNo,
						 prCfaMp4->rVidInf.eTxMode,
						 prCfaMp4->rVidInf.u8FileOfst, prCfaMp4->rVidInf.u8Len,
						 prCfaMp4->rVidInf.fgUnitStart,
						 prCfaMp4->rVidInf.u8TotalAULen);
				if ((CFA_VID_WMV7 == prCfaMp4->eVidType)
					 || (CFA_VID_WMV8 == prCfaMp4->eVidType)
					 || (CFA_VID_WMV9 == prCfaMp4->eVidType)) {
					prCfaMp4->rVidInf.fgUnitStart = FALSE;
				}
			}
		}
	}
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4TxAvcPayload2Fifo
*
* Description:
*
* Inputs:
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
void CfaMp4TxAvcPayload2Fifo(void *pvSptHdl, CfaMp4Inst *prCfaMp4)
{
	CFA_VIDEO_INFO_T rVidInf = {0};
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prCfaMp4)
		return;

	if (FALSE == prCfaMp4->fgSyncBuf) {
		prCfaMp4->fgSyncBuf = TRUE;
		if (CfaMp4GetRangeEa(prCfaMp4) <
			 (prCfaMp4->rVidInf.u8FileOfst + prCfaMp4->u1SyncBufSize)) {
			prCfaMp4->fgSyncBuf = FALSE;
			DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
		} else {
			mrRet =
				Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaMp4->rVidInf.u8FileOfst,
						   prCfaMp4->u1SyncBufSize,
						   (u8 *) &prCfaMp4->ptrPfrMemAddress);
			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4] Spt4CfaPbb2SyncBuf error ret = %d\n"),
					mrRet);
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
				return;
			}
		}
	} else {
		prCfaMp4->pu1HdrBuf = (u8 *) prCfaMp4->ptrPfrMemAddress;
		if (NULL == prCfaMp4->pu1HdrBuf) {
			return;
		}

		if (4 == prCfaMp4->u1SyncBufSize) {
			LOADB_DWRD(prCfaMp4->pu1HdrBuf, rVidInf.u8Len);
		} else if (3 == prCfaMp4->u1SyncBufSize) {
			rVidInf.u8Len = GET_3BYTE(prCfaMp4->pu1HdrBuf);
		} else if (2 == prCfaMp4->u1SyncBufSize) {
			LOADB_WORD(prCfaMp4->pu1HdrBuf, rVidInf.u8Len);
		} else {
			rVidInf.u8Len = (u8) (*prCfaMp4->pu1HdrBuf);
		}

		if ((1 == rVidInf.u8Len) && (4 == prCfaMp4->u1SyncBufSize)) {
			rVidInf.u8Len = prCfaMp4->rVidInf.u8Len - 4;
		}

		if (rVidInf.u8Len > (prCfaMp4->rVidInf.u8Len - prCfaMp4->u1SyncBufSize)) {
			rVidInf.u8Len = prCfaMp4->rVidInf.u8Len;
			rVidInf.u8FileOfst = prCfaMp4->rVidInf.u8FileOfst;
			prCfaMp4->fgTxAvcHdr = TRUE;//Handle error length, fix  AC8317LINUXHBS-95
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4][%s] line %d, Donnot add 0x000001 before data!\n"),
					__func__, __LINE__);
		} else {
			rVidInf.u8FileOfst = prCfaMp4->rVidInf.u8FileOfst + prCfaMp4->u1SyncBufSize;
		}

		/*Tx avc header "00 00 01"*/
		if (!prCfaMp4->fgTxAvcHdr) {

			mrRet =
				Spt4CfaBuf2VFifo(pvSptHdl, prCfaMp4->pucAvcHdr, 0, CFA_PTM_SAME_POS,
					 prCfaMp4->eVidType, 3);
			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4] Spt4CfaBuf2VFifo error ret = %d\n"),
					mrRet);
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
				return;
			}
			prCfaMp4->fgTxAvcHdr = TRUE;
			return;
		}

		prCfaMp4->fgSyncBuf = FALSE;
		prCfaMp4->rVidInf.u8FileOfst = rVidInf.u8FileOfst + rVidInf.u8Len;
		if (prCfaMp4->rVidInf.u8Len > rVidInf.u8Len + prCfaMp4->u1SyncBufSize)
			prCfaMp4->rVidInf.u8Len -= rVidInf.u8Len + prCfaMp4->u1SyncBufSize;
		else
			prCfaMp4->rVidInf.u8Len = 0;

		rVidInf.eTxMode = CFA_PTM_EXACT_POS;
		rVidInf.eVidType = prCfaMp4->eVidType;
		rVidInf.u4PrsStrmId = prCfaMp4->u4CurVidInfoId;
		if (0 == rVidInf.u8Len) {
			rVidInf.u8Len = prCfaMp4->rVidInf.u8Len;
			prCfaMp4->rVidInf.u8Len = 0;
		}
		if (0 == prCfaMp4->rVidInf.u8Len) {
#if MP4_SUPPORT_FRAGMENT
			if (prCfaMp4->eCfaMoofType == TYPE_ONLY_MOOF) {
				DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4][%s] line %d ,TYPE_ONLY_MOOF!\n"),__func__,__LINE__);
				prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_MOOF_TRUN;

				if (prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleNo >=
						prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleCount ) {
					mrRet = CfaMp4MoofInit(&(prCfaMp4->rCfaMp4VInf.TCfaMoofInfo));
					if (mrRet != RET_DMX_OK) {
						DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
							TEXT("[CFA MP4]CfaMp4MoofInit video fail,call CfaMp4FinishPrs!\n"));
						CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
						return;
					}
					if ((prCfaMp4->u4CurPrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_A) &&
						(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.fgTrunBufValid == TRUE)) {
						prCfaMp4->eCurStreamType = CFA_MP4_AUDIO;
					} else {
						prCfaMp4->eCurStreamType = CFA_MP4_UNKNOWN;
						prCfaMp4->u8CfaCurMoofOffset = prCfaMp4->u8CfaNextMoofOffset;
						prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_NEXT_STATE;//CFA_MP4_ANA_PRS_MOOF_HEADER
					}
				}
			} else {
#endif
			if (CFA_MP4_CUR_TABLE_PRS_TYPE_NONE == prCfaMp4->u4CurTablePrsDoneFlag) {
				DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
			} else {
				if (0 == prCfaMp4->rCfaMp4VInf.u4NeedPrsSampleNums) {
					prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_NEXT_STATE;
				} else
					prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_GET_V_PTS_TO_FIFO;
			}
#if MP4_SUPPORT_FRAGMENT
		}
#endif
		} else
			prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_TX_AVC_TO_FIFO;

		if (!prCfaMp4->fgFinished) {
			if ((CfaMp4GetRangeEa(prCfaMp4) < (rVidInf.u8FileOfst + rVidInf.u8Len))
				|| (rVidInf.u8FileOfst < prCfaMp4->u8Ca)) {
					DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA MP4][%s] line %d, call cfamp4finishprs\n"),__func__,__LINE__);
					CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
			} else {

				prCfaMp4->fgTxAvcHdr = FALSE;
				prCfaMp4->u8Ca = rVidInf.u8FileOfst + rVidInf.u8Len;
				rVidInf.fgDummyAU = FALSE;
				mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rVidInf);
				if (RET_DMX_OK != mrRet) {
					DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA MP4] Spt4CfaPbb2VFifoAUCtrl error ret = %d\n"),
						mrRet);
					CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
					return;
				}
			}
		}
	}
}


/*-----------------------------------------------------------------------------
* Name: vCfaMp4PrsNextState
*
* Description:
*	PRS next state
*
* Inputs:
*
* Outputs:
*
* Returns: None
*
*-----------------------------------------------------------------------------*/
void CfaMp4PrsNextState(void *pvSptHdl, CfaMp4Inst *prCfaMp4)
{
	u32 i = 0;
	u64 u8VidOfst = 0;
	u64 u8TempMinOfst = 0;
	TChunkInfo *pTemp = NULL;
	u64 u8Temp4PrsSubDone = DMX_INVALID_UINT64;
	u64 u8Temp4PrsAudDone = DMX_INVALID_UINT64;
	u64 u8AudOfst[8] = {0};
	u64 u8SubOfst[32] = {0};

#if MP4_SUPPORT_FRAGMENT
	MRESULT mrRet = RET_MSDKC_OK;
	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4][%s] line %d!\n"),__func__,__LINE__);
	if (prCfaMp4->eCfaMoofType == TYPE_ONLY_MOOF)
	{
		mrRet = Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaMp4->u8CfaCurMoofOffset, 8,
							(u8 *) &prCfaMp4->ptrPfrMemAddress);
		if (RET_DMX_OK != mrRet) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4] Spt4CfaPbb2SyncBuf error ret = %d\n"),
				mrRet);
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
			return;
		}
		prCfaMp4->fgGetMoofData = FALSE;
		prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_MOOF_HEADER;
		DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4][%s] line %d,prCfaMp4->u8CfaCurMoofOffset 0x%llx!\n"),
			__func__,__LINE__,prCfaMp4->u8CfaCurMoofOffset);
		//CfaMp4AnaPrsMoofHeader(pvSptHdl, prCfaMp4);
		return;
	}
#endif

	if ((0 != prCfaMp4->rCfaMp4VInf.u8CurPrsVidChunkNo)
		&& (DMX_INVALID_UINT64 != prCfaMp4->rCfaMp4VInf.u8CurPrsVidChunkNo)
		&& (0 != prCfaMp4->rCfaMp4VInf.u8CurPrsVidSampleNo)
		&& (DMX_INVALID_UINT64 != prCfaMp4->rCfaMp4VInf.u8CurPrsVidSampleNo)
		&& (NULL != prCfaMp4->rCfaMp4VInf.pTChunkInfo)
		&& (prCfaMp4->rCfaMp4VInf.u8CurPrsVidChunkNo <=
		  prCfaMp4->rCurTblPos.rTblVidPos.u4TabelUb[STCO]) &&
		  (prCfaMp4->rCfaRange.u4PrsFlag & CFA_MP4_PRS_BIT_STRM_TYPE_V) &&
		  (prCfaMp4->u4CurPrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_V)) {
		pTemp = prCfaMp4->rCfaMp4VInf.pTChunkInfo +
			(prCfaMp4->rCfaMp4VInf.u8CurPrsVidChunkNo -
			 prCfaMp4->rCfaMp4VInf.u8CurVidTableStartChunkNo);
		u8VidOfst = pTemp->u8ChunkOfst;
	} else
		u8VidOfst = DMX_INVALID_UINT64;

	u8TempMinOfst = u8VidOfst;
	CFA_MP4_LIMIT_S_NUM(prCfaMp4->u2SubStrmNums);
	for (i = 0; i < prCfaMp4->u2SubStrmNums; i++) {
		if ((0 != prCfaMp4->rCfaMp4SInf[i].u8CurPrsSubChunkNo)
			&& (DMX_INVALID_UINT64 != prCfaMp4->rCfaMp4SInf[i].u8CurPrsSubChunkNo)
			&& (0 != prCfaMp4->rCfaMp4SInf[i].u8CurPrsSubSampleNo)
			&& (DMX_INVALID_UINT64 != prCfaMp4->rCfaMp4SInf[i].u8CurPrsSubSampleNo)
			&& (NULL != prCfaMp4->rCfaMp4SInf[i].pTChunkInfo)
			&& (prCfaMp4->rCfaMp4SInf[i].u8CurPrsSubChunkNo <=
			prCfaMp4->rCurTblPos.rTblSubPos[i].u4TabelUb[STCO]) &&
			(prCfaMp4->rCfaMp4SInf[i].u8CurPrsSubSampleNo <= prCfaMp4->rCfaRange.u8SubEndSampleNo[i])
			&& (prCfaMp4->rCfaRange.u4PrsFlag & CFA_MP4_PRS_BIT_STRM_TYPE_SP)
			 /*& (prCfaMp4->u4CurPrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_SP)*/
			) {
			pTemp = prCfaMp4->rCfaMp4SInf[i].pTChunkInfo +
				(prCfaMp4->rCfaMp4SInf[i].u8CurPrsSubChunkNo -
				 prCfaMp4->rCfaMp4SInf[i].u8CurSubTableStartChunkNo);
			u8SubOfst[i] = pTemp->u8ChunkOfst;
		} else
			u8SubOfst[i] = DMX_INVALID_UINT64;

		u8Temp4PrsSubDone = MIN(u8Temp4PrsSubDone, u8SubOfst[i]);
		u8TempMinOfst = MIN(u8TempMinOfst, u8SubOfst[i]);
	}
	if (DMX_INVALID_UINT64 == u8Temp4PrsSubDone)
		prCfaMp4->u4CurPrsFlg &= ~((u32) CFA_MP4_PRS_BIT_STRM_TYPE_SP);

	CFA_MP4_LIMIT_A_NUM(prCfaMp4->u2AudStrmNums);
	for (i = 0; i < prCfaMp4->u2AudStrmNums; i++) {
		if ((0 != prCfaMp4->rCfaMp4AInf[i].u8CurPrsAudChunkNo)
			&& (DMX_INVALID_UINT64 != prCfaMp4->rCfaMp4AInf[i].u8CurPrsAudChunkNo)
			&& (0 != prCfaMp4->rCfaMp4AInf[i].u8CurPrsAudSampleNo)
			&& (DMX_INVALID_UINT64 != prCfaMp4->rCfaMp4AInf[i].u8CurPrsAudSampleNo)
			&& (NULL != prCfaMp4->rCfaMp4AInf[i].pTChunkInfo)
			&& (prCfaMp4->rCfaMp4AInf[i].u8CurPrsAudChunkNo <=
			prCfaMp4->rCurTblPos.rTblAudPos[i].u4TabelUb[STCO]) &&
			(prCfaMp4->rCfaMp4AInf[i].u8CurPrsAudSampleNo <= prCfaMp4->rCfaRange.u8AudEndSampleNo[i])
			&& (prCfaMp4->rCfaRange.u4PrsFlag & CFA_MP4_PRS_BIT_STRM_TYPE_A)
			/*& (prCfaMp4->u4CurPrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_A)*/
			) {
			pTemp = prCfaMp4->rCfaMp4AInf[i].pTChunkInfo +
				(prCfaMp4->rCfaMp4AInf[i].u8CurPrsAudChunkNo -
				 prCfaMp4->rCfaMp4AInf[i].u8CurAudTableStartChunkNo);
			u8AudOfst[i] = pTemp->u8ChunkOfst;
		} else
			u8AudOfst[i] = DMX_INVALID_UINT64;

		u8Temp4PrsAudDone = MIN(u8Temp4PrsAudDone, u8AudOfst[i]);
		u8TempMinOfst = MIN(u8TempMinOfst, u8AudOfst[i]);
	}
	if (DMX_INVALID_UINT64 == u8Temp4PrsAudDone)
		prCfaMp4->u4CurPrsFlg &= ~((u32) CFA_MP4_PRS_BIT_STRM_TYPE_A);

	if (DMX_INVALID_UINT64 == u8TempMinOfst) {
#if MP4_SUPPORT_FRAGMENT
		if (prCfaMp4->eCfaMoofType == TYPE_MOOV_AND_MOOF) {
			DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] line %d,Moov data parse over,type transfore TYPE_ONLY_MOOF!\n"),__func__,__LINE__);
			prCfaMp4->eCfaMoofType = TYPE_ONLY_MOOF;
			mrRet = Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaMp4->u8CfaCurMoofOffset, 8,
						   (u8 *) &prCfaMp4->ptrPfrMemAddress);
			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4] Spt4CfaPbb2SyncBuf error ret = %d\n"),
					mrRet);
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
				return;
			}
			prCfaMp4->fgGetMoofData = FALSE;
			prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_MOOF_HEADER;
			if (prCfaMp4->u4PrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_V) {
				prCfaMp4->u4CurPrsFlg |= CFA_MP4_PRS_BIT_STRM_TYPE_V;
			}
			if (prCfaMp4->u4PrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_A) {
				prCfaMp4->u4CurPrsFlg |= CFA_MP4_PRS_BIT_STRM_TYPE_A;
			}
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] line %d,prCfaMp4->u8CfaCurMoofOffset 0x%llx!\n"),
				__func__,__LINE__,prCfaMp4->u8CfaCurMoofOffset);
			return;
		}
#endif
		DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
		CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
		return;
	}
	if (u8TempMinOfst == u8VidOfst) {
		prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_V_RANGE;
		CfaMp4AnaPrsVRange(pvSptHdl, prCfaMp4);
	} else {
		CFA_MP4_LIMIT_A_NUM(prCfaMp4->u2AudStrmNums);
		CFA_MP4_LIMIT_S_NUM(prCfaMp4->u2SubStrmNums);
		for (i = 0; i < prCfaMp4->u2AudStrmNums; i++) {
			if (u8TempMinOfst == u8AudOfst[i]) {
				prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_A_RANGE;
				prCfaMp4->u4CurAudInfoId = i;
			}
		}
		for (i = 0; i < prCfaMp4->u2SubStrmNums; i++) {
			if (u8TempMinOfst == u8SubOfst[i]) {
				prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_S_RANGE;
				prCfaMp4->u4CurSubInfoId = i;
			}
		}
		if (CFA_MP4_ANA_PRS_A_RANGE == prCfaMp4->eCurCfaMp4AnaSt)
			CfaMp4AnaPrsARange(pvSptHdl, prCfaMp4);
		else
			CfaMp4AnaPrsSRange(pvSptHdl, prCfaMp4);

	}
}

#if MP4_SUPPORT_FRAGMENT
MRESULT  CfaMp4MoofInit(MP4_MOOF_INFO *TCfaMoofInfo)
{
	if(NULL == TCfaMoofInfo) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	dmx_memset(&(TCfaMoofInfo->mTfhdInfo), 0, sizeof(CfaTfhdInfo));
	//dmx_memset(TCfaMoofInfo->pvTrunBuf, 0, sizeof(TCfaMoofInfo->u4TrunBufMaxSize));

	TCfaMoofInfo->fgTrunBufValid = FALSE;
	TCfaMoofInfo->u4TrunBufSize = 0;
	TCfaMoofInfo->u4TrunBufOffset = 0;
	TCfaMoofInfo->u4TrunFlags = 0;
	TCfaMoofInfo->u4TrunSampleCount = 0;
	TCfaMoofInfo->u4TrunSampleNo = 0;
	TCfaMoofInfo->u4TrunSampleDuration = 0;
	TCfaMoofInfo->u4TrunSampleSize = 0;
	TCfaMoofInfo->u4TrunSampleFlags = 0;
	TCfaMoofInfo->u4TrunSampleCtsOffset = 0;
	TCfaMoofInfo->u8TrunDataOffset = 0;

	MM_RETURN(RET_DMX_OK);
}
MRESULT  CfaMp4MoofTrunMemAlloc(MP4_MOOF_INFO *TCfaMoofInfo)
{
	u32 u4TmpMaxSize = 0;

	if(NULL == TCfaMoofInfo) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	u4TmpMaxSize = (TCfaMoofInfo->u4TrunBufSize / 1024 + 1)* 1024;//1k
	if (FALSE == TCfaMoofInfo->fgTrunBufValid) {
		if ((NULL != TCfaMoofInfo->pvTrunBuf) &&
			(TCfaMoofInfo->u4TrunBufMaxSize < u4TmpMaxSize)) {
			DMX_FreeMemory(TCfaMoofInfo->pvTrunBuf);
			TCfaMoofInfo->pvTrunBuf = NULL;
		}
		if (NULL == TCfaMoofInfo->pvTrunBuf) {
			TCfaMoofInfo->u4TrunBufMaxSize = u4TmpMaxSize;
			DMX_NewMemory(TCfaMoofInfo->u4TrunBufMaxSize, TCfaMoofInfo->pvTrunBuf);
			if (NULL == TCfaMoofInfo->pvTrunBuf)
			{
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4] DMX_NewMemory fail!\n"));
				MM_RETURN(RET_DMX_NO_MEM);
			}
			dmx_memset(TCfaMoofInfo->pvTrunBuf, 0, sizeof(TCfaMoofInfo->u4TrunBufMaxSize));
		}
		TCfaMoofInfo->fgTrunBufValid = TRUE;
		MM_RETURN(RET_DMX_OK);
	}else {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
}
void CfaMp4AnaPrsMoofHeader(void *pvSptHdl, CfaMp4Inst *prCfaMp4)
{
	u32 u4MoofHeaderLen = 0;
	u32 u4SkipLen = 0;
	u32 u4MoofTrafLen = 0;
	u32 u4MoofTfhdLen = 0;
	u32 u4MoofTfdtLen = 0;
	u32 u4TrackId = 0;
	u32 u4MoofTrunLen = 0;

	CfaCurStreamType eTrackType = CFA_MP4_UNKNOWN;
	MRESULT mrRet = RET_MSDKC_OK;

	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4][%s] line %d enter!\n"),__func__,__LINE__);

	if (FALSE == prCfaMp4->fgGetMoofData) {
		prCfaMp4->pu1HdrBuf = (u8 *) prCfaMp4->ptrPfrMemAddress;
		if (NULL == prCfaMp4->pu1HdrBuf)
			return;

		if (fgCfaMP4Is4cc(prCfaMp4->pu1HdrBuf + 4, 'm', 'o', 'o', 'f'))
		{
			LOADB_DWRD(prCfaMp4->pu1HdrBuf, prCfaMp4->u4CfaCurMoofSize);
		} else if (fgCfaMP4Is4cc(prCfaMp4->pu1HdrBuf + 4, 'm', 'f', 'r', 'a')) {
			DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4]End moof ,call cfamp4finishprs\n"));
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
			return;
		} else {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4]ERROR moof call cfamp4finishprs\n"));
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
			return;
		}
		if (CfaMp4GetRangeEa(prCfaMp4)<
			(prCfaMp4->u4CfaCurMoofSize + prCfaMp4->u8CfaCurMoofOffset)) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4]ERROR moof size, call cfamp4finishprs\n"));
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
			return;
		}
		mrRet = Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaMp4->u8CfaCurMoofOffset + 8, prCfaMp4->u4CfaCurMoofSize,//m_u4MoofSize + 8 = moof+mdat
							(u8 *) &prCfaMp4->ptrPfrMemAddress);
		if (RET_DMX_OK != mrRet) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4] Spt4CfaPbb2SyncBuf error ret = %d\n"),
				mrRet);
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
			return;
		}
		prCfaMp4->fgGetMoofData = TRUE;
		prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_MOOF_HEADER;

		DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4][%s] line %d CFA_MP4_ANA_PRS_MOOF_HEADER enter!\n"),__func__,__LINE__);
		return;
	} else {
		DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4][%s] line %d enter!\n"),__func__,__LINE__);

		prCfaMp4->pu1HdrBuf = (u8 *) prCfaMp4->ptrPfrMemAddress;
		if (NULL == prCfaMp4->pu1HdrBuf)
			return;

		if (fgCfaMP4Is4cc(prCfaMp4->pu1HdrBuf + prCfaMp4->u4CfaCurMoofSize - 4, 'm', 'd', 'a', 't')) {
			LOADB_DWRD(prCfaMp4->pu1HdrBuf + prCfaMp4->u4CfaCurMoofSize - 8, prCfaMp4->u4CfaCurMoofMdatSize);
			prCfaMp4->u8CfaNextMoofOffset = prCfaMp4->u8CfaCurMoofOffset +
				prCfaMp4->u4CfaCurMoofSize + prCfaMp4->u4CfaCurMoofMdatSize;//u8MoofDataOffset = u8MoofOffset + u4MoofSize;
			if (CfaMp4GetRangeEa(prCfaMp4) < prCfaMp4->u8CfaNextMoofOffset) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4]ERROR moof size, call cfamp4finishprs\n"));
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
				return;
			}
			DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] line %d ,prCfaMp4->u8CfaNextMoofOffset 0x%llx!\n"),__func__,__LINE__,prCfaMp4->u8CfaNextMoofOffset);
		} else {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4]Moof box can't find mdat, call cfamp4finishprs\n"));
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
			return;
		}

		if (fgCfaMP4Is4cc(prCfaMp4->pu1HdrBuf + u4SkipLen + 4, 'm', 'f', 'h', 'd')) {
			LOADB_DWRD(prCfaMp4->pu1HdrBuf + u4SkipLen, u4MoofHeaderLen);
		}
		u4SkipLen += u4MoofHeaderLen;
		if (prCfaMp4->u4CurPrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_V) {
			mrRet = CfaMp4MoofInit(&(prCfaMp4->rCfaMp4VInf.TCfaMoofInfo));
			if (mrRet != RET_DMX_OK) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4]CfaMp4MoofInit video fail,call CfaMp4FinishPrs!\n"));
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
				return;
			}
		}
		if (prCfaMp4->u4CurPrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_A) {
			mrRet = CfaMp4MoofInit(&(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo));
			if (mrRet != RET_DMX_OK) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4]CfaMp4MoofInit audio fail,call CfaMp4FinishPrs!\n"));
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
				return;
			}
		}

		while(u4SkipLen < prCfaMp4->u4CfaCurMoofSize - 8 - 4) {
			if (fgCfaMP4Is4cc(prCfaMp4->pu1HdrBuf + u4SkipLen + 4, 't', 'r', 'a', 'f')) {
				u4MoofTrafLen = 0;
				eTrackType = CFA_MP4_UNKNOWN;
				LOADB_DWRD(prCfaMp4->pu1HdrBuf + u4SkipLen, u4MoofTrafLen);
				u4SkipLen += 8;
				if (u4SkipLen >= prCfaMp4->u4CfaCurMoofSize - 8 - 4) {
					DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA MP4]Current Moof parse over, call CfaMp4FinishPrs!\n"));
					CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
					return;
				}
				if (fgCfaMP4Is4cc(prCfaMp4->pu1HdrBuf + u4SkipLen + 4, 't', 'f', 'h', 'd')) {
					u4MoofTfhdLen = 0;
					u4TrackId = 0;
					LOADB_DWRD(prCfaMp4->pu1HdrBuf + u4SkipLen, u4MoofTfhdLen);
					u4SkipLen += 8;
					if (u4SkipLen >= prCfaMp4->u4CfaCurMoofSize - 8 - 4) {
						DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
							TEXT("[CFA MP4]Current Moof parse over, call CfaMp4FinishPrs!\n"));
						CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
						return;
					}
					LOADB_DWRD(prCfaMp4->pu1HdrBuf + u4SkipLen + 4, u4TrackId);

					if((prCfaMp4->u4CurPrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_V) &&
						(u4TrackId == prCfaMp4->rCfaMp4VInf.u4TrackID)) {
						eTrackType = CFA_MP4_VIDEO;
						if (prCfaMp4->eCurStreamType == CFA_MP4_UNKNOWN) {
							prCfaMp4->eCurStreamType =  CFA_MP4_VIDEO;
						}
						prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.mTfhdInfo.u4TrackID = u4TrackId;
						mrRet = CfaMp4AnaPrsMoofTfhd(prCfaMp4->pu1HdrBuf + u4SkipLen,
							&prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.mTfhdInfo, u4MoofTfhdLen - 8);
						if (mrRet != RET_DMX_OK) {
							DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
								TEXT("[CFA MP4]CfaMp4AnaPrsMoofTfhd fail,call CfaMp4FinishPrs!\n"));
							CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
							return;
						}
						if (0 == prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.mTfhdInfo.u8BaseDataOffset) {
							prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.mTfhdInfo.u8BaseDataOffset = prCfaMp4->u8CfaCurMoofOffset;
						}
					} else if ((prCfaMp4->u4CurPrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_A) &&
							(u4TrackId == prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4TrackID)) {
						eTrackType = CFA_MP4_AUDIO;
						if (prCfaMp4->eCurStreamType == CFA_MP4_UNKNOWN) {
							prCfaMp4->eCurStreamType =  CFA_MP4_AUDIO;
						}
						prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.mTfhdInfo.u4TrackID = u4TrackId;
						mrRet = CfaMp4AnaPrsMoofTfhd(prCfaMp4->pu1HdrBuf + u4SkipLen, &prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.mTfhdInfo, u4MoofTfhdLen - 8);
						if (mrRet != RET_DMX_OK) {
							DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
								TEXT("[CFA MP4]CfaMp4AnaPrsMoofTfhd fail,call CfaMp4FinishPrs!\n"));
							CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
							return;
						}

						if (0 == prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.mTfhdInfo.u8BaseDataOffset) {
							prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.mTfhdInfo.u8BaseDataOffset = prCfaMp4->u8CfaCurMoofOffset;
						}
					}
					u4SkipLen += (u4MoofTfhdLen - 8);
				}
				if (u4SkipLen >= prCfaMp4->u4CfaCurMoofSize - 8 - 4) {
					DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA MP4]Current Moof parse over, call CfaMp4FinishPrs!\n"));
					CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
					return;

				}
				if (fgCfaMP4Is4cc(prCfaMp4->pu1HdrBuf + u4SkipLen + 4, 't', 'f', 'd', 't')) {
					u4MoofTfdtLen = 0;
					LOADB_DWRD(prCfaMp4->pu1HdrBuf + u4SkipLen, u4MoofTfdtLen);
					u4SkipLen += u4MoofTfdtLen;
				}
				if (u4SkipLen >= prCfaMp4->u4CfaCurMoofSize - 8 - 4) {
					DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA MP4]Current Moof parse over, call CfaMp4FinishPrs!\n"));
					CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
					return;
				}
				if (fgCfaMP4Is4cc(prCfaMp4->pu1HdrBuf + u4SkipLen + 4, 't', 'r', 'u', 'n')) {
					u4MoofTrunLen = 0;
					LOADB_DWRD(prCfaMp4->pu1HdrBuf + u4SkipLen, u4MoofTrunLen);
					DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
							TEXT("[CFA MP4][%s] line %d ,u4SkipLen %d ,u4MoofTrunLen 0x%x!\n"),
							__func__, __LINE__, u4SkipLen, u4MoofTrunLen);
					switch (eTrackType) {
					case CFA_MP4_VIDEO:
						prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunBufSize = u4MoofTrunLen - 8;
						mrRet = CfaMp4MoofTrunMemAlloc(&(prCfaMp4->rCfaMp4VInf.TCfaMoofInfo));
						if (mrRet != RET_DMX_OK) {
							DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
								TEXT("[CFA MP4]CfaMp4MoofTrunMemAlloc video fail,call CfaMp4FinishPrs!\n"));
							CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
							return;
						}
						mm_memcpy(prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.pvTrunBuf,
									prCfaMp4->pu1HdrBuf + u4SkipLen + 8,
									prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunBufSize);
						break;
					case CFA_MP4_AUDIO:
						prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunBufSize = u4MoofTrunLen - 8;
						mrRet = CfaMp4MoofTrunMemAlloc(&(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo));
						if (mrRet != RET_DMX_OK) {
							DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
								TEXT("[CFA MP4]CfaMp4MoofTrunMemAlloc audio fail,call CfaMp4FinishPrs!\n"));
							CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
							return;
						}
						mm_memcpy(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.pvTrunBuf,
									prCfaMp4->pu1HdrBuf + u4SkipLen + 8,
									prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunBufSize);
						break;
					default:
						break;
					}
					u4SkipLen += u4MoofTrunLen;
				}
			} else {
				u4SkipLen += 4;
			}
		}
		prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_MOOF_TRUN;
		mrRet = CfaMp4AnaPrsMoofTrun(pvSptHdl, prCfaMp4);
		if (mrRet != RET_DMX_OK) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4]CfaMp4AnaPrsMoofTrun fail,call CfaMp4FinishPrs!\n"));
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
			return;
		}
	}
}

MRESULT  CfaMp4AnaPrsMoofTfhd(void *pvBuf, CfaTfhdInfo *mTfhdInfo, u32 u4BufLen)
{
	u32 u4SkipLen = 0;
	u32 u4ParsedTrackId = 0;

	if (NULL == pvBuf || u4BufLen < 8) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4][%s] line %d ,u4BufLen 0x%x!\n"),__func__,__LINE__,u4BufLen);

	LOADB_DWRD(pvBuf, mTfhdInfo->u4Flags);// actually version + flags
	if (mTfhdInfo->u4Flags & 0xff000000) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	u4SkipLen += 4;
	LOADB_DWRD(pvBuf + u4SkipLen, u4ParsedTrackId);
	if (u4ParsedTrackId != mTfhdInfo->u4TrackID) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4]This is not the right track ,need skip it!\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4]fragment header: %08x %08x!\n"),mTfhdInfo->u4Flags,mTfhdInfo->u4TrackID);

	u4SkipLen += 4;

	if (mTfhdInfo->u4Flags & kBaseDataOffsetPresent) {
		if (u4BufLen - u4SkipLen < 8) {
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
		LOADB_DOBL(pvBuf + u4SkipLen, mTfhdInfo->u8BaseDataOffset);
		if (0 == mTfhdInfo->u8BaseDataOffset) {
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
		u4SkipLen += 8;
	} else {
		mTfhdInfo->u8BaseDataOffset = 0;
	}

	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4]mTfhdInfo->u8BaseDataOffset 0x%llx!\n"),mTfhdInfo->u8BaseDataOffset);

	if (mTfhdInfo->u4Flags & kSampleDescriptionIndexPresent) {
		if (u4BufLen - u4SkipLen < 4) {
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
		LOADB_DWRD(pvBuf + u4SkipLen, mTfhdInfo->u4SampleDescriptionIndex);
		if (0 == mTfhdInfo->u4SampleDescriptionIndex) {
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
		u4SkipLen += 4;
	}

	if (mTfhdInfo->u4Flags & kDefaultSampleDurationPresent) {
		if (u4BufLen - u4SkipLen < 4) {
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
		LOADB_DWRD(pvBuf + u4SkipLen, mTfhdInfo->u4DefaultSampleDuration);
		if (0 == mTfhdInfo->u4DefaultSampleDuration) {
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
		u4SkipLen += 4;
	}

	if (mTfhdInfo->u4Flags & kDefaultSampleSizePresent) {
		if (u4BufLen - u4SkipLen < 4) {
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
		LOADB_DWRD(pvBuf + u4SkipLen, mTfhdInfo->u4DefaultSampleSize);
		if (0 == mTfhdInfo->u4DefaultSampleSize) {
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
		u4SkipLen += 4;
	}

	if (mTfhdInfo->u4Flags & kDefaultSampleFlagsPresent) {
		if (u4BufLen - u4SkipLen < 4) {
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
		LOADB_DWRD(pvBuf + u4SkipLen, mTfhdInfo->u4DefaultSampleFlags);
		if (0 == mTfhdInfo->u4DefaultSampleFlags) {
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
		u4SkipLen += 4;
	}

	mTfhdInfo->u8DataOffset = 0;
	MM_RETURN(RET_DMX_OK);
}
MRESULT  CfaMp4AnaPrsMoofTrunParse(MP4_MOOF_INFO *TCfaMoofInfo)
{
	u32 u4dataOffsetDelta = 0;
	u32 u4firstSampleFlags = 0;
	u32 u4BytesPerSample = 0;

	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4][%s] line %d enter!\n"),__func__,__LINE__);

	if (TCfaMoofInfo->u4TrunBufSize < 8) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4][%s] line %d ,TCfaMoofInfo->u4TrunBufSize %d!\n"),__func__,__LINE__,TCfaMoofInfo->u4TrunBufSize);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	LOADB_DWRD(TCfaMoofInfo->pvTrunBuf,TCfaMoofInfo->u4TrunFlags);
	if (0 == TCfaMoofInfo->u4TrunFlags) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	TCfaMoofInfo->u4TrunBufOffset += 4;

	if (TCfaMoofInfo->u4TrunFlags & 0xff000000) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	if ((TCfaMoofInfo->u4TrunFlags & kFirstSampleFlagsPresent) &&
		(TCfaMoofInfo->u4TrunFlags & kSampleFlagsPresent)) {
		// These two shall not be used together.
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4]kFirstSampleFlagsPresent & kSampleFlagsPresent shall not be used together\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	LOADB_DWRD(TCfaMoofInfo->pvTrunBuf + TCfaMoofInfo->u4TrunBufOffset, TCfaMoofInfo->u4TrunSampleCount);
	if (0 == TCfaMoofInfo->u4TrunSampleCount) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4][CfaMp4AnaPrsMoofTrunParse]:u4TrunBufOffset %d flags %x count %x!\r\n"),
			TCfaMoofInfo->u4TrunBufOffset, TCfaMoofInfo->u4TrunFlags, TCfaMoofInfo->u4TrunSampleCount);
	TCfaMoofInfo->u4TrunBufOffset += 4;

	TCfaMoofInfo->u8TrunDataOffset = TCfaMoofInfo->mTfhdInfo.u8DataOffset;

	if (TCfaMoofInfo->u4TrunFlags & kDataOffsetPresent) {
		if (TCfaMoofInfo->u4TrunBufSize - TCfaMoofInfo->u4TrunBufOffset < 4) {
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
		LOADB_DWRD(TCfaMoofInfo->pvTrunBuf + TCfaMoofInfo->u4TrunBufOffset, u4dataOffsetDelta);
		TCfaMoofInfo->u8TrunDataOffset = TCfaMoofInfo->mTfhdInfo.u8BaseDataOffset + u4dataOffsetDelta;
		TCfaMoofInfo->u4TrunBufOffset += 4;
	}
	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4][CfaMp4AnaPrsMoofTrunParse]:u8TrunDataOffset 0x%llx u8DataOffset 0x%llx, u8BaseDataOffset 0x%llx!\r\n"),
			TCfaMoofInfo->u8TrunDataOffset, TCfaMoofInfo->mTfhdInfo.u8DataOffset, TCfaMoofInfo->mTfhdInfo.u8BaseDataOffset );

	if (TCfaMoofInfo->u4TrunFlags & kFirstSampleFlagsPresent) {
		if (TCfaMoofInfo->u4TrunBufSize - TCfaMoofInfo->u4TrunBufOffset < 4) {
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
		LOADB_DWRD(TCfaMoofInfo->pvTrunBuf + TCfaMoofInfo->u4TrunBufOffset, u4firstSampleFlags);
		TCfaMoofInfo->u4TrunBufOffset += 4;
	}

	if (TCfaMoofInfo->u4TrunFlags & kSampleDurationPresent) {
		u4BytesPerSample += 4;
	} else if (TCfaMoofInfo->mTfhdInfo.u4Flags & kDefaultSampleDurationPresent) {
		TCfaMoofInfo->u4TrunSampleDuration = TCfaMoofInfo->mTfhdInfo.u4DefaultSampleDuration;
	} else {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4]CfaMp4AnaPrsMoofTrunParse: need parse trex box!\n"));
	}

	if (TCfaMoofInfo->u4TrunFlags & kSampleSizePresent) {
		u4BytesPerSample += 4;
	} else if (TCfaMoofInfo->mTfhdInfo.u4Flags & kDefaultSampleSizePresent) {
		TCfaMoofInfo->u4TrunSampleSize = TCfaMoofInfo->mTfhdInfo.u4DefaultSampleSize;
	} else {
		TCfaMoofInfo->u4TrunSampleSize = TCfaMoofInfo->mTfhdInfo.u4DefaultSampleSize;
	}

	if (TCfaMoofInfo->u4TrunFlags & kSampleFlagsPresent) {
		u4BytesPerSample += 4;
	} else if (TCfaMoofInfo->mTfhdInfo.u4Flags & kDefaultSampleFlagsPresent) {
		TCfaMoofInfo->u4TrunSampleFlags = TCfaMoofInfo->mTfhdInfo.u4DefaultSampleFlags;
	} else {
		TCfaMoofInfo->u4TrunSampleFlags = TCfaMoofInfo->mTfhdInfo.u4DefaultSampleFlags;
	}
	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4][%s] line %d ,u4TrunSampleSize %x!\n"),
		__func__,__LINE__,TCfaMoofInfo->u4TrunSampleSize);

	if (TCfaMoofInfo->u4TrunFlags & kSampleCompositionTimeOffsetPresent) {
		u4BytesPerSample += 4;
	} else {
		TCfaMoofInfo->u4TrunSampleCtsOffset = 0;
	}

	if (TCfaMoofInfo->u4TrunBufSize - TCfaMoofInfo->u4TrunBufOffset <
		(TCfaMoofInfo->u4TrunSampleCount * u4BytesPerSample)) {
			MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	MM_RETURN(RET_DMX_OK);

}

MRESULT  CfaMp4AnaPrsMoofTrun(void *pvSptHdl, CfaMp4Inst *prCfaMp4)
{
	MRESULT mrRet = RET_MSDKC_OK;

	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4][%s] line %d ,prCfaMp4->eCurStreamType %d!\n"),__func__,__LINE__,prCfaMp4->eCurStreamType);

	if((prCfaMp4->u4CurPrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_V) &&
		(prCfaMp4->eCurStreamType ==  CFA_MP4_VIDEO)) {
		if (prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleNo == 0) {
			mrRet = CfaMp4AnaPrsMoofTrunParse(&prCfaMp4->rCfaMp4VInf.TCfaMoofInfo);
			if (mrRet != RET_DMX_OK) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4]CfaMp4AnaPrsMoofTrunParse video fail!\n"));
				MM_RETURN(mrRet);
			}
		}

		if (prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleNo <
			prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleCount) {
			if (prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunFlags & kSampleDurationPresent) {
				prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleDuration = 0;
				LOADB_DWRD(prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.pvTrunBuf + prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunBufOffset,
					prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleDuration);
				prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunBufOffset += 4;
			}

			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] line %d ,u4TrunSampleDuration %d!\n"),
				__func__,__LINE__,prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleDuration);

			if (prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunFlags & kSampleSizePresent) {
				prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleSize = 0;
				LOADB_DWRD(prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.pvTrunBuf + prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunBufOffset,
					prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleSize);
				prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunBufOffset += 4;
			}
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] line %d ,u4TrunSampleSize %d!\n"),
				__func__,__LINE__,prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleSize);

			if (prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunFlags & kSampleFlagsPresent) {
				prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleFlags = 0;
				LOADB_DWRD(prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.pvTrunBuf + prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunBufOffset,
					prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleFlags);
				prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunBufOffset += 4;
			}

			if (prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunFlags & kSampleCompositionTimeOffsetPresent) {
				prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleCtsOffset = 0;
				LOADB_DWRD(prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.pvTrunBuf + prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunBufOffset,
					prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleCtsOffset);
				prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunBufOffset += 4;
			}

			prCfaMp4->rCurOfst.u8VidCurOfst = prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u8TrunDataOffset;
			prCfaMp4->rCfaMp4VInf.u4SampleSize = prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleSize;
			prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u8TrunDataOffset += prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleSize;
			prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleNo ++;
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] line %d, u8Len: 0x%llx, u8VidCurOfst 0x%llx, u8Vpts 0x%llx\n"),
				__func__,__LINE__,prCfaMp4->rVidInf.u8Len, prCfaMp4->rCurOfst.u8VidCurOfst, prCfaMp4->u8Vpts);

			prCfaMp4->u8CurVDuation += prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.u4TrunSampleDuration;
			prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_GET_V_PTS_TO_FIFO;
			CfaMp4AnaGetVPts2Fifo(pvSptHdl, prCfaMp4);
			if (0 != prCfaMp4->rCfaMp4VInf.u4TimeScale) {
				prCfaMp4->u8Vpts = (CFA_STC_CLK * prCfaMp4->u8CurVDuation) / prCfaMp4->rCfaMp4VInf.u4TimeScale;
				DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[MP4 CFA][%s] Cfa_Mp4_Vid_Track u4TimeScale = %d,"),__func__,prCfaMp4->rCfaMp4VInf.u4TimeScale);
			} else {
				DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[MP4 CFA][%s] Cfa_Mp4_Vid_Track u4TimeScale = 0,")
					TEXT("so Set PTS = 0! \r\n"), __func__);
				prCfaMp4->u8Vpts = 0;
			}
		} else {
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	} else if ((prCfaMp4->u4CurPrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_A) &&
			(prCfaMp4->eCurStreamType == CFA_MP4_AUDIO)) {
		if (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunSampleNo == 0) {
			mrRet = CfaMp4AnaPrsMoofTrunParse(&prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo);
			if (mrRet != RET_DMX_OK) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4]CfaMp4AnaPrsMoofTrunParse audio fail!\n"));
				MM_RETURN(mrRet);
			}
		}

		if (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunSampleNo <
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunSampleCount) {
			if (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunFlags & kSampleDurationPresent) {
				prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunSampleDuration = 0;
				LOADB_DWRD(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.pvTrunBuf +
					prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunBufOffset,
					prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunSampleDuration);
				prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunBufOffset += 4;
			}

			if (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunFlags & kSampleSizePresent) {
				prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunSampleSize = 0;
				LOADB_DWRD(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.pvTrunBuf +
					prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunBufOffset,
					prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunSampleSize);
				prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunBufOffset += 4;
			}

			if (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunFlags & kSampleFlagsPresent) {
				prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunSampleFlags = 0;
				LOADB_DWRD(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.pvTrunBuf +
					prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunBufOffset,
					prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunSampleFlags);
				prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunBufOffset += 4;
			}

			if (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunFlags & kSampleCompositionTimeOffsetPresent) {
				prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunSampleCtsOffset = 0;
				LOADB_DWRD(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.pvTrunBuf +
					prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunBufOffset,
					prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunSampleCtsOffset);
				prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunBufOffset += 4;
			}

			prCfaMp4->rCurOfst.u8AudCurOfst = prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u8TrunDataOffset;
			prCfaMp4->rAudInf.u8Len = prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunSampleSize;
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u8TrunDataOffset += prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunSampleSize;
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunSampleNo ++;
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] line %d, u8Len: 0x%llx, u8AudCurOfst 0x%llx, u8Apts 0x%llx\n"),
				__func__,__LINE__,prCfaMp4->rAudInf.u8Len, prCfaMp4->rCurOfst.u8AudCurOfst, prCfaMp4->u8Apts[prCfaMp4->u4CurAudInfoId]);

			prCfaMp4->u8CurADuation[prCfaMp4->u4CurAudInfoId] +=
				prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].TCfaMoofInfo.u4TrunSampleDuration;
			prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_GET_A_PTS_TO_FIFO;
			CfaMp4AnaGetAPts2Fifo(pvSptHdl, prCfaMp4);
			if (0 != prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4TimeScale) {
				prCfaMp4->u8Apts[prCfaMp4->u4CurAudInfoId] =
					(CFA_STC_CLK * prCfaMp4->u8CurADuation[prCfaMp4->u4CurAudInfoId]) /
					prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4TimeScale;
				DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[MP4 CFA][%s] Cfa_Mp4_Aud_Track u4TimeScale = %d,"),
					__func__,prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4TimeScale);
			} else {
				DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[MP4 CFA][%s] Cfa_Mp4_Aud_Track u4TimeScale = 0,")
					TEXT("so Set PTS = 0! \r\n"), __func__);
				prCfaMp4->u8Apts[prCfaMp4->u4CurAudInfoId] = 0;
			}

		} else {
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}
	MM_RETURN(RET_DMX_OK);
}
#endif
