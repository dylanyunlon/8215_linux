/*
 * MUSB OTG controller driver for Blackfin Processors
 *
 * Copyright 2006-2008 Analog Devices Inc.
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Licensed under the GPL-2 or later.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/io.h>
#include <linux/switch.h>
#include <linux/i2c.h>
#include <mach/irqs.h>
#include "musb_core.h"
#include "atc_musb.h"
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include "musbhsdma.h"
#ifdef CONFIG_ATC_CLKMGR
#include <mach/ac_clkmgr.h>
#else
#include <linux/clk.h>
struct clk *h_musb_clk;
#endif
#include "usb20.h"
#include <linux/delay.h>
#ifdef CONFIG_OF
#include <linux/of_irq.h>
#include <linux/of_address.h>
#ifndef CONFIG_ATC_LEGACY
#include <linux/regulator/consumer.h>
#endif
#endif

#include <mach/ac83xx_pinmux_table.h>
#include <linux/gpio.h>
#include <mach/ac83xx_gpio_pinmux_mapping.h>
#include <mach/pinmux.h>

#ifdef MUSB_QMU_SUPPORT
#include "musb_qmu.h"
#endif

static DEFINE_SEMAPHORE(power_clock_lock);
/* static bool platform_init_first = true; */
#ifndef CONFIG_ATC_LEGACY
static struct regulator *reg;
#endif
/* add for linux kernel 3.10 */


#ifdef CONFIG_OF
static unsigned long usb_phy_base;
#endif

struct gpio_desc *usb1power = NULL;
static bool is_set_default_vale = false;
static u32 power_on_value;


static int usb_power_state = 1;
#ifdef CONFIG_ATC_UART_USB_SWITCH
u32 port_mode = PORT_MODE_USB;
u32 sw_tx = 0;
u32 sw_rx = 0;
u32 sw_uart_path = 0;
#define AP_UART0_COMPATIBLE_NAME "autochip,AP_UART0"
void __iomem *ap_uart0_base;
#endif

/*EP Fifo Config*/
static struct musb_fifo_cfg fifo_cfg[] __initdata = {
	{.hw_ep_num = 1, .style = MUSB_FIFO_TX, .maxpacket = 512, .ep_mode = EP_BULK, .mode =
	 MUSB_BUF_DOUBLE},
	{.hw_ep_num = 1, .style = MUSB_FIFO_RX, .maxpacket = 512, .ep_mode = EP_BULK, .mode =
	 MUSB_BUF_DOUBLE},
	{.hw_ep_num = 2, .style = MUSB_FIFO_TX, .maxpacket = 512, .ep_mode = EP_BULK, .mode =
	 MUSB_BUF_DOUBLE},
	{.hw_ep_num = 2, .style = MUSB_FIFO_RX, .maxpacket = 512, .ep_mode = EP_BULK, .mode =
	 MUSB_BUF_DOUBLE},
	{.hw_ep_num = 3, .style = MUSB_FIFO_TX, .maxpacket = 512, .ep_mode = EP_BULK, .mode =
	 MUSB_BUF_DOUBLE},
	{.hw_ep_num = 3, .style = MUSB_FIFO_RX, .maxpacket = 512, .ep_mode = EP_BULK, .mode =
	 MUSB_BUF_DOUBLE},
	{.hw_ep_num = 4, .style = MUSB_FIFO_TX, .maxpacket = 512, .ep_mode = EP_BULK, .mode =
	 MUSB_BUF_DOUBLE},
	{.hw_ep_num = 4, .style = MUSB_FIFO_RX, .maxpacket = 512, .ep_mode = EP_BULK, .mode =
	 MUSB_BUF_DOUBLE},
	{.hw_ep_num = 5, .style = MUSB_FIFO_TX, .maxpacket = 512, .ep_mode = EP_INT, .mode =
	 MUSB_BUF_SINGLE},
	{.hw_ep_num = 5, .style = MUSB_FIFO_RX, .maxpacket = 512, .ep_mode = EP_INT, .mode =
	 MUSB_BUF_SINGLE},
	{.hw_ep_num = 6, .style = MUSB_FIFO_TX, .maxpacket = 512, .ep_mode = EP_INT, .mode =
	 MUSB_BUF_SINGLE},
	{.hw_ep_num = 6, .style = MUSB_FIFO_RX, .maxpacket = 512, .ep_mode = EP_INT, .mode =
	 MUSB_BUF_SINGLE},
	{.hw_ep_num = 7, .style = MUSB_FIFO_TX, .maxpacket = 512, .ep_mode = EP_BULK, .mode =
	 MUSB_BUF_SINGLE},
	{.hw_ep_num = 7, .style = MUSB_FIFO_RX, .maxpacket = 512, .ep_mode = EP_BULK, .mode =
	 MUSB_BUF_SINGLE},
#if 0
	{.hw_ep_num = 8, .style = MUSB_FIFO_TX, .maxpacket = 512, .ep_mode = EP_ISO, .mode =
	 MUSB_BUF_DOUBLE},
	{.hw_ep_num = 8, .style = MUSB_FIFO_RX, .maxpacket = 512, .ep_mode = EP_ISO, .mode =
	 MUSB_BUF_DOUBLE},
#endif
};

/*=======================================================================*/
/* AC8317 USB GADGET                                                     */
/*=======================================================================*/
#ifdef CONFIG_OF
static const struct of_device_id apusb_of_ids[] = {
	{.compatible = "autochip,USB1",},
	{},
};

