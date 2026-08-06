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
 *    CFA_APE File
 *
 * @par Author_Name
 *    Qing Li 
*****************************************************************************/

 /*!
  * @file       cfa_ape.c
  * @author  
  * @version 1.0
  * @brief     The C file of the interface for Audio CFA
  */
#include "cfa_ape.h"
#include "dmx_mem.h"
#include "mmisc.h"
#include "cfa_macro.h"

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
typedef struct {
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKStart;
#endif
	__u64 u8Sa;/* < start file offset, 0-based */
	__u64 u8Ea;/* < end file offset.    The byte of this offset is transferred. */
	__s32 i4Rate; /* FF/RW */
	__u32 u4UnitTxDataSz;
	bool fgSetSeekInfo;
	__u32 au4SeekInfo[2];
	__u32 u4TxUnitRange;
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKEnd;
#endif
} CfaApePR32;

typedef struct {
	__u64 u8Pos;
	__u64 u8Pts;
	__u32 u4Blocks;
	__u32 u4Size;
	__u32 u4Skip;
} CFA_APE_FRAME_INFO_T32;

typedef struct {
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKStart;
#endif
	__u32 u4TotalFrames;	/* the total number frames (frames are used internally) */
	__u32 u4BlocksPerFrame;	/* the samples in a frame (frames are used internally) */
	__u32 u4FinalFrameBlocks;	/* the number of samples in the final frame */
	__u32 u4Channels;	/* audio channels */
	__u32 u4SampleRate;	/* audio samples per second */
	__u32 u4BitsPerSample;	/* audio bits per sample */
	__u32 u4BytesPerSample;	/* audio bytes per sample */
	__u32 u4BlockAlign;	/* audio block align (channels * bytes per sample) */
	__u32 u4TotalBlocks;	/* the total number audio blocks */
	__u32 u4AverageBitrate;	/* the kbps (i.e. 637 kpbs) */
	__u32 u4SeekTableElements;	/* the number of elements in the seek table(s) */
	compat_caddr_t pSeekByteTable;

	compat_caddr_t pFrames;
	bool fgSeekable;

	__u32 u4AudioByteRate;	/* < byte / s */
	__u32 u4Duration;
	bool fgAc3Type;
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKEnd;
#endif
} CfaApeCfgInf32;

static long CfaApeCompatRangeInfo(CfaApePR __user *usr_ptr,
  CfaApePR32 __user *usr_ptr32)
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

	if (copy_in_user(&(usr_ptr->u4UnitTxDataSz), &(usr_ptr32->u4UnitTxDataSz), sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->fgSetSeekInfo), &(usr_ptr32->fgSetSeekInfo), sizeof(bool)))
		return -EFAULT;

	if (copy_in_user(usr_ptr->au4SeekInfo, usr_ptr32->au4SeekInfo, 2 * sizeof(__u32)))
		return -EFAULT;

	if (copy_in_user(&(usr_ptr->u4TxUnitRange), &(usr_ptr32->u4TxUnitRange), sizeof(__u32)))
		return -EFAULT;

#ifdef MM_ATC_CHECK
	if (copy_in_user(&(usr_ptr->u4MMATECHKEnd), &(usr_ptr32->u4MMATECHKEnd), sizeof(__u32)))
		return -EFAULT;
#endif

	return 0;
}

