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
#include <media/atc/drv_esm_if.h>
/* #include <media/atc/mm_debug.h> */

#include "dmx_def.h"
#include "dmx_mem.h"
#include "cfa_macro.h"
#include "cfa_asf_st_ctrl.h"
#include "cfa_asf.h"

/*#pragma warning(disable: 6011) //disable	warning C6011: Dereferencing NULL pointer checking*/
/*-----------------------------------------------------------------------------
						function prototype
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 * Name: CfaAsfInitPara
 *
 * Description:
 *		Init CFA ASF internal parameters
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: None
 *
 *-----------------------------------------------------------------------------*/
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
typedef struct {
	bool fgCfgRespliter;
	bool fgOnlyWma;
	bool fgIsNrd;
	__u32 u4DataPacketSize;
	__u64 u8FileSize;
	__u64 u8HeaderObjectSize;
	__u64 u8DataPacketCount;
	__u64 u8PrerollTime;
}  CfaAsfCfgFileInfo_T32;

typedef struct {
	CfaPayloadExtSysId_E ePayloadExtSysId;
	__u16 u2DataSize;
}  CfaAsfPayloadExtSysInfo_T32;

typedef struct {
	__u8 u1StrmNum;
	CfaStrmTypeInfo_E eStrmType;
	CfaAsfPayloadExtSysInfo_T32 arPayloadExtSysInfo[MAX_ASF_PL_EXT_SYS_ID];
}  CfaAsfCfgStrmInfo_T32;


typedef struct {
	CfaAsfCfgStrmInfo_T32 rCfaAsfStrmInfo;
	AVCODECID_T eCodecID;
	__u16 u2RawCodecID;
	__u16 u2ChannelNum;	/*the number of audio channel, monaural ,stereo, 5.1 audio */
	__u32 u4SamPS;	/*Samples per second */
	__u32 u4AveBytePS;	/* average number of bytes per second */
	__u16 u2BlockAlign;	/* the block alignment, or block size */
	__u16 u2BitPerSample;	/* the number of bits per sample of  monaural data */
	/* mtk40504 */
	__u32 u4AudCodecSpecDataLen;
	__u8 au1AudCodecSpecData[LP_ASF_CODEC_SPEC_DATA_MAX_LEN];
	bool fgVBR;
	bool fgNeedAdtsHeader;
	__u8 u1AacProfile;
}  CfaAsfCfgAudInfo_T32;

typedef struct {
	CfaAsfCfgStrmInfo_T32 rCfaAsfStrmInfo;
	AVCODECID_T eCodecID;
	VCODECVERSION_T eCodecVer;
	__u64 u8VidCodecSpecDataOft;
	__u32 u4VidCodecSpecDataLen;
	/* Modified by jie.tang */
	__u8 au1VidCodecSpecData[LP_ASF_CODEC_SPEC_DATA_MAX_LEN];
	CfaAsfQIVc1Mode_T rCfaAsfVc1Mode;	/* avaliable when vid codec is MVC1 */
}  CfaAsfCfgVidInfo_T32;

typedef struct {
	__u8 u1WMDRMType;	/* 0 : not wmdrm,  PD - 1, ND - 2 */
	/* Each bit presents whether the related stream is encrypted. LSB bit 0 represent strem 1 */
	__u8 au1StreamEncrypted[16]; //num = 128/8
	__u32 u4DecInfSize;	/* sizeof decryption info. For pd, it is WRMHEADER */
	compat_caddr_t pu1DecInf;	/* decryption info */
}  CfaAsfCfgDRMInfo_T32;

typedef struct {
	CfaAsfCfgFileInfo_T32 rCfaAsfCfgFileInfo;
	CfaAsfCfgAudInfo_T32 raCfaAsfCfgAudInfo[MAX_ASF_AUD_STRM_NUM];
	CfaAsfCfgVidInfo_T32 rCfaAsfCfgVidInfo;
	CfaAsfCfgDRMInfo_T32 rCfaAsfCfgDrmInfo;	/* wmdrm */
}  CfaAsfCfgInfo_T32;
 
typedef struct {
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKStart;
#endif
	bool fgEnableVid;
	bool fgEnableAud;
	bool fgVirFinish;

	__u64 u8VidSa;
	__u64 u8VidEa;
	__u64 u8AudSa;
	__u64 u8AudEa;

	__u64 u8DispPicPTS;
	CfaAsfSkipType_E eSkipMode;
	__u32 u4SkipPacketCount;
	__u32 u4CfaAudioAULen;
	__u64 u8AudMaxRestLen;
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKEnd;
#endif
}  CfaAsfRange_T32;

typedef struct {
	CfaAsfRange_T32 rCfaRangeInfo;
}  CfaAsfKeyFrameRange_T32;

static long CfaAsfCompatRange(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaAsfRange_T __user *usr_ptr = NULL;
	CfaAsfRange_T32 __user *usr_ptr32 = (CfaAsfRange_T32 __user *)prInfo->usr_ptr32;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaAsfRange_T32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	usr_ptr = (CfaAsfRange_T *)compat_alloc_user_space(sizeof(CfaAsfRange_T));

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaAsfRange_T));
	if (copy_in_user(&(usr_ptr->fgEnableVid), &(usr_ptr32->fgEnableVid), sizeof(bool)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->fgEnableAud), &(usr_ptr32->fgEnableAud), sizeof(bool)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->fgVirFinish), &(usr_ptr32->fgVirFinish), sizeof(bool)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->u8VidSa), &(usr_ptr32->u8VidSa), sizeof(__u64)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->u8VidEa), &(usr_ptr32->u8VidEa), sizeof(__u64)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->u8AudSa), &(usr_ptr32->u8AudSa), sizeof(__u64)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->u8AudEa), &(usr_ptr32->u8AudEa), sizeof(__u64)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->u8DispPicPTS), &(usr_ptr32->u8DispPicPTS), sizeof(__u64)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->eSkipMode), &(usr_ptr32->eSkipMode), sizeof(CfaAsfSkipType_E)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->u4SkipPacketCount), &(usr_ptr32->u4SkipPacketCount), sizeof(__u32)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->u4CfaAudioAULen), &(usr_ptr32->u4CfaAudioAULen), sizeof(__u32)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->u8AudMaxRestLen), &(usr_ptr32->u8AudMaxRestLen), sizeof(__u64)))
		return -EFAULT; 

	*pfgIsUserMem = TRUE;
	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaAsfRange_T);

	return 0;
	
}

static long CfaAsfCompatJumpRange(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaAsfKeyFrameRange_T __user *usr_ptr = NULL;
	CfaAsfKeyFrameRange_T32 __user *usr_ptr32 = (CfaAsfKeyFrameRange_T32 __user *)prInfo->usr_ptr32;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaAsfKeyFrameRange_T32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}

	usr_ptr = (CfaAsfKeyFrameRange_T *)compat_alloc_user_space(sizeof(CfaAsfKeyFrameRange_T));

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaAsfKeyFrameRange_T));

	if (copy_in_user(&(usr_ptr->rCfaRangeInfo.fgEnableVid), 
			&(usr_ptr32->rCfaRangeInfo.fgEnableVid), sizeof(bool)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->rCfaRangeInfo.fgEnableAud), 
			&(usr_ptr32->rCfaRangeInfo.fgEnableAud), sizeof(bool)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->rCfaRangeInfo.fgVirFinish), 
			&(usr_ptr32->rCfaRangeInfo.fgVirFinish), sizeof(bool)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->rCfaRangeInfo.u8VidSa), 
			&(usr_ptr32->rCfaRangeInfo.u8VidSa), sizeof(__u64)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->rCfaRangeInfo.u8VidEa), 
			&(usr_ptr32->rCfaRangeInfo.u8VidEa), sizeof(__u64)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->rCfaRangeInfo.u8AudSa), 
			&(usr_ptr32->rCfaRangeInfo.u8AudSa), sizeof(__u64)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->rCfaRangeInfo.u8AudEa), 
			&(usr_ptr32->rCfaRangeInfo.u8AudEa), sizeof(__u64)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->rCfaRangeInfo.u8DispPicPTS), 
			&(usr_ptr32->rCfaRangeInfo.u8DispPicPTS), sizeof(__u64)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->rCfaRangeInfo.eSkipMode), 
			&(usr_ptr32->rCfaRangeInfo.eSkipMode), sizeof(CfaAsfSkipType_E)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->rCfaRangeInfo.u4SkipPacketCount), 
			&(usr_ptr32->rCfaRangeInfo.u4SkipPacketCount), sizeof(__u32)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->rCfaRangeInfo.u4CfaAudioAULen), 
			&(usr_ptr32->rCfaRangeInfo.u4CfaAudioAULen), sizeof(__u32)))
		return -EFAULT; 
	if (copy_in_user(&(usr_ptr->rCfaRangeInfo.u8AudMaxRestLen), 
			&(usr_ptr32->rCfaRangeInfo.u8AudMaxRestLen), sizeof(__u64)))
		return -EFAULT; 

	*pfgIsUserMem = TRUE;
	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaAsfKeyFrameRange_T);

	return 0;
	
}
static long CfaAsfCompatCfgStmInfo(CfaAsfCfgStrmInfo_T __user *usr_ptr,
	CfaAsfCfgStrmInfo_T32 __user *usr_ptr32)
{
	__u32 i;
	
	if (copy_from_user(&(usr_ptr->u1StrmNum), &(usr_ptr32->u1StrmNum), sizeof(__u8)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->eStrmType), &(usr_ptr32->eStrmType), sizeof(CfaStrmTypeInfo_E)))
		return -EFAULT;
	for (i = 0; i < MAX_ASF_PL_EXT_SYS_ID; i++) {
		if (copy_from_user(&(usr_ptr->arPayloadExtSysInfo[i].ePayloadExtSysId), 
				&(usr_ptr32->arPayloadExtSysInfo[i].ePayloadExtSysId), sizeof(CfaPayloadExtSysId_E)))
			return -EFAULT;
		if (copy_from_user(&(usr_ptr->arPayloadExtSysInfo[i].u2DataSize), 
				&(usr_ptr32->arPayloadExtSysInfo[i].u2DataSize), sizeof(__u16)))
			return -EFAULT;
	}

	return 0;
}

