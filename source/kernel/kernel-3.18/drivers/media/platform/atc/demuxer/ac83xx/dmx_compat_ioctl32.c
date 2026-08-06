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
 * @file demuxer.c
 *
 * @par Project
 *	  MT3360
 *
 * @par Description
 *	  Demuxer Os interface layer, demuxer ioctrl definitions
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */


#ifndef DMX_COMPAT_IOCTL32
#define DMX_COMPAT_IOCTL32
 
#include "x_typedef.h"
#include <linux/types.h>

#ifdef CONFIG_COMPAT
 /*
==========================COMPAT 32 BIT=========================
*/
/*
=============================================================
==============================S P L I T T E R======================
=============================================================
*/
/**
 * Create splitter Instance.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_SPT_CREATE
 * pBufIn			 -- NULL.
 * dwLenIn			 -- 0
 * pBufOut			 -- Handle of splitter created.
 * dwLenOut		 -- sizeof(HANDLE)
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed for this function, or else return TRUE.
 * Notes: This should be call before enable splitter.
 */
#define DMX_IOCTL_SPT_CREATE32				_IOR('D', 1, compat_uptr_t)

/**
 * Enable splitter.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_SPT_ENABLE
 * pBufIn			 -- The handle of splitter instance created.
 * dwLenIn			 -- sizeof(HANDLE)
 * pBufOut			 -- NULL
 * dwLenOut		 -- 0
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed for this function, or else return TRUE.
 */
#define DMX_IOCTL_SPT_ENABLE32				_IOW('D', 2, struct SPT_PARAM_ENABLE32)

/**
 * Disnable splitter.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_SPT_DISABLE
 * pBufIn			 -- The handle of splitter instance created.
 * dwLenIn			 -- sizeof(HANDLE)
 * pBufOut			 -- NULL
 * dwLenOut		 -- 0
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed for this function, or else return TRUE.
 */
#define DMX_IOCTL_SPT_DISABLE32				_IOW('D', 3, compat_uptr_t)

/**
 * Check splitter is enabled or not.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_SPT_IS_ENABLED
 * pBufIn			 -- The handle of splitter instance created.
 * dwLenIn			 -- sizeof(HANDLE)
 * pBufOut			 -- NULL
 * dwLenOut		 -- 0
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed for this function, or else return TRUE.
 */
#define DMX_IOCTL_SPT_IS_ENABLED32			_IOW('D', 4, compat_uptr_t)


#define DMX_IOCTL_SPT_SET_RATE32				_IOW('D', 6, struct SPT_PARAM_SET_RATE32)


#define DMX_IOCTL_SPT_DESTROY32				_IOW('D', 11, compat_uptr_t)

/*
=============================================================
==============================S T R E A M========================
=============================================================
*/
/**
 * Create Stream Instance by Stream type and stream UID.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_STM_CREATE
 * pBufIn			 -- The handle of splitter instance created, and Stm Type and Stm UID.
 * dwLenIn			 -- sizeof(HANDLE) + sizeof(__u32) + sizeof(__u32)
 * pBufOut			 -- Handle of Stream Created.
 * dwLenOut		 -- sizeof(HANDLE)
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed for this function, or else return TRUE.
 */
#define DMX_IOCTL_STM_CREATE32				_IOWR('D', 12, struct DMX_CREATE_STM_PARAM_T32)

/**
 * Enable Stream which created.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_STM_ENABLE
 * pBufIn			 -- The handle of stream instance created.
 * dwLenIn			 -- sizeof(HANDLE)
 * pBufOut			 -- NULL.
 * dwLenOut		 -- 0
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed for this function, or else return TRUE.
 */
#define DMX_IOCTL_STM_ENABLE32				_IOW('D', 13, compat_uptr_t)

/**
 * Enable Stream which enabled.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_STM_DISABLE
 * pBufIn			 -- The handle of stream instance created.
 * dwLenIn			 -- sizeof(HANDLE)
 * pBufOut			 -- NULL.
 * dwLenOut		 -- 0
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed for this function, or else return TRUE.
 */
#define DMX_IOCTL_STM_DISABLE32				_IOW('D', 14, compat_uptr_t)

/**
 * Set FIFO size for one stream which enabled.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_STM_SET_FIFO_SZ
 * pBufIn			 -- The handle of stream instance created, and FIFO Size
 * dwLenIn			 -- sizeof(HANDLE) + sizeof(__u32)
 * pBufOut			 -- NULL.
 * dwLenOut		 -- 0
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed for this function, or else return TRUE.
 */
#define DMX_IOCTL_STM_SET_FIFO_SZ32			_IOW('D', 15, struct STM_PARAM_SET_FIFO_SZ32)

/**
 * Set threshold for one stream which enabled.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_STM_SET_THRESHOLD
 * pBufIn			 -- The handle of stream instance created, and Threshold
 * dwLenIn			 -- sizeof(HANDLE) + sizeof(__u32)
 * pBufOut			 -- NULL.
 * dwLenOut		 -- 0
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed for this function, or else return TRUE.
 */
#define DMX_IOCTL_STM_SET_THRESHOLD32			_IOW('D', 16, struct STM_PARAM_SET_THRESHOLD32)

/**
 * Set threshold for one stream which enabled.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_STM_FLUSH_FIFO
 * pBufIn			 -- The handle of stream instance created
 * dwLenIn			 -- sizeof(HANDLE)
 * pBufOut			 -- NULL.
 * dwLenOut		 -- 0
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed for this function, or else return TRUE.
 */
#define DMX_IOCTL_STM_FLUSH_FIFO32			_IOW('D', 17, compat_uptr_t)

#define DMX_IOCTL_STM_GETAU32					_IOWR('D', 18, struct DMX_STM_MANAGE_AU_T32)

#define DMX_IOCTL_STM_RELEASEAU32				_IOW('D', 19, struct DMX_STM_MANAGE_AU_T32)

#define DMX_IOCTL_STM_DESTROY32				_IOW('D', 20, struct STM_PARAM_DESTROY32)

#define DMX_IOCTL_STM_SETUID32				_IOW('D', 21, struct STM_PARAM_SET_UID32)

#define DMX_IOCTL_STM_RELEASE_FF_AUDIOAU32		_IOW('D', 22, struct DMX_STM_MANAGE_AU_T32)

/*
=============================================================
==============================P A R S E R========================
=============================================================
*/
/**
 * Get Video FIFO Usage.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_PSR_VFIFO_USAGE
 * pBufIn			 -- The handle of splitter instance.
 * dwLenIn			 -- sizeof(HANDLE)
 * pBufOut			 -- The usage of V FIFO.
 * dwLenOut		 -- sizeof(__u32)
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed for this function, or else return TRUE.
 */
#define DMX_IOCTL_PSR_VFIFO_USAGE32			_IOWR('D', 25, struct DMX_PSR_FIFO_USAGE_T32)

/**
 * Get Audio FIFO Usage.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_PSR_AFIFO_USAGE
 * pBufIn			 -- The handle of splitter instance.
 * dwLenIn			 -- sizeof(HANDLE)
 * pBufOut			 -- The usage of A FIFO.
 * dwLenOut		 -- sizeof(__u32)
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed for this function, or else return TRUE.
 */
#define DMX_IOCTL_PSR_AFIFO_USAGE32			_IOWR('D', 26, struct DMX_PSR_FIFO_USAGE_T32)

/**
 *
 */
#define DMX_IOCTL_PSR_ON32					_IOW('D', 27, struct DMX_PSR_ON_PARAM_T32)

/**
 *
 */
#define DMX_IOCTL_PSR_OFF32					_IOW('D', 28, compat_uptr_t)

/**
 *
 */
