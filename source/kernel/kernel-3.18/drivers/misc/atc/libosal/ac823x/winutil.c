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
#include "inc/winutil.h"

#if 1
#ifdef __linux__
#include "inc/windows.h"
#endif
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/err.h>
#endif


typedef struct {
	char pName[32];
	bool use ;
	u32 ref_cnt;
	//EV_GRP_EVENT_T event;
} win_event;

typedef struct os_ev_grp_light
{
    struct os_ev_grp_light *previous;
    struct os_ev_grp_light *next;
    wait_queue_head_t wq;
    EV_GRP_EVENT_T e_events;
    s16 i2_refcount;
    char s_name[16 + 1];
} OS_EV_GRP_LIGHT_T;

typedef struct {
	char pName[32];
	u32 hSema;
	bool use;
	int count ;
} win_sema ;

#define MAX_WIN_SEMA (16)
static win_sema _g_sema_all[MAX_WIN_SEMA];
static u32 _g_sema_array_lock = (u32)NULL ;


#if 0
#define debug_printk  printk
#define FUNC_LOG	printk("enter %s\n", __FUNCTION__ )
#else
#define debug_printk(x...)
#define FUNC_LOG
#endif

//return -1: not found
static int find_sema_byname(char *lpName)
{
	int n = 0 ;

	if ( lpName == NULL )
		return -1;

	for ( n=0 ; n < MAX_WIN_SEMA ; n++)
	{
		if ( _g_sema_all[n].use  && strcmp( _g_sema_all[n].pName, lpName) == 0 )
		{
			break;
		}
	}

	if ( n == MAX_WIN_SEMA )
		n = -1;

	return n;
}

static int get_free_sema(void)
{
	int n = 0 ;
	for ( ; n < MAX_WIN_SEMA ; n++)
	{
		if ( _g_sema_all[n].use == FALSE )
		{
			break;
		}
	}

	if ( n == MAX_WIN_SEMA )
		n = -1;

	return n;
}

bool InitWin32Envrionment(void)
{
	bool ret = TRUE;

	if ( OSR_OK != x_sema_create(&_g_sema_array_lock , X_SEMA_TYPE_MUTEX , X_SEMA_STATE_UNLOCK) )
	{
		ret = FALSE ;
		goto out;
	}
	memset( _g_sema_all , 0 , sizeof(_g_sema_all) );

out:
	return ret;
}

bool DeinitWin32Envrionment(void)
{
	x_sema_delete( _g_sema_array_lock );
	return TRUE;
}

void* CreateSemaphore(
	void* lpSemaphoreAttributes,
	s64 lInitialCount,
	s64 lMaximumCount,
	char* lpName
)
{
	u32 hsema = (u32)NULL ;
	s32 ret = 0 ;
	int n = 0 ;

	//debug_printk("enter CreateSemaphore lpname=%s\n", lpName);
	x_sema_lock( (u32)_g_sema_array_lock, X_SEMA_OPTION_WAIT);

	if ( lpName != NULL)
	{
		n = find_sema_byname((char*)lpName);
		//debug_printk("find sema n=%d\n", n);
		if ( n != -1 )
		{
			//current->win32_errno = ERROR_ALREADY_EXISTS; //task_struct has no member 'win32_errno'
			hsema = _g_sema_all[n].hSema;
			_g_sema_all[n].count++;
			goto _ret;
		}
	}

	n = get_free_sema();
	if (  n == -1 )
	{
		//current->win32_errno = -ENOMEM; //task_struct has no member 'win32_errno'
		hsema = (u32)NULL;
		goto _ret;
	}

	ret = x_sema_create(&hsema, X_SEMA_TYPE_MUTEX , X_SEMA_STATE_UNLOCK);

	if ( ret == OSR_OK )
	{
		_g_sema_all[n].use = TRUE;
		_g_sema_all[n].hSema = hsema ;
		_g_sema_all[n].count = 1 ;
		if ( lpName != NULL )
		{
			strncpy( _g_sema_all[n].pName, lpName, sizeof(_g_sema_all[n].pName)-1 );
		}
		else
		{
			_g_sema_all[n].pName[0] = 0;
		}

		//current->win32_errno = NO_ERROR; //task_struct has no member 'win32_errno'
	}
	else
	{
		//current->win32_errno = ERROR_OTHER; //task_struct has no member 'win32_errno'
		hsema = (u32)NULL;
	}

_ret:
	x_sema_unlock( (u32)_g_sema_array_lock );
	return (void*)hsema ;
}

