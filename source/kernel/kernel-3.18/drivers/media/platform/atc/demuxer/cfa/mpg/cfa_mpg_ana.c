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
/* #include <media/atc/mm_debug.h> */

#include "dmx_def.h"
#include "dmx_mem.h"
#include "cfa_macro.h"
#include "cfa_mpg_ana.h"
#include "cfa_mpg_st_ctrl.h"

#if CFA_MPG_SUPPORT_AVC

static bool fgCfaMpgChkAVC(CfaMpgInst *prCfaMpgInst)
{
	u16 u2PgStrmInfoLen = 0;
	u16 u2ElemStrmMapLen = 0;
	u16 u2ElemStrmInfoLen = 0;
	u16 u2Index = 0;
	u16 u2Count = 0;
	u16 u2StreamTypeTbl[CFA_MPG_MAX_CHN] = { 0 };
	bool fgIsAVC = FALSE;

	LOADB_WORD(prCfaMpgInst->pucHdrBufRp + 8, u2PgStrmInfoLen);
	LOADB_WORD(prCfaMpgInst->pucHdrBufRp + u2PgStrmInfoLen + 10, u2ElemStrmMapLen);
	u2Count = u2ElemStrmMapLen;
	if(u2Count >= CFA_MPG_MAX_CHN) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("<CFA MPG> u2Count :%d!\r\n"),u2Count);
		return FALSE;
	}
	while (u2Count > 0) {
		LOADB_WORD(prCfaMpgInst->pucHdrBufRp + u2PgStrmInfoLen +
			   (u2ElemStrmMapLen - u2Count) + 12, u2StreamTypeTbl[u2Index]);
		LOADB_WORD(prCfaMpgInst->pucHdrBufRp + u2PgStrmInfoLen +
			   (u2ElemStrmMapLen - u2Count) + 14, u2ElemStrmInfoLen);
		u2Count = u2Count - (u2ElemStrmInfoLen + (u16)4);
		u2Index++;
	}
	while (u2Index) {
		if ((u16)AVC_PKT_MSK == (u2StreamTypeTbl[u2Index - (u16)1] & (u16)0xFFF0)) {
			prCfaMpgInst->u2StreamVideoType = AVCODEC_ID_H264;
			fgIsAVC = TRUE;
		}

		u2Index--;
	}
	if (fgIsAVC)
		return TRUE;
	return FALSE;
}

#endif


/* MPG CFA processes CFA_MPG_ANA_ST_IDLE*/
/* @return None*/
static void CfaMpgAnaStIdle(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
	/* cannot go here*/
}


/* MPG CFA processes CFA_MPG_ANA_ST_PKT_TXING*/
/* @return None*/
static void CfaMpgAnaStPktTxing(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
	if (CFA_MPG_SYS_STRM_TYPE_VIDEO_SEQUENCE == prCfaMpg->eCfaMpgSysStrmType) {
		/* MPEG video stream only*/
		CfaMpgIncPrsPos(prCfaMpg, u8TxLen);

		/* finish current parsing*/
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
	} else {		/* MPEG stream*/
		prCfaMpg->u8PrsPrevPts = prCfaMpg->u8PrsPts;
		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u8TxLen - prCfaMpg->u2SkipLen));
		prCfaMpg->u2SkipLen = 0;
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
	}
}


/* MPG CFA processes CFA_MPG_ANA_ST_SEARCH_SC*/
/* @return None*/
static void CfaMpgAnaStSearchSc(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
	u32 u4HdrAdr = 0;
	u8 auMatchScLst[2] = { MPEG_SC_POSTFIX_PACK_HDR, MPEG_SC_POSTFIX_SEQ_HDR };
	u32 u4SearchSCSize = 0;

	/* check if finishing to parse bitstream*/
	if ((u64) DMX_INVALID_UINT64 == prCfaMpg->u8Ca) {
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
		return;
	}
	/* check if data is enough to be parsed*/
	if (FALSE == CfaMpgChkPrsDataRdy(4, prCfaMpg)) {
		CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
		return;
	}
	/*add by lqq for demux error handle*/
	u4SearchSCSize = prCfaMpg->u4RdyDataSz;

	if (prCfaMpg->fgDealDxEr) {
		if ((prCfaMpg->u8Ca - prCfaMpg->u8PckPos) >= CFA_MPG_DEMUX_MAX_LEN) {
			prCfaMpg->fgDemuxError = TRUE;
			CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
			return;
		} else if (((u64)CFA_MPG_DEMUX_MAX_LEN - (prCfaMpg->u8Ca - prCfaMpg->u8PckPos)) <
			   u4SearchSCSize) {
			u4SearchSCSize =
				(u32) (CFA_MPG_DEMUX_MAX_LEN -
					  (prCfaMpg->u8Ca - prCfaMpg->u8PckPos));
		} else {
			/*do nothing*/
		}
		
	}

	u4HdrAdr = CfaMpgScSearch(prCfaMpg->pucHdrBufRp, u4SearchSCSize, auMatchScLst, (u32)2);
	if (DMX_INVALID_UINT32 != u4HdrAdr)	{
		/* prefix found*/
		if (CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_SC_INF & prCfaMpg->u4CurQueryInfType) {
			u32 u4Sc = 0;
			u64 u8ScPos = 0;

			u4Sc = (u32)0x00000100 | (u32)(prCfaMpg->pucHdrBufRp[u4HdrAdr + (u32)3]);
			/*u8ScPos = prCfaMpg->rCfaRange.u8Sa + (u64)u4HdrAdr;*/
			u8ScPos = prCfaMpg->u8Ca + (u64) u4HdrAdr;

			CfaMpgQIFoundFirstMpegScInf(pvSptHdl, prCfaMpg, u4Sc, u8ScPos);
		}

		if ((u8)MPEG_SC_POSTFIX_PACK_HDR == prCfaMpg->pucHdrBufRp[u4HdrAdr + (u32)3]) {
			/* keep current pack address*/
			/*prCfaMpg->u8PckPos = prCfaMpg->rCfaRange.u8Sa + (u64)u4HdrAdr;*/
			prCfaMpg->u8PckPos = prCfaMpg->u8Ca + (u64) u4HdrAdr;

			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)(u4HdrAdr + (u32)4));
			prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_STRM_VER;
		} else if (((u8)MPEG_SC_POSTFIX_SEQ_HDR == prCfaMpg->pucHdrBufRp[u4HdrAdr + (u32)3])
			   && (CFA_MPG_SYS_STRM_TYPE_VIDEO_SEQUENCE == prCfaMpg->eCfaMpgSysStrmType)
			   && (CFA_MPG_PRS_BIT_STRM_TYPE_V & prCfaMpg->u4CurPrsFlg)) {
			prCfaMpg->eCurPrsPktType = CFA_MPG_PRS_BIT_STRM_TYPE_V;

			if ((prCfaMpg->u8Ca + (u64) u4HdrAdr) <= prCfaMpg->rCfaRange.u8Ea) {
				u64 u8Sa = 0;

				u8Sa =
					(prCfaMpg->u8Ca <=
					 prCfaMpg->rCfaRange.u8Sa) ? prCfaMpg->rCfaRange.
					u8Sa : prCfaMpg->u8Ca;
				CfaMpgTxNextStrmDataToFifo(pvSptHdl, prCfaMpg, (u64) u4HdrAdr,
							   prCfaMpg->rCfaRange.u8Ea - u8Sa + (u64)1);
			} else {
				/* finish current parsing*/
				CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
			}
		} else {	/* search next start code*/
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64) (prCfaMpg->u4RdyDataSz - 3));
		}
	} else {
		/* search next start code*/
		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64) (u4SearchSCSize));
		/*add by lqq for demux error handle*/
		if (((prCfaMpg->u8Ca - prCfaMpg->u8PckPos) >= CFA_MPG_DEMUX_MAX_LEN) &&
			(prCfaMpg->fgDealDxEr)) {
			prCfaMpg->fgDemuxError = TRUE;
			CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
		}
	}
}

/* MPG CFA processes CFA_MPG_ANA_ST_SEARCH_PCK*/
/* @return None*/
static void CfaMpgAnaStSearchPck(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
	u32 u4HdrAdr = 0;
	u8 auMatchScLst[1] = { MPEG_SC_POSTFIX_PACK_HDR };

	/* check if finishing to parse bitstream*/
	if ((u64) DMX_INVALID_UINT64 == prCfaMpg->u8Ca) {
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
		return;
	}
#if CFA_MPG_HIGH_BIT_RATE_HANDLE
	if ((prCfaMpg->u4AvailSz < PSR_RESERVE_FIFO_SPACE) && (prCfaMpg->fgSupportHibitRt)) {
		if (prCfaMpg->u2CmdQIndex != 0) {
			CfaMpgCmdQTxing(pvSptHdl, prCfaMpg);
			prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_SEARCH_PCK;
			return;
		}
		prCfaMpg->fgTxByPbbuf = TRUE;
	}
#endif
	/* check if data is enough to be parsed*/
	if (FALSE == CfaMpgChkPrsDataRdy(4, prCfaMpg)) {
		CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
		return;
	}

	u4HdrAdr = CfaMpgScSearch(prCfaMpg->pucHdrBufRp, prCfaMpg->u4RdyDataSz, auMatchScLst, (u32)1);
	if (DMX_INVALID_UINT32 != u4HdrAdr) {
		/* prefix found*/
		/* keep current pack address*/
		/*prCfaMpg->u8PckPos = prCfaMpg->rCfaRange.u8Sa + (u64)u4HdrAdr;*/
		prCfaMpg->u8PckPos = prCfaMpg->u8Ca + (u64) u4HdrAdr;

		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64) (u4HdrAdr + (u32)4));
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_STRM_VER;
	} else {
		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64) (prCfaMpg->u4RdyDataSz - 3));
	}
}

/* MPG CFA processes CFA_MPG_ANA_ST_CHK_STRM_VER*/
/* @return None*/
static void CfaMpgAnaStChkStrmVer(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
	/* check if finishing to parse bitstream*/
	if ((u64) DMX_INVALID_UINT64 == prCfaMpg->u8Ca) {
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
		return;
	}
	/* check if data is enough to be parsed, SCR + mux rate*/
	if (FALSE == CfaMpgChkPrsDataRdy(9, prCfaMpg)) {
		CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
		return;
	}

	if (FALSE == CfaMpgChkStrmVer(pvSptHdl, prCfaMpg))
		/* search pack header*/
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_SEARCH_PCK;
}