#define DMX_IOCTL_PSR_PAUSE32					_IOW('D', 29, compat_uptr_t)


/**
 * Get Current Parser file offset.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_PSR_FILE_OFST
 * pBufIn			 -- The handle of splitter instance.
 * dwLenIn			 -- sizeof(HANDLE)
 * pBufOut			 -- The offset of parser currently.
 * dwLenOut		 -- sizeof(u64)
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed for this function, or else return TRUE.
 */
#define DMX_IOCTL_PSR_FILE_OFST32				_IOWR('D', 31, struct DMX_PSR_FILE_OFST_T32)


/*
=============================================================
==============================C F A===========================
=============================================================
*/
/**
 * Set CFA Type.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_CFA_SET_TYPE
 * pBufIn			 -- The handle of splitter instance created, and CFA Type
 * dwLenIn			 -- sizeof(HANDLE) + sizeof(__u32)
 * pBufOut			 -- NULL
 * dwLenOut		 -- 0
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed for this function, or else return TRUE.
 * Notes: This should be call before enable splitter.
 */
#define DMX_IOCTL_CFA_SET_TYPE32				_IOW('D', 32, struct CFA_PARAM_SET_TYPE32)

/**
 * Set CFA Config.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_CFA_SET_TYPE
 * pBufIn			 -- The handle of splitter instance created, and CFA Config.
 * dwLenIn			 -- sizeof(HANDLE) + sizeof(CfaMp4ConfigInfo)
 * pBufOut			 -- NULL
 * dwLenOut		 -- 0
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed for this function, or else return TRUE.
 * Notes: This should be call after enable splitter.
 */
#define DMX_IOCTL_CFA_CONFIG32				_IOW('D', 33, struct CFA_PARAM_SET_CONFIG32)

/**
 * Set CFA Range.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_CFA_SET_TYPE
 * pBufIn			 -- The handle of splitter instance created, and CFA Range.
 * dwLenIn			 -- sizeof(HANDLE) + sizeof(CfaMp4Range)
 * pBufOut			 -- NULL
 * dwLenOut		 -- 0
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed for this function, or else return TRUE.
 * Notes: This should be call after enable splitter.
 */
#define DMX_IOCTL_CFA_SET_RANGE32				_IOW('D', 34, struct CFA_PARAM_SET_RANGE32)

/**
 *
 */
#define DMX_IOCTL_CFA_SET_INQUIRE_TYPE32		_IOW('D', 35, struct CFA_PARAM_SET_INQ_TYPE32)


/**
 *
 */
#define DMX_IOCTL_CFA_SET_GEN32				_IOW('D', 37, struct CFA_PARAM_SET_INFO32)


/**
 *
 */
#define DMX_IOCTL_CFA_GET_POSI32				_IOWR('D', 39, struct DMX_PSR_FILE_OFST_T32)

/*
=============================================================
==============================P B B U F=========================
=============================================================
*/


/**
 * Alloc pbbuf for send.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_PBBUF_ALLOC_BUF
 * pBufIn			 -- The handle of splitter instance created.
 * dwLenIn			 -- sizeof(HANDLE)
 * pBufOut			 -- The pointer of send buffer.
 * dwLenOut		 -- sizeof(SEND_BUFFER)
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed to alloc pb buffer(busy), or else return TRUE.
 */
#define DMX_IOCTL_PBBUF_ALLOC_BUF32			_IOWR('D', 42, struct PBBUF_PARAM_SEND_BUF32)

/**
 * Cancel alloc pbbuf.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_PBBUF_CANCEL_BUF
 * pBufIn			 -- The handle of splitter instance created.
 * dwLenIn			 -- sizeof(HANDLE)
 * pBufOut			 -- NULL
 * dwLenOut		 -- 0
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed for this function, or else return TRUE.
 * Notes: After user called alloc buf function and failed. User should call this
 *				function when discard alloc.
 */
#define DMX_IOCTL_PBBUF_CANCEL_BUF32			_IOW('D', 43, compat_uptr_t)

/**
 * Send pbbuf.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_PBBUF_SEND_BUF
 * pBufIn			 -- The handle of splitter instance created and pointer of send buffer.
 * dwLenIn			 -- sizeof(HANDLE) + sizeof(SEND_BUFFER)
 * pBufOut			 -- NULL.
 * dwLenOut		 -- 0
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed for this function, or else return TRUE.
 * Notes: Before this function, user should be call alloc function first.
 *				And this function no busy mode, it means return success always.
 */
#define DMX_IOCTL_PBBUF_SEND_BUF32			_IOW('D', 44, struct DMX_PBBUF_SEND_BUF_T32)

/**
 * Release pbbuf.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_PBBUF_RELEASE_BUF
 * pBufIn			 -- The handle of splitter instance created and the pointer of sent buffer.
 * dwLenIn			 -- sizeof(HANDLE) + sizeof(SEND_BUFFER)
 * pBufOut			 -- NULL
 * dwLenOut		 -- 0
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed for this function, or else return TRUE.
 * Notes: After user sent pbbuf and successful, but now user wish discard it,
 *				user should be call this function.
 */
#define DMX_IOCTL_PBBUF_RELEASE_BUF32			_IOW('D', 45, struct PBBUF_PARAM_SEND_BUF32)


#define DMX_IOCTL_PBBUF_NODATA32				_IOW('D', 51, struct DMX_PBBUF_NODATA_PARAM_T32)

/*
============================================================
==============================R E S P L I T T E R==================
===========================================================
 *
 * @brief turn on demuxer
 *
 * We use this to turn on demuxer.
 */
#define DMX_IOCTL_RSP_ON32					_IOW('D', 52, struct SPLITTER_PTX_RSP_ON_INFO_T32)

/*!
 * @brief turn off demuxer
 *
 * We use this to turn off demuxer.
 */

#define DMX_IOCTL_RSP_OFF32					_IOWR('D', 53, struct SPLITTER_PTX_RSP_OFF_INFO_T32)
/*!
 * @brief Get the information report by Parser for Playback Buffer file offset start.
 *
 * It is for data rebuffer
 * We use this to get current pbb start file offset (__u8).
 */
#define DMX_IOCTL_REBUFFER_RANGE32			_IOWR('D', 54, struct SPLITTER_PTX_REBUFFER_RANGE_INFO_T32)

/*
=============================================================
================================= CLI =========================
=============================================================
*/

#define DMX_IOCTL_IS_BUSY32                           _IOR('D', 56, compat_uptr_t)

/*
=============================================================
================================= DECRYPT =====================
=============================================================
*/
/* Input:  DECRYPT_OPER_PARAM_T*/
/* Output: according to the command*/
#define DMX_IOCTL_DECRYPT_EXEC_CMD32					_IOW('D', 59, struct DECRYPT_OPER_PARAM_T32)

/* Input:  rParam.eDecryptType, rParam.pvInst = pvInst, */
/* rParam's Type: DECRYPT_INST_PARAM_T*/
/* Output: NULL*/
#define DMX_IOCTL_DECRYPT_RELEASE_INST32			_IOW('D', 60, struct DECRYPT_INST_PARAM_T32)


/* Input:  rParam.eDecryptType, rParam.pvInst = pvInst, */
/*				 rParam's Type: DECRYPT_INST_PARAM_T*/
/* Output: MRESULT, defined in dmx_define.h*/
#define DMX_IOCTL_DECRYPT_GET_LAST_ERROR32			_IOW('D', 64, struct DECRYPT_INST_PARAM_T32)


/*
============================================================
=====================IOCTL PARAMETER(arg)========================
============================================================
*/


