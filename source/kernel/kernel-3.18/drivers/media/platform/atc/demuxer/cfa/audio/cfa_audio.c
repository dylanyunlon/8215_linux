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
 * @par Description
 *    CFA audio File
 *
 * @par Author_Name
 *    Qing Li
*****************************************************************************/

 /*!
  * @file       cfa_audio.c
  * @author
  * @version 1.0
  * @brief     The C file of the interface for Audio CFA
  */

#include "cfa_audio.h"
#include "mmisc.h"
#include "dmx_mem.h"
#include "cfa_macro.h"

#ifdef CONFIG_COMPAT
#include <linux/compat.h>

typedef struct {
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKStart;
#endif
	__u64 u8Sa;	/* < start file offset, 0-based */
	__u64 u8Ea;	/* < end file offset.    The byte of this offset is transferred. */
	__s32 i4Rate;
	__u32 u4SeekNum;
	__u64 u8SeekTime;
	__u32 u4Ac3FrameNo;
	__u32 u4TxUnitRange;
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKEnd;
#endif
} CfaAudioPR32;
typedef struct
{
	compat_caddr_t prAc3FrameInfo;
	__u32 u4FrameCount;
} LP_CFA_Ac3FrameInf32;

typedef struct {
	__u32 u4AudioByteRate;	/* < byte / s */
	bool fgAc3Type;
	FILE_TYPE_E eFileType;
	AVCODECID_T eAudType;
	compat_caddr_t pcPoints;
	__u32 u4NumPoint;
	__u32 u4SeekTableSz;
	__u64 u8FrameStartOfst;
	__u32 u4Duration;
	__u32 u4SampeRate;
	__u32 u4TxUnitRange;
	LP_CFA_Ac3FrameInf32 rAc3FrameInfo;
} CfaAudioCfgInf32;

typedef struct {
	CfaAudioPR32 rCfaAudioRange;
	__u32 u4TxUnitKeyFrmRange;
} CfaAudioKeyFrmRange_T32;

static long CfaAudioCompatRangeInfo(CfaAudioPR __user *usr_ptr,
  CfaAudioPR32 __user *usr_ptr32)
{

#ifdef MM_ATC_CHECK
	if (copy_in_user(&(usr_ptr->u4MMATECHKStart), &(usr_ptr32->u4MMATECHKStart), sizeof(__u32)))
		return -EFAULT;
#endif

	if (copy_in_user(&(usr_ptr->u8Sa), &(usr_ptr32->u8Sa), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8Ea), &(usr_ptr32->u8Ea), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->i4Rate), &(usr_ptr32->i4Rate), sizeof(__s32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4SeekNum), &(usr_ptr32->u4SeekNum), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u8SeekTime), &(usr_ptr32->u8SeekTime), sizeof(__u64)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4Ac3FrameNo), &(usr_ptr32->u4Ac3FrameNo), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4TxUnitRange), &(usr_ptr32->u4TxUnitRange), sizeof(__u32)))
		return -EFAULT;

#ifdef MM_ATC_CHECK
	if (copy_in_user(&(usr_ptr->u4MMATECHKEnd), &(usr_ptr32->u4MMATECHKEnd), sizeof(__u32)))
		return -EFAULT;
#endif

	return 0;
}

