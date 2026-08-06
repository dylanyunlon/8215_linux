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
#include "windows.h"

static DEFINE_SPINLOCK(ac83xx_queue_lock);

#ifdef WIN32

#define QUEUE_TAG		0x12345606

#define QUEUE_CHECK_TAG(p)		do { \
	if (!p || ((X_QUEUE *)p)->queue_tag != QUEUE_TAG) { \
		RETAILMSG(1, (TEXT("QUEUE_TAG error\r\n"))); \
		return OSR_INV_ARG; \
	} \
} while (0)

typedef struct _QUEUE_INFO {
      void 					*pData;
      struct _QUEUE_INFO 	*pNext;
} QUEUE_INFO;

typedef struct {
	u32    queue_tag;
	u32    u4MaxSize;
	u32    u4Size;
	QUEUE_INFO *pHead;
	QUEUE_INFO *pFree;
	void *pMem;
} X_QUEUE;


__s32 x_queue_create(uintptr_t*  phQueue, u32 u4MaxSize)
{
	X_QUEUE *pQueue;
	QUEUE_INFO *pQueueInfo;
	__u32 i=0;

	if (!phQueue || !u4MaxSize)
		return OSR_INV_ARG;

	pQueue = (X_QUEUE *)LocalAlloc(LPTR, sizeof(*pQueue));
	if (!pQueue)
		return OSR_NO_RESOURCE;

	pQueueInfo = (QUEUE_INFO *)LocalAlloc(LPTR, sizeof(QUEUE_INFO) * u4MaxSize);

	if (!pQueueInfo) {
		LocalFree(pQueue);
		return OSR_NO_RESOURCE;
	}

	while(i < u4MaxSize - 1)
	{
		pQueueInfo[i].pNext = &pQueueInfo[i+1];
		i++;
	}

	pQueueInfo[i].pNext = NULL;

	pQueue->queue_tag = QUEUE_TAG;
	pQueue->u4MaxSize = u4MaxSize;
	pQueue->u4Size = 0;
	pQueue->pHead = NULL;
	pQueue->pFree = pQueueInfo;
	pQueue->pMem = (void *)pQueueInfo;
   
	*phQueue = (uintptr_t)pQueue;

	return OSR_OK;
}

__s32 x_queue_delete(uintptr_t hQueue)
{
	X_QUEUE *pQueue = (X_QUEUE *)hQueue;
	
	QUEUE_CHECK_TAG(pQueue);

	LocalFree(pQueue->pMem);
	LocalFree(pQueue);

	return OSR_OK;
}

__s32 x_queue_pop_head(uintptr_t hQueue, void **ppData)
{
	X_QUEUE *pQueue = (X_QUEUE *)hQueue;
	QUEUE_INFO *pQInfo;
	void *pData;
	unsigned long flags;
	
	QUEUE_CHECK_TAG(pQueue);

	if (pQueue->u4Size == 0)
	{
		 RETAILMSG(1, (TEXT("Queue is empty, can not pop data\r\n")));
		 return OSR_NO_RESOURCE;
	}
	spin_lock_irqsave(&ac83xx_queue_lock, flags);
	pQInfo = pQueue->pHead;
	pQueue->pHead = pQInfo->pNext;
	pData = pQInfo->pData;
	
	pQInfo->pData = NULL;
	pQInfo->pNext = pQueue->pFree;
	pQueue->pFree = pQInfo;
	pQueue->u4Size--;

	*ppData = pData;
	spin_unlock_irqrestore(&ac83xx_queue_lock, flags);
	return OSR_OK;
}

__s32 x_queue_push_tail(uintptr_t  hQueue, void *pData)
{
	X_QUEUE *pQueue = (X_QUEUE *)hQueue;
	QUEUE_INFO *pQInfo, *pFreeQInfo;
	unsigned long flags;

	QUEUE_CHECK_TAG(pQueue);	

	if (pQueue->u4Size >= pQueue->u4MaxSize)
	{
		 RETAILMSG(1, (TEXT("Queue is full, can not push data\r\n")));
		 return OSR_NO_RESOURCE;
	}
	spin_lock_irqsave(&ac83xx_queue_lock, flags);
	pQInfo = pQueue->pHead;
	while (pQInfo && pQInfo->pNext)
	   pQInfo = pQInfo->pNext;

	pFreeQInfo = pQueue->pFree;
	pQueue->pFree = pFreeQInfo->pNext;
	pFreeQInfo->pData = pData;
	pFreeQInfo->pNext = NULL;

	if (pQInfo)
		pQInfo->pNext = pFreeQInfo;
	else
		pQueue->pHead = pFreeQInfo;

	pQueue->u4Size++;
	spin_unlock_irqrestore(&ac83xx_queue_lock, flags);
	return OSR_OK;
}

__s32 x_queue_peek_nth(uintptr_t  hQueue, __u32 u4Index, void **ppData)  //get nth data
{
	X_QUEUE *pQueue = (X_QUEUE *)hQueue;
	QUEUE_INFO* pQInfo;
	__u32 i = 0;
	unsigned long flags;
	
	QUEUE_CHECK_TAG(pQueue);
	
	if (pQueue->u4Size == 0)
	{
		 RETAILMSG(1, (TEXT("Queue is empty, can not pop data\r\n")));
		 return OSR_NO_RESOURCE;
	}

	if((u4Index >= pQueue->u4MaxSize) || !ppData )
	{
		 RETAILMSG(1, (TEXT("Parameter is error\r\n")));
		 return OSR_INV_ARG;
	}
	spin_lock_irqsave(&ac83xx_queue_lock, flags);

	pQInfo = pQueue->pHead;
	while(i < u4Index)
	{
	   pQInfo = pQInfo->pNext;	
	   i++;
	}
	*ppData = pQInfo->pData;
	spin_unlock_irqrestore(&ac83xx_queue_lock, flags);
	return OSR_OK;
}

__s32 x_queue_get_length(uintptr_t  hQueue, __u32 *pLength)
{
	X_QUEUE *pQueue = (X_QUEUE *)hQueue;
	
	QUEUE_CHECK_TAG(pQueue);

	*pLength = pQueue->u4Size;

	return OSR_OK;
}


EXPORT_SYMBOL(x_queue_create);
EXPORT_SYMBOL(x_queue_delete);
EXPORT_SYMBOL(x_queue_pop_head);
EXPORT_SYMBOL(x_queue_push_tail);
EXPORT_SYMBOL(x_queue_peek_nth);
EXPORT_SYMBOL(x_queue_get_length);

#endif
