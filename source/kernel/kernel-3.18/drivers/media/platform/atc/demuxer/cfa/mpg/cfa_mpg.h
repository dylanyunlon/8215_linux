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


#ifndef CFA_MPG_H
#define CFA_MPG_H

#include "x_typedef.h"
#include <media/atc/dmx_cfa_mpg.h>
#include "cfa_cfg.h"
#include "dmx_spt_cfa.h"
#include "cfa_if.h"


/* Maximum MPG CFA instance number*/
#define CFA_MPG_MAX_INST_NS					(CONFIG_CFA_MPG_MAX_INST_NS)

/* CFA MPG start code residue number*/
#define CFA_MPG_SC_RESIDUE_NS					(4)

/* CFA MPG header buffer size*/
#define CFA_MPG_HDR_BUF_SZ						(2040)

/* CFA MPG maximum stream number*/
#define CFA_MPG_MAX_STRM_NS					(32)

/* MPG CFA DEMUX ERROR LENGTH*/
#define CFA_MPG_DEMUX_MAX_LEN					(u64)(6*900*1024)


#define CFA_MPG_PURE_VIDO_TX_LEN				(4*1024)


/*mpg high bit rate*/
#if CONFIG_DRV_HIGH_BITRATE_SPECIAL_PROC
#define CFA_MPG_HIGH_BIT_RATE_HANDLE			(1)
#else
#define CFA_MPG_HIGH_BIT_RATE_HANDLE			(0)
#endif
#define CFA_MPG_HIGH_BUF_SIZE					(200*1024)


/* MPG CFA analyze state*/
/*< cfa mpg analysis state number, this is not a state. if you change following enum,must change this value */
#define CFA_MPG_ANA_ST_NUM						(19+1)

#if CFA_MPG_HIGH_BIT_RATE_HANDLE
#define CFA_MPG_TX_HIBI_BUFSIZE		(60 * 1024)
#endif

/* MPG CFA system stream type*/
typedef enum CfaMpgSysStrmType {
	CFA_MPG_SYS_STRM_TYPE_NONE = 0,	/*< none*/
	CFA_MPG_SYS_STRM_TYPE_ISO11172_SYS_STRM = 1,	/*< MPEG-1*/
	CFA_MPG_SYS_STRM_TYPE_ISO13818_PRG_STRM = 2,	/*< MPEG-2*/
	CFA_MPG_SYS_STRM_TYPE_VIDEO_SEQUENCE = 3	/*< MPEG video only*/
} CfaMpgSysStrmType;

/* MPG CFA parsing bitstream type*/
typedef enum CfaMpgPrsBitStrmType {
	CFA_MPG_PRS_BIT_STRM_TYPE_NONE = (0x00),	/*< none*/
	CFA_MPG_PRS_BIT_STRM_TYPE_V = (0x01U << 0),	/*< video*/
	CFA_MPG_PRS_BIT_STRM_TYPE_A = (0x01U << 1),	/*< audio*/
	CFA_MPG_PRS_BIT_STRM_TYPE_SP0 = (0x01U << 2),	/*< sp 0*/
	CFA_MPG_PRS_BIT_STRM_TYPE_NV = (0x01U << 3),	/*< navigation pack*/
	CFA_MPG_PRS_BIT_STRM_TYPE_HD = (0x01U << 4)
} CfaMpgPrsBitStrmType;

/* MPG CFA Medium type*/
typedef enum CfaMpgMediumType {
	CFA_MPG_MED_TYPE_NONE = 0,	/*< none*/
	CFA_MPG_MED_TYPE_FILE = 1,	/*< File*/
	CFA_MPG_MED_TYPE_DVD = 2,	/*< DVD*/
	CFA_MPG_MED_TYPE_VCD = 3,	/*< VCD*/
	CFA_MPG_MED_TYPE_MVR = 4,	/*< -VR*/
	CFA_MPG_MED_TYPE_PVR = 5,	/*< +VR*/
	CFA_MPG_MED_TYPE_SVCD = 6	/*< SVCD*/
} CfaMpgMediumType;