/* MPG CFA processes CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR*/
/* @return None*/
static void CfaMpgAnaStChkGnrlPktHdr(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
	u32 u4Prfx = 0;

	/* check if finishing to parse bitstream*/
	if ((u64) DMX_INVALID_UINT64 == prCfaMpg->u8Ca) {
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
		return;
	}

#if CFA_MPG_HIGH_BIT_RATE_HANDLE
	if ((prCfaMpg->u4AvailSz < PSR_RESERVE_FIFO_SPACE) && (prCfaMpg->fgSupportHibitRt)) {
		if (prCfaMpg->u2CmdQIndex != 0) {
			CfaMpgCmdQTxing(pvSptHdl, prCfaMpg);
			prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
			return;
		}
		prCfaMpg->fgTxByPbbuf = TRUE;
	}
#endif
	/* check if data is enough to be parsed*/
	if (FALSE == CfaMpgChkPrsDataRdy(2020, prCfaMpg)) {
		CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
		return;
	}
	/*lqq, for demux error handle*/
	if (((prCfaMpg->u8Ca - prCfaMpg->u8PckPos) >= CFA_MPG_DEMUX_MAX_LEN) &&
		(prCfaMpg->fgDealDxEr)) {
		prCfaMpg->fgDemuxError = TRUE;
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
		return;
	}

	prCfaMpg->u2PrevPktLen = prCfaMpg->u2PktLen;
	prCfaMpg->u2PktLen = 0;

	/* read start code*/
	LOADB_DWRD(prCfaMpg->pucHdrBufRp, u4Prfx);

	if ((u32)CFA_MPG_SC_PACK == u4Prfx) {
		/* keep current pack address*/
		prCfaMpg->u8PckPos = prCfaMpg->u8Ca;

		/* for SCR*/
		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)4);
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_STRM_VER;
		return;
	}

	if ((u32)CFA_MPG_SC_PG_END == u4Prfx) {
		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)4);
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_SEARCH_PCK;
		return;
	}
	/* check if data is enough to be parsed*/
	/* 28 = 24(NV pack system header) + 4(packet header sc)*/
	/*if (FALSE == CfaMpgChkPrsDataRdy(2044, prCfaMpg))
	   {
	   CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
	   return;
	   } */

	if ((u4Prfx == (u32)CFA_MPG_SC_NV_PKT) &&
		((CFA_MPG_SYS_STRM_TYPE_ISO13818_PRG_STRM == prCfaMpg->eCfaMpgSysStrmType) ||
		 (CFA_MPG_SYS_STRM_TYPE_ISO11172_SYS_STRM == prCfaMpg->eCfaMpgSysStrmType))) {
		/* ccma 2008.1.17 fix wrong len. load system header length*/
		LOADB_WORD(prCfaMpg->pucHdrBufRp + 4, prCfaMpg->u2PktLen);
		/*Jimmy 2008.1.9 we should check the nv type of the bitstream by packet streamid. ex. -VR case.*/
		LOADB_DWRD(prCfaMpg->pucHdrBufRp + 6 + prCfaMpg->u2PktLen, u4Prfx);
		/* audio, video bound*/
		if (prCfaMpg->ucAudBound == 0xff) {
			prCfaMpg->ucAudBound = prCfaMpg->pucHdrBufRp[9];
			prCfaMpg->ucAudBound = (prCfaMpg->ucAudBound >> 2U) & (u8)0x3f;
		}
		if (prCfaMpg->ucVidBound == 0xff) {
			prCfaMpg->ucVidBound = prCfaMpg->pucHdrBufRp[10];
			prCfaMpg->ucVidBound = prCfaMpg->ucVidBound & (u8)0x1f;
		}

		if (u4Prfx == (u32)CFA_MPG_SC_PRV2_PKT) {
			/* from system header, 24 + 4(PCI/RDI packet hdr stream id)*/
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)28);
			prCfaMpg->eCurPrsPktType = CFA_MPG_PRS_BIT_STRM_TYPE_NV;
			prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_NV_PKTHDR;
		} else {
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)((u16)6 + prCfaMpg->u2PktLen));
		}
		return;
	}

	if ((u4Prfx == (u32)CFA_MPG_SC_V_PKT) || (u4Prfx == (u32)CFA_MPG_SC_STN_PKT)
		|| (u4Prfx == (u32)CFA_MPG_SC_STH_PKT)) {
		if (u4Prfx == (u32)CFA_MPG_SC_V_PKT) {
			prCfaMpg->ucPrsVidStId = (u8)CFA_MPG_PRS_VID_STRM_ID_MV;
		} else if (u4Prfx == (u32)CFA_MPG_SC_STN_PKT) {
			prCfaMpg->ucPrsVidStId = (u8)CFA_MPG_PRS_VID_STRM_ID_STN;
		} else if (u4Prfx == (u32)CFA_MPG_SC_STH_PKT) {
			prCfaMpg->ucPrsVidStId = (u8)CFA_MPG_PRS_VID_STRM_ID_STH;
		} else {	/* impossible*/
			prCfaMpg->ucPrsVidStId = (u8)CFA_MPG_PRS_VID_STRM_ID_NONE;
		}

		CfaMpgChkVidStrmInf(prCfaMpg, BYTE0(u4Prfx));
		/* load pktlen for search efficiently, 20080330*/
		LOADB_WORD(prCfaMpg->pucHdrBufRp + 4, prCfaMpg->u2PktLen);
		if (prCfaMpg->u2PktLen == 0) {
			/* may parsing wrong data, need to skip it. 080406*/
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)4);
			return;
		}
		if ((CFA_MPG_PRS_BIT_STRM_TYPE_V & prCfaMpg->u4CurPrsFlg) ||
			(CFA_MPG_QUERY_INF_TYPE_NONE != prCfaMpg->u4CurQueryInfType)) {
			prCfaMpg->eCurPrsPktType = CFA_MPG_PRS_BIT_STRM_TYPE_V;
			prCfaMpg->u8PicPckPos = prCfaMpg->u8PckPos;	/* keep picture pack position*/

			if ((prCfaMpg->fgIsCSSDecOn) || (prCfaMpg->fgIsCPRMDecOn)) {
				if ((prCfaMpg->u8Ca + (u64)prCfaMpg->u2PktLen + (u64)6) <
					(prCfaMpg->u8PckPos + (u64)0x80)) {
					prCfaMpg->u2CSSJumpLen =
						(u16) (prCfaMpg->u8PckPos + (u64)0x80 -
							  (prCfaMpg->u8Ca + (u64)prCfaMpg->u2PktLen + (u64)6));
				}
			}

			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)6);
			prCfaMpg->u4PESHdrDataLen = 0;
			if (CFA_MPG_SYS_STRM_TYPE_ISO11172_SYS_STRM == prCfaMpg->eCfaMpgSysStrmType)
				prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_M1_CHK_PKTHDR;
			 else
				prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_M2_CHK_PKTHDR;
			return;
		}
	}

	if (CFA_MPG_SYS_STRM_TYPE_ISO13818_PRG_STRM == prCfaMpg->eCfaMpgSysStrmType) {
		/* assume MPEG2 bitstream is derived from DVD specification, and support extension audio*/
		if (((u4Prfx & (u32)CFA_MPG_DVD_MPA_PKT_MSK) == (u32)CFA_MPG_SC_AUD_PKT) ||
			((u32)CFA_MPG_AUD_5CH_SUPPORT
			 && ((u4Prfx & (u32)CFA_MPG_DVD_MPA_PKT_MSK) == (u32)CFA_MPG_SC_AUD_EXT_PKT))) {
			u16 u2AudId = 0;

			u4Prfx = (u32)CFA_MPG_SC_AUD_PKT | (u4Prfx & (u32)0x7);

			u2AudId = (u16) ((u4Prfx & (u32)0x000000ff) << 8U);
			CfaMpgChkAudStrmInf(prCfaMpg, u2AudId, (u8)0);

			/* load pktlen for search efficiently, 20080330*/
			LOADB_WORD(prCfaMpg->pucHdrBufRp + 4, prCfaMpg->u2PktLen);
			if (prCfaMpg->u2PktLen == 0) {
				/* may parsing wrong data, need to skip it. 080406*/
				CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)4);
				return;
			}

			if (TRUE == CfaMpgChkPrsAudPkt(prCfaMpg)) {
				if ((prCfaMpg->fgIsCSSDecOn) || (prCfaMpg->fgIsCPRMDecOn)) {
					if ((prCfaMpg->u8Ca + (u64)prCfaMpg->u2PktLen + (u64)6) <
						(prCfaMpg->u8PckPos + (u64)0x80)) {
						prCfaMpg->u2CSSJumpLen =
							(u16) (prCfaMpg->u8PckPos + (u64)0x80 -
								  (prCfaMpg->u8Ca + (u64)prCfaMpg->u2PktLen +
								   (u64)6));
					}
				}
				CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)6);
				prCfaMpg->u2PrsAudStId = u2AudId;
				prCfaMpg->eCurPrsPktType = CFA_MPG_PRS_BIT_STRM_TYPE_A;
				prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_M2_CHK_PKTHDR;
				return;
			}
		} else if (CFA_MPG_SC_PRV1_PKT == u4Prfx) {
			/* assume private packet for DVD*/
			LOADB_WORD(prCfaMpg->pucHdrBufRp + 4, prCfaMpg->u2PktLen);
			if (prCfaMpg->u2PktLen == 0) {
				/* may parsing wrong data, need to skip it. 080406*/
				CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, 4);
				return;
			}

			if ((prCfaMpg->fgIsCSSDecOn) || (prCfaMpg->fgIsCPRMDecOn)) {
				if ((prCfaMpg->u8Ca + prCfaMpg->u2PktLen + 6) <
					(prCfaMpg->u8PckPos + 0x80)) {
					prCfaMpg->u2CSSJumpLen =
						(u16) (prCfaMpg->u8PckPos + 0x80 -
							  (prCfaMpg->u8Ca + prCfaMpg->u2PktLen + 6));
				}
			}
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, 6);
			prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_DVDPRV1_CHK_PKTHDR;
			return;
		}	else if (CFA_MPG_SC_PADD_PKT == u4Prfx) {
			/* assume private packet for DVD */
			LOADB_WORD(prCfaMpg->pucHdrBufRp + 4, prCfaMpg->u2PktLen);
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)6);
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64) (prCfaMpg->u2PktLen));
			prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
			return;
		}
#if CFA_MPG_SUPPORT_AVC
		else if ((CFA_MPG_SC_PG_STRM_MAP == u4Prfx)
			 && (prCfaMpg->eCfaMpgMediumType == CFA_MPG_MED_TYPE_FILE)) {
			/* parse program_stream_map stream_type*/
			LOADB_WORD(prCfaMpg->pucHdrBufRp + 4, prCfaMpg->u2PktLen);
			if (fgCfaMpgChkAVC(prCfaMpg)) {
				DmxLogT(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
						TEXT("... [bean] find AVC Success! ...\n"));
			}
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, 6);
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64) (prCfaMpg->u2PktLen));
			prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
			return;
		} else {
			/*do nothing*/
		}
