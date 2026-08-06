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




#ifndef _CFA_FLV_H
#define _CFA_FLV_H

#include "x_typedef.h"
#include <media/atc/dmx_cfa_flv.h>

#include "dmx_spt_cfa.h"
#include "cfa_if.h"
#include "cfa_cfg.h"

/*C Header File*/
#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------------
			Macros, Typedefs, Enumerations
-----------------------------------------------------------------------------*/
/* Maximum FLV CFA instance number */
#define CFA_FLV_MAX_INST_NS             (CONFIG_CFA_FLV_MAX_INST_NS)

#define CFA_FLV_CHECK_PIC_TYPE          1

#define CFA_FLV_READ_LEN                (1024 * 32)


#define CFA_FLV_AVSYNC_THRETHOLD        500	/*500ms*/

#define CONFIG_CFA_FLV_FOR_ERR_JUMP     1	/*add by mtk94053 for CNB00139016*/

#define CFA_FLV_ERR_JUMP_MAX_NUM        (30)	/*(1000)*/

#define CFA_FLV_SYNC_DATA_MAX_LEN       (FLV_READ_FOR_CODEC_SIZE * 200)

#define FLV_VID_AVC_DATA_HDR     (5)
#define FLV_TAG_PKG_TYPE_HDR     ((u8)0)
#define FLV_TAG_PKG_TYPE_FRAME   ((u8)1)

#define CFA_FLV_ERR_TIMESTAMP_AUD_DELTA_MS     (1 * 1000)

#define CFA_FLV_ERR_TIMESTAMP_VID_DELTA_MS     (3 * 1000)

#define CFA_FLV_DBG_VID_PTS         0
#define CFA_FLV_DBG_VID_CMD_Q_FLOW  0

#define CFA_FLV_DBG_AUD_PTS         0
#define CFA_FLV_DBG_AUD_CMD_Q_FLOW  0

	typedef enum {
		CFA_FLV_LOG_DEFAULT = 1U << 0U,
		CFA_FLV_LOG_FFRW = 1U << 1U,
		CFA_FLV_LOG_STATE = 1U << 2U,
		CFA_FLV_LOG_SRCH_HDR = 1U << 3U,
		CFA_FLV_LOG_DMA_V = 1U << 4U,
		CFA_FLV_LOG_DMA_A = 1U << 5U,
		CFA_FLV_LOG_CMDQ_V = 1U << 6U,
		CFA_FLV_LOG_CMDQ_A = 1U << 7U,
		CFA_FLV_LOG_FILLAU_V = 1U << 8U,
		CFA_FLV_LOG_FILLAU_A = 1U << 9U,
		CFA_FLV_LOG_PTS_V = 1U << 10U,
		CFA_FLV_LOG_PTS_A = 1U << 11U,
	} CfaFlvLogLvl_E;

	typedef enum {
		CFA_FLV_PRS_BIT_STRM_TYPE_NONE = (0x00),
		CFA_FLV_PRS_BIT_STRM_TYPE_VID = (0x01 << 0),
		CFA_FLV_PRS_BIT_STRM_TYPE_AUD = (0x01 << 1),
		CFA_FLV_PRS_BIT_STRM_TYPE_PACKET_HDR = (0x01 << 2),
	} CfaFlvPrsBitStrmType_E;

	typedef enum {
		CFA_FLV_TX_STRM_TYPE_NONE = 0x0,
		CFA_FLV_TX_STRM_TYPE_VID = 0x1,
		CFA_FLV_TX_STRM_TYPE_AUD = 0x2
	} CfaFlvTxStrmType_E;

	typedef struct {
		bool fgErrCorExist;
		u8 u1DataLen;
	} CfaFlvErrCorFlag_T;

	typedef struct {
		u64 u8FileSize;
		u64 u8HeaderSize;
		u32 u4DataPacketSize;
		u64 u8DataPacketCount;
		u64 u8PrerollTime;
	} CfaFlvFileInfo_T;

	typedef enum {
		FLV_FRM_TYPE_NONE,
		FLV_FRM_TYPE_I,
		FLV_FRM_TYPE_P,
		FLV_FRM_TYPE_B,
	} FLV_FRM_TYPE_E;

	typedef enum {
		CFA_FLV_ANA_ST_IDLE = 0x00,
		CFA_FLV_ANA_ST_SEARCH_HEADER,
		CFA_FLV_ANA_ST_SEARCH_TAG_HEADER = 0x02,
		/* if we tx audio hdr, we will set this state, in txdone, we will tx corresponding audio data*/
		CFA_FLV_ANA_ST_TX_AUD,
		/* if we tx video hdr, we will set this state, in txdone, we will tx corresponding video data*/
		CFA_FLV_ANA_ST_TX_VID,
		/* Tx Audio Total Dec Hdr*/
		CFA_FLV_ANA_ST_TX_A_HDR,
		CFA_FLV_ANA_ST_TX_PARTTAG_VIDEO,
		CFA_FLV_ANA_ST_TX_PARTTAG_V_HDR,
		CFA_FLV_ANA_ST_TX_PARTTAG_H265_HDR,
		CFA_FLV_ANA_ST_TX_START_CODE,
		CFA_FLV_ANA_ST_TX_AV_CMDQ
	} CfaFlvAnaSt_E;

	typedef struct {
		u64 u8LastTxSa;
		u64 u8LastTxLen;
	} CfaFlvLastTxInfo_T;

	typedef enum {
		CFA_FLV_TAG_TYPE_UNKNOWN,
		CFA_FLV_TAG_TYPE_DATA,
		CFA_FLV_TAG_TYPE_AUDIO,
		CFA_FLV_TAG_TYPE_VIDEO
	} CfaFlvTagType_E;

	typedef enum {
		CFA_FLV_SLICE_TYPE_NONE,
		CFA_FLV_SLICE_TYPE_NON_IDR,
		CFA_FLV_SLICE_TYPE_IDR,
		CFA_FLV_SLICE_TYPE_SEI,
		CFA_FLV_SLICE_TYPE_SPS,
		CFA_FLV_SLICE_TYPE_PPS
	} CfaFlvSliceType_E;

	typedef struct {
		CfaFlvTagType_E eTagType;	/*8 is Audio,9 is Video, 18 is Script data*/
		u32 u4DataSize;
		u32 u4TimeStamp;	/* ms*/
		u32 u4StreamID;	/*Always 0*/
		u64 u8Offset;
	} CfaFlvTagInfo_T;

	typedef enum {
		CFA_FLV_TIMESTAMP_OK,
		CFA_FLV_TIMESTAMP_ERROR,
		CFA_FLV_TIMESTAMP_BIG_CHG,
	} E_CFA_FLV_TIMESTAMP_STATE_T;

