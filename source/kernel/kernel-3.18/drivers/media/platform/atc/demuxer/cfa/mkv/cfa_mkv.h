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




#ifndef CFA_MKV_H
#define CFA_MKV_H

/* C header file */
#ifdef __cplusplus
extern "C" {
#endif

#include <media/atc/dmx_cfa_mkv.h>

#include "dmx_spt_cfa.h"
#include "cfa_macro.h"

/*System clock unit by KHz per second*/
#define CFA_MKV_SYS_CLK						(90)

/* Maximum MKV CFA instance number */
#define CFA_MKV_MAX_INST_NS					(CONFIG_CFA_MKV_MAX_INST_NS)

#define CFA_MKV_HDR_READ_BYTES				(u64)(5)

#define CFA_MKV_BLOCK_READ_BYTES			(7)

#define CFA_MKV_XIPH_LACING_SEGMENT			(255)

#define CFA_MKV_HDR_BUF						(u64)(64)

#define CFA_MKV_SPS_PPS_SIZE				(2)

#define CFA_MKV_DIVX3_P_FRM					(0x40)

#define CFA_MKV_MAX_FRAME_NUM				(128)

#define CFA_MKV_DECOMP_BUF_LEN				(0x5000)

#define CFA_MKV_SP_MAX_LAST_PTS				(5 * CFA_STC_CLK)

#define CFA_MKV_SP_MAX_LAST_BLOCK			(3)

#define CFA_MKV_MP4_SEQ_HDR_LEN				((u64)15)

#define CFA_MKV_FIRST_FRAME_READ_BYTES		((u64)20)

#define CFA_MKV_AVC_PPS_BUF_LEN				(138)

#define CFA_MKV_AVC_SPS_BUF_LEN				(138)

#define CFA_MKV_AVC_STARTCODE_LEN			(4)

#define CFA_MKV_WVC1_SPECDATA_LEN			(0X200)

#define CFA_MKV_WVC1_HEADER_LEN				(4)

#define CFA_MKV_AAC_HEADER_LEN				(9)

#define CFA_MKV_AVC_HEADER_LEN				(32)

#define CFA_MKV_AUD_AULEN_WITHOUT_VID		(20 * 1024)

#define OGG_HEAD_SIZE						(0X11A)
#define VORBIS_ID_LENGTH					(0X1E)

typedef enum {
	CFA_MKV_LOG_DEFAULT = (u32)1 << (u32)0,
	CFA_MKV_LOG_COMMON  = (u32)1 << (u32)1,
	CFA_MKV_LOG_AUINFO  = (u32)1 << (u32)2,
	CFA_MKV_LOG_VORBIS  = (u32)1 << (u32)3,
	CFA_MKV_LOG_TX_AUD_HEADER = (u32)1 << (u32)4,
	CFA_MKV_LOG_TX_VID_H264 = (u32)1 << (u32)5,
	CFA_MKV_LOG_STATE  = (u32)1 << (u32)6,
	CFA_MKV_LOG_TO_FIFO  = (u32)1 << (u32)7
} CfaMkvLogLvl_E;


typedef struct {
	bool   fgUnitStart;
	u64 u8FileOffset;
	u32 u4Len;
	u64 u8Pts;
} CfaMkvAudCmdQEntry_T;

typedef struct {
	bool   fgIsInDma;
	u32 u4EntryCnt;
	u32 u4RealTxLen;
	u64 u8TotalLen;
	CfaMkvAudCmdQEntry_T arEntrys[DMX_MAX_TX_CNT_FOR_CMD_Q];
} CfaMkvAudCmdQInfo_T;

typedef struct {
	bool   fgUnitStart;
	u64 u8FileOffset;
	u32 u4Len;
	u64 u8Pts;
	u32 u4VType;
} CfaMkvVidCmdQEntry_T;

typedef struct {
	bool   fgIsInDma;
	u32 u4EntryCnt;
	u32 u4RealTxLen;
	u64 u8TotalLen;
	CfaMkvVidCmdQEntry_T arEntrys[DMX_MAX_TX_CNT_FOR_CMD_Q];
} CfaMkvVidCmdQInfo_T;
typedef enum {
	MKV_ID_TIME_CODE = 0,
	MKV_ID_POSITION,	/* 1*/
	MKV_ID_PREV_SIZE,	/* 2*/
	MKV_ID_BLOCK_GROUP,	/* 3*/
	MKV_ID_SIMPLE_BLOCK,	/* 4*/
	MKV_ID_BLOCK,	/* 5*/
	MKV_ID_REFERENCEBLOCK,	/* 6*/
	MKV_ID_BLOCK_DURATION,	/* 7*/
	MKV_ID_CRC32,	/* 8*/
	MKV_ID_UNKNOWM_ID,	/* 9*/
	MKV_ID_EBML,	/* 10*/
	MKV_ID_SEGMENT,	/* 11*/
	MKV_ID_SEGMENT_INFO,	/* 12*/
	MKV_ID_SEEK_HEAD,	/* 13*/
	MKV_ID_CLUSTER,	/* 14*/
	MKV_ID_TRACK,	/* 15*/
	MKV_ID_CUES,	/* 16*/
	MKV_ID_ATTACHMENTS,	/* 17*/
	MKV_ID_CHAPTERS,	/* 18*/
	MKV_ID_TAGS,	/* 19*/
#if CONFIG_CFA_MKV_SUPPORT_DRM
	MKV_ID_DRMINFO,	/* 20*/
#endif

	/*new IDs should be added here! */

	MKV_ID_UNKNOW = 0xff
} CfaMkvIDs;

#if CONFIG_CFA_MKV_SUPPORT_DRM
typedef struct {
	/* file header info */
	bool fgTrackDataExist;

	/* chunk info */
	bool fgDrmExist;
	u16 u2KeyIdx;
	u32 u4EncryptOfst;
	u32 u4EncryptLen;
} CfaMkvDRMInf;
#endif

typedef enum {
	MKV_SEQUENCE_UNKOWN = 0,
	MKV_SEQUENCE_SPS,
	MKV_SEQUENCE_PPS
} CfaMkvSequenceType;

typedef enum {
	CFA_MKV_UNKOWN_LACING = (0x00),
	CFA_MKV_XIPH_LACING,
	CFA_MKV_EBML_LACING,
	CFA_MKV_FIX_LACING,
	CFA_MKV_NO_LACING
} CfaMkvLacingType;

/*CFA MKV parsing state */
typedef enum {
	CFA_MKV_ST_IDLE = (0x00),
	CFA_MKV_ST_TX_SC = (0x01),
	CFA_MKV_ST_SC_ANA = (0x02),
	CFA_MKV_ST_BLOCK_DURATION = (0x03),
	CFA_MKV_ST_TIMECODE = (0x04),
	CFA_MKV_ST_BLOCK = (0x05),
	CFA_MKV_ST_CLUSTER = (0x06),
	CFA_MKV_ST_BLOCKGROUP = (0x07),
	CFA_MKV_ST_SIZE_ANA = (0x08),
	CFA_MKV_ST_LACING_ANA = (0x09),
	CFA_MKV_ST_TX_BLOCK = (0x0a),
	CFA_MKV_ST_TX_SEQUENCE_INFO = (0x0b),
	CFA_MKV_ST_TX_H264H265 = (0x0c),
	CFA_MKV_ST_TX_SEQUENCE_RETURN = (0x0d),
	CFA_MKV_ST_WVC1_TXMODE_ANA = (0x0e),
	CFA_MKV_ST_WVC1_TX = (0x0f),
	CFA_MKV_ST_HEADER_STRIPING = (0x10),
	CFA_MKV_ST_AAC_TX_DONE = (0x11),
	CFA_MKV_ST_DECOMPRESSION = (0x12),
	CFA_MKV_ST_SEARCH_CLUSTER = (0x13),
	CFA_MKV_ST_READ_FIRST_FRAME = (0x14),
	CFA_MKV_ST_TX_H264H265_STARTCODE = (0x15),
	CFA_MKV_ST_TX_H264H265_STARTCODE_RET = (0x16)
#if CONFIG_CFA_MKV_SUPPORT_DRM
		, CFA_MKV_ST_PARSER_DRM_INFO = (0x17)
#endif
		, CFA_MKV_ST_TX_RV_PARCIAL = (0x18)
		, CFA_MKV_ST_TX_DIXV_HDR = (0x19)
		, CFA_MKV_ST_TX_DIXV_HDR_RET = (0x1a)
		, CFA_MKV_ST_TX_RV_BLOCK = (0x1b)
		, CFA_MKV_ST_MPEG4_VOL_HEADER = (0x1c)
		, CFA_MKV_ST_TX_AUD_AU_BYEND = (0X1d)
		, CFA_MKV_ST_TX_FINISH = (0x1e)
		, CFA_MKV_ST_TX_RV_BLOCKDATA = (0X1f)
		, CFA_MKV_ST_TX_FIRST_OGG_HDR = (0x20)
		, CFA_MKV_ST_TX_VORBIS_HDR = (0x21)
		, CFA_MKV_ST_TX_OGG_HDR = (0x22)
		, CFA_MKV_ST_TX_VORBIS_DATA = (0x23)
		, CFA_MKV_ST_TX_VORBIS_ID = (0x24)
		, CFA_MKV_ST_TX_VORBIS_PRIV_OGG_HDR = (0x25)
} ECfaMkvAnaState;


typedef enum {
	CFA_MKV_PIC_UNKNOWN,
	CFA_MKV_PIC_I,
	CFA_MKV_PIC_P,
	CFA_MKV_PIC_B
} ECfaMkvPicType;


typedef struct {
	bool fgHeaderStriping;
	u8 uHeaderLen;	/*header numbers */
	/* Yi Feng modified to fix BDP00123457 @2009/08/01 */
	/*u8 auHeader[32];*/
	u8 *auHeader;
} CfaMkvHeaderStriping;

typedef struct {
#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
#endif
	CfaApiVidType eVidCodec;
	u64 u8VidTrackNo;
	u64 u8VidTrackUID;
	bool fgTimeCodeScaleEn;	/*when timecodescale exist,set TRUE,else set FALSE */
	u64 u8TrackTimeCodeScale;
	u64 u8CodecPrivOfst;
	u64 u8CodecPrivLen;
	u64 u8CodecLen;
	u8 *pucCodecPrivBuf;
	u32 u4DivxHdrLen;

	AVC_SPS_PPS_INFO_T rSPSPPSInfo;

	u8 *pucDivxHdrBuf;
	bool fgFirstVC1;	/*useful flag when parsing */
	u32 u4NaluSize;
	CfaMkvHeaderStriping rCfaMkvHeader;	/*struct for header striping */
	CfaMkvContentEncoding_T rCfaMkvContentEncoding;

	bool fgWmvSeqRet;
	bool fgTxMpeg4VOLHeader;
	u8 pucMpeg4Header[MKV_MPEG4_HEADER_LEN];
	u32 u4Mpeg4HeaderLen;
#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
#endif
} CfaMkvVidStreamInfo;

typedef struct {
	CfaApiAudType eAudCodec;
	u64 u8AudTrackNo;	/*identification number of the track */
	u64 u8AudTrackUID;	/*a unique identificator of the track */
	bool fgTimeCodeScaleEn;	/*when timecodescale exist,set TRUE,else set FALSE */
	/*if this element is exited, block timecode should be multiplied by this value */
	u64 u8TrackTimeCodeScale;
	u64 u8CodecPrivOfst;	/*offset of CodecPrivate in file */
	u64 u8CodecPrivLen;	/*length of CodecPrivate element */
	CfaMkvHeaderStriping rCfaMkvHeader;	/*struct for header striping */
	u32 u4SampleRate;
	u32 u4Channles;
	u8 uAacHeaderSize;
	/* Yi Feng modify to fix BDP00121877 @2009/06/04 */
	u8 *auAacHeader;
	/*u8 auAacHeader[9];*/
	bool fgHaveTxHeader;

	u8 *pucAudCodecPrivData;
	u8 *pucAudCodecVorbisID;
	u8 u1VorbisCommHdrLenth;

	CfaMkvContentEncoding_T rCfaMkvContentEncoding;

	u64 u8VorbisIdOft;
	u64 u8VorbisIdSize;
	u64 u8VorbisCommHdrOft;
	u64 u8VorbisCommHdrSize;
	u64 u8VorbisHeadOft;
	u64 u8VorbisHeadSize;
} CfaMkvAudStreamInfo;

typedef struct {
	u64 u8SpTrackNo;	/*identification number of the track */
	u64 u8SpTrackUID;	/*a unique identificator of the track */
	bool fgTimeCodeScaleEn;	/*when timecodescale exist,set TRUE,else set FALSE */
	u64 u8TrackTimeCodeScale;/*if this element is exited, block timecode should be multiplied by this value */
	u64 u8CodecPrivOfst;	/*offset of CodecPrivate in file */
	u64 u8CodecPrivLen;	/*length of CodecPrivate element */
	bool fgCompresion;
	AVCODECID_T eCfaMkvSpType;
	CfaMkvContentEncoding_T rCfaMkvContentEncoding;
} CfaMkvSpStreamInfo;

typedef struct {
	/*if true,the information of cluster are available ,it also means this cluster are on parsing */
	bool fgClusterEn;

	bool fgPrsVid;
	bool fgPrsAud;
	bool fgPrsSub;
	/*first */
	bool fgFirstVid;
	bool fgFirstAud;
	bool fgFirstSub;
	u64 u8ClusterOfst;	/*the offset of  first byte of the cluster */
	u64 u8ClusterSize;	/*data size of this cluster */
	u64 u8TimeCode;

	bool fgClusterGetTimeode;
	u64 u8LastClusterTimecodePhyics;
	u64 u8LastClusterTimecode;

} CfaMkvClusterInfo;

typedef struct {
	u32 u4PayloadSize;
	u64 u8PayloadOffset;
} CfaMkvAvcInfo;

/*include blockgroup and simpleblock*/
typedef struct {
	bool fgGroupEn;
	/*if true,the information of blockgroup are available ,it also means this group are on parsing */
	u64 u8GroupOfst;
	u64 u8GroupSize;
} CfaMkvBlockGroupInfo;




/*store current block/simpleblock element information */
typedef struct {
#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
#endif
	bool fgReady;	/*if true:this block could be transfer */
	bool fgDurationReady;	/*if block duration are get for this subtitle block */
	bool fgDataReady;	/*if data is ready,it will be true,set for subtitle block */
	u64 u8TrackNum;
	u8 ucFrameCount;
	CfaMkvLacingType eLacingType;

	u32 u4BlockNo;	/*block number in cluster */
	u64 u8BlockOfst;	/*start offset of this block */
	u64 u8BlockSize;	/*size of block */

	u64 u8DataOfst;	/*start offset of block data */
	u64 u8DataSize;	/*size of data */
	CfaPrsBitStrmType eBlockType;
	u8 ucStrmIdx;
	u64 u8TimeCode;	/*absolute timecode */
	u64 u8TimeDuration;
	u64 u8Pts;	/*start pts of this block */
	u64 u8EndPts;	/*only for subtitle */

	u64 u8LastPts;	/*PTS of last FillAU */
	u32 u4AvcSize;

	u32 au4FrameSize[CFA_MKV_MAX_FRAME_NUM];
	s32 i4FrameNum;
	bool fgFrameEnd;

	u64 u8NextTxOfst;
	s32 i4CurFrameNum;
	bool fgFillPTS;

	s32 i4BFramCount;
	u64 u8LastBlockPTS;
	bool fgDiscardable;

	u64 u8VorbisPageNum;
	u32 u4VorbisPageIndex;
	u64 u8VorbisCurPageDataSize;
	u64 u8VorbisCurPageTotalAULen;

#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
#endif
} CfaMkvBlockInfo;

typedef struct {
	bool fgEnable;
	u64 u8ElementOfst;
	u64 u8Length;
} CfaMkvSkipElement;

/*added by Mingxu Wang @2011/3/5*/
typedef struct _CFA_MKV_SLICE_IFO {
	u8 u1TotalSliceNum;
	RM_VID_SLICE_ELEM_INF rSliceInf[RM_VID_SLICE_MAX_NUM];
} CFA_MKV_SLICE_INFO;

#define RM_MAX_SLICE_HEADER_LEN		((u32)(2048 + 1))
#define RM_SLICE_LEN_BYTES			((u32)8)
#define RM_SLICE_NUM_BYTE			((u32)1)


typedef struct {
#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
#endif
	/*CFA Mkv internal data */
	void  *pvSptHdl;
	uintptr_t ptrMemAddr;
	u32 u4AvailSize;
	u64 u8Ca;
	uintptr_t ptrLastReadMemAddr;
	bool   fgNoNeedSyncPb;
	bool   fgRealSyncPb;
	bool   fgCrossSlot;
	u64 u8AvalOfst;
	u64 u8TxLen;
	u64 u8LastCa;
	bool   fgIfNeedRebuf;

	ECfaMkvAnaState eCurState;
	CfaPrsBitStrmType eCurPrsStrm;
	u64 u8KeyTimeCode;	/*timecode of last keyframe */
	u64 u8KeyClusterOfst;	/*cluster start offset of last keyframe */
	u32 u4KeyBlockNo;	/*block number of last keyframe */

	u64 u8AudioTimeCode;
	u64 u8AudioClusterOfst;
	u32 u4AudioBlockNo;

	u8 auDecompBuf[CFA_MKV_DECOMP_BUF_LEN];
	size_t zBufSize;


	CfaMkvClusterInfo rCurCluster;
	CfaMkvBlockGroupInfo rCurGroup;
	CfaMkvBlockInfo rCurBlock;
	CfaMkvAvcInfo rCurAvc;

	u8 aucHdr[CFA_MKV_HDR_BUF];
	u64 u8HdrLen;

	CfaMkvAbnormalFlag_T rAbnormalFlags;

	ECfaMkvPicType eLastPicType;

	CfaMkvSkipElement rSkip;

	bool fgWVC1TxHeader;
	bool fgTxVC1SeqHdr;
	u8 *pu1Wvc1Header;
	u8 *pu1Mp4SeqHdr;

	bool fgStartAjustB;

	bool fgDemuxError;
	bool fgCheckRangeCluster;
	bool fgNotStartFromCluster;
	bool fgTxSeqHdrFromBuf;

#if 0
	bool fgFirstTxVid;	/*First TX video*/
#endif

	u64 u8TimeCodeScale;

	u64 u8LastVPts;
	u64 u8LastAPts;

	u64 u8SpLastPts;
	u32 u4SpBlockLastNs;


	u8 uSpsNs;
	u8 uPpsNs;
	u32 u4SeqHdrMemOfst;
	CfaMkvSequenceType eSequenceType;

	/*WMV789 flag */
	bool fgPrsInterp;
	bool fgPrsPre;
	u32 u4PrsBFrameNs;

	/*current parsing infomation */
	CfaMkvIDs eCurElement;
	u32 u4PrsFlg;
	u64 u8CurAId;
	u64 u8CurVId;
	u64 u8CurSpId;
	u8 ucCurAIndex;
	u8 ucCurSpIndex;

	u32 u4AudNum;	/*number of audio tracks */
	u32 u4SpNum;	/*number of sp tracks */
	CfaMkvVidStreamInfo rVidStmInfo;
	CfaMkvAudStreamInfo arAudStmInfo[MAX_NS_MKV_AUD];
	CfaMkvSpStreamInfo arSpStmInfo[MAX_NS_MKV_SP];
	bool fgSecondFillAU;

	u32 u4RWUnitAULen;
	bool fgHasVideo;
	bool fgTxFirst;
	bool fgFinishRWAU;

	s32 i4AudAULenWithoutVid;

	CfaMkvRange_T rRange;

#if CONFIG_CFA_MKV_SUPPORT_DRM

	CfaMkvAudCryptInfo rAudCryptInfo;

	/* Drm part */
	CfaMkvDRMInf rDRMInf;
	CFA_DIVXDRM_INFO_T rDivxDRMInf;	/* transfer to splitter as a parameter */
#endif

	/*added by Mingxu Wang, for rm support @2011/3/5*/
	u64 u8MemOffset;	/* the offset relative to u4MemAddr */
	u32 u4SliceNum;	/* the slice number */
	u32 u4SliceTotalNum;	/* the total slice num in block */
	u8 u1FirstBytePay;	/* the first byte of one frame */
	u64 u8SliceOffset;	/* the offset relative to the start addr of block data */
	u64 u8LastSliceDataOffset;	/* last slice data offset relative to the start addr of frame data */
	CfaApiPicTxMode eCurPicType;	/*get payload pts */
	u64 u8PrsPts;
	u32 u4CurPicTr;
	u32 u4First4BytesPay;
	u32 u4ForwardRefTr;
	u64 u8ForwardRefPts;
	u32 u4BackwardRefTr;
	u64 u8BackwardRefPts;
	u8 aucSliceHeader[RM_MAX_SLICE_HEADER_LEN];
	u32 u4SliceHeaderOffst;
	u32 u4SliceHeaderDataPartLen;
	CFA_MKV_SLICE_INFO rSliceInf;

	u8 *pucAvcHevcStartCode;
	u8 *pucAvcSPSBuf;
	u8 *pucAvcPPSBuf;
	bool fgNeedAddHeadStrip;

	u8 *pucWVC1SpecData;

	/*just for test */
	bool fgFillDummyAU;

	bool fgFindIdx;

	u32 u4VorbisHeadPageNum;
	u32 u4VorbisHeadPageIndex;
	u64 u8VorbisHeadCurPageDataSize;
	u64 u8VorbisHeadCurPageTotalAULen;
	u64 u8VorbisHeadDataSize;
	u64 u8VorbisHeadDataOfst;
	u32 u4VorbisAuNs;
	/*Add for TX vorbis head(id/common/setup hdr) for the resume play (FF->PLAY)*/
	u8 *pTotalVorbisHeadPage;
	u64 u8TotalVorbisHeadPageSize;
	u64 u8TotalVorbisHeadPageMemSize;

	u8 ucOggHdrVorbisID[300];
	u32 u4OggHdrVorbisIDLength;
	u8 ucOggHdrVorbisComSetup[300];
	u32 u4OggHdrVorbisComSetupLength;

	CfaApiPicTxMode eCfaMkvTxMode;
#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
#endif
	CfaMkvAudCmdQInfo_T *prAudCmdQsInfo;
	CfaMkvVidCmdQInfo_T *prVidCmdQsInfo;
} CfaMkvInst;




/* C header file */
#ifdef __cplusplus
}
#endif
#endif				/* _CFA_MKV_H_ */