#endif
	} else	{
	/* the file isn't from DVD*/
		if ((u4Prfx & CFA_MPG_MPA_PKT_MSK) == CFA_MPG_SC_AUD_PKT) {
			u16 u2AudId = 0;

			u2AudId = (u16) ((u4Prfx & (u32)0x000000ff) << 8U);
			CfaMpgChkAudStrmInf(prCfaMpg, u2AudId, 0);

			/* load pktlen for search efficiently, 20080330*/
			LOADB_WORD(prCfaMpg->pucHdrBufRp + 4, prCfaMpg->u2PktLen);
			if (prCfaMpg->u2PktLen == 0) {
				/* may parsing wrong data, need to skip it. 080406*/
				CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, 4);
				return;
			}
			if (TRUE == CfaMpgChkPrsAudPkt(prCfaMpg)) {
				prCfaMpg->u2PrsAudStId = u2AudId;
				prCfaMpg->eCurPrsPktType = CFA_MPG_PRS_BIT_STRM_TYPE_A;

				CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, 6);
				prCfaMpg->u4PESHdrDataLen = 0;

				/* This "if" must be TRUE if ASSUME_MP2_PS_FROM_DVD_VIDEO is defined...*/
				if (CFA_MPG_SYS_STRM_TYPE_ISO11172_SYS_STRM ==
					prCfaMpg->eCfaMpgSysStrmType) {
					prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_M1_CHK_PKTHDR;
				} else {
					prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_M2_CHK_PKTHDR;
				}
				return;
			}
		}
	}

	/*
	   // comment this for MPEG2 file parsing (fix SVCD mosaic drop pic bug)
	   if ((0xFFFFFF00 & u4Prfx) == 0x00000100) // other streams
	   {
	   LOADB_WORD(prCfaMpg->pucHdrBufRp + 4, prCfaMpg->u2PktLen);
	   CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)(6 + prCfaMpg->u2PktLen));
	   }
	   else
	 */
	/* bad stream, start to search pack header*/
	{
		/* unknown audio stream in DVD, 20080318*/
		if ((0xFFFFFFF0 & u4Prfx) == 0x000001D0) {
			if (prCfaMpg->u4RdyDataSz >= 6) {
				LOADB_WORD(prCfaMpg->pucHdrBufRp + 4, prCfaMpg->u2PktLen);
				CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg,
							   (u64) (6 + prCfaMpg->u2PktLen));
				return;
			}
		}

		if (prCfaMpg->u4RdyDataSz >= 5) {
			LOADB_DWRD(prCfaMpg->pucHdrBufRp + 1, u4Prfx);
			if (CFA_MPG_SC_PACK == u4Prfx) {
				/* keep current pack address*/
				prCfaMpg->u8PckPos = prCfaMpg->u8Ca + 1;

				/* Pack header starts from _pbHdrBuf[1].*/
				/* Check version information in _pbHdrBuf[5]*/
				/* reserve 1 SCR byte, move 4 more SCR byte*/
				CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, 5);
				prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_STRM_VER;
				return;
			}

			if (prCfaMpg->u4RdyDataSz >= 6) {
				LOADB_DWRD(prCfaMpg->pucHdrBufRp + 2, u4Prfx);
				if (CFA_MPG_SC_PACK == u4Prfx) {
					/* keep current pack address*/
					prCfaMpg->u8PckPos = prCfaMpg->u8Ca + 2;

					/* Pack header starts from _pbHdrBuf[2].*/
					/* Issue next header information for version information.*/
					CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, 6);
					prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_STRM_VER;
					return;
				}
			}
		}

		if (prCfaMpg->eCfaMpgMediumType == CFA_MPG_MED_TYPE_FILE) {
			/* add this parsing rule for MPEG2 file*/
			/* skip pktlen for search efficiently, 20080330*/
			LOADB_DWRD(prCfaMpg->pucHdrBufRp, u4Prfx);
			if (CFA_MPG_SC_PADD_PKT == u4Prfx)
				LOADB_WORD(prCfaMpg->pucHdrBufRp + 4, prCfaMpg->u2PktLen);
			if (prCfaMpg->u2PktLen != 0)
				CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, prCfaMpg->u2PktLen);
			 else
				CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, 1);

			prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
		}
		/* reset after skip pktlen for search efficiently, 20080330*/
		prCfaMpg->u2PktLen = 0;
	}
}

/* MPG CFA processes CFA_MPG_ANA_ST_M2_CHK_PCKSTF_LEN*/
/* @return None*/
static void CfaMpgAnaStM2ChkPckStfLen(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
	/* check if finishing to parse bitstream*/
	if ((u64) DMX_INVALID_UINT64 == prCfaMpg->u8Ca) {
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
		return;
	}
	/* check if data is enough to be parsed*/
	if (FALSE == CfaMpgChkPrsDataRdy(1, prCfaMpg)) {
		CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
		return;
	}

	CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)(((u8)0x07 & prCfaMpg->pucHdrBufRp[0]) + (u8)1));
	prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
}

/* MPG CFA processes CFA_MPG_ANA_ST_M2_CHK_PKTHDR*/
/* @return None*/
static void CfaMpgAnaStM2ChkPktHdr(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
#if CFA_MPG_HIGH_BIT_RATE_HANDLE
	u32 u4SeqCode = 0;
#endif
	/* check if finishing to parse bitstream*/
	if ((u64) DMX_INVALID_UINT64 == prCfaMpg->u8Ca) {
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
		return;
	}
	/* check if data is enough to be parsed*/
	if (FALSE == CfaMpgChkPrsDataRdy(3, prCfaMpg)) {
		CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
		return;
	}

	prCfaMpg->fgExistDts = FALSE;
	prCfaMpg->fgExistPts = FALSE;
	prCfaMpg->u4PESHdrDataLen = prCfaMpg->pucHdrBufRp[2];
#if CFA_MPG_HIGH_BIT_RATE_HANDLE
	if ((prCfaMpg->u4BitRate == 0) && (prCfaMpg->eCfaMpgMediumType == CFA_MPG_MED_TYPE_FILE) &&
		(prCfaMpg->u4CurQueryInfType != 0)) {
		if (FALSE == CfaMpgChkPrsDataRdy(prCfaMpg->u4PESHdrDataLen + 14, prCfaMpg)) {
			CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
			return;
		}
		u4SeqCode = *((u32 *) (prCfaMpg->pucHdrBufRp + prCfaMpg->u4PESHdrDataLen + 3));
		if (u4SeqCode == 0xb3010000) {
			LOADB_DWRD(prCfaMpg->pucHdrBufRp + prCfaMpg->u4PESHdrDataLen + 11,
				   prCfaMpg->u4BitRate);
			prCfaMpg->u4BitRate = prCfaMpg->u4BitRate >> 14;
		}
	}
	if ((prCfaMpg->fgSupportHibitRt == TRUE)
		&& (prCfaMpg->u8Ca + 1024 * 4 < prCfaMpg->rCfaRange.u8Ea)
		&& (prCfaMpg->u4CurQueryInfType == 0)) {
		if (FALSE == CfaMpgChkPrsDataRdy(prCfaMpg->u2PktLen, prCfaMpg)) {
			if (prCfaMpg->u4RdyDataSz >= CFA_MPG_HDR_BUF_SZ) {
				if (prCfaMpg->u2CmdQIndex != 0) {
					CfaMpgCmdQTxing(pvSptHdl, prCfaMpg);
					prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_M2_CHK_PKTHDR;
					return;
				}
			} else {
				CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
				return;
			}
		}
	}
	prCfaMpg->fgTxByPbbuf = TRUE;
#endif
	if ((u8)0x80 == ((u8)0xC0 & prCfaMpg->pucHdrBufRp[1])) {
		/* Only PTS exist*/
		prCfaMpg->fgExistDts = FALSE;
		prCfaMpg->fgExistPts = TRUE;

		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)3);
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_M2_GETPTS;
	} else if ((u8)0xC0 == ((u8)0xC0 & prCfaMpg->pucHdrBufRp[1])) {
		/* PTS/DTS exist*/
		prCfaMpg->fgExistDts = TRUE;
		prCfaMpg->fgExistPts = TRUE;

		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)3);
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_M2_GETPTS;
	} else {
		if ((TRUE == CfaMpgChkTxPkt(prCfaMpg)) && (prCfaMpg->u2PktLen >= (3 + prCfaMpg->pucHdrBufRp[2]))) {
			/*error handle: skip the packet length is invalid.*/
			prCfaMpg->u8PrsPts = INVALID_TIMESTAMP;
			prCfaMpg->u8PrsDts = INVALID_TIMESTAMP;
			CfaMpgSetEsPts(prCfaMpg);
			CfaMpgSetEsDts(prCfaMpg);
			DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					TEXT("\r\nCfaMpgAnaStM2ChkPktHdr: Tx data to Fifo\r\n"));
			CfaMpgTxNextStrmDataToFifo(pvSptHdl, prCfaMpg, (u64)((u8)3 + prCfaMpg->pucHdrBufRp[2]),
						   (u64)(prCfaMpg->u2PktLen - (u16)3 -
						   prCfaMpg->pucHdrBufRp[2]));
		} else {
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64) (prCfaMpg->u2PktLen));
			prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
		}
	}
}


/* MPG CFA processes CFA_MPG_ANA_ST_M2_GETPTS*/
/* @return None*/
/*< [IN] handle of fdmx*/
/*< [IN] Actual transferred data length.
	Normally this value should be equal to the u8TxLen in the previous transfer issue,
	unless file end is hit.*/
/*< [IN] pointer to CfaMpgInst*/
static void CfaMpgAnaStM2GetPts(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
	u32 u4Tmp = 0;

	/* check if finishing to parse bitstream*/
	if ((u64) DMX_INVALID_UINT64 == prCfaMpg->u8Ca) {
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
		return;
	}
	/* check if data is enough to be parsed*/
	if (TRUE == prCfaMpg->fgExistDts)
		u4Tmp = (u32)10;
	 else
		u4Tmp = (u32)5;

	if (FALSE == CfaMpgChkPrsDataRdy(u4Tmp, prCfaMpg)) {
		CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
		return;
	}

	CfaMpgGetPktTS(&(prCfaMpg->u8PrsPts), prCfaMpg->pucHdrBufRp);

	/*adjust PTS/DTS/SCR by StcOffset*/
	if ((prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK) ||
		(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK_RVS) ||
		(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK_RVS_EX) ||
		(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_VALUE) ||
		(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_KEEP_CURRENT)) {
		/*u64 u8Temp = 0;*/
		if (prCfaMpg->u8PrsPts != INVALID_TIMESTAMP) {
			CfaMpgSTCAdd(prCfaMpg->u8PrsPts, (u64) prCfaMpg->i8StcOffset,
					 &(prCfaMpg->u8PrsPts));
		}
	}

	CfaMpgSetEsPts(prCfaMpg);

	if (CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_PTS_INF & prCfaMpg->u4CurQueryInfType)
		CfaMpgQIFoundFirstMpegPtsInf(pvSptHdl, prCfaMpg);
	if (TRUE == CfaMpgChkTxPkt(prCfaMpg)) {
		if (TRUE == prCfaMpg->fgExistDts) {
			/* Get DTS for Video AV Sync*/
			CfaMpgGetPktTS(&(prCfaMpg->u8PrsDts), prCfaMpg->pucHdrBufRp + 5);
			/*adjust PTS/DTS/SCR by StcOffset*/
			if ((prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK) ||
				(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK_RVS) ||
				(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK_RVS_EX) ||
				(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_VALUE) ||
				(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_KEEP_CURRENT)) {
				if (prCfaMpg->u8PrsDts != INVALID_TIMESTAMP)
					CfaMpgSTCAdd(prCfaMpg->u8PrsDts,
							 (u64) prCfaMpg->i8StcOffset,
							 &(prCfaMpg->u8PrsDts));
			}
			CfaMpgSetEsDts(prCfaMpg);
		}
		/* Skip the packet, when packet length < PED header data length @pingzhao,2008/10/29*/
		if ((u32) prCfaMpg->u2PktLen < (prCfaMpg->u4PESHdrDataLen + (u32)3)) {
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64) (prCfaMpg->u2PktLen - 3));
			prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
		} else {
			DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					TEXT("\r\nCfaMpgAnaStM2GetPts: Tx data to Fifo\r\n"));
			CfaMpgTxNextStrmDataToFifo(pvSptHdl, prCfaMpg,
						   (u64) prCfaMpg->u4PESHdrDataLen,
						   (u64) ((u32) prCfaMpg->u2PktLen - (u32)3 -
								 (u32)prCfaMpg->u4PESHdrDataLen));
		}
	} else {
	/* information query mode skip data transfer*/
		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64) (prCfaMpg->u2PktLen - 3));
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
	}
}


