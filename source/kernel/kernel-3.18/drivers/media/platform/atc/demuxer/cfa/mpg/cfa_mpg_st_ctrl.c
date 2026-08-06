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

#include "cfa_mpg_st_ctrl.h"
#include "dmx_def.h"
#include "dmx_mem.h"
#include "cfa_macro.h"
#include "dmx_spt.h"

static void CfaMpgAdjustAudPts(CfaMpgInst *prCfaMpg)
{
	if (prCfaMpg->u8PrsPts < (u64)prCfaMpg->i8PtsResetAdValue)
		prCfaMpg->u8PrsPts = 0;
	else
		prCfaMpg->u8PrsPts = (u64)(prCfaMpg->u8PrsPts - prCfaMpg->i8PtsResetAdValue);
}

static void CfaMpgAdjustVidPts(CfaMpgInst *prCfaMpg)
{
	if (prCfaMpg->u8PrsPts >= (u64)prCfaMpg->i8PtsResetAdValue)
		prCfaMpg->u8PrsPts = (u64)(prCfaMpg->u8PrsPts - (u64)prCfaMpg->i8PtsResetAdValue);
	else
		prCfaMpg->u8PrsPts = INVALID_TIMESTAMP;
}

static CfaApiAudType CfaMpgGetAudType(u8 u1AudCodec)
{
	switch (u1AudCodec) {
	case 1:		/*< MPEG*/
		return CFA_AUD_DRV_FMT_MPEG;
	case 2:		/*< AC3*/
		return CFA_AUD_DRV_FMT_AC3;
	case 3:		/*< DTS*/
		return CFA_AUD_DRV_FMT_DTS;
	case 4:		/*< PCM*/
		return CFA_AUD_DRV_FMT_PCM;
	case 5:	/*< pure AAC*/
	case 6:		/*<	AAC*/
		return CFA_AUD_DRV_FMT_AAC;
	default:
		break;
	}

	return CFA_AUD_DRV_FMT_UNKNOWN;
}

static CfaApiAudType CfaMpgGetAudInfo(const CfaMpgInst *prCfaMpg)
{
	u32 u4Index = 0;

	for (u4Index = 0; u4Index < prCfaMpg->u4AstNs; u4Index++) {
		if (prCfaMpg->u2PrsAudStId == prCfaMpg->arAudInf[u4Index].u2DecAstId)
			return CfaMpgGetAudType(prCfaMpg->arAudInf[u4Index].ucDecAstType);
	}

	return CFA_AUD_DRV_FMT_UNKNOWN;
}

/*/ MPG CFA increment header buffer read pointer*/
/*/ @return None*/
/*< [IN] pointer to CfaMpgInst*/
/*< [IN] file length added to MPG CFA current parsing position*/
static void CfaMpgIncHdrBufRp(CfaMpgInst *prCfaMpg, u32 u4Len)
{
	if (u4Len >= prCfaMpg->u4RdyDataSz) {
		prCfaMpg->u4RdyDataSz = 0;
		prCfaMpg->pucHdrBufRp = NULL;
	} else {
		prCfaMpg->u4RdyDataSz -= u4Len;
		prCfaMpg->pucHdrBufRp += u4Len;
#if CFA_MPG_HIGH_BIT_RATE_HANDLE
		if (u4Len >= prCfaMpg->u4AvailSz) {
			DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("CFA_MPG, error!! u4Len >= prCfaMpg->u4AvailSz, u4AvailSz < 0\n"));
			prCfaMpg->u4AvailSz = 0;
		} else
			prCfaMpg->u4AvailSz  -= u4Len;

#endif
	}
}

void CfaMpgSTCSub(u64 u8Stc, u64 u8Value, u64 *pu8NewStc)
{
	u64 u8Delta = 0;
	s64 i8Delta = 0;

	u8Stc &= (u64)MASK_1F;
	u8Value &= (u64)MASK_1F;

	if (u8Value > u8Stc) {
		u8Delta = (u64) (u8Value - u8Stc);

		if (u8Delta < 0x100000000LL)
			i8Delta = -(s64)u8Delta;
		else {
			/* handle wrap around*/
			u8Delta = ((u64)MASK_1F - u8Delta);
			i8Delta = (s64) u8Delta;
		}
	} else {
		u8Delta = (u8Stc - u8Value);

		if (u8Delta < 0x100000000LL)
			i8Delta = (s64) u8Delta;

		else {
			/* handle wrap around*/
			u8Delta = ((u64)MASK_1F - u8Delta);
			i8Delta = -(s64) u8Delta;
		}
	}
	*pu8NewStc = (u64)i8Delta;
}


/* MPG CFA matches start code in start code match list*/
/* @return TRUE - Success, FALSE - Fail*/
/*< [IN] code to be match*/
/*< [IN] start code match list*/
/*< [IN] start code match list size*/
bool CfaMpgMatchSc(u8 ucSrcCode, const u8 *pucScMatchLst, u32 u4ScMatchLstSz)
{
	u32 i = 0;

	if (NULL == pucScMatchLst)
		return FALSE;

	for (i = 0; i < u4ScMatchLstSz; ++i) {
		if (ucSrcCode == pucScMatchLst[i])
			return TRUE;
	}

	return FALSE;
}


/* Search start code from (pucHdrBuf - 3) to (pucHdrBuf + u4DataSz - 4)*/
/* @return 0~(u4DataSz - 4)	->	Prefix starts from byte 0~(u4DataSz - 4) of pucHdrBuf.*/
/* DMX_INVALID_UINT32  -> Prefix doesn't exist in 0 ~ (u4DataSz - 3) of pucHdrBuf,
	nor does it exist in the last 3 bytes of previous data range.*/
/*< [IN] buffer that the space of 3 bytes previous than pucHdrBuf can also be used*/
/*< [IN] data size (counting from pucHdrBuf)*/
/*< [IN] start code match list*/
/*< [IN] size of start code match list*/
u32 CfaMpgScSearch(const u8 *pucHdrBuf, u32 u4DataSz, const u8 *pucScMatchLst,
			u32	u4ScMatchLstSz)
{
	u32 i = 0, j = 0;

	if ((!pucHdrBuf) || (!u4ScMatchLstSz) || (!pucScMatchLst) || ((u32)4 > u4DataSz))
		return DMX_INVALID_UINT32;

	i = 0;
	while (i <= u4DataSz - (u32)4) {
		if (*(pucHdrBuf + i) == 0x00) {
			for (j = i + (u32)1; (*(pucHdrBuf + j) == (u8)0x00) && (j <= (u4DataSz - (u32)3));
				 j++) {
				;
			}
			if (j >= i + (u32)2) {
				if ((*(pucHdrBuf + j) == (u8)0x01) &&
					(TRUE == CfaMpgMatchSc(*(pucHdrBuf + j + 1), (const u8 *)pucScMatchLst, u4ScMatchLstSz)))

					return (j - (u32)2);
			}
			i = j + (u32)1;
		} else
			i++;
	}

	return DMX_INVALID_UINT32;
}


/* Search vcd sync pattern 12 bytes 0x00FFFFFFFFFFFFFFFFFFFF00*/
/* @return 0~(u4DataSz - 16)		 -> Prefix starts from byte 0~(u4DataSz - 16) of pucHdrBuf.*/
/* DMX_INVALID_UINT32  -> Prefix doesn't exist in 0 ~ (u4DataSz - 15) of pucHdrBuf,
	nor does it exist in the last 15 bytes of previous data range.*/
/*< [IN] buffer that the space of 3 bytes previous than pucHdrBuf can also be used*/
/*< [IN] data size (counting from pucHdrBuf)*/
u32 CfaMpgVcdPatternSearch(const u8 *pucHdrBuf, u32 u4DataSz)
{
	u32 i = 0, j = 0;
	bool fgMatch11 = FALSE;

	if ((!pucHdrBuf) || ((u32)16 > u4DataSz))
		return DMX_INVALID_UINT32;

	i = 0;
	while (i <= u4DataSz - (u32)12) {
		if (*(pucHdrBuf + i) == 0x00) {
			for (j = i + (u32)1; j <= i + (u32)10; j++) {
				if (*(pucHdrBuf + j) == 0xFF) {
					if (j == i + (u32)10)
						fgMatch11 = TRUE;
				} else
					break;
			}

			j = i + (u32)11;
			if ((*(pucHdrBuf + j) == (u8)0x00) && fgMatch11)
				return i;
			i++;
		} else
			i++;
	}

	return DMX_INVALID_UINT32;
}


/*/ MPG CFA increases current parsing position*/
/*/ @return TRUE - meet range end, FALSE - not meet range end*/
/*< [IN] pointer to CfaMpgInst*/
/*< [IN] file length added to MPG CFA current parsing position*/
bool CfaMpgIncPrsPos(CfaMpgInst *prCfaMpg, u64 u8Len)
{
	prCfaMpg->u8Ca += u8Len;

	/*if (prCfaMpg->u8Ca > prCfaMpg->rCfaRange.u8Ea)*/
	if (prCfaMpg->u8Ca >= prCfaMpg->rCfaRange.u8Ea) {
		DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("[CFA MPG] Parsing End, Ca = %d, Ea = %d"),
			(u32)(prCfaMpg->u8Ca),
			(u32)(prCfaMpg->rCfaRange.u8Ea));
		prCfaMpg->u8Ca = DMX_INVALID_UINT64;
		prCfaMpg->fgDealDxEr = TRUE;
		return TRUE;
	} else
		return FALSE;
}

/*/ MPG CFA increases current parsing position and header buffer read pointer*/
/*/ @return None*/
void CfaMpgIncPrsPosAndHdrBufRp(CfaMpgInst *prCfaMpg, u64 u8Len)
{
	CfaMpgIncPrsPos(prCfaMpg, u8Len);
	CfaMpgIncHdrBufRp(prCfaMpg, (u32)u8Len);
}


/*/ MPG CFA get available transfer length*/
/*/ @return available transfer length*/
static u64 CfaMpgGetAvaTxLen(const CfaMpgInst *prCfaMpg, u64 u8TxLen)
{
	u64 u8Ca = 0;

	if (0 < u8TxLen) {
		/*/ fix BDP00115649*/
		u8Ca = prCfaMpg->u8Ca + u8TxLen - (u64)prCfaMpg->u2SkipLen - (u64)1;
	} else
		return 0;

	if (prCfaMpg->rCfaRange.u8Ea >= u8Ca)
		return u8TxLen;

	else
		return u8TxLen - (u8Ca - prCfaMpg->rCfaRange.u8Ea);
}


