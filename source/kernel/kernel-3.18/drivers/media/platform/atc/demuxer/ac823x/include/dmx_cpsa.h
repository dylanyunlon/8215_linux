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
 * @file dmx_cpas.h
 *
 * @par Project
 *
 * @par Description
 *    Demuxer internal-used decryption structures, macros, interfaces declarations
 *
 * @par Author_Name
 *    Shuhui Zhang
 *
 */

#ifndef DMX_INTERNAL_CPSA_H
#define DMX_INTERNAL_CPSA_H

#include "x_typedef.h"
#ifdef __linux__
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/dmx_cfa_def.h>
#include <media/atc/drv_esm_if.h>
#include <media/atc/dmx_decrypt.h>
#include <media/atc/x_dmx.h>
#include <media/atc/mm_errcode.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "dmx_splitter.h"
#include "dmx_cfa_def.h"
#include "drv_esm_if.h"
#include "dmx_decrypt.h"
#include "x_dmx.h"
#include "mm_errcode.h"
#include "mm_debug.h"
#endif	/* __linux__ */

#include "dmx_def.h"

#ifdef __cplusplus
extern "C" {

#endif

/* DivxDRM Drm invalid frameindex */
#define DMX_DIVXDRM_INVALID_FRAMEIDX  (DMX_INVALID_UINT16)
/* DivxDRM Drm Info Size */
#define DMX_DIVXDRM_DRMINFO_SZ        (10)
/* DivxDRM Drm Memory Size */
#define DMX_DIVXDRM_MEMORY_LEN        (49)

EXTERN MRESULT DmxInitDecrypt(void);

EXTERN MRESULT DmxDeInitDecrypt(void);

EXTERN MRESULT DmxDecryptIsDevActived(
		E_DECRYPT_TYPE_T	eDecryptType,
		bool			*pfgActived);

EXTERN MRESULT DmxDecryptGetActStatus(
		E_DECRYPT_TYPE_T	eDecryptType,
		void			*pvStatus,
		u32			u4StatusSz);

EXTERN MRESULT DmxDecryptGetVersion(
		E_DECRYPT_TYPE_T	eDecryptType,
		void			*pvVersionInfo,
		u32			u4VersionInfo);

EXTERN MRESULT DmxDecryptInitMemory(E_DECRYPT_TYPE_T eDecryptType);

EXTERN MRESULT DmxDecryptCreateInst(
		E_DECRYPT_TYPE_T	eDecryptType,
		void			**ppvInst);

EXTERN MRESULT DmxDecryptReleaseInst(
		E_DECRYPT_TYPE_T	eDecryptType,
		void			*pvInst);

EXTERN MRESULT DmxDecryptGetPlayInst(
		E_DECRYPT_TYPE_T	eDecryptType,
		s32			i4DecryptId,
		void			**ppvInst);

EXTERN MRESULT DmxDecryptRelPlayInst(
		E_DECRYPT_TYPE_T	eDecryptType,
		void			*pvInst);

EXTERN MRESULT DmxDecryptExecCmd(
		void			*pvInst,
		u32			u4OperCode,
		void			*pvBufIn,
		u32			u4LenIn,
		void			*pvBufOut,
		u32			u4LenOut);

EXTERN MRESULT DmxDecryptGetInstLastError(
		E_DECRYPT_TYPE_T	eDecryptType,
		void			*pvInst);


/*///////////////////////////////////////////////////////////////////////////*/
/*///                              OTHER                                  ///*/
/*///////////////////////////////////////////////////////////////////////////*/

#ifdef __cplusplus
}
#endif

#endif				/* #ifndef DMX_INTERNAL_CPSA_H */
