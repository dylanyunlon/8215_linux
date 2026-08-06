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
#include <linux/ioctl.h>
#include <linux/module.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include "dmx_def.h"
#include "dmx_log.h"
#include "dmx_spt.h"
#include "dmx_spt_cfa.h"
#include "dmx_stream.h"
#include <media/atc/ioctl_dmx.h>

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/ioctl_dmx.h>
#include <media/atc/drv_esm_if.h>

EXTERN DMX_STM_MAN_INFO_T g_rDmxStmMan;
EXTERN DMX_SPT_MAN_INFO_T g_rSptMan;

/*
==========================COMPAT 32 BIT=========================
*/

/*
============================================================
=====================IOCTL PARAMETER(arg)========================
============================================================
*/


typedef struct {
	bool fgBadInterLeave;
	compat_caddr_t pvSptHdl;
	DMX_PBBUF_CONFIG_INFO_T rPbbufCfgInfo;
} SPT_PARAM_ENABLE32;

typedef struct {
	bool fgDmaAud;
	__s32 i4Rate;
	compat_caddr_t pvSptHdl;
} SPT_PARAM_SET_RATE32;

typedef struct {
	__u32 u4StmType;
	__u32 u4StmUID;
	compat_caddr_t pvSptHdl;
	__u64 u8DecSendBufMask;
} STM_PARAM_CREATE32;

typedef struct {
	compat_caddr_t pvStmHdl;
	STM_PARAM_CREATE rStmParam;
} DMX_CREATE_STM_PARAM_T32;

typedef struct {
	__u32 u4Sz;
	compat_caddr_t pvStmHdl;
} STM_PARAM_SET_FIFO_SZ32;

typedef struct {
	__u32 u4Threshold;
	compat_caddr_t pvStmHdl;
} STM_PARAM_SET_THRESHOLD32;

typedef struct {
	compat_caddr_t pvStmHdl;
	compat_caddr_t prEsmParam;
} DMX_STM_MANAGE_AU_T32;

typedef struct {
	compat_caddr_t pvSptHdl;
	compat_caddr_t pvStmHdl;
} STM_PARAM_DESTROY32;

typedef struct {
	__u32 u4Uid;
	compat_caddr_t pvStmHdl;
} STM_PARAM_SET_UID32;

typedef struct {
	__u32 u4Fifo;
	compat_caddr_t pvSptHdl;
} DMX_PSR_FIFO_USAGE_T32;

typedef struct {
	__u32 u4VidStmCnt;
	__u32 u4AudStmCnt;
	__u32 u4SPStmCnt;
} DMX_STM_CNT_INFO_T32;

typedef struct {
	__s32 i4DecryptId;
	compat_caddr_t pvSptHdl;
	DMX_STM_CNT_INFO_T32 rStmsCnt;
}  DMX_PSR_ON_PARAM_T32;

typedef struct {
	compat_caddr_t pvSptHdl;
	__u64 u8FileOfst;
} DMX_PSR_FILE_OFST_T32;

typedef struct {
	__u32 u4Type;
	compat_caddr_t pvSptHdl;
} CFA_PARAM_SET_TYPE32;

typedef struct {
	__u32 u4ConfigSz;
	compat_caddr_t pvSptHdl;
	compat_caddr_t pvConfig;
} CFA_PARAM_SET_CONFIG32;

typedef struct {
	__u32 u4RangeSz;
	compat_caddr_t pvSptHdl;
	compat_caddr_t pvRange;
} CFA_PARAM_SET_RANGE32;

typedef struct {
	__u32 u4CfaQID;
	compat_caddr_t pvSptHdl;
} CFA_PARAM_SET_INQ_TYPE32;

typedef struct {
	__u32 u4CfaQID;
	__u32 u4ParamSize;
	compat_caddr_t pvSptHdl;
	compat_caddr_t pvCfaParam;
} CFA_PARAM_GET_INFO32, CFA_PARAM_SET_INFO32;

typedef struct {
	compat_caddr_t pvSptHdl;
	compat_caddr_t prBUF;
} PBBUF_PARAM_SEND_BUF32;

typedef struct {
	bool fgExitSent;
	PBBUF_PARAM_SEND_BUF32 rBufParam;
} DMX_PBBUF_SEND_BUF_T32;

typedef struct {
	__u32 u4Status;
	compat_caddr_t pvSptHdl;
} DMX_PBBUF_NODATA_PARAM_T32;

typedef struct {
	/*! input parameter */
	bool fgRebuf;		/*! [IN] Rsp has rebuffered or not */
	bool fgByPassSp;	/*! [IN] Rsp not only tx audio, but also tx subpic */

	compat_caddr_t pvSptHdl;
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
	compat_caddr_t pvSptHdl;

} SPLITTER_PTX_RSP_OFF_INFO_T32;

typedef struct {
	/*! input parameter */
	bool fgRebuf;

	compat_caddr_t pvSptHdl;
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
}  DECRYPT_OPER_PARAM_T32;

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
	compat_caddr_t pvBuffer;		/* no need to be a pointer */
	__u64 u8SrcOffset;
	__u64 u8AlignedIdx;
	__u64 u8IssueLen;
#if DMX_SUPPORT_FFRW
	PBBUF_SLOT_HEADER_INFO_T32 rHeader;
#endif	
} SEND_BUFFER32;

/// Picture information of a Access unit
typedef struct
{
  __u32 u4VType;              ///< Byte3~2: Extra Type; Byte 1~0: picture coded type, please refer drv_Common.h
  compat_uptr_t ptrSAddr;                ///< start address in fifo
  compat_uptr_t ptrEAddr;              ///< end address in fifo

  compat_uptr_t ptrSVAddr;                ///< start address in fifo
  compat_uptr_t ptrEVAddr;              ///< end address in fifo

  __u64 u8Pts;                   ///< PTS
  __u64 u8PrevPTS;          ///< Previous PTS
  __u64 u8Dts;                  ///< DTS
  __u64 u8Offset;              ///< Lba or file offset
  __u64 u8PTSOffset;              ///< PTS Offset From MW
  __u64 u8SoftPts;            ///< Soft PTS
  compat_uptr_t ptrSeqHdrSa;      ///< Sequence Header Starting Address //Only for WMV789 (No Start Code WMV Source)
  __u32 u4SeqHdrLen;      ///< Sequence Header Data Length //Only for WMV789 (No Start Code WMV Source)
  compat_uptr_t ptrPPSHdrSa;          ///< Picture Header Starting Address
  __u32 u4PPSHdrLen;        ///< Picture Header Data Length
  compat_uptr_t ptrCCSa;               ///< Start address of user data which contains closed caption
  DiscType eDiscType;        ///< disc type, please refer drv_common.h
  __u32 u4Duration;         ///< For Variable Frame Rate File Format
  __u32 u4PrevDuration;   ///< For Variable Frame Rate File Format
  __u32 u4WMVSliceAddr[3];   ///< WMV Slice Address information
  bool fgxvColor;
  __u32 u4xvColorR;     ///< xvColor: R data
  __u32 u4xvColorG;     ///< xvColor: G data
  __u32 u4xvColorB;     ///< xvColor: B data
  __u32 u4EsdIndex;
  __u32 u4EsdNums;

  __u32 u4RMSliceNum;
  __u32 auRM4SliceSize[128];
} PicInfo32;

/// access unit information of video
typedef struct
{
  AU_TYPE eAuType;                     ///< Access unit type
  bool      fgIBCSent;                        ///< Inband command had sent back to MW
  union
  {
    PicInfo32 rInfo;                             ///< Picture information
  } rAUInfo;
} AU_VPic32;

/// SubPicture information of a Access unit
typedef struct
{
  compat_uptr_t ptrAddr;               ///< start address in fifo
  __u32 u4Size;                ///< Size in fifo
  __u64 u8StartPts;          ///< Start PTS
  __u64 u8EndPts;            ///< End PTS
  __u64 u8Offset;              ///< Lba or file offset
  __u64 u8Dts;                  ///< Dts
} SPicInfo32;

/// access unit information of Subpicture
typedef struct
{
  AU_TYPE eAuType;                     ///< Access unit type
  bool fgIBCSent;                        ///< Inband command had sent back to MW
  union
  {
    SPicInfo rInfo;                           ///< Subpicture information
  } rAUInfo;
} AU_SP32;

/// Audio information of a Access unit
typedef struct
{
  __u64 u8Pts;                ///< Pts
  AUD_TYPE eAudType;          ///< 0: AC3, 1:MLP
} AudInfo32;

/// access unit information of Audio
typedef struct
{
	bool fgSkipData;               ///< start skip audio data flag

    compat_uptr_t ptrSAddr;                ///< start address in fifo
    compat_uptr_t ptrEAddr;              ///< end address in fifo
	AU_TYPE eAuType;                     ///< Access unit type
  
	union
	{
		AudInfo32 rInfo;                          ///< audio data information
	} rAUInfo;
} AU_AUDIO32;

/// access unit information of Section
typedef struct
{
  bool fgSkipData;               ///< start skip audio data flag

  compat_uptr_t ptrSAddr;                ///< start address in fifo
  compat_uptr_t ptrEAddr;              ///< end address in fifo
  compat_uptr_t	ptrVirSA;
  compat_uptr_t	ptrPhySA;
  compat_uptr_t ptrUserVirSA;
  __u32  u4Size;
  AU_TYPE eAuType;                     ///< Access unit type
} AU_SECTION32;

typedef struct
{
	__u16 u2OffTop;
	__u16 u2OffBottom;
	__u8  achPAL[32];
	__u32	sx;
	__u32	sy;
	__u32	ex;
	__u32	ey;
	__u32 u4Size;
	compat_uptr_t ptrRLESWDecVirSA;
	compat_uptr_t	ptrVirSA;
	compat_uptr_t	ptrPhySA;
	compat_uptr_t ptrUserVirSA;    // user space address
	compat_uptr_t ptrAUEndAddrOfFifo;  // au's end addr of fifo
	__u64 u8StartPts;
	__s64	i8Delay;
} ESM_IO_BUF_SP_INFO32;

typedef struct
{
	bool  fgSetSeekInfo;
	__u32 au4SeekInfo[2];
} APEINFO_EXT_T32;

typedef union
{
  APEINFO_EXT_T rApe;
} AU_AUDIO_EXT_INFO_T32;

typedef struct
{
	__u64 ptrFifoSPAddr;	// phisical address
	__u64 ptrFifoEPAddr;	// phisical address

	__u64 ptrFifoSVAddr;	//  Virtule address
	__u64 ptrFifoEVAddr;	//  Virtule address

	__u32 u4Status;        // dummy decoder's error status
	__u32 u4TimeWait;      // time out

	union
	{
		AU_AUDIO32 rAudioAU;
		AU_VPic32 rVPicAU;
		AU_SECTION32 rSectionAU;
		ESM_IO_BUF_SP_INFO32 rSPStruct;
	} rAU;

	union
	{
		AU_AUDIO_EXT_INFO_T32 rAudEx;
	} rAUEx;
} ESM_IO_BUF_INFO32;

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
#define DMX_IOCTL_SPT_ENABLE32				_IOW('D', 2, SPT_PARAM_ENABLE32)

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


#define DMX_IOCTL_SPT_SET_RATE32				_IOW('D', 6, SPT_PARAM_SET_RATE32)


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
#define DMX_IOCTL_STM_CREATE32				_IOWR('D', 12, DMX_CREATE_STM_PARAM_T32)

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
#define DMX_IOCTL_STM_SET_FIFO_SZ32			_IOW('D', 15, STM_PARAM_SET_FIFO_SZ32)

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
#define DMX_IOCTL_STM_SET_THRESHOLD32			_IOW('D', 16, STM_PARAM_SET_THRESHOLD32)

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

#define DMX_IOCTL_STM_GETAU32					_IOWR('D', 18, DMX_STM_MANAGE_AU_T32)

#define DMX_IOCTL_STM_RELEASEAU32				_IOW('D', 19, DMX_STM_MANAGE_AU_T32)

#define DMX_IOCTL_STM_DESTROY32				_IOW('D', 20, STM_PARAM_DESTROY32)

#define DMX_IOCTL_STM_SETUID32				_IOW('D', 21, STM_PARAM_SET_UID32)

#define DMX_IOCTL_STM_RELEASE_FF_AUDIOAU32		_IOW('D', 22, DMX_STM_MANAGE_AU_T32)

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
#define DMX_IOCTL_PSR_VFIFO_USAGE32			_IOWR('D', 25, DMX_PSR_FIFO_USAGE_T32)

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
#define DMX_IOCTL_PSR_AFIFO_USAGE32			_IOWR('D', 26, DMX_PSR_FIFO_USAGE_T32)

/**
 *
 */
