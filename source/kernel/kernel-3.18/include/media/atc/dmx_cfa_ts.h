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

#ifndef DMX_CFA_TS_H
#define DMX_CFA_TS_H

#include "x_typedef.h"
#include "mm_debug.h"
#include "mm_common.h"

/* Old C header file */
#ifdef __cplusplus
extern "C" {

#endif	/*  */

#define TS_SYC_WORD									(0x47)
#define TS_COM_PKT_SIZE_188					(188)
#define TS_DVHS_PKT_SIZE_192					(192)
#define TS_FEC_PKT_SIZE_204					(204)
#define TS_PKT_SIZE_MAX							(TS_FEC_PKT_SIZE_204)

/* audio stream info */
typedef struct {
	__u16 u2Pid;
	__u16 u2PcrPid;
	AVCODECID_T eCodec;
} CfaTsAStreamInfo_T;

/* max number of audio stream; */
#define CFA_TS_AUD_STREAM_NB_MAX	10

/* video stream info */
typedef struct {
	__u16 u2Pid;
	__u16 u2PcrPid;
	__u32 u4Fps_n;
	__u32 u4Fps_d;
	__u32 u4BitRate;
	AVCODECID_T eCodec;
} CfaTsVStreamInfo_T;

typedef struct {
	__u16 u2Pid;
	__u16 u2TableId;
	__u16 u2ServiceId;
} CfaTsSectionInfo_T;

typedef struct {
	__u16 u2Pid;
} CfaTsCcInfo_T;

typedef enum {
	TS_USE_TYPE_UNKNOWN,
	TS_PLAYBACK,	/* Playback, perhaps record */
	TS_RECORD,	/* Only Record */
	TS_TIMESHIFT,
	TS_DMA,
	MAX_NUM_TMX_USE_TYPE
} TsUseType;

typedef enum {
	TS_DATA_FROM_UNKNOWN = 0x0,
	TS_DATA_FROM_PBBUF,
	TS_DATA_FROM_TS_SERIAL,
	TS_DATA_FROM_TS_PARALLEL,
	MAX_OF_TS_DATA_FROM_TYPE
} TsDataFromType;

typedef enum {
	DATA_SOURCE_UNKNOWN,
	DATA_SOURCE_FILE,	/* the data source is file */
	DATA_SOURCE_STREAM,	/* the data source is stream */
	DATA_SOURCE_WFD
} DATA_SOURCE;

typedef enum {
	TS_CHANNEL_UNKNOWN,
	TS_CHANNEL_ONESEG,
	TS_CHANNEL_FULLSEG,
	MAX_CNT_OF_TS_CHANNEL_TYPE,
} E_TS_CHANNEL_TYPE_T;

#define CFA_TS_MAX_SECINFO_NB  32
#define CFA_TS_FILE_NAME_MAX_LEN 256

/* configure info */
typedef struct {
	bool fgCfgDataSource;
	E_TS_CHANNEL_TYPE_T eChannelType;
	TsUseType eUseType;
	TsDataFromType eDataFromType;
	DATA_SOURCE eDataSource;
	__u32 u4TsPktSize;	/* TS packet size */
	bool fgIsISDBT1Seg;
	bool fgCfgVid;
	CfaTsVStreamInfo_T rVStreamInfo;
	bool fgDelVidFilter;
	CfaTsVStreamInfo_T rDelVStreamInfo;
	bool fgCfgPcrPid;
	__u16 u2PcrPid;
	bool fgDelPcrPidFilter;
	__u16 u2DelPcrPid;
	bool fgCfgPcrBase;
	__u64 u8PcrBase;
	bool fgStartRecord;
	__u16 au2RecFileName[CFA_TS_FILE_NAME_MAX_LEN + 1];
	bool fgStopRecord;
	bool fgCfgCc;
	CfaTsCcInfo_T rCcStreamInfo;
	bool fgDelCcFilter;
	CfaTsCcInfo_T rDelCcStreamInfo;
	bool fgCfgAud;
	__u32 u4AStreamNb;	/* audio stream number */
	CfaTsAStreamInfo_T arAStreamInfo[CFA_TS_AUD_STREAM_NB_MAX];
	bool fgDelAudFilter;
	__u32 u4DelAStreamNb;	/* audio stream number */
	CfaTsAStreamInfo_T arDelAStreamInfo[CFA_TS_AUD_STREAM_NB_MAX];
	bool fgSetCurAudPid;	/* for audio track switch */
	__u32 u4CurAudPid;
	bool fgCfgSection;	/* configure section filter */
	__u32 u4CfgSecInfoNb;
	/* for configure section */
	CfaTsSectionInfo_T arCfgSectionInfo[CFA_TS_MAX_SECINFO_NB];
	bool fgDelSecFilter;	/* delete section filter */
	__u32 u4DelSecInfoNb;
	/* for delete section filter */
	CfaTsSectionInfo_T arDelSectionInfo[CFA_TS_MAX_SECINFO_NB];
	bool fgConfigHdcp;
	bool fgHdcp;
	bool fgAudSwDec;
} CfaTsConfigInfo_T;

typedef struct {
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKStart;
#endif	/*  */
	__u64 u8Sa;	/* start address of file */
	__u64 u8Ea;	/* end address of file */
	bool fgIsSeek;
	__u64 u8SeekTime;	/* unit is ms */
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKEnd;
#endif	/*  */
} CfaTsRange_T;

#ifdef __cplusplus
}
#endif	/*  */

#endif	/*  */
