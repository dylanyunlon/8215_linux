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
#include "x_ver.h"
#include "mmisc.h"
#include "generated/atc_project.h"

#define MKV_CFA_MOD  _T("CFA MKV")
#define MKV_CFA_MM	 1
#define MKV_CFA_mm	 1
#define MKV_CFA_rev  1

/*-----------------------------------------------------------------------------
macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
function prototype
-----------------------------------------------------------------------------*/

static MRESULT CfaMkvInitPara(CfaMkvInst * prCfaMkv);
static MRESULT CfaMkvUninit(void *pvSptHdl, void *pvCfaPrivData);
#ifdef CONFIG_COMPAT
#include <linux/compat.h>

typedef struct {
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKStart;
#endif

	__u64 u8TargetTimeCode;	/*time code of the target position */
	__u64 u8ClusterStartAddr;

	/* video */
	__u64 u8VidTimeCode;	/*time code of the last keyframe position */

	__u32 u4VidBlockNo;
	__u64 u8VidStartOfst;	/*start offset of first cluster */
	__u64 u8VidEndOfst;	/*end offset */

	/* audio */
	__u32 u4AudBlockNo;
	__u64 u8AudStartOfst;	/*start offset of first cluster */
	__u64 u8AudEndOfst;	/*end offset */

	/* subtitle */
	__u32 u4SubBlockNo;	/*block number of the cluster */
	__u64 u8SubStartOfst;	/* start offset of first cluster */
	__u64 u8SubEndOfst;	/* ending offset */
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKEnd;
#endif
} CfaMkvRange_T32;

typedef struct {
	__u32 u4PPSNum;
	__u32 u4SPSNum;
	__u64 u8SPSDataOffset;
	__u64 u8PPSDataOffset;
	__u32 u4SPSDataLen;
	__u32 u4PPSDataLen;
	bool fgHasSPSStartCode;
} AVC_SPS_PPS_INFO_T32;

typedef struct {
	__u32 u4SamplingFrequency;
	__u32 u4OutputSamplingFrequency;
	__u32 u4Channels;
	__u32 u4BitDepth;
	CfaMkvAacHeader_T rCfaMkvAacHeader;

} CfaMkvAudSpecialInfo_T32;


typedef struct {
	AVCODECID_T eVidCodec;
	VCODECVERSION_T eRVVersion;
	__u64 u8VidTrackNo;
	__u64 u8VidTrackUID;
	bool fgTimeCodeScaleEn; /*when timecodescale exist,set TRUE,else set FALSE */
	/*if this element exist, block timecode should be multiplied by this value */
	__u64 u8TrackTimeCodeScale;
	__u64 u8CodecPrivOfst;  /*offset of CodecPrivate in file */
	__u64 u8CodecPrivLen;   /*length of CodecPrivate element */
	compat_caddr_t pucCodecPrivBuf;
	__u32 u4DivxHdrLen;
	compat_caddr_t pucDivxHdrBuf;
	__u32 u4NalSizeLen;
	CfaMkvHeaderStriping_T rCfaMkvHeaderStrip;
	CfaMkvContentEncoding_T rCfaMkvContentEncoding;
	AVC_SPS_PPS_INFO_T32 rSPSPPSInfo;

	__u32 u4IndexTableAddr;
	__u64 u8IndexTableIndex;

	__u8 pucMpeg4Header[MKV_MPEG4_HEADER_LEN];
	__u32 u4Mpeg4HeaderLen;

} CfaMkvVidInfo_T32;

typedef struct {
	AVCODECID_T eCFAAudCodec;
	__u64 u8AudTrackNo;	/*identification number of the track */
	__u64 u8AudTrackUID;	/*a unique identificator of the track */
	bool fgTimeCodeScaleEn;	/*when timecodescale exist,set TRUE,else set FALSE */
	/*if this element exist, block timecode should be multiplied by this value */
	__u64 u8TrackTimeCodeScale;
	__u64 u8CodecPrivOfst;	/*offset of CodecPrivate in file */
	__u64 u8CodecPrivLen;	/*length of CodecPrivate element */
	compat_uptr_t ptrVorbisPrivDataMem;
	__u8 u1VorbisCommHdrLen;
	CfaMkvHeaderStriping_T rCfaMkvHeaderStrip;
	CfaMkvAudSpecialInfo_T32 rCfaMkvAudSpecialInfo;
	CfaMkvContentEncoding_T rCfaMkvContentEncoding;

} CfaMkvAudInfo_T32;

typedef struct {
	/*all these information get from TrackEntry */
	__u64 u8SpTrackNo;	/*identification number of the track */
	__u64 u8SpTrackUID;	/*a unique identificator of the track */
	bool fgTimeCodeScaleEn;	/*when timecodescale exist,set TRUE,else set FALSE */
	/*if this element exist, block timecode should be multiplied by this value */
	__u64 u8TrackTimeCodeScale;
	__u64 u8CodecPrivOfst;	/*offset of CodecPrivate in file */
	__u64 u8CodecPrivLen;	/*length of CodecPrivate element */
	AVCODECID_T eCfaMkvSpType;
	CfaMkvContentEncoding_T rCfaMkvContentEncoding;

} CfaMkvSpInfo_T32;

typedef struct {
	/*TimeCodeScale */
	__u64 u8TimeCodeScale;
	bool fgHasVideo;

	/* Video info */
	CfaMkvVidInfo_T32 rCfaMkvVidInfo;

	/* Audio info */
	__u32 u4AudioTrackNs;	/*track numbers of      Audio Track */
	CfaMkvAudInfo_T32 arCfaMkvAudInfo[MAX_NS_MKV_AUD];

	/* subtitle info */
	__u32 u4SubTrackNs;	/*track numbers of      subtitle Track */
	CfaMkvSpInfo_T32 arCfaMkvSpInfo[MAX_NS_MKV_SP];

	/*all of abnormal case flag are stored at this struct */
	CfaMkvAbnormalFlag_T rCfaMkvAbnormalFlags;

#if CONFIG_CFA_MKV_SUPPORT_DRM
	CfaMkvAudCryptInfo rAudCryptInfo;
#endif

} CfaMkvConfigInfo_T32;

typedef struct {
	CfaMkvRange_T32 rCfaRangeInfo;
	__u32 u4RWUnitAULen;
} CfaMkvKeyFrameRange_T32;


static long CfaMkvCompatRangeInfo(CfaMkvRange_T __user *usr_ptr,
  CfaMkvRange_T32 __user *usr_ptr32)
{

#ifdef MM_ATC_CHECK
	if (copy_in_user(&(usr_ptr->u4MMATECHKStart), &(usr_ptr32->u4MMATECHKStart), sizeof(__u32)))
		return -EFAULT;
#endif
	if (copy_in_user(&(usr_ptr->u8TargetTimeCode), &(usr_ptr32->u8TargetTimeCode), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8ClusterStartAddr), &(usr_ptr32->u8ClusterStartAddr), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8VidTimeCode), &(usr_ptr32->u8VidTimeCode), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u4VidBlockNo), &(usr_ptr32->u4VidBlockNo), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8VidStartOfst), &(usr_ptr32->u8VidStartOfst), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8VidEndOfst), &(usr_ptr32->u8VidEndOfst),
	sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4AudBlockNo), &(usr_ptr32->u4AudBlockNo),
	sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8AudStartOfst), &(usr_ptr32->u8AudStartOfst),
	sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8AudEndOfst), &(usr_ptr32->u8AudEndOfst), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4SubBlockNo), &(usr_ptr32->u4SubBlockNo), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8SubStartOfst), &(usr_ptr32->u8SubStartOfst),
		sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8SubEndOfst), &(usr_ptr32->u8SubEndOfst), sizeof(__u64)))
		return -EFAULT;
#ifdef MM_ATC_CHECK
	if (copy_in_user(&(usr_ptr->u4MMATECHKEnd), &(usr_ptr32->u4MMATECHKEnd), sizeof(__u32)))
		return -EFAULT;
#endif

	return 0;
}

static long CfaMkvCompatRange(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaMkvRange_T __user *usr_ptr = NULL;
	CfaMkvRange_T32 __user *usr_ptr32 = (CfaMkvRange_T32 __user *)prInfo->usr_ptr32;
	long ret = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaMkvRange_T32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	usr_ptr = (CfaMkvRange_T *)compat_alloc_user_space(sizeof(CfaMkvRange_T));

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
		TEXT("%s line %d fail in alloc compat user space.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}

	mm_memset(usr_ptr, 0, sizeof(CfaMkvRange_T));
	
	ret = CfaMkvCompatRangeInfo(usr_ptr, usr_ptr32);
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
		TEXT("%s line %d fail in CfaMkvCompatRangeInfo.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}

	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaMkvRange_T);

	return 0;
}

static long CfaMkvCompatAvcSPSPPSInfo(AVC_SPS_PPS_INFO_T __user *usr_ptr,
  AVC_SPS_PPS_INFO_T32 __user *usr_ptr32)
{
	if (copy_from_user(&(usr_ptr->u4PPSNum), &(usr_ptr32->u4PPSNum),
		sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4SPSNum), &(usr_ptr32->u4SPSNum),
		sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8SPSDataOffset), &(usr_ptr32->u8SPSDataOffset),
		sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8PPSDataOffset), &(usr_ptr32->u8PPSDataOffset),
		sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4SPSDataLen), &(usr_ptr32->u4SPSDataLen),
		sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4PPSDataLen), &(usr_ptr32->u4PPSDataLen),
		sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->fgHasSPSStartCode), &(usr_ptr32->fgHasSPSStartCode),
		sizeof(bool)))
		return -EFAULT;
	return 0;
}

static long CfaMkvCompatSpInfo(CfaMkvSpInfo_T __user *usr_ptr,
  CfaMkvSpInfo_T32 __user *usr_ptr32)
{
	if (copy_from_user(&(usr_ptr->u8SpTrackNo), &(usr_ptr32->u8SpTrackNo),
		sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8SpTrackUID), &(usr_ptr32->u8SpTrackUID),
		sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->fgTimeCodeScaleEn), &(usr_ptr32->fgTimeCodeScaleEn),
		sizeof(bool)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8TrackTimeCodeScale), &(usr_ptr32->u8TrackTimeCodeScale),
		sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8CodecPrivOfst), &(usr_ptr32->u8CodecPrivOfst),
		sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8CodecPrivLen), &(usr_ptr32->u8CodecPrivLen),
		sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->eCfaMkvSpType), &(usr_ptr32->eCfaMkvSpType),
		sizeof(AVCODECID_T)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->rCfaMkvContentEncoding), &(usr_ptr32->rCfaMkvContentEncoding),
		sizeof(CfaMkvContentEncoding_T)))
		return -EFAULT;

	return 0;
}

static long CfaMkvCompatVidInfo(CfaMkvVidInfo_T __user *usr_ptr,
  CfaMkvVidInfo_T32 __user *usr_ptr32, __u8 **ppu1NextBufAddr, __u32 *pu4Sz, __u32 u4TotalSz)
{
	compat_caddr_t compatSeqHdr = 0;
	long ret = 0;

	if (copy_from_user(&(usr_ptr->eVidCodec), &(usr_ptr32->eVidCodec),
		sizeof(AVCODECID_T)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->eRVVersion), &(usr_ptr32->eRVVersion),
		sizeof(VCODECVERSION_T)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8VidTrackNo), &(usr_ptr32->u8VidTrackNo),
		sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8VidTrackUID), &(usr_ptr32->u8VidTrackUID),
		sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->fgTimeCodeScaleEn), &(usr_ptr32->fgTimeCodeScaleEn),
		sizeof(bool)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8TrackTimeCodeScale), &(usr_ptr32->u8TrackTimeCodeScale),
		sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8CodecPrivOfst), &(usr_ptr32->u8CodecPrivOfst),
		sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8CodecPrivLen), &(usr_ptr32->u8CodecPrivLen),
		sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4NalSizeLen), &(usr_ptr32->u4NalSizeLen),
		sizeof(__u32)))
		return -EFAULT;

	if ((0 < usr_ptr32->u8CodecPrivLen) && (0 == usr_ptr32->pucCodecPrivBuf)) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d fail for no aac header, but header len(%d) > 0.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr->u8CodecPrivLen);
		return -EINVAL;
	}

	if (0 != usr_ptr32->pucCodecPrivBuf) {
		__u8 *pucCodecPrivBuf = NULL;

		usr_ptr->pucCodecPrivBuf =  *ppu1NextBufAddr;

		if (NULL == usr_ptr->pucCodecPrivBuf) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
			return -ENOMEM;
		}
		mm_memset(usr_ptr->pucCodecPrivBuf, 0, sizeof(__u8) * usr_ptr->u8CodecPrivLen);
		*ppu1NextBufAddr +=
			CFA_ALIGN_SZ(sizeof(__u8) * usr_ptr->u8CodecPrivLen, sizeof(uintptr_t));
		*pu4Sz += CFA_ALIGN_SZ(sizeof(__u8) * usr_ptr->u8CodecPrivLen, sizeof(uintptr_t));
		if (*pu4Sz > u4TotalSz)
		{
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d size is large than total size.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
			return -ENOMEM;
		}
		if (get_user(compatSeqHdr, &(usr_ptr32->pucCodecPrivBuf)))
			return -EFAULT;
		if (0 == compatSeqHdr)
			return -EFAULT;
		pucCodecPrivBuf = compat_ptr(compatSeqHdr);
		if (!access_ok(VERIFY_READ, pucCodecPrivBuf,
			sizeof(__u8) * usr_ptr->u8CodecPrivLen))
			return -EFAULT;

		if (copy_from_user((__u8 __user *)usr_ptr->pucCodecPrivBuf,
			pucCodecPrivBuf, sizeof(__u8) * usr_ptr->u8CodecPrivLen))
			return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u4DivxHdrLen), &(usr_ptr32->u4DivxHdrLen),
		sizeof(__u32)))
		return -EFAULT;

	if ((0 < usr_ptr32->u4DivxHdrLen) && (0 == usr_ptr32->pucDivxHdrBuf)) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
		TEXT("%s line %d fail for no aac header, but header len(%d) > 0.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr->u8CodecPrivLen);
		return -EINVAL;
	}

	if (0 != usr_ptr32->pucDivxHdrBuf) {
		__u8 *pucDivxHdrBuf = NULL;

		usr_ptr->pucDivxHdrBuf = *ppu1NextBufAddr;

		if (NULL == usr_ptr->pucDivxHdrBuf) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("%s line %d fail in alloc compat user space.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return -ENOMEM;
		}
		mm_memset(usr_ptr->pucDivxHdrBuf, 0, sizeof(__u8) * usr_ptr->u4DivxHdrLen);
		*ppu1NextBufAddr +=
			CFA_ALIGN_SZ(sizeof(__u8) * usr_ptr->u4DivxHdrLen, sizeof(uintptr_t));
		*pu4Sz += CFA_ALIGN_SZ(sizeof(__u8) * usr_ptr->u4DivxHdrLen, sizeof(uintptr_t));
		if (*pu4Sz > u4TotalSz)
		{
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d size is large than total size.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
			return -ENOMEM;
		}
		if (get_user(compatSeqHdr, &(usr_ptr32->pucDivxHdrBuf)))
			return -EFAULT;
		if (0 == compatSeqHdr)
			return -EFAULT;
		pucDivxHdrBuf = compat_ptr(compatSeqHdr);
		if (!access_ok(VERIFY_READ, pucDivxHdrBuf,
			sizeof(__u8) * usr_ptr->u4DivxHdrLen))
			return -EFAULT;

		if (copy_from_user((__u8 __user *)usr_ptr->pucDivxHdrBuf,
			 pucDivxHdrBuf, sizeof(__u8) * usr_ptr->u4DivxHdrLen))
			return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->rCfaMkvHeaderStrip), &(usr_ptr32->rCfaMkvHeaderStrip),
		sizeof(CfaMkvHeaderStriping_T)))
		return -EFAULT;

	if (copy_from_user(&(usr_ptr->rCfaMkvContentEncoding), &(usr_ptr32->rCfaMkvContentEncoding),
		sizeof(CfaMkvContentEncoding_T)))
		return -EFAULT;

	ret = CfaMkvCompatAvcSPSPPSInfo(&(usr_ptr->rSPSPPSInfo),
		&(usr_ptr32->rSPSPPSInfo));

	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
		  TEXT("%s line %d fail in CfaMkvCompatAvcSPSPPSInfo.\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}

	if (copy_from_user(&(usr_ptr->u4IndexTableAddr), &(usr_ptr32->u4IndexTableAddr),
		sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8IndexTableIndex), &(usr_ptr32->u8IndexTableIndex),
		sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->pucMpeg4Header), &(usr_ptr32->pucMpeg4Header),
		sizeof(__u8) * MKV_MPEG4_HEADER_LEN))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4Mpeg4HeaderLen), &(usr_ptr32->u4Mpeg4HeaderLen),
		sizeof(__u32)))
		return -EFAULT;

	return 0;
}

