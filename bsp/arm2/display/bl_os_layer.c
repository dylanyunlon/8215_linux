/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */
#define   __KERNEL__
#include "x_types.h"
#include "x_os.h"

#ifndef  __KERNEL__
#include<stdarg.h>
#include<stdlib.h>
#endif

#include "backcar_cfg.h"

#include <generated/atc_project.h>


//#ifdef WIN32

#define EVENT_GROUP_TAG		0x12345601
#define INFINITE            (0xFFFFFFFFul)
#define WAIT_OBJECT_0       (0)
/*
#define EVENT_GROUP_CHECK_TAG(p)		do { \
	if (!p || ((X_EVENT_GROUP *)p)->eg_tag != EVENT_GROUP_TAG) \
		return OSR_INV_ARG; \
} while (0)

#define MAX_ARRAY_SIZE	32

typedef struct {
	UINT32				eg_tag;
	INT32				event_num;
	EV_GRP_EVENT_T		event_req;
	HANDLE				event_array[MAX_ARRAY_SIZE];
	CHAR				event_name[MAX_ARRAY_SIZE];
	CRITICAL_SECTION	cs;
} X_EVENT_GROUP;

*/
unsigned long jiffies = 0;
	
unsigned long msecs_to_jiffies(int seconds)
{
	return 0;
}

void* x_event_create(void* lpEventAttributes, bool bManualReset, bool bInitialState, const char* lpName)
{
	return 0xFFF000;
}

bool x_event_set( void* hEvent )
{
	return TRUE;
}

unsigned long x_event_wait_for_objects(unsigned long nCount, const void ** lpHandles, bool bWaitAll, unsigned long dwMilliseconds)
{
	return 0;
}

s32 x_ev_group_create (u32         *ph_hdl,
						const CHAR       *ps_name,
						EV_GRP_EVENT_T   e_init_events)
{
    return OSR_OK;
}

s32 x_ev_group_delete (u32  h_hdl)
{
    return OSR_OK;
}

s32 x_ev_group_set_event (u32             h_hdl,
                                   EV_GRP_EVENT_T       e_events,
                                   EV_GRP_OPERATION_T   e_operation)
{
    return OSR_OK;
}

void x_thread_delay (u32  u4Delay)
{
    return;
}

s32 x_ev_group_wait_event_timeout(u32           h_hdl,
                                           EV_GRP_EVENT_T     e_events_req,
                                           EV_GRP_EVENT_T     *pe_events_got,
                                           EV_GRP_OPERATION_T e_operation,
                                           UINT32             ui4_time)
{
    return OSR_OK;
}

s32 x_ev_group_wait_event (u32            h_hdl,
                                    EV_GRP_EVENT_T      e_events_req,
                                    EV_GRP_EVENT_T      *pe_events_got,
                                    EV_GRP_OPERATION_T  e_operation)
{
    return x_ev_group_wait_event_timeout(h_hdl,
										e_events_req,
										pe_events_got,
										e_operation,
										INFINITE);
}

s32 x_ev_group_get_info (u32          h_hdl,
                                  EV_GRP_EVENT_T    *pe_cur_events,
                                  u8             *pui1_num_thread_waiting,
                                  char              *ps_ev_group_name,
                                  char              *ps_first_wait_thread)
{
    return OSR_OK;
}

/*
INT32 x_reg_isr()
{

 return OSR_OK;
}
*/

s32 x_sema_create(u32 *ph_sema_hdl,
					SEMA_TYPE_T  e_types,
					u32       ui4_init_value)
{
    return OSR_OK;
}

s32 x_sema_delete(u32 h_sema_hdl)
{
    return OSR_OK;
}

s32 x_sema_force_delete(u32 h_sema_hdl)
{
    return OSR_OK;
}

s32 x_sema_lock(u32 h_sema_hdl,
					SEMA_OPTION_T e_options)
{
    return OSR_OK;
}

void InterruptDone(
  u32 idInt
)
{
    return;
}

s32 x_sema_lock_timeout(u32 h_sema_hdl,
							u32 ui4_time)
{
    return OSR_OK;
}

s32 x_sema_unlock(u32 h_sema_hdl)
{
    return OSR_OK;
}

bool CloseHandle(
  void* hObject
)
{
    return TRUE;
}

