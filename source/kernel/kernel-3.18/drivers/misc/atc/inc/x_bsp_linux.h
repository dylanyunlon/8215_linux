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


#ifndef _X_BSP_LINUX_H_
#define _X_BSP_LINUX_H_

#include "x_timer.h"

#define DEV_BSP "/dev/bsp"

/* ioctl id */
#define IOCTL_BSP_HAL_GET_TIME       1
#define IOCTL_BSP_HAL_GET_DELTA_TIME 2
#define IOCTL_BSP_GET_IC_FUNCTION	3
#define IOCTL_BSP_GET_IC_VERSION	4

/* error id */
#define ERR_BSP_HAL_GET_TIME            1
#define ERR_BSP_HAL_GET_DELTA_TIME_IN   2
#define ERR_BSP_HAL_GET_DELTA_TIME_OUT  3
#define ERR_BSP_GET_IC_FUNCTION_IN	4
#define ERR_BSP_GET_IC_FUNCTION_OUT	5
#define ERR_BSP_GET_IC_FUNCTION_PNT_NULL	6
#define ERR_BSP_GET_IC_VERSION_IN	7
#define ERR_BSP_GET_IC_VERSION_OUT	8
#define ERR_BSP_GET_IC_VERSION_PNT_NULL	9

/* struct */
typedef struct
{
    struct 
	{
        HAL_TIME_T tOlder;
        HAL_TIME_T tNewer;
    }in;
	struct
	{
        HAL_TIME_T tResult;
	}out;
}BSP_HAL_GET_DELTA_TIME_T;

typedef BOOL (*_CallBackfunc)(UINT32 u4FuncId);
typedef CHAR *(*_GetIcVerFunc)(void);

typedef struct
{
	UINT32	FuncID;
	BOOL	ret_val;
	_CallBackfunc CallBackfunc;
	_GetIcVerFunc GetIcVerFunc;
	
}BSP_GET_FUNCTION_ID;

#define VERSION_BUF		0x100

#endif // _BSP_LINUX_H_
