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
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/drv_esm_if.h>
/* #include <media/atc/mm_debug.h> */

#include "cfa_rm.h"
#include "cfa_rm_st_ctrl.h"
#include "dmx_def.h"
#include "dmx_mem.h"
#include "cfa_macro.h"
#include "mmisc.h"


//#pragma warning(disable : 4100) /* disable warning C4100: unreferenced formal parameter*/
//#pragma warning(disable : 4127) /* disable warning C4127: conditional expression is constant*/

/*-----------------------------------------------------------------------------
			macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
			function prototype
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
	pvSptHdl: Provided by splitter.  When using API in splitter4cfa.h, CFA should pass this handle as the 1st parameter.
	pvPrivData: Provided by App in MPC_CMD_INIT as MPC2FFDescr.pvCfaPrivData.
	MPC passes it to splitter which passes to CFA.
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Name: CfaRmInit
 *
 * Description:
 *		Init CFA RM
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
#ifdef CONFIG_COMPAT
#include <linux/compat.h>

typedef struct {
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKStart;
#endif
	bool fgEnableVid;
	bool fgEnableAud;
	bool fgVidValid;	/* second range */
	bool fgAudValid;	/* second range */
	__u64 u8VidSa;
	__u64 u8VidEa;

	__u64 u8AudSa;
	__u64 u8AudEa;
	__u64 u8RealVidSa;

	__u64 u8targetTime;

	__u32 u4JumpUnitSz;
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKEnd;
#endif
} CfaRmRange_T32;

typedef struct {
	__u64 u8FileSize;
	__u32 u4HeaderSize;
	__u32 u4StreamNum;
	bool fgHasVideo;
	compat_caddr_t prStreamInfo;
	bool fgCfgRespliter;
}  CfaRmCfgFileInfo_T32;

typedef struct {
	__u8 uObjectProfile;
	__u8 uSampFreqIdx;
	__u8 uChannels;
	__u8 uHeaderLen;
	compat_caddr_t puHeader;
} RmAacCfgInfo_T32;

typedef struct {
	__u8 u1StrmNum;	/* CFA_RM_AUDIO_STREAM_ID */
	__u8 u1NumChannels;
	AVCODECID_T eCodecID;
	CfaRmAudioType_E eSoundType;	/* the number of audio channel */
	__u32 u4SoundRate;	/* Samples per second ;u4SampleRate */
	__u16 u2SoundSize;	/* Size of each sample;u2SampleSize */
	__u16 u2FlavorIdx;
	__u16 u2InterleaveFactor;
	__u16 u2BlockSize;
	__u16 u2FrameSize;	/* u2CodecFrameSize */
	__u16 u2SamplePerFrame;
	__u16 u2NumRegions;
	__u16 u2CplStart;
	__u16 u2CplQBits;
	__u32 u4MaxPacketSize;
	__u32 u4AvgPacketSize;
	RmAacCfgInfo_T32 rAacInfo;
}  CfaRmCfgAudInfo_T32;

typedef struct {
	CfaRmCfgFileInfo_T32 rCfaRmCfgFileInfo;
	CfaRmCfgAudInfo_T32 rCfaRmCfgAudInfo;
	CfaRmCfgVidInfo_T rCfaRmCfgVidInfo;
}  CfaRmCfgInfo_T32;

static long CfaRmCompatCalcSz(CfaRmCfgInfo_T32 __user *usr_ptr32, __u32 *pu4OutSz)
{
	__u8 u1HeaderLen = 0;
	__u32 u4TotalSize = 0;
	__u32 u4StreamNum = 0;

	if (NULL == usr_ptr32 || NULL == pu4OutSz) {
		DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32 or pu4OutSz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	u4TotalSize += CFA_ALIGN_SZ(sizeof(CfaRmCfgInfo_T), sizeof(uintptr_t));

	if (get_user(u4StreamNum, &(usr_ptr32->rCfaRmCfgFileInfo.u4StreamNum))) {
		DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
			TEXT("%s line %d fail in get_user(u4StreamNum).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (0 == u4StreamNum)
	{
		DmxLogT(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
			TEXT("%s line %d u4StreamNum is %d.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4StreamNum);
	}

	u4TotalSize += CFA_ALIGN_SZ(sizeof(RM_STREAM_TYPE_INFO_T) * u4StreamNum, sizeof(uintptr_t));

	if (get_user(u1HeaderLen, &(usr_ptr32->rCfaRmCfgAudInfo.rAacInfo.uHeaderLen))) {
		DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
			TEXT("%s line %d fail in get_user(u4HeaderDataSize).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	
	u4TotalSize += CFA_ALIGN_SZ(u1HeaderLen, sizeof(uintptr_t));

	*pu4OutSz = u4TotalSize;

	return 0;
}

static long CfaRmCompatRange(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaRmRange_T __user *usr_ptr = NULL;
	CfaRmRange_T32 __user *usr_ptr32 = (CfaRmRange_T32 __user *)prInfo->usr_ptr32;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaRmRange_T32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	usr_ptr = (CfaRmRange_T *)compat_alloc_user_space(sizeof(CfaRmRange_T));

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaRmRange_T));
	if (copy_in_user(&(usr_ptr->fgEnableVid), &(usr_ptr32->fgEnableVid), sizeof(bool)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->fgEnableAud), &(usr_ptr32->fgEnableAud), sizeof(bool)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->fgVidValid), &(usr_ptr32->fgVidValid), sizeof(bool)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->fgAudValid), &(usr_ptr32->fgAudValid), sizeof(bool)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8VidSa), &(usr_ptr32->u8VidSa), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8VidEa), &(usr_ptr32->u8VidEa), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8AudSa), &(usr_ptr32->u8AudSa), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8AudEa), &(usr_ptr32->u8AudEa), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8RealVidSa), &(usr_ptr32->u8RealVidSa), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8targetTime), &(usr_ptr32->u8targetTime), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u4JumpUnitSz), &(usr_ptr32->u4JumpUnitSz), sizeof(__u32)))
		return -EFAULT;

	*pfgIsUserMem = TRUE;
	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaRmRange_T);

	return 0;
}

static long CfaRmCompatCfgFileInfo(CfaRmCfgFileInfo_T __user *usr_ptr,
	CfaRmCfgFileInfo_T32 __user *usr_ptr32)
{
	if (copy_from_user(&(usr_ptr->u8FileSize), &(usr_ptr32->u8FileSize), sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4HeaderSize), &(usr_ptr32->u4HeaderSize), sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4StreamNum), &(usr_ptr32->u4StreamNum), sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->fgHasVideo), &(usr_ptr32->fgHasVideo), sizeof(bool)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->fgCfgRespliter), &(usr_ptr32->fgCfgRespliter), sizeof(bool)))
		return -EFAULT;

	return 0;
}