static long CfaMkvCompatAudSpecialInfo(CfaMkvAudSpecialInfo_T __user *usr_ptr,
  CfaMkvAudSpecialInfo_T32 __user *usr_ptr32)
{
	long ret = 0;

	if (copy_from_user(&(usr_ptr->u4SamplingFrequency), &(usr_ptr32->u4SamplingFrequency),
    sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4OutputSamplingFrequency), &(usr_ptr32->u4OutputSamplingFrequency),
    sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4Channels), &(usr_ptr32->u4Channels),
    sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4BitDepth), &(usr_ptr32->u4BitDepth),
    sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->rCfaMkvAacHeader), &(usr_ptr32->rCfaMkvAacHeader),
    sizeof(CfaMkvAacHeader_T)))
		return -EFAULT;

  return 0;
}

static long CfaMkvCompatAudInfo(CfaMkvAudInfo_T __user *usr_ptr,
  CfaMkvAudInfo_T32 __user *usr_ptr32,  __u8 **pu1NextUserBuf, __u32 *pu4Sz, __u32 u4TotalSz)
{
	compat_caddr_t compatSeqHdr = 0;
	long ret = 0;

	if (copy_from_user(&(usr_ptr->eCFAAudCodec), &(usr_ptr32->eCFAAudCodec),
		sizeof(AVCODECID_T)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8AudTrackNo), &(usr_ptr32->u8AudTrackNo),
		sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8AudTrackUID), &(usr_ptr32->u8AudTrackUID),
		sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->fgTimeCodeScaleEn), &(usr_ptr32->fgTimeCodeScaleEn),
		sizeof(bool)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8TrackTimeCodeScale), &(usr_ptr32->u8TrackTimeCodeScale),
		sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8CodecPrivOfst), &(usr_ptr32->u8CodecPrivOfst),
		sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8CodecPrivLen), &(usr_ptr32->u8CodecPrivLen),
		sizeof(__u64)))
		return -EFAULT;

	if ((0 < usr_ptr32->u8CodecPrivLen) && (0 == usr_ptr->u8CodecPrivLen)) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
		TEXT("%s line %d fail for no Vorbis header, but header len(%d) > 0.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr->u8CodecPrivLen);
		return -EINVAL;
	}

	if (0 != usr_ptr32->ptrVorbisPrivDataMem) {
		compat_uptr_t ptrVorbisPrivDataMem = NULL;

		usr_ptr->ptrVorbisPrivDataMem = *pu1NextUserBuf;

		if (NULL == usr_ptr->ptrVorbisPrivDataMem) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("%s line %d fail in alloc compat user space.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return -ENOMEM;
		}
		mm_memset(usr_ptr->ptrVorbisPrivDataMem, 0, usr_ptr->u8CodecPrivLen);
		*pu1NextUserBuf += CFA_ALIGN_SZ(usr_ptr->u8CodecPrivLen, sizeof(uintptr_t));
		*pu4Sz += CFA_ALIGN_SZ(usr_ptr->u8CodecPrivLen, sizeof(uintptr_t));
		if (*pu4Sz > u4TotalSz)
		{
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d size is large than total size.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
			return -ENOMEM;
		}

		if (get_user(compatSeqHdr, &(usr_ptr32->ptrVorbisPrivDataMem))){
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(ptrVorbisPrivDataMem).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return -EINVAL;
		}
		if (0 == compatSeqHdr){
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(compatSeqHdr = 0).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return -EINVAL;
		}
		ptrVorbisPrivDataMem = compat_ptr(compatSeqHdr);
		if (!access_ok(VERIFY_READ, ptrVorbisPrivDataMem,
			usr_ptr->u8CodecPrivLen)){
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("%s line %d fail in access_ok(ptrVorbisPrivDataMem: 0x%p).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, ptrVorbisPrivDataMem);
			return -EINVAL;
		}

		if (copy_from_user((void __user *)usr_ptr->ptrVorbisPrivDataMem,
			 ptrVorbisPrivDataMem, usr_ptr->u8CodecPrivLen)){
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(ptrVorbisPrivDataMem).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return -EINVAL;
		}
	}
	if (copy_from_user(&(usr_ptr->u1VorbisCommHdrLen), &(usr_ptr32->u1VorbisCommHdrLen),
		sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->rCfaMkvHeaderStrip), &(usr_ptr32->rCfaMkvHeaderStrip),
		sizeof(CfaMkvHeaderStriping_T)))
		return -EFAULT;
	ret = CfaMkvCompatAudSpecialInfo(&(usr_ptr->rCfaMkvAudSpecialInfo), &(usr_ptr32->rCfaMkvAudSpecialInfo));
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
		  TEXT("%s line %d fail in CfaMkvCompatAudSpecialInfo.\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}
	if (copy_from_user(&(usr_ptr->rCfaMkvContentEncoding), &(usr_ptr32->rCfaMkvContentEncoding),
		sizeof(CfaMkvContentEncoding_T)))
		return -EFAULT;

  return 0;
}

static long CfaMkvCompatConfigCalcSz(CfaMkvConfigInfo_T32 __user *usr_ptr32,
	__u32 *pu4OutSz)
{
	__u32 u4TotalSz = 0;
	__u64 u8HeaderLen = 0;
	__u32 u4HeaderLen = 0;
	__u32 i = 0;
	long ret = 0;

	if ((NULL == usr_ptr32) || (NULL == pu4OutSz)) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	u4TotalSz += CFA_ALIGN_SZ(sizeof(CfaMkvConfigInfo_T), sizeof(uintptr_t));

	if (0 != get_user(u8HeaderLen,	&(usr_ptr32->rCfaMkvVidInfo.u8CodecPrivLen))) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d fail in get_user(rCfaMkvVidInfo.u8CodecPrivLen)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	u4TotalSz += CFA_ALIGN_SZ((__u32)u8HeaderLen, sizeof(uintptr_t));
	
	if (0 != get_user(u4HeaderLen,	&(usr_ptr32->rCfaMkvVidInfo.u4DivxHdrLen))) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d fail in get_user(rCfaFlvCfgVidInfo.u4DivxHdrLen)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	
	u4TotalSz += CFA_ALIGN_SZ(u4HeaderLen, sizeof(uintptr_t));
	for (i = 0; i < MAX_NS_MKV_AUD; i++)
	{
		ret = get_user(u4HeaderLen, &(usr_ptr32->arCfaMkvAudInfo[i].u8CodecPrivLen));
		if (ret != 0) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d fail in get_user(rCfaAviAudInfo[%d].u4AudCodecSpecDataLen).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, i);
			return -EFAULT;
		}
		u4TotalSz += CFA_ALIGN_SZ(u4HeaderLen, sizeof(uintptr_t));

	}
	*pu4OutSz = u4TotalSz;

	return 0;
}