#define DMX_IOCTL_PSR_ON32					_IOW('D', 27, DMX_PSR_ON_PARAM_T32)

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
#define DMX_IOCTL_PSR_FILE_OFST32				_IOWR('D', 31, DMX_PSR_FILE_OFST_T32)


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
#define DMX_IOCTL_CFA_SET_TYPE32				_IOW('D', 32, CFA_PARAM_SET_TYPE32)

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
#define DMX_IOCTL_CFA_CONFIG32				_IOW('D', 33, CFA_PARAM_SET_CONFIG32)

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
#define DMX_IOCTL_CFA_SET_RANGE32				_IOW('D', 34, CFA_PARAM_SET_RANGE32)

/**
 *
 */
#define DMX_IOCTL_CFA_SET_INQUIRE_TYPE32		_IOW('D', 35, CFA_PARAM_SET_INQ_TYPE32)


/**
 *
 */
#define DMX_IOCTL_CFA_SET_GEN32				_IOW('D', 37, CFA_PARAM_SET_INFO32)


/**
 *
 */
#define DMX_IOCTL_CFA_GET_POSI32				_IOWR('D', 36, DMX_PSR_FILE_OFST_T32)

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
#define DMX_IOCTL_PBBUF_ALLOC_BUF32			_IOWR('D', 42, PBBUF_PARAM_SEND_BUF32)

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
#define DMX_IOCTL_PBBUF_SEND_BUF32			_IOW('D', 44, DMX_PBBUF_SEND_BUF_T32)

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
#define DMX_IOCTL_PBBUF_RELEASE_BUF32			_IOW('D', 45, PBBUF_PARAM_SEND_BUF32)


#define DMX_IOCTL_PBBUF_NODATA32				_IOW('D', 51, DMX_PBBUF_NODATA_PARAM_T32)

/*
============================================================
==============================R E S P L I T T E R==================
===========================================================
 *
 * @brief turn on demuxer
 *
 * We use this to turn on demuxer.
 */
#define DMX_IOCTL_RSP_ON32					_IOW('D', 52, SPLITTER_PTX_RSP_ON_INFO_T32)

/*!
 * @brief turn off demuxer
 *
 * We use this to turn off demuxer.
 */

#define DMX_IOCTL_RSP_OFF32					_IOWR('D', 53, SPLITTER_PTX_RSP_OFF_INFO_T32)
/*!
 * @brief Get the information report by Parser for Playback Buffer file offset start.
 *
 * It is for data rebuffer
 * We use this to get current pbb start file offset (__u8).
 */
#define DMX_IOCTL_REBUFFER_RANGE32			_IOWR('D', 54, SPLITTER_PTX_REBUFFER_RANGE_INFO_T32)

/*
=============================================================
================================= CLI =========================
=============================================================
*/

#define DMX_IOCTL_IS_BUSY32           _IOR('D', 56, compat_uptr_t)

/*
=============================================================
================================= DECRYPT =====================
=============================================================
*/
/* Input:  DECRYPT_OPER_PARAM_T*/
/* Output: according to the command*/
#define DMX_IOCTL_DECRYPT_EXEC_CMD32					_IOW('D', 59, DECRYPT_OPER_PARAM_T32)

/* Input:  rParam.eDecryptType, rParam.pvInst = pvInst, */
/* rParam's Type: DECRYPT_INST_PARAM_T*/
/* Output: NULL*/
#define DMX_IOCTL_DECRYPT_RELEASE_INST32			_IOW('D', 60, DECRYPT_INST_PARAM_T32)


/* Input:  rParam.eDecryptType, rParam.pvInst = pvInst, */
/*				 rParam's Type: DECRYPT_INST_PARAM_T*/
/* Output: MRESULT, defined in dmx_define.h*/
#define DMX_IOCTL_DECRYPT_GET_LAST_ERROR32			_IOW('D', 64, DECRYPT_INST_PARAM_T32)

void *get_dmx_spt_inst_from32(void *hSpt)
{
	DMX_SPT_INST_T *prSpt = NULL;
	u32 i = 0;

	for (i = 0; i < DMX_MAX_SPT_INST_CNT; i++) {
		prSpt = g_rSptMan.aprSptInst[i];
		if ((u32)prSpt == (u32)hSpt) {
			pr_debug("%s line %d succes, hSpt: 0x%p, return prSpt: 0x%p\r\n",
				__func__, __LINE__, hSpt, prSpt);
			return (void *)prSpt;
		}
	}

	pr_info("%s line %d fail, hSpt: 0x%p, return NULL\r\n",
		__func__, __LINE__, hSpt);
	return NULL;
}

void *get_dmx_stm_inst_from32(void *hStm)
{
	DMX_STM_INST_T *prStm = NULL;
	u32	i = 0;

	for (i = 0; i < (u32)MAX_STREAM_INSTANCE_CNT; i++) {
		prStm = &(g_rDmxStmMan.arStmInst[i]);
		if ((u32)prStm == (u32)hStm) {
			pr_debug("%s line %d succes, hStm: 0x%p, return pvStmHdl: 0x%p\r\n",
				__func__, __LINE__, (void *)hStm, prStm);
			return (void *)prStm;
		}
	}

	pr_info("%s line %d fail, hStm: 0x%p, return NULL\r\n",
		__func__, __LINE__, (void *)hStm);
	return NULL;
}

/*===============================================================*/
static long dmx_native_ioctl(struct file *file, unsigned int cmd,
  unsigned long arg, bool fgIsUserMem)
{
	long ret = 0;
	void *pvContext = NULL;

	pvContext = file->private_data;
	if (!fgIsUserMem) {
		if (!DMX_IOControl(pvContext, cmd, arg, FALSE)) {
			return -1;
		}
	} else {
		if (file->f_op->unlocked_ioctl)
			ret = file->f_op->unlocked_ioctl(file, cmd, arg);
	}

	return ret;
}

static unsigned int dmx_cmd_switch(unsigned int cmd)
{
	unsigned int rCmd = 0;
	
	pr_debug("%s enter, cmd: 0x%08x\r\n", __func__, cmd);

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
	case DMX_IOCTL_CFA_SET_GEN32: rCmd = DMX_IOCTL_CFA_SET_GEN; break;
	case DMX_IOCTL_CFA_GET_POSI32: rCmd = DMX_IOCTL_CFA_GET_POSI; break;
	case DMX_IOCTL_PBBUF_ALLOC_BUF32: rCmd = DMX_IOCTL_PBBUF_ALLOC_BUF; break;
	case DMX_IOCTL_PBBUF_CANCEL_BUF32: rCmd = DMX_IOCTL_PBBUF_CANCEL_BUF; break;
	case DMX_IOCTL_PBBUF_SEND_BUF32: rCmd = DMX_IOCTL_PBBUF_SEND_BUF; break;
	case DMX_IOCTL_PBBUF_RELEASE_BUF32: rCmd = DMX_IOCTL_PBBUF_RELEASE_BUF; break;
	case DMX_IOCTL_PBBUF_NODATA32: rCmd = DMX_IOCTL_PBBUF_NODATA; break;
	case DMX_IOCTL_RSP_ON32: rCmd = DMX_IOCTL_RSP_ON; break;
	case DMX_IOCTL_RSP_OFF32: rCmd = DMX_IOCTL_RSP_OFF; break;
	case DMX_IOCTL_REBUFFER_RANGE32: rCmd = DMX_IOCTL_REBUFFER_RANGE; break;
	case DMX_IOCTL_IS_BUSY32: rCmd = DMX_IOCTL_IS_BUSY; break;
	case DMX_IOCTL_DECRYPT_EXEC_CMD32: rCmd = DMX_IOCTL_DECRYPT_EXEC_CMD; break;
	case DMX_IOCTL_DECRYPT_RELEASE_INST32: rCmd = DMX_IOCTL_DECRYPT_RELEASE_INST; break;
	case DMX_IOCTL_DECRYPT_GET_LAST_ERROR32: rCmd = DMX_IOCTL_DECRYPT_GET_LAST_ERROR; break;
	case DMX_IOCTL_SPT_OPEN_GET_CNT:
	case DMX_IOCTL_SPT_REBUF_RANGE:
	case DMX_IOCTL_SPT_SET_LASTMEM:
	case DMX_IOCTL_SPT_LOCK:
	case DMX_IOCTL_SPT_UNLOCK:
  	case DMX_IOCTL_STM_TS_SET_TIMEOUT:
	case DMX_IOCTL_STM_CHECK_FIFO_USAGE:
	case DMX_IOCTL_PSR_RESUME:
	case DMX_IOCTL_PBBUF_ENABLE:
	case DMX_IOCTL_PBBUF_DISABLE:
	case DMX_IOCTL_PBBUF_CLEAN_BUF:
	case DMX_IOCTL_SET_CLI_CMD_INFO:
	case DMX_IOCTL_RESET:
	case DMX_IOCTL_SET_FLAG:
		rCmd = cmd;
		break;	
	default:
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("Dmx_Cmd_Switch Fail for Unsupport dwCode = 0x%x\r\n"), cmd);
		break;
	}

	pr_debug("%s exit, cmd: 0x%08x\r\n", __func__, rCmd);
	return rCmd;
}

static long get_dmx_spt_enable32(SPT_PARAM_ENABLE *kernel_ptr,
  SPT_PARAM_ENABLE32 __user *usr_ptr)
{
	compat_caddr_t tmp;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(SPT_PARAM_ENABLE32)) ||
		get_user(kernel_ptr->fgBadInterLeave, &usr_ptr->fgBadInterLeave) ||
		get_user(tmp, &usr_ptr->pvSptHdl) ||
		copy_from_user(&(kernel_ptr->rPbbufCfgInfo), &(usr_ptr->rPbbufCfgInfo),
  		sizeof(kernel_ptr->rPbbufCfgInfo)))
			return -EFAULT;
	kernel_ptr->pvSptHdl = (void __user *)compat_ptr(tmp);
	kernel_ptr->pvSptHdl = get_dmx_spt_inst_from32(kernel_ptr->pvSptHdl);
	pr_info("%s line %d, tmp: 0x%08x, pvSptHdl: 0x%p\r\n",
		__func__, __LINE__, tmp, kernel_ptr->pvSptHdl);
	return 0;
}

static long get_dmx_spt_set_rate32(SPT_PARAM_SET_RATE *kernel_ptr,
  SPT_PARAM_SET_RATE32 __user *usr_ptr)
{
	compat_caddr_t tmp;
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(SPT_PARAM_SET_RATE32)) ||
		get_user(kernel_ptr->fgDmaAud, &usr_ptr->fgDmaAud) ||
		get_user(kernel_ptr->i4Rate, &usr_ptr->i4Rate) ||
		get_user(tmp, &usr_ptr->pvSptHdl))
			return -EFAULT;
	kernel_ptr->pvSptHdl = (void __user *)compat_ptr(tmp);
	kernel_ptr->pvSptHdl = get_dmx_spt_inst_from32(kernel_ptr->pvSptHdl);
	pr_info("%s line %d, tmp: 0x%08x, pvSptHdl: 0x%p\r\n",
		__func__, __LINE__, tmp, kernel_ptr->pvSptHdl);
	return 0;
}
static long get_dmx_stm_create32(DMX_CREATE_STM_PARAM_T *kernel_ptr,
  DMX_CREATE_STM_PARAM_T32 __user *usr_ptr)
{
	compat_caddr_t tmp;
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(DMX_CREATE_STM_PARAM_T32)) ||
		get_user(kernel_ptr->rStmParam.u4StmType, &(usr_ptr->rStmParam.u4StmType)) ||
		get_user(kernel_ptr->rStmParam.u4StmUID, &usr_ptr->rStmParam.u4StmUID) ||
		get_user(kernel_ptr->rStmParam.u8DecSendBufMask, &usr_ptr->rStmParam.u8DecSendBufMask) ||
		get_user(tmp, &usr_ptr->rStmParam.pvSptHdl))
		return -EFAULT;
	kernel_ptr->rStmParam.pvSptHdl = (void __user *)compat_ptr(tmp);
	kernel_ptr->rStmParam.pvSptHdl = get_dmx_spt_inst_from32(kernel_ptr->rStmParam.pvSptHdl);
	pr_info("%s line %d, tmp: 0x%08x, rStmParam.pvSptHdl: 0x%p\r\n",
		__func__, __LINE__, tmp, kernel_ptr->rStmParam.pvSptHdl);
	return 0;
}
static long get_dmx_stm_set_fifo_sz32(STM_PARAM_SET_FIFO_SZ *kernel_ptr,
  STM_PARAM_SET_FIFO_SZ32 __user *usr_ptr)
{
	compat_caddr_t tmp;
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(STM_PARAM_SET_FIFO_SZ32)) ||
		get_user(kernel_ptr->u4Sz, &usr_ptr->u4Sz) ||
		get_user(tmp, &usr_ptr->pvStmHdl))
		return -EFAULT;
	kernel_ptr->pvStmHdl = (void __user *)compat_ptr(tmp);
	kernel_ptr->pvStmHdl = get_dmx_stm_inst_from32(kernel_ptr->pvStmHdl);
	pr_info("get_dmx_stm_set_fifo_sz32 line %d, tmp: 0x%08x, pvStmHdl: 0x%p\r\n",
		__LINE__, tmp, kernel_ptr->pvStmHdl);
	return 0;
}
static long get_dmx_stm_set_threshold32(STM_PARAM_SET_THRESHOLD *kernel_ptr,
  STM_PARAM_SET_THRESHOLD32 __user *usr_ptr)
{
	compat_caddr_t tmp;
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(STM_PARAM_SET_THRESHOLD32)) ||
		get_user(kernel_ptr->u4Threshold, &usr_ptr->u4Threshold) ||
		get_user(tmp, &usr_ptr->pvStmHdl))
		return -EFAULT;
	kernel_ptr->pvStmHdl = (void __user *)compat_ptr(tmp);
	kernel_ptr->pvStmHdl = get_dmx_stm_inst_from32(kernel_ptr->pvStmHdl);
	pr_info("get_dmx_stm_set_threshold32 line %d, tmp: 0x%08x, pvStmHdl: 0x%p\r\n",
		__LINE__, tmp, kernel_ptr->pvStmHdl);
	return 0;
}