/*/ Notify inquiry infomation is available*/
/*/ @return s32 error code*/
/*/< [IN] Splitter Handle*/
/*/< [IN] Inquiry type, TODO: remove it by put into pvInqInf*/
/*/< [IN] Inquiry private data sent to Splitter's User (Mainly for MPC)*/
/*/< [IN] pointer to CfaMpgInst*/
static MRESULT CfaMpgNotifyInq(void *pvSptHdl, u32 u4InfType, const void *pvInqInf,
							CfaMpgInst *prCfaMpg)
{
	switch (u4InfType) {
	case CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_SC_INF:
		VERIFY(RET_DMX_OK == dmx_sema_lock(prCfaMpg->hMutex, DMX_SEMA_OPTION_WAIT));
		mm_memcpy(&prCfaMpg->rFirstMpgScInf, pvInqInf,
			sizeof(prCfaMpg->rFirstMpgScInf));
		VERIFY(RET_DMX_OK == dmx_sema_unlock(prCfaMpg->hMutex));
		break;

	case CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_PTS_INF:
		VERIFY(RET_DMX_OK == dmx_sema_lock(prCfaMpg->hMutex, DMX_SEMA_OPTION_WAIT));
		mm_memcpy(&prCfaMpg->rFirstMpgPtsInf, pvInqInf,
			sizeof(prCfaMpg->rFirstMpgPtsInf));
		VERIFY(RET_DMX_OK == dmx_sema_unlock(prCfaMpg->hMutex));
		break;

	case CFA_MPG_QUERY_INF_TYPE_STRM_INF:
		VERIFY(RET_DMX_OK == dmx_sema_lock(prCfaMpg->hMutex, DMX_SEMA_OPTION_WAIT));
		#ifdef MM_ATE_CHECK
		mm_memcpy(&(prCfaMpg->rStrmInf.u4AudStrmNs),
			&(((CfaMpgQIStrmInf *)pvInqInf)->u4AudStrmNs),
		sizeof(prCfaMpg->rStrmInf) - 2 * sizeof(u32));
		#else
		mm_memcpy(&prCfaMpg->rStrmInf, pvInqInf, sizeof(prCfaMpg->rStrmInf));
		#endif
		VERIFY(RET_DMX_OK == dmx_sema_unlock(prCfaMpg->hMutex));
		break;

	case CFA_MPG_QUERY_INF_TYPE_AUTO_PAUSE_INF:
		VERIFY(RET_DMX_OK == dmx_sema_lock(prCfaMpg->hMutex, DMX_SEMA_OPTION_WAIT));
		mm_memcpy(&prCfaMpg->rAutoPauseInf, pvInqInf, sizeof(prCfaMpg->rAutoPauseInf));
		VERIFY(RET_DMX_OK == dmx_sema_unlock(prCfaMpg->hMutex));
		break;

	case CFA_MPG_QUERY_INF_TYPE_MUX_RATE_INF:
		VERIFY(RET_DMX_OK == dmx_sema_lock(prCfaMpg->hMutex, DMX_SEMA_OPTION_WAIT));
		mm_memcpy(&prCfaMpg->rMuxRateInf, pvInqInf, sizeof(prCfaMpg->rMuxRateInf));
		VERIFY(RET_DMX_OK == dmx_sema_unlock(prCfaMpg->hMutex));
		break;

	case CFA_MPG_QUERY_INF_TYPE_LAST_MPEG_PTS_INF:
		VERIFY(RET_DMX_OK == dmx_sema_lock(prCfaMpg->hMutex, DMX_SEMA_OPTION_WAIT));
		mm_memcpy(&prCfaMpg->rLastMpgPtsInf, pvInqInf,
			sizeof(prCfaMpg->rLastMpgPtsInf));
		VERIFY(RET_DMX_OK == dmx_sema_unlock(prCfaMpg->hMutex));
		break;

	default:
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	/*return Spt4CfaInqInfNotify(pvSptHdl, u4InfType);*/
	MM_RETURN(RET_DMX_OK);
}


#if CFA_MPG_HIGH_BIT_RATE_HANDLE
/*/ MPG CFA processes CfaMpgCmdQTxing*/
/*/ @return None*/
void CfaMpgCmdQTxing(void *pvSptHdl, CfaMpgInst *prCfaMpg)
{
	MRESULT mrRet = RET_DMX_OK;

	prCfaMpg->rCfaVidInf.u8FileOfst = prCfaMpg->u8FirstOffset;
	prCfaMpg->rCfaVidInf.eTxMode = CFA_PTM_EXACT_POS;
	prCfaMpg->rCfaVidInf.eVidType = CFA_VID_MPEG2;
	prCfaMpg->rCfaVidInf.u8Len = prCfaMpg->u8PreTxEndOff - prCfaMpg->rCfaVidInf.u8FileOfst;
	prCfaMpg->rCfaVidInf.u4PrsStrmId = (u32)((u32)prCfaMpg->ucPrsVidStId << 8);
	prCfaMpg->rCfaVidInf.fgUseCmdQ = TRUE;
	prCfaMpg->rCfaVidInf.u2TxEntryCnt = prCfaMpg->u2CmdQIndex;
	prCfaMpg->rCfaVidInf.u8RealTxLen = prCfaMpg->u8DataInBuf;
	prCfaMpg->rCfaVidInf.fgDummyAU = FALSE;
	prCfaMpg->rCfaVidInf.fgUnitStart = FALSE;
	prCfaMpg->rCfaVidInf.fgUnitEnd = FALSE;
	prCfaMpg->rCfaVidInf.u8TotalAULen = 0;
	prCfaMpg->rCfaVidInf.fgAUCompleteByEnd = FALSE;

	prCfaMpg->u2CmdQIndex = 0;
	prCfaMpg->u8DataInBuf = 0;

	if (prCfaMpg->u4AvailSz < PSR_RESERVE_FIFO_SPACE)
		prCfaMpg->fgTxByPbbuf = TRUE;

	prCfaMpg->fgExitTxDoneCtrl = TRUE;
	prCfaMpg->fgTxData2FIFO = TRUE;
	prCfaMpg->fgEverTx = TRUE;
	prCfaMpg->u8CfaIssueTxLen = prCfaMpg->rCfaVidInf.u8Len;
	prCfaMpg->u8LatTxAvaLen = 0;
	prCfaMpg->fgCMDQTx = TRUE;
	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
		TEXT("<CFA> CfaMpgCmdQTxing rVidInf.u8FileOfst 0x%lx\r\n"),
		prCfaMpg->rCfaVidInf.u8FileOfst);
	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
		TEXT("<CFA> CfaMpgCmdQTxing rVidInf.u8Len 0x%lx\r\n"),
		prCfaMpg->rCfaVidInf.u8Len);
	mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &prCfaMpg->rCfaVidInf);
	if (mrRet != RET_DMX_OK) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("CfaMpgCmdQTxing(): tx data to fifo error\n"));
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
		return;
	}

}
#endif

/*/ MPG CFA found last MPEG PTS information*/
/*/ @return None*/
/*/ @note This API stops parsing process*/
static void CfaMpgQIFoundLastMpegPtsInf(void *pvSptHdl, CfaMpgInst *prCfaMpg)
{
	CfaMpgQIFirstMpgPtsInf rQILastMpgPts;

	mm_memset(&rQILastMpgPts, 0, sizeof(CfaMpgQIFirstMpgPtsInf));

	rQILastMpgPts.u8Pts = prCfaMpg->u8LastPts;
	rQILastMpgPts.u8Pos = prCfaMpg->u8LastPtsAddr;
	rQILastMpgPts.u2CurPrsPktType = prCfaMpg->eCurPrsPktType;
	CfaMpgNotifyInq(pvSptHdl, CFA_MPG_QUERY_INF_TYPE_LAST_MPEG_PTS_INF, &rQILastMpgPts, prCfaMpg);

	/* When CFA_MPG_QUERY_INF_TYPE_LAST_MPEG_PTS_INF is got, turn it off*/
	prCfaMpg->u4CurQueryInfType &= (~CFA_MPG_QUERY_INF_TYPE_LAST_MPEG_PTS_INF);
}

#if CFA_MPG_HIGH_BIT_RATE_HANDLE
static void CfaMpgAnaStPktTxingDP(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
	if (CFA_MPG_SYS_STRM_TYPE_VIDEO_SEQUENCE == prCfaMpg->eCfaMpgSysStrmType) {
		/* MPEG video stream only*/
		CfaMpgIncPrsPos(prCfaMpg, u8TxLen);

		/* finish current parsing*/
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
	} else {   /* MPEG stream*/
		prCfaMpg->u8PrsPrevPts = prCfaMpg->u8PrsPts;
		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u8TxLen - prCfaMpg->u2SkipLen));
		prCfaMpg->u2SkipLen = 0;
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
	}
}
#endif

/*/ CFA Mpg transfer next header information*/
/*/ @return None*/
/*/ @note This API should also check if it is in CFA_MPG_QUERY_INF_TYPE_STRM_INF*/
void CfaMpgFinishPrs(void *pvSptHdl, CfaMpgInst *prCfaMpg)
{
	u32 u4Index = 0;

	MMATE_CHECK_POINTER(prCfaMpg);
	MMATE_CHECK_STRUCT(prCfaMpg->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaMpg->rStrmInf);
	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
		TEXT("<CFA MPG> finish parse!\r\n"));

#if CFA_MPG_HIGH_BIT_RATE_HANDLE
	if ((prCfaMpg->u2CmdQIndex != 0) && (prCfaMpg->fgSupportHibitRt)) {
		CfaMpgCmdQTxing(pvSptHdl, prCfaMpg);
		return;
	}