static long CfaMkvCompatConfig(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaMkvConfigInfo_T __user *usr_ptr = NULL;
	CfaMkvConfigInfo_T32 __user *usr_ptr32 = (CfaMkvConfigInfo_T32 __user *)prInfo->usr_ptr32;
	__u32 u4TotalSz = 0;
	__u32 u4Sz = 0;
	__u8 __user *pu1UsrBufAddr = NULL;
	__u8 __user *pu1NextBufAddr = NULL;
	long ret = 0;
	int i = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaMkvConfigInfo_T32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	if (0 != CfaMkvCompatConfigCalcSz(usr_ptr32, &u4TotalSz)) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaFlvCompatConfigCalcSz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	//pu1UsrBufAddr = (__u8 __user *)compat_alloc_user_space(u4TotalSz);
	//size > 8 K may cause fail
	DMX_NewMemory(u4TotalSz, pu1UsrBufAddr);
	*pfgIsUserMem = FALSE;

	if (NULL == pu1UsrBufAddr) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d  alloc mem, u4TotalSz = %d.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4TotalSz);
	mm_memset(pu1UsrBufAddr, 0, u4TotalSz);

	usr_ptr = (CfaMkvConfigInfo_T __user *)pu1UsrBufAddr;

	pu1NextBufAddr = pu1UsrBufAddr + CFA_ALIGN_SZ(sizeof(CfaMkvConfigInfo_T), sizeof(uintptr_t));
	u4Sz = pu1NextBufAddr - pu1UsrBufAddr;
	if (u4Sz > u4TotalSz)
	{
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d size is large than total size.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -ENOMEM;
	}

	if (copy_from_user(&(usr_ptr->u8TimeCodeScale),
		&(usr_ptr32->u8TimeCodeScale),
		sizeof(__u64)))
	{
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->fgHasVideo),
		&(usr_ptr32->fgHasVideo),
		sizeof(bool)))
	{
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	ret = CfaMkvCompatVidInfo(&(usr_ptr->rCfaMkvVidInfo),
				&(usr_ptr32->rCfaMkvVidInfo), &pu1NextBufAddr, &u4Sz, u4TotalSz);

	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
		  TEXT("%s line %d fail in CfaMkvCompatVidInfo.\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return ret;
	}
	if (copy_from_user(&(usr_ptr->u4AudioTrackNs), &(usr_ptr32->u4AudioTrackNs),
		sizeof(__u32)))
	{
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	for (i = 0; i < MAX_NS_MKV_AUD; i++)
	{
		ret = CfaMkvCompatAudInfo((usr_ptr->arCfaMkvAudInfo + i),
			(usr_ptr32->arCfaMkvAudInfo + i), &pu1NextBufAddr, &u4Sz, u4TotalSz);

		if (0 != ret) {
			DMX_FreeMemory(pu1UsrBufAddr);
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			  TEXT("%s line %d fail in CfaMkvCompatConfigAudInfo.\r\n"),
			  DMX_FUNC_NAME, DMX_LINE_NO);
			return ret;
		}
	}
	if (copy_from_user(&(usr_ptr->u4SubTrackNs), &(usr_ptr32->u4SubTrackNs), sizeof(__u32)))
	{
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	for (i = 0; i < MAX_NS_MKV_SP; i++)
	{
		ret = CfaMkvCompatSpInfo((usr_ptr->arCfaMkvSpInfo+ i),
			(usr_ptr32->arCfaMkvSpInfo + i));

		if (0 != ret) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			  TEXT("%s line %d fail in CfaMkvCompatSpInfo.\r\n"),
			  DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return ret;
		}
	}
	if (copy_from_user(&(usr_ptr->rCfaMkvAbnormalFlags), &(usr_ptr32->rCfaMkvAbnormalFlags),
		sizeof(CfaMkvAbnormalFlag_T)))
	{
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

#if CONFIG_CFA_MKV_SUPPORT_DRM
	if (copy_from_user(&(usr_ptr->rAudCryptInfo), &(usr_ptr32->rAudCryptInfo),
		sizeof(CfaMkvAudCryptInfo)))
	{
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
#endif
	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaMkvConfigInfo_T);

	return 0;
}

static long CfaMkvCompatJumpRange(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaMkvKeyFrameRange_T __user *usr_ptr = NULL;
	CfaMkvKeyFrameRange_T32 __user *usr_ptr32 = (CfaMkvKeyFrameRange_T32 __user *)prInfo->usr_ptr32;
	long ret = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaMkvKeyFrameRange_T32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	usr_ptr = (CfaMkvKeyFrameRange_T *)compat_alloc_user_space(sizeof(CfaMkvKeyFrameRange_T));

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
		TEXT("%s line %d fail in alloc compat user space.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaMkvKeyFrameRange_T));
	ret = CfaMkvCompatRangeInfo(&(usr_ptr->rCfaRangeInfo),&(usr_ptr32->rCfaRangeInfo));
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
		TEXT("%s line %d fail in CfaMkvCompatRangeInfo.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}
	if (copy_in_user(&(usr_ptr->u4RWUnitAULen), &(usr_ptr32->u4RWUnitAULen), sizeof(__u32)))
		return -EFAULT;


	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaMkvKeyFrameRange_T);

	return 0;
}

static int CfaMkvProcCompat(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	long ret = 0;

	if ((NULL == prInfo) || (NULL == pfgIsUserMem)) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("%s line %d fail for invalid parameter.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	}
	switch (prInfo->type) {
	case CFA_CONFIG:
	if (prInfo->is_get) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	}
	ret = CfaMkvCompatConfig(prInfo, pfgIsUserMem);
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaMkvCompatConfig.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}
	break;
	case CFA_RANGE:
    if (prInfo->is_get) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	}
	ret = CfaMkvCompatRange(prInfo, pfgIsUserMem);
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaMkvCompatRange.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}
	break;
	case CFA_GEN_INFO:
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("%s line %d fail for don;t support get info for cfa mkv.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	case CFA_JUMP_INFO:
		if (prInfo->is_get) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
		}
		ret = CfaMkvCompatJumpRange(prInfo, pfgIsUserMem);
		if (0 != ret) {
		  DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
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
static CfaApiVidType CfaMkvGetVidType(AVCODECID_T eInVidType)
{
	CfaApiVidType eMappedVidType = CFA_VID_UNKNOWN;

	switch (eInVidType) {
	case AVCODEC_ID_UNKNOWN:
		eMappedVidType = CFA_VID_UNKNOWN;
		break;

	case AVCODEC_ID_MPEG1:
	case AVCODEC_ID_MPEG2:
		eMappedVidType = CFA_VID_MPEG2;
		break;

	case AVCODEC_ID_DIVX3:
		eMappedVidType = CFA_VID_DIVX3;
		break;

	case AVCODEC_ID_MPEG4:
		eMappedVidType = CFA_VID_MPEG4;
		break;

	case AVCODEC_ID_H263:
		eMappedVidType = CFA_VID_H263;
		break;

	case AVCODEC_ID_H264:
		eMappedVidType = CFA_VID_H264;
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

	case AVCODEC_ID_VC1:
		eMappedVidType = CFA_VID_VC1;
		break;

	case AVCODEC_ID_DIVX4:
		eMappedVidType = CFA_VID_DIVX4;
		break;

	case AVCODEC_ID_DIVX5:
		eMappedVidType = CFA_VID_DIVX6;
		break;

	case AVCODEC_ID_SORENSON:
		eMappedVidType = CFA_VID_H263_SORENSON;
		break;

	case AVCODEC_ID_MJPEG:
		eMappedVidType = CFA_VID_MJPEG;
		break;

	case AVCODEC_ID_VP8:
		eMappedVidType = CFA_VID_VP8;
		break;

	case AVCODEC_ID_VP6:
		eMappedVidType = CFA_VID_VP6;
		break;

	case AVCODEC_ID_H265:
		eMappedVidType = CFA_VID_H265;
		break;
	default:
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV] eInVidType is %d, map unknown\r\n"), eInVidType);
		eMappedVidType = CFA_VID_UNKNOWN;
		break;
	}

	return eMappedVidType;
}

static CfaApiAudType CfaMkvGetAudType(AVCODECID_T eInAudType)
{
	CfaApiAudType eMappedAudType = CFA_AUD_DRV_FMT_UNKNOWN;

	switch (eInAudType) {
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

	case AVCODEC_ID_AAC:
		eMappedAudType = CFA_AUD_DRV_FMT_AAC;
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

	case AVCODEC_ID_FLAC:
		eMappedAudType = CFA_AUD_DRV_FMT_FLAC;
		break;

	default:
		eMappedAudType = CFA_AUD_DRV_FMT_UNKNOWN;
		break;
	}

	return eMappedAudType;
}

static MRESULT CfaMkvInit(void *pvSptHdl, void **ppvCfaPrivData)
{
	CfaMkvInst *prCfaMkv = NULL;
	MRESULT  mrResult = RET_DMX_OK;
	u8  uAudNum = 0;

	MOD_VERSION_INFO(MKV_CFA_MOD, MKV_CFA_MM, MKV_CFA_mm, MKV_CFA_rev);

	DMX_NewMemory(sizeof(CfaMkvInst), prCfaMkv);
	if (NULL == prCfaMkv) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV] alloc prCfaMkv fail!\r\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}

	dmx_memset(prCfaMkv, 0, sizeof(CfaMkvInst));

	MMATE_INIT_POINTER(prCfaMkv);
	MMATE_INIT_STRUCT(prCfaMkv->rCurBlock);

	prCfaMkv->u4PrsFlg = 0;
	prCfaMkv->pu1Wvc1Header = NULL;
	prCfaMkv->pu1Mp4SeqHdr = NULL;

	DMX_NewHwMemory(CFA_MKV_WVC1_HEADER_LEN, prCfaMkv->pu1Wvc1Header);
	if (NULL == prCfaMkv->pu1Wvc1Header) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV] alloc prCfaMkv fail!\r\n"));
		CfaMkvUninit(pvSptHdl, (void *)prCfaMkv);
		MM_RETURN(RET_DMX_NO_MEM);
	} else
		dmx_memset((void *)prCfaMkv->pu1Wvc1Header, 0, CFA_MKV_WVC1_HEADER_LEN);

	DMX_NewHwMemory(CFA_MKV_MP4_SEQ_HDR_LEN, prCfaMkv->pu1Mp4SeqHdr);
	if (NULL == prCfaMkv->pu1Mp4SeqHdr) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV] alloc prCfaMkv fail!\r\n"));
		CfaMkvUninit(pvSptHdl, (void *)prCfaMkv);
		MM_RETURN(RET_DMX_NO_MEM);
	} else
		dmx_memset((void *)prCfaMkv->pu1Mp4SeqHdr, 0, CFA_MKV_MP4_SEQ_HDR_LEN);

	prCfaMkv->pvSptHdl = pvSptHdl;
	prCfaMkv->ptrMemAddr = DMX_INVALID_UINTPTR_T;
	prCfaMkv->u4AvailSize = 0;
	prCfaMkv->u4SliceHeaderDataPartLen = 0;

	prCfaMkv->u4VorbisHeadPageNum = 0;
	prCfaMkv->u4VorbisHeadPageIndex = 0;
	prCfaMkv->u8VorbisHeadCurPageDataSize = 0;
	prCfaMkv->u8VorbisHeadCurPageTotalAULen = 0;
	prCfaMkv->u8VorbisHeadDataSize = 0;
	prCfaMkv->u8VorbisHeadDataOfst = 0;
	prCfaMkv->u4VorbisAuNs = 0;

#if CONFIG_CFA_MKV_SUPPORT_DRM
	dmx_memset((void *)&(prCfaMkv->rAudCryptInfo), 0, sizeof(CfaMkvAudCryptInfo));
#endif

	prCfaMkv->u4AudNum = 0;
	prCfaMkv->u4SpNum = 0;
	dmx_memset((void *)&(prCfaMkv->rVidStmInfo), 0, sizeof(prCfaMkv->rVidStmInfo));
	dmx_memset((void *)(prCfaMkv->arAudStmInfo), 0, sizeof(prCfaMkv->arAudStmInfo));
	dmx_memset((void *)(prCfaMkv->arSpStmInfo), 0, sizeof(prCfaMkv->arSpStmInfo));
	dmx_memset((void *)&(prCfaMkv->rSkip), 0, sizeof(prCfaMkv->rSkip));

	prCfaMkv->pucAvcPPSBuf = NULL;
	prCfaMkv->pucAvcSPSBuf = NULL;
	prCfaMkv->pucAvcHevcStartCode = NULL;

	prCfaMkv->u8HdrLen = 0;
	prCfaMkv->eCurElement = MKV_ID_UNKNOW;
	prCfaMkv->u8CurAId = DMX_INVALID_UINT64;
	prCfaMkv->u8CurSpId = DMX_INVALID_UINT64;
	prCfaMkv->u8CurVId = DMX_INVALID_UINT64;
	prCfaMkv->ucCurAIndex = DMX_INVALID_UINT8;
	prCfaMkv->ucCurSpIndex = DMX_INVALID_UINT8;
	dmx_memset((void *)&(prCfaMkv->rRange), 0, sizeof(prCfaMkv->rRange));
	prCfaMkv->eCurPrsStrm = CFA_PRS_BIT_STRM_TYPE_NONE;
	prCfaMkv->fgFillDummyAU = FALSE;
	prCfaMkv->fgTxSeqHdrFromBuf = FALSE;
	prCfaMkv->fgFindIdx = FALSE;

	for (uAudNum = 0; uAudNum < MAX_NS_MKV_AUD; uAudNum++) {
		DMX_NewHwMemory(CFA_MKV_AAC_HEADER_LEN, prCfaMkv->arAudStmInfo[uAudNum].auAacHeader);
		if (NULL == prCfaMkv->arAudStmInfo[uAudNum].auAacHeader) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA MKV] alloc prCfaMkv->arAudStmInfo[%d].auAacHeader fail!\r\n"), uAudNum);
			CfaMkvUninit(pvSptHdl, (void *)prCfaMkv);
			MM_RETURN(RET_DMX_NO_MEM);
		} else {
			dmx_memset((void *)prCfaMkv->arAudStmInfo[uAudNum].auAacHeader, 0, CFA_MKV_AAC_HEADER_LEN);
		}

		DMX_NewHwMemory(OGG_HEAD_SIZE, prCfaMkv->arAudStmInfo[uAudNum].rCfaMkvHeader.auHeader);
		if (NULL == prCfaMkv->arAudStmInfo[uAudNum].rCfaMkvHeader.auHeader) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA MKV] alloc prCfaMkv->arAudStmInfo[%d].")
				TEXT("rCfaMkvHeader.auHeader fail!\r\n"), uAudNum);
			CfaMkvUninit(pvSptHdl, (void *)prCfaMkv);
			MM_RETURN(RET_DMX_NO_MEM);
		} else
			dmx_memset((void *)prCfaMkv->arAudStmInfo[uAudNum].rCfaMkvHeader.auHeader, 0, OGG_HEAD_SIZE);

	}

	DMX_NewHwMemory(MKV_HEADER_STRPING_LENGTH, prCfaMkv->rVidStmInfo.rCfaMkvHeader.auHeader);
	if (NULL ==  prCfaMkv->rVidStmInfo.rCfaMkvHeader.auHeader) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV] alloc	prCfaMkv->rVidStmInfo.rCfaMkvHeader.auHeader fail!\r\n"));
		CfaMkvUninit(pvSptHdl, (void *)prCfaMkv);
		MM_RETURN(RET_DMX_NO_MEM);
	} else
		dmx_memset((void *)prCfaMkv->rVidStmInfo.rCfaMkvHeader.auHeader, 0, MKV_HEADER_STRPING_LENGTH);

	mrResult = CfaMkvInitPara(prCfaMkv);
	if (RET_DMX_OK != mrResult) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT, TEXT("[CFA MKV] CfaMkvInitPara error!\r\n"));
		CfaMkvUninit(pvSptHdl, (void *)prCfaMkv);
		MM_RETURN(mrResult);
	}

	*ppvCfaPrivData = (void *) prCfaMkv;

	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON, TEXT("[CFA MKV] Init OK!\r\n"));

	MMATE_CHECK_POINTER(prCfaMkv);
	MMATE_CHECK_STRUCT(prCfaMkv->rVidStmInfo);

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaMkvTxDone(void *pvSptHdl, u64 u8TxLen, void *pvPrivData, bool fgRsp)
{
	CfaMkvInst *prCfaMkv = NULL;
	MRESULT mrResult = RET_DMX_OK;

	if (NULL == pvPrivData) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV] line %d Send EOS ,pvPrivData is NULL! \r\n"), DMX_LINE_NO);
		MM_RETURN(RET_DMX_NO_MEM);
	}
	prCfaMkv = (CfaMkvInst *)pvPrivData;

	MMATE_CHECK_POINTER(prCfaMkv);
	MMATE_CHECK_STRUCT(prCfaMkv->rCurBlock);
	MMATE_CHECK_STRUCT(prCfaMkv->rRange);
	MMATE_CHECK_STRUCT(prCfaMkv->rVidStmInfo);

	if (fgRsp) {
		prCfaMkv->u8LastCa = prCfaMkv->u8Ca;
		prCfaMkv->fgRealSyncPb = TRUE;
		prCfaMkv->u8AvalOfst = 0;
		prCfaMkv->u4AvailSize = 0;
		prCfaMkv->fgIfNeedRebuf = FALSE;
		mrResult = Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaMkv->u8Ca, u8TxLen, (u8 *)&(prCfaMkv->ptrMemAddr));
		if (RET_DMX_OK != mrResult) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT, TEXT("[CFA_MKV] Send EOS\r\n"));
			MkvFinishPrs(pvSptHdl,prCfaMkv);
		}
		prCfaMkv->u8AvalOfst = prCfaMkv->u8Ca + u8TxLen + prCfaMkv->u4AvailSize;
		MM_RETURN(mrResult);
	}
	if (prCfaMkv->fgRealSyncPb) {
        	prCfaMkv->ptrLastReadMemAddr = prCfaMkv->ptrMemAddr;
	}
    	prCfaMkv->u8TxLen = u8TxLen;
    	prCfaMkv->fgIfNeedRebuf = FALSE;
    	do{
		prCfaMkv->fgNoNeedSyncPb = FALSE;
		if (prCfaMkv->fgRealSyncPb) {
			if (0 == prCfaMkv->ptrMemAddr) {
				DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA_MKV] failed for prCfaMkv->ptrMemAddr is 0\r\n"));
				Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, TRUE, GAU_E_FAIL);
				MM_RETURN(mrResult);
			}
			prCfaMkv->fgRealSyncPb = FALSE;
		}
		CfaMkvTxDoneStCtrl(pvSptHdl, prCfaMkv->u8TxLen, prCfaMkv);
	}while(prCfaMkv->fgNoNeedSyncPb);

	MM_RETURN(mrResult);
}

