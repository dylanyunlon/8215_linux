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

#ifdef CONFIG_OF
struct device_node		*usb_node;
struct device_node		*iddig_node;
#if 0
static unsigned int iddig_pin;
static unsigned int iddig_pin_mode;
static unsigned int iddig_if_config = 1;
static unsigned int drvvbus_pin;
static unsigned int drvvbus_pin_mode;
static unsigned int drvvbus_if_config = 1;
#endif
#endif

#if !defined(CONFIG_ATC_LEGACY)
struct pinctrl *pinctrl;
struct pinctrl_state *pinctrl_iddig;
struct pinctrl_state *pinctrl_drvvbus;
struct pinctrl_state *pinctrl_drvvbus_low;
struct pinctrl_state *pinctrl_drvvbus_high;
#endif
/* static int usb_iddig_number; */

static struct musb_fifo_cfg fifo_cfg_host[] = {
{ .hw_ep_num =  1, .style = MUSB_FIFO_TX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  1, .style = MUSB_FIFO_RX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  2, .style = MUSB_FIFO_TX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  2, .style = MUSB_FIFO_RX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  3, .style = MUSB_FIFO_TX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  3, .style = MUSB_FIFO_RX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  4, .style = MUSB_FIFO_TX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  4, .style = MUSB_FIFO_RX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =  5, .style = MUSB_FIFO_TX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	5, .style = MUSB_FIFO_RX,   .maxpacket = 0, .mode = MUSB_BUF_SINGLE},
#if 0
{ .hw_ep_num =  6, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	6, .style = MUSB_FIFO_RX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	7, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	7, .style = MUSB_FIFO_RX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	8, .style = MUSB_FIFO_TX,   .maxpacket = 512, .mode = MUSB_BUF_SINGLE},
{ .hw_ep_num =	8, .style = MUSB_FIFO_RX,   .maxpacket = 64,  .mode = MUSB_BUF_SINGLE},
#endif
};

u32 delay_time = 15;
module_param(delay_time, int, 0644);
u32 delay_time1 = 55;
module_param(delay_time1, int, 0644);
u32 iddig_cnt = 0;
module_param(iddig_cnt, int, 0644);

void ac_usb_set_vbus(struct musb *musb, int is_on)
{
	DBG(0, "ac_usb20_vbus++,is_on=%d\r\n", is_on);
#ifndef FPGA_PLATFORM
	if (is_on) {
		/* power on VBUS, implement later... */
	} else {
	}
#endif
}

int ac_usb_get_vbus_status(struct musb *musb)
{
#if 1
	return true;
#else
	int	ret = 0;

	if ((musb_readb(musb->mregs, MUSB_DEVCTL) & MUSB_DEVCTL_VBUS) != MUSB_DEVCTL_VBUS)
		ret = 1;
	else
		DBG(0, "VBUS error, devctl=%x, power=%d\n", musb_readb(musb->mregs, MUSB_DEVCTL), musb->power);
	pr_debug("vbus ready = %d\n", ret);
	return ret;
#endif
}

void ac_usb_init_drvvbus(void)
{
#if !(defined(SWITCH_CHARGER) || defined(FPGA_PLATFORM))
	#ifdef CONFIG_OF
	#if defined(CONFIG_ATC_LEGACY)
	/* should set GPIO2 as gpio mode. */
	#else
	int ret = 0;

	pr_debug("****%s:%d before Init Drive VBUS KS!!!!!\n", __func__, __LINE__);

	pinctrl_drvvbus = pinctrl_lookup_state(pinctrl, "drvvbus_init");
	if (IS_ERR(pinctrl_drvvbus)) {
		ret = PTR_ERR(pinctrl_drvvbus);
		dev_err(atc_musb->controller, "Cannot find usb pinctrl drvvbus\n");
	}

	pinctrl_drvvbus_low = pinctrl_lookup_state(pinctrl, "drvvbus_low");
	if (IS_ERR(pinctrl_drvvbus_low)) {
		ret = PTR_ERR(pinctrl_drvvbus_low);
		dev_err(atc_musb->controller, "Cannot find usb pinctrl drvvbus_low\n");
	}

	pinctrl_drvvbus_high = pinctrl_lookup_state(pinctrl, "drvvbus_high");
	if (IS_ERR(pinctrl_drvvbus_high)) {
		ret = PTR_ERR(pinctrl_drvvbus_high);
		dev_err(atc_musb->controller, "Cannot find usb pinctrl drvvbus_high\n");
	}

	pinctrl_select_state(pinctrl, pinctrl_drvvbus);
	pr_debug("****%s:%d end Init Drive VBUS KS!!!!!\n", __func__, __LINE__);
	#endif
	#else
	/* should set GPIO2 as gpio mode. */
#endif
#endif
}

