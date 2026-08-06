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

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#include <linux/gpio.h>
#include <mach/ac83xx_basic.h>
#include <mach/ac83xx_gpio_pinmux.h>
#include <mach/pinmux.h>
#include <mach/gpio_reg.h>

#include <linux/spinlock.h>
#include <asm/delay.h>
#include <mach/ac83xx_pinmux_table.h>
#include <linux/irq.h>
#include <mach/irqs.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/printk.h>

#define IRQ_TYPE_EDGE_BOTH   (IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING)
#define IRQ_TYPE_LEVEL_MASK    (IRQF_TRIGGER_HIGH | IRQF_TRIGGER_LOW)

struct ac83xx_gpio {
	struct device *master;
	struct gpio_chip gpio_chip;
	struct irq_domain *irq_domain;
};

struct ac83xx_gpio_platform_data {
	unsigned gpio_start;
	u8 gpio_en_mask;
	u8 gpio_pullup_mask;
};

typedef struct _intr_source {
	unsigned  int id;
	irq_handler_t handler;
	const char *name;
	void *dev;
} intr_source;

static intr_source g_gpio_intrsource[TOTAL_GPIO_NUM];
bool g_AlreadyReqgpioIrq;
int g_gpiointrgroup;

spinlock_t  gpio_irq_lock;

#define REG_RW_GPIO_INT_GROUP_OFFSET   0x298

#define REG_RW_INT_ED2_OFFSET          0x384
#define REG_RW_INT_LEV_OFFSET          0x390
#define REG_RW_INT_POL_OFFSET          0x39C
#define REG_RW_INT_EN_OFFSET           0x3A8

#define REG_RW_INT_STA_OFFSET          0x3B4

#define GPIO_INT_STA_REG(idx)    CKGEN_READ32(REG_RW_INT_STA_OFFSET+(4*(idx)))
#define GPIO_INT_STA_WRITE(idx, val)    \
	CKGEN_WRITE32((REG_RW_INT_STA_OFFSET+(4*(idx))), (val))

#define GPIO_INT_STA_REG1()  CKGEN_READ32(REG_RW_INT_STA_OFFSET+(4*0))
#define GPIO_INT_STA_REG2()  CKGEN_READ32(REG_RW_INT_STA_OFFSET+(4*1))
#define GPIO_INT_STA_REG3()  CKGEN_READ32(REG_RW_INT_STA_OFFSET+(4*2))

#define GPIO_INT_STA_CLN1()    \
	CKGEN_WRITE32((REG_RW_INT_STA_OFFSET+(4*(0))), (0))
#define GPIO_INT_STA_CLN2()    \
	CKGEN_WRITE32((REG_RW_INT_STA_OFFSET+(4*(1))), (0))
#define GPIO_INT_STA_CLN3()    \
	CKGEN_WRITE32((REG_RW_INT_STA_OFFSET+(4*(2))), (0))

#define GPIO_INT_GROUP_REG()    CKGEN_READ32(REG_RW_GPIO_INT_GROUP_OFFSET)
#define GPIO_INT_GROUP_WRITE(val)    \
	CKGEN_WRITE32(REG_RW_GPIO_INT_GROUP_OFFSET, (val))

#define GPIO_INT_EN_REG(idx)    CKGEN_READ32(REG_RW_INT_EN_OFFSET+(4*(idx)))
#define GPIO_INT_EN_WRITE(idx, val)    \
	CKGEN_WRITE32((REG_RW_INT_EN_OFFSET+(4*(idx))), (val))

#define GPIO_INT_ED2_REG(idx)    CKGEN_READ32(REG_RW_INT_ED2_OFFSET+(4*(idx)))
#define GPIO_INT_ED2_WRITE(idx, val)    \
	CKGEN_WRITE32((REG_RW_INT_ED2_OFFSET+(4*(idx))), (val))

#define GPIO_INT_LEV_REG(idx)    CKGEN_READ32(REG_RW_INT_LEV_OFFSET+(4*(idx)))
#define GPIO_INT_LEV_WRITE(idx, val)    \
	CKGEN_WRITE32((REG_RW_INT_LEV_OFFSET+(4*(idx))), (val))

