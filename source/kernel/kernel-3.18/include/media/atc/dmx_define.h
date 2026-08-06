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

#ifndef _DMX_DEFINE_H_
#define _DMX_DEFINE_H_

#include "mm_errcode.h"
#include "chip_ver.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MM_SUPPORT_DIVXHT31
/* 1: Support Fast Forward and Fast Rewind Mechanism */
#define DMX_SUPPORT_FFRW									1
/* 1: Support 3 splitter instances, 0: unsupport(default) */
#define DMX_SUPPORT_3RD_INSTS							1
/* 1: Support DivxDRM Decryption, 0: unsupport */
#define DMX_SUPPORT_DIVXDRM								1
#else				/* MM_SUPPORT_DIVXHT31 */
/* 1: Support Fast Forward and Fast Rewind Mechanism */
#define DMX_SUPPORT_FFRW									1
/* 1: Support 3 splitter instances, 0: unsupport(default) */
#define DMX_SUPPORT_3RD_INSTS							1
/* 1: Support DivxDRM Decryption, 0: unsupport */
#define DMX_SUPPORT_DIVXDRM								0
#endif				/* MM_SUPPORT_DIVXHT31 */

#define DMX_GAU_GETAU_WAIT_THRESHOLD_EVT	0

/* unit : second */
#define AVI_AVSYNC_THRESHOLD							3

#ifndef HANDLE
#ifndef __linux__
	typedef void * HANDLE;
#endif				/* __linux__ */
#endif


/* ////////////////////////////////////////////////////////////////////////////// */
/* GAU(GetAU) return State/errcode                                               // */
/* ////////////////////////////////////////////////////////////////////////////// */
#define GAU_S_OK							0
#define GAU_E_FIFOFLUSH				1
#define GAU_E_EOS							((__u32)2)
#define GAU_E_FAIL						((__u32)3)
#define GAU_E_EJECTCARD				4
#define GAU_E_INVALIDARG			5
#define GAU_E_TIMEOUT					6
#define GAU_E_MEMORY					7
#define GAU_E_THRESHOLD				8
#define GAU_E_ERRCHUNK				((__u32)9)
#define GAU_E_ERRDATA					10
#define GAU_S_DISCONTINUOUS		11
#define GAU_E_FFRWABNORMALEND	12

/* ////////////////////////////////////////////////////////////////////////////// */
/* Demuxer Error Code Definition                                                  // */
/* ////////////////////////////////////////////////////////////////////////////// */
#define EN_PUB_ID					(MRESULT)0x00000000
#define IN_PUB_ID					(MRESULT)0x00000001

/* public error code for external useing */
#define EP_CODE(x)				(MRESULT)((EN_PUB_ID << 12) | (MRESULT)(x))
/* public error code for total demux module inner useing */
#define IP_CODE(x)				(MRESULT)((IN_PUB_ID << 12) | (MRESULT)(x))

/* Demuxer External Real ErrCode */
#define MAKE_DMX_EXT_REAL_ERRCODE(x)	 MAKE_ERR_CODE(MOD_ERRCODE_DMX, EP_CODE(x))

/* Demuxer External State ErrCode */
#define MAKE_DMX_EXT_STATE_ERRCODE(x)  MAKE_STATE_CODE(MOD_ERRCODE_DMX, EP_CODE(x))

/* Demuxer Internal Real ErrCode */
#define MAKE_DMX_INT_REAL_ERRCODE(x)	 MAKE_ERR_CODE(MOD_ERRCODE_DMX, IP_CODE(x))

/* Demuxer Internal State ErrCode */
#define MAKE_DMX_INT_STATE_ERRCODE(x)  MAKE_STATE_CODE(MOD_ERRCODE_DMX, IP_CODE(x))

/* ////////////////////////////////////////////////////////////////////////////// */
/* External Error Code of DMX, Middleware should check these errcode               // */
/* ////////////////////////////////////////////////////////////////////////////// */

#define RET_DMX_EXT_OK								MAKE_DMX_EXT_REAL_ERRCODE(0x00000000)

/* No System Resource for demuxer, such as, failed in create or open event, */
/* semaphore,... */
#define RET_DMX_EXT_PARAM_WRONG				MAKE_DMX_EXT_REAL_ERRCODE(0x00000001)

/* No enough memory to alloc for demuxer */
#define RET_DMX_EXT_NO_MEM						MAKE_DMX_EXT_REAL_ERRCODE(0x00000002)

/* Excceed Demuxer Limitation, such MW need to create splitter instance, */
/* but no unused */
#define RET_DMX_EXT_OVER_LIMIT				MAKE_DMX_EXT_REAL_ERRCODE(0x00000003)

/* Forbidden Operation, which demuxer not support */
#define RET_DMX_EXT_OPER_FORBID				MAKE_DMX_EXT_REAL_ERRCODE(0x00000004)

/* Sub Module(such as EMS, DMYDEC, PSR, ..) not init, may be caused by the */
/* calling sequence */
#define RET_DMX_EXT_NO_INIT						MAKE_DMX_EXT_REAL_ERRCODE(0x00000005)

/* Demuxer 's some instance's state is error, such as splitte not enable, */
/* splitter's state is error, and so on */
#define RET_DMX_EXT_INV_STATE					MAKE_DMX_EXT_REAL_ERRCODE(0x00000006)

/* Demuxer 's other errocde */
#define RET_DMX_EXT_EXCEPTION					MAKE_DMX_EXT_REAL_ERRCODE(0x00000007)

/* DECRYPT Related errorcode */

/* Decrypt Not Authorized */
#define RET_DMX_EXT_DECRYPT_GENERAL_ERROR			 MAKE_DMX_EXT_REAL_ERRCODE(0x00000014)

#define RET_DMX_EXT_DECRYPT_NEVER_REGISTERED	 MAKE_DMX_EXT_STATE_ERRCODE(0x00000014)

#define RET_DMX_EXT_DECRYPT_NOT_AUTHORIZED		 MAKE_DMX_EXT_STATE_ERRCODE(0x00000015)

#define RET_DMX_EXT_DECRYPT_NOT_REGISTERED		 MAKE_DMX_EXT_STATE_ERRCODE(0x00000016)

#define RET_DMX_EXT_DECRYPT_RENTAL_EXPIRED		 MAKE_DMX_EXT_STATE_ERRCODE(0x00000017)


#ifdef __cplusplus
}
#endif
#endif				/* #ifndef _DMX_DEFINE_H_ */