#if defined(CONFIG_USBIF_COMPLIANCE)
u32 sw_deboun_time = 1;
#else
/*******====		changed by zwx for AC8317MFGE-276 	  		 ======*****/
/******====when change mode by sofeware,we don't need to delay 400ms		 =====****/
/*****====== delay too long time will led to switch mode abnormal in Carplay feature ====***/
//u32 sw_deboun_time = 400;
u32 sw_deboun_time = 10;
#endif
module_param(sw_deboun_time, int, 0644);
struct switch_dev otg_state;

static bool musb_is_host(void)
{
	u8 devctl = 0;
	int iddig_state = 1;
	bool usb_is_host = 0;

	DBG(0, "will mask PMIC charger detection\n");
#ifndef FPGA_PLATFORM
	pmic_chrdet_int_en(0);
#endif

	musb_platform_enable(atc_musb);

#ifdef ID_PIN_USE_EX_EINT
#ifndef CONFIG_ATC_FPGA
	#ifdef CONFIG_OF
	#if defined(CONFIG_ATC_LEGACY)
	/* iddig_state = ac_get_gpio_in(iddig_pin); */
	#else
	iddig_state = __gpio_get_value(0);
	#endif
	#else
	iddig_state = ac_get_gpio_in(GPIO_OTG_IDDIG_EINT_PIN);
	#endif
	DBG(0, "iddig_state = %d\n", iddig_state);
#endif
#else
	iddig_state = 0;
	devctl = musb_readb(atc_musb->mregs, MUSB_DEVCTL);
	DBG(0, "devctl = %x before end session\n", devctl);
	devctl &= ~MUSB_DEVCTL_SESSION;	/* this will cause A-device change back to B-device after A-cable plug out */
	musb_writeb(atc_musb->mregs, MUSB_DEVCTL, devctl);
	msleep(delay_time);

	devctl = musb_readb(atc_musb->mregs, MUSB_DEVCTL);
	DBG(0, "devctl = %x before set session\n", devctl);

	devctl |= MUSB_DEVCTL_SESSION;
	musb_writeb(atc_musb->mregs, MUSB_DEVCTL, devctl);
	msleep(delay_time1);
	devctl = musb_readb(atc_musb->mregs, MUSB_DEVCTL);
	DBG(0, "devclt = %x\n", devctl);
#endif

	if (devctl & MUSB_DEVCTL_BDEVICE || iddig_state) {
		DBG(0, "will unmask PMIC charger detection\n");
#ifndef FPGA_PLATFORM
		pmic_chrdet_int_en(1);
#endif
		usb_is_host = false;
	} else {
		usb_is_host = true;
	}

	DBG(0, "usb_is_host = %d\n", usb_is_host);
	return usb_is_host;
}

void musb_session_restart(struct musb *musb)
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

void switch_int_to_device(struct musb *musb)
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

