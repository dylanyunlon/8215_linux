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
 * @file dmx_psr_cc.h
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

#ifndef DMX_PSR_CC_H
#define DMX_PSR_CC_H

#include "x_typedef.h"
#ifdef __linux__
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_decrypt.h>
#else
#include "drv_common.h"
#include "dmx_define.h"
#include "dmx_decrypt.h"
#endif /* __linux__*/
#include "drv_ibc.h"
#include "dmx_def.h"
#include "dmx_pbbuf_if.h"
#include "dmx_psr_filter.h"

#ifdef __cplusplus
extern "C" {
#endif

/*/ CC flag*/
#define CCF_USED						(1 << 0)
#define CCF_DATAIN_ENABLE				(1 << 1)
/* Indicate that there are some slots in the Parser CC*/
#define CCF_PBBUF_EXIST					(1 << 2)
#define CCF_CPS_ON						(1 << 3)  /* Encrption related*/
#define CCF_FIFOHOLD					(1 << 4)
#define CCF_PBBUF_RELEASE_NOTIFY		(1 << 5)
#define CCF_TIMEHOLD					(1 << 6)



/* Parser CC State*/
typedef enum {
	CCS_IDLE = 0,	/* Indicate whether Parser CC is in unused*/
	CCS_INIT,		/* indicate whether Parser CC is used, and not in txing*/
	CCS_TX,			/* Indicate whether Parser CC is in TX*/
	CCS_PAUSE,		/* Indicate whether Parser CC is in tx pause*/
	CCS_ABORT		/* Indicate whether Parser CC is in tx abort*/
} PSR_CC_STATE;

/* Parser CC TX State*/
typedef enum {
	TXS_WAIT_PBBUF = 0, /* Wait sync pbbuf's data has been synced*/
	TXS_PBBUF_OK,  /* If the data needed in pbbuf, set this state*/
	TXS_WAIT_FIFO, /* If AU table is full, set this state*/
	/* If the different between Video/Audio/SP and STC excceed
	the threshold, set this state, mainly for AVSync of badinterleave file*/
	TXS_WAIT_VFIFO_PTS_THRESHOLD,
	/* Shile the parser cc's txstate is TXS_WAIT_FIFO or TXS_WAIT_FIFO_PTS_THRESHOLD
	, if the fifo's free space satisfied to tx, set thsi state*/
	TXS_FIFO_OK,
	TXS_WAIT_DECRYPT,		/* no use, decrypt related*/
	TXS_WAIT_HW,			/* If find HW busy(obtain fail) before trigger to tx, set this state*/
	TXS_WAIT_IRQ_PROC,		/* In HW_CB event handler, before call PSR_FILTER_PROC_IRQ, set this state*/
	TXS_TXING,				/* Before trigger HW to tx data, set this state*/
	TXS_TX_OK,				/* If Txdata to fifo process has been complete, set this state*/
	TXS_TX_JUMP
} PSR_TX_STATE;

typedef enum {
	PBBUF_CONTINUITY_UNKNOWN,
	PBBUF_CONTINUOUS,
	PBBUF_UNCONTINUOUS
} E_PBBUF_CONTINUITY_TYPE_T;

/* May be Cmd Q entrys need to tx several times, e.g, total entry count is 50, but first tx 10, next tx 20, ...*/
typedef struct {
	u16 u2CurTxRngEIdx;/* base 0(Current Tx end Entry Index in Cmd Q Entrys array(u4CmdQTxEntryBuffer))*/
	u16 u2CurTxRngSIdx;/* base 0(Current Tx Start Entry Index in Cmd Q Entrys array(u4CmdQTxEntryBuffer))*/
	u32 u4CurTxRngSIdxOfst;	  /* start entry tx start offset(Current Tx Start Entry 's start file offset)*/
	u32 u4CurTxRngSIdxLen;	  /* start entry tx len(Current Tx Start Entry 's tx len)*/
	u32 u4CurTxRngEIdxOfst;	  /* end entry tx start offset(Current Tx end Entry 's start file offset)*/
	u32 u4CurTxRngEIdxLen;	  /* end entry tx len(Current Tx end Entry 's tx len)*/
	
	/* end entry remain len(need tx in future)(may be the Current TX end Entry's real tx len is
	smaller than its real needed tx len, the remained data will be tx next time)*/
	u32 u4CurTxRngEIdxRmnLen;
	
	u64 u8RmnTotalRealTxLen;   /* Only for particular accelerating(Cmd Q total remained data len to tx)*/
} PSR_CMDQ_TX_INF;

/*/ PBBuf Status Information*/
typedef struct {
	bool	 fgPbbufReleaseNotify;/*/< [OUT] Pbbuf is needed to be notified, wakeup must go even if RspOff.*/
	u32 u4ValideDataSize;/*/< [OUT] Valid Total Data Size in PBBUF*/
	u32 u4TotalBufSize;/*/< [OUT] Total PBBUF Size*/
	u32 u4CurSlotDataSize;/*/< the current Txing Pbbufer's current remain data size*/
	u32 u4SlotBufSize; /*/< the Current Txing PBBUF Slot's Buffer Size*/
	u64 u8CurSlotSrcOfst;/*/< FileOfst corresponding to the current Txing Pbbufer's current remain data's sa*/
} PSR_PBBUFInfo;

/*/ CSS/CPRM Decryption Interface Information*/
typedef struct {
	bool	 fgOn;						/*/< [IN] Flag of on/off*/
	u8 bSampleID[8];				/*/< [IN] Sample ID (For WMDRM ND)*/
	u8 bIVData[16];				/*/< [IN] IV Data (Only for Marlin MP4)*/
	u32 u4MaxPacketSize;			 /*/[IN] Max Packet Size (For WMDRM PD)*/
	u32 u4MediaObjSize;			  /*/<[IN] Media Obj Size (For WMDRM ND)*/
	u32 u4IPMPDescriptorID;		/*/< [IN] IPMP_DescriptorID (Only for Marlin MP4)*/
	u32 u4RealSampleSize;			/*/< [OUT] Real Sample Size (Only for Marlin MP4)*/
	u64 u8Offset;					/*/< [IN] Decrypt data start offset*/
	u64 u8DTKC;
	u64 u8DCI_CCI;					/*/< [IN] DCI_CCI (Only in CPRM)*/
	u64 u8DCI_CCI_VERIFY;			/*/< [IN] DCI_CCI verify data h (Only in CPRM)*/
	u64 u8DecLen;					/*/< [IN] Decrypt data length (For SACD)*/
} PSR_CPSInfo;

typedef struct {
	u16 u2FrameKeyIdx;
	u8  *pu1DrmInfo;
	u32 u4ProtectOffset;
	u32 u4ProtectSize;
} PSR_DECRYPT_DIVXDRM_PRIVDATA_T;

#if DMX_DRM_DECRYPT_USE_HW
typedef struct {
	bool   fgCBC;
	u16 u2KeyLen;
	u8  au1InitVector[16];
	u8  au1Key[32];
} PSR_DRM_AES_INFO_T;

typedef enum {
	DECRYPT_NOT_IN_SLOT,
	DECRYPT_WHOLE_IN_SLOT,
	DECRYPT_ACCROSS_SLOTS,
} E_PSR_DECRYPT_INSLOT_TYPE_T;

typedef struct {
	u32 u4DecryptLclOft;
	u32 u4DecryptLclLen;
	u32 u4DecryptTotalLen;
	E_PSR_DECRYPT_INSLOT_TYPE_T eInSlotType;
	PSR_DRM_AES_INFO_T rAudio;
	PSR_DRM_AES_INFO_T rVideo;
} PSR_DRM_INFO_T;

typedef union {
	PSR_DRM_INFO_T rDRM;
} PSR_DECRYPT_HW_PARAM_T;
#endif /* DMX_DRM_DECRYPT_USE_HW*/

typedef enum {
	DECRYPT_UNCOMPLETE,
	DECRYPT_DECRYPTING,
	DECRYPT_COMPLETE,
} E_PSR_DECRYPT_STATUS_T;

typedef enum {
	DECRYPT_BY_NONE,
	DECRYPT_BY_SW,
	DECRYPT_BY_HW,
} E_PSR_DECRYPT_METHOD_T;

typedef struct {
	s32  i4DecryptId;				/*/< Decrypt play id*/
	u32 u4DecryptLen;			/*/< pbbuf decrypt data length*/
	u32 u4AlignSize;				/*/< CPS Align Limitation*/
	uintptr_t ptrTxMemAddr;				/*/< Temp Buffer for Not-align issue*/
	u32 u4TxMemSize;				/*/< Should be the same as Aling limitation*/
	uintptr_t ptrTxMemWPtr;				/*/< Data size in Temp Buffer*/
	uintptr_t ptrTxMemRPtr;				/*/< size of the Data has been tx to fifo*/
	void   *pvInst;					/*/< Decrypt Instance Handle*/
	void  *pvPrivData;				/*/< Decryption Private Data*/
	E_DECRYPT_TYPE_T  eDecryptType; /*/< Decrypt Type*/	
	E_SPT_DATA_TYPE_T eDataType;	/*/< Decrypt Data Type*/
	
	E_PSR_DECRYPT_STATUS_T eStatus; /*/< decrypt status*/
	E_PSR_DECRYPT_METHOD_T eMethod; /*/< decrypt method*/
	u64 u8DecryptStOft;			/*/< pbbuf decrypt start offset*/	
	
#if DMX_DRM_DECRYPT_USE_HW
	PSR_DECRYPT_HW_PARAM_T rHWParam; /*/< Param for using HW decrypt*/
#endif
} PSR_DECRYPT_MAN_T;

/*/ Control center data structure, sizeof(PSR_CC):640*/
typedef struct _PSR_CC {
	bool   fgHaveSubsequentData;	  /*/< Indicate whether have subsequent/remain data to tx*/
	bool   fgVidPass;/*/< TRUE: pass through pbbuf to vfifo at lease once, for VOD data exhausted detection*/
	bool   fgTxMem2Fifo;			/*/< TRUE: Tx Memory data to FIFO*/

	bool   fgWakingWaitTx;			   /*/< For avoid two ESI CB wake up waiting tx @20090613*/
	bool   fgCfaPrsEnd;
	
	bool   fgNeedHighBitRateProc;

	bool   fgLastMem;

	bool   fgUseCmdQ;					/* indicate whether use Cmd Q*/
	bool   fgAUByCmdQEnd;

	bool   fgCurTotalCmdQTxStarted; /* Indicate whether tx cmd Q start*/
	bool   fgChkedAndWaitTx;/* Indicate whether to wait  Cmd Q tx and need to check tx Cmd Q*/
	u16 u2TxEntryCnt;				/* Cmd Q Total Cmd Entry Count*/
	
#if ENABLE_DMX_ADVANCED_VER
	bool   fgInsertHdr;
	u8  au1InsertHdrBuf[DMX_MAX_INST_BYTES_CNT];
	u32 u4InsertHdrLen;
#endif /* ENABLE_DMX_ADVANCED_VER*/

	u32 u4Idx;
  	u32 u4PsrFtrCnt;				/*/< Attached Parser Filter Count*/
	uintptr_t ptrTxCurrSa;				/*/< Current tx start address, base ?*/
	u32 u4Flag;					/*/< Flag of CC*/
	uintptr_t ptrSrcMemSa;				/*/< Tx source is memory, tx source memory's start address*/
	u32 u4SrcMemLen;/*/< Tx source is memory, u4SrcMemLen is total data length to tx*/

	/*/< Tx source is memory, Current Tx memory offset from u4SrcMemSa, because data may
	separate to several packets for unenough fifo free size*/
	u32 u4MemOffset;

	u32	u4TxPBBufIdx;			 /*/< Tx pbbuf index*/

	uintptr_t ptrPTXFifoRdPtr;
	uintptr_t ptrPTXFifoWrPtr;
	uintptr_t ptrPTXSrcSa;
	u32 u4PTXSrcLen;

	void *pvCmdQTxEntryBuffer;	/* pointer to Cmd Q Entry Array*/

	u32	u4TxPBBufJumpIdx;		 /*/< Tx pbbuf index*/

	u32 u4AVStmFlags;
	u32 u4AVStmPlayFlags;
	
	void *pvSptHdl;					/*/< User private data*/
	void *apvFtr[MAX_PSR_FILTER_PER_CC];	/*/< Parser Filter handle array*/
	void *pvActFilter;				/*/< Active Filter Handle*/

	void *pvNormalWaitFtr;			/*/< Normal Tx wait filter handle*/
	void *pvNormalWaitOthFtr;		/*/< Normal Tx wait the other psrcc's filter handle*/

#ifdef PSR_SEMA_ENABLE
	void *pvLock;
#endif /* PSR_SEMA_ENABLE*/

	void *pvDmxInst;

	void *pvPBBuf; 				/*/< Handle of PBBuf for this instance.*/
	
	void   *pvHwData;

	PSR_CC_STATE eState;			/*/< CC state*/
	PSR_TX_STATE eTxState;			/*/< Tx state*/

	u64 u8TxStartOffset;			/*/< Tx start offset, base ?*/
	u64 u8TxLen;					/*/< Tx length, base*/
	
	u64 u8TxCurrOffset;			/*/< Current reading PBBuf slot's tx offset, base ?*/
	u64 u8TxCurrLen;				/*/< Current tx length*/

	#ifdef MM_SUPPORT_DIVXHT31
	u64 u8BaseSTC;
	#endif /* MM_SUPPORT_DIVXHT31*/

	u64 u8NormalWaitPts;			/*/< Normal Tx wait PTS*/

	PSR_CMDQ_TX_INF rCmdQPrevTxInf;
	PSR_CMDQ_TX_INF rCmdQTxInf;
	
	PSR_DECRYPT_MAN_T rDecryptMan;

	DMX_READ_BUFFER arPBBuf[MAX_CACHE_PBBUF];/*/< The PBBuf Slot array, each of them has data to read*/
} PSR_CC;

#ifdef PSR_SEMA_ENABLE
#define PSR_CC_LOCK_INIT(hSema, mrRet)	do { \
	if (NULL == hSema) {\
		if (RET_DMX_OK != (mrRet = dmx_sema_create(&hSema, DMX_SEMA_TYPE_BINARY,\
			DMX_SEMA_STATE_UNLOCK)) { \
			DMXLOG_ERROR((TEXT("[PSR] %s line %d -- Failed to create ")\
				TEXT("sema g_hPSRSema\r\n")),	\
				DMX_FUNC_NAME, DMX_LINE_NO);  \
		}  \
	} \
} while (0)

#define PSR_CC_LOCK(hSema)	do { \
	if (NULL != hSema) \
		dmx_sema_lock(hSema, DMX_SEMA_OPTION_WAIT);\
} while (0)