#define GPIO_INT_POL_REG(idx)    CKGEN_READ32(REG_RW_INT_POL_OFFSET+(4*(idx)))
#define GPIO_INT_POL_WRITE(idx, val)    \
	CKGEN_WRITE32((REG_RW_INT_POL_OFFSET+(4*(idx))), (val))

extern void ac83xx_mask_ack_bim_irq(uint32_t irq);

static inline unsigned gpio_get_addridx(unsigned gpio_pinmux)
{
	return gpio_pinmux/32;
}

static inline unsigned gpio_get_addroffset(unsigned gpio_pinmux)
{
	return gpio_pinmux%32;
}

static inline int valid_gpio(unsigned gpio)
{
	return gpio <= TOTAL_GPIO_NUM;
}

static inline int valid_pinmux(unsigned pinmux_sel, unsigned function)
{
	return ((function >= 0) &&
			(function <= MAX_PINMUX_FUNCTION) &&
			(function <= _au1PinmuxFunctionMasks[pinmux_sel]) &&
			(pinmux_sel >= 0) &&
			(pinmux_sel <= MAX_PINMUX_SEL) &&
			(_au1PinmuxFunctionMasks[pinmux_sel] != 0));
}

/* protects */
static spinlock_t gpio_lock = __SPIN_LOCK_UNLOCKED(gpio_lock);

static int ac83xx_gpio_inout_sel(unsigned gpio, int dir)
{
	int ret = -EINVAL;

	ret = ac83xx_gpio_inout_sel_reg(gpio, dir);

	return ret;
}

int ac83xx_gpio_get_value(struct gpio_chip *chip, unsigned gpio)
{
	struct ac83xx_gpio *dev;

	int val;
	unsigned long flags;

	dev = container_of(chip, struct ac83xx_gpio, gpio_chip);

	spin_lock_irqsave(&gpio_lock, flags);
	val = ac83xx_gpio_get_value_reg(gpio);
	spin_unlock_irqrestore(&gpio_lock, flags);

	return val;

}
EXPORT_SYMBOL(ac83xx_gpio_get_value);

void ac83xx_gpio_set_value(struct gpio_chip *chip,
		unsigned gpio, int value)
{
	int val;
	unsigned long flags;
	struct ac83xx_gpio *dev;
	dev = container_of(chip, struct ac83xx_gpio, gpio_chip);

	spin_lock_irqsave(&gpio_lock, flags);
	val = ac83xx_gpio_set_value_reg(gpio, value);
	spin_unlock_irqrestore(&gpio_lock, flags);
	
}
EXPORT_SYMBOL(ac83xx_gpio_set_value);

int ac83xx_gpio_to_irq(struct gpio_chip *chip, unsigned gpio)
{
	struct ac83xx_gpio *dev;
	dev = container_of(chip, struct ac83xx_gpio, gpio_chip);
	dev_err(dev->master, "ac83xx_gpio_to_irq gpio=%d\n", gpio);
	if (gpio >= TOTAL_GPIO_NUM)
		return -1;

	return irq_create_mapping(dev->irq_domain, gpio);
}
EXPORT_SYMBOL(ac83xx_gpio_to_irq);


int ac83xx_gpio_of_xlate(struct gpio_chip *gc,
		const struct of_phandle_args *gpiospec, u32 *flags)
{
	if (flags)
		*flags = gpiospec->args[1];
	return gpiospec->args[0];
}

int ac83xx_gpio_direction_input(struct gpio_chip *chip, unsigned gpio)
{
	struct ac83xx_gpio *dev;
	dev = container_of(chip, struct ac83xx_gpio, gpio_chip);

	if (likely(valid_gpio(gpio))) {
		int ret = -EINVAL;
		unsigned long flags;

		spin_lock_irqsave(&gpio_lock, flags);
		ret = ac83xx_gpio_inout_sel_reg(gpio, INPUT);
		spin_unlock_irqrestore(&gpio_lock, flags);

		if (ret == 0) {
			ret = ac83xx_gpio_get_value_reg(gpio);
		}

		return ret;
	} else
		return -EINVAL;
}
EXPORT_SYMBOL(ac83xx_gpio_direction_input);

