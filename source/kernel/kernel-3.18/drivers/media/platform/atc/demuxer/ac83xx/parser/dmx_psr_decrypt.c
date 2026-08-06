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

/*!
 * @file dmx_psr_decrypt.c
 *
 * @par Project
 *	  MT3360
 *
 * @par Description
 *
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */
#ifdef __linux__
#include <linux/mm.h>
#include "windows.h"
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_decrypt.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "dmx_decrypt.h"
#include "mm_debug.h"
#endif				/* __linux__ */

#include "x_debug.h"
#include "x_rtos.h"

#include "dmx_def.h"
#include "dmx_cli.h"
#include "dmx_dump.h"
#include "dmx_psr_cc.h"
#include "dmx_psr_filter.h"
#include "dmx_parser.h"
#include "dmx_psr_pbbuf.h"
#include "dmx_esm_if.h"
#include "dmx_pbbuf_if.h"
#include "dmx_mem.h"
#include "dmx_gau.h"
#include "dmx_spt.h"
#include "dmx_cpsa.h"
#include "dmx_spt_main.h"
#include "dmx_psr_decrypt.h"
#include "dmx_pfm.h"

#ifndef __linux__
#pragma warning(disable : 4127)	/*disable warning C4127: conditional expression is constant */
#endif

#if DMX_PFM_TEST
EXTERN PSR_PFM g_rPsrPfm;
#endif				/* DMX_PFM_TEST */

EXTERN DMX_CLI_MAN_T g_rDmxCliMan;
EXTERN DMX_DUMP_MAN_T g_rDmxDumpMan;

