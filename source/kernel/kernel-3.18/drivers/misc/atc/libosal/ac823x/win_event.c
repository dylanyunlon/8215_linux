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

#ifndef _WIN_EVENT_
#define _WIN_EVENT_

#include <linux/types.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/virtio.h>

#include "x_os.h"
#include "types.h"
#include "drv_win32_if.h"
#include <linux/spinlock_types.h>

static DEFINE_SPINLOCK(ac83xx_event_lock);

#define WIN_EVENT_MAGIC_NUMBER	(0xF1020304UL)
#define WIN_EVENT_MAX_NAME_LEN	(32)
#define WIN_WQ_MAX_EVENT		(32)

typedef struct _WIN_EVENT {
	u32 magic;
	char name[WIN_EVENT_MAX_NAME_LEN];
	int refcnt;
	bool bSignaled;
	u32 dwData;
	bool bManualReset;
	bool bWaitingForFree;
	struct list_head wq_head;
	struct list_head ev_self;
	int mPID;
} WIN_EVENT;

#define WIN_WQ_MAGIC_NUMBER	(0xF1020305UL)

struct _WIN_WQ;

typedef struct _WIN_WQ_REF {
	struct list_head wq_ref;
	struct _WIN_WQ *pWQ;
	bool bUsed;
} WIN_WQ_REF;

typedef struct _WIN_WQ {
	u32 magic;
	bool bSignaled;
	struct list_head wq_self;
	wait_queue_head_t wq;
	WIN_WQ_REF refs[WIN_WQ_MAX_EVENT];
} WIN_WQ;

static int attach_event_to_wq(WIN_EVENT *pEvent, WIN_WQ *pWQ)
{
	int cnt;
	WIN_WQ_REF *pRef;

	for (cnt = 0; cnt < WIN_WQ_MAX_EVENT; cnt++) {
		pRef = &pWQ->refs[cnt];
		if (pRef->bUsed)
			continue;

		pRef->pWQ = pWQ;
		pRef->bUsed = true;
		list_add(&pRef->wq_ref, &pEvent->wq_head);
		return 0;
	}

	return -1;
}

static int dettach_event_from_wq(WIN_EVENT *pEvent, WIN_WQ *pWQ)
{
	WIN_WQ_REF *pRef = NULL;

	if (list_empty(&pEvent->wq_head))
		return -1;

	list_for_each_entry(pRef, &pEvent->wq_head, wq_ref) {
		if (pRef && pRef->pWQ == pWQ) {
			list_del(&pRef->wq_ref);
			pRef->bUsed = false;
			return 0;
		}
	}

	return -2;
}

static int unknownEvent(WIN_EVENT *pEvent)
{
    if ((u32)pEvent < PAGE_OFFSET) {
        printk("[OSAL] WIN32 pEvent error, 0x%x\n", (u32)pEvent);
        return -1;
    }

    if (pEvent->magic != WIN_EVENT_MAGIC_NUMBER)
    {
        printk("[OSAL] WIN32 pEvent error\n");
        return -2;
    }

    return 0;
}

LIST_HEAD(global_event_list_head);

void* sysCreateEvent(
    void* lpEventAttributes,
    bool bManualReset,
    bool bInitialState,
    char* lpName
)
{
	WIN_EVENT *pEvent = NULL;
	unsigned long flags;
	struct list_head *pglobal_list = &global_event_list_head;

	if (lpName && !list_empty(pglobal_list)) {
		//local_irq_save(flags);
		spin_lock_irqsave(&ac83xx_event_lock, flags);
		list_for_each_entry(pEvent, pglobal_list, ev_self) {
			if (pEvent && 0 < strlen(pEvent->name) && 0 == strncmp(pEvent->name, lpName,WIN_EVENT_MAX_NAME_LEN-1)) {
				// current->win32_errno = ERROR_ALREADY_EXISTS; //task_struct has no member 'win32_errno'
				pEvent->refcnt++;
				if (pEvent->bWaitingForFree)
					pEvent->bWaitingForFree = false;
				//local_irq_restore(flags);
				spin_unlock_irqrestore(&ac83xx_event_lock, flags);
				return (void*)pEvent;
			}
		}
		//local_irq_restore(flags);
		spin_unlock_irqrestore(&ac83xx_event_lock, flags);
	}

	pEvent = kmalloc(sizeof(*pEvent), GFP_KERNEL);
	if (!pEvent) {
		//current->win32_errno = -ENOMEM; //task_struct has no member 'win32_errno'
		return (void*)NULL;
	}

	memset(pEvent, 0, sizeof(*pEvent));

	pEvent->magic = WIN_EVENT_MAGIC_NUMBER;
	pEvent->refcnt = 1;
	pEvent->dwData = 0;
	pEvent->bSignaled = bInitialState;
	pEvent->bManualReset = bManualReset ? true : false;
	pEvent->bWaitingForFree = false;
	INIT_LIST_HEAD(&pEvent->wq_head);
	pEvent->mPID = (int)current->pid;
	if (lpName) {
		strncpy(pEvent->name, lpName, WIN_EVENT_MAX_NAME_LEN-1);
		pEvent->name[WIN_EVENT_MAX_NAME_LEN-1] = 0;
	}

	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	list_add(&pEvent->ev_self, pglobal_list);
	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	return (void*)pEvent;
}

