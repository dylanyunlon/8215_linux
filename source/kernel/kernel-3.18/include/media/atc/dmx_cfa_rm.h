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




#ifndef DMX_CFA_RM_H
#define DMX_CFA_RM_H

#include "x_typedef.h"
#include "mm_debug.h"
#include "mm_common.h"

/* Old C header file */
#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------------
		    macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/
#define MAX_RM_AUD_STRM_NUM  1	/* RM is only one audio stream */
#define MAX_RM_PL_EXT_SYS_ID 6

#define CFA_RM_USE_VARIABLE_FRAME_RATE 1

#define CFA_RM_VIDEO_STREAM_ID          0	/* 0 in spec */
#define CFA_RM_AUDIO_STREAM_ID          1	/* 0 in spec */

/*CFA RM tag header size (1 + 3 + 3 + 1 + 3)*/
#define RM_TAG_HDR_SIZE (11)

/* 8 + 17 + 5 + 8 + 3 (+ 16 + 16) + 2   bits */
#define RM_EXTRA_DATA_SIZE  (10)

#define RM_READ_FOR_CODEC_SIZE  (RM_TAG_HDR_SIZE + RM_EXTRA_DATA_SIZE)

#define RM_READ_FOR_PACKET_HEADER_SIZE   (32)

/* CFA RM tag size(__u32) */
#define RM_TAG_SIZE_BYTES     (4)

/* static const __u32  RM_TBL_SZ =  200 * 1024; */

/* ------------------------------------ */
typedef enum {
	RM_INTLEV_ID_UNKNOWN,
	RM_INTLEV_ID_GENR,
	RM_INTLEV_ID_SIPR,
	RM_INTLEV_ID_VBRS,
	RM_INTLEV_ID_VBRF
} RM_INTERLEAVER_ID_E;

typedef enum {
	RM_STREAM_TYPE_UNKNOWN,
	RM_STREAM_TYPE_AUDIO,
	RM_STREAM_TYPE_VIDEO,
	RM_STREAM_TYPE_LOGIC,
	RM_STREAM_TYPE_SUBPI
} RM_STREAM_TYPE_E;

typedef struct {
	__u16 u2FlavorIndx;
	__u16 u2IntlevFactor;
	__u16 u2IntlevBlockSize;
	__u16 u2CodecFrameSize;
	__u32 u4BytesPperMin;
	__u32 u4SampleRate;
	__u16 u2SampleSize;
	__u16 u2NumChannels;
	RM_INTERLEAVER_ID_E eIntLevId;
	AVCODECID_T eAudioCodec;
	__u8 u1StreamType;
	__u16 u2SamplePerFrame;
	__u16 u2NumRegions;
	__u16 u2CplStart;
	__u16 u2CplQBits;
} RM_Audio_Info;

typedef struct {
	AVCODECID_T eVideoCodec;
	VCODECVERSION_T eCodecVer;
	__u16 u2FramWidth;
	__u16 u2FramHeight;
	__u16 u2BitCount;
	__u16 u2PadWidth;
	__u16 u2PadHeight;
	__u16 u2RprSize;
	__u32 u4FrameRate;
	__u32 u4SpoExtraFlags;
	__u32 au4RprSizeArry[16];	/* 2 * (8 + 1) */
	__u8 u1EccMask;
} RM_Video_Info;

typedef union {
	RM_Audio_Info rAInfo;
	RM_Video_Info rVInfo;
} RM_A_V_INFO_U;

typedef struct RM_MDPR_INFO {
	__u16 u2StreamNum;
	__u32 u4AvgBitrate;
	__u32 u4MaxPacketSize;
	__u32 u4AvgPacketSize;
	__u32 u4StartTime;
	__u32 u4Duration;
	RM_STREAM_TYPE_E eStreamType;
	RM_A_V_INFO_U uAVInfo;
	struct RM_MDPR_INFO *prNext;
} RM_MDPR_INFO_T;

/* ------------------------------------ */

typedef enum {
	CFA_RM_AUDIO_TYPE_UNKNOWN,
	CFA_RM_AUDIO_TYPE_STEREO,
	CFA_RM_AUDIO_TYPE_MONO
} CfaRmAudioType_E;

typedef struct {
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKStart;
#endif
	bool fgEnableVid;
	bool fgEnableAud;
	bool fgVidValid;	/* second range */
	bool fgAudValid;	/* second range */
	__u64 u8VidSa;
	__u64 u8VidEa;

	__u64 u8AudSa;
	__u64 u8AudEa;
	__u64 u8RealVidSa;

	__u64 u8targetTime;

	__u32 u4JumpUnitSz;
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKEnd;
#endif
} CfaRmRange_T;

typedef struct {
	__u64 u8IFrmCurOfst;
	__u64 u8AudCurOfst;
	__u64 u8VidCurOfst;
	__u64 u8PrsCurOfst;
} CfaRmCurPosInfo_T;

typedef struct {
	__u8 uObjectProfile;
	__u8 uSampFreqIdx;
	__u8 uChannels;
	__u8 uHeaderLen;
	__u8 *puHeader;
} RmAacCfgInfo_T;

typedef struct {
	__u16 u2StreamNum;
	RM_STREAM_TYPE_E eStreamType;
} RM_STREAM_TYPE_INFO_T;

typedef struct {
	__u64 u8FileSize;
	__u32 u4HeaderSize;
	__u32 u4StreamNum;
	bool fgHasVideo;
	RM_STREAM_TYPE_INFO_T *prStreamInfo;
	bool fgCfgRespliter;
} CfaRmCfgFileInfo_T;

typedef struct {
	__u8 u1StrmNum;	/* CFA_RM_AUDIO_STREAM_ID */
	__u8 u1NumChannels;
	AVCODECID_T eCodecID;
	CfaRmAudioType_E eSoundType;	/* the number of audio channel */
	__u32 u4SoundRate;	/* Samples per second ;u4SampleRate */
	__u16 u2SoundSize;	/* Size of each sample;u2SampleSize */
	__u16 u2FlavorIdx;
	__u16 u2InterleaveFactor;
	__u16 u2BlockSize;
	__u16 u2FrameSize;	/* u2CodecFrameSize */
	__u16 u2SamplePerFrame;
	__u16 u2NumRegions;
	__u16 u2CplStart;
	__u16 u2CplQBits;
	__u32 u4MaxPacketSize;
	__u32 u4AvgPacketSize;
	RmAacCfgInfo_T rAacInfo;
} CfaRmCfgAudInfo_T;


typedef struct {
	__u8 u1StrmNum;	/* CFA_RM_VIDEO_STREAM_ID */
	__u8 u1FrameRate;
	AVCODECID_T eCodecID;
	VCODECVERSION_T eCodecVer;
} CfaRmCfgVidInfo_T;

typedef struct {
	CfaRmCfgFileInfo_T rCfaRmCfgFileInfo;
	CfaRmCfgAudInfo_T rCfaRmCfgAudInfo;
	CfaRmCfgVidInfo_T rCfaRmCfgVidInfo;
} CfaRmCfgInfo_T;

typedef struct {
	__u8 u1StrmId;
	__u32 u4Pts;
} CfaRmGetPtsInfo;


#ifdef __cplusplus
}
#endif
#endif				/* DMX_CFA_RM_H */