typedef struct {
	bool fgBadInterLeave;
	compat_uptr_t ptrSpt;
	DMX_PBBUF_CONFIG_INFO_T rPbbufCfgInfo;
} SPT_PARAM_ENABLE32;

typedef struct {
	bool fgDmaAud;
	__s32 i4Rate;
	compat_uptr_t ptrSpt;
} SPT_PARAM_SET_RATE32;

typedef struct {
	__u32 u4StmType;
	__u32 u4StmUID;
	compat_uptr_t ptrSpt;
	__u64 u8DecSendBufMask;
} STM_PARAM_CREATE32;

typedef struct {
	compat_uptr_t ptrStmHdl;
	STM_PARAM_CREATE32 rStmParam;
} DMX_CREATE_STM_PARAM_T32;

typedef struct {
	__u32 u4Sz;
	compat_uptr_t ptrStm;
} STM_PARAM_SET_FIFO_SZ32;

typedef struct {
	__u32 u4Threshold;
	compat_uptr_t ptrStm;
} STM_PARAM_SET_THRESHOLD32;

typedef struct {
	compat_uptr_t ptrStmHdl;
	compat_caddr_t prEsmParam;
} DMX_STM_MANAGE_AU_T32;

typedef struct {
	compat_uptr_t ptrSpt;
	compat_uptr_t ptrStm;
} STM_PARAM_DESTROY32;

typedef struct {
	__u32 u4Uid;
	compat_uptr_t ptrStm;
} STM_PARAM_SET_UID32;

typedef struct {
	u32 u4Fifo;
	compat_uptr_t ptrSpt;
} DMX_PSR_FIFO_USAGE_T32;

typedef struct {
	__s32 i4DecryptId;
	compat_uptr_t ptrSpt;
	DMX_STM_CNT_INFO_T rStmsCnt;
} DMX_PSR_ON_PARAM_T32;

typedef struct {
	compat_uptr_t ptrSptHdl;
	u64 u8FileOfst;
} DMX_PSR_FILE_OFST_T32;

typedef struct {
	__u32 u4Type;
	compat_uptr_t ptrSpt;
} CFA_PARAM_SET_TYPE32;

typedef struct {
	__u32 u4ConfigSz;
	compat_uptr_t ptrSpt;
	compat_caddr_t pvConfig;
} CFA_PARAM_SET_CONFIG32;

typedef struct {
	__u32 u4RangeSz;
	compat_uptr_t ptrSpt;
	compat_caddr_t pvRange;
} CFA_PARAM_SET_RANGE32;

typedef struct {
	__u32 u4CfaQID;
	compat_uptr_t ptrSpt;
} CFA_PARAM_SET_INQ_TYPE32;

typedef struct {
	__u32 u4CfaQID;
	__u32 u4ParamSize;
	compat_uptr_t ptrSpt;
	compat_caddr_t pvCfaParam;
} CFA_PARAM_GET_INFO32, CFA_PARAM_SET_INFO32;

typedef struct {
	bool fgExitSent;
	PBBUF_PARAM_SEND_BUF32 rBufParam;
} DMX_PBBUF_SEND_BUF_T32;


typedef struct {
	compat_uptr_t ptrSpt;
	compat_caddr_t prBUF;
} PBBUF_PARAM_SEND_BUF32;

typedef struct {
	__u32 u4Status;
	compat_uptr_t ptrSpt;
} DMX_PBBUF_NODATA_PARAM_T32;

typedef struct {
	/*! input parameter */
	bool fgRebuf;		/*! [IN] Rsp has rebuffered or not */
	bool fgByPassSp;	/*! [IN] Rsp not only tx audio, but also tx subpic */

	compat_uptr_t ptrSpt;
	__u64 u8PtsDelay;	/*! [IN] The delay parsing PTS */

} SPLITTER_PTX_RSP_ON_INFO_T32;

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
	compat_uptr_t ptrSpt;

} SPLITTER_PTX_RSP_OFF_INFO_T32;

typedef struct {
	/*! input parameter */
	bool fgRebuf;

	compat_uptr_t ptrSpt;
	__u64 u8PtsDelay;	/*! [IN] The delay parsing PTS */
	__u64 u8RspStartPts;
	/*! output result : the output result is valid only when fuction result is no error */
	/*! [OUT] The Re-split Start offset, measured in __u8 (offset is from set range) */
	__u64 u8RspStartOffset;
	/*! [OUT] The PBB current data Start offset, measured in __u8 (offset is from Parser and PBB) */
	__u64 u8PbbStartOffset;

} SPLITTER_PTX_REBUFFER_RANGE_INFO_T32;

typedef struct {
	/* DivxDRM Instance Handle, DMX_IOCTL_DECRYPT_CREATE_INST's output param */
	__u32 u4OperParamSz;	/* Operation Input Param Size */
	__u32 u4OperCode;	/* Operation Code */
	compat_caddr_t pvInst;
	compat_caddr_t pvOperParam;	/* Operation Input Param */
} DECRYPT_OPER_PARAM_T32;

typedef struct {
	E_DECRYPT_TYPE_T eDecryptType;	/* Decrypt Type */
	/* DivxDRM Instance Handle, DMX_IOCTL_DECRYPT_CREATE_INST's output param */
	compat_caddr_t pvInst;
} DECRYPT_INST_PARAM_T32;

typedef struct {
	__u32 u4ParamSz;
	E_PBBUF_SLOT_HEADER_TYPE_T eType;
	compat_caddr_t pvParam;
} PBBUF_SLOT_HEADER_INFO_T32;

typedef struct {
	__u32 u4BufferSize;
	__u32 u4DataOffset;
	__u32 u4DataSize;
	__u32 u4PlayOffset;
	__u32 u4PlaySize;
	__u32 u4SessionID;
	compat_caddr_t pcBuffer;
	uintptr_t ptrBuffer;		/* no need to be a pointer */
	__u64 u8SrcOffset;
	__u64 u8AlignedIdx;
	__u64 u8IssueLen;
#if DMX_SUPPORT_FFRW
	PBBUF_SLOT_HEADER_INFO_T32 rHeader;
#endif	
} SEND_BUFFER32;


/*===============================================================*/
static long dmx_native_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = -ENOIOCTLCMD;

	if (file->f_op->unlocked_ioctl)
		ret = file->f_op->unlocked_ioctl(file, cmd, arg);

	return ret;
}