#endif

	if (CFA_MPG_QUERY_INF_TYPE_STRM_INF & prCfaMpg->u4CurQueryInfType) {
		/* query stream type information*/
		u32 u4Cnt = 0;
		u32 i = 0;
		CfaMpgQIStrmInf rCfaMpgQIStrmInf;

		mm_memset(&rCfaMpgQIStrmInf, 0, sizeof(rCfaMpgQIStrmInf));

		u4Cnt = MIN(prCfaMpg->u4AstNs, CFA_MPG_QUERY_INF_MAX_STRM_NS);
		if (0 != u4Cnt) {
			rCfaMpgQIStrmInf.u4AudStrmNs = u4Cnt;
			for (i = 0; i < u4Cnt; i++) {
				rCfaMpgQIStrmInf.au2DecAstId[i] = prCfaMpg->arAudInf[i].u2DecAstId;
				rCfaMpgQIStrmInf.aucDecAstType[i] = prCfaMpg->arAudInf[i].ucDecAstType;
				rCfaMpgQIStrmInf.aucDecAstAtt[i] = prCfaMpg->arAudInf[i].ucDecAstAtt;
			}
		}

		u4Cnt = MIN(prCfaMpg->u4SpstNs, CFA_MPG_QUERY_INF_MAX_STRM_NS);
		if (0 != u4Cnt) {
			rCfaMpgQIStrmInf.u4SpStrmNs  = u4Cnt;
			for (i = 0; i < u4Cnt; i++)
				rCfaMpgQIStrmInf.aucDecSpstId[i] = prCfaMpg->arSPInf[i].ucDecSpstId;
		}

		u4Cnt = MIN(prCfaMpg->u4VstNs, CFA_MPG_QUERY_INF_MAX_STRM_NS);
		if (0 != u4Cnt) {
			rCfaMpgQIStrmInf.u4VidStrmNs  = u4Cnt;
			for (i = 0; i < u4Cnt; i++) {
				/*change by lqq for add video type*/
				rCfaMpgQIStrmInf.aucDecVstId[i] = prCfaMpg->aucDecVstId[i];
				rCfaMpgQIStrmInf.aucDecVstType[i] = prCfaMpg->aucDecVstType[i];
			}
		}
#if CFA_MPG_HIGH_BIT_RATE_HANDLE
		rCfaMpgQIStrmInf.u4BitRate = prCfaMpg->u4BitRate;
#endif
		CfaMpgNotifyInq(pvSptHdl, CFA_MPG_QUERY_INF_TYPE_STRM_INF, &rCfaMpgQIStrmInf, prCfaMpg);
		prCfaMpg->u4CurQueryInfType &= (~CFA_MPG_QUERY_INF_TYPE_STRM_INF);
	}

	/* For BDP00116658, add last pts inquery*/
	if ((CFA_MPG_QUERY_INF_TYPE_LAST_MPEG_PTS_INF & prCfaMpg->u4CurQueryInfType) &&
		(prCfaMpg->u8LastPts != INVALID_TIMESTAMP))
		CfaMpgQIFoundLastMpegPtsInf(pvSptHdl, prCfaMpg);

	/*/ VOBU still pause case*/
#if (CFA_MPG_SUPPORT_DVD_VOBU_STILL_AUTOPAUSE)
	if ((prCfaMpg->fgEverTx) && (prCfaMpg->rCfaRange.fgVobuStill)) {
		Cfa2PsrStrmInfo rAutoPauseStrmInfo = {0};

		if (CFA_MPG_PRS_BIT_STRM_TYPE_V & prCfaMpg->u4CurPrsFlg) {
			u32 i = 0;

			rAutoPauseStrmInfo.u4VstrmNs = prCfaMpg->u4VstNs;

			for (i = 0; i < prCfaMpg->u4VstNs && i < 2; ++i)
				rAutoPauseStrmInfo.ucDecVidStId[i] = prCfaMpg->aucDecVstId[i];
		}
		rAutoPauseStrmInfo.fgVobuStill = TRUE;
		Spt4CfaAutoPause(pvSptHdl, (void *)&rAutoPauseStrmInfo);
	}
#endif
	prCfaMpg->eDbgCurCfaMpgAnaSt = prCfaMpg->eCurCfaMpgAnaSt;
	prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_IDLE;
	prCfaMpg->fgExitTxDoneCtrl = TRUE;
	/*for save last SPUinfo*/
	for (u4Index = 0; u4Index < prCfaMpg->u4SpstNs; u4Index++) {
		mm_memcpy((void *)(&(prCfaMpg->arLastSPInf[u4Index])),
			(void *)(&(prCfaMpg->arSPInf[u4Index])), sizeof(CFAMPG_SP_INF));
	}
	prCfaMpg->u4LastSpstNs = prCfaMpg->u4SpstNs;

	if (prCfaMpg->fgDemuxError) {
		Cfa2PsrStrmInfo rPathStrmInfo = {0};
		/* patch for audio only get IBC before WrIdx move.*/
		if (CFA_MPG_PRS_BIT_STRM_TYPE_V & prCfaMpg->u4CurPrsFlg) {
			u32 i = 0;

			rPathStrmInfo.u4VstrmNs = 1;

			for (i = 0; i < rPathStrmInfo.u4VstrmNs; ++i)
				rPathStrmInfo.ucDecVidStId[i] = prCfaMpg->ucDecVidStId;
		}

		if (CFA_MPG_PRS_BIT_STRM_TYPE_A & prCfaMpg->u4CurPrsFlg) {
			u32 j = 0;

			rPathStrmInfo.u4AstrmNs = 1;

			for (j = 0; j < rPathStrmInfo.u4AstrmNs; ++j)
				rPathStrmInfo.u2DecAudStId[j] = prCfaMpg->u2DecAudStId;
		}

		DmxLogT(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("<CFA_MPG>ERRORCHUNK: CfaMpgFinishPrs Set eos u8Ca 0x%llx\r\n"), prCfaMpg->u8Ca);
		prCfaMpg->fgDemuxError = FALSE;
		if (DMX_IS_RW_PLAY(pvSptHdl))
			Spt4CfaFinishedEx(pvSptHdl, prCfaMpg->u8Ca, FALSE, (u32)GAU_E_ERRCHUNK);

		else
			Spt4CfaFinishedEx(pvSptHdl, prCfaMpg->u8Ca, TRUE, (u32)GAU_E_ERRCHUNK);
	} else {
		/* Workaround for No audio after seeking and No EOS issue by jie.tang*/
		if (DMX_IS_FF_PLAY(pvSptHdl))
			Spt4CfaFinishedEx(pvSptHdl, (prCfaMpg->rCfaRange.u8Ea-(u64)2020), TRUE, (u32)GAU_E_EOS);
		else
			Spt4CfaFinishedEx(pvSptHdl, (prCfaMpg->rCfaRange.u8Ea-(u64)2020), FALSE, (u32)GAU_E_EOS);
	}
}

static void CfaMpgTxNextVidStmDataToVFifo(void *pvSptHdl, CfaMpgInst *prCfaMpg,
			u64 u8TxAvaLen, u64 u8Sa)
{
	MRESULT  mrRet = RET_DMX_OK;

	if (prCfaMpg->u2CmdQIndex == 0) {
		prCfaMpg->u8PreTxEndOff = u8Sa;
		prCfaMpg->u8FirstOffset = u8Sa;
	}

	if ((prCfaMpg->u8DataInBuf >
		(prCfaMpg->u8HiBiBuSize - (u64)2 * (u64)2048 - u8TxAvaLen)) ||
		(prCfaMpg->u8Ca + PSR_RESERVE_FIFO_SPACE > prCfaMpg->rCfaRange.u8Ea) ||
		(prCfaMpg->u4AvailSz < PSR_RESERVE_FIFO_SPACE)
		|| (prCfaMpg->u2CmdQIndex == DMX_MAX_TX_CNT_FOR_CMD_Q - 1)
		|| ((prCfaMpg->u8Ca + u8TxAvaLen - prCfaMpg->u8FirstOffset) > (u64)DMX_VID_TX_MAX_SIZE)) {

		u64 u8OldTxAvaLen = u8TxAvaLen;

		prCfaMpg->fgIsNoBeyondMaxTx = FALSE;

		if ((prCfaMpg->u8Ca - prCfaMpg->u8FirstOffset + u8TxAvaLen) < (u64)DMX_VID_TX_MAX_SIZE) {
			prCfaMpg->rCfaVidInf.parCmdQTxEntry[prCfaMpg->u2CmdQIndex].u4TxOfst
				 = (u32)(u8Sa - prCfaMpg->u8PreTxEndOff);
			prCfaMpg->rCfaVidInf.parCmdQTxEntry[prCfaMpg->u2CmdQIndex].u4TxLen =
				(u32)u8TxAvaLen;
			prCfaMpg->u8PreTxEndOff = u8Sa + u8TxAvaLen;

			prCfaMpg->u2CmdQIndex++;
			prCfaMpg->u8DataInBuf += u8TxAvaLen;
			prCfaMpg->fgIsNoBeyondMaxTx = TRUE;
		}

		prCfaMpg->rCfaVidInf.u8FileOfst = prCfaMpg->u8FirstOffset;
		prCfaMpg->rCfaVidInf.eTxMode = CFA_PTM_EXACT_POS;
		prCfaMpg->rCfaVidInf.eVidType = CFA_VID_MPEG2;

		#if CFA_MPG_SUPPORT_AVC
		if ((prCfaMpg->eCfaMpgMediumType == CFA_MPG_MED_TYPE_FILE)
			&& (prCfaMpg->u2StreamVideoType == AVCODEC_ID_H264))
				prCfaMpg->rCfaVidInf.eVidType = CFA_VID_H264;

		#endif
		if ((prCfaMpg->u8Ca - prCfaMpg->u8FirstOffset + u8TxAvaLen) < (u64)DMX_VID_TX_MAX_SIZE) {
			prCfaMpg->rCfaVidInf.u8Len =
			prCfaMpg->u8Ca - prCfaMpg->rCfaVidInf.u8FileOfst + u8TxAvaLen;
		} else {
			prCfaMpg->rCfaVidInf.u8Len =
			prCfaMpg->u8PreTxEndOff - prCfaMpg->rCfaVidInf.u8FileOfst;
		}

		prCfaMpg->rCfaVidInf.u4PrsStrmId =
			(u32)((u32)prCfaMpg->ucPrsVidStId << 8);
		prCfaMpg->rCfaVidInf.fgUseCmdQ = TRUE;
		prCfaMpg->rCfaVidInf.u2TxEntryCnt = prCfaMpg->u2CmdQIndex;
		prCfaMpg->rCfaVidInf.u8RealTxLen = prCfaMpg->u8DataInBuf;
		prCfaMpg->rCfaVidInf.fgUnitStart = FALSE;
		prCfaMpg->rCfaVidInf.fgAUCompleteByEnd = FALSE;

		DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("<CFA> CfaMpgTxNextStrmDataToFifo_0")
			TEXT("rVidInf.u8FileOfst 0x%lx\r\n"),
			prCfaMpg->rCfaVidInf.u8FileOfst);
		mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &prCfaMpg->rCfaVidInf);
		if (mrRet != RET_DMX_OK) {
			CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
			return;
		}
		u8TxAvaLen = prCfaMpg->rCfaVidInf.u8Len;
		prCfaMpg->fgCMDQTx = TRUE;
		prCfaMpg->u2CmdQIndex = 0;
		prCfaMpg->u8DataInBuf = 0;

		if ((prCfaMpg->u4PtsArrayIndex < 50) && (prCfaMpg->fgIsNoBeyondMaxTx)) {
			prCfaMpg->au8FileOffSet[prCfaMpg->u4PtsArrayIndex] = prCfaMpg->u8Ca;
			prCfaMpg->au8FileOffSetPts[prCfaMpg->u4PtsArrayIndex] = prCfaMpg->u8PrsPts;
			prCfaMpg->u4PtsArrayIndex++;
		}
		prCfaMpg->fgTxData2FIFO = TRUE;
		if (prCfaMpg->u4AvailSz < PSR_RESERVE_FIFO_SPACE)
			prCfaMpg->fgTxByPbbuf = TRUE;

		if (!prCfaMpg->fgIsNoBeyondMaxTx) {
			if (prCfaMpg->u2CmdQIndex == 0) {
				prCfaMpg->u8PreTxEndOff = u8Sa;
				prCfaMpg->u8FirstOffset = u8Sa;
			}
			(prCfaMpg->rCfaVidInf.parCmdQTxEntry + prCfaMpg->u2CmdQIndex)->u4TxOfst =
				(u32)(u8Sa - prCfaMpg->u8PreTxEndOff);
			(prCfaMpg->rCfaVidInf.parCmdQTxEntry + prCfaMpg->u2CmdQIndex)->u4TxLen	=
				(u32)u8OldTxAvaLen;
			prCfaMpg->u8PreTxEndOff = u8Sa + u8OldTxAvaLen;

			prCfaMpg->u8DataInBuf += u8OldTxAvaLen;
			prCfaMpg->u2CmdQIndex++;
			prCfaMpg->u8FileOffSet = prCfaMpg->u8Ca;
			prCfaMpg->u8FileOffSetPts = prCfaMpg->u8PrsPts;
		}
	} else {
		(prCfaMpg->rCfaVidInf.parCmdQTxEntry + prCfaMpg->u2CmdQIndex)->u4TxOfst =
			(u32)(u8Sa - prCfaMpg->u8PreTxEndOff);
		(prCfaMpg->rCfaVidInf.parCmdQTxEntry + prCfaMpg->u2CmdQIndex)->u4TxLen =
			(u32)u8TxAvaLen;
		prCfaMpg->u8PreTxEndOff = u8Sa + u8TxAvaLen;
		prCfaMpg->u8DataInBuf += u8TxAvaLen;
		prCfaMpg->u2CmdQIndex++;
		if (prCfaMpg->u4PtsArrayIndex < 50) {
			prCfaMpg->au8FileOffSet[prCfaMpg->u4PtsArrayIndex] = prCfaMpg->u8Ca;
			prCfaMpg->au8FileOffSetPts[prCfaMpg->u4PtsArrayIndex] = prCfaMpg->u8PrsPts;
			prCfaMpg->u4PtsArrayIndex++;
		}
	}
}

