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

#ifndef YBR_VGA_DRV_IF_H__
#define YBR_VGA_DRV_IF_H__

#include "windev.h"
#include "x_typedef.h"
#include "ybr_vga_oal.h"
#include <linux/types.h>

/**
Version Control
*/
#define YBR_VGA_MOD_NAME    "YBRVGA"
#define YBR_VGA_VER_MAIN    1
#define YBR_VGA_VER_MINOR   0
#define YBR_VGA_VER_REV     0

/**
    IOCTL
*/
#define FILE_DEVICE_YBR_VGA             ((0x00000000)|('Y'<<16) |('B'<<8)|('R'))
#define IOCTL_YBR_VGA_INIT              CTL_CODE(FILE_DEVICE_YBR_VGA, 0x0101, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_YBR_VGA_CONFIG            CTL_CODE(FILE_DEVICE_YBR_VGA, 0x0102, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_YBR_VGA_START             CTL_CODE(FILE_DEVICE_YBR_VGA, 0x0103, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_YBR_VGA_STOP              CTL_CODE(FILE_DEVICE_YBR_VGA, 0x0104, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_YBR_VGA_GET_VIDEO_INFO    CTL_CODE(FILE_DEVICE_YBR_VGA, 0x0105, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_YBR_VGA_AUTO                  CTL_CODE(FILE_DEVICE_YBR_VGA, 0x0106, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_YBR_VGA_GET_SINGAL_STATUS  CTL_CODE(FILE_DEVICE_YBR_VGA, 0x0107, METHOD_BUFFERED, FILE_ANY_ACCESS)
/**
EVENT Define
*/
#define EVT_YBR_VGA_SIG_STATE    _T("YBR_VGA_SIGNAL_STATE") 
#define EVT_YBR_VGA_SIG_OFF         0x00000001
#define EVT_YBR_VGA_SIG_ON          0x00000002
#define EVT_YBR_VGA_SIG_NO_SUPPORT  0x00000004
//#define EVT_YBR_VGA_SIG_CHG           0x00000008

#define EVT_YBR_VGA_SIG_MASK        0x00000007
/**
    Struct Define
*/

typedef struct {
    unsigned int source_type;
    int reserved;
}YBR_VGA_CFG;


typedef enum
{
    YBR_VGA_SUSPEND_STATE = 0,
    YBR_VGA_IDLE_STATE, 
    YBR_VGA_STANDBY_STATE,
    YBR_VGA_INIT_STATE,
    YBR_VGA_CONFIG_STATE,
    YBR_VGA_RUN_STATE,
    YBR_VGA_PAUSE_STATE,
    YBR_VGA_STOP_STATE,
    YBR_VGA_STATE_MAX
}E_YBR_VGA_OP_STATE;


typedef struct
{
    unsigned int u2Width;
    unsigned int u2Height;
   // unsigned char u1Timing;
    unsigned char u1Interlace;
   // unsigned char u1RefreshRate;
} YBR_VGA_VDO_INFO;



typedef struct
{
    YBR_VGA_CFG     tYbrVgaCfg;             
    HANDLE          hSigStateEvt;
}YBR_VGA_DRV_T, *PYBR_VGA_DRV_T;


u32 YBR_Init(LPCTSTR pszContext);
bool  YBR_Deinit(u32 dwContext);
u32 YBR_Open(u32 dwContext, u32 dwAccessMode, u32 dwShareMode);
bool  YBR_Close(u32 dwContext);
bool  YBR_IOControl(u32 context, u32 code, const u8 *pInBuffer, u32 inSize,
                  u8 *pOutBuffer, u32 outSize, u32 *pOutSize);


#endif
