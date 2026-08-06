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




#ifndef CFA_AVI_H
#define CFA_AVI_H

#include <media/atc/dmx_cfa_avi.h>

#include "dmx_spt_cfa.h"
#include "cfa_cfg.h"

/* C header file */
#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
					Macros, Typedefs, Enumerations
-----------------------------------------------------------------------------*/


#define  CONFIG_CHECK_TX_DATA_TIME					0
#define  CONFIG_CFA_ADJ_PTS							0

/* CFA AVI invalid stream number */
#define CFA_AVI_INVALID_STRM_ID						(0xff)

/* AVI CFA header buffer start address offset */
#define CFA_AVI_HDR_BUF_SA_OFST						(8)

/* CFA AVI header buffer size */
#define CFA_AVI_HDR_BUF_SZ							(64)

/* CFA AVI DRM temp buffer size */
#define CFA_AVI_DRM_TMP_BUF_SZ						(128)

#define VDEC_NEED_PIC_CNT_MIN						8

#define CFA_AVI_GARBAGE_SKIP_DATA_LEN			(100 * 1024)

#if CONFIG_CFA_AVI_DIVIDE_LARGE_AUD_CHUNK
#define CFA_AVI_AUD_CHUNK_MAX_DURATION_N	(2)
#define CFA_AVI_AUD_CHUNK_MAX_DURATION_D	(10)
#define CFA_AVI_AUD_BYTE_PER_SAMPLE (48)
#endif

#define AVI_4CC_BYTES								(4)

#define CFA_AVI_DRM_INIT							0
#define CFA_AVI_DRM_FINISH							1
#define CFA_AVI_DRM_STAGE1							2
#define CFA_AVI_DRM_OK								3

typedef enum {
	CFA_AVI_LOG_DEFAULT = (u32)1 << (u32)0,
	CFA_AVI_LOG_COMMON  = (u32)1 << (u32)1,
	CFA_AVI_LOG_AUINFO  = (u32)1 << (u32)2,
	CFA_AVI_LOG_STATE = (u32)1 << (u32)3
} CfaAviLogLvl_E;


#if CONFIG_CHECK_TX_DATA_TIME
typedef struct {
	u64 u4StartMs;
	u64 u4EndMs;
	bool fgStart;
} TCfaStatTime;
#endif

/* AVI CFA analyze state */
typedef enum CfaAviAnaSt {
	CFA_AVI_ANA_ST_IDLE				= (0x00),
	CFA_AVI_ANA_AVIPRS_INIT			= (0x01),
	CFA_AVI_ANA_AVIPRS_MOVI			= (0x02),
	CFA_AVI_ANA_AVIPRS_MOVI_LIST_ID	= (0x03),
	CFA_AVI_ANA_AVIPRS_MOVI_SRCH	 = (0x04),
	CFA_AVI_ANA_AVIPRS_PTX			 = (0x05),

	#if CONFIG_CFA_DIVX_DRM_SUPPORT
	CFA_AVI_ANA_AVIPRS_DRMINFO		= (0x06),
	CFA_AVI_ANA_AVIPRS_DRMTMPBUF	 = (0x07),
	CFA_AVI_ANA_AVIPRS_DRMTMPBUF_P	= (0x08),
	#endif

	#if CONFIG_CFA_DIVX_SP_SUPPORT
	CFA_AVI_ANA_AVIPRS_SUBT_DURATION = (0x09),
	#endif

	CFA_AVI_ANA_AVIPRS_TX_DIVX3_VCHUNK = (0x0C),

	CFA_AVI_ANA_AVIPRS_TX_PARSING_MODE = (0x0D),
	CFA_AVI_ANA_AVIPRS_TX_EXTRA_DATA = (0x0E),
	CFA_AVI_ANA_AVIPRS_TX_EXTRA_DATA_DONE = (0x0F),
	CFA_AVI_ANA_AVIPRS_START_CODE = (0x10),
	CFA_AVI_ANA_AVIPRS_TX_VC1_START_CODE = (0x11),

	#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
	CFA_AVI_ANA_AVIPRS_DETECT_VBR_GARBAGE_DATA = (0x12),
	#endif

	#if CONFIG_CFA_AVI_DIVIDE_LARGE_AUD_CHUNK
	CFA_AVI_ANA_AVIPRS_TX_DIVIDED_AUD = (0x13),
	#endif

	CFA_AVI_ANA_AVIPRS_TX_AUD_HDR = (0x14),
	CFA_AVI_ANA_AVIPRS_TX_VID_HDR = (0x15),

	CFA_AVI_ANA_AVIPRS_RE_TX_VID_MPEG1_CODEC_HEADER = (0x21),

	CFA_AVI_ANA_AVIPRS_RE_TX_VID_MPEG4_VOL_HEADER = (0x22),

	CFA_AVI_ANA_AVIPRS_AVC_SLICE_LEN = (0x31),
	CFA_AVI_ANA_AVIPRS_TX_AVC_START_CODE = (0x32),
	CFA_AVI_ANA_AVIPRS_TX_AVC_SLICE = (0x33),

	CFA_AVI_ANA_AVIPRS_TX_VP68_VCHUNK = (0x34),
	CFA_AVI_ANA_AVIPRS_RE_TX_VID_H264OrH265_HEADER = (0x35)
} CfaAviAnaSt;