static long CfaAudioCompatRange(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaAudioPR __user *usr_ptr = NULL;
	CfaAudioPR32 __user *usr_ptr32 = (CfaAudioPR32 __user *)prInfo->usr_ptr32;
	long ret = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaAudioPR32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	usr_ptr = (CfaAudioPR *)compat_alloc_user_space(sizeof(CfaAudioPR));

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
		TEXT("%s line %d fail in alloc compat user space.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaAudioPR));
	ret = CfaAudioCompatRangeInfo(usr_ptr, usr_ptr32);
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
		TEXT("%s line %d fail in CfaAudioCompatRangeInfo.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}

	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaAudioPR);

	return 0;
}

static long CfaAudioCompatJumpRange(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaAudioKeyFrmRange_T __user *usr_ptr = NULL;
	CfaAudioKeyFrmRange_T32 __user *usr_ptr32 = (CfaAudioKeyFrmRange_T32 __user *)prInfo->usr_ptr32;
	long ret = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaAudioKeyFrmRange_T32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	usr_ptr = (CfaAudioKeyFrmRange_T *)compat_alloc_user_space(sizeof(CfaAudioKeyFrmRange_T));

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
		TEXT("%s line %d fail in alloc compat user space.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaAudioKeyFrmRange_T));
	ret = CfaAudioCompatRangeInfo(&(usr_ptr->rCfaAudioRange),&(usr_ptr32->rCfaAudioRange));
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
		TEXT("%s line %d fail in CfaAudioCompatRangeInfo.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}
	if (copy_in_user(&(usr_ptr->u4TxUnitKeyFrmRange), &(usr_ptr32->u4TxUnitKeyFrmRange), sizeof(__u32)))
		return -EFAULT;


	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaAudioKeyFrmRange_T);

	return 0;
}

static long CfaAudioCompatAC3Info(LP_CFA_Ac3FrameInf __user *usr_ptr,
  LP_CFA_Ac3FrameInf32 __user *usr_ptr32, __u8 **ppu1NextBufAddr, __u32 *pu4Sz, __u32 u4TotalSz)
{
	compat_caddr_t compatSeqHdr = 0;
	if (copy_from_user(&(usr_ptr->u4FrameCount), &(usr_ptr32->u4FrameCount), sizeof(__u32)))
		return -EFAULT;

	if ((0 < usr_ptr32->u4FrameCount) && (0 == usr_ptr32->prAc3FrameInfo)) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("%s line %d fail for no ac3 frame info, but header len(%d) > 0.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr->u4FrameCount);
		return -EINVAL;
	}

	if (0 != usr_ptr32->prAc3FrameInfo) {
		LP_AC3_FRAME_INFO *prAc3FrameInfo = NULL;

		usr_ptr->prAc3FrameInfo = *ppu1NextBufAddr;

		if (NULL == usr_ptr->prAc3FrameInfo) {
			DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
			return -ENOMEM;
		}
		mm_memset(usr_ptr->prAc3FrameInfo, 0, sizeof(LP_AC3_FRAME_INFO) * usr_ptr->u4FrameCount);
		*ppu1NextBufAddr += CFA_ALIGN_SZ((__u32)(sizeof(LP_AC3_FRAME_INFO) * usr_ptr->u4FrameCount), sizeof(uintptr_t));
		*pu4Sz += CFA_ALIGN_SZ((__u32)(sizeof(LP_AC3_FRAME_INFO) * usr_ptr->u4FrameCount), sizeof(uintptr_t));
		if (*pu4Sz > u4TotalSz)
		{
			DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("%s line %d size is large than total size.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
			return -ENOMEM;
		}

		if (get_user(compatSeqHdr, &(usr_ptr32->prAc3FrameInfo)))
			return -EFAULT;
		if (0 == compatSeqHdr)
			return -EFAULT;
		prAc3FrameInfo = compat_ptr(compatSeqHdr);
		if (!access_ok(VERIFY_READ, prAc3FrameInfo,
			sizeof(LP_AC3_FRAME_INFO) * usr_ptr->u4FrameCount))
			return -EFAULT;

		if (copy_from_user((LP_AC3_FRAME_INFO __user *)usr_ptr->prAc3FrameInfo,
			prAc3FrameInfo, sizeof(LP_AC3_FRAME_INFO) * usr_ptr->u4FrameCount))
			return -EFAULT;
	}
	return 0;
}

static long CfaAudioCompatConfigCalcSz(CfaAudioCfgInf32 __user *usr_ptr32,
	__u32 *pu4OutSz)
{
	__u32 u4TotalSz = 0;
	__u32 u4HeaderLen = 0;
	__u32 i = 0;

	if ((NULL == usr_ptr32) || (NULL == pu4OutSz)) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	u4TotalSz += CFA_ALIGN_SZ((__u32)sizeof(CfaAudioCfgInf), sizeof(uintptr_t));

	if (0 != get_user(u4HeaderLen,	&(usr_ptr32->u4SeekTableSz))) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("%s line %d fail in get_user(u4SeekTableSz)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	u4TotalSz += CFA_ALIGN_SZ((__u32)u4HeaderLen, sizeof(uintptr_t));
	if (0 != get_user(u4HeaderLen,	&(usr_ptr32->rAc3FrameInfo.u4FrameCount))) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("%s line %d fail in get_user(rAc3FrameInfo.u4FrameCount)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	u4TotalSz += CFA_ALIGN_SZ((__u32)(sizeof(LP_AC3_FRAME_INFO) * u4HeaderLen), sizeof(uintptr_t));
		
	*pu4OutSz = u4TotalSz;

	return 0;
}

static long CfaAudioCompatConfig(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaAudioCfgInf __user *usr_ptr = NULL;
	CfaAudioCfgInf32 __user *usr_ptr32 = (CfaAudioCfgInf32 __user *)prInfo->usr_ptr32;
	__u8 __user *pu1UsrBufAddr = NULL;
	__u8 __user *pu1NextBufAddr = NULL;
	__u32 u4TotalSz = 0;
	__u32 u4Sz = 0;
	long ret = 0;
	compat_caddr_t compatSeqHdr = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaAudioCfgInf32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	if (0 != CfaAudioCompatConfigCalcSz(usr_ptr32, &u4TotalSz)) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaFlvCompatConfigCalcSz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	//pu1UsrBufAddr = (__u8 __user *)compat_alloc_user_space(u4TotalSz);
	DMX_NewMemory(u4TotalSz, pu1UsrBufAddr);
	*pfgIsUserMem = FALSE;
	if (NULL == pu1UsrBufAddr) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(pu1UsrBufAddr, 0, u4TotalSz);

	usr_ptr = (CfaAudioCfgInf __user *)pu1UsrBufAddr;

	pu1NextBufAddr = pu1UsrBufAddr + CFA_ALIGN_SZ((__u32)sizeof(CfaAudioCfgInf), sizeof(uintptr_t));
	u4Sz += CFA_ALIGN_SZ((__u32)sizeof(CfaAudioCfgInf), sizeof(uintptr_t));
	if (u4Sz > u4TotalSz)
	{
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
		TEXT("%s line %d size is large than total size.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -ENOMEM;
	}

	if (copy_from_user(&(usr_ptr->u4AudioByteRate),
		&(usr_ptr32->u4AudioByteRate),
		sizeof(__u32))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->fgAc3Type),
		&(usr_ptr32->fgAc3Type),
		sizeof(bool))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->eFileType),
		&(usr_ptr32->eFileType),
		sizeof(FILE_TYPE_E))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->eAudType),
		&(usr_ptr32->eAudType),
		sizeof(AVCODECID_T))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u4NumPoint),
		&(usr_ptr32->u4NumPoint),
		sizeof(__u32))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->u4SeekTableSz),
		&(usr_ptr32->u4SeekTableSz),
		sizeof(__u32))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if ((0 < usr_ptr32->u4SeekTableSz) && (0 == usr_ptr32->pcPoints)) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("%s line %d fail for no seektable, but header len(%d) > 0.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr->u4SeekTableSz);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}

	if (0 != usr_ptr32->pcPoints) {
		char *pcPoints = NULL;

		usr_ptr->pcPoints = pu1NextBufAddr;

		if (NULL == usr_ptr->pcPoints) {
			DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -ENOMEM;
		}
		mm_memset(usr_ptr->pcPoints, 0, sizeof(char) * usr_ptr->u4SeekTableSz);
		pu1NextBufAddr += CFA_ALIGN_SZ((__u32)(sizeof(char) * usr_ptr->u4SeekTableSz), sizeof(uintptr_t));
		u4Sz += CFA_ALIGN_SZ((__u32)(sizeof(char) * usr_ptr->u4SeekTableSz), sizeof(uintptr_t));
		if (u4Sz > u4TotalSz)
		{
			DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("%s line %d size is large than total size.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -ENOMEM;
		}
		if (get_user(compatSeqHdr, &(usr_ptr32->pcPoints))) {
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}
		if (0 == compatSeqHdr) {
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}
		pcPoints = compat_ptr(compatSeqHdr);
		if (!access_ok(VERIFY_READ, pcPoints,
			sizeof(char) * usr_ptr->u4SeekTableSz)) {
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}
		if (copy_from_user((char __user *)usr_ptr->pcPoints,
			pcPoints, sizeof(char) * usr_ptr->u4SeekTableSz)) {
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}
	}

	if (copy_from_user(&(usr_ptr->u8FrameStartOfst),
		&(usr_ptr32->u8FrameStartOfst),
		sizeof(__u64))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u4Duration),
		&(usr_ptr32->u4Duration),
		sizeof(__u32))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u4SampeRate),
		&(usr_ptr32->u4SampeRate),
		sizeof(__u32))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u4TxUnitRange),
		&(usr_ptr32->u4TxUnitRange),
		sizeof(__u32))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	ret = CfaAudioCompatAC3Info(&(usr_ptr->rAc3FrameInfo),
		&(usr_ptr32->rAc3FrameInfo), &pu1NextBufAddr, &u4Sz, u4TotalSz);
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
		TEXT("%s line %d fail in CfaAudioCompatAC3Info.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return ret;
	}
	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaAudioCfgInf);

	return 0;
}

