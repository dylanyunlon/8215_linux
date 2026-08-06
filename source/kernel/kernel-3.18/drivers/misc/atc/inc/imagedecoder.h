#ifndef __JPGDECODER_H__
#define __JPGDECODER_H__

#include "windows.h"

uintptr_t JPG_Init(LPCTSTR szContext, LPCVOID pBusContext);

BOOL JPG_Deinit( uintptr_t context );

DWORD JPG_Open(uintptr_t context, uintptr_t accessCode, uintptr_t shareMode);

BOOL JPG_Close(DWORD context);

extern BOOL JPG_IOControl( uintptr_t hDevice, uintptr_t dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, 
					LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned);





#endif 

