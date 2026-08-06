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

#include <mach/hardware.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <asm/io.h>

#include <mach/ac83xx_system.h>

#include <mach/ac83xx_gpio_pinmux.h>
#include <mach/ac83xx_pinmux_table.h>
#include <linux/gpio.h>
#include <mach/ac83xx_gpio_pinmux_mapping.h>
#include <mach/pinmux.h>
#include <linux/timer.h>
#include <linux/delay.h>



#define SPM_READ32(REG)            __raw_readl(__io(SPM_BASE_VA + REG))
#define SPM_WRITE32(VAL, REG)      __raw_writel(VAL, __io(SPM_BASE_VA + REG))



#define MAX_PDWNC_INTR_SOURCE   (4)
typedef struct _intr_source{
	unsigned  int id;
	irq_handler_t handler;
	const char *name;
}intr_source;

static intr_source g_pdwnc_intrsource[MAX_PDWNC_INTR_SOURCE];
spinlock_t  irq_lock;

int request_pdwnc_irq(unsigned int sourceid, irq_handler_t handler, unsigned long flags,
		const char *name, void *dev)
{
	if(sourceid >= (MAX_PDWNC_INTR_SOURCE))
		return -1;


	pr_info("[PDWNC] request_pdwnc_irq:  sourceid=%d\n", sourceid);
	g_pdwnc_intrsource[sourceid].id = sourceid;
	g_pdwnc_intrsource[sourceid].handler = handler;
	g_pdwnc_intrsource[sourceid].name = name;

	return 0;

}
EXPORT_SYMBOL(request_pdwnc_irq);

static void PLLPowerDown(void)
{
	CKGEN_WRITE32(0x4,0);
	CKGEN_WRITE32(0x8,0);
	CKGEN_WRITE32(0xc,0);
	CKGEN_WRITE32(0x10,0);
	CKGEN_WRITE32(0x14,0);
	CKGEN_WRITE32(0x18,0);
	CKGEN_WRITE32(0X1C,0);
	CKGEN_WRITE32(0x20,0);
	CKGEN_WRITE32(0x24,0);
	CKGEN_WRITE32(0x28,0);
	CKGEN_WRITE32(0x2C,0);
	CKGEN_WRITE32(0x30,0);
	CKGEN_WRITE32(0x34,0);
	CKGEN_WRITE32(0xD4,0);
	CKGEN_WRITE32(0xD8,0);
	CKGEN_WRITE32(0xDC,0);
	CKGEN_WRITE32(0x188,0);
	CKGEN_WRITE32(0x18C,0);
	CKGEN_WRITE32(0x190,0);

#if 0
	HAL_WRITE32(0xF0008448,0x0);
	HAL_WRITE32(0xF0055060,0x0);
	HAL_WRITE32(0xF0000594,0xBE4C2020);
	unsigned int u4Tmp;

	/*all pll power off @standby*/
	u4Tmp = PDWNC_READ32(0x30);
	u4Tmp = u4Tmp | (1U << 7 );
	PDWNC_WRITE32(0x30,u4Tmp);

	/*ARMPLL Power off*/
	u4Tmp = PDWNC_READ32(0X184);
	u4Tmp =  (1U << 7 );
	PDWNC_WRITE32(0x184,u4Tmp);
	PDWNC_WRITE32(0x180,0xB5DFF);
	//decrease  pdwnc clock 
	//	 PDWNC_WRITE32(REG_RW_PDCKSEL,RW_PDCKSEL_250K);
#else

	PDWNC_WRITE32(0x180,0xB5DFB);
#endif
}

void set_pdwnc_gpio_value(uint32_t u4Pin,uint32_t u4value)
{
	uint32_t u4Mode =0;
	uint32_t u4Tmp;

	//set pin to GPIO
	u4Tmp = (1<<  u4Pin);
	PDWNC_WRITE32(REG_RW_PAD_PINMUX1, u4Tmp);

	//GPIO direct
	u4Tmp = PDWNC_READ32(REG_RW_GPIOEN);

	if(u4Mode == 1)
		u4Tmp = u4Tmp & (~(1U << u4Pin));
	else
		u4Tmp = u4Tmp | ((1U << u4Pin));

	PDWNC_WRITE32(REG_RW_GPIOEN, u4Tmp);

	//set output value
	u4Tmp = PDWNC_READ32(REG_RW_GPIOOUT);
	u4Tmp = u4Tmp & ( ~(1 << u4Pin));
	u4Tmp = u4Tmp | (u4value << u4Pin);
	if(u4Mode == 0)
		PDWNC_WRITE32(REG_RW_GPIOOUT,u4Tmp);

}