/*--- from Mps_util.h */
#define CfaAviIsM4vCodec(arg) (((arg) == CFA_AVI_V_CODEC_DX4_M4V) || \
							   ((arg) == CFA_AVI_V_CODEC_DX5_M4V) || \
							   ((arg) == CFA_AVI_V_CODEC_RMP4_M4V) || \
							   ((arg) == CFA_AVI_V_CODEC_ISO_M4V))

#define CfaAviIsDivXM4v(arg) (((arg) == CFA_AVI_V_CODEC_DX4_M4V) || \
								((arg) == CFA_AVI_V_CODEC_DX5_M4V))

#define CfaAviIsDx4M4v(arg) ((arg) == CFA_AVI_V_CODEC_DX4_M4V)

#define CfaAviIsCbrAlloc(arg) ((arg) == CFA_AVI_AST_CBRA)

#define CfaAviIsVbrAlloc(arg) (CFA_AVI_AST_VBRA == (arg))

typedef struct {
	/*-----------------------------------------------------------------------------
	// CFA AVI ignore
	// u16 u2Channels;		  // number of channels (i.e. mono, stereo...)
	// u32 u4SamplesPerSec;	  // sample rate
	// u32 u4AvgBytesPerSec;
	// u16 u2BitsPerSample;	  // Number of bits per sample of mono data
	// u16 u2EncodeOpts;  // WMA: used as spec;	PCM: used to distinguish ADPCM
	-----------------------------------------------------------------------------*/

	/* [Playback Set] need to be set by playback before demux (vCfaAviPrsDmux) */
	u16 u2BlockAlign;
} TCfaASamplingInf;

typedef struct {
	u8 *pu1CodecSpecData;
	u32 u4CodecSpecDataLen;
	u64 u8VidFirstFrameOfst;
} TCfaAVICodecSpecInfo;

typedef struct {
	u8 *pu1CodecAccFileSpecData;
	u32 u4CodecAccFileSpecDataLen;
} TCfaAVICodecAccFileSpecInfo;

