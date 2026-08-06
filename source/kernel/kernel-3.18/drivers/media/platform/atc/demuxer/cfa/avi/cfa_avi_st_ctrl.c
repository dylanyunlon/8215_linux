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




/*-----------------------------------------------------------------------------
		include files
-----------------------------------------------------------------------------*/

#include "cfa_avi.h"
#include "cfa_avi_st_ctrl.h"
#include "cfa_macro.h"
#include "dmx_mem.h"
#include "dmx_cpsa.h"
#include "dmx_spt.h"


/*-----------------------------------------------------------------------------
		data declarations
-----------------------------------------------------------------------------*/


#if CONFIG_CFA_AVI_NEW_SYNC_PB_BUF_FLOW
#define AVI_MAX_NOT_SYNC_PB_BUF_TIME        10
#endif

#define AVI_TX_DONE_DBG_LOG_ON              0


/*-----------------------------------------------------------------------------
		macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
		function prototype
-----------------------------------------------------------------------------*/
static void CfaAviMoviSrchSc(void *pvSptHdl, CfaAviInst *prCfaAvi, u32 u4Range);

static bool CfaAviRETxMPEG1CocecHeader(void *pvSptHdl, CfaAviInst *prCfaAvi);

static bool CfaAviRETxMPEG4VolHeader(void *pvSptHdl, CfaAviInst *prCfaAvi);

#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
static bool CfaAviJudgeVbrGarbageData(void *pvSptHdl, CfaAviInst *prCfaAvi);
static void CfaAviAnaStDetectVbrGarbageData(void *pvSptHdl, CfaAviInst *prCfaAvi);
static bool CfaAviGetVbrGarbageAudioPts(void *pvSptHdl, CfaAviInst *prCfaAvi, TCfaAviStrmInf *ptAStrmInf);
#endif

static void CfaAviDrmProcBeforeTxData(void *pvSptHdl, CfaAviInst *prCfaAvi);

#if CONFIG_CFA_AVI_SUPPORT_AUDIO_DRM
static bool CfaAviDrmProcBeforeTxAudData(void *pvSptHdl,const CfaAviInst *prCfaAvi);
#endif

#if CONFIG_CFA_AVI_DIVIDE_LARGE_AUD_CHUNK
static bool CfaAviTxDividedAudData(void *pvSptHdl, CfaAviInst *prCfaAvi);
#endif

#if CONFIG_CFA_AVI_SUPPORT_CORRECT_CHUNK_ABNORMITY
static bool CfaAviProcAbnormalChunk(void *pvSptHdl, CfaAviInst *prCfaAvi, u8 *pucHdrBuf);
#endif

#if CONFIG_CFA_AVI_SUPPORT_FIFO_LIMITATION
static bool CfaAviTooLargeChunkForFifo(void *pvSptHdl, CfaAviInst *prCfaAvi, u8 *pucHdrBuf);
#endif

static bool CfaAviNotiDmxError(void *pvSptHdl, CfaAviInst *prCfaAvi);
static bool CfaAviNotiDmxChunkError(void *pvSptHdl, CfaAviInst *prCfaAvi);
static bool CfaAviMatchChunkId(u8 bType, volatile u8 *pbBuf, u8 bIdx);
static bool CfaAviMatchAudCkId(volatile u8 *pbBuf);
static bool CfaAviMatchSpCkId(volatile u8 *pbBuf);
static BOOL CfaAviRETxH264H265Header(void *pvSptHdl, CfaAviInst* prCfaAvi);

#if AVI_DRM_DEBUG_LOG_ON
#define AVI_CFA_PRINT_ADDR_FOR_DUMP(u8Offset)	\
							DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,\
							TEXT("[0x%08x 0x%08x]: ")\
							, (u32)((u8Offset & (0xFFFFFFFF00000000ll))>>32)\
							, (u32)(u8Offset & (0x00000000FFFFFFFFll)))

static void vDumpData(u8 *pu1Buf, u32 u4Len, u64 u8Offset)
{
	u32 u4AlgDataLen = (u32)(u8Offset%16);
	u64 u8PrintCA = (u8Offset / 16) * 16;
	u64 u8BufEA	 = u8Offset + u4Len;
	u32 u4BufIdx  = 0;

	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON, TEXT("[CFA AVI][AVI DRM]========= vDumpData =============\r\n"));

	AVI_CFA_PRINT_ADDR_FOR_DUMP(u8PrintCA);

	while (u8PrintCA < u8Offset) {
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON, TEXT("-- "));

		if ((u8PrintCA + 1)%4 == 0)
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON, TEXT("  "));

		if ((u8PrintCA + 1)%16 == 0)
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON, TEXT("\r\n"));

		++u8PrintCA;
	}

	while (u8PrintCA < u8BufEA) {
		u4BufIdx = (u32)(u8PrintCA - u8Offset);
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON, TEXT("%02x "), pu1Buf[u4BufIdx]);

		if ((u8PrintCA + 1)%4 == 0)
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON, TEXT("  "));

		if ((u8PrintCA + 1)%16 == 0) {
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON, TEXT("\r\n"));

			if (u8PrintCA + 1 < u8BufEA)
				AVI_CFA_PRINT_ADDR_FOR_DUMP(u8PrintCA + 1);
		}

		++u8PrintCA;
	}

	while (0 != (u8PrintCA % 16)) {
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON, TEXT("-- "));

		if ((u8PrintCA + 1)%4 == 0)
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON, TEXT("  "));

		++u8PrintCA;
	}

	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON, TEXT("\r\n"));
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON, TEXT("[CFA AVI][AVI DRM]===== vDumpData END =======\r\n"));
}
#endif
/*-----------------------------------------------------------------------------
 * Name: fgCfaAviIsAsciiNum
 *
 * Description:
 *		check if the given byte is an ASCII number
 *		original function: fgIsAsciiNum
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: TRUE: it's an ASCII number;  FALSE: not an ASCII number
 *
 *-----------------------------------------------------------------------------*/
static bool CfaAviIsAsciiNum(u8 bVal)
{
	if ((bVal >= (u8)'0') && (bVal <= (u8)'9'))
		return TRUE;

	return FALSE;
}


/*-----------------------------------------------------------------------------
 * Name: UtStrCmp
 *
 * Description:
 *		String Compare
 *
 * Inputs:pbSrc1: string1, pbSrv2: String2, wLen: String Length
 *
 * Outputs:
 *
 * Returns: TRUE: 2 string same; FALSE: 2 string different
 *
 *-----------------------------------------------------------------------------*/
bool UtStrCmp(const u8 *pbSrc1, const u8 *pbSrc2, u32 u4Len)
{
	u32 i;

	for (i = 0; i < u4Len; i++) {
		if (pbSrc1[i] != pbSrc2[i])
			return FALSE;

	}
	return TRUE;
}

/*-----------------------------------------------------------------------------
 * Name: fgCfaAviIsDMCChunkId
 *
 * Description:
 *		check if pbBuf has known useless chunk ID to skip
 *		original function: fgIsKnownChunkId
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: TRUE: known;  FALSE: unknown
 *
 *-----------------------------------------------------------------------------*/
static bool fgCfaAviIsDMCChunkId(const u8 *pbBuf)
{
	u8 ppbStrs[4][5] = {{"MENU"}, {"LGMN"}, {"WMEN"}, {"BMEN"}/*, "MREC",
						"PLAY", "MNTX", "BNTX", "MDIA", "MEDS", "MEDT",
						"ACTN", "STAC", "ASAC", "MNTK", "TITL", "CHAP",
						"TRTB", "TRLK", "TRAN"*/};
	u8 i = 0;
	u8 bStrArrLen = 0;

	bStrArrLen = sizeof(ppbStrs) / sizeof(ppbStrs[0]);

	for (i = 0; i < bStrArrLen; i++) {
		if (UtStrCmp(ppbStrs[i], pbBuf, (u32)AVI_4CC_BYTES))
			return TRUE;
	}

	return FALSE;
}


/*-----------------------------------------------------------------------------
 * Name: fgCfaAviIsKnownChunkId
 *
 * Description:
 *		check if pbBuf has known useless chunk ID to skip
 *		original function: fgIsKnownChunkId
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: TRUE: known;  FALSE: unknown
 *
 *-----------------------------------------------------------------------------*/
static bool CfaAviIsKnownChunkId(u8 *pbBuf)
{
	#if CONFIG_CFA_DIVX_DMC_SUPPORT
	u8 ppbStrs[8][5] = {{"JUNK"}, {"RES2"}, {"RES1"}, {"RES3"}, {"idx1"}, {"1idx"}, {"0032"}, {"00iv"}};
	#else
	u8 ppbStrs[1][5] = {"JUNK"};
	#endif

	u8 i = 0;
	u8 bStrArrLen = 0;
	u8 szVidF4CC[5] = "00id";

	bStrArrLen = sizeof(ppbStrs) / sizeof(ppbStrs[0]);

	/*for (i = 0; ppbStrs[i][0] != '\0'; i++)*/
	for (i = 0; i < bStrArrLen; i++) {
		if (UtStrCmp(ppbStrs[i], pbBuf, (u32)AVI_4CC_BYTES))
			return TRUE;
	}
	/* Todo: Temp, cvid video codec, maybe also identified as "00dc" (can remove following)*/
	if (UtStrCmp(szVidF4CC, pbBuf, (u32)AVI_4CC_BYTES))
		return TRUE;

	return FALSE;
}


/*-----------------------------------------------------------------------------
 * Name: CfaAviKnown4cc
 *
 * Description:
 *		Check if the 4cc is known
 *		original function: fgAviKnown4cc
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: TRUE: known 4cc;  FALSE: unknown
 *
 *-----------------------------------------------------------------------------*/
static bool CfaAviIsKnownExtStr(u8 *pbBuf, bool fgSufx)
{
	/* "dd" maybe need judgment as CfaAviKnown4cc. "ix" ??*/
	u8 ppbSufxes[3][3] = {{"dd"}, {"st"}, {""}};

	#if CONFIG_CFA_DIVX_DMC_SUPPORT
	/* 00dc -> dc00, 01wb -> wb01, 02sb -> sb02 in DivX 6.0 Media Menu */
	u8 ppbPrfxes[4][3] = {{"ix"}, {"dd"}, {"st"}, {""}};
	#else
	u8 ppbPrfxes[2][3] = {{"ix"}, {""}};
	#endif

	u8 (*ppbStrs)[3] = {NULL};
	u8 i = 0;

	if (fgSufx)
		ppbStrs = ppbSufxes;
	else
		ppbStrs = ppbPrfxes;

	for (i = 0; ppbStrs[i][0] != (u8)'\0'; i++) {
		if (UtStrCmp(ppbStrs[i], pbBuf, (u32)2))
			return TRUE;
	}

	return FALSE;
}

static bool CfaAviKnown4cc(CfaAviInst *prCfaAvi, u8 *pbBuf)
{
	bool fgSufx = TRUE;
	u16 u2StrmIdx = 0;
	u8 *pbBufTmp = NULL;

	if ((CfaAviIsAsciiNum(pbBuf[0])) && (CfaAviIsAsciiNum(pbBuf[1]))) {
		fgSufx = TRUE;
		u2StrmIdx = (u16)((pbBuf[0] - (u8)'0') * ((u8)10) + (pbBuf[1] - (u8)'0'));
		pbBufTmp = pbBuf + 2;
	} else if ((CfaAviIsAsciiNum(pbBuf[2])) && (CfaAviIsAsciiNum(pbBuf[3]))) {
		fgSufx = FALSE;
		u2StrmIdx = (u16)((pbBuf[2] - (u8)'0') * ((u8)10) + (pbBuf[3] - (u8)'0'));
		pbBufTmp = pbBuf;
	} else if (CfaAviIsKnownChunkId(pbBuf)) {
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:CfaAviKnown4cc, known, ca:0x%llx\r\n"), prCfaAvi->u8Ca);
		return TRUE;
	} else
		return FALSE;

	/* Todo: audio and sp maybe can be judged in same function fgCfaAviMatchChunkId as video...
	One video stream, can return true directly*/
	if (CfaAviMatchChunkId(CFA_AVI_PRS_BIT_STRM_TYPE_V, pbBuf,
		(u8)(prCfaAvi->rCfaAviVInf.tStrmInf.u4StrmIdx))) {
		/*DMX_LogE(TEXT("[CFA AVI] CfaAviKnown4cc, got video, ca:0x%llx\r\n"), prCfaAvi->u8Ca);
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:CfaAviKnown4cc, got video, ca:0x%llx\r\n"), prCfaAvi->u8Ca);*/
		return TRUE;
	} else if (CfaAviMatchAudCkId(pbBufTmp)) {
		u8 uAudIdx = 0;
		for (uAudIdx = 0; uAudIdx < (u8)MAX_NS_AVI_AUD; uAudIdx++) {
			if ((TRUE == (prCfaAvi->rCfaRange).rAudioRange[uAudIdx].fgIsValid) &&
				(u2StrmIdx == prCfaAvi->rCfaAviAInf[uAudIdx].tStrmInf.u4StrmIdx)) {
				DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
					TEXT("[CFA AVI]:CfaAviKnown4cc, got audio 01, ca:0x%llx, u2StrmIdx:0x%x\r\n"),
					prCfaAvi->u8Ca, u2StrmIdx);
				return TRUE;
			}
		}
	} else if (CfaAviMatchSpCkId(pbBufTmp)) {
		u8 uSpIdx = 0;

		for (uSpIdx = 0; uSpIdx < MAX_NS_AVI_INTERNAL_SP; uSpIdx++) {
			/* Todo: need LPE give valid sp strm index for comparing, for ignore particular cases: '00sb'*/
			if (u2StrmIdx == prCfaAvi->rCfaAviSpInf[uSpIdx].tStrmInf.u4StrmIdx) {
				DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
					TEXT("[CFA AVI]:CfaAviKnown4cc, got audio 02, ca:0x%llx, u2StrmIdx:0x%x\r\n"),
					prCfaAvi->u8Ca, u2StrmIdx);
				return TRUE;
			}
		}
	} else if (CfaAviIsKnownExtStr(pbBufTmp, fgSufx)) {
		/*mtk40156 20090224,for 222047,pbBuf -->pbBufTmp */
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:CfaAviKnown4cc, Known Ext Str, ca:0x%llx, fgSufx:0x%x\r\n"),
		prCfaAvi->u8Ca, fgSufx);
		return TRUE;
	} else {
		/*do nothing*/
	}

	return FALSE;
}



/*-----------------------------------------------------------------------------
 * Name: CfaAviMatchAudCkId
 *
 * Description:
 *		check if the 4cc in pbBuf is audio type
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: TRUE: match.  FALSE: not match
 *
 *-----------------------------------------------------------------------------*/
static bool CfaAviMatchAudCkId(volatile u8 *pbBuf)
{
	u8 ppbASufx[][2] = {{'w', 'b'} , {'\0', '\0'} };

	u16 u2KnownSc = 0;
	u16 u2Sc = 0;
	u16 u2Sc1 = 0;
	int i = 0;

	LOADL_WORD(pbBuf, u2Sc);
	LOADL_WORD(pbBuf+2, u2Sc1);
	do {
		u2KnownSc = (u16)dwCfaAviTwoCC((u16)ppbASufx[i][0], (u16)ppbASufx[i][1]);
		if (u2Sc1 == u2KnownSc)
			return TRUE;

		#if CONFIG_CFA_DIVX_DMC_SUPPORT
		/* 00dc -> dc00, 01wb -> wb01, 02sb -> sb02 in DivX 6.0 Media Menu */
		if (u2Sc == u2KnownSc)
			return TRUE; /* Should also return true. */

		#endif

		i++;
	} while (ppbASufx[i][0] != (u8)'\0');

	return FALSE;
}


/*-----------------------------------------------------------------------------
 * Name: CfaAviMatchSpCkId
 *
 * Description:
 *		check if the 4cc in pbBuf is SP type
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: TRUE: match.  FALSE: not match
 *
 *-----------------------------------------------------------------------------*/
#if CONFIG_CFA_AVI_TX_ALL_SP
static bool CfaAviMatchSpCkId(volatile u8 *pbBuf)
{
	u8 ppbSpSufx[][2] = {{'s', 'b'}, {'\0', '\0'} };

	u16 u2KnownSc = 0;
	u16 u2Sc = 0;
	u16 u2Sc1 = 0;
	int i = 0;

	LOADL_WORD(pbBuf, u2Sc);
	LOADL_WORD(pbBuf+2, u2Sc1);
	do {
		u2KnownSc = dwCfaAviTwoCC((u16)ppbSpSufx[i][0], (u16)ppbSpSufx[i][1]);
		if (u2Sc1 == u2KnownSc)
			return TRUE;

		#if CONFIG_CFA_DIVX_DMC_SUPPORT
		/* 00dc -> dc00, 01wb -> wb01, 02sb -> sb02 in DivX 6.0 Media Menu */
		if (u2Sc == u2KnownSc)
			return TRUE; /* Should also return true. */
		#endif

		i++;
	} while (ppbSpSufx[i][0] != (u8)'\0');

	return FALSE;
}
#endif

void GetAviVPictType(const CfaAviInst *prCfaAvi, CFA_VIDEO_INFO_T *PVidInf)
{
	u8 u1Temp1 = 0;
	u8 u1Temp2 = 0;
	u64 u4PictSizeOft = (u64)11;/*skip frm type,codecid,start code,version,temporalreferance*/


	LOAD_BYTE(prCfaAvi->pucHdrBuf + u4PictSizeOft, u1Temp1);
	u1Temp1 &= (u8)0x03;
	u1Temp1 = u1Temp1 << 1;
	LOAD_BYTE(prCfaAvi->pucHdrBuf + u4PictSizeOft + 1, u1Temp2);
	u1Temp2 &= (u8)0x80;
	u1Temp2 = u1Temp2 >> 7;
	u1Temp1 = u1Temp1 + u1Temp2;

	if (u1Temp1 == 0)
		u4PictSizeOft += (u32)3;
	else if (u1Temp1 == (u8)1)
		u4PictSizeOft += (u32)5;
	else
		u4PictSizeOft += (u32)1;

	LOAD_BYTE(prCfaAvi->pucHdrBuf + u4PictSizeOft, u1Temp2);
	u1Temp2 &= (u8)0x60;

	if (u1Temp2 == 0)
		PVidInf->eTxMode = CFA_PTM_H263_SORENSON_I;
	else if (u1Temp2 == (u8)0x20)
		PVidInf->eTxMode = CFA_PTM_H263_SORENSON_P;
	else if (u1Temp2 == (u8)0x40)
		PVidInf->eTxMode = CFA_PTM_H263_SORENSON_P;/*need modify*/
	else
		PVidInf->eTxMode = CFA_PTM_H263_SORENSON_P;

}

/*-----------------------------------------------------------------------------
 * Name: ucCfaAviGetAudStrmID
 *
 * Description:
 *		Get Stream ID
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static u16 CfaAviGetStrmID(u8 *pbBuf)
{
	u16 u2StrmID = 0;

	if ((CfaAviIsAsciiNum(pbBuf[0])) && (CfaAviIsAsciiNum(pbBuf[1])))
		u2StrmID = (u16)((pbBuf[0] - (u8)'0')*((u8)10) + (pbBuf[1] - (u8)'0'));

	if ((CfaAviIsAsciiNum(pbBuf[2])) && (CfaAviIsAsciiNum(pbBuf[3])))
		u2StrmID = (u16)((pbBuf[2] - (u8)'0')*((u8)10) + (pbBuf[3] - (u8)'0'));

	return u2StrmID;
}


/*-----------------------------------------------------------------------------
 * Name: CfaAviMatchChunkId
 *
 * Description:
 *		check if the 4cc in pbBuf matches the given type and index
 *		the original function: fgMatchChunkId
 *
 * Inputs:
 *		[IN] CfaAviPrsBitStrmType
 *		[IN] byte pointer to the 4cc
 *		[IN] stream index
 *
 * Outputs:
 *
 * Returns: TRUE: match.  FALSE: not match
 *
 *-----------------------------------------------------------------------------*/
static bool CfaAviMatchChunkId(u8 bType, volatile u8 *pbBuf, u8 bIdx)
{
	u8 ppbVSufx[][2] = {{'d', 'b'} , {'d', 'c'} , {'\0', '\0'} };
	u8 ppbASufx[][2] = {{'w', 'b'} , {'\0', '\0'} };

	#if CONFIG_CFA_DIVX_DRM_SUPPORT
	u8 ppbDSufx[][2] = {{'d', 'd'} , {'\0', '\0'} };
	#endif

	#if CONFIG_CFA_DIVX_SP_SUPPORT
	u8 ppbSpSufx[][2] = {{'s', 'b'} , {'\0', '\0'} };
	#endif

	u8 (*ppbSufx)[2] = {NULL};
	u32 u4KnownSc = 0;
	u32 u4Sc = 0;
	int i = 0;
	u8 bTmp1 = 0;
	u8 bTmp2 = 0;

	if (bType == CFA_AVI_PRS_BIT_STRM_TYPE_V)
		ppbSufx = ppbVSufx;
	else if (bType == CFA_AVI_PRS_BIT_STRM_TYPE_A)
		ppbSufx = ppbASufx;
	else if (bType == CFA_AVI_PRS_BIT_STRM_TYPE_DRM)
		ppbSufx = ppbDSufx;
	else
		ppbSufx = ppbSpSufx;


	i = 0;
	LOADL_DWRD(pbBuf, u4Sc);

	if (bIdx > (u8)99)
		return FALSE;

	bTmp1 = (bIdx / ((u8)10)) + (u8)'0';
	bTmp2 = (bIdx % ((u8)10)) + (u8)'0';

	do {
		u4KnownSc = dwCfaAviFourCC(bTmp1, bTmp2, ppbSufx[i][0], ppbSufx[i][1]);
		if (u4Sc == u4KnownSc)
			return TRUE;

		#if CONFIG_CFA_DIVX_DMC_SUPPORT
		/* 00dc -> dc00, 01wb -> wb01, 02sb -> sb02 in DivX 6.0 Media Menu */
		u4KnownSc = dwCfaAviFourCC(ppbSufx[i][0], ppbSufx[i][1], bTmp1, bTmp2);
		if (u4Sc == u4KnownSc)
			return TRUE; /* Should also return true. */

		#endif

		i++;
	} while (ppbSufx[i][0] != (u8)'\0');

	return FALSE;
}


/*-----------------------------------------------------------------------------
 * Name: u8CfaAviConvert3Stc
 *
 * Description:
 *		Calculate (u4A * u4B * STC_CKL / u4C)
 *		the original function: dwConvert3Stc
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static u64 CfaAviConvert3Stc(u32 u4A, u32 u4B, u32 u4C)
{
	u64 llA, llB, llC, llS, llR;

	llA = (u64)u4A;
	llB = (u64)u4B;
	llC = (u64)u4C;
	llS = (u64)CFA_STC_CLK;

	if (0 == llC) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: llC is 0 !\r\n"));
		return 0;
	}

	llR = llA * llB * llS / llC;

	return llR;
}


/*-----------------------------------------------------------------------------
 * Name: CfaAviChunk2Pts
 *
 * Description:
 *		calculate the PTS from a given chunk number
 *		the original function: dAviChunk2Pts
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: PTS in CFA_STC_CLK unit
 *
 *-----------------------------------------------------------------------------*/
u64 CfaAviChunk2Pts(const TCfaAviStrmInf *ptStrmInf, u32 u4ChunkNo)
{
	return CfaAviConvert3Stc(u4ChunkNo, ptStrmInf->u4Scale, ptStrmInf->u4Rate);
}


/*-----------------------------------------------------------------------------
 * Name: CfaAviConvert2Stc
 *
 * Description:
 *		Calculate (u4A * STC_CKL / u4B) without overflow
 *		the original function: dwConvert2Stc
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
u64 CfaAviConvert2Stc(u32 u4A, u32 u4B)
{
	u64 llA, llB, llS, llR;

	llA = (u64)u4A;
	llB = (u64)u4B;
	llS = (u64)CFA_STC_CLK;

	if (0 == llB) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: llB is 0 !\r\n"));
		return 0;
	}

	llR = llA * llS / llB;
	return llR;
}


/*-----------------------------------------------------------------------------
 * Name: CfaAviPts
 *
 * Description:
 *		generate the PTS for the chunk currently being transferred
 *		the original function: dwAviPts
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: PTS value
 *
 *-----------------------------------------------------------------------------*/
