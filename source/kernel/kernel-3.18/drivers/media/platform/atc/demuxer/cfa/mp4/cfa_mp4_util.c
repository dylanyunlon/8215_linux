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


/*#pragma warning(push)*/
/*#pragma warning(disable: 4127) *//*disable warning C4127: conditional expression is constant*/
/*#pragma warning(disable: 4115) *//*disable warning C4115: named type definition in parentheses*/
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
/* #include <media/atc/mm_debug.h> */

#include "cfa_mp4_util.h"
#include "cfa_macro.h"
#include "dmx_def.h"
#include "dmx_mem.h"
#include "cfa_mp4_state.h"
/*#pragma warning(pop)*/

/*#pragma warning(disable: 4127) *//*disable warning C4127: conditional expression is constant*/
u64 u8CfaMp4GetCo64(u8 *puc)
{
	return ((((u64) *(puc)) << 56) |
		 (((u64) *(puc + 1)) << 48) |
		 (((u64) *(puc + 2)) << 40) |
		 (((u64) *(puc + 3)) << 32) |
		 (((u64) *(puc + 4)) << 24) |
		 (((u64) *(puc + 5)) << 16) | (((u64) *(puc + 6)) << 8) | (*(puc + 7)));
}

u64 ConvertFromStc(u64 u8A, u32 u4B)
{
	u64 u8B = 0, u8R = 0;

	u8B = (u64) u4B;
	u8R = (u8A * u8B) / (CFA_STC_CLK);
	return u8R;
}


/*-----------------------------------------------------------------------------
* Name: u4CfaMp4GetSpecSize
*
* Description:
*	   .
*
* Inputs:
*
* Outputs:
*
* Returns: u32
*
*-----------------------------------------------------------------------------*/
u32 CfaMp4GetSpecSize(u8 u1ParamNums, u8 *pucSpecInfo)
{
	u32 u4SpecSize = 0;
	u32 u4Temp = 0;

	while (u1ParamNums--) {
		LOADB_WORD(pucSpecInfo, u4Temp);
		u4SpecSize += u4Temp + 4;
		pucSpecInfo += u4Temp + 2;
		}
	return u4SpecSize;
}


/*
Description: Get the Ca of Range(Minimum of Video/Audio/Subpic)
*/
u64 CfaMp4GetRangeCa(CfaMp4Range *prCfaRange)
{
	u64 u8FileOfst = DMX_INVALID_UINT64;
	u64 u8AudOfst = DMX_INVALID_UINT64;
	u64 u8SubOfst = DMX_INVALID_UINT64;
	u32 i = 0;

	/*Video offset.*/
	if (prCfaRange->u4PrsFlag & CFA_MP4_PRS_BIT_STRM_TYPE_V) {
		u8FileOfst = prCfaRange->u8VidStartOffset;
		u8FileOfst = (u8FileOfst == 0) ? DMX_INVALID_UINT64 : u8FileOfst;
	}

	/*Get the Minimum Audio Offset.*/
	if (prCfaRange->u4PrsFlag & CFA_MP4_PRS_BIT_STRM_TYPE_A) {
		for (i = 0; i < MAX_NS_MP4_AUD; i++) {
			u8AudOfst = prCfaRange->u8AudStartOffset[i];
			if (0 == u8AudOfst)
				u8AudOfst = DMX_INVALID_UINT64;

			u8FileOfst = MIN(u8FileOfst, u8AudOfst);
		}
	}

	/* Get the Minimum Subpicture Offset.*/
	if (prCfaRange->u4PrsFlag & CFA_MP4_PRS_BIT_STRM_TYPE_SP) {
		for (i = 0; i < MAX_NS_MP4_SUB; i++) {
			u8SubOfst = prCfaRange->u8SubStartOffset[i];
			if (0 == u8SubOfst)
				u8SubOfst = DMX_INVALID_UINT64;

			u8FileOfst = MIN(u8FileOfst, u8SubOfst);
		}
	}
	if (DMX_INVALID_UINT64 == u8FileOfst)
		u8FileOfst = 0;

	return u8FileOfst;
}


/*
Description: Get the Ea of Range(Minimum of Video/Audio/Subpic)
*/
u64 CfaMp4GetRangeEa(CfaMp4Inst *prCfaMp4)
{
	u64 u8Ca = 0, u8Ea = 0;

	u8Ca = CfaMp4GetRangeCa(&prCfaMp4->rCfaRange);
	u8Ea = u8Ca + prCfaMp4->rCfaRange.u8Length - 1;
	return u8Ea;
}

void CfaMp4FreeSampleAndChunk(CfaMp4Inst *prCfaMp4)
{
	u32 i = 0;

	DMX_FreeMemory(prCfaMp4->rCfaMp4VInf.pTChunkInfo);
	DMX_FreeMemory(prCfaMp4->rCfaMp4VInf.pTSampleInfo);
	CFA_MP4_LIMIT_S_NUM(prCfaMp4->u2SubStrmNums);
	for (i = 0; i < prCfaMp4->u2SubStrmNums; i++) {
		DMX_FreeMemory(prCfaMp4->rCfaMp4SInf[i].pTChunkInfo);
		DMX_FreeMemory(prCfaMp4->rCfaMp4SInf[i].pTSampleInfo);
	}
	CFA_MP4_LIMIT_A_NUM(prCfaMp4->u2AudStrmNums);
	for (i = 0; i < prCfaMp4->u2AudStrmNums; i++) {
		DMX_FreeMemory(prCfaMp4->rCfaMp4AInf[i].pTChunkInfo);
		DMX_FreeMemory(prCfaMp4->rCfaMp4AInf[i].pTSampleInfo);
	}
}


