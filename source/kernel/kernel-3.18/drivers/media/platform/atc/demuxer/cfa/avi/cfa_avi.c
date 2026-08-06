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
#include "dmx_mem.h"
#include "dmx_cpsa.h"
#include "dmx_spt_main.h"
#include "cfa_macro.h"
#include "cfa_avi_st_ctrl.h"
#include "mmisc.h"
/*-----------------------------------------------------------------------------
					macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/


static u8 _u1CFAInstanceNs;

/*-----------------------------------------------------------------------------
 * Name: CfaAvi_InitPara
 *
 * Description:
 *		Init CFA AVI internal parameters
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: None
 *
 *-----------------------------------------------------------------------------*/
void CfaAviInitPara(CfaAviInst *prCfaAviInst)
{
	/*-----------------------------------------------------------------------------
	//TCfaAviVInf* prCfaAviVInf = &prCfaAviInst->rCfaAviVInf;
	//TCfaAviAInf* prCfaAviAInf = NULL;
	//TCfaAviStrmInf* prCfaAviSpStrmInf = &prCfaAviInst->rCfaAviSpStrmInf;
	//prCfaAviAInf = &prCfaAviInst->rCfaAviAInf[prCfaAviInst->ucCurAudInfoIdx];
	-----------------------------------------------------------------------------*/

	prCfaAviInst->eCurCfaAviAnaSt = CFA_AVI_ANA_ST_IDLE;
	prCfaAviInst->eCurPrsPktType  = CFA_AVI_PRS_BIT_STRM_TYPE_NONE;
	prCfaAviInst->u8Ca			  = DMX_INVALID_UINT64;
	prCfaAviInst->u4DataSz		  = 0;
	prCfaAviInst->u8PrsPts		  = DMX_INVALID_UINT64;
	prCfaAviInst->u8SpEndPts	  = DMX_INVALID_UINT64;

	/* for CFA only, can be remove to another structure. */
	/* For VOP check. */
	prCfaAviInst->fgBGrouped = FALSE;
	prCfaAviInst->u1ChunkVopNs = 0;
	prCfaAviInst->u1CurBGrpNum = 0;
	prCfaAviInst->u1TotalBGrpNum = 0;
	prCfaAviInst->ePrePicType  = CFA_PIC_UNDEFINE;
	prCfaAviInst->u8CurVPts    = DMX_INVALID_UINT64;
	prCfaAviInst->u8PreVPts    = DMX_INVALID_UINT64;

	/*-----------------------------------------------------------------------------
	// these parameters should not be reset:
	//prCfaAviInst->u4InstHandle;
	//prCfaAviInst->pucHdrBuf;
	//prCfaAviInst->u4CurPrsFlg;
	//prCfaAviInst->rCfaRange;
	//prCfaAviInst->eVidType;

	//prCfaAviVInf->tStrmInf.dwTxedChunk = prCfaAviInst->rCfaRange.u4VidStartChunkNo;
	//prCfaAviAInf->tStrmInf.dwTxedByte  = prCfaAviInst->rCfaRange.u8AudStartByte;	// for CBR
	//prCfaAviAInf->tStrmInf.dwTxedChunk = prCfaAviInst->rCfaRange.u4AudStartChunkNo; // for VBR
	//prCfaAviInst->rCfaAviSpStrmInf.dwTxedChunk = 0;
	-----------------------------------------------------------------------------*/

	/* CFA AVI DRM related. */
	#if CONFIG_CFA_DIVX_DRM_SUPPORT
	prCfaAviInst->u4DrmState = CFA_AVI_DRM_INIT;
	prCfaAviInst->rCfaAviDRMInf.fgDrmExist	  = FALSE;
	prCfaAviInst->rCfaAviDRMInf.u2KeyIdx	   = 0;
	prCfaAviInst->rCfaAviDRMInf.u4EncryptOfst = 0;
	prCfaAviInst->rCfaAviDRMInf.u4EncryptLen  = 0;

	prCfaAviInst->rDivxDRMInf.fgOn = FALSE;
	prCfaAviInst->rDivxDRMInf.u8DecryptStOfst = DMX_INVALID_UINT64;
	prCfaAviInst->rDivxDRMInf.u4DecryptLen = 0;
	prCfaAviInst->rDivxDRMInf.u2FrameKeyIdx = DMX_DIVXDRM_INVALID_FRAMEIDX;
	#endif
}

static CfaApiVidType eCfaGetMapVidType(AVCODECID_T eVidCodec)
{
	CfaApiVidType eMappedVidType = CFA_VID_UNKNOWN;

	switch (eVidCodec) {
	case AVCODEC_ID_UNKNOWN:
		eMappedVidType = CFA_VID_UNKNOWN;
		break;

	case AVCODEC_ID_MPEG2:
		eMappedVidType = CFA_VID_MPEG2;
		break;

	case AVCODEC_ID_DIVX3:
		eMappedVidType = CFA_VID_DIVX3;
		break;

	case AVCODEC_ID_DIVX4:
		eMappedVidType = CFA_VID_DIVX4;
		break;

	case AVCODEC_ID_DIVX5:
		eMappedVidType = CFA_VID_DIVX6;
		break;

	case AVCODEC_ID_MPEG4:
		eMappedVidType = CFA_VID_MPEG4;
		break;

	case AVCODEC_ID_H264:
		eMappedVidType = CFA_VID_H264;
		break;

	case AVCODEC_ID_VC1:
		eMappedVidType = CFA_VID_VC1;
		break;

	case AVCODEC_ID_H263:
		eMappedVidType = CFA_VID_H263;
		break;

	case AVCODEC_ID_WMV1:
		eMappedVidType = CFA_VID_WMV7;
		break;

	case AVCODEC_ID_WMV2:
		eMappedVidType = CFA_VID_WMV8;
		break;

	case AVCODEC_ID_WMV3:
		eMappedVidType = CFA_VID_WMV9;
		break;
	case AVCODEC_ID_SORENSON:
		eMappedVidType = CFA_VID_H263_SORENSON;
		break;

	case AVCODEC_ID_MJPEG:
		eMappedVidType = CFA_VID_MJPEG;
		break;

	case AVCODEC_ID_H265:
		eMappedVidType = CFA_VID_H265;
		break;

	case AVCODEC_ID_VP6:
		eMappedVidType = CFA_VID_VP6;
		break;

	case AVCODEC_ID_VP8:
		eMappedVidType = CFA_VID_VP8;
		break;

	default:
		eMappedVidType = CFA_VID_UNKNOWN;
		break;
	}

	return eMappedVidType;
}

/*-----------------------------------------------------------------------------
 * Name: CfaAviGetAudType
 *
 * Description:
 *		Cfa transfer Audio Codec Enum from LPE to Splitter
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static CfaApiAudType CfaAviGetAudType(AVCODECID_T eAudCodec)
{
	CfaApiAudType eMappedAudType = CFA_AUD_DRV_FMT_UNKNOWN;

	switch (eAudCodec) {
	case AVCODEC_ID_MPEG:
		eMappedAudType = CFA_AUD_DRV_FMT_MPEG;
		break;

	case AVCODEC_ID_AC3:
		eMappedAudType = CFA_AUD_DRV_FMT_AC3;
		break;

	case AVCODEC_ID_PCM:
		eMappedAudType = CFA_AUD_DRV_FMT_PCM;
		break;

	case AVCODEC_ID_MP3:
		eMappedAudType = CFA_AUD_DRV_FMT_MP3;
		break;

	case AVCODEC_ID_DTS:
		eMappedAudType = CFA_AUD_DRV_FMT_DTS;
		break;

	case AVCODEC_ID_WMA:
		eMappedAudType = CFA_AUD_DRV_FMT_WMA;
		break;

	case AVCODEC_ID_VORBIS:
		eMappedAudType = CFA_AUD_DRV_FMT_VORBIS;
		break;

	case AVCODEC_ID_AAC_PURE:
	case AVCODEC_ID_AAC:
		eMappedAudType = CFA_AUD_DRV_FMT_AAC;
		break;
	case AVCODEC_ID_FLAC:
		eMappedAudType = CFA_AUD_DRV_FMT_FLAC;
		break;
	default:
		eMappedAudType = CFA_AUD_DRV_FMT_UNKNOWN;
		break;
	}

	return eMappedAudType;
}

/*-----------------------------------------------------------------------------
					function prototype
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
	pvSptHdl: Provided by splitter.  When using API in splitter4cfa.h, CFA should pass this uintptr_t as the 1st parameter.
	pvPrivData: Provided by App in MPC_CMD_INIT as MPC2FFDescr.pvCfaPrivData.
	MPC passes it to splitter which passes to CFA.
-----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
 * Name: CfaAvi_Init
 *
 * Description:
 *		Init CFA AVI
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAvi_Init(void *pvSptHdl, void **ppvCfaPrivData)
{
	CfaAviInst *prCfaAviInst = NULL;


	DMX_NewMemory(sizeof(CfaAviInst), prCfaAviInst);

	if (NULL == prCfaAviInst) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA_AVI] Alloc prCfaAviInst memory fail\r\n"));
		/*OSE_SetErrCode(ERROR_DMX_NOT_ENOUGH_MEMORY);*/
		MM_RETURN(RET_DMX_NO_MEM);
	}

	dmx_memset(prCfaAviInst, 0, sizeof(CfaAviInst));

	if (prCfaAviInst->pucVc1 != NULL)
		prCfaAviInst->pucVc1 = NULL;

	prCfaAviInst->u4CurPrsFlg  = 0;
	prCfaAviInst->u8LastVPts = 0;
	prCfaAviInst->u8LastAPts = 0;

	#if CONFIG_CFA_DIVX_DRM_SUPPORT
	dmx_memset(&(prCfaAviInst->rDivxDRMInf), 0, sizeof(CFA_DIVXDRM_INFO_T));
	#endif

	/* using sync DMA, 071228 */
	prCfaAviInst->ptrMemAddress	 = DMX_INVALID_UINTPTR_T;
	#if CONFIG_CFA_DIVX_DRM_SUPPORT
	prCfaAviInst->ptrDrmMemAddress = DMX_INVALID_UINTPTR_T;
	#endif
	prCfaAviInst->eVidType = CFA_VID_MPEG4;

	/* init CFA avi internal parameters. */
	prCfaAviInst->fgBGrouped = FALSE;

	#if CONFIG_CFA_AVI_TX_ALL_AUD
	prCfaAviInst->u4CurParseAudStrmID = CFA_AVI_INVALID_STRM_ID;
	#endif
	prCfaAviInst->u4CurAudStrmID = CFA_AVI_INVALID_STRM_ID;
	prCfaAviInst->ucCurAudInfoIdx = CFA_AVI_INVALID_STRM_ID;

	#if CONFIG_CFA_AVI_TX_ALL_SP
	prCfaAviInst->u4CurParseSpStrmID = CFA_AVI_INVALID_STRM_ID;

	prCfaAviInst->u4CurSpStrmID = CFA_AVI_INVALID_STRM_ID;
	prCfaAviInst->ucCurSpInfoIdx = CFA_AVI_INVALID_STRM_ID;
	#endif

	/* current position infomation. */
	prCfaAviInst->rCfaAviCurPosiInfo.u8VidCurOfst	 = 0;
	prCfaAviInst->rCfaAviCurPosiInfo.u4VidCurChunkNo = 0;
	prCfaAviInst->rCfaAviCurPosiInfo.u8AudCurOfst	 = 0;
	prCfaAviInst->rCfaAviCurPosiInfo.u4AudCurChunkNo = 0;
	prCfaAviInst->rCfaAviCurPosiInfo.u8AudCurByte	 = 0;
	prCfaAviInst->rCfaAviCurPosiInfo.u8SubCurOfst	 = 0;
	CfaAviInitPara(prCfaAviInst);

	prCfaAviInst->u4CfaQueryType = CFA_AVI_QUERY_TYPE_NONE;
	prCfaAviInst->rParsingModeInf.uParsingMode = 0;
	prCfaAviInst->u8MinStartStrmOfst = 0;
	prCfaAviInst->fgGetVc1Case = FALSE;
	prCfaAviInst->fgNeedTxStartCode = FALSE;

	prCfaAviInst->u8SearchCnt = 0;

	#if CONFIG_CFA_AVI_NEW_METHOD_FOR_ERR_JUMP
	prCfaAviInst->u8OffsetJumpCnt = 0;
	#endif

	#if CONFIG_CFA_AVI_SUPPORT_CORRECT_CHUNK_ABNORMITY
	prCfaAviInst->fgNextChunkDetecting = FALSE;
	dmx_memset(prCfaAviInst->aucPrev4cc, 0, AVI_4CC_BYTES);
	prCfaAviInst->u8PrevCa = 0;
	prCfaAviInst->u4PrevDataSize = 0;
	#endif

	prCfaAviInst->u8PrevVidOffset = 0;

	#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
	dmx_memset(&(prCfaAviInst->rVbrGarbageInf), 0, sizeof(TCfaAviVbrGarbageInf));
	#endif

	#if CONFIG_CFA_AVI_SUPPORT_AUDIO_DRM
	dmx_memset(&(prCfaAviInst->rAudCryptInfo), 0, sizeof(CfaAviAudCryptInfo));
	#endif

	#if CONFIG_CFA_AVI_SUPPORT_FIFO_LIMITATION
	dmx_memset(&(prCfaAviInst->rFifoInfo), 0, sizeof(CfaAviFifoInfo));
	#endif

	#if CONFIG_CFA_AVI_PLAY_ONE_FRM_ONLY_TX_I_FRM
	prCfaAviInst->fgNeedFinishIfTxDone = FALSE;
	#endif

	#if CONFIG_CFA_AVI_USE_THRESHOLD_TMP
	prCfaAviInst->fgUseThreshold = FALSE;
	#endif

	prCfaAviInst->fgFillDummyAU = FALSE;
	/*add for gabagepts*/
	prCfaAviInst->fgDeteGabageData = FALSE;
	prCfaAviInst->fgDeteGabageDataEnd = FALSE;
	prCfaAviInst->u8TrueStartPts = 0;
	prCfaAviInst->u4GabageChunkNum = 0;


	/* Assign cfa function pointer	*/

	*ppvCfaPrivData = (void *) prCfaAviInst;

	_u1CFAInstanceNs++;

	if (1 == _u1CFAInstanceNs)
		prCfaAviInst->fgCurInstIsSecond = FALSE;
	else
		prCfaAviInst->fgCurInstIsSecond = TRUE;

	MMATE_CHECK_POINTER(prCfaAviInst);
	MMATE_CHECK_STRUCT(prCfaAviInst->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaAviInst->rCfaAviCurPosiInfo);
	MMATE_CHECK_STRUCT(prCfaAviInst->rVbrGarbageInf);
	MMATE_CHECK_STRUCT(prCfaAviInst->rFifoInfo);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON, TEXT("[CFA AVI] Cfa avi init OK!\r\n"));

	MM_RETURN(RET_DMX_OK);
}



/*-----------------------------------------------------------------------------
 * Name: CfaAvi_Uninit
 *
 * Description:
 *		Uninit CFA AVI
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAvi_Uninit(void *pvSptHdl, void *pvCfaPrivData)
{
	CfaAviInst *prCfaAviInst = (CfaAviInst *) pvCfaPrivData;
	void *pvPointer = NULL;
	u8 u1AudIdx = 0;

	if (NULL == prCfaAviInst) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: prCfaAviInst is null!!\r\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}
	MMATE_CHECK_POINTER(prCfaAviInst);
	MMATE_CHECK_STRUCT(prCfaAviInst->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaAviInst->rCfaAviCurPosiInfo);
	MMATE_CHECK_STRUCT(prCfaAviInst->rVbrGarbageInf);
	MMATE_CHECK_STRUCT(prCfaAviInst->rFifoInfo);

	if (NULL != prCfaAviInst->pucVc1) {
		DMX_FreeHwMemory(prCfaAviInst->pucVc1);
		prCfaAviInst->pucVc1 = NULL;
	}

	for (u1AudIdx = 0; u1AudIdx < ((u8)MAX_NS_AVI_AUD); u1AudIdx++) {
		TCfaAviAInf *prCfaAviAInf = &prCfaAviInst->rCfaAviAInf[u1AudIdx];

		if ((prCfaAviAInf->tStrmInf.rSpecInfo.pu1CodecSpecData != NULL) &&
			(prCfaAviAInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen != 0)) {
			DMX_FreeHwMemory(prCfaAviAInf->tStrmInf.rSpecInfo.pu1CodecSpecData);
			prCfaAviAInf->tStrmInf.rSpecInfo.pu1CodecSpecData = NULL;
		}
		if ((prCfaAviAInf->tStrmInf.rAccFileSpecInfo.pu1CodecAccFileSpecData != NULL) &&
			(prCfaAviAInf->tStrmInf.rAccFileSpecInfo.u4CodecAccFileSpecDataLen != 0)) {
			DMX_FreeHwMemory(prCfaAviAInf->tStrmInf.rAccFileSpecInfo.pu1CodecAccFileSpecData);
			prCfaAviAInf->tStrmInf.rAccFileSpecInfo.pu1CodecAccFileSpecData = NULL;
		}
	}
	if ((prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecInfo.pu1CodecSpecData != NULL) &&
		(prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecInfo.u4CodecSpecDataLen != 0)) {
		DMX_FreeHwMemory(prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecInfo.pu1CodecSpecData);
		prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecInfo.pu1CodecSpecData = NULL;
	}

	if ((prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecH264H265Info.pu1CodecSpecData != NULL) &&
		(prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecH264H265Info.u4CodecSpecDataLen != 0)) {
		DMX_FreeHwMemory(prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecH264H265Info.pu1CodecSpecData);
		prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecH264H265Info.pu1CodecSpecData = NULL;
	}
	if ((NULL != prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecMPEG1Info.pu1CodecSpecData) &&
		 (prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecMPEG1Info.u4CodecSpecDataLen > 0)) {
		DMX_FreeHwMemory(prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecMPEG1Info.pu1CodecSpecData);
		prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecMPEG1Info.pu1CodecSpecData = NULL;
	}

	if ((NULL != prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecMPEG4Info.pu1CodecSpecData) &&
	   (prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecMPEG4Info.u4CodecSpecDataLen > 0)) {
		DMX_FreeHwMemory(prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecMPEG4Info.pu1CodecSpecData);
		prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecMPEG4Info.u4CodecSpecDataLen = 0;
		prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecMPEG4Info.pu1CodecSpecData = NULL;
	}

	if (NULL != prCfaAviInst->rCfaAviVInf.tStrmInf.pu1AvcPayloadHdr) {
		DMX_FreeHwMemory(prCfaAviInst->rCfaAviVInf.tStrmInf.pu1AvcPayloadHdr);
		prCfaAviInst->rCfaAviVInf.tStrmInf.pu1AvcPayloadHdr = NULL;
	}

	prCfaAviInst->u8LastVPts = 0;
	prCfaAviInst->u8LastAPts = 0;

	pvPointer = (void *)prCfaAviInst;

	if (pvPointer != NULL)
		DMX_FreeMemory(pvPointer);

	_u1CFAInstanceNs--;

	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON, TEXT("[CFA AVI] Cfa avi Uninit OK!\r\n"));
	MM_RETURN(RET_DMX_OK);
}

#ifdef CONFIG_COMPAT
#include <linux/compat.h>

#if CONFIG_CFA_AVI_MULTIPLE_AUDIO_INFO
typedef struct {
	bool fgIsValid;
	__u64 u8AudStartOfst;	/* file offset of u4AudStartChunkNo */
	__u32 u4AudStartChunkNo;	/* chunk number of the starting chunk */
	__u64 u8AudStartByte;	/* accumulated number of bytes of the starting chunk */
	__u32 u4AudSkipChunkNs;	/* number of chunks to be skipped by Demuxer */
	__u32 u4AudSkipByte;	/* number of bytes to be skipped by Demuxer */
	__u32 u4AudEndChunkNo;	/* ending audio chunk number */
	__u64 u8AudEndByte;	/* ending accumulated number of bytes */
} CfaAviAudioRange32;
#endif