static unsigned int dmx_cmd_switch(unsigned int cmd)
{
	unsigned int rCmd = 0;
	
	switch(cmd){
	case DMX_IOCTL_SPT_CREATE32: rCmd = DMX_IOCTL_SPT_CREATE; break;
	case DMX_IOCTL_SPT_ENABLE32: rCmd = DMX_IOCTL_SPT_ENABLE; break;
	case DMX_IOCTL_SPT_DISABLE32: rCmd = DMX_IOCTL_SPT_DISABLE; break;
	case DMX_IOCTL_SPT_IS_ENABLED32: rCmd = DMX_IOCTL_SPT_IS_ENABLED; break;
	case DMX_IOCTL_SPT_SET_RATE32: rCmd = DMX_IOCTL_SPT_SET_RATE; break;
	case DMX_IOCTL_SPT_DESTROY32: rCmd = DMX_IOCTL_SPT_DESTROY; break;
	case DMX_IOCTL_STM_CREATE32: rCmd = DMX_IOCTL_STM_CREATE; break;
	case DMX_IOCTL_STM_ENABLE32: rCmd = DMX_IOCTL_STM_ENABLE; break;
	case DMX_IOCTL_STM_DISABLE32: rCmd = DMX_IOCTL_STM_DISABLE; break;
	case DMX_IOCTL_STM_SET_FIFO_SZ32: rCmd = DMX_IOCTL_STM_SET_FIFO_SZ; break;
	case DMX_IOCTL_STM_SET_THRESHOLD32: rCmd = DMX_IOCTL_STM_SET_THRESHOLD; break;
	case DMX_IOCTL_STM_FLUSH_FIFO32: rCmd = DMX_IOCTL_STM_FLUSH_FIFO; break;
	case DMX_IOCTL_STM_GETAU32: rCmd = DMX_IOCTL_STM_GETAU; break;
	case DMX_IOCTL_STM_RELEASEAU32: rCmd = DMX_IOCTL_STM_RELEASEAU; break;
	case DMX_IOCTL_STM_DESTROY32: rCmd = DMX_IOCTL_STM_DESTROY; break;
	case DMX_IOCTL_STM_SETUID32: rCmd = DMX_IOCTL_STM_SETUID; break;
	case DMX_IOCTL_STM_RELEASE_FF_AUDIOAU32: rCmd = DMX_IOCTL_STM_RELEASE_FF_AUDIOAU; break;
	case DMX_IOCTL_PSR_VFIFO_USAGE32: rCmd = DMX_IOCTL_PSR_VFIFO_USAGE; break;
	case DMX_IOCTL_PSR_AFIFO_USAGE32: rCmd = DMX_IOCTL_PSR_AFIFO_USAGE; break;
	case DMX_IOCTL_PSR_ON32: rCmd = DMX_IOCTL_PSR_ON; break;
	case DMX_IOCTL_PSR_OFF32: rCmd = DMX_IOCTL_PSR_OFF; break;
	case DMX_IOCTL_PSR_PAUSE32: rCmd = DMX_IOCTL_PSR_PAUSE; break;
	case DMX_IOCTL_PSR_FILE_OFST32: rCmd = DMX_IOCTL_PSR_FILE_OFST; break;
	case DMX_IOCTL_CFA_SET_TYPE32: rCmd = DMX_IOCTL_CFA_SET_TYPE; break;
	case DMX_IOCTL_CFA_CONFIG32: rCmd = DMX_IOCTL_CFA_CONFIG; break;
	case DMX_IOCTL_CFA_SET_RANGE32: rCmd = DMX_IOCTL_CFA_SET_RANGE; break;
	case DMX_IOCTL_CFA_SET_INQUIRE_TYPE32: rCmd = DMX_IOCTL_CFA_SET_INQUIRE_TYPE; break;
	case DMX_IOCTL_CFA_SET_GEN32: rCmd = DMX_IOCTL_CFA_SET_GEN; break;
	case DMX_IOCTL_CFA_GET_POSI32: rCmd = DMX_IOCTL_CFA_GET_POSI; break;
	case DMX_IOCTL_PBBUF_ALLOC_BUF32: rCmd = DMX_IOCTL_PBBUF_ALLOC_BUF; break;
	case DMX_IOCTL_PBBUF_CANCEL_BUF32: rCmd = DMX_IOCTL_PBBUF_CANCEL_BUF; break;
	case DMX_IOCTL_PBBUF_SEND_BUF: rCmd = DMX_IOCTL_PBBUF_SEND_BUF; break;
	case DMX_IOCTL_PBBUF_RELEASE_BUF32: rCmd = DMX_IOCTL_PBBUF_RELEASE_BUF; break;
	case DMX_IOCTL_PBBUF_NODATA32: rCmd = DMX_IOCTL_PBBUF_NODATA; break;
	case DMX_IOCTL_RSP_ON32: rCmd = DMX_IOCTL_RSP_ON; break;
	case DMX_IOCTL_RSP_OFF32: rCmd = DMX_IOCTL_RSP_OFF; break;
	case DMX_IOCTL_REBUFFER_RANGE32: rCmd = DMX_IOCTL_REBUFFER_RANGE; break;
	case DMX_IOCTL_IS_BUSY32: rCmd = DMX_IOCTL_IS_BUSY; break;
	case DMX_IOCTL_DECRYPT_EXEC_CMD32: rCmd = DMX_IOCTL_DECRYPT_EXEC_CMD; break;
	case DMX_IOCTL_DECRYPT_RELEASE_INST32: rCmd = DMX_IOCTL_DECRYPT_RELEASE_INST; break;
	case DMX_IOCTL_DECRYPT_GET_LAST_ERROR32: rCmd = DMX_IOCTL_DECRYPT_GET_LAST_ERROR; break;
	default:
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("Dmx_Cmd_Switch Fail for Unsupport dwCode = 0x%x\r\n"), cmd);
		break;
	}

	return rCmd;
}

static int get_dmx_spt_enable32(struct SPT_PARAM_ENABLE *kernel_ptr, struct SPT_PARAM_ENABLE32 __user *usr_ptr)
{
	compat_uptr_t tmp;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct SPT_PARAM_ENABLE32)) ||
		get_user(kernel_ptr->fgBadInterLeave, &usr_ptr->fgBadInterLeave) ||
		get_user(tmp, &usr_ptr->prtSpt) ||
		copy_from_user(kernel_ptr->rPbbufCfgInfo, usr_ptr->rPbbufCfgInfo, sizeof(kernel_ptr->rPbbufCfgInfo)))
			return -EFAULT;
	kernel_ptr->ptrSpt= (uintptr_t)compat_ptr(tmp);
	return 0;
}