bool sysDestroyEvent(void* hEvent)
{
	WIN_EVENT *pEvent = NULL;
	WIN_WQ *pWaitQueue = NULL;
	WIN_WQ_REF *pWqRef = NULL;
	unsigned long flags;
	bool bNeedFree = false;

	if (hEvent == NULL) {
		printk("[OSAL] Invalid arg \n");
		return FALSE;
	}
    pEvent = (WIN_EVENT *)hEvent;
    if (unknownEvent(pEvent)){
    	printk("[OSAL] unknownEvent from sysDestroyEvent\n");
        return FALSE;
}
	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	pEvent->refcnt--;

	if (pEvent->refcnt > 0) {
		//local_irq_restore(flags);
		spin_unlock_irqrestore(&ac83xx_event_lock, flags);
		return TRUE;
	}

	pEvent->bWaitingForFree = true;

	if (list_empty(&pEvent->wq_head)) {
		list_del(&pEvent->ev_self);
		bNeedFree = true;
	} else {
		list_for_each_entry(pWqRef, &pEvent->wq_head, wq_ref) {
			if (pWqRef) {
				pWaitQueue = pWqRef->pWQ;
				pWaitQueue->bSignaled = true;
				wake_up_all(&pWaitQueue->wq);
			}
		}
	}

	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	if (bNeedFree)
		kfree(pEvent);

	return TRUE;
}

bool sysSetEvent(void* hEvent)
{
	WIN_EVENT *pEvent = NULL; 
	WIN_WQ *pWaitQueue = NULL;
	WIN_WQ_REF *pWqRef = NULL;
	unsigned long flags;

	if (hEvent == NULL) {
		printk("[OSAL] Invalid arg \n");
		return FALSE;
	}
    pEvent = (WIN_EVENT *)hEvent;
    if (unknownEvent(pEvent)){
    	printk("[OSAL] unknownEvent from sysSetEvent\n");
        return FALSE;
	}
	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	pEvent->bSignaled = true;

	if (!list_empty(&pEvent->wq_head)) {
		list_for_each_entry(pWqRef, &pEvent->wq_head, wq_ref) {
			if (!pWqRef)
				continue;

			pWaitQueue = pWqRef->pWQ;
			pWaitQueue->bSignaled = true;
			wake_up_all(&pWaitQueue->wq);
		}
	}

	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	return TRUE;
}

bool sysSetEventData(void* hEvent, u32 dwData)
{
	WIN_EVENT *pEvent = NULL;
	unsigned long flags;

	if (hEvent == NULL) {
		printk("[OSAL] Invalid arg \n");
		return FALSE;
	}
    pEvent = (WIN_EVENT *)hEvent;
	if (unknownEvent(pEvent)){
    	printk("[OSAL] unknownEvent from sysSetEventData\n");
        return FALSE;
	}

	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	pEvent->dwData = dwData;
	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	return TRUE;
}

u32 sysGetEventData(void* hEvent)
{
	WIN_EVENT *pEvent = NULL;
	unsigned long flags;
	u32 dwResult;

	if (hEvent == NULL) {
		printk("[OSAL] Invalid arg \n");
		return FALSE;
	}
    pEvent = (WIN_EVENT *)hEvent;
	if (unknownEvent(pEvent)){
    	printk("[OSAL] unknownEvent from sysGetEventData\n");
        return FALSE;
	}

	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	dwResult = pEvent->dwData;
	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	return dwResult;
}

