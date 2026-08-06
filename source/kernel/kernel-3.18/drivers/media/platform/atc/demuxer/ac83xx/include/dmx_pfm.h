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
 * @file dmx_pfm.h
 *
 *
 * @par Project
 *
 * @par Description
 *
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 *
 */

#ifndef DMX_INTERNAL_PFM_H
#define DMX_INTERNAL_PFM_H

#include "x_typedef.h"
#include "drv_common.h"
#ifdef __linux__
#include <media/atc/dmx_decrypt.h>
#else	/*  */
#include "dmx_decrypt.h"
#endif	/* __linux__ */
#include "dmx_spt_cfa.h"
#include "dmx_def.h"

#ifdef __cplusplus
extern "C" {

#endif	/*  */

#if DMX_PFM_TEST
void DmxPfmInit(void);
void DmxPfmInstStart(void *pvSptHdl);
void DmxPfmInstStop(void *pvSptHdl);
void DmxPfmCfaStart(void *pvSptHdl);
void DmxPfmCfaEnd(void *pvSptHdl);
void DmxPfmSyncPbbufStart(void *pvSptHdl);
void DmxPfmStmSWDmaStart(void *pvSptHdl, E_SPT_DATA_TYPE_T eDataType);
void DmxPfmPtxDoneEnd(void *pvSptHdl);
void DmxPfmStmHwDmaStart(E_SPT_DATA_TYPE_T eDataType);
void DmxPfmStmHwDmaEnd(E_SPT_DATA_TYPE_T eDataType);
void DmxPfmStmComposeAUStart(E_SPT_DATA_TYPE_T eDataType);
void DmxPfmStmComposeAUEnd(E_SPT_DATA_TYPE_T eDataType);
void DmxPfmStmGetAUStart(E_SPT_DATA_TYPE_T eDataType);
void DmxPfmStmGetAUEnd(E_SPT_DATA_TYPE_T eDataType);
void DmxPfmPrintInfo(void *pvSptHdl);
void DmxPfmStmIncDecryptPb2Fifo(E_SPT_DATA_TYPE_T eDataType);
void DmxPfmStmIncDecryptSyncPbCnt(E_SPT_DATA_TYPE_T eDataType);
#endif	/* DMX_PFM_TEST */

#ifdef __cplusplus
}
#endif	/*  */

#endif	/* #ifndef DMX_INTERNAL_PFM_H */
