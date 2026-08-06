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


#ifndef DMX_CFA_MKV_H
#define DMX_CFA_MKV_H

#include "x_typedef.h"
#include "mm_debug.h"
#include "mm_common.h"
#include <linux/types.h>

/* Old C header file */
#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
										macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/
#define MAX_NS_MKV_AUD							10
#define MAX_NS_MKV_SP							32

#define MKV_INVALID_BLOCK_NO				(0xffffffff)

#define MKV_HEADER_STRPING_LENGTH	32
#define CONFIG_CFA_MKV_SUPPORT_DRM	1

#define MKV_AAC_HDR_BUF_LEN				5

#define MKV_MPEG4_HEADER_LEN				(0x20)

	typedef enum {
		CFA_MKV_ENCODING_COMP_NO = 0,
		CFA_MKV_ENCODING_COMP_ZLIB,
		CFA_MKV_ENCODING_COMP_BZLIB,
		CFA_MKV_ENCODING_COMP_LZO1X,
		CFA_MKV_ENCODING_COMP_UNKNOWN = 0xff
	} CfaMkvEncodingComp_E;

	typedef enum {
		CFA_MKV_ENCODING_ENCRYPT_NO = 0,
		CFA_MKV_ENCODING_ENCRYPT_DES,
		CFA_MKV_ENCODING_ENCRYPT_3DES,
		CFA_MKV_ENCODING_ENCRYPT_TWOFISH,
		CFA_MKV_ENCODING_ENCRYPT_BLOWFISH,
		CFA_MKV_ENCODING_ENCRYPT_AES,
		CFA_MKV_ENCODING_ENCRYPT_UNKNOWN = 0xff
	} CfaMkvEncodingEncrypt_E;

	typedef struct {
		CfaMkvEncodingComp_E eCfaMKvEncodingCompression;
		CfaMkvEncodingEncrypt_E eCfaMkvEncodingEncryption;

	} CfaMkvContentEncoding_T;

	typedef struct {
		bool fgHeaderStriping;
		__u8 uHeaderLen;	/*header numbers */
		__u8 auHeader[MKV_HEADER_STRPING_LENGTH];

	} CfaMkvHeaderStriping_T;

	typedef struct {
		__u8 uHeaderSize;
		__u8 auAacHeader[MKV_AAC_HDR_BUF_LEN];

	} CfaMkvAacHeader_T;

	typedef struct {
		__u32 u4SamplingFrequency;
		__u32 u4OutputSamplingFrequency;
		__u32 u4Channels;
		__u32 u4BitDepth;
		CfaMkvAacHeader_T rCfaMkvAacHeader;

	} CfaMkvAudSpecialInfo_T;


	typedef struct {
#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKStart;
#endif

		__u64 u8TargetTimeCode;	/*time code of the target position */
		__u64 u8ClusterStartAddr;

		/* video */
		__u64 u8VidTimeCode;	/*time code of the last keyframe position */

		__u32 u4VidBlockNo;
		__u64 u8VidStartOfst;	/*start offset of first cluster */
		__u64 u8VidEndOfst;	/*end offset */

		/* audio */
		__u32 u4AudBlockNo;
		__u64 u8AudStartOfst;	/*start offset of first cluster */
		__u64 u8AudEndOfst;	/*end offset */

		/* subtitle */
		__u32 u4SubBlockNo;	/*block number of the cluster */
		__u64 u8SubStartOfst;	/* start offset of first cluster */
		__u64 u8SubEndOfst;	/* ending offset */
#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKEnd;
#endif
	} CfaMkvRange_T;

	typedef struct {
		CfaMkvRange_T rCfaRangeInfo;
		__u32 u4RWUnitAULen;
	} CfaMkvKeyFrameRange_T;


	typedef struct {
		bool fgFourCCH264H265;	/*for the case of  FourCC--h264 */
		bool fgNoSeqHdr;

	} CfaMkvAbnormalFlag_T;

	typedef struct {
		AVCODECID_T eCFAAudCodec;
		__u64 u8AudTrackNo;	/*identification number of the track */
		__u64 u8AudTrackUID;	/*a unique identificator of the track */
		bool fgTimeCodeScaleEn;	/*when timecodescale exist,set TRUE,else set FALSE */
		/*if this element exist, block timecode should be multiplied by this value */
		__u64 u8TrackTimeCodeScale;
		__u64 u8CodecPrivOfst;	/*offset of CodecPrivate in file */
		__u64 u8CodecPrivLen;	/*length of CodecPrivate element */
		uintptr_t ptrVorbisPrivDataMem;
		__u8 u1VorbisCommHdrLen;
		CfaMkvHeaderStriping_T rCfaMkvHeaderStrip;
		CfaMkvAudSpecialInfo_T rCfaMkvAudSpecialInfo;
		CfaMkvContentEncoding_T rCfaMkvContentEncoding;

	} CfaMkvAudInfo_T;


	typedef struct {
		/*all these information get from TrackEntry */
		__u64 u8SpTrackNo;	/*identification number of the track */
		__u64 u8SpTrackUID;	/*a unique identificator of the track */
		bool fgTimeCodeScaleEn;	/*when timecodescale exist,set TRUE,else set FALSE */
		/*if this element exist, block timecode should be multiplied by this value */
		__u64 u8TrackTimeCodeScale;
		__u64 u8CodecPrivOfst;	/*offset of CodecPrivate in file */
		__u64 u8CodecPrivLen;	/*length of CodecPrivate element */
		AVCODECID_T eCfaMkvSpType;
		CfaMkvContentEncoding_T rCfaMkvContentEncoding;

	} CfaMkvSpInfo_T;

	typedef struct {
		__u64 u8KeyframeTimeCode;
		__u64 u8ClusterOffset;
		__u32 u4BlockNumber;
		__u64 u8CueTrackNum;
	} LP_MKV_INDEX_ITEM_T;

	typedef struct {
		__u32 u4PPSNum;
		__u32 u4SPSNum;
		__u64 u8SPSDataOffset;
		__u64 u8PPSDataOffset;
		__u32 u4SPSDataLen;
		__u32 u4PPSDataLen;
		bool fgHasSPSStartCode;
	} AVC_SPS_PPS_INFO_T;

	typedef struct {
		AVCODECID_T eVidCodec;
		VCODECVERSION_T eRVVersion;
		__u64 u8VidTrackNo;
		__u64 u8VidTrackUID;
		bool fgTimeCodeScaleEn;	/*when timecodescale exist,set TRUE,else set FALSE */
		/*if this element exist, block timecode should be multiplied by this value */
		__u64 u8TrackTimeCodeScale;
		__u64 u8CodecPrivOfst;	/*offset of CodecPrivate in file */
		__u64 u8CodecPrivLen;	/*length of CodecPrivate element */
		__u8 *pucCodecPrivBuf;
		__u32 u4DivxHdrLen;
		__u8 *pucDivxHdrBuf;
		__u32 u4NalSizeLen;
		CfaMkvHeaderStriping_T rCfaMkvHeaderStrip;
		CfaMkvContentEncoding_T rCfaMkvContentEncoding;
		AVC_SPS_PPS_INFO_T rSPSPPSInfo;

		__u32 u4IndexTableAddr;
		__u64 u8IndexTableIndex;

		__u8 pucMpeg4Header[MKV_MPEG4_HEADER_LEN];
		__u32 u4Mpeg4HeaderLen;

	} CfaMkvVidInfo_T;


