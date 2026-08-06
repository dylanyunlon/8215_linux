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

#include "dmx_def.h"
#include "dmx_mem.h"
#include "cfa_macro.h"
#include "cfa_if.h"
#include "cfa_asf.h"
#include "cfa_asf_st_ctrl.h"

/*#pragma warning(disable: 6011) *//*disable  warning C6011: Dereferencing NULL pointer checking*/
/*#pragma warning(disable: 6297) *//*disable  warning C6297: Arithmetic overflow*/
/*#pragma warning(disable: 6385) *//*disable  warning C6385: Invalid data: accessing*/

#define INVALID_EXT_SYS_ENCRYPTION_ID	0xFFFFFFFFFFFFFFFFull

#if ASF_AAC_SUPPORT
static u32 MapAacSampleRate(u32 u4SampleFreq)
{
	switch (u4SampleFreq) {
	case (u32)96000:
		return (u32)0x0;

	case (u32)88200:
		return (u32)0x1;

	case (u32)64000:
		return (u32)0x2;

	case (u32)48000:
		return (u32)0x3;

	case (u32)44100:
		return (u32)0x4;

	case (u32)32000:
		return (u32)0x5;

	case (u32)24000:
		return (u32)0x6;

	case (u32)22050:
		return (u32)0x7;

	case (u32)16000:
		return (u32)0x8;

	case (u32)12000:
		return (u32)0x9;

	case (u32)11025:
		return (u32)0xa;

	case (u32)8000:
		return (u32)0xb;

	case (u32)7350:
		return (u32)0xc;

	default:
		return (u32)0xff;
	}
}

static void SetAacADTSHdr(CfaAsfInst_T *prCfaAsf)
{
	u32 u4AudSamplePerSec = 0;
	u32 u4AudChannels = 0;
	u32 u4FrameSize = 0;
	u8 u1AuHeader[7] = {0};

	u4AudSamplePerSec = MapAacSampleRate(prCfaAsf->raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx].u4SamPS);
	u4AudChannels = prCfaAsf->raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx].u2ChannelNum;
	u4FrameSize = prCfaAsf->u4MediaObjSize + (u32)7;

	u1AuHeader[0] = 0xFF;
	u1AuHeader[1] = 0xF9;
	u1AuHeader[2] = (u8)(((u32)1 << 6) | ((u4AudSamplePerSec << 2) & (u32)0x3C) | ((u4AudChannels >> 2) & (u32)0x1));
	u1AuHeader[3] = (u8)(((u4AudChannels & (u32)0x3) << 6) | ((u4FrameSize >> 11) & (u32)0x3));
	u1AuHeader[4] = (u8)((u4FrameSize >> 3) & (u32)0xFF);
	u1AuHeader[5] = (u8)(((u4FrameSize << 5) & (u32)0xE0) | (((u32)0x7FF >> 6) & (u32)0x1F));
	u1AuHeader[6] = (u8)(((u16)0x7FF << 2) & (u16)0xFC);

	dmx_memcpy((void *)(prCfaAsf->raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx].auheader),
		   (void *)(&u1AuHeader[0]),
		   (u32)7);
}
#endif //ASF_AAC_SUPPORT


/*//Description: When asf cfa finished parsing, call this function.*/
/**/
/*//@return none*/
static void CfaAsfFinishPrs(void *pvSptHdl, CfaAsfInst_T *prCfaAsf)
{
	u64 u8Ea = 0;

	MMATE_CHECK_STRUCT(prCfaAsf->rCfaAsfRange);
	u8Ea = prCfaAsf->rCfaAsfRange.u8VidEa;
	prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_IDLE;
	DmxLogD(DMX_MOD_CFA_ASF,CFA_ASF_LOG_STATE,TEXT("[CFA ASF] Finished parsing for current range!\n"));
	DmxLogT(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[CFA ASF]CfaAsfFinishPrs u8Ea: 0x%x%x\n"), (u32)(u8Ea >> 32), (u32) u8Ea);

	if (DMX_IS_FF_PLAY(pvSptHdl) &&
		(0 == (prCfaAsf->u4CurPrsFlag & CFA_ASF_PRS_BIT_STRM_TYPE_VID)))
		Spt4CfaFinishedEx(pvSptHdl, u8Ea, (BOOL)TRUE, (u32)GAU_E_EOS);
	else
		Spt4CfaFinishedEx(pvSptHdl, u8Ea, (BOOL)FALSE, (u32)GAU_E_EOS);

	prCfaAsf->fgNoSyncPbb = TRUE;
}

static CfaApiAudType CfaAsfGetAudType(AVCODECID_T eAudCodec)
{
	CfaApiAudType eMappedAudType = CFA_AUD_DRV_FMT_UNKNOWN;

	switch (eAudCodec) {
	case AVCODEC_ID_MPEG:
		eMappedAudType = CFA_AUD_DRV_FMT_MPEG;
		break;

	case AVCODEC_ID_AC3:
		eMappedAudType = CFA_AUD_DRV_FMT_AC3;
		break;

	case AVCODEC_ID_PCM:
		eMappedAudType = CFA_AUD_DRV_FMT_PCM;
		break;

	case AVCODEC_ID_MP3:
		eMappedAudType = CFA_AUD_DRV_FMT_MP3;
		break;

	case AVCODEC_ID_DTS:
		eMappedAudType = CFA_AUD_DRV_FMT_DTS;
		break;

	case AVCODEC_ID_WMA:
		eMappedAudType = CFA_AUD_DRV_FMT_WMA;
		break;

	case AVCODEC_ID_AAC_PURE:
	case AVCODEC_ID_AAC:
		eMappedAudType = CFA_AUD_DRV_FMT_AAC;
		break;

	default:
		eMappedAudType = CFA_AUD_DRV_FMT_UNKNOWN;
		break;
		}

	return eMappedAudType;
}

/*add by mcn08025@2008-11-24*/
/*//Description: set EOS before notify splliter demux error*/
/*/return none*/
static void CfaAsfSetSplliterEOS(void *pvSptHdl, CfaAsfInst_T *prCfaAsf)
{
	Cfa2PsrStrmInfo rPathStrmInfo = {0};
	u32 i = 0;
	u32 j = 0;

	if (prCfaAsf->u4CurPrsFlag & CFA_ASF_PRS_BIT_STRM_TYPE_VID) {
		rPathStrmInfo.u4VstrmNs = 1;

		for (i = 0; i < rPathStrmInfo.u4VstrmNs; i++)
			rPathStrmInfo.ucDecVidStId[i] = (u8)(prCfaAsf->rCfaAsfVidInfo.rCfaAsfStrmInfo.u1StrmNum);
	}

	if (prCfaAsf->u4CurPrsFlag & CFA_ASF_PRS_BIT_STRM_TYPE_AUD) {
		if (prCfaAsf->u1CurAudInfoIdx >= MAX_ASF_AUD_STRM_NUM)
			prCfaAsf->u1CurAudInfoIdx = 0;

		rPathStrmInfo.u4AstrmNs = 1;

		for (j = 0; j < rPathStrmInfo.u4AstrmNs; j++) {
			rPathStrmInfo.u2DecAudStId[j] = 
				prCfaAsf->raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx].rCfaAsfStrmInfo.u1StrmNum;
		}
	}

	Spt4CfaFinishedEx(pvSptHdl, prCfaAsf->u8Ca, (BOOL)TRUE, (u32)GAU_E_FAIL);
}

/*//Description: Tx video spec codec data to Video decoder, just one time enough.*/
/**/
/*//@return*/
static void CfaAsfFillVidSpecCodecInfo(void *pvSptHdl, CfaAsfInst_T *prCfaAsf)
{
	CFA_VIDEO_INFO_T rVidInfo = {0};
	MRESULT mrRet = RET_DMX_OK;
	/*Get sequence info from codec spec data.*/
	MMATE_CHECK_STRUCT(prCfaAsf->rCfaAsfVidInfo);

	if ((prCfaAsf->rCfaAsfVidInfo.u4CodecSpecDataLen >= 4) &&
		(prCfaAsf->eVidCodecType == CFA_VID_WMV9)) {
		prCfaAsf->fgPrsSeqFrameInterpolation = (prCfaAsf->rCfaAsfVidInfo.pu1VidCodecSpecData[3] & 0x02) >> 1;
		prCfaAsf->fgPrsPreProcRange = (prCfaAsf->rCfaAsfVidInfo.pu1VidCodecSpecData[3] & 0x80) >> 7;
		prCfaAsf->u4PrsNumBFrames = (prCfaAsf->rCfaAsfVidInfo.pu1VidCodecSpecData[3] & 0x70) >> 4;
	}

	DmxLogD(DMX_MOD_CFA_ASF,CFA_ASF_LOG_TXDONE,TEXT("[CFA ASF] Transfering video codec specific data to VFifo!\n"));

	/*Modified by jie.tang*/
	if (((prCfaAsf->eVidCodecType) == CFA_VID_MPEG2) ||
		((prCfaAsf->eVidCodecType) == CFA_VID_WMV8) ||
		((prCfaAsf->eVidCodecType) == CFA_VID_WMV9)  ||
		((prCfaAsf->eVidCodecType) == CFA_VID_VC1) ||
		((prCfaAsf->eVidCodecType) == CFA_VID_DIVX6)) {
		rVidInfo.eTxMode = CFA_PTM_WMV_SEQHDR;
		rVidInfo.fgUnitStart = TRUE;
		rVidInfo.u8TotalAULen = prCfaAsf->rCfaAsfVidInfo.u4CodecSpecDataLen;
	} else {
		rVidInfo.fgUnitStart = FALSE;
		rVidInfo.u8TotalAULen = 0;
		rVidInfo.eTxMode = CFA_PTM_EXACT_POS;
	}

	rVidInfo.eVidType = prCfaAsf->eVidCodecType;
	rVidInfo.u8FileOfst = prCfaAsf->rCfaAsfVidInfo.u8CodecSpecDataOft;
	rVidInfo.u8Len = prCfaAsf->rCfaAsfVidInfo.u4CodecSpecDataLen;
	rVidInfo.u4PrsStrmId = prCfaAsf->rCfaAsfVidInfo.rCfaAsfStrmInfo.u1StrmNum;
	mrRet = Spt4CfaBuf2VFifoAUCtrl(pvSptHdl, prCfaAsf->rCfaAsfVidInfo.pu1VidCodecSpecData, &rVidInfo);
	prCfaAsf->fgNeedEBIHInfo = FALSE;

	if (RET_DMX_OK != mrRet) {
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[ASF CFA] call Spt4CfaBuf2VFifoAUCtrl()")
			TEXT("retun err(%d) in CfaAsfFillVidSpecCodecInfo(),call FinishPrs: %x\n."),
				mrRet, 0x01);
		CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
		return;
	}
}


/*//Descripiton: Start to search next packet header and get parsing payload info.*/
/*//@return None*/
/*/<[IN] handle of splitter*/
/*/<[IN] pointer to CfaAsfInst*/
/*/<[IN] next analyze state*/
/*/<[IN] previous transfer file length before search next start code*/
/*/<[IN] the data length we want to read*/
/*/<[IN] header buffer ofst. to put the data from this ofst*/
EXTERN void CfaAsfSearchHeader(void *pvSptHdl, CfaAsfInst_T *prCfaAsf,
				   CfaAsfAnaSt_E eNextAnaSt, u64 u8PreLen,
				   u64 u8ReadLen, u32 u4DestOft)
{
	u64 u8Sa = 0;
	MRESULT mrRet = RET_DMX_OK;

	u8Sa = prCfaAsf->u8Ca;
	if ((prCfaAsf->rCfaAsfRange.u8AudSa >= prCfaAsf->rCfaAsfRange.u8AudEa) ||
		(prCfaAsf->rCfaAsfRange.u8VidSa >= prCfaAsf->rCfaAsfRange.u8VidEa))
	{
		DmxLogT(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,
					TEXT("[ASF CFA] in CfaAsfSearchHeader(), u8Ca is equal u8Ea,should finish parser!")
					TEXT(" u8AudSa:0x%llx, u8AudEa:0x%llx, u8VidSa:0x%llx, u8VidEa:0x%llx.\n"),
					prCfaAsf->rCfaAsfRange.u8AudSa, prCfaAsf->rCfaAsfRange.u8AudEa, 
					prCfaAsf->rCfaAsfRange.u8VidSa, prCfaAsf->rCfaAsfRange.u8VidEa);
		CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
		return;
	}
	MMATE_CHECK_STRUCT(prCfaAsf->rCfaAsfRange);
	MMATE_CHECK_STRUCT(prCfaAsf->rCfaAsfVidInfo);

	if ((TRUE == (prCfaAsf->fgNeedSyncBufForSpecData))
		&& (TRUE == (prCfaAsf->fgEnableVid))
		&& ((prCfaAsf->rCfaAsfVidInfo.u4CodecSpecDataLen) != 0)
		&& (TRUE == (prCfaAsf->fgNeedEBIHInfo))) 
	{
		/* Need sync buffer to send inband command to filter first*/
		if (u8Sa + u8ReadLen >= prCfaAsf->rCfaAsfRange.u8VidEa) {
			if (prCfaAsf->rCfaAsfRange.u8VidEa < u8Sa)
			{
				DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] u8VidEa is incorrect in CfaAsfSearchHeader().\r\n"));
				CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
				return;
			}
			u8ReadLen = prCfaAsf->rCfaAsfRange.u8VidEa - u8Sa;
		}

		if (prCfaAsf->u4MemDataLen > (u8Sa - prCfaAsf->u8LastCa)) {
			prCfaAsf->u4MemDataLen -= (u32)(u8Sa - prCfaAsf->u8LastCa);
			prCfaAsf->ptrPfrMemAddress += (uintptr_t)(u8Sa - prCfaAsf->u8LastCa);
		} else
			prCfaAsf->u4MemDataLen = 0;

		if (prCfaAsf->u4MemDataLen > u8ReadLen) 
		{
			/* RETAILMSG(1, (L"NO SYNC PBBUF++++++++++++++++++++++++++++++++++++++++\n"));*/
			prCfaAsf->fgNoSyncPbb = TRUE;
		} 
		else 
		{
			/*u4Ret = Spt4CfaPbb2SyncBuf(pvSptHdl, u8Sa, u8ReadLen, (u8 *)
			&prCfaAsf->ptrPfrMemAddress);*/
			mrRet = Spt4CfaPbb2SyncBufEx(pvSptHdl, u8Sa, u8ReadLen, (u8 *)
							 &prCfaAsf->ptrPfrMemAddress, &prCfaAsf->u4MemDataLen);

			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,
					TEXT("[ASF CFA] call Spt4CfaPbb2SyncBufEx()")
					TEXT("retun err(%d) in CfaAsfSearchHeader(),call FinishPrs: %x.\n"),
						mrRet, 0x03);
				CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
				return;
			}

			prCfaAsf->fgRealSyncPbbuf = TRUE;
		}

		prCfaAsf->u8LastCa = u8Sa;


		/*u4Ret = Spt4CfaPbb2SyncBuf(pvSptHdl, u8Sa, u8ReadLen,
		 (u8 *)&prCfaAsf->ptrPfrMemAddress);*/
		/*DMX_ASSERT(RET_DMX_OK == u4Ret);*/
		prCfaAsf->fgTxData2HdrBuf = TRUE;
		prCfaAsf->fgNeedSyncBufForSpecData	= FALSE;
		prCfaAsf->eCurCfaAsfTxStrmType = CFA_ASF_TX_STRM_TYPE_NONE;
		prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_SEARCH_HEADER;
		return;
	}

	if (prCfaAsf->rCfaAsfVidInfo.u4CodecSpecDataLen != 0) {
		if ((TRUE == (prCfaAsf->fgEnableVid)) && (TRUE == (prCfaAsf->fgNeedEBIHInfo)) &&
			((prCfaAsf->u4CurPrsFlag) & CFA_ASF_PRS_BIT_STRM_TYPE_VID)) {
			prCfaAsf->eCurCfaAsfTxStrmType = CFA_ASF_TX_STRM_TYPE_NONE;
			prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_SEARCH_HEADER;
			CfaAsfFillVidSpecCodecInfo(pvSptHdl, prCfaAsf);
			return;
		}
	}

	u8Sa = prCfaAsf->u8Ca;

	if ((prCfaAsf->u4PacketLen == prCfaAsf->rCfaAsfFileInfo.u4DataPacketSize) &&
		(0 != prCfaAsf->rCfaAsfFileInfo.u4DataPacketSize)) {
		/*get current data packet id.*/
		prCfaAsf->u8CurPacketId = (prCfaAsf->u8Ca - prCfaAsf->rCfaAsfFileInfo.u8HeaderObjectSize - CFA_ASF_DATA_OBJECT_HDR_SIZE) /
					  prCfaAsf->rCfaAsfFileInfo.u4DataPacketSize;

		if ((((prCfaAsf->u8CurPacketId) + 1) > (prCfaAsf->rCfaAsfFileInfo.u8DataPacketCount))
			|| (((prCfaAsf->u8Ca) >= 1) && (((prCfaAsf->u8Ca) - 1) >= (prCfaAsf->rCfaAsfRange.u8VidEa)))) {
			MMATE_CHECK_STRUCT(prCfaAsf->rCfaAsfRange);
			CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
			return;
		}
	} else {
		if (prCfaAsf->u8Ca >= 1) {
			if ((prCfaAsf->u8Ca - 1) >= prCfaAsf->rCfaAsfRange.u8VidEa) {
				MMATE_CHECK_STRUCT(prCfaAsf->rCfaAsfRange);
				CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
				return;
			}
		}
	}

	if (prCfaAsf->eCurCfaAsfPrsBitStrmType == CFA_ASF_PRS_BIT_STRM_TYPE_PAYLOAD_HDR)
		prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_SEARCH_PAYLOAD_HEADER;
	else if (prCfaAsf->eCurCfaAsfPrsBitStrmType == CFA_ASF_PRS_BIT_STRM_TYPE_PACKET_HDR)
		prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_SEARCH_PACKET_HEADER;
	else
		prCfaAsf->eCurCfaAsfAnaSt = eNextAnaSt;

	if (u8Sa + u8ReadLen >= prCfaAsf->rCfaAsfRange.u8VidEa) {
		MMATE_CHECK_STRUCT(prCfaAsf->rCfaAsfRange);
		if (prCfaAsf->rCfaAsfRange.u8VidEa < u8Sa)
		{
			DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] u8VidEa error in CfaAsfSearchHeader().\r\n"));
			CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
			return;
		}
		u8ReadLen = prCfaAsf->rCfaAsfRange.u8VidEa - u8Sa;
	}

	if (prCfaAsf->u4MemDataLen > (u8Sa - prCfaAsf->u8LastCa)) {
		prCfaAsf->u4MemDataLen -= (u32)(u8Sa - prCfaAsf->u8LastCa);
		prCfaAsf->ptrPfrMemAddress += (uintptr_t)(u8Sa - prCfaAsf->u8LastCa);
	} else
		prCfaAsf->u4MemDataLen = 0;

	if (prCfaAsf->u4MemDataLen > u8ReadLen) {
		/* RETAILMSG(1, (L"NO SYNC PBBUF++++++++++++++++++++++++++++++++++++++++\n"));*/
		prCfaAsf->fgNoSyncPbb = TRUE;
		} 
	else 
	{
		/*u4Ret = Spt4CfaPbb2SyncBuf(pvSptHdl, u8Sa, u8ReadLen,
		 (u8 *)&prCfaAsf->ptrPfrMemAddress);*/
		mrRet = Spt4CfaPbb2SyncBufEx(pvSptHdl, u8Sa, u8ReadLen, (u8 *) &prCfaAsf->ptrPfrMemAddress,
						 &prCfaAsf->u4MemDataLen);

		if (RET_DMX_OK != mrRet) {
			DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,
				TEXT("[ASF CFA] call Spt4CfaPbb2SyncBufEx()")
				TEXT("retun err(%d) in CfaAsfSearchHeader(),call FinishPrs: %x.\n"),
					mrRet, 0x04);
			CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
			return;
		}

		prCfaAsf->fgRealSyncPbbuf = TRUE;
	}

	prCfaAsf->u8LastCa = u8Sa;

	if (RET_DMX_OK != mrRet)
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] mrRet err(%d) in CfaAsfSearchHeader().\n"),mrRet);
		CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
		return;
	}
	prCfaAsf->fgTxData2HdrBuf = TRUE;
}

