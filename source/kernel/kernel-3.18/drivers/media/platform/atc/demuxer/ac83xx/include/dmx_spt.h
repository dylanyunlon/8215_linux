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
 * @file dmx_spt.h
 *
 * @par Project
 *
 * @par Description
 *    Demuxer Splitter Structures, Macros declarations
 *
 * @par Author_Name
 *    Shuhui Zhang
 *
 */

#ifndef DMX_INTERNAL_SPT_H
#define DMX_INTERNAL_SPT_H

#include "x_typedef.h"
#include "drv_common.h"
#ifdef __linux__
#include <media/atc/dmx_decrypt.h>
#else
#include "dmx_decrypt.h"
#endif				/* __linux__ */
#include "dmx_spt_cfa.h"
#include "dmx_def.h"
#include "dmx_sema.h"

#ifdef __cplusplus
extern "C" {

#endif

#define DMX_PSR_WAIT_OFF_MAXTIME     (5000)
#define DMX_PSR_WAIT_OFF_MINTIME     (50)

#define DMX_RSP_WAIT_FINISH_MAXTIME     (15000)
#define DMX_RSP_WAIT_FINISH_MINTIME     (10)

#define DMX_SPT_WAIT_EXIT_MAXTIME   (10000)
#define DMX_SPT_WAIT_EXIT_MINTIME   (10)

#define DMX_RSP_WAIT_MAXTIME 	(5000)

#define DMX_SUSPEND_WAIT_MAXTIME    (5000)

#define DMX_PBBUF_WAIT_MAXTIME  (4000)

/* Splitter Commands Max Count */
#define MAX_SPT_CMD_CNT              (10)

/* Notify */
typedef enum {
	DMX_SPT_NTY_NONE,
	DMX_SPT_NTY_TX_CONTINUE,
	DMX_SPT_NTY_TX_HW_CB,
	DMX_SPT_NTY_TX_END,
	DMX_SPT_NTY_TX_JUMP,
	DMX_SPT_NTY_TX_PAUSE,
	DMX_SPT_NTY_TX_ABORT,
	DMX_SPT_NTY_TX_OFF,
} E_DMX_SPT_NTY_TYPE_T;

typedef enum {
	DMX_CMD_NONE,
	DMX_CMD_PTX_ON,
	DMX_CMD_RSP_OFF,
	DMX_CMD_RSP_REBUF,
	DMX_CMD_RSP_ON,
	DMX_CMD_PTX_PAUSE,
	DMX_CMD_PTX_RESUME,
	DMX_CMD_PTX_OFF,
	DMX_CMD_EXIT,
	DMX_CMD_STM_ENABLE,
	MAX_OF_DMX_CMD_TYPE_CNT
} E_DMX_CMD_TYPE_T;

typedef struct _DMX_CMD_INFO_T {
	bool fgASync;
	u32 u4UsrEvts;
	u32 u4WaitTime;
	MRESULT mrRet;
	u32 u4InBufSz;
	u32 u4OutBufSz;
	u32 u4RefCnt;
	E_DMX_CMD_TYPE_T eCmd;	
	void *pvInBuf;
	void *pvOutBuf;
	struct _DMX_CMD_INFO_T *prHdl;
} DMX_CMD_INFO_T;

/* / Enumerate Splitter Transfer State */
typedef enum {
	SPLITTER_STATE_NONE = 0x0,	/* /< splitter error (Parser error) */
	SPLITTER_STATE_IDLE = 0x1,	/* /< splitter off, task suspending */
	SPLITTER_STATE_RUNING = 0x2,	/* /< splitter on, data transferring, task suspending */
} SplitterState;
typedef enum {
	/* /< splitter tx none state */
	SPLITTER_TX_STATE_NONE = 0x0,
	/* /< splitter for CFA to check if there is any data to transfer */
	SPLITTER_TX_STATE_CHECK = 0x1,
	/* /< splitter for CFA to check if there is any data to transfer */
	SPLITTER_TX_STATE_TXING = 0x2,
	/* /< splitter paused, Parser may also paused, task suspending */
	SPLITTER_TX_STATE_PAUSE = 0x3,
	SPLITTER_TX_STATE_RSPOFF = 0x4,	/* /< splitter resplit off */
	SPLITTER_TX_STATE_ABORT = 0x5,	/* /< splitter abort */
	SPLITTER_TX_STATE_JUMP = 0x6,	/* /< splitter Tx Jump */
	SPLITTER_TX_STATE_ERROR = 0x7,	/* /< splitter Error */
} SplitterTxState;
/*! @name Splitter To Filter Internal structure for SPT_DATA_BUF (6.2.1) */
/*! @{ */
typedef struct {
	u64 u8FromFileOfst;
	u64 u8TxLen;
	u8 *pucToAddress;
	u32 *pu4AvailSize;
	bool fgSync;
} DMAInfo;

/*! @} */
typedef struct {
	bool fgDST;	/* notify Parser if this SACD is DST type */
} SACD_INFO_T;
typedef struct {
	bool fgQueryWVC1Mode;	/* /< [IN] Query WVC1 Mode or not */
} WVC1_INFO_T;

/* Extra format-specific information that needs to bypassed from CFA to Pfr */
typedef union _exinf {
	SACD_INFO_T rSACD;	/* /< Sacd type information */
	WVC1_INFO_T rWVC1;	/* /< WMV VC1 type information */
} EXINF;

typedef struct {
	u64 u8FromFileOfst;
	void *pvFromAddress;
	u64 u8TxLen;
	u32 u4TxStreamType;
	void *pvToAddress;
	u32 u4TxVideoCodec;
	u32 u4TxPictureMode;
	u64 u8PtsSa;
	u64 u8PtsEa;

	/* MPG */
	u32 u4PackCnt;
	u64 u8TotalAULen;
	u32 u4TxUID;
	bool fgDummyUnit;
	bool fgDummyAUEnd;
	bool fgDummyCmdAU;
	u32 u4TxAudioCodec;
	bool fgAUByEnd;
	bool fgUnitEnd;
	bool fgCreateAU;
	EXINF rExInf;
	bool fgUseCmdQ;
	bool fgAUByCmdQEnd;
	u16 u2TxEntryCnt;
	u64 u8RealTxLen;
	DMX_CMDQ_TX_ENTRY_T *parCmdQTxEntry;

#if ENABLE_DMX_ADVANCED_VER
	bool fgInsertHdr;
	u32 u4InsertHdrLen;
	u8 *pu1InsertHdrBuf;

#endif	/* ENABLE_DMX_ADVANCED_VER */
} DMX_SPT_DMA2FIFO_INFO_T;
typedef struct _RSP_HDR_MEM_NODE {
	void *pvAddr;
	u32 u4Size;
	struct _RSP_HDR_MEM_NODE *prPrev;
	struct _RSP_HDR_MEM_NODE *prNext;
} RSP_HDR_MEM_NODE;
typedef struct _RSP_HDR_MEM_LIST {
	bool fgHdrRpprior;	/* true: HdrRp> HdrWp  false: HdrRp <= HdrWp */
	RSP_HDR_MEM_NODE *prHead;
	RSP_HDR_MEM_NODE *prTail;
	void *pvSa;	/* /< Sa of hdr Entrys fifo, and size just 64X1024 */
	void *pvEa;	/* /< Ea of hdr Entrys fifo */
	void *pvHdrRp;	/* pointer to Free memory begin position */
	void *pvHdrWp;	/* pointer to alloc memory begin position */
} RSP_HDR_MEM_LIST;

	    /*! @name Splitter Structures (6.2) */

/*! @{ */
/* / Splitter private instance data, sizeof(DMX_SPT_INST_T) : ~550 */
typedef struct {
	/* / Splitter Key Information */
	bool fgCreated;	/* /< This element indicated instance "created or not". */
	bool fgExitThread;

	/* / Splitter Connection Information */
	bool fgEnable;	/* /< this instance is enable */

	bool fgSptPauseWithDone;	/* /< Pause with parser tx done */

	bool fgPtxBusy;	/* /< Indicate Parser's last operation finish or not */

	bool fgTxGrd;
	bool fgCfaPrsEnd;	/* /< indicate cfa is parsing end */

	bool fgRspStart;	/* /< Flag for rsp off for parser */
	bool fgRspRebuf;	/* /< Flag for rsp has rebuffered, for CPS issue */

	/* /< Repeat resplitter the same stream which the stream is in resplittering */
	bool fgReRsp;

	bool fgRspEnable;
	
	/* Only can RspTx AU_Start or PTS unit, skip the partial AU. 080417 */
	bool fgAUCtrl;

	/* Indicate the cps service is turn on or not. */
	bool fgCPSOn;

	/* Indicate the cps service is turn on or not. */
	bool fgDivxDRMOn;
	/* Designate whether to dma audio data into fifo or virtual dma,  TRUE: Real-DMA, FALSE: Virtual-DMA */
	bool fgDmaAud;

	/* / Re-Split Related */
	u8 ucRspTxAStmHdrState;
	/* /< indicate the header transfer stage for Audio(such as AAC ) sample, 0: init, 1: txing, 2: done */
	u8 ucRspTxHdrState;	/* /< Rsp Tx Header State */
	u8 ucRspMode;	/* /< rsp mode (0:pts, 1:offset, 2:index) */
	
	u8 ucAudMaxDuration;	/* /< Maximum audio au duration in current title. */
	u8 ucRspTxType;	/* /< Resplit data type */

	/* /< The DivxDRM related */
	u16 u2FrameKeyIndex;
	u16 u2RspAuSize;	/* /< Resplit entry AU size = MAX(sizeof(Audio AU), sizeof(SP AU)) */
	/* / Splitter Key Information */
	u32 u4SptCompId;	/* /< Splitter Component Id of this instance */
	
	/* /< Connected Streams Handle Array of this splitter */
	u32 u4StmHandleNs;	/* /< connected stream count */

	/* /< PBBuf component Type. PlayBackBuffer handle of this instance */
	u32 u4PBBCompType;
	/* /< PBBuf component ID. PlayBackBuffer handle of this instance */
	u32 u4PBBCompId;

	/* /< Indicate transfer to User's Buffer or stream FIFO type, SptDataType */
	u32 u4PtxToStreamType;

	/* /< The transfer data's video codec. SptVideoCodec */
	u32 u4PtxVideoCodec;

	/* /< For Parser Picuture dectect header mode. SptPictureType */
	u32 u4PtxPictureMode;
	/* /< The DivxDRM related */
	u32 u4DecLen;
	/* / CFA Related Information 81 */
	u32 u4CfaType;	/* /< CFA type */
	
	u32 u4PsrEndStatus;           ///< Eos Status
	u32 u4RspState;	/* /< Sub State Machine for Re-Split */
	u32 u4RspSID;	/* /< Re-Split Working SID */
	u32 u4RepeatErrChkCnt;
	u32 u4RspEntrySID;	/* /< Entry Serial ID */
	u32 u4RspEntryCount;	/* /< Entry Counter */
	u32 u4RspStreamUID;	/* /< Resplit Stream UID */
	u32 u4RspCount;	/* /< Resplit entry count in Resplit Entrys fifo--pvSa */
	u32 u4RspEntryMax;
	u32 u4RspDrop;	/* /< Resplit entry drop count */
	u32 u4CmdRdIdx;
	u32 u4CmdWrIdx;
	
	/* Play Rate */
	s32 i4Rate;
	s32 i4DecryptId;
	
	int	waitexitqueueflag;
	wait_queue_head_t  waitexitqueue;
	
	SplitterState eSptState;	/* /< Main State Machine, @see SplitterState */
	SplitterTxState eSptTxState;	/* /< Main State Machine, @see SplitterState */

	/* / Splitter OS Information */
	HANDLE_T hSptTask;	/* /< pointer to splitter task handle. */
	HANDLE   u4SptSemaphore;	/* /< pointer to splitter semaphore handle. */
	HANDLE_T u4SptEvent;	/* /< pointer to splitter event handle. */
	HANDLE_T u4SptTimer;	/* /< pointer to splitter Timer handle. */

	HANDLE rCmdCrits;
	void *pvDmxInst;

	/* / Filter Related Information */
	/* / The filter handle can map to the following instance */
	/* / Video filter handle of this instance */
	/* / Audio filter handle of this instance */
	/* / Subpicture filter handle of this instance */
	/* / Txt Subtitle filter handle of this instance */
	/* / NV filter handle of this instance */
	void *pvStmHandles[MAX_SPT_STM_CONNECTED];
	
	void *pvOtherAudioSptHandle;	/* /< Other audio-only splitter handle */

	/* / PlayBack Buffer Related Information */
	void *pvPBBuf;	/* /< Handle of PBBuf connect to this Splitter Instance. */

	/* / Parser Transfer Related Information 73 */
	void *pvPsrCC;	/* /< Parser handle of this instance */

	/* / Splitter User (Mainly for MPC) Related Information */
	/* /< pointer to MPC event handle. (This is only used by splitter) */
	u32 u4UsrEvent;

	/* /< Indicate transfer from PlayBack Buffer or User's Buffer  */
	void *pvPtxFromDramAddress;

	/* /< Indicate transfer to Dram's address if it is user's buffer */
	void *pvPtxToDramAddress;

	/* /< CfaIntf *prCfaIntf;	//< specific file demuxer interface function */
	void *pvCfaInterface;
	void *pvCfaPrivateData;	/* /< format analyzer private data */

	void *pvRspTx;	/* /< Re-Split Working Pointer */
	
	void *pvRspFirstTx;	/* /< Read Pointer for TX, AU */
	void *pvRspLastTx;	/* /< Write Pointer for TX, AU */

	void *pvSampleHdrBufSa;	/* /< Audio & SP Sample header buffer */
	void *pvRspWp;	/* /< Resplit latest write entry end position in Resplit Entrys fifo--pvSa */
	void *pvRspRp;	/* /< Resplit latest read entry end position in Resplit Entrys fifo--pvSa */
	/* /< Sa of Resplit Entrys fifo, its each entry size is*/
	/*sizeof(SPT_RSP_T) + MAX(sizeof(Audio AU), sizeof(SP AU)) */
	void *pvRspAUTblSa;

	void *pvRspAUTblEa;	/* /< Ea of Resplit Entrys fifo */
	void *pvRspCurPtr;	/* /< Resplit latest write entry start position */
	
	void *pvTempPsrAu;	/* /< Resplit parser AU, pointer to a PSR_AU */
	/* /< Resplit AU, pointer to a RSP_AU, whose size is MAX(sizeof(Audio AU), sizeof(SP AU)) */
	void *pvTempAu;

	void *pvRspExtInfSa;	/* /<RSP_STRM_INF Array */

	/* /< PBBuf Start Offse, We use it to save current PBB offset, Rsp need this*?*/
	/*but after parser off, we can not get it */
	u64 u8PBBOffsetSa;
	
	u64 u8OtherAudioPts;	/* /< Other audio-only present time start time */

	u64 u8PtxFromFileOffset;	/* /< start file offset of current transfer (u8) */
	
	/* /< transfer length of current transfer (u8) */
	u64 u8PtxLen;

	/* /< The transfer data's present time end time */
	u64 u8PtxPtsEa;

	/* /< The DivxDRM offset */
	u64 u8DivxDRMOffset;

	u64 u8RspLastPtxLen;	/* /< Previous Ptx Len */

	u64 u8RspStartPts;	/* /< Resplit Start Pts */
	u64 u8RspPtsDelta;	/* /< Resplit PTS delta */
	u64 u8RspOffset;	/* /< Resplit offset delta */
	u64 u8RspOffsetDelta;	/* /< Resplit offset delta */
	/* save the pure audio au pts */
	u64 u8PureAudPts;
	u64 u8TxLenBkp;

	u64 u8FileEndOffset;

	RSP_HDR_MEM_LIST rHdrMemList;
	DMX_CMD_INFO_T arCmdInfo[MAX_SPT_CMD_CNT];
} DMX_SPT_INST_T;
typedef struct {
	bool fgInit;
	bool fgUsed;
	bool fgSptInitial;
	bool fgBadInterLeave;
	bool fgPsrOff;
	u32 u4SptInstsCnt;
	uintptr_t ptrDmxInst;
	DMX_SPT_INST_T * aprSptInst[MAX_SPT_INST_CNT_PER_DMX];
} DMX_MAN_INFO_T;

typedef struct {
	bool fgSptInitial;
	DMX_SPT_INST_T * aprSptInst[DMX_MAX_SPT_INST_CNT];
}DMX_SPT_MAN_INFO_T;


#define SplitterIsTaskRunning(pvSptHdl)                                            \
	 ((NULL != (pvSptHdl)) ? (FALSE == ((DMX_SPT_INST_T *)(pvSptHdl))->fgExitThread) : FALSE)
#define SplitterIsEnable(pvSptHdl)                                            \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->fgEnable) : FALSE)
#define SplitterRspIsEnabled(pvSptHdl)                                        \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->fgRspEnable) : FALSE)
#define SplitterGetPtxHandle(pvSptHdl)                                        \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->pvPsrCC) : NULL)
#define SplitterGetCfaInterface(pvSptHdl)                                    \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->pvCfaInterface) : NULL)
#define SplitterGetCfaPrivateData(pvSptHdl)                                    \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->pvCfaPrivateData) : NULL)
#define SplitterGetSptTask(pvSptHdl)                                        \
	    ((NULL != (pvSptHdl)) ? &(((DMX_SPT_INST_T *)(pvSptHdl))->ptrSptTask) : NULL)
#define SplitterGetState(pvSptHdl)                                             \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->eSptState) : SPLITTER_STATE_NONE)
#define SplitterGetPBBCompType(pvSptHdl)                                    \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->u4PBBCompType) : 0)
#define SplitterGetPBBCompId(pvSptHdl)                                        \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->u4PBBCompId) : 0)
#define SplitterGetPBBOffsetSa(pvSptHdl)                                    \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->u8PBBOffsetSa) : 0)
#define SplitterGetSptSemaphore(pvSptHdl)                                    \
	    ((NULL != (pvSptHdl)) ? &(((DMX_SPT_INST_T *)(pvSptHdl))->u4SptSemaphore) : NULL)
#define SplitterGetSptEvent(pvSptHdl)                                        \
	    ((NULL != (pvSptHdl)) ? &(((DMX_SPT_INST_T *)(pvSptHdl))->u4SptEvent) : NULL)
#define SplitterGetPBBuf(pvSptHdl)                                            \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->pvPBBuf) : NULL)
#define SplitterGetPtxPixtureMode(pvSptHdl)                                    \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->u4PtxPictureMode) : DMX_INVALID_UINT32)
#define SplitterGetPtxLen(pvSptHdl)                                            \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->u8PtxLen) : DMX_INVALID_UINT64)
#define SplitterGetCfaType(pvSptHdl)                                        \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->u4CfaType) : DMX_INVALID_UINT32)
#define SplitterGetRspFirstTx(pvSptHdl)                                        \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->pvRspFirstTx) : NULL)
#define SplitterGetRspLastTx(pvSptHdl)                                        \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->pvRspLastTx) : NULL)
#define SplitterGetRspTx(pvSptHdl)                                            \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->pvRspTx) : NULL)
#define SplitterGetRspMode(pvSptHdl)                                            \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->ucRspMode) : 0)
#define SplitterGetRspState(pvSptHdl)                                        \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->u4RspState) : DMX_INVALID_UINT32)
#define SplitterGetRspSID(pvSptHdl)                                            \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->u4RspSID) : DMX_INVALID_UINT32)
#define SplitterGetRspLastPtxLen(pvSptHdl)                                    \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->u8RspLastPtxLen) : DMX_INVALID_UINT64)
#define SplitterGetRspStreamUID(pvSptHdl)                                    \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->u4RspStreamUID) : DMX_INVALID_UINT32)
#define SplitterGetRspPtsDelta(pvSptHdl)                                    \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->u8RspPtsDelta) : INVALID_TIMESTAMP)
#define SplitterGetRspStartPts(pvSptHdl)                                    \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->u8RspStartPts) : INVALID_TIMESTAMP)
#define SplitterGetRspOffsetDelta(pvSptHdl)                                   \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->u8RspOffsetDelta) : DMX_INVALID_UINT64)
#define SplitterGetRspStartOffset(pvSptHdl)                                    \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->u8RspOffset) : DMX_INVALID_UINT64)
#define SplitterIsReResplitter(pvSptHdl)                                        \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->fgReRsp) : FALSE)
#define SplitterGetRspTxType(pvSptHdl)                                        \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->ucRspTxType) : 0xFF)
#define SplitterSetPtxNotBusy(pvSptHdl)                                        \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->fgPtxBusy = FALSE) : 0)
#define SplitterSetPauseWithDone(pvSptHdl)                                    \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->fgSptPauseWithDone = TRUE) : 0)
#define SplitterClrPauseWithDone(pvSptHdl)                                    \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->fgSptPauseWithDone = FALSE) : 0)
#define SplitterIsResumeWithDone(pvSptHdl)                                    \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->fgSptPauseWithDone) : FALSE)
#define SplitterIsRspOffStart(pvSptHdl)                                       \
	    ((NULL != pvSptHdl) ? (((DMX_SPT_INST_T *)(pvSptHdl))->fgRspStart) : FALSE)
#define SplitterSetCfaPrivateData(pvSptHdl, pvData)                              \
	    (((DMX_SPT_INST_T *)(pvSptHdl))->pvCfaPrivateData = (pvData))
#define SplitterSetPtxHandle(pvSptHdl, pvPsrHandle)                                     \
	    (((DMX_SPT_INST_T *)(pvSptHdl))->pvPsrCC = (pvPsrHandle))
#define SptSetPBBComp(pvSptHdl, u4Type, u4Id)	do {\
	((DMX_SPT_INST_T *)(pvSptHdl))->u4PBBCompType = (u4Type);\
	((DMX_SPT_INST_T *)(pvSptHdl))->u4PBBCompId = (u4Id);\
} while (0)
#define SplitterSetRspPtsDelta(pvSptHdl, u8DeltaPts)                             \
	 (((DMX_SPT_INST_T *)(pvSptHdl))->u8RspPtsDelta = (u8DeltaPts))