static long CfaRmCompatConfigAudInfo(CfaRmCfgAudInfo_T __user *usr_ptr,
	CfaRmCfgAudInfo_T32 __user *usr_ptr32)
{
	if (copy_from_user(&(usr_ptr->u1StrmNum), &(usr_ptr32->u1StrmNum), sizeof(__u8)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u1NumChannels), &(usr_ptr32->u1NumChannels), sizeof(__u8)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->eCodecID), &(usr_ptr32->eCodecID), sizeof(AVCODECID_T)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->eSoundType), &(usr_ptr32->eSoundType), sizeof(CfaRmAudioType_E)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4SoundRate), &(usr_ptr32->u4SoundRate), sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u2SoundSize), &(usr_ptr32->u2SoundSize), sizeof(__u16)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u2FlavorIdx), &(usr_ptr32->u2FlavorIdx), sizeof(__u16)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u2InterleaveFactor), &(usr_ptr32->u2InterleaveFactor), sizeof(__u16)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u2BlockSize), &(usr_ptr32->u2BlockSize), sizeof(__u16)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u2FrameSize), &(usr_ptr32->u2FrameSize), sizeof(__u16)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u2SamplePerFrame), &(usr_ptr32->u2SamplePerFrame), sizeof(__u16)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u2NumRegions), &(usr_ptr32->u2NumRegions), sizeof(__u16)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u2CplStart), &(usr_ptr32->u2CplStart), sizeof(__u16)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u2CplQBits), &(usr_ptr32->u2CplQBits), sizeof(__u16)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4MaxPacketSize), &(usr_ptr32->u4MaxPacketSize), sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4AvgPacketSize), &(usr_ptr32->u4AvgPacketSize), sizeof(__u32)))
		return -EFAULT;

	if (copy_from_user(&(usr_ptr->rAacInfo.uObjectProfile), &(usr_ptr32->rAacInfo.uObjectProfile), sizeof(__u8)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->rAacInfo.uSampFreqIdx), &(usr_ptr32->rAacInfo.uSampFreqIdx), sizeof(__u8)))
		return -EFAULT;

	if (copy_from_user(&(usr_ptr->rAacInfo.uChannels), &(usr_ptr32->rAacInfo.uChannels), sizeof(__u8)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->rAacInfo.uHeaderLen), &(usr_ptr32->rAacInfo.uHeaderLen), sizeof(__u8)))
		return -EFAULT;

	return 0;
}