static long get_dmx_stm_get_vidau32(ESM_IO_BUF_INFO __user *prEsmParam,
  ESM_IO_BUF_INFO32 __user *prEsmParam32)
{
  compat_uptr_t compataddr = 0;
	__u32 i = 0;

  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.eAuType),
    &(prEsmParam32->rAU.rVPicAU.eAuType),
    sizeof(AU_TYPE))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.eAuType).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.fgIBCSent),
    &(prEsmParam32->rAU.rVPicAU.fgIBCSent),
    sizeof(bool))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.fgIBCSent).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4VType),
		&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4VType), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4VType).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (get_user(compataddr, &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.ptrSAddr))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(ptrSAddr).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.ptrSAddr = (uintptr_t)compat_ptr(compataddr);
  if (get_user(compataddr, &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.ptrEAddr))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(ptrEAddr).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.ptrEAddr = (uintptr_t)compat_ptr(compataddr);
  if (get_user(compataddr, &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.ptrSVAddr))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(ptrSVAddr).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.ptrSVAddr = (uintptr_t)compat_ptr(compataddr);
  if (get_user(compataddr, &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.ptrEVAddr))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(ptrEVAddr).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.ptrEVAddr = (uintptr_t)compat_ptr(compataddr);
  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u8Pts), &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u8Pts), sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Pts).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u8PrevPTS), &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u8PrevPTS), sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8PrevPTS).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u8Dts), &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u8Dts), sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Dts).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u8Offset), &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u8Offset), sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8Offset).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u8PTSOffset), &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u8PTSOffset), sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8PTSOffset).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u8SoftPts), &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u8SoftPts), sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u8SoftPts).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (get_user(compataddr, &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.ptrSeqHdrSa))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(ptrSeqHdrSa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.ptrSeqHdrSa = (uintptr_t)compat_ptr(compataddr);
  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4SeqHdrLen), &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4SeqHdrLen), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4SeqHdrLen).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (get_user(compataddr, &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.ptrPPSHdrSa))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(ptrPPSHdrSa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.ptrPPSHdrSa = (uintptr_t)compat_ptr(compataddr);
  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4PPSHdrLen), &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4PPSHdrLen), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4PPSHdrLen).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (get_user(compataddr, &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.ptrCCSa))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(ptrCCSa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.ptrCCSa = (uintptr_t)compat_ptr(compataddr);
  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.eDiscType), &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.eDiscType), sizeof(DiscType))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(eDiscType).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4Duration), &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4Duration), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4Duration).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4PrevDuration), &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4PrevDuration), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4PrevDuration).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	for (i = 0; i < 3; i++) {
	  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4WMVSliceAddr[i]),
			&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4WMVSliceAddr[i]),
	    sizeof(__u32))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("%s line %d fail in copy_in_user(u4WMVSliceAddr[%d]).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, i);
			return -EFAULT;
		}
	}
  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.fgxvColor), &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.fgxvColor), sizeof(bool))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(fgxvColor).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4xvColorR), &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4xvColorR), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4xvColorR).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4xvColorG), &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4xvColorG), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4xvColorG).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4xvColorB), &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4xvColorB), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4xvColorB).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4EsdIndex), &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4EsdIndex), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4EsdIndex).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4EsdNums), &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4EsdNums), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4EsdNums).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4RMSliceNum), &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4RMSliceNum), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4RMSliceNum).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	for (i = 0; i < 128; i++) {
	  if (copy_in_user(&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.auRM4SliceSize[i]),
			&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.auRM4SliceSize[i]),
	    sizeof(__u32))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("%s line %d fail in copy_in_user(auRM4SliceSize[i]).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, i);
			return -EFAULT;
		}
	}

  return 0;
}

static long get_dmx_stm_get_audau32(ESM_IO_BUF_INFO __user *prEsmParam,
  ESM_IO_BUF_INFO32 __user *prEsmParam32)
{
  compat_uptr_t compataddr = 0;

  if (copy_in_user(&(prEsmParam->rAU.rAudioAU.fgSkipData),
    &(prEsmParam32->rAU.rAudioAU.fgSkipData),
    sizeof(bool))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rAudioAU.fgSkipData).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

  if (get_user(compataddr, &(prEsmParam32->rAU.rAudioAU.ptrSAddr))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in get_user(rAU.rAudioAU.ptrSAddr).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  prEsmParam->rAU.rAudioAU.ptrSAddr = (uintptr_t)compat_ptr(compataddr);
  if (get_user(compataddr, &(prEsmParam32->rAU.rAudioAU.ptrEAddr))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in get_user(rAU.rAudioAU.ptrEAddr).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  prEsmParam->rAU.rAudioAU.ptrEAddr = (uintptr_t)compat_ptr(compataddr);
  if (copy_in_user(&(prEsmParam->rAU.rAudioAU.eAuType),
	    &(prEsmParam32->rAU.rAudioAU.eAuType),
	    sizeof(AU_TYPE))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rAudioAU.eAuType).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  
  if (copy_in_user(&(prEsmParam->rAU.rAudioAU.rAUInfo.rInfo.eAudType),
		&(prEsmParam32->rAU.rAudioAU.rAUInfo.rInfo.eAudType), sizeof(AUD_TYPE))){
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rAudioAU.rInfo.eAudType).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rAudioAU.rAUInfo.rInfo.u8Pts),
		&(prEsmParam32->rAU.rAudioAU.rAUInfo.rInfo.u8Pts), sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rAudioAU.rInfo.u8Pts).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAUEx.rAudEx.rApe.fgSetSeekInfo),
    &(prEsmParam32->rAUEx.rAudEx.rApe.fgSetSeekInfo),
    sizeof(bool))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAUEx.rAudEx.rApe.fgSetSeekInfo).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

  if (copy_in_user(prEsmParam->rAUEx.rAudEx.rApe.au4SeekInfo,
    prEsmParam32->rAUEx.rAudEx.rApe.au4SeekInfo,
    sizeof(prEsmParam32->rAUEx.rAudEx.rApe.au4SeekInfo))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAUEx.rAudEx.rApe.au4SeekInfo).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
    

  return 0;
}

static long get_dmx_stm_get_spau32(ESM_IO_BUF_INFO __user *prEsmParam,
  ESM_IO_BUF_INFO32 __user *prEsmParam32)
{
  compat_uptr_t compataddr = 0;

  if (copy_in_user(&(prEsmParam->rAU.rSPStruct.u2OffTop), &(prEsmParam32->rAU.rSPStruct.u2OffTop), sizeof(__u16))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rSPStruct.u2OffTop).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rSPStruct.u2OffBottom), &(prEsmParam32->rAU.rSPStruct.u2OffBottom), sizeof(__u16))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rSPStruct.u2OffBottom).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(prEsmParam->rAU.rSPStruct.achPAL, prEsmParam32->rAU.rSPStruct.achPAL, sizeof(prEsmParam->rAU.rSPStruct.achPAL))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rSPStruct.achPAL).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rSPStruct.sx), &(prEsmParam32->rAU.rSPStruct.sx), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rSPStruct.sx).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rSPStruct.sy), &(prEsmParam32->rAU.rSPStruct.sy), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rSPStruct.sy).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rSPStruct.ex), &(prEsmParam32->rAU.rSPStruct.ex), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rSPStruct.ex).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rSPStruct.ey), &(prEsmParam32->rAU.rSPStruct.ey), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rSPStruct.ey).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rSPStruct.u4Size), &(prEsmParam32->rAU.rSPStruct.u4Size), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rSPStruct.u4Size).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

  if (get_user(compataddr, &(prEsmParam32->rAU.rSPStruct.ptrRLESWDecVirSA))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in get_user(rAU.rSPStruct.ptrRLESWDecVirSA).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  prEsmParam->rAU.rSPStruct.ptrRLESWDecVirSA = (uintptr_t)compat_ptr(compataddr);
  if (get_user(compataddr, &(prEsmParam32->rAU.rSPStruct.ptrVirSA))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in get_user(rAU.rSPStruct.ptrVirSA).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  prEsmParam->rAU.rSPStruct.ptrVirSA = (uintptr_t)compat_ptr(compataddr);
  if (get_user(compataddr, &(prEsmParam32->rAU.rSPStruct.ptrPhySA))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in get_user(rAU.rSPStruct.ptrPhySA).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  prEsmParam->rAU.rSPStruct.ptrPhySA = (uintptr_t)compat_ptr(compataddr);
  if (get_user(compataddr, &(prEsmParam32->rAU.rSPStruct.ptrUserVirSA))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in get_user(rAU.rSPStruct.ptrUserVirSA).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  prEsmParam->rAU.rSPStruct.ptrUserVirSA = (uintptr_t)compat_ptr(compataddr);
  if (get_user(compataddr, &(prEsmParam32->rAU.rSPStruct.ptrAUEndAddrOfFifo))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in get_user(rAU.rSPStruct.ptrAUEndAddrOfFifo).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  prEsmParam->rAU.rSPStruct.ptrAUEndAddrOfFifo = (uintptr_t)compat_ptr(compataddr);
  if (copy_in_user(&(prEsmParam->rAU.rSPStruct.u8StartPts), &(prEsmParam32->rAU.rSPStruct.u8StartPts),
    sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rSPStruct.u8StartPts).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rSPStruct.i8Delay), &(prEsmParam32->rAU.rSPStruct.i8Delay), sizeof(__s64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rSPStruct.i8Delay).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

  return 0;
}

static long get_dmx_stm_get_secau32(ESM_IO_BUF_INFO __user *prEsmParam,
  ESM_IO_BUF_INFO32 __user *prEsmParam32)
{
  compat_uptr_t compataddr = 0;

  if (copy_in_user(&(prEsmParam->rAU.rSectionAU.fgSkipData),
		&(prEsmParam32->rAU.rSectionAU.fgSkipData), sizeof(bool))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(fgSkipData).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

  if (get_user(compataddr, &(prEsmParam32->rAU.rSectionAU.ptrSAddr))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in get_user(ptrSAddr).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  prEsmParam->rAU.rSectionAU.ptrSAddr = (uintptr_t)compat_ptr(compataddr);
  if (get_user(compataddr, &(prEsmParam32->rAU.rSectionAU.ptrEAddr))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in get_user(ptrEAddr).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  prEsmParam->rAU.rSectionAU.ptrEAddr = (uintptr_t)compat_ptr(compataddr);
  if (get_user(compataddr, &(prEsmParam32->rAU.rSectionAU.ptrVirSA))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in get_user(ptrVirSA).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  prEsmParam->rAU.rSectionAU.ptrVirSA = (uintptr_t)compat_ptr(compataddr);
  if (get_user(compataddr, &(prEsmParam32->rAU.rSectionAU.ptrPhySA))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in get_user(ptrPhySA).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  prEsmParam->rAU.rSectionAU.ptrPhySA = (uintptr_t)compat_ptr(compataddr);
  if (get_user(compataddr, &(prEsmParam32->rAU.rSectionAU.ptrUserVirSA))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in get_user(ptrUserVirSA).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  prEsmParam->rAU.rSectionAU.ptrUserVirSA = (uintptr_t)compat_ptr(compataddr);
  
  if (copy_in_user(&(prEsmParam->rAU.rSectionAU.u4Size), &(prEsmParam32->rAU.rSectionAU.u4Size), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4Size).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam->rAU.rSectionAU.eAuType), &(prEsmParam32->rAU.rSectionAU.eAuType), sizeof(AU_TYPE))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(eAuType).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

  return 0;
}

static long get_dmx_stm_get_au32(DMX_STM_MANAGE_AU_T *kernel_ptr,
  DMX_STM_MANAGE_AU_T32 __user *usr_ptr)
{
  ESM_IO_BUF_INFO32 __user *prEsmParam32 = NULL;
	ESM_IO_BUF_INFO __user *prEsmParam = NULL;
  DMX_STM_INST_T *prStm = NULL;
  compat_caddr_t p_tmp = 0;
  compat_caddr_t tmp = 0;
  long ret = 0;

  if (!access_ok(VERIFY_READ, usr_ptr, sizeof(DMX_STM_MANAGE_AU_T32)) ||
		get_user(tmp, &usr_ptr->pvStmHdl) ||
		get_user(p_tmp, &usr_ptr->prEsmParam)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(pvStmHdl or prEsmParam).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	kernel_ptr->pvStmHdl = (void __user *)compat_ptr(tmp);
	kernel_ptr->pvStmHdl = get_dmx_stm_inst_from32(kernel_ptr->pvStmHdl);

  prEsmParam32 = (ESM_IO_BUF_INFO32 __user *)compat_ptr(p_tmp);
  if (NULL == prEsmParam32) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in compat_ptr(prEsmParam32).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (!access_ok(VERIFY_READ, prEsmParam32,
    sizeof(ESM_IO_BUF_INFO32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in access_ok(prEsmParam32: 0x%p).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, (void *)prEsmParam32);
		return -EFAULT;
	}

	prEsmParam = (ESM_IO_BUF_INFO __user *)compat_alloc_user_space(
    	sizeof(ESM_IO_BUF_INFO));
	if (NULL == prEsmParam) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in compat_alloc_user_space.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -ENOMEM;
	}
	mm_memset(prEsmParam, 0, sizeof(ESM_IO_BUF_INFO));
	prStm = (DMX_STM_INST_T *)(kernel_ptr->pvStmHdl);
	if (NULL == prStm) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail for kernel_ptr->pvStmHdl is NULL.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	if (copy_in_user(&(prEsmParam->ptrFifoSPAddr), &(prEsmParam32->ptrFifoSPAddr),
		sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(ptrFifoSPAddr).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam->ptrFifoEPAddr), &(prEsmParam32->ptrFifoEPAddr),
		sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(ptrFifoEPAddr).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam->ptrFifoSVAddr), &(prEsmParam32->ptrFifoSVAddr),
		sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(ptrFifoSVAddr).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam->ptrFifoEVAddr), &(prEsmParam32->ptrFifoEVAddr),
		sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(ptrFifoEVAddr).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam->u4Status), &(prEsmParam32->u4Status),
    	sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4Status).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam->u4TimeWait), &(prEsmParam32->u4TimeWait),
    	sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4TimeWait).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	switch (prStm->u4StmType) {
	case STREAM_AUDIO:
    ret = get_dmx_stm_get_audau32(prEsmParam, prEsmParam32);
		if (0 != ret) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			  "[DMX] %s line %d fail in get_dmx_stm_get_audau32, err: %d\r\n",
				DMX_FUNC_NAME, DMX_LINE_NO, ret);
			return ret;
		}
    break;
	case STREAM_VIDEO:
    ret = get_dmx_stm_get_vidau32(prEsmParam, prEsmParam32);
		if (0 != ret) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			  "[DMX] %s line %d fail in get_dmx_stm_get_vidau32, err: %d\r\n",
				DMX_FUNC_NAME, DMX_LINE_NO, ret);
			return ret;
		}
    break;
	case STREAM_SUBTITLE:
    ret = get_dmx_stm_get_spau32(prEsmParam, prEsmParam32);
		if (0 != ret) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			  "[DMX] %s line %d fail in get_dmx_stm_get_spau32, err: %d\r\n",
				DMX_FUNC_NAME, DMX_LINE_NO, ret);
			return ret;
		}
    break;
	case STREAM_SECTION:
    ret = get_dmx_stm_get_secau32(prEsmParam, prEsmParam32);
		if (0 != ret) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			  "[DMX] %s line %d fail in get_dmx_stm_get_secau32, err: %d\r\n",
				DMX_FUNC_NAME, DMX_LINE_NO, ret);
			return ret;
		}
    break;
	default:
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			"[DMX] %s line %d fail for inalid stream type(%d)\r\n",
      DMX_FUNC_NAME, DMX_LINE_NO, prStm->u4StmType);
    return -EINVAL;
  }

  if (0 != ret) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			"[DMX] %s line %d fail for get compat au info, stmtype(%d), ret(%ld)\r\n",
      DMX_FUNC_NAME, DMX_LINE_NO, prStm->u4StmType, ret);
    return ret;
  }

	kernel_ptr->prEsmParam = prEsmParam;

	return 0;
}

