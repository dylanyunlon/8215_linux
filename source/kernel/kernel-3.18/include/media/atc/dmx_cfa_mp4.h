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


#ifndef DMX_CFA_MP4_H
#define DMX_CFA_MP4_H

#include "x_typedef.h"
#include "dmx_define.h"
#include "mm_debug.h"
#include "mm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SPECIAL_LPCM_SUPPORT 1
#define LPE_SUPPORT_COMPRESSED_DATA 1

#define MIN_AUD_SMP_SZ 0x200

#define MAX_NS_MP4_VID									1
#define MAX_NS_MP4_AUD									8
#define MAX_NS_MP4_SUB									8

#define ENTRY_SIZE_STTS								8
#define ENTRY_SIZE_STSC								12
#define ENTRY_SIZE_STSZ								4
#define ENTRY_SIZE_STCO								4
#define ENTRY_SIZE_CO64								8
#define ENTRY_SIZE_STSS								4
#define ENTRY_SIZE_STSD								0


#define MP4_STSD_TABLE_MAX_NUMS 8

#define MP4_SUPPORT_FRAGMENT 1
/*! @} */


/* Must be compatible with MPC2Mp4PR */
	typedef struct {
#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKStart;
#endif
		__u64 u8Length;
		bool fgFirstRange;
		/* video */
		__u64 u8VidStartChunkNo;
		__u64 u8VidStartChunk1stSmp;
		__u64 u8VidStartSampleNo;
		__u64 u8VidStartOffset;
		__u64 u8VidEndOffset;

		__u64 u8VidEndChunkNo;
		__u64 u8VidEndSampleNo;
		__u64 u8VidPts;

		/* audio */
		__u64 u8AudStartChunkNo[MAX_NS_MP4_AUD];
		__u64 u8AudStartChunk1stSmp[MAX_NS_MP4_AUD];
		__u64 u8AudStartSampleNo[MAX_NS_MP4_AUD];
		__u64 u8AudStartOffset[MAX_NS_MP4_AUD];
		__u64 u8AudEndOffset[MAX_NS_MP4_AUD];
		__u64 u8AudEndChunkNo[MAX_NS_MP4_AUD];
		__u64 u8AudEndSampleNo[MAX_NS_MP4_AUD];
		__u64 u8AudPts[MAX_NS_MP4_AUD];

		/* subpicture */
		__u64 u8SubStartChunkNo[MAX_NS_MP4_SUB];
		__u64 u8SubStartChunk1stSmp[MAX_NS_MP4_SUB];
		__u64 u8SubStartSampleNo[MAX_NS_MP4_SUB];
		__u64 u8SubStartOffset[MAX_NS_MP4_SUB];
		__u64 u8SubEndOffset[MAX_NS_MP4_SUB];
		__u64 u8SubEndChunkNo[MAX_NS_MP4_SUB];
		__u64 u8SubEndSampleNo[MAX_NS_MP4_SUB];
		__u64 u8SubPts[MAX_NS_MP4_SUB];

		__u32 u4PrsFlag;
		__u64 u8StartPts;
		__u64 u8SeekPts;	/* add by zhiwei chen */

		__u64 u8MoovSkipSize;	/* for some file into at begin of moov box ,need skip it */
#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKEnd;
#endif
	} CfaMp4Range;

	typedef struct {
		CfaMp4Range rCfaRangeInfo;
	} CfaMP4KeyFrameRange;


