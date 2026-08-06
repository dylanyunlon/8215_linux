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

#ifndef _HAL_STC_H_
#define _HAL_STC_H_

#include <linux/types.h>
#include <media/atc/ioctl_stc.h>
#include <x_os.h>

#define MAX_OF_STC_DEV_CNT 2

extern void __iomem *stc_base_regs;

bool STC_HalInit(void);
void STC_HalDeinit(void);

bool STC_HalGetTime(u32 u4DevId, u64 *pu8Time);
bool STC_HalSetTime(u32 u4DevId, u64 u8Time);
bool STC_HalSetRate(u32 u4DevId, u32 u4Rate, bool is_slow_down);

bool STC_HalStart(u32 u4DevId);
bool STC_HalPause(u32 u4DevId);
bool STC_HalGetStatus(u32 u4DevId, STC_STATUS_T *peStatus);

bool STC_HalUpdate(u32 u4DevId, STC_UPDATE_IN_INFO_T *prIn, STC_UPDATE_OUT_INFO_T *prOut);


#endif				/* _HAL_STC_H_ */