#if CFA_ASF_SYNC_BUF_BEFORE_TX
static MRESULT CfaAsfSyncBuf(void *pvSptHdl, CfaAsfInst_T *prCfaAsf, u64 u8Sa,
				 u64 u8SyncLen, bool fgTurnOnCps, u64 u8DecLen)
{
	MRESULT mrRet = RET_DMX_OK;

	DmxLogD(DMX_MOD_CFA_ASF,CFA_ASF_LOG_SET,TEXT("[CFA ASF] Sync buf %lld, %lld, %d, %lld\n"), u8Sa,
			u8SyncLen, fgTurnOnCps, u8DecLen);

	if (fgTurnOnCps) {	 /* turn ON CPS before sync buffer when necessary*/
		prCfaAsf->fgDrmEncrypt = TRUE;
		prCfaAsf->rCfaAsfCpsInfo.fgOn = TRUE;

		if (2 == prCfaAsf->rCfaAsfDrmInfo.u1WMDRMType) {
			prCfaAsf->rCfaAsfCpsInfo.u4CpsType = CFA_CPS_TYPE_WMDRM_ND;
			/*CFA_CPS_TYPE_WMDRM_PD/CFA_CPS_TYPE_WMDRM_ND*/
		} else {
			prCfaAsf->rCfaAsfCpsInfo.u4CpsType = CFA_CPS_TYPE_WMDRM_PD;
			/*CFA_CPS_TYPE_WMDRM_PD/CFA_CPS_TYPE_WMDRM_ND*/
		}

		prCfaAsf->rCfaAsfCpsInfo.u8Offset = u8Sa;
		prCfaAsf->rCfaAsfCpsInfo.u4MaxPacketSize = prCfaAsf->rCfaAsfFileInfo.u4DataPacketSize;
		prCfaAsf->rCfaAsfCpsInfo.u8DecLen = u8DecLen;
		prCfaAsf->rCfaAsfCpsInfo.u4MediaObjSize = prCfaAsf->u4MediaObjSize;
		dmx_memcpy((void *) prCfaAsf->rCfaAsfCpsInfo.bSampleID, (void *)(&prCfaAsf->u8ExtSampleId), 8);

		mrRet = Spt4CfaTurnCPS(pvSptHdl, &prCfaAsf->rCfaAsfCpsInfo);

		if (RET_DMX_OK != mrRet) {
			DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[ASF CFA] call Spt4CfaTurnCPS() retun err(%d)")
				TEXT("in CfaAsfSyncBuf(),call FinishPrs: %x.\n"), mrRet, 0x05);
			CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
			MM_RETURN(mrRet);
		}
	}


	if (prCfaAsf->u4MemDataLen > (u8Sa - prCfaAsf->u8LastCa)) {
		prCfaAsf->u4MemDataLen -= (u32)(u8Sa - prCfaAsf->u8LastCa);
		prCfaAsf->ptrPfrMemAddress += (uintptr_t)(u8Sa - prCfaAsf->u8LastCa);
	} else
		prCfaAsf->u4MemDataLen = 0;

	if (prCfaAsf->u4MemDataLen > u8SyncLen) {
		/* RETAILMSG(1, (L"NO SYNC PBBUF++++++++++++++++++++++++++++++++++++++++\n"));*/
		prCfaAsf->fgNoSyncPbb = TRUE;
		} else {
		/*u4Ret = Spt4CfaPbb2SyncBuf(pvSptHdl, u8Sa, u8SyncLen,
		(u8 *)&prCfaAsf->ptrPfrMemAddress);*/
		mrRet = Spt4CfaPbb2SyncBufEx(pvSptHdl, u8Sa, u8SyncLen, (u8 *) &prCfaAsf->ptrPfrMemAddress, &prCfaAsf->u4MemDataLen);

		if (RET_DMX_OK != mrRet) {
			DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[ASF CFA] call Spt4CfaPbb2SyncBufEx()")
				TEXT("retun err(%d) in CfaAsfSyncBuf(),call FinishPrs: %x.\n"), mrRet, 0x05);
			CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
			MM_RETURN(mrRet);
		}

		prCfaAsf->fgRealSyncPbbuf = TRUE;
	}

	prCfaAsf->u8LastCa = u8Sa;

	prCfaAsf->fgTxData2HdrBuf = TRUE;

	MM_RETURN(mrRet);
}
#endif //CFA_ASF_SYNC_BUF_BEFORE_TX

/*//Descripition: Error handle for skiping error data in asf file.*/
/**/
/*//@return NONE.*/
static void CfaAsfSkipErr2NextPacket(void *pvSptHdl, CfaAsfInst_T *prCfaAsf)
{
	prCfaAsf->fgSkipErrPacket = TRUE;
	prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_SEARCH_HEADER;
	prCfaAsf->eCurCfaAsfPrsBitStrmType = CFA_ASF_PRS_BIT_STRM_TYPE_PACKET_HDR;
	prCfaAsf->u8Ca = prCfaAsf->u8PacketStartAdr + prCfaAsf->u4PacketLen;
	prCfaAsf->u8PacketStartAdr += prCfaAsf->u4PacketLen;
	DmxLogD(DMX_MOD_CFA_ASF,CFA_ASF_LOG_SET,TEXT("[CFA ASF Err] Skip2NextPacketSa: 0x%llx\n"), prCfaAsf->u8Ca);
	CfaAsfSearchHeader(pvSptHdl, prCfaAsf, CFA_ASF_ANA_ST_SEARCH_PACKET_HEADER, (u64)0, (u64)CFA_ASF_HDR_BUF_SZ, (u32)0);
}


/*//Description: Check error correction data exist in data packet*/
/*//@return value: true-->error corretion data exist*/
/*//					 false-->error correction data do not exist.*/
static bool CfaAsfExistErrCorData(void *pvSptHdl, CfaAsfInst_T *prCfaAsf)
{
	u8 u1PacketFirstByte = 0;

	u1PacketFirstByte = *prCfaAsf->pu1HdrBuf;

	/*/get the first bit from the data packet first byte.*/
	if ((u1PacketFirstByte >> 7) == (u8)1) {
		prCfaAsf->rCfaAsfErrCorFlag.fgErrCorExist = TRUE;
		prCfaAsf->rCfaAsfErrCorFlag.u1DataLen = ((u8) *prCfaAsf->pu1HdrBuf) & (u8)0x0F;
		return TRUE;
	}
	prCfaAsf->rCfaAsfErrCorFlag.fgErrCorExist = FALSE;
	prCfaAsf->rCfaAsfErrCorFlag.u1DataLen = 0;
	return FALSE;
}

bool CfaAsfExsitVc1StartCode(u32 u4Vc1Sc)
{
	switch (u4Vc1Sc) {
	/*VC1 start code*/
	case (u32)0x0D010000:
	case (u32)0x0E010000:
	case (u32)0x0F010000:
		return TRUE;

	default:
		return FALSE;
	}
}

/*//Description:get Length Type Flags.*/
/*//@return none.*/
static MRESULT CfaAsfGetLenTypeFlag(u8 u1OneByte, CfaAsfLenTypeFlag_T *prCfaAsfLenTypeFlag,
				 CfaAsfInst_T *prCfaAsf)
{
	if ((u1OneByte & (u8)0x01) == (u8)1) {
		prCfaAsf->fgExistMultiPayload = TRUE;
		prCfaAsf->fgFirstParsingMultiPayload = TRUE;
	} else
		prCfaAsf->fgExistMultiPayload = FALSE;

	switch ((u1OneByte & (u8)0x06) >> 1) {
	case (u8)0:
		prCfaAsfLenTypeFlag->eSequenceType = Field_Type_UNEXIST;
		break;

	case (u8)1:
		prCfaAsfLenTypeFlag->eSequenceType = Field_Type_BYTE;
		break;

	case (u8)2:
		prCfaAsfLenTypeFlag->eSequenceType = Field_Type_WORD;
		break;

	case (u8)3:
		prCfaAsfLenTypeFlag->eSequenceType = Field_Type_DWORD;
		break;

	default:
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] eSequenceType is incorrect in CfaAsfGetLenTypeFlag().\r\n"));
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	switch ((u1OneByte & (u8)0x18) >> 3) {
	case (u8)0:
		prCfaAsfLenTypeFlag->ePadLenType = Field_Type_UNEXIST;
		break;

	case (u8)1:
		prCfaAsfLenTypeFlag->ePadLenType = Field_Type_BYTE;
		break;

	case (u8)2:
		prCfaAsfLenTypeFlag->ePadLenType = Field_Type_WORD;
		break;

	case (u8)3:
		prCfaAsfLenTypeFlag->ePadLenType = Field_Type_DWORD;
		break;

	default:
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] ePadLenType is incorrect in CfaAsfGetLenTypeFlag().\r\n"));
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	switch ((u1OneByte & (u8)0x60) >> 5) {
	case (u8)0:
		prCfaAsfLenTypeFlag->ePacketLenType = Field_Type_UNEXIST;
		break;

	case (u8)1:
		prCfaAsfLenTypeFlag->ePacketLenType = Field_Type_BYTE;
		break;

	case (u8)2:
		prCfaAsfLenTypeFlag->ePacketLenType = Field_Type_WORD;
		break;

	case (u8)3:
		prCfaAsfLenTypeFlag->ePacketLenType = Field_Type_DWORD;
		break;

	default:
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] ePacketLenType is incorrect in CfaAsfGetLenTypeFlag().\r\n"));
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	MM_RETURN(RET_DMX_OK);
}

/*//Description:get Property Flags.*/
/*//@return none.*/
MRESULT CfaAsfGetPropertyFlag(u8 u1OneByte, CfaAsfInst_T *prCfaAsf)
{
	switch ((u1OneByte & (u8)0x03)) {
	case (u8)0:
		prCfaAsf->rCfaAsfPropertyFlag.eRepDataLenType = Field_Type_UNEXIST;
		break;

	case (u8)1:
		prCfaAsf->rCfaAsfPropertyFlag.eRepDataLenType = Field_Type_BYTE;
		break;

	case (u8)2:
		prCfaAsf->rCfaAsfPropertyFlag.eRepDataLenType = Field_Type_WORD;
		break;

	default:
		prCfaAsf->rCfaAsfPropertyFlag.eRepDataLenType = Field_Type_DWORD;
		break;
	}

	switch ((u1OneByte & (u8)0x0C) >> 2) {
	case (u8)0:
		prCfaAsf->rCfaAsfPropertyFlag.ePreTimeType = Field_Type_UNEXIST;
		break;

	case (u8)1:
		prCfaAsf->rCfaAsfPropertyFlag.ePreTimeType = Field_Type_BYTE;
		break;

	case (u8)2:
		prCfaAsf->rCfaAsfPropertyFlag.ePreTimeType = Field_Type_WORD;
		break;

	case (u8)3:
		prCfaAsf->rCfaAsfPropertyFlag.ePreTimeType = Field_Type_DWORD;
		break;

	default:
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] ePreTimeType is incorrect in CfaAsfGetPropertyFlag().\r\n"));
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	switch ((u1OneByte & (u8)0x30) >> 4) {
	case (u8)0:
		prCfaAsf->rCfaAsfPropertyFlag.eObjNumType = Field_Type_UNEXIST;
		break;

	case (u8)1:
		prCfaAsf->rCfaAsfPropertyFlag.eObjNumType = Field_Type_BYTE;
		break;

	case (u8)2:
		prCfaAsf->rCfaAsfPropertyFlag.eObjNumType = Field_Type_WORD;
		break;

	case (u8)3:
		prCfaAsf->rCfaAsfPropertyFlag.eObjNumType = Field_Type_DWORD;
		break;

	default:
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] eObjNumType is incorrect in CfaAsfGetPropertyFlag().\r\n"));
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	switch ((u1OneByte & (u8)0xC0) >> 6) {
	case (u8)0:
		prCfaAsf->rCfaAsfPropertyFlag.eStrmNumType = Field_Type_UNEXIST;
		break;

	case (u8)1:
		prCfaAsf->rCfaAsfPropertyFlag.eStrmNumType = Field_Type_BYTE;
		break;

	case (u8)2:
		prCfaAsf->rCfaAsfPropertyFlag.eStrmNumType = Field_Type_WORD;
		break;

	case (u8)3:
		prCfaAsf->rCfaAsfPropertyFlag.eStrmNumType = Field_Type_DWORD;
		break;

	default:
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] eStrmNumType is incorrect in CfaAsfGetPropertyFlag().\r\n"));
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	
	MM_RETURN(RET_DMX_OK);
}