static long CfaRmCompatConfig(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaRmCfgInfo_T __user *usr_ptr = NULL;
	CfaRmCfgInfo_T32 __user *usr_ptr32 = (CfaRmCfgInfo_T32 __user *)prInfo->usr_ptr32;
	__u8 __user *pu1UsrBufAddr = NULL;
	__u8 __user *pu1NextBufAddr = NULL;
	__u32 u4TotalSz = 0;
	long ret = 0;
	compat_caddr_t compatStreamInfo = 0;
	RM_STREAM_TYPE_INFO_T __user *stream_info = NULL;
	compat_caddr_t compatAachdr = 0;
	__u8 __user *aac_hdr = NULL;
	__u32 u4UseSz = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaRmCfgInfo_T32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	ret = CfaRmCompatCalcSz(usr_ptr32, &u4TotalSz);
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaOgmCompatCalcSz: u4TotalSz:%d.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4TotalSz);
	}

	DmxLogT(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
			TEXT("%s line %d ---- CfaOgmCompatCalcSz: u4TotalSz:%d.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4TotalSz);

	//pu1UsrBufAddr = (__u8 *)compat_alloc_user_space(u4TotalSz);
	DMX_NewMemory(u4TotalSz, pu1UsrBufAddr);

	if (NULL == pu1UsrBufAddr) {
		DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(pu1UsrBufAddr, 0, u4TotalSz);

	usr_ptr = (CfaRmCfgInfo_T *)pu1UsrBufAddr;

	pu1NextBufAddr = pu1UsrBufAddr + CFA_ALIGN_SZ(sizeof(CfaRmCfgInfo_T), sizeof(uintptr_t));
	u4UseSz += CFA_ALIGN_SZ(sizeof(CfaRmCfgInfo_T), sizeof(uintptr_t));
	if (u4UseSz > u4TotalSz) {
		DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
			TEXT("%s line %d fail for u4UseSz > u4TotalSz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}

	ret = CfaRmCompatCfgFileInfo(&(usr_ptr->rCfaRmCfgFileInfo), &(usr_ptr32->rCfaRmCfgFileInfo));
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaRmCompatCfgFileInfo.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return ret;
	}

	ret = CfaRmCompatConfigAudInfo(&(usr_ptr->rCfaRmCfgAudInfo), &(usr_ptr32->rCfaRmCfgAudInfo));
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaRmCompatConfigAudInfo.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return ret;
	}

	if (copy_from_user(&(usr_ptr->rCfaRmCfgVidInfo.u1StrmNum), &(usr_ptr32->rCfaRmCfgVidInfo.u1StrmNum), sizeof(__u8))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rCfaRmCfgVidInfo.u1FrameRate), &(usr_ptr32->rCfaRmCfgVidInfo.u1FrameRate), sizeof(__u8))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rCfaRmCfgVidInfo.eCodecID), &(usr_ptr32->rCfaRmCfgVidInfo.eCodecID), sizeof(AVCODECID_T))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
		
	if (copy_from_user(&(usr_ptr->rCfaRmCfgVidInfo.eCodecVer), &(usr_ptr32->rCfaRmCfgVidInfo.eCodecVer), sizeof(VCODECVERSION_T))) {
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	DmxLogT(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
			TEXT("%s line %d, CfaRmCompatCfgFileInfo, eCodecID:%d  eCodecVer:%d.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr->rCfaRmCfgVidInfo.eCodecID, usr_ptr->rCfaRmCfgVidInfo.eCodecVer);
	if (0 != usr_ptr32->rCfaRmCfgFileInfo.prStreamInfo) {
		usr_ptr->rCfaRmCfgFileInfo.prStreamInfo = (RM_STREAM_TYPE_INFO_T *)pu1NextBufAddr;
		pu1NextBufAddr += CFA_ALIGN_SZ(sizeof(RM_STREAM_TYPE_INFO_T) * usr_ptr32->rCfaRmCfgFileInfo.u4StreamNum, sizeof(uintptr_t));
		u4UseSz += CFA_ALIGN_SZ(sizeof(RM_STREAM_TYPE_INFO_T) * usr_ptr32->rCfaRmCfgFileInfo.u4StreamNum, sizeof(uintptr_t));
		if (u4UseSz > u4TotalSz) {
			DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
				TEXT("%s line %d fail for u4UseSz > u4TotalSz.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}

		if (NULL == usr_ptr->rCfaRmCfgFileInfo.prStreamInfo) {
			DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
				TEXT("%s line %d fail in alloc compat user space.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -ENOMEM;
		}
		mm_memset(usr_ptr->rCfaRmCfgFileInfo.prStreamInfo, 0, 
					sizeof(RM_STREAM_TYPE_INFO_T) * usr_ptr32->rCfaRmCfgFileInfo.u4StreamNum);

		if (get_user(compatStreamInfo, &(usr_ptr32->rCfaRmCfgFileInfo.prStreamInfo))) {
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}
		if (0 == compatStreamInfo) {
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}
		stream_info = compat_ptr(compatStreamInfo);
		if (!access_ok(VERIFY_READ, stream_info, sizeof(RM_STREAM_TYPE_INFO_T) * usr_ptr32->rCfaRmCfgFileInfo.u4StreamNum)) {
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}

		if (copy_from_user((__u8 __user *)usr_ptr->rCfaRmCfgFileInfo.prStreamInfo,
			stream_info, sizeof(RM_STREAM_TYPE_INFO_T) * usr_ptr32->rCfaRmCfgFileInfo.u4StreamNum)) {
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}
	}

	if ((0 != usr_ptr32->rCfaRmCfgAudInfo.rAacInfo.puHeader) &&
			(0 < usr_ptr32->rCfaRmCfgAudInfo.rAacInfo.uHeaderLen)) {
		usr_ptr->rCfaRmCfgAudInfo.rAacInfo.puHeader = (__u8 *)pu1NextBufAddr;
		u4UseSz += CFA_ALIGN_SZ(sizeof(__u8) * usr_ptr32->rCfaRmCfgAudInfo.rAacInfo.uHeaderLen, sizeof(uintptr_t));
		if (u4UseSz > u4TotalSz) {
			DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
				TEXT("%s line %d fail for u4UseSz > u4TotalSz.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EINVAL;
		}

		if (NULL == usr_ptr->rCfaRmCfgAudInfo.rAacInfo.puHeader) {
			DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
				TEXT("%s line %d fail for address is null.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -ENOMEM;
		}
		mm_memset(usr_ptr->rCfaRmCfgAudInfo.rAacInfo.puHeader, 0, sizeof(__u8) * usr_ptr32->rCfaRmCfgAudInfo.rAacInfo.uHeaderLen);

		if (get_user(compatAachdr, &(usr_ptr32->rCfaRmCfgAudInfo.rAacInfo.puHeader))) {
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}
		if (0 == compatAachdr) {
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}
		aac_hdr = compat_ptr(compatAachdr);
		if (!access_ok(VERIFY_READ, aac_hdr, 
			sizeof(__u8) * usr_ptr->rCfaRmCfgAudInfo.rAacInfo.uHeaderLen)) {
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}

		if (copy_from_user((__u8 __user *)usr_ptr->rCfaRmCfgAudInfo.rAacInfo.puHeader,
			aac_hdr, sizeof(__u8) * usr_ptr->rCfaRmCfgAudInfo.rAacInfo.uHeaderLen)) {
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}
	}

	*pfgIsUserMem = FALSE;
	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaRmCfgInfo_T);

	return 0;
}

static long CfaRmProcCompat(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	long ret = 0;

	if ((NULL == prInfo) || (NULL == pfgIsUserMem)) {
		DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
					TEXT("%s line %d fail for invalid parameter.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	}
	switch (prInfo->type) {
		case CFA_CONFIG:
			if (prInfo->is_get) {
				DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
					TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EPERM;
			}
			ret = CfaRmCompatConfig(prInfo, pfgIsUserMem);
			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaRmCompatConfig.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return ret;
			}
			break;
		case CFA_RANGE:
		case CFA_JUMP_INFO:
			if (prInfo->is_get) {
				DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
					TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EPERM;
			}
			ret = CfaRmCompatRange(prInfo, pfgIsUserMem);
			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaRmCompatRange.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return ret;
			}
			break;
		case CFA_GEN_INFO:
			DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
				TEXT("%s line %d fail for don't support get info for cfa rm.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return -EPERM;
		default:
			break;
	}

	return 0;
}

#endif

void vCfaRmInternalMemFree(CfaRmInst_T * prCfaRm)
{
	if (NULL == prCfaRm)
		return;

	if (NULL != prCfaRm->puAudioBuf) {
		DMX_FreeHwMemory(prCfaRm->puAudioBuf);
		prCfaRm->puAudioBuf = NULL;
	}

	if (NULL != prCfaRm->rAudioInfo.pauAdtsHeader) {
		DMX_FreeHwMemory(prCfaRm->rAudioInfo.pauAdtsHeader);
		prCfaRm->rAudioInfo.pauAdtsHeader = NULL;
	}

	if (NULL != prCfaRm->rFileInfo.prStreamInfo) {
		DMX_FreeMemory(prCfaRm->rFileInfo.prStreamInfo);
		prCfaRm->rFileInfo.prStreamInfo = NULL;
	}

	if (prCfaRm->rAacCfgInfo.puHeader != NULL) {
		DMX_FreeHwMemory(prCfaRm->rAacCfgInfo.puHeader);
		prCfaRm->rAacCfgInfo.puHeader  = NULL;
	}

	return ;
}

static MRESULT CfaRmInit(void *pvSptHdl, void **ppvCfaPrivData)
{
	CfaRmInst_T *prCfaRm = NULL;

	DMX_NewMemory(sizeof(CfaRmInst_T), prCfaRm);
	if (NULL == prCfaRm) {
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM] Alloc prCfaRm memory failed\r\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}

	dmx_memset(prCfaRm, 0, sizeof(CfaRmInst_T));
	DMX_NewHwMemory(CFA_RM_AUDIO_BUFFER_SIZE, prCfaRm->puAudioBuf);

	if (NULL == prCfaRm->puAudioBuf) {
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM] Alloc prCfaRm->puAudioBuf memory failed\r\n"));
		vCfaRmInternalMemFree(prCfaRm);
		DMX_FreeMemory(prCfaRm);
		prCfaRm = NULL;
		MM_RETURN(RET_DMX_NO_MEM);
	}

	DMX_NewHwMemory(8, prCfaRm->rAudioInfo.pauAdtsHeader);
	if (NULL == prCfaRm->rAudioInfo.pauAdtsHeader) {
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM] Alloc prCfaRm->rAudioInfo.pauAdtsHeader memory failed\r\n"));
		vCfaRmInternalMemFree(prCfaRm);
		DMX_FreeMemory(prCfaRm);
		prCfaRm = NULL;
		MM_RETURN(RET_DMX_NO_MEM);
	}

	prCfaRm->ptrPfrMemAddress = DMX_INVALID_UINTPTR_T;
	prCfaRm->pu1HdrBuf = NULL;
	prCfaRm->u8Ca = DMX_INVALID_UINT64;
	prCfaRm->u4CurPrsFlag = 0;
	prCfaRm->u1FrameIdxInPacket = 0;
	prCfaRm->fgSetJumpRange = FALSE;

	CfaRmInitPara(prCfaRm);/*set initial information*/

	*ppvCfaPrivData = (void *) prCfaRm;/*Assign cfa function pointer*/

	MM_RETURN(RET_DMX_OK);
}

/*-----------------------------------------------------------------------------
 * Name: CfaRmUninit
 *
 * Description:
 *		Uninit CFA RM
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaRmUninit(void *pvSptHdl, void *pvCfaPrivData)
{
	CfaRmInst_T *prCfaRm = (CfaRmInst_T *) pvCfaPrivData;
	void *pvPointer = NULL;

	if (NULL == prCfaRm)
	{
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaRm);
	MMATE_CHECK_STRUCT(prCfaRm->rPacketInfo);
#endif

	pvPointer = (void *)prCfaRm;
	prCfaRm->pu1HdrBuf = NULL;

	vCfaRmInternalMemFree(prCfaRm);

	if (NULL != pvPointer) {
		DMX_FreeMemory(pvPointer);
		pvPointer = NULL;
	}

	DmxLogT(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM] Exit CfaRmUninit successfully!\r\n"));
	MM_RETURN(RET_DMX_OK);
}

/*-----------------------------------------------------------------------------
 * Name: CfaRmInitPara
 *
 * Description:
 *		Init CFA RM internal parameters
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: None
 *
 *-----------------------------------------------------------------------------*/
void CfaRmInitPara(CfaRmInst_T *prCfaRm)
{
	if (NULL == prCfaRm)
	{
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
		TEXT("[CFA_RM] CfaRmInitPara:: prCfaRm is NULL.\r\n"));
		return;
	}

	prCfaRm->eCurAnaSt = CFA_RM_ANA_ST_IDLE;
	prCfaRm->u8PrsPts = 0;
	prCfaRm->rCurPosInfo.u8AudCurOfst = 0;
	prCfaRm->rCurPosInfo.u8IFrmCurOfst = 0;
	prCfaRm->rCurPosInfo.u8PrsCurOfst = 0;
	prCfaRm->rCurPosInfo.u8VidCurOfst = 0;
	prCfaRm->eCurPrsBitStrmType = CFA_RM_PRS_BIT_STRM_TYPE_NONE;
	prCfaRm->u1CurStrmId = DMX_INVALID_UINT8;
	prCfaRm->u8PreVPts = 0;
	prCfaRm->u4Tx2VFifoLen = 0;
	prCfaRm->u4CurFrmTotalLen = 0;
	prCfaRm->u4StrmErrCnt = 0;
}

/*-----------------------------------------------------------------------------
 * Name: CfaRmSetRange
 *
 * Description:
 *		RM CFA sets demuxing range
 *		splitter will ensure that pfvSetRange is only called in "off" state.
 *		If used with MPC, the range of MPC_SCMD_SPR will be passed here
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: s32
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaRmSetRange(void *pvSptHdl, void *pvRange, void *pvPrivData, bool fgIsUserMem)
{
	CfaRmInst_T *prCfaRm = NULL;

	prCfaRm = (CfaRmInst_T *) pvPrivData;

	if ((NULL == prCfaRm) || (NULL == pvRange)) {
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM] CfaRmSetRange:: The parameter is Invalid!\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

#ifdef MM_ATE_CHECK
	dmx_memcpy(&(prCfaRm->rRange.fgEnableVid), &(((CfaRmRange_T *)pvRange)->fgEnableVid),
		sizeof(CfaRmRange_T) - 2 * sizeof(u32));
#else
	dmx_memcpy(&(prCfaRm->rRange), pvRange, sizeof(CfaRmRange_T));
#endif
#ifdef MM_ATE_CHECK
	MMATE_INIT_STRUCT(prCfaRm->rRange);
	MMATE_CHECK_POINTER(prCfaRm);
	MMATE_CHECK_STRUCT(prCfaRm->rPacketInfo);
#endif

	prCfaRm->rCurPosInfo.u8VidCurOfst = prCfaRm->rRange.u8VidSa;
	prCfaRm->rCurPosInfo.u8AudCurOfst = prCfaRm->rRange.u8AudSa;

	DmxLogT(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
		TEXT("[CFA_RM] CfaRmSetRange:: The target time is %lldms\r\n"),
		prCfaRm->rRange.u8targetTime);

	if ((prCfaRm->rRange.u8AudSa == 0) &&
		 (prCfaRm->rRange.u8VidSa == 0)) {
		prCfaRm->u8Ca = prCfaRm->rFileInfo.u4HeaderSize;
	} else {
		if ((CFA_RM_PRS_BIT_STRM_TYPE_VID == (prCfaRm->u4CurPrsFlag)) &&
				(prCfaRm->rRange.fgEnableVid))
			prCfaRm->u8Ca = prCfaRm->rRange.u8VidSa;
		else if ((CFA_RM_PRS_BIT_STRM_TYPE_AUD == (prCfaRm->u4CurPrsFlag)) &&
				(prCfaRm->rRange.fgEnableAud))
			prCfaRm->u8Ca = prCfaRm->rRange.u8AudSa;
		else
			prCfaRm->u8Ca = MIN(prCfaRm->rRange.u8AudSa, prCfaRm->rRange.u8VidSa);
	}
	prCfaRm->u8Endoffst = prCfaRm->rRange.u8VidEa;
	prCfaRm->fgEnableAud = prCfaRm->rRange.fgEnableAud;
	prCfaRm->fgEnableVid = prCfaRm->rRange.fgEnableVid;

	prCfaRm->fgFirstTxAud = TRUE;
	prCfaRm->fgFirstTxVid = TRUE;
	prCfaRm->u4PacketSum = 0;
	prCfaRm->u1FrameIdxInPacket = 0;
	/*mtk40504 add, fix seek bug*/
	if (!prCfaRm->fgSetJumpRange) {
		prCfaRm->u4BackwardRefTr  = 0;
		prCfaRm->u4ForwardRefTr   = 0;
		prCfaRm->u8BackwardRefPts = 0;
		prCfaRm->u8ForwardRefPts  = 0;
	}

	DmxLogD(DMX_MOD_CFA_RM,CFA_RM_LOG_SET,TEXT("[CFA_RM] CfaRmSetRange::prCfaRm->u8Ca = 0X%x,prCfaRm->u8Endoffst =0x%x\r\n"),
		prCfaRm->u8Ca, prCfaRm->u8Endoffst);
	MM_RETURN(RET_DMX_OK);
}

/*-----------------------------------------------------------------------------
 * Name: CfaRmEnableStrm
 *
 * Description:
 *		RM CFA sets stream to parse, may be combinations of V/A/S.
 *		splitter will ensure that pfvSetStrm() is only called in "off" or "paused" state.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: s32
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaRmEnableStrm(void *pvSptHdl, u32 u4StrmToPrs, CfaStreamOp eOp, void *pvPrivData)
{
	CfaRmInst_T *prCfaRm = NULL;

	if (NULL == pvPrivData)
	{
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	prCfaRm = (CfaRmInst_T *) pvPrivData;
#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaRm);
	MMATE_CHECK_STRUCT(prCfaRm->rRange);
	MMATE_CHECK_STRUCT(prCfaRm->rPacketInfo);
#endif

	if (CFA_STREAM_ON == eOp) {
		/*enable*/
		if (CFA_STRM_V & u4StrmToPrs)
			prCfaRm->u4CurPrsFlag |= CFA_RM_PRS_BIT_STRM_TYPE_VID;

		if (CFA_STRM_A & u4StrmToPrs) {
			prCfaRm->u4CurPrsFlag |= CFA_RM_PRS_BIT_STRM_TYPE_AUD;
			prCfaRm->fgFirstTxAud = TRUE;
		}
	} else {/*disable*/
		if (CFA_STRM_V & u4StrmToPrs)
			prCfaRm->u4CurPrsFlag &= ~((u32)CFA_RM_PRS_BIT_STRM_TYPE_VID);

		if (CFA_STRM_A & u4StrmToPrs)
			prCfaRm->u4CurPrsFlag &= ~((u32)CFA_RM_PRS_BIT_STRM_TYPE_AUD);
	}
	DmxLogD(DMX_MOD_CFA_RM,CFA_RM_LOG_SET,TEXT("[CFA_RM] Exit CfaRmEnableStrm!\r\n"));
	MM_RETURN(RET_DMX_OK);
}