static long CfaApeCompatRange(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaApePR __user *usr_ptr = NULL;
	CfaApePR32 __user *usr_ptr32 = (CfaApePR32 __user *)prInfo->usr_ptr32;
	long ret = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaApePR32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	usr_ptr = (CfaApePR *)compat_alloc_user_space(sizeof(CfaApePR));

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
		TEXT("%s line %d fail in alloc compat user space.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaApePR));
	ret = CfaApeCompatRangeInfo(usr_ptr, usr_ptr32);
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
		TEXT("%s line %d fail in CfaApeCompatRangeInfo.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}

	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaApePR);

	return 0;
}

static long CfaApeCompatJumpRange(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaApePR __user *usr_ptr = NULL;
	CfaApePR32 __user *usr_ptr32 = (CfaApePR32 __user *)prInfo->usr_ptr32;
	long ret = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaApePR32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	usr_ptr = (CfaApePR *)compat_alloc_user_space(sizeof(CfaApePR));

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
		TEXT("%s line %d fail in alloc compat user space.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaApePR));
	ret = CfaApeCompatRangeInfo(usr_ptr, usr_ptr32);
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
		TEXT("%s line %d fail in CfaApeCompatRangeInfo.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}

	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaApePR);

	return 0;
}

static long CfaApeCompatConfigCalcSz(CfaApeCfgInf32 __user *usr_ptr32,
	__u32 *pu4OutSz)
{
	__u32 u4TotalSz = 0;
	__u32 u4TotalFrames = 0;
	__u32 i = 0;

	if ((NULL == usr_ptr32) || (NULL == pu4OutSz)) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	u4TotalSz += CFA_ALIGN_SZ(sizeof(CfaApeCfgInf), sizeof(uintptr_t));

	if (0 != get_user(u4TotalFrames,	&(usr_ptr32->u4TotalFrames))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in get_user(u4TotalFrames)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	u4TotalSz += CFA_ALIGN_SZ((sizeof(CFA_APE_FRAME_INFO_T) * u4TotalFrames), sizeof(uintptr_t));
		
	*pu4OutSz = u4TotalSz;

	return 0;
}

static long CfaApeCompatConfig(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaApeCfgInf __user *usr_ptr = NULL;
	CfaApeCfgInf32 __user *usr_ptr32 = (CfaApeCfgInf32 __user *)prInfo->usr_ptr32;
	__u8 __user *pu1UsrBufAddr = NULL;
	__u8 __user *pu1NextBufAddr = NULL;
	__u32 u4TotalSz = 0;
	__u32 u4Sz = 0;
	long ret = 0;
	compat_caddr_t compatSeqHdr = 0;
	int i = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaApeCfgInf32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	if (0 != CfaApeCompatConfigCalcSz(usr_ptr32, &u4TotalSz)) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaFlvCompatConfigCalcSz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	//pu1UsrBufAddr = (__u8 __user *)compat_alloc_user_space(u4TotalSz);
	DMX_NewMemory(u4TotalSz, pu1UsrBufAddr);
	*pfgIsUserMem = FALSE;

	if (NULL == pu1UsrBufAddr) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(pu1UsrBufAddr, 0, u4TotalSz);

	usr_ptr = (CfaApeCfgInf __user *)pu1UsrBufAddr;

	pu1NextBufAddr = pu1UsrBufAddr + 
		CFA_ALIGN_SZ(sizeof(CfaApeCfgInf), sizeof(uintptr_t));
	u4Sz += CFA_ALIGN_SZ(sizeof(CfaApeCfgInf), sizeof(uintptr_t));
	if (u4Sz > u4TotalSz)
	{
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
		TEXT("%s line %d size is large than total size.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -ENOMEM;
	}
#ifdef MM_ATC_CHECK
	if (copy_from_user(&(usr_ptr->u4MMATECHKStart), &(usr_ptr32->u4MMATECHKStart), sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4MMATECHKStart).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
#endif

	if (copy_from_user(&(usr_ptr->u4TotalFrames),
		&(usr_ptr32->u4TotalFrames),
		sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4TotalFrames).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u4BlocksPerFrame),
		&(usr_ptr32->u4BlocksPerFrame),
		sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4BlocksPerFrame).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u4FinalFrameBlocks),
		&(usr_ptr32->u4FinalFrameBlocks),
		sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4FinalFrameBlocks).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u4Channels),
		&(usr_ptr32->u4Channels),
		sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4Channels).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u4SampleRate),
		&(usr_ptr32->u4SampleRate),
		sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4SampleRate).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u4BitsPerSample),
		&(usr_ptr32->u4BitsPerSample),
		sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4BitsPerSample).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u4BytesPerSample),
		&(usr_ptr32->u4BytesPerSample),
		sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4BytesPerSample).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u4BlockAlign),
		&(usr_ptr32->u4BlockAlign),
		sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4BlockAlign).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u4TotalBlocks),
		&(usr_ptr32->u4TotalBlocks),
		sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4TotalBlocks).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u4AverageBitrate),
		&(usr_ptr32->u4AverageBitrate),
		sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4AverageBitrate).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u4SeekTableElements),
		&(usr_ptr32->u4SeekTableElements),
		sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4SeekTableElements).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->fgSeekable),
		&(usr_ptr32->fgSeekable),
		sizeof(bool))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(fgSeekable).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u4AudioByteRate),
		&(usr_ptr32->u4AudioByteRate),
		sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4AudioByteRate).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->u4Duration),
		&(usr_ptr32->u4Duration),
		sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4Duration).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	if (copy_from_user(&(usr_ptr->fgAc3Type),
		&(usr_ptr32->fgAc3Type),
		sizeof(bool))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(fgAc3Type).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

#ifdef MM_ATC_CHECK
	if (copy_from_user(&(usr_ptr->u4MMATECHKEnd), &(usr_ptr32->u4MMATECHKEnd),
		sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4MMATECHKEnd).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
#endif

	if ((0 < usr_ptr32->u4TotalFrames) && (0 == usr_ptr32->pFrames)) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail for no seektable, but header len(%d) > 0.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr->u4TotalFrames);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}

	if (0 != usr_ptr32->pFrames) {
		CFA_APE_FRAME_INFO_T32 *pFrames = NULL;

		usr_ptr->pFrames = pu1NextBufAddr;

		if (NULL == usr_ptr->pFrames) {
			DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -ENOMEM;
		}
		mm_memset(usr_ptr->pFrames, 0, sizeof(CFA_APE_FRAME_INFO_T) * usr_ptr->u4TotalFrames);
		pu1NextBufAddr = pu1NextBufAddr +
			CFA_ALIGN_SZ(sizeof(CFA_APE_FRAME_INFO_T) * usr_ptr->u4TotalFrames, sizeof(uintptr_t));
		u4Sz += CFA_ALIGN_SZ(sizeof(CFA_APE_FRAME_INFO_T) * usr_ptr->u4TotalFrames, sizeof(uintptr_t));
		if (u4Sz > u4TotalSz)
		{
			DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d size is large than total size.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -ENOMEM;
		}

		if (get_user(compatSeqHdr, &(usr_ptr32->pFrames))) {
			DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_from_user(pFrames).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}
		if (0 == compatSeqHdr) {
			DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
				TEXT("%s line %d fail in compatSeqHdr == 0.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}
		pFrames = compat_ptr(compatSeqHdr);
		if (!access_ok(VERIFY_READ, pFrames,
			sizeof(CFA_APE_FRAME_INFO_T) * usr_ptr->u4TotalFrames)) {
			DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
				TEXT("%s line %d fail in access_ok(pFrames).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}
		for (i = 0; i < usr_ptr->u4TotalFrames; i++)
		{
			if (copy_from_user(&(usr_ptr->pFrames[i].u8Pos),
				&(pFrames[i].u8Pos), sizeof(__u64))) {
				DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
					TEXT("%s line %d fail in copy_from_user(pFrames[%d]).u8Pos).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, i);
				DMX_FreeMemory(pu1UsrBufAddr);
				return -EFAULT;
			}
			if (copy_from_user(&(usr_ptr->pFrames[i].u8Pts),
				&(pFrames[i].u8Pts), sizeof(__u64))) {
				DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
					TEXT("%s line %d fail in copy_from_user(pFrames[%d]).u8Pts).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, i);
				DMX_FreeMemory(pu1UsrBufAddr);
				return -EFAULT;
			}
			if (copy_from_user(&(usr_ptr->pFrames[i].u4Blocks),
				&(pFrames[i].u4Blocks), sizeof(__u32))) {
				DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
					TEXT("%s line %d fail in copy_from_user(pFrames[%d]).u4Blocks).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, i);
				DMX_FreeMemory(pu1UsrBufAddr);
				return -EFAULT;
			}
			if (copy_from_user(&(usr_ptr->pFrames[i].u4Size),
				&(pFrames[i].u4Size), sizeof(__u32))) {
				DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
					TEXT("%s line %d fail in copy_from_user(pFrames[%d]).u4Size).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, i);
				DMX_FreeMemory(pu1UsrBufAddr);
				return -EFAULT;
			}
			if (copy_from_user(&(usr_ptr->pFrames[i].u4Skip),
				&(pFrames[i].u4Skip), sizeof(__u32))) {
				DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
					TEXT("%s line %d fail in copy_from_user(pFrames[%d]).u4Skip).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, i);
				DMX_FreeMemory(pu1UsrBufAddr);
				return -EFAULT;
			}
		}
	}

	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaApeCfgInf);

	return 0;
}

static int CfaApeProcCompat(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	long ret = 0;

	if ((NULL == prInfo) || (NULL == pfgIsUserMem)) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaApeProcCompat.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	}
	switch (prInfo->type) {
	case CFA_CONFIG:
	if (prInfo->is_get) {
			DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
				TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	}
	ret = CfaApeCompatConfig(prInfo, pfgIsUserMem);
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaAudioCompatConfig.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}
	break;
	case CFA_RANGE:
    if (prInfo->is_get) {
			DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
				TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	}
	ret = CfaApeCompatRange(prInfo, pfgIsUserMem);
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaAudioCompatRange.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}
	break;
	case CFA_GEN_INFO:
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("%s line %d fail for don;t support get info for cfa audio.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	case CFA_JUMP_INFO:
		if (prInfo->is_get) {
			DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
				TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
		}
		ret = CfaApeCompatJumpRange(prInfo, pfgIsUserMem);
		if (0 != ret) {
		  DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
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
static MRESULT CfaApeInit(void *pvSptHdl, void **ppvCfaPrivData)
{
	CfaApeInst *prCfaApeInst = NULL;

	DMX_NewMemory(sizeof(CfaApeInst), prCfaApeInst);
	if (NULL == prCfaApeInst) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("[CFA_APE] Alloc prCfaApeInst memory fail\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}
	dmx_memset(prCfaApeInst, 0x00, sizeof(CfaApeInst));

	prCfaApeInst->rRange.u8Sa = 0;
	prCfaApeInst->rRange.u8Ea = 0;
	prCfaApeInst->u8CurrTxOft = DMX_INVALID_UINT64;
	prCfaApeInst->pucPbBuf = NULL;
	DMX_NewHwMemory(4 * sizeof(u8), prCfaApeInst->pucHeader);
	if (NULL == prCfaApeInst->pucHeader) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
				TEXT("[CFA_APE] Alloc prCfaApeInst Header memory fail\n"));
		DMX_FreeMemory(prCfaApeInst);
		MM_RETURN(RET_DMX_NO_MEM);
	}
	dmx_memset(prCfaApeInst->pucHeader, 0, 4 * sizeof(u8));

	*ppvCfaPrivData = (void *)prCfaApeInst;

#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaApeInst);
	MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
	MMATE_CHECK_STRUCT(prCfaApeInst->cfaApeinfo);
#endif

	MM_RETURN(RET_DMX_OK);
}


static MRESULT CfaApeUninit(void *pvSptHdl, void *pvCfaPrivData)
{
	CfaApeInst *prCfaApeInst = (CfaApeInst *) pvCfaPrivData;

	if (NULL == prCfaApeInst)
		MM_RETURN(RET_DMX_OK);
#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaApeInst);
	MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
	MMATE_CHECK_STRUCT(prCfaApeInst->cfaApeinfo);
#endif

	prCfaApeInst->pucPbBuf = NULL;
	if (NULL != prCfaApeInst->cfaApeinfo.pFrames)
		DMX_FreeMemory(prCfaApeInst->cfaApeinfo.pFrames);
	if (NULL != prCfaApeInst->pucHeader)
		DMX_FreeHwMemory(prCfaApeInst->pucHeader);
	if (NULL != prCfaApeInst)
		DMX_FreeMemory(prCfaApeInst);


	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaApeSetRange(void *pvSptHdl, void *pvRange, void *pvPrivData, bool fgIsUserMem)
{
	CfaApeInst *prCfaApeInst = (CfaApeInst *) pvPrivData;

	if (NULL == prCfaApeInst)
		MM_RETURN(RET_DMX_PARAM_WRONG);
#ifdef MM_ATE_CHECK
	if (0 != mm_copy_from_user(&(prCfaApeInst->rRange.u8Sa), &(((CfaApePR *) pvRange)->u8Sa),
			sizeof(CfaApePR) - 2 * sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("[CFA APE] %s line %d failed in mm_copy_from_user\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}

#else
	if (0 != mm_copy_from_user(&(prCfaApeInst->rRange), pvRange,
			sizeof(CfaApePR))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("[CFA APE] %s line %d failed in mm_copy_from_user\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}

#endif
#ifdef MM_ATE_CHECK
	MMATE_INIT_STRUCT(prCfaApeInst->rRange);
	MMATE_CHECK_POINTER(prCfaApeInst);
#endif

	prCfaApeInst->u8CurrTxOft = prCfaApeInst->rRange.u8Sa;
	prCfaApeInst->i4Rate = prCfaApeInst->rRange.i4Rate;

	prCfaApeInst->fgSetSeekInfo = prCfaApeInst->rRange.fgSetSeekInfo;
	prCfaApeInst->au4SeekInfo[0] = prCfaApeInst->rRange.au4SeekInfo[0];
	prCfaApeInst->au4SeekInfo[1] = prCfaApeInst->rRange.au4SeekInfo[1];
	DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_COMMON,
		TEXT("[CFA APE] %s::%d,invalid byte is %d\r\n"),
		DMX_FUNC_NAME, prCfaApeInst->fgSetSeekInfo,
		prCfaApeInst->au4SeekInfo[1]);
	prCfaApeInst->u4TxUnitRange = prCfaApeInst->rRange.u4TxUnitRange;
	prCfaApeInst->u4StartFrmNo = 0;
	prCfaApeInst->u4CurFrmNo = 0;
	if (prCfaApeInst->rRange.u8Ea < prCfaApeInst->rRange.u8Sa) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
		TEXT("[CFA_AUDIO] end offset is large than start offset!\r\n"));
		MM_RETURN(RET_DMX_EXT_PARAM_WRONG);
	}
	DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_COMMON,
		TEXT("[CFA_APE]:  Entry %s\r\n"), DMX_FUNC_NAME);
	DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_COMMON,
		TEXT("[CFA_APE]:  Rate is %d\r\n"),prCfaApeInst->i4Rate);
	DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_COMMON,
		TEXT("[CFA_APE]:  Range Start Address = 0x%x\r\n"),prCfaApeInst->rRange.u8Sa);
	DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_COMMON,
		TEXT("[CFA_APE]:  Range End Address = 0x%x\r\n"),prCfaApeInst->rRange.u8Ea);
	DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_COMMON,
		TEXT("[CFA_APE]:  Current Offset = 0x%x\r\n"),prCfaApeInst->u8CurrTxOft);
	DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_COMMON,
		TEXT("[CFA_APE]:  Exit %s!\r\n"), DMX_FUNC_NAME);

#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaApeInst);
	MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
	MMATE_CHECK_STRUCT(prCfaApeInst->cfaApeinfo);
#endif

	MM_RETURN(RET_DMX_OK);
}


static MRESULT CfaApeEnableStrm(void *pvSptHdl, u32 u4StrmToPrs, CfaStreamOp eOp, void *pvPrivData)
{
	MM_RETURN(RET_DMX_OK);
}


static MRESULT CfaApeSetStrmInf(void *pvSptHdl, u32 u4Strm, u32 u4Info, void *pvPrivData)
{
	MM_RETURN(RET_DMX_OK);
}


static MRESULT CfaApeHeaderToAfifo(void *pvSptHdl, CfaApeInst *prCfaApeInst, u32 u4Skip)
{
	MRESULT mrResult = RET_DMX_OK;
	u8 *puApeheader = NULL;
	u8 u1AuHeader[4] = { 0 };

	DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_COMMON,
		TEXT("[CFA_APE]ENTRY %s! \r\n"), DMX_FUNC_NAME);
	puApeheader = prCfaApeInst->pucHeader;

	dmx_memset(puApeheader, 0, 4 * sizeof(u8));
	u1AuHeader[0] = (u8) u4Skip;
	u1AuHeader[1] = (u8) 'E';
	u1AuHeader[2] = (u8) 'P';
	u1AuHeader[3] = (u8) 'A';

	dmx_memcpy((void *) puApeheader, (void *) (&(u1AuHeader[0])), 4 * sizeof(u8));
	mrResult = Spt4CfaBuf2AFifo(pvSptHdl, puApeheader, (u64)4, 0, CFA_AUD_DRV_FMT_APE);
	if (RET_DMX_OK != mrResult) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("[CFA_APE] Send EOS\r\n"));
		Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->u8CurrTxOft, TRUE, GAU_E_FAIL);
		MM_RETURN(RET_DMX_OK);
	}

	prCfaApeInst->eCfaApeAnaSt = CFA_APE_ANA_TX;
	DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_COMMON,
		TEXT("[CFA_APE]EXIT %s! \r\n"), DMX_FUNC_NAME);
	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaApeProcCliCmd(void *pvSptHdl, E_DMX_CFA_CLI_TYPE_T eCliType, /*< [IN] Cfa Cli Command*/
				u32 arg1,
				u32 arg2, u32 arg3, const char *szParam, void *pvPrivData)
{
	MRESULT mrRet = RET_DMX_OK;
	CfaApeInst *prCfaApe = NULL;

	prCfaApe = (CfaApeInst *) pvPrivData;

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

			DmxLogT(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
				TEXT("CfaApeProcCliCmd -- fgEnable: %d, Loglvl: %d, ModLogLvl: 0x%08x \r\n"),
				arg1, arg2, arg3);

			DmxLogEnable(fgEnable, arg2, DMX_MOD_CFA_APE, arg3);
		}
		break;
	case DMX_CFA_CLI_CMD_DUMP_INFO:
		{
			DmxLogT(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
				TEXT("Cfa APE Instance(prCfaApe is %p)")
				TEXT(" Info list as follow: \r\n"),
				prCfaApe);
			DmxLogT(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
				TEXT("Current Analyse State is %d \r\n"),
				prCfaApe->eCfaApeAnaSt);
			DmxLogT(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
				TEXT("Current Analyse Position is 0x%08x%08x, "),
				(u32) ((prCfaApe->u8CurrTxOft) >> 32), (u32) (prCfaApe->u8CurrTxOft));
		}
		break;
	default:
		break;
	}

	MM_RETURN(mrRet);
}