/*//Description: get Payload parsing information last bytes to skip*/
/*//					 get padding length of Data Packet.*/
/*//@return skiped bytes*/
MRESULT CfaAsfGetSkipByte(void *pvSptHdl, CfaAsfLenTypeFlag_T rCfaAsfLenTypeFlag,
			 const u8 *pu1DataBuf,	/*/<[IN] point to stored data buffer.*/

			 CfaAsfInst_T *prCfaAsf, /*/<[IN] pointer to CfaAsfInst*/
			 u32 *pu4SkipByte
			)
{
	u32 u4SkipByte = 0;
	u32 u4PaddingLen = 0;
	u32 u4PacketSCR = 0;

	if (NULL == pu4SkipByte)
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] Line %d, pu4SkipByte is NULL.\r\n"),DMX_LINE_NO);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	/*point to Field Packet Length.*/
	pu1DataBuf++;

	switch (rCfaAsfLenTypeFlag.ePacketLenType) {
	case Field_Type_UNEXIST:
		u4SkipByte = 0;
		prCfaAsf->u4PacketLen = prCfaAsf->rCfaAsfFileInfo.u4DataPacketSize;
		break;

	case Field_Type_BYTE:
		u4SkipByte++;
		prCfaAsf->u4PacketLen = *pu1DataBuf;
		prCfaAsf->u4PacketLen &= (u32)0x000000FF;
		break;

	case Field_Type_WORD:
		LOADL_WORD(pu1DataBuf, prCfaAsf->u4PacketLen);
		prCfaAsf->u4PacketLen &= (u32)0x0000FFFF;
		u4SkipByte += (u32)2;
		break;

	case Field_Type_DWORD:
		LOADL_DWRD(pu1DataBuf, prCfaAsf->u4PacketLen);
		u4SkipByte += (u32)4;
		break;

	default:
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] Invalid  ePacketLenType in CfaAsfGetSkipByte().\r\n"));
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	prCfaAsf->u4SkipLen = prCfaAsf->u4PacketLen;
	prCfaAsf->u4PacketLen = prCfaAsf->rCfaAsfFileInfo.u4DataPacketSize;

	switch (rCfaAsfLenTypeFlag.eSequenceType) {
	case Field_Type_UNEXIST:
		break;

	case Field_Type_BYTE:
		u4SkipByte++;
		break;

	case Field_Type_WORD:
		u4SkipByte += (u32)2;
		break;

	case Field_Type_DWORD:
		u4SkipByte += (u32)4;
		break;

	default:
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] Invalid  eSequenceType in CfaAsfGetSkipByte().\r\n"));
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	/*point to Field Padding Length.*/
	pu1DataBuf += u4SkipByte;

	switch (rCfaAsfLenTypeFlag.ePadLenType) {
	case Field_Type_UNEXIST:
		u4PaddingLen = 0;
		break;

	case Field_Type_BYTE:
		u4PaddingLen = *pu1DataBuf;
		u4PaddingLen &= (u32)0x000000FF;
		u4SkipByte++;
		pu1DataBuf++;
		break;

	case Field_Type_WORD:
		LOADL_WORD(pu1DataBuf, u4PaddingLen);
		u4PaddingLen &= (u32)0x0000FFFF;
		u4SkipByte += (u32)2;
		pu1DataBuf += 2;
		break;

	case Field_Type_DWORD:
		LOADL_DWRD(pu1DataBuf, u4PaddingLen);
		u4PaddingLen &= (u32)0xFFFFFFFF;
		u4SkipByte += (u32)4;
		pu1DataBuf += 4;
		break;

	default:
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] Invalid  ePadLenType in CfaAsfGetSkipByte().\r\n"));
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	prCfaAsf->u4PaddingLen = u4PaddingLen;

	/*point to Field Send Time (SCR).*/
	LOADL_DWRD(pu1DataBuf, u4PacketSCR);
	Spt4CfaSCRNotify(pvSptHdl, u4PacketSCR * CFA_ASF_SYS_CLK);
	prCfaAsf->u4PacketSCR = u4PacketSCR;

	/*Payload parsing information last fileld Send Time and Duration*/
	/*these fieldS have fixed size, u32 and u16, total 6bytes.*/
	u4SkipByte = u4SkipByte + 6;
	*pu4SkipByte = u4SkipByte;

	MM_RETURN(RET_DMX_OK);
}

/*//Description: Set picture tx mode in terms of video codec type*/
/*//@Return: cfa picture tx mode. 04/10/2008*/
static CfaApiPicTxMode CfaAsfSetPicTxMode(const CfaAsfInst_T *prCfaAsf)
{
	u32 u4Offset = (u32)8;

	switch (prCfaAsf->eVidCodecType) {
	case CFA_VID_DIVX3:
		if (TRUE == prCfaAsf->fgExistCompressData) {
			if (((u8)DIVX311_PVOP) == (prCfaAsf->u1FirstBytePay >> 6))
				return CFA_PTM_ONE_PIC_DX3_P;

			return CFA_PTM_ONE_PIC_DX3_I;
		}
		if (TRUE == prCfaAsf->fgKeyFrame)
			return CFA_PTM_ONE_PIC_DX3_I;

		return CFA_PTM_ONE_PIC_DX3_P;

	case CFA_VID_WMV7:
		if (WMV_PVOP == (prCfaAsf->u1FirstBytePay >> 6))
			return CFA_PTM_WMV_P;

		return CFA_PTM_WMV_I;

	case CFA_VID_WMV8:
		if (WMV_PVOP == (prCfaAsf->u1FirstBytePay >> 7)) {
			return CFA_PTM_WMV_P;

			return CFA_PTM_WMV_I;

		case CFA_VID_VP6A:
			if (prCfaAsf->pu1HdrBuf != NULL) {
				u8 u1FrmType = 0;

				u1FrmType = (prCfaAsf->pu1HdrBuf[3]); /* 3 JUMP ALPH AREA {*/

				if ((u1FrmType & (u8)0x80) == (u8)0)
					return CFA_PTM_ONE_PIC_VP6_I;
				else
					return CFA_PTM_ONE_PIC_VP6_P;
			}
		}

		break;

	case CFA_VID_VP6: {
		if ((prCfaAsf->u1FirstBytePay & (u8)0x80) == 0)
			return CFA_PTM_ONE_PIC_VP6_I;
		else
			return CFA_PTM_ONE_PIC_VP6_P;
	}
	break;

	case CFA_VID_VP8: {
		if ((prCfaAsf->u1FirstBytePay & (u8)0x80) == 0)
			return CFA_PTM_ONE_PIC_VP8_I;
		else
			return CFA_PTM_ONE_PIC_VP8_P;
	}
	break;

	case CFA_VID_WMV9:
		if (prCfaAsf->fgPrsSeqFrameInterpolation)
			u4Offset--;

		u4Offset -= (u32)2;

		if (prCfaAsf->fgPrsPreProcRange)
			u4Offset--;

		if ((u32)(prCfaAsf->u1FirstBytePay) & ((u32)1 << (--u4Offset)))
			return CFA_PTM_WMV_P;

		if (prCfaAsf->u4PrsNumBFrames == 0)
			return CFA_PTM_WMV_I;

		if ((u32)(prCfaAsf->u1FirstBytePay) & ((u32)1 << (--u4Offset)))
			return CFA_PTM_WMV_I;
		else
			return CFA_PTM_WMV_B;
		break;


	case CFA_VID_VC1:
		/*do something here later !!*/
		break;

	default:
		break;
	}

	return CFA_PTM_EXACT_POS;
}


/*//Description: TH added for VC1 codec payload no exist start code of WMV file.*/
/*///					Time = 2008-05-20*/
/*//@return none.*/
static void CfaAsfTxVc1Sc(void *pvSptHdl, CfaAsfInst_T *prCfaAsf)
{
	MRESULT mrRet = RET_DMX_OK;

	/*need tx vc1 start code before tx video data to vfifo every time.*/
	mrRet = Spt4CfaBuf2VFifo(pvSptHdl, prCfaAsf->pu1CfaAsfVc1Sc, 0,
				 CFA_PTM_EXACT_POS, prCfaAsf->eVidCodecType, 4);
	prCfaAsf->eLastCfaAsfAnaSt = prCfaAsf->eCurCfaAsfAnaSt;
	prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_TX_VC1_SC_DONE;

	if (RET_DMX_OK != mrRet) {
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,
				TEXT("[ASF CFA] call Spt4CfaBuf2VFifo() retun err(%d) in ")
				 TEXT("CfaAsfTxVc1Sc(),call FinishPrs: %x.\n"), mrRet, 0x06);
		CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
	}

}


/*//Description: Transfer video data from playback buffer to VFifo.*/
/**/
/*//@Return none.*/
/*/<[IN] start address of  transfering data*/
/*/<[IN]transfer actual audio / video data length.*/
/*/<[IN] handle of splitter.*/
/*/<[IN] pointer to CfaAsfInst*/
static MRESULT CfaAsfTxVidData2VFifo(u64 u8Sa, u64 u8TxLen, void *pvSptHdl,
					 CfaAsfInst_T *prCfaAsf, bool fgNeedTxVcSc)
{
	CFA_VIDEO_INFO_T rCfaAsfTxVidInfo = {0};
	MRESULT mrRet = RET_DMX_OK;

	if (TRUE == prCfaAsf->fgFirstTxVid) {
		if ((TRUE != (prCfaAsf->fgKeyFrame)) ||
			((prCfaAsf->fgKeyFrame) && (0 != (prCfaAsf->u4OftInMeidaObj)) &&
			(CFA_VID_MPEG2 != (prCfaAsf->eVidCodecType)))) {
			/* MPEG2 may need last video payload for GOP header */
			CfaAsfSearchHeader(pvSptHdl, prCfaAsf, prCfaAsf->eCurCfaAsfAnaSt, 0, CFA_ASF_HDR_BUF_SZ, 0);
			MM_RETURN(RET_DMX_OK);
		}
		prCfaAsf->fgFirstTxVid = FALSE;
	}
	/*transfer video data to video fifo.*/
	prCfaAsf->eCurCfaAsfTxStrmType = CFA_ASF_TX_STRM_TYPE_VID;

	rCfaAsfTxVidInfo.u8FileOfst = u8Sa;
	rCfaAsfTxVidInfo.eVidType = prCfaAsf->eVidCodecType;

	if (CFA_VID_VC1 == prCfaAsf->eVidCodecType) {
#if CFA_ASF_SYNC_BUF_BEFORE_TX

		if (prCfaAsf->rCfaAsfDrmInfo.au1StreamEncrypted[(prCfaAsf->u1CurStrmId - (u8)1) / (u8)8] & ((u8) 1 <<
				((prCfaAsf->u1CurStrmId - (u8)1) % (u8)8))) {
			/* For DRM file, CFA has to turn on CPS and sync buf before tx. so check VC1 SC here*/
			LOADL_DWRD(prCfaAsf->pu1HdrBuf, prCfaAsf->u4Vc1StartCode);
			prCfaAsf->fgExsitVc1StratCode = CfaAsfExsitVc1StartCode(prCfaAsf->u4Vc1StartCode);
		}

#endif

		if ((FALSE == prCfaAsf->fgExsitVc1StratCode) &&
			(0 == prCfaAsf->u4OftInMeidaObj)) {
			prCfaAsf->rCfaAsfLastTxInfo.u8LastTxSa = u8Sa;
			prCfaAsf->rCfaAsfLastTxInfo.u8LastTxLen = u8TxLen;

			if (fgNeedTxVcSc == TRUE) {
				CfaAsfTxVc1Sc(pvSptHdl, prCfaAsf);
				MM_RETURN(RET_DMX_OK);
			}
		}
	}

#if CFA_ASF_SYNC_BUF_BEFORE_TX

	if (prCfaAsf->rCfaAsfDrmInfo.au1StreamEncrypted[(prCfaAsf->u1CurStrmId - (u8)1) / (u8)8] & ((u8) 1 <<
			((prCfaAsf->u1CurStrmId - (u8)1) % (u8)8))) {
		/* For DRM file, CFA has to turn on CPS and sync buf before tx. so get picture type here*/
		prCfaAsf->u1FirstBytePay = *prCfaAsf->pu1HdrBuf;
	}

#endif
	rCfaAsfTxVidInfo.eTxMode = CfaAsfSetPicTxMode(prCfaAsf);
	rCfaAsfTxVidInfo.u4PrsStrmId = (u32) prCfaAsf->u1CurStrmId;
	rCfaAsfTxVidInfo.u8Len = u8TxLen;

	if (u8TxLen + prCfaAsf->u4OftInMeidaObj > prCfaAsf->u4MediaObjSize) {
		/* if payload length is invalid*/
		DmxLogD(DMX_MOD_CFA_ASF,CFA_ASF_LOG_TXDONE,TEXT("Invalid TX length! %lld, %ld, %ld\n"),
				u8TxLen, prCfaAsf->u4OftInMeidaObj, prCfaAsf->u4MediaObjSize);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (0 == prCfaAsf->u4OftInMeidaObj) {
		rCfaAsfTxVidInfo.fgUnitStart = TRUE;
		rCfaAsfTxVidInfo.u8TotalAULen = (u64) prCfaAsf->u4MediaObjSize;
		rCfaAsfTxVidInfo.u8TotalAULen &= 0x00000000FFFFFFFF;
		prCfaAsf->rCfaAsfVidInfo.u8AUTotalLen = rCfaAsfTxVidInfo.u8TotalAULen;
		prCfaAsf->rCfaAsfVidInfo.u8SentLen = 0;

		if (TRUE == prCfaAsf->fgKeyFrame) {
			u64 u8RealPTS = (prCfaAsf->u8PrsPts > prCfaAsf->rCfaAsfFileInfo.u8PrerollTime) ?
					   (prCfaAsf->u8PrsPts - prCfaAsf->rCfaAsfFileInfo.u8PrerollTime) : 0;
			prCfaAsf->u8LastKeyFramePts = u8RealPTS;
			prCfaAsf->u8LastKeyFramePacketId = prCfaAsf->u8KeyPacketId;
		}

		if (prCfaAsf->u4MediaObjSize <= 1) {/*mcn08025 fix bug @2008-11-14 */
			if (CFA_VID_MPEG4 == prCfaAsf->eVidCodecType) {
				rCfaAsfTxVidInfo.eTxMode = CFA_PTM_DUMMY;
				rCfaAsfTxVidInfo.fgDummyAU = TRUE;
				rCfaAsfTxVidInfo.fgDummyCmdAU = FALSE;
			} else if ((CFA_VID_WMV7 == (prCfaAsf->eVidCodecType))
				   || (CFA_VID_WMV8 == (prCfaAsf->eVidCodecType))
				   || (CFA_VID_WMV9 == (prCfaAsf->eVidCodecType))
				   || (CFA_VID_VC1 == (prCfaAsf->eVidCodecType)))
				rCfaAsfTxVidInfo.eTxMode = CFA_PTM_WMV_SKIPFRAME;
			else
			{
		/*do nothing*/
		}
		}
	} else {
		rCfaAsfTxVidInfo.fgUnitStart = FALSE;
		rCfaAsfTxVidInfo.u8TotalAULen = 0;
	}
	if ((u8TxLen + prCfaAsf->rCfaAsfVidInfo.u8SentLen) > prCfaAsf->rCfaAsfVidInfo.u8AUTotalLen) {
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[ASF CFA] invalid packet, u8AUTotalLen:0x%llx, u8TxLen:0x%llx")
				TEXT( "already sent len:0x%llx.\r\n"),
				prCfaAsf->rCfaAsfVidInfo.u8AUTotalLen, u8TxLen, prCfaAsf->rCfaAsfVidInfo.u8SentLen);
		Spt4CfaFinishedEx(pvSptHdl, rCfaAsfTxVidInfo.u8FileOfst, TRUE, GAU_E_ERRCHUNK);
		MM_RETURN(RET_DMX_OK);
	}

	mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rCfaAsfTxVidInfo);
	prCfaAsf->fgNoSyncPbb = FALSE;
	if (RET_DMX_OK != mrRet) {
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[ASF CFA] call Spt4CfaPbb2VFifoAUCtrl() retun err(%d)")
			TEXT("in CfaAsfTxVidData2VFifo(),call FinishPrs: %x.\n"), mrRet, 0x07);
		CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
		MM_RETURN(mrRet);
	} else {
		prCfaAsf->rCfaAsfVidInfo.u8SentLen += u8TxLen;
	}

	MM_RETURN(RET_DMX_OK);
}

/*//Description: Transfer audio data from playback buffer to AFifo*/
/**/
/*//@Return none.*/
/*/<[IN] start address of  transfering data*/
/*/<[IN]transfer actual audio / video data length.*/
/*/<[IN] handle of splitter.*/
/*/<[IN] pointer to CfaAsfInst*/
static void CfaAsfTxAudData2AFifo(u64 u8Sa, u64 u8TxLen, void *pvSptHdl,
				  CfaAsfInst_T *prCfaAsf)
{
	CFA_AUDIO_INFO_T rCfaAsfTxAudInfo = {0};
	u64 u8CfaAudioAULen = 0;
	MRESULT mrRet = RET_DMX_OK;

	if (TRUE == prCfaAsf->fgFirstTxAud) {
		if (0 != prCfaAsf->u4OftInMeidaObj)
		{
			CfaAsfSearchHeader(pvSptHdl, prCfaAsf, prCfaAsf->eCurCfaAsfAnaSt, 0, CFA_ASF_HDR_BUF_SZ, 0);
			return;
		}


		if (prCfaAsf->fgSetLpcm == TRUE) {
			prCfaAsf->fgSetLpcm = FALSE;
			Spt4CfaSetLpcmEmphasis(pvSptHdl, TRUE);
		}

		prCfaAsf->fgFirstTxAud = FALSE;
#if ASF_AAC_SUPPORT

		if (AVCODEC_ID_AAC == prCfaAsf->raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx].eCodecID) {
			if (prCfaAsf->raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx].u4AudCodecSpecDataLen) {
				prCfaAsf->rCfaAsfLastTxInfo.u8LastTxSa = u8Sa;
				prCfaAsf->rCfaAsfLastTxInfo.u8LastTxLen = u8TxLen;
				prCfaAsf->eLastCfaAsfAnaSt = prCfaAsf->eCurCfaAsfAnaSt;
				prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_TX_A_HDR;

				mrRet = Spt4CfaBuf2AFifo(pvSptHdl, prCfaAsf->raCfaAsfAudInfo[prCfaAsf->
					u1CurAudInfoIdx].au1AudCodecSpecData,
					prCfaAsf->raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx].u4AudCodecSpecDataLen,
					prCfaAsf->raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx].rCfaAsfStrmInfo.u1StrmNum,
					CfaAsfGetAudType((u32)
					prCfaAsf->raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx].eCodecID));
					prCfaAsf->fgNoSyncPbb = FALSE;

				if (RET_DMX_OK != mrRet) {
					DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,
						TEXT("[ASF CFA] call Spt4CfaBuf2AFifo() retun err(%d)")
						TEXT("in CfaAsfTxAudData2AFifo(),call FinishPrs: %x.\n"), mrRet, 0x08);
					CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
				}
				return;
			}
		}