/* MPG CFA processes CFA_MPG_ANA_ST_M1_CHK_PKTHDR*/
/* @return None*/
static void CfaMpgAnaStM1ChkPktHdr(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
#if CFA_MPG_HIGH_BIT_RATE_HANDLE
	/*u32 u4SeqCode = 0;*/
	u8 auMatchScLst[2] = { MPEG_SC_POSTFIX_SEQ_HDR, 0 };
	u32 u4HdrAdr = 0;
#endif

	/* check if finishing to parse bitstream*/
	if ((u64) DMX_INVALID_UINT64 == prCfaMpg->u8Ca) {
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
		return;
	}
	/* check if data is enough to be parsed*/
	if (FALSE == CfaMpgChkPrsDataRdy(3, prCfaMpg)) {
		CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
		return;
	}

	prCfaMpg->fgExistDts = FALSE;
	prCfaMpg->fgExistPts = FALSE;
#if CFA_MPG_HIGH_BIT_RATE_HANDLE
	if ((prCfaMpg->u4BitRate == 0) && (prCfaMpg->eCfaMpgMediumType == CFA_MPG_MED_TYPE_FILE) &&
		(prCfaMpg->u4CurQueryInfType != 0)) {
		if (FALSE == CfaMpgChkPrsDataRdy(prCfaMpg->u4PESHdrDataLen + 28, prCfaMpg)) {
			CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
			return;
		}
		u4HdrAdr = CfaMpgScSearch(prCfaMpg->pucHdrBufRp, (u32)28, auMatchScLst, (u32)1);
		/*u4SeqCode = *((u32*)(prCfaMpg->pucHdrBufRp + prCfaMpg->u4PESHdrDataLen + 3));*/
		/*if (u4SeqCode == 0xb3010000)*/
		if (DMX_INVALID_UINT32 != u4HdrAdr) {
			LOADB_DWRD(prCfaMpg->pucHdrBufRp + prCfaMpg->u4PESHdrDataLen + (u32)8 + u4HdrAdr,
				   prCfaMpg->u4BitRate);
			prCfaMpg->u4BitRate = prCfaMpg->u4BitRate >> 14;
		}
	}

	if ((prCfaMpg->fgSupportHibitRt == TRUE)
		&& (prCfaMpg->u8Ca + 1024 * 4 < prCfaMpg->rCfaRange.u8Ea)
		&& (prCfaMpg->u4CurQueryInfType == 0)) {
		if (FALSE == CfaMpgChkPrsDataRdy(prCfaMpg->u2PktLen, prCfaMpg)) {
			if (prCfaMpg->u4RdyDataSz >= CFA_MPG_HDR_BUF_SZ) {
				if (prCfaMpg->u2CmdQIndex != 0) {
					CfaMpgCmdQTxing(pvSptHdl, prCfaMpg);
					prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_M1_CHK_PKTHDR;
					return;
				}
			} else {
				CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
				return;
			}
		}
	}
	prCfaMpg->fgTxByPbbuf = TRUE;
#endif
	if ((u8)0xff == prCfaMpg->pucHdrBufRp[0]) {
		/* stuffing byte*/
		prCfaMpg->u4PESHdrDataLen += 1;
		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)1);
	} else if ((u8)0x40 == (prCfaMpg->pucHdrBufRp[0] & (u8)0xc0)) {
		/* STD buffer scale/size exists*/
		/* check if data is enough to be parsed*/
		prCfaMpg->u4PESHdrDataLen += 2;
		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)2);
	} else if ((u8)0x20 == (prCfaMpg->pucHdrBufRp[0] & (u8)0xf0)) {
		/* PTS exists*/
		prCfaMpg->fgExistPts = TRUE;
		prCfaMpg->u4PESHdrDataLen += 5;

		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_M1_GETPTS;
	} else if ((u8)0x30 == (prCfaMpg->pucHdrBufRp[0] & (u8)0xf0)) {
		/* PTS/DTS exists*/
		prCfaMpg->fgExistDts = TRUE;
		prCfaMpg->fgExistPts = TRUE;
		prCfaMpg->u4PESHdrDataLen += 10;

		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_M1_GETPTS;
	} else if ((u8)0x0f == prCfaMpg->pucHdrBufRp[0]) {
		/* no PTS*/
		prCfaMpg->u8PrsPts = INVALID_TIMESTAMP;
		prCfaMpg->u8PrsDts = INVALID_TIMESTAMP;
		CfaMpgSetEsPts(prCfaMpg);
		CfaMpgSetEsDts(prCfaMpg);
		prCfaMpg->u4PESHdrDataLen += 1;

		if (TRUE == CfaMpgChkTxPkt(prCfaMpg)) {
			DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					TEXT("\r\nCfaMpgAnaStM1ChkPktHdr: Tx data to Fifo\r\n"));
			CfaMpgTxNextStrmDataToFifo(pvSptHdl, prCfaMpg, (u64)1,
						   (u64) ((u32) prCfaMpg->u2PktLen -
								 prCfaMpg->u4PESHdrDataLen));
		} else {
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg,
						   (u64) ((u32) prCfaMpg->u2PktLen -
								 prCfaMpg->u4PESHdrDataLen + (u32)1));
			prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
		}
	} else {
		/* error case*/
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_SEARCH_PCK;
	}
}

/* MPG CFA processes CFA_MPG_ANA_ST_M1_GETPTS*/
/* @return None*/
static void CfaMpgAnaStM1GetPts(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
	u32 u4Tmp = 0;

	/* check if finishing to parse bitstream*/
	if ((u64) DMX_INVALID_UINT64 == prCfaMpg->u8Ca) {
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
		return;
	}
	/* check if data is enough to be parsed*/
	if (TRUE == prCfaMpg->fgExistDts)
		u4Tmp = (u32)10;
	 else
		u4Tmp = (u32)5;

	if (FALSE == CfaMpgChkPrsDataRdy(u4Tmp, prCfaMpg)) {
		CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
		return;
	}

	CfaMpgGetPktTS(&(prCfaMpg->u8PrsPts), prCfaMpg->pucHdrBufRp);
	/*adjust PTS/DTS/SCR by StcOffset*/
	if ((prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK) ||
		(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK_RVS) ||
		(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK_RVS_EX) ||
		(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_VALUE) ||
		(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_KEEP_CURRENT)) {
		if (prCfaMpg->u8PrsPts != INVALID_TIMESTAMP)
			CfaMpgSTCAdd(prCfaMpg->u8PrsPts, (u64) prCfaMpg->i8StcOffset,
					 &(prCfaMpg->u8PrsPts));
	}

	CfaMpgSetEsPts(prCfaMpg);

	if (CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_PTS_INF & prCfaMpg->u4CurQueryInfType)
		CfaMpgQIFoundFirstMpegPtsInf(pvSptHdl, prCfaMpg);

	if (TRUE == CfaMpgChkTxPkt(prCfaMpg)) {
		if (TRUE == prCfaMpg->fgExistDts) {
			CfaMpgGetPktTS(&(prCfaMpg->u8PrsDts), prCfaMpg->pucHdrBufRp + 5);
			/*adjust PTS/DTS/SCR by StcOffset*/
			if ((prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK) ||
				(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK_RVS) ||
				(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK_RVS_EX) ||
				(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_VALUE) ||
				(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_KEEP_CURRENT)) {
				if (prCfaMpg->u8PrsDts != INVALID_TIMESTAMP)
					CfaMpgSTCAdd(prCfaMpg->u8PrsDts,
							 (u64) prCfaMpg->i8StcOffset,
							 &(prCfaMpg->u8PrsDts));
			}
			CfaMpgSetEsDts(prCfaMpg);

			DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					TEXT("\r\nCfaMpgAnaStM1GetPts:DTS exist Tx data to Fifo\r\n"));
			CfaMpgTxNextStrmDataToFifo(pvSptHdl, prCfaMpg, (u64)10,
						   (u64) ((u32) prCfaMpg->u2PktLen -
								 prCfaMpg->u4PESHdrDataLen));
		} else {
			DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					TEXT("\r\nCfaMpgAnaStM1GetPts: Tx data to Fifo\r\n"));
			CfaMpgTxNextStrmDataToFifo(pvSptHdl, prCfaMpg, (u64)5,
						   (u64) ((u32) prCfaMpg->u2PktLen -
								 prCfaMpg->u4PESHdrDataLen));
		}
	} else {
		if (TRUE == prCfaMpg->fgExistDts) {
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg,
						   (u64) ((u32) prCfaMpg->u2PktLen -
								 prCfaMpg->u4PESHdrDataLen + (u32)10));
		} else {
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg,
						   (u64) ((u32) prCfaMpg->u2PktLen -
								 prCfaMpg->u4PESHdrDataLen + (u32)5));
		}

		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
	}
}


/* MPG CFA processes CFA_MPG_ANA_ST_DVDPRV1_CHK_PKTHDR*/
/* @return None*/
static void CfaMpgAnaStDVDPrv1ChkPktHdr(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
	if (FALSE == CfaMpgChkPrsDataRdy(3, prCfaMpg)) {
		CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
		return;
	}

	if ((prCfaMpg->u2PktLen - prCfaMpg->pucHdrBufRp[2]) < 3) {
		/* error; skip this packet , Rp[2] = u4PESHdrDataLen*/
		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64) (prCfaMpg->u2PktLen));
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
	} else {
		prCfaMpg->fgExistDts = FALSE;
		if ((u8)0x80 & prCfaMpg->pucHdrBufRp[1])
			prCfaMpg->fgExistPts = TRUE;
		 else
			prCfaMpg->fgExistPts = FALSE;

		prCfaMpg->u4PESHdrDataLen = prCfaMpg->pucHdrBufRp[2];
		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)3);
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_DVDPRV1_CHK_SUBSTID;
	}
}