int ac83xx_gpio_direction_output(struct gpio_chip *chip,
		unsigned gpio, int value)
{
	struct ac83xx_gpio *dev;
	int ret = 0;
	unsigned long flags;
	dev = container_of(chip, struct ac83xx_gpio, gpio_chip);

	pr_debug("ac83xx_gpio_direction_output is called  gpio = %d value = %d\n",
				gpio, value);

	if (likely(valid_gpio(gpio))) {
		ret = -EINVAL;

		spin_lock_irqsave(&gpio_lock, flags);
		ret = ac83xx_gpio_set_value_reg(gpio, value);
		ret |= ac83xx_gpio_inout_sel_reg(gpio, OUTPUT);
		spin_unlock_irqrestore(&gpio_lock, flags);

		return ret;
	} else
		return -EINVAL;
}
EXPORT_SYMBOL(ac83xx_gpio_direction_output);

int gpio_inout_sel(unsigned gpio, int dir)
{
	unsigned long flags;
	int ret;
	pr_alert("******************gpio_inout_sel************\n");
	spin_lock_irqsave(&gpio_lock, flags);
	ret = ac83xx_gpio_inout_sel_reg(gpio, dir);
	spin_unlock_irqrestore(&gpio_lock, flags);

	return ret;
}
EXPORT_SYMBOL(gpio_inout_sel);

/*----------------------------------------------------------------------------
 * Function: gpio_verify
 *---------------------------------------------------------------------------*/

int gpio_configure(unsigned gpio, int dir, int value)
{
	if (dir == INPUT) {
		return gpio_direction_input(gpio);
	} else if (dir == OUTPUT) {
		return gpio_direction_output(gpio, value);
	} else
		return -EINVAL;
}
EXPORT_SYMBOL(gpio_configure);

void gpio_verify(unsigned refergpio, unsigned gpio)
{
	/*test input*/
	gpio_configure(gpio, INPUT, 0);

	gpio_configure(refergpio, OUTPUT, 1);
	__delay(100);
	if (ac83xx_gpio_get_value(NULL, gpio) == 1) {
		pr_debug("GPIO %d Input 1 OK!\n", gpio);
	} else {
		pr_debug("GPIO %d Input 1 Fail!!!!!!!!\n", gpio);
	}

	gpio_configure(refergpio, OUTPUT, 0);
	__delay(100);
	if (ac83xx_gpio_get_value(NULL, gpio) == 0) {
		pr_debug("GPIO %d Input 0 OK!\n", gpio);
	} else {
		pr_debug("GPIO %d Input 0 Fail!!!!!!!!\n", gpio);
	}

	/*test output*/
	gpio_configure(refergpio, INPUT, 0);

	gpio_configure(gpio, OUTPUT, 1);
	__delay(100);
	if (ac83xx_gpio_get_value(NULL, refergpio) == 1) {
		pr_debug("GPIO %d Output 1 OK!\n", gpio);
	} else {
		pr_debug("GPIO %d Output 1 Fail!!!!!!!!\n", gpio);
	}

	gpio_configure(gpio, OUTPUT, 0);
	__delay(100);
	if (ac83xx_gpio_get_value(NULL, refergpio) == 0) {
		pr_debug("GPIO %d Output 0 OK!\n", gpio);
	} else {
		pr_debug("GPIO %d Output 0 Fail!!!!!!!!\n", gpio);
	}
}
EXPORT_SYMBOL(gpio_verify);

int bsp_pinset(unsigned pinmux_sel, unsigned function)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&gpio_lock, flags);
	ret = __bsp_pinset(pinmux_sel, function);
	spin_unlock_irqrestore(&gpio_lock, flags);

	return ret;
}
EXPORT_SYMBOL(bsp_pinset);

int bsp_pinget(unsigned pinmux_sel)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&gpio_lock, flags);
	ret = __bsp_pinget(pinmux_sel);
	spin_unlock_irqrestore(&gpio_lock, flags);

	return ret;
}
EXPORT_SYMBOL(bsp_pinget);

int BSP_PinSet(int i4FuncSel, int i4Func)
{
	return bsp_pinset(i4FuncSel, i4Func);
}
EXPORT_SYMBOL(BSP_PinSet);

int BSP_PinGet(int i4FuncSel)
{
	return bsp_pinget(i4FuncSel);
}
EXPORT_SYMBOL(BSP_PinGet);