#endif
	}

	/*transfer audio data to audio fifo.*/
	prCfaAsf->eCurCfaAsfTxStrmType = CFA_ASF_TX_STRM_TYPE_AUD;

	rCfaAsfTxAudInfo.u8FileOfst = u8Sa;
	rCfaAsfTxAudInfo.u8Len = u8TxLen;
	rCfaAsfTxAudInfo.u8Pts = (prCfaAsf->u8PrsPts - prCfaAsf->rCfaAsfFileInfo.u8PrerollTime) *
				 CFA_ASF_SYS_CLK; /*change unit in Hz, STC Clock*/
	rCfaAsfTxAudInfo.u4PrsStrmId = (u32) prCfaAsf->u1CurStrmId;

	if (0 == prCfaAsf->u4OftInMeidaObj) {
#if ASF_AAC_SUPPORT

		if (prCfaAsf->raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx].fgNeedAdtsHeader) {
			prCfaAsf->rCfaAsfLastTxInfo.u8LastTxSa = u8Sa;
			prCfaAsf->rCfaAsfLastTxInfo.u8LastTxLen = u8TxLen;

			if (FALSE == prCfaAsf->fgHasAdts) {
				SetAacADTSHdr(prCfaAsf);
				prCfaAsf->eLastCfaAsfAnaSt = prCfaAsf->eCurCfaAsfAnaSt;
				prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_TX_AAC_ADTS;
				mrRet = Spt4CfaBuf2AFifo
					(pvSptHdl, prCfaAsf->raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx].auheader,
					7, prCfaAsf->raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx]
					.rCfaAsfStrmInfo.u1StrmNum,
					CfaAsfGetAudType((u32)
					prCfaAsf->raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx].eCodecID));
					prCfaAsf->fgNoSyncPbb = FALSE;

				if (RET_DMX_OK != mrRet) {
					DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,
						TEXT("[ASF CFA] call Spt4CfaBuf2AFifo() retun err(%d)")
						TEXT("in CfaAsfTxAudData2AFifo(),call FinishPrs: %x.\n"), mrRet, 0x09);
					CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
				}

				return;
			}
		}

#endif

		if (DMX_IS_RW_PLAY(pvSptHdl) && (0 == (prCfaAsf->u4CurPrsFlag & CFA_ASF_PRS_BIT_STRM_TYPE_VID))) {
			u8CfaAudioAULen = prCfaAsf->rCfaAsfRange.u4CfaAudioAULen;

			if (prCfaAsf->rCfaAsfRange.fgVirFinish) {
				u64 u8CurRestLen = prCfaAsf->rCfaAsfRange.u8AudEa - prCfaAsf->u8Ca - u8TxLen;

				if (u8CurRestLen <= prCfaAsf->rCfaAsfRange.u8AudMaxRestLen) {
					CFA_AUDIO_INFO_T rCfaAsfTxAudInfo1 = {0};

					prCfaAsf->rCfaAsfRange.fgVirFinish = FALSE;
					rCfaAsfTxAudInfo1.fgAUCompleteByEnd = TRUE;
					rCfaAsfTxAudInfo1.fgUnitEnd = TRUE;
					rCfaAsfTxAudInfo1.fgUnitStart = TRUE;
					rCfaAsfTxAudInfo1.u8Pts =
						(prCfaAsf->u8PrsPts - prCfaAsf->rCfaAsfFileInfo.u8PrerollTime) *
						CFA_ASF_SYS_CLK;
					rCfaAsfTxAudInfo1.u4PrsStrmId = (u32)
						prCfaAsf->raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx]
						.rCfaAsfStrmInfo.u1StrmNum;
					rCfaAsfTxAudInfo1.u8FileOfst = prCfaAsf->u8Ca;
					rCfaAsfTxAudInfo1.u8Len = 0;
					rCfaAsfTxAudInfo1.eAudType = CfaAsfGetAudType((u32)
						prCfaAsf->raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx].eCodecID);
					rCfaAsfTxAudInfo1.u8TotalAULen = 0;
					MMATE_CHECK_STRUCT(prCfaAsf->rCfaAsfRange);
					mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rCfaAsfTxAudInfo1);
					prCfaAsf->fgNoSyncPbb = FALSE;

					if (DMX_FAILED(mrRet)) {
						DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,
							TEXT("[CFA_ASF] %s failed in Spt4CfaPbb2AFifoAUCtrl,")
							TEXT("mrRet: 0x%x\r\n"), DMX_FUNC_NAME, mrRet);
						CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
						return;
					}

					prCfaAsf->u4FRAudDataTxLen = 0;
					prCfaAsf->fgFRCurSmpFinish = TRUE;
					return;
				}
			}

			if (prCfaAsf->fgFRCurSmpFinish) {
				prCfaAsf->fgFRCurSmpFinish = FALSE;
				CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
				return;
				}

			if (0 == prCfaAsf->u4FRAudDataTxLen)
				rCfaAsfTxAudInfo.fgUnitStart = TRUE;
			else
				rCfaAsfTxAudInfo.fgUnitStart = FALSE;

			prCfaAsf->fgHasAdts = FALSE;

			if (prCfaAsf->u1CurStrmId ==
				prCfaAsf->raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx].rCfaAsfStrmInfo.u1StrmNum) {
				prCfaAsf->u4FRAudDataTxLen += (u32) u8TxLen;

				if (prCfaAsf->u4FRAudDataTxLen >= u8CfaAudioAULen) {
					rCfaAsfTxAudInfo.u8Len = u8TxLen -
						(prCfaAsf->u4FRAudDataTxLen - u8CfaAudioAULen);
					prCfaAsf->u4FRAudDataTxLen = 0;
					prCfaAsf->fgFRCurSmpFinish = TRUE;
				}

				rCfaAsfTxAudInfo.eAudType = CfaAsfGetAudType((u32)
					prCfaAsf->raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx].eCodecID);
				rCfaAsfTxAudInfo.u8TotalAULen = u8CfaAudioAULen;
				mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rCfaAsfTxAudInfo);
				prCfaAsf->fgNoSyncPbb = FALSE;
			} else
				CfaAsfSearchHeader(pvSptHdl, prCfaAsf, prCfaAsf->eCurCfaAsfAnaSt, 0, CFA_ASF_HDR_BUF_SZ, 0);
			} else {
			rCfaAsfTxAudInfo.fgUnitStart = TRUE;
			prCfaAsf->fgHasAdts = FALSE;
			rCfaAsfTxAudInfo.eAudType = CfaAsfGetAudType((u32)
			prCfaAsf->raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx].eCodecID);
			rCfaAsfTxAudInfo.u8TotalAULen = prCfaAsf->u4MediaObjSize;
			mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rCfaAsfTxAudInfo);
			prCfaAsf->fgNoSyncPbb = FALSE;
		}

		if (RET_DMX_OK != mrRet) {
			DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[ASF CFA] call Spt4CfaPbb2AFifoAUCtrl() retun")
				TEXT("err(%d) in CfaAsfTxAudData2AFifo(),call FinishPrs: %x.\n"), mrRet, 0x0a);
			CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
			return;
		}
	} else {
		rCfaAsfTxAudInfo.fgUnitStart = FALSE;
		rCfaAsfTxAudInfo.eAudType =
			CfaAsfGetAudType((u32) prCfaAsf->raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx].eCodecID);
		rCfaAsfTxAudInfo.u8TotalAULen = 0;
		mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rCfaAsfTxAudInfo);
		prCfaAsf->fgNoSyncPbb = FALSE;

		if (RET_DMX_OK != mrRet) {
			DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[ASF CFA] call Spt4CfaPbb2AFifoAUCtrl() retun")
				TEXT("err(%d) in CfaAsfTxAudData2AFifo(),call FinishPrs: %x.\n"), mrRet, 0x0b);
			CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
			return;
		}
	}
	
	/*mtk40504 add @for pure audio file*/
	if (CFA_VID_UNKNOWN == prCfaAsf->eVidCodecType)
		Sleep(1);
}



/*//Description:Transfer actual audio or video payload data to corresponding FIFO.*/
/*//@return none.*/
static MRESULT CfaAsfTxStrmData2Fifo
(
	u64 u8Sa,			/*/<[IN] start address of  transfering data*/
	u64 u8TxLen,/*/<[IN]transfer actual audio / video data length.*/
	void *pvSptHdl,			 /*/<[IN] handle of splitter.*/

	CfaAsfInst_T *prCfaAsf		/*/<[IN] pointer to CfaAsfInst*/
)
{
	bool fgNeedTxVcSc = TRUE;

	if (prCfaAsf->eCurCfaAsfAnaSt == CFA_ASF_ANA_ST_TX_VC1_SC_DONE) {
		fgNeedTxVcSc = FALSE;
		prCfaAsf->eCurCfaAsfAnaSt = prCfaAsf->eLastCfaAsfAnaSt;
	} else if (prCfaAsf->eCurCfaAsfAnaSt == CFA_ASF_ANA_ST_TX_PAYLOAD_DATA)
			prCfaAsf->eCurCfaAsfAnaSt = prCfaAsf->eLastCfaAsfAnaSt;
	else if (prCfaAsf->eCurCfaAsfAnaSt == CFA_ASF_ANA_ST_TX_A_HDR)
		prCfaAsf->eCurCfaAsfAnaSt = prCfaAsf->eLastCfaAsfAnaSt;
	else if (prCfaAsf->eCurCfaAsfAnaSt == CFA_ASF_ANA_ST_TX_AAC_ADTS) {
		prCfaAsf->fgHasAdts = TRUE;
		prCfaAsf->eCurCfaAsfAnaSt = prCfaAsf->eLastCfaAsfAnaSt;
	}
	else
	{
		/*do nothing*/
	}

	if ((prCfaAsf->u4SkipLen !=  prCfaAsf->rCfaAsfFileInfo.u4DataPacketSize) &&
		(FALSE == prCfaAsf->fgExistMultiPayload)) {
		if (prCfaAsf->u4SkipLen > prCfaAsf->rCfaAsfFileInfo.u4DataPacketSize)
		{
			DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[CFA ASF] Line %d, Skip length error\n"),DMX_LINE_NO);
			CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
			MM_RETURN(RET_DMX_OK);
		}

		if (prCfaAsf->fgErrSingleCompressed)
			prCfaAsf->fgErrSingleCompressed = FALSE;
		else
			u8TxLen -= (prCfaAsf->rCfaAsfFileInfo.u4DataPacketSize - prCfaAsf->u4SkipLen);
	}

	/*Check parsed payload data is over file end.*/
	if ((u8Sa + u8TxLen) > prCfaAsf->rCfaAsfFileInfo.u8FileSize) {
		DmxLogD(DMX_MOD_CFA_ASF,CFA_ASF_LOG_TXDONE,TEXT("[CFA ASF] Finished parsing!\n"));
		CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
		MM_RETURN(RET_DMX_OK);
	}

#if CFA_ASF_SUPPORT_VOD_DRM

	if ((0 == (prCfaAsf->u1CurStrmId)) || ((prCfaAsf->u1CurStrmId) >= 128))/*fix bug BDP00119776 */
		MM_RETURN(RET_DMX_UNEXPECT);

#if !CFA_ASF_SYNC_BUF_BEFORE_TX

	if (prCfaAsf->rCfaAsfDrmInfo.au1StreamEncrypted[(prCfaAsf->u1CurStrmId - 1) / 8]
	& ((u8) 1 << ((prCfaAsf->u1CurStrmId - 1) % 8))) {
		MRESULT mrRet = RET_DMX_OK;

		prCfaAsf->fgDrmEncrypt = TRUE;
		prCfaAsf->rCfaAsfCpsInfo.fgOn = TRUE;

		if (2 == prCfaAsf->rCfaAsfDrmInfo.u1WMDRMType) {
			prCfaAsf->rCfaAsfCpsInfo.u4CpsType = CFA_CPS_TYPE_WMDRM_ND;
			/*CFA_CPS_TYPE_WMDRM_PD/CFA_CPS_TYPE_WMDRM_ND*/
		} else {
			prCfaAsf->rCfaAsfCpsInfo.u4CpsType = CFA_CPS_TYPE_WMDRM_PD;
			/*/CFA_CPS_TYPE_WMDRM_PD/CFA_CPS_TYPE_WMDRM_ND*/
		}

		prCfaAsf->rCfaAsfCpsInfo.u8Offset = u8Sa;
		prCfaAsf->rCfaAsfCpsInfo.u4MaxPacketSize = prCfaAsf->rCfaAsfFileInfo.u4DataPacketSize;
		dmx_memcpy((void *) prCfaAsf->rCfaAsfCpsInfo.bSampleID, (void *)(&prCfaAsf->u8ExtSampleId), 8);

		mrRet = Spt4CfaTurnCPS(pvSptHdl, &prCfaAsf->rCfaAsfCpsInfo);

		if (RET_DMX_OK != mrRet) {
			DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[ASF CFA] call Spt4CfaTurnCPS() retun")
				TEXT("err(%d) in CfaAsfSyncBuf(),call FinishPrs: %x.\n"), mrRet, 0x05);
			CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
			MM_RETURN(mrRet);
		}
	}

#endif
#endif

	if (prCfaAsf->u1CurAudInfoIdx >= MAX_ASF_AUD_STRM_NUM)
		MM_RETURN(RET_DMX_UNEXPECT);

	/* todo:
		 For netflix, need check stream type(updated by IBC) and stream id
	*/
#if CFA_ASF_NRD_SUPPORT

	if (prCfaAsf->rCfaAsfFileInfo.fgIsNrd) {
		/* for NRD stream. Need check current NRD stream type and then check stream id*/
		if (CFA_NrdDataType_PacketMux == prCfaAsf->rCfaAsfNfInfo.eNrdType) {/* mux stream*/
			if ((prCfaAsf->u1CurStrmId == prCfaAsf->rCfaAsfVidInfo.rCfaAsfStrmInfo.u1StrmNum) &&
				(prCfaAsf->u4CurPrsFlag & CFA_ASF_PRS_BIT_STRM_TYPE_VID) &&
				TRUE == prCfaAsf->fgEnableVid) {
				MRESULT mrRet = RET_DMX_OK;

				mrRet = CfaAsfTxVidData2VFifo(u8Sa, u8TxLen, pvSptHdl, prCfaAsf, fgNeedTxVcSc);

				if (mrRet != RET_DMX_OK)
					MM_RETURN(mrRet);
				}
			/* If it is not video, all tx to audio fifo*/
			else if (prCfaAsf->rCfaAsfFileInfo.fgCfaRespliter &&
				 (prCfaAsf->u4CurPrsFlag & CFA_ASF_PRS_BIT_STRM_TYPE_AUD) &&
				 TRUE == prCfaAsf->fgEnableAud && prCfaAsf->u1CurStrmId !=
				 prCfaAsf->rCfaAsfVidInfo.rCfaAsfStrmInfo.u1StrmNum)
				CfaAsfTxAudData2AFifo(u8Sa, u8TxLen, pvSptHdl, prCfaAsf);
			else if (!prCfaAsf->rCfaAsfFileInfo.fgCfaRespliter &&
				 prCfaAsf->fgEnableAud && (prCfaAsf->u4CurPrsFlag &
							   CFA_ASF_PRS_BIT_STRM_TYPE_AUD) &&
				 prCfaAsf->u1CurStrmId == prCfaAsf->
				 raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx].rCfaAsfStrmInfo.u1StrmNum)
				CfaAsfTxAudData2AFifo(u8Sa, u8TxLen, pvSptHdl, prCfaAsf);
			else
				CfaAsfSearchHeader(pvSptHdl, prCfaAsf, prCfaAsf->eCurCfaAsfAnaSt, 0, CFA_ASF_HDR_BUF_SZ, 0);
		} else if (CFA_NrdDataType_PacketVideo == prCfaAsf->rCfaAsfNfInfo.eNrdType) {

			/* NRD video stream*/
			if ((prCfaAsf->u1CurStrmId == prCfaAsf->rCfaAsfVidInfo.rCfaAsfStrmInfo.u1StrmNum) &&
				(prCfaAsf->u4CurPrsFlag & CFA_ASF_PRS_BIT_STRM_TYPE_VID) &&
				TRUE == prCfaAsf->fgEnableVid) {
				MRESULT mrRet = RET_DMX_OK;

				mrRet = CfaAsfTxVidData2VFifo(u8Sa, u8TxLen, pvSptHdl, prCfaAsf, fgNeedTxVcSc);

				if (mrRet != RET_DMX_OK)
					MM_RETURN(mrRet);
				} else
				CfaAsfSearchHeader(pvSptHdl, prCfaAsf, prCfaAsf->eCurCfaAsfAnaSt, 0, CFA_ASF_HDR_BUF_SZ, 0);
			} else {
			/* NRD audio stream*/
			if (prCfaAsf->fgEnableAud && (prCfaAsf->u4CurPrsFlag & CFA_ASF_PRS_BIT_STRM_TYPE_AUD)
				&& prCfaAsf->u1CurStrmId == prCfaAsf->raCfaAsfAudInfo[0].rCfaAsfStrmInfo.u1StrmNum)
				CfaAsfTxAudData2AFifo(u8Sa, u8TxLen, pvSptHdl, prCfaAsf);
			else
				CfaAsfSearchHeader(pvSptHdl, prCfaAsf, prCfaAsf->eCurCfaAsfAnaSt, 0, CFA_ASF_HDR_BUF_SZ, 0);
		}
	} else