static MRESULT CfaApeTurnOn(void *pvSptHdl, void *pvPrivData)
{
	CfaApeInst *prCfaApeInst = (CfaApeInst *) pvPrivData;
	MRESULT mrResult = RET_DMX_OK;
	u32 u4SkipData = 0;
	u64 u8Txlen;
	u32 i = 0;
	CFA_APE_FRAME_INFO_T *pApeFrames = NULL;

	DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
		TEXT("[CFA_APE] Turn On::Sa[0X%llX],Ea[0X%llX]\r\n"),
		prCfaApeInst->rRange.u8Sa, prCfaApeInst->rRange.u8Ea);
	if (NULL == prCfaApeInst)
		MM_RETURN(RET_DMX_PARAM_WRONG);

#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaApeInst);
	MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
	MMATE_CHECK_STRUCT(prCfaApeInst->cfaApeinfo);
#endif

	pApeFrames = prCfaApeInst->cfaApeinfo.pFrames;
	if (NULL == pApeFrames)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	if (prCfaApeInst->u8CurrTxOft >= prCfaApeInst->rRange.u8Ea) {
		DmxLogT(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("[CFA_APE] line %d Send EOS\r\n"), DMX_LINE_NO);
#ifdef MM_ATE_CHECK
		MMATE_CHECK_POINTER(prCfaApeInst);
		MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
		MMATE_CHECK_STRUCT(prCfaApeInst->cfaApeinfo);
#endif

		mrResult = Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->rRange.u8Ea, FALSE, GAU_E_EOS);
		MM_RETURN(RET_DMX_OK);
	}

	if (MM_IS_NORMAL_PLAY(prCfaApeInst->i4Rate)) {
		if (prCfaApeInst->u8CurrTxOft + prCfaApeInst->u4TxUnitRange <=
		    prCfaApeInst->rRange.u8Ea) {
			mrResult =
			    Spt4CfaPbb2AFifo(pvSptHdl, prCfaApeInst->u8CurrTxOft, 0,
					     (u64)(prCfaApeInst->u4TxUnitRange), CFA_AUD_DRV_FMT_APE);

			if (RET_DMX_OK != mrResult) {
				DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
					TEXT("[CFA_APE] Send EOS\r\n"));
				Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->u8CurrTxOft, TRUE,
						  GAU_E_FAIL);
				MM_RETURN(RET_DMX_OK);
			}
		} else {
			u8Txlen = prCfaApeInst->rRange.u8Ea - prCfaApeInst->rRange.u8Sa;
			if (u8Txlen == 0) {
				prCfaApeInst->u8CurrTxOft = DMX_INVALID_UINT64;
				DmxLogT(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
						TEXT("[CFA_APE] line %d Send EOS\r\n"), DMX_LINE_NO);
#ifdef MM_ATE_CHECK
				MMATE_CHECK_POINTER(prCfaApeInst);
				MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
				MMATE_CHECK_STRUCT(prCfaApeInst->cfaApeinfo);
#endif
				Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->rRange.u8Ea, FALSE,
						  GAU_E_EOS);
				MM_RETURN(RET_DMX_OK);
			} else {
				mrResult = Spt4CfaPbb2AFifo(pvSptHdl, prCfaApeInst->u8CurrTxOft,
							    0, u8Txlen, CFA_AUD_DRV_FMT_APE);
				if (RET_DMX_OK != mrResult) {
					DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
							TEXT("[CFA_APE] Send EOS\r\n"));
					Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->u8CurrTxOft, TRUE,
							  GAU_E_FAIL);
					MM_RETURN(RET_DMX_OK);
				}
			}
		}
	} else if (MM_IS_FF_PLAY(prCfaApeInst->i4Rate)) {
		for (i = 0; i < prCfaApeInst->cfaApeinfo.u4TotalFrames; i++) {
			if (prCfaApeInst->u8CurrTxOft == pApeFrames[i].u8Pos) {
				prCfaApeInst->u4StartFrmNo = i;
				prCfaApeInst->u4CurFrmNo = i;
				u4SkipData = pApeFrames[i].u4Skip;

				DmxLogT(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
						TEXT("[CFA_APE]: Current Tx Offset is %d,")
						TEXT("Current frame number is %d\r\n"),
						(u32) prCfaApeInst->u8CurrTxOft, i);
				break;
			}
		}
		prCfaApeInst->eCfaApeAnaSt = CFA_APE_ANA_HEADER;
		mrResult = CfaApeHeaderToAfifo(pvSptHdl, prCfaApeInst, u4SkipData);
	} else if (MM_IS_RW_PLAY(prCfaApeInst->i4Rate)) {
		DmxLogT(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			    TEXT("[CFA_APE] RW:Spt4CfaPbb2AFifo,Tx Unit Range is 0x%x\r\n"),
			    prCfaApeInst->u4TxUnitRange);

		if (prCfaApeInst->u8CurrTxOft + prCfaApeInst->u4TxUnitRange <=
		    prCfaApeInst->rRange.u8Ea) {
			mrResult =
			    Spt4CfaPbb2AFifo(pvSptHdl, prCfaApeInst->u8CurrTxOft, 0,
					     (u64)prCfaApeInst->u4TxUnitRange, CFA_AUD_DRV_FMT_APE);
			if (RET_DMX_OK != mrResult) {
				DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
					TEXT("[CFA_APE] Send EOS\r\n"));

#ifdef MM_ATE_CHECK
				MMATE_CHECK_POINTER(prCfaApeInst);
				MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
				MMATE_CHECK_STRUCT(prCfaApeInst->cfaApeinfo);
#endif

				Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->u8CurrTxOft, TRUE,
						  GAU_E_FAIL);
				MM_RETURN(RET_DMX_OK);
			}
			prCfaApeInst->eCfaApeAnaSt = CFA_APE_ANA_RW;
		} else {
			u8Txlen = prCfaApeInst->rRange.u8Ea - prCfaApeInst->rRange.u8Sa;
			if (u8Txlen == 0) {
				prCfaApeInst->u8CurrTxOft = DMX_INVALID_UINT64;
				DmxLogT(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
					TEXT("[CFA_APE] line %d Send EOS\r\n"), DMX_LINE_NO);
#ifdef MM_ATE_CHECK
				MMATE_CHECK_POINTER(prCfaApeInst);
				MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
				MMATE_CHECK_STRUCT(prCfaApeInst->cfaApeinfo);
#endif

				Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->rRange.u8Ea, FALSE,
						  GAU_E_EOS);
				MM_RETURN(RET_DMX_OK);
			} else {
				mrResult = Spt4CfaPbb2AFifo(pvSptHdl, prCfaApeInst->u8CurrTxOft,
							    0, u8Txlen, CFA_AUD_DRV_FMT_APE);
				if (RET_DMX_OK != mrResult) {
					DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
						    TEXT("[CFA_APE] Send EOS\r\n"));
					Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->u8CurrTxOft, TRUE,
							  GAU_E_FAIL);
					MM_RETURN(RET_DMX_OK);
				}
				prCfaApeInst->eCfaApeAnaSt = CFA_APE_ANA_RW;
			}
		}
	} else {
	/*do nothing ,just for misra*/
	}

	MM_RETURN(mrResult);
}