int gpio_int_read(void)
{
	unsigned val;

	val = GPIO_INT_STA_REG1();
	pr_alert("***************gpio_int_read one :0x%x************\n", val);
	val = GPIO_INT_STA_REG2();
	pr_alert("***************gpio_int_read two :0x%x************\n", val);
	val = GPIO_INT_STA_REG3();
	pr_alert("***************gpio_int_read three :0x%x**********\n", val);

	return val;
}

void gpio_int_clean(void)
{
	GPIO_INT_STA_CLN1();
	pr_alert("******************gpio_int_read one clean************\n");
	GPIO_INT_STA_CLN2();
	pr_alert("******************gpio_int_read two clean************\n");
	GPIO_INT_STA_CLN3();
	pr_alert("******************gpio_int_read three clean************\n");
}

int gpio_int_clear(int i4GpioNum)
{
	unsigned  offset, idx, val;
	if (i4GpioNum >= TOTAL_GPIO_NUM) {
		return -1;
	}
	offset = gpio_get_addroffset((_au1IntSel[i4GpioNum][2]));
	idx = gpio_get_addridx((_au1IntSel[i4GpioNum][2]));

	val = GPIO_INT_STA_REG(idx);
	val = val & ~(1 << offset);
	GPIO_INT_STA_WRITE(idx, val);

	return 0;
}
EXPORT_SYMBOL(gpio_int_clear);

int gpio_int_read_to_gpio(void)
{
	unsigned  offset, idx, val, i;
	bool haveintr = false;
	unsigned intrgroup = 0x0;

	intrgroup = (GPIO_INT_GROUP_REG() & 0x600);

	for (idx = 0; idx < 3; idx++) {
		val = GPIO_INT_STA_REG(idx);
		if (val != 0x0) {
			for (offset = 0; offset < 32; offset++) {
				if (((val >> offset) & 0x1) != 0x0) {
					haveintr = true;
					break;
				}
			}
		}
		if (haveintr == true)
			break;
	}

	if (haveintr == true) {
		offset = idx * 32 +  offset;

		for (i = 0; i < 205; i++) {
			if ((_au1IntSel[i][1] == g_gpiointrgroup)
					&& (_au1IntSel[i][2] == offset))
				return _au1IntSel[i][0];
		}
	}

	return -1;
}
EXPORT_SYMBOL(gpio_int_read_to_gpio);

/********************************************************
**function: set gpio of group as irq
********************************************************/
int set_gpio_of_group_irq(unsigned int gpio)
{
	unsigned int u4GroupVal = 0;

	if (gpio >= (TOTAL_GPIO_NUM))
		return -1;

	GPIO_MultiFun_Set(gpio, PINMUX_LEVEL_GPIO_END_FLAG);
	ac83xx_gpio_inout_sel(gpio, INPUT);
	if (g_AlreadyReqgpioIrq != true) {
		g_gpiointrgroup = _au1IntSel[gpio][1];
		if (_au1IntSel[gpio][1] == 0) {
			u4GroupVal = GPIO_INT_GROUP_REG();
			u4GroupVal = u4GroupVal & 0xfffff9ff;
			GPIO_INT_GROUP_WRITE(u4GroupVal);
		} else if (_au1IntSel[gpio][1] == 1) {
			u4GroupVal = GPIO_INT_GROUP_REG();
			u4GroupVal = u4GroupVal & 0xfffff9ff;
			u4GroupVal = u4GroupVal | 0x200;
			GPIO_INT_GROUP_WRITE(u4GroupVal);
		} else if (_au1IntSel[gpio][1] == 2) {
			u4GroupVal = GPIO_INT_GROUP_REG();
			u4GroupVal = u4GroupVal & 0xfffff9ff;
			u4GroupVal = u4GroupVal | 0x400;
			GPIO_INT_GROUP_WRITE(u4GroupVal);
		}
		g_AlreadyReqgpioIrq = true;
	}
	if ((g_gpiointrgroup == true) &&
			(_au1IntSel[gpio][1] != g_gpiointrgroup)) {
		pr_err("request_gpio_irq:  gpio:%d irq is not in the same group\n",
				gpio);
		return -1;
	}

	return 0;
}
EXPORT_SYMBOL(set_gpio_of_group_irq);

