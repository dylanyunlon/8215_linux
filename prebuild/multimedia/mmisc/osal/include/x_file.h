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
#ifndef __X_FILE_H__
#define __X_FILE_H__

#include "GDef.h"
#include "misc/atc/inc/types.h"
#include "windows.h"

#define INVALID_SET_FILE_POINTER (-1)

#ifdef __cplusplus
extern "C"
{
#endif

enum {
	GENERIC_READ = 0x1 , 
	GENERIC_WRITE = 0x2,
	GENERIC_RW = GENERIC_READ | GENERIC_WRITE,
	OPEN_EXISTING ,
	FILE_SHARE_READ,
	FILE_SHARE_WRITE,
	FILE_ATTRIBUTE_NORMAL,
	FILE_BEGIN,

	GetFileExInfoStandard
};

typedef struct {
	DWORD nFileSizeHigh;
	DWORD nFileSizeLow;

} WIN32_FILE_ATTRIBUTE_DATA  ;

typedef DWORD (*LPTHREAD_START_ROUTINE) ( LPVOID lpThreadParameter);
typedef int GET_FILEEX_INFO_LEVELS;

HANDLE CreateFile(
  LPCTSTR lpFileName, 
  DWORD dwDesiredAccess, 
  DWORD dwShareMode, 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
  DWORD dwCreationDispostion, 
  DWORD dwFlagsAndAttributes, 
  HANDLE hTemplateFile
); 

BOOL GetFileAttributesEx(
  LPCTSTR lpFileName, 
  GET_FILEEX_INFO_LEVELS fInfoLevelId, 
  LPVOID lpFileInformation 
);

DWORD SetFilePointer(
	HANDLE hFile,
	LONG lDistanceToMove,
	PLONG lpDistanceToMoveHigh,
	DWORD dwMoveMethod
);

BOOL ReadFile(
	HANDLE hFile,
	LPVOID lpBuffer,
	DWORD nNumberOfBytesToRead,
	LPDWORD lpNumberOfBytesRead,
	LPOVERLAPPED lpOverlapped
);

BOOL CloseHandle(HANDLE hObject);

BOOL DeviceIoControl(HANDLE hDevice, DWORD IoControlCOde, VOID* lpInBuf, DWORD InBufSize ,
                    VOID* lpOutBuf, DWORD OutBufSize , DWORD* lpBytesReturned, VOID* reserver3);

HANDLE CreateThread(
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	SIZE_T dwStackSize,
	LPTHREAD_START_ROUTINE lpStartAddress,
	LPVOID lpParameter,
	DWORD dwCreationFlags,
	LPDWORD lpThreadId
);

BOOL UnmapViewOfFile( LPCVOID lpBaseAddress );

#ifdef __linux__
DWORD GetFileSize( LPCTSTR lpFileName, LPDWORD lpFileSizeHigh);
#else
DWORD GetFileSize( HANDLE hFile, LPDWORD lpFileSizeHigh);
#endif

#ifdef __cplusplus
}
#endif


#endif 

