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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>  
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include <asm/io.h>

#include "atc_pm.h"
#include "x_pdwnc.h"
#include "mach/bim.h"

void __iomem *pdwnc_base;
int pdwnc_irqnr;

spinlock_t irq_lock;

static intr_source g_pdwnc_intrsource[MAX_PDWNC_INTR_SOURCE];

#define PDWNC_READ32(offset)         __raw_readl(pdwnc_base + offset)
#define PDWNC_WRITE32(offset, value) \
	do {  \
		__raw_writel(value, (pdwnc_base + offset));  \
		mb();  \
	} while(0);


void arch_reset(char mode, const char *cmd)
{
	/*
	 * use powerdown watch dog to reset system
	 */
	uint32_t u4Test;

	pr_err("[PDWNC] %s\n", __func__);

	PDWNC_WRITE32(REG_RW_RESRV1, 0x33653365);

	PDWNC_WRITE32(REG_RW_WDT, 0xff000000);

	for(u4Test = 0; u4Test < 10000; u4Test++)
	{
	}

	PDWNC_WRITE32(REG_RW_WDTSET, 1);

	while(1);
}


void set_pdwnc_gpio_value(uint32_t u4Pin, uint32_t u4value)
{
	uint32_t u4Tmp = 0;
	uint32_t i = 0;

	if(u4Pin > 6 || u4value > 1){
		pr_err("[PDWNC] %s, error para u4Pin(%d), u4value(%d)\n", __func__, u4Pin, u4value);
		return;
	}

	pr_err("[PDWNC] %s++, PIN(0x%x), EN(0x%x), OUT(0x%x)\n", __func__,
		PDWNC_READ32(REG_RW_PAD_PINMUX1), PDWNC_READ32(REG_RW_GPIOEN), PDWNC_READ32(REG_RW_GPIOOUT));
	// u4Idx:
	// 0 --> wakeup_sts
	// 1 --> wakeup_src
	// 2 --> ir pin
	// 3 --> gpio7
	// 4 --> gpio8
	// 5 --> gpio9
	// 6 --> opwrsb

	do {
		//set pin to GPIO
		u4Tmp = PDWNC_READ32(REG_RW_PAD_PINMUX1);
		u4Tmp = u4Tmp | (1U << u4Pin);
		PDWNC_WRITE32(REG_RW_PAD_PINMUX1, u4Tmp);
		i++;
	} while (0 == (PDWNC_READ32(REG_RW_PAD_PINMUX1) & (1U << u4Pin)));

	do {
		//GPIO direct
		u4Tmp = PDWNC_READ32(REG_RW_GPIOEN);
		u4Tmp = u4Tmp | (1U << u4Pin);
		PDWNC_WRITE32(REG_RW_GPIOEN, u4Tmp);
		i++;
	} while (0 == (PDWNC_READ32(REG_RW_GPIOEN) & (1U << u4Pin)));

	do {
		//set output value
		u4Tmp = PDWNC_READ32(REG_RW_GPIOOUT);
		u4Tmp = u4Tmp & ( ~(1U << u4Pin));
		u4Tmp = u4Tmp | (u4value << u4Pin);
		PDWNC_WRITE32(REG_RW_GPIOOUT,u4Tmp);
		i++;
	} while ((u4value << u4Pin) != (PDWNC_READ32(REG_RW_GPIOOUT) & (1U << u4Pin)));

	pr_err("[PDWNC] %s--, PIN(0x%x), EN(0x%x), OUT(0x%x), count(%d)\n", __func__,
		PDWNC_READ32(REG_RW_PAD_PINMUX1), PDWNC_READ32(REG_RW_GPIOEN), PDWNC_READ32(REG_RW_GPIOOUT), i);
}

static uint32_t pdwnc_gpio_init(uint32_t u4Pin, uint32_t u4TrigMode, uint32_t u4Enable)
{
	uint32_t  u4Tmp;
	pr_err("[PDWNC] %s\n", __func__);
	//set pin to GPIO
	u4Tmp = (1 << u4Pin);
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
		pr_err("[PDWNC] Trigger mode %d is error.\n",u4TrigMode);
		return -1;
	}
	pr_err("[PDWNC] gpio trigger mode setting:0x%x\n",u4Tmp);
	PDWNC_WRITE32(REG_RW_EXINTCFG,u4Tmp);

	//Interropt enable or disable
	if(0 == u4Enable)
	{
		PDWNC_WRITE32(REG_RW_INTEN, PDWNC_READ32(REG_RW_INTEN)&(~(1<<u4Pin)));
		pr_err("[PDWNC] Pin %d interrupt Disable.\n",u4Pin);
	}
	else
	{
		PDWNC_WRITE32(REG_RW_INTEN, PDWNC_READ32(REG_RW_INTEN)|(1<< u4Pin));
		pr_err("[PDWNC] Pin %d interrupt Enable.\n",u4Pin);
	}

	return 0;

}