/* CFA Mpg transfer next stream data to FIFO*/
/* @return None*/
/* @note 1. If file offset is over transfer range,
	MPG CFA state is changed to CFA_MPG_ANA_ST_IDLE and finish the parsing*/
/*			 2. If transfer length make file offset over transfer range,
	MPG CFA state is changed to CFA_MPG_ANA_ST_IDLE and finish the parsing*/
/*			 3. Replace vMpsPrsNextP()*/
/*< [IN] handle of fdmx*/
/*< [IN] pointer to CfaMpgInst*/
/*< [IN] advance file length before transferring next bitstrema data*/
/*< [IN] transfer length*/
void CfaMpgTxNextStrmDataToFifo(void *pvSptHdl, CfaMpgInst *prCfaMpg, u64 u8AdvLen,
						u64 u8TxLen)
{
	u64 u8Sa = 0, u8TxAvaLen = 0;
	bool   fgSkipDataTx = FALSE;
#if CFA_MPG_HIGH_BIT_RATE_HANDLE
	CFA_CPS_INFO_T rCPS;

	mm_memset(&rCPS, 0, sizeof(CFA_CPS_INFO_T));
#endif
	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
		TEXT("Enter into CfaMpgTxNextStrmDataToFifo: Prs Type %d\r\n"), prCfaMpg->eCurPrsPktType);

	/* check if all data are parsed*/
	if (TRUE == CfaMpgIncPrsPos(prCfaMpg, u8AdvLen)) {
		/* finish current parsing*/
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
	} else {
		CfaMpgIncHdrBufRp(prCfaMpg, (u32)u8AdvLen);
		/* transfer header information*/
		u8TxAvaLen = CfaMpgGetAvaTxLen(prCfaMpg, u8TxLen);
		if ((0 != u8TxAvaLen) && (u8TxAvaLen == u8TxLen)) {
			CFA_AUDIO_INFO_T rAudInf = {0};
			CFA_VIDEO_INFO_T rVidInf = {0};
			CFA_SUBPIC_INFO_T rSpInf = {0};
			u64 u8PrsSpStId = 0;
			u8  u1CurSpIdx = 0;
			u8  u1Idx = 0;
			u32 u4Cnt = 0;
			MRESULT  mrRet = RET_DMX_OK;

			u8Sa = prCfaMpg->u8Ca;
			if(DMX_INVALID_UINT64 == u8Sa)
			{
			    CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
				return;
			}

			if (((prCfaMpg->fgIsCSSDecOn) || (prCfaMpg->fgIsCPRMDecOn))
				&& (prCfaMpg->eCurPrsPktType != CFA_MPG_PRS_BIT_STRM_TYPE_NV)) {
				CFA_CPS_INFO_T rTmpCPS = {0};
				/*rCPS*/

				rTmpCPS.fgOn = TRUE;
				rTmpCPS.u8Offset = prCfaMpg->u8PckPos;
				if (prCfaMpg->fgIsCPRMDecOn) {
					rTmpCPS.u8DCI_CCI	= prCfaMpg->u8DCI_CCI;
					rTmpCPS.u8DCI_CCI_VERIFY = prCfaMpg->u8DCI_CCI_Verify;
					rTmpCPS.pu8DTKC	= &(prCfaMpg->u8DTKC);
				}
				Spt4CfaTurnCPS(pvSptHdl, &rTmpCPS);
			}

			if ((prCfaMpg->fgExistSCR) && (prCfaMpg->fgFillAU)) {
				Spt4CfaSCRNotify(pvSptHdl, prCfaMpg->u8SCR);
				prCfaMpg->fgExistSCR = FALSE;
				prCfaMpg->fgFillAU = FALSE;
			}


			switch (prCfaMpg->eCurPrsPktType) {
			case CFA_MPG_PRS_BIT_STRM_TYPE_V:
				if ((prCfaMpg->rCfaRange.u8VidSa != CFA_MPG_INVALID_RANGE_START_ADDRESS) &&
					(u8Sa < prCfaMpg->rCfaRange.u8VidSa)) {
					fgSkipDataTx = TRUE;
					break;
				}
#if CFA_MPG_HIGH_BIT_RATE_HANDLE
				prCfaMpg->u8LatTxAvaLen = u8TxAvaLen;
				if ((u8TxAvaLen <= prCfaMpg->u4RdyDataSz) &&
					(prCfaMpg->fgSupportHibitRt) && (!prCfaMpg->fgTxByPbbuf)) {
					CfaMpgTxNextVidStmDataToVFifo(pvSptHdl, prCfaMpg, u8TxAvaLen, u8Sa);
				} else {
#endif
					rVidInf.u8FileOfst = u8Sa;
					rVidInf.eTxMode = CFA_PTM_EXACT_POS;
					rVidInf.eVidType = CFA_VID_MPEG2;

#if CFA_MPG_SUPPORT_AVC
					if ((prCfaMpg->eCfaMpgMediumType == CFA_MPG_MED_TYPE_FILE)
						&& (prCfaMpg->u2StreamVideoType == AVCODEC_ID_H264))
						rVidInf.eVidType = CFA_VID_H264;

#endif
					rVidInf.u8Len = u8TxAvaLen;
					rVidInf.u4PrsStrmId = (u32)((u32)prCfaMpg->ucPrsVidStId << 8);

					DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
						TEXT("<CFA> CfaMpgTxNextStrmDataToFifo_1 rVidInf.u8FileOfst 0x%lx\r\n"),
						rVidInf.u8FileOfst);
					DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
						TEXT("<CFA> CfaMpgTxNextStrmDataToFifo_1 rVidInf.u8Len	0x%lx\r\n"),
						rVidInf.u8Len);
					mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rVidInf);
					if (mrRet != RET_DMX_OK) {
						DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
							TEXT("CfaMpgVideoPckTxing():poddy-- tx data to fifo error\n"));
						CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
						return;
					}
#if CFA_MPG_HIGH_BIT_RATE_HANDLE
					if (prCfaMpg->fgSupportHibitRt)
						prCfaMpg->fgTxData2FIFO = TRUE;
					if ((prCfaMpg->fgSupportHibitRt) &&
						(prCfaMpg->u4AvailSz > PSR_RESERVE_FIFO_SPACE))
						prCfaMpg->fgTxByPbbuf = FALSE;
#endif

#if CFA_MPG_HIGH_BIT_RATE_HANDLE
				}