int set_gpio_irq_trigglemode(unsigned int gpio, unsigned int flags)
{
	unsigned int  offset, idx, val;

	if (gpio >= (TOTAL_GPIO_NUM))
		return -1;

	if (((flags & IRQ_TYPE_EDGE_BOTH) && (flags & IRQ_TYPE_LEVEL_MASK))
		|| ((flags & IRQ_TYPE_LEVEL_MASK) == IRQ_TYPE_LEVEL_MASK)) {
		return -2;
	}
	offset = gpio_get_addroffset((_au1IntSel[gpio][2]));
	idx = gpio_get_addridx((_au1IntSel[gpio][2]));

	pr_debug("##############set_gpio_irq_trigglemode\n");
	/*set  triggle mode	*/
	switch (flags & IRQF_TRIGGER_MASK) {
	case IRQF_TRIGGER_RISING:/* 001 */
		val = GPIO_INT_ED2_REG(idx);
		val &= ~(0x01 << offset);
		GPIO_INT_ED2_WRITE(idx, val);

		val = GPIO_INT_LEV_REG(idx);
		val &= ~(0x01 << offset);
		GPIO_INT_LEV_WRITE(idx, val);

		val = GPIO_INT_POL_REG(idx);
		val |= (0x01 << offset);
		GPIO_INT_POL_WRITE(idx, val);
		break;

	case IRQF_TRIGGER_FALLING:/* 000 */
		val = GPIO_INT_ED2_REG(idx);
		val &= ~(0x01 << offset);
		GPIO_INT_ED2_WRITE(idx, val);

		val = GPIO_INT_LEV_REG(idx);
		val &= ~(0x01 << offset);
		GPIO_INT_LEV_WRITE(idx, val);

		val = GPIO_INT_POL_REG(idx);
		val &= ~(0x01 << offset);
		GPIO_INT_POL_WRITE(idx, val);
		break;

	case IRQ_TYPE_EDGE_BOTH:/* 1xx */
		val = GPIO_INT_ED2_REG(idx);
		val |= (0x01 << offset);
		GPIO_INT_ED2_WRITE(idx, val);
		break;

	case IRQF_TRIGGER_HIGH:/* 011 */
		val = GPIO_INT_ED2_REG(idx);
		val &= ~(0x01 << offset);
		GPIO_INT_ED2_WRITE(idx, val);

		val = GPIO_INT_LEV_REG(idx);
		val |= (0x01 << offset);
		GPIO_INT_LEV_WRITE(idx, val);

		val = GPIO_INT_POL_REG(idx);
		val |= (0x01 << offset);
		GPIO_INT_POL_WRITE(idx, val);
		break;

	case IRQF_TRIGGER_LOW:/* 010 */
		val = GPIO_INT_ED2_REG(idx);
		val &= ~(0x01 << offset);
		GPIO_INT_ED2_WRITE(idx, val);

		val = GPIO_INT_LEV_REG(idx);
		val |= (0x01 << offset);
		GPIO_INT_LEV_WRITE(idx, val);

		val = GPIO_INT_POL_REG(idx);
		val &= ~(0x01 << offset);
		GPIO_INT_POL_WRITE(idx, val);
		break;

	default:
		break;
	}

	return 0;
}
EXPORT_SYMBOL(set_gpio_irq_trigglemode);

int set_gpio_irq_enable(unsigned int gpio)
{
	unsigned int offset, idx, val;
	if (gpio >= (TOTAL_GPIO_NUM))
		return -1;

	offset = gpio_get_addroffset((_au1IntSel[gpio][2]));
	idx = gpio_get_addridx((_au1IntSel[gpio][2]));

	val = GPIO_INT_EN_REG(idx);
	val |= (0x01 << offset);
	GPIO_INT_EN_WRITE(idx, val);

	return 0;
}
EXPORT_SYMBOL(set_gpio_irq_enable);

