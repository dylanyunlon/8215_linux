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



#ifndef IOCTL_DMX_H
#define IOCTL_DMX_H

#include "x_typedef.h"
#include <linux/ioctl.h>
#include <linux/types.h>

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
#define DMX_IOCTL_SPT_CREATE				_IOR('D', 1, uintptr_t)

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
#define DMX_IOCTL_SPT_ENABLE				_IOW('D', 2, SPT_PARAM_ENABLE)

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
#define DMX_IOCTL_SPT_DISABLE				_IOW('D', 3, uintptr_t)

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
#define DMX_IOCTL_SPT_IS_ENABLED			_IOW('D', 4, uintptr_t)

/**
 *
 */
#define DMX_IOCTL_SPT_REBUF_RANGE			_IO('D', 5)

#define DMX_IOCTL_SPT_SET_RATE				_IOW('D', 6, SPT_PARAM_SET_RATE)

#define DMX_IOCTL_SPT_SET_LASTMEM			_IO('D', 7)

#define DMX_IOCTL_SPT_OPEN_GET_CNT			_IOR('D', 8, __u32)

#define DMX_IOCTL_SPT_LOCK					_IO('D', 9)

#define DMX_IOCTL_SPT_UNLOCK				_IO('D', 10)

#define DMX_IOCTL_SPT_DESTROY				_IOW('D', 11, uintptr_t)

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
#define DMX_IOCTL_STM_CREATE				_IOWR('D', 12, DMX_CREATE_STM_PARAM_T)

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
#define DMX_IOCTL_STM_ENABLE				_IOW('D', 13, uintptr_t)

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
#define DMX_IOCTL_STM_DISABLE				_IOW('D', 14, uintptr_t)

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
#define DMX_IOCTL_STM_SET_FIFO_SZ			_IOW('D', 15, STM_PARAM_SET_FIFO_SZ)

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
#define DMX_IOCTL_STM_SET_THRESHOLD			_IOW('D', 16, STM_PARAM_SET_THRESHOLD)

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
#define DMX_IOCTL_STM_FLUSH_FIFO			_IOW('D', 17, uintptr_t)

#define DMX_IOCTL_STM_GETAU					_IOWR('D', 18, DMX_STM_MANAGE_AU_T)

#define DMX_IOCTL_STM_RELEASEAU				_IOW('D', 19, DMX_STM_MANAGE_AU_T)

#define DMX_IOCTL_STM_DESTROY				_IOW('D', 20, STM_PARAM_DESTROY)

#define DMX_IOCTL_STM_SETUID				_IOW('D', 21, STM_PARAM_SET_UID)

#define DMX_IOCTL_STM_RELEASE_FF_AUDIOAU		_IOW('D', 22, DMX_STM_MANAGE_AU_T)

#define DMX_IOCTL_STM_TS_SET_TIMEOUT				_IO('D', 23)

#define DMX_IOCTL_STM_CHECK_FIFO_USAGE			_IO('D', 24)

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
#define DMX_IOCTL_PSR_VFIFO_USAGE			_IOWR('D', 25, DMX_PSR_FIFO_USAGE_T)

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
#define DMX_IOCTL_PSR_AFIFO_USAGE			_IOWR('D', 26, DMX_PSR_FIFO_USAGE_T)

/**
 *
 */
#define DMX_IOCTL_PSR_ON					_IOW('D', 27, DMX_PSR_ON_PARAM_T)

/**
 *
 */
#define DMX_IOCTL_PSR_OFF					_IOW('D', 28, uintptr_t)

/**
 *
 */
#define DMX_IOCTL_PSR_PAUSE					_IOW('D', 29, uintptr_t)

/**
 *
 */
#define DMX_IOCTL_PSR_RESUME				_IO('D', 30)

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
#define DMX_IOCTL_PSR_FILE_OFST				_IOWR('D', 31, DMX_PSR_FILE_OFST_T)


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
#define DMX_IOCTL_CFA_SET_TYPE				_IOW('D', 32, CFA_PARAM_SET_TYPE)

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
#define DMX_IOCTL_CFA_CONFIG				_IOW('D', 33, CFA_PARAM_SET_CONFIG)

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
#define DMX_IOCTL_CFA_SET_RANGE				_IOW('D', 34, CFA_PARAM_SET_RANGE)

/**
 *
 */
#define DMX_IOCTL_CFA_SET_GEN				_IOW('D', 35, CFA_PARAM_SET_INFO)


/**
 *
 */
#define DMX_IOCTL_CFA_GET_POSI				_IOWR('D', 36, DMX_PSR_FILE_OFST_T)

/*
=============================================================
==============================P B B U F=========================
=============================================================
*/
/**
 *
 */
#define DMX_IOCTL_PBBUF_ENABLE				_IO('D', 40)

/**
 *
 */
#define DMX_IOCTL_PBBUF_DISABLE				_IO('D', 41)

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
#define DMX_IOCTL_PBBUF_ALLOC_BUF			_IOWR('D', 42, PBBUF_PARAM_SEND_BUF)

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
#define DMX_IOCTL_PBBUF_CANCEL_BUF			_IOW('D', 43, uintptr_t)

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
#define DMX_IOCTL_PBBUF_SEND_BUF			_IOW('D', 44, DMX_PBBUF_SEND_BUF_T)

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
#define DMX_IOCTL_PBBUF_RELEASE_BUF			_IOW('D', 45, PBBUF_PARAM_SEND_BUF)

