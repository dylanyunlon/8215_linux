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

#ifndef _IOCTL_WCM_
#define _IOCTL_WCM_

#define WCM_IOCTL_ALLOC_WCBUF              0X1

#define WCM_IOCTL_FREE_WCBUF               0x2

typedef struct _WCM_INFO
{
    UINT32 u4Width;
    UINT32 u4Height;
} WCM_INFO_T;


typedef struct _WCM_DECBUF_INFO
{
    UINT32 u4YPhyAddr;
    UINT32 u4CPhyAddr;
    UINT32 u4BufNum;
} WCM_DECBUF_INFO_T;

#endif //_IOCTL_WCM_

