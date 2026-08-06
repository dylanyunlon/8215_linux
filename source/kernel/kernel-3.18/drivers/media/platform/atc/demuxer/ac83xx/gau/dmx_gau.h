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
* @file dmx_gau.h
*
* @par Project
*
*
* @par Description
*
*
* @par Author_Name
*	  Shuhui Zhang
*
*/

#ifndef _DMX_GAU_H_
#define _DMX_GAU_H_

#ifdef __linux__
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#else
#include "dmx_define.h"
#include "dmx_splitter.h"
#endif /* __linux__ */

#include "dmx_def.h"
#include "dmx_esm.h"
#include "dmx_gau_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	GAU_EV_DISABLE_GETAU   = 1 << 0,
	GAU_EV_SKIP_THRESHOLD  = 1 << 1,
	GAU_EV_REACH_THRESHOLD = 1 << 2,
	GAU_EV_AU_IN		   = 1 << 3,
	GAU_EV_FIFO_FLUSH	   = 1 << 4,
	GAU_EV_AUCMD_RELEASE   = 1 << 5,
} GAU_EVENT_T;

typedef struct {
	void	*pvSptHdl;
	u32	u4FilterType;
	u32	u4FilterId;
} GAU_CONNECT_INFO;

typedef struct {
	uintptr_t	ptrFifoPSa;
	uintptr_t	ptrFifoPEa;
	uintptr_t	ptrFifoVSa;
	uintptr_t	ptrUserVirSA;
	HANDLE	hCallProcess;
	u32	u4FifoSz;
	uintptr_t ptrMMRsvBufBase;
} GAU_SELF_USE_FIFO_T;

typedef struct {
	bool	 fgUsed;
	bool	 fgEOS;
	bool	 fgReachThreshold;
	bool	 fgGetAUEnable; /* only section gau need to check this param. */
	u32	 u4Handle;
	u32	 u4ESHandle;
	u32	 u4AUConsumedCnt;
	u32	 u4Status;
	u32	 u4StmType;
	u32	 u4StmCodec;
	u32	 u4Threshold;
	HANDLE_T hAUInEG;

	ESM_IO_BUF_INFO *prRelEsmIOBuf;
	ESM_IO_BUF_INFO *prGetEsmIOBuf;
	void *pvSptHdl;
	void *pvDmxInst;
	uintptr_t  ptrMMRsvBufBase;
	u64	 u8DecSendBufMask;
	GAU_SELF_USE_FIFO_T  rSelfFifoInfo;
	GAU_Q_T  rGetAUQueue;
} GAU_INSTANCE_T;

typedef struct {
	bool	fgReachThreshold;
	GAU_INSTANCE_T arGAUInstance[MAX_GAU_INSTANCE_CNT];
} GAU_MANAMENT_INFO_T;

typedef struct {
	u32 u4StmType;
	u32 u4StmUID;
	u32 u4QueueElemCnt;
	void *pvSptHdl;
	u32 *pu4Handle;
	u64 u8DecSendBufMask;
} GAU_CONNECT_PARAM_T;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _DMX_GAU_H_ */

