#ifdef CONFIG_ATC_CLKMGR
#include <mach/ac_clkmgr.h>
#else
#include <linux/clk.h>
#endif
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/spinlock.h>
#include "atc_musb.h"
#include "musb_core.h"
#include "usb20.h"

#define FRA (48)
#define PARA (28)


#ifdef FPGA_PLATFORM
bool usb_enable_clock(bool enable)
{
	return true;
}

void usb_phy_poweron(void)
{
}

void usb_phy_savecurrent(void)
{
}

void usb_phy_recover(void)
{
}

/* BC1.2 */
void Charger_Detect_Init(void)
{
}

void Charger_Detect_Release(void)
{
}

void usb_phy_context_save(void)
{
}

void usb_phy_context_restore(void)
{
}

#ifdef CONFIG_ATC_UART_USB_SWITCH
bool usb_phy_check_in_uart_mode(void)
{
	UINT8 usb_port_mode;

	usb_enable_clock(true);
	udelay(50);

	usb_port_mode = USB_PHY_Read_Register8(0x6B);
	usb_enable_clock(false);

	if ((usb_port_mode == 0x5C) || (usb_port_mode == 0x5E))
		return true;
	else
		return false;
}

void usb_phy_switch_to_uart(void)
{
	int var;
#if 0
	/* SW disconnect */
	var = USB_PHY_Read_Register8(0x68);
	DBG(0, "[MUSB]addr: 0x68, value: %x\n", var);
	USB_PHY_Write_Register8(0x15, 0x68);
	DBG(0, "[MUSB]addr: 0x68, value after: %x\n", USB_PHY_Read_Register8(0x68));

	var = USB_PHY_Read_Register8(0x6A);
	DBG(0, "[MUSB]addr: 0x6A, value: %x\n", var);
	USB_PHY_Write_Register8(0x0, 0x6A);
	DBG(0, "[MUSB]addr: 0x6A, value after: %x\n", USB_PHY_Read_Register8(0x6A));
	/* SW disconnect */
#endif
	/* Set ru_uart_mode to 2'b01 */
	var = USB_PHY_Read_Register8(0x6B);
	DBG(0, "[MUSB]addr: 0x6B, value: %x\n", var);
	USB_PHY_Write_Register8(var | 0x7C, 0x6B);
	DBG(0, "[MUSB]addr: 0x6B, value after: %x\n", USB_PHY_Read_Register8(0x6B));

	/* Set RG_UART_EN to 1 */
	var = USB_PHY_Read_Register8(0x6E);
	DBG(0, "[MUSB]addr: 0x6E, value: %x\n", var);
	USB_PHY_Write_Register8(var | 0x07, 0x6E);
	DBG(0, "[MUSB]addr: 0x6E, value after: %x\n", USB_PHY_Read_Register8(0x6E));

	/* Set RG_USB20_DM_100K_EN to 1 */
	var = USB_PHY_Read_Register8(0x22);
	DBG(0, "[MUSB]addr: 0x22, value: %x\n", var);
	USB_PHY_Write_Register8(var | 0x02, 0x22);
	DBG(0, "[MUSB]addr: 0x22, value after: %x\n", USB_PHY_Read_Register8(0x22));

	var = DRV_Reg8(UART1_BASE + 0x90);
	DBG(0, "[MUSB]addr: 0x11002090 (UART1), value: %x\n", var);
	DRV_WriteReg8(UART1_BASE + 0x90, var | 0x01);
	DBG(0, "[MUSB]addr: 0x11002090 (UART1), value after: %x\n\n", DRV_Reg8(UART1_BASE + 0x90));

	/* SW disconnect */
	ac_usb_disconnect();
}

