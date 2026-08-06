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




#ifndef DMX_CFA_ASF_H
#define DMX_CFA_ASF_H

#include "x_typedef.h"
#include "dmx_define.h"
#include "mm_debug.h"
#include "mm_common.h"

/* Old C header file */
#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
		    macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/
#define MAX_ASF_AUD_STRM_NUM  8
#define MAX_ASF_PL_EXT_SYS_ID 6

#define CFA_ASF_USE_VARIABLE_FRAME_RATE 1

#define CFA_MPG_QUERY_INF_MAX_STRM_NS (32)

#define ASF_AAC_SUPPORT 1	/* mtk40504 */

#define LP_ASF_CODEC_SPEC_DATA_MAX_LEN (0x200)

/* / ASF CFA query information type, max 32 information */
	enum {
		CFA_ASF_QUERY_INF_TYPE_NONE = 0x00000000,	/* < none */
		CFA_ASF_QUERY_INF_TYPE_VC1_MODE = 0x00000001,	/* < WVC1 parsing mode */
	};

	typedef enum {
		CFA_ASF_VC1_MD_WITH_SC,
		CFA_ASF_VC1_MD_WITH_OUT_SC,
	} CFA_ASF_VC1_MODE_E;

	typedef struct CfaAsfQIVc1Mode {
		CFA_ASF_VC1_MODE_E eVc1Mode;
	} CfaAsfQIVc1Mode_T;

	typedef enum {
		CFA_STRM_TYPE_VBR,	/* >For audio */
		CFA_STRM_TYPE_CBR,	/* >for audio */
		CFA_STRM_TYPE_NORMAL	/* >normal video */
	} CfaStrmTypeInfo_E;

/* For wmdrm ND */
	typedef enum {
		CFA_PL_EXT_SYS_ID_UNKNOW = 0x0,
		CFA_PL_EXT_SYS_ID_TC,
		CFA_PL_EXT_SYS_ID_FN,
		CFA_PL_EXT_SYS_ID_CT,
		CFA_PL_EXT_SYS_ID_AR,
		CFA_PL_EXT_SYS_ID_SR,
		CFA_PL_EXT_SYS_ID_SID
	} CfaPayloadExtSysId_E;

	typedef struct {
		CfaPayloadExtSysId_E ePayloadExtSysId;
		__u16 u2DataSize;
	} CfaAsfPayloadExtSysInfo_T;

	typedef enum {
		CFA_ASF_SKIP_NONE,	/* >Do not skip */
		CFA_ASF_SKIP_BY_PACKET,	/* >Skip by packet number */
		CFA_ASF_SKIP_BY_PTS	/* >Skip by PTS */
	} CfaAsfSkipType_E;

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
	} CfaAsfRange_T;

	typedef struct {
		CfaAsfRange_T rCfaRangeInfo;
	} CfaAsfKeyFrameRange_T;


	typedef struct {
		__u64 u8PacketCurOfst;
		__u64 u8AudCurOfst;
		__u64 u8VidCurOfst;
		__u64 u8PrsCurOfst;
	} CfaAsfCurPosInfo_T;

	typedef struct {
		bool fgCfgRespliter;
		bool fgOnlyWma;
		bool fgIsNrd;
		__u32 u4DataPacketSize;
		__u64 u8FileSize;
		__u64 u8HeaderObjectSize;
		__u64 u8DataPacketCount;
		__u64 u8PrerollTime;
	} CfaAsfCfgFileInfo_T;

	typedef struct {
		__u8 u1StrmNum;
		CfaStrmTypeInfo_E eStrmType;
		CfaAsfPayloadExtSysInfo_T arPayloadExtSysInfo[MAX_ASF_PL_EXT_SYS_ID];
	} CfaAsfCfgStrmInfo_T;

	typedef struct {
		CfaAsfCfgStrmInfo_T rCfaAsfStrmInfo;
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
	} CfaAsfCfgAudInfo_T;

	typedef struct {
		CfaAsfCfgStrmInfo_T rCfaAsfStrmInfo;
		AVCODECID_T eCodecID;
		VCODECVERSION_T eCodecVer;
		__u64 u8VidCodecSpecDataOft;
		__u32 u4VidCodecSpecDataLen;
		/* Modified by jie.tang */
		__u8 au1VidCodecSpecData[LP_ASF_CODEC_SPEC_DATA_MAX_LEN];
		CfaAsfQIVc1Mode_T rCfaAsfVc1Mode;	/* avaliable when vid codec is MVC1 */
	} CfaAsfCfgVidInfo_T;

/* wmdrm */
	typedef struct {
		__u8 u1WMDRMType;	/* 0 : not wmdrm,  PD - 1, ND - 2 */
		/* Each bit presents whether the related stream is encrypted. LSB bit 0 represent strem 1 */
		__u8 au1StreamEncrypted[128 / 8];
		__u32 u4DecInfSize;	/* sizeof decryption info. For pd, it is WRMHEADER */
		__u8 *pu1DecInf;	/* decryption info */
	} CfaAsfCfgDRMInfo_T;


	typedef struct {
		CfaAsfCfgFileInfo_T rCfaAsfCfgFileInfo;
		CfaAsfCfgAudInfo_T raCfaAsfCfgAudInfo[MAX_ASF_AUD_STRM_NUM];
		CfaAsfCfgVidInfo_T rCfaAsfCfgVidInfo;
		CfaAsfCfgDRMInfo_T rCfaAsfCfgDrmInfo;	/* wmdrm */
	} CfaAsfCfgInfo;

	typedef struct {
		__u8 u1StrmId;
		__u32 u4Pts;
	} CfaAsfGetPtsInfo;

#ifdef __cplusplus
}
#endif
#endif				/* DMX_CFA_ASF_H */
