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



#ifndef VIRTUAL_MICIN_H
#define VIRTUAL_MICIN_H

#include "bt_cfg.h"
#include "aud_micin_hal_if.h"
#include "audiosys.h"
#include "aud_if_hw_asrc.h"

typedef enum {
	VMT_NONE = 0U,
	VMT_NORMAL,
	VMT_NDC,
	VMT_AEC
} VIRTUAL_MIC_TYPE;


typedef struct _Virtual_MicIn_{
    s32 (*Start)(void * pThis);
    s32 (*ResetRP)(void * pThis);
    s32 (*Stop)(void * pThis);
	s32 (*GetBuffer)(void * pThis, WAVE_DATA_BUF_T *prBuffer);
	s32 (*Setup)(void * pThis, u32 u4Fs);
	void (*UpdateRP)(void * pThis, u32 u4RP);
	u32  (*GetFS)(void * pThis);
	u32  (*GetWP)(void * pThis);
	u32 u4State;
	u32 u4Fs;
	u32 u4MicFs;
	WAVE_DATA_BUF_T rMicBuf;
	u32 u4AsrcIdx;
	u32 u4RP;
	u32 u4MicRP;
	u32 u4AsrcInFs;
	bool   fgFirstFillASRC;

	ASRC_CHS_FMT_T rAsrcFmt;
	VIRTUAL_MIC_TYPE eType;
	
} VirtualMicIn, *PVirtualMicIn;

s32 CreateVirtualMicIn(VIRTUAL_MIC_TYPE type, PVirtualMicIn *ppVMicIn);
s32 DeleteVirtualMicIn( PVirtualMicIn pVMicIn);
void NdcVirtualMicIn_CopyData(void *pvData, u32 u4DataSize);
void NdcVirtualMicIn_FsChange(u32 u4NewFs);


#endif //VIRTUAL_MICIN_H