void usb_phy_switch_to_usb(void)
{
	int var;
	/* Set RG_UART_EN to 0 */
	var = USB_PHY_Read_Register8(0x6E);
	DBG(0, "[MUSB]addr: 0x6E, value: %x\n", var);
	USB_PHY_Write_Register8(var & ~0x01, 0x6E);
	DBG(0, "[MUSB]addr: 0x6E, value after: %x\n", USB_PHY_Read_Register8(0x6E));

	/* Set RG_USB20_DM_100K_EN to 0 */
	var = USB_PHY_Read_Register8(0x22);
	DBG(0, "[MUSB]addr: 0x22, value: %x\n", var);
	USB_PHY_Write_Register8(var & ~0x02, 0x22);
	DBG(0, "[MUSB]addr: 0x22, value after: %x\n", USB_PHY_Read_Register8(0x22));

	var = DRV_Reg8(UART1_BASE + 0x90);
	DBG(0, "[MUSB]addr: 0x11002090 (UART1), value: %x\n", var);
	DRV_WriteReg8(UART1_BASE + 0x90, var & ~0x01);
	DBG(0, "[MUSB]addr: 0x11002090 (UART1), value after: %x\n\n", DRV_Reg8(UART1_BASE + 0x90));
#if 0
	/* SW connect */
	var = USB_PHY_Read_Register8(0x68);
	DBG(0, "[MUSB]addr: 0x68, value: %x\n", var);
	USB_PHY_Write_Register8(0x0, 0x68);
	DBG(0, "[MUSB]addr: 0x68, value after: %x\n", USB_PHY_Read_Register8(0x68));

	var = USB_PHY_Read_Register8(0x6A);
	DBG(0, "[MUSB]addr: 0x6A, value: %x\n", var);
	USB_PHY_Write_Register8(0x0, 0x6A);
	DBG(0, "[MUSB]addr: 0x6A, value after: %x\n", USB_PHY_Read_Register8(0x6A));
	/* SW connect */
#endif
	/* SW connect */
	ac_usb_connect();
}
#endif

#else

#ifdef CONFIG_ATC_UART_USB_SWITCH
bool in_uart_mode = false;
#endif


bool usb_enable_clock(bool enable)
{
	return 1;
}

#ifdef CONFIG_ATC_UART_USB_SWITCH
bool usb_phy_check_in_uart_mode(void)
{
	UINT8 usb_port_mode;

	usb_enable_clock(true);
	udelay(50);
	usb_port_mode = USBPHY_READ8(0x6B);
	usb_enable_clock(false);

	if ((usb_port_mode == 0x5C) || (usb_port_mode == 0x5E)) {
		DBG(0, "%s:%d - IN UART MODE : 0x%x\n", __func__, __LINE__, usb_port_mode);
		DBG(0, "Mask PMIC charger detection in UART mode.\n");
		pmic_chrdet_int_en(0);
		in_uart_mode = true;
	} else {
		DBG(0, "%s:%d - NOT IN UART MODE : 0x%x\n", __func__, __LINE__, usb_port_mode);
		in_uart_mode = false;
	}
	return in_uart_mode;
}

void usb_phy_switch_to_uart(void)
{
	if (usb_phy_check_in_uart_mode())
		return;
	DBG(0, "Mask PMIC charger detection in UART mode.\n");
	pmic_chrdet_int_en(0);

	usb_enable_clock(true);
	udelay(50);

	/* RG_USB20_BC11_SW_EN = 1'b0 */
	USBPHY_CLR8(0x1a, 0x80);

	/* Set RG_SUSPENDM to 1 */
	USBPHY_SET8(0x68, 0x08);

	/* force suspendm = 1 */
	USBPHY_SET8(0x6a, 0x04);

	/* Set ru_uart_mode to 2'b01 */
	USBPHY_SET8(0x6B, 0x5C);

	/* Set RG_UART_EN to 1 */
	USBPHY_SET8(0x6E, 0x07);

	/* Set RG_USB20_DM_100K_EN to 1 */
	USBPHY_SET8(0x22, 0x02);
	usb_enable_clock(false);

	/* GPIO Selection */
	DRV_WriteReg32(ap_uart0_base + 0xB0, 0x1);
}


void usb_phy_switch_to_usb(void)
{
	/* GPIO Selection */
	DRV_WriteReg32(ap_uart0_base + 0xB0, 0x0);

	usb_enable_clock(true);
	udelay(50);
	/* clear force_uart_en */
	USBPHY_WRITE8(0x6B, 0x00);
	usb_enable_clock(false);
	usb_phy_poweron();
	/* disable the USB clock turned on in usb_phy_poweron() */
	usb_enable_clock(false);

	DBG(0, "Unmask PMIC charger detection in USB mode.\n");
	pmic_chrdet_int_en(1);
}
#endif