typedef struct {
	CfaApiAudType eAudCodec;
	/* [Playback Set] need to be set by playback before demux (vCfaAviPrsDmux) */
	ECfaAviStrmType eStrmType;
	u32 u4StrmIdx;	 /* e.g.: 00dc: u4StrmIdx = 0, 01wb: u4StrmIdx = 1 */
	u32 u4Scale;		/* from "strh" */
	u32 u4Rate;		 /* from "strh" */
	u32 u4Bps;		 /* byte per second; only used in CBRA */
	u16 u2AudBitsPerSample;

	/* CFA AVI Internal, for PTS */
	u32 u4TxedByte;	  /* only used in CBRA */
	u32 u4TxedChunk;   /* only used in VBRA and INTV */

	u32 u4SampleRate;
	u16 u2Channels;

	/* for audio vorbis codec*/
	bool fgSetAudHdr; /* for Vorbis*/
	TCfaAVICodecSpecInfo rSpecInfo;
	u64 u8CaH264InfoTxbefore;
	bool fgSpecH264InfoTxed;
	TCfaAVICodecSpecInfo rSpecH264H265Info;

	TCfaAVICodecSpecInfo rSpecMPEG1Info;
	u64 u8SpecMPEG1InfoDataOfst;
	bool fgSpecMPEG1InfoTxed;
	u64 u8CaMPEG1Txbefore;

	TCfaAVICodecSpecInfo rSpecMPEG4Info;
	u64 u8SpecMPEG4InfoDataOfst;
	bool   fgSpecMPEG4InfoTxed;
	u64 u8CaMPEG4InfoTxbefore;

	u8  u1AvcPayloadLenFieldSz;
	bool   fgAdvanceAvc;
	u8  *pu1AvcPayloadHdr;

	/*for ACC fileinfo*/
	bool fgSetAccfileAudHdr;
	TCfaAVICodecAccFileSpecInfo rAccFileSpecInfo;

	/*-----------------------------------------------------------------------------
	// set in vCfaAviPrsDmux
	// u32 u4LastOfst;	 // offset of last parsed(not necessarily transferred) chunk
	// u32 u4LastSz;	 // size of last parsed(not necessarily transferred) chunk
	// u32 u4ChukNs;
	// u32 u4Duration;	// from "strh"
	// u32 u4FrmStc;	// elapsed STC in one frame; from dwScale and dwRate; only used in VBRA and INTV
	-----------------------------------------------------------------------------*/
} TCfaAviStrmInf;

typedef struct {
	TCfaAviStrmInf tStrmInf;
	/*-----------------------------------------------------------------------------
	// TAviIdx1Abs tAbs;
	// u32 u4Width;
	// u32 u4Height;
	-----------------------------------------------------------------------------*/

	/* [Playback Set] need to be set by playback before demux (vCfaAviPrsDmux) */
	AVCODECID_T eCodec;
} TCfaAviVInf;

typedef struct {
	/*TAviIdx1Abs tAbs; */
	TCfaAviStrmInf tStrmInf;

	/* [Playback Set] need to be set by playback before demux (vCfaAviPrsDmux) */
	TCfaASamplingInf tSamInf;
} TCfaAviAInf;

#if CONFIG_CFA_AVI_TX_ALL_SP
typedef struct {
	/*TAviIdx1Abs tAbs; */
	TCfaAviStrmInf tStrmInf;

	/*Todo: if need other info*/
} TCfaAviSpInf;
#endif

typedef struct {
	/* file header info */
	bool fgStrdExist;

	/* chunk info */
	bool fgDrmExist;
	u16 u2KeyIdx;
	u32 u4EncryptOfst;
	u32 u4EncryptLen;
} TCfaAviDRMInf;

/* Avi parsed chunk info ,for parser finished check*/
typedef struct {
	u32 u4VidPrsChunk;  /* parsed video chunk number */
	/*Todo: need support multiple audio range info */
	u32 u4AudPrsChunk;  /* parsed audio chunk number */
	u64 u8AudPrsByte;  /* parsed audio bytes */

	u32 u4SubPrsChunk;  /* parsed subpicture chunk number */
} TCfaAviPrsStrmInf;