void switch_int_to_host(struct musb *musb)
{
#ifdef ID_PIN_USE_EX_EINT
	/* irq_set_irq_type(usb_iddig_number, IRQF_TRIGGER_LOW); */
	/* enable_irq(usb_iddig_number); */
#else
	musb_writel(musb->mregs, USB_L1INTP, IDDIG_INT_STATUS);
	musb_writel(musb->mregs, USB_L1INTM, IDDIG_INT_STATUS|musb_readl(musb->mregs, USB_L1INTM));
#endif
	DBG(0, "switch_int_to_host is done\n");

}

void switch_int_to_host_and_mask(struct musb *musb)
{
#ifdef ID_PIN_USE_EX_EINT
	/* irq_set_irq_type(usb_iddig_number, IRQF_TRIGGER_LOW); */
	/* disable_irq(usb_iddig_number); */
#else
	musb_writel(musb->mregs, USB_L1INTM, (~IDDIG_INT_STATUS)&musb_readl(musb->mregs, USB_L1INTM));
	mb();
	musb_writel(musb->mregs, USB_L1INTP, IDDIG_INT_STATUS);
#endif
	DBG(0, "swtich_int_to_host_and_mask is done\n");
}
void atc_musb_host_init(void)
{
	u8 devctl = 0;
	/* setup fifo for host mode */
	ep_config_from_table_for_host(atc_musb);
	//wake_lock(&atc_musb->usb_lock);//zwx
	musb_platform_set_vbus(atc_musb, 1);

	/* for no VBUS sensing IP*/
#if 1
	/* wait VBUS ready */
	/*cannot sleep when system suspen/resume */
	DBG(0,"musb_resume_flag is %d\n",musb_resume_flag);
	if(musb_resume_flag == false)
		msleep(100);
	/* clear session*/
	devctl = musb_readb(atc_musb->mregs, MUSB_DEVCTL);
	musb_writeb(atc_musb->mregs, MUSB_DEVCTL, (devctl&(~MUSB_DEVCTL_SESSION)));
	/* USB MAC OFF*/
	/* VBUSVALID=0, AVALID=0, BVALID=0, SESSEND=1, IDDIG=X, IDPULLUP=1 */
	USBPHY_SET8(0x6c, 0x11);
	USBPHY_CLR8(0x6c, 0x2e);
	USBPHY_SET8(0x6d, 0x3f);
	DBG(0, "force PHY to idle, 0x6d=%x, 0x6c=%x\n", USBPHY_READ8(0x6d), USBPHY_READ8(0x6c));
	/* wait */
	mdelay(5);
	/* restart session */
	devctl = musb_readb(atc_musb->mregs, MUSB_DEVCTL);
	musb_writeb(atc_musb->mregs, MUSB_DEVCTL, (devctl | MUSB_DEVCTL_SESSION));
	/* USB MAC ONand Host Mode*/
	/* VBUSVALID=1, AVALID=1, BVALID=1, SESSEND=0, IDDIG=0, IDPULLUP=1 */
	USBPHY_CLR8(0x6c, 0x10);
	USBPHY_SET8(0x6c, 0x2d);
	USBPHY_SET8(0x6d, 0x3f);
	USBPHY_SET32(0x10, 0x50000);
	USBPHY_SET32(0x18, 0xe0000);
	DBG(0, "force PHY to host mode, 0x6d=%x, 0x6c=%x\n", USBPHY_READ8(0x6d), USBPHY_READ8(0x6c));
#endif
	atc_musb->xceiv->state = OTG_STATE_A_HOST;
	musb_start(atc_musb);
	MUSB_HST_MODE(atc_musb);
	switch_int_to_device(atc_musb);
}