static u64 CfaAviPts(const CfaAviInst *prCfaAvi, TCfaAviStrmInf *ptStrmInf)
{
	u64 u8Pts = 0;

	switch (ptStrmInf->eStrmType) {
	case CFA_AVI_AST_INTV:
		u8Pts = CfaAviChunk2Pts(ptStrmInf, ptStrmInf->u4TxedChunk);

		#if CONFIG_CFA_AVI_NEW_METHOD_FOR_ABR_AUDIO
		#if CONFIG_CFA_AVI_SUPPORT_MULTI_AUDIO_FOR_ABR
		for (u32 u4Idx = 0; u4Idx < MAX_NS_AVI_AUD; u4Idx++)
			prCfaAvi->fgPrevIsVidChunk[u4Idx] = TRUE;

		
		#else
		prCfaAvi->fgPrevIsVidChunk = TRUE;
		#endif
		#endif
		break;

	case CFA_AVI_AST_VBRA:
		u8Pts = CfaAviChunk2Pts(ptStrmInf, ptStrmInf->u4TxedChunk);
		break;

	case CFA_AVI_AST_CBRA:
		u8Pts = CfaAviConvert2Stc(ptStrmInf->u4TxedByte, ptStrmInf->u4Bps);
		if ((ptStrmInf->u4Bps > 8000000) && (ptStrmInf->eAudCodec == CFA_AUD_DRV_FMT_MP3)) {
			/*add for fix bug 14786 when u4Bps is a erreo data by MTK40495*/
			u8Pts = 0;
		}
		break;

	case CFA_AVI_AST_ABRA: {
		u32 u4SkipByte = prCfaAvi->rCfaRange.rAudioRange[prCfaAvi->ucAudInfoIdx].u4AudSkipByte;
		u64 u4SkipPts = CfaAviConvert2Stc(u4SkipByte, ptStrmInf->u4Bps);

		u8Pts =  prCfaAvi->rCfaRange.u8StartPts - u4SkipPts;
		}
		break;

	default:
		return 0;
	}

	if (u8Pts == 0) {
		/*to ensure av_play (or dsp?) to update variables like _dwSoftStc */
		/*u8Pts = 1; //remove by mtk40144*/
	}

	/* !!! Tmp code, A/V module will take care this. */
	/* we will initial STCA with prCfaAvi->u8PrsPts - PR_AVI_STC_SHIFT in new AV sync */
	/*dPts += PR_AVI_STC_SHIFT; */

	return u8Pts;
}


/*-----------------------------------------------------------------------------
 * Name: CfaAviGetAudInfoIdx
 *
 * Description:
 *		Get the index of audio info by audio stream ID.
 *		For multi-channel.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: the index of audio info
 *
 *-----------------------------------------------------------------------------*/
EXTERN u8 CfaAviGetAudInfoIdx(const CfaAviInst *prCfaAvi, u32 u4StrmID)
{
	u8 uIdx = 0;

	for (uIdx = 0; uIdx < (u8)MAX_NS_AVI_AUD; uIdx++) {
		if (prCfaAvi->rCfaAviAInf[uIdx].tStrmInf.u4StrmIdx == u4StrmID)
			return uIdx;
	}

	return (u8)0xff;  /* error handling only. */
}


/*-----------------------------------------------------------------------------
 * Name: ucCfaAviGetSpInfoIdx
 *
 * Description:
 *		Get the index of SP info by SP stream ID.
 *		For multi-channel.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: the index of SP info
 *
 *-----------------------------------------------------------------------------*/
#if CONFIG_CFA_AVI_TX_ALL_SP
u8 CfaAviGetSpInfoIdx(const CfaAviInst *prCfaAvi, u32 u4StrmID)
{
	u8 uIdx = 0;

	for (uIdx = 0; uIdx < (u8)MAX_NS_AVI_INTERNAL_SP; uIdx++) {
		if (prCfaAvi->rCfaAviSpInf[uIdx].tStrmInf.u4StrmIdx == u4StrmID)
			return uIdx;
	}

	return 0;  /* error handling only. */
}
#endif

#if AVI_SUPPORT_SKIP_DATA_IN_TX_DATA
static bool fgCfaAviSkipInTxData(u64 u8Ca, u64 u8Offset)
{
	if (u8Ca < u8Offset)
		return TRUE;

	return FALSE;
}
#endif

/*-----------------------------------------------------------------------------
 * Name: fgCfaAviPrsSkiped
 *
 * Description:
 *		Check if this chunk should be skipped
 *		the original function: fgAviPrsSkip
 *
 * Inputs:
 *		[IN] stream type
 *		[IN] pointer to the number of chunk or byte to be skipped
 *		[IN] size of the to-be-transferred chunk
 *		[IN] size of a single sample for CFA_AVI_AST_CBRA
 *
 * Outputs:
 *
 * Returns:TRUE: skip.	FALSE: don't skip
 *
 *-----------------------------------------------------------------------------*/
static bool CfaAviPrsSkiped(ECfaAviStrmType eStrmType, u32 *pdSkipPr,
						  u32 dNextSz, u16 wSampleSz)
{
	if (*pdSkipPr == 0)
		return FALSE;  /* don't skip */


	if (0 == wSampleSz) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: sample size is zero !\r\n"));
		return FALSE;
	}

	switch (eStrmType) {
	case CFA_AVI_AST_VBRA:
	case CFA_AVI_AST_INTV:
		(*pdSkipPr)--;
		return TRUE;
	case CFA_AVI_AST_CBRA:
		if ((*pdSkipPr) < dNextSz) {
			/* -----------------------------------------------------------------------------
			This chunk has some bytes that shouldn't be skipped,
			Align the start offset to wSampleSz (nBlockAlign),
			and then set the value to *pdSkipPr.
			-----------------------------------------------------------------------------*/
			*pdSkipPr = (*pdSkipPr / wSampleSz) * wSampleSz;
			if (*pdSkipPr % 2)	/*Wanpeng Yu For bug fix. Special reason. */
				(*pdSkipPr)--;

			return FALSE;
		}
		/* The whole chunk should be skipped */
		*pdSkipPr -= dNextSz;
		return TRUE;
	case CFA_AVI_AST_ABRA:
		if ((*pdSkipPr) < dNextSz)
			return FALSE;

		*pdSkipPr -= dNextSz;
		return TRUE;

	default:
		return FALSE;
	}
}


/*-----------------------------------------------------------------------------
 * Name: fgCfaAviPrsVid
 *
 * Description:
 *			 check if the video stream is in video range
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: TRUE - parse, FALSE - not parse
 *
 *-----------------------------------------------------------------------------*/
static bool CfaAviPrsVid(const CfaAviInst *prCfaAvi, u64 u8ChunkOfst)
{
	if ((u8ChunkOfst >= prCfaAvi->rCfaRange.u8VidStartOfst) &&
		(prCfaAvi->rCfaAviPrsStrmInf.u4VidPrsChunk <= prCfaAvi->rCfaRange.u4VidEndChunkNo) &&
		(u8ChunkOfst + (u64)1 < prCfaAvi->u8Endoffst)) {
		return TRUE;
	}

	return FALSE;
}


/*-----------------------------------------------------------------------------
 * Name: fgCfaAviPrsAud
 *
 * Description:
 *			 check if the audio stream is in audio range
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: TRUE - parse, FALSE - not parse
 *
 *-----------------------------------------------------------------------------*/
static bool CfaAviPrsAud(CfaAviInst *prCfaAvi, u64 u8ChunkOfst)
{
	#if CONFIG_CFA_AVI_TX_ALL_AUD
	if (0xff == prCfaAvi->ucAudInfoIdx) {
		prCfaAvi->ucAudInfoIdx = 0;
		return FALSE;
	}
	#endif
	if (prCfaAvi->ucAudInfoIdx >= MAX_NS_AVI_AUD) {
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]line %d invalid audio info index!\r\n"), DMX_LINE_NO);
		return FALSE;
	}

	if (
		#if CONFIG_CFA_AVI_TX_ALL_AUD
		(u8ChunkOfst >= prCfaAvi->rCfaRange.rAudioRange[prCfaAvi->ucAudInfoIdx].u8AudStartOfst)
		#else
		(u8ChunkOfst >= prCfaAvi->rCfaRange.u8AudStartOfst)
		#endif
		&& (u8ChunkOfst + (u64)1 < prCfaAvi->u8Endoffst)
	   )
		return TRUE;

	return FALSE;
}


/*-----------------------------------------------------------------------------
 * Name: fgCfaAviPrsSub
 *
 * Description:
 *			 check if the subpicture stream is in subpicture range
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: TRUE - parse, FALSE - not parse
 *
 *-----------------------------------------------------------------------------*/
static bool CfaAviPrsSub(const CfaAviInst *prCfaAvi, u64 u8ChunkOfst)
{
	if ((prCfaAvi->rCfaRange.u8SubStartOfst < prCfaAvi->rCfaRange.u8SubEndOfst)
		&& (u8ChunkOfst <= prCfaAvi->rCfaRange.u8SubEndOfst)
		&& (u8ChunkOfst >= prCfaAvi->rCfaRange.u8SubStartOfst))
		return TRUE;

	return FALSE;
}


/*-----------------------------------------------------------------------------
 * Name: CfaAviIncPrsPos
 *
 * Description:
 *		AVI CFA increases current parsing position
 *
 * Inputs:
 *		[IN] pointer to CfaAviInst
 *		[IN] file length added to AVI CFA current parsing position
 *
 * Outputs:
 *
 * Returns: TRUE - meet range end, FALSE - not meet range end
 *
 *-----------------------------------------------------------------------------*/
static bool CfaAviIncPrsPos(CfaAviInst *prCfaAvi, u64 u8Len)
{
	/*TCfaAviStrmInf *ptSpStrmInf = NULL;*/
	/*TCfaAviAInf *ptAInf	  = NULL;*/
	TCfaAviVInf *ptVInf = NULL;
	CfaAviRange *prCfaRange = NULL;
	bool fgAdoFinished = FALSE;
	bool fgVdoFinished = FALSE;
	bool fgspdoFinished = FALSE;

	ptVInf	   = &(prCfaAvi->rCfaAviVInf);

	prCfaRange = &(prCfaAvi->rCfaRange);

	if (prCfaAvi->u8Ca + 1 >= prCfaAvi->u8Endoffst)
		return TRUE;

	#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
	if (CFA_RANGE_TYPE_INQUERY_VBR_GARBAGE_INF == prCfaRange->eRangeType)
		return FALSE;

	#endif

	/* Todo: need check if all audio is finished*/
	if ((CfaAviToPlay(prCfaAvi->u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_A)) &&
		(prCfaRange->u8AudStartOfst != DMX_INVALID_UINT64)) {
		/*
		*/
	} else if (
		/*(FALSE == prCfaAvi->fgCurInstIsSecond) &&*/
	   (prCfaAvi->u4PrsFlg == CFA_AVI_PRS_BIT_STRM_TYPE_A) &&
	   (prCfaRange->u8AudStartOfst != DMX_INVALID_UINT64)) {
		/*
		*/
	} else
		fgAdoFinished = TRUE;

	if ((CfaAviToPlay(prCfaAvi->u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_V)) &&
		(prCfaRange->u8VidStartOfst != DMX_INVALID_UINT64)) {
		if (ptVInf->tStrmInf.u4TxedChunk > prCfaRange->u4VidEndChunkNo)
			fgVdoFinished = TRUE;

	} else
		fgVdoFinished = TRUE;

	#if CONFIG_CFA_DIVX_SP_SUPPORT
	if ((CfaAviToPlay(prCfaAvi->u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_SP0))
		&& (prCfaRange->u8SubStartOfst != DMX_INVALID_UINT64)
		&& (prCfaRange->u8SubStartOfst < prCfaRange->u8SubEndOfst)) {
		if (prCfaAvi->u8Ca >= prCfaRange->u8SubEndOfst)
			fgspdoFinished = TRUE;

	} else
		fgspdoFinished = TRUE;

	#else
		fgspdoFinished = TRUE;

	#endif

	/* debug log only: */
	if (fgAdoFinished && fgVdoFinished && fgspdoFinished) {
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:CfaAviIncPrsPos----->u8AudStartByte: 0x%llx\r\n"),
			prCfaRange->u8AudStartByte);
		/*DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			 TEXT("[CFA AVI]:CfaAviIncPrsPos----->u4TxedByte: 0x%llx\r\n"),
				ptAInf->tStrmInf.u4TxedByte);*/
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:CfaAviIncPrsPos----->u8AudEndByte: 0x%llx\r\n"),
			prCfaRange->u8AudEndByte);
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:CfaAviIncPrsPos----->u4VidStartChunkNo: 0x%llx\r\n"),
			prCfaRange->u4VidStartChunkNo);
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:CfaAviIncPrsPos----->u4TxedChunk: 0x%llx\r\n"),
			ptVInf->tStrmInf.u4TxedChunk);
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:CfaAviIncPrsPos----->u4VidEndChunkNo: 0x%llx\r\n"),
			prCfaRange->u4VidEndChunkNo);
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:CfaAviIncPrsPos----->u4SubStartChunkNo: 0x%llx\r\n"),
			prCfaRange->u4SubStartChunkNo);
		/*DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:CfaAviIncPrsPos----->u4TxedChunk: 0x%llx\r\n"),
			ptSpStrmInf->u4TxedChunk);*/
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:CfaAviIncPrsPos----->u4SubEndChunkNo: 0x%llx\r\n"),
			prCfaRange->u4SubEndChunkNo);

	}

	if (fgAdoFinished && fgVdoFinished && fgspdoFinished) {
		if ((TRUE == prCfaAvi->fgCurInstIsSecond) &&
			(0 == prCfaAvi->u4CurPrsFlg) &&
			CfaAviToPlay(prCfaAvi->u4PrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_A))
			return FALSE;
	}

	return (fgAdoFinished && fgVdoFinished && fgspdoFinished);
}



/*-----------------------------------------------------------------------------
 * Name: CfaAviGetAvaTxLen
 *
 * Description:
 *		AVI CFA get available transfer length
 *
 * Inputs:
 *		[IN] pointer to CfaAviInst
 *		[IN] transfer length
 *
 * Outputs:
 *
 * Returns: available transfer length
 *
 *-----------------------------------------------------------------------------*/
static u64 CfaAviGetAvaTxLen(CfaAviInst *prCfaAvi, u64 u8TxLen)
{
	return u8TxLen;
}


/*-----------------------------------------------------------------------------
 * Name: CfaAviGetAvaTxSa
 *
 * Description:
 *		AVI CFA get available transfer start address
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: available transfer start address
 *
 *-----------------------------------------------------------------------------*/
u64 CfaAviGetAvaTxSa(CfaAviInst *prCfaAvi)
{
	u64 u8StartAddr = DMX_INVALID_UINT64;
	u32 u4CurPrsFlg = 0;

	if (prCfaAvi->u8Ca != DMX_INVALID_UINT64)
		return prCfaAvi->u8Ca;

	u4CurPrsFlg = prCfaAvi->u4PrsFlg;

#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
	if (CFA_RANGE_TYPE_INQUERY_VBR_GARBAGE_INF == prCfaAvi->rCfaRange.eRangeType)
		u4CurPrsFlg = prCfaAvi->u4PrsFlg;
#endif

	if ((CfaAviToPlay(u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_V)) &&
		(prCfaAvi->rCfaRange.u8VidStartOfst != DMX_INVALID_UINT64))
		u8StartAddr = prCfaAvi->rCfaRange.u8VidStartOfst;

	#if CONFIG_CFA_AVI_TX_ALL_AUD && 0 /* Todo: need process multiple audio ???*/
	if ((CfaAviToPlay(u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_A)) &&
		(prCfaAvi->rCfaRange.u8AudStartOfst != DMX_INVALID_UINT64))
		u8StartAddr = MIN(u8StartAddr, prCfaAvi->rCfaRange.u8AudStartOfst);

	#else
	if ((CfaAviToPlay(u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_A)) &&
		(prCfaAvi->rCfaRange.u8AudStartOfst != DMX_INVALID_UINT64))
		u8StartAddr = MIN(u8StartAddr, prCfaAvi->rCfaRange.u8AudStartOfst);

	#endif

	#if CONFIG_CFA_DIVX_SP_SUPPORT
	if ((CfaAviToPlay(u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_SP0)) &&
		(prCfaAvi->rCfaRange.u8SubStartOfst != DMX_INVALID_UINT64))
		u8StartAddr = MIN(u8StartAddr, prCfaAvi->rCfaRange.u8SubStartOfst);

	#endif

	/* error handling only. */
	if (u8StartAddr == DMX_INVALID_UINT64)
		u8StartAddr = 0;

	prCfaAvi->u8Ca = u8StartAddr;

	return u8StartAddr;
}


/*-----------------------------------------------------------------------------
 * Name: CfaAviFinishPrs
 *
 * Description:
 *
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
void CfaAviFinishPrs(void *pvSptHdl, CfaAviInst *prCfaAvi, u64 u8EndAddr)
{
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:CfaAviFinishPrs!, pvSptHdl:%p, ca:0x%llx, sz:0x%x\r\n"),
		pvSptHdl, prCfaAvi->u8Ca, prCfaAvi->u4DataSz);

	DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("[CFA AVI]:CfaAviFinishPrs!, PrsFlag:0x%x (V:1 A:2 SP:4)")
		TEXT("u8LastVPts:%ds u8LastAPts:%ds EndPTS:%ds\r\n"),
		prCfaAvi->u4CurPrsFlg,
		(u32)(prCfaAvi->u8LastVPts/90000),
		(u32)(prCfaAvi->u8LastAPts/90000),
		(u32)(prCfaAvi->rCfaRange.u8EndPts/90000));

	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:CfaAviFinishPrs!, pvSptHdl:%p\r\n"), pvSptHdl);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:dwTxedVChunk: %d\r\n"), prCfaAvi->rCfaAviVInf.tStrmInf.u4TxedChunk);

	if (prCfaAvi->ucCurAudInfoIdx >= MAX_NS_AVI_AUD) {
 		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]line %d invalid audio info index!\r\n"), DMX_LINE_NO);
		return;
	}
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:dwTxedAChunk: %d\r\n"),
		prCfaAvi->rCfaAviAInf[prCfaAvi->ucCurAudInfoIdx].tStrmInf.u4TxedChunk);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:dwTxedABytes: %d\r\n"),
		prCfaAvi->rCfaAviAInf[prCfaAvi->ucCurAudInfoIdx].tStrmInf.u4TxedByte);

	#if CONFIG_CFA_AVI_TX_ALL_SP
	if (prCfaAvi->ucCurSpInfoIdx >= MAX_NS_AVI_INTERNAL_SP) {
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]line %d invalid sp info index!\r\n"), DMX_LINE_NO);
		return;
	}
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:dwTxedSpChunk: %d\r\n"),
		prCfaAvi->rCfaAviSpInf[prCfaAvi->ucCurSpInfoIdx].tStrmInf.u4TxedChunk);
	#else
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:dwTxedSpChunk: %d\r\n"), prCfaAvi->rCfaAviSpStrmInf.u4TxedChunk);
	#endif

	prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_ST_IDLE;
	MMATE_CHECK_POINTER(prCfaAvi);
	MMATE_CHECK_STRUCT(prCfaAvi->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaAvi->rCfaAviCurPosiInfo);
	MMATE_CHECK_STRUCT(prCfaAvi->rVbrGarbageInf);
	MMATE_CHECK_STRUCT(prCfaAvi->rFifoInfo);

	if (!(DMX_IS_RW_PLAY(pvSptHdl)))
		Spt4CfaFinishedEx(pvSptHdl, u8EndAddr, TRUE, (u32)GAU_E_EOS);
	else
		Spt4CfaFinishedEx(pvSptHdl, u8EndAddr, FALSE, (u32)GAU_E_EOS);

}


/*-----------------------------------------------------------------------------
 * Name: CfaAviSearchSc
 *
 * Description:
 *		Search prefix location in _pbHdrBuf
 *		the original function: iSearchSc
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: Assume HDRBUF_SZ == 64
 *		0~60: Prefix starts from byte 0-60 of current 64 bytes.
 *		-3~-1: Prefix starts from the last 3 bytes of the previous 64 bytes
 *		CFA_AVI_SC_NOT_FOUND: Prefix doesn't exist in the 1st 61 bytes of current 64 bytes,
 *		 nor does it exist in the last 3 bytes of previous 64 bytes.
 *
 *-----------------------------------------------------------------------------*/

bool fgCfaAviIsMovi(u8 *pbBuf)
{
	if (((u8)'m' == pbBuf[0]) &&
		((u8)'o' == pbBuf[1]) &&
		((u8)'v' == pbBuf[2]) &&
		((u8)'i' == pbBuf[3])) {
		return TRUE;
	}

	return FALSE;
}

static u16 CfaAviSearchSc(void *pvSptHdl, CfaAviInst *prCfaAvi, u32 u4Range)
{
	u16 i = 0;

	if (u4Range < (u32)4)
		return CFA_AVI_SC_NOT_FOUND;

	for (i = 0; i <= (u16)(u4Range - (u32)4); i++) {
		if (CfaAviKnown4cc(prCfaAvi, (u8 *)prCfaAvi->pucHdrBuf + i))
			return i;
		else if (fgCfaAviIsDMCChunkId((u8 *)prCfaAvi->pucHdrBuf + i)) {
			if (CFA_VID_MJPEG == prCfaAvi->eVidType)
				continue;

			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]:fgCfaAviDMC4cc, known, ca:0x%llx, i: 0x%x\r\n"),
				prCfaAvi->u8Ca, i);
			CfaAviNotiDmxError(pvSptHdl, prCfaAvi);
			return CFA_AVI_SC_DMC_FOUND;
		} else if (fgCfaAviIsMovi((u8 *)prCfaAvi->pucHdrBuf + i)) {
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]:fgCfaAviIsMovi, ca:0x%x\r\n"), prCfaAvi->u8Ca);
			return i;
		} else {
			/*do nothing*/
		}
	}

	return CFA_AVI_SC_NOT_FOUND;
}

/*-----------------------------------------------------------------------------
 * Name: CfaAviNextScSearch
 *
 * Description:
 *		AVI CFA search next start code
 *		1. If file offset is over transfer range, ???
 *		AVI CFA state is changed to CFA_AVI_ANA_ST_IDLE and finish the parsing
 *		2. the original function: vMpsPrsNextG
 *
 * Inputs:
 *		[IN] uintptr_t of splitter
 *		[IN] pointer to CfaAviInst
 *		[IN] next analyze state
 *		[IN] advance file length before search next start code
 *		[IN] the data length we want to read.
 *		[IN] header buffer ofst. to put the data from this ofst.
 *
 * Outputs:
 *
 * Returns: None
 *
 *-----------------------------------------------------------------------------*/
void CfaAviNextScSearch(void *pvSptHdl, CfaAviInst *prCfaAvi, CfaAviAnaSt eNxtAnaSt,
					  u64 u8AdvLen, u64 u8ReadLen, u32 u4DestOfst)
{
	MRESULT mrRet = RET_DMX_OK;
	u64	u8Sa = 0;
	u64	u8RangEndOffset = 0;

	u8RangEndOffset = prCfaAvi->u8Endoffst;

#if AVI_DRM_DEBUG_LOG_ON
	DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
	TEXT("[CFA AVI][NEXT SC]eNxtAnaSt:0x%x, u8Ca:0x%llx, u8AdvLen:%lld, u8ReadLen:%lld\r\n"),
		eNxtAnaSt, prCfaAvi->u8Ca, u8AdvLen, u8ReadLen);