typedef struct {
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
	#endif
	/* CFA AVI internal */
	CfaAviAnaSt eCurCfaAviAnaSt;		   /*< current AVI CFA analyze state */
	CfaAviPrsBitStrmType eCurPrsPktType;   /*< original: _bStrmType */
	void *pvSptHdl;				   /*< currently ignore, for multi instance issue, */
	u8 *pucHdrBuf;
	/*< header buffer to pre-read data for analyzing,
	(pucHdrBuf -CFA_AVI_HDR_BUF_SA_OFST) ~ (pucHdrBuf + CFA_AVI_HDR_BUF_SZ-1) */
	uintptr_t ptrMemAddress;			/*< memory address got from Pfr, using sync DMA, 071228 */
	u64 u8Ca;						   /*< current analyzed address */
	u32 u4CurPrsFlg;				/*< current parsing stream flag ,for enablestream to fill*/
	bool fgPlayFrmFlag;						/*To check if playing a Frame*/
	u32 u4PrsFlg;					   /*< current parsing stream flag ,for LPE to fill*/
	u32 u4DataSz;					   /*< original: _dwDataSz */
	u64 u8PrsPts;				/*< original: _dwPrsPts	 set in vCfaAviPrsDmux */
	u64 u8SpEndPts;					   /*< original: _dwSpEndPts */
	u64 u8CurChkOfst;

	/* for CFA only, can be remove to another structure. */
	/* For VOP check. */
	u8 u1ChunkVopNs;				/*< original: _bChunkVopNs; */
	u8 u1CurBGrpNum;
	u8 u1TotalBGrpNum;
	CfaPicType ePrePicType;

	CfaPicType ePreValidPicType;

	u64 u8CurVPts;
	u64 u8PreVPts;

	u64 u8LastVPts;
	u64 u8LastAPts;

	CfaApiPicTxMode eSaveTxMode;
	u32 u4GTxLen;
	u32 u4TxTimeAfterSyncPB;

	#if CONFIG_CHECK_TX_DATA_TIME
	TCfaStatTime rCfaStattime;
	#endif

	/* set by playback */
	CfaAviRange  rCfaRange;
	CfaApiVidType eVidType;

	TCfaAviVInf rCfaAviVInf;

	u32 u4CurAudStrmID;				   /*< current audio stream idx. */
	u8 ucCurAudInfoIdx;				   /*< current audio info idx related to rCfaAviAInf[]. */
	#if CONFIG_CFA_AVI_TX_ALL_AUD
	u32 u4CurParseAudStrmID;
	u8 ucAudInfoIdx;						/* save the idx of  the Tx audio chunk */
	#endif

	TCfaAviAInf rCfaAviAInf[MAX_NS_AVI_AUD];
	CfaAviVbrMp3GarbageInfo rVbrMp3GarbageInfo;

	#if CONFIG_CFA_AVI_TX_ALL_SP
	u32 u4CurSpStrmID;
	u8 ucCurSpInfoIdx;

	u32 u4CurParseSpStrmID;
	u8 ucSpInfoIdx;
	TCfaAviSpInf rCfaAviSpInf[MAX_NS_AVI_INTERNAL_SP];
	#else
	TCfaAviStrmInf rCfaAviSpStrmInf;
	#endif

	/* playback will request this information by callback */
	CfaAviCurPosiInfo rCfaAviCurPosiInfo;

	/* Avi parsed chunk related information */
	TCfaAviPrsStrmInf rCfaAviPrsStrmInf;

	/* CFA AVI DRM related. */
	#if CONFIG_CFA_DIVX_DRM_SUPPORT
	u32 u4DrmState;					  /*< original: _dwDrmState */
	u32 u4DrmRmnLen;					/*< original: _dwDrmRmnLen */
	u8 *pucDrmTmpBuf;				  /*< original: _pbDrmTmpBuf */
	uintptr_t ptrDrmMemAddress;			  /*< memory address got from Pfr, using sync DMA, 071228 */
	TCfaAviDRMInf rCfaAviDRMInf;		  /*< original: _tDrmInf */
	CFA_DIVXDRM_INFO_T rDivxDRMInf;	/* transfer to splitter as a parameter */
	#endif

	bool fgBGrouped;					  /*< original: _fgBGrouped */

	/* CFA function pointer list */
	/*CfaDrvIntf *pCfaDrvIntf;*/

	u64 u8Endoffst;
	u64 u8EndPts;
	#ifdef MM_SUPPORT_DIVXHT31
	u64 u8FFRangeEndPts;
	#endif
	u64 u8FileSz ;  /*< current file size in bytes */

	bool	fgCurInstIsSecond;

	u32 u4MaxAudChunkDuration;

	HANDLE_T hMutex;

	u32 u4CfaQueryType;

	CfaAviParsingModeInfo rParsingModeInf;

	u64 u8VidCodecSpecDataOfst;

	u32 u4VidCodecSpecDataLen;

	u64 u8MinStartStrmOfst;

	bool fgGetVc1Case;

	bool fgNeedTxStartCode;

	u8 *pucVc1;	/*mtk40261 add for continous mem @2009/04/30*/

	#if CONFIG_CFA_AVI_DETECT_VBR_GARBAGE_DATA
	TCfaAviVbrGarbageInf rVbrGarbageInf;
	bool fgVbrGarbageBufSync;
	#endif

	u64 u8TransferdAudChunks;

	/*Todo: need support timer control...*/
	u64 u8SearchCnt;

	#if CONFIG_CFA_AVI_SUPPORT_CORRECT_CHUNK_ABNORMITY
	bool fgNextChunkDetecting;
	u8 aucPrev4cc[AVI_4CC_BYTES];
	u64 u8PrevCa;
	u32 u4PrevDataSize;
	#endif

	u64 u8PrevVidOffset;

	u64 u8CurListLen;

	#if CONFIG_CFA_AVI_NEW_METHOD_FOR_ABR_AUDIO
	#if CONFIG_CFA_AVI_SUPPORT_MULTI_AUDIO_FOR_ABR
	bool fgPrevIsVidChunk[MAX_NS_AVI_AUD];
	#else
	bool fgPrevIsVidChunk;
	#endif
	#endif

	bool fgGotValidVidChunk;

	#if CONFIG_CFA_AVI_SUPPORT_H26_NEW_METHOD
	u32 u4VOneFrmDuration;
	#endif

	#if CONFIG_CFA_AVI_NEW_METHOD_FOR_ERR_JUMP
	bool fgIsFileHasIndex;

	u64 u8OffsetJumpCnt;
	#endif

	#if CONFIG_CFA_AVI_SUPPORT_AUDIO_DRM
	CfaAviAudCryptInfo rAudCryptInfo;
	#endif

	#if CONFIG_CFA_AVI_NEW_SYNC_PB_BUF_FLOW
	bool fgNeedUseNewSyncFlow;

	/*u64 u8MaxOfstInPbBuf;*/

	u32 u4RemainDataInPbBuf;

	bool fgCanUseNewSyncFlow;

	u64 u8LastCa;

	bool fgNeedAdjustAfterTxDone;
	#endif

	#if CONFIG_CFA_AVI_DIVIDE_LARGE_AUD_CHUNK
	u32 u4CurAudDividingSize[MAX_NS_AVI_AUD];

	u32 u4CurAudChunkTxedOfst;

	u32 u4AudChunkLastTxedByte;

	bool fgStartTxAudChunk;
	#endif

	#if CONFIG_CFA_AVI_SUPPORT_FIFO_LIMITATION
	CfaAviFifoInfo rFifoInfo;
	#endif

	#if CONFIG_CFA_AVI_PLAY_ONE_FRM_ONLY_TX_I_FRM
	bool fgNeedFinishIfTxDone;
	#endif

	u32 u4CurIFrmChunkNo;

	u64 u8CurIFrmChunkOfst;

	#if CONFIG_CFA_AVI_USE_THRESHOLD_TMP
	bool fgUseThreshold;
	#endif

	/*sequence info of codec specical data*/
	bool fgPrsSeqFrameInterpolation;
	bool fgPrsPreProcRange;
	u32 u4PrsNumBFrames;
	bool  fgFillDummyAU;
	/* add for VBR Garbage data*/
	bool fgDeteGabageData;
	bool fgDeteGabageDataEnd;
	u64 u8TrueStartPts;
	u32 u4GabageChunkNum;

	#if AVI_SUPPORT_SKIP_DATA_IN_TX_DATA
	u64 u8SkipVDataInTxData;
	u64 u8SkipADataInTxData;
	u64 u8SkipSpDataInTxData;
	#endif

	bool fgFristVidBlk;
	bool fgFristAudBlk;

	u32 u4AvcSliceLen;
	u32 u4AvcChunkRemSz;

	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
	#endif
} CfaAviInst;



/*-----------------------------------------------------------------------------
					data declarations
-----------------------------------------------------------------------------*/

/* AVI CFA interface */
EXTERN CfaIntf _rAviCfaIntf;


/*-----------------------------------------------------------------------------
					function declarations
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
* Name: vCfaAviInitPara
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
EXTERN void CfaAviInitPara(CfaAviInst *prCfaAviInst);

/* C header file */
#ifdef __cplusplus
}
#endif

#endif	/* _CFA_AVI_H_ */