#define SplitterSetRspStartPts(pvSptHdl, u8StartPts)                                \
	    (((DMX_SPT_INST_T *)(pvSptHdl))->u8RspStartPts = (u8StartPts))
#define SplitterSetRspOffsetDelta(pvSptHdl, u8DeltaOffset)                       \
	    (((DMX_SPT_INST_T *)(pvSptHdl))->u8RspOffsetDelta = (u8DeltaOffset))
#define SplitterSetRspStartOffset(pvSptHdl, u8StartOffset)                       \
	    (((DMX_SPT_INST_T *)(pvSptHdl))->u8RspOffset = (u8StartOffset))
#define SplitterSetReResplitter(pvSptHdl, fgRersp)                                \
	    (((DMX_SPT_INST_T *)(pvSptHdl))->fgReRsp = (fgRersp))
#define SplitterSetRspTxType(pvSptHdl, ucType)                                   \
	    (((DMX_SPT_INST_T *)(pvSptHdl))->ucRspTxType = (ucType))
#define SplitterSetRspMode(pvSptHdl, ucMode)                                     \
	    (((DMX_SPT_INST_T *)(pvSptHdl))->ucRspMode = (ucMode))
#define SplitterSetRspState(pvSptHdl, u4State)                                   \
	    (((DMX_SPT_INST_T *)(pvSptHdl))->u4RspState = (u4State))
