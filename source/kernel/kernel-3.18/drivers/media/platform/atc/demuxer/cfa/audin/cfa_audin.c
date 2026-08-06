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


/*****************************************************************************
 * @par Project
 *    
 *
 * @par Description
 *    CFA audIn File
 *
 * @par Author_Name
 *    Fengw.Wang
*****************************************************************************/

#ifdef __linux__
#include <media/atc/dmx_define.h>
/* #include <media/atc/mm_debug.h> */
#include <media/atc/mm_errcode.h>
#include <media/atc/drv_esm_if.h>
#include <media/atc/x_audin.h>
#else
#include "x_audin.h"
#include "mm_debug.h"
#include "mm_errcode.h"
#include "dmx_define.h"
#endif
#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_errcode.h"
#include "cfa_if.h"
#include "cfa_audin.h"

ValidDataType_T rValidDataType[CFA_AUDIO_IN_CODEC_NUM] = {
	/*{0,0},*/	  /*null data burst*/ {1, 0}, {2, 0}, {2, 1}, {2, 2}, {2, 3},
	/*{3,0},*/	 /*pause data burst*/ {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0},
	{9, 0}, {10, 0}, {11, 0}, {12, 0}, {13, 0}, {14, 0}, {15, 0}, {16, 0},
	{17, 0}, {18, 0}, {18, 1}, {18, 2}, {18, 3}, {19, 0}, {19, 1}, {19, 2},
	{19, 3}, {20, 0}, {20, 1}, {20, 2}, {20, 3}, {21, 0}, {22, 0}, {27, 0},
	{27, 1}, {27, 2}, {27, 3}, {28, 0}, {28, 1}, {28, 2}, {28, 3}, {29, 0},
	{29, 1}, {29, 2}, {29, 3}, {30, 0}, {30, 1}, {30, 2}, {30, 3}, {31, 0},
	{31, 1}, {31, 2}, {31, 3}
};

static CfaApiAudType CfaAudInGetAudType(const CfaAudInInst *prCfaAudInInst)
{
	switch (prCfaAudInInst->eFormat) {
	case CFA_AUDIO_IN_PCM:
		return CFA_AUD_DRV_FMT_PCM;

	case CFA_AUDIO_IN_DTS14:
	case CFA_AUDIO_IN_DTS16:
		return CFA_AUD_DRV_FMT_DTS;

	case CFA_AUDIO_IN_RAW:
		return CFA_AUD_DRV_FMT_PCM;

	default:
		break;
	}

	return CFA_AUD_DRV_FMT_PCM;
}

static MRESULT CfaAudInInit(HANDLE hSpt, void **ppvCfaPrivData)
{
	CfaAudInInst *prCfaAudInInst = NULL;

	DMX_NewMemory(sizeof(CfaAudInInst), prCfaAudInInst);

	if (NULL == prCfaAudInInst) {
		DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] Alloc prCfaAudInInst memory fail\r\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	} else {
		dmx_memset(prCfaAudInInst, 0X00, sizeof(CfaAudInInst));
	}

	prCfaAudInInst->rRange.u8Sa = 0;
	prCfaAudInInst->rRange.u8Ea = 0;
	prCfaAudInInst->u4FindReadNum = 0;
	prCfaAudInInst->eFormat = CFA_AUDIO_IN_UNKNOWN;
	prCfaAudInInst->eAudApiType = CFA_AUD_DRV_FMT_UNKNOWN;
	prCfaAudInInst->u8CurrTxOft = (u64)CFA_AUDIN_INVALID_VALUE;
	/*prCfaAudInInst->u4TxLen = CFA_AUDIO_IN_PCM_UNIT_SIZE;*/
	prCfaAudInInst->u4PfrMemAddress = (u32)CFA_AUDIN_INVALID_ADDRESS;
	prCfaAudInInst->pucAudBuf = NULL;
	prCfaAudInInst->u4PcmUnitSize = CFA_AUDIO_IN_PCM_UNIT_SIZE;
#if CFA_AUDIO_IN_SUPPORT_HDMIIN
	prCfaAudInInst->pucReOrderBuf = NULL;
	prCfaAudInInst->u4ReOrderBufValidSize = 0;
	DMX_NewHwMemory(CFA_AUDIO_IN_BUFF_SIZE * sizeof(u8), prCfaAudInInst->pucReOrderBuf);
	if (NULL == prCfaAudInInst->pucReOrderBuf) {
		DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] Alloc pucReOrderBuf memory fail\r\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}
	prCfaAudInInst->fgReOrder = FALSE;
	prCfaAudInInst->fgNeedMat = FALSE;
#endif

#if CFA_AUDIN_SUPPORT_MAT
	dmx_memset(&prCfaAudInInst->rAudInMatDecInfo, 0, sizeof(AudInMatDecInfo_T));
	DMX_NewHwMemory(CFA_MATDEC_AU_MAX_SIZE * sizeof(u8), prCfaAudInInst->rAudInMatDecInfo.pucMatDecAuBuf);
	if (NULL == prCfaAudInInst->rAudInMatDecInfo.pucMatDecAuBuf) {
		DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] Alloc pucMatDecAuBuf memory fail\r\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}
#endif
	prCfaAudInInst->eAudInSt = CFA_AUDIO_IN_ST_IDL;
	*ppvCfaPrivData = (void *) prCfaAudInInst;

	DmxLogD(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_COMMON,
		TEXT("[CFA_AUDIN] audio in init OK\r\n"));
	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaAudInUninit(HANDLE hSpt, void *pvCfaPrivData)
{
	CfaAudInInst *prCfaAudInInst = (CfaAudInInst *) pvCfaPrivData;

	/*DMX_ASSERT(NULL != prCfaAudInInst);*/
	if (NULL == prCfaAudInInst) {
		DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s: Alloc prCfaAudInInst memory fail\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_NO_MEM);
	}

#if CFA_AUDIO_IN_SUPPORT_HDMIIN
	if (prCfaAudInInst->pucReOrderBuf != NULL) {
		DMX_FreeHwMemory(prCfaAudInInst->pucReOrderBuf);
		prCfaAudInInst->pucReOrderBuf = NULL;
	}