/* MPG CFA processes CFA_MPG_ANA_ST_DVDPRV1_CHK_SUBSTID*/
/* @return None*/
static void CfaMpgAnaStDVDPrv1ChkSubStId(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
	u8 ucSubStId = 0;

	if (FALSE == CfaMpgChkPrsDataRdy(prCfaMpg->u4PESHdrDataLen + 4, prCfaMpg)) {
		CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
		return;
		}

	ucSubStId = prCfaMpg->pucHdrBufRp[prCfaMpg->u4PESHdrDataLen];

	if ((ucSubStId & (u8)A_PKT_MSK) == (u8)PCM_SUBST_SC) {
		prCfaMpg->eCurPrsPktType = CFA_MPG_PRS_BIT_STRM_TYPE_A;
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_DVDPRV1_LPCM_PARM;
	} else if (((ucSubStId & (u8)A_PKT_MSK) == (u8)AC3_SUBST_SC) ||
		   ((ucSubStId & (u8)A_PKT_MSK) == (u8)DTS_SUBST_SC)) {
		prCfaMpg->eCurPrsPktType = CFA_MPG_PRS_BIT_STRM_TYPE_A;
		prCfaMpg->u2PrsAudStId =
			(((u16)(((u16) MPEG_SC_POSTFIX_PRV1_PKT) << 8)) | ((u16) ucSubStId));
		CfaMpgChkAudStrmInf(prCfaMpg, prCfaMpg->u2PrsAudStId, (u8)0);
		if (TRUE == CfaMpgChkPrsAudPkt(prCfaMpg)) {
			prCfaMpg->u8PrsPts = INVALID_TIMESTAMP;
			prCfaMpg->u8PrsDts = INVALID_TIMESTAMP;

			if (TRUE == prCfaMpg->fgExistPts) {
				CfaMpgGetPktTS(&(prCfaMpg->u8PrsPts), prCfaMpg->pucHdrBufRp);
				/*adjust PTS/DTS/SCR by StcOffset*/
				if ((prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK)
					|| (prCfaMpg->rCfaRange.ePtsAdjustType ==
					MPG_PTS_ADJUST_BY_NV_PCK_RVS)
					|| (prCfaMpg->rCfaRange.ePtsAdjustType ==
					MPG_PTS_ADJUST_BY_NV_PCK_RVS_EX)
					|| (prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_VALUE)
					|| (prCfaMpg->rCfaRange.ePtsAdjustType ==
					MPG_PTS_ADJUST_KEEP_CURRENT)) {
					if (prCfaMpg->u8PrsPts != INVALID_TIMESTAMP)
						CfaMpgSTCAdd(prCfaMpg->u8PrsPts,
								 (u64) prCfaMpg->i8StcOffset,
								 &(prCfaMpg->u8PrsPts));
				}

				if (CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_PTS_INF & prCfaMpg->
					u4CurQueryInfType) {
					CfaMpgQIFoundFirstMpegPtsInf(pvSptHdl, prCfaMpg);
				}
			}
			CfaMpgSetEsPts(prCfaMpg);

			if (TRUE == CfaMpgChkTxAudPkt(prCfaMpg)) {
				DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
						TEXT
						("\r\nCfaMpgAnaStDVDPrv1ChkSubStId: Tx data to fifo\r\n"));
				CfaMpgTxNextStrmDataToFifo(pvSptHdl, prCfaMpg,
							   (u64)(prCfaMpg->u4PESHdrDataLen + (u32)4),
							   (u64) ((u32) prCfaMpg->u2PktLen -
									 (u32)3 - prCfaMpg->u4PESHdrDataLen -
									 (u32)4));
			} else {	/* information query mode skip data transfer*/
				if ((prCfaMpg->fgIsCSSDecOn) || (prCfaMpg->fgIsCPRMDecOn)) {
					prCfaMpg->u2PktLen =
						(u16) (prCfaMpg->u2PktLen + prCfaMpg->u2CSSJumpLen);
					prCfaMpg->u2CSSJumpLen = 0;
					prCfaMpg->fgIsCSSDecOn = FALSE;
					prCfaMpg->fgIsCPRMDecOn = FALSE;
				}
				CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg,
							   (u64) (prCfaMpg->u2PktLen - 3));
				prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
			}
		} else {	/* skip this packet*/
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64) (prCfaMpg->u2PktLen - 3));
			prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
		}
	} else if ((ucSubStId & (u8)SP_PKT_MSK) == (u8)SP_SUBST_SC) {
		prCfaMpg->eCurPrsPktType = CFA_MPG_PRS_BIT_STRM_TYPE_SP0;
		prCfaMpg->ucPrsSpStId = ucSubStId;
		CfaMpgChkSpStrmInf(pvSptHdl, prCfaMpg, ucSubStId);
		if (TRUE == CfaMpgChkPrsSpPkt(prCfaMpg)) {
			prCfaMpg->u8PrsPts = INVALID_TIMESTAMP;
			prCfaMpg->u8PrsDts = INVALID_TIMESTAMP;

			if (TRUE == prCfaMpg->fgExistPts) {
				CfaMpgGetPktTS(&(prCfaMpg->u8PrsPts), prCfaMpg->pucHdrBufRp);
				/*adjust PTS/DTS/SCR by StcOffset*/
				if ((prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK)
					|| (prCfaMpg->rCfaRange.ePtsAdjustType ==
					MPG_PTS_ADJUST_BY_NV_PCK_RVS)
					|| (prCfaMpg->rCfaRange.ePtsAdjustType ==
					MPG_PTS_ADJUST_BY_NV_PCK_RVS_EX)
					|| (prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_VALUE)
					|| (prCfaMpg->rCfaRange.ePtsAdjustType ==
					MPG_PTS_ADJUST_KEEP_CURRENT)) {
					if (prCfaMpg->u8PrsPts != INVALID_TIMESTAMP)
						CfaMpgSTCAdd(prCfaMpg->u8PrsPts,
								 (u64) prCfaMpg->i8StcOffset,
								 &(prCfaMpg->u8PrsPts));
				}

				if (CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_PTS_INF & prCfaMpg->
					u4CurQueryInfType) {
					CfaMpgQIFoundFirstMpegPtsInf(pvSptHdl, prCfaMpg);
				}

				if (CFA_MPG_PRS_BIT_STRM_TYPE_SP0 == prCfaMpg->eCurPrsPktType) {
					u8 u1Idx;
					u32 u4Cnt;

					prCfaMpg->u2SpuSz =
						(((u16)(((u16) prCfaMpg->
						   pucHdrBufRp[prCfaMpg->u4PESHdrDataLen +
							   1]) << 8)) | ((u16) prCfaMpg->
									pucHdrBufRp[prCfaMpg->
											u4PESHdrDataLen
											+ 2]));
					u4Cnt =
						(prCfaMpg->u4SpstNs >
						 CFA_MPG_MAX_STRM_NS) ? (CFA_MPG_MAX_STRM_NS)
						: (prCfaMpg->u4SpstNs);
				for (u1Idx = 0; u1Idx < u4Cnt; u1Idx++) {
					if (prCfaMpg->ucPrsSpStId == prCfaMpg->arSPInf[u1Idx].ucDecSpstId)
							prCfaMpg->arSPInf[u1Idx].u2SpuSz = prCfaMpg->u2SpuSz;
					}
				}
				CfaMpgSetEsPts(prCfaMpg);
			} else {
				/*prCfaMpg->u2SpuSz = 0;*/
			}

			if (TRUE == prCfaMpg->fgExistDts) {	/* Get DTS for Video AV Sync*/
				CfaMpgGetPktTS(&(prCfaMpg->u8PrsDts), prCfaMpg->pucHdrBufRp + 5);
				/*adjust PTS/DTS/SCR by StcOffset*/
				if ((prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK)
					|| (prCfaMpg->rCfaRange.ePtsAdjustType ==
					MPG_PTS_ADJUST_BY_NV_PCK_RVS)
					|| (prCfaMpg->rCfaRange.ePtsAdjustType ==
					MPG_PTS_ADJUST_BY_NV_PCK_RVS_EX)
					|| (prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_VALUE)
					|| (prCfaMpg->rCfaRange.ePtsAdjustType ==
					MPG_PTS_ADJUST_KEEP_CURRENT)) {
					if (prCfaMpg->u8PrsDts != INVALID_TIMESTAMP)
						CfaMpgSTCAdd(prCfaMpg->u8PrsDts,
								 (u64) prCfaMpg->i8StcOffset,
								 &(prCfaMpg->u8PrsDts));

				}
				CfaMpgSetEsDts(prCfaMpg);
			}

			if ((CFA_MPG_PRS_BIT_STRM_TYPE_SP0 == prCfaMpg->eCurPrsPktType) &&
				(CFA_MPG_PRS_BIT_STRM_TYPE_SP0 & prCfaMpg->u4CurPrsFlg)) {
				CfaMpgTxNextStrmDataToFifo(pvSptHdl, prCfaMpg,
							   (u64)(prCfaMpg->u4PESHdrDataLen + (u32)1),
							   (u64) ((u32) prCfaMpg->u2PktLen -
									 (u32)3 - prCfaMpg->u4PESHdrDataLen -
									 (u32)1));
			} else {	/* information query mode skip data transfer*/
				if ((prCfaMpg->fgIsCSSDecOn) || (prCfaMpg->fgIsCPRMDecOn)) {
					prCfaMpg->u2PktLen =
						(u16) (prCfaMpg->u2PktLen + prCfaMpg->u2CSSJumpLen);
					prCfaMpg->u2CSSJumpLen = 0;
					prCfaMpg->fgIsCSSDecOn = FALSE;
					prCfaMpg->fgIsCPRMDecOn = FALSE;
				}
				CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg,
							   (u64) (prCfaMpg->u2PktLen - 3));
				prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
			}
		} else {	/* skip this packet*/
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64) (prCfaMpg->u2PktLen - 3));
			prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
		}
	}
	/* AC3 payload sync word mapping*/
else if (ucSubStId == (u8)AC3_SYNC_WD1) {
	u8 ucSubStId2 = prCfaMpg->pucHdrBufRp[prCfaMpg->u4PESHdrDataLen + 1];

	if ((ucSubStId2 == (u8)AC3_SYNC_WD2)
		&& (prCfaMpg->eCfaMpgMediumType == CFA_MPG_MED_TYPE_FILE)) {
		prCfaMpg->eCurPrsPktType = CFA_MPG_PRS_BIT_STRM_TYPE_A;
		prCfaMpg->u2PrsAudStId =
			(((u16)(((u16) MPEG_SC_POSTFIX_PRV1_PKT) << 8U)) | (u16)0x80);
		CfaMpgChkAudStrmInf(prCfaMpg, prCfaMpg->u2PrsAudStId, (u8)0);
		if (TRUE == CfaMpgChkPrsAudPkt(prCfaMpg)) {
			prCfaMpg->u8PrsPts = INVALID_TIMESTAMP;
			prCfaMpg->u8PrsDts = INVALID_TIMESTAMP;

			if (TRUE == prCfaMpg->fgExistPts) {
				CfaMpgGetPktTS(&(prCfaMpg->u8PrsPts),
						   prCfaMpg->pucHdrBufRp);
				/*adjust PTS/DTS/SCR by StcOffset*/
				if ((prCfaMpg->rCfaRange.ePtsAdjustType ==
					MPG_PTS_ADJUST_BY_NV_PCK)
					|| (prCfaMpg->rCfaRange.ePtsAdjustType ==
					MPG_PTS_ADJUST_BY_NV_PCK_RVS)
					|| (prCfaMpg->rCfaRange.ePtsAdjustType ==
					MPG_PTS_ADJUST_BY_VALUE)
					|| (prCfaMpg->rCfaRange.ePtsAdjustType ==
					MPG_PTS_ADJUST_KEEP_CURRENT)) {
					if (prCfaMpg->u8PrsPts != INVALID_TIMESTAMP)
						CfaMpgSTCAdd(prCfaMpg->u8PrsPts,
						(u64) prCfaMpg->i8StcOffset,
						&(prCfaMpg->u8PrsPts));
						}

					if (CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_PTS_INF & prCfaMpg->
						u4CurQueryInfType) {
						CfaMpgQIFoundFirstMpegPtsInf(pvSptHdl, prCfaMpg);
					}
				}
				CfaMpgSetEsPts(prCfaMpg);

				if (TRUE == CfaMpgChkTxAudPkt(prCfaMpg)) {
					DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
							TEXT
							("\r\nCfaMpgAnaStDVDPrv1ChkSubStId: Tx data to fifo\r\n"));
					CfaMpgTxNextStrmDataToFifo(pvSptHdl, prCfaMpg,
								   prCfaMpg->u4PESHdrDataLen,
								   (u64) ((u32) prCfaMpg->
										 u2PktLen - (u32)3 -
										 prCfaMpg->
										 u4PESHdrDataLen));
				} else {	/* information query mode skip data transfer*/
					if ((prCfaMpg->fgIsCSSDecOn) || (prCfaMpg->fgIsCPRMDecOn)) {
						prCfaMpg->u2PktLen =
							(u16) (prCfaMpg->u2PktLen +
								  prCfaMpg->u2CSSJumpLen);
						prCfaMpg->u2CSSJumpLen = 0;
						prCfaMpg->fgIsCSSDecOn = FALSE;
						prCfaMpg->fgIsCPRMDecOn = FALSE;
					}
					CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg,
								   (u64) (prCfaMpg->u2PktLen -
										 3));
					prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
				}
			} else {	/* skip this packet*/
				CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg,
							   (u64) (prCfaMpg->u2PktLen - 3));
				prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
			}
		} else {	/* skip this packet*/
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64) (prCfaMpg->u2PktLen - 3));
			prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
		}
	} else {		/* skip this packet*/
		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64) (prCfaMpg->u2PktLen - 3));
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
	}
}


