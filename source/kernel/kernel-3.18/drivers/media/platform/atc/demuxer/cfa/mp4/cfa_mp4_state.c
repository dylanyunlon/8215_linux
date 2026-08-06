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

#include "dmx_def.h"
#include "dmx_mem.h"
#include "cfa_mp4_state.h"
#include "cfa_mp4_ana.h"
#include "cfa_mp4_util.h"

/*#pragma warning(pop)*/

/*#pragma warning(disable: 4127) //disable warning C4127: conditional expression is constant*/

/*-----------------------------------------------------------------------------
* Name: CfaMp4EncryptData2Fifo
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
void CfaMp4EncryptData2Fifo(void *pvSptHdl, CfaMp4Inst *prCfaMp4)
{
	MRESULT mrRet = RET_DMX_OK;
	DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
		TEXT("[CFA MP4][%s] line %d, enter!!\n"),__func__,__LINE__);

	prCfaMp4->eCurCfaMp4AnaSt = prCfaMp4->eLastCfaMp4AnaSt;
	prCfaMp4->rMarlinCPSInf.fgOn = TRUE;
	prCfaMp4->pu1HdrBuf = (u8 *) prCfaMp4->ptrPfrMemAddress;
	dmx_memcpy(prCfaMp4->rMarlinCPSInf.bIVData, prCfaMp4->pu1HdrBuf,
			  sizeof(prCfaMp4->rMarlinCPSInf.bIVData));
	switch (prCfaMp4->eCurStreamType) {
	case CFA_MP4_VIDEO_AVC:
		prCfaMp4->rMarlinCPSInf.u4IPMPDescriptorID = prCfaMp4->rCfaMp4VInf.u4IPMPID;
		prCfaMp4->rMarlinCPSInf.u8Offset = prCfaMp4->rVidInf.u8FileOfst;
		prCfaMp4->rMarlinCPSInf.u8DecLen = prCfaMp4->rVidInf.u8Len;
		if (CfaMp4GetRangeEa(prCfaMp4) <
			   (prCfaMp4->rVidInf.u8FileOfst + prCfaMp4->rVidInf.u8Len)) {
			DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
		}

		else {
			Spt4CfaTurnCPS(pvSptHdl, &prCfaMp4->rMarlinCPSInf);
			prCfaMp4->rVidInf.u8Len = prCfaMp4->rMarlinCPSInf.u4RealSampleSize;
			prCfaMp4->fgSyncBuf = FALSE;
			CfaMp4TxAvcPayload2Fifo(pvSptHdl, prCfaMp4);
		}
		break;
	case CFA_MP4_VIDEO:
		prCfaMp4->rMarlinCPSInf.u4IPMPDescriptorID = prCfaMp4->rCfaMp4VInf.u4IPMPID;
		prCfaMp4->rMarlinCPSInf.u8Offset = prCfaMp4->rVidInf.u8FileOfst;
		prCfaMp4->rMarlinCPSInf.u8DecLen = prCfaMp4->rVidInf.u8Len;
		if (CfaMp4GetRangeEa(prCfaMp4) <
			   (prCfaMp4->rVidInf.u8FileOfst + prCfaMp4->rVidInf.u8Len)) {
			DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
		}

		else {
			Spt4CfaTurnCPS(pvSptHdl, &prCfaMp4->rMarlinCPSInf);
			prCfaMp4->rVidInf.u8Len = prCfaMp4->rMarlinCPSInf.u4RealSampleSize;
			prCfaMp4->eCurStreamType = CFA_MP4_UNKNOWN;
			prCfaMp4->rVidInf.fgDummyAU = FALSE;
			if (CfaMp4GetRangeEa(prCfaMp4) <
				 (prCfaMp4->rVidInf.u8FileOfst + prCfaMp4->rVidInf.u8Len)) {
				DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
			}

			else {
				mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &(prCfaMp4->rVidInf));
				if (RET_DMX_OK != mrRet) {
					DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA MP4] Spt4CfaPbb2VFifoAUCtrl error ret = %d\n"),
						mrRet);
					CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
					return;
				}
			}
		}
		break;
	case CFA_MP4_AUDIO:
		prCfaMp4->rMarlinCPSInf.u4IPMPDescriptorID =
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4IPMPID;
		prCfaMp4->rMarlinCPSInf.u8Offset = prCfaMp4->rAudInf.u8FileOfst;
		prCfaMp4->rMarlinCPSInf.u8DecLen = prCfaMp4->rAudInf.u8Len;
		if (CfaMp4GetRangeEa(prCfaMp4) <
			   (prCfaMp4->rAudInf.u8FileOfst + prCfaMp4->rAudInf.u8Len)) {
			DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
		}

		else {
			Spt4CfaTurnCPS(pvSptHdl, &prCfaMp4->rMarlinCPSInf);
			prCfaMp4->rAudInf.u8Len = prCfaMp4->rMarlinCPSInf.u4RealSampleSize;
			prCfaMp4->eCurStreamType = CFA_MP4_UNKNOWN;
			prCfaMp4->rAudInf.eAudType = prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].eCfaAudType;
			/*add by mtk68014*/
			if (CfaMp4GetRangeEa(prCfaMp4) <
				(prCfaMp4->rAudInf.u8FileOfst + prCfaMp4->rAudInf.u8Len)) {
				DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
			}

			else {
				prCfaMp4->rAudInf.u8TotalAULen = 0;
				mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &prCfaMp4->rAudInf);
				if (RET_DMX_OK != mrRet) {
					DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA MP4] Spt4CfaPbb2AFifoAUCtrl error ret = %d\n"),
						mrRet);
					CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
					return;
				}
			}
		}
		break;
	case CFA_MP4_SUBPIC:
		prCfaMp4->rMarlinCPSInf.u4IPMPDescriptorID =
			prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u4IPMPID;
		prCfaMp4->rMarlinCPSInf.u8Offset = prCfaMp4->rSubInf.u8FileOfst;
		prCfaMp4->rMarlinCPSInf.u8DecLen = prCfaMp4->rSubInf.u8Len;
		if (CfaMp4GetRangeEa(prCfaMp4) <
			   (prCfaMp4->rSubInf.u8FileOfst + prCfaMp4->rSubInf.u8Len)) {
			DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
		}

		else {
			Spt4CfaTurnCPS(pvSptHdl, &prCfaMp4->rMarlinCPSInf);
			prCfaMp4->rSubInf.u8Len = prCfaMp4->rMarlinCPSInf.u4RealSampleSize;
			prCfaMp4->eCurStreamType = CFA_MP4_UNKNOWN;
			if (CfaMp4GetRangeEa(prCfaMp4) <
				 (prCfaMp4->rSubInf.u8FileOfst + prCfaMp4->rSubInf.u8Len)) {
				DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
			}

			else {
				mrRet = Spt4CfaPbb2SpFifoAUCtrl(pvSptHdl, &prCfaMp4->rSubInf, 0);
				if (RET_DMX_OK != mrRet) {
					DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA MP4] Spt4CfaPbb2SpFifoAUCtrl error ret = %d\n"),
						mrRet);
					CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
					return;
				}
			}
		}
		break;
	case CFA_MP4_ADTS:
		prCfaMp4->rMarlinCPSInf.u4IPMPDescriptorID =
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4IPMPID;

		/*prCfaMp4->rMarlinCPSInf.u4IPMPDescriptorID = 81;*/
		if (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].fgSyncIV) {
			u32 u4AudSamplePerSec = 0;
			u32 u4AudChannels = 0;

			prCfaMp4->rMarlinCPSInf.u8Offset = prCfaMp4->rAudInf.u8FileOfst;
			prCfaMp4->rMarlinCPSInf.u8DecLen = prCfaMp4->rAudInf.u8Len;
			Spt4CfaTurnCPS(pvSptHdl, &prCfaMp4->rMarlinCPSInf);
			prCfaMp4->rAudInf.u8Len = prCfaMp4->rMarlinCPSInf.u4RealSampleSize;
			u4AudSamplePerSec =
				prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4AudSamplePerSec;
			u4AudChannels =
				prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u2AudChannels;
			*prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pucADTSBuf = 0xFF;
			*(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pucADTSBuf + 1) = 0xF9;
			*(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pucADTSBuf + 2) =
				(u8) ((1 << 6) | ((u4AudSamplePerSec << 2) & 0x3C) |
					   ((u4AudChannels >> 2) & 0x1));
			*(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pucADTSBuf + 3) =
				(u8) (((u4AudChannels & 0x3) << 6) |
					   (u32) (((prCfaMp4->rAudInf.u8Len + 7) >> 11) & 0x3));
			*(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pucADTSBuf + 4) =
				(u8) (((prCfaMp4->rAudInf.u8Len + 7) >> 3) & 0xFF);
			*(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pucADTSBuf + 5) =
				(u8) ((((prCfaMp4->rAudInf.u8Len + 7) << 5) & 0xE0) |
					   ((0x7FF >> 6) & 0x1F));
			*(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pucADTSBuf + 6) =
				((0x7FF << 2) & 0xFC);
			prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_TX_ENCRYPT_DATA_TO_FIFO;
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].fgSyncIV = FALSE;
			mrRet =
				Spt4CfaBuf2AFifo(pvSptHdl, prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pucADTSBuf,
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

		else {
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].fgSyncIV = TRUE;
			prCfaMp4->eCurStreamType = CFA_MP4_UNKNOWN;
			prCfaMp4->rAudInf.eAudType = prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].eCfaAudType;
			/*add by mtk68014*/
			prCfaMp4->rAudInf.u8TotalAULen = 0;
			mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &prCfaMp4->rAudInf);
			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4] Spt4CfaPbb2AFifoAUCtrl error ret = %d\n"),
					mrRet);
				CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
				return;
			}
		}
		break;
	default:
		DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
		CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
		break;
	}
}


