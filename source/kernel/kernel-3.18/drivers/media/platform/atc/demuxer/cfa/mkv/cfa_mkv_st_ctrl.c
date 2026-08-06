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
#include "dmx_mem.h"
#include "dmx_cpsa.h"
#include "cfa_mkv.h"
#include "cfa_mkv_st_ctrl.h"
#include "dmx_spt_cfa.h"
#include "dmx_spt_psr.h"
#include "cfa_cfg.h"

/*-----------------------------------------------------------------------------
macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/

#define CFA_MKV_ID_LINE		21
#define CFA_MKV_SIZE_BYTES	8
#define CFA_MKV_WMV_PVOP	0x01
#define CFA_MKV_ID_COL		5

#define MKV_DUMP_FILE 0

#if MKV_DUMP_FILE
struct file *fd = NULL;
char filename[20] = {"/data/dump.txt"};
#endif

#define VORBIS_DEBUG 0

#if VORBIS_DEBUG
#define DEBUG_VORBIS DMXLOG_TRACE
#else
#define DEBUG_VORBIS DMXLOG_DEBUG
#endif

#define DUMP_FILE	0

#if DUMP_FILE
#define INVALID_MKV_FILE_POINTER	0XFFFFFFFF

static HANDLE dumpDataFile = INVALID_HANDLE_VALUE;
static u32  dwWriteBytes;
static u32  dwPtr;
static u8  strTag[16] = {0XFF, 0XEE, 0XEE, 0XEE, 0XEE, 0XEE, 0XEE, 0XEE,
							0XEE, 0XEE, 0XEE, 0XEE, 0XEE, 0XEE, 0XEE, 0XFF};
#endif

/*MKV EBML element IDs, CFA related*/
u8 aucMkvId[CFA_MKV_ID_LINE][CFA_MKV_ID_COL] = {
	{0xe7, 0xff, 0, 0, 0},				  /*0.TimeCode*/
	{0xa7, 0xff, 0, 0, 0},				  /*1.Position*/
	{0xab, 0xff, 0, 0, 0},				  /*2.PrevSize*/
	{0xa0, 0xff, 0, 0, 0},				  /*3.BlockGroup*/
	{0xa3, 0xff, 0, 0, 0},				  /*4.SimpleBlock*/
	{0xa1, 0xff, 0, 0, 0},				  /*5.Block*/
	{0xfb, 0xff, 0, 0, 0},				  /*6.ReferenceBlock*/
	{0x9b, 0xff, 0, 0, 0},				  /*7.BlockDuration*/
	{0xbf, 0xff, 0, 0, 0},				  /*8.CRC32*/
	{0xc8, 0xff, 0, 0, 0},				  /*9.Unkown element ID*/
	{0x1a, 0x45, 0xdf, 0xa3, 0xff},   /*10.EBML*/
	{0x18, 0x53, 0x80, 0x67, 0xff},   /*11.Segment*/
	{0x15, 0x49, 0xa9, 0x66, 0xff},   /*12.SegmentInfo*/
	{0x11, 0x4d, 0x9b, 0x74, 0xff},   /*13.SeekHead*/
	{0x1f, 0x43, 0xb6, 0x75, 0xff},   /*14.Cluster*/
	{0x16, 0x54, 0xae, 0x6b, 0xff},   /*15.Track*/
	{0x1c, 0x53, 0xbb, 0x6b, 0xff},   /*16.Cues*/
	{0x19, 0x41, 0xa4, 0x69, 0xff},   /*17.Attachments*/
	{0x10, 0x43, 0xa7, 0x70, 0xff},   /*18.Chapters*/
	{0x12, 0x54, 0xc3, 0x67, 0xff}	  /*19.Tags*/
#if CONFIG_CFA_MKV_SUPPORT_DRM
	, {0xdd, 0xff,0 ,0 ,0}
#endif

	/*new IDs should be added here! And must modify CfaMkvIDs also!*/
	};

u64 _au8VsintSub[] = {
	0x3f,
	0x1fff,
	0x0fffff,
	0x07ffffff,
	0x03ffffffffLL,
	0x01ffffffffffLL,
	0x00ffffffffffffffLL,
	0x007fffffffffffffLL
};



static void GenerateVirtualMP4SequenceHeader(const CfaMkvInst *prCfaMkv,
	const u8 *pu1FstFrameData, u32 u4DataLen)
{
	u32 u4ByteOffset = 0;
	u8  u1BitOffsetInByte = 0;
	u8  u1DataIn = 0;
	u8  u1TempData = 0;
	u16	u2VopTimeIncrementLen = 0;
	u16	u2VopTimeBitStr = 0;

	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA_MKV_ST_CTRL] Generate Virtual Header!\r\n"));

	/*at first we need to check the first 4 bytes if is '0x000001b6'*/
	if ((pu1FstFrameData[0] != 0x0) ||
		(pu1FstFrameData[1] != 0x0) ||
		(pu1FstFrameData[2] != 0x1) ||
		(pu1FstFrameData[3] != 0xB6)) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] Generate Virtual Header Error1!\r\n"));
		return;
	}
	u4ByteOffset += (u32)4;
	u1BitOffsetInByte += (u8)2; /*video coding type 2 bits*/
	/*scan the modulo time base*/
	u1DataIn = pu1FstFrameData[u4ByteOffset];
	u1TempData = (u1DataIn << u1BitOffsetInByte);

	while ((u1TempData >> 7) == 0) {
		u1BitOffsetInByte++;
		if (u1BitOffsetInByte == ((u8)8)) {
			u1BitOffsetInByte = 0;
			u4ByteOffset++;
			u1DataIn = pu1FstFrameData[u4ByteOffset];
		}
		if (u4ByteOffset >= u4DataLen) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] Generate Virtual Header Error2!\r\n"));
			return;
		}
		u1TempData = (u1DataIn << u1BitOffsetInByte);
	}
	u1BitOffsetInByte++;/*marker bit*/
	if (u1BitOffsetInByte == ((u8)8)) {
		u1BitOffsetInByte = 0;
		u4ByteOffset++;
	}
	u1DataIn = pu1FstFrameData[u4ByteOffset];
	u1TempData = (u1DataIn << u1BitOffsetInByte);
	/*scan the length of VOP time increment*/
	while ((u1TempData >> 7) == 0) {
		u1BitOffsetInByte++;
		u2VopTimeIncrementLen++;
		if (u1BitOffsetInByte == ((u8)8)) {
			u1BitOffsetInByte = 0;
			u4ByteOffset++;
			u1DataIn = pu1FstFrameData[u4ByteOffset];
		}
		if (u4ByteOffset >= u4DataLen) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] Generate Virtual Header Error2!\r\n"));
			return;
		}
		u1TempData = (u1DataIn << u1BitOffsetInByte);
	}
	if (u2VopTimeIncrementLen > 0)
		u2VopTimeBitStr = ((u16)1);

	while (u2VopTimeIncrementLen > ((u16)1)) {
		u2VopTimeBitStr = (u16)((u16)(u2VopTimeBitStr << ((u16)1)) | (u2VopTimeBitStr));
		u2VopTimeIncrementLen--;
	}
	prCfaMkv->pu1Mp4SeqHdr[7] = (u8)0x80 | ((u8)(u2VopTimeBitStr >> 9));
	prCfaMkv->pu1Mp4SeqHdr[8] = (u8)0x00 | ((u8)(u2VopTimeBitStr >> 1));
	prCfaMkv->pu1Mp4SeqHdr[9] = (u8)((u8)0x50 | ((u8)(u2VopTimeBitStr << 7)));
}



static u32 MapAacSampleRate(u32 u4SampleFreq)
{
	switch (u4SampleFreq) {
	case (u32)96000:
		return (u32)0x0;
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


/*Large Endian*/
static u64 MkvComputeEndian(const u8 *pucPoint, u8 ucLen)
{
	u64 u8Result = 0;
	u8 ucOfst = 0;

	if (NULL == pucPoint) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] pcPoint is NULL\r\n"));
		return 0;
	}

	while (ucLen > 0) {
		u8Result += (((u64)(*(pucPoint + ucOfst)))<<((u64)8 * ((u64)ucLen	- (u64)1)));
		ucOfst++;
		ucLen--;
	}
	return u8Result;

}

static void SetAacADTSHdr(void *pvSptHdl, CfaMkvInst *prCfaMkv)
{
	u32 u4AudSamplePerSec = 0;
	u32 u4AudChannels = 0;
	u32 u4FrameSize = 0;
	u8 u1AuHeader[7] = {0};

	if (prCfaMkv->rCurBlock.ucStrmIdx >= MAX_NS_MKV_AUD) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] ucStrmIdx is %d,line %d Send EOS\r\n"),
			prCfaMkv->rCurBlock.ucStrmIdx, DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl,prCfaMkv);
		return;
	}

	u4AudSamplePerSec = MapAacSampleRate(prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].u4SampleRate);
	u4AudChannels = prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].u4Channles;
	u4FrameSize = prCfaMkv->rCurBlock.au4FrameSize[prCfaMkv->rCurBlock.i4CurFrameNum] + (u32)7;

	u1AuHeader[0] = 0xFF;
	u1AuHeader[1] = 0xF9;
	u1AuHeader[2] = (u8)(((u32)1 << (u32)6) | ((u4AudSamplePerSec << (u32)2) & (u32)0x3C) | ((u4AudChannels >> ((u32)2)) & (u32)0x1));
	u1AuHeader[3] = (u8)(((u4AudChannels & ((u32)0x3)) << (u32)6) | ((u4FrameSize >> ((u32)11)) & (u32)0x3));
	u1AuHeader[4] = (u8)(((u4FrameSize >> ((u32)3)) & (u32)0xFF));
	u1AuHeader[5] = (u8)(((u4FrameSize << ((u32)5)) & (u32)0xE0) | (((u32)0x7FF >> (u32)6) & (u32)0x1F));
	u1AuHeader[6] = (u8)((((u32)0x7FF << (u32)2) & (u32)0xFC));

	dmx_memcpy((void *)(prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.auHeader),
		(void *)(&u1AuHeader[0]),
		sizeof(u1AuHeader));
}

static void TxWVC1Header(void *pvSptHdl, CfaMkvInst *prCfaMkv)
{
	MRESULT mrResult = RET_DMX_OK;
	CFA_VIDEO_INFO_T rVidInf = {0};

	prCfaMkv->eCurState = CFA_MKV_ST_WVC1_TX; /*CFA_MKV_ST_WVC1_TX_SC_RET;*/

	rVidInf.eTxMode = CFA_PTM_EXACT_POS;
	rVidInf.eVidType = prCfaMkv->rVidStmInfo.eVidCodec;

	rVidInf.u4PrsStrmId = (u32)(prCfaMkv->rVidStmInfo.u8VidTrackNo);
	rVidInf.u8Len = 4;

	/*i4Ret = Spt4CfaBuf2VFifoAUCtrl(pvSptHdl, prCfaMkv->pu1Wvc1Header, &rVidInf);*/

	mrResult = Spt4CfaBuf2VFifo(pvSptHdl, prCfaMkv->pu1Wvc1Header, 0,
							 CFA_PTM_EXACT_POS, prCfaMkv->rVidStmInfo.eVidCodec , (u64)4);

	if (RET_DMX_OK != mrResult) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
		Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, TRUE, GAU_E_FAIL);
	}

}


static void TxAVCHEVCStartCode(void *pvSptHdl, CfaMkvInst *prCfaMkv)
{
	MRESULT mrResult = RET_DMX_OK;

	mrResult = Spt4CfaBuf2VFifo(pvSptHdl, prCfaMkv->pucAvcHevcStartCode, 0, CFA_PTM_SAME_POS, prCfaMkv->rVidStmInfo.eVidCodec, (u64)4);
	if (RET_DMX_OK != mrResult) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
		Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, TRUE, GAU_E_FAIL);
		return;
	}

	prCfaMkv->eCurState = CFA_MKV_ST_TX_H264H265_STARTCODE_RET;
}


/*-----------------------------------------------------------------------------
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
void MkvFinishPrs(void *pvSptHdl, CfaMkvInst *prCfaMkv)
{
	prCfaMkv->eCurState = CFA_MKV_ST_IDLE;

#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaMkv);
	MMATE_CHECK_STRUCT(prCfaMkv->rCurBlock);
	MMATE_CHECK_STRUCT(prCfaMkv->rRange);
#endif
	if (prCfaMkv->fgDemuxError) {
		Cfa2PsrStrmInfo rPathStrmInfo = {0};

		if (CFA_PRS_BIT_STRM_TYPE_V & prCfaMkv->u4PrsFlg) {
			u32 i;

			rPathStrmInfo.u4VstrmNs = 1;
			for (i = 0; i < rPathStrmInfo.u4VstrmNs; ++i)
				rPathStrmInfo.ucDecVidStId[i] = (u8)prCfaMkv->rVidStmInfo.u8VidTrackNo;
		}

		if ((CFA_PRS_BIT_STRM_TYPE_A & prCfaMkv->u4PrsFlg) &&
			(DMX_INVALID_UINT8 != prCfaMkv->ucCurAIndex)) {
			u32 j;

			rPathStrmInfo.u4AstrmNs = 1;
			if (prCfaMkv->ucCurAIndex >= MAX_NS_MKV_AUD) {
				DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA_MKV_ST_CTRL] ucCurAIndex is %d,line %d Send EOS\r\n"),
					prCfaMkv->ucCurAIndex, DMX_LINE_NO);
				MkvFinishPrs(pvSptHdl,prCfaMkv);
				return;
			}

			for (j = 0; j < rPathStrmInfo.u4AstrmNs; ++j) {
				rPathStrmInfo.u2DecAudStId[j] =
					(u16)prCfaMkv->arAudStmInfo[prCfaMkv->ucCurAIndex].u8AudTrackNo;
			}

		}

		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] bad file data!\r\n"));
		Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, TRUE, (u32)GAU_E_FAIL);
	} else {

		DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
			TEXT("[CFA_MKV_ST_CTRL]Finish parse at %x,%x!\r\n"),
			prCfaMkv->rRange.u8VidEndOfst, prCfaMkv->u8Ca);
		if (DMX_IS_FF_PLAY(pvSptHdl))
			Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->rRange.u8VidEndOfst, TRUE, (u32)GAU_E_EOS);
		else
			Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->rRange.u8VidEndOfst, FALSE, (u32)GAU_E_EOS);
	}
}

static void MkvMoveHdr(CfaMkvInst *prCfaMkv, u32 u4MoveLen)
{
	if (prCfaMkv->u8HdrLen < u4MoveLen) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] find invalid data and Seek\r\n"));
		if (DMX_IS_RW_PLAY(prCfaMkv->pvSptHdl)) {
			Spt4CfaFinishedEx(prCfaMkv->pvSptHdl, prCfaMkv->u8Ca, FALSE, GAU_E_ERRCHUNK);
		} else {
			Spt4CfaFinishedEx(prCfaMkv->pvSptHdl, prCfaMkv->u8Ca, TRUE, GAU_E_ERRCHUNK);
		}
		return;
	}

	prCfaMkv->u8HdrLen -= u4MoveLen;
	if (prCfaMkv->u8HdrLen != 0) {
		if ((prCfaMkv->u8HdrLen + (u64)u4MoveLen) > CFA_MKV_HDR_BUF) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] prCfaMkv->u8HdrLen=%lld,u4MoveLen=%d\r\n"),
				prCfaMkv->u8HdrLen, u4MoveLen);

			prCfaMkv->u8HdrLen = (u64)((u32)CFA_MKV_HDR_BUF - u4MoveLen);
		}
		dmx_memcpy((void *)(prCfaMkv->aucHdr), (void *)(prCfaMkv->aucHdr + u4MoveLen),
					(u32)prCfaMkv->u8HdrLen);
	}
}


/*-----------------------------------------------------------------------------
* Name: GetVaraibleSize
*
* Description:
*	   get size of varaible int value
*
*
* Inputs:
*
* Outputs:
*
* Returns: void
*
*-----------------------------------------------------------------------------*/
u8 GetVaraibleSize(const u8 *pucSize)
{

	u8 ucSizeHdr = *pucSize;

	if ((ucSizeHdr < (u8)0x10) && (ucSizeHdr >= (u8)0x08))
		return (u8)5;
	else if ((ucSizeHdr < (u8)0x08) && (ucSizeHdr >= (u8)0x04))
		return (u8)6;
	else if ((ucSizeHdr < (u8)0x04) && (ucSizeHdr >= (u8)0x02))
		return (u8)7;
	else if (ucSizeHdr < (u8)0x02)
		return (u8)8;
	else if ((ucSizeHdr < (u8)0x20) && (ucSizeHdr >= (u8)0x10))
		return (u8)4;
	else if ((ucSizeHdr < (u8)0x40) && (ucSizeHdr >= (u8)0x20))
		return (u8)3;
	else if ((ucSizeHdr < (u8)0x80) && (ucSizeHdr >= (u8)0x40))
		return (u8)2;
	else
		return (u8)1;

}


static MRESULT CfaMkvPbb2FifoAUCtrl(CfaMkvInst *prCfaMkv, void *pvSptHdl,
								 void *pvStmInfo,
								 CfaPrsBitStrmType eCfaMkvPrsType,
								 u64 u8TotalAULen)
{
	MRESULT mrResult = RET_DMX_OK;
	CFA_VIDEO_INFO_T *prVidInf = NULL;
	CFA_AUDIO_INFO_T *prAudInf = NULL;
	CFA_SUBPIC_INFO_T *prSpInf = NULL;

	switch(eCfaMkvPrsType) {
	case CFA_PRS_BIT_STRM_TYPE_V:
		prVidInf = (CFA_VIDEO_INFO_T *)pvStmInfo;
		if (0 == prVidInf->u8Len) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] find invalid data and Seek\r\n"));
			if (DMX_IS_RW_PLAY(pvSptHdl)) {
				Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, FALSE, GAU_E_ERRCHUNK);
			} else {
				Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, TRUE, GAU_E_ERRCHUNK);
			}
			return 0;
		} else if ((prVidInf->u8FileOfst + prVidInf->u8Len) >= prCfaMkv->rRange.u8VidEndOfst) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
			MkvFinishPrs(pvSptHdl, prCfaMkv);
			return 0;
		} else {
			/*do
			nothing*/
		}

		if (prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_MJPEG)
			prVidInf->eTxMode = CFA_PTM_MJPEG_I;

		mrResult = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, prVidInf);
		if (mrResult != RET_DMX_OK) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
			MkvFinishPrs(pvSptHdl, prCfaMkv);
			return 0;
		}
		break;

	case CFA_PRS_BIT_STRM_TYPE_A:
		prAudInf = (CFA_AUDIO_INFO_T *)pvStmInfo;
		if ((prAudInf->u8FileOfst + prAudInf->u8Len) >= prCfaMkv->rRange.u8VidEndOfst) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
			MkvFinishPrs(pvSptHdl, prCfaMkv);
			return 0;
		}

		DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
			TEXT("[CFA_MKV_ST_CTRL][AU] line %d prAudInf->u4PrsStrmId is %d!\r\n"),
			__LINE__, prAudInf->u4PrsStrmId);

		if (prCfaMkv->rCurBlock.u8TimeCode < prCfaMkv->rRange.u8TargetTimeCode) {

			DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
				TEXT("[CFA_MKV_ST_CTRL] u8TimeCode is %lld < u8TargetTimeCode is %lld\r\n"),
				prCfaMkv->rCurBlock.u8TimeCode, prCfaMkv->rRange.u8TargetTimeCode);
			MkvSkipBlock(pvSptHdl, prCfaMkv);
			return 0;
		}

		if ((DMX_IS_RW_PLAY(pvSptHdl)) && (!prCfaMkv->fgHasVideo) &&
			(prCfaMkv->u8CurAId == prAudInf->u4PrsStrmId)) {
			if (prCfaMkv->fgFinishRWAU) {
				DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA_MKV_ST_CTRL] RW----Create an AU, so Finish\r\n"));
				mrResult = Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->rRange.u8VidEndOfst, FALSE, GAU_E_EOS);
				MM_RETURN(mrResult);
			}

			prAudInf->fgAUCompleteByEnd = TRUE;
			prAudInf->fgUnitEnd = FALSE;

			if (prCfaMkv->fgTxFirst) {
				prCfaMkv->fgTxFirst = FALSE;
				prAudInf->fgUnitStart = TRUE;
			} else
				prAudInf->fgUnitStart = FALSE;

			if (0 == prCfaMkv->u4RWUnitAULen) {
				prAudInf->fgUnitStart = FALSE;

				prAudInf->fgUnitEnd = TRUE;
				prAudInf->u8FileOfst = prCfaMkv->u8Ca;
				prAudInf->u8Len = 0;
				prAudInf->u8TotalAULen = 0;
				prCfaMkv->fgFinishRWAU = TRUE;
				DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA_RM] RW----Create an AU\r\n"));
				mrResult = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, prAudInf);
				MM_RETURN(mrResult);
			}

			if (prCfaMkv->u4RWUnitAULen < prAudInf->u8Len)
				prAudInf->u8Len = prCfaMkv->u4RWUnitAULen;

			prCfaMkv->u4RWUnitAULen = prCfaMkv->u4RWUnitAULen - (u32)prAudInf->u8Len;
			prAudInf->u8TotalAULen = 0;
			mrResult = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, prAudInf);
		} else if ((!prCfaMkv->fgHasVideo) && (prCfaMkv->u8CurAId == prAudInf->u4PrsStrmId)) {
			DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
				TEXT("[CFA_MKV_ST_CTRL][AU] line %d compose AU by end, stream id is %d\r\n"),
				__LINE__, prAudInf->u4PrsStrmId);

			prAudInf->fgAUCompleteByEnd = TRUE;
			prAudInf->fgUnitEnd = FALSE;

			if (prCfaMkv->fgTxFirst) {
				prCfaMkv->fgTxFirst = FALSE;
				prAudInf->fgUnitStart = TRUE;
			} else
				prAudInf->fgUnitStart = FALSE;

			if (prCfaMkv->i4AudAULenWithoutVid > (s32)prAudInf->u8Len) {
				prCfaMkv->i4AudAULenWithoutVid = prCfaMkv->i4AudAULenWithoutVid -
												(s32)prAudInf->u8Len;
				DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
					TEXT("[CFA_MKV_ST_CTRL][FF_AUD] line %d ")
					TEXT("i4AudAULenWithoutVid is %d, aud len is %d!\r\n"),
					__LINE__, prCfaMkv->i4AudAULenWithoutVid, (s32)prAudInf->u8Len);
				prAudInf->u8TotalAULen = 0;
				mrResult = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, prAudInf);
			} else if ((prCfaMkv->i4AudAULenWithoutVid > 0) &&
				(prCfaMkv->i4AudAULenWithoutVid <= (s32)prAudInf->u8Len)) {
				DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
					TEXT("[CFA_MKV_ST_CTRL][FF_AUD] line %d ")
					TEXT("i4AudAULenWithoutVid is %d, aud len is %d!\r\n"),
					__LINE__, prCfaMkv->i4AudAULenWithoutVid, (s32)prAudInf->u8Len);
				prAudInf->u8TotalAULen = 0;
				mrResult = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, prAudInf);
				prCfaMkv->eCurState = CFA_MKV_ST_TX_AUD_AU_BYEND;
			} else {
				DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA_MKV_ST_CTRL][FF_AUD] line %d ")
					TEXT("i4AudAULenWithoutVid is %d, aud len is %d!\r\n"),
					__LINE__, prCfaMkv->i4AudAULenWithoutVid, (s32)prAudInf->u8Len);
			}
		} else {
			DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
				TEXT("[CFA_MKV_ST_CTRL][AU] line %d compose AU by length, stream id is %d\r\n"),
				__LINE__, prAudInf->u4PrsStrmId);
			prAudInf->u8TotalAULen = u8TotalAULen;
			mrResult = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, prAudInf);
		}

		break;

	case CFA_PRS_BIT_STRM_TYPE_SP:
		prSpInf = (CFA_SUBPIC_INFO_T *)pvStmInfo;
		if ((prSpInf->u8FileOfst + prSpInf->u8Len) >= prCfaMkv->rRange.u8VidEndOfst) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
			MkvFinishPrs(pvSptHdl, prCfaMkv);
			return 0;
		}
		mrResult = Spt4CfaPbb2SpFifoAUCtrl(pvSptHdl, prSpInf, u8TotalAULen);

		break;

	default:
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] invalid stream type(A,V,SP)\r\n"));
	}

	MM_RETURN(mrResult);

}

static void CfaMkvTxAudAUByEnd(void *pvSptHdl, CfaMkvInst *prCfaMkv)
{
	MRESULT mrResult = RET_DMX_OK;

	CFA_AUDIO_INFO_T rAudInf;

	mm_memset(&rAudInf, 0X00, sizeof(CFA_AUDIO_INFO_T));

	prCfaMkv->i4AudAULenWithoutVid = CFA_MKV_AUD_AULEN_WITHOUT_VID;
	prCfaMkv->fgTxFirst = TRUE;

	rAudInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;
	rAudInf.eAudType = prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec;
	rAudInf.u8FileOfst = prCfaMkv->rCurBlock.u8DataOfst;
	rAudInf.fgUnitStart = FALSE;
	rAudInf.fgUnitEnd = TRUE;
	rAudInf.fgAUCompleteByEnd = TRUE;
	rAudInf.u8Len = 0;
	rAudInf.u8TotalAULen = 0;
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA_MKV_ST_CTRL][FF_AUD] ++++Finish ONE AUD AU!\r\n"));
	mrResult = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rAudInf);

	prCfaMkv->eCurState = CFA_MKV_ST_TX_BLOCK;

}


static void MoveUnknownElementID(void *pvSptHdl, CfaMkvInst *prCfaMkv)
{
	u8 uLength = GetVaraibleSize(prCfaMkv->aucHdr);

	/*for bug116148*/
	if (uLength > (u8)4) {
		prCfaMkv->fgDemuxError = TRUE;
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] find invalid data and Seek\r\n"));
		if (DMX_IS_RW_PLAY(pvSptHdl)) {
			Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, FALSE, GAU_E_ERRCHUNK);
		} else {
			Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, TRUE, GAU_E_ERRCHUNK);
		}
		prCfaMkv->fgFindIdx = TRUE;
		return;
	}

	MkvMoveHdr(prCfaMkv, (u32)uLength);

}



static void TxHeader2Fifo(void *pvSptHdl, CfaMkvInst *prCfaMkv)
{
	MRESULT mrResult = RET_DMX_OK;
	CFA_VIDEO_INFO_T rVidInf = {0};

	switch (prCfaMkv->rCurBlock.eBlockType) {
	case CFA_PRS_BIT_STRM_TYPE_V:
		if (prCfaMkv->rVidStmInfo.rCfaMkvHeader.fgHeaderStriping) {
			prCfaMkv->eCurState = CFA_MKV_ST_HEADER_STRIPING;
			rVidInf.eVidType = prCfaMkv->rVidStmInfo.eVidCodec;
			rVidInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;
			rVidInf.u8Len = prCfaMkv->rVidStmInfo.rCfaMkvHeader.uHeaderLen;

			mrResult = Spt4CfaBuf2VFifoAUCtrl(pvSptHdl,
				prCfaMkv->rVidStmInfo.rCfaMkvHeader.auHeader,
				&rVidInf);
			if (RET_DMX_OK != mrResult) {
				DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
				MkvFinishPrs(pvSptHdl, prCfaMkv);
			}
			return;
		}

		break;

	case CFA_PRS_BIT_STRM_TYPE_A:
		if (prCfaMkv->rCurBlock.ucStrmIdx >= MAX_NS_MKV_AUD) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] ucStrmIdx is %d,line %d Send EOS\r\n"),
				prCfaMkv->rCurBlock.ucStrmIdx, DMX_LINE_NO);
			MkvFinishPrs(pvSptHdl, prCfaMkv);
			return;
		}

		if (prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.fgHeaderStriping) {
			prCfaMkv->eCurState = CFA_MKV_ST_HEADER_STRIPING;
			#ifdef NOT_USED
			if (prCfaMkv->rCurBlock.i4CurFrameNum != 0)
				prCfaMkv->rCurBlock.u8Pts = INVALID_TIMESTAMP;

			#endif

			if (prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec == CFA_AUD_DRV_FMT_AAC)
				SetAacADTSHdr(pvSptHdl, prCfaMkv);

			mrResult = Spt4CfaBuf2AFifo(pvSptHdl,
				prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.auHeader,
				prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.uHeaderLen,
				(u32)prCfaMkv->rCurBlock.u8TrackNum,
				prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec);

			if (RET_DMX_OK != mrResult) {
				DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
				MkvFinishPrs(pvSptHdl, prCfaMkv);
			}
			return;
		}
		break;

	case CFA_PRS_BIT_STRM_TYPE_SP:
		break;

	default:
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] eBlockType is %d, Send EOS\r\n"),
			prCfaMkv->rCurBlock.eBlockType);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
		break;
	}
}