static long CfaAsfCompatCfgFileInfo(CfaAsfCfgFileInfo_T __user *usr_ptr,
	CfaAsfCfgFileInfo_T32 __user *usr_ptr32)
{
	if (copy_from_user(&(usr_ptr->fgCfgRespliter), &(usr_ptr32->fgCfgRespliter), sizeof(bool)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->fgOnlyWma), &(usr_ptr32->fgOnlyWma), sizeof(bool)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->fgIsNrd), &(usr_ptr32->fgIsNrd), sizeof(bool)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4DataPacketSize), &(usr_ptr32->u4DataPacketSize), sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8FileSize), &(usr_ptr32->u8FileSize), sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8HeaderObjectSize), &(usr_ptr32->u8HeaderObjectSize), sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8DataPacketCount), &(usr_ptr32->u8DataPacketCount), sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8PrerollTime), &(usr_ptr32->u8PrerollTime), sizeof(__u64)))
		return -EFAULT;

	return 0;
}

static long CfaAsfCompatCfgDrmInfo(CfaAsfCfgDRMInfo_T __user *usr_ptr,
	CfaAsfCfgDRMInfo_T32 __user *usr_ptr32)
{
	compat_caddr_t compatDefInf = 0;
	__u8 __user *dec_inf = NULL;

	if (copy_from_user(&(usr_ptr->u1WMDRMType), &(usr_ptr32->u1WMDRMType), sizeof(__u8)))
		return -EFAULT;
	if (copy_from_user((usr_ptr->au1StreamEncrypted), (usr_ptr32->au1StreamEncrypted), sizeof(__u8) * 16))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4DecInfSize), &(usr_ptr32->u4DecInfSize), sizeof(__u32)))
		return -EFAULT;
#if 0
	if ((0 != usr_ptr32->pu1DecInf) &&
			(0 < usr_ptr32->u4DecInfSize)) {
		usr_ptr->pu1DecInf =  (__force __u8 *)compat_alloc_user_space(
									sizeof(__u8) * usr_ptr->u4DecInfSize);

		if (NULL == usr_ptr->pu1DecInf) {
			DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
				TEXT("%s line %d fail in alloc compat user space.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return -ENOMEM;
		}
		mm_memset(usr_ptr->pu1DecInf, 0, sizeof(__u8) * usr_ptr->u4DecInfSize);

		if (get_user(compatDefInf, &(usr_ptr32->pu1DecInf)))
			return -EFAULT;
		if (0 == compatDefInf)
			return -EFAULT;
		dec_inf = compat_ptr(compatDefInf);
		if (!access_ok(VERIFY_READ, dec_inf, 
			sizeof(__u8) * usr_ptr->u4DecInfSize))
			return -EFAULT;

		if (copy_in_user((__u8 __user *)usr_ptr->pu1DecInf,
			dec_inf, sizeof(__u8) * usr_ptr->u4DecInfSize))
			return -EFAULT;
	}
#endif
	return 0;
}


static long CfaAsfCompatConfigAudInfo(CfaAsfCfgAudInfo_T __user *usr_ptr,
	CfaAsfCfgAudInfo_T32 __user *usr_ptr32)
{
	long ret = 0;

	if (copy_from_user(&(usr_ptr->eCodecID), &(usr_ptr32->eCodecID), sizeof(AVCODECID_T)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u2RawCodecID), &(usr_ptr32->u2RawCodecID), sizeof(__u16)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u2ChannelNum), &(usr_ptr32->u2ChannelNum), sizeof(__u16)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4SamPS), &(usr_ptr32->u4SamPS), sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4AveBytePS), &(usr_ptr32->u4AveBytePS), sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u2BlockAlign), &(usr_ptr32->u2BlockAlign), sizeof(__u16)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u2BitPerSample), &(usr_ptr32->u2BitPerSample), sizeof(__u16)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4AudCodecSpecDataLen), &(usr_ptr32->u4AudCodecSpecDataLen), sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->fgVBR), &(usr_ptr32->fgVBR), sizeof(bool)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->fgNeedAdtsHeader), &(usr_ptr32->fgNeedAdtsHeader), sizeof(bool)))
		return -EFAULT;
	if (copy_from_user((usr_ptr->au1AudCodecSpecData), (usr_ptr32->au1AudCodecSpecData), 
			sizeof(__u8) * LP_ASF_CODEC_SPEC_DATA_MAX_LEN))
		return -EFAULT;

	ret = CfaAsfCompatCfgStmInfo(&(usr_ptr->rCfaAsfStrmInfo), &(usr_ptr32->rCfaAsfStrmInfo));
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaAsfCompatConfigAudInfo.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}
	return 0;
		
}

static long CfaAsfCompatConfigVidInfo(CfaAsfCfgVidInfo_T __user *usr_ptr,
	CfaAsfCfgVidInfo_T32 __user *usr_ptr32)
{
	long ret = 0;

	if (copy_from_user(&(usr_ptr->eCodecID), &(usr_ptr32->eCodecID), sizeof(AVCODECID_T)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->eCodecVer), &(usr_ptr32->eCodecVer), sizeof(VCODECVERSION_T)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u8VidCodecSpecDataOft), &(usr_ptr32->u8VidCodecSpecDataOft), sizeof(__u64)))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->u4VidCodecSpecDataLen), &(usr_ptr32->u4VidCodecSpecDataLen), sizeof(__u32)))
		return -EFAULT;
	if (copy_from_user((usr_ptr->au1VidCodecSpecData), &(usr_ptr32->au1VidCodecSpecData), 
			sizeof(__u8) * LP_ASF_CODEC_SPEC_DATA_MAX_LEN))
		return -EFAULT;
	if (copy_from_user(&(usr_ptr->rCfaAsfVc1Mode.eVc1Mode), &(usr_ptr32->rCfaAsfVc1Mode.eVc1Mode), 
			sizeof(CFA_ASF_VC1_MODE_E)))
		return -EFAULT;

	ret = CfaAsfCompatCfgStmInfo(&(usr_ptr->rCfaAsfStrmInfo), &(usr_ptr32->rCfaAsfStrmInfo));
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaAsfCompatConfigVidInfo.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return ret;
	}
	
	return 0;
}