/*
 Mapping the video codec type
*/
CfaApiVidType CfaMp4GetVidCodec(AVCODECID_T eCodec)
{
	switch (eCodec) {
	case AVCODEC_ID_MPEG1:
		return CFA_VID_MPEG2;
    case AVCODEC_ID_MPEG2:
		return CFA_VID_MPEG2;
	case AVCODEC_ID_DIVX3:
		return CFA_VID_DIVX3;
	case AVCODEC_ID_MPEG4:
		return CFA_VID_MPEG4;
	case AVCODEC_ID_H263:
		return CFA_VID_H263;
	case AVCODEC_ID_H264:
		return CFA_VID_H264;
	case AVCODEC_ID_WMV1:
		return CFA_VID_WMV7;
	case AVCODEC_ID_WMV2:
		return CFA_VID_WMV8;
	case AVCODEC_ID_WMV3:
		return CFA_VID_WMV9;
	case AVCODEC_ID_VC1:
		return CFA_VID_VC1;
	case AVCODEC_ID_DIVX4:
		return CFA_VID_DIVX4;
	case AVCODEC_ID_DIVX6:
		return CFA_VID_DIVX6;
	case AVCODEC_ID_SORENSON:
		return CFA_VID_H263_SORENSON;
	case AVCODEC_ID_H265:
		return CFA_VID_H265;
	case AVCODEC_ID_VP6:
		return CFA_VID_VP6;
	case AVCODEC_ID_VP8:
		return CFA_VID_VP8;
	case AVCODEC_ID_MJPEG:
		return CFA_VID_MJPEG;
	case AVCODEC_ID_RV:
		return CFA_VID_RV40;
	default:
		return CFA_VID_UNKNOWN;
	}
}

