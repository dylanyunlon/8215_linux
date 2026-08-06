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



#ifndef _CFA_MPG_ST_CTRL_H_
#define _CFA_MPG_ST_CTRL_H_

#include "x_typedef.h"
#include "cfa_mpg.h"

/* MPG CFA MPEG start code*/
#define CFA_MPG_SC_PACK			(0x000001BA)	/*< pack start code*/
#define CFA_MPG_SC_PG_END		(0x000001B9)	/*< program end code*/
#define CFA_MPG_SC_V_PKT		(0x000001E0)	/*< video packet*/
#define CFA_MPG_SC_STN_PKT		(0x000001E1)	/*< normal resolution still*/
#define CFA_MPG_SC_STH_PKT		(0x000001E2)	/*< high resolution still*/
#define CFA_MPG_SC_AUD_PKT		(0x000001C0)	/*< audio packet start code*/
#define CFA_MPG_SC_AUD_EXT_PKT	(0x000001D0)	/*< audio packet start code, mpeg2 extension*/
#define CFA_MPG_SC_PRV1_PKT		(0x000001BD)	/*< private packet 1*/
#define CFA_MPG_SC_PRV2_PKT		(0x000001BF)	/*<	private packet 2*/
#define CFA_MPG_SC_PADD_PKT		(0x000001BE)	/*<	padding packet*/
#define CFA_MPG_SC_NV_PKT		(0x000001BB)	/*<	system header navigation pack*/
#if CFA_MPG_SUPPORT_AVC
#define CFA_MPG_SC_PG_STRM_MAP (0x000001BC)		/*< program_stream_map to get AVC codec by bean.li*/
/* add for AVC playback mask by bean.li*/
#define AVC_PKT_MSK			0x1BE0
#define AAC_PKT_MSK			0x0FC0
#define AAC_TS_PKT_MSK		0x11C0
#define AAC_NOTS_PKT_MSK	0x1CC0
/* audio and video total channel*/
#define CFA_MPG_MAX_CHN			32
#endif
/* MPG CFA MPEG packet mask*/
#define CFA_MPG_DVD_MPA_PKT_MSK	(0xFFFFFFF8)   /*< audio packet mask for DVD*/
#define CFA_MPG_MPA_PKT_MSK		(0xFFFFFFE0)   /*< audio packet mask for MPEG*/
/* MPG CFA MPEG start code postfix*/
#define MPEG_SC_POSTFIX_PACK_HDR	(0xBA)
#define MPEG_SC_POSTFIX_SEQ_HDR		(0xB3)
#define MPEG_SC_POSTFIX_PRV1_PKT	(0xBD)
#define LINEAR_PCM				0xA0
#define PKT_PCM					0xA1
#define RTI_PKT					0x48
/* added by Harrison for DVD audio system header*/
#define MPEG2_SYS_OFST			18
#define SP_PKT_MSK				0xE0
#define SP_SUBST_MSK			0x1F
#define SP_SUBST_SC				0x20
#define A_PKT_MSK				0xF8
#define A_SUBST_MSK				0x07
#define AC3_SUBST_SC			0x80
#define DTS_SUBST_SC			0x88
#define SDDS_SUBST_SC			0x90
#define PCM_SUBST_SC			0xA0
#define AC3_SYNC_WD1			0x0B
#define AC3_SYNC_WD2			0x77
/* added by CCMa for DVD/+VR NV pack packet header*/
#define PCI_SUBST_SC			0x00
#define DSI_SUBST_SC			0x01
/* -VR*/
#define RDI_SUBST_SC			0x50
#define CFA_MPG_AUD_5CH_SUPPORT			1
#define CFA_MPG_VCD_ERR_PTS_DETECT		1
/*add by qq for STC reset flow define -- for BDP00117682*/
#define CFA_MPG_STC_RESET				1
#define MASK_1F							0x1FFFFFFFFLL
#define CfaMpgSTCAdd(u8Stc, u8Value, pu8NewStc)	\
{	\
	if ((pu8NewStc)) { \
		*(pu8NewStc) = ((((u64)u8Stc) & (u64)MASK_1F) + (((u64)u8Value) & (u64)MASK_1F)) & (u64)MASK_1F;\
	}	\
}