/* Must be compatible with MPC2AviPR */
typedef struct {
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKStart;
#endif
	ECfaRangeType eRangeType;

	bool fgReset;	/* set TRUE except in FUN_PTR_MPC2_GET_PR_CB pfvGetPR. */
	__u32 u4PrsFlag;	/*< current parsing stream flag */
	bool fgPlayFrmFlag;	/*To check if playing a Frame */

	__u64 u8IdxlOffset;	/* index start address */
	__u32 u4IdxlSz;	/* index size */


	/* video */
	__u64 u8VidStartOfst;	/* file offset of u4VidStartChunkNo */
	__u32 u4VidStartChunkNo;	/* chunk number of the starting chunk */
	__u32 u4VidSkipChunkNs;	/* number of chunks to be skipped by Demuxer */
	__u32 u4VidEndChunkNo;	/* ending video chunk number */

	/* audio */
#if CONFIG_CFA_AVI_MULTIPLE_AUDIO_INFO
	CfaAviAudioRange32 rAudioRange[MAX_NS_AVI_AUD];
#endif

	__u64 u8AudStartPts;

	__u64 u8StartPts;

	/* Yi Feng added to fix BDP00224825 @2009/04/29 */
	__u64 u8EndPts;
#ifdef MM_SUPPORT_DIVXHT31
	__u64 u8FFRangeEndPts;
#endif
	bool fgCompareSubPtsFlag;	/* The flag to compare subtitle pts with end pts */

	__u64 u8AudStartOfst;	/* file offset of u4AudStartChunkNo */
	__u32 u4AudStartChunkNo;	/* chunk number of the starting chunk */
	__u64 u8AudStartByte;	/* accumulated number of bytes of the starting chunk */
	__u32 u4AudSkipChunkNs;	/* number of chunks to be skipped by Demuxer */
	__u32 u4AudSkipByte;	/* number of bytes to be skipped by Demuxer */
	__u32 u4AudEndChunkNo;	/* ending audio chunk number */
	__u64 u8AudEndByte;	/* ending accumulated number of bytes */

	/* subpicture */
	__u64 u8SubStartOfst;	/* file offset of starting subtitle chunk */
	__u32 u4SubStartChunkNo;	/* chunk number of the starting chunk */
	__u32 u4SubSkipChunkNs;	/* number of chunks to be skipped by Demuuxer */
	__u32 u4SubEndChunkNo;	/* ending subpicture chunk number      */
	__u64 u8SubEndOfst;	/* ending offset of subtitle */

	/* end of stream */
	__u64 u8Endoffst;
	/*< current file size in bytes ,for Checking if a transfer is within IO read session range */
	__u64 u8FileSz;
	__u64 u8SeekPts;
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKEnd;
#endif
}CfaAviRange32;

typedef struct {
	/* Information of Current Key Frame */
	__u64 u8StartPts;	/* The Start Pts of the Key Frame */

	__u64 u8VidStartOfst;	/* file offset of u4VidStartChunkNo */
	__u32 u4VidStartChunkNo;	/* chunk number of the starting chunk */
	__u32 u4CurKeyFrmeMaxSZ;	/* The max size of current key Frame	*/

	/* End Information of File, other than current key Frame */
	__u64 u8EndPts; /* The Pts of File End, other than end of the Key Frame */
	__u64 u8Endoffst;
	__u64 u8FileSz;
	__u32 u4VidEndChunkNoInFile;	/* Ending video chunk number in the File */
	__u32 u4SpDataSize;

	CfaAviRange32 rCfaRangeInfo;
} CfaAviKeyFrameRange32;

typedef struct {
	AVCODECID_T eAudCodec;	/* from 'strf' for audio */
	ECfaAviStrmType eAudStrmType;
	__u32 u4AudStrmIdx;	/* e.g.: 00dc: u4StrmIdx = 0, 01wb: u4StrmIdx = 1 */
	__u32 u4AudScale;	/* from "strh" */
	__u32 u4AudRate;	/* from "strh" */
	__u32 u4AudBps;	/* from "strf", byte per second; only used in CBRA */
	__u16 u2AudBlockAlign;	/* from "strf" */
#if 1				/* mtk40093 add for 227045 @2009/09/04 */
	__u16 u2AudBitsPerSample;
#endif
	__u32 u4SampleRate;	/* for vorbis audio */
	__u16 u2Channel;	/* for vorbis audio */
	__u16 u2AudCodecID;
	__u16 u2EncOptions;
	/*< memory address got from Pfr, using sync DMA, 071228 */
	compat_caddr_t pucAudCodecSpecData;
	__u32 u4AudCodecSpecDataLen;
} CfaAviAudInfo32;

typedef struct {
	__u32 u4GarbageDataChkNum;
	__u64 u8GarbageDataEndOffset;
	__u64 u8GarbageDataDuration;
} CfaAviVbrMp3GarbageInfo32;

#if AVI_SUPPOTR_DRM
typedef struct rDrmInf {
	bool fgHasDrm;
	bool fgAdjustDDChunkRang;
	__u32 u4DrmVersion;
	__u32 u4DrmHdrSz;
	__u64 u8DrmOffset;
	compat_caddr_t uDrmData;
} PB_RIFF_DRM_INF_T32;
#endif

#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
	typedef struct {
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKStart;
#endif

	__u64 u8GarbageDataEndPtsByCbr;
	__u64 u8GarbageDataEndPtsByVbr;
	__u32 u4GarbageBytes;
	__u32 u4GarbageChunks;

#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKEnd;
#endif
} TCfaAviVbrGarbageInf32;
#endif

typedef struct {
	/* V info */
	ECfaAviStrmType eVidStrmType;
	__u32 u4VidStrmIdx;	/* e.g.: 00dc: u4StrmIdx = 0, 01wb: u4StrmIdx = 1 */
	__u32 u4VidScale;	/* from "strh" */
	__u32 u4VidRate;	/* from "strh" */
	/* from "strf" eg. "DX50" => CFA_AVI_V_CODEC_DX5_M4V */
	AVCODECID_T eVidCodec;

	__u64 u8FirstVideoFrameOft;

	/* A info */
	__u32 u4MaxAudChunkDuration;
	CfaAviAudInfo32 rCfaAviAudInfo[MAX_NS_AVI_AUD];

	/* subtitle info */
#if CONFIG_CFA_AVI_TX_ALL_SP
	CfaAviSpInfo rCfaAviSpInfo[MAX_NS_AVI_INTERNAL_SP];
#else
	/* e.g.: 00dc: u4StrmIdx = 0, 01wb: u4StrmIdx = 1, 02sb: u4StrmIdx = 2 */
	__u32 u4SpStrmIdx;
#endif

	__u64 u8VidCodecSpecDataOfst;
	__u32 u4VidCodecSpecDataLen;
	compat_caddr_t pu1VidCodecSpecData;

	__u64 u8VidMPEG1CodecSpecDataOfst;
	__u32 u4VidMPEG1CodecSpecDataLen;
	compat_caddr_t pu1VidMPEG1CodecSpecData;

	__u64 u8VidMPEG4VolDataOffset;
	__u32 u4VidMPEG4VolDataLen;
	compat_caddr_t pu1VidMPEG4VolData;

	compat_caddr_t pu1VidPPSSPSHeaderData;
	__u32 u4VidPPSSPSHeaderDataLen;
	__u8 u1AvcPayloadLenFieldSz;

#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
	TCfaAviVbrGarbageInf32 rVbrGarbageInf;
#endif

#if CONFIG_CFA_AVI_NEW_METHOD_FOR_ERR_JUMP
	bool fgIsFileHasIndex;
#endif

#if CONFIG_CFA_AVI_SUPPORT_AUDIO_DRM
	CfaAviAudCryptInfo rAudCryptInfo;
#endif

#if CONFIG_CFA_AVI_NEW_SYNC_PB_BUF_FLOW
	bool fgTooHighFrameRate;
#endif

#if CONFIG_CFA_AVI_SUPPORT_FIFO_LIMITATION
	CfaAviFifoInfo rFifoInfo;
#endif

	CfaAviVbrMp3GarbageInfo32 rVbrMp3GarbageInfo;

#if CONFIG_CFA_AVI_USE_THRESHOLD_TMP
	bool fgUseThreshold;
#endif

#if AVI_SUPPOTR_DRM
	PB_RIFF_DRM_INF_T32 rDrmInfo;
#endif
} CfaAviConfigInfo32;

static long CfaAviCompatAudRangeInfo(CfaAviAudioRange __user *usr_ptr,
  CfaAviAudioRange32 __user *usr_ptr32)
{
	if (copy_in_user(&(usr_ptr->fgIsValid), &(usr_ptr32->fgIsValid), sizeof(bool)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8AudStartOfst), &(usr_ptr32->u8AudStartOfst), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u4AudStartChunkNo), &(usr_ptr32->u4AudStartChunkNo), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8AudStartByte), &(usr_ptr32->u8AudStartByte), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4AudSkipChunkNs), &(usr_ptr32->u4AudSkipChunkNs), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4AudSkipByte), &(usr_ptr32->u4AudSkipByte), sizeof(__u32)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u4AudEndChunkNo), &(usr_ptr32->u4AudEndChunkNo), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8AudEndByte), &(usr_ptr32->u8AudEndByte), sizeof(__u64)))
		return -EFAULT;
	return 0;
}

static long CfaAviCompatRangeInfo(CfaAviRange __user *usr_ptr,
  CfaAviRange32 __user *usr_ptr32)
{
	long ret = 0;
	int i = 0;
#ifdef MM_ATE_CHECK
	if (copy_in_user(&(usr_ptr->u4MMATECHKStart), &(usr_ptr32->u4MMATECHKStart), sizeof(__u32)))
		return -EFAULT;
#endif
	if (copy_in_user(&(usr_ptr->eRangeType), &(usr_ptr32->eRangeType), sizeof(ECfaRangeType)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->fgReset), &(usr_ptr32->fgReset), sizeof(bool)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u4PrsFlag), &(usr_ptr32->u4PrsFlag), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->fgPlayFrmFlag), &(usr_ptr32->fgPlayFrmFlag), sizeof(bool)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8IdxlOffset), &(usr_ptr32->u8IdxlOffset),
		sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4IdxlSz), &(usr_ptr32->u4IdxlSz), sizeof(__u32)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8VidStartOfst), &(usr_ptr32->u8VidStartOfst), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4VidStartChunkNo), &(usr_ptr32->u4VidStartChunkNo), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4VidSkipChunkNs), &(usr_ptr32->u4VidSkipChunkNs), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4VidEndChunkNo), &(usr_ptr32->u4VidEndChunkNo), sizeof(__u32)))
		return -EFAULT;
#if CONFIG_CFA_AVI_MULTIPLE_AUDIO_INFO
	for (i = 0; i < MAX_NS_AVI_AUD; i++)
	{
		ret = CfaAviCompatAudRangeInfo((usr_ptr->rAudioRange + i),
		(usr_ptr32->rAudioRange + i));

		if (0 != ret) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			  TEXT("%s line %d fail in CfaAviCompatAudRangeInfo.\r\n"),
			  DMX_FUNC_NAME, DMX_LINE_NO);
			return ret;
		}
	}
#endif
	if (copy_in_user(&(usr_ptr->u8AudStartPts), &(usr_ptr32->u8AudStartPts), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8StartPts), &(usr_ptr32->u8StartPts), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8EndPts), &(usr_ptr32->u8EndPts), sizeof(__u64)))
		return -EFAULT;
#ifdef MM_SUPPORT_DIVXHT31
	if (copy_in_user(&(usr_ptr->u8FFRangeEndPts), &(usr_ptr32->u8FFRangeEndPts), sizeof(__u64)))
		return -EFAULT;
#endif
	if (copy_in_user(&(usr_ptr->fgCompareSubPtsFlag), &(usr_ptr32->fgCompareSubPtsFlag), sizeof(bool)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8AudStartOfst), &(usr_ptr32->u8AudStartOfst), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u4AudStartChunkNo), &(usr_ptr32->u4AudStartChunkNo), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8AudStartByte), &(usr_ptr32->u8AudStartByte), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4AudSkipChunkNs), &(usr_ptr32->u4AudSkipChunkNs), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4AudSkipByte), &(usr_ptr32->u4AudSkipByte), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4AudEndChunkNo), &(usr_ptr32->u4AudEndChunkNo), sizeof(__u32)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8AudEndByte), &(usr_ptr32->u8AudEndByte), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8SubStartOfst), &(usr_ptr32->u8SubStartOfst), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4SubStartChunkNo), &(usr_ptr32->u4SubStartChunkNo), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4SubSkipChunkNs), &(usr_ptr32->u4SubSkipChunkNs), sizeof(__u32)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u4SubEndChunkNo), &(usr_ptr32->u4SubEndChunkNo), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8SubEndOfst), &(usr_ptr32->u8SubEndOfst), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8Endoffst), &(usr_ptr32->u8Endoffst), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8FileSz), &(usr_ptr32->u8FileSz), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8SeekPts), &(usr_ptr32->u8SeekPts), sizeof(__u64)))
		return -EFAULT;

#ifdef MM_ATE_CHECK
	if (copy_in_user(&(usr_ptr->u4MMATECHKEnd), &(usr_ptr32->u4MMATECHKEnd), sizeof(__u32)))
		return -EFAULT;
#endif
	return 0;
}

static long CfaAviCompatRange(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaAviRange __user *usr_ptr = NULL;
	CfaAviRange32 __user *usr_ptr32 = (CfaAviRange32 __user *)prInfo->usr_ptr32;
	long ret = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaAviRange32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	usr_ptr = (CfaAviRange *)compat_alloc_user_space(sizeof(CfaAviRange));

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("%s line %d fail in alloc compat user space.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaAviRange));
	ret = CfaAviCompatRangeInfo(usr_ptr, usr_ptr32);
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("%s line %d fail in CfaAviCompatRangeInfo.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}

	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaAviRange);

	return 0;
}

static long CfaAviCompatJumpRange(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaAviKeyFrameRange __user *usr_ptr = NULL;
	CfaAviKeyFrameRange32 __user *usr_ptr32 = (CfaAviKeyFrameRange32 __user *)prInfo->usr_ptr32;
	long ret = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaAviKeyFrameRange32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	usr_ptr = (CfaAviKeyFrameRange *)compat_alloc_user_space(sizeof(CfaAviKeyFrameRange));

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("%s line %d fail in alloc compat user space.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaAviKeyFrameRange));

	if (copy_in_user(&(usr_ptr->u8StartPts), &(usr_ptr32->u8StartPts), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8VidStartOfst), &(usr_ptr32->u8VidStartOfst), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4VidStartChunkNo), &(usr_ptr32->u4VidStartChunkNo), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4CurKeyFrmeMaxSZ), &(usr_ptr32->u4CurKeyFrmeMaxSZ), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8EndPts), &(usr_ptr32->u8EndPts), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8Endoffst), &(usr_ptr32->u8Endoffst), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8FileSz), &(usr_ptr32->u8FileSz), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4VidEndChunkNoInFile), &(usr_ptr32->u4VidEndChunkNoInFile), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4SpDataSize), &(usr_ptr32->u4SpDataSize), sizeof(__u32)))
		return -EFAULT;

	ret = CfaAviCompatRangeInfo(&(usr_ptr->rCfaRangeInfo),&(usr_ptr32->rCfaRangeInfo));
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("%s line %d fail in CfaAviCompatRangeInfo.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}

	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaAviKeyFrameRange);

	return 0;
}

static long CfaAviCompatAudInfo(CfaAviAudInfo __user *usr_ptr,
	  CfaAviAudInfo32 __user *usr_ptr32, __u8 **pu1NextUserBuf, __u32 *pu4Sz, __u32 u4TotalSz)
{
	compat_caddr_t compatSeqHdr = 0;
	if (copy_from_user(&(usr_ptr->eAudCodec), &(usr_ptr32->eAudCodec), sizeof(AVCODECID_T)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->eAudStrmType), &(usr_ptr32->eAudStrmType), sizeof(ECfaAviStrmType)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4AudStrmIdx), &(usr_ptr32->u4AudStrmIdx), sizeof(__u32)))
		return -EFAULT;

	if (copy_from_user(&(usr_ptr->u4AudScale), &(usr_ptr32->u4AudScale), sizeof(__u32)))
		return -EFAULT;

	if (copy_from_user(&(usr_ptr->u4AudRate), &(usr_ptr32->u4AudRate), sizeof(__u32)))
		return -EFAULT;

	if (copy_from_user(&(usr_ptr->u4AudBps), &(usr_ptr32->u4AudBps), sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u2AudBlockAlign), &(usr_ptr32->u2AudBlockAlign), sizeof(__u16)))
		return -EFAULT;

	if (copy_from_user(&(usr_ptr->u2AudBitsPerSample), &(usr_ptr32->u2AudBitsPerSample), sizeof(__u16)))
		return -EFAULT;

	if (copy_from_user(&(usr_ptr->u4SampleRate), &(usr_ptr32->u4SampleRate), sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u2Channel), &(usr_ptr32->u2Channel), sizeof(__u16)))
		return -EFAULT;

	if (copy_from_user(&(usr_ptr->u2AudCodecID), &(usr_ptr32->u2AudCodecID), sizeof(__u16)))
		return -EFAULT;

	if (copy_from_user(&(usr_ptr->u2EncOptions), &(usr_ptr32->u2EncOptions), sizeof(__u16)))
		return -EFAULT;

	if (copy_from_user(&(usr_ptr->u4AudCodecSpecDataLen), &(usr_ptr32->u4AudCodecSpecDataLen), sizeof(__u32)))
		return -EFAULT;

	if ((0 < usr_ptr32->u4AudCodecSpecDataLen) && (0 == usr_ptr32->pucAudCodecSpecData)) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("%s line %d fail for no aud header, but header len(%d) > 0.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr->u4AudCodecSpecDataLen);
		return -EINVAL;
	}

	if (0 != usr_ptr32->pucAudCodecSpecData) {
		__u8 *pucAudCodecSpecData = NULL;

		usr_ptr->pucAudCodecSpecData =  *pu1NextUserBuf;

		if (NULL == usr_ptr->pucAudCodecSpecData) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in alloc compat user space.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return -ENOMEM;
		}
		mm_memset(usr_ptr->pucAudCodecSpecData, 0, sizeof(__u8) * usr_ptr->u4AudCodecSpecDataLen);
		*pu1NextUserBuf += CFA_ALIGN_SZ((__u32)(sizeof(__u8) * usr_ptr->u4AudCodecSpecDataLen), sizeof(uintptr_t));
		*pu4Sz += CFA_ALIGN_SZ((__u32)(sizeof(__u8) * usr_ptr->u4AudCodecSpecDataLen), sizeof(uintptr_t));
		if (*pu4Sz > u4TotalSz)
		{
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d size is large than total size.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
			return -ENOMEM;
		}
		if (get_user(compatSeqHdr, &(usr_ptr32->pucAudCodecSpecData)))
			return -EFAULT;
		if (0 == compatSeqHdr)
			return -EFAULT;
		pucAudCodecSpecData = compat_ptr(compatSeqHdr);
		if (!access_ok(VERIFY_READ, pucAudCodecSpecData,
			sizeof(__u8) * usr_ptr->u4AudCodecSpecDataLen))
			return -EFAULT;

		if (copy_from_user((__u8 __user *)usr_ptr->pucAudCodecSpecData,
			 pucAudCodecSpecData, sizeof(__u8) * usr_ptr->u4AudCodecSpecDataLen))
			return -EFAULT;
	}
	return 0;
}
#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
static long CfaAviCompatVbrGarbageInfInfo(TCfaAviVbrGarbageInf __user *usr_ptr,
  TCfaAviVbrGarbageInf32 __user *usr_ptr32)
{
#ifdef MM_ATE_CHECK
	if (copy_from_user(&(usr_ptr->u4MMATECHKStart),
		&(usr_ptr32->u4MMATECHKStart), sizeof(__u32)))
		return -EFAULT;
#endif
	if (copy_from_user(&(usr_ptr->u8GarbageDataEndPtsByCbr),
		&(usr_ptr32->u8GarbageDataEndPtsByCbr), sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8GarbageDataEndPtsByVbr),
		&(usr_ptr32->u8GarbageDataEndPtsByVbr), sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4GarbageBytes),
		&(usr_ptr32->u4GarbageBytes), sizeof(__u32)))
		return -EFAULT;

	if (copy_from_user(&(usr_ptr->u4GarbageChunks),
		&(usr_ptr32->u4GarbageChunks), sizeof(__u32)))
		return -EFAULT;

#ifdef MM_ATE_CHECK
	if (copy_from_user(&(usr_ptr->u4MMATECHKEnd),
		&(usr_ptr32->u4MMATECHKEnd), sizeof(__u32)))
		return -EFAULT;
#endif

	return 0;
}
#endif

