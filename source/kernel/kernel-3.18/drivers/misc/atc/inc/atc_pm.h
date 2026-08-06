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

#ifndef __ATC_PM_H
#define __ATC_PM_H

#include <linux/interrupt.h>

#define PDWNC_INTR_GPIO_WAKEUP_STS  0
#define PDWNC_INTR_GPIO_WAKEUP_SRC  1
#define PDWNC_INTR_GPIO_IR          2
#define PDWNC_INTR_IR               3
#define MAX_PDWNC_INTR_SOURCE       4

typedef struct _intr_source{
	unsigned  int id;
	irq_handler_t handler;
	const char *name;
}intr_source;

void set_pdwnc_gpio_value(uint32_t u4Pin, uint32_t u4value);
int request_pdwnc_irq(unsigned int sourceid, irq_handler_t handler, 
    unsigned long flags,const char *name,void *dev);
void ac83xx_power_off(void);
void arch_reset(char mode, const char *cmd);


//spm interface.
void clusterx_poweron(u32 cluster_id, u32 l2c_from);
void clusterx_poweroff(u32 cluster_id, u32 l2c_into);
void corex_poweron(u32 core_id);
void corex_poweroff(u32 core_id);

void spm_power_core(u32 cpu, u32 on_off);
void spm_power_g3d(u32 sw_hw_sel, u32 on_off);
void spm_power_vdec(u32 sw_hw_sel, u32 on_off);
void spm_power_arm9(u32 sw_hw_sel, u32 on_off);
void spm_power_msdc(u32 sw_hw_sel, u32 on_off);
void spm_power_usb20(u32 sw_hw_sel, u32 on_off);
void spm_power_ssusb(u32 sw_hw_sel, u32 on_off);
void spm_power_all(u32 sw_hw_sel, u32 on_off);

#endif