/* /////////////////////////////////////////////////////////////////// */
/* //////Follow define is for CFA MP4 Config info, and this ////////// */
/* //////should be set before file start play///////////////////////// */
/* /////////////////////////////////////////////////////////////////// */
	typedef struct {
		bool fgValid;
		bool fgIsCo64;
		__u32 u4EntryNs;
		void *pvEntry;
		__u64 u8FileOffset;	/* point to "number of entries" field */
		__u32 u4Allotted;
		__u32 u4EntryLb;
		__u32 u4EntryUb;
		__u32 u4LastSmp;
		bool fgIndexErr;
	} MP4_STBL_INFO;

	typedef struct {
		__u16 u2AudStrmNum;
		__u16 u2Height;	/* add by guoqing yang for bug116914 */
		__u16 u2Width;	/* add by guoqing yang for bug116914 */
		__u32 u4FrameperSecond;	/* add by guoqing yang for bug116914 */
		bool fgVCO64Valid;
		bool fgACO64Valid[MAX_NS_MP4_AUD];
		__u32 u4VTimeScale;
		__u32 u4ATimeScale[MAX_NS_MP4_AUD];
		__u32 u4VSampSz;
		__u32 u4ASampSz[MAX_NS_MP4_AUD];
		MP4_STBL_INFO *prVTable;
		MP4_STBL_INFO *prATable[MAX_NS_MP4_AUD];
	} Mp4BadIntlvdCheck;

	typedef struct {
		AVCODECID_T eVidType;
		bool fgCO64Valid;
		__u32 u4IPMPID;
		__u32 u4TrackID;	/* add by zhiwei chen mtk40495        for streamid    2011.3.17 */
		__u32 u4TimeScale;
		__u64 u8TimeDuration;
		__u8 *pucDecSpecInfo;
		__u32 u4DecSpecSz;

		__u32 u4PicSize[MP4_STSD_TABLE_MAX_NUMS];
		__u32 u4SeqSize[MP4_STSD_TABLE_MAX_NUMS];
		__u32 u4VPSSize[MP4_STSD_TABLE_MAX_NUMS];
		__u8 u1PayLoadLength[MP4_STSD_TABLE_MAX_NUMS];
		__u8 u1VPSNum[MP4_STSD_TABLE_MAX_NUMS];
		__u8 *pucVPSInfo[MP4_STSD_TABLE_MAX_NUMS];
		__u8 u1SeqParamNum[MP4_STSD_TABLE_MAX_NUMS];
		__u8 u1SeqParamNumbkp[MP4_STSD_TABLE_MAX_NUMS];
		__u8 *pucSeqParamInfo[MP4_STSD_TABLE_MAX_NUMS];
		__u8 u1PicParamNum[MP4_STSD_TABLE_MAX_NUMS];
		__u8 u1PicParamNumbkp[MP4_STSD_TABLE_MAX_NUMS];
		__u8 *pucPicParamInfo[MP4_STSD_TABLE_MAX_NUMS];

		__u32 u4SampSz;
		__u8 u1AvcSmpDesNums;
		MP4_STBL_INFO *prVTable;
	} CfaMp4VidInfo;

	typedef struct {
		__u16 u2AudStrmNum;
		AVCODECID_T eAudType[MAX_NS_MP4_AUD];
		__u32 u4IPMPID[MAX_NS_MP4_AUD];
		__u32 u4TrackID[MAX_NS_MP4_AUD];	/*  streamid */
		bool fgCO64Valid[MAX_NS_MP4_AUD];
		__u16 u2AudChannels[MAX_NS_MP4_AUD];
		__u8 *pucDecSpecInfo[MAX_NS_MP4_AUD];
		__u32 u4DecSpecSz[MAX_NS_MP4_AUD];
		__u32 u4AudBitRate[MAX_NS_MP4_AUD];
		__u32 u4AudSamplePerSec[MAX_NS_MP4_AUD];
		__u32 u4TimeScale[MAX_NS_MP4_AUD];
		__u64 u8TimeDuration[MAX_NS_MP4_AUD];
		__u32 u4SampSz[MAX_NS_MP4_AUD];
		MP4_STBL_INFO *prATable[MAX_NS_MP4_AUD];
	} CfaMp4AudInfo;

	typedef enum {
		MP4_SUB_NONE,
		MP4_SUB_SUBP,
		MP4_SUB_SUBT,
		MP4_SUB_TEXT
	} MP4_SUB_TYPE;

	typedef struct {
		__u16 u2SubStrmNum;
		bool fgPG[MAX_NS_MP4_SUB];
		__u32 u4IPMPID[MAX_NS_MP4_SUB];
		__u32 u4TrackID[MAX_NS_MP4_SUB];	/* streamid  */
		bool fgCO64Valid[MAX_NS_MP4_SUB];
		__u32 u4TimeScale[MAX_NS_MP4_SUB];
		__u64 u8TimeDuration[MAX_NS_MP4_SUB];
		__u32 u4SampSz[MAX_NS_MP4_SUB];
		MP4_SUB_TYPE eSubType[MAX_NS_MP4_SUB];
		MP4_STBL_INFO *prSTable[MAX_NS_MP4_SUB];
		MP4_STBL_INFO *prMFSTable[MAX_NS_MP4_SUB];
	} CfaMp4SubInfo;

#if MP4_SUPPORT_FRAGMENT
	typedef enum {
		TYPE_NONE,
		TYPE_ONLY_MOOV,
		TYPE_ONLY_MOOF,
		TYPE_MOOV_AND_MOOF
	}MP4_FRAGMENT_TYPE;
#endif

	typedef struct {
		/* V info */
		CfaMp4VidInfo rCfaMp4VidInfo;

		/* A info */
		CfaMp4AudInfo rCfaMp4AudInfo;

		/* subtitle info */
		CfaMp4SubInfo rCfaMp4SubInfo;
#if MP4_SUPPORT_FRAGMENT
		MP4_FRAGMENT_TYPE eMoofType;
		__u64 u8MoofOffset;
#endif
	} CfaMp4ConfigInfo;

/* MP4 CFA parsing bitstream type */
	typedef enum CfaMp4PrsBitStrmType {
		CFA_MP4_PRS_BIT_STRM_TYPE_NONE = (0x00),	/*< none */
		CFA_MP4_PRS_BIT_STRM_TYPE_V = (0x01 << 0),	/*< video */
		CFA_MP4_PRS_BIT_STRM_TYPE_A = (0x01 << 1),	/*< audio */
		CFA_MP4_PRS_BIT_STRM_TYPE_SP = (0x01 << 2),	/*< sp 0 */
	} CfaMp4PrsBitStrmType;



#ifdef __cplusplus
}
#endif
#endif				/* #ifndef DMX_CFA_MP4_H */
