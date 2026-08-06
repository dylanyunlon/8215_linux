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

#include "windows.h"
#include <media/atc/dmx_define.h>
/* #include <media/atc/mm_debug.h> */
#include <media/atc/drv_esm_if.h>
#include <media/atc/ioctl_dmx.h>
#include <media/atc/ose_mem.h>

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_spt.h"
#include "cfa_macro.h"
#include "cfa_ogm.h"
#include "cfa_ogm_st_ctrl.h"


/*-----------------------------------------------------------------------------
			macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/

#ifdef CONFIG_COMPAT
#include <linux/compat.h>

typedef struct {
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKStart;
#endif	/* 
 */
	/* video */
	bool fgVidRangeEn;	/*only when true,the value below are valid */
	__s64 i8VidLastGranule;	/*granule position of last video page */
	__u64 u8VidStartOfst;	/*start offset of first page to be demux */
	__u8 uVidPacketStartNo;	/*loaction of first packet in the page */
	__u64 u8VidEndOfst;	/*end offset */

  /* audio */
	bool fgAudRangeEn;	/*only when true,the value below are valid */
	__s64 i8AudLastGranule;	/*granule position of last audio page */
	__u64 u8AudStartOfst;	/*start offset of first page to be demux */
	__u8 uAudPacketStartNo;	/*loaction of first packet in the page */
	__u64 u8AudEndOfst;	/*end offset */
	__u64 u8AudRangeOfst;	/*  audio range offset */
	__u64 u8SeekTime;

#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKEnd;
#endif	/* 
 */
} CfaOgmRange_T32;

typedef struct {
	CfaOgmRange_T32 rCfaOmgRane;
   /* for audio FR */
	__u32 u4FRAudAuLen;
} CfaOgmKeyRange_T32;

typedef struct {
	AVCODECID_T eCfaAudCodec;
	__u32 u4AudStreamNo;
	CfaOgmAudSample rAudSample;
	CfaOgmAacInfo rCfaOgmAacInfo;

	/* for vorbis seek */
	__u32 u4VorbisSerialNo;
	__u8 u1HeaderPacketNum;
	compat_caddr_t puHeaderData;
	__u32 u4HeaderDataSize;

	/* end */
	__u32 u4BitRate;
} CfaOgmAudInfo_T32;

typedef struct {
	/* Video info */
	CfaOgmVidInfo rCfaOgmVidInfo;

	/* Audio info */
	__u8 uAudioStreamNs;	/*stream total numbers of      Audio Streams */
	CfaOgmAudInfo_T32 arCfaOgmAudInfo[MAX_NS_OGM_AUD];

	__u32 u4DurationMs;
} CfaOgmConfigInfo_T32;
static long CfaOgmCompatConfigAudInfo(CfaOgmAudInfo __user *usr_ptr,
	CfaOgmAudInfo_T32 __user *usr_ptr32)
{
	if (copy_from_user(&(usr_ptr->eCfaAudCodec), &(usr_ptr32->eCfaAudCodec), sizeof(AVCODECID_T))) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(eCfaAudCodec).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->u4AudStreamNo), &(usr_ptr32->u4AudStreamNo), sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4AudStreamNo).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->u4VorbisSerialNo), &(usr_ptr32->u4VorbisSerialNo), sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4VorbisSerialNo).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->u1HeaderPacketNum), &(usr_ptr32->u1HeaderPacketNum), sizeof(__u8))) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u1HeaderPacketNum).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->u4HeaderDataSize), &(usr_ptr32->u4HeaderDataSize), sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4HeaderDataSize).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->u4BitRate), &(usr_ptr32->u4BitRate), sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4BitRate).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rAudSample.u4AudSampRate),
		&(usr_ptr32->rAudSample.u4AudSampRate), sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(rAudSample.u4AudSampRate).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rAudSample.rCfaOgmAudTime.u8SamplePerUnit), 
			&(usr_ptr32->rAudSample.rCfaOgmAudTime.u8SamplePerUnit), sizeof(__u64))) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(rAudSample.rCfaOgmAudTime.u8SamplePerUnit).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rAudSample.rCfaOgmAudTime.u8TimeUnit), 
			&(usr_ptr32->rAudSample.rCfaOgmAudTime.u8TimeUnit), sizeof(__u64))) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(rAudSample.rCfaOgmAudTime.u8TimeUnit).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rCfaOgmAacInfo.uChannles),
		&(usr_ptr32->rCfaOgmAacInfo.uChannles), sizeof(__u8))) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(rAudSample.rCfaOgmAacInfo.uChannles).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_from_user((usr_ptr->rCfaOgmAacInfo.auAacHeader),
			(usr_ptr32->rCfaOgmAacInfo.auAacHeader), 
			sizeof(__u8) * CFA_OGM_MAX_AAC_HEADER_LEN)) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(rAudSample.rCfaOgmAacInfo.auAacHeader).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rCfaOgmAacInfo.uAacHeaderLen),
		&(usr_ptr32->rCfaOgmAacInfo.uAacHeaderLen), sizeof(__u8))) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(rAudSample.rCfaOgmAudTime.uAacHeaderLen).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	return 0;
}