#endif

#if CFA_AUDIN_SUPPORT_MAT
	if (prCfaAudInInst->rAudInMatDecInfo.pucMatDecAuBuf != NULL) {
		DMX_FreeHwMemory(prCfaAudInInst->rAudInMatDecInfo.pucMatDecAuBuf);
		prCfaAudInInst->rAudInMatDecInfo.pucMatDecAuBuf = NULL;
	}
#endif

	prCfaAudInInst->pucAudBuf = NULL;

	DMX_FreeMemory(prCfaAudInInst);
	prCfaAudInInst = NULL;

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaAudInSetRange(HANDLE hSpt, void *pvRange, void *pvPrivData, bool fgIsUserMem)
{
	CfaAudInInst *prCfaAudInInst = (CfaAudInInst *)pvPrivData;

	/*DMX_ASSERT((NULL != pvPrivData) && (NULL != pvRange));*/
	if ((NULL == pvPrivData) || (NULL == pvRange)) {
		DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s: Alloc pvPrivData or pvRange memory fail\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_NO_MEM);
	}

	dmx_memcpy(&(prCfaAudInInst->rRange), pvRange, sizeof(CfaAudInPR));

	prCfaAudInInst->u8CurrTxOft = prCfaAudInInst->rRange.u8Sa;
	prCfaAudInInst->u4TxLen = prCfaAudInInst->u4PcmUnitSize;

	/*DMX_ASSERT(prCfaAudInInst->rRange.u8Ea >= prCfaAudInInst->rRange.u8Sa);*/
	if (prCfaAudInInst->rRange.u8Ea < prCfaAudInInst->rRange.u8Sa) {
		DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s: Ea < Sa\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	DmxLogT(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
		TEXT("[CFA_AUDIN] %s: u8Sa=0x%llx, u8Ea=0x%llx\r\n"),
		DMX_FUNC_NAME, prCfaAudInInst->rRange.u8Sa, prCfaAudInInst->rRange.u8Ea);

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaAudInEnableStrm(HANDLE hSpt, u32 u4StrmToPrs, CfaStreamOp eOp, void *pvPrivData)
{
	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaAudInSetStrmInf(HANDLE hSpt, u32 u4Strm, u32 u4Info, void *pvPrivData)
{
	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaAudInConfigure(HANDLE hSpt, void *pvParam, void *pvPrivData, bool fgIsUserMem)
{
	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaAudInTurnOn(HANDLE hSpt, void *pvPrivData)
{
	CfaAudInInst *prCfaAudInInst = (CfaAudInInst *)pvPrivData;
#if CFA_AUDIO_IN_SUPPORT_HDMIIN
	AUDIN_PARSING_INFO_T rAudInParingInfo;
	MRESULT mrResult = RET_DMX_OK;
#endif

	/*DMX_ASSERT(NULL != pvPrivData);*/
	if (NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s: NULL == pvPrivData\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_NO_MEM);
	}
	/*DMX_ASSERT(prCfaAudInInst->u8CurrTxOft != (u64)CFA_AUDIN_INVALID_VALUE);*/
	if (prCfaAudInInst->u8CurrTxOft == (u64)CFA_AUDIN_INVALID_VALUE) {
		DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s: u8CurrTxOft is invalid\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	prCfaAudInInst->u4TxLen = prCfaAudInInst->u4PcmUnitSize;

#if CFA_AUDIO_IN_SUPPORT_HDMIIN
	mm_memset(&rAudInParingInfo, 0, sizeof(AUDIN_PARSING_INFO_T));
	mrResult = Spt4CfaGetAudInParsingInfo(hSpt, &rAudInParingInfo);
	/*DMX_ASSERT(RET_DMX_OK == mrResult);*/
	if (RET_DMX_OK != mrResult) {
		DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s: mrResult is not ok\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_NO_MEM);
	}
	DmxLogT(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s,Raw:%d,ReOrder:%d,Mat:%d,PcmSize:0x%x\r\n"),
			DMX_FUNC_NAME,
			rAudInParingInfo.fgAudinRAW,		rAudInParingInfo.fgAudinReOrder,
			rAudInParingInfo.AudinHBRAudioType, rAudInParingInfo.u4PsrPcmUintSize);

	if (rAudInParingInfo.u4PsrPcmUintSize > 0)
		prCfaAudInInst->u4PcmUnitSize = rAudInParingInfo.u4PsrPcmUintSize;

#endif

	/*Get data format*/
	/*PSR_SUPPORT_AudIn*/
	#if CFA_AUDIO_IN_SUPPORT_HDMIIN
	if (Spt4CfaAudInIsRAW(hSpt)) {
		prCfaAudInInst->eFormat = CFA_AUDIO_IN_RAW;
	} else
	#endif /* CFA_AUDIO_IN_SUPPORT_HDMIIN */
	{
		prCfaAudInInst->eFormat = CFA_AUDIO_IN_PCM;
	}

	if (CFA_AUDIO_IN_UNKNOWN == prCfaAudInInst->eFormat) {
		DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s in line %d, Unknown codec format!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OK);
	}

	prCfaAudInInst->eAudApiType = CfaAudInGetAudType(prCfaAudInInst);

#if CFA_AUDIO_IN_SUPPORT_HDMIIN
	/*need or not ReOrder ?*/
	if (rAudInParingInfo.fgAudinReOrder)
		prCfaAudInInst->fgReOrder = TRUE;
	else
		prCfaAudInInst->fgReOrder = FALSE;

	/*need or not mat ?*/
	if (AUDIN_HBR_MAT == rAudInParingInfo.AudinHBRAudioType)
		prCfaAudInInst->fgNeedMat = TRUE;
	else
		prCfaAudInInst->fgNeedMat = FALSE;

	if ((AUDIN_HBR_MAT == rAudInParingInfo.AudinHBRAudioType) ||
		(AUDIN_HBR_DTSMA == rAudInParingInfo.AudinHBRAudioType))
		prCfaAudInInst->fgIsHBR = TRUE;
	else
		prCfaAudInInst->fgIsHBR = FALSE;

#endif

	if (fgNeedFilterData(prCfaAudInInst)) {
		vCfaAudInNextScSearch(hSpt, prCfaAudInInst, CFA_AUDIO_IN_ST_HEAD, CFA_AUDIO_IN_BURST_HEAD_LEN);
	} else {
	#if CFA_AUDIO_IN_SUPPORT_HDMIIN
		if (prCfaAudInInst->fgReOrder || prCfaAudInInst->fgNeedMat) {
			if ((prCfaAudInInst->u4ReOrderBufValidSize + prCfaAudInInst->u4PcmUnitSize)
				< CFA_AUDIO_IN_BUFF_SIZE) {
				vCfaAudInNextScSearch(hSpt, prCfaAudInInst, CFA_HDMI_IN_ST_SYNC,
					prCfaAudInInst->u4PcmUnitSize);
				DmxLogT(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
					TEXT("[CFA_AUDIN] line %d, trun on OK\r\n"), DMX_LINE_NO);
			} else {
				DmxLogT(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
					TEXT("[CFA_AUDIN] line %d send EOS\r\n"), DMX_LINE_NO);
				Spt4CfaFinishedEx(hSpt, prCfaAudInInst->rRange.u8Ea, FALSE, GAU_E_EOS);
			}
			MM_RETURN(RET_DMX_OK);
		}
	#endif
		prCfaAudInInst->eAudInSt = CFA_AUDIO_IN_ST_TX;/*no mean*/
		prCfaAudInInst->u4TxLen = prCfaAudInInst->u4PcmUnitSize;/*CFA_AUDIO_IN_PCM_UNIT_SIZE;*/
		vData2AFifo(hSpt, prCfaAudInInst);
	}

	DmxLogT(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
		TEXT("[CFA_AUDIN] line %d, trun on OK\r\n"), DMX_LINE_NO);

	MM_RETURN(RET_DMX_OK);
}

bool fgNeedFilterData(const CfaAudInInst *prCfaAudInInst)
{
	switch (prCfaAudInInst->eFormat) {
	case CFA_AUDIO_IN_PCM:
	case CFA_AUDIO_IN_DTS14:
	case CFA_AUDIO_IN_DTS16:
		return FALSE;

	case CFA_AUDIO_IN_RAW:
		return TRUE;

	default:
		break;
	}
	DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s: eFormat is error\r\n"), DMX_FUNC_NAME);

	return FALSE;
}

void vCfaAudInNextScSearch(HANDLE hSpt, CfaAudInInst *prCfaAudInInst, ECfaAudInSt eNextSt, u32 u4TxLen)
{
	MRESULT mrResult = RET_DMX_OK;
	/*if arrived end, get next data address*/
	if ((prCfaAudInInst->u8CurrTxOft > prCfaAudInInst->rRange.u8Ea) ||
		((u64)CFA_AUDIN_INVALID_VALUE == prCfaAudInInst->u8CurrTxOft)) {
		DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s: u8CurrTxOft=0x%llx\r\n"),
			DMX_FUNC_NAME, prCfaAudInInst->u8CurrTxOft);
		return;
	}

	if (prCfaAudInInst->u4FindReadNum > CFA_AUDIO_IN_MAX_SEARCH_NUM) {
		DmxLogD(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_COMMON,
			TEXT("[CFA_AUDIN] %s: times=%d, offset=0x%llx!!\r\n"),
			DMX_FUNC_NAME, prCfaAudInInst->u4FindReadNum, prCfaAudInInst->u8CurrTxOft);
	}

	prCfaAudInInst->u4FindReadNum++;
	prCfaAudInInst->eAudInSt = eNextSt;
	prCfaAudInInst->u4TxLen = u4TxLen;
	DmxLogD(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_COMMON,
		TEXT("[CFA_AUDIN] %s in line %d: u8CurrTxOft=0x%llx, TxLen=%d!!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prCfaAudInInst->u8CurrTxOft, prCfaAudInInst->u4TxLen);
	mrResult = Spt4CfaPbb2SyncBuf(hSpt,
									prCfaAudInInst->u8CurrTxOft,
									prCfaAudInInst->u4TxLen,
									(u8 *)&(prCfaAudInInst->u4PfrMemAddress));
	/*DMX_ASSERT(mrResult == RET_DMX_OK);*/
	if (RET_DMX_OK != mrResult) {
		DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s: mrResult is not ok\r\n"), DMX_FUNC_NAME);
		return;
	}
}

void vGetAudInInfo(HANDLE hSpt, CfaAudInInst *prCfaAudInInst)
{
	/*DMX_ASSERT(NULL != prCfaAudInInst->pucAudBuf);*/
	if (NULL == prCfaAudInInst->pucAudBuf) {
		DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s: pucAudBuf is null\r\n"), DMX_FUNC_NAME);
		return;
	}
	dmx_memcpy(&(prCfaAudInInst->rAudInInfo), prCfaAudInInst->pucAudBuf, sizeof(AudInInfo_T));
	DmxLogD(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_COMMON,
		TEXT("[CFA_AUDIN] %s in line %d: AudioType=%d, Channel=%d!!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prCfaAudInInst->rAudInInfo.u4Codec,
		prCfaAudInInst->rAudInInfo.u4ChannelNum);
}

bool fgGetBurstInfo(HANDLE hSpt, CfaAudInInst *prCfaAudInInst, u16 u2BurstInfo)
{
	u8 u1DataType	= 0;
	u8 u1SubDataType = 0;
	u8 u1ErrFlag     = 0;
	u8 u1DependInfo	= 0;
	u8 u1StreamNum	= 0;
	u8 u1CodecNum	= 0;

	u1DataType	  = (u8)(u2BurstInfo & (u16)0x001F);
	u1SubDataType = (u8)(u2BurstInfo & (u16)0x0060);
	u1ErrFlag	  = (u8)(u2BurstInfo & (u16)0x0080);
	u1DependInfo  = (u8)((u2BurstInfo & (u16)0x1F00) >> 8);
	u1StreamNum   = (u8)((u2BurstInfo & (u16)0xE000) >> 8);

	prCfaAudInInst->rBurstInfo.u1DataType  = u1DataType;
	prCfaAudInInst->rBurstInfo.u1SubType   = u1SubDataType;
	prCfaAudInInst->rBurstInfo.u1ErrFlag   = u1ErrFlag;
	prCfaAudInInst->rBurstInfo.u1DepndInfo = u1DependInfo;
	prCfaAudInInst->rBurstInfo.u1StreamNum = u1StreamNum;

	if (1 == u1ErrFlag) {
		DmxLogT(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s in line %d: ErrFlag = 1,burst_payload may contain error\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
	}

	for (u1CodecNum = 0; u1CodecNum < (u8)CFA_AUDIO_IN_CODEC_NUM; u1CodecNum++) {
		if ((rValidDataType[u1CodecNum].u1DataType == u1DataType) &&
			(rValidDataType[u1CodecNum].u1SubDataType == u1SubDataType)) {
			prCfaAudInInst->u4AudInType = u1CodecNum;	/*maybe need modify value*/
			return TRUE;
		}
	}

	prCfaAudInInst->u1CurStreamNum = 0xFF;
	prCfaAudInInst->u4AudInType = 0xFFFFFFFF;

	DmxLogT(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
		TEXT("[CFA_AUDIN] %s in line %d: DataType=%d, SubDataType=%d,Skip this burst data!!!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, u1DataType, u1SubDataType);

	return FALSE;
}

void vData2AFifo(HANDLE hSpt, CfaAudInInst *prCfaAudInInst)
{
	MRESULT mrRet = RET_DMX_OK;

	if (prCfaAudInInst->u8CurrTxOft + prCfaAudInInst->u4TxLen > prCfaAudInInst->rRange.u8Ea) {
		u64 u8LastTxlen = 0;

		u8LastTxlen = prCfaAudInInst->rRange.u8Ea - prCfaAudInInst->u8CurrTxOft + 1;

		DmxLogD(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_COMMON,
			TEXT("[CFA_AUDIN] %s line %d, Offset=0x%llx, u8LastTxlen=%lld\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prCfaAudInInst->u8CurrTxOft, u8LastTxlen);

		mrRet = Spt4CfaPbb2AFifo(hSpt, prCfaAudInInst->u8CurrTxOft, 0,
			u8LastTxlen, prCfaAudInInst->eAudApiType);

		/*DMX_ASSERT(mrRet == RET_DMX_OK);*/
		if (mrRet != RET_DMX_OK) {
			DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
				TEXT("[CFA_AUDIN] %s: mrResult is not ok\r\n"), DMX_FUNC_NAME);
			return;
		}
	} else {
		DmxLogD(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_COMMON,
			TEXT("[CFA_AUDIN] %s line %d, Offset=0x%llx, TxLen=%d\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prCfaAudInInst->u8CurrTxOft, prCfaAudInInst->u4TxLen);

		mrRet = Spt4CfaPbb2AFifo(hSpt, prCfaAudInInst->u8CurrTxOft, 0,
			prCfaAudInInst->u4TxLen, prCfaAudInInst->eAudApiType);

		/*DMX_ASSERT(mrRet == RET_DMX_OK);*/
		if (mrRet != RET_DMX_OK) {
			DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
				TEXT("[CFA_AUDIN] %s: mrResult is not ok\r\n"), DMX_FUNC_NAME);
			return;
		}
	}
}

u16 u2AdjustBurstLen(const CfaAudInInst *prCfaAudInInst, u16 u2Pd)
{
	if ((17 == prCfaAudInInst->rBurstInfo.u1DataType) && (0 == prCfaAudInInst->rBurstInfo.u1SubType)) {
		DmxLogD(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_COMMON, TEXT("[CFA_AUDIN] %s line %d, u2Pd=0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u2Pd);
		return u2Pd;
	}

	if ((u2Pd & ((u16)0x0007)) > 0) {
		DmxLogD(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_COMMON,
			TEXT("[CFA_AUDIN] %s line %d, BurstLen maybe error, u2Pd=0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u2Pd);
	}

	return (u2Pd >> 3);
}

void vSearchSyncWord(HANDLE hSpt, CfaAudInInst *prCfaAudInInst, u64 u8TxLen)
{
	u16 u2Pa = 0;
	u16 u2Pb = 0;
	u16 u2Pc = 0;
	u16 u2Pd = 0;

	u32 u4ParsedLen = 0;
	u32 u4PreLen = 0;
	bool   fgIsFirst = TRUE;

	while (prCfaAudInInst->u4TxLen >= (u4ParsedLen + CFA_AUDIO_IN_BURST_HEAD_LEN)) {
		LOADB_WORD(prCfaAudInInst->pucAudBuf + u4ParsedLen, u2Pa);
		LOADB_WORD(prCfaAudInInst->pucAudBuf + u4ParsedLen + 2, u2Pb);

		if ((0XF872 == u2Pa) && (0X4E1F == u2Pb)) {
			LOADB_WORD(prCfaAudInInst->pucAudBuf + u4ParsedLen + 4, u2Pc);
			LOADB_WORD(prCfaAudInInst->pucAudBuf + u4ParsedLen + 6, u2Pd);

			if (fgIsFirst) {
				prCfaAudInInst->u8CurrTxOft =
					prCfaAudInInst->u8CurrTxOft + CFA_AUDIO_IN_BURST_HEAD_LEN
					- (prCfaAudInInst->u4TxLen - u4ParsedLen);
				u4PreLen = u4ParsedLen;
				fgIsFirst = FALSE;
			} else {
				prCfaAudInInst->u8CurrTxOft = prCfaAudInInst->u8CurrTxOft + u4ParsedLen - u4PreLen;
				u4PreLen = u4ParsedLen;
			}

			prCfaAudInInst->u4FindReadNum = 0;

			/*maybe exist multi audio ?*/
			if ((fgGetBurstInfo(hSpt, prCfaAudInInst, u2Pc)) &&
					(0 == prCfaAudInInst->rBurstInfo.u1StreamNum) &&
					(u2AdjustBurstLen(prCfaAudInInst, u2Pd) != 0) /*&&
									(prCfaAudInInst->u4AudInType != 0) &&
									(prCfaAudInInst->u4AudInType != 6)) */) {
				u2Pd = u2AdjustBurstLen(prCfaAudInInst, u2Pd);
				prCfaAudInInst->eAudInSt = CFA_AUDIO_IN_ST_SYNC;
				prCfaAudInInst->u4TxLen = u2Pd;
				prCfaAudInInst->u4FindReadNum = 0;
				vData2AFifo(hSpt, prCfaAudInInst);
				goto EXIT;
			}

			u2Pd = u2AdjustBurstLen(prCfaAudInInst, u2Pd);
			prCfaAudInInst->u8CurrTxOft += u2Pd;

			if (prCfaAudInInst->u4TxLen <
				((u4ParsedLen + CFA_AUDIO_IN_BURST_HEAD_LEN + u2Pd)
				+ CFA_AUDIO_IN_BURST_HEAD_LEN)) {
				vCfaAudInNextScSearch(hSpt, prCfaAudInInst, CFA_AUDIO_IN_ST_HEAD, CFA_AUDIO_IN_TX_1K);

				goto EXIT;
			} else {
				u4ParsedLen = u4ParsedLen + u2Pd + CFA_AUDIO_IN_BURST_HEAD_LEN;
			}
		} else {
			u4ParsedLen++;
		}
	}

	if ((!fgIsFirst) && (u4ParsedLen > u4PreLen)) {
		prCfaAudInInst->u8CurrTxOft = prCfaAudInInst->u8CurrTxOft + u4ParsedLen - u4PreLen - 1;
		u4PreLen = u4ParsedLen;
	}

	if (prCfaAudInInst->u8CurrTxOft >= ((u64)CFA_AUDIO_IN_BURST_HEAD_LEN)) {
		prCfaAudInInst->u8CurrTxOft = prCfaAudInInst->u8CurrTxOft - CFA_AUDIO_IN_BURST_HEAD_LEN + 1;
	} else {
		DmxLogD(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_COMMON,
			TEXT("[CFA_AUDIN] %s line %d, CurOffset=0x%llx\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prCfaAudInInst->u8CurrTxOft);
	}

	vCfaAudInNextScSearch(hSpt, prCfaAudInInst, CFA_AUDIO_IN_ST_HEAD, CFA_AUDIO_IN_TX_1K);
EXIT:
	return;
}

static MRESULT CfaAudInTxDoneCtrl_PMCDTS(HANDLE hSpt, u64 u8TxLen, CfaAudInInst *prCfaAudInInst)
{
	u32	u4ReOrderWordNum = 0;
	u32	u4TxLen = (u32)u8TxLen;
	u32	u4RemainLen = 0;
	CFA_AUDIO_INFO_T rTxAudInfo;
	MRESULT mrResult = RET_DMX_OK;

	mm_memset(&rTxAudInfo, 0, sizeof(CFA_AUDIO_INFO_T));

	if ((!prCfaAudInInst->fgReOrder) && (!prCfaAudInInst->fgNeedMat))
		MM_RETURN(RET_DMX_OK);

	if (CFA_HDMI_IN_ST_SYNC == prCfaAudInInst->eAudInSt) {
		if (prCfaAudInInst->fgReOrder) {
			/*todo*/
			if (prCfaAudInInst->fgIsHBR) {
				for (u4ReOrderWordNum = 0;
					(u4ReOrderWordNum + 16) <= u4TxLen; u4ReOrderWordNum += 16) {
					/* HBR is Little-Endian*/
				    LOADL_WORD(prCfaAudInInst->pucAudBuf + u4ReOrderWordNum,
						*(u16 *)(prCfaAudInInst->pucReOrderBuf
						+ prCfaAudInInst->u4ReOrderBufValidSize));
					LOADL_WORD(prCfaAudInInst->pucAudBuf + u4ReOrderWordNum + 8,
						*(u16 *)(prCfaAudInInst->pucReOrderBuf
						+ prCfaAudInInst->u4ReOrderBufValidSize + 2));
					LOADL_WORD(prCfaAudInInst->pucAudBuf + u4ReOrderWordNum + 2,
						*(u16 *)(prCfaAudInInst->pucReOrderBuf
						+ prCfaAudInInst->u4ReOrderBufValidSize + 4));
					LOADL_WORD(prCfaAudInInst->pucAudBuf + u4ReOrderWordNum + 10,
						*(u16 *)(prCfaAudInInst->pucReOrderBuf
						+ prCfaAudInInst->u4ReOrderBufValidSize + 6));
					LOADL_WORD(prCfaAudInInst->pucAudBuf + u4ReOrderWordNum + 4,
						*(u16 *)(prCfaAudInInst->pucReOrderBuf
						+ prCfaAudInInst->u4ReOrderBufValidSize + 8));
					LOADL_WORD(prCfaAudInInst->pucAudBuf + u4ReOrderWordNum + 12,
						*(u16 *)(prCfaAudInInst->pucReOrderBuf
						+ prCfaAudInInst->u4ReOrderBufValidSize + 10));
					LOADL_WORD(prCfaAudInInst->pucAudBuf + u4ReOrderWordNum + 6,
						*(u16 *)(prCfaAudInInst->pucReOrderBuf
						+ prCfaAudInInst->u4ReOrderBufValidSize + 12));
					LOADL_WORD(prCfaAudInInst->pucAudBuf + u4ReOrderWordNum + 14,
						*(u16 *)(prCfaAudInInst->pucReOrderBuf
						+ prCfaAudInInst->u4ReOrderBufValidSize + 14));

					prCfaAudInInst->u4ReOrderBufValidSize += 16;

					u4RemainLen += 16;
				}
			} else {
				for (u4ReOrderWordNum = 0;
					(u4ReOrderWordNum + 24) <= u4TxLen; u4ReOrderWordNum += 24) {
					LOADB_3BYTES(prCfaAudInInst->pucAudBuf + u4ReOrderWordNum,
						(prCfaAudInInst->pucReOrderBuf +
						prCfaAudInInst->u4ReOrderBufValidSize));
					LOADB_3BYTES(prCfaAudInInst->pucAudBuf + u4ReOrderWordNum + 12,
						(prCfaAudInInst->pucReOrderBuf +
						prCfaAudInInst->u4ReOrderBufValidSize + 3));
					LOADB_3BYTES(prCfaAudInInst->pucAudBuf + u4ReOrderWordNum + 3,
						(prCfaAudInInst->pucReOrderBuf +
						prCfaAudInInst->u4ReOrderBufValidSize + 6));
					LOADB_3BYTES(prCfaAudInInst->pucAudBuf + u4ReOrderWordNum + 15,
						(prCfaAudInInst->pucReOrderBuf +
						prCfaAudInInst->u4ReOrderBufValidSize + 9));
					LOADB_3BYTES(prCfaAudInInst->pucAudBuf + u4ReOrderWordNum + 6,
						(prCfaAudInInst->pucReOrderBuf +
						prCfaAudInInst->u4ReOrderBufValidSize + 12));
					LOADB_3BYTES(prCfaAudInInst->pucAudBuf + u4ReOrderWordNum + 18,
						(prCfaAudInInst->pucReOrderBuf +
						prCfaAudInInst->u4ReOrderBufValidSize + 15));
					LOADB_3BYTES(prCfaAudInInst->pucAudBuf + u4ReOrderWordNum + 9,
						(prCfaAudInInst->pucReOrderBuf +
						prCfaAudInInst->u4ReOrderBufValidSize + 18));
					LOADB_3BYTES(prCfaAudInInst->pucAudBuf + u4ReOrderWordNum + 21,
						(prCfaAudInInst->pucReOrderBuf +
						prCfaAudInInst->u4ReOrderBufValidSize + 21));

					prCfaAudInInst->u4ReOrderBufValidSize += 24;

					u4RemainLen += 24;
				}
			}
			u4RemainLen = u4TxLen - u4RemainLen;
		} else {
			dmx_memcpy(
				(prCfaAudInInst->pucReOrderBuf +
				prCfaAudInInst->u4ReOrderBufValidSize),
				prCfaAudInInst->pucAudBuf, u4TxLen
				);

			prCfaAudInInst->u4ReOrderBufValidSize += (u32)u8TxLen;

			u4RemainLen = 0;
		}
		if ((u4RemainLen != 0) && ((u32)(prCfaAudInInst->u8CurrTxOft) > u4RemainLen))
			prCfaAudInInst->u8CurrTxOft -= u4RemainLen;
		else if ((u32)(prCfaAudInInst->u8CurrTxOft) < u4RemainLen) {
			DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
				TEXT("[CFA_AUDIN] %s line %d, ReOrder is error\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		}

		if (prCfaAudInInst->fgNeedMat) {
			if ((prCfaAudInInst->u4ReOrderBufValidSize + prCfaAudInInst->u4PcmUnitSize)
				< CFA_AUDIO_IN_BUFF_SIZE) {
				if (prCfaAudInInst->u4ReOrderBufValidSize
					>= AUDIN_REORDER_BUF_VALID_SIZE) {
					prCfaAudInInst->eAudInSt = CFA_HDMI_IN_ST_MAT;
					DmxLogT(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
						TEXT("[CFA_AUDIN] %s line %d, CFA_HDMI_IN_ST_MAT\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
					/*call mat interface,mat end ?*/
				#if CFA_AUDIN_SUPPORT_MAT
					prCfaAudInInst->rAudInMatDecInfo.u4ReorderBufPosition = 0;
					i4Ret = i4CfaAudInMatDecProc(hSpt, prCfaAudInInst);
					if (CFA_MAT_RETURN_CALL_TXTDONE == i4Ret) {
						vCfaAudInNextScSearch(hSpt, prCfaAudInInst,
									CFA_HDMI_IN_ST_SYNC,
									prCfaAudInInst->u4PcmUnitSize);
					} else if (CFA_MAT_RETURN_CALL_MAC_DEC == i4Ret) {
						CfaAudInTxDoneCtrl(hSpt, 0, prCfaAudInInst);
					}
				#endif
				} else {
					vCfaAudInNextScSearch(hSpt, prCfaAudInInst,
						CFA_HDMI_IN_ST_SYNC, prCfaAudInInst->u4PcmUnitSize);
					/*CFA_AUDIO_IN_PCM_UNIT_SIZE*/
				}
			} else {
				DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
					TEXT("[CFA_AUDIN] line %d send EOS\r\n"), DMX_LINE_NO);
				Spt4CfaFinishedEx(hSpt, prCfaAudInInst->rRange.u8Ea, FALSE, GAU_E_EOS);
			}
		} else {
			/*pCfaDrvIntf->pfi4Splitter4CfaBuf2AFifo*/
			DmxLogT(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
				TEXT("[CFA_AUDIN] %s line %d, Offset=0x%llx, TxLen=%d\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				prCfaAudInInst->u8CurrTxOft, prCfaAudInInst->u4ReOrderBufValidSize);
				/*
			i4Ret = prCfaAudInInst->pCfaDrvIntf->pfi4Splitter4CfaBuf2AFifo(hSpt,
				prCfaAudInInst->pucReOrderBuf,
				0,
				(u32)INVALID_TIMESTAMP,
				u4TxLen
				#if  CFA_AAC_UID_TEST
				,0
				#endif
				);
			*/
			mm_memset(&rTxAudInfo, 0, sizeof(CFA_AUDIO_INFO_T));
			rTxAudInfo.u8FileOfst = 0;
			rTxAudInfo.u8Len = (u64)prCfaAudInInst->u4ReOrderBufValidSize;
			rTxAudInfo.u8Pts = (u64)INVALID_TIMESTAMP; /*change unit in Hz, STC Clock*/
			rTxAudInfo.u4PrsStrmId = 0;/*need modify*/
			rTxAudInfo.eAudType = prCfaAudInInst->eAudApiType;

			rTxAudInfo.fgUnitStart = TRUE;

			prCfaAudInInst->u4ReOrderBufValidSize = 0;

			prCfaAudInInst->eAudInSt = CFA_HDMI_IN_ST_NEED_SYNC;

			mrResult = Spt4CfaBuf2AFifoAUCtrl(hSpt,
							prCfaAudInInst->pucReOrderBuf,
							&rTxAudInfo,
							rTxAudInfo.u8Len);
			/*DMX_ASSERT(mrResult == RET_DMX_OK);*/
			if (mrResult != RET_DMX_OK) {
				DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
				TEXT("[CFA_AUDIN] %s: mrResult is not ok\r\n"), DMX_FUNC_NAME);
				MM_RETURN(RET_DMX_OK);
			}
		}
	}	else if (CFA_HDMI_IN_ST_MAT == prCfaAudInInst->eAudInSt) {
		/*call mat interface,if mat ended,need set CFA_HDMI_IN_ST_SYNC
		to prCfaAudInInst->eAudInSt
		and set 0 to prCfaAudInInst->u4ReOrderBufValidSize*/

		DmxLogT(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s line %d, CFA_HDMI_IN_ST_MAT\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
	#if CFA_AUDIN_SUPPORT_MAT
		i4Ret = i4CfaAudInMatDecProc(hSpt, prCfaAudInInst);
		if (CFA_MAT_RETURN_CALL_TXTDONE == i4Ret) {
			vCfaAudInNextScSearch(hSpt, prCfaAudInInst,
				CFA_HDMI_IN_ST_SYNC, prCfaAudInInst->u4PcmUnitSize);
			/*CFA_AUDIO_IN_PCM_UNIT_SIZE*/
		} else if (CFA_MAT_RETURN_CALL_MAC_DEC == i4Ret) {
			CfaAudInTxDoneCtrl(hSpt, 0, prCfaAudInInst);
		}
	#endif
	} else if (CFA_HDMI_IN_ST_NEED_SYNC == prCfaAudInInst->eAudInSt) {
		vCfaAudInNextScSearch(hSpt, prCfaAudInInst,
			CFA_HDMI_IN_ST_SYNC, prCfaAudInInst->u4PcmUnitSize);
	} else {
		DmxLogT(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s line %d, AudioSt=%d\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prCfaAudInInst->eAudInSt);
	}

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaAudInTxDoneCtrl(HANDLE hSpt, u64 u8TxLen, CfaAudInInst *prCfaAudInInst)
{
#if CFA_AUDIO_IN_SUPPORT_HDMIIN
	MRESULT mrResult = RET_DMX_OK;
#endif

	prCfaAudInInst->u8CurrTxOft += u8TxLen;

	prCfaAudInInst->pucAudBuf = (u8 *)prCfaAudInInst->u4PfrMemAddress;

	DmxLogD(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_COMMON,
		TEXT("%[CFA_AUDIN] s line %d, pucAudBuf=0x%x\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prCfaAudInInst->pucAudBuf);

	if (prCfaAudInInst->u8CurrTxOft >= prCfaAudInInst->rRange.u8Ea) /*no come in*/ {
		DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s line %d, CurrTxOft=0x%llx, Ea=0x%llx\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prCfaAudInInst->u8CurrTxOft, prCfaAudInInst->rRange.u8Ea);
		MM_RETURN(RET_DMX_EXT_EXCEPTION);
	}
	/*	  need not notify
	if (CFA_AUDIO_IN_ST_QUER == prCfaAudInInst->eAudInSt) {
		vGetAudInInfo(hSpt, prCfaAudInInst);
		i4Ret = prCfaAudInInst->pCfaDrvIntf->pfi4Splitter4CfaInqInfNotify(hSpt, prCfaAudInInst->eQueryType);
		DMX_ASSERT(i4Ret >= 0);
		MM_RETURN(RET_DMX_OK);
	}
	*/
	switch (prCfaAudInInst->eFormat) {
	case CFA_AUDIO_IN_PCM:
	case CFA_AUDIO_IN_DTS14:
	case CFA_AUDIO_IN_DTS16: {
		#if CFA_AUDIO_IN_SUPPORT_HDMIIN
		mrResult = CfaAudInTxDoneCtrl_PMCDTS(hSpt, u8TxLen, prCfaAudInInst);
		#endif

		prCfaAudInInst->eAudInSt = CFA_AUDIO_IN_ST_TX;
		prCfaAudInInst->u4TxLen = prCfaAudInInst->u4PcmUnitSize;/*CFA_AUDIO_IN_PCM_UNIT_SIZE;*/
		vData2AFifo(hSpt, prCfaAudInInst);
	}
	break;

	case CFA_AUDIO_IN_RAW:
		if (prCfaAudInInst->eAudInSt == CFA_AUDIO_IN_ST_IDL) {
			/*no come in*/
			vCfaAudInNextScSearch(hSpt, prCfaAudInInst, CFA_AUDIO_IN_ST_INFO, sizeof(AudInInfo_T));
		} else if (prCfaAudInInst->eAudInSt == CFA_AUDIO_IN_ST_INFO) {
			/*Get Audio In Info*/
			vGetAudInInfo(hSpt, prCfaAudInInst);
			vCfaAudInNextScSearch(hSpt, prCfaAudInInst,
				CFA_AUDIO_IN_ST_HEAD, CFA_AUDIO_IN_BURST_HEAD_LEN);
		} else if (prCfaAudInInst->eAudInSt == CFA_AUDIO_IN_ST_HEAD) {
			prCfaAudInInst->eAudInSt = CFA_AUDIO_IN_ST_TX;
			/*Analyze codec,data type,etc*/
			vSearchSyncWord(hSpt, prCfaAudInInst, u8TxLen);
		} else if (prCfaAudInInst->eAudInSt == CFA_AUDIO_IN_ST_TX) {
			/*no come in*/
			prCfaAudInInst->eAudInSt = CFA_AUDIO_IN_ST_SYNC;
			vData2AFifo(hSpt, prCfaAudInInst);
		} else if (prCfaAudInInst->eAudInSt == CFA_AUDIO_IN_ST_SYNC) {
			vCfaAudInNextScSearch(hSpt, prCfaAudInInst,
				CFA_AUDIO_IN_ST_HEAD, CFA_AUDIO_IN_BURST_HEAD_LEN);
		} else {
			DmxLogD(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_COMMON,
				TEXT("[CFA_AUDIN] %s line %d, AudioSt=%d\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prCfaAudInInst->eAudInSt);
		}
		break;

#if CFA_AUDIN_SUPPORT_MAT
	case CFA_AUDIO_IN_MAT:
		if (prCfaAudInInst->eAudInSt == CFA_AUDIO_IN_MAT_UNLOCKED) {
			vCfaAudInMatUnlockProc(hSpt, prCfaAudInInst);
		} else if (prCfaAudInInst->eAudInSt == CFA_AUDIO_IN_MAT_CHECKING_MAIN_HEADER) {
			vCfaAudInMatCheckMainHeaderProc(hSpt, prCfaAudInInst);
		} else if (prCfaAudInInst->eAudInSt == CFA_AUDIO_IN_MAT_CHECKING_TOC_HEADER) {
			vCfaAudInMatCheckTocHeaderProc(hSpt, prCfaAudInInst);
		} else if (prCfaAudInInst->eAudInSt == CFA_AUDIO_IN_MAT_CHECKING_BOC_HEADER) {
			vCfaAudInMatCheckBocHeaderProc(hSpt, prCfaAudInInst);
		} else if (prCfaAudInInst->eAudInSt == CFA_AUDIO_IN_MAT_LOADING_TOC_PAYLOAD) {
			vCfaAudInMatLoadTocPayloadProc(hSpt, prCfaAudInInst);
		} else if (prCfaAudInInst->eAudInSt == CFA_AUDIO_IN_MAT_LOADING_BOC_PAYLOAD) {
			vCfaAudInMatLoadBocPayloadProc(hSpt, prCfaAudInInst);
		} else if (prCfaAudInInst->eAudInSt == CFA_AUDIO_IN_MAT_CHECKING_TOC_FOOTER) {
			vCfaAudInMatCheckTocFooterProc(hSpt, prCfaAudInInst);
		} else if (prCfaAudInInst->eAudInSt == CFA_AUDIO_IN_MAT_CHECKING_BOC_FOOTER) {
			vCfaAudInMatCheckBocFooterProc(hSpt, prCfaAudInInst);
		} else {
			DmxLogT(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
				TEXT("[CFA_AUDIN] %s line %d, AudioSt=%d\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prCfaAudInInst->eAudInSt);
		}
		break;
#endif

	default:
		DmxLogT(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s line %d, eFormat=%d\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prCfaAudInInst->eFormat);
		break;
	}

	MM_RETURN(mrResult);
}

static MRESULT CfaAudInTxDone(HANDLE hSpt, u64 u8TxLen, void *pvPrivData, bool fgRsp)
{
	MRESULT mrResult = RET_DMX_OK;

	CfaAudInInst *prCfaAudInInst = (CfaAudInInst *)pvPrivData;

	mrResult = CfaAudInTxDoneCtrl(hSpt, u8TxLen, prCfaAudInInst);

	MM_RETURN(mrResult);
}

static MRESULT CfaAudInFillAUInfo(HANDLE hSpt, void *pvAUInfo, void *pvAUExtInfo, void *pvPrivData)
{
	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaAudInSetInqTypes(HANDLE hSpt, u32 u4InfTypes, void *pvPrivData)
{
	CfaAudInInst *prCfaAudInInst = (CfaAudInInst *)pvPrivData;

	/*DMX_ASSERT(NULL != prCfaAudInInst);*/
	if (NULL == prCfaAudInInst) {
		DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s: Alloc prCfaAudInInst memory fail\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_NO_MEM);
	}

	prCfaAudInInst->u4QueryType = CFA_AUDIN_QUERY_TYPE_NONE;

	if (CFA_AUDIN_QUERY_TYPE_CHANNEL & u4InfTypes)
		prCfaAudInInst->u4QueryType |= CFA_AUDIN_QUERY_TYPE_CHANNEL;

	if (CFA_AUDIN_QUERY_TYPE_CODEC & u4InfTypes)
		prCfaAudInInst->u4QueryType |= CFA_AUDIN_QUERY_TYPE_CODEC;

	vCfaAudInNextScSearch(hSpt, prCfaAudInInst, CFA_AUDIO_IN_ST_QUER, sizeof(AudInInfo_T));

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaAudInGetGeneral(HANDLE hSpt,
									  u32 u4CfaFID,
									  void *pvPrivData,
									  void *pvCfaParameter,
									  u32 u4CfaParameterSize)
{
	CfaAudInInst *prCfaAudInInst = (CfaAudInInst *)pvPrivData;

	/*DMX_ASSERT(u4CfaParameterSize == sizeof(AudInInfo_T));*/
	if (u4CfaParameterSize != sizeof(AudInInfo_T)) {
		DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s: Alloc  memory fail\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	switch (u4CfaFID) {
	case CFA_AUDIN_QUERY_TYPE_CHANNEL:
		mm_memcpy(pvCfaParameter, &(prCfaAudInInst->rAudInInfo.u4ChannelNum), sizeof(u32));
		break;

	case CFA_AUDIN_QUERY_TYPE_CODEC:
		mm_memcpy(pvCfaParameter, &(prCfaAudInInst->rAudInInfo.u4Codec), sizeof(u32));
		break;

	case CFA_AUDIN_QUERY_TYPE_CHNL_CODEC:
		mm_memcpy(pvCfaParameter, &(prCfaAudInInst->rAudInInfo), sizeof(AudInInfo_T));
		break;

	default:
		DmxLogE(DMX_MOD_CFA_AUDIN, CFA_AUDIN_LOG_DEFAULT,
			TEXT("[CFA_AUDIN] %s: Alloc u4CfaFID is not invalid\r\n"), DMX_FUNC_NAME);

		break;
	}

	MM_RETURN(RET_DMX_OK);
}

CfaIntf _rAudInCfaIntf = {
	&CfaAudInInit,
	&CfaAudInUninit,
	&CfaAudInSetRange,
	&CfaAudInEnableStrm,
	&CfaAudInSetStrmInf,
	&CfaAudInTurnOn,
	&CfaAudInTxDone,
	NULL,
	NULL,
	&CfaAudInConfigure,
	&CfaAudInSetInqTypes,
	&CfaAudInGetGeneral,
	NULL,
	NULL,
	&CfaAudInFillAUInfo,
	NULL,
	NULL
};

void *CfaAudInGetInterface(void)
{
	return ((void *)&_rAudInCfaIntf);
}