static MRESULT CfaMkvSetStrmInf(void *pvSptHdl, u32 u4Strm, u32 u4Info,
							 void *pvPrivData)
{
	CfaMkvInst *prCfaMkv = NULL;
	if (NULL == pvPrivData) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT, TEXT("[CFA_MKV] line %d , pvPrivData is NULL! \r\n"), DMX_LINE_NO);
		MM_RETURN(RET_DMX_EXT_NO_MEM);
	}
	prCfaMkv = (CfaMkvInst *)pvPrivData;

	MMATE_CHECK_POINTER(prCfaMkv);
	MMATE_CHECK_STRUCT(prCfaMkv->rCurBlock);
	MMATE_CHECK_STRUCT(prCfaMkv->rVidStmInfo);

	if (CFA_STRM_V == u4Strm)
		prCfaMkv->u8CurVId = (u64)u4Info;
	else if (CFA_STRM_A == u4Strm) {
		prCfaMkv->u8CurAId = (u64)u4Info;
		DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV] Line %d, The current stream id is %d\r\n"), DMX_LINE_NO, u4Info);
		prCfaMkv->ucCurAIndex = CfaMkvGetAudIndex(prCfaMkv, (u64)u4Info);
	} else if (CFA_STRM_SP == u4Strm) {
		prCfaMkv->u8CurSpId = (u64)u4Info;
		prCfaMkv->ucCurSpIndex = CfaMkvGetSpIndex(prCfaMkv, (u64)u4Info);
	} else {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV] stream type is unknown ! \r\n"));
	}

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaMkvEnableStrm(void *pvSptHdl, u32 u4StrmToPrs, CfaStreamOp eOp,
							  void *pvPrivData)
{
	CfaMkvInst *prCfaMkv = NULL;

	if (NULL == pvPrivData) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV] line %d , pvPrivData is NULL! \r\n"), DMX_LINE_NO);
		MM_RETURN(RET_DMX_EXT_NO_MEM);
	}
	prCfaMkv = (CfaMkvInst *)pvPrivData;

	MMATE_CHECK_STRUCT(prCfaMkv->rVidStmInfo);

	if (CFA_STREAM_ON == eOp) {
		if (CFA_STRM_V & u4StrmToPrs) {
			prCfaMkv->u4PrsFlg |= CFA_PRS_BIT_STRM_TYPE_V;
			DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
				TEXT("[CFA MKV] Enable video stream!\r\n"));
		}

		if (CFA_STRM_A & u4StrmToPrs) {
			prCfaMkv->u4PrsFlg |= CFA_PRS_BIT_STRM_TYPE_A;
			prCfaMkv->fgTxFirst = TRUE;
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA MKV] Enable audio stream!\r\n"));
		}

		if (CFA_STRM_SP & u4StrmToPrs) {
			prCfaMkv->u4PrsFlg |= CFA_PRS_BIT_STRM_TYPE_SP;
			DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
				TEXT("[CFA MKV] Enable subtitle stream!\r\n"));
		}
	} else {
		if (CFA_STRM_V & u4StrmToPrs) {
			prCfaMkv->u4PrsFlg &= ~((u32)CFA_PRS_BIT_STRM_TYPE_V);
			DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
				TEXT("[CFA MKV] Disable video stream!\r\n"));
		}

		if (CFA_STRM_A & u4StrmToPrs) {
			prCfaMkv->u4PrsFlg &= ~((u32)CFA_PRS_BIT_STRM_TYPE_A);
			DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
				TEXT("[CFA MKV] Disable audio stream!\r\n"));
		}

		if (CFA_STRM_SP & u4StrmToPrs) {
			prCfaMkv->u4PrsFlg &= ~((u32)CFA_PRS_BIT_STRM_TYPE_SP);
			DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
				TEXT("[CFA MKV] Disable subtitle stream!\r\n"));
		}
	}

	MM_RETURN(RET_DMX_OK);
}
#define SIZE_MAX  ((UINT64) -1)

static MRESULT CfaMkvConfigure(void *pvSptHdl, void *pvParam, void *pvPrivData, bool fgIsUserMem)
{
	CfaMkvConfigInfo_T *prCfaMkvConfigInfo = NULL;
	CfaMkvConfigInfo_T *prCfaMkvCogInfo = NULL;
	CfaMkvInst *prCfaMkv = NULL;
	CfaMkvVidStreamInfo *prCfaMkvVInf = NULL;
	CfaMkvAudStreamInfo *prCfaMkvAInf = NULL;
	CfaMkvSpStreamInfo *prCfaMkvSpInf = NULL;
	u32 u4Idx = 0;
	u64 u8Len1 = 0;
	u64 u8Len2 = 0;
	u64 u8Offset = 0;
	u8* privData = NULL;
	u64 u8PrivDataSize = 0;
	bool ret = FALSE;

	if (NULL == pvParam) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV] line %d Send EOS ,pvParam is NULL! \r\n"), DMX_LINE_NO);
		MM_RETURN(RET_DMX_EXT_NO_MEM);
	}

	if (NULL == pvPrivData) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV] line %d Send EOS ,pvPrivData is NULL! \r\n"), DMX_LINE_NO);
		if (!fgIsUserMem) {
			DMX_FreeMemory(pvParam);
		}
		MM_RETURN(RET_DMX_EXT_NO_MEM);
	}
	DMX_NewMemory(sizeof(CfaMkvConfigInfo_T), prCfaMkvCogInfo);
	if (NULL == prCfaMkvCogInfo)
	{
		if (!fgIsUserMem) {
			DMX_FreeMemory(pvParam);
		}
		MM_RETURN(RET_DMX_NO_MEM);
	}

	mm_memset(prCfaMkvCogInfo, 0, sizeof(CfaMkvConfigInfo_T));
	if (fgIsUserMem) {
		if (0 != mm_copy_from_user(prCfaMkvCogInfo,
			pvParam, sizeof(CfaMkvConfigInfo_T))) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA_MKV] %s line %d failed in mm_copy_from_user\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			if (prCfaMkvCogInfo != NULL)
				DMX_FreeMemory(prCfaMkvCogInfo);
			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
		}
	}
	else
	{
		mm_memcpy(prCfaMkvCogInfo,
			pvParam, sizeof(CfaMkvConfigInfo_T));
	}
	prCfaMkvConfigInfo = prCfaMkvCogInfo;

	prCfaMkv = (CfaMkvInst *)pvPrivData;
	prCfaMkvVInf = &(prCfaMkv->rVidStmInfo);

	dmx_memcpy((void *)(&(prCfaMkv->rAbnormalFlags)),
				&(prCfaMkvConfigInfo->rCfaMkvAbnormalFlags),
					sizeof(CfaMkvAbnormalFlag_T));
	prCfaMkv->u8TimeCodeScale = prCfaMkvConfigInfo->u8TimeCodeScale;
	prCfaMkv->fgHasVideo = prCfaMkvConfigInfo->fgHasVideo;

	/* video related info from playback */
	if (prCfaMkvConfigInfo->rCfaMkvVidInfo.eVidCodec == AVCODEC_ID_RV)
	{
		if (prCfaMkvConfigInfo->rCfaMkvVidInfo.eRVVersion == VCODEC_VERSION_RV_30)
		{
		    prCfaMkvVInf->eVidCodec = CFA_VID_RV30;
		} else if (prCfaMkvConfigInfo->rCfaMkvVidInfo.eRVVersion == VCODEC_VERSION_RV_40) {
		    prCfaMkvVInf->eVidCodec = CFA_VID_RV40;
		}
	}
	else
	{
        prCfaMkvVInf->eVidCodec		= CfaMkvGetVidType(prCfaMkvConfigInfo->rCfaMkvVidInfo.eVidCodec);
	}

	prCfaMkvVInf->fgTimeCodeScaleEn    = prCfaMkvConfigInfo->rCfaMkvVidInfo.fgTimeCodeScaleEn;
	prCfaMkvVInf->u8TrackTimeCodeScale = prCfaMkvConfigInfo->rCfaMkvVidInfo.u8TrackTimeCodeScale;
	prCfaMkvVInf->u8VidTrackNo		   = prCfaMkvConfigInfo->rCfaMkvVidInfo.u8VidTrackNo;
	prCfaMkvVInf->u8VidTrackUID			= prCfaMkvConfigInfo->rCfaMkvVidInfo.u8VidTrackUID;
	prCfaMkvVInf->u8CodecPrivLen	   = prCfaMkvConfigInfo->rCfaMkvVidInfo.u8CodecPrivLen;
	prCfaMkvVInf->u8CodecPrivOfst	   = prCfaMkvConfigInfo->rCfaMkvVidInfo.u8CodecPrivOfst;
	prCfaMkvVInf->u4DivxHdrLen		   = prCfaMkvConfigInfo->rCfaMkvVidInfo.u4DivxHdrLen;
	prCfaMkvVInf->u4NaluSize           = prCfaMkvConfigInfo->rCfaMkvVidInfo.u4NalSizeLen;

	dmx_memcpy(&(prCfaMkvVInf->rSPSPPSInfo),
				&(prCfaMkvConfigInfo->rCfaMkvVidInfo.rSPSPPSInfo),
					sizeof(AVC_SPS_PPS_INFO_T));

	if (prCfaMkvConfigInfo->rCfaMkvVidInfo.u4Mpeg4HeaderLen > MKV_MPEG4_HEADER_LEN)
		prCfaMkvConfigInfo->rCfaMkvVidInfo.u4Mpeg4HeaderLen = MKV_MPEG4_HEADER_LEN;

	prCfaMkvVInf->u4Mpeg4HeaderLen = prCfaMkvConfigInfo->rCfaMkvVidInfo.u4Mpeg4HeaderLen;

	if (prCfaMkvConfigInfo->rCfaMkvVidInfo.u4Mpeg4HeaderLen != 0) {
		dmx_memcpy(prCfaMkvVInf->pucMpeg4Header,
				prCfaMkvConfigInfo->rCfaMkvVidInfo.pucMpeg4Header,
					prCfaMkvConfigInfo->rCfaMkvVidInfo.u4Mpeg4HeaderLen);
	}

	dmx_memcpy(&(prCfaMkvVInf->rCfaMkvContentEncoding),
				&(prCfaMkvConfigInfo->rCfaMkvVidInfo.rCfaMkvContentEncoding),
					sizeof(CfaMkvContentEncoding_T));

	if ((NULL == prCfaMkvVInf->pucCodecPrivBuf) && (prCfaMkvVInf->u8CodecPrivLen != 0)) {
		DMX_NewHwMemory((u32)prCfaMkvVInf->u8CodecPrivLen, prCfaMkvVInf->pucCodecPrivBuf);
		if (NULL == prCfaMkvVInf->pucCodecPrivBuf) {
			if (prCfaMkvCogInfo != NULL)
				DMX_FreeMemory(prCfaMkvCogInfo);
			if (!fgIsUserMem) {
				DMX_FreeMemory(pvParam);
			}
			MM_RETURN(RET_DMX_NO_MEM);
		}
	}

#if 0
	DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV][SPS_PPS] has sps pps header?%s\r\n"),
			prCfaMkvVInf->rSPSPPSInfo.fgHasSPSStartCode?L"TRUE":L"FALSE");

	DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV][SPS_PPS] u4PPSNum is %d\r\n"),
			prCfaMkvVInf->rSPSPPSInfo.u4PPSNum);
	DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV][SPS_PPS] u4PPSDataLen is %d\r\n"),
			prCfaMkvVInf->rSPSPPSInfo.u4PPSDataLen);
	DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV][SPS_PPS] u8PPSDataOffset is %I64d[0x%llx]\r\n"),
			prCfaMkvVInf->rSPSPPSInfo.u8PPSDataOffset,
			prCfaMkvVInf->rSPSPPSInfo.u8PPSDataOffset);

	DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV][SPS_PPS] u4SPSNum is %d\r\n"),
			prCfaMkvVInf->rSPSPPSInfo.u4SPSNum);
	DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV][SPS_PPS] u4SPSDataLen is %d\r\n"),
			prCfaMkvVInf->rSPSPPSInfo.u4SPSDataLen);
	DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV][SPS_PPS] u8SPSDataOffset is %I64d[0x%llx]\r\n"),
			prCfaMkvVInf->rSPSPPSInfo.u8SPSDataOffset,
			prCfaMkvVInf->rSPSPPSInfo.u8SPSDataOffset);
