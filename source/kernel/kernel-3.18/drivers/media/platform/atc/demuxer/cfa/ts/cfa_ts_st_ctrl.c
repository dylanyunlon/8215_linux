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

#include <media/atc/dmx_define.h>
/* #include <media/atc/mm_debug.h> */

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_sema.h"
#include "cfa_ts.h"
#include "cfa_ts_st_ctrl.h"
#include "stc_hal.h"

#define GetTickCount() \
	(1000*jiffies/HZ)


/*-----------------------------------------------------------------------------
* Name: TsFinishPrs
*
* Description:
*	   After current range are processed already ,this function will be call to finish parse
*
*
* Inputs:
*
* Outputs:
*
* Returns: void
*
*-----------------------------------------------------------------------------*/
static void CfaTsFinishPrs(void *pvSptHdl, CfaTsInst *prCfaTs)
{
	DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
	TEXT("[CFA TS]CfaTsFinishPrs(): Finish parse Ca: %lld, Ea: %lld!\n"),
	prCfaTs->u8Ca, prCfaTs->rRange.u8Ea);
#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaTs);
	MMATE_CHECK_STRUCT(prCfaTs->rRange);
	MMATE_CHECK_STRUCT(prCfaTs->rVidDeltaPesPtsInfo);
	MMATE_CHECK_STRUCT(prCfaTs->rAudDeltaPesPtsInfo);
	MMATE_CHECK_STRUCT(prCfaTs->rAudCmdQBakInfo);
#endif
	prCfaTs->fgStopHandlePkt = TRUE;
	Spt4CfaFinishedEx(pvSptHdl, prCfaTs->u8Ca, FALSE, GAU_E_EOS);

}


#define CFA_TS_ANA_PES_HEADER_ERROR ((__u32)-1)

static bool CfaTsAnaPESMinHeader(CfaTsInst *prCfaTs, CfaTsPesHeader_T *prPesHeader,
	u8 u1StreamId)
{
	if (CFA_TS_STREAM_ID_PADDING_STREAM == u1StreamId) {
		/* padding stream*/
		prPesHeader->eState = CFA_TS_ST_PES_SKIP;
		return true;
	}

	/*get PES packet length*/
	prPesHeader->u4PesPacketLen = ((u32)prPesHeader->aucHeader[4] << 8) | prPesHeader->aucHeader[5];
	prPesHeader->u4PesPacketRealLen = prPesHeader->u4PesPacketLen;
	if (prPesHeader->u4PesPacketLen > CFA_TS_MAX_PES_PACKET_LEN) {
		DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT,
			TEXT("[CFA TS]CfaTsAnaPesHeader():")
			TEXT("PES packet length > CFA_TS_MAX_PES_PACKET_LEN!\r\n"));
			prPesHeader->u4PesPacketLen = 0;
	}
	if (0 == prPesHeader->u4PesPacketLen)
		prPesHeader->u4PesPacketLen = CFA_TS_MAX_PES_PACKET_LEN;

	/*analyse stream id*/
	if ((CFA_TS_STREAM_ID_PROG_STREAM_MAP != u1StreamId)
		&&      /* program_stream_map */
		(CFA_TS_STREAM_ID_PRIVATE_STREAM_2 != u1StreamId)
		&&      /* private_stream_2 */
		(CFA_TS_STREAM_ID_ECM != u1StreamId)
		&&                      /* ECM */
		(CFA_TS_STREAM_ID_EMM != u1StreamId)
		&&                      /*  EMM */
		(CFA_TS_SREEAM_ID_PROG_STREAM_DIRECTORY != u1StreamId)
		&&  /* program_stream_directory */
		(CFA_TS_STREAM_ID_DSMCC_STREAM != u1StreamId)
		&&          /* DSMCC_stream */
		(CFA_TS_STREAM_ID_H2221_TYEPE_E_STREAM != u1StreamId)) {
		/* ITU-T Rec, H.222.1 type E stream */
		prPesHeader->eState = CFA_TS_ST_PES_HEADER;
	} else {
		prPesHeader->eState = CFA_TS_ST_PES_SKIP;
		prPesHeader->u4HeaderLen = 0;
	}

	return FALSE;
}


#if CFG_SUPPORT_HDCP
static u32 CfaTsAnaHDCP(CfaTsPesHeader_T *prPesHeader)
{
	u32 u4SkipByte = 0;

	u8 PTS_DTS_flags = (prPesHeader->aucHeader[7] & 0xc0) >> 6;
	u8 ESCR_flag = (prPesHeader->aucHeader[7] & 0x20) >> 5;
	u8 ES_rate_flag = (prPesHeader->aucHeader[7] & 0x10) >> 4;
	u8 DSM_trick_mode_flag = (prPesHeader->aucHeader[7] & 0x08) >> 3;
	u8 additional_copy_info_flag = (prPesHeader->aucHeader[7] & 0x04) >> 2;
	u8 PES_CRC_flag = (prPesHeader->aucHeader[7] & 0x02) >> 1;

	u8 PES_private_data_flag = 0;
	u8 pack_header_field_falg = 0;
	u8 program_packet_sequence_counter_flag = 0;
	u8 P_STD_buffer_flag = 0;
	u8 PES_extension_flag_2 = 0;

	if (0x02 == PTS_DTS_flags)
		u4SkipByte += 5;
	else if (0x03 == PTS_DTS_flags)
		u4SkipByte += 10;

	if (0x01 == ESCR_flag)
		u4SkipByte += 6;

	if (0x01 == ES_rate_flag)
		u4SkipByte += 3;

	if (0x01 == DSM_trick_mode_flag)
		u4SkipByte += 1;

	if (0x01 == additional_copy_info_flag)
		u4SkipByte += 1;

	if (0x01 == PES_CRC_flag)
		u4SkipByte += 2;
	u4SkipByte += 9;

	PES_private_data_flag
		= (prPesHeader->aucHeader[u4SkipByte] & 0x80) >> 7;
	pack_header_field_falg
		= (prPesHeader->aucHeader[u4SkipByte] & 0x40) >> 6;

	program_packet_sequence_counter_flag
		= (prPesHeader->aucHeader[u4SkipByte] & 0x20) >> 5;
	P_STD_buffer_flag
		= (prPesHeader->aucHeader[u4SkipByte] & 0x10) >> 4;

	PES_extension_flag_2
		= (prPesHeader->aucHeader[u4SkipByte] & 0x01) >> 0;

	u4SkipByte += 1;

	if (0x01 == PES_private_data_flag) {
		if ((u4SkipByte + 16) > prPesHeader->u4PesHeaderDataLen) {
			DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsAnaPesHeader():")
				TEXT("Get u4SkipByte error!\r\n"));
			return CFA_TS_ANA_PES_HEADER_ERROR;
		}

		prPesHeader->u4StreamCounter
			= ((unsigned long)
				(prPesHeader->aucHeader[u4SkipByte + 1] & 0x06) << 29)
			| ((unsigned long)
				(prPesHeader->aucHeader[u4SkipByte + 2]) << 22)
			| ((unsigned long)
				(prPesHeader->aucHeader[u4SkipByte + 3] & 0xfe) << 14)
			| ((unsigned long)
				(prPesHeader->aucHeader[u4SkipByte + 4]) << 7)
			| ((unsigned long)
				(prPesHeader->aucHeader[u4SkipByte + 5] & 0xfe) >> 1);

		prPesHeader->u8InputCounter
			=  ((unsigned long long)
				(prPesHeader->aucHeader[u4SkipByte + 7] & 0x1e) << 59)
			| ((unsigned long long)
				(prPesHeader->aucHeader[u4SkipByte + 8]) << 52)
			| ((unsigned long long)
				(prPesHeader->aucHeader[u4SkipByte + 9] & 0xfe) << 44)
			| ((unsigned long long)
				(prPesHeader->aucHeader[u4SkipByte + 10]) << 37)
			| ((unsigned long long)
				(prPesHeader->aucHeader[u4SkipByte + 11] & 0xfe) << 29)
			| ((unsigned long long)
				(prPesHeader->aucHeader[u4SkipByte + 12]) << 22)
			| ((unsigned long long)
				(prPesHeader->aucHeader[u4SkipByte + 13] & 0xfe) << 14)
			| ((unsigned long long)
				(prPesHeader->aucHeader[u4SkipByte + 14]) << 7)
			| ((unsigned long long)
				(prPesHeader->aucHeader[u4SkipByte + 15] & 0xfe) >> 1);
			((unsigned long long)
				(prPesHeader->aucHeader[30] & 0xfe) >> 1);

		prPesHeader->fgHDCP = TRUE;
	}

	return 0;
}
#endif

/*
*  name: CfaTsAnaPesHeader
*
*  description: analyse pes packet header
*
*  inputs:
*
*  outputs:
*
*  return: >0, we get payload
*/
static u32 CfaTsAnaPesHeader(CfaTsInst *prCfaTs, CfaTsPesHeader_T *prPesHeader,
	u8 *pucData, u32 u4DataLen)
{
	u32 u4Len = 0;
	u8 u1StreamId = 0;

	while (u4DataLen > 0) {
		switch (prPesHeader->eState) {
		case CFA_TS_ST_PES_MIN_HEADER:
			u4Len = CFA_TS_MIN_PES_HEADER_LEN - prPesHeader->u4HeaderLen;
			u4Len = MIN(u4Len, u4DataLen);
			dmx_memcpy(prPesHeader->aucHeader + prPesHeader->u4HeaderLen, pucData, u4Len);
			prPesHeader->u4HeaderLen += u4Len;

			pucData += u4Len;
			u4DataLen -= u4Len;
			if (CFA_TS_MIN_PES_HEADER_LEN == prPesHeader->u4HeaderLen) {
				if ((CFA_TS_START_CODE_BYTE0 == prPesHeader->aucHeader[0]) &&
					(CFA_TS_START_CODE_BYTE1 == prPesHeader->aucHeader[1]) &&
					(CFA_TS_START_CODE_BYTE2 == prPesHeader->aucHeader[2])) {
					u1StreamId = prPesHeader->aucHeader[3];
					if (CfaTsAnaPESMinHeader(prCfaTs, prPesHeader, u1StreamId))
						continue;

				} else {
					prPesHeader->eState = CFA_TS_ST_PES_SKIP;
					continue;
				}
			}
			break;

		case CFA_TS_ST_PES_HEADER:
			if (CFA_TS_PES_HEADER_LEN < prPesHeader->u4HeaderLen) {
				DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("[CFA TS]CfaTsAnaPesHeader(): PES header length error!\r\n"));
				return CFA_TS_ANA_PES_HEADER_ERROR;
			}

			u4Len = CFA_TS_PES_HEADER_LEN - prPesHeader->u4HeaderLen;
			u4Len = DMX_MIN(u4Len, u4DataLen);
			dmx_memcpy(prPesHeader->aucHeader + prPesHeader->u4HeaderLen, pucData, u4Len);
			prPesHeader->u4HeaderLen += u4Len;

			pucData += u4Len;
			u4DataLen -= u4Len;
			if (CFA_TS_PES_HEADER_LEN == prPesHeader->u4HeaderLen) {
				prPesHeader->u4PesHeaderDataLen
					= prPesHeader->aucHeader[8] + CFA_TS_PES_HEADER_LEN;
				if ((prPesHeader->u4PesHeaderDataLen > CFA_TS_MAX_PES_HEADER_LEN) ||
					((prPesHeader->u4PesPacketLen + CFA_TS_MIN_PES_HEADER_LEN)
						< prPesHeader->u4PesHeaderDataLen)) {
					prPesHeader->eState = CFA_TS_ST_PES_SKIP;
					prPesHeader->u4PesHeaderDataLen = 0;
					continue;
				}
				prPesHeader->u4PesPacketDataLen
					= (prPesHeader->u4PesPacketLen + CFA_TS_MIN_PES_HEADER_LEN)
						- prPesHeader->u4PesHeaderDataLen;
				prPesHeader->eState = CFA_TS_ST_PES_HEADER_OPTIONAL;
			}
			break;

		case CFA_TS_ST_PES_HEADER_OPTIONAL:
			if (prPesHeader->u4PesHeaderDataLen < prPesHeader->u4HeaderLen) {
				DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("[CFA TS]CfaTsAnaPesHeader(): PES header optional length error!\r\n"));
				return CFA_TS_ANA_PES_HEADER_ERROR;
			}

			u4Len = prPesHeader->u4PesHeaderDataLen - prPesHeader->u4HeaderLen;
			u4Len = DMX_MIN(u4Len, u4DataLen);
			dmx_memcpy(prPesHeader->aucHeader + prPesHeader->u4HeaderLen, pucData, u4Len);
			prPesHeader->u4HeaderLen += u4Len;

			pucData += u4Len;
			u4DataLen -= u4Len;
			if (prPesHeader->u4PesHeaderDataLen == prPesHeader->u4HeaderLen) {
				/*get pts/dts*/
				prPesHeader->u8Pts = INVALID_TIMESTAMP;
				if (prPesHeader->aucHeader[7] & CFA_TS_FLAG_PTS) {
					/*has pts*/
					prPesHeader->u8Pts
						= ((u64)(prPesHeader->aucHeader[9] & 0x0e) << 29) |
							((u64)(prPesHeader->aucHeader[10]) << 22) |
							((u64)(prPesHeader->aucHeader[11] & 0xfe) << 14) |
							((u64)(prPesHeader->aucHeader[12]) << 7) |
							((u64)(prPesHeader->aucHeader[13]) >> 1);
				   /* RETAILMSG(1, (L"[CFA TS] PES PTS: 0x%08x%08x\n",
				   (u32)(prPesHeader->u8Pts >> 32), (u32)(prPesHeader->u8Pts)));*/
				}
#if CFG_SUPPORT_HDCP
				if ((DATA_SOURCE_WFD == prCfaTs->eDataSource) &&
					prCfaTs->fgHDCP &&
					(prPesHeader->aucHeader[7] & CFA_TS_FLAG_PES_EXTENSION)) {
					u32 u4Ret = CfaTsAnaHDCP(prPesHeader);

					if (0 != u4Ret)
						return CFA_TS_ANA_PES_HEADER_ERROR;
				}
#endif
				prPesHeader->eState = CFA_TS_ST_PES_PAYLOD;
				prPesHeader->u4HeaderLen = 0;
			}
			break;

		case CFA_TS_ST_PES_PAYLOD:
		{
			return u4DataLen;
		}

		case CFA_TS_ST_PES_SKIP:
		default:
			u4DataLen = 0;
			break;
		}
	}

	return u4DataLen;
}

#if 0
static void CfaTsAudDataStatistics(u64 u8Len)
{
	static u32 u4TimeElapsed;
	static u64 u8DataLne;
	static u32 u4LastTickCnt;
	u32 u4TickCnt = GetTickCount();

	if (0 == u4LastTickCnt) {
		u4LastTickCnt = u4TickCnt;
		return;
	}

	u8DataLne += u8Len;
	u4TimeElapsed += (u4TickCnt - u4LastTickCnt);
	u4LastTickCnt = u4TickCnt;

	if (60 * 1000 <= u4TimeElapsed) {
		DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
		TEXT("[CFA TS] CfaTsAudDataStatistics(): tx data len: %lld in one min\n"),
		u8DataLne);

		u8DataLne = 0;
		u4TimeElapsed = 0;
	}
}
#endif

static void CfaTsTxVidCmdQ(CfaTsInst *prCfaTs)
{
	MRESULT mrRet = RET_DMX_OK;
	CfaTsVidPrivate_T *prVidPriv = NULL;

	prVidPriv
		= (CfaTsVidPrivate_T *)prCfaTs->Pids[prCfaTs->u4CurVidPID]->pvPrivate;
	prCfaTs->eCurPrsStm = CFA_TS_PRS_STRM_TYPE_V;

	prCfaTs->rVidInf.u8FileOfst = prCfaTs->u8VidFirstOffset;
	prCfaTs->rVidInf.eTxMode = CFA_PTM_EXACT_POS;
	prCfaTs->rVidInf.eVidType = prVidPriv->eVCodec;
	prCfaTs->rVidInf.u8Len = prCfaTs->u8VidPreTxEndOff - prCfaTs->u8VidFirstOffset;
	prCfaTs->rVidInf.u4PrsStrmId = prCfaTs->u4VideoStreamID;
	prCfaTs->rVidInf.fgUseCmdQ = TRUE;
	prCfaTs->rVidInf.u2TxEntryCnt = (u16)prCfaTs->u4VidCmdQIndex;
	prCfaTs->rVidInf.u8RealTxLen = prCfaTs->u8VidDataInBuf;
	prCfaTs->rVidInf.fgUnitStart = FALSE;
	prCfaTs->rVidInf.fgAUCompleteByEnd = FALSE;

	mrRet = Spt4CfaPbb2VFifoAUCtrl(prCfaTs->pvSptHdl, &prCfaTs->rVidInf);
	if (RET_DMX_OK != mrRet){
		CfaTsFinishPrs(prCfaTs->pvSptHdl, prCfaTs);
	}
	prCfaTs->u4VidCmdQIndex = 0;
	prCfaTs->u8VidDataInBuf = 0;
}


static void CfaTsTxAudCmdQ(CfaTsInst *prCfaTs)
{
	MRESULT mrRet = RET_DMX_OK;
	CfaTsAudPrivate_T *prAudPriv = NULL;

	prAudPriv = (CfaTsAudPrivate_T *)prCfaTs->Pids[prCfaTs->u4CurAudPID]->pvPrivate;

	prCfaTs->eCurPrsStm = CFA_TS_PRS_STRM_TYPE_A;

	prCfaTs->rAudInf.parCmdQTxEntry[prCfaTs->u4AudCmdQIndex - 1].fgEndAU  = TRUE;
	prCfaTs->rAudInf.u8FileOfst = prCfaTs->u8AudFirstOffset;
	prCfaTs->rAudInf.eAudType = prAudPriv->eACodec;
	prCfaTs->rAudInf.u8Len = prCfaTs->u8AudPreTxEndOff - prCfaTs->u8AudFirstOffset;
	prCfaTs->rAudInf.u4PrsStrmId = prCfaTs->u4AudioStreamID;
	prCfaTs->rAudInf.fgUseCmdQ = TRUE;
	prCfaTs->rAudInf.u2TxEntryCnt = (u16)prCfaTs->u4AudCmdQIndex;
	prCfaTs->rAudInf.u8RealTxLen = prCfaTs->u8AudDataInBuf;
	prCfaTs->rAudInf.fgUnitStart = TRUE;
	prCfaTs->rAudInf.fgAUCompleteByEnd = FALSE;

	if (prCfaTs->rAudCmdQPtsInfo.u4EntryNb > 0)
		prCfaTs->eACmdQFirstPtsEntryState = PTS_VALID;
	else
		prCfaTs->eACmdQFirstPtsEntryState = PTS_INVALID;

	prCfaTs->rAudInf.u8TotalAULen = prCfaTs->rAudInf.u8Len;
	mrRet = Spt4CfaPbb2AFifoAUCtrl(prCfaTs->pvSptHdl, &prCfaTs->rAudInf);
	if (RET_DMX_OK != mrRet) {
		CfaTsFinishPrs(prCfaTs->pvSptHdl, prCfaTs);
	}
	prCfaTs->u4AudCmdQIndex = 0;
	prCfaTs->u8AudDataInBuf = 0;
}