bool CfaMp4MoofTableValid(MP4_STBL_INFO *prTable)
{
	if (prTable[0].fgValid && prTable[1].fgValid &&
		prTable[2].fgValid && prTable[3].fgValid)
		return TRUE;

	return FALSE;
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4MatchStts
*
* Description:
*
*
* Inputs:
*
* Outputs:
*
* Returns: None
*
*-----------------------------------------------------------------------------*/
u32 CfaMp4MatchStts(MP4_STBL_INFO *prTable, u64 u8StartSampleNo)
{
	u32 u4Ret = 1;
	u32 u4Temp = 0;
	MP4_STBL_INFO *prTTS = &prTable[STTS];
	u8 *pucSttsEntry = (u8 *) prTTS->pvEntry;

	for (u4Temp = 0; u4Temp < prTTS->u4EntryLb; u4Temp++) {
		u4Ret += GET_DWORD(pucSttsEntry);
		pucSttsEntry += 8;
	}
	if (u4Ret < u8StartSampleNo) {
		u4Ret = 1;
		pucSttsEntry = (u8 *) prTTS->pvEntry;
		prTTS->u4EntryLb = 1;
		for (; u4Ret <= u8StartSampleNo;) {
			u4Ret += GET_DWORD(pucSttsEntry);
			pucSttsEntry += 8;
			prTTS->u4EntryLb++;
		}
		pucSttsEntry -= 8;
		prTTS->u4EntryLb--;
		if (prTTS->u4EntryLb > prTTS->u4EntryUb)
			prTTS->u4EntryLb = prTTS->u4EntryUb;

	}
	u4Ret = u4Ret - (u32) u8StartSampleNo;
	if (0 == prTTS->u4EntryUb) {	/*temp , for 121395 , the ub will to be 0.*/
		prTTS->u4EntryUb = prTTS->u4Allotted;
	}
	return u4Ret;
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4CreateNewTable2GetEndSmpNo
*
* Description:
*	  Creat Internal new table for prser,and get the sample No of cur table
*
* Inputs:
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
u64 CfaMp4CreateNewTable2GetEndSmpNo(MP4_STBL_INFO *prTable, u32 u4SampleSize,
						TSampleInfo *pTSampleInfo, u64 u8CurPrsSampleNo,
						TChunkInfo *pTChunkInfo, u64 u8CurPrsChunkNo,
						TTablePosInfo *pTablePosInfo, bool fgCO64Valid
#if	SPECIAL_LPCM_SUPPORT
						, bool fgReloadSampleTable , u32 u4FirstChunkSampleNs
						/* not the first chunk sampleNO*/
#endif
						, u64 u8MoovSkipSize)
{
	u64 u8StcoGetOfst = 0;
	u8 *pucSttsEntry = (u8 *) prTable[STTS].pvEntry;
	u8 *pucStscEntry = (u8 *) prTable[STSC].pvEntry;
	u8 *pucStszEntry = (u8 *) prTable[STSZ].pvEntry;
	u8 *pucStcoEntry = (u8 *) prTable[STCO].pvEntry;

	if (((pTablePosInfo->u4TabelUb[STCO] - u8CurPrsChunkNo) >
		(pTablePosInfo->u4ChunkBufNums - 10))
		   ||
		   (((pTablePosInfo->u4TabelUb[STSZ] - u8CurPrsSampleNo) >
		  (pTablePosInfo->u4SampleBufNums - 10)) && (0 == u4SampleSize))) {
		return pTablePosInfo->u8CurSampleNo;
	}
	pucStszEntry += ENTRY_SIZE_STSZ * (prTable[STSZ].u4EntryLb - 1);
	pucSttsEntry += ENTRY_SIZE_STTS * (prTable[STTS].u4EntryLb - 1);
	pucStscEntry += ENTRY_SIZE_STSC * (prTable[STSC].u4EntryLb - 1);
	pucStcoEntry +=
		(fgCO64Valid ? ENTRY_SIZE_CO64 : ENTRY_SIZE_STCO) * (prTable[STCO].u4EntryLb - 1);
	if ((pTablePosInfo->u4TabelUb[STSZ] <= prTable[STSZ].u4EntryUb)
		   && (pTablePosInfo->u4TabelUb[STTS] <= prTable[STTS].u4EntryUb)
		   && (pTablePosInfo->u4TabelUb[STSC] <= prTable[STSC].u4EntryUb)
		   && (pTablePosInfo->u4TabelUb[STCO] <= prTable[STCO].u4EntryUb)) {
		u32 u4SampleNumsSameDur = 0;
		TChunkInfo *pTChunk = pTChunkInfo;
		TSampleInfo *pTSample = pTSampleInfo;

#if SPECIAL_LPCM_SUPPORT
		if (fgReloadSampleTable) {

#endif
			do {
				if (0 == u4SampleSize) {
					if (pucStszEntry)
						pTSample->u4SampleSize = GET_DWORD(pucStszEntry);

					pucStszEntry += 4;
					pTablePosInfo->u4TabelUb[STSZ]++;
				}
				if (!u4SampleNumsSameDur && pucSttsEntry)
					u4SampleNumsSameDur = GET_DWORD(pucSttsEntry);

				if (pucSttsEntry)
					pTSample->u4SampleDur = GET_DWORD((pucSttsEntry + 4));

				if (0 == u4SampleSize) {
					pTSample++;
				} else if (pucSttsEntry) {
					pTSample->u4SampleSize =
						GET_DWORD(pucSttsEntry) -
						pTablePosInfo->u4SttsInvaildSampleNums + 1;
				}
				pTablePosInfo->u8CurSampleNo++;
				if (0 != pTablePosInfo->u4SttsInvaildSampleNums) {
					pTablePosInfo->u4SttsInvaildSampleNums--;
					u4SampleNumsSameDur =
						pTablePosInfo->u4SttsInvaildSampleNums;
				} else
					u4SampleNumsSameDur--;

				if (0 == u4SampleNumsSameDur) {
					pucSttsEntry += 8;
					if (0 == u4SampleSize)
						pTablePosInfo->u4TabelUb[STTS]++;
					else
						pTSample++;

				}
				if (pucStcoEntry) {
					if (!fgCO64Valid)
						pTChunk->u8ChunkOfst = GET_DWORD(pucStcoEntry);
					else
						pTChunk->u8ChunkOfst = u8CfaMp4GetCo64(pucStcoEntry);

				}
				if ((pucStscEntry != NULL) && (GET_DWORD(pucStscEntry) == GET_DWORD(pucStscEntry + 12)))
				{//fix CNB00239267
					pucStscEntry += 12;
					DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
						TEXT("[CFA MP4]STSC has error data, jump data success!\n"));
				}
				if (pucStscEntry) {
					pTChunk->u4SampleNums = GET_DWORD(pucStscEntry + 4);
					pTChunk->u4SampleDecIndex = GET_DWORD(pucStscEntry + 8);
				}
				pTChunk->u4LastSampleNo = (u32) pTablePosInfo->u8CurSampleNo - 1;
				pTablePosInfo->u4PrsDoneSampleNumsEveryChunk++;
				if (pTChunk->u4SampleNums <=
					 pTablePosInfo->u4PrsDoneSampleNumsEveryChunk) {
					if (pTablePosInfo->u4TabelUb[STCO] <
						 prTable[STCO].u4EntryUb) {
						if (!fgCO64Valid) {
							pucStcoEntry += 4;
							if (pucStcoEntry)
								u8StcoGetOfst = GET_DWORD(pucStcoEntry);
						} else {
							pucStcoEntry += 8;
							if (pucStcoEntry)
								u8StcoGetOfst = u8CfaMp4GetCo64(pucStcoEntry);

						}
					}
					++(pTablePosInfo->u4TabelUb[STCO]);
					++pTChunk;
					++(pTablePosInfo->u4CurTableLastChunkNo);
					pTablePosInfo->u4PrsDoneSampleNumsEveryChunk = 0;
					if ((u8StcoGetOfst < (pTChunk - 1)->u8ChunkOfst) &&
						   (pTablePosInfo->u4TabelUb[STCO] <=
						prTable[STCO].u4EntryUb)) {
						break;
					}
				}
				if (pTablePosInfo->u4TabelUb[STSC] < prTable[STSC].u4EntryUb) {
					if ((pucStscEntry != NULL)
						 && (GET_DWORD(pucStscEntry + 12) <
						 GET_DWORD(pucStscEntry))) {
							break;
					}
					if ((pucStscEntry != NULL)
						   && (1 != pTablePosInfo->u4CurTableLastChunkNo)
						   && (pTablePosInfo->u4CurTableLastChunkNo >=
							GET_DWORD(pucStscEntry + 12))) {
						pucStscEntry += 12;
						++(pTablePosInfo->u4TabelUb[STSC]);
					}
					if ((pTablePosInfo->u4TabelUb[STSC] ==
						prTable[STSC].u4EntryUb)
						   && (pTablePosInfo->u4TabelUb[STSC] !=
							prTable[STSC].u4EntryNs)) {
						break;
					}
				}
				if (((pTablePosInfo->u4TabelUb[STCO] - u8CurPrsChunkNo) >
					(pTablePosInfo->u4ChunkBufNums - 10))
					   ||
					   (((pTablePosInfo->u4TabelUb[STSZ] - u8CurPrsSampleNo) >
					  (pTablePosInfo->u4SampleBufNums - 10))
					 && (0 == u4SampleSize))) {
					break;
				}
			} while ((pTablePosInfo->u4TabelUb[STSZ] <=
						prTable[STSZ].u4EntryUb)
					   && (pTablePosInfo->u4TabelUb[STTS] <=
							prTable[STTS].u4EntryUb)
					   && (pTablePosInfo->u4TabelUb[STSC] <=
							prTable[STSC].u4EntryUb)
					   && (pTablePosInfo->u4TabelUb[STCO] <=
							prTable[STCO].u4EntryUb));

#if SPECIAL_LPCM_SUPPORT
		} else {
			DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("CfaMp4CreateNewTable2GetEndSmpNo lpcm or quicktime version 1")
				TEXT("u4LpcmFirstTxSampleCount\n"));
			if (pucSttsEntry != NULL)
				pTSample->u4SampleDur = GET_DWORD((pucSttsEntry + 4));
			if (pucSttsEntry != NULL)
				pTSample->u4SampleSize = GET_DWORD(pucSttsEntry);
			if (pTSample->u4SampleSize != u4SampleSize) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4]now get stss samplesize[%u] error update[%u]\n"),
						 pTSample->u4SampleSize, u4SampleSize);
				pTSample->u4SampleSize = u4SampleSize;
			}
			pTablePosInfo->u4TabelUb[STSZ] = (prTable + STSZ)->u4EntryUb + 1;
			DmxLogT(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("CfaMp4CreateNewTable2GetEndSmpNo u8CurSampleNo = %llx u4SampleDur[%d],")
				TEXT("u4SampleSize[%d]\n"),
				pTablePosInfo->u8CurSampleNo, pTSample->u4SampleDur,
				pTSample->u4SampleSize);

			do {
				if (pucStcoEntry != NULL) {
					if (!fgCO64Valid)
						pTChunk->u8ChunkOfst = GET_DWORD(pucStcoEntry);

					else
						pTChunk->u8ChunkOfst =
							u8CfaMp4GetCo64(pucStcoEntry);
					pTChunk->u8ChunkOfst += u8MoovSkipSize;
				}
				if (pucStscEntry != NULL) {
					pTChunk->u4SampleNums = GET_DWORD((pucStscEntry + 4));
					pTChunk->u4SampleDecIndex = GET_DWORD((pucStscEntry + 8));
					if (u4FirstChunkSampleNs > 0) {
						pTablePosInfo->u8CurSampleNo +=
							(pTChunk->u4SampleNums - u4FirstChunkSampleNs);
						u4FirstChunkSampleNs = 0;
					}

					else {
						pTablePosInfo->u8CurSampleNo +=
							pTChunk->u4SampleNums;
					}

/*x_dbg_stmt("CfaMp4CreateNewTable2GetEndSmpNo u4SampleNums = %ux,u4SampleDecIndex = %u\n"*/
/*, pTChunk->u4SampleNums,pTChunk->u4SampleDecIndex);*/
				}
				pTChunk->u4LastSampleNo =
					(u32) pTablePosInfo->u8CurSampleNo - 1;
				pTablePosInfo->u4PrsDoneSampleNumsEveryChunk =
					pTChunk->u4SampleNums;
				if (!fgCO64Valid)
					pucStcoEntry += 4;
				else
					pucStcoEntry += 8;

				pTablePosInfo->u4PrsDoneSampleNumsEveryChunk = 0;

/*x_dbg_stmt("[num] = %d, num = %d u4LastSampleNo = %d\n", pTablePosInfo->u4TabelUb[STCO],*/
/*pTablePosInfo->u4CurTableLastChunkNo, pTChunk->u4LastSampleNo);*/
				if ((pTablePosInfo->u4TabelUb[STCO] >=
					(prTable + STCO)->u4EntryUb))
					break;
				pTablePosInfo->u4CurTableLastChunkNo++;
				pTablePosInfo->u4TabelUb[STCO]++;
				pTChunk++;
				if (pTablePosInfo->u4TabelUb[STSC] <
					 (prTable + STSC)->u4EntryUb) {
					if ((pucStscEntry != NULL)
						 && (GET_DWORD(pucStscEntry + 12) <
						 GET_DWORD(pucStscEntry)))
						break;
					if ((pucStscEntry != NULL)
						   && (1 != pTablePosInfo->u4CurTableLastChunkNo)
						   && (pTablePosInfo->u4CurTableLastChunkNo >=
							GET_DWORD(pucStscEntry + 12))) {
						pucStscEntry += 12;
						pTablePosInfo->u4TabelUb[STSC]++;
					}
					if ((pTablePosInfo->u4TabelUb[STSC] ==
						(prTable + STSC)->u4EntryUb)
						   && (pTablePosInfo->u4TabelUb[STSC] !=
						   (prTable + STSC)->u4EntryNs))
						break;
				}
				if (((pTablePosInfo->u4TabelUb[STCO] - u8CurPrsChunkNo) >
					(pTablePosInfo->u4ChunkBufNums - 10))
					   && (0 == u4SampleSize))
					break;
			} while ((pTablePosInfo->u4TabelUb[STSC] <=
						 (prTable + STSC)->u4EntryUb)
						&& (pTablePosInfo->u4TabelUb[STCO] <=
							 (prTable + STCO)->u4EntryUb));
			}