void arch_reset(enum reboot_mode mode, const char *cmd)
{
	/*
	 * use powerdown watch dog to reset system
	 */
	uint32_t u4Test;


	PDWNC_WRITE32(REG_RW_RESRV1, 0x33633363);

	PDWNC_WRITE32(REG_RW_WDT, 0xff000000);

	for(u4Test = 0; u4Test < 10000; u4Test++)
	{

	}
	PDWNC_WRITE32(REG_RW_WDTSET, 1);
	while(1);

}


static void pdwnc_set_wakeup_src(uint32_t u4Pin, uint32_t u4value)
{
	uint32_t u4Tmp;

	u4Tmp = PDWNC_READ32(REG_RW_WAKEN);
	u4Tmp |=(1U << u4Pin);
	PDWNC_WRITE32(REG_RW_WAKEN, u4Tmp);

	u4Tmp = PDWNC_READ32(REG_RW_PDIO);
	if(u4value)
		u4Tmp |=(1U << u4Pin);
	else
		u4Tmp |=(0U << u4Pin);

	PDWNC_WRITE32(REG_RW_PDIO, u4Tmp);
}


static void pdwnc_standby(void)
{
	PDWNC_WRITE32(REG_RW_PDCODE, 0x14);
	PDWNC_WRITE32(REG_RW_PDCODE, 0x04);
}

void ac83xx_power_off(void)
{
	//Set next bootup reason to "normal boot"
	PDWNC_WRITE32(REG_RW_RESRV1, 0x33633363);

	//Set opwrsb to high
	//set_pdwnc_gpio_value(3, 1);

	//Set wakeup_sts to high
	set_pdwnc_gpio_value(0, 0);

	//Set wakeup_src as wakeup_source
	pdwnc_set_wakeup_src(1, 1);
	PDWNC_WRITE32(REG_RW_WKRSC, 0x170000);
	
        PLLPowerDown();
	//Trigger standy
	pdwnc_standby();

	while(1);
}

void usb0_vbus_power_init()
{
	pr_info("[PDWNC] usb0_vbus_power_init \n");
#if 0
    /* config power-controller gpio */
    GPIO_MultiFun_Set(PIN_1_GPIO1, PINMUX_LEVEL_GPIO_END_FLAG);
    
    gpio_request(PIN_1_GPIO1, "usb0_power_switch");
    gpio_direction_output(PIN_1_GPIO1, 0);
    gpio_set_value(PIN_1_GPIO1, 0);
#endif
}
EXPORT_SYMBOL(usb0_vbus_power_init);

void usb0_vbus_power_reset()
{
	pr_info("[PDWNC] usb0_vbus_power_reset \n");
#if 0
    gpio_set_value(PIN_1_GPIO1, 1);
    msleep(200);
    gpio_set_value(PIN_1_GPIO1, 0);
#endif
	
}
EXPORT_SYMBOL(usb0_vbus_power_reset);


void usb1_vbus_power_init()
{
	pr_info("[PDWNC] usb1_vbus_power_init \n");
#if 0
		/* config power-controller gpio */
		GPIO_MultiFun_Set(PIN_2_GPIO2, PINMUX_LEVEL_GPIO_END_FLAG);
		
		gpio_request(PIN_2_GPIO2, "usb_power_switch");
		gpio_direction_output(PIN_2_GPIO2, 0);
		gpio_set_value(PIN_2_GPIO2, 0);
#endif

}
EXPORT_SYMBOL(usb1_vbus_power_init);

void usb1_vbus_power_reset()
{
	pr_info("[PDWNC] usb1_vbus_power_reset \n");
#if 0
		gpio_set_value(PIN_2_GPIO2, 1);
		msleep(200);
		gpio_set_value(PIN_2_GPIO2, 0);
#endif

	
}
EXPORT_SYMBOL(usb1_vbus_power_reset);