/*-----------------------------------------------------------------------------
* Name: vCfaMp4TxDoneStCtrl
*
* Description:
*	   MP4 CFA state control for transfer done
*	   This function will be called after a transfer is complete.
*
* Inputs:
*
* Outputs:
*
* Returns: None
*
*-----------------------------------------------------------------------------*/
void CfaMp4TxDoneStCtrl(void *pvSptHdl, CfaMp4Inst *prCfaMp4)
{
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prCfaMp4) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			TEXT("[CFA MP4] prCfaMp4 is NULL\r\n"));
		return;
	}
	do {
		prCfaMp4->fgHasSkipData = FALSE;
		if (prCfaMp4->rMarlinCPSInf.fgOn && (prCfaMp4->eCurStreamType == CFA_MP4_UNKNOWN)) {
			prCfaMp4->rMarlinCPSInf.fgOn = FALSE;
			Spt4CfaTurnCPS(pvSptHdl, &prCfaMp4->rMarlinCPSInf);
		}
		switch (prCfaMp4->eCurCfaMp4AnaSt) {
		case CFA_MP4_ANA_ST_IDLE:

			/*Do nothing.*/
			break;
		case CFA_MP4_ANA_PRS_V_RANGE:
			CfaMp4AnaPrsVRange(pvSptHdl, prCfaMp4);
			break;
		case CFA_MP4_ANA_PRS_A_RANGE:
			CfaMp4AnaPrsARange(pvSptHdl, prCfaMp4);
			break;
		case CFA_MP4_ANA_PRS_S_RANGE:
			CfaMp4AnaPrsSRange(pvSptHdl, prCfaMp4);
			break;
		case CFA_MP4_ANA_GET_V_PTS_TO_FIFO:
			CfaMp4AnaGetVPts2Fifo(pvSptHdl, prCfaMp4);
			break;
		case CFA_MP4_ANA_GET_A_PTS_TO_FIFO:
			CfaMp4AnaGetAPts2Fifo(pvSptHdl, prCfaMp4);
			break;
		case CFA_MP4_ANA_GET_S_PTS_TO_FIFO:
			CfaMp4AnaGetSPts2Fifo(pvSptHdl, prCfaMp4);
			break;
		case CFA_MP4_ANA_RELOAD_TABLE:
			if (!prCfaMp4->fgFinished) {
				mrRet =
					Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaMp4->u8Ca, 16,
							   (u8 *) &prCfaMp4->ptrPfrMemAddress);
				if (RET_DMX_OK != mrRet) {
					DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA MP4] CFA_MP4_ANA_RELOAD_TABLE")
						TEXT("Spt4CfaPbb2SyncBuf error ret = %d\n"),
						mrRet);
					CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
					return;
				}
				prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_NEXT_STATE;
			}
			break;
		case CFA_MP4_ANA_TX_AVC_TO_FIFO:
			CfaMp4TxAvcPayload2Fifo(pvSptHdl, prCfaMp4);
			break;
		case CFA_MP4_ANA_TX_WMV_TO_FIFO:
			CfaMp4TxwmvPayload2Fifo(pvSptHdl, prCfaMp4);
			break;
		case CFA_MP4_ANA_TX_VC1_TO_FIFO:
			CfaMp4TxWVC1Payload2Fifo(pvSptHdl, prCfaMp4);
			break;
		case CFA_MP4_ANA_PRS_NEXT_STATE:
			CfaMp4PrsNextState(pvSptHdl, prCfaMp4);
			break;
		case CFA_MP4_ANA_TX_ENCRYPT_DATA_TO_FIFO:
			CfaMp4EncryptData2Fifo(pvSptHdl, prCfaMp4);
			break;