#if CONFIG_CFA_MKV_SUPPORT_DRM
static void CfaMkvTxDrmInfo(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	u8 *pucHdrBuf = NULL;


	if ((prCfaMkv->u8HdrLen + u8TxLen) > ((u64)CFA_MKV_HDR_BUF)) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] u8HdrLen is %lld, u8TxLen is %lld,line %d Send EOS\r\n"),
			prCfaMkv->u8HdrLen, u8TxLen, DMX_LINE_NO);
		if (DMX_IS_RW_PLAY(pvSptHdl)) {
			Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, FALSE, GAU_E_ERRCHUNK);
		} else {
			Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, TRUE, GAU_E_ERRCHUNK);
		}
		return;
	}

	dmx_memcpy((void *)(prCfaMkv->aucHdr + prCfaMkv->u8HdrLen),
			   (void *)prCfaMkv->ptrMemAddr,
				(u32)u8TxLen);

	pucHdrBuf = prCfaMkv->aucHdr;

	LOADL_WORD(pucHdrBuf, prCfaMkv->rDRMInf.u2KeyIdx);
	LOADL_DWRD(pucHdrBuf + 2, prCfaMkv->rDRMInf.u4EncryptOfst);
	LOADL_DWRD(pucHdrBuf + 6, prCfaMkv->rDRMInf.u4EncryptLen);

	prCfaMkv->rDRMInf.fgDrmExist = (prCfaMkv->rDRMInf.u4EncryptLen > 0);

	if (prCfaMkv->rDRMInf.fgDrmExist)
		prCfaMkv->rDivxDRMInf.fgOn = TRUE;
	else
		prCfaMkv->rDivxDRMInf.fgOn = FALSE;

	prCfaMkv->rDivxDRMInf.u4DecryptLen = prCfaMkv->rDRMInf.u4EncryptLen;
	prCfaMkv->rDivxDRMInf.u2FrameKeyIdx = prCfaMkv->rDRMInf.u2KeyIdx;

	/* after read xxdd DRM chunk, continue to read the xxdc video chunk */
	ToNextState(pvSptHdl, prCfaMkv, (u64)CFA_MKV_HDR_READ_BYTES, CFA_MKV_ST_SC_ANA);

}


static void CfaMkvDrmProcBeforeTxData(void *pvSptHdl, CfaMkvInst *prCfaMkv)
{
	MRESULT mrResult = RET_DMX_OK;

	if (prCfaMkv->rDRMInf.fgDrmExist) {
		prCfaMkv->rDivxDRMInf.u8DecryptStOfst =
			prCfaMkv->rCurBlock.u8DataOfst + (u64)prCfaMkv->rDRMInf.u4EncryptOfst;
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] CfaMkvDrmProcBeforeTxData----->Spt4CfaTurnDivxDRM\r\n"));
		mrResult = Spt4CfaTurnDivxDRM(pvSptHdl, &prCfaMkv->rDivxDRMInf);
		if (mrResult != RET_DMX_OK) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
			MkvFinishPrs(pvSptHdl, prCfaMkv);
			return;
		}
		prCfaMkv->rDivxDRMInf.fgOn = FALSE;
		prCfaMkv->rDRMInf.fgDrmExist = FALSE;
	}

}


static void CfaMkvTurnOffDivXDrm(void *pvSptHdl, CfaMkvInst *prCfaMkv)
{
	MRESULT mrResult = RET_DMX_OK;

	if (prCfaMkv->rDRMInf.fgDrmExist) {
		prCfaMkv->rDivxDRMInf.fgOn = FALSE;
		mrResult = Spt4CfaTurnDivxDRM(pvSptHdl, &prCfaMkv->rDivxDRMInf);
		if (mrResult != RET_DMX_OK) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
			MkvFinishPrs(pvSptHdl, prCfaMkv);
			return;
		}

		prCfaMkv->rDRMInf.fgDrmExist = FALSE;
	}

}


static void CfaMkvDrmProcBeforeTxAudData(void *pvSptHdl, CfaMkvInst *prCfaMkv)
{
	MRESULT mrResult = RET_DMX_OK;

	if (prCfaMkv->rAudCryptInfo.u1CryptoSize <= 0) {
		prCfaMkv->rDRMInf.fgDrmExist = FALSE;
		prCfaMkv->rDivxDRMInf.fgOn = FALSE;
		return;
	}
	{
		prCfaMkv->rDRMInf.fgDrmExist = TRUE;
		prCfaMkv->rDivxDRMInf.fgOn = TRUE;
	}
	prCfaMkv->rDivxDRMInf.u8DecryptStOfst =
		prCfaMkv->rCurBlock.u8DataOfst + (u64)prCfaMkv->rAudCryptInfo.u1CryptoOffset;
	prCfaMkv->rDivxDRMInf.u4DecryptLen = prCfaMkv->rAudCryptInfo.u1CryptoSize;
	prCfaMkv->rDivxDRMInf.u2FrameKeyIdx = DMX_DIVXDRM_INVALID_FRAMEIDX;

	/* Yi Feng modify to fix BDP00229785 @ 2009/11/29 */
	if ((prCfaMkv->rDivxDRMInf.u8DecryptStOfst + prCfaMkv->rDivxDRMInf.u4DecryptLen) >
		(prCfaMkv->rCurBlock.u8BlockSize + prCfaMkv->rCurBlock.u8BlockOfst)) {
		if ((prCfaMkv->rCurBlock.u8BlockSize + prCfaMkv->rCurBlock.u8BlockOfst) >
			prCfaMkv->rDivxDRMInf.u8DecryptStOfst) {
			prCfaMkv->rDivxDRMInf.u4DecryptLen =
				(u32)((prCfaMkv->rCurBlock.u8BlockSize +
				prCfaMkv->rCurBlock.u8BlockOfst) - prCfaMkv->rDivxDRMInf.u8DecryptStOfst);
			prCfaMkv->rDivxDRMInf.u4DecryptLen = prCfaMkv->rDivxDRMInf.u4DecryptLen / 16 * 16;
		} else
			prCfaMkv->rDivxDRMInf.u4DecryptLen = 0;
	}


	mrResult = Spt4CfaTurnDivxDRM(pvSptHdl, &prCfaMkv->rDivxDRMInf);
	if (mrResult != RET_DMX_OK) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
	}

}
#endif


/*-----------------------------------------------------------------------------
* Name: TxBlock2Fifo
*
* Description:
*	   After one block parsed,this function will be called to transfer data to specific FIFO
*
*
* Inputs:
*
* Outputs:
*
* Returns: void
*
*-----------------------------------------------------------------------------*/
static void TxBlock2Fifo(void *pvSptHdl, CfaMkvInst *prCfaMkv)
{
	MRESULT mrResult = RET_DMX_OK;
	u8 idx = prCfaMkv->rCurBlock.ucStrmIdx;

	MkvGetPts(pvSptHdl, prCfaMkv);

	prCfaMkv->eCurPrsStrm = prCfaMkv->rCurBlock.eBlockType;

#if CONFIG_CFA_MKV_SUPPORT_DRM
	if (prCfaMkv->rCurBlock.eBlockType == CFA_PRS_BIT_STRM_TYPE_V)
		CfaMkvDrmProcBeforeTxData(pvSptHdl, prCfaMkv);
	else if (prCfaMkv->rCurBlock.eBlockType == CFA_PRS_BIT_STRM_TYPE_A)
		CfaMkvDrmProcBeforeTxAudData(pvSptHdl, prCfaMkv);
	else {
		/*do
		nothing*/
	}

#endif

	if (prCfaMkv->rCurBlock.eBlockType == CFA_PRS_BIT_STRM_TYPE_A) {
		if (prCfaMkv->rCurBlock.ucStrmIdx >= MAX_NS_MKV_AUD) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] ucStrmIdx is %d,line %d Send EOS\r\n"),
				prCfaMkv->rCurBlock.ucStrmIdx, DMX_LINE_NO);
			MkvFinishPrs(pvSptHdl,prCfaMkv);
			return;
		}
		if (prCfaMkv->rCurBlock.u8TimeCode < prCfaMkv->rRange.u8TargetTimeCode) {
			DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
				TEXT("[CFA_MKV_ST_CTRL] u8TimeCode is %lld < u8TargetTimeCode is %lld\r\n"),
			prCfaMkv->rCurBlock.u8TimeCode, prCfaMkv->rRange.u8TargetTimeCode);
			MkvSkipBlock(pvSptHdl, prCfaMkv);
			return;
		}

		if ((prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec == CFA_AUD_DRV_FMT_AAC) &&
			(prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].fgHaveTxHeader == FALSE)) {
			prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].fgHaveTxHeader = TRUE;
			if (prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].uAacHeaderSize > 4) {
				prCfaMkv->eCurState = CFA_MKV_ST_AAC_TX_DONE;
				mrResult =
					Spt4CfaBuf2AFifo(pvSptHdl,
					prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].auAacHeader,
					prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].uAacHeaderSize,
					(u32)prCfaMkv->rCurBlock.u8TrackNum,
					prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec);

				if (RET_DMX_OK != mrResult) {
					DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
						TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
					MkvFinishPrs(pvSptHdl, prCfaMkv);
				}

				return;
			}
		}

		if (prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec == CFA_AUD_DRV_FMT_VORBIS) {
			vCfaMkvVorbisPageNumCal(prCfaMkv);

			if (prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].fgHaveTxHeader == FALSE) {
				prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].fgHaveTxHeader  = TRUE;

				vCfaMkvSendVorbisIdHead(pvSptHdl, prCfaMkv);
			} else
				vCfaMkvSendOggHdr(pvSptHdl, (u64)0, prCfaMkv);

			return;
		}
	}

	if ((prCfaMkv->rCurBlock.eBlockType == CFA_PRS_BIT_STRM_TYPE_V) &&
		((prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_H264) ||
		(prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_H265)) &&
		(prCfaMkv->rAbnormalFlags.fgFourCCH264H265 == FALSE)) {
		prCfaMkv->u8Ca = prCfaMkv->rCurBlock.u8DataOfst;
		prCfaMkv->u8HdrLen = 0;
		if (prCfaMkv->rVidStmInfo.rCfaMkvHeader.fgHeaderStriping)
			prCfaMkv->fgNeedAddHeadStrip = TRUE;

		TxAVCHEVCStartCode(pvSptHdl, prCfaMkv);

		return;
	}
	if ((prCfaMkv->rVidStmInfo.fgFirstVC1 == FALSE) &&
		(prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_VC1) &&
		(prCfaMkv->rCurBlock.eBlockType == CFA_PRS_BIT_STRM_TYPE_V)) {
		ToNextState(pvSptHdl, prCfaMkv, (u64)1, CFA_MKV_ST_WVC1_TXMODE_ANA);
		return;
	}

	if ((prCfaMkv->fgTxVC1SeqHdr == FALSE) && (prCfaMkv->fgWVC1TxHeader == TRUE) &&
		(prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_VC1) &&
		(prCfaMkv->rCurBlock.eBlockType == CFA_PRS_BIT_STRM_TYPE_V)) {
		TxWVC1Header(pvSptHdl, prCfaMkv);
		return;
	}

	if ((prCfaMkv->fgTxVC1SeqHdr == TRUE) && (prCfaMkv->fgWVC1TxHeader == TRUE) &&
		(prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_VC1) &&
		(prCfaMkv->rCurBlock.eBlockType == CFA_PRS_BIT_STRM_TYPE_V)) {
		prCfaMkv->fgTxVC1SeqHdr = FALSE;
	}

	prCfaMkv->eCurState = CFA_MKV_ST_TX_BLOCK;

	switch (prCfaMkv->rCurBlock.eBlockType) {
	case  CFA_PRS_BIT_STRM_TYPE_V: {
		CFA_VIDEO_INFO_T rVidInf = {0};

		if (prCfaMkv->rVidStmInfo.rCfaMkvHeader.fgHeaderStriping) {
			TxHeader2Fifo(pvSptHdl, prCfaMkv);
			return;
		}

		if (prCfaMkv->rAbnormalFlags.fgNoSeqHdr) {
			prCfaMkv->u8Ca = prCfaMkv->rCurBlock.u8DataOfst;
			if (prCfaMkv->rCurBlock.u8DataSize >= CFA_MKV_FIRST_FRAME_READ_BYTES) {
				ToNextState(pvSptHdl, prCfaMkv, CFA_MKV_FIRST_FRAME_READ_BYTES,
							CFA_MKV_ST_READ_FIRST_FRAME);
			} else {
				ToNextState(pvSptHdl, prCfaMkv, prCfaMkv->rCurBlock.u8DataSize,
							CFA_MKV_ST_READ_FIRST_FRAME);
			}
			return;
		}

		rVidInf.eVidType	 = prCfaMkv->rVidStmInfo.eVidCodec;
		rVidInf.u8FileOfst	 = prCfaMkv->rCurBlock.u8DataOfst;
		rVidInf.u8Len		 = prCfaMkv->rCurBlock.u8DataSize;
		rVidInf.u4PrsStrmId  = (u32)prCfaMkv->rCurBlock.u8TrackNum;
		rVidInf.u8TotalAULen = prCfaMkv->rCurBlock.u8DataSize;
		rVidInf.fgUnitStart  = TRUE;
		if ((prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_DIVX3) ||
			(prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_WMV7) ||
			(prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_WMV8) ||
			(prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_WMV9) ||
			(prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_VC1) ||
			(prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_H263_SORENSON) ||
			(prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_VP8) ||
			(prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_VP6))
			rVidInf.eTxMode = prCfaMkv->eCfaMkvTxMode;

		prCfaMkv->fgSecondFillAU = FALSE;
		CfaMkvPbb2FifoAUCtrl(prCfaMkv, pvSptHdl, (void *)&rVidInf, CFA_PRS_BIT_STRM_TYPE_V, (u64)0);
	}
	break;

	case CFA_PRS_BIT_STRM_TYPE_A: {
			CFA_AUDIO_INFO_T rAudInf = {0};
			if (prCfaMkv->arAudStmInfo[idx].rCfaMkvHeader.fgHeaderStriping &&
				(prCfaMkv->rCurBlock.u8TimeCode >= prCfaMkv->rRange.u8TargetTimeCode)) {
				prCfaMkv->rCurBlock.u8NextTxOfst = prCfaMkv->rCurBlock.u8DataOfst;
				TxHeader2Fifo(pvSptHdl, prCfaMkv);
				return;
			}
			rAudInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;
			rAudInf.eAudType = prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec;
			rAudInf.u8FileOfst = prCfaMkv->rCurBlock.u8DataOfst;
			rAudInf.u8Len = prCfaMkv->rCurBlock.u8DataSize;
			rAudInf.u8Pts = prCfaMkv->rCurBlock.u8Pts;
			CfaMkvPbb2FifoAUCtrl(prCfaMkv, pvSptHdl, (void *)&rAudInf, CFA_PRS_BIT_STRM_TYPE_A, (u64)0);
		}
		break;

	case CFA_PRS_BIT_STRM_TYPE_SP: {
			CFA_SUBPIC_INFO_T rSpInf = {0};
#if CONFIG_CFA_MKV_SUPPORT_DECOMPRESSION
			if (prCfaMkv->arCfaMkvSpStreamInfo[idx].rCfaMkvContentEncoding.eCfaMKvEncodingCompression
				== CFA_MKV_ENCODING_COMP_ZLIB) {
				prCfaMkv->u8Ca = prCfaMkv->rCurBlock.u8DataOfst;
				ToNextState(pvSptHdl, prCfaMkv, prCfaMkv->rCurBlock.u8DataSize,
							CFA_MKV_ST_DECOMPRESSION);
				return;
			}
#endif
			rSpInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;
			rSpInf.u8FileOfst = prCfaMkv->rCurBlock.u8DataOfst;
			rSpInf.u8Len = prCfaMkv->rCurBlock.u8DataSize;
			rSpInf.u8Pts = prCfaMkv->rCurBlock.u8Pts;
			rSpInf.u8EndPts = prCfaMkv->rCurBlock.u8EndPts;
			rSpInf.fgUnitStart = FALSE;
			CfaMkvPbb2FifoAUCtrl(prCfaMkv, pvSptHdl, (void *)&rSpInf, CFA_PRS_BIT_STRM_TYPE_SP, (u64)0);
			prCfaMkv->rCurBlock.fgDataReady = FALSE;
			prCfaMkv->rCurBlock.fgDurationReady = FALSE;
		}
		break;

	default:
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] stream type error! not A,V,SP\r\n"));
	}

	prCfaMkv->eCurPrsStrm = prCfaMkv->rCurBlock.eBlockType;
	prCfaMkv->rCurBlock.fgReady = FALSE;/*maybe sth wrong here*/

}


/*-----------------------------------------------------------------------------
* Name: eGetTrackType
*
* Description:
*	   1.Identify this track is audio or video or subtitle by tracknum
*	   2.get index of this A/SP track in their config array
*
*
* Inputs:
*
* Outputs:
*
* Returns: enum
*
*-----------------------------------------------------------------------------*/
CfaPrsBitStrmType GetTrackType(CfaMkvInst *prCfaMkv, u64 u8TrackNum)
{
	u32 u4Index = 0;

	if (u8TrackNum == prCfaMkv->rVidStmInfo.u8VidTrackNo)
		return CFA_PRS_BIT_STRM_TYPE_V;

	for (u4Index = 0; u4Index < prCfaMkv->u4AudNum; u4Index++) {
		if (u8TrackNum == prCfaMkv->arAudStmInfo[u4Index].u8AudTrackNo) {
			prCfaMkv->rCurBlock.ucStrmIdx = (u8)u4Index;
			return CFA_PRS_BIT_STRM_TYPE_A;
		}
	}
	for (u4Index = 0; u4Index < prCfaMkv->u4SpNum; u4Index++) {
		if (u8TrackNum == prCfaMkv->arSpStmInfo[u4Index].u8SpTrackNo) {
			prCfaMkv->rCurBlock.ucStrmIdx = (u8)u4Index;
			return CFA_PRS_BIT_STRM_TYPE_SP;
		}
	}
	return CFA_PRS_BIT_STRM_TYPE_NONE;
}




/*-----------------------------------------------------------------------------
* Name: i4MkvIdentifyID
*
* Description:
*	   MKV CFA get element type bu compare ID with aucMkvId[][]
*	   aucMkvId[0] to aucMkvId[7] are one bytes ID
*	   aucMkvId[8] to aucMkvId[17] are 4 bytes ID
*	   the last byte "0xff" are end flag which will never encountered in IDs
*
* Inputs:
*
* Outputs:
*
* Returns: s32
*
*-----------------------------------------------------------------------------*/
static u32 MkvIdentifyID(void *pvSptHdl, CfaMkvInst *prCfaMkv)
{
	s32 i4Index = 0;
	s32 i4InterIndex = 0;

	for (; i4Index < CFA_MKV_ID_LINE; i4Index++) {
		i4InterIndex = 0;
		while (aucMkvId[i4Index][i4InterIndex] != 0xff) {
			if (aucMkvId[i4Index][i4InterIndex] != *(prCfaMkv->aucHdr + i4InterIndex))
				break;

			i4InterIndex++;

			if (i4InterIndex >= CFA_MKV_ID_COL)
				break;
		}
		if ((i4Index <= 9) && (i4Index >= 0) && (i4InterIndex == 1)) {
			/*from 0 to 9 are EBML IDs of one bytes*/
			MkvMoveHdr(prCfaMkv, (u32)1);
			return (u32)i4Index;
		} else if ((i4Index >= 10) && (i4Index <= 19) && (i4InterIndex == 4)) {
			/*from 10 to 19 are EBML IDs of 4 bytes*/
			MkvMoveHdr(prCfaMkv, (u32)4);
			return (u32)i4Index;
		}
#if CONFIG_CFA_MKV_SUPPORT_DRM
		else if ((i4Index == 20) && (i4InterIndex == 1)) {
			/* 20 is EBML IDs of one bytes*/
			MkvMoveHdr(prCfaMkv, (u32)1);
			return (u32)i4Index;
		}
#endif
		else
			continue;
	}

	MoveUnknownElementID(pvSptHdl, prCfaMkv);

	return (u32)MKV_ID_UNKNOW;

}


/*-----------------------------------------------------------------------------
* Name: MkvGetSize
*
* Description:
*	   Compute start size of varaible int
*
*
* Inputs:
*
* Outputs:
*
* Returns: void
*
*-----------------------------------------------------------------------------*/
static u32 MkvGetSize(const u8 *pucBuff, u64 *pu8ReturnSize,
						CfaMkvInst *prCfaMkv)
{
	u64 u8SizeLen;
	u8 ucSizeHdr;

	u8SizeLen = prCfaMkv->u8HdrLen;
	ucSizeHdr = *pucBuff;
	if ((ucSizeHdr < (u8)0x10) && (ucSizeHdr >= (u8)0x08)) {
		if (u8SizeLen >= (u64)5) {
			*pu8ReturnSize = (MkvComputeEndian(prCfaMkv->aucHdr, (u8)5) - (u64)0x0800000000LL);
			MkvMoveHdr(prCfaMkv, (u32)5);
			return 0;
		}
		{
			*pu8ReturnSize = 0;
			return (u32)((u32)5 - (u32)u8SizeLen);
		}
	} else if ((ucSizeHdr < (u8)0x08) && (ucSizeHdr >= (u8)0x04)) {
		if (u8SizeLen >= (u64)6) {
			*pu8ReturnSize = (MkvComputeEndian(prCfaMkv->aucHdr, (u8)6) - (u64)0x040000000000LL);
			MkvMoveHdr(prCfaMkv, (u32)6);
			return 0;
		}
		{
			*pu8ReturnSize = 0;
			return (u32)((u32)6 - (u32)u8SizeLen);
		}
	} else if ((ucSizeHdr < (u8)0x04) && (ucSizeHdr >= (u8)0x02)) {
		if (u8SizeLen >= (u64)7) {
			*pu8ReturnSize = (MkvComputeEndian(prCfaMkv->aucHdr, (u8)7) - (u64)0x02000000000000LL);
			MkvMoveHdr(prCfaMkv, (u32)7);
			return 0;
		}
		{
			*pu8ReturnSize = 0;
			return (u32)((u32)7 - (u32)u8SizeLen);
		}
	} else if (ucSizeHdr == (u8)0x01) {
		if (u8SizeLen >= (u64)8) {
			*pu8ReturnSize = (MkvComputeEndian(prCfaMkv->aucHdr, (u8)8) - (u64)0x0100000000000000LL);
			MkvMoveHdr(prCfaMkv, (u32)8);
			return 0;
		}
		{
			*pu8ReturnSize = 0;
			return (u32)((u32)8 - (u32)u8SizeLen);
		}
	} else if (ucSizeHdr == 0x00) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT, TEXT("[CFA_MKV_ST_CTRL]Size Error!"));
		if (u8SizeLen >= (u64)9) {
			*pu8ReturnSize = (MkvComputeEndian((prCfaMkv->aucHdr + (u8)1), (u8)8));
			MkvMoveHdr(prCfaMkv, (u32)9);
			return 0;
		}
		{
			*pu8ReturnSize = 0;
			return (u32)((u32)9 - (u32)u8SizeLen);
		}

	} else if ((ucSizeHdr < (u8)0x20) && (ucSizeHdr >= (u8)0x10)) {
		if (u8SizeLen >= (u64)4) {
			*pu8ReturnSize = (MkvComputeEndian(prCfaMkv->aucHdr, (u8)4) - (u64)0x10000000);
			MkvMoveHdr(prCfaMkv, (u32)4);
			return 0;
		}
		{
			*pu8ReturnSize = 0;
			return (u32)((u32)4 - (u32)u8SizeLen);
		}
	} else if ((ucSizeHdr < (u8)0x40) && (ucSizeHdr >= (u8)0x20)) {
		if (u8SizeLen >= (u64)3) {
			*pu8ReturnSize = (MkvComputeEndian(prCfaMkv->aucHdr, (u8)3) - (u64)0x200000);
			MkvMoveHdr(prCfaMkv, (u32)3);
			return 0;
		}
		{
			*pu8ReturnSize = 0;
			return (u32)((u32)3 - (u32)u8SizeLen);
		}
	} else if ((ucSizeHdr < (u8)0x80) && (ucSizeHdr >= (u8)0x40)) {
		if (u8SizeLen >= (u64)2) {
			*pu8ReturnSize = (MkvComputeEndian(prCfaMkv->aucHdr, (u8)2) - (u64)0x4000);
			MkvMoveHdr(prCfaMkv, (u32)2);
			return 0;
		}
		{
			*pu8ReturnSize = 0;
			return (u32)((u32)2 - (u32)u8SizeLen);
		}
	} else {
		if (u8SizeLen >= (u64)1) {
			*pu8ReturnSize = (MkvComputeEndian(prCfaMkv->aucHdr, (u8)1) - (u64)0x80);
			MkvMoveHdr(prCfaMkv, (u32)1);
			return 0;
		}
		{
			*pu8ReturnSize = 0;
			return (u32)((u32)1 - (u32)u8SizeLen);
		}
	}
}


/*-----------------------------------------------------------------------------
* Name: ucMkvCountLacing
*
* Description:
*	   Get	frame amount  by lacing at aucHdr[0] to aucHdr[u8HdrLen - 1] and return it
*
*
* Inputs:
*
* Outputs:
*
* Returns: u8
*
*-----------------------------------------------------------------------------*/
u8  MkvCountLacing(void *pvSptHdl,CfaMkvInst *prCfaMkv)
{
	/*u8 ucLoop = 0;*/
	u64 u8Len = prCfaMkv->u8HdrLen;
	u8 ucReturnFrame = 0;
	u8 ucSize = 0;
	u64 u8TempFrameSize = 0;
	s32 i4FrameSub = 0;

	switch (prCfaMkv->rCurBlock.eLacingType) {
	case CFA_MKV_XIPH_LACING:
		while (u8Len > 0) {
			if (prCfaMkv->rCurBlock.i4FrameNum >= CFA_MKV_MAX_FRAME_NUM) {
				DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA_MKV_ST_CTRL] line %d, FrameNum is larger than 128 ,may cause memory problem! \r\n"), DMX_LINE_NO);                
				if (DMX_IS_RW_PLAY(pvSptHdl)) {
					Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, FALSE, GAU_E_ERRCHUNK);
				} else {
					Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, TRUE, GAU_E_ERRCHUNK);
				}
				return 0;
			}
			prCfaMkv->rCurBlock.au4FrameSize[prCfaMkv->rCurBlock.i4FrameNum] +=
				prCfaMkv->aucHdr[prCfaMkv->u8HdrLen - u8Len];
			if (prCfaMkv->aucHdr[prCfaMkv->u8HdrLen - u8Len] < CFA_MKV_XIPH_LACING_SEGMENT) {
				ucReturnFrame++;
				prCfaMkv->rCurBlock.i4FrameNum++;
			}
			u8Len--;
		}
		MkvMoveHdr(prCfaMkv, (u32)prCfaMkv->u8HdrLen);
		return ucReturnFrame;

	case CFA_MKV_EBML_LACING:
		while (u8Len != 0) {
			ucSize = GetVaraibleSize(prCfaMkv->aucHdr);
			if (u8Len >= ucSize) {
				if (prCfaMkv->rCurBlock.i4FrameNum == 0) {
					MkvGetSize(prCfaMkv->aucHdr,
				(u64 *)(&(prCfaMkv->rCurBlock.au4FrameSize[prCfaMkv->rCurBlock.i4FrameNum])),
					prCfaMkv);
				} else {
					MkvGetSize(prCfaMkv->aucHdr, &u8TempFrameSize, prCfaMkv);
					i4FrameSub = (s32)(u8TempFrameSize - (_au8VsintSub[ucSize - (u8)1]));
					i4FrameSub +=
					(s32)(prCfaMkv->rCurBlock.au4FrameSize[prCfaMkv->rCurBlock.i4FrameNum - 1]);
					prCfaMkv->rCurBlock.au4FrameSize[prCfaMkv->rCurBlock.i4FrameNum] = i4FrameSub;
				}
				ucReturnFrame++;
				u8Len -= ucSize;
				prCfaMkv->rCurBlock.i4FrameNum++;
			} else {
				break;
			}

		}
		return ucReturnFrame;

	default:
		break;
	}
	return 0;
}