/* Denali_USB_PWR Sequence 20141030.xls */
void usb_phy_poweron(void)
{
#if 0
#ifdef CONFIG_ATC_UART_USB_SWITCH
	if (usb_phy_check_in_uart_mode())
		return;
#endif

	/* enable USB MAC clock. */
	usb_enable_clock(true);

	/* wait 50 usec for PHY3.3v/1.8v stable. */
	udelay(50);

	/* force_uart_en, 1'b0 */
	USBPHY_CLR8(0x6b, 0x04);
	/* RG_UART_EN, 1'b0 */
	USBPHY_CLR8(0x6e, 0x01);
	/* rg_usb20_gpio_ctl, 1'b0, usb20_gpio_mode, 1'b0 */
	USBPHY_CLR8(0x21, 0x03);

	/* RG_USB20_BC11_SW_EN, 1'b0 */
	USBPHY_CLR8(0x1a, 0x80);

	/* rg_usb20_dp_100k_mode, 1'b1 */
	USBPHY_SET8(0x22, 0x04);
	/* USB20_DP_100K_EN 1'b0, RG_USB20_DM_100K_EN, 1'b0 */
	USBPHY_CLR8(0x22, 0x03);

	/* RG_USB20_OTG_VBUSCMP_EN, 1'b1 */
	USBPHY_SET8(0x1a, 0x10);

	/* force_suspendm, 1'b0 */
	USBPHY_CLR8(0x6a, 0x04);

	/* 7 s7: wait for 800 usec. */
	udelay(800);

	/* force enter device mode, from K2, FIXME */
	USBPHY_CLR8(0x6c, 0x10);
	USBPHY_SET8(0x6c, 0x2F);
	USBPHY_SET8(0x6d, 0x3F);

#endif
	DBG(0, "usb power on success\n");
}

#ifdef CONFIG_ATC_UART_USB_SWITCH
static bool skipDisableUartMode = true;
#endif

