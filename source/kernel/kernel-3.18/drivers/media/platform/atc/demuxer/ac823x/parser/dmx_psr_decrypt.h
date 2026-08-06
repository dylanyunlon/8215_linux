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
 * @file dmx_psr_decrypt.h
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

#ifndef DMX_PSR_DECRYPT_H
#define DMX_PSR_DECRYPT_H

#include "x_typedef.h"
#include "drv_ibc.h"
#include "drv_common.h"
#ifdef __linux__
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_decrypt.h>
#else
#include "dmx_define.h"
#include "dmx_decrypt.h"
#endif /* __linux__*/
#include "dmx_def.h"
#include "dmx_pbbuf_if.h"
#include "dmx_psr_filter.h"
#include "dmx_psr_cc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DMX_DECRYPT_DIVXDRM_DECRYPTION_ALIGNSIZE	32

typedef enum {
	PSR_DEC_COMPOSE_1ST_PART,
	PSR_DEC_COMPOSE_MID_PART,
	PSR_DEC_COMPOSE_END_PART,
	PSR_DEC_DECRYPT,
	PSR_DEC_START_DMA,
	PSR_DEC_DMA_DATA,
	PSR_DEC_END_DMA
} E_PSR_DEC_OPER_TYPE_T;

MRESULT PSR_Decrypt_Create(PSR_CC *prPsrCC, E_DECRYPT_TYPE_T eDecryptType);

MRESULT PSR_Decrypt_Release(PSR_CC *prPsrCC);

MRESULT PSR_Decrypt_Reset(PSR_CC *prPsrCC);

#if DMX_DRM_DECRYPT_USE_HW
MRESULT PSR_Decrypt_GetKeyInfo(
	PSR_CC *prPsrCC, E_SPT_DATA_TYPE_T eDataType);
#endif /* DMX_DRM_DECRYPT_USE_HW*/

MRESULT PSR_Decrypt_DecryptData(
	PSR_CC *prPsrCC,
	E_SPT_DATA_TYPE_T eDataType,
	u8  *pu1FrameData,
	u32 u4FrameDataSz);

MRESULT PSR_Decrypt_PbbufCheck(
	PSR_CC *prPsrCC,
	u32	u4PbbufIdx,
	bool	*fgDecryptEndOftIn,
	E_PBBUF_CONTINUITY_TYPE_T *pePbbufCon,
	bool fgForceDecrypt);

MRESULT PSR_Decrypt_DecryptCheck(
	PSR_CC *prPsrCC,
	PSR_FILTER *prPsrFtr,
	u64 u8Offset,
	E_PBBUF_CONTINUITY_TYPE_T *pePbbufCon);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef DMX_PSR_DECRYPT_H*/