#endif
#if 0
			if (pTChunk != pTChunkInfo) {	/* pTChunk pointer added at least one.*/
			pTChunk->u4LastSampleNo =
				(pTChunk - 1)->u4LastSampleNo + pTChunk->u4SampleNums;
	}

#endif
			pTablePosInfo->u4SttsInvaildSampleNums = u4SampleNumsSameDur;
	}
	return pTablePosInfo->u8CurSampleNo;
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4BadInterleaved
*
* Description:
*
*
* Inputs:
*
* Outputs:
*
* Returns: None
*
*-----------------------------------------------------------------------------*/
bool CfaMp4BadInterleaved(void *pvSptHdl, CfaMp4Inst *prCfaMp4)
{
	u64 u8Max = 0;
	u64 u8Min = 0;
	u32 u4Loop = 0;

	prCfaMp4->fgFinished = TRUE;
	prCfaMp4->eCurCfaMp4AnaSt = CFA_MP4_ANA_PRS_NEXT_STATE;
	u4Loop = 150000;
	while (u4Loop) {
		CfaMp4TxDoneStCtrl(pvSptHdl, prCfaMp4);
		if (CFA_MP4_ANA_RELOAD_TABLE == prCfaMp4->eCurCfaMp4AnaSt) {
			if (CFA_MP4_PRS_BIT_STRM_TYPE_V == prCfaMp4->eCurPrsSampleType) {
				u8Max = DMX_MAX(prCfaMp4->u8Vpts,
					prCfaMp4->u8CurAPts[prCfaMp4->u4CurAudInfoId]);
				u8Min = DMX_MIN(prCfaMp4->u8Vpts,
					prCfaMp4->u8CurAPts[prCfaMp4->u4CurAudInfoId]);
			}
			if (CFA_MP4_PRS_BIT_STRM_TYPE_A == prCfaMp4->eCurPrsSampleType) {
				u8Max = DMX_MAX(prCfaMp4->u8CurVPts,
					prCfaMp4->u8Apts[prCfaMp4->u4CurAudInfoId]);
				u8Min = DMX_MIN(prCfaMp4->u8CurVPts,
					prCfaMp4->u8Apts[prCfaMp4->u4CurAudInfoId]);
			}
			u4Loop--;
			if ((u8Max - u8Min) > (CFA_STC_CLK * 3)) {
				DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4] u4Loop1= %d\n"), u4Loop);
				return TRUE;
			}
		}
		if (CFA_MP4_ANA_PRS_NEXT_STATE == prCfaMp4->eCurCfaMp4AnaSt) {
			if (CFA_MP4_PRS_BIT_STRM_TYPE_V == prCfaMp4->eCurPrsSampleType) {
				u8Max = DMX_MAX(prCfaMp4->u8Vpts,
					prCfaMp4->u8CurAPts[prCfaMp4->u4CurAudInfoId]);
				u8Min = DMX_MIN(prCfaMp4->u8Vpts,
					prCfaMp4->u8CurAPts[prCfaMp4->u4CurAudInfoId]);
			}
			if (CFA_MP4_PRS_BIT_STRM_TYPE_A == prCfaMp4->eCurPrsSampleType) {
				u8Max = DMX_MAX(prCfaMp4->u8CurVPts,
					prCfaMp4->u8Apts[prCfaMp4->u4CurAudInfoId]);
				u8Min = DMX_MIN(prCfaMp4->u8CurVPts,
					prCfaMp4->u8Apts[prCfaMp4->u4CurAudInfoId]);
			}
			u4Loop--;
			if ((u8Max - u8Min) > (CFA_STC_CLK * 3)) {
				DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4] u4Loop2= %d\n"), u4Loop);
				return TRUE;
			}
		}
		if ((CFA_MP4_PRS_BIT_STRM_TYPE_NONE ==
			(prCfaMp4->u4CurPrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_V))
			   || (CFA_MP4_PRS_BIT_STRM_TYPE_NONE ==
				(prCfaMp4->u4CurPrsFlg & CFA_MP4_PRS_BIT_STRM_TYPE_A))) {
			return FALSE;
		}
	}
	return FALSE;
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4PrsStscForChunkToSample
*
* Description:
*	   input the chunkNo, get the NO.of the first sample in cur chunk
get the sample numbers in cur chunk
*
* Inputs: eType, u8ChunkNo,
*
* Outputs:pu8CurSampleNo,pu4CurChunkSampleNums
*
* Returns:
*
*-----------------------------------------------------------------------------*/
u64 CfaMp4PrsStscForChunkToSample(CfaMp4Inst *prCfaMp4, ECfaTrackType eType,
u64 u8ChunkNo, u32 *pu4CurChunkSampleNums)
{
	TChunkInfo *pTemp = NULL;
	u64 u8StartSampleNo = 0;

	if (NULL == pu4CurChunkSampleNums)
		return u8StartSampleNo;

	switch (eType) {
	case Cfa_Mp4_Vid_Track:
		pTemp =
			prCfaMp4->rCfaMp4VInf.pTChunkInfo + (u8ChunkNo -
							 prCfaMp4->rCfaMp4VInf.u8CurVidTableStartChunkNo);
		break;
	case Cfa_Mp4_Sub_Track:
		pTemp = prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].pTChunkInfo +
			(u8ChunkNo -
			 prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurSubTableStartChunkNo);
		break;
	case Cfa_Mp4_Aud_Track:
		pTemp = prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pTChunkInfo +
			(u8ChunkNo -
			 prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurAudTableStartChunkNo);
		break;
	default:
		break;
	}
	if (NULL == pTemp)
		return u8StartSampleNo;

	*pu4CurChunkSampleNums = pTemp->u4SampleNums;
	prCfaMp4->u4EsdIndex = pTemp->u4SampleDecIndex;
	u8StartSampleNo = pTemp->u4LastSampleNo - pTemp->u4SampleNums + 1;
	return u8StartSampleNo;
}