void atc_musb_device_init(void)
{
	DBG(0, "devctl is %x\n", musb_readb(atc_musb->mregs, MUSB_DEVCTL));
	if(force_mode)
	force_mode = 0;
	musb_writeb(atc_musb->mregs, MUSB_DEVCTL, 0);
	//if (wake_lock_active(&atc_musb->usb_lock))
	//wake_unlock(&atc_musb->usb_lock);//zwx
	musb_platform_set_vbus(atc_musb, 0);

	/* for no VBUS sensing IP */
#if 1
	/* USB MAC OFF*/
	/* VBUSVALID=0, AVALID=0, BVALID=0, SESSEND=1, IDDIG=X, IDPULLUP=1 */
	USBPHY_SET8(0x6c, 0x11);
	USBPHY_CLR8(0x6c, 0x2e);
	USBPHY_SET8(0x6d, 0x3f);
	DBG(0, "force PHY to idle, 0x6d=%x, 0x6c=%x\n", USBPHY_READ8(0x6d), USBPHY_READ8(0x6c));
#endif

	musb_stop(atc_musb);
	/* stop dma controller */
	if (is_dma_capable() && atc_musb->dma_controller) {
	struct dma_controller *c = atc_musb->dma_controller;
	(void)c->stop(c);
	}
	atc_musb->xceiv->state = OTG_STATE_B_IDLE;
	MUSB_DEV_MODE(atc_musb);
	switch_int_to_host(atc_musb);
	ep_config_from_table(atc_musb);
	musb_start(atc_musb);
}
static void musb_id_pin_work(struct work_struct *data)
{
	unsigned long flags;

	spin_lock_irqsave(&atc_musb->lock, flags);
	musb_generic_disable(atc_musb);
	spin_unlock_irqrestore(&atc_musb->lock, flags);

	down(&atc_musb->musb_lock);
#ifdef CONFIG_ATC_KERNEL_POWER_OFF_CHARGING
	if (g_boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT || g_boot_mode == LOW_POWER_OFF_CHARGING_BOOT) {
		DBG(0, "do nothing due to in power off charging\n");
		goto out;
	}
#endif
	if (atc_musb->in_ipo_off) {
		DBG(0, "do nothing due to in_ipo_off\n");
		goto out;
	}

	if (atc_musb->usb_mode == MUSB_PERIPHERAL)
		atc_musb->is_host = 0;
	else if (atc_musb->usb_mode == MUSB_HOST)
		atc_musb->is_host = 1;
	else
		atc_musb->is_host = musb_is_host();
	DBG(0, "musb is as %s\n", atc_musb->is_host?"host":"device");
	switch_set_state((struct switch_dev *)&otg_state, atc_musb->is_host);

	musb_start(atc_musb);
	if (atc_musb->is_host) {
		atc_musb_host_init();
	} else {
		atc_musb_device_init();
	}
out:
	DBG(0, "work end, is_host=%d\n", atc_musb->is_host);
	up(&atc_musb->musb_lock);

}

#if 0
static irqreturn_t ac_usb_ext_iddig_int(int irq, void *dev_id)
{
	iddig_cnt++;
	if (!atc_musb->is_ready) {
		/* dealy 5 sec if usb function is not ready */
		schedule_delayed_work(&atc_musb->id_pin_work, 5000*HZ/1000);
	} else {
		schedule_delayed_work(&atc_musb->id_pin_work, sw_deboun_time*HZ/1000);
	}
	DBG(0, "id pin interrupt assert\n");
	disable_irq_nosync(usb_iddig_number);
	return IRQ_HANDLED;
}
#endif

void ac_usb_iddig_int(struct musb *musb)
{
	u32 usb_l1_ploy = musb_readl(musb->mregs, USB_L1INTP);

	DBG(0, "id pin interrupt assert,polarity=0x%x\n", usb_l1_ploy);
	if (usb_l1_ploy & IDDIG_INT_STATUS)
		usb_l1_ploy &= (~IDDIG_INT_STATUS);
	else
		usb_l1_ploy |= IDDIG_INT_STATUS;

	musb_writel(musb->mregs, USB_L1INTP, usb_l1_ploy);
	musb_writel(musb->mregs, USB_L1INTM, (~IDDIG_INT_STATUS)&musb_readl(musb->mregs, USB_L1INTM));

	if (!atc_musb->is_ready) {
		/* dealy 5 sec if usb function is not ready */
		schedule_delayed_work(&atc_musb->id_pin_work, 5000*HZ/1000);
	} else {
		schedule_delayed_work(&atc_musb->id_pin_work, sw_deboun_time*HZ/1000);
	}
	DBG(0, "id pin interrupt assert\n");
}

