/*
 * MUSB OTG controller driver for Blackfin Processors
 *
 * Copyright 2006-2008 Analog Devices Inc.
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Licensed under the GPL-2 or later.
 */

#ifdef CONFIG_USB_ATC_OTG
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/gpio.h>
#include <linux/io.h>
#ifndef CONFIG_OF
#include <mach/irqs.h>
#endif
#if defined(CONFIG_ATC_LEGACY)
#include <mach/ac_gpio.h>
#include <cust_gpio_usage.h>
#endif
#include "musb_core.h"
#include <linux/platform_device.h>
#include "musbhsdma.h"
#include <linux/switch.h>
#include "usb20.h"
#ifdef CONFIG_OF
#include <linux/of_irq.h>
#include <linux/of_address.h>
#endif


#if !defined(CONFIG_ATC_LEGACY)
struct pinctrl *pinctrl;
struct pinctrl_state *pinctrl_iddig;
struct pinctrl_state *pinctrl_drvvbus;
struct pinctrl_state *pinctrl_drvvbus_low;
struct pinctrl_state *pinctrl_drvvbus_high;
#endif

static struct musb_fifo_cfg fifo_cfg_host[] = {
{ .hw_ep_num =  1, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  1, .style = MUSB_FIFO_RX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  2, .style = MUSB_FIFO_TX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  2, .style = MUSB_FIFO_RX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  3, .style = MUSB_FIFO_TX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  3, .style = MUSB_FIFO_RX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  4, .style = MUSB_FIFO_TX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  4, .style = MUSB_FIFO_RX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  5, .style = MUSB_FIFO_TX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	5, .style = MUSB_FIFO_RX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  6, .style = MUSB_FIFO_TX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	6, .style = MUSB_FIFO_RX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	7, .style = MUSB_FIFO_TX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	7, .style = MUSB_FIFO_RX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
#if 0
{ .hw_ep_num =	8, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	8, .style = MUSB_FIFO_RX,   .maxpacket = 64,  .mode = MUSB_BUF_SINGLE},
#endif
};

#if defined(CONFIG_USBIF_COMPLIANCE)
static u32 sw_deboun_time = 1;
#else
static u32 sw_deboun_time = 400;
#endif

void h_musb_session_restart(struct musb *musb)
{
	void __iomem	*mbase = musb->mregs;

	musb_writeb(mbase, MUSB_DEVCTL, (musb_readb(mbase, MUSB_DEVCTL) & (~MUSB_DEVCTL_SESSION)));
	DBG(0, "[MUSB] stopped session for VBUSERROR interrupt\n");
	USBPHY_SET8(0x6d, 0x3c);
	USBPHY_SET8(0x6c, 0x10);
	USBPHY_CLR8(0x6c, 0x2c);
	DBG(0, "[MUSB] force PHY to idle, 0x6d=%x, 0x6c=%x\n", USBPHY_READ8(0x6d), USBPHY_READ8(0x6c));
	mdelay(5);
	USBPHY_CLR8(0x6d, 0x3c);
	USBPHY_CLR8(0x6c, 0x3c);
	DBG(0, "[MUSB] let PHY resample VBUS, 0x6d=%x, 0x6c=%x\n", USBPHY_READ8(0x6d), USBPHY_READ8(0x6c));
	musb_writeb(mbase, MUSB_DEVCTL, (musb_readb(mbase, MUSB_DEVCTL) | MUSB_DEVCTL_SESSION));
	DBG(0, "[MUSB] restart session\n");
}

static void switch_int_to_device(struct musb *musb)
{
#ifdef ID_PIN_USE_EX_EINT
	/* irq_set_irq_type(usb_iddig_number, IRQF_TRIGGER_HIGH); */
	/* enable_irq(usb_iddig_number); */
#else
	 musb_writel(musb->mregs, USB_L1INTP, 0);
	 musb_writel(musb->mregs, USB_L1INTM, IDDIG_INT_STATUS|musb_readl(musb->mregs, USB_L1INTM));
#endif
	 DBG(0, "switch_int_to_device is done\n");
}
void atc_h_musb_host_init(void)
{
	u8 devctl = 0;
	/* setup fifo for host mode */
	h_ep_config_from_table_for_host(h_atc_musb);
	/* wake_lock(&h_atc_musb->usb_lock); */
	musb_platform_set_vbus(h_atc_musb, 1);

	/* for no VBUS sensing IP*/
#if 1
	/* wait VBUS ready */
	/*cannot sleep when system suspen/resume */
	if(h_musb_resume_flag == false)
		msleep(100);
	/* clear session*/
	devctl = musb_readb(h_atc_musb->mregs, MUSB_DEVCTL);
	musb_writeb(h_atc_musb->mregs, MUSB_DEVCTL, (devctl&(~MUSB_DEVCTL_SESSION)));
	/* USB MAC OFF*/
	/* VBUSVALID=0, AVALID=0, BVALID=0, SESSEND=1, IDDIG=X, IDPULLUP=1 */
	USBPHY_SET8(0x6c, 0x11);
	USBPHY_CLR8(0x6c, 0x2e);
	USBPHY_SET8(0x6d, 0x3f);
	DBG(0, "force PHY to idle, 0x6d=%x, 0x6c=%x\n", USBPHY_READ8(0x6d), USBPHY_READ8(0x6c));
	/* wait */
	mdelay(5);
	/* restart session */
	devctl = musb_readb(h_atc_musb->mregs, MUSB_DEVCTL);
	musb_writeb(h_atc_musb->mregs, MUSB_DEVCTL, (devctl | MUSB_DEVCTL_SESSION));
	/* USB MAC ONand Host Mode*/
	/* VBUSVALID=1, AVALID=1, BVALID=1, SESSEND=0, IDDIG=0, IDPULLUP=1 */
	USBPHY_CLR8(0x6c, 0x10);
	USBPHY_SET8(0x6c, 0x2d);
	USBPHY_SET8(0x6d, 0x3f);
	USBPHY_SET32(0x10, 0x50000);
	USBPHY_SET32(0x18, 0xe0000);
	DBG(0, "force PHY to host mode, 0x6d=%x, 0x6c=%x\n", USBPHY_READ8(0x6d), USBPHY_READ8(0x6c));
#endif

	h_musb_start(h_atc_musb);
	MUSB_HST_MODE(h_atc_musb);
	switch_int_to_device(h_atc_musb);
	DBG(0, "force0 PHY to host mode, 0x6d=%x, 0x6c=%x\n", USBPHY_READ8(0x6d), USBPHY_READ8(0x6c));
}

static void musb_id_pin_work(struct work_struct *data)
{
	unsigned long flags;

	spin_lock_irqsave(&h_atc_musb->lock, flags);
	h_musb_generic_disable(h_atc_musb);
	spin_unlock_irqrestore(&h_atc_musb->lock, flags);

	down(&h_atc_musb->musb_lock);
#ifdef CONFIG_ATC_KERNEL_POWER_OFF_CHARGING
	if (g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT || g_boot_mode == LOW_POWER_OFF_CHARGING_BOOT) {
		DBG(0, "do nothing due to in power off charging\n");
		goto out;
	}
#endif
	if (h_atc_musb->in_ipo_off) {
		DBG(0, "do nothing due to in_ipo_off\n");
		goto out;
	}

	h_atc_musb->is_host = 1;
	DBG(0, "musb is as %s\n", h_atc_musb->is_host?"host":"device");

	if (h_atc_musb->is_host) {
		atc_h_musb_host_init();
	}
out:
	DBG(0, "work end, is_host=%d\n", h_atc_musb->is_host);
	up(&h_atc_musb->musb_lock);

}

void h_ac_usb_iddig_int(struct musb *musb)
{
	u32 usb_l1_ploy = musb_readl(musb->mregs, USB_L1INTP);

	DBG(0, "id pin interrupt assert,polarity=0x%x\n", usb_l1_ploy);
	if (usb_l1_ploy & IDDIG_INT_STATUS)
		usb_l1_ploy &= (~IDDIG_INT_STATUS);
	else
		usb_l1_ploy |= IDDIG_INT_STATUS;

	musb_writel(musb->mregs, USB_L1INTP, usb_l1_ploy);
	musb_writel(musb->mregs, USB_L1INTM, (~IDDIG_INT_STATUS)&musb_readl(musb->mregs, USB_L1INTM));

	if (!h_atc_musb->is_ready) {
		/* dealy 5 sec if usb function is not ready */
		schedule_delayed_work(&h_atc_musb->id_pin_work, 5000*HZ/1000);
	} else {
		schedule_delayed_work(&h_atc_musb->id_pin_work, sw_deboun_time*HZ/1000);
	}
	DBG(0, "id pin interrupt assert\n");
}

static void otg_int_init(void)
{
#ifdef ID_PIN_USE_EX_EINT
	/* int	ret = 0; */
#else
	u32 phy_id_pull = 0;

	phy_id_pull = __raw_readl(U2PHYDTM1);
	phy_id_pull |= ID_PULL_UP;
	__raw_writel(phy_id_pull, U2PHYDTM1);

	musb_writel(h_atc_musb->mregs, USB_L1INTM, IDDIG_INT_STATUS|musb_readl(h_atc_musb->mregs, USB_L1INTM));
#endif
}

void h_ac_usb_otg_init(struct musb *musb)
{
#ifdef CONFIG_OF
	#if !defined(CONFIG_ATC_LEGACY)
	int ret = 0;

	pinctrl = devm_pinctrl_get(h_atc_musb->controller);
	if (IS_ERR(pinctrl)) {
		ret = PTR_ERR(pinctrl);
		dev_err(h_atc_musb->controller, "Cannot find usb pinctrl!\n");
	}
	#endif
#endif

	/* init idpin interrupt */
	INIT_DELAYED_WORK(&musb->id_pin_work, musb_id_pin_work);
	otg_int_init();

	/* EP table */
	musb->fifo_cfg_host = fifo_cfg_host;
	musb->fifo_cfg_host_size = ARRAY_SIZE(fifo_cfg_host);

}
#else

/* for not define CONFIG_USB_ATC_OTG */
void h_ac_usb_otg_init(struct musb *musb) {}
//void ac_usb_set_vbus(struct musb *musb, int is_on) {}
void h_ac_usb_iddig_int(struct musb *musb) {}
static void switch_int_to_device(struct musb *musb) {}
void h_musb_session_restart(struct musb *musb) {}
#endif