static bool CfaTsCheckConsecutivePts(u64 *pu8Rec4CorrectAPts, u32 u4PtsCnt)
{
	u64 u8Delta1 = 0;
	u64 u8Delta2 = 0;
	u32 i = 0;

	if (u4PtsCnt < 2)
		return FALSE;

	for (i = 2; i < u4PtsCnt; i++) {
		u8Delta1 = pu8Rec4CorrectAPts[i] - pu8Rec4CorrectAPts[i - 1];
		u8Delta2 = pu8Rec4CorrectAPts[i - 1] - pu8Rec4CorrectAPts[i - 2];
		if (u8Delta1 != u8Delta2) {
			DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsCheckConsecutivePts(): pts is not consecutive\n"));
			return FALSE;
		}
	}

	DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
		TEXT("[CFA TS]CfaTsCheckConsecutivePts(): pts is consecutive\n"));
	return TRUE;
}


static bool CfaTsCheckFaintlyConsecutivePts(u64 *pu8Rec4CorrectAPts, u32 u4PtsCnt)
{
	u64 u8Delta1 = 0;
	u64 u8Delta2 = 0;
	u64 u8DeltaPtsDiff = 0;
	u32 i = 0;

	if (u4PtsCnt < 2)
		return FALSE;

	for (i = 2; i < u4PtsCnt; i++) {
		u8Delta1 = pu8Rec4CorrectAPts[1] - pu8Rec4CorrectAPts[0];
		u8Delta2 = pu8Rec4CorrectAPts[i - 1] - pu8Rec4CorrectAPts[i - 2];

		if (u8Delta1 >= u8Delta2)
			u8DeltaPtsDiff = u8Delta1 - u8Delta2;

		else
			u8DeltaPtsDiff = u8Delta2 - u8Delta1;

		if (u8DeltaPtsDiff > CFA_TS_DIFF_DELTA_PES_PTS) {
			DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsCheckConsecutivePts(): pts is not Faintly consecutive\n"));
			return FALSE;
		}
	}

	DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
		TEXT("[CFA TS]CfaTsCheckConsecutivePts(): pts is Faintly consecutive\n"));
	return TRUE;
}


static void CfaTsGetDeltaPesPts(DeltaPesPtsInfo_t *prDeltaPesPtsInfo, u64 u8Pts)
{
	u32 i = 0;

	if (INVALID_TIMESTAMP == u8Pts) {
		for (i = 0; i < CFA_TS_MAX_PTS_RECORD_LEN; i++)
			prDeltaPesPtsInfo->au8PtsRecord[i] = 0;
		return;
	}

	for (i = 0; i < CFA_TS_MAX_PTS_RECORD_LEN; i++) {
		if (0 == prDeltaPesPtsInfo->au8PtsRecord[i]) {
			prDeltaPesPtsInfo->au8PtsRecord[i] = u8Pts;

			if (VID_DELTA_PE_PTS_INFO == prDeltaPesPtsInfo->eInfoType) {
				DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("VID prDeltaPesPtsInfo->au8PtsRecord[%d]: 0x%08x%08x\r\n"), i,
					 (u32)(prDeltaPesPtsInfo->au8PtsRecord[i] >> 32),
					 (u32)(prDeltaPesPtsInfo->au8PtsRecord[i]));
			} else if (AUD_DELTA_PE_PTS_INFO == prDeltaPesPtsInfo->eInfoType) {
				DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("AUD prDeltaPesPtsInfo->au8PtsRecord[%d]: 0x%08x%08x\r\n"), i,
					(u32)(prDeltaPesPtsInfo->au8PtsRecord[i] >> 32),
					(u32)(prDeltaPesPtsInfo->au8PtsRecord[i]));

			} else {
				DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("unknown pts info type\r\n"));
			}

			break;
		}
	}


	if ((i > 0) && (i < CFA_TS_MAX_PTS_RECORD_LEN)) {
		if (prDeltaPesPtsInfo->au8PtsRecord[i] <
			prDeltaPesPtsInfo->au8PtsRecord[i-1]) {
			for (i = 0; i < CFA_TS_MAX_PTS_RECORD_LEN; i++)
				prDeltaPesPtsInfo->au8PtsRecord[i] = 0;

			return;
		}
	}

	if (i == 1) {
		prDeltaPesPtsInfo->u8DeltaPesPts =
			prDeltaPesPtsInfo->au8PtsRecord[1] - prDeltaPesPtsInfo->au8PtsRecord[0];

		if (prDeltaPesPtsInfo->u8DeltaPesPts > PTS_INTERVAL) {
			for (i = 0; i < CFA_TS_MAX_PTS_RECORD_LEN; i++)
				prDeltaPesPtsInfo->au8PtsRecord[i] = 0;

			prDeltaPesPtsInfo->u8DeltaPesPts = 0;

			return;
		}
	} else if (i > 1) {
		if (CFA_TS_MAX_PTS_RECORD_LEN == i) {
			if (0 != prDeltaPesPtsInfo->u8DeltaPesPts) {
				prDeltaPesPtsInfo->fgNeedGet = FALSE;
				prDeltaPesPtsInfo->u4GetCnt = 0;
			} else {
				for (i = 0; i < CFA_TS_MAX_PTS_RECORD_LEN; i++)
					prDeltaPesPtsInfo->au8PtsRecord[i] = 0;
			}

			return;
		} else if (prDeltaPesPtsInfo->u8DeltaPesPts !=
			(prDeltaPesPtsInfo->au8PtsRecord[i] - prDeltaPesPtsInfo->au8PtsRecord[i - 1])) {
			if (prDeltaPesPtsInfo->u4GetCnt < CFA_TS_MAX_GET_DELTA_PES_PTS_CNT) {
				for (i = 0; i < CFA_TS_MAX_PTS_RECORD_LEN; i++)
					prDeltaPesPtsInfo->au8PtsRecord[i] = 0;

				prDeltaPesPtsInfo->u8DeltaPesPts = 0;
				prDeltaPesPtsInfo->u4GetCnt += 1;

				if (VID_DELTA_PE_PTS_INFO == prDeltaPesPtsInfo->eInfoType) {
					DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("VID prDeltaPesPtsInfo->u4GetCnt: %d\r\n"),
					prDeltaPesPtsInfo->u4GetCnt);
				} else if (AUD_DELTA_PE_PTS_INFO == prDeltaPesPtsInfo->eInfoType) {
					DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("AUD prDeltaPesPtsInfo->u4GetCnt: %d\r\n"),
					prDeltaPesPtsInfo->u4GetCnt);
				} else
					DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
						TEXT("unknown pts info type\r\n"));
			} else {
				u64 u8DeltaPtsDiff = 0;

				u64 u8TempDeltaPts
					= prDeltaPesPtsInfo->au8PtsRecord[i]
						- prDeltaPesPtsInfo->au8PtsRecord[i - 1];

				if (prDeltaPesPtsInfo->u8DeltaPesPts > u8TempDeltaPts)
					u8DeltaPtsDiff = prDeltaPesPtsInfo->u8DeltaPesPts - u8TempDeltaPts;

				else
					u8DeltaPtsDiff = u8TempDeltaPts - prDeltaPesPtsInfo->u8DeltaPesPts;

				if (u8DeltaPtsDiff > CFA_TS_DIFF_DELTA_PES_PTS) {
					for (i = 0; i < CFA_TS_MAX_PTS_RECORD_LEN; i++)
						prDeltaPesPtsInfo->au8PtsRecord[i] = 0;

					prDeltaPesPtsInfo->u8DeltaPesPts = 0;
					prDeltaPesPtsInfo->u4GetCnt += 1;

					if (VID_DELTA_PE_PTS_INFO == prDeltaPesPtsInfo->eInfoType) {
						DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
						TEXT("VID prDeltaPesPtsInfo->u4GetCnt: %d\r\n"),
						prDeltaPesPtsInfo->u4GetCnt);
					} else if (AUD_DELTA_PE_PTS_INFO == prDeltaPesPtsInfo->eInfoType) {
						DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
						TEXT("AUD prDeltaPesPtsInfo->u4GetCnt: %d\r\n"),
						prDeltaPesPtsInfo->u4GetCnt);
					} else
						DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
							TEXT("unknown pts info type\r\n"));
				}
			}
		}
	}

}

static void CfaTsCheckVPtsState(CfaTsInst *prCfaTs, CfaTsPesHeader_T *prPesHeader)
{
	if (prCfaTs->fgIsISDBT1Seg || (!prCfaTs->fgIsISDBT1Seg
			&& (INVALID_TIMESTAMP != prCfaTs->u8CurPesVidPts)))
		prCfaTs->u8LastPesVidPts = prCfaTs->u8CurPesVidPts;

	prCfaTs->u8CurPesVidPts = prPesHeader->u8Pts;

	if (prCfaTs->fgNoSignal) {
		prCfaTs->fgNoSignal = FALSE;
		prCfaTs->eVPtsState = VPTS_NO_SIGNAL;
		prCfaTs->u4LastVidTicCnt = prCfaTs->u4CurVidTicCnt = 0;

		return;
	}

	if (prCfaTs->fgPcrChange) {
		prCfaTs->fgPcrChange = FALSE;
		prCfaTs->eVPtsState = VPTS_PCR_CHANGE;

		return;
	}
#ifdef BSP_WIFI_WFD
	if (((INVALID_TIMESTAMP == prPesHeader->u8Pts)
			&& (DATA_SOURCE_WFD == prCfaTs->eDataSource))) {
		prCfaTs->eVPtsState = VPTS_OK;

		return;
	}
 #endif
	{
		if (INVALID_TIMESTAMP == prPesHeader->u8Pts) {
			prCfaTs->eVPtsState = VPTS_DATA_ERROR;

			return;
		}
	}

	/*check whether has audio*/
	prCfaTs->u4LastVidTicCnt = prCfaTs->u4CurVidTicCnt;
	prCfaTs->u4CurVidTicCnt = GetTickCount();
	if ((prCfaTs->u4LastVidTicCnt > 0) && (prCfaTs->u4CurVidTicCnt
			> prCfaTs->u4LastVidTicCnt)) {
		if ((((prCfaTs->u4CurVidTicCnt - prCfaTs->u4LastVidTicCnt)
				> NO_SIGNAL_TICK_COUNT) && (prCfaTs->u4NoAudTicCnt > 0))
				|| ((prCfaTs->u4CurVidTicCnt - prCfaTs->u4LastVidTicCnt)
					< NO_SIGNAL_TICK_COUNT)) {
			prCfaTs->u4NoAudTicCnt += (prCfaTs->u4CurVidTicCnt - prCfaTs->u4LastVidTicCnt);
		}
	}
   /* RETAILMSG(1, (L"prCfaTs->u4NoAudTicCnt: %d\n", prCfaTs->u4NoAudTicCnt));*/

	if (prCfaTs->u4NoAudTicCnt > NO_AUD_TICK_COUNT) {
		if ((prCfaTs->u4LastVidTicCnt > 0)
			&& (prCfaTs->u4CurVidTicCnt > (prCfaTs->u4LastVidTicCnt + NO_SIGNAL_TICK_COUNT))
			&& ((DATA_SOURCE_STREAM == prCfaTs->eDataSource)
			#ifdef BSP_WIFI_WFD
				|| (DATA_SOURCE_WFD == prCfaTs->eDataSource)
			#endif
				)) {
			DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsCheckVPtsState(): u4CurVidTicCnt: %d, u4LastVidTicCnt: %d\n"),
				prCfaTs->u4CurVidTicCnt, prCfaTs->u4LastVidTicCnt);
			prCfaTs->eVPtsState = VPTS_NO_SIGNAL;

			return;
		}
	}

	#ifdef BSP_WIFI_WFD
	if ((!prCfaTs->fgFirstGetVidPts) && (DATA_SOURCE_WFD == prCfaTs->eDataSource)) {
		prCfaTs->eVPtsState = VPTS_OK;

		return;
	}
	#endif

	if (prCfaTs->fgFirstGetVidPts) {
		prCfaTs->eVPtsState = VPTS_OK;

		prCfaTs->fgFirstGetVidPts = FALSE;

		/*if the first video pts is too larger than the first audio pts, adjust it*/
		if ((INVALID_TIMESTAMP != prCfaTs->u8AudBasePts)
			&& (prPesHeader->u8Pts > (prCfaTs->u8AudBasePts + MAX_GAP_PTS_BETWEEN_AV))) {
			DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsCheckVPtsState():")
				TEXT("First video pes pts is too large-> u8Pts:")
				TEXT("0x%08x%08x, u8AudBasePts: 0x%08x%08x\r\n"),
				(u32)(prPesHeader->u8Pts >> 32), (u32)(prPesHeader->u8Pts),
				(u32)(prCfaTs->u8AudBasePts >> 32), (u32)(prCfaTs->u8AudBasePts));
			prPesHeader->u8Pts = prCfaTs->u8AudBasePts;
		}

		prCfaTs->u8VidBasePts = prPesHeader->u8Pts;
		prCfaTs->u8CurPesVidPts = prPesHeader->u8Pts;
		DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsCheckVPtsState(): u8VidBasePts: 0x%08x%08x\r\n"),
			(u32)(prCfaTs->u8VidBasePts >> 32), (u32)(prCfaTs->u8VidBasePts));
		return;
	}
	if (prCfaTs->u8CurPesVidPts < prCfaTs->u8LastPesVidPts) {
		prCfaTs->eVPtsState = VPTS_DATA_ERROR;

		if (0 == prCfaTs->u4PesVidPtsDecCnt) {
			prCfaTs->u8FirstAbnormalPesVidPts = prCfaTs->u8CurPesVidPts;
			prCfaTs->u8OldLastPesVidPts = prCfaTs->u8LastPesVidPts;
		}

		if ((prCfaTs->u8OldLastPesVidPts > 0) && (prCfaTs->u8CurPesVidPts
				< prCfaTs->u8OldLastPesVidPts)) {
			if (prCfaTs->u4PesVidPtsDecCnt < CFA_TS_MAX_PTS_CHG_CNT) {
				prCfaTs->au8Rec4CorrectVPts[prCfaTs->u4PesVidPtsDecCnt]
					= prCfaTs->u8CurPesVidPts;

				DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("[CFA TS]CfaTsCheckVPtsState():")
					TEXT("au8Rec4CorrectVPts[%d]: 0x%x%x\r\n"),
					prCfaTs->u4PesVidPtsDecCnt,
					(u32)(prCfaTs->au8Rec4CorrectVPts[prCfaTs->u4PesVidPtsDecCnt] >> 32),
					(u32)(prCfaTs->au8Rec4CorrectVPts[prCfaTs->u4PesVidPtsDecCnt]));
			}
			prCfaTs->u4PesVidPtsDecCnt += 1;

			if (prCfaTs->u4PesVidPtsDecCnt >= CFA_TS_MAX_PTS_CHG_CNT) {
				prCfaTs->u4PesVidPtsDecCnt = 0;
				prCfaTs->u8FirstAbnormalPesVidPts = 0;
				prCfaTs->u8OldLastPesVidPts = 0;

				if (CfaTsCheckConsecutivePts(prCfaTs->au8Rec4CorrectVPts,
						CFA_TS_MAX_PTS_CHG_CNT)
					&& (APTS_OK == prCfaTs->eAPtsState)) {

					if (prCfaTs->u4NoAudTicCnt <= NO_AUD_TICK_COUNT)
						prCfaTs->eVPtsState = VPTS_OK;

					else
						prCfaTs->eVPtsState = VPTS_PCR_CHANGE;
				}
			}
			DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsCheckVPtsState(): u4PesVidPtsDecCnt: %d\n"),
				prCfaTs->u4PesVidPtsDecCnt);
		} else {
			prCfaTs->u4PesVidPtsDecCnt = 0;
			prCfaTs->u8OldLastPesVidPts = 0;
			prCfaTs->u8FirstAbnormalPesVidPts = 0;
			prCfaTs->eVPtsState = VPTS_OK;
		}

		prCfaTs->u4PesVidPtsIncCnt = 0;
	} else {
		if ((prCfaTs->u8LastPesVidPts > 0)
			&& (prCfaTs->u8CurPesVidPts > (prCfaTs->u8LastPesVidPts + 2 * PTS_1S))) {
			prCfaTs->eVPtsState = VPTS_DATA_ERROR;

			if (0 == prCfaTs->u4PesVidPtsIncCnt)
				prCfaTs->u8FirstAbnormalPesVidPts
					= prCfaTs->u8CurPesVidPts;

			if (prCfaTs->u4PesVidPtsIncCnt < CFA_TS_MAX_PTS_CHG_CNT) {
				prCfaTs->au8Rec4CorrectVPts[prCfaTs->u4PesVidPtsIncCnt]
					= prCfaTs->u8CurPesVidPts;
				DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("[CFA TS]CfaTsCheckVPtsState():")
					TEXT("au8Rec4CorrectVPts[%d]: 0x%08x%08x\r\n"),
					prCfaTs->u4PesVidPtsIncCnt,
					(u32)(prCfaTs->au8Rec4CorrectVPts[prCfaTs->u4PesVidPtsIncCnt] >> 32),
					(u32)(prCfaTs->au8Rec4CorrectVPts[prCfaTs->u4PesVidPtsIncCnt]));
			}
			prCfaTs->u4PesVidPtsIncCnt += 1;

			if (prCfaTs->u4PesVidPtsIncCnt >= CFA_TS_MAX_PTS_CHG_CNT) {
				prCfaTs->u4PesVidPtsIncCnt = 0;
				prCfaTs->u8FirstAbnormalPesVidPts = 0;

				if (CfaTsCheckConsecutivePts(prCfaTs->au8Rec4CorrectVPts,
					CFA_TS_MAX_PTS_CHG_CNT) && (APTS_OK == prCfaTs->eAPtsState)) {
					if (prCfaTs->u4NoAudTicCnt <= NO_AUD_TICK_COUNT)
						prCfaTs->eVPtsState = VPTS_OK;

					else
						prCfaTs->eVPtsState = VPTS_PCR_CHANGE;
				}
			}

			DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsCheckVPtsState(): u4PesVidPtsIncCnt: %d\n"),
				prCfaTs->u4PesVidPtsIncCnt);
		} else {
			prCfaTs->u4PesVidPtsIncCnt = 0;
			prCfaTs->u8FirstAbnormalPesVidPts = 0;
			prCfaTs->eVPtsState = VPTS_OK;
		}

		prCfaTs->u4PesVidPtsDecCnt = 0;
	}
}