#endif
			break;

			case CFA_MPG_PRS_BIT_STRM_TYPE_A: {

					if ((prCfaMpg->rCfaRange.u8AudSa !=
						CFA_MPG_INVALID_RANGE_START_ADDRESS) &&
						(u8Sa < prCfaMpg->rCfaRange.u8AudSa)) {
						fgSkipDataTx = TRUE;
						break;
					}

					rAudInf.u8Pts = prCfaMpg->u8PrsPts + prCfaMpg->i8DeltaPts;
					if (rAudInf.u8Pts < (prCfaMpg->rCfaRange.u8SeekTime / 10000) * 90) {
						fgSkipDataTx = TRUE;
						DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
							TEXT("<CFA MPG> %s line %d, Audio AU PTS:%lld(1/90000s)")
							TEXT("< SeekTime:%lld(1/90000s),skip this data!\r\n"),
							__func__, __LINE__, rAudInf.u8Pts, (prCfaMpg->rCfaRange.u8SeekTime / 10000) * 90);
						break;
					}

					rAudInf.u8FileOfst = u8Sa;
					rAudInf.u8Len = u8TxAvaLen;
					rAudInf.fgUnitStart = FALSE;
					rAudInf.u4PrsStrmId = (u32)prCfaMpg->u2PrsAudStId;

					rAudInf.eAudType = CfaMpgGetAudInfo(prCfaMpg);
					rAudInf.u8TotalAULen = 0;
					mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rAudInf);
					DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
						TEXT("<CFA> CfaMpgTxNextStrmDataToFifo")
						TEXT("rAudInf.u8FileOfst 0x%lx\r\n"),
						rAudInf.u8FileOfst);
					DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
						TEXT("<CFA> CfaMpgTxNextStrmDataToFifo")
						TEXT("rAudInf.u8Len  0x%lx\r\n"),
						rAudInf.u8Len);
					if (mrRet != RET_DMX_OK) {
						DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
							TEXT("CfaMpgVideoPckTxing():")
							TEXT("poddy--1--tx data to fifo error\n"));
						CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
						return;
					}
				}
				break;
			case CFA_MPG_PRS_BIT_STRM_TYPE_SP0:
				if ((prCfaMpg->rCfaRange.u8SPSa !=
					CFA_MPG_INVALID_RANGE_START_ADDRESS) && (u8Sa < prCfaMpg->rCfaRange.u8SPSa)) {
					fgSkipDataTx = TRUE;
					break;
				}

				rSpInf.u8FileOfst = u8Sa;
				rSpInf.fgUnitStart = (prCfaMpg->u8PrsPts != INVALID_TIMESTAMP);
				rSpInf.u8Pts	  = prCfaMpg->u8PrsPts;
				rSpInf.u8EndPts   = 0;
				rSpInf.u4SpuPos   = 0;
				u4Cnt =
					(prCfaMpg->u4SpstNs > CFA_MPG_MAX_STRM_NS) ?
						(CFA_MPG_MAX_STRM_NS) : (prCfaMpg->u4SpstNs);

				for (u1Idx = 0; u1Idx < u4Cnt; u1Idx++) {
					if (prCfaMpg->ucPrsSpStId == prCfaMpg->arSPInf[u1Idx].ucDecSpstId)
						u1CurSpIdx = u1Idx;
				}
				/* fix zero-length bug(BDP00006368), ccma 080325*/
				if ((u8TxAvaLen > prCfaMpg->arSPInf[u1CurSpIdx].u2SpuSz) &&
					(prCfaMpg->arSPInf[u1CurSpIdx].u2SpuSz != 0))
					u8TxAvaLen = prCfaMpg->arSPInf[u1CurSpIdx].u2SpuSz;

				/* BDP117260 @pingzhao, 2008/10/31*/
				if (rSpInf.fgUnitStart) {
					prCfaMpg->arSPInf[u1CurSpIdx].u2SpuTotalSz =
						prCfaMpg->arSPInf[u1CurSpIdx].u2SpuSz;
					prCfaMpg->u4SubPicUNum++;
				}
				rSpInf.u8Len = u8TxAvaLen;
				u8PrsSpStId = prCfaMpg->u8DecSpStId & (u64)(0xFFFFFFFFFFFFFF00ll);
				u8PrsSpStId |= prCfaMpg->ucPrsSpStId;

				rSpInf.u4PrsStrmId = (u32)u8PrsSpStId;
				mrRet = Spt4CfaPbb2SpFifoAUCtrl(pvSptHdl, &rSpInf,
					(u64)prCfaMpg->arSPInf[u1CurSpIdx].u2SpuSz);
				if (u8TxAvaLen < prCfaMpg->arSPInf[u1CurSpIdx].u2SpuSz) {
					prCfaMpg->arSPInf[u1CurSpIdx].u2SpuSz =
					(u16)(prCfaMpg->arSPInf[u1CurSpIdx].u2SpuSz - u8TxAvaLen);
				} else
					prCfaMpg->arSPInf[u1CurSpIdx].u2SpuSz = 0;
					break;

				case CFA_MPG_PRS_BIT_STRM_TYPE_NV:
					u8Sa = prCfaMpg->u8PckPos;
					/*mrRet = Spt4CfaPbb2NvFifo(pvSptHdl, u8Sa, u8TxAvaLen);*/
				break;

				default:
					CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
					return;
			}

			if (mrRet != RET_DMX_OK) {
				DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					TEXT("CfaMpgTxNextStrmDataToFifo(): tx data to fifo error:\n"));
				CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
				return;
			}

			if (fgSkipDataTx) {
				prCfaMpg->u8PrsPrevPts = prCfaMpg->u8PrsPts;
				CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, u8TxAvaLen);
				prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
				fgSkipDataTx = FALSE;
			} else {
				prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_PKT_TXING;
				prCfaMpg->fgExitTxDoneCtrl = TRUE;
				prCfaMpg->fgEverTx = TRUE;
				prCfaMpg->u8CfaIssueTxLen = u8TxAvaLen;
#if CFA_MPG_HIGH_BIT_RATE_HANDLE
				if ((prCfaMpg->fgSupportHibitRt) &&
					(prCfaMpg->eCurPrsPktType == CFA_MPG_PRS_BIT_STRM_TYPE_V)) {
					if (prCfaMpg->fgTxData2FIFO)
						prCfaMpg->fgExitTxDoneCtrl = TRUE;
					else {
						prCfaMpg->fgExitTxDoneCtrl = FALSE;
						/* clear cps(CSS or CPRM) decrypt flag*/
						rCPS.fgOn = FALSE;
						rCPS.u8Offset = 0;
						Spt4CfaTurnCPS(pvSptHdl, &rCPS);
						prCfaMpg->fgIsCSSDecOn = FALSE;
						prCfaMpg->fgIsCPRMDecOn = FALSE;
						CfaMpgAnaStPktTxingDP(pvSptHdl, prCfaMpg->u8LatTxAvaLen, prCfaMpg);
						prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
					}
				}
#endif
			}
		} else {
			/* if TxLen = 0, continue to parsing pack 080328 (fix BDP00006510)*/
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, u8TxLen);
			prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
		}
	}
}


/*/ MPG CFA set system clock reference*/
/*/ @return None*/
/*< [IN] handle of fdmx*/
/*< [IN] SCR buffer in MPEG bitstream*/
/*< [IN] pointer to CfaMpgInst*/
static void CfaMpgSetSCR(void *pvSptHdl, const u8 *pucSCRBuf, CfaMpgInst *prCfaMpg)
{
	u32 u4SCR  = 0;
	u64 u8SCR = 0;
	bool fgBit33On = FALSE;

	if ((pucSCRBuf[0] & (u8)0xC0) == (u8)0x40) {
		/* MPEG 2, SCR start with '01b'*/
		if ((pucSCRBuf[0] & (u8)0x20) == (u8)0x20)
			fgBit33On = TRUE;

		BYTE3(u4SCR) = (GET_BITS34(pucSCRBuf[0]) << 6) +
					   (GET_BITS01(pucSCRBuf[0]) << 4) +
						GET_BITS47(pucSCRBuf[1]);
		BYTE2(u4SCR) = (GET_BITS03(pucSCRBuf[1]) << 4) +
						GET_BITS47(pucSCRBuf[2]);
		BYTE1(u4SCR) = (GET_BIT3(pucSCRBuf[2]) << 7) +
					   (GET_BITS01(pucSCRBuf[2]) << 5) +
						GET_BITS37(pucSCRBuf[3]);
		BYTE0(u4SCR) = (GET_BITS02(pucSCRBuf[3]) << 5) +
						GET_BITS37(pucSCRBuf[4]);
	} else {
		/* MPEG1, SCR start with '0010b'*/
		if ((pucSCRBuf[0] & (u8)0x08) == (u8)0x08)
			fgBit33On = TRUE;

		BYTE3(u4SCR) = (GET_BITS12(pucSCRBuf[0]) << 6) +
						GET_BITS27(pucSCRBuf[1]);
		BYTE2(u4SCR) = (GET_BITS01(pucSCRBuf[1]) << 6) +
						GET_BITS27(pucSCRBuf[2]);
		BYTE1(u4SCR) = (GET_BIT1(pucSCRBuf[2]) << 7) +
						GET_BITS17(pucSCRBuf[3]);
		BYTE0(u4SCR) = (GET_BIT0(pucSCRBuf[3]) << 7) +
						GET_BITS17(pucSCRBuf[4]);
	}

	/*adjust PTS/DTS/SCR by StcOffset*/
	u8SCR = (u64)u4SCR;
	if (fgBit33On) {
		u8SCR |= (0x100000000ll);
		fgBit33On = FALSE;
	}

	if ((prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK) ||
		(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK_RVS) ||
		(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK_RVS_EX) ||
		(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_VALUE) ||
		(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_KEEP_CURRENT))

		CfaMpgSTCAdd(u8SCR, (u64)prCfaMpg->i8StcOffset, &(prCfaMpg->u8SCR));

	prCfaMpg->fgExistSCR = TRUE;
}

