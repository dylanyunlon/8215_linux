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




#ifndef _CFA_OGM_H_
#define _CFA_OGM_H_

#include "x_typedef.h"
#ifdef __linux__
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_cfa_ogm.h>
#else
#include "dmx_define.h"
#include "dmx_cfa_ogm.h"
#endif /* __linux__*/

#include "dmx_def.h"
#include "dmx_spt_cfa.h"
#include "dmx_spt.h"
#include "cfa_cfg.h"
#include "cfa_if.h"

/* C header file */
#ifdef __cplusplus
extern "C" {
#endif


/* Maximum OGM CFA instance number */
#define CFA_OGM_MAX_INST_NS				(CONFIG_CFA_OGM_MAX_INST_NS)

#define CFA_OGM_HDR_READ_BYTES			(27)

#define CFA_OGM_PACKET_HDR_READ			(1)

#define CFA_OGM_HDR_TYPE				(5)

#define CFA_OGM_HDR_GRANUL_POSITION		(6)

#define CFA_OGM_HDR_STREAM_NO			(14)

#define CFA_OGM_HDR_PAGE_NO				(18)

#define CFA_OGM_HDR_SEGMENT				(26)

#define CFA_OGM_GRANULE_POSITION_LEN	(8)

#define CFA_OGM_STREAM_NO_LEN			(4)

#define CFA_OGM_PAGE_NO_LEN				(4)

#define CFA_OGM_SEGMENT_LEN				(1)

#define CFA_OGM_DIVX3_P_FRM				(0x40)

#define CFA_OGM_FIND_OGGS_BYTES			(0x100000)

#define CFA_OGM_OPTIM_PERM_LIMIT		(50)

#define CFA_OGM_CIRCLE_TIME_LIMIT		(10000)    /*for BDP00121640*/

#define CFA_OGM_INTERNAL_DEBUG			(0)

#define CFA_OGM_DBG_AUD_CMD_Q_FLOW      (0)

#define CFA_OGM_DBG_VID_CMD_Q_FLOW      (0)

#define CFA_OGM_TMP_DBG                 (0)

#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
#define CFA_OGM_AAC_PACKET_MAX_LEN		(1024 * 8 + 7)
#endif


/* OGM CFA parsing stream type */
typedef enum CfaOgmPrsStrmType {
	CFA_OGM_PRS_STRM_TYPE_NONE = (0x00),     /*< none */
	CFA_OGM_PRS_STRM_TYPE_V    = (0x01U<<0U),  /*< video */
	CFA_OGM_PRS_STRM_TYPE_A    = (0x01U<<1U),  /*< audio */
	CFA_OGM_PRS_STRM_TYPE_SP   = (0x01U<<2U),  /*< sp  */
} CfaOgmPrsStrmType;



/*CFA OGM parsing state */
typedef enum {
	CFA_OGM_ST_IDLE = (0x00),
	CFA_OGM_ST_PAGE_HDR_ANA,
	CFA_OGM_ST_PAGE_LACING_ANA,
	CFA_OGM_ST_PACKET_HDR_ANA,
	CFA_OGM_ST_DUMMY_DATA,
	CFA_OGM_ST_DIVX3_PACKET_ANA,
	CFA_OGM_ST_PACKET_TX,
	CFA_OGM_ST_LENBYTES_ANA,
	CFA_OGM_ST_FIND_OGGS,
#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
	CFA_OGM_ST_AAC_PACKET_ANA,
	/*CFA_OGM_ST_AAC_PAGE_HEADER_ANA*/
#endif
	CFA_OGM_ST_NEXT_STATE,
	CFA_OGM_ST_VORBIS_PAGE_TX,
	CFA_OGM_ST_AVCMDQ_TX,
	CFA_OGM_ST_PACKET_DATA_TX,
	CFA_OGM_ST_FINISH
} CfaOgmAnaState_E;


typedef enum {
	CFA_OGM_PIC_UNKNOWN,
	CFA_OGM_PIC_I,
	CFA_OGM_PIC_P,
	CFA_OGM_PIC_B
} CfaOgmPicType_E;

typedef struct {
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
	#endif
	u8 uVidPacketNo;
	u64 u8LastVidGranule;
	u64 u8StartOfst;
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
	#endif
} CfaOgmAU;




typedef struct {
	CfaApiAudType  eCfaAudCodec;
	u32 u4AudStreamNo;

	/* for vorbis audio*/
	u32 u4AudSampRate;

	/* for non-vorbis audio*/
	u64 u8TimeUnit;
	u64 u8SamplePerUnit;

	u64 u8DefaultSample;
	bool fgFirst;             /*if first page of this audio stream parsed*/

#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
	u8 uChannel;
	u8 auAacHeader[CFA_OGM_MAX_AAC_HEADER_LEN];
	u8 uAacHeaderLen;

	u8 *auPcmHeader;
	u32 uPcmHeaderLen;
	u64 u8FileOfst;
#endif

	u8 *pu1VorbisHeaderData;
	u32 u4VorbisHeaderSize;
	bool fgNeedTxVorbisHeaderData;
} CfaOgmAudStream;



typedef struct {
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
	#endif
	CfaApiVidType   eCfaVidCodec;
	u32 u4VidStreamNo;
	u64 u8TimeUnit;
	u64 u8FramePerUnit;

	u64 u8DefaultFrame;

	#if CONFIG_CFA_OGM_LENBYTES_ISNOT_ZERO
	u8  u1GotVItypeNo;
	#endif
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
	#endif
} CfaOgmVidStream;

typedef struct {
	bool   fgUnitStart;
	u64 u8FileOffset;
	u32 u4Len;
	u64 u8Pts;
} CfaOgmAudCmdQEntry_T;

typedef struct
{
    BOOL   fgUnitStart;
	BOOL   fgUnitEnd;
    UINT64 u8FileOffset;
    UINT32 u4Len;
    UINT64 u8Pts;
    UINT32 u4VType;
	CfaApiPicTxMode eTxMode;
} CfaOgmVidCmdQEntry_T;


typedef struct {
	bool   fgIsInDma;
	u32 u4EntryCnt;
	u32 u4RealTxLen;
	u64 u8TotalLen;
	CfaOgmAudCmdQEntry_T arEntrys[DMX_MAX_TX_CNT_FOR_CMD_Q];
	u8 uAudIdx;
} CfaOgmAudCmdQInfo_T;

typedef struct
{
    BOOL   fgIsInDma;
    UINT32 u4EntryCnt;
    UINT32 u4RealTxLen;
    UINT64 u8TotalLen;
    CfaOgmVidCmdQEntry_T arEntrys[DMX_MAX_TX_CNT_FOR_CMD_Q];
} CfaOgmVidCmdQInfo_T;

typedef struct {
	u64 u8StartOfst;
	u64 u8DataOfst;
	u64 u8DataLen;
	u32 u4StreamNo;
	u8 uAudIdx;
	u8 uPacketNs;                        /*the number of packets ending in this page*/
	u64 u8ThisAudGranulePosition;
	u64 u8LastAudGranulePosition;        /*granule position of last page*/
	u64 u8GranulPosition;
	bool fgFreshPacket;         /*if this page contain packet lasting from last page,it should be false*/
	bool fgEOS;                             /*if this page is the ending page of bitstream,it should be true */
	bool fgBOS;                             /*if this page is the begining page of bitstream,it should be true */
	bool fgParseAudio;
	bool fgParseVideo;
	u32 u4PageNo;                        /*sequence number of this page in the bitstream*/
	u8 uSegmentNs;                       /*the amount of segments */
	u8 *puSegmentTable;
	bool fgFirstAudPage;                    /*if this page is the first page of the range,it should be set true*/
	bool fgFirstVidPage;                    /*if this page is the first page of the range,it should be set true*/

	CfaOgmPrsStrmType eCfaOgmStrmType;
	/*store length of all packet of this page,inlcude the packet lasts from last page*/
	u32 au4PacketLen[DMX_INVALID_UINT8];
	/*for vorbis ,it means pts per packet,for other codec ,it means pts per granule*/
	u64 au8AudPtsPerGranule[MAX_NS_OGM_AUD];
	u64 u8VidPtsPerGranule;               /*pts per video frame*/
} CfaOgmPage;


typedef struct {
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
	#endif
	u8 uPacketHdr;                               /*one byte packet header*/
	u64 u8VidStartOfst;
	u64 u8VidLength;
	u64 u8AudStartOfst;
	u64 u8AudLength;
	u64 u8LastVidGranule;
	u64 au8LastAudGranule[MAX_NS_OGM_AUD];       /*granule position before this packet*/
	u64 u8CurVidGranule;                         /*granule values of this packet*/
	u64 au8CurAudGranule[MAX_NS_OGM_AUD];
	u64 u8VidStartPTS;
	u64 u8VidEndPTS;
	u64 u8AudStartPTS;
	u64 u8AudEndPTS;
	u8 uVidPacketNo;                             /*the packet number in this page*/
	u8 uAudPacketNo;
	u64 u8LastVidStartPTS;
	u32 u4VidPacketTotalNo;      /*the number of one whole logic video packet from file beginning*/
	bool   fgKeyFrame;/*mtk40301 fix bug 122405*/
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
	#endif
} CfaOgmPacket;

typedef enum {
	CFA_OGM_LOG_DEFAULT = 1U << 0U,
	CFA_OGM_LOG_FFRW = 1U << 1U,
	CFA_OGM_LOG_STATE = 1U << 2U,
	CFA_OGM_LOG_SRCH_HDR = 1U << 3U,
	CFA_OGM_LOG_DMA_V = 1U << 4U,
	CFA_OGM_LOG_DMA_A = 1U << 5U,
	CFA_OGM_LOG_CMDQ_V = 1U << 6U,
	CFA_OGM_LOG_CMDQ_A = 1U << 7U,
	CFA_OGM_LOG_FILLAU_V = 1U << 8U,
	CFA_OGM_LOG_FILLAU_A = 1U << 9U,
	CFA_OGM_LOG_PTS_V = 1U << 10U,
	CFA_OGM_LOG_PTS_A = 1U << 11U,
} CfaFlvLogLvl_E;


#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT

typedef struct {
	u64  uLastNoEndPacketLen;
	bool    fgTxAACStrmHdr;
	bool    fgOnlyTxAACHeader;
	bool    fgOnlyTxAACData;
	u8   *pu1AACPacketData;
	u64  u8CurAACPacketLen;
	/*u64  u8CurPacketSa;*/
	u64  u8AnaAACDataLen;
} CfaOgmAudAACPacket;

#endif

typedef struct {
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
	#endif
	/*CFA OGM internal data*/
	u32 u4InstHandle;
	uintptr_t ptrMemAddr;
	u64 u8Ca;

	u64 u8AvalOfst;
	uintptr_t ptrLastReadMemAddr;
	u64 u8LastReadOfst;
	u32 u4TxLen;

	CfaOgmAnaState_E  eCfaOgmCurState;
	CfaOgmAnaState_E  eCfaOgmNextState;
	CfaOgmPrsStrmType eCfaOgmCurPrsStrm;
	u8   *puOgmHdr;   /*point to the page header or packet header*/
	u64  u8HdrLen;

	/*information of last keyframe,for FillAU*/
	u64 u8VidKeyLastGranule;
	u64 u8VidKeyStartOffset;
	u8 uVidKeyPacketNs;

	CfaOgmAU rCfaOgmAu;

	u64 u8AudLastGranule;
	u64 u8AudStartOffset;
	u8 uAudPacketNs;

	bool fgIfNotifyPTS;

	bool    fgCrossSlot;/*Old Code, the case fgCmdq=False, only vorbis use cmdq */
	bool    fgNoNeedSyncPb;
	bool    fgRealSyncPb;
	bool fgIfRebuf;
	u32 u4AvalSize;

	u64 u8VidParsedOfst;
	u64 au8AudParsedOfst[MAX_NS_OGM_AUD];
	u64 u8RspCa;
	u64 u8RspTxLen;
	CfaOgmAnaState_E eRspState;
	u64 u8PageOfst;
	CfaOgmPage rRspPage;
	CfaOgmPacket rRspPacket;

	bool fgUseCMDQ;
    CfaOgmVidCmdQInfo_T rVidCmdQInfo;
    bool fgHasTxVidCmdQ;
    u64 u8PrePTSForComposeAU;
    CfaOgmAudCmdQInfo_T  arAudCmdQInfo[MAX_NS_OGM_AUD];
	CfaOgmAudCmdQInfo_T rAudCmdQInfo;
    u8 au8TxAudCmdQIndex[MAX_NS_OGM_AUD];
    u8 uTxAudCmdQNs;
    u8 uCurCmdQIdx;
    u8 uIdxForFillAU;
    CfaOgmAnaState_E eTmpStateWhenCmdQ;


	CfaApiPicTxMode eCfaOgmTxMode;

	u32 u4LastVidPacketNo;
	u64 u8AudLastRealPTS;   /*the related not INVALID_TIMESTAMP PTS*/

#if CONFIG_CFA_OGM_DIVX3_AU_SUPPORT
	bool fgPageContainEnd;
	bool fgUnitStart;
#endif

	s32 i4CallTimes;
	u32 u4CallFillAuCnt;

	/*current parsing infomation*/

	u32 u4PrsFlg;
	u32 u4CurAId;
	u32 u4CurVId;
	u32 u4CurSpId;
	u8  uCurAIndex;

	CfaOgmPicType_E ePrevPicType;
	bool fgAdjustPTS;

	u32 u4AudNum;       /*number of audio streams*/
	u32 u4SpNum;         /*number of sp streams*/
	CfaOgmVidStream  rCfaOgmVidStream;
	CfaOgmAudStream arCfaOgmAudStream[MAX_NS_OGM_AUD];

	CfaOgmPage rCurPage;
	CfaOgmPacket rCurPacket;

    CfaOgmPage arCmdQStartPage[MAX_NS_OGM_AUD];
    CfaOgmPacket arCmdQStartPacket[MAX_NS_OGM_AUD];
    CfaOgmPage rCmdQStartPage;
    CfaOgmPacket rCmdQStartPacket;
	CfaOgmPage rVidCmdQStartPage;
    CfaOgmPacket rVidCmdQStartPacket;
	DMX_CMDQ_TX_ENTRY_T  arOgmInstCmdEntrys[DMX_MAX_TX_CNT_FOR_CMD_Q];

	CfaOgmRange  rCfaOgmRange;


	bool fgFirstSetCfaRange;
	u64 u4FirstAudStartOfst;
	u64 u4FirstVidStartOfst;
    u32 u4DurationMs;
	u64 u8PreAudPts;
	
#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
	CfaOgmAudAACPacket rCfaOgmAACPacket;
#endif

	/*for RW*/
	u32 u4FRAudDataTxLen;
	u32 u4FRAudAuLen;
	bool fgRWFinish;
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
	#endif
} CfaOgmInst;


/*-----------------------------------------------------------------------------
					data declarations
-----------------------------------------------------------------------------*/
/* OGM OGM instance pool */
EXTERN CfaOgmInst _arCfaOgmInstPool[CONFIG_CFA_OGM_MAX_INST_NS];

/* OGM CFA interface */
EXTERN CfaIntf _rOgmCfaIntf;




/*-----------------------------------------------------------------------------
					function declarations
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * Name: vCfaOgmInitPara
 *
 * Description:
 *      Init CFA OGM internal parameters
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: None
 *
 *-----------------------------------------------------------------------------*/
void CfaOgmInitPara(CfaOgmInst *prCfaOgm);


/* C header file */
#ifdef __cplusplus
}
#endif

#endif  /* _CFA_OGM_H_ */