void DeleteSemaphore(void* hSemaphore)
{
	int n = 0 ;

	x_sema_lock( (u32)_g_sema_array_lock, X_SEMA_OPTION_WAIT);
	for ( ; n < MAX_WIN_SEMA ; n++)
	{
		if ( _g_sema_all[n].use && _g_sema_all[n].hSema == (u32)hSemaphore )
		{
			if ( _g_sema_all[n].count == 1 )
			{
				//current->win32_errno = NO_ERROR; //task_struct has no member 'win32_errno'
				_g_sema_all[n].use = FALSE;
				_g_sema_all[n].pName[0] = '\0';
				_g_sema_all[n].hSema = 0;
				_g_sema_all[n].count = 0 ;
				x_sema_delete( (u32) hSemaphore);
			} else
			{
				_g_sema_all[n].count--;
			}
			break;
		}
	}
	x_sema_unlock( (u32)_g_sema_array_lock );
}

void GetSemaphore(void* hSemaphore)
{
	#if 0
	x_sema_lock( (u32) hSemaphore, X_SEMA_OPTION_WAIT);
	#else
	int n;
	x_sema_lock( (u32)_g_sema_array_lock, X_SEMA_OPTION_WAIT);

	n = 0 ;
	for ( ; n < MAX_WIN_SEMA ; n++)
	{
		if ( _g_sema_all[n].use && _g_sema_all[n].hSema == (u32)hSemaphore )
		{
			break;
		}
	}

	x_sema_unlock( (u32)_g_sema_array_lock );

	if ( n < MAX_WIN_SEMA )
		x_sema_lock( (u32) hSemaphore, X_SEMA_OPTION_WAIT);

	#endif

	return ;
}
bool ReleaseSemaphore(
	void* hSemaphore,
	s64 lReleaseCount,
	u64* lpPreviousCount
)
{
	int n;

	x_sema_lock( (u32)_g_sema_array_lock, X_SEMA_OPTION_WAIT);

	n = 0 ;
	for ( ; n < MAX_WIN_SEMA ; n++)
	{
		if ( _g_sema_all[n].use && _g_sema_all[n].hSema == (u32)hSemaphore )
		{
			break;
		}
	}

	x_sema_unlock( (u32)_g_sema_array_lock );

	if ( n < MAX_WIN_SEMA )
		x_sema_unlock(  (u32) hSemaphore );

	
	return TRUE;
}

u32 GetLastError(void)
{
	FUNC_LOG;
	//return current->win32_errno; //task_struct has no member 'win32_errno'
	return -1;
}

void SetLastError(u32 dwErrCode)
{
   // current->win32_errno = dwErrCode; //task_struct has no member 'win32_errno'
   return;
}

extern void* sysOpenEvent(
	u32 dwDesiredAccess,
	bool bInheritHandle,
	char* lpName
);

void* OpenEvent(
	u32 dwDesiredAccess,
	bool bInheritHandle,
	char* lpName
)
{
	return sysOpenEvent(dwDesiredAccess, bInheritHandle, lpName);
}

extern bool sysSetEvent(void* hEvent);

bool SetEvent( void* hEvent )
{
	return sysSetEvent(hEvent);
}

extern bool sysSetEventData(void* hEvent, u32 dwData);

bool SetEventData(void* hEvent, u32 dwData)
{
	return sysSetEventData(hEvent, dwData);
}

extern u32 sysGetEventData(void* hEvent);

u32 GetEventData(void* hEvent)
{
	return sysGetEventData(hEvent);
}

extern bool sysResetEvent(void* hEvent);

bool ResetEvent(void* hEvent)
{
	return sysResetEvent(hEvent);
}

extern bool sysDestroyEvent(void* hEvent);

bool DestroyEvent(void* hEvent)
{
	return sysDestroyEvent(hEvent);
}

extern void* sysCreateEvent(
    void* lpEventAttributes,
    bool bManualReset,
    bool bInitialState,
    char* lpName
);

void* CreateEvent(
    void* lpEventAttributes,
    bool bManualReset,
    bool bInitialState,
    char* lpName
)
{
	return sysCreateEvent(lpEventAttributes, bManualReset, bInitialState, lpName);
}