static MRESULT CfaApeTxDone(void *pvSptHdl, u64 u8TxLen, void *pvPrivData, bool fgRsp)
{
	MRESULT mrResult = RET_DMX_OK;
	u64 u8LastTxlen = 0;
	u32 u4SkipData = 0;
	u32 u4RemainTime = 0;
	u32 u4TxPlayTime = 0;
	u32 u4DataSz = 0;

	CfaApeInst *prCfaApeInst = (CfaApeInst *) pvPrivData;
	CFA_APE_FRAME_INFO_T *pApeFrames = NULL;

	if (NULL == prCfaApeInst) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("[CFA_APE] prCfaApeInst is NULL!\r\n"));
		MM_RETURN(RET_DMX_EXT_NO_MEM);
	}

#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaApeInst);
	MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
	MMATE_CHECK_STRUCT(prCfaApeInst->cfaApeinfo);
#endif

	pApeFrames = prCfaApeInst->cfaApeinfo.pFrames;
	if (NULL == pApeFrames) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
		TEXT("[CFA_APE] pApeFrames is NULL!\r\n"));
		MM_RETURN(RET_DMX_EXT_NO_MEM);
	}
	DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_COMMON,
		TEXT("[CFA_APE] %s :Rate is %d\r\n"),
		DMX_FUNC_NAME, prCfaApeInst->i4Rate);

	if (MM_IS_NORMAL_PLAY(prCfaApeInst->i4Rate)) {
		prCfaApeInst->u8CurrTxOft += prCfaApeInst->u4TxUnitRange;

		if (prCfaApeInst->u8CurrTxOft >= prCfaApeInst->rRange.u8Ea) {
			prCfaApeInst->u8CurrTxOft = DMX_INVALID_UINT64;
			DmxLogT(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
					TEXT("[CFA_APE] line %d, Send EOS\r\n"), DMX_LINE_NO);
			mrResult =
			    Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->rRange.u8Ea, FALSE, GAU_E_EOS);
			MM_RETURN(RET_DMX_OK);
		}

		if (prCfaApeInst->u8CurrTxOft + prCfaApeInst->u4TxUnitRange >
		    prCfaApeInst->rRange.u8Ea) {
			u8LastTxlen = prCfaApeInst->rRange.u8Ea - prCfaApeInst->u8CurrTxOft;
			mrResult = Spt4CfaPbb2AFifo(pvSptHdl, prCfaApeInst->u8CurrTxOft, 0,
						    u8LastTxlen, CFA_AUD_DRV_FMT_APE);
			if (RET_DMX_OK != mrResult) {
				DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
						TEXT("[CFA_APE] Send EOS\r\n"));
				Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->u8CurrTxOft, TRUE,
						  GAU_E_FAIL);
				MM_RETURN(RET_DMX_OK);
			}
		} else {
			mrResult = Spt4CfaPbb2AFifo(pvSptHdl, prCfaApeInst->u8CurrTxOft, 0,
						    (prCfaApeInst->u4TxUnitRange),
						    CFA_AUD_DRV_FMT_APE);
			if (RET_DMX_OK != mrResult) {
				DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
					TEXT("[CFA_APE] Send EOS\r\n"));
				Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->u8CurrTxOft, TRUE,
						  GAU_E_FAIL);
				MM_RETURN(RET_DMX_OK);
			}
		}
	} else if (MM_IS_FF_PLAY(prCfaApeInst->i4Rate)) {
		if (CFA_APE_ANA_IDLE == prCfaApeInst->eCfaApeAnaSt) {
			prCfaApeInst->u8CurrTxOft = DMX_INVALID_UINT64;
			DmxLogT(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
					TEXT("[CFA_APE] line %d, Send EOS\r\n"), DMX_LINE_NO);
#ifdef MM_ATE_CHECK
			MMATE_CHECK_POINTER(prCfaApeInst);
			MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
			MMATE_CHECK_STRUCT(prCfaApeInst->cfaApeinfo);
#endif
			mrResult =
			    Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->rRange.u8Ea, TRUE, GAU_E_EOS);
			MM_RETURN(RET_DMX_OK);
		} else if (CFA_APE_ANA_HEADER == prCfaApeInst->eCfaApeAnaSt) {
			if (prCfaApeInst->u4CurFrmNo >= prCfaApeInst->cfaApeinfo.u4TotalFrames) {
				prCfaApeInst->u8CurrTxOft = DMX_INVALID_UINT64;
				DmxLogT(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
						TEXT("[CFA_APE] line %d, Send EOS\r\n"), DMX_LINE_NO);

#ifdef MM_ATE_CHECK
				MMATE_CHECK_POINTER(prCfaApeInst);
				MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
				MMATE_CHECK_STRUCT(prCfaApeInst->cfaApeinfo);
#endif

				Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->rRange.u8Ea, FALSE,
						  GAU_E_EOS);
				MM_RETURN(RET_DMX_OK);
			}

			u4SkipData = pApeFrames[prCfaApeInst->u4CurFrmNo].u4Skip;
			mrResult = CfaApeHeaderToAfifo(pvSptHdl, prCfaApeInst, u4SkipData);
			MM_RETURN(mrResult);
		} else if (CFA_APE_ANA_TX == prCfaApeInst->eCfaApeAnaSt) {
			if (prCfaApeInst->u4CurFrmNo < prCfaApeInst->cfaApeinfo.u4TotalFrames - 1) {
				prCfaApeInst->u4DataLenth =
				    (u32) (((pApeFrames[prCfaApeInst->u4CurFrmNo + (u32)1].u8Pos -
						pApeFrames[prCfaApeInst->u4CurFrmNo].u8Pos) +
					       (u64)3) & ((u64)(~3))) + (u32)4;

				u4RemainTime =
				    prCfaApeInst->cfaApeinfo.u4Duration / prCfaApeInst->i4Rate;
				u4TxPlayTime =
				    (prCfaApeInst->u4DataLenth * 8) /
				    prCfaApeInst->cfaApeinfo.u4AverageBitrate;

				if ((prCfaApeInst->cfaApeinfo.u4AverageBitrate != 0) &&
				    (u4RemainTime != 0) && (u4TxPlayTime > u4RemainTime)) {
					u4DataSz = (prCfaApeInst->cfaApeinfo.u4AverageBitrate *
						    (prCfaApeInst->cfaApeinfo.u4Duration /
						     prCfaApeInst->i4Rate)) / 8;
					DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_COMMON,
						    TEXT("[CFA_APE]: u4DataSz = %d\r\n"), u4DataSz);
					DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_COMMON,
						    TEXT("[CFA_APE]: line %d,prCfaApeInst->u4DataLenth = %d\r\n"),
						    DMX_LINE_NO, prCfaApeInst->u4DataLenth);

					if (prCfaApeInst->u4DataLenth > u4DataSz)
						prCfaApeInst->u4DataLenth = u4DataSz;

					DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_COMMON,
						    TEXT("[CFA_APE]: line %d,prCfaApeInst->u4DataLenth = %d\r\n"),
						    DMX_LINE_NO, prCfaApeInst->u4DataLenth);
					if (0 == prCfaApeInst->u4DataLenth)
						prCfaApeInst->u4DataLenth = 1;
				}

				DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_COMMON,
					    TEXT("[CFA_APE]: line %d,prCfaApeInst->u4DataLenth = %d\r\n"),
					    DMX_LINE_NO, prCfaApeInst->u4DataLenth);
			} else if (prCfaApeInst->u4CurFrmNo ==
				   prCfaApeInst->cfaApeinfo.u4TotalFrames - 1) {
				if (prCfaApeInst->u8CurrTxOft >= prCfaApeInst->rRange.u8Ea) {
					DmxLogT(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
						    TEXT("[CFA_APE] line %d, Send EOS\r\n"), DMX_LINE_NO);
					Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->rRange.u8Ea, TRUE,
							  GAU_E_EOS);
#ifdef MM_ATE_CHECK
					MMATE_CHECK_POINTER(prCfaApeInst);
					MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
					MMATE_CHECK_STRUCT(prCfaApeInst->cfaApeinfo);
#endif

					MM_RETURN(RET_DMX_OK);
				} else if (prCfaApeInst->u8CurrTxOft +
					   prCfaApeInst->u4TxUnitRange <=
					   prCfaApeInst->rRange.u8Ea) {
					mrResult =
					    Spt4CfaPbb2AFifo(pvSptHdl, prCfaApeInst->u8CurrTxOft, 0,
							     prCfaApeInst->u4TxUnitRange,
							     CFA_AUD_DRV_FMT_APE);
					if (RET_DMX_OK != mrResult) {
						DmxLogT(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
							    TEXT("[CFA_APE] line %d, Send EOS\r\n"), DMX_LINE_NO);
#ifdef MM_ATE_CHECK
						MMATE_CHECK_POINTER(prCfaApeInst);
						MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
						MMATE_CHECK_STRUCT(prCfaApeInst->cfaApeinfo);
#endif
						Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->rRange.u8Ea,
								  TRUE, GAU_E_EOS);
					}
					prCfaApeInst->eCfaApeAnaSt = CFA_APE_ANA_IDLE;
					MM_RETURN(mrResult);
				} else {
					u8LastTxlen =
					    prCfaApeInst->rRange.u8Ea - prCfaApeInst->u8CurrTxOft;
					mrResult =
					    Spt4CfaPbb2AFifo(pvSptHdl, prCfaApeInst->u8CurrTxOft, 0,
							     u8LastTxlen, CFA_AUD_DRV_FMT_APE);
					if (RET_DMX_OK != mrResult) {
						DmxLogT(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
							    TEXT("[CFA_APE] line %d, Send EOS\r\n"), DMX_LINE_NO);
#ifdef MM_ATE_CHECK
						MMATE_CHECK_POINTER(prCfaApeInst);
						MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
						MMATE_CHECK_STRUCT(prCfaApeInst->cfaApeinfo);
#endif
						Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->rRange.u8Ea,
								  TRUE, GAU_E_EOS);
					}
					prCfaApeInst->eCfaApeAnaSt = CFA_APE_ANA_IDLE;
					MM_RETURN(mrResult);
				}
			} else{
			/*(prCfaApeInst->u4CurFrmNo >  prCfaApeInst->cfaApeinfo.u4TotalFrames - 1)	*/
				prCfaApeInst->u8CurrTxOft = DMX_INVALID_UINT64;
				DmxLogT(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
						TEXT("[CFA_APE] line %d, Send EOS\r\n"), DMX_LINE_NO);
#ifdef MM_ATE_CHECK
				MMATE_CHECK_POINTER(prCfaApeInst);
				MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
				MMATE_CHECK_STRUCT(prCfaApeInst->cfaApeinfo);
#endif
				mrResult = Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->rRange.u8Ea, TRUE,
						      GAU_E_EOS);
				MM_RETURN(RET_DMX_OK);
			}

			prCfaApeInst->u8CurrTxOft = pApeFrames[prCfaApeInst->u4CurFrmNo].u8Pos;
		} else {
				/*do nothing, just for misra*/
		}

		if (prCfaApeInst->u4DataLenth <= 0) {
			prCfaApeInst->u8CurrTxOft = DMX_INVALID_UINT64;
			DmxLogT(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
					TEXT("[CFA_APE] line %d, Send EOS\r\n"), DMX_LINE_NO);
#ifdef MM_ATE_CHECK
			MMATE_CHECK_POINTER(prCfaApeInst);
			MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
			MMATE_CHECK_STRUCT(prCfaApeInst->cfaApeinfo);
#endif
			mrResult =
			    Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->rRange.u8Ea, FALSE, GAU_E_EOS);
			MM_RETURN(RET_DMX_OK);
		}
		if (prCfaApeInst->u8CurrTxOft >= prCfaApeInst->rRange.u8Ea) {
			prCfaApeInst->u8CurrTxOft = DMX_INVALID_UINT64;
			DmxLogT(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
					TEXT("[CFA_APE] line %d, Send EOS\r\n"), DMX_LINE_NO);
#ifdef MM_ATE_CHECK
			MMATE_CHECK_POINTER(prCfaApeInst);
			MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
			MMATE_CHECK_STRUCT(prCfaApeInst->cfaApeinfo);
#endif
			mrResult =
			    Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->rRange.u8Ea, FALSE, GAU_E_EOS);
			MM_RETURN(RET_DMX_OK);
		}

		if (prCfaApeInst->u8CurrTxOft + prCfaApeInst->u4DataLenth >
		    prCfaApeInst->rRange.u8Ea) {
			u8LastTxlen = prCfaApeInst->rRange.u8Ea - prCfaApeInst->u8CurrTxOft;
			DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_COMMON,
				    TEXT("[CFA_APE]: Spt4CfaPbb2AFifo--u4Txlen= 0X%X!/n"), u8LastTxlen);
			mrResult =
			    Spt4CfaPbb2AFifo(pvSptHdl, prCfaApeInst->u8CurrTxOft, 0, u8LastTxlen,
					     CFA_AUD_DRV_FMT_APE);
			if (RET_DMX_OK != mrResult) {
				DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
						TEXT("[CFA_APE] Send EOS\r\n"));
				Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->u8CurrTxOft, TRUE, GAU_E_FAIL);
				MM_RETURN(RET_DMX_OK);
			}
		} else {
			prCfaApeInst->u4CurFrmNo += prCfaApeInst->i4Rate;

			mrResult = Spt4CfaPbb2AFifo(pvSptHdl, prCfaApeInst->u8CurrTxOft, 0,
						    prCfaApeInst->u4DataLenth, CFA_AUD_DRV_FMT_APE);
			if (RET_DMX_OK != mrResult) {
				DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
						TEXT("[CFA_APE] Send EOS\r\n"));
				Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->u8CurrTxOft, TRUE, GAU_E_FAIL);
				MM_RETURN(RET_DMX_OK);
			}

			prCfaApeInst->eCfaApeAnaSt = CFA_APE_ANA_HEADER;

			prCfaApeInst->u8CurrTxOft += prCfaApeInst->u4DataLenth;
		}
	} else if (MM_IS_RW_PLAY(prCfaApeInst->i4Rate)) {
		if (CFA_APE_ANA_RW == prCfaApeInst->eCfaApeAnaSt) {
			DmxLogT(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
					TEXT("[CFA_APE]: %s--RW--Send EOS \r\n"), DMX_FUNC_NAME);
#ifdef MM_ATE_CHECK
			MMATE_CHECK_POINTER(prCfaApeInst);
			MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
			MMATE_CHECK_STRUCT(prCfaApeInst->cfaApeinfo);
#endif
			Spt4CfaFinishedEx(pvSptHdl, prCfaApeInst->rRange.u8Ea, FALSE, GAU_E_EOS);
			MM_RETURN(RET_DMX_OK);
		} else {
			MM_RETURN(RET_DMX_UNEXPECT);
		}
	} else {
		/* do nothing*/
	}

	MM_RETURN(RET_DMX_OK);
}