/*-----------------------------------------------------------------------------
* Name: u8CfaMp4PrsSttsGetPts
*
* Description:
*	   get pts from stts box
*
* Inputs: eType, u8SampleNo
*
* Outputs:pu4CurPts
*
* Returns:
*
*-----------------------------------------------------------------------------*/
u64 CfaMp4PrsSttsGetPts(CfaMp4Inst *prCfaMp4, ECfaTrackType eType, u64 u8SampleNo
#if SPECIAL_LPCM_SUPPORT
				  , u32 u4SampleInc
#endif
				  ) {
	u64 u8CurSampleDur = 0;
	u64 u8CurPts = 0;
	u64 u8Pts = 0;
	u64 u8SampleNums = 0;
	TSampleInfo *pTSample = NULL;

	if (NULL == prCfaMp4)
		return u8Pts;

	switch (eType) {
	case Cfa_Mp4_Vid_Track:
		pTSample = prCfaMp4->rCfaMp4VInf.pTSampleInfo;
		if (NULL == pTSample) {
			return u8Pts;
		}

	/* error handle*/
		if (pTSample->u4SampleDur >= 0x80000000)
			pTSample->u4SampleDur = 3000;

		if (0 == prCfaMp4->rCfaMp4VInf.u4SampleSize) {
			if (NULL !=
				 (pTSample +
				  (u8SampleNo - prCfaMp4->rCfaMp4VInf.u8CurVidTableStartSampleNo))) {
				u8CurSampleDur =
					(pTSample + (u8SampleNo -
					  prCfaMp4->rCfaMp4VInf.u8CurVidTableStartSampleNo))->u4SampleDur;
			}
		} else {
			for (u8SampleNums = pTSample->u4SampleSize;
			(u8SampleNo - prCfaMp4->rCfaMp4VInf.u8CurVidTableStartSampleNo) > u8SampleNums;) {
				pTSample++;
				u8SampleNums += pTSample->u4SampleSize;
			}
			u8CurSampleDur = pTSample->u4SampleDur;
		}
		u8CurPts = u8CurSampleDur * CFA_STC_CLK;
		if (0 != prCfaMp4->rCfaMp4VInf.u4TimeScale) {
			u8CurPts = u8CurPts / prCfaMp4->rCfaMp4VInf.u4TimeScale;
			if (u8CurPts > (prCfaMp4->u8VDuration * 10000) && (0 != prCfaMp4->u8VDuration)) {
				/*errhandle duration jump to larger*/
				u8CurSampleDur = prCfaMp4->u8VDuration;
				u8CurSampleDur =
					u8CurSampleDur * prCfaMp4->rCfaMp4VInf.u4TimeScale / CFA_STC_CLK;
			} else
				prCfaMp4->u8VDuration = u8CurPts;

		}
		prCfaMp4->u8CurVDuation += u8CurSampleDur;
		if (0 != prCfaMp4->rCfaMp4VInf.u4TimeScale) {
			u8Pts =
				(CFA_STC_CLK * prCfaMp4->u8CurVDuation) /
				prCfaMp4->rCfaMp4VInf.u4TimeScale;
		} else {
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[MP4 CFA][%s] Cfa_Mp4_Vid_Track u4TimeScale = 0,")
				TEXT("so Set PTS = 0! \r\n"), __func__);
			u8Pts = 0;
		}
		break;
	case Cfa_Mp4_Sub_Track:
		pTSample = prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].pTSampleInfo;
		if (NULL == pTSample) {
			return u8Pts;
		}
		if (0 == prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u4SampleSize) {
			u8CurSampleDur =
				(pTSample + (u8SampleNo -
				prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurSubTableStartSampleNo))
				->u4SampleDur;
		} else {
			for (u8SampleNums = pTSample->u4SampleSize;
				  (u8SampleNo -
					prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurSubTableStartSampleNo) >
					u8SampleNums;) {
				pTSample++;
				u8SampleNums += pTSample->u4SampleSize;
			}
			u8CurSampleDur = pTSample->u4SampleDur;
		}
		u8CurPts = u8CurSampleDur * CFA_STC_CLK;
		if (0 != prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u4TimeScale) {
			u8CurPts =
				u8CurPts / prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u4TimeScale;
		}
		prCfaMp4->u8CurSDuation[prCfaMp4->u4CurSubInfoId] += u8CurSampleDur;
		if (0 != prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u4TimeScale) {
			u8Pts =
				(CFA_STC_CLK * prCfaMp4->u8CurSDuation[prCfaMp4->u4CurSubInfoId]) /
				prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u4TimeScale;
		} else {
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[MP4 CFA][%s] Cfa_Mp4_Sub_Track u4TimeScale = 0,")
				TEXT("so Set PTS = 0! \r\n"), __func__);
			u8Pts = 0;
		}
		break;
	case Cfa_Mp4_Aud_Track:
		pTSample = prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pTSampleInfo;
		if (NULL == pTSample) {
			return u8Pts;
		}

		if (0 == prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4SampleSize) {
			u8CurSampleDur =
				(pTSample + (u8SampleNo -
				prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurAudTableStartSampleNo))
				->u4SampleDur;
		} else {

#if SPECIAL_LPCM_SUPPORT	/*Li Lu*/
			if (AVCODEC_ID_PCM == prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].eAudType)
				u8CurSampleDur = (u64)pTSample->u4SampleDur * u4SampleInc;


