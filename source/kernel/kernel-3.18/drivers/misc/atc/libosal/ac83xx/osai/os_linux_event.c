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



#include <asm/current.h>
#include <linux/module.h>
#include <linux/sched.h>
 
#include "x_typedef.h"
#include "types.h"
#include "x_os.h"
 
#if 1
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/slab.h>

#endif

#if 0
#define debug_printk  printk
#define FUNC_LOG	printk("enter %s\n", __FUNCTION__ )
#else
#define debug_printk(x...)
#define FUNC_LOG
#endif

static DEFINE_SPINLOCK(ac83xx_event_lock);

#define OS_LINUX_EVENT_MAGIC_NUMBER	(0xF1020304UL)
#define OS_LINUX_EVENT_MAX_NAME_LEN	(32)
#define OS_LINUX_Q_MAX_EVENT		(32)

#define WAIT_TIMEOUT    (0x00000102L)
#define WAIT_FAILED     0xFFFFFFFFUL
#define WAIT_OBJECT_0   (0)

#ifdef KERNEL_STANDARD_API
#include <linux/types.h> 

typedef struct _OS_LINUX_EVENT_T {
     u32 magic;
     char name[OS_LINUX_EVENT_MAX_NAME_LEN];
     s32 refcnt;
     bool bSignaled;
     unsigned long data;
     bool bManualReset;
     bool bWaitingForFree;
     struct list_head wq_head;
     struct list_head ev_self;
     s32 mPID;
 } OS_LINUX_EVENT_T;

 