#endif
	{

		if (((prCfaAsf->u1CurStrmId) == (prCfaAsf->rCfaAsfVidInfo.rCfaAsfStrmInfo.u1StrmNum)) &&
		((prCfaAsf->u4CurPrsFlag) & CFA_ASF_PRS_BIT_STRM_TYPE_VID) && (TRUE == (prCfaAsf->fgEnableVid))) {
			MRESULT mrRet = RET_DMX_OK;

			mrRet = CfaAsfTxVidData2VFifo(u8Sa, u8TxLen, pvSptHdl, prCfaAsf, fgNeedTxVcSc);

			if (mrRet != RET_DMX_OK)
				MM_RETURN(mrRet);
		} else if ((prCfaAsf->rCfaAsfFileInfo.fgCfaRespliter) &&
			((prCfaAsf->u4CurPrsFlag) & CFA_ASF_PRS_BIT_STRM_TYPE_AUD) &&
			(TRUE == (prCfaAsf->fgEnableAud)) &&
			((prCfaAsf->u1CurStrmId) != (prCfaAsf->rCfaAsfVidInfo.rCfaAsfStrmInfo.u1StrmNum)))
			CfaAsfTxAudData2AFifo(u8Sa, u8TxLen, pvSptHdl, prCfaAsf);
		else if ((!prCfaAsf->rCfaAsfFileInfo.fgCfaRespliter) &&
			(prCfaAsf->fgEnableAud) && ((prCfaAsf->u4CurPrsFlag) & CFA_ASF_PRS_BIT_STRM_TYPE_AUD) &&
			((prCfaAsf->u1CurStrmId) ==
			(prCfaAsf->raCfaAsfAudInfo[prCfaAsf->u1CurAudInfoIdx].rCfaAsfStrmInfo.u1StrmNum)))
			CfaAsfTxAudData2AFifo(u8Sa, u8TxLen, pvSptHdl, prCfaAsf);
		else {
			CfaAsfSearchHeader(pvSptHdl, prCfaAsf, prCfaAsf->eCurCfaAsfAnaSt,
			0, CFA_ASF_HDR_BUF_SZ, 0);
		}
	}

	MM_RETURN(RET_DMX_OK);
}

/*//Description: Demux audio and video stream payload data.*/
/*//		  Transfer those payloads data to corresponding FIFO.*/
/*//@return none.*/
static MRESULT CfaAsfDemuxStrmData
(
	u64 u8Sa,			/*/<[IN] start address of	transfering data*/
	u64 u8TxLen,			/*/<[IN]transfer actual audio / video data length.*/
	void *pvSptHdl,			 /*/<[IN] handle of splitter.*/

	CfaAsfInst_T *prCfaAsf		/*/<[IN] pointer to CfaAsfInst.*/
)
{
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prCfaAsf)
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[CFA ASF] Line %d, prCfaAsf is NULL.\r\n"),DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	MMATE_CHECK_STRUCT(prCfaAsf->rCfaAsfRange);

	switch (prCfaAsf->rCfaAsfRange.eSkipMode) {
	case CFA_ASF_SKIP_NONE:
		/*CFA ASF normally parsing whole file. */
#if CFA_ASF_SYNC_BUF_BEFORE_TX

	/*mtk40145 add here. When it is DRM file, CFA need to turn on CPS and sync buf before tx payload data*/
	if (prCfaAsf->rCfaAsfDrmInfo.au1StreamEncrypted[(prCfaAsf->u1CurStrmId - (u8)1) / (u8)8] & ((u8) 1 <<
			((prCfaAsf->u1CurStrmId - (u8)1) % (u8)8))) {
		mrRet = CfaAsfSyncBuf(pvSptHdl, prCfaAsf, u8Sa, CFA_ASF_HDR_BUF_SZ, TRUE, u8TxLen);
		prCfaAsf->eLastCfaAsfAnaSt = prCfaAsf->eCurCfaAsfAnaSt;
		prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_TX_PAYLOAD_DATA;
		prCfaAsf->rCfaAsfLastTxInfo.u8LastTxSa = u8Sa;
		prCfaAsf->rCfaAsfLastTxInfo.u8LastTxLen = u8TxLen;
		} else
#endif
	{
		mrRet = CfaAsfTxStrmData2Fifo(u8Sa, u8TxLen, pvSptHdl, prCfaAsf);
	}

	if (mrRet != RET_DMX_OK)
		MM_RETURN(mrRet);

	break;

	case CFA_ASF_SKIP_BY_PTS:
	/*for Time Search by pts of ASF playback Engine. {//mtk40504*/
	if (((prCfaAsf->u8PrsPts < prCfaAsf->rCfaAsfRange.u8DispPicPTS) &&
	((prCfaAsf->rCfaAsfRange.u8DispPicPTS - prCfaAsf->u8PrsPts) < CFA_ASF_AVSYNC_THRETHOLD)) ||
	(prCfaAsf->u8PrsPts >= prCfaAsf->rCfaAsfRange.u8DispPicPTS) ||
	(prCfaAsf->u1CurStrmId == prCfaAsf->rCfaAsfVidInfo.rCfaAsfStrmInfo.u1StrmNum)) {
		mrRet = CfaAsfTxStrmData2Fifo(u8Sa, u8TxLen, pvSptHdl, prCfaAsf);
		if (RET_DMX_OK != mrRet)
		{
			MM_RETURN(mrRet);
		}
		/*prCfaAsf->rCfaAsfRange.eSkipMode = CFA_ASF_SKIP_NONE;*/
	} else
		CfaAsfSearchHeader(pvSptHdl, prCfaAsf, prCfaAsf->eCurCfaAsfAnaSt, 0, CFA_ASF_HDR_BUF_SZ, 0);

	break;

	case CFA_ASF_SKIP_BY_PACKET:
	/*for Time Search by packet count of ASF playback Engine. */
	if (TRUE == prCfaAsf->fgKeyFrame) {
		mrRet = CfaAsfTxStrmData2Fifo(u8Sa, u8TxLen, pvSptHdl, prCfaAsf);
		if (RET_DMX_OK != mrRet)
		{
			MM_RETURN(mrRet);
		}

		if ((prCfaAsf->u4OftInMeidaObj + prCfaAsf->u4PayloadLen) == prCfaAsf->u4MediaObjSize) {
			prCfaAsf->u8Ca =
				prCfaAsf->rCfaAsfRange.u8VidSa + prCfaAsf->rCfaAsfFileInfo.u4DataPacketSize *
					 prCfaAsf->rCfaAsfRange.u4SkipPacketCount;
			prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_SEARCH_HEADER;
			prCfaAsf->eCurCfaAsfPrsBitStrmType = CFA_ASF_PRS_BIT_STRM_TYPE_PACKET_HDR;
			prCfaAsf->rCfaAsfRange.eSkipMode = CFA_ASF_SKIP_NONE;
			prCfaAsf->u8PacketStartAdr = prCfaAsf->u8Ca;
		}
	} else
		CfaAsfSearchHeader(pvSptHdl, prCfaAsf, prCfaAsf->eCurCfaAsfAnaSt, 0, CFA_ASF_HDR_BUF_SZ, 0);
	break;

	default:
		MM_RETURN(RET_DMX_UNEXPECT);
	}
	MM_RETURN(RET_DMX_OK);
}

/*//Description: Transfer multi sub-payload data to fifo.*/
/**/
/*//@return none.*/
static void CfaAsfTxComSubPayload2Fifo
(
	void *pvSptHdl,		 /*/<[IN] handle of splitter.*/

	CfaAsfInst_T *prCfaAsf		/*/<[IN] pointer to CfaAsfInst.*/
)
{
	u64 u8Sa = 0;
	u64 u8SubPayloadLen = 0;
	u32 u4PrsLen = 0;
	MRESULT mrRet = RET_DMX_OK;

	/*jump to next sub-payload data*/
	u8Sa = prCfaAsf->u8Ca + 1;
	u8SubPayloadLen = *prCfaAsf->pu1HdrBuf;
	prCfaAsf->u4MediaObjSize = (u32) u8SubPayloadLen;
	prCfaAsf->fgExistCompressData = TRUE;

	/*Skip error data*/
	if (u8SubPayloadLen == 0) {
		CfaAsfSkipErr2NextPacket(pvSptHdl, prCfaAsf);
		return;
	}

#if CFA_ASF_SYNC_BUF_BEFORE_TX

	/* keep previous flow (don't sync buf) if it is not a DRM file*/
	if (!(prCfaAsf->rCfaAsfDrmInfo.au1StreamEncrypted[(prCfaAsf->u1CurStrmId - (u8)1) / (u8)8] &
		  ((u8) 1 << ((prCfaAsf->u1CurStrmId - (u8)1) % (u8)8)))) {
#else
	{
#endif
		/*For multi sub-payload data whether need to check tx mode??*/
		/*Get first byte of sub-payload,*/
		/*To be determined. added 04/10/2008*/
		prCfaAsf->u1FirstBytePay = *(prCfaAsf->pu1HdrBuf + 1);

		if (prCfaAsf->eVidCodecType == CFA_VID_VC1) {
			if (u8SubPayloadLen < CFA_ASF_VC1_SC_LEN)
				prCfaAsf->fgExsitVc1StratCode = FALSE;
			else {
				LOADL_DWRD(prCfaAsf->pu1HdrBuf, prCfaAsf->u4Vc1StartCode);
				prCfaAsf->fgExsitVc1StratCode = CfaAsfExsitVc1StartCode(prCfaAsf->u4Vc1StartCode);
			}
		}
	}

	u4PrsLen = (u32)(u8Sa - prCfaAsf->u8PacketStartAdr + u8SubPayloadLen + prCfaAsf->u4PaddingLen);

	if (FALSE == prCfaAsf->fgExistMultiPayload) {
		prCfaAsf->fgErrSingleCompressed = TRUE;

		if ((u4PrsLen < (prCfaAsf->u4PacketLen)) &&
			(u4PrsLen < (prCfaAsf->u4SkipLen))) {
			/*pointer to next sub-payload.*/
			prCfaAsf->u8Ca = u8Sa + u8SubPayloadLen;
			prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_PRS_COMPRESS_PAYLOAD;
			/* prCfaAsf->u8PrsPts = prCfaAsf->u8PrsPts + prCfaAsf->u1PtsDelta * prCfaAsf->u4SubPayloadNum;*/
			prCfaAsf->u8PrsPts += prCfaAsf->u1PtsDelta;
			prCfaAsf->u4SubPayloadNum++;
			prCfaAsf->fgNeedPrs = FALSE;
		} 
		else if ((u4PrsLen == (prCfaAsf->u4PacketLen)) ||
				(u4PrsLen == (prCfaAsf->u4SkipLen))) 
		{
			/*single payload, compressed multi sub-payloads parsing finished.*/
			/*start to parsing next data packet.*/
				prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_SEARCH_HEADER;
				prCfaAsf->eCurCfaAsfPrsBitStrmType = CFA_ASF_PRS_BIT_STRM_TYPE_PACKET_HDR;
				prCfaAsf->u8Ca = prCfaAsf->u8PacketStartAdr + prCfaAsf->u4PacketLen;
				prCfaAsf->u8PacketStartAdr += prCfaAsf->u4PacketLen;
				prCfaAsf->u8PrsPts += prCfaAsf->u1PtsDelta; /*Guangjie Hu patch here*/
		} 
		else
		{
			DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] u4PrsLen=%d is incorrect,in CfaAsfTxComSubPayload2Fifo(),u4PacketLen=%d,u4SkipLen=%d.\r\n"),
						u4PrsLen,prCfaAsf->u4PacketLen,prCfaAsf->u4SkipLen);
			CfaAsfSetSplliterEOS(pvSptHdl, prCfaAsf);
			return;
		}
		mrRet = CfaAsfDemuxStrmData(u8Sa, u8SubPayloadLen, pvSptHdl, prCfaAsf);

		if (mrRet != RET_DMX_OK) {
			CfaAsfSetSplliterEOS(pvSptHdl, prCfaAsf);
			return;
		}
	} else {
		/*process multi-payload.*/
		u4PrsLen = (u32)(prCfaAsf->u8Ca -
					(prCfaAsf->u8PacketStartAdr + prCfaAsf->u4PacketHdrSkipr2Fifo) +
					u8SubPayloadLen + 1);

		if (u4PrsLen < prCfaAsf->u4PayloadLen) {
			/*means multiple sub-payload data.*/
			prCfaAsf->u8Ca = u8Sa + u8SubPayloadLen;
			prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_PRS_COMPRESS_PAYLOAD;
			/* prCfaAsf->u8PrsPts = prCfaAsf->u8PrsPts + prCfaAsf->u1PtsDelta * prCfaAsf->u4SubPayloadNum;*/
			prCfaAsf->u8PrsPts += prCfaAsf->u1PtsDelta;
			prCfaAsf->u4SubPayloadNum++;
			prCfaAsf->fgNeedPrs = FALSE;
		} else if (u4PrsLen == prCfaAsf->u4PayloadLen) {
				prCfaAsf->rCfaAsfMultiPayloadStart.u4PayloadNum--;
				prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_SEARCH_HEADER;

				if (0 == prCfaAsf->rCfaAsfMultiPayloadStart.u4PayloadNum) {
					/*parsing multipayloads finished,*/
					/*pointer to next packet start address.*/
					prCfaAsf->eCurCfaAsfPrsBitStrmType = CFA_ASF_PRS_BIT_STRM_TYPE_PACKET_HDR;
					prCfaAsf->u8Ca = prCfaAsf->u8PacketStartAdr + prCfaAsf->u4PacketLen;
					prCfaAsf->u8PacketStartAdr += prCfaAsf->u4PacketLen;
					} else {
					/*pointer to next payload data start.*/
					prCfaAsf->eCurCfaAsfPrsBitStrmType = CFA_ASF_PRS_BIT_STRM_TYPE_PAYLOAD_HDR;
					prCfaAsf->u8Ca += (u8SubPayloadLen + 1);
					prCfaAsf->u4PacketHdrSkipr2Fifo =
						(u32)(prCfaAsf->u8Ca - prCfaAsf->u8PacketStartAdr);
				}

				prCfaAsf->u8PrsPts += prCfaAsf->u1PtsDelta;
			} else {
					CfaAsfSetSplliterEOS(pvSptHdl, prCfaAsf);
					return;
			}
		mrRet = CfaAsfDemuxStrmData(u8Sa, u8SubPayloadLen, pvSptHdl, prCfaAsf);

		if (mrRet != RET_DMX_OK) {
			CfaAsfSetSplliterEOS(pvSptHdl, prCfaAsf);
			return;
		}
	}
}

/*//Description: get current stream id and check whether current payload is KeyFrame.*/
/**/
/*//@return u8 u1StrmId.*/
static u8 CfaAsfGetCurStrmIdKeyFrame
(
	u8 u1StrmNum,			/*/<[IN] include stream id and key frame flag.*/

	CfaAsfInst_T *prCfaAsf	/*/< [IN] point to CfaAsfInst*/
)
{
	u8 u1KeyFrame = 0;
	u8 u1StrmId = 0;

	u1StrmId = u1StrmNum & (u8)0x7F;

	u1KeyFrame = (u1StrmNum & (u8)0x80) >> 7;

	if ((u8)1 == u1KeyFrame)
		prCfaAsf->fgKeyFrame = TRUE;
	else
		prCfaAsf->fgKeyFrame = FALSE;

	return u1StrmId;
}