/**
 *
 */
#define DMX_IOCTL_PBBUF_CLEAN_BUF			_IO('D', 48)


#define DMX_IOCTL_PBBUF_NODATA				_IOW('D', 51, DMX_PBBUF_NODATA_PARAM_T)

/*
============================================================
==============================R E S P L I T T E R==================
===========================================================
 *
 * @brief turn on demuxer
 *
 * We use this to turn on demuxer.
 */
#define DMX_IOCTL_RSP_ON						_IOW('D', 52, SPLITTER_PTX_RSP_ON_INFO_T)

/*!
 * @brief turn off demuxer
 *
 * We use this to turn off demuxer.
 */

#define DMX_IOCTL_RSP_OFF						_IOWR('D', 53, SPLITTER_PTX_RSP_OFF_INFO_T)
/*!
 * @brief Get the information report by Parser for Playback Buffer file offset start.
 *
 * It is for data rebuffer
 * We use this to get current pbb start file offset (__u8).
 */
#define DMX_IOCTL_REBUFFER_RANGE						_IOWR('D', 54, SPLITTER_PTX_REBUFFER_RANGE_INFO_T)

/*
=============================================================
================================= CLI =========================
=============================================================
*/

#define DMX_IOCTL_SET_CLI_CMD_INFO					_IO('D', 55)

#define DMX_IOCTL_IS_BUSY                           _IOR('D', 56, uintptr_t)

#define DMX_IOCTL_RESET                             _IO('D', 57)

#define DMX_IOCTL_SET_FLAG                          _IOW('D', 58, __u32)

/*
=============================================================
================================= DECRYPT =====================
=============================================================
*/
/* Input:  eDecryptType, dwLenIn = sizeof(E_DECRYPT_TYPE_T) */
/* Output: &pvInst , dwLenOUt = sizeof(void **)*/
#define DMX_IOCTL_DECRYPT_CREATE_INST				_IOW('D', 60, E_DECRYPT_TYPE_T)

/* Input:  DECRYPT_OPER_PARAM_T*/
/* Output: according to the command*/
#define DMX_IOCTL_DECRYPT_EXEC_CMD					_IOW('D', 61, DECRYPT_OPER_PARAM_T)

/* Input:  rParam.eDecryptType, rParam.pvInst = pvInst, */
/* rParam's Type: DECRYPT_INST_PARAM_T*/
/* Output: NULL*/
#define DMX_IOCTL_DECRYPT_RELEASE_INST			_IOW('D', 62, DECRYPT_INST_PARAM_T)

/* Input:  eDecryptType, dwLenIn = sizeof(E_DECRYPT_TYPE_T)*/
/* Output: &fgActived, dwLenOut: sizeof(bool)*/
#define DMX_IOCTL_DECRYPT_DEVICE_IS_ACTIVED	 _IOWR('D', 63, DMX_CHECK_DECRYPT_DEVICE_T)

/* Input:  eDecryptType, dwLenIn = sizeof(E_DECRYPT_TYPE_T)*/
/* Output: according to decrypt type,*/
/*	 for DivxDRM, Output: &rActStatus,*/
/*	 dwLenOut: sizeof(DECRYPT_DIVXDRM_ACT_STATUS_T)*/
#define DMX_IOCTL_DECRYPT_DEVICE_GET_ACT_STATUS	_IOW('D', 64, E_DECRYPT_TYPE_T)

/* Input:  eDecryptType, dwLenIn = sizeof(E_DECRYPT_TYPE_T)*/
/* Output: according to decrypt type,*/
/*						for DivxDRM, Output: &rVersionInfo,*/
/*				dwLenOut: sizeof(DECRYPT_DIVXDRM_VERSION_INFO_T)*/
#define DMX_IOCTL_DECRYPT_DEVICE_GET_VERSION		_IOW('D', 65, E_DECRYPT_TYPE_T)

/* Input:  rParam.eDecryptType, rParam.pvInst = pvInst, */
/*				 rParam's Type: DECRYPT_INST_PARAM_T*/
/* Output: MRESULT, defined in dmx_define.h*/
#define DMX_IOCTL_DECRYPT_GET_LAST_ERROR			_IOW('D', 66, DECRYPT_INST_PARAM_T)

/* Input:  rParam.eDecryptType*/
#define DMX_IOCTL_DECRYPT_INIT_MEMORY				_IOW('D', 67, E_DECRYPT_TYPE_T)

/*
=============================================================
=============================== STRUCTURE =================
=============================================================
*/
typedef struct {
	void *pInBuf;
	int InSize ;
	void *pOutBuf;
	int OutSize ;
	unsigned int *pBytesReturned;
} DMX_IOCTL_DATA;


/*
=============================================================
=============================== EXPORT FUNCTION =================
=============================================================
*/
__u32 DMX_Init(const char * pContext, __u32 dwBusContext);

bool DMX_Deinit(void);


void * DMX_Open(void);

bool DMX_Close(void *pvInst);

bool DMX_IOControl(void *pvInst, unsigned int cmd, unsigned long arg, bool fgIsUserMem);



#endif				/*#ifndef IOCTL_DMX_H */