static struct platform_device ac_usb_device = {
	.name = "ac_usbh",
	.id = -1,
};

#endif
MODULE_DEVICE_TABLE(of, apusb_of_ids);

#ifndef FPGA_PLATFORM
#ifdef CONFIG_ARCH_MT6735
#include <mach/ac_vcore_dvfs.h>
static int vcore_releasing;
static struct workqueue_struct *vcore_wq;
static struct work_struct vcore_work;

void vcore_hold(void)
{
	int vcore_ret;

	DBG(0, "before releasing\n");
	while (vcore_releasing)
		;
	DBG(0, "after releasing\n");

	vcore_ret = vcorefs_request_dvfs_opp(KIR_USB, OPP_0);
	if (vcore_ret)
		DBG(0, "hold VCORE fail (%d)\n", vcore_ret);
	else
		DBG(0, "hold VCORE ok\n");

}

void vcore_workqueue(struct work_struct *work)
{
	int vcore_ret;

	vcore_ret = vcorefs_request_dvfs_opp(KIR_USB, OPP_OFF);
	if (vcore_ret)
		DBG(0, "workqueue release VCORE fail(%d)\n", vcore_ret);
	else
		DBG(0, "workqueue release VCORE ok\n");
}

void vcore_release(void)
{
	int vcore_ret;
	int lock_ret;
	unsigned long flags;

	vcore_releasing = 1;

	if (h_atc_musb) {
		lock_ret = spin_trylock_irqsave(&h_atc_musb->lock, flags);
		if (!lock_ret) {
			DBG(0, "musb lock fail, h_atc_musb:%p\n", h_atc_musb);
			/* lock fail, spin lock case, should schedule work */
			queue_work(vcore_wq, &vcore_work);
			vcore_releasing = 0;
			return;
		}
		DBG(0, "musb lock get, release it, h_atc_musb:%p\n", h_atc_musb);
		spin_unlock_irqrestore(&h_atc_musb->lock, flags);
	}
	vcore_ret = vcorefs_request_dvfs_opp(KIR_USB, OPP_OFF);
	if (vcore_ret)
		DBG(0, "release VCORE fail(%d)\n", vcore_ret);
	else
		DBG(0, "release VCORE ok\n");

	vcore_releasing = 0;
}
#endif
#endif

static struct timer_list musb_idle_timer;

static void musb_do_idle(unsigned long _musb)
{
	struct musb *musb = (void *)_musb;
	unsigned long flags;
	u8 devctl;

	if (musb->is_active) {
		return;
	}

	spin_lock_irqsave(&musb->lock, flags);
	devctl = musb_readb(musb->mregs, MUSB_DEVCTL);
	spin_unlock_irqrestore(&musb->lock, flags);

}

static void ac_usb_try_idle(struct musb *musb, unsigned long timeout)
{
	unsigned long default_timeout = jiffies + msecs_to_jiffies(3);
	static unsigned long last_timer;

	if (timeout == 0)
		timeout = default_timeout;

	if (time_after(last_timer, timeout)) {
		if (!timer_pending(&musb_idle_timer))
			last_timer = timeout;
		else {
			DBG(2, "Longer idle timer already pending, ignoring\n");
			return;
		}
	}
	last_timer = timeout;
	mod_timer(&musb_idle_timer, timeout);
}

static int real_enable = 0, real_disable;
static int virt_enable = 0, virt_disable;
static void ac_usb_enable(struct musb *musb)
{
	unsigned long flags;

	virt_enable++;
	DBG(0, "<%d,%d>,<%d,%d,%d,%d>\n", h_atc_usb_power, musb->power, virt_enable, virt_disable,
	    real_enable, real_disable);

	if (musb->power == true)
		return;

	flags = musb_readl(musb->mregs, USB_L1INTM);

	/* mask ID pin, so "open clock" and "set flag" won't be interrupted. ISR may call clock_disable. */
	musb_writel(h_atc_musb->mregs, USB_L1INTM, (~IDDIG_INT_STATUS) & flags);

	/* Mark by ALPS01262215
	   if (platform_init_first) {
	   DBG(0,"usb init first\n\r");
	   musb->is_host = true;
	   } */

	if (!h_atc_usb_power) {
		if (down_interruptible(&power_clock_lock))
			DBG(0, "USB20: %s: busy, Couldn't get power_clock_lock\n", __func__);

#ifndef FPGA_PLATFORM
#ifdef CONFIG_ARCH_MT6735
		/* enable_pll(UNIVPLL, "USB_PLL"); */
		DBG(0, "enable UPLL before connect\n");
		vcore_hold();
#endif
#endif
		mdelay(10);

		h_usb_phy_recover();

		h_atc_usb_power = true;
		real_enable++;
		if (in_interrupt()) {
			DBG(0, "in interrupt !!!!!!!!!!!!!!!\n");
			DBG(0, "in interrupt !!!!!!!!!!!!!!!\n");
			DBG(0, "in interrupt !!!!!!!!!!!!!!!\n");
		}
		DBG(0, "<%d,%d,%d,%d>\n", virt_enable, virt_disable, real_enable, real_disable);

		up(&power_clock_lock);
	}
	musb->power = true;

	musb_writel(h_atc_musb->mregs, USB_L1INTM, flags);
}