static irqreturn_t pdwnc_isr_handler(int irq, void *dev_id)
{
	unsigned long irqflags;

	//Check which INT coming
	uint32_t u4tmp = PDWNC_READ32(REG_RW_INTSTA);

	spin_lock_irqsave(&irq_lock, irqflags);
	disable_irq_nosync(pdwnc_irqnr);
	spin_unlock_irqrestore(&irq_lock, irqflags);
	//pr_err("[PDWNC] %s\n", __func__);

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

	//Clear INT
	PDWNC_WRITE32(REG_RW_INTCLR, u4tmp);

	//BIM Ack
	mt33xx_mask_ack_bim_irq(pdwnc_irqnr);

	irqflags = 0;
	spin_lock_irqsave(&irq_lock, irqflags);
	enable_irq(pdwnc_irqnr);
	spin_unlock_irqrestore(&irq_lock, irqflags);

	return IRQ_HANDLED;
}

int request_pdwnc_irq(unsigned int sourceid, irq_handler_t handler, unsigned long flags,
		const char *name, void *dev)
{
	if(sourceid >= (MAX_PDWNC_INTR_SOURCE))
		return -1;

	pr_err("[PDWNC] register_pdwnc_irq:  sourceid=%d\n", sourceid);
	g_pdwnc_intrsource[sourceid].id = sourceid;
	g_pdwnc_intrsource[sourceid].handler = handler;
	g_pdwnc_intrsource[sourceid].name = name;

	return 0;
}
EXPORT_SYMBOL(request_pdwnc_irq);


static int pdwnc_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device_node *node;

	node = pdev->dev.of_node;
	if(!node) {
		pr_err("[PDWNC] get device node failed\n");
		return -ENODEV;
	}

	pdwnc_base = of_iomap(node, 0);
	if(!pdwnc_base) {
		pr_err("[PDWNC] iomap failed\n");
	}

	pdwnc_irqnr = irq_of_parse_and_map(node, 0);
	if(!pdwnc_irqnr) {
		pr_err("[PDWNC] get irq failed\n");
	}

	pr_err("[PDWNC] %s, base(0x%p), irq(%d)\n", __func__, pdwnc_base, pdwnc_irqnr);

	spin_lock_init(&irq_lock);

	memset((void *)g_pdwnc_intrsource, 0x0, sizeof(g_pdwnc_intrsource));

	ret = request_irq(pdwnc_irqnr, pdwnc_isr_handler, 0, "PDWNC_ISR", NULL);

	if (ret){
		pr_err("[PDWNC] pdwnc request_irq failed\n");
	}

	/* enable spm clock  */
	//SPM_WRITE32(0, 0x02860001);

	/* Set wakeup_src as positive edge trigger interrupt. */
	pdwnc_gpio_init(1, 3, 1);

	return ret;
}

static int pdwnc_remove(struct platform_device *pdev)
{
	pr_err("[PDWNC] %s\n", __func__);

	free_irq(pdwnc_irqnr, NULL);
	return 0;
}

static const struct of_device_id pdwnc_of_match[] = {
	{.compatible = "Autochips,pdwnc",},
	{},
};

static struct platform_driver pdwnc_driver = {
	.driver = {
		.name = "pdwnc",
		.owner = THIS_MODULE,
		.of_match_table = pdwnc_of_match,
	},
	.probe = pdwnc_probe,
	.remove = pdwnc_remove,
};

#if 0
static struct platform_device pdwnc_device = {
	.name = "pdwnc",
	.id = -1,
};
#endif

static int __init pdwnc_init(void)
{
	int ret = 0;
	pr_err("[PDWNC] %s\n", __func__);

#if 0
	ret = platform_device_register(&pdwnc_device);
	if (ret){
		pr_err("[PDWNC] pdwnc register device failed\n");
	}
#endif

	ret = platform_driver_register(&pdwnc_driver);
	if (ret){
		pr_err("[PDWNC] pdwnc register driver failed\n");
	}

	return ret;
}

static void __exit pdwnc_exit(void)
{
	platform_driver_unregister(&pdwnc_driver);
}

module_init(pdwnc_init);
module_exit(pdwnc_exit);

