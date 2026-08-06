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

#include<linux/module.h>
#include<linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/cpufreq.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <mach/dma.h>
#include <linux/delay.h>
#include <asm/delay.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <mach/ac83xx_gpio_pinmux.h>
#include <mach/ac83xx_system.h>
#include "ac83xx_keyadc.h"

#ifdef CONFIG_AC83XX_POWER_KEY

#define POWER_KEY_POLLING_TIME   (jiffies + 100*(HZ/1000))  /* 5ms */
#define POWER_KEY       116   //android POWER_KEY

void powerkey_timer(unsigned long data);

static struct  timer_list power_key_timer;

static  struct input_dev   *input = NULL;

static uint32_t get_powerkey_value(void)
{
    uint32_t u4Value = PDWNC_READ32(REG_RW_GPIOIN);

    return (u4Value & (1U << PDWNC_INTR_GPIO_WAKEUP_SRC));

}

void powerkey_timer(unsigned long data)
{
    uint32_t tmp = 0;
    tmp = get_powerkey_value();
    pr_info("[KP]power key value:%s\n",tmp?("high"):("low"));
    //tmp = 0;//delete by cgx to enable long press powerkey
    if(tmp == 0)
    {
        pr_info("[KP]power key release\r\n");
        input_report_key(input,POWER_KEY,0);
        input_sync(input);
    }    
    else
    {
        mod_timer(&power_key_timer,POWER_KEY_POLLING_TIME);
    }
}

extern s32 is_handle_power_key(void);
#ifndef CONFIG_PWRK_ATOMIC_PROTECT
extern void enable_handle_power_key(int b_enable);
#endif

static irqreturn_t powerkey_isr_handler(int irq, void *dev_id)
{
    if (is_handle_power_key())
    {
        pr_info("[KP]power key down\r\n");
        input_report_key(input,POWER_KEY,1);
        input_sync(input);
        mod_timer(&power_key_timer,POWER_KEY_POLLING_TIME);
#ifndef CONFIG_PWRK_ATOMIC_PROTECT
        enable_handle_power_key(0);
#endif
    }
    return IRQ_HANDLED;
}

int  ac83xx_powerkey_init(struct input_dev   *keypad_input)
{
    int ret = -1;
    input = keypad_input;
    init_timer(&power_key_timer);
    power_key_timer.data = 0;
    power_key_timer.expires = 0;
    power_key_timer.function = powerkey_timer;
    ret = request_pdwnc_irq(PDWNC_INTR_GPIO_WAKEUP_SRC, powerkey_isr_handler,0,"POWER-KEY-ISR", NULL);

    return ret;
}
#endif //