#if DMX_SUPPORT_DIVXDRM
MRESULT PSR_Decrypt_DecDataDivxDRM(PSR_CC *prPsrCC,
				   E_SPT_DATA_TYPE_T eDataType,
				   u8 *pu1FrameData, u32 u4FrameDataSz)
{
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prPsrCC) || (NULL == pu1FrameData)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[PSR] %s fail for invalid args(framesz: %d)\r\n"),
			DMX_FUNC_NAME, u4FrameDataSz);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (SPT_DATA_V == eDataType) {
		DECRYPT_DIVXDRM_DECRYPT_DATA_T rDecryptData;
		PSR_DECRYPT_DIVXDRM_PRIVDATA_T *prPrivData = NULL;

		mm_memset(&rDecryptData, 0, sizeof(rDecryptData));

		prPrivData = (PSR_DECRYPT_DIVXDRM_PRIVDATA_T *) prPsrCC->rDecryptMan.pvPrivData;

		if (NULL == prPrivData) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[PSR] %s line %d fail for no DivDRM PrivData\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		if (NULL == prPrivData->pu1DrmInfo) {
			DMX_NewMemory(DMX_DIVXDRM_DRMINFO_SZ, prPrivData->pu1DrmInfo);
			if (NULL == prPrivData->pu1DrmInfo) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					TEXT("[PSR] %s line %d fail for no memory\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				MM_RETURN(RET_DMX_NO_MEM);
			}
		}

		LOADL_WORD2BYTE(prPrivData->u2FrameKeyIdx, prPrivData->pu1DrmInfo);
		LOADL_DWRD2BYTE(u4FrameDataSz, prPrivData->pu1DrmInfo + 6);

		rDecryptData.pu1FrameData = pu1FrameData;
		rDecryptData.u4FrameDataSz = u4FrameDataSz;
		rDecryptData.pu1DrmInfo = prPrivData->pu1DrmInfo;

#if DMX_PFM_TEST
		QueryPerformanceCounter(&(g_rPsrPfm.rDecryptV.u4DecryptStTick));
		g_rPsrPfm.rDecryptV.u4DecryptEndTick = g_rPsrPfm.rDecryptV.u4DecryptStTick;
#endif				/* DMX_PFM_TEST */

		mrRet = DmxDecryptExecCmd(prPsrCC->rDecryptMan.pvInst,
					  DIVXDRM_DECRYPT_VIDEO,
					  &rDecryptData, sizeof(rDecryptData), NULL, 0);

#if DMX_PFM_TEST
		QueryPerformanceCounter(&(g_rPsrPfm.rDecryptV.u4DecryptEndTick));
		g_rPsrPfm.rDecryptV.u8DecryptCnt++;
		g_rPsrPfm.rDecryptV.u8DecryptTime.QuadPart +=
		    (g_rPsrPfm.rDecryptV.u4DecryptEndTick.QuadPart -
		     g_rPsrPfm.rDecryptV.u4DecryptStTick.QuadPart);
#endif				/* DMX_PFM_TEST */

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT
				("[PSR] %s line %d fail in decrypting video data, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			MM_RETURN(mrRet);
		}
	} else if (SPT_DATA_A == eDataType) {
		DECRYPT_DIVXDRM_DECRYPT_DATA_T rDecryptData;

		mm_memset(&rDecryptData, 0, sizeof(rDecryptData));

		rDecryptData.pu1FrameData = pu1FrameData;
		rDecryptData.u4FrameDataSz = u4FrameDataSz;
		rDecryptData.pu1DrmInfo = NULL;

#if DMX_PFM_TEST
		QueryPerformanceCounter(&(g_rPsrPfm.rDecryptA.u4DecryptStTick));
		g_rPsrPfm.rDecryptA.u4DecryptEndTick = g_rPsrPfm.rDecryptA.u4DecryptStTick;
#endif				/* DMX_PFM_TEST */

		mrRet = DmxDecryptExecCmd(prPsrCC->rDecryptMan.pvInst,
					  DIVXDRM_DECRYPT_AUDIO,
					  &rDecryptData, sizeof(rDecryptData), NULL, 0);

#if DMX_PFM_TEST
		QueryPerformanceCounter(&(g_rPsrPfm.rDecryptA.u4DecryptEndTick));
		g_rPsrPfm.rDecryptA.u8DecryptCnt++;
		g_rPsrPfm.rDecryptA.u8DecryptTime.QuadPart +=
		    (g_rPsrPfm.rDecryptA.u4DecryptEndTick.QuadPart -
		     g_rPsrPfm.rDecryptA.u4DecryptStTick.QuadPart);
#endif				/* DMX_PFM_TEST */

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT
				("[PSR] %s line %d fail in decrypting audio data, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			MM_RETURN(mrRet);
		}
	} else {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT
			("[PSR] %s line %d fail in unsupport stream type(%d) to do decrypt\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDataType);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	MM_RETURN(RET_DMX_OK);
}
#endif				/* DMX_SUPPORT_DIVXDRM */

#if DMX_DRM_DECRYPT_USE_HW
MRESULT PSR_Decrypt_GetKeyInfo(PSR_CC *prPsrCC, E_SPT_DATA_TYPE_T eDataType)
{
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prPsrCC) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[PSR] %s fail for invalid args\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (DECRYPT_DIVXDRM == prPsrCC->rDecryptMan.eDecryptType) {
		if (SPT_DATA_V == eDataType) {
			DECRYPT_DIVXDRM_VIDEO_KEYINFO_T rKeyInfo;
			PSR_DECRYPT_DIVXDRM_PRIVDATA_T *prPrivData = NULL;

			mm_memset(&rKeyInfo, 0, sizeof(rKeyInfo));

			prPrivData =
			    (PSR_DECRYPT_DIVXDRM_PRIVDATA_T *) prPsrCC->rDecryptMan.pvPrivData;

			if (NULL == prPrivData) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					TEXT("[PSR] %s line %d fail for no DivDRM PrivData\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			rKeyInfo.u2KeyIndex = prPrivData->u2FrameKeyIdx;

			mrRet = DmxDecryptExecCmd(prPsrCC->rDecryptMan.pvInst,
						  DIVXDRM_GET_VIDEO_KEYINFO,
						  NULL, 0, &rKeyInfo, sizeof(rKeyInfo));

			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					TEXT
					 ("[PSR] %s line %d fail in getting video decrypt key info,")
					 TEXT(" mrRet: 0x%x\r\n"), DMX_FUNC_NAME, DMX_LINE_NO,
					mrRet);
				MM_RETURN(mrRet);
			}

			prPsrCC->rDecryptMan.rHWParam.rDRM.rVideo.fgCBC = rKeyInfo.fgCBC;
			prPsrCC->rDecryptMan.rHWParam.rDRM.rVideo.u2KeyLen = rKeyInfo.u2KeyLen;
			if ((rKeyInfo.fgCBC) && (NULL != rKeyInfo.pu1InitVector)) {
				dmx_memcpy(prPsrCC->rDecryptMan.rHWParam.rDRM.rVideo.au1InitVector,
					   rKeyInfo.pu1InitVector, sizeof(u8) * 16);
			}

			if (NULL != rKeyInfo.pu1Key) {
				dmx_memcpy(prPsrCC->rDecryptMan.rHWParam.rDRM.rVideo.au1Key,
					   rKeyInfo.pu1Key, sizeof(u8) * rKeyInfo.u2KeyLen);
			} else {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					TEXT
					("[PSR] %s line %d fail for no video decrypt key info\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
			}

			prPsrCC->rDecryptMan.rHWParam.rDRM.rVideo.u2KeyLen *= 8;

			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[PSR] %s -- (Video) --> KeyLen: %d, fgCBC: %d\r\n"),
				DMX_FUNC_NAME, prPsrCC->rDecryptMan.rHWParam.rDRM.rVideo.u2KeyLen,
				(rKeyInfo.fgCBC ? 1 : 0));
		} else if (SPT_DATA_A == eDataType) {
			DECRYPT_DIVXDRM_AUDIO_KEYINFO_T rKeyInfo;
			PSR_DECRYPT_DIVXDRM_PRIVDATA_T *prPrivData = NULL;

			prPrivData =
			    (PSR_DECRYPT_DIVXDRM_PRIVDATA_T *) prPsrCC->rDecryptMan.pvPrivData;

			if (NULL == prPrivData) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					TEXT("[PSR] %s line %d fail for no DivDRM PrivData\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			mm_memset(&rKeyInfo, 0, sizeof(rKeyInfo));

			mrRet = DmxDecryptExecCmd(prPsrCC->rDecryptMan.pvInst,
						  DIVXDRM_GET_AUDIO_KEYINFO,
						  NULL, 0, &rKeyInfo, sizeof(rKeyInfo));

			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					TEXT
					 ("[PSR] %s line %d fail in getting audio decrypt key info,")
					 TEXT(" mrRet: 0x%x\r\n"), DMX_FUNC_NAME, DMX_LINE_NO,
					mrRet);
				MM_RETURN(mrRet);
			}

			prPrivData->u4ProtectOffset =
			    (u32) (((u32) (rKeyInfo.u1ProtectOffset)) & 0x000000FF);
			prPrivData->u4ProtectSize =
			    (u32) (((u32) (rKeyInfo.u1ProtectSize)) & 0x000000FF);
			prPsrCC->rDecryptMan.rHWParam.rDRM.rAudio.fgCBC = rKeyInfo.fgCBC;
			prPsrCC->rDecryptMan.rHWParam.rDRM.rAudio.u2KeyLen = rKeyInfo.u2KeyLen;
			if ((rKeyInfo.fgCBC) && (NULL != rKeyInfo.pu1InitVector)) {
				dmx_memcpy(prPsrCC->rDecryptMan.rHWParam.rDRM.rAudio.au1InitVector,
					   rKeyInfo.pu1InitVector, sizeof(u8) * 16);
			}

			if (NULL != rKeyInfo.pu1Key) {
				dmx_memcpy(prPsrCC->rDecryptMan.rHWParam.rDRM.rAudio.au1Key,
					   rKeyInfo.pu1Key, sizeof(u8) * rKeyInfo.u2KeyLen);
				prPsrCC->rDecryptMan.rHWParam.rDRM.rAudio.u2KeyLen *= 8;
			} else {
				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					TEXT
					 ("[PSR] %s line %d -- All the audio data don't need to do")
					 TEXT(" decryption\r\n"), DMX_FUNC_NAME, DMX_LINE_NO);
				prPsrCC->rDecryptMan.rHWParam.rDRM.rAudio.u2KeyLen = 0;
				prPrivData->u4ProtectOffset = DMX_INVALID_UINT32;
				prPrivData->u4ProtectSize = DMX_INVALID_UINT32;
			}

			DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT
				 ("[PSR] %s -- (Audio) --> KeyLen: %d, fgCBC: %d, ProtectOffset: %d,")
				 TEXT(" ProtectSize: %d\r\n"), DMX_FUNC_NAME,
				prPsrCC->rDecryptMan.rHWParam.rDRM.rAudio.u2KeyLen,
				(rKeyInfo.fgCBC ? 1 : 0), prPrivData->u4ProtectOffset,
				prPrivData->u4ProtectSize);
		} else {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT
				 ("[PSR] %s line %d fail in unsupport stream type(%d) to do Get")
				 TEXT(" DivxDRM Key Info\r\n"), DMX_FUNC_NAME, DMX_LINE_NO,
				eDataType);
			MM_RETURN(RET_DMX_UNEXPECT);
		}
	}

	MM_RETURN(mrRet);
}
#endif				/* DMX_DRM_DECRYPT_USE_HW */

MRESULT PSR_Decrypt_DecryptData(PSR_CC *prPsrCC,
				E_SPT_DATA_TYPE_T eDataType,
				u8 *pu1FrameData, u32 u4FrameDataSz)
{
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prPsrCC) || (NULL == pu1FrameData)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[PSR] %s fail for invalid args(framesz: %d)\r\n"),
			DMX_FUNC_NAME, u4FrameDataSz);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (DECRYPT_DIVXDRM == prPsrCC->rDecryptMan.eDecryptType) {
#if DMX_SUPPORT_DIVXDRM
		if ((SPT_DATA_V == eDataType) || (SPT_DATA_A == eDataType)) {
			mrRet = PSR_Decrypt_DecDataDivxDRM(prPsrCC, eDataType, pu1FrameData,
							   u4FrameDataSz);
			if (DMX_SUCCEED(mrRet)) {
				prPsrCC->rDecryptMan.eStatus = DECRYPT_COMPLETE;
				MM_RETURN(RET_DMX_OK);
			}
		}
#endif				/* DMX_SUPPORT_DIVXDRM */
	}

	MM_RETURN(mrRet);
}