static MRESULT CfaApeFillAUInfo(void *pvSptHdl, void *pvAUInfo, void *pvAUExtInfo, void *pvPrivData)
{
	CfaApeInst *prCfaApeInst = (CfaApeInst *) pvPrivData;

	if ((NULL != pvAUExtInfo) && (NULL != prCfaApeInst)) {
		((AU_AUDIO_EXT_INFO_T *) pvAUExtInfo)->rApe.fgSetSeekInfo =
		    prCfaApeInst->fgSetSeekInfo;
		((AU_AUDIO_EXT_INFO_T *) pvAUExtInfo)->rApe.au4SeekInfo[0] =
		    prCfaApeInst->au4SeekInfo[0];
		((AU_AUDIO_EXT_INFO_T *) pvAUExtInfo)->rApe.au4SeekInfo[1] =
		    prCfaApeInst->au4SeekInfo[1];

		if (prCfaApeInst->fgSetSeekInfo) {
			DmxLogT(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
					TEXT("[CFA APE] CfaApeFillAUExtInfo -- SeekInfo[0]: 0x%02x,")
					TEXT("SeekInfo[1]: 0x%02x\r\n"),
					((AU_AUDIO_EXT_INFO_T *) pvAUExtInfo)->rApe.au4SeekInfo[0],
					((AU_AUDIO_EXT_INFO_T *) pvAUExtInfo)->rApe.au4SeekInfo[1]);
		}

		prCfaApeInst->fgSetSeekInfo = FALSE;
	}

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaApeCurPos(void *pvSptHdl, void *pvCurPos, void *pvPrivData)
{
	CfaApeInst *prCfaApeInst = (CfaApeInst *) pvPrivData;
	u64 *pvu8 = pvCurPos;

	prCfaApeInst = (CfaApeInst *) pvPrivData;

	*pvu8 = prCfaApeInst->u8CurrTxOft;

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaApeConfigure(void *pvSptHdl, void *pvParam, void *pvPrivData, bool fgIsUserMem)
{
	CfaApeInst *prCfaApeInst;
	CfaApeCfgInf *prCfaApeCfgInf;
	CfaApeCfgInf rCfaApeCfgInf;

	if ((NULL == pvParam) || (NULL == pvPrivData)) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
		TEXT("[CFA_APE] the parameter is invalid\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
		TEXT("[CFA_APE]:  Entry %s!\r\n"), DMX_FUNC_NAME);
	mm_memset(&rCfaApeCfgInf, 0, sizeof(rCfaApeCfgInf));
	if (fgIsUserMem) {
		if (0 != mm_copy_from_user(&rCfaApeCfgInf,
			pvParam, sizeof(CfaApeCfgInf))) {
			DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
				TEXT("[CFA_APE] %s line %d failed in mm_copy_from_user\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
		}
	} else {
		mm_memcpy(&rCfaApeCfgInf,
			pvParam, sizeof(CfaApeCfgInf));
	}
	prCfaApeCfgInf = &rCfaApeCfgInf;

	prCfaApeInst = (CfaApeInst *) pvPrivData;

#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaApeInst);
	MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
#endif

	prCfaApeInst->fgAc3Type = prCfaApeCfgInf->fgAc3Type;
	prCfaApeInst->cfaApeinfo.fgAc3Type = prCfaApeCfgInf->fgAc3Type;
	prCfaApeInst->cfaApeinfo.u4AverageBitrate = prCfaApeCfgInf->u4AverageBitrate;
	prCfaApeInst->cfaApeinfo.u4Duration = prCfaApeCfgInf->u4Duration;
	DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_COMMON,
		TEXT("[CFA_APE] Average Bitrate is %d\n"),
		prCfaApeInst->cfaApeinfo.u4AverageBitrate);
	DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_COMMON,
		TEXT("[CFA_APE] Duration is %d\n"),
		prCfaApeInst->cfaApeinfo.u4Duration);
	prCfaApeInst->cfaApeinfo.fgSeekable = prCfaApeCfgInf->fgSeekable;
	prCfaApeInst->cfaApeinfo.u4TotalFrames = prCfaApeCfgInf->u4TotalFrames;

	if (prCfaApeInst->cfaApeinfo.u4TotalFrames != 0) {
		DMX_NewMemory(prCfaApeInst->cfaApeinfo.u4TotalFrames * sizeof(CFA_APE_FRAME_INFO_T),
			      prCfaApeInst->cfaApeinfo.pFrames);
		if (NULL == prCfaApeInst->cfaApeinfo.pFrames) {

			DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
				TEXT("[CFA_APE] Alloc prCfaApeInst->cfaApeinfo.pFrames memory fail \r\n"));
			if (!fgIsUserMem) {
				DMX_FreeMemory(pvParam);
			}
			MM_RETURN(RET_DMX_NO_MEM);
		}

		dmx_memset(prCfaApeInst->cfaApeinfo.pFrames, 0x00,
			   prCfaApeCfgInf->u4TotalFrames * sizeof(CFA_APE_FRAME_INFO_T));
		if (fgIsUserMem) {
			if (0 != mm_copy_from_user(prCfaApeInst->cfaApeinfo.pFrames,
					prCfaApeCfgInf->pFrames,
					prCfaApeCfgInf->u4TotalFrames *
					sizeof(CFA_APE_FRAME_INFO_T))) {
				DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
					TEXT("[CFA_APE] copy from user fail\r\n"));
				MM_RETURN(RET_DMX_EXT_EXCEPTION);
			}
			} else {
				dmx_memcpy(prCfaApeInst->cfaApeinfo.pFrames,
					prCfaApeCfgInf->pFrames,
					prCfaApeCfgInf->u4TotalFrames *
					sizeof(CFA_APE_FRAME_INFO_T));
		}
	}
