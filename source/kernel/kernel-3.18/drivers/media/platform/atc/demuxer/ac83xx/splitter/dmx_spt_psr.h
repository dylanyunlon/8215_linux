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
*/

/*!
 * @file dmx_spt_psr.h
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
 */

#ifndef DMX_SPT_PSR_H
#define DMX_SPT_PSR_H

#include "x_typedef.h"
#include "drv_common.h"
#ifdef __linux__
#if CONFIG_DRV_HDMI_RX
#include <media/atc/x_audin.h>
#endif /*CONFIG_DRV_HDMI_RX*/
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#else
#if CONFIG_DRV_HDMI_RX
#include "x_audin.h"
#endif /*CONFIG_DRV_HDMI_RX*/
#include "dmx_define.h"
#include "dmx_splitter.h"
#endif /*__linux__*/

#include "dmx_spt.h"

#ifdef __cplusplus
extern "C" {
#endif

MRESULT SplitterPbb2Buf(void *pvSptHdl, DMAInfo *prDMA);

MRESULT SplitterBuf2Fifo(void *pvSptHdl, DMX_SPT_DMA2FIFO_INFO_T *prInf);

MRESULT SplitterBuf2FifoAUCtrl(void *pvSptHdl, DMX_SPT_DMA2FIFO_INFO_T *prInf);

MRESULT SplitterPbb2Fifo(void *pvSptHdl, DMX_SPT_DMA2FIFO_INFO_T *prInf);

MRESULT SplitterSetPsrAuTable(void *pvSptHdl, void *pvEventData);

MRESULT SplitterTxEndCmdAU(void *pvSptHdl, E_SPT_DATA_TYPE_T eDataType);

MRESULT SplitterPsrSetEOS(void *pvSptHdl, u32 u4Status);

#if DMX_SUPPORT_DIVXDRM
MRESULT SplitterSetPtxDivxDRMInf(void *pvSptHdl, u64 u8DecryptStOfst,
	u32 u4DecryptLen, u16 u2FrameKeyIdx);

MRESULT SplitterPsrTurnDivxDRM(void *pvSptHdl, CFA_DIVXDRM_INFO_T *prDivxDRMInf);
#endif /*DMX_SUPPORT_DIVXDRM*/

#if CONFIG_DRV_HDMI_RX
MRESULT SplitterPsrAudInIsRaw(void *pvSptHdl, bool *pfgIsRawAud);

MRESULT SplitterPsrGetAudInParsingInfo(void *pvSptHdl, AUDIN_PARSING_INFO_T *prPsrInfo);
#endif /*CONFIG_DRV_HDMI_RX*/

MRESULT SplitterCreateCmdAU(void *pvStmHdl, E_SPT_DATA_TYPE_T eDataType);

#ifdef __cplusplus
}
#endif

#endif /*#ifndef DMX_SPT_PSR_H*/


