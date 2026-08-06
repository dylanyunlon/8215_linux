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
#include <media/atc/dmx_cfa_def.h>
#include <media/atc/dmx_cfa_mp4.h>
/* #include <media/atc/mm_debug.h> */

#include "cfa_mp4.h"
#include "cfa_mp4_util.h"
#include "dmx_def.h"
#include "dmx_mem.h"
/*#pragma warning(pop)*/

MRESULT CfaMp4CfgStmTbl(MP4_STBL_INFO **prMp4ToTbl, MP4_STBL_INFO *prMp4FromTbl)
{
	u32 i = 0;
	u32 u4EntrySize = 0;

	DMX_NewMemory(MAX_MP4_SMP_TBL * sizeof(MP4_STBL_INFO), *prMp4ToTbl);
	if (*prMp4ToTbl) {
		dmx_memcpy(*prMp4ToTbl, prMp4FromTbl, MAX_MP4_SMP_TBL * sizeof(MP4_STBL_INFO));
		for (i = 0; i < MAX_MP4_SMP_TBL; i++) {	/*for klockwork issues*/
			(*prMp4ToTbl)[i].pvEntry = NULL;
		}
	}

	else {
		MM_RETURN(RET_DMX_NO_MEM);
	}
	for (i = 0; i < MAX_MP4_SMP_TBL; i++) {
		if (STTS == i) {
			u4EntrySize = prMp4FromTbl[i].u4EntryNs * ENTRY_SIZE_STTS;
		}

		else if (STSC == i) {
			u4EntrySize = prMp4FromTbl[i].u4EntryNs * ENTRY_SIZE_STSC;
		}

		else if (STSZ == i)
			u4EntrySize = prMp4FromTbl[i].u4EntryNs * ENTRY_SIZE_STSZ;


		else if (STCO == i) {
			if (prMp4FromTbl[i].fgIsCo64)
				u4EntrySize = prMp4FromTbl[i].u4EntryNs * ENTRY_SIZE_CO64;


			else if (!prMp4FromTbl[i].fgIsCo64)
				u4EntrySize = prMp4FromTbl[i].u4EntryNs * ENTRY_SIZE_STCO;

		}

		else if (STSD == i) {
			u4EntrySize = prMp4FromTbl[i].u4EntryNs * ENTRY_SIZE_STSD;
		}
		if (u4EntrySize && prMp4FromTbl[i].pvEntry) {
			DMX_NewMemory(u4EntrySize, (*prMp4ToTbl)[i].pvEntry);
			if ((*prMp4ToTbl)[i].pvEntry) {
				dmx_memcpy((*prMp4ToTbl)[i].pvEntry, prMp4FromTbl[i].pvEntry,
						u4EntrySize);
			}

			else {
				MM_RETURN(RET_DMX_NO_MEM);
			}
		}
	}
	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: vCfaMp4FreeMem
*
* Description:
*	  Internal Free Mem
*
* Inputs:
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
void CfaMp4InternalFreeMem(CfaMp4Inst *prCfaMp4)
{
	u32 i = 0, j = 0;

		/* Free Video Information memory. */
	if (prCfaMp4->pucAvcHdr) {
		DMX_FreeHwMemory(prCfaMp4->pucAvcHdr);
		prCfaMp4->pucAvcHdr = NULL;
	}
	if (prCfaMp4->rCfaMp4VInf.pucMpeg4CodecSC) {
		DMX_FreeHwMemory(prCfaMp4->rCfaMp4VInf.pucMpeg4CodecSC);
		prCfaMp4->rCfaMp4VInf.pucMpeg4CodecSC = NULL;
	}
	if (prCfaMp4->rCfaMp4VInf.pucWVc1CodecSC) {
		DMX_FreeHwMemory(prCfaMp4->rCfaMp4VInf.pucWVc1CodecSC);
		prCfaMp4->rCfaMp4VInf.pucWVc1CodecSC = NULL;
	}
	if (prCfaMp4->rCfaMp4VInf.puCodecSC) {
		DMX_FreeHwMemory(prCfaMp4->rCfaMp4VInf.puCodecSC);
		prCfaMp4->rCfaMp4VInf.puCodecSC = NULL;
	}
	for (i = 0; i < MP4_STSD_TABLE_MAX_NUMS; i++) {
		if (prCfaMp4->rCfaMp4VInf.pucDecSpecInfo[i]) {
			DMX_FreeHwMemory(prCfaMp4->rCfaMp4VInf.pucDecSpecInfo[i]);
			prCfaMp4->rCfaMp4VInf.pucDecSpecInfo[i] = NULL;
		}
		if (prCfaMp4->rCfaMp4VInf.pucPicParamInfo[i]) {
			DMX_FreeMemory(prCfaMp4->rCfaMp4VInf.pucPicParamInfo[i]);
			prCfaMp4->rCfaMp4VInf.pucPicParamInfo[i] = NULL;
		}
		if (prCfaMp4->rCfaMp4VInf.pucSeqParamInfo[i]) {
			DMX_FreeMemory(prCfaMp4->rCfaMp4VInf.pucSeqParamInfo[i]);
			prCfaMp4->rCfaMp4VInf.pucSeqParamInfo[i] = NULL;
		}
		if (prCfaMp4->rCfaMp4VInf.pucVPSInfo[i]) {
			DMX_FreeMemory(prCfaMp4->rCfaMp4VInf.pucVPSInfo[i]);
			prCfaMp4->rCfaMp4VInf.pucVPSInfo[i] = NULL;
		}
	}
	if (prCfaMp4->rCfaMp4VInf.pucPicParamAdress) {
		DMX_FreeMemory(prCfaMp4->rCfaMp4VInf.pucPicParamAdress);
		prCfaMp4->rCfaMp4VInf.pucPicParamAdress = NULL;
	}
	if (prCfaMp4->rCfaMp4VInf.pucSeqParamAdress) {
		DMX_FreeMemory(prCfaMp4->rCfaMp4VInf.pucSeqParamAdress);
		prCfaMp4->rCfaMp4VInf.pucSeqParamAdress = NULL;
	}
	if (prCfaMp4->rCfaMp4VInf.pTChunkInfo) {
		DMX_FreeMemory(prCfaMp4->rCfaMp4VInf.pTChunkInfo);
		prCfaMp4->rCfaMp4VInf.pTChunkInfo = NULL;
	}
	if (prCfaMp4->rCfaMp4VInf.pTSampleInfo) {
		DMX_FreeMemory(prCfaMp4->rCfaMp4VInf.pTSampleInfo);
		prCfaMp4->rCfaMp4VInf.pTSampleInfo = NULL;
	}
	if (!prCfaMp4->fgFinished) {
		if (prCfaMp4->rCfaMp4VInf.prVTable) {
			for (i = 0; i < MAX_MP4_SMP_TBL; i++) {
				if (prCfaMp4->rCfaMp4VInf.prVTable[i].pvEntry) {
					DMX_FreeMemory(prCfaMp4->rCfaMp4VInf.prVTable[i].pvEntry);
					prCfaMp4->rCfaMp4VInf.prVTable[i].pvEntry = NULL;
				}
			}
			DMX_FreeMemory(prCfaMp4->rCfaMp4VInf.prVTable);
			prCfaMp4->rCfaMp4VInf.prVTable = NULL;
		}
#if MP4_SUPPORT_FRAGMENT
		if (NULL != prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.pvTrunBuf) {
			DMX_FreeMemory(prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.pvTrunBuf);
			prCfaMp4->rCfaMp4VInf.TCfaMoofInfo.pvTrunBuf = NULL;
		}
#endif
	}

		/* Free audio Information memory. */
	CFA_MP4_LIMIT_A_NUM(prCfaMp4->u2AudStrmNums);
	for (i = 0; i < prCfaMp4->u2AudStrmNums; i++) {
		if (prCfaMp4->rCfaMp4AInf[i].pucDecSpecInfo) {
			DMX_FreeHwMemory(prCfaMp4->rCfaMp4AInf[i].pucDecSpecInfo);
			prCfaMp4->rCfaMp4AInf[i].pucDecSpecInfo = NULL;
		}
		if (prCfaMp4->rCfaMp4AInf[i].pucADTSBuf) {
			DMX_FreeHwMemory(prCfaMp4->rCfaMp4AInf[i].pucADTSBuf);
			prCfaMp4->rCfaMp4AInf[i].pucADTSBuf = NULL;
		}
		if (prCfaMp4->rCfaMp4AInf[i].pTChunkInfo) {
			DMX_FreeMemory(prCfaMp4->rCfaMp4AInf[i].pTChunkInfo);
			prCfaMp4->rCfaMp4AInf[i].pTChunkInfo = NULL;
		}
		if (prCfaMp4->rCfaMp4AInf[i].pTSampleInfo) {
			DMX_FreeMemory(prCfaMp4->rCfaMp4AInf[i].pTSampleInfo);
			prCfaMp4->rCfaMp4AInf[i].pTSampleInfo = NULL;
		}
		if (!prCfaMp4->fgFinished) {
			if (prCfaMp4->rCfaMp4AInf[i].prATable) {
				for (j = 0; j < MAX_MP4_SMP_TBL; j++) {
					if (prCfaMp4->rCfaMp4AInf[i].prATable[j].pvEntry) {
						DMX_FreeMemory(prCfaMp4->rCfaMp4AInf[i].
								prATable[j].pvEntry);
						prCfaMp4->rCfaMp4AInf[i].prATable[j].pvEntry =
							NULL;
					}
				}
				DMX_FreeMemory(prCfaMp4->rCfaMp4AInf[i].prATable);
				prCfaMp4->rCfaMp4AInf[i].prATable = NULL;
			}
#if MP4_SUPPORT_FRAGMENT
			if (NULL != prCfaMp4->rCfaMp4AInf[i].TCfaMoofInfo.pvTrunBuf) {
				DMX_FreeMemory(prCfaMp4->rCfaMp4AInf[i].TCfaMoofInfo.pvTrunBuf);
				prCfaMp4->rCfaMp4AInf[i].TCfaMoofInfo.pvTrunBuf = NULL;
			}
#endif
		}
	}

	/* Free Audio Information memory. */
	CFA_MP4_LIMIT_S_NUM(prCfaMp4->u2SubStrmNums);
	for (i = 0; i < prCfaMp4->u2SubStrmNums; i++) {
		if (prCfaMp4->rCfaMp4SInf[i].pTChunkInfo) {
			DMX_FreeMemory(prCfaMp4->rCfaMp4SInf[i].pTChunkInfo);
			prCfaMp4->rCfaMp4SInf[i].pTChunkInfo = NULL;
		}
		if (prCfaMp4->rCfaMp4SInf[i].pTSampleInfo) {
			DMX_FreeMemory(prCfaMp4->rCfaMp4SInf[i].pTSampleInfo);
			prCfaMp4->rCfaMp4SInf[i].pTSampleInfo = NULL;
		}
		if (prCfaMp4->rCfaMp4SInf[i].prSTable) {
			for (j = 0; j < MAX_MP4_SMP_TBL; j++) {
				if (prCfaMp4->rCfaMp4SInf[i].prSTable[j].pvEntry) {
					DMX_FreeMemory(prCfaMp4->rCfaMp4SInf[i].prSTable[j].
							pvEntry);
					prCfaMp4->rCfaMp4SInf[i].prSTable[j].pvEntry = NULL;
				}
			}
			DMX_FreeMemory(prCfaMp4->rCfaMp4SInf[i].prSTable);
			prCfaMp4->rCfaMp4SInf[i].prSTable = NULL;
		}
		if (prCfaMp4->rCfaMp4SInf[i].prMFSTable) {
			for (j = 0; j < MAX_MP4_SMP_TBL; j++) {
				if (prCfaMp4->rCfaMp4SInf[i].prMFSTable[j].pvEntry) {
					DMX_FreeMemory(prCfaMp4->rCfaMp4SInf[i].prMFSTable[j].
							pvEntry);
					prCfaMp4->rCfaMp4SInf[i].prMFSTable[j].pvEntry = NULL;
				}
			}
			DMX_FreeMemory(prCfaMp4->rCfaMp4SInf[i].prMFSTable);
			prCfaMp4->rCfaMp4SInf[i].prMFSTable = NULL;
		}
		if (prCfaMp4->rCfaMp4SInf[i].prMVSTable) {
			for (j = 0; j < MAX_MP4_SMP_TBL; j++) {
				if (prCfaMp4->rCfaMp4SInf[i].prMVSTable[j].pvEntry) {
					DMX_FreeMemory(prCfaMp4->rCfaMp4SInf[i].prMVSTable[j].
							pvEntry);
					prCfaMp4->rCfaMp4SInf[i].prMVSTable[j].pvEntry = NULL;
				}
			}
			DMX_FreeMemory(prCfaMp4->rCfaMp4SInf[i].prMVSTable);
			prCfaMp4->rCfaMp4SInf[i].prMVSTable = NULL;
		}
	}
}

MRESULT CfaMp4PlayNextMoof(CfaMp4Inst *prCfaMp4)
{
	u32 i = 0;
	u32 u4BufNums = 0;

	CfaMp4FreeSampleAndChunk(prCfaMp4);
	prCfaMp4->u4CurPrsFlg = CFA_MP4_PRS_BIT_STRM_TYPE_NONE;
	for (i = 0; i < prCfaMp4->u2SubStrmNums; i++) {
		if ((prCfaMp4->rCfaMp4SInf[i].prMFSTable)
			 && (CfaMp4MoofTableValid(prCfaMp4->rCfaMp4SInf[i].prMFSTable))) {
			prCfaMp4->rCfaMp4SInf[i].prSTable = prCfaMp4->rCfaMp4SInf[i].prMFSTable;
			prCfaMp4->rCfaMp4SInf[i].u4SampleSize = 0;
			prCfaMp4->rCfaMp4SInf[i].fgCO64Valid = TRUE;
			u4BufNums = prCfaMp4->rCfaMp4SInf[i].prSTable[STSZ].u4Allotted + 20;
			prCfaMp4->rCurTblPos.rTblSubPos[i].u4SampleBufNums = u4BufNums;
			DMX_NewMemory(sizeof(TSampleInfo) * u4BufNums,
				prCfaMp4->rCfaMp4SInf[i].pTSampleInfo);
			if (NULL == prCfaMp4->rCfaMp4SInf[i].pTSampleInfo)
				MM_RETURN(RET_DMX_NO_MEM);

			u4BufNums = prCfaMp4->rCfaMp4SInf[i].prSTable[STCO].u4Allotted + 20;
			prCfaMp4->rCurTblPos.rTblSubPos[i].u4ChunkBufNums = u4BufNums;
			DMX_NewMemory(sizeof(TChunkInfo) * u4BufNums,
				prCfaMp4->rCfaMp4SInf[i].pTChunkInfo);
			if (NULL == prCfaMp4->rCfaMp4AInf[i].pTChunkInfo)
				MM_RETURN(RET_DMX_NO_MEM);

			prCfaMp4->u4CurPrsFlg |= CFA_MP4_PRS_BIT_STRM_TYPE_SP;
			if (!prCfaMp4->rCfaRange.fgFirstRange)
				prCfaMp4->rCfaRange.u8SubPts[i] = prCfaMp4->u8CurSPts[i];

		}
	}

	if (CFA_MP4_PRS_BIT_STRM_TYPE_NONE != prCfaMp4->u4CurPrsFlg) {
		/* have stream*/
		MM_RETURN(RET_DMX_OK);
	} else {
		/* No Stream*/
		MM_RETURN(RET_DMX_UNEXPECT);
	}

}

/*
Parse Range info from preparser to get the pts, duration, chunkNO, sampleNO information
*/
void CfaMp4ParseRange(CfaMp4Inst *prCfaMp4)
{
	u32 i = 0;
	CfaMp4Range *prCfaRange = &prCfaMp4->rCfaRange;

	/*video*/
	if ((0 == prCfaRange->u8VidStartChunkNo) ||
		 (0 == prCfaRange->u8VidEndChunkNo) ||
		 (0 == prCfaRange->u8VidStartSampleNo) || (0 == prCfaRange->u8VidEndSampleNo)) {
			prCfaMp4->u4CurPrsFlg &= ~((u32) CFA_MP4_PRS_BIT_STRM_TYPE_V);
	}
	prCfaMp4->u8CurVPts = prCfaRange->u8VidPts;
	prCfaMp4->u8CurVDuation =
		ConvertFromStc(prCfaRange->u8VidPts, prCfaMp4->rCfaMp4VInf.u4TimeScale);
	prCfaMp4->rCfaMp4VInf.u8CurPrsVidChunkNo = prCfaRange->u8VidStartChunkNo;
	prCfaMp4->rCfaMp4VInf.u8CurPrsVidSampleNo = prCfaRange->u8VidStartSampleNo;
	prCfaMp4->rCfaMp4VInf.u4NeedPrsSampleNums = 0;

	/* audio*/
	CFA_MP4_LIMIT_A_NUM(prCfaMp4->u2AudStrmNums);
	for (i = 0; i < prCfaMp4->u2AudStrmNums; i++) {
		prCfaMp4->u8CurAPts[i] = prCfaRange->u8AudPts[i];
		prCfaMp4->u8CurADuation[i] =
			ConvertFromStc(prCfaRange->u8AudPts[i], prCfaMp4->rCfaMp4AInf[i].u4TimeScale);
		prCfaMp4->rCfaMp4AInf[i].u8CurPrsAudChunkNo = prCfaRange->u8AudStartChunkNo[i];
		prCfaMp4->rCfaMp4AInf[i].u8CurPrsAudSampleNo = prCfaRange->u8AudStartSampleNo[i];
		prCfaMp4->rCfaMp4AInf[i].u4NeedPrsSampleNums = 0;
	}
	DmxLogD(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
			   TEXT("u8StartPts:0x%llx, u8SeekPts:0x%llx, u8CurVPts:0x%llx,")
			   TEXT("u8CurVidSampleNo:0x%llx, u8CurAPts:0x%llx, u8CurAudSampleNo:0x%llx \r\n"),
			   prCfaRange->u8StartPts, prCfaRange->u8SeekPts, prCfaMp4->u8CurVPts,
			   prCfaMp4->rCfaMp4VInf.u8CurPrsVidSampleNo, prCfaMp4->u8CurAPts[0],
			   prCfaMp4->rCfaMp4AInf[0].u8CurPrsAudSampleNo);

	/*subpic*/
	CFA_MP4_LIMIT_S_NUM(prCfaMp4->u2SubStrmNums);
	for (i = 0; i < prCfaMp4->u2SubStrmNums; i++) {
		prCfaMp4->u8CurSPts[i] = prCfaRange->u8SubPts[i];
		prCfaMp4->u8CurSDuation[i] =
			ConvertFromStc(prCfaRange->u8SubPts[i], prCfaMp4->rCfaMp4SInf[i].u4TimeScale);
		prCfaMp4->rCfaMp4SInf[i].u8CurPrsSubChunkNo = prCfaRange->u8SubStartChunkNo[i];
		prCfaMp4->rCfaMp4SInf[i].u8CurPrsSubSampleNo = prCfaRange->u8SubStartSampleNo[i];
		prCfaMp4->rCfaMp4SInf[i].u4NeedPrsSampleNums = 0;
	}
}


/*-----------------------------------------------------------------------------
* Name: mrCfaMp4ParseTable
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
MRESULT mrCfaMp4ParseTable(CfaMp4Inst *prCfaMp4)
{
	u32 i = 0;
	TCfaMp4VInf *prVInfo = &prCfaMp4->rCfaMp4VInf;
	CfaMp4CurTablePos *prCurTbl = &prCfaMp4->rCurTblPos;

	if (NULL != prVInfo->prVTable) {
		prCurTbl->rTblVidPos.u4TabelUb[STCO] = prVInfo->prVTable[STCO].u4EntryLb;
		prCurTbl->rTblVidPos.u4TabelUb[STSC] = prVInfo->prVTable[STSC].u4EntryLb;
		prCurTbl->rTblVidPos.u4TabelUb[STTS] = prVInfo->prVTable[STTS].u4EntryLb;
		prCurTbl->rTblVidPos.u4TabelUb[STSZ] = prVInfo->prVTable[STSZ].u4EntryLb;
	}
	if ((NULL != prVInfo->prVTable) &&
		   (0 != prVInfo->prVTable->u4EntryNs) &&
		   (0 != prVInfo->u8CurPrsVidChunkNo) &&
		   (0 != prVInfo->u8CurPrsVidSampleNo) &&
		   (DMX_INVALID_UINT64 != prVInfo->u8CurPrsVidSampleNo) &&
		   (DMX_INVALID_UINT64 != prVInfo->u8CurPrsVidChunkNo) &&
		   (prCfaMp4->rCfaRange.u4PrsFlag & CFA_MP4_PRS_BIT_STRM_TYPE_V)) {
		if (prVInfo->prVTable[STCO].u4EntryLb != prVInfo->u8CurPrsVidChunkNo)
			MM_RETURN(RET_DMX_PARAM_WRONG);

		prVInfo->u8CurVidTableStartChunkNo = prCfaMp4->rCfaRange.u8VidStartChunkNo;
		prVInfo->u8CurVidTableStartSampleNo = prCfaMp4->rCfaRange.u8VidStartSampleNo;
		prCurTbl->rTblVidPos.u8CurSampleNo = prCfaMp4->rCfaRange.u8VidStartSampleNo;
		prCurTbl->rTblVidPos.u4CurTableLastChunkNo =
			(u32) prCfaMp4->rCfaRange.u8VidStartChunkNo;
		if (1 == prCfaMp4->rCfaRange.u8VidStartSampleNo)
			prCfaMp4->rCfaRange.u8VidStartChunk1stSmp = 1;

		prCurTbl->rTblVidPos.u4SttsInvaildSampleNums =
			CfaMp4MatchStts(prVInfo->prVTable, prCfaMp4->rCfaRange.u8VidStartSampleNo);
		prCurTbl->rTblVidPos.u4TabelUb[STTS] = prVInfo->prVTable[STTS].u4EntryLb;
		prCurTbl->rTblVidPos.u4PrsDoneSampleNumsEveryChunk = (u32)
			(prCfaMp4->rCfaRange.u8VidStartSampleNo -
			 prCfaMp4->rCfaRange.u8VidStartChunk1stSmp);
		prCfaMp4->rCfaMp4VInf.u8CurVidTableEndSampleNo =
			CfaMp4CreateNewTable2GetEndSmpNo(prVInfo->prVTable, prVInfo->u4SampleSize,
							prVInfo->pTSampleInfo,
							prVInfo->u8CurPrsVidSampleNo,
							prVInfo->pTChunkInfo,
							prVInfo->u8CurPrsVidChunkNo,
							&prCfaMp4->rCurTblPos.rTblVidPos,
							prVInfo->fgCO64Valid
#if SPECIAL_LPCM_SUPPORT
							  , TRUE , 0
#endif
							  , 0);
		prVInfo->fgGetRangeOfst = TRUE;
		prCfaMp4->u4CurTablePrsDoneFlag |= CFA_MP4_CUR_TABLE_PRS_TYPE_V;
	}

	else {
		prCfaMp4->u4CurPrsFlg &= ~(CFA_MP4_PRS_BIT_STRM_TYPE_V);
	}
	if (0 == prCfaMp4->u2AudStrmNums)
		prCfaMp4->u4CurPrsFlg &= ~(CFA_MP4_PRS_BIT_STRM_TYPE_A);

	CFA_MP4_LIMIT_A_NUM(prCfaMp4->u2AudStrmNums);
	CFA_MP4_LIMIT_S_NUM(prCfaMp4->u2SubStrmNums);
	for (i = 0; i < prCfaMp4->u2AudStrmNums; i++) {
		if (prCfaMp4->rCfaMp4AInf[i].prATable) {
			if (AVCODEC_ID_AAC == prCfaMp4->rCfaMp4AInf[i].eAudType) {
				if (prCfaMp4->rCfaMp4AInf[i].pucDecSpecInfo
					 && prCfaMp4->rCfaMp4AInf[i].u4DecSpecSz) {
					prCfaMp4->rCfaMp4AInf[i].fgTxAacSCDone = FALSE;
				}
				prCfaMp4->rCfaMp4AInf[i].fgAddAdtsDone = FALSE;
				prCfaMp4->rCfaMp4AInf[i].fgSyncIV = FALSE;
			}

			else {
				prCfaMp4->rCfaMp4AInf[i].fgAddAdtsDone = TRUE;
			}
			prCurTbl->rTblAudPos[i].u4TabelUb[STCO] =
				prCfaMp4->rCfaMp4AInf[i].prATable[STCO].u4EntryLb;
			prCurTbl->rTblAudPos[i].u4TabelUb[STSC] =
				prCfaMp4->rCfaMp4AInf[i].prATable[STSC].u4EntryLb;
			prCurTbl->rTblAudPos[i].u4TabelUb[STTS] =
				prCfaMp4->rCfaMp4AInf[i].prATable[STTS].u4EntryLb;
			prCurTbl->rTblAudPos[i].u4TabelUb[STSZ] =
				prCfaMp4->rCfaMp4AInf[i].prATable[STSZ].u4EntryLb;
		}
		if (prCfaMp4->rCfaMp4AInf[i].prATable &&
			   (prCfaMp4->rCfaMp4AInf[i].prATable)->u4EntryNs
			   && prCfaMp4->rCfaMp4AInf[i].u8CurPrsAudSampleNo
			   && (DMX_INVALID_UINT64 != prCfaMp4->rCfaMp4AInf[i].u8CurPrsAudSampleNo)
			   && prCfaMp4->rCfaMp4AInf[i].u8CurPrsAudChunkNo
			   && (DMX_INVALID_UINT64 != prCfaMp4->rCfaMp4AInf[i].u8CurPrsAudChunkNo)
			   && (prCfaMp4->rCfaRange.u4PrsFlag & CFA_MP4_PRS_BIT_STRM_TYPE_A)) {
			if (prCfaMp4->rCfaMp4AInf[i].prATable[STCO].u4EntryLb !=
				 prCfaMp4->rCfaMp4AInf[i].u8CurPrsAudChunkNo) {
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			prCfaMp4->rCfaMp4AInf[i].u8CurAudTableStartSampleNo =
				prCfaMp4->rCfaRange.u8AudStartSampleNo[i];
			prCurTbl->rTblAudPos[i].u8CurSampleNo =
				prCfaMp4->rCfaRange.u8AudStartSampleNo[i];
			prCfaMp4->rCfaMp4AInf[i].u8CurAudTableStartChunkNo =
				prCfaMp4->rCfaRange.u8AudStartChunkNo[i];
			prCurTbl->rTblAudPos[i].u4CurTableLastChunkNo =
				(u32) prCfaMp4->rCfaRange.u8AudStartChunkNo[i];
			if (1 == prCfaMp4->rCfaRange.u8AudStartSampleNo[i])
				prCfaMp4->rCfaRange.u8AudStartChunk1stSmp[i] = 1;

			prCurTbl->rTblAudPos[i].u4SttsInvaildSampleNums =
				CfaMp4MatchStts(prCfaMp4->rCfaMp4AInf[i].prATable,
						 prCfaMp4->rCfaRange.u8AudStartSampleNo[i]);
			prCurTbl->rTblAudPos[i].u4PrsDoneSampleNumsEveryChunk =
				(u32) (prCfaMp4->rCfaRange.u8AudStartSampleNo[i] -
					prCfaMp4->rCfaRange.u8AudStartChunk1stSmp[i]);
			if (AVCODEC_ID_PCM != prCfaMp4->rCfaMp4AInf[i].eAudType) {
				prCfaMp4->rCfaMp4AInf[i].u8CurAudTableEndSampleNo =
					CfaMp4CreateNewTable2GetEndSmpNo(
								prCfaMp4->rCfaMp4AInf[i].prATable,
								prCfaMp4->rCfaMp4AInf[i].u4SampleSize,
								prCfaMp4->rCfaMp4AInf[i].pTSampleInfo,
								prCfaMp4->rCfaMp4AInf[i].u8CurPrsAudSampleNo,
								prCfaMp4->rCfaMp4AInf[i].pTChunkInfo,
								prCfaMp4->rCfaMp4AInf[i].u8CurPrsAudChunkNo,
								&prCfaMp4->rCurTblPos.rTblAudPos[i],
								prCfaMp4->rCfaMp4AInf[i].fgCO64Valid
#if SPECIAL_LPCM_SUPPORT
							  , TRUE , 0
#endif
							  , prCfaMp4->rCfaRange.u8MoovSkipSize);
			} else {
				prCfaMp4->rCfaMp4AInf[i].u8CurAudTableEndSampleNo =
					CfaMp4CreateNewTable2GetEndSmpNo(prCfaMp4->rCfaMp4AInf[i].prATable,
						prCfaMp4->rCfaMp4AInf[i].u4SampleSize,
						prCfaMp4->rCfaMp4AInf[i].pTSampleInfo,
						prCfaMp4->rCfaMp4AInf[i].u8CurPrsAudSampleNo,
						prCfaMp4->rCfaMp4AInf[i].pTChunkInfo,
						prCfaMp4->rCfaMp4AInf[i].u8CurPrsAudChunkNo,
						&prCfaMp4->rCurTblPos.rTblAudPos[i],
						prCfaMp4->rCfaMp4AInf[i].fgCO64Valid
#if SPECIAL_LPCM_SUPPORT
						, FALSE ,
						prCurTbl->rTblAudPos[i].u4PrsDoneSampleNumsEveryChunk

#endif
						, prCfaMp4->rCfaRange.u8MoovSkipSize);
			}
			prCfaMp4->rCfaMp4AInf[i].fgGetRangeOfst = TRUE;
			prCfaMp4->u4CurTablePrsDoneFlag |= CFA_MP4_CUR_TABLE_PRS_TYPE_A;
		}
	}
	if (0 == prCfaMp4->u2SubStrmNums)
		prCfaMp4->u4CurPrsFlg &= ~((u32) CFA_MP4_PRS_BIT_STRM_TYPE_SP);

	for (i = 0; i < prCfaMp4->u2SubStrmNums; i++) {
		if (prCfaMp4->rCfaMp4SInf[i].prSTable) {
			prCurTbl->rTblSubPos[i].u4TabelUb[STCO] =
				prCfaMp4->rCfaMp4SInf[i].prSTable[STCO].u4EntryLb;
			prCurTbl->rTblSubPos[i].u4TabelUb[STSC] =
				prCfaMp4->rCfaMp4SInf[i].prSTable[STSC].u4EntryLb;
			prCurTbl->rTblSubPos[i].u4TabelUb[STTS] =
				prCfaMp4->rCfaMp4SInf[i].prSTable[STTS].u4EntryLb;
			prCurTbl->rTblSubPos[i].u4TabelUb[STSZ] =
				prCfaMp4->rCfaMp4SInf[i].prSTable[STSZ].u4EntryLb;
		}
		if (prCfaMp4->rCfaMp4SInf[i].prSTable &&
			   (prCfaMp4->rCfaMp4SInf[i].prSTable)->u4EntryNs
			   && prCfaMp4->rCfaMp4SInf[i].u8CurPrsSubSampleNo
			   && (DMX_INVALID_UINT64 != prCfaMp4->rCfaMp4SInf[i].u8CurPrsSubSampleNo)
			   && prCfaMp4->rCfaMp4SInf[i].u8CurPrsSubChunkNo
			   && (DMX_INVALID_UINT64 != prCfaMp4->rCfaMp4SInf[i].u8CurPrsSubChunkNo)
			   && (prCfaMp4->rCfaRange.u4PrsFlag & CFA_MP4_PRS_BIT_STRM_TYPE_SP)) {
			if (prCfaMp4->rCfaMp4SInf[i].prSTable[STCO].u4EntryLb !=
				 prCfaMp4->rCfaMp4SInf[i].u8CurPrsSubChunkNo) {
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			prCfaMp4->rCfaMp4SInf[i].u8CurSubTableStartSampleNo =
				prCfaMp4->rCfaRange.u8SubStartSampleNo[i];
			prCurTbl->rTblSubPos[i].u8CurSampleNo =
				prCfaMp4->rCfaRange.u8SubStartSampleNo[i];
			prCfaMp4->rCfaMp4SInf[i].u8CurSubTableStartChunkNo =
				prCfaMp4->rCfaRange.u8SubStartChunkNo[i];
			prCurTbl->rTblSubPos[i].u4CurTableLastChunkNo =
				(u32) prCfaMp4->rCfaRange.u8SubStartChunkNo[i];
			if (1 == prCfaMp4->rCfaRange.u8SubStartSampleNo[i])
				prCfaMp4->rCfaRange.u8SubStartChunk1stSmp[i] = 1;

			prCurTbl->rTblSubPos[i].u4SttsInvaildSampleNums =
				CfaMp4MatchStts(prCfaMp4->rCfaMp4SInf[i].prSTable,
						 prCfaMp4->rCfaRange.u8SubStartSampleNo[i]);
			prCurTbl->rTblSubPos[i].u4PrsDoneSampleNumsEveryChunk =
				(u32) (prCfaMp4->rCfaRange.u8SubStartSampleNo[i] -
					prCfaMp4->rCfaRange.u8SubStartChunk1stSmp[i]);
			prCfaMp4->rCfaMp4SInf[i].u8CurSubTableEndSampleNo =
				CfaMp4CreateNewTable2GetEndSmpNo(prCfaMp4->rCfaMp4SInf[i].prSTable,
				prCfaMp4->rCfaMp4SInf[i].u4SampleSize, prCfaMp4->rCfaMp4SInf[i].pTSampleInfo,
				prCfaMp4->rCfaMp4SInf[i].u8CurPrsSubSampleNo, prCfaMp4->rCfaMp4SInf[i].pTChunkInfo,
				prCfaMp4->rCfaMp4SInf[i].u8CurPrsSubChunkNo, &prCfaMp4->rCurTblPos.rTblSubPos[i],
				prCfaMp4->rCfaMp4SInf[i].fgCO64Valid
#if SPECIAL_LPCM_SUPPORT
								  , TRUE , 0
#endif
								  , 0);
			prCfaMp4->rCfaMp4SInf[i].fgGetRangeOfst = TRUE;
			prCfaMp4->u4CurTablePrsDoneFlag |= CFA_MP4_CUR_TABLE_PRS_TYPE_S;
		}
	}
	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
* Name: fgBadInterleaved
*
* Description:
*	   MP4 CFA get table is fg bad interleaved

*
* Inputs:
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
bool BadInterleaved(void *pvSptHdl, Mp4BadIntlvdCheck *prMp4BadIntlvd)
{
	CfaMp4Inst *prMp4Inst = NULL;
	u32 i = 0;
	u32 u4BufNums = 0;
	bool fgBadIntlvd = FALSE;

	if ((NULL == prMp4BadIntlvd->prVTable) || (0 == prMp4BadIntlvd->u2AudStrmNum))
		return FALSE;

	DMX_NewMemory(sizeof(CfaMp4Inst), prMp4Inst);
	if (NULL == prMp4Inst) {
		DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4] Alloc prMp4Inst Memory Failed! \r\n"));
		return FALSE;
	}
	if ((NULL != prMp4BadIntlvd->prVTable) && (0 != (prMp4BadIntlvd->prVTable)->u4EntryNs)) {
		prMp4Inst->rCfaMp4VInf.prVTable = prMp4BadIntlvd->prVTable;
		if (0 != prMp4BadIntlvd->u4VSampSz)
			u4BufNums = prMp4BadIntlvd->prVTable[STTS].u4Allotted + 20;


		else
			u4BufNums = prMp4BadIntlvd->prVTable[STSZ].u4Allotted + 20;

		prMp4Inst->rCurTblPos.rTblVidPos.u4SampleBufNums = u4BufNums;
		DMX_NewMemory(sizeof(TSampleInfo) * u4BufNums,
				   prMp4Inst->rCfaMp4VInf.pTSampleInfo);
		if (NULL == prMp4Inst->rCfaMp4VInf.pTSampleInfo) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4] Alloc Video pTSampleInfo Memory Failed! \r\n"));
			CfaMp4InternalFreeMem(prMp4Inst);
			DMX_FreeMemory(prMp4Inst);
			return FALSE;
		}
		u4BufNums = prMp4BadIntlvd->prVTable[STCO].u4Allotted + 20;
		prMp4Inst->rCurTblPos.rTblVidPos.u4ChunkBufNums = u4BufNums;
		DMX_NewMemory(sizeof(TChunkInfo) * u4BufNums, prMp4Inst->rCfaMp4VInf.pTChunkInfo);
		if (NULL == prMp4Inst->rCfaMp4VInf.pTChunkInfo) {
			DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
				TEXT("[CFA MP4] Alloc Video pTChunkInfo Memory Failed! \r\n"));
			CfaMp4InternalFreeMem(prMp4Inst);
			DMX_FreeMemory(prMp4Inst);
			return FALSE;
		}
		prMp4Inst->rCfaMp4VInf.fgCO64Valid = prMp4BadIntlvd->fgVCO64Valid;
		prMp4Inst->rCfaMp4VInf.u4TimeScale = prMp4BadIntlvd->u4VTimeScale;
		prMp4Inst->rCfaMp4VInf.u4SampleSize = prMp4BadIntlvd->u4VSampSz;
		prMp4Inst->u4CurPrsFlg |= CFA_MP4_PRS_BIT_STRM_TYPE_V;
	}
	CFA_MP4_LIMIT_A_NUM(prMp4BadIntlvd->u2AudStrmNum);
	prMp4Inst->u2AudStrmNums = prMp4BadIntlvd->u2AudStrmNum;
	for (i = 0; i < prMp4BadIntlvd->u2AudStrmNum; i++) {
		if ((NULL != prMp4BadIntlvd->prATable[i])
			 && ((prMp4BadIntlvd->prATable[i])->u4EntryNs != 0)) {
			prMp4Inst->rCfaMp4AInf[i].prATable = prMp4BadIntlvd->prATable[i];
			if (0 != prMp4BadIntlvd->u4ASampSz[i])
				u4BufNums = (prMp4BadIntlvd->prATable[i] + STTS)->u4Allotted + 20;

			else
				u4BufNums = (prMp4BadIntlvd->prATable[i] + STSZ)->u4Allotted + 20;
			prMp4Inst->rCurTblPos.rTblAudPos[i].u4SampleBufNums = u4BufNums;
			DMX_NewMemory(sizeof(TSampleInfo) * u4BufNums,
					   prMp4Inst->rCfaMp4AInf[i].pTSampleInfo);
			if (NULL == prMp4Inst->rCfaMp4AInf[i].pTSampleInfo) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4] Alloc Audio pTSampleInfo Memory Failed! \r\n"));
				CfaMp4InternalFreeMem(prMp4Inst);
				DMX_FreeMemory(prMp4Inst);
				return FALSE;
			}
			u4BufNums = (prMp4BadIntlvd->prATable[i] + STCO)->u4Allotted + 20;
			prMp4Inst->rCurTblPos.rTblAudPos[i].u4ChunkBufNums = u4BufNums;
			DMX_NewMemory(sizeof(TChunkInfo) * u4BufNums,
					   prMp4Inst->rCfaMp4AInf[i].pTChunkInfo);
			if (NULL == prMp4Inst->rCfaMp4AInf[i].pTChunkInfo) {
				DmxLogE(DMX_MOD_CFA_MP4, CFA_MP4_LOG_DEFAULT,
					TEXT("[CFA MP4] Alloc Audio pTChunkInfo Memory Failed! \r\n"));
				CfaMp4InternalFreeMem(prMp4Inst);
				DMX_FreeMemory(prMp4Inst);
				return FALSE;
			}
			prMp4Inst->rCfaMp4AInf[i].fgCO64Valid = prMp4BadIntlvd->fgACO64Valid[i];
			prMp4Inst->rCfaMp4AInf[i].u4TimeScale = prMp4BadIntlvd->u4ATimeScale[i];
			prMp4Inst->rCfaMp4AInf[i].u4SampleSize = prMp4BadIntlvd->u4ASampSz[i];
		}
		prMp4Inst->u4CurPrsFlg |= CFA_MP4_PRS_BIT_STRM_TYPE_A;
	}
	prMp4Inst->rCfaRange.u4PrsFlag = CFA_MP4_PRS_BIT_STRM_TYPE_V | CFA_MP4_PRS_BIT_STRM_TYPE_A;
	prMp4Inst->rCfaMp4VInf.u8CurPrsVidChunkNo = 1;
	prMp4Inst->rCfaMp4VInf.u8CurPrsVidSampleNo = 1;
	prMp4Inst->rCurTblPos.rTblVidPos.u4CurTableLastChunkNo = 1;
	prMp4Inst->rCurTblPos.rTblVidPos.u8CurSampleNo = 1;
	prMp4Inst->rCfaRange.u8VidStartChunk1stSmp = 1;
	prMp4Inst->rCfaRange.u8VidStartChunkNo = 1;
	prMp4Inst->rCfaRange.u8VidStartSampleNo = 1;
	prMp4Inst->rCfaRange.u8VidEndChunkNo = DMX_INVALID_UINT64 - 1;
	prMp4Inst->rCfaRange.u8VidEndSampleNo = DMX_INVALID_UINT64 - 1;
	for (i = 0; i < prMp4BadIntlvd->u2AudStrmNum; i++) {
		prMp4Inst->rCfaMp4AInf[i].u8CurPrsAudChunkNo = 1;
		prMp4Inst->rCfaMp4AInf[i].u8CurPrsAudSampleNo = 1;
		prMp4Inst->rCurTblPos.rTblAudPos[i].u4CurTableLastChunkNo = 1;
		prMp4Inst->rCurTblPos.rTblAudPos[i].u8CurSampleNo = 1;
		prMp4Inst->rCfaRange.u8AudStartChunk1stSmp[i] = 1;
		prMp4Inst->rCfaRange.u8AudStartChunkNo[i] = 1;
		prMp4Inst->rCfaRange.u8AudStartSampleNo[i] = 1;
		prMp4Inst->rCfaRange.u8AudEndChunkNo[i] = DMX_INVALID_UINT64 - 1;
		prMp4Inst->rCfaRange.u8AudEndSampleNo[i] = DMX_INVALID_UINT64 - 1;
		}
	prMp4Inst->fgFinished = TRUE;
	if (RET_DMX_OK != mrCfaMp4ParseTable(prMp4Inst)) {
		CfaMp4InternalFreeMem(prMp4Inst);
		prMp4Inst->fgFinished = FALSE;
		DMX_FreeMemory(prMp4Inst);
		return FALSE;
	}
	fgBadIntlvd = CfaMp4BadInterleaved(pvSptHdl, prMp4Inst);
	if ((prMp4BadIntlvd->prATable[0] + STCO)->fgIndexErr)
		fgBadIntlvd = TRUE;


	/*fix bug116914*/
	if (prMp4BadIntlvd->u2Width == 1920 && prMp4BadIntlvd->u2Height == 1080
		&& prMp4BadIntlvd->u4FrameperSecond >= 60) {
		fgBadIntlvd = TRUE;
	}
	CfaMp4InternalFreeMem(prMp4Inst);
	prMp4Inst->fgFinished = FALSE;
	DMX_FreeMemory(prMp4Inst);
	return fgBadIntlvd;
}