#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaApeInst);
	MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
#endif
	if (!fgIsUserMem) {
		DMX_FreeMemory(pvParam);
	}
	DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
		TEXT("[CFA_APE]:  Exit %s!\r\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaApeSetJumpRange(void *pvSptHdl, void *pvJmpRange, void *pvPrivData)
{
	CfaApeInst *prCfaApeInst = NULL;
	CfaApePR *prCfaApeKeyFrmRange = (CfaApePR *) pvJmpRange;

	if ((NULL == pvPrivData) || (NULL == pvJmpRange)) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
		TEXT("[CFA_APE] pvPrivData or pvJmpRange is NULL!\r\n"));
		MM_RETURN(RET_DMX_EXT_NO_MEM);
	}
	prCfaApeInst = (CfaApeInst *) pvPrivData;

#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaApeInst);
	MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
#endif

#ifdef MM_ATE_CHECK
	dmx_memcpy(&(prCfaApeInst->rRange.u8Sa), prCfaApeKeyFrmRange->u8Sa,
		   sizeof(CfaApePR) - 2 * sizeof(u32));
	if (0 != mm_copy_from_user(&(prCfaApeInst->rRange.u8Sa), &(prCfaApeKeyFrmRange->u8Sa),
			sizeof(CfaApePR) - 2 * sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("[CFA APE] %s line %d failed in mm_copy_from_user\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}
#else
	dmx_memcpy(&(prCfaApeInst->rRange), prCfaApeKeyFrmRange, sizeof(CfaApePR));
	if (0 != mm_copy_from_user(&(prCfaApeInst->rRange), prCfaApeKeyFrmRange,
			sizeof(CfaApePR))) {
		DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
			TEXT("[CFA APE] %s line %d failed in mm_copy_from_user\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}

#endif

	prCfaApeInst->u8CurrTxOft = prCfaApeInst->rRange.u8Sa;
	prCfaApeInst->i4Rate = prCfaApeInst->rRange.i4Rate;
	prCfaApeInst->fgSetSeekInfo = prCfaApeInst->rRange.fgSetSeekInfo;
	prCfaApeInst->au4SeekInfo[0] = prCfaApeInst->rRange.au4SeekInfo[0];
	prCfaApeInst->au4SeekInfo[1] = prCfaApeInst->rRange.au4SeekInfo[1];
	/* CFA_APE_UNIT_RANGE_SIZE*/
	prCfaApeInst->u4TxUnitRange = prCfaApeInst->rRange.u4UnitTxDataSz;
	prCfaApeInst->u4StartFrmNo = 0;
	prCfaApeInst->u4CurFrmNo = 0;

	if (prCfaApeInst->rRange.u8Ea < prCfaApeInst->rRange.u8Sa) {
        DmxLogE(DMX_MOD_CFA_APE, CFA_APE_LOG_DEFAULT,
		TEXT("[CFA_AUDIO] end offset is large than start offset!\r\n"));
		MM_RETURN(RET_DMX_EXT_PARAM_WRONG);
	}
	DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_FFRW,
		TEXT("[CFA APE] %s::Current Tx Offset[0X%llX]\r\n"),
		DMX_FUNC_NAME, prCfaApeInst->u8CurrTxOft);
	DmxLogD(DMX_MOD_CFA_APE, CFA_APE_LOG_FFRW,
		TEXT("[CFA APE] %s::invalid byte is %d\r\n"),
		DMX_FUNC_NAME, prCfaApeInst->au4SeekInfo[1]);

#ifdef MM_ATE_CHECK
	MMATE_CHECK_STRUCT(prCfaApeInst->rRange);
#endif
	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaApeGetParamSize(void *pvSptHdl, u32 u4ParamID,
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
				*pu4Tmp = sizeof(CfaApePR);
			}
			break;
		}

	default:
		mrResult = RET_DMX_PARAM_WRONG;
		break;
	}

	MM_RETURN(mrResult);
}

CfaIntf _rApeCfaIntf = {
	&CfaApeInit,
	&CfaApeUninit,
	&CfaApeSetRange,
	&CfaApeEnableStrm,
	&CfaApeSetStrmInf,
	&CfaApeTurnOn,
	&CfaApeTxDone,
	&CfaApeCurPos,
	NULL,
	&CfaApeConfigure,
	NULL,
	NULL,
	NULL,
	NULL,
	&CfaApeFillAUInfo,
	NULL,
	NULL,
	&CfaApeSetJumpRange,
	&CfaApeGetParamSize,
	&CfaApeProcCliCmd
	#ifdef CONFIG_COMPAT
	,&CfaApeProcCompat
	#endif
};

EXTERN void *CfaApeGetInterface(void);


void *CfaApeGetInterface(void)
{
	return ((void *) &_rApeCfaIntf);
}