static long get_dmx_stm_destroy32(STM_PARAM_DESTROY *kernel_ptr,
  STM_PARAM_DESTROY32 __user *usr_ptr)
{
	compat_caddr_t tmp1;
	compat_caddr_t tmp2;
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(STM_PARAM_DESTROY32)) ||
		get_user(tmp1, &usr_ptr->pvSptHdl) ||
		get_user(tmp2, &usr_ptr->pvStmHdl))
		return -EFAULT;
	kernel_ptr->pvSptHdl = (void __user *)compat_ptr(tmp1);
	kernel_ptr->pvSptHdl = get_dmx_spt_inst_from32(kernel_ptr->pvSptHdl);
	kernel_ptr->pvStmHdl = (void __user *)compat_ptr(tmp2);
	kernel_ptr->pvStmHdl = get_dmx_stm_inst_from32(kernel_ptr->pvStmHdl);
	return 0;
}
static long get_dmx_stm_setuid32(STM_PARAM_SET_UID *kernel_ptr,
  STM_PARAM_SET_UID32 __user *usr_ptr)
{
	compat_caddr_t tmp;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(STM_PARAM_SET_UID32)) ||
		get_user(kernel_ptr->u4Uid, &usr_ptr->u4Uid) ||
		get_user(tmp, &usr_ptr->pvStmHdl))
		return -EFAULT;

	kernel_ptr->pvStmHdl = (void __user *)compat_ptr(tmp);
	kernel_ptr->pvStmHdl = get_dmx_stm_inst_from32(kernel_ptr->pvStmHdl);
	return 0;
}
static long get_dmx_psr_fifo_use32(DMX_PSR_FIFO_USAGE_T *kernel_ptr,
  DMX_PSR_FIFO_USAGE_T32 __user *usr_ptr)
{
	compat_caddr_t tmp;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(STM_PARAM_SET_UID32)) ||
		get_user(tmp, &usr_ptr->pvSptHdl))
		return -EFAULT;

	kernel_ptr->pvSptHdl = (void __user *)compat_ptr(tmp);
	kernel_ptr->pvSptHdl = get_dmx_spt_inst_from32(kernel_ptr->pvSptHdl);
	return 0;
}
static long get_dmx_psr_on32(DMX_PSR_ON_PARAM_T *kernel_ptr,
  DMX_PSR_ON_PARAM_T32 __user *usr_ptr)
{
	compat_caddr_t tmp;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(DMX_PSR_ON_PARAM_T32)) ||
		get_user(kernel_ptr->i4DecryptId, &usr_ptr->i4DecryptId) ||
		get_user(tmp, &usr_ptr->pvSptHdl) ||
		get_user(kernel_ptr->rStmsCnt.u4VidStmCnt, &usr_ptr->rStmsCnt.u4VidStmCnt) ||
		get_user(kernel_ptr->rStmsCnt.u4AudStmCnt, &usr_ptr->rStmsCnt.u4AudStmCnt) ||
		get_user(kernel_ptr->rStmsCnt.u4SPStmCnt, &usr_ptr->rStmsCnt.u4SPStmCnt))
		return -EFAULT;

	kernel_ptr->pvSptHdl = (void __user *)compat_ptr(tmp);
	kernel_ptr->pvSptHdl = get_dmx_spt_inst_from32(kernel_ptr->pvSptHdl);
	pr_debug("%s line %d, tmp: 0x%08x, pvSptHdl: 0x%p\r\n",
		__func__, __LINE__, tmp, kernel_ptr->pvSptHdl);
	return 0;
}
static long get_dmx_psr_file_ofst32(DMX_PSR_FILE_OFST_T *kernel_ptr,
  DMX_PSR_FILE_OFST_T32 __user *usr_ptr)
{
	compat_caddr_t tmp;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(DMX_PSR_FILE_OFST_T32)) ||
		get_user(tmp, &usr_ptr->pvSptHdl))
		return -EFAULT;

	kernel_ptr->pvSptHdl = (void __user *)compat_ptr(tmp);
	kernel_ptr->pvSptHdl = get_dmx_spt_inst_from32(kernel_ptr->pvSptHdl);
	pr_debug("%s line %d, tmp: 0x%08x, pvSptHdl: 0x%p\r\n",
		__func__, __LINE__, tmp, kernel_ptr->pvSptHdl);
	return 0;
}
static long get_dmx_cfa_set_type32(CFA_PARAM_SET_TYPE *kernel_ptr,
  CFA_PARAM_SET_TYPE32 __user *usr_ptr)
{
	compat_caddr_t tmp;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(CFA_PARAM_SET_TYPE32)) ||
		get_user(kernel_ptr->u4Type, &usr_ptr->u4Type) ||
		get_user(tmp, &usr_ptr->pvSptHdl))
		return -EFAULT;

	kernel_ptr->pvSptHdl = (void __user *)compat_ptr(tmp);
	kernel_ptr->pvSptHdl = get_dmx_spt_inst_from32(kernel_ptr->pvSptHdl);
	pr_debug("%s line %d, tmp: 0x%08x, pvSptHdl: 0x%p\r\n",
		__func__, __LINE__, tmp, kernel_ptr->pvSptHdl);
	return 0;
}
static long get_dmx_cfa_config32(CFA_PARAM_SET_CONFIG *kernel_ptr,
	CFA_PARAM_SET_CONFIG32 __user *usr_ptr, bool *pfgIsUserMem)
{
	compat_caddr_t tmp = 0;
	compat_caddr_t addr_tmp = 0;
	CFA_COMPAT_PROC_INFO_T rInfo;
	long ret = 0;
	void __user * pvConfig = NULL;

	if (NULL == pfgIsUserMem) {
		DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail for pfgIsUserMem is NULL.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(CFA_PARAM_SET_CONFIG32)) ||
		get_user(kernel_ptr->u4ConfigSz, &usr_ptr->u4ConfigSz) ||
		get_user(addr_tmp, &usr_ptr->pvConfig) ||
		get_user(tmp, &usr_ptr->pvSptHdl))
		return -EFAULT;

	kernel_ptr->pvSptHdl = (void __user *)compat_ptr(tmp);
	kernel_ptr->pvSptHdl = get_dmx_spt_inst_from32(kernel_ptr->pvSptHdl);
	pr_debug("%s line %d, tmp: 0x%08x, pvSptHdl: 0x%p\r\n",
		__func__, __LINE__, tmp, kernel_ptr->pvSptHdl);

	pvConfig = compat_ptr(addr_tmp);

	mm_memset(&rInfo, 0, sizeof(rInfo));

	rInfo.type = CFA_CONFIG;
	rInfo.is_get = false;
	rInfo.usr_ptr = NULL;
	rInfo.usr_ptr32 = pvConfig;
	rInfo.buf_sz = kernel_ptr->u4ConfigSz;

	ret = SptCfaProcCompat(kernel_ptr->pvSptHdl, &rInfo, pfgIsUserMem);
	if (0 != ret) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in SptCfaProcCompat, ret: %ld, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, ret, (void *)(kernel_ptr->pvSptHdl));
	return ret;
	}

	kernel_ptr->pvConfig = rInfo.usr_ptr;
	kernel_ptr->u4ConfigSz = rInfo.buf_sz;

	return 0;
}

static long get_dmx_cfa_set_range32(CFA_PARAM_SET_RANGE *kernel_ptr,
  CFA_PARAM_SET_RANGE32 __user *usr_ptr, bool *pfgIsUserMem)
{
	compat_caddr_t tmp;
	compat_caddr_t add_tmp;
	CFA_COMPAT_PROC_INFO_T rInfo;
	long ret = 0;
	void __user * pvRange = NULL;

	if (NULL == pfgIsUserMem) {
		DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail for pfgIsUserMem is NULL.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(CFA_PARAM_SET_RANGE32)) ||
		get_user(kernel_ptr->u4RangeSz, &usr_ptr->u4RangeSz) ||
		get_user(add_tmp, &usr_ptr->pvRange) ||
		get_user(tmp, &usr_ptr->pvSptHdl))
		return -EFAULT;

	kernel_ptr->pvSptHdl = (void __user *)compat_ptr(tmp);
	kernel_ptr->pvSptHdl = get_dmx_spt_inst_from32(kernel_ptr->pvSptHdl);

	pvRange = compat_ptr(add_tmp);

	mm_memset(&rInfo, 0, sizeof(rInfo));
	rInfo.type = CFA_RANGE;
	rInfo.is_get = false;
	rInfo.usr_ptr = NULL;
	rInfo.usr_ptr32 = pvRange;
	rInfo.buf_sz = kernel_ptr->u4RangeSz;

	ret = SptCfaProcCompat(kernel_ptr->pvSptHdl, &rInfo, pfgIsUserMem);
	if (0 != ret) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in SptCfaProcCompat, ret: %ld, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, ret, (void *)(kernel_ptr->pvSptHdl));
	return ret;
	}

	kernel_ptr->pvRange = rInfo.usr_ptr;
	kernel_ptr->u4RangeSz = rInfo.buf_sz;

	return 0;
}