static long CfaAviCompatVbrMp3GarbageInfInfo(CfaAviVbrMp3GarbageInfo __user *usr_ptr,
  CfaAviVbrMp3GarbageInfo32 __user *usr_ptr32)
{
	if (copy_from_user(&(usr_ptr->u4GarbageDataChkNum),
		&(usr_ptr32->u4GarbageDataChkNum), sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8GarbageDataEndOffset),
		&(usr_ptr32->u8GarbageDataEndOffset), sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8GarbageDataDuration),
		&(usr_ptr32->u8GarbageDataDuration), sizeof(__u32)))
		return -EFAULT;

	return 0;
}

#if AVI_SUPPOTR_DRM
static long CfaAviCompatDrmInfInfo(PB_RIFF_DRM_INF_T __user *usr_ptr,
  PB_RIFF_DRM_INF_T32 __user *usr_ptr32,
  __u8 __user **pu1NextUsrBufAddr,
  __u32 *pu4Sz, __u32 u4TotalSz)
{
	compat_caddr_t compatSeqHdr = 0;
	if (copy_from_user(&(usr_ptr->fgHasDrm),
		&(usr_ptr32->fgHasDrm), sizeof(bool)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->fgAdjustDDChunkRang),
		&(usr_ptr32->fgAdjustDDChunkRang), sizeof(bool)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4DrmVersion),
		&(usr_ptr32->u4DrmVersion), sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4DrmHdrSz),
		&(usr_ptr32->u4DrmHdrSz), sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8DrmOffset),
		&(usr_ptr32->u8DrmOffset), sizeof(__u64)))
		return -EFAULT;
	if ((0 < usr_ptr32->u4DrmHdrSz) && (0 == usr_ptr32->uDrmData)) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("%s line %d fail for no drm header, but header len(%d) > 0.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr->u4DrmHdrSz);
		return -EINVAL;
	}

	if (0 != usr_ptr32->uDrmData) {
		__u8 *uDrmData = NULL;

		usr_ptr->uDrmData = *pu1NextUsrBufAddr;

		if (NULL == usr_ptr->uDrmData) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in alloc compat user space.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return -ENOMEM;
		}
		mm_memset(usr_ptr->uDrmData, 0, sizeof(__u8) * usr_ptr->u4DrmHdrSz);
		*pu1NextUsrBufAddr += CFA_ALIGN_SZ((__u32) usr_ptr->u4DrmHdrSz, sizeof(uintptr_t));
		*pu4Sz += CFA_ALIGN_SZ((__u32) usr_ptr->u4DrmHdrSz, sizeof(uintptr_t));
		if (*pu4Sz > u4TotalSz)
		{
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d size is large than total size.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
			return -ENOMEM;
		}

		if (get_user(compatSeqHdr, &(usr_ptr32->uDrmData)))
			return -EFAULT;
		if (0 == compatSeqHdr)
			return -EFAULT;
		uDrmData = compat_ptr(compatSeqHdr);
		if (!access_ok(VERIFY_READ, uDrmData,
			usr_ptr->u4DrmHdrSz))
			return -EFAULT;

		if (copy_from_user((__u8 __user *)usr_ptr->uDrmData,
			 uDrmData, sizeof(__u8) * usr_ptr->u4DrmHdrSz))
			return -EFAULT;
	}

	return 0;
}
#endif /* AVI_SUPPOTR_DRM */

static long CfaAviCompatConfigCalcSz(CfaAviConfigInfo32 __user *usr_ptr32,
	__u32 *pu4OutSz)
{
	__u32 u4TotalSz = 0;
	__u32 u4HeaderLen = 0;
	__u32 i = 0;
	long ret = 0;

	if ((NULL == usr_ptr32) || (NULL == pu4OutSz)) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	u4TotalSz += CFA_ALIGN_SZ((__u32)sizeof(CfaAviConfigInfo), sizeof(uintptr_t));
	for (i = 0; i < MAX_NS_AVI_AUD; i++)
	{
		ret = get_user(u4HeaderLen, &(usr_ptr32->rCfaAviAudInfo[i].u4AudCodecSpecDataLen));
		if (ret != 0) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in get_user(rCfaAviAudInfo[%d].u4AudCodecSpecDataLen).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, i);
			return -EFAULT;
		}
		u4TotalSz += CFA_ALIGN_SZ(u4HeaderLen, sizeof(uintptr_t));
	}
	
	ret = get_user(u4HeaderLen, &(usr_ptr32->u4VidCodecSpecDataLen));
	if (ret != 0) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("%s line %d fail in get_user(u4VidCodecSpecDataLen).\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, i);
		return -EFAULT;
	}
	u4TotalSz += CFA_ALIGN_SZ(u4HeaderLen, sizeof(uintptr_t));
	ret = get_user(u4HeaderLen, &(usr_ptr32->u4VidMPEG1CodecSpecDataLen));
	if (ret != 0) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("%s line %d fail in get_user(u4VidMPEG1CodecSpecDataLen).\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, i);
		return -EFAULT;
	}
	u4TotalSz += CFA_ALIGN_SZ(u4HeaderLen, sizeof(uintptr_t));
	ret = get_user(u4HeaderLen, &(usr_ptr32->u4VidMPEG4VolDataLen));
	if (ret != 0) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("%s line %d fail in get_user(u4VidMPEG4VolDataLen).\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, i);
		return -EFAULT;
	}
	u4TotalSz += CFA_ALIGN_SZ(u4HeaderLen, sizeof(uintptr_t));
	ret = get_user(u4HeaderLen, &(usr_ptr32->u4VidPPSSPSHeaderDataLen));
	if (ret != 0) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("%s line %d fail in get_user(u4VidPPSSPSHeaderDataLen).\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, i);
		return -EFAULT;
	}
	u4TotalSz += CFA_ALIGN_SZ(u4HeaderLen, sizeof(uintptr_t));
	
#if AVI_SUPPOTR_DRM
	ret = get_user(u4HeaderLen, &(usr_ptr32->rDrmInfo.u4DrmHdrSz));
	if (ret != 0) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("%s line %d fail in get_user(rDrmInfo.u4DrmHdrSz).\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, i);
		return -EFAULT;
	}
	u4TotalSz += CFA_ALIGN_SZ(u4HeaderLen, sizeof(uintptr_t));
#endif

	*pu4OutSz = u4TotalSz;

	return 0;
}


static long CfaAviCompatConfig(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaAviConfigInfo __user *usr_ptr = NULL;
	CfaAviConfigInfo32 __user *usr_ptr32 = (CfaAviConfigInfo32 __user *)prInfo->usr_ptr32;
	long ret = 0;
	int i = 0;
	compat_caddr_t compatSeqHdr = 0;
	__u8 __user *pu1UsrBufAddr = NULL;
	__u8 __user *pu1NextUsrBufAddr = NULL;
	__u32 u4TotalSz = 0;
	__u32 u4Sz = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaAviConfigInfo32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz(0x%08x/0x%08x).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prInfo->buf_sz, sizeof(CfaAviConfigInfo32));
		return -EINVAL;
	}

	if (0 != CfaAviCompatConfigCalcSz(usr_ptr32, &u4TotalSz)) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaAviCompatConfigCalcSz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	//pu1UsrBufAddr = (__u8 __user *)compat_alloc_user_space(u4TotalSz);
	//size > 8 K may cause fail
	DMX_NewMemory(u4TotalSz, pu1UsrBufAddr);
	*pfgIsUserMem = FALSE;
	if (NULL == pu1UsrBufAddr) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(pu1UsrBufAddr, 0, u4TotalSz);

	usr_ptr = (CfaAviConfigInfo __user *)pu1UsrBufAddr;

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -ENOMEM;
	}

	mm_memset(usr_ptr, 0, sizeof(CfaAviConfigInfo));

	pu1NextUsrBufAddr = pu1UsrBufAddr + CFA_ALIGN_SZ(sizeof(CfaAviConfigInfo), sizeof(uintptr_t));
	u4Sz += CFA_ALIGN_SZ(sizeof(CfaAviConfigInfo), sizeof(uintptr_t));
	if (u4Sz > u4TotalSz)
	{
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("%s line %d size is large than total size.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -ENOMEM;
	}
	if (copy_from_user(&(usr_ptr->eVidStrmType),
		&(usr_ptr32->eVidStrmType),
		sizeof(ECfaAviStrmType))) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(eVidStrmType).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}
	if (copy_from_user(&(usr_ptr->u4VidStrmIdx),
		&(usr_ptr32->u4VidStrmIdx),
		sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4VidStrmIdx).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}

	if (copy_from_user(&(usr_ptr->u4VidScale),
		&(usr_ptr32->u4VidScale),
		sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4VidScale).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}

	if (copy_from_user(&(usr_ptr->u4VidRate),
		&(usr_ptr32->u4VidRate),
		sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4VidRate).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}
	if (copy_from_user(&(usr_ptr->eVidCodec),
		&(usr_ptr32->eVidCodec),
		sizeof(AVCODECID_T))){
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(eVidCodec).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}

	if (copy_from_user(&(usr_ptr->u8FirstVideoFrameOft),
		&(usr_ptr32->u8FirstVideoFrameOft),
		sizeof(__u64))){
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u8FirstVideoFrameOft).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}

	if (copy_from_user(&(usr_ptr->u4MaxAudChunkDuration),
		&(usr_ptr32->u4MaxAudChunkDuration),
		sizeof(__u32))){
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4MaxAudChunkDuration).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}
	for (i = 0; i < MAX_NS_AVI_AUD; i++)
	{
		ret =  CfaAviCompatAudInfo((usr_ptr->rCfaAviAudInfo + i),
			(usr_ptr32->rCfaAviAudInfo + i), &pu1NextUsrBufAddr, &u4Sz, u4TotalSz);
		if (ret != 0)
		{
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaAviCompatAudInfo.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}
	}
#if CONFIG_CFA_AVI_TX_ALL_SP
	for (i = 0; i < MAX_NS_AVI_INTERNAL_SP; i++)
	{
		if (copy_from_user((usr_ptr->rCfaAviSpInfo + i),
			(usr_ptr32->rCfaAviSpInfo + i),
			sizeof(CfaAviSpInfo))){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(rCfaAviSpInfo).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}
	}
#else
	if (copy_from_user(&(usr_ptr->u4SpStrmIdx),
		&(usr_ptr32->u4SpStrmIdx),
		sizeof(__u32))){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(u4SpStrmIdx).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}
#endif
	if (copy_from_user(&(usr_ptr->u8VidCodecSpecDataOfst),
		&(usr_ptr32->u8VidCodecSpecDataOfst),
		sizeof(__u64))){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(u8VidCodecSpecDataOfst).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}

	if (copy_from_user(&(usr_ptr->u4VidCodecSpecDataLen),
		&(usr_ptr32->u4VidCodecSpecDataLen),
		sizeof(__u32))){
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4VidCodecSpecDataLen).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}
	
	DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("%s line %d  -- u4VidCodecSpecDataLen: 0x%08x.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr32->u4VidCodecSpecDataLen);

	if ((0 < usr_ptr32->u4VidCodecSpecDataLen) && (0 == usr_ptr32->pu1VidCodecSpecData)) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("%s line %d fail for no aac header, but header len(%d) > 0.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr->u4VidCodecSpecDataLen);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}

	if (0 != usr_ptr32->pu1VidCodecSpecData) {
		void *pu1VidCodecSpecData = NULL;

		usr_ptr->pu1VidCodecSpecData = pu1NextUsrBufAddr;

		if (NULL == usr_ptr->pu1VidCodecSpecData) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in alloc compat user space.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -ENOMEM;
		}
		mm_memset(usr_ptr->pu1VidCodecSpecData, 0, usr_ptr->u4VidCodecSpecDataLen);
		pu1NextUsrBufAddr += CFA_ALIGN_SZ(usr_ptr->u4VidCodecSpecDataLen, sizeof(uintptr_t));
		u4Sz += CFA_ALIGN_SZ(usr_ptr->u4VidCodecSpecDataLen, sizeof(uintptr_t));
		if (u4Sz > u4TotalSz)
		{
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d size is large than total size.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -ENOMEM;
		}
		if (get_user(compatSeqHdr, &(usr_ptr32->pu1VidCodecSpecData))){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(pu1VidCodecSpecData).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}
		if (0 == compatSeqHdr){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(compatSeqHdr = 0).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}

		pu1VidCodecSpecData = compat_ptr(compatSeqHdr);
		if (!access_ok(VERIFY_READ, pu1VidCodecSpecData,
			usr_ptr->u4VidCodecSpecDataLen)){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in access_ok(pu1VidCodecSpecData).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}

		if (copy_from_user((void __user *)usr_ptr->pu1VidCodecSpecData,
			 pu1VidCodecSpecData, usr_ptr->u4VidCodecSpecDataLen)){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(pu1VidCodecSpecData: 0x%p).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pu1VidCodecSpecData);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}
	}

	if (copy_from_user(&(usr_ptr->u8VidMPEG1CodecSpecDataOfst),
		&(usr_ptr32->u8VidMPEG1CodecSpecDataOfst),
		sizeof(__u64))){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(u8VidMPEG1CodecSpecDataOfst).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}

	if (copy_from_user(&(usr_ptr->u4VidMPEG1CodecSpecDataLen),
		&(usr_ptr32->u4VidMPEG1CodecSpecDataLen),
		sizeof(__u32))){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(u4VidMPEG1CodecSpecDataLen).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}

	if ((0 < usr_ptr32->u4VidMPEG1CodecSpecDataLen) && (0 == usr_ptr32->pu1VidMPEG1CodecSpecData)) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("%s line %d fail for no mpeg1 header, but header len(%d) > 0.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr->u4VidMPEG1CodecSpecDataLen);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}

	if (0 != usr_ptr32->pu1VidMPEG1CodecSpecData) {
		void *pu1VidMPEG1CodecSpecData = NULL;

		usr_ptr->pu1VidMPEG1CodecSpecData = pu1NextUsrBufAddr;

		if (NULL == usr_ptr->pu1VidMPEG1CodecSpecData) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in alloc compat user space.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -ENOMEM;
		}
		mm_memset(usr_ptr->pu1VidMPEG1CodecSpecData, 0, usr_ptr->u4VidMPEG1CodecSpecDataLen);
		pu1NextUsrBufAddr += CFA_ALIGN_SZ(usr_ptr->u4VidMPEG1CodecSpecDataLen, sizeof(uintptr_t));
		u4Sz += CFA_ALIGN_SZ(usr_ptr->u4VidMPEG1CodecSpecDataLen, sizeof(uintptr_t));
		if (u4Sz > u4TotalSz)
		{
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d size is large than total size.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -ENOMEM;
		}
		if (get_user(compatSeqHdr, &(usr_ptr32->pu1VidMPEG1CodecSpecData))){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(pu1VidMPEG1CodecSpecData).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return -EINVAL;
		}
		if (0 == compatSeqHdr){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(compatSeqHdr = 0).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}
		pu1VidMPEG1CodecSpecData = compat_ptr(compatSeqHdr);
		if (!access_ok(VERIFY_READ, pu1VidMPEG1CodecSpecData,
			usr_ptr->u4VidMPEG1CodecSpecDataLen)){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in access_ok(pu1VidMPEG1CodecSpecData: 0x%p).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pu1VidMPEG1CodecSpecData);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}

		if (copy_from_user((void __user *)usr_ptr->pu1VidMPEG1CodecSpecData,
			 pu1VidMPEG1CodecSpecData, usr_ptr->u4VidMPEG1CodecSpecDataLen)){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(pu1VidMPEG1CodecSpecData).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}
	}

	if (copy_from_user(&(usr_ptr->u8VidMPEG4VolDataOffset),
		&(usr_ptr32->u8VidMPEG4VolDataOffset),
		sizeof(__u64))){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(u8VidMPEG4VolDataOffset).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}

	if (copy_from_user(&(usr_ptr->u4VidMPEG4VolDataLen),
		&(usr_ptr32->u4VidMPEG4VolDataLen),
		sizeof(__u32))){
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4VidMPEG4VolDataLen).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}

	if ((0 < usr_ptr32->u4VidMPEG4VolDataLen) && (0 == usr_ptr32->pu1VidMPEG4VolData)) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("%s line %d fail for no mpeg4 header, but header len(%d) > 0.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr->u4VidMPEG4VolDataLen);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}

	if (0 != usr_ptr32->pu1VidMPEG4VolData) {
		void *pu1VidMPEG4VolData = NULL;

		usr_ptr->pu1VidMPEG4VolData = pu1NextUsrBufAddr;

		if (NULL == usr_ptr->pu1VidMPEG4VolData) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in alloc compat user space.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -ENOMEM;
		}
		mm_memset(usr_ptr->pu1VidMPEG4VolData, 0, usr_ptr->u4VidMPEG4VolDataLen);
		pu1NextUsrBufAddr += CFA_ALIGN_SZ(usr_ptr->u4VidMPEG4VolDataLen, sizeof(uintptr_t));
		u4Sz += CFA_ALIGN_SZ(usr_ptr->u4VidMPEG4VolDataLen, sizeof(uintptr_t));
		if (u4Sz > u4TotalSz) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d size is large than total size.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -ENOMEM;
		}

		if (get_user(compatSeqHdr, &(usr_ptr32->pu1VidMPEG4VolData))){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(pu1VidMPEG4VolData).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}
		if (0 == compatSeqHdr){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(compatSeqHdr = 0).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}
		pu1VidMPEG4VolData = compat_ptr(compatSeqHdr);
		if (!access_ok(VERIFY_READ, pu1VidMPEG4VolData,
			usr_ptr->u4VidMPEG4VolDataLen)){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in access_ok(pu1VidMPEG4VolData: 0x%p).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pu1VidMPEG4VolData);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}

		if (copy_from_user((void __user *)usr_ptr->pu1VidMPEG4VolData,
			 pu1VidMPEG4VolData, usr_ptr->u4VidMPEG4VolDataLen)){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(pu1VidMPEG4VolData).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}
	}

	if (copy_from_user(&(usr_ptr->u4VidPPSSPSHeaderDataLen),
		&(usr_ptr32->u4VidPPSSPSHeaderDataLen),
		sizeof(__u32))){
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4VidPPSSPSHeaderDataLen).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}

	if ((0 < usr_ptr32->u4VidPPSSPSHeaderDataLen) && (0 == usr_ptr32->pu1VidPPSSPSHeaderData)) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("%s line %d fail for no H264 header, but header len(%d) > 0.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr->u4VidPPSSPSHeaderDataLen);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}

	if (0 != usr_ptr32->pu1VidPPSSPSHeaderData) {
		void *pu1VidPPSSPSHeaderData = NULL;

		usr_ptr->pu1VidPPSSPSHeaderData = pu1NextUsrBufAddr;

		if (NULL == usr_ptr->pu1VidPPSSPSHeaderData) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in alloc compat user space.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -ENOMEM;
		}
		mm_memset(usr_ptr->pu1VidPPSSPSHeaderData, 0, usr_ptr->u4VidPPSSPSHeaderDataLen);
		pu1NextUsrBufAddr += CFA_ALIGN_SZ(usr_ptr->u4VidPPSSPSHeaderDataLen, sizeof(uintptr_t));
		u4Sz += CFA_ALIGN_SZ(usr_ptr->u4VidPPSSPSHeaderDataLen, sizeof(uintptr_t));
		if (u4Sz > u4TotalSz)
		{
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d size is large than total size.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -ENOMEM;
		}

		if (get_user(compatSeqHdr, &(usr_ptr32->pu1VidPPSSPSHeaderData))){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(pu1VidPPSSPSHeaderData).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}
		if (0 == compatSeqHdr){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(compatSeqHdr = 0).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}
		pu1VidPPSSPSHeaderData = compat_ptr(compatSeqHdr);
		if (!access_ok(VERIFY_READ, pu1VidPPSSPSHeaderData,
			usr_ptr->u4VidPPSSPSHeaderDataLen)){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in access_ok(pu1VidPPSSPSHeaderData: 0x%p).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pu1VidPPSSPSHeaderData);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}

		if (copy_from_user((void __user *)usr_ptr->pu1VidPPSSPSHeaderData,
			 pu1VidPPSSPSHeaderData, usr_ptr->u4VidPPSSPSHeaderDataLen)){
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(pu1VidPPSSPSHeaderData).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}
	}
	if (copy_from_user(&(usr_ptr->u1AvcPayloadLenFieldSz),
		&(usr_ptr32->u1AvcPayloadLenFieldSz),
		sizeof(__u8))){
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u1AvcPayloadLenFieldSz).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}

#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
	ret = CfaAviCompatVbrGarbageInfInfo(&(usr_ptr->rVbrGarbageInf),
		&(usr_ptr32->rVbrGarbageInf));
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaAviCompatVbrGarbageInfInfo.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return ret;
	}