/*-----------------------------------------------------------------------------
* Description:
*	   Compute start PTS of this block,and also end PTS for subtitle
*
*
* Inputs:
*
* Outputs:
*
* Returns: void
*
*-----------------------------------------------------------------------------*/
void MkvGetPts(void *pvSptHdl, CfaMkvInst *prCfaMkv)
{
	CfaMkvBlockInfo *prCurBlk = &(prCfaMkv->rCurBlock);
	u64 u8TC = prCurBlk->u8TimeCode;

	switch (prCurBlk->eBlockType) {
	case CFA_PRS_BIT_STRM_TYPE_V:
		if (prCfaMkv->rVidStmInfo.fgTimeCodeScaleEn &&
			(0 != prCfaMkv->rVidStmInfo.u8TrackTimeCodeScale))
			u8TC *= prCfaMkv->rVidStmInfo.u8TrackTimeCodeScale;

		prCurBlk->u8Pts = MS2PTS(u8TC * prCfaMkv->u8TimeCodeScale / MKV_NS_TO_MS);

		if (prCurBlk->u8Pts > (prCfaMkv->u8LastVPts + (u64)((u64)10 * (u64)60 * MKV_PTS_1S))) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] video pts is too large\r\n"));
			prCurBlk->u8Pts = prCfaMkv->u8LastVPts + (u64)(MKV_PTS_1S / (u64)30);
		}

		prCfaMkv->u8LastVPts = prCurBlk->u8Pts;

		break;

	case CFA_PRS_BIT_STRM_TYPE_A:
		if (prCurBlk->ucStrmIdx >= MAX_NS_MKV_AUD) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] ucStrmIdx is %d, line %d Send EOS\r\n"),
				prCurBlk->ucStrmIdx, DMX_LINE_NO);
			MkvFinishPrs(pvSptHdl, prCfaMkv);
			return ;
		}

		if (prCfaMkv->arAudStmInfo[prCurBlk->ucStrmIdx].fgTimeCodeScaleEn)
			u8TC *= prCfaMkv->arAudStmInfo[prCurBlk->ucStrmIdx].u8TrackTimeCodeScale;

		prCurBlk->u8Pts = MS2PTS(u8TC * prCfaMkv->u8TimeCodeScale / MKV_NS_TO_MS);

		if (prCurBlk->u8Pts > (prCfaMkv->u8LastAPts + (u64)((u64)10 * (u64)60 * MKV_PTS_1S))) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] audio pts is too large\r\n"));
			prCurBlk->u8Pts = prCfaMkv->u8LastAPts + (u64)(MKV_PTS_1S / (u64)30);
		}

		prCfaMkv->u8LastAPts = prCurBlk->u8Pts;

		break;

	case CFA_PRS_BIT_STRM_TYPE_SP:
		if (prCurBlk->ucStrmIdx >= MAX_NS_MKV_SP) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] ucStrmIdx is %d,line %d Send EOS\r\n"),
				prCurBlk->ucStrmIdx, DMX_LINE_NO);
			MkvFinishPrs(pvSptHdl, prCfaMkv);
			return;
		}

		if (prCfaMkv->arSpStmInfo[prCurBlk->ucStrmIdx].fgTimeCodeScaleEn)
			u8TC *= prCfaMkv->arSpStmInfo[prCurBlk->ucStrmIdx].u8TrackTimeCodeScale;

		prCurBlk->u8Pts = MS2PTS(u8TC * prCfaMkv->u8TimeCodeScale / MKV_NS_TO_MS);
		prCurBlk->u8EndPts = MS2PTS(((u8TC + prCurBlk->u8TimeDuration)
			* prCfaMkv->u8TimeCodeScale) / MKV_NS_TO_MS);
		break;

	case CFA_PRS_BIT_STRM_TYPE_NONE:
	default:
		break;
	}
}

void MkvSkipBlock(void *pvSptHdl, CfaMkvInst *prCfaMkv)
{
	prCfaMkv->u8Ca = prCfaMkv->rCurGroup.u8GroupOfst + prCfaMkv->rCurGroup.u8GroupSize;
	prCfaMkv->u8HdrLen = 0;

	ToNextState(pvSptHdl, prCfaMkv, (u64)CFA_MKV_HDR_READ_BYTES, CFA_MKV_ST_SC_ANA);
}


/*-----------------------------------------------------------------------------
* Name: vMkvProcCluster
*
* Description:
*	   Get demux information of a whole cluster
*
*
* Inputs:
*
* Outputs:
*
* Returns: void
*
*-----------------------------------------------------------------------------*/
void MkvProcCluster(void *pvSptHdl, CfaMkvInst *prCfaMkv)
{
	prCfaMkv->rCurCluster.u8TimeCode = DMX_INVALID_UINT64;
	prCfaMkv->rCurBlock.u4BlockNo = 0;
	if (prCfaMkv->rCurCluster.u8ClusterOfst < prCfaMkv->rRange.u8VidStartOfst) {
		prCfaMkv->rCurCluster.fgPrsVid = FALSE;
		prCfaMkv->rCurCluster.fgFirstVid = FALSE;
	} else if (prCfaMkv->rCurCluster.u8ClusterOfst == prCfaMkv->rRange.u8VidStartOfst) {
		prCfaMkv->rCurCluster.fgPrsVid = TRUE;
		prCfaMkv->rCurCluster.fgFirstVid = TRUE;
	} else if ((prCfaMkv->rCurCluster.u8ClusterOfst > prCfaMkv->rRange.u8VidStartOfst) &&
		(prCfaMkv->rCurCluster.u8ClusterOfst < prCfaMkv->rRange.u8VidEndOfst)) {
		prCfaMkv->rCurCluster.fgPrsVid = TRUE;
		prCfaMkv->rCurCluster.fgFirstVid = FALSE;
	} else {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
		return;
	}

	if (prCfaMkv->rCurCluster.u8ClusterOfst < prCfaMkv->rRange.u8AudStartOfst) {
		prCfaMkv->rCurCluster.fgPrsAud	 = FALSE;
		prCfaMkv->rCurCluster.fgFirstAud = FALSE;
	} else if (prCfaMkv->rCurCluster.u8ClusterOfst == prCfaMkv->rRange.u8AudStartOfst) {
		prCfaMkv->rCurCluster.fgPrsAud	 = TRUE;
		prCfaMkv->rCurCluster.fgFirstAud = TRUE;
	} else if ((prCfaMkv->rCurCluster.u8ClusterOfst > prCfaMkv->rRange.u8AudStartOfst) &&
		(prCfaMkv->rCurCluster.u8ClusterOfst < prCfaMkv->rRange.u8AudEndOfst)) {
		prCfaMkv->rCurCluster.fgPrsAud	 = TRUE;
		prCfaMkv->rCurCluster.fgFirstAud = FALSE;
	} else {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
		return;
	}

	ToNextState(pvSptHdl, prCfaMkv, (u64)CFA_MKV_HDR_READ_BYTES - (prCfaMkv->u8HdrLen),
				CFA_MKV_ST_SC_ANA);

}





/*-----------------------------------------------------------------------------
* Name: vCfaMkvAnaStIdle
*
* Description:
*	   MKV CFA processes CFA_MKV_ANA_ST_IDLE
*
* Inputs:
*	   [IN] uintptr_t of splitter
*	   [IN] Actual transferred data length.  Normally this value should be equal to the u8TxLen in
*			 the previous transfer issue, unless file end is hit.
*	   [IN] pointer to CfaMkvInst
*
* Outputs:
*
* Returns:
*
*-----------------------------------------------------------------------------*/
static void CfaMkvAnaStIdle(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	/* do nothing */
}


/*-----------------------------------------------------------------------------
* Name: vMkvGetSize
*
* Description:
*	  Optional state for get size when need rebuffer
*
*
* Inputs:
*
* Outputs:
*
* Returns: void
*
*-----------------------------------------------------------------------------*/
static void CfaMkvAnaSize(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	u64 u8ElementSize = 0;

	if ((prCfaMkv->u8HdrLen + u8TxLen) > CFA_MKV_HDR_BUF) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] u8HdrLen is %lld, u8TxLen is %lld,line %d Send EOS\r\n"),
			prCfaMkv->u8HdrLen, u8TxLen, DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl,prCfaMkv);
		return;
	}

	dmx_memcpy((void *)(prCfaMkv->aucHdr + prCfaMkv->u8HdrLen),
					   (void *)prCfaMkv->ptrMemAddr, (u32)u8TxLen);

	prCfaMkv->u8HdrLen += u8TxLen;

	switch (prCfaMkv->eCurElement) {
	case MKV_ID_CLUSTER:
		MkvGetSize(prCfaMkv->aucHdr, &u8ElementSize, prCfaMkv);
		prCfaMkv->rCurCluster.u8ClusterSize = u8ElementSize;
		MkvProcCluster(pvSptHdl, prCfaMkv);

		break;

	case MKV_ID_BLOCK_GROUP:
		MkvGetSize(prCfaMkv->aucHdr, &u8ElementSize, prCfaMkv);
		prCfaMkv->rCurGroup.u8GroupSize = u8ElementSize;
		prCfaMkv->rCurGroup.u8GroupOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen;

		ToNextState(pvSptHdl, prCfaMkv, CFA_MKV_HDR_READ_BYTES - (prCfaMkv->u8HdrLen),
					CFA_MKV_ST_SC_ANA);
		break;

	case MKV_ID_ATTACHMENTS:
	case MKV_ID_UNKNOW:
		MkvGetSize(prCfaMkv->aucHdr, &u8ElementSize, prCfaMkv);
		prCfaMkv->rSkip.u8Length = u8ElementSize;
		prCfaMkv->rSkip.u8ElementOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen;

		prCfaMkv->u8Ca = prCfaMkv->rSkip.u8ElementOfst + prCfaMkv->rSkip.u8Length;
		prCfaMkv->u8HdrLen = 0;
		prCfaMkv->rSkip.fgEnable = FALSE;

		ToNextState(pvSptHdl, prCfaMkv, CFA_MKV_HDR_READ_BYTES,
					CFA_MKV_ST_SC_ANA);
		break;

	case MKV_ID_TIME_CODE:

		if ((prCfaMkv->rRange.u8TargetTimeCode == 0) &&
			(prCfaMkv->rCurCluster.fgClusterGetTimeode == FALSE)) {
			prCfaMkv->rCurCluster.u8TimeCode = 0;
			prCfaMkv->rCurCluster.fgClusterGetTimeode = TRUE;

			prCfaMkv->rCurCluster.u8LastClusterTimecodePhyics =
				MkvComputeEndian(prCfaMkv->aucHdr, (u8)prCfaMkv->u8HdrLen);
			prCfaMkv->rCurCluster.u8LastClusterTimecode = 0;
		} else {
			prCfaMkv->rCurCluster.u8TimeCode =
				MkvComputeEndian(prCfaMkv->aucHdr, (u8)prCfaMkv->u8HdrLen);
			if (prCfaMkv->rCurCluster.u8TimeCode == prCfaMkv->rCurCluster.u8LastClusterTimecodePhyics)
				prCfaMkv->rCurCluster.u8TimeCode = prCfaMkv->rCurCluster.u8LastClusterTimecode;
		}

		prCfaMkv->u8HdrLen = 0;

		ToNextState(pvSptHdl, prCfaMkv,    CFA_MKV_HDR_READ_BYTES,
					CFA_MKV_ST_SC_ANA);
		break;

	case MKV_ID_SIMPLE_BLOCK:
		MkvGetSize(prCfaMkv->aucHdr, &u8ElementSize, prCfaMkv);
		prCfaMkv->rCurGroup.u8GroupSize = u8ElementSize;
		prCfaMkv->rCurGroup.u8GroupOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen;
		prCfaMkv->rCurBlock.u8BlockSize = prCfaMkv->rCurGroup.u8GroupSize;
		prCfaMkv->rCurBlock.u8BlockOfst = prCfaMkv->rCurGroup.u8GroupOfst;

		ToNextState(pvSptHdl, prCfaMkv, CFA_MKV_BLOCK_READ_BYTES - (prCfaMkv->u8HdrLen),
					CFA_MKV_ST_BLOCK);

		break;

	default:
		break;
	}
}


/*-----------------------------------------------------------------------------
* Name: vCfaMkvAnaStartCode
*
* Description:
*	   Analyse start code for any element ID
*
*
* Inputs:
*
* Outputs:
*
* Returns: void
*
*-----------------------------------------------------------------------------*/
static void CfaMkvAnaStartCode(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	u32 i4ElementId = DMX_INVALID_UINT32;
	u64 u8ElementSize = DMX_INVALID_UINT64;
	u32 u4ReturnBytes = 0;/*function GetSize return value*/

	if ((prCfaMkv->u8HdrLen + u8TxLen) > CFA_MKV_HDR_BUF) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] u8HdrLen is %lld, u8TxLen is %lld,line %d Send EOS\r\n"),
			prCfaMkv->u8HdrLen, u8TxLen, DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
		return;
	}

	dmx_memcpy((void *)(prCfaMkv->aucHdr + prCfaMkv->u8HdrLen),
					   (void *)prCfaMkv->ptrMemAddr, (u32)u8TxLen);
	prCfaMkv->u8HdrLen += u8TxLen;

	i4ElementId = MkvIdentifyID(pvSptHdl, prCfaMkv);

	if (prCfaMkv->fgFindIdx) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] CfaMkvAnaStartCode::emulate seeking and find index\r\n"));
		prCfaMkv->fgFindIdx = FALSE;
		return;
	}
	/*DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,TEXT("[CFA_MKV_ST_CTRL] i4ElementId=%d!\r\n"),i4ElementId);*/
	if (prCfaMkv->fgCheckRangeCluster) {
		prCfaMkv->fgCheckRangeCluster = FALSE;
		if (i4ElementId != MKV_ID_CLUSTER) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] find invalid data and Seek\r\n"));
			if (DMX_IS_RW_PLAY(pvSptHdl)) {
				Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, FALSE, GAU_E_ERRCHUNK);
			} else {
				Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, TRUE, GAU_E_ERRCHUNK);
			}
			return;
		}
	}

	switch (i4ElementId) {
	case MKV_ID_CLUSTER:
		prCfaMkv->eCurElement = MKV_ID_CLUSTER;
		prCfaMkv->rCurCluster.fgClusterEn = TRUE;
		prCfaMkv->rCurCluster.u8ClusterOfst = (prCfaMkv->u8Ca - CFA_MKV_HDR_READ_BYTES);

		u4ReturnBytes = MkvGetSize(prCfaMkv->aucHdr, &u8ElementSize, prCfaMkv);

		if (u4ReturnBytes == 0) {
			prCfaMkv->rCurCluster.u8ClusterSize = u8ElementSize;
			MkvProcCluster(pvSptHdl, prCfaMkv);
		} else {
			ToNextState(pvSptHdl, prCfaMkv, (u64)u4ReturnBytes,
						CFA_MKV_ST_SIZE_ANA);
		}
		break;

	case MKV_ID_TIME_CODE:
		prCfaMkv->eCurElement = MKV_ID_TIME_CODE;
		u4ReturnBytes = MkvGetSize(prCfaMkv->aucHdr, &u8ElementSize, prCfaMkv);
		if (u8ElementSize <= prCfaMkv->u8HdrLen) {
			if ((prCfaMkv->rRange.u8TargetTimeCode == 0) &&
				(prCfaMkv->rCurCluster.fgClusterGetTimeode == FALSE)) {
				prCfaMkv->rCurCluster.u8TimeCode = 0;
				prCfaMkv->rCurCluster.fgClusterGetTimeode = TRUE;
			} else {
				prCfaMkv->rCurCluster.u8TimeCode =
				MkvComputeEndian(prCfaMkv->aucHdr, (u8)u8ElementSize);
			}
			MkvMoveHdr(prCfaMkv, (u32)u8ElementSize);

			ToNextState(pvSptHdl, prCfaMkv,    CFA_MKV_HDR_READ_BYTES - (prCfaMkv->u8HdrLen),
						CFA_MKV_ST_SC_ANA);

		} else {
			ToNextState(pvSptHdl, prCfaMkv, u8ElementSize - prCfaMkv->u8HdrLen,
						CFA_MKV_ST_SIZE_ANA);
		}

		break;

	case MKV_ID_SIMPLE_BLOCK:
		prCfaMkv->eCurElement = MKV_ID_SIMPLE_BLOCK;
		u4ReturnBytes = MkvGetSize(prCfaMkv->aucHdr, &u8ElementSize, prCfaMkv);
		if (u4ReturnBytes == 0) {
			prCfaMkv->rCurGroup.u8GroupSize = u8ElementSize;
			prCfaMkv->rCurGroup.u8GroupOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen;
			prCfaMkv->rCurBlock.u8BlockSize = prCfaMkv->rCurGroup.u8GroupSize;
			prCfaMkv->rCurBlock.u8BlockOfst = prCfaMkv->rCurGroup.u8GroupOfst;
			ToNextState(pvSptHdl, prCfaMkv, CFA_MKV_BLOCK_READ_BYTES - (prCfaMkv->u8HdrLen),
						CFA_MKV_ST_BLOCK);

		} else {
			ToNextState(pvSptHdl, prCfaMkv,    (u64)u4ReturnBytes,
						CFA_MKV_ST_SIZE_ANA);
		}
		break;

	case MKV_ID_BLOCK_GROUP:
		prCfaMkv->eCurElement = MKV_ID_BLOCK_GROUP;
		prCfaMkv->rCurGroup.fgGroupEn = TRUE;
		u4ReturnBytes = MkvGetSize(prCfaMkv->aucHdr, &u8ElementSize, prCfaMkv);

		if (u4ReturnBytes == 0)/*after get size,offset can be correct*/ {
			prCfaMkv->rCurGroup.u8GroupSize = u8ElementSize;
			prCfaMkv->rCurGroup.u8GroupOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen;
			ToNextState(pvSptHdl, prCfaMkv,    CFA_MKV_HDR_READ_BYTES - (prCfaMkv->u8HdrLen),
						CFA_MKV_ST_SC_ANA);

		} else {
			ToNextState(pvSptHdl, prCfaMkv,    (u64)u4ReturnBytes,
						CFA_MKV_ST_SIZE_ANA);
		}
		break;

	case MKV_ID_BLOCK_DURATION: {
			prCfaMkv->eCurElement = MKV_ID_BLOCK_DURATION;
			u4ReturnBytes = MkvGetSize(prCfaMkv->aucHdr, &u8ElementSize, prCfaMkv);
			prCfaMkv->rCurBlock.u8TimeDuration = MkvComputeEndian(prCfaMkv->aucHdr, (u8)u8ElementSize);
			MkvMoveHdr(prCfaMkv, (u32)u8ElementSize);
			prCfaMkv->rCurBlock.fgDurationReady = TRUE;
			if (!prCfaMkv->rCurBlock.fgDataReady) {
				ToNextState(pvSptHdl, prCfaMkv, CFA_MKV_HDR_READ_BYTES - (prCfaMkv->u8HdrLen),
							CFA_MKV_ST_SC_ANA);
			} else
				TxBlock2Fifo(pvSptHdl, prCfaMkv);
		}
		break;

#if CONFIG_CFA_MKV_SUPPORT_DRM
	case MKV_ID_DRMINFO:
		prCfaMkv->eCurElement = MKV_ID_DRMINFO;
		u4ReturnBytes = MkvGetSize(prCfaMkv->aucHdr, &u8ElementSize, prCfaMkv);
		prCfaMkv->u8Ca = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen;
		prCfaMkv->u8HdrLen = 0;
		ToNextState(pvSptHdl, prCfaMkv, MKV_DRM_CHUNK_BYTES,
					CFA_MKV_ST_PARSER_DRM_INFO);
		break;
#endif

	case MKV_ID_BLOCK:
		prCfaMkv->eCurElement = MKV_ID_BLOCK;
		u4ReturnBytes = MkvGetSize(prCfaMkv->aucHdr, &u8ElementSize, prCfaMkv);
		prCfaMkv->rCurBlock.u8BlockSize = u8ElementSize;
		prCfaMkv->rCurBlock.u8BlockOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen;

		if ((prCfaMkv->rCurBlock.u8BlockSize == 4) ||
			(prCfaMkv->rCurBlock.u8BlockSize < 4 &&
			(prCfaMkv->rCurGroup.u8GroupSize != 0) ))/*an empty block*/ {
			prCfaMkv->u8Ca = prCfaMkv->rCurGroup.u8GroupOfst + prCfaMkv->rCurGroup.u8GroupSize;
			prCfaMkv->u8HdrLen = 0;
			prCfaMkv->rCurBlock.u4BlockNo++;
			ToNextState(pvSptHdl, prCfaMkv, CFA_MKV_HDR_READ_BYTES,
						CFA_MKV_ST_SC_ANA);
			return;
		}
		if (prCfaMkv->rCurBlock.u8BlockSize < 4) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] prCfaMkv->u8Ca = %lld, line %d Send EOS\r\n"),
				prCfaMkv->u8Ca,DMX_LINE_NO);
			prCfaMkv->fgDemuxError = TRUE;
			MkvFinishPrs(pvSptHdl, prCfaMkv);
			return;
		}

		ToNextState(pvSptHdl, prCfaMkv, ((u64)CFA_MKV_BLOCK_READ_BYTES) - (prCfaMkv->u8HdrLen),
					CFA_MKV_ST_BLOCK);
		break;


		/*all these elements should be skipped*/
	case MKV_ID_ATTACHMENTS:
	case MKV_ID_CHAPTERS:
	case MKV_ID_CUES:
	case MKV_ID_EBML:
	case MKV_ID_POSITION:
	case MKV_ID_PREV_SIZE:
	case MKV_ID_REFERENCEBLOCK:
	case MKV_ID_SEEK_HEAD:
	case MKV_ID_SEGMENT:
	case MKV_ID_SEGMENT_INFO:
	case MKV_ID_TAGS:
	case MKV_ID_TRACK:
	case MKV_ID_CRC32:
	case MKV_ID_UNKNOWM_ID:
		if (prCfaMkv->eCurState == CFA_MKV_ST_IDLE)
			return;
		/*attachments means all these useless elements for cfa*/
		prCfaMkv->eCurElement = MKV_ID_ATTACHMENTS;
		prCfaMkv->rSkip.fgEnable = TRUE;
		u4ReturnBytes = MkvGetSize(prCfaMkv->aucHdr, &u8ElementSize, prCfaMkv);
		if (u4ReturnBytes == 0) {
			prCfaMkv->rSkip.u8Length = u8ElementSize;
			prCfaMkv->rSkip.u8ElementOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen;
			if (prCfaMkv->u8Ca < prCfaMkv->rSkip.u8ElementOfst + prCfaMkv->rSkip.u8Length) {
				prCfaMkv->u8Ca = prCfaMkv->rSkip.u8ElementOfst + prCfaMkv->rSkip.u8Length;
				prCfaMkv->u8HdrLen = 0;
			} else {
				MkvMoveHdr(prCfaMkv, (u32)prCfaMkv->rSkip.u8Length);
			}
			ToNextState(pvSptHdl, prCfaMkv, CFA_MKV_HDR_READ_BYTES - (prCfaMkv->u8HdrLen),
						CFA_MKV_ST_SC_ANA);

		} else {
			ToNextState(pvSptHdl, prCfaMkv, (u64)u4ReturnBytes,
						CFA_MKV_ST_SIZE_ANA);
		}
		break;

	case MKV_ID_UNKNOW:
		if (prCfaMkv->eCurState == CFA_MKV_ST_IDLE)
			return;

		prCfaMkv->eCurElement = MKV_ID_UNKNOW;
		if ((prCfaMkv->u8Ca <
			(prCfaMkv->rCurCluster.u8ClusterOfst + prCfaMkv->rCurCluster.u8ClusterSize)) &&
			(prCfaMkv->rCurCluster.fgClusterEn == TRUE)) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] find invalid data and Seek\r\n"));
			if (DMX_IS_RW_PLAY(pvSptHdl)) {
				Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, FALSE, GAU_E_ERRCHUNK);
			} else {
				Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, TRUE, GAU_E_ERRCHUNK);
			}
			return;
		}
		prCfaMkv->rSkip.fgEnable = TRUE;
		u4ReturnBytes = MkvGetSize(prCfaMkv->aucHdr, &u8ElementSize, prCfaMkv);

		if (u4ReturnBytes == 0) {
			prCfaMkv->rSkip.u8Length = u8ElementSize;
			prCfaMkv->rSkip.u8ElementOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen;
			if (prCfaMkv->u8Ca < prCfaMkv->rSkip.u8ElementOfst + prCfaMkv->rSkip.u8Length) {
				prCfaMkv->u8Ca = prCfaMkv->rSkip.u8ElementOfst + prCfaMkv->rSkip.u8Length;
				prCfaMkv->u8HdrLen = 0;
			} else
				MkvMoveHdr(prCfaMkv, (u32)prCfaMkv->rSkip.u8Length);

			ToNextState(pvSptHdl, prCfaMkv, CFA_MKV_HDR_READ_BYTES - (prCfaMkv->u8HdrLen),
						CFA_MKV_ST_SC_ANA);

		} else {
			ToNextState(pvSptHdl, prCfaMkv, (u64)u4ReturnBytes,
						CFA_MKV_ST_SIZE_ANA);
		}
		break;

	default:
		break;
	}
}


static void CfaMkvRvGetPicPts(CfaMkvInst *prCfaMkv)
{
	s32 i4TrDelta = 0;

	switch (prCfaMkv->rVidStmInfo.eVidCodec) {
	case CFA_VID_RV30:
		prCfaMkv->u4CurPicTr = ((prCfaMkv->u4First4BytesPay & (u32)0xFFF80) >> 7);
		if ((u8)0x18 == (prCfaMkv->u1FirstBytePay & (u8)0x18)) {/*TRUEBPIC*/
			i4TrDelta = prCfaMkv->u4CurPicTr - prCfaMkv->u4ForwardRefTr;
			if (i4TrDelta < 0)
				i4TrDelta += 0x2000;

			prCfaMkv->u8PrsPts = prCfaMkv->u8ForwardRefPts + (i4TrDelta * CFA_MKV_SYS_CLK);
		} else {
			prCfaMkv->u4ForwardRefTr = prCfaMkv->u4BackwardRefTr;
			prCfaMkv->u4BackwardRefTr = prCfaMkv->u4CurPicTr;
			prCfaMkv->u8ForwardRefPts = prCfaMkv->u8BackwardRefPts;
			prCfaMkv->u8BackwardRefPts = prCfaMkv->rCurBlock.u8TimeCode * CFA_MKV_SYS_CLK;
		}
		break;

	case CFA_VID_RV40:
		prCfaMkv->u4CurPicTr = ((prCfaMkv->u4First4BytesPay & (u32)0x7FFC0) >> 6);
		if ((u8)0x60 == (prCfaMkv->u1FirstBytePay & (u8)0x60)) {/*TRUEBPIC*/
			i4TrDelta = prCfaMkv->u4CurPicTr - prCfaMkv->u4ForwardRefTr;
			if (i4TrDelta < 0)
				i4TrDelta += 0x2000;

			prCfaMkv->u8PrsPts = prCfaMkv->u8ForwardRefPts + (i4TrDelta * CFA_MKV_SYS_CLK);
		} else {
			prCfaMkv->u4ForwardRefTr = prCfaMkv->u4BackwardRefTr;
			prCfaMkv->u4BackwardRefTr = prCfaMkv->u4CurPicTr;
			prCfaMkv->u8ForwardRefPts = prCfaMkv->u8BackwardRefPts;
			prCfaMkv->u8BackwardRefPts = prCfaMkv->rCurBlock.u8TimeCode * CFA_MKV_SYS_CLK;
		}
		break;

	default:
		break;
	}

}