#if 0
static void CfaMpgParserAVCCodec(CfaMpgInst *prCfaMpg, u32 u4VstCnt)
{
	u32 u4SC = 0;
	u32 u4SkipLength = 0;

	if (CFA_MPG_SYS_STRM_TYPE_ISO13818_PRG_STRM == prCfaMpg->eCfaMpgSysStrmType) {
		u4SkipLength = (u32)prCfaMpg->pucHdrBufRp[8] + (u32)9;
		LOADB_DWRD(prCfaMpg->pucHdrBufRp + u4SkipLength, u4SC);
		if ((u32)0x00000001 == u4SC) {
			u32 u4Length = 0;

			LOADB_WORD(prCfaMpg->pucHdrBufRp + 4, u4Length);
			if ((u4Length < (u32)10) && (u4SkipLength == (u32)9))
				prCfaMpg->aucDecVstType[u4VstCnt] = 0x02;
			else {
				prCfaMpg->aucDecVstType[u4VstCnt] = 0x03;
				prCfaMpg->u2StreamVideoType = AVCODEC_ID_H264;
			}
		} else
			prCfaMpg->aucDecVstType[u4VstCnt] = 0x02;
	} else if (CFA_MPG_SYS_STRM_TYPE_ISO11172_SYS_STRM == prCfaMpg->eCfaMpgSysStrmType) {
		u4SkipLength = (u32)6;
		if ((u8)0x40 == (prCfaMpg->pucHdrBufRp[u4SkipLength] & (u8)0xc0)) {
			u4SkipLength += (u32)2;
			if ((u8)0x20 == (prCfaMpg->pucHdrBufRp[u4SkipLength] & (u8)0xf0))
				u4SkipLength += (u32)5;
			else if ((u8)0x30 == (prCfaMpg->pucHdrBufRp[u4SkipLength] & (u8)0xf0))
				u4SkipLength += (u32)10;
			else {
				/*do nothing*/
			}
		}
		if ((u8)0x20 == (prCfaMpg->pucHdrBufRp[u4SkipLength] & (u8)0xf0))
			u4SkipLength += (u32)5;
		else if ((u8)0x30 == (prCfaMpg->pucHdrBufRp[u4SkipLength] & (u8)0xf0))
			u4SkipLength += (u32)10;
		else if ((u8)0x0f == prCfaMpg->pucHdrBufRp[u4SkipLength])
			u4SkipLength += (u32)1;
		else {
			/*do nothing*/
		}

		LOADB_DWRD(prCfaMpg->pucHdrBufRp + u4SkipLength, u4SC);
		if ((u32)0x00000001 == u4SC) {
			prCfaMpg->aucDecVstType[u4VstCnt] = (u8)0x03;
			prCfaMpg->u2StreamVideoType = (u16)AVCODEC_ID_H264;
		} else
			prCfaMpg->aucDecVstType[u4VstCnt] = (u8)0x01;
	} else {
		/*do nothing*/
	}
}
#endif

/*/ MPG CFA checks video stream information*/
/*/ @return None*/
void CfaMpgChkVidStrmInf(CfaMpgInst *prCfaMpg, u8 ucVstId)
{
	u32 i = 0;
	bool fgVstIdMatch = 0;

	if (CFA_MPG_MAX_STRM_NS > prCfaMpg->u4VstNs) {
		fgVstIdMatch = FALSE;
		for (i = 0; i < prCfaMpg->u4VstNs; ++i) {
			if (ucVstId == prCfaMpg->aucDecVstId[i]) {
				fgVstIdMatch = TRUE;
				break;
			}
		}

		if (FALSE == fgVstIdMatch) {
			prCfaMpg->aucDecVstId[i]  = ucVstId;
			/*add by lqq for video type*/
			if (CFA_MPG_SYS_STRM_TYPE_ISO13818_PRG_STRM == prCfaMpg->eCfaMpgSysStrmType) {
				/*CfaMpgParserAVCCodec(prCfaMpg, i);*/
				prCfaMpg->aucDecVstType[i] = 0x02;
			} else if (CFA_MPG_SYS_STRM_TYPE_ISO11172_SYS_STRM == prCfaMpg->eCfaMpgSysStrmType) {
				/*CfaMpgParserAVCCodec(prCfaMpg, i);*/
				prCfaMpg->aucDecVstType[i] = 0x01;
			} else
				prCfaMpg->aucDecVstType[i] = 0x00;

			++(prCfaMpg->u4VstNs);
		}

#if CFA_MPG_SUPPORT_AVC
		if ((AVCODEC_ID_H264 == prCfaMpg->u2StreamVideoType)
			&& (prCfaMpg->eCfaMpgMediumType == CFA_MPG_MED_TYPE_FILE))
			prCfaMpg->aucDecVstType[i] = 0x03;
#endif
	}
}

/*/ MPG CFA checks if parsing the audio packet information*/
/*/ @return TRUE - parse audio, FALSE - not parse audio*/
bool CfaMpgChkPrsAudPkt(CfaMpgInst *prCfaMpg)
{
	return TRUE;
}

/*/ MPG CFA checks if transferring audio packet data*/
/*/ @return TRUE - transfer into A FIFO, FALSE - not transfer into A FIFO*/
bool CfaMpgChkTxAudPkt(const CfaMpgInst *prCfaMpg)
{
	/*/ always transferring audio pkt for rsp, 080325*/
	if ((CFA_MPG_PRS_BIT_STRM_TYPE_A == prCfaMpg->eCurPrsPktType)
		&& (CFA_MPG_PRS_BIT_STRM_TYPE_A & prCfaMpg->u4CurPrsFlg))
		return TRUE;
	else
		return FALSE;
}

/*/ MPG CFA checks audio stream information*/
/*/ @return None*/
/*< [IN] pointer to CfaMpgInst*/
/*< [IN] audio stream ID ((MPEG stream id<<8) | MPEG sub-stream id)*/
/*< [IN] LPCM attributes, if it's LPCM audio packet. Otherwise setting 0 to it.*/
void CfaMpgChkAudStrmInf(CfaMpgInst *prCfaMpg, u16 u2AstId, u8 ucLpcmAtr)
{
	u32 i = 0;
	bool fgAstIdMatch = 0;
	u8 ucAstMode = 0, ucMpegAstId = 0, ucMpegSubAstId = 0;

	ucMpegAstId = (u8)((u2AstId&(u16)0xff00)>>8);
	ucMpegSubAstId = (u8)(u2AstId&(u16)0x00ff);

	ucAstMode = 0;
	if ((ucMpegAstId & (u8)0xE0) == (u8)0xC0)
		ucAstMode = 1; /*< MPEG*/
	else if ((u8)MPEG_SC_POSTFIX_PRV1_PKT == ucMpegAstId) {
		/* audio defined in DVD spec*/
		switch (ucMpegSubAstId & (u8)A_PKT_MSK) {
		case (u8)AC3_SUBST_SC:
			ucAstMode = 2; /*< AC3*/
			break;

		case (u8)DTS_SUBST_SC:
			ucAstMode = 3; /*< DTS*/
			break;

		case (u8)PCM_SUBST_SC:
			ucAstMode = 4; /*< PCM*/
			break;
		default:
			/* error*/
			break;
		}
	} else {
		/* not support yet*/
	}

	if (CFA_MPG_MAX_STRM_NS > prCfaMpg->u4AstNs) {
		fgAstIdMatch = FALSE;
		for (i = 0; i < prCfaMpg->u4AstNs; ++i) {
			if (u2AstId == prCfaMpg->arAudInf[i].u2DecAstId) {
				fgAstIdMatch = TRUE;
				break;
			}
		}

		if (FALSE == fgAstIdMatch) {
			prCfaMpg->arAudInf[i].u2DecAstId = u2AstId;
			prCfaMpg->arAudInf[i].ucDecAstType = ucAstMode;
			if (((u8)MPEG_SC_POSTFIX_PRV1_PKT == ucMpegAstId) &&
				((ucMpegSubAstId & (u8)A_PKT_MSK) == (u8)PCM_SUBST_SC))
				prCfaMpg->arAudInf[i].ucDecAstAtt = ucLpcmAtr;

			++(prCfaMpg->u4AstNs);
		}
	}
}

/*/ MPG CFA checks if parsing the sub-picture packet information*/
/*/ @return TRUE - parse audio, FALSE - not parse audio*/
bool CfaMpgChkPrsSpPkt(CfaMpgInst *prCfaMpg)
{
	return TRUE;
}

/*/ MPG CFA checks sp stream information*/
/*/ @return None*/
/*< [IN] handle of fdmx*/
/*< [IN] pointer to CfaMpgInst*/
/*< [IN] sp stream ID*/
void CfaMpgChkSpStrmInf(void *pvSptHdl, CfaMpgInst *prCfaMpg, u8 ucSpStId)
{
	u32 i = 0;
	bool fgSpstIdMatch = FALSE;

	if (CFA_MPG_MAX_STRM_NS > prCfaMpg->u4SpstNs) {
		fgSpstIdMatch = FALSE;
		for (i = 0; i < prCfaMpg->u4SpstNs; ++i) {
			if (ucSpStId == prCfaMpg->arSPInf[i].ucDecSpstId) {
				fgSpstIdMatch = TRUE;
				break;
			}
		}

		if (!fgSpstIdMatch) {
			prCfaMpg->arSPInf[i].ucDecSpstId = ucSpStId;
			prCfaMpg->arSPInf[i].u8SpuPrsPts = INVALID_TIMESTAMP;
			++(prCfaMpg->u4SpstNs);
			Spt4CfaSubPicFound(pvSptHdl, ucSpStId);
		}
	}
}


/*/ MPG CFA checks if transferring packet data*/
/*/ @return TRUE - transfer into FIFO, FALSE - not transfer into FIFO*/
bool CfaMpgChkTxPkt(const CfaMpgInst *prCfaMpg)
{
	if (((CFA_MPG_PRS_BIT_STRM_TYPE_V == prCfaMpg->eCurPrsPktType) &&
		(CFA_MPG_PRS_BIT_STRM_TYPE_V & prCfaMpg->u4CurPrsFlg) &&
		(prCfaMpg->ucPrsVidStId == prCfaMpg->ucDecVidStId))		||
		(TRUE == CfaMpgChkTxAudPkt(prCfaMpg))		||
		((CFA_MPG_PRS_BIT_STRM_TYPE_SP0 == prCfaMpg->eCurPrsPktType) &&
		 (CFA_MPG_PRS_BIT_STRM_TYPE_SP0 & prCfaMpg->u4CurPrsFlg))	||
		((CFA_MPG_PRS_BIT_STRM_TYPE_NV == prCfaMpg->eCurPrsPktType) &&
		 (CFA_MPG_PRS_BIT_STRM_TYPE_NV & prCfaMpg->u4CurPrsFlg)))
		return TRUE;

	return FALSE;
}