static int get_dmx_spt_set_rate32(struct SPT_PARAM_SET_RATE *kernel_ptr, struct SPT_PARAM_SET_RATE32 __user *usr_ptr)
{
	compat_uptr_t tmp;
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct SPT_PARAM_SET_RATE32)) ||
		get_user(kernel_ptr->fgDmaAud, &usr_ptr->fgDmaAud) ||
		get_user(kernel_ptr->i4Rate, &usr_ptr->i4Rate) ||
		get_user(tmp, &usr_ptr->ptrSpt))
			return -EFAULT;
	kernel_ptr->ptrSpt = (uintptr_t)compat_ptr(tmp);
	return 0;
}
static int get_dmx_stm_create32(struct DMX_CREATE_STM_PARAM_T *kernel_ptr, struct DMX_CREATE_STM_PARAM_T32 __user *usr_ptr)
{
	DMX_CREATE_STM_PARAM_T32 tmp;
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct DMX_CREATE_STM_PARAM_T32)) ||
		copy_from_user(tmp, usr_ptr->rStmParam, sizeof(tmp)))
		return -EFAULT;
	kernel_ptr->rStmParam.ptrSpt = (uintptr_t)compat_ptr(tmp.rStmParam.ptrSpt);
	kernel_ptr->rStmParam.u4StmType = tmp.rStmParam.u4StmType;
	kernel_ptr->rStmParam.u4StmUID = tmp.rStmParam.u4StmUID;
	kernel_ptr->rStmParam.u8DecSendBufMask = tmp.rStmParam.u8DecSendBufMask;
	return 0;
}
static int get_dmx_stm_set_fifo_sz32(struct STM_PARAM_SET_FIFO_SZ *kernel_ptr, struct STM_PARAM_SET_FIFO_SZ32 __user *usr_ptr)
{
	compat_uptr_t tmp;
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct STM_PARAM_SET_FIFO_SZ32)) ||
		get_user(kernel_ptr->u4Sz, &usr_ptr->u4Sz) ||
		get_user(tmp, &usr_ptr->ptrStm))
		return -EFAULT;
	kernel_ptr->ptrStm = (uintptr_t)compat_ptr(tmp);
	return 0;
}
static int get_dmx_stm_set_threshold32(struct STM_PARAM_SET_THRESHOLD *kernel_ptr, struct STM_PARAM_SET_THRESHOLD32 __user *usr_ptr)
{
	compat_uptr_t tmp;
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct STM_PARAM_SET_THRESHOLD32)) ||
		get_user(kernel_ptr->u4Threshold, &usr_ptr->u4Threshold) ||
		get_user(tmp, &usr_ptr->ptrStm))
		return -EFAULT;
	kernel_ptr->ptrStm = (uintptr_t)compat_ptr(tmp);
	return 0;
}
static int get_dmx_stm_manage_au32(struct DMX_STM_MANAGE_AU_T *kernel_ptr, struct DMX_STM_MANAGE_AU_T32 __user *usr_ptr)
{
	compat_uptr_t tmp;
	compat_caddr_t p_tmp;
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct DMX_STM_MANAGE_AU_T32)) ||
		get_user(tmp, &usr_ptr->ptrStmHdl) ||
		get_user(p_tmp, &usr_ptr->prEsmParam))
		return -EFAULT;
	kernel_ptr->ptrStmHdl = (uintptr_t)compat_ptr(tmp);
	kernel_ptr->prEsmParam = (__force struct ESM_IO_BUF_INFO *)compat_ptr(p_tmp);
	return 0;
}
static int get_dmx_stm_destroy32(struct STM_PARAM_DESTROY *kernel_ptr, struct STM_PARAM_DESTROY32 __user *usr_ptr)
{
	compat_uptr_t tmp1;
	compat_uptr_t tmp2;
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct STM_PARAM_DESTROY32)) ||
		get_user(tmp1, &usr_ptr->ptrSpt) ||
		get_user(tmp2, &usr_ptr->ptrStm))
		return -EFAULT;
	kernel_ptr->ptrSpt = (uintptr_t)compat_ptr(tmp1);
	kernel_ptr->ptrStm = (uintptr_t)compat_ptr(tmp2);
	return 0;
}
static int get_dmx_stm_setuid32(struct STM_PARAM_SET_UID *kernel_ptr, struct STM_PARAM_SET_UID32 __user *usr_ptr)
{
	compat_uptr_t tmp;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct STM_PARAM_SET_UID32)) ||
		get_user(kernel_ptr->u4Uid, &usr_ptr->u4Uid) ||
		get_user(tmp, &usr_ptr->ptrStm))
		return -EFAULT;

	kernel_ptr->ptrStm = (uintptr_t)compat_ptr(tmp);
	return 0;
}
static int get_dmx_psr_fifo_use32(struct DMX_PSR_FIFO_USAGE_T *kernel_ptr, struct DMX_PSR_FIFO_USAGE_T32 __user *usr_ptr)
{
	compat_uptr_t tmp;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct STM_PARAM_SET_UID32)) ||
		get_user(tmp, &usr_ptr->ptrSpt))
		return -EFAULT;

	kernel_ptr->ptrSpt = (uintptr_t)compat_ptr(tmp);
	return 0;
}
static int get_dmx_psr_on32(struct DMX_PSR_ON_PARAM_T *kernel_ptr, struct DMX_PSR_ON_PARAM_T32 __user *usr_ptr)
{
	compat_uptr_t tmp;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct DMX_PSR_ON_PARAM_T32)) ||
		get_user(kernel_ptr->i4DecryptId, &usr_ptr->i4DecryptId) ||
		get_user(tmp, &usr_ptr->ptrSpt) ||
		copy_from_user(kernel_ptr->rStmsCnt, usr_ptr->rStmsCnt, sizeof(DMX_STM_CNT_INFO_T)))
		return -EFAULT;

	kernel_ptr->ptrSpt = (uintptr_t)compat_ptr(tmp);
	return 0;
}
static int get_dmx_psr_file_ofst32(struct DMX_PSR_FILE_OFST_T *kernel_ptr, struct DMX_PSR_FILE_OFST_T32 __user *usr_ptr)
{
	compat_uptr_t tmp;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct DMX_PSR_FILE_OFST_T32)) ||
		get_user(tmp, &usr_ptr->ptrSptHdl))
		return -EFAULT;

	kernel_ptr->ptrSptHdl = (uintptr_t)compat_ptr(tmp);
	return 0;
}
static int get_dmx_cfa_set_type32(struct CFA_PARAM_SET_TYPE *kernel_ptr, struct CFA_PARAM_SET_TYPE32 __user *usr_ptr)
{
	compat_uptr_t tmp;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct CFA_PARAM_SET_TYPE32)) ||
		get_user(kernel_ptr->u4Type, &usr_ptr->u4Type) ||
		get_user(tmp, &usr_ptr->ptrSpt))
		return -EFAULT;

	kernel_ptr->ptrSpt = (uintptr_t)compat_ptr(tmp);
	return 0;
}
static int get_dmx_cfa_config32(struct CFA_PARAM_SET_CONFIG *kernel_ptr, struct CFA_PARAM_SET_CONFIG32 __user *usr_ptr)
{
	compat_uptr_t tmp;
	compat_caddr_t add_tmp;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct CFA_PARAM_SET_CONFIG32)) ||
		get_user(kernel_ptr->u4ConfigSz, &usr_ptr->u4ConfigSz) ||
		get_user(add_tmp, &usr_ptr->pvConfig) ||
		get_user(tmp, &usr_ptr->ptrSpt))
		return -EFAULT;

	kernel_ptr->ptrSpt = (uintptr_t)compat_ptr(tmp);
	kernel_ptr->pvConfig = (__force void *)compat_ptr(add_tmp);
	
	return 0;
}
static int get_dmx_cfa_set_range32(struct CFA_PARAM_SET_RANGE *kernel_ptr, struct CFA_PARAM_SET_RANGE32 __user *usr_ptr)
{
	compat_uptr_t tmp;
	compat_caddr_t add_tmp;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct CFA_PARAM_SET_RANGE32)) ||
		get_user(kernel_ptr->u4RangeSz, &usr_ptr->u4RangeSz) ||
		get_user(add_tmp, &usr_ptr->pvRange) ||
		get_user(tmp, &usr_ptr->ptrSpt))
		return -EFAULT;

	kernel_ptr->ptrSpt = (uintptr_t)compat_ptr(tmp);
	kernel_ptr->pvRange = (__force void *)compat_ptr(add_tmp);
	
	return 0;
}
static int get_dmx_cfa_set_inq_type32(struct CFA_PARAM_SET_INQ_TYPE *kernel_ptr, struct CFA_PARAM_SET_INQ_TYPE32 __user *usr_ptr)
{
	compat_uptr_t tmp;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct CFA_PARAM_SET_INQ_TYPE32)) ||
		get_user(kernel_ptr->u4CfaQID, &usr_ptr->u4CfaQID) ||
		get_user(tmp, &usr_ptr->ptrSpt))
		return -EFAULT;

	kernel_ptr->ptrSpt = (uintptr_t)compat_ptr(tmp);
	
	return 0;
}
static int get_dmx_cfa_set_info32(struct CFA_PARAM_SET_INFO *kernel_ptr, struct CFA_PARAM_SET_INFO32 __user *usr_ptr)
{
	compat_uptr_t tmp;
	compat_caddr_t add_tmp;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct CFA_PARAM_SET_INFO32)) ||
		get_user(kernel_ptr->u4CfaQID, &usr_ptr->u4CfaQID) ||
		get_user(kernel_ptr->u4ParamSize, &usr_ptr->u4ParamSize) ||
		get_user(tmp, &usr_ptr->ptrSpt) ||
		get_user(add_tmp, &usr_ptr->pvCfaParam))
		return -EFAULT;

	kernel_ptr->ptrSpt = (uintptr_t)compat_ptr(tmp);
	kernel_ptr->pvCfaParam = (__force void *)compat_ptr(add_tmp);
	
	return 0;
}