static long get_dmx_cfa_set_info32(CFA_PARAM_SET_INFO *kernel_ptr,
CFA_PARAM_SET_INFO32 __user *usr_ptr, bool *pfgIsUserMem)
{
  compat_caddr_t tmp;
  compat_caddr_t addr_tmp;
  CFA_COMPAT_PROC_INFO_T rInfo;
  long ret = 0;
  void __user * pvSetInfo = NULL;

	if (NULL == pfgIsUserMem) {
		DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail for pfgIsUserMem is NULL.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(CFA_PARAM_SET_INFO32)) ||
		get_user(kernel_ptr->u4CfaQID, &usr_ptr->u4CfaQID) ||
		get_user(kernel_ptr->u4ParamSize, &usr_ptr->u4ParamSize) ||
		get_user(tmp, &usr_ptr->pvSptHdl) ||
		get_user(addr_tmp, &usr_ptr->pvCfaParam))
		return -EFAULT;

	kernel_ptr->pvSptHdl = (void __user *)compat_ptr(tmp);
	kernel_ptr->pvSptHdl = get_dmx_spt_inst_from32(kernel_ptr->pvSptHdl);
	pr_debug("%s line %d, tmp: 0x%08x, pvSptHdl: 0x%p\r\n",
		__func__, __LINE__, tmp, kernel_ptr->pvSptHdl);

	pvSetInfo = compat_ptr(addr_tmp);

  mm_memset(&rInfo, 0, sizeof(rInfo));
  rInfo.type = CFA_GEN_INFO;
  rInfo.is_get = false;
  rInfo.usr_ptr = NULL;
  rInfo.usr_ptr32 = pvSetInfo;
  rInfo.buf_sz = kernel_ptr->u4ParamSize;

  ret = SptCfaProcCompat(kernel_ptr->pvSptHdl, &rInfo, pfgIsUserMem);
  if (0 != ret) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in SptCfaProcCompat, ret: %ld, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, ret, (void *)(kernel_ptr->pvSptHdl));
    return ret;
  }

  kernel_ptr->pvCfaParam = rInfo.usr_ptr;
  kernel_ptr->u4ParamSize = rInfo.buf_sz;
	
	return 0;
}

static long get_dmx_send_pbbuf32(void *pvSptHdl,
  SEND_BUFFER __user *usr_ptr,
  SEND_BUFFER32 __user *usr_ptr32, bool *pfgIsUserMem)
{
	compat_caddr_t tmp;
	compat_caddr_t add_tmp;
	u8 __user *pcBuf;
	void __user *pvBuf;
	PBBUF_SLOT_HEADER_INFO_T32 rHeadInf;
	void __user *pvHdrParam = NULL;
	CFA_COMPAT_PROC_INFO_T rInfo;
	long ret = 0;

	if (NULL == pfgIsUserMem) {
		DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail for pfgIsUserMem is NULL.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(usr_ptr, usr_ptr32, 6 * sizeof(u32)))
		return -EFAULT;
	if (get_user(add_tmp, &usr_ptr32->pcBuffer))
		return -EFAULT;
	pcBuf = (u8 __user *)compat_ptr(add_tmp);
	if (put_user(pcBuf, &usr_ptr->pcBuffer))
		return -EFAULT;

	if (get_user(tmp, &usr_ptr32->pvBuffer))
		return -EFAULT;
	pvBuf = (void __user *)compat_ptr(tmp);
	if (put_user(pvBuf, &usr_ptr->pvBuffer))
		return -EFAULT;

	if (get_user(usr_ptr->u8AlignedIdx, &usr_ptr32->u8AlignedIdx) ||
		get_user(usr_ptr->u8IssueLen, &usr_ptr32->u8IssueLen) ||
		get_user(usr_ptr->u8SrcOffset, &usr_ptr32->u8SrcOffset) ||
		get_user(rHeadInf.eType, &(usr_ptr32->rHeader.eType)) ||
		get_user(rHeadInf.u4ParamSz, &(usr_ptr32->rHeader.u4ParamSz)) ||
		get_user(rHeadInf.pvParam, &(usr_ptr32->rHeader.pvParam)))
		return -EFAULT;

	if ((compat_caddr_t)0 != rHeadInf.pvParam) {
		pvHdrParam = compat_ptr(rHeadInf.pvParam);
		mm_memset(&rInfo, 0, sizeof(rInfo));
		rInfo.type = CFA_JUMP_INFO;
		rInfo.is_get = false;
		rInfo.usr_ptr = NULL;
		rInfo.usr_ptr32 = pvHdrParam;
		rInfo.buf_sz = rHeadInf.u4ParamSz;
		ret = SptCfaProcCompat(pvSptHdl, &rInfo, pfgIsUserMem);
		if (0 != ret) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in SptCfaProcCompat, ret: %ld, pvSptHdl: 0x%p\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, ret, (pvSptHdl));
			return ret;
		}
		
		usr_ptr->rHeader.pvParam = rInfo.usr_ptr;
		usr_ptr->rHeader.u4ParamSz = rInfo.buf_sz;
	}

	return 0;
}

static long get_dmx_pbbuf_send_buf32(DMX_PBBUF_SEND_BUF_T *kernel_ptr,
  DMX_PBBUF_SEND_BUF_T32 __user *usr_ptr, bool *pfgIsUserMem)
{
	PBBUF_PARAM_SEND_BUF32 rtmp;
	SEND_BUFFER32 __user *send_buffer32;
	SEND_BUFFER __user *send_buffer;
	int ret;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(DMX_PBBUF_SEND_BUF_T32)) ||
		get_user(kernel_ptr->fgExitSent, &usr_ptr->fgExitSent) ||
		copy_from_user(&rtmp, &(usr_ptr->rBufParam), sizeof(PBBUF_PARAM_SEND_BUF32)))
		return -EFAULT;

	kernel_ptr->rBufParam.pvSptHdl = (void __user *)compat_ptr(rtmp.pvSptHdl);
	kernel_ptr->rBufParam.pvSptHdl = get_dmx_spt_inst_from32(kernel_ptr->rBufParam.pvSptHdl);
	send_buffer32 = (SEND_BUFFER32 __user *)compat_ptr(rtmp.prBUF);

	if (!access_ok(VERIFY_READ, send_buffer32, sizeof(SEND_BUFFER32)))
		return -EFAULT;

	send_buffer = (SEND_BUFFER __user *)compat_alloc_user_space(
    	sizeof(SEND_BUFFER));
	if (NULL == send_buffer)
		return -ENOMEM;
	mm_memset(send_buffer, 0, sizeof(SEND_BUFFER));

	ret = get_dmx_send_pbbuf32(kernel_ptr->rBufParam.pvSptHdl,
    send_buffer, send_buffer32, pfgIsUserMem);
	if (ret)
		return ret;

	kernel_ptr->rBufParam.prBUF = (__force SEND_BUFFER *)send_buffer;
	
	return 0;
}

static long get_dmx_send_pbbuf_param32(PBBUF_PARAM_SEND_BUF *kernel_ptr,
  PBBUF_PARAM_SEND_BUF32 __user *usr_ptr, bool *pfgIsUserMem)
{
	compat_caddr_t tmp;
	compat_caddr_t add_tmp;
	SEND_BUFFER32 __user *send_buffer32;
	SEND_BUFFER __user *send_buffer;
	int ret;

	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(PBBUF_PARAM_SEND_BUF32)) ||
		get_user(tmp, &usr_ptr->pvSptHdl) ||
		get_user(add_tmp, &(usr_ptr->prBUF)))
		return -EFAULT;

	kernel_ptr->pvSptHdl = (void __user *)compat_ptr(tmp);
	kernel_ptr->pvSptHdl = get_dmx_spt_inst_from32(kernel_ptr->pvSptHdl);

	send_buffer32 = (SEND_BUFFER32 __user *)compat_ptr(add_tmp);

	if (!access_ok(VERIFY_READ, send_buffer32, sizeof(SEND_BUFFER32)))
		return -EFAULT;

	send_buffer = (SEND_BUFFER *)compat_alloc_user_space(sizeof(SEND_BUFFER));
	if (NULL == send_buffer)
		return -ENOMEM;
	mm_memset(send_buffer, 0, sizeof(SEND_BUFFER));

	ret = get_dmx_send_pbbuf32(kernel_ptr->pvSptHdl, send_buffer, send_buffer32, pfgIsUserMem);
	if (ret)
		return ret;

	kernel_ptr->prBUF = (__force SEND_BUFFER *)send_buffer;
	
	return 0;
}

static long get_dmx_pbbuf_no_data32(DMX_PBBUF_NODATA_PARAM_T *kernel_ptr,
  DMX_PBBUF_NODATA_PARAM_T32 __user *usr_ptr)
{
	compat_caddr_t tmp;
	
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(DMX_PBBUF_NODATA_PARAM_T32)) ||
		get_user(kernel_ptr->u4Status, &usr_ptr->u4Status) ||
		get_user(tmp, &usr_ptr->pvSptHdl))
		return -EFAULT;
	kernel_ptr->pvSptHdl = (void __user *)compat_ptr(tmp);
	kernel_ptr->pvSptHdl = get_dmx_spt_inst_from32(kernel_ptr->pvSptHdl);
	return 0;
}
static long get_dmx_spt_rsp_on32(SPLITTER_PTX_RSP_ON_INFO_T *kernel_ptr,
  SPLITTER_PTX_RSP_ON_INFO_T32 __user *usr_ptr)
{
	compat_caddr_t tmp;
	
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(SPLITTER_PTX_RSP_ON_INFO_T32)) ||
		get_user(kernel_ptr->fgRebuf, &usr_ptr->fgRebuf) ||
		get_user(kernel_ptr->fgByPassSp, &usr_ptr->fgByPassSp) ||
		get_user(tmp, &usr_ptr->pvSptHdl) ||
		get_user(kernel_ptr->u8PtsDelay, &usr_ptr->u8PtsDelay))
		return -EFAULT;
	
	kernel_ptr->pvSptHdl = (void __user *)compat_ptr(tmp);
	kernel_ptr->pvSptHdl = get_dmx_spt_inst_from32(kernel_ptr->pvSptHdl);
	return 0;
}
static long get_dmx_spt_rsp_off32(SPLITTER_PTX_RSP_OFF_INFO_T *kernel_ptr,
  SPLITTER_PTX_RSP_OFF_INFO_T32 __user *usr_ptr)
{
	compat_caddr_t tmp;
	
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(SPLITTER_PTX_RSP_OFF_INFO_T32)) ||
		get_user(kernel_ptr->fgCurPbPause, &usr_ptr->fgCurPbPause) ||
		get_user(kernel_ptr->ucState, &usr_ptr->ucState) ||
		get_user(kernel_ptr->ucRspTxRet, &usr_ptr->ucRspTxRet) ||
		get_user(kernel_ptr->ucRspTxType, &usr_ptr->ucRspTxType) ||
		get_user(kernel_ptr->ucRspTxUid, &usr_ptr->ucRspTxUid) ||
		get_user(kernel_ptr->ucRspMode, &usr_ptr->ucRspMode) ||
		get_user(tmp, &usr_ptr->pvSptHdl))
		return -EFAULT;
	
	kernel_ptr->pvSptHdl = (void __user *)compat_ptr(tmp);
	kernel_ptr->pvSptHdl = get_dmx_spt_inst_from32(kernel_ptr->pvSptHdl);
	return 0;
}
static long get_dmx_spt_rebuf_range32(SPLITTER_PTX_REBUFFER_RANGE_INFO_T *kernel_ptr, 
	SPLITTER_PTX_REBUFFER_RANGE_INFO_T32 __user *usr_ptr)
{
	compat_caddr_t tmp;
	
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(SPLITTER_PTX_REBUFFER_RANGE_INFO_T32)) ||
		get_user(kernel_ptr->fgRebuf, &usr_ptr->fgRebuf) ||
		get_user(tmp, &usr_ptr->pvSptHdl) ||
		get_user(kernel_ptr->u8PtsDelay, &usr_ptr->u8PtsDelay) ||
		get_user(kernel_ptr->u8RspStartPts, &usr_ptr->u8RspStartPts) ||
		get_user(kernel_ptr->u8RspStartOffset, &usr_ptr->u8RspStartOffset) ||
		get_user(kernel_ptr->u8PbbStartOffset, &usr_ptr->u8PbbStartOffset))
		return -EFAULT;
	
	kernel_ptr->pvSptHdl = (void __user *)compat_ptr(tmp);
	kernel_ptr->pvSptHdl = get_dmx_spt_inst_from32(kernel_ptr->pvSptHdl);
	return 0;
}

