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

#ifndef __WINUTIL_H__
#define __WINUTIL_H__

#include "x_typedef.h"
#include "drv_win32_if.h"

#if 1
enum {
	GENERIC_READ = 0x1 ,
	GENERIC_WRITE = 0x2,
	GENERIC_RW = GENERIC_READ | GENERIC_WRITE,
	OPEN_EXISTING ,
	FILE_SHARE_READ,
	FILE_SHARE_WRITE,
	FILE_ATTRIBUTE_NORMAL,
	FILE_BEGIN,
	CREATE_ALWAYS,
};
#endif

typedef struct {
	char name[32];
	bool bManualReset;
	bool bInitialState;
} EVENT_PARA_T;

#endif