static void CfaTsAdjustVPtsPcrChangeOrNoSig(CfaTsInst *prCfaTs, CfaTsPesHeader_T *prPesHeader)
{
	if ((prCfaTs->u8CurPesVidPts > (prCfaTs->u8AudBasePts + 10 * PTS_1S))
		|| ((prCfaTs->u8CurPesVidPts + 10 * PTS_1S) < prCfaTs->u8AudBasePts)) {
		DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
            TEXT("[CFA TS]CfaTsAdjustVPtsPcrChangeOrNoSig():")
			TEXT("u8CurPesVidPts abnormal!!!!!!!!!!!!!!!!!!!!!!\n"));
		prPesHeader->u8Pts = prCfaTs->u8AudBasePts;
		prCfaTs->u8CurPesVidPts = prPesHeader->u8Pts;

		return;
	}
}

static void CfaTsCheckVidPesPts(CfaTsInst *prCfaTs, CfaTsPesHeader_T *prPesHeader)
{
	/*get delta video Pes Pts*/
	#ifdef BSP_WIFI_WFD
	if (DATA_SOURCE_WFD == prCfaTs->eDataSource)
		DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
		TEXT("[CFA TS] CfaTsCheckVidPesPts():wfd not get delta pts\n"));

	else
	#endif
	{
	if (prCfaTs->rVidDeltaPesPtsInfo.fgNeedGet
			&& prCfaTs->fgIsISDBT1Seg)
		CfaTsGetDeltaPesPts(&prCfaTs->rVidDeltaPesPtsInfo, prPesHeader->u8Pts);
	}

	/*check pts state*/
	CfaTsCheckVPtsState(prCfaTs, prPesHeader);

	switch (prCfaTs->eVPtsState) {
	case VPTS_DATA_ERROR:
		DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS] CfaTsCheckVidPesPts():")
			TEXT("======================   VPTS_DATA_ERROR\n"));
		if (prCfaTs->fgIsISDBT1Seg) {
			prPesHeader->u8Pts
				= prCfaTs->u8LastPesVidPts + prCfaTs->rVidDeltaPesPtsInfo.u8DeltaPesPts;
			prCfaTs->u8CurPesVidPts = prPesHeader->u8Pts;

#if CFG_SHOW_LOG
		DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
		TEXT("[CFA TS]CfaTsCheckVidPesPts():")
		TEXT("u8CurPesVidPts: 0x%08x%08x, u8LastPesVidPts:")
		TEXT("0x%08x%08x, u8DeltaVPesPts: 0x%08x%08x\r\n"),
			(u32)(prCfaTs->u8CurPesVidPts >> 32), (u32)(prCfaTs->u8CurPesVidPts),
			(u32)(prCfaTs->u8LastPesVidPts >> 32), (u32)(prCfaTs->u8LastPesVidPts),
			(u32)(prCfaTs->rVidDeltaPesPtsInfo.u8DeltaPesPts >> 32),
			(u32)(prCfaTs->rVidDeltaPesPtsInfo.u8DeltaPesPts));
#endif
		}
		break;

	case VPTS_PCR_CHANGE:
		DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS] CfaTsCheckVidPesPts():")
			TEXT("========================	 VPTS_PCR_CHANGE\n"));

		if (prCfaTs->u4NoAudTicCnt <= NO_AUD_TICK_COUNT)
			CfaTsAdjustVPtsPcrChangeOrNoSig(prCfaTs, prPesHeader);

		else {
			/*no audio*/
			prCfaTs->u8PtsIncrement += prCfaTs->u8LastPesVidPts
				+ prCfaTs->rVidDeltaPesPtsInfo.u8DeltaPesPts - prCfaTs->u8VidBasePts;

			prCfaTs->u4VidCmdQIndex = 0;
			prCfaTs->u8VidDataInBuf = 0;
			prCfaTs->u4AudCmdQIndex = 0;
			prCfaTs->u8AudDataInBuf = 0;
			prCfaTs->rAudCmdQPtsInfo.u4EntryNb = 0;
			prCfaTs->rVidCmdQPtsInfo.u4EntryNb = 0;
		}
		prCfaTs->u8VidBasePts = prCfaTs->u8CurPesVidPts;
#if CFG_SHOW_LOG
		DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS] CfaTsCheckVidPesPts():")
			TEXT("u8CurPesVidPts: 0x%08x%08x, u8LastPesVidPts:")
			TEXT("0x%08x%08x, u8DeltaVPesPts: 0x%08x%08x\r\n"),
			(u32)(prCfaTs->u8CurPesVidPts >> 32), (u32)(prCfaTs->u8CurPesVidPts),
			(u32)(prCfaTs->u8LastPesVidPts >> 32), (u32)(prCfaTs->u8LastPesVidPts),
			(u32)(prCfaTs->rVidDeltaPesPtsInfo.u8DeltaPesPts >> 32),
			(u32)(prCfaTs->rVidDeltaPesPtsInfo.u8DeltaPesPts));
#endif
		prCfaTs->u4PesVidPtsDecCnt = 0;
		prCfaTs->u4PesVidPtsIncCnt = 0;
		prCfaTs->u8FirstAbnormalPesVidPts = 0;

		break;

	case VPTS_NO_SIGNAL:
		DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS] CfaTsCheckVidPesPts():")
			TEXT("==========================   VPTS_NO_SIGNAL\n"));
		if (prCfaTs->u4NoAudTicCnt <= NO_AUD_TICK_COUNT)
			CfaTsAdjustVPtsPcrChangeOrNoSig(prCfaTs, prPesHeader);
		else {
			Spt4CfaClearAllStmData(prCfaTs->pvSptHdl);
			STC_HalGetTime(0, &prCfaTs->u8PtsIncrement);

			if ((1 == ((prCfaTs->u8PtsIncrement >> 32) & 0x0000000000000001))
				&& (prCfaTs->u8PtsIncrement > 0x1FFFE0000LL)) {
				DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsCheckVidPesPts():")
				TEXT("STC_HalGetTime get prCfaTs->u8PtsIncrement ERROR: %lld\r\n"),
					 (prCfaTs->u8PtsIncrement)*(1000/9));

				prCfaTs->u8PtsIncrement = 0;
			}

			prCfaTs->u4VidCmdQIndex = 0;
			prCfaTs->u8VidDataInBuf = 0;
			prCfaTs->u4AudCmdQIndex = 0;
			prCfaTs->u8AudDataInBuf = 0;
			prCfaTs->rAudCmdQPtsInfo.u4EntryNb = 0;
			prCfaTs->rVidCmdQPtsInfo.u4EntryNb = 0;
		}
		prCfaTs->u8VidBasePts = prCfaTs->u8CurPesVidPts;
#if CFG_SHOW_LOG
		DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS] CfaTsCheckVidPesPts():")
			TEXT("u8CurPesVidPts: 0x%08x%08x, u8LastPesVidPts:")
			TEXT("0x%08x%08x, u8DeltaVPesPts: 0x%08x%08x\r\n"),
			(u32)(prCfaTs->u8CurPesVidPts >> 32), (u32)(prCfaTs->u8CurPesVidPts),
			(u32)(prCfaTs->u8LastPesVidPts >> 32), (u32)(prCfaTs->u8LastPesVidPts),
			(u32)(prCfaTs->rVidDeltaPesPtsInfo.u8DeltaPesPts >> 32),
			(u32)(prCfaTs->rVidDeltaPesPtsInfo.u8DeltaPesPts));
#endif
		prCfaTs->u4PesVidPtsDecCnt = 0;
		prCfaTs->u4PesVidPtsIncCnt = 0;
		prCfaTs->u8FirstAbnormalPesVidPts = 0;

		break;

	case VPTS_OK:
	default:
		break;
	}

}

static void CfaTsVidFillCmdQ(CfaTsInst *prCfaTs, u32 u4DataLen)
{
	/*fill cmdq entry*/
	if (prCfaTs->u4VidCmdQIndex == 0) {
		prCfaTs->u8VidDataInBuf = 0;
		prCfaTs->u8VidPreTxEndOff = prCfaTs->u8ParseOffset;
		prCfaTs->u8VidFirstOffset = prCfaTs->u8ParseOffset;
	}

	(prCfaTs->rVidInf.parCmdQTxEntry + prCfaTs->u4VidCmdQIndex)->u4TxOfst
		= (u32)(prCfaTs->u8ParseOffset - prCfaTs->u8VidPreTxEndOff);
	(prCfaTs->rVidInf.parCmdQTxEntry + prCfaTs->u4VidCmdQIndex)->u4TxLen
		= u4DataLen;
	prCfaTs->u8VidPreTxEndOff = prCfaTs->u8ParseOffset + u4DataLen;

	prCfaTs->u8VidDataInBuf += u4DataLen;
	prCfaTs->u4VidCmdQIndex += 1;
}

void CfaTsVidPesCb(CfaTsPidFilter_T *prPidFilter, u8 *pucData, u32 u4DataLen)
{
	CfaTsVidPrivate_T *prPrivate = (CfaTsVidPrivate_T *)prPidFilter->pvPrivate;
	CfaTsPesHeader_T *prPesHeader = &prPrivate->rPesHeader;
	CfaTsInst *prCfaTs = prPrivate->prCfaTs;
	u32 u4Len = 0;
	MRESULT mrRet = RET_DMX_OK;

	prCfaTs->eDataType = DATA_V;
	if (prCfaTs->fgVidSeek) {
		if (!prPidFilter->fgUnitStart) {
			prCfaTs->u8ParseOffset += u4DataLen;

			return;
		}
		//prCfaTs->fgVidSeek = FALSE;
	}

	if (prPidFilter->fgUnitStart) {
		prPesHeader->eState = CFA_TS_ST_PES_MIN_HEADER;
		prPesHeader->u4HeaderLen = 0;
		prPesHeader->u4PesPacketDataLen = 0;
		prPrivate->fgAnaPesHeaderEnd = FALSE;
	}
	#ifdef BSP_WIFI_WFD
	/*RETAILMSG(1,(L"eDataSource:%d\n",prCfaTs->eDataSource));*/
	if (DATA_SOURCE_WFD == prCfaTs->eDataSource) {
		if (0 == prCfaTs->u4VidCmdQIndex && prCfaTs->fgFillAu) {
			u32 i = 0;
			u32 u4entryNb = prCfaTs->rVidCmdQPtsInfoBak.u4EntryNb;
			/*backup pts info*/
			prCfaTs->rVidCmdQPtsInfoBak.u4EntryNb = 0;
			prCfaTs->rVidCmdQPtsInfoBak.u4UsedEntryCnt = 0;
			for (i = 0; i < prCfaTs->rVidCmdQPtsInfo.u4EntryNb; i++) {
				if (PTS_VALID == prCfaTs->rVidCmdQPtsInfo.aruEntryInfo[i].eState) {
					prCfaTs->rVidCmdQPtsInfoBak.aruEntryInfo[u4entryNb].u8FileOffset
							= prCfaTs->rVidCmdQPtsInfo.aruEntryInfo[i].u8FileOffset;

					prCfaTs->rVidCmdQPtsInfoBak.aruEntryInfo[u4entryNb].u8Pts
							= prCfaTs->rVidCmdQPtsInfo.aruEntryInfo[i].u8Pts;

					prCfaTs->rVidCmdQPtsInfoBak.aruEntryInfo[u4entryNb].eState
							= PTS_VALID;

					prCfaTs->rVidCmdQPtsInfoBak.u4EntryNb++;
					/*RETAILMSG(1, (L"[CFA TS] CfaTsVidPesCb rVidCmdQPtsInfoBak pts:
						0x%08x%08x u8FileOffset: 0x%08x%08x\n", */
					/*(u32)(prCfaTs->rVidCmdQPtsInfo.aruEntryInfo[i].u8Pts>>32),
						(u32)prCfaTs->rVidCmdQPtsInfo.aruEntryInfo[i].u8Pts, */
					/*(u32)(prCfaTs->rVidCmdQPtsInfo.aruEntryInfo[i].u8FileOffset>>32),
						(u32)prCfaTs->rVidCmdQPtsInfo.aruEntryInfo[i].u8FileOffset));*/

				}
			}
			prCfaTs->rVidCmdQPtsInfo.u4EntryNb = 0;
			prCfaTs->rVidCmdQPtsInfo.u4UsedEntryCnt = 0;
			prCfaTs->fgFillAu = FALSE;
		}

	} else
	#endif
	{
		if (0 == prCfaTs->u4VidCmdQIndex) {
			u32 i = 0;

			/*backup pts info*/
			prCfaTs->rVidCmdQPtsInfoBak.u4EntryNb = 0;
			prCfaTs->rVidCmdQPtsInfoBak.u4UsedEntryCnt = 0;
			for (i = 0; i < prCfaTs->rVidCmdQPtsInfo.u4EntryNb; i++) {
				if (PTS_VALID == prCfaTs->rVidCmdQPtsInfo.aruEntryInfo[i].eState) {
					prCfaTs->rVidCmdQPtsInfoBak
						.aruEntryInfo[prCfaTs->rVidCmdQPtsInfoBak.u4EntryNb].u8FileOffset
							= prCfaTs->rVidCmdQPtsInfo.aruEntryInfo[i].u8FileOffset;
					prCfaTs->rVidCmdQPtsInfoBak
						.aruEntryInfo[prCfaTs->rVidCmdQPtsInfoBak.u4EntryNb].u8Pts
							= prCfaTs->rVidCmdQPtsInfo.aruEntryInfo[i].u8Pts;
					prCfaTs->rVidCmdQPtsInfoBak
						.aruEntryInfo[prCfaTs->rVidCmdQPtsInfoBak.u4EntryNb].eState
							= PTS_VALID;
					prCfaTs->rVidCmdQPtsInfoBak.u4EntryNb++;
				}
			}
			prCfaTs->rVidCmdQPtsInfo.u4EntryNb = 0;
			prCfaTs->rVidCmdQPtsInfo.u4UsedEntryCnt = 0;

#ifdef BSP_WIFI_WFD
			prCfaTs->fgFillAu = FALSE;
#endif
		}
	}
	if (!prPrivate->fgAnaPesHeaderEnd) {
		/*RETAILMSG(1, (L"[CFA TS] ana vid pes head\n"));*/
		u4Len = CfaTsAnaPesHeader(prCfaTs, prPesHeader, pucData, u4DataLen);
		if (CFA_TS_ANA_PES_HEADER_ERROR == u4Len) {
			if (DATA_SOURCE_FILE == prCfaTs->eDataSource)
				CfaTsFinishPrs(prPrivate->pvSptHdl, prCfaTs);
			else if ((DATA_SOURCE_STREAM == prCfaTs->eDataSource)
				#ifdef BSP_WIFI_WFD
				|| (DATA_SOURCE_WFD == prCfaTs->eDataSource)
				#endif
				) {
				prCfaTs->u8ParseOffset += u4DataLen;
			} else {
					CfaTsFinishPrs(prPrivate->pvSptHdl, prCfaTs);
			}

			return;
		}
		if (prCfaTs->fgVidSeek && (INVALID_TIMESTAMP == prPesHeader->u8Pts)) {
			prCfaTs->u8ParseOffset += u4DataLen;
			return;
		}
		if (prCfaTs->fgVidSeek && (INVALID_TIMESTAMP != prPesHeader->u8Pts)) {
			prCfaTs->fgVidSeek = FALSE;
		}

		if (u4Len > 0) {
			/*RETAILMSG(1, (L"[CFA TS] VIDEO pes pts: 0x%08x%08x\n",
			(u32)(prPesHeader->u8Pts>>32), (u32)prPesHeader->u8Pts));*/
			CfaTsCheckVidPesPts(prCfaTs, prPesHeader);

			prPrivate->fgAnaPesHeaderEnd = TRUE;

			prCfaTs->u8ParseOffset += (u4DataLen - u4Len);

			prCfaTs->rVidCmdQPtsInfo.aruEntryInfo[prCfaTs->rVidCmdQPtsInfo.u4EntryNb].u8FileOffset
				= prCfaTs->u8ParseOffset;
			prCfaTs->rVidCmdQPtsInfo.aruEntryInfo[prCfaTs->rVidCmdQPtsInfo.u4EntryNb].u8Pts
				= prPesHeader->u8Pts;
			prCfaTs->rVidCmdQPtsInfo.aruEntryInfo[prCfaTs->rVidCmdQPtsInfo.u4EntryNb].eState
				= PTS_VALID;

			prCfaTs->rVidCmdQPtsInfo.u4EntryNb++;

		}
	} else {
		u4Len = u4DataLen;
	}


	if (u4Len > 0) {
		if (prCfaTs->u4PacketBufLen == prCfaTs->u4TsPacketSize) {
			prCfaTs->eCurPrsStm = CFA_TS_PRS_STRM_TYPE_V;

			/*tx a ts packet data which cross pbbuf slot*/
			prCfaTs->rVidInf.fgUseCmdQ = FALSE;
			prCfaTs->rVidInf.eVidType = prPrivate->eVCodec;
			prCfaTs->rVidInf.u4PrsStrmId = prCfaTs->u4AudioStreamID;
			prCfaTs->rVidInf.u8Len = (u64)u4Len;
			prCfaTs->rVidInf.u8TotalAULen = (u64)u4Len;

			Spt4CfaBuf2VFifoAUCtrl(prCfaTs->pvSptHdl,
				prCfaTs->pucPacketBuf + TS_COM_PKT_SIZE_188 - u4Len, &prCfaTs->rVidInf);
			prCfaTs->fgStopHandlePkt = TRUE;
			prPesHeader->u4PesPacketDataLen = 0;
		} else {
			if ((prCfaTs->u4VidCmdQIndex == DMX_MAX_TX_CNT_FOR_CMD_Q - 1)
				|| ((prCfaTs->u8ParseOffset + u4Len - prCfaTs->u8VidFirstOffset)
				> DMX_VID_TX_MAX_SIZE)
				|| (prCfaTs->u4MemDataLen < (2 * prCfaTs->u4TsPacketSize))) {
				/*fill the last cmdq entry and tx cmdq*/
				prCfaTs->eCurPrsStm = CFA_TS_PRS_STRM_TYPE_V;

				if (prCfaTs->u4VidCmdQIndex == 0) {
					prCfaTs->u8VidDataInBuf = 0;
					prCfaTs->u8VidPreTxEndOff = prCfaTs->u8ParseOffset;
					prCfaTs->u8VidFirstOffset = prCfaTs->u8ParseOffset;
				}

				if ((prCfaTs->u8ParseOffset + u4Len - prCfaTs->u8VidFirstOffset) <=
						DMX_VID_TX_MAX_SIZE) {
					prCfaTs->rVidInf.parCmdQTxEntry[prCfaTs->u4VidCmdQIndex].u4TxOfst =
						(u32)(prCfaTs->u8ParseOffset - prCfaTs->u8VidPreTxEndOff);
					prCfaTs->rVidInf.parCmdQTxEntry[prCfaTs->u4VidCmdQIndex].u4TxLen =
						u4Len;
					prCfaTs->u8VidPreTxEndOff = prCfaTs->u8ParseOffset + u4Len;

					prCfaTs->u4VidCmdQIndex += 1;
					prCfaTs->u8VidDataInBuf += u4Len;
				}

				prCfaTs->rVidInf.u8FileOfst = prCfaTs->u8VidFirstOffset;
				prCfaTs->rVidInf.eTxMode = CFA_PTM_EXACT_POS;
				prCfaTs->rVidInf.eVidType = prPrivate->eVCodec;
				prCfaTs->rVidInf.u8Len = prCfaTs->u8VidPreTxEndOff - prCfaTs->u8VidFirstOffset;
				prCfaTs->rVidInf.u4PrsStrmId = prCfaTs->u4VideoStreamID;
				prCfaTs->rVidInf.fgUseCmdQ = TRUE;
				prCfaTs->rVidInf.u2TxEntryCnt = (u16)prCfaTs->u4VidCmdQIndex;
				prCfaTs->rVidInf.u8RealTxLen = prCfaTs->u8VidDataInBuf;
				prCfaTs->rVidInf.fgUnitStart = FALSE;
				prCfaTs->rVidInf.fgAUCompleteByEnd = FALSE;


				mrRet = Spt4CfaPbb2VFifoAUCtrl(prCfaTs->pvSptHdl, &prCfaTs->rVidInf);
				if (RET_DMX_OK != mrRet) {
					CfaTsFinishPrs(prCfaTs->pvSptHdl, prCfaTs);
				}
				prCfaTs->u4VidCmdQIndex = 0;
				prCfaTs->u8VidDataInBuf = 0;

				if ((prCfaTs->u8ParseOffset + u4Len - prCfaTs->u8VidFirstOffset)
					> DMX_VID_TX_MAX_SIZE)
					CfaTsVidFillCmdQ(prCfaTs, u4Len);

				prCfaTs->fgStopHandlePkt = TRUE;
			} else {
				CfaTsVidFillCmdQ(prCfaTs, u4Len);
			}
		}

		prCfaTs->u8ParseOffset += u4Len;

		return;
	}

	prCfaTs->u8ParseOffset += u4DataLen;
}