#define CfaMpgChkPrsDataRdy(u4MinReqDataSz, prCfaMpg)						\
	(((u4MinReqDataSz) <= (prCfaMpg)->u4RdyDataSz) ? TRUE : FALSE)
bool CfaMpgIncPrsPos(CfaMpgInst *prCfaMpg, u64 u8Len);

#if CFA_MPG_HIGH_BIT_RATE_HANDLE
void CfaMpgCmdQTxing(void *pvSptHdl, CfaMpgInst *prCfaMpg);
#endif
void CfaMpgFinishPrs(void *pvSptHdl, CfaMpgInst *prCfaMpg);
void CfaMpgIncPrsPosAndHdrBufRp(CfaMpgInst *prCfaMpg, u64 u8Len);
void CfaMpgRebufPrsData(void *pvSptHdl, CfaMpgInst *prCfaMpg);
u32 CfaMpgScSearch(const u8 *pucHdrBuf, u32 u4DataSz, const u8 *pucScMatchLst,
	u32	u4ScMatchLstSz);
void CfaMpgQIFoundFirstMpegScInf(void *pvSptHdl, CfaMpgInst *prCfaMpg, u32 u4Sc,
	u64 u8ScPos);
bool CfaMpgChkStrmVer(void *pvSptHdl, CfaMpgInst *prCfaMpg);
void CfaMpgChkVidStrmInf(CfaMpgInst *prCfaMpg, u8 ucVstId);
void CfaMpgChkAudStrmInf(CfaMpgInst *prCfaMpg, u16 u2AstId, u8 ucLpcmAtr);
bool CfaMpgChkPrsAudPkt(CfaMpgInst *prCfaMpg);
bool CfaMpgChkTxPkt(const CfaMpgInst *prCfaMpg);
void CfaMpgSetEsPts(CfaMpgInst *prCfaMpg);
void CfaMpgSetEsDts(CfaMpgInst *prCfaMpg);
void CfaMpgGetPktTS(u64 *pu8Des, const u8 *pucSrc);
void CfaMpgSTCSub(u64 u8Stc, u64 u8Value, u64 *pu8NewStc);
void CfaMpgQIFoundFirstMpegPtsInf(void *pvSptHdl, CfaMpgInst *prCfaMpg);
bool CfaMpgChkTxAudPkt(const CfaMpgInst *prCfaMpg);
void CfaMpgChkSpStrmInf(void *pvSptHdl, CfaMpgInst *prCfaMpg, u8 ucSpStId);
bool CfaMpgChkPrsSpPkt(CfaMpgInst *prCfaMpg);
u32 CfaMpgVcdPatternSearch(const u8 *pucHdrBuf, u32 u4DataSz);

/* MPG CFA re-buffer parsing data*/
/* @return None*/
EXTERN void CfaMpgRebufPrsData(void *pvSptHdl, CfaMpgInst *prCfaMpg);

/* CFA Mpg transfer next stream data to FIFO*/
/* @return None*/
/* @note 1. If file offset is over transfer range, MPG CFA state is changed to*/
/* CFA_MPG_ANA_ST_IDLE and finish the parsing*/
/*		  2. If transfer length make file offset over transfer range,
			MPG CFA state is changed to CFA_MPG_ANA_ST_IDLE and finish the parsing*/
/*		  3. Replace vMpsPrsNextP()*/
/*< [IN] handle of fdmx*/
/*< [IN] pointer to CfaMpgInst*/
/*< [IN] advance file length before transferring next bitstrema data*/
/*< [IN] transfer length*/
EXTERN void CfaMpgTxNextStrmDataToFifo(void *pvSptHdl, CfaMpgInst *prCfaMpg,
	u64 u8AdvLen, u64 u8TxLen);


#endif
/* _CFA_MPG_ST_CTRL_H_*/