static int CfaAudioProcCompat(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	long ret = 0;

	if ((NULL == prInfo) || (NULL == pfgIsUserMem)) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				TEXT("%s line %d fail for invalid parameter.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	}
	switch (prInfo->type) {
	case CFA_CONFIG:
	if (prInfo->is_get) {
			DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	}
	ret = CfaAudioCompatConfig(prInfo, pfgIsUserMem);
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaAudioCompatConfig.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}
	break;
	case CFA_RANGE:
    if (prInfo->is_get) {
			DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	}
	ret = CfaAudioCompatRange(prInfo, pfgIsUserMem);
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaAudioCompatRange.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}
	break;
	case CFA_GEN_INFO:
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("%s line %d fail for don;t support get info for cfa audio.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	case CFA_JUMP_INFO:
		if (prInfo->is_get) {
			DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
		}
		ret = CfaAudioCompatJumpRange(prInfo, pfgIsUserMem);
		if (0 != ret) {
		  DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
		    TEXT("%s line %d fail in CfaAudioCompatJumpRange.\r\n"),
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
static CfaApiAudType CfaAudioGetAudType(AVCODECID_T eType)
{
	switch (eType) {
	case AVCODEC_ID_PCM:
		return CFA_AUD_DRV_FMT_PCM;
	case AVCODEC_ID_MP3:
		return CFA_AUD_DRV_FMT_MP3;
	case AVCODEC_ID_DTS:
		return CFA_AUD_DRV_FMT_DTS;
	case AVCODEC_ID_DTSCD:
		return CFA_AUD_DRV_FMT_DTSCD;
	case AVCODEC_ID_MPEG:
		return CFA_AUD_DRV_FMT_MPEG;
	case AVCODEC_ID_AAC_PURE:
		return CFA_AUD_DRV_FMT_AAC;
	case AVCODEC_ID_FLAC:
		return CFA_AUD_DRV_FMT_FLAC;
	case AVCODEC_ID_AC3:
        return CFA_AUD_DRV_FMT_AC3;
	default:
		break;
	}

	return CFA_AUD_DRV_FMT_UNKNOWN;
}

static MRESULT CfaAudioInit(void *pvSptHdl, void **ppvCfaPrivData)
{
	CfaAudioInst *prCfaAudioInst = NULL;

	DMX_NewMemory(sizeof(CfaAudioInst), prCfaAudioInst);
	if (NULL == prCfaAudioInst) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				TEXT("[CFA_AUDIO] Alloc prCfaAudioInst memory fail\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	} else {
		dmx_memset(prCfaAudioInst, 0X00, sizeof(CfaAudioInst));
	}

	prCfaAudioInst->rRange.i4Rate = 1;
	prCfaAudioInst->rRange.u8Sa = 0;
	prCfaAudioInst->rRange.u8Ea = 0;
	prCfaAudioInst->u8CurrTxOft = DMX_INVALID_UINT64;
	prCfaAudioInst->pucPbBuf = NULL;

	*ppvCfaPrivData = (void *)prCfaAudioInst;
	prCfaAudioInst->rAc3FrameInfo.prAc3FrameInfo = NULL;

#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaAudioInst);
	MMATE_CHECK_STRUCT(prCfaAudioInst->rRange);
#endif

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaAudioUninit(void *pvSptHdl, void *pvCfaPrivData)
{
	CfaAudioInst *prCfaAudioInst = (CfaAudioInst *) pvCfaPrivData;

	if (NULL == prCfaAudioInst)
		MM_RETURN(RET_DMX_OK);

#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaAudioInst);
	MMATE_CHECK_STRUCT(prCfaAudioInst->rRange);
#endif

	if (prCfaAudioInst->pcPoints != NULL) {
		DMX_FreeMemory(prCfaAudioInst->pcPoints);
		prCfaAudioInst->pcPoints = NULL;
	}
	if (prCfaAudioInst->rAc3FrameInfo.prAc3FrameInfo != NULL)
    {
        DMX_FreeMemory(prCfaAudioInst->rAc3FrameInfo.prAc3FrameInfo);
        prCfaAudioInst->rAc3FrameInfo.prAc3FrameInfo = NULL;
    }

	prCfaAudioInst->pucPbBuf = NULL;
	DMX_FreeMemory(prCfaAudioInst);
	prCfaAudioInst = NULL;

	MM_RETURN(RET_DMX_OK);
}

static bool fgInCfaAudioRange(const CfaAudioInst *prCfaAudioInst)
{
	if (NULL == prCfaAudioInst) {    
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
		TEXT("[CFA_AUDIO] prCfaAudioInst is NULL!\r\n"));
		MM_RETURN(RET_DMX_EXT_NO_MEM);
	}
#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaAudioInst);
#endif

	if (prCfaAudioInst->u8CurrTxOft + prCfaAudioInst->u4TxUnitRange >
	    prCfaAudioInst->rRange.u8Ea) {
		return FALSE;
	} else {
		return TRUE;
	}
}

static MRESULT CfaAudioSetRange(void *pvSptHdl, void *pvRange, void *pvPrivData, bool fgIsUserMem)
{
	CfaAudioInst *prCfaAudioInst = (CfaAudioInst *)pvPrivData;

	DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT, TEXT("[CFA_AUDIO] Entry %s !\n"), DMX_FUNC_NAME);
#ifdef MM_ATE_CHECK
	if (0 != mm_copy_from_user(&(prCfaAudioInst->rRange.u8Sa), &(((CfaAudioPR *)pvRange)->u8Sa),
			sizeof(CfaAudioPR) - 2 * sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("[CFA MKV] %s line %d failed in mm_copy_from_user\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}
#else
	if (0 != mm_copy_from_user(&(prCfaAudioInst->rRange), pvRange,
			sizeof(CfaAudioPR))) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("[CFA MKV] %s line %d failed in mm_copy_from_user\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}
#endif
#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaAudioInst);
	MMATE_INIT_STRUCT(prCfaAudioInst->rRange);
#endif

	prCfaAudioInst->u8CurrTxOft   = prCfaAudioInst->rRange.u8Sa;
	
	prCfaAudioInst->i4Rate		  = prCfaAudioInst->rRange.i4Rate;
	prCfaAudioInst->u4SeekNum	  = prCfaAudioInst->rRange.u4SeekNum;
	prCfaAudioInst->u8SeekTime	  = prCfaAudioInst->rRange.u8SeekTime;
	if (CFA_FILE_AC3 != prCfaAudioInst->eFileType || (MM_IS_RW_PLAY(prCfaAudioInst->i4Rate)))
	{
		prCfaAudioInst->u4TxUnitRange = prCfaAudioInst->rRange.u4TxUnitRange;
	}

	if (CFA_FILE_AC3 == prCfaAudioInst->eFileType && (!MM_IS_RW_PLAY(prCfaAudioInst->i4Rate)))
	{
		prCfaAudioInst->u4Ac3CurFrameNo = prCfaAudioInst->rRange.u4Ac3FrameNo;
		
		DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_COMMON, TEXT("[CFA_AUDIO]	%s ,u4Ac3CurFrameNo = %d !\n"),
			DMX_FUNC_NAME,prCfaAudioInst->u4Ac3CurFrameNo );
		
		DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_COMMON, TEXT("[CFA_AUDIO] %s ,u8FrameSize = 0x%llx\r\n"),
						   DMX_FUNC_NAME,prCfaAudioInst->rAc3FrameInfo.prAc3FrameInfo[prCfaAudioInst->u4Ac3CurFrameNo].u8FrameSize);
	}

	DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_COMMON, TEXT("[CFA_AUDIO]	%s ,i4Rate = %d ,u4TxUnitRange = %d! \r\n"),
			DMX_FUNC_NAME,prCfaAudioInst->i4Rate , prCfaAudioInst->u4TxUnitRange);
	if (prCfaAudioInst->rRange.u8Ea < prCfaAudioInst->rRange.u8Sa)
	{
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,TEXT("[CFA_AUDIO] end offset is large than start offset!\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT, TEXT("[CFA_AUDIO] Exit %s !\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}


static MRESULT CfaAudioEnableStrm(void *pvSptHdl, u32 u4StrmToPrs,
				  CfaStreamOp eOp, void *pvPrivData)
{
	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaAudioSetStrmInf(void *pvSptHdl, u32 u4Strm, u32 u4Info, void *pvPrivData)
{
	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaAudioProcCliCmd(void *pvSptHdl, E_DMX_CFA_CLI_TYPE_T eCliType, /*< [IN] Cfa Cli Command*/
				u32 arg1,
				u32 arg2, u32 arg3, const char *szParam, void *pvPrivData)
{
	MRESULT mrRet = RET_DMX_OK;
	CfaAudioInst *prCfaAudio = NULL;

	prCfaAudio = (CfaAudioInst *) pvPrivData;

	switch (eCliType) {
	case DMX_CFA_CLI_CMD_TURN_ONOFF_LOG:
		{
			BOOL fgEnable = (BOOL)TRUE;
		/**
		* arg1: u4OnOff
		* arg2: LogLevel(T, E, W, D)
		* arg3: Module Log Level
		**/
			if (0 == arg1)
				fgEnable = (BOOL)FALSE;

			DmxLogT(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				TEXT("CfaAudioProcCliCmd -- fgEnable: %d, Loglvl: %d, ModLogLvl: 0x%08x \r\n"),
				arg1, arg2, arg3);

			DmxLogEnable(fgEnable, arg2, DMX_MOD_CFA_AUDIO, arg3);
		}
		break;
	case DMX_CFA_CLI_CMD_DUMP_INFO:
		{
			DmxLogT(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				TEXT("Cfa AUDIO Instance(prCfaAudio is %p)")
				TEXT(" Info list as follow: \r\n"),
				prCfaAudio);
			DmxLogT(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				TEXT("Current Analyse State is %d \r\n"),
				prCfaAudio->eCfaAudioAnaSt);
			DmxLogT(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				TEXT("Current Analyse Position is 0x%08x%08x, "),
				(u32) ((prCfaAudio->u8CurrTxOft) >> 32), (u32) (prCfaAudio->u8CurrTxOft));
		}
		break;
	default:
		break;
	}

	MM_RETURN(mrRet);
}
static MRESULT CfaAudioTurnOn(void *pvSptHdl, void *pvPrivData)
{
	CfaAudioInst *prCfaAudioInst = (CfaAudioInst *) pvPrivData;
	MRESULT mrResult = RET_DMX_OK;
	u64 u8Txlen;

	prCfaAudioInst->u8StepTime = 0;
#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaAudioInst);
	MMATE_CHECK_STRUCT(prCfaAudioInst->rRange);
#endif

	DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
		TEXT("[CFA_AUDIO] %s: Current Tx Offset is 0x%llx, Rate is %d\r\n"),
		DMX_FUNC_NAME, prCfaAudioInst->u8CurrTxOft, prCfaAudioInst->i4Rate);

	if (MM_IS_RW_PLAY(prCfaAudioInst->i4Rate)) {
		if (!prCfaAudioInst->fgJumpTurnOn) {
			DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				    TEXT("[CFA_AUDIO] %s,doesn't Call jump,so only send one AU\r\n"),
				    DMX_FUNC_NAME);
			if (prCfaAudioInst->u8CurrTxOft >= prCfaAudioInst->rRange.u8Ea) {
				DmxLogT(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
					    TEXT("[CFA_AUDIO] line %d sent EOS OK\r\n"), DMX_LINE_NO);
				mrResult =
				    Spt4CfaFinishedEx(pvSptHdl, prCfaAudioInst->rRange.u8Ea, FALSE, GAU_E_EOS);
				MM_RETURN(RET_DMX_OK);
			} else if (prCfaAudioInst->u8CurrTxOft + prCfaAudioInst->u4TxUnitRange <=
				   prCfaAudioInst->rRange.u8Ea) {

				mrResult = Spt4CfaPbb2AFifo(pvSptHdl, prCfaAudioInst->u8CurrTxOft, 0,
							    (u64)prCfaAudioInst->u4TxUnitRange,
							    prCfaAudioInst->eAudType);
				if (RET_DMX_OK != mrResult) {
					DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
							TEXT("[CFA_AUDIO] Send EOS\r\n"));
					Spt4CfaFinishedEx(pvSptHdl, prCfaAudioInst->u8CurrTxOft, TRUE, GAU_E_FAIL);
					MM_RETURN(RET_DMX_OK);
				}
				prCfaAudioInst->eCfaAudioAnaSt = CFA_AUDIO_ANA_JUMP;
				MM_RETURN(mrResult);
			} else {
				u64 u8LastTxlen = 0;

				u8LastTxlen =
				    prCfaAudioInst->rRange.u8Ea - prCfaAudioInst->u8CurrTxOft;
				mrResult =
				    Spt4CfaPbb2AFifo(pvSptHdl, prCfaAudioInst->u8CurrTxOft, (u64)0,
						     u8LastTxlen, prCfaAudioInst->eAudType);
				if (RET_DMX_OK != mrResult) {
					DmxLogT(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
							TEXT("[CFA_AUDIO] line %d, Send EOS\r\n"), DMX_LINE_NO);
					Spt4CfaFinishedEx(pvSptHdl, prCfaAudioInst->rRange.u8Ea, TRUE,
							  GAU_E_EOS);
				}
				prCfaAudioInst->eCfaAudioAnaSt = CFA_AUDIO_ANA_JUMP;
				MM_RETURN(mrResult);
			}
		}
	}

	if ((CFA_FILE_FLAC == prCfaAudioInst->eFileType) && MM_IS_FF_PLAY(prCfaAudioInst->i4Rate)) {
		if (!prCfaAudioInst->fgJumpTurnOn) {
			DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				    TEXT("[CFA_AUDIO] %s,doesn't Call jump,so don't turn on\r\n"),
				    DMX_FUNC_NAME);
			mrResult =
			    Spt4CfaFinishedEx(pvSptHdl, prCfaAudioInst->rRange.u8Ea, FALSE, GAU_E_EOS);
			MM_RETURN(RET_DMX_OK);
		} else if (prCfaAudioInst->u8CurrTxOft >= prCfaAudioInst->rRange.u8Ea) {
			DmxLogT(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
					TEXT("[CFA_AUDIO] line %d sent EOS OK\r\n"), DMX_LINE_NO);
			mrResult =
			    Spt4CfaFinishedEx(pvSptHdl, prCfaAudioInst->rRange.u8Ea, TRUE, GAU_E_EOS);
			MM_RETURN(RET_DMX_OK);
		} else {
			/*do nothing*/
		}
	}

	if (prCfaAudioInst->u8CurrTxOft >= prCfaAudioInst->rRange.u8Ea) {
		DmxLogT(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				TEXT("[CFA_AUDIO] line %d sent EOS OK\r\n"), DMX_LINE_NO);
		mrResult = Spt4CfaFinishedEx(pvSptHdl, prCfaAudioInst->rRange.u8Ea, FALSE, GAU_E_EOS);
		MM_RETURN(RET_DMX_OK);
	}

	if (prCfaAudioInst->u8CurrTxOft == DMX_INVALID_UINT64) {    
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("[CFA_AUDIO] current tx offset is invalid!\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	if (CFA_FILE_AC3 == prCfaAudioInst->eFileType && (!MM_IS_RW_PLAY(prCfaAudioInst->i4Rate))) {
		if (prCfaAudioInst->rAc3FrameInfo.u4FrameCount <= prCfaAudioInst->u4Ac3CurFrameNo) {
			DmxLogT(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT, TEXT("[CFA_AUDIO] line %d sent EOS OK\r\n"), DMX_LINE_NO);
			Spt4CfaFinishedEx(pvSptHdl, prCfaAudioInst->u8CurrTxOft, TRUE, GAU_E_FAIL);
			MM_RETURN(RET_DMX_OK);
		}
		prCfaAudioInst->u4TxUnitRange = (UINT32)prCfaAudioInst->rAc3FrameInfo.prAc3FrameInfo[prCfaAudioInst->u4Ac3CurFrameNo].u8FrameSize;
		DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_COMMON, TEXT("[CFA_AUDIO] %s: u4Ac3CurFrameNo = %d, u4TxUnitRange = %d \r\n"),
                    DMX_FUNC_NAME,
                    prCfaAudioInst->u4Ac3CurFrameNo,prCfaAudioInst->u4TxUnitRange);
		DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_COMMON, TEXT("[CFA_AUDIO] %s ,u8FrameSize = 0x%llx\r\n"),
                        DMX_FUNC_NAME,prCfaAudioInst->rAc3FrameInfo.prAc3FrameInfo[prCfaAudioInst->u4Ac3CurFrameNo].u8FrameSize);
	}
	
	if (fgInCfaAudioRange(prCfaAudioInst)) {
		mrResult = Spt4CfaPbb2AFifo(pvSptHdl, prCfaAudioInst->u8CurrTxOft, 0,
				(u64)prCfaAudioInst->u4TxUnitRange,
				prCfaAudioInst->eAudType);
		if (RET_DMX_OK != mrResult) {
			DmxLogT(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				TEXT("[CFA_AUDIO] line %d sent EOS OK\r\n"), DMX_LINE_NO);
			Spt4CfaFinishedEx(pvSptHdl, prCfaAudioInst->u8CurrTxOft, TRUE, GAU_E_FAIL);
			MM_RETURN(RET_DMX_OK);
		}

		if (MM_IS_RW_PLAY(prCfaAudioInst->i4Rate) ||
		    ((CFA_FILE_FLAC == prCfaAudioInst->eFileType)
		     && MM_IS_FF_PLAY(prCfaAudioInst->i4Rate))) {
			prCfaAudioInst->fgJumpTurnOn = FALSE;
			prCfaAudioInst->eCfaAudioAnaSt = CFA_AUDIO_ANA_JUMP;
		} else {
			prCfaAudioInst->eCfaAudioAnaSt = CFA_AUDIO_ANA_TX;
		}
	} else {
		u8Txlen = prCfaAudioInst->rRange.u8Ea - prCfaAudioInst->rRange.u8Sa;
		if (u8Txlen == 0) {
			prCfaAudioInst->u8CurrTxOft = DMX_INVALID_UINT64;
			DmxLogT(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				TEXT("[CFA_AUDIO] line %d sent EOS OK\r\n"), DMX_LINE_NO);
			Spt4CfaFinishedEx(pvSptHdl, prCfaAudioInst->rRange.u8Ea, FALSE, GAU_E_EOS);
			MM_RETURN(RET_DMX_OK);
		} else {
			mrResult = Spt4CfaPbb2AFifo(pvSptHdl, prCfaAudioInst->u8CurrTxOft, 0,
					u8Txlen, prCfaAudioInst->eAudType);
			if (RET_DMX_OK != mrResult) {
				DmxLogT(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
					TEXT("[CFA_AUDIO] line %d sent EOS OK\r\n"), DMX_LINE_NO);
				Spt4CfaFinishedEx(pvSptHdl, prCfaAudioInst->u8CurrTxOft, TRUE,
					GAU_E_FAIL);
				prCfaAudioInst->u8CurrTxOft = DMX_INVALID_UINT64;
				MM_RETURN(RET_DMX_OK);
			}
			if (MM_IS_RW_PLAY(prCfaAudioInst->i4Rate) ||
				((CFA_FILE_FLAC == prCfaAudioInst->eFileType)
				&& MM_IS_FF_PLAY(prCfaAudioInst->i4Rate))) {
				prCfaAudioInst->fgJumpTurnOn = FALSE;
				prCfaAudioInst->eCfaAudioAnaSt = CFA_AUDIO_ANA_JUMP;
			} else {
				prCfaAudioInst->eCfaAudioAnaSt = CFA_AUDIO_ANA_TX;
			}
		}
	}

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaAudioTxDone(void *pvSptHdl, u64 u8TxLen, void *pvPrivData, bool fgRsp)
{
	MRESULT mrResult = RET_DMX_OK;
	u64 u8LastTxlen = 0;

	CfaAudioInst *prCfaAudioInst = (CfaAudioInst *) pvPrivData;

	DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_COMMON,
		TEXT("[CFA_AUDIO] %s::Rate is %d,Current Tx Offset is %lld,End offset is %lld\r\n"),
		DMX_FUNC_NAME, prCfaAudioInst->i4Rate,
		prCfaAudioInst->u8CurrTxOft, prCfaAudioInst->rRange.u8Ea);
#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaAudioInst);
	MMATE_CHECK_STRUCT(prCfaAudioInst->rRange);
#endif

	if (CFA_AUDIO_ANA_JUMP == prCfaAudioInst->eCfaAudioAnaSt) {
		DmxLogT(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				TEXT("[CFA_AUDIO]: %s--JUMP--Send EOS \r\n"), DMX_FUNC_NAME);
		Spt4CfaFinishedEx(pvSptHdl, prCfaAudioInst->u8CurrTxOft, FALSE, GAU_E_EOS);
		prCfaAudioInst->eCfaAudioAnaSt = CFA_AUDIO_ANA_TX;
		MM_RETURN(RET_DMX_OK);
	}

	if (MM_IS_NORMAL_PLAY(prCfaAudioInst->i4Rate)
	    || (prCfaAudioInst->eFileType != CFA_FILE_FLAC)) {
		prCfaAudioInst->u8CurrTxOft += prCfaAudioInst->u4TxUnitRange;

		if (prCfaAudioInst->u8CurrTxOft >= prCfaAudioInst->rRange.u8Ea) {
			prCfaAudioInst->u8CurrTxOft = DMX_INVALID_UINT64;
			DmxLogT(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
					TEXT("[CFA_AUDIO] line %d sent EOS OK\r\n"), DMX_LINE_NO);
			mrResult = Spt4CfaFinishedEx(pvSptHdl, prCfaAudioInst->rRange.u8Ea, TRUE, GAU_E_EOS);
			MM_RETURN(RET_DMX_OK);
		}

		if (CFA_FILE_AC3 == prCfaAudioInst->eFileType && (!(MM_IS_RW_PLAY(prCfaAudioInst->i4Rate))))
        {
            prCfaAudioInst->u4Ac3CurFrameNo += 1;
            if (prCfaAudioInst->rAc3FrameInfo.u4FrameCount <= prCfaAudioInst->u4Ac3CurFrameNo)
            {
                DmxLogT(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT, TEXT("[CFA_AUDIO] line %d sent EOS OK\r\n"), DMX_LINE_NO);
                Spt4CfaFinishedEx(pvSptHdl, prCfaAudioInst->u8CurrTxOft, TRUE, GAU_E_FAIL);
                MM_RETURN(RET_DMX_OK);
            }
            prCfaAudioInst->u8CurrTxOft = prCfaAudioInst->rAc3FrameInfo.prAc3FrameInfo[prCfaAudioInst->u4Ac3CurFrameNo].u8FrameOft;
            prCfaAudioInst->u4TxUnitRange = (UINT32)prCfaAudioInst->rAc3FrameInfo.prAc3FrameInfo[prCfaAudioInst->u4Ac3CurFrameNo].u8FrameSize;   
            DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_COMMON, TEXT("[CFA_AUDIO] %s: u4Ac3CurFrameNo = %d, u4TxUnitRange = %d u8CurrTxOft = %lld\r\n"),
                           DMX_FUNC_NAME,
                           prCfaAudioInst->u4Ac3CurFrameNo,prCfaAudioInst->u4TxUnitRange,
                           prCfaAudioInst->u8CurrTxOft);
        }
		
		if (prCfaAudioInst->u8CurrTxOft + prCfaAudioInst->u4TxUnitRange >
		    prCfaAudioInst->rRange.u8Ea) {
			u8LastTxlen = prCfaAudioInst->rRange.u8Ea - prCfaAudioInst->u8CurrTxOft;
			DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_COMMON,
				    TEXT("[CFA_AUDIO] line %d Spt4CfaPbb2AFifo (u8CurrTxOft: %lld, Len: %lld)\r\n"),
				    DMX_LINE_NO, prCfaAudioInst->u8CurrTxOft, u8LastTxlen);
			mrResult =
			    Spt4CfaPbb2AFifo(pvSptHdl, prCfaAudioInst->u8CurrTxOft, (u64)0, u8LastTxlen,
					     prCfaAudioInst->eAudType);
			if (RET_DMX_OK != mrResult) {
				DmxLogT(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
					    TEXT("[CFA_AUDIO] line %d sent EOS OK\r\n"), DMX_LINE_NO);
				Spt4CfaFinishedEx(pvSptHdl, prCfaAudioInst->u8CurrTxOft, TRUE, GAU_E_FAIL);
				prCfaAudioInst->u8CurrTxOft = DMX_INVALID_UINT64;
				MM_RETURN(RET_DMX_OK);
			}
		} else {
			DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_COMMON,
				    TEXT("[CFA_AUDIO] line %d Spt4CfaPbb2AFifo (u8CurrTxOft: %d, Len: %d)\r\n"),
				    DMX_LINE_NO, (u32) prCfaAudioInst->u8CurrTxOft, (u32) u8LastTxlen);
			mrResult = Spt4CfaPbb2AFifo(pvSptHdl, prCfaAudioInst->u8CurrTxOft, 0,
						(u64)(prCfaAudioInst->u4TxUnitRange),
						prCfaAudioInst->eAudType);
			if (RET_DMX_OK != mrResult) {
				DmxLogT(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
					    TEXT("[CFA_AUDIO] line %d sent EOS OK\r\n"),
					    DMX_LINE_NO);
				Spt4CfaFinishedEx(pvSptHdl, prCfaAudioInst->u8CurrTxOft, TRUE, GAU_E_FAIL);
				prCfaAudioInst->u8CurrTxOft = DMX_INVALID_UINT64;
				MM_RETURN(RET_DMX_OK);
			}
		}
	}

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaAudioFillAUInfo(void *pvSptHdl, void *pvAUInfo, void *pvAUExtInfo,
				  void *pvPrivData)
{
	DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_COMMON,
			TEXT("[CFA_AUDIO] =========Cfa Audio Fill AU Info==========\r\n"));
	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaAudioCurPos(void *pvSptHdl, void *pvCurPos, void *pvPrivData)
{
	CfaAudioInst *prCfaAudioInst = NULL;
	u64 *pvu8 = pvCurPos;
	if (NULL == pvPrivData) {    
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
		TEXT("[CFA_AUDIO] pvPrivData is NULL!\r\n"));
		MM_RETURN(RET_DMX_EXT_NO_MEM);
	}
	prCfaAudioInst = (CfaAudioInst *) pvPrivData;

	*pvu8 = prCfaAudioInst->u8CurrTxOft;