void CfaTsBufVidPesCb(CfaTsPidFilter_T *prPidFilter, u8 *pucData, u32 u4DataLen)
{
	CfaTsVidPrivate_T *prPrivate = (CfaTsVidPrivate_T *)prPidFilter->pvPrivate;
	CfaTsPesHeader_T *prPesHeader = &prPrivate->rPesHeader;
	CfaTsInst *prCfaTs = prPrivate->prCfaTs;
	u32 u4Len = 0;


	if (prCfaTs->fgVidSeek) {
		if (!prPidFilter->fgUnitStart) {
			prCfaTs->u8ParseOffset += u4DataLen;

			return;
		}
		prCfaTs->fgVidSeek = FALSE;
	}

	if (prPidFilter->fgUnitStart) {
#if 1
		if ((prPrivate->u4DataLen > 0) && (0 == prPesHeader->u4PesPacketRealLen)) {
			CFA_VIDEO_INFO_T rVidInfo = {0};
			u32 u4Ret = 0;

			prCfaTs->eCurPrsStm = CFA_TS_PRS_STRM_TYPE_V;

			rVidInfo.eVidType = prPrivate->eVCodec;
			rVidInfo.u8Len = prPrivate->u4DataLen;
			rVidInfo.u4PrsStrmId = prCfaTs->u4VideoStreamID;
			rVidInfo.u8FileOfst = prPrivate->u8FileOffset;

#if CFG_SUPPORT_HDCP
			if ((DATA_SOURCE_WFD == prCfaTs->eDataSource) &&
				(prPesHeader->fgHDCP && prCfaTs->fgHDCP)) {
				HDCP2X_DEC_UNIT rtDhcp2xDecInfo = {0};

				rtDhcp2xDecInfo.strCounter = prPesHeader->u4StreamCounter;
				rtDhcp2xDecInfo.inputCounter = prPesHeader->u8InputCounter;
				rtDhcp2xDecInfo.srcFrame = prPrivate->pucDataBuf;
				rtDhcp2xDecInfo.len = prPrivate->u4DataLen;
				rtDhcp2xDecInfo.desFrame = prPrivate->pucDataBuf;

#if 0
			{
				unsigned int i;

				RETAILMSG(1, (L"before decrypt vid data:\n"));
				for (i = 0; i < prPrivate->u4DataLen; i++) {
					RETAILMSG(1, (L"%02x ", prPrivate->pucDataBuf[i]));
					if (!((i+1)%16) && i != 0)
						RETAILMSG(1, (L"\n"));
				}
				RETAILMSG(1, (L"\n"));
			}
#endif
				DeviceIoControl(prCfaTs->hHdcpDrv, HDCP_IOCTL_DECRYPTION,
					&rtDhcp2xDecInfo, sizeof(rtDhcp2xDecInfo),
					NULL, 0, NULL, NULL);

				 /*RETAILMSG(1, (L"rVidInfo.u4PrsStrmId: 0x%x\n", rVidInfo.u4PrsStrmId));*/
			}
#endif

			u4Ret = Spt4CfaBuf2VFifoAUCtrl(prPrivate->pvSptHdl, prPrivate->pucDataBuf, &rVidInfo);
			if (RET_DMX_OK != u4Ret) {
				CfaTsFinishPrs(prCfaTs->pvSptHdl, prCfaTs);
			}
			prCfaTs->fgStopHandlePkt = TRUE;
			prPesHeader->eState = CFA_TS_ST_PES_MIN_HEADER;
			prPesHeader->u4HeaderLen = 0;
			prPrivate->u4DataLen = 0;
			prPesHeader->u4PesPacketDataLen = 0;
			prPesHeader->u4PesPacketRealLen = 0;
			prPidFilter->fgNeedRollback = TRUE;
			return;
		}
#endif
		prPesHeader->eState = CFA_TS_ST_PES_MIN_HEADER;
		prPesHeader->u4HeaderLen = 0;
		prPrivate->u4DataLen = 0;
		prPesHeader->u4PesPacketDataLen = 0;
		prPesHeader->u4PesPacketRealLen = 0;

#if CFG_SUPPORT_HDCP
		prPesHeader->fgHDCP = FALSE;
#endif
	}

	/*gather pes packet data byte*/
	if (0 == prPrivate->u4DataLen) {
		u4Len = CfaTsAnaPesHeader(prCfaTs, prPesHeader, pucData, u4DataLen);
		if (CFA_TS_ANA_PES_HEADER_ERROR == u4Len) {
			if (DATA_SOURCE_FILE == prCfaTs->eDataSource)
				CfaTsFinishPrs(prPrivate->pvSptHdl, prCfaTs);
			else if (DATA_SOURCE_STREAM == prCfaTs->eDataSource)
				prCfaTs->u8ParseOffset += u4DataLen;
			 else
                		CfaTsFinishPrs(prPrivate->pvSptHdl, prCfaTs);
			return;
		}


		if ((u4Len > 0) && (CFA_TS_ST_PES_PAYLOD == prPesHeader->eState)) {
			if ((PTS_INVALID == prCfaTs->rVidBufPts.eState) &&
				(INVALID_TIMESTAMP != prPesHeader->u8Pts)) {
				prCfaTs->rVidBufPts.u8Pts = prPesHeader->u8Pts;
				prCfaTs->rVidBufPts.eState = PTS_VALID;
			}

			/*we get the paylod for the first time, start to save payload*/
                       if ((DMX_MIN(u4Len, prPesHeader->u4PesPacketDataLen) > prPrivate->u4BufLen) 
				|| (u4Len > u4DataLen)){
                                CfaTsFinishPrs(prPrivate->pvSptHdl, prCfaTs);
				return;
			}
			dmx_memcpy(prPrivate->pucDataBuf, pucData + u4DataLen - u4Len,
				DMX_MIN(u4Len, prPesHeader->u4PesPacketDataLen));
			prPrivate->u4DataLen += DMX_MIN(u4Len, prPesHeader->u4PesPacketDataLen);
			prPrivate->u8FileOffset = prCfaTs->u8ParseOffset + u4DataLen - u4Len;

		   CfaTsCheckVidPesPts(prCfaTs, prPesHeader);
		}
	} else {
		/*save payload*/
		u4Len = DMX_MIN(u4DataLen, prPesHeader->u4PesPacketDataLen - prPrivate->u4DataLen);
		if ((prPrivate->u4DataLen + u4Len) < prPrivate->u4BufLen) {
			if (((prPrivate->u4DataLen + u4Len) > prPrivate->u4BufLen) 
				|| (u4Len > u4DataLen)){
				CfaTsFinishPrs(prPrivate->pvSptHdl, prCfaTs);
				return;
			}
			dmx_memcpy(prPrivate->pucDataBuf + prPrivate->u4DataLen,
				pucData + u4DataLen - u4Len, u4Len);
			prPrivate->u4DataLen += u4Len;
		}
	}
#if 1
	/*if a whole pes packet data byte has been gathered, send it to vfifo*/
	if ((prPesHeader->u4PesPacketDataLen == prPrivate->u4DataLen)
			&& (prPrivate->u4DataLen > 0)) {
		CFA_VIDEO_INFO_T rVidInfo = {0};
		u32 u4Ret = 0;

		prCfaTs->eCurPrsStm = CFA_TS_PRS_STRM_TYPE_V;

		rVidInfo.eVidType = prPrivate->eVCodec;
		rVidInfo.u8Len = prPrivate->u4DataLen;
		rVidInfo.u4PrsStrmId = prCfaTs->u4VideoStreamID;
#if CFG_SUPPORT_HDCP
		if ((DATA_SOURCE_WFD == prCfaTs->eDataSource) &&
			(prPesHeader->fgHDCP && prCfaTs->fgHDCP)) {
			HDCP2X_DEC_UNIT rtDhcp2xDecInfo = {0};

			rtDhcp2xDecInfo.strCounter = prPesHeader->u4StreamCounter;
			rtDhcp2xDecInfo.inputCounter = prPesHeader->u8InputCounter;
			rtDhcp2xDecInfo.srcFrame = prPrivate->pucDataBuf;
			rtDhcp2xDecInfo.len = prPrivate->u4DataLen;
			rtDhcp2xDecInfo.desFrame = prPrivate->pucDataBuf;

#if 0
			{
			unsigned int i;

			RETAILMSG(1, (L"before decrypt vid data:\n"));
			for (i = 0; i < prPrivate->u4DataLen; i++) {
				RETAILMSG(1, (L"%02x ", prPrivate->pucDataBuf[i]));
				if (!((i+1)%16) && i != 0)
					RETAILMSG(1, (L"\n"));
			}
			RETAILMSG(1, (L"\n"));
			}
#endif
			DeviceIoControl(prCfaTs->hHdcpDrv, HDCP_IOCTL_DECRYPTION,
				&rtDhcp2xDecInfo, sizeof(rtDhcp2xDecInfo),
				NULL, 0, NULL, NULL);

#if 0
			{
			unsigned int i;

			RETAILMSG(1, (L"after decrypt vid data:\n"));
			for (i = 0; i < prPrivate->u4DataLen; i++) {
				RETAILMSG(1, (L"%02x ", prPrivate->pucDataBuf[i]));
				if (!((i+1)%16) && i != 0)
					RETAILMSG(1, (L"\n"));
			}
			RETAILMSG(1, (L"\n"));
			}
#endif
			 /*RETAILMSG(1, (L"rVidInfo.u4PrsStrmId: 0x%x\n", rVidInfo.u4PrsStrmId));*/
		}
#endif


		u4Ret = Spt4CfaBuf2VFifoAUCtrl(prPrivate->pvSptHdl, prPrivate->pucDataBuf, &rVidInfo);
		if (RET_DMX_OK != u4Ret) {
			CfaTsFinishPrs(prCfaTs->pvSptHdl, prCfaTs);
		}
		prCfaTs->fgStopHandlePkt = TRUE;
		prPrivate->u4DataLen = 0;
	}
#endif
	prCfaTs->u8ParseOffset += u4DataLen;

}



static void CfaTsClearAPtsState(CfaTsInst *prCfaTs)
{
	prCfaTs->u8OldLastPesAudPts = 0;
	prCfaTs->u8OldCurPesAudPts = 0;
	prCfaTs->u4PesAudPtsDecCnt = 0;
	prCfaTs->u4PesAudPtsIncCnt = 0;
	prCfaTs->fgNeedCheckAPtsDec = FALSE;
	prCfaTs->fgNeedCheckAPtsInc = FALSE;
}

static void CfaCheckAPtcRealDec(CfaTsInst *prCfaTs)
{
	if (prCfaTs->u8CurPesAudPts < prCfaTs->u8OldLastPesAudPts) {
		if (prCfaTs->u4PesAudPtsDecCnt < CFA_TS_MAX_PTS_RECORD_LEN) {
			prCfaTs->au8Rec4CorrectAPts[prCfaTs->u4PesAudPtsDecCnt]
				= prCfaTs->u8CurPesAudPts;
			DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsCheckAPtsState():")
				TEXT("au8Rec4CorrectAPts[%d]: 0x%08x%08x\r\n"),
				prCfaTs->u4PesAudPtsDecCnt,
				(u32)(prCfaTs->au8Rec4CorrectAPts[prCfaTs->u4PesAudPtsDecCnt] >> 32),
				(u32)(prCfaTs->au8Rec4CorrectAPts[prCfaTs->u4PesAudPtsDecCnt]));
		}

		prCfaTs->u4PesAudPtsDecCnt += 1;

		/*get pcr change ?*/
		if (prCfaTs->u4PesAudPtsDecCnt >= CFA_TS_MIN_PTS_CHG_CNT) {

			if (prCfaTs->u4AudPesPtsInconsecutiveCnt
					< CFA_TS_MAX_PES_PTS_NOT_CONSECUTIVE_CNT) {
				if (CfaTsCheckConsecutivePts(prCfaTs->au8Rec4CorrectAPts
						, CFA_TS_MIN_PTS_CHG_CNT))
					prCfaTs->eAPtsState = APTS_PCR_CHANGE;

				else {
					prCfaTs->u4AudPesPtsInconsecutiveCnt += 1;
					DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
						TEXT("[CFA TS] A0")
						TEXT("prCfaTs->u4AudPesPtsInconsecutiveCnt:")
						TEXT("%d\n"),
						prCfaTs->u4AudPesPtsInconsecutiveCnt);

					prCfaTs->eAPtsState = APTS_DATA_ERROR;
					CfaTsClearAPtsState(prCfaTs);
				}
			} else {
				if (CfaTsCheckFaintlyConsecutivePts(prCfaTs->au8Rec4CorrectAPts
						, CFA_TS_MIN_PTS_CHG_CNT))
					prCfaTs->eAPtsState = APTS_PCR_CHANGE;

				else {
					prCfaTs->u4AudPesPtsInconsecutiveCnt += 1;
					DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
						TEXT("[CFA TS] A01")
						TEXT("prCfaTs->u4AudPesPtsInconsecutiveCnt:%d\n"),
						prCfaTs->u4AudPesPtsInconsecutiveCnt);

					prCfaTs->eAPtsState = APTS_DATA_ERROR;
					CfaTsClearAPtsState(prCfaTs);
				}
			}
		} else {
			prCfaTs->eAPtsState = APTS_DATA_ERROR;
		}
	} else {
		CfaTsClearAPtsState(prCfaTs);
	}
}