void CfaMkvAdjustSliceInfo(CfaMkvInst *prCfaMkv)
{
	if (CFA_VID_RV30 == prCfaMkv->rVidStmInfo.eVidCodec) {
		if ((bool)((prCfaMkv->u1FirstBytePay) & (u8)0x20) && ((prCfaMkv->rSliceInf.u1TotalSliceNum) > (u8)1))
			prCfaMkv->rSliceInf.u1TotalSliceNum--;

	} else if (CFA_VID_RV40 == prCfaMkv->rVidStmInfo.eVidCodec) {
		if ((bool)((prCfaMkv->u1FirstBytePay) & (u8)0x80) && ((prCfaMkv->rSliceInf.u1TotalSliceNum) > (u8)1))
			prCfaMkv->rSliceInf.u1TotalSliceNum--;

	} else {
		/*do
		nothing*/
	}
}


/*Description: Set picture tx mode in terms of video codec type
@Return: cfa picture tx mode. 04/10/2008*/
static CfaApiPicTxMode CfaMkvRvSetPicTxMode(CfaMkvInst *prCfaMkv)
{
	prCfaMkv->eCurPicType = CFA_PTM_EXACT_POS;

	switch (prCfaMkv->rVidStmInfo.eVidCodec) {
	case CFA_VID_RV30:
		if ((u8)0x00 == (prCfaMkv->u1FirstBytePay & (u8)0x18))
			prCfaMkv->eCurPicType = CFA_PTM_RM_INTRAPIC; /*INTRAPIC*/
		else if ((u8)0x08 == (prCfaMkv->u1FirstBytePay & (u8)0x18))
			prCfaMkv->eCurPicType = CFA_PTM_RM_FORCED_INTRAPIC; /*FORCED_INTRAPIC*/
		else if ((u8)0x10 == (prCfaMkv->u1FirstBytePay & (u8)0x18))
			prCfaMkv->eCurPicType = CFA_PTM_RM_INTERPIC; /*INTERPIC*/
		else
			prCfaMkv->eCurPicType = CFA_PTM_RM_TRUEBPIC; /*TRUEBPIC*/

		break;

	case CFA_VID_RV40:
		if ((u8)0x00 == (prCfaMkv->u1FirstBytePay & (u8)0x60))
			prCfaMkv->eCurPicType = CFA_PTM_RM_INTRAPIC; /*INTRAPIC*/
		else if ((u8)0x20 == (prCfaMkv->u1FirstBytePay & (u8)0x60))
			prCfaMkv->eCurPicType = CFA_PTM_RM_FORCED_INTRAPIC; /*FORCED_INTRAPIC*/
		else if ((u8)0x40 == (prCfaMkv->u1FirstBytePay & (u8)0x60))
			prCfaMkv->eCurPicType = CFA_PTM_RM_INTERPIC; /*INTERPIC*/
		else
			prCfaMkv->eCurPicType = CFA_PTM_RM_TRUEBPIC;  /*TRUEBPIC*/

		break;

	default:
		break;
	}

	return prCfaMkv->eCurPicType;
}

static void CfaMkvTxRvLastParcialFrame(void *pvSptHdl, CfaMkvInst *prCfaMkv)
{
	CFA_VIDEO_INFO_T rVidInf = {0};
	MRESULT mrResult = RET_DMX_OK;
	u64 u8SliceDataSize = prCfaMkv->rCurBlock.u8DataSize - prCfaMkv->u8SliceOffset;

	prCfaMkv->eCurState = CFA_MKV_ST_TX_BLOCK;
	/*transfer data*/
	rVidInf.u8FileOfst = 0;
	rVidInf.eTxMode = CfaMkvRvSetPicTxMode(prCfaMkv);
	rVidInf.eVidType = prCfaMkv->rVidStmInfo.eVidCodec;
	rVidInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;
	rVidInf.u8Len = u8SliceDataSize;
	rVidInf.u8TotalAULen = prCfaMkv->rCurBlock.u8DataSize - prCfaMkv->u8SliceOffset;

	if (prCfaMkv->u4SliceNum >= RM_VID_SLICE_MAX_NUM) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] u4SliceNum is %d, Send EOS\r\n"),
			prCfaMkv->u4SliceNum);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
		return;
	}

	prCfaMkv->rSliceInf.rSliceInf[prCfaMkv->u4SliceNum].u1SliceElemNum = (u8)prCfaMkv->u4SliceNum + (u8)1;
	prCfaMkv->rSliceInf.rSliceInf[prCfaMkv->u4SliceNum].u2SliceElemSize = (u16)u8SliceDataSize;
	if (0 == prCfaMkv->u4SliceNum)
		prCfaMkv->rSliceInf.u1TotalSliceNum = (u8)(prCfaMkv->u4SliceTotalNum + 1);

	CfaMkvAdjustSliceInfo(prCfaMkv);

	rVidInf.fgUnitStart = FALSE;
	rVidInf.u8TotalAULen = 0;
	rVidInf.u8FileOfst = prCfaMkv->rCurBlock.u8DataOfst + prCfaMkv->u8SliceOffset;

	rVidInf.u2RmCurAuSliceNum = prCfaMkv->rSliceInf.u1TotalSliceNum;

	mrResult = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rVidInf);
	if (RET_DMX_OK != mrResult) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
	}
	return;

}

static void CfaMkvTxRvParcialFrame(void *pvSptHdl, CfaMkvInst *prCfaMkv)
{
	u64 u8SliceDataOffset = 0;
	CFA_VIDEO_INFO_T rVidInf = {0};
	MRESULT mrResult = RET_DMX_OK;

	prCfaMkv->eCurState = CFA_MKV_ST_TX_RV_PARCIAL;

	if (prCfaMkv->u8HdrLen < RM_SLICE_LEN_BYTES) {
		if ((prCfaMkv->u4SliceHeaderOffst + RM_SLICE_LEN_BYTES) > RM_MAX_SLICE_HEADER_LEN)
			prCfaMkv->u4SliceHeaderOffst -= RM_SLICE_LEN_BYTES;

		dmx_memcpy((void *)(prCfaMkv->aucHdr + prCfaMkv->u8HdrLen),
						   (void *)((u8 *)prCfaMkv->aucSliceHeader +
						   prCfaMkv->u4SliceHeaderOffst),
						   RM_SLICE_LEN_BYTES);
		prCfaMkv->u8HdrLen += RM_SLICE_LEN_BYTES;
		prCfaMkv->u8MemOffset += RM_SLICE_LEN_BYTES;
		prCfaMkv->u4SliceHeaderOffst += RM_SLICE_LEN_BYTES;
	}

	if (prCfaMkv->u4SliceTotalNum == prCfaMkv->u4SliceNum) {
		prCfaMkv->u8MemOffset -= RM_SLICE_LEN_BYTES;
		CfaMkvTxRvLastParcialFrame(pvSptHdl, prCfaMkv);
		return;
	}
	{
		((u8 *)&u8SliceDataOffset)[7] = 0;
		((u8 *)&u8SliceDataOffset)[6] = 0;
		((u8 *)&u8SliceDataOffset)[5] = 0;
		((u8 *)&u8SliceDataOffset)[4] = 0;
		((u8 *)&u8SliceDataOffset)[3] = prCfaMkv->aucHdr[7];
		((u8 *)&u8SliceDataOffset)[2] = prCfaMkv->aucHdr[6];
		((u8 *)&u8SliceDataOffset)[1] = prCfaMkv->aucHdr[5];
		((u8 *)&u8SliceDataOffset)[0] = prCfaMkv->aucHdr[4];
		MkvMoveHdr(prCfaMkv, (u32)RM_SLICE_LEN_BYTES);

	}

	rVidInf.eTxMode = CfaMkvRvSetPicTxMode(prCfaMkv);
	rVidInf.u8FileOfst = prCfaMkv->rCurBlock.u8DataOfst + prCfaMkv->u8SliceOffset;
	rVidInf.eVidType = prCfaMkv->rVidStmInfo.eVidCodec;
	rVidInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;
	rVidInf.u8Len = u8SliceDataOffset - prCfaMkv->u8LastSliceDataOffset;
	prCfaMkv->u8LastSliceDataOffset = u8SliceDataOffset;

	if (0 == prCfaMkv->u4SliceNum) {
		rVidInf.fgUnitStart = TRUE;
		rVidInf.u8TotalAULen = prCfaMkv->rCurBlock.u8DataSize - prCfaMkv->u8SliceOffset;
		CfaMkvRvGetPicPts(prCfaMkv);
		prCfaMkv->rSliceInf.u1TotalSliceNum = (u8)(prCfaMkv->u4SliceTotalNum + 1);
	} else {
		rVidInf.fgUnitStart = FALSE;
		rVidInf.u8TotalAULen = 0;
	}
	prCfaMkv->u8SliceOffset += rVidInf.u8Len;

	if (prCfaMkv->u4SliceNum >= RM_VID_SLICE_MAX_NUM) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] u4SliceNum is %d, Send EOS\r\n"),
			prCfaMkv->u4SliceNum);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
		return;
	}

	prCfaMkv->rSliceInf.rSliceInf[prCfaMkv->u4SliceNum].u1SliceElemNum = (u8)prCfaMkv->u4SliceNum + (u8)1;
	prCfaMkv->rSliceInf.rSliceInf[prCfaMkv->u4SliceNum].u2SliceElemSize = (u16)rVidInf.u8Len;

	CfaMkvAdjustSliceInfo(prCfaMkv);

	rVidInf.u2RmCurAuSliceNum = 0;
	if (prCfaMkv->u4SliceNum == prCfaMkv->u4SliceTotalNum)
		rVidInf.u2RmCurAuSliceNum = prCfaMkv->rSliceInf.u1TotalSliceNum + 1;

	if ((0 == (rVidInf.u8Len)) ||
		((rVidInf.fgUnitStart) && ((rVidInf.u8Len) > (rVidInf.u8TotalAULen)))) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] u8TotalAULen(%lld) and u8Len(%lld),")
			TEXT("u8Ca is %lld, find invalid data and Seek\r\n"),
			rVidInf.u8TotalAULen, rVidInf.u8Len, prCfaMkv->u8Ca);
		if (DMX_IS_RW_PLAY(pvSptHdl)) {
			Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, FALSE, GAU_E_ERRCHUNK);
		} else {
			Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, TRUE, GAU_E_ERRCHUNK);
		}
		return;
	}

	mrResult = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rVidInf);
	if (RET_DMX_OK != mrResult) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
		return;
	}
	prCfaMkv->u4SliceNum += 1;

	if (prCfaMkv->u4SliceNum >= RM_VID_SLICE_MAX_NUM)
		prCfaMkv->u4SliceNum = RM_VID_SLICE_MAX_NUM - 1;

}

static void CfaMkvTxRvBlockDataEx(void *pvSptHdl, CfaMkvInst *prCfaMkv,  u64 u8TxLen)
{
	CFA_VIDEO_INFO_T rVidInf = {0};
	MRESULT mrResult = RET_DMX_OK;

	prCfaMkv->eCurPrsStrm = prCfaMkv->rCurBlock.eBlockType;

	DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
		TEXT("[CFA_MKV_ST_CTRL] Entry CfaMkvTxRvBlockDataEx!\r\n"));

	prCfaMkv->eCurState = CFA_MKV_ST_TX_BLOCK;
	prCfaMkv->fgSecondFillAU = FALSE;

	if ((prCfaMkv->rCurBlock.u8DataOfst +	prCfaMkv->rCurBlock.u8DataSize) >=
		prCfaMkv->rRange.u8VidEndOfst) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
		return;
	}

	if (((u64)prCfaMkv->u4SliceHeaderDataPartLen + u8TxLen) > (u64)RM_MAX_SLICE_HEADER_LEN) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] u4SliceHeaderDataPartLe is %lld,u8TxLen is %lld, line %d Send EOS\r\n"),
		   prCfaMkv->u4SliceHeaderDataPartLen, u8TxLen, DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl,prCfaMkv);
		return;
	}

	dmx_memcpy((void *)(prCfaMkv->aucSliceHeader + prCfaMkv->u4SliceHeaderDataPartLen),
		(void *)((u8 *)prCfaMkv->ptrMemAddr), (u32)u8TxLen);

	prCfaMkv->u4SliceHeaderDataPartLen = prCfaMkv->u4SliceHeaderDataPartLen + (u32)u8TxLen;
	prCfaMkv->u8MemOffset = u8TxLen;

	if (prCfaMkv->u4SliceHeaderDataPartLen !=
		((prCfaMkv->u4SliceTotalNum + (u32)1) * RM_SLICE_LEN_BYTES + RM_SLICE_NUM_BYTE)) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
		return;
	}

	MkvMoveHdr(prCfaMkv, RM_SLICE_LEN_BYTES + RM_SLICE_NUM_BYTE);
	prCfaMkv->u8SliceOffset = (u64)((u64)(prCfaMkv->u4SliceTotalNum + (u32)1) * RM_SLICE_LEN_BYTES + RM_SLICE_NUM_BYTE);

	LOADB_DWRD(((u8 *)prCfaMkv->ptrMemAddr + prCfaMkv->u8MemOffset), prCfaMkv->u4First4BytesPay);
	dmx_memcpy((void *)(&prCfaMkv->u1FirstBytePay),
		(void *)((u8 *)prCfaMkv->ptrMemAddr + prCfaMkv->u8MemOffset), 1);

	if (prCfaMkv->u4SliceTotalNum > 0) {  /*partial frame */
		prCfaMkv->u4SliceHeaderOffst = RM_SLICE_LEN_BYTES + RM_SLICE_NUM_BYTE;
		prCfaMkv->u4SliceNum = 0;
		prCfaMkv->u8LastSliceDataOffset = 0;
		CfaMkvTxRvParcialFrame(pvSptHdl, prCfaMkv);
	} else { /*whole frame */
		rVidInf.eTxMode = CfaMkvRvSetPicTxMode(prCfaMkv);

		rVidInf.u8FileOfst = prCfaMkv->rCurBlock.u8DataOfst + prCfaMkv->u8SliceOffset;
		rVidInf.eVidType = prCfaMkv->rVidStmInfo.eVidCodec;
		rVidInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;
		rVidInf.u8Len = prCfaMkv->rCurBlock.u8DataSize - prCfaMkv->u8SliceOffset;
		rVidInf.u8TotalAULen = prCfaMkv->rCurBlock.u8DataSize - prCfaMkv->u8SliceOffset;
		rVidInf.fgUnitStart = TRUE;

		prCfaMkv->rSliceInf.rSliceInf[0].u1SliceElemNum = 1;
		prCfaMkv->rSliceInf.rSliceInf[0].u2SliceElemSize =
			(u16)(prCfaMkv->rCurBlock.u8DataSize - prCfaMkv->u8SliceOffset);
		prCfaMkv->rSliceInf.u1TotalSliceNum = 1;

		rVidInf.u2RmCurAuSliceNum = 1;

		CfaMkvRvGetPicPts(prCfaMkv);

		CfaMkvAdjustSliceInfo(prCfaMkv);

		mrResult = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rVidInf);
		if (RET_DMX_OK != mrResult) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
			MkvFinishPrs(pvSptHdl, prCfaMkv);
		}
	}
}


/*added by Mingxu Wang, to proc real video data*/
static void CfaMkvTxRvBlockData(void *pvSptHdl, CfaMkvInst *prCfaMkv, u64 u8TxLen)
{
	CFA_VIDEO_INFO_T rVidInf = {0};
	MRESULT mrResult = RET_DMX_OK;
	u32 u4SliceHeaderLen = 0;

	if (0 == prCfaMkv->u4AvailSize) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d, maybe read header data in two slots!\r\n"), DMX_LINE_NO);
		ToNextState(pvSptHdl, prCfaMkv, u8TxLen, CFA_MKV_ST_TX_RV_BLOCKDATA);
		prCfaMkv->fgCrossSlot = TRUE;
		return;
	}

	MkvGetPts(pvSptHdl, prCfaMkv);
	prCfaMkv->eCurPrsStrm = prCfaMkv->rCurBlock.eBlockType;

	prCfaMkv->eCurState = CFA_MKV_ST_TX_BLOCK;
	prCfaMkv->fgSecondFillAU = FALSE;
	if (prCfaMkv->fgCrossSlot == TRUE)
	{
		prCfaMkv->fgCrossSlot = FALSE;
		prCfaMkv->u8MemOffset = 0;
	}
	else
	{
		prCfaMkv->u8MemOffset = u8TxLen;
	}
	if ((prCfaMkv->rCurBlock.u8DataOfst +	prCfaMkv->rCurBlock.u8DataSize) >=
		prCfaMkv->rRange.u8VidEndOfst) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
		return;
	}

	/*(8 + 1), vint is 8 bytes, one byte for slice num*/
	if (prCfaMkv->u8HdrLen < (RM_SLICE_LEN_BYTES + RM_SLICE_NUM_BYTE)) {
		dmx_memcpy((void *)(prCfaMkv->aucHdr + prCfaMkv->u8HdrLen),
			(void *)((u8 *)prCfaMkv->ptrMemAddr + prCfaMkv->u8MemOffset),
			((RM_SLICE_LEN_BYTES + RM_SLICE_NUM_BYTE) - (u32)prCfaMkv->u8HdrLen));
		prCfaMkv->u8MemOffset += ((RM_SLICE_LEN_BYTES + RM_SLICE_NUM_BYTE) - prCfaMkv->u8HdrLen);
		prCfaMkv->u8HdrLen = (RM_SLICE_LEN_BYTES + RM_SLICE_NUM_BYTE);
	}

	prCfaMkv->u4SliceTotalNum = (u32)(prCfaMkv->aucHdr[0]);
	/*include slice number (1 byte) and the first slice offset(8 bytes)*/
	u4SliceHeaderLen = (prCfaMkv->u4SliceTotalNum + 1) * RM_SLICE_LEN_BYTES + RM_SLICE_NUM_BYTE;
	/* the data length that will be read is larger than available size in current slot*/
	if (u4SliceHeaderLen >
		 (prCfaMkv->u4AvailSize - prCfaMkv->u8MemOffset + RM_SLICE_LEN_BYTES + RM_SLICE_NUM_BYTE)) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] u4AvailSize is %d, u8MemOffset is %lld, u4SliceHeaderLen is %d\r\n"),
			prCfaMkv->u4AvailSize, prCfaMkv->u8MemOffset, u4SliceHeaderLen);
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d, maybe read header data in two slots!\r\n"), DMX_LINE_NO);
		prCfaMkv->u4SliceHeaderDataPartLen =
								prCfaMkv->u4AvailSize - (u32)prCfaMkv->u8MemOffset +
								RM_SLICE_LEN_BYTES + RM_SLICE_NUM_BYTE;
		dmx_memcpy((void *)(prCfaMkv->aucSliceHeader),
			(void *)((u8 *)prCfaMkv->ptrMemAddr + prCfaMkv->u8MemOffset -
			(RM_SLICE_LEN_BYTES + RM_SLICE_NUM_BYTE)),
			prCfaMkv->u4SliceHeaderDataPartLen);

		prCfaMkv->u8Ca = prCfaMkv->u8Ca + prCfaMkv->u4AvailSize - u8TxLen;
		ToNextState(pvSptHdl, prCfaMkv,
			(u4SliceHeaderLen - prCfaMkv->u4SliceHeaderDataPartLen), CFA_MKV_ST_TX_RV_BLOCK);
		return;
	}

	MkvMoveHdr(prCfaMkv, RM_SLICE_LEN_BYTES + RM_SLICE_NUM_BYTE);
	prCfaMkv->u8SliceOffset = (prCfaMkv->u4SliceTotalNum + 1) * RM_SLICE_LEN_BYTES + RM_SLICE_NUM_BYTE;

	LOADB_DWRD(((u8 *)prCfaMkv->ptrMemAddr + prCfaMkv->u8MemOffset +
		prCfaMkv->u8SliceOffset - (RM_SLICE_LEN_BYTES + RM_SLICE_NUM_BYTE)),
		prCfaMkv->u4First4BytesPay);
	dmx_memcpy((void *)(&prCfaMkv->u1FirstBytePay),
		(void *)((u8 *)prCfaMkv->ptrMemAddr + prCfaMkv->u8MemOffset +
		prCfaMkv->u8SliceOffset - (RM_SLICE_LEN_BYTES + RM_SLICE_NUM_BYTE)), 1);

	if (prCfaMkv->u4SliceTotalNum > 0) {
		/*partial frame */
		prCfaMkv->u4SliceNum = 0;
		prCfaMkv->u8LastSliceDataOffset = 0;

		if (u4SliceHeaderLen > RM_MAX_SLICE_HEADER_LEN)
			u4SliceHeaderLen = RM_MAX_SLICE_HEADER_LEN;
		/*copy data skip slice number(1 byte) and the first slice offset(8 bytes)*/
		dmx_memcpy((void *)(prCfaMkv->aucSliceHeader),
					(void *)((u8 *)prCfaMkv->ptrMemAddr + prCfaMkv->u8MemOffset),
					u4SliceHeaderLen - RM_SLICE_LEN_BYTES - RM_SLICE_NUM_BYTE);
		prCfaMkv->u4SliceHeaderOffst = 0;
		CfaMkvTxRvParcialFrame(pvSptHdl, prCfaMkv);
	} else {/*whole frame */
		rVidInf.eTxMode = CfaMkvRvSetPicTxMode(prCfaMkv);

		rVidInf.u8FileOfst = prCfaMkv->rCurBlock.u8DataOfst + prCfaMkv->u8SliceOffset;
		rVidInf.eVidType = prCfaMkv->rVidStmInfo.eVidCodec;
		rVidInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;
		rVidInf.u8Len = prCfaMkv->rCurBlock.u8DataSize - prCfaMkv->u8SliceOffset;
		rVidInf.u8TotalAULen = prCfaMkv->rCurBlock.u8DataSize - prCfaMkv->u8SliceOffset;
		rVidInf.fgUnitStart = TRUE;

		prCfaMkv->rSliceInf.rSliceInf[0].u1SliceElemNum = 1;
		prCfaMkv->rSliceInf.rSliceInf[0].u2SliceElemSize =
			(u16)(prCfaMkv->rCurBlock.u8DataSize - prCfaMkv->u8SliceOffset);
		prCfaMkv->rSliceInf.u1TotalSliceNum = 1;

		rVidInf.u2RmCurAuSliceNum = 1;

		CfaMkvRvGetPicPts(prCfaMkv);

		CfaMkvAdjustSliceInfo(prCfaMkv);

		mrResult = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rVidInf);
		if (RET_DMX_OK != mrResult) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
			MkvFinishPrs(pvSptHdl, prCfaMkv);
		}
	}
}

static void CfaMkvNoLacing(void *pvSptHdl, CfaMkvInst *prCfaMkv, u32 u4CfaMkvOffset, u64 u8TxLen)
{
	switch (prCfaMkv->rVidStmInfo.eVidCodec) {
	case CFA_VID_DIVX3:
		prCfaMkv->eCfaMkvTxMode = CFA_PTM_ONE_PIC_DX3_I;

	#if 0
		if (TRUE == prCfaMkv->fgFirstTxVid) {
			prCfaMkv->fgFirstTxVid = FALSE;
			/* Set DIVX311 first frame as I frame anyway. For CQ BDP122706*/
		} else
	#endif
		{
			if ((prCfaMkv->aucHdr[1]) & (u8)CFA_MKV_DIVX3_P_FRM)
				prCfaMkv->eCfaMkvTxMode = CFA_PTM_ONE_PIC_DX3_P;
			else
				prCfaMkv->eCfaMkvTxMode = CFA_PTM_ONE_PIC_DX3_I;

		}
		break;

	case CFA_VID_WMV7:
		if (CFA_MKV_WMV_PVOP == ((prCfaMkv->aucHdr[1]) >> 6))
			prCfaMkv->eCfaMkvTxMode =  CFA_PTM_WMV_P;
		else
			prCfaMkv->eCfaMkvTxMode = CFA_PTM_WMV_I;

		break;

	case CFA_VID_WMV8:
		if (CFA_MKV_WMV_PVOP == ((prCfaMkv->aucHdr[1]) >> 7))
			prCfaMkv->eCfaMkvTxMode =  CFA_PTM_WMV_P;
		else
			prCfaMkv->eCfaMkvTxMode =  CFA_PTM_WMV_I;

		break;

	case CFA_VID_WMV9:
		if (prCfaMkv->fgPrsInterp)
			u4CfaMkvOffset--;

		u4CfaMkvOffset -= (u32)2;
		if (prCfaMkv->fgPrsPre)
			u4CfaMkvOffset--;

		--u4CfaMkvOffset;
		if ((prCfaMkv->aucHdr[1]) & ((u8)((u32)1 << (u4CfaMkvOffset))))
			prCfaMkv->eCfaMkvTxMode =  CFA_PTM_WMV_P;
		else {
			if (prCfaMkv->u4PrsBFrameNs == 0)

				prCfaMkv->eCfaMkvTxMode =  CFA_PTM_WMV_I;
			else {
				--u4CfaMkvOffset;
				if ((prCfaMkv->aucHdr[1]) & ((u8)((u32)1 << (u4CfaMkvOffset))))
					prCfaMkv->eCfaMkvTxMode =  CFA_PTM_WMV_I;
				else
					prCfaMkv->eCfaMkvTxMode =  CFA_PTM_WMV_B;

			}
		}
		break;

	case CFA_VID_VC1:
		/*TODO: add sth here later*/
		prCfaMkv->eCfaMkvTxMode = CFA_PTM_EXACT_POS;
		break;

	case CFA_VID_H263_SORENSON:
	{
		u8 u1Temp1 = 0;
		u8 u1Temp2 = 0;
		/*skip frm type,codecid,start code,version,temporalreferance*/
		u32 u4PictSizeOft = (u32)4;

		if (prCfaMkv->u8HdrLen < (u64)6) {
			/*be sure the fifth byte of block date in the buffer.*/
			dmx_memcpy((void *)(prCfaMkv->aucHdr + prCfaMkv->u8HdrLen),
				(void *)((u8 *)prCfaMkv->ptrMemAddr + (u32)u8TxLen), 10);
		}

		u1Temp1 = prCfaMkv->aucHdr[4];
		u1Temp1 &= (u8)0x03;
		u1Temp1 = u1Temp1 << 1;

		u1Temp2 = prCfaMkv->aucHdr[5];
		u1Temp2 &= (u8)0x80;
		u1Temp2 = u1Temp2 >> 7;

		u1Temp1 = u1Temp1 + u1Temp2;

		if (u1Temp1 == 0)
			u4PictSizeOft += (u32)3;
		else if (u1Temp1 == 1)
			u4PictSizeOft += (u32)5;
		else
			u4PictSizeOft += (u32)1;

		if ((prCfaMkv->aucHdr[u4PictSizeOft] & (u8)0x60) == 0)
			prCfaMkv->eCfaMkvTxMode = CFA_PTM_H263_SORENSON_I;
		else
			prCfaMkv->eCfaMkvTxMode = CFA_PTM_H263_SORENSON_P;

		break;
	}

	case CFA_VID_VP8:
	{
		/*DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
		TEXT("[CFA_MKV_ST_CTRL][VP8] u8Ca is %lld[0x%llx]\r\n"),
		prCfaMkv->u8Ca,prCfaMkv->u8Ca);*/
		if ((prCfaMkv->aucHdr[0] & (u8)0x80) == 0x80) {
			/*DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL][VP8] I-Frame\r\n"));*/
			prCfaMkv->eCfaMkvTxMode = CFA_PTM_ONE_PIC_VP8_I;
		} else {
			/*DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL][VP8] P-Frame\r\n"));*/
			prCfaMkv->eCfaMkvTxMode = CFA_PTM_ONE_PIC_VP8_P;
		}
		break;
	}

	case CFA_VID_VP6:
	{
		/*DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
		TEXT("[CFA_MKV_ST_CTRL][VP6] u8Ca is %lld[0x%llx]\r\n"),
		prCfaMkv->u8Ca,prCfaMkv->u8Ca);*/
		if ((prCfaMkv->aucHdr[0] & (u8)0x80) == 0x80) {
			/*DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL][VP6] I-Frame\r\n"));*/
			prCfaMkv->eCfaMkvTxMode = CFA_PTM_ONE_PIC_VP6_I;
		} else {
			/*DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL][VP6] P-Frame\r\n"));*/
			prCfaMkv->eCfaMkvTxMode = CFA_PTM_ONE_PIC_VP6_P;
		}
		break;
	}

	default:
		break;
	}
	/*1byte frame_count*/
	prCfaMkv->rCurBlock.u8DataOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen + 1;
	prCfaMkv->rCurBlock.u8DataSize =
		(prCfaMkv->rCurBlock.u8BlockOfst + prCfaMkv->rCurBlock.u8BlockSize) -
		prCfaMkv->rCurBlock.u8DataOfst;

	prCfaMkv->rCurBlock.fgReady = TRUE;
	prCfaMkv->rCurBlock.eLacingType = CFA_MKV_NO_LACING;

	MkvMoveHdr(prCfaMkv, (u32)1);
	if ((CFA_VID_RV30 == prCfaMkv->rVidStmInfo.eVidCodec) ||
	   (CFA_VID_RV40 == prCfaMkv->rVidStmInfo.eVidCodec))
		CfaMkvTxRvBlockData(pvSptHdl, prCfaMkv, u8TxLen);
	else
		TxBlock2Fifo(pvSptHdl, prCfaMkv);

}
/*-----------------------------------------------------------------------------
* Name: vCfaMkvProcBlock
*
* Description:
*	   Main process of a block
*
*
* Inputs:
*
* Outputs:
*
* Returns: void
*
*-----------------------------------------------------------------------------*/
static void CfaMkvProcBlock(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	u32 u4TotalFrameSize = 0;
	s32 i4Loop = 0;
	u32 u4CfaMkvOffset = (u32)8;
	CfaPrsBitStrmType eCfaMkvPrsStrmType = CFA_PRS_BIT_STRM_TYPE_NONE;
	u8 idx = prCfaMkv->rCurBlock.ucStrmIdx;

	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("CfaMkvProcBlock::u8TxLen is %lld, u8HdrLen is %lld\r\n"),
		u8TxLen, prCfaMkv->u8HdrLen);

	/*DMX_ASSERT((prCfaMkv->u8HdrLen + u8TxLen) <= CFA_MKV_HDR_BUF);*/
	if ((prCfaMkv->u8HdrLen + u8TxLen) > CFA_MKV_HDR_BUF) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] u8HdrLen is %lld, u8TxLen is %lld,line %d Send EOS\r\n"),
			prCfaMkv->u8HdrLen, u8TxLen, DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl,prCfaMkv);
		return;
	}

	dmx_memcpy((void *)(prCfaMkv->aucHdr + prCfaMkv->u8HdrLen),
				(void *)prCfaMkv->ptrMemAddr, (u32)u8TxLen);
	prCfaMkv->u8HdrLen += u8TxLen;
	prCfaMkv->rCurBlock.i4FrameNum = 0;
	prCfaMkv->rCurBlock.i4CurFrameNum = 0;
	dmx_memset(prCfaMkv->rCurBlock.au4FrameSize, 0, sizeof(prCfaMkv->rCurBlock.au4FrameSize));
	prCfaMkv->rCurBlock.i4BFramCount = 1;
	prCfaMkv->rCurBlock.u4BlockNo++;
	MkvGetSize(prCfaMkv->aucHdr, &(prCfaMkv->rCurBlock.u8TrackNum), prCfaMkv);

	prCfaMkv->rCurBlock.u8TimeCode = (u64)((s64)(prCfaMkv->rCurCluster.u8TimeCode) +
							(s16)MkvComputeEndian(prCfaMkv->aucHdr, (u8)2));

	MkvMoveHdr(prCfaMkv, (u32)2);/*remove the 2 bytes TimeCode*/ /* Spec Page39*/
	eCfaMkvPrsStrmType = GetTrackType(prCfaMkv, prCfaMkv->rCurBlock.u8TrackNum);