#endif

	prCfaAvi->eCurPrsPktType = CFA_AVI_PRS_BIT_STRM_TYPE_HDR;

	/* check if all data are parsed */
	if (TRUE == CfaAviIncPrsPos(prCfaAvi, u8AdvLen)) {
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]:SCode call FinishPrs - 1\r\n"));
		CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - (u64)1);
	} else {
		u8Sa = CfaAviGetAvaTxSa(prCfaAvi);
		if (DMX_INVALID_UINT64 == u8Sa) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI]: invalid start offset!\r\n"));
			return;
		}

		prCfaAvi->eCurCfaAviAnaSt = eNxtAnaSt;

		/*Check if this transfer is within IO read session range */
		if (u8RangEndOffset <= (u8Sa + u8ReadLen + (u64)u4DestOfst)) {
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]:CfaAviNextScSearch, u8RangEndOffset:%x,")
				TEXT("u8Sa:%x, u8ReadLen:%x, u4DestOfst:%x\r\n"),
				u8RangEndOffset, u8Sa, u8ReadLen, u4DestOfst);
			u8ReadLen = u8RangEndOffset - u8Sa - u4DestOfst;
		}

    #if CONFIG_CFA_AVI_NEW_SYNC_PB_BUF_FLOW
		if (prCfaAvi->fgNeedUseNewSyncFlow) {
			u32 u4LastAdvLen = 0;

			if (prCfaAvi->u8Ca > prCfaAvi->u8LastCa)
				u4LastAdvLen = (u32)(prCfaAvi->u8Ca - prCfaAvi->u8LastCa);
			else {
				DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
					TEXT("[CFA AVI]: CfaAviNextScSearch, error! u8Ca:%llx, u8LastCa:%llx\r\n"),
					prCfaAvi->u8Ca, prCfaAvi->u8LastCa);
			}

			prCfaAvi->u8LastCa = prCfaAvi->u8Ca;

			if ((u4LastAdvLen < prCfaAvi->u4RemainDataInPbBuf) &&
				(prCfaAvi->fgCanUseNewSyncFlow == TRUE) &&
				(prCfaAvi->u4TxTimeAfterSyncPB < AVI_MAX_NOT_SYNC_PB_BUF_TIME)) {
				prCfaAvi->u4RemainDataInPbBuf -= u4LastAdvLen;
				prCfaAvi->ptrMemAddress += u4LastAdvLen;

				prCfaAvi->u4TxTimeAfterSyncPB += 1;
				CfaAviTxDoneStCtrl(pvSptHdl, u8ReadLen, prCfaAvi);
			} else {
				DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
					TEXT("[CFA AVI]: CfaAviNextScSearch, ca:0x%llx,")
					TEXT("u4LastAdvLen:0x%x, u4RemainDataInPbBuf:0x%x\r\n"),
					prCfaAvi->u8Ca, u4LastAdvLen, prCfaAvi->u4RemainDataInPbBuf);

				prCfaAvi->fgNeedAdjustAfterTxDone = TRUE;

				prCfaAvi->u4TxTimeAfterSyncPB = 0;

				if ((prCfaAvi->fgDeteGabageData) && (!prCfaAvi->fgDeteGabageDataEnd)) {
					mrRet = Spt4CfaPbb2SyncBufEx(pvSptHdl, prCfaAvi->u8Ca, u8ReadLen + (u64)2,
						(u8 *)&(prCfaAvi->ptrMemAddress),
						&(prCfaAvi->u4RemainDataInPbBuf));
					prCfaAvi->u8Ca = prCfaAvi->u8Ca - 2;
					prCfaAvi->fgVbrGarbageBufSync = TRUE;
				} else {
					mrRet = Spt4CfaPbb2SyncBufEx(pvSptHdl, prCfaAvi->u8Ca, u8ReadLen,
						(u8 *)&(prCfaAvi->ptrMemAddress),
						&(prCfaAvi->u4RemainDataInPbBuf));
				}
			}
		} else
    #endif
		{
			if ((prCfaAvi->fgDeteGabageData) && (!prCfaAvi->fgDeteGabageDataEnd)) {
				mrRet = Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaAvi->u8Ca, u8ReadLen + (u64)2,
										(u8 *)&(prCfaAvi->ptrMemAddress));
				prCfaAvi->u8Ca = prCfaAvi->u8Ca - 2;
				prCfaAvi->fgVbrGarbageBufSync = TRUE;
			} else {
				mrRet = Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaAvi->u8Ca, u8ReadLen,
										(u8 *)&(prCfaAvi->ptrMemAddress));
			}
			/* For fix CNB00004853, To avoid cpu using too busy*/
			if (prCfaAvi->u8SearchCnt != 0)
				Sleep(1);
		}

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI]: CfaAviNextScSearch fail in Spt4CfaPbb2SyncBuf(),")
				TEXT("ret:%d, u8RangEnd:0x%llx, u8Ca:0x%llx, u8ReadLen:0x%llx,")
				TEXT("u4DestOfst:0x%x, mem:0x%x\r\n"),
				mrRet, u8RangEndOffset, prCfaAvi->u8Ca, u8ReadLen,
				u4DestOfst, prCfaAvi->ptrMemAddress);

			CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - (u64)1);
		}
	}
}



/*-----------------------------------------------------------------------------
 * Name: vCfaAviPIsrState
 *
 * Description:
 *		P_Int service routine for AVI files
 *		the original function: vAviPIsrState
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static void CfaAviPIsrState(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	MRESULT mrRet = RET_DMX_OK;

	if ((prCfaAvi->u4GTxLen % (u32)2)
		&& (prCfaAvi->eCurPrsPktType != CFA_AVI_PRS_BIT_STRM_TYPE_SP0)) {

		if ((CFA_VID_MJPEG == prCfaAvi->eVidType) &&
			(!CfaAviKnown4cc(prCfaAvi, (u8 *)prCfaAvi->pucHdrBuf)))
			prCfaAvi->u8Ca += prCfaAvi->u8Ca&((u8)0x01);
		else if (CFA_VID_MJPEG != prCfaAvi->eVidType)
			prCfaAvi->u8Ca += prCfaAvi->u8Ca&((u8)0x01);
		else {
			/*do nothing*/
		}

		/* MTK40144 Think this should be prCfaAvi->u8Ca -= prCfaAvi->u8Ca&0x01;*/
	}

/*-----------------------------------------------------------------------------
	#if CONFIG_CFA_DIVX_DRM_SUPPORT
	if (fgCfaAviDrmPIsr(pvSptHdl, prCfaAvi)) {
		return;
	}
	#endif


	// DivX special knowledge:
	// If the chunk size is odd, there will be a padding zero,
	// we can skip it to speed up the parsing.
	// Ignore in DivX3, since AVI_DIVX3_CHK_BYTE, we may transfter (Even bytes-AVI_DIVX3_CHK_BYTE) bytes
	// Ignore in DRM
	-----------------------------------------------------------------------------*/


	if (prCfaAvi->eCurPrsPktType == CFA_AVI_PRS_BIT_STRM_TYPE_V) {
		#if CONFIG_CFA_DIVX_DRM_SUPPORT
		if ((prCfaAvi->rCfaAviDRMInf.fgDrmExist) && (prCfaAvi->rDivxDRMInf.fgOn == TRUE)) {
			prCfaAvi->rDivxDRMInf.fgOn = FALSE;
			mrRet = Spt4CfaTurnDivxDRM(pvSptHdl, &(prCfaAvi->rDivxDRMInf));
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI]: invalid ret:%d !\r\n"), mrRet);
				CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - 1);
				return;
			}
			prCfaAvi->u4DrmState = CFA_AVI_DRM_FINISH;
		}
		#endif

		prCfaAvi->rCfaAviVInf.tStrmInf.u4TxedChunk++;
		prCfaAvi->rCfaAviPrsStrmInf.u4VidPrsChunk++;
	} else if (prCfaAvi->eCurPrsPktType == CFA_AVI_PRS_BIT_STRM_TYPE_A) {
		#if CONFIG_CFA_AVI_SUPPORT_AUDIO_DRM
		if ((prCfaAvi->rCfaAviDRMInf.fgDrmExist) && (prCfaAvi->rDivxDRMInf.fgOn == TRUE)) {
			prCfaAvi->rDivxDRMInf.fgOn = FALSE;
			mrRet = Spt4CfaTurnDivxDRM(pvSptHdl, &(prCfaAvi->rDivxDRMInf));
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,TEXT("[CFA AVI]: invalid ret:%d !\r\n"), mrRet);
                		CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - 1);
                		return;
			}
			prCfaAvi->u4DrmState = CFA_AVI_DRM_FINISH;
		}
		#endif

		#if !CONFIG_CFA_AVI_TX_ALL_AUD
		if (prCfaAvi->ucCurAudInfoIdx >= MAX_NS_AVI_AUD) {
			DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI]line %d invalid audio info index!\r\n"), DMX_LINE_NO);
			return;
		}
		prCfaAvi->rCfaAviAInf[prCfaAvi->ucCurAudInfoIdx].tStrmInf.u4TxedChunk++;
		prCfaAvi->rCfaAviAInf[prCfaAvi->ucCurAudInfoIdx].tStrmInf.u4TxedByte += prCfaAvi->u4DataSz;
		#else
		if (prCfaAvi->ucAudInfoIdx >= MAX_NS_AVI_AUD) {
			DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI]line %d invalid audio info index!\r\n"), DMX_LINE_NO);
			return;
		}
		prCfaAvi->rCfaAviAInf[prCfaAvi->ucAudInfoIdx].tStrmInf.u4TxedChunk++;
		prCfaAvi->rCfaAviAInf[prCfaAvi->ucAudInfoIdx].tStrmInf.u4TxedByte += prCfaAvi->u4DataSz;
		#endif

		prCfaAvi->rCfaAviPrsStrmInf.u4AudPrsChunk++;
		prCfaAvi->rCfaAviPrsStrmInf.u8AudPrsByte += prCfaAvi->u4DataSz;
	}
	#if CONFIG_CFA_DIVX_SP_SUPPORT
	else if (prCfaAvi->eCurPrsPktType == CFA_AVI_PRS_BIT_STRM_TYPE_SP0) {
		#if CONFIG_CFA_AVI_TX_ALL_SP
		if (prCfaAvi->ucSpInfoIdx >= MAX_NS_AVI_INTERNAL_SP) {
			DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI]line %d invalid sp info index!\r\n"), DMX_LINE_NO);
			return;
		}
		prCfaAvi->rCfaAviSpInf[prCfaAvi->ucSpInfoIdx].tStrmInf.u4TxedChunk++;
		#else
		prCfaAvi->rCfaAviSpStrmInf.u4TxedChunk++;
		#endif
		prCfaAvi->rCfaAviPrsStrmInf.u4SubPrsChunk++;
	}
	#endif
	else {
		/*do nothing*/
	}
	{
		CfaAviNextScSearch(pvSptHdl,
							prCfaAvi,
							prCfaAvi->eCurCfaAviAnaSt,
							0,
							AVI_GEN_READ_BYTES,
							0);
	}
}

/*Description: Set picture tx mode in terms of video codec type
@Return: cfa picture tx mode. 04/10/2008*/
static CfaApiPicTxMode CfaAviVidSetPicTxMode(const CfaAviInst *prCfaAvi)
{
	u32 u4Offset = (u32)8;

	switch (prCfaAvi->eVidType) {
	case CFA_VID_WMV7:
		if (WMV_PVOP == (u8)((*(prCfaAvi->pucHdrBuf)) >> 6))
			return CFA_PTM_WMV_P;

		return CFA_PTM_WMV_I;

	case CFA_VID_WMV8:
		if (WMV_PVOP == (u8)((*(prCfaAvi->pucHdrBuf))  >> 7))
			return CFA_PTM_WMV_P;

		return CFA_PTM_WMV_I;

	case CFA_VID_WMV9:
		if (prCfaAvi->fgPrsSeqFrameInterpolation)
			u4Offset--;

		u4Offset -= (u32)2;
		if (prCfaAvi->fgPrsPreProcRange)
			u4Offset--;

		--u4Offset;
		if ((*(prCfaAvi->pucHdrBuf + 8)) & ((u8)1 << ((u8)(u4Offset))))
			return CFA_PTM_WMV_P;

		if (prCfaAvi->u4PrsNumBFrames == 0)
			return CFA_PTM_WMV_I;

		--u4Offset;
		if ((*(prCfaAvi->pucHdrBuf + 8)) & ((u8)1 << ((u8)(u4Offset))))
			return CFA_PTM_WMV_I;

		return CFA_PTM_WMV_B;

	case CFA_VID_VC1:
		/*do something here later !!*/
		break;
	default:
		break;
	}

	return CFA_PTM_EXACT_POS;
}

/*-----------------------------------------------------------------------------
 * Name: CfaAviTxNextStrmDataToFifo
 *
 * Description:
 *		CFA Avi transfer next stream data to FIFO
 *		1. If file offset is over transfer range, AVI CFA state is changed to
		CFA_AVI_ANA_ST_IDLE and finish the parsing
 *		2. If transfer length make file offset over transfer range, AVI CFA state is changed to
		CFA_AVI_ANA_ST_IDLE and finish the parsing
 *		3. the original function: vMpsPrsNextP()
 *
 * Inputs:
 *		[IN] uintptr_t of splitter
 *		[IN] pointer to CfaAviInst
 *		[IN] advance file length before transferring next bitstrema data
 *		[IN] transfer length
 *
 * Outputs:
 *
 * Returns: None
 *
 *-----------------------------------------------------------------------------*/
static void CfaAviTxNextStrmDataToFifo(void *pvSptHdl, CfaAviInst *prCfaAvi,
									u64 u8AdvLen, u32 u4TxLen)
{
	MRESULT mrRet = RET_DMX_OK;
	u64 u8Sa = 0;
	u64 u8TxAvaLen = 0;
	TCfaAviStrmInf *ptAStrmInf = NULL;

	/* check if all data are parsed */

	/*if (!fgMpsPrsOfstAdv(lAdv)) */
	if (CfaAviIncPrsPos(prCfaAvi, u8AdvLen)) {
		/* finish current parsing */
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]CfaAviTxNextStrmDataToFifo Strm call FinishPrs - 1\r\n"));
		CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - (u64)1);
	} else if (prCfaAvi->u8PrsPts >= prCfaAvi->rCfaRange.u8EndPts) {
		/* finish current parsing */
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("[CFA AVI]CfaAviTxNextStrmDataToFifo Strm call FinishPrs - 2\r\n"));
		CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Ca);
	}
	#ifdef MM_SUPPORT_DIVXHT31
	else if ((DMX_IS_FF_PLAY(pvSptHdl)) &&
		(prCfaAvi->u8FFRangeEndPts != INVALID_TIMESTAMP) &&
		(prCfaAvi->u8PrsPts >= prCfaAvi->u8FFRangeEndPts)) {
		prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_ST_IDLE;
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: %s line %d -- u8PrsPts: 0x%llx, u8FFRangeEndPts: 0x%llx\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prCfaAvi->u8PrsPts,
			prCfaAvi->u8FFRangeEndPts);
		Spt4CfaFinishedEx(pvSptHdl, prCfaAvi->u8Ca, TRUE, GAU_E_FFRWABNORMALEND);
	}
	#endif
	else {
		u8TxAvaLen = CfaAviGetAvaTxLen(prCfaAvi, (u64)u4TxLen);
		if ((0 != u8TxAvaLen) && (u8TxAvaLen == u4TxLen)) {
			CFA_AUDIO_INFO_T rAudInf = {0};
			CFA_VIDEO_INFO_T rVidInf = {0};
			CFA_SUBPIC_INFO_T rSpInf = {0};

			switch (prCfaAvi->eCurPrsPktType) {
			case CFA_AVI_PRS_BIT_STRM_TYPE_V:

				u8Sa = prCfaAvi->u8Ca;
				prCfaAvi->u8PrevVidOffset = u8Sa;

				if (!prCfaAvi->fgFristVidBlk) {
					prCfaAvi->fgFristVidBlk = TRUE;
					DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
						TEXT("[CFA AVI]FristVid: 0x%llx\n"), prCfaAvi->u8Ca);
				}

				if (prCfaAvi->rCfaAviVInf.eCodec == AVCODEC_ID_DIVX3) {
					/*DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
					TEXT("[CFA AVI]: StrmV: 0x%llx, Len 0x%llx\r\n"), u8Sa , u8TxAvaLen);*/
					rVidInf.u8FileOfst = u8Sa;
					rVidInf.eTxMode = prCfaAvi->eSaveTxMode;
					rVidInf.eVidType = prCfaAvi->eVidType;
					rVidInf.u8Len = u8TxAvaLen;
					rVidInf.u4PrsStrmId = (u32)(prCfaAvi->rCfaAviVInf.tStrmInf.u4StrmIdx);
					prCfaAvi->u8LastVPts = prCfaAvi->u8PrsPts; /*add for fix bug 14914*/
					mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rVidInf);
				} else if (prCfaAvi->rCfaAviVInf.eCodec == AVCODEC_ID_H264) {
					/*DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
					TEXT("[CFA AVI]: StrmH264: 0x%llx, Len 0x%llx\r\n"), u8Sa , u8TxAvaLen);*/
					rVidInf.u8FileOfst = u8Sa;
					rVidInf.eTxMode = CFA_PTM_EXACT_POS;
					rVidInf.eVidType = prCfaAvi->eVidType;
					rVidInf.u8Len = u8TxAvaLen;
					rVidInf.u4PrsStrmId = (u32)(prCfaAvi->rCfaAviVInf.tStrmInf.u4StrmIdx);
					prCfaAvi->u8LastVPts = prCfaAvi->u8PrsPts;
					mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rVidInf);
				} else if (prCfaAvi->rCfaAviVInf.eCodec == AVCODEC_ID_H265) {
					/*DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
					TEXT("[CFA AVI]: StrmH264: 0x%llx, Len 0x%llx\r\n"), u8Sa , u8TxAvaLen);*/
					rVidInf.u8FileOfst = u8Sa;
					rVidInf.eTxMode = CFA_PTM_EXACT_POS;
					rVidInf.eVidType = prCfaAvi->eVidType;
					rVidInf.u8Len = u8TxAvaLen;
					rVidInf.u4PrsStrmId = (u32)(prCfaAvi->rCfaAviVInf.tStrmInf.u4StrmIdx);
					prCfaAvi->u8LastVPts = prCfaAvi->u8PrsPts;
					mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rVidInf);
				}
				else if ((prCfaAvi->rCfaAviVInf.eCodec == AVCODEC_ID_VC1) &&
						 (prCfaAvi->rCfaRange.eRangeType == CFA_RANGE_TYPE_INQUERY)) {
					/*DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
					TEXT("[CFA AVI]:Get VC1 parsing mode: 0x%llx, Len 0x%llx\r\n"),
					u8Sa , u8TxAvaLen);*/

					prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_TX_PARSING_MODE;

					rVidInf.fgQueryWVC1Mode = TRUE;
					rVidInf.u8FileOfst = u8Sa;
					rVidInf.eTxMode = CFA_PTM_SAME_POS;
					if (u8TxAvaLen <= (u64)1)
						rVidInf.eTxMode = CFA_PTM_WMV_SKIPFRAME;

					rVidInf.eVidType = prCfaAvi->eVidType;
					rVidInf.u8Len = u8TxAvaLen;
					rVidInf.u4PrsStrmId = (u32)(prCfaAvi->rCfaAviVInf.tStrmInf.u4StrmIdx);
					prCfaAvi->u8LastVPts = prCfaAvi->u8PrsPts;
					mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rVidInf);
				}
				else if ((prCfaAvi->rCfaAviVInf.eCodec == AVCODEC_ID_WMV1) ||
					(prCfaAvi->rCfaAviVInf.eCodec == AVCODEC_ID_WMV2) ||
					(prCfaAvi->rCfaAviVInf.eCodec == AVCODEC_ID_WMV3)) {
					/*DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
					TEXT("[CFA AVI]:StrmV: 0x%llx, Len 0x%llx\r\n"), u8Sa , u8TxAvaLen);*/
					rVidInf.u8FileOfst = u8Sa;
					rVidInf.eTxMode = CfaAviVidSetPicTxMode(prCfaAvi);

					rVidInf.fgUnitStart = TRUE;
					if (u8TxAvaLen <= (u64)1)
						rVidInf.eTxMode = CFA_PTM_WMV_SKIPFRAME;

					rVidInf.eVidType = prCfaAvi->eVidType;
					rVidInf.u8Len = u8TxAvaLen;
					rVidInf.u8TotalAULen = u8TxAvaLen;
					rVidInf.u4PrsStrmId = (u32)(prCfaAvi->rCfaAviVInf.tStrmInf.u4StrmIdx);
					prCfaAvi->u8LastVPts = prCfaAvi->u8PrsPts;
					mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rVidInf);
				} else if (prCfaAvi->rCfaAviVInf.eCodec == AVCODEC_ID_SORENSON) {
					GetAviVPictType(prCfaAvi, &rVidInf);
					rVidInf.u8FileOfst = u8Sa;
					rVidInf.eVidType = prCfaAvi->eVidType;
					rVidInf.u8Len = u8TxAvaLen;
					rVidInf.u8TotalAULen = u8TxAvaLen;
					rVidInf.u4PrsStrmId = (u32)(prCfaAvi->rCfaAviVInf.tStrmInf.u4StrmIdx);
					prCfaAvi->u8LastVPts = prCfaAvi->u8PrsPts;
					mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rVidInf);

				} else if (AVCODEC_ID_MJPEG == prCfaAvi->rCfaAviVInf.eCodec) {
					rVidInf.u8FileOfst = u8Sa;
					rVidInf.eVidType = prCfaAvi->eVidType;
					rVidInf.eTxMode = CFA_PTM_MJPEG_I;
					rVidInf.u8Len = u8TxAvaLen;
					rVidInf.u4PrsStrmId = (u32)(prCfaAvi->rCfaAviVInf.tStrmInf.u4StrmIdx);
					prCfaAvi->u8LastVPts = prCfaAvi->u8PrsPts;
					/*DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
					TEXT("[cfa_avi_st_ctrl]: VVVV AVCODEC_ID_MJPEG--Frame Time:%llds")
					TEXT("u8FileOfst:0x%llx \r\n"),
					(u64)((prCfaAvi->u8PrsPts)/((u64)90000)), rVidInf.u8FileOfst);*/
					mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rVidInf);
				} else if ((prCfaAvi->rCfaAviVInf.eCodec == AVCODEC_ID_VP6) ||
						 (prCfaAvi->rCfaAviVInf.eCodec == AVCODEC_ID_VP8)) {
					rVidInf.u8FileOfst = u8Sa;
					rVidInf.eTxMode = prCfaAvi->eSaveTxMode;
					rVidInf.eVidType = prCfaAvi->eVidType;
					rVidInf.u8Len = u8TxAvaLen;
					rVidInf.u4PrsStrmId = (u32)(prCfaAvi->rCfaAviVInf.tStrmInf.u4StrmIdx);
					prCfaAvi->u8LastVPts = prCfaAvi->u8PrsPts;
					/*DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
					TEXT("[cfa_avi_st_ctrl]: VVVV ---- Frame Time:%lldms
					u8FileOfst:0x%llx len:0x%llx\r\n"),
					(u64)((prCfaAvi->u8PrsPts)/((u64)90)), rVidInf.u8FileOfst,
					rVidInf.u8Len);*/
					mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rVidInf);
				} else {
					rVidInf.u8FileOfst = u8Sa;
					rVidInf.eTxMode = CFA_PTM_SAME_POS;
					rVidInf.eVidType = prCfaAvi->eVidType;
					rVidInf.u8Len = u8TxAvaLen;
					rVidInf.u4PrsStrmId = (u32)(prCfaAvi->rCfaAviVInf.tStrmInf.u4StrmIdx);
					prCfaAvi->u8LastVPts = prCfaAvi->u8PrsPts;
					/*DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
					TEXT("[cfa_avi_st_ctrl]: VVV---Frame Time:%llds	u8FileOfst:0x%llx \r\n"),
					(u64)((prCfaAvi->u8PrsPts)/((u64)90000)), rVidInf.u8FileOfst);*/
					mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rVidInf);
				}
				break;

			case CFA_AVI_PRS_BIT_STRM_TYPE_A:
				#if CONFIG_CFA_AVI_MULTIPLE_AUDIO_INFO
				if (prCfaAvi->ucAudInfoIdx >= MAX_NS_AVI_AUD) {
					DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
						TEXT("[CFA AVI] invalid audio info index!\r\n"));
					return;
				}

				if (prCfaAvi->rCfaRange.rAudioRange[prCfaAvi->ucAudInfoIdx].u4AudSkipByte) {
					DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
						TEXT("[CFA AVI]:TxAdoCa: 0x%llx, 0x%x\r\n"),
						prCfaAvi->u8Ca, prCfaAvi->ucAudInfoIdx);
					DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
						TEXT("[CFA AVI]:TxAdoSkip: 0x%lx\r\n"),
						prCfaAvi->rCfaRange.rAudioRange[prCfaAvi->ucAudInfoIdx].u4AudSkipByte);
				}

				/* skip bytes handling */
				ptAStrmInf = &(prCfaAvi->rCfaAviAInf[prCfaAvi->ucAudInfoIdx].tStrmInf);
				prCfaAvi->u8CurChkOfst = prCfaAvi->u8Ca;

				if (ptAStrmInf->eStrmType != CFA_AVI_AST_ABRA) {
					prCfaAvi->u8Ca +=
						prCfaAvi->rCfaRange.rAudioRange[prCfaAvi->ucAudInfoIdx].u4AudSkipByte;
					prCfaAvi->rCfaRange.rAudioRange[prCfaAvi->ucAudInfoIdx].u4AudSkipByte = 0;
				}
				#else
				if (prCfaAvi->rCfaRange.u4AudSkipByte) {
					DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
						TEXT("[CFA AVI]:TxAdoCa: 0x%llx\r\n"), prCfaAvi->u8Ca);
					DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
						TEXT("[CFA AVI]:TxAdoSkip: 0x%lx\r\n"),
						prCfaAvi->rCfaRange.u4AudSkipByte);
				}

				/* skip bytes handling */
				prCfaAvi->u8Ca += prCfaAvi->rCfaRange.u4AudSkipByte;
				prCfaAvi->rCfaRange.u4AudSkipByte = 0;
				#endif

				u8Sa = prCfaAvi->u8Ca;
				/*DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
						TEXT("[CFA AVI]:StrmA: 0x%llx, Len 0x%llx\r\n"), u8Sa , u8TxAvaLen);*/
				rAudInf.u8FileOfst = u8Sa;
				rAudInf.u8Len = u8TxAvaLen;
				rAudInf.fgUnitStart = FALSE;
				prCfaAvi->ucCurAudInfoIdx = CfaAviGetAudInfoIdx(prCfaAvi, prCfaAvi->u4CurAudStrmID);
				if (CFA_AVI_INVALID_STRM_ID == prCfaAvi->ucCurAudInfoIdx) {
					DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
						TEXT("[CFA AVI] CfaAviTxNextStrmDataToFifo: ")
						TEXT("CfaAvi_SetStrmInf CFA_AVI_INVALID_STRM_ID\r\n"));
					prCfaAvi->ucCurAudInfoIdx = 0;
				}

				/*prCfaAvi->ucCurAudInfoIdx =
				CfaAviGetAudInfoIdx(prCfaAvi, prCfaAvi->u4CurParseAudStrmID);
				//add by zhiwei chen for fix bug 14455(delete	top row is right)*/
				#if CONFIG_CFA_AVI_TX_ALL_AUD
				rAudInf.u4PrsStrmId = prCfaAvi->u4CurParseAudStrmID;
				#else
				rAudInf.u4PrsStrmId =
				(u32)(prCfaAvi->rCfaAviAInf[prCfaAvi->ucCurAudInfoIdx].tStrmInf.u4StrmIdx);
				#endif

				rAudInf.u8Pts = prCfaAvi->u8PrsPts;
				rAudInf.eAudType = prCfaAvi->rCfaAviAInf[prCfaAvi->ucAudInfoIdx].tStrmInf.eAudCodec;
				/*DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,*/
				/*TEXT("[cfa_avi_st_ctrl]: rAudInf.u4PrsStrmId:%d\r\n"),  rAudInf.u4PrsStrmId);*/
				#if CONFIG_CFA_AVI_SUPPORT_AUDIO_DRM
				CfaAviDrmProcBeforeTxAudData(pvSptHdl, prCfaAvi);
				#endif

				/* add for fix bug 14914 BY MTK40495*/
				if ((prCfaAvi->u4CurPrsFlg & CFA_AVI_PRS_BIT_STRM_TYPE_V) &&
					(prCfaAvi->u8LastVPts > rAudInf.u8Pts) &&
					((prCfaAvi->u8LastVPts - rAudInf.u8Pts) > 90000*60*5) &&
					(ptAStrmInf->eStrmType != CFA_AVI_AST_ABRA)) {
					/* if rAudInf.u8Pts all is 0,file is abr */
					DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
						TEXT("[CFA AVI]: THIS file is abnormal\r\n"));
					DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
						TEXT("[CFA AVI]: (u8LastVPts - rAudInf.u8Pts) = %d\r\n"),
						(prCfaAvi->u8LastVPts - rAudInf.u8Pts));
					prCfaAvi->u8Ca += u8TxAvaLen;

					if (prCfaAvi->u8Ca + u8TxAvaLen >= prCfaAvi->rCfaRange.u8Endoffst) {
						CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->rCfaRange.u8Endoffst - (u64)1);
						return;
					}
					/*DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
						TEXT("[cfa_avi_st_ctrl]: AAAA----u8TxAvaLen:%d\r\n"), u8TxAvaLen);*/
					CfaAviTxDoneStCtrl(pvSptHdl, u8TxAvaLen, prCfaAvi);
					if ((prCfaAvi->fgDeteGabageData) && (!prCfaAvi->fgDeteGabageDataEnd)) {
						mrRet = Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaAvi->u8Ca,
						AVI_GEN_READ_BYTES + (u64)2, (u8 *)&(prCfaAvi->ptrMemAddress));
						prCfaAvi->fgVbrGarbageBufSync = TRUE;
						prCfaAvi->u8Ca = prCfaAvi->u8Ca - 2;
					} else {
						mrRet = Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaAvi->u8Ca, AVI_GEN_READ_BYTES,
							(u8 *)&(prCfaAvi->ptrMemAddress));
					}

					if (DMX_FAILED(mrRet)) {
						DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
							TEXT("[CFA AVI] CfaAviTxNextStrmDataToFifo fail in")
							TEXT("Spt4CfaPbb2SyncBuf, ret:%d\r\n"), mrRet);
						CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - (u64)1);
					}

					return;
				}

				prCfaAvi->u8LastAPts = rAudInf.u8Pts;

				rAudInf.u8TotalAULen = 0;
				mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rAudInf);
				if (!(prCfaAvi->u4PrsFlg & CFA_AVI_PRS_BIT_STRM_TYPE_V))/* if has video don't sleep*/
					Sleep(1);

				break;

			case CFA_AVI_PRS_BIT_STRM_TYPE_SP0:

				u8Sa = prCfaAvi->u8Ca;
				/*DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
						TEXT("[CFA AVI]:StrmSP0: 0x%llx, Len 0x%llx\r\n"), u8Sa , u8TxAvaLen);*/
				rSpInf.u8FileOfst = u8Sa;
				rSpInf.fgUnitStart = FALSE;
				rSpInf.u8Pts	  = prCfaAvi->u8PrsPts;
				rSpInf.u8EndPts   = prCfaAvi->u8SpEndPts;
				rSpInf.u4SpuPos   = 0;
				rSpInf.u8Len	  = u8TxAvaLen;
				#if CONFIG_CFA_AVI_TX_ALL_SP
				rSpInf.u4PrsStrmId = prCfaAvi->u4CurParseSpStrmID;
				#else
				rSpInf.u4PrsStrmId = prCfaAvi->rCfaAviSpStrmInf.u4StrmIdx;
				#endif
				mrRet = Spt4CfaPbb2SpFifoAUCtrl(pvSptHdl, &rSpInf, 0);
				break;

			default:
				DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
					TEXT("[CFA AVI]: invalid stream type!\r\n"));
				break;
			}

			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
					TEXT("[CFA AVI]: CfaAviTxNextStrmDataToFifo ERROR ret:%d\r\n"),  mrRet);
				CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - (u64)1);
			}

		} else {
			/* finish current parsing */
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]:Strm call FinishPrs: %x\r\n"), 0x03);
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]:Strm type: %x\r\n"), (u32)prCfaAvi->eCurPrsPktType);
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]:PrEnd TxAdoCa: 0x%llx\r\n"), prCfaAvi->u8Ca);
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]:PrEnd TxAdoSkip: 0x%lx --- error \r\n"), prCfaAvi->u8Ca);

			CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - (u64)1);
		}
	}
}


