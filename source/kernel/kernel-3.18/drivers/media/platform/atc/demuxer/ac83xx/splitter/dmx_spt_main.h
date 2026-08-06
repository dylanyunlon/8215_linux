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
 * @file dmx_spt_main.h
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

#ifndef DMX_SPT_MAIN_H
#define DMX_SPT_MAIN_H

#include "x_typedef.h"
#include "dmx_spt.h"

#ifdef __cplusplus
extern "C" {
#endif

MRESULT SptInit(void);

MRESULT SptUninit(void);

MRESULT SptCreateInst(void *pvDmxInst, void **ppvHandle);

void	*SptHandleFromCompID(u32 u4SptCompId);

MRESULT SplitterSetEnable(void *pvSptHdl);

MRESULT SplitterSetDisable(void *pvSptHdl);

MRESULT SplitterSetAudioMaxDuration(void *pvSptHdl, u8 ucAudSec);

MRESULT SplitterCreatePsr(void *pvDmxInst, void *pvSptHdl);

MRESULT SplitterDestroyPsr(void *pvSptHdl);

MRESULT SplitterSetPsrLastMem(void *pvSptHdl, bool fgLastMem);

void **SplitterGetStmHandles(void *pvSptHdl, u32 *pu4StmHandleNs);

MRESULT SplitterAddStmHandle(void *pvSptHdl, void *pvStm);

MRESULT SplitterDelStmHandle(void *pvSptHdl, void *pvStm);

MRESULT SplitterSetPtxData(void *pvSptHdl, DMX_SPT_DMA2FIFO_INFO_T *prInf,
void *pvStm, bool fgNeedRspLog);

u64	SplitterGetPureAudioSTC(void *pvSptHdl);

bool	SplitterIstPureAudioPIPE(void *pvSptHdl);

DMX_SPT_INST_T *SplitterGet2pipeVSptInst(void);

MRESULT SplitterSetPBBOffsetSa(void *pvSptHdl, u64 u8PBBOffsetSa);

MRESULT SplitterSetFileEndOffset(void *pvSptHdl, u64 u8FileEndOffset);

MRESULT SplitterSetCfaPsrEnd(void *pvSptHdl, bool fgCfaPrsEnd, u32 u4Status);

E_DECRYPT_TYPE_T SplitterGetDecryptType(void *pvSptHdl);

MRESULT SplitterSetDecryptType(void *pvSptHdl, E_DECRYPT_TYPE_T eDecryptType);

void	SplitterSetEOSForError(void *pvSptHdl, MRESULT mrRet);

#ifdef __cplusplus
}
#endif

#endif /*#ifndef DMX_SPT_MAIN_H*/