static void ac_usb_disable(struct musb *musb)
{
	virt_disable++;
	DBG(0, "<%d,%d>,<%d,%d,%d,%d>\n", h_atc_usb_power, musb->power, virt_enable, virt_disable,
	    real_enable, real_disable);

	if (musb->power == false)
		return;

	/* Mark by ALPS01262215
	   if (platform_init_first) {
	   DBG(0,"usb init first\n\r");
	   musb->is_host = false;
	   platform_init_first = false;
	   } */

	if (h_atc_usb_power) {
		if (down_interruptible(&power_clock_lock))
			DBG(0, "USB20: %s: busy, Couldn't get power_clock_lock\n", __func__);

		h_atc_usb_power = false;
		real_disable++;
		DBG(0, "<%d,%d,%d,%d>\n", virt_enable, virt_disable, real_enable, real_disable);

		up(&power_clock_lock);
	}

	musb->power = false;
}

/*-------------------------------------------------------------------------*/
static irqreturn_t generic_interrupt(int irq, void *__hci)
{
	unsigned long flags;
	irqreturn_t retval = IRQ_NONE;
	struct musb *musb = __hci;
	u32 usb_0, usb_4, usb_8;

	spin_lock_irqsave(&musb->lock, flags);

	usb_0 = musb_readl(musb->mregs, MUSB_FADDR);
	usb_4 = musb_readl(musb->mregs, MUSB_INTRRX);
	usb_8 = musb_readl(musb->mregs, MUSB_INTRRXE);
	musb->int_usb = ((usb_8 >> 16) & 0xff) & ((usb_8 >> 24) & 0xff);
	musb->int_tx = ((usb_0 >> 16) & 0xffff) & ((usb_4 >> 16) & 0xffff);
	musb->int_rx = (usb_4 & 0xffff) & (usb_8 & 0xffff);
#ifdef MUSB_QMU_SUPPORT
	musb->int_queue = musb_readl(musb->mregs, MUSB_QISAR);
#endif
	mb();
	usb_0 = (usb_0 & 0xffff) | (musb->int_tx << 16);
	usb_4 = (usb_4 & 0xffff0000) | musb->int_rx;
	usb_8 = (usb_8 & 0xff00ffff) | (musb->int_usb << 16);
	musb_writel(musb->mregs, MUSB_FADDR, usb_0);
	musb_writel(musb->mregs, MUSB_INTRRX, usb_4);
	musb_writel(musb->mregs, MUSB_INTRRXE, usb_8);

#ifdef MUSB_QMU_SUPPORT
	if (musb->int_queue) {
		musb_writel(musb->mregs, MUSB_QISAR, musb->int_queue);
		musb->int_queue &= ~(musb_readl(musb->mregs, MUSB_QIMR));
	}
#endif
	/* musb_read_clear_generic_interrupt */

#ifdef MUSB_QMU_SUPPORT
	if (musb->int_usb || musb->int_tx || musb->int_rx || musb->int_queue)
		retval = h_musb_interrupt(musb);
#else
	if (musb->int_usb || musb->int_tx || musb->int_rx)
		retval = h_musb_interrupt(musb);
#endif

	spin_unlock_irqrestore(&musb->lock, flags);

	return retval;
}

static irqreturn_t ac_usb_interrupt(int irq, void *dev_id)
{
	irqreturn_t tmp_status;
	irqreturn_t status = IRQ_NONE;
	struct musb *musb = (struct musb *)dev_id;
	u32 usb_l1_ints;

	usb_l1_ints = musb_readl(musb->mregs, USB_L1INTS) & musb_readl(h_atc_musb->mregs, USB_L1INTM);
	DBG(1, "usb interrupt assert %x %x  %x %x %x\n", usb_l1_ints,
	    musb_readl(h_atc_musb->mregs, USB_L1INTM), musb_readb(musb->mregs, MUSB_INTRUSBE),
	    musb_readw(musb->mregs, MUSB_INTRTX), musb_readw(musb->mregs, MUSB_INTRTXE));

	if ((usb_l1_ints & TX_INT_STATUS) || (usb_l1_ints & RX_INT_STATUS)
	    || (usb_l1_ints & USBCOM_INT_STATUS)
#ifdef MUSB_QMU_SUPPORT
	    || (usb_l1_ints & QINT_STATUS)
#endif
	   ) {
		tmp_status = generic_interrupt(irq, musb);
		if (tmp_status != IRQ_NONE)
			status = tmp_status;
	}

	/* FIXME, workaround for device_qmu + host_dma */
#if 1
/* #ifndef MUSB_QMU_SUPPORT */
	if (usb_l1_ints & DMA_INT_STATUS) {
		tmp_status = h_dma_controller_irq(irq, musb->dma_controller);
		if (tmp_status != IRQ_NONE)
			status = tmp_status;
	}
#endif

#ifdef	CONFIG_USB_ATC_OTG
	if (usb_l1_ints & IDDIG_INT_STATUS) {
		h_ac_usb_iddig_int(musb);
		status = IRQ_HANDLED;
	}
#endif

	return status;

}