/*/ MPG CFA gets packet PTS/DTS information*/
/*/ @return None*/
/*/ @note 1. It reads PTS information from PTS field in MPEG bitstream*/
/*< [IN/OUT] destination PTS buffer*/
/*<[IN] source PTS buffer. It includes marker_bit in buffer.*/
void CfaMpgGetPktTS(u64 *pu8Des, const u8 *pucSrc)
{
	const CFAMPG_PTS_DTS *pTS = NULL;
	CfaMpgTimeStamp rTS;

	mm_memset(&rTS, 0, sizeof(CfaMpgTimeStamp));

	if (!pu8Des || !pucSrc) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("<CFA MPG> CfaMpgGetPktTS: pu8Des NULL || pucSrc NULL\n"));
		return;
	}

	rTS.i8QuadPart = 0;

	pTS = (const CFAMPG_PTS_DTS *)(pucSrc);

	rTS.u.u4LowPart = (u32)(pTS->PTS_30 << 30U | pTS->PTS_22 << 22U |
					   pTS->PTS_15 << 15U | pTS->PTS_7 << 7U | pTS->PTS_0);
	rTS.u.i4HighPart = (s32)(pTS->PTS_32 & 1U);

	*pu8Des = rTS.i8QuadPart;
}

/*/ Dispatch DTS/PTS to its own ES, for our current AU update flow*/
/*/ MPG CFA sets ES DTS information*/
/*/ @return None*/
void CfaMpgSetEsDts(CfaMpgInst *prCfaMpg)
{
	u32 u4Index = 0;
	u32 u4Cnt = 0;

	if ((prCfaMpg->eCfaMpgMediumType == CFA_MPG_MED_TYPE_FILE) &&
		(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_NONE)) {
		if (prCfaMpg->u8PrsDts != INVALID_TIMESTAMP)
			prCfaMpg->u8PrsDts += (s64)prCfaMpg->i8StcOffset;
	}

	if (prCfaMpg->eCurPrsPktType == CFA_MPG_PRS_BIT_STRM_TYPE_A) {
		u4Cnt = MIN(prCfaMpg->u4AstNs, CFA_MPG_MAX_STRM_NS);
		for (u4Index = 0; u4Index < u4Cnt; u4Index++) {
			if (prCfaMpg->u2PrsAudStId == prCfaMpg->arAudInf[u4Index].u2DecAstId)
				prCfaMpg->arAudInf[u4Index].u8AudPrsDts = prCfaMpg->u8PrsDts;
		}
	} else if (prCfaMpg->eCurPrsPktType == CFA_MPG_PRS_BIT_STRM_TYPE_SP0) {
		u4Cnt = MIN(prCfaMpg->u4SpstNs, CFA_MPG_MAX_STRM_NS);
		for (u4Index = 0; u4Index < u4Cnt; u4Index++) {
			if (prCfaMpg->ucPrsSpStId == prCfaMpg->arSPInf[u4Index].ucDecSpstId)
				prCfaMpg->arSPInf[u4Index].u8SpuPrsDts = prCfaMpg->u8PrsDts;
		}
	} else {
		/*do nothing*/
	}
}

/*/ MPG CFA sets ES PTS information*/
/*/ @return None*/
void CfaMpgSetEsPts(CfaMpgInst *prCfaMpg)
{
	u32 u4Index = 0;
	s64 i8CuPts = 0;
	u32 u4Cnt = 0;
	CFAMPG_AUD_INF *prAudInf = NULL;
	CFAMPG_SP_INF  *prSPInf = NULL;

	#if 0
	if ((prCfaMpg->eCfaMpgMediumType == CFA_MPG_MED_TYPE_FILE) &&
		(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_NONE)) {
		if (prCfaMpg->u8PrsPts != INVALID_TIMESTAMP)
			prCfaMpg->u8PrsPts += (s64)prCfaMpg->i8StcOffset;
	}
	#endif

	/* Save the current valid PTS, move from bottom to top,
		because the u8PrsPts maybe change in below code @pingzhao*/
	if ((prCfaMpg->u8PrsPts != INVALID_TIMESTAMP) && (prCfaMpg->fgExistPts)) {
		prCfaMpg->u8LastPts = prCfaMpg->u8PrsPts;
		prCfaMpg->u8LastPtsAddr =
			(DMX_INVALID_UINT64 == prCfaMpg->u8Ca) ?
			(prCfaMpg->rCfaRange.u8Sa) : (prCfaMpg->u8Ca + 1);
	}

	if (prCfaMpg->eCurPrsPktType == CFA_MPG_PRS_BIT_STRM_TYPE_A) {
		u4Cnt = MIN(prCfaMpg->u4AstNs, CFA_MPG_MAX_STRM_NS);
		for (u4Index = 0; u4Index < u4Cnt; u4Index++) {
			prAudInf = &prCfaMpg->arAudInf[u4Index];
			if (prCfaMpg->u2PrsAudStId == prAudInf->u2DecAstId) {
				/* BDP00116536 special handle*/
				/* refer ISO 11172-1 2.4.5.3 Frequency of pts coding, page 26*/
				if ((prAudInf->i8AudLastPrsPts != 0) &&
					(prAudInf->i8AudLastPrsPts < (s64)prCfaMpg->u8PrsPts) &&
					(prCfaMpg->u8PrsPts != INVALID_TIMESTAMP)) {
					u64 u8Tm = prCfaMpg->u8PrsPts - prAudInf->i8AudLastPrsPts;

					if (CFA_MPG_VCD_ERR_PTS_DETECT && (u8Tm > ((u64)23 * (u64)90090)))
						prCfaMpg->u8PrsPts = INVALID_TIMESTAMP;
				}
#if (CFA_MPG_STC_RESET)
				/*BDP00117302, add for STC reset file normal play, set gap = 4mins*/
				if ((prCfaMpg->eCfaMpgMediumType == CFA_MPG_MED_TYPE_FILE) &&
					(prCfaMpg->u8PrsPts != INVALID_TIMESTAMP)) {
					i8CuPts = prCfaMpg->u8PrsPts;
					if (prAudInf->fgPtsAdjust)
						i8CuPts += prAudInf->i8AudPtsAdjust;

					if ((prAudInf->i8AudLastPrsPts != 0) &&
						(prAudInf->i8AudLastPrsPts < i8CuPts) &&
						(i8CuPts - prAudInf->i8AudLastPrsPts > (s64)5 * (s64)CFA_STC_CLK)) {
						prAudInf->fgPtsAdjust = TRUE;
						prAudInf->i8AudPtsAdjust -=
							i8CuPts - 1000 - prAudInf->i8AudLastPrsPts;
						i8CuPts -= i8CuPts - 1000 - prAudInf->i8AudLastPrsPts;
					}

					if ((prAudInf->i8AudLastPrsPts > i8CuPts) &&
						(prAudInf->i8AudLastPrsPts - i8CuPts > 0)) {
						prAudInf->fgPtsAdjust = TRUE;
						i8CuPts -= prAudInf->i8AudPtsAdjust;
						prAudInf->i8AudPtsAdjust =
							prAudInf->i8AudLastPrsPts - i8CuPts + 1000;
						i8CuPts += prAudInf->i8AudPtsAdjust;
					}

					if ((prCfaMpg->fgFindFirstVideoPts) &&
						(prCfaMpg->rCfaRange.ePtsAdjustType ==
						MPG_PTS_ADJUST_FROM_VALUE))

						i8CuPts = (s64)INVALID_TIMESTAMP;

					if (i8CuPts >= 0)
						prCfaMpg->u8PrsPts = i8CuPts;
					else
						prCfaMpg->u8PrsPts = INVALID_TIMESTAMP;
				}
#endif
				if (prCfaMpg->u8PrsPts != INVALID_TIMESTAMP) {
					prAudInf->i8AudLastPrsPts = prCfaMpg->u8PrsPts;
					if (prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_FROM_VALUE) {
						CfaMpgAdjustAudPts(prCfaMpg);
						DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
							TEXT("Set Audio Pts %lld\r\n"), prCfaMpg->u8PrsPts);
					}
				}
				prAudInf->u8AudPrsPts = prCfaMpg->u8PrsPts;
			}
		}
	} else if (prCfaMpg->eCurPrsPktType ==
			CFA_MPG_PRS_BIT_STRM_TYPE_SP0) {
		u4Cnt = MIN(prCfaMpg->u4SpstNs, CFA_MPG_MAX_STRM_NS);
		for (u4Index = 0; u4Index < u4Cnt; u4Index++) {
			prSPInf = &prCfaMpg->arSPInf[u4Index];
			if (prCfaMpg->ucPrsSpStId == prSPInf->ucDecSpstId)
				prSPInf->u8SpuPrsPts = prCfaMpg->u8PrsPts;
		}
	}
#if (CFA_MPG_STC_RESET)
	else if ((prCfaMpg->eCurPrsPktType == CFA_MPG_PRS_BIT_STRM_TYPE_V) &&
			(prCfaMpg->eCfaMpgMediumType == CFA_MPG_MED_TYPE_FILE)) {
		if ((prCfaMpg->fgFindFirstVideoPts)
			&& (prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_FROM_VALUE)
			&& (prCfaMpg->u8PrsPts != INVALID_TIMESTAMP)) {
			prCfaMpg->i8PtsResetAdValue = prCfaMpg->u8PrsPts;
			prCfaMpg->fgFindFirstVideoPts = FALSE;
			DmxLogT(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT(" Pts Reset Value %lld\r\n"), prCfaMpg->u8PrsPts);
		}
		u4Cnt = MIN(prCfaMpg->u4VstNs, CFA_MPG_MAX_STRM_NS);
		for (u4Index = 0; u4Index < u4Cnt; u4Index++) {
			if (prCfaMpg->ucPrsVidStId == prCfaMpg->aucDecVstId[u4Index]) {
				/*Adjust the video pts gap ,if two video pts gap more then 29s,
				set the large one to invalid @pingzhao, 2008/10/30,
				for JinzhenJinqu 001-01,track 1*/
				prCfaMpg->u8VideoPacketPts = prCfaMpg->u8PrsPts;
				if ((prCfaMpg->u8PrsPts != INVALID_TIMESTAMP) &&
					(prCfaMpg->au8VidLastPrsPts[u4Index] != 0) &&
					(prCfaMpg->au8VidLastPrsPts[u4Index] < prCfaMpg->u8PrsPts)) {
					u64 u8Tm = prCfaMpg->u8PrsPts - prCfaMpg->au8VidLastPrsPts[u4Index];

					if (u8Tm > (u64)29 * (u64)CFA_STC_CLK)
						prCfaMpg->u8PrsPts = INVALID_TIMESTAMP;
				}
				if ((prCfaMpg->u8PrsPts != INVALID_TIMESTAMP) &&
					(prCfaMpg->au8VidLastPrsPts[u4Index] > prCfaMpg->u8PrsPts))
					prCfaMpg->u8PrsPts = INVALID_TIMESTAMP;

				if (prCfaMpg->u8PrsPts != INVALID_TIMESTAMP) {
					prCfaMpg->au8VidLastPrsPts[u4Index] = prCfaMpg->u8PrsPts;
					if (prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_FROM_VALUE)
						CfaMpgAdjustVidPts(prCfaMpg);
				}
			}
		}
	}else {
		/*do nothing*/
	}