/*-----------------------------------------------------------------------------
 * Name: CfaRmSetStrmInf
 *
 * Description:
 *		Set Stream information
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaRmSetStrmInf(void *pvSptHdl, u32 u4Strm, u32 u4Info, void *pvPrivData)
{
	CfaRmInst_T *prCfaRm = NULL;

	if (NULL == pvPrivData)
	{
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	prCfaRm = (CfaRmInst_T *)pvPrivData;
#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaRm);
	MMATE_CHECK_STRUCT(prCfaRm->rRange);
	MMATE_CHECK_STRUCT(prCfaRm->rPacketInfo);
#endif

	if (CFA_STRM_V == u4Strm)
	{
		prCfaRm->rVideoInfo.rCfgInfo.u1StrmNum = (u8)((u32)0x000000FF & u4Info);
	}
	else if (CFA_STRM_A == u4Strm)
	{
		prCfaRm->rAudioInfo.rCfgInfo.u1StrmNum = (u8)((u32)0x000000FF & u4Info);
	}
	else
	{
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxLogD(DMX_MOD_CFA_RM,CFA_RM_LOG_SET,TEXT("[CFA_RM] Exit CfaRmSetStrmInf!\r\n"));

	MM_RETURN(RET_DMX_OK);
}

/*-----------------------------------------------------------------------------
 * Name: CfaRmTurnOn
 *
 * Description:
 *		RM CFA turns on file demuxing
 *		A transfer should be issued in this function.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: s32
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaRmTurnOn(void *pvSptHdl, void *pvPrivData)
{
	u64 u8Sa = 0;
	CfaRmInst_T *prCfaRm = NULL;

	if (NULL == pvPrivData)
	{
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	prCfaRm = (CfaRmInst_T *)pvPrivData;
#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaRm);
	MMATE_CHECK_STRUCT(prCfaRm->rRange);
	MMATE_CHECK_STRUCT(prCfaRm->rPacketInfo);
#endif

	CfaRmInitPara(prCfaRm);

	if (prCfaRm->u8Ca == DMX_INVALID_UINT64) {
		u8Sa = CfaRmGetTxSa(prCfaRm);
		if (DMX_INVALID_UINT64 == u8Sa)
		{
			MM_RETURN(RET_DMX_OVER_LIMIT);
		}
	}

	if (prCfaRm->fgSetJumpRange)
		prCfaRm->fgSetJumpRange = FALSE;


	/*need rewrite*/
	CfaRmSearchNextSc(pvSptHdl, prCfaRm, CFA_RM_ANA_ST_MEDIA_PACKET_HEADER, (u64)0,
			(u64)RM_READ_FOR_PACKET_HEADER_SIZE, (u32)0); /*RM_READ_FOR_CODEC_SIZE*/

	DmxLogT(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM] Turn On at 0x%llx[%lld]!\r\n"), prCfaRm->u8Ca, prCfaRm->u8Ca);

	MM_RETURN(RET_DMX_OK);
}

