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


#ifndef _CFA_MP4_UTIL_H_
#define _CFA_MP4_UTIL_H_

#include "x_typedef.h"
#include <media/atc/dmx_cfa_def.h>
#include <media/atc/dmx_cfa_mp4.h>
#include "dmx_spt_cfa.h"
#include "cfa_mp4.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GET_DWORD(puc)										\
	((((u32)*(puc)) << 24) | ((*(puc + 1)) << 16)|		\
	((*(puc + 2)) << 8) | (*(puc + 3)))

#define GET_3BYTE(puc)										\
	((((u32)*(puc)) << 16) | ((*(puc + 1)) << 8) | (*(puc + 2)))


typedef enum {
	Cfa_Mp4_Aud_Track,
	Cfa_Mp4_Vid_Track,
	Cfa_Mp4_Sub_Track,
} ECfaTrackType;


#define CFA_MP4_LIMIT_A_NUM(u2AudStrmNums)					\
		do {                                                \
			if (u2AudStrmNums > MAX_NS_MP4_AUD)				\
					u2AudStrmNums = MAX_NS_MP4_AUD;			\
		} while (0)

#define CFA_MP4_LIMIT_S_NUM(u2SubStrmNums)					\
		do {                                                \
			if (u2SubStrmNums > MAX_NS_MP4_SUB)				\
					u2SubStrmNums = MAX_NS_MP4_SUB;			\
		} while (0)



	u32 CfaMp4GetSpecSize(u8 u1ParamNums, u8 *pucSpecInfo);

	u64 CfaMp4GetRangeCa(CfaMp4Range *prCfaRange);

	CfaApiVidType CfaMp4GetVidCodec(AVCODECID_T eCodec);

	CfaApiAudType CfaMp4GetAudType(AVCODECID_T eType);

	u64 CfaMp4PrsStscForChunkToSample(CfaMp4Inst *prCfaMp4,
		ECfaTrackType eType,
		u64 u8ChunkNo,
		u32 *pu4CurChunkSampleNums);

	u64 CfaMp4PrsStcoGetVideoFileOfst(CfaMp4Inst *prCfaMp4,
		ECfaTrackType eType, u64 u8ChunkNo,
		u64 u8SampleNo);

	u64 CfaMp4PrsSttsGetPts(CfaMp4Inst *prCfaMp4,
		ECfaTrackType eType,
		u64 u8SampleNo
#if SPECIAL_LPCM_SUPPORT
		, u32 u4SampleInc
#endif
	);

	u64 CfaMp4GetRangeEa(CfaMp4Inst *prCfaMp4);

	void CfaMp4FreeSampleAndChunk(CfaMp4Inst *prCfaMp4);

	bool CfaMp4MoofTableValid(MP4_STBL_INFO *prTable);

	u64 ConvertFromStc(u64 u8A, u32 u4B);

	u32 CfaMp4MatchStts(MP4_STBL_INFO *prTable, u64 u8StartSampleNo);

	u64 CfaMp4CreateNewTable2GetEndSmpNo(MP4_STBL_INFO *prTable,
		u32 u4SampleSize,
		TSampleInfo *pTSampleInfo,
		u64 u8CurPrsSampleNo,
		TChunkInfo *pTChunkInfo,
		u64 u8CurPrsChunkNo,
		TTablePosInfo *pTablePosInfo,
		bool fgCO64Valid
#if SPECIAL_LPCM_SUPPORT
		, bool fgReloadSampleTable
		, u32 u4FirstChunkSampleNs/*not the first chunk sampleNO*/
#endif
		, u64 u8MoovSkipSize
	);

	bool CfaMp4BadInterleaved(void *pvSptHdl, CfaMp4Inst *prCfaMp4);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef _CFA_MP4_UTIL_H_*/
