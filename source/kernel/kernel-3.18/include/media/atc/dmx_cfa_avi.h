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


#ifndef DMX_CFA_AVI_H
#define DMX_CFA_AVI_H

#include "x_typedef.h"
#include "chip_ver.h"
#include "dmx_define.h"
#include "mm_debug.h"
#include "mm_common.h"

/* Old C header file */
#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
										macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/
#define MAX_NS_AVI_AUD 10
#define MAX_NS_AVI_INTERNAL_SP 8
#define MAX_RIFF_CNT	 5

/* to control if transfer all SP chunk to fifo*/
/*  enable resplit */
#define CONFIG_CFA_AVI_TX_ALL_SP 1
/*	enable multiple audio range info */
#define CONFIG_CFA_AVI_MULTIPLE_AUDIO_INFO 1

/*	support detecting vbr garbage data */
#define CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA 1

/*  support new abr audio method */
#define CONFIG_CFA_AVI_SUPPORT_ABR_AUDIO 1

/*  support that CFA can notify LPE error */
#define CONFIG_CFA_AVI_SUPPORT_ERR_NOTI_LPE 1

/*  support correctting abnormal chunk */
#define CONFIG_CFA_AVI_SUPPORT_CORRECT_CHUNK_ABNORMITY	 0

#define CONFIG_CFA_AVI_NEW_METHOD_FOR_ABR_AUDIO	0

#define CONFIG_CFA_AVI_SUPPORT_MULTI_AUDIO_FOR_ABR	 0

/*	support H264 new au procession  */
#define CONFIG_CFA_AVI_SUPPORT_H26_NEW_METHOD	0

/*  divx3 drm new flow @2008/10/15 */
#define CONFIG_CFA_AVI_DIVX3_DRM_NEW_FLOW	1

#define CONFIG_CFA_AVI_NEW_METHOD_FOR_ERR_JUMP	 1

#define CONFIG_CFA_AVI_NEW_SYNC_PB_BUF_FLOW	1

#define AVI_SUPPOTR_DRM		DMX_SUPPORT_DIVXDRM

#define AVI_SUPPORT_SKIP_DATA_IN_TX_DATA	DMX_SUPPORT_DIVXDRM

#define AVI_DRM_DEBUG_LOG_ON										0

#define CONFIG_CFA_AVI_SUPPORT_AUDIO_DRM				1

#define CONFIG_CFA_AVI_SUPPORT_FIFO_LIMITATION	 1

#define CONFIG_CFA_AVI_PLAY_ONE_FRM_ONLY_TX_I_FRM	1

/* Tmp solution, for strict judgment */
#define CONFIG_CFA_AVI_USE_THRESHOLD_TMP	 1

/*  divide too large audio chunk @2008/.. */
#define CONFIG_CFA_AVI_DIVIDE_LARGE_AUD_CHUNK 1

/* Support AVIX File */
#define CONFIG_SUPPORT_AVIX_FILE						1

	typedef enum {
		CFA_AVI_AST_INVALID = 0x00,
		CFA_AVI_AST_VBRA = 0x1,	/* VBR audio/no-inter video */
		CFA_AVI_AST_INTV = 0x2,	/* normal video */
		CFA_AVI_AST_CBRA = 0x3,	/* CBR audio */
		CFA_AVI_AST_SUB = 0x4,	/* subpicture or subtitle */
		CFA_AVI_AST_ABRA = 0x5	/* ABR audio */
	} ECfaAviStrmType;

/* AVI CFA parsing bitstream type */
	typedef enum CfaAviPrsBitStrmType {
		CFA_AVI_PRS_BIT_STRM_TYPE_NONE = (0x00),	/*< none */
		CFA_AVI_PRS_BIT_STRM_TYPE_V = (0x01 << 0),	/*< video */
		CFA_AVI_PRS_BIT_STRM_TYPE_A = (0x01 << 1),	/*< audio */
		CFA_AVI_PRS_BIT_STRM_TYPE_SP0 = (0x01 << 2),	/*< sp 0 */
		CFA_AVI_PRS_BIT_STRM_TYPE_DRM = (0x01 << 3),
		CFA_AVI_PRS_BIT_STRM_TYPE_HDR = (0x01 << 4),	/*< 4cc for vid/aud/sp, eg: 01wb */
	} CfaAviPrsBitStrmType;

