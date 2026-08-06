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
 * @file dmx_spt_if.h
 *
 * @par Project
 *
 * @par Description
 *    Demuxer Stream, splitter main interfaces declarations
 *
 * @par Author_Name
 *    Shuhui Zhang
 *
 */

#ifndef DMX_INTERNAL_SPT_IF_H
#define DMX_INTERNAL_SPT_IF_H

#include "x_typedef.h"
#ifdef __linux__
#include <media/atc/x_dmx.h>
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/dmx_cfa_def.h>
/* #include <media/atc/mm_debug.h> */
#include <media/atc/drv_aud.h>

#else				/* __linux__ */

#include "x_dmx.h"
#include "dmx_define.h"
#include "dmx_splitter.h"
#include "dmx_cfa_def.h"
#include "drv_aud.h"
#include "mm_debug.h"
#include <pm.h>
#endif				/* __linux__ */

#include "dmx_def.h"

#ifdef __cplusplus
extern "C" {

#endif

/*! @name Stream Related Interface For Include Header File (1.2) */
/*! @{ */
MRESULT StreamCreate(void *pvDmxInst, STM_PARAM_CREATE *prParam, void **ppvHandle);
MRESULT StreamDestroy(void *pvSptHdl, void *pvStm);
MRESULT StreamEnable(void *pvStm);
MRESULT StreamDisable(void *pvStm);
MRESULT StreamSetFifoInfo(void *pvStm, u32 u4FifoSz);
MRESULT StreamSetFifoThreshold(void *pvStm, u32 u4FifoThreshold);
MRESULT StreamSetFlush(void *pvStm);

/*! @} */
MRESULT DmxInit(void);
MRESULT DmxUninit(void);

/*! @name Splitter Related Interface For Include Header File (1.0) */
/*! @{ */
MRESULT SplitterCreate(void *pvDmxInst, void **ppvHandle);
MRESULT SplitterDestroy(void *pvSptHdl);
MRESULT SplitterEnable(void *pvSptHdl, DMX_PBBUF_CONFIG_INFO_T *prPbbufCfgInfo);
MRESULT SplitterDisable(void *pvSptHdl);
MRESULT SplitterSetRate(void *pvSptHdl, s32 i4Rate, bool fgDmaAud);
MRESULT SplitterSetLastMem(void *pvSptHdl, bool fgLastMem);
MRESULT SplitterHandleCliCmd(DMX_CLI_CFG *prCliCfg);

/*! @} */

/*! @name CFA Related Interface For Include Header File (1.1) */
/*! @{ */
MRESULT SplitterSetCfaType(void *pvSptHdl, u32 u4CfaType);
MRESULT SplitterSetCfaConfigure(void *pvSptHdl, void *pvCfaParameter, bool fgIsUserMem);
MRESULT SplitterSetCfaRange(void *pvSptHdl, void *pvCfaRange, bool fgIsUserMem);
MRESULT SplitterSetCfaInquire(void *pvSptHdl, u32 u4InquirerTypes);

/**
 * The position is the video sample number of current parsing.
 *
 */
MRESULT SplitterGetCfaPosition(void *pvSptHdl, void *pvCfaPosition);
MRESULT SplitterGetCfaGeneral(void *pvSptHdl, u32 u4CfaFID,
void *pvCfaParameter, u32 u4CfaParameterSize);
MRESULT SplitterSetCfaGeneral(void *pvSptHdl, u32 u4CfaFID,
	void *pvCfaParameter, u32 u4CfaParameterSize);
MRESULT Splitter_CheckFifo(SPT_PARAM_FIFO_USAGE *prFifoUsage,
	SPT_PARAM_FIFO_USAGE_OUTPUT *prOutInfo);

/*! @} */

/*! @name Parser Related Interface For Include Header File (1.3) */
/*! @{ */
MRESULT SplitterGetStmFifoFullness(void *pvSptHdl,
	u32 u4SptDataType, u32 *pu4FullNess);
MRESULT SplitterSetParserOn(DMX_PSR_ON_PARAM_T *prParam);
MRESULT SplitterSetParserOff(void *pvSptHdl);
MRESULT SplitterSetParserPause(void *pvSptHdl);
MRESULT SplitterGetFileOfst(void *pvSptHdl, u64 *pu8FileOfst);

/*! @} */

/*! @name Resplitter Related Interface For Include Header File (1.3) */
/*! @{ */
MRESULT SplitterSetParserRspOn(void *pvSptHdl, bool fgRebuf);
MRESULT SplitterSetParserRspOff(void *pvSptHdl,
	u8 ucRspTxType, u8 ucRspMode, u8 *pu1RspTxRet, u8 ucState);
MRESULT SplitterGetRebufferRange(void *pvSptHdl, u64 u8RspDelta,
	u64 u8RspStartPts, u64 *pu8RspStartOffset,
	u64 *pu8PbbStartOffset, bool *fgRebuff);
MRESULT StreamSetUID(void *pvStm, u32 u4StreamUID);

MRESULT SplitterSetPowerState(DMX_PM_STATE PowerState);
DMX_PM_STATE SplitterGetPowerState(void);


/*! @} */

#ifdef __cplusplus
}
#endif

#endif				/* #ifndef DMX_INTERNAL_SPT_IF_H */