static long CfaOgmCompatJumpRange(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	
	CfaOgmKeyRange __user *usr_ptr = NULL;
	CfaOgmKeyRange_T32 __user *usr_ptr32 = (CfaOgmKeyRange_T32 __user *)prInfo->usr_ptr32;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaOgmKeyRange_T32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	usr_ptr = (CfaOgmKeyRange *)compat_alloc_user_space(sizeof(CfaOgmKeyRange));

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaOgmKeyRange));
	if (copy_in_user(&(usr_ptr->u4FRAudAuLen), &(usr_ptr32->u4FRAudAuLen), sizeof(__u32)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->rCfaOmgRane.fgVidRangeEn), &(usr_ptr32->rCfaOmgRane.fgVidRangeEn), 
			sizeof(bool)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->rCfaOmgRane.i8VidLastGranule), &(usr_ptr32->rCfaOmgRane.i8VidLastGranule), 
			sizeof(__s64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->rCfaOmgRane.u8VidStartOfst), &(usr_ptr32->rCfaOmgRane.u8VidStartOfst), 
			sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->rCfaOmgRane.uVidPacketStartNo), &(usr_ptr32->rCfaOmgRane.uVidPacketStartNo), 
			sizeof(__u8)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->rCfaOmgRane.u8VidEndOfst), &(usr_ptr32->rCfaOmgRane.u8VidEndOfst), 
			sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->rCfaOmgRane.fgAudRangeEn), &(usr_ptr32->rCfaOmgRane.fgAudRangeEn), 
			sizeof(bool)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->rCfaOmgRane.i8AudLastGranule), &(usr_ptr32->rCfaOmgRane.i8AudLastGranule), 
			sizeof(__s64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->rCfaOmgRane.u8AudStartOfst), &(usr_ptr32->rCfaOmgRane.u8AudStartOfst), 
			sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->rCfaOmgRane.uAudPacketStartNo), &(usr_ptr32->rCfaOmgRane.uAudPacketStartNo), 
			sizeof(__u8)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->rCfaOmgRane.u8AudEndOfst), &(usr_ptr32->rCfaOmgRane.u8AudEndOfst), 
			sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->rCfaOmgRane.u8AudRangeOfst), &(usr_ptr32->rCfaOmgRane.u8AudRangeOfst), 
			sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->rCfaOmgRane.u8SeekTime), &(usr_ptr32->rCfaOmgRane.u8SeekTime), sizeof(__u64)))
		return -EFAULT;

	*pfgIsUserMem = TRUE;
	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaOgmKeyRange);

	return 0;
}
static long CfaOgmCompatCalcSz(CfaOgmConfigInfo_T32 __user *usr_ptr32, __u32 *pu4OutSz)
{
	__u32 u4HeaderDataSize = 0;
	__u32 u4TotalSize = 0;
	__u32 i = 0;

	if (NULL == usr_ptr32 || NULL == pu4OutSz) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32 or pu4OutSz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	u4TotalSize += CFA_ALIGN_SZ(sizeof(CfaOgmConfigInfo), sizeof(uintptr_t));

	for (i = 0; i< MAX_NS_OGM_AUD; i++) {
		if (get_user(u4HeaderDataSize, &(usr_ptr32->arCfaOgmAudInfo[i].u4HeaderDataSize))) {
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("%s line %d fail in get_user(u4HeaderDataSize).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EFAULT;
		}
		
		u4TotalSize += CFA_ALIGN_SZ(u4HeaderDataSize, sizeof(uintptr_t));
	}

	*pu4OutSz = u4TotalSize;

	return 0;
}

static long CfaOgmCompatRange(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaOgmRange __user *usr_ptr = NULL;
	CfaOgmRange_T32 __user *usr_ptr32 = (CfaOgmRange_T32 __user *)prInfo->usr_ptr32;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaOgmRange_T32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz(0x%08x/0x%08x, 0x%08x).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prInfo->buf_sz, sizeof(CfaOgmRange_T32),
			sizeof(CfaOgmRange));
		return -EINVAL;
	}

	usr_ptr = (CfaOgmRange *)compat_alloc_user_space(sizeof(CfaOgmRange));

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaOgmRange));
	if (copy_in_user(&(usr_ptr->fgVidRangeEn), &(usr_ptr32->fgVidRangeEn), sizeof(bool)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->i8VidLastGranule), &(usr_ptr32->i8VidLastGranule), sizeof(__s64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8VidStartOfst), &(usr_ptr32->u8VidStartOfst), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->uVidPacketStartNo), &(usr_ptr32->uVidPacketStartNo), sizeof(__u8)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8VidEndOfst), &(usr_ptr32->u8VidEndOfst), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->fgAudRangeEn), &(usr_ptr32->fgAudRangeEn), sizeof(bool)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->i8AudLastGranule), &(usr_ptr32->i8AudLastGranule), sizeof(__s64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8AudStartOfst), &(usr_ptr32->u8AudStartOfst), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->uAudPacketStartNo), &(usr_ptr32->uAudPacketStartNo), sizeof(__u8)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8AudEndOfst), &(usr_ptr32->u8AudEndOfst), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8AudRangeOfst), &(usr_ptr32->u8AudRangeOfst), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(usr_ptr->u8SeekTime), &(usr_ptr32->u8SeekTime), sizeof(__u64)))
		return -EFAULT;

	*pfgIsUserMem = TRUE;
	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaOgmRange);

	return 0;
}
static long CfaOgmCompatConfig(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaOgmConfigInfo __user *usr_ptr = NULL;
	CfaOgmConfigInfo_T32 __user *usr_ptr32 = (CfaOgmConfigInfo_T32 __user *)prInfo->usr_ptr32;
	__u8 __user *pu1UsrBufAddr = NULL;
	__u8 __user *pu1NextBufAddr = NULL;
	__u32 u4TotalSz = 0;
	compat_caddr_t compatHdrData = 0;
	__u8 __user *header_data = NULL;
	long ret = 0;
	__u32 i = 0;
	__u32 u4UseSz = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaOgmConfigInfo_T32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz(0x%08x/0x%08x, 0x%08x, 0x%08x, 0x%08x).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prInfo->buf_sz, sizeof(CfaOgmConfigInfo_T32),
			sizeof(CfaOgmVidInfo), sizeof(CfaOgmAudInfo_T32) * MAX_NS_OGM_AUD,
			sizeof(CfaOgmConfigInfo));
		return -EINVAL;
	}

	ret = CfaOgmCompatCalcSz(usr_ptr32, &u4TotalSz);
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}

	DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d ---- u4TotalSz:%d.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4TotalSz);
	//pu1UsrBufAddr = (__u8 *)compat_alloc_user_space(u4TotalSz);
	DMX_NewMemory(u4TotalSz, pu1UsrBufAddr);

	if (NULL == pu1UsrBufAddr) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(pu1UsrBufAddr, 0, u4TotalSz);

	usr_ptr = (CfaOgmConfigInfo *)pu1UsrBufAddr;

	pu1NextBufAddr = pu1UsrBufAddr + CFA_ALIGN_SZ(sizeof(CfaOgmConfigInfo), sizeof(uintptr_t));
	u4UseSz += CFA_ALIGN_SZ(sizeof(CfaOgmConfigInfo), sizeof(uintptr_t));
	if (u4UseSz > u4TotalSz) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail for use size > total size.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EINVAL;
	}

	for (i = 0; i < MAX_NS_OGM_AUD; i++) {
		ret = CfaOgmCompatConfigAudInfo(&(usr_ptr->arCfaOgmAudInfo[i]), &(usr_ptr32->arCfaOgmAudInfo[i]));
		if (0 != ret) {
			DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("%s line %d fail in CfaOgmCompatConfigAudInfo(%d).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, i);
			DMX_FreeMemory(pu1UsrBufAddr);
			return ret;
		}
	}

	for (i = 0; i < MAX_NS_OGM_AUD; i++) {
		if (copy_from_user(&(usr_ptr->arCfaOgmAudInfo[i].u4HeaderDataSize), &(usr_ptr32->arCfaOgmAudInfo[i].u4HeaderDataSize), 
				sizeof(__u32))) {
			DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("%s line %d fail in copy_in_user(u4HeaderDataSize).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(pu1UsrBufAddr);
			return -EFAULT;
		}

		if ((0 != usr_ptr32->arCfaOgmAudInfo[i].puHeaderData) &&
			(0 < usr_ptr32->arCfaOgmAudInfo[i].u4HeaderDataSize)) {
			usr_ptr->arCfaOgmAudInfo[i].puHeaderData =  pu1NextBufAddr;
			u4UseSz += CFA_ALIGN_SZ(usr_ptr32->arCfaOgmAudInfo[i].u4HeaderDataSize, sizeof(uintptr_t));
			if (u4UseSz > u4TotalSz) {
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("%s line %d fail for use size > total size.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				DMX_FreeMemory(pu1UsrBufAddr);
				return -EINVAL;
			}

			if (NULL == usr_ptr->arCfaOgmAudInfo[i].puHeaderData) {
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("%s line %d fail for address is null.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				DMX_FreeMemory(pu1UsrBufAddr);
				return -ENOMEM;
			}
			mm_memset(usr_ptr->arCfaOgmAudInfo[i].puHeaderData, 0, usr_ptr32->arCfaOgmAudInfo[i].u4HeaderDataSize);

			if (get_user(compatHdrData, &(usr_ptr32->arCfaOgmAudInfo[i].puHeaderData))) {
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("%s line %d fail in get_user(puHeaderData).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				DMX_FreeMemory(pu1UsrBufAddr);
				return -EFAULT;
			}
			if (0 == compatHdrData) {
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("%s line %d fail for compatHdrData is 0(puHeaderData: 0x%p).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr32->arCfaOgmAudInfo[i].puHeaderData);
				DMX_FreeMemory(pu1UsrBufAddr);
				return -EFAULT;
			}
			header_data = compat_ptr(compatHdrData);
			if (!access_ok(VERIFY_READ, header_data, 
				sizeof(__u8) * usr_ptr->arCfaOgmAudInfo[i].u4HeaderDataSize)) {
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("%s line %d fail in access_ok(puHeaderData).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				DMX_FreeMemory(pu1UsrBufAddr);
				return -EFAULT;
			}

			if (copy_from_user((__u8 __user *)usr_ptr->arCfaOgmAudInfo[i].puHeaderData,
				header_data, sizeof(__u8) * usr_ptr->arCfaOgmAudInfo[i].u4HeaderDataSize)) {
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("%s line %d fail in copy_from_user(usr_ptr->puHeaderData, sz: %d).\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, usr_ptr->arCfaOgmAudInfo[i].u4HeaderDataSize);
				DMX_FreeMemory(pu1UsrBufAddr);
				return -EFAULT;
			}
		}
	}
	if (copy_from_user(&(usr_ptr->uAudioStreamNs), &(usr_ptr32->uAudioStreamNs), sizeof(__u8))) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(uAudioStreamNs).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->u4DurationMs), &(usr_ptr32->u4DurationMs), sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(u4DurationMs).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	
	if (copy_from_user(&(usr_ptr->rCfaOgmVidInfo.eCfaVidCodec),
		&(usr_ptr32->rCfaOgmVidInfo.eCfaVidCodec), sizeof(AVCODECID_T))) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(rCfaOgmVidInfo.eCfaVidCodec).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rCfaOgmVidInfo.u4VidStreamNo),
		&(usr_ptr32->rCfaOgmVidInfo.u4VidStreamNo), sizeof(__u32))) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(rCfaOgmVidInfo.u4VidStreamNo).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rCfaOgmVidInfo.u8TimeUnit),
		&(usr_ptr32->rCfaOgmVidInfo.u8TimeUnit), sizeof(__u64))) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(rCfaOgmVidInfo.u8TimeUnit).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}
	if (copy_from_user(&(usr_ptr->rCfaOgmVidInfo.u8FramePerUnit),
		&(usr_ptr32->rCfaOgmVidInfo.u8FramePerUnit), sizeof(__u64))) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("%s line %d fail in copy_from_user(rCfaOgmVidInfo.u8FramePerUnit).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(pu1UsrBufAddr);
		return -EFAULT;
	}

	*pfgIsUserMem = FALSE;
	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaOgmConfigInfo);

	return 0;
}
static long CfaOgmProcCompat(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	long ret = 0;

	if ((NULL == prInfo) || (NULL == pfgIsUserMem)) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("%s line %d fail for invalid parameter.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
		return -EPERM;
	}
	switch (prInfo->type) {
		case CFA_CONFIG:
			if (prInfo->is_get) {
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EPERM;
			}
			ret = CfaOgmCompatConfig(prInfo, pfgIsUserMem);
			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaOgmCompatConfig.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return ret;
			}
			break;
		case CFA_RANGE:
			if (prInfo->is_get) {
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EPERM;
			}
			ret = CfaOgmCompatRange(prInfo, pfgIsUserMem);
			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaOgmCompatRange.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return ret;
			}
			break;
		case CFA_GEN_INFO:
			DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("%s line %d fail for don't support get info for cfa ogm.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return -EPERM;
		case CFA_JUMP_INFO:
			if (prInfo->is_get) {
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EPERM;
			}
			ret = CfaOgmCompatJumpRange(prInfo, pfgIsUserMem);
			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaOgmCompatJumpRange.\r\n"),
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
 * Name: CfaOgmGetVidType
 *
 * Description:
 *      Cfa transfer Video Codec Enum from LPE to Splitter
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static CfaApiVidType CfaOgmGetVidType(AVCODECID_T eInVidType)
{
	CfaApiVidType eMappedVidType = CFA_VID_UNKNOWN;

	switch (eInVidType) {
	case AVCODEC_ID_UNKNOWN:
		eMappedVidType = CFA_VID_UNKNOWN;
		break;

	case AVCODEC_ID_MPEG1:
		eMappedVidType = CFA_VID_MPEG2;
		break;

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

	case AVCODEC_ID_DIVX4:	/*added at 08-09-05,mcn08033 */
		DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] New Divx4 Codec!\n"));
		eMappedVidType = CFA_VID_DIVX4;
		break;

	case AVCODEC_ID_DIVX6:	/*added at 08-09-05,mcn08033 */
		DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] New Divx4 Codec!\n"));
		eMappedVidType = CFA_VID_DIVX6;
		break;

	default:
		eMappedVidType = CFA_VID_UNKNOWN;
		break;
	}

	return eMappedVidType;
}





/*-----------------------------------------------------------------------------
 * Name: CfaOgmGetAudType
 *
 * Description:
 *      Cfa transfer Audio Codec Enum from LPE to Splitter
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static CfaApiAudType CfaOgmGetAudType(AVCODECID_T eInAudType)
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

	case AVCODEC_ID_FLAC:
		eMappedAudType = CFA_AUD_DRV_FMT_FLAC;
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

	case AVCODEC_ID_RA_COOK:
		eMappedAudType = CFA_AUD_DRV_FMT_RA;
		break;
        
	case AVCODEC_ID_VORBIS:
		eMappedAudType = CFA_AUD_DRV_FMT_VORBIS;
		break;

	default:
		eMappedAudType = CFA_AUD_DRV_FMT_UNKNOWN;
		break;
	}

	return eMappedAudType;
}




/*-----------------------------------------------------------------------------
 * Name: CfaOgmInit
 *
 * Description:
 *      Init CFA OGM
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaOgmInit(void *pvSptHdl, void **ppvCfaPrivData)
{
	CfaOgmInst *prCfaOgm = NULL;

	DMX_NewMemory(sizeof(CfaOgmInst), prCfaOgm);
	if (NULL == prCfaOgm) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] Alloc prCfaOgm memory fail\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}
	dmx_memset((void *) prCfaOgm, 0, sizeof(CfaOgmInst));

	prCfaOgm->u4PrsFlg = 0;

	prCfaOgm->ptrMemAddr = DMX_INVALID_UINTPTR_T;

	prCfaOgm->u4AudNum = 0;
	prCfaOgm->u4SpNum = 0;


	prCfaOgm->u4CurAId = DMX_INVALID_UINT32;
	prCfaOgm->u4CurSpId = DMX_INVALID_UINT32;
	prCfaOgm->u4CurVId = DMX_INVALID_UINT32;
	prCfaOgm->uCurAIndex = DMX_INVALID_UINT8;

	prCfaOgm->fgFirstSetCfaRange = TRUE;
	prCfaOgm->u4DurationMs = 0;

	dmx_memset((void *) &(prCfaOgm->rCfaOgmRange), 0, sizeof(prCfaOgm->rCfaOgmRange));
	prCfaOgm->eCfaOgmCurPrsStrm = CFA_OGM_PRS_STRM_TYPE_NONE;

	prCfaOgm->fgUseCMDQ = FALSE;

	CfaOgmInitPara(prCfaOgm);

	/*prCfaOgm->pCfaDrvIntf = &_rCfaDrvIntf;*/
	*ppvCfaPrivData = (void *) prCfaOgm;

	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
		TEXT("[CFA_OGM] Init Finish!\n"));
	MMATE_CHECK_POINTER(prCfaOgm);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmRange);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmAu);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmVidStream);

	MM_RETURN(RET_DMX_OK);

}



