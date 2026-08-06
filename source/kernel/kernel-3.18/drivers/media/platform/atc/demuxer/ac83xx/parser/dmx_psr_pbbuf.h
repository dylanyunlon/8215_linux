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
 * @file dmx_psr_pbbuf.h
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

#ifndef DMX_PSR_PBBUF_H
#define DMX_PSR_PBBUF_H

#include "x_typedef.h"
#ifdef __linux__
#if CONFIG_DRV_HDMI_RX
#include <media/atc/x_audin.h>
#endif /* CONFIG_DRV_HDMI_RX*/
#else /* __linux__*/
#if CONFIG_DRV_HDMI_RX
#include "x_audin.h"
#endif /* CONFIG_DRV_HDMI_RX*/
#endif /* __linux__*/
#include "dmx_def.h"
#include "dmx_psr_cc.h"
#include "dmx_pbbuf_if.h"

#ifdef __cplusplus
extern "C" {
#endif

MRESULT PSR_CC_GetPBBufSlot(PSR_CC *prPsrCC, u32 *pu4Idx);

bool	PSR_CC_IsOffsetInPbbuf(PSR_CC *prPsrCC, u32 u4PbbufIdx,	u64 u8Offset);

void	PSR_CC_ReleaseAllPBBuf(PSR_CC *prPsrCC);

MRESULT PSR_CC_ReleasePBBuf(PSR_CC *prPsrCC, u32 u4PbbufIdx);

void	PSR_CC_ClearAllPBBufInfo(PSR_CC *prPsrCC);

MRESULT PSR_CC_GetWaitTxBufSa(PSR_CC *prPsrCC, uintptr_t *pptrSa);

void	PSR_CC_PBBuf_Notify(void *pvTag, DRV_PBBUF_NOTIFY_COND_T eReadyCond,
						u32 u4Data);

void	PSR_CC_GetPBBufStatus(PSR_CC *prPsrCC, PSR_PBBUFInfo *prPBBufInfo);

MRESULT PSR_CC_GetCurPbbufStartOffset(PSR_CC *prPsrCC, u64 *pu8Offset);

bool	PSR_CC_IsPbbufUnCon(PSR_CC *prPsrCC, u32 u4PbbufIdx);

#if DMX_SUPPORT_DIVXDRM
MRESULT PSR_CC_GetWaitTxBufInfo(PSR_CC *prPsrCC, u64 u8Offset,
				uintptr_t *pptrSa, u64 *pu8Sz);
#endif /* #if DMX_SUPPORT_DIVXDRM*/

#if CONFIG_DRV_HDMI_RX
MRESULT PSR_CC_IsAudinRaw(void *pvPsrCC, bool *pfgIsAudRaw);

MRESULT PSR_CC_GetAudInParsingInfo(void *pvPsrCC, AUDIN_PARSING_INFO_T *prPsrInfo);
#endif /* CONFIG_DRV_HDMI_RX*/

#ifdef __cplusplus
}
#endif

#endif /* #ifndef DMX_PSR_PBBUF_H*/