#endif

			else {
				for (u8SampleNums = pTSample->u4SampleSize;
					  (u8SampleNo -
						prCfaMp4
						->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurAudTableStartSampleNo) >
						u8SampleNums;) {

#if SPECIAL_LPCM_SUPPORT	/*Li Lu*/
					pTSample += u4SampleInc;
					u8SampleNums += (pTSample->u4SampleSize * u4SampleInc);

#else
					pTSample++;
					u8SampleNums += pTSample->u4SampleSize;

#endif
				}

#if SPECIAL_LPCM_SUPPORT
					u8CurSampleDur = (u64)pTSample->u4SampleDur * u4SampleInc;

#else
					u8CurSampleDur = pTSample->u4SampleDur;

#endif
		}
	}
		u8CurPts = u8CurSampleDur * CFA_STC_CLK;
		if (0 != prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4TimeScale) {
			u8CurPts =
				u8CurPts / prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4TimeScale;
		}
		prCfaMp4->u8CurADuation[prCfaMp4->u4CurAudInfoId] += u8CurSampleDur;
		if (0 != prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4TimeScale) {
			u8Pts =
				(CFA_STC_CLK * prCfaMp4->u8CurADuation[prCfaMp4->u4CurAudInfoId]) /
				prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4TimeScale;
		} else {
			DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[MP4 CFA][%s] Cfa_Mp4_Aud_Track u4TimeScale = 0,")
				TEXT("so Set PTS = 0! \r\n"), __func__);
			u8Pts = 0;
		}
		break;
	default:
		break;
	}
	return u8Pts;
}