#if CONFIG_CFA_AVI_MULTIPLE_AUDIO_INFO
	typedef struct {
		bool fgIsValid;
		__u64 u8AudStartOfst;	/* file offset of u4AudStartChunkNo */
		__u32 u4AudStartChunkNo;	/* chunk number of the starting chunk */
		__u64 u8AudStartByte;	/* accumulated number of bytes of the starting chunk */
		__u32 u4AudSkipChunkNs;	/* number of chunks to be skipped by Demuxer */
		__u32 u4AudSkipByte;	/* number of bytes to be skipped by Demuxer */
		__u32 u4AudEndChunkNo;	/* ending audio chunk number */
		__u64 u8AudEndByte;	/* ending accumulated number of bytes */
	} CfaAviAudioRange;
#endif

	typedef enum {
		CFA_RANGE_TYPE_UNKNOWN,
		CFA_RANGE_TYPE_NORMAL,
		CFA_RANGE_TYPE_INQUERY,
		CFA_RANGE_TYPE_INC_EXTRA_DATA
#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
		    , CFA_RANGE_TYPE_INQUERY_VBR_GARBAGE_INF
#endif
	} ECfaRangeType;

/* CFA query information type, max 32 information */
	typedef enum {
		CFA_AVI_QUERY_TYPE_NONE = 0x00000000,

		CFA_AVI_QUERY_TYPE_PARSING_MODE = 0x00000001
#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
		    , CFA_AVI_QUERY_TYPE_VBR_GARBAGE_DATA = 0x00000002
#endif
	} ECfaQueryType;

	typedef struct {
		__u8 uParsingMode;
	} CfaAviParsingModeInfo;

#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
	typedef struct {
#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKStart;
#endif

		__u64 u8GarbageDataEndPtsByCbr;
		__u64 u8GarbageDataEndPtsByVbr;
		__u32 u4GarbageBytes;
		__u32 u4GarbageChunks;

#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKEnd;
#endif
	} TCfaAviVbrGarbageInf;
#endif

/* Must be compatible with MPC2AviPR */
	typedef struct {
#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKStart;
#endif
		ECfaRangeType eRangeType;

		bool fgReset;	/* set TRUE except in FUN_PTR_MPC2_GET_PR_CB pfvGetPR. */
		__u32 u4PrsFlag;	/*< current parsing stream flag */
		bool fgPlayFrmFlag;	/*To check if playing a Frame */

		__u64 u8IdxlOffset;	/* index start address */
		__u32 u4IdxlSz;	/* index size */


		/* video */
		__u64 u8VidStartOfst;	/* file offset of u4VidStartChunkNo */
		__u32 u4VidStartChunkNo;	/* chunk number of the starting chunk */
		__u32 u4VidSkipChunkNs;	/* number of chunks to be skipped by Demuxer */
		__u32 u4VidEndChunkNo;	/* ending video chunk number */

		/* audio */
#if CONFIG_CFA_AVI_MULTIPLE_AUDIO_INFO
		CfaAviAudioRange rAudioRange[MAX_NS_AVI_AUD];
#endif

		__u64 u8AudStartPts;

		__u64 u8StartPts;

		/* Yi Feng added to fix BDP00224825 @2009/04/29 */
		__u64 u8EndPts;
#ifdef MM_SUPPORT_DIVXHT31
		__u64 u8FFRangeEndPts;
#endif
		bool fgCompareSubPtsFlag;	/* The flag to compare subtitle pts with end pts */

		__u64 u8AudStartOfst;	/* file offset of u4AudStartChunkNo */
		__u32 u4AudStartChunkNo;	/* chunk number of the starting chunk */
		__u64 u8AudStartByte;	/* accumulated number of bytes of the starting chunk */
		__u32 u4AudSkipChunkNs;	/* number of chunks to be skipped by Demuxer */
		__u32 u4AudSkipByte;	/* number of bytes to be skipped by Demuxer */
		__u32 u4AudEndChunkNo;	/* ending audio chunk number */
		__u64 u8AudEndByte;	/* ending accumulated number of bytes */

		/* subpicture */
		__u64 u8SubStartOfst;	/* file offset of starting subtitle chunk */
		__u32 u4SubStartChunkNo;	/* chunk number of the starting chunk */
		__u32 u4SubSkipChunkNs;	/* number of chunks to be skipped by Demuuxer */
		__u32 u4SubEndChunkNo;	/* ending subpicture chunk number      */
		__u64 u8SubEndOfst;	/* ending offset of subtitle */

		/* end of stream */
		__u64 u8Endoffst;
		/*< current file size in bytes ,for Checking if a transfer is within IO read session range */
		__u64 u8FileSz;
		__u64 u8SeekPts;
#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKEnd;
#endif
	} CfaAviRange;

	typedef struct {
		/* Information of Current Key Frame */
		__u64 u8StartPts;	/* The Start Pts of the Key Frame */

		__u64 u8VidStartOfst;	/* file offset of u4VidStartChunkNo */
		__u32 u4VidStartChunkNo;	/* chunk number of the starting chunk */
		__u32 u4CurKeyFrmeMaxSZ;	/* The max size of current key Frame    */

		/* End Information of File, other than current key Frame */
		__u64 u8EndPts;	/* The Pts of File End, other than end of the Key Frame */
		__u64 u8Endoffst;
		__u64 u8FileSz;
		__u32 u4VidEndChunkNoInFile;	/* Ending video chunk number in the File */
		__u32 u4SpDataSize;

		CfaAviRange rCfaRangeInfo;
	} CfaAviKeyFrameRange;