/*-----------------------------------------------------------------------------
 * Name: CfaAviPrsNextP
 *
 * Description:
 *		the original function: vAviPrsNextP
 *
 * Inputs:
 *		[IN] uintptr_t of splitter
 *		[IN] pointer to CfaAviInst
 *		[IN] !!! may be ignored.
 *		[IN] transfer length
 *
 * Outputs:
 *
 * Returns: None
 *
 *-----------------------------------------------------------------------------*/
void CfaAviPrsNextP(void *pvSptHdl, CfaAviInst *prCfaAvi, s32 lAdv, u32 u4Len)
{
	#if CONFIG_CFA_AVI_DIVIDE_LARGE_AUD_CHUNK
	if (prCfaAvi->eCurCfaAviAnaSt != CFA_AVI_ANA_AVIPRS_TX_DIVIDED_AUD)
	#endif
		prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_PTX;

	CfaAviTxNextStrmDataToFifo(pvSptHdl, prCfaAvi, (u64)lAdv, u4Len);
}


/*-----------------------------------------------------------------------------
 * Name: vCfaAviTxDummyVToFifo
 *
 * Description:
 *		transfer dummy video chunk (0-size video chunk) into video buffer
 *
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static void CfaAviTxDummyVToFifo(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	MRESULT mrRet = RET_DMX_OK;
	CFA_VIDEO_INFO_T rVidInf = {0};

	prCfaAvi->eCurPrsPktType = CFA_AVI_PRS_BIT_STRM_TYPE_V;

	#if CONFIG_CFA_VDEC_MPEG4_SUPPORT
	prCfaAvi->fgBGrouped = FALSE;
	prCfaAvi->u1ChunkVopNs = 0;
	prCfaAvi->u1CurBGrpNum = 0;
	prCfaAvi->u8PrsPts = CfaAviPts(prCfaAvi, &(prCfaAvi->rCfaAviVInf.tStrmInf));
	if (prCfaAvi->u8PrsPts >= prCfaAvi->rCfaRange.u8EndPts) {
		/* finish current parsing */
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:Strm call FinishPrs: %x\r\n"), 0x03);
		CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Ca);
		return;
	}
	Spt4CfaPTSNotify(pvSptHdl, (u64)(prCfaAvi->u8PrsPts));
	#endif	/* #if CONFIG_CFA_VDEC_MPEG4_SUPPORT */

	prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_PTX;

	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:StrmV_0: 0x%llx, Len 0x%lx\r\n"),
		prCfaAvi->u8Ca , prCfaAvi->u4DataSz);
	rVidInf.u8FileOfst = prCfaAvi->u8Ca;
	rVidInf.eTxMode = CFA_PTM_DUMMY;
	rVidInf.eVidType = prCfaAvi->eVidType;
	rVidInf.u8Len = prCfaAvi->u4DataSz;
	rVidInf.u4PrsStrmId = (u32)(prCfaAvi->rCfaAviVInf.tStrmInf.u4StrmIdx);
	rVidInf.fgDummyAU = TRUE;
	rVidInf.fgDummyAUEnd = TRUE;
	rVidInf.fgDummyCmdAU = FALSE;
	mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rVidInf);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: CfaAviTxDummyVToFifo fail in")
			TEXT("Spt4CfaPbb2VFifoAUCtrl, ret:%d\r\n"),  mrRet);
		CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - (u64)1);
	}
}


/*-----------------------------------------------------------------------------
 * Name: vCfaAviPrsTxV
 *
 * Description:
 *		transfer video data into video buffer
 *		the original function: vAviPrsTxV
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static void CfaAviPrsTxV(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	prCfaAvi->eCurPrsPktType = CFA_AVI_PRS_BIT_STRM_TYPE_V;

	prCfaAvi->fgGotValidVidChunk = TRUE;

	#if CONFIG_CFA_VDEC_MPEG4_SUPPORT
	prCfaAvi->fgBGrouped = FALSE;
	prCfaAvi->u1ChunkVopNs = 0;
	prCfaAvi->u1CurBGrpNum = 0;

	prCfaAvi->u8PrsPts = CfaAviPts(prCfaAvi, &(prCfaAvi->rCfaAviVInf.tStrmInf));
	Spt4CfaPTSNotify(pvSptHdl, prCfaAvi->u8PrsPts);

	#endif	/* #if CONFIG_CFA_VDEC_MPEG4_SUPPORT */

	#if CONFIG_CFA_AVI_DIVX3_DRM_NEW_FLOW
	if (prCfaAvi->rCfaAviVInf.eCodec != AVCODEC_ID_DIVX3)
	#endif
		CfaAviDrmProcBeforeTxData(pvSptHdl, prCfaAvi);

	CfaAviPrsNextP(pvSptHdl, prCfaAvi, prCfaAvi->u4GTxLen, prCfaAvi->u4DataSz);
}


static void CfaAviAnaStTxVc1VChunk(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:vCfaAviAnaStTxVc1VChunk, fgTxStartCode: 0x%x\r\n"),
		prCfaAvi->fgNeedTxStartCode);

	if (prCfaAvi->fgNeedTxStartCode) {
		MRESULT mrRet = RET_DMX_OK;

		prCfaAvi->eCurPrsPktType = CFA_AVI_PRS_BIT_STRM_TYPE_V;

		prCfaAvi->u8PrsPts = CfaAviPts(prCfaAvi, &(prCfaAvi->rCfaAviVInf.tStrmInf));
		if (prCfaAvi->u8PrsPts >= prCfaAvi->rCfaRange.u8EndPts) {
			/* finish current parsing */
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]:Strm call FinishPrs: %x\r\n"), 0x03);
			CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Ca);
			return;
		}

		mrRet = Spt4CfaBuf2VFifo(pvSptHdl, (u8 *)prCfaAvi->pucVc1, 0,
								 CFA_PTM_EXACT_POS, prCfaAvi->eVidType, (u64)4);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI]: CfaAviAnaStTxVc1VChunk Spt4CfaBuf2VFifo ret:%d \r\n"),  mrRet);
			CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - (u64)1);

			return;
		}

		prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_TX_VC1_START_CODE;
	} else {
		CfaAviPrsTxV(pvSptHdl, prCfaAvi);
	}
}

static void CfaAviAnaStTxAvcSlice(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:CfaAviAnaStTxAvcSlice, fgTxStartCode: 0x%x\r\n"),
		prCfaAvi->fgNeedTxStartCode);

	switch (prCfaAvi->eCurCfaAviAnaSt) {
	case CFA_AVI_ANA_AVIPRS_TX_AVC_SLICE:
		prCfaAvi->eCurPrsPktType = CFA_AVI_PRS_BIT_STRM_TYPE_V;
		if (0 == prCfaAvi->u4AvcChunkRemSz) {
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]: CfaAviAnaStTxAvcSlice line %d")
				TEXT("-- u4AvcChunkRemSz==0, ANA Next Chunk\r\n"),
				__LINE__, prCfaAvi->u4AvcChunkRemSz);
			prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_MOVI;
			CfaAviPIsrState(pvSptHdl, prCfaAvi);
			break;
		}
		{
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]: CfaAviAnaStTxAvcSlice line %d")
				TEXT("-- u4AvcChunkRemSz: %d, u8Ca: %I64d, ANA Next Slice Len\r\n"),
				__LINE__, prCfaAvi->u8Ca, prCfaAvi->u4AvcChunkRemSz);
			CfaAviNextScSearch(pvSptHdl,
								prCfaAvi,
								CFA_AVI_ANA_AVIPRS_AVC_SLICE_LEN,
								(u64)0,
								(u64)((prCfaAvi->rCfaAviVInf.tStrmInf).u1AvcPayloadLenFieldSz),
								(u32)0);
		}
		break;
	case CFA_AVI_ANA_AVIPRS_TX_AVC_START_CODE:
		prCfaAvi->eCurPrsPktType = CFA_AVI_PRS_BIT_STRM_TYPE_V;

		if (0 < prCfaAvi->u4AvcSliceLen) {
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]: CfaAviAnaStTxAvcSlice line %d ")
				TEXT("-- DMA AVC Slice, u8Ca: %I64d, AvcSliceLen: %d\r\n"),
				__LINE__, prCfaAvi->u8Ca, prCfaAvi->u4AvcSliceLen);
			prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_TX_AVC_SLICE;
			CfaAviTxNextStrmDataToFifo(pvSptHdl, prCfaAvi, (u64)0, prCfaAvi->u4AvcSliceLen);
			if (prCfaAvi->u4AvcSliceLen <= prCfaAvi->u4AvcChunkRemSz)
				prCfaAvi->u4AvcChunkRemSz -= prCfaAvi->u4AvcSliceLen;
			else {
				DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
					TEXT("[CFA AVI]: CfaAviAnaStTxAvcSlice ")
					TEXT("FATAL ERROR prCfaAvi->u4AvcSliceLen < 0\r\n"));

				CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Ca);
			}
		} else {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI]: CfaAviAnaStTxAvcSlice ")
				TEXT("FATAL ERROR prCfaAvi->u4AvcSliceLen < 0\r\n"));

			CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Ca);
		}
		break;
	case CFA_AVI_ANA_AVIPRS_AVC_SLICE_LEN:
		if (4 == prCfaAvi->rCfaAviVInf.tStrmInf.u1AvcPayloadLenFieldSz) {
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI] %s line %d -- AVC Slice Len 0x%02x 0x%02x 0x%02x 0x%02x\r\n"),
				DMX_FUNC_NAME, __LINE__, prCfaAvi->pucHdrBuf[0],
				prCfaAvi->pucHdrBuf[1], prCfaAvi->pucHdrBuf[2], prCfaAvi->pucHdrBuf[3]);
			LOADB_DWRD(prCfaAvi->pucHdrBuf, prCfaAvi->u4AvcSliceLen);
		} else if (3 == prCfaAvi->rCfaAviVInf.tStrmInf.u1AvcPayloadLenFieldSz) {
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI] %s line %d -- AVC Slice Len 0x%02x 0x%02x 0x%02x\r\n"),
				DMX_FUNC_NAME, __LINE__, prCfaAvi->pucHdrBuf[0],
				prCfaAvi->pucHdrBuf[1], prCfaAvi->pucHdrBuf[2]);
			LOADB_3BYTES2DWORD(prCfaAvi->pucHdrBuf, prCfaAvi->u4AvcSliceLen);
		} else if (2 == prCfaAvi->rCfaAviVInf.tStrmInf.u1AvcPayloadLenFieldSz) {
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI] %s line %d -- AVC Slice Len 0x%02x 0x%02x\r\n"),
				DMX_FUNC_NAME, __LINE__, prCfaAvi->pucHdrBuf[0],
				prCfaAvi->pucHdrBuf[1]);
			LOADB_WORD(prCfaAvi->pucHdrBuf, prCfaAvi->u4AvcSliceLen);
		} else if (1 == prCfaAvi->rCfaAviVInf.tStrmInf.u1AvcPayloadLenFieldSz) {
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI] %s line %d -- AVC Slice Len 0x%02x\r\n"),
				DMX_FUNC_NAME, __LINE__, prCfaAvi->pucHdrBuf[0]);
			LOAD_BYTE(prCfaAvi->pucHdrBuf, prCfaAvi->u4AvcSliceLen);
		} else {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI]: CfaAviAnaStTxAvcSlice fail for ")
				TEXT("prCfaAvi->rCfaAviVInf.tStrmInf.u1AvcPayloadLenFieldSz(%u) error\r\n"),
				prCfaAvi->rCfaAviVInf.tStrmInf.u1AvcPayloadLenFieldSz);
			CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Ca);
			break;
		}
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]: CfaAviAnaStTxAvcSlice line %d -- u4AvcChunkRemSz: %d,")
			TEXT("PayloadLenFieldSz: %u, AvcSliceLen: %d\r\n"),
			__LINE__, prCfaAvi->u4AvcChunkRemSz,
			prCfaAvi->rCfaAviVInf.tStrmInf.u1AvcPayloadLenFieldSz,
			prCfaAvi->u4AvcSliceLen);
		prCfaAvi->u4AvcChunkRemSz -= prCfaAvi->rCfaAviVInf.tStrmInf.u1AvcPayloadLenFieldSz;
		if ((prCfaAvi->u4AvcSliceLen <= prCfaAvi->u4AvcChunkRemSz) &&
			(0 < prCfaAvi->u4AvcSliceLen)) {
			MRESULT mrRet = RET_DMX_OK;

			prCfaAvi->eCurPrsPktType = CFA_AVI_PRS_BIT_STRM_TYPE_V;
			prCfaAvi->u8PrsPts = CfaAviPts(prCfaAvi, &(prCfaAvi->rCfaAviVInf.tStrmInf));
			if (prCfaAvi->u8PrsPts >= prCfaAvi->rCfaRange.u8EndPts) {
				/* finish current parsing */
				DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
					TEXT("[CFA AVI]:Strm call FinishPrs: %x\r\n"), 0x03);
				CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Ca);
				return;
			}
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]: CfaAviAnaStTxAvcSlice line %d -- DMA AVC StartCode, u8Ca: %I64d\r\n"),
				__LINE__, prCfaAvi->u8Ca);
			mrRet = Spt4CfaBuf2VFifo(pvSptHdl, (u8 *)prCfaAvi->rCfaAviVInf.tStrmInf.pu1AvcPayloadHdr, (u64)0,
									 CFA_PTM_EXACT_POS, prCfaAvi->eVidType, (u64)3);

			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
					TEXT("[CFA AVI]: CfaAviAnaStTxAvcSlice Spt4CfaBuf2VFifo ret:%d \r\n"),	mrRet);
				CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - (u64)1);
				return;
			}

			prCfaAvi->fgNeedTxStartCode = FALSE;
			prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_TX_AVC_START_CODE;
		} else {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI]: CfaAviAnaStTxAvcSlice FATAL ERROR")
				TEXT("prCfaAvi->u4AvcSliceLen < 0\r\n"));
			CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Ca);
		}
		break;
	default:
		break;
	}
}

/*-----------------------------------------------------------------------------
 * Name: CfaAviPrsTxA
 *
 * Description:
 *		transfer audio data into video buffer
 *		the original function: fgAviPrsTxA
 *
 * Inputs:
 *		[IN] uintptr_t of splitter
 *		[IN] pointer to CfaAviInst
 *		[IN] stream data size
 *
 * Outputs:
 *
 * Returns: TRUE: transfer this chunk;	FALSE: a/v interleaving is too bad, cancel audio playback
 *
 *-----------------------------------------------------------------------------*/
static bool CfaAviPrsTxA(void *pvSptHdl, CfaAviInst *prCfaAvi, u32 u4DataSz)
{
	TCfaAviStrmInf *ptAStrmInf = NULL;
	u32 u4SkipPr = 0;
	u32 u4RealTxLen = 0;

	#if CONFIG_CFA_AVI_TX_ALL_AUD
	if (prCfaAvi->ucAudInfoIdx >= MAX_NS_AVI_AUD) {
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]%s,invalid audio info index!\r\n"), DMX_FUNC_NAME);
		return FALSE;
	}
	ptAStrmInf = &(prCfaAvi->rCfaAviAInf[prCfaAvi->ucAudInfoIdx].tStrmInf);
	#else
	if (prCfaAvi->ucCurAudInfoIdx >= MAX_NS_AVI_AUD) {
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]%s,invalid audio info index!\r\n"),DMX_FUNC_NAME);
		return FALSE;
	}
	ptAStrmInf = &(prCfaAvi->rCfaAviAInf[prCfaAvi->ucCurAudInfoIdx].tStrmInf);
	#endif

	if (0 == u4DataSz)
		return FALSE;

	prCfaAvi->eCurPrsPktType = CFA_AVI_PRS_BIT_STRM_TYPE_A;

	#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
	if (prCfaAvi->fgDeteGabageData) {
		if (!prCfaAvi->fgDeteGabageDataEnd) {
			CfaAviGetVbrGarbageAudioPts(pvSptHdl, prCfaAvi, ptAStrmInf);
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]CfaAviPrsTxA, prCfaAvi->u8PrsPts:%llx ")
				TEXT("- fgDeteGabageDataEnd is FALSE\r\n"), prCfaAvi->u8PrsPts);
		} else {
			prCfaAvi->u8PrsPts = prCfaAvi->u8TrueStartPts +
				CfaAviPts(prCfaAvi, ptAStrmInf)-
				CfaAviChunk2Pts(ptAStrmInf, prCfaAvi->u4GabageChunkNum);
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]CfaAviPrsTxA, prCfaAvi->u8PrsPts:%llx ")
				TEXT("- fgDeteGabageDataEnd is TRUE\r\n"), prCfaAvi->u8PrsPts);
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]CfaAviPrsTxA, u8TrueStartPts:%llx u4GabageChunkNum:%d")
				TEXT("ptAStrmInf->u4TxedChunk:%d\r\n"),
				prCfaAvi->u8TrueStartPts, prCfaAvi->u4GabageChunkNum, ptAStrmInf->u4TxedChunk);
		}
	} else
		prCfaAvi->u8PrsPts = CfaAviPts(prCfaAvi, ptAStrmInf);
	#else
	prCfaAvi->u8PrsPts = CfaAviPts(prCfaAvi, ptAStrmInf);
	#endif

	#if CONFIG_CFA_AVI_MULTIPLE_AUDIO_INFO
	if (CfaAviIsCbrAlloc(ptAStrmInf->eStrmType) &&
		(prCfaAvi->rCfaRange.rAudioRange[prCfaAvi->ucAudInfoIdx].u4AudSkipByte != 0)) {
		prCfaAvi->u8PrsPts +=
			CfaAviConvert2Stc(
			prCfaAvi->rCfaRange.rAudioRange[prCfaAvi->ucAudInfoIdx].u4AudSkipByte,
			ptAStrmInf->u4Bps);
		Spt4CfaPTSNotify(pvSptHdl, prCfaAvi->u8PrsPts);

		u4SkipPr = prCfaAvi->rCfaRange.rAudioRange[prCfaAvi->ucAudInfoIdx].u4AudSkipByte;
	} else
	#else
	if (CfaAviIsCbrAlloc(ptAStrmInf->eStrmType) && (prCfaAvi->rCfaRange.u4AudSkipByte != 0)) {
		prCfaAvi->u8PrsPts += CfaAviConvert2Stc(prCfaAvi->rCfaRange.u4AudSkipByte, ptAStrmInf->u4Bps);
		Spt4CfaPTSNotify(pvSptHdl, prCfaAvi->u8PrsPts);


		u4SkipPr = prCfaAvi->rCfaRange.u4AudSkipByte;
	} else
	#endif
		Spt4CfaPTSNotify(pvSptHdl, prCfaAvi->u8PrsPts);

	if (prCfaAvi->u4DataSz < u4SkipPr) {
		prCfaAvi->u8TransferdAudChunks++;

		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:CfaAviPrsTxA, skip, u4DataSz:0x%x, u4SkipPr:0x%x\r\n"),
			prCfaAvi->u4DataSz, u4SkipPr);
		return FALSE;
	}

	u4RealTxLen = prCfaAvi->u4DataSz - u4SkipPr;

	#if CONFIG_CFA_AVI_DIVIDE_LARGE_AUD_CHUNK
	if (CfaAviIsCbrAlloc(ptAStrmInf->eStrmType) &&
		(prCfaAvi->u4CurAudDividingSize[prCfaAvi->ucAudInfoIdx] > 0) &&
		(prCfaAvi->u4DataSz > (prCfaAvi->u4CurAudDividingSize[prCfaAvi->ucAudInfoIdx]))) {
		prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_TX_DIVIDED_AUD;

		prCfaAvi->u4CurAudChunkTxedOfst += u4SkipPr;

		if (prCfaAvi->u4CurAudChunkTxedOfst  >= prCfaAvi->u4DataSz) {
			DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI]: line %d, audio chunk offset is large than data size!\r\n"), DMX_LINE_NO);
			return FALSE;
		}

		if ((prCfaAvi->u4CurAudChunkTxedOfst +
			prCfaAvi->u4CurAudDividingSize[prCfaAvi->ucAudInfoIdx]) < prCfaAvi->u4DataSz) {
			u4RealTxLen = prCfaAvi->u4CurAudDividingSize[prCfaAvi->ucAudInfoIdx];

			prCfaAvi->u4CurAudChunkTxedOfst += prCfaAvi->u4CurAudDividingSize[prCfaAvi->ucAudInfoIdx];
		} else {
			u4RealTxLen = prCfaAvi->u4DataSz - prCfaAvi->u4CurAudChunkTxedOfst;

			prCfaAvi->u4CurAudChunkTxedOfst = prCfaAvi->u4DataSz;
			prCfaAvi->u8TransferdAudChunks++;
		}
	} else {
		prCfaAvi->u8TransferdAudChunks++;

		prCfaAvi->u4CurAudChunkTxedOfst = prCfaAvi->u4DataSz;
	}

	prCfaAvi->u4AudChunkLastTxedByte = u4SkipPr + u4RealTxLen;
	#else
	prCfaAvi->u8TransferdAudChunks++;
	#endif

	CfaAviPrsNextP(pvSptHdl, prCfaAvi, (s32)(prCfaAvi->u4GTxLen + u4SkipPr), u4RealTxLen);

	return TRUE;
}