#ifdef CONFIG_ATC_UART_USB_SWITCH
static void uart_usb_switch_dump_register(void)
{
#ifdef FPGA_PLATFORM
	DBG(0, "[MUSB]addr: 0x6B, value: %x\n", USB_PHY_Read_Register8(0x6B));
	DBG(0, "[MUSB]addr: 0x6E, value: %x\n", USB_PHY_Read_Register8(0x6E));
	DBG(0, "[MUSB]addr: 0x22, value: %x\n", USB_PHY_Read_Register8(0x22));
	DBG(0, "[MUSB]addr: 0x68, value: %x\n", USB_PHY_Read_Register8(0x68));
	DBG(0, "[MUSB]addr: 0x6A, value: %x\n", USB_PHY_Read_Register8(0x6A));
	DBG(0, "[MUSB]addr: 0x1A, value: %x\n", USB_PHY_Read_Register8(0x1A));
#else
	DBG(0, "[MUSB]addr: 0x6B, value: %x\n", USBPHY_READ8(0x6B));
	DBG(0, "[MUSB]addr: 0x6E, value: %x\n", USBPHY_READ8(0x6E));
	DBG(0, "[MUSB]addr: 0x22, value: %x\n", USBPHY_READ8(0x22));
	DBG(0, "[MUSB]addr: 0x68, value: %x\n", USBPHY_READ8(0x68));
	DBG(0, "[MUSB]addr: 0x6A, value: %x\n", USBPHY_READ8(0x6A));
	DBG(0, "[MUSB]addr: 0x1A, value: %x\n", USBPHY_READ8(0x1A));
#endif
	DBG(0, "[MUSB]addr: 0x110020B0 (UART0), value: %x\n\n", DRV_Reg8(ap_uart0_base + 0xB0));
}

static ssize_t ac_usb_show_portmode(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (!dev) {
		DBG(0, "dev is null!!\n");
		return 0;
	}

	if (usb_phy_check_in_uart_mode())
		port_mode = PORT_MODE_UART;
	else
		port_mode = PORT_MODE_USB;

	if (port_mode == PORT_MODE_USB)
		DBG(0, "\nUSB Port mode -> USB\n");
	else if (port_mode == PORT_MODE_UART)
		DBG(0, "\nUSB Port mode -> UART\n");
	uart_usb_switch_dump_register();

	return scnprintf(buf, PAGE_SIZE, "%d\n", port_mode);
}

static ssize_t ac_usb_store_portmode(struct device *dev, struct device_attribute *attr,
				     const char *buf, size_t count)
{
	unsigned int portmode;

	if (!dev) {
		DBG(0, "dev is null!!\n");
		return count;
	/* } else if (1 == sscanf(buf, "%d", &portmode)) { */
	} else if (kstrtol(buf, 10, &portmode) == 0) {
		DBG(0, "\nUSB Port mode: current => %d (port_mode), change to => %d (portmode)\n",
		    port_mode, portmode);
		if (portmode >= PORT_MODE_MAX)
			portmode = PORT_MODE_USB;

		if (port_mode != portmode) {
			if (portmode == PORT_MODE_USB) {	/* Changing to USB Mode */
				DBG(0, "USB Port mode -> USB\n");
				usb_phy_switch_to_usb();
			} else if (portmode == PORT_MODE_UART) {	/* Changing to UART Mode */
				DBG(0, "USB Port mode -> UART\n");
				usb_phy_switch_to_uart();
			}
			uart_usb_switch_dump_register();
			port_mode = portmode;
		}
	}
	return count;
}

DEVICE_ATTR(portmode, 0664, ac_usb_show_portmode, ac_usb_store_portmode);


static ssize_t ac_usb_show_tx(struct device *dev, struct device_attribute *attr, char *buf)
{
	UINT8 var;
	UINT8 var2;

	if (!dev) {
		DBG(0, "dev is null!!\n");
		return 0;
	}
#ifdef FPGA_PLATFORM
	var = USB_PHY_Read_Register8(0x6E);
#else
	var = USBPHY_READ8(0x6E);
#endif
	var2 = (var >> 3) & ~0xFE;
	DBG(0, "[MUSB]addr: 0x6E (TX), value: %x - %x\n", var, var2);

	sw_tx = var;

	return scnprintf(buf, PAGE_SIZE, "%x\n", var2);
}

static ssize_t ac_usb_store_tx(struct device *dev, struct device_attribute *attr,
			       const char *buf, size_t count)
{
	unsigned int val;
	UINT8 var;
	UINT8 var2;

	if (!dev) {
		DBG(0, "dev is null!!\n");
		return count;
	/* } else if (1 == sscanf(buf, "%d", &val)) { */
	} else if (kstrtol(buf, 10, &val) == 0) {
		DBG(0, "\n Write TX : %d\n", val);

#ifdef FPGA_PLATFORM
		var = USB_PHY_Read_Register8(0x6E);
#else
		var = USBPHY_READ8(0x6E);
#endif

		if (val == 0)
			var2 = var & ~(1 << 3);
		else
			var2 = var | (1 << 3);

#ifdef FPGA_PLATFORM
		USB_PHY_Write_Register8(var2, 0x6E);
		var = USB_PHY_Read_Register8(0x6E);
#else
		USBPHY_WRITE8(0x6E, var2);
		var = USBPHY_READ8(0x6E);
#endif

		var2 = (var >> 3) & ~0xFE;

		DBG(0, "[MUSB]addr: 0x6E TX [AFTER WRITE], value after: %x - %x\n", var, var2);
		sw_tx = var;
	}
	return count;
}

DEVICE_ATTR(tx, 0664, ac_usb_show_tx, ac_usb_store_tx);