#define PSR_CC_UNLOCK(hSema)	do { \
	if (NULL != hSema) \
		dmx_sema_unlock(hSema);\
} while (0)


#define PSR_CC_LOCK_UNINIT(hSema)	do { \
	if (NULL != hSema)  {\
		dmx_sema_delete(hSema); \
		hSema = NULL;  \
	} \
} while (0)

#else /* PSR_SEMA_ENABLE*/
#define PSR_CC_LOCK_INIT(hSema, mrRet)
#define PSR_CC_LOCK_UNINIT(hSema)
#define PSR_CC_LOCK(hSema)
#define PSR_CC_UNLOCK(hSema)
#endif /* PSR_SEMA_ENABLE*/

MRESULT PSR_CC_Create(void *pvDmxInst, void *pvSptHdl, void *pvPBBuf, void **ppvPsrCC);

MRESULT PSR_CC_Destroy(void *pvPsrCC);

MRESULT PSR_CC_SetLastMemState(void *pvPsrCC, bool fgLastMem);

MRESULT PSR_CC_Enable(PSR_CC *prPsrCC, bool fgEnable);

MRESULT PSR_CC_MainLoop(void *pvPsrCC);

MRESULT PSR_CC_PauseTx(PSR_CC *prPsrCC, bool *pfgNeedToPause);