static void CfaAviDrmProcBeforeTxData(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	MRESULT mrRet = RET_DMX_OK;

	prCfaAvi->u4DrmState = CFA_AVI_DRM_INIT;

	#if AVI_DRM_DEBUG_LOG_ON
	DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("[CFA AVI][CFA DRM]CfaAviDrmProcBeforeTxData Entry!!!\r\n"));
	#endif

	if ((prCfaAvi->rCfaAviDRMInf.fgStrdExist) &&
		(prCfaAvi->rCfaAviDRMInf.fgDrmExist) && (prCfaAvi->rDivxDRMInf.fgOn)) {
		#if AVI_DRM_DEBUG_LOG_ON
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI][CFA DRM]TX Video Drm Frame Info!!!\r\n"));
		#endif

		prCfaAvi->rDivxDRMInf.u2FrameKeyIdx = prCfaAvi->rCfaAviDRMInf.u2KeyIdx;
		prCfaAvi->rDivxDRMInf.u8DecryptStOfst =
			prCfaAvi->u8Ca + (u64)prCfaAvi->rCfaAviDRMInf.u4EncryptOfst;
		prCfaAvi->rDivxDRMInf.u4DecryptLen = prCfaAvi->rCfaAviDRMInf.u4EncryptLen;

		#if AVI_DRM_DEBUG_LOG_ON
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI][CFA DRM]fgOn:%d u2FrameKeyIdx:%d  u4EncryptOfst:0x%x CA:0x%llx Len:0x%x\r\n"),
			prCfaAvi->rDivxDRMInf.fgOn,
			prCfaAvi->rDivxDRMInf.u2FrameKeyIdx,
			prCfaAvi->rCfaAviDRMInf.u4EncryptOfst,
			prCfaAvi->u8Ca,
			prCfaAvi->rDivxDRMInf.u4DecryptLen);
		#endif

		mrRet = Spt4CfaTurnDivxDRM(pvSptHdl, &(prCfaAvi->rDivxDRMInf));

		if (RET_DMX_OK != mrRet) {
			DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI][CFA DRM] TURN ON FAILED!!!\r\n"));
		}
		prCfaAvi->rDivxDRMInf.fgOn = FALSE;
		prCfaAvi->rCfaAviDRMInf.fgDrmExist = FALSE;
	} else {
		#if AVI_DRM_DEBUG_LOG_ON
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI][CFA DRM] No Need To TX Video Drm Frame Info!!!\r\n"));
		#endif

		prCfaAvi->u4DrmState = CFA_AVI_DRM_OK;
	}

}

static u32 MapAacSampleRate(u32 u4SampleFreq)
{
	switch (u4SampleFreq) {
	case (u32)96000:
	  return 0x0;
	case (u32)88200:
	  return (u32)0x1;
	case (u32)64000:
	  return (u32)0x2;
	case (u32)48000:
	  return (u32)0x3;
	case (u32)44100:
	  return (u32)0x4;
	case (u32)32000:
	  return (u32)0x5;
	case (u32)24000:
	  return (u32)0x6;
	case (u32)22050:
	  return (u32)0x7;
	case (u32)16000:
	  return (u32)0x8;
	case (u32)12000:
	  return (u32)0x9;
	case (u32)11025:
	  return (u32)0xa;
	case (u32)8000:
	  return (u32)0xb;
	case (u32)7350:
	  return (u32)0xc;
	default:
	  return (u32)0xff;
	}
}

static bool CfaAviSetAACHdr(CfaAviInst *prCfaAvi, u32 u4SampleSz)
{
	u32 u4AudSamplePerSec = 0;
	u32 u4AudChannels = 0;
	u8 u1AuHeader[7] = {0};

	if (prCfaAvi->ucAudInfoIdx >= MAX_NS_AVI_AUD) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: Audio Index is larger than 10 !\r\n"));
		return FALSE;
	}
	u4AudSamplePerSec =
		MapAacSampleRate(prCfaAvi->rCfaAviAInf[prCfaAvi->ucAudInfoIdx].tStrmInf.u4SampleRate);
	u4AudChannels = prCfaAvi->rCfaAviAInf[prCfaAvi->ucAudInfoIdx].tStrmInf.u2Channels;
	u4SampleSz += (u32)7;

	u1AuHeader[0] = 0xFF;
	u1AuHeader[1] = 0xF9;
	u1AuHeader[2] =
		(u8) (((u32)1 << 6) | ((u4AudSamplePerSec << 2) & (u32)0x3C) | ((u4AudChannels >> 2) & (u32)0x1));
	u1AuHeader[3] = (u8) (((u4AudChannels & (u32)0x3) << 6) | ((u4SampleSz >> 11) & (u32)0x3));
	u1AuHeader[4] = (u8) ((u4SampleSz >> 3) & (u32)0xFF);
	u1AuHeader[5] = (u8) (((u4SampleSz << 5) & (u32)0xE0) | (((u32)0x7FF >> 6) & (u32)0x1F));
	u1AuHeader[6] = (u8)(((u32)0x7FF << 2) & (u32)0xFC);

	dmx_memcpy((void *)(prCfaAvi->rCfaAviAInf[prCfaAvi->ucAudInfoIdx].tStrmInf.rSpecInfo.pu1CodecSpecData),
		(void *)(&u1AuHeader[0]),
		sizeof(u1AuHeader));

	return TRUE;
}


static bool CfaAviTxHeader(void *pvSptHdl, CfaAviInst *prCfaAvi, ECfaAviStrmType eStrmType)
{
	switch (eStrmType) {
	case CFA_AVI_AST_INTV: {
		MRESULT mrRet = RET_DMX_OK;
		TCfaAviVInf *ptVInf = NULL;

		ptVInf = &(prCfaAvi->rCfaAviVInf);
		if ((CFA_VID_H264 == prCfaAvi->eVidType) &&
			(ptVInf->tStrmInf.fgAdvanceAvc)) {

			if ((ptVInf->tStrmInf.fgSetAudHdr) &&
				(ptVInf->tStrmInf.rSpecInfo.pu1CodecSpecData != NULL) &&
				(ptVInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen != 0)) {
				ptVInf->tStrmInf.fgSetAudHdr = FALSE;
				prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_TX_VID_HDR;
				mrRet = Spt4CfaBuf2VFifo(pvSptHdl,
								ptVInf->tStrmInf.rSpecInfo.pu1CodecSpecData, 0,
								CFA_PTM_WMV_SEQHDR, prCfaAvi->eVidType,
								(u64)(ptVInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen));

				if (DMX_FAILED(mrRet)) {
					DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
						TEXT("[CFA AVI]: CfaAviTxHeader fail in ")
						TEXT("Spt4CfaBuf2VFifo, ret:%d \r\n"),  mrRet);
					CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - (u64)1);
				}
				return TRUE;
			}
		}
		if (CFA_VID_VC1 == prCfaAvi->eVidType) {
			TCfaAviVInf *ptVInf = NULL;

			ptVInf = &(prCfaAvi->rCfaAviVInf);
			if ((ptVInf->tStrmInf.fgSetAudHdr) &&
				(ptVInf->tStrmInf.rSpecInfo.pu1CodecSpecData != NULL) &&
				(ptVInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen != 0)) {
				ptVInf->tStrmInf.fgSetAudHdr = FALSE;
				prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_TX_VID_HDR;
				mrRet = Spt4CfaBuf2VFifo(pvSptHdl,
							ptVInf->tStrmInf.rSpecInfo.pu1CodecSpecData, 0,
							CFA_PTM_WMV_SEQHDR, prCfaAvi->eVidType,
							(u64)(ptVInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen));

				if (DMX_FAILED(mrRet)) {
					DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
						TEXT("[CFA AVI]: CfaAviTxHeader fail in ")
						TEXT("Spt4CfaBuf2VFifo, ret:%d \r\n"), mrRet);
					CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - (u64)1);
				}

				return TRUE;
			}
		} else if ((CFA_VID_WMV9 == prCfaAvi->eVidType) ||
			(CFA_VID_WMV8 == prCfaAvi->eVidType) ||
			(CFA_VID_WMV7 == prCfaAvi->eVidType)) {
			CFA_VIDEO_INFO_T rVidInf = {0};
			TCfaAviVInf *ptVInf = NULL;

			ptVInf = &(prCfaAvi->rCfaAviVInf);

			if ((ptVInf->tStrmInf.fgSetAudHdr) &&
				(ptVInf->tStrmInf.rSpecInfo.pu1CodecSpecData != NULL) &&
				(ptVInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen >= 4)) {
				prCfaAvi->fgPrsSeqFrameInterpolation =
					(ptVInf->tStrmInf.rSpecInfo.pu1CodecSpecData[3] & (u32)0x02) >> 1;
				prCfaAvi->fgPrsPreProcRange =
					(ptVInf->tStrmInf.rSpecInfo.pu1CodecSpecData[3] & (u32)0x80) >> 7;
				prCfaAvi->u4PrsNumBFrames =
					(ptVInf->tStrmInf.rSpecInfo.pu1CodecSpecData[3] & (u32)0x70) >> 4;
				ptVInf->tStrmInf.fgSetAudHdr = FALSE;
				rVidInf.eTxMode = CFA_PTM_WMV_SEQHDR;
				rVidInf.fgUnitStart = TRUE;
				rVidInf.u8TotalAULen = ptVInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen;
				rVidInf.eVidType = prCfaAvi->eVidType;
				rVidInf.u8Len = ptVInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen;
				rVidInf.u4PrsStrmId = ptVInf->tStrmInf.u4StrmIdx;
				prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_TX_VID_HDR;

				mrRet = Spt4CfaBuf2VFifoAUCtrl(pvSptHdl,
					ptVInf->tStrmInf.rSpecInfo.pu1CodecSpecData, &rVidInf);

				if (DMX_FAILED(mrRet)) {
					DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
						TEXT("[CFA AVI] CfaAviTxHeader fail in ")
						TEXT("Spt4CfaBuf2VFifoAUCtrl, ret:%d\r\n"), mrRet);
					CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - (u64)1);
				}

				return TRUE;
			}
		} else {
			/*do nothing*/
		}
		}
		break;
	case CFA_AVI_AST_SUB:
		break;
	default:
		{
			u16 u2StrmIdx = 0;
			MRESULT mrRet = RET_DMX_OK;
			TCfaAviAInf *ptAInf = NULL;
#if CONFIG_CFA_AVI_TX_ALL_AUD
			u2StrmIdx = (prCfaAvi->ucAudInfoIdx < MAX_NS_AVI_AUD) ? (prCfaAvi->ucAudInfoIdx):(0);
#else
			u2StrmIdx = prCfaAvi->rCfaAviAInf[prCfaAvi->ucCurAudInfoIdx].tStrmInf.u4StrmIdx;
#endif
			ptAInf = &(prCfaAvi->rCfaAviAInf[u2StrmIdx]);
			if (CFA_AUD_DRV_FMT_VORBIS == ptAInf->tStrmInf.eAudCodec) {
				if ((ptAInf->tStrmInf.fgSetAudHdr) &&
					(ptAInf->tStrmInf.rSpecInfo.pu1CodecSpecData != NULL) &&
					(ptAInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen != 0)) {
					ptAInf->tStrmInf.fgSetAudHdr = FALSE;
					prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_TX_AUD_HDR;
					mrRet = Spt4CfaBuf2AFifo(pvSptHdl, ptAInf->tStrmInf.rSpecInfo.pu1CodecSpecData,
						ptAInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen,
						ptAInf->tStrmInf.u4StrmIdx, ptAInf->tStrmInf.eAudCodec);

					if (DMX_FAILED(mrRet)) {
						DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
							TEXT("[CFA AVI] CfaAviTxHeader fail in")
							TEXT("Spt4CfaBuf2AFifo, ret:%d\r\n"), mrRet);
						CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - (u64)1);
					}

					return TRUE;
				}
		   }
#if CONFIG_CFA_MKV_SUPPORT_AAC
		   else if (CFA_AUD_DRV_FMT_AAC == ptAInf->tStrmInf.eAudCodec) {
				if ((ptAInf->tStrmInf.fgSetAccfileAudHdr) &&
					(ptAInf->tStrmInf.rAccFileSpecInfo.pu1CodecAccFileSpecData != NULL) &&
					(ptAInf->tStrmInf.rAccFileSpecInfo.u4CodecAccFileSpecDataLen != 0)) {
					ptAInf->tStrmInf.fgSetAccfileAudHdr = FALSE;
					prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_TX_AUD_HDR;
					mrRet = Spt4CfaBuf2AFifo(pvSptHdl,
						ptAInf->tStrmInf.rAccFileSpecInfo.pu1CodecAccFileSpecData,
						 ptAInf->tStrmInf.rAccFileSpecInfo.u4CodecAccFileSpecDataLen,
						 ptAInf->tStrmInf.u4StrmIdx, ptAInf->tStrmInf.eAudCodec);

					if (DMX_FAILED(mrRet)) {
						DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
							TEXT("[CFA AVI] CfaAviTxHeader fail in ")
							TEXT("Spt4CfaBuf2AFifo, ret:%d\r\n"), mrRet);
						CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - (u64)1);
					}

					return TRUE;
				}
				if ((ptAInf->tStrmInf.fgSetAudHdr) &&
					(ptAInf->tStrmInf.rSpecInfo.pu1CodecSpecData != NULL) &&
					(ptAInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen != 0) &&
					(prCfaAvi->u4DataSz > 0)) {
					ptAInf->tStrmInf.fgSetAudHdr = FALSE;
					CfaAviSetAACHdr(prCfaAvi, prCfaAvi->u4DataSz);
					prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_TX_AUD_HDR;
					mrRet = Spt4CfaBuf2AFifo(pvSptHdl, ptAInf->tStrmInf.rSpecInfo.pu1CodecSpecData,
						 ptAInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen,
						 ptAInf->tStrmInf.u4StrmIdx, ptAInf->tStrmInf.eAudCodec);

					if (DMX_FAILED(mrRet)) {
						DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
							TEXT("[CFA AVI] CfaAviTxHeader fail in ")
							TEXT("Spt4CfaBuf2AFifo, ret:%d\r\n"), mrRet);
						CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - (u64)1);
					}

					return TRUE;
				}
			} else {
				/*do nothing*/
			}
#endif
		}
		break;
	}

	return FALSE;
}

/*-----------------------------------------------------------------------------
 * Name: vCfaAviPrsDmux
 *
 * Description:
 *		the original function: vAviPrsDmux
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static void CfaAviPrsDmux(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	MRESULT mrRet = RET_DMX_OK;
	u16 u2StrmIdx = 0;
	u32 *pu4Skip = NULL;
	TCfaAviAInf *ptAInf = NULL;
	/*	FdmxInstData *prFdmxInst; */
	u64 u8RangEndOffset = prCfaAvi->u8Endoffst;
	TCfaAviVInf *ptVInf = &(prCfaAvi->rCfaAviVInf);

	/* CfaAviRange *prCfaRange = &(prCfaAvi->rCfaRange); */
	/*u32 dChunkOfst = _dwFileOfst + prCfaAvi->rCfaNonUsed.wGTxLen - 8;	*/

	u64 u8ChunkOfst = prCfaAvi->u8Ca - (u64)8;
	TCfaAviPrsStrmInf *ptPrsInf = &(prCfaAvi->rCfaAviPrsStrmInf);

#if CONFIG_CFA_AVI_TX_ALL_SP
	TCfaAviStrmInf *ptSpStrmInf = NULL;
#else
	TCfaAviStrmInf *ptSpStrmInf = &(prCfaAvi->rCfaAviSpStrmInf);
#endif

	u8 *pucHdrBuf = NULL;

#if CONFIG_CFA_AVI_SUPPORT_CORRECT_CHUNK_ABNORMITY
	if (prCfaAvi->fgNextChunkDetecting) {
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:vCfaAviPrsDmux, detected next chunk, ca:0x%llx, prevCa:0x%llx\r\n"),
			prCfaAvi->u8Ca, prCfaAvi->u8PrevCa);

		prCfaAvi->u4PrevDataSize = (u32)(prCfaAvi->u8Ca - prCfaAvi->u8PrevCa -
									(AVI_4CC_BYTES + AVI_SZ_BYTES));

		prCfaAvi->pucHdrBuf = prCfaAvi->aucPrev4cc;

		pucHdrBuf = prCfaAvi->aucPrev4cc;

		prCfaAvi->u8Ca = prCfaAvi->u8PrevCa;

		prCfaAvi->u4DataSz = prCfaAvi->u4PrevDataSize;

		prCfaAvi->fgNextChunkDetecting = FALSE;
	} else
#endif
	{
		pucHdrBuf = prCfaAvi->pucHdrBuf;

		LOADL_DWRD(pucHdrBuf + AVI_4CC_BYTES, prCfaAvi->u4DataSz);	/* +AVI_4CC_BYTES: eg,'nndb' */
	}

#if CONFIG_CFA_AVI_SUPPORT_FIFO_LIMITATION
	if (CfaAviTooLargeChunkForFifo(pvSptHdl, prCfaAvi, pucHdrBuf)) {
		#if CONFIG_CFA_AVI_SUPPORT_CORRECT_CHUNK_ABNORMITY
			CfaAviProcAbnormalChunk(pvSptHdl, prCfaAvi, pucHdrBuf);
		#else
			CfaAviNextScSearch(pvSptHdl, prCfaAvi, CFA_AVI_ANA_AVIPRS_MOVI_SRCH,
				0, CFA_AVI_HDRBUF_SZ, 0);
		#endif

		return;
	}