/* RM CFA callback for transfer done
@return None
 @note This function will be called after a transfer is complete.*/
static MRESULT CfaRmTxDone(void *pvSptHdl, u64 u8TxLen, void *pvPrivData, bool fgRsp)
{
	CfaRmInst_T *prCfaRm = NULL;
	MRESULT mrResult = RET_DMX_OK;

	if (NULL == pvPrivData)
	{
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	prCfaRm = (CfaRmInst_T *) pvPrivData;
#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaRm);
	MMATE_CHECK_STRUCT(prCfaRm->rRange);
	MMATE_CHECK_STRUCT(prCfaRm->rVideoInfo);
#endif

	if (fgRsp) {
		mrResult = Spt4CfaPbb2SyncBuf(pvSptHdl, prCfaRm->u8Ca, u8TxLen, (u8 *)&(prCfaRm->ptrPfrMemAddress));
		MM_RETURN(mrResult);
	}

	/*need rewrite*/
	CfaRmTxDoneStCtrl(pvSptHdl, u8TxLen, prCfaRm);

	MM_RETURN(RET_DMX_OK);
}

/*-----------------------------------------------------------------------------
 * Name: CfaRmGetCurPos
 *
 * Description:
 *		RM CFA callback for when FMPC needs to know CFA's current position.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaRmGetCurPos(void *pvSptHdl, void *pvCurPos, void *pvPrivData)
{
	CfaRmInst_T *prCfaRm = NULL;
	u64 *pvu8 = pvCurPos;

	if (NULL == pvPrivData)
	{
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prCfaRm = (CfaRmInst_T *)pvPrivData;
	*pvu8 = prCfaRm->u8Ca;

	MM_RETURN(RET_DMX_OK);
}

/*-----------------------------------------------------------------------------
 * Name: CfaRmFillPicInfo
 *
 * Description:
 *		RM CFA callback for each picture is demuxed
 *		original related function: vRmM4vPIsr
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: TRUE - this picture should be retained in video FIFO.
 *			FALSE - this picture should be removed from video FIFO.
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaRmFillPicInfo(void *pvSptHdl, Spt2CfaPicInfo *ptPicInfo, void *pvPrivData)
{
	CfaRmInst_T *prCfaRm = NULL;

	prCfaRm = (CfaRmInst_T *)pvPrivData;

	if ((NULL == prCfaRm) || (NULL == ptPicInfo)) {
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM] CfaRmFillPicInfo:: The parameter is Invalid!\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaRm);
	MMATE_CHECK_STRUCT(prCfaRm->rRange);
	MMATE_CHECK_STRUCT(prCfaRm->rPacketInfo);
#endif

	ptPicInfo->u8ThisPts = prCfaRm->u8PrsPts;
	prCfaRm->u8PrsPts = DMX_INVALID_UINT64;

	MM_RETURN(RET_DMX_OK);
}

/*-----------------------------------------------------------------------------
 * Name: CfaRmSetVidType
 *
 * Description:
 *		CFA RM sets video type for transfering video data to Video FIFO with the codec info
 *		by playback module setting.
 * Inputs:
 *
 * Outputs:
 *
 * Returns: s32
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaRmSetVidType(CfaRmInst_T *prCfaRm)
{
	if (NULL == prCfaRm)
	{
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	switch (prCfaRm->rVideoInfo.rCfgInfo.eCodecVer) {
	case VCODEC_VERSION_RV_10:
		prCfaRm->eVidCodecType = CFA_VID_UNKNOWN;
		break;

	case VCODEC_VERSION_RV_13:
		prCfaRm->eVidCodecType = CFA_VID_UNKNOWN;
		break;

	case VCODEC_VERSION_RV_20:
		prCfaRm->eVidCodecType = CFA_VID_UNKNOWN;/*error*/
		break;

	case VCODEC_VERSION_RV_30:
		prCfaRm->eVidCodecType = CFA_VID_RV30;/*RV30*/
		break;

	case VCODEC_VERSION_RV_40:
		prCfaRm->eVidCodecType = CFA_VID_RV40;/*RV40*/
		break;

	default:
		prCfaRm->eVidCodecType = CFA_VID_UNKNOWN;
			break;
	}

	MM_RETURN(RET_DMX_OK);
}

static CfaApiAudType CfaRmGetAudType(AVCODECID_T eInAudType)
{
	switch (eInAudType) {
	case AVCODEC_ID_RA_LPCJ:
		return CFA_AUD_DRV_FMT_UNKNOWN;

	case AVCODEC_ID_RA_28_8:
		return CFA_AUD_DRV_FMT_UNKNOWN;

	case AVCODEC_ID_RA_DNET:
		return CFA_AUD_DRV_FMT_UNKNOWN;

	case AVCODEC_ID_RA_SIPR:
		return CFA_AUD_DRV_FMT_UNKNOWN;

	case AVCODEC_ID_RA_COOK:
		return CFA_AUD_DRV_FMT_COOK;

	case AVCODEC_ID_RA_ATRC:
		return CFA_AUD_DRV_FMT_UNKNOWN;

	case AVCODEC_ID_RA_RALF:
		return CFA_AUD_DRV_FMT_UNKNOWN;

	case AVCODEC_ID_AAC:
		return CFA_AUD_DRV_FMT_AAC;

	default:
		return CFA_AUD_DRV_FMT_UNKNOWN;
	}
}

