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

#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/spinlock_types.h>

#include "x_os.h"
#include <media/atc/memdbg_c.h>
#include "x_assert.h"
#include "windows.h"

static DEFINE_SPINLOCK(ac83xx_mem_chunk_lock);

#ifdef WIN32

#define MEM_CHUNK_TAG			0x12345608

#define MEM_CHUNK_CHECK_TAG(p)		do { \
	if (!p || ((X_MEM_CHUNK *)p)->mem_chunk_tag != MEM_CHUNK_TAG) \
		return OSR_INV_ARG; \
} while (0)

typedef struct _INDEX_INFO {
	u32				u4Id;
	struct _INDEX_INFO	*pNext;
} INDEX_INFO;

typedef struct {
	u32		mem_chunk_tag;
	u32		u4MemSize;
	u32		u4ChunkSize;
	void		*pMem;
	INDEX_INFO	*pIndexArray;
	INDEX_INFO	*pFree;
	u32		u4FreeSize;
} X_MEM_CHUNK;

__s32 x_mem_chunk_create(uintptr_t *phMemChunk,
								u32	u4MemSize,
								u32	u4ChunkSize)
{
	X_MEM_CHUNK *pMemChunk = NULL;
	INDEX_INFO *pIndexInfo;
	__u32 u4Cnt;

	if (!phMemChunk)
		return OSR_INV_ARG;

	//pMemChunk = (X_MEM_CHUNK *)LocalAlloc(LPTR, sizeof(X_MEM_CHUNK));
	pMemChunk = (X_MEM_CHUNK *)kmalloc(sizeof(X_MEM_CHUNK), GFP_KERNEL);
	if (!pMemChunk)
		return OSR_NO_RESOURCE;

	memset(pMemChunk, 0, sizeof(*pMemChunk));

	if (u4MemSize & 0x3)
		u4MemSize = (u4MemSize & ~0x03) + 4;

	//pMemChunk->pMem = LocalAlloc(LPTR, (u4MemSize + sizeof(INDEX_INFO))* u4ChunkSize);
	pMemChunk->pMem = kmalloc((u4MemSize + sizeof(INDEX_INFO))* u4ChunkSize, GFP_KERNEL);
	if (!pMemChunk->pMem) {
		//LocalFree(pMemChunk);
		kfree(pMemChunk);
		return OSR_NO_RESOURCE;
	}

	pMemChunk->mem_chunk_tag = MEM_CHUNK_TAG;
	pMemChunk->u4MemSize = u4MemSize;
	pMemChunk->u4ChunkSize = u4ChunkSize;
	pMemChunk->pIndexArray = (INDEX_INFO *)((u32)pMemChunk->pMem + u4MemSize * u4ChunkSize);
	pMemChunk->pFree = NULL;
	pMemChunk->u4FreeSize = u4ChunkSize;

	for (u4Cnt = 0; u4Cnt < u4ChunkSize; u4Cnt++) {
		pIndexInfo = &pMemChunk->pIndexArray[u4Cnt];
		pIndexInfo->u4Id = u4Cnt;
		pIndexInfo->pNext = pMemChunk->pFree;
		pMemChunk->pFree = pIndexInfo;
	}

	*phMemChunk = (uintptr_t)pMemChunk;

	return OSR_OK;
}

__s32 x_mem_chunk_delete(__u32 hMemChunk)
{
	X_MEM_CHUNK *pMemChunk = (X_MEM_CHUNK *)hMemChunk;

	MEM_CHUNK_CHECK_TAG(pMemChunk);

	if (pMemChunk->pMem == NULL) {
		return OSR_INV_ARG;
	}

	//LocalFree(pMemChunk->pMem);
	//LocalFree(pMemChunk);
	kfree(pMemChunk->pMem);
	pMemChunk->pMem = NULL;
	kfree(pMemChunk);
	pMemChunk = NULL;

	return OSR_OK;
}

__s32 x_mem_chunk_alloc(uintptr_t hMemChunk, void **pptr)
{
	X_MEM_CHUNK *pMemChunk = (X_MEM_CHUNK *)hMemChunk;
	INDEX_INFO *pIndexInfo;
	unsigned long flags;

	MEM_CHUNK_CHECK_TAG(pMemChunk);

	if (pMemChunk->u4FreeSize == 0)
		return OSR_NO_RESOURCE;

	pIndexInfo = pMemChunk->pFree;
	ASSERT(pIndexInfo);
	spin_lock_irqsave(&ac83xx_mem_chunk_lock, flags);
	pMemChunk->pFree = pIndexInfo->pNext;
	pMemChunk->u4FreeSize--;
	pIndexInfo->pNext = NULL;

	*pptr = (void *)((uintptr_t)pMemChunk->pMem + (pIndexInfo->u4Id * pMemChunk->u4MemSize));
	spin_unlock_irqrestore(&ac83xx_mem_chunk_lock, flags);
	return OSR_OK;
}

__s32 x_mem_chunk_free(uintptr_t hMemChunk, void *ptr)
{
	X_MEM_CHUNK *pMemChunk = (X_MEM_CHUNK *)hMemChunk;
	INDEX_INFO *pIndexInfo;
	__u32 u4Index;
	unsigned long flags;

	MEM_CHUNK_CHECK_TAG(pMemChunk);

	ASSERT(ptr >= pMemChunk->pMem && (uintptr_t)ptr < ((uintptr_t)pMemChunk->pMem + pMemChunk->u4MemSize * pMemChunk->u4ChunkSize));
	u4Index = ((uintptr_t)ptr - (uintptr_t)pMemChunk->pMem) / pMemChunk->u4MemSize;
	pIndexInfo = &pMemChunk->pIndexArray[u4Index];

	// TODO, Check if ptr in free list
	ASSERT(pIndexInfo->pNext == NULL);
	spin_lock_irqsave(&ac83xx_mem_chunk_lock, flags);
	pIndexInfo->pNext = pMemChunk->pFree;
	pMemChunk->pFree = pIndexInfo;
	pMemChunk->u4FreeSize++;
	spin_unlock_irqrestore(&ac83xx_mem_chunk_lock, flags);

	return OSR_OK;
}

EXPORT_SYMBOL(x_mem_chunk_create);
EXPORT_SYMBOL(x_mem_chunk_delete);
EXPORT_SYMBOL(x_mem_chunk_alloc);
EXPORT_SYMBOL(x_mem_chunk_free);

#endif