static ssize_t ac_usb_show_rx(struct device *dev, struct device_attribute *attr, char *buf)
{
	UINT8 var;
	UINT8 var2;

	if (!dev) {
		DBG(0, "dev is null!!\n");
		return 0;
	}
#ifdef FPGA_PLATFORM
	var = USB_PHY_Read_Register8(0x77);
#else
	var = USBPHY_READ8(0x77);
#endif
	var2 = (var >> 7) & ~0xFE;
	DBG(0, "[MUSB]addr: 0x77 (RX), value: %x - %x\n", var, var2);
	sw_rx = var;

	return scnprintf(buf, PAGE_SIZE, "%x\n", var2);
}

DEVICE_ATTR(rx, 0444, ac_usb_show_rx, NULL);

static ssize_t ac_usb_show_uart_path(struct device *dev, struct device_attribute *attr, char *buf)
{
	UINT8 var;

	if (!dev) {
		DBG(0, "dev is null!!\n");
		return 0;
	}

	var = DRV_Reg8(ap_uart0_base + 0xB0);
	DBG(0, "[MUSB]addr: (UART0) 0xB0, value: %x\n\n", DRV_Reg8(ap_uart0_base + 0xB0));
	sw_uart_path = var;

	return scnprintf(buf, PAGE_SIZE, "%x\n", var);
}

DEVICE_ATTR(uartpath, 0444, ac_usb_show_uart_path, NULL);
#endif

#ifdef FPGA_PLATFORM
static struct i2c_client *usb_i2c_client;
static const struct i2c_device_id usb_i2c_id[] = { {"atc-usb", 0}, {} };

static struct i2c_board_info usb_i2c_dev __initdata = { I2C_BOARD_INFO("atc-usb", 0x60) };


void USB_PHY_Write_Register8(UINT8 var, UINT8 addr)
{
	char buffer[2];

	buffer[0] = addr;
	buffer[1] = var;
	i2c_master_send(usb_i2c_client, buffer, 2);
}

UINT8 USB_PHY_Read_Register8(UINT8 addr)
{
	UINT8 var;

	i2c_master_send(usb_i2c_client, &addr, 1);
	i2c_master_recv(usb_i2c_client, &var, 1);
	return var;
}

static int usb_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
#ifdef CONFIG_OF
	unsigned long base;
	/* if i2c probe before musb prob, this would cause KE */
	base = usb_phy_base;
	DBG(0, "[MUSB]usb_i2c_probe, start, base:%lx\n", base);
#endif
	usb_i2c_client = client;

#ifdef CONFIG_OF
	/* disable usb mac suspend */
	DRV_WriteReg8(base + 0x86a, 0x00);
#else
	DRV_WriteReg8(USB_SIF_BASE + 0x86a, 0x00);
#endif
	/* usb phy initial sequence */
	USB_PHY_Write_Register8(0x00, 0xFF);
	USB_PHY_Write_Register8(0x04, 0x61);
	USB_PHY_Write_Register8(0x00, 0x68);
	USB_PHY_Write_Register8(0x00, 0x6a);
	USB_PHY_Write_Register8(0x6e, 0x00);
	USB_PHY_Write_Register8(0x0c, 0x1b);
	USB_PHY_Write_Register8(0x44, 0x08);
	USB_PHY_Write_Register8(0x55, 0x11);
	USB_PHY_Write_Register8(0x68, 0x1a);


	DBG(0, "[MUSB]addr: 0xFF, value: %x\n", USB_PHY_Read_Register8(0xFF));
	DBG(0, "[MUSB]addr: 0x61, value: %x\n", USB_PHY_Read_Register8(0x61));
	DBG(0, "[MUSB]addr: 0x68, value: %x\n", USB_PHY_Read_Register8(0x68));
	DBG(0, "[MUSB]addr: 0x6a, value: %x\n", USB_PHY_Read_Register8(0x6a));
	DBG(0, "[MUSB]addr: 0x00, value: %x\n", USB_PHY_Read_Register8(0x00));
	DBG(0, "[MUSB]addr: 0x1b, value: %x\n", USB_PHY_Read_Register8(0x1b));
	DBG(0, "[MUSB]addr: 0x08, value: %x\n", USB_PHY_Read_Register8(0x08));
	DBG(0, "[MUSB]addr: 0x11, value: %x\n", USB_PHY_Read_Register8(0x11));
	DBG(0, "[MUSB]addr: 0x1a, value: %x\n", USB_PHY_Read_Register8(0x1a));


	DBG(0, "[MUSB]usb_i2c_probe, end\n");
	return 0;

}

/*static int usb_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {
    strcpy(info->type, "atc-usb");
    return 0;
}*/

static int usb_i2c_remove(struct i2c_client *client)
{
	return 0;
}


struct i2c_driver usb_i2c_driver = {
	.probe = usb_i2c_probe,
	.remove = usb_i2c_remove,
	/*.detect = usb_i2c_detect, */
	.driver = {
		   .name = "atc-usb",
		   },
	.id_table = usb_i2c_id,
};

static int add_usb_i2c_driver(void)
{
	i2c_register_board_info(2, &usb_i2c_dev, 1);

	if (i2c_add_driver(&usb_i2c_driver) != 0) {
		DBG(0, "[MUSB]usb_i2c_driver initialization failed!!\n");
		return -1;
	}
	DBG(0, "[MUSB]usb_i2c_driver initialization succeed!!\n");
	return 0;
}
#endif				/* End of FPGA_PLATFORM */

