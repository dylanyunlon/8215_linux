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
 */

/*!
 * @file dmx_spt_util.c
 *
 *
 * @par Project
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
#include <linux/errno.h>
#include "windows.h"
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/dmx_cfa_def.h>
#include <media/atc/drv_aud.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "dmx_splitter.h"
#include "dmx_cfa_def.h"
#include "drv_aud.h"
#include "mm_debug.h"
#endif /* __linux__*/

#include "dmx_spt_util.h"
#include "dmx_spt_cfa.h"
#include "dmx_stream.h"
/*#include "cfa_mp4.h"*/
#include "cfa_macro.h"

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterGetCfaStreamType*/
/* Get Cfa Stream Type by Spt streamType(SPT_DATA_V, SPT_DATA_A,xxx) and Stream No*/
/*/////////////////////////////////////////////////////////////////////////////*/
u32 SplitterGetCfaStreamType(u32 u4SptStreamType, u32 u4SptStreamNo)
{
	switch (u4SptStreamType) {
	case SPT_DATA_V:
		if (u4SptStreamNo == 0)
			return CFA_STRM_V0;
		if (u4SptStreamNo == 1)
			return CFA_STRM_V1;
		if (u4SptStreamNo == 2)
			return CFA_STRM_V2;
		break;

	case SPT_DATA_A:
		if (u4SptStreamNo == 0)
			return CFA_STRM_A0;
		if (u4SptStreamNo == 1)
			return CFA_STRM_A1;
		if (u4SptStreamNo == 2)
			return CFA_STRM_A2;
		break;

	case SPT_DATA_SECTION: /*added by Mingxu Wang 2011/11/9*/
		return CFA_STRM_SEC;

	case SPT_DATA_SP:
		if (u4SptStreamNo == 0)
			return CFA_STRM_SP0;
		if (u4SptStreamNo == 1)
			return CFA_STRM_SP1;
		break;
	default:
		break;
	}

	return 0;
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* Spt4CfaGetPitureType*/
/* Get Start Code Type by Cfa Picture type*/
/*/////////////////////////////////////////////////////////////////////////////*/
u32 Spt4CfaGetPitureType(u32 u4CfaPictureType)
{
	switch (u4CfaPictureType) {
	case CFA_PTM_NO_PIC:
		return 0;

	case CFA_PTM_EXACT_POS:
		return 0;

	case CFA_PTM_SAME_POS:
		return 0;

	case CFA_PTM_ONE_PIC_DX3_I:
		return DX3_I_FRM;

	case CFA_PTM_ONE_PIC_DX3_P:
		return DX3_P_FRM;

	case CFA_PTM_WMV_I:
		return IVOP;

	case CFA_PTM_WMV_P:
		return PVOP;

	case CFA_PTM_WMV_B:
		return BVOP;

	case CFA_PTM_WMV_SEQHDR:
		return SEQ_HDR;

	case CFA_PTM_WMV_SKIPFRAME:
		return SKIPFRAME;

	case CFA_PTM_DUMMY:
		return DUMMY_FRM;

	case CFA_PTM_H263_SORENSON_I:
		return SH_I_VOP;

	case CFA_PTM_H263_SORENSON_P:
		return SH_P_VOP;

	case CFA_PTM_RM_INTRAPIC:
		return INTRAPIC;

	case CFA_PTM_RM_FORCED_INTRAPIC:
		return FORCED_INTRAPIC;

	case CFA_PTM_RM_INTERPIC:
		return INTERPIC;

	case CFA_PTM_RM_TRUEBPIC:
		return TRUEBPIC;

	case CFA_PTM_MJPEG_I:
		return I_TYPE;

	case CFA_PTM_ONE_PIC_VP6_I:
		return I_TYPE;

	case CFA_PTM_ONE_PIC_VP6_P:
		return P_TYPE;


	case CFA_PTM_ONE_PIC_VP8_I:
		return I_TYPE;

	case CFA_PTM_ONE_PIC_VP8_P:
		return P_TYPE;

	default:
		break;
	}

	return 0;
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* Spt4CfaGetAudioCodec*/
/* Get Audio Driver Codec by Cfa Audio Codec*/
/*/////////////////////////////////////////////////////////////////////////////*/
u32 Spt4CfaGetAudioCodec(u32 u4CfaAudioCodec)
{
	switch (u4CfaAudioCodec) {
	case CFA_AUD_DRV_FMT_MPEG:
		return AUD_DRV_FMT_MPEG;

	case CFA_AUD_DRV_FMT_AC3:
		return AUD_DRV_FMT_AC3;

	case CFA_AUD_DRV_FMT_PCM:
		return AUD_DRV_FMT_PCM;

	case CFA_AUD_DRV_FMT_MP3:
		return AUD_DRV_FMT_MP3;

	case CFA_AUD_DRV_FMT_AAC:
		return AUD_DRV_FMT_AAC;

	case CFA_AUD_DRV_FMT_DTS:
		return AUD_DRV_FMT_DTS;

	case CFA_AUD_DRV_FMT_WMA:
		return AUD_DRV_FMT_WMA;

	case CFA_AUD_DRV_FMT_RA:
		return AUD_DRV_FMT_RA;

	case CFA_AUD_DRV_FMT_HDCD:
		return AUD_DRV_FMT_HDCD;

	case CFA_AUD_DRV_FMT_MLP:
		return AUD_DRV_FMT_MLP;

	case CFA_AUD_DRV_FMT_MTS:
		return AUD_DRV_FMT_MTS;

	case CFA_AUD_DRV_FMT_EU_CANAL_PLUS:
		return AUD_DRV_FMT_EU_CANAL_PLUS;

	case CFA_AUD_DRV_FMT_TV_SYS:
		return AUD_DRV_FMT_TV_SYS;

	case CFA_AUD_DRV_FMT_EAC3:
		return AUD_DRV_FMT_EAC3;

	case CFA_AUD_DRV_FMT_EAC3_SEC:
		return AUD_DRV_FMT_EAC3_SEC;

	case CFA_AUD_DRV_FMT_DTSHD_PRI_XLL:
		return AUD_DRV_FMT_DTSHD_PRI_XLL;

	case CFA_AUD_DRV_FMT_DTSHD_PRI_NO_XLL:
		return AUD_DRV_FMT_DTSHD_PRI_NO_XLL;

	case CFA_AUD_DRV_FMT_DTSHD_SEC:
		return AUD_DRV_FMT_DTSHD_SEC;

	case CFA_AUD_DRV_FMT_DTSCD:
		return AUD_DRV_FMT_DTSCD;

	case CFA_AUD_DRV_FMT_TRUE_HD:
		return AUD_DRV_FMT_TRUE_HD;

	case CFA_AUD_DRV_FMT_CDDA:
		return AUD_DRV_FMT_CDDA;

	case CFA_AUD_DRV_FMT_SACD:
		return AUD_DRV_FMT_SACD;

	case CFA_AUD_DRV_FMT_VORBIS:
		return AUD_DRV_FMT_VORBIS;

	case CFA_AUD_DRV_FMT_DST:
		return AUD_DRV_FMT_DST;

	case CFA_AUD_DRV_FMT_FLAC:	 /*added by Mingxu Wang @2011/2/26*/
		return AUD_DRV_FMT_FLAC;

	case CFA_AUD_DRV_FMT_COOK:
		return AUD_DRV_FMT_RA_COOK;

	case CFA_AUD_DRV_FMT_APE:
		return AUD_DRV_FMT_APE;

	default:
		break;
	}

	return AUD_DRV_FMT_UNKNOWN;
}