static void CfaCheckAPtcRealInc(CfaTsInst *prCfaTs)
{
	if ((prCfaTs->u8CurPesAudPts > prCfaTs->u8OldCurPesAudPts)
		&& (prCfaTs->u8CurPesAudPts > prCfaTs->u8LastPesAudPts)) {
		if (prCfaTs->u4PesAudPtsIncCnt < CFA_TS_MIN_PTS_CHG_CNT) {
			prCfaTs->au8Rec4CorrectAPts[prCfaTs->u4PesAudPtsIncCnt]
				= prCfaTs->u8CurPesAudPts;
			DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsCheckAPtsState():")
				TEXT("au8Rec4CorrectAPts[%d]: 0x%08x%08x\r\n"),
				prCfaTs->u4PesAudPtsIncCnt,
				(u32)(prCfaTs->au8Rec4CorrectAPts[prCfaTs->u4PesAudPtsIncCnt] >> 32),
				(u32)(prCfaTs->au8Rec4CorrectAPts[prCfaTs->u4PesAudPtsIncCnt]));
		}
		prCfaTs->u4PesAudPtsIncCnt += 1;

		/*get pcr change ?*/
		if (prCfaTs->u4PesAudPtsIncCnt >= CFA_TS_MIN_PTS_CHG_CNT) {
			if (prCfaTs->u4AudPesPtsInconsecutiveCnt
				< CFA_TS_MAX_PES_PTS_NOT_CONSECUTIVE_CNT) {
				if (CfaTsCheckConsecutivePts(prCfaTs->au8Rec4CorrectAPts
					, CFA_TS_MIN_PTS_CHG_CNT))
					prCfaTs->eAPtsState = APTS_PCR_CHANGE;

				else {
					prCfaTs->u4AudPesPtsInconsecutiveCnt += 1;
					DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
						TEXT("[CFA TS] A1")
						TEXT("prCfaTs->u4AudPesPtsInconsecutiveCnt: %d\n"),
						prCfaTs->u4AudPesPtsInconsecutiveCnt);

					prCfaTs->eAPtsState = APTS_DATA_ERROR;
					CfaTsClearAPtsState(prCfaTs);
				}
			} else {
				if (CfaTsCheckFaintlyConsecutivePts(prCfaTs->au8Rec4CorrectAPts
					, CFA_TS_MIN_PTS_CHG_CNT))
					prCfaTs->eAPtsState = APTS_PCR_CHANGE;

				else {
					prCfaTs->u4AudPesPtsInconsecutiveCnt += 1;
					DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
						TEXT("[CFA TS] A11")
						TEXT("prCfaTs->u4AudPesPtsInconsecutiveCnt: %d\n"),
						prCfaTs->u4AudPesPtsInconsecutiveCnt);

					prCfaTs->eAPtsState = APTS_DATA_ERROR;
					CfaTsClearAPtsState(prCfaTs);
				}
			}
		} else {
			prCfaTs->eAPtsState = APTS_DATA_ERROR;
		}
	} else {
		CfaTsClearAPtsState(prCfaTs);
	}
}


static void CfaTsCheckAPtsState(CfaTsInst *prCfaTs, CfaTsPesHeader_T *prPesHeader)
{
	if (prCfaTs->fgIsISDBT1Seg || (!prCfaTs->fgIsISDBT1Seg && (INVALID_TIMESTAMP != prCfaTs->u8CurPesAudPts)))
		prCfaTs->u8LastPesAudPts = prCfaTs->u8CurPesAudPts;

	prCfaTs->u8CurPesAudPts = prPesHeader->u8Pts;

	if (INVALID_TIMESTAMP == prPesHeader->u8Pts) {
		prCfaTs->eAPtsState = APTS_DATA_ERROR;
		CfaTsClearAPtsState(prCfaTs);

		return;
	}

#ifdef BSP_WIFI_WFD
	if ((prCfaTs->u4NoAudTicCnt > NO_AUD_TICK_COUNT)
		&& (DATA_SOURCE_WFD == prCfaTs->eDataSource)) {
		prCfaTs->eAPtsState = APTS_NO_SIGNAL;
		prCfaTs->u4NoAudTicCnt = 0;

		return;
	}
#endif

	if ((prCfaTs->u4NoAudTicCnt > NO_AUD_TICK_COUNT)
		&& (DATA_SOURCE_STREAM == prCfaTs->eDataSource)) {
		prCfaTs->eAPtsState = APTS_NO_SIGNAL;
		prCfaTs->u4NoAudTicCnt = 0;

		return;
	}
	prCfaTs->u4NoAudTicCnt = 0;

	/*get and check tick count*/
	prCfaTs->u4LastAudTicCnt = prCfaTs->u4CurAudTicCnt;
	prCfaTs->u4CurAudTicCnt = GetTickCount();

	if ((prCfaTs->u4LastAudTicCnt > 0)
		&& (prCfaTs->u4CurAudTicCnt > (prCfaTs->u4LastAudTicCnt + NO_SIGNAL_TICK_COUNT))
		&& ((DATA_SOURCE_STREAM == prCfaTs->eDataSource)
			#ifdef BSP_WIFI_WFD
			|| (DATA_SOURCE_WFD == prCfaTs->eDataSource)
			#endif
			)) {
		DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsCheckAPtsState(): u4CurAudTicCnt: %d, u4LastAudTicCnt: %d\n"),
			prCfaTs->u4CurAudTicCnt, prCfaTs->u4LastAudTicCnt);
		prCfaTs->eAPtsState = APTS_NO_SIGNAL;

		return;
	}

	if (prCfaTs->fgFirstGetAudPts) {
		prCfaTs->eAPtsState = APTS_OK;
		prCfaTs->fgFirstGetAudPts = FALSE;

		/*if the first audio pts is too smaller than the first video pts, adjust it*/
		if ((INVALID_TIMESTAMP != prCfaTs->u8VidBasePts)
			&& (prCfaTs->u8VidBasePts > (prPesHeader->u8Pts + MAX_GAP_PTS_BETWEEN_AV))) {
			DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsCheckAPtsState():")
				TEXT("First audio pes pts is too small-> u8Pts:")
				TEXT("0x%08x%08x, u8VidBasePts: 0x%08x%08x\r\n"),
				(u32)(prPesHeader->u8Pts >> 32), (u32)(prPesHeader->u8Pts),
				(u32)(prCfaTs->u8VidBasePts >> 32), (u32)(prCfaTs->u8VidBasePts));
			prPesHeader->u8Pts = prCfaTs->u8VidBasePts;
		}

		prCfaTs->u8AudBasePts = prPesHeader->u8Pts;
		prCfaTs->u8CurPesAudPts = prPesHeader->u8Pts;
		DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsCheckAPtsState():")
			TEXT("prCfaTs->u8AudBasePts: 0x%08x%08x\r\n"),
			(u32)(prCfaTs->u8AudBasePts >> 32), (u32)(prCfaTs->u8AudBasePts));

	} else {
		if (prCfaTs->u8CurPesAudPts < prCfaTs->u8LastPesAudPts) {
			prCfaTs->u4PesAudPtsIncCnt = 0;
			prCfaTs->fgNeedCheckAPtsInc = FALSE;

			prCfaTs->eAPtsState = APTS_DATA_ERROR;
			if (0 == prCfaTs->u4PesAudPtsDecCnt) {
				prCfaTs->u8OldLastPesAudPts = prCfaTs->u8LastPesAudPts;
				prCfaTs->u8OldCurPesAudPts = prCfaTs->u8CurPesAudPts;
				prCfaTs->fgNeedCheckAPtsDec = TRUE;
			}
		} else {
			prCfaTs->u4PesAudPtsDecCnt = 0;
			prCfaTs->fgNeedCheckAPtsDec = FALSE;

			if ((prCfaTs->u8LastPesAudPts > 0)
				&& (prCfaTs->u8CurPesAudPts > (prCfaTs->u8LastPesAudPts + 5 * PTS_1S))) {
				prCfaTs->eAPtsState = APTS_DATA_ERROR;

				if (0 == prCfaTs->u4PesAudPtsIncCnt) {
					prCfaTs->u8OldCurPesAudPts = prCfaTs->u8CurPesAudPts;
					prCfaTs->u8OldLastPesAudPts = prCfaTs->u8LastPesAudPts;
					prCfaTs->fgNeedCheckAPtsInc = TRUE;

					if (prCfaTs->u4PesAudPtsIncCnt < CFA_TS_MIN_PTS_CHG_CNT) {
						prCfaTs->au8Rec4CorrectAPts[prCfaTs->u4PesAudPtsIncCnt]
							= prCfaTs->u8CurPesAudPts;
						DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
						TEXT("[CFA TS]CfaTsCheckAPtsState():")
						TEXT("au8Rec4CorrectAPts[%d]: 0x%08x%08x\r\n"),
						prCfaTs->u4PesAudPtsIncCnt,
						(u32)(prCfaTs->au8Rec4CorrectAPts[prCfaTs->u4PesAudPtsIncCnt]
							>> 32),
						(u32)(prCfaTs->au8Rec4CorrectAPts[prCfaTs->u4PesAudPtsIncCnt]));
					}
					prCfaTs->u4PesAudPtsIncCnt += 1;

					return;
				}
			} else {
				prCfaTs->eAPtsState = APTS_OK;
			}
		}


		/*check if the pts real decrease*/
		if (prCfaTs->fgNeedCheckAPtsDec)
			CfaCheckAPtcRealDec(prCfaTs);

		/*check if the pts real increase*/
		if (prCfaTs->fgNeedCheckAPtsInc)
			CfaCheckAPtcRealInc(prCfaTs);
	}
}

static void CfaTsCheckAudPesPts(CfaTsInst *prCfaTs, CfaTsPesHeader_T *prPesHeader)
{
	/*get delta Audio Pes Pts*/
	if (prCfaTs->rAudDeltaPesPtsInfo.fgNeedGet && prCfaTs->fgIsISDBT1Seg)
		CfaTsGetDeltaPesPts(&prCfaTs->rAudDeltaPesPtsInfo, prPesHeader->u8Pts);

	/*check pts state*/
	CfaTsCheckAPtsState(prCfaTs, prPesHeader);

	switch (prCfaTs->eAPtsState) {
	case APTS_DATA_ERROR:
		if (prCfaTs->fgIsISDBT1Seg) {
			DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsCheckAudPesPts():")
				TEXT("+++++++++++++++++++++++++++	APTS_DATA_ERROR\n"));

			prPesHeader->u8Pts = prCfaTs->u8LastPesAudPts
				+ prCfaTs->rAudDeltaPesPtsInfo.u8DeltaPesPts;
			prCfaTs->u8CurPesAudPts = prPesHeader->u8Pts;
#if CFG_SHOW_LOG
			DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsCheckAudPesPts():")
			TEXT("u8CurPesAudPts: 0x%08x%08x, u8LastPesAudPts:")
			TEXT("0x%08x%08x, u8DeltaAPesPts:")
			TEXT("0x%08x%08x, au Pts: 0x%08x%08x, basepts:")
			TEXT("0x%08x%08x\n"),
				(u32)(prCfaTs->u8CurPesAudPts >> 32),
				(u32)(prCfaTs->u8CurPesAudPts),
				(u32)(prCfaTs->u8LastPesAudPts >> 32),
				(u32)(prCfaTs->u8LastPesAudPts),
				(u32)(prCfaTs->rAudDeltaPesPtsInfo.u8DeltaPesPts >> 32),
				(u32)(prCfaTs->rAudDeltaPesPtsInfo.u8DeltaPesPts),
				(u32)((prCfaTs->u8CurPesAudPts +
					prCfaTs->u8PtsIncrement - prCfaTs->u8AudBasePts) >> 32),
				(u32)(prCfaTs->u8CurPesAudPts +
					prCfaTs->u8PtsIncrement - prCfaTs->u8AudBasePts),
				(u32)(prCfaTs->u8AudBasePts >> 32),
				(u32)(prCfaTs->u8AudBasePts));
#endif
		}
		break;

	case APTS_PCR_CHANGE:
		DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT(" [CFA TS]CfaTsCheckAudPesPts():")
			TEXT("+++++++++++++++++ APTS_PCR_CHANGE\n"));
#if CFG_SHOW_LOG
	DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
	TEXT("[CFA TS]CfaTsCheckAudPesPts():")
	TEXT("u8CurPesAudPts: 0x%08x%08x, u8LastPesAudPts:")
	TEXT("0x%08x%08x, u8DeltaAPesPts: 0x%08x%08x\r\n"),
		(u32)(prCfaTs->u8CurPesAudPts >> 32),
		(u32)(prCfaTs->u8CurPesAudPts),
		(u32)(prCfaTs->u8LastPesAudPts >> 32),
		(u32)(prCfaTs->u8LastPesAudPts),
		(u32)(prCfaTs->rAudDeltaPesPtsInfo.u8DeltaPesPts >> 32),
		(u32)(prCfaTs->rAudDeltaPesPtsInfo.u8DeltaPesPts));
#endif
		prCfaTs->u8PtsIncrement +=
			prCfaTs->u8LastPesAudPts + prCfaTs->rAudDeltaPesPtsInfo.u8DeltaPesPts - prCfaTs->u8AudBasePts;
		prCfaTs->u8AudBasePts = prPesHeader->u8Pts;

		DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsCheckAudPesPts():")
			TEXT("u8PtsIncrement: 0x%08x%08x, u8AudBasePts:")
			TEXT("0x%08x%08x\r\n"),
				(u32)(prCfaTs->u8PtsIncrement >> 32),
				(u32)(prCfaTs->u8PtsIncrement),
				(u32)(prCfaTs->u8AudBasePts >> 32),
				(u32)(prCfaTs->u8AudBasePts));

		CfaTsClearAPtsState(prCfaTs);

		prCfaTs->u4VidCmdQIndex = 0;
		prCfaTs->u8VidDataInBuf = 0;
		prCfaTs->u4AudCmdQIndex = 0;
		prCfaTs->u8AudDataInBuf = 0;
		prCfaTs->rAudCmdQPtsInfo.u4EntryNb = 0;
		prCfaTs->rVidCmdQPtsInfo.u4EntryNb = 0;
		prCfaTs->fgPcrChange = TRUE;
		prCfaTs->u4AudPesPtsInconsecutiveCnt = 0;

		break;

	case APTS_NO_SIGNAL:
		DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT(" [CFA TS]CfaTsCheckAudPesPts():")
			TEXT("+++++++++++++++++   APTS_NO_SIGNAL\n"));

		if ((DATA_SOURCE_STREAM == prCfaTs->eDataSource)
			#ifdef BSP_WIFI_WFD
			|| (DATA_SOURCE_WFD == prCfaTs->eDataSource)
			#endif
			) {
			Spt4CfaClearAllStmData(prCfaTs->pvSptHdl);
			STC_HalGetTime(0, &prCfaTs->u8PtsIncrement);
			if (DATA_SOURCE_WFD == prCfaTs->eDataSource)
				prCfaTs->u8PtsIncrement += 40 * 90;

			if ((1 == ((prCfaTs->u8PtsIncrement >> 32) & 0x0000000000000001))
				&& (prCfaTs->u8PtsIncrement > 0x1FFFE0000LL)) {
				DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsCheckAudPesPts():")
				TEXT("STC_HalGetTime get prCfaTs->u8PtsIncrement ERROR: %lld\r\n"),
					(prCfaTs->u8PtsIncrement)*(1000/9));

				prCfaTs->u8PtsIncrement = 0;
			}

			prCfaTs->u8AudBasePts = prPesHeader->u8Pts;
		} else {
			prCfaTs->u8AudBasePts += prCfaTs->u8CurPesAudPts
				- prCfaTs->u8OldLastPesAudPts + prCfaTs->rAudDeltaPesPtsInfo.u8DeltaPesPts;
		}
#if CFG_SHOW_LOG
		DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
		TEXT("[CFA TS]CfaTsCheckAudPesPts():")
		TEXT("u8PtsIncrement: 0x%08x%08x, u8AudBasePts:")
		TEXT("0x%08x%08x\r\n"),
			(u32)(prCfaTs->u8PtsIncrement >> 32),
			(u32)(prCfaTs->u8PtsIncrement),
			(u32)(prCfaTs->u8AudBasePts >> 32),
			(u32)(prCfaTs->u8AudBasePts));
#endif
		CfaTsClearAPtsState(prCfaTs);

		prCfaTs->u4VidCmdQIndex = 0;
		prCfaTs->u8VidDataInBuf = 0;
		prCfaTs->u4AudCmdQIndex = 0;
		prCfaTs->u8AudDataInBuf = 0;
		prCfaTs->rAudCmdQPtsInfo.u4EntryNb = 0;
		prCfaTs->rVidCmdQPtsInfo.u4EntryNb = 0;
		prCfaTs->fgNoSignal = TRUE;
		prCfaTs->u4AudPesPtsInconsecutiveCnt = 0;

		break;

	case APTS_OK:
		prCfaTs->u4AudPesPtsInconsecutiveCnt = 0;
		break;

	default:
		break;
	}

}

static void CfaTsAudCmdQBackupLastPesPacket(CfaTsInst *prCfaTs)
{
	u32 u4PtsEntry = prCfaTs->rAudCmdQPtsInfo.u4EntryNb;
	u32 i = 0;
	u32 u4LastEntryCmdQIndex = 0;

	if ((u4PtsEntry < 1) ||
		((1 == u4PtsEntry)
		&& (0 == prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[u4PtsEntry - 1].u4CmdQIndex))) {
		prCfaTs->rAudCmdQBakInfo.rAudCmdQPtsInfo.u4EntryNb = 0;
		prCfaTs->rAudCmdQBakInfo.u4AudCmdQIndex = 0;
		prCfaTs->rAudCmdQBakInfo.u8AudDataInBuf = 0;

		return;
	}


	/*backup the part of a pes data in CMDQ*/
	prCfaTs->rAudCmdQBakInfo.rAudCmdQPtsInfo.aruEntryInfo[0].u8FileOffset
		= prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[u4PtsEntry - 1].u8FileOffset;
	prCfaTs->rAudCmdQBakInfo.rAudCmdQPtsInfo.aruEntryInfo[0].u8Pts
		= prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[u4PtsEntry - 1].u8Pts;
	prCfaTs->rAudCmdQBakInfo.rAudCmdQPtsInfo.aruEntryInfo[0].eState
		= prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[u4PtsEntry - 1].eState;
	prCfaTs->rAudCmdQBakInfo.rAudCmdQPtsInfo.aruEntryInfo[0].u4CmdQIndex = 0;
	prCfaTs->rAudCmdQBakInfo.rAudCmdQPtsInfo.aruEntryInfo[0].u8DataInBuf = 0;
	prCfaTs->rAudCmdQBakInfo.rAudCmdQPtsInfo.aruEntryInfo[0].u8PreTxEndOff = 0;
	prCfaTs->rAudCmdQBakInfo.rAudCmdQPtsInfo.u4EntryNb = 1;
	prCfaTs->rAudCmdQPtsInfo.u4EntryNb -= 1;

	u4LastEntryCmdQIndex
		= prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[u4PtsEntry - 1].u4CmdQIndex;
	for (i = 0; i < prCfaTs->u4AudCmdQIndex - u4LastEntryCmdQIndex; i++) {
		prCfaTs->rAudCmdQBakInfo.rAudInf.parCmdQTxEntry[i].u4TxOfst
			= prCfaTs->rAudInf.parCmdQTxEntry[i + u4LastEntryCmdQIndex].u4TxOfst;
		prCfaTs->rAudCmdQBakInfo.rAudInf.parCmdQTxEntry[i].u4TxLen
			= prCfaTs->rAudInf.parCmdQTxEntry[i + u4LastEntryCmdQIndex].u4TxLen;
		prCfaTs->rAudCmdQBakInfo.rAudInf.parCmdQTxEntry[i].fgEndAU
			= FALSE;
	}
	prCfaTs->rAudCmdQBakInfo.rAudInf.parCmdQTxEntry[0].u4TxOfst = 0;

	prCfaTs->rAudCmdQBakInfo.u4AudCmdQIndex
		= prCfaTs->u4AudCmdQIndex - u4LastEntryCmdQIndex;
	prCfaTs->rAudCmdQBakInfo.u8AudDataInBuf
		= prCfaTs->u8AudDataInBuf
		- prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[u4PtsEntry - 1].u8DataInBuf;

	/*updata will be txed data*/
	prCfaTs->rAudInf.u8RealTxLen
		= prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[u4PtsEntry - 1].u8DataInBuf;
	prCfaTs->rAudInf.u2TxEntryCnt
		= (u16)u4LastEntryCmdQIndex;
	prCfaTs->rAudInf.u8Len
		= prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[u4PtsEntry - 1].u8PreTxEndOff
		- prCfaTs->u8AudFirstOffset;
	prCfaTs->rAudInf.parCmdQTxEntry[prCfaTs->rAudInf.u2TxEntryCnt - 1].fgEndAU	= TRUE;

	prCfaTs->rAudCmdQBakInfo.fgBackup = TRUE;

}