/*-----------------------------------------------------------------------------
* Name: u8CfaMp4PrsStszGetSampleSize
*
* Description:
*	   get the all of size from start to end  sampleNo.  from stsz box
*
* Inputs: eType, u8startSampleNo,u8EndSampleNo,
*
* Outputs:pu8allSampleSize
*
* Returns:
*
*-----------------------------------------------------------------------------*/
u64 CfaMp4PrsStszGetSampleSize(CfaMp4Inst *prCfaMp4, ECfaTrackType eType,
					   u64 u8StartSampleNo, u64 u8EndSampleNo)
{
	u64 u8AllSampleSize = 0;

	TSampleInfo *pTemp = NULL;

	switch (eType) {
	case Cfa_Mp4_Vid_Track:
		if (0 == prCfaMp4->rCfaMp4VInf.u4SampleSize) {
			pTemp =
				prCfaMp4->rCfaMp4VInf.pTSampleInfo + (u8StartSampleNo -
								  prCfaMp4->rCfaMp4VInf.
								  u8CurVidTableStartSampleNo);
			for (; u8StartSampleNo < u8EndSampleNo; u8StartSampleNo++, pTemp++)
				u8AllSampleSize += pTemp->u4SampleSize;

		} else {
			for (; u8StartSampleNo < u8EndSampleNo; u8StartSampleNo++)
				u8AllSampleSize += prCfaMp4->rCfaMp4VInf.u4SampleSize;

		}
		break;
	case Cfa_Mp4_Sub_Track:
		if (0 == prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u4SampleSize) {
			pTemp = prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].pTSampleInfo +
				(u8StartSampleNo -
				 prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurSubTableStartSampleNo);
			for (; u8StartSampleNo < u8EndSampleNo; u8StartSampleNo++, pTemp++)
				u8AllSampleSize += pTemp->u4SampleSize;

		} else {
			for (; u8StartSampleNo < u8EndSampleNo; u8StartSampleNo++)
				u8AllSampleSize += prCfaMp4->rCfaMp4VInf.u4SampleSize;

		}
		break;
	case Cfa_Mp4_Aud_Track:
		if (0 == prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u4SampleSize) {
			pTemp = prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pTSampleInfo +
				(u8StartSampleNo -
				 prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurAudTableStartSampleNo);
			for (; u8StartSampleNo < u8EndSampleNo; u8StartSampleNo++, pTemp++)
				u8AllSampleSize += pTemp->u4SampleSize;

		} else {
			for (; u8StartSampleNo < u8EndSampleNo; u8StartSampleNo++)
				u8AllSampleSize += prCfaMp4->rCfaMp4VInf.u4SampleSize;

		}
		break;
	default:
		break;
	}
	return u8AllSampleSize;
}