#endif

#if CONFIG_CFA_AVI_NEW_METHOD_FOR_ERR_JUMP
	if (copy_from_user(&(usr_ptr->fgIsFileHasIndex),
		&(usr_ptr32->fgIsFileHasIndex),
		sizeof(bool))){
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(fgIsFileHasIndex).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}
#endif
#if CONFIG_CFA_AVI_SUPPORT_AUDIO_DRM
	if (copy_from_user(&(usr_ptr->rAudCryptInfo),
		&(usr_ptr32->rAudCryptInfo),
		sizeof(CfaAviAudCryptInfo))){
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(rAudCryptInfo)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}
#endif
#if CONFIG_CFA_AVI_NEW_SYNC_PB_BUF_FLOW
	if (copy_from_user(&(usr_ptr->fgTooHighFrameRate),
		&(usr_ptr32->fgTooHighFrameRate),
		sizeof(bool))){
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(fgTooHighFrameRate).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}
#endif
#if CONFIG_CFA_AVI_SUPPORT_FIFO_LIMITATION
	if (copy_from_user(&(usr_ptr->rFifoInfo),
		&(usr_ptr32->rFifoInfo),
		sizeof(CfaAviFifoInfo))){
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(rFifoInfo).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}
#endif

	ret = CfaAviCompatVbrMp3GarbageInfInfo(&(usr_ptr->rVbrMp3GarbageInfo), &(usr_ptr32->rVbrMp3GarbageInfo));
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaAviCompatVbrMp3GarbageInfInfo.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return ret;
	}

#if CONFIG_CFA_AVI_USE_THRESHOLD_TMP
	if (copy_from_user(&(usr_ptr->fgUseThreshold),
		&(usr_ptr32->fgUseThreshold),
		sizeof(bool))){
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(fgUseThreshold).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}
#endif

#if AVI_SUPPOTR_DRM
	ret = CfaAviCompatDrmInfInfo(&(usr_ptr->rDrmInfo),
		&(usr_ptr32->rDrmInfo), &pu1NextUsrBufAddr, &u4Sz, u4TotalSz);
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaAviCompatDrmInfInfo.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return ret;
	}
#endif

	DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("%s line %d Success, usr_ptr: 0x%p, buf_sz: %d.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr, sizeof(CfaAviConfigInfo));

	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaAviConfigInfo);

	return 0;
}

static int CfaAviProcCompat(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	long ret = 0;

	if ((NULL == prInfo) || (NULL == pfgIsUserMem)) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail for invalid parameter.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	}
	switch (prInfo->type) {
	case CFA_CONFIG:
	if (prInfo->is_get) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	}
	ret = CfaAviCompatConfig(prInfo, pfgIsUserMem);
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaMkvCompatConfig.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}
	break;
	case CFA_RANGE:
    if (prInfo->is_get) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	}
	ret = CfaAviCompatRange(prInfo, pfgIsUserMem);
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaMkvCompatRange.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}
	break;
	case CFA_GEN_INFO:
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("%s line %d fail for don;t support get info for cfa avi.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	case CFA_JUMP_INFO:
		if (prInfo->is_get) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
		}
		ret = CfaAviCompatJumpRange(prInfo, pfgIsUserMem);
		if (0 != ret) {
		  DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		    TEXT("%s line %d fail in CfaMkvCompatJumpRange.\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO);
		  return ret;
		}
		break;
	default:
		break;
	}
	return 0;
}

#endif
/*-----------------------------------------------------------------------------
 * Name: CfaAvi_SetRange
 *
 * Description:
 *		AVI CFA sets demuxing range
 *		splitter will ensure that pfvSetRange is only called in "off" state.
 *		If used with MPC, the range of MPC_SCMD_SPR will be passed here
 *
 * Inputs:
 *		[IN] uintptr_t of splitter
 *		[IN] pointer to CfaAviRange
 *		[IN] pointer to CfaAviInst
 *
 * Outputs:
 *
 * Returns: s32
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAvi_SetRange(void *pvSptHdl, void *pvRange, void *pvPrivData, bool fgIsUserMem)
{
	CfaAviInst *prCfaAviInst = NULL;

	TCfaAviStrmInf *ptVStrmInf = NULL;
	TCfaAviStrmInf *ptAStrmInf = NULL;
	TCfaAviStrmInf *ptSpStrmInf = NULL;
	TCfaAviPrsStrmInf *ptPrsStrmInf = NULL;

	u64 u8MinVidioOfst = (u64)(-1LL);
	u64 u8MinAudioOfst = (u64)(-1LL);
	u64 u8MinSpOfst = (u64)(-1LL);

#if CONFIG_CFA_AVI_MULTIPLE_AUDIO_INFO
	CfaAviAudioRange *prAudioRange = NULL;
	u32 u4Idx = 0;
#endif

	if ((NULL == pvRange) || (NULL == pvPrivData))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	prCfaAviInst = (CfaAviInst *)pvPrivData;
	MMATE_CHECK_POINTER(prCfaAviInst);

	prCfaAviInst->u8LastVPts = 0;
	prCfaAviInst->u8LastAPts = 0;
	prCfaAviInst->u4GTxLen = 0;
	prCfaAviInst->u4TxTimeAfterSyncPB = 0;

	if (0 != mm_copy_from_user(&(prCfaAviInst->rCfaRange), pvRange, sizeof(CfaAviRange))) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA MKV] %s line %d failed in mm_copy_from_user\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}
	MMATE_INIT_STRUCT(prCfaAviInst->rCfaRange);


	if (CfaAviToPlay(prCfaAviInst->u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_V))
		u8MinVidioOfst = prCfaAviInst->rCfaRange.u8VidStartOfst;

	if (CfaAviToPlay(prCfaAviInst->u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_A))
		u8MinAudioOfst = prCfaAviInst->rCfaRange.u8AudStartOfst;

	if (CfaAviToPlay(prCfaAviInst->u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_SP0))
		u8MinSpOfst = prCfaAviInst->rCfaRange.u8SubStartOfst;

	ptVStrmInf = &(prCfaAviInst->rCfaAviVInf.tStrmInf);

	ptVStrmInf->u4TxedChunk = prCfaAviInst->rCfaRange.u4VidStartChunkNo;

#if CONFIG_CFA_AVI_TX_ALL_SP
	prCfaAviInst->ucCurSpInfoIdx = CfaAviGetSpInfoIdx(prCfaAviInst, prCfaAviInst->u4CurSpStrmID);
	prCfaAviInst->ucSpInfoIdx = 0;
	ptSpStrmInf = &(prCfaAviInst->rCfaAviSpInf[prCfaAviInst->ucCurSpInfoIdx].tStrmInf);
#else
	ptSpStrmInf = &(prCfaAviInst->rCfaAviSpStrmInf);
#endif

	ptSpStrmInf->u4TxedChunk = prCfaAviInst->rCfaRange.u4SubStartChunkNo;

#if CONFIG_CFA_AVI_MULTIPLE_AUDIO_INFO
	for (u4Idx = 0; u4Idx < MAX_NS_AVI_AUD; u4Idx++) {
		ptAStrmInf = &(prCfaAviInst->rCfaAviAInf[u4Idx].tStrmInf);
		ptAStrmInf->fgSetAudHdr = TRUE;
		ptAStrmInf->fgSetAccfileAudHdr = TRUE;

		prAudioRange = &(prCfaAviInst->rCfaRange.rAudioRange[u4Idx]);

		if (TRUE == (prCfaAviInst->rCfaRange).rAudioRange[u4Idx].fgIsValid) {
			if ((prCfaAviInst->rCfaRange).rAudioRange[u4Idx].u8AudStartOfst < u8MinAudioOfst)
				u8MinAudioOfst = (prCfaAviInst->rCfaRange).rAudioRange[u4Idx].u8AudStartOfst;

			DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI][CFA KEY REG]rAudioRange[%d].u8AudStartOfst:0x%llx\r\n"),
				u4Idx, (prCfaAviInst->rCfaRange).rAudioRange[u4Idx].u8AudStartOfst);

			ptAStrmInf->u4TxedByte = (u32)prAudioRange->u8AudStartByte;
			ptAStrmInf->u4TxedChunk = prAudioRange->u4AudStartChunkNo;
		} else {
		   /*ptAStrmInf->u4TxedByte = prAudioRange->u8AudStartByte;*/
		   /*ptAStrmInf->u4TxedChunk = prAudioRange->u4AudStartChunkNo;*/
		}
	}
#endif

	/*-----------------------------------------------------------------------------
		we can't get the Aud info idx when vSetStrmInf,
		since we set the audio related info later in the vConfigInfo.
		-----------------------------------------------------------------------------*/
	prCfaAviInst->ucCurAudInfoIdx = CfaAviGetAudInfoIdx(prCfaAviInst, prCfaAviInst->u4CurAudStrmID);

	if (CFA_AVI_INVALID_STRM_ID == prCfaAviInst->ucCurAudInfoIdx) {/*add by czw for fix bug 114372 */
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]CfaAvi_SetRange: ucCurAudInfoIdx CFA_AVI_INVALID_STRM_ID\r\n"));
			prCfaAviInst->ucCurAudInfoIdx = 0;
	}

	ptAStrmInf = &(prCfaAviInst->rCfaAviAInf[prCfaAviInst->ucCurAudInfoIdx].tStrmInf);

#if CONFIG_CFA_AVI_TX_ALL_AUD
	prCfaAviInst->ucAudInfoIdx = 0;
#endif

#if !CONFIG_CFA_AVI_TX_ALL_AUD
	ptAStrmInf->u4TxedChunk = prCfaAviInst->rCfaRange.u4AudStartChunkNo; /* for VBR */
	ptAStrmInf->u4TxedByte	= (u32)prCfaAviInst->rCfaRange.u8AudStartByte;  /* for CBR */
#endif

	ptPrsStrmInf = &(prCfaAviInst->rCfaAviPrsStrmInf);

	prCfaAviInst->u4PrsFlg = prCfaAviInst->rCfaRange.u4PrsFlag;
	prCfaAviInst->fgPlayFrmFlag = prCfaAviInst->rCfaRange.fgPlayFrmFlag;

	ptPrsStrmInf->u4VidPrsChunk = prCfaAviInst->rCfaRange.u4VidStartChunkNo;
	ptPrsStrmInf->u4AudPrsChunk = prCfaAviInst->rCfaRange.u4AudStartChunkNo; /* for VBR */
	ptPrsStrmInf->u8AudPrsByte = prCfaAviInst->rCfaRange.u8AudStartByte;  /* for CBR */

	ptPrsStrmInf->u4SubPrsChunk = prCfaAviInst->rCfaRange.u4SubStartChunkNo;

	if (prCfaAviInst->rCfaRange.u8Endoffst == 0) {
		prCfaAviInst->u4PrsFlg = 0;
		MM_RETURN(RET_DMX_OK);
	}

	prCfaAviInst->u8Endoffst = prCfaAviInst->rCfaRange.u8Endoffst;
	prCfaAviInst->u8EndPts = prCfaAviInst->rCfaRange.u8EndPts;
	#ifdef MM_SUPPORT_DIVXHT31
	prCfaAviInst->u8FFRangeEndPts = prCfaAviInst->rCfaRange.u8FFRangeEndPts;
	#endif
	prCfaAviInst->u8FileSz = prCfaAviInst->rCfaRange.u8FileSz;

	/* current position infomation. */
	prCfaAviInst->rCfaAviCurPosiInfo.u8VidCurOfst	 = prCfaAviInst->rCfaRange.u8VidStartOfst;
	prCfaAviInst->rCfaAviCurPosiInfo.u4VidCurChunkNo = prCfaAviInst->rCfaRange.u4VidStartChunkNo;
	prCfaAviInst->rCfaAviCurPosiInfo.u8AudCurOfst	 = prCfaAviInst->rCfaRange.u8AudStartOfst;
	prCfaAviInst->rCfaAviCurPosiInfo.u4AudCurChunkNo = prCfaAviInst->rCfaRange.u4AudStartChunkNo;
	prCfaAviInst->rCfaAviCurPosiInfo.u8AudCurByte	 = prCfaAviInst->rCfaRange.u8AudStartByte;
	prCfaAviInst->rCfaAviCurPosiInfo.u8SubCurOfst	 = prCfaAviInst->rCfaRange.u8SubStartOfst;
	prCfaAviInst->rCfaAviCurPosiInfo.u4SubCurChunkNo = prCfaAviInst->rCfaRange.u4SubStartChunkNo;

	if (prCfaAviInst->rCfaRange.fgReset) {
		prCfaAviInst->fgBGrouped = FALSE;
		prCfaAviInst->u1ChunkVopNs = 0;
		prCfaAviInst->u1CurBGrpNum = 0;
		prCfaAviInst->u1TotalBGrpNum = 0;
	}

	prCfaAviInst->u8TransferdAudChunks = 0;

	prCfaAviInst->u8SearchCnt = 0;

#if CONFIG_CFA_AVI_NEW_METHOD_FOR_ERR_JUMP
	prCfaAviInst->u8OffsetJumpCnt = 0;
#endif

#if CONFIG_CFA_AVI_SUPPORT_CORRECT_CHUNK_ABNORMITY
	prCfaAviInst->fgNextChunkDetecting = FALSE;

	dmx_memset(prCfaAviInst->aucPrev4cc, 0, AVI_4CC_BYTES);

	prCfaAviInst->u8PrevCa = 0;
	prCfaAviInst->u4PrevDataSize = 0;
#endif

#if CONFIG_CFA_AVI_NEW_METHOD_FOR_ABR_AUDIO
	#if CONFIG_CFA_AVI_SUPPORT_MULTI_AUDIO_FOR_ABR
		for (u4Idx = 0; u4Idx < MAX_NS_AVI_AUD; u4Idx++)
			prCfaAviInst->fgPrevIsVidChunk[u4Idx] = FALSE;
	#else
		prCfaAviInst->fgPrevIsVidChunk = FALSE;
	#endif
#endif

	prCfaAviInst->fgGotValidVidChunk = FALSE;

#if CONFIG_CFA_AVI_NEW_SYNC_PB_BUF_FLOW
	prCfaAviInst->u4RemainDataInPbBuf = 0;
	prCfaAviInst->u8LastCa = 0;
	prCfaAviInst->fgNeedAdjustAfterTxDone = FALSE;
#endif

#if CONFIG_CFA_AVI_DIVIDE_LARGE_AUD_CHUNK
	prCfaAviInst->u4CurAudChunkTxedOfst = 0;
	prCfaAviInst->u4AudChunkLastTxedByte = 0;
	prCfaAviInst->fgStartTxAudChunk = FALSE;
#endif

#if CONFIG_CFA_AVI_PLAY_ONE_FRM_ONLY_TX_I_FRM
	prCfaAviInst->fgNeedFinishIfTxDone = FALSE;
#endif

	prCfaAviInst->u4CurIFrmChunkNo = DMX_INVALID_UINT32;

	prCfaAviInst->u8CurIFrmChunkOfst = DMX_INVALID_UINT64;

	DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT, TEXT("[CFA AVI]: vCfaAviSetRange\r\n"));

	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]: vCfaAviSetRange, pvSptHdl:%p\r\n"), pvSptHdl);

	prCfaAviInst->u8MinStartStrmOfst = MIN(u8MinVidioOfst, MIN(u8MinAudioOfst, u8MinSpOfst));
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]: eRangeType: %d\r\n"),
		(u32)(prCfaAviInst->rCfaRange.eRangeType));/* build error ,so close this log*/

	#if AVI_SUPPORT_SKIP_DATA_IN_TX_DATA
	prCfaAviInst->u8SkipVDataInTxData = 0;
	prCfaAviInst->u8SkipADataInTxData = 0;
	prCfaAviInst->u8SkipSpDataInTxData = 0;
	#endif

	prCfaAviInst->fgFristVidBlk = FALSE;
	prCfaAviInst->fgFristAudBlk = FALSE;

	MMATE_CHECK_POINTER(prCfaAviInst);
	MMATE_CHECK_STRUCT(prCfaAviInst->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaAviInst->rCfaAviCurPosiInfo);
	MMATE_CHECK_STRUCT(prCfaAviInst->rVbrGarbageInf);
	MMATE_CHECK_STRUCT(prCfaAviInst->rFifoInfo);

	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:SetVdoSOft: 0x%llx\r\n"), prCfaAviInst->rCfaRange.u8VidStartOfst);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:SetVdoSChk: 0x%lx\r\n"), prCfaAviInst->rCfaRange.u4VidStartChunkNo);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:SetVdoSkipChk: 0x%lx\r\n"), prCfaAviInst->rCfaRange.u4VidSkipChunkNs);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:SetVdoEndChk: 0x%lx\r\n"), prCfaAviInst->rCfaRange.u4VidEndChunkNo);

	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:SetAdoSOft: 0x%llx\r\n"), prCfaAviInst->rCfaRange.u8AudStartOfst);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:SetAdoSBytes: 0x%llx\r\n"), prCfaAviInst->rCfaRange.u8AudStartByte);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:SetAdoSkipBytes: 0x%lx\r\n"), prCfaAviInst->rCfaRange.u4AudSkipByte);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:SetAdoEndBytes: 0x%lx\r\n"), prCfaAviInst->rCfaRange.u8AudEndByte);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:SetAdoSChk: 0x%lx\r\n"), prCfaAviInst->rCfaRange.u4AudStartChunkNo);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:SetAdoSkipChk: 0x%lx\r\n"), prCfaAviInst->rCfaRange.u4AudSkipChunkNs);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:SetAdoEndChk: 0x%lx\r\n"), prCfaAviInst->rCfaRange.u4AudEndChunkNo);

	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:SetStSOft: 0x%llx\r\n"), prCfaAviInst->rCfaRange.u8SubStartOfst);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:SetStSChk: 0x%lx\r\n"), prCfaAviInst->rCfaRange.u4SubStartChunkNo);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:SetStSkipChk: 0x%lx\r\n"), prCfaAviInst->rCfaRange.u4SubSkipChunkNs);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:SetStEndChk: 0x%lx\r\n"), prCfaAviInst->rCfaRange.u4SubEndChunkNo);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:SetStEndOft: 0x%llx\r\n"), prCfaAviInst->rCfaRange.u8SubEndOfst);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:SetEndOft: 0x%llx\r\n"), prCfaAviInst->rCfaRange.u8Endoffst);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:SetFileSize: 0x%llx\r\n"), prCfaAviInst->rCfaRange.u8FileSz);

	MM_RETURN(RET_DMX_OK);
}

