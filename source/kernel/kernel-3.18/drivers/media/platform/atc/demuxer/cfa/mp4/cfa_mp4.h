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



#ifndef _CFA_MP4_H_
#define _CFA_MP4_H_

#include "x_typedef.h"
#include <media/atc/dmx_cfa_def.h>
#include <media/atc/dmx_cfa_mp4.h>
#include "dmx_spt_cfa.h"
#include "cfa_if.h"

#ifdef __cplusplus
extern "C" {

#endif

#define LAST_MEMORY 1
typedef enum CfaMp4CurTablePrsState { CFA_MP4_CUR_TABLE_PRS_TYPE_NONE = (0x00), CFA_MP4_CUR_TABLE_PRS_TYPE_V =
(0x01 << 0), CFA_MP4_CUR_TABLE_PRS_TYPE_A = (0x01 << 1), CFA_MP4_CUR_TABLE_PRS_TYPE_S = (0x01 << 2),
} CfaMp4CurTablePrsState;

/* MP4 CFA analyze state */
typedef enum CfaMp4AnaSt { CFA_MP4_ANA_ST_IDLE = (0x00), CFA_MP4_ANA_PRS_V_RANGE =
(0x01), CFA_MP4_ANA_PRS_A_RANGE = (0x02), CFA_MP4_ANA_PRS_S_RANGE =
(0x03), CFA_MP4_ANA_GET_V_PTS_TO_FIFO = (0x04), CFA_MP4_ANA_GET_A_PTS_TO_FIFO =
(0x05), CFA_MP4_ANA_GET_S_PTS_TO_FIFO = (0x06), CFA_MP4_ANA_PRS_NEXT_STATE =
(0x07), CFA_MP4_ANA_RELOAD_TABLE = (0x08), CFA_MP4_ANA_TX_AVC_TO_FIFO =
(0x09), CFA_MP4_ANA_TX_ENCRYPT_DATA_TO_FIFO = (0x0A),
CFA_MP4_ANA_TX_WMV_TO_FIFO = (0x0C), CFA_MP4_ANA_TX_VC1_TO_FIFO = (0x0D),
#if MP4_SUPPORT_FRAGMENT
CFA_MP4_ANA_PRS_MOOF_HEADER = (0x0E),CFA_MP4_ANA_PRS_MOOF_TRUN = (0x0F),
#endif
} CfaMp4AnaSt;
typedef enum CfaCurParserStream { CFA_MP4_UNKNOWN = (0x00), CFA_MP4_VIDEO =
(0x01), CFA_MP4_AUDIO = (0x02), CFA_MP4_SUBPIC = (0x03), CFA_MP4_ADTS =
(0x04), CFA_MP4_VIDEO_AVC
} CfaCurStreamType;
typedef enum { mp4, m4a, m4v
} EcfaFileType;

#define MOV_RM_VID_SLICE_MAX_NUM	(128)
typedef struct MOV_RM_VID_SLICE_ELEM_INF {
u8 u1SliceElemNum;
u16 u2SliceElemSize;
} MOV_RM_VID_SLICE_ELEM_INF;
typedef struct CFA_MP4_RM_SLICE_INFO {
u8 u1TotalSliceNum;
MOV_RM_VID_SLICE_ELEM_INF rSliceInf[MOV_RM_VID_SLICE_MAX_NUM];
} CFA_MP4_RM_SLICE_INFO;
typedef enum{ STTS, STSC, STSZ, STCO, STSD, MAX_MP4_SMP_TBL
	} MP4_SAMPLE_TABLE;
typedef struct{
u32 u4SampleSize;
u32 u4SampleDur;
} TSampleInfo;
typedef struct{
u32 u4SampleNums;
u32 u4LastSampleNo;
u32 u4SampleDecIndex;
u64 u8ChunkOfst;
} TChunkInfo;

enum Flags {
	kBaseDataOffsetPresent          = 0x01,
	kSampleDescriptionIndexPresent  = 0x02,
	kDefaultSampleDurationPresent   = 0x08,
	kDefaultSampleSizePresent       = 0x10,
	kDefaultSampleFlagsPresent      = 0x20,
	kDurationIsEmpty                = 0x10000,
};

typedef struct{
	u32 u4TrackID;
	u32 u4Flags;
	u32 u4SampleDescriptionIndex;
	u32 u4DefaultSampleDuration;
	u32 u4DefaultSampleSize;
	u32 u4DefaultSampleFlags;

	u64 u8BaseDataOffset;
	u64 u8DataOffset;
}CfaTfhdInfo ;

enum {
	kDataOffsetPresent                  = 0x01,
	kFirstSampleFlagsPresent            = 0x04,
	kSampleDurationPresent              = 0x100,
	kSampleSizePresent                  = 0x200,
	kSampleFlagsPresent                 = 0x400,
	kSampleCompositionTimeOffsetPresent = 0x800,
};

typedef struct{
	CfaTfhdInfo mTfhdInfo;
	void *pvTrunBuf;
	bool fgTrunBufValid;
	u32 u4TrunBufSize;
	u32 u4TrunBufMaxSize;
	u32 u4TrunBufOffset;
	u32 u4TrunFlags;
	u32 u4TrunSampleCount;
	u32 u4TrunSampleNo;
	u32 u4TrunSampleDuration;
	u32 u4TrunSampleSize;
	u32 u4TrunSampleFlags;
	u32 u4TrunSampleCtsOffset;
	u64 u8TrunDataOffset;
}MP4_MOOF_INFO;

typedef struct{

#ifdef MM_ATE_CHECK
		u32 u4MMATECHKStart;

#endif
		CfaApiVidType eVidType;
u64 u8CurPrsVidSampleNo;
u64 u8CurPrsVidChunkNo;
u32 u4NeedPrsSampleNums;
u64 u8CurVidTableStartSampleNo;
u64 u8CurVidTableEndSampleNo;
u64 u8CurVidTableStartChunkNo;
u32 u4TimeScale;
u64 u8Ofst;
u64 u8CurChunk1stSmpNo;
MP4_STBL_INFO *prVTable;
TSampleInfo *pTSampleInfo;
TChunkInfo *pTChunkInfo;
u8 *pucMpeg4CodecSC;
u8 *pucWVc1CodecSC;	/*only once .get from stsd box*/
u8 *pucSeqParamAdress;
u8 *pucPicParamAdress;
u8 *pucDecSpecInfo[MP4_STSD_TABLE_MAX_NUMS];
u8 u1VPSNum[MP4_STSD_TABLE_MAX_NUMS];
u8 *pucVPSInfo[MP4_STSD_TABLE_MAX_NUMS];
u8 u1SeqParamNum[MP4_STSD_TABLE_MAX_NUMS];
u8 u1SeqParamNumbkp[MP4_STSD_TABLE_MAX_NUMS];
u8 *pucSeqParamInfo[MP4_STSD_TABLE_MAX_NUMS];
u8 u1PicParamNum[MP4_STSD_TABLE_MAX_NUMS];
u8 u1PicParamNumbkp[MP4_STSD_TABLE_MAX_NUMS];
u8 *pucPicParamInfo[MP4_STSD_TABLE_MAX_NUMS];
u32 u4DecSpecSz[MP4_STSD_TABLE_MAX_NUMS];
u32 u4IPMPID;
u32 u4TrackID;	/*add by zhiwei chen for streamid*/
bool fgTxMp4SCDone;
bool fgGetRangeOfst;
bool fgCO64Valid;
u32 u4SampleSize;
u8 u1AvcSmpDesNums;
u16 u2Height;	/*add by guoqing yang for bug116914*/
u16 u2Width;	/*add by guoqing yang for bug116914*/
u32 u4FrameperSecond;	/*add by guoqing yang for bug116914*/
u32 u4NextSeqParamPos;
u32 u4NextPicParamPos;
u32 u4NextVPSPos;
u8 *puCodecSC;	/*for every au*/
bool fgCodecSCDone;	/*for every au*/
#ifdef MM_ATE_CHECK
		u32 u4MMATECHKEnd;

#endif
MP4_MOOF_INFO TCfaMoofInfo;
	} TCfaMp4VInf;
typedef struct{
	u32 u4AudBitRate;
	u32 u4AudSamplePerSec;
	u16 u2AudChannels;
	u64 u8CurPrsAudSampleNo;
	u64 u8CurPrsAudChunkNo;
	u32 u4NeedPrsSampleNums;
	u8 *pucADTSBuf;
	u8 *pucDecSpecInfo;
	u32 u4DecSpecSz;
	bool fgCO64Valid;
	bool fgAddAdtsDone;
	bool fgTxAacSCDone;
	bool fgSyncIV;
	u32 u4TimeScale;
	u32 u4IPMPID;
	u32 u4TrackID;	/*add by zhiwei chen for streamid*/
	AVCODECID_T eAudType;
	CfaApiAudType eCfaAudType;
	u64 u8CurAudTableStartSampleNo;
	u64 u8CurAudTableEndSampleNo;
	u64 u8CurAudTableStartChunkNo;
	MP4_STBL_INFO *prATable;
	TSampleInfo *pTSampleInfo;
	TChunkInfo *pTChunkInfo;
	bool fgGetRangeOfst;
	u32 u4SampleSize;
	MP4_MOOF_INFO TCfaMoofInfo;
} TCfaMp4AInf;
typedef struct{
	u64 u8CurPrsSubSampleNo;
	u64 u8CurPrsSubChunkNo;
	u32 u4NeedPrsSampleNums;
	u64 u8CurSubTableStartSampleNo;
	u64 u8CurSubTableEndSampleNo;
	u64 u8CurSubTableStartChunkNo;
	u32 u4TimeScale;
	u32 u4IPMPID;
	u32 u4TrackID;	/*add by zhiwei chen for streamid*/
	MP4_SUB_TYPE eSubType;
	MP4_STBL_INFO *prSTable;
	MP4_STBL_INFO *prMFSTable;
	MP4_STBL_INFO *prMVSTable;
	TSampleInfo *pTSampleInfo;
	TChunkInfo *pTChunkInfo;
	bool fgGetRangeOfst;
	bool fgCO64Valid;
	bool fgPG;
	u32 u4SampleSize;
} TCfaMp4SInf;

/* CFA will give the current parsed position information of streams by a callback function. */
typedef struct{

#ifdef MM_ATE_CHECK
		u32 u4MMATECHKStart;

#endif
			/* video */
		u64 u8VidCurOfst;

			/* audio */
		u64 u8AudCurOfst;

			/* subpicture */
		u64 u8SubCurOfst;

#ifdef MM_ATE_CHECK
		u32 u4MMATECHKEnd;

#endif
	} CfaMp4CurPos;
typedef struct{
	u32 u4TabelUb[4];
	u32 u4SttsInvaildSampleNums;
	u32 u4CurTableLastChunkNo;
	u64 u8CurSampleNo;
	u32 u4PrsDoneSampleNumsEveryChunk;
	u32 u4SampleBufNums;
	u32 u4ChunkBufNums;
} TTablePosInfo;
typedef struct{
	TTablePosInfo rTblVidPos;
	TTablePosInfo rTblAudPos[MAX_NS_MP4_AUD];
	TTablePosInfo rTblSubPos[MAX_NS_MP4_SUB];
} CfaMp4CurTablePos;

typedef enum {
	CFA_MP4_LOG_DEFAULT = 1 << 0,
	CFA_MP4_LOG_FFRW = 1 << 1,
} CfaMp4LogLvl_E;

typedef struct{

#ifdef MM_ATE_CHECK
		u32 u4MMATECHKStart;

#endif
bool fgFinished;
bool fgBadInterLeave;
bool *pfgTxBadIntDone;
bool fgGetATbl;
u32 u4EsdIndex;
u32 u4EsdNums;
u32 u4CurAvcEsdIndex;

/* CFA MP4 internal */
CfaMp4AnaSt eCurCfaMp4AnaSt;	/*< current MP4 CFA analyze state */
CfaMp4AnaSt eLastCfaMp4AnaSt;
CfaCurStreamType eCurStreamType;
CfaMp4PrsBitStrmType eCurPrsSampleType;	/*< original: _bStrmType */
#if MP4_SUPPORT_FRAGMENT
bool fgGetMoofData;
//u32 u4CfaCurMoofNums;
u32 u4CfaCurMoofSize;
u32 u4CfaCurMoofMdatSize;
u64 u8CfaCurMoofOffset;
u64 u8CfaNextMoofOffset;
MP4_FRAGMENT_TYPE eCfaMoofType;
#endif
u8 *pu1HdrBuf;
uintptr_t ptrPfrMemAddress;
u32 u4PrsFlg;
u32 u4CurPrsFlg;	/*< current parsing stream flag */
u32 u4CurTablePrsDoneFlag;
CfaPicType ePrePicType;
u64 u8VDuration;
u64 u8Vpts;
u64 u8Apts[MAX_NS_MP4_AUD];
u64 u8Spts[MAX_NS_MP4_SUB];
u64 u8CurVPts;
u64 u8CurVDuation;
u64 u8CurAPts[MAX_NS_MP4_AUD];
u64 u8CurADuation[MAX_NS_MP4_AUD];
u64 u8CurSPts[MAX_NS_MP4_SUB];
u64 u8CurSDuation[MAX_NS_MP4_SUB];
u64 u8Ca;
u64 u8Ea;

/* set by playback */
bool fgLpcmSet;
bool fgSyncBuf;
u8 u1SyncBufSize;
CfaMp4Range rCfaRange;	/* Emphases*/
CfaApiVidType eVidType;
EcfaFileType eFileType;
bool fgTxAvcHdr;
u8 *pucAvcHdr;
TCfaMp4VInf rCfaMp4VInf;	/*Emphases*/
TCfaMp4AInf rCfaMp4AInf[MAX_NS_MP4_AUD];	/* Emphases*/
TCfaMp4SInf rCfaMp4SInf[MAX_NS_MP4_SUB];	/*Emphases*/
CFA_VIDEO_INFO_T rVidInf;	/*Emphases*/
CFA_AUDIO_INFO_T rAudInf;	/*Emphases*/
CFA_SUBPIC_INFO_T rSubInf;	/*Emphases*/
CFA_CPS_INFO_T rMarlinCPSInf;
u16 u2SubStrmNums;
u16 u2AudStrmNums;
u32 u4CurVidInfoId;
u32 u4CurAudInfoId;	/*< current audio info idx related to rCfaAviAInf[]. */
u32 u4CurAudPlayId;
u32 u4CurSubInfoId;

/* playback will request this information by callback */
CfaMp4CurPos rCurOfst;
CfaMp4CurTablePos rCurTblPos;
u32 u4TxTableTimes;
Mp4BadIntlvdCheck rAvInterleaveChkParm;
bool fgGetDate;
CFA_MP4_RM_SLICE_INFO rSliceInf;

/*WMV789 flag */
bool fgPrsSeqFrameInterpolation;
bool fgPrsPreProcRange;
u32 u4PrsNumBFrames;
bool fgHasSkipData;

bool fgSh263GetedHdr;
bool fgGetSPLen;
#ifdef MM_ATE_CHECK
		 u32 u4MMATECHKEnd;

#endif
	} CfaMp4Inst;

#ifdef __cplusplus
}
#endif

#endif				/* _CFA_MP4_H_*/