int request_gpio_irq(unsigned int gpio, irq_handler_t handler,
		unsigned long flags, const char *name, void *dev)
{
	unsigned int u4GroupVal = 0;
	unsigned int  offset, idx, val;

	pr_debug("request_gpio_irq:  gpio=%d,dev:0X%x\n",
				gpio, (unsigned int)dev);

	if (gpio >= (TOTAL_GPIO_NUM))
		return -1;

	GPIO_MultiFun_Set(gpio, PINMUX_LEVEL_GPIO_END_FLAG);
	ac83xx_gpio_inout_sel(gpio, INPUT);
	if (g_AlreadyReqgpioIrq != true) {
		g_gpiointrgroup = _au1IntSel[gpio][1];
		if (_au1IntSel[gpio][1] == 0) {
			u4GroupVal = GPIO_INT_GROUP_REG();
			u4GroupVal = u4GroupVal & 0xfffff9ff;
			GPIO_INT_GROUP_WRITE(u4GroupVal);
		} else if (_au1IntSel[gpio][1] == 1) {
			u4GroupVal = GPIO_INT_GROUP_REG();
			u4GroupVal = u4GroupVal & 0xfffff9ff;
			u4GroupVal = u4GroupVal | 0x200;
			GPIO_INT_GROUP_WRITE(u4GroupVal);
		} else if (_au1IntSel[gpio][1] == 2) {
			u4GroupVal = GPIO_INT_GROUP_REG();
			u4GroupVal = u4GroupVal & 0xfffff9ff;
			u4GroupVal = u4GroupVal | 0x400;
			GPIO_INT_GROUP_WRITE(u4GroupVal);
		}
		g_AlreadyReqgpioIrq = true;
	}
	if ((g_gpiointrgroup == true) &&
			(_au1IntSel[gpio][1] != g_gpiointrgroup)) {
		pr_err("request_gpio_irq:  gpio:%d irq is not in the same group\n",
			gpio);
		return -1;
	}

	offset = gpio_get_addroffset((_au1IntSel[gpio][2]));
	idx = gpio_get_addridx((_au1IntSel[gpio][2]));

	/* set  triggle mode */
	switch (flags) {
	case GPIO_IRQTYPE_RISINGEDGE:/* 001 */
		val = GPIO_INT_ED2_REG(idx);
		val &= ~(0x01 << offset);
		GPIO_INT_ED2_WRITE(idx, val);

		val = GPIO_INT_LEV_REG(idx);
		val &= ~(0x01 << offset);
		GPIO_INT_LEV_WRITE(idx, val);

		val = GPIO_INT_POL_REG(idx);
		val |= (0x01 << offset);
		GPIO_INT_POL_WRITE(idx, val);
		break;

	case GPIO_IRQTYPE_FALLINGEDGE:/* 000 */
		val = GPIO_INT_ED2_REG(idx);
		val &= ~(0x01 << offset);
		GPIO_INT_ED2_WRITE(idx, val);

		val = GPIO_INT_LEV_REG(idx);
		val &= ~(0x01 << offset);
		GPIO_INT_LEV_WRITE(idx, val);

		val = GPIO_INT_POL_REG(idx);
		val &= ~(0x01 << offset);
		GPIO_INT_POL_WRITE(idx, val);
		break;

	case GPIO_IRQTYPE_TWOEDGE:/* 1xx */
		val = GPIO_INT_ED2_REG(idx);
		val |= (0x01 << offset);
		GPIO_INT_ED2_WRITE(idx, val);
		break;

	case GPIO_IRQTYPE_HIGHLEVEL:/* 011 */
		val = GPIO_INT_ED2_REG(idx);
		val &= ~(0x01 << offset);
		GPIO_INT_ED2_WRITE(idx, val);

		val = GPIO_INT_LEV_REG(idx);
		val |= (0x01 << offset);
		GPIO_INT_LEV_WRITE(idx, val);

		val = GPIO_INT_POL_REG(idx);
		val |= (0x01 << offset);
		GPIO_INT_POL_WRITE(idx, val);
		break;

	case GPIO_IRQTYPE_LOWLEVEL:/* 010 */
		val = GPIO_INT_ED2_REG(idx);
		val &= ~(0x01 << offset);
		GPIO_INT_ED2_WRITE(idx, val);

		val = GPIO_INT_LEV_REG(idx);
		val |= (0x01 << offset);
		GPIO_INT_LEV_WRITE(idx, val);

		val = GPIO_INT_POL_REG(idx);
		val &= ~(0x01 << offset);
		GPIO_INT_POL_WRITE(idx, val);
		break;

	default:
		break;
	}

	/* enable */
	val = GPIO_INT_EN_REG(idx);
	val |= (0x01 << offset);
	GPIO_INT_EN_WRITE(idx, val);

	g_gpio_intrsource[gpio].id = gpio;
	g_gpio_intrsource[gpio].handler = handler;
	g_gpio_intrsource[gpio].name = name;
	g_gpio_intrsource[gpio].dev = dev;

	return 0;
}
EXPORT_SYMBOL(request_gpio_irq);