/*-----------------------------------------------------------------------------
 * Name: CfaOgmTxDone
 *
 * Description:
 *      OGM CFA callback for transfer done
 *      This function will be called after a transfer is complete.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: MRESULT
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaOgmTxDone(void *pvSptHdl, u64 u8TxLen, void *pvPrivData, bool fgRsp)
{
	MRESULT mrRet = RET_DMX_OK;
	CfaOgmInst *prCfaOgm = NULL;

	if (NULL == pvPrivData)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	prCfaOgm = (CfaOgmInst *) pvPrivData;
	MMATE_CHECK_POINTER(prCfaOgm);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmRange);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmAu);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmVidStream);

	if (fgRsp) {
		if (prCfaOgm->fgUseCMDQ) {
			prCfaOgm->fgRealSyncPb = TRUE;
			prCfaOgm->fgNoNeedSyncPb = FALSE;
			prCfaOgm->u4AvalSize = 0;
			prCfaOgm->u8Ca = prCfaOgm->u8RspCa;
			prCfaOgm->u4TxLen = prCfaOgm->u8RspTxLen;
			prCfaOgm->u8LastReadOfst = prCfaOgm->u8Ca;
			prCfaOgm->eCfaOgmCurState = prCfaOgm->eRspState;
			prCfaOgm->rCurPage = prCfaOgm->rRspPage;
			prCfaOgm->rCurPacket = prCfaOgm->rRspPacket;
			
			DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] %s line:%d,RspCa:0x%llx, RspTxLen:0x%llx,eRspState:0x%d\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prCfaOgm->u8RspCa, prCfaOgm->u8RspTxLen, prCfaOgm->eRspState);
			mrRet = Spt4CfaPbb2SyncBufEx(pvSptHdl, prCfaOgm->u8Ca, prCfaOgm->u4TxLen,
							     (u8 *) &(prCfaOgm->ptrMemAddr),
							     &(prCfaOgm->u4AvalSize));		
			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("[CFA_OGM] Spt4CfaPbb2SyncBufEx error ret = %d\n"),
					mrRet);
			}
			u8TxLen = prCfaOgm->u8RspTxLen;

			dmx_memset((VOID *)(&prCfaOgm->rVidCmdQInfo),0,sizeof(prCfaOgm->rVidCmdQInfo));
			dmx_memset((VOID *)(&prCfaOgm->rAudCmdQInfo),0,sizeof(prCfaOgm->rAudCmdQInfo));
			dmx_memset((VOID *)(prCfaOgm->arAudCmdQInfo),0,sizeof(prCfaOgm->arAudCmdQInfo[0])*MAX_NS_OGM_AUD);
			dmx_memset((VOID *)(prCfaOgm->arCmdQStartPage),0,sizeof(prCfaOgm->arCmdQStartPage[0])*MAX_NS_OGM_AUD);
			dmx_memset((VOID *)(prCfaOgm->arCmdQStartPacket),0,sizeof(prCfaOgm->arCmdQStartPacket[0])*MAX_NS_OGM_AUD);
			dmx_memset((VOID *)(&prCfaOgm->rCmdQStartPage),0,sizeof(prCfaOgm->rCmdQStartPage));
			dmx_memset((VOID *)(&prCfaOgm->rCmdQStartPacket),0,sizeof(prCfaOgm->rCmdQStartPacket));
			dmx_memset((VOID *)(&prCfaOgm->rVidCmdQStartPage),0,sizeof(prCfaOgm->rVidCmdQStartPage));
			dmx_memset((VOID *)(&prCfaOgm->rVidCmdQStartPacket),0,sizeof(prCfaOgm->rVidCmdQStartPacket));
			dmx_memset((VOID *)(prCfaOgm->au8TxAudCmdQIndex),0,sizeof(UINT8)*MAX_NS_OGM_AUD);
			dmx_memset((VOID *)(prCfaOgm->arOgmInstCmdEntrys),0,sizeof(DMX_CMDQ_TX_ENTRY_T) * DMX_MAX_TX_CNT_FOR_CMD_Q);

			MM_RETURN(mrRet);
		} else {
			prCfaOgm->fgRealSyncPb = TRUE;
			prCfaOgm->fgNoNeedSyncPb = FALSE;
			prCfaOgm->u4AvalSize = 0;
			prCfaOgm->u4TxLen = u8TxLen;
			mrRet = Spt4CfaPbb2SyncBufEx(pvSptHdl, prCfaOgm->u8Ca, u8TxLen,
							 (u8 *) &(prCfaOgm->ptrMemAddr),
							 &(prCfaOgm->u4AvalSize));
			if (RET_DMX_OK != mrRet) {
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("[CFA OGM] Spt4CfaPbb2SyncBufEx error ret = %d\n"),
					mrRet);
			}
			MM_RETURN(mrRet);
		}
	}

#if CONFIG_CFA_OGM_NEW_SYNCBUF
#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
	if ((prCfaOgm->eCfaOgmCurState != CFA_OGM_ST_PACKET_TX)
	    && (prCfaOgm->eCfaOgmCurState != CFA_OGM_ST_AAC_PACKET_ANA)) {
		prCfaOgm->fgIfRebuf = TRUE;
	}
#endif
#endif
	CfaOgmTxDoneStCtrl(pvSptHdl, u8TxLen, prCfaOgm);

	MM_RETURN(RET_DMX_OK);
}



/*-----------------------------------------------------------------------------
 * Name: CfaOgmSetStrmInf
 *
 * Description:
 *      Set Stream information
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaOgmSetStrmInf(void *pvSptHdl, u32 u4Strm, u32 u4Info, void *pvPrivData)
{
	CfaOgmInst *prCfaOgm = NULL;

	if (NULL == pvPrivData)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
		TEXT("[CFA_OGM] %s enter!%d\n"), DMX_FUNC_NAME, DMX_LINE_NO);

	prCfaOgm = (CfaOgmInst *) pvPrivData;
	MMATE_CHECK_POINTER(prCfaOgm);

	if ((u32)CFA_STRM_V == u4Strm) {
		prCfaOgm->u4CurVId = u4Info;
		DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] Set Video StreamInf!\n"));
	} else if ((u32)CFA_STRM_A == u4Strm) {
		prCfaOgm->u4CurAId = u4Info;
		prCfaOgm->uCurAIndex = CfaOgmGetAudIndex(prCfaOgm, u4Info);
		DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] Set Audio StreamInf!\n"));

	} else if ((u32)CFA_STRM_SP == u4Strm) {
		/*subtitle is not support currently */
		DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] Set Subtitle StreamInf!\n"));
	} else {
		DMXLOG_WARN(TEXT("[CFA_OGM] Set StreamInf Error!\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	MM_RETURN(RET_DMX_OK);
}



/*-----------------------------------------------------------------------------
 * Name: CfaOgmFillAUInfo
 *
 * Description:
 *      OGM CFA sets AU table information
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaOgmFillAUInfo(void *pvSptHdl, void *pvAUInfo, void *pvAUExtInfo, void *pvPrivData)
{
	CfaOgmInst *prCfaOgm = NULL;

    if ((NULL == pvAUInfo) || (NULL == pvPrivData)) {
    	MM_RETURN(RET_DMX_PARAM_WRONG);
    }

	prCfaOgm = (CfaOgmInst *) pvPrivData;
	MMATE_CHECK_POINTER(prCfaOgm);
	MMATE_CHECK_STRUCT(prCfaOgm->rCurPacket);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmRange);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmAu);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmVidStream);

	if (prCfaOgm->fgUseCMDQ) {
		u64 u8RealPTS = 0;
		u64 u8FileOfst = ((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Offset;
		u32 u4Idx = 0;
		u32 u4Idx2 = 0;
	
		if (prCfaOgm->arAudCmdQInfo[prCfaOgm->uIdxForFillAU].fgIsInDma) {
			if (prCfaOgm->arAudCmdQInfo[prCfaOgm->uIdxForFillAU].u4EntryCnt > 0) {
				bool   fgInUnit = FALSE;
				for (u4Idx = 0; u4Idx < prCfaOgm->arAudCmdQInfo[prCfaOgm->uIdxForFillAU].u4EntryCnt; u4Idx++) {
					if (prCfaOgm->arAudCmdQInfo[prCfaOgm->uIdxForFillAU].arEntrys[u4Idx].fgUnitStart) {
						if (!fgInUnit) {
							fgInUnit = TRUE;
							prCfaOgm->arAudCmdQInfo[prCfaOgm->uIdxForFillAU].arEntrys[u4Idx].fgUnitStart = FALSE;
							u8RealPTS = prCfaOgm->arAudCmdQInfo[prCfaOgm->uIdxForFillAU].arEntrys[u4Idx].u8Pts;
							DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
								TEXT("[CFA_OGM]StreamId:%d, u4Idx:%d, Audio Au pts:%d\r\n"),
								prCfaOgm->uIdxForFillAU, u4Idx, PTS_TO_MS(u8RealPTS));
							break;
						}
					}
				}
				/*set the fgUnitStart to True because CfaOgmFillAuInfo will be called two times sometimes*/
				if(u4Idx >= prCfaOgm->arAudCmdQInfo[prCfaOgm->uIdxForFillAU].u4EntryCnt - 1) {
					for(u4Idx2 = 0; u4Idx2 < prCfaOgm->arAudCmdQInfo[prCfaOgm->uIdxForFillAU].u4EntryCnt; u4Idx2++) {
						prCfaOgm->arAudCmdQInfo[prCfaOgm->uIdxForFillAU].arEntrys[u4Idx2].fgUnitStart = TRUE;
					}
				}

			}
			
			if(prCfaOgm->arCmdQStartPage[prCfaOgm->uIdxForFillAU].uAudIdx >= MAX_NS_OGM_AUD) {
				prCfaOgm->arCmdQStartPage[prCfaOgm->uIdxForFillAU].uAudIdx = MAX_NS_OGM_AUD - 1;
			}

			prCfaOgm->u8AudLastGranule = prCfaOgm->arCmdQStartPacket[prCfaOgm->uIdxForFillAU].au8LastAudGranule[prCfaOgm->rCurPage.uAudIdx]; /*store audio information for stop-resume*/
			prCfaOgm->u8AudStartOffset = prCfaOgm->arCmdQStartPage[prCfaOgm->uIdxForFillAU].u8StartOfst;
			if (prCfaOgm->arCmdQStartPage[prCfaOgm->uIdxForFillAU].fgFreshPacket == TRUE) {
				 prCfaOgm->uAudPacketNs = prCfaOgm->arCmdQStartPacket[prCfaOgm->uIdxForFillAU].uAudPacketNo;
			} else {
				 prCfaOgm->uAudPacketNs = prCfaOgm->arCmdQStartPacket[prCfaOgm->uIdxForFillAU].uAudPacketNo - 1;/*convert to LPE's concept of packet no*/
			}

			if (prCfaOgm->arCmdQStartPacket[prCfaOgm->uIdxForFillAU].u8AudStartPTS != DMX_INVALID_UINT64) {
				prCfaOgm->u8AudLastRealPTS = prCfaOgm->arCmdQStartPacket[prCfaOgm->uIdxForFillAU].u8AudStartPTS;
			}
	
			((AU_AUDIO *)pvAUInfo)->rAUInfo.rInfo.u8Pts = u8RealPTS;

			DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM]%s line %d, (CMDQ)Audio AU PTS:%lldms, StreamIndex:%d, u4Idx:%d\r\n"),
				__func__, __LINE__, PTS_TO_MS(((AU_AUDIO *)pvAUInfo)->rAUInfo.rInfo.u8Pts),
				prCfaOgm->uIdxForFillAU,
				u4Idx);
	
			MM_RETURN(RET_DMX_OK);
		}
		if (prCfaOgm->rVidCmdQInfo.fgIsInDma) {
			bool fgFind = FALSE;
			
			if (CFA_VID_DIVX3 == prCfaOgm->rCfaOgmVidStream.eCfaVidCodec) {
				u32 u4Idx = 0;
	
				for (u4Idx = 0; u4Idx < prCfaOgm->rVidCmdQInfo.u4EntryCnt; u4Idx++) {
					if (prCfaOgm->rVidCmdQInfo.arEntrys[u4Idx].fgUnitEnd) {
						prCfaOgm->rVidCmdQInfo.arEntrys[u4Idx].fgUnitEnd = FALSE;
						u8RealPTS = prCfaOgm->rVidCmdQInfo.arEntrys[u4Idx].u8Pts;
						break;
					}
				}
			} else {
				for (u4Idx = 0; u4Idx < prCfaOgm->rVidCmdQInfo.u4EntryCnt; u4Idx++) {
					if ((prCfaOgm->rVidCmdQInfo.arEntrys[u4Idx].u8FileOffset<= u8FileOfst) &&
						(u8FileOfst <= prCfaOgm->rVidCmdQInfo.arEntrys[u4Idx].u8FileOffset +
							prCfaOgm->rVidCmdQInfo.arEntrys[u4Idx].u4Len)) {
						u8RealPTS = prCfaOgm->rVidCmdQInfo.arEntrys[u4Idx].u8Pts;
						fgFind = TRUE;
						prCfaOgm->u8PrePTSForComposeAU = u8RealPTS;
						break;
					}
				}
	
				if (!fgFind) {
					u8RealPTS = prCfaOgm->u8PrePTSForComposeAU;
				}
			}
			
			if ((fgIsIType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType))	 /*store information of keyframe to prCfaOgm*/
				 || (prCfaOgm->rCurPacket.fgKeyFrame)) {
				 prCfaOgm->u8VidKeyLastGranule = prCfaOgm->rCfaOgmAu.u8LastVidGranule;
				 prCfaOgm->u8VidKeyStartOffset = prCfaOgm->rCfaOgmAu.u8StartOfst;
				 prCfaOgm->uVidKeyPacketNs = prCfaOgm->rCfaOgmAu.uVidPacketNo;
	
#if CONFIG_CFA_OGM_LENBYTES_ISNOT_ZERO
				 if (prCfaOgm->rCfaOgmVidStream.u1GotVItypeNo < 3) {
					 prCfaOgm->rCfaOgmVidStream.u1GotVItypeNo++;
				 }
#endif
			}
	
			if ((fgIsBType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) && (prCfaOgm->ePrevPicType == CFA_OGM_PIC_P)) {
				 prCfaOgm->fgAdjustPTS = TRUE;
	
#if CONFIG_CFA_OGM_PTS_ADJUST_SUPPORT
				 ((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8PrevPTS = DMX_INVALID_UINT64;
#endif
			}
	
			if ((fgIsBType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) && (prCfaOgm->ePrevPicType == CFA_OGM_PIC_I)) {
				prCfaOgm->fgAdjustPTS = TRUE;
			}
	
			if ((!fgIsBType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) && (prCfaOgm->fgAdjustPTS == TRUE)) {
				 prCfaOgm->fgAdjustPTS = FALSE;
			}
	
			/*give some information about keyframe to LPE*/
	
			//((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Offset = u8FileOfst;//prCfaOgm->rVidCmdQStartPacket.u8VidStartOfst;
			((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = u8RealPTS;
			((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.eDiscType = DT_DATADISC;

			DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] %s line:%d,(CMDQ)Video Au pts:%lld ms\n"),
				DMX_FUNC_NAME,DMX_LINE_NO,PTS_TO_MS(u8RealPTS));
	
#if CONFIG_CFA_OGM_PTS_ADJUST_SUPPORT
			if ((fgIsBType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) && (prCfaOgm->fgAdjustPTS == TRUE) &&
				 (prCfaOgm->rCurPacket.u4VidPacketTotalNo != prCfaOgm->u4LastVidPacketNo) && (prCfaOgm->u4LastVidPacketNo != 0)) {
				 ((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts -= prCfaOgm->rVidCmdQStartPage.u8VidPtsPerGranule;
			}
#endif
	
			prCfaOgm->rVidCmdQStartPacket.u8LastVidStartPTS = prCfaOgm->rVidCmdQStartPacket.u8VidStartPTS;
	
			prCfaOgm->u4LastVidPacketNo = prCfaOgm->rVidCmdQStartPacket.u4VidPacketTotalNo;
	
			if (fgIsIType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) {
				 prCfaOgm->ePrevPicType = CFA_OGM_PIC_I ;
			} else if (fgIsBType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) {
				 prCfaOgm->ePrevPicType = CFA_OGM_PIC_B ;
			} else if (fgIsPType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) {
				 prCfaOgm->ePrevPicType = CFA_OGM_PIC_P ;
			} else {
				 prCfaOgm->ePrevPicType = CFA_OGM_PIC_UNKNOWN;
			}
			MM_RETURN(RET_DMX_OK);
		}
	
		switch(prCfaOgm->rCurPage.eCfaOgmStrmType) {
			case CFA_OGM_PRS_STRM_TYPE_V:
			{
				if ((fgIsIType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType))	 /*store information of keyframe to prCfaOgm*/
					 || (prCfaOgm->rCurPacket.fgKeyFrame)) {
					 prCfaOgm->u8VidKeyLastGranule = prCfaOgm->rCfaOgmAu.u8LastVidGranule;
					 prCfaOgm->u8VidKeyStartOffset = prCfaOgm->rCfaOgmAu.u8StartOfst;
					 prCfaOgm->uVidKeyPacketNs = prCfaOgm->rCfaOgmAu.uVidPacketNo;
	
#if CONFIG_CFA_OGM_LENBYTES_ISNOT_ZERO
					 if (prCfaOgm->rCfaOgmVidStream.u1GotVItypeNo < 3) {
						 prCfaOgm->rCfaOgmVidStream.u1GotVItypeNo++;
					 }
#endif
				}
	
				if ((fgIsBType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) && (prCfaOgm->ePrevPicType == CFA_OGM_PIC_P)) {
					 prCfaOgm->fgAdjustPTS = TRUE;
	
#if CONFIG_CFA_OGM_PTS_ADJUST_SUPPORT
					 ((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8PrevPTS = DMX_INVALID_UINT64;
#endif
				}
	
				if ((fgIsBType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) && (prCfaOgm->ePrevPicType == CFA_OGM_PIC_I)) {
					prCfaOgm->fgAdjustPTS = TRUE;
				}
	
				if ((!fgIsBType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) && (prCfaOgm->fgAdjustPTS == TRUE)) {
					 prCfaOgm->fgAdjustPTS = FALSE;
				}
	
				/*give some information about keyframe to LPE*/	
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Offset = prCfaOgm->rCurPacket.u8VidStartOfst;
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = prCfaOgm->rCurPacket.u8VidStartPTS;
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.eDiscType = DT_DATADISC;
	
#if CONFIG_CFA_OGM_PTS_ADJUST_SUPPORT
				if ((fgIsBType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) && (prCfaOgm->fgAdjustPTS == TRUE) &&
					 (prCfaOgm->rCurPacket.u4VidPacketTotalNo != prCfaOgm->u4LastVidPacketNo) && (prCfaOgm->u4LastVidPacketNo != 0)) {
					 ((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts -= prCfaOgm->rCurPage.u8VidPtsPerGranule;
				}
#endif
	
				prCfaOgm->rCurPacket.u8LastVidStartPTS = prCfaOgm->rCurPacket.u8VidStartPTS;
	
				prCfaOgm->u4LastVidPacketNo = prCfaOgm->rCurPacket.u4VidPacketTotalNo;
	
				if (fgIsIType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) {
					 prCfaOgm->ePrevPicType = CFA_OGM_PIC_I ;
				} else if (fgIsBType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) {
					 prCfaOgm->ePrevPicType = CFA_OGM_PIC_B ;
				} else if (fgIsPType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) {
					 prCfaOgm->ePrevPicType = CFA_OGM_PIC_P ;
				} else {
					 prCfaOgm->ePrevPicType = CFA_OGM_PIC_UNKNOWN;
				}
	
				break;
			 }
	
			case CFA_OGM_PRS_STRM_TYPE_A:
			{
				if (prCfaOgm->rCurPage.u4StreamNo == prCfaOgm->u4CurAId) {
					if(prCfaOgm->rCurPage.uAudIdx >= MAX_NS_OGM_AUD) {
						prCfaOgm->rCurPage.uAudIdx = MAX_NS_OGM_AUD - 1;
					}
					prCfaOgm->u8AudLastGranule = prCfaOgm->rCurPacket.au8LastAudGranule[prCfaOgm->rCurPage.uAudIdx]; /*store audio information for stop-resume*/
					prCfaOgm->u8AudStartOffset = prCfaOgm->rCurPage.u8StartOfst;
					if (prCfaOgm->rCurPage.fgFreshPacket == TRUE) {
						 prCfaOgm->uAudPacketNs = prCfaOgm->rCurPacket.uAudPacketNo;
					} else {
						 prCfaOgm->uAudPacketNs = prCfaOgm->rCurPacket.uAudPacketNo - 1;/*convert to LPE's concept of packet no*/
					}
	
				    if (prCfaOgm->rCurPacket.u8AudStartPTS != DMX_INVALID_UINT64) {
					    prCfaOgm->u8AudLastRealPTS = prCfaOgm->rCurPacket.u8AudStartPTS;
				    }
				}
		
			  ((AU_AUDIO *)pvAUInfo)->rAUInfo.rInfo.u8Pts = prCfaOgm->rCurPacket.u8AudStartPTS;

				DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("[CFA_OGM]%s line %d, (CMDQ)Audio AU PTS:%lldms, StreamIndex:%d\r\n"),
					__func__, __LINE__, PTS_TO_MS(((AU_AUDIO *)pvAUInfo)->rAUInfo.rInfo.u8Pts),
					prCfaOgm->rCurPage.uAudIdx);
	
				break;
			}
	
			default:
				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("[CFA_OGM] %s,line %d,prCfaOgm->rCurPage.eCfaOgmStrmType is error\r\n"),DMX_FUNC_NAME, DMX_LINE_NO);
				MM_RETURN(RET_DMX_UNSUPPORT);
		}
	
		MMATE_CHECK_STRUCT(prCfaOgm->rCurPacket);
		
		MM_RETURN(RET_DMX_OK);

	}


	if (prCfaOgm->rAudCmdQInfo.fgIsInDma) {
		u64 u8RealPTS = 0;
		
		if (prCfaOgm->rAudCmdQInfo.u4EntryCnt > 0) {
			u32 u4Idx = 0;
			bool   fgInUnit = FALSE;
			for (u4Idx = 0; u4Idx < prCfaOgm->rAudCmdQInfo.u4EntryCnt; u4Idx++) {
				if (prCfaOgm->rAudCmdQInfo.arEntrys[u4Idx].fgUnitStart) {
					if (!fgInUnit) {
						fgInUnit = TRUE;
						prCfaOgm->rAudCmdQInfo.arEntrys[u4Idx].fgUnitStart = FALSE;
						u8RealPTS = prCfaOgm->rAudCmdQInfo.arEntrys[u4Idx].u8Pts;
						break;
					}
				}
			}
		}

		if (prCfaOgm->rCmdQStartPage.u4StreamNo == prCfaOgm->u4CurAId) {
			if(prCfaOgm->rCmdQStartPage.uAudIdx >= MAX_NS_OGM_AUD)
				prCfaOgm->rCmdQStartPage.uAudIdx = MAX_NS_OGM_AUD - 1;
			/*store audio information for stop-resume */
			prCfaOgm->u8AudLastGranule =
				prCfaOgm->rCmdQStartPacket.au8LastAudGranule[prCfaOgm->rCurPage.uAudIdx];
			prCfaOgm->u8AudStartOffset = prCfaOgm->rCmdQStartPage.u8StartOfst;
			if (prCfaOgm->rCmdQStartPage.fgFreshPacket)
				prCfaOgm->uAudPacketNs = prCfaOgm->rCmdQStartPacket.uAudPacketNo;
			else {
				/*convert to LPE's concept of packet no */
				prCfaOgm->uAudPacketNs = prCfaOgm->rCmdQStartPacket.uAudPacketNo - 1;
			}

			if (prCfaOgm->rCmdQStartPacket.u8AudStartPTS != DMX_INVALID_UINT64) {
				prCfaOgm->u8AudLastRealPTS =
				    prCfaOgm->rCmdQStartPacket.u8AudStartPTS;
			}
		}

		((AU_AUDIO *) pvAUInfo)->rAUInfo.rInfo.u8Pts =
		    u8RealPTS;

		MM_RETURN(RET_DMX_OK);
	}

	switch (prCfaOgm->rCurPage.eCfaOgmStrmType) {
	case CFA_OGM_PRS_STRM_TYPE_V:
		{
			/*store information of keyframe to prCfaOgm */
			if ((fgIsIType(((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u4VType))
				|| (prCfaOgm->rCurPacket.fgKeyFrame)) {
				prCfaOgm->u8VidKeyLastGranule =
				    prCfaOgm->rCfaOgmAu.u8LastVidGranule;
				prCfaOgm->u8VidKeyStartOffset = prCfaOgm->rCfaOgmAu.u8StartOfst;
				prCfaOgm->uVidKeyPacketNs = prCfaOgm->rCfaOgmAu.uVidPacketNo;

#if CONFIG_CFA_OGM_LENBYTES_ISNOT_ZERO
				if (prCfaOgm->rCfaOgmVidStream.u1GotVItypeNo < 3)
					prCfaOgm->rCfaOgmVidStream.u1GotVItypeNo++;
#endif
			}

			if ((fgIsBType(((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u4VType))
			    && (prCfaOgm->ePrevPicType == CFA_OGM_PIC_P)) {
				prCfaOgm->fgAdjustPTS = TRUE;

#if CONFIG_CFA_OGM_PTS_ADJUST_SUPPORT
				((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8PrevPTS =
				    DMX_INVALID_UINT64;
#endif
			}

			if ((fgIsBType(((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u4VType))
			    && (prCfaOgm->ePrevPicType == CFA_OGM_PIC_I)) {
				prCfaOgm->fgAdjustPTS = TRUE;
			}

			if ((!fgIsBType(((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u4VType))
			    && (prCfaOgm->fgAdjustPTS == TRUE)) {
				prCfaOgm->fgAdjustPTS = FALSE;
			}

			/*give some information about keyframe to LPE */

			((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Offset =
			    prCfaOgm->rCurPacket.u8VidStartOfst;
			((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Pts =
			    prCfaOgm->rCurPacket.u8VidStartPTS;
			((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.eDiscType = DT_DATADISC;

#if CONFIG_CFA_OGM_PTS_ADJUST_SUPPORT
			if ((fgIsBType(((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u4VType))
			    && (prCfaOgm->fgAdjustPTS == TRUE)
			    && (prCfaOgm->rCurPacket.u4VidPacketTotalNo !=
				prCfaOgm->u4LastVidPacketNo)
			    && (prCfaOgm->u4LastVidPacketNo != 0)) {
				((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Pts -=
				    prCfaOgm->rCurPage.u8VidPtsPerGranule;
			}
#endif
			DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("Video AU PTS:%lldms\r\n"),
				((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u8Pts / 90);

			prCfaOgm->rCurPacket.u8LastVidStartPTS = prCfaOgm->rCurPacket.u8VidStartPTS;
				
			prCfaOgm->u4LastVidPacketNo = prCfaOgm->rCurPacket.u4VidPacketTotalNo;

			if (fgIsIType(((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u4VType))
				prCfaOgm->ePrevPicType = CFA_OGM_PIC_I;
			else if (fgIsBType(((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u4VType))
				prCfaOgm->ePrevPicType = CFA_OGM_PIC_B;
			else if (fgIsPType(((AU_VPic *) pvAUInfo)->rAUInfo.rInfo.u4VType))
				prCfaOgm->ePrevPicType = CFA_OGM_PIC_P;
			else
				prCfaOgm->ePrevPicType = CFA_OGM_PIC_UNKNOWN;

			break;
		}

	case CFA_OGM_PRS_STRM_TYPE_A:
		{
			if (prCfaOgm->rCurPage.u4StreamNo == prCfaOgm->u4CurAId) {
				if(prCfaOgm->rCurPage.uAudIdx >= MAX_NS_OGM_AUD) {
					prCfaOgm->rCurPage.uAudIdx = MAX_NS_OGM_AUD - 1;
				}

				/*store audio information for stop-resume */
				prCfaOgm->u8AudLastGranule =
					prCfaOgm->rCurPacket.au8LastAudGranule[prCfaOgm->rCurPage.uAudIdx];
				prCfaOgm->u8AudStartOffset = prCfaOgm->rCurPage.u8StartOfst;
				if (prCfaOgm->rCurPage.fgFreshPacket)
					prCfaOgm->uAudPacketNs = prCfaOgm->rCurPacket.uAudPacketNo;
				else{
					/*convert to LPE's concept of packet no */
					prCfaOgm->uAudPacketNs = prCfaOgm->rCurPacket.uAudPacketNo - 1;
				}

				if (prCfaOgm->rCurPacket.u8AudStartPTS != DMX_INVALID_UINT64)
					prCfaOgm->u8AudLastRealPTS =
					    prCfaOgm->rCurPacket.u8AudStartPTS;
			}

			((AU_AUDIO *) pvAUInfo)->rAUInfo.rInfo.u8Pts =
			    prCfaOgm->rCurPacket.u8AudStartPTS;

			break;
		}

	default:
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] %s,line %d,prCfaOgm->rCurPage.eCfaOgmStrmType is error\r\n"),DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_UNSUPPORT);


		break;
	}

	MMATE_CHECK_STRUCT(prCfaOgm->rCurPacket);

	MM_RETURN(RET_DMX_OK);
}





/*-----------------------------------------------------------------------------
 * Name: i4CfaOgmEnableStrm
 *
 * Description:
 *      OGM CFA sets stream to parse, may be combinations of V/A/S.
 *      splitter will ensure that pfvSetStrm() is only called in "off" or "paused" state.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaOgmEnableStrm(void *pvSptHdl, u32 u4StrmToPrs, CfaStreamOp eOp, void *pvPrivData)
{
	CfaOgmInst *prCfaOgm = NULL;

	if (NULL == pvPrivData)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	prCfaOgm = (CfaOgmInst *) pvPrivData;
	MMATE_CHECK_POINTER(prCfaOgm);

	if (CFA_STREAM_ON == eOp) {
		/* enable */
		if (((u32)CFA_STRM_V & u4StrmToPrs)
		    && (prCfaOgm->rCfaOgmVidStream.eCfaVidCodec != CFA_VID_UNKNOWN)) {
			prCfaOgm->u4PrsFlg |= CFA_OGM_PRS_STRM_TYPE_V;
			DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] Enable Video Stream!\n"));
		}

		if ((u32)CFA_STRM_A & u4StrmToPrs) {
			if (prCfaOgm->uCurAIndex >= MAX_NS_OGM_AUD)
				prCfaOgm->uCurAIndex = MAX_NS_OGM_AUD - 1;
			
			prCfaOgm->u4PrsFlg |= CFA_OGM_PRS_STRM_TYPE_A;
			DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] Enable Audio Stream!\n"));
		}

		if ((u32)CFA_STRM_SP & u4StrmToPrs) {
			prCfaOgm->u4PrsFlg |= CFA_OGM_PRS_STRM_TYPE_SP;
			DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] Enable Subtitle Stream!\n"));
		}
	} else {		/* CFA_STRM_OFF */

		/* disable */
		if ((u32)CFA_STRM_V & u4StrmToPrs) {
			prCfaOgm->u4PrsFlg &= ~((u32) CFA_OGM_PRS_STRM_TYPE_V);
			DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] Disable Video Stream!\n"));
		}

		if ((u32)CFA_STRM_A & u4StrmToPrs) {
			/* audio and subtitle should not be disabled
			   prCfaOgm->u4PrsFlg &= ~((u32)CFA_OGM_PRS_STRM_TYPE_A);
			 */
		}

		if ((u32)CFA_STRM_SP & u4StrmToPrs) {
			/* audio and subtitle should not be disabled
			   prCfaOgm->u4PrsFlg &= ~((u32)CFA_OGM_PRS_STRM_TYPE_SP);
			 */
		}
	}

	MM_RETURN(RET_DMX_OK);
}




/*-----------------------------------------------------------------------------
 * Name: CfaOgmTurnOn
 *
 * Description:
 *      OGM CFA turns on file demuxing
 *      A transfer should be issued in this function.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: s32
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaOgmTurnOn(void *pvSptHdl, void *pvPrivData)
{
	CfaOgmInst *prCfaOgm = NULL;
	u32 i = 0;

	if (NULL == pvPrivData)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	prCfaOgm = (CfaOgmInst *) pvPrivData;
	CfaOgmInitPara(prCfaOgm);
	MMATE_CHECK_POINTER(prCfaOgm);
	MMATE_CHECK_STRUCT(prCfaOgm->rCurPacket);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmRange);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmAu);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmVidStream);

	if ((prCfaOgm->rCfaOgmRange.fgAudRangeEn == TRUE)
	    && (prCfaOgm->rCfaOgmRange.fgVidRangeEn == FALSE)) {
		prCfaOgm->u8Ca = prCfaOgm->rCfaOgmRange.u8AudStartOfst;
		DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] Range has only audio info!!\n"));
	} else if ((prCfaOgm->rCfaOgmRange.fgAudRangeEn == FALSE)
		   && (prCfaOgm->rCfaOgmRange.fgVidRangeEn == TRUE)) {
		prCfaOgm->u8Ca = prCfaOgm->rCfaOgmRange.u8VidStartOfst;
		DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] Range has only video info!!\n"));
	} else if ((prCfaOgm->rCfaOgmRange.fgAudRangeEn == TRUE)
		   && (prCfaOgm->rCfaOgmRange.fgVidRangeEn == TRUE)) {
		prCfaOgm->u8Ca =
		    (prCfaOgm->rCfaOgmRange.u8VidStartOfst > prCfaOgm->rCfaOgmRange.u8AudStartOfst)
		    ? (prCfaOgm->rCfaOgmRange.u8AudStartOfst) : (prCfaOgm->rCfaOgmRange.
								 u8VidStartOfst);
		if (prCfaOgm->rCfaOgmRange.u8VidStartOfst > prCfaOgm->rCfaOgmRange.u8AudStartOfst) {
			DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				    TEXT
				    ("[CFA_OGM] Range has all video and audio,offset of audio before video!\n"));
		} else {
			DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				    TEXT
				    ("[CFA_OGM] Range has all video and audio,offset of video before audio!\n"));
		}
	} else {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] Range not enable!\n"));
		Spt4CfaFinishedEx(pvSptHdl, prCfaOgm->u8Ca, FALSE, (u32)GAU_E_EOS);
	}

	if ((prCfaOgm->u8Ca) >=
	    ((prCfaOgm->rCfaOgmRange.u8AudEndOfst >
	      prCfaOgm->rCfaOgmRange.u8VidEndOfst) ? (prCfaOgm->rCfaOgmRange.
						      u8AudEndOfst) : (prCfaOgm->rCfaOgmRange.
								       u8VidEndOfst))) {
		Spt4CfaFinishedEx(pvSptHdl, prCfaOgm->u8Ca, FALSE, (u32)GAU_E_EOS);
		DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] Send EOS Finish!\n"));
	}

	for (i = 0; i < prCfaOgm->u4AudNum; i++) {
		prCfaOgm->arCfaOgmAudStream[i].fgNeedTxVorbisHeaderData = FALSE;

		if ((CFA_AUD_DRV_FMT_VORBIS == prCfaOgm->arCfaOgmAudStream[i].eCfaAudCodec)
		    && ((prCfaOgm->rCfaOgmRange.u8SeekTime > 0) || DMX_IS_RW_PLAY(pvSptHdl))) {
			prCfaOgm->arCfaOgmAudStream[i].fgNeedTxVorbisHeaderData = TRUE;
			DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				    TEXT
				    ("[CFA_OGM] Turn on   need tx vorbis first page, auido index: %d!\n"),
				    i);
		}
	}
	prCfaOgm->u4FRAudDataTxLen = 0;
	prCfaOgm->fgRWFinish = FALSE;

	CfaOgmNextState(pvSptHdl, prCfaOgm, (u64)CFA_OGM_HDR_READ_BYTES, CFA_OGM_ST_PAGE_HDR_ANA);

	DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
		TEXT("[CFA_OGM] Turn on  parsing!UseCMDQ:%d\n"), prCfaOgm->fgUseCMDQ);

	MM_RETURN(RET_DMX_OK);
}




/*-----------------------------------------------------------------------------
 * Name: CfaOgmSetRange
 *
 * Description:
 *      OGM CFA sets demuxing range
 *      splitter will ensure that pfvSetRange is only called in "off" state.
 *      If used with MPC, the range of MPC_SCMD_SPR will be passed here
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: MRESULT
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaOgmSetRange(void *pvSptHdl, void *pvRange, void *pvPrivData, bool fgIsUserMem)
{
	CfaOgmInst *prCfaOgm = NULL;

	if ((NULL == pvPrivData) || (NULL == pvRange))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	prCfaOgm = (CfaOgmInst *) pvPrivData;

#ifdef MM_ATE_CHECK
	if (0 != mm_copy_from_user(&(prCfaOgm->rCfaOgmRange.fgVidRangeEn),
		&(((CfaOgmRange *) pvRange)->fgVidRangeEn),
		   sizeof(prCfaOgm->rCfaOgmRange) - 2 * sizeof(u32))) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_AUDIO] %s line %d failed in mm_copy_from_user\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}
#else
	if (0 != mm_copy_from_user(&(prCfaOgm->rCfaOgmRange),
		pvRange, sizeof(prCfaOgm->rCfaOgmRange))) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_AUDIO] %s line %d failed in mm_copy_from_user\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}
#endif

	MMATE_INIT_STRUCT(prCfaOgm->rCfaOgmRange);
	MMATE_CHECK_POINTER(prCfaOgm);
	MMATE_CHECK_STRUCT(prCfaOgm->rCurPacket);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmRange);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmAu);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmVidStream);

	if (prCfaOgm->fgFirstSetCfaRange) {
		prCfaOgm->u4FirstVidStartOfst = prCfaOgm->rCfaOgmRange.u8VidStartOfst;
		prCfaOgm->u4FirstAudStartOfst = prCfaOgm->rCfaOgmRange.u8AudStartOfst;
	}
	prCfaOgm->fgFirstSetCfaRange = FALSE;

	if (prCfaOgm->rCfaOgmRange.fgAudRangeEn == FALSE) {
		prCfaOgm->rCfaOgmRange.u8AudStartOfst = prCfaOgm->rCfaOgmRange.u8VidStartOfst;
		prCfaOgm->rCfaOgmRange.u8AudEndOfst = prCfaOgm->rCfaOgmRange.u8VidEndOfst;
		prCfaOgm->rCfaOgmRange.i8AudLastGranule = 0;
	}

	if (prCfaOgm->rCfaOgmRange.fgVidRangeEn == FALSE) {
		prCfaOgm->rCfaOgmRange.u8VidStartOfst = prCfaOgm->rCfaOgmRange.u8AudStartOfst;
		prCfaOgm->rCfaOgmRange.u8VidEndOfst = prCfaOgm->rCfaOgmRange.u8AudEndOfst;
		prCfaOgm->rCfaOgmRange.i8VidLastGranule = 0;
	}

	prCfaOgm->rCfaOgmRange.u8AudRangeOfst = prCfaOgm->rCfaOgmRange.u8AudStartOfst;
	if ((prCfaOgm->rCfaOgmRange.u8VidStartOfst == prCfaOgm->u4FirstVidStartOfst)
	    && (0 == prCfaOgm->rCfaOgmRange.u8SeekTime)) {
		prCfaOgm->rCfaOgmRange.u8AudStartOfst = prCfaOgm->u4FirstAudStartOfst;
	}

	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
		TEXT("[CFA_OGM] Set Range Done!!\n"));
	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM]Video Last Granule: 0x%llx\n"),
		    prCfaOgm->rCfaOgmRange.i8VidLastGranule);
	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM]Video Start Offset: 0x%llx\n"),
		    prCfaOgm->rCfaOgmRange.u8VidStartOfst);
	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM]Video Packet Start no: %d\n"),
		    prCfaOgm->rCfaOgmRange.uVidPacketStartNo);
	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM]Audio Last Granule: 0x%llx\n"),
		    prCfaOgm->rCfaOgmRange.i8AudLastGranule);
	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM]Audio Start Offset: 0x%llx\n"),
		    prCfaOgm->rCfaOgmRange.u8AudStartOfst);
	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM]Audio Packet Start no: %d\n"),
		    prCfaOgm->rCfaOgmRange.uAudPacketStartNo);

	MM_RETURN(RET_DMX_OK);

}


static void CfaOgmFreeMemoryforAud(CfaOgmInst *prCfaOgm, u32 u4AudIdx)
{
	u32 i = 0;

	for (i = 0; i < u4AudIdx; i++) {
		if (NULL !=
			prCfaOgm->arCfaOgmAudStream[i].pu1VorbisHeaderData) {
			DMX_FreeHwMemory(prCfaOgm->arCfaOgmAudStream[i].pu1VorbisHeaderData);
			prCfaOgm->arCfaOgmAudStream[i].pu1VorbisHeaderData = NULL;
		}
	}

}

/*-----------------------------------------------------------------------------
 * Name: CfaOgmConfigure
 *
 * Description:
 *      splitter will ensure that it is only called in "off" or "paused" state.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaOgmConfigure(void *pvSptHdl, void *pvParam, void *pvPrivData, bool fgIsUserMem)
{
	CfaOgmConfigInfo *prCfaOgmConfigInfo = NULL;
	CfaOgmConfigInfo rCfaOgmConfigInfo;
	CfaOgmInst *prCfaOgm = NULL;
	u32 u4Idx = 0;

	if ((NULL == pvPrivData) || (NULL == pvParam))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	prCfaOgm = (CfaOgmInst *) pvPrivData;
	mm_memset(&rCfaOgmConfigInfo, 0, sizeof(rCfaOgmConfigInfo));

	if (fgIsUserMem) {
		if (0 != mm_copy_from_user(&rCfaOgmConfigInfo,
			pvParam, sizeof(CfaOgmConfigInfo))) {
			DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] %s line %d failed in mm_copy_from_user\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
		}
	} else {
		mm_memcpy(&rCfaOgmConfigInfo,
					pvParam, sizeof(CfaOgmConfigInfo));
	}
	

	prCfaOgmConfigInfo = &rCfaOgmConfigInfo;

	MMATE_CHECK_POINTER(prCfaOgm);
	MMATE_CHECK_STRUCT(prCfaOgm->rCurPacket);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmRange);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmAu);
	MMATE_CHECK_STRUCT(prCfaOgm->rCfaOgmVidStream);

	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
		TEXT("[CFA_OGM] CfaOgmConfigure,prCfaOgm->fgUseCMDQ:%d!!!\n"),
		prCfaOgm->fgUseCMDQ);

	/*video configuration */
	prCfaOgm->rCfaOgmVidStream.eCfaVidCodec =
	    CfaOgmGetVidType(prCfaOgmConfigInfo->rCfaOgmVidInfo.eCfaVidCodec);
	prCfaOgm->rCfaOgmVidStream.u4VidStreamNo = prCfaOgmConfigInfo->rCfaOgmVidInfo.u4VidStreamNo;
	prCfaOgm->rCfaOgmVidStream.u8FramePerUnit =
	    prCfaOgmConfigInfo->rCfaOgmVidInfo.u8FramePerUnit;
	prCfaOgm->rCfaOgmVidStream.u8TimeUnit = prCfaOgmConfigInfo->rCfaOgmVidInfo.u8TimeUnit;

	/*audio configuration */
	if(prCfaOgmConfigInfo->uAudioStreamNs > MAX_NS_OGM_AUD) {
		DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM]%s,line %d,prCfaOgmConfigInfo->uAudioStreamNs(%d) > MAX_NS_OGM_AUD(%d)!\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prCfaOgmConfigInfo->uAudioStreamNs, MAX_NS_OGM_AUD);
		prCfaOgmConfigInfo->uAudioStreamNs = MAX_NS_OGM_AUD;
	}

	for (u4Idx = 0; u4Idx < (prCfaOgmConfigInfo->uAudioStreamNs); u4Idx++) {
		prCfaOgm->arCfaOgmAudStream[u4Idx].eCfaAudCodec =
		    CfaOgmGetAudType(prCfaOgmConfigInfo->arCfaOgmAudInfo[u4Idx].eCfaAudCodec);
		prCfaOgm->arCfaOgmAudStream[u4Idx].u4AudStreamNo =
		    prCfaOgmConfigInfo->arCfaOgmAudInfo[u4Idx].u4AudStreamNo;
		if ((prCfaOgm->arCfaOgmAudStream[u4Idx].eCfaAudCodec == CFA_AUD_DRV_FMT_VORBIS)
		    || (prCfaOgm->arCfaOgmAudStream[u4Idx].eCfaAudCodec == CFA_AUD_DRV_FMT_FLAC)) {
			prCfaOgm->arCfaOgmAudStream[u4Idx].u4AudSampRate =
			    prCfaOgmConfigInfo->arCfaOgmAudInfo[u4Idx].rAudSample.u4AudSampRate;
		} else {
			prCfaOgm->arCfaOgmAudStream[u4Idx].u8TimeUnit =
			    prCfaOgmConfigInfo->arCfaOgmAudInfo[u4Idx].rAudSample.rCfaOgmAudTime.
			    u8TimeUnit;
			prCfaOgm->arCfaOgmAudStream[u4Idx].u8SamplePerUnit =
			    prCfaOgmConfigInfo->arCfaOgmAudInfo[u4Idx].rAudSample.rCfaOgmAudTime.
			    u8SamplePerUnit;
		}

		if ((prCfaOgm->arCfaOgmAudStream[u4Idx].eCfaAudCodec == CFA_AUD_DRV_FMT_VORBIS)
		    && (NULL == prCfaOgm->arCfaOgmAudStream[u4Idx].pu1VorbisHeaderData)
		    && (NULL != prCfaOgmConfigInfo->arCfaOgmAudInfo[u4Idx].puHeaderData)) {
			u32 u4VorbisHeaderPageSize =
			    prCfaOgmConfigInfo->arCfaOgmAudInfo[u4Idx].u4HeaderDataSize;

			DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					TEXT("[CFA_OGM] u4VorbisHeaderPageSize %d\n"),
				    u4VorbisHeaderPageSize);
			DMX_NewHwMemory(u4VorbisHeaderPageSize,
					prCfaOgm->arCfaOgmAudStream[u4Idx].pu1VorbisHeaderData);
			if (NULL == prCfaOgm->arCfaOgmAudStream[u4Idx].pu1VorbisHeaderData) {

				DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
					    TEXT("[CFA_OGM] Alloc prCfaOgm->arCfaOgmAudStream[u4Idx]")
					    TEXT(".pu1VorbisHeaderData memory fail\n"));
				if (!fgIsUserMem)
					DMX_FreeMemory(pvParam);
				MM_RETURN(RET_DMX_NO_MEM);
			}

			if (!fgIsUserMem) {
				dmx_memcpy(prCfaOgm->arCfaOgmAudStream[u4Idx].pu1VorbisHeaderData,
						  prCfaOgmConfigInfo->arCfaOgmAudInfo[u4Idx].puHeaderData,
						  prCfaOgmConfigInfo->arCfaOgmAudInfo[u4Idx].
						  u4HeaderDataSize);
			} else {
				mm_copy_from_user(prCfaOgm->arCfaOgmAudStream[u4Idx].pu1VorbisHeaderData,
						  prCfaOgmConfigInfo->arCfaOgmAudInfo[u4Idx].puHeaderData,
						  prCfaOgmConfigInfo->arCfaOgmAudInfo[u4Idx].
						  u4HeaderDataSize);
			}

			prCfaOgm->arCfaOgmAudStream[u4Idx].u4VorbisHeaderSize =
			    u4VorbisHeaderPageSize;

			DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				    TEXT
				    ("[CFA_OGM] prCfaOgm->arCfaOgmAudStream[%d].u4VorbisHeaderSize: %d\n"),
				    u4Idx, prCfaOgm->arCfaOgmAudStream[u4Idx].u4VorbisHeaderSize);
		}
#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT

		/*initial the parameter of AAC */
		if (CFA_AUD_DRV_FMT_AAC == prCfaOgm->arCfaOgmAudStream[u4Idx].eCfaAudCodec) {
			if (NULL == prCfaOgm->rCfaOgmAACPacket.pu1AACPacketData) {

				DMX_NewHwMemory(sizeof(u8) * CFA_OGM_AAC_PACKET_MAX_LEN,
						prCfaOgm->rCfaOgmAACPacket.pu1AACPacketData);
				if (NULL == prCfaOgm->rCfaOgmAACPacket.pu1AACPacketData) {
					CfaOgmFreeMemoryforAud(prCfaOgm, u4Idx);
					DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
						    TEXT("[CFA_OGM] Alloc prCfaOgm->rCfaOgmAACPacket.")
						    TEXT("pu1AACPacketData memory fail\n"));
					if (!fgIsUserMem)
						DMX_FreeMemory(pvParam);
					MM_RETURN(RET_DMX_NO_MEM);
				}

				dmx_memset((u8 *) prCfaOgm->rCfaOgmAACPacket.pu1AACPacketData, 0,
					   sizeof(u8) * (CFA_OGM_AAC_PACKET_MAX_LEN));
			}

			prCfaOgm->rCfaOgmAACPacket.fgTxAACStrmHdr = FALSE;
			prCfaOgm->rCfaOgmAACPacket.fgOnlyTxAACHeader = FALSE;
			prCfaOgm->rCfaOgmAACPacket.fgOnlyTxAACData = FALSE;
			prCfaOgm->arCfaOgmAudStream[u4Idx].uChannel =
			    prCfaOgmConfigInfo->arCfaOgmAudInfo[u4Idx].rCfaOgmAacInfo.uChannles;
			prCfaOgm->arCfaOgmAudStream[u4Idx].uAacHeaderLen =
			    prCfaOgmConfigInfo->arCfaOgmAudInfo[u4Idx].rCfaOgmAacInfo.uAacHeaderLen;

			if (prCfaOgm->arCfaOgmAudStream[u4Idx].uAacHeaderLen >
			    CFA_OGM_MAX_AAC_HEADER_LEN) {
				prCfaOgm->arCfaOgmAudStream[u4Idx].uAacHeaderLen =
				    CFA_OGM_MAX_AAC_HEADER_LEN;
			}

			dmx_memcpy(prCfaOgm->arCfaOgmAudStream[u4Idx].auAacHeader,
					   prCfaOgmConfigInfo->arCfaOgmAudInfo[u4Idx].rCfaOgmAacInfo.auAacHeader,
					   sizeof(u8) * prCfaOgm->arCfaOgmAudStream[u4Idx].uAacHeaderLen /*9 */);
			

		}
		prCfaOgm->arCfaOgmAudStream[u4Idx].auPcmHeader = NULL;
		prCfaOgm->arCfaOgmAudStream[u4Idx].uPcmHeaderLen = 0;
#endif //CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT

	}
	prCfaOgm->u4AudNum = prCfaOgmConfigInfo->uAudioStreamNs;
	prCfaOgm->rCfaOgmVidStream.u8DefaultFrame = 1;
	prCfaOgm->u4DurationMs = prCfaOgmConfigInfo->u4DurationMs;

#if	ENABLE_DMX_ADVANCED_VER
	if (prCfaOgm->u4AudNum > 1) {
		prCfaOgm->fgUseCMDQ = FALSE;
	} else {
		prCfaOgm->fgUseCMDQ = TRUE;
	}
#else
	prCfaOgm->fgUseCMDQ = FALSE;
#endif

	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM] Get Configure Done!\n"));
	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM]Video Stream No: 0x%lx\n"),
		    prCfaOgm->rCfaOgmVidStream.u4VidStreamNo);
	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM]Video frame per unit: 0x%llx\n"),
		    prCfaOgmConfigInfo->rCfaOgmVidInfo.u8FramePerUnit);
	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM]Video time unit: 0x%llx\n"),
		    prCfaOgm->rCfaOgmVidStream.u8TimeUnit);
	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM]Video codec: %d\n"),
		    prCfaOgm->rCfaOgmVidStream.eCfaVidCodec);
	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM]Audio numbers: %d, fgUseCMDQ:%d.\n"), prCfaOgm->u4AudNum, prCfaOgm->fgUseCMDQ);
	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
			TEXT("[CFA_OGM]1st Audio codec: %d\n"),
		    prCfaOgm->arCfaOgmAudStream[0].eCfaAudCodec);

	if (!fgIsUserMem)
		DMX_FreeMemory(pvParam);

	MM_RETURN(RET_DMX_OK);


}



/*-----------------------------------------------------------------------------
 * Name: CfaOgmUninit
 *
 * Description:
 *      Uninit CFA OGM
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaOgmUninit(void *pvSptHdl, void *pvCfaPrivData)
{
	void *pvPointer = NULL;
	u32 u4Idx = 0;
	CfaOgmInst *prCfaOgm = NULL;

	if (NULL == pvCfaPrivData)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	prCfaOgm = (CfaOgmInst *) pvCfaPrivData;
	MMATE_CHECK_POINTER(prCfaOgm);

#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT

	if (NULL != prCfaOgm->rCfaOgmAACPacket.pu1AACPacketData) {
		/* DMX_FreeMemory(pvSptHdl, prCfaOgm->rCfaOgmAACPacket.pu1AACPacketData);*/
		DMX_FreeHwMemory(prCfaOgm->rCfaOgmAACPacket.pu1AACPacketData);

	}
	prCfaOgm->rCfaOgmAACPacket.fgTxAACStrmHdr = FALSE;
	prCfaOgm->rCfaOgmAACPacket.fgOnlyTxAACHeader = FALSE;
	prCfaOgm->rCfaOgmAACPacket.fgOnlyTxAACData = FALSE;

	for (u4Idx = 0; u4Idx < (prCfaOgm->u4AudNum); u4Idx++) {
		if (NULL != prCfaOgm->arCfaOgmAudStream[u4Idx].auPcmHeader) {
			DMX_FreeHwMemory((void *) prCfaOgm->arCfaOgmAudStream[u4Idx].auPcmHeader);
			prCfaOgm->arCfaOgmAudStream[u4Idx].auPcmHeader = NULL;
			prCfaOgm->arCfaOgmAudStream[u4Idx].uPcmHeaderLen = 0;
		}

		if (NULL != prCfaOgm->arCfaOgmAudStream[u4Idx].pu1VorbisHeaderData) {
			DMX_FreeHwMemory(prCfaOgm->arCfaOgmAudStream[u4Idx].pu1VorbisHeaderData);
			prCfaOgm->arCfaOgmAudStream[u4Idx].pu1VorbisHeaderData = NULL;
		}
	}
#endif

	pvPointer = (void *) prCfaOgm;
	DMX_FreeMemory(pvPointer);

	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
		TEXT("[CFA_OGM] Uninit!\n"));

	MM_RETURN(RET_DMX_OK);
}




/*-----------------------------------------------------------------------------
 * Name: CfaOgmSetInqTypes
 *
 * Description:
 *      OGM CFA sets information query types
 *      splitter will ensure that it is only called in "off" or "paused" state.
 *      now unused.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaOgmSetInqTypes(void *pvSptHdl, u32 u4InfTypes, void *pvPrivData)
{
	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
		TEXT("[CFA_OGM] vCfaOgmSetInqTypes\n"));


	/*do nothing */

	MM_RETURN(RET_DMX_OK);
}




