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


#ifndef DMX_CFA_OGM_H
#define DMX_CFA_OGM_H

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
#define MAX_NS_OGM_AUD								 10
/*#define MAX_NS_OGM_SP									 16*/

#define CFA_OGM_MAX_AAC_HEADER_LEN			9

typedef struct CfaOgmRange {
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKStart;
#endif	/* 
 */
	/* video */
	bool fgVidRangeEn;	/*only when true,the value below are valid */
	__s64 i8VidLastGranule;	/*granule position of last video page */
	__u64 u8VidStartOfst;	/*start offset of first page to be demux */
	__u8 uVidPacketStartNo;	/*loaction of first packet in the page */
	__u64 u8VidEndOfst;	/*end offset */

  /* audio */
	bool fgAudRangeEn;	/*only when true,the value below are valid */
	__s64 i8AudLastGranule;	/*granule position of last audio page */
	__u64 u8AudStartOfst;	/*start offset of first page to be demux */
	__u8 uAudPacketStartNo;	/*loaction of first packet in the page */
	__u64 u8AudEndOfst;	/*end offset */
	__u64 u8AudRangeOfst;	/*  audio range offset */
	__u64 u8SeekTime;

#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKEnd;
#endif	/* 
 */
} CfaOgmRange;

typedef struct CfaOgmKeyRange {
	CfaOgmRange rCfaOmgRane;
   /* for audio FR */
	__u32 u4FRAudAuLen;
} CfaOgmKeyRange;

typedef struct {
	__u64 u8TimeUnit;
	__u64 u8SamplePerUnit;
} CfaOgmAudTime;

typedef union {
	__u32 u4AudSampRate;	/* for vorbis audio */
	CfaOgmAudTime rCfaOgmAudTime;	/* for non-vorbis audio */
} CfaOgmAudSample;

typedef struct {
	__u8 uChannles;
	__u8 auAacHeader[CFA_OGM_MAX_AAC_HEADER_LEN];
	__u8 uAacHeaderLen;
} CfaOgmAacInfo;

#define VORBIS_HEADER_PACKET_NUM 3

typedef struct {
	AVCODECID_T eCfaAudCodec;
	__u32 u4AudStreamNo;
	CfaOgmAudSample rAudSample;
	CfaOgmAacInfo rCfaOgmAacInfo;

	/* for vorbis seek */
	__u32 u4VorbisSerialNo;
	__u8 u1HeaderPacketNum;
	__u8 *puHeaderData;
	__u32 u4HeaderDataSize;

	/* end */
	__u32 u4BitRate;
} CfaOgmAudInfo;

typedef struct {
	AVCODECID_T eCfaVidCodec;
	__u32 u4VidStreamNo;
	__u64 u8TimeUnit;
	__u64 u8FramePerUnit;
} CfaOgmVidInfo;

typedef struct CfaOgmConfigInfo {
	/* Video info */
	CfaOgmVidInfo rCfaOgmVidInfo;

	/* Audio info */
	__u8 uAudioStreamNs;	/*stream total numbers of      Audio Streams */
	CfaOgmAudInfo arCfaOgmAudInfo[MAX_NS_OGM_AUD];

	__u32 u4DurationMs;
} CfaOgmConfigInfo;

/* Old C header file */
#ifdef __cplusplus
}
#endif	/* 
 */

#endif	/* DMX_CFA_OGM_H */