#endif

	if ((NULL != prCfaMkvConfigInfo->rCfaMkvVidInfo.pucCodecPrivBuf)
		&& (NULL != prCfaMkvVInf->pucCodecPrivBuf)
		&& (prCfaMkvVInf->u8CodecPrivLen != 0)) {
		dmx_memset(prCfaMkvVInf->pucCodecPrivBuf,
			0,
			(u32)(prCfaMkvVInf->u8CodecPrivLen * sizeof(u8)));
		if (fgIsUserMem) {
			if (0 != mm_copy_from_user(prCfaMkvVInf->pucCodecPrivBuf,
					prCfaMkvConfigInfo->rCfaMkvVidInfo.pucCodecPrivBuf,
					prCfaMkvVInf->u8CodecPrivLen * sizeof(u8))) {
				DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA MKV] copy from user fail\r\n"));
				if (prCfaMkvCogInfo != NULL)
					DMX_FreeMemory(prCfaMkvCogInfo);
				MM_RETURN(RET_DMX_EXT_EXCEPTION);
			}
		}
		else {
			dmx_memcpy(prCfaMkvVInf->pucCodecPrivBuf,
					prCfaMkvConfigInfo->rCfaMkvVidInfo.pucCodecPrivBuf,
					prCfaMkvVInf->u8CodecPrivLen * sizeof(u8));
		}
	}

	if ((NULL == prCfaMkvVInf->pucDivxHdrBuf) && (prCfaMkvVInf->u4DivxHdrLen > 0)) {
		DMX_NewHwMemory(prCfaMkvVInf->u4DivxHdrLen, prCfaMkvVInf->pucDivxHdrBuf);
		if (NULL == prCfaMkvVInf->pucDivxHdrBuf) {
			if (prCfaMkvCogInfo != NULL)
				DMX_FreeMemory(prCfaMkvCogInfo);
			if (!fgIsUserMem) {
				DMX_FreeMemory(pvParam);
			}
			MM_RETURN(RET_DMX_NO_MEM);
		}
	}
	if ((NULL != prCfaMkvConfigInfo->rCfaMkvVidInfo.pucDivxHdrBuf)
		&& (NULL != prCfaMkvVInf->pucDivxHdrBuf)
		&& (prCfaMkvVInf->u4DivxHdrLen > 0)) {
		dmx_memset(prCfaMkvVInf->pucDivxHdrBuf,
			0,
			prCfaMkvVInf->u4DivxHdrLen * sizeof(u8));
		if (fgIsUserMem) {
			if (0 != mm_copy_from_user(prCfaMkvVInf->pucDivxHdrBuf,
					prCfaMkvConfigInfo->rCfaMkvVidInfo.pucDivxHdrBuf,
					prCfaMkvVInf->u4DivxHdrLen * sizeof(u8))) {
				DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA MKV] mm copy from user fail\r\n"));
				if (prCfaMkvCogInfo != NULL)
					DMX_FreeMemory(prCfaMkvCogInfo);
				MM_RETURN(RET_DMX_EXT_EXCEPTION);
			}
		} else {
			dmx_memcpy(prCfaMkvVInf->pucDivxHdrBuf,
					prCfaMkvConfigInfo->rCfaMkvVidInfo.pucDivxHdrBuf,
					prCfaMkvVInf->u4DivxHdrLen * sizeof(u8));
		}
	}

	prCfaMkvVInf->rCfaMkvHeader.fgHeaderStriping =
		prCfaMkvConfigInfo->rCfaMkvVidInfo.rCfaMkvHeaderStrip.fgHeaderStriping;
	prCfaMkvVInf->rCfaMkvHeader.uHeaderLen = prCfaMkvConfigInfo->rCfaMkvVidInfo.rCfaMkvHeaderStrip.uHeaderLen;
	dmx_memset((void *)(prCfaMkvVInf->rCfaMkvHeader.auHeader), 0, MKV_HEADER_STRPING_LENGTH);
	dmx_memcpy((void *)(prCfaMkvVInf->rCfaMkvHeader.auHeader),
				(void *)(prCfaMkvConfigInfo->rCfaMkvVidInfo.rCfaMkvHeaderStrip.auHeader),
					MKV_HEADER_STRPING_LENGTH);

	prCfaMkv->u4AudNum = prCfaMkvConfigInfo->u4AudioTrackNs;
	for (u4Idx = 0; u4Idx < prCfaMkv->u4AudNum; u4Idx++) {
		prCfaMkvAInf = NULL;
		prCfaMkvAInf = &(prCfaMkv->arAudStmInfo[u4Idx]);

		prCfaMkvAInf->eAudCodec = CfaMkvGetAudType(prCfaMkvConfigInfo->arCfaMkvAudInfo[u4Idx].eCFAAudCodec);
		prCfaMkvAInf->fgTimeCodeScaleEn = prCfaMkvConfigInfo->arCfaMkvAudInfo[u4Idx].fgTimeCodeScaleEn;
		prCfaMkvAInf->u8TrackTimeCodeScale = prCfaMkvConfigInfo->arCfaMkvAudInfo[u4Idx].u8TrackTimeCodeScale;
		prCfaMkvAInf->u8AudTrackNo = prCfaMkvConfigInfo->arCfaMkvAudInfo[u4Idx].u8AudTrackNo;
		prCfaMkvAInf->u8AudTrackUID = prCfaMkvConfigInfo->arCfaMkvAudInfo[u4Idx].u8AudTrackUID;
		prCfaMkvAInf->u8CodecPrivLen = prCfaMkvConfigInfo->arCfaMkvAudInfo[u4Idx].u8CodecPrivLen;
		prCfaMkvAInf->u8CodecPrivOfst = prCfaMkvConfigInfo->arCfaMkvAudInfo[u4Idx].u8CodecPrivOfst;
		prCfaMkvAInf->u1VorbisCommHdrLenth = prCfaMkvConfigInfo->arCfaMkvAudInfo[u4Idx].u1VorbisCommHdrLen;

		if (CFA_AUD_DRV_FMT_VORBIS == prCfaMkvAInf->eAudCodec) {
			privData = (UINT8 *)prCfaMkvConfigInfo->arCfaMkvAudInfo[u4Idx].ptrVorbisPrivDataMem;
			u8PrivDataSize = prCfaMkvConfigInfo->arCfaMkvAudInfo[u4Idx].u8CodecPrivLen;
			ret = FALSE;
			do {
				if ((privData == NULL) || (u8PrivDataSize < 1)) {
					break;
				}
				if (privData[0] != 0x2) {
					break;
				}
				u8Len1 = 0;
				u8Offset = 1;
				ret = TRUE;
				while ((u8Offset < u8PrivDataSize)
					&& (privData[u8Offset] == 0xff)) {
					if (u8Len1 > (SIZE_MAX - 0xff)) {
						ret = FALSE;
						break;
					}
					u8Len1 += 0xff;
					u8Offset++;
				}
				if (!ret) {
					break;
				}
				if (u8Offset >= u8PrivDataSize) {
					break;
				}
				if (u8Len1 > (SIZE_MAX - privData[u8Offset])) {
					break;
				}
				u8Len1 += privData[u8Offset++];

				u8Len2 = 0;
				ret = TRUE;
				while ((u8Offset < u8PrivDataSize)
					&& (privData[u8Offset] == 0xff)) {
					if (u8Len2 > (SIZE_MAX - 0xff)) {
						ret = FALSE;
						break;
					}

					u8Len2 += 0xff;
					u8Offset++;
				}
				if (!ret) {
					break;
				}
				if (u8Offset >= u8PrivDataSize) {
					break;
				}
				if (u8Len2 > (SIZE_MAX - privData[u8Offset])) {
					break;
				}
				u8Len2 += privData[u8Offset++];

				if ((u8Len1 > (SIZE_MAX - u8Len2))
				|| (u8Offset > (SIZE_MAX - (u8Len1 + u8Len2)))
				|| (u8PrivDataSize < (u8Offset + u8Len1 + u8Len2))
				) {
					break;
				}

				if (privData[u8Offset] != 0x1) {
					break;
				}
				prCfaMkvAInf->u8VorbisIdOft = u8Offset;
				prCfaMkvAInf->u8VorbisIdSize = u8Len1;
				u8Offset += u8Len1;
				if (privData[u8Offset] != 0x3) {
					break;
				}

				prCfaMkvAInf->u8VorbisCommHdrOft = u8Offset;
				prCfaMkvAInf->u8VorbisCommHdrSize = u8Len2;
				u8Offset += u8Len2;
				if (privData[u8Offset] != 0x5) {
					break;
				}
				//prCfaMkvAInf->u8VorbisHeadOft = u8Offset;
				//prCfaMkvAInf->u8VorbisHeadSize = u8Len2;
				//prCfaMkvAInf->u8VorbisHeadSize = u8PrivDataSize - u8Offset;
				prCfaMkvAInf->u8VorbisHeadOft = prCfaMkvAInf->u8VorbisCommHdrOft;
				prCfaMkvAInf->u8VorbisHeadSize = u8PrivDataSize - prCfaMkvAInf->u8VorbisCommHdrOft;
				ret = TRUE;
			} while (0);

			if (!ret) {
				MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);
			}

		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT, TEXT("[CFA MKV][VORBIS]--  vorbisIdOffset(%lld), vorbisIdOffset(%lld), vorbisHeadOffset(%lld), vorbisHeadOffset(%lld)\r\n"),
		prCfaMkvAInf->u8VorbisIdOft, prCfaMkvAInf->u8VorbisIdSize, prCfaMkvAInf->u8VorbisHeadOft, prCfaMkvAInf->u8VorbisHeadSize);

		DMX_NewHwMemory(prCfaMkvAInf->u8VorbisHeadSize, prCfaMkvAInf->pucAudCodecPrivData);
		if (NULL == prCfaMkvAInf->pucAudCodecPrivData) {
			if (prCfaMkvCogInfo != NULL)
				DMX_FreeMemory(prCfaMkvCogInfo);
			if (!fgIsUserMem) {
				DMX_FreeMemory(pvParam);
			}
			MM_RETURN(RET_DMX_NO_MEM);
		} else {
			dmx_memset((void *)prCfaMkvAInf->pucAudCodecPrivData,
			0,
			prCfaMkvAInf->u8VorbisHeadSize);
		}

		DMX_NewHwMemory(prCfaMkvAInf->u8VorbisIdSize, prCfaMkvAInf->pucAudCodecVorbisID);
		if (NULL == prCfaMkvAInf->pucAudCodecVorbisID) {
			if (!fgIsUserMem) {
				DMX_FreeMemory(pvParam);
			}
			MM_RETURN(RET_DMX_NO_MEM);
		}

		dmx_memset((void *)prCfaMkvAInf->pucAudCodecVorbisID, 0, prCfaMkvAInf->u8VorbisIdSize);
		if (fgIsUserMem) {
			if (0 != mm_copy_from_user(prCfaMkvAInf->pucAudCodecVorbisID,
				privData + prCfaMkvAInf->u8VorbisIdOft,
				prCfaMkvAInf->u8VorbisIdSize)) {
				if (prCfaMkvCogInfo != NULL)
					DMX_FreeMemory(prCfaMkvCogInfo);
				MM_RETURN(RET_DMX_EXT_EXCEPTION);
			}
			if (0 !=
				mm_copy_from_user(prCfaMkvAInf->pucAudCodecPrivData,
				privData + prCfaMkvAInf->u8VorbisHeadOft,
				prCfaMkvAInf->u8VorbisHeadSize)) {
				DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA MKV] mm copy from user fail\r\n"));
				if (prCfaMkvCogInfo != NULL)
					DMX_FreeMemory(prCfaMkvCogInfo);
					MM_RETURN(RET_DMX_EXT_EXCEPTION);
				}
		} else {
			dmx_memcpy(prCfaMkvAInf->pucAudCodecVorbisID,
			privData + prCfaMkvAInf->u8VorbisIdOft,
			prCfaMkvAInf->u8VorbisIdSize);
			dmx_memcpy(prCfaMkvAInf->pucAudCodecPrivData,
			privData + prCfaMkvAInf->u8VorbisHeadOft,
			prCfaMkvAInf->u8VorbisHeadSize);
		}
			DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
			TEXT("[CFA MKV][VORBIS]-- Common header length is 0x%x(%d)\r\n"),
			prCfaMkvAInf->u1VorbisCommHdrLenth, prCfaMkvAInf->u1VorbisCommHdrLenth);
			DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
			TEXT("[CFA MKV][VORBIS] priv len is 0x%llx(%lld), priv ofst is 0x%llx(%lld)\r\n"),
			prCfaMkvAInf->u8CodecPrivLen, prCfaMkvAInf->u8CodecPrivLen,
			prCfaMkvAInf->u8CodecPrivOfst, prCfaMkvAInf->u8CodecPrivOfst);
		}

		prCfaMkvAInf->rCfaMkvHeader.fgHeaderStriping =
			prCfaMkvConfigInfo->arCfaMkvAudInfo[u4Idx].rCfaMkvHeaderStrip.fgHeaderStriping;
		prCfaMkvAInf->rCfaMkvHeader.uHeaderLen =
			prCfaMkvConfigInfo->arCfaMkvAudInfo[u4Idx].rCfaMkvHeaderStrip.uHeaderLen;
		dmx_memset((void *)(prCfaMkvAInf->rCfaMkvHeader.auHeader), 0, MKV_HEADER_STRPING_LENGTH);
		dmx_memcpy((void *)(prCfaMkvAInf->rCfaMkvHeader.auHeader),
					(void *)prCfaMkvConfigInfo->arCfaMkvAudInfo[u4Idx].rCfaMkvHeaderStrip.auHeader,
						MKV_HEADER_STRPING_LENGTH);

		dmx_memcpy(&(prCfaMkvAInf->rCfaMkvContentEncoding),
					&(prCfaMkvConfigInfo->arCfaMkvAudInfo[u4Idx].rCfaMkvContentEncoding),
						sizeof(CfaMkvContentEncoding_T));

		prCfaMkvAInf->u4Channles =
			prCfaMkvConfigInfo->arCfaMkvAudInfo[u4Idx].rCfaMkvAudSpecialInfo.u4Channels;
		prCfaMkvAInf->u4SampleRate =
			prCfaMkvConfigInfo->arCfaMkvAudInfo[u4Idx].rCfaMkvAudSpecialInfo.u4SamplingFrequency;
		prCfaMkvAInf->uAacHeaderSize =
			prCfaMkvConfigInfo->arCfaMkvAudInfo[u4Idx].rCfaMkvAudSpecialInfo.rCfaMkvAacHeader.uHeaderSize;
		if (prCfaMkvAInf->uAacHeaderSize > MKV_AAC_HDR_BUF_LEN)
			prCfaMkvAInf->uAacHeaderSize = MKV_AAC_HDR_BUF_LEN;

		dmx_memset((void *)(prCfaMkvAInf->auAacHeader), 0, CFA_MKV_AAC_HEADER_LEN);
		prCfaMkvAInf->auAacHeader[3] = prCfaMkvAInf->uAacHeaderSize;
		dmx_memcpy((void *)&(prCfaMkvAInf->auAacHeader[4]),
			prCfaMkvConfigInfo->arCfaMkvAudInfo[u4Idx].rCfaMkvAudSpecialInfo.rCfaMkvAacHeader.auAacHeader,
			prCfaMkvAInf->uAacHeaderSize);
		prCfaMkvAInf->uAacHeaderSize += 4;
		if (prCfaMkvAInf->eAudCodec == CFA_AUD_DRV_FMT_AAC) {
			prCfaMkvAInf->rCfaMkvHeader.fgHeaderStriping = TRUE;
			prCfaMkvAInf->rCfaMkvHeader.uHeaderLen = 7;
		}
		prCfaMkvAInf->fgHaveTxHeader = FALSE;
	}

	/* subtitle related info from playback */
	prCfaMkv->u4SpNum = prCfaMkvConfigInfo->u4SubTrackNs;
	for (u4Idx = 0; u4Idx < prCfaMkv->u4SpNum; u4Idx++) {
		prCfaMkvSpInf = NULL;

		prCfaMkvSpInf = &(prCfaMkv->arSpStmInfo[u4Idx]);
		prCfaMkvSpInf->fgTimeCodeScaleEn =
			prCfaMkvConfigInfo->arCfaMkvSpInfo[u4Idx].fgTimeCodeScaleEn;
		prCfaMkvSpInf->u8TrackTimeCodeScale =
			prCfaMkvConfigInfo->arCfaMkvSpInfo[u4Idx].u8TrackTimeCodeScale;
		prCfaMkvSpInf->u8CodecPrivLen =
			prCfaMkvConfigInfo->arCfaMkvSpInfo[u4Idx].u8CodecPrivLen;
		prCfaMkvSpInf->u8SpTrackNo = prCfaMkvConfigInfo->arCfaMkvSpInfo[u4Idx].u8SpTrackNo;
		prCfaMkvSpInf->u8SpTrackUID = prCfaMkvConfigInfo->arCfaMkvSpInfo[u4Idx].u8SpTrackUID;
		prCfaMkvSpInf->u8CodecPrivOfst =
			prCfaMkvConfigInfo->arCfaMkvSpInfo[u4Idx].u8CodecPrivOfst;
		prCfaMkvSpInf->eCfaMkvSpType =
			prCfaMkvConfigInfo->arCfaMkvSpInfo[u4Idx].eCfaMkvSpType;
		dmx_memcpy(&(prCfaMkvSpInf->rCfaMkvContentEncoding),
					&(prCfaMkvConfigInfo->arCfaMkvSpInfo[u4Idx].rCfaMkvContentEncoding),
						sizeof(CfaMkvContentEncoding_T));
	}

	if (prCfaMkv->rVidStmInfo.u8CodecPrivLen != 0)
		prCfaMkv->fgNotStartFromCluster = TRUE;/*for later check if range start from cluster*/

	prCfaMkv->rVidStmInfo.u8CodecLen = prCfaMkv->rVidStmInfo.u8CodecPrivLen;