void h_usb_clock_enable(void)
{
	unsigned int val;

	val = readl((void __iomem *)0xfd000284);
	val &= ~0x1;
	writel(val, (void __iomem *)0xfd000284);
	val = readl((void __iomem *)0xfd0000a0);
	val |= (1<<13);
	writel(val, (void __iomem *)0xfd0000a0);
}

void atc_h_musb_int_init(struct musb *musb)
{
	musb_writel(musb->mregs, MUSB_HSDMA_INTR, 0xff | (0xff << DMA_INTR_UNMASK_SET_OFFSET));
	DBG(0, "musb platform init %x\n", musb_readl(musb->mregs, MUSB_HSDMA_INTR));

#ifdef MUSB_QMU_SUPPORT
	/* FIXME, workaround for device_qmu + host_dma */
	musb_writel(musb->mregs, USB_L1INTM,
		TX_INT_STATUS | RX_INT_STATUS | USBCOM_INT_STATUS | DMA_INT_STATUS |
		QINT_STATUS);
#else
	musb_writel(musb->mregs, USB_L1INTM,
		TX_INT_STATUS | RX_INT_STATUS | USBCOM_INT_STATUS | DMA_INT_STATUS);
#endif
}

static int ac_usb_init(struct musb *musb)
{
#ifndef CONFIG_ATC_LEGACY
	int ret;
#endif
	DBG(0, "ac_usb_init\n");

#ifdef CONFIG_OF
	/* musb->nIrq = usb_irq_number1; */
#else
	musb->nIrq = USB_MCU_IRQ_BIT1_ID;
#endif
	musb->dma_irq = (int)SHARE_IRQ;
	musb->fifo_cfg = fifo_cfg;
	musb->fifo_cfg_size = ARRAY_SIZE(fifo_cfg);
	musb->dyn_fifo = true;
	musb->power = false;
	musb->is_host = false;
	musb->fifo_size = 6 * 1024;

	/* wake_lock_init(&musb->usb_lock, WAKE_LOCK_SUSPEND, "USB suspend lock"); */

#ifndef FPGA_PLATFORM
#ifdef CONFIG_ARCH_MT6735
	INIT_WORK(&vcore_work, vcore_workqueue);
	vcore_wq = create_freezable_workqueue("usb20_vcore_work");
#endif
#endif

#ifndef FPGA_PLATFORM
#ifdef CONFIG_ATC_LEGACY
	DBG(0, "enable VBUS LDO\n");
#else
	reg = regulator_get(musb->controller, "VUSB33");
	if (!IS_ERR(reg)) {
#define	VUSB33_VOL_MIN 3300000
#define	VUSB33_VOL_MAX 3300000
		ret = regulator_set_voltage(reg, VUSB33_VOL_MIN, VUSB33_VOL_MAX);
		if (ret < 0)
			DBG(0, "regulator set vol failed: %d\n", ret);
		else
			DBG(0, "regulator set vol ok, <%d,%d>\n", VUSB33_VOL_MIN, VUSB33_VOL_MAX);
		ret = regulator_enable(reg);
		if (ret < 0) {
			DBG(0, "regulator_enable failed: %d\n", ret);
			regulator_put(reg);
		} else {
			DBG(0, "enable USB regulator\n");
		}
	} else {
		DBG(0, "regulator_get failed\n");
	}
#endif
#endif

	/* ac_usb_enable(musb); */

	musb->isr = ac_usb_interrupt;
	atc_h_musb_int_init(musb);

	setup_timer(&musb_idle_timer, musb_do_idle, (unsigned long)musb);

#ifdef CONFIG_USB_ATC_OTG
	h_ac_usb_otg_init(musb);
#endif

	return 0;
}

static int ac_usb_exit(struct musb *musb)
{
	del_timer_sync(&musb_idle_timer);
	return 0;
}

static int ac_usb_set_mode(struct musb *musb, u8 mode)
{
	musb->usb_mode = mode;
	/* reset DMA_INTR_ENABLE to cause this register clear */
	musb_writel(musb->mregs, MUSB_HSDMA_INTR, 0xff | (0xff << DMA_INTR_UNMASK_SET_OFFSET));
	h_ac_usb_iddig_int(musb);
	return 0;
}

static int ac_usb_power_status(struct musb *musb)
{
	return usb_power_state;
}

static void ac_usb_set_power(struct musb *musb, int on)
{
	int err;
	
	if(usb1power == NULL)
	{
		pr_info("[MUSBH]usb1power is NULL,please check gpio config in dts\n");
		err = -1;
		return err;
	}
	pr_info("[MUSBH]set power val is %d\n",on);
	
	if(GPIO_MultiFun_Get(PIN_2_GPIO2) != PINMUX_LEVEL_GPIO_END_FLAG)
		GPIO_MultiFun_Set(PIN_2_GPIO2, PINMUX_LEVEL_GPIO_END_FLAG);
	if(on){
		err = gpiod_direction_output(usb1power,!power_on_value);
		if(err){
			pr_info("[MUSBH]set gpio faile err is %d\n",err);
			return err;
		}
		else{
			usb_power_state = on;
			return 0;
		}
	}
	else{
		err = gpiod_direction_output(usb1power,!!power_on_value);
		if(err){
			pr_info("[MUSBH]set gpio faile err is %d\n",err);
			return err;
		}
		else{
			usb_power_state = on;
			return 0;
		}
	}
}