/* CFA will give the current parsed position information of streams by a callback function. */
	typedef struct {
#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKStart;
#endif
		/* video */
		__u64 u8VidCurOfst;	/* file offset of u4VidCurChunkNo. */
		__u32 u4VidCurChunkNo;	/* chunk numbers parsed. */

		/* audio  need process multiple audio info --- mcn05027 @2008/04/28 */
		__u64 u8AudCurOfst;	/* file offset of u4AudCurChunkNo. */
		__u32 u4AudCurChunkNo;	/* used when VBR: chunk numbers parsed. */
		__u64 u8AudCurByte;	/* used when CBR: accumulated number of parsed bytes. */

		/* subpicture */
		__u64 u8SubCurOfst;	/* file offset of current parsed subtitle chunk. */
		__u32 u4SubCurChunkNo;	/* chunk numbers parsed. */
#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKEnd;
#endif
	} CfaAviCurPosiInfo;

	typedef struct {
		AVCODECID_T eAudCodec;	/* from 'strf' for audio */
		ECfaAviStrmType eAudStrmType;
		__u32 u4AudStrmIdx;	/* e.g.: 00dc: u4StrmIdx = 0, 01wb: u4StrmIdx = 1 */
		__u32 u4AudScale;	/* from "strh" */
		__u32 u4AudRate;	/* from "strh" */
		__u32 u4AudBps;	/* from "strf", byte per second; only used in CBRA */
		__u16 u2AudBlockAlign;	/* from "strf" */
#if 1				/* mtk40093 add for 227045 @2009/09/04 */
		__u16 u2AudBitsPerSample;
#endif
		__u32 u4SampleRate;	/* for vorbis audio */
		__u16 u2Channel;	/* for vorbis audio */
		__u16 u2AudCodecID;
		__u16 u2EncOptions;
		/*< memory address got from Pfr, using sync DMA, 071228 */
		__u8 *pucAudCodecSpecData;
		__u32 u4AudCodecSpecDataLen;
	} CfaAviAudInfo;


#if CONFIG_CFA_AVI_TX_ALL_SP
	typedef struct {
		ECfaAviStrmType eSpStrmType;
		/* e.g.: 00dc: u4StrmIdx = 0, 01wb: u4StrmIdx = 1,  02sb: u4StrmIdx = 2 */
		__u32 u4SpStrmIdx;
		__u32 u4SpScale;	/* from "strh" */
		__u32 u4SpRate;	/* from "strh" */
		__u32 u4SpBps;	/* from "strf", byte per second; only used in CBRA */
		__u32 u4SpChunkNo;
	} CfaAviSpInfo;
#endif

#if CONFIG_CFA_AVI_SUPPORT_AUDIO_DRM
	typedef struct {
		__u8 u1CryptoOffset;	/* means audio crypto offset */
		__u8 u1CryptoSize;	/* means audio crypto size, if it's zero, no encryption */
	} CfaAviAudCryptInfo;
#endif

#if CONFIG_CFA_AVI_SUPPORT_FIFO_LIMITATION
	typedef struct {
#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKStart;
#endif
		__u32 u4VidFifoSize;	/* means video fifo size */
		__u32 u4AudFifoSize;	/* means audio fifo size */
		__u32 u4SpFifoSize;	/* means sp fifo size */
#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKEnd;
#endif
	} CfaAviFifoInfo;
#endif

	typedef struct {
		__u32 u4GarbageDataChkNum;
		__u64 u8GarbageDataEndOffset;
		__u64 u8GarbageDataDuration;
	} CfaAviVbrMp3GarbageInfo;

	typedef struct rDrmInf {
		bool fgHasDrm;
		bool fgAdjustDDChunkRang;
		__u32 u4DrmVersion;
		__u32 u4DrmHdrSz;
		__u64 u8DrmOffset;
		__u8 *uDrmData;
	} PB_RIFF_DRM_INF_T;