MRESULT PSR_CC_ResumeTx(PSR_CC *prPsrCC);

bool	PSR_CC_IsPause(PSR_CC *prPsrCC);

MRESULT PSR_CC_AbortTx(PSR_CC *prPsrCC);

MRESULT	PSR_CC_GetWaitTxBufSize(PSR_CC *prPsrCC, u64 *pu8Sz);

MRESULT PSR_CC_SetTxSt(PSR_CC *prPsrCC, PSR_TX_STATE eTxSt);

MRESULT PSR_CC_SetState(PSR_CC *prPsrCC, PSR_CC_STATE eState);

MRESULT PSR_CC_CheckPBBuf(
		PSR_CC *prPsrCC,
		u64 u8Offset,
		bool   *pfgOffsetIn,
		E_PBBUF_CONTINUITY_TYPE_T *pePbbufCon,
		bool   fgSyncPb);

void	PSR_CC_DumpCurrentState(PSR_CC *prPsrCC);

MRESULT PSR_CC_AttachFilter(PSR_CC *prPsrCC, void *pvPsrFtr);

MRESULT PSR_CC_DetachFilter(PSR_CC *prPsrCC, void *pvPsrFtr);

MRESULT PSR_CC_GetCurrentOffset(PSR_CC *prPsrCC, u64 *pu8Offset);

