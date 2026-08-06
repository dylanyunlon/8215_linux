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


#ifndef BT_OS_LAYER_H
#define BT_OS_LAYER_H

#include <windows.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include "bt_cfg.h"


#if defined(__cplusplus)
extern "C" {
#endif

#define BT_SEEK_BGN		1U
#define BT_SEEK_CUR		2U
#define BT_SEEK_END		3U

#define MICROSECOND		1000000U

#define SPH_NAME TEXT("ARM1SPEECH")

#define ENABLE_PERFORMANCE_STAT		1U
#define ENABLE_CSV_OUTPUT			0U


typedef void *BT_HANDLE;

u32 BT_MemoryInit(void);
u32 BT_MemoryUninit(void);
void *BT_Malloc(u32 u4Size);
void  BT_Free(void *pvMemory);
u64 BTGetSysFrequency(void);
u64 BTGetSysTick(void);
u64 GetARM2TickCount(void);


#define BTMemCopy memcpy
#define BTMemSet  memset

#define ARM1PHY2ARM2UCV(x) (x)


#include "speech_dual.h"

#define TASK_IDLE			(0x1U)
#define TASK_BUSY			(0x2U)

#if defined(__cplusplus)
}
#endif

#endif
