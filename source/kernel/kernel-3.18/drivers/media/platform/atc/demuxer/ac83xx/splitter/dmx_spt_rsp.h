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


/*!
 * @file dmx_spt_rsp.h
 *
 *
 * @par Project
 *
 * @par Description
 *
 *
 * @par Author_Name
 *    Shuhui Zhang
 *
 */

#ifndef DMX_SPT_RSP_H
#define DMX_SPT_RSP_H

#include "x_typedef.h"
#include "dmx_spt.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RSP_ENABLE                       1

#define SPT_REC_RSP_INF_STRM_MAX         (25)

#define RSP_SAMPHDR_LEN_MAX              (64 * 1024 * 2)

/*256M*/
#define MAX_OF_RSP_LOGAU_AUDSP_CNT       (2048)
#define MAX_OF_RSP_LOGAU_AUD_CNT         (2048)
#define MAX_OF_RSP_LOGAU_SP_CNT          (2048)

/*128M*/
#define MAX_OF_RSP_LOGAU_AUDSP_CNT_128M  (2048)
#define MAX_OF_RSP_LOGAU_AUD_CNT_128M    (1548)
#define MAX_OF_RSP_LOGAU_SP_CNT_128M     (500)

typedef enum{
	SPLITTER_RSP_INF_IDLE    = 0x0,
	SPLITTER_RSP_INF_ADD,
	SPLITTER_RSP_INF_REMOVE,
	SPLITTER_RSP_INF_CLEAR
} E_DMX_RSP_EXTINFPROC_TYPE_T;

/* Enumerate Splitter Transfer State*/
typedef enum{
	SPLITTER_STATE_RSP_IDLE    = 0x0, /*< rsp idle*/
	SPLITTER_STATE_RSP_LOGING  = 0x3, /*< rsp log on*/
	SPLITTER_STATE_RSP_RSPING  = 0x6, /*< rsp working*/
	SPLITTER_STATE_RSP_ERROR   = 0xF  /*< rsp error*/
} E_DMX_RSP_STATE_T;

typedef enum{
	SPLITTER_RSP_ENTRY_IDLE    = 0x0, /*< rsp idle. i.e. Resplitter Entry has not been used*/
	/*< rsp log done, i.e. Resplitter Entry's Log Info(SPT_RSP_T) has been filled*/
	SPLITTER_RSP_ENTRY_TX_DONE = 0x2,
	SPLITTER_RSP_ENTRY_AU_DONE = 0x5, /*< rsp au done, i.e. Resplitter Entry's AU info has been filled*/
} E_DMX_SPT_RSP_ENTRY_TYPE_T;

/*the header transfer stage for Audio  sample, 0: init, 1: txing, 2: done*/
typedef enum{
	RSP_TX_AHEADS_IDLE = 0,   /*  Indicate heads transfer state of  audio sample  init*/
	RSP_TX_AHEADS_TXING,      /*  Indicate heads transfer state of  audio sample  txing*/
	RSP_TX_AHEADS_DONE,       /*  Indicate heads transfer state of  audio sample  done*/
} E_DMX_SPT_RSP_AHEAD_STATE_T;

/* the headers transfer data for total head , and transfer first sample head and other header in the rsp*/
typedef enum{
	RSP_TXHDRINFO_IDLE = 0,/* Indicate txing hdrinfo initing*/
	RSP_TXHDRINFO_TOTAL_HEAD_TXING,/*  Indicate txing total head*/
	RSP_TXHDRINFO_SAMPLE_HEAD_INIT,/*  Indicate txing total head done and prepare transfer sample head*/
	RSP_TXHDRINFO_SAMPLE_HEAD_TXING,/*  Indicate txing transfering sample head*/
	/*Indicate txing transfering sample head but skip the first one ,only used  for ogm type cfa 's pcm codec*/
	RSP_TXHDRINFO_SKIP_FIRSTSAMPLE_HEAD_TXING,
} E_DMX_SPT_RSP_TXHDR_STATE_T;

typedef struct{
	u32 u4StrmID;
	u32 u4AvailLogCnt;
	u64 u8AvailLogSize;
	u8  ucRspTxType;
	bool   fgUsed;
} RSP_STRM_INF;

