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

#ifndef DMX_SPLITTER_H
#define DMX_SPLITTER_H

#include "x_typedef.h"
#include "x_dmx.h"
#include "dmx_define.h"
#include "drv_common.h"
#include "dmx_decrypt.h"
#include "dmx_cfa_def.h"
#include "mm_debug.h"
#include "drv_esm_if.h"

typedef enum {
	SPT_DATA_UNDEFINE,	/* /< Undefined stream type */
	SPT_DATA_V,		/* /< Video stream type */
	SPT_DATA_A,		/* /< Audio stream type */
	SPT_DATA_SP,		/* /< Sub Picture type stream type */
	SPT_DATA_SECTION,	/* /< Section stream type */
	SPT_DATA_BUF,		/* /< DMA User's Buffer stream type */
	SPT_DATA_GRD,		/* /< Ground Buffer stream type */
	MAX_SPT_DATA_TYPE_CNT
} E_SPT_DATA_TYPE_T;

typedef enum {
	SPT_PBUFF_UNKNOWN = 0,
	SPT_PBUFF_FILE = 1,
	SPT_PBUFF_ONE_SEG = 2,
	SPT_PBUFF_FULL_SEG = 3,
	SPT_PBUFF_AUDIN = 4,
} E_SPT_PBBUF_TYPE_T;

typedef struct {
	E_SPT_PBBUF_TYPE_T eSptPbuffType;
	__u32 u4PBBufTotalSz;
	__u32 u4PBBufSlotSz;
} DMX_PBBUF_CONFIG_INFO_T;

typedef struct {
	__u32 u4Status;
	__u64 u8FileEndOffset;
	void *pvSptHdl;
} DMX_PBBUF_NODATA_PARAM_T;

typedef struct {
	__u32 u4VidStmCnt;
	__u32 u4AudStmCnt;
	__u32 u4SPStmCnt;
} DMX_STM_CNT_INFO_T;

/*! @name Splitter Interface Constants, enumerations and macros (6.1) */
/*! @{ */
typedef struct {
	bool fgBadInterLeave;
	void *pvSptHdl;
	DMX_PBBUF_CONFIG_INFO_T rPbbufCfgInfo;
} SPT_PARAM_ENABLE;

typedef struct {
	__s32 i4DecryptId;
	void *pvSptHdl;
	DMX_STM_CNT_INFO_T rStmsCnt;
} DMX_PSR_ON_PARAM_T;

typedef struct {
	void *pvStmHdl;
	ESM_IO_BUF_INFO *prEsmParam;
} DMX_STM_MANAGE_AU_T;

typedef struct {
	__u32 u4Fifo;
	void *pvSptHdl;
} DMX_PSR_FIFO_USAGE_T;

typedef struct {
	void *pvSptHdl;
	__u64 u8FileOfst;
} DMX_PSR_FILE_OFST_T;

typedef struct {
	bool fgActived;
	E_DECRYPT_TYPE_T eDecryptType;
} DMX_CHECK_DECRYPT_DEVICE_T;

typedef struct {
	void *pvSptHdl;
	/* (1) If the MW set this flag to be TRUE, this means that: */
	/* A. if the fifo threshold is > 0, we will flush the data in fifo and set the fifo to be empty */
	/* B. if the fifo threshold is 0, we will get the current empty status of the V&A&SP Fifo without flush fifo */
	/* (2) If the MW set the flag to be FALSE, this means that: */
	/* we only get the current empty status of the V&A&SP fifo without flush fifo */
	/* and doesn't care the fifo threshold */
	bool fgFlushFifo;
} SPT_PARAM_FIFO_USAGE;

typedef struct {
	bool fgEmpty;		/* / Indicate whether the fifo availd data size is 0 or AU count is 0 */
	bool fgThresholdEnabled;	/* / Indicated whether the fifo threhold has been enabled */
} SPT_PARAM_FIFO_USAGE_OUTPUT;

typedef struct {
	bool fgDmaAud;
	__s32 i4Rate;
	void *pvSptHdl;
} SPT_PARAM_SET_RATE;

typedef struct {
	void *pvSptHdl;
	bool fgLastMem;
} SPT_PARAM_SET_LASTMEM;

typedef struct {
	__u32 u4StmType;
	__u32 u4StmUID;
	void *pvSptHdl;
	__u64 u8DecSendBufMask;
} STM_PARAM_CREATE;

typedef struct {
	void *pvStmHdl;
	STM_PARAM_CREATE rStmParam;
} DMX_CREATE_STM_PARAM_T;