//>TODO: Need to modify, pvOperParam
static long get_dmx_decrypt_oper32(DECRYPT_OPER_PARAM_T *kernel_ptr,
  DECRYPT_OPER_PARAM_T32 __user *usr_ptr)
{
	compat_caddr_t tmp_inst;
	compat_caddr_t tmp_oper;
	
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(DECRYPT_OPER_PARAM_T32)) ||
		get_user(kernel_ptr->u4OperParamSz, &usr_ptr->u4OperParamSz) ||
		get_user(kernel_ptr->u4OperCode, &usr_ptr->u4OperCode) ||
		get_user(tmp_inst, &usr_ptr->pvInst) ||
		get_user(tmp_oper, &usr_ptr->pvOperParam))
		return -EFAULT;
	
	kernel_ptr->pvInst = (__force void *)compat_ptr(tmp_inst);
	kernel_ptr->pvOperParam = (__force void *)compat_ptr(tmp_oper);
	return 0;
}

static long get_dmx_decrypt_inst32(DECRYPT_INST_PARAM_T *kernel_ptr,
  DECRYPT_INST_PARAM_T32 __user *usr_ptr)
{
	compat_caddr_t tmp_inst;
	
	if (!access_ok(VERIFY_READ, usr_ptr, sizeof(DECRYPT_INST_PARAM_T32)) ||
		get_user(kernel_ptr->eDecryptType, &usr_ptr->eDecryptType) ||
		get_user(tmp_inst, &usr_ptr->pvInst))
		return -EFAULT;
	
	kernel_ptr->pvInst = (__force void *)compat_ptr(tmp_inst);
	return 0;
}

static int put_dmx_stm_create32(DMX_CREATE_STM_PARAM_T *kernel_ptr,
  DMX_CREATE_STM_PARAM_T32 __user *usr_ptr)
{
	compat_caddr_t tmp;
	
	if (!access_ok(VERIFY_WRITE, usr_ptr, sizeof(DMX_CREATE_STM_PARAM_T32)))
		return -EFAULT;

	tmp = (compat_caddr_t)ptr_to_compat(kernel_ptr->pvStmHdl);

	if (put_user(tmp, &(usr_ptr->pvStmHdl)))
		return -EFAULT;

	return 0;
}

static int put_dmx_psr_fifo_use32(DMX_PSR_FIFO_USAGE_T *kernel_ptr,
  DMX_PSR_FIFO_USAGE_T32 __user *usr_ptr)
{
	if (!access_ok(VERIFY_WRITE, usr_ptr, sizeof(DMX_PSR_FIFO_USAGE_T32)) ||
		put_user(kernel_ptr->u4Fifo, &(usr_ptr->u4Fifo)))
		return -EFAULT;

	return 0;
}
static int put_dmx_psr_file_ofst32(DMX_PSR_FILE_OFST_T *kernel_ptr,
  DMX_PSR_FILE_OFST_T32 __user *usr_ptr)
{
	if (!access_ok(VERIFY_WRITE, usr_ptr, sizeof(DMX_PSR_FILE_OFST_T32)) ||
		put_user(kernel_ptr->u8FileOfst, &(usr_ptr->u8FileOfst)))
		return -EFAULT;

	return 0;
}
static int put_dmx_spt_rsp_off32(SPLITTER_PTX_RSP_OFF_INFO_T *kernel_ptr,
  SPLITTER_PTX_RSP_OFF_INFO_T32 __user *usr_ptr)
{
	compat_caddr_t tmp = 0;

	if (!access_ok(VERIFY_WRITE, usr_ptr, sizeof(SPLITTER_PTX_RSP_OFF_INFO_T32)) ||
		put_user(kernel_ptr->fgCurPbPause, &(usr_ptr->fgCurPbPause)) ||
		put_user(kernel_ptr->ucState, &(usr_ptr->ucState)) ||
		put_user(kernel_ptr->ucRspTxRet, &(usr_ptr->ucRspTxRet)) ||
		put_user(kernel_ptr->ucRspTxType, &(usr_ptr->ucRspTxType)) ||
		put_user(kernel_ptr->ucRspTxUid, &(usr_ptr->ucRspTxUid)) ||
		put_user(kernel_ptr->ucRspMode, &(usr_ptr->ucRspMode)))
		return -EFAULT;

	tmp = (compat_caddr_t)ptr_to_compat(kernel_ptr->pvSptHdl);

  if (put_user(tmp, &(usr_ptr->pvSptHdl)))
    return -EFAULT;

	return 0;
}
static int put_dmx_spt_rebuf_range32(SPLITTER_PTX_REBUFFER_RANGE_INFO_T *kernel_ptr,
  SPLITTER_PTX_REBUFFER_RANGE_INFO_T32 __user *usr_ptr)
{
	compat_caddr_t tmp = 0;
	
	if (!access_ok(VERIFY_WRITE, usr_ptr, sizeof(SPLITTER_PTX_REBUFFER_RANGE_INFO_T32)) ||
		put_user(kernel_ptr->fgRebuf, &usr_ptr->fgRebuf) ||
		put_user(kernel_ptr->u8PtsDelay, &usr_ptr->u8PtsDelay) ||
		put_user(kernel_ptr->u8RspStartPts, &usr_ptr->u8RspStartPts) ||
		put_user(kernel_ptr->u8RspStartOffset, &usr_ptr->u8RspStartOffset) ||
		put_user(kernel_ptr->u8PbbStartOffset, &usr_ptr->u8PbbStartOffset))
		return -EFAULT;
	
	tmp = (compat_caddr_t)ptr_to_compat(kernel_ptr->pvSptHdl);

  if (put_user(tmp, &(usr_ptr->pvSptHdl)))
    return -EFAULT;

	return 0;
}

static int put_dmx_send_buf32(SEND_BUFFER *kernel_ptr,
SEND_BUFFER32 __user *usr_ptr32)
{
	compat_caddr_t tmp;
	compat_caddr_t addr_tmp;
	
	if (put_user(kernel_ptr->u4BufferSize, &(usr_ptr32->u4BufferSize)))
		return -EFAULT;
	if (put_user(kernel_ptr->u4DataOffset, &(usr_ptr32->u4DataOffset)))
		return -EFAULT;
	if (put_user(kernel_ptr->u4DataSize, &(usr_ptr32->u4DataSize)))
		return -EFAULT;
	if (put_user(kernel_ptr->u4PlayOffset, &(usr_ptr32->u4PlayOffset)))
		return -EFAULT;
	if (put_user(kernel_ptr->u4PlaySize, &(usr_ptr32->u4PlaySize)))
		return -EFAULT;
	if (put_user(kernel_ptr->u4SessionID, &(usr_ptr32->u4SessionID)))
		return -EFAULT;

	addr_tmp = (compat_caddr_t)ptr_to_compat(kernel_ptr->pcBuffer);
	if (put_user(addr_tmp, &(usr_ptr32->pcBuffer)))
    	return -EFAULT;

	tmp = (compat_uptr_t)ptr_to_compat(kernel_ptr->pvBuffer);
	if (put_user(tmp, &(usr_ptr32->pvBuffer)))
    	return -EFAULT;

	if (put_user(kernel_ptr->u8SrcOffset, &(usr_ptr32->u8SrcOffset)) ||
		put_user(kernel_ptr->u8AlignedIdx, &(usr_ptr32->u8AlignedIdx)) ||
		put_user(kernel_ptr->u8IssueLen, &(usr_ptr32->u8IssueLen)))
		return -EFAULT;

	if (put_user(kernel_ptr->rHeader.eType, &(usr_ptr32->rHeader.eType)) ||
		put_user(kernel_ptr->rHeader.u4ParamSz, &(usr_ptr32->rHeader.u4ParamSz)))
		return -EFAULT;
	
	return 0;
}

static int put_dmx_send_buf_param32(PBBUF_PARAM_SEND_BUF *kernel_ptr,
  PBBUF_PARAM_SEND_BUF32 __user *usr_ptr)
{
	int ret = 0;
	compat_caddr_t p = 0;
	SEND_BUFFER32 __user *send_buffer32 = NULL;
	
	if (!access_ok(VERIFY_WRITE | VERIFY_READ, usr_ptr, sizeof(PBBUF_PARAM_SEND_BUF32)))
		return -EFAULT;

	if (get_user(p, &usr_ptr->prBUF))
		return -EFAULT;
	send_buffer32 = compat_ptr(p);

	if (!access_ok(VERIFY_WRITE | VERIFY_READ, send_buffer32,
    sizeof(SEND_BUFFER32)))
		return -EFAULT;

	ret = put_dmx_send_buf32(kernel_ptr->prBUF, send_buffer32);
	if (ret)
		return ret;
	
	return 0;
}

static long put_dmx_stm_get_vidau32(ESM_IO_BUF_INFO __user *prEsmParam,
  ESM_IO_BUF_INFO32 __user *prEsmParam32)
{
	compat_uptr_t compataddr = 0;
	u32 i = 0;

	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.eAuType),
		&(prEsmParam->rAU.rVPicAU.eAuType),
    	sizeof(AU_TYPE))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.eAuType).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.fgIBCSent),
    	&(prEsmParam->rAU.rVPicAU.fgIBCSent),
    	sizeof(bool))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.fgIBCSent).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4VType),
		&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4VType), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.u4VType).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	compataddr = (compat_uptr_t)ptr_to_compat((void *)prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.ptrSAddr);
	if (put_user(compataddr, &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.ptrSAddr))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in put_user(rAU.rVPicAU.rAUInfo.rInfo.ptrSAddr).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	compataddr = (compat_uptr_t)ptr_to_compat((void *)prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.ptrEAddr);
	if (put_user(compataddr, &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.ptrEAddr))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in put_user(rAU.rVPicAU.rAUInfo.rInfo.ptrEAddr).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	compataddr = (compat_uptr_t)ptr_to_compat((void *)prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.ptrSVAddr);
	if (put_user(compataddr, &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.ptrSVAddr))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in put_user(rAU.rVPicAU.rAUInfo.rInfo.ptrSVAddr).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	compataddr = (compat_uptr_t)ptr_to_compat((void *)prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.ptrEVAddr);
	if (put_user(compataddr, &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.ptrEVAddr))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in put_user(rAU.rVPicAU.rAUInfo.rInfo.ptrEVAddr).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u8Pts),
		&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u8Pts), sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.u8Pts).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u8PrevPTS),
		&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u8PrevPTS), sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.u8PrevPTS).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u8Dts),
		&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u8Dts), sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.u8Dts).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u8Offset),
		&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u8Offset), sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.u8Offset).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u8PTSOffset),
		&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u8PTSOffset), sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.u8PTSOffset).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u8SoftPts),
		&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u8SoftPts), sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.u8Pts).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4SeqHdrLen),
		&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4SeqHdrLen), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.u4SeqHdrLen).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	compataddr = (compat_uptr_t)ptr_to_compat((void *)prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.ptrSeqHdrSa);
	if (put_user(compataddr, &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.ptrSeqHdrSa))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.ptrSeqHdrSa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	compataddr = (compat_uptr_t)ptr_to_compat((void *)prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.ptrPPSHdrSa);
	if (put_user(compataddr, &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.ptrPPSHdrSa))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.ptrPPSHdrSa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4PPSHdrLen),
		&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4PPSHdrLen), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.u4PPSHdrLen).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	compataddr = (compat_uptr_t)ptr_to_compat((void *)prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.ptrCCSa);
	if (put_user(compataddr, &(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.ptrCCSa))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.ptrCCSa).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.eDiscType),
		&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.eDiscType), sizeof(DiscType))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.eDiscType).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4Duration),
		&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4Duration), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.u4Duration).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4PrevDuration),
		&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4PrevDuration), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.u4PrevDuration).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	for (i = 0; i < 3; i++) {
		if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4WMVSliceAddr[i]),
			&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4WMVSliceAddr[i]),
			sizeof(__u32))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.u4WMVSliceAddr[%d]).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, i);
			return -EFAULT;
		}
	}
	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.fgxvColor),
		&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.fgxvColor), sizeof(bool))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.fgxvColor).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4xvColorR),
		&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4xvColorR), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.u4xvColorR).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4xvColorG),
		&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4xvColorG), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.u4xvColorG).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4xvColorB),
		&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4xvColorB), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.u4xvColorB).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4EsdIndex),
		&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4EsdIndex), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.u4EsdIndex).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4EsdNums),
		&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4EsdNums), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.u4EsdNums).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.u4RMSliceNum),
		&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.u4RMSliceNum), sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.u4RMSliceNum).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
	for (i = 0; i < 128; i++) {
		if (copy_in_user(&(prEsmParam32->rAU.rVPicAU.rAUInfo.rInfo.auRM4SliceSize[i]),
			&(prEsmParam->rAU.rVPicAU.rAUInfo.rInfo.auRM4SliceSize[i]),
			sizeof(__u32))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("%s line %d fail in copy_in_user(rAU.rVPicAU.rAUInfo.rInfo.auRM4SliceSize[%d]).\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, i);
			return -EFAULT;
		}
	}

	return 0;
}