/*-----------------------------------------------------------------------------
 * Name: CfaAvi_SetJumpRange
 *
 * Description:
 *		Fro Support 8/16/32 fast forward and fast backward, to Reset CFA All state.
 *
 * Inputs:
 *		[IN] uintptr_t of splitter
 *		[IN] pointer to CfaAviKeyFrameRange
 *		[IN] pointer to CfaAviInst
 *
 * Outputs:
 *
 * Returns: s32

 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAvi_SetJumpRange(void *pvSptHdl, void *pvJmpRange, void *pvPrivData)
{
	CfaAviInst *prCfaAviInst = NULL;
	CfaAviKeyFrameRange *prAviKFrmRange = NULL;
	CfaAviRange *prCfaRangeInfo = NULL;

	if ((NULL == pvPrivData) || (NULL == pvJmpRange)) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: pvPrivData is null or pvJmpRange is null!!\r\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}

	prCfaAviInst = (CfaAviInst *) pvPrivData;
	prAviKFrmRange = (CfaAviKeyFrameRange *)pvJmpRange;

	prCfaRangeInfo = &(prAviKFrmRange->rCfaRangeInfo);
	MMATE_CHECK_POINTER(prCfaAviInst);

	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI][CFA KEY REG]CfaAvi_SetJumpRange, u4PsrFlag:%d (v1,a2),")
		TEXT("u8VidStartOfst:0x%llx u8AudStartOfst:0x%llx")
		TEXT("u8AudStartPts:%lld u8SubStartOfst:0x%llx\r\n"),
		prCfaAviInst->u4CurPrsFlg,
		prCfaRangeInfo->u8VidStartOfst,
		prCfaRangeInfo->u8AudStartOfst,
		prCfaRangeInfo->u8AudStartPts,
		prCfaRangeInfo->u8SubStartOfst);

	CfaAvi_SetRange(pvSptHdl, (void *)(&(prAviKFrmRange->rCfaRangeInfo)), pvPrivData, FALSE);

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaAviGetParamSize(void *pvSptHdl, u32 u4ParamID,
	void  *pvPrivData, void *pvCfaParam, u32 u4CfaParamSz)
{
	MRESULT mrRet = RET_DMX_OK;

	switch (u4ParamID) {
	case CFA_PARAM_ID_JUMP_INFO_SIZE:

		if ((NULL == pvCfaParam) || ((u4CfaParamSz) < sizeof(u32)))
			mrRet = RET_DMX_PARAM_WRONG;
		else {
			u32 *pu4Tmp = (u32 *)pvCfaParam;
			*pu4Tmp = sizeof(CfaAviKeyFrameRange);
		}

		break;
	default:
		mrRet = RET_DMX_PARAM_WRONG;
		break;
	}

	MM_RETURN(mrRet);
}

/*-----------------------------------------------------------------------------
 * Name: CfaAvi_EnableStrm
 *
 * Description:
 *		AVI CFA sets stream to parse, may be combinations of V/A/S.
 *		splitter will ensure that pfvSetStrm() is only called in "off" or "paused" state.
 *
 * Inputs:
 *		[IN] uintptr_t of splitter
 *		[IN] streams to parse or to cancel parsing
 *		[IN] CFA_STREAM_ON:  The bits turned ON in u4StrmToPrs are the streams that FMPC would like to parse.
 *			CFA_STRM_OFF: The bits turned ON in u4StrmToPrs are the streams
			that FMPC would like to stop parsing
 *		[IN] pointer to CfaAviInst)
 *
 * Outputs:
 *
 * Returns: s32
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAvi_EnableStrm(void *pvSptHdl, u32 u4StrmToPrs, CfaStreamOp eOp,
							  void *pvPrivData)
{
	CfaAviInst *prCfaAviInst = NULL;

	if (NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: pvPrivData is null!!\r\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}
	prCfaAviInst = (CfaAviInst *)pvPrivData;
	MMATE_CHECK_POINTER(prCfaAviInst);

	/*-----------------------------------------------------------------------------
		we can't get the Aud info idx when vSetStrmInf,
		since we set the audio related info later in the vConfigInfo.
		-----------------------------------------------------------------------------*/
	prCfaAviInst->ucCurAudInfoIdx = CfaAviGetAudInfoIdx(prCfaAviInst, prCfaAviInst->u4CurAudStrmID);
	if (CFA_AVI_INVALID_STRM_ID == prCfaAviInst->ucCurAudInfoIdx) {/* add for CNB00123304 */

		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI] CfaAvi_EnableStrm: CFA_AVI_INVALID_STRM_ID\r\n"));
		prCfaAviInst->ucCurAudInfoIdx = 0;
	}

	#if CONFIG_CFA_AVI_TX_ALL_SP
	prCfaAviInst->ucCurSpInfoIdx = CfaAviGetSpInfoIdx(prCfaAviInst, prCfaAviInst->u4CurSpStrmID);
	#endif

	if (CFA_STREAM_ON == eOp) {
		/* enable */
		if (((u32)CFA_STRM_V) & u4StrmToPrs) {
			prCfaAviInst->u4CurPrsFlg |= CFA_AVI_PRS_BIT_STRM_TYPE_V;
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]:vCfaAviEnableStrm: Vdo\r\n"));
		}

		if (((u32)CFA_STRM_A) & u4StrmToPrs) {
			prCfaAviInst->u4CurPrsFlg |= CFA_AVI_PRS_BIT_STRM_TYPE_A;
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]:vCfaAviEnableStrm: Ado\r\n"));
		}

		if (((u32)CFA_STRM_SP) & u4StrmToPrs) {
			prCfaAviInst->u4CurPrsFlg |= CFA_AVI_PRS_BIT_STRM_TYPE_SP0;
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]:vCfaAviEnableStrm: SP\r\n"));
		}
	} else  /* CFA_STRM_OFF */ {
		/* disable */
		if (((u32)CFA_STRM_V) & u4StrmToPrs) {
			prCfaAviInst->u4CurPrsFlg &= ~((u32)CFA_AVI_PRS_BIT_STRM_TYPE_V);
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]:vCfaAviDisableStrm: Vdo\r\n"));
		}

		if (((u32)CFA_STRM_A) & u4StrmToPrs) {
			prCfaAviInst->u4CurPrsFlg &= ~((u32)CFA_AVI_PRS_BIT_STRM_TYPE_A);
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]:vCfaAviDisableStrm: Ado\r\n"));
		}

		if (((u32)CFA_STRM_SP) & u4StrmToPrs) {
			prCfaAviInst->u4CurPrsFlg &= ~((u32)CFA_AVI_PRS_BIT_STRM_TYPE_SP0);
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]:vCfaAviDisableStrm: SP\r\n"));
		}
	}

	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
 * Name: CfaAvi_SetStrmInf
 *
 * Description:
 *		Set Stream information
 *
 * Inputs:
 *		[IN] uintptr_t of splitter
 *		[IN] stream to set
 *		[IN] stream info
 *		[IN] pointer to CfaAviInst
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAvi_SetStrmInf(void *pvSptHdl, u32 u4Strm, u32 u4Info, void *pvPrivData)
{
	CfaAviInst *prCfaAviInst = NULL;
	if (NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: pvPrivData is null!!\r\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}
	prCfaAviInst = (CfaAviInst *)pvPrivData;

	if (((u32)CFA_STRM_V) == u4Strm) {
		TCfaAviVInf *prCfaAviVInf = &prCfaAviInst->rCfaAviVInf;

		prCfaAviVInf->tStrmInf.u4StrmIdx = ((u32)0x000000ff&u4Info);
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:vCfaAviSetStrmInf V: 0x%lx\r\n"), prCfaAviVInf->tStrmInf.u4StrmIdx);
	} else if (((u32)CFA_STRM_A) == u4Strm) {
		prCfaAviInst->u4CurAudStrmID = (((u32)0x000000ff)&u4Info);
		prCfaAviInst->ucCurAudInfoIdx = CfaAviGetAudInfoIdx(prCfaAviInst, ((u32)0x000000ff&u4Info));
	if (((u8)CFA_AVI_INVALID_STRM_ID) == prCfaAviInst->ucCurAudInfoIdx) {/*add for CNB00123304 */
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI] CfaAvi_SetRange: CfaAvi_SetStrmInf CFA_AVI_INVALID_STRM_ID\r\n"));
			prCfaAviInst->ucCurAudInfoIdx = 0;
	}
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:vCfaAviSetStrmInf A: 0x%lx\r\n"), (u32)0x000000ff & u4Info);
	} else if (((u32)CFA_STRM_SP) == u4Strm) {
		#if CONFIG_CFA_AVI_TX_ALL_SP
		prCfaAviInst->u4CurSpStrmID = ((u32)0x000000ff&u4Info);
		prCfaAviInst->ucCurSpInfoIdx = CfaAviGetSpInfoIdx(prCfaAviInst, ((u32)0x000000ff&u4Info));
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:vCfaAviSetStrmInf SP: 0x%lx\r\n"), (u32)0x000000ff & u4Info);
		#else
		prCfaAviInst->rCfaAviSpStrmInf.u4StrmIdx = ((u32)0x000000ff&u4Info);
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:vCfaAviSetStrmInf SP: 0x%lx\r\n"), prCfaAviInst->rCfaAviSpStrmInf.u4StrmIdx);
		#endif
	} else {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: stream type is IDLE!!\r\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}

	MM_RETURN(RET_DMX_OK);
}

/*-----------------------------------------------------------------------------
 * Name: CfaAvi_TurnOn
 *
 * Description:
 *		AVI CFA turns on file demuxing
 *		A transfer should be issued in this function.
 *
 * Inputs:
 *		[IN] uintptr_t of splitter
 *		[IN] pointer to CfaAviInst
 *
 * Outputs:
 *
 * Returns: s32
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAvi_TurnOn(void *pvSptHdl, void *pvPrivData)
{
	u64 u8CfaAviFinishEaTmp = 0;
	CfaAviInst *prCfaAviInst = NULL;
	u64 u8Sa = 0;
	u8 uAudMaxDuration = 0;
	TCfaAviVInf *prCfaAviVInf = NULL;

	if (NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: pvPrivData is null!!\r\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}
	prCfaAviInst = (CfaAviInst *)pvPrivData;
	MMATE_CHECK_POINTER(prCfaAviInst);
	MMATE_CHECK_STRUCT(prCfaAviInst->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaAviInst->rCfaAviCurPosiInfo);
	MMATE_CHECK_STRUCT(prCfaAviInst->rVbrGarbageInf);
	MMATE_CHECK_STRUCT(prCfaAviInst->rFifoInfo);

	if (CFA_AVI_INVALID_STRM_ID == prCfaAviInst->ucCurAudInfoIdx) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: invalid stream ID!\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	CfaAviInitPara(prCfaAviInst);

	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON, TEXT("[CFA AVI] Cfa avi Turn On\r\n"));

	prCfaAviVInf = &prCfaAviInst->rCfaAviVInf;

	if (DMX_IS_RW_PLAY(pvSptHdl) || DMX_IS_FF_PLAY(pvSptHdl))
		prCfaAviVInf->tStrmInf.fgSetAudHdr = FALSE;
	else
		prCfaAviVInf->tStrmInf.fgSetAudHdr = TRUE;

	if (0 == prCfaAviInst->u4PrsFlg) {
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]CfaAvi_TurnOn, u4PrsFlg is Zero!\r\n"));

		u8CfaAviFinishEaTmp = 0;
		CfaAviFinishPrs(pvSptHdl, prCfaAviInst, u8CfaAviFinishEaTmp);

		MM_RETURN(RET_DMX_OK);
	}

	if (prCfaAviInst->u8Ca == DMX_INVALID_UINT64) {
		u8Sa = CfaAviGetAvaTxSa(prCfaAviInst);
		if (DMX_INVALID_UINT64 == u8Sa) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI]: invalid start offset!\r\n"));
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
	}

#if CONFIG_CFA_AVI_NEW_SYNC_PB_BUF_FLOW
	prCfaAviInst->u8LastCa = prCfaAviInst->u8Ca;
#endif

	prCfaAviInst->fgFillDummyAU = FALSE;

	uAudMaxDuration = (u8) prCfaAviInst->u4MaxAudChunkDuration;

	SplitterSetAudioMaxDuration(pvSptHdl, uAudMaxDuration);
	prCfaAviInst->rCfaAviVInf.tStrmInf.fgSpecMPEG1InfoTxed = FALSE;
	if ((0 == prCfaAviInst->u8PrevVidOffset) &&
		/*(prCfaAviInst->rCfaRange.u8StartPts > 0) &&*/
		(prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecMPEG1Info.u4CodecSpecDataLen > 0) &&
		(prCfaAviInst->u4PrsFlg  & CFA_AVI_PRS_BIT_STRM_TYPE_V) &&
		(CFA_VID_MPEG2 == prCfaAviInst->eVidType) &&
		(FALSE == prCfaAviInst->rCfaAviVInf.tStrmInf.fgSpecMPEG1InfoTxed)) {
		prCfaAviInst->rCfaAviVInf.tStrmInf.u8CaMPEG1Txbefore = prCfaAviInst->u8Ca;
		prCfaAviInst->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_RE_TX_VID_MPEG1_CODEC_HEADER;

		/* read bytes after 'movi' 4cc. */
		CfaAviNextScSearch(pvSptHdl,
						   prCfaAviInst,
						   CFA_AVI_ANA_AVIPRS_RE_TX_VID_MPEG1_CODEC_HEADER,
						   0,
						   AVI_GEN_READ_BYTES,
						   0);

		MM_RETURN(RET_DMX_OK);
	}
	prCfaAviInst->rCfaAviVInf.tStrmInf.fgSpecMPEG4InfoTxed = FALSE;
	if ((prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecMPEG4Info.u4CodecSpecDataLen > 0) &&
	   (prCfaAviInst->u4PrsFlg	& CFA_AVI_PRS_BIT_STRM_TYPE_V) &&
	   (FALSE == prCfaAviInst->rCfaAviVInf.tStrmInf.fgSpecMPEG4InfoTxed)) {
		prCfaAviInst->rCfaAviVInf.tStrmInf.u8CaMPEG4InfoTxbefore = prCfaAviInst->u8Ca;
		prCfaAviInst->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_RE_TX_VID_MPEG4_VOL_HEADER;

		/* read bytes after 'movi' 4cc. */
		CfaAviNextScSearch(pvSptHdl,
						   prCfaAviInst,
						   CFA_AVI_ANA_AVIPRS_RE_TX_VID_MPEG4_VOL_HEADER,
						   0,
						   AVI_GEN_READ_BYTES,
						   0);

		MM_RETURN(RET_DMX_OK);
	}
	prCfaAviInst->rCfaAviVInf.tStrmInf.fgSpecH264InfoTxed = FALSE;
	if((prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecH264H265Info.pu1CodecSpecData > 0 ) &&
           (prCfaAviInst->u4PrsFlg & CFA_AVI_PRS_BIT_STRM_TYPE_V) &&
           (prCfaAviInst->rCfaAviVInf.tStrmInf.rSpecH264H265Info.u8VidFirstFrameOfst < prCfaAviInst->u8Ca)) {
		prCfaAviInst->rCfaAviVInf.tStrmInf.u8CaH264InfoTxbefore = prCfaAviInst->u8Ca;
		prCfaAviInst->eCurCfaAviAnaSt = CFA_AVI_ANA_AVIPRS_RE_TX_VID_H264OrH265_HEADER;
		/* read bytes after 'movi' 4cc. */
		CfaAviNextScSearch(pvSptHdl,
			prCfaAviInst,
			CFA_AVI_ANA_AVIPRS_RE_TX_VID_H264OrH265_HEADER,
			0,
			AVI_GEN_READ_BYTES,
			0);
		MM_RETURN(RET_DMX_OK);
	}

	if ((prCfaAviInst->u8Ca > prCfaAviInst->rVbrMp3GarbageInfo.u8GarbageDataEndOffset) &&
		(prCfaAviInst->fgDeteGabageData) &&
		(!(prCfaAviInst->fgDeteGabageDataEnd))) {
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]CfaAvi_TurnOn, ===fgDeteGabageDataEnd Set TRUE ===!\r\n"));
		prCfaAviInst->fgDeteGabageDataEnd = TRUE;
		prCfaAviInst->u8TrueStartPts = prCfaAviInst->rVbrMp3GarbageInfo.u8GarbageDataDuration;
		prCfaAviInst->u4GabageChunkNum = prCfaAviInst->rVbrMp3GarbageInfo.u4GarbageDataChkNum;
	}

#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
	if (CFA_RANGE_TYPE_INQUERY_VBR_GARBAGE_INF == prCfaAviInst->rCfaRange.eRangeType) {
		CfaAviNextScSearch(pvSptHdl,
						   prCfaAviInst,
						   CFA_AVI_ANA_AVIPRS_MOVI,
						   0,
						   AVI_GEN_READ_BYTES,
						   0);
	} else
#endif

	if (CFA_RANGE_TYPE_INQUERY == prCfaAviInst->rCfaRange.eRangeType) {
		CfaAviNextScSearch(pvSptHdl,
						   prCfaAviInst,
						   CFA_AVI_ANA_AVIPRS_MOVI,
						   0,
						   AVI_GEN_READ_BYTES,
						   0);
	} else if (CFA_RANGE_TYPE_INC_EXTRA_DATA == prCfaAviInst->rCfaRange.eRangeType) {
		if (prCfaAviInst->u8VidCodecSpecDataOfst >= u8Sa) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI]: spec data offset is large than start offset!\r\n"));
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}

		prCfaAviInst->u8Ca = prCfaAviInst->u8VidCodecSpecDataOfst;

		CfaAviNextScSearch(pvSptHdl,
							prCfaAviInst,
							CFA_AVI_ANA_AVIPRS_TX_EXTRA_DATA,
							0,
							AVI_READ_EXTRA_DATA_LEN_BYTES,
							0);
	} else if ((CFA_RANGE_TYPE_NORMAL == prCfaAviInst->rCfaRange.eRangeType) ||
	(CFA_RANGE_TYPE_UNKNOWN == prCfaAviInst->rCfaRange.eRangeType)) {
		/* read bytes after 'movi' 4cc. */
		CfaAviNextScSearch(pvSptHdl,
							prCfaAviInst,
							CFA_AVI_ANA_AVIPRS_MOVI,
							0,
							AVI_GEN_READ_BYTES,
							0);
	} else {
		/*do nothing*/
	}
	MM_RETURN(RET_DMX_OK);
}




/*-----------------------------------------------------------------------------
 * Name: CfaAvi_TxDone
 *
 * Description:
 *		AVI CFA callback for transfer done
 *		This function will be called after a transfer is complete.
 *
 * Inputs:
 *		[IN] uintptr_t of splitter
 *		[IN] Actual transferred data length.  Normally this value should be equal to the u4Len
 *			  in the previous transfer issue, unless file end is hit.
 *		[IN] pointer to CfaAviInst
 *
 * Outputs:
 *
 * Returns: s32
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAvi_TxDone(void *pvSptHdl, u64 u8TxLen, void *pvPrivData, bool fgRsp)
{
	CfaAviInst *prCfaAviInst = NULL;
	TCfaAviStrmInf *ptAStrmInf = NULL;

#if CONFIG_CFA_AVI_PLAY_ONE_FRM_ONLY_TX_I_FRM
	Cfa2PsrStrmInfo rPathStrmInfo = {0};
#endif

	if (NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: pvPrivData is NULL!\r\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}

	prCfaAviInst = (CfaAviInst *)pvPrivData;
	MMATE_CHECK_POINTER(prCfaAviInst);
	MMATE_CHECK_STRUCT(prCfaAviInst->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaAviInst->rCfaAviCurPosiInfo);
	MMATE_CHECK_STRUCT(prCfaAviInst->rVbrGarbageInf);
	MMATE_CHECK_STRUCT(prCfaAviInst->rFifoInfo);

	if (fgRsp) {
		if (prCfaAviInst->fgVbrGarbageBufSync) {
			prCfaAviInst->u8Ca += 2;
			DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: fgVbrGarbageBufSyn = %d, u8Ca = 0x%llx!\r\n"), prCfaAviInst->fgVbrGarbageBufSync,
			prCfaAviInst->u8Ca );
		}
		MRESULT mrRet = Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaAviInst->u8Ca, u8TxLen,
									(u8 *)&(prCfaAviInst->ptrMemAddress));

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI] CfaAvi_TxDone(fgRsp=TRUE) fail in")
				TEXT("Spt4CfaPbb2SyncBuf ret:%d\r\n"), mrRet);
		}
		if (prCfaAviInst->fgVbrGarbageBufSync) {
			prCfaAviInst->u8Ca -= 2;
		}
		MM_RETURN(mrRet);
	}
	prCfaAviInst->fgVbrGarbageBufSync = FALSE;
#if CONFIG_CFA_AVI_PLAY_ONE_FRM_ONLY_TX_I_FRM
	if (prCfaAviInst->fgNeedFinishIfTxDone) {
		prCfaAviInst->u8Ca += u8TxLen;

		if (CfaAviToPlay(prCfaAviInst->u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_V)) {
			TCfaAviVInf *ptVInf = &(prCfaAviInst->rCfaAviVInf);
			u32 i = 0;

			rPathStrmInfo.u4VstrmNs = 1;

			for (i = 0; i < rPathStrmInfo.u4VstrmNs; i++)
				rPathStrmInfo.ucDecVidStId[i] = (u8)(ptVInf->tStrmInf.u4StrmIdx);
		}

		if (CfaAviToPlay(prCfaAviInst->u4CurPrsFlg, CFA_AVI_PRS_BIT_STRM_TYPE_A)) {
			u32 j = 0;

			rPathStrmInfo.u4AstrmNs = 1;

			for (j = 0; j < rPathStrmInfo.u4AstrmNs; j++)
				rPathStrmInfo.u2DecAudStId[j] = (u16) prCfaAviInst->u4CurAudStrmID;

		}

		Spt4CfaFinishedEx(pvSptHdl, prCfaAviInst->u8Ca, FALSE, GAU_E_EOS);
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]CfaAvi_TxDone, Spt4CfaFinishedEx CfaAviFinishPrs - 1\r\n"));
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]i4CfaAviTxDone, pvSptHdl:%p, u8Ca:0x%llx,")
			TEXT("u8TxLen:0x%llx, u8Endoffst:0x%llx\r\n"),
			pvSptHdl, prCfaAviInst->u8Ca, u8TxLen, prCfaAviInst->u8Endoffst);
		CfaAviFinishPrs(pvSptHdl, prCfaAviInst, prCfaAviInst->u8Endoffst - 1);

		prCfaAviInst->fgNeedFinishIfTxDone = FALSE;

		MM_RETURN(RET_DMX_OK);
	}
#endif

	/*For badinterleave , the file may finish unnormal	9620 bug 's file*/
	ptAStrmInf = &(prCfaAviInst->rCfaAviAInf[prCfaAviInst->ucAudInfoIdx].tStrmInf);

	/*---------------------------------------------------------------------
	//DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
	TEXT("[CFA AVI]: ----done-----ptAStrmInf->u4TxedChunk-------- = %d\r\n"), ptAStrmInf->u4TxedChunk);
	//DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
	TEXT("[CFA AVI]: ----done-----prCfaAviInst->rCfaRange.u4AudEndChunkNo-------- = %d\r\n"),
	prCfaAviInst->rCfaRange.u4AudEndChunkNo);
	//DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
	TEXT("[CFA AVI]: ---done---------prCfaAviInst->u4PrsFlg = %d\r\n"), prCfaAviInst->u4PrsFlg);
	----------------------------------------------------------------------*/

	if ((ptAStrmInf->u4TxedChunk >= (prCfaAviInst->rCfaRange).u4AudEndChunkNo) &&
		(prCfaAviInst->u4PrsFlg == (u32)CFA_AVI_PRS_BIT_STRM_TYPE_A) &&
		(prCfaAviInst->fgDeteGabageData)) {
		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]u4TxedChunk:%d  u4AudEndChunkNo:%d\r\n"),
			ptAStrmInf->u4TxedChunk, prCfaAviInst->rCfaRange.u4AudEndChunkNo);

		DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]CfaAvi_TxDone  ----done--- CfaAviFinishPrs\r\n"));
		CfaAviFinishPrs(pvSptHdl, prCfaAviInst, prCfaAviInst->u8Ca);
	}

	CfaAviTxDoneStCtrl(pvSptHdl, u8TxLen, prCfaAviInst);

	MM_RETURN(RET_DMX_OK);
}