#if MP4_SUPPORT_FRAGMENT
		case CFA_MP4_ANA_PRS_MOOF_HEADER:
			CfaMp4AnaPrsMoofHeader(pvSptHdl, prCfaMp4);
			break;
		case CFA_MP4_ANA_PRS_MOOF_TRUN:
			CfaMp4AnaPrsMoofTrun(pvSptHdl, prCfaMp4);
			break;
#endif
		default:
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4] state(%d) is error\r\n"),
				prCfaMp4->eCurCfaMp4AnaSt);
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
			return;
		}
	}while (prCfaMp4->fgHasSkipData);
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4LpcmChunk2Fifo
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
void CfaMp4LpcmChunk2Fifo(void *pvSptHdl, CfaMp4Inst *prCfaMp4)
{
	u64 u8Len = 0;
	u64 u8FileOffset = 0;
	MRESULT mrRet = RET_DMX_OK;
	CFA_AUDIO_INFO_T rAudInf = {
	0};

#if SPECIAL_LPCM_SUPPORT
		u32 u4SampleNumberInc = 0xFFFFFFFF;

#endif
	if ((TRUE == prCfaMp4->fgLpcmSet) && (FALSE == prCfaMp4->fgFinished)) {
		prCfaMp4->fgLpcmSet = FALSE;
		Spt4CfaSetLpcmEmphasis(pvSptHdl, TRUE);
	}
	u8FileOffset = prCfaMp4->rCurOfst.u8AudCurOfst;
	prCfaMp4->u8Apts[prCfaMp4->u4CurAudInfoId] = prCfaMp4->u8CurAPts[prCfaMp4->u4CurAudInfoId];
	while (0 < prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4NeedPrsSampleNums) {
		if (0 == prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4SampleSize) {
			u8Len += (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pTSampleInfo +
				   (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudSampleNo -
				   prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurAudTableStartSampleNo))
				   ->u4SampleSize;
		}
#if SPECIAL_LPCM_SUPPORT
		else if (4 >= prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4SampleSize)
			/* && sample count == 1)*/
		{
			u4SampleNumberInc = MIN_AUD_SMP_SZ;
			if (u4SampleNumberInc >
				 prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4NeedPrsSampleNums) {
					u4SampleNumberInc =
					prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4NeedPrsSampleNums;
			}
			u8Len +=
				(prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4SampleSize *
				 u4SampleNumberInc *
				 prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u2AudChannels);
		}
#endif
		else
			u8Len += prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4SampleSize;

		prCfaMp4->u8CurAPts[prCfaMp4->u4CurAudInfoId] =
		CfaMp4PrsSttsGetPts(prCfaMp4, Cfa_Mp4_Aud_Track,
				prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudSampleNo
#if SPECIAL_LPCM_SUPPORT
				, u4SampleNumberInc
#endif
		);
		if (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudSampleNo ==
			   (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurAudTableEndSampleNo -
			1)) {
			prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_RELOAD_TABLE;
		} else {
			prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_GET_A_PTS_TO_FIFO;
		}
		if (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudSampleNo ==
			 prCfaMp4->rCfaRange.u8AudEndSampleNo[prCfaMp4->u4CurAudInfoId]) {
				prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_NEXT_STATE;
		}

#if SPECIAL_LPCM_SUPPORT
		if (MIN_AUD_SMP_SZ >= u4SampleNumberInc) {
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudSampleNo +=
				u4SampleNumberInc;
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4NeedPrsSampleNums -=
				u4SampleNumberInc;
		} else {
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudSampleNo++;
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4NeedPrsSampleNums--;
		}

#else
		prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudSampleNo++;
		prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4NeedPrsSampleNums--;

#endif
		if (0 == prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4NeedPrsSampleNums) {
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudChunkNo++;
			if (CFA_MP4_ANA_RELOAD_TABLE != prCfaMp4->eCurCfaMp4AnaSt)
				prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_NEXT_STATE;
			break;
		}
		if (prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurPrsAudSampleNo ==
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurAudTableEndSampleNo)
			break;
	}
	if (FALSE == prCfaMp4->fgFinished)
		Spt4CfaPTSNotify(pvSptHdl, prCfaMp4->u8Apts[prCfaMp4->u4CurAudInfoId]);

	prCfaMp4->rCurOfst.u8AudCurOfst += u8Len;
	rAudInf.u8FileOfst = u8FileOffset;
	rAudInf.u8Len = u8Len;
	rAudInf.u8Pts = prCfaMp4->u8Apts[prCfaMp4->u4CurAudInfoId];
	rAudInf.fgUnitStart = FALSE;

	/*rAudInf.u4PrsStrmId = prCfaMp4->u4CurAudInfoId + MAX_NS_MP4_VID;*/
	rAudInf.u4PrsStrmId = prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4TrackID;
	/*add by zhiwei chen for streamid 2011.3.17*/
	rAudInf.eAudType = prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].eCfaAudType;
	/*add by mtk68014*/
	prCfaMp4->eCurPrsSampleType = CFA_MP4_PRS_BIT_STRM_TYPE_A;
	if (FALSE == prCfaMp4->fgFinished) {
		if ((CfaMp4GetRangeEa(prCfaMp4) < (rAudInf.u8FileOfst + rAudInf.u8Len))
			 || (rAudInf.u8FileOfst < prCfaMp4->u8Ca) || (0 == rAudInf.u8Len)) {
			DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4][%s] line %d,call cfamp4finishprs\n"),__func__,__LINE__);
			CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
		}

		else {
			prCfaMp4->u8Ca = rAudInf.u8FileOfst + rAudInf.u8Len;

			/*rAudInf.u8TotalAULen = 0;*/
			/*u4Ret = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rAudInf, 0);*/
			if (rAudInf.u8Pts >= prCfaMp4->rCfaRange.u8SeekPts) {	/* 2011.4.25*/
				rAudInf.u8TotalAULen = 0;
				mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rAudInf);
				if (RET_DMX_OK != mrRet) {
					DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA MP4] Spt4CfaPbb2AFifoAUCtrl error ret = %d\n"),
						mrRet);
					CfaMp4FinishPrs(pvSptHdl, prCfaMp4);
					return;
				}
				if (!(prCfaMp4->u4PrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_V))
				{
					Sleep(1);
				}

			} else {
				/*skip this data*/
				prCfaMp4->fgHasSkipData = TRUE;
			}
		}
	}
}