#define SplitterSetRspOffStart(pvSptHdl, fgStart)                                \
	    (((DMX_SPT_INST_T *)(pvSptHdl))->fgRspStart = (fgStart))
#define SplitterSetRepeatErrChkCnt(pvSptHdl, Cnt)                                     \
	    ((NULL != (pvSptHdl)) ? (((DMX_SPT_INST_T *)(pvSptHdl))->u4RepeatErrChkCnt = Cnt) : 0)
#define SplitterGetPlayRate(pvSptHdl)                                            \
	    ((NULL != (pvSptHdl)) ? ((DMX_SPT_INST_T *)(pvSptHdl))->i4Rate : 1)
#define DMX_IS_RW_PLAY(pvSptHdl)                                                 \
	    MM_IS_RW_PLAY(SplitterGetPlayRate(pvSptHdl))
#define DMX_IS_FF_PLAY(pvSptHdl)                                                 \
	    MM_IS_FF_PLAY(SplitterGetPlayRate(pvSptHdl))
#define DMX_IS_FFRW_PLAY(pvSptHdl)                                               \
	    MM_IS_FFRW_PLAY(SplitterGetPlayRate(pvSptHdl))
#define DMX_IS_NORMAL_PLAY(pvSptHdl)                                             \
	    MM_IS_NORMAL_PLAY(SplitterGetPlayRate(pvSptHdl))