/*//Description: process single payload ,compressed payload data.*/
/**/
/*//@return none.*/
static void CfaAsfPrsCompressedPayload
(
	u8 *pu1CompressedPayloadStart, /*/< [IN] point to compressed payload data start addrress.*/
	u32 u4CompPayLoadDataLen,
	void *pvSptHdl,						 /*/< [IN] handle of splitter*/

	CfaAsfInst_T *prCfaAsf					/*/< [IN] point to CfaAsfInst*/
)
{
	/*parsing compressed data, and transfer to FIFO.*/
	u8 u1StrmNum = 0;
	u8 *pu1CurTemp = NULL;
	u32 u4SkipByte = 0;
	u32 u4ComPts = 0;
	u32 u4PayloadLen = 0;
	u8 u1SubPayloadLen = 0;
	u64 u8Sa = 0;
	u64 u8TxLen = 0;
	u32 u4PrsLen = 0;
	MRESULT mrRet = RET_DMX_OK;

	if (u4CompPayLoadDataLen < 1) {
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("CfaAsfPrsCompressedPayload() Compressed")
						  TEXT("Payload Data not enough 1\n"));
		CfaAsfFinishPrs(pvSptHdl, prCfaAsf);

		return;
	}

	pu1CurTemp = pu1CompressedPayloadStart;
	u1StrmNum = *pu1CurTemp;

	/*get current stream id and check whether current payload is KeyFrame.*/
	prCfaAsf->u1CurStrmId = CfaAsfGetCurStrmIdKeyFrame(u1StrmNum, prCfaAsf);

	u4SkipByte++;

	switch (prCfaAsf->rCfaAsfPropertyFlag.eObjNumType) {
	case Field_Type_UNEXIST:
		break;

	case Field_Type_BYTE:
		u4SkipByte += 1;
		break;

	case Field_Type_WORD:
		u4SkipByte += 2;
		break;

	case Field_Type_DWORD:
		u4SkipByte += 4;
		break;

	default:
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] Invalid  eObjNumType in CfaAsfPrsCompressedPayload().\r\n"));
		CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
		return;
	}

	if (u4CompPayLoadDataLen <= u4SkipByte) {
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("CfaAsfPrsCompressedPayload() Compressed")
						  TEXT("Payload Data not enough 2\n"));
		CfaAsfFinishPrs(pvSptHdl, prCfaAsf);

		return;
	}

	/*get compressed payload PTS*/
	pu1CurTemp = pu1CompressedPayloadStart + u4SkipByte;

	switch (prCfaAsf->rCfaAsfPropertyFlag.ePreTimeType) {
	case Field_Type_UNEXIST:
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] Line %d, invalid  ePreTimeType.\r\n"),DMX_LINE_NO);
		break;

	case Field_Type_BYTE:
		u4ComPts = *pu1CurTemp;
		u4ComPts &= 0x000000FF;
		u4SkipByte++;
		break;

	case Field_Type_WORD:
		LOADL_WORD(pu1CurTemp, u4ComPts);
		u4ComPts &= 0x0000FFFF;
		u4SkipByte += 2;
		break;

	case Field_Type_DWORD:
		LOADL_DWRD(pu1CurTemp, u4ComPts);
		u4ComPts &= 0xFFFFFFFF;
		u4SkipByte += 4;
		break;

	default:
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] Invalid  ePreTimeType in CfaAsfPrsCompressedPayload().\r\n"));
		CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
		return;
	}

	prCfaAsf->u8PrsPts = u4ComPts;

	switch (prCfaAsf->rCfaAsfPropertyFlag.eRepDataLenType) {
	case Field_Type_UNEXIST:
		break;

	case Field_Type_BYTE:
		u4SkipByte++;
		break;

	case Field_Type_WORD:
		u4SkipByte += 2;
		break;

	case Field_Type_DWORD:
		u4SkipByte += 4;
		break;

	default:
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] Invalid  eRepDataLenType in CfaAsfPrsCompressedPayload().\r\n"));
		CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
		return;
	}

	if (u4CompPayLoadDataLen <= (u4SkipByte + 1)) {
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("CfaAsfPrsCompressedPayload() Compressed")
						  TEXT("Payload Data not enough 3\n"));
		CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
		return;
	}

	pu1CurTemp = pu1CompressedPayloadStart + u4SkipByte;
	/*get presentation time delta for subpayload*/
	prCfaAsf->u1PtsDelta = *pu1CurTemp;
	u4SkipByte++;

	/*single or multiple payloads ,compressed payload data.*/
	/*the pointer to those division line.*/
	pu1CurTemp = pu1CompressedPayloadStart + u4SkipByte;

	if (FALSE == prCfaAsf->fgExistMultiPayload) {
		/*process single payload, compressed payload data.*/
		/*get sub-payload data length.*/
		u1SubPayloadLen = *pu1CurTemp;
		prCfaAsf->fgErrSingleCompressed = TRUE;
		/*for compressed payload, sub-payload length equal an entire Midea Object Size.*/
		prCfaAsf->u4MediaObjSize = u1SubPayloadLen;
		u4SkipByte++;

		prCfaAsf->u4PacketHdrSkipr2Fifo += u4SkipByte;

#if CFA_ASF_SYNC_BUF_BEFORE_TX

		/*keep previous flow (don't sync buf) if it is not a DRM file*/
		if (!(prCfaAsf->rCfaAsfDrmInfo.au1StreamEncrypted[(prCfaAsf->u1CurStrmId - (u8)1) / (u8)8] &
			  ((u8) 1 << ((prCfaAsf->u1CurStrmId - (u8)1) % (u8)8)))) {
#else
		{
#endif
			if (u4CompPayLoadDataLen <= u4SkipByte) {
				DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("CfaAsfPrsCompressedPayload()")
								  TEXT("Compressed Payload Data not enough 4\n"));
				CfaAsfFinishPrs(pvSptHdl, prCfaAsf);

				return;
			}

			/*Get first byte of payload data, added 04/10/2008*/
			prCfaAsf->u1FirstBytePay = *(pu1CompressedPayloadStart + u4SkipByte);

			if (prCfaAsf->eVidCodecType == CFA_VID_VC1) {
				if (u1SubPayloadLen < CFA_ASF_VC1_SC_LEN)
					prCfaAsf->fgExsitVc1StratCode = FALSE;
				else {
					LOADL_DWRD((pu1CompressedPayloadStart + u4SkipByte),
										  prCfaAsf->u4Vc1StartCode);
					prCfaAsf->fgExsitVc1StratCode =
						CfaAsfExsitVc1StartCode(prCfaAsf->u4Vc1StartCode);
				}
			}
		}

		u8Sa = prCfaAsf->u8PacketStartAdr + prCfaAsf->u4PacketHdrSkipr2Fifo;
		u8TxLen = u1SubPayloadLen;
		u4PrsLen = prCfaAsf->u4PacketHdrSkipr2Fifo + u1SubPayloadLen + prCfaAsf->u4PaddingLen;

		if ((u4PrsLen < (prCfaAsf->u4PacketLen)) &&
			(u4PrsLen < (prCfaAsf->u4SkipLen))) {
			prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_PRS_COMPRESS_PAYLOAD;
			prCfaAsf->eCurCfaAsfPrsBitStrmType = CFA_ASF_PRS_BIT_STRM_TYPE_NONE;
			prCfaAsf->u8Ca = u8Sa + u8TxLen;
			prCfaAsf->u4SubPayloadNum++;
			prCfaAsf->fgNeedPrs = FALSE;
		} else {
			prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_SEARCH_HEADER;
			prCfaAsf->eCurCfaAsfPrsBitStrmType = CFA_ASF_PRS_BIT_STRM_TYPE_PACKET_HDR;
			prCfaAsf->u8PacketStartAdr += prCfaAsf->u4PacketLen;
			prCfaAsf->u8Ca = prCfaAsf->u8PacketStartAdr;
			/*before parsing next payload or next packet ,shoule set SubpayloadNum = 0.*/
			prCfaAsf->u4SubPayloadNum = 0;
		}

		mrRet = CfaAsfDemuxStrmData(u8Sa, u8TxLen, pvSptHdl, prCfaAsf);

		if (mrRet != RET_DMX_OK) {
			CfaAsfSetSplliterEOS(pvSptHdl, prCfaAsf);
			return;
		}
	} else {
		/*process multiple payloads, compressed payload data.*/
		switch (prCfaAsf->rCfaAsfMultiPayloadStart.ePayloadLenType) {
		case Field_Type_BYTE:
			u4PayloadLen = *pu1CurTemp;
			u4PayloadLen &= 0x000000FF;
			u4SkipByte++;
			break;

		case Field_Type_WORD:
			LOADL_WORD(pu1CurTemp, u4PayloadLen);
			u4PayloadLen &= 0x0000FFFF;
			u4SkipByte += 2;
			break;

		case Field_Type_DWORD:
			LOADL_DWRD(pu1CurTemp, u4PayloadLen);
			u4PayloadLen &= 0xFFFFFFFF;
			u4SkipByte += 4;
			break;

		default:
			DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] Invalid  ePayloadLenType in CfaAsfPrsCompressedPayload().\r\n"));
			CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
			return;
		}

		prCfaAsf->u4PayloadLen = u4PayloadLen;

		if (u4CompPayLoadDataLen <= (u4SkipByte + 1)) {
			DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("CfaAsfPrsCompressedPayload()")
							   TEXT("Compressed Payload Data not enough 5\n"));
			CfaAsfFinishPrs(pvSptHdl, prCfaAsf);

			return;
		}

		pu1CurTemp = pu1CompressedPayloadStart + u4SkipByte;
		u1SubPayloadLen = *pu1CurTemp;

		/*for compressed payload, sub-payload length equal an entire Midea Object Size.*/
		prCfaAsf->u4MediaObjSize = u1SubPayloadLen;

		u4SkipByte++;
		prCfaAsf->u4PacketHdrSkipr2Fifo = prCfaAsf->u4PacketHdrSkipr2Fifo + u4SkipByte;

		u8Sa = prCfaAsf->u8PacketStartAdr + prCfaAsf->u4PacketHdrSkipr2Fifo;
		u8TxLen = u1SubPayloadLen;


		/*Skip error data in asf file.*/
		/*if ((u1SubPayloadLen == 0) && (prCfaAsf->u4PacketHdrSkipr2Fifo +
		prCfaAsf->u4PaddingLen == prCfaAsf->u4PacketLen))*/
		if (u1SubPayloadLen == 0) {
			if (1 == prCfaAsf->rCfaAsfMultiPayloadStart.u4PayloadNum) {
				/*skip to next packet start address*/
				CfaAsfSkipErr2NextPacket(pvSptHdl, prCfaAsf);
			} else {
				/*skip to next payload start address in DataPacket*/
				prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_SEARCH_HEADER;
				prCfaAsf->eCurCfaAsfPrsBitStrmType = CFA_ASF_PRS_BIT_STRM_TYPE_PAYLOAD_HDR;
				prCfaAsf->u8Ca = u8Sa + (u4PayloadLen - 1);
				prCfaAsf->u4PacketHdrSkipr2Fifo += (u4PayloadLen - 1);
				prCfaAsf->rCfaAsfMultiPayloadStart.u4PayloadNum--;
				DmxLogD(DMX_MOD_CFA_ASF,CFA_ASF_LOG_STATE,TEXT("[CFA ASF] Skip2NextPayloadSa: 0x%llx\n"),
					prCfaAsf->u8Ca);
				CfaAsfSearchHeader(pvSptHdl, prCfaAsf,
					CFA_ASF_ANA_ST_SEARCH_PAYLOAD_HEADER, 0, CFA_ASF_HDR_BUF_SZ, 0);
			}
			return;
		}

#if CFA_ASF_SYNC_BUF_BEFORE_TX

		/* keep previous flow (don't sync buf) if it is not a DRM file*/
		if (!(prCfaAsf->rCfaAsfDrmInfo.au1StreamEncrypted[(prCfaAsf->u1CurStrmId - (u8)1) / (u8)8] & ((u8) 1 <<
				((prCfaAsf->u1CurStrmId - (u8)1) % (u8)8)))) {
#endif		 /*Get first byte of payload data, added 04/10/2008*/

			if (u4CompPayLoadDataLen <= u4SkipByte) {
				DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,
					TEXT("CfaAsfPrsCompressedPayload() Compressed")
					TEXT("Payload Data not enough 6\n"));
				CfaAsfFinishPrs(pvSptHdl, prCfaAsf);

				return;
			}

			prCfaAsf->u1FirstBytePay = *(pu1CompressedPayloadStart + u4SkipByte);

			if (prCfaAsf->eVidCodecType == CFA_VID_VC1) {
				if (u1SubPayloadLen < CFA_ASF_VC1_SC_LEN)
					prCfaAsf->fgExsitVc1StratCode = FALSE;
				else {
					LOADL_DWRD((pu1CompressedPayloadStart + u4SkipByte),
										  prCfaAsf->u4Vc1StartCode);
					prCfaAsf->fgExsitVc1StratCode =
						CfaAsfExsitVc1StartCode(prCfaAsf->u4Vc1StartCode);
				}
			}
		}

		if ((u32)(u1SubPayloadLen + 1) < u4PayloadLen) {
			/*means multiple sub-payload data.*/
			prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_PRS_COMPRESS_PAYLOAD;
			prCfaAsf->eCurCfaAsfPrsBitStrmType = CFA_ASF_PRS_BIT_STRM_TYPE_NONE;
			prCfaAsf->fgNeedPrs = FALSE;
			prCfaAsf->u8Ca = u8Sa + u8TxLen;
			prCfaAsf->u4PacketHdrSkipr2Fifo -= 1;
			prCfaAsf->u4SubPayloadNum++;
		} else {
			prCfaAsf->rCfaAsfMultiPayloadStart.u4PayloadNum--;
			prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_SEARCH_HEADER;

			if (0 == prCfaAsf->rCfaAsfMultiPayloadStart.u4PayloadNum) {
				/*parsing multipayloads finished,*/
				/*pointer to next packet start address.*/
				prCfaAsf->eCurCfaAsfPrsBitStrmType = CFA_ASF_PRS_BIT_STRM_TYPE_PACKET_HDR;
				prCfaAsf->u8Ca = prCfaAsf->u8PacketStartAdr + prCfaAsf->u4PacketLen;
				prCfaAsf->u8PacketStartAdr += prCfaAsf->u4PacketLen;
			} else {
				prCfaAsf->eCurCfaAsfPrsBitStrmType = CFA_ASF_PRS_BIT_STRM_TYPE_PAYLOAD_HDR;
				prCfaAsf->u8Ca = u8Sa  + u8TxLen;
				prCfaAsf->u4PacketHdrSkipr2Fifo += (u32) u8TxLen;
			}

			/*before parsing next payload or next packet ,shoule set SubpayloadNum = 0.*/
			prCfaAsf->u4SubPayloadNum = 0;
		}

		mrRet = CfaAsfDemuxStrmData(u8Sa, u8TxLen, pvSptHdl, prCfaAsf);

		if (mrRet != RET_DMX_OK)
			CfaAsfSetSplliterEOS(pvSptHdl, prCfaAsf);
			return;
	}

}


/*//Description: Get cfa asf compessed subpayload data length*/
/*//return none.*/
static void CfaAsfGetCompressPayloadLen
(
	void *pvSptHdl,		 /*/< [IN] handle of splitter*/

	CfaAsfInst_T *prCfaAsf	/*/< [IN] point to CfaAsfInst*/
)
{

	if (TRUE == prCfaAsf->fgNeedPrs)
		CfaAsfTxComSubPayload2Fifo(pvSptHdl, prCfaAsf);
	else {
		prCfaAsf->fgNeedPrs = TRUE;
		CfaAsfSearchHeader(pvSptHdl, prCfaAsf, prCfaAsf->eCurCfaAsfAnaSt, (u64)0,
				   (u64)CFA_ASF_COMPRESS_PAYLOAD_HDR_SZ, (u32)0);
	}
}

#if CFA_ASF_SUPPORT_VOD_DRM
/*//Description: For WDRM ND format*/
/*//Get standard payload extension system encryption sample id*/
/*//@Return u64*/
static u64 CfaAsfGetPLExtSysSampleId
(
	const u8 *pu1RepField,
	bool fgVid,
	u8 u1StrmId,
	const CfaAsfInst_T *prCfaAsf
)
{
	u8 u1Idx = 0;
	u8 u1Idy = 0;
	u64 u8SampleId = 0;
	u32 u4SkipSize = 0;
	CfaPayloadExtSysId_E ePayloadExtSysId = CFA_PL_EXT_SYS_ID_UNKNOW;
	u16 u2SysDataSize = 0;
	u8 u1AudInfoIdx = 0;

	for (u1Idx = 0; u1Idx < (u8)MAX_ASF_PL_EXT_SYS_ID; u1Idx++) {
		if (fgVid) {
			ePayloadExtSysId = prCfaAsf->
					   rCfaAsfVidInfo.rCfaAsfStrmInfo.arPayloadExtSysInfo[u1Idx].ePayloadExtSysId;
			u2SysDataSize = prCfaAsf->rCfaAsfVidInfo.rCfaAsfStrmInfo.arPayloadExtSysInfo[u1Idx].u2DataSize;
		} else {
			u1AudInfoIdx = CfaAsfGetCurAudInfoIdx(prCfaAsf, (u32)u1StrmId);
			if (u1AudInfoIdx >= (u8)MAX_ASF_AUD_STRM_NUM)
			{
				DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[CFA ASF] Invalid u1AudInfoIdx,u1AudInfoIdx=%d in CfaAsfGetPLExtSysSampleId().\r\n"),u1AudInfoIdx);
				return INVALID_EXT_SYS_ENCRYPTION_ID;
			}
			ePayloadExtSysId = prCfaAsf->
				raCfaAsfAudInfo[u1AudInfoIdx].rCfaAsfStrmInfo.arPayloadExtSysInfo
				[u1Idx].ePayloadExtSysId;
			u2SysDataSize = prCfaAsf->
				raCfaAsfAudInfo[u1AudInfoIdx].rCfaAsfStrmInfo.arPayloadExtSysInfo[u1Idx].u2DataSize;
		}

		switch (ePayloadExtSysId) {
		case CFA_PL_EXT_SYS_ID_UNKNOW:
			break;

		case CFA_PL_EXT_SYS_ID_TC:
		case CFA_PL_EXT_SYS_ID_FN:
		case CFA_PL_EXT_SYS_ID_CT:
		case CFA_PL_EXT_SYS_ID_AR:
		case CFA_PL_EXT_SYS_ID_SR:
			u4SkipSize += (u32)u2SysDataSize;
			break;

		case CFA_PL_EXT_SYS_ID_SID: {
			pu1RepField += u4SkipSize;

			if ((u16)0xFFFF == u2SysDataSize) {
				LOADL_WORD(pu1RepField, u2SysDataSize);
				pu1RepField += 2;
			} while (u2SysDataSize != 0) {
				u8SampleId |= (u64)((u64)(*(pu1RepField + u1Idy) << 8 * (u2SysDataSize-(u16)1)));
				u1Idy++;
				u2SysDataSize--;
			}

			return u8SampleId;
			}

		default:
			break;
		}
	}

	return INVALID_EXT_SYS_ENCRYPTION_ID;
}
#endif