/*-----------------------------------------------------------------------------
* Name: CfaMp4PrsStcoGetVideoFileOfst
*
* Description:
*	 get cur fileoffset with parsering the stco box
*
* Inputs: eType, u8ChunkNo,u8SampleNo
*
* Outputs:pu8FileOffset
*
* Returns:
*
*-----------------------------------------------------------------------------*/
u64 CfaMp4PrsStcoGetVideoFileOfst(CfaMp4Inst *prCfaMp4, ECfaTrackType eType,
					  u64 u8ChunkNo, u64 u8SampleNo)
{
	TChunkInfo *pTemp = NULL;
	u64 u8StartSampleNo = 0;
	u64 u8AllSampleSize = 0;
	u64 u8FileOffset = 0;
	u32 u4CurChunkSampleNums = 0;

	switch (eType) {
	case Cfa_Mp4_Vid_Track:
		pTemp = prCfaMp4->rCfaMp4VInf.pTChunkInfo +
			(u8ChunkNo - prCfaMp4->rCfaMp4VInf.u8CurVidTableStartChunkNo);
		u8StartSampleNo =
			CfaMp4PrsStscForChunkToSample(prCfaMp4, Cfa_Mp4_Vid_Track, u8ChunkNo,
						  &u4CurChunkSampleNums);
		u8AllSampleSize =
			CfaMp4PrsStszGetSampleSize(prCfaMp4, Cfa_Mp4_Vid_Track, u8StartSampleNo,
						   u8SampleNo);
		break;
	case Cfa_Mp4_Sub_Track:
		pTemp =
			prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].pTChunkInfo + (u8ChunkNo -
			prCfaMp4->rCfaMp4SInf[prCfaMp4->u4CurSubInfoId].u8CurSubTableStartChunkNo);
		u8StartSampleNo =
			CfaMp4PrsStscForChunkToSample(prCfaMp4, Cfa_Mp4_Sub_Track, u8ChunkNo,
						  &u4CurChunkSampleNums);
		u8AllSampleSize =
			CfaMp4PrsStszGetSampleSize(prCfaMp4, Cfa_Mp4_Sub_Track, u8StartSampleNo,
						   u8SampleNo);
		break;
	case Cfa_Mp4_Aud_Track:
		pTemp =
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].pTChunkInfo + (u8ChunkNo -
			prCfaMp4->rCfaMp4AInf[prCfaMp4->u4CurAudInfoId].u8CurAudTableStartChunkNo);
		u8StartSampleNo =
			CfaMp4PrsStscForChunkToSample(prCfaMp4, Cfa_Mp4_Aud_Track, u8ChunkNo,
						  &u4CurChunkSampleNums);
		u8AllSampleSize =
			CfaMp4PrsStszGetSampleSize(prCfaMp4, Cfa_Mp4_Aud_Track, u8StartSampleNo,
						   u8SampleNo);
		break;
	default:
		break;
	}
	if (NULL == pTemp) {
		return u8FileOffset;
	}
	u8FileOffset = pTemp->u8ChunkOfst;
	u8FileOffset += u8AllSampleSize;
	return u8FileOffset;
}

CfaApiAudType CfaMp4GetAudType(AVCODECID_T eType)
{
	switch (eType) {
	case AVCODEC_ID_AAC_PURE:
	case AVCODEC_ID_AAC:
		return CFA_AUD_DRV_FMT_AAC;
	case AVCODEC_ID_MP3:
		return CFA_AUD_DRV_FMT_MP3;
	case AVCODEC_ID_PCM:
		return CFA_AUD_DRV_FMT_PCM;
	default:
		return CFA_AUD_DRV_FMT_UNKNOWN;
	}
}