#if CONFIG_CFA_MKV_SUPPORT_DRM
	if (CFA_PRS_BIT_STRM_TYPE_SP == eCfaMkvPrsStrmType)
		CfaMkvTurnOffDivXDrm(pvSptHdl, prCfaMkv);

#endif

	if (CFA_PRS_BIT_STRM_TYPE_A == eCfaMkvPrsStrmType) {
		prCfaMkv->rCurBlock.u8VorbisPageNum = 0;
		prCfaMkv->rCurBlock.u4VorbisPageIndex = 0;
		prCfaMkv->rCurBlock.u8VorbisCurPageDataSize = 0;
		prCfaMkv->rCurBlock.u8VorbisCurPageTotalAULen = 0;
	}

	switch (eCfaMkvPrsStrmType) {
	case CFA_PRS_BIT_STRM_TYPE_V:
		prCfaMkv->rCurBlock.eBlockType = CFA_PRS_BIT_STRM_TYPE_V;

		if ((prCfaMkv->u4PrsFlg  & CFA_PRS_BIT_STRM_TYPE_V) == 0) {
			MkvSkipBlock(pvSptHdl, prCfaMkv);
			return;
		}
		if (prCfaMkv->rCurCluster.fgPrsVid) {
			if (prCfaMkv->rCurCluster.fgFirstVid) {
				if (prCfaMkv->rRange.u4VidBlockNo != MKV_INVALID_BLOCK_NO) {
					if (prCfaMkv->rCurBlock.u4BlockNo < prCfaMkv->rRange.u4VidBlockNo) {
						MkvSkipBlock(pvSptHdl, prCfaMkv);
						return;
					}
				}
			}

			prCfaMkv->rCurBlock.fgDiscardable = FALSE;

			if ((prCfaMkv->aucHdr[0] & (u8)0x01) == (u8)0x01) {
				DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
					TEXT("prCfaMkv->aucHdr[0] is %d, timeStamp is %lld, u8Ca is %lld\r\n"),
					prCfaMkv->aucHdr[0], prCfaMkv->rCurBlock.u8TimeCode, prCfaMkv->u8Ca);
				prCfaMkv->rCurBlock.fgDiscardable = TRUE;
				/* MkvSkipBlock(pvSptHdl,prCfaMkv);*/
				/* return;*/
			}

			if ((prCfaMkv->aucHdr[0] & (u8)0x06) == 0)/*no lacing*/ {
				CfaMkvNoLacing(pvSptHdl, prCfaMkv, u4CfaMkvOffset, u8TxLen);
				return;
			}
			if ((prCfaMkv->aucHdr[0] & (u8)0x06) == (u8)0x02)/*Xiph lacing*/ {
				prCfaMkv->rCurBlock.ucFrameCount = prCfaMkv->aucHdr[1];/*frame_count - 1*/
				prCfaMkv->rCurBlock.eLacingType = CFA_MKV_XIPH_LACING;
				MkvMoveHdr(prCfaMkv, (u32)2);/*remove the 1 bytes flag and 1 byte frame count*/
				if (prCfaMkv->rCurBlock.ucFrameCount == 0) {
					prCfaMkv->rCurBlock.u8DataOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen + 1;
					prCfaMkv->rCurBlock.u8DataSize = (prCfaMkv->rCurBlock.u8BlockOfst
									+ prCfaMkv->rCurBlock.u8BlockSize)
									- prCfaMkv->rCurBlock.u8DataOfst;
					prCfaMkv->rCurBlock.fgReady = TRUE;
					TxBlock2Fifo(pvSptHdl, prCfaMkv);
					return;
				}
				if (prCfaMkv->u8HdrLen >= prCfaMkv->rCurBlock.ucFrameCount) {
					prCfaMkv->rCurBlock.ucFrameCount = prCfaMkv->rCurBlock.ucFrameCount -
											MkvCountLacing(pvSptHdl, prCfaMkv);
				}
				if (prCfaMkv->rCurBlock.ucFrameCount == 0) {
					prCfaMkv->rCurBlock.u8DataOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen;
					prCfaMkv->rCurBlock.u8DataSize =
						(prCfaMkv->rCurBlock.u8BlockOfst + prCfaMkv->rCurBlock.u8BlockSize) -
						prCfaMkv->rCurBlock.u8DataOfst;
					prCfaMkv->rCurBlock.fgReady = TRUE;

					TxBlock2Fifo(pvSptHdl, prCfaMkv);
					return;
				}
				{
					ToNextState(pvSptHdl, prCfaMkv,
						(u64)(prCfaMkv->rCurBlock.ucFrameCount) - (prCfaMkv->u8HdrLen),
						CFA_MKV_ST_LACING_ANA);
				}

			} else if ((prCfaMkv->aucHdr[0] & (u8)0x06) == (u8)0x06)/*EBML lacing*/ {
				prCfaMkv->rCurBlock.ucFrameCount = prCfaMkv->aucHdr[1];/*frame_count - 1*/
				prCfaMkv->rCurBlock.eLacingType = CFA_MKV_EBML_LACING;
				MkvMoveHdr(prCfaMkv, (u32)2);/*remove the 1 bytes flag and 1 byte frame count*/

				if (GetVaraibleSize(prCfaMkv->aucHdr) > ((u8)prCfaMkv->u8HdrLen)) {
					ToNextState(pvSptHdl, prCfaMkv,
						(u64)(GetVaraibleSize(prCfaMkv->aucHdr) -
						((u8)prCfaMkv->u8HdrLen)),
						CFA_MKV_ST_LACING_ANA);
					return;
				}

				if (prCfaMkv->u8HdrLen >= prCfaMkv->rCurBlock.ucFrameCount) {
					prCfaMkv->rCurBlock.ucFrameCount = prCfaMkv->rCurBlock.ucFrameCount -
											MkvCountLacing(pvSptHdl, prCfaMkv);
				}
				if (prCfaMkv->rCurBlock.ucFrameCount == 0) {
					prCfaMkv->rCurBlock.u8DataOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen;
					prCfaMkv->rCurBlock.u8DataSize =
						(prCfaMkv->rCurBlock.u8BlockOfst + prCfaMkv->rCurBlock.u8BlockSize) -
						prCfaMkv->rCurBlock.u8DataOfst;
					prCfaMkv->rCurBlock.fgReady = TRUE;
					TxBlock2Fifo(pvSptHdl, prCfaMkv);
					return;
				}
				{
					ToNextState(pvSptHdl, prCfaMkv,
						(u64)(prCfaMkv->rCurBlock.ucFrameCount) - (prCfaMkv->u8HdrLen),
						CFA_MKV_ST_LACING_ANA);
					return;
				}
			} else/*fix lacing*/ {
				/*1 byte flag,1byte frame_count*/
				prCfaMkv->rCurBlock.u8DataOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen + 2;
				prCfaMkv->rCurBlock.u8DataSize =
					(prCfaMkv->rCurBlock.u8BlockOfst + prCfaMkv->rCurBlock.u8BlockSize) -
					prCfaMkv->rCurBlock.u8DataOfst;
				prCfaMkv->rCurBlock.fgReady = TRUE;
				prCfaMkv->rCurBlock.eLacingType = CFA_MKV_FIX_LACING;
				TxBlock2Fifo(pvSptHdl, prCfaMkv);
				return;
			}
		} else {
			MkvSkipBlock(pvSptHdl, prCfaMkv);
			return;
		}
		break;
	case CFA_PRS_BIT_STRM_TYPE_A:
		prCfaMkv->rCurBlock.eBlockType = CFA_PRS_BIT_STRM_TYPE_A;

		if ((prCfaMkv->u4PrsFlg  & CFA_PRS_BIT_STRM_TYPE_A) == 0) {
			MkvSkipBlock(pvSptHdl, prCfaMkv);
			return;
		}

		if (((prCfaMkv->rCurBlock.u8TrackNum) != (prCfaMkv->u8CurAId)) && (!prCfaMkv->fgHasVideo)) {
			DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
				TEXT("[CFA_MKV_ST_CTRL] line %d, the tx track num is %I64d,")
				TEXT("current track num is %I64d\r\n"),
				DMX_LINE_NO, prCfaMkv->u8CurAId, prCfaMkv->rCurBlock.u8TrackNum);
			MkvSkipBlock(pvSptHdl, prCfaMkv);
			return;
		}

		if (prCfaMkv->rCurCluster.fgPrsAud == TRUE) {
			if (prCfaMkv->rCurCluster.fgFirstAud == TRUE) {
				if (prCfaMkv->rRange.u4AudBlockNo != MKV_INVALID_BLOCK_NO) {
					if (prCfaMkv->rCurBlock.u4BlockNo < prCfaMkv->rRange.u4AudBlockNo) {
						MkvSkipBlock(pvSptHdl, prCfaMkv);
						return;
					}
				} else {
				#if 1
					if (prCfaMkv->rCurBlock.u8TimeCode < prCfaMkv->rRange.u8TargetTimeCode) {
						DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
							TEXT("[CFA_MKV_ST_CTRL] -3-u8TimeCode is %lld <")
							TEXT("u8TargetTimeCode is %lld\r\n"),
							prCfaMkv->rCurBlock.u8TimeCode,
							prCfaMkv->rRange.u8TargetTimeCode);
						MkvSkipBlock(pvSptHdl, prCfaMkv);
						return;
					}
				#endif
				}
			}

			if (prCfaMkv->arAudStmInfo[idx].rCfaMkvHeader.fgHeaderStriping) {
				if (prCfaMkv->u8HdrLen < 3) { /*be sure the first 2 bytes of block date in the buffer.*/
					dmx_memcpy((void *)(prCfaMkv->aucHdr + prCfaMkv->u8HdrLen),
							(void *)((u8 *)prCfaMkv->ptrMemAddr + (u32)u8TxLen), 2);
				}

				/*if the first 12 bits of the block data is "1111 1111 1111",
				that indicate the start of a adts frame*/
				if ((prCfaMkv->arAudStmInfo[idx].eAudCodec == CFA_AUD_DRV_FMT_AAC) &&
					(((u8)0xFF) == prCfaMkv->aucHdr[1]) && (((u8)0xF0) == (((u8)0xF0) & prCfaMkv->aucHdr[2])))
					prCfaMkv->arAudStmInfo[idx].rCfaMkvHeader.fgHeaderStriping = FALSE;

			}

			if ((prCfaMkv->aucHdr[0] & ((u8)0x06)) == 0)/*no lacing*/ {
				/*1byte frame_count*/
				prCfaMkv->rCurBlock.u8DataOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen + 1;
				prCfaMkv->rCurBlock.u8DataSize =
					(prCfaMkv->rCurBlock.u8BlockOfst + prCfaMkv->rCurBlock.u8BlockSize) -
					prCfaMkv->rCurBlock.u8DataOfst;
				prCfaMkv->rCurBlock.fgReady = TRUE;
				prCfaMkv->rCurBlock.eLacingType = CFA_MKV_NO_LACING;
				prCfaMkv->rCurBlock.au4FrameSize[prCfaMkv->rCurBlock.i4FrameNum] =
										(u32)prCfaMkv->rCurBlock.u8DataSize;
				TxBlock2Fifo(pvSptHdl, prCfaMkv);
				return;
			} else if ((prCfaMkv->aucHdr[0] & ((u8)0x06)) == ((u8)0x02))/*Xiph lacing*/ {
				prCfaMkv->rCurBlock.ucFrameCount = prCfaMkv->aucHdr[1];/*frame_count - 1*/
				prCfaMkv->rCurBlock.eLacingType = CFA_MKV_XIPH_LACING;
				MkvMoveHdr(prCfaMkv, (u32)2);/*remove the 1 bytes flag and 1 byte frame count*/
				/*for the case one block has only 2 Xiph-lacing frames
				and lacing has only one byte,then headerlen should be 1*/
				if ((prCfaMkv->rCurBlock.ucFrameCount == 1) &&
					(prCfaMkv->aucHdr[0] < CFA_MKV_XIPH_LACING_SEGMENT)) {
					prCfaMkv->u8HdrLen -= 1;
					prCfaMkv->u8Ca     -= 1;
				}
				if (prCfaMkv->u8HdrLen >= prCfaMkv->rCurBlock.ucFrameCount) {
					prCfaMkv->rCurBlock.ucFrameCount = prCfaMkv->rCurBlock.ucFrameCount -
											MkvCountLacing(pvSptHdl, prCfaMkv);
				}
				if (prCfaMkv->rCurBlock.ucFrameCount == 0) {
					prCfaMkv->rCurBlock.u8DataOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen;
					prCfaMkv->rCurBlock.u8DataSize =
						(prCfaMkv->rCurBlock.u8BlockOfst + prCfaMkv->rCurBlock.u8BlockSize) -
						prCfaMkv->rCurBlock.u8DataOfst;
					prCfaMkv->rCurBlock.fgReady = TRUE;
					for (i4Loop = 0; i4Loop < prCfaMkv->rCurBlock.i4FrameNum; i4Loop++)
						u4TotalFrameSize += prCfaMkv->rCurBlock.au4FrameSize[i4Loop];

					prCfaMkv->rCurBlock.au4FrameSize[prCfaMkv->rCurBlock.i4FrameNum] =
						(u32)prCfaMkv->rCurBlock.u8DataSize - u4TotalFrameSize;
					TxBlock2Fifo(pvSptHdl, prCfaMkv);
					return;
				}
				{
					ToNextState(pvSptHdl, prCfaMkv,
						(u64)(prCfaMkv->rCurBlock.ucFrameCount) - (prCfaMkv->u8HdrLen),
						CFA_MKV_ST_LACING_ANA);
				}

			} else if ((prCfaMkv->aucHdr[0] & ((u8)0x06)) == ((u8)0x06))/*EBML lac(ing*/ {
				prCfaMkv->rCurBlock.ucFrameCount = prCfaMkv->aucHdr[1];/*frame_count - 1*/
				prCfaMkv->rCurBlock.eLacingType = CFA_MKV_EBML_LACING;
				MkvMoveHdr(prCfaMkv, (u32)2);/*remove the 1 bytes flag and 1 byte frame count*/

				if (GetVaraibleSize(prCfaMkv->aucHdr) > ((u8)prCfaMkv->u8HdrLen)) {
					ToNextState(pvSptHdl, prCfaMkv,
						(u64)(GetVaraibleSize(prCfaMkv->aucHdr) -
						((u8)prCfaMkv->u8HdrLen)),
						CFA_MKV_ST_LACING_ANA);
					return;
				}
				/*for the case one block has only 2 EBML-lacing frames and lacing has
				only one byte,then headerlen should be 1*/
				if ((prCfaMkv->rCurBlock.ucFrameCount == 1) &&
					(GetVaraibleSize(prCfaMkv->aucHdr) == 1))
					prCfaMkv->u8HdrLen -= 1;

				if (prCfaMkv->u8HdrLen >= prCfaMkv->rCurBlock.ucFrameCount) {
					prCfaMkv->rCurBlock.ucFrameCount = prCfaMkv->rCurBlock.ucFrameCount -
											MkvCountLacing(pvSptHdl, prCfaMkv);
				}
				if (prCfaMkv->rCurBlock.ucFrameCount == 0) {
					prCfaMkv->rCurBlock.u8DataOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen;
					prCfaMkv->rCurBlock.u8DataSize =
						(prCfaMkv->rCurBlock.u8BlockOfst + prCfaMkv->rCurBlock.u8BlockSize) -
						prCfaMkv->rCurBlock.u8DataOfst;
					prCfaMkv->rCurBlock.fgReady = TRUE;
					for (i4Loop = 0; i4Loop < prCfaMkv->rCurBlock.i4FrameNum; i4Loop++)
						u4TotalFrameSize += prCfaMkv->rCurBlock.au4FrameSize[i4Loop];

					prCfaMkv->rCurBlock.au4FrameSize[prCfaMkv->rCurBlock.i4FrameNum] =
						(u32)prCfaMkv->rCurBlock.u8DataSize - u4TotalFrameSize;
					TxBlock2Fifo(pvSptHdl, prCfaMkv);
					return;
				}
				{
					ToNextState(pvSptHdl, prCfaMkv,
						(u64)(prCfaMkv->rCurBlock.ucFrameCount) - (prCfaMkv->u8HdrLen),
						CFA_MKV_ST_LACING_ANA);
					return;
				}
			} else/*fix lacing*/ {
				/*1 byte flag,1byte frame_count*/
				prCfaMkv->rCurBlock.u8DataOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen + 2;
				prCfaMkv->rCurBlock.u8DataSize =
					(prCfaMkv->rCurBlock.u8BlockOfst + prCfaMkv->rCurBlock.u8BlockSize) -
					prCfaMkv->rCurBlock.u8DataOfst;

				prCfaMkv->rCurBlock.fgReady = TRUE;
				prCfaMkv->rCurBlock.eLacingType = CFA_MKV_FIX_LACING;
				prCfaMkv->rCurBlock.ucFrameCount = prCfaMkv->aucHdr[1];/*frame_count - 1*/
				for (i4Loop = 0; i4Loop <= prCfaMkv->rCurBlock.ucFrameCount; i4Loop++) {
					prCfaMkv->rCurBlock.au4FrameSize[i4Loop] =
									(u32)prCfaMkv->rCurBlock.u8DataSize /
									(prCfaMkv->rCurBlock.ucFrameCount + 1);
				}
				prCfaMkv->rCurBlock.i4FrameNum = prCfaMkv->rCurBlock.ucFrameCount;
				TxBlock2Fifo(pvSptHdl, prCfaMkv);
				return;
			}
		} else {
			MkvSkipBlock(pvSptHdl, prCfaMkv);
			return;
		}

		break;

	case CFA_PRS_BIT_STRM_TYPE_SP:
		prCfaMkv->rCurBlock.eBlockType = CFA_PRS_BIT_STRM_TYPE_SP;

		if ((prCfaMkv->u4PrsFlg  & CFA_PRS_BIT_STRM_TYPE_SP) == 0) {
			MkvSkipBlock(pvSptHdl, prCfaMkv);
			return;
		}
		{
			if ((prCfaMkv->aucHdr[0] & ((u8)0x06)) == 0)/*no lacing*/ {
				/*1byte frame_count*/
				prCfaMkv->rCurBlock.u8DataOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen + 1;
				prCfaMkv->rCurBlock.u8DataSize = (prCfaMkv->rCurBlock.u8BlockOfst +
					prCfaMkv->rCurBlock.u8BlockSize) - prCfaMkv->rCurBlock.u8DataOfst;
				prCfaMkv->rCurBlock.fgReady = TRUE;
				prCfaMkv->rCurBlock.fgDataReady = TRUE;
				prCfaMkv->rCurBlock.eLacingType = CFA_MKV_NO_LACING;
				if (prCfaMkv->rCurBlock.fgDurationReady == TRUE)
					TxBlock2Fifo(pvSptHdl, prCfaMkv);
				else {
					prCfaMkv->u8Ca = prCfaMkv->rCurBlock.u8DataOfst +
									prCfaMkv->rCurBlock.u8DataSize;
					prCfaMkv->u8HdrLen = 0;
					ToNextState(pvSptHdl, prCfaMkv, CFA_MKV_HDR_READ_BYTES, CFA_MKV_ST_SC_ANA);
				}
				return;

			} else if ((prCfaMkv->aucHdr[0] & ((u8)0x06)) == ((u8)0x02))/*Xiph lacing*/ {
				prCfaMkv->rCurBlock.ucFrameCount = prCfaMkv->aucHdr[1];/*frame_count - 1*/
				prCfaMkv->rCurBlock.eLacingType = CFA_MKV_XIPH_LACING;
				MkvMoveHdr(prCfaMkv, (u32)2);/*remove the 1 bytes flag and 1 byte frame count*/

				if (GetVaraibleSize(prCfaMkv->aucHdr) > ((u8)prCfaMkv->u8HdrLen)) {
					ToNextState(pvSptHdl, prCfaMkv,
					(u64)(GetVaraibleSize(prCfaMkv->aucHdr) - ((u8)prCfaMkv->u8HdrLen)),
					CFA_MKV_ST_LACING_ANA);
					return;
				}

				if (prCfaMkv->u8HdrLen >= prCfaMkv->rCurBlock.ucFrameCount) {
					prCfaMkv->rCurBlock.ucFrameCount = prCfaMkv->rCurBlock.ucFrameCount -
											MkvCountLacing(pvSptHdl, prCfaMkv);
				}
				if (prCfaMkv->rCurBlock.ucFrameCount == 0) {
					prCfaMkv->rCurBlock.u8DataOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen;
					prCfaMkv->rCurBlock.u8DataSize = (prCfaMkv->rCurBlock.u8BlockOfst +
						prCfaMkv->rCurBlock.u8BlockSize) - prCfaMkv->rCurBlock.u8DataOfst;
					prCfaMkv->rCurBlock.fgReady = TRUE;
					TxBlock2Fifo(pvSptHdl, prCfaMkv);
					return;
				}
				ToNextState(pvSptHdl, prCfaMkv,
					(u64)(prCfaMkv->rCurBlock.ucFrameCount) - (prCfaMkv->u8HdrLen),
					CFA_MKV_ST_LACING_ANA);

			} else if ((prCfaMkv->aucHdr[0] & ((u8)0x06)) == ((u8)0x06))/*EBML lacing*/ {
				prCfaMkv->rCurBlock.ucFrameCount = prCfaMkv->aucHdr[1];/*frame_count - 1*/
				prCfaMkv->rCurBlock.eLacingType = CFA_MKV_EBML_LACING;
				MkvMoveHdr(prCfaMkv, (u32)2);/*remove the 1 bytes flag and 1 byte frame count*/

				if (prCfaMkv->u8HdrLen >= prCfaMkv->rCurBlock.ucFrameCount) {
					prCfaMkv->rCurBlock.ucFrameCount = prCfaMkv->rCurBlock.ucFrameCount -
											MkvCountLacing(pvSptHdl, prCfaMkv);
				}
				if (prCfaMkv->rCurBlock.ucFrameCount == 0) {
					prCfaMkv->rCurBlock.u8DataOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen;
					prCfaMkv->rCurBlock.u8DataSize = (prCfaMkv->rCurBlock.u8BlockOfst +
										prCfaMkv->rCurBlock.u8BlockSize) -
										prCfaMkv->rCurBlock.u8DataOfst;
					prCfaMkv->rCurBlock.fgReady = TRUE;

					TxBlock2Fifo(pvSptHdl, prCfaMkv);
					return;
				}
				ToNextState(pvSptHdl, prCfaMkv,
						(u64)(prCfaMkv->rCurBlock.ucFrameCount) - (prCfaMkv->u8HdrLen),
						CFA_MKV_ST_LACING_ANA);

			} else/*fix lacing*/ {
				/*1 byte flag,1byte frame_count*/
				prCfaMkv->rCurBlock.u8DataOfst = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen + 2;
				prCfaMkv->rCurBlock.u8DataSize = (prCfaMkv->rCurBlock.u8BlockOfst +
										prCfaMkv->rCurBlock.u8BlockSize) -
										prCfaMkv->rCurBlock.u8DataOfst;
				prCfaMkv->rCurBlock.fgReady = TRUE;
				prCfaMkv->rCurBlock.eLacingType = CFA_MKV_FIX_LACING;
				TxBlock2Fifo(pvSptHdl, prCfaMkv);
				return;
			}


		}

		break;

	case CFA_PRS_BIT_STRM_TYPE_NONE:

		MkvSkipBlock(pvSptHdl, prCfaMkv);
		return;

	default:
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] eCfaMkvPrsStrmType is %d, Send EOS\r\n"),
			eCfaMkvPrsStrmType);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
		break;
	}
}