bool sysResetEvent(void* hEvent)
{
	WIN_EVENT *pEvent = NULL;
	unsigned long flags;

	if (hEvent == NULL) {
		printk("[OSAL] Invalid arg \n");
		return FALSE;
	}
    pEvent = (WIN_EVENT *)hEvent;

    if (unknownEvent(pEvent)){
    	printk("[OSAL] unknownEvent from sysResetEvent\n");
        return FALSE;
	}

	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	pEvent->bSignaled = false;
	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	return TRUE;
}

void* sysOpenEvent(
	u32 dwDesiredAccess,
	bool bInheritHandle,
	char* lpName
)
{
	WIN_EVENT *pEvent = NULL;
	unsigned long flags;

	if (lpName && !list_empty(&global_event_list_head)) {
		//local_irq_save(flags);
		spin_lock_irqsave(&ac83xx_event_lock, flags);
		list_for_each_entry(pEvent, &global_event_list_head, ev_self) {
			if (pEvent && 0 < strlen(pEvent->name) && 0 == strncmp(pEvent->name, lpName,WIN_EVENT_MAX_NAME_LEN-1)) {
				pEvent->refcnt++;
				//local_irq_restore(flags);
				spin_unlock_irqrestore(&ac83xx_event_lock, flags);
				return (void*)pEvent;
			}
		}
		//local_irq_restore(flags);
		spin_unlock_irqrestore(&ac83xx_event_lock, flags);
	}

	return (void*)NULL;
}

static const int quantum_ms = 1000 / HZ;