static void otg_int_init(void)
{
#ifdef ID_PIN_USE_EX_EINT
#else
	u32 phy_id_pull = 0;

	phy_id_pull = __raw_readl(U2PHYDTM1);
	phy_id_pull |= ID_PULL_UP;
	__raw_writel(phy_id_pull, U2PHYDTM1);

	musb_writel(atc_musb->mregs, USB_L1INTM, IDDIG_INT_STATUS|musb_readl(atc_musb->mregs, USB_L1INTM));
#endif
}

void ac_usb_otg_init(struct musb *musb)
{
/* #ifdef CONFIG_OF */
#if 0
	usb_node = of_find_compatible_node(NULL, NULL, "autochip,USB0");
	if (usb_node == NULL) {
		pr_err("USB OTG - get USB0 node failed\n");
	} else {
		if (of_property_read_u32_index(usb_node, "iddig_gpio", 0, &iddig_pin)) {
			iddig_if_config = 0;
			pr_err("get dtsi iddig_pin fail\n");
		}
		if (of_property_read_u32_index(usb_node, "iddig_gpio", 1, &iddig_pin_mode))
			pr_err("get dtsi iddig_pin_mode fail\n");
		if (of_property_read_u32_index(usb_node, "drvvbus_gpio", 0, &drvvbus_pin)) {
			drvvbus_if_config = 0;
			pr_err("get dtsi drvvbus_pin fail\n");
		}
		if (of_property_read_u32_index(usb_node, "drvvbus_gpio", 1, &drvvbus_pin_mode))
			pr_err("get dtsi drvvbus_pin_mode fail\n");
		iddig_pin |= 0x80000000;
		drvvbus_pin |= 0x80000000;
	}

	iddig_node = of_find_compatible_node(NULL, NULL, "IDDIG-eint");
	if (iddig_node == NULL)
		pr_err("USB IDDIG EINT - get IDDIG EINT node failed\n");
	#if !defined(CONFIG_ATC_LEGACY)
	int ret = 0;

	pinctrl = devm_pinctrl_get(atc_musb->controller);
	if (IS_ERR(pinctrl)) {
		ret = PTR_ERR(pinctrl);
		dev_err(atc_musb->controller, "Cannot find usb pinctrl!\n");
	}
	#endif
#endif
	/*init drrvbus*/
	ac_usb_init_drvvbus();

	/* init idpin interrupt */
	INIT_DELAYED_WORK(&musb->id_pin_work, musb_id_pin_work);
	otg_int_init();

	/* EP table */
	musb->fifo_cfg_host = fifo_cfg_host;
	musb->fifo_cfg_host_size = ARRAY_SIZE(fifo_cfg_host);

	otg_state.name = "otg_state";
	otg_state.index = 0;
	otg_state.state = 0;

	if (switch_dev_register(&otg_state))
		pr_err("switch_dev_register fail\n");
	else
		pr_debug("switch_dev register success\n");
}
#else

/* for not define CONFIG_USB_ATC_OTG */
void ac_usb_otg_init(struct musb *musb) {}
void ac_usb_init_drvvbus(void){}
void ac_usb_set_vbus(struct musb *musb, int is_on) {}
int ac_usb_get_vbus_status(struct musb *musb) {return 1; }
void ac_usb_iddig_int(struct musb *musb) {}
void switch_int_to_device(struct musb *musb) {}
void switch_int_to_host(struct musb *musb) {}
void switch_int_to_host_and_mask(struct musb *musb) {}
void musb_session_restart(struct musb *musb) {}
#endif