void CfaTsResumeBak(CfaTsInst *prCfaTs)
{
	u32 i = 0;

	if (prCfaTs->rAudCmdQBakInfo.rAudCmdQPtsInfo.u4EntryNb < 1)
		return;

	/*resume cmdq entry*/
	for (i = 0; i < prCfaTs->rAudCmdQBakInfo.u4AudCmdQIndex; i++) {
		prCfaTs->rAudInf.parCmdQTxEntry[i].u4TxOfst
			= prCfaTs->rAudCmdQBakInfo.rAudInf.parCmdQTxEntry[i].u4TxOfst;
		prCfaTs->rAudInf.parCmdQTxEntry[i].u4TxLen
			= prCfaTs->rAudCmdQBakInfo.rAudInf.parCmdQTxEntry[i].u4TxLen;
		prCfaTs->rAudInf.parCmdQTxEntry[i].fgEndAU
			= FALSE;
	}

	prCfaTs->u4AudCmdQIndex = prCfaTs->rAudCmdQBakInfo.u4AudCmdQIndex;
	prCfaTs->u8AudFirstOffset
		= prCfaTs->rAudCmdQBakInfo.rAudCmdQPtsInfo.aruEntryInfo[0].u8FileOffset;
	prCfaTs->u8AudDataInBuf = prCfaTs->rAudCmdQBakInfo.u8AudDataInBuf;

	/*resume pts info*/
	prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[0].u8FileOffset
		= prCfaTs->rAudCmdQBakInfo.rAudCmdQPtsInfo.aruEntryInfo[0].u8FileOffset;
	prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[0].u8Pts
		= prCfaTs->rAudCmdQBakInfo.rAudCmdQPtsInfo.aruEntryInfo[0].u8Pts;
	prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[0].eState
		= prCfaTs->rAudCmdQBakInfo.rAudCmdQPtsInfo.aruEntryInfo[0].eState;

	prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[0].u4CmdQIndex = 0;
	prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[0].u8DataInBuf = 0;
	prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[0].u8PreTxEndOff = 0;

	prCfaTs->rAudCmdQPtsInfo.u4EntryNb = 1;

	/*clear backup info*/
	prCfaTs->rAudCmdQBakInfo.rAudCmdQPtsInfo.u4EntryNb = 0;
	prCfaTs->rAudCmdQBakInfo.u4AudCmdQIndex = 0;
	prCfaTs->rAudCmdQBakInfo.u8AudDataInBuf = 0;
	prCfaTs->rAudCmdQBakInfo.fgBackup = FALSE;

}


#if CFG_DISCARD_ERROR_PES
static void CfaTsDiscardErrPesCmdQEntry(CfaTsInst *prCfaTs)
{
	u32 u4EntryNb = 0;

	if (0 == prCfaTs->u4AudCmdQIndex)
		return;

	if (prCfaTs->rAudCmdQPtsInfo.u4EntryNb <= 1) {
		prCfaTs->u4AudCmdQIndex = 0;
		prCfaTs->u8AudDataInBuf = 0;
		prCfaTs->rAudCmdQPtsInfo.u4EntryNb = 0;

		return;
	}

	u4EntryNb = prCfaTs->rAudCmdQPtsInfo.u4EntryNb;
	prCfaTs->u4AudCmdQIndex
		= prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[u4EntryNb].u4CmdQIndex;
	prCfaTs->u8AudDataInBuf
		= prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[u4EntryNb].u8DataInBuf;
	prCfaTs->u8AudPreTxEndOff
		= prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[u4EntryNb].u8PreTxEndOff;
	prCfaTs->rAudCmdQPtsInfo.u4EntryNb -= 1;

	DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
		TEXT("[CFA TS]CfaTsDiscardErrPesCmdQEntry():")
		TEXT("discard some error audio data\n"));

}
#endif

void CfaTsAudPesCb(CfaTsPidFilter_T *prPidFilter, u8 *pucData, u32 u4DataLen)
{
	CfaTsAudPrivate_T *prPrivate = (CfaTsAudPrivate_T *)prPidFilter->pvPrivate;
	CfaTsPesHeader_T *prPesHeader = &prPrivate->rPesHeader;
	CfaTsInst *prCfaTs = prPrivate->prCfaTs;
	u32 u4Len = 0;
	MRESULT mrRet = RET_DMX_OK;

	prCfaTs->eDataType = DATA_A;


	if (prCfaTs->fgAudSeek || prPidFilter->fgTxDataFromUinitStart) {
		if (!prPidFilter->fgUnitStart) {
			prCfaTs->u8ParseOffset += u4DataLen;

			return;
		}
		prCfaTs->fgAudSeek = FALSE;
		prPidFilter->fgTxDataFromUinitStart = FALSE;
	}

	if (prPidFilter->fgUnitStart) {
		prPidFilter->u4CcErrCnt = 0;
		prPidFilter->fgCcOk = TRUE;

		prPesHeader->eState = CFA_TS_ST_PES_MIN_HEADER;
		prPesHeader->u4HeaderLen = 0;
		prPesHeader->u4PesPacketDataLen = 0;
		prPrivate->fgAnaPesHeaderEnd = FALSE;
		/*RETAILMSG(1, (L"[CFA TS] CfaTsAudPesCb fgUnitStart is true: u4EntryNb: %d\n",
		prCfaTs->rAudCmdQPtsInfo.u4EntryNb));*/
	}

#if CFG_DISCARD_ERROR_PES
	if (!prPidFilter->fgCcOk)
		prPidFilter->u4CcErrCnt += 1;

	/*continuty count error upto CFA_TS_MAX_CC_ERR_CNT_IN_PES, discard this pes*/
	if (prPidFilter->u4CcErrCnt >= CFA_TS_MAX_CC_ERR_CNT_IN_PES) {
		CfaTsDiscardErrPesCmdQEntry(prCfaTs);

		if (prCfaTs->u4AudCmdQIndex > 0) {
			CfaTsTxAudCmdQ(prCfaTs);
			prCfaTs->fgStopHandlePkt = TRUE;

		}

		prPidFilter->u4CcErrCnt = 0;
		prPidFilter->fgTxDataFromUinitStart = TRUE;
		prCfaTs->u8ParseOffset += u4DataLen;

		return;
	}
#endif

	if (0 == prCfaTs->u4AudCmdQIndex) {
		prCfaTs->rAudCmdQPtsInfo.u4EntryNb = 0;
		prCfaTs->rAudCmdQPtsInfo.u4UsedEntryCnt = 0;
	}

	if (!prPrivate->fgAnaPesHeaderEnd) {
		/* RETAILMSG(1, (L"[CFA TS] ana aud pes head\n"));*/
		u4Len = CfaTsAnaPesHeader(prCfaTs, prPesHeader, pucData, u4DataLen);
		if (CFA_TS_ANA_PES_HEADER_ERROR == u4Len) {
			if (DATA_SOURCE_FILE == prCfaTs->eDataSource)
				CfaTsFinishPrs(prPrivate->pvSptHdl, prCfaTs);

			else if ((DATA_SOURCE_STREAM == prCfaTs->eDataSource)
				#ifdef BSP_WIFI_WFD
				|| (DATA_SOURCE_WFD == prCfaTs->eDataSource)
				#endif
				) {
				prCfaTs->u8ParseOffset += u4DataLen;
			} else {
				CfaTsFinishPrs(prPrivate->pvSptHdl, prCfaTs);
			}

			return;
		}

		if (u4Len > 0) {
			/*RETAILMSG(1, (L"[CFA TS] AUDIO pes pts: 0x%08x%08x,\n",
			(u32)(prPesHeader->u8Pts >> 32), (u32)(prPesHeader->u8Pts)));*/
			CfaTsCheckAudPesPts(prCfaTs, prPesHeader);

			prPrivate->fgAnaPesHeaderEnd = TRUE;
			prCfaTs->u8ParseOffset += (u4DataLen - u4Len);

			prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[prCfaTs->rAudCmdQPtsInfo.u4EntryNb].u4CmdQIndex
				= prCfaTs->u4AudCmdQIndex;
			prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[prCfaTs->rAudCmdQPtsInfo.u4EntryNb].u8DataInBuf
				= prCfaTs->u8AudDataInBuf;
			prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[prCfaTs->rAudCmdQPtsInfo.u4EntryNb].u8PreTxEndOff
				= prCfaTs->u8AudPreTxEndOff;
			prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[prCfaTs->rAudCmdQPtsInfo.u4EntryNb].u8FileOffset
				= prCfaTs->u8ParseOffset;

			prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[prCfaTs->rAudCmdQPtsInfo.u4EntryNb].u8Pts
				= prPesHeader->u8Pts;
			prCfaTs->rAudCmdQPtsInfo.aruEntryInfo[prCfaTs->rAudCmdQPtsInfo.u4EntryNb].eState
				= PTS_VALID;

			prCfaTs->rAudCmdQPtsInfo.u4EntryNb++;
		}
	} else {
		u4Len = u4DataLen;
	}


	if (u4Len > 0) {
		if (prCfaTs->u4PacketBufLen == prCfaTs->u4TsPacketSize) {
			/*tx a ts packet data which cross pbbuf slot*/
			prCfaTs->eCurPrsStm = CFA_TS_PRS_STRM_TYPE_A;

			prCfaTs->rAudInf.fgUseCmdQ = FALSE;
			prCfaTs->rAudInf.eAudType = prPrivate->eACodec;
			prCfaTs->rAudInf.u4PrsStrmId = prCfaTs->u4AudioStreamID;
			prCfaTs->rAudInf.u8Len = (u64)u4Len;
			prCfaTs->rAudInf.fgUnitStart = FALSE;

			if (prCfaTs->rAudCmdQPtsInfo.u4EntryNb > 0)
				prCfaTs->eACmdQFirstPtsEntryState = PTS_VALID;

			else
				prCfaTs->eACmdQFirstPtsEntryState = PTS_INVALID;

			Spt4CfaBuf2AFifoAUCtrl(prCfaTs->pvSptHdl, prCfaTs->pucPacketBuf
				+ prCfaTs->u4PacketBufLen - u4Len, &prCfaTs->rAudInf, 0);
			prCfaTs->fgStopHandlePkt = TRUE;
			prPesHeader->u4PesPacketDataLen = 0;
			prCfaTs->u8AudDataInBuf += u4Len;
		} else {
			/*for audio cmdq, DMX has no 200KB limitation,
			every CMDQ tx a PES packet or part (when CMDQ cross slot)*/
			if ((prCfaTs->u4AudCmdQIndex == DMX_MAX_TX_CNT_FOR_CMD_Q - 1)
				|| ((prCfaTs->u8ParseOffset + u4Len - prCfaTs->u8AudFirstOffset)
				> (DMX_VID_TX_MAX_SIZE - 20 * 1024))
				|| (prCfaTs->u4MemDataLen < (2 * prCfaTs->u4TsPacketSize))
				|| (prCfaTs->rAudCmdQPtsInfo.u4EntryNb > 1)
				|| ((1 == prCfaTs->rAudCmdQPtsInfo.u4EntryNb)
				&& (prCfaTs->rAudCmdQBakInfo.u4AudCmdQIndex > 0))) {
				/*fill the last cmdq entry and tx cmdq*/
				if (prCfaTs->u4AudCmdQIndex == 0) {
					prCfaTs->u8AudDataInBuf = 0;
					prCfaTs->u8AudPreTxEndOff = prCfaTs->u8ParseOffset;
					prCfaTs->u8AudFirstOffset = prCfaTs->u8ParseOffset;
				}

				prCfaTs->rAudInf.parCmdQTxEntry[prCfaTs->u4AudCmdQIndex].u4TxOfst
					= (u32)(prCfaTs->u8ParseOffset - prCfaTs->u8AudPreTxEndOff);
				prCfaTs->rAudInf.parCmdQTxEntry[prCfaTs->u4AudCmdQIndex].u4TxLen  = u4Len;
				prCfaTs->rAudInf.parCmdQTxEntry[prCfaTs->u4AudCmdQIndex].fgEndAU  = TRUE;
				prCfaTs->u8AudPreTxEndOff = prCfaTs->u8ParseOffset + u4Len;

				prCfaTs->u4AudCmdQIndex += 1;
				prCfaTs->u8AudDataInBuf += u4Len;

				prCfaTs->eCurPrsStm = CFA_TS_PRS_STRM_TYPE_A;

				prCfaTs->rAudInf.u8FileOfst = prCfaTs->u8AudFirstOffset;
				prCfaTs->rAudInf.eAudType = prPrivate->eACodec;
				prCfaTs->rAudInf.u8Len = prCfaTs->u8AudPreTxEndOff - prCfaTs->u8AudFirstOffset;
				prCfaTs->rAudInf.u4PrsStrmId = prCfaTs->u4AudioStreamID;
				prCfaTs->rAudInf.fgUseCmdQ = TRUE;
				prCfaTs->rAudInf.u2TxEntryCnt = (u16)prCfaTs->u4AudCmdQIndex;
				prCfaTs->rAudInf.u8RealTxLen = prCfaTs->u8AudDataInBuf;
				prCfaTs->rAudInf.fgUnitStart = TRUE;
				prCfaTs->rAudInf.fgAUCompleteByEnd = FALSE;

				CfaTsAudCmdQBackupLastPesPacket(prCfaTs);

				if (prCfaTs->rAudCmdQPtsInfo.u4EntryNb > 0)
					prCfaTs->eACmdQFirstPtsEntryState = PTS_VALID;
				else
					prCfaTs->eACmdQFirstPtsEntryState = PTS_INVALID;

				prCfaTs->rAudInf.u8TotalAULen = prCfaTs->rAudInf.u8Len;
				mrRet = Spt4CfaPbb2AFifoAUCtrl(prCfaTs->pvSptHdl, &prCfaTs->rAudInf);
				if (RET_DMX_OK != mrRet) {
					CfaTsFinishPrs(prCfaTs->pvSptHdl, prCfaTs);
				}
				prCfaTs->u4AudCmdQIndex = 0;
				prCfaTs->u8AudDataInBuf = 0;

				prCfaTs->fgStopHandlePkt = TRUE;
			} else {
				/*fill cmdq entry*/
				if (prCfaTs->u4AudCmdQIndex == 0) {
					prCfaTs->u8AudDataInBuf = 0;
					prCfaTs->u8AudPreTxEndOff = prCfaTs->u8ParseOffset;
					prCfaTs->u8AudFirstOffset = prCfaTs->u8ParseOffset;
				}
				(prCfaTs->rAudInf.parCmdQTxEntry + prCfaTs->u4AudCmdQIndex)->u4TxOfst
					= (u32)(prCfaTs->u8ParseOffset - prCfaTs->u8AudPreTxEndOff);
				(prCfaTs->rAudInf.parCmdQTxEntry + prCfaTs->u4AudCmdQIndex)->u4TxLen
					= u4Len;
				(prCfaTs->rAudInf.parCmdQTxEntry + prCfaTs->u4AudCmdQIndex)->fgEndAU
					= FALSE;

				prCfaTs->u8AudPreTxEndOff = prCfaTs->u8ParseOffset + u4Len;

				prCfaTs->u8AudDataInBuf += u4Len;
				prCfaTs->u4AudCmdQIndex++;
			   /* RETAILMSG(1, (L"[CFA TS] A prCfaTs->u8ParseOffset: 0x%08x%08x\n",
			   (u32)(prCfaTs->u8AudPreTxEndOff>>32), (u32)prCfaTs->u8AudPreTxEndOff));*/
			}
		}

		prCfaTs->u8ParseOffset += u4Len;

		return;
	}

	prCfaTs->u8ParseOffset += u4DataLen;
}