/***************************************************************/
static void gpio_eint_irq_handler(unsigned irq, struct irq_desc *desc)
{
	struct irq_chip *chip = irq_get_chip(irq);
	struct ac83xx_gpio *dev = irq_get_handler_data(irq);
	int virq;
	int gpio;

	dev_err(dev->master, "gpio_eint_irq_handler irq=%d\n", irq);

	chained_irq_enter(chip, desc);
	gpio = gpio_int_read_to_gpio();
	virq = irq_find_mapping(dev->irq_domain, gpio);
	dev_err(dev->master, "generic_handle_irq before gpio=%d virq=%d\n",
			gpio, virq);
	generic_handle_irq(virq);
	dev_err(dev->master, "generic_handle_irq after gpio=%d virq=%d\n",
			gpio, virq);
	gpio_int_clear(gpio);
	ac83xx_mask_ack_bim_irq(virq);
	chained_irq_exit(chip, desc);
}

static void gpio_eint_mask(struct irq_data *d)
{
	unsigned gpio;

	irq_data_get_irq_chip_data(d);
	gpio = d->hwirq;
	gpio_int_clear(gpio);
}

static void gpio_eint_unmask(struct irq_data *d)
{
	unsigned gpio;

	irq_data_get_irq_chip_data(d);
	gpio = d->hwirq;
	set_gpio_irq_enable(gpio);
}

static void gpio_eint_ack(struct irq_data *d)
{
	struct ac83xx_gpio *dev = irq_data_get_irq_chip_data(d);
	unsigned gpio = d->hwirq;
	int virq;

	virq = irq_find_mapping(dev->irq_domain, gpio);
	ac83xx_mask_ack_bim_irq(virq);
}

static int gpio_eint_set_type(struct irq_data *d, unsigned int type)
{
	unsigned gpio = d->hwirq;
	int ret;

	ret = set_gpio_irq_trigglemode(gpio, type);
	if (ret != 0) {
		pr_err("Can't configure gpio=%d as eint for type=0x%X\n",
				gpio, type);
		return -1;
	}
	return 0;
}

static int gpio_irq_request_resources(struct irq_data *d)
{
	struct ac83xx_gpio *dev = irq_data_get_irq_chip_data(d);
	unsigned gpio = d->hwirq;
	int ret;

	ret = gpio_lock_as_irq(&(dev->gpio_chip), gpio);
	if (ret) {
		dev_err(dev->master, "unable to lock HW IRQ %lu for IRQ\n",
			irqd_to_hwirq(d));
		return ret;
	}

	ret = set_gpio_of_group_irq(gpio);
	if (ret != 0) {
		dev_err(dev->master, "fail to set gpio of group as irq\n");
		return ret;
	}

	return 0;
}

static void gpio_irq_release_resources(struct irq_data *d)
{
	struct ac83xx_gpio *dev = irq_data_get_irq_chip_data(d);
	unsigned gpio = d->hwirq;

	gpio_unlock_as_irq(&(dev->gpio_chip), gpio);
}

static struct irq_chip gpio_eint_irq_chip = {
	.name = "gpio-eint",
	.irq_mask = gpio_eint_mask,
	.irq_unmask = gpio_eint_unmask,
	.irq_ack = gpio_eint_ack,
	.irq_set_type = gpio_eint_set_type,
	.irq_request_resources = gpio_irq_request_resources,
	.irq_release_resources = gpio_irq_release_resources,
};