#if CONFIG_CFA_MKV_SUPPORT_DRM
	dmx_memcpy(&(prCfaMkv->rAudCryptInfo), &(prCfaMkvConfigInfo->rAudCryptInfo),
				sizeof(CfaMkvAudCryptInfo));
#endif

	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON, TEXT("[CFA MKV] Config!\r\n"));
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA MKV] Config:TimeCodeScale: 0x%llx\r\n"), prCfaMkv->u8TimeCodeScale);
	if (prCfaMkvVInf->fgTimeCodeScaleEn) {
		DMXLOG_DEBUG(
			TEXT("[CFA MKV] Config:Video TimeCodeScale: %llx\r\n"),
			prCfaMkvVInf->u8TrackTimeCodeScale);
	}
	if (prCfaMkvCogInfo != NULL)
		DMX_FreeMemory(prCfaMkvCogInfo);

	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA MKV] Config:CodecPrivate Offset: 0x%llx\r\n"), prCfaMkvVInf->u8CodecPrivOfst);
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA MKV] Config:CodecPrivate Len: 0x%llx\r\n"), prCfaMkvVInf->u8CodecPrivLen);
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA MKV] Config:Video Codec: %d\r\n"), prCfaMkvVInf->eVidCodec);
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA MKV] Config:Audio numbers: %d\r\n"), prCfaMkv->u4AudNum);
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA MKV] Config:1st Audio Codec: %d\r\n"), prCfaMkv->arAudStmInfo[0].eAudCodec);
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA MKV] Config:Subtitle numbers: %d\r\n"), prCfaMkv->u4SpNum);

	MMATE_CHECK_POINTER(prCfaMkv);
	MMATE_CHECK_STRUCT(prCfaMkv->rCurBlock);
	MMATE_CHECK_STRUCT(prCfaMkv->rVidStmInfo);
	if (!fgIsUserMem) {
		DMX_FreeMemory(pvParam);
	}
	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaMkvTurnOn(void *pvSptHdl, void *pvPrivData)
{
	CfaMkvInst *prCfaMkv = NULL;
	MRESULT mrResult = RET_DMX_OK;

	if (NULL == pvPrivData) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV] line %d Send EOS ,pvPrivData is NULL! \r\n"), DMX_LINE_NO);
		MM_RETURN(RET_DMX_NO_MEM);
	}
	prCfaMkv = (CfaMkvInst *)pvPrivData;

	MMATE_CHECK_POINTER(prCfaMkv);
	MMATE_CHECK_STRUCT(prCfaMkv->rCurBlock);
	MMATE_CHECK_STRUCT(prCfaMkv->rRange);
	MMATE_CHECK_STRUCT(prCfaMkv->rVidStmInfo);

	if (prCfaMkv->rRange.u8VidStartOfst >= (prCfaMkv->rRange.u8VidEndOfst - 1)) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV] Line %d Send EOS\r\n"), DMX_LINE_NO);
		Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->rRange.u8VidEndOfst, TRUE, GAU_E_FAIL);
		MM_RETURN(RET_DMX_OK);
	}

	mrResult = CfaMkvInitPara(prCfaMkv);
	if (RET_DMX_OK != mrResult) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV] CfaMkvInitPara error!\r\n"));
		MM_RETURN(mrResult);
	}

	/* must after InitPara */
	if (prCfaMkv->fgNotStartFromCluster) {
		prCfaMkv->fgCheckRangeCluster = FALSE;
		prCfaMkv->fgNotStartFromCluster = FALSE;
	}
	prCfaMkv->rVidStmInfo.fgTxMpeg4VOLHeader = FALSE;
	if ((prCfaMkv->rVidStmInfo.u8CodecLen != 0) &&
		(prCfaMkv->u4PrsFlg  & CFA_PRS_BIT_STRM_TYPE_V)) { /* no seek */
		if ((prCfaMkv->rRange.u8TargetTimeCode != 0) ||
			(CFA_VID_H265 == prCfaMkv->rVidStmInfo.eVidCodec)) {
			prCfaMkv->fgTxSeqHdrFromBuf = TRUE;
			prCfaMkv->u8Ca = prCfaMkv->rRange.u8VidStartOfst;
		} else {
			if (prCfaMkv->rVidStmInfo.rSPSPPSInfo.fgHasSPSStartCode) {
				prCfaMkv->u8Ca = prCfaMkv->rVidStmInfo.rSPSPPSInfo.u8SPSDataOffset;
				DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA MKV][SPS_PPS] Parse SPS PPS Info!\r\n"));
			} else
				prCfaMkv->u8Ca = prCfaMkv->rVidStmInfo.u8CodecPrivOfst;
		}
		prCfaMkv->rVidStmInfo.u8CodecPrivLen = prCfaMkv->rVidStmInfo.u8CodecLen;
		ToNextState(pvSptHdl, prCfaMkv, prCfaMkv->rVidStmInfo.u8CodecPrivLen,
					CFA_MKV_ST_TX_SEQUENCE_INFO);
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV] Parse SequenceHeader!\r\n"));
	} else if ((prCfaMkv->rVidStmInfo.u4DivxHdrLen != 0) &&
		(prCfaMkv->u4PrsFlg  & CFA_PRS_BIT_STRM_TYPE_V) &&
		(prCfaMkv->rRange.u8TargetTimeCode != 0)) {
		prCfaMkv->u8Ca = prCfaMkv->rRange.u8VidStartOfst;
		ToNextState(pvSptHdl, prCfaMkv, prCfaMkv->rVidStmInfo.u4DivxHdrLen, CFA_MKV_ST_TX_DIXV_HDR);
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV] Parse divx SequenceHeader!\r\n"));
	} else if ((prCfaMkv->rRange.u8TargetTimeCode != 0) &&
			(CFA_VID_MPEG4 == prCfaMkv->rVidStmInfo.eVidCodec) &&
			(!prCfaMkv->rVidStmInfo.fgTxMpeg4VOLHeader) &&
			(prCfaMkv->rVidStmInfo.u4Mpeg4HeaderLen != 0)) {
		prCfaMkv->u8Ca = prCfaMkv->rRange.u8SubStartOfst;
		ToNextState(pvSptHdl, prCfaMkv, CFA_MKV_MP4_SEQ_HDR_LEN, CFA_MKV_ST_MPEG4_VOL_HEADER);
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV] Parse MPEG4 VOL Header!\r\n"));
	} else {
		//prCfaMkv->rVidStmInfo.u4DivxHdrLen = 0;
		 /*video,audio,sub's start address is the same*/
		prCfaMkv->u8Ca = prCfaMkv->rRange.u8SubStartOfst;
		prCfaMkv->fgTxFirst = TRUE;

		ToNextState(pvSptHdl, prCfaMkv, CFA_MKV_HDR_READ_BYTES, CFA_MKV_ST_SC_ANA);

		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT, TEXT("[CFA MKV] Parse normal!\r\n"));
	}

	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
		TEXT("[CFA MKV] Turn On at 0X%llX[%lld]!\r\n"), prCfaMkv->u8Ca, prCfaMkv->u8Ca);
	MM_RETURN(RET_DMX_OK);

}

static MRESULT CfaMkvSetRange(void *pvSptHdl, void *pvRange, void *pvPrivData, bool fgIsUserMem)
{
	CfaMkvInst *prCfaMkv = NULL;
	CfaMkvRange_T rCfaMkvRange;

	mm_memset((void *)&rCfaMkvRange, 0, (u32)sizeof(rCfaMkvRange));

	if (NULL == pvRange || NULL == pvPrivData) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV] CfaMkvSetRange:: The parameter is invalid!\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	prCfaMkv = (CfaMkvInst*)pvPrivData;

	MMATE_CHECK_POINTER(prCfaMkv);
	MMATE_CHECK_STRUCT(prCfaMkv->rCurBlock);
	MMATE_CHECK_STRUCT(prCfaMkv->rVidStmInfo);

	prCfaMkv->u4ForwardRefTr = 0;
	prCfaMkv->u8ForwardRefPts = 0;
	prCfaMkv->u8BackwardRefPts = 0;
	prCfaMkv->u4BackwardRefTr = 0;
#ifdef MM_ATE_CHECK
	if (0 != mm_copy_from_user(&(rCfaMkvRange.u8TargetTimeCode), &(((CfaMkvRange_T *)pvRange)->u8TargetTimeCode),
			sizeof(CfaMkvRange_T) - 2 * sizeof(u32))) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV] %s line %d failed in mm_copy_from_user\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}
#else
	if (0 != mm_copy_from_user(&rCfaMkvRange, pvRange, sizeof(CfaMkvRange_T))) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV] %s line %d failed in mm_copy_from_user\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}
#endif

	MMATE_INIT_STRUCT(prCfaMkv->rRange);

	prCfaMkv->rRange.u4AudBlockNo = rCfaMkvRange.u4AudBlockNo;
	prCfaMkv->rRange.u4SubBlockNo = rCfaMkvRange.u4SubBlockNo;
	prCfaMkv->rRange.u4VidBlockNo = rCfaMkvRange.u4VidBlockNo;
	prCfaMkv->rRange.u8AudEndOfst = rCfaMkvRange.u8AudEndOfst;
	prCfaMkv->rRange.u8AudStartOfst = rCfaMkvRange.u8AudStartOfst;
	prCfaMkv->rRange.u8SubEndOfst = rCfaMkvRange.u8SubEndOfst;
	prCfaMkv->rRange.u8SubStartOfst = rCfaMkvRange.u8SubStartOfst;
	prCfaMkv->rRange.u8TargetTimeCode = rCfaMkvRange.u8TargetTimeCode;
	prCfaMkv->rRange.u8VidEndOfst = rCfaMkvRange.u8VidEndOfst;
	prCfaMkv->rRange.u8VidStartOfst = rCfaMkvRange.u8VidStartOfst;
	prCfaMkv->rRange.u8VidTimeCode = rCfaMkvRange.u8VidTimeCode;

	prCfaMkv->u8LastVPts =
		MS2PTS(rCfaMkvRange.u8TargetTimeCode * prCfaMkv->u8TimeCodeScale / MKV_NS_TO_MS);
	prCfaMkv->u8LastAPts = prCfaMkv->u8LastVPts;

	prCfaMkv->rCurBlock.fgDataReady = FALSE;
	prCfaMkv->rCurBlock.fgDurationReady = FALSE;
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON, TEXT("[CFA MKV] Set Range!\r\n"));
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA MKV] Range:Video TimeCode: 0x%x\r\n"), (u32)rCfaMkvRange.u8VidTimeCode);
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA MKV] Range:Target TimeCode: 0x%x\r\n"), (u32)rCfaMkvRange.u8TargetTimeCode);
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA MKV] Range:Video Offset: 0x%x\r\n"), (u32)rCfaMkvRange.u8VidStartOfst);
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA MKV] Range:Video BlockNo: %d\r\n"), rCfaMkvRange.u4VidBlockNo);
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA MKV] Range:Audio Offset: 0x%x\r\n"), (u32)prCfaMkv->rRange.u8AudStartOfst);
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA MKV] Range:Audio BlockNo: %d\r\n"), rCfaMkvRange.u4AudBlockNo);
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA MKV] Range:sub Offset: 0x%x\r\n"), (u32)prCfaMkv->rRange.u8SubStartOfst);

	MM_RETURN(RET_DMX_OK);

}