/*-----------------------------------------------------------------------------
* Name: vCfaMkvAnaLacing
*
* Description:
*	   Get Lacing value(EBML Lacing ,Xiph Lacing and Constant Lacing) when exist
*
*
* Inputs:
*
* Outputs:
*
* Returns: void
*
*-----------------------------------------------------------------------------*/
void CfaMkvAnaLacing(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	u32 u4TotalFrameSize = 0;
	s32 i4Loop = 0;

	if (prCfaMkv->u8HdrLen > CFA_MKV_HDR_BUF) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] CfaMkvAnaLacing::line %d,prCfaMkv->u8HdrLen=%lld is larger than 64\r\n"),
			DMX_LINE_NO, prCfaMkv->u8HdrLen);  
		if (DMX_IS_RW_PLAY(pvSptHdl)) {
			Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, FALSE, GAU_E_ERRCHUNK);
		} else {
			Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, TRUE, GAU_E_ERRCHUNK);
		}
		return;
	}

	if ((prCfaMkv->u8HdrLen + u8TxLen) > CFA_MKV_HDR_BUF) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] CfaMkvAnaLacing::prCfaMkv->u8HdrLen=%lld,u8TxLen=%lld\r\n"),
			prCfaMkv->u8HdrLen, u8TxLen);
		u8TxLen = CFA_MKV_HDR_BUF - prCfaMkv->u8HdrLen;
	}

	if (prCfaMkv->u8HdrLen < CFA_MKV_HDR_BUF) {
	    dmx_memcpy((void *)(prCfaMkv->aucHdr + prCfaMkv->u8HdrLen),
				(void *)prCfaMkv->ptrMemAddr, (u32)u8TxLen);
	}
	prCfaMkv->u8HdrLen += u8TxLen;

	prCfaMkv->rCurBlock.ucFrameCount = prCfaMkv->rCurBlock.ucFrameCount - MkvCountLacing(pvSptHdl, prCfaMkv);


	if (prCfaMkv->rCurBlock.ucFrameCount == 0) {
		prCfaMkv->rCurBlock.u8DataOfst = prCfaMkv->u8Ca;
		prCfaMkv->rCurBlock.u8DataSize = (prCfaMkv->rCurBlock.u8BlockOfst +
			prCfaMkv->rCurBlock.u8BlockSize) - prCfaMkv->rCurBlock.u8DataOfst;
		prCfaMkv->rCurBlock.fgReady = TRUE;
		for (i4Loop = 0; i4Loop < prCfaMkv->rCurBlock.i4FrameNum; i4Loop++)
			u4TotalFrameSize += prCfaMkv->rCurBlock.au4FrameSize[i4Loop];

		prCfaMkv->rCurBlock.au4FrameSize[prCfaMkv->rCurBlock.i4FrameNum] =
			(u32)prCfaMkv->rCurBlock.u8DataSize - u4TotalFrameSize;
		TxBlock2Fifo(pvSptHdl, prCfaMkv);
		return;
	}
	ToNextState(pvSptHdl, prCfaMkv, prCfaMkv->rCurBlock.ucFrameCount,
				CFA_MKV_ST_LACING_ANA);

}



/*-----------------------------------------------------------------------------
* Name: vCfaMkvTxBlock
*
* Description:
*	   When a block transfered return,this function will be call to process next action
*
*
* Inputs:
*
* Outputs:
*
* Returns: void
*
*-----------------------------------------------------------------------------*/
void CfaMkvTxBlock(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	prCfaMkv->u8HdrLen = 0;

	if (prCfaMkv->rCurBlock.eLacingType != CFA_MKV_NO_LACING) {
		if (prCfaMkv->rCurBlock.ucStrmIdx >= MAX_NS_MKV_AUD) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] ucStrmIdx is %d, line %d Send EOS\r\n"),
				prCfaMkv->rCurBlock.ucStrmIdx, DMX_LINE_NO);
			MkvFinishPrs(pvSptHdl,prCfaMkv);
			return;
		}

		if ((prCfaMkv->rCurBlock.eBlockType == CFA_PRS_BIT_STRM_TYPE_A) &&
		(prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec == CFA_AUD_DRV_FMT_VORBIS)) {
			/*vCfaMkvDbgMsg(("[CFA MKV]vCfaMkvTxBlock :
			Warning! Now it lacing, Do nothing for Vorbis ,it's already done
			in the vorbis flow\n"));*/
			/*Do nothing, the lacing rame is already done in the vorbis flow*/
		} else if (
			(prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.fgHeaderStriping
			== TRUE) &&
			(prCfaMkv->rCurBlock.i4CurFrameNum <= prCfaMkv->rCurBlock.i4FrameNum)) {
			TxHeader2Fifo(pvSptHdl, prCfaMkv);
			return;
		} else {
			/*do
			nothing*/
		}
	}


	prCfaMkv->u8Ca = prCfaMkv->rCurAvc.u8PayloadOffset + prCfaMkv->rCurAvc.u4PayloadSize;
	if (((prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_H264) ||
		(prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_H265)) &&
		(prCfaMkv->rCurBlock.eBlockType == CFA_PRS_BIT_STRM_TYPE_V) &&
		(prCfaMkv->u8Ca < (prCfaMkv->rCurBlock.u8DataOfst + prCfaMkv->rCurBlock.u8DataSize)) &&
		(prCfaMkv->rAbnormalFlags.fgFourCCH264H265 == FALSE)) {
		ToNextState(pvSptHdl, prCfaMkv, prCfaMkv->rVidStmInfo.u4NaluSize,
					CFA_MKV_ST_TX_H264H265_STARTCODE);

	} else {
		prCfaMkv->u8Ca = prCfaMkv->rCurGroup.u8GroupOfst + prCfaMkv->rCurGroup.u8GroupSize;
		if (prCfaMkv->u8Ca > prCfaMkv->rCurCluster.u8ClusterOfst + prCfaMkv->rCurCluster.u8ClusterSize +12) {
			prCfaMkv->u8Ca =prCfaMkv->rCurCluster.u8ClusterOfst + prCfaMkv->rCurCluster.u8ClusterSize;            
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL]error chunk:prCfaMkv->u8ClusterOfst = %lld ,u8ClusterSize = %lld\r\n"),
				prCfaMkv->rCurCluster.u8ClusterOfst, prCfaMkv->rCurCluster.u8ClusterSize);
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL]error chunk:prCfaMkv->u8Ca = %lld\r\n"), prCfaMkv->u8Ca);
			if (DMX_IS_RW_PLAY(pvSptHdl)) {
				Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, FALSE, GAU_E_ERRCHUNK);
			} else {
				Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, TRUE, GAU_E_ERRCHUNK);
			}
			return;
		}
		ToNextState(pvSptHdl, prCfaMkv, CFA_MKV_HDR_READ_BYTES, CFA_MKV_ST_SC_ANA);
	}

}

static void CfaMkvTxMpeg4VolHeader(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	MRESULT mrResult = RET_DMX_OK;

	GenerateVirtualMP4SequenceHeader(prCfaMkv, (u8 *)(prCfaMkv->rVidStmInfo.pucMpeg4Header), (u32)u8TxLen);
	DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
		TEXT("[CFA_MKV_ST_CTRL] Entry CfaMkvTxMpeg4VolHeader\r\n"));

	mrResult = Spt4CfaBuf2VFifo(pvSptHdl, prCfaMkv->pu1Mp4SeqHdr, 0,
		CFA_PTM_SAME_POS, CFA_VID_MPEG4, CFA_MKV_MP4_SEQ_HDR_LEN);
	if (RET_DMX_OK != mrResult) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
		return;
	}

	prCfaMkv->eCurState = CFA_MKV_ST_TX_DIXV_HDR_RET;
	prCfaMkv->rVidStmInfo.fgTxMpeg4VOLHeader = TRUE;

}

static void CfaMkvTxDivxHdr(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	MRESULT mrResult = RET_DMX_OK;

	mrResult = Spt4CfaBuf2VFifo(pvSptHdl, prCfaMkv->rVidStmInfo.pucDivxHdrBuf, 0,
		CFA_PTM_SAME_POS, CFA_VID_DIVX4, prCfaMkv->rVidStmInfo.u4DivxHdrLen);
	if (RET_DMX_OK != mrResult) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
		return;
	}

	prCfaMkv->eCurState = CFA_MKV_ST_TX_DIXV_HDR_RET;
	//prCfaMkv->rVidStmInfo.u4DivxHdrLen = 0;

}

static void CfaMkvTxDivxHdrRet(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	prCfaMkv->u8Ca = prCfaMkv->rRange.u8VidStartOfst;
	ToNextState(pvSptHdl, prCfaMkv, CFA_MKV_HDR_READ_BYTES, CFA_MKV_ST_SC_ANA);
}


/*-----------------------------------------------------------------------------
* Name: vCfaMkvTxSequenceInfo
*
* Description:
*	   Transfer SequenceHeader when necessary.
*
*
* Inputs:
*
* Outputs:
*
* Returns: void
*
*-----------------------------------------------------------------------------*/
static void CfaMkvTxSequenceInfo(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	MRESULT mrResult = RET_DMX_OK;
	u32 u4PpsSpsSize = 0;
	CFA_VIDEO_INFO_T rVidInf = {0};

	rVidInf.eTxMode = CFA_PTM_SAME_POS;
	rVidInf.eVidType = prCfaMkv->rVidStmInfo.eVidCodec;
	rVidInf.u4PrsStrmId = (u32)prCfaMkv->rVidStmInfo.u8VidTrackNo;

	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA_MKV_ST_CTRL] CfaMkvTxSequenceInfo, TxLen=%d"), u8TxLen);

	if (prCfaMkv->fgTxSeqHdrFromBuf)
		prCfaMkv->ptrMemAddr = (uintptr_t)prCfaMkv->rVidStmInfo.pucCodecPrivBuf;

	switch (prCfaMkv->rVidStmInfo.eVidCodec) {
	case CFA_VID_H264: {
		if (prCfaMkv->rVidStmInfo.rSPSPPSInfo.fgHasSPSStartCode) {
			if (prCfaMkv->rVidStmInfo.rSPSPPSInfo.u4SPSNum > 0)
				prCfaMkv->eSequenceType = MKV_SEQUENCE_SPS;
			else if ((prCfaMkv->eSequenceType == MKV_SEQUENCE_SPS) &&
				(0 == prCfaMkv->rVidStmInfo.rSPSPPSInfo.u4SPSNum))
				prCfaMkv->eSequenceType = MKV_SEQUENCE_PPS;
			else {
				/*do
				nothing*/
			}

			if (prCfaMkv->eSequenceType == MKV_SEQUENCE_PPS)
				prCfaMkv->rVidStmInfo.rSPSPPSInfo.u4PPSNum--;
			else if (prCfaMkv->eSequenceType == MKV_SEQUENCE_SPS)
				prCfaMkv->rVidStmInfo.rSPSPPSInfo.u4SPSNum--;
			else {
				DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA_MKV_ST_CTRL] send eos, H264 sequence type error!\r\n"));
				MkvFinishPrs(pvSptHdl, prCfaMkv);
				return;
			}

			if ((prCfaMkv->eSequenceType == MKV_SEQUENCE_PPS) && (prCfaMkv->uPpsNs == 0)) {
				/*prCfaMkv->u8Ca = prCfaMkv->rVidStmInfo.u8CodecPrivOfst +
									prCfaMkv->rVidStmInfo.u8CodecPrivLen;*/
				prCfaMkv->rVidStmInfo.u8CodecPrivLen = 0;
				prCfaMkv->eCurState = CFA_MKV_ST_TX_SEQUENCE_RETURN;
			} else
				prCfaMkv->eCurState = CFA_MKV_ST_TX_SEQUENCE_INFO;
		} else {
			if (prCfaMkv->rVidStmInfo.u8CodecPrivLen == u8TxLen) {
				if (prCfaMkv->rAbnormalFlags.fgFourCCH264H265 == TRUE) {
					/*when 4cc h264, seqhdr have no nalu size*/
					prCfaMkv->rVidStmInfo.u4NaluSize = 0;
					prCfaMkv->u4SeqHdrMemOfst = 0;
				} else {
					prCfaMkv->rVidStmInfo.u4NaluSize =
						(u32)(*((u8 *)(prCfaMkv->ptrMemAddr)) & (u8)0x3) + (u32)1;
					prCfaMkv->u4SeqHdrMemOfst = 1;/*this 2 lines for get NALU size*/
				} 
				prCfaMkv->uSpsNs =
					(u8)(MkvComputeEndian((u8 *)(prCfaMkv->ptrMemAddr +
											prCfaMkv->u4SeqHdrMemOfst), (u8)1))
											& (u8)0x1f;

				prCfaMkv->eSequenceType = MKV_SEQUENCE_SPS;
				prCfaMkv->u4SeqHdrMemOfst++;
			} else if ((prCfaMkv->eSequenceType == MKV_SEQUENCE_SPS) && (prCfaMkv->uSpsNs == 0)) {
				prCfaMkv->uPpsNs =
					(u8)(MkvComputeEndian((u8 *)(prCfaMkv->ptrMemAddr +
							prCfaMkv->u4SeqHdrMemOfst), (u8)1));
				prCfaMkv->eSequenceType = MKV_SEQUENCE_PPS;
				prCfaMkv->u4SeqHdrMemOfst++;
			} else {
					/*do
					nothing*/
			}

			u4PpsSpsSize = (u32)MkvComputeEndian(
				(u8 *)(prCfaMkv->ptrMemAddr + prCfaMkv->u4SeqHdrMemOfst), (u8)CFA_MKV_SPS_PPS_SIZE);
			prCfaMkv->u4SeqHdrMemOfst += CFA_MKV_SPS_PPS_SIZE;

			if (prCfaMkv->eSequenceType == MKV_SEQUENCE_PPS)
				prCfaMkv->uPpsNs--;
			else if (prCfaMkv->eSequenceType == MKV_SEQUENCE_SPS)
				prCfaMkv->uSpsNs--;
			else {
				DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA_MKV_ST_CTRL] send eos, H264 sequence type error!\r\n"));
				MkvFinishPrs(pvSptHdl, prCfaMkv);
				return;
			}
			if (((prCfaMkv->eSequenceType == MKV_SEQUENCE_PPS) && (prCfaMkv->uPpsNs == 0)) ||
				(u4PpsSpsSize == (u32)(prCfaMkv->rVidStmInfo.u8CodecPrivLen - (u64)1))) {
				/*prCfaMkv->u8Ca = prCfaMkv->rVidStmInfo.u8CodecPrivOfst +
								prCfaMkv->rVidStmInfo.u8CodecPrivLen;*/
				prCfaMkv->rVidStmInfo.u8CodecPrivLen = 0;
				prCfaMkv->eCurState = CFA_MKV_ST_TX_SEQUENCE_RETURN;
			} else
				prCfaMkv->eCurState = CFA_MKV_ST_TX_SEQUENCE_INFO;

			rVidInf.u8FileOfst = prCfaMkv->rVidStmInfo.u8CodecPrivOfst + prCfaMkv->u4SeqHdrMemOfst;
			rVidInf.u8Len = u4PpsSpsSize;

			prCfaMkv->u4SeqHdrMemOfst += u4PpsSpsSize;
		}
	}
		break;

	case CFA_VID_WMV7:
	case CFA_VID_WMV8:
	case CFA_VID_WMV9:
	case CFA_VID_VC1:
		if ((prCfaMkv->rVidStmInfo.u8CodecPrivLen >= (u64)4) &&
			(prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_WMV9)) {
			prCfaMkv->fgPrsInterp = (bool)(((*(u8 *)(prCfaMkv->ptrMemAddr + 3)) & (u8)0x02) >> 1);
			prCfaMkv->fgPrsPre = (bool)(((*(u8 *)(prCfaMkv->ptrMemAddr + 3)) & (u8)0x80) >> 7);
			prCfaMkv->u4PrsBFrameNs = (u32)(((*(u8 *)(prCfaMkv->ptrMemAddr + 3)) & (u8)0x70) >> 4);
		}
		rVidInf.u8FileOfst = prCfaMkv->rVidStmInfo.u8CodecPrivOfst;
		rVidInf.u8Len = prCfaMkv->rVidStmInfo.u8CodecPrivLen;
		rVidInf.eTxMode = CFA_PTM_WMV_SEQHDR;
		rVidInf.fgUnitStart = TRUE;
		rVidInf.u8TotalAULen = prCfaMkv->rVidStmInfo.u8CodecPrivLen;

		prCfaMkv->u8Ca = prCfaMkv->rVidStmInfo.u8CodecPrivOfst + prCfaMkv->rVidStmInfo.u8CodecPrivLen;
		if (prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_WMV9)
			prCfaMkv->rVidStmInfo.u8CodecPrivLen = 0;

		prCfaMkv->rVidStmInfo.fgWmvSeqRet = TRUE;
		prCfaMkv->eCurState = CFA_MKV_ST_TX_SEQUENCE_RETURN;

		break;

	default:
		rVidInf.u8FileOfst = prCfaMkv->rVidStmInfo.u8CodecPrivOfst;
		rVidInf.u8Len = prCfaMkv->rVidStmInfo.u8CodecPrivLen;
		prCfaMkv->u8Ca = prCfaMkv->rVidStmInfo.u8CodecPrivOfst + prCfaMkv->rVidStmInfo.u8CodecPrivLen;
		prCfaMkv->rVidStmInfo.u8CodecPrivLen = 0;

		prCfaMkv->eCurState = CFA_MKV_ST_TX_SEQUENCE_RETURN;

		break;
	}

	if (prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_H264) {
		if (prCfaMkv->rVidStmInfo.rSPSPPSInfo.fgHasSPSStartCode) {
			rVidInf.u8FileOfst = 0;

			if (prCfaMkv->eSequenceType == MKV_SEQUENCE_PPS) {
				if ((prCfaMkv->rVidStmInfo.rSPSPPSInfo.u4PPSDataLen + 4) > CFA_MKV_AVC_PPS_BUF_LEN) {
					DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
						TEXT("[CFA_MKV_ST_CTRL] WARNINIG::u4PpsSpsSize is %d, line %d\r\n"),
					   prCfaMkv->rVidStmInfo.rSPSPPSInfo.u4PPSDataLen, DMX_LINE_NO);
					prCfaMkv->rVidStmInfo.rSPSPPSInfo.u4PPSDataLen = CFA_MKV_AVC_PPS_BUF_LEN - 4;
				}

				dmx_memcpy(prCfaMkv->pucAvcPPSBuf + 4,
						(u8 *)(prCfaMkv->ptrMemAddr +
						prCfaMkv->rVidStmInfo.rSPSPPSInfo.u4SPSDataLen + 4),
						prCfaMkv->rVidStmInfo.rSPSPPSInfo.u4PPSDataLen);

				/*i4Ret = Spt4CfaBuf2VFifo(pvSptHdl, prCfaMkv->pucAvcPPSBuf, 0,
				CFA_PTM_SAME_POS, CFA_VID_H264, 4 + u4PpsSpsSize);*/

				DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA_MKV_ST_CTRL] line %d 0x%x 0x%x 0x%x 0x%x \r\n"), DMX_LINE_NO,
					*(u8 *)(prCfaMkv->ptrMemAddr +
					prCfaMkv->rVidStmInfo.rSPSPPSInfo.u4SPSDataLen + 4),
					*(u8 *)(prCfaMkv->ptrMemAddr +
					prCfaMkv->rVidStmInfo.rSPSPPSInfo.u4SPSDataLen + 4 + 1),
					*(u8 *)(prCfaMkv->ptrMemAddr +
					prCfaMkv->rVidStmInfo.rSPSPPSInfo.u4SPSDataLen + 4 + 2),
					*(u8 *)(prCfaMkv->ptrMemAddr +
					prCfaMkv->rVidStmInfo.rSPSPPSInfo.u4SPSDataLen + 4 + 3));

				rVidInf.u8Len = 4 + prCfaMkv->rVidStmInfo.rSPSPPSInfo.u4PPSDataLen;

				mrResult = Spt4CfaBuf2VFifoAUCtrl(pvSptHdl, prCfaMkv->pucAvcPPSBuf, &rVidInf);
				if (RET_DMX_OK != mrResult) {
					DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
						TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
					MkvFinishPrs(pvSptHdl, prCfaMkv);
					return;
				}
			} else if (prCfaMkv->eSequenceType == MKV_SEQUENCE_SPS) {
				if ((prCfaMkv->rVidStmInfo.rSPSPPSInfo.u4SPSDataLen + 4) > CFA_MKV_AVC_PPS_BUF_LEN) {
					DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
						TEXT("[CFA_MKV_ST_CTRL] WARNING::u4PpsSpsSize is %d, line %d\r\n"),
					   prCfaMkv->rVidStmInfo.rSPSPPSInfo.u4SPSDataLen, DMX_LINE_NO);
					prCfaMkv->rVidStmInfo.rSPSPPSInfo.u4SPSDataLen = CFA_MKV_AVC_SPS_BUF_LEN - 4;
				}

				dmx_memcpy(prCfaMkv->pucAvcSPSBuf + 4,
						(u8 *)(prCfaMkv->ptrMemAddr),
						prCfaMkv->rVidStmInfo.rSPSPPSInfo.u4SPSDataLen);

				DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA_MKV_ST_CTRL]line %d 0x%x 0x%x 0x%x 0x%x \r\n"), DMX_LINE_NO,
					*(u8 *)(prCfaMkv->ptrMemAddr),
					*(u8 *)(prCfaMkv->ptrMemAddr+1),
					*(u8 *)(prCfaMkv->ptrMemAddr+2),
					*(u8 *)(prCfaMkv->ptrMemAddr+3));

				rVidInf.u8Len = 4 + prCfaMkv->rVidStmInfo.rSPSPPSInfo.u4SPSDataLen;

				/*i4Ret = Spt4CfaBuf2VFifo(pvSptHdl, prCfaMkv->pucAvcSPSBuf, 0, CFA_PTM_SAME_POS,
				CFA_VID_H264, 4 + u4PpsSpsSize);*/
				mrResult = Spt4CfaBuf2VFifoAUCtrl(pvSptHdl, prCfaMkv->pucAvcSPSBuf, &rVidInf);

				if (RET_DMX_OK != mrResult) {
					DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
						TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
					MkvFinishPrs(pvSptHdl, prCfaMkv);
					return;
				}
			} else {
				/*do 
				nothing*/
			}
		} else {
			rVidInf.u8FileOfst = 0;
			rVidInf.u8Len = 4 + u4PpsSpsSize;

			if (prCfaMkv->eSequenceType == MKV_SEQUENCE_PPS) {
				if ((u4PpsSpsSize + 4) > CFA_MKV_AVC_PPS_BUF_LEN) {
					DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
						TEXT("[CFA_MKV_ST_CTRL] WARNING::u4PpsSpsSize is %d, line %d\r\n"),
						u4PpsSpsSize, DMX_LINE_NO);
					u4PpsSpsSize = CFA_MKV_AVC_PPS_BUF_LEN - 4;
				}

				dmx_memcpy(prCfaMkv->pucAvcPPSBuf + 4,
						(u8 *)(prCfaMkv->ptrMemAddr +
						prCfaMkv->u4SeqHdrMemOfst - u4PpsSpsSize),
						u4PpsSpsSize);

				/*i4Ret = Spt4CfaBuf2VFifo(pvSptHdl, prCfaMkv->pucAvcPPSBuf, 0,
				CFA_PTM_SAME_POS, CFA_VID_H264, 4 + u4PpsSpsSize);*/

				DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA_MKV_ST_CTRL] line %d 0x%x 0x%x 0x%x 0x%x \r\n"), DMX_LINE_NO,
					*(u8 *)(prCfaMkv->ptrMemAddr + prCfaMkv->u4SeqHdrMemOfst - u4PpsSpsSize),
					*(u8 *)(prCfaMkv->ptrMemAddr + prCfaMkv->u4SeqHdrMemOfst - u4PpsSpsSize+1),
					*(u8 *)(prCfaMkv->ptrMemAddr + prCfaMkv->u4SeqHdrMemOfst - u4PpsSpsSize+2),
					*(u8 *)(prCfaMkv->ptrMemAddr + prCfaMkv->u4SeqHdrMemOfst - u4PpsSpsSize+3));

				mrResult = Spt4CfaBuf2VFifoAUCtrl(pvSptHdl, prCfaMkv->pucAvcPPSBuf, &rVidInf);
				if (RET_DMX_OK != mrResult) {
					DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
						TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
					MkvFinishPrs(pvSptHdl, prCfaMkv);
					return;
				}
			} else if (prCfaMkv->eSequenceType == MKV_SEQUENCE_SPS) {
				if ((u4PpsSpsSize + 4) > CFA_MKV_AVC_PPS_BUF_LEN) {
					DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
						TEXT("[CFA_MKV_ST_CTRL] u4PpsSpsSize is %d, line %d\r\n"),
					   u4PpsSpsSize, DMX_LINE_NO);
					u4PpsSpsSize = CFA_MKV_AVC_PPS_BUF_LEN - 4;
				}

				dmx_memcpy(prCfaMkv->pucAvcSPSBuf + 4,
						(u8 *)(prCfaMkv->ptrMemAddr +
						prCfaMkv->u4SeqHdrMemOfst - u4PpsSpsSize),
						u4PpsSpsSize);

				DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA_MKV_ST_CTRL]line %d 0x%x 0x%x 0x%x 0x%x \r\n"), DMX_LINE_NO,
					*(u8 *)(prCfaMkv->ptrMemAddr + prCfaMkv->u4SeqHdrMemOfst - u4PpsSpsSize),
					*(u8 *)(prCfaMkv->ptrMemAddr + prCfaMkv->u4SeqHdrMemOfst - u4PpsSpsSize+1),
					*(u8 *)(prCfaMkv->ptrMemAddr + prCfaMkv->u4SeqHdrMemOfst - u4PpsSpsSize+2),
					*(u8 *)(prCfaMkv->ptrMemAddr + prCfaMkv->u4SeqHdrMemOfst - u4PpsSpsSize+3));

				/*i4Ret = Spt4CfaBuf2VFifo(pvSptHdl, prCfaMkv->pucAvcSPSBuf, 0,
				CFA_PTM_SAME_POS, CFA_VID_H264, 4 + u4PpsSpsSize);*/
				mrResult = Spt4CfaBuf2VFifoAUCtrl(pvSptHdl, prCfaMkv->pucAvcSPSBuf, &rVidInf);

				if (RET_DMX_OK != mrResult) {
					DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
						TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
					MkvFinishPrs(pvSptHdl, prCfaMkv);
					return;
				}
			} else {
				/*do
				nothing*/
			}
		}
	} else if (prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_VC1) {
		if (u8TxLen > CFA_MKV_WVC1_SPECDATA_LEN) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] u8TxLen is %d, line %d Send EOS\r\n"),
			   u8TxLen, DMX_LINE_NO);
			MkvFinishPrs(pvSptHdl,prCfaMkv);
			return;
		}

		dmx_memcpy(prCfaMkv->pucWVC1SpecData, (u8 *)(prCfaMkv->ptrMemAddr), (u32)u8TxLen);
		mrResult = Spt4CfaBuf2VFifoAUCtrl(pvSptHdl, prCfaMkv->pucWVC1SpecData, &rVidInf);
		if (RET_DMX_OK != mrResult) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
			MkvFinishPrs(pvSptHdl, prCfaMkv);
			return;
		}
	} else {
		if (prCfaMkv->fgTxSeqHdrFromBuf) {
			mrResult = Spt4CfaBuf2VFifoAUCtrl(pvSptHdl, prCfaMkv->rVidStmInfo.pucCodecPrivBuf, &rVidInf);
			if (RET_DMX_OK != mrResult) {
				DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
				MkvFinishPrs(pvSptHdl, prCfaMkv);
				return;
			}
		} else
			CfaMkvPbb2FifoAUCtrl(prCfaMkv, pvSptHdl, (void *)&rVidInf, CFA_PRS_BIT_STRM_TYPE_V, 0);

	}
	DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA_MKV_ST_CTRL] Transfer Sequence Header\r\n"));

}

