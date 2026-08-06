#ifndef __DRV_WIN32_IF_H__
#define __DRV_WIN32_IF_H__
//#include "ac83xx_memory.h"

#ifdef __KERNEL__
#pragma message "compile in kernel mode"
#include <generated/atc_project.h>
#else
#ifndef __ARM2__
#pragma message "compile in user mode"
#else
#pragma message "compile in arm2"
#endif
#endif

#if defined(CONFIG_ATC_OS_android) && defined(CONFIG_ATC_PLATFORM_ac823x)

#pragma message "823x android"

#include "x_typedef.h"
#include <linux/ioctl.h>
#include <linux/types.h>

#define OSAL_DRV_MAGIC  'O'

/*
=============================================================
==============================S P L I T T E R======================
=============================================================
*/
/**
 * Create splitter Instance.
 * Parameter for this function:
 * dwOpenContext -- The return value of driver open.
 * dwCode			 -- DMX_IOCTL_SPT_CREATE
 * pBufIn			 -- NULL.
 * dwLenIn			 -- 0
 * pBufOut			 -- Handle of splitter created.
 * dwLenOut		 -- sizeof(HANDLE)
 * pdwActualOut  -- NULL
 *
 * Return: FALSE if failed for this function, or else return TRUE.
 * Notes: This should be call before enable splitter.
 */
typedef struct
{
	char  name[32];
	__u32 bManualReset;
	__u32 bInitialState;
	__u64 hEvent;
} OSAL_CREATE_EVENT_T;

typedef struct
{
	__u32  dwDesiredAccess;
	__u32  bInheritHandle;
	char   szName[32];
	__u64  hEvent;
} OSAL_OPEN_EVENT_T;

typedef struct
{
	__u32 nCount;
	void *lpHandles;
	__u32 dwMilliseconds;
	__u32 u4WaitResult;
} OSAL_WAIT_EVENT_T;

typedef struct
{
	__u64 hEvent;
	unsigned long ulData;
} OSAL_GSET_EVENT_DATA_T;

#define WIN32_IOCTL_CREATE_EVENT	_IOWR(OSAL_DRV_MAGIC, 1, OSAL_CREATE_EVENT_T)
#define WIN32_IOCTL_OPEN_EVENT	    _IOWR(OSAL_DRV_MAGIC, 2, OSAL_OPEN_EVENT_T)
#define WIN32_IOCTL_SET_EVENT		  _IOW(OSAL_DRV_MAGIC, 3, uintptr_t)
#define WIN32_IOCTL_RESET_EVENT		_IOW(OSAL_DRV_MAGIC, 4, uintptr_t)
#define WIN32_IOCTL_WAIT_EVENT		_IOWR(OSAL_DRV_MAGIC, 5, OSAL_WAIT_EVENT_T)
#define WIN32_IOCTL_DELETE_EVENT	_IOW(OSAL_DRV_MAGIC, 6, uintptr_t)
#define WIN32_IOCTL_SET_EVENT_DATA	_IOW(OSAL_DRV_MAGIC, 7, OSAL_GSET_EVENT_DATA_T)
#define WIN32_IOCTL_GET_EVENT_DATA	_IOWR(OSAL_DRV_MAGIC, 8, OSAL_GSET_EVENT_DATA_T)

enum {
    WIN32_NO_ERROR = 0,
    ERROR_ALREADY_EXISTS = -0x1111,
    ERROR_INVALID_ARG = -0x1112,
    ERROR_NOT_EXIST = -0x1113,
    WIN32_ERROR_OTHER = -1
};

#define WAIT_TIMEOUT    (0x00000102L)
#define WAIT_FAILED     0xFFFFFFFFUL
#define WAIT_OBJECT_0   (0)

#define INFINITE  (0xFFFFFFFFul)

#define PAGE_READWRITE  (0x100)

#define EVENT_ALL_ACCESS  (0)


typedef struct {
	int nCount;
	HANDLE *lpHandles;
	unsigned int dwMilliseconds;
} WIN_EVENT_WAIT_FOR_DATA;

typedef struct {
	DWORD dwDesiredAccess;
	BOOL bInheritHandle;
	char szName[32];
} WIN_EVENT_OPEN;

typedef struct {
	HANDLE handle;
	DWORD dwData;
} WIN_EVENT_DATA;

#else  //for 8317 linux/8317 m
#pragma message "83xx android or linux"

#include "types.h"
#define WIN32_IOCTL_CREATE_EVENT	(0x1000) //4096
#define WIN32_IOCTL_SET_EVENT		(0x1001)
#define WIN32_IOCTL_RESET_EVENT		(0x1002)
#define WIN32_IOCTL_WAIT_EVENT		(0x1003)
#define WIN32_IOCTL_DELETE_EVENT	(0x1004) //4100
#define WIN32_IOCTL_SET_EVENT_DATA	(0x1005)
#define WIN32_IOCTL_GET_EVENT_DATA	(0x1006)
#define WIN32_IOCTL_OPEN_EVENT	    (0x1007)
#define WIN32_IOCTL_GET_LAST_ERRORS (0x1010)
#define WIN32_IOCTL_SET_LAST_ERRORS (0x1011)
#define WIN32_IOCTL_PRINT_LOG       (0x1020)//yunjie add
#define WIN_EVENT_FD_OFFSET	(0x1000)
enum {
    WIN32_NO_ERROR = 0,
    ERROR_ALREADY_EXISTS = -0x1111,
    ERROR_INVALID_ARG = -0x1112,
    ERROR_NOT_EXIST = -0x1113,
    WIN32_ERROR_OTHER = -1
};
#define WAIT_TIMEOUT    (0x00000102L)
#define WAIT_FAILED     0xFFFFFFFFUL
#define WAIT_OBJECT_0   (0)
#define INFINITE  (0xFFFFFFFFul)
#define PAGE_READWRITE  (0x100)
#define EVENT_ALL_ACCESS  (0)
typedef struct {
	int nCount;
	HANDLE *lpHandles;
	unsigned int dwMilliseconds;
} WIN_EVENT_WAIT_FOR_DATA;
typedef struct {
	DWORD dwDesiredAccess;
	BOOL bInheritHandle;
	char szName[32];
} WIN_EVENT_OPEN;
typedef struct {
	HANDLE handle;
	DWORD dwData;
} WIN_EVENT_DATA;

#endif
#endif