static MRESULT CfaMkvUninit(void *pvSptHdl, void *pvCfaPrivData)
{
	CfaMkvInst *prCfaMkv = (CfaMkvInst *)pvCfaPrivData;
	void *pvPointer = NULL;
	u8  uAudNum = 0;
	if (NULL == prCfaMkv) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV] prCfaMkv is NULL!\r\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}

	MMATE_CHECK_POINTER(prCfaMkv);
	MMATE_CHECK_STRUCT(prCfaMkv->rCurBlock);
	MMATE_CHECK_STRUCT(prCfaMkv->rVidStmInfo);

	if (NULL != prCfaMkv->pu1Wvc1Header)
		DMX_FreeHwMemory(prCfaMkv->pu1Wvc1Header);

	if (NULL != prCfaMkv->pu1Mp4SeqHdr)
		DMX_FreeHwMemory(prCfaMkv->pu1Mp4SeqHdr);

	if (NULL != prCfaMkv->pTotalVorbisHeadPage) {
		DMX_FreeHwMemory(prCfaMkv->pTotalVorbisHeadPage);
		prCfaMkv->pTotalVorbisHeadPage = NULL;
	}

	for (uAudNum = 0; uAudNum < (u8)MAX_NS_MKV_AUD; uAudNum++) {
		if (NULL != prCfaMkv->arAudStmInfo[uAudNum].auAacHeader) {
			DMX_FreeHwMemory(prCfaMkv->arAudStmInfo[uAudNum].auAacHeader);
			prCfaMkv->arAudStmInfo[uAudNum].auAacHeader = NULL;
		}

		if (NULL != prCfaMkv->arAudStmInfo[uAudNum].pucAudCodecPrivData) {
			DMX_FreeHwMemory(prCfaMkv->arAudStmInfo[uAudNum].pucAudCodecPrivData);
			prCfaMkv->arAudStmInfo[uAudNum].pucAudCodecPrivData = NULL;
		}

		if (NULL != prCfaMkv->arAudStmInfo[uAudNum].pucAudCodecVorbisID) {
			DMX_FreeHwMemory(prCfaMkv->arAudStmInfo[uAudNum].pucAudCodecVorbisID);
			prCfaMkv->arAudStmInfo[uAudNum].pucAudCodecVorbisID = NULL;
		}

		if (NULL != prCfaMkv->arAudStmInfo[uAudNum].rCfaMkvHeader.auHeader) {
			DMX_FreeHwMemory(prCfaMkv->arAudStmInfo[uAudNum].rCfaMkvHeader.auHeader);
			prCfaMkv->arAudStmInfo[uAudNum].rCfaMkvHeader.auHeader = NULL;
		}
	}

	if (NULL != prCfaMkv->rVidStmInfo.rCfaMkvHeader.auHeader) {
		DMX_FreeHwMemory(prCfaMkv->rVidStmInfo.rCfaMkvHeader.auHeader);
		prCfaMkv->rVidStmInfo.rCfaMkvHeader.auHeader = NULL;
	}
	if (NULL != prCfaMkv->rVidStmInfo.pucDivxHdrBuf) {
		DMX_FreeHwMemory(prCfaMkv->rVidStmInfo.pucDivxHdrBuf);
		prCfaMkv->rVidStmInfo.pucDivxHdrBuf = NULL;
	}
	if (NULL != prCfaMkv->rVidStmInfo.pucCodecPrivBuf) {
		DMX_FreeHwMemory(prCfaMkv->rVidStmInfo.pucCodecPrivBuf);
		prCfaMkv->rVidStmInfo.pucCodecPrivBuf = NULL;
	}
	if (NULL != prCfaMkv->pucAvcPPSBuf) {
		DMX_FreeHwMemory(prCfaMkv->pucAvcPPSBuf);
		prCfaMkv->pucAvcPPSBuf = NULL;
	}

	if (NULL != prCfaMkv->pucAvcSPSBuf) {
		DMX_FreeHwMemory(prCfaMkv->pucAvcSPSBuf);
		prCfaMkv->pucAvcSPSBuf = NULL;
	}

	if (NULL != prCfaMkv->pucAvcHevcStartCode) {
		DMX_FreeHwMemory(prCfaMkv->pucAvcHevcStartCode);
		prCfaMkv->pucAvcHevcStartCode = NULL;
	}

	if (NULL != prCfaMkv->pucWVC1SpecData) {
		DMX_FreeHwMemory(prCfaMkv->pucWVC1SpecData);
		prCfaMkv->pucWVC1SpecData = NULL;
	}

	pvPointer = (void *)prCfaMkv;
	if (NULL != pvPointer) {
		DMX_FreeMemory(pvPointer);
		pvPointer = NULL;
	}
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT, TEXT("[CFA MKV] Uninit OK!\r\n"));

	MM_RETURN(RET_DMX_OK);
}


static MRESULT CfaMkvFillAUInfo(void *pvSptHdl, void *pvAUInfo, void *pvAUExtInfo, void *pvPrivData)
{
	CfaMkvInst *prCfaMkv = NULL;

	prCfaMkv = (CfaMkvInst *)pvPrivData;

	if ((NULL == pvAUInfo) || (NULL == prCfaMkv)) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV]: CfaMkvFillAUInfo:: The parameter is invalid!!\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	MMATE_CHECK_POINTER(prCfaMkv);
	MMATE_CHECK_STRUCT(prCfaMkv->rCurBlock);
	MMATE_CHECK_STRUCT(prCfaMkv->rRange);
	MMATE_CHECK_STRUCT(prCfaMkv->rVidStmInfo);

	switch (prCfaMkv->eCurPrsStrm) {
	case CFA_PRS_BIT_STRM_TYPE_V: {
			prCfaMkv->u8KeyClusterOfst = 0;
			if (fgIsIType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) {
				prCfaMkv->u8KeyTimeCode = prCfaMkv->rCurBlock.u8TimeCode;
				prCfaMkv->u8KeyClusterOfst = prCfaMkv->rCurCluster.u8ClusterOfst;
				prCfaMkv->u4KeyBlockNo = prCfaMkv->rCurBlock.u4BlockNo;

				/*DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA MKV] u8Ca is 0x%llx, I frame!!!\r\n"),prCfaMkv->u8Ca);*/
			}

			if (fgIsIType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType))
				prCfaMkv->fgStartAjustB = FALSE;
			else if (fgIsBType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) {
				/*DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA MKV] u8Ca is 0x%llx, B frame!!!\r\n"),prCfaMkv->u8Ca);*/
			} else if (fgIsPType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) {
				/*DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA MKV] u8Ca is 0x%llx, P frame!!!\r\n"),prCfaMkv->u8Ca);*/
				prCfaMkv->fgStartAjustB = FALSE;
			} else
				DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_AUINFO, TEXT("[CFA MKV] Unknown Frame\r\n"));

			if ((prCfaMkv->rCurBlock.u8Pts != prCfaMkv->rCurBlock.u8LastPts) &&
				(prCfaMkv->rCurBlock.u8Pts != 0))
				prCfaMkv->rCurBlock.u8LastBlockPTS = prCfaMkv->rCurBlock.u8LastPts;

			if ((prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_H264) &&
				(!prCfaMkv->rAbnormalFlags.fgFourCCH264H265))
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Offset = prCfaMkv->rCurAvc.u8PayloadOffset;
			else
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Offset = prCfaMkv->rCurBlock.u8DataOfst;

			DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_AUINFO,
				TEXT("\r[CFA MKV] AU offset is 0x%llx[%lld]!!!\r\n"),
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Offset,
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Offset);

			if ((CFA_VID_RV30 == prCfaMkv->rVidStmInfo.eVidCodec) ||
				(CFA_VID_RV40 == prCfaMkv->rVidStmInfo.eVidCodec)) {
				int i = 0;

				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4RMSliceNum = prCfaMkv->rSliceInf.u1TotalSliceNum;
				for (; i < prCfaMkv->rSliceInf.u1TotalSliceNum; ++i) {
					((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.auRM4SliceSize[i] =
						prCfaMkv->rSliceInf.rSliceInf[i].u2SliceElemSize;
				}

				dmx_memset(&(prCfaMkv->rSliceInf), 0, sizeof(prCfaMkv->rSliceInf));
			}
			/*when one block contains PB frame,PTS of P frame should be reset to
			DMX_INVALID_UINT64,and PTS of B should be the PTS of P frame*/
			if ((prCfaMkv->rCurBlock.u8Pts == prCfaMkv->rCurBlock.u8LastPts) &&
				(prCfaMkv->eLastPicType == CFA_MKV_PIC_P)) {
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8PrevPTS = INVALID_TIMESTAMP;
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = prCfaMkv->rCurBlock.u8Pts;
			}
			/*when decoder order is P-B or I-B,and PTS of P or I frame are less than
			B frame,we should reset PTS (add case of lasts BB..)*/
			else if (((prCfaMkv->eLastPicType == CFA_MKV_PIC_P) ||
				(prCfaMkv->eLastPicType == CFA_MKV_PIC_I))
				&& (fgIsBType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType))
				&& (prCfaMkv->rCurBlock.u8Pts > prCfaMkv->rCurBlock.u8LastPts)) {
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8PrevPTS = INVALID_TIMESTAMP;
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = prCfaMkv->rCurBlock.u8LastPts;
				prCfaMkv->fgStartAjustB = TRUE;
			} else
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = prCfaMkv->rCurBlock.u8Pts;

			if ((CFA_VID_RV30 == prCfaMkv->rVidStmInfo.eVidCodec) ||
				(CFA_VID_RV40 == prCfaMkv->rVidStmInfo.eVidCodec)) {
				u64 u8RealPTS = 0;

				u8RealPTS = prCfaMkv->rCurBlock.u8TimeCode;
				u8RealPTS *= CFA_MKV_SYS_CLK;
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8PrevPTS = 0;

				if (CFA_PTM_RM_TRUEBPIC == prCfaMkv->eCurPicType)
					((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = prCfaMkv->u8PrsPts;
				else
					((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = u8RealPTS;

				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4Duration = 0;
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4PrevDuration = INVALID_DURATION;

			}

			((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.eDiscType = DT_DATADISC;

			/*fill video keyframe*/

			prCfaMkv->rCurBlock.u8LastPts = prCfaMkv->rCurBlock.u8Pts;
			if (fgIsIType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType))
				prCfaMkv->eLastPicType = CFA_MKV_PIC_I;
			else if (fgIsBType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType))
				prCfaMkv->eLastPicType = CFA_MKV_PIC_B;
			else if (fgIsPType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType))
				prCfaMkv->eLastPicType = CFA_MKV_PIC_P;
			else
				prCfaMkv->eLastPicType = CFA_MKV_PIC_UNKNOWN;

			if (prCfaMkv->rCurBlock.fgDiscardable) {
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = INVALID_TIMESTAMP;
				DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_AUINFO,
					TEXT("[CFA MKV] Cur Block is discardable, so set PTS to -1\r\n"));
			}

			break;
		}

	case CFA_PRS_BIT_STRM_TYPE_A: {
			((AU_AUDIO *)pvAUInfo)->rAUInfo.rInfo.u8Pts = prCfaMkv->rCurBlock.u8Pts;

			if (prCfaMkv->rCurBlock.u8TrackNum == prCfaMkv->u8CurAId) {
				prCfaMkv->u8AudioTimeCode = prCfaMkv->rCurBlock.u8TimeCode;
				prCfaMkv->u8AudioClusterOfst = prCfaMkv->rCurCluster.u8ClusterOfst;
				prCfaMkv->u4AudioBlockNo = prCfaMkv->rCurBlock.u4BlockNo;

			}
			DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_AUINFO,
				TEXT("[CFA MKV][FF_AUD] Fill one AU, pts is %lld\r\n"), prCfaMkv->rCurBlock.u8TimeCode);
			break;
		}

	case CFA_PRS_BIT_STRM_TYPE_SP: {
			((AU_SP *)pvAUInfo)->rAUInfo.rInfo.u8StartPts = prCfaMkv->rCurBlock.u8Pts;
			((AU_SP *)pvAUInfo)->rAUInfo.rInfo.u8EndPts = prCfaMkv->rCurBlock.u8EndPts;

			if (prCfaMkv->rCurBlock.ucStrmIdx >= MAX_NS_MKV_SP) {
				DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA MKV] ucStrmIdx is %d\r\n"), prCfaMkv->rCurBlock.ucStrmIdx);
				Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, TRUE, GAU_E_FAIL);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			break;
		}
	default:
		DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_AUINFO, TEXT("[CFA MKV] Fill AU Error!\r\n"));
		break;
	}

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaMkvGetCurPos(void *pvSptHdl, void *pvCurPos, void *pvPrivData)
{

	CfaMkvInst *prCfaMkv;
	u64 *pvu8 = pvCurPos;

	if (NULL == pvPrivData) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV] line %d Send EOS ,pvPrivData is NULL! \r\n"), DMX_LINE_NO);
		MM_RETURN(RET_DMX_EXT_NO_MEM);
	}

	prCfaMkv = (CfaMkvInst *)pvPrivData;
	*pvu8 = prCfaMkv->u8Ca;

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaMkvTxAudHDRInfo(void *pvSptHdl, u32 u4TxUID, void *pvPrivData)
{
	CfaMkvInst *prCfaMkv = NULL;
	MRESULT mrResult = RET_DMX_OK;

	if (NULL == pvPrivData) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV] line %d Send EOS ,pvPrivData is NULL! \r\n"), DMX_LINE_NO);
		MM_RETURN(RET_DMX_EXT_NO_MEM);
	}
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON, TEXT("[CFA MKV] TxAudHDRInfo !\r\n"));
	prCfaMkv = (CfaMkvInst *)pvPrivData;

	if (prCfaMkv->ucCurAIndex >= MAX_NS_MKV_AUD) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA MKV] ucCurAIndex is %d\r\n"), prCfaMkv->ucCurAIndex);
		Spt4CfaFinishedEx(pvSptHdl, prCfaMkv->u8Ca, TRUE, GAU_E_FAIL);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if ((DMX_INVALID_UINT32 == u4TxUID) &&
		((CFA_AUD_DRV_FMT_AAC == prCfaMkv->arAudStmInfo[prCfaMkv->ucCurAIndex].eAudCodec) ||
		(CFA_AUD_DRV_FMT_VORBIS == prCfaMkv->arAudStmInfo[prCfaMkv->ucCurAIndex].eAudCodec))) {
		prCfaMkv->arAudStmInfo[prCfaMkv->ucCurAIndex].fgHaveTxHeader = FALSE;
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (prCfaMkv->u8CurAId != u4TxUID)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	if ((CFA_AUD_DRV_FMT_AAC != prCfaMkv->arAudStmInfo[prCfaMkv->ucCurAIndex].eAudCodec) &&
		(CFA_AUD_DRV_FMT_VORBIS != prCfaMkv->arAudStmInfo[prCfaMkv->ucCurAIndex].eAudCodec)) {
		MM_RETURN(RET_DMX_UNSUPPORT);
	} else {
		if (CFA_AUD_DRV_FMT_VORBIS == prCfaMkv->arAudStmInfo[prCfaMkv->ucCurAIndex].eAudCodec) {
			mrResult = Spt4CfaBuf2AFifo(pvSptHdl, prCfaMkv->pTotalVorbisHeadPage,
				prCfaMkv->u8TotalVorbisHeadPageSize,
				u4TxUID, prCfaMkv->arAudStmInfo[prCfaMkv->ucCurAIndex].eAudCodec);
		} else {
			mrResult = Spt4CfaBuf2AFifo(pvSptHdl, prCfaMkv->arAudStmInfo[prCfaMkv->ucCurAIndex].auAacHeader,
				prCfaMkv->arAudStmInfo[prCfaMkv->ucCurAIndex].uAacHeaderSize,
				u4TxUID, prCfaMkv->arAudStmInfo[prCfaMkv->ucCurAIndex].eAudCodec);
		}
		MM_RETURN(mrResult);
	}
}