static int get_dmx_send_buf32(struct SEND_BUFFER __user *usr_ptr, struct SEND_BUFFER32 __user *usr_ptr32)
{
	compat_uptr_t tmp;
	compat_caddr_t add_tmp;
	u8 __user *pcBuf;
	uintptr_t ptrBuf;
	PBBUF_SLOT_HEADER_INFO_T32 rHeadInf;
	
	if (copy_in_user(usr_ptr, usr_ptr32, 6 * sizeof(u32)))
		return -EFAULT;
	if (get_user(add_tmp, &usr_ptr32->pcBuffer))
		return -EFAULT;
	pcBuf = compat_ptr(add_tmp);
	if (put_user((unsigned long)pcBuf, &usr_ptr->pcBuffer))
		return -EFAULT;
	if (get_user(tmp, &usr_ptr32->ptrBuffer))
		return -EFAULT;
	ptrBuf = (uintptr_t)compat_ptr(tmp);
	if (put_user(ptrBuf, &usr_ptr->ptrBuffer))
		return -EFAULT;
	if (get_user(usr_ptr->u8AlignedIdx, &usr_ptr32->u8AlignedIdx) ||
		get_user(usr_ptr->u8IssueLen, &usr_ptr32->u8IssueLen) ||
		get_user(usr_ptr->u8SrcOffset, &usr_ptr32->u8SrcOffset) ||
		copy_from_user(rHeadInf, usr_ptr32->rHeader, sizeof(rHeadInf)))
		return -EFAULT;
	usr_ptr->rHeader.eType = rHeadInf.eType;
	usr_ptr->rHeader.u4ParamSz = rHeadInf.u4ParamSz;
	usr_ptr->rHeader.pvParam = (__force void *)compat_ptr(rHeadInf.pvParam);
	
	return 0;
}
static int get_dmx_pbbuf_send_buf32(struct DMX_PBBUF_SEND_BUF_T *kernel_ptr, struct DMX_PBBUF_SEND_BUF_T32 __user *usr_ptr)
{
	PBBUF_PARAM_SEND_BUF32 tmp;
	struct SEND_BUFFER32 __user *send_buffer32;
	struct SEND_BUFFER __user *send_buffer;
	int ret;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct DMX_PBBUF_SEND_BUF_T32)) ||
		get_user(kernel_ptr->fgExitSent, &usr_ptr->fgExitSent) ||
		copy_from_user(tmp, usr_ptr->rBufParam, sizeof(tmp)))
		return -EFAULT;

	kernel_ptr->rBufParam.ptrSpt = (uintptr_t)compat_ptr(tmp.ptrSpt);
	send_buffer32 = compat_ptr(tmp.prBUF);

	if (!access_ok(VERIFY_READ, send_buffer32, sizeof(struct SEND_BUFFER32)))
			return -EFAULT;

	send_buffer = compat_alloc_user_space(sizeof(struct SEND_BUFFER));

	ret = get_dmx_send_buf32(send_buffer, send_buffer32);
	if (ret)
		return ret;

	kernel_ptr->rBufParam.prBUF = (__force struct SEND_BUFFER *)send_buffer;
	
	return 0;
}
static int get_dmx_send_buf_param32(struct PBBUF_PARAM_SEND_BUF *kernel_ptr, struct PBBUF_PARAM_SEND_BUF32 __user *usr_ptr)
{
	compat_uptr_t tmp;
	compat_caddr_t add_tmp;
	struct SEND_BUFFER32 __user *send_buffer32;
	struct SEND_BUFFER __user *send_buffer;
	int ret;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct PBBUF_PARAM_SEND_BUF32)) ||
		get_user(tmp, &usr_ptr->ptrSpt) ||
		get_user(add_tmp, &usr_ptr->prBUF)))
		return -EFAULT;

	kernel_ptr->ptrSpt = (uintptr_t)compat_ptr(tmp.ptrSpt);
	send_buffer32 = compat_ptr(add_tmp);

	if (!access_ok(VERIFY_READ, send_buffer32, sizeof(struct SEND_BUFFER32)))
		return -EFAULT;

	send_buffer = compat_alloc_user_space(sizeof(struct SEND_BUFFER));

	ret = get_dmx_send_buf32(send_buffer, send_buffer32);
	if (ret)
		return ret;

	kernel_ptr->rBufParam.prBUF = (__force struct SEND_BUFFER *)send_buffer;
	
	return 0;
}
static int get_dmx_pbbuf_no_data32(struct DMX_PBBUF_NODATA_PARAM_T *kernel_ptr, struct DMX_PBBUF_NODATA_PARAM_T32 __user *usr_ptr)
{
	compat_uptr_t tmp;
	
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct DMX_PBBUF_NODATA_PARAM_T32)) ||
		get_user(kernel_ptr->u4Status, &usr_ptr->u4Status) ||
		get_user(tmp, &usr_ptr->ptrSpt))
		return -EFAULT;
	kernel_ptr->ptrSpt = (uintptr_t)compat_ptr(tmp);
	return 0;
}
static int get_dmx_spt_rsp_on32(struct SPLITTER_PTX_RSP_ON_INFO_T *kernel_ptr, struct SPLITTER_PTX_RSP_ON_INFO_T32 __user *usr_ptr)
{
	compat_uptr_t tmp;
	
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct SPLITTER_PTX_RSP_ON_INFO_T32)) ||
		get_user(kernel_ptr->fgRebuf, &usr_ptr->fgRebuf) ||
		get_user(kernel_ptr->fgByPassSp, &usr_ptr->fgByPassSp) ||
		get_user(tmp, &usr_ptr->ptrSpt) ||
		get_user(kernel_ptr->u8PtsDelay, &usr_ptr->u8PtsDelay))
		return -EFAULT;
	
	kernel_ptr->ptrSpt = (uintptr_t)compat_ptr(tmp);
	return 0;
}
static int get_dmx_spt_rsp_off32(struct SPLITTER_PTX_RSP_OFF_INFO_T *kernel_ptr, struct SPLITTER_PTX_RSP_OFF_INFO_T32 __user *usr_ptr)
{
	compat_uptr_t tmp;
	
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct SPLITTER_PTX_RSP_OFF_INFO_T32)) ||
		get_user(kernel_ptr->fgCurPbPause, &usr_ptr->fgCurPbPause) ||
		get_user(kernel_ptr->ucState, &usr_ptr->ucState) ||
		get_user(kernel_ptr->ucRspTxRet, &usr_ptr->ucRspTxRet) ||
		get_user(kernel_ptr->ucRspTxType, &usr_ptr->ucRspTxType) ||
		get_user(kernel_ptr->ucRspTxUid, &usr_ptr->ucRspTxUid) ||
		get_user(kernel_ptr->ucRspMode, &usr_ptr->ucRspMode) ||
		get_user(tmp, &usr_ptr->ptrSpt))
		return -EFAULT;
	
	kernel_ptr->ptrSpt = (uintptr_t)compat_ptr(tmp);
	return 0;
}
static int get_dmx_spt_rebuf_range32(struct SPLITTER_PTX_REBUFFER_RANGE_INFO_T *kernel_ptr, 
	struct SPLITTER_PTX_REBUFFER_RANGE_INFO_T32 __user *usr_ptr)
{
	compat_uptr_t tmp;
	
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct SPLITTER_PTX_REBUFFER_RANGE_INFO_T32)) ||
		get_user(kernel_ptr->fgRebuf, &usr_ptr->fgRebuf) ||
		get_user(tmp, &usr_ptr->ptrSpt) ||
		get_user(kernel_ptr->u8PtsDelay, &usr_ptr->u8PtsDelay) ||
		get_user(kernel_ptr->u8RspStartPts, &usr_ptr->u8RspStartPts) ||
		get_user(kernel_ptr->u8RspStartOffset, &usr_ptr->u8RspStartOffset) ||
		get_user(kernel_ptr->u8PbbStartOffset, &usr_ptr->u8PbbStartOffset))
		return -EFAULT;
	
	kernel_ptr->ptrSpt = (uintptr_t)compat_ptr(tmp);
	return 0;
}
static int get_dmx_decrypt_oper32(struct DECRYPT_OPER_PARAM_T *kernel_ptr, DECRYPT_OPER_PARAM_T32 __user *usr_ptr)
{
	compat_caddr_t tmp_inst;
	compat_caddr_t tmp_oper;
	
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct DECRYPT_OPER_PARAM_T32)) ||
		get_user(kernel_ptr->u4OperParamSz, &usr_ptr->u4OperParamSz) ||
		get_user(kernel_ptr->u4OperCode, &usr_ptr->u4OperCode) ||
		get_user(tmp_inst, &usr_ptr->pvInst) ||
		get_user(tmp_oper, &usr_ptr->pvOperParam))
		return -EFAULT;
	
	kernel_ptr->pvInst = (__force void *)compat_ptr(tmp_inst);
	kernel_ptr->pvOperParam = (__force void *)compat_ptr(tmp_oper);
	return 0;
}
static int get_dmx_decrypt_inst32(struct DECRYPT_INST_PARAM_T *kernel_ptr, DECRYPT_INST_PARAM_T32 __user *usr_ptr)
{
	compat_caddr_t tmp_inst;
	
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct DECRYPT_INST_PARAM_T32)) ||
		get_user(kernel_ptr->eDecryptType, &usr_ptr->eDecryptType) ||
		get_user(tmp_inst, &usr_ptr->pvInst))
		return -EFAULT;
	
	kernel_ptr->pvInst = (__force void *)compat_ptr(tmp_inst);
	return 0;
}