static long put_dmx_stm_get_audau32(ESM_IO_BUF_INFO __user *prEsmParam,
  ESM_IO_BUF_INFO32 __user *prEsmParam32)
{
	compat_uptr_t compataddr = 0;

	if (copy_in_user(&(prEsmParam32->rAU.rAudioAU.fgSkipData),
		&(prEsmParam->rAU.rAudioAU.fgSkipData), sizeof(bool)))
	return -EFAULT;

	compataddr = (compat_uptr_t)ptr_to_compat((void *)prEsmParam->rAU.rAudioAU.ptrSAddr);
	if (put_user(compataddr, &(prEsmParam32->rAU.rAudioAU.ptrSAddr)))
		return -EFAULT;
	compataddr = (compat_uptr_t)ptr_to_compat((void *)prEsmParam->rAU.rAudioAU.ptrEAddr);
	if (put_user(compataddr, &(prEsmParam32->rAU.rAudioAU.ptrEAddr)))
		return -EFAULT;
	if (copy_in_user(&(prEsmParam32->rAU.rAudioAU.eAuType),
		&(prEsmParam->rAU.rAudioAU.eAuType),
		sizeof(AU_TYPE)))
		return -EFAULT;

	if (copy_in_user(&(prEsmParam32->rAU.rAudioAU.rAUInfo.rInfo.eAudType),
		&(prEsmParam->rAU.rAudioAU.rAUInfo.rInfo.eAudType), sizeof(AUD_TYPE)))
		return -EFAULT;
	if (copy_in_user(&(prEsmParam32->rAU.rAudioAU.rAUInfo.rInfo.u8Pts),
		&(prEsmParam->rAU.rAudioAU.rAUInfo.rInfo.u8Pts), sizeof(__u64)))
		return -EFAULT;
	if (copy_in_user(&(prEsmParam32->rAUEx.rAudEx.rApe.fgSetSeekInfo),
		&(prEsmParam->rAUEx.rAudEx.rApe.fgSetSeekInfo),
		sizeof(bool)))
		return -EFAULT;

	if (copy_in_user(prEsmParam32->rAUEx.rAudEx.rApe.au4SeekInfo,
		prEsmParam->rAUEx.rAudEx.rApe.au4SeekInfo,
		sizeof(prEsmParam->rAUEx.rAudEx.rApe.au4SeekInfo)))
		return -EFAULT;

	return 0;
}

static long put_dmx_stm_get_spau32(ESM_IO_BUF_INFO __user *prEsmParam,
  ESM_IO_BUF_INFO32 __user *prEsmParam32)
{
  compat_uptr_t compataddr = 0;

  if (copy_in_user(&(prEsmParam32->rAU.rSPStruct.u2OffTop), &(prEsmParam->rAU.rSPStruct.u2OffTop), sizeof(__u16)))
    return -EFAULT;
  if (copy_in_user(&(prEsmParam32->rAU.rSPStruct.u2OffBottom), &(prEsmParam->rAU.rSPStruct.u2OffBottom), sizeof(__u16)))
    return -EFAULT;
  if (copy_in_user(prEsmParam32->rAU.rSPStruct.achPAL, prEsmParam->rAU.rSPStruct.achPAL,
		sizeof(prEsmParam->rAU.rSPStruct.achPAL)))
    return -EFAULT;
  if (copy_in_user(&(prEsmParam32->rAU.rSPStruct.sx), &(prEsmParam->rAU.rSPStruct.sx),
		sizeof(__u32)))
    return -EFAULT;
  if (copy_in_user(&(prEsmParam32->rAU.rSPStruct.sy), &(prEsmParam->rAU.rSPStruct.sy),
		sizeof(__u32)))
    return -EFAULT;
  if (copy_in_user(&(prEsmParam32->rAU.rSPStruct.ex), &(prEsmParam->rAU.rSPStruct.ex),
		sizeof(__u32)))
    return -EFAULT;
  if (copy_in_user(&(prEsmParam32->rAU.rSPStruct.ey), &(prEsmParam->rAU.rSPStruct.ey),
		sizeof(__u32)))
    return -EFAULT;
  if (copy_in_user(&(prEsmParam32->rAU.rSPStruct.u4Size), &(prEsmParam->rAU.rSPStruct.u4Size), 
		sizeof(__u32)))
    return -EFAULT;

  compataddr = (compat_uptr_t)ptr_to_compat((void *)prEsmParam->rAU.rSPStruct.ptrRLESWDecVirSA);
  if (put_user(compataddr, &(prEsmParam32->rAU.rSPStruct.ptrRLESWDecVirSA)))
    return -EFAULT;
  compataddr = (compat_uptr_t)ptr_to_compat((void *)prEsmParam->rAU.rSPStruct.ptrVirSA);
  if (put_user(compataddr, &(prEsmParam32->rAU.rSPStruct.ptrVirSA)))
    return -EFAULT;
  compataddr = (compat_uptr_t)ptr_to_compat((void *)prEsmParam->rAU.rSPStruct.ptrPhySA);
  if (put_user(compataddr, &(prEsmParam32->rAU.rSPStruct.ptrPhySA)))
    return -EFAULT;
  compataddr = (compat_uptr_t)ptr_to_compat((void *)prEsmParam->rAU.rSPStruct.ptrUserVirSA);
  if (put_user(compataddr, &(prEsmParam32->rAU.rSPStruct.ptrUserVirSA)))
    return -EFAULT;
  compataddr = (compat_uptr_t)ptr_to_compat((void *)prEsmParam->rAU.rSPStruct.ptrAUEndAddrOfFifo);
  if (put_user(compataddr, &(prEsmParam32->rAU.rSPStruct.ptrAUEndAddrOfFifo)))
    return -EFAULT;
  if (copy_in_user(&(prEsmParam32->rAU.rSPStruct.u8StartPts),
		&(prEsmParam->rAU.rSPStruct.u8StartPts),
    sizeof(__u64)))
    return -EFAULT;
  if (copy_in_user(&(prEsmParam32->rAU.rSPStruct.i8Delay),
		&(prEsmParam->rAU.rSPStruct.i8Delay), sizeof(__s64)))
    return -EFAULT;

  return 0;
}

static long put_dmx_stm_get_secau32(ESM_IO_BUF_INFO __user *prEsmParam,
  ESM_IO_BUF_INFO32 __user *prEsmParam32)
{
  compat_uptr_t compataddr = 0;

  if (copy_in_user(&(prEsmParam->rAU.rSectionAU.fgSkipData),
		&(prEsmParam32->rAU.rSectionAU.fgSkipData), sizeof(bool)))
    return -EFAULT;

  compataddr = (compat_uptr_t)ptr_to_compat((void *)prEsmParam->rAU.rSectionAU.ptrSAddr);
  if (put_user(compataddr, &(prEsmParam32->rAU.rSectionAU.ptrSAddr)))
    return -EFAULT;
  compataddr = (compat_uptr_t)ptr_to_compat((void *)prEsmParam->rAU.rSectionAU.ptrEAddr);
  if (put_user(compataddr, &(prEsmParam32->rAU.rSectionAU.ptrEAddr)))
    return -EFAULT;
  compataddr = (compat_uptr_t)ptr_to_compat((void *)prEsmParam->rAU.rSectionAU.ptrVirSA);
  if (put_user(compataddr, &(prEsmParam32->rAU.rSectionAU.ptrVirSA)))
    return -EFAULT;
  compataddr = (compat_uptr_t)ptr_to_compat((void *)prEsmParam->rAU.rSectionAU.ptrPhySA);
  if (put_user(compataddr, &(prEsmParam32->rAU.rSectionAU.ptrPhySA)))
    return -EFAULT;
  compataddr = (compat_uptr_t)ptr_to_compat((void *)prEsmParam->rAU.rSectionAU.ptrUserVirSA);
  if (put_user(compataddr, &(prEsmParam32->rAU.rSectionAU.ptrUserVirSA)))
    return -EFAULT;

  if (copy_in_user(&(prEsmParam32->rAU.rSectionAU.u4Size),
		&(prEsmParam->rAU.rSectionAU.u4Size), sizeof(__u32)))
    return -EFAULT;
  if (copy_in_user(&(prEsmParam32->rAU.rSectionAU.eAuType),
		&(prEsmParam->rAU.rSectionAU.eAuType), sizeof(AU_TYPE)))
    return -EFAULT;

  return 0;
}