static const struct musb_platform_ops ac_usb_ops = {
	.init = ac_usb_init,
	.exit = ac_usb_exit,
	.set_mode     = ac_usb_set_mode,
	.try_idle = ac_usb_try_idle,
	.enable = ac_usb_enable,
	.disable = ac_usb_disable,
	.power_status = ac_usb_power_status,
	.set_power = ac_usb_set_power,
};

static u64 ac_usb_dmamask = DMA_BIT_MASK(32);

static int ac_usb_probe(struct platform_device *pdev)
{
	struct musb_hdrc_platform_data *pdata = pdev->dev.platform_data;
	struct platform_device *musb;
	struct ac_usb_glue *glue;
#ifdef CONFIG_OF
	struct musb_hdrc_config *config;
	struct device_node *np = pdev->dev.of_node;
#endif
#ifdef CONFIG_ATC_UART_USB_SWITCH
	struct device_node *ap_uart0_node = NULL;
#endif
	int ret = -ENOMEM;

	glue = kzalloc(sizeof(*glue), GFP_KERNEL);
	if (!glue) {
		/* dev_err(&pdev->dev, "failed to allocate glue context\n"); */
		goto err0;
	}

	musb = platform_device_alloc("musbh-hdrc", PLATFORM_DEVID_AUTO);
	if (!musb) {
		dev_err(&pdev->dev, "failed to allocate musb device\n");
		goto err1;
	}
#ifdef CONFIG_OF

	/* usb_irq_number1 = irq_of_parse_and_map(pdev->dev.of_node, 0); */
	usb_phy_base = (unsigned long)of_iomap(pdev->dev.of_node, 1);
	pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		dev_err(&pdev->dev, "failed to allocate musb platfrom data\n");
		goto err2;
	}

	config = devm_kzalloc(&pdev->dev, sizeof(*config), GFP_KERNEL);
	if (!config) {
		/* dev_err(&pdev->dev, "failed to allocate musb hdrc config\n"); */
		goto err2;
	}
#ifdef CONFIG_USB_ATC_OTG
	pdata->mode = MUSB_OTG;
#else
	of_property_read_u32(np, "mode", (u32 *) &pdata->mode);
#endif

#ifdef CONFIG_ATC_UART_USB_SWITCH
	ap_uart0_node = of_find_compatible_node(NULL, NULL, AP_UART0_COMPATIBLE_NAME);

	if (ap_uart0_node == NULL) {
		dev_err(&pdev->dev, "USB get ap_uart0_node failed\n");
		if (ap_uart0_base)
			iounmap(ap_uart0_base);
		ap_uart0_base = 0;
	} else {
		ap_uart0_base = of_iomap(ap_uart0_node, 0);
	}
#endif

	of_property_read_u32(np, "num_eps", (u32 *) &config->num_eps);
	config->multipoint = of_property_read_bool(np, "multipoint");

	/* deprecated on musb.h, mark it to reduce build warning */
	of_property_read_u32(np, "dma_channels", (u32 *) &config->dma_channels);
	config->dyn_fifo = of_property_read_bool(np, "dyn_fifo");
	/* config->soft_con = of_property_read_bool(np, "soft_con"); */
	config->dma = of_property_read_bool(np, "dma");

	pdata->config = config;
#endif

	musb->dev.parent = &pdev->dev;
	musb->dev.dma_mask = &ac_usb_dmamask;
	musb->dev.coherent_dma_mask = ac_usb_dmamask;
#ifdef CONFIG_OF
	pdev->dev.dma_mask = &ac_usb_dmamask;
	pdev->dev.coherent_dma_mask = ac_usb_dmamask;

	/****** add by zwx
		1.we need config GPIO(usb1power)&GPIO_default_value(power_gpio_value) in dts;
		2.get GPIO desc & GPIO_default_value from dts;
		3.if cmd is 1, set GPIO_default_value to GPIO ,
		   if cmd is 0, set !GPIO_default_value to GPIO .
	*/
	usb1power = __gpiod_get(&(pdev->dev),"usb1power",GPIOD_ASIS);
	if(IS_ERR(usb1power))
	{
		pr_info("[MUSBH] can't get usb1power gpio\n");
		usb1power = NULL;
	}
	else{
		is_set_default_vale = of_property_read_bool(np, "power_gpio_value");
		if(is_set_default_vale == true){
			of_property_read_u32(np,"power_gpio_value",(u32 *)&power_on_value);
			pr_info("[MUSBH] power_on_value is :%d\n",power_on_value);
		}
		else{
			usb1power = NULL;
			pr_info("[MUSBH] not set gpio default value \n");
		}
		
	}
	
#endif

	glue->dev = &pdev->dev;
	glue->musb = musb;

	pdata->platform_ops = &ac_usb_ops;

	platform_set_drvdata(pdev, glue);

	ret = platform_device_add_resources(musb, pdev->resource, pdev->num_resources);
	if (ret) {
		dev_err(&pdev->dev, "failed to add resources\n");
		goto err2;
	}

	ret = platform_device_add_data(musb, pdata, sizeof(*pdata));
	if (ret) {
		dev_err(&pdev->dev, "failed to add platform_data\n");
		goto err2;
	}

	ret = platform_device_add(musb);

	if (ret) {
		dev_err(&pdev->dev, "failed to register musb device\n");
		goto err2;
	}

#ifdef CONFIG_ATC_UART_USB_SWITCH
	ret = device_create_file(&pdev->dev, &dev_attr_portmode);
	ret = device_create_file(&pdev->dev, &dev_attr_tx);
	ret = device_create_file(&pdev->dev, &dev_attr_rx);
	ret = device_create_file(&pdev->dev, &dev_attr_uartpath);