#endif
	if (prCfaAvi->u8Ca + prCfaAvi->u4DataSz + 1  > u8RangEndOffset) {
		if ((CfaAviKnown4cc(prCfaAvi, (u8 *)pucHdrBuf)) &&
			 ((prCfaAvi->u8Ca + prCfaAvi->u4DataSz) != u8RangEndOffset)) {
			/* finish current parsing */
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:vCfaAviPrsDmux, FinishPrs, ca:0x%llx, sz:%x, end:0x%llx\r\n"),
			prCfaAvi->u8Ca, prCfaAvi->u4DataSz, u8RangEndOffset);

			if (prCfaAvi->u8Ca + prCfaAvi->u4DataSz + 1  >
				(u8RangEndOffset + ERR_CHUNKRANG_END_OFST_SCALE)) {
				#if CONFIG_CFA_AVI_SUPPORT_CORRECT_CHUNK_ABNORMITY
					CfaAviProcAbnormalChunk(pvSptHdl, prCfaAvi, pucHdrBuf);
				#else
					CfaAviNextScSearch(pvSptHdl, prCfaAvi, CFA_AVI_ANA_AVIPRS_MOVI_SRCH,
						0, CFA_AVI_HDRBUF_SZ, 0);
				#endif
			} else
				CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - (u64)1);

			return;
		} else if (!CfaAviKnown4cc(prCfaAvi, (u8 *)pucHdrBuf)) {
			if (fgCfaAviIsDMCChunkId((u8 *)pucHdrBuf)) {
				DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
					TEXT("[CFA AVI]:fgCfaAviDMC4cc, known, ca:0x%llx\r\n"), prCfaAvi->u8Ca);
				CfaAviNotiDmxError(pvSptHdl, prCfaAvi);
			} else
				CfaAviMoviSrchSc(pvSptHdl, prCfaAvi, prCfaAvi->u4GTxLen);

			return;
		} else {
			/*do nothing*/
		}
		prCfaAvi->u4DataSz = (u32)(u8RangEndOffset - prCfaAvi->u8Ca);
	}

	/* Demux */
	if ((CfaAviToPlay(prCfaAvi->u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_V)) &&
		(CfaAviMatchChunkId(CFA_AVI_PRS_BIT_STRM_TYPE_V, pucHdrBuf,
		(u8)(ptVInf->tStrmInf.u4StrmIdx)))) {
		prCfaAvi->u8SearchCnt = 0;

		#if CONFIG_CFA_AVI_NEW_METHOD_FOR_ERR_JUMP
		prCfaAvi->u8OffsetJumpCnt = 0;
		#endif

		if (CfaAviPrsVid(prCfaAvi, u8ChunkOfst)) {
		#if AVI_SUPPORT_SKIP_DATA_IN_TX_DATA
			if ((!CfaAviPrsSkiped(ptVInf->tStrmInf.eStrmType,
								&(prCfaAvi->rCfaRange.u4VidSkipChunkNs),
								prCfaAvi->u4DataSz,
								(u16)1)) &&
				(!fgCfaAviSkipInTxData(prCfaAvi->u8Ca, prCfaAvi->u8SkipVDataInTxData))) {
		#else
			if (!CfaAviPrsSkiped(ptVInf->tStrmInf.eStrmType,
				&(prCfaAvi->rCfaRange.u4VidSkipChunkNs),
				prCfaAvi->u4DataSz,
				(u16)1)) {
		#endif

				if (CfaAviTxHeader(pvSptHdl, prCfaAvi, ptVInf->tStrmInf.eStrmType))
					return;

				/* current parsed position information for playback callback: */
				prCfaAvi->rCfaAviCurPosiInfo.u8VidCurOfst = u8ChunkOfst;
				prCfaAvi->rCfaAviCurPosiInfo.u4VidCurChunkNo = ptVInf->tStrmInf.u4TxedChunk;
				/* ptVInf->tStrmInf.u4LastSz = prCfaAvi->u4DataSz; */

				/*_rCfaNonUsed.fgVLastOfstOk = TRUE; */
				if (/*(prCfaAvi->fgBGrouped) &&*/
					 (prCfaAvi->u1TotalBGrpNum > 0) &&
					 (prCfaAvi->u4DataSz <= 0xA)) {
					/* If previous chunk is B-grouped, skip this chunk */
					prCfaAvi->u1TotalBGrpNum--;
				}
				/*else if ((prCfaAvi->u4DataSz == 0) && (FALSE == prCfaAvi->fgGotValidVidChunk))*/
				else if (((0 == prCfaAvi->u4DataSz) || (prCfaAvi->u4DataSz <= 8)) &&
					(FALSE == prCfaAvi->fgGotValidVidChunk)) {
					/*skip directly
					//
					*/
				} else if ((prCfaAvi->u4DataSz == 0)) {/* In RMP4, 0-sized chunk should be skipped */
					if (ptVInf->eCodec != AVCODEC_ID_MPEG4) {
						/* add a dummy picture */
						prCfaAvi->u8PrsPts = CfaAviPts(prCfaAvi, &(ptVInf->tStrmInf));
						Spt4CfaPTSNotify(pvSptHdl, prCfaAvi->u8PrsPts);
						/* !!! to do need parser. */
						/*vAddDummyVop(); */
						CfaAviTxDummyVToFifo(pvSptHdl, prCfaAvi);
						return;
					}
				} else {
					if (prCfaAvi->rCfaAviVInf.eCodec == AVCODEC_ID_DIVX3) {
						#if CONFIG_CFA_AVI_DIVX3_DRM_NEW_FLOW
						CfaAviDrmProcBeforeTxData(pvSptHdl, prCfaAvi);
						#endif

						CfaAviNextScSearch(pvSptHdl,
										prCfaAvi,
										CFA_AVI_ANA_AVIPRS_TX_DIVX3_VCHUNK,
										0,
										AVI_DIVX3_CHK_BYTE,
										0);
					} else if ((prCfaAvi->rCfaAviVInf.eCodec == AVCODEC_ID_VP6) ||
							 (prCfaAvi->rCfaAviVInf.eCodec == AVCODEC_ID_VP8)) {
						CfaAviNextScSearch(pvSptHdl,
										prCfaAvi,
										CFA_AVI_ANA_AVIPRS_TX_VP68_VCHUNK,
										0,
										AVI_DIVX3_CHK_BYTE,
										0);
					} else if ((prCfaAvi->rCfaAviVInf.eCodec == AVCODEC_ID_VC1) &&
						(prCfaAvi->fgGetVc1Case == FALSE)) {
						CfaAviNextScSearch(pvSptHdl,
										prCfaAvi,
										CFA_AVI_ANA_AVIPRS_START_CODE,
										0,
										AVI_VC1_CHK_START_CODE_BYTE,
										0);
					} else if (prCfaAvi->rCfaAviVInf.eCodec == AVCODEC_ID_VC1) {
						CfaAviAnaStTxVc1VChunk(pvSptHdl, prCfaAvi);
					} else if ((prCfaAvi->rCfaAviVInf.eCodec == AVCODEC_ID_H264) &&
						(prCfaAvi->rCfaAviVInf.tStrmInf.fgAdvanceAvc)) {
						DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
							TEXT("[LP AVI] %s line %d -- Advanced AVC, ")
							TEXT("ChunkDataSz: %d, Ca: %I64d\r\n"),
							DMX_FUNC_NAME, __LINE__, prCfaAvi->u4DataSz,
							prCfaAvi->u8Ca);
						prCfaAvi->u4AvcChunkRemSz = prCfaAvi->u4DataSz;
						CfaAviNextScSearch(pvSptHdl,
								prCfaAvi,
								CFA_AVI_ANA_AVIPRS_AVC_SLICE_LEN,
								0,
								prCfaAvi->rCfaAviVInf.tStrmInf.u1AvcPayloadLenFieldSz,
								0);
					} else {
						#if CONFIG_CHECK_TX_DATA_TIME
						prCfaAvi->rCfaStattime.fgStart = TRUE;
						prCfaAvi->rCfaStattime.u4StartMs = GetTickCount();
						#endif
						CfaAviPrsTxV(pvSptHdl, prCfaAvi);
					}

					/*_fgAviVTxed = TRUE; //CFA avi, only access by playback. */
					return;
				}
			} else {
				#if CONFIG_CFA_DIVX_DRM_SUPPORT
				if ((prCfaAvi->rCfaAviDRMInf.fgDrmExist) &&
					(prCfaAvi->rDivxDRMInf.fgOn == TRUE)) {

					prCfaAvi->rDivxDRMInf.fgOn = FALSE;
					mrRet = Spt4CfaTurnDivxDRM(pvSptHdl, &(prCfaAvi->rDivxDRMInf));
					if (RET_DMX_OK != mrRet) {
					 	DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,TEXT("[CFA AVI][CFA DRM] TURN ON FAILED!!!\r\n"));
						return;
					}
				}
				#endif
			}

			ptVInf->tStrmInf.u4TxedChunk++;
			ptPrsInf->u4VidPrsChunk++;
		}
	}
	#if CONFIG_CFA_AVI_TX_ALL_AUD
	else if ((!prCfaAvi->fgPlayFrmFlag) && (CfaAviMatchAudCkId(pucHdrBuf)) &&
			(CfaAviToPlay(prCfaAvi->u4PrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_A))) {
	#else
	else if (CfaAviToPlay(prCfaAvi->u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_A)
			&& CfaAviMatchChunkId(CFA_AVI_PRS_BIT_STRM_TYPE_A, pucHdrBuf,
			(u8)(prCfaAvi->rCfaAviAInf[prCfaAvi->ucCurAudInfoIdx].tStrmInf.u4StrmIdx))) {
	#endif

		prCfaAvi->u8SearchCnt = 0;

		#if CONFIG_CFA_AVI_NEW_METHOD_FOR_ERR_JUMP
		prCfaAvi->u8OffsetJumpCnt = 0;
		#endif

		#if CONFIG_CFA_AVI_TX_ALL_AUD
		prCfaAvi->u4CurParseAudStrmID = CfaAviGetStrmID(pucHdrBuf);
		prCfaAvi->ucAudInfoIdx = CfaAviGetAudInfoIdx(prCfaAvi, prCfaAvi->u4CurParseAudStrmID);

		if (prCfaAvi->ucAudInfoIdx != 0xff) {
			u2StrmIdx = prCfaAvi->ucAudInfoIdx;
			#else
			u2StrmIdx = prCfaAvi->rCfaAviAInf[prCfaAvi->ucCurAudInfoIdx].tStrmInf.u4StrmIdx;
			#endif

			if (prCfaAvi->ucAudInfoIdx >= MAX_NS_AVI_AUD) {
				DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
					TEXT("[CFA AVI]: Audio Index is larger than 10 !\r\n"));
				Spt4CfaFinishedEx(pvSptHdl, prCfaAvi->u8Ca, FALSE, GAU_E_EOS);
				return;
			}

			ptAInf = &(prCfaAvi->rCfaAviAInf[prCfaAvi->ucAudInfoIdx]);
		} else
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI] CfaAviPrsDmux: CfaAvi_SetStrmInf CFA_AVI_INVALID_STRM_ID 01\r\n"));


		/* if (u4ChunkOfst >= prCfaAvi->rCfaRange.u8AudStartOfst) */
		if ((CfaAviPrsAud(prCfaAvi, u8ChunkOfst)) && (NULL != ptAInf)) {
			if (CfaAviTxHeader(pvSptHdl, prCfaAvi, ptAInf->tStrmInf.eStrmType))
				return;

			#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
			if (TRUE == CfaAviJudgeVbrGarbageData(pvSptHdl, prCfaAvi))
				return;
			#endif

			if (prCfaAvi->ucAudInfoIdx >= MAX_NS_AVI_AUD) {
				DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
					TEXT("[CFA AVI]: Audio Index is larger than 10 !\r\n"));
				Spt4CfaFinishedEx(pvSptHdl, prCfaAvi->u8Ca, FALSE, GAU_E_EOS);
				return;
			}

			if (CfaAviIsVbrAlloc(ptAInf->tStrmInf.eStrmType))
				pu4Skip = &(prCfaAvi->rCfaRange.rAudioRange[prCfaAvi->ucAudInfoIdx].u4AudSkipChunkNs);
			else
				pu4Skip = &(prCfaAvi->rCfaRange.rAudioRange[prCfaAvi->ucAudInfoIdx].u4AudSkipByte);

			#if AVI_SUPPORT_SKIP_DATA_IN_TX_DATA
			if ((!CfaAviPrsSkiped(ptAInf->tStrmInf.eStrmType,
								pu4Skip,
								prCfaAvi->u4DataSz,
								ptAInf->tSamInf.u2BlockAlign)) &&
				(!fgCfaAviSkipInTxData(prCfaAvi->u8Ca, prCfaAvi->u8SkipADataInTxData))) {
			#else
			if (!CfaAviPrsSkiped(ptAInf->tStrmInf.eStrmType,
								pu4Skip,
								prCfaAvi->u4DataSz,
								ptAInf->tSamInf.u2BlockAlign)) {
			#endif

				/* current parsed position information for playback callback: */
				prCfaAvi->rCfaAviCurPosiInfo.u8AudCurOfst = u8ChunkOfst;
				prCfaAvi->rCfaAviCurPosiInfo.u4AudCurChunkNo = ptAInf->tStrmInf.u4TxedChunk;
				prCfaAvi->rCfaAviCurPosiInfo.u8AudCurByte = ptAInf->tStrmInf.u4TxedByte;

				#if CONFIG_CFA_AVI_DIVIDE_LARGE_AUD_CHUNK
				prCfaAvi->u4CurAudChunkTxedOfst = 0;
				prCfaAvi->u4AudChunkLastTxedByte = 0;
				prCfaAvi->fgStartTxAudChunk = TRUE;
				#endif

				if (CfaAviPrsTxA(pvSptHdl, prCfaAvi, prCfaAvi->u4DataSz)) {
#if CONFIG_CFA_MKV_SUPPORT_AAC
					/*for AAC, we should transfer header for each frame*/
					if ((CFA_AUD_DRV_FMT_AAC == ptAInf->tStrmInf.eAudCodec) &&
						(CFA_AVI_ANA_AVIPRS_TX_DIVIDED_AUD != prCfaAvi->eCurCfaAviAnaSt))
						ptAInf->tStrmInf.fgSetAudHdr = TRUE;
#endif
					return;
				}
			}

			prCfaAvi->rCfaAviAInf[u2StrmIdx].tStrmInf.u4TxedChunk++;
			prCfaAvi->rCfaAviAInf[u2StrmIdx].tStrmInf.u4TxedByte += prCfaAvi->u4DataSz;
			ptPrsInf->u4AudPrsChunk++;
			ptPrsInf->u8AudPrsByte += prCfaAvi->u4DataSz;
		}
	}
#if CONFIG_CFA_DIVX_DRM_SUPPORT
	else if ((CfaAviToPlay(prCfaAvi->u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_V))
			&& (CfaAviMatchChunkId(CFA_AVI_PRS_BIT_STRM_TYPE_DRM, pucHdrBuf,
			(u8)(ptVInf->tStrmInf.u4StrmIdx)))) {
		prCfaAvi->u8SearchCnt = 0;

		#if CONFIG_CFA_AVI_NEW_METHOD_FOR_ERR_JUMP
		prCfaAvi->u8OffsetJumpCnt = 0;
		#endif

		#if AVI_DRM_DEBUG_LOG_ON
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI][CFA DRM]CfaAviPrsDmux, DRM CHK CA:0x%llx\r\n"), prCfaAvi->u8Ca);
		#endif

		#if AVI_SUPPORT_SKIP_DATA_IN_TX_DATA
		if (CfaAviPrsVid(prCfaAvi, u8ChunkOfst) &&
			(!fgCfaAviSkipInTxData(prCfaAvi->u8Ca, prCfaAvi->u8SkipVDataInTxData))) {
		#else
		if (CfaAviPrsVid(prCfaAvi, u8ChunkOfst)) {
		#endif

			CfaAviNextScSearch(pvSptHdl,
							prCfaAvi,
							CFA_AVI_ANA_AVIPRS_DRMINFO,
							prCfaAvi->u4GTxLen,
							DRM_CHUNK_BYTES,
							0);
			return;
		}
		{
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI][CFA DRM][=====]CfaAviPrsDmux, ")
				TEXT("DRM CHK Not In VidRange, ChkOfst:0x%llx\r\n"), u8ChunkOfst);
		}
	}
#endif

	#if CONFIG_CFA_DIVX_SP_SUPPORT
	else if ((CfaAviToPlay(prCfaAvi->u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_SP0)) &&
			 (CfaAviMatchSpCkId(pucHdrBuf))) {
		prCfaAvi->u8SearchCnt = 0;

		#if CONFIG_CFA_AVI_NEW_METHOD_FOR_ERR_JUMP
		prCfaAvi->u8OffsetJumpCnt = 0;
		#endif

		if (CfaAviPrsSub(prCfaAvi, u8ChunkOfst)) {
			#if CONFIG_CFA_AVI_TX_ALL_SP
			prCfaAvi->u4CurParseSpStrmID = CfaAviGetStrmID(pucHdrBuf);
			prCfaAvi->ucSpInfoIdx = CfaAviGetSpInfoIdx(prCfaAvi, prCfaAvi->u4CurParseSpStrmID);

			ptSpStrmInf = &(prCfaAvi->rCfaAviSpInf[prCfaAvi->ucSpInfoIdx].tStrmInf);
			#endif

			#if AVI_SUPPORT_SKIP_DATA_IN_TX_DATA
			if ((!CfaAviPrsSkiped(ptSpStrmInf->eStrmType,
								&(prCfaAvi->rCfaRange.u4SubSkipChunkNs),
								prCfaAvi->u4DataSz,
								1)) &&
				 (!fgCfaAviSkipInTxData(prCfaAvi->u8Ca,
				 prCfaAvi->u8SkipSpDataInTxData))) {
			#else
			if (!CfaAviPrsSkiped(ptSpStrmInf->eStrmType,
								&(prCfaAvi->rCfaRange.u4SubSkipChunkNs),
								prCfaAvi->u4DataSz,
								1)) {
			#endif

				/* current parsed position information for playback callback: */
				prCfaAvi->rCfaAviCurPosiInfo.u8SubCurOfst = u8ChunkOfst;
				prCfaAvi->rCfaAviCurPosiInfo.u4SubCurChunkNo = ptSpStrmInf->u4TxedChunk;
				/*prCfaAvi->u8Ca -= 8;*/

				CfaAviNextScSearch(pvSptHdl,
									prCfaAvi,
									CFA_AVI_ANA_AVIPRS_SUBT_DURATION,
									prCfaAvi->u4GTxLen,
									DIVX_SUBT_DURATION,
									0);/*AVI_GEN_READ_BYTES);*/
				return;
			}

			ptSpStrmInf->u4TxedChunk++;

			ptPrsInf->u4SubPrsChunk++;
		}
	} else {
		/*do nothing*/
	}
	#endif

	/* Skip this chunk */
	/*	prCfaAvi->u4DataSz += (prCfaAvi->u4DataSz & 0x1); // for odd chunk size, 1 byte padding exists */
	if (!CfaAviKnown4cc(prCfaAvi, (u8 *)pucHdrBuf)) {
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]Not Knows F4CC, ChkOfst:0x%llx\r\n"), u8ChunkOfst);
		/* Why do I have to search, instead of just adding one byte for odd-sized chunk? */
		/* It's due to 0:54:30 in the disc "Two Towers". */
		/* There's an odd-sized chunk with no 1-byte padding, followed by */
		/* a chunk with 2-byte padding. */

		/*_dAviMoviSrch++;	// CFA AVI ignore since debug variable only. */

		/*vCfaAviSetScResidue(4, NULL);*/
		/* whatever _rCfaNonUsed.wGTxLen is, set search range as 8 */
		if (fgCfaAviIsDMCChunkId((u8 *)pucHdrBuf)) {
			/*DMX_LogE(TEXT("[CFA AVI] fgCfaAviDMC4cc, known, ca:0x%llx\r\n"), prCfaAvi->u8Ca);*/
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:fgCfaAviDMC4cc, known, ca:0x%llx\r\n"), prCfaAvi->u8Ca);
			CfaAviNotiDmxError(pvSptHdl, prCfaAvi);
		} else
			CfaAviMoviSrchSc(pvSptHdl, prCfaAvi, AVI_GEN_READ_BYTES);
		return;
	}
	prCfaAvi->u8SearchCnt = 0;

#if CONFIG_CFA_AVI_NEW_METHOD_FOR_ERR_JUMP
	prCfaAvi->u8OffsetJumpCnt = 0;
#endif

	#if !CONFIG_CFA_AVI_TX_ALL_AUD
	else if (CfaAviMatchAudCkId(pucHdrBuf)) {
		if (!CfaAviToPlay(prCfaAvi->u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_A)
		   || !CfaAviMatchChunkId(CFA_AVI_PRS_BIT_STRM_TYPE_A, pucHdrBuf,
		   (u8)(prCfaAvi->rCfaAviAInf[prCfaAvi->ucCurAudInfoIdx].tStrmInf.u4StrmIdx))) {
			u16 u2StrmIdx = CfaAviGetAudInfoIdx(prCfaAvi , CfaAviGetStrmID(pucHdrBuf));

			if (CFA_AVI_INVALID_STRM_ID == u2StrmIdx) {
				DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
					TEXT("[CFA AVI] CfaAviPrsDmux: CfaAvi_SetStrmInf ")
					TEXT("CFA_AVI_INVALID_STRM_ID 02\r\n"));
				u2StrmIdx = 0;
			}
			prCfaAvi->rCfaAviAInf[u2StrmIdx].tStrmInf.u4TxedChunk++;
			prCfaAvi->rCfaAviAInf[u2StrmIdx].tStrmInf.u4TxedByte += prCfaAvi->u4DataSz;
			ptPrsInf->u4AudPrsChunk++;
			ptPrsInf->u8AudPrsByte += prCfaAvi->u4DataSz;
		}
	}
	#endif
	{
		prCfaAvi->u8Ca += prCfaAvi->u4DataSz;

		/* DivX special knowledge: */
		/* If the chunk size is odd, there will be a padding zero, */
		/* we can skip it to speed up the parsing. */
		/* Ignore in DivX3, since AVI_DIVX3_CHK_BYTE, we may transfter (Even bytes-AVI_DIVX3_CHK_BYTE) bytes */
		/* Ignore in DRM */
		/* Ignore in SP*/
		if ((prCfaAvi->u4DataSz%2) &&
			(prCfaAvi->eCurPrsPktType != CFA_AVI_PRS_BIT_STRM_TYPE_SP0)) {
			/*MTK40144 think it should be prCfaAvi->u8Ca--;*/
			prCfaAvi->u8Ca += prCfaAvi->u8Ca&((u32)0x01);
		}

		CfaAviNextScSearch(pvSptHdl,
							prCfaAvi,
							prCfaAvi->eCurCfaAviAnaSt, /*means: no change */
							0, /*_rCfaNonUsed.wGTxLen + prCfaAvi->u4DataSz, */
							AVI_GEN_READ_BYTES,
							0);
	}
}


/*-----------------------------------------------------------------------------
 * Name: CfaAviMovi4ccChk
 *
 * Description:
 *		 the original function: vAviMovi4ccChk
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static void CfaAviMovi4ccChk(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	/*if (fgCfaAviIs4cc(prCfaAvi->pucHdrBuf, 'L', 'I', 'S', 'T'))*/
	if ((prCfaAvi->pucHdrBuf[0] == (u8)'L') &&
		(prCfaAvi->pucHdrBuf[1] == (u8)'I') &&
		(prCfaAvi->pucHdrBuf[2] == (u8)'S') &&
		(prCfaAvi->pucHdrBuf[3] == (u8)'T')) {
		u32 u4listlen = 0;
		LOADL_DWRD(((prCfaAvi->pucHdrBuf) + AVI_4CC_BYTES), u4listlen);
		prCfaAvi->u8CurListLen = (u64)u4listlen;

		/* ??? vMpsPrsNextG(_rCfaNonUsed.wGTxLen, 4, 8);  // read list ID */
		CfaAviNextScSearch(pvSptHdl, prCfaAvi, CFA_AVI_ANA_AVIPRS_MOVI_LIST_ID,
							(u64)0, (u64)AVI_4CC_BYTES,		/* read 4cc after List size. */
							(u32)0);/*AVI_GEN_READ_BYTES);  keep 'LIST' and size info. */
	}
#if CONFIG_SUPPORT_AVIX_FILE
	else if ((prCfaAvi->pucHdrBuf[0] == (u8)'m') &&
		(prCfaAvi->pucHdrBuf[1] == (u8)'o') &&
		(prCfaAvi->pucHdrBuf[2] == (u8)'v') &&
		(prCfaAvi->pucHdrBuf[3] == (u8)'i')) {
		prCfaAvi->u8Ca -= 4;
		CfaAviNextScSearch(pvSptHdl,
						   prCfaAvi,
						   CFA_AVI_ANA_AVIPRS_MOVI,
						   (u64)0,
						   AVI_GEN_READ_BYTES,
						   (u32)0);
	}
#endif
	else {
		/* !!! need to take care the picture index related. */
		/* _u4ChunkOfst = prCfaAvi->u8Ca +	prCfaAvi->u4GTxLen - 8; */

		CfaAviPrsDmux(pvSptHdl, prCfaAvi);
	}
}

/*-----------------------------------------------------------------------------
 * Name: vCfaAviMoviSrchSc
 *
 * Description:
 *		search start code and perform actions based on the results
 *		the original function: vAviMoviSrchSc
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static void CfaAviMoviSrchSc(void *pvSptHdl, CfaAviInst *prCfaAvi, u32 u4Range)
{
	u16 u2ScOfst = 0;
	bool fgFound = FALSE;
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
	TEXT("[CFA AVI]:----- Entry CfaAviMoviSrchSc ----- Offset:0x%llx \r\n"),
	prCfaAvi->u8Ca);

	u2ScOfst = CfaAviSearchSc(pvSptHdl, prCfaAvi, u4Range);
	if (u2ScOfst != CFA_AVI_SC_NOT_FOUND) {
		if (u2ScOfst == CFA_AVI_SC_DMC_FOUND)
			return;

		fgFound = TRUE;
	}

	if (fgFound) {
		prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_MOVI;

		if (u2ScOfst <= (u16)(u4Range - (u32)8)) {
			prCfaAvi->u8Ca -= (u64)(prCfaAvi->u4GTxLen - ((u32)u2ScOfst + (u32)8));
			prCfaAvi->u4GTxLen = (u32)(u2ScOfst + (u16)8);
			prCfaAvi->pucHdrBuf += u2ScOfst;
			prCfaAvi->fgDeteGabageDataEnd = TRUE;

			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]:vCfaAviMoviSrchSc, ca:0x%llx, u4GTxLen:0x%x\r\n"),
				prCfaAvi->u8Ca, prCfaAvi->u4GTxLen);

			CfaAviMovi4ccChk(pvSptHdl, prCfaAvi);
		} else {
			prCfaAvi->u8Ca -= (u64)(prCfaAvi->u4GTxLen - (u32)u2ScOfst);
			prCfaAvi->u4GTxLen = (u32)u2ScOfst;
			prCfaAvi->fgDeteGabageDataEnd = TRUE;

			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]:vCfaAviMoviSrchSc, continue srch, ca:0x%llx, u4GTxLen:0x%x\r\n"),
				prCfaAvi->u8Ca, prCfaAvi->u4GTxLen);

			CfaAviNextScSearch(pvSptHdl, prCfaAvi, prCfaAvi->eCurCfaAviAnaSt,
								(u64)prCfaAvi->u4GTxLen, AVI_GEN_READ_BYTES, 0);
		}

		return;
	}

	prCfaAvi->u8SearchCnt++;

	if (prCfaAvi->u8SearchCnt > CFA_AVI_SRCH_KNOWNCHK_TIMES) {
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]vCfaAviMoviSrchSc, u8SearchCnt > CFA_AVI_SRCH_KNOWNCHK_TIMES\r\n"));

		if (prCfaAvi->u8OffsetJumpCnt >= CFA_AVI_OFFSET_JUMP_TIMES) {
			if ((!(prCfaAvi->fgIsFileHasIndex)) ||
				(((CfaApiVidType)VC_UNKNOW) == prCfaAvi->eVidType)) {
				DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
					TEXT("[CFA AVI]: CfaAviMoviSrchSc -- CfaAviNotiDmxError\r\n"));
				CfaAviNotiDmxError(pvSptHdl, prCfaAvi);
			} else
				CfaAviNotiDmxChunkError(pvSptHdl, prCfaAvi);

			return;
		}

		prCfaAvi->u8SearchCnt = 0;
		prCfaAvi->u8OffsetJumpCnt++;
		prCfaAvi->u8Ca += CFA_AVI_GARBAGE_SKIP_DATA_LEN;

		CfaAviNextScSearch(pvSptHdl, prCfaAvi,
			   CFA_AVI_ANA_AVIPRS_MOVI_SRCH,
			   (u64)prCfaAvi->u4GTxLen, CFA_AVI_HDRBUF_SZ, 0);

		return;
	}

	if (prCfaAvi->u8Ca + 1 >= prCfaAvi->u8Endoffst) {
		CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - (u64)1);

		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]:vCfaAviMoviSrchSc, reach end\r\n"));
		return;
	}

	prCfaAvi->u8Ca -= 3;
	prCfaAvi->u4GTxLen -= 3;

	if ((prCfaAvi->fgPlayFrmFlag) &&
		(prCfaAvi->u8SearchCnt > CFA_AVI_SRCH_KNOWNCHK_TIMES)) {
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]:vCfaAviMoviSrchSc, play one frame, ca:%llx, sz:%x, txLen:%lx\r\n"),
			prCfaAvi->u8Ca, prCfaAvi->u4DataSz, prCfaAvi->u4GTxLen);
		CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - (u64)1);
	} else {
		CfaAviNextScSearch(pvSptHdl, prCfaAvi, CFA_AVI_ANA_AVIPRS_MOVI_SRCH,
							(u64)prCfaAvi->u4GTxLen, CFA_AVI_HDRBUF_SZ, 0);
	}

}


/*-----------------------------------------------------------------------------
 * Name: CfaAviAnaStMovi
 *
 * Description:
 *		uintptr_t the CFA_AVI_ANA_AVIPRS_MOVI state
 *		the original function: AVIPRS_MOVI in vAviGIsr
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static void CfaAviAnaStMovi(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	CfaAviMovi4ccChk(pvSptHdl, prCfaAvi);
}


/*-----------------------------------------------------------------------------
 * Name: CfaAviAnaStMoviListId
 *
 * Description:
 *		uintptr_t the CFA_AVI_ANA_AVIPRS_MOVI_LIST_ID state
 *		the original function: AVIPRS_MOVI_LIST_ID in vAviGIsr
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static void CfaAviAnaStMoviListId(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	u32 u4Sz = 0;

	if (((prCfaAvi->pucHdrBuf[0] == (u8)'r') &&
		(prCfaAvi->pucHdrBuf[1] == (u8)'e') &&
		(prCfaAvi->pucHdrBuf[2] == (u8)'c') &&
		(prCfaAvi->pucHdrBuf[3] == (u8)' ')) ||
		((prCfaAvi->pucHdrBuf[0] == (u8)'m') &&
		(prCfaAvi->pucHdrBuf[1] == (u8)'o') &&
		(prCfaAvi->pucHdrBuf[2] == (u8)'v') &&
		(prCfaAvi->pucHdrBuf[3] == (u8)'i'))) {
		/*	  if (fgCfaAviIs4cc(prCfaAvi->pucHdrBuf, 'r', 'e', 'c', ' ') ||
		//		  fgCfaAviIs4cc(prCfaAvi->pucHdrBuf, 'm', 'o', 'v', 'i'))*/
		CfaAviNextScSearch(pvSptHdl,
						prCfaAvi,
						CFA_AVI_ANA_AVIPRS_MOVI,
						(u64)prCfaAvi->u4GTxLen,
						AVI_GEN_READ_BYTES,
						(u32)0);
	} else {/* unknown LIST inside movi */
		/* skip unknown LIST directly*/
		if ((u64)AVI_4CC_BYTES < prCfaAvi->u8CurListLen) {
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:vCfaAviMoviSrchSc, finish for reach end\r\n"),
			prCfaAvi->u8Ca, prCfaAvi->u8CurListLen);

			if ((prCfaAvi->u8Ca + (prCfaAvi->u8CurListLen - AVI_4CC_BYTES)) < prCfaAvi->u8Endoffst)
				prCfaAvi->u8Ca += (prCfaAvi->u8CurListLen - AVI_4CC_BYTES);

			CfaAviNextScSearch(pvSptHdl,
								prCfaAvi,
								CFA_AVI_ANA_AVIPRS_MOVI,
								(u64)0, /*_rCfaNonUsed.wGTxLen + u4Sz - 4, */
								AVI_GEN_READ_BYTES,
								(u32)0);

			return;
		}

		LOADL_DWRD(prCfaAvi->pucHdrBuf + AVI_4CC_BYTES, u4Sz);
		{

			prCfaAvi->u8Ca += (u4Sz - AVI_4CC_BYTES);
			CfaAviNextScSearch(pvSptHdl,
								prCfaAvi,
								CFA_AVI_ANA_AVIPRS_MOVI,
								(u64)0, /*_rCfaNonUsed.wGTxLen + u4Sz - 4, */
								AVI_GEN_READ_BYTES,
								(u32)0);
		}
	}
}


