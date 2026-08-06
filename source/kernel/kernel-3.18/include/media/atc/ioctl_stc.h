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

#ifndef _IOCTL_ATC_STC_H
#define _IOCTL_ATC_STC_H

#include <linux/types.h>

#define STC_IOCTL_MAGIC  'S'

typedef enum _STCSTATUS
{
  STC_PAUSE,
  STC_RUN
} STC_STATUS_T;

typedef struct _stc_update_ininfo
{
  __s64    i8BaseTime;
  __s64    i8StartTime;
  __u64    u8StartMediaTime;
} STC_UPDATE_IN_INFO_T;

typedef struct _stc_update_outinfo
{
  __s64    i8BaseTime;
  __u64    u8STCTime;
} STC_UPDATE_OUT_INFO_T;

typedef struct stc_sget_time
{
  __u32 u4DevId;
  __u64 u8TimeVal;
} STC_SGET_TIME_T;

typedef struct stc_sget_status
{
  __u32 u4DevId;
  STC_STATUS_T eStatus;
} STC_SGET_STATUS_T;

typedef struct stc_sget_rate
{
  __u32 u4DevId;
  __u32 u4Rate;
  bool  is_slow_down;
} STC_SGET_RATE_T;

typedef struct stc_update_info
{
  __u32 u4DevId;
  STC_UPDATE_IN_INFO_T  rIn;
  STC_UPDATE_OUT_INFO_T rOut;
} STC_UPDATE_INFO_T;

#define IOCTL_STCDEV_ACQUIRE   _IOR(STC_IOCTL_MAGIC, 0, __u32)
#define IOCTL_STCDEV_RELEASE   _IOW(STC_IOCTL_MAGIC, 1, __u32)
#define IOCTL_STCDEV_GETTIME   _IOR(STC_IOCTL_MAGIC, 2, STC_SGET_TIME_T)
#define IOCTL_STCDEV_SETTIME   _IOW(STC_IOCTL_MAGIC, 3, STC_SGET_TIME_T)
#define IOCTL_STCDEV_GETSTATUS _IOR(STC_IOCTL_MAGIC, 4, STC_SGET_STATUS_T)
#define IOCTL_STCDEV_SETSTATUS _IOW(STC_IOCTL_MAGIC, 5, STC_SGET_STATUS_T)
#define IOCTL_STCDEV_SETRATE   _IOW(STC_IOCTL_MAGIC, 6, STC_SGET_RATE_T)
#define IOCTL_STCDEV_UPDATE    _IOWR(STC_IOCTL_MAGIC, 7, STC_UPDATE_INFO_T)

#endif //_IOCTL_ATC_STC_H