/* MPG CFA parsing video bitstream id*/
enum {
	CFA_MPG_PRS_VID_STRM_ID_NONE = 0x00,
	CFA_MPG_PRS_VID_STRM_ID_MV = 0xE0,
	CFA_MPG_PRS_VID_STRM_ID_STN = 0xE1,
	CFA_MPG_PRS_VID_STRM_ID_STH = 0xE2
};

typedef struct {
	u8 mux_r1;
	u8 mux_r2;
	u8 mux_r3;
} CFAMPG_1_MUXRATE;

typedef struct {
	u8 mux_r1;
	u8 mux_r2;
	u8 mux_r3;
} CFAMPG_2_MUXRATE;

typedef struct {
	u8 marker_bit_0:1;
	u8 PTS_30:2;
	u8 PTS_32:1;
	u8 _001x_:4;
	u8 PTS_22;
	u8 marker_bit_1:1;
	u8 PTS_15:7;
	u8 PTS_7;
	u8 marker_bit_2:1;
	u8 PTS_0:7;
} CFAMPG_PTS_DTS;

typedef union _CFAMPG_TIME_STAMP {
	struct {
		u32 u4LowPart;
		s32 i4HighPart;
	} u;
	s64 i8QuadPart;
} CfaMpgTimeStamp;

typedef struct {
	u8 bDataBlk[15];	/*< vcd data block, 15 bytes*/
	u8 bDataMode;		/*< data mode, 1 byte*/
} CFAMPG_VCD_HDRINF;


typedef enum CfaMpgAnaSt {
	CFA_MPG_ANA_ST_IDLE = 0,	/*< Idle*/
	CFA_MPG_ANA_ST_SEARCH_SC = 1,	/*< search start code*/
	CFA_MPG_ANA_ST_SEARCH_PCK = 2,	/*< search pack*/
	CFA_MPG_ANA_ST_CHK_STRM_VER = 3,	/*< check stream version*/
	CFA_MPG_ANA_ST_CHK_GNRL_PKTHDR = 4,	/*< check general packet header*/
	CFA_MPG_ANA_ST_M2_CHK_PCKSTF_LEN = 5,	/*< remove pack stuff for MPEG-2*/
	CFA_MPG_ANA_ST_M2_CHK_PKTHDR = 6,	/*< check packet header for MPEG-2*/
	CFA_MPG_ANA_ST_M2_GETPTS = 7,	/*< get PTS for MPEG-2*/
	CFA_MPG_ANA_ST_DVDPRV1_CHK_PKTHDR = 8,	/*< check private packet 1 header of DVD*/
	CFA_MPG_ANA_ST_DVDPRV1_CHK_SUBSTID = 9,	/*< check sub-stream id for private packet 1 header of DVD*/
	CFA_MPG_ANA_ST_DVDPRV1_LPCM_PARM = 10,	/* DVD VOB only*/
	CFA_MPG_ANA_ST_CHK_NV_PKTHDR = 11,	/*< packet transferring*/
	CFA_MPG_ANA_ST_M1_CHK_PKTHDR = 12,	/*< check packet header for MPEG-1*/
	CFA_MPG_ANA_ST_M1_GETPTS = 13,	/*< get PTS for MPEG-1*/
	CFA_MPG_ANA_ST_PKT_TXING = 14,	/*< packet transferring*/
	CFA_MPG_ANA_ST_SEARCH_VCD = 15,	/*< search vcd*/
	CFA_MPG_ANA_ST_SEARCH_VCDM2SUB = 16,	/*< search vcd mode2 subheader*/
	CFA_MPG_ANA_ST_CHK_RDI_VERIFY = 17,	/*< -VR RDI read DCI_CCI verify data*/
	CFA_MPG_ANA_ST_EXTRACT_NV_HLI = 18,	/*< DVD/+VR NV HLI data*/
	CFA_MPG_ANA_ST_MPGV = 19,
	CFA_MPG_ANA_ST_VID_HEADER = 20
} CfaMpgAnaSt;

