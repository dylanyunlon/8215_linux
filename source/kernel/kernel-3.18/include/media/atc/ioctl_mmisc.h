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

#ifndef _IOCTL_MISCDEV_H
#define _IOCTL_MISCDEV_H

#include <linux/types.h>

#define MMISC_IOCTL_MAGIC  'M'

typedef enum {
  OSE_MEM_GET_NONE,
  OSE_MEM_GET_AVPBBUF,
  OSE_MEM_GET_APBBUF,
  OSE_MEM_GET_AVSLOTSIZE,
  OSE_MEM_GET_SPPBBUF,
  OSE_MEM_GET_SPSLOTSIZE,
  OSE_MEM_GET_VFIFO,
  OSE_MEM_GET_HBVFIFO,
  OSE_MEM_GET_SPFIFO,
  OSE_MEM_GET_MM_AFIFO,
  OSE_MEM_GET_4TH_AFIFO,
  OSE_MEM_GET_AUDIOIN_FIFO_SA,
  OSE_MEM_GET_AUDIOIN_FIFO_SIZE,
  OSE_MEM_GET_MM_RESERVED_PHY_SA,
  OSE_MEM_GET_MM_RESERVED_SIZE,
  OSE_MEM_GET_TYPE_MAX
} E_OSE_MEM_GET_TYPE_T;

typedef union {
  void *pvVirAddr;
  __u64 u8PhyAddr;
  __u32 size;
} OSE_MEM_GET_OUTINFO_T;

typedef struct {
  E_OSE_MEM_GET_TYPE_T eType;
  OSE_MEM_GET_OUTINFO_T rOut;
} OSE_MEM_GET_INFO_T;

typedef struct {
  void *pvAddr;
} MM_MEMDBG_INSTNODE_T;

typedef struct {
  void *pvAddr;
  void *pvOutAddr;
} MM_MEMDBG_RMNODE_T;

typedef struct {
  void *pvAddr;
  __u32 u4Size;
} MM_MEMDBG_GETNODESZ_T;

typedef struct {
  __u64 u8PhyAddr;
  __u32 u4HwVirAddr; /* local offset to the mm ose reserved memory address */
  __u32 u4Size;
  __u32 u4Align;
} OSE_MEM_HWMEM_INFO_T;

#define IOCTL_MMISC_GET_OSE_MEM_IFNO  _IOW(MMISC_IOCTL_MAGIC, 0, OSE_MEM_GET_INFO_T)

#define IOCTL_MMISC_ALLOC_HW_MEM  	  _IOWR(MMISC_IOCTL_MAGIC, 1, OSE_MEM_HWMEM_INFO_T)
#define IOCTL_MMISC_RELEASE_HW_MEM    _IOR(MMISC_IOCTL_MAGIC, 2,  __u32)

#define IOCTL_MMISC_MEMDBG_INSTNODE  _IOWR(MMISC_IOCTL_MAGIC, 10, MM_MEMDBG_INSTNODE_T)
#define IOCTL_MMISC_MEMDBG_RMNODE    _IOWR(MMISC_IOCTL_MAGIC, 11, MM_MEMDBG_RMNODE_T)
#define IOCTL_MMISC_MEMDBG_DUMP      _IO(MMISC_IOCTL_MAGIC, 12)
#define IOCTL_MMISC_MEMDBG_FLUSH     _IO(MMISC_IOCTL_MAGIC, 13)
#define IOCTL_MMISC_MEMDBG_GETSZ     _IOWR(MMISC_IOCTL_MAGIC, 14, MM_MEMDBG_GETNODESZ_T)

#endif				/* _IOCTL_MISCDEV_H */
