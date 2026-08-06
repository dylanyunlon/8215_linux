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


/*****************************************************************************
*  Audio Driver: Interface
*****************************************************************************/

#ifndef _MEMDBGC_H_
#define _MEMDBGC_H_

#include "memchk_cfg.h"
#include "x_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

void *MemDbgAlloc(size_t nSize, char *file, int nLine);
void MemDbgFree(void *p, char *file, int nSize);
void *MemDbgRealloc(void *p, size_t size, char *file, int line);


#ifdef __linux__
void MemDump(void);
#else
void MemDump(void);
#endif


#if MEMCHK_DEBUG
#define MM_ALLOC(sz)		MemDbgAlloc(((size_t)(sz)), (char *)__FILE__, (int)__LINE__)
#define MM_FREE(sz)	 MemDbgFree(((void *)(sz)), (char *)__FILE__, (int)__LINE__)
#define MM_REALLOC(p, sz)	 MemDbgRealloc(((void *)(p)), ((size_t)(sz)), ((char *)__FILE__), ((int)__LINE__))
/* #define x_mem_alloc(x)                MemDbgAlloc((x), __FILE__, __LINE__) */
/* #define x_mem_free(x)          MemDbgFree((x), __FILE__, __LINE__) */
#else
/* #define MM_ALLOC(sz)          MemDbgAlloc((sz), __FILE__, __LINE__) */
/* #define MM_FREE(sz)    MemDbgFree((sz), __FILE__, __LINE__) */
/* #define MM_REALLOC(p, sz)      MemDbgRealloc((p), (sz), __FILE__, __LINE__) */

#define MM_ALLOC(sz)		calloc(1, sz)	/* malloc(sz) */
#define MM_FREE(sz)	 free((void *)sz)	/* free(sz) */
#define MM_REALLOC(p, sz)	 realloc((void *)p, (size_t)sz)	/* realloc(p, sz) */
/* #define x_mem_alloc   //(x)    malloc(x) */
/* #define x_mem_free            //free */
#endif

#define mm_memset  memset
#define mm_memcpy  memcpy
#define mm_memcmp  memcmp
#define mm_memmove memmove

#ifdef __cplusplus
}
#endif
#endif				/* _MEMDBGC_H_ */