typedef struct {
	u16 u2DecAstId;	/*< audio stream id*/
	u8 ucDecAstType;	/*< audio stream type*/
	u8 ucDecAstAtt;	/*< audio stream decoding attribute(now, only for LPCM)*/
	/* Dispatch DTS/PTS to its own ES, for our current AU update flow*/
	u64 u8AudPrsPts;	/*< audio parsing PTS*/
	u64 u8AudPrsDts;	/*< audio parsing DTS*/
	s64 i8AudLastPrsPts;	/*< audio last valid parsing PTS*/
	/*audio pts reset adjust*/
	bool fgPtsAdjust;	/*<if adjust audio pts*/
	s64 i8AudPtsAdjust;	/*< audio pts adjust*/
} CFAMPG_AUD_INF;

typedef struct {
	u8 ucDecSpstId;	/*< sp stream id*/
	u16 u2SpuSz;		/*< sp AU size.*/
	u16 u2SpuTotalSz;	/*< sp AU total size ,BDP117260 @pingzhao 2008/10/31*/
	/* Dispatch DTS/PTS to its own ES, for our current AU update flow*/
	u64 u8SpuPrsPts;	/*< SPU parsing PTS*/
	u64 u8SpuPrsDts;	/*< SPU parsing DTS*/
} CFAMPG_SP_INF;

typedef struct {
	u8 *pu1VidSpecData;
	u32 u4VidSpecDataLen;
	u64 u8VidSpecDataOfst;
} CfaMpgSpecInfo_T;

typedef enum {
	CFA_MPG_LOG_DEFAULT = (u32)1 << (u32)0,
	CFA_MPG_LOG_FFRW = (u32)1 << (u32)1,
} CfaMpgLogLvl_E;