#define OS_LINUX_EVT_Q_MAGIC_NUMBER	(0xF1020305UL)
 
 struct _OS_LINUX_EVT_Q;
 
 typedef struct _OS_LINUX_EVT_Q_REF {
     struct list_head evt_q_ref;
     struct OS_LINUX_EVT_Q *pWQ;
     bool bUsed;
 } OS_LINUX_EVT_Q_REF;
 
 typedef struct _OS_LINUX_EVT_Q {
     u32 magic;
     bool bSignaled;
     struct list_head evt_q_self;
     wait_queue_head_t evt_q;
     OS_LINUX_EVT_Q_REF refs[OS_LINUX_Q_MAX_EVENT];
 } OS_LINUX_EVT_Q;

 static const s32 quantum_ms = (s32)((s32)1000 / (s32)HZ);

 /*-----------------------------------------
  Private function
  ------------------------------------------*/
 static s32 attach_event_to_wq(OS_LINUX_EVENT_T *pEvent, OS_LINUX_EVT_Q *pWQ)
 {
     s32 cnt;
     OS_LINUX_EVT_Q_REF *pRef;
 
     for (cnt = 0; cnt < OS_LINUX_Q_MAX_EVENT; cnt++) {
         pRef = &pWQ->refs[cnt];
         if (pRef->bUsed)
             continue;
 
         pRef->pWQ = pWQ;
         pRef->bUsed = true;
         list_add(&pRef->evt_q_ref, &pEvent->wq_head);
         return 0;
     }
 
     return -1;
 }
 
 static s32 dettach_event_from_wq(OS_LINUX_EVENT_T *pEvent, OS_LINUX_EVT_Q *pWQ)
 {
     OS_LINUX_EVT_Q_REF *pRef = NULL;
 
     if (list_empty(&pEvent->wq_head))
         return -1;
 
     list_for_each_entry(pRef, &pEvent->wq_head, evt_q_ref) {
         if ((pRef != NULL) && (pRef->pWQ == pWQ)) {
             list_del(&pRef->evt_q_ref);
             pRef->bUsed = false;
             return 0;
         }
     }
 
     return -2;
 }
 
 static s32 unknown_event(OS_LINUX_EVENT_T *pEvent)
 {
     if ((u32)pEvent < PAGE_OFFSET) {
         printk("[OSAL] pEvent error, 0x%x\n", (u32)pEvent);
         return -1;
     }
 
     if (pEvent->magic != OS_LINUX_EVENT_MAGIC_NUMBER)
     {
         printk("[OSAL] pEvent error\n");
         return -2;
     }
 
     return 0;
 }
 
 LIST_HEAD(g_event_list_head);

 /*-----------------------------------------
 Public function
 ------------------------------------------*/
 void* x_event_open(
     unsigned long dwDesiredAccess,
     bool bInheritHandle,
     const char* lpName
 )
 {
	OS_LINUX_EVENT_T *pEvent = NULL;
	unsigned long flags;

	if ((lpName != NULL) && !list_empty(&g_event_list_head)) {
		//local_irq_save(flags);
		spin_lock_irqsave(&ac83xx_event_lock, flags);
		list_for_each_entry(pEvent, &g_event_list_head, ev_self) {
			if ((pEvent != NULL) && ((size_t)0 < strlen(pEvent->name)) && (0 == strncmp(pEvent->name, lpName,(size_t)(OS_LINUX_EVENT_MAX_NAME_LEN-1)))) {
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

 
 bool x_event_set( void* hEvent )
 {
	OS_LINUX_EVENT_T *pEvent = NULL; 
	OS_LINUX_EVT_Q *pWaitQueue = NULL;
	OS_LINUX_EVT_Q_REF *pWqRef = NULL;
	unsigned long flags;

	if (hEvent == NULL) {
		printk("[OSAL] Invalid arg \n");
		return false;
	}
    pEvent = (OS_LINUX_EVENT_T *)hEvent;
    if (unknown_event(pEvent)){
    	printk("[OSAL] unknownEvent from sysSetEvent\n");
        return false;
	}
	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	pEvent->bSignaled = true;

	if (!list_empty(&pEvent->wq_head)) {
		list_for_each_entry(pWqRef, &pEvent->wq_head, evt_q_ref) {
			if (!pWqRef)
				continue;

			pWaitQueue = pWqRef->pWQ;
			pWaitQueue->bSignaled = true;
			wake_up_all(&(pWaitQueue->evt_q));
		}
	}

	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	return true;
}
 
 bool x_event_set_data(void* hEvent, unsigned long dwData)
 {
	OS_LINUX_EVENT_T *pEvent = NULL;
	unsigned long flags;

	if (hEvent == NULL) {
		printk("[OSAL] Invalid arg \n");
		return false;
	}
    pEvent = (OS_LINUX_EVENT_T *)hEvent;
	if (unknown_event(pEvent)){
    	printk("[OSAL] unknownEvent from sysSetEventData\n");
        return false;
	}

	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	pEvent->data = dwData;
	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	return true;
}
  
 unsigned long x_event_get_data(void* hEvent)
 {
	OS_LINUX_EVENT_T *pEvent = NULL;
	unsigned long flags;
	unsigned long dwResult;

	if (hEvent == NULL) {
		printk("[OSAL] Invalid arg \n");
		return false;
	}
    pEvent = (OS_LINUX_EVENT_T *)hEvent;
	if (unknown_event(pEvent)){
    	printk("[OSAL] unknownEvent from sysGetEventData\n");
        return false;
	}

	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	dwResult = pEvent->data;
	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	return dwResult;
}
 
 bool x_event_reset(void* hEvent)
 {
	OS_LINUX_EVENT_T *pEvent = NULL;
	unsigned long flags;

	if (hEvent == NULL) {
		printk("[OSAL] Invalid arg \n");
		return false;
	}
    pEvent = (OS_LINUX_EVENT_T *)hEvent;

    if (unknown_event(pEvent)){
    	printk("[OSAL] unknownEvent from sysResetEvent\n");
        return false;
	}

	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	pEvent->bSignaled = false;
	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	return true;
}
 
 
 bool x_event_destroy(void* hEvent)
 {
	OS_LINUX_EVENT_T *pEvent = NULL;
	OS_LINUX_EVT_Q *pWaitQueue = NULL;
	OS_LINUX_EVT_Q_REF *pWqRef = NULL;
	unsigned long flags;
	bool bNeedFree = false;

	if (hEvent == NULL) {
		printk("[OSAL] Invalid arg \n");
		return false;
	}
    pEvent = (OS_LINUX_EVENT_T *)hEvent;
    if (unknown_event(pEvent)){
    	printk("[OSAL] unknownEvent from sysDestroyEvent\n");
        return false;
}
	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	pEvent->refcnt--;

	if (pEvent->refcnt > 0) {
		//local_irq_restore(flags);
		spin_unlock_irqrestore(&ac83xx_event_lock, flags);
		return true;
	}

	pEvent->bWaitingForFree = true;

	if (list_empty(&pEvent->wq_head)) {
		list_del(&pEvent->ev_self);
		bNeedFree = true;
	} else {
		list_for_each_entry(pWqRef, &pEvent->wq_head, evt_q_ref) {
			if (pWqRef) {
				pWaitQueue = pWqRef->pWQ;
				pWaitQueue->bSignaled = true;
				wake_up_all(&(pWaitQueue->evt_q));
			}
		}
	}

	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	if (bNeedFree)
		kfree(pEvent);

	return true;
}
 

 
 void* x_event_create(
     void* lpEventAttributes,
     bool bManualReset,
     bool bInitialState,
     const char* lpName
 )
 {
	OS_LINUX_EVENT_T *pEvent = NULL;
	unsigned long flags;
	struct list_head *pglobal_list = &g_event_list_head;

	if ((lpName != NULL) && !list_empty(pglobal_list)) {
		//local_irq_save(flags);
		spin_lock_irqsave(&ac83xx_event_lock, flags);
		list_for_each_entry(pEvent, pglobal_list, ev_self) {
			if ((pEvent != NULL) && ((size_t)0 < strlen(pEvent->name)) && (0 == strncmp(pEvent->name, lpName,(size_t)(OS_LINUX_EVENT_MAX_NAME_LEN-1)))) {
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

	memset((void *)pEvent, 0, sizeof(*pEvent));

	pEvent->magic = OS_LINUX_EVENT_MAGIC_NUMBER;
	pEvent->refcnt = 1;
	pEvent->data = 0;
	pEvent->bSignaled = bInitialState;
	pEvent->bManualReset = bManualReset ? true : false;
	pEvent->bWaitingForFree = false;
	INIT_LIST_HEAD(&pEvent->wq_head);
	pEvent->mPID = (s32)current->pid;
	if (lpName) {
		strncpy(pEvent->name, lpName, (size_t)(OS_LINUX_EVENT_MAX_NAME_LEN-1));
		pEvent->name[OS_LINUX_EVENT_MAX_NAME_LEN-1] = (char)0;
	}

	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	list_add(&pEvent->ev_self, pglobal_list);
	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	return (void*)pEvent;
}
 

 unsigned long x_event_wait_for_objects(
     unsigned long nCount,
     const void* *lpHandles,
     bool bWaitAll,
     unsigned long dwMilliseconds
 )
 {
	OS_LINUX_EVT_Q *pWaitQueue = NULL;
	s32 cnt;
	OS_LINUX_EVENT_T *pEvent;
	unsigned long dwRet = (unsigned long)WAIT_FAILED;
	unsigned long flags;

	if (nCount >= (unsigned long)OS_LINUX_Q_MAX_EVENT) {
		//current->win32_errno = ERROR_INVALID_ARG; //task_struct has no member 'win32_errno'
		return (unsigned long)WAIT_FAILED;
	}

    for (cnt = 0; cnt < (s32)nCount; cnt++) {
        pEvent = (OS_LINUX_EVENT_T *)lpHandles[cnt];
        if (unknown_event(pEvent)) {
        	printk("[OSAL] unknownEvent from sysWaitForMultipleObjects\n");
            //current->win32_errno = ERROR_INVALID_ARG; //task_struct has no member 'win32_errno'
            return (unsigned long)WAIT_FAILED;
        }
    }

	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	for (cnt = 0; cnt < (s32)nCount; cnt++) {
		pEvent = (OS_LINUX_EVENT_T *)lpHandles[cnt];
		if (pEvent->bSignaled) {
			if (!pEvent->bManualReset)
				pEvent->bSignaled = false;
			//local_irq_restore(flags);
			spin_unlock_irqrestore(&ac83xx_event_lock, flags);
			return (unsigned long)((unsigned long)WAIT_OBJECT_0 + (unsigned long)cnt);
		}
	}
	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	if (0 == dwMilliseconds) {
		return (unsigned long)WAIT_TIMEOUT;
	}

	pWaitQueue = kmalloc(sizeof(*pWaitQueue), GFP_KERNEL);
	if (!pWaitQueue) {
		//current->win32_errno = -ENOMEM; //task_struct has no member 'win32_errno'
		return (unsigned long)WAIT_FAILED;
	}

	memset((void *)pWaitQueue, 0, sizeof(*pWaitQueue));

	pWaitQueue->magic = OS_LINUX_EVT_Q_MAGIC_NUMBER;
	pWaitQueue->bSignaled = false;
	init_waitqueue_head(&pWaitQueue->evt_q);

	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	for (cnt = 0; cnt < (s32)nCount; cnt++) {
		pEvent = (OS_LINUX_EVENT_T *)lpHandles[cnt];
		attach_event_to_wq(pEvent, pWaitQueue);
	}

	for (cnt = 0; cnt < (s32)nCount; cnt++) {
		pEvent = (OS_LINUX_EVENT_T *)lpHandles[cnt];
		if (pEvent->bSignaled) {
			if (!pEvent->bManualReset)
				pEvent->bSignaled = false;
			//local_irq_restore(flags);
			spin_unlock_irqrestore(&ac83xx_event_lock, flags);
			dwRet = (unsigned long)((unsigned long)WAIT_OBJECT_0 + (unsigned long)cnt);
			goto _timeout;
		}
	}

	if (dwMilliseconds == 0xFFFFFFFFUL) {
		// if use gdb debug , use wait_event_interruptible
		//local_irq_restore(flags);
		spin_unlock_irqrestore(&ac83xx_event_lock, flags);
		wait_event_interruptible(pWaitQueue->evt_q, pWaitQueue->bSignaled);
		//wait_event(pWaitQueue->wq, pWaitQueue->bSignaled);
	} else {
		//local_irq_restore(flags);
		spin_unlock_irqrestore(&ac83xx_event_lock, flags);
		if ((long)0 == wait_event_interruptible_timeout(pWaitQueue->evt_q, pWaitQueue->bSignaled, (long)((long)dwMilliseconds/(long)quantum_ms))) {
			dwRet = (unsigned long)WAIT_TIMEOUT;
			goto _timeout;
		}
	}

	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	for (cnt = 0; cnt < (s32)nCount; cnt++) {
		pEvent = (OS_LINUX_EVENT_T *)lpHandles[cnt];
		if (pEvent->bSignaled) {
			dwRet = (unsigned long)((unsigned long)WAIT_OBJECT_0 + (unsigned long)cnt);
			if (!pEvent->bManualReset)
				pEvent->bSignaled = false;
			break;
		} else if (pEvent->bWaitingForFree) {
			dwRet = (unsigned long)WAIT_FAILED;
			break;
		}
		else
		{
		  //do nothing
		}
	}
	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

_timeout:
	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	for (cnt = 0; cnt < (s32)nCount; cnt++) {
		pEvent = (OS_LINUX_EVENT_T *)lpHandles[cnt];
		//list_del(&pWaitQueue->wq_self);
		dettach_event_from_wq(pEvent, pWaitQueue);
		if ((pEvent->bWaitingForFree) && (list_empty(&pEvent->wq_head) != 0)) {
			list_del(&pEvent->ev_self);
			kfree(pEvent);
		}
	}
	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	kfree(pWaitQueue);

	return dwRet;
}
 
#else  //old api

typedef struct _OS_LINUX_EVENT_T {
     u32 magic;
     char name[OS_LINUX_EVENT_MAX_NAME_LEN];
     int refcnt;
     bool bSignaled;
     unsigned long data;
     bool bManualReset;
     bool bWaitingForFree;
     struct list_head wq_head;
     struct list_head ev_self;
     int mPID;
 } OS_LINUX_EVENT_T;

 
#define OS_LINUX_EVT_Q_MAGIC_NUMBER	(0xF1020305UL)
 
 struct _OS_LINUX_EVT_Q;
 
 typedef struct _OS_LINUX_EVT_Q_REF {
     struct list_head evt_q_ref;
     struct _OS_LINUX_EVT_Q *pWQ;
     bool bUsed;
 } OS_LINUX_EVT_Q_REF;
 
 typedef struct _OS_LINUX_EVT_Q {
     u32 magic;
     bool bSignaled;
     struct list_head evt_q_self;
     wait_queue_head_t evt_q;
     OS_LINUX_EVT_Q_REF refs[OS_LINUX_Q_MAX_EVENT];
 } OS_LINUX_EVT_Q;

 static const int quantum_ms = 1000 / HZ;

 /*-----------------------------------------
  Private function
  ------------------------------------------*/
 static int attach_event_to_wq(OS_LINUX_EVENT_T *pEvent, OS_LINUX_EVT_Q *pWQ)
 {
     int cnt;
     OS_LINUX_EVT_Q_REF *pRef;
 
     for (cnt = 0; cnt < OS_LINUX_Q_MAX_EVENT; cnt++) {
         pRef = &pWQ->refs[cnt];
         if (pRef->bUsed)
             continue;
 
         pRef->pWQ = pWQ;
         pRef->bUsed = true;
         list_add(&pRef->evt_q_ref, &pEvent->wq_head);
         return 0;
     }
 
     return -1;
 }
 
 static int dettach_event_from_wq(OS_LINUX_EVENT_T *pEvent, OS_LINUX_EVT_Q *pWQ)
 {
     OS_LINUX_EVT_Q_REF *pRef = NULL;
 
     if (list_empty(&pEvent->wq_head))
         return -1;
 
     list_for_each_entry(pRef, &pEvent->wq_head, evt_q_ref) {
         if (pRef && pRef->pWQ == pWQ) {
             list_del(&pRef->evt_q_ref);
             pRef->bUsed = false;
             return 0;
         }
     }
 
     return -2;
 }
 
 static int unknown_event(OS_LINUX_EVENT_T *pEvent)
 {
     if ((u32)pEvent < PAGE_OFFSET) {
         printk("[OSAL] pEvent error, 0x%x\n", (u32)pEvent);
         return -1;
     }
 
     if (pEvent->magic != OS_LINUX_EVENT_MAGIC_NUMBER)
     {
         printk("[OSAL] pEvent error\n");
         return -2;
     }
 
     return 0;
 }
 
 LIST_HEAD(g_event_list_head);

 /*-----------------------------------------
 Public function
 ------------------------------------------*/
 void* x_event_open(
     unsigned long dwDesiredAccess,
     bool bInheritHandle,
     const char* lpName
 )
 {
	OS_LINUX_EVENT_T *pEvent = NULL;
	unsigned long flags;

	if (lpName && !list_empty(&g_event_list_head)) {
		//local_irq_save(flags);
		spin_lock_irqsave(&ac83xx_event_lock, flags);
		list_for_each_entry(pEvent, &g_event_list_head, ev_self) {
			if (pEvent && 0 < strlen(pEvent->name) && 0 == strncmp(pEvent->name, lpName,OS_LINUX_EVENT_MAX_NAME_LEN-1)) {
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

 
 bool x_event_set( void* hEvent )
 {
	OS_LINUX_EVENT_T *pEvent = NULL; 
	OS_LINUX_EVT_Q *pWaitQueue = NULL;
	OS_LINUX_EVT_Q_REF *pWqRef = NULL;
	unsigned long flags;

	if (hEvent == NULL) {
		printk("[OSAL] Invalid arg \n");
		return FALSE;
	}
    pEvent = (OS_LINUX_EVENT_T *)hEvent;
    if (unknown_event(pEvent)){
    	printk("[OSAL] unknownEvent from sysSetEvent\n");
        return FALSE;
	}
	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	pEvent->bSignaled = true;

	if (!list_empty(&pEvent->wq_head)) {
		list_for_each_entry(pWqRef, &pEvent->wq_head, evt_q_ref) {
			if (!pWqRef)
				continue;

			pWaitQueue = pWqRef->pWQ;
			pWaitQueue->bSignaled = true;
			wake_up_all(&pWaitQueue->evt_q);
		}
	}

	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	return TRUE;
}
 
 bool x_event_set_data(void* hEvent, unsigned long dwData)
 {
	OS_LINUX_EVENT_T *pEvent = NULL;
	unsigned long flags;

	if (hEvent == NULL) {
		printk("[OSAL] Invalid arg \n");
		return FALSE;
	}
    pEvent = (OS_LINUX_EVENT_T *)hEvent;
	if (unknown_event(pEvent)){
    	printk("[OSAL] unknownEvent from sysSetEventData\n");
        return FALSE;
	}

	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	pEvent->data = dwData;
	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	return TRUE;
}
  
 unsigned long x_event_get_data(void* hEvent)
 {
	OS_LINUX_EVENT_T *pEvent = NULL;
	unsigned long flags;
	unsigned long dwResult;

	if (hEvent == NULL) {
		printk("[OSAL] Invalid arg \n");
		return FALSE;
	}
    pEvent = (OS_LINUX_EVENT_T *)hEvent;
	if (unknown_event(pEvent)){
    	printk("[OSAL] unknownEvent from sysGetEventData\n");
        return FALSE;
	}

	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	dwResult = pEvent->data;
	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	return dwResult;
}
 
 bool x_event_reset(void* hEvent)
 {
	OS_LINUX_EVENT_T *pEvent = NULL;
	unsigned long flags;

	if (hEvent == NULL) {
		printk("[OSAL] Invalid arg \n");
		return FALSE;
	}
    pEvent = (OS_LINUX_EVENT_T *)hEvent;

    if (unknown_event(pEvent)){
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
 
 
 bool x_event_destroy(void* hEvent)
 {
	OS_LINUX_EVENT_T *pEvent = NULL;
	OS_LINUX_EVT_Q *pWaitQueue = NULL;
	OS_LINUX_EVT_Q_REF *pWqRef = NULL;
	unsigned long flags;
	bool bNeedFree = false;

	if (hEvent == NULL) {
		printk("[OSAL] Invalid arg \n");
		return FALSE;
	}
    pEvent = (OS_LINUX_EVENT_T *)hEvent;
    if (unknown_event(pEvent)){
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
		list_for_each_entry(pWqRef, &pEvent->wq_head, evt_q_ref) {
			if (pWqRef) {
				pWaitQueue = pWqRef->pWQ;
				pWaitQueue->bSignaled = true;
				wake_up_all(&pWaitQueue->evt_q);
			}
		}
	}

	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	if (bNeedFree)
		kfree(pEvent);

	return TRUE;
}
 

 
 void* x_event_create(
     void* lpEventAttributes,
     bool bManualReset,
     bool bInitialState,
     const char* lpName
 )
 {
	OS_LINUX_EVENT_T *pEvent = NULL;
	unsigned long flags;
	struct list_head *pglobal_list = &g_event_list_head;

	if (lpName && !list_empty(pglobal_list)) {
		//local_irq_save(flags);
		spin_lock_irqsave(&ac83xx_event_lock, flags);
		list_for_each_entry(pEvent, pglobal_list, ev_self) {
			if (pEvent && 0 < strlen(pEvent->name) && 0 == strncmp(pEvent->name, lpName,OS_LINUX_EVENT_MAX_NAME_LEN-1)) {
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

	pEvent->magic = OS_LINUX_EVENT_MAGIC_NUMBER;
	pEvent->refcnt = 1;
	pEvent->data = 0;
	pEvent->bSignaled = bInitialState;
	pEvent->bManualReset = bManualReset ? true : false;
	pEvent->bWaitingForFree = false;
	INIT_LIST_HEAD(&pEvent->wq_head);
	pEvent->mPID = (int)current->pid;
	if (lpName) {
		strncpy(pEvent->name, lpName, OS_LINUX_EVENT_MAX_NAME_LEN-1);
		pEvent->name[OS_LINUX_EVENT_MAX_NAME_LEN-1] = 0;
	}

	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	list_add(&pEvent->ev_self, pglobal_list);
	//local_irq_restore(flags);
	spin_unlock_irqrestore(&ac83xx_event_lock, flags);

	return (void*)pEvent;
}
 

 unsigned long x_event_wait_for_objects(
     unsigned long nCount,
     const void* *lpHandles,
     bool bWaitAll,
     unsigned long dwMilliseconds
 )
 {
	OS_LINUX_EVT_Q *pWaitQueue = NULL;
	int cnt;
	OS_LINUX_EVENT_T *pEvent;
	unsigned long dwRet = WAIT_FAILED;
	unsigned long flags;

	if (nCount >= OS_LINUX_Q_MAX_EVENT) {
		//current->win32_errno = ERROR_INVALID_ARG; //task_struct has no member 'win32_errno'
		return WAIT_FAILED;
	}

    for (cnt = 0; cnt < (int)nCount; cnt++) {
        pEvent = (OS_LINUX_EVENT_T *)lpHandles[cnt];
        if (unknown_event(pEvent)) {
        	printk("[OSAL] unknownEvent from sysWaitForMultipleObjects\n");
            //current->win32_errno = ERROR_INVALID_ARG; //task_struct has no member 'win32_errno'
            return WAIT_FAILED;
        }
    }

	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	for (cnt = 0; cnt < nCount; cnt++) {
		pEvent = (OS_LINUX_EVENT_T *)lpHandles[cnt];
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

	pWaitQueue->magic = OS_LINUX_EVT_Q_MAGIC_NUMBER;
	pWaitQueue->bSignaled = false;
	init_waitqueue_head(&pWaitQueue->evt_q);

	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	for (cnt = 0; cnt < nCount; cnt++) {
		pEvent = (OS_LINUX_EVENT_T *)lpHandles[cnt];
		attach_event_to_wq(pEvent, pWaitQueue);
	}

	for (cnt = 0; cnt < nCount; cnt++) {
		pEvent = (OS_LINUX_EVENT_T *)lpHandles[cnt];
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
		wait_event_interruptible(pWaitQueue->evt_q, pWaitQueue->bSignaled);
		//wait_event(pWaitQueue->wq, pWaitQueue->bSignaled);
	} else {
		//local_irq_restore(flags);
		spin_unlock_irqrestore(&ac83xx_event_lock, flags);
		if (0 == wait_event_interruptible_timeout(pWaitQueue->evt_q, pWaitQueue->bSignaled, dwMilliseconds/quantum_ms)) {
			dwRet = WAIT_TIMEOUT;
			goto _timeout;
		}
	}

	//local_irq_save(flags);
	spin_lock_irqsave(&ac83xx_event_lock, flags);
	for (cnt = 0; cnt < nCount; cnt++) {
		pEvent = (OS_LINUX_EVENT_T *)lpHandles[cnt];
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
	for (cnt = 0; cnt < nCount; cnt++) {
		pEvent = (OS_LINUX_EVENT_T *)lpHandles[cnt];
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


#endif
 
 
 EXPORT_SYMBOL(x_event_create);
 EXPORT_SYMBOL(x_event_wait_for_objects);
 EXPORT_SYMBOL(x_event_open);
 EXPORT_SYMBOL(x_event_destroy);
 EXPORT_SYMBOL(x_event_set);
 EXPORT_SYMBOL(x_event_set_data);
 EXPORT_SYMBOL(x_event_get_data);
 EXPORT_SYMBOL(x_event_reset);