/*-----------------------------------------------------------------------------
 * Name: CfaOgmGetCurPos
 *
 * Description:
 *      OGM CFA callback for when FMPC needs to know CFA's current position.But now unused.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaOgmGetCurPos(void *pvSptHdl, void *pvCurPos, void *pvPrivData)
{
	CfaOgmInst *prCfaOgm = NULL;
	u64 *pvu8 = NULL;

	if (NULL == pvCurPos)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
		TEXT("[CFA_OGM] vCfaOgmGetCurPos\n"));
	if (NULL == pvPrivData)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	pvu8 = (u64 *) pvCurPos;

	prCfaOgm = (CfaOgmInst *) pvPrivData;
	*pvu8 = prCfaOgm->u8Ca;

	MM_RETURN(RET_DMX_OK);
}





/*-----------------------------------------------------------------------------
 * Name: CfaOgmGetGeneral
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
static MRESULT CfaOgmGetGeneral(void *pvSptHdl, u32 u4CfaFID, void *pvPrivData,
				void *pvCfaParameter, u32 u4CfaParameterSize)
{

	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
		TEXT("[CFA_OGM] i4CfaOgmGetGeneral\n"));

	/*do nothing */

	MM_RETURN(RET_DMX_OK);
}


#if CONFIG_CFA_OGM_NEW_SYNCBUF
/*-----------------------------------------------------------------------------
 * Name: CfaOgmRebuf
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
static MRESULT CfaOgmRebuf(void *pvSptHdl, bool fgRebuf, void *pvPrivData)
{
	CfaOgmInst *prCfaOgm = NULL;

	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
		TEXT("[CFA_OGM] Spt notify cfa rebuf!\n"));

	if (NULL == pvPrivData)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	prCfaOgm = (CfaOgmInst *) pvPrivData;
	if (fgRebuf)
		prCfaOgm->fgIfRebuf = FALSE;

	MM_RETURN(RET_DMX_OK);
}

#endif

#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
/*-----------------------------------------------------------------------------
 * Name: CfaOgmTxAudHDRInfo
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

static MRESULT CfaOgmTxAudHDRInfo(void *pvSptHdl, u32 u4TxUID, void *pvPrivData)
{
	CfaOgmInst *prCfaOgm = NULL;

	if (NULL == pvPrivData)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
		TEXT("[CFA_OGM] TxAudHDRInfo!\n"));
	prCfaOgm = (CfaOgmInst *) pvPrivData;
	MMATE_CHECK_POINTER(prCfaOgm);

	if ((DMX_INVALID_UINT32 == u4TxUID)
	    && (CFA_AUD_DRV_FMT_AAC ==
		prCfaOgm->arCfaOgmAudStream[prCfaOgm->uCurAIndex].eCfaAudCodec)) {
		prCfaOgm->rCfaOgmAACPacket.fgTxAACStrmHdr = FALSE;
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (prCfaOgm->u4CurAId != u4TxUID)
		MM_RETURN(RET_DMX_UNSUPPORT);

	if (CFA_AUD_DRV_FMT_AAC == prCfaOgm->arCfaOgmAudStream[prCfaOgm->uCurAIndex].eCfaAudCodec) {
		MRESULT mrRet = RET_DMX_OK;

		prCfaOgm->eCfaOgmCurPrsStrm = CFA_OGM_PRS_STRM_TYPE_NONE;
		prCfaOgm->rCfaOgmAACPacket.fgOnlyTxAACHeader = TRUE;
		prCfaOgm->rCfaOgmAACPacket.fgTxAACStrmHdr = TRUE;

		mrRet =
		    Spt4CfaBuf2AFifo(pvSptHdl,
				prCfaOgm->arCfaOgmAudStream[prCfaOgm->uCurAIndex].auAacHeader,
				prCfaOgm->arCfaOgmAudStream[prCfaOgm->uCurAIndex].
					uAacHeaderLen, u4TxUID,
					prCfaOgm->arCfaOgmAudStream[prCfaOgm->uCurAIndex].
					eCfaAudCodec);
		if (RET_DMX_OK != mrRet) {
			DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] Spt4CfaBuf2AFifo error ret = %d\n"),
				mrRet);
		}
		MM_RETURN(mrRet);

	}

	if (CFA_AUD_DRV_FMT_PCM == prCfaOgm->arCfaOgmAudStream[prCfaOgm->uCurAIndex].eCfaAudCodec) {
		MRESULT mrRet = RET_DMX_OK;

		mrRet =
			Spt4CfaBuf2AFifo(pvSptHdl,
					prCfaOgm->arCfaOgmAudStream[prCfaOgm->uCurAIndex].auPcmHeader,
					prCfaOgm->arCfaOgmAudStream[prCfaOgm->uCurAIndex].
					uPcmHeaderLen, u4TxUID,
					prCfaOgm->arCfaOgmAudStream[prCfaOgm->uCurAIndex].
					eCfaAudCodec);
		if (RET_DMX_OK != mrRet) {
			DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] Spt4CfaBuf2AFifo error ret = %d\n"), mrRet);
		}
		MM_RETURN(mrRet);
	}

	if(CFA_AUD_DRV_FMT_VORBIS == prCfaOgm->arCfaOgmAudStream[prCfaOgm->uCurAIndex].eCfaAudCodec) {
		MRESULT mrRet = RET_DMX_OK;
		mrRet = Spt4CfaBuf2AFifo(pvSptHdl, prCfaOgm->arCfaOgmAudStream[prCfaOgm->uCurAIndex].pu1VorbisHeaderData,
		prCfaOgm->arCfaOgmAudStream[prCfaOgm->uCurAIndex].u4VorbisHeaderSize,
		prCfaOgm->u4CurAId,
		prCfaOgm->arCfaOgmAudStream[prCfaOgm->uCurAIndex].eCfaAudCodec);
		if(RET_DMX_OK != mrRet) {
			DmxLogE(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("[CFA_OGM] Spt4CfaBuf2AFifo error ret = %d \n"), mrRet);
		}
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_UNSUPPORT);
}

#endif

static MRESULT CfaOgmSetJumpRange(void *pvSptHdl, void *pvJmpRange, void *pvPrivData)
{
	MRESULT mrResult = RET_DMX_OK;
	CfaOgmKeyRange *pKeyRange = NULL;
	CfaOgmInst *prCfaOgm = NULL;

	prCfaOgm = (CfaOgmInst *) pvPrivData;

	if ((NULL == pvPrivData) || (NULL == pvJmpRange))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	pKeyRange = (CfaOgmKeyRange *) pvJmpRange;
	prCfaOgm = (CfaOgmInst *) pvPrivData;
	MMATE_CHECK_POINTER(prCfaOgm);

	prCfaOgm->u4FRAudAuLen = pKeyRange->u4FRAudAuLen;

	mrResult = CfaOgmSetRange(pvSptHdl, (void *) (&pKeyRange->rCfaOmgRane), pvPrivData, TRUE);

	MM_RETURN(mrResult);
}

static MRESULT CfaOgmGetParamSize(void *pvSptHdl, u32 u4ParamID,
				  void *pvPrivData, void *pvCfaParam, u32 u4CfaParamSz)
{
	MRESULT mrResult = RET_DMX_OK;

	DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
		TEXT("[CFA_OGM] Entry CfaOgmGetParamSize!\r\n"));

	switch (u4ParamID) {
	case CFA_PARAM_ID_JUMP_INFO_SIZE:
		{
			if ((NULL == pvCfaParam) || ((u4CfaParamSz) < sizeof(u32))) {
				mrResult = RET_DMX_PARAM_WRONG;
			} else {
				u32 *pu4Tmp = (u32 *) pvCfaParam;
				*pu4Tmp = sizeof(CfaOgmKeyRange);
			}
			break;
		}

	default:
		mrResult = RET_DMX_PARAM_WRONG;
		break;
	}

	MM_RETURN(mrResult);
}

static MRESULT CfaOgmProcCliCmd(void *pvSptHdl, E_DMX_CFA_CLI_TYPE_T eCliType, /*< [IN] Cfa Cli Command*/
				u32 arg1,
				u32 arg2, u32 arg3, const char *szParam, void *pvPrivData)
{
	MRESULT mrRet = RET_DMX_OK;
	CfaOgmInst *prCfaOgm = NULL;

	prCfaOgm = (CfaOgmInst *) pvPrivData;

	switch (eCliType) {
	case DMX_CFA_CLI_CMD_TURN_ONOFF_LOG:
		{
			bool fgEnable = TRUE;
		/**
		* arg1: u4OnOff
		* arg2: LogLevel(T, E, W, D)
		* arg3: Module Log Level
		**/
			if (0 == arg1)
				fgEnable = FALSE;

			DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("CfaOgmProcCliCmd -- fgEnable: %d, Loglvl: %d, ModLogLvl: 0x%08x \r\n"),
				arg1, arg2, arg3);

			DmxLogEnable(fgEnable, arg2, DMX_MOD_CFA_OGM, arg3);
		}
		break;
	case DMX_CFA_CLI_CMD_DUMP_INFO:
		{
			DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("Cfa Ogm Instance(handle is 0x%x)")
				TEXT(" Info list as follow: \r\n"),
				prCfaOgm);
			DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("Current Analyse State is %d, ")
				TEXT("Current Parsing stream type is %d \r\n"),
				prCfaOgm->eCfaOgmCurState, prCfaOgm->eCfaOgmCurPrsStrm);
			DmxLogT(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
				TEXT("Current Analyse Position is 0x%08x%08x\r\n"),
				(u32) ((prCfaOgm->u8Ca) >> 32), (u32) (prCfaOgm->u8Ca));
		}
		break;
	default:
		break;
	}

	MM_RETURN(mrRet);
}