/* MPG CFA instance*/
typedef struct CfaMpgInst {
#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
#endif
	/* instance information*/
	void *pvSptHdl;		/*< instance handle, debug only*/

	u8 *pucHdrBufRp;	/*< header buffer read pointer*/
	u32 u4RdyDataSz;	/*< ready data size in header buffer*/
	u8 *pucHdrBuf;	/*< header buffer to pre-read data for analyzing*/
	uintptr_t ptrPfrMemAddress;	/*< memory address got from Pfr, using sync DMA, 071220*/
	u32 u4AvailSz;	/*< available size in pbbuf*/
	u64 u8LastSyncPbbufCa;
	u32 u4LastAvailSz;
#if CFA_MPG_HIGH_BIT_RATE_HANDLE
	u8 *pucVideoTempBuf;
	u64 u8DataInBuf;
	u64 u8HiBiBuSize;
	u64 au8FileOffSet[50];
	u64 au8FileOffSetPts[50];
	u64 u8FileOffSet;
	u64 u8FileOffSetPts;
	u32 u4PtsArrayIndex;
	bool fgIsNoBeyondMaxTx;
	bool fgTxData2FIFO;	/*< flag indicates if transferring to header buffer*/

	bool fgHasAud;

	/*for parser cmd q*/
	CFA_VIDEO_INFO_T rCfaVidInf;
	u16 u2CmdQIndex;
	u64 u8PreTxEndOff;
	u64 u8FirstOffset;
	bool fgTxByPbbuf;
	u64 u8LatTxAvaLen;
#endif
	u64 u8TimeCode;
	s64 i8DeltaPts;
	bool fgIsSeek;
	bool fgCMDQTx;
	bool fgCMDQFirstFill;
	CfaMpgPrsBitStrmType eCMDQCurPrsPktType;
	bool fgSupportHibitRt;
	u32 u4BitRate;	/*< add for high birate*/

	CfaMpgAnaSt eDbgCurCfaMpgAnaSt;	/*< debug MPG CFA analyze state*/
	CfaMpgAnaSt eCurCfaMpgAnaSt;	/*< current MPG CFA analyze state*/
	u64 u8Ca;		/*< current analyzed address*/
	u16 u2DiscSectorSz;	/*< The disc read sector size*/
	u32 u4ParsedBytes;	/*< already parsed bytes after the latest tx done*/
	u16 u2StreamVideoType;
	CfaMpgMediumType eCfaMpgMediumType;	/*< medium type of MPG CFA*/
	CfaMpgSysStrmType eCfaMpgSysStrmType;	/*< current parsing system stream type*/
	CfaMpgPrsBitStrmType eCurPrsPktType;	/*< current parsing packet type*/
	u16 u2PrsAudStId;	/*< current parsing audio packet id*/
	u8 ucPrsSpStId;	/*< current parsing sp packet id*/
	u8 ucPrsVidStId;	/*< current parsing video packet id*/
	u64 u8CfaIssueTxLen;

	/*for pvr error handle*/
	bool fgCalPtsByScr;
	u64 u8SCR;		/*< SCR*/
	u64 u8PrsPts;	/*< parsing PTS*/
	u64 u8PrsPrevPts;	/*< parsing Previous PTS*/
	u64 u8PrsDts;	/*< parsing DTS*/
	u32 u4PktAddDataSz;	/*< MPEG1 packet header stuffing and optional data size*/
	u16 u2PrevPktLen;	/*< previous packet length*/
	u16 u2PktLen;	/*< packet length*/
	bool fgExistSCR;	/*< flag indicate if SCR exist*/
	bool fgExistDts;	/*< flag indicate if DTS exist*/
	bool fgExistPts;	/*< flag indicate if PTS exist*/
	bool fgFillAU;		/*< flag indicate if Au filled*/
	u32 u4PESHdrDataLen;	/*< PES_header_data_length, MPEG-1 is calculated by program,*/
	/*MPEG-2 is gotten from packet header*/
	u32 u4CurQueryInfType;	/*< current query information type*/
	u8 ucAudBound;	/*< audio bound in system header*/
	u8 ucVidBound;	/*< video bound in system header*/
	u16 u2SkipLen;	/*< skip length of increase hdrbuf and prspos in packet txing stage, for NV.*/
	/*Add by pingzhao for BDP00116658, add last pts inquery */
	u64 u8LastPts;	/*< Save the last valid pts in current range*/
	u64 u8LastPtsAddr;	/*< Save the last pts address*/

	bool fgExitTxDoneCtrl;	/*< flag indicates if exitting Tx done control*/
	bool fgTxData2HdrBuf;	/*< flag indicates if transferring to header buffer*/
	bool fgNoSupportAutoPause;	/*< flag indicates if support auto pause, only for VCD, */
	/*default: 0 is supported*/
	bool fgEverTx;		/*< flag indicates if CFA transferred data*/
	bool fgFindIFrame;	/*< flag indicates if  find I frame in current range*/
	bool fgIFrameEnd;
	/* CPS information*/
	u8 ucAPS;
	u8 ucEpn;
	u8 ucCGMS;
	u8 ucAnalogSrc;
	/*add for sony -vr CPRM info*/
	u16 u2DiscCpsType;
	u64 u8DTKC;
	/* Aspect ratio information*/
	u8 ucAspectRatio;

	/* HLI information*/
	u32 u4HLI_S_PTM;
	u32 u4HLI_E_PTM;

	/* information for CSS*/
	bool fgIsCSSDecOn;	/*< flag indicates if CSS needs to decrypt on*/
	u16 u2CSSJumpLen;	/*< The length that Cfa needs to jump for CSS and CPRM spec.*/

	/* NV information for seamless pts increase*/
	u32 u4VOBU_V_E_PTM;
	u32 u4VOBU_V_S_PTM;
	u32 u4VOB_V_E_PTM;
	u32 u4VOB_V_S_PTM;
	s64 i8StcOffset;
	u16 u2VOB_ID;

	/* -VR RDI DCI_CCI data to decrypt and CPS*/
	bool fgIsCPRMDecOn;	/*< flag indicates if CPRM needs to decrypt on*/
	u64 u8DCI_CCI;
	u64 u8DCI_CCI_Verify;

	/* information for playback*/
	u64 u8PckPos;	/*< pack position*/
	u64 u8PicPckPos;	/*< picture pack position*/
	u32 u4AstNs;		/*< total audio stream number <= CFA_MPG_MAX_STRM_NS*/
	CFAMPG_AUD_INF arAudInf[CFA_MPG_MAX_STRM_NS];	/*< audio info*/
	bool fgLpcmEmphasis;	/*< LPCM emphasis flag, just check 1st tx LPCM pack*/
	bool fgLastLpcmEmphasis;	/*< last LPCM emphasis flag*/

	u32 u4SpstNs;	/*< total sp stream number <= CFA_MPG_MAX_STRM_NS*/
	CFAMPG_SP_INF arSPInf[CFA_MPG_MAX_STRM_NS];	/*< subpic info*/
	u16 u2SpuSz;		/*< SPU size, max: 53220 bytes*/

	u32 u4LastSpstNs;	/*< last total sp stream number <= CFA_MPG_MAX_STRM_NS*/
	CFAMPG_SP_INF arLastSPInf[CFA_MPG_MAX_STRM_NS];	/*<last subpic info*/
	u32 u4SubPicUNum;

	u32 u4VideoFrameDuration;
	u32 u4VstNs;
	u8 aucDecVstId[CFA_MPG_MAX_STRM_NS];	/*< video stream id*/
	u64 au8VidLastPrsPts[CFA_MPG_MAX_STRM_NS];	/*< last video pts*/
	bool fgVidPtsAdjust[CFA_MPG_MAX_STRM_NS];	/*<if adjust audio pts	-- add for stc reset*/
	s64 i8VidPtsAdjust[CFA_MPG_MAX_STRM_NS];	/*< video pts adjust  -- add for stc reset*/
	/*add for store video type*/
	u8 aucDecVstType[CFA_MPG_MAX_STRM_NS];	/*< video type*/
	/*add by lqq for demux error handle*/
	bool fgDealDxEr;
	bool fgDemuxError;
	/*add by qq for One AU fill two or three times problem*/
	uintptr_t ptrLastVideoAUSAddr;	/*< last video AU start address*/
	u64 u8VideoPacketPts;	/*<real video pts without pts adjust*/
	/*add for mpeg first video pts too large problem*/
	s64 i8PtsResetAdValue;
	bool fgFindFirstVideoPts;
	/* information setting by application*/
	CfaMpgRange rCfaRange;	/*< file analyze range*/
	u32 u8DecSpStId;	/*< decoding sp stream uid*/
	u16 u2DecAudStId;	/*< decoding audio stream id ((MPEG stream id<<8) | MPEG sub-stream id))*/
	u8 ucDecSpStId;	/*< decoding sp stream id*/
	u8 ucDecVidStId;	/*< decoding vid stream id*/
	u32 u4CurPrsFlg;	/*< current parsing stream flag*/
	u32 u4QueryInfType;	/*< query information type*/

	HANDLE hMutex;	/*< Mutex for lock query information structure*/
	CfaMpgQIFirstMpgScInf rFirstMpgScInf;	/*< Save the first start code*/
	CfaMpgQIFirstMpgPtsInf rFirstMpgPtsInf;	/*< Save the first PTS*/
	CfaMpgQIFirstMpgPtsInf rLastMpgPtsInf;	/*< Save the last PTS*/
	CfaMpgQIStrmInf rStrmInf;	/*< Save the stream information*/
	CfaMpgQIAutoPauseInf rAutoPauseInf;	/*< Save the auto pause information*/
	CfaMpgQIMuxRateInf_T rMuxRateInf;	/*< Save the mux rate information*/
	CfaMpgSpecInfo_T rVidSpecInfo;
	/* CFA function pointer list*/
	/*CfaDrvIntf *pCfaDrvIntf;*/

	bool fgFillDummyAU;
	bool fgSetJumpRange;
#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
#endif
} CfaMpgInst;

/* MPG CFA instance pool*/
/*EXTERN CfaMpgInst _arCfaMpgInstPool[CFA_MPG_MAX_INST_NS];*/

/* MPG CFA interface*/
EXTERN const CfaIntf _rMpgCfaIntf;
#endif
/* CFA_MPG_H*/