typedef struct {
	void *pvSptHdl;
	void *pvStmHdl;
} STM_PARAM_DESTROY;

typedef struct {
	__u32 u4Sz;
	void *pvStmHdl;
} STM_PARAM_SET_FIFO_SZ;

typedef struct {
	__u32 u4Threshold;
	void *pvStmHdl;
} STM_PARAM_SET_THRESHOLD;

typedef struct {
	__u32 u4Threshold;
	__u32 u4IrqElmCnt;
	__u32 u4TimeOut;
} SPT_TSDMA_PARAM_ENABLE;

typedef struct {
	__u32 u4Uid;
	void *pvStmHdl;
} STM_PARAM_SET_UID;

typedef struct {
	__u32 u4Type;
	void *pvSptHdl;
} CFA_PARAM_SET_TYPE;

typedef struct {
	__u32 u4ConfigSz;
	void *pvSptHdl;
	void *pvConfig;
} CFA_PARAM_SET_CONFIG;

typedef struct {
	__u32 u4RangeSz;
	void *pvSptHdl;
	void *pvRange;
} CFA_PARAM_SET_RANGE;

#define DMX_CFA_GENERAL_CODE(cfatype, code)			\
		((__u32)\
		 ((__u32)(0x80000000) |					\
		 (__u32)(((cfatype) & 0xFF) << 16) | \
		 (__u32)((code) & 0xffff))			\
		)

typedef enum {
	CFA_GENERAL_UNKNOWN = DMX_CFA_GENERAL_CODE(CFA_TYPE_UNINITIAL, 0),
	CFA_GENERAL_BADINT = DMX_CFA_GENERAL_CODE(CFA_TYPE_UNINITIAL, 1),
	CFA_GENERAL_JUMPINFO = DMX_CFA_GENERAL_CODE(CFA_TYPE_UNINITIAL, 2),
	CFA_AUD_IN_GENRAL_NONE = DMX_CFA_GENERAL_CODE(CFA_TYPE_AUDIN, 0),
	CFA_AUD_IN_GENRAL_CHANNEL = DMX_CFA_GENERAL_CODE(CFA_TYPE_AUDIN, 1),
	CFA_AUD_IN_GENRAL_CODEC = DMX_CFA_GENERAL_CODE(CFA_TYPE_AUDIN, 2),
	CFA_AUD_IN_GENRAL_CHNL_CODEC = DMX_CFA_GENERAL_CODE(CFA_TYPE_AUDIN, 3)
} E_CFA_GENERAL_TYPE_ID_T;

typedef struct {
	__u32 u4CfaQID;
	void *pvSptHdl;
} CFA_PARAM_SET_INQ_TYPE;

typedef struct {
	__u32 u4CfaQID;
	__u32 u4ParamSize;
	void *pvSptHdl;
	void *pvCfaParam;
} CFA_PARAM_GET_INFO, CFA_PARAM_SET_INFO;

#if DMX_SUPPORT_FFRW
typedef enum {
	PBBUF_SLOT_NORMAL,
	PBBUF_SLOT_JUMP,
	PBBUF_SLOT_END
} E_PBBUF_SLOT_HEADER_TYPE_T;

typedef struct {
	__u32 u4ParamSz;
	E_PBBUF_SLOT_HEADER_TYPE_T eType;
	void *pvParam;
} PBBUF_SLOT_HEADER_INFO_T;
#endif				/* DMX_SUPPORT_FFRW */

typedef struct _SEND_BUFFER_TAG {
	__u32 u4BufferSize;
	__u32 u4DataOffset;
	__u32 u4DataSize;
	__u32 u4PlayOffset;
	__u32 u4PlaySize;
	__u32 u4SessionID;
	__u8 *pcBuffer;
	void *pvBuffer;		/* no need to be a pointer */
	__u64 u8SrcOffset;
	__u64 u8AlignedIdx;
	__u64 u8IssueLen;
#if DMX_SUPPORT_FFRW
	PBBUF_SLOT_HEADER_INFO_T rHeader;
#endif				/* DMX_SUPPORT_FFRW */
} SEND_BUFFER;

typedef struct {
	void *pvSptHdl;
	SEND_BUFFER *prBUF;
} PBBUF_PARAM_SEND_BUF;

typedef struct {
	bool fgExitSent;
	PBBUF_PARAM_SEND_BUF rBufParam;
} DMX_PBBUF_SEND_BUF_T;

typedef struct {
	void *pvSptHdl;
	__u32 u4SlotSz;
} PBBUF_PARAM_CHANGE_SLOT_SZ;