/* OGM CFA interface */
CfaIntf _rOgmCfaIntf = {
	&CfaOgmInit,
	&CfaOgmUninit,
	&CfaOgmSetRange,
	&CfaOgmEnableStrm,
	&CfaOgmSetStrmInf,
	&CfaOgmTurnOn,
	&CfaOgmTxDone,
	&CfaOgmGetCurPos,
	NULL,
	&CfaOgmConfigure,
	&CfaOgmSetInqTypes,
	&CfaOgmGetGeneral,
	NULL,
	NULL,
	&CfaOgmFillAUInfo,

#if CONFIG_CFA_OGM_AAC_AUDIO_SUPPORT
	&CfaOgmTxAudHDRInfo
#else
	NULL
#endif
#if CONFIG_CFA_OGM_NEW_SYNCBUF
	    , &CfaOgmRebuf
#endif
	, &CfaOgmSetJumpRange, &CfaOgmGetParamSize,
	&CfaOgmProcCliCmd
	#ifdef CONFIG_COMPAT
	, &CfaOgmProcCompat
	#endif
};



/*-----------------------------------------------------------------------------
 * Name: vCfaOgmInitPara
 *
 * Description:
 *      Init CFA OGM internal parameters
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: None
 *
 *-----------------------------------------------------------------------------*/