static void CfaMkvTxAvcPayload(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	CFA_VIDEO_INFO_T rVidInf = {0};
	u8 aucHdr[CFA_MKV_AVC_HEADER_LEN] = {0};

	/*DMX_ASSERT((prCfaMkv->u8HdrLen + u8TxLen) <= CFA_MKV_HDR_BUF);*/
	if ((prCfaMkv->u8HdrLen + u8TxLen) > CFA_MKV_HDR_BUF) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] u8HdrLen is %lld, u8TxLen is %lld,line %d Send EOS\r\n"),
			prCfaMkv->u8HdrLen, u8TxLen, DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl,prCfaMkv);
		return;
	}

	dmx_memcpy((void *)(prCfaMkv->aucHdr + prCfaMkv->u8HdrLen),
					   (void *)prCfaMkv->ptrMemAddr, (u32)u8TxLen);

	prCfaMkv->u8HdrLen += u8TxLen;

	if (prCfaMkv->fgNeedAddHeadStrip) {
		u8 len = prCfaMkv->rVidStmInfo.rCfaMkvHeader.uHeaderLen;

		prCfaMkv->u8HdrLen -= prCfaMkv->rVidStmInfo.u4NaluSize;
		if (len >= prCfaMkv->rVidStmInfo.u4NaluSize) {
			if (prCfaMkv->rVidStmInfo.u4NaluSize > CFA_MKV_AVC_HEADER_LEN) {
				DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA_MKV_ST_CTRL] u4NaluSize is %d, line %d Send EOS\r\n"),
					prCfaMkv->rVidStmInfo.u4NaluSize, DMX_LINE_NO);
				MkvFinishPrs(pvSptHdl,prCfaMkv);
				return;
			}
			mm_memcpy(aucHdr, prCfaMkv->rVidStmInfo.rCfaMkvHeader.auHeader,
						prCfaMkv->rVidStmInfo.u4NaluSize);
		} else {
			if (len > CFA_MKV_AVC_HEADER_LEN) {
				DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA_MKV_ST_CTRL] len is %d, line %d Send EOS\r\n"),
				   len, DMX_LINE_NO);
				MkvFinishPrs(pvSptHdl,prCfaMkv);
				return;
			}
			mm_memcpy(aucHdr, prCfaMkv->rVidStmInfo.rCfaMkvHeader.auHeader, len);
			if (len < CFA_MKV_AVC_HEADER_LEN) {
			    mm_memcpy(aucHdr + len, prCfaMkv->aucHdr, prCfaMkv->rVidStmInfo.u4NaluSize - len);
			}
			prCfaMkv->u8HdrLen += len;
		}
		prCfaMkv->rCurAvc.u4PayloadSize =
			(u32)MkvComputeEndian(aucHdr, (u8)prCfaMkv->rVidStmInfo.u4NaluSize);

		prCfaMkv->fgNeedAddHeadStrip = FALSE;
	} else {
		prCfaMkv->rCurAvc.u4PayloadSize =
			(u32)MkvComputeEndian(prCfaMkv->aucHdr, (u8)prCfaMkv->rVidStmInfo.u4NaluSize);
		prCfaMkv->u8HdrLen -= prCfaMkv->rVidStmInfo.u4NaluSize;
	}

	prCfaMkv->rCurAvc.u8PayloadOffset = prCfaMkv->u8Ca - prCfaMkv->u8HdrLen;
	   if (((prCfaMkv->rCurGroup.fgGroupEn)) && 
        ((prCfaMkv->rCurBlock.u8DataOfst + prCfaMkv->rCurBlock.u8DataSize) 
        > ((prCfaMkv->rCurGroup.u8GroupOfst + prCfaMkv->rCurGroup.u8GroupSize)))) {    
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL]u8PayloadOffset = %lld , u4PayloadSize=%d!!\r\n"),
			prCfaMkv->rCurAvc.u8PayloadOffset, prCfaMkv->rCurAvc.u4PayloadSize);
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL]u8GroupOfst  = %lld , u8GroupSize=%lld!!\r\n"),
			prCfaMkv->rCurGroup.u8GroupOfst, prCfaMkv->rCurGroup.u8GroupSize);
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL]u8DataOfst = %lld , u8DataSize=%lld!!\r\n"),
			prCfaMkv->rCurBlock.u8DataOfst,prCfaMkv->rCurBlock.u8DataSize);
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL]prCfaMkv->rCurAvc.u4PayloadSize error!!\r\n"));
		prCfaMkv->rCurBlock.u8DataSize = prCfaMkv->rCurGroup.u8GroupOfst 
						+ prCfaMkv->rCurGroup.u8GroupSize
						- prCfaMkv->rCurBlock.u8DataOfst;
	}
	if ((prCfaMkv->rCurAvc.u8PayloadOffset + prCfaMkv->rCurAvc.u4PayloadSize) >
		(prCfaMkv->rCurBlock.u8DataOfst + prCfaMkv->rCurBlock.u8DataSize)) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL]u8PayloadOffset = %lld , u4PayloadSize=%d!!\r\n"),
			prCfaMkv->rCurAvc.u8PayloadOffset,prCfaMkv->rCurAvc.u4PayloadSize);      
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL]u8DataOfst = %lld , u8DataSize=%lld!!\r\n"),
			prCfaMkv->rCurBlock.u8DataOfst,prCfaMkv->rCurBlock.u8DataSize);
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL]prCfaMkv->rCurAvc.u4PayloadSize error!!\r\n"));
		prCfaMkv->rCurAvc.u4PayloadSize = (u32)(prCfaMkv->rCurBlock.u8DataOfst +
			prCfaMkv->rCurBlock.u8DataSize - prCfaMkv->rCurAvc.u8PayloadOffset);
	}

	prCfaMkv->u8HdrLen = 0;

	rVidInf.eVidType = prCfaMkv->rVidStmInfo.eVidCodec;
	rVidInf.eTxMode = CFA_PTM_EXACT_POS;
	rVidInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;
	rVidInf.u8FileOfst = prCfaMkv->rCurAvc.u8PayloadOffset;
	rVidInf.u8Len = prCfaMkv->rCurAvc.u4PayloadSize;

	prCfaMkv->eCurState = CFA_MKV_ST_TX_BLOCK;

	if((prCfaMkv->rCurAvc.u8PayloadOffset >
	(prCfaMkv->rCurBlock.u8DataOfst + prCfaMkv->rCurBlock.u8DataSize))) {
		prCfaMkv->u8Ca = prCfaMkv->rCurBlock.u8DataOfst + prCfaMkv->rCurBlock.u8DataSize;
		prCfaMkv->fgNoNeedSyncPb = TRUE;
	} else {
		prCfaMkv->u8Ca = prCfaMkv->rCurAvc.u8PayloadOffset + prCfaMkv->rCurAvc.u4PayloadSize;
		CfaMkvPbb2FifoAUCtrl(prCfaMkv, pvSptHdl, &rVidInf, CFA_PRS_BIT_STRM_TYPE_V, 0);
	}

	prCfaMkv->eCurPrsStrm = CFA_PRS_BIT_STRM_TYPE_V;
}


/*-----------------------------------------------------------------------------
* Name: vCfaMkvTxSequenceRet
*
* Description:
*	   After transfer end,this function will be call to start next pasing
*
*
* Inputs:
*
* Outputs:
*
* Returns: void
*
*-----------------------------------------------------------------------------*/
static void CfaMkvTxSequenceRet(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{

	if (((prCfaMkv->rVidStmInfo.u8CodecPrivLen) == 0) ||
		(prCfaMkv->rVidStmInfo.fgWmvSeqRet)) {
		prCfaMkv->u8Ca = prCfaMkv->rRange.u8VidStartOfst;

		ToNextState(pvSptHdl, prCfaMkv, CFA_MKV_HDR_READ_BYTES, CFA_MKV_ST_SC_ANA);
		return;
	}

	switch (prCfaMkv->rVidStmInfo.eVidCodec) {
	case CFA_VID_H264:
		break;
	default:
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] video codec is not H264!\r\n"));
		return;
	}
}

static void CfaMkvTxH264StartCode(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	prCfaMkv->u8Ca -= u8TxLen;
	TxAVCHEVCStartCode(pvSptHdl, prCfaMkv);
}


static void CfaMkvTxH264H265StartCodeRet(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	switch (prCfaMkv->rVidStmInfo.eVidCodec) {
	case CFA_VID_H264:
	case CFA_VID_H265:
		prCfaMkv->u8Ca -= u8TxLen;
		ToNextState(pvSptHdl, prCfaMkv, prCfaMkv->rVidStmInfo.u4NaluSize, CFA_MKV_ST_TX_H264H265);
		break;
	default:
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] video codec is not H264!\r\n"));
		return;
	}
}



static void CfaMkvWvc1Ana(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	u32 u4Header = 0;

	if (0 == prCfaMkv->ptrMemAddr) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] memory addr is zero!!\r\n"));      
		MkvFinishPrs(pvSptHdl, prCfaMkv);
	}
	if ((prCfaMkv->u8HdrLen + u8TxLen) > CFA_MKV_HDR_BUF) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] u8HdrLen is %lld, u8TxLen is %lld,line %d Send EOS\r\n"),
			prCfaMkv->u8HdrLen, u8TxLen, DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl,prCfaMkv);
		return;
	}
	dmx_memcpy((void *)(prCfaMkv->aucHdr + prCfaMkv->u8HdrLen),
					   (void *)prCfaMkv->ptrMemAddr, (u32)u8TxLen);

	prCfaMkv->u8HdrLen += u8TxLen;

	u4Header = (u32)MkvComputeEndian(prCfaMkv->aucHdr, (u8)4);
	prCfaMkv->u8HdrLen = 0;
	switch (u4Header) {
	case 0x10D:
	case 0x10E:
	case 0x10F:
		prCfaMkv->fgWVC1TxHeader = FALSE;
		prCfaMkv->rVidStmInfo.fgFirstVC1 = TRUE;
		TxBlock2Fifo(pvSptHdl, prCfaMkv);
		break;

	default:
		prCfaMkv->fgWVC1TxHeader = TRUE;
		prCfaMkv->rVidStmInfo.fgFirstVC1 = TRUE;
		prCfaMkv->fgTxVC1SeqHdr = FALSE;
		TxBlock2Fifo(pvSptHdl, prCfaMkv);
		break;

	}


}

static void CfaMkvWvc1TxDone(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	prCfaMkv->fgTxVC1SeqHdr = TRUE;
	TxBlock2Fifo(pvSptHdl, prCfaMkv);
}


static void CfaMkvTxHeader(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	CFA_AUDIO_INFO_T rAudInf = {0};
	CFA_VIDEO_INFO_T rVidInf = {0};

	prCfaMkv->eCurState = CFA_MKV_ST_TX_BLOCK;

	switch (prCfaMkv->rCurBlock.eBlockType) {
	case CFA_PRS_BIT_STRM_TYPE_V:
		rVidInf.eVidType = prCfaMkv->rVidStmInfo.eVidCodec;
		rVidInf.u8FileOfst = prCfaMkv->rCurBlock.u8DataOfst;
		rVidInf.u8Len = prCfaMkv->rCurBlock.u8DataSize;
		rVidInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;

		DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
			TEXT("[CFA_MKV_ST_CTRL] Video Data Tx Offset	: 0x%llx\r\n"),
			prCfaMkv->rCurBlock.u8DataOfst);


		if ((prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_DIVX3) ||
			(prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_WMV7) ||
			(prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_WMV8) ||
			(prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_WMV9) ||
			(prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_H263_SORENSON)) {
			rVidInf.eTxMode = prCfaMkv->eCfaMkvTxMode;
		}

		prCfaMkv->fgSecondFillAU = FALSE;
		CfaMkvPbb2FifoAUCtrl(prCfaMkv, pvSptHdl, (void *)&rVidInf, CFA_PRS_BIT_STRM_TYPE_V, (u64)0);
		break;

	case CFA_PRS_BIT_STRM_TYPE_A:
		rAudInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;
		if (prCfaMkv->rCurBlock.ucStrmIdx >= MAX_NS_MKV_AUD) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] ucStrmIdx is %d, line %d Send EOS\r\n"),
			   prCfaMkv->rCurBlock.ucStrmIdx, DMX_LINE_NO);
			MkvFinishPrs(pvSptHdl, prCfaMkv);
			return;
		}

		rAudInf.eAudType = prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec;
		rAudInf.u8FileOfst = prCfaMkv->rCurBlock.u8NextTxOfst;
		rAudInf.u8Len = prCfaMkv->rCurBlock.au4FrameSize[prCfaMkv->rCurBlock.i4CurFrameNum];
		#ifdef NOT_USED    /* mask by mtk40148*/
		if (prCfaMkv->rCurBlock.i4CurFrameNum == 0)
			rAudInf.u8Pts = prCfaMkv->rCurBlock.u8Pts;
		else
			rAudInf.u8Pts = INVALID_TIMESTAMP;

		#else
			rAudInf.u8Pts = prCfaMkv->rCurBlock.u8Pts;
		#endif
		rAudInf.fgUnitStart = FALSE;
		if ((prCfaMkv->rCurBlock.u8DataSize == DMX_INVALID_UINT64) ||
			(prCfaMkv->rCurBlock.u8DataOfst == DMX_INVALID_UINT64)) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] send eos, invalid datasize or offset!\r\n"));
			MkvFinishPrs(pvSptHdl, prCfaMkv);
			return;
		}
		CfaMkvPbb2FifoAUCtrl(prCfaMkv, pvSptHdl, (void *)&rAudInf, CFA_PRS_BIT_STRM_TYPE_A, (u64)0);

		prCfaMkv->rCurBlock.i4CurFrameNum++;
		prCfaMkv->rCurBlock.u8NextTxOfst += rAudInf.u8Len;
		break;

	case CFA_PRS_BIT_STRM_TYPE_SP:
		break;

	default:
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] eBlockType is %d, Send EOS\r\n"),
			prCfaMkv->rCurBlock.eBlockType);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
		break;
	}
}


static void CfaMkvTxAacHeader(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	TxBlock2Fifo(pvSptHdl, prCfaMkv);
}

#if CONFIG_CFA_MKV_SUPPORT_DECOMPRESSION
static void CfaMkvDecompression(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	s32 i4Ret = 0;
	MRESULT mrResult = RET_DMX_OK;

	CFA_SUBPIC_INFO_T rSpInf = {0};
	z_stream Stream = {0};
	u8 *pucDestBuf = NULL;
	s32 i4Loop = 0;
	u8 idx = prCfaMkv->rCurBlock.ucStrmIdx;

	dmx_memset(prCfaMkv->auDecompBuf, 0, CFA_MKV_DECOMP_BUF_LEN);

	switch (prCfaMkv->arCfaMkvSpStreamInfo[idx].rCfaMkvContentEncoding.eCfaMKvEncodingCompression) {
	case CFA_MKV_ENCODING_COMP_ZLIB:

		Stream.zalloc = (alloc_func)0;
		Stream.zfree = (free_func)0;
		Stream.opaque = (voidpf)0;

		i4Ret = inflateInit(&Stream);

		Stream.next_in = (Bytef *)(prCfaMkv->ptrMemAddr);
		Stream.avail_in = u8TxLen;

		do {
			i4Loop++;
			pucDestBuf = (unsigned char *)(prCfaMkv->auDecompBuf);
			Stream.next_out = (Bytef *)&pucDestBuf[(i4Loop - 1) * 1000];
			Stream.avail_out = 1000;
			i4Ret = inflate(&Stream, Z_NO_FLUSH);
			if ((i4Ret != Z_OK) && (i4Ret != Z_STREAM_END)) {
				DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("Zlib decompression failed: %d"), i4Ret);
				break;
			}
		} while ((Stream.avail_out == 0) && (Stream.avail_in != 0) && (i4Ret != Z_STREAM_END));

		prCfaMkv->zBufSize	= Stream.total_out;
		inflateEnd(&Stream);

		prCfaMkv->rCurBlock.fgDataReady = FALSE;
		prCfaMkv->rCurBlock.fgDurationReady = FALSE;
		prCfaMkv->eCurState = CFA_MKV_ST_TX_BLOCK;

		rSpInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;

		rSpInf.u8Len = (u64)prCfaMkv->zBufSize;
		rSpInf.u8Pts = prCfaMkv->rCurBlock.u8Pts;
		rSpInf.u8EndPts = prCfaMkv->rCurBlock.u8EndPts;

		/* for check buff->sp fifo error issue
		//Flush and Invalidate DCache for Cache Memory Data
		if (0 == _i4PSRIsNonCacheMem(prCfaMkv->auDecompBuf)) {
		HalFlushInvalidateDCache();
		}
		BIM_WAIT_WALE();	//Must do this for noncache and cache data
		*/

		mrResult = Spt4CfaBuf2SpFifoAUCtrl(pvSptHdl,
			prCfaMkv->auDecompBuf,
			&rSpInf,
			0);
		if (RET_DMX_OK != mrResult) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
			MkvFinishPrs(pvSptHdl, prCfaMkv);
		}
		break;

	case CFA_MKV_ENCODING_COMP_BZLIB:
	case CFA_MKV_ENCODING_COMP_LZO1X:
	case CFA_MKV_ENCODING_COMP_UNKNOWN:

		break;
	}

}
#endif


static void CfaMkvGetVirtualSequenceHeader(void *pvSptHdl, u64 u8TxLen,
										 CfaMkvInst *prCfaMkv)
{
	MRESULT mrResult = RET_DMX_OK;
	CFA_VIDEO_INFO_T rVidInf = {0};

	if (prCfaMkv->rAbnormalFlags.fgNoSeqHdr == TRUE) {
		prCfaMkv->eCurState = CFA_MKV_ST_HEADER_STRIPING;
		rVidInf.eVidType = prCfaMkv->rVidStmInfo.eVidCodec;
		rVidInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;
		rVidInf.u8Len = CFA_MKV_MP4_SEQ_HDR_LEN;
		if (prCfaMkv->rCurBlock.u8Pts == 0) {
			GenerateVirtualMP4SequenceHeader(prCfaMkv, (u8 *)(prCfaMkv->ptrMemAddr),
											 (u32)u8TxLen);
		}
		mrResult = Spt4CfaBuf2VFifoAUCtrl(pvSptHdl,
			prCfaMkv->pu1Mp4SeqHdr,
			&rVidInf);
		if (RET_DMX_OK != mrResult) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
			MkvFinishPrs(pvSptHdl, prCfaMkv);
			return;
		}
		prCfaMkv->rAbnormalFlags.fgNoSeqHdr = FALSE;
		prCfaMkv->rVidStmInfo.fgTxMpeg4VOLHeader = TRUE;
	}

	return;

}

