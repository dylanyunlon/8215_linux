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
#ifndef __WINDOWS_H__
#define __WINDOWS_H__

//#include "x_typedef.h"
//#include "GDef.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include "x_file.h"
//#include "types.h"
#include "assert.h"
#include "event.h"
//#include "x_assert.h"
#include <stdlib.h>

#define TEXT(x)  x
#define _T(x) x 
#define E_FAIL  (0x8004005U)

#define MAX_PATH (512)

#define MSDKCORE_DEBUG(x...)  LOG_ModWmsgD(LOG_MOD_MSDKCORE , x )

#define RETAILMSG(fg, x... )  \
do { \
	if (fg) { \
		MSDKCORE_DEBUG x ; \
		LOG_ModWmsgT(LOG_MOD_NONE, "\n" ); \
	} \
} while ( 0 )

/*
#define MM_FREE(sz) LocalFree(sz)
#define MM_ALLOC(sz) LocalAlloc(0, sz)
*/
#define MAKEFOURCC(a,b,c,d) 	( ((uint32_t)d) | ( ((uint32_t)c) << 8 ) | ( ((uint32_t)b) << 16 ) | ( ((uint32_t)a) << 24 ) ) 

typedef struct {
  WORD  wFormatTag;
  WORD  nChannels;
  DWORD nSamplesPerSec;
  DWORD nAvgBytesPerSec;
  WORD  nBlockAlign;
  WORD  wBitsPerSample;
  WORD  cbSize;
} __attribute__ ((packed)) WAVEFORMATEX; 

typedef INT64 REFERENCE_TIME;
#if 0
typedef struct _RECT { 
    LONG left; 
    LONG top; 
    LONG right; 
    LONG bottom; 
} RECT; 
#endif
typedef struct tagVIDEOINFOHEADER {
    RECT                rcSource;
    RECT                rcTarget;
    DWORD               dwBitRate;
    DWORD               dwBitErrorRate;
    REFERENCE_TIME      AvgTimePerFrame;
    BITMAPINFOHEADER    bmiHeader;
} VIDEOINFOHEADER;


#define LPTR 0 
static inline void* LocalAlloc(unsigned int flag , size_t size)
{
	void *p = malloc(size);
	if ( p != NULL )
		memset(p,0,size);
	return p;
}

static inline void* LocalFree(void *p)
{
	free(p);
	return NULL;
}


#define _tcsrchr strrchr
#define _tcslen  strlen
#define _tcscpy strcpy
#define _tcscmp strcmp
static inline char *_tcslwr(char *string)
{
    char   *cp;
    int     ch;

    for (cp = string; (ch = *cp) != 0; cp++)
        if (isupper(ch))
            *cp = tolower(ch);
    return (string);
}

#define x_memset memset
#define x_strlen strlen 
#define x_memcpy memcpy
#define x_strncpy strncpy
#define x_memcmp memcmp

#define Sleep(n) usleep(1000*n)

#define ASSERT(x)
#define VERIFY(x)
#endif 

