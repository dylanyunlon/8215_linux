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
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <asm/io.h>
#include <mach/bim.h>

#define REG_IRQST           0x0030        //RISC L1 IRQ Status Register
#define REG_IRQEN           0x0034        //RISC L1 IRQ Enable Register
#define REG_IRQCL           0x0038        //RISC L1 IRQ Clear Register
#define REG_FIQST           0x003C        //RISC L1 FIQ Status Register
#define REG_FIQEN           0x0040        //RISC L1 FIQ Enable Register
#define REG_FIQCL           0x0044        //RISC L1 FIQ Clear Register

#define REG_IRQST2          0x0048        //RISC L2 IRQ Status Register
#define REG_IRQEN2          0x004C        //RISC L2 IRQ Enable Register
#define REG_IRQCL2          0x0050        //RISC L2 IRQ Clear Register
#define REG_FIQST2          0x0054        //RISC L2 FIQ Status Register
#define REG_FIQEN2          0x0058        //RISC L2 FIQ Enable Register
#define REG_FIQCL2          0x005C        //RISC L2 FIQ Clear Register

#define REG_IRQST3          0x0060        //RISC L3 IRQ Status Register
#define REG_IRQEN3          0x0064        //RISC L3 IRQ Enable Register
#define REG_IRQCL3          0x0068        //RISC L3 IRQ Clear Register
#define REG_FIQST3          0x006c        //RISC L3 FIQ Status Register
#define REG_FIQEN3          0x0070        //RISC L3 FIQ Enable Register
#define REG_FIQCL3          0x0074        //RISC L3 FIQ Clear Register

#define REG_IRQST4          0x0230        //RISC L4 IRQ Status Register
#define REG_IRQEN4          0x0234        //RISC L4 IRQ Enable Register
#define REG_IRQCL4          0x0238        //RISC L4 IRQ Clear Register
#define REG_FIQST4          0x023C        //RISC L4 FIQ Status Register
#define REG_FIQEN4          0x0240        //RISC L4 FIQ Enable Register
#define REG_FIQCL4          0x0244        //RISC L4 FIQ Clear Register

#define REG_IRQST5          0x0310        //RISC L5 IRQ Status Register
#define REG_IRQEN5          0x0314        //RISC L5 IRQ Enable Register
#define REG_IRQCL5          0x0318        //RISC L5 IRQ Clear Register
#define REG_FIQST5          0x031C        //RISC L5 FIQ Status Register
#define REG_FIQEN5          0x0320        //RISC L5 FIQ Enable Register
#define REG_FIQCL5          0x0324        //RISC L5 FIQ Clear Register

#define REG_ARMPLL_DIV_MP0  0x0448
#define REG_ARMPLL_DIV_MP1  0x0458

#define ARMCLK_CKDIV_TOG	(1<<25)
#define ARMCLK_CKDIV_SEL_MASK	(0x1F << 20)
#define ARMCLK_CKDIV_SEL_SET(v)	((v) << 20)


//For cluster freq
#define SYS_ARMPLL_1G			1001000  //KHz
#define ARMCLK_1G_1d1		(SYS_ARMPLL_1G)
#define ARMCLK_1G_3d4		((SYS_ARMPLL_1G*3)/4)
#define ARMCLK_1G_1d2		(SYS_ARMPLL_1G/2)
#define ARMCLK_1G_1d4		(SYS_ARMPLL_1G/4)
#define ARMCLK_1G_4d5		((SYS_ARMPLL_1G*4)/5)
#define ARMCLK_1G_3d5		((SYS_ARMPLL_1G*3)/5)
#define ARMCLK_1G_2d5		((SYS_ARMPLL_1G*2)/5)
#define ARMCLK_1G_1d5		(SYS_ARMPLL_1G/5)
#define ARMCLK_1G_5d6		((SYS_ARMPLL_1G*5)/6)
#define ARMCLK_1G_2d3		((SYS_ARMPLL_1G*2)/3)
#define ARMCLK_1G_1d3		(SYS_ARMPLL_1G/3)
#define ARMCLK_1G_1d6		(SYS_ARMPLL_1G/6)