/*//Description: Analyze video and audio payload data header.*/
/*//@return none.*/

static MRESULT CfaAsfPrsPayloadHeader
(
	u8 *pu1PayloadStartAdr,  /*/<[IN] point to payload start address.*/
	u32 u4PayloadDataLen,
	void *pvSptHdl,			  /*/<[IN] handle of splitter*/

	CfaAsfInst_T *prCfaAsf		/*/< [IN] point to CfaAsfInst*/
)
{
	u32 u4SkipByte = 0;
	u32 u4RepDataLen = 0;
	u8 *pu1Temp = NULL;
	u32 u4PayloadLen = 0;
	u32 u4OftInMediaObj = 0;
	u32 u4TmpPsrPts = 0;

	if ((NULL ==  pu1PayloadStartAdr) || (NULL == prCfaAsf))
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[CFA ASF] Line %d, pu1PayloadStartAdr or prCfaAsf is NULL.\r\n"),DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	
	pu1Temp = pu1PayloadStartAdr;
	/*skip first byte "Stream Number"*/
	u4SkipByte++;

	/*/parsing single video payload.*/
	/*/field type name: Media Object Number.*/
	switch (prCfaAsf->rCfaAsfPropertyFlag.eObjNumType) {
	case Field_Type_UNEXIST:
		/*no action, skip zero.*/
		break;

	case Field_Type_BYTE:
		u4SkipByte++;
		break;

	case Field_Type_WORD:
		u4SkipByte += (u32)2;
		break;

	case Field_Type_DWORD:
		u4SkipByte += (u32)4;
		break;

	default:
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[CFA ASF] Line %d, invalid eObjNumType.\r\n"),DMX_LINE_NO);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (u4PayloadDataLen <= u4SkipByte) {
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("CfaAsfPrsPayloadHeader() Payload Data not enough 1\n"));

		MM_RETURN(RET_DMX_UNEXPECT);
	}

	/*Field name:Express Offset Into Media Object value for non-compressed payload.*/
	/*Note : when this payload is compressed, the field is Presentation Time.*/
	pu1Temp = pu1PayloadStartAdr + u4SkipByte;

	switch (prCfaAsf->rCfaAsfPropertyFlag.ePreTimeType) {
	case Field_Type_UNEXIST:
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[CFA ASF] Line %d, invalid ePreTimeType.\r\n"),DMX_LINE_NO);
		break;

	case Field_Type_BYTE:
		u4OftInMediaObj = *pu1Temp;
		u4OftInMediaObj &= (u32)0x000000FF;
		u4SkipByte++;
		break;

	case Field_Type_WORD:
		LOADL_WORD(pu1Temp, u4OftInMediaObj);
		u4OftInMediaObj &= (u32)0x0000FFFF;
		u4SkipByte += (u32)2;
		break;

	case Field_Type_DWORD:
		LOADL_DWRD(pu1Temp, u4OftInMediaObj);
		u4OftInMediaObj &= 0xFFFFFFFF;
		u4SkipByte += (u32)4;
		break;

	default:
		CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
		MM_RETURN(RET_DMX_UNEXPECT);
		}

	prCfaAsf->u4OftInMeidaObj = u4OftInMediaObj;

	/*added by tinghu. For video stream, fill object start packet instead of last packet number when fill AU info*/
	/* Audio stream fill AU info also need change!*/
	if ((0 == (prCfaAsf->
		u4OftInMeidaObj)) && ((prCfaAsf->u1CurStrmId) == (prCfaAsf->rCfaAsfVidInfo.rCfaAsfStrmInfo.u1StrmNum)))
		prCfaAsf->u8KeyPacketId = prCfaAsf->u8CurPacketId;

	if (u4PayloadDataLen <= u4SkipByte) {
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("CfaAsfPrsPayloadHeader() Payload Data not enough 2\n"));

		MM_RETURN(RET_DMX_UNEXPECT);
	}

	/*Field name:Replicated Data Length.*/
	pu1Temp = pu1PayloadStartAdr + u4SkipByte;

	switch (prCfaAsf->rCfaAsfPropertyFlag.eRepDataLenType) {
	case Field_Type_UNEXIST:
		u4RepDataLen = 0;
		break;

	case Field_Type_BYTE:
		u4RepDataLen = *pu1Temp;
		u4RepDataLen &= (u32)0x000000FF;
		u4SkipByte++;
		break;

	case Field_Type_WORD:
		LOADL_WORD(pu1Temp, u4RepDataLen);
		u4RepDataLen &= (u32)0x0000FFFF;
		u4SkipByte += (u32)2;
		break;

	case Field_Type_DWORD:
		LOADL_DWRD(pu1Temp, u4RepDataLen);
		u4RepDataLen &= 0xFFFFFFFF;
		u4SkipByte += (u32)4;
		break;

	default:
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[CFA ASF] Line %d, invalid eRepDataLenType.\n"),DMX_LINE_NO);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	/*if u4RepDataLen = 1, the payload should be interpreted as a compressed payload.*/
	/*otherwise, valid values are 0 or values greater than or equal to 8. by ASF Specification.*/
	if (0 == u4RepDataLen) {
		DmxLogD(DMX_MOD_CFA_ASF,CFA_ASF_LOG_STATE,TEXT("[CFA ASF] Finish for replicated data length zero!\n"));
		MM_RETURN(RET_DMX_UNEXPECT);
	} else if (1 == u4RepDataLen) {
					prCfaAsf->u4OftInMeidaObj = 0;
					prCfaAsf->fgExistCompressData = TRUE;
					CfaAsfPrsCompressedPayload(pu1PayloadStartAdr,
						u4PayloadDataLen, pvSptHdl, prCfaAsf);
	} else {
		prCfaAsf->fgExistCompressData = FALSE;
		/*Error handle for	skipping error packet to next packet , parsing from start AU.*/

		if (u4PayloadDataLen <= (u4SkipByte + 4)) {
			DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("CfaAsfPrsPayloadHeader()")
				TEXT("Payload Data not enough 3\n"));
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		/*Field name: Replicated Data.*/
		/*get an entire Media Object size that this payload belongs to.*/
		pu1Temp = pu1PayloadStartAdr + u4SkipByte;
		LOADL_DWRD(pu1Temp, prCfaAsf->u4MediaObjSize);

		/*get PTS of the Media Object that this payload belongs to.*/
		pu1Temp = pu1PayloadStartAdr + u4SkipByte + 4;
		LOADL_DWRD(pu1Temp, u4TmpPsrPts);
		prCfaAsf->u8PrsPts = u4TmpPsrPts;

#if CFA_ASF_SUPPORT_VOD_DRM

		if (2 == prCfaAsf->rCfaAsfDrmInfo.u1WMDRMType) {
			bool fgVidStrm = FALSE;

			if (u4PayloadDataLen <= (u4SkipByte + 8)) {
				DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("CfaAsfPrsPayloadHeader() Payload Data not enough 4\n"));

				MM_RETURN(RET_DMX_UNEXPECT);
			}

			pu1Temp = pu1PayloadStartAdr + u4SkipByte + 8;

			if (prCfaAsf->u1CurStrmId == prCfaAsf->rCfaAsfVidInfo.rCfaAsfStrmInfo.u1StrmNum)
				fgVidStrm = TRUE;
			else
				fgVidStrm = FALSE;

			prCfaAsf->u8ExtSampleId =
				CfaAsfGetPLExtSysSampleId(pu1Temp, fgVidStrm, prCfaAsf->u1CurStrmId, prCfaAsf);
		}

#endif //CFA_ASF_SUPPORT_VOD_DRM

		/*get payload length of multipayload.*/
		if (u4PayloadDataLen <= (u4RepDataLen + u4SkipByte)) {
			DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("CfaAsfPrsPayloadHeader() Payload Data not enough 5\n"));

			MM_RETURN(RET_DMX_UNEXPECT);
		}

		pu1Temp = pu1PayloadStartAdr + u4RepDataLen + u4SkipByte;

		if (TRUE == prCfaAsf->fgExistMultiPayload) {
			switch (prCfaAsf->rCfaAsfMultiPayloadStart.ePayloadLenType) {
			case Field_Type_BYTE:
				u4PayloadLen = *pu1Temp;
				u4PayloadLen &= 0x000000FF;
				u4SkipByte++;
				break;

			case Field_Type_WORD:
				LOADL_WORD(pu1Temp, u4PayloadLen);
				u4PayloadLen &= 0x0000FFFF;
				u4SkipByte += 2;
				break;

			case Field_Type_DWORD:
				LOADL_DWRD(pu1Temp, u4PayloadLen);
				u4PayloadLen &= 0xFFFFFFFF;
				u4SkipByte += 4;
				break;

			default:
				DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[CFA ASF]Line %d, invalid ePayloadLenType.\n"),DMX_LINE_NO);
				MM_RETURN(RET_DMX_UNEXPECT);
			}
		}

		/*prCfaAsf->u4PayloadLen just useful for multipayload in Data Packet,*/
		/*Should reset zero value for single paylaod.*/
		prCfaAsf->u4PayloadLen = u4PayloadLen;

		prCfaAsf->u4PacketHdrSkipr2Fifo =
			prCfaAsf->u4PacketHdrSkipr2Fifo + u4RepDataLen + u4SkipByte;

#if CFA_ASF_SYNC_BUF_BEFORE_TX

		/* keep previous flow (don't sync buf) if it is not a DRM file*/
		if (!(prCfaAsf->rCfaAsfDrmInfo.au1StreamEncrypted[(prCfaAsf->u1CurStrmId - (u8)1) / (u8)8] &
			  ((u8) 1 << ((prCfaAsf->u1CurStrmId - (u8)1) % (u8)8)))) {
#endif		  /*Get first byte of payload data, added 04/10/2008*/

			if (u4PayloadDataLen <= (u4RepDataLen + u4SkipByte)) {
				DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("CfaAsfPrsPayloadHeader() Payload Data not enough 6\n"));

				MM_RETURN(RET_DMX_UNEXPECT);
			}

			prCfaAsf->u1FirstBytePay = *(pu1PayloadStartAdr + u4RepDataLen + u4SkipByte);

			if (prCfaAsf->eVidCodecType == CFA_VID_VC1) {
				if ((TRUE == prCfaAsf->fgExistMultiPayload) &&
					(u4PayloadLen < CFA_ASF_VC1_SC_LEN))
					prCfaAsf->fgExsitVc1StratCode = FALSE;
				else {
					/*maybe need to calculate single payload data length*/
					LOADL_DWRD((pu1PayloadStartAdr + u4RepDataLen + u4SkipByte),
						prCfaAsf->u4Vc1StartCode);
					prCfaAsf->
					fgExsitVc1StratCode = CfaAsfExsitVc1StartCode(prCfaAsf->u4Vc1StartCode);
				}
			}
		}
	}
	MM_RETURN(RET_DMX_OK);
}

/*//Description:start parsing video or audio information.*/
/*//return none.*/

static void CfaAsfPrsPayload
(
	void *pvSptHdl,		 /*/<[IN] handle of splitter*/
	u8 *pu1PayloadStart,  /*/<[IN] pointer to video payload start address.*/
	u32 u4PayloadLen,
	CfaAsfInst_T *prCfaAsf	/*/< [IN] point to CfaAsfInst*/
)
{
	u64 u8Sa = 0;
	u64 u8TxLen = 0;
	MRESULT mrRet = RET_DMX_OK;

	mrRet = CfaAsfPrsPayloadHeader(pu1PayloadStart, u4PayloadLen, pvSptHdl, prCfaAsf);

	if (mrRet != RET_DMX_OK) {
		CfaAsfSetSplliterEOS(pvSptHdl, prCfaAsf);
		return;
	} 

	/*Fix some payload length is too large problem. Need to use payload + payload start addr*/
	if (prCfaAsf->u4PayloadLen > prCfaAsf->rCfaAsfFileInfo.u4DataPacketSize) {
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[CFA ASF] Finish for error payload length!\n"));
		CfaAsfSetSplliterEOS(pvSptHdl, prCfaAsf);
		return;
	}

	if (TRUE == prCfaAsf->fgExistCompressData) {
		/*compressed payload data was finished to handle independently.*/
		prCfaAsf->fgExistCompressData = FALSE;
		return;
	}

	if (FALSE == prCfaAsf->fgExistMultiPayload) {
		/*process single payload.*/
		prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_SEARCH_HEADER;
		prCfaAsf->eCurCfaAsfPrsBitStrmType = CFA_ASF_PRS_BIT_STRM_TYPE_PACKET_HDR;
		prCfaAsf->u8Ca = prCfaAsf->u8PacketStartAdr + prCfaAsf->u4PacketLen;
		u8Sa = prCfaAsf->u8PacketStartAdr + prCfaAsf->u4PacketHdrSkipr2Fifo;
		u8TxLen = (prCfaAsf->u4PacketLen
			 - prCfaAsf->u4PacketHdrSkipr2Fifo - prCfaAsf->u4PaddingLen);
		/*get next packet start address.*/
		prCfaAsf->u8PacketStartAdr += prCfaAsf->u4PacketLen;
		} else {
		/*process multipayload.*/
		/*pointer to next payload.*/
		/*prCfaAsf->u8Ca = prCfaAsf->u8PacketStartAdr + prCfaAsf->u4PacketHdrSkipr2Fifo*/
		/* + prCfaAsf->u4PayloadLen;*/
		u8Sa = prCfaAsf->u8PacketStartAdr + prCfaAsf->u4PacketHdrSkipr2Fifo;
		u8TxLen = prCfaAsf->u4PayloadLen;

		prCfaAsf->rCfaAsfMultiPayloadStart.u4PayloadNum--;
		prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_SEARCH_HEADER;

		if (0 == prCfaAsf->rCfaAsfMultiPayloadStart.u4PayloadNum) {
			/*parsing multipayloads finished,*/
			/*pointer to next packet start address.*/
			prCfaAsf->eCurCfaAsfPrsBitStrmType = CFA_ASF_PRS_BIT_STRM_TYPE_PACKET_HDR;
			prCfaAsf->u8Ca = prCfaAsf->u8PacketStartAdr + prCfaAsf->u4PacketLen;
			prCfaAsf->u8PacketStartAdr += prCfaAsf->u4PacketLen;
			} else {
			prCfaAsf->eCurCfaAsfPrsBitStrmType = CFA_ASF_PRS_BIT_STRM_TYPE_PAYLOAD_HDR;
			prCfaAsf->u8Ca = u8Sa + u8TxLen;
			prCfaAsf->u4PacketHdrSkipr2Fifo += (u32) u8TxLen;
		}
	}

	mrRet = CfaAsfDemuxStrmData(u8Sa, u8TxLen, pvSptHdl, prCfaAsf);

	if (mrRet != RET_DMX_OK) {
		CfaAsfSetSplliterEOS(pvSptHdl, prCfaAsf);
		return;
	}

}
static void CfaAsfAnalyzePayload(void *pvSptHdl, CfaAsfInst_T *prCfaAsf);

/*//Descripition: Error handle for skiping OpaqueDataPacket in asf file.*/
/*//*/
/*//@return NONE.*/
static void CfaAsfSkipOpaqueDataPacket(void *pvSptHdl, CfaAsfInst_T *prCfaAsf)
{
	prCfaAsf->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_SEARCH_HEADER;
	prCfaAsf->eCurCfaAsfPrsBitStrmType = CFA_ASF_PRS_BIT_STRM_TYPE_PACKET_HDR;
	prCfaAsf->u8Ca = prCfaAsf->u8PacketStartAdr + prCfaAsf->rCfaAsfFileInfo.u4DataPacketSize;
	prCfaAsf->u8PacketStartAdr += prCfaAsf->rCfaAsfFileInfo.u4DataPacketSize;
	DmxLogD(DMX_MOD_CFA_ASF,CFA_ASF_LOG_SET,TEXT("[CFA ASF OpaqueData] Skip2NextPacketSa: 0x%llx\n"), prCfaAsf->u8Ca);
	CfaAsfSearchHeader(pvSptHdl, prCfaAsf, CFA_ASF_ANA_ST_SEARCH_PACKET_HEADER, (u64)0, (u64)CFA_ASF_HDR_BUF_SZ, (u32)0);
}

