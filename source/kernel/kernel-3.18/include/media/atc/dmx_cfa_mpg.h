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


#ifndef DMX_CFA_MPG_H
#define DMX_CFA_MPG_H

#include "x_typedef.h"
#include "mm_debug.h"
#include "mm_common.h"

/* C header file */
#ifdef __cplusplus
extern "C" {
#endif

/* #include "x_typedef.h" */
#define CFA_MPG_INVALID_RANGE_START_ADDRESS			(0xFFFFFFFFFFFFFFFFll)
#define CFA_MPG_SUPPORT_SELFDECTECT_VOB_ILVU			(0)	/* 0: open in the SR integration */
#define CFA_MPG_SUPPORT_DVD_VOBU_STILL_AUTOPAUSE	(1)
#define CFA_MPG_SEAMLESS_2ND_RANGE_SUPPORT				(1)
#define CFA_MPG_KEEP_VOB_PTM											(1)
#define CFA_MPG_NOTIFY_SCR_IN_FILLAU							(1)
#define CFA_MPG_DVD_AUDIO_DEFINE_SUPPORT					(1)
#define CFA_MPG_QUERY_INF_MAX_STRM_NS						(32)
#define CFA_MPG_PTS_1S				(90000)
#define CFA_MPG_SUPPORT_AVC		(1)


/* / MPG CFA configuration type */
	enum {
		/* < none */
		CFA_MPG_CFG_TYPE_NONE = 0x00,
		/* < configure file stream information */
		CFA_MPG_CFG_TYPE_FILE_STRM_INF = 0x01,
		/* < configure MEDIUM info, ex: type(file, dvd, vcd... CFA_MPG_MED_CFG_TYPE_XXX) */
		CFA_MPG_CFG_TYPE_MEDIUM_INF = 0x02,
		/* < configure sector size info, ex: dvd = 2048, vcd = 2448.... */
		CFA_MPG_CFG_TYPE_SECTOR_INF = 0x03
	};

/* / MPG CFA medium type */
	enum {
		CFA_MPG_MED_CFG_TYPE_NONE = 0x00,	/* < none */
		CFA_MPG_MED_CFG_TYPE_FILE = 0x01,	/* < File */
		CFA_MPG_MED_CFG_TYPE_DVD = 0x02,	/* < DVD */
		CFA_MPG_MED_CFG_TYPE_VCD = 0x03,	/* < VCD */
		CFA_MPG_MED_CFG_TYPE_MVR = 0x04,	/* < -VR */
		CFA_MPG_MED_CFG_TYPE_PVR = 0x05,	/* < +VR */
		CFA_MPG_MED_CFG_TYPE_SVCD = 0x06,	/* < SVCD */
		CFA_MPG_MED_CFG_TYPE_CINEMANOW = 0x07,	/* < CinemaNow */
		CFA_MPG_MED_CFG_TYPE_DVDA = 0x08	/* < DVD-AUDIO */
	};

/* / MPG CFA DISC type */
	enum {
		CFA_MPG_CFG_DISC_NONE = 0x00,	/* < none */
		CFA_MPG_CFG_DISC_VR = 0x01,	/* < -R */
		CFA_MPG_CFG_DISC_VRW = 0x02	/* < -RW */
	};


/* / MPG CFA medium information for CFA_MPG_CFG_TYPE_MEDIUM_INF */
	typedef struct CfaMpgCfgMediumInf {
		/* < Flag indicates if the stream is only MPEG vide stream only */
		bool fgMpgVidStrmOnly;
		/* <  MPG CFA medium type */
		__u32 u4CfaMpgMediumType;
		/* < flag indicates if "don't" support auto pause, only for VCD, default: 0 is supported */
		bool fgNoSupportVCDAutoPause;
		__u16 u2DiscType;	/* < for disc type, -r/-rw */
		__u16 u2DiscCpsType;	/* < for disc cprm information. */
		__u8 ucAstMode;
		__u16 u2VideoType;	/* < for CinemaNow only, config video type by LPE */
		__u16 u2AudioType;	/* < for CinemaNow only, config audio type by LPE */
		__u16 u2DiscSectorSz;	/* <    MPG CFA sector size */

		__u32 u4VideoFps_n;
		__u32 u4VideoFps_d;

		bool fgHighBitrate;
		__u64 u8VidSpecDataOfst;
		__u32 u4VidSpecDataLen;
		void *pu1VidSpecData;
	} CfaMpgCfg;

/* / MPG CFA query information type, max 32 information */
	enum {
		CFA_MPG_QUERY_INF_TYPE_NONE = 0x00000000,	/* < none */
		/* < first MPEG start code information */
		CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_SC_INF = 0x00000001,
		/* < first MPEG PTS information */
		CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_PTS_INF = 0x00000002,
		/* < stream information */
		CFA_MPG_QUERY_INF_TYPE_STRM_INF = 0x00000004,
		/* < VCD check auto pause */
		CFA_MPG_QUERY_INF_TYPE_AUTO_PAUSE_INF = 0x00000008,
		/* < VCD check mux rate */
		CFA_MPG_QUERY_INF_TYPE_MUX_RATE_INF = 0x00000010,
		/* < last MPEG PTS information */
		CFA_MPG_QUERY_INF_TYPE_LAST_MPEG_PTS_INF = 0x00000020
	};

/* MPG CFA first MPEG start code information for CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_SC_INF */
	typedef struct CfaMpgQIFirstMpgScInf {
		__u32 u4Sc;	/* < MPEG start code */
		__u64 u8Pos;	/* < file offset of first byte of MPEG start code (value range is __u32) */
	} CfaMpgQIFirstMpgScInf;

/* MPG CFA first MPEG PTS information for CFA_MPG_QUERY_INF_TYPE_FIRST_MPEG_PTS_INF */
	typedef struct CfaMpgQIFirstMpgPtsInf {
		__u64 u8Pts;	/* < MPEG PTS */
		__u64 u8Pos;	/* < file offset of PTS position (value range is __u32) */
		__u16 u2CurPrsPktType;
	} CfaMpgQIFirstMpgPtsInf;

/* MPG CFA stream information for CFA_MPG_QUERY_INF_TYPE_STRM_INF */
	typedef struct CfaMpgQIStrmInf {
#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKStart;
#endif
		__u32 u4AudStrmNs;	/* < audio stream number */
		__u32 u4SpStrmNs;	/* < sub-picture stream number */
		__u32 u4VidStrmNs;	/* < video stream number */

		__u16 au2DecAstId[CFA_MPG_QUERY_INF_MAX_STRM_NS];	/* < audio stream id */
		__u8 aucDecAstType[CFA_MPG_QUERY_INF_MAX_STRM_NS];	/* < audio stream type */
		__u8 aucDecAstAtt[CFA_MPG_QUERY_INF_MAX_STRM_NS];	/* < audio stream decoding attribute */
		__u8 aucDecSpstId[CFA_MPG_QUERY_INF_MAX_STRM_NS];	/* < sub-picture stream id */
		__u8 aucDecVstId[CFA_MPG_QUERY_INF_MAX_STRM_NS];	/* < sub-picture stream id */
		__u8 aucDecVstType[CFA_MPG_QUERY_INF_MAX_STRM_NS];	/* < video stream type */

		__u32 u4BitRate;
#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKEnd;
#endif
	} CfaMpgQIStrmInf;

/* MPG CFA check auto pause point information for CFA_MPG_QUERY_INF_TYPE_AUTO_PAUSE_INF */
	typedef struct CfaMpgQIAutoPauseInf {
		bool fgIsPrsAutoPause;
	} CfaMpgQIAutoPauseInf;

/* MPG CFA get mux-rate information for CFA_MPG_QUERY_INF_TYPE_MUX_RATE_INF */
	typedef struct CfaMpgQIMuxRateInf_T {
		__u32 u4MuxRate;
	} CfaMpgQIMuxRateInf_T;


	typedef enum {
		/* no PTS value adjustment */
		MPG_PTS_ADJUST_NONE,
		/* All PTS/DTS/SCR should be adjusted by i8AdjustValue */
		MPG_PTS_ADJUST_BY_VALUE,
		/* All PTS/DTS/SCR should be adjusted by
		   ( VOB_V_E_PTM(preceding) - VOB_V_S_PTM(succeeding) ) */
		MPG_PTS_ADJUST_BY_NV_PCK,
		/* All PTS/DTS/SCR should be adjusted by
		   ( VOB_V_S_PTM(previous) - VOB_V_E_PTM(current) ) */
		MPG_PTS_ADJUST_BY_NV_PCK_RVS,
		/* All PTS/DTS/SCR should be adjusted by
		   ( VOB_V_S_PTM(previous) - VOB_V_E_PTM(current) - i8AdjustValue ) */
		MPG_PTS_ADJUST_BY_NV_PCK_RVS_EX,
		/* CFA should keep the current PTS/DTS/SCR adjustment value */
		MPG_PTS_ADJUST_KEEP_CURRENT,
		/* adjust all pts from  i8AdjustValue,- used for mpeg file */
		MPG_PTS_ADJUST_FROM_VALUE
	} MpgPtsAdjustType;

	typedef enum {
		MPG_REX_NORMAL,	/* no parameter */
		MPG_REX_A_FOLLOW_1ST_PIC,	/* no parameter */
		MPG_REX_A_FOLLOW_NTH_PIC,	/* parm: rAFollowNthPicParm */
		MPG_REX_A_FOLLOW_PTS,	/* parm: u8Pts */
		MPG_REX_REPARSE_A,	/* parm: u8RecoverOfst */
		MPG_REX_REPARSE_SP	/* parm: u8RecoverOfst */
	} MpgRangeEx;

	typedef struct {
		__u32 u4PicNo;
		__u32 u4FrmPts;
	} MpgAFollowNthPicParm;

	typedef union {
		/* only audio data whose PTS is larger than the nth pic is parsed into A FIFO */
		/* for MPG_REX_A_FOLLOW_NTH_PIC */
		MpgAFollowNthPicParm rAFollowNthPicParm;
		/* only audio data whose PTS is larger than this value is parsed into A FIFO */
		/* for MPG_REX_A_FOLLOW_PTS */
		__u64 u8Pts;
		/* until this offset, only audio (or subtitle) is parsed into FIFO */
		/* for MPG_REX_REPARSE_A and MPG_REX_REPARSE_SP */
		__u64 u8RecoverOfst;
	} MpgRangeExParm;
#if CFA_MPG_DVD_AUDIO_DEFINE_SUPPORT
	typedef enum {
		M_NORMAL,	/* <normal case */
		M_SEC_PARSE_N_SEC_SKIP,	/* <Parse m seconds and the skip n seconds, */
		M_SEC_PARSE_NOTIFY_SEC_DONE	/* <Parse m seconds and the notify done, */
	} MpgParseMode;
	typedef struct {
		MpgParseMode eMpgParseMode;	/* < cfa parse mode setting */
		__u16 u2ParseSec;	/* < parse data length(measured in second) */
		__u16 u2SkipSec;	/* < skip data length(measured in second) */
	} MpgRangMode;

	typedef enum {
		MPEG_DVDA_VOB,	/* <VOB case */
		MPEG_DVDA_ASV,	/* <ASV, */
		MPEG_DVDA_AOB	/* <AOB, */
	} MpgDVDAType;
#endif

/* / MPG CFA range */
	typedef struct CfaMpgRange {
#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKStart;
#endif
		__u64 u8Sa;	/* < start file offset (value range is __u32) */
		__u64 u8Ea;	/* < end file offset (value range is __u32) */
		/* < Video start file offset (value range is __u32), default: 0xFFFFFFFFFFFFFFFF, ignore. */
		__u64 u8VidSa;
		/* < Audio start file offset (value range is __u32), default: 0xFFFFFFFFFFFFFFFF, ignore. */
		__u64 u8AudSa;
		__u64 u8SPSa;	/* < start file offset (value range is __u32), default: 0xFFFFFFFFFFFFFFFF, ignore. */
#if CFA_MPG_DVD_AUDIO_DEFINE_SUPPORT
		MpgRangMode rMpgRangMode;	/* < mpeg cfa lpcm/pcm parse mode */
		MpgDVDAType eMpgDVDAType;	/* <dvd audio type */
#endif
#if (CFA_MPG_SUPPORT_DVD_VOBU_STILL_AUTOPAUSE)
		bool fgVobuStill;	/* < Vobu still case, normal title: 0, Vobu still: 1 */
#endif

		/* PTS adjustment */
		MpgPtsAdjustType ePtsAdjustType;
		__s64 i8AdjustValue;	/* for MPG_PTS_ADJUST_BY_VALUE */
		__u8 u1ApRatFlag;

		/* for -VR AspRatio */
		__u8 u1PassAspRatFlag;
		__u8 ucAspRatioFromLpe;

		/* Range extension */
		MpgRangeEx eExType;
		MpgRangeExParm rExParm;

#if CFA_MPG_SEAMLESS_2ND_RANGE_SUPPORT
		/* Add by pingzhao,  for BDP00018159, 2008/12/30 */
		bool fgSeamless2ndRange;
#endif

		/* add for +vr error handle, parse end after find first i frame */
		bool fgParseEndAfIFrame;

#if CFA_MPG_KEEP_VOB_PTM
		bool fgKeepVobPtm;
#endif

#if 1				/* CFA_MPG_HIGH_BIT_RATE_HANDLE */
		bool fgHighBitrate;
#endif
		bool fgIsSeek;
		__u64 u8SeekTime;
#ifdef MM_ATE_CHECK
		__u32 u4MMATECHKEnd;
#endif
	} CfaMpgRange;

/* / MPG CFA picture information */
	typedef struct CfaMpgPicInf {
	/* < picture pack position, because current driver firmware limitation, */
	/* using __u32 to indicate position */
		__u32 u4PicPckPos;
	} CfaMpgPicInf;

/* / MPG CFA stream information */
/* / Stream ID information can be seperated into four bytes. */
/* / 1st byte means stream_id_extension */
/* /     2nd byte means sub_stream_id_private_stream_2 */
/* /     3th byte means stream_id */
/* /     4th byte means sub_stream_id_private_stream_1 */
	typedef struct CfaMpgStrmInf {
		__u8 u1SubStrmIdPriStrm1;	/* < sub_stream_id for private_stream_1 */
		__u8 u1StrmId;	/* < stream_id */
		__u8 u1SubStrmIdPriStrm2;	/* < sub_stream_id for private_stream_2 */
		__u8 u1StrmIdExtension;	/* < stream_id extension */
	} CfaMpgStrmInf;

	typedef struct CfaMpgKeyFrameRange {
		CfaMpgRange rCfaMpgRange;
	} CfaMpgKeyFrameRange;

/* C header file */
#ifdef __cplusplus
}
#endif
#endif				/* DMX_CFA_MPG_H */
