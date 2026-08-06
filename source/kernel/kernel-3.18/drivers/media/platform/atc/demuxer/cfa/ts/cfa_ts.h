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

#ifndef _CFA_TS_H_
#define _CFA_TS_H_

/* C header file */
#ifdef __cplusplus
extern "C" {
#endif

#include "x_typedef.h"
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/dmx_cfa_ts.h>
#include <media/atc/dmx_event.h>
#include <media/atc/drv_esm_if.h>
/* #include <media/atc/mm_debug.h> */

#include "dmx_spt_cfa.h"
#include "cfa_cfg.h"
#include "cfa_macro.h"


#define CFA_TS_MAX_TS_PACKET_SIZE	204
#define CFA_TS_MAX_PTS_RECORD_LEN		3
#define CFA_TS_MAX_DELTA_PTS_REC_LEN	3
#define CFA_TS_MIN_PTS_CHG_CNT	   3 /*min count of  pts record*/
#define CFA_TS_MAX_PTS_CHG_CNT	   5 /*min count of  pts record*/
#define CFA_TS_MAX_PES_PTS_NOT_CONSECUTIVE_CNT		50
/*the max count pes pts not consecutive, when pts inconsectuive times upto this value, use faintly consecutive*/
#define CFA_TS_DIFF_DELTA_PES_PTS					900
#define CFA_TS_MAX_GET_DELTA_PES_PTS_CNT			50
/*the max count for not getting delta pes pts*/
#define CFA_TS_MAX_FILTER_IDEX		((u32)8192)
#define CFA_TS_INVALID_PID			DMX_INVALID_UINT32
#define CFG_ALLOC_CC_MEM_WHEN_INIT					1

#ifdef BSP_WIFI_WFD
#define CFG_SUPPORT_HDCP			1
#else
#define CFG_SUPPORT_HDCP			0
#endif

#include "winutil.h"
#define CFA_TS_SEMA_NAME		   TEXT("CFA_TS_SEMA")

#if CFG_SUPPORT_HDCP
#include "hdcp2x_ioctl.h"
#endif
/* TS CFA parsing stream type */
typedef enum CfaTsPrsStrmType {
	CFA_TS_PRS_STRM_TYPE_NONE =	(0x00),		/*< none */
	CFA_TS_PRS_STRM_TYPE_V	  = (0x01<<0),	/*< video */
	CFA_TS_PRS_STRM_TYPE_A	  = (0x01<<1),	/*< audio */
	CFA_TS_PRS_STRM_TYPE_SP   = (0x01<<2),	/*< sp	*/
	CFA_TS_PRS_STRM_TYPE_SEC   = (0x01<<3)	/*< section  */
} CfaTsPrsStrmType;


typedef enum {
	CFA_TS_LOG_DEFAULT = 1 << 0,
	CFA_TS_LOG_COMMON  = 1 << 1,
} CfaTsLogLvl_E;


typedef struct _CfaTsInst CfaTsInst;

/*pid type, section or pes packet*/
typedef enum {TS_UNKNOWN, TS_SECTION, TS_PESPACKET} CfaTsPidType;

typedef struct CfaTsPidFilter CfaTsPidFilter_T;
typedef void CfaTsPktCB(CfaTsPidFilter_T *prPidFilter, u8 *pucData, u32 u4Len);
/*callback handle pes packet or section*/
typedef void PidFilterUninitPrivate(void *pvPrivate);	/*Uninit filter private data*/
typedef u32 PidFilterInitPrivate(void **ppvPrivate, CfaTsInst *prCfaTs);  /*Init filter private data*/


/*PID filter*/
struct CfaTsPidFilter {
	u32 u4PID;	/*filter PID*/
	bool fgCcOk;	/*flag continuity counter*/
	s32 i4LastCc;  /*last continuity counter, initialization value is -1*/
	u32 u4CcErrCnt;
	CfaTsPidType ePidType; /* section or PES packet*/
	u8 u1TableId; /*for section filter only*/

	void *pvPrivate; /*filter pravite data*/
	PidFilterInitPrivate *pfInitPrivate;  /*init pvPrivate*/
	PidFilterUninitPrivate *pfUninitPrivate;  /*free pvPrivate;*/

	bool fgUnitStart;
	CfaTsPktCB *pfTsPktCb;	/*callback to process this PID stream*/

	bool fgTxDataFromUinitStart;
	bool fgNeedRollback;
};


typedef enum {APTS_OK, APTS_PCR_CHANGE, APTS_DATA_ERROR, APTS_NO_SIGNAL} CfaTsAPtsState;

typedef enum {VPTS_OK, VPTS_PCR_CHANGE, VPTS_DATA_ERROR, VPTS_NO_SIGNAL} CfaTsVPtsState;

typedef enum {PTS_INVALID, PTS_VALID} CmdQPtsEntryState;
typedef struct {
	u32 u4CmdQIndex;			/*the cmdq index when get this pts*/
	u64 u8DataInBuf;			/*the data in CMDQ buf when get this pts*/
	u64 u8PreTxEndOff;		/*previous tx end offset when get this pts*/
	u64 u8FileOffset;
	u64 u8Pts;
	CmdQPtsEntryState eState;
} CmdQPtsEntryInfo_t;

typedef struct {
	u64 u8Pts;
	CmdQPtsEntryState eState;
} BufPtsInfo_t;

typedef struct {
	u32 u4EntryNb;
	u32 u4UsedEntryCnt;
	CmdQPtsEntryInfo_t aruEntryInfo[DMX_MAX_TX_CNT_FOR_CMD_Q];
} CmdQPtsInfo_t;

typedef struct {
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
	#endif
	u32 u4AudCmdQIndex;
	u64 u8AudPreTxEndOff;
	u64 u8AudDataInBuf;
	CmdQPtsInfo_t rAudCmdQPtsInfo;

	CFA_AUDIO_INFO_T rAudInf;
	bool fgBackup;
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
	#endif
} AudCmdQBakInfo_t;

typedef enum {UNKNOWN_INFO, AUD_DELTA_PE_PTS_INFO, VID_DELTA_PE_PTS_INFO} DeltaPesPtsInfoType;
typedef struct {
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
	#endif
	bool   fgNeedGet;
	DeltaPesPtsInfoType eInfoType;
	u64 au8PtsRecord[CFA_TS_MAX_PTS_RECORD_LEN];
	u64 u8DeltaPesPts;
	u32 u4GetCnt;
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
	#endif
} DeltaPesPtsInfo_t;

typedef enum {DATA_NO, DATA_A, DATA_V, DATA_SEC, DATA_CC} DATA_TYPE;

/*CFA ts*/
struct _CfaTsInst {
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKStart;
	#endif
	HANDLE	sCfaTs;

#if CFG_SUPPORT_HDCP
	HANDLE hHdcpDrv;
	bool   fgHDCP;
#endif
	bool fgAudSwDec;

	void *pvSptHdl;
	uintptr_t ptrMemAddr;			/*address of pbbuf*/
	u32 u4MemDataLen;		/*memory data len of address ptrMemAddr*/
	u32 u4MemDataRemain;		/*remain data length of address ptrMemAddr*/
	u8 *pucPacketBuf;		/*buffer memory data when u4MemDataRemain < u4TsPacketSize*/
	u32 u4PacketBufLen;		/*data length of aucPacketBuf*/
	u64 u8Ca;				/*current analytic address*/
	u64 u8ParseOffset;
	bool fgStopHandlePkt;		/*flag that indicate stop handle ts packet*/
#if CFG_ALLOC_CC_MEM_WHEN_INIT
	u8 *pucCcPacketBuf;
	u32 u4CcPacketBufLen;
#endif

	bool fgSyncPbbuf;
	bool fgAudSeek;
	bool fgVidSeek;

	DATA_SOURCE eDataSource;	/*file or stream*/
	u32 u4TsPacketSize;		/*packet size, 188/192/204*/
	bool fgIsISDBT1Seg;

	CfaTsRange_T rRange;
	CfaTsPrsStrmType eCurPrsStm;

	u32 u4CurPrsFlg;			/*record which stream has been enable*/

	u32 u4VideoStreamID;		/*stream ID which created by splitter*/
	u32 u4AudioStreamID;		/*stream ID which created by splitter*/
	u32 u4SectionStreamID;	/*stream ID which created by splitter*/
	u32 u4SpStreamID;		/*cc*/

	u32 u4AudStreamNb;		/*audio stream total number*/

	CfaTsPidFilter_T *Pids[CFA_TS_MAX_FILTER_IDEX];
	DATA_TYPE eDataType;

	u32 u4CurVidPID;
	u32 u4CurAudPID;
	u32 u4CurCcPID;

	CFA_VIDEO_INFO_T rVidInf;
	CFA_AUDIO_INFO_T rAudInf;

	u64 u8PtsIncrement;

	/* about video pts */
	u64 u8LastRealVidPts;
	u64 u8CurRealVidPts;
	u32 u4VidFrameCnt;
	u64 u8DeltaVPts;
	u64 au8DeltaVPtsRec[CFA_TS_MAX_DELTA_PTS_REC_LEN];
	u32 u4DeltaVPtsRecIdx;
	u64 u8LastVidPts;

	DeltaPesPtsInfo_t rVidDeltaPesPtsInfo;
	u64 au8Rec4CorrectVPts[CFA_TS_MAX_PTS_CHG_CNT];	/*record for correct video pts*/

	u64 u8LastPesVidPts;
	u64 u8CurPesVidPts;

	bool   fgFirstGetVidPts;
	u64 u8VidBasePts;

	u32 u4PesVidPtsIncCnt; /*pts increase count*/
	u64 u8OldCurPesVidPts;

	u32 u4PesVidPtsDecCnt; /*pts decrease count*/
	u64 u8OldLastPesVidPts;
	u64 u8FirstAbnormalPesVidPts;

	CfaTsVPtsState	eVPtsState;
	BufPtsInfo_t rVidBufPts;
	/* about video pts end*/

	/* about video tick count */
	u32 u4LastVidTicCnt;			/*tick count when get last video data*/
	u32 u4CurVidTicCnt;			/*tick count when get current video data*/
	/* about video tick count end */

	/* about tick count */
	u32 u4LastAudTicCnt;			/*tick count when get last audio data*/
	u32 u4CurAudTicCnt;			/*tick count when get current audio data*/
	/* about tick count end */

	/* about audio pts*/
	u32 u4NoAudTicCnt;			/*tick count record how long time has no audio data*/
	u64 u8LastAudPts;
	u64 u8LastFinalAudPts;		/*for dump abnormal PTS*/


	u32 u4PesAudPtsDecCnt;		/*pts decrease count*/
	u64 u8OldLastPesAudPts;
	bool   fgNeedCheckAPtsDec;
	u64 u8LastPesAudPts;
	u64 u8CurPesAudPts;
	DeltaPesPtsInfo_t rAudDeltaPesPtsInfo;
	u64 au8Rec4CorrectAPts[CFA_TS_MIN_PTS_CHG_CNT];	/*record for correct audio pts*/
	u32 u4AudPesPtsInconsecutiveCnt;

	u32 u4PesAudPtsIncCnt; /*pts increase count*/
	u64 u8OldCurPesAudPts;
	bool   fgNeedCheckAPtsInc;

	bool   fgFirstGetAudPts;
	u64 u8AudBasePts;

	CfaTsAPtsState	eAPtsState;

	bool   fgPcrChange;
	bool   fgNoSignal;
	BufPtsInfo_t rAudBufPts;
	/* about audio pts end*/

	/*for cmd q*/
	u32 u4VidCmdQIndex;
	u64 u8VidPreTxEndOff;
	u64 u8VidFirstOffset;
	u64 u8VidDataInBuf;
	CmdQPtsInfo_t rVidCmdQPtsInfo;
	CmdQPtsInfo_t rVidCmdQPtsInfoBak;
	/*if some video data has been txed to fifo, but has not form au. it's pts info should be backup*/

	u32 u4AudCmdQIndex;
	u64 u8AudPreTxEndOff;
	u64 u8AudFirstOffset;
	u64 u8AudDataInBuf;
	CmdQPtsInfo_t rAudCmdQPtsInfo;
	CmdQPtsEntryState eACmdQFirstPtsEntryState;

	AudCmdQBakInfo_t rAudCmdQBakInfo;

 #ifdef BSP_WIFI_WFD
	 bool  fgFillAu;
 #endif

	/*for remnant data of a ts packet*/
	bool	fgHasRemnant;
	u8	aucRemnantBuf[CFA_TS_MAX_TS_PACKET_SIZE];
	u32	u4RemnantLen;
	u32	u4RemnantPID;
	#ifdef MM_ATE_CHECK
	u32 u4MMATECHKEnd;
	#endif
};

/* C header file */
#ifdef __cplusplus
}
#endif

#endif	/* _CFA_TS_H_ */