/***************************************************************/
static int  ac83xx_gpio_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct ac83xx_gpio *dev;
	struct gpio_chip *gc;
	int ret, i;

	pr_alert("******************ac83xx_gpio_probe ************\n");

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);

	if (dev == NULL) {
		dev_err(&pdev->dev, "failed to alloc memory\n");
		return -ENOMEM;
	}

	gc = &(dev->gpio_chip);
	gc->of_node = pdev->dev.of_node;
	gc->dev = &(pdev->dev);
	gc->of_gpio_n_cells = 2;
	gc->direction_input  = ac83xx_gpio_direction_input;
	gc->direction_output = ac83xx_gpio_direction_output;
	gc->get = ac83xx_gpio_get_value;
	gc->set = ac83xx_gpio_set_value;
	gc->to_irq = ac83xx_gpio_to_irq;
	gc->of_xlate = ac83xx_gpio_of_xlate;

	gc->base = 0;
	gc->ngpio = 202;
	gc->owner = THIS_MODULE;

	ret = gpiochip_add(&dev->gpio_chip);
	if (ret)
		goto err;

	#if 1

	dev->irq_domain = irq_domain_add_linear(np,
			202, &irq_domain_simple_ops, NULL);/*init domain*/
	if (!dev->irq_domain) {
		dev_err(&pdev->dev, "Couldn't register IRQ domain\n");
		ret = -ENOMEM;
		goto err;
	}

	for (i = 0; i < 202; i++) {
		int virq = irq_create_mapping(dev->irq_domain, i);

		irq_set_chip_and_handler(virq, &gpio_eint_irq_chip,
			handle_level_irq);

		irq_set_chip_data(virq, dev);
		set_irq_flags(virq, IRQF_VALID);
	};
	irq_set_chained_handler(82, gpio_eint_irq_handler);

	irq_set_handler_data(82, dev);
	set_irq_flags(82, IRQF_VALID);

	#else
	gpio_int_read();
	ret = request_irq((unsigned int)82, gpio_irq, 0, "gpio", NULL);
	if (ret) {
		pr_err("[MSDC0] ---> eint4 failed\n");
	}
	#endif

	platform_set_drvdata(pdev, dev);
	return 0;

err:
	kfree(dev);
	return ret;
}

static int ac83xx_gpio_remove(struct platform_device *pdev)
{
	struct ac83xx_gpio *dev;

	dev = platform_get_drvdata(pdev);
	gpiochip_remove(&dev->gpio_chip);

	kfree(dev);
	return 0;
}
static int  ac83xx_gpio_suspend(struct platform_device *pdev)
{
    pr_debug("******************ac83xx_gpio_suspend enter ************\n");
	g_AlreadyReqgpioIrq = false;
	return 0;
}


static int  ac83xx_gpio_resume(struct platform_device *pdev)
{
    pr_debug("******************ac83xx_gpio_resume enter ************\n");
	return 0;
}

#if 0
static struct platform_driver ac83xx_gpio_driver = {
	.driver	= {
		.name	= "ac83xx_gpio",
		.owner	= THIS_MODULE,
	},
	.probe		= ac83xx_gpio_probe,
	.remove		= ac83xx_gpio_remove,
	.suspend    = ac83xx_gpio_suspend,
	.resume     = ac83xx_gpio_resume,
};
#else
static const struct of_device_id gpio_of_ids[] = {
		{ .compatible = "Autochips,ac83xx-gpio", },
		{}
};

static struct platform_driver ac83xx_gpio_driver = {
	.probe		= ac83xx_gpio_probe,
	.remove		= ac83xx_gpio_remove,
	.suspend    = ac83xx_gpio_suspend,
	.resume     = ac83xx_gpio_resume,
	.driver		= {
		.name	= "ac83xx_gpio",
		.owner	= THIS_MODULE,
		.of_match_table = gpio_of_ids,
	},
};

static int ac83xx_gpio_init(void)
{
	int ret;
	pr_debug("ac83xx_gpio_init : start\n");
	ret = platform_driver_register(&ac83xx_gpio_driver);
	if (ret)
		pr_err("gpio register failed\n");
	pr_debug("gpio register ok\n");
	return 0;
}
#endif

subsys_initcall(ac83xx_gpio_init);

MODULE_AUTHOR("Michael Hennerich <hennerich@blackfin.uclinux.org>");
MODULE_DESCRIPTION("GPIO ADP5520 Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:adp5520-gpio");