u32 sysWaitForMultipleObjects(
	u32 nCount,
	const void* *lpHandles,
	bool bWaitAll,
	u32 dwMilliseconds
)
{
	WIN_WQ *pWaitQueue = NULL;
	int cnt;
	WIN_EVENT *pEvent;
	u32 dwRet = WAIT_FAILED;
	unsigned long flags;

	if (nCount >= WIN_WQ_MAX_EVENT) {
		//current->win32_errno = ERROR_INVALID_ARG; //task_struct has no member 'win32_errno'
		return WAIT_FAILED;
	}
	
    spin_lock_irqsave(&ac83xx_event_lock, flags);
	
	if (nCount >= WIN_WQ_MAX_EVENT) {
		printk("[OSAL] nCount not right from sysWaitForMultipleObjects\r\n");
		return WAIT_FAILED;
	}
	
    for (cnt = 0; cnt < (int)nCount; cnt++) {
        pEvent = (WIN_EVENT *)lpHandles[cnt];
        if (unknownEvent(pEvent)) {
        	printk("[OSAL] unknownEvent from sysWaitForMultipleObjects\n");
            //current->win32_errno = ERROR_INVALID_ARG; //task_struct has no member 'win32_errno'
            return WAIT_FAILED;
        }
    }
    spin_unlock_irqrestore(&ac83xx_event_lock, flags);
	
	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	if (nCount >= WIN_WQ_MAX_EVENT) {
		printk("[OSAL] nCount not right from sysWaitForMultipleObjects\r\n");
		return WAIT_FAILED;
	}
	
	for (cnt = 0; cnt < nCount; cnt++) {
		pEvent = (WIN_EVENT *)lpHandles[cnt];
		if (unknownEvent(pEvent)) {
        	printk("[OSAL] unknownEvent from sysWaitForMultipleObjects 2\n");
            //current->win32_errno = ERROR_INVALID_ARG; //task_struct has no member 'win32_errno'
            return WAIT_FAILED;
        }
		if (pEvent->bSignaled) {
			if (!pEvent->bManualReset)
				pEvent->bSignaled = false;
			//local_irq_restore(flags);
			spin_unlock_irqrestore(&ac83xx_event_lock, flags);
			return (WAIT_OBJECT_0 + cnt);
		}
	}
	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	if (0 == dwMilliseconds) {
		return WAIT_TIMEOUT;
	}

	pWaitQueue = kmalloc(sizeof(*pWaitQueue), GFP_KERNEL);
	if (!pWaitQueue) {
		//current->win32_errno = -ENOMEM; //task_struct has no member 'win32_errno'
		return WAIT_FAILED;
	}

	memset(pWaitQueue, 0, sizeof(*pWaitQueue));

	pWaitQueue->magic = WIN_WQ_MAGIC_NUMBER;
	pWaitQueue->bSignaled = false;
	init_waitqueue_head(&pWaitQueue->wq);

	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	if (nCount >= WIN_WQ_MAX_EVENT) {
		printk("[OSAL] nCount not right from sysWaitForMultipleObjects\r\n");
		return WAIT_FAILED;
	}
	for (cnt = 0; cnt < nCount; cnt++) {
		pEvent = (WIN_EVENT *)lpHandles[cnt];
		if (unknownEvent(pEvent)) {
        	printk("[OSAL] unknownEvent from sysWaitForMultipleObjects 3\n");
            //current->win32_errno = ERROR_INVALID_ARG; //task_struct has no member 'win32_errno'
            return WAIT_FAILED;
        }
		attach_event_to_wq(pEvent, pWaitQueue);
	}

	for (cnt = 0; cnt < nCount; cnt++) {
		pEvent = (WIN_EVENT *)lpHandles[cnt];
		if (pEvent->bSignaled) {
			if (!pEvent->bManualReset)
				pEvent->bSignaled = false;
			//local_irq_restore(flags);
			spin_unlock_irqrestore(&ac83xx_event_lock, flags);
			dwRet = (WAIT_OBJECT_0 + cnt);
			goto _timeout;
		}
	}

	if (dwMilliseconds == 0xFFFFFFFFUL) {
		// if use gdb debug , use wait_event_interruptible
		//local_irq_restore(flags);
		spin_unlock_irqrestore(&ac83xx_event_lock, flags);
		wait_event_interruptible(pWaitQueue->wq, pWaitQueue->bSignaled);
		//wait_event(pWaitQueue->wq, pWaitQueue->bSignaled);
	} else {
		//local_irq_restore(flags);
		spin_unlock_irqrestore(&ac83xx_event_lock, flags);
		if (0 == wait_event_interruptible_timeout(pWaitQueue->wq, pWaitQueue->bSignaled, dwMilliseconds/quantum_ms)) {
			dwRet = WAIT_TIMEOUT;
			goto _timeout;
		}
	}

	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	if (nCount >= WIN_WQ_MAX_EVENT) {
		printk("[OSAL] nCount not right from sysWaitForMultipleObjects\r\n");
		return WAIT_FAILED;
	}
	for (cnt = 0; cnt < nCount; cnt++) {
		pEvent = (WIN_EVENT *)lpHandles[cnt];
		if (unknownEvent(pEvent)) {
        	printk("[OSAL] unknownEvent from sysWaitForMultipleObjects 4\n");
            //current->win32_errno = ERROR_INVALID_ARG; //task_struct has no member 'win32_errno'
            return WAIT_FAILED;
        }
		if (pEvent->bSignaled) {
			dwRet = (WAIT_OBJECT_0 + cnt);
			if (!pEvent->bManualReset)
				pEvent->bSignaled = false;
			break;
		} else if (pEvent->bWaitingForFree) {
			dwRet = WAIT_FAILED;
			break;
		}
	}
	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

_timeout:
	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	if (nCount >= WIN_WQ_MAX_EVENT) {
		printk("[OSAL] nCount not right from sysWaitForMultipleObjects\r\n");
		return WAIT_FAILED;
	}
	for (cnt = 0; cnt < nCount; cnt++) {
		pEvent = (WIN_EVENT *)lpHandles[cnt];
		if (unknownEvent(pEvent)) {
        	printk("[OSAL] unknownEvent from sysWaitForMultipleObjects 4\n");
            //current->win32_errno = ERROR_INVALID_ARG; //task_struct has no member 'win32_errno'
            return WAIT_FAILED;
        }
		//list_del(&pWaitQueue->wq_self);
		dettach_event_from_wq(pEvent, pWaitQueue);
		if (pEvent->bWaitingForFree && list_empty(&pEvent->wq_head)) {
			list_del(&pEvent->ev_self);
			kfree(pEvent);
		}
	}
	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	kfree(pWaitQueue);

	return dwRet;
}

#endif // _WIN_EVENT_