/* MPG CFA processes CFA_MPG_ANA_ST_DVDPRV1_LPCM_PARM*/
/* @return None*/
static void CfaMpgAnaStDVDPrv1LpcmParm(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
	u8 ucSubStId = 0;
	u8 ucAudAttr1 = 0;

	if (FALSE == CfaMpgChkPrsDataRdy(prCfaMpg->u4PESHdrDataLen + 7, prCfaMpg)) {
		CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
		return;
	}

	ucSubStId = prCfaMpg->pucHdrBufRp[prCfaMpg->u4PESHdrDataLen];
	prCfaMpg->eCurPrsPktType = CFA_MPG_PRS_BIT_STRM_TYPE_A;
	prCfaMpg->u2PrsAudStId =
		(((u16)(((u16) MPEG_SC_POSTFIX_PRV1_PKT) << 8)) | ((u16) ucSubStId));
	ucAudAttr1 = prCfaMpg->pucHdrBufRp[prCfaMpg->u4PESHdrDataLen + 4];

	CfaMpgChkAudStrmInf(prCfaMpg, prCfaMpg->u2PrsAudStId,
				prCfaMpg->pucHdrBufRp[prCfaMpg->u4PESHdrDataLen + 5]);
	if (TRUE == CfaMpgChkPrsAudPkt(prCfaMpg)) {
		/*for BDP00117192*/
		prCfaMpg->u8PrsPts = INVALID_TIMESTAMP;
		prCfaMpg->u8PrsDts = INVALID_TIMESTAMP;

		if (TRUE == prCfaMpg->fgExistPts) {
			CfaMpgGetPktTS(&(prCfaMpg->u8PrsPts), prCfaMpg->pucHdrBufRp);
			/*adjust PTS/DTS/SCR by StcOffset*/
			if ((prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK) ||
				(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK_RVS) ||
				(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK_RVS_EX) ||
				(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_VALUE) ||
				(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_KEEP_CURRENT)) {
				if (prCfaMpg->u8PrsPts != INVALID_TIMESTAMP)
					CfaMpgSTCAdd(prCfaMpg->u8PrsPts,
							 (u64) prCfaMpg->i8StcOffset,
							 &(prCfaMpg->u8PrsPts));
			}

			if (CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_PTS_INF & prCfaMpg->u4CurQueryInfType)
				CfaMpgQIFoundFirstMpegPtsInf(pvSptHdl, prCfaMpg);
		}

		CfaMpgSetEsPts(prCfaMpg);

		if (TRUE == CfaMpgChkTxAudPkt(prCfaMpg)) {
			/*check 1st LPCM real tx and difference.*/
			bool fgEmp = ((ucAudAttr1 & (u8)0x80) == (u8)0x80) ? TRUE : FALSE;

			if (((prCfaMpg->fgLpcmEmphasis) || (prCfaMpg->fgLastLpcmEmphasis != fgEmp)) &&
				(CFA_MPG_PRS_BIT_STRM_TYPE_A & prCfaMpg->u4CurPrsFlg) &&
				(prCfaMpg->u2PrsAudStId == prCfaMpg->u2DecAudStId)) {
				/* notify splitter to set emphasis*/
				Spt4CfaSetLpcmEmphasis(pvSptHdl, fgEmp);
				prCfaMpg->fgLastLpcmEmphasis = fgEmp;
				prCfaMpg->fgLpcmEmphasis = FALSE;
			}

			DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
					TEXT("\r\nCfaMpgAnaStDVDPrv1LpcmParm: Tx data to fifo\r\n"));
			CfaMpgTxNextStrmDataToFifo(pvSptHdl, prCfaMpg, (u64)(prCfaMpg->u4PESHdrDataLen + (u32)7),
						   (u64) ((u32) prCfaMpg->u2PktLen - 3 -
								 prCfaMpg->u4PESHdrDataLen - 7));
		} else {
			if ((prCfaMpg->fgIsCSSDecOn) || (prCfaMpg->fgIsCPRMDecOn)) {
				prCfaMpg->u2PktLen =
					(u16) (prCfaMpg->u2PktLen + prCfaMpg->u2CSSJumpLen);
				prCfaMpg->u2CSSJumpLen = 0;
				prCfaMpg->fgIsCSSDecOn = FALSE;
				prCfaMpg->fgIsCPRMDecOn = FALSE;
			}
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64) (prCfaMpg->u2PktLen - 3));
			prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
		}
	} else {		/* skip this packet*/
		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64) (prCfaMpg->u2PktLen - 3));
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
	}
}

/* MPG CFA processes CFA_MPG_ANA_ST_CHK_NV_PKTHDR*/
/* @return None*/
static void CfaMpgAnaStChkNvPktHdr(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
	u32 u4VOBU_V_S_PTM = 0, u4VOBU_V_E_PTM = 0;
	u32 u4VOB_V_S_PTM = 0, u4VOB_V_E_PTM = 0;
	u16 u2VOB_ID = 0;
	u16 u2VOBU_SML_CAT = 0;
	s64 i8TempStcOffset = 0;
	u8 ucSubStId = 0;

	if(prCfaMpg->u8Ca < 14)
	{
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
		return;	    
	}

	/* check if finishing to parse bitstream*/
	if ((u64) DMX_INVALID_UINT64 == prCfaMpg->u8Ca) {
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
		return;
	}
	/* 41 = PCI/RDI pes packet length(2) + subst id(1) +*/
	/* 1. PCI skip(12) + VOBU_S/E_PTM(8) ~ Ext ATR2 2. RDI_GI(16) + DCI_CCI(8), choose the bigger 1.*/
	if (FALSE == CfaMpgChkPrsDataRdy(2006, prCfaMpg)) {
		CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
		return;
	}
	/* load PCI/RDI packet length(0x3D4/0x7D4)*/
	LOADB_WORD(prCfaMpg->pucHdrBufRp, prCfaMpg->u2PktLen);
	/* load subst id (PCI/RDI)*/
	ucSubStId = prCfaMpg->pucHdrBufRp[2];

	/* DVD-Video/+VR*/
	if (ucSubStId == PCI_SUBST_SC) {
		u16 u2VOBU_CAT = 0;
		u8 u1APS = 0;

		/* read VOBU_CAT for APS*/
		LOADB_WORD(prCfaMpg->pucHdrBufRp + 7, u2VOBU_CAT);
		u1APS = (u8) ((u2VOBU_CAT >> 14U) & (u16)0x3);
		/* read VOB_ID*/
		LOADB_WORD(prCfaMpg->pucHdrBufRp + 1013, u2VOB_ID);
		/* read VOBU SML Category*/
		LOADB_WORD(prCfaMpg->pucHdrBufRp + 1021, u2VOBU_SML_CAT);
		/* read VOB_V_S_PTM VOB_V_E_PTM*/
		LOADB_DWRD(prCfaMpg->pucHdrBufRp + 1033, u4VOB_V_S_PTM);
		LOADB_DWRD(prCfaMpg->pucHdrBufRp + 1037, u4VOB_V_E_PTM);
		/* read VOBU_V_S_PTM, VOBU_V_E_PTM*/
		LOADB_DWRD(prCfaMpg->pucHdrBufRp + 15, u4VOBU_V_S_PTM);
		LOADB_DWRD(prCfaMpg->pucHdrBufRp + 19, u4VOBU_V_E_PTM);

		if ((prCfaMpg->eCfaMpgMediumType == CFA_MPG_MED_TYPE_FILE) &&
			(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_NONE) &&
			(prCfaMpg->u4VOBU_V_E_PTM != u4VOBU_V_S_PTM) && (prCfaMpg->u4VOBU_V_S_PTM)) {
			/* cast s64 to fix negative value error*/
			i8TempStcOffset =
				(s64) (prCfaMpg->u4VOBU_V_E_PTM) - (s64) (u4VOBU_V_S_PTM);
			prCfaMpg->i8StcOffset += i8TempStcOffset;

			if (prCfaMpg->u2VOB_ID != u2VOB_ID)
				Spt4CfaPbb2Skip(pvSptHdl, (u64)0, (u64)0);
		}
#if CFA_MPG_SUPPORT_SELFDECTECT_VOB_ILVU
		if (prCfaMpg->u4VOB_V_S_PTM && prCfaMpg->u2VOB_ID != u2VOB_ID &&
			prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK) {
			bool fgCalStcOffset = FALSE;
			/* multi-angle, or seamless playback*/
			if (prCfaMpg->u4VOB_V_S_PTM == u4VOB_V_S_PTM
				&& prCfaMpg->u4VOB_V_E_PTM == u4VOB_V_E_PTM) {
				/* Not ILVU, not seamless angle change, so need stc reset*/
				if ((u2VOBU_SML_CAT & 0x4000) == 0)
					fgCalStcOffset = TRUE;
			} else {
				if (prCfaMpg->u4VOB_V_E_PTM != u4VOB_V_S_PTM)
					fgCalStcOffset = TRUE;
			}

			if (fgCalStcOffset) {
				/* cast s64 to fix negative value error*/
				i8TempStcOffset =
					(s64) (prCfaMpg->u4VOB_V_E_PTM) - (s64) (u4VOB_V_S_PTM);
				prCfaMpg->i8StcOffset += i8TempStcOffset;
			}
		}

		if (prCfaMpg->u4VOB_V_S_PTM && prCfaMpg->u2VOB_ID != u2VOB_ID &&
			(prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK_RVS
			 || prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK_RVS_EX)) {
			bool fgCalStcOffset = FALSE;
			/* multi-angle, or seamless playback*/
			if (prCfaMpg->u4VOB_V_S_PTM == u4VOB_V_S_PTM
				&& prCfaMpg->u4VOB_V_E_PTM == u4VOB_V_E_PTM) {
				/* Not ILVU, not seamless angle change, so need stc reset*/
				if ((u2VOBU_SML_CAT & 0x4000) == 0)
					fgCalStcOffset = TRUE;
			} else {
				if (prCfaMpg->u4VOB_V_S_PTM != u4VOB_V_E_PTM)
					fgCalStcOffset = TRUE;
			}

			if (fgCalStcOffset) {
				/* cast s64 to fix negative value error*/
				i8TempStcOffset =
					(s64) (prCfaMpg->u4VOB_V_S_PTM) - (s64) (u4VOB_V_E_PTM);
				if (prCfaMpg->rCfaRange.ePtsAdjustType ==
					MPG_PTS_ADJUST_BY_NV_PCK_RVS_EX)
					i8TempStcOffset -= prCfaMpg->rCfaRange.i8AdjustValue;
				prCfaMpg->i8StcOffset += i8TempStcOffset;
			}
		}
#else
		if ((prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK) &&
			(prCfaMpg->u2VOB_ID != u2VOB_ID)) {
			u64 u8TempStcOffset = 0;

			CfaMpgSTCSub((u64) (prCfaMpg->u4VOB_V_E_PTM), (u64) (u4VOB_V_S_PTM),
					 &u8TempStcOffset);
			CfaMpgSTCAdd((u64) (prCfaMpg->i8StcOffset), u8TempStcOffset,
					 (u64 *) (&prCfaMpg->i8StcOffset));
		}

		if ((prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK_RVS) &&
			(prCfaMpg->u2VOB_ID != u2VOB_ID)) {
			u64 u8TempStcOffset = 0;

			CfaMpgSTCSub((u64) (prCfaMpg->u4VOB_V_S_PTM), (u64) (u4VOB_V_E_PTM),
					 &u8TempStcOffset);
			CfaMpgSTCAdd((u64) (prCfaMpg->i8StcOffset), u8TempStcOffset,
					 (u64 *) (&prCfaMpg->i8StcOffset));
		}

		if ((prCfaMpg->rCfaRange.ePtsAdjustType == MPG_PTS_ADJUST_BY_NV_PCK_RVS_EX) &&
			(prCfaMpg->u2VOB_ID != u2VOB_ID)) {
			u64 u8TempStcOffset = 0;

			CfaMpgSTCSub((u64) (prCfaMpg->u4VOB_V_S_PTM), (u64) (u4VOB_V_E_PTM),
					 &u8TempStcOffset);
			CfaMpgSTCAdd((u64) (prCfaMpg->i8StcOffset), u8TempStcOffset,
					 (u64 *) (&prCfaMpg->i8StcOffset));
		}
#endif

		prCfaMpg->u4VOBU_V_S_PTM = u4VOBU_V_S_PTM;
		prCfaMpg->u4VOBU_V_E_PTM = u4VOBU_V_E_PTM;
		prCfaMpg->u4VOB_V_S_PTM = u4VOB_V_S_PTM;
		prCfaMpg->u4VOB_V_E_PTM = u4VOB_V_E_PTM;
		prCfaMpg->u2VOB_ID = u2VOB_ID;

		/* from system header, 2(pkt_len) + 1(sub_stream id) +*/
		/* 60(PCI_GI) + 36(AGLI)*/
		/*CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, 99);*/
		/*prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_EXTRACT_NV_HLI;*/
		{
			u16 u2HLI_SS = 0;

			LOADB_WORD(prCfaMpg->pucHdrBufRp + (u8)99, u2HLI_SS);
			if ((u2HLI_SS & (u16)0x3) != 0) {
				LOADB_DWRD(prCfaMpg->pucHdrBufRp + 101, prCfaMpg->u4HLI_S_PTM);
				LOADB_DWRD(prCfaMpg->pucHdrBufRp + 105, prCfaMpg->u4HLI_E_PTM);
			} else {
				prCfaMpg->u4HLI_S_PTM = DMX_INVALID_UINT32;
				prCfaMpg->u4HLI_E_PTM = DMX_INVALID_UINT32;
			}
		}
		/*return;*/
	}
	/* DVD-VR*/
	else if (ucSubStId == RDI_SUBST_SC) {
		u32 u4DCI_LOW = 0, u4DCI_HI = 0;
		/* its StcOffset should be assigned by LPE*/
		/* just load VOBU_S_PTM*/

		LOADB_DWRD(prCfaMpg->pucHdrBufRp + 5, prCfaMpg->u4VOBU_V_S_PTM);
		/* load DCI_CCI*/
		LOADB_DWRD(prCfaMpg->pucHdrBufRp + 19, u4DCI_HI);
		LOADB_DWRD(prCfaMpg->pucHdrBufRp + 23, u4DCI_LOW);
		prCfaMpg->u8DCI_CCI = (((u64) u4DCI_HI) << 32) | (u64) u4DCI_LOW;
		/*prCfaMpg->u8DCI_CCI = ((u64)(*((u64*)(prCfaMpg->pucHdrBufRp + 19))));*/
		LOADB_DWRD(prCfaMpg->pucHdrBufRp + 1998, u4DCI_HI);
		LOADB_DWRD(prCfaMpg->pucHdrBufRp + 2002, u4DCI_LOW);
		prCfaMpg->u8DCI_CCI_Verify = (((u64) u4DCI_HI) << 32) | (u64) u4DCI_LOW;
		/*prCfaMpg->u8DCI_CCI_Verify = (u64)(*((u64*)(prCfaMpg->pucHdrBufRp + 1998)));*/
		/* change state to read DCI_CCI verify data*/
		/* 1998 = 1979 - 8 (DCI verify) + 27 (RDI pes pktlen ~ DCI_CCI)*/
	} else {
		/*do nothing*/
	}

	if (TRUE == CfaMpgChkTxPkt(prCfaMpg)) {
		/* For NV pack, should not remove pack header.*/
		u64 u8DataLen = 2048;
		/*prCfaMpg->u8Ca -= 14; // move current address to start of pack*/

#if (CFA_MPG_SUPPORT_DVD_VOBU_STILL_AUTOPAUSE)
		/* VOBU still pause case*/
		if ((prCfaMpg->fgEverTx) && (prCfaMpg->rCfaRange.fgVobuStill)) {
			Cfa2PsrStrmInfo rAutoPauseStrmInfo = { 0 };

			if (CFA_MPG_PRS_BIT_STRM_TYPE_V & prCfaMpg->u4CurPrsFlg) {
				u32 i;

				rAutoPauseStrmInfo.u4VstrmNs = prCfaMpg->u4VstNs;

				for (i = 0; i < prCfaMpg->u4VstNs; ++i) {
					rAutoPauseStrmInfo.ucDecVidStId[i] =
						prCfaMpg->aucDecVstId[i];
				}
			}
			rAutoPauseStrmInfo.fgVobuStill = TRUE;
			Spt4CfaAutoPause(pvSptHdl, (void *) &rAutoPauseStrmInfo);
		}
#endif
		prCfaMpg->u2SkipLen = 42;
		prCfaMpg->eCurPrsPktType = CFA_MPG_PRS_BIT_STRM_TYPE_NV;

		DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("\r\nCfaMpgAnaStChkNvPktHdr: tx data to Fifo\r\n"));
		CfaMpgTxNextStrmDataToFifo(pvSptHdl, prCfaMpg, 0, u8DataLen);
	} else {
		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64) (2006));
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
	}

}