#endif

	if (ret) {
		dev_err(&pdev->dev, "failed to create musb device\n");
		goto err2;
	}
#ifdef CONFIG_OF
	DBG(0, "USB probe done!\n");
#endif

	return 0;

err2:
	platform_device_put(musb);

err1:
	kfree(glue);

err0:
	return ret;
}

static int ac_usb_dts_probe(struct platform_device *pdev)
{
	int retval = 0;

	/* enable uart log */
	/* musb_uart_debug = 1; */
#if 0
#ifndef CONFIG_ATC_CLKMGR
	h_musb_clk = devm_clk_get(&pdev->dev, "usb0");
	if (IS_ERR(h_musb_clk)) {
		DBG(0, KERN_WARNING "cannot get musb clock\n");
		return PTR_ERR(h_musb_clk);
	}
	DBG(0, KERN_WARNING "get musb clock ok, prepare it\n");
	retval = clk_prepare(h_musb_clk);
	if (retval == 0) {
		DBG(0, KERN_WARNING "prepare done\n");
	} else {
		DBG(0, KERN_WARNING "prepare fail\n");
		return retval;
	}
#endif
#endif
	ac_usb_device.dev.of_node = pdev->dev.of_node;
	retval = platform_device_register(&ac_usb_device);
	if (retval != 0)
		DBG(0, "register musbfsh device fail!\n");

	return retval;

}

static int ac_usb_remove(struct platform_device *pdev)
{
	struct ac_usb_glue *glue = platform_get_drvdata(pdev);

	platform_device_unregister(glue->musb);
	kfree(glue);

	return 0;
}

static int ac_usb_dts_remove(struct platform_device *pdev)
{
	struct ac_usb_glue *glue = platform_get_drvdata(pdev);

	platform_device_unregister(glue->musb);
	kfree(glue);

#ifndef CONFIG_ATC_CLKMGR
	clk_unprepare(h_musb_clk);
#endif

	return 0;
}


static struct platform_driver ac_usb_driver = {
	.remove = ac_usb_remove,
	.probe = ac_usb_probe,
	.driver = {
		   .name = "ac_usbh",
		   },
};

static struct platform_driver ac_usb_dts_driver = {
	.remove = ac_usb_dts_remove,
	.probe = ac_usb_dts_probe,
	.driver = {
		   .name = "ac_dts_usbh",
#ifdef CONFIG_OF
		   .of_match_table = apusb_of_ids,
#endif
		   },
};

static void ac83xx_h_usb_suspend(struct musb* musb)
{
	u8 reg_bk,reg_com,reg_power;
	
	reg_bk = reg_com = musb_readb(musb->mregs,MUSB_INTRUSBE);
	reg_com &= ~MUSB_INTR_SOF;
	musb_writeb(musb->mregs,MUSB_INTRUSBE,reg_com);
	//clear int
	reg_com = musb_readb(musb->mregs,MUSB_INTRUSB);
	musb_writeb(musb->mregs,MUSB_INTRUSB,reg_com);

	reg_power = musb_readb(musb->mregs,MUSB_POWER);
	reg_power &= ~(MUSB_POWER_ENSUSPEND | MUSB_POWER_SUSPENDM | MUSB_POWER_RESUME);
	reg_power |= (MUSB_POWER_ENSUSPEND | MUSB_POWER_SUSPENDM);

	musb_writeb(musb->mregs,MUSB_POWER,reg_power);
	musb_writeb(musb->mregs,MUSB_INTRUSBE,reg_bk);
}


static void ac83xx_h_usb_resume(struct musb* musb)
{
	if(musb->is_host != 1){
		musb->is_host = 1;
	}	
	//init PHY REG
	h_usb_clock_enable();
	h_usb_phy_recover();
	atc_h_musb_host_init();
	atc_h_musb_int_init(musb);
	/*======add by zwx to fix [AC8317M-6385]      ========================================*/       
	/**======The default status of USB should be disconnection when system resume back===== ***/
	/**=====the status of USB will be updata in CONNECT/DISCONNECT/ irq=======================*/
	musb->port1_status &= ~USB_PORT_STAT_CONNECTION;
}


void h_platform_musb_suspend(struct musb* musb)
{
	printk("[USB1]-- ac83xx m usb suspend --\n");
	//usb_hcd_resume_root_hub(musb_to_hcd(musb));
	//h_musb_root_disconnect(musb);
	ac83xx_h_usb_suspend(musb);
	printk("[USB1]-- ac83xx m usb suspend end--\n");
}
void h_platform_musb_resume(struct musb* musb)
{
	
	/* Automatically process */
	printk("[USB1]++ac83xx m usb resume++\n");
	ac83xx_h_usb_resume(musb);
	printk("[USB1]++ac83xx m usb resume end++\n");
}



static int __init usb20_init(void)
{
	DBG(0, "usb20 init\n");

#ifdef FPGA_PLATFORM
	add_usb_i2c_driver();
#endif
	platform_driver_register(&ac_usb_driver);
	return platform_driver_register(&ac_usb_dts_driver);
}
fs_initcall(usb20_init);

static void __exit usb20_exit(void)
{
	platform_driver_unregister(&ac_usb_driver);
	platform_driver_unregister(&ac_usb_dts_driver);
}
module_exit(usb20_exit)
