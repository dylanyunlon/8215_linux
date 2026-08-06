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

#ifndef __WINUTIL_H__
#define __WINUTIL_H__

#include "x_typedef.h"
#include "drv_win32_if.h"

#if 0
enum {
	NO_ERROR = 0,
	ERROR_ALREADY_EXISTS = -0x1111,
	ERROR_OTHER = -0x2222
};

#define WAIT_TIMEOUT	(0x00000102L)
#define WAIT_FAILED		0xFFFFFFFFUL
#define WAIT_OBJECT_0	(0)

#define INFINITE  (0xFFFFFFFFul)

#define PAGE_READWRITE  (0x100)

#define EVENT_ALL_ACCESS  (0)
#endif

#if 1
enum {
	GENERIC_READ = 0x1 ,
	GENERIC_WRITE = 0x2,
	GENERIC_RW = GENERIC_READ | GENERIC_WRITE,
	OPEN_EXISTING ,
	FILE_SHARE_READ,
	FILE_SHARE_WRITE,
	FILE_ATTRIBUTE_NORMAL,
	FILE_BEGIN,
	CREATE_ALWAYS,
};
#endif

typedef struct {
	char name[32];
	bool bManualReset;
	bool bInitialState;
} EVENT_PARA_T;

#if 0
void* CreateSemaphore(
	void* lpSemaphoreAttributes,
	s64 lInitialCount,
	s64 lMaximumCount,
	char* lpName
);

void DeleteSemaphore(void* hSemaphore);

bool ReleaseSemaphore(
	void* hSemaphore,
	s64 lReleaseCount,
	u64* lpPreviousCount
);


u32 GetLastError(void);
void SetLastError(u32 dwErrCode);

void GetSemaphore(void* hSemaphore);

void* OpenEvent(
	u32 dwDesiredAccess,
	bool bInheritHandle,
	char* lpName
);

void* CreateEvent(
    void* lpEventAttributes,
    bool bManualReset,
    bool bInitialState,
    char* lpName
);

bool ResetEvent(void* hEvent);

bool SetEvent(void* hEvent );

bool SetEventData(void* hEvent, u32 dwData);

u32 GetEventData(void* hEvent);

bool DestroyEvent(void* hEvent);

u32 WaitForMultipleObjects(u32 nCount,
							const void* *lpHandles,
							bool bWaitAll,
							u32 dwMilliseconds
						);


bool InitWin32Envrionment(void);

bool DeinitWin32Envrionment(void);

#if 1
void* CreateFile(
  char* lpFileName,
  u32 dwDesiredAccess,
  u32 dwShareMode,
  void* lpSecurityAttributes,
  u32 dwCreationDispostion,
  u32 dwFlagsAndAttributes,
  void* hTemplateFile
);

bool CloseHandle(void* hObject);
#endif
#endif
#endif