static MRESULT CfaMkvSetJumpRange(void *pvSptHdl, void *pvJmpRange, void *pvPrivData)
{
	CfaMkvInst *prCfaMkv = NULL;

	CfaMkvKeyFrameRange_T *prCfaMkvKeyFrameRange = NULL;
	CfaMkvKeyFrameRange_T rCfaMkvKeyFrameRange;

	mm_memset(&rCfaMkvKeyFrameRange, 0, sizeof(rCfaMkvKeyFrameRange));

	if ((NULL == pvPrivData) || (NULL == pvJmpRange)) {
		DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_MKV] line %d Send EOS ,pvPrivData is NULL or pvJmpRange is NULL! \r\n"), DMX_LINE_NO);
		MM_RETURN(RET_DMX_EXT_NO_MEM);
	}
	if (0 != mm_copy_from_user(&rCfaMkvKeyFrameRange,
		pvJmpRange, sizeof(CfaMkvKeyFrameRange_T))) {
		DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
			TEXT("[CFA_AUDIO] %s line %d failed in mm_copy_from_user\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}
	prCfaMkvKeyFrameRange = &rCfaMkvKeyFrameRange;
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT, TEXT("[CFA MKV] Entry CfaMkvSetJumpRange!\r\n"));
	prCfaMkv = (CfaMkvInst *)pvPrivData;

	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA MKV] CfaMkvSetJumpRange::u8VidStartOfst[0x%x]\r\n"),
		prCfaMkvKeyFrameRange->rCfaRangeInfo.u8VidStartOfst);
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA MKV] CfaMkvSetJumpRange::u8AudStartOfst[0x%x]\r\n"),
		prCfaMkvKeyFrameRange->rCfaRangeInfo.u8AudStartOfst);
	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA MKV] CfaMkvSetJumpRange::u8SubStartOfst[0x%x]\r\n"),
		prCfaMkvKeyFrameRange->rCfaRangeInfo.u8SubStartOfst);

	prCfaMkv->u4RWUnitAULen = prCfaMkvKeyFrameRange->u4RWUnitAULen;
	prCfaMkv->fgTxFirst = TRUE;
	prCfaMkv->fgFinishRWAU = FALSE;

	CfaMkvSetRange(pvSptHdl, (void *)(&(prCfaMkvKeyFrameRange->rCfaRangeInfo)), pvPrivData, FALSE);

	MMATE_CHECK_POINTER(prCfaMkv);

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaMkvGetParamSize(void *pvSptHdl, u32 u4ParamID,
	void  *pvPrivData, void  *pvCfaParam, u32 u4CfaParamSz)
{
	MRESULT mrResult = RET_DMX_OK;

	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA MKV] Entry CfaMkvGetParamSize!\r\n"));

	switch (u4ParamID) {
	case CFA_PARAM_ID_JUMP_INFO_SIZE: {
		if ((NULL == pvCfaParam) || ((u4CfaParamSz) < sizeof(u32)))
			mrResult = RET_DMX_PARAM_WRONG;
		else {
			u32 *pu4Tmp = (u32 *)pvCfaParam;
			*pu4Tmp = sizeof(CfaMkvKeyFrameRange_T);
			DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
				TEXT("[CFA MKV] CfaMkvKeyFrameRange_T is %d(Bytes)!\r\n"), *pu4Tmp);
		}
		break;
	}

	default:
		mrResult = RET_DMX_PARAM_WRONG;
		break;
	}

	MM_RETURN(mrResult);
}

static MRESULT CfaMkvProcCliCmd(void *pvSptHdl, 
	E_DMX_CFA_CLI_TYPE_T eCliType,   ///< [IN] Cfa Cli Command
	u32 arg1,
	u32 arg2,
	u32 arg3,
	const char *szParam,
	void *pvPrivData)
{
	MRESULT mrRet = RET_DMX_OK;
	CfaMkvInst *prCfaMkv = NULL;

	prCfaMkv = (CfaMkvInst *) pvPrivData;

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

			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("CfaMkvProcCliCmd -- fgEnable: %d, Loglvl: %d, ModLogLvl: 0x%08x \r\n"),
				arg1, arg2, arg3);

			DmxLogEnable(fgEnable, arg2, CFA_MKV_LOG_DEFAULT, arg3);
		}
		break;
	case DMX_CFA_CLI_CMD_DUMP_INFO:
		{
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("Cfa MKV Instance(prCfaMkv:%p)")
				TEXT(" Info list as follow: \r\n"),
				prCfaMkv);
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("Current Analyse State is %d \r\n"),
				prCfaMkv->eCurState);
			DmxLogT(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("Current Analyse Position is 0x%08x%08x, "),
				(u32) ((prCfaMkv->u8Ca) >> 32), (u32) (prCfaMkv->u8Ca));
		}
		break;
	default:
		break;
	}

	MM_RETURN(mrRet);
}


CfaIntf _rMkvCfaIntf = {
	&CfaMkvInit,
	&CfaMkvUninit,
	&CfaMkvSetRange,
	&CfaMkvEnableStrm,
	&CfaMkvSetStrmInf,
	&CfaMkvTurnOn,
	&CfaMkvTxDone,
	&CfaMkvGetCurPos,
	NULL,
	&CfaMkvConfigure,
	NULL,
	NULL,
	NULL,
	NULL,
	&CfaMkvFillAUInfo,
	&CfaMkvTxAudHDRInfo,
	NULL,
	&CfaMkvSetJumpRange,
	&CfaMkvGetParamSize,
	&CfaMkvProcCliCmd
	#ifdef CONFIG_COMPAT
	,&CfaMkvProcCompat
	#endif
};

static MRESULT CfaMkvInitPara(CfaMkvInst *prCfaMkv)
{
	u32 i = 0;

	prCfaMkv->eCurState = CFA_MKV_ST_IDLE;
	prCfaMkv->u8Ca = 0;
	prCfaMkv->u8LastCa = 0;
	dmx_memset((void *)&(prCfaMkv->rCurBlock), 0, sizeof(CfaMkvBlockInfo));
	dmx_memset((void *)&(prCfaMkv->rCurGroup), 0, sizeof(CfaMkvBlockGroupInfo));
	dmx_memset((void *)&(prCfaMkv->rCurCluster), 0, sizeof(CfaMkvClusterInfo));
	prCfaMkv->u8KeyClusterOfst = DMX_INVALID_UINT64;
	prCfaMkv->u8KeyTimeCode = DMX_INVALID_UINT64;
	prCfaMkv->u8AudioClusterOfst = DMX_INVALID_UINT64;
	prCfaMkv->u8AudioTimeCode = DMX_INVALID_UINT64;
	prCfaMkv->u4AudioBlockNo = DMX_INVALID_UINT32;
	prCfaMkv->u4KeyBlockNo = DMX_INVALID_UINT32;
	prCfaMkv->fgDemuxError = FALSE;
	prCfaMkv->u8HdrLen = 0;
	prCfaMkv->fgCheckRangeCluster = TRUE;
	prCfaMkv->i4AudAULenWithoutVid = CFA_MKV_AUD_AULEN_WITHOUT_VID;
	prCfaMkv->fgIfNeedRebuf = TRUE;

	if (prCfaMkv->pu1Wvc1Header) {
		prCfaMkv->pu1Wvc1Header[0] = 0x00;
		prCfaMkv->pu1Wvc1Header[1] = 0x00;
		prCfaMkv->pu1Wvc1Header[2] = 0x01;
		prCfaMkv->pu1Wvc1Header[3] = 0x0D;
	}

	if (prCfaMkv->pu1Mp4SeqHdr) {
		prCfaMkv->pu1Mp4SeqHdr[0] = 0x00;
		prCfaMkv->pu1Mp4SeqHdr[1] = 0x00;
		prCfaMkv->pu1Mp4SeqHdr[2] = 0x01;
		prCfaMkv->pu1Mp4SeqHdr[3] = 0x20;
		prCfaMkv->pu1Mp4SeqHdr[4] = 0x00;
		prCfaMkv->pu1Mp4SeqHdr[5] = 0x44;
		prCfaMkv->pu1Mp4SeqHdr[6] = 0x08;
		prCfaMkv->pu1Mp4SeqHdr[7] = 0xBF;
		prCfaMkv->pu1Mp4SeqHdr[8] = 0xFF;
		prCfaMkv->pu1Mp4SeqHdr[9] = 0xD0;
		prCfaMkv->pu1Mp4SeqHdr[10] = 0x00;
		prCfaMkv->pu1Mp4SeqHdr[11] = 0x40;
		prCfaMkv->pu1Mp4SeqHdr[12] = 0x01;
		prCfaMkv->pu1Mp4SeqHdr[13] = 0x46;
		prCfaMkv->pu1Mp4SeqHdr[14] = 0x00;
	}

#if CONFIG_CFA_MKV_SUPPORT_DRM
	prCfaMkv->rDRMInf.fgTrackDataExist = FALSE;
	prCfaMkv->rDRMInf.fgDrmExist	= FALSE;
	prCfaMkv->rDRMInf.u2KeyIdx		 = 0;
	prCfaMkv->rDRMInf.u4EncryptOfst = 0;
	prCfaMkv->rDRMInf.u4EncryptLen	= 0;

	prCfaMkv->rDivxDRMInf.fgOn = FALSE;
	prCfaMkv->rDivxDRMInf.u8DecryptStOfst = DMX_INVALID_UINT64;
	prCfaMkv->rDivxDRMInf.u4DecryptLen = 0;
	prCfaMkv->rDivxDRMInf.u2FrameKeyIdx = DMX_DIVXDRM_INVALID_FRAMEIDX;
#endif

	DmxLogD(DMX_MOD_CFA_MKV, CFA_MKV_LOG_COMMON,
		TEXT("[CFA MKV] Init internal parameters!\r\n"));

	for (i = 0; i < prCfaMkv->u4AudNum; i++)
		prCfaMkv->arAudStmInfo[i].fgHaveTxHeader = FALSE;

	if ((prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_H265) &&
		(NULL == prCfaMkv->pucAvcHevcStartCode)) {
		DMX_NewHwMemory(CFA_MKV_AVC_STARTCODE_LEN, prCfaMkv->pucAvcHevcStartCode);
		if (NULL == prCfaMkv->pucAvcHevcStartCode) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA MKV] alloc prCfaMkv->pucAvcHevcStartCode fail\r\n"));
			MM_RETURN(RET_DMX_NO_MEM);
		} else
			dmx_memset((void *)prCfaMkv->pucAvcHevcStartCode, 0, CFA_MKV_AVC_STARTCODE_LEN);
		prCfaMkv->pucAvcHevcStartCode[0] = 0x00;
		prCfaMkv->pucAvcHevcStartCode[1] = 0x00;
		prCfaMkv->pucAvcHevcStartCode[2] = 0x00;
		prCfaMkv->pucAvcHevcStartCode[3] = 0x01;
	}
	if ((prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_H264)
		&& (NULL == prCfaMkv->pucAvcPPSBuf) &&
		(NULL == prCfaMkv->pucAvcSPSBuf) &&
		(NULL == prCfaMkv->pucAvcHevcStartCode)) {/*add by guoqing yang for H264 */
		DMX_NewHwMemory(CFA_MKV_AVC_PPS_BUF_LEN, prCfaMkv->pucAvcPPSBuf);
		if (NULL == prCfaMkv->pucAvcPPSBuf) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA MKV] alloc prCfaMkv->pucAvcPPSBuf fail\r\n"));
			MM_RETURN(RET_DMX_NO_MEM);
		} else
			dmx_memset((void *)prCfaMkv->pucAvcPPSBuf, 0 , CFA_MKV_AVC_PPS_BUF_LEN);

		prCfaMkv->pucAvcPPSBuf[0] = 0x00;
		prCfaMkv->pucAvcPPSBuf[1] = 0x00;
		prCfaMkv->pucAvcPPSBuf[2] = 0x00;
		prCfaMkv->pucAvcPPSBuf[3] = 0x01;

		DMX_NewHwMemory(CFA_MKV_AVC_SPS_BUF_LEN, prCfaMkv->pucAvcSPSBuf);
		if (NULL == prCfaMkv->pucAvcSPSBuf) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA MKV] alloc prCfaMkv->pucAvcSPSBuf fail\r\n"));
			DMX_FreeHwMemory(prCfaMkv->pucAvcPPSBuf);
			prCfaMkv->pucAvcPPSBuf = NULL;
			MM_RETURN(RET_DMX_NO_MEM);
		} else {
			dmx_memset((void *)prCfaMkv->pucAvcSPSBuf, 0, CFA_MKV_AVC_SPS_BUF_LEN);
		}

		prCfaMkv->pucAvcSPSBuf[0] = 0x00;
		prCfaMkv->pucAvcSPSBuf[1] = 0x00;
		prCfaMkv->pucAvcSPSBuf[2] = 0x00;
		prCfaMkv->pucAvcSPSBuf[3] = 0x01;

		DMX_NewHwMemory(CFA_MKV_AVC_STARTCODE_LEN, prCfaMkv->pucAvcHevcStartCode);
		if (NULL == prCfaMkv->pucAvcHevcStartCode) {
			DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
				TEXT("[CFA MKV] alloc prCfaMkv->pucAvcHevcStartCode fail\r\n"));
			DMX_FreeHwMemory(prCfaMkv->pucAvcPPSBuf);
			prCfaMkv->pucAvcPPSBuf = NULL;
			DMX_FreeHwMemory(prCfaMkv->pucAvcSPSBuf);
			prCfaMkv->pucAvcSPSBuf = NULL;
			MM_RETURN(RET_DMX_NO_MEM);
		} else
			dmx_memset((void *)prCfaMkv->pucAvcHevcStartCode, 0, CFA_MKV_AVC_STARTCODE_LEN);

		prCfaMkv->pucAvcHevcStartCode[0] = 0x00;
		prCfaMkv->pucAvcHevcStartCode[1] = 0x00;
		prCfaMkv->pucAvcHevcStartCode[2] = 0x00;
		prCfaMkv->pucAvcHevcStartCode[3] = 0x01;
	} else if (prCfaMkv->rVidStmInfo.eVidCodec == CFA_VID_VC1) {
		if (NULL == prCfaMkv->pucWVC1SpecData) {
			DMX_NewHwMemory(CFA_MKV_WVC1_SPECDATA_LEN, prCfaMkv->pucWVC1SpecData);
			if (NULL == prCfaMkv->pucWVC1SpecData) {
				DmxLogE(DMX_MOD_CFA_MKV, CFA_MKV_LOG_DEFAULT,
					TEXT("[CFA MKV] alloc prCfaMkv->pucWVC1SpecData fail\r\n"));
				MM_RETURN(RET_DMX_NO_MEM);
			}
		}

		dmx_memset((void *)prCfaMkv->pucWVC1SpecData, 0, CFA_MKV_WVC1_SPECDATA_LEN);
	} else {
		/*do nothing*/
	}

	prCfaMkv->fgNeedAddHeadStrip = FALSE;

	MM_RETURN(RET_DMX_OK);
}

void *CfaMkvGetInterface(void)
{
	return ((void *)(&_rMkvCfaIntf));
}
