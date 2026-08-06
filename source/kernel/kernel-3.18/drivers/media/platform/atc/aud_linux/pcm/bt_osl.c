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

/******************************************************************************
*[Description]
*	 Implementation for memory malloc/free for AEC/NDC
*
*
******************************************************************************/


#include <linux/time.h>

#include "bt_osl.h"
#include "aud_pcm_dbg.h"

#include "pcm_debug.h"
#define LOG_TAG "btosl"

#define BT_FAST_MEM_SIZE	0x20000U
#define BT_MEM_SIZE			0U

static u8 BT_FAST_MEM_START[BT_FAST_MEM_SIZE];

static u8 *_pbFastMemory;
static u32 _u4FastMallocSize;
static u32 _u4FastMaxSize;

static u8 *_pbMemory;
static u32 _u4MaxSize;
static u32 _u4MallocSize;


u32 BT_MemoryInit(void)
{
	_pbFastMemory = (u8 *)BT_FAST_MEM_START;
	_u4FastMaxSize = BT_FAST_MEM_SIZE;
	_u4FastMallocSize = 0;

	_pbMemory = NULL;
	_u4MaxSize = BT_MEM_SIZE;
	_u4MallocSize = 0;

	return 0;
}

u32 BT_MemoryUninit(void)
{
	_u4MaxSize = 0;
	_u4MallocSize = 0;

	return 0;
}

void *BT_Malloc(u32 u4Size)
{
	void *pvRet = NULL;

	u4Size += 3U;
	u4Size &= 0xFFFFFFFC;

	if (_pbFastMemory && (u4Size + _u4FastMallocSize <= _u4FastMaxSize)) {
		_u4FastMallocSize += u4Size;
		pvRet = (_pbFastMemory + _u4FastMallocSize - u4Size);
	} else if ((u4Size + _u4MallocSize) <= _u4MaxSize) {
		_u4MallocSize += u4Size;
		pvRet = (_pbMemory + _u4MallocSize - u4Size);
	} else {
		PCM_ERROR(LOG_TAG, "BT_Malloc: return failed. u4Size (%d)\r\n", (s32)u4Size);
	}

	return pvRet;
}

void BT_Free(void *pvMemory)
{
}

u64 BTGetSysFrequency(void)
{
	return 1000000U;
}

u64 BTGetSysTick(void)
{
	struct timespec tv = {0};

	getnstimeofday(&tv);

	return (u64)(((u64)tv.tv_sec) * 1000000U + ((u64)tv.tv_nsec) / 1000U);
}

u64 GetARM2TickCount(void)
{
	struct timespec tv = {0};

	getnstimeofday(&tv);

	return (u64)(((u64)tv.tv_sec) * 1000000U + ((u64)tv.tv_nsec) / 1000U);
}