static long put_dmx_stm_get_au32(DMX_STM_MANAGE_AU_T *kernel_ptr,
  DMX_STM_MANAGE_AU_T32 __user *usr_ptr)
{
  ESM_IO_BUF_INFO32 __user *prEsmParam32 = NULL;
	ESM_IO_BUF_INFO __user *prEsmParam = NULL;
  DMX_STM_INST_T *prStm = NULL;
  long ret = 0;

  if (!access_ok(VERIFY_READ | VERIFY_WRITE,
    usr_ptr, sizeof(DMX_STM_MANAGE_AU_T32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in access_ok(DMX_STM_MANAGE_AU_T32).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

  prStm = (DMX_STM_INST_T *)(kernel_ptr->pvStmHdl);
  if (NULL == prStm) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail for invalid prStm.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

  prEsmParam = (ESM_IO_BUF_INFO __user *)kernel_ptr->prEsmParam;
  prEsmParam32 = (ESM_IO_BUF_INFO32 __user *)compat_ptr(usr_ptr->prEsmParam);
  if (!access_ok(VERIFY_READ | VERIFY_WRITE, prEsmParam32,
    sizeof(ESM_IO_BUF_INFO32))) {
	 DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		 TEXT("%s line %d fail in access_ok(ESM_IO_BUF_INFO32: 0x%p).\r\n"),
		 DMX_FUNC_NAME, DMX_LINE_NO, prEsmParam32);
	 return -EFAULT;
 }
    

  if (copy_in_user(&(prEsmParam32->ptrFifoSPAddr), &(prEsmParam->ptrFifoSPAddr), sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(ptrFifoSPAddr).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  
  if (copy_in_user(&(prEsmParam32->ptrFifoEPAddr), &(prEsmParam->ptrFifoEPAddr), sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(ptrFifoEPAddr).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

  if (copy_in_user(&(prEsmParam32->ptrFifoSVAddr), &(prEsmParam->ptrFifoSVAddr), sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in put_user(ptrFifoSVAddr).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam32->ptrFifoEVAddr), &(prEsmParam->ptrFifoEVAddr), sizeof(__u64))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in put_user(ptrFifoEVAddr).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

  switch (prStm->u4StmType) {
  case STREAM_AUDIO:
    ret = put_dmx_stm_get_audau32(prEsmParam, prEsmParam32);
		if (ret != 0) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("%s line %d fail in put_dmx_stm_get_audau32, ret: %d.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, ret);
	  }
    break;
  case STREAM_VIDEO:
    ret = put_dmx_stm_get_vidau32(prEsmParam, prEsmParam32);
		if (ret != 0) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("%s line %d fail in put_dmx_stm_get_vidau32, ret: %d.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, ret);
	  }
    break;
  case STREAM_SUBTITLE:
    ret = put_dmx_stm_get_spau32(prEsmParam, prEsmParam32);
		if (ret != 0) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("%s line %d fail in put_dmx_stm_get_spau32, ret: %d.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, ret);
	  }
    break;
  case STREAM_SECTION:
    ret = put_dmx_stm_get_secau32(prEsmParam, prEsmParam32);
		if (ret != 0) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("%s line %d fail in put_dmx_stm_get_secau32, ret: %d.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, ret);
	  }
    break;
  default:
    pr_err("[DMX] %s line %d fail for inalid stream type(%d)\r\n",
      DMX_FUNC_NAME, DMX_LINE_NO, prStm->u4StmType);
    return -EINVAL;
  }

  if (0 != ret) {
    pr_err("[DMX] %s line %d fail for get compat au info, stmtype(%d), ret(%ld)\r\n",
      DMX_FUNC_NAME, DMX_LINE_NO, prStm->u4StmType, ret);
    return ret;
  }

	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("%s line %d -- prEsmParam->u4Status: %d.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		prEsmParam->u4Status);

  if (copy_in_user(&(prEsmParam32->u4Status), &(prEsmParam->u4Status),
    sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4Status).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}
  if (copy_in_user(&(prEsmParam32->u4TimeWait), &(prEsmParam->u4TimeWait),
    sizeof(__u32))) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d fail in copy_in_user(u4TimeWait).\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return -EFAULT;
	}

	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("%s line %d exit, prEsmParam32->u4Status: %d, prEsmParam32->u4Status: %d.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		prEsmParam32->u4Status,
		prEsmParam->u4Status);

	return 0;
}

long DMX_IOControl_Compat(struct file *file, unsigned int cmd, unsigned long arg)
{
	union {
		SPT_PARAM_ENABLE rSptEnable;
		SPT_PARAM_SET_RATE rSptSetRate;
		DMX_CREATE_STM_PARAM_T rCreateStmParam;
		STM_PARAM_SET_FIFO_SZ rStmSetFifoSize;
		STM_PARAM_SET_THRESHOLD rStmSetThreshold;
		DMX_STM_MANAGE_AU_T rManAU;
		STM_PARAM_DESTROY rStmDestroy;
		STM_PARAM_SET_UID rStmSetUID;
		DMX_PSR_FIFO_USAGE_T rPsrFifoUse;
		DMX_PSR_ON_PARAM_T rPsrOn;
		DMX_PSR_FILE_OFST_T rPsrFileOfst;
		CFA_PARAM_SET_TYPE rCfaSetType;
		CFA_PARAM_SET_CONFIG rCfaSetConfig;
		CFA_PARAM_SET_RANGE rCfaSetRange;
		CFA_PARAM_SET_INQ_TYPE rCfaSetInqType;
		CFA_PARAM_SET_INFO rCfaSetInfo;
		DMX_PBBUF_SEND_BUF_T rPbbufSendBuf;
		PBBUF_PARAM_SEND_BUF rSendBufParam;
		DMX_PBBUF_NODATA_PARAM_T rPbbufNoData;
		SPLITTER_PTX_RSP_ON_INFO_T rSptRspOn;
		SPLITTER_PTX_RSP_OFF_INFO_T rSptRspOff;
		SPLITTER_PTX_REBUFFER_RANGE_INFO_T rSptRebufRange;
		DECRYPT_OPER_PARAM_T rDecryptOper;
		DECRYPT_INST_PARAM_T rDecyptInst;
		void *rPtr;
		u32 rUint;
	} rArg;
	
	void __user *usr_ptr = compat_ptr(arg);
	bool fgCompatible = TRUE;
	long err = 0;
	long err2 = 0;
	compat_caddr_t usr_tmp;
	bool fgIsUserMem = FALSE;

	unsigned int u4CompCmd = dmx_cmd_switch(cmd);
	switch (u4CompCmd) {
		case DMX_IOCTL_SPT_DISABLE:
		case DMX_IOCTL_SPT_IS_ENABLED:
		case DMX_IOCTL_SPT_DESTROY:
		case DMX_IOCTL_PSR_OFF:
		case DMX_IOCTL_PSR_PAUSE:
		case DMX_IOCTL_PBBUF_CANCEL_BUF:
			err = get_user(usr_tmp, (compat_caddr_t __user *)usr_ptr);
			rArg.rPtr = (void __user *)compat_ptr(usr_tmp);
			rArg.rPtr = get_dmx_spt_inst_from32(rArg.rPtr);
			fgCompatible = FALSE;
	    break;			
		case DMX_IOCTL_STM_ENABLE:
		case DMX_IOCTL_STM_DISABLE:
		case DMX_IOCTL_STM_FLUSH_FIFO:
			err = get_user(usr_tmp, (compat_caddr_t __user *)usr_ptr);
			rArg.rPtr = (void __user *)compat_ptr(usr_tmp);
			rArg.rPtr = get_dmx_stm_inst_from32(rArg.rPtr);
			fgCompatible = FALSE;
	    break;			
		case DMX_IOCTL_IS_BUSY:
			fgCompatible = FALSE;
	    break;			
		case DMX_IOCTL_SPT_ENABLE:
			err = get_dmx_spt_enable32(&rArg.rSptEnable, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_SPT_SET_RATE:
			err = get_dmx_spt_set_rate32(&rArg.rSptSetRate, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_STM_CREATE:
			err = get_dmx_stm_create32(&rArg.rCreateStmParam, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_STM_SET_FIFO_SZ:
			err = get_dmx_stm_set_fifo_sz32(&rArg.rStmSetFifoSize, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_STM_SET_THRESHOLD:
			err = get_dmx_stm_set_threshold32(&rArg.rStmSetThreshold, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_STM_GETAU:
			err = get_dmx_stm_get_au32(&rArg.rManAU, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_STM_RELEASEAU:
		case DMX_IOCTL_STM_RELEASE_FF_AUDIOAU:
			err = get_dmx_stm_get_au32(&rArg.rManAU, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_STM_DESTROY:
			err = get_dmx_stm_destroy32(&rArg.rStmDestroy, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_STM_SETUID:
			err = get_dmx_stm_setuid32(&rArg.rStmSetUID, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_PSR_VFIFO_USAGE:
		case DMX_IOCTL_PSR_AFIFO_USAGE:
			err = get_dmx_psr_fifo_use32(&rArg.rPsrFifoUse, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_PSR_ON:
			err = get_dmx_psr_on32(&rArg.rPsrOn, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_PSR_FILE_OFST:
		case DMX_IOCTL_CFA_GET_POSI:
			err = get_dmx_psr_file_ofst32(&rArg.rPsrFileOfst, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_CFA_SET_TYPE:
			err = get_dmx_cfa_set_type32(&rArg.rCfaSetType, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_CFA_CONFIG:
			err = get_dmx_cfa_config32(&rArg.rCfaSetConfig, usr_ptr, &fgIsUserMem);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_CFA_SET_RANGE:
			err = get_dmx_cfa_set_range32(&rArg.rCfaSetRange, usr_ptr, &fgIsUserMem);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_CFA_SET_GEN:
			err = get_dmx_cfa_set_info32(&rArg.rCfaSetInfo, usr_ptr, &fgIsUserMem);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_PBBUF_SEND_BUF:
			err = get_dmx_pbbuf_send_buf32(&rArg.rPbbufSendBuf, usr_ptr, &fgIsUserMem);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_PBBUF_ALLOC_BUF:
		case DMX_IOCTL_PBBUF_RELEASE_BUF:
			err = get_dmx_send_pbbuf_param32(&rArg.rSendBufParam, usr_ptr, &fgIsUserMem);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_PBBUF_NODATA:
			err = get_dmx_pbbuf_no_data32(&rArg.rPbbufNoData, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_RSP_ON:
			err = get_dmx_spt_rsp_on32(&rArg.rSptRspOn, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_RSP_OFF:
			err = get_dmx_spt_rsp_off32(&rArg.rSptRspOff, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_REBUFFER_RANGE:
			err = get_dmx_spt_rebuf_range32(&rArg.rSptRebufRange, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_DECRYPT_EXEC_CMD:
			err = get_dmx_decrypt_oper32(&rArg.rDecryptOper, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_DECRYPT_RELEASE_INST:
		case DMX_IOCTL_DECRYPT_GET_LAST_ERROR:
			err = get_dmx_decrypt_inst32(&rArg.rDecyptInst, usr_ptr);
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_SPT_CREATE:
		case DMX_IOCTL_SPT_OPEN_GET_CNT:
		case DMX_IOCTL_SET_FLAG:
			fgCompatible = FALSE;
			break;
		case DMX_IOCTL_SPT_REBUF_RANGE:
		case DMX_IOCTL_SPT_SET_LASTMEM:
		case DMX_IOCTL_SPT_LOCK:
		case DMX_IOCTL_SPT_UNLOCK:
		case DMX_IOCTL_STM_TS_SET_TIMEOUT:
		case DMX_IOCTL_STM_CHECK_FIFO_USAGE:
		case DMX_IOCTL_PSR_RESUME:
		case DMX_IOCTL_PBBUF_ENABLE:
		case DMX_IOCTL_PBBUF_DISABLE:
		case DMX_IOCTL_PBBUF_CLEAN_BUF:
		case DMX_IOCTL_SET_CLI_CMD_INFO:
		case DMX_IOCTL_RESET:
			break;
		default:
			break;
	}

	if (0 != err) {
    pr_err("[DMX] %s line %d fail for get compat data, err(%ld)\r\n",
      DMX_FUNC_NAME, DMX_LINE_NO, err);
		return err;
  }

	if (fgCompatible) {
		err = dmx_native_ioctl(file, u4CompCmd, (unsigned long)usr_ptr, fgIsUserMem);
	} else {
		mm_segment_t old_fs = get_fs();

		set_fs(KERNEL_DS);
		err = dmx_native_ioctl(file, u4CompCmd, (unsigned long)&rArg, fgIsUserMem);
		set_fs(old_fs);
	}

	switch (u4CompCmd) {
		case DMX_IOCTL_IS_BUSY:
		case DMX_IOCTL_SPT_CREATE:
			usr_tmp = (compat_caddr_t)ptr_to_compat((void *)rArg.rPtr);
			err2 = put_user(usr_tmp, (compat_caddr_t __user *)usr_ptr);
			if (0 != err2) {
				pr_err("[DMX] %s line %d fail in put_user, err2(%ld)\r\n",
					DMX_FUNC_NAME, DMX_LINE_NO, err2);
				return err2;
			}
			break;
		case DMX_IOCTL_SPT_OPEN_GET_CNT:
			err2 = put_user(rArg.rUint, (u32 __user *)usr_ptr);
			if (0 != err2) {
				pr_err("[DMX] %s line %d fail in DMX_IOCTL_SPT_OPEN_GET_CNT, err2(%ld)\r\n",
					DMX_FUNC_NAME, DMX_LINE_NO, err2);
				return err2;
			}
			break;
		case DMX_IOCTL_STM_CREATE:
			err2 = put_dmx_stm_create32(&rArg.rCreateStmParam, usr_ptr);
			if (0 != err2) {
				pr_err("[DMX] %s line %d fail in put_dmx_stm_create32, err2(%ld)\r\n",
					DMX_FUNC_NAME, DMX_LINE_NO, err2);
				return err2;
			}
			break;
		case DMX_IOCTL_STM_GETAU:
			err2 = put_dmx_stm_get_au32(&rArg.rManAU, usr_ptr);
			if (0 != err2) {
				pr_err("[DMX] %s line %d fail in put_dmx_stm_get_au32, err2(%ld)\r\n",
					DMX_FUNC_NAME, DMX_LINE_NO, err2);
				return err2;
			}
			break;
		case DMX_IOCTL_PSR_VFIFO_USAGE:
		case DMX_IOCTL_PSR_AFIFO_USAGE:
			err2 = put_dmx_psr_fifo_use32(&rArg.rPsrFifoUse, usr_ptr);
			if (0 != err2) {
				pr_err("[DMX] %s line %d fail in put_dmx_psr_fifo_use32, err2(%ld)\r\n",
					DMX_FUNC_NAME, DMX_LINE_NO, err2);
				return err2;
			}
			break;
		case DMX_IOCTL_PSR_FILE_OFST:
		case DMX_IOCTL_CFA_GET_POSI:
			err2 = put_dmx_psr_file_ofst32(&rArg.rPsrFileOfst, usr_ptr);
			if (0 != err2) {
				pr_err("[DMX] %s line %d fail in put_dmx_psr_file_ofst32, err2(%ld)\r\n",
					DMX_FUNC_NAME, DMX_LINE_NO, err2);
				return err2;
			}
			break;
		case DMX_IOCTL_RSP_OFF:
			err2 = put_dmx_spt_rsp_off32(&rArg.rSptRspOff, usr_ptr);
			if (0 != err2) {
				pr_err("[DMX] %s line %d fail in put_dmx_spt_rsp_off32, err2(%ld)\r\n",
					DMX_FUNC_NAME, DMX_LINE_NO, err2);
				return err2;
			}
			break;
		case DMX_IOCTL_REBUFFER_RANGE:
			err2 = put_dmx_spt_rebuf_range32(&rArg.rSptRebufRange, usr_ptr);
			if (0 != err2) {
				pr_err("[DMX] %s line %d fail in put_dmx_spt_rebuf_range32, err2(%ld)\r\n",
					DMX_FUNC_NAME, DMX_LINE_NO, err2);
				return err2;
			}
			break;
		case DMX_IOCTL_PBBUF_ALLOC_BUF:
			err2 = put_dmx_send_buf_param32(&rArg.rSendBufParam, usr_ptr);
			if (0 != err2) {
				pr_err("[DMX] %s line %d fail in put_dmx_send_buf_param32, err2(%ld)\r\n",
					DMX_FUNC_NAME, DMX_LINE_NO, err2);
				return err2;
			}
			break;
		case DMX_IOCTL_DECRYPT_DEVICE_IS_ACTIVED:
		default:
			break;
		
	}
	
  if (0 != err) {
		switch (u4CompCmd) {
		case DMX_IOCTL_STM_GETAU:
		case DMX_IOCTL_PBBUF_ALLOC_BUF:
			break;
	  default:
	    pr_err("[DMX] %s line %d fail in proc ioctl '%c', dir=%d, #%d (0x%08x)\n",
	      DMX_FUNC_NAME, DMX_LINE_NO,
	      _IOC_TYPE(cmd), _IOC_DIR(cmd), _IOC_NR(cmd), cmd);
			return err;
		}
  }

	return err;
}

#endif    //CONFIG_COMPAT


/*****************************************************************************************/

#endif				/*#ifndef DMX_COMPAT_IOCTL32 */