void CfaTsBufAudPesCb(CfaTsPidFilter_T *prPidFilter, u8 *pucData, u32 u4DataLen)
{
	CfaTsAudPrivate_T *prPrivate = (CfaTsAudPrivate_T *)prPidFilter->pvPrivate;
	CfaTsPesHeader_T *prPesHeader = &prPrivate->rPesHeader;
	CfaTsInst *prCfaTs = prPrivate->prCfaTs;
	u32 u4Len = 0;


	if (prCfaTs->fgAudSeek) {
		if (!prPidFilter->fgUnitStart) {
			prCfaTs->u8ParseOffset += u4DataLen;

			return;
		}
		prCfaTs->fgAudSeek = FALSE;
	}


	if (prPidFilter->fgUnitStart) {
		prPrivate->fgCcError = FALSE;
#if 0
		if (prPesHeader->u4PesPacketDataLen == prPrivate->u4DataLen) {
			if (prPrivate->u4DataLen > 0) {
				CFA_AUDIO_INFO_T rAudInfo = {0};
				u32 u4Ret = 0;

				prCfaTs->eCurPrsStm = CFA_TS_PRS_STRM_TYPE_A;

				rAudInfo.eAudType = prPrivate->eACodec;
				rAudInfo.u8Len = prPrivate->u4DataLen;
				rAudInfo.u8Pts = prPesHeader->u8Pts;
				rAudInfo.u4PrsStrmId = prCfaTs->u4AudioStreamID;


				/* RETAILMSG(1, (L"rAudInfo.u4PrsStrmId: 0x%x\n", rAudInfo.u4PrsStrmId));*/
				/* RETAILMSG(1, (L"rAudInfo.u8Len: 0x%x\n", rAudInfo.u8Len));*/
				/*  RETAILMSG(1, (L"pucDataBuf: 0x%x\n", prPrivate->pucDataBuf));*/
				u4Ret = Spt4CfaBuf2AFifoAUCtrl(prPrivate->pvSptHdl,
					prPrivate->pucDataBuf, &rAudInfo, 0);
				DMX_ASSERT(u4Ret == E_DMX_OK);
				prCfaTs->fgStopHandlePkt = TRUE;
			}
		} else
			DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS] CfaTsAudPesCb(): audio PES Packet length error!!!!!\n"));

#endif
		prPesHeader->eState = CFA_TS_ST_PES_MIN_HEADER;
		prPesHeader->u4HeaderLen = 0;
		prPrivate->u4DataLen = 0;
		prPesHeader->u4PesPacketDataLen = 0;

#if CFG_SUPPORT_HDCP
		prPesHeader->fgHDCP = FALSE;
#endif
	}


	/*gather pes packet data byte*/
	if (0 == prPrivate->u4DataLen) {
		/*RETAILMSG(1, (L"[CFA TS] ana aud pes head\n"));*/
		u4Len = CfaTsAnaPesHeader(prCfaTs, prPesHeader, pucData, u4DataLen);
		if (CFA_TS_ANA_PES_HEADER_ERROR == u4Len) {
			if (DATA_SOURCE_FILE == prCfaTs->eDataSource)
				CfaTsFinishPrs(prPrivate->pvSptHdl, prCfaTs);
			else if (DATA_SOURCE_STREAM == prCfaTs->eDataSource)
				prCfaTs->u8ParseOffset += u4DataLen;
			else
				CfaTsFinishPrs(prPrivate->pvSptHdl, prCfaTs);

			return;
		}

		if ((u4Len > 0) && (CFA_TS_ST_PES_PAYLOD == prPesHeader->eState)) {
			if ((PTS_INVALID == prCfaTs->rAudBufPts.eState) &&
				(INVALID_TIMESTAMP != prPesHeader->u8Pts)) {
				prCfaTs->rAudBufPts.u8Pts = prPesHeader->u8Pts;
				prCfaTs->rAudBufPts.eState = PTS_VALID;
			}

			/*we get the paylod for the first time, start to save payload*/
			if ((DMX_MIN(u4Len, prPesHeader->u4PesPacketDataLen) > prPrivate->u4BufLen)
				|| (u4Len > u4DataLen)){
				CfaTsFinishPrs(prPrivate->pvSptHdl, prCfaTs);
				return;
			}
			dmx_memcpy(prPrivate->pucDataBuf,
				pucData + u4DataLen - u4Len, DMX_MIN(u4Len, prPesHeader->u4PesPacketDataLen));
			prPrivate->u4DataLen += DMX_MIN(u4Len, prPesHeader->u4PesPacketDataLen);

			/*RETAILMSG(1, (L"[CFA TS] A rPesHeader.u8Pts: %d\n", prPesHeader->u8Pts));*/
			CfaTsCheckAudPesPts(prCfaTs, prPesHeader);
		}
	} else {
		/*save payload*/
		u4Len = DMX_MIN(u4DataLen, prPesHeader->u4PesPacketDataLen - prPrivate->u4DataLen);
		if ((prPrivate->u4DataLen + u4Len) < prPrivate->u4BufLen) {
			dmx_memcpy(prPrivate->pucDataBuf + prPrivate->u4DataLen,
				pucData + u4DataLen - u4Len, u4Len);
			prPrivate->u4DataLen += u4Len;
		}
	}
#if 1
	/*if a whole pes packet data byte has been gathered, send it to afifo*/
	if ((prPesHeader->u4PesPacketDataLen == prPrivate->u4DataLen)
			&& (prPrivate->u4DataLen > 0)) {
		CFA_AUDIO_INFO_T rAudInfo = {0};
		u32 u4Ret = 0;

		prCfaTs->eCurPrsStm = CFA_TS_PRS_STRM_TYPE_A;

		rAudInfo.eAudType = prPrivate->eACodec;
		rAudInfo.u8Len = prPrivate->u4DataLen;
		rAudInfo.u8Pts = prPesHeader->u8Pts;
		rAudInfo.u4PrsStrmId = prCfaTs->u4AudioStreamID;

#if CFG_SUPPORT_HDCP
		if ((DATA_SOURCE_WFD == prCfaTs->eDataSource) &&
			(prPesHeader->fgHDCP && prCfaTs->fgHDCP)) {
			HDCP2X_DEC_UNIT rtDhcp2xDecInfo = {0};

			rtDhcp2xDecInfo.strCounter = prPesHeader->u4StreamCounter;
			rtDhcp2xDecInfo.inputCounter = prPesHeader->u8InputCounter;
			rtDhcp2xDecInfo.srcFrame = prPrivate->pucDataBuf;
			rtDhcp2xDecInfo.len = prPrivate->u4DataLen;
			rtDhcp2xDecInfo.desFrame = prPrivate->pucDataBuf;

#if 0
			{
			unsigned int i;

			RETAILMSG(1, (L"before decrypt aud data:\n"));
			for (i = 0; i < prPrivate->u4DataLen; i++) {
				RETAILMSG(1, (L"%02x ", prPrivate->pucDataBuf[i]));
				if (!((i+1)%16) && i != 0)
					RETAILMSG(1, (L"\n"));
			}
			RETAILMSG(1, (L"\n"));
			}
#endif

			DeviceIoControl(prCfaTs->hHdcpDrv, HDCP_IOCTL_DECRYPTION,
				&rtDhcp2xDecInfo, sizeof(rtDhcp2xDecInfo),
				NULL, 0, NULL, NULL);
			{
#if 0
			unsigned int i = 0;

			RETAILMSG(1, (L"after decrypt aud data:\n"));
			for (i; i < (prPrivate->u4DataLen); i++) {
				RETAILMSG(1, (L"%02x ", prPrivate->pucDataBuf[i]));
				if (!((i+1)%16) && i != 0)
					RETAILMSG(1, (L"\n"));
			}
			RETAILMSG(1, (L"\n"));
#endif
			}
		}
#endif

		/*RETAILMSG(1, (L"rAudInfo.u4PrsStrmId: 0x%x\n", rAudInfo.u4PrsStrmId));*/
		u4Ret = Spt4CfaBuf2AFifoAUCtrl(prPrivate->pvSptHdl,
			prPrivate->pucDataBuf, &rAudInfo, 0);
		if (RET_DMX_OK != u4Ret) {
			CfaTsFinishPrs(prCfaTs->pvSptHdl, prCfaTs);
			return;
		}
		prCfaTs->fgStopHandlePkt = TRUE;
	}
#endif
	prCfaTs->u8ParseOffset += u4DataLen;

}


void CfaTsSectionCb(CfaTsPidFilter_T *prPidFilter, u8 *pucData, u32 u4DataLen)
{
	CfaTsSecPrivate_T *prPrivate = (CfaTsSecPrivate_T *)prPidFilter->pvPrivate;
	u32 u4Len = 0;
	u16 u2CurTblIdIdx = 0;
	CfaTsInst *prCfaTs = prPrivate->prCfaTs;

	prCfaTs->eDataType = DATA_SEC;
	/*check if the data is we need to tx*/
	if (0 == prPrivate->arSecInfo[prPrivate->u2CurTableIdIdx].u4DataLen) {
		u32 i = 0;
		u32 u4SkipBytes = 0;

		if (!prCfaTs->fgHasRemnant)
			u4SkipBytes += (pucData[u4SkipBytes] + 1);

#if 0
		if (0x27 == prPidFilter->u4PID) {
			u32 i;

			RETAILMSG(1, (L"prPidFilter->u4PID: 0x%x\n", prPidFilter->u4PID));
			for (i = 0; i < u4DataLen; i++) {
				if (!((i)%16) && i != 0)
					RETAILMSG(1, (L"\n"));

				RETAILMSG(1, (L"%02x ", pucData[i]));
			}
			RETAILMSG(1, (L"\n"));
		}
#endif
		/*DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
		TEXT("[CFA TS] current TABLE ID: 0x%x\n"),
		(u16)(*(pucData + u4SkipBytes)));*/
		for (i = 0; i < prPrivate->u2TableIdNum; i++) {
			if ((u16)(*(pucData + u4SkipBytes))
					== prPrivate->arSecInfo[i].u2TableId) {
				/*if EIT, check service id*/
				if ((prPrivate->arSecInfo[i].u2TableId >= MIN_EIT_TABLE_ID)
					&& (prPrivate->arSecInfo[i].u2TableId <= MAX_EIT_TABLE_ID)) {
					/*DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("[CFA TS] current service ID: 0x%x\n"),
					(((u16)(*(pucData + u4SkipBytes + 3)) << 8)
					| *(pucData + u4SkipBytes + 4)));*/
					if (prPrivate->arSecInfo[i].u2ServiceId
						== (((u16)(*(pucData + u4SkipBytes + 3)) << 8)
						| *(pucData + u4SkipBytes + 4)))
						break;
				} else {
					break;
				}
			}
		}


		if (prPrivate->u2TableIdNum == i) {
			if (prCfaTs->fgHasRemnant)
				prCfaTs->fgHasRemnant = FALSE;
			else
				prCfaTs->u8ParseOffset += u4DataLen;

			return;
		}
		prPrivate->u2CurTableIdIdx = (u16)i;
		/*DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
		TEXT("[CFA TS] tx section-->TABLE ID: 0x%x\n"),
		prPrivate->arSecInfo[i].u2TableId);*/
		/*DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
		TEXT("[CFA TS] tx section-->PID: 0x%x\n"),
		prPidFilter->u4PID);*/
	}

	u2CurTblIdIdx = prPrivate->u2CurTableIdIdx;

	if (prPidFilter->fgUnitStart) {
		u32 u4PointerField = 1 + pucData[0];

		if (1 == u4PointerField)
			prPrivate->arSecInfo[u2CurTblIdIdx].u4DataLen = 0;

		pucData += 1;
		u4DataLen -= 1;
		prCfaTs->u8ParseOffset += 1;
	}

	/*gather pes packet data byte*/
	if (0 == prPrivate->arSecInfo[u2CurTblIdIdx].u4DataLen) {
		/*DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
		TEXT("[CFA TS] section header: %x %x %x %x %x\n"),
		pucData[0], pucData[1], pucData[2], pucData[3], pucData[4]);*/
		prPrivate->arSecInfo[u2CurTblIdIdx].u4SectionLen
		= ((((u32)pucData[1] << 8) | pucData[2]) & 0x00000fff) + 3;
		/*DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
		TEXT("[CFA TS] section len: %d\n"),
		prPrivate->arSecInfo[u2CurTblIdIdx].u4SectionLen);*/
		if ((prPrivate->arSecInfo[u2CurTblIdIdx].u4SectionLen > MAX_SECTION_LEN)
			|| (3 == prPrivate->arSecInfo[u2CurTblIdIdx].u4SectionLen)) {
			if (!prCfaTs->fgHasRemnant)
				prCfaTs->u8ParseOffset += u4DataLen;
			else
				prCfaTs->fgHasRemnant = FALSE;

			return;
		}

		/*we get the paylod for the first time, start to save payload*/
		u4Len = DMX_MIN(u4DataLen, prPrivate->arSecInfo[u2CurTblIdIdx].u4SectionLen);
		if (u4Len > MAX_SECTION_LEN) {
			CfaTsFinishPrs(prCfaTs->pvSptHdl, prCfaTs);
			return;
		}
		dmx_memcpy(prPrivate->arSecInfo[u2CurTblIdIdx].aucDataBuf, pucData, u4Len);
		prPrivate->arSecInfo[u2CurTblIdIdx].u4DataLen += u4Len;

		/*DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
		TEXT("[CFA TS] section DataLen: %d\n"),
		prPrivate->arSecInfo[u2CurTblIdIdx].u4DataLen);*/
	} else {
		/*save payload*/
		u4Len = DMX_MIN(u4DataLen, prPrivate->arSecInfo[u2CurTblIdIdx].u4SectionLen
			- prPrivate->arSecInfo[u2CurTblIdIdx].u4DataLen);

		if ((prPrivate->arSecInfo[u2CurTblIdIdx].u4DataLen + u4Len) > MAX_SECTION_LEN) {
			CfaTsFinishPrs(prCfaTs->pvSptHdl, prCfaTs);
			return;
		}
		dmx_memcpy(prPrivate->arSecInfo[u2CurTblIdIdx].aucDataBuf
			+ prPrivate->arSecInfo[u2CurTblIdIdx].u4DataLen, pucData, u4Len);
		prPrivate->arSecInfo[u2CurTblIdIdx].u4DataLen += u4Len;
		/*DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
		TEXT("[CFA TS] section DataLen 1: %d\n"),
		prPrivate->arSecInfo[u2CurTblIdIdx].u4DataLen);*/
	}

	if (!prCfaTs->fgHasRemnant)
		prCfaTs->u8ParseOffset += u4DataLen;

	else {
		if (prPrivate->arSecInfo[u2CurTblIdIdx].u4SectionLen > prCfaTs->u4RemnantLen)
			prCfaTs->fgHasRemnant = FALSE;

	}

	/*if a whole section data byte has been gathered,
	send it to section fifo*/
	if (prPrivate->arSecInfo[u2CurTblIdIdx].u4SectionLen
			== prPrivate->arSecInfo[u2CurTblIdIdx].u4DataLen) {
		CFA_SECTION_INFO_T rSecInfo = {0};
		MRESULT mrRet = RET_DMX_OK;

		prCfaTs->eCurPrsStm = CFA_TS_PRS_STRM_TYPE_SEC;

		rSecInfo.u8Len = prPrivate->arSecInfo[u2CurTblIdIdx].u4DataLen;
		rSecInfo.u4PrsStrmId = prCfaTs->u4SectionStreamID;

#if 0
		if (0x27 == prPidFilter->u4PID) {
			u32 i;

			RETAILMSG(1, (L"CfaTsSectionCb tx data, prPidFilter->u4PID: 0x%x\n", prPidFilter->u4PID));
			RETAILMSG(1, (L"\n"));
			for (i = 0; i < prPrivate->arSecInfo[u2CurTblIdIdx].u4DataLen; i++) {
				if (!((i)%16) && i != 0)
					RETAILMSG(1, (L"\n"));

				RETAILMSG(1, (L"%02x ", prPrivate->arSecInfo[u2CurTblIdIdx].aucDataBuf[i]));
			}
			RETAILMSG(1, (L"\n"));
		}
#endif
		mrRet = Spt4CfaBuf2SectionFifoAUCtrl(prPrivate->pvSptHdl,
			prPrivate->arSecInfo[u2CurTblIdIdx].aucDataBuf, &rSecInfo, 0);
		if (RET_DMX_OK != mrRet){
			CfaTsFinishPrs(prCfaTs->pvSptHdl, prCfaTs);
			return;
		}
		prCfaTs->fgStopHandlePkt = TRUE;

		if (u4DataLen > u4Len) {
			prCfaTs->fgHasRemnant = TRUE;
			if ((u4DataLen - u4Len) > CFA_TS_MAX_TS_PACKET_SIZE){
				CfaTsFinishPrs(prCfaTs->pvSptHdl, prCfaTs);
				return;
			}
			dmx_memcpy(prCfaTs->aucRemnantBuf, pucData + u4Len, u4DataLen - u4Len);
			prCfaTs->u4RemnantLen = u4DataLen - u4Len;
			prCfaTs->u4RemnantPID = prPidFilter->u4PID;
		} else {
			prCfaTs->fgHasRemnant = FALSE;
		}
		prPrivate->arSecInfo[u2CurTblIdIdx].u4DataLen = 0;
	}

}



void CfaTsCcPesCb(CfaTsPidFilter_T *prPidFilter, u8 *pucData, u32 u4DataLen)
{
	CfaTsCcPrivate_T *prPrivate = (CfaTsCcPrivate_T *)prPidFilter->pvPrivate;
	CfaTsInst *prCfaTs = prPrivate->prCfaTs;
	CfaTsPesHeader_T *prPesHeader = &prPrivate->rPesHeader;
	u32 u4Len = 0;

	prCfaTs->eDataType = DATA_CC;

	if (prPidFilter->fgUnitStart) {
		prPesHeader->eState = CFA_TS_ST_PES_MIN_HEADER;
		prPesHeader->u4HeaderLen = 0;
		prPesHeader->u4PesPacketDataLen = 0;
		prPrivate->u4DataLen = 0;
	}

	/*gather pes packet data byte*/
	if (0 == prPrivate->u4DataLen) {
		u4Len = CfaTsAnaPesHeader(prCfaTs, prPesHeader, pucData, u4DataLen);
		if (CFA_TS_ANA_PES_HEADER_ERROR == u4Len) {
			if (DATA_SOURCE_FILE == prCfaTs->eDataSource)
				CfaTsFinishPrs(prPrivate->pvSptHdl, prCfaTs);

			else if ((DATA_SOURCE_STREAM == prCfaTs->eDataSource)
				#ifdef BSP_WIFI_WFD
				|| (DATA_SOURCE_WFD == prCfaTs->eDataSource)
				#endif
				) {
				prCfaTs->u8ParseOffset += u4DataLen;
			} else {
				CfaTsFinishPrs(prPrivate->pvSptHdl, prCfaTs);
			}

			return;
		}


		/*we get the paylod for the first time, start to save payload*/
		if ((u4Len > 0) && (CFA_TS_ST_PES_PAYLOD == prPesHeader->eState)) {
			/*RETAILMSG(1, (L"[CFA TS] C rPesHeader.u8Pts: 0x%08x%08x\n",
			(u32)(prPesHeader->u8Pts>>32), (u32)prPesHeader->u8Pts));*/

			if ((DMX_MIN(u4Len, (prPesHeader->u4PesPacketLen + CFA_TS_MIN_PES_HEADER_LEN)) > prPrivate->u4BufLen) 
				|| (u4Len > u4DataLen)){
				CfaTsFinishPrs(prPrivate->pvSptHdl, prCfaTs);
				return;
			}
			dmx_memcpy(prPrivate->pucDataBuf, pucData,
				DMX_MIN(u4DataLen, (prPesHeader->u4PesPacketLen
				+ CFA_TS_MIN_PES_HEADER_LEN)));
			prPrivate->u4DataLen
				+= DMX_MIN(u4DataLen, (prPesHeader->u4PesPacketLen
				+ CFA_TS_MIN_PES_HEADER_LEN));
		}
	} else {
		/*save payload*/
		u4Len = DMX_MIN(u4DataLen,
			(prPesHeader->u4PesPacketLen + CFA_TS_MIN_PES_HEADER_LEN)
			- prPrivate->u4DataLen);
		if ((prPrivate->u4DataLen + u4Len) < prPrivate->u4BufLen) {
			if (((prPrivate->u4DataLen + u4Len) > prPrivate->u4BufLen) 
				|| (u4Len > u4DataLen)){
				CfaTsFinishPrs(prPrivate->pvSptHdl, prCfaTs);
				return;
			}
			dmx_memcpy(prPrivate->pucDataBuf + prPrivate->u4DataLen,
				pucData + u4DataLen - u4Len, u4Len);
			prPrivate->u4DataLen += u4Len;
		}
	}

	/*if a whole pes packet data byte has been gathered, send it to ccfifo*/
	if (((prPesHeader->u4PesPacketLen + CFA_TS_MIN_PES_HEADER_LEN)
			== prPrivate->u4DataLen) && (prPrivate->u4DataLen > 0)) {
		CFA_SUBPIC_INFO_T rSpInfo = {0};
		MRESULT mrRet = RET_DMX_OK;

		prCfaTs->eCurPrsStm = CFA_TS_PRS_STRM_TYPE_SP;

		rSpInfo.u8Len = prPrivate->u4DataLen;
		rSpInfo.u4PrsStrmId = prCfaTs->u4SpStreamID;

#if 0
		{
			u32 i;

			RETAILMSG(1, (L"\n"));
			for (i = 0; i < prPrivate->u4DataLen; i++) {
				if (!((i)%16) && i != 0)
					RETAILMSG(1, (L"\n"));

				RETAILMSG(1, (L"%02x ", prPrivate->pucDataBuf[i]));
			}
			RETAILMSG(1, (L"\n"));
		}
#endif
		mrRet = Spt4CfaBuf2SpFifoAUCtrl(prPrivate->pvSptHdl,
			prPrivate->pucDataBuf, &rSpInfo, 0);
        	if (RET_DMX_OK != mrRet){
			CfaTsFinishPrs(prPrivate->pvSptHdl, prCfaTs);
    		}
		prCfaTs->fgStopHandlePkt = TRUE;
	}

	prCfaTs->u8ParseOffset += u4DataLen;

}