MRESULT PSR_CC_NotiCfaPrsEnd(PSR_CC *prPsrCC, bool fgCfaPrsEnd);

#if CONFIG_DRV_HIGH_BITRATE_SPECIAL_PROC
MRESULT PSR_CC_NotiCurStrmInf(PSR_CC *prPsrCC, bool fgHighBitrate);
#endif /* CONFIG_DRV_HIGH_BITRATE_SPECIAL_PROC*/

MRESULT PSR_CC_GetWMVParsingMode(PSR_CC *prPsrCC, u8 *pucWMVParsingMode);

MRESULT PSR_CC_ResumeUnfinishTx(PSR_CC *prPsrCC,  bool fgIsPrevBkpSt);

bool	PSR_CC_IsStillWaitSt(PSR_CC *prPsrCC, bool fgIsPrevBkpSt);

bool	PSR_CC_IsUnfinishTx(PSR_CC *prPsrCC, bool fgIsPrevBkpSt);

MRESULT PSR_CC_BkpTxSt(PSR_CC *prPsrCC);

MRESULT PSR_CC_UpdateOccupyInf(PSR_CC *prPsrCC , u32 u4OccupyInf);

MRESULT PSR_CC_Reset4NonConPbbufSlot(PSR_CC *prPsrCC);

MRESULT PSR_CC_RelPbbuf2UnCon(
	PSR_CC *prPsrCC, u32 u4TxPbbufIdx);

