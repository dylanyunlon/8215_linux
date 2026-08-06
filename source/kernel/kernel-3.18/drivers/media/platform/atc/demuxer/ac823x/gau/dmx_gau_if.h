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
* @file dmx_gau_if.h
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

#ifndef _DMX_GAU_IF_H_
#define _DMX_GAU_IF_H_


#include "x_typedef.h"
#include "x_os.h"
#ifdef __linux__
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/drv_esm_if.h>
#else
#include "dmx_define.h"
#include "dmx_splitter.h"
#include "drv_esm_if.h"
#endif /* __linux__ */

#include "dmx_def.h"
#include "dmx_gau.h"

#ifdef __cplusplus
extern "C" {
#endif

void	GAU_Init(void);
void	GAU_Uninit(void);

MRESULT GAU_Connect(GAU_CONNECT_PARAM_T *prParam);
MRESULT GAU_Disconnect(u32 u4Handle);

ESM_IO_BUF_INFO *GAU_Get_GEsmIOBuf(u32 u4Handle);
ESM_IO_BUF_INFO *GAU_Get_REsmIOBuf(u32 u4Handle);

MRESULT GAU_GetAU(u32 u4Handle, void *pvIOBuf);
MRESULT GAU_ReleaseAU(u32 u4Handle, void *pvIOBuf, bool fgFF);

bool	GAU_GetEOSStatus(u32 u4Handle, bool *pfgEOS, u32 *pu4Status);
void	GAU_SetEOS(void *pvSptHdl, bool fgEOS, u32 u4Status);

MRESULT GAU_GetSectionAUEx(u32 u4Handle, void *pvIOBuf, u32 u4BuffSize);

MRESULT GAU_CheckFifo(u32 u4Handle, void *pvIOBuf);

MRESULT GAU_GetEvent(u32 u4Handle, EV_GRP_EVENT_T u4WaitOnEvent,
					EV_GRP_EVENT_T *pu8ReceiveEvent, u32 u4TimeOut);
MRESULT GAU_SetEvent(u32 u4Handle, EV_GRP_EVENT_T u4SetEvent);
MRESULT GAU_ResetEvent(u32 u4Handle, EV_GRP_EVENT_T u4WaitOnEvent);

MRESULT GAU_Enable(u32 u4Handle, bool fgEnable);

MRESULT GAU_SetThreshold(u32 u4Handle, u32 u4Threshold);

void	GAU_DisableThreshold(void);

void	GAU_CheckThreshold(u32 u4Handle);

bool	GAU_IsReachThreshold(void);

MRESULT GAU_SetSkipThreshold(u32 u4Handle);

void	GAU_ClearThreshold(u32 u4Handle);

MRESULT GAU_DumpInfo(void);

void	GAU_PrintLogAUInfo(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _DMX_GAU_IF_H_ */