static u32 CfaTsFindSyncWord(u8 *pucBuffer, u32 u4DataLen)
{
	u32 u4SyncBytes = 0;

	for (u4SyncBytes = 0; u4SyncBytes < u4DataLen; ) {
		if (CFA_TS_SYNC_WORD == pucBuffer[u4SyncBytes]) {
			DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsFindSyncWord():")
				TEXT("find the sync word!\n"));
			break;
		}
		u4SyncBytes++;
	}

	return u4SyncBytes;
}

/*
*  name: CfaTsHandleRemnant
*
*  description: handle remnant data
*
*  inputs:
*
*  outputs:
*
*  return:
*/
static u32 CfaTsHandleRemnant(void *pvSptHdl, CfaTsInst *prCfaTs, u8 *pucBuf, u32 u4Len)
{
	CfaTsPidFilter_T *prPidFilter = NULL;

	prPidFilter = prCfaTs->Pids[prCfaTs->u4RemnantPID];
	if (NULL == prPidFilter) {
		prCfaTs->fgHasRemnant = FALSE;

		return u4Len;
	}

#if 0
	if (0x27 == prPidFilter->u4PID) {
		u32 i;

		RETAILMSG(1, (L"\n"));
		for (i = 0; i < u4Len; i++) {
			if (!((i)%16) && i != 0)
				RETAILMSG(1, (L"\n"));

			RETAILMSG(1, (L"%02x ", pucBuf[i]));
		}
		RETAILMSG(1, (L"\n"));
	}
#endif

	prPidFilter->fgUnitStart = FALSE;
	prPidFilter->pfTsPktCb(prPidFilter, pucBuf, u4Len);

	return u4Len;
}


/*
*  name: CfaTsHandleTsPacket
*
*  description: handle one ts packet, acorrding the PID, send it to PID filter
*
*  inputs:
*
*  outputs:
*
*  return:
*/
static u32 CfaTsHandleTsPacket(void *pvSptHdl, CfaTsInst *prCfaTs, u8 *pucBuf, u32 u4Len)
{
	CfaTsPidFilter_T *prPidFilter = NULL;
	s32 i4Cc = 0;
	u32 u4Pid = 0;
	u32 u4SkipBytes = 0;

	if (CFA_TS_SYNC_WORD != pucBuf[0]) {
		u32 u4Ret = CfaTsFindSyncWord(pucBuf, u4Len);
		prCfaTs->u8ParseOffset += u4Ret;

		return u4Ret;
	}

	u4Pid = CFA_TS_GET_PID(pucBuf[1], pucBuf[2]);
	if ((u4Pid >= CFA_TS_PMT_MIN_PID) && (u4Pid <= CFA_TS_PMT_MAX_PID))
		u4Pid = CFA_TS_PMT_MIN_PID;

	prPidFilter = prCfaTs->Pids[u4Pid];

	if ((NULL == prPidFilter) ||
		((u4Pid != prCfaTs->u4CurAudPID) && (u4Pid != prCfaTs->u4CurVidPID)
		&& (u4Pid != prCfaTs->u4CurCcPID) && (TS_SECTION != prPidFilter->ePidType)) ||
		(NULL == prPidFilter->pvPrivate))
		goto TsPacketError;

	/*unit start*/
	prPidFilter->fgUnitStart = FALSE;
	if (pucBuf[1] & CFA_TS_FLAG_UNIT_START) {
		/*RETAILMSG(1, (L"[CFA TS]fgUnitStart = TRUE,
			prCfaTs->u8ParseOffset: 0x%x!\n", prCfaTs->u8ParseOffset));*/
		prPidFilter->fgUnitStart = TRUE;
	}

	/*check if stream enable*/
	if (((u4Pid == prCfaTs->u4CurAudPID)
		&& (0 == (prCfaTs->u4CurPrsFlg & ((u32)CFA_TS_PRS_STRM_TYPE_A))))
		|| ((u4Pid == prCfaTs->u4CurVidPID)
		&& (0 == (prCfaTs->u4CurPrsFlg & ((u32)CFA_TS_PRS_STRM_TYPE_V))))
		|| ((u4Pid == prCfaTs->u4CurCcPID)
		&& (0 == (prCfaTs->u4CurPrsFlg & ((u32)CFA_TS_PRS_STRM_TYPE_SP))))
		|| ((TS_SECTION == prPidFilter->ePidType)
		&& (0 == (prCfaTs->u4CurPrsFlg & ((u32)CFA_TS_PRS_STRM_TYPE_SEC))))) {
		/*DmxLogE(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
			TEXT("[CFA TS]CfaTsHandleTsPacket(): stream not enable\n"));*/
		goto TsPacketError;
	}


#if CFG_CHECK_ONLY_transport_error_indicator
	/*check transport_error_indicator*/
	if ((pucBuf[1] & CFA_TS_FLAG_TRANSPORT_ERROR_INDICATOR)
		&& (u4Pid == prCfaTs->u4CurAudPID))
#else
	/*check transport_error_indicator && (payload_unit_start_indicator || transport_priority)*/
	if ((pucBuf[1] & CFA_TS_FLAG_TRANSPORT_ERROR_INDICATOR)
		&& (prPidFilter->fgUnitStart || (pucBuf[1] & CFA_TS_FLAG_TRANSPORT_PRIORITY)))
#endif
		{
			goto TsPacketError;
		}

	/*check continuity counter*/
	i4Cc = (s32)(pucBuf[3] & CFA_TS_MASK_CONTINUTY_COUNTER);
	prPidFilter->fgCcOk = (prPidFilter->i4LastCc < 0)
		|| (((prPidFilter->i4LastCc + 1) & CFA_TS_MASK_CONTINUTY_COUNTER) == i4Cc);

	if ((prPidFilter->i4LastCc >= 0) &&
		(prPidFilter->i4LastCc == i4Cc) &&
		((TS_SECTION == prPidFilter->ePidType) ||
		!prPidFilter->fgUnitStart)) {
		/*duplicate*/
		DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
		TEXT("[CFA TS]CfaTsHandleTsPacket(): duplicate TS packet!!!\n"));
		goto TsPacketError;
	}
	prPidFilter->i4LastCc = i4Cc;

	if (!(pucBuf[3] & CFA_TS_FLAG_PAYLOAD)) {
		/*no payload*/
		goto TsPacketError;
	}

	/*skip adaptation field*/
	if ((pucBuf[3] & CFA_TS_FLAG_ADAPTATION_FIELD)) {
		/*has adaptation field*/
		if (pucBuf[4] > CFA_TS_MAX_ADAPTATION_FIELD_LEN) {
			DmxLogD(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsHandleTsPacket(): adaptation field len: %d!\n"),
				pucBuf[4]);
			goto TsPacketError;
		}

		u4SkipBytes = 5 + pucBuf[4];
	} else {
		/*only payloads*/
		u4SkipBytes = 4;
	}

#if 0
	if (0x1011 == u4Pid) {
		unsigned int i;

		RETAILMSG(1, (L"ts vid data:\n"));
		for (i = 0; i < u4Len; i++) {
			RETAILMSG(1, (L"%02x ", pucBuf[i]));
			if (!((i+1)%16) && i != 0)
				RETAILMSG(1, (L"\n"));
		}
		RETAILMSG(1, (L"\n"));
	}
#endif

	if (NULL != prPidFilter->pfTsPktCb) {
		prCfaTs->u8ParseOffset += u4SkipBytes;
		prPidFilter->fgNeedRollback = FALSE;
		prPidFilter->pfTsPktCb(prPidFilter, pucBuf + u4SkipBytes, u4Len - u4SkipBytes);
		if (prPidFilter->fgNeedRollback) {
			prCfaTs->u8ParseOffset -= u4SkipBytes;
			return 0;
		}
	} else {
		goto TsPacketError;
	}

	return u4Len;

TsPacketError:
	prCfaTs->u8ParseOffset += u4Len;

	return u4Len;
}


void CfaTsTxDoneStCtrl(void *pvSptHdl, u64 u8TxLen, CfaTsInst *prCfaTs)
{
	u32 u4SkipDataLen = 0;

	dmx_sema_lock(prCfaTs->sCfaTs, DMX_SEMA_OPTION_WAIT);

	prCfaTs->fgStopHandlePkt = FALSE;

	if (prCfaTs->fgHasRemnant) {
		/*if a part of	ts packet data has been handled, go on for the remnant*/
		CfaTsHandleRemnant(pvSptHdl, prCfaTs, prCfaTs->aucRemnantBuf, prCfaTs->u4RemnantLen);
	} else if (prCfaTs->fgSyncPbbuf) {
		prCfaTs->u4MemDataLen += (u32)u8TxLen;

		if (prCfaTs->u4PacketBufLen > 0) {
			u32 u4CopyBytes
				= DMX_MIN(prCfaTs->u4TsPacketSize - prCfaTs->u4PacketBufLen, prCfaTs->u4MemDataLen);

			/*save another part ts packet data after sync pbbuf*/
			if ((prCfaTs->u4PacketBufLen + u4CopyBytes) > CFA_TS_MAX_TS_PACKET_SIZE) {
				CfaTsFinishPrs(prCfaTs->pvSptHdl, prCfaTs);
	            		dmx_sema_unlock(prCfaTs->sCfaTs);
				return;
			}
			dmx_memcpy(prCfaTs->pucPacketBuf + prCfaTs->u4PacketBufLen,
				(u8 *)prCfaTs->ptrMemAddr, u4CopyBytes);
			prCfaTs->u4PacketBufLen += u4CopyBytes;
			prCfaTs->u8Ca += u4CopyBytes;
			prCfaTs->ptrMemAddr += (uintptr_t)u4CopyBytes;
			prCfaTs->u4MemDataLen -= u4CopyBytes;

			/*handle the whole ts packet*/
			if (prCfaTs->u4PacketBufLen == prCfaTs->u4TsPacketSize) {
				u4SkipDataLen
					= CfaTsHandleTsPacket(pvSptHdl, prCfaTs,
					prCfaTs->pucPacketBuf, TS_COM_PKT_SIZE_188);
				prCfaTs->u8ParseOffset += (prCfaTs->u4TsPacketSize - u4SkipDataLen);
				prCfaTs->u4PacketBufLen = 0;
			}
		}

		prCfaTs->fgSyncPbbuf = FALSE;
	}

	/*handle ts packet one by one*/
	while ((prCfaTs->u4MemDataLen >= prCfaTs->u4TsPacketSize)
			&& !prCfaTs->fgStopHandlePkt) {

		prCfaTs->eDataType = DATA_NO;

		u4SkipDataLen = CfaTsHandleTsPacket(pvSptHdl, prCfaTs,
			(u8 *)prCfaTs->ptrMemAddr, TS_COM_PKT_SIZE_188);
		if (u4SkipDataLen == TS_COM_PKT_SIZE_188) {
			prCfaTs->u8ParseOffset += (prCfaTs->u4TsPacketSize - u4SkipDataLen);
			u4SkipDataLen = prCfaTs->u4TsPacketSize;
		}
		prCfaTs->u4MemDataLen -= u4SkipDataLen;
		prCfaTs->u8Ca += u4SkipDataLen;
		prCfaTs->ptrMemAddr += (uintptr_t)u4SkipDataLen;


		if (prCfaTs->u8ParseOffset != prCfaTs->u8Ca) {
			switch (prCfaTs->eDataType) {
			case DATA_V:
				DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("[CFA TS]CfaTsTxDoneStCtrl():")
					TEXT("ERROR in handle video data\n"));
				break;

			case DATA_A:
				DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("[CFA TS]CfaTsTxDoneStCtrl():")
					TEXT("ERROR in handle audio data\n"));
				break;

			case DATA_SEC:
				DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("[CFA TS]CfaTsTxDoneStCtrl():")
					TEXT("ERROR in handle section data\n"));
				break;

			case DATA_CC:
				DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("[CFA TS]CfaTsTxDoneStCtrl():")
					TEXT("ERROR in handle cc data\n"));
				break;

			case DATA_NO:
				DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("[CFA TS]CfaTsTxDoneStCtrl():")
					TEXT("ERROR in not handle data\n"));
				break;

			default:
				DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
					TEXT("eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeerror!\n"));
				break;
			}

			DmxLogT(DMX_MOD_CFA_TS, CFA_TS_LOG_DEFAULT, 
				TEXT("[CFA TS]CfaTsTxDoneStCtrl():")
				TEXT("******** prCfaTs->u8Ca:")
				TEXT("0x%08x%08x, prCfaTs->u8ParseOffset:")
				TEXT("0x%08x%08x ******\n"),
				((u32)(prCfaTs->u8Ca >> 32)), (u32)prCfaTs->u8Ca,
				((u32)(prCfaTs->u8ParseOffset >> 32)), (u32)prCfaTs->u8ParseOffset);
			/*while(1);*/
			prCfaTs->u8ParseOffset = prCfaTs->u8Ca;
		}
	}

	/*the data in prCfaTs->ptrMemAddr less than a ts packet, we need to sync pbbuf*/
	if ((prCfaTs->u4MemDataLen < prCfaTs->u4TsPacketSize) && !prCfaTs->fgStopHandlePkt) {
		/*avoid cmdq cross pbbuf slot*/
		if (prCfaTs->u4VidCmdQIndex > 0) {
			CfaTsTxVidCmdQ(prCfaTs);
			dmx_sema_unlock(prCfaTs->sCfaTs);

			return;
		}

		if (prCfaTs->u4AudCmdQIndex > 0) {
			CfaTsTxAudCmdQ(prCfaTs);
			dmx_sema_unlock(prCfaTs->sCfaTs);

			return;
		}

		/*save part ts packet data before sync pbbuf*/
		if (prCfaTs->u4MemDataLen > CFA_TS_MAX_TS_PACKET_SIZE){
			CfaTsFinishPrs(prCfaTs->pvSptHdl, prCfaTs);
	        	dmx_sema_unlock(prCfaTs->sCfaTs);
			return;
		}
		dmx_memcpy(prCfaTs->pucPacketBuf, (u8 *)prCfaTs->ptrMemAddr, prCfaTs->u4MemDataLen);
		prCfaTs->u4PacketBufLen += prCfaTs->u4MemDataLen;
		prCfaTs->u8Ca += prCfaTs->u4MemDataLen;
		prCfaTs->u4MemDataLen = 0;

		CfaTsSyncPbbuf(pvSptHdl, prCfaTs, 1);
	}

	dmx_sema_unlock(prCfaTs->sCfaTs);

}


u32 CfaTsSyncPbbuf(void *pvSptHdl, CfaTsInst *prCfaTs, u64 u8ReadLen)
{
	u64 u8Len = 0;
	MRESULT mrRet = RET_DMX_OK;

	if ((DATA_SOURCE_FILE == prCfaTs->eDataSource)
			&& (prCfaTs->u8Ca >= prCfaTs->rRange.u8Ea)) {
		CfaTsFinishPrs(pvSptHdl, prCfaTs);

		MM_RETURN(RET_DMX_OK);
	}

	if (DATA_SOURCE_FILE == prCfaTs->eDataSource)
		u8Len = MIN(u8ReadLen, prCfaTs->rRange.u8Ea - prCfaTs->u8Ca);
	else if ((DATA_SOURCE_STREAM == prCfaTs->eDataSource)
		#ifdef BSP_WIFI_WFD
		|| (DATA_SOURCE_WFD == prCfaTs->eDataSource)
		#endif
		) {
		u8Len = u8ReadLen;
	} else {
			CfaTsFinishPrs(pvSptHdl, prCfaTs);

			MM_RETURN(RET_DMX_OK);
	}

	mrRet = Spt4CfaPbb2SyncBufEx(pvSptHdl, prCfaTs->u8Ca, u8Len,
			(u8 *)&prCfaTs->ptrMemAddr, &prCfaTs->u4MemDataLen);
    	if (RET_DMX_OK != mrRet) {
        	CfaTsFinishPrs(pvSptHdl, prCfaTs);
       	 	MM_RETURN(RET_DMX_OK);
	}
	prCfaTs->fgSyncPbbuf = TRUE;

	MM_RETURN(mrRet);
}