static int put_dmx_stm_create32(struct DMX_CREATE_STM_PARAM_T *kernel_ptr, DMX_CREATE_STM_PARAM_T32 __user *usr_ptr)
{
	uintptr_t tmp;
	
	if (!access_ok(VERIFY_WRITE, usr_ptr, sizeof(struct DMX_CREATE_STM_PARAM_T32)) ||
		put_user(kernel_ptr->ptrStmHdl, &tmp))
		return -EFAULT;
	
	usr_ptr->ptrStmHdl = (compat_uptr_t)ptr_to_compat(&tmp);
	return 0;
}

static int put_dmx_psr_fifo_use32(struct DMX_PSR_FIFO_USAGE_T *kernel_ptr, DMX_PSR_FIFO_USAGE_T32 __user *usr_ptr)
{
	if (!access_ok(VERIFY_WRITE, usr_ptr, sizeof(struct DMX_PSR_FIFO_USAGE_T32)) ||
		put_user(kernel_ptr->u4Fifo, &usr_ptr->u4Fifo))
		return -EFAULT;

	return 0;
}
static int put_dmx_psr_file_ofst32(struct DMX_PSR_FILE_OFST_T *kernel_ptr, DMX_PSR_FILE_OFST_T32 __user *usr_ptr)
{
	if (!access_ok(VERIFY_WRITE, usr_ptr, sizeof(struct DMX_PSR_FILE_OFST_T32)) ||
		put_user(kernel_ptr->u8FileOfst, &usr_ptr->u8FileOfst))
		return -EFAULT;

	return 0;
}
static int put_dmx_spt_rsp_off32(struct SPLITTER_PTX_RSP_OFF_INFO_T *kernel_ptr, SPLITTER_PTX_RSP_OFF_INFO_T32 __user *usr_ptr)
{
	uintptr_t tmp;
	if (!access_ok(VERIFY_WRITE, usr_ptr, sizeof(struct SPLITTER_PTX_RSP_OFF_INFO_T32)) ||
		put_user(kernel_ptr->fgCurPbPause, &usr_ptr->fgCurPbPause) ||
		put_user(kernel_ptr->ucState, &usr_ptr->ucState) ||
		put_user(kernel_ptr->ucRspTxRet, &usr_ptr->ucRspTxRet) ||
		put_user(kernel_ptr->ucRspTxType, &usr_ptr->ucRspTxType) ||
		put_user(kernel_ptr->ucRspTxUid, &usr_ptr->ucRspTxUid) ||
		put_user(kernel_ptr->ucRspMode, &usr_ptr->ucRspMode) ||
		put_user(kernel_ptr->ptrSpt, &tmp))
		return -EFAULT;
	usr_ptr->ptrSpt = (compat_uptr_t)ptr_to_compat(&tmp);

	return 0;
}
static int put_dmx_spt_rebuf_range32(struct SPLITTER_PTX_REBUFFER_RANGE_INFO_T *kernel_ptr, SPLITTER_PTX_REBUFFER_RANGE_INFO_T32 __user *usr_ptr)
{
	uintptr_t tmp;
	
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct SPLITTER_PTX_REBUFFER_RANGE_INFO_T32)) ||
		put_user(kernel_ptr->fgRebuf, &usr_ptr->fgRebuf) ||
		put_user(kernel_ptr->ptrSpt, &tmp) ||
		put_user(kernel_ptr->u8PtsDelay, &usr_ptr->u8PtsDelay) ||
		put_user(kernel_ptr->u8RspStartPts, &usr_ptr->u8RspStartPts) ||
		put_user(kernel_ptr->u8RspStartOffset, &usr_ptr->u8RspStartOffset) ||
		put_user(kernel_ptr->u8PbbStartOffset, &usr_ptr->u8PbbStartOffset))
		return -EFAULT;
	
	kernel_ptr->ptrSpt = (compat_uptr_t)compat_ptr(&tmp);
	return 0;
}
static int put_dmx_send_buf_param32(struct PBBUF_PARAM_SEND_BUF *kernel_ptr, PBBUF_PARAM_SEND_BUF32 __user *usr_ptr)
{
	int ret;
	compat_caddr_t add_tmp;
	struct SEND_BUFFER32 __user *send_buffer32;
	struct SEND_BUFFER __user *send_buffer;
	
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(struct PBBUF_PARAM_SEND_BUF32)) ||
		put_user(kernel_ptr->prBUF, &add_tmp)))
		return -EFAULT;

	send_buffer32 = ptr_to_compat(&add_tmp);

	if (!access_ok(VERIFY_READ, send_buffer32, sizeof(struct SEND_BUFFER32)))
		return -EFAULT;

	send_buffer = compat_alloc_user_space(sizeof(struct SEND_BUFFER));

	ret = get_dmx_send_buf32(send_buffer, send_buffer32);
	if (ret)
		return ret;

	kernel_ptr->rBufParam.prBUF = (__force struct SEND_BUFFER *)send_buffer;
	
	return 0;
}