/*-----------------------------------------------------------------------------
 * Name: CfaAvi_GetCurPos
 *
 * Description:
 *		AVI CFA callback for when FMPC needs to know CFA's current position.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAvi_GetCurPos(void *pvSptHdl, void *pvCurPos, void *pvPrivData)
{
	CfaAviInst *prCfaAviInst = NULL;
	u64 *ppu8 = pvCurPos;
	if (NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: pvPrivData is NULL!\r\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}
	prCfaAviInst = (CfaAviInst *)pvPrivData;
	DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("[CFA AVI]:CfaAvi_GetCurPos: 0x%x\r\n"), (u32)prCfaAviInst->u8Ca);

	*ppu8 = prCfaAviInst->u8Ca;


	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
 * Name: CfaAvi_Configure
 *
 * Description:
 *		splitter will ensure that it is only called in "off" or "paused" state.
 *
 * Inputs:
 *		[IN] uintptr_t of splitter
 *		[IN] configure paramter
 *		[IN] pointer to CfaAviInst
 *
 * Outputs:
 *
 * Returns: s32
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAvi_Configure(void *pvSptHdl, void *pvParam, void *pvPrivData, bool fgIsUserMem)
{
	CfaAviConfigInfo rCfaAviConfigInfo;
	CfaAviConfigInfo *prCfaAviConfigInfo = NULL;
	CfaAviInst *prCfaAviInst = NULL;
	TCfaAviVInf *prCfaAviVInf = NULL;
	TCfaAviAInf *prCfaAviAInf = NULL;
	u32 u4Idx = 0;
	int i = 0;

	if (NULL == pvParam) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: %s:: pvParam is NULL!\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_NO_MEM);
	}
	if (fgIsUserMem) {
		if (0 != mm_copy_from_user(&rCfaAviConfigInfo,
			pvParam, sizeof(CfaAviConfigInfo))) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA_FLV] %s line %d failed in mm_copy_from_user\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
		}
	} else {
		mm_memcpy(&rCfaAviConfigInfo,
			pvParam, sizeof(CfaAviConfigInfo));
	}
	prCfaAviConfigInfo = &rCfaAviConfigInfo;
	
	if (NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: pvPrivData is NULL!\r\n"));
		if (!fgIsUserMem) {
			DMX_FreeMemory(pvParam);
		}
		MM_RETURN(RET_DMX_NO_MEM);
	}
	prCfaAviInst = (CfaAviInst *)pvPrivData;
	prCfaAviVInf = &prCfaAviInst->rCfaAviVInf;
	
	MMATE_CHECK_POINTER(prCfaAviInst);
	MMATE_CHECK_STRUCT(prCfaAviInst->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaAviInst->rCfaAviCurPosiInfo);
	MMATE_CHECK_STRUCT(prCfaAviInst->rVbrGarbageInf);
	MMATE_CHECK_STRUCT(prCfaAviInst->rFifoInfo);

	/* video related info from playback */
	prCfaAviInst->eVidType = eCfaGetMapVidType(prCfaAviConfigInfo->eVidCodec);
	prCfaAviVInf->tStrmInf.eStrmType = prCfaAviConfigInfo->eVidStrmType;
	prCfaAviVInf->tStrmInf.u4StrmIdx = prCfaAviConfigInfo->u4VidStrmIdx;
	prCfaAviVInf->tStrmInf.u4Scale = prCfaAviConfigInfo->u4VidScale;
	prCfaAviVInf->tStrmInf.u4Rate = prCfaAviConfigInfo->u4VidRate;
	prCfaAviVInf->eCodec = prCfaAviConfigInfo->eVidCodec;

	#if AVI_SUPPOTR_DRM
	if (prCfaAviConfigInfo->rDrmInfo.fgHasDrm)
		prCfaAviInst->rCfaAviDRMInf.fgStrdExist = TRUE;
	else
		prCfaAviInst->rCfaAviDRMInf.fgStrdExist = FALSE;

	#endif

	#if CONFIG_CFA_AVI_NEW_SYNC_PB_BUF_FLOW
	/*Used new sync pb buffer flow only case too large frame rate*/
	prCfaAviInst->fgNeedUseNewSyncFlow = prCfaAviConfigInfo->fgTooHighFrameRate;

	prCfaAviInst->u8LastCa = 0;
	prCfaAviInst->u4RemainDataInPbBuf = 0;
	prCfaAviInst->fgCanUseNewSyncFlow = FALSE;
	prCfaAviInst->fgNeedAdjustAfterTxDone = FALSE;
	#endif


	/* audio related info from playback */
	#if CONFIG_CFA_AVI_DIVIDE_LARGE_AUD_CHUNK
	prCfaAviInst->u4MaxAudChunkDuration = 1;/*CFA_AVI_AUD_CHUNK_MAX_DURATION * 2; // 1s*/
	#else
	prCfaAviInst->u4MaxAudChunkDuration = prCfaAviConfigInfo->u4MaxAudChunkDuration;
	#endif

	prCfaAviVInf->tStrmInf.u1AvcPayloadLenFieldSz = 0;
	prCfaAviVInf->tStrmInf.fgAdvanceAvc = FALSE;
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA_AVI] %s line %d -- eVidCodec: %d, pu1VidCodecSpecData: 0x%x, Len: %d\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		prCfaAviConfigInfo->pu1VidCodecSpecData,
		prCfaAviConfigInfo->u4VidCodecSpecDataLen);
	if ((AVCODEC_ID_H264 == prCfaAviConfigInfo->eVidCodec) ||
		(AVCODEC_ID_H265 == prCfaAviConfigInfo->eVidCodec))
	{
		if (0 != prCfaAviConfigInfo->u4VidPPSSPSHeaderDataLen) {
			if (prCfaAviVInf->tStrmInf.rSpecH264H265Info.pu1CodecSpecData != NULL) {
				DMX_FreeHwMemory(prCfaAviVInf->tStrmInf.rSpecH264H265Info.pu1CodecSpecData);
				prCfaAviVInf->tStrmInf.rSpecH264H265Info.pu1CodecSpecData = NULL;
			}
			DMX_NewHwMemory(prCfaAviConfigInfo->u4VidPPSSPSHeaderDataLen * sizeof(UCHAR), prCfaAviVInf->tStrmInf.rSpecH264H265Info.pu1CodecSpecData);
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI] line %d, H264SPSSPS Header len = %d"),
				DMX_LINE_NO,
				prCfaAviConfigInfo->u4VidPPSSPSHeaderDataLen);

			if( NULL == prCfaAviVInf->tStrmInf.rSpecH264H265Info.pu1CodecSpecData) {
				DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
					TEXT("[CFA_AVI] Alloc prCfaAviVInf->tStrmInf.rSpecInfo.pu1CodecSpecData memory fail\r\n"));
				if (!fgIsUserMem) {
					DMX_FreeMemory(pvParam);
				}
				MM_RETURN(RET_DMX_NO_MEM);
			}

			for (i = 0; i < prCfaAviVInf->tStrmInf.rSpecH264H265Info.u4CodecSpecDataLen ; i++) {
				DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
					TEXT("[cfa AVI]%s, AVI H264: Data[%d] = 0x%x. \r\n"), DMX_FUNC_NAME,i,
					*((CHAR *)prCfaAviConfigInfo->pu1VidPPSSPSHeaderData + i));
			}
			prCfaAviVInf->tStrmInf.rSpecH264H265Info.u4CodecSpecDataLen = prCfaAviConfigInfo->u4VidPPSSPSHeaderDataLen;
			dmx_memset(prCfaAviVInf->tStrmInf.rSpecH264H265Info.pu1CodecSpecData, 0, prCfaAviConfigInfo->u4VidPPSSPSHeaderDataLen);
			if (fgIsUserMem) {
				mm_copy_from_user(prCfaAviVInf->tStrmInf.rSpecH264H265Info.pu1CodecSpecData,
					prCfaAviConfigInfo->pu1VidPPSSPSHeaderData,
					prCfaAviVInf->tStrmInf.rSpecH264H265Info.u4CodecSpecDataLen);
			} else {
				dmx_memcpy(prCfaAviVInf->tStrmInf.rSpecH264H265Info.pu1CodecSpecData,
					prCfaAviConfigInfo->pu1VidPPSSPSHeaderData,
					prCfaAviVInf->tStrmInf.rSpecH264H265Info.u4CodecSpecDataLen);
			}
			prCfaAviVInf->tStrmInf.rSpecH264H265Info.u8VidFirstFrameOfst = prCfaAviConfigInfo->u8FirstVideoFrameOft;
			for (  i = 0; i < prCfaAviVInf->tStrmInf.rSpecH264H265Info.u4CodecSpecDataLen ; i++) {
				DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
					TEXT("[cfa AVI]%s, AVI H264: Data = 0x%x. \r\n"), DMX_FUNC_NAME,
					*((CHAR *)prCfaAviVInf->tStrmInf.rSpecH264H265Info.pu1CodecSpecData + i));
			}
		}
	}
	if (AVCODEC_ID_H264 == prCfaAviConfigInfo->eVidCodec) {
		if (NULL != prCfaAviConfigInfo->pu1VidCodecSpecData) {
			prCfaAviVInf->tStrmInf.fgSetAudHdr = TRUE;
			prCfaAviVInf->tStrmInf.rSpecInfo.pu1CodecSpecData = NULL;
			if (0 == prCfaAviConfigInfo->u4VidCodecSpecDataLen) {
				DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
					TEXT("[CFA_AVI] %s line %d fail for u4VidCodecSpecDataLen is 0\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				if (!fgIsUserMem) {
					DMX_FreeMemory(pvParam);
				}
				MM_RETURN(RET_DMX_PARAM_WRONG);
		  }
			DMX_NewHwMemory(prCfaAviConfigInfo->u4VidCodecSpecDataLen * sizeof(u8),
								prCfaAviVInf->tStrmInf.rSpecInfo.pu1CodecSpecData);
			if (NULL == prCfaAviVInf->tStrmInf.rSpecInfo.pu1CodecSpecData) {
				DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
					TEXT("[CFA_AVI] Alloc prCfaAviVInf->tStrmInf.rSpecInfo.")
					TEXT("pu1CodecSpecData memory fail\r\n"));
				if (!fgIsUserMem) {
					DMX_FreeMemory(pvParam);
				}
				MM_RETURN(RET_DMX_NO_MEM);
			}
			prCfaAviVInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen = prCfaAviConfigInfo->u4VidCodecSpecDataLen;
			dmx_memset(prCfaAviVInf->tStrmInf.rSpecInfo.pu1CodecSpecData, 0,
				prCfaAviVInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen);
			if (fgIsUserMem) {
				mm_copy_from_user(prCfaAviVInf->tStrmInf.rSpecInfo.pu1CodecSpecData,
					prCfaAviConfigInfo->pu1VidCodecSpecData,
					prCfaAviVInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen);
			} else {
				dmx_memcpy(prCfaAviVInf->tStrmInf.rSpecInfo.pu1CodecSpecData,
					prCfaAviConfigInfo->pu1VidCodecSpecData,
					prCfaAviVInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen);
			}
			prCfaAviVInf->tStrmInf.u1AvcPayloadLenFieldSz = prCfaAviConfigInfo->u1AvcPayloadLenFieldSz;
			if ((0 < prCfaAviVInf->tStrmInf.u1AvcPayloadLenFieldSz) &&
				(prCfaAviVInf->tStrmInf.u1AvcPayloadLenFieldSz <= 4)) {
				prCfaAviVInf->tStrmInf.fgAdvanceAvc = TRUE;
				DMX_NewHwMemory(3 * sizeof(u8), prCfaAviVInf->tStrmInf.pu1AvcPayloadHdr);
				if (NULL == prCfaAviVInf->tStrmInf.pu1AvcPayloadHdr) {
					DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
						TEXT("[CFA_AVI] Alloc prCfaAviVInf->tStrmInf.")
						TEXT("pu1AvcPayloadHdr memory fail\r\n"));
					if (!fgIsUserMem) {
						DMX_FreeMemory(pvParam);
					}
					MM_RETURN(RET_DMX_NO_MEM);
				}
				prCfaAviVInf->tStrmInf.pu1AvcPayloadHdr[0] = 0x00;
				prCfaAviVInf->tStrmInf.pu1AvcPayloadHdr[1] = 0x00;
				prCfaAviVInf->tStrmInf.pu1AvcPayloadHdr[2] = 0x01;
			}
			DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA_AVI] %s line %d -- Advanced H264, FileHdrLen: %d\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				prCfaAviVInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen);
		} 
	} else if (NULL != prCfaAviConfigInfo->pu1VidCodecSpecData) {
		prCfaAviVInf->tStrmInf.fgSetAudHdr = TRUE;
		prCfaAviVInf->tStrmInf.rSpecInfo.pu1CodecSpecData = NULL;
		DMX_NewHwMemory(prCfaAviConfigInfo->u4VidCodecSpecDataLen * sizeof(u8),
						prCfaAviVInf->tStrmInf.rSpecInfo.pu1CodecSpecData);

		if (NULL == prCfaAviVInf->tStrmInf.rSpecInfo.pu1CodecSpecData) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA_AVI] Alloc prCfaAviVInf->tStrmInf.rSpecInfo.")
				TEXT("pu1CodecSpecData memory fail\r\n"));
			if (!fgIsUserMem) {
				DMX_FreeMemory(pvParam);
			}
			MM_RETURN(RET_DMX_NO_MEM);
		}
		prCfaAviVInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen = prCfaAviConfigInfo->u4VidCodecSpecDataLen;
		dmx_memset(prCfaAviVInf->tStrmInf.rSpecInfo.pu1CodecSpecData, 0,
			prCfaAviVInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen);
		if (fgIsUserMem) {
			mm_copy_from_user(prCfaAviVInf->tStrmInf.rSpecInfo.pu1CodecSpecData,
				prCfaAviConfigInfo->pu1VidCodecSpecData,
				prCfaAviVInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen);
		} else {
			dmx_memcpy(prCfaAviVInf->tStrmInf.rSpecInfo.pu1CodecSpecData,
				prCfaAviConfigInfo->pu1VidCodecSpecData,
				prCfaAviVInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen);
		}
	} else {
		prCfaAviVInf->tStrmInf.fgSetAudHdr = FALSE;
		prCfaAviVInf->tStrmInf.fgSetAccfileAudHdr = FALSE;
		dmx_memset(&(prCfaAviVInf->tStrmInf.rSpecInfo), 0, sizeof(TCfaAVICodecSpecInfo));
	}

	if (NULL != prCfaAviConfigInfo->pu1VidMPEG1CodecSpecData) {
		prCfaAviVInf->tStrmInf.rSpecMPEG1Info.pu1CodecSpecData = NULL;
		prCfaAviVInf->tStrmInf.rSpecMPEG1Info.u4CodecSpecDataLen = 0;
		prCfaAviVInf->tStrmInf.u8SpecMPEG1InfoDataOfst = 0;
		prCfaAviInst->rCfaAviVInf.tStrmInf.fgSpecMPEG1InfoTxed = FALSE;

		DMX_NewHwMemory(prCfaAviConfigInfo->u4VidMPEG1CodecSpecDataLen * sizeof(u8),
			prCfaAviVInf->tStrmInf.rSpecMPEG1Info.pu1CodecSpecData);

		if (NULL == prCfaAviVInf->tStrmInf.rSpecMPEG1Info.pu1CodecSpecData) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA_AVI] Alloc prCfaAviVInf->tStrmInf.rSpecMPEG1Info.")
				TEXT("pu1CodecSpecData memory fail\r\n"));
			if (!fgIsUserMem) {
				DMX_FreeMemory(pvParam);
			}
			MM_RETURN(RET_DMX_NO_MEM);
		}

		prCfaAviVInf->tStrmInf.rSpecMPEG1Info.u4CodecSpecDataLen =
			prCfaAviConfigInfo->u4VidMPEG1CodecSpecDataLen;
		prCfaAviVInf->tStrmInf.u8SpecMPEG1InfoDataOfst = prCfaAviConfigInfo->u8VidMPEG1CodecSpecDataOfst;

		dmx_memset(prCfaAviVInf->tStrmInf.rSpecMPEG1Info.pu1CodecSpecData, 0,
					prCfaAviVInf->tStrmInf.rSpecMPEG1Info.u4CodecSpecDataLen);
		if (fgIsUserMem) {
			mm_copy_from_user(prCfaAviVInf->tStrmInf.rSpecMPEG1Info.pu1CodecSpecData,
						prCfaAviConfigInfo->pu1VidMPEG1CodecSpecData,
						prCfaAviVInf->tStrmInf.rSpecMPEG1Info.u4CodecSpecDataLen);
		} else {
			dmx_memcpy(prCfaAviVInf->tStrmInf.rSpecMPEG1Info.pu1CodecSpecData,
						prCfaAviConfigInfo->pu1VidMPEG1CodecSpecData,
						prCfaAviVInf->tStrmInf.rSpecMPEG1Info.u4CodecSpecDataLen);
		}
	} else {
		prCfaAviVInf->tStrmInf.rSpecMPEG1Info.pu1CodecSpecData = NULL;
		prCfaAviVInf->tStrmInf.rSpecMPEG1Info.u4CodecSpecDataLen = 0;
		prCfaAviVInf->tStrmInf.u8SpecMPEG1InfoDataOfst = 0;
		prCfaAviInst->rCfaAviVInf.tStrmInf.fgSpecMPEG1InfoTxed = TRUE;
	}

	if (NULL != prCfaAviConfigInfo->pu1VidMPEG4VolData) {
		prCfaAviVInf->tStrmInf.rSpecMPEG4Info.pu1CodecSpecData = NULL;
		prCfaAviVInf->tStrmInf.rSpecMPEG4Info.u4CodecSpecDataLen = 0;
		prCfaAviVInf->tStrmInf.u8SpecMPEG4InfoDataOfst = 0;
		prCfaAviInst->rCfaAviVInf.tStrmInf.fgSpecMPEG4InfoTxed = FALSE;

		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA_AVI] AVI MPEG4: pu1VidMPEG4VolData not NULL\r\n"));

		DMX_NewHwMemory(prCfaAviConfigInfo->u4VidMPEG4VolDataLen * sizeof(u8),
			prCfaAviVInf->tStrmInf.rSpecMPEG4Info.pu1CodecSpecData);

		if (NULL == prCfaAviVInf->tStrmInf.rSpecMPEG4Info.pu1CodecSpecData) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA_AVI] Alloc prCfaAviVInf->tStrmInf.rSpecMPEG4Info.")
				TEXT("pu1CodecSpecData memory fail\n"));
			if (!fgIsUserMem) {
				DMX_FreeMemory(pvParam);
			}
			MM_RETURN(RET_DMX_NO_MEM);
		}

		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA_AVI] AVI MPEG4: pu1VidMPEG4VolData capy data\r\n"));

		prCfaAviVInf->tStrmInf.rSpecMPEG4Info.u4CodecSpecDataLen = prCfaAviConfigInfo->u4VidMPEG4VolDataLen;
		prCfaAviVInf->tStrmInf.u8SpecMPEG4InfoDataOfst = prCfaAviConfigInfo->u8VidMPEG4VolDataOffset;

		dmx_memset(prCfaAviVInf->tStrmInf.rSpecMPEG4Info.pu1CodecSpecData, 0,
					prCfaAviVInf->tStrmInf.rSpecMPEG4Info.u4CodecSpecDataLen);
		if (fgIsUserMem) {
			mm_copy_from_user(prCfaAviVInf->tStrmInf.rSpecMPEG4Info.pu1CodecSpecData,
							prCfaAviConfigInfo->pu1VidMPEG4VolData,
							prCfaAviVInf->tStrmInf.rSpecMPEG4Info.u4CodecSpecDataLen);
		} else {
			dmx_memcpy(prCfaAviVInf->tStrmInf.rSpecMPEG4Info.pu1CodecSpecData,
						prCfaAviConfigInfo->pu1VidMPEG4VolData,
						prCfaAviVInf->tStrmInf.rSpecMPEG4Info.u4CodecSpecDataLen);
		}
	} else {

		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA_AVI] AVI MPEG4: pu1VidMPEG4VolData is NULL\r\n"));

		prCfaAviVInf->tStrmInf.rSpecMPEG4Info.pu1CodecSpecData = NULL;
		prCfaAviVInf->tStrmInf.rSpecMPEG4Info.u4CodecSpecDataLen = 0;
		prCfaAviVInf->tStrmInf.u8SpecMPEG4InfoDataOfst = 0;
		prCfaAviInst->rCfaAviVInf.tStrmInf.fgSpecMPEG4InfoTxed = TRUE;
	}

	dmx_memcpy(&(prCfaAviInst->rVbrMp3GarbageInfo),
				&(prCfaAviConfigInfo->rVbrMp3GarbageInfo), sizeof(CfaAviVbrMp3GarbageInfo));
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA_AVI][conf]rVbrMp3GarbageInfo:%d u8GarbageDataEndOffset:0x%llx")
		TEXT("u8GarbageDataDuration:0x%llx\r\n"),
		prCfaAviInst->rVbrMp3GarbageInfo.u4GarbageDataChkNum,
		prCfaAviInst->rVbrMp3GarbageInfo.u8GarbageDataEndOffset,
		prCfaAviInst->rVbrMp3GarbageInfo.u8GarbageDataDuration);

	for (u4Idx = 0; u4Idx < MAX_NS_AVI_AUD; u4Idx++) {
		unsigned char *pucSize = NULL;

		prCfaAviAInf = &prCfaAviInst->rCfaAviAInf[u4Idx];
		prCfaAviAInf->tStrmInf.eAudCodec =
			CfaAviGetAudType(prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].eAudCodec);
		prCfaAviAInf->tStrmInf.eStrmType = prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].eAudStrmType;
		if ((((u32)0) == u4Idx) &&
			(CFA_AUD_DRV_FMT_MP3 == prCfaAviAInf->tStrmInf.eAudCodec) &&
			(CFA_AVI_AST_VBRA == prCfaAviAInf->tStrmInf.eStrmType)) {
			prCfaAviInst->fgDeteGabageData = TRUE;
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA AVI]:config, prCfaAviInst->fgDeteGabageData = %d\r\n"),
				prCfaAviInst->fgDeteGabageData);
		}
		prCfaAviAInf->tStrmInf.u4StrmIdx = prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].u4AudStrmIdx;
		prCfaAviAInf->tStrmInf.u4Scale = prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].u4AudScale;
		prCfaAviAInf->tStrmInf.u4Rate = prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].u4AudRate;
		prCfaAviAInf->tStrmInf.u4Bps = prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].u4AudBps;
		prCfaAviAInf->tStrmInf.u2AudBitsPerSample =
			prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].u2AudBitsPerSample;

		prCfaAviAInf->tSamInf.u2BlockAlign = prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].u2AudBlockAlign;

		prCfaAviAInf->tStrmInf.fgSetAudHdr = FALSE;
		prCfaAviAInf->tStrmInf.fgSetAccfileAudHdr = FALSE;
		dmx_memset(&(prCfaAviAInf->tStrmInf.rSpecInfo), 0, sizeof(TCfaAVICodecSpecInfo));
		dmx_memset(&(prCfaAviAInf->tStrmInf.rAccFileSpecInfo), 0, sizeof(TCfaAVICodecAccFileSpecInfo));

		/* for Vorbis Audio*/
		switch (prCfaAviAInf->tStrmInf.eAudCodec) {
		case CFA_AUD_DRV_FMT_VORBIS:
			prCfaAviAInf->tStrmInf.fgSetAudHdr = TRUE;
			prCfaAviAInf->tStrmInf.rSpecInfo.pu1CodecSpecData = NULL;
			DMX_NewHwMemory(prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].u4AudCodecSpecDataLen,
							prCfaAviAInf->tStrmInf.rSpecInfo.pu1CodecSpecData);

		   if (NULL == prCfaAviAInf->tStrmInf.rSpecInfo.pu1CodecSpecData) {
				DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
					TEXT("[CFA_AVI] Alloc prCfaAviAInf->tStrmInf.")
					TEXT("rSpecInfo.pu1CodecSpecData memory fail\r\n"));
				if (!fgIsUserMem) {
					DMX_FreeMemory(pvParam);
				}
				MM_RETURN(RET_DMX_NO_MEM);
			}
			prCfaAviAInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen =
				prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].u4AudCodecSpecDataLen;
			if (fgIsUserMem) {
				mm_copy_from_user(prCfaAviAInf->tStrmInf.rSpecInfo.pu1CodecSpecData,
					prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].pucAudCodecSpecData,
					prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].u4AudCodecSpecDataLen);
			} else {
				dmx_memcpy(prCfaAviAInf->tStrmInf.rSpecInfo.pu1CodecSpecData,
					prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].pucAudCodecSpecData,
					prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].u4AudCodecSpecDataLen);
			}
			break;
		case CFA_AUD_DRV_FMT_AAC:
			if (AVCODEC_ID_AAC_PURE == prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].eAudCodec)
				break;
			prCfaAviAInf->tStrmInf.fgSetAccfileAudHdr = TRUE;
			prCfaAviAInf->tStrmInf.rAccFileSpecInfo.pu1CodecAccFileSpecData = NULL;

			prCfaAviAInf->tStrmInf.rAccFileSpecInfo.u4CodecAccFileSpecDataLen =
				prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].u4AudCodecSpecDataLen;

			pucSize = (unsigned char *)&(prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].u4AudCodecSpecDataLen);

			DMX_NewHwMemory(prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].u4AudCodecSpecDataLen + 8,
					prCfaAviAInf->tStrmInf.rAccFileSpecInfo.pu1CodecAccFileSpecData);

			if (NULL == prCfaAviAInf->tStrmInf.rAccFileSpecInfo.pu1CodecAccFileSpecData) {
				DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
					TEXT("[CFA_AVI] Alloc prCfaAviAInf->tStrmInf.rAccFileSpecInfo.")
					TEXT("pu1CodecAccFileSpecData memory fail\r\n"));
				if (!fgIsUserMem) {
					DMX_FreeMemory(pvParam);
				}
				MM_RETURN(RET_DMX_NO_MEM);
			}

			prCfaAviAInf->tStrmInf.rAccFileSpecInfo.pu1CodecAccFileSpecData[0] = pucSize[3];
			prCfaAviAInf->tStrmInf.rAccFileSpecInfo.pu1CodecAccFileSpecData[1] = pucSize[2];
			prCfaAviAInf->tStrmInf.rAccFileSpecInfo.pu1CodecAccFileSpecData[2] = pucSize[1];
			prCfaAviAInf->tStrmInf.rAccFileSpecInfo.pu1CodecAccFileSpecData[3] = pucSize[0];
			if (fgIsUserMem) {
				mm_copy_from_user(prCfaAviAInf->tStrmInf.rAccFileSpecInfo.pu1CodecAccFileSpecData + 4,
					 prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].pucAudCodecSpecData,
					 prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].u4AudCodecSpecDataLen);
			} else {
				dmx_memcpy(prCfaAviAInf->tStrmInf.rAccFileSpecInfo.pu1CodecAccFileSpecData + 4,
					 prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].pucAudCodecSpecData,
					 prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].u4AudCodecSpecDataLen);
			}
			prCfaAviAInf->tStrmInf.rAccFileSpecInfo.u4CodecAccFileSpecDataLen =
				prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].u4AudCodecSpecDataLen + 4;

			/*dmx_memcpy(prCfaAviAInf->tStrmInf.rAccFileSpecInfo.pu1CodecAccFileSpecData,*/
			/* prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].pucAudCodecSpecData,
			prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].u4AudCodecSpecDataLen);*/
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA_AVI] AAC u4CodecAccFileSpecDataLen: %d\r\n"),
				prCfaAviAInf->tStrmInf.rAccFileSpecInfo.u4CodecAccFileSpecDataLen);

			prCfaAviAInf->tStrmInf.u4SampleRate = prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].u4SampleRate;
			prCfaAviAInf->tStrmInf.u2Channels = prCfaAviConfigInfo->rCfaAviAudInfo[u4Idx].u2Channel;
			prCfaAviAInf->tStrmInf.fgSetAudHdr = TRUE;
			prCfaAviAInf->tStrmInf.rSpecInfo.pu1CodecSpecData = NULL;
			DMX_NewHwMemory(8 * sizeof(u8), prCfaAviAInf->tStrmInf.rSpecInfo.pu1CodecSpecData);
			if (NULL == prCfaAviAInf->tStrmInf.rSpecInfo.pu1CodecSpecData) {
				DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
					TEXT("[CFA_AVI] Alloc prCfaAviAInf->tStrmInf.")
					TEXT("rSpecInfo.pu1CodecSpecData memory fail\r\n"));
				if (!fgIsUserMem) {
					DMX_FreeMemory(pvParam);
				}
				MM_RETURN(RET_DMX_NO_MEM);
			}
			prCfaAviAInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen = 7;
			dmx_memset(prCfaAviAInf->tStrmInf.rSpecInfo.pu1CodecSpecData, 0,
				prCfaAviAInf->tStrmInf.rSpecInfo.u4CodecSpecDataLen);
			DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
				TEXT("[CFA_AVI] AAC Audio SamplingRate: %d, Channels: %d\r\n"),
				prCfaAviAInf->tStrmInf.u4SampleRate, prCfaAviAInf->tStrmInf.u2Channels);
			break;
		default:
			break;
		}

		#if CONFIG_CFA_AVI_DIVIDE_LARGE_AUD_CHUNK
		prCfaAviInst->u4CurAudDividingSize[u4Idx] = (u32)(prCfaAviAInf->tStrmInf.u4Bps * CFA_AVI_AUD_CHUNK_MAX_DURATION_N / CFA_AVI_AUD_CHUNK_MAX_DURATION_D);
		{
		u32 u4AudDivideAlign = prCfaAviAInf->tStrmInf.u4Bps; /*0xFFFFFFFF;*/

		if (0 != prCfaAviAInf->tStrmInf.u2AudBitsPerSample) {
			if (CFA_AVI_AUD_BYTE_PER_SAMPLE <
				(prCfaAviAInf->tStrmInf.u2AudBitsPerSample * 8)) {
				/* Use u2AudBitsPerSample*/
				u4AudDivideAlign = (prCfaAviAInf->tStrmInf.u2AudBitsPerSample * 8);
			} else {
				/* Use default setting*/
				u4AudDivideAlign = CFA_AVI_AUD_BYTE_PER_SAMPLE;
			}
		}

		if (u4AudDivideAlign < prCfaAviAInf->tSamInf.u2BlockAlign) {
			/* Use block align*/
			u4AudDivideAlign = prCfaAviAInf->tSamInf.u2BlockAlign;
		}

		if ((prCfaAviInst->u4CurAudDividingSize[u4Idx] > u4AudDivideAlign) &&
			(0 != u4AudDivideAlign)) {
			prCfaAviInst->u4CurAudDividingSize[u4Idx] -=
			(prCfaAviInst->u4CurAudDividingSize[u4Idx] % u4AudDivideAlign);
		}

		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]:i4CfaAviConfigure, audio %d, dividing size:%d, align:%d\r\n"),
			u4Idx, prCfaAviInst->u4CurAudDividingSize[u4Idx], u4AudDivideAlign);
		}
		#endif
	}

	#if CONFIG_CFA_AVI_NEW_METHOD_FOR_ERR_JUMP
		prCfaAviInst->fgIsFileHasIndex = prCfaAviConfigInfo->fgIsFileHasIndex;
	#endif

	/* subtitle related info from playback */
	#if CONFIG_CFA_AVI_TX_ALL_SP
	for (u4Idx = 0; u4Idx < MAX_NS_AVI_INTERNAL_SP; u4Idx++) {
		TCfaAviStrmInf *prCfaAviSpStrmInf = NULL;

		prCfaAviSpStrmInf = &(prCfaAviInst->rCfaAviSpInf[u4Idx].tStrmInf);

		prCfaAviSpStrmInf->eStrmType = prCfaAviConfigInfo->rCfaAviSpInfo[u4Idx].eSpStrmType;
		prCfaAviSpStrmInf->u4StrmIdx = prCfaAviConfigInfo->rCfaAviSpInfo[u4Idx].u4SpStrmIdx;
		prCfaAviSpStrmInf->u4Scale = prCfaAviConfigInfo->rCfaAviSpInfo[u4Idx].u4SpScale;
		prCfaAviSpStrmInf->u4Rate = prCfaAviConfigInfo->rCfaAviSpInfo[u4Idx].u4SpRate;
		/*prCfaAviSpStrmInf->u4Bps = prCfaAviConfigInfo->rCfaAviSpInfo[u4Idx].u4SpBps;*/
	}
	#else
	prCfaAviInst->rCfaAviSpStrmInf.u4StrmIdx = prCfaAviConfigInfo->u4SpStrmIdx;
	#endif

	prCfaAviInst->u8VidCodecSpecDataOfst = prCfaAviConfigInfo->u8VidCodecSpecDataOfst;
	prCfaAviInst->u4VidCodecSpecDataLen = prCfaAviConfigInfo->u4VidCodecSpecDataLen;

	#if CONFIG_CFA_AVI_SUPPORT_H26_NEW_METHOD /*mcn05027 for support H264 @2008/10/14*/
	prCfaAviInst->u4VOneFrmDuration = u8CfaAviChunk2Pts(&(prCfaAviVInf->tStrmInf), 1);

	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:i4CfaAviConfigure,  one video frame duration:0x%x\r\n"),
		prCfaAviInst->u4VOneFrmDuration);
	#endif

	#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
	dmx_memcpy(&(prCfaAviInst->rVbrGarbageInf), &(prCfaAviConfigInfo->rVbrGarbageInf),
				sizeof(TCfaAviVbrGarbageInf));
	#endif

	#if CONFIG_CFA_AVI_SUPPORT_AUDIO_DRM
	dmx_memcpy(&(prCfaAviInst->rAudCryptInfo), &(prCfaAviConfigInfo->rAudCryptInfo),
				sizeof(CfaAviAudCryptInfo));
	#endif

	#if CONFIG_CFA_AVI_SUPPORT_FIFO_LIMITATION
	dmx_memcpy(&(prCfaAviInst->rFifoInfo), &(prCfaAviConfigInfo->rFifoInfo),
				sizeof(CfaAviFifoInfo));
	#endif

	#if CONFIG_CFA_AVI_USE_THRESHOLD_TMP
	prCfaAviInst->fgUseThreshold = prCfaAviConfigInfo->fgUseThreshold;
	#endif
	if (!fgIsUserMem) {
		DMX_FreeMemory(pvParam);
	}
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON, TEXT("[CFA AVI]:vCfaAviConfigure\r\n"));

	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:VStrmType: 0x%x\r\n"), prCfaAviConfigInfo->eVidStrmType);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:VStrmIdx: 0x%lx\r\n"), prCfaAviConfigInfo->u4VidStrmIdx);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:VScale: 0x%lx\r\n"), prCfaAviConfigInfo->u4VidScale);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:VRate: 0x%lx\r\n"), prCfaAviConfigInfo->u4VidRate);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:VCodec: 0x%x\r\n"), (AVCODECID_T)prCfaAviConfigInfo->eVidCodec);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:VCodec: 0x%x\r\n"), prCfaAviConfigInfo->eVidCodec);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:AStrmIdx: 0x%lx\r\n"), prCfaAviConfigInfo->rCfaAviAudInfo[0].u4AudStrmIdx);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:AScale: 0x%lx\r\n"), prCfaAviConfigInfo->rCfaAviAudInfo[0].u4AudScale);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:ARate: 0x%lx\r\n"), prCfaAviConfigInfo->rCfaAviAudInfo[0].u4AudRate);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:ABps: 0x%lx\r\n"), prCfaAviConfigInfo->rCfaAviAudInfo[0].u4AudBps);
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:ABlockAlign: 0x%x\r\n"), prCfaAviConfigInfo->rCfaAviAudInfo[0].u2AudBlockAlign);

	#if CONFIG_CFA_AVI_TX_ALL_SP

	#else
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:SpStrmIdx: 0x%lx\r\n"), prCfaAviConfigInfo->u4SpStrmIdx);
	#endif
	MMATE_CHECK_POINTER(prCfaAviInst);
	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
 * Name: CfaAvi_SetInqTypes
 *
 * Description:
 *		AVI CFA sets information query types
 *		splitter will ensure that it is only called in "off" or "paused" state.
 *
 * Inputs:
 *		[IN] uintptr_t of splitter
 *		[IN] information type for AVI CFA
 *		[IN] pointer to CfaAviInst
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAvi_SetInqTypes(void *pvSptHdl, u32 u4InfTypes, void *pvPrivData)
{
	CfaAviInst *prCfaAviInst = NULL;

	/*DMX_LogE(TEXT("[CFA AVI] vCfaAviSetInqTypes\r\n"));*/
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON, TEXT("[CFA AVI]: vCfaAviSetInqTypes\r\n"));

	if (NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: pvPrivData is NULL!\r\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}
	prCfaAviInst = (CfaAviInst *)pvPrivData;

	if (CFA_AVI_QUERY_TYPE_PARSING_MODE & u4InfTypes)
		prCfaAviInst->u4CfaQueryType |= CFA_AVI_QUERY_TYPE_PARSING_MODE;

	#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
	if (CFA_AVI_QUERY_TYPE_VBR_GARBAGE_DATA & u4InfTypes)
		prCfaAviInst->u4CfaQueryType |= CFA_AVI_QUERY_TYPE_VBR_GARBAGE_DATA;

	#endif

	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
 * Name: CfaAvi_GetGeneral
 *
 * Description:
 *		AVI CFA gets information
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/