static void GenericInterleavePatte(void *pvSptHdl, CfaRmInst_T *prCfaRm)
{
	u16 i = 0;	/*Frame index within superblock*/
	u16 f = 0;	/*Frame index within block*/
	u16 b = 0;	/*Block index within superblock*/
	bool even = TRUE;
	u16 frames_per_block = 0;
	u32 frames_per_superblock = 0;

	CfaRmCfgAudInfo_T *prCfgAudioInfo = &prCfaRm->rAudioInfo.rCfgInfo;
	if (0 == prCfgAudioInfo->u2FrameSize)
	{
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT, TEXT("[CFA_RM] line %d,u2FrameSize is zero!\r\n"),DMX_LINE_NO);
		return;
	}
	frames_per_superblock = (prCfgAudioInfo->u2InterleaveFactor * prCfgAudioInfo->u2BlockSize) / prCfgAudioInfo->u2FrameSize;
	frames_per_block = prCfgAudioInfo->u2BlockSize / prCfgAudioInfo->u2FrameSize;
	if (frames_per_superblock > (u32)RM_MAX_FRAME_NUM_IN_SUP_BLOCK) {
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
			TEXT("[CFA_RM] GenericInterleavePatte::")
			TEXT("The frames_per_superblock[%d] is larger than ")
			TEXT("RM_MAX_FRAME_NUM_IN_SUP_BLOCK!\r\n"),
			frames_per_superblock);
		frames_per_superblock = RM_MAX_FRAME_NUM_IN_SUP_BLOCK;
	}

	if (prCfgAudioInfo->u2InterleaveFactor == 1) {
		for (i = 0; i < frames_per_superblock; i++)
			prCfaRm->u2RmPattern[i] = i;

	} else {
		while (i < frames_per_superblock) {
			prCfaRm->u2RmPattern[i] = b * frames_per_block + f;
			i++;
			b += 2;
			if (b >= prCfgAudioInfo->u2InterleaveFactor) {
				if (even) {
					even = FALSE;
					b = 1;
				} else {
					even = TRUE;
					b = 0;
					f++;
				}
			}
		}
	}
}
/*-----------------------------------------------------------------------------
 * Name: CfaRmConfigure
 *
 * Description:
 *		splitter will ensure that it is only called in "off" or "paused" state.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: s32
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaRmConfigure(void *pvSptHdl, void *pvParam, void *pvPrivData, bool fgIsUserMem)
{
	CfaRmInst_T *prCfaRmInst = NULL;
	CfaRmVidInfo_T *prCfaRmVidInfo = NULL;
	CfaRmAudInfo_T *prCfaRmAudInfo = NULL;
	CfaRmCfgInfo_T rCfaRmCfgInfo;

	prCfaRmInst = (CfaRmInst_T *)pvPrivData;

	mm_memset(&rCfaRmCfgInfo, 0x00, sizeof(CfaRmCfgInfo_T));

	if ((NULL == prCfaRmInst) || (NULL == pvParam)) {
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM] CfaRmConfigure:: The parameter is Invalid!\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	if (fgIsUserMem) {
		if (0 != mm_copy_from_user(&rCfaRmCfgInfo,
			pvParam, sizeof(CfaRmCfgInfo_T))) {
			DmxLogE(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
				TEXT("[CFA_RM] %s line %d failed in mm_copy_from_user\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
		}
	} else {
		mm_memcpy(&rCfaRmCfgInfo, pvParam, sizeof(CfaRmCfgInfo_T));
	}
	
	
#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaRmInst);
#endif
	/*config video info*/
	prCfaRmVidInfo = &prCfaRmInst->rVideoInfo;


	dmx_memcpy(&(prCfaRmVidInfo->rCfgInfo), &(rCfaRmCfgInfo.rCfaRmCfgVidInfo),
				sizeof(CfaRmCfgVidInfo_T));

	/*set video type for transfering video data to video FIFO.*/
	CfaRmSetVidType(prCfaRmInst);

	DmxLogT(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM] CfaRmConfigure, eVidCodecType:%d.\r\n"),prCfaRmInst->eVidCodecType);

	if (((prCfaRmVidInfo->rCfgInfo.u1FrameRate) > 0) &&
		((prCfaRmVidInfo->rCfgInfo.u1FrameRate) != 0xFF)) {
		prCfaRmInst->u2Duration = 1000 / prCfaRmVidInfo->rCfgInfo.u1FrameRate;
	}

	/*config audio info*/
	prCfaRmAudInfo = &prCfaRmInst->rAudioInfo;

	dmx_memcpy(&(prCfaRmAudInfo->rCfgInfo), &(rCfaRmCfgInfo.rCfaRmCfgAudInfo),
				sizeof(CfaRmCfgAudInfo_T));
	

	prCfaRmAudInfo->eAudType = CfaRmGetAudType(prCfaRmAudInfo->rCfgInfo.eCodecID);

	if (NULL != prCfaRmInst->rAacCfgInfo.puHeader) {
		DMX_FreeHwMemory(prCfaRmInst->rAacCfgInfo.puHeader);
		prCfaRmInst->rAacCfgInfo.puHeader = NULL;
	}
	prCfaRmInst->rAacCfgInfo.uHeaderLen = prCfaRmInst->rAudioInfo.rCfgInfo.rAacInfo.uHeaderLen;

	if (prCfaRmInst->rAacCfgInfo.uHeaderLen > 0) {
		DMX_NewHwMemory(prCfaRmInst->rAacCfgInfo.uHeaderLen * sizeof(u8),
						prCfaRmInst->rAacCfgInfo.puHeader);
		if (NULL == prCfaRmInst->rAacCfgInfo.puHeader) {
			DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
				TEXT("[CFA_RM] Alloc prCfaRmInst->rAacCfgInfo.puHeader memory failed\r\n"));
			vCfaRmInternalMemFree(prCfaRmInst);
			if (!fgIsUserMem)
				DMX_FreeMemory(pvParam);
			MM_RETURN(RET_DMX_NO_MEM);
		}

		dmx_memset(prCfaRmInst->rAacCfgInfo.puHeader,
			0,
			prCfaRmInst->rAacCfgInfo.uHeaderLen * sizeof(u8));

		if (!access_ok(VERIFY_READ,
					   (void __user *)(prCfaRmInst->rAudioInfo.rCfgInfo.rAacInfo.puHeader),
						prCfaRmInst->rAacCfgInfo.uHeaderLen * sizeof(u8))) {
			DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM] line %d access_ok return false\r\n"), DMX_LINE_NO);
			if (!fgIsUserMem)
				DMX_FreeMemory(pvParam);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		if (!fgIsUserMem) {
			dmx_memcpy(prCfaRmInst->rAacCfgInfo.puHeader,
				prCfaRmInst->rAudioInfo.rCfgInfo.rAacInfo.puHeader,
				prCfaRmInst->rAacCfgInfo.uHeaderLen * sizeof(u8));
		} else {
			mm_copy_from_user(prCfaRmInst->rAacCfgInfo.puHeader,
				prCfaRmInst->rAudioInfo.rCfgInfo.rAacInfo.puHeader,
				prCfaRmInst->rAacCfgInfo.uHeaderLen * sizeof(u8));
		}

	}

	/*config others info*/
	prCfaRmInst->rFileInfo.u8FileSize	  = rCfaRmCfgInfo.rCfaRmCfgFileInfo.u8FileSize;
	prCfaRmInst->rFileInfo.u4HeaderSize   = rCfaRmCfgInfo.rCfaRmCfgFileInfo.u4HeaderSize;
	prCfaRmInst->rFileInfo.fgCfaRespliter = rCfaRmCfgInfo.rCfaRmCfgFileInfo.fgCfgRespliter;
	prCfaRmInst->rFileInfo.u4StreamNum	  = rCfaRmCfgInfo.rCfaRmCfgFileInfo.u4StreamNum;
	prCfaRmInst->fgHasVideo 			  = rCfaRmCfgInfo.rCfaRmCfgFileInfo.fgHasVideo;

	if (!prCfaRmInst->fgHasVideo) {
		DmxLogT(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
			TEXT("[CFA_RM] Configure, the file is ra\r\n"));
	}
	if (NULL != prCfaRmInst->rFileInfo.prStreamInfo) {
		DMX_FreeMemory(prCfaRmInst->rFileInfo.prStreamInfo);
		prCfaRmInst->rFileInfo.prStreamInfo = NULL;
	}

	if (prCfaRmInst->rFileInfo.u4StreamNum > 0) {
		DMX_NewMemory(sizeof(RM_STREAM_TYPE_INFO_T) * prCfaRmInst->rFileInfo.u4StreamNum,
			prCfaRmInst->rFileInfo.prStreamInfo);

		if (NULL == prCfaRmInst->rFileInfo.prStreamInfo) {
			DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
				TEXT("[CFA_RM] Alloc prCfaRmInst->rFileInfo.prStreamInfo memory failed\r\n"));
			vCfaRmInternalMemFree(prCfaRmInst);
			if (!fgIsUserMem)
				DMX_FreeMemory(pvParam);
			MM_RETURN(RET_DMX_NO_MEM);
		}

		dmx_memset(prCfaRmInst->rFileInfo.prStreamInfo,
			0,
			prCfaRmInst->rFileInfo.u4StreamNum * sizeof(RM_STREAM_TYPE_INFO_T));
		if (!fgIsUserMem) {
			dmx_memcpy(prCfaRmInst->rFileInfo.prStreamInfo,
				rCfaRmCfgInfo.rCfaRmCfgFileInfo.prStreamInfo,
				prCfaRmInst->rFileInfo.u4StreamNum * sizeof(RM_STREAM_TYPE_INFO_T));
		} else {
			if (0 != mm_copy_from_user(prCfaRmInst->rFileInfo.prStreamInfo,
				rCfaRmCfgInfo.rCfaRmCfgFileInfo.prStreamInfo,
				prCfaRmInst->rFileInfo.u4StreamNum * sizeof(RM_STREAM_TYPE_INFO_T))) {
				DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM] copy from user fail\r\n"));
				MM_RETURN(RET_DMX_EXT_EXCEPTION);
			}
		}

	}

	if (CFA_AUD_DRV_FMT_COOK == prCfaRmAudInfo->eAudType) {
		prCfaRmInst->u4SuperBlockSize = prCfaRmInst->rAudioInfo.rCfgInfo.u2InterleaveFactor *
			prCfaRmInst->rAudioInfo.rCfgInfo.u2BlockSize;

		GenericInterleavePatte(pvSptHdl, prCfaRmInst);
	}

	if (!fgIsUserMem)
		DMX_FreeMemory(pvParam);