/*//Description:Analyze each data packet header*/
/*//			   and search next analysis start code*/
/*//@return none*/
static void CfaAsfAnalyzePacketHeader
(
	void *pvSptHdl,			 /*/<[IN] handle of splitter*/

	CfaAsfInst_T *prCfaAsf		/*/< [IN] point to CfaAsfInst*/
)
{
	u8 *pu1Temp = NULL;
	u8 u1OneByte = 0;
	CfaAsfLenTypeFlag_T rCfaAsfLenTypeFlag = {0};
	u32 u4SkipByte = 0;
	u32 u4ErrorCorDataLen = 0;
	u32 u4PayloadParsingInfoLen = 0;
	MRESULT mrResult = RET_DMX_OK;

	pu1Temp = prCfaAsf->pu1HdrBuf;

	/*initialize parsing some data packet  information*/
	prCfaAsf->u4PacketHdrSkipr2Fifo = 0;
	prCfaAsf->u4SubPayloadNum = 0;
	prCfaAsf->fgExistCompressData = FALSE;
	prCfaAsf->u4PaddingLen = 0;

	/*Check Opaque Data whether skip to next packet or finished parsing.*/
	/*Opaque Data Present == 1 reference to ASF SPEC.*/
	if ((u8)1 == ((*pu1Temp & (u8)0x10) >> 4)) {
		DmxLogD(DMX_MOD_CFA_ASF,CFA_ASF_LOG_STATE,
			TEXT("[ASF CFA] Skip to next packet for Opaque Data Present in Data Packet.\n"));
		CfaAsfSkipOpaqueDataPacket(pvSptHdl, prCfaAsf);  /*modify by mtk94053 at 20120820 for fixbug:CNB00135700*/

		/*CfaAsfSetSplliterEOS(pvSptHdl, prCfaAsf);*/
		return;
	}

	/* Guangjie Hu add here @ 2008-9-11 to fix CQ BDP116158*/
	/*error correct data length should be 2 or be 0 when error correction length type is not 0*/
	if (((*pu1Temp & (u8)0x0F) != (u8)2) && (!(((*pu1Temp & (u8)0x0F) == 0) && (((*pu1Temp & (u8)0x60) >> 5) != 0)))) {
		DmxLogD(DMX_MOD_CFA_ASF,CFA_ASF_LOG_STATE,TEXT("[ASF CFA] Skip to next packet for error correction length error.\n"));
		CfaAsfSkipOpaqueDataPacket(pvSptHdl, prCfaAsf);

		/*CfaAsfSetSplliterEOS(pvSptHdl, prCfaAsf);*/
		return;
	}

	if (TRUE == CfaAsfExistErrCorData(pvSptHdl, prCfaAsf))
		pu1Temp = pu1Temp + prCfaAsf->rCfaAsfErrCorFlag.u1DataLen;

	/*skip first byte field: Error Correction Flags.*/
	pu1Temp++;

	if (prCfaAsf->u4HdrBufDataLen < (u32)(prCfaAsf->rCfaAsfErrCorFlag.u1DataLen + 3)) {
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("CfaAsfAnalyzePacketHeader() u4HdrBufDataLen not enough\n"));

		CfaAsfFinishPrs(pvSptHdl, prCfaAsf);

		return;
	}

	/*pu1Temp point to payload parsing information start byte.*/
	/*get Length Type Flags.*/
	u1OneByte = *pu1Temp;
	mrResult = CfaAsfGetLenTypeFlag(u1OneByte, &rCfaAsfLenTypeFlag, prCfaAsf);
	if (RET_DMX_OK != mrResult)
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[CFA ASF] Line %d,call CfaAsfGetLenTypeFlag() fail!\n"),DMX_LINE_NO);
		CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
		return;
	}

	/*pu1Temp pointer to next byte at Payload parsing information.*/
	/*get property flags.*/
	pu1Temp++;
	u1OneByte = *pu1Temp;
	mrResult = CfaAsfGetPropertyFlag(u1OneByte, prCfaAsf);
	if (RET_DMX_OK != mrResult)
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[CFA ASF] Line %d,call CfaAsfGetPropertyFlag() fail!\n"),DMX_LINE_NO);
		CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
		return;
	}

	/*get Payload parsing information last bytes to skip*/
	mrResult = CfaAsfGetSkipByte(pvSptHdl, rCfaAsfLenTypeFlag, pu1Temp, prCfaAsf, &u4SkipByte);
	if (RET_DMX_OK != mrResult)
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[CFA ASF] Line %d,call CfaAsfGetSkipByte() fail!\n"),DMX_LINE_NO);
		CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
		return;
	}

	u4ErrorCorDataLen = (u32)((u8)1 + prCfaAsf->rCfaAsfErrCorFlag.u1DataLen);
	u4PayloadParsingInfoLen = (u32)2 + u4SkipByte;

	prCfaAsf->u8Ca = prCfaAsf->u8Ca + u4ErrorCorDataLen + u4PayloadParsingInfoLen;
	prCfaAsf->u4PacketHdrSkipr2Fifo = u4ErrorCorDataLen + u4PayloadParsingInfoLen;


	prCfaAsf->eCurCfaAsfPrsBitStrmType = CFA_ASF_PRS_BIT_STRM_TYPE_NONE;

	/*decrease times of sync pbbuf, improve performance*/
	if ((u4ErrorCorDataLen + u4PayloadParsingInfoLen) <= ((u32)CFA_ASF_HDR_BUF_SZ >> 1)) {
		prCfaAsf->pu1HdrBuf = prCfaAsf->pu1HdrBuf + u4ErrorCorDataLen + u4PayloadParsingInfoLen;

		if (prCfaAsf->u4HdrBufDataLen < (u4ErrorCorDataLen + u4PayloadParsingInfoLen)) {
			DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("CfaAsfAnalyzePacketHeader() u4HdrBufDataLen not enough\n"));

			CfaAsfFinishPrs(pvSptHdl, prCfaAsf);

			return;
		}

		prCfaAsf->u4HdrBufDataLen = prCfaAsf->u4HdrBufDataLen - (u4ErrorCorDataLen + u4PayloadParsingInfoLen);
		CfaAsfAnalyzePayload(pvSptHdl, prCfaAsf);
	} else {
		/*change to next analyze state: search payload start code.*/
		CfaAsfSearchHeader(pvSptHdl, prCfaAsf,
				   CFA_ASF_ANA_ST_SEARCH_PAYLOAD_HEADER,
				   (u64)0, (u64)CFA_ASF_HDR_BUF_SZ, (u32)0);
	}
}



/*//Description:Analyze each payload data header.*/
/*//			   and demux audio & vedio.*/
/*//@return none.*/
static void CfaAsfAnalyzePayload
(
	void *pvSptHdl,			 /*/[IN] handle of splitter*/

	CfaAsfInst_T *prCfaAsf		/*/< [IN] point to CfaAsfInst*/
)
{
	u8 u1StrmNum = 0;
	u8 u1StartByte = 0;
	u8 *pu1PayloadHdr = NULL;
	u32 u4PayloadHdrDataLen = 0;

	pu1PayloadHdr = prCfaAsf->pu1HdrBuf;
	u4PayloadHdrDataLen = prCfaAsf->u4HdrBufDataLen;

	if (u4PayloadHdrDataLen < (u32)2) {
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("CfaAsfAnalyzePayload() Payload Data not enough\n"));

		CfaAsfFinishPrs(pvSptHdl, prCfaAsf);

		return;
	}

	u1StartByte = *pu1PayloadHdr;

	if (TRUE == prCfaAsf->fgExistMultiPayload) {
		if (TRUE == prCfaAsf->fgFirstParsingMultiPayload) {
			prCfaAsf->rCfaAsfMultiPayloadStart.u4PayloadNum = (u32)(u1StartByte & (u8)0x3F);

			switch ((u1StartByte & (u8)0xC0) >> 6) {
			case (u8)1:
				prCfaAsf->rCfaAsfMultiPayloadStart.ePayloadLenType = Field_Type_BYTE;
				break;

			case (u8)2:
				prCfaAsf->rCfaAsfMultiPayloadStart.ePayloadLenType = Field_Type_WORD;
				break;

			case (u8)3:
				prCfaAsf->rCfaAsfMultiPayloadStart.ePayloadLenType = Field_Type_DWORD;
				break;

			default:
				DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,
					TEXT("[CFA ASF] Finished for error multipayloads in  data packet.\n"));
				/* Guangjie Hu modify, notify err instead of finish parsing*/
				CfaAsfSetSplliterEOS(pvSptHdl, prCfaAsf);
				return;
			}

			pu1PayloadHdr++;
			u4PayloadHdrDataLen--;

			u1StrmNum = *pu1PayloadHdr;
			prCfaAsf->fgFirstParsingMultiPayload = FALSE;

			/*for multi-payload start byte.*/
			prCfaAsf->u4PacketHdrSkipr2Fifo += CFA_ASF_MULTI_PAYLOAD_START_B;
		}

		u1StrmNum = *pu1PayloadHdr;
	} else
			u1StrmNum = *pu1PayloadHdr;

	/*get current stream id and check whether current payload is KeyFrame.*/
	prCfaAsf->u1CurStrmId = CfaAsfGetCurStrmIdKeyFrame(u1StrmNum, prCfaAsf);
	if (prCfaAsf->u1CurStrmId >= 128)
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("CfaAsfAnalyzePayload() u1CurStrmId=%d error.\n"),prCfaAsf->u1CurStrmId);

		CfaAsfFinishPrs(pvSptHdl, prCfaAsf);

		return;
	}
	/*Error handle, Best solution checking all video stream and audioo stream number.*/

	CfaAsfPrsPayload(pvSptHdl, pu1PayloadHdr, u4PayloadHdrDataLen, prCfaAsf);

}


/*//Description: Get the index of audio info by audio stream ID.*/
/*//				For multi audio.*/
/*//@return the index of audio info.*/
u8 CfaAsfGetCurAudInfoIdx(const CfaAsfInst_T *prCfaAsf, u32 u4StrmId)
{
	u8 u4Idx = 0;

	for (u4Idx = 0; u4Idx < (u8)MAX_ASF_AUD_STRM_NUM; u4Idx++) {
		if (prCfaAsf->raCfaAsfAudInfo[u4Idx].rCfaAsfStrmInfo.u1StrmNum == u4StrmId)
			return u4Idx;
	}

	/*/error handling only.*/
	return 0;
}

/*// ASF CFA processes CFA_ASF_ANA_ST_IDLE*/
/*// @return None*/
static void CfaAsfAnaStIdle
(
	void		*pvSptHdl,		  /*/< [IN] handle of fdmx*/
	u64 	   u8TxLen,/*/< [IN] Actual transferred data length.
	Normally this value should be equal to the u8TxLen in the previous transfer issue, unless file end is hit.*/
	CfaAsfInst_T  *prCfaAsf/*/< [IN] point to CfaAsfInst*/
)
{
	/*/ can not go here!*/

}

/*// ASF CFA state control for transfer done*/
/*// @return None*/
/*// @note	This function will be called after a transfer is complete.*/
void CfaAsfTxDoneStCtrl(
	void *pvSptHdl,		  /*/[IN] handle of splitter*/
	u64 u8TxLen,		/*/[IN]  Actual transferred data length.Normally this value should be
	equal to the u4Lenin the previous transfer issue, unless file end is hit.*/
	CfaAsfInst_T *prCfaAsf	/*/[IN] pointer to CfaAviInst.*/
)
{
	MRESULT mrRet = RET_DMX_OK;

	do {
		prCfaAsf->fgNoSyncPbb = FALSE;

		/*using sync DMA, 2007/12/28*/
		/* check if Tx done results from Tx data to header buffer*/
		if (TRUE == prCfaAsf->fgTxData2HdrBuf) {
			/*DMXLOG_TRACE(TEXT("CfaAsfTxDoneStCtrl() prCfaAsf->u4MemDataLen: %d,
			eCurCfaAsfAnaSt: 0x%x\n"), prCfaAsf->u4MemDataLen, prCfaAsf->eCurCfaAsfAnaSt);*/

			prCfaAsf->pu1HdrBuf = (u8 *) prCfaAsf->ptrPfrMemAddress;
			prCfaAsf->u4HdrBufDataLen = prCfaAsf->u4MemDataLen;

			if (prCfaAsf->fgRealSyncPbbuf) {
				prCfaAsf->fgRealSyncPbbuf = FALSE;
				prCfaAsf->u4HdrBufDataLen += (u32) u8TxLen;
			}

			if (NULL == prCfaAsf->pu1HdrBuf)
			{
				DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] Invalid pu1HdrBuf pointer in CfaAsfTxDoneStCtrl().\r\n"));
				CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
				return;
			}
			prCfaAsf->fgTxData2HdrBuf = FALSE;

			if (0 == prCfaAsf->u4HdrBufDataLen) {
				DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,
					TEXT("CfaAsfTxDoneStCtrl()==========================================")
					TEXT("prCfaAsf->u4HdrBufDataLen == 0\n"));

				CfaAsfFinishPrs(pvSptHdl, prCfaAsf);

				return;
			}
		}

#if CFA_ASF_SUPPORT_VOD_DRM
		else if (prCfaAsf->fgDrmEncrypt == TRUE) {
			/*u32 u4Ret = 0;*/
			prCfaAsf->rCfaAsfCpsInfo.fgOn = FALSE;
			mrRet = Spt4CfaTurnCPS(pvSptHdl, &prCfaAsf->rCfaAsfCpsInfo);

			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[ASF CFA] call Spt4CfaTurnCPS() retun err(%d) in")
					TEXT("CfaAsfSyncBuf(),call FinishPrs: %x.\n"), mrRet, 0x05);
				CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
				return;
			}

			prCfaAsf->fgDrmEncrypt = FALSE;
		}
		else 
		{
			/*do nothing*/
		}

#endif

		switch (prCfaAsf->eCurCfaAsfAnaSt) {
		case CFA_ASF_ANA_ST_IDLE:
			CfaAsfAnaStIdle(pvSptHdl, u8TxLen, prCfaAsf);
			break;

		case CFA_ASF_ANA_ST_SEARCH_PACKET_HEADER:
			CfaAsfAnalyzePacketHeader(pvSptHdl, prCfaAsf);
			break;

		case CFA_ASF_ANA_ST_SEARCH_PAYLOAD_HEADER:
			CfaAsfAnalyzePayload(pvSptHdl, prCfaAsf);
			break;

		case CFA_ASF_ANA_ST_SEARCH_HEADER:
			CfaAsfSearchHeader(pvSptHdl, prCfaAsf,
				CFA_ASF_ANA_ST_SEARCH_PACKET_HEADER, 0, CFA_ASF_HDR_BUF_SZ, 0);
			break;

		case CFA_ASF_ANA_ST_PRS_COMPRESS_PAYLOAD:
			CfaAsfGetCompressPayloadLen(pvSptHdl, prCfaAsf);
			break;

		case CFA_ASF_ANA_ST_TX_VC1_SC_DONE:
			mrRet = CfaAsfTxStrmData2Fifo(prCfaAsf->rCfaAsfLastTxInfo.u8LastTxSa,
							  prCfaAsf->rCfaAsfLastTxInfo.u8LastTxLen, pvSptHdl, prCfaAsf);
			break;

#if CFA_ASF_SYNC_BUF_BEFORE_TX

		case CFA_ASF_ANA_ST_TX_PAYLOAD_DATA:
			mrRet = CfaAsfTxStrmData2Fifo(prCfaAsf->rCfaAsfLastTxInfo.u8LastTxSa,
							  prCfaAsf->rCfaAsfLastTxInfo.u8LastTxLen, pvSptHdl, prCfaAsf);
			break;
#endif
#if ASF_AAC_SUPPORT

		case CFA_ASF_ANA_ST_TX_A_HDR:
			mrRet = CfaAsfTxStrmData2Fifo(prCfaAsf->rCfaAsfLastTxInfo.u8LastTxSa,
							  prCfaAsf->rCfaAsfLastTxInfo.u8LastTxLen, pvSptHdl, prCfaAsf);
			break;

		case CFA_ASF_ANA_ST_TX_AAC_ADTS:
			mrRet = CfaAsfTxStrmData2Fifo(prCfaAsf->rCfaAsfLastTxInfo.u8LastTxSa,
							  prCfaAsf->rCfaAsfLastTxInfo.u8LastTxLen, pvSptHdl, prCfaAsf);
			break;
#endif

		default:
			DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] Invalid eCurCfaAsfAnaSt in CfaAsfTxDoneStCtrl().\n"));
			CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
			return;
		}
	} while (prCfaAsf->fgNoSyncPbb);

	if (RET_DMX_OK != mrRet)
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[ASF CFA] mrRet err(%d) in CfaAsfTxDoneStCtrl().\n"), mrRet);
		CfaAsfFinishPrs(pvSptHdl, prCfaAsf);
		return;
	}
}