MRESULT PSR_Decrypt_PbbufCheck4NotAligned(PSR_CC *prPsrCC, u8 *pu1FrameData)
{
	MRESULT mrRet = RET_DMX_OK;

	if (prPsrCC->rDecryptMan.u4TxMemSize < prPsrCC->rDecryptMan.u4DecryptLen) {
		if (0 != prPsrCC->rDecryptMan.ptrTxMemAddr) {
			DMX_FreeHwMemory(prPsrCC->rDecryptMan.ptrTxMemAddr);
			prPsrCC->rDecryptMan.ptrTxMemAddr = 0;
		}
		prPsrCC->rDecryptMan.u4TxMemSize = 0;
		prPsrCC->rDecryptMan.ptrTxMemWPtr = 0;
#ifdef __linux__
		DMX_NewHwAlignMemory(prPsrCC->rDecryptMan.u4DecryptLen,
				     DMX_DECRYPT_DIVXDRM_DECRYPTION_ALIGNSIZE,
				     prPsrCC->rDecryptMan.ptrTxMemAddr);
#else
		DMX_NewHwAlignMemory(prPsrCC->rDecryptMan.u4DecryptLen,
				     DMX_DECRYPT_DIVXDRM_DECRYPTION_ALIGNSIZE,
				     (void *) (prPsrCC->rDecryptMan.ptrTxMemAddr));
#endif				/* #ifdef __linux__ */
		if (0 == prPsrCC->rDecryptMan.ptrTxMemAddr) {
			DmxLogE(DMX_MOD_OTH,
				DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[PSR] %s line %d fail in alloc DIVXDRM Working")
				 TEXT(" Buffer, PsrCC: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC);
			MM_RETURN(RET_DMX_NO_MEM);
		}
		prPsrCC->rDecryptMan.u4TxMemSize = prPsrCC->rDecryptMan.u4DecryptLen;
		dmx_memset((void *) (prPsrCC->rDecryptMan.ptrTxMemAddr), 0,
			   prPsrCC->rDecryptMan.u4TxMemSize);
	}
	dmx_memcpy((void *) (prPsrCC->rDecryptMan.ptrTxMemAddr),
		   pu1FrameData, prPsrCC->rDecryptMan.u4DecryptLen);
	mrRet = PSR_Decrypt_DecryptData(prPsrCC,
					prPsrCC->rDecryptMan.eDataType,
					(u8 *) (prPsrCC->rDecryptMan.ptrTxMemAddr),
					prPsrCC->rDecryptMan.u4DecryptLen);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d fail in PSR_Decrypt_DecryptData")
			 TEXT
			 ("(%s, u4DecryptLen: %d), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			DMX_SPTDATATYPE_STR(prPsrCC->rDecryptMan.eDataType),
			prPsrCC->rDecryptMan.u4DecryptLen, mrRet);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	dmx_memcpy(pu1FrameData,
		   (void *) (prPsrCC->rDecryptMan.ptrTxMemAddr), prPsrCC->rDecryptMan.u4DecryptLen);

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_Decrypt_PbbufCheck4SWDecrypt(PSR_CC *prPsrCC, DMX_READ_BUFFER *prPbbuf)
{
	/* the u4PbbufIdxTmp slot contains the whole encrypted data */
	u8 *pu1FrameData = (u8 *) (prPbbuf->pcPlayBuffer);
	MRESULT mrRet = RET_DMX_OK;

	pu1FrameData += (u32) (prPsrCC->rDecryptMan.u8DecryptStOft - prPbbuf->u8SrcOffset);

#if DMX_PRINT_DECRYPT_KEY_LOG
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[PSR] %s line %d -- PSR_Decrypt_DecryptData(In Slot, %s), ")
		TEXT("DecryptStOfst(" DMX_UINT64_16U_LOGSTR "), ")
		TEXT("DecryptLen(0x%08x)\r\n")),
	    DMX_FUNC_NAME, DMX_LINE_NO,
	    DMX_SPTDATATYPE_STR(prPsrCC->rDecryptMan.eDataType),
	    DMX_UINT64_16U_LOG_H(prPsrCC->rDecryptMan.u8DecryptStOft),
	    DMX_UINT64_16U_LOG_L(prPsrCC->rDecryptMan.u8DecryptStOft),
	    prPsrCC->rDecryptMan.u4DecryptLen);
#endif				/* DMX_PRINT_DECRYPT_KEY_LOG */

	if (g_rDmxCliMan.fgDumpFlow) {
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo1;
		DMX_DUMP_FLOW_OPER_INFO_T rOperInfo2;

		mm_memset(&rOperInfo1, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo1.pvSptHdl = prPsrCC->pvSptHdl;
		rOperInfo1.unFlow.rComposeDecrypt.u4StmType = prPsrCC->rDecryptMan.eDataType;
		rOperInfo1.unFlow.rComposeDecrypt.u8FileOfst = prPsrCC->rDecryptMan.u8DecryptStOft;
		rOperInfo1.unFlow.rComposeDecrypt.u4Len = prPsrCC->rDecryptMan.u4DecryptLen;
		rOperInfo1.unFlow.rComposeDecrypt.u8SlotSrcOfst = prPbbuf->u8SrcOffset;
		rOperInfo1.unFlow.rComposeDecrypt.u4SlotDataSz = prPbbuf->u4DataSize;
		rOperInfo1.unFlow.rComposeDecrypt.ePosition = DMX_COMPOSE_DECRYPT_POS_WHOLE;
		DmxDumpFlow(DMX_OPER_COMPOSE_DECRYPT_DATA, &rOperInfo1);

		mm_memset(&rOperInfo2, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
		rOperInfo2.pvSptHdl = prPsrCC->pvSptHdl;
		rOperInfo2.unFlow.rDecrypt.u4StmType = prPsrCC->rDecryptMan.eDataType;
		rOperInfo2.unFlow.rDecrypt.u8FileOfst = prPsrCC->rDecryptMan.u8DecryptStOft;
		rOperInfo2.unFlow.rDecrypt.u4Len = prPsrCC->rDecryptMan.u4DecryptLen;
		DmxDumpFlow(DMX_OPER_DECRYPT, &rOperInfo2);
	}

	if ((((u32) pu1FrameData) & 0x03) != 0) {
		mrRet = PSR_Decrypt_PbbufCheck4NotAligned(prPsrCC, pu1FrameData);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[DECRYPT] %s line %d fail in ")
				TEXT("PSR_Decrypt_PbbufCheck4NotAligned(%s, u4DecryptLen: %d),")
				TEXT("mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				DMX_SPTDATATYPE_STR(prPsrCC->rDecryptMan.eDataType),
				prPsrCC->rDecryptMan.u4DecryptLen, mrRet);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	} else if ((0 != ((prPsrCC->rDecryptMan.u4DecryptLen) %
			  (prPsrCC->rDecryptMan.u4AlignSize)))) {
		mrRet = PSR_Decrypt_PbbufCheck4NotAligned(prPsrCC, pu1FrameData);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[DECRYPT] %s line %d fail in ")
				 TEXT("PSR_Decrypt_PbbufCheck4NotAligned(%s, u4DecryptLen: %d),")
				 TEXT("mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				DMX_SPTDATATYPE_STR(prPsrCC->rDecryptMan.eDataType),
				prPsrCC->rDecryptMan.u4DecryptLen, mrRet);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	} else {
		mrRet =
		    PSR_Decrypt_DecryptData(prPsrCC,
					    prPsrCC->rDecryptMan.eDataType,
					    pu1FrameData, prPsrCC->rDecryptMan.u4DecryptLen);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[DECRYPT] %s line %d fail in PSR_Decrypt_DecryptData")
				 TEXT("(%s, u4DecryptLen: %d), mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				DMX_SPTDATATYPE_STR(prPsrCC->rDecryptMan.eDataType),
				prPsrCC->rDecryptMan.u4DecryptLen, mrRet);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}

	MM_RETURN(mrRet);
}

MRESULT PSR_Decrypt_PbbufCheck(PSR_CC *prPsrCC,
			       u32 u4PbbufIdx,
			       bool *fgDecryptEndOftIn,
			       E_PBBUF_CONTINUITY_TYPE_T *pePbbufCon, bool fgForceDecrypt) {
	DMX_READ_BUFFER *prPbbuf = NULL;
	bool fgPbbufIn = FALSE;
	u32 u4PbbufIdxTmp = 0;

	u64 u8LastSrcOfst = DMX_INVALID_UINT64;

	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prPsrCC) || (u4PbbufIdx >= MAX_CACHE_PBBUF)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s fail for invalid args\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (NULL != fgDecryptEndOftIn)
		*fgDecryptEndOftIn = FALSE;

	if (NULL != pePbbufCon)
		*pePbbufCon = PBBUF_CONTINUOUS;

	if ((0 == (prPsrCC->u4Flag & CCF_CPS_ON)) ||
	    (0 == prPsrCC->rDecryptMan.u4DecryptLen) ||
	    (DECRYPT_COMPLETE == prPsrCC->rDecryptMan.eStatus)) {
		if (NULL != fgDecryptEndOftIn)
			*fgDecryptEndOftIn = TRUE;

		if (NULL != pePbbufCon)
			*pePbbufCon = PBBUF_CONTINUOUS;

		MM_RETURN(RET_DMX_OK);
	}

	for (u4PbbufIdxTmp = u4PbbufIdx; u4PbbufIdxTmp < MAX_CACHE_PBBUF; u4PbbufIdxTmp++) {
		if (NULL == prPsrCC->arPBBuf[u4PbbufIdxTmp].pcPlayBuffer) {
			break;
		} else if (PSR_CC_IsOffsetInPbbuf(prPsrCC, u4PbbufIdxTmp,
						  (prPsrCC->rDecryptMan.u8DecryptStOft +
						   prPsrCC->rDecryptMan.u4DecryptLen))) {
			fgPbbufIn = TRUE;
			break;
		}
		if (DMX_INVALID_UINT64 != u8LastSrcOfst) {
			if (u8LastSrcOfst != prPsrCC->arPBBuf[u4PbbufIdxTmp].u8SrcOffset) {
				prPsrCC->u4TxPBBufJumpIdx = u4PbbufIdxTmp;
				if (NULL != pePbbufCon)
					*pePbbufCon = PBBUF_UNCONTINUOUS;
				if (NULL != fgDecryptEndOftIn)
					*fgDecryptEndOftIn = FALSE;
				MM_RETURN(RET_DMX_OK);
			}
		}

		u8LastSrcOfst = prPsrCC->arPBBuf[u4PbbufIdxTmp].u8SrcOffset +
		    prPsrCC->arPBBuf[u4PbbufIdxTmp].u4DataSize;
	}

	if (!fgForceDecrypt)
		MM_RETURN(RET_DMX_OK);

	while (!fgPbbufIn) {
		mrRet = PSR_CC_GetPBBufSlot(prPsrCC, &u4PbbufIdxTmp);
		if (DMX_FAILED(mrRet)) {
			SplitterSetEOSForError(prPsrCC->pvSptHdl, mrRet);
			MM_RETURN(mrRet);
		}

		if (MAX_CACHE_PBBUF == u4PbbufIdxTmp) {
			if (NULL != fgDecryptEndOftIn)
				*fgDecryptEndOftIn = FALSE;

			if (NULL != pePbbufCon)
				*pePbbufCon = PBBUF_CONTINUOUS;

			break;
		}

		if (PSR_CC_IsPbbufUnCon(prPsrCC, u4PbbufIdxTmp)) {
			prPsrCC->u4TxPBBufJumpIdx = u4PbbufIdxTmp;
			if (NULL != fgDecryptEndOftIn)
				*fgDecryptEndOftIn = FALSE;

			if (NULL != pePbbufCon)
				*pePbbufCon = PBBUF_UNCONTINUOUS;

			MM_RETURN(RET_DMX_OK);
		} else if (PSR_CC_IsOffsetInPbbuf(prPsrCC, u4PbbufIdxTmp,
						  (prPsrCC->rDecryptMan.u8DecryptStOft +
						   prPsrCC->rDecryptMan.u4DecryptLen))) {
			if (NULL != fgDecryptEndOftIn)
				*fgDecryptEndOftIn = TRUE;

			if (NULL != pePbbufCon)
				*pePbbufCon = PBBUF_CONTINUOUS;

			fgPbbufIn = TRUE;
		}
	}

	u8LastSrcOfst = DMX_INVALID_UINT64;

	for (u4PbbufIdxTmp = 0; u4PbbufIdxTmp < MAX_CACHE_PBBUF; u4PbbufIdxTmp++) {
		prPbbuf = &(prPsrCC->arPBBuf[u4PbbufIdxTmp]);
		if ((0 == prPbbuf->u8SrcOffset) || (0 == prPbbuf->u4DataSize))
			break;

		if ((prPbbuf->u8SrcOffset <= prPsrCC->rDecryptMan.u8DecryptStOft) &&
		    (prPsrCC->rDecryptMan.u8DecryptStOft + prPsrCC->rDecryptMan.u4DecryptLen
		     <= prPbbuf->u8SrcOffset + prPbbuf->u4DataSize)) {
			if (DECRYPT_BY_SW == prPsrCC->rDecryptMan.eMethod) {
				mrRet = PSR_Decrypt_PbbufCheck4SWDecrypt(prPsrCC, prPbbuf);
				if (DMX_FAILED(mrRet)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
						TEXT
						 ("[DECRYPT] %s line %d fail in PSR_Decrypt_PbbufCheck4SWDecrypt")
						 TEXT
						 ("(%s, u4DecryptLen: %d), mrRet: 0x%x\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						DMX_SPTDATATYPE_STR(prPsrCC->rDecryptMan.eDataType),
						prPsrCC->rDecryptMan.u4DecryptLen, mrRet);
					MM_RETURN(RET_DMX_PARAM_WRONG);
				}
			}

			if (NULL != fgDecryptEndOftIn)
				*fgDecryptEndOftIn = TRUE;

			if (NULL != pePbbufCon)
				*pePbbufCon = PBBUF_CONTINUOUS;

			MM_RETURN(RET_DMX_OK);
		} else if ((prPsrCC->rDecryptMan.u8DecryptStOft < prPbbuf->u8SrcOffset) &&
			   (prPsrCC->rDecryptMan.u8DecryptStOft + prPsrCC->rDecryptMan.u4DecryptLen
			    <= prPbbuf->u8SrcOffset + prPbbuf->u4DataSize)) {
			/* the u4PbbufIdxTmp slot contains the end part of the encrypted data */

			u64 u8TxStOffset = DMX_MAX(prPsrCC->rDecryptMan.u8DecryptStOft,
						      prPbbuf->u8SrcOffset);
			u64 u8TxEndOffset = DMX_MIN((prPsrCC->rDecryptMan.u8DecryptStOft +
							prPsrCC->rDecryptMan.u4DecryptLen),
						       (prPbbuf->u8SrcOffset +
							prPbbuf->u4DataSize));
			u8 *pu1TxSa =
			    (u8 *) ((u32) (prPbbuf->pcPlayBuffer) +
				       (u32) (u8TxStOffset - prPbbuf->u8SrcOffset));

#if DMX_PRINT_DECRYPT_KEY_LOG
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[PSR] %s line %d -- Accross Slots(%s) (")
				 TEXT("the End Part(PartLen: 0x%08x), PbbufIdx(%d), TmpIdx(%d)), ")
				 TEXT("DecryptStOfst(" DMX_UINT64_16U_LOGSTR "), ")
				 TEXT("DecryptLen(0x%08x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				DMX_SPTDATATYPE_STR(prPsrCC->rDecryptMan.eDataType),
				(u32) (u8TxEndOffset - u8TxStOffset),
				u4PbbufIdx, u4PbbufIdxTmp,
				DMX_UINT64_16U_LOG_H(prPsrCC->rDecryptMan.u8DecryptStOft),
				DMX_UINT64_16U_LOG_L(prPsrCC->rDecryptMan.u8DecryptStOft),
				prPsrCC->rDecryptMan.u4DecryptLen);
#endif				/* DMX_PRINT_DECRYPT_KEY_LOG */

			if (prPsrCC->rDecryptMan.ptrTxMemWPtr < prPsrCC->rDecryptMan.u4DecryptLen) {
				if (prPsrCC->rDecryptMan.ptrTxMemWPtr + (u32) (u8TxEndOffset -
										 u8TxStOffset) >
				    prPsrCC->rDecryptMan.u4TxMemSize) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
						TEXT
						 ("[DECRYPT] %s line %d fail for Decrypted Data Size(%d)")
						 TEXT(" overflow Temp Buffer's Size(%d)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						prPsrCC->rDecryptMan.u4DecryptLen,
						prPsrCC->rDecryptMan.u4TxMemSize);
					MM_RETURN(RET_DMX_OVER_LIMIT);
				}

				dmx_memcpy((u8 *) (prPsrCC->rDecryptMan.ptrTxMemAddr +
						      prPsrCC->rDecryptMan.ptrTxMemWPtr), pu1TxSa,
					   (u32) (u8TxEndOffset - u8TxStOffset));

				prPsrCC->rDecryptMan.ptrTxMemWPtr +=
				    (u32) (u8TxEndOffset - u8TxStOffset);

				if (g_rDmxCliMan.fgDumpFlow) {
					DMX_DUMP_FLOW_OPER_INFO_T rOperInfo1;

					mm_memset(&rOperInfo1, 0,
						  sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
					rOperInfo1.pvSptHdl = prPsrCC->pvSptHdl;
					rOperInfo1.unFlow.rComposeDecrypt.u4StmType =
					    prPsrCC->rDecryptMan.eDataType;
					rOperInfo1.unFlow.rComposeDecrypt.u8FileOfst = u8TxStOffset;
					rOperInfo1.unFlow.rComposeDecrypt.u4Len =
					    (u32) (u8TxEndOffset - u8TxStOffset);
					rOperInfo1.unFlow.rComposeDecrypt.u8SlotSrcOfst =
					    prPbbuf->u8SrcOffset;
					rOperInfo1.unFlow.rComposeDecrypt.u4SlotDataSz =
					    prPbbuf->u4DataSize;
					rOperInfo1.unFlow.rComposeDecrypt.ePosition =
					    DMX_COMPOSE_DECRYPT_POS_END;
					DmxDumpFlow(DMX_OPER_COMPOSE_DECRYPT_DATA, &rOperInfo1);
				}

				if (DECRYPT_BY_SW == prPsrCC->rDecryptMan.eMethod) {
					u8 *pu1FrameData =
					    (u8 *) (prPsrCC->rDecryptMan.ptrTxMemAddr);

#if DMX_PRINT_DECRYPT_KEY_LOG
					DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
						TEXT
						 ("[PSR] %s line %d -- PSR_Decrypt_DecryptData(")
						 TEXT("Accross Slot, %s), ")
						 TEXT("DecryptStOfst(" DMX_UINT64_16U_LOGSTR "), ")
						 TEXT("ptrTxMemWPtr(%d), DecryptLen(0x%08x)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						DMX_SPTDATATYPE_STR(prPsrCC->rDecryptMan.eDataType),
						DMX_UINT64_16U_LOG_H(prPsrCC->rDecryptMan.u8DecryptStOft),
						DMX_UINT64_16U_LOG_L(prPsrCC->rDecryptMan.u8DecryptStOft),
						prPsrCC->rDecryptMan.ptrTxMemWPtr,
						prPsrCC->rDecryptMan.u4DecryptLen);
#endif				/* DMX_PRINT_DECRYPT_KEY_LOG */

					if (g_rDmxCliMan.fgDumpFlow) {
						DMX_DUMP_FLOW_OPER_INFO_T rOperInfo2;

						mm_memset(&rOperInfo2, 0,
							  sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
						rOperInfo2.pvSptHdl = prPsrCC->pvSptHdl;
						rOperInfo2.unFlow.rDecrypt.u4StmType =
						    prPsrCC->rDecryptMan.eDataType;
						rOperInfo2.unFlow.rDecrypt.u8FileOfst =
						    prPsrCC->rDecryptMan.u8DecryptStOft;
						rOperInfo2.unFlow.rDecrypt.u4Len =
						    prPsrCC->rDecryptMan.u4DecryptLen;
						DmxDumpFlow(DMX_OPER_DECRYPT, &rOperInfo2);
					}

					mrRet = PSR_Decrypt_DecryptData(prPsrCC,
									prPsrCC->
									rDecryptMan.eDataType,
									pu1FrameData,
									prPsrCC->
									rDecryptMan.u4DecryptLen);
					if (DMX_FAILED(mrRet)) {
						DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
							TEXT
							 ("[DECRYPT] %s line %d fail in PSR_Decrypt_DecryptData")
							 TEXT
							 ("(%s, u4DecryptLen: %d), mrRet: 0x%x\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							DMX_SPTDATATYPE_STR(prPsrCC->
									    rDecryptMan.eDataType),
							prPsrCC->rDecryptMan.u4DecryptLen, mrRet);
						MM_RETURN(RET_DMX_PARAM_WRONG);
					}
				}
			}

			if (NULL != fgDecryptEndOftIn)
				*fgDecryptEndOftIn = TRUE;

			if (NULL != pePbbufCon)
				*pePbbufCon = PBBUF_CONTINUOUS;

			MM_RETURN(RET_DMX_OK);
		} else if ((prPbbuf->u8SrcOffset <= prPsrCC->rDecryptMan.u8DecryptStOft) &&
			   (prPsrCC->rDecryptMan.u8DecryptStOft < prPbbuf->u8SrcOffset +
			    prPbbuf->u4DataSize) &&
			   (prPbbuf->u8SrcOffset + prPbbuf->u4DataSize <
			    prPsrCC->rDecryptMan.u8DecryptStOft +
			    prPsrCC->rDecryptMan.u4DecryptLen)) {
			/* the u4PbbufIdxTmp slot contains the first part of the encrypted data */

			u64 u8TxStOffset = DMX_MAX(prPsrCC->rDecryptMan.u8DecryptStOft,
						      prPbbuf->u8SrcOffset);
			u64 u8TxEndOffset = DMX_MIN((prPsrCC->rDecryptMan.u8DecryptStOft +
							prPsrCC->rDecryptMan.u4DecryptLen),
						       (prPbbuf->u8SrcOffset +
							prPbbuf->u4DataSize));
			u8 *pu1TxSa =
			    (u8 *) ((u32) (prPbbuf->pcPlayBuffer) +
				       (u32) (u8TxStOffset - prPbbuf->u8SrcOffset));

#if DMX_PRINT_DECRYPT_KEY_LOG
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[PSR] %s line %d -- Accross Slots(%s) (")
				 TEXT("the first Part(PartLen: %d), PbbufIdx(%d), TmpIdx(%d)), ")
				 TEXT("DecryptStOfst(" DMX_UINT64_16U_LOGSTR "), ")
				 TEXT("DecryptLen(0x%08x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				DMX_SPTDATATYPE_STR(prPsrCC->rDecryptMan.eDataType),
				(u32) (u8TxEndOffset - u8TxStOffset),
				u4PbbufIdx, u4PbbufIdxTmp,
				DMX_UINT64_16U_LOG(prPsrCC->rDecryptMan.u8DecryptStOft),
				DMX_UINT64_16U_LOG_H(prPsrCC->rDecryptMan.u8DecryptStOft),
				DMX_UINT64_16U_LOG_L(prPsrCC->rDecryptMan.u8DecryptStOft),
				prPsrCC->rDecryptMan.u4DecryptLen);
#endif				/* DMX_PRINT_DECRYPT_KEY_LOG */

			if (0 == prPsrCC->rDecryptMan.ptrTxMemWPtr) {
				if (prPsrCC->rDecryptMan.ptrTxMemWPtr + (u32) (u8TxEndOffset -
										 u8TxStOffset) >
				    prPsrCC->rDecryptMan.u4TxMemSize) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
						TEXT
						 ("[DECRYPT] %s line %d fail for Decrypted Data Size(%d)")
						 TEXT(" overflow Temp Buffer's Size(%d)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						prPsrCC->rDecryptMan.u4DecryptLen,
						prPsrCC->rDecryptMan.u4TxMemSize);
					MM_RETURN(RET_DMX_OVER_LIMIT);
				}

				dmx_memcpy((u8 *) prPsrCC->rDecryptMan.ptrTxMemAddr, pu1TxSa,
					   (u32) (u8TxEndOffset - u8TxStOffset));

				prPsrCC->rDecryptMan.ptrTxMemWPtr +=
				    (u32) (u8TxEndOffset - u8TxStOffset);

				if (g_rDmxCliMan.fgDumpFlow) {
					DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

					mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
					rOperInfo.pvSptHdl = prPsrCC->pvSptHdl;
					rOperInfo.unFlow.rComposeDecrypt.u4StmType =
					    prPsrCC->rDecryptMan.eDataType;
					rOperInfo.unFlow.rComposeDecrypt.u8FileOfst = u8TxStOffset;
					rOperInfo.unFlow.rComposeDecrypt.u4Len =
					    (u32) (u8TxEndOffset - u8TxStOffset);
					rOperInfo.unFlow.rComposeDecrypt.u8SlotSrcOfst =
					    prPbbuf->u8SrcOffset;
					rOperInfo.unFlow.rComposeDecrypt.u4SlotDataSz =
					    prPbbuf->u4DataSize;
					rOperInfo.unFlow.rComposeDecrypt.ePosition =
					    DMX_COMPOSE_DECRYPT_POS_1ST;
					DmxDumpFlow(DMX_OPER_COMPOSE_DECRYPT_DATA, &rOperInfo);
				}
			}
		} else if ((prPsrCC->rDecryptMan.u8DecryptStOft < prPbbuf->u8SrcOffset) &&
			   (prPbbuf->u8SrcOffset + prPbbuf->u4DataSize <
			    prPsrCC->rDecryptMan.u8DecryptStOft +
			    prPsrCC->rDecryptMan.u4DecryptLen)) {
			/* the u4PbbufIdxTmp slot contains the middle part of the encrypted data */

			u64 u8TxStOffset = DMX_MAX(prPsrCC->rDecryptMan.u8DecryptStOft,
						      prPbbuf->u8SrcOffset);
			u64 u8TxEndOffset = DMX_MIN((prPsrCC->rDecryptMan.u8DecryptStOft +
							prPsrCC->rDecryptMan.u4DecryptLen),
						       (prPbbuf->u8SrcOffset +
							prPbbuf->u4DataSize));
			u8 *pu1TxSa =
			    (u8 *) ((u32) (prPbbuf->pcPlayBuffer) +
				       (u32) (u8TxStOffset - prPbbuf->u8SrcOffset));

			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[DECRYPT] %s line %d -- Decrypted data size(%d) exceed one")
				 TEXT(" slot's data size(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				prPsrCC->rDecryptMan.u4DecryptLen, prPbbuf->u4DataSize);

#if DMX_PRINT_DECRYPT_KEY_LOG
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[PSR] %s line %d -- Accross Slot(%s) (")
				 TEXT("the Mid Part(PartLen: %d), PbbufIdx(%d), TmpIdx(%d)), ")
				 TEXT("DecryptStOfst(" DMX_UINT64_16U_LOGSTR "), ")
				 TEXT("DecryptLen(0x%08x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				DMX_SPTDATATYPE_STR(prPsrCC->rDecryptMan.eDataType),
				(u32) (u8TxEndOffset - u8TxStOffset),
				u4PbbufIdx, u4PbbufIdxTmp,
				DMX_UINT64_16U_LOG_H(prPsrCC->rDecryptMan.u8DecryptStOft),
				DMX_UINT64_16U_LOG_L(prPsrCC->rDecryptMan.u8DecryptStOft),
				prPsrCC->rDecryptMan.u4DecryptLen);
#endif				/* DMX_PRINT_DECRYPT_KEY_LOG */

			if (prPsrCC->rDecryptMan.ptrTxMemWPtr <
			    (u8TxEndOffset - prPsrCC->rDecryptMan.u8DecryptStOft)) {
				if (prPsrCC->rDecryptMan.ptrTxMemWPtr +
				    (u32) (u8TxEndOffset - u8TxStOffset) >
				    prPsrCC->rDecryptMan.u4TxMemSize) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
						TEXT
						 ("[DECRYPT] %s line %d fail for Decrypted Data Size(%d)")
						 TEXT(" overflow Temp Buffer's Size(%d)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						prPsrCC->rDecryptMan.u4DecryptLen,
						prPsrCC->rDecryptMan.u4TxMemSize);
					MM_RETURN(RET_DMX_OVER_LIMIT);
				}

				dmx_memcpy((u8 *) (prPsrCC->rDecryptMan.ptrTxMemAddr +
						      prPsrCC->rDecryptMan.ptrTxMemWPtr), pu1TxSa,
					   (u32) (u8TxEndOffset - u8TxStOffset));

				prPsrCC->rDecryptMan.ptrTxMemWPtr +=
				    (u32) (u8TxEndOffset - u8TxStOffset);

				if (g_rDmxCliMan.fgDumpFlow) {
					DMX_DUMP_FLOW_OPER_INFO_T rOperInfo;

					mm_memset(&rOperInfo, 0, sizeof(DMX_DUMP_FLOW_OPER_INFO_T));
					rOperInfo.pvSptHdl = prPsrCC->pvSptHdl;
					rOperInfo.unFlow.rComposeDecrypt.u4StmType =
					    prPsrCC->rDecryptMan.eDataType;
					rOperInfo.unFlow.rComposeDecrypt.u8FileOfst = u8TxStOffset;
					rOperInfo.unFlow.rComposeDecrypt.u4Len =
					    (u32) (u8TxEndOffset - u8TxStOffset);
					rOperInfo.unFlow.rComposeDecrypt.u8SlotSrcOfst =
					    prPbbuf->u8SrcOffset;
					rOperInfo.unFlow.rComposeDecrypt.u4SlotDataSz =
					    prPbbuf->u4DataSize;
					rOperInfo.unFlow.rComposeDecrypt.ePosition =
					    DMX_COMPOSE_DECRYPT_POS_MID;
					DmxDumpFlow(DMX_OPER_COMPOSE_DECRYPT_DATA, &rOperInfo);
				}
			}
		} else {
			/* the u4PbbufIdxTmp slot contains no encrypted data */
		}

		u8LastSrcOfst = prPbbuf->u8SrcOffset + prPbbuf->u4DataSize;
	}

	if ((MAX_CACHE_PBBUF <= u4PbbufIdxTmp) ||
	    ((0 == prPbbuf->u8SrcOffset) || (0 == prPbbuf->u4DataSize))) {
		if (!fgPbbufIn) {
			if (DMX_INVALID_UINT64 != u8LastSrcOfst) {
				if (NULL != fgDecryptEndOftIn)
					*fgDecryptEndOftIn = FALSE;

				if (NULL != pePbbufCon)
					*pePbbufCon = PBBUF_CONTINUOUS;

				prPsrCC->u8TxCurrOffset = u8LastSrcOfst;
				prPsrCC->u4TxPBBufIdx = 0;
				PSR_CC_SetTxSt(prPsrCC, TXS_WAIT_PBBUF);
				MM_RETURN(RET_DMX_OK);
			} else {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					TEXT
					("[DECRYPT] %s line %d -- encounter unexpected error\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				MM_RETURN(RET_DMX_UNEXPECT);
			}
		} else {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[DECRYPT] %s line %d -- encounter unexpected error\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_UNEXPECT);
		}
	} else if (!fgPbbufIn) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d -- encounter unexpected error\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_UNEXPECT);
	} else {
		if (NULL != fgDecryptEndOftIn)
			*fgDecryptEndOftIn = TRUE;

		if (NULL != pePbbufCon)
			*pePbbufCon = PBBUF_CONTINUOUS;
	}

	PSR_CC_SetTxSt(prPsrCC, TXS_PBBUF_OK);

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_Decrypt_Create(PSR_CC *prPsrCC, E_DECRYPT_TYPE_T eDecryptType)
{
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prPsrCC) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[PSR] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	switch (eDecryptType) {
	case DECRYPT_NONE:{
			prPsrCC->rDecryptMan.pvInst = NULL;
			prPsrCC->rDecryptMan.i4DecryptId = DECRYPT_PLAY_INVALID_ID;
			prPsrCC->rDecryptMan.eDecryptType = DECRYPT_NONE;
			prPsrCC->rDecryptMan.eDataType = SPT_DATA_UNDEFINE;
			prPsrCC->rDecryptMan.u8DecryptStOft = DMX_INVALID_UINT64;
			prPsrCC->rDecryptMan.ptrTxMemAddr = 0;
			prPsrCC->rDecryptMan.u4TxMemSize = 0;
			prPsrCC->rDecryptMan.ptrTxMemWPtr = 0;
			prPsrCC->rDecryptMan.ptrTxMemRPtr = 0;
			prPsrCC->rDecryptMan.u8DecryptStOft = DMX_INVALID_UINT64;
			prPsrCC->rDecryptMan.u4DecryptLen = 0;
			prPsrCC->rDecryptMan.u4AlignSize = 0;
			prPsrCC->rDecryptMan.eStatus = DECRYPT_UNCOMPLETE;
			prPsrCC->rDecryptMan.eMethod = DECRYPT_BY_NONE;
		}
		break;
	case DECRYPT_DIVXDRM:{
			PSR_DECRYPT_DIVXDRM_PRIVDATA_T *prDivxDrmPriv = NULL;

			if (DECRYPT_DIVXDRM != prPsrCC->rDecryptMan.eDecryptType) {
				mrRet = PSR_Decrypt_Release(prPsrCC);
				if (DMX_FAILED(mrRet)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
						TEXT
						 ("[PSR] %s line %d fail in PSR_Decrypt_Release(eDecryptType:")
						 TEXT(" %d), mrRet: 0x%x\r\n"), DMX_FUNC_NAME,
						DMX_LINE_NO, prPsrCC->rDecryptMan.eDecryptType,
						mrRet);
					MM_RETURN(mrRet);
				}
			}

			prPsrCC->rDecryptMan.i4DecryptId = SplitterGetDecryptId(prPsrCC->pvSptHdl);
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT
				("[PSR_CC] %s line %d -- prPsrCC->rDecryptMan.i4DecryptId(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->rDecryptMan.i4DecryptId);

			mrRet = DmxDecryptGetPlayInst(eDecryptType,
						      prPsrCC->rDecryptMan.i4DecryptId,
						      &(prPsrCC->rDecryptMan.pvInst));

			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					TEXT
					 ("[PSR] %s line %d fail in DmxGetDecryptInst(eDecryptType: %d),")
					 TEXT(" mrRet: 0x%x\r\n"), DMX_FUNC_NAME, DMX_LINE_NO,
					eDecryptType, mrRet);
				MM_RETURN(mrRet);
			}

			if (NULL_HANDLE == (HANDLE_T) (prPsrCC->rDecryptMan.pvInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					TEXT
					("[PSR] %s line %d fail for Decrypt instance obtained is NULL\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			if (NULL == prPsrCC->rDecryptMan.pvPrivData) {
				DMX_NewMemory(sizeof(PSR_DECRYPT_DIVXDRM_PRIVDATA_T),
					      prPsrCC->rDecryptMan.pvPrivData);
				if (NULL == prPsrCC->rDecryptMan.pvPrivData) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
						TEXT
						 ("[PSR] %s line %d fail in alloc DIVXDRM Priv data, ")
						 TEXT("PsrCC: 0x%x\r\n"), DMX_FUNC_NAME,
						DMX_LINE_NO, prPsrCC);
					MM_RETURN(RET_DMX_NO_MEM);
				}
				dmx_memset(prPsrCC->rDecryptMan.pvPrivData, 0,
					   sizeof(PSR_DECRYPT_DIVXDRM_PRIVDATA_T));
				prDivxDrmPriv =
				    (PSR_DECRYPT_DIVXDRM_PRIVDATA_T *) (prPsrCC->
									rDecryptMan.pvPrivData);
				prDivxDrmPriv->u2FrameKeyIdx = DMX_DIVXDRM_INVALID_FRAMEIDX;
				prDivxDrmPriv->u4ProtectOffset = DMX_INVALID_UINT32;
				prDivxDrmPriv->u4ProtectSize = DMX_INVALID_UINT32;
			}

			prPsrCC->rDecryptMan.eDecryptType = eDecryptType;
			prPsrCC->rDecryptMan.eDataType = SPT_DATA_UNDEFINE;
			prPsrCC->rDecryptMan.ptrTxMemAddr = 0;
			prPsrCC->rDecryptMan.u4TxMemSize = 0;
			prPsrCC->rDecryptMan.ptrTxMemWPtr = 0;
			prPsrCC->rDecryptMan.ptrTxMemRPtr = 0;
			prPsrCC->rDecryptMan.u8DecryptStOft = DMX_INVALID_UINT64;
			prPsrCC->rDecryptMan.u4DecryptLen = 0;
			prPsrCC->rDecryptMan.u4AlignSize = DMX_DECRYPT_DIVXDRM_DECRYPTION_ALIGNSIZE;
			prPsrCC->rDecryptMan.eStatus = DECRYPT_UNCOMPLETE;
			prPsrCC->rDecryptMan.eMethod = DECRYPT_BY_NONE;

#if DMX_DRM_DECRYPT_USE_HW
			dmx_memset(&(prPsrCC->rDecryptMan.rHWParam), 0,
				   sizeof(PSR_DECRYPT_HW_PARAM_T));

			mrRet = PSR_Decrypt_GetKeyInfo(prPsrCC, SPT_DATA_A);

			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					TEXT
					("[PSR] %s line %d failed in PSR_Decrypt_GetKeyInfo(Audio)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType);

				MM_RETURN(mrRet);
			}
#endif				/* DMX_DRM_DECRYPT_USE_HW */
		}
		break;
	default:
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[PSR] %s line %d --- unsupported decrypt type(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eDecryptType);
		break;
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT PSR_Decrypt_Release(PSR_CC *prPsrCC)
{
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_OK);

	switch (prPsrCC->rDecryptMan.eDecryptType) {
	case DECRYPT_NONE:
		break;
	case DECRYPT_DIVXDRM:{
			PSR_DECRYPT_DIVXDRM_PRIVDATA_T *prDivxDrmPriv = NULL;

			if (0 != prPsrCC->rDecryptMan.ptrTxMemAddr) {
				DMX_FreeHwMemory(prPsrCC->rDecryptMan.ptrTxMemAddr);
				prPsrCC->rDecryptMan.ptrTxMemAddr = 0;
			}
			prDivxDrmPriv =
			    (PSR_DECRYPT_DIVXDRM_PRIVDATA_T *) (prPsrCC->rDecryptMan.pvPrivData);
			if (NULL != prDivxDrmPriv) {
				if (NULL != prDivxDrmPriv->pu1DrmInfo)
					DMX_FreeMemory(prDivxDrmPriv->pu1DrmInfo);

				DMX_FreeMemory(prDivxDrmPriv);
				prPsrCC->rDecryptMan.pvPrivData = NULL;
			}
			if (NULL_HANDLE != (HANDLE_T) (prPsrCC->rDecryptMan.pvInst)) {
				mrRet = DmxDecryptRelPlayInst(prPsrCC->rDecryptMan.eDecryptType,
							      prPsrCC->rDecryptMan.pvInst);
				if (DMX_FAILED(mrRet)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
						TEXT
						 ("[PSR_CC] %s line %d fail in DmxReleaseDecryptInst,")
						 TEXT(" mrRet: 0x%x\r\n"), DMX_FUNC_NAME,
						DMX_LINE_NO, mrRet);
				}
			}
		}
		break;
	default:
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[PSR_CC] %s line %d -- unsupport decrytp type(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->rDecryptMan.eDecryptType);
		break;
	}

	prPsrCC->rDecryptMan.pvInst = NULL;
	prPsrCC->rDecryptMan.i4DecryptId = DECRYPT_PLAY_INVALID_ID;
	prPsrCC->rDecryptMan.eDecryptType = DECRYPT_NONE;
	prPsrCC->rDecryptMan.eDataType = SPT_DATA_UNDEFINE;
	prPsrCC->rDecryptMan.ptrTxMemAddr = 0;
	prPsrCC->rDecryptMan.u4TxMemSize = 0;
	prPsrCC->rDecryptMan.ptrTxMemWPtr = 0;
	prPsrCC->rDecryptMan.u8DecryptStOft = DMX_INVALID_UINT64;
	prPsrCC->rDecryptMan.u4DecryptLen = 0;
	prPsrCC->rDecryptMan.u4AlignSize = 0;
	prPsrCC->rDecryptMan.eStatus = DECRYPT_UNCOMPLETE;
	prPsrCC->rDecryptMan.eMethod = DECRYPT_BY_NONE;
	prPsrCC->rDecryptMan.pvPrivData = NULL;
#if DMX_DRM_DECRYPT_USE_HW
	dmx_memset(&(prPsrCC->rDecryptMan.rHWParam), 0, sizeof(PSR_DECRYPT_HW_PARAM_T));
#endif				/* DMX_DRM_DECRYPT_USE_HW */

	MM_RETURN(mrRet);
}