void CfaOgmInitPara(CfaOgmInst *prCfaOgm)
{
	u8 ucLoop = 0;

	prCfaOgm->eCfaOgmCurState = CFA_OGM_ST_IDLE;
	prCfaOgm->eCfaOgmNextState = CFA_OGM_ST_IDLE;
	prCfaOgm->u8Ca = DMX_INVALID_UINT64;
	prCfaOgm->fgCrossSlot = FALSE;

	for (ucLoop = 0; ucLoop < (u8)MAX_NS_OGM_AUD; ucLoop++)
		prCfaOgm->arCfaOgmAudStream[ucLoop].fgFirst = FALSE;

	dmx_memset((VOID * )(&(prCfaOgm->rCurPacket)),0,sizeof(prCfaOgm->rCurPacket));
	dmx_memset((VOID *)(&(prCfaOgm->rCurPage)),0,sizeof(prCfaOgm->rCurPage));

	dmx_memset((VOID * )(&(prCfaOgm->rRspPacket)),0,sizeof(prCfaOgm->rRspPacket));
	dmx_memset((VOID *)(&(prCfaOgm->rRspPage)),0,sizeof(prCfaOgm->rRspPage));
	
	dmx_memset((VOID *)(&prCfaOgm->rVidCmdQInfo),0,sizeof(prCfaOgm->rVidCmdQInfo));
	dmx_memset((VOID *)(&prCfaOgm->rAudCmdQInfo),0,sizeof(prCfaOgm->rAudCmdQInfo));
	dmx_memset((VOID *)(prCfaOgm->arAudCmdQInfo),0,sizeof(prCfaOgm->arAudCmdQInfo[0])*MAX_NS_OGM_AUD);
	dmx_memset((VOID *)(prCfaOgm->arCmdQStartPage),0,sizeof(prCfaOgm->arCmdQStartPage[0])*MAX_NS_OGM_AUD);
	dmx_memset((VOID *)(prCfaOgm->arCmdQStartPacket),0,sizeof(prCfaOgm->arCmdQStartPacket[0])*MAX_NS_OGM_AUD);
	dmx_memset((VOID *)(&prCfaOgm->rCmdQStartPage),0,sizeof(prCfaOgm->rCmdQStartPage));
	dmx_memset((VOID *)(&prCfaOgm->rCmdQStartPacket),0,sizeof(prCfaOgm->rCmdQStartPacket));
	dmx_memset((VOID *)(&prCfaOgm->rVidCmdQStartPage),0,sizeof(prCfaOgm->rVidCmdQStartPage));
	dmx_memset((VOID *)(&prCfaOgm->rVidCmdQStartPacket),0,sizeof(prCfaOgm->rVidCmdQStartPacket));
	dmx_memset((VOID *)(prCfaOgm->au8TxAudCmdQIndex),0,sizeof(UINT8)*MAX_NS_OGM_AUD);
	dmx_memset((VOID *)(prCfaOgm->arOgmInstCmdEntrys),0,sizeof(DMX_CMDQ_TX_ENTRY_T) * DMX_MAX_TX_CNT_FOR_CMD_Q);
	prCfaOgm->fgCrossSlot = FALSE;
	prCfaOgm->uTxAudCmdQNs = 0;
	prCfaOgm->uCurCmdQIdx = 0;
	prCfaOgm->eTmpStateWhenCmdQ = CFA_OGM_ST_IDLE;
	prCfaOgm->fgHasTxVidCmdQ = FALSE;
	prCfaOgm->u8PrePTSForComposeAU = 0;


	prCfaOgm->fgIfNotifyPTS = FALSE;
	prCfaOgm->fgIfRebuf = FALSE;
	prCfaOgm->i4CallTimes = 0;
	prCfaOgm->u4FRAudDataTxLen = 0;
	prCfaOgm->u8PreAudPts = DMX_INVALID_UINT64;
	prCfaOgm->u4CallFillAuCnt = 0;
	prCfaOgm->fgRealSyncPb = FALSE;
	prCfaOgm->fgNoNeedSyncPb = FALSE;

	prCfaOgm->u8RspCa = 0;
	prCfaOgm->u8RspTxLen = 0;
	prCfaOgm->eRspState = CFA_OGM_ST_IDLE;
	prCfaOgm->u8PageOfst = 0;

	for(ucLoop = 0; ucLoop < (u8)MAX_NS_OGM_AUD; ucLoop++) {
		prCfaOgm->au8AudParsedOfst[ucLoop] = 0;
	}
	prCfaOgm->u8VidParsedOfst = 0;

	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
		TEXT("[CFA_OGM] Init internal parameters!\n"));

}


/*-----------------------------------------------------------------------------
 * Name: pvCfaOgmGetInterface
 *
 * Description:
 *      Start of Public Function
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
void *CfaOgmGetInterface(void)
{
	DmxLogD(DMX_MOD_CFA_OGM, CFA_OGM_LOG_DEFAULT,
		TEXT("[CFA_OGM] Get Interface of CFA OGM!\n"));
	return (void *)(&_rOgmCfaIntf);
}