#define CFA_FLV_MAX_PTS_RECORD_LEN       5
#define CFA_FLV_MAX_DELTA_PTS_REC_LEN    5

#define CFA_FLV_MAX_60FPS_FRAME_DURATION (20)
#define CFA_FLV_60FPS_FRAME_DURATION (1000 / 60)
#define CFA_FLV_MAX_30FPS_FRAME_DURATION (35)
#define CFA_FLV_30FPS_FRAME_DURATION (1000 / 30)
#define CFA_FLV_MAX_25FPS_FRAME_DURATION (42)
#define CFA_FLV_25FPS_FRAME_DURATION (1000 / 25)
#define CFA_FLV_MAX_20FPS_FRAME_DURATION (52)
#define CFA_FLV_20FPS_FRAME_DURATION (1000 / 20)
#define CFA_FLV_15FPS_FRAME_DURATION (1000 / 15)
#define CFA_FLV_VID_AUD_TIME_MAX_DIFF (1000)

	typedef struct {
		bool fgNeedCheckTimeInc;
		bool fgNeedCheckTimeDec;
		bool fgNeedGetDeltaTime;

		u32 u4TimeIncCnt;	/* pts increase count*/
		u32 u4TimeDecCnt;	/* pts increase count*/

		u32 u4DeltaTime;
		u32 u4TimeIncrement;

		u32 u4LastTime;
		u32 u4CurTime;

		u64 u8TotalTagCnt;

		u32 u4OldNormnalTime;
		u32 u4FirstAbnormalTime;
		u32 au4TimeRecord[CFA_FLV_MAX_PTS_RECORD_LEN];
		u32 au4Rec4CorrectTime[CFA_FLV_MAX_PTS_RECORD_LEN];

		bool fgFirstGetTime;
		u32 u4BaseTime;

		u32 u4TimeStamp;

		u32 u4SeekTime;

		E_CFA_FLV_TIMESTAMP_STATE_T eState;
	} FLV_TIMESTAMP_MAN_T;

	typedef struct {
		CfaFlvTagInfo_T rATagInfo;
		CfaFlvCfgAudInfo_T rCfgInfo;
		u8 *pauAudHeader;
		CfaApiAudType eAudType;
		u32 u4TimeStamp;
	} CfaFlvAudInfo_T;