#endif
}


/*/ MPG CFA found first MPEG start code information*/
/*/ @return None*/
/*/ @note This API stops parsing process*/
/*< [IN] handle of fdmx*/
/*< [IN] pointer to CfaMpgInst*/
/*< [IN] start code*/
/*< [IN] start code position*/
void CfaMpgQIFoundFirstMpegScInf(void *pvSptHdl, CfaMpgInst *prCfaMpg, u32 u4Sc,
							   u64 u8ScPos)
{
	CfaMpgQIFirstMpgScInf rCfaMpgQIFirstMpgScInf;

	mm_memset(&rCfaMpgQIFirstMpgScInf, 0, sizeof(CfaMpgQIFirstMpgScInf));

	rCfaMpgQIFirstMpgScInf.u4Sc = u4Sc;
	rCfaMpgQIFirstMpgScInf.u8Pos = u8ScPos;
	CfaMpgNotifyInq(pvSptHdl, CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_SC_INF, &rCfaMpgQIFirstMpgScInf, prCfaMpg);

	/* when  CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_SC_INF got, turn it off*/
	prCfaMpg->u4CurQueryInfType &= (~CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_SC_INF);
}


/*/ MPG CFA found first MPEG PTS information*/
/*/ @return None*/
/*/ @note This API stops parsing process*/
void CfaMpgQIFoundFirstMpegPtsInf(void *pvSptHdl, CfaMpgInst *prCfaMpg)
{
	CfaMpgQIFirstMpgPtsInf rCfaMpgQIFirstMpgPtsInf;

	mm_memset(&rCfaMpgQIFirstMpgPtsInf, 0, sizeof(CfaMpgQIFirstMpgPtsInf));

	rCfaMpgQIFirstMpgPtsInf.u8Pts = prCfaMpg->u8PrsPts;
	rCfaMpgQIFirstMpgPtsInf.u8Pos = (DMX_INVALID_UINT64 ==
		prCfaMpg->u8Ca) ? (prCfaMpg->rCfaRange.u8Sa) : (prCfaMpg->u8Ca+1);
	rCfaMpgQIFirstMpgPtsInf.u2CurPrsPktType = prCfaMpg->eCurPrsPktType;

	CfaMpgNotifyInq(pvSptHdl, CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_PTS_INF,
		&rCfaMpgQIFirstMpgPtsInf, prCfaMpg);

	/* When CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_PTS_INF is got, turn it off*/
	prCfaMpg->u4CurQueryInfType &= (~CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_PTS_INF);
}


/*/ MPG CFA get mux-rate information*/
/*/ @return None*/
/*< [IN] handle of fdmx*/
/*< [IN] pointer to CfaMpgInst*/
/*< [IN] muxrate infomation, 50 bytes/sec*/
static void CfaMpgQIMuxRateInf(void *pvSptHdl, CfaMpgInst *prCfaMpg, u32 u4MuxRate)
{
	CfaMpgQIMuxRateInf_T rCfaMpgQIMuxRateInf;

	mm_memset(&rCfaMpgQIMuxRateInf, 0, sizeof(CfaMpgQIMuxRateInf_T));

	rCfaMpgQIMuxRateInf.u4MuxRate = u4MuxRate;
	CfaMpgNotifyInq(pvSptHdl, CFA_MPG_QUERY_INF_TYPE_MUX_RATE_INF,
		&rCfaMpgQIMuxRateInf, prCfaMpg);

	/* when  CFA_MPG_QUERY_INF_TYPE_MUX_RATE_INF got, turn it off*/
	prCfaMpg->u4CurQueryInfType &= (~CFA_MPG_QUERY_INF_TYPE_MUX_RATE_INF);
}


/*/ MPG CFA check MPEG version*/
/*/ @return TRUE - Success, FALSE - Fail*/
/*< [IN] handle of fdmx*/
/*< [IN] Actual transferred data length.
	Normally this value should be equal to the u8TxLen in the previous transfer issue, unless file end is hit.*/
/*< [IN] pointer to CfaMpgInst*/
bool CfaMpgChkStrmVer(void *pvSptHdl, CfaMpgInst *prCfaMpg)
{
	u8 ucVer = 0;
	u32 u4MuxRate = 0;

	ucVer = prCfaMpg->pucHdrBufRp[0];
	prCfaMpg->fgExistSCR = FALSE;
	if ((ucVer & (u8)0xf0) == (u8)0x20) {
		/* iso11172 system stream*/

		prCfaMpg->eCfaMpgSysStrmType = CFA_MPG_SYS_STRM_TYPE_ISO11172_SYS_STRM;
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;

		CfaMpgSetSCR(pvSptHdl, prCfaMpg->pucHdrBufRp, prCfaMpg);

		if (CFA_MPG_QUERY_INF_TYPE_MUX_RATE_INF & prCfaMpg->u4CurQueryInfType) {
			const CFAMPG_1_MUXRATE *pMux;
			u8  bMux1, bMux3;

			pMux = (const CFAMPG_1_MUXRATE *)(prCfaMpg->pucHdrBufRp + 5);
			bMux1 = pMux->mux_r1 & (u8)0x7F;
			bMux3 = (pMux->mux_r3 >> (u8)1) & (u8)0x7F;
			u4MuxRate = (u32)(((u32)bMux1 << (u32)15) | (u32)((u32)pMux->mux_r2 << (u32)7) | (u32)bMux3);
			CfaMpgQIMuxRateInf(pvSptHdl, prCfaMpg, u4MuxRate);
		}

		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)5 + (u64)3);
	} else if ((ucVer & (u8)0xc0) == (u8)0x40) {
		/* iso13818 program stream*/

		prCfaMpg->eCfaMpgSysStrmType = CFA_MPG_SYS_STRM_TYPE_ISO13818_PRG_STRM;
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_M2_CHK_PCKSTF_LEN;

		CfaMpgSetSCR(pvSptHdl, prCfaMpg->pucHdrBufRp, prCfaMpg);
		if (CFA_MPG_QUERY_INF_TYPE_MUX_RATE_INF & prCfaMpg->u4CurQueryInfType) {
			const CFAMPG_2_MUXRATE *pMux;
			u8  bMux3;

			pMux = (const CFAMPG_2_MUXRATE *)(prCfaMpg->pucHdrBufRp + 6);
			bMux3 = (pMux->mux_r3 >> 2U) & (u8)0x3F;
			u4MuxRate = (u32)(((u32)pMux->mux_r1 << (u32)14) | ((u32)pMux->mux_r2 << (u32)6) | (u32)bMux3);
			CfaMpgQIMuxRateInf(pvSptHdl, prCfaMpg, u4MuxRate);
		}

		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)6 + (u64)3);
		/*SCR, mux rate*/
	} else {
		/*prCfaMpg->u8PckPos = DMX_INVALID_UINT64;*/
		return FALSE;
	}
	return TRUE;
}


/*/ MPG CFA re-buffer parsing data*/
/*/ @return None*/
void CfaMpgRebufPrsData(void *pvSptHdl, CfaMpgInst *prCfaMpg)
{
	u64	u8Sa = 0, u8TxSz = 0;
	u32	u4Ofst = 0;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prCfaMpg) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("<CFA MPG> CfaMpgRebufPrsData: prCfaMpg NULL\n"));
		return;
	}

	/* check if finishing to parse bitstream*/
	if (DMX_INVALID_UINT64 == prCfaMpg->u8Ca) {
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
		return;
	}
#if CFA_MPG_HIGH_BIT_RATE_HANDLE
	if ((prCfaMpg->u8Ca > (prCfaMpg->u8LastSyncPbbufCa + prCfaMpg->u4LastAvailSz)) &&
		(prCfaMpg->u2CmdQIndex != 0)) {
		DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("<CFA> CfaMpgRebufPrsData ============================ txing cmdq\n"));
		CfaMpgCmdQTxing(pvSptHdl, prCfaMpg);
		return;
	}
#endif

	u8TxSz = (u64)CFA_MPG_HDR_BUF_SZ;
	u4Ofst = 0;

	/* check if moving the residue data to the header buffer start*/
	if (0 != prCfaMpg->u4RdyDataSz) {
		u8TxSz -= prCfaMpg->u4RdyDataSz;
		u4Ofst = prCfaMpg->u4RdyDataSz;
	}

	u8Sa = prCfaMpg->u8Ca + (u64)u4Ofst;
	/* check if transferring data to be parsed*/
	/* Workaround for No audio after seeking and No EOS issue by jie.tang*/
	if (u8Sa <= (prCfaMpg->rCfaRange.u8Ea - (u64)2020)) {
		/* transferring valid data*/
		u8TxSz = MIN(u8TxSz, prCfaMpg->rCfaRange.u8Ea - u8Sa + (u64)1);
		mrRet = Spt4CfaPbb2SyncBufEx(pvSptHdl, prCfaMpg->u8Ca, u8TxSz + u4Ofst,
			(u8 *)&prCfaMpg->ptrPfrMemAddress, &prCfaMpg->u4AvailSz);

		prCfaMpg->u8CfaIssueTxLen = (u64)(u8TxSz+u4Ofst);
		if (mrRet != RET_DMX_OK) {
			DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("CfaMpgRebufPrsData(): sync pbbuf fail!\n"));
			CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
			return;
		}
		/*using sync DMA, 071220*/
		prCfaMpg->u8LastSyncPbbufCa = prCfaMpg->u8Ca;
		prCfaMpg->u4LastAvailSz = prCfaMpg->u4AvailSz;
		prCfaMpg->fgTxData2HdrBuf = TRUE;
		prCfaMpg->fgExitTxDoneCtrl = TRUE;
	} else {
	/* not enough data to transfer*/
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
	}
}