#define SYS_ARMPLL_1G2			1188000  //KHz
#define ARMCLK_1G2_1d1		(SYS_ARMPLL_1G2)
#define ARMCLK_1G2_3d4		((SYS_ARMPLL_1G2*3)/4)
#define ARMCLK_1G2_1d2		(SYS_ARMPLL_1G2/2)
#define ARMCLK_1G2_1d4		(SYS_ARMPLL_1G2/4)
#define ARMCLK_1G2_4d5		((SYS_ARMPLL_1G2*4)/5)
#define ARMCLK_1G2_3d5		((SYS_ARMPLL_1G2*3)/5)
#define ARMCLK_1G2_2d5		((SYS_ARMPLL_1G2*2)/5)
#define ARMCLK_1G2_1d5		(SYS_ARMPLL_1G2/5)
#define ARMCLK_1G2_5d6		((SYS_ARMPLL_1G2*5)/6)
#define ARMCLK_1G2_2d3		((SYS_ARMPLL_1G2*2)/3)
#define ARMCLK_1G2_1d3		(SYS_ARMPLL_1G2/3)
#define ARMCLK_1G2_1d6		(SYS_ARMPLL_1G2/6)

#define VECTOR_GIC_OFFSET	104
#define __io(a)	((void __iomem *)(a))

static void __iomem *bim_reg_base;

static inline u32 __bim_readl(u64 regaddr)
{
	return __raw_readl(__io(bim_reg_base + regaddr));
}

static inline void __bim_writel(u32 regval32, u64 regaddr)
{
	__raw_writel(regval32, __io(bim_reg_base + regaddr));
}

static u32 __bim_get_armpll_val(u32 cluster_id)
{
    return (cluster_id == 0) ? (__bim_readl(REG_ARMPLL_DIV_MP0)) : (__bim_readl(REG_ARMPLL_DIV_MP1));
}

static void __bim_set_armpll_val(u32 cluster_id, u32 regval)
{
    (cluster_id == 0) ? (__bim_writel(regval, REG_ARMPLL_DIV_MP0)) : (__bim_writel(regval, REG_ARMPLL_DIV_MP1));
}

static inline void __bim_tog_clk_div(u32 cluster_id)
{
    u32 regv=0;

    pr_debug("[BIM] tog %d clock \n", cluster_id);

	regv = __bim_get_armpll_val(cluster_id);
	
	if(regv & ARMCLK_CKDIV_TOG) {
		regv &= ~ARMCLK_CKDIV_TOG;
	} else {
	    regv |= ARMCLK_CKDIV_TOG;
	}

	__bim_set_armpll_val(cluster_id, regv);

	return;
}

#include "../../../gpio/ac823x_pinmux.h"
#include "../../../gpio/ac823x_pinmux_reg.h"
#include "../../../gpio/ac823x_gpio_reg.h"
#include "../../../gpio/gpio_ac823x_pinmux_table.h"
void ac8237_tmp_gpio_value(uint32_t u4Pin, uint32_t u4value)
{
    pr_info("Set GPIO%d as %d \n", u4Pin, u4value);
	atc_set_gpio_pinmux(u4Pin, PINMUX_LEVEL_GPIO_END_FLAG);
	atc_set_gpio_dir(u4Pin, OUTPUT);

	atc_set_gpio_out(u4Pin, u4value);
    return;
}
EXPORT_SYMBOL(ac8237_tmp_gpio_value);

u32 bim_get_cluster_freq(u32 cluster_id)
{
	return ((__bim_get_armpll_val(cluster_id) & (ARMCLK_CKDIV_SEL_MASK))>>20);
}
EXPORT_SYMBOL(bim_get_cluster_freq);

u32 bim_set_cluster_freq(u32 cluster_id, u32 drvdata)
{
    u32 regv = 0;
    pr_debug("[BIM] abjust %d clock to 0x%x \n", cluster_id, drvdata);
    
    regv = __bim_get_armpll_val(cluster_id);
	regv = (regv & ~(ARMCLK_CKDIV_SEL_MASK));
	regv = (regv | (drvdata<<20));
	__bim_set_armpll_val(cluster_id, regv);

	__bim_tog_clk_div(cluster_id);
	
    return 0;
}
EXPORT_SYMBOL(bim_set_cluster_freq);

