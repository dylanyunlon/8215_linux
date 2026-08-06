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

#ifndef _HAL_MUT_H_
#define _HAL_MUT_H_

#include "windows.h"
#include <media/atc/ioctl_stc.h>

extern bool stcdevs_init(void);
extern bool stcdevs_deinit(void);
extern void *stc_inst_create(void);
extern long stc_inst_release(void *pvInst);
extern long stc_inst_acquire_dev(void *pvInst, __u32 *pu4DevId);
extern long stc_inst_release_dev(void *pvInst, __u32 u4DevId);
extern long stc_inst_get_time(void *pvInst, STC_SGET_TIME_T *prTimeVal);
extern long stc_inst_set_time(void *pvInst, STC_SGET_TIME_T *prTimeVal);
extern long stc_inst_get_status(void *pvInst, STC_SGET_STATUS_T *prStatus);
extern long stc_inst_set_status(void *pvInst, STC_SGET_STATUS_T *prStatus);
extern long stc_inst_set_rate(void *pvInst, STC_SGET_RATE_T *prRate);
extern long stc_inst_update(void *pvInst, STC_UPDATE_INFO_T *prInfo);

#endif				/* _HAL_STC_H_ */
