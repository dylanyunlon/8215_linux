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
 * @file dmx_psr_esm.h
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

#ifndef DMX_PSR_ESM_H
#define DMX_PSR_ESM_H

#include "x_typedef.h"
#include "drv_common.h"
#ifdef __linux__
#include <media/atc/dmx_define.h>
#include <media/atc/drv_esm_if.h>
#else
#include "dmx_define.h"
#include "drv_esm_if.h"
#endif /* __linux__*/

#include "dmx_psr_filter.h"
#include "dmx_psr_util.h"

#ifdef __cplusplus
extern "C" {
#endif

void PSR_Filter_ESICB(ES_CBEVENT eEvent, void *pvData, void *pvPrivate);

MRESULT PSR_Filter_IsAUTableFull(PSR_FILTER *pPsrFtr, bool *pfgFull);

MRESULT PSR_AFilter_UpdateESIInfo(PSR_FILTER *pPsrFtr);

MRESULT PSR_SectionFilter_UpdateESIInfo(PSR_FILTER *pPsrFtr);

MRESULT PSR_DMA_UpdateESIInfo(PSR_FILTER *pPsrFtr);

MRESULT PSR_SPFilter_UpdateESIInfo(PSR_FILTER *pPsrFtr);

MRESULT PSR_VFilter_UpdateESIInfo(PSR_FILTER *pPsrFtr);

MRESULT PSR_Filter_IsESBufFull(PSR_FILTER *pPsrFtr, u32 u4Len, bool *pfgFull);

MRESULT PSR_Filter_GetFifoAvailSize(PSR_FILTER *pPsrFtr, u32 *pu4Size);

MRESULT	PSR_Filter_GetFifoFreeSpace(PSR_FILTER *pPsrFtr, u32 *pu4AvailSize);

MRESULT PSR_Filter_SetESBufSize(PSR_FILTER *pPsrFtr, u32 u4Size);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef DMX_PSR_ESM_H*/