void bim_mask_irq(unsigned int virq)
{
	u32 regval32;
	unsigned long flags;
	unsigned long irq;
	struct irq_data *irq_data;
	
	irq_data = irq_get_irq_data(virq);
	if (!irq_data)
		BUG();
	
	irq = irq_data->hwirq - VECTOR_GIC_OFFSET;
	
	local_irq_save(flags);
	if (irq < 32) {
		regval32 = __bim_readl(REG_IRQEN);
		regval32 &= ~(1 << irq);
		__bim_writel(regval32, REG_IRQEN);
	} else if ((irq >= 32) && (irq < 64)) {
		irq = irq - 32;
		regval32 = __bim_readl(REG_IRQEN2);
		regval32 &= ~(1 << irq);
		__bim_writel(regval32, REG_IRQEN2);
	} else if ((irq >= 64) && (irq < 96)) {
		irq = irq - 64;
		regval32 = __bim_readl(REG_IRQEN3);
		regval32 &= ~(1 << irq);
		__bim_writel(regval32, REG_IRQEN3);
	} else if ((irq >= 96) && (irq < 128)) {
		irq = irq - 96;
		regval32 = __bim_readl(REG_IRQEN4);
		regval32 &= ~(1 << irq);
		__bim_writel(regval32, REG_IRQEN4);
	} else if ((irq >= 128) && (irq < 160)) {
		irq = irq - 128;
		regval32 = __bim_readl(REG_IRQEN5);
		regval32 &= ~(1 << irq);
		__bim_writel(regval32, REG_IRQEN5);
	} else {
		BUG();
	}
	local_irq_restore(flags);
}
EXPORT_SYMBOL(bim_mask_irq);

void mt33xx_mask_bim_irq(unsigned int virq)
{
	return bim_mask_irq(virq);
}
EXPORT_SYMBOL(mt33xx_mask_bim_irq);

/** 
* Disable IRQ and Clear IRQ status
*/
void bim_mask_ack_irq(unsigned int virq)
{
	u32 regval32;
	unsigned long flags;
	unsigned long irq;
	struct irq_data *irq_data;
	
	irq_data = irq_get_irq_data(virq);
	if (!irq_data)
		BUG();

	irq = irq_data->hwirq - VECTOR_GIC_OFFSET;
	local_irq_save(flags);
	if (irq < 32) {
		/* set 1 to clear */
		regval32 = (1 << irq);
		__bim_writel(regval32, REG_IRQCL);
		__bim_writel(regval32, REG_IRQST);
	} else if ((irq >= 32) && (irq < 64)) {
		irq = irq - 32;
		/* set 1 to clear */
		regval32 = (1 << irq);
		__bim_writel(regval32, REG_IRQCL2);
		__bim_writel(regval32, REG_IRQST2);
	} else if ((irq >= 64) && (irq < 96)) {
		irq = irq - 64;
		/* set 1 to clear */
		regval32 = (1 << irq);
		__bim_writel(regval32, REG_IRQCL3);
		__bim_writel(regval32, REG_IRQST3);
	} else if ((irq >= 96) && (irq < 128)) {
		irq = irq - 96;
		/* set 1 to clear */
		regval32 = (1 << irq);
		__bim_writel(regval32, REG_IRQCL4);
		__bim_writel(regval32, REG_IRQST4);
	} else if ((irq >= 128) && (irq < 160)) {
		irq = irq - 128;
		/* set 1 to clear */
		regval32 = (1 << irq);
		__bim_writel(regval32, REG_IRQCL5);
		__bim_writel(regval32, REG_IRQST5);
	} else {
		BUG();
	}
	local_irq_restore(flags);
}
EXPORT_SYMBOL(bim_mask_ack_irq);

void mt33xx_mask_ack_bim_irq(unsigned int virq)
{
	return bim_mask_ack_irq(virq);
}
EXPORT_SYMBOL(mt33xx_mask_ack_bim_irq);

/**
* Enable IRQ
*/
void bim_unmask_irq(unsigned int virq)
{
	u32 regval32;
	unsigned long flags;
	unsigned int irq;
	struct irq_data *irq_data;
	
	irq_data = irq_get_irq_data(virq);
	if (!irq_data)
		BUG();

	irq = irq_data->hwirq - VECTOR_GIC_OFFSET;

	local_irq_save(flags);
	if (irq < 32) {
		regval32 = __bim_readl(REG_IRQEN);
		regval32 |= (1 << irq);
		__bim_writel(regval32, REG_IRQEN);
	} else if ((irq >= 32) && (irq < 64)) {
		irq = irq - 32;
		regval32 = __bim_readl(REG_IRQEN2);
		regval32 |= (1 << irq);
		__bim_writel(regval32, REG_IRQEN2);
	} else if ((irq >= 64) && (irq < 96)) {
		irq = irq - 64;
		regval32 = __bim_readl(REG_IRQEN3);
		regval32 |= (1 << irq);
		__bim_writel(regval32, REG_IRQEN3);
	} else if ((irq >= 96) && (irq < 128)) {
		irq = irq - 96;
		regval32 = __bim_readl(REG_IRQEN4);
		regval32 |= (1 << irq);
		__bim_writel(regval32, REG_IRQEN4);
	} else if ((irq >= 128) && (irq < 160)) {
		irq = irq - 128;
		regval32 = __bim_readl(REG_IRQEN5);
		regval32 |= (1 << irq);
		__bim_writel(regval32, REG_IRQEN5);
	} else {
		BUG();
	}
	local_irq_restore(flags);
}
EXPORT_SYMBOL(bim_unmask_irq);