MRESULT PSR_Decrypt_Reset(PSR_CC *prPsrCC)
{
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prPsrCC)
		MM_RETURN(RET_DMX_OK);

	switch (prPsrCC->rDecryptMan.eDecryptType) {
	case DECRYPT_NONE:
		break;
	case DECRYPT_DIVXDRM:{
			PSR_DECRYPT_DIVXDRM_PRIVDATA_T *prDivxDrmPriv = NULL;

			if ((0 < prPsrCC->rDecryptMan.u4TxMemSize) &&
			    (0 != prPsrCC->rDecryptMan.ptrTxMemAddr)) {
				dmx_memset((void *) (prPsrCC->rDecryptMan.ptrTxMemAddr), 0,
					   prPsrCC->rDecryptMan.u4TxMemSize);
			}
			prPsrCC->rDecryptMan.ptrTxMemWPtr = 0;
			prPsrCC->rDecryptMan.ptrTxMemRPtr = 0;
			prPsrCC->rDecryptMan.u8DecryptStOft = DMX_INVALID_UINT64;
			prPsrCC->rDecryptMan.u4DecryptLen = 0;
			prPsrCC->rDecryptMan.eStatus = DECRYPT_UNCOMPLETE;
			prPsrCC->rDecryptMan.eDataType = SPT_DATA_UNDEFINE;
			prPsrCC->rDecryptMan.eStatus = DECRYPT_UNCOMPLETE;
			prPsrCC->rDecryptMan.eMethod = DECRYPT_BY_NONE;
			prDivxDrmPriv =
			    (PSR_DECRYPT_DIVXDRM_PRIVDATA_T *) (prPsrCC->rDecryptMan.pvPrivData);
			if (NULL != prDivxDrmPriv) {
				if (NULL != prDivxDrmPriv->pu1DrmInfo) {
					DMX_FreeMemory(prDivxDrmPriv->pu1DrmInfo);
					prDivxDrmPriv->pu1DrmInfo = NULL;
				}
				prDivxDrmPriv->u2FrameKeyIdx = DMX_DIVXDRM_INVALID_FRAMEIDX;
			}
#if DMX_DRM_DECRYPT_USE_HW
			prPsrCC->rDecryptMan.rHWParam.rDRM.eInSlotType = DECRYPT_NOT_IN_SLOT;
			prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclLen = 0;
			prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptLclOft = 0;
			prPsrCC->rDecryptMan.rHWParam.rDRM.u4DecryptTotalLen = 0;
			/* we should not to clear audio drm info */
			dmx_memset(&(prPsrCC->rDecryptMan.rHWParam.rDRM.rVideo), 0,
				   sizeof(PSR_DRM_AES_INFO_T));
#endif				/* DMX_DRM_DECRYPT_USE_HW */
		}
		break;
	default:
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[PSR_CC] %s line %d -- unsupport decrytp type(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrCC->rDecryptMan.eDecryptType);
		break;
	}

	MM_RETURN(mrRet);
}