#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaRmInst);
	MMATE_CHECK_STRUCT(prCfaRmInst->rRange);
	MMATE_CHECK_STRUCT(prCfaRmInst->rPacketInfo);
#endif

	MM_RETURN(RET_DMX_OK);
}

/*-----------------------------------------------------------------------------
 * Name: CfaRmFillAUInfo
 *
 * Description:
 *		RM CFA callback for each AU is demuxed
 *
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaRmFillAUInfo(void *pvSptHdl, void *pvAUInfo, void *pvAUExtInfo, void *pvPrivData)
{
	u64 u8RealPTS = 0;
	u64 u8Offset = 0;
	CfaRmInst_T *prCfaRm = NULL;
	u8 u1TSliceNum = 0;

	prCfaRm = (CfaRmInst_T *)pvPrivData;

	if ((NULL == pvAUInfo) || (NULL == prCfaRm)) {
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM] CfaRmFillAUInfo:: The parameter is Invalid!\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaRm);
#endif

	switch (prCfaRm->eCurTxStrmType) {
	case CFA_RM_TX_STRM_TYPE_VID:
		/* error handle: for some files, payload PTS may be earlier than preroll time*/
		u8RealPTS = (u64)prCfaRm->rPacketInfo.u4TimeStamp;
		u8RealPTS *= CFA_RM_SYS_CLK;
		u8Offset = prCfaRm->rPacketInfo.u8Offset;
		/*prCfaRm->u8Ca;//need modify*/
		((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.eDiscType = DT_DATADISC;

		u1TSliceNum = prCfaRm->rSliceInf.u1TotalSliceNum;

		((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4RMSliceNum = prCfaRm->rSliceInf.u1TotalSliceNum;
		{
			int i = 0;

			for (; i < prCfaRm->rSliceInf.u1TotalSliceNum; ++i) {
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.auRM4SliceSize[i] =
					prCfaRm->rSliceInf.rSliceInf[i].u2SliceElemSize;
			}
		}
		dmx_memset(&prCfaRm->rSliceInf, 0, sizeof(prCfaRm->rSliceInf));

	#if CFA_RM_CHECK_PIC_TYPE
		if (CFA_PTM_RM_TRUEBPIC == prCfaRm->eCurPicType)
			((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = prCfaRm->u8PrsPts;
		else
			((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = u8RealPTS;

		((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4Duration = 0;
	#else
		((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = u8RealPTS;
	#endif

		((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4PrevDuration = (u32)(INVALID_DURATION);
		/*DMXLOG_DEBUG(
		TEXT("[CFA_RM] Fill AU prCfaRm->eCurPicType = %d, Time= %lld ms\r\n"),
		prCfaRm->eCurPicType,(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts / CFA_RM_SYS_CLK));*/
		break;

	case CFA_RM_TX_STRM_TYPE_AUD:
		/* error handle: for some files, payload PTS may be earlier than preroll time*/
		u8RealPTS = prCfaRm->rPacketInfo.u4TimeStamp;
		u8RealPTS *= CFA_RM_SYS_CLK;

		if (AVCODEC_ID_RA_COOK == prCfaRm->rAudioInfo.rCfgInfo.eCodecID) {
			u8RealPTS = prCfaRm->u8AudPts;
			prCfaRm->u4TxSizeInSuperBlock = 0;
		}

		prCfaRm->u4PacketSum = 0;

		((AU_AUDIO *)pvAUInfo)->rAUInfo.rInfo.u8Pts = u8RealPTS;
		/*DMXLOG_DEBUG(
		TEXT("[CFA_RM] Fill AU AUDIO: Time= %d ms\r\n"), (u32)(u8RealPTS / CFA_RM_SYS_CLK));*/
		break;

	case CFA_RM_TX_STRM_TYPE_NONE:
		DmxLogE(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,TEXT("[CFA_RM] Current tx no vid & aud data to FIFO!\r\n"));
		break;

	default:
		MM_RETURN(RET_DMX_UNSUPPORT);
	}

	DmxLogD(DMX_MOD_CFA_RM,CFA_RM_LOG_FILLAU,TEXT("[CFA_RM] fill au info,u8Pts=%lld!\r\n"),
		((AU_AUDIO *)pvAUInfo)->rAUInfo.rInfo.u8Pts);

#ifdef MM_ATE_CHECK
	MMATE_CHECK_POINTER(prCfaRm);
	MMATE_CHECK_STRUCT(prCfaRm->rRange);
	MMATE_CHECK_STRUCT(prCfaRm->rPacketInfo);
#endif
	MM_RETURN(RET_DMX_OK);
}

/*-----------------------------------------------------------------------------
 * Name: CfaRmTxAudHDRInfo
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
static MRESULT CfaRmTxAudHdrInfo(void *pvSptHdl, u32 u4TxUID, void *pvPrivData)
{
	MRESULT mrResult = RET_DMX_OK;
	CfaRmInst_T *prCfaRm = NULL;

	if (NULL == pvPrivData)
	{
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prCfaRm = (CfaRmInst_T *)pvPrivData;

	DmxLogD(DMX_MOD_CFA_RM,CFA_RM_LOG_TXDONE,TEXT("[CFA_RM] TxAudHdrInfo!\r\n"));

	if ((DMX_INVALID_UINT32 == u4TxUID) &&
		(AVCODEC_ID_AAC == prCfaRm->rAudioInfo.rCfgInfo.eCodecID))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	if (prCfaRm->rAudioInfo.rCfgInfo.u1StrmNum != u4TxUID)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	if (AVCODEC_ID_AAC != prCfaRm->rAudioInfo.rCfgInfo.eCodecID) {
		MM_RETURN(RET_DMX_UNSUPPORT);
	} else {
		if (!prCfaRm->fgFirstTxAud)
			MM_RETURN(RET_DMX_OK);

		mrResult = Spt4CfaBuf2AFifo(pvSptHdl, prCfaRm->rAacCfgInfo.puHeader,
			prCfaRm->rAacCfgInfo.uHeaderLen, u4TxUID,
			prCfaRm->rAudioInfo.eAudType);
		MM_RETURN(mrResult);
	}
}

static MRESULT CfaRmSetJumpRange(void *pvSptHdl, void *pvJmpRange, void *pvPrivData)
{
	CfaRmInst_T *prCfaRm = NULL;
	CfaRmRange_T *prCfaRmRange = NULL;

	prCfaRmRange = (CfaRmRange_T *)pvJmpRange;

	if ((NULL == pvPrivData) || (NULL == pvJmpRange))
	{
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	prCfaRm = (CfaRmInst_T *)pvPrivData;

	prCfaRm->fgSetJumpRange = TRUE;

	DmxLogT(DMX_MOD_CFA_RM,CFA_RM_LOG_DEFAULT,
		TEXT("[CFA_RM] Entry CfaRmSetJumpRange,u8VidStartOfst is 0x%llx,")
		TEXT("u8AudStartOfst is 0x%llx!\r\n"),
		prCfaRmRange->u8VidSa, prCfaRmRange->u8AudSa);

	CfaRmSetRange(pvSptHdl, (void *)prCfaRmRange, pvPrivData, TRUE);

	prCfaRm->u4RWUnitAULen = prCfaRmRange->u4JumpUnitSz;
	prCfaRm->fgFinishRWAU = FALSE;

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaRmGetParamSize(void *pvSptHdl, u32 u4ParamID,
	void  *pvPrivData, void  *pvCfaParam, u32 u4CfaParamSz)
{
	MRESULT mrResult = RET_DMX_OK;

	DmxLogD(DMX_MOD_CFA_RM,CFA_RM_LOG_STATE,TEXT("[CFA_RM] Entry CfaRmGetParamSize!\r\n"));

	switch (u4ParamID) {
	case CFA_PARAM_ID_JUMP_INFO_SIZE: {
		if ((NULL == pvCfaParam) || (u4CfaParamSz < sizeof(u32))) {
			mrResult = RET_DMX_PARAM_WRONG;
		} else {
			u32 *pu4Tmp = (u32 *)pvCfaParam;
			*pu4Tmp = sizeof(CfaRmRange_T);
			DmxLogD(DMX_MOD_CFA_RM,CFA_RM_LOG_STATE,TEXT("[CFA_RM] CfaRmRange_T is %d(Bytes)!\r\n"), *pu4Tmp);
		}
		break;
	}

	default:
		mrResult = RET_DMX_PARAM_WRONG;
			break;
	}

	MM_RETURN(mrResult);
}

static MRESULT CfaRmProcCliCmd(void *pvSptHdl, E_DMX_CFA_CLI_TYPE_T eCliType, /*< [IN] Cfa Cli Command*/
				u32 arg1,
				u32 arg2, u32 arg3, const char *szParam, VOID *pvPrivData)
{
	MRESULT mrRet = RET_DMX_OK;
	CfaRmInst_T *prCfaRm = NULL;

	prCfaRm = (CfaRmInst_T *) pvPrivData;

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

			DmxLogT(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
				TEXT("CfaRmProcCliCmd -- fgEnable: %d, Loglvl: %d, ModLogLvl: 0x%08x \r\n"),
				arg1, arg2, arg3);

			DmxLogEnable(fgEnable, arg2, DMX_MOD_CFA_RM, arg3);
		}
		break;
	case DMX_CFA_CLI_CMD_DUMP_INFO:
		{
			DmxLogT(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
				TEXT("Cfa RM Instance(handle is 0x%x)")
				TEXT(" Info list as follow: \r\n"),
				prCfaRm);
			DmxLogT(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
				TEXT("Current Analyse State is %d, Previous Analyse State is %d \r\n"),
				prCfaRm->eCurAnaSt, prCfaRm->eLastAnaSt);
			DmxLogT(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
				TEXT("Current Analyse Position is 0x%08x%08x, ")
				TEXT("Previous Analyse Position is 0x%08x%08x\r\n"),
				(UINT32) ((prCfaRm->u8Ca) >> 32), (UINT32) (prCfaRm->u8Ca),
				(UINT32) ((prCfaRm->u8PreCa) >> 32), (UINT32) (prCfaRm->u8PreCa));
			DmxLogT(DMX_MOD_CFA_RM, CFA_RM_LOG_DEFAULT,
				TEXT("First Tx Video Flag is %d, First Tx Audio Flag is %d\r\n"),
				((prCfaRm->fgFirstTxVid) ? 1 : 0),
				((prCfaRm->fgFirstTxAud) ? 1 : 0));
		}
		break;
	default:
		break;
	}

	MM_RETURN(mrRet);
}

/* RM CFA interface */
CfaIntf _rRmCfaIntf = {
	&CfaRmInit,
	&CfaRmUninit,
	&CfaRmSetRange,
	&CfaRmEnableStrm,
	&CfaRmSetStrmInf,
	&CfaRmTurnOn,
	&CfaRmTxDone,
	&CfaRmGetCurPos,
	&CfaRmFillPicInfo,
	&CfaRmConfigure,
	NULL,
	NULL,
	NULL,
	NULL,
	&CfaRmFillAUInfo,
	&CfaRmTxAudHdrInfo,
	NULL,
	&CfaRmSetJumpRange,
	&CfaRmGetParamSize,
	&CfaRmProcCliCmd
	#ifdef CONFIG_COMPAT
	, &CfaRmProcCompat
	#endif
};

/*-----------------------------------------------------------------------------
 * Name: pvCfaRmGetInterface
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
void *CfaRmGetInterface(void)
{
	return ((void *) &_rRmCfaIntf);
}