void mt33xx_unmask_bim_irq(unsigned int virq)
{	
	bim_unmask_irq(virq);
}
EXPORT_SYMBOL(mt33xx_unmask_bim_irq);

u32 bim_ismask_bim_irq(unsigned int virq)
{
	uint32_t regval32;
	unsigned long flags;
	unsigned int irq;
	struct irq_data *irq_data;
	
	irq_data = irq_get_irq_data(virq);
	if (!irq_data)
		BUG();

	irq = irq_data->hwirq - VECTOR_GIC_OFFSET;

	local_irq_save(flags);
	if (irq < 32) {
		regval32 = __bim_readl(REG_IRQEN);
	} else if ((irq >= 32) && (irq < 64)) {
		irq = irq - 32;
		regval32 = __bim_readl(REG_IRQEN2);
	} else if ((irq >= 64) && (irq < 96)) {
		irq = irq - 64;
		regval32 = __bim_readl(REG_IRQEN3);
	} else if ((irq >= 96) && (irq < 128)) {
		irq = irq - 96;
		regval32 = __bim_readl(REG_IRQEN4);
	} else if ((irq >= 128) && (irq < 160)) {
		irq = irq - 128;
		regval32 = __bim_readl(REG_IRQEN5);
	} else {
		BUG();
		while (1);
	}

	if (regval32 & (1 << irq)) {
		local_irq_restore(flags);
		return 0;
	}

	local_irq_restore(flags);
	return 1;
}

u32 mt33xx_ismask_bim_irq(unsigned int virq)
{
	return bim_ismask_bim_irq(virq);
}
EXPORT_SYMBOL(mt33xx_ismask_bim_irq);

u32 bim_pending_bim_irq(unsigned int virq)
{
	uint32_t regval32;
	unsigned long flags;
	unsigned int irq;
	struct irq_data *irq_data;
	
	irq_data = irq_get_irq_data(virq);
	if (!irq_data)
		BUG();

	irq = irq_data->hwirq - VECTOR_GIC_OFFSET;
	local_irq_save(flags);
	if (irq < 32) {
		regval32 = __bim_readl(REG_IRQST);
	} else if ((irq >= 32) && (irq < 64)) {
		irq = irq - 32;
		regval32 = __bim_readl(REG_IRQST2);
	} else if ((irq >= 64) && (irq < 96)) {
		irq = irq - 64;
		regval32 = __bim_readl(REG_IRQST3);
	} else if ((irq >= 96) && (irq < 128)) {
		irq = irq - 96;
		regval32 = __bim_readl(REG_IRQST4);
	} else if ((irq >= 128) && (irq < 160)) {
		irq = irq - 128;
		regval32 = __bim_readl(REG_IRQST5);
	} else {
		BUG();
		while (1);
	}
	local_irq_restore(flags);
	return (regval32 & (1 << irq));
}

u32 mt33xx_pending_bim_irq(unsigned int virq)
{
	return bim_pending_bim_irq(virq);
}
EXPORT_SYMBOL(mt33xx_pending_bim_irq);

static const struct of_device_id bim_of_match[] = {
	{.compatible = "mediatek,mt33xx-bim",},
	{},
};
MODULE_DEVICE_TABLE(of, bim_of_match);

static int bim_probe(struct platform_device *dev)
{
	bim_reg_base = of_iomap(dev->dev.of_node, 0);
	if (bim_reg_base == 0) {
		return -ENODEV;
	}
	
	printk("bim_probe is called\n");
	return 0;
}

static struct platform_driver bim_driver = {
	.probe = bim_probe,
	.remove = NULL,
	.shutdown = NULL,
	.suspend = NULL,
	.resume = NULL,
	.driver = {
		.name = "mt33xx-bim",
		.of_match_table = bim_of_match,
	},
};

static int __init bim_init(void)
{
	int ret;

	ret = platform_driver_register(&bim_driver);

	return 0;
}
arch_initcall(bim_init);

MODULE_AUTHOR("ATC");
MODULE_DESCRIPTION("MT33XX-BIM Device Driver");
MODULE_LICENSE("GPL");
