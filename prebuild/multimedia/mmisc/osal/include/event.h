/*
* Copyright (c) 2016 AutoChips Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef __EVENT_H__
#define __EVENT_H__

#include "PrivLog.h"
#include "windows.h"
#include "misc/atc/inc/drv_win32_if.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
	char name[32];
	BOOL bManualReset;
	BOOL bInitialState;
} EVENT_PARA;

HANDLE CreateEvent(
	LPSECURITY_ATTRIBUTES lpEventAttributes,
	BOOL bManualReset,
	BOOL bInitialState,
	LPCTSTR lpName
);

HANDLE OpenEvent(
	DWORD dwDesiredAccess,
	BOOL bInheritHandle,
	LPCTSTR lpName
);

BOOL DestroyEvent(HANDLE hEvent);


DWORD WaitForSingleObject(
	HANDLE hHandle,
	DWORD dwMilliseconds
);

DWORD WaitForMultipleObjects(
	DWORD nCount,
	const HANDLE *lpHandles,
	BOOL bWaitAll,
	DWORD dwMilliseconds
);


BOOL ResetEvent(HANDLE hEvent);

BOOL SetEvent(HANDLE hEvent);

BOOL SetEventData(HANDLE hEvent, DWORD dwData);

DWORD GetEventData(HANDLE hEvent);

BOOL DestroyEvent(HANDLE hEvent);

DWORD GetTickCount(void);

DWORD GetLastError(void);

void SetLastError(DWORD dwErrCode);

#ifdef __cplusplus
}
#endif

#endif

