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
#ifndef __ARM2__
#include "windows.h"
#include "winutil.h"
#else
#include "x_types.h"
#endif
#include "fbm.h"
#include "log.h"
#include "vdp_hal.h"

HANDLE _CS;

#define INVALID_BUFFER_ID (-1)

static __s32             _i4Cur[3] = {0, 0 , 0};
static __s32             _i4Displaying[3] = {0};

static __s32             _i4TblSize[3] = {0};

static void*                   *_pTable[3];
static bool _fgCSReady = FALSE;
static void  LeaveCriticalSection(HANDLE lpCriticalSection);
static void  EnterCriticalSection(HANDLE lpCriticalSection);

void*  _apBuffer[3][IMG_RESZ_BUFF_SIZE];

__u32 FBM_Init(__u32 u4GroupID, void* *pTable, __s32 i4Size)
{
	VDO_LOG(VDO_LOG_LVL_DBG, "FBM_Init: %d %d\n", (int)u4GroupID, (int)i4Size);

	if (!_fgCSReady) {
		/*_CS = CreateSemaphore(NULL, 1, 1, NULL);*/
		_fgCSReady = TRUE;
	}

	if (_pTable[u4GroupID]) {
		/*vfree(_pTable[u4GroupID]);*/
		_pTable[u4GroupID] = NULL;
	}

	_pTable[u4GroupID] = _apBuffer[u4GroupID];/* (void* *)vmalloc(sizeof(void*) * i4Size);*/
	memcpy(_pTable[u4GroupID], pTable, sizeof(void*) * i4Size);
	_i4TblSize[u4GroupID] = i4Size;
	_i4Cur[u4GroupID] = 0;
	_i4Displaying[u4GroupID] = INVALID_BUFFER_ID;

	return TRUE;
}

void* FBM_Lock(__u32 u4GroupID)
{
	void* pBuffer;
	__u32 u4Cur = 0;

	if (!_pTable[u4GroupID]) {
		VDO_LOG(VDO_LOG_LVL_ERR, "FBM_Lock : Failed ..GRP %d\r\n", u4GroupID);
		return NULL;
	}

	EnterCriticalSection(&_CS);
	u4Cur = _i4Cur[u4GroupID];
	pBuffer = _pTable[u4GroupID][u4Cur];
	VDO_LOG(VDO_LOG_LVL_DBG, "FBM_Lock u4Cur = %d pbuffer = %x\n", u4Cur, (unsigned int)pBuffer);
	LeaveCriticalSection(&_CS);

	return pBuffer;
}

__u32 FBM_Unlock(__u32 u4GroupID)
{
	EnterCriticalSection(&_CS);
	LeaveCriticalSection(&_CS);

	return TRUE;
}

void* FBM_Flip(__u32 u4GroupID)
{
	void* pBuffer = NULL;
	__s32 *pi4Cur;

	if (!_pTable[u4GroupID]) {
		VDO_LOG(VDO_LOG_LVL_ERR, "FBM_Flip: Failed u4GroupID %d >>>\n", (int)u4GroupID);
		return NULL;
	}

	EnterCriticalSection(&_CS);
	pi4Cur  = &_i4Cur[u4GroupID];
	pBuffer = _pTable[u4GroupID][*pi4Cur];
	_i4Displaying[u4GroupID] = *pi4Cur;
	VDO_LOG(VDO_LOG_LVL_DBG, "FBM_Flip grp %d to %d\r\n", (int)u4GroupID, (int)pi4Cur[0]);
	(*pi4Cur)++;

	if (*pi4Cur >= _i4TblSize[u4GroupID]) {
		*pi4Cur = 0;
	}

	LeaveCriticalSection(&_CS);

	return pBuffer;
}

void* FBM_GetOnScreen(__u32 u4GroupID)
{
	void* pBuffer;
	__s32 *pi4On;

	if (!_pTable[u4GroupID]) {
		VDO_LOG(VDO_LOG_LVL_ERR, "FBM_GetOnScreen: Failed grp %d\r\n", (int)u4GroupID);
		return NULL;
	}

	EnterCriticalSection(&_CS);

	pi4On = &_i4Displaying[u4GroupID];
	VDO_LOG(VDO_LOG_LVL_DBG, "FBM_GetOnScreen grp %d to %d\r\n", (int)u4GroupID, (int)pi4On[0]);

	if (*pi4On == INVALID_BUFFER_ID) {
		pBuffer  = NULL;
	} else {
		pBuffer = _pTable[u4GroupID][*pi4On];
	}

	LeaveCriticalSection(&_CS);

	return pBuffer;
}


void FBM_Uninit(__u32 u4GroupID)
{
	if (_pTable[u4GroupID]) {
		/* vfree(_pTable[u4GroupID]);*/
		_pTable[u4GroupID]  = NULL;
		_i4TblSize[u4GroupID] = 0;
	}
}

bool FBM_IsNotEmpty(__u32 u4GroupID)
{
	bool ret = _pTable[u4GroupID] ? TRUE : FALSE;

	return ret;
}

void EnterCriticalSection(HANDLE lpCriticalSection)
{
	/* GetSemaphore(lpCriticalSection);*/
}

void  LeaveCriticalSection(HANDLE lpCriticalSection)
{
	/* ReleaseSemaphore(lpCriticalSection, 1, NULL);*/
}


