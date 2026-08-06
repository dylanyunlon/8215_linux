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

#ifndef _VDEC_DRV_IF_PUBLIC_H_
#define _VDEC_DRV_IF_PUBLIC_H_

#include "ioctl_vdec.h"
#include "mm_errcode.h"
#include "drv_vdec.h"
#include "vdec_init.h"

typedef enum _VPARMTYPE
{
    VDEC_PARAM_NONE_T = 0,
    VDEC_PARAM_SET_PROG_TYPE = 1,
    VDEC_PARAM_GET_ASPECT_RATIO = 0x50,
    VDEC_PARAM_MAX_T = 0xFF,
}VParamType;

#ifdef __cplusplus
extern "C" {
#endif
bool fgVDecHWIsBusy(void *hVDecInst);
void vVDecResetVdecHW(void *hVDecInst);
extern void *mrVDecDrvCreate(VDEC_CODEC_T eVideoCodec);
extern MRESULT mrVDecDrvRelease(void *hVDecInst);
extern MRESULT mrVDecDrvGetParam(void *hVDecInst, VParamType dwCmd, void *prParam);
extern MRESULT mrVDecDrvSetParam(void *hVDecInst, VParamType dwCmd, void *prParam);
extern MRESULT mrVDecDrvGetDisplayBuf(void *hVDecInst, void* lpInBuffer);
extern MRESULT mrVDecDrvClearDisplayBuf(void *hVDecInst, void* lpInBuffer);
extern MRESULT mrVDecDrvDecode(void *hVDecInst, __u32 dwIoControlCode,
                              void* lpInBuffer, void* lpOutBuffer);
extern MRESULT mrVDecDrvFlush(void *hVDecInst, void *lpInBuffer);

extern void vHalCmdProc(VDEC_HANDLE_T *phHandle, __u32 cmd, void *pvInParam, void *pvOutParam);
extern MRESULT mrVideoMemAlloc(
    VDEC_HANDLE_T *phHandle,
    VAL_MEMORY_T *prParam,
    __u32 u4ParamSize
);

MRESULT mrVideoMemFree(
    VDEC_HANDLE_T *phHandle,
    VAL_MEMORY_T *prParam,
    __u32 u4ParamSize
);
extern void* VDEC_ReservedMemory_PAToVA(void *hVDecInst, void* pvPhyAddr);


#ifdef __cplusplus
}
#endif

#endif