/*
HLOCAL LocalAlloc(
  UINT uFlags,
  UINT uBytes
)
{

  return NULL;
}
*/

VOID CacheSync(
  int flags 
)
{
    return;
}

VOID CacheFlush(UINT32 u4Start, UINT32 u4Len)
{
	extern Flush_Cache(UINT32 u4Start, UINT32 u4Len);
	Flush_Cache(u4Start, u4Len);
    return;
}

void CacheRangeFlush(
  LPVOID pAddr,
  DWORD dwLength,
  DWORD dwFlags
)
{
    return;
}
#if  0
void Sleep(
  DWORD dwMilliseconds
)
{

	return;
}
#endif

__crt_unrecoverable_error()
{
    return  0;
}

DWORD WaitForSingleObject(
  HANDLE hHandle,
  DWORD dwMilliseconds
)
{
    return WAIT_OBJECT_0;
}
/*
HANDLE CreateThread(
  LPSECURITY_ATTRIBUTES lpsa,
  DWORD cbStack,
  LPTHREAD_START_ROUTINE lpStartAddr,
  LPVOID lpvThreadParam,
  DWORD fdwCreate,
  LPDWORD lpIDThread
)
{


    return 0xFFF000;
}
*/

HANDLE CreateEvent(
  LPSECURITY_ATTRIBUTES lpEventAttributes,
  BOOL bManualReset,
  BOOL bInitialState,
  LPTSTR lpName
)
{
    return 0xFFF000;
}
/*
BOOL QueryPerformanceCounter(
  LARGE_INTEGER* lpPerformanceCount
)
{

  return TRUE;
}

BOOL QueryPerformanceFrequency(
  LARGE_INTEGER* lpFrequency
)
{
  return TRUE;
}
*/

BOOL ReleaseMutex(
  HANDLE hMutex
)
{
    return TRUE;
}

void* CreateMutex(
  LPSECURITY_ATTRIBUTES lpMutexAttributes,
  bool bInitialOwner,
  LPCTSTR lpName
)
{
    return 0xFFF000;
}

bool KernelIoControl(
  DWORD dwIoControlCode,
  LPVOID lpInBuf,
  DWORD nInBufSize,
  LPVOID lpOutBuf,
  DWORD nOutBufSize,
  LPDWORD lpBytesReturned
)
{
    return TRUE;
}

VOID InterruptMask(
  DWORD idInt,
  BOOL fDisable
)
{
    return;
}

VOID InterruptDisable(
  DWORD idInt
)
{
    return;
}

extern unsigned int fbm_base;
extern unsigned int fbm_size;
#define PA_START (fbm_base + fbm_size - 0xF8000)


ULONG  dwCurrentPoint = 0;

void * x_mem_alloc(SIZE_T u4Size)
{
    LPVOID  lpVirAddr = NULL;
    DWORD   dwOffset = 0;
    DWORD   dwRemainSize = 0;

    lpVirAddr = (LPVOID)(ARM1PHY2ARM2UCV(PA_START) + (dwCurrentPoint - PA_START));
    dwRemainSize = (u4Size & 0x3)?(4 -(u4Size & 0x3)):0;
    dwCurrentPoint += u4Size + dwRemainSize;
    Printf("call x_mem_alloc\r\n");

    return lpVirAddr;
}

void x_mem_free(void *pv_mem_block)
{
    return;
}

void* x_mem_alloc_ret_phy_addr (SIZE_T  u4Size, UINT32 *pu4PhyAddr )
{

   LPVOID  lpVirAddr = NULL;
    DWORD   dwOffset = 0;
    DWORD   dwRemainSize = 0;

    lpVirAddr = (LPVOID)(ARM1PHY2ARM2UCV(PA_START) + (dwCurrentPoint - PA_START));
    *pu4PhyAddr =  dwCurrentPoint;
    dwRemainSize = (u4Size & 0x3)?(4 -(u4Size & 0x3)):0;
    dwCurrentPoint += u4Size + dwRemainSize;

    Printf("[arm2] call x_mem_alloc  %x \r\n", *pu4PhyAddr);

    return lpVirAddr;
}



INT32 __arm_ioremap(UINT32 u4Pa, UINT32 u4Size, UINT32  u4Type)
{
    LPVOID  lpVirAddr;

    lpVirAddr = (LPVOID)(ARM1PHY2ARM2UCV(PA_START) + (u4Pa - PA_START));

    return (lpVirAddr);
}