#if CONFIG_CFA_DIVX_SP_SUPPORT
/*-----------------------------------------------------------------------------
 * Name: Str2Dec
 *
 * Description:
 *		convert ASCII string to decimal number, similar to atoi()
 *
 * Inputs: pbStr: string
 *		   bDigit: number of digit
 *
 * Outputs:
 *
 * Returns: number
 *
 *-----------------------------------------------------------------------------*/
u32 Str2Dec(const u8 *pbStr, u8 bDigit)
{
	u8 i = 0;
	u32 u4Val = 0;

	while (TRUE) {
		u4Val += (u32)(pbStr[i] - 0x30);
		i++;
		if (i >= bDigit)
			break;

		u4Val *= (u32)10;
	}

	return u4Val;
}


/*-----------------------------------------------------------------------------
 * Name: TxtTime2Pts1
 *
 * Description:
 *		convert h:mm:ss:nn text time to PTS
 *
 * Inputs: pbStr: string pointing to hh
 *
 * Outputs:
 *
 * Returns: PTS
 *
 *-----------------------------------------------------------------------------*/
u32 TxtTime2Pts1(const u8 *pbStr, u8 bHDigitNs, u8 bNnDigitNs)
{
	u32 i = 0;
	u32 u4Base = (u32)1;
	u32 u4Hr = Str2Dec(pbStr, bHDigitNs);
	u32 u4Min = Str2Dec(pbStr + (u8)2 + (bHDigitNs - (u8)1), (u8)2);
	u32 u4Sec = Str2Dec(pbStr + (u8)5 + (bHDigitNs - (u8)1), (u8)2);
	u32 u4Ms = Str2Dec(pbStr + (u8)8 + (bHDigitNs - (u8)1), bNnDigitNs);

	for (i = 0; i < (u32)bNnDigitNs; i++)
		u4Base = u4Base * (u32)10;

	u4Ms += ((u4Hr * (u32)3600) + (u4Min * (u32)60) + u4Sec) * u4Base;
	return(u4Ms * ((u32)CFA_STC_CLK / u4Base));
}

/*-----------------------------------------------------------------------------
 * Name: vCfaAviAnaStSubtDuration
 *
 * Description:
 *		uintptr_t the CFA_AVI_ANA_AVIPRS_SUBT_DURATION state
 *		the original function: AVIPRS_SUBT_DURATION in vAviGIsr
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: PTS value
 *
 *-----------------------------------------------------------------------------*/
static void CfaAviAnaStSubtDuration(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	prCfaAvi->u8PrsPts = (u64)TxtTime2Pts1(((u8 *)prCfaAvi->pucHdrBuf + (u8)9 - (u8)8), (u8)2, (u8)3);
	Spt4CfaPTSNotify(pvSptHdl, prCfaAvi->u8PrsPts);
	prCfaAvi->u8SpEndPts = (u64)TxtTime2Pts1(((u8 *)prCfaAvi->pucHdrBuf + (u8)22 - (u8)8), (u8)2, (u8)3);

	/* -----------------------------------------------------------------------------
	// CFA Avi ignore
	_dwPrsAddr = dPRSP0FifoWp();
	_rCfaNonUsed.dChunkSpWp = _dwPrsAddr;

	if ((_dwPrsAddr + DRAMA_NONCACH_BASE_ADDRESS+ prCfaAvi->u4DataSz - 27) > (u32)_pbSp0End)
	{
		// Tell sp_divx.c that fifo wraparound happens.
		_dwPrsAddr |= 0x80000000;
	}
	-----------------------------------------------------------------------------*/
	prCfaAvi->eCurPrsPktType = CFA_AVI_PRS_BIT_STRM_TYPE_SP0;
	prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_MOVI;

	if ((TRUE == prCfaAvi->rCfaRange.fgCompareSubPtsFlag) &&
		 (prCfaAvi->u8PrsPts > prCfaAvi->rCfaRange.u8EndPts)) {
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:vCfaAviAnaStSubtDuration, ignore sp,")
			TEXT("ca:0x%llx, u4DataSz:0x%x, u8PrsPts:0x%llx, u8EndPts:0x%llx\r\n"),
			prCfaAvi->u8Ca, prCfaAvi->u4DataSz, prCfaAvi->u8PrsPts, prCfaAvi->rCfaRange.u8EndPts);

		prCfaAvi->u8Ca += prCfaAvi->u4DataSz - DIVX_SUBT_DURATION;

		CfaAviNextScSearch(pvSptHdl, prCfaAvi, CFA_AVI_ANA_AVIPRS_MOVI,
						   (u64)prCfaAvi->u4DataSz - DIVX_SUBT_DURATION,
						   AVI_GEN_READ_BYTES, 0);
	} else
		CfaAviPrsNextP(pvSptHdl, prCfaAvi, (s32)prCfaAvi->u4GTxLen,
					prCfaAvi->u4DataSz - (u32)DIVX_SUBT_DURATION);

	/*_fgAviSPTxed = TRUE;	//CFA avi, only access by playback. */
}
#endif	/* #ifdef CONFIG_CFA_DIVX_SP_SUPPORT */


#if CONFIG_CFA_DIVX_DRM_SUPPORT
/*-----------------------------------------------------------------------------
 * Name: vCfaAviAnaStDRMInfo
 *
 * Description:
 *		uintptr_t the CFA_AVI_ANA_AVIPRS_DRMINFO state
 *		the original function: AVIPRS_DRMINFO in vAviGIsr
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static void CfaAviAnaStDRMInfo(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	#if AVI_DRM_DEBUG_LOG_ON
	vDumpData(prCfaAvi->pucHdrBuf, DRM_CHUNK_BYTES, prCfaAvi->u8Ca - DRM_CHUNK_BYTES);
	#endif

	LOADL_WORD(prCfaAvi->pucHdrBuf, prCfaAvi->rCfaAviDRMInf.u2KeyIdx);
	LOADL_DWRD(prCfaAvi->pucHdrBuf + 2, prCfaAvi->rCfaAviDRMInf.u4EncryptOfst);
	LOADL_DWRD(prCfaAvi->pucHdrBuf + 6, prCfaAvi->rCfaAviDRMInf.u4EncryptLen);

	#if AVI_DRM_DEBUG_LOG_ON
	DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("[CFA AVI][CFA DRM][DRM DATA] CA:0x%llx u2KeyIdx:%d u4EncryptOfst:0x%x u4EncryptLen:0x%x\r\n"),
		prCfaAvi->u8Ca, prCfaAvi->rCfaAviDRMInf.u2KeyIdx,
		prCfaAvi->rCfaAviDRMInf.u4EncryptOfst, prCfaAvi->rCfaAviDRMInf.u4EncryptLen);
	#endif

	prCfaAvi->rCfaAviDRMInf.fgDrmExist = (prCfaAvi->rCfaAviDRMInf.u4EncryptLen > 0);

	if (prCfaAvi->rCfaAviDRMInf.fgDrmExist)
		prCfaAvi->rDivxDRMInf.fgOn = TRUE;
	else
		prCfaAvi->rDivxDRMInf.fgOn = FALSE;

	prCfaAvi->rDivxDRMInf.u4DecryptLen = prCfaAvi->rCfaAviDRMInf.u4EncryptLen;
	prCfaAvi->rDivxDRMInf.u2FrameKeyIdx = prCfaAvi->rCfaAviDRMInf.u2KeyIdx;

	/* after read xxdd DRM chunk, continue to read the xxdc video chunk */
	CfaAviNextScSearch(pvSptHdl,
						prCfaAvi,
						CFA_AVI_ANA_AVIPRS_MOVI,
						(u64)prCfaAvi->u4GTxLen,
						AVI_GEN_READ_BYTES,
						0);
}

#endif	/* #if CONFIG_CFA_DIVX_DRM_SUPPORT */


/*-----------------------------------------------------------------------------
 * Name: CfaAviAnaStTxDivX3VChunk
 *
 * Description:
 *		determine the divx3 frame type
 *		the original function: vAviDx3PIsr, vDx3AddPic
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static void CfaAviAnaStTxDivX3VChunk(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	/* u64 u8Ofst = prCfaAvi->u8Ca-AVI_DIVX3_CHK_BYTE; */
	prCfaAvi->eSaveTxMode = CFA_PTM_ONE_PIC_DX3_I;
	if (*(prCfaAvi->pucHdrBuf) & DIVX3_P_FRM)
		prCfaAvi->eSaveTxMode = CFA_PTM_ONE_PIC_DX3_P;
	else
		prCfaAvi->eSaveTxMode = CFA_PTM_ONE_PIC_DX3_I;

	/* Generally, the PTS should be set in the vCfaAviPrsTxV(). */
	prCfaAvi->u8PrsPts = CfaAviPts(prCfaAvi, &(prCfaAvi->rCfaAviVInf.tStrmInf));
	Spt4CfaPTSNotify(pvSptHdl, prCfaAvi->u8PrsPts);
	prCfaAvi->u8Ca -= AVI_DIVX3_CHK_BYTE;

#if CONFIG_CHECK_TX_DATA_TIME
	prCfaAvi->rCfaStattime.fgStart = TRUE;
	prCfaAvi->rCfaStattime.u4StartMs = GetTickCount();
#endif
	CfaAviPrsTxV(pvSptHdl, prCfaAvi);
}

static void CfaAviAnaVP68TxModeVChunk(void *pvSptHdl, CfaAviInst *prCfaAviInst)
{
	u8 u1FrmType = (prCfaAviInst->pucHdrBuf[0]);

	switch (prCfaAviInst->eVidType) {
	case CFA_VID_VP6:
		{
			if ((u1FrmType & (u8)0x80) == 0)
				prCfaAviInst->eSaveTxMode = (CFA_PTM_ONE_PIC_VP6_I);
			else
				prCfaAviInst->eSaveTxMode = (CFA_PTM_ONE_PIC_VP6_P);
		}
		break;

	case CFA_VID_VP6A:
		u1FrmType = (prCfaAviInst->pucHdrBuf[3]);
		{
			if ((u1FrmType & (u8)0x80) == 0)
				prCfaAviInst->eSaveTxMode = (CFA_PTM_ONE_PIC_VP6_I);
			else
				prCfaAviInst->eSaveTxMode = (CFA_PTM_ONE_PIC_VP6_P);
		}
		break;
	case CFA_VID_VP8:
		{
			if ((u1FrmType & (u8)0x80) == 0)
				prCfaAviInst->eSaveTxMode = (CFA_PTM_ONE_PIC_VP8_I);
			else
				prCfaAviInst->eSaveTxMode = (CFA_PTM_ONE_PIC_VP8_P);
		}
		break;
	default:
		break;
	}

	prCfaAviInst->u8Ca -= AVI_DIVX3_CHK_BYTE;
	CfaAviPrsTxV(pvSptHdl, prCfaAviInst);
}

/*-----------------------------------------------------------------------------
 * Name: vCfaAviAnaStIdle
 *
 * Description:
 *		AVI CFA processes CFA_AVI_ANA_ST_IDLE
 *
 * Inputs:
 *		[IN] uintptr_t of splitter
 *		[IN] Actual transferred data length.  Normally this value should be equal to the u8TxLen
 *			  in the previous transfer issue, unless file end is hit.
 *		[IN] pointer to CfaAviInst
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static void CfaAviAnaStIdle(void *pvSptHdl, u64 u8TxLen, const CfaAviInst *prCfaAvi)
{
	if (NULL == prCfaAvi) {
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]:prCfaAvi is NULL\r\n"));
		return;
	}
	/* do nothing */
}

static void CfaAviAnaStParsingMode(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	MRESULT mrRet = RET_DMX_OK;
	u8 uParsingMode = 0;

	mrRet = Spt4CfaGetWMVParsingMode(pvSptHdl, &uParsingMode);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: CfaAviAnaStParsingMode fail in Spt4CfaGetWMVParsingMode, ret:%d \r\n"), mrRet);
		return;
	}

	prCfaAvi->rParsingModeInf.uParsingMode = uParsingMode;

	/* Now only CFA_AVI_QUERY_TYPE_PARSING_MODE, so not use prCfaAvi->u4CfaQueryType*/
	Spt4CfaInqInfNotify(pvSptHdl, CFA_AVI_QUERY_TYPE_PARSING_MODE);

	/* Finish parsing ??? --- need LPE range is accurated!!*/
	CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - (u64)1);
}


static void CfaAviAnaStExtraDataDone(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:Send extra data done, cur:0x%llx, to:0x%llx\r\n"),
		prCfaAvi->u8Ca , prCfaAvi->u8MinStartStrmOfst);

	prCfaAvi->u8Ca = prCfaAvi->u8MinStartStrmOfst;

	CfaAviNextScSearch(pvSptHdl, prCfaAvi, CFA_AVI_ANA_AVIPRS_MOVI, (u64)0,
					   AVI_GEN_READ_BYTES, (u32)0);
}


static void CfaAviAnaStExtraData(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	MRESULT mrRet = RET_DMX_OK;
	u64 u8Sa = prCfaAvi->u8Ca;
	u32 u4ExtraDataLen = 0;
	CFA_VIDEO_INFO_T rVidInf = {0};

	u4ExtraDataLen = prCfaAvi->u4VidCodecSpecDataLen;
	DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:Send extra data: 0x%llx, Len 0x%llx\r\n"), u8Sa, u4ExtraDataLen);

	prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_TX_EXTRA_DATA_DONE;

	LOAD_BYTE(prCfaAvi->pucHdrBuf, u4ExtraDataLen);

	if (0 == u4ExtraDataLen) {
		/*DMX_ASSERT(FALSE);*/
		CfaAviAnaStExtraDataDone(pvSptHdl, prCfaAvi);
		return;
	}

	rVidInf.fgQueryWVC1Mode = FALSE;
	rVidInf.u8FileOfst = u8Sa;
	rVidInf.eTxMode = CFA_PTM_WMV_SEQHDR;
	rVidInf.fgUnitStart = TRUE;
	rVidInf.u8TotalAULen = u4ExtraDataLen;
	rVidInf.eVidType = prCfaAvi->eVidType;
	rVidInf.u8Len = u4ExtraDataLen;
	rVidInf.u4PrsStrmId = (u32)(prCfaAvi->rCfaAviVInf.tStrmInf.u4StrmIdx);

	mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rVidInf);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI] CfaAviAnaStExtraData fail")
			TEXT("in Spt4CfaPbb2VFifoAUCtrl, ret:%d\r\n"), mrRet);
		CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - (u64)1);
	}
}


/*-----------------------------------------------------------------------------
 * Name: vCfaAviAnaStTxVc1VChunk
 *
 * Description:
 *		Judging if the vc1 start code exists in video chunk for future processing
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static void CfaAviAnaStCheckStartCode(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	if (((prCfaAvi->pucHdrBuf[0] == 0x0) &&
		 (prCfaAvi->pucHdrBuf[1] == 0x0) &&
		 (prCfaAvi->pucHdrBuf[2] == 0x1) &&
		 (prCfaAvi->pucHdrBuf[3] == 0xD))	||
		((prCfaAvi->pucHdrBuf[0] == 0x0) &&
		 (prCfaAvi->pucHdrBuf[1] == 0x0) &&
		 (prCfaAvi->pucHdrBuf[2] == 0x1) &&
		 (prCfaAvi->pucHdrBuf[3] == 0xE))	||
		((prCfaAvi->pucHdrBuf[0] == 0x0) &&
		 (prCfaAvi->pucHdrBuf[1] == 0x0) &&
		 (prCfaAvi->pucHdrBuf[2] == 0x1) &&
		 (prCfaAvi->pucHdrBuf[3] == 0xF)))
		prCfaAvi->fgNeedTxStartCode = FALSE;
	else {
		prCfaAvi->fgNeedTxStartCode = TRUE;

		if (NULL == prCfaAvi->pucVc1) {
			DMX_NewHwMemory(sizeof(u8) * 4, prCfaAvi->pucVc1);

			if (prCfaAvi->pucVc1 != NULL) {
				*(prCfaAvi->pucVc1) = 0x0;
				*(prCfaAvi->pucVc1 + 1) = 0x0;
				*(prCfaAvi->pucVc1 + 2) = 0x1;
				*(prCfaAvi->pucVc1 + 3) = 0xD;
			} else {
				DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
					TEXT("[CFA AVI]: Malloc prCfaAvi->pucVc1 Fail!\r\n"));

				CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - 1);
			}
		} else
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]:prCfaAvi->pucVc1 != NULL\r\n"));
	}

	prCfaAvi->fgGetVc1Case = TRUE;

	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:vCfaAviAnaStVc1StartCode, fgTxStartCode:0x%x, u8Ca:0x%llx.\r\n"),
		prCfaAvi->fgNeedTxStartCode, prCfaAvi->u8Ca);
	prCfaAvi->u8Ca -= AVI_VC1_CHK_START_CODE_BYTE;

	CfaAviAnaStTxVc1VChunk(pvSptHdl, prCfaAvi);
}

/*-----------------------------------------------------------------------------
 * Name: CfaAviTxDoneStCtrl
 *
 * Description:
 *		AVI CFA state control for transfer done
 *		This function will be called after a transfer is complete.
 *
 * Inputs:
 *		[IN] uintptr_t of splitter
 *		[IN] Actual transferred data length.  Normally this value should be equal to the u4Len in
 *			  the previous transfer issue, unless file end is hit.
 *		[IN] pointer to CfaAviInst
 *
 * Outputs:
 *
 * Returns: None
 *
 *-----------------------------------------------------------------------------*/
void CfaAviTxDoneStCtrl(void *pvSptHdl, u64 u8TxLen, CfaAviInst *prCfaAvi)
{
	#if AVI_DRM_DEBUG_LOG_ON
	DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("[CFA AVI][DONE CTRL] ==Ca:0x%llx u8TxLen:%lld eCurCfaAviAnaSt:0x%x ==\r\n"),
		prCfaAvi->u8Ca, u8TxLen, prCfaAvi->eCurCfaAviAnaSt);
	#endif

	/*using sync DMA, 071228 */
	prCfaAvi->pucHdrBuf = (u8 *)prCfaAvi->ptrMemAddress;

	#if CONFIG_CFA_DIVX_DRM_SUPPORT
	prCfaAvi->pucDrmTmpBuf = (u8 *)prCfaAvi->ptrDrmMemAddress;
	#endif

	#if CONFIG_CHECK_TX_DATA_TIME
	if (prCfaAvi->rCfaStattime.fgStart) {
		prCfaAvi->rCfaStattime.fgStart = FALSE;
		prCfaAvi->rCfaStattime.u4EndMs = GetTickCount();
		/*DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON, TEXT("[CFA][AVI] V Consume: %d\r\n"),
			_rCfaStattime.u4EndMs - _rCfaStattime.u4StartMs);*/
	}
	#endif

	if (CFA_AVI_ANA_AVIPRS_TX_VC1_START_CODE != prCfaAvi->eCurCfaAviAnaSt) {
		if ((CFA_AVI_ANA_AVIPRS_TX_AUD_HDR != prCfaAvi->eCurCfaAviAnaSt) &&
			(CFA_AVI_ANA_AVIPRS_TX_VID_HDR != prCfaAvi->eCurCfaAviAnaSt) &&
			(CFA_AVI_ANA_AVIPRS_TX_AVC_START_CODE != prCfaAvi->eCurCfaAviAnaSt) &&
			(CFA_AVI_ANA_AVIPRS_RE_TX_VID_MPEG1_CODEC_HEADER != prCfaAvi->eCurCfaAviAnaSt) &&
			(CFA_AVI_ANA_AVIPRS_RE_TX_VID_MPEG4_VOL_HEADER != prCfaAvi->eCurCfaAviAnaSt)&&
			(CFA_AVI_ANA_AVIPRS_RE_TX_VID_H264OrH265_HEADER != prCfaAvi->eCurCfaAviAnaSt)
		) {
			#if AVI_DRM_DEBUG_LOG_ON
			DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI][DONE CTRL] ADD CA \r\n"));
			#endif

			prCfaAvi->u8Ca += u8TxLen;
			prCfaAvi->u4GTxLen = (u32)u8TxLen;
		}
	}

	#if CONFIG_CFA_AVI_NEW_SYNC_PB_BUF_FLOW
	prCfaAvi->fgCanUseNewSyncFlow = TRUE;

	if (prCfaAvi->fgNeedAdjustAfterTxDone) {
		/*Avoid reach boundary*/
		if (prCfaAvi->u4RemainDataInPbBuf > (4 * 1024))
			prCfaAvi->u4RemainDataInPbBuf -= (4 * 1024);

		prCfaAvi->fgNeedAdjustAfterTxDone = FALSE;
	}
	#endif

	switch (prCfaAvi->eCurCfaAviAnaSt) {
	case CFA_AVI_ANA_ST_IDLE:
		CfaAviAnaStIdle(pvSptHdl, u8TxLen, prCfaAvi);
		break;

	case CFA_AVI_ANA_AVIPRS_INIT:
		/*CfaAviAnaStInit(); */
		break;

	case CFA_AVI_ANA_AVIPRS_MOVI:
		CfaAviAnaStMovi(pvSptHdl, prCfaAvi);
		break;
	case CFA_AVI_ANA_AVIPRS_TX_VID_HDR:
		prCfaAvi->u4GTxLen = (u32)AVI_GEN_READ_BYTES;
		u8TxLen = AVI_GEN_READ_BYTES;
		prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_MOVI;
		CfaAviAnaStMovi(pvSptHdl, prCfaAvi);
		break;

	case CFA_AVI_ANA_AVIPRS_TX_AUD_HDR:
		prCfaAvi->u4GTxLen = (u32)AVI_GEN_READ_BYTES;
		u8TxLen = AVI_GEN_READ_BYTES;
		prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_MOVI;
		CfaAviAnaStMovi(pvSptHdl, prCfaAvi);
		break;

	case CFA_AVI_ANA_AVIPRS_MOVI_LIST_ID:
		CfaAviAnaStMoviListId(pvSptHdl, prCfaAvi);
		break;

	case CFA_AVI_ANA_AVIPRS_MOVI_SRCH:
		CfaAviMoviSrchSc(pvSptHdl, prCfaAvi, prCfaAvi->u4GTxLen);
		break;

	case CFA_AVI_ANA_AVIPRS_PTX:
		prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_MOVI;
		CfaAviPIsrState(pvSptHdl, prCfaAvi);
		break;

	#if CONFIG_CFA_DIVX_DRM_SUPPORT
	case CFA_AVI_ANA_AVIPRS_DRMINFO:
		CfaAviAnaStDRMInfo(pvSptHdl, prCfaAvi);
		break;
	#endif /* #if CONFIG_CFA_DIVX_DRM_SUPPORT */

	#if CONFIG_CFA_DIVX_SP_SUPPORT
	case CFA_AVI_ANA_AVIPRS_SUBT_DURATION:
		CfaAviAnaStSubtDuration(pvSptHdl, prCfaAvi);
		break;
	#endif /* #if CONFIG_CFA_DIVX_SP_SUPPORT */

	case CFA_AVI_ANA_AVIPRS_TX_DIVX3_VCHUNK:
		CfaAviAnaStTxDivX3VChunk(pvSptHdl, prCfaAvi);
		break;

	case CFA_AVI_ANA_AVIPRS_TX_VP68_VCHUNK:
		CfaAviAnaVP68TxModeVChunk(pvSptHdl, prCfaAvi);
		break;

	case CFA_AVI_ANA_AVIPRS_TX_PARSING_MODE:
		CfaAviAnaStParsingMode(pvSptHdl, prCfaAvi);
		break;

	case CFA_AVI_ANA_AVIPRS_TX_EXTRA_DATA:
		CfaAviAnaStExtraData(pvSptHdl, prCfaAvi);
		break;

	case CFA_AVI_ANA_AVIPRS_TX_EXTRA_DATA_DONE:
		CfaAviAnaStExtraDataDone(pvSptHdl, prCfaAvi);
		break;

	case CFA_AVI_ANA_AVIPRS_START_CODE:
		CfaAviAnaStCheckStartCode(pvSptHdl, prCfaAvi);
		break;

	case CFA_AVI_ANA_AVIPRS_TX_VC1_START_CODE:
		CfaAviPrsTxV(pvSptHdl, prCfaAvi);
		break;

	#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
	case CFA_AVI_ANA_AVIPRS_DETECT_VBR_GARBAGE_DATA:
		CfaAviAnaStDetectVbrGarbageData(pvSptHdl, prCfaAvi);
		break;
	#endif

	#if CONFIG_CFA_AVI_DIVIDE_LARGE_AUD_CHUNK
	case CFA_AVI_ANA_AVIPRS_TX_DIVIDED_AUD:
		CfaAviTxDividedAudData(pvSptHdl, prCfaAvi);
		break;
	#endif

	case CFA_AVI_ANA_AVIPRS_RE_TX_VID_MPEG1_CODEC_HEADER:
		CfaAviRETxMPEG1CocecHeader(pvSptHdl, prCfaAvi);
		break;

	case CFA_AVI_ANA_AVIPRS_RE_TX_VID_MPEG4_VOL_HEADER:
		CfaAviRETxMPEG4VolHeader(pvSptHdl, prCfaAvi);
		break;
	case CFA_AVI_ANA_AVIPRS_RE_TX_VID_H264OrH265_HEADER:
		CfaAviRETxH264H265Header(pvSptHdl, prCfaAvi);
		break;

	case CFA_AVI_ANA_AVIPRS_AVC_SLICE_LEN:
	case CFA_AVI_ANA_AVIPRS_TX_AVC_START_CODE:
	case CFA_AVI_ANA_AVIPRS_TX_AVC_SLICE:
		CfaAviAnaStTxAvcSlice(pvSptHdl, prCfaAvi);
		break;

	default:
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]invalid tx state!!!\r\n"));
		break;
	}
}