/* Denali_USB_PWR Sequence 20141030.xls */
static void usb_phy_savecurrent_internal(void)
{
#if 0
	/* 4 1. swtich to USB function. (system register, force ip into usb mode. */
#ifdef CONFIG_ATC_UART_USB_SWITCH
	if (!usb_phy_check_in_uart_mode()) {
		/* enable USB MAC clock. */
		usb_enable_clock(true);

		/* wait 50 usec for PHY3.3v/1.8v stable. */
		udelay(50);

		/* force_uart_en, 1'b0 */
		USBPHY_CLR8(0x6b, 0x04);
		/* RG_UART_EN, 1'b0 */
		USBPHY_CLR8(0x6e, 0x01);
		/* rg_usb20_gpio_ctl, 1'b0, usb20_gpio_mode, 1'b0 */
		USBPHY_CLR8(0x21, 0x03);

		/* RG_USB20_BC11_SW_EN, 1'b0 */
		USBPHY_CLR8(0x1a, 0x80);
		/* RG_USB20_OTG_VBUSCMP_EN, 1'b0 */
		USBPHY_CLR8(0x1a, 0x10);

		/* RG_SUSPENDM, 1'b1 */
		USBPHY_SET8(0x68, 0x08);
		/* force_suspendm, 1'b1 */
		USBPHY_SET8(0x6a, 0x04);

		usb_enable_clock(false);
	} else {
		if (skipDisableUartMode)
			skipDisableUartMode = false;
		else
			return;
	}
#else
	/* force_uart_en, 1'b0 */
	USBPHY_CLR8(0x6b, 0x04);
	/* RG_UART_EN, 1'b0 */
	USBPHY_CLR8(0x6e, 0x01);
	/* rg_usb20_gpio_ctl, 1'b0, usb20_gpio_mode, 1'b0 */
	USBPHY_CLR8(0x21, 0x03);

	/* RG_USB20_BC11_SW_EN, 1'b0 */
	USBPHY_CLR8(0x1a, 0x80);
	/* RG_USB20_OTG_VBUSCMP_EN, 1'b0 */
	USBPHY_CLR8(0x1a, 0x10);

	/* RG_SUSPENDM, 1'b1 */
	USBPHY_SET8(0x68, 0x08);
	/* force_suspendm, 1'b1 */
	USBPHY_SET8(0x6a, 0x04);
#endif

	/* RG_DPPULLDOWN, 1'b1, RG_DMPULLDOWN, 1'b1 */
	USBPHY_SET8(0x68, 0xc0);
	/* RG_XCVRSEL[1:0], 2'b01. */
	USBPHY_CLR8(0x68, 0x20);
	USBPHY_SET8(0x68, 0x10);
	/* RG_TERMSEL, 1'b1 */
	USBPHY_SET8(0x68, 0x04);
	/* RG_DATAIN[3:0], 4'b0000 */
	USBPHY_CLR8(0x69, 0x3c);

	/* force_dp_pulldown, 1'b1, force_dm_pulldown, 1'b1,
	   force_xcversel, 1'b1, force_termsel, 1'b1, force_datain, 1'b1 */
	USBPHY_SET8(0x6a, 0xba);

	udelay(800);

	/* RG_SUSPENDM, 1'b0 */
	USBPHY_CLR8(0x68, 0x08);

	/* ALPS00427972, implement the analog register formula */
	/*
	   DBG(0, "%s: USBPHY_READ8(0x05) = 0x%x\n", __func__, USBPHY_READ8(0x05));
	   DBG(0, "%s: USBPHY_READ8(0x07) = 0x%x\n", __func__, USBPHY_READ8(0x07));
	 */
	/* ALPS00427972, implement the analog register formula */

	udelay(1);

	/* force enter device mode, from K2, FIXME */
	/* force enter device mode */
	/* USBPHY_CLR8(0x6c, 0x10); */
	/* USBPHY_SET8(0x6c, 0x2E); */
	/* USBPHY_SET8(0x6d, 0x3E); */

#ifdef CONFIG_ATC_UART_USB_SWITCH
	if (in_uart_mode) {
		USBPHY_SET8(0x68, 0x08);
		DBG(0, "%s:%d - SWITCH to UART MODE after savecurrent!\n", __func__, __LINE__);
	}
#endif
#endif
}

void usb_phy_savecurrent(void)
{

	/* to avoid hw acess during clock-off */
	unsigned long flags;
	int do_lock;

	do_lock = 0;
	usb_phy_savecurrent_internal();

	/* to avoid deadlock, musb_shutdown will hold this clock too */
	if (atc_musb && !musb_is_shutting) {
		spin_lock_irqsave(&atc_musb->lock, flags);
		do_lock = 1;
	}

	/* 4 14. turn off internal 48Mhz PLL. */
	usb_enable_clock(false);

	if (do_lock)
		spin_unlock_irqrestore(&atc_musb->lock, flags);

	DBG(0, "usb save current success\n");
}