static long CfaAsfCompatConfig(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	CfaAsfCfgInfo __user *usr_ptr = NULL;
	CfaAsfCfgInfo_T32 __user *usr_ptr32 = (CfaAsfCfgInfo_T32 __user *)prInfo->usr_ptr32;
	long ret = 0;
	__u32 i = 0;

	if (NULL == usr_ptr32) {
		DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: usr_ptr32.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EINVAL;
	}
	if (sizeof(CfaAsfCfgInfo_T32) != prInfo->buf_sz) {
		DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
			TEXT("%s line %d fail in invalid args: buf_sz(0x%08x/0x%08x, 0x%08x).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prInfo->buf_sz, sizeof(CfaAsfCfgInfo_T32),
			sizeof(CfaAsfCfgInfo));
		return -EINVAL;
	}
	DmxLogT(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
			TEXT("%s line %d Alloc configure size(0x%08x).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, sizeof(CfaAsfCfgInfo));
	//usr_ptr = (CfaAsfCfgInfo *)compat_alloc_user_space(sizeof(CfaAsfCfgInfo));
	DMX_NewMemory(sizeof(CfaAsfCfgInfo), usr_ptr);

	if (NULL == usr_ptr) {
		DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
			TEXT("%s line %d fail in alloc compat user space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(usr_ptr, 0, sizeof(CfaAsfCfgInfo));

	ret = CfaAsfCompatCfgFileInfo(&(usr_ptr->rCfaAsfCfgFileInfo), &(usr_ptr32->rCfaAsfCfgFileInfo));
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaAsfCompatCfgFileInfo.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(usr_ptr);
		return ret;
	}

	for (i = 0; i < MAX_ASF_AUD_STRM_NUM; i++) {
		ret = CfaAsfCompatConfigAudInfo(&(usr_ptr->raCfaAsfCfgAudInfo[i]), &(usr_ptr32->raCfaAsfCfgAudInfo[i]));
		if (0 != ret) {
			DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
				TEXT("%s line %d fail in CfaAsfCompatConfigAudInfo.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_FreeMemory(usr_ptr);
			return ret;
		}
	}

	ret = CfaAsfCompatConfigVidInfo(&(usr_ptr->rCfaAsfCfgVidInfo), &(usr_ptr32->rCfaAsfCfgVidInfo));
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaAsfCompatConfigVidInfo.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(usr_ptr);
		return ret;
	}

	ret = CfaAsfCompatCfgDrmInfo(&(usr_ptr->rCfaAsfCfgDrmInfo), &(usr_ptr32->rCfaAsfCfgDrmInfo));
	if (0 != ret) {
		DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
			TEXT("%s line %d fail in CfaAsfCompatCfgDrmInfo.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(usr_ptr);
		return ret;
	}

	*pfgIsUserMem = FALSE;
	prInfo->usr_ptr = usr_ptr;
	prInfo->buf_sz = sizeof(CfaAsfRange_T);

	return 0;
}

static long CfaAsfProcCompat(CFA_COMPAT_PROC_INFO_T *prInfo, bool *pfgIsUserMem)
{
	long ret = 0;

	if ((NULL == prInfo) || (NULL == pfgIsUserMem)) {
		DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
					TEXT("%s line %d fail for invalid parameter.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EPERM;
	}
	switch (prInfo->type) {
		case CFA_CONFIG:
			if (prInfo->is_get) {
				DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
					TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EPERM;
			}
			ret = CfaAsfCompatConfig(prInfo, pfgIsUserMem);
			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaAsfCompatConfig.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return ret;
			}
			break;
		case CFA_RANGE:
			if (prInfo->is_get) {
				DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
					TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EPERM;
			}
			ret = CfaAsfCompatRange(prInfo, pfgIsUserMem);
			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaAsfCompatRange.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return ret;
			}
			break;
		case CFA_GEN_INFO:
			DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
				TEXT("%s line %d fail for don't support get info for cfa asf.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			return -EPERM;
		case CFA_JUMP_INFO:
			if (prInfo->is_get) {
				DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
					TEXT("%s line %d fail for don't support get cfa config info.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				return -EPERM;
			}
			ret = CfaAsfCompatJumpRange(prInfo, pfgIsUserMem);
			if (0 != ret) {
				DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
					TEXT("%s line %d fail in CfaAsfCompatJumpRange.\r\n"),
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
void CfaAsfInitPara(CfaAsfInst_T *prCfaAsfInst)
{
	prCfaAsfInst->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_IDLE;
	prCfaAsfInst->u8PrsPts = 0;

	prCfaAsfInst->rCfaAsfCurPosInfo.u8AudCurOfst = 0;
	prCfaAsfInst->rCfaAsfCurPosInfo.u8PacketCurOfst = 0;
	prCfaAsfInst->rCfaAsfCurPosInfo.u8PrsCurOfst = 0;
	prCfaAsfInst->rCfaAsfCurPosInfo.u8VidCurOfst = 0;
	prCfaAsfInst->u4OftInMeidaObj = 0;
	prCfaAsfInst->eCurCfaAsfPrsBitStrmType = CFA_ASF_PRS_BIT_STRM_TYPE_NONE;
	prCfaAsfInst->u4PacketLen = prCfaAsfInst->rCfaAsfFileInfo.u4DataPacketSize;
	prCfaAsfInst->u8KeyPacketId = 0;
	prCfaAsfInst->u1CurStrmId = DMX_INVALID_UINT8;
	prCfaAsfInst->fgDrmEncrypt = FALSE;
	prCfaAsfInst->fgSkipErrPacket = FALSE;

	dmx_memset((void*)(&prCfaAsfInst->rCfaAsfCpsInfo), (u8)0, (u32)sizeof(prCfaAsfInst->rCfaAsfCpsInfo));
	if (prCfaAsfInst->pu1CfaAsfVc1Sc) {
		prCfaAsfInst->pu1CfaAsfVc1Sc[0] = 0x00;
		prCfaAsfInst->pu1CfaAsfVc1Sc[1] = 0x00;
		prCfaAsfInst->pu1CfaAsfVc1Sc[2] = 0x01;
		prCfaAsfInst->pu1CfaAsfVc1Sc[3] = 0x0D;
	}
	prCfaAsfInst->fgErrSingleCompressed = FALSE;
	prCfaAsfInst->u8PreVPts = 0;
	prCfaAsfInst->fgFirstVidAU = TRUE;
	prCfaAsfInst->fgDisplayOrder = FALSE;
	prCfaAsfInst->fgHasAdts = FALSE;
	prCfaAsfInst->fgVidData = FALSE;
}


/*-----------------------------------------------------------------------------
 * Name: CfaAsfSetVidType
 *
 * Description:
 *		CFA ASF sets video type for transfering video data to Video FIFO with the codec info
 *		by playback module setting.
 * Inputs:
 *
 * Outputs:
 *
 * Returns: s32
 *
 *-----------------------------------------------------------------------------*/
static void CfaAsfSetVidType(CfaAsfInst_T *prCfaAsfInst)
{
	MMATE_CHECK_STRUCT(prCfaAsfInst->rCfaAsfVidInfo);
	switch (prCfaAsfInst->rCfaAsfVidInfo.eCodecID) {
	case AVCODEC_ID_NONE:
	case AVCODEC_ID_UNKNOWN:
		prCfaAsfInst->eVidCodecType = CFA_VID_UNKNOWN;
		break;

	case AVCODEC_ID_DIVX3:
		prCfaAsfInst->eVidCodecType = CFA_VID_DIVX3;
		break;

	case AVCODEC_ID_MPEG4:
		prCfaAsfInst->eVidCodecType = CFA_VID_MPEG4;
		prCfaAsfInst->fgNeedEBIHInfo = TRUE;
		prCfaAsfInst->fgNeedSyncBufForSpecData = TRUE;
		break;

	case AVCODEC_ID_WMV1:
		prCfaAsfInst->eVidCodecType = CFA_VID_WMV7;
		prCfaAsfInst->fgNeedEBIHInfo = TRUE;
		prCfaAsfInst->fgNeedSyncBufForSpecData = TRUE;
		break;

	case AVCODEC_ID_WMV2:
		prCfaAsfInst->eVidCodecType = CFA_VID_WMV8;
		prCfaAsfInst->fgNeedEBIHInfo = TRUE;
		prCfaAsfInst->fgNeedSyncBufForSpecData = TRUE;
		break;

	case AVCODEC_ID_WMV3:
		prCfaAsfInst->eVidCodecType = CFA_VID_WMV9;
		prCfaAsfInst->fgNeedEBIHInfo = TRUE;
		prCfaAsfInst->fgNeedSyncBufForSpecData = TRUE;
		break;

	case AVCODEC_ID_VC1:
		prCfaAsfInst->eVidCodecType = CFA_VID_VC1;
		prCfaAsfInst->fgNeedEBIHInfo = TRUE;
		prCfaAsfInst->fgNeedSyncBufForSpecData = TRUE;
		break;

	case AVCODEC_ID_MPEG1:
	case AVCODEC_ID_MPEG2:
		prCfaAsfInst->eVidCodecType = CFA_VID_MPEG2;
			/*Modified by jie.tang*/
		prCfaAsfInst->fgNeedEBIHInfo = TRUE;
		prCfaAsfInst->fgNeedSyncBufForSpecData = TRUE;
		break;

	case AVCODEC_ID_H263:
		prCfaAsfInst->eVidCodecType = CFA_VID_H263;
		prCfaAsfInst->fgNeedEBIHInfo = TRUE;
		break;

	case AVCODEC_ID_H264:
		prCfaAsfInst->eVidCodecType = CFA_VID_H264;
		prCfaAsfInst->fgNeedEBIHInfo = TRUE;
		break;

	case AVCODEC_ID_DIVX4:
		prCfaAsfInst->eVidCodecType = CFA_VID_DIVX4;
		break;

	case AVCODEC_ID_DIVX6:
		prCfaAsfInst->eVidCodecType = CFA_VID_DIVX6;
		prCfaAsfInst->fgNeedEBIHInfo = TRUE;
		prCfaAsfInst->fgNeedSyncBufForSpecData = TRUE;
		break;

	case AVCODEC_ID_VP6:
		prCfaAsfInst->eVidCodecType = CFA_VID_VP6;
		if (VCODEC_VERSION_VP6_WITH_ALPHA == prCfaAsfInst->rCfaAsfVidInfo.eCodecVer){
			prCfaAsfInst->eVidCodecType = CFA_VID_VP6A;
		}
		break;

	case AVCODEC_ID_VP8:
		prCfaAsfInst->eVidCodecType = CFA_VID_VP8;
		break;

	default:
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[CFA_ASF] Line %d, invalid rCfaAsfVidInfo.eCodecID.\n"), DMX_LINE_NO);
		prCfaAsfInst->eVidCodecType = CFA_VID_UNKNOWN;
		break;
	}

}


/*-----------------------------------------------------------------------------
	pvSptHdl: Provided by splitter.  When using API in splitter4cfa.h,
	CFA should pass this handle as the 1st parameter.
	pvPrivData: Provided by App in MPC_CMD_INIT as MPC2FFDescr.pvCfaPrivData.
	MPC passes it to splitter which passes to CFA.
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Name: CfaAsf_Init
 *
 * Description:
 *		Init CFA ASF
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAsf_Init(void *pvSptHdl, void **ppvCfaPrivData)
{
	u32 u4Idx = 0;
	CfaAsfInst_T *prCfaAsfInst = NULL;

	DMX_NewMemory(sizeof(CfaAsfInst_T), prCfaAsfInst);
	if (NULL == prCfaAsfInst) 
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[CFA_ASF] Alloc prCfaAsfInst memory fail\n"));
		MM_RETURN(RET_DMX_NO_MEM);
	}

	dmx_memset(prCfaAsfInst, 0, sizeof(CfaAsfInst_T));

	prCfaAsfInst->ptrPfrMemAddress = DMX_INVALID_UINTPTR_T;
	prCfaAsfInst->pu1HdrBuf = NULL;
	/*prCfaAsfInst->pucVidCodecSpecData = NULL;*/
	prCfaAsfInst->pu1CfaAsfVc1Sc = NULL;
	prCfaAsfInst->u8Ca = DMX_INVALID_UINT64;
	prCfaAsfInst->u4CurPrsFlag = 0;
	prCfaAsfInst->u8CurPacketId = 0;
	prCfaAsfInst->fgNeedEBIHInfo = FALSE;
	prCfaAsfInst->fgSetLpcm = FALSE;
	prCfaAsfInst->fgNoSyncPbb = FALSE;
	prCfaAsfInst->fgFRCurSmpFinish = FALSE;

	DMX_NewHwMemory(4, prCfaAsfInst->pu1CfaAsfVc1Sc);
	if (NULL == prCfaAsfInst->pu1CfaAsfVc1Sc) {
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[CFA_ASF] Alloc prCfaAsfInst->pu1CfaAsfVc1Sc memory fail\n"));
		DMX_FreeMemory(prCfaAsfInst);
		prCfaAsfInst = NULL;
		MM_RETURN(RET_DMX_NO_MEM);
	}

	for (u4Idx = 0; u4Idx < MAX_ASF_AUD_STRM_NUM; u4Idx++) {
		prCfaAsfInst->raCfaAsfAudInfo[u4Idx].u4AudCodecSpecDataLen = 0;
		prCfaAsfInst->raCfaAsfAudInfo[u4Idx].au1AudCodecSpecData = NULL;
	}

	prCfaAsfInst->rCfaAsfVidInfo.u4CodecSpecDataLen = 0;
	prCfaAsfInst->rCfaAsfVidInfo.pu1VidCodecSpecData = NULL;

	/*set initial information*/
	CfaAsfInitPara(prCfaAsfInst);
	DmxLogD(DMX_MOD_CFA_ASF,CFA_ASF_LOG_SET,TEXT("[CFA ASF] init ok!"));

	/*Assign cfa function pointer*/
	*ppvCfaPrivData = (void *)prCfaAsfInst;
	MMATE_CHECK_POINTER(prCfaAsfInst);
	MMATE_CHECK_STRUCT(prCfaAsfInst->rCfaAsfVidInfo);
	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
 * Name: CfaAsf_Uninit
 *
 * Description:
 *		Uninit CFA ASF
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAsf_Uninit(void *pvSptHdl, void *pvCfaPrivData)
{
	u32 u4Idx = 0;
	CfaAsfInst_T *prCfaAsfInst = NULL;

	if (NULL == pvCfaPrivData)
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[CFA_ASF] Line %d, pvCfaPrivData is NULL.\n"), DMX_LINE_NO);
		MM_RETURN((u32)RET_DMX_PARAM_WRONG);
	}

	prCfaAsfInst = (CfaAsfInst_T *)pvCfaPrivData;
	MMATE_CHECK_POINTER(prCfaAsfInst);
	MMATE_CHECK_STRUCT(prCfaAsfInst->rCfaAsfVidInfo);

	if (NULL != prCfaAsfInst->pu1CfaAsfVc1Sc) {
		DMX_FreeHwMemory(prCfaAsfInst->pu1CfaAsfVc1Sc);
		prCfaAsfInst->pu1CfaAsfVc1Sc = NULL;
	}

	for (u4Idx = 0; u4Idx < (u32)MAX_ASF_AUD_STRM_NUM; u4Idx++) {
		if ((prCfaAsfInst->raCfaAsfAudInfo[u4Idx].u4AudCodecSpecDataLen > 0) &&
			(NULL != prCfaAsfInst->raCfaAsfAudInfo[u4Idx].au1AudCodecSpecData))
			DMX_FreeHwMemory(prCfaAsfInst->raCfaAsfAudInfo[u4Idx].au1AudCodecSpecData);
	}

	if ((prCfaAsfInst->rCfaAsfVidInfo.u4CodecSpecDataLen > 0) &&
		(NULL != prCfaAsfInst->rCfaAsfVidInfo.pu1VidCodecSpecData)) {
		DMX_FreeHwMemory(prCfaAsfInst->rCfaAsfVidInfo.pu1VidCodecSpecData);
		prCfaAsfInst->rCfaAsfVidInfo.pu1VidCodecSpecData = NULL;
	}

	prCfaAsfInst->pu1HdrBuf = NULL;

	DMX_FreeMemory(prCfaAsfInst);

	DmxLogD(DMX_MOD_CFA_ASF,CFA_ASF_LOG_SET,TEXT("[CFA ASF] uninit ok!"));

	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
 * Name: CfaAsf_SetRange
 *
 * Description:
 *		ASF CFA sets demuxing range
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
static MRESULT CfaAsf_SetRange(void *pvSptHdl, void *pvRange, void *pvPrivData, bool fgIsUserMem)
{
	CfaAsfInst_T *prCfaAsfInst = NULL;

	if ((NULL == pvPrivData) || (NULL == pvRange))
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[CFA_ASF] Line %d, prPrivData or pvRange is NULL.\n"), DMX_LINE_NO);
		MM_RETURN((u32)RET_DMX_PARAM_WRONG);
	}


	prCfaAsfInst = (CfaAsfInst_T *)pvPrivData;
	MMATE_CHECK_POINTER(prCfaAsfInst);
#ifdef MM_ATE_CHECK
	dmx_memcpy(&(prCfaAsfInst->rCfaAsfRange.fgEnableVid), &(((CfaAsfRange_T *)pvRange)->fgEnableVid),
	sizeof(prCfaAsfInst->rCfaAsfRange) - 2 * sizeof(u32));
#else
	dmx_memcpy(&(prCfaAsfInst->rCfaAsfRange), pvRange, sizeof(prCfaAsfInst->rCfaAsfRange));
#endif
	MMATE_INIT_STRUCT(prCfaAsfInst->rCfaAsfRange);

	prCfaAsfInst->rCfaAsfCurPosInfo.u8VidCurOfst = prCfaAsfInst->rCfaAsfRange.u8VidSa;
	prCfaAsfInst->rCfaAsfCurPosInfo.u8AudCurOfst = prCfaAsfInst->rCfaAsfRange.u8AudSa;
	if ((prCfaAsfInst->rCfaAsfRange.u8AudSa == 0) &&
		 (prCfaAsfInst->rCfaAsfRange.u8VidSa == 0)) {
	#if CFA_ASF_NRD_SUPPORT
		if (TRUE == prCfaAsfInst->rCfaAsfFileInfo.fgIsNrd) {
			/* NRD would feed data from first packet. Not from data object header*/
			prCfaAsfInst->u8Ca = prCfaAsfInst->rCfaAsfFileInfo.u8HeaderObjectSize;
		} else
	#endif
		{
			prCfaAsfInst->u8Ca = prCfaAsfInst->rCfaAsfFileInfo.u8HeaderObjectSize +
			CFA_ASF_DATA_OBJECT_HDR_SIZE;
		}
	} else {
	#if 0 /*mtk40504*/
		prCfaAsfInst->u8Ca = MIN(prCfaAsfInst->rCfaAsfRange.u8AudSa, prCfaAsfInst->rCfaAsfRange.u8VidSa);
	#else
		if ((CFA_ASF_PRS_BIT_STRM_TYPE_VID == (prCfaAsfInst->u4CurPrsFlag)) &&
				(prCfaAsfInst->rCfaAsfRange.fgEnableVid)) {
			prCfaAsfInst->u8Ca = prCfaAsfInst->rCfaAsfRange.u8VidSa;
		} else if ((CFA_ASF_PRS_BIT_STRM_TYPE_AUD == (prCfaAsfInst->u4CurPrsFlag)) &&
				(prCfaAsfInst->rCfaAsfRange.fgEnableAud)) {
				prCfaAsfInst->u8Ca = prCfaAsfInst->rCfaAsfRange.u8AudSa;
		} else {
			prCfaAsfInst->u8Ca = MIN(prCfaAsfInst->rCfaAsfRange.u8AudSa,
				prCfaAsfInst->rCfaAsfRange.u8VidSa);
		}
	#endif
	}

	prCfaAsfInst->u8PacketStartAdr = prCfaAsfInst->u8Ca;
	prCfaAsfInst->fgEnableAud = prCfaAsfInst->rCfaAsfRange.fgEnableAud;
	prCfaAsfInst->fgEnableVid = prCfaAsfInst->rCfaAsfRange.fgEnableVid;

	prCfaAsfInst->fgFirstTxAud = TRUE;
	prCfaAsfInst->fgFirstTxVid = TRUE;
	/*mtk40504 add*/
	prCfaAsfInst->u1AudAUCnt = 0;
	prCfaAsfInst->u8PreAudPts = 0;

	MMATE_CHECK_STRUCT(prCfaAsfInst->rCfaAsfRange);
	DmxLogT(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[CFA ASF] prCfaAsfInst->u8Ca: 0x%x%x\n"),
		(u32)(prCfaAsfInst->u8Ca >> 32), (u32)prCfaAsfInst->u8Ca);
	DmxLogT(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[CFA ASF] set cfa range OK!\n"));
	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
 * Name: CfaAsf_EnableStrm
 *
 * Description:
 *		ASF CFA sets stream to parse, may be combinations of V/A/S.
 *		splitter will ensure that pfvSetStrm() is only called in "off" or "paused" state.
 *
 * Inputs:
 *	  [IN] handle of splitter
 *	  [IN] streams to parse or to cancel parsing
 *	  [IN] CFA_STREAM_ON:  The bits turned ON in u4StrmToPrs are the streams that FMPC would like to parse.
 *	  CFA_STRM_OFF: The bits turned ON in u4StrmToPrs are the streams that FMPC would like to stop parsing
 *	  [IN] pointer to CfaAsfInst
 *
 * Outputs:
 *
 * Returns: s32
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAsf_EnableStrm(void *pvSptHdl, u32 u4StrmToPrs, CfaStreamOp eOp,
							  void *pvPrivData)
{
	CfaAsfInst_T *prCfaAsfInst = NULL;

	if (NULL == pvPrivData)
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[CFA_ASF] Line %d, pvPrivData is NULL.\n"), DMX_LINE_NO);
		MM_RETURN((u32)RET_DMX_PARAM_WRONG);
	}


	prCfaAsfInst = (CfaAsfInst_T *) pvPrivData;
	MMATE_CHECK_POINTER(prCfaAsfInst);
	MMATE_CHECK_STRUCT(prCfaAsfInst->rCfaAsfVidInfo);

	if (CFA_STREAM_ON == eOp) {
		/*enable*/
		if (CFA_STRM_V & u4StrmToPrs)
			prCfaAsfInst->u4CurPrsFlag |= CFA_ASF_PRS_BIT_STRM_TYPE_VID;


		if (CFA_STRM_A & u4StrmToPrs)
			prCfaAsfInst->u4CurPrsFlag |= CFA_ASF_PRS_BIT_STRM_TYPE_AUD;

	} else {/*disable*/
		if (CFA_STRM_V & u4StrmToPrs)
			prCfaAsfInst->u4CurPrsFlag &= ~((u32)CFA_ASF_PRS_BIT_STRM_TYPE_VID);


		if (CFA_STRM_A & u4StrmToPrs)
			prCfaAsfInst->u4CurPrsFlag &= ~((u32)CFA_ASF_PRS_BIT_STRM_TYPE_AUD);

	}

	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
 * Name: CfaAsf_SetStrmInf
 *
 * Description:
 *		Set Stream information
 *
 * Inputs:
 *	   [IN] handle of splitter
 *	   [IN] stream to set
 *	   [IN] stream info
 *	   [IN] pointer to CfaAsfInst
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAsf_SetStrmInf(void *pvSptHdl, u32 u4Strm, u32 u4Info,
							 void *pvPrivData)
{
	CfaAsfInst_T *prCfaAsfInst = NULL;

	if (NULL == pvPrivData)
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[CFA_ASF] Line %d, pvPrivData is NULL.\n"), DMX_LINE_NO);
		MM_RETURN((u32)RET_DMX_PARAM_WRONG);
	}


	prCfaAsfInst = (CfaAsfInst_T *)pvPrivData;
	MMATE_CHECK_POINTER(prCfaAsfInst);
	MMATE_CHECK_STRUCT(prCfaAsfInst->rCfaAsfVidInfo);

	if (CFA_STRM_V == u4Strm) {
		CfaAsfVidInfo_T *prCfaAsfVidInfo = &prCfaAsfInst->rCfaAsfVidInfo;

		prCfaAsfVidInfo->rCfaAsfStrmInfo.u1StrmNum = (u8)(0x000000FF & u4Info);
		}
	else if (CFA_STRM_A == u4Strm) {
		prCfaAsfInst->u4CurAudStrmId = (0x000000FF & u4Info);
		prCfaAsfInst->u1CurAudInfoIdx =
		CfaAsfGetCurAudInfoIdx(prCfaAsfInst, prCfaAsfInst->u4CurAudStrmId);
	} else {
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
 * Name: CfaAsf_TurnOn
 *
 * Description:
 *		ASF CFA turns on file demuxing
 *		A transfer should be issued in this function.
 *
 * Inputs:
 *	  [IN] handle of splitter
 *	  [IN] pointer to CfaAsfInst
 *
 * Outputs:
 *
 * Returns: s32
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAsf_TurnOn(void *pvSptHdl, void *pvPrivData)
{
	CfaAsfInst_T *prCfaAsfInst = NULL;

	if (NULL == pvPrivData)
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[CFA_ASF] Line %d, pvPrivData is NULL.\n"), DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}


	prCfaAsfInst = (CfaAsfInst_T *)pvPrivData;
	MMATE_CHECK_POINTER(prCfaAsfInst);
	MMATE_CHECK_STRUCT(prCfaAsfInst->rCfaAsfVidInfo);

	CfaAsfSetVidType(prCfaAsfInst);

	CfaAsfInitPara(prCfaAsfInst);
	prCfaAsfInst->u4MemDataLen = 0;
	prCfaAsfInst->u4FRAudDataTxLen = 0;
	prCfaAsfInst->fgFRCurSmpFinish = FALSE;
	prCfaAsfInst->eCurInstSt = CFA_ASF_INST_ST_READY;

	CfaAsfSearchHeader(pvSptHdl, prCfaAsfInst, CFA_ASF_ANA_ST_SEARCH_PACKET_HEADER,
						0, CFA_ASF_HDR_BUF_SZ, 0);

	DmxLogT(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[CFA ASF] turn on cfa!"));

	MM_RETURN(RET_DMX_OK);
}

/*// ASF CFA callback for transfer done*/
/*// @return None*/
/*// @note This function will be called after a transfer is complete.*/
/*< [IN] handle of splitter */
/*< [IN] Actual transferred data length.  Normally this value should be equal to the u4Len */
/*in the previous transfer issue, unless file end is hit. */
/*< [IN] pointer to CfaAsfInst */
static MRESULT CfaAsf_TxDone(void *pvSptHdl, u64 u8TxLen, void *pvPrivData, bool fgRsp)
{
	CfaAsfInst_T *prCfaAsfInst = (CfaAsfInst_T *)pvPrivData;

	if (NULL == pvPrivData)
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[CFA_ASF] Line %d, pvPrivData is NULL.\n"), DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	MMATE_CHECK_POINTER(prCfaAsfInst);
	MMATE_CHECK_STRUCT(prCfaAsfInst->rCfaAsfVidInfo);
	MMATE_CHECK_STRUCT(prCfaAsfInst->rCfaAsfLastTxInfo);

	if (fgRsp) {
#if 0
		MRESULT mrRet = Spt4CfaPbb2SyncBufEx(pvSptHdl, prCfaAsfInst->u8Ca, u8TxLen, (u8 *)
		&prCfaAsfInst->u4PfrMemAddress, &prCfaAsfInst->u4MemDataLen);
		if (RET_DMX_OK != mrRet) {
			DMXLOG_ERROR(TEXT("[ASF CFA] call Spt4CfaPbb2SyncBufEx()")
				TEXT("retun err(%d) in CfaAsf_TxDone().\n"), mrRet);
			MM_RETURN(mrRet);
	}
		prCfaAsfInst->fgRealSyncPbbuf = TRUE;

		MM_RETURN(mrRet);
#else
		prCfaAsfInst->u4MemDataLen = 0;
		prCfaAsfInst->eCurCfaAsfAnaSt = CFA_ASF_ANA_ST_SEARCH_HEADER;
		prCfaAsfInst->eCurCfaAsfPrsBitStrmType = CFA_ASF_PRS_BIT_STRM_TYPE_PACKET_HDR;
		prCfaAsfInst->u8Ca = prCfaAsfInst->u8PacketStartAdr
			/* + prCfaAsfInst->rCfaAsfFileInfo.u4DataPacketSize*/;
		/*prCfaAsfInst->u8PacketStartAdr += prCfaAsfInst->rCfaAsfFileInfo.u4DataPacketSize;*/
		DmxLogT(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,TEXT("[CFA ASF] Skip2NextPacketSa: 0x%llx\n"), prCfaAsfInst->u8Ca);
		CfaAsfSearchHeader(pvSptHdl, prCfaAsfInst, CFA_ASF_ANA_ST_SEARCH_PACKET_HEADER, 0, CFA_ASF_HDR_BUF_SZ, 0);

		MM_RETURN(RET_DMX_OK);
#endif
	}

	CfaAsfTxDoneStCtrl(pvSptHdl, u8TxLen, prCfaAsfInst);

	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
 * Name: CfaAsf_GetCurPos
 *
 * Description:
 *		ASF CFA callback for when FMPC needs to know CFA's current position.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAsf_GetCurPos(void *pvSptHdl, void *pvCurPos, void *pvPrivData)
{
	CfaAsfInst_T *prCfaAsfInst = NULL;
	u64 *pvu8 = NULL;

	if ((NULL == pvCurPos) || (NULL == pvPrivData))
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[CFA_ASF] Line %d, pvCurPos or pvPrivData is NULL.\n"), DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	pvu8 = (u64 *)pvCurPos;

	prCfaAsfInst = (CfaAsfInst_T *)pvPrivData;
	*pvu8 = prCfaAsfInst->u8Ca;
	MMATE_CHECK_POINTER(prCfaAsfInst);

	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
 * Name: CfaAsf_FillPicInfo
 *
 * Description:
 *		ASF CFA callback for each picture is demuxed
 *		original related function: vAsfM4vPIsr
 *
 * Inputs:
 *	   [IN] handle of splitter
 *	   [IN/OUT] Picture info, @see Spt2CfaPicInfo
 *	   [IN] pointer to CfaAsfInst
 *
 * Outputs:
 *
 * Returns: TRUE - this picture should be retained in video FIFO.
 *			FALSE - this picture should be removed from video FIFO.
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAsf_FillPicInfo(void *pvSptHdl, Spt2CfaPicInfo *ptPicInfo, void *pvPrivData)
{
	CfaAsfInst_T *prCfaAsfInst = (CfaAsfInst_T *)pvPrivData;

	if ((NULL == ptPicInfo) || (NULL == pvPrivData))
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[CFA_ASF] Line %d, ptPicInfo or pvPrivData is NULL.\n"), DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}


	ptPicInfo->u8ThisPts = prCfaAsfInst->u8PrsPts;
	prCfaAsfInst->u8PrsPts = DMX_INVALID_UINT64;
	MMATE_CHECK_POINTER(prCfaAsfInst);
	MMATE_CHECK_STRUCT(prCfaAsfInst->rCfaAsfVidInfo);

	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
 * Name: CfaAsf_Configure
 *
 * Description:
 *		splitter will ensure that it is only called in "off" or "paused" state.
 *
 * Inputs:
 *		[IN] handle of splitter
 *		[IN] configure paramter
 *		[IN] pointer to CfaAsfInst
 *
 * Outputs:
 *
 * Returns: s32
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAsf_Configure(void *pvSptHdl, void *pvParam, void *pvPrivData, bool fgIsUserMem)
{
	CfaAsfInst_T *prCfaAsfInst = NULL;
	CfaAsfCfgInfo *prCfaAsfCfgInfo = NULL;
	CfaAsfCfgInfo rCfaAsfCfgInfo;
	CfaAsfVidInfo_T *prCfaAsfVidInfo = NULL;
	CfaAsfAudInfo_T *prCfaAsfAudInfo = NULL;
	u32 u4Idx = 0;

	if ((NULL == pvPrivData) || (NULL == pvParam)) {
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[CFA_ASF] Line %d, pvParam or pvPrivData is NULL.\n"), DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mm_memset(&rCfaAsfCfgInfo, 0, sizeof(CfaAsfCfgInfo));
	prCfaAsfCfgInfo = &rCfaAsfCfgInfo;

	prCfaAsfInst = (CfaAsfInst_T *)pvPrivData;
	MMATE_CHECK_POINTER(prCfaAsfInst);

	if (fgIsUserMem) {
		if (0 != mm_copy_from_user(prCfaAsfCfgInfo,
			pvParam, sizeof(CfaAsfCfgInfo))) {
			DmxLogE(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
				TEXT("[CFA_ASF] %s line %d failed in mm_copy_from_user\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
		}
	} else {
		mm_memcpy(prCfaAsfCfgInfo, pvParam, sizeof(CfaAsfCfgInfo));
	}

	/*config video info*/
	prCfaAsfVidInfo = &prCfaAsfInst->rCfaAsfVidInfo;
	prCfaAsfVidInfo->eCodecID = prCfaAsfCfgInfo->rCfaAsfCfgVidInfo.eCodecID;
	prCfaAsfVidInfo->eCodecVer = prCfaAsfCfgInfo->rCfaAsfCfgVidInfo.eCodecVer;
	prCfaAsfVidInfo->rCfaAsfStrmInfo.eStrmType = prCfaAsfCfgInfo->rCfaAsfCfgVidInfo.rCfaAsfStrmInfo.eStrmType;
	prCfaAsfVidInfo->rCfaAsfStrmInfo.u1StrmNum = prCfaAsfCfgInfo->rCfaAsfCfgVidInfo.rCfaAsfStrmInfo.u1StrmNum;
	prCfaAsfVidInfo->u8CodecSpecDataOft = prCfaAsfCfgInfo->rCfaAsfCfgVidInfo.u8VidCodecSpecDataOft;
	prCfaAsfVidInfo->u4CodecSpecDataLen = prCfaAsfCfgInfo->rCfaAsfCfgVidInfo.u4VidCodecSpecDataLen;
	prCfaAsfInst->rCfaAsfQIVc1Mode.eVc1Mode = prCfaAsfCfgInfo->rCfaAsfCfgVidInfo.rCfaAsfVc1Mode.eVc1Mode;
	dmx_memcpy(prCfaAsfVidInfo->rCfaAsfStrmInfo.arPayloadExtSysInfo,
					   prCfaAsfCfgInfo->rCfaAsfCfgVidInfo.rCfaAsfStrmInfo.arPayloadExtSysInfo,
					   MAX_ASF_PL_EXT_SYS_ID * sizeof(CfaAsfPayloadExtSysInfo_T));
	if (((prCfaAsfVidInfo->u4CodecSpecDataLen) > 0) && ((prCfaAsfVidInfo->u4CodecSpecDataLen) <= 0x200)) {
		DMX_NewHwMemory(prCfaAsfVidInfo->u4CodecSpecDataLen, prCfaAsfVidInfo->pu1VidCodecSpecData);
		if (NULL == prCfaAsfVidInfo->pu1VidCodecSpecData) {
			DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,
				TEXT("[CFA ASF] Alloc prCfaAsfVidInfo->pu1VidCodecSpecData memory fail\n!"));
			if (!fgIsUserMem)
				DMX_FreeMemory(pvParam);
			MM_RETURN(RET_DMX_NO_MEM);
		}
		dmx_memset(prCfaAsfVidInfo->pu1VidCodecSpecData, 0, prCfaAsfInst->
					rCfaAsfVidInfo.u4CodecSpecDataLen);

		dmx_memcpy(prCfaAsfVidInfo->pu1VidCodecSpecData, prCfaAsfCfgInfo->rCfaAsfCfgVidInfo.au1VidCodecSpecData, 
				prCfaAsfVidInfo->u4CodecSpecDataLen);
		
	}

	/*set video type for transfering video data to video FIFO.*/
	CfaAsfSetVidType(prCfaAsfInst);

	/*config audio info*/
	for (u4Idx = 0; u4Idx < MAX_ASF_AUD_STRM_NUM; u4Idx++) {
		prCfaAsfAudInfo = &prCfaAsfInst->raCfaAsfAudInfo[u4Idx];
		/*mtk40504 add*/
		prCfaAsfAudInfo->fgVBR = prCfaAsfCfgInfo->raCfaAsfCfgAudInfo[u4Idx].fgVBR;
		prCfaAsfAudInfo->eCodecID = prCfaAsfCfgInfo->raCfaAsfCfgAudInfo[u4Idx].eCodecID;
		if (prCfaAsfAudInfo->eCodecID == AVCODEC_ID_PCM) {
			prCfaAsfInst->fgSetLpcm = TRUE;
		}
		prCfaAsfAudInfo->rCfaAsfStrmInfo.eStrmType = prCfaAsfCfgInfo->
		raCfaAsfCfgAudInfo[u4Idx].rCfaAsfStrmInfo.eStrmType;
		prCfaAsfAudInfo->rCfaAsfStrmInfo.u1StrmNum = prCfaAsfCfgInfo->
		raCfaAsfCfgAudInfo[u4Idx].rCfaAsfStrmInfo.u1StrmNum;
		prCfaAsfAudInfo->u4AveBytePS = prCfaAsfCfgInfo->raCfaAsfCfgAudInfo[u4Idx].u4AveBytePS;

		dmx_memcpy(prCfaAsfAudInfo->rCfaAsfStrmInfo.arPayloadExtSysInfo,
					prCfaAsfCfgInfo->raCfaAsfCfgAudInfo[u4Idx].rCfaAsfStrmInfo.arPayloadExtSysInfo,
					MAX_ASF_PL_EXT_SYS_ID * sizeof(CfaAsfPayloadExtSysInfo_T));
		
		/*MTK40504*/
		prCfaAsfAudInfo->u4SamPS = prCfaAsfCfgInfo->raCfaAsfCfgAudInfo[u4Idx].u4SamPS;
		prCfaAsfAudInfo->u2ChannelNum = prCfaAsfCfgInfo->raCfaAsfCfgAudInfo[u4Idx].u2ChannelNum;
		prCfaAsfAudInfo->fgNeedAdtsHeader = prCfaAsfCfgInfo->raCfaAsfCfgAudInfo[u4Idx].fgNeedAdtsHeader;
		if (((prCfaAsfCfgInfo->raCfaAsfCfgAudInfo[u4Idx].u4AudCodecSpecDataLen) > 0) &&
			((prCfaAsfCfgInfo->raCfaAsfCfgAudInfo[u4Idx].u4AudCodecSpecDataLen) <= 0x200)) {
			prCfaAsfAudInfo->u4AudCodecSpecDataLen = prCfaAsfCfgInfo->
			raCfaAsfCfgAudInfo[u4Idx].u4AudCodecSpecDataLen + 4;
			DMX_NewHwMemory(prCfaAsfAudInfo->u4AudCodecSpecDataLen, prCfaAsfAudInfo->au1AudCodecSpecData);
			if (NULL == prCfaAsfAudInfo->au1AudCodecSpecData) {
				DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,
					TEXT("[CFA ASF] Alloc prCfaAsfAudInfo->au1AudCodecSpecData memory fail\n"));
				if (!fgIsUserMem)
					DMX_FreeMemory(pvParam);
				MM_RETURN(RET_DMX_NO_MEM);
			}
			dmx_memset(prCfaAsfAudInfo->au1AudCodecSpecData, 0, prCfaAsfAudInfo->u4AudCodecSpecDataLen);

			dmx_memcpy(prCfaAsfAudInfo->au1AudCodecSpecData + 4,
						prCfaAsfCfgInfo->raCfaAsfCfgAudInfo[u4Idx].au1AudCodecSpecData,
						prCfaAsfCfgInfo->raCfaAsfCfgAudInfo[u4Idx].u4AudCodecSpecDataLen);
			
			
			prCfaAsfAudInfo->au1AudCodecSpecData[0] =
				(u8)((prCfaAsfCfgInfo->raCfaAsfCfgAudInfo[u4Idx].u4AudCodecSpecDataLen >> 24) &
				0xFF);
			prCfaAsfAudInfo->au1AudCodecSpecData[1] =
				(u8)((prCfaAsfCfgInfo->raCfaAsfCfgAudInfo[u4Idx].u4AudCodecSpecDataLen >> 16) &
				0xFF);
			prCfaAsfAudInfo->au1AudCodecSpecData[2] =
				(u8)((prCfaAsfCfgInfo->raCfaAsfCfgAudInfo[u4Idx].u4AudCodecSpecDataLen >> 8) &
				0xFF);
			prCfaAsfAudInfo->au1AudCodecSpecData[3] =
				(u8)((prCfaAsfCfgInfo->raCfaAsfCfgAudInfo[u4Idx].u4AudCodecSpecDataLen) &
				0xFF);
	}
}

	/*config others info*/
	prCfaAsfInst->rCfaAsfFileInfo.u4DataPacketSize = prCfaAsfCfgInfo->rCfaAsfCfgFileInfo.u4DataPacketSize;
	prCfaAsfInst->rCfaAsfFileInfo.u8DataPacketCount = prCfaAsfCfgInfo->rCfaAsfCfgFileInfo.u8DataPacketCount;
	prCfaAsfInst->rCfaAsfFileInfo.u8FileSize = prCfaAsfCfgInfo->rCfaAsfCfgFileInfo.u8FileSize;
	prCfaAsfInst->rCfaAsfFileInfo.u8HeaderObjectSize = prCfaAsfCfgInfo->rCfaAsfCfgFileInfo.u8HeaderObjectSize;
	prCfaAsfInst->rCfaAsfFileInfo.u8PrerollTime = prCfaAsfCfgInfo->rCfaAsfCfgFileInfo.u8PrerollTime;
	prCfaAsfInst->rCfaAsfFileInfo.fgCfaRespliter = prCfaAsfCfgInfo->rCfaAsfCfgFileInfo.fgCfgRespliter;
	prCfaAsfInst->rCfaAsfFileInfo.fgOnlyWma = prCfaAsfCfgInfo->rCfaAsfCfgFileInfo.fgOnlyWma;

#if CFA_ASF_NRD_SUPPORT
	prCfaAsfInst->rCfaAsfFileInfo.fgIsNrd = prCfaAsfCfgInfo->rCfaAsfCfgFileInfo.fgIsNrd;
#endif

	dmx_memcpy(prCfaAsfInst->rCfaAsfDrmInfo.au1StreamEncrypted, prCfaAsfCfgInfo->rCfaAsfCfgDrmInfo.au1StreamEncrypted,
				sizeof(prCfaAsfCfgInfo->rCfaAsfCfgDrmInfo.au1StreamEncrypted)/sizeof(u8));

	prCfaAsfInst->rCfaAsfDrmInfo.u1WMDRMType = prCfaAsfCfgInfo->rCfaAsfCfgDrmInfo.u1WMDRMType; /* wmdrm 080709*/

	if (!fgIsUserMem)
		DMX_FreeMemory(pvParam);

	MMATE_CHECK_POINTER(prCfaAsfInst);
	MMATE_CHECK_STRUCT(prCfaAsfInst->rCfaAsfVidInfo);

	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
 * Name: CfaAsf_SetInqTypes
 *
 * Description:
 *		ASF CFA sets information query types
 *		splitter will ensure that it is only called in "off" or "paused" state.
 *
 * Inputs:
 *		[IN] handle of splitter
 *		[IN] information type for ASF CFA
 *		[IN] pointer to CfaAsfInst
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAsf_SetInqTypes(void *pvSptHdl, u32 u4InfTypes, void *pvPrivData)
{
	MM_RETURN(RET_DMX_OK);
}


/*//< [IN] input splitter Handle*/
/*//< [IN] CFA function id, set or get id, it shall be defined by CFA and LPE*/
/*//< [IN] input CFA private data*/
/*//< [OUT] The parameter of this FID, it shall be defined by CFA and LPE*/
/*//< [IN] The size of this parameter (of this FID), it shall be defined by CFA and LPE*/
static MRESULT CfaAsf_GetGeneral(void *pvSptHdl, u32 u4CfaFID, void *pvPrivData,
							 void *pvCfaParameter, u32 u4CfaParameterSize)
{
	MM_RETURN(RET_DMX_OK);
}


/*-----------------------------------------------------------------------------
 * Name: CfaAsf_FillAUInfo
 *
 * Description:
 *		ASF CFA callback for each AU is demuxed
 *
 *
 * Inputs:
 *		[IN] input splitter Handle
 *		[IN/OUT] AU info,
 *		[IN] input CFA private data
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAsf_FillAUInfo(void *pvSptHdl, void *pvAUInfo, void *pvAUExtInfo, void *pvPrivData)
{
	CfaAsfInst_T *prCfaAsfInst = NULL;
	u64 u8RealPTS = 0;

	if ((NULL == pvAUInfo) || (NULL == pvPrivData))
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[CFA_ASF] Line %d, pvAUInfo or pvPrivData is NULL.\n"), DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	prCfaAsfInst = (CfaAsfInst_T *)pvPrivData;
	MMATE_CHECK_POINTER(prCfaAsfInst);
	MMATE_CHECK_STRUCT(prCfaAsfInst->rCfaAsfVidInfo);

	/*support WMA for AU table*/

	switch (prCfaAsfInst->eCurCfaAsfTxStrmType) {
	case CFA_ASF_TX_STRM_TYPE_VID:
		/*error handle: for some files, payload PTS may be earlier than preroll time*/
		u8RealPTS = (prCfaAsfInst->u8PrsPts > prCfaAsfInst->rCfaAsfFileInfo.u8PrerollTime) ?
		(prCfaAsfInst->u8PrsPts - prCfaAsfInst->rCfaAsfFileInfo.u8PrerollTime) : 0;
#if CFA_ASF_NRD_SUPPORT
		if (TRUE == prCfaAsfInst->rCfaAsfFileInfo.fgIsNrd) {
			/*[20090616] Kate: write DT_NRD to notify VDEC for different aspect ratio handle */
			((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.eDiscType = DT_NRD;
		} else
#endif
		((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.eDiscType = DT_DATADISC;

#if  !CFA_ASF_USE_VARIABLE_FRAME_RATE
	#if CFA_ASF_CHECK_PIC_TYPE
		if (fgIsBType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) {
			if (prCfaAsfInst->ePrePicType != CFA_PIC_B) {
				/*Reset previous I/P frame pts as Invalid.*/
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8PrevPTS = DMX_INVALID_UINT64;
			}
			((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = prCfaAsfInst->u8PreVPts;
		} else {
			((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = u8RealPTS * CFA_ASF_SYS_CLK;
		}
		prCfaAsfInst->u8PreVPts = u8RealPTS * CFA_ASF_SYS_CLK;
		if (fgIsIType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType))
			prCfaAsfInst->ePrePicType = CFA_PIC_I;

		else if (fgIsPType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType))
			prCfaAsfInst->ePrePicType = CFA_PIC_P;

		else if (fgIsBType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType))
			prCfaAsfInst->ePrePicType = CFA_PIC_B;

		else
			prCfaAsfInst->ePrePicType = CFA_PIC_UNDEFINE;

		((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4Duration = 0;
	#else
		((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = u8RealPTS * CFA_ASF_SYS_CLK;
	#endif
#else
			/*
		 Variable farme rate rule:
		 1) fill pts and previous duration
		 2) if duration < 0, fill INVALID_TIMSTAMP for rest of the AUs
			*/
		if (fgIsBType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType)) {
			if ((prCfaAsfInst->ePrePicType != CFA_PIC_B) &&
				(prCfaAsfInst->ePrePicType != CFA_PIC_I)) {
				/*Reset previous I/P frame pts as Invalid.*/
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8PrevPTS = DMX_INVALID_UINT64;
				prCfaAsfInst->fgDisplayOrder = TRUE;
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = prCfaAsfInst->u8PreVPts;
			} else {
				if (TRUE == prCfaAsfInst->fgDisplayOrder)
					((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = prCfaAsfInst->u8PreVPts;

			else
				((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts =
				u8RealPTS * (u64)CFA_ASF_SYS_CLK;

			/* prCfaAsfInst->fgDisplayOrder = FALSE;*/
			}
		} else {
			((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts = u8RealPTS * (u64)CFA_ASF_SYS_CLK;
			prCfaAsfInst->fgDisplayOrder = FALSE;
		}

		if (fgIsIType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType))
			prCfaAsfInst->ePrePicType = CFA_PIC_I;

		else if (fgIsPType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType))
			prCfaAsfInst->ePrePicType = CFA_PIC_P;

		else if (fgIsBType(((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4VType))
			prCfaAsfInst->ePrePicType = CFA_PIC_B;

		else
			prCfaAsfInst->ePrePicType = CFA_PIC_UNDEFINE;


		if (prCfaAsfInst->u8PrsPts <
			(prCfaAsfInst->u8PreVPts / CFA_ASF_SYS_CLK +
			prCfaAsfInst->rCfaAsfFileInfo.u8PrerollTime)) {
			((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4PrevDuration = DMX_INVALID_UINT32;
		} else {
			((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u4PrevDuration = (u32)(prCfaAsfInst->
			u8PrsPts - prCfaAsfInst->u8PreVPts / (u64)CFA_ASF_SYS_CLK - prCfaAsfInst->
			rCfaAsfFileInfo.u8PrerollTime) * (u32)CFA_ASF_SYS_CLK;
		}

		prCfaAsfInst->u8PreVPts = u8RealPTS * (u64)CFA_ASF_SYS_CLK;

		prCfaAsfInst->fgFirstVidAU = FALSE;
#endif
		/*RETAILMSG(1, (TEXT("V PTS: %d\n"), ((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts));*/
		/*DMXLOG_TRACE(TEXT("V PTS: %lld\n"), ((AU_VPic *)pvAUInfo)->rAUInfo.rInfo.u8Pts);*/
		break;

	case CFA_ASF_TX_STRM_TYPE_AUD:
	/* error handle: for some files, payload PTS may be earlier than preroll time*/
	u8RealPTS = (prCfaAsfInst->u8PrsPts > prCfaAsfInst->rCfaAsfFileInfo.u8PrerollTime) ?
	(prCfaAsfInst->u8PrsPts - prCfaAsfInst->rCfaAsfFileInfo.u8PrerollTime) : 0;
	#if 0	/*mtk40504, correct CBR pts*/
		if (FALSE == prCfaAsfInst->raCfaAsfAudInfo[prCfaAsfInst->u1CurAudInfoIdx].fgVBR) {
			prCfaAsfInst->u1AudAUCnt++;
			if (prCfaAsfInst->u1AudAUCnt > 2) {
				u64 u8Diff = u8RealPTS - prCfaAsfInst->u8PreAudPts;

				if (u8Diff > prCfaAsfInst->u8DiffPts * 3 / 2)
					u8RealPTS = prCfaAsfInst->u8PreAudPts + prCfaAsfInst->u8DiffPts;

				else
					prCfaAsfInst->u8DiffPts = u8RealPTS - prCfaAsfInst->u8PreAudPts;

				prCfaAsfInst->u8PreAudPts = u8RealPTS;
				prCfaAsfInst->u1AudAUCnt = 3;
			} else if (1 == prCfaAsfInst->u1AudAUCnt) {
				prCfaAsfInst->u8PreAudPts = u8RealPTS;
				prCfaAsfInst->u8DiffPts = 0;
			} else if (2 == prCfaAsfInst->u1AudAUCnt) {
				prCfaAsfInst->u8DiffPts = u8RealPTS - prCfaAsfInst->u8PreAudPts;
				prCfaAsfInst->u8PreAudPts = u8RealPTS;
			}
		}
	#endif
		((AU_AUDIO *)pvAUInfo)->rAUInfo.rInfo.u8Pts = u8RealPTS * (u64)CFA_ASF_SYS_CLK;
		/*RETAILMSG(1, (TEXT("A PTS: %d\n"), ((AU_AUDIO *)pvAUInfo)->rAUInfo.rInfo.u8Pts));*/
		/*DMXLOG_TRACE(TEXT("A PTS: %lld\n"),*/
		/* ((AU_AUDIO *)pvAUInfo)->rAUInfo.rInfo.u8Pts);*/
		break;

	case CFA_ASF_TX_STRM_TYPE_NONE:
		break;

	default:
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	MM_RETURN(RET_DMX_OK);
}

/*-----------------------------------------------------------------------------
 * Name: i4CfaAsf_TxAudHDRInfo
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
static MRESULT CfaAsf_TxAudHdrInfo(void *pvSptHdl, u32 u4TxUID, void *pvPrivData)
{
	MM_RETURN(RET_DMX_UNSUPPORT);
}

#if CFA_ASF_NRD_SUPPORT
/*!
 * @brief Notify Netflix data inform
 *
 * It is for splitter to notify CFA that Netflix data inform change
 * It is called once splitter receive IBC_InbandCmdTypeNrdDataInfo
 *
 * @see
 *
 * @retval NULL (0)
 *			 return ok.
 * @retval NOT NULL (~0)
 *			 return fail.
 */
/*//< [IN] input splitter Handle*/
/*//< [IN] netflix data inform*/
/*//< [IN] input CFA private data*/
MRESULT CfaAsf_NfSetDataInfo(void *pvSptHdl, Spt2CfaNetFlixDataInform *ptNfDataInf,
						void *pvPrivData)
{
	CfaAsfInst_T *prCfaAsfInst = NULL;

	if ((NULL == pvPrivData) || (NULL == ptNfDataInf))
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[CFA_ASF] Line %d, pvNfDataInf or pvPrivData is NULL.\n"), DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}


	prCfaAsfInst = (CfaAsfInst_T *)pvPrivData;

	prCfaAsfInst->rCfaAsfNfInfo.eNrdType = ptNfDataInf->eNrdType;
	prCfaAsfInst->rCfaAsfNfInfo.u8AudioStreamID = ptNfDataInf->u8AudioStreamID;
	prCfaAsfInst->rCfaAsfNfInfo.u8VideoStreamID = ptNfDataInf->u8VideoStreamID;
	prCfaAsfInst->rCfaAsfNfInfo.u8Preroll = ptNfDataInf->u8Preroll;
	prCfaAsfInst->rCfaAsfNfInfo.u8PacketSize = ptNfDataInf->u8PacketSize;

	prCfaAsfInst->rCfaAsfFileInfo.u4DataPacketSize = ptNfDataInf->u8PacketSize;
	prCfaAsfInst->rCfaAsfFileInfo.u8PrerollTime = ptNfDataInf->u8Preroll;
	if (CFA_NrdDataType_PacketMux == ptNfDataInf->eNrdType ||
		CFA_NrdDataType_PacketVideo == ptNfDataInf->eNrdType) {
		prCfaAsfInst->rCfaAsfVidInfo.rCfaAsfStrmInfo.u1StrmNum = ptNfDataInf->u8VideoStreamID;
	}
	if (CFA_NrdDataType_PacketMux == ptNfDataInf->eNrdType ||
		CFA_NrdDataType_PacketAudio == ptNfDataInf->eNrdType) {
		prCfaAsfInst->raCfaAsfAudInfo[0].rCfaAsfStrmInfo.u1StrmNum = ptNfDataInf->u8AudioStreamID;
	}
	MMATE_CHECK_POINTER(prCfaAsfInst);

	MM_RETURN(RET_DMX_OK);
}

/*!
 * @brief Notify cfa the Netflix CFA_END
 *
 * It is for splitter to notify CFA that Netflix CFA_END
 * It is called once splitter receive EOS from LPCH
 *
 * @see
 *
 * @retval NULL (0)
 *			 return ok.
 * @retval NOT NULL (~0)
 *			 return fail.
 */
/*//< [IN] input splitter Handle*/
/*//< [IN] input CFA private data*/
MRESULT CfaAsf_NfEndNfy(void *pvSptHdl, void *pvPrivData)
{
	CfaAsfInst_T *prCfaAsfInst = NULL;

	if (NULL == pvPrivData)
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[CFA_ASF] Line %d, pvPrivData is NULL.\n"), DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	prCfaAsfInst = (CfaAsfInst_T *)pvPrivData;

	if (CFA_ASF_INST_ST_READY == prCfaAsfInst->eCurInstSt) {
		CfaAsf_FinishPrsNrd(pvSptHdl, pvPrivData);
		prCfaAsfInst->eCurInstSt = CFA_ASF_INST_ST_INITED;
	}

	MM_RETURN(RET_DMX_OK);
}
#endif //CFA_ASF_NRD_SUPPORT

/*-----------------------------------------------------------------------------
 * Name: CfaAsf_SetJumpRange
 *
 * Description:
 *		Fro Support 8/16/32 fast forward and fast backward, to Reset CFA All state.
 *
 * Inputs:
 *		[IN] handle of splitter
 *		[IN] pointer to CfaAsfKeyFrameRange
 *		[IN] pointer to CfaAsfInst
 *
 * Outputs:
 *
 * Returns: s32

 *
 *-----------------------------------------------------------------------------*/
static MRESULT CfaAsf_SetJumpRange(void *pvSptHdl, void *pvJmpRange, void *pvPrivData)
{
	CfaAsfInst_T *prCfaAsfInst = NULL;
	CfaAsfKeyFrameRange_T *prAsfKFrmRange = NULL;
	CfaAsfRange_T *prCfaRangeInfo = NULL;

	if ((NULL == pvPrivData) || (NULL == pvJmpRange))
	{
		DmxLogE(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT, TEXT("[CFA_ASF] Line %d, pvPrivData or pvPrivData is NULL.\n"), DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prCfaAsfInst = (CfaAsfInst_T *) pvPrivData;
	prAsfKFrmRange = (CfaAsfKeyFrameRange_T *)pvJmpRange;

	prCfaRangeInfo = &(prAsfKFrmRange->rCfaRangeInfo);

	DmxLogT(DMX_MOD_CFA_ASF,CFA_ASF_LOG_DEFAULT,
		TEXT("[CFA ASF][CFA KEY REG]CfaAsf_SetJumpRange,")
		TEXT("u8VidStartOfst:0x%llx u8AudStartOfst:0x%llx \r\n"),
		prCfaRangeInfo->u8VidSa,
		prCfaRangeInfo->u8AudSa);

	CfaAsf_SetRange(pvSptHdl, (void *)(&(prAsfKFrmRange->rCfaRangeInfo)), pvPrivData, TRUE);
	MMATE_CHECK_POINTER(prCfaAsfInst);

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaAsfGetParamSize(void *pvSptHdl, u32 u4ParamID,
	void  *pvPrivData, void  *pvCfaParam, u32 u4CfaParamSz)
{
	MRESULT mrRet = RET_DMX_OK;

	switch (u4ParamID) {
	case CFA_PARAM_ID_JUMP_INFO_SIZE:

		if ((NULL == pvCfaParam) || (u4CfaParamSz < sizeof(u32))) {
			mrRet = RET_DMX_PARAM_WRONG;
		} else {
			u32 *pu4Tmp = (u32 *)pvCfaParam;
			*pu4Tmp = sizeof(CfaAsfKeyFrameRange_T);
		}

		break;

	default:
		mrRet = RET_DMX_PARAM_WRONG;
		break;
	}

	MM_RETURN(mrRet);
}

static MRESULT CfaAsfProcCliCmd(void *pvSptHdl, E_DMX_CFA_CLI_TYPE_T eCliType, /*< [IN] Cfa Cli Command*/
				u32 arg1,
				u32 arg2, u32 arg3, const char *szParam, VOID *pvPrivData)
{
	MRESULT mrRet = RET_DMX_OK;
	CfaAsfInst_T *prCfaAsf = NULL;

	prCfaAsf = (CfaAsfInst_T *) pvPrivData;

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

			DmxLogT(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
				TEXT("CfaAsfProcCliCmd -- fgEnable: %d, Loglvl: %d, ModLogLvl: 0x%08x \r\n"),
				arg1, arg2, arg3);

			DmxLogEnable(fgEnable, arg2, DMX_MOD_CFA_ASF, arg3);
		}
		break;
	case DMX_CFA_CLI_CMD_DUMP_INFO:
		{
			DmxLogT(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
				TEXT("Cfa ASF Instance(handle is 0x%x)")
				TEXT(" Info list as follow: \r\n"),
				prCfaAsf);
			DmxLogT(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
				TEXT("Current Analyse State is %d, Previous Analyse State is %d \r\n"),
				prCfaAsf->eCurCfaAsfAnaSt, prCfaAsf->eLastCfaAsfAnaSt);
			DmxLogT(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
				TEXT("Current Analyse Position is 0x%08x%08x, ")
				TEXT("Previous Analyse Position is 0x%08x%08x\r\n"),
				(UINT32) ((prCfaAsf->u8Ca) >> 32), (UINT32) (prCfaAsf->u8Ca),
				(UINT32) ((prCfaAsf->u8LastCa) >> 32), (UINT32) (prCfaAsf->u8LastCa));
			DmxLogT(DMX_MOD_CFA_ASF, CFA_ASF_LOG_DEFAULT,
				TEXT("First Tx Video Flag is %d, First Tx Audio Flag is %d\r\n"),
				((prCfaAsf->fgFirstTxVid) ? 1 : 0),
				((prCfaAsf->fgFirstTxAud) ? 1 : 0));
		}
		break;
	default:
		break;
	}

	MM_RETURN(mrRet);
}

/* ASF CFA interface */
CfaIntf _rAsfCfaIntf = {
	&CfaAsf_Init,
	&CfaAsf_Uninit,
	&CfaAsf_SetRange,
	&CfaAsf_EnableStrm,
	&CfaAsf_SetStrmInf,
	&CfaAsf_TurnOn,
	&CfaAsf_TxDone,
	&CfaAsf_GetCurPos,
	&CfaAsf_FillPicInfo,
	&CfaAsf_Configure,
	&CfaAsf_SetInqTypes,
	&CfaAsf_GetGeneral,
	NULL,
	NULL,
	&CfaAsf_FillAUInfo,
	&CfaAsf_TxAudHdrInfo,
	NULL,
	&CfaAsf_SetJumpRange,
	&CfaAsfGetParamSize,
	&CfaAsfProcCliCmd
	#ifdef CONFIG_COMPAT
	,&CfaAsfProcCompat
	#endif
};


/*-----------------------------------------------------------------------------
 * Name: pvCfaAsfGetInterface
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
void *CfaAsfGetInterface(void)
{
	return ((void *)&_rAsfCfaIntf);
}