#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaAudioInst);
#endif

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaAudioConfigure(void *pvSptHdl, void *pvParam, void *pvPrivData, bool fgIsUserMem)
{
	CfaAudioInst *prCfaAudioInst = NULL;
	CfaAudioCfgInf *prCfaAudioCfgInf = NULL;
	CfaAudioCfgInf rCfaAudioCfgInf;

	if ((NULL == pvParam) || (NULL == pvPrivData)) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			    TEXT("[CFA_AUDIO] the parameter is invalid\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mm_memset(&rCfaAudioCfgInf, 0, sizeof(rCfaAudioCfgInf));
	if (fgIsUserMem) {
		if (0 != mm_copy_from_user(&rCfaAudioCfgInf,
			pvParam, sizeof(CfaAudioCfgInf))) {
			DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				TEXT("[CFA_AUDIO] %s line %d failed in mm_copy_from_user\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
		}
	} else {
		mm_memcpy(&rCfaAudioCfgInf,
			pvParam, sizeof(CfaAudioCfgInf));
	}
	prCfaAudioCfgInf = &rCfaAudioCfgInf;

	prCfaAudioInst = (CfaAudioInst *) pvPrivData;
	prCfaAudioInst->fgAc3Type = prCfaAudioCfgInf->fgAc3Type;
	prCfaAudioInst->eFileType = prCfaAudioCfgInf->eFileType;
	prCfaAudioInst->eAudType = CfaAudioGetAudType(prCfaAudioCfgInf->eAudType);
	prCfaAudioInst->u8FrameStartOfst = prCfaAudioCfgInf->u8FrameStartOfst;
	prCfaAudioInst->u4NumPoint = prCfaAudioCfgInf->u4NumPoint;
	prCfaAudioInst->u4Duration = prCfaAudioCfgInf->u4Duration;
	prCfaAudioInst->u4SampeRate = prCfaAudioCfgInf->u4SampeRate;
	prCfaAudioInst->u4TxUnitRange = prCfaAudioCfgInf->u4TxUnitRange;
	
	if (CFA_FILE_FLAC == prCfaAudioInst->eFileType) {
		DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("[CFA_AUDIO] File type is FLAC\r\n"));
	} else if (CFA_FILE_AC3 == prCfaAudioInst->eFileType) {
		prCfaAudioInst->rAc3FrameInfo.u4FrameCount = prCfaAudioCfgInf->rAc3FrameInfo.u4FrameCount;
    	if (0 == prCfaAudioInst->rAc3FrameInfo.u4FrameCount) {
			if (!fgIsUserMem) {
				DMX_FreeMemory(pvParam);
			}
			MM_RETURN(RET_DMX_PARAM_WRONG);
    	} else {
		DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_COMMON,
		TEXT("[CFA_AUDIO] %s:frame count is %d\r\n"), DMX_FUNC_NAME,
			prCfaAudioInst->rAc3FrameInfo.u4FrameCount);
    	}
    	DMX_NewMemory(prCfaAudioInst->rAc3FrameInfo.u4FrameCount * sizeof(LP_CFA_AC3_FRAME_INFO_T),
		prCfaAudioInst->rAc3FrameInfo.prAc3FrameInfo);
    	if (NULL == prCfaAudioCfgInf->rAc3FrameInfo.prAc3FrameInfo) {
			if (!fgIsUserMem) {
				DMX_FreeMemory(pvParam);
			}
			MM_RETURN(RET_DMX_EXT_NO_MEM);
		} else {
			dmx_memset( prCfaAudioInst->rAc3FrameInfo.prAc3FrameInfo, 0, 
			prCfaAudioInst->rAc3FrameInfo.u4FrameCount * sizeof(LP_CFA_AC3_FRAME_INFO_T));
			if (fgIsUserMem) {
				if (0 != mm_copy_from_user( prCfaAudioInst->rAc3FrameInfo.prAc3FrameInfo,
						prCfaAudioCfgInf->rAc3FrameInfo.prAc3FrameInfo,
						prCfaAudioInst->rAc3FrameInfo.u4FrameCount * sizeof(LP_CFA_AC3_FRAME_INFO_T))) {
					DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,TEXT("[CFA_AUDIO] %s:copy from user fail\r\n"), DMX_FUNC_NAME);
					MM_RETURN(RET_DMX_EXT_EXCEPTION);
				}
			} else {
				dmx_memcpy( prCfaAudioInst->rAc3FrameInfo.prAc3FrameInfo,
					prCfaAudioCfgInf->rAc3FrameInfo.prAc3FrameInfo,
					prCfaAudioInst->rAc3FrameInfo.u4FrameCount * sizeof(LP_CFA_AC3_FRAME_INFO_T));
			}
		}
		DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
		TEXT("[CFA_AUDIO]%s: File type is AC3\r\n"), DMX_FUNC_NAME);
	} else if (CFA_FILE_AAC == prCfaAudioInst->eFileType) {
		DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				TEXT("[CFA_AUDIO] File type is AAC\r\n"));
	} else if (CFA_FILE_MP3 == prCfaAudioInst->eFileType) {
		DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				TEXT("[CFA_AUDIO] File type is MP3\r\n"));
	} else if (CFA_FILE_WAV == prCfaAudioInst->eFileType) {
		DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				TEXT("[CFA_AUDIO] File type is WAV\r\n"));
	} else {
		DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				TEXT("[CFA_AUDIO] File type is UNKNOWN\r\n"));
	}