MRESULT PSR_CC_RelPbbufAcrossSlot2UnCon(
	PSR_CC *prPsrCC, u32 u4TxPbbufIdx, bool *pfgExistUnCon);

MRESULT PSR_CC_SetDecryptType(PSR_CC *prPsrCC, E_DECRYPT_TYPE_T eDecryptType);

#if DMX_SUPPORT_DIVXDRM
MRESULT PSR_CC_EnableDivxDRMDecrypt(PSR_CC *prPsrCC, PSR_DivxDRMInfo *prPsrDRMInf);
#endif /* ENABLE_DIVXDRM*/

#if DMX_DRM_DECRYPT_USE_HW
MRESULT PSR_CC_CalcHWDecryptOfst(PSR_CC *prPsrCC, PSR_FILTER *prPsrFtr);

MRESULT PSR_CC_CheckHWDecryptStatus(PSR_CC *prPsrCC, PSR_FILTER *prPsrFtr);
#endif /* DMX_DRM_DECRYPT_USE_HW*/

#define PSR_CC_GETOWNER(pvPsrCC)  \
	((NULL != pvPsrCC) ? (((PSR_CC *)pvPsrCC)->pvSptHdl) : NULL)

#ifdef __cplusplus
}
#endif

#endif /* #ifndef DMX_PSR_CC_H*/