/*-----------------------------------------------------------------------------
* Name: vCfaMkvTxDoneStCtrl
*
* Description:
*	   MKV CFA state control for transfer done
*	   This function will be called after a transfer is complete.
*
* Inputs: -
*	   [IN] uintptr_t of splitter
*	   [IN] Actual transferred data length.  Normally this value should be equal to the u4Len in
*			 the previous transfer issue, unless file end is hit.
*	   [IN] pointer to CfaMkvInst
*
* Outputs: -
*
* Returns: None
*-----------------------------------------------------------------------------*/
EXTERN void CfaMkvTxDoneStCtrl(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{

	prCfaMkv->u8Ca += u8TxLen;

	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA_MKV_ST_CTRL] u8Ca is 0x%llx[0x%lld], curstate is 0x%x\r\n"),
			prCfaMkv->u8Ca,
			prCfaMkv->u8Ca,
			prCfaMkv->eCurState);

	switch (prCfaMkv->eCurState) {
	case CFA_MKV_ST_IDLE:
		CfaMkvAnaStIdle(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_SC_ANA:
		CfaMkvAnaStartCode(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_SIZE_ANA:
		CfaMkvAnaSize(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_LACING_ANA:
		CfaMkvAnaLacing(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_BLOCK:
		CfaMkvProcBlock(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_TX_BLOCK:
		CfaMkvTxBlock(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_TX_SEQUENCE_INFO:
		CfaMkvTxSequenceInfo(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_TX_H264H265:
		CfaMkvTxAvcPayload(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_TX_SEQUENCE_RETURN:
		CfaMkvTxSequenceRet(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_TX_DIXV_HDR:
		CfaMkvTxDivxHdr(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_TX_DIXV_HDR_RET:
		CfaMkvTxDivxHdrRet(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_TX_H264H265_STARTCODE:
		CfaMkvTxH264StartCode(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_TX_H264H265_STARTCODE_RET:
		CfaMkvTxH264H265StartCodeRet(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_WVC1_TXMODE_ANA:
		CfaMkvWvc1Ana(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_WVC1_TX:
		CfaMkvWvc1TxDone(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_HEADER_STRIPING:
		CfaMkvTxHeader(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_AAC_TX_DONE:
		CfaMkvTxAacHeader(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_TX_RV_PARCIAL:
		CfaMkvTxRvParcialFrame(pvSptHdl, prCfaMkv);
		break;

	case CFA_MKV_ST_TX_RV_BLOCK:
		CfaMkvTxRvBlockDataEx(pvSptHdl, prCfaMkv, u8TxLen);
		break;

	case CFA_MKV_ST_TX_RV_BLOCKDATA:
		CfaMkvTxRvBlockData(pvSptHdl, prCfaMkv, u8TxLen);
		break;

#if CONFIG_CFA_MKV_SUPPORT_DRM
	case CFA_MKV_ST_PARSER_DRM_INFO:

		CfaMkvTxDrmInfo(pvSptHdl, u8TxLen, prCfaMkv);
		break;
#endif

#if CONFIG_CFA_MKV_SUPPORT_DECOMPRESSION
	case CFA_MKV_ST_DECOMPRESSION:
		CfaMkvDecompression(pvSptHdl, u8TxLen, prCfaMkv);
		break;
#endif

	case CFA_MKV_ST_READ_FIRST_FRAME:
		CfaMkvGetVirtualSequenceHeader(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_MPEG4_VOL_HEADER:
		CfaMkvTxMpeg4VolHeader(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_TX_AUD_AU_BYEND:
		CfaMkvTxAudAUByEnd(pvSptHdl, prCfaMkv);
		break;

	case CFA_MKV_ST_TX_FINISH:
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
		break;

	case CFA_MKV_ST_TX_VORBIS_ID:
		vCfaMkvSendVorbisID(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_TX_VORBIS_PRIV_OGG_HDR:
		vCfaMkvSendVorbisPrivOggHdr(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_TX_VORBIS_HDR:
		vCfaMkvSendVorbisComSetupHead(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_TX_OGG_HDR:
		vCfaMkvSendOggHdr(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	case CFA_MKV_ST_TX_VORBIS_DATA:
		vCfaMkvSendVorbisData(pvSptHdl, u8TxLen, prCfaMkv);
		break;

	default:
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] unknown tx state!\r\n"));
		MkvFinishPrs(pvSptHdl, prCfaMkv);
		break;
	}

}


/*-----------------------------------------------------------------------------
* Name: ucCfaMkvGetAudIndex
*
* Description:
*	   Get the index of audio info by audio stream ID.
*	   For multi-channel.
*
* Inputs:
*	   [IN] pointer to CfaMkvInst
*	   [IN] audio stream ID.
*
* Outputs:
*
* Returns: the index of audio info
*
*-----------------------------------------------------------------------------*/
EXTERN u8 CfaMkvGetAudIndex(const CfaMkvInst *prCfaMkv, u64 u8StrmID)
{
	u8 uIdx = 0;

	for (uIdx = 0; uIdx < prCfaMkv->u4AudNum; uIdx++) {
		if (uIdx >= MAX_NS_MKV_AUD) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] uIdx is %d\r\n"), uIdx);
			return DMX_INVALID_UINT8;
		}

		if (prCfaMkv->arAudStmInfo[uIdx].u8AudTrackNo == u8StrmID)
			return uIdx;
	}

	return DMX_INVALID_UINT8;
}


/*-----------------------------------------------------------------------------
* Name: ucCfaMkvGetSpIndex
*
* Description:
*	   Get the index of subtitle info by subtitle  track ID.
*	   For multi-subtitle.
*
* Inputs:
*
* Outputs:
*
* Returns: the index of subtitle info
*
*-----------------------------------------------------------------------------*/
EXTERN u8 CfaMkvGetSpIndex(const CfaMkvInst *prCfaMkv, u64 u8StrmID)
{
	u8 uIdx = 0;

	for (uIdx = 0; uIdx < prCfaMkv->u4SpNum; uIdx++) {
		if (uIdx >= MAX_NS_MKV_SP) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] uIdx is %d\r\n"), uIdx);
			return DMX_INVALID_UINT8;
		}

		if (prCfaMkv->arSpStmInfo[uIdx].u8SpTrackNo == u8StrmID)
			return uIdx;
	}

	return DMX_INVALID_UINT8;
}

#if PRINT_TONEXTSTATE_CALLBACK
EXTERN void ToNextState_I(void *pvSptHdl,
							CfaMkvInst *prCfaMkv,
							u64 u8ReadLen,
							ECfaMkvAnaState eCfaMkvNextState,
							char *function,
							u32 u4Line)
#else
EXTERN void ToNextState(void *pvSptHdl, CfaMkvInst *prCfaMkv, u64 u8ReadLen,
					   ECfaMkvAnaState eCfaMkvNextState)
#endif
{
	MRESULT mrResult = RET_DMX_OK;
	CFA_AUDIO_INFO_T rAudInf;
	if ((u8ReadLen == DMX_INVALID_UINT64) ||
        ((u8ReadLen & 0x8000000000000000LL) != 0)) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] readlen is invalid, line %d Send EOS\r\n"), DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
	}

#if PRINT_TONEXTSTATE_CALLBACK
	DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
	TEXT("[CFA_MKV_ST_CTRL] %s in line %d call To Next State\r\n"), function, u4Line);
#endif

	if (prCfaMkv->u8Ca >= prCfaMkv->rRange.u8VidEndOfst) {
		if ((prCfaMkv->i4AudAULenWithoutVid > 0) &&
			(prCfaMkv->i4AudAULenWithoutVid != CFA_MKV_AUD_AULEN_WITHOUT_VID)) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] line %d i4AudAULenWithoutVid is %d, compose AU\r\n"),
				__LINE__, prCfaMkv->i4AudAULenWithoutVid);

			mm_memset(&rAudInf, 0X00, sizeof(CFA_AUDIO_INFO_T));

			rAudInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;
			rAudInf.eAudType = prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec;
			rAudInf.u8FileOfst = prCfaMkv->rCurBlock.u8DataOfst;
			rAudInf.fgUnitStart = FALSE;
			rAudInf.fgUnitEnd = TRUE;
			rAudInf.fgAUCompleteByEnd = TRUE;
			rAudInf.u8Len = 0;
			rAudInf.u8TotalAULen = 0;
			mrResult = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rAudInf);
			prCfaMkv->eCurState = CFA_MKV_ST_TX_FINISH;
			return;
		}

		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
	} else {
		DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
			TEXT("[CFA_MKV_ST_CTRL]  u8LastCa= %llx ,u8Ca = %llx u8ReadLen = %llx, prCfaMkv->u8AvalOfst = %llx prCfaMkv->fgIfNeedRebuf = %d \r\n"),
			prCfaMkv->u8LastCa, prCfaMkv->u8Ca, u8ReadLen,prCfaMkv->u8AvalOfst, prCfaMkv->fgIfNeedRebuf);
		prCfaMkv->eCurState = eCfaMkvNextState;     
		prCfaMkv->u8TxLen = u8ReadLen;
		if(prCfaMkv->u8LastCa > prCfaMkv->u8Ca || (prCfaMkv->fgIfNeedRebuf)) {     
		}
		else if (((prCfaMkv->u8Ca + u8ReadLen) < prCfaMkv->u8AvalOfst)) {       
 			prCfaMkv->ptrMemAddr  = prCfaMkv->ptrLastReadMemAddr + (prCfaMkv->u8Ca - prCfaMkv->u8LastCa);
			prCfaMkv->fgNoNeedSyncPb = TRUE;
			prCfaMkv->u8LastCa = prCfaMkv->u8Ca;
			prCfaMkv->ptrLastReadMemAddr = prCfaMkv->ptrMemAddr;          
			return;
		}
		prCfaMkv->u8LastCa = prCfaMkv->u8Ca;
		prCfaMkv->fgRealSyncPb = TRUE;
		//prCfaMkv->fgCrossSlot = TRUE;
		prCfaMkv->u8AvalOfst = 0;
		prCfaMkv->u4AvailSize = 0;
		prCfaMkv->fgIfNeedRebuf = FALSE;
		mrResult = Spt4CfaPbb2SyncBufEx(pvSptHdl, prCfaMkv->u8Ca, u8ReadLen,(__u8 *)&(prCfaMkv->ptrMemAddr),&(prCfaMkv->u4AvailSize));
	#if PRINT_TONEXTSTATE_CALLBACK
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT, TEXT("[CFA_MKV_ST_CTRL] u8Ca is %lld[0x%llx], STATE is %d, u8ReadLen is %lld, available size is %d\r\n"),
		        prCfaMkv->u8Ca,prCfaMkv->u8Ca,eCfaMkvNextState,u8ReadLen,prCfaMkv->u4AvailSize);
	#endif
		prCfaMkv->ptrLastReadMemAddr = prCfaMkv->ptrMemAddr;
		prCfaMkv->u8AvalOfst = prCfaMkv->u8Ca + u8ReadLen + prCfaMkv->u4AvailSize;

		if (RET_DMX_OK != mrResult) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT, TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"),DMX_LINE_NO);
			MkvFinishPrs(pvSptHdl,prCfaMkv);
		}
	}
}

void vCfaMkvVorbisPageNumCal(CfaMkvInst *prCfaMkv)
{
	/*Calculate cur page *page number*/
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_VORBIS,
		TEXT("[CFA_MKV_ST_CTRL][VORBIS] %s line %d Enter\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	prCfaMkv->rCurBlock.u8VorbisPageNum = 0x1;

	/*Calculate The Vobis Header *page number*/
	prCfaMkv->u8VorbisHeadDataSize = prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].u8VorbisHeadSize;
	prCfaMkv->u8VorbisHeadDataOfst= prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].u8VorbisHeadOft;

	prCfaMkv->u4VorbisHeadPageNum = 1;

	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_VORBIS,
		TEXT("[CFA_MKV_ST_CTRL][VORBIS] VorbisHeadDataSize is %lld\r\n"),
		prCfaMkv->u8VorbisHeadDataSize);
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_VORBIS,
		TEXT("[CFA_MKV_ST_CTRL][VORBIS] %s line %d Exit\r\n"), DMX_FUNC_NAME, DMX_LINE_NO);
}

void vCfaMkvSendVorbisID(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	CFA_AUDIO_INFO_T rAudInf = {0};
	MRESULT mrResult = RET_DMX_OK;

	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_VORBIS,
		TEXT("[CFA_MKV_ST_CTRL][VORBIS] %s line %d Enter\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	rAudInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;
	rAudInf.eAudType = prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec;
	rAudInf.u8FileOfst = 0;
	rAudInf.u8Len = prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].u8VorbisIdSize;
	rAudInf.u8Pts = INVALID_TIMESTAMP;
	rAudInf.fgUnitStart = FALSE; /*Second part*/

	prCfaMkv->eCurState = CFA_MKV_ST_TX_VORBIS_PRIV_OGG_HDR;
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_VORBIS,
		TEXT("[CFA_MKV_ST_CTRL][VORBIS][TX]%s line %d header to buf len is %lld\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, rAudInf.u8Len);
	mrResult = Spt4CfaBuf2AFifo(pvSptHdl, prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].pucAudCodecVorbisID,
						rAudInf.u8Len,
						(u32)prCfaMkv->rCurBlock.u8TrackNum,
						prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec);
	if (RET_DMX_OK != mrResult) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
		return;
	}

}

void vCfaMkvSendVorbisIdHead(void *pvSptHdl, CfaMkvInst *prCfaMkv)
{
	u8 OggHead[30] = {0};
	CFA_AUDIO_INFO_T rAudInf = {0};
	MRESULT mrResult = RET_DMX_OK;

	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_VORBIS,
		TEXT("[CFA_MKV_ST_CTRL][VORBIS] %s line %d Enter\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	/*4 bytes[0-3]:Ogg packet id "OggS"*/
	OggHead[0] = (u8)'O';
	OggHead[1] = (u8)'g';
	OggHead[2] = (u8)'g';
	OggHead[3] = (u8)'S';
	/*1 byte[4]:stream structure version*/
	OggHead[4] = (u8)0x0;

	/*
		   1 byte[5]  packet flag:
						  bit 0: true if page continued
						  bit 1: true if first page
						  bit 2: true if last page
						  bit 3..7: reserved
		*/
	OggHead[5] = (u8)0x2; /* set  BOS.*/

	/*4 bytes[18-21]: Page nums. */
	OggHead[18] = (u8)(prCfaMkv->u4VorbisAuNs & (u8)0xff);
	OggHead[19] = (u8)(prCfaMkv->u4VorbisAuNs >> (u32)8) & (u8)0xff;
	OggHead[20] = (u8)(prCfaMkv->u4VorbisAuNs >> (u32)16) & (u8)0xff;
	OggHead[21] = (u8)(prCfaMkv->u4VorbisAuNs >> (u32)24) & (u8)0xff;

	prCfaMkv->u4VorbisAuNs++;

	/*1 bytes[26]: Number of segments */
	OggHead[26] = (u8)0x1;
	/*xx bytes[27..N]: Segment table (N<=255)*/
	OggHead[27] = (u8)prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].u8VorbisIdSize;

	dmx_memset((void *)(prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.auHeader),
			 0,
			 OGG_HEAD_SIZE);
	dmx_memcpy((void *)(prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.auHeader),
			 (void *)(&OggHead[0]),
			 28);
	prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.uHeaderLen = 28;

	prCfaMkv->eCurState = CFA_MKV_ST_TX_VORBIS_ID;

	rAudInf.fgUnitStart = TRUE; /*Fisrt part*/
	rAudInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;
	rAudInf.u8Len = prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.uHeaderLen;
	rAudInf.u8Pts = INVALID_TIMESTAMP;
	rAudInf.u8FileOfst = 0;

	rAudInf.eAudType = prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec;

	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_VORBIS,
		TEXT("[CFA_MKV_ST_CTRL][VORBIS][TX]%s line %d header to buf len is %lld\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, rAudInf.u8Len);
	mrResult = Spt4CfaBuf2AFifo(pvSptHdl,
					prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.auHeader,
					rAudInf.u8Len,
					(u32)prCfaMkv->rCurBlock.u8TrackNum,
					prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec);
	if (RET_DMX_OK != mrResult) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
		return;
	}

	/*Save this OGG header (Vorbis ID Hdr)*/
	prCfaMkv->u4OggHdrVorbisIDLength = rAudInf.u8Len;
	dmx_memcpy((void *)prCfaMkv->ucOggHdrVorbisID, (void *)(&OggHead[0]), prCfaMkv->u4OggHdrVorbisIDLength);
}

void vCfaMkvSendVorbisPrivOggHdr(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	u32 i = 0;
	u8  OggHead[27+256];
	u32 u4HeadLength = 0;
	u32 u4OggCommHdrLength = (u32)0x3A;
	u32 u4TmpSize = 0;

	bool   b_remain = FALSE;
	CFA_AUDIO_INFO_T rAudInf = {0};
	u32 u4Offset = 0;
	u32 u4CommHdrSegMentTable = 0;
	u32 u4CommHdrSegMentSize = 0;

    MRESULT mrResult = RET_DMX_OK;
	/*
		Calculate the vorbis common header length,Improved to use the direct length
	*/
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_VORBIS,
		TEXT("[CFA_MKV_ST_CTRL][VORBIS] %s line %d Enter\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	u4OggCommHdrLength = prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].u1VorbisCommHdrLenth;

	/*4 bytes[0-3]:Ogg packet id "OggS"*/
	OggHead[0] = ((u8)('O'));
	OggHead[1] = ((u8)('g'));
	OggHead[2] = ((u8)('g'));
	OggHead[3] = ((u8)('S'));
	/*1 byte[4]:stream structure version*/
	OggHead[4] = 0x0;

	prCfaMkv->u4VorbisHeadPageIndex++;

	/*
	   1 byte[5]  packet flag:
					  bit 0: true if page continued
					  bit 1: true if first page
					  bit 2: true if last page
					  bit 3..7: reserved
	*/
	OggHead[5] &= (u8)(~((u8)0xff));

	/*Calculate FIRST page data size !*/
	if (prCfaMkv->u8VorbisHeadDataSize > CFA_MKV_VORBIS_DATA_MAX_SIZE)
		prCfaMkv->u8VorbisHeadCurPageDataSize = CFA_MKV_VORBIS_DATA_MAX_SIZE;
	else
		prCfaMkv->u8VorbisHeadCurPageDataSize = prCfaMkv->u8VorbisHeadDataSize;


	if (prCfaMkv->u4VorbisHeadPageIndex < prCfaMkv->u4VorbisHeadPageNum) {
		DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_VORBIS,
			TEXT("[CFA_MKV_ST_CTRL][VORBIS] WARNING-----\r\n"));
		OggHead[5] &= (u8)(~((u8)0xff));
		OggHead[5] = 0x1; /* set  1st bit, this is the same logical stream as the previous page.*/
	}

	if (prCfaMkv->rCurBlock.u8DataSize + prCfaMkv->rCurBlock.u8DataOfst >= prCfaMkv->rRange.u8AudEndOfst) {
		OggHead[5] &= (u8)(~((u8)0xff));
		OggHead[5] = (u8)0x8; /* set  EOS.*/
	}

	/*4 bytes[18-21]: Page nums. */
	OggHead[18] = (u8)(prCfaMkv->u4VorbisAuNs & (u32)0xff);
	OggHead[19] = (u8)((prCfaMkv->u4VorbisAuNs >> (u32)8) & (u32)0xff);
	OggHead[20] = (u8)((prCfaMkv->u4VorbisAuNs >> (u32)16) & (u32)0xff);
	OggHead[21] = (u8)((prCfaMkv->u4VorbisAuNs >> (u32)24) & (u32)0xff);

	prCfaMkv->u4VorbisAuNs++;

	/*set bit 26*/
	if (CFA_MKV_VORBIS_DATA_MAX_SIZE == prCfaMkv->u8VorbisHeadCurPageDataSize)
		OggHead[26] = 255;
	else {
		u4TmpSize = (u32)(prCfaMkv->u8VorbisHeadCurPageDataSize & (u64)0xffffffff);
		u4TmpSize -= prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].u8VorbisCommHdrSize;

		OggHead[26] = (u8)(u4TmpSize / (u32)255);

		if (0 == (u4TmpSize % ((u32)255)))
			b_remain = FALSE;
		else {
			b_remain = TRUE;
			OggHead[26] += 1;
		}
	}

	do {
		u4Offset = 27;
		u4CommHdrSegMentSize = (prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].u8VorbisCommHdrSize & 0xffffffff);

		while (u4CommHdrSegMentSize > 255) {
			OggHead[u4Offset++] = 255;
			u4CommHdrSegMentSize -= 255;
			u4CommHdrSegMentTable++;
		}
		OggHead[u4Offset] = u4CommHdrSegMentSize;
		u4CommHdrSegMentTable++;
	} while(0);

    //OggHead[27] = prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].u8VorbisCommHdrSize;

	/*xx bytes[27..N]: Segment table (N<=255)*/
	if (b_remain) {
		for (i = 1; i < (u32)OggHead[26]; i++)
			OggHead[u4Offset+i] = (u8)255;

		OggHead[u4Offset+i] = (u8)(u4TmpSize - ((u32)OggHead[26] - 1)*255) ;/*last*/
	} else {
		for (i = 1; i <= (u32)OggHead[26]; i++)
			OggHead[u4Offset+i] = 255;

	}
	OggHead[26] += u4CommHdrSegMentTable;
	u4HeadLength = 27 + (u32)OggHead[26];

	/*TO-DO: Verfy u4HeadLength < MKV_HEADER_STRPING_LENGTH*/
	dmx_memset((void *)(prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.auHeader),
			 0,
			 OGG_HEAD_SIZE);
	dmx_memcpy((void *)(prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.auHeader),
			 (void *)(&OggHead[0]),
			 u4HeadLength);
	prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.uHeaderLen = u4HeadLength;
	prCfaMkv->u8VorbisHeadCurPageTotalAULen = prCfaMkv->u8VorbisHeadCurPageDataSize + u4HeadLength;

	prCfaMkv->eCurState = CFA_MKV_ST_TX_VORBIS_HDR;

	rAudInf.fgUnitStart = TRUE; /*Fisrt part*/
	rAudInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;
	rAudInf.u8Len = prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.uHeaderLen;
	rAudInf.u8Pts = INVALID_TIMESTAMP;
	rAudInf.u8FileOfst = 0 & (u64)0xff;

	rAudInf.eAudType = prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec;

	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_VORBIS,
		TEXT("[CFA_MKV_ST_CTRL][VORBIS][TX]%s line %d header to buf len is %lld\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, rAudInf.u8Len);
	mrResult = Spt4CfaBuf2AFifo(pvSptHdl,
		prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.auHeader,
		rAudInf.u8Len,
		(u32)prCfaMkv->rCurBlock.u8TrackNum,
		prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec);
	if (RET_DMX_OK != mrResult) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
		return;
	}

	/*Save this OGG header (Vorbis Common Hdr, Setup Hdr)*/
	dmx_memcpy((void *)prCfaMkv->ucOggHdrVorbisComSetup, (void *)(&OggHead[0]), u4HeadLength);
	prCfaMkv->u4OggHdrVorbisComSetupLength = u4HeadLength;
}

void vCfaMkvSetOggHdr(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	u32 i = 0;
	u32 u4HeadLength = 0;
	bool   b_remain = FALSE;
	u32 u4TmpSize = 0;

	u8 OggHead[27+256] = {0};

	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_VORBIS,
		TEXT("[CFA_MKV_ST_CTRL][VORBIS] %s line %d Enter\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	/*4 bytes[0-3]:Ogg packet id "OggS"*/
	OggHead[0] = (u8)('O');
	OggHead[1] = (u8)('g');
	OggHead[2] = (u8)('g');
	OggHead[3] = (u8)('S');
	/*1 byte[4]:stream structure version*/
	OggHead[4] = 0x0;

	prCfaMkv->rCurBlock.u4VorbisPageIndex++;

	/*Support lacing, if it's lacing ,we have to tranfer the data according to the frame*/
	if ((prCfaMkv->rCurBlock.eLacingType != CFA_MKV_NO_LACING) &&
		(prCfaMkv->rCurBlock.i4CurFrameNum <= prCfaMkv->rCurBlock.i4FrameNum)) {
		prCfaMkv->rCurBlock.u8VorbisCurPageDataSize =
			prCfaMkv->rCurBlock.au4FrameSize[prCfaMkv->rCurBlock.i4CurFrameNum];
	} else {
		if ((prCfaMkv->rCurBlock.u8DataSize) > CFA_MKV_VORBIS_DATA_MAX_SIZE)
			prCfaMkv->rCurBlock.u8VorbisCurPageDataSize = CFA_MKV_VORBIS_DATA_MAX_SIZE;
		else
			prCfaMkv->rCurBlock.u8VorbisCurPageDataSize = prCfaMkv->rCurBlock.u8DataSize;
	}

	/*
	   1 byte[5]  packet flag:
					  bit 0: true if page continued
					  bit 1: true if first page
					  bit 2: true if last page
					  bit 3..7: reserved
	*/
	if (prCfaMkv->rCurBlock.u4VorbisPageIndex < prCfaMkv->rCurBlock.u8VorbisPageNum) {
		OggHead[5] &= (u8)(~((u8)0xff));
		OggHead[5] = 0x1; /*set  1st bit, this is the same logical stream as the previous page.*/
	}
	if (prCfaMkv->rCurBlock.u8DataSize + prCfaMkv->rCurBlock.u8DataOfst >= prCfaMkv->rRange.u8AudEndOfst) {
		OggHead[5] &= (u8)(~((u8)0xff));
		OggHead[5] = 0x8; /* set  EOS.*/
	}

	/*4 bytes[18-21]: Page nums. */
	OggHead[18] = (u8)(prCfaMkv->u4VorbisAuNs & (u32)0xff);
	OggHead[19] = (u8)((prCfaMkv->u4VorbisAuNs >> (u32)8) & (u32)0xff);
	OggHead[20] = (u8)((prCfaMkv->u4VorbisAuNs >> (u32)16) & (u32)0xff);
	OggHead[21] = (u8)((prCfaMkv->u4VorbisAuNs >> (u32)24) & (u32)0xff);

	prCfaMkv->u4VorbisAuNs++;

	/*1 bytes[26]: Number of segments */
	if (CFA_MKV_VORBIS_DATA_MAX_SIZE == prCfaMkv->rCurBlock.u8VorbisCurPageDataSize)
		OggHead[26] = 255;
	else {
		OggHead[26] = (u8)(prCfaMkv->rCurBlock.u8VorbisCurPageDataSize / 255);

		u4TmpSize = (u32)(prCfaMkv->rCurBlock.u8VorbisCurPageDataSize & (u64)0xffffffff);

		if (0 == (u4TmpSize % 255))
			b_remain = FALSE;
		else {
			b_remain = TRUE;
			OggHead[26] += 1;
		}
	}

	u4HeadLength = 27 + OggHead[26];

	/*xx bytes[27..N]: Segment table (N<=255)*/
	if (b_remain) {
		for (i = 1; i < OggHead[26]; i++)
			OggHead[26+i] = 255;

		OggHead[26+i] = u4TmpSize - (OggHead[26] - 1)*255 ;/*last*/
	} else {
		for (i = 1; i <= OggHead[26]; i++)
			OggHead[26+i] = 255;

	}

	dmx_memset((void *)(prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.auHeader),
			 0,
			 OGG_HEAD_SIZE);
	dmx_memcpy((void *)(prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.auHeader),
			 (void *)(&OggHead[0]),
			 u4HeadLength);
	prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.uHeaderLen = u4HeadLength;
	prCfaMkv->rCurBlock.u8VorbisCurPageTotalAULen = prCfaMkv->rCurBlock.u8VorbisCurPageDataSize + u4HeadLength;

	prCfaMkv->eCurState = CFA_MKV_ST_TX_VORBIS_DATA;
}

void vCfaMkvSendOggHdr(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	CFA_AUDIO_INFO_T rAudInf = {0};
	MRESULT mrResult = RET_DMX_OK;

	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_VORBIS,
		TEXT("[CFA_MKV_ST_CTRL][VORBIS] %s line %d Enter\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	if ((prCfaMkv->rCurBlock.eLacingType != CFA_MKV_NO_LACING) &&
		(prCfaMkv->rCurBlock.i4CurFrameNum == 0) &&
		(prCfaMkv->rCurBlock.i4CurFrameNum <= prCfaMkv->rCurBlock.i4FrameNum)) {
		prCfaMkv->rCurBlock.u8NextTxOfst = prCfaMkv->rCurBlock.u8DataOfst;
	}

	vCfaMkvSetOggHdr(pvSptHdl, u8TxLen, prCfaMkv);

	rAudInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;
	rAudInf.u8Len = prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.uHeaderLen;
	rAudInf.u8Pts = INVALID_TIMESTAMP;
	rAudInf.fgUnitStart = TRUE; /*Fisrt part*/

	rAudInf.eAudType = prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec;

#if MKV_DUMP_FILE
	loff_t t_cur_pos = 0;

	if (NULL == fd) {
		fd = filp_open(filename , O_RDWR | O_CREAT | O_APPEND, 644);
		if (NULL == fd) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] open file fail\r\n"));
		} else {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] open file success\r\n"));
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] set file pointer success--1--, write file\r\n"));
			if (rAudInf.u8Len != 0) {
				char firsttemp[16] = {0XAA, 0XAA, 0XAA, 0XAA, 0XAA, 0XAA, 0XAA,
					0XAA, 0XAA, 0XAA, 0XAA, 0XAA, 0XAA, 0XAA, 0XAA, 0XAA};
				/*fwrite(firsttemp, sizeof(char), 16, fd);*/
				vfs_write(fd, firsttemp, 16, &t_cur_pos);

				vfs_write(fd,
				(char *)(prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.auHeader),
				(int)rAudInf.u8Len, &t_cur_pos);
			}
		}
	} else {
		char temp[16] = {0XAA, 0XAA, 0XAA, 0XAA, 0XAA, 0XAA, 0XAA, 0XAA, 0XAA,
						0XAA, 0XAA, 0XAA, 0XAA, 0XAA, 0XAA, 0XAA};
		vfs_write(fd, temp, 16, &t_cur_pos);
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] set file pointer success, write file\r\n"));
		vfs_write(fd,
			(char *)(prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.auHeader),
			(int)rAudInf.u8Len, &t_cur_pos);
	}
#endif
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_VORBIS,
		TEXT("[CFA_MKV_ST_CTRL][VORBIS][TX]%s line %d header to buf len is %lld\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, rAudInf.u8Len);
	mrResult = Spt4CfaBuf2AFifo(pvSptHdl,
		prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].rCfaMkvHeader.auHeader,
		rAudInf.u8Len,
		(u32)prCfaMkv->rCurBlock.u8TrackNum,
		prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec);
	if (RET_DMX_OK != mrResult) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
	}
}

void vCfaMkvSendVorbisData(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	CFA_AUDIO_INFO_T rAudInf = {0};
	MRESULT mrResult = RET_DMX_OK;

	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_VORBIS,
		TEXT("[CFA_MKV_ST_CTRL][VORBIS] %s line %d Enter, eLacingType is %d\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prCfaMkv->rCurBlock.eLacingType);

	if ((prCfaMkv->rCurBlock.eLacingType != CFA_MKV_NO_LACING) &&
		(prCfaMkv->rCurBlock.i4CurFrameNum <= prCfaMkv->rCurBlock.i4FrameNum)) {
		rAudInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;
		rAudInf.eAudType = prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec;

		rAudInf.u8FileOfst = prCfaMkv->rCurBlock.u8NextTxOfst;
		rAudInf.u8Len = prCfaMkv->rCurBlock.au4FrameSize[prCfaMkv->rCurBlock.i4CurFrameNum];
		if (prCfaMkv->rCurBlock.i4CurFrameNum == 0)
			rAudInf.u8Pts = prCfaMkv->rCurBlock.u8Pts;
		else
			rAudInf.u8Pts = INVALID_TIMESTAMP;

		rAudInf.fgUnitStart = FALSE; /*Second part*/

		prCfaMkv->rCurBlock.i4CurFrameNum++;
		prCfaMkv->rCurBlock.u8NextTxOfst += rAudInf.u8Len;

		if (prCfaMkv->rCurBlock.i4CurFrameNum > prCfaMkv->rCurBlock.i4FrameNum)
			prCfaMkv->eCurState = CFA_MKV_ST_TX_BLOCK;
		else
			prCfaMkv->eCurState = CFA_MKV_ST_TX_OGG_HDR;

	} else {
		rAudInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;
		rAudInf.eAudType = prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec;

		rAudInf.u8FileOfst = prCfaMkv->rCurBlock.u8DataOfst;
		rAudInf.u8Len = prCfaMkv->rCurBlock.u8VorbisCurPageDataSize;
		rAudInf.u8Pts = prCfaMkv->rCurBlock.u8Pts;
		rAudInf.fgUnitStart = FALSE; /*Second part*/

		prCfaMkv->eCurState = CFA_MKV_ST_TX_BLOCK;
	}

	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_VORBIS,
		TEXT("[CFA_MKV_ST_CTRL][VORBIS][TX] CurPageTotalAULen 0x%llx[%lld],")
		TEXT("CurPageDataSize is 0x%llx[%lld]\r\n"),
		prCfaMkv->rCurBlock.u8VorbisCurPageTotalAULen, prCfaMkv->rCurBlock.u8VorbisCurPageTotalAULen,
		rAudInf.u8Len, rAudInf.u8Len);
	rAudInf.u8TotalAULen = prCfaMkv->rCurBlock.u8VorbisCurPageTotalAULen;
	if (prCfaMkv->rRange.u8AudEndOfst <= rAudInf.u8FileOfst ) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
		return;
	}
	if (prCfaMkv->rRange.u8AudEndOfst < (rAudInf.u8FileOfst + rAudInf.u8Len)) {
		rAudInf.u8Len = prCfaMkv->rRange.u8AudEndOfst - rAudInf.u8FileOfst;
	}
	mrResult = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rAudInf);
	if (RET_DMX_OK != mrResult) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
		MkvFinishPrs(pvSptHdl, prCfaMkv);
	}
}

void vCfaMkvSendVorbisComSetupHead(void *pvSptHdl, u64 u8TxLen, CfaMkvInst *prCfaMkv)
{
	u8 *pVorbisPageTmp = NULL;
	u64 u8VorbisPrivDataLen = 0;
	MRESULT mrResult = RET_DMX_OK;
	
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_VORBIS,
		TEXT("[CFA_MKV_ST_CTRL][VORBIS] %s line %d Enter\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	if (0 == prCfaMkv->u8VorbisHeadCurPageDataSize) {

		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV_ST_CTRL] vorbis header page data size is zero!\r\n"));
		return;
	} else {
		CFA_AUDIO_INFO_T rAudInf = {0};

		rAudInf.u4PrsStrmId = (u32)prCfaMkv->rCurBlock.u8TrackNum;
		rAudInf.eAudType = prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec;

		rAudInf.u8FileOfst = prCfaMkv->u8VorbisHeadDataOfst;
		rAudInf.u8Len = prCfaMkv->u8VorbisHeadCurPageDataSize;
		u8VorbisPrivDataLen = rAudInf.u8Len;

		rAudInf.u8Pts = INVALID_TIMESTAMP;
		rAudInf.fgUnitStart = FALSE; /*Second part*/

		/*Update Block data size and ofst*/
		prCfaMkv->u8VorbisHeadDataSize -= prCfaMkv->u8VorbisHeadCurPageDataSize;
		prCfaMkv->u8VorbisHeadDataOfst += prCfaMkv->u8VorbisHeadCurPageDataSize;

		/*Calculate FIRST page data size !*/
		if ((prCfaMkv->u8VorbisHeadDataSize) > CFA_MKV_VORBIS_DATA_MAX_SIZE) {
			prCfaMkv->u8VorbisHeadCurPageDataSize = CFA_MKV_VORBIS_DATA_MAX_SIZE;
			prCfaMkv->eCurState = CFA_MKV_ST_TX_VORBIS_PRIV_OGG_HDR;
		} else {
			prCfaMkv->u8VorbisHeadCurPageDataSize = prCfaMkv->u8VorbisHeadDataSize;
			prCfaMkv->eCurState = CFA_MKV_ST_TX_OGG_HDR;
		}
		DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_VORBIS,
			TEXT("[CFA_MKV_ST_CTRL][VORBIS][TX]%s line %d header to buf len is %lld\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, rAudInf.u8Len);
		mrResult = Spt4CfaBuf2AFifo(pvSptHdl,
				prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].pucAudCodecPrivData,
				rAudInf.u8Len,
				(u32)prCfaMkv->rCurBlock.u8TrackNum,
				prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].eAudCodec);
		if (RET_DMX_OK != mrResult) {
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
			MkvFinishPrs(pvSptHdl, prCfaMkv);
			return;
		}

		/*
			* Save the total Vorbis head page data for resume play
			*/
		prCfaMkv->u8TotalVorbisHeadPageSize =
			prCfaMkv->u8VorbisHeadCurPageTotalAULen + CFA_MKV_VORBIS_ID_TOTAL_LENGTH;

		if (prCfaMkv->u8TotalVorbisHeadPageMemSize < prCfaMkv->u8TotalVorbisHeadPageSize) {
			if (NULL != prCfaMkv->pTotalVorbisHeadPage) {
				DMX_FreeHwMemory(prCfaMkv->pTotalVorbisHeadPage);
				prCfaMkv->pTotalVorbisHeadPage = NULL;
			}
			prCfaMkv->u8TotalVorbisHeadPageMemSize = prCfaMkv->u8TotalVorbisHeadPageSize;
			DMX_NewHwMemory(prCfaMkv->u8TotalVorbisHeadPageMemSize, prCfaMkv->pTotalVorbisHeadPage);
			if (NULL == prCfaMkv->pTotalVorbisHeadPage) {
				DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV_ST_CTRL] line %d Send EOS\r\n"), DMX_LINE_NO);
				MkvFinishPrs(pvSptHdl, prCfaMkv);
				return;
			}
		}

		pVorbisPageTmp = prCfaMkv->pTotalVorbisHeadPage;

		/*1st OGG Page*/
		dmx_memcpy((void *)pVorbisPageTmp, prCfaMkv->ucOggHdrVorbisID, prCfaMkv->u4OggHdrVorbisIDLength);
		pVorbisPageTmp += prCfaMkv->u4OggHdrVorbisIDLength;

		dmx_memcpy((void *) pVorbisPageTmp, prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].pucAudCodecVorbisID, prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].u8VorbisIdSize);
		pVorbisPageTmp += prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].u8VorbisIdSize;

		/*2nd OGG Page*/
		dmx_memcpy((void *)pVorbisPageTmp,
			prCfaMkv->ucOggHdrVorbisComSetup, prCfaMkv->u4OggHdrVorbisComSetupLength);
		pVorbisPageTmp += prCfaMkv->u4OggHdrVorbisComSetupLength;

		dmx_memcpy((void *) pVorbisPageTmp,
			prCfaMkv->arAudStmInfo[prCfaMkv->rCurBlock.ucStrmIdx].pucAudCodecPrivData,
			u8VorbisPrivDataLen);
		pVorbisPageTmp += u8VorbisPrivDataLen;
		/*Save end*/
	}
}