#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaAudioInst);
	MMATE_CHECK_STRUCT(prCfaAudioInst->rRange);
#endif

	if (CFA_FILE_FLAC == prCfaAudioInst->eFileType) {
		DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			    TEXT("[CFA_AUDIO] %s:: The addr of seek table is 0x%x\r\n"),
			    DMX_FUNC_NAME, prCfaAudioCfgInf->pcPoints);
		DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			    TEXT("[CFA_AUDIO] %s:: The number of seek table is %d\r\n"),
			    DMX_FUNC_NAME, prCfaAudioCfgInf->u4NumPoint);
		DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			    TEXT("[CFA_AUDIO] %s:: Seek Table Size = %d!!\r\n"),
			    DMX_FUNC_NAME, prCfaAudioCfgInf->u4SeekTableSz);
		if ((prCfaAudioCfgInf->u4SeekTableSz != 0) && (prCfaAudioCfgInf->u4NumPoint != 0)
		    && (prCfaAudioCfgInf->pcPoints != NULL)) {
			DMX_NewMemory(prCfaAudioCfgInf->u4SeekTableSz * sizeof(char),
				      prCfaAudioInst->pcPoints);
			if (NULL == prCfaAudioInst->pcPoints) {
				if (!fgIsUserMem) {
					DMX_FreeMemory(pvParam);
				}
				MM_RETURN(RET_DMX_EXT_NO_MEM);
			} else {
				dmx_memset(prCfaAudioInst->pcPoints, 0,
					   prCfaAudioCfgInf->u4SeekTableSz * sizeof(char));
				if (fgIsUserMem) {
					if (0 !=
					    mm_copy_from_user(prCfaAudioInst->pcPoints,
							prCfaAudioCfgInf->pcPoints,
							prCfaAudioCfgInf->u4SeekTableSz *
							sizeof(char))) {
						DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
							    TEXT("[CFA_AUDIO] copy from user fail\r\n"));
						MM_RETURN(RET_DMX_EXT_EXCEPTION);
					}
				} else {
					dmx_memcpy(prCfaAudioInst->pcPoints,
						prCfaAudioCfgInf->pcPoints,
						prCfaAudioCfgInf->u4SeekTableSz *
						sizeof(char));
				}
			}
		} else {
			prCfaAudioInst->pcPoints = NULL;
			DmxLogT(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
				    TEXT("[CFA_AUDIO] %s:: The addr of seek table is NULL\r\n"),
				    DMX_FUNC_NAME);
		}
	}
	if (!fgIsUserMem) {
		DMX_FreeMemory(pvParam);
	}
	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaAudioSetJumpRange(void *pvSptHdl, void *pvJmpRange, void *pvPrivData)
{
	CfaAudioInst *prCfaAudioInst = NULL;
	CfaAudioKeyFrmRange_T *prCfaAudioKeyFrmRange = NULL;
	CfaAudioKeyFrmRange_T rCfaAudioKeyFrmRange;

	if ((NULL == pvPrivData) || (NULL == pvJmpRange)) {    
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
		TEXT("[CFA_AUDIO] pvPrivData or pvJmpRange is NULL!\r\n"));
		MM_RETURN(RET_DMX_EXT_NO_MEM);
	}
	mm_memset(&rCfaAudioKeyFrmRange, 0, sizeof(rCfaAudioKeyFrmRange));
	if (0 != mm_copy_from_user(prCfaAudioKeyFrmRange,
		pvJmpRange, sizeof(CfaAudioKeyFrmRange_T))) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
			TEXT("[CFA_AUDIO] %s line %d failed in mm_copy_from_user\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}
	prCfaAudioKeyFrmRange = &rCfaAudioKeyFrmRange;
	prCfaAudioInst = (CfaAudioInst *) pvPrivData;
#ifdef MM_ATE_CHECK
	dmx_memcpy(&(prCfaAudioInst->rRange.u8Sa), &(prCfaAudioKeyFrmRange->rCfaAudioRange.u8Sa),
		   sizeof(CfaAudioPR) - 2 * sizeof(u32));
#else
	dmx_memcpy(&(prCfaAudioInst->rRange), &(prCfaAudioKeyFrmRange->rCfaAudioRange),
		   sizeof(CfaAudioPR));
#endif

#ifdef MM_ATE_CHECK
	MMATE_INIT_STRUCT(prCfaAudioInst->rRange);
#endif

	prCfaAudioInst->u8CurrTxOft = prCfaAudioInst->rRange.u8Sa;
	prCfaAudioInst->u4TxUnitRange = prCfaAudioKeyFrmRange->u4TxUnitKeyFrmRange;
	prCfaAudioInst->i4Rate = prCfaAudioInst->rRange.i4Rate;

	prCfaAudioInst->fgJumpTurnOn = TRUE;

	if (prCfaAudioInst->rRange.u8Ea < prCfaAudioInst->rRange.u8Sa) {
		DmxLogE(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_DEFAULT,
		TEXT("[CFA_AUDIO] end offset is large than start offset!\r\n"));
		MM_RETURN(RET_DMX_EXT_PARAM_WRONG);
	}
	DmxLogD(DMX_MOD_CFA_AUDIO, CFA_AUDIO_LOG_FFRW,
		TEXT("[CFA_AUDIO] %s:Rate is %d,Current Tx Offset[0x%llx],Tx Unit Range[0x%x]\r\n"),
                DMX_FUNC_NAME, prCfaAudioInst->i4Rate,
		prCfaAudioInst->u8CurrTxOft,prCfaAudioInst->u4TxUnitRange);

#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaAudioInst);
#endif
	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaAudioGetParamSize(void *pvSptHdl, u32 u4ParamID,
				    void *pvPrivData, void *pvCfaParam, u32 u4CfaParamSz)
{
	MRESULT mrResult = RET_DMX_OK;

	switch (u4ParamID) {
	case CFA_PARAM_ID_JUMP_INFO_SIZE:
		{
			if ((NULL == pvCfaParam) || ((u4CfaParamSz) < sizeof(u32))) {
				mrResult = RET_DMX_PARAM_WRONG;
			} else {
				u32 *pu4Tmp = (u32 *) pvCfaParam;
				*pu4Tmp = sizeof(CfaAudioKeyFrmRange_T);
			}
			break;
		}

	default:
		mrResult = RET_DMX_PARAM_WRONG;
		break;
	}

	MM_RETURN(mrResult);
}


CfaIntf _rAudioCfaIntf = {
	&CfaAudioInit,
	&CfaAudioUninit,
	&CfaAudioSetRange,
	&CfaAudioEnableStrm,
	&CfaAudioSetStrmInf,
	&CfaAudioTurnOn,
	&CfaAudioTxDone,
	&CfaAudioCurPos,
	NULL,
	&CfaAudioConfigure,
	NULL,
	NULL,
	NULL,
	NULL,
	&CfaAudioFillAUInfo,
	NULL,
	NULL, &CfaAudioSetJumpRange, &CfaAudioGetParamSize, &CfaAudioProcCliCmd
	#ifdef CONFIG_COMPAT
	,&CfaAudioProcCompat
	#endif
};

void *CfaAudioGetInterface(void)
{
	return ((void *) &_rAudioCfaIntf);
}