static uint32_t  pdwnc_gpio_init(uint32_t u4Pin,uint32_t u4TrigMode,uint32_t u4Enable)
{
	uint32_t  u4Tmp;

	//set pin to GPIO
	u4Tmp = (1<<  u4Pin);
	PDWNC_WRITE32(REG_RW_PAD_PINMUX1, u4Tmp); 


	//GPIO input set
	u4Tmp = PDWNC_READ32(REG_RW_GPIOEN);
	u4Tmp = u4Tmp & (~(1U << u4Pin));
	PDWNC_WRITE32(REG_RW_GPIOEN, u4Tmp);
	//set Interrupt Trigger mode
	switch(u4TrigMode)
	{
		case 0://Level low mode
			u4Tmp = (0x100 << u4Pin);
			u4Tmp = u4Tmp | (0 << u4Pin);
			break;
		case 1://Level high mode
			u4Tmp = (0x100 << u4Pin);
			u4Tmp = u4Tmp | (1 << u4Pin);
			break;
		case 2://negative edge mode
			u4Tmp = (0x000 << u4Pin);
			u4Tmp = u4Tmp | (0 << u4Pin);
			break;
		case 3://positive edge mode
			u4Tmp = (0x000 << u4Pin);
			u4Tmp = u4Tmp | (1 << u4Pin);
			break;
		case 4://Double trigger mode
			u4Tmp = (0x10000 << u4Pin);
			break;
		default :
			pr_info("[PDWNC] Trigger mode %d is error.\n",u4TrigMode);
			return -1;
	}
	pr_info("[PDWNC] gpio trigger mode setting:0x%x\n",u4Tmp);
	PDWNC_WRITE32(REG_RW_EXINTCFG,u4Tmp);

	//Interropt enable or disable
	if(0 == u4Enable)
	{
		PDWNC_WRITE32(REG_RW_INTEN, PDWNC_READ32(REG_RW_INTEN)&(~(1<<u4Pin)));
		pr_info("[PDWNC] Pin %d interrupt Disable.\n",u4Pin);
	}
	else
	{
		PDWNC_WRITE32(REG_RW_INTEN, PDWNC_READ32(REG_RW_INTEN)|(1<< u4Pin));
		pr_info("[PDWNC] Pin %d interrupt Enable.\n",u4Pin);
	}

	return 0;

}

extern void ac83xx_mask_ack_bim_irq(uint32_t);
static irqreturn_t pdwnc_isr_handler(int irq, void *dev_id)
{
	uint32_t u4tmp = PDWNC_READ32(REG_RW_INTSTA);

	unsigned long irqflags;
	spin_lock_irqsave(&irq_lock, irqflags);
	disable_irq_nosync(VECTOR_PWDNC);
	spin_unlock_irqrestore(&irq_lock, irqflags);


	if(u4tmp & (1U << 0)){

		if(g_pdwnc_intrsource[PDWNC_INTR_GPIO_WAKEUP_STS].handler != NULL){
			g_pdwnc_intrsource[PDWNC_INTR_GPIO_WAKEUP_STS].handler(irq,dev_id);
		}

	}

	if(u4tmp & (1U << 1)){

		if(g_pdwnc_intrsource[PDWNC_INTR_GPIO_WAKEUP_SRC].handler != NULL){
			g_pdwnc_intrsource[PDWNC_INTR_GPIO_WAKEUP_SRC].handler(irq,dev_id);
		}
	}

	if(u4tmp & (1U << 2)){
		if(g_pdwnc_intrsource[PDWNC_INTR_GPIO_IR].handler != NULL){
			g_pdwnc_intrsource[PDWNC_INTR_GPIO_IR].handler(irq,dev_id);
		}
	}

	if(u4tmp & (1U << 13)){
		if(g_pdwnc_intrsource[PDWNC_INTR_IR].handler != NULL){
			g_pdwnc_intrsource[PDWNC_INTR_IR].handler(irq,dev_id);
		}
	}

	PDWNC_WRITE32(REG_RW_INTCLR,u4tmp);
	ac83xx_mask_ack_bim_irq(VECTOR_PWDNC);

	irqflags = 0;
	spin_lock_irqsave(&irq_lock, irqflags);
	enable_irq(VECTOR_PWDNC);
	spin_unlock_irqrestore(&irq_lock, irqflags);

	return IRQ_HANDLED;
}






static int __init  pdwnc_init(void)
{
	int ret = 0;

	/* enable spm clock  */
	SPM_WRITE32(0x02860001, 0);

	pdwnc_gpio_init(1,3,1);
	spin_lock_init(&irq_lock);

	memset((void *)g_pdwnc_intrsource,0x0,sizeof(g_pdwnc_intrsource));
	ret = request_irq(VECTOR_PWDNC, pdwnc_isr_handler,
			0,"PDWNC_ISR", NULL);
	if(0 == ret)
		pr_info("[PDWNC] pdwnc_init success\r\n");
	else
		pr_info("[PDWNC] pdwnc_init failed\r\n");

	return ret;

}
static void pdwnc_exit(void)
{

	free_irq(VECTOR_PWDNC,NULL);

}

module_init(pdwnc_init);
module_exit(pdwnc_exit);