/* MPG CFA processes CFA_MPG_ANA_ST_SEARCH_VCD*/
/* @return None*/
static void CfaMpgAnaStSearchVCD(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
	u32 u4HdrAdr = 0;

	/* check if data is enough to be parsed*/
	if (FALSE == CfaMpgChkPrsDataRdy(12, prCfaMpg)) {
		CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
		return;
	}

	u4HdrAdr = CfaMpgVcdPatternSearch(prCfaMpg->pucHdrBufRp, prCfaMpg->u4RdyDataSz);

	if (DMX_INVALID_UINT32 != u4HdrAdr)
		/* prefix found*/
	{
		const CFAMPG_VCD_HDRINF *prVCDHdrInf;

		prVCDHdrInf = (const CFAMPG_VCD_HDRINF *)&prCfaMpg->pucHdrBufRp[u4HdrAdr];
		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)u4HdrAdr);

		/* mode 2*/
		if (0x02 == prVCDHdrInf->bDataMode) {
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)16);
			prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_SEARCH_VCDM2SUB;
		} else {
			CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)16);
			prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
		}
		prCfaMpg->u4ParsedBytes += 16;
	} else {		/* search next vcd sync pattern*/
		prCfaMpg->u4ParsedBytes += (prCfaMpg->u4RdyDataSz - 11);
		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)(prCfaMpg->u4RdyDataSz - 11));
	}

}


/* MPG CFA processes CFA_MPG_ANA_ST_SEARCH_VCDM2SUB*/
/* @return None*/
static void CfaMpgAnaStSearchVCDM2SUB(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
	/* check if data is enough to be parsed*/
	if (FALSE == CfaMpgChkPrsDataRdy(8, prCfaMpg)) {
		CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
		return;
	}

	if ((prCfaMpg->pucHdrBufRp[0] != prCfaMpg->pucHdrBufRp[4]) ||
		(prCfaMpg->pucHdrBufRp[1] != prCfaMpg->pucHdrBufRp[5]) ||
		(prCfaMpg->pucHdrBufRp[2] != prCfaMpg->pucHdrBufRp[6]) ||
		(prCfaMpg->pucHdrBufRp[3] != prCfaMpg->pucHdrBufRp[7])) {
		/* formless*/
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
	}
	/* VCD Auto Pause*/
	else if ((FALSE == prCfaMpg->fgNoSupportAutoPause) && (prCfaMpg->pucHdrBufRp[2] & (u8)0x10)) {
		Cfa2PsrStrmInfo rAutoPauseStrmInfo = { 0 };

		if (CFA_MPG_PRS_BIT_STRM_TYPE_V & prCfaMpg->u4CurPrsFlg) {
			u32 i;
			u32 u4Cnt = DMX_MIN(prCfaMpg->u4VstNs, 2);	/*due to ucDecVidStId[2]*/

			rAutoPauseStrmInfo.u4VstrmNs = prCfaMpg->u4VstNs;

			for (i = 0; i < u4Cnt; ++i)
				rAutoPauseStrmInfo.ucDecVidStId[i] = prCfaMpg->aucDecVstId[i];
		}
		if (CFA_MPG_PRS_BIT_STRM_TYPE_A & prCfaMpg->u4CurPrsFlg) {
			u32 j;
			u32 u4Cnt = DMX_MIN(prCfaMpg->u4AstNs, 8);	/*due to u2DecAudStId[8]*/

			rAutoPauseStrmInfo.u4AstrmNs = prCfaMpg->u4AstNs;

			for (j = 0; j < u4Cnt; ++j) {
				rAutoPauseStrmInfo.u2DecAudStId[j] =
					prCfaMpg->arAudInf[j].u2DecAstId;
			}
		}

		Spt4CfaAutoPause(pvSptHdl, (void *) &rAutoPauseStrmInfo);
	} else {
		/*do nothing*/
	}
	
	CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)8);
	prCfaMpg->u4ParsedBytes += 8;
	prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;

}


/* MPG CFA processes CFA_MPG_ANA_ST_CHK_RDI_VERIFY*/
/* @return None*/
static void CfaMpgAnaStChkRDIVerify(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
	u32 u4DCI_HI = 0, u4DCI_LOW = 0;

	/* check if data is enough to be parsed*/
	if (FALSE == CfaMpgChkPrsDataRdy(8, prCfaMpg)) {
		CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
		return;
	}
	/* load DCI_CCI verify data*/
	LOADB_DWRD(prCfaMpg->pucHdrBufRp, u4DCI_HI);
	LOADB_DWRD(prCfaMpg->pucHdrBufRp + 4, u4DCI_LOW);
	prCfaMpg->u8DCI_CCI_Verify = (((u64) u4DCI_HI) << 32) | (u64) u4DCI_LOW;

	if (TRUE == CfaMpgChkTxPkt(prCfaMpg)) {
		/* For RDI pack, should not remove pack header.*/
		u64 u8DataLen = (u64)2048;

		prCfaMpg->u8Ca -= 2040;	/* move current address to start of pack*/

		DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("\r\nCfaMpgAnaStChkRDIVerify: tx data to Fifo\r\n"));
		CfaMpgTxNextStrmDataToFifo(pvSptHdl, prCfaMpg, (u64)0, u8DataLen);
	} else {
		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64) (8));
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
	}
}


