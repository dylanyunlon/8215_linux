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


#ifndef ANIMATION_H
#define ANIMATION_H

#include "../inc/windev.h"
#include <linux/types.h>

//How define file of Boot-Ani
#define    FILE_DEVICE_BOOTANI 0x0F73107F
#define    BOOTANI_IOC_SENDMESSAGE              CTL_CODE(FILE_DEVICE_BOOTANI, 0x0101, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define    BOOTANI_IOC_GETMESSAGE               CTL_CODE(FILE_DEVICE_BOOTANI, 0x0102, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define    BOOTANI_IOC_RLSREVMEM                CTL_CODE(FILE_DEVICE_BOOTANI, 0x0103, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define    BOOTANI_IOC_GETOSDPHY                CTL_CODE(FILE_DEVICE_BOOTANI, 0x0104, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define    BOOTANI_IOC_SLEEP                    CTL_CODE(FILE_DEVICE_BOOTANI, 0x0105, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define    BOOTANI_IOC_WAKEUP                   CTL_CODE(FILE_DEVICE_BOOTANI, 0x0106, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define    BOOTANI_IOC_ENABLE_BACKCARUI		  CTL_CODE(FILE_DEVICE_BOOTANI, 0x0107, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define    BOOTANI_IOC_DISABLE_BACKCARUI           CTL_CODE(FILE_DEVICE_BOOTANI, 0x0108, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define    BOOTANI_IOC_GETVBAPHY                CTL_CODE(FILE_DEVICE_BOOTANI, 0x0109, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define    BOOTANI_IOC_SENDAWTKMESSAGE          CTL_CODE(FILE_DEVICE_BOOTANI, 0x0110, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define    BOOTANI_IOC_STOPAWTKDISPLAY          CTL_CODE(FILE_DEVICE_BOOTANI, 0x0111, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define    BOOTANI_IOC_STARTAWTKDISPLAY         CTL_CODE(FILE_DEVICE_BOOTANI, 0x0112, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define    BOOTANI_IOC_GETVDECSTATUS         CTL_CODE(FILE_DEVICE_BOOTANI, 0x0113, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define    MESSAGE_LAST_SOURCE_READY 0x1
#define    MESSAGE_ANIMATION_STOP 0x2


typedef struct _LOGO_BUF_INFO_T
{
    void   *pvVirAddr;
    __u32 u4BufPhyAdr;
    __u32 u4BufSz;
	__u32 u4Width;
	__u32 u4Height;
} LOGO_BUF_INFO_T;

typedef struct {
	unsigned int value1;
	unsigned int value2;
	unsigned int value3;
	unsigned int value4;
}DUALARM_PARAM;

enum VDEC_STATUS {
	VDEC_STATUS_FREE = 0,
	VDEC_STATUS_USED
};

#endif  