#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
static bool CfaAviJudgeVbrGarbageData(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	if (CFA_RANGE_TYPE_INQUERY_VBR_GARBAGE_INF != prCfaAvi->rCfaRange.eRangeType)
		return FALSE;

	CfaAviNextScSearch(pvSptHdl,
						prCfaAvi,
						CFA_AVI_ANA_AVIPRS_DETECT_VBR_GARBAGE_DATA,
						(u64)0,
						AVI_CHK_VBR_GARBAGE_DATA_BYTE,
						(u32)0);

	return TRUE;
}


static void CfaAviAnaStDetectVbrGarbageData(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	if ((prCfaAvi->pucHdrBuf[0] == (u8)0xff) && ((prCfaAvi->pucHdrBuf[1] & (u8)0xf0) == (u8)0xf0)) {
		/* Todo: only support detecting one vbr audio stream now*/
		prCfaAvi->rVbrGarbageInf.u4GarbageBytes = prCfaAvi->rCfaAviAInf[0].tStrmInf.u4TxedByte;

		prCfaAvi->rVbrGarbageInf.u4GarbageChunks = prCfaAvi->rCfaAviAInf[0].tStrmInf.u4TxedChunk;

		prCfaAvi->rVbrGarbageInf.u8GarbageDataEndPtsByCbr =
			CfaAviConvert2Stc(prCfaAvi->rVbrGarbageInf.u4GarbageBytes,
			prCfaAvi->rCfaAviAInf[0].tStrmInf.u4Rate);

		prCfaAvi->rVbrGarbageInf.u8GarbageDataEndPtsByVbr =
			CfaAviChunk2Pts(&(prCfaAvi->rCfaAviAInf[0].tStrmInf),
			prCfaAvi->rVbrGarbageInf.u4GarbageChunks);

		Spt4CfaInqInfNotify(pvSptHdl, CFA_AVI_QUERY_TYPE_VBR_GARBAGE_DATA);

		CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Ca);
	} else {
		TCfaAviPrsStrmInf *ptPrsInf = &(prCfaAvi->rCfaAviPrsStrmInf);

		prCfaAvi->rCfaAviAInf[0].tStrmInf.u4TxedChunk++;
		prCfaAvi->rCfaAviAInf[0].tStrmInf.u4TxedByte += prCfaAvi->u4DataSz;

		ptPrsInf->u4AudPrsChunk++;
		ptPrsInf->u8AudPrsByte += prCfaAvi->u4DataSz;

		prCfaAvi->u8Ca += prCfaAvi->u4DataSz;

		if (prCfaAvi->u4DataSz % 2)
			prCfaAvi->u8Ca += prCfaAvi->u8Ca&((u64)0x01);

		prCfaAvi->u8Ca -= AVI_CHK_VBR_GARBAGE_DATA_BYTE;

		CfaAviNextScSearch(pvSptHdl,
							prCfaAvi,
							CFA_AVI_ANA_AVIPRS_MOVI,
							0,
							AVI_GEN_READ_BYTES,
							0);
	}
}

static bool CfaAviGetVbrGarbageAudioPts(void *pvSptHdl, CfaAviInst *prCfaAvi,
							 TCfaAviStrmInf *ptAStrmInf)
{
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]: ok-----2---prCfaAvi->pucHdrBuf[0] = %x\r\n"), prCfaAvi->pucHdrBuf[8]);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]: ok-----2---prCfaAvi->pucHdrBuf[1] = %x\r\n"), prCfaAvi->pucHdrBuf[9]);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]: ok-----2---prCfaAvi->pucHdrBuf[2] = %x\r\n"), prCfaAvi->pucHdrBuf[10]);
	if ((prCfaAvi->pucHdrBuf[8] == (u8)0xff) && ((prCfaAvi->pucHdrBuf[9] & (u8)0xf0) == (u8)0xf0)) {
		prCfaAvi->fgDeteGabageDataEnd = TRUE;
		prCfaAvi->u8PrsPts = prCfaAvi->u8TrueStartPts +
			CfaAviPts(prCfaAvi, ptAStrmInf) - CfaAviChunk2Pts(ptAStrmInf, prCfaAvi->u4GabageChunkNum);
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]prCfaAvi->u8PrsPts:%lld	Ca:0x%llx  u4GabageChunkNum:%d\r\n"),
			prCfaAvi->u8PrsPts, prCfaAvi->u8Ca, prCfaAvi->u4GabageChunkNum);
	} else {
		prCfaAvi->u4GabageChunkNum++;

		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:befor---1---CfaAviGetVbrGarbageAudioPts, prCfaAvi->u8PrsPts = %d\r\n"),
			prCfaAvi->u8PrsPts);
		prCfaAvi->u8PrsPts = prCfaAvi->u8TrueStartPts;
		prCfaAvi->u8TrueStartPts +=
			CfaAviConvert2Stc(prCfaAvi->u4DataSz, prCfaAvi->rCfaAviAInf[0].tStrmInf.u4Rate);

		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:after---2---CfaAviGetVbrGarbageAudioPts, ")
			TEXT("prCfaAvi->u8PrsPts = %d\r\n"), prCfaAvi->u8PrsPts);
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:CfaAviGetVbrGarbageAudioPts, prCfaAvi->u4DataSz = %d\r\n"), prCfaAvi->u4DataSz);
	}


	return TRUE;
}
#endif

#if CONFIG_CFA_AVI_SUPPORT_AUDIO_DRM
static bool CfaAviDrmProcBeforeTxAudData(void *pvSptHdl, const CfaAviInst *prCfaAvi)
{
	MRESULT mrRet = RET_DMX_OK;
	CFA_DIVXDRM_INFO_T rDivxAudioDRMInf;

	mm_memset((void *)&rDivxAudioDRMInf, (s32)0, (u32)sizeof(CFA_DIVXDRM_INFO_T));

	#if AVI_DRM_DEBUG_LOG_ON
	DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("[CFA AVI][CFA DRM]CfaAviDrmProcBeforeTxAudData Entry!!!\r\n"));
	#endif

	#if CONFIG_CFA_AVI_DIVIDE_LARGE_AUD_CHUNK
	if (FALSE == prCfaAvi->fgStartTxAudChunk) {
		#if AVI_DRM_DEBUG_LOG_ON
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI][CFA DRM] Duration TX Big Chunk!!!\r\n"));
		#endif

		return TRUE;
	}
	#endif

	if (!(prCfaAvi->rCfaAviDRMInf.fgStrdExist)) {
		#if AVI_DRM_DEBUG_LOG_ON
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI][CFA DRM] No STRD Chunk!!!\r\n"));
		#endif

		return TRUE;
	}

	rDivxAudioDRMInf.fgOn				= TRUE;
	rDivxAudioDRMInf.u8DecryptStOfst	= prCfaAvi->u8CurChkOfst;
	rDivxAudioDRMInf.u4DecryptLen		= prCfaAvi->u4DataSz;
	rDivxAudioDRMInf.u2FrameKeyIdx		= DMX_DIVXDRM_INVALID_FRAMEIDX;

	mrRet = Spt4CfaTurnDivxDRM(pvSptHdl, &rDivxAudioDRMInf);

	if (RET_DMX_OK != mrRet) {
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI][CFA DRM] TURN ON FAILED!!!\r\n"));
		return FALSE;
	}
	return TRUE;
}
#endif

#if CONFIG_CFA_AVI_DIVIDE_LARGE_AUD_CHUNK
static bool CfaAviTxDividedAudData(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	bool fgTotalAudChunkTxed = FALSE;
	u64 u8AdvLen = 0;

	prCfaAvi->fgStartTxAudChunk = FALSE;

	#if CONFIG_CFA_AVI_TX_ALL_AUD
	prCfaAvi->rCfaAviAInf[prCfaAvi->ucAudInfoIdx].tStrmInf.u4TxedByte += prCfaAvi->u4AudChunkLastTxedByte;
	#else
	prCfaAvi->rCfaAviAInf[prCfaAvi->ucCurAudInfoIdx].tStrmInf.u4TxedByte += prCfaAvi->u4AudChunkLastTxedByte;
	#endif

	prCfaAvi->rCfaAviPrsStrmInf.u8AudPrsByte += prCfaAvi->u4AudChunkLastTxedByte;

	if (prCfaAvi->u4CurAudChunkTxedOfst >= prCfaAvi->u4DataSz)
		fgTotalAudChunkTxed = TRUE;
	else {
		if (FALSE == CfaAviPrsTxA(pvSptHdl, prCfaAvi, prCfaAvi->u4DataSz)) {
			fgTotalAudChunkTxed = TRUE;
			u8AdvLen = prCfaAvi->u4DataSz - prCfaAvi->u4CurAudChunkTxedOfst;
#if CONFIG_CFA_MKV_SUPPORT_AAC
			if (CFA_AUD_DRV_FMT_AAC ==
				prCfaAvi->rCfaAviAInf[prCfaAvi->ucCurAudInfoIdx].tStrmInf.eAudCodec)
				prCfaAvi->rCfaAviAInf[prCfaAvi->ucCurAudInfoIdx].tStrmInf.fgSetAudHdr = TRUE;

#endif
		}
	}

	if (TRUE == fgTotalAudChunkTxed) {
		prCfaAvi->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_MOVI;

		if (prCfaAvi->eCurPrsPktType != CFA_AVI_PRS_BIT_STRM_TYPE_A) {
			DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI][CFA DRM]Parse type is not Audio, error!!!\r\n"));
			return FALSE;
		}

		if ((CFA_VID_MJPEG == prCfaAvi->eVidType) && ((bool)(prCfaAvi->u4DataSz % (u32)2)))
			prCfaAvi->u8Ca += prCfaAvi->u8Ca&((u64)0x01);
		else if (CFA_VID_MJPEG != prCfaAvi->eVidType)
			prCfaAvi->u8Ca += prCfaAvi->u8Ca & ((u64)0x01);
		else {
			/*do nothing*/
		}

		#if CONFIG_CFA_AVI_SUPPORT_AUDIO_DRM
		if ((prCfaAvi->rCfaAviDRMInf.fgDrmExist) && (prCfaAvi->rDivxDRMInf.fgOn == TRUE)) {
			MRESULT mrRet = RET_DMX_OK;

			prCfaAvi->rDivxDRMInf.fgOn = FALSE;

			mrRet = Spt4CfaTurnDivxDRM(pvSptHdl, &(prCfaAvi->rDivxDRMInf));
			if (RET_DMX_OK != mrRet) {
				DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
					TEXT("[CFA AVI][CFA DRM] TURN ON FAILED!!!\r\n"));
				return FALSE;
			}

			prCfaAvi->u4DrmState = CFA_AVI_DRM_FINISH;
		}
		#endif

		#if CONFIG_CFA_AVI_TX_ALL_AUD
		prCfaAvi->rCfaAviAInf[prCfaAvi->ucAudInfoIdx].tStrmInf.u4TxedChunk++;
		#else
		prCfaAvi->rCfaAviAInf[prCfaAvi->ucCurAudInfoIdx].tStrmInf.u4TxedChunk++;
		#endif

		prCfaAvi->rCfaAviPrsStrmInf.u4AudPrsChunk++;

		CfaAviNextScSearch(pvSptHdl,
				prCfaAvi,
				prCfaAvi->eCurCfaAviAnaSt,
				u8AdvLen,
				AVI_GEN_READ_BYTES,
				0);
	}

	return TRUE;
}
#endif

#if CONFIG_CFA_AVI_SUPPORT_CORRECT_CHUNK_ABNORMITY
static bool CfaAviProcAbnormalChunk(void *pvSptHdl, CfaAviInst *prCfaAvi,
								  u8 *pucHdrBuf)
{
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:CfaAviProcAbnormalChunk, abnormal chunk, ca:0x%llx, ")
		TEXT("size:0x%x, range ea:0x%llx\r\n"),
		prCfaAvi->u8Ca, prCfaAvi->u4DataSz, prCfaAvi->u8Endoffst);

	prCfaAvi->fgNextChunkDetecting = TRUE;

	dmx_memcpy(prCfaAvi->aucPrev4cc, pucHdrBuf, AVI_4CC_BYTES);

	prCfaAvi->u8PrevCa = prCfaAvi->u8Ca;

	prCfaAvi->u4PrevDataSize = (u32)(-1);

	CfaAviNextScSearch(pvSptHdl, prCfaAvi, CFA_AVI_ANA_AVIPRS_MOVI_SRCH,
						0, CFA_AVI_HDRBUF_SZ, 0);

	return TRUE;
}
#endif

#if CONFIG_CFA_AVI_SUPPORT_FIFO_LIMITATION
static bool CfaAviTooLargeChunkForFifo(void *pvSptHdl, CfaAviInst *prCfaAvi,
									u8 *pucHdrBuf)
{
	TCfaAviVInf *ptVInf = &(prCfaAvi->rCfaAviVInf);
	u32 u4FifoSize = 0;

	if (CfaAviMatchChunkId(CFA_AVI_PRS_BIT_STRM_TYPE_V,
		pucHdrBuf, (u8)(ptVInf->tStrmInf.u4StrmIdx)))
		u4FifoSize = prCfaAvi->rFifoInfo.u4VidFifoSize;
	else if (CfaAviMatchAudCkId(pucHdrBuf))
		u4FifoSize = prCfaAvi->rFifoInfo.u4AudFifoSize;
	else if (CfaAviMatchSpCkId(pucHdrBuf))
		u4FifoSize = prCfaAvi->rFifoInfo.u4SpFifoSize;
	else
		return FALSE;

	if (prCfaAvi->u4DataSz > (u4FifoSize / (u32)2)) {
		if (prCfaAvi->u4DataSz > u4FifoSize)

			return TRUE;
	}

	return FALSE;
}
#endif

static bool CfaAviNotiDmxError(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	Cfa2PsrStrmInfo rPathStrmInfo = {0};

	DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("[CFA AVI]:CfaAviNotiDmxError!!, u8Ca:0x%llx\r\n"), prCfaAvi->u8Ca);

	if (CfaAviToPlay(prCfaAvi->u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_V)) {
		TCfaAviVInf *ptVInf = &(prCfaAvi->rCfaAviVInf);
		u32 i = 0;

		rPathStrmInfo.u4VstrmNs = 1;

		for (i = 0; i < rPathStrmInfo.u4VstrmNs; i++)
			rPathStrmInfo.ucDecVidStId[i] = (u8)(ptVInf->tStrmInf.u4StrmIdx);
	}

	if (CfaAviToPlay(prCfaAvi->u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_A)) {
		u32 j = 0;

		rPathStrmInfo.u4AstrmNs = 1;

		for (j = 0; j < rPathStrmInfo.u4AstrmNs; j++)
			rPathStrmInfo.u2DecAudStId[j] = (u16) prCfaAvi->u4CurAudStrmID;
	}

	Spt4CfaFinishedEx(pvSptHdl, prCfaAvi->u8Ca, TRUE, (u32)GAU_E_FAIL);

	return TRUE;
}

static bool CfaAviNotiDmxChunkError(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	Cfa2PsrStrmInfo rPathStrmInfo = {0};

	DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("[CFA AVI]:CfaAviNotiDmxChunkError!!, u8Ca:0x%llx\r\n"), prCfaAvi->u8Ca);

	if (CfaAviToPlay(prCfaAvi->u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_V)) {
		TCfaAviVInf *ptVInf = &(prCfaAvi->rCfaAviVInf);
		u32 i = 0;

		rPathStrmInfo.u4VstrmNs = 1;

		for (i = 0; i < rPathStrmInfo.u4VstrmNs; i++)
			rPathStrmInfo.ucDecVidStId[i] = (u8)(ptVInf->tStrmInf.u4StrmIdx);
	}

	if (CfaAviToPlay(prCfaAvi->u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_A)) {
		u32 j = 0;

		rPathStrmInfo.u4AstrmNs = 1;

		for (j = 0; j < rPathStrmInfo.u4AstrmNs; j++)
			rPathStrmInfo.u2DecAudStId[j] = (u16) prCfaAvi->u4CurAudStrmID;
	}

	if (DMX_IS_RW_PLAY(pvSptHdl))
		Spt4CfaFinishedEx(pvSptHdl, prCfaAvi->u8Ca, FALSE, GAU_E_ERRCHUNK);
	else
		Spt4CfaFinishedEx(pvSptHdl, prCfaAvi->u8Ca, TRUE, GAU_E_ERRCHUNK);

	return TRUE;
}


static bool CfaAviRETxMPEG1CocecHeader(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	CFA_VIDEO_INFO_T rVidInf = {0};
	TCfaAviVInf *prCfaAviVInf = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (prCfaAvi->rCfaAviVInf.tStrmInf.fgSpecMPEG1InfoTxed) {
		prCfaAvi->u8Ca = prCfaAvi->rCfaAviVInf.tStrmInf.u8CaMPEG1Txbefore;

		CfaAviNextScSearch(pvSptHdl,
							prCfaAvi,
							CFA_AVI_ANA_AVIPRS_MOVI,
							0,
							AVI_GEN_READ_BYTES,
							0);
		return TRUE;
	}


	prCfaAviVInf = &prCfaAvi->rCfaAviVInf;

	rVidInf.eTxMode = CFA_PTM_SAME_POS;
	rVidInf.eVidType =	prCfaAvi->eVidType;
	rVidInf.u4PrsStrmId = (u32)(prCfaAviVInf->tStrmInf.u4StrmIdx);

	rVidInf.u8Len = prCfaAviVInf->tStrmInf.rSpecMPEG1Info.u4CodecSpecDataLen;
	rVidInf.u8TotalAULen = prCfaAviVInf->tStrmInf.rSpecMPEG1Info.u4CodecSpecDataLen;

	/*Tx finish*/
	prCfaAvi->rCfaAviVInf.tStrmInf.fgSpecMPEG1InfoTxed = TRUE;

	mrRet = Spt4CfaBuf2VFifoAUCtrl(pvSptHdl, prCfaAviVInf->tStrmInf.rSpecMPEG1Info.pu1CodecSpecData, &rVidInf);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI] CfaAviRETxMPEG1CocecHeader fail")
			TEXT("in Spt4CfaBuf2VFifoAUCtrl, ret:%d\r\n"), mrRet);
		CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - 1);

		return FALSE;
	}

	return TRUE;
}

static bool CfaAviRETxMPEG4VolHeader(void *pvSptHdl, CfaAviInst *prCfaAvi)
{
	CFA_VIDEO_INFO_T rVidInf = {0};
	TCfaAviVInf *prCfaAviVInf = NULL;
	u32 mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("[CFA AVI] CfaAviRETxMPEG4VolHeader entry\r\n"));

	if (prCfaAvi->rCfaAviVInf.tStrmInf.fgSpecMPEG4InfoTxed) {
		prCfaAvi->u8Ca = prCfaAvi->rCfaAviVInf.tStrmInf.u8CaMPEG4InfoTxbefore;

		CfaAviNextScSearch(pvSptHdl,
						   prCfaAvi,
						   CFA_AVI_ANA_AVIPRS_MOVI,
						   0,
						   AVI_GEN_READ_BYTES,
						   0);
		return TRUE;
	}


	prCfaAviVInf = &prCfaAvi->rCfaAviVInf;

	rVidInf.eTxMode = CFA_PTM_SAME_POS;
	rVidInf.eVidType =	prCfaAvi->eVidType;
	rVidInf.u4PrsStrmId = (u32)(prCfaAviVInf->tStrmInf.u4StrmIdx);

	rVidInf.u8Len = prCfaAviVInf->tStrmInf.rSpecMPEG4Info.u4CodecSpecDataLen;
	rVidInf.u8TotalAULen = prCfaAviVInf->tStrmInf.rSpecMPEG4Info.u4CodecSpecDataLen;

	/*Tx finish*/
	prCfaAvi->rCfaAviVInf.tStrmInf.fgSpecMPEG4InfoTxed = TRUE;

	mrRet = Spt4CfaBuf2VFifoAUCtrl(pvSptHdl, prCfaAviVInf->tStrmInf.rSpecMPEG4Info.pu1CodecSpecData, &rVidInf);

	if (RET_DMX_OK != mrRet) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI] CfaAviRETxMPEG4VolHeader fail in ")
			TEXT("Spt4CfaBuf2VFifoAUCtrl, ret:%d\r\n"), mrRet);
		CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - 1);

		return FALSE;
	}

	return TRUE;
}

static BOOL CfaAviRETxH264H265Header(void *pvSptHdl, CfaAviInst* prCfaAvi)
{
	CFA_VIDEO_INFO_T rVidInf = {0};
	TCfaAviVInf* prCfaAviVInf = NULL;
	u32 mrRet = RET_DMX_OK;

	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT, TEXT("[CFA AVI]Entry %s \r\n"), DMX_FUNC_NAME);

	if( prCfaAvi->rCfaAviVInf.tStrmInf.fgSpecH264InfoTxed ) {
		prCfaAvi->u8Ca = prCfaAvi->rCfaAviVInf.tStrmInf.u8CaH264InfoTxbefore;

		CfaAviNextScSearch(pvSptHdl,
			prCfaAvi,
			CFA_AVI_ANA_AVIPRS_MOVI,
			0,
			AVI_GEN_READ_BYTES,
			0);
		return TRUE;
	}


	prCfaAviVInf = &prCfaAvi->rCfaAviVInf;

	rVidInf.eTxMode = CFA_PTM_SAME_POS;
	rVidInf.eVidType =  prCfaAvi->eVidType;
	rVidInf.u4PrsStrmId = (UINT32)(prCfaAviVInf->tStrmInf.u4StrmIdx);

	rVidInf.u8Len = prCfaAviVInf->tStrmInf.rSpecH264H265Info.u4CodecSpecDataLen;
	rVidInf.u8TotalAULen = prCfaAviVInf->tStrmInf.rSpecH264H265Info.u4CodecSpecDataLen;

	//Tx finish
	prCfaAvi->rCfaAviVInf.tStrmInf.fgSpecH264InfoTxed = TRUE;

	mrRet = Spt4CfaBuf2VFifoAUCtrl(pvSptHdl, prCfaAviVInf->tStrmInf.rSpecH264H265Info.pu1CodecSpecData, &rVidInf);

	if (RET_DMX_OK != mrRet) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI] fail in Spt4CfaBuf2VFifoAUCtrl, ret:%d\r\n"), mrRet);
		CfaAviFinishPrs(pvSptHdl, prCfaAvi, prCfaAvi->u8Endoffst - 1);

		return FALSE;
	}
	DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("[CFA AVI]Exit %s \r\n"), DMX_FUNC_NAME);
	return TRUE;

}
