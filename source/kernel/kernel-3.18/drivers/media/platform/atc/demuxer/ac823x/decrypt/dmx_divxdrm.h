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
 * @file dmx_DivxDRM.h
 *
 * @par Project
 *
 *
 * @par Description
 *	  DivxDRM decryption structures, macros, interfaces declarations
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */

#ifndef DMX_DECRYPT_DIVXDRM_H
#define DMX_DECRYPT_DIVXDRM_H

#include "x_typedef.h"
#ifdef __linux__
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_decrypt.h>
#include <media/atc/mm_errcode.h>
#else
#include "dmx_define.h"
#include "dmx_decrypt.h"
#include "mm_errcode.h"
#endif /* __linux__*/

#include "dmx_def.h"
#include "dmx_cpsa.h"

#ifdef __cplusplus
extern "C" {
#endif

#if DMX_SUPPORT_DIVXDRM

typedef struct {
	u8  *pu1DrmHdrBuf;
	u32 u4DrmHdrBufSz;
} DECRYPT_DIVXDRM_HDR_T;

typedef enum {
	DECRYPT_DIVXDRM_STATUS_UNKNOWN,
	DECRYPT_DIVXDRM_STATUS_SYSTEM_UNINIT,
	DECRYPT_DIVXDRM_STATUS_SYSTEM_INITED,
	DECRYPT_DIVXDRM_STATUS_FILE_INITED,
	DECRYPT_DIVXDRM_STATUS_FILE_PLAYING,
} E_DECRYPT_DIVXDRM_STATUS_T;

typedef struct DecryptDivxDrmInst {
	E_DECRYPT_TYPE_T		   eType;
	s32					   i4DecryptId;
	E_DECRYPT_DIVXDRM_STATUS_T eStatus;
	DECRYPT_DIVXDRM_HDR_T	   rHeader;
	void					  *pvContext;
	u32					   u4ContextSz;
	u32					   u4Ref;
#ifndef __linux__
	CRITICAL_SECTION		   rCrit;
#else
	HANDLE					   hSema;
#endif /*__linux__*/
	struct DecryptDivxDrmInst *prNext;
} DECRYPT_DIVXDRM_INST_T;

EXTERN MRESULT DivxDRMInitialize(void);

EXTERN MRESULT DivxDRMDeInitialize(void);

EXTERN MRESULT DivxDRMGetPlayInst(s32 i4DecryptId, void **ppvInst);

EXTERN MRESULT DivxDRMRelPlayInst(void *pvInst);

EXTERN bool DivxDRMIsInstValid(DECRYPT_DIVXDRM_INST_T *prInst);

EXTERN MRESULT DivxDRMCreateInst(void **ppvInst);

EXTERN MRESULT DivxDRMReleaseInst(void *pvInst);

EXTERN MRESULT DivxDRMExecCmd(
					void	 *pvInst,
					u32	 u4Code,
					void	 *pvBufIn,
					u32	 u4LenIn,
					void	 *pvBufOut,
					u32	 u4LenOut);

EXTERN MRESULT DivxDRMGetInstLastError(void *pvInst);

EXTERN bool    DivxDRMIsDeviceActived(void);

EXTERN MRESULT DivxDRMGetActivationStatus(void *pvActStatus, u32 u4StatusSz);

EXTERN MRESULT DivxDRMGetVersion(void *pvVersionInfo, u32 u4InfoBufLen);

EXTERN MRESULT DivxDRMInitMemory(void);

#endif /* #if DMX_SUPPORT_DIVXDRM */

#ifdef __cplusplus
}
#endif


#endif /* #ifndef DMX_DECRYPT_DIVXDRM_H*/