bool DMX_IOControl_Compat(struct file *file, unsigned int cmd, unsigned long arg)
{
	union {
		struct SPT_PARAM_ENABLE rSptEnable;
		struct SPT_PARAM_SET_RATE rSptSetRate;
		struct DMX_CREATE_STM_PARAM_T rCreateStmParam;
		struct STM_PARAM_SET_FIFO_SZ rStmSetFifoSize;
		struct STM_PARAM_SET_THRESHOLD rStmSetThreshold;
		struct DMX_STM_MANAGE_AU_T rManAU;
		struct STM_PARAM_DESTROY rStmDestroy;
		struct STM_PARAM_SET_UID rStmSetUID;
		struct DMX_PSR_FIFO_USAGE_T rPsrFifoUse;
		struct DMX_PSR_ON_PARAM_T rPsrOn;
		struct DMX_PSR_FILE_OFST_T rPsrFileOfst;
		struct CFA_PARAM_SET_TYPE rCfaSetType;
		struct CFA_PARAM_SET_CONFIG rCfaSetConfig;
		struct CFA_PARAM_SET_RANGE rCfaSetRange;
		struct CFA_PARAM_SET_INQ_TYPE rCfaSetInqType;
		struct CFA_PARAM_SET_INFO rCfaSetInfo;
		struct DMX_PBBUF_SEND_BUF_T rPbbufSendBuf;
		struct PBBUF_PARAM_SEND_BUF rSendBufParam;
		struct DMX_PBBUF_NODATA_PARAM_T rPbbufNoData;
		struct SPLITTER_PTX_RSP_ON_INFO_T rSptRspOn;
		struct SPLITTER_PTX_RSP_OFF_INFO_T rSptRspOff;
		struct SPLITTER_PTX_REBUFFER_RANGE_INFO_T rSptRebufRange;
		struct DECRYPT_OPER_PARAM_T rDecryptOper;
		struct DECRYPT_INST_PARAM_T rDecyptInst;
		uintptr_t rPtr;
		u32 rUint;
	} rArg;
	
	void __user *usr_ptr = compat_ptr(arg);
	bool fgCompatible = TRUE;
	long err = 0;
	compat_uptr_t usr_tmp;
	uintptr_t kernel_tmp;

	unsigned int u4CompCmd = dmx_cmd_switch(cmd);
	switch (u4CompCmd) {
		case DMX_IOCTL_SPT_DISABLE:
		case DMX_IOCTL_SPT_IS_ENABLED:
		case DMX_IOCTL_SPT_DESTROY:
		case DMX_IOCTL_STM_ENABLE:
		case DMX_IOCTL_STM_DISABLE:
		case DMX_IOCTL_STM_FLUSH_FIFO:
		case DMX_IOCTL_PSR_OFF:
		case DMX_IOCTL_PSR_PAUSE:
		case DMX_IOCTL_PBBUF_CANCEL_BUF:
		case DMX_IOCTL_IS_BUSY:
			err = get_user(usr_tmp, (compat_uptr_t __user *)usr_ptr);
			rArg.rPtr = (uintptr_t)compat_ptr(usr_tmp);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_SPT_ENABLE:
			err = get_dmx_spt_enable32(rArg.rSptEnable, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_SPT_SET_RATE:
			err = get_dmx_spt_set_rate32(rArg.rSptSetRate, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_STM_CREATE:
			err = get_dmx_stm_create32(rArg.rCreateStmParam, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_STM_SET_FIFO_SZ:
			err = get_dmx_stm_set_fifo_sz32(rArg.rStmSetFifoSize, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_STM_SET_THRESHOLD32:
			err = get_dmx_stm_set_threshold32(rArg.rStmSetThreshold, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_STM_GETAU:
		case DMX_IOCTL_STM_RELEASEAU:
		case DMX_IOCTL_STM_RELEASE_FF_AUDIOAU:
			err = get_dmx_stm_manage_au32(rArg.rManAU, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_STM_DESTROY:
			err = get_dmx_stm_destroy32(rArg.rStmDestroy, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_STM_SETUID:
			err = get_dmx_stm_setuid32(rArg.rStmSetUID, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_PSR_VFIFO_USAGE:
		case DMX_IOCTL_PSR_AFIFO_USAGE:
			err = get_dmx_psr_fifo_use32(rArg.rPsrFifoUse, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_PSR_ON:
			err = get_dmx_psr_on32(rArg.rPsrOn, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_PSR_FILE_OFST:
		case DMX_IOCTL_CFA_GET_POSI:
			err = get_dmx_psr_file_ofst32(rArg.rPsrFileOfst, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_CFA_SET_TYPE:
			err = get_dmx_cfa_set_type32(rArg.rCfaSetType, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_CFA_CONFIG:
			err = get_dmx_cfa_config32(rArg.rCfaSetConfig, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_CFA_SET_RANGE:
			err = get_dmx_cfa_set_range32(rArg.rCfaSetRange, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_CFA_SET_INQUIRE_TYPE:
			err = get_dmx_cfa_set_inq_type32(rArg.rCfaSetInqType, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_CFA_SET_GEN:
			err = get_dmx_cfa_set_info32(rArg.rCfaSetInfo, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_PBBUF_SEND_BUF:
			err = get_dmx_pbbuf_send_buf32(rArg.rPbbufSendBuf, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_PBBUF_ALLOC_BUF:
		case DMX_IOCTL_PBBUF_RELEASE_BUF:
			err = get_dmx_send_buf_param32(rArg.rSendBufParam, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_PBBUF_NODATA:
			err = get_dmx_pbbuf_no_data32(rArg.rPbbufNoData, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_RSP_ON:
			err = get_dmx_spt_rsp_on32(rArg.rSptRspOn, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_RSP_OFF:
			err = get_dmx_spt_rsp_off32(rArg.rSptRspOff, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_REBUFFER_RANGE:
			err = get_dmx_spt_rebuf_range32(rArg.rSptRebufRange, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_DECRYPT_EXEC_CMD:
			err = get_dmx_decrypt_oper32(rArg.rDecryptOper, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_DECRYPT_RELEASE_INST:
		case DMX_IOCTL_DECRYPT_GET_LAST_ERROR:
			err = get_dmx_decrypt_inst32(rArg.rDecyptInst, usr_ptr);
			fgCompatible = FALSE;
			break;
		
	}

	if (err)
		return err;

	if (fgCompatible)
		err = dmx_native_ioctl(file, cmd, (unsigned long)usr_ptr);
	else {
		mm_segment_t old_fs = get_fs();

		set_fs(KERNEL_DS);
		err = dmx_native_ioctl(file, cmd, (unsigned long)&rArg);
		set_fs(old_fs);
	}

	switch (u4CompCmd) {
		case DMX_IOCTL_IS_BUSY:
		case DMX_IOCTL_SPT_CREATE:
			err = put_user(rArg.rPtr, kernel_tmp);
			usr_ptr = ptr_to_compat(kernel_tmp);
			break;
		case DMX_IOCTL_SPT_OPEN_GET_CNT:
			err = put_user(rArg.rUint, usr_ptr);
			break;
		case DMX_IOCTL_STM_CREATE:
			err = put_dmx_stm_create32(rArg.rCreateStmParam, usr_ptr);
			break;
		case DMX_IOCTL_STM_GETAU:
			break;
		case DMX_IOCTL_PSR_VFIFO_USAGE:
		case DMX_IOCTL_PSR_AFIFO_USAGE:
			err = put_dmx_psr_fifo_use32(rArg.rPsrFifoUse, usr_ptr);
			break;
		case DMX_IOCTL_PSR_FILE_OFST:
		case DMX_IOCTL_CFA_GET_POSI:
			err = put_dmx_psr_file_ofst32(rArg.rPsrFileOfst, usr_ptr);
			break;
		case DMX_IOCTL_RSP_OFF:
			err = put_dmx_spt_rsp_off32(rArg.rSptRspOff, usr_ptr)
			break;
		case DMX_IOCTL_REBUFFER_RANGE:
			err = put_dmx_spt_rebuf_range32(rArg.rSptRebufRange, usr_ptr);
			break;
		case DMX_IOCTL_PBBUF_ALLOC_BUF:
			err = put_dmx_send_buf_param32(rArg.rSendBufParam, usr_ptr);
			break;
		case DMX_IOCTL_DECRYPT_DEVICE_IS_ACTIVED:
			break;
		
	}

	
}

#endif    //CONFIG_COMPAT


/*****************************************************************************************/

#endif				/*#ifndef DMX_COMPAT_IOCTL32 */