/* about video pts end*/
	typedef struct {
		CfaFlvTagInfo_T rVTagInfo;
		CfaFlvCfgVidInfo_T rCfgInfo;
		FLV_FRM_TYPE_E eFrmType;
		u32 u4TimeStamp;
	} CfaFlvVidInfo_T;

	typedef struct {
		bool fgUnitStart;
		u64 u8FileOffset;
		u32 u4Len;
		u64 u8Pts;
#if ENABLE_DMX_ADVANCED_VER
		u8 au1InsertHdrBuf[DMX_MAX_INST_BYTES_CNT];
		bool fgInsertHdr;
		u32 u4InsertHdrLen;
#endif
	} CfaFlvAudCmdQEntry_T;

	typedef struct {
		bool fgIsInDma;
		u32 u4EntryCnt;
		u32 u4RealTxLen;
		u64 u8TotalLen;
		CfaFlvAudCmdQEntry_T arEntrys[DMX_MAX_TX_CNT_FOR_CMD_Q];
	} CfaFlvAudCmdQInfo_T;

	typedef struct {
		bool fgUnitStart;
		u64 u8FileOffset;
		u32 u4Len;
		u64 u8Pts;
		u32 u4VType;
		bool fgEndAU;
		CfaApiPicTxMode eTxMode;
	} CfaFlvVidCmdQEntry_T;

	typedef struct {
		bool fgIsInDma;
		u32 u4EntryCnt;
		u32 u4RealTxLen;
		u64 u8TotalLen;
		CfaFlvVidCmdQEntry_T arEntrys[DMX_MAX_TX_CNT_FOR_CMD_Q];
	} CfaFlvVidCmdQInfo_T;

	typedef struct {
#ifdef MM_ATE_CHECK
		u32 u4MMATECHKStart;
#endif
		/* for cfa flv internal use */
		CfaFlvAnaSt_E eCurAnaSt;	/* current cfa flv analyze state */
		CfaFlvAnaSt_E eNextAnaSt;	/* Next cfa flv analyze state */
		u8 *pu1HdrBuf;
		u64 u8Ca;	/* < current analyzed address */
		u64 u8PreCa;	/* previous valid address */

		uintptr_t ptrPfrMemAddress;	/* Memory address got from Pfr, using Sync Pbbuf,2007/12/28 */
		u32 u4CurPrsFlag;
		u32 u4TxLen;
		CfaFlvTxStrmType_E eCurTxStrmType;

		u32 u4CurTagNum;
		u32 u4TagSize;
		bool fgTagIsValid;

		u32 u4PacketLen;	/*error handle , maybe each packet length  is not identical*/

		/*get payload pts */
		u64 u8PrsPts;

		/*for Time Search fo PB Engine */
		bool fgKeyFrame;
		bool fgFirstTxVid;
		bool fgFirstTxAud;

		/* set by playback engine */
		CfaFlvRange_T rRange;
		CfaApiVidType eVidCodecType;

		CfaFlvTagInfo_T rTagInfo;
		CfaFlvAudInfo_T rAudioInfo;
		CfaFlvVidInfo_T rVideoInfo;
		CfaFlvFileInfo_T rFileInfo;

		DecoderCfgInfo_T rDecoderCfgInfo;
		u8 *puDecoderCfgBuf;
		u32 u4DecoderCfgBufLen;
		bool fgNeedTxVidSeq;

		AacCfgInfo_T rAacCfgInfo;

		/* playback request these information */
		CfaFlvCurPosInfo_T rCurPosInfo;

		/*for analyze compressed payload data */
		bool fgNeedPrs;
		bool fgExistCompressData;

		u32 u4QueryInfType;	/*< query information type*/
		CfaFlvLastTxInfo_T rCfaFlvLastTxInfo;

		/*sequence info of codec specical data */
		bool fgPrsSeqFrameInterpolation;
		bool fgPrsPreProcRange;
		u32 u4PrsNumBFrames;

		/* For I/P/B frame check when filling video pts in AU info. */
		CfaPicType ePrePicType;
		u64 u8PreVPts;
		bool fgFirstVidAU;	/* If it is first video AU. If yes, previous PTS should be invalid*/
		bool fgDisplayOrder;	/* If it is display order or not*/

		/*Payload data first byte, check  I / B /P VOP tx mode */
		u8 u1FirstBytePay;

		u64 u8Endoffst;
		u64 u8FileSz;	/* current file size in bytes*/
		u32 u4FirstUintPay;
		u32 u4PayloadLenFieldSz;
		u32 u4VideoPartTagOffset;

#if CONFIG_CFA_FLV_FOR_ERR_JUMP
		u32 u4OffsetJumpCnt;
#endif

		u64 u8IFrameCnt;

		CfaFlvAudCmdQInfo_T *prAudCmdQsInfo;
		CfaFlvVidCmdQInfo_T *prVidCmdQsInfo;
		DMX_CMDQ_TX_ENTRY_T *prCmdEntrys;

		u32 u4PureAudCurUnitTxSz;	/* Current Pure Audio Tx Sz for one Audio AU*/
		u32 u4PureAudTxUnitSz;	/* One Audio AU's Max Size in FLV Pure Audio File's FF & RW*/
		bool fgJumpTurnOn;

		u32 u4PureAudSkipTagCnt;

		bool fgNoNeedSyncPb;
		bool fgRealSyncPb;
		u32 u4AvailDataSz;

		FLV_TIMESTAMP_MAN_T rVTimeStampMan;
		FLV_TIMESTAMP_MAN_T rATimeStampMan;
#ifdef MM_ATE_CHECK
		u32 u4MMATECHKEnd;
#endif
	} CfaFlvInst_T;

/*cfa flv interface need to implementation*/
	EXTERN CfaIntf _rFlvCfaIntf;

#ifdef __cplusplus
}
#endif
#endif				/* _CFA_FLV_H */