static MRESULT CfaAvi_GetGeneral(void *pvSptHdl, u32 u4CfaFID, void *pvPrivData,
							 void *pvCfaParameter, u32 u4CfaParameterSize)
{
	CfaAviInst *prCfaAviInst = NULL;

	/*DMX_LogE(TEXT("[CFA AVI] i4CfaAviGetGeneral\r\n"));*/
	DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT, TEXT("[CFA AVI]: Entry i4CfaAviGetGeneral\r\n"));

	if (NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,TEXT("[CFA AVI]: pvPrivData is NULL!\r\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}
	prCfaAviInst = (CfaAviInst *)pvPrivData;

	MM_RETURN(RET_DMX_OK);
}

#if AVI_SUPPORT_SKIP_DATA_IN_TX_DATA
static MRESULT CfaAvi_SetGeneral(void *pvSptHdl, u32 u4CfaFID,
							   void *pvPrivData, void *pvCfaParameter,
							   u32 u4CfaParameterSize) {
	CfaAviInst *prCfaAviInst =	(CfaAviInst *)pvPrivData;
	CfaJumpInfoInTxData *prJumpInfo = (CfaJumpInfoInTxData *)pvCfaParameter;

	if (!pvPrivData || !pvCfaParameter) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: pvPrivData or pvCfaParameter is NULL!\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	MMATE_CHECK_POINTER(prCfaAviInst);

	if (CFA_GENERAL_JUMPINFO == u4CfaFID) {
		if (u4CfaParameterSize != sizeof(CfaJumpInfoInTxData)) {
        		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,TEXT("[CFA AVI]: param size is not equal !\r\n"));
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
			TEXT("[CFA AVI]CfaAvi_SetGeneral, u8VidValidOfst:0x%llx ")
			TEXT("u8AudValidOfst:0x%llx  u8SubValidOfst:0x%llx\r\n"),
			prJumpInfo->u8VidValidOfst,
			prJumpInfo->u8AudValidOfst,
			prJumpInfo->u8SubValidOfst);
		if (prJumpInfo->u4Flags & MASK_CFAJUMPINFO_VIDEO)
			prCfaAviInst->u8SkipVDataInTxData	= prJumpInfo->u8VidValidOfst;

		if (prJumpInfo->u4Flags & MASK_CFAJUMPINFO_AUDIO)
			prCfaAviInst->u8SkipADataInTxData	= prJumpInfo->u8AudValidOfst;

		if (prJumpInfo->u4Flags & MASK_CFAJUMPINFO_SUBTITLE)
			prCfaAviInst->u8SkipSpDataInTxData	= prJumpInfo->u8SubValidOfst;

		prCfaAviInst->fgFristVidBlk = FALSE;
		prCfaAviInst->fgFristAudBlk = FALSE;
	}

	DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("[CFA AVI]CfaAvi_SetGeneral, u8VidValidOfst:0x%llx ")
		TEXT("u8AudValidOfst:0x%llx  u8SubValidOfst:0x%llx\r\n"),
		prCfaAviInst->u8SkipVDataInTxData,
		prCfaAviInst->u8SkipADataInTxData,
		prCfaAviInst->u8SkipSpDataInTxData);

	MM_RETURN(RET_DMX_OK);
}
#endif