extern u32 sysWaitForMultipleObjects(
	u32 nCount,
	const void* *lpHandles,
	bool bWaitAll,
	u32 dwMilliseconds
);

u32 WaitForMultipleObjects(
	u32 nCount,
	const void* *lpHandles,
	bool bWaitAll,
	u32 dwMilliseconds
)
{
	return sysWaitForMultipleObjects(nCount, lpHandles, bWaitAll, dwMilliseconds);
}

#if 1
#define INVALID_HANDLE_VALUE     (-1)
void* CreateFile(
  char* lpFileName,
  u32 dwDesiredAccess,
  u32 dwShareMode,
  void* lpSecurityAttributes,
  u32 dwCreationDispostion,
  u32 dwFlagsAndAttributes,
  void* hTemplateFile
)
{
    struct file *pfile = NULL;
	int flags = 0 ;
	void* ret ;
    struct inode *pinode = NULL;
    loff_t t_cur_pos = 0;

	switch (dwDesiredAccess)
	{
	case GENERIC_READ:
		flags = O_RDONLY;
		break;
	case GENERIC_WRITE:
		flags = O_WRONLY;
		break;
	case GENERIC_RW :
		flags = O_RDWR;
		break;
	default:
		break;
	}

    if (dwCreationDispostion == CREATE_ALWAYS)
    {
        flags |= O_TRUNC | O_CREAT;
    }

	pfile = filp_open(lpFileName, flags, 0666);
	if (IS_ERR(pfile))
	{
        int errorno = PTR_ERR(pfile);
        printk( "[OSAL] %s line %d filp_open %s error, errorno: %d\r\n",
            __FUNCTION__, __LINE__, lpFileName, errorno);
        ret = (void*)INVALID_HANDLE_VALUE;
        return ret;
	}
	else
    {
		ret = (void*)pfile;
        //printk( "%s line %d -- success, file name: %s\r\n",
           // __FUNCTION__, __LINE__, lpFileName);
	}

    t_cur_pos = vfs_llseek(pfile, (loff_t)0, SEEK_SET);
    t_cur_pos = vfs_llseek(pfile, (loff_t)t_cur_pos, SEEK_SET);
    pinode = pfile->f_dentry->d_inode;

    //printk("*%s line %d --> create file %s success, pfile: 0x%lx, curPos: %lld, f_pos: %lld\r\n",
       //__FUNCTION__, __LINE__, lpFileName, (UINT32)pfile, t_cur_pos, pfile->f_pos);

	return ret;
}

bool CloseHandle(void* hObject)
{
    struct file *pfile = (struct file *)hObject;

    if (IS_ERR(pfile))
    {
        return FALSE;
    }
	else
	{
		filp_close(pfile, NULL);
        return TRUE;
	}
}
EXPORT_SYMBOL(CreateFile);
EXPORT_SYMBOL(CloseHandle);
#endif

EXPORT_SYMBOL(CreateEvent);
EXPORT_SYMBOL(WaitForMultipleObjects);
EXPORT_SYMBOL(OpenEvent);
EXPORT_SYMBOL(ResetEvent);
EXPORT_SYMBOL(SetEvent);
EXPORT_SYMBOL(SetEventData);
EXPORT_SYMBOL(GetEventData);
EXPORT_SYMBOL(DestroyEvent);

EXPORT_SYMBOL(GetSemaphore);
EXPORT_SYMBOL(ReleaseSemaphore);
EXPORT_SYMBOL(CreateSemaphore);
EXPORT_SYMBOL(DeleteSemaphore);
EXPORT_SYMBOL(GetLastError);
EXPORT_SYMBOL(SetLastError);

#if 0
LPVOID AllocPhysMem(
  u32 cbSize,
  u32 fdwProtect,
  u32 dwAlignmentMask,
  u32 dwFlags,
  PULONG pPhysicalAddress
)
{
	LPVOID *pRet = dma_alloc_writecombine(NULL, PAGE_ALIGN(cbSize),
			(dma_addr_t *)pPhysicalAddress, GFP_KERNEL);

	return pRet;

}


bool FreePhysMem(
  LPVOID lpvAddress
)
{
	dma_free_coherent( NULL,  );
	return TRUE;
}
#endif