/*!
 * @see SPLITTER_SET_TYPE_PTX_RSP_ON
 */
typedef struct {
#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKStart;
#endif

	/*! input parameter */
	bool fgRebuf;		/*! [IN] Rsp has rebuffered or not */
	bool fgByPassSp;	/*! [IN] Rsp not only tx audio, but also tx subpic */

	void *pvSptHdl;
	__u64 u8PtsDelay;	/*! [IN] The delay parsing PTS */

#ifdef MM_ATE_CHECK
	__u32 u4MMATECHKEnd;
#endif
} SPLITTER_PTX_RSP_ON_INFO_T;

typedef enum {
	SPLITTER_PTX_RSP_BY_PTS,
	SPLITTER_PTX_RSP_BY_OFFSET,
} E_SPLITTER_PTX_RSP_MODE_T;

/*!
 * @see SPLITTER_SET_TYPE_PTX_RSP_OFF
 */
typedef struct {
	bool fgCurPbPause;	/*! [IN] If current in pause pb state */
	/* The return message to tell MPC that Rsp really need to transfer or not */
	__u8 ucState;
	/*! [OUT] The return message to tell MPC that Rsp really need to transfer or not */
	__u8 ucRspTxRet;

	__u8 ucRspTxType;	/*! [IN] The data type that Rsp really need to transfer */
	__u8 ucRspTxUid;	/*! [IN] The data Uid  that Rsp really need to transfer */
	__u8 ucRspMode;	/*! [IN] RSP mode: 0->ptx, 1->offset, 2->index */

	/*! input parameter */
	void *pvSptHdl;
} SPLITTER_PTX_RSP_OFF_INFO_T;

/*!
 * @see SPLITTER_GET_TYPE_PTX_REBUFFER_RANGE
 */
typedef struct {
	/*! input parameter */
	bool fgRebuf;

	void *pvSptHdl;
	__u64 u8PtsDelay;	/*! [IN] The delay parsing PTS */
	__u64 u8RspStartPts;
	/*! output result : the output result is valid only when fuction result is no error */
	/*! [OUT] The Re-split Start offset, measured in __u8 (offset is from set range) */
	__u64 u8RspStartOffset;
	/*! [OUT] The PBB current data Start offset, measured in __u8 (offset is from Parser and PBB) */
	__u64 u8PbbStartOffset;
} SPLITTER_PTX_REBUFFER_RANGE_INFO_T;

#define MASK_CFAJUMPINFO_VIDEO				0x00000001
#define MASK_CFAJUMPINFO_AUDIO				0x00000002
#define MASK_CFAJUMPINFO_SUBTITLE			0x00000004
#define MASK_CFAJUMPINFO_CHANGEAUDIORANGE					0x00000008

typedef struct {
	__u32 u4Flags;
	__u64 u8VidValidOfst;	/* file offset of u4AudStartChunkNo */
	__u64 u8AudValidOfst;
	__u64 u8SubValidOfst;
} CfaJumpInfoInTxData;

#ifdef __linux__
typedef struct _DRV_AUDIN_CBS_T {
	void (*pvUpdateRpCb)(uintptr_t ptrRP);
	bool (*pfgAudInIsRaw)(void);
	void (*pvGetAudInParsingInfo)(void *pvAudInPsringInfo);
	__u32 (*pu4GetPsrSessionID)(void);
} DRV_AUDIN_CB_LIST_T;

typedef struct _IDMXPBBUFCALLBACKS_T_ {
	__u32 (*pu4SendSlot)(void *pvDmxTag, SEND_BUFFER *prSendBuffer);
	__u32 (*pi4SubscribeDrvCb)(void *pvDmxTag, DRV_AUDIN_CB_LIST_T *prCbsList);
	__u32 (*pi4UnSubscribeDrvCb)(void *pvDmxTag);
} IDMXPBBUFCALLBACKS_T;
#else
typedef struct _AUDINBUF_POSINFO_T {
	__u32 u4SessionID;	/* Current Session ID */
	uintptr_t ptrVirRP;		/* AudioIn Buffer's Virtual Read Pointer */
	uintptr_t ptrVirWP;		/* AudioIn Buffer's Virtual Write Pointer */
	uintptr_t ptrVirSa;		/* AudioIn Buffer's Virtual Start Address */
	uintptr_t ptrVirEa;		/* AudioIn Buffer's Virtual End Address */
} AUDINBUF_POSINFO_T;
#endif				/* #ifdef __linux__ */

#endif				/* #ifndef DMX_SPLITTER_H */
