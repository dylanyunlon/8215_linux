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


#ifndef DUALARM_H
#define DUALARM_H

#include "windev.h"
#include <linux/types.h>


#define    FILE_DEVICE_DUALARM 0x0F73102D
#define    DUALARM_IOC_SENDMESSAGE               CTL_CODE(FILE_DEVICE_DUALARM, 0x0101, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define    DUALARM_IOC_GETMESSAGE                 CTL_CODE(FILE_DEVICE_DUALARM, 0x0102, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define    MESSAGE_LAST_SOURCE_READY 0x1
#define    MESSAGE_ANIMATION_STOP 0x2
#define    GET_BOOTLOGO_BUFFER                   CTL_CODE(FILE_DEVICE_DUALARM, 0x0103, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _LOGO_BUF_INFO_T
{
    void   *pvVirAddr;
    __u32 u4BufPhyAdr;
    __u32 u4BufSz;
} LOGO_BUF_INFO_T;

typedef struct {
	unsigned int value1;
	unsigned int value2;
	unsigned int value3;
	unsigned int value4;
}DUALARM_PARAM;

#endif  