void  x_mem_free_ret_phy_addr (void*  pv_mem_block )
{
     return;
}

CRIT_STATE_T x_crit_start(void)
{
    return 0;
}
void x_crit_end(CRIT_STATE_T handle)
{
    return;
}

UINT32 x_va_to_pa(UINT32 u4VirAdd)
{
    return ARM2UCV2ARM1PHY(u4VirAdd);
}

BOOL BIM_ClearIrq(UINT32 u4Vector)
{
    return  TRUE;
}

HANDLE_T    MutexVDP;

BOOL MMInit()
{
    dwCurrentPoint = PA_START;

    return TRUE;
}

LPVOID AllocPhysMem(
  DWORD dwSize,
  DWORD fdwProtect,
  DWORD dwAlignmentMask,
  DWORD dwFlags,
  PULONG pPhysicalAddress
)
{
    LPVOID  lpVirAddr = NULL;
    DWORD   dwOffset = 0;
    DWORD   dwRemainSize = 0;

    if (dwCurrentPoint & dwAlignmentMask)
    {
        dwCurrentPoint = (dwCurrentPoint + dwAlignmentMask) & (~dwAlignmentMask);
    }

    *pPhysicalAddress = dwCurrentPoint ;
    lpVirAddr =(LPVOID)(ARM1PHY2ARM2UCV(PA_START) + (dwCurrentPoint - PA_START));
    dwRemainSize = (dwSize & 0x3)?(4 -(dwSize & 0x3)):0;
    dwCurrentPoint += dwSize + dwRemainSize;

#if 0
    Printf("dwSize = %d, dwRemainSize=%d, dwCurrentPoint = 0x%X, lpVirAddr=0X%x\r\n",dwSize,
		dwRemainSize,dwCurrentPoint,lpVirAddr);
#endif

    return lpVirAddr;
}

BOOL FreePhysMem(
  LPVOID lpvAddress
)
{
    return TRUE;
}

//#endif

BOOL EventModify(HANDLE h)
{
    return TRUE;
}

VOID panic(const char* fmt, ...)
{
    return;
}

static void *mymemset(void *s, int c, int n)
{
    char *ss = s;
    while (n--)
    {
        *ss++ = c;
    }

    return s;
}

void * __memzero(void* s, int n)
{
    return mymemset(s, 0, n);
}

VOID SetEvent(HANDLE pEvent)
{
    return;
}

DWORD WaitForMultipleObjects(
	DWORD nCount,
	const HANDLE *lpHandles,
	BOOL bWaitAll,
	DWORD dwMilliseconds)
{
    return 0;
}

void* __kmalloc(int size, INT8 flags)
{
    return NULL;
}

void kfree(const void *x)
{
    return;
}


#ifdef CONFIG_ATC_OS_android
//UINT IrtDma_Rotate(VOID * pCtrlPara)
//{
	//return 0;
//}
#endif

unsigned long __copy_to_user(void *to, void *from,unsigned long n)
{
   memcpy(to,from,n);
   return 0;
}
unsigned long __copy_from_user(void *to, void *from,unsigned long n)
{
   memcpy(to,from,n);
   return 0;
}

void *vmalloc (int len)
{
	return NULL;
}
void vfree (void *ptr)
{
	return NULL;
}
void ac83xx_mask_ack_bim_irq(UINT32 irq)
{
    return;
}
void _raw_spin_unlock_irqrestore(void)
{
    return;
}
void _raw_spin_lock_irqsave(void)
{
    return;
}

void __raw_spin_lock_init (void)
{
    return;
}
void udelay (unsigned long usec)
{
	UINT64 tmo, tmp;
  
  tmp = usec * 27;
  tmo = GetHiTimerTick() + tmp;
  while(GetHiTimerTick() < tmo);

}




#ifndef memcmp
int memcmp(const void * cs, const void * ct, unsigned int count)
{
	const unsigned char *su1, *su2;
	int res = 0;

	for( su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}
#endif

#ifndef vVDec_ClearDispUse
VOID vVDec_ClearDispUse(UINT32 u4PYAddr, UINT32 u4PCAddr)
{
}
#endif