static MRESULT CfaAVITxAudHDRInfo(void *pvSptHdl, u32 u4TxUID, void *pvPrivData)
{
	CfaAviInst *prCfaAviInst = NULL;
	MRESULT mrRet = RET_DMX_OK;
	TCfaAviAInf *ptAInf = NULL;

	if (NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,TEXT("[CFA AVI]: pvPrivData is NULL!\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prCfaAviInst = (CfaAviInst *)pvPrivData;
	ptAInf = &(prCfaAviInst->rCfaAviAInf[prCfaAviInst->ucCurAudInfoIdx]);

	if (DMX_INVALID_UINT32 == u4TxUID)
		MM_RETURN(RET_DMX_PARAM_WRONG);


	if (CFA_AUD_DRV_FMT_AAC !=
		prCfaAviInst->rCfaAviAInf[prCfaAviInst->ucCurAudInfoIdx].tStrmInf.eAudCodec)
		MM_RETURN(RET_DMX_UNSUPPORT);

	if ((prCfaAviInst->u4CurAudStrmID != u4TxUID) ||
		(NULL ==
		ptAInf->tStrmInf.rAccFileSpecInfo.pu1CodecAccFileSpecData))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	mrRet = Spt4CfaBuf2AFifo(pvSptHdl, ptAInf->tStrmInf.rAccFileSpecInfo.pu1CodecAccFileSpecData,
			 ptAInf->tStrmInf.rAccFileSpecInfo.u4CodecAccFileSpecDataLen,
			 ptAInf->tStrmInf.u4StrmIdx, ptAInf->tStrmInf.eAudCodec);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI] CfaAVITxAudHDRInfo fail in Spt4CfaBuf2AFifo, ret: %d\r\n"), mrRet);
		CfaAviFinishPrs(pvSptHdl, prCfaAviInst, prCfaAviInst->u8Endoffst - 1);

		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

const char *getPicTypeStr(u32 u4VType)
{
	if (fgIsIType(u4VType))
		return "I";
	else if (fgIsBType(u4VType))
		return "B";
	else if (fgIsPType(u4VType))
		return "P";
	else
		return "U";
}

void CfaAvi_FillAUInfo_V(void *pvSptHdl, void *pvAUInfo, void *pvAUExtInfo, void *pvPrivData)
{
	CfaAviInst *prCfaAviInst = NULL;
	TCfaAviStrmInf *ptStrmInf = NULL;/*mtk40156 for clean warning*/

	if ((NULL == pvPrivData) || (NULL == pvAUInfo)) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: pvPrivData or pvAUInfo is NULL !\r\n"));
		return;
	}
	prCfaAviInst = (CfaAviInst *)pvPrivData;

	/*((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Offset =
	prCfaAviInst->u8Ca - (AVI_4CC_BYTES+AVI_SZ_BYTES);*/
	((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Dts = INVALID_TIMESTAMP;
	((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.eDiscType = DT_DATADISC;

	prCfaAviInst->u8PreVPts = prCfaAviInst->u8CurVPts;
	prCfaAviInst->u8CurVPts = prCfaAviInst->u8PrsPts;

	((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = prCfaAviInst->u8PrsPts;

	((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8PrevPTS = 0;

	if (fgIsIType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) {
		prCfaAviInst->u4CurIFrmChunkNo = prCfaAviInst->rCfaAviPrsStrmInf.u4VidPrsChunk;

		prCfaAviInst->u8CurIFrmChunkOfst = prCfaAviInst->u8Ca - AVI_GEN_READ_BYTES;
	}

	ptStrmInf = &(prCfaAviInst->rCfaAviVInf.tStrmInf);
	if (ptStrmInf->u4Rate != 0)  {

		((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4Duration =
			CfaAviConvert2Stc(ptStrmInf->u4Scale, ptStrmInf->u4Rate);
	} else
		((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4Duration = INVALID_DURATION;

#if CONFIG_CFA_AVI_SUPPORT_H26_NEW_METHOD
	((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4Duration = prCfaAviInst->u4VOneFrmDuration;

	if (prCfaAviInst->rCfaAviVInf.eCodec == AVCODEC_ID_H264) {
		if (fgIsIType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType))
			((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = prCfaAviInst->u8CurVPts;
		else
			((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = INVALID_TIMESTAMP;

		break;
	}
#endif

	if (prCfaAviInst->u4DataSz == 0) {
		if (CFA_PIC_P == prCfaAviInst->ePreValidPicType) {
			if (CFA_PIC_P == prCfaAviInst->ePrePicType) {
				/* Set previous P frame as invalid PTS*/
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8PrevPTS = INVALID_TIMESTAMP;
			}
			prCfaAviInst->u8CurVPts = DMX_INVALID_UINT64;
		}

		prCfaAviInst->ePrePicType = CFA_PIC_UNDEFINE;
	} else {
		if (fgIsIType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType) ||
			fgIsBType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType) ||
			fgIsPType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) {
			prCfaAviInst->u1ChunkVopNs++;
			prCfaAviInst->u1CurBGrpNum++;
		}

		if (prCfaAviInst->u1CurBGrpNum > 1)
			prCfaAviInst->u1TotalBGrpNum++;

		if (prCfaAviInst->rCfaAviVInf.eCodec != AVCODEC_ID_MPEG4) {
			/* In RMP4, the PTS should be set to the P-VOP */
			if (prCfaAviInst->u1ChunkVopNs == 2) {/* B-grouped */

				prCfaAviInst->u8CurVPts = DMX_INVALID_UINT64;

				prCfaAviInst->fgBGrouped = TRUE;
				prCfaAviInst->u1ChunkVopNs = 0;
			} else if (prCfaAviInst->u1ChunkVopNs == 1) {
				if (fgIsBType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) {
					DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_AUINFO, TEXT("[CFA AVI] CFA_PIC_B\r\n"));

					prCfaAviInst->u8CurVPts = INVALID_TIMESTAMP;

					((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Dts = prCfaAviInst->u8CurVPts;

					if ((prCfaAviInst->ePrePicType != CFA_PIC_B) &&
						(prCfaAviInst->ePrePicType != CFA_PIC_I)) {
						prCfaAviInst->u8PreVPts = DMX_INVALID_UINT64;
						((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8PrevPTS =
										INVALID_TIMESTAMP;
					}

					prCfaAviInst->u1ChunkVopNs = 0;
				}
			} else {
				/*do
				nothing*/
			}
		}

		if (fgIsIType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType))
			prCfaAviInst->ePrePicType = CFA_PIC_I;
		else if (fgIsBType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType))
			prCfaAviInst->ePrePicType = CFA_PIC_B;
		else if (fgIsPType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType))
			prCfaAviInst->ePrePicType = CFA_PIC_P;
		else
			prCfaAviInst->ePrePicType = CFA_PIC_UNDEFINE;
	}

	if (CFA_PIC_UNDEFINE != prCfaAviInst->ePrePicType)
		prCfaAviInst->ePreValidPicType = prCfaAviInst->ePrePicType;

	if (DMX_INVALID_UINT64 == prCfaAviInst->u8CurVPts)
		((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = INVALID_TIMESTAMP;
	else
		((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = prCfaAviInst->u8CurVPts;

#if CONFIG_CFA_AVI_PLAY_ONE_FRM_ONLY_TX_I_FRM
	if ((DMX_INVALID_UINT32 != prCfaAviInst->u4CurIFrmChunkNo) &&
		((prCfaAviInst->rCfaRange.u4VidStartChunkNo + VDEC_NEED_PIC_CNT_MIN) <=
		prCfaAviInst->rCfaAviPrsStrmInf.u4VidPrsChunk)) {
		if ((prCfaAviInst->fgPlayFrmFlag)
		#if CONFIG_CFA_AVI_USE_THRESHOLD_TMP
			&& (prCfaAviInst->fgUseThreshold)
		#endif
			)
			prCfaAviInst->fgNeedFinishIfTxDone = TRUE;
	}
#endif

#if 0
	DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
		TEXT("[CFA AVI] FILL_AU: idx:%d, VPts: %lldms, PicType: %s\r\n"),
		++_uIdx,
		((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts/90,
		getPicTypeStr(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType));
#endif

}

/*-----------------------------------------------------------------------------
 * Name: CfaAvi_FillAUInfo
 *
 * Description:
 *		AVI CFA sets AU table information
 *
 * Inputs:
 *		[IN] uintptr_t of Splitter
 *		[IN/OUT] AU info, @see Spt2CfaPicInfo
 *		[IN] pointer to CfaAviInst
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAvi_FillAUInfo(void *pvSptHdl, void *pvAUInfo, void *pvAUExtInfo, void *pvPrivData)
{
	CfaAviInst *prCfaAviInst = NULL;

	if ((NULL == pvPrivData) || (NULL == pvAUInfo)) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: pvPrivData or pvAUInfo is NULL !\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	prCfaAviInst = (CfaAviInst *)pvPrivData;
	MMATE_CHECK_POINTER(prCfaAviInst);
	MMATE_CHECK_STRUCT(prCfaAviInst->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaAviInst->rCfaAviCurPosiInfo);
	MMATE_CHECK_STRUCT(prCfaAviInst->rVbrGarbageInf);
	MMATE_CHECK_STRUCT(prCfaAviInst->rFifoInfo);

	/* Todo: if splitter also let cfa set au info in these states ???*/
	if ((prCfaAviInst->eCurCfaAviAnaSt == CFA_AVI_ANA_AVIPRS_TX_PARSING_MODE) ||
		(prCfaAviInst->eCurCfaAviAnaSt == CFA_AVI_ANA_AVIPRS_TX_EXTRA_DATA_DONE))
		MM_RETURN(RET_DMX_OK);

	switch (prCfaAviInst->eCurPrsPktType) {
	case CFA_AVI_PRS_BIT_STRM_TYPE_V: {
		CfaAvi_FillAUInfo_V(pvSptHdl, pvAUInfo, pvAUExtInfo, pvPrivData);
		break;
	}

	case CFA_AVI_PRS_BIT_STRM_TYPE_A: {
		if (!prCfaAviInst->fgFristAudBlk) {
			prCfaAviInst->fgFristAudBlk = TRUE;
			/*DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI]u8Ca: 0x%llx\n"), prCfaAviInst->u8Ca);*/
		}

		((AU_AUDIO *)pvAUInfo)->rAUInfo.rInfo.u8Pts = prCfaAviInst->u8PrsPts;
		/*DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:FILL_AU:APts:0x%llx, Wr:0x%lx, Ca:0x%llx\r\n"),
		prCfaAviInst->u8PrsPts,  (u32)pvAUInfo, prCfaAviInst->u8Ca);*/
		break;
	}

	case CFA_AVI_PRS_BIT_STRM_TYPE_SP0:

		((AU_SP *)pvAUInfo)->rAUInfo.rInfo.u8StartPts = prCfaAviInst->u8PrsPts;
		((AU_SP *)pvAUInfo)->rAUInfo.rInfo.u8EndPts = prCfaAviInst->u8SpEndPts;
		#if CONFIG_CFA_AVI_TX_ALL_SP
		if (prCfaAviInst->ucCurSpInfoIdx >= MAX_NS_AVI_INTERNAL_SP) {
			DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("[CFA AVI]: Sp Index is larger than 8 !\r\n"));
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
		/*DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:FILL_AU:SpTxed:0x%lx,StartPts:0x%llx, EndPts:0x%llx, Wr:0x%lx, Ca:0x%llx\r\n"),
		prCfaAviInst->rCfaAviSpInf[prCfaAviInst->ucCurSpInfoIdx].tStrmInf.u4TxedChunk, prCfaAviInst->u8PrsPts,
		((AU_SP *)pvAUInfo)->rAUInfo.rInfo.u8EndPts, (u32)pvAUInfo, prCfaAviInst->u8Ca);*/
		#else
		DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_AUINFO,
			TEXT("[CFA AVI]:FILL_AU:SpTxed:0x%lx,StartPts:0x%llx, EndPts:0x%llx, Wr:0x%lx, Ca:0x%llx\r\n"),
			prCfaAviInst->rCfaAviSpStrmInf.u4TxedChunk, prCfaAviInst->u8PrsPts,
			((AU_SP *)pvAUInfo)->rAUInfo.rInfo.u8EndPts, (u32)pvAUInfo, prCfaAviInst->u8Ca);
		#endif
		break;

	case CFA_AVI_PRS_BIT_STRM_TYPE_DRM:

		break;

	case CFA_AVI_PRS_BIT_STRM_TYPE_HDR:

		break;

/*-----------------------------------------------------------------------------
	case CFA_AVI_PRS_BIT_STRM_TYPE_SP0:
		((AU_SP *)pvAUInfo)->rAUInfo.rInfo.u8StartPts = prCfaMpgInst->u8PrsDts;
		((AU_SP *)pvAUInfo)->rAUInfo.rInfo.u8EndPts = DMX_INVALID_UINT64;
		((AU_SP *)pvAUInfo)->rAUInfo.rInfo.u8Dts = prCfaMpgInst->u8PrsDts;
		break;

	case CFA_MPG_PRS_BIT_STRM_TYPE_NV:
		break;
-----------------------------------------------------------------------------*/

	default:
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: parser stream is IDLE !\r\n"));
		break;
	}

	/* After fill AU, Pts/Dts reset  */
	prCfaAviInst->u8PrsPts = DMX_INVALID_UINT64;
	MM_RETURN(RET_DMX_OK);
}


#if CONFIG_CFA_AVI_NEW_SYNC_PB_BUF_FLOW
/*-----------------------------------------------------------------------------
 * Name: CfaAvi_Rebuf
 *
 * Description:
 *
 *
 * Inputs:
 *		[IN] uintptr_t of Splitter
 *		[IN] flag to indicate Rebuf
 *		[IN] pointer to CfaAviInst
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAvi_Rebuf(void *pvSptHdl, bool fgRebuf, void *pvPrivData)
{
	CfaAviInst *prCfaAviInst = NULL;

	/*DMX_LogE(TEXT("[CFA AVI] i4CfaAviRebuf, pvSptHdl:%p, fgRebuf:0x%x\r\n"), pvSptHdl,  fgRebuf);*/
	DmxLogD(DMX_MOD_CFA_AVI, CFA_AVI_LOG_COMMON,
		TEXT("[CFA AVI]:i4CfaAviRebuf, pvSptHdl:%p, fgRebuf:0x%x\r\n"), pvSptHdl, fgRebuf);

	if (NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
			TEXT("[CFA AVI]: pvPrivData is NULL !\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prCfaAviInst = (CfaAviInst *)pvPrivData;
	MMATE_CHECK_POINTER(prCfaAviInst);
	MMATE_CHECK_STRUCT(prCfaAviInst->rCfaRange);
	MMATE_CHECK_STRUCT(prCfaAviInst->rCfaAviCurPosiInfo);
	MMATE_CHECK_STRUCT(prCfaAviInst->rVbrGarbageInf);
	MMATE_CHECK_STRUCT(prCfaAviInst->rFifoInfo);

	if (fgRebuf) {
		prCfaAviInst->fgCanUseNewSyncFlow = FALSE;
		prCfaAviInst->u4RemainDataInPbBuf = 0;
		prCfaAviInst->u8LastCa = 0;
		prCfaAviInst->fgNeedAdjustAfterTxDone = FALSE;
	}

	MM_RETURN(RET_DMX_OK);
}
#endif

static MRESULT CfaAviProcCliCmd(void *pvSptHdl, E_DMX_CFA_CLI_TYPE_T eCliType, /*< [IN] Cfa Cli Command*/
				u32 arg1,
				u32 arg2, u32 arg3, const char *szParam, void *pvPrivData)
{
	MRESULT mrRet = RET_DMX_OK;
	CfaAviInst *prCfaAvi = NULL;

	prCfaAvi = (CfaAviInst *) pvPrivData;

	switch (eCliType) {
	case DMX_CFA_CLI_CMD_TURN_ONOFF_LOG:
		{
			BOOL fgEnable = TRUE;
		/**
		* arg1: u4OnOff
		* arg2: LogLevel(T, E, W, D)
		* arg3: Module Log Level
		**/
			if (0 == arg1)
				fgEnable = FALSE;

			DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("CfaAviProcCliCmd -- fgEnable: %d, Loglvl: %d, ModLogLvl: 0x%08x \r\n"),
				arg1, arg2, arg3);

			DmxLogEnable(fgEnable, arg2, DMX_MOD_CFA_AVI, arg3);
		}
		break;
	case DMX_CFA_CLI_CMD_DUMP_INFO:
		{
			DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("Cfa AVI Instance(uintptr_t is %p)")
				TEXT(" Info list as follow: \r\n"),
				prCfaAvi);
			DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("Current Analyse State is %d, Previous Analyse State is %d \r\n"),
				prCfaAvi->eCurCfaAviAnaSt, prCfaAvi->eCurCfaAviAnaSt);
			DmxLogT(DMX_MOD_CFA_AVI, CFA_AVI_LOG_DEFAULT,
				TEXT("Current Analyse Position is 0x%08x%08x, ")
				TEXT("Previous Analyse Position is 0x%08x%08x\r\n"),
				(u32) ((prCfaAvi->u8Ca) >> 32), (u32) (prCfaAvi->u8Ca),
				(u32) ((prCfaAvi->u8LastCa) >> 32), (u32) (prCfaAvi->u8LastCa));
		}
		break;
	default:
		break;
	}

	MM_RETURN(mrRet);
}

/* AVI CFA interface */
CfaIntf _rAviCfaIntf = {
	&CfaAvi_Init,
	&CfaAvi_Uninit,
	&CfaAvi_SetRange,
	&CfaAvi_EnableStrm,
	&CfaAvi_SetStrmInf,
	&CfaAvi_TurnOn,
	&CfaAvi_TxDone,
	&CfaAvi_GetCurPos,
	NULL,
	&CfaAvi_Configure,
	&CfaAvi_SetInqTypes,
	&CfaAvi_GetGeneral,
	#if AVI_SUPPORT_SKIP_DATA_IN_TX_DATA
	&CfaAvi_SetGeneral,
	#else
	NULL,
	#endif
	NULL,
	&CfaAvi_FillAUInfo,
	&CfaAVITxAudHDRInfo,
	#if CONFIG_CFA_AVI_NEW_SYNC_PB_BUF_FLOW
	&CfaAvi_Rebuf,
	#endif
	&CfaAvi_SetJumpRange,
	&CfaAviGetParamSize,
	&CfaAviProcCliCmd
	#ifdef CONFIG_COMPAT
	,&CfaAviProcCompat
	#endif
};


/*-----------------------------------------------------------------------------
 * Name: pvCfaAviGetInterface
 *
 * Description:
 *		Start of Public Function
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
void *CfaAviGetInterface(void)
{
	return ((void *)(&_rAviCfaIntf));
}