#if CONFIG_CFA_MKV_SUPPORT_DRM
	typedef struct {
		__u8 u1CryptoOffset;	/* means audio crypto offset */
		__u8 u1CryptoSize;	/* means audio crypto size, if it's zero, no encryption */
	} CfaMkvAudCryptInfo;
#endif

	typedef struct {
		/*TimeCodeScale */
		__u64 u8TimeCodeScale;
		bool fgHasVideo;

		/* Video info */
		CfaMkvVidInfo_T rCfaMkvVidInfo;

		/* Audio info */
		__u32 u4AudioTrackNs;	/*track numbers of      Audio Track */
		CfaMkvAudInfo_T arCfaMkvAudInfo[MAX_NS_MKV_AUD];

		/* subtitle info */
		__u32 u4SubTrackNs;	/*track numbers of      subtitle Track */
		CfaMkvSpInfo_T arCfaMkvSpInfo[MAX_NS_MKV_SP];

		/*all of abnormal case flag are stored at this struct */
		CfaMkvAbnormalFlag_T rCfaMkvAbnormalFlags;

#if CONFIG_CFA_MKV_SUPPORT_DRM
		CfaMkvAudCryptInfo rAudCryptInfo;
#endif

	} CfaMkvConfigInfo_T;



/* Old C header file */
#ifdef __cplusplus
}
#endif
#endif				/* DMX_CFA_MKV_H */