#define SplitterIsEnableDmaAud(pvSptHdl)                                     \
	    ((NULL != pvSptHdl) ? (((DMX_SPT_INST_T *)(pvSptHdl))->fgDmaAud) : FALSE)
#define SplitterIsCfaPsrEnd(pvSptHdl)                                     \
	    ((NULL != pvSptHdl) ? (((DMX_SPT_INST_T *)(pvSptHdl))->fgCfaPrsEnd) : FALSE)
#define SplitterGetPsrEndStatus(pvSptHdl)                                     \
    	    ((NULL != pvSptHdl) ? (((DMX_SPT_INST_T *)pvSptHdl)->u4PsrEndStatus) : 0)
#define SplitterSetDecryptId(pvSptHdl, DecryptId)                            \
	    ((NULL != pvSptHdl) ? (((DMX_SPT_INST_T *)(pvSptHdl))->i4DecryptId = DecryptId) : 0)
#define SplitterGetDecryptId(pvSptHdl)                                       \
	    ((NULL != pvSptHdl) ? (((DMX_SPT_INST_T *)(pvSptHdl))->i4DecryptId) : DECRYPT_PLAY_INVALID_ID)

#define ENABLE_SPTLITTER_LOCK

#ifdef ENABLE_SPTLITTER_LOCK
EXTERN HANDLE g_hDmxManLock;
#define SPLITTER_LOCK_INIT(mrRet)	do {\
	mrRet = dmx_sema_create(&(g_hDmxManLock), DMX_SEMA_TYPE_BINARY, \
		DMX_SEMA_STATE_UNLOCK); \
	if (DMX_FAILED(mrRet)) {\
		DMXLOG_ERROR(\
			TEXT("[DECRYPT] %s line %d, fail in create semaphore, mrRet: 0x%x\r\n"),\
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);\
	} \
} while (0)
#define SPLITTER_LOCK		dmx_sema_lock(g_hDmxManLock, DMX_SEMA_OPTION_WAIT)

#define SPLITTER_UNLOCK		dmx_sema_unlock(g_hDmxManLock)

#define SPLITTER_LOCK_UNINIT(mrRet)	do {\
	dmx_sema_delete(g_hDmxManLock);\
	mrRet = mrRet;\
} while (0)
#else				/* ENABLE_SPTLITTER_LOCK */
#define SPLITTER_LOCK_INIT(mrRet)
#define SPLITTER_LOCK_UNINIT(mrRet)
#define SPLITTER_LOCK
#define SPLITTER_UNLOCK
#endif				/* ENABLE_SPTLITTER_LOCK */

#define STREAM_LOCK(pvSptHdl)   SplitterLockSema(pvSptHdl)
#define STREAM_UNLOCK(pvSptHdl) SplitterReleaseSema(pvSptHdl)

#ifdef __cplusplus
}
#endif

#endif				/* #ifndef DMX_INTERNAL_SPT_H */