#define AVC_SPS_MAX_CNT  (32)
#define AVC_PPS_MAX_CNT  (255)
	typedef struct {
		__u16 u2SeqParSetLen;
		__u8 *puSeqParSet;
	} TCfaAVISeqParSet_T;

	typedef struct {
		__u16 u2PicParSetLen;
		__u8 *puPicParSet;
	} TCfaAVIPicParSet_T;
	typedef struct {
		__u8 u1NumSeqParSet;
		TCfaAVISeqParSet_T rSeqParSet[AVC_SPS_MAX_CNT];
		__u8 u1NumPicParSet;
		TCfaAVIPicParSet_T rPicParSet[AVC_PPS_MAX_CNT];
		__u8 u1LenFieldSz;
	} TCfaAVCCfgInfo_T;

	typedef struct {
		/* V info */
		ECfaAviStrmType eVidStrmType;
		__u32 u4VidStrmIdx;	/* e.g.: 00dc: u4StrmIdx = 0, 01wb: u4StrmIdx = 1 */
		__u32 u4VidScale;	/* from "strh" */
		__u32 u4VidRate;	/* from "strh" */
		/* from "strf" eg. "DX50" => CFA_AVI_V_CODEC_DX5_M4V */
		AVCODECID_T eVidCodec;

		__u64 u8FirstVideoFrameOft;

		/* A info */
		__u32 u4MaxAudChunkDuration;
		CfaAviAudInfo rCfaAviAudInfo[MAX_NS_AVI_AUD];

		/* subtitle info */
#if CONFIG_CFA_AVI_TX_ALL_SP
		CfaAviSpInfo rCfaAviSpInfo[MAX_NS_AVI_INTERNAL_SP];
#else
		/* e.g.: 00dc: u4StrmIdx = 0, 01wb: u4StrmIdx = 1, 02sb: u4StrmIdx = 2 */
		__u32 u4SpStrmIdx;
#endif

		__u64 u8VidCodecSpecDataOfst;
		__u32 u4VidCodecSpecDataLen;
		void *pu1VidCodecSpecData;

		__u64 u8VidMPEG1CodecSpecDataOfst;
		__u32 u4VidMPEG1CodecSpecDataLen;
		void *pu1VidMPEG1CodecSpecData;

		__u64 u8VidMPEG4VolDataOffset;
		__u32 u4VidMPEG4VolDataLen;
		void *pu1VidMPEG4VolData;

		void   *pu1VidPPSSPSHeaderData;
		__u32 u4VidPPSSPSHeaderDataLen;
		__u8 u1AvcPayloadLenFieldSz;

#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
		TCfaAviVbrGarbageInf rVbrGarbageInf;
#endif

#if CONFIG_CFA_AVI_NEW_METHOD_FOR_ERR_JUMP
		bool fgIsFileHasIndex;
#endif

#if CONFIG_CFA_AVI_SUPPORT_AUDIO_DRM
		CfaAviAudCryptInfo rAudCryptInfo;
#endif

#if CONFIG_CFA_AVI_NEW_SYNC_PB_BUF_FLOW
		bool fgTooHighFrameRate;
#endif

#if CONFIG_CFA_AVI_SUPPORT_FIFO_LIMITATION
		CfaAviFifoInfo rFifoInfo;
#endif

		CfaAviVbrMp3GarbageInfo rVbrMp3GarbageInfo;

#if CONFIG_CFA_AVI_USE_THRESHOLD_TMP
		bool fgUseThreshold;
#endif

#if AVI_SUPPOTR_DRM
		PB_RIFF_DRM_INF_T rDrmInfo;
#endif
	} CfaAviConfigInfo;

	typedef struct {
		__u32 u4ChunkNo;	/* AVI chunk number of this picture */
	} CfaAviPicInf;

/* for FUN_PTR_MPC2_CFA_CB's u4CbEvt. */
#define AVI_CB_GET_PTS 0x1

/*! AVI_CB_GET_PTS
			pvPar = &CfaAviGetPtsInf;
*/
	typedef struct {
		/* [in] audio or subtitle stream id of multi-audio or multi-subtitle file, 0-based */
		__u32 u4StrmId;
		/* [in] chunk number */
		__u32 u4ChunkNo;
		/* [in] accumulated data amount in byte */
		__u64 u8AccuByte;
		/* [out] PTS in 90kHz.  A value of 0 means PTS is unavailable, not 0-valued PTS */
		__u32 u4Pts;
	} CfaAviGetPtsInf;


/* Old C header file */
#ifdef __cplusplus
}
#endif
#endif				/* DMX_CFA_AVI_H */
