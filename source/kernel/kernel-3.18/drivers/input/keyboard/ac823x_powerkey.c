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
//#include <mach/dma.h>//cgx 823x
#include <linux/delay.h>
#include <asm/delay.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
//#include <mach/ac83xx_gpio_pinmux.h>//cgx 823x
//#include <mach/ac83xx_system.h>//cgx 823x
#include "ac823x_keyadc.h"
#include "../../misc/atc/inc/x_ioopt.h"//cgx
#include "../../misc/atc/inc/x_pdwnc.h"//cgx
#include "../../misc/atc/inc/atc_pm.h"//cgx

#ifdef CONFIG_AC823X_POWER_KEY

#define POWER_KEY_POLLING_TIME   (jiffies + 100*(HZ/1000))  /* 5ms */
#define POWER_KEY       116   //android POWER_KEY

// pdwnc//cgx
#define REG_RW_GPIO_WAKEN             0x080
#define REG_RW_GPIO_PDSTAT            0x088
#define REG_RW_GPIO_PDSTCLR           0x08C
//#define REG_RW_PDIO                   0x0C4
#define REG_RW_GPIOIN                 0x0D0
#define REG_RW_GPIOEN                 0x0D4
#define REG_RW_GPIOOUT                0x0D8
#define REG_RW_PAD_PINMUX1            0x0F4
#define REG_RW_PAD_PINMUX2            0x0F8
#define REG_RW_PAD_PINMUX3            0x0FC
#define REG_RW_INTSTA				  0x140                     //PDWNC INTERRUPT STATUS REGISTER
#define REG_RW_INTEN				  0x144                     //PDWNC INTERRUPT ENABLE REGISTER
#define REG_RW_INTCLR				  0x148 

#define PDWNC_INTR_GPIO_WAKEUP_STS  0
#define PDWNC_INTR_GPIO_WAKEUP_SRC  1
#define PDWNC_INTR_GPIO_IR          2

#define PDWNC_VIRT                  (IO_UCV_BASE_FOR_KP + 0x00024000)
#define PDWNC_READ32(offset)        IO_READ32(PDWNC_VIRT, (offset))


void powerkey_timer(unsigned long data);

static struct  timer_list power_key_timer;

static  struct input_dev   *input = NULL;
static int sPowerCount = 0;

static uint32_t get_powerkey_value(void)
{
    uint32_t u4Value = PDWNC_READ32(REG_RW_GPIOIN);//cgx 823x

    return (u4Value & (1U << PDWNC_INTR_GPIO_WAKEUP_SRC));

}

void powerkey_timer(unsigned long data)
{
    uint32_t tmp = 0;
    tmp = get_powerkey_value();
    pr_info("power key value:%s\n",tmp?("high"):("low"));
    //tmp = 0;//delete by cgx to enable long press powerkey
    if(tmp == 0)
    {
        pr_info("power key release\r\n");
        input_report_key(input,POWER_KEY,0);
        input_sync(input);
    }    
    else
    {
        mod_timer(&power_key_timer,POWER_KEY_POLLING_TIME);
    }
}

extern s32 is_handle_power_key(void);
extern void enable_handle_power_key(int b_enable);

static irqreturn_t powerkey_isr_handler(int irq, void *dev_id)
{
    uint32_t temp;
	
    if (is_handle_power_key())
    {
        pr_info("[Keypad] power key down\r\n");
        input_report_key(input,POWER_KEY,1);
        input_sync(input);
        mod_timer(&power_key_timer,POWER_KEY_POLLING_TIME);   
        enable_handle_power_key(0);
    
        /*
        * When start suspend/resume, disable/enable card detect for mmc, 
        * otherwise plug in/plug out card will block quickboot flow. (CR: CNB00188149)
        */
        
        
        temp = PDWNC_READ32(REG_RW_GPIOOUT);//cgx 823x	 
        if ((temp & 0x1) != 0) { 
                sPowerCount++;
                if (sPowerCount == 1) {
                        enable_irq(201);
                        enable_irq(202);
                } else {
                        //pr_err("[Keypad] Too many power key %d\r\n", sPowerCount);
                }
        } else {
                sPowerCount = 0;
                disable_irq_nosync(201);
                disable_irq_nosync(202);
        }
        
    	
        return IRQ_HANDLED;
    }
}



int  ac823x_powerkey_init(struct input_dev   *keypad_input)
{
    int ret = -1;
    pr_info("ac823x_powerkey_init\n");

    input = keypad_input;
    init_timer(&power_key_timer);
    power_key_timer.data = 0;
    power_key_timer.expires = 0;
    power_key_timer.function = powerkey_timer;
    ret = request_pdwnc_irq(PDWNC_INTR_GPIO_WAKEUP_SRC, powerkey_isr_handler,0,"POWER-KEY-ISR", NULL);//cgx 823x

    return ret;
}
#endif //