/* MPG CFA processes CFA_MPG_ANA_ST_EXTRACT_NV_HLI*/
/* @return None*/
static void CfaMpgAnaStExtractNvHLI(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
	u16 u2HLI_SS = 0;

	/* check if data is enough to be parsed*/
	if (FALSE == CfaMpgChkPrsDataRdy(10, prCfaMpg)) {
		CfaMpgRebufPrsData(pvSptHdl, prCfaMpg);
		return;
	}

	LOADB_WORD(prCfaMpg->pucHdrBufRp, u2HLI_SS);
	if ((u2HLI_SS & (u16)0x3) != 0) {
		LOADB_DWRD(prCfaMpg->pucHdrBufRp + 2, prCfaMpg->u4HLI_S_PTM);
		LOADB_DWRD(prCfaMpg->pucHdrBufRp + 6, prCfaMpg->u4HLI_E_PTM);
	} else {
		prCfaMpg->u4HLI_S_PTM = DMX_INVALID_UINT32;
		prCfaMpg->u4HLI_E_PTM = DMX_INVALID_UINT32;
	}

	prCfaMpg->u8Ca -= 127;	/* move current address to pack system header 28 + 99*/

	if (TRUE == CfaMpgChkTxPkt(prCfaMpg)) {
		/* For NV pack, should not remove pack header.*/
		u64 u8DataLen = (u64)2048;

		prCfaMpg->u8Ca -= 14;	/* move current address to start of pack*/
		DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("\r\nCfaMpgAnaStExtractNvHLI: tx data to Fifo\r\n"));
		CfaMpgTxNextStrmDataToFifo(pvSptHdl, prCfaMpg, (u64)0, u8DataLen);
	} else {
		CfaMpgIncPrsPosAndHdrBufRp(prCfaMpg, (u64)(2034));
		prCfaMpg->eCurCfaMpgAnaSt = CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR;
	}

}

static void CfaMpgAnaStMpgVideo(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
	u32 u4TxLen = (u32)CFA_MPG_PURE_VIDO_TX_LEN;
	MRESULT mrRet = RET_DMX_OK;
	CFA_VIDEO_INFO_T rVidInf = { 0 };

	if (prCfaMpg->u8Ca >= prCfaMpg->rCfaRange.u8Ea) {
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
		prCfaMpg->fgExitTxDoneCtrl = TRUE;
		return;
	}

	if ((prCfaMpg->rCfaRange.u8Ea - prCfaMpg->u8Ca) <= (u64) (u4TxLen))
		u4TxLen = (u32) (prCfaMpg->rCfaRange.u8Ea - prCfaMpg->u8Ca);

	rVidInf.u8FileOfst = prCfaMpg->u8Ca;
	rVidInf.eTxMode = CFA_PTM_EXACT_POS;
	rVidInf.eVidType = CFA_VID_MPEG2;
	rVidInf.u8Len = (u64) u4TxLen;
	rVidInf.u4PrsStrmId = (u32) ((u32)prCfaMpg->ucPrsVidStId << (u32)8);

	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("<CFA> CfaMpgTxNextStrmDataToFifo rVidInf.u8Len 0x%lx\r\n"),
			rVidInf.u8Len);
	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("\r\n<CFA> CfaMpgAnaStMpgVideo rVidInf.u8FileOfst 0x%lx\r\n"),
			rVidInf.u8FileOfst);
	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("\r\n<CFA> CfaMpgAnaStMpgVideo rVidInf.u8Len 0x%lx\r\n"),
			rVidInf.u8Len);
	mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rVidInf);
	if (RET_DMX_OK != mrRet)
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);

	prCfaMpg->u8CfaIssueTxLen = rVidInf.u8Len;
	prCfaMpg->u4AvailSz = 0;
	prCfaMpg->fgExitTxDoneCtrl = TRUE;
	prCfaMpg->u8Ca = prCfaMpg->u8Ca + u4TxLen;

}

/* CFA MPG state process function pointer type*/
typedef void(*FP_CFA_MPG_ST_PROC_FUN) (void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg);

/* CFA MPG state process function pointer array*/
static FP_CFA_MPG_ST_PROC_FUN _apfvCfaMpgStProcFunc[CFA_MPG_ANA_ST_NUM] = {
	&CfaMpgAnaStIdle,
	&CfaMpgAnaStSearchSc,
	&CfaMpgAnaStSearchPck,
	&CfaMpgAnaStChkStrmVer,
	&CfaMpgAnaStChkGnrlPktHdr,
	&CfaMpgAnaStM2ChkPckStfLen,
	&CfaMpgAnaStM2ChkPktHdr,
	&CfaMpgAnaStM2GetPts,
	&CfaMpgAnaStDVDPrv1ChkPktHdr,
	&CfaMpgAnaStDVDPrv1ChkSubStId,
	&CfaMpgAnaStDVDPrv1LpcmParm,
	&CfaMpgAnaStChkNvPktHdr,
	&CfaMpgAnaStM1ChkPktHdr,
	&CfaMpgAnaStM1GetPts,
	&CfaMpgAnaStPktTxing,
	&CfaMpgAnaStSearchVCD,
	&CfaMpgAnaStSearchVCDM2SUB,
	&CfaMpgAnaStChkRDIVerify,
	&CfaMpgAnaStExtractNvHLI, &CfaMpgAnaStMpgVideo
};


/* MPG CFA state control for transfer done*/
/* @return None*/
/* @note This function will be called after a transfer is complete.*/
/*< [IN] handle of fdmx*/
/*< [IN] Actual transferred data length.
	Normally this value should be equal to the u4Len in the previous transfer issue,
	unless file end is hit.*/
/*< [IN] pointer to CfaMpgInst*/
void CfaMpgTxDoneStCtrl(void *pvSptHdl, u64 u8TxLen, CfaMpgInst *prCfaMpg)
{
	CFA_CPS_INFO_T rCPS = { 0 };

    if ((CFA_MPG_ANA_ST_IDLE == prCfaMpg->eCurCfaMpgAnaSt)
        ||(CFA_MPG_ANA_ST_NUM <= prCfaMpg->eCurCfaMpgAnaSt)) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT, 
			TEXT("<CFA MPG> prCfaMpg->eCurCfaMpgAnaSt:%d is not")
			TEXT("correct, call CfaMpgFinishPrs\r\n"), prCfaMpg->eCurCfaMpgAnaSt);
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
		return;
    }

	if ((u64) DMX_INVALID_UINT64 == prCfaMpg->u8Ca) {
		DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT("<CFA MPG> prCfaMpg->u8Ca is invalid!")
			TEXT("call CfaMpgFinishPrs\r\n"));
		CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
		return;
	}
		
	/* check if Tx done results from Tx data to header buffer*/
	if (prCfaMpg->fgTxData2HdrBuf) {
		u32 u4PrevRdyDataSz = prCfaMpg->u4RdyDataSz;

		prCfaMpg->pucHdrBuf = (u8 *) prCfaMpg->ptrPfrMemAddress;
		prCfaMpg->pucHdrBufRp = prCfaMpg->pucHdrBuf;
		prCfaMpg->u4RdyDataSz += (u32) (u8TxLen - u4PrevRdyDataSz);
		if (prCfaMpg->u4RdyDataSz > CFA_MPG_HDR_BUF_SZ) {
			DmxLogE(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("<CFA MPG> prCfaMpg->u4RdyDataSz(%d) > CFA_MPG_HDR_BUF_SZ(%d), call CfaMpgFinishPrs\r\n"),
				prCfaMpg->u4RdyDataSz, CFA_MPG_HDR_BUF_SZ);
			CfaMpgFinishPrs(pvSptHdl, prCfaMpg);
			return;
		}

		if (prCfaMpg->u4AvailSz >= 28 * 1024)
			/* reduce DMA interrupts rate*/
			prCfaMpg->u4RdyDataSz += 24 * 1024;
#if CFA_MPG_HIGH_BIT_RATE_HANDLE
		prCfaMpg->u4AvailSz += (u32) (prCfaMpg->u8CfaIssueTxLen - 1);
#endif
	}
	prCfaMpg->u4ParsedBytes = 0;

	{
		prCfaMpg->u8CfaIssueTxLen = 0;
	}

	/* clear cps(CSS or CPRM) decrypt flag*/
	rCPS.fgOn = FALSE;
	rCPS.u8Offset = 0;
	Spt4CfaTurnCPS(pvSptHdl, &rCPS);
	if (!((TRUE == prCfaMpg->fgTxData2HdrBuf) && (prCfaMpg->fgSupportHibitRt == TRUE))) {
		prCfaMpg->fgIsCSSDecOn = FALSE;
		prCfaMpg->fgIsCPRMDecOn = FALSE;
	}
	prCfaMpg->fgTxData2HdrBuf = FALSE;

#if CFA_MPG_HIGH_BIT_RATE_HANDLE
	if ((prCfaMpg->fgSupportHibitRt) && (prCfaMpg->fgTxData2FIFO == TRUE)) {
		prCfaMpg->u4PtsArrayIndex = 0;

		if (!prCfaMpg->fgIsNoBeyondMaxTx) {
			prCfaMpg->au8FileOffSet[prCfaMpg->u4PtsArrayIndex] = prCfaMpg->u8FileOffSet;
			prCfaMpg->au8FileOffSetPts[prCfaMpg->u4PtsArrayIndex] =
				prCfaMpg->u8FileOffSetPts;
			prCfaMpg->u4PtsArrayIndex++;
			prCfaMpg->fgIsNoBeyondMaxTx = TRUE;
		}

		prCfaMpg->fgTxData2FIFO = FALSE;
		u8TxLen = prCfaMpg->u8LatTxAvaLen;
		if (prCfaMpg->u8Ca + (u64)PSR_RESERVE_FIFO_SPACE - u8TxLen > prCfaMpg->rCfaRange.u8Ea)
			prCfaMpg->fgSupportHibitRt = FALSE;
	}
	if ((prCfaMpg->fgSupportHibitRt) && (prCfaMpg->fgCMDQTx == TRUE)) {
		if (prCfaMpg->fgCMDQFirstFill)
			prCfaMpg->eCurPrsPktType = prCfaMpg->eCMDQCurPrsPktType;
		prCfaMpg->fgCMDQTx = FALSE;
		prCfaMpg->fgCMDQFirstFill = FALSE;
	}
#endif

	DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
			TEXT
			("<CFA MPG> CfaMpgTxDoneStCtrl 1 prCfaMpg->u8Ca: 0x%llx, u8TxLen: 0x%llx\r\n\n"),
			prCfaMpg->u8Ca, u8TxLen);
	while (1) {
		/* parsing the bitstream according to parsing state*/
		DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("<CFA MPG> CfaMpgTxDoneStCtrl %d\r\n"),
				prCfaMpg->eCurCfaMpgAnaSt);
		DmxLogD(DMX_MOD_CFA_MPG, CFA_MPG_LOG_DEFAULT,
				TEXT("<CFA MPG> CfaMpgTxDoneStCtrl prCfaMpg->u8Ca: 0x%llx\r\n"),
				prCfaMpg->u8Ca);
		_apfvCfaMpgStProcFunc[prCfaMpg->eCurCfaMpgAnaSt] (pvSptHdl, u8TxLen, prCfaMpg);

		/* check if exitting the parsing loop to start or finish transferring*/
		if (prCfaMpg->fgExitTxDoneCtrl) {
			prCfaMpg->fgExitTxDoneCtrl = FALSE;
			break;
		}
	}
}
