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

#ifndef TVD_HAL_IF_H
#define TVD_HAL_IF_H

#include "tvd_data_struct.h"
#ifndef __ARM2__
#include <linux/irqreturn.h>
#include <linux/types.h>
#endif


extern struct clk *clk_ac8317_tvd1;
extern struct clk *clk_ac8317_tvd2;
extern void mt33xx_mask_ack_bim_irq(uint32_t irq);

extern void   tvd_hal_open(TVD_CHANNEL_ID channel_id);
extern void   tvd_hal_close(TVD_CHANNEL_ID channel_id);
extern void   tvd_channel_on_off(TVD_CHANNEL_ID channel_id, u32 channel, bool on_off);
void tvd_clock_on_off(bool on_off);
extern bool   tvd_channel_port_config(TVD_CHANNEL_ID channel_id, u32 channel, u32 port_num, u32 u4Rear, u32 u4CfgType);
extern bool   tvd_get_di_flag(TVD_CHANNEL_ID channel_id);
extern void tvd_hal_notify_close_wch(TVD_CHANNEL_ID channel_id);
extern TVD_SIG_STATE_T tvd_hal_get_signal_status(TVD_CHANNEL_ID channel_id);
extern s8 _tvd_register_notify(tvd_notify notify_fun, void *arg);
extern void   _tvd_unregister_notify(void);


#ifdef __ARM2__
extern void _tvd_interrupt_process(s32 irq);
#else
int _signal_process(void *arg);
extern irqreturn_t _tvd_interrupt_process(s32 irq, void *dev_id);
#endif




#endif