/* Denali_USB_PWR Sequence 20141030.xls */
#if 1
void usb_phy_recover(void)
{
	uint32_t val;

	USBPHY_SET32(0x68, 0x4000);
	mdelay(1);
	USBPHY_CLR32(0x68, 0x4000);

	val = USBPHY_READ32(0x6c);
	val &= ~0x3f3f;
	val |= 0x3e2e;
	USBPHY_WRITE32(0x6c, val);

	USBPHY_CLR32(0x68, 0x40000);
}
#else
void usb_phy_recover(void)
{

	/* to avoid hw acess during clock-on */
	unsigned long flags;
	int do_lock;

	do_lock = 0;

	if (atc_musb) {
		spin_lock_irqsave(&atc_musb->lock, flags);
		do_lock = 1;
	}

	/* turn on USB reference clock. */
	usb_enable_clock(true);


	if (do_lock)
		spin_unlock_irqrestore(&atc_musb->lock, flags);

	/* wait 50 usec. */
	udelay(50);

#ifdef CONFIG_ATC_UART_USB_SWITCH
	if (!usb_phy_check_in_uart_mode()) {
		/* clean PUPD_BIST_EN */
		/* PUPD_BIST_EN = 1'b0 */
		/* PMIC will use it to detect charger type */
		USBPHY_CLR8(0x1d, 0x10);

		/* force_uart_en, 1'b0 */
		USBPHY_CLR8(0x6b, 0x04);
		/* RG_UART_EN, 1'b0 */
		USBPHY_CLR8(0x6e, 0x01);
		/* rg_usb20_gpio_ctl, 1'b0, usb20_gpio_mode, 1'b0 */
		USBPHY_CLR8(0x21, 0x03);

		/* force_suspendm, 1'b0 */
		USBPHY_CLR8(0x6a, 0x04);

		skipDisableUartMode = false;
	} else {
		if (!skipDisableUartMode)
			return;
	}
#else

	/* clean PUPD_BIST_EN */
	/* PUPD_BIST_EN = 1'b0 */
	/* PMIC will use it to detect charger type */
	USBPHY_CLR8(0x1d, 0x10);

	/* force_uart_en, 1'b0 */
	USBPHY_CLR8(0x6b, 0x04);
	/* RG_UART_EN, 1'b0 */
	USBPHY_CLR8(0x6e, 0x01);
	/* rg_usb20_gpio_ctl, 1'b0, usb20_gpio_mode, 1'b0 */
	USBPHY_CLR8(0x21, 0x03);

	/* force_suspendm, 1'b0 */
	USBPHY_CLR8(0x6a, 0x04);
#endif

	/* RG_DPPULLDOWN, 1'b0, RG_DMPULLDOWN, 1'b0 */
	USBPHY_CLR8(0x68, 0xc0);
	/* RG_XCVRSEL[1:0], 2'b00. */
	USBPHY_CLR8(0x68, 0x30);
	/* RG_TERMSEL, 1'b0 */
	USBPHY_CLR8(0x68, 0x04);
	/* RG_DATAIN[3:0], 4'b0000 */
	USBPHY_CLR8(0x69, 0x3c);

	/* force_dp_pulldown, 1'b0, force_dm_pulldown, 1'b0,
	   force_xcversel, 1'b0, force_termsel, 1'b0, force_datain, 1'b0 */
	USBPHY_CLR8(0x6a, 0xba);

	/* RG_USB20_BC11_SW_EN, 1'b0 */
	USBPHY_CLR8(0x1a, 0x80);
	/* RG_USB20_OTG_VBUSCMP_EN, 1'b1 */
	USBPHY_SET8(0x1a, 0x10);


	/* wait 800 usec. */
	udelay(800);

	/* force enter device mode, from K2, FIXME */
	USBPHY_CLR8(0x6c, 0x10);
	USBPHY_SET8(0x6c, 0x2F);
	USBPHY_SET8(0x6d, 0x3F);

	/* from K2, FIXME */
#if defined(ATC_HDMI_SUPPORT)
	USBPHY_SET8(0x05, 0x05);
	USBPHY_SET8(0x05, 0x50);
#endif

	/* adjustment after HQA */
	HQA_special();

	hs_slew_rate_cal();

	DBG(0, "usb recovery success\n");
}
#endif

/* BC1.2 */
void Charger_Detect_Init(void)
{
	/* turn on USB reference clock. */
	usb_enable_clock(true);
	/* wait 50 usec. */
	udelay(50);
	/* RG_USB20_BC11_SW_EN = 1'b1 */
	USBPHY_SET8(0x1a, 0x80);
	DBG(0, "Charger_Detect_Init\n");
}

void Charger_Detect_Release(void)
{
	/* RG_USB20_BC11_SW_EN = 1'b0 */
	USBPHY_CLR8(0x1a, 0x80);
	udelay(1);
	/* 4 14. turn off internal 48Mhz PLL. */
	usb_enable_clock(false);
	DBG(0, "Charger_Detect_Release\n");
}

void usb_phy_context_save(void)
{
#ifdef CONFIG_ATC_UART_USB_SWITCH
	in_uart_mode = usb_phy_check_in_uart_mode();
#endif
}

void usb_phy_context_restore(void)
{
#ifdef CONFIG_ATC_UART_USB_SWITCH
	if (in_uart_mode)
		usb_phy_switch_to_uart();
#endif
	usb_phy_savecurrent_internal();
}

#endif