typedef struct SPT_RSP_T {
	u32 u4StreamUID; /* TODO: Add Log of Stream UID */

	u64 u8CpsOffset;
	u64 u8DivxDRMOffset;
	u32 u4DecLen;
	s16 u2FrameKeyIndex;

	/* for audio 's header*/
	RSP_HDR_MEM_NODE rAHeadInfo;
	E_DMX_SPT_RSP_ENTRY_TYPE_T eFilledType;

	/*Ftr2FifoInfo rTxInfo;*/
	DMX_SPT_DMA2FIFO_INFO_T rTxInfo;

	void *pvAU;            /* Pointer to this Resplitter entry's AU*/
	u16 u2AUSz;          /* this resplitter entry's AU real used size*/

	struct SPT_RSP_T *prPrevEntry;
	struct SPT_RSP_T *prNextEntry;
} SPT_RSP_T;

typedef struct _SPT_RSPOFF_CMDINBUF_T {
	u8  ucRspTxType;
	u8  ucRspMode;
} DMX_SPT_RSPOFF_CMDINBUF_T;

typedef struct _SPT_GETREBUFRANGE_INBUF_T {
	u64 u8RspStartOffset;
	u64 u8RspDelta;
	u64 u8RspStartPts;
} DMX_SPT_GETREBUFRANGE_INBUF_T;

typedef struct _SPT_GETREBUFRANGE_OUTBUF_T {
	u64 u8RspStartOffset;
	u64 u8PbbStartOffset;
	bool   fgNeedRebuf;
} DMX_SPT_GETREBUFRANGE_OUTBUF_T;

MRESULT SplitterRspLogSampleHdr(void *pvSptHdl, RSP_HDR_MEM_NODE *prNode,
	void *pvHdrSrcAddr, u32 u4Size);

MRESULT SplitterRspInit(void *pvSptHdl, DMX_STM_CNT_INFO_T *parStmCnt);

void    SplitterRspUnInit(void *pvSptHdl);

void    SplitterRspClear(void *pvSptHdl);

MRESULT SplitterRspClearLog(void *pvSptHdl);

bool    SplitterRspIsLoging(DMX_SPT_INST_T *prSpt);

bool    SplitterRspIsRsping(DMX_SPT_INST_T *prSpt);

MRESULT SplitterRspSetLogEnable(void *pvSptHdl);

MRESULT SplitterRspSetRspEnable(void *pvSptHdl);

MRESULT SplitterRspGetRspRangeByPts(void *pvSptHdl, u64 u8RspPtsDelta,
	u64 *pu8RspStartOffset, u64 *pu8PbbStartOffset);

MRESULT SplitterRspGetRspRangeByOffset(void *pvSptHdl, u64 u8RspOffsetDelta,
u64 *pu8RspStartOffset, u64 *pu8PbbStartOffset);

MRESULT SplitterRspSetRspEnableByOffset(void *pvSptHdl);

MRESULT SplitterRspTxDone(void *pvSptHdl, u64 u8TotalTxLen);

MRESULT SplitterRspSetLogAu(void *pvSptHdl, void *pvAUInf);

MRESULT SplitterRspSetRspAu(void *pvSptHdl, void *pvAUinf);

MRESULT SplitterRspSetLogTx(void *pvSptHdl, DMX_SPT_DMA2FIFO_INFO_T *prInf);

bool    SplitterRspEntryRemoveable(void *pvSptHdl, u64 u8RspTM,
	SPT_RSP_T *prCurRsp, SPT_RSP_T *prLastRsp, SPT_RSP_T *prNextRsp,
	u32	*pu4RspTxType);

MRESULT SplitterRspSetRspTx(void *pvSptHdl, SPT_RSP_T *prSptRsp);

MRESULT SplitterRspTxFinish(void *pvSptHdl);

MRESULT SplitterRspTxFinish4ReRsp(void *pvSptHdl);

MRESULT SplitterSetRspLastPtxLen(void *pvSptHdl, u64 u8PtxLen);

MRESULT SplitterSetRspTx(void *pvSptHdl, void *pvRspTx);

MRESULT SplitterRspUpdateExtInf(void *pvSptHdl,
	E_DMX_RSP_EXTINFPROC_TYPE_T eProcType,
	SPT_RSP_T	*pvEntryNeedProc);

MRESULT SplitterRspRemoveLog(void *pvSptHdl, SPT_RSP_T *pvRemoveEntry);

MRESULT SplitterRspAddLog(void *pvSptHdl, SPT_RSP_T *pvAddEntry);

MRESULT SplitterSetRspEntryCountDecrease(void *pvSptHdl);

MRESULT SplitterSetRspEntryCountIncrease(void *pvSptHdl);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef DMX_SPT_RSP_H*/



