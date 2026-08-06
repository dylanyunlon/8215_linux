#ifndef __MUSB_ATC_MUSB_H__
#define __MUSB_ATC_MUSB_H__

#include <generated/atc_project.h>


#ifdef CONFIG_ATC_PLATFORM_ac83xx
#ifdef CONFIG_OF
#define USBPHY_READ8(offset)          musb_readb((void __iomem *)(((unsigned long)atc_musb->xceiv->io_priv)+0x800), offset)
#define USBPHY_WRITE8(offset, value)	musb_writeb((void __iomem *)\
		(((unsigned long)atc_musb->xceiv->io_priv)+0x800), offset, value);

#define USBPHY_SET8(offset, mask)     USBPHY_WRITE8(offset, (USBPHY_READ8(offset)) | (mask))
#define USBPHY_CLR8(offset, mask)     USBPHY_WRITE8(offset, (USBPHY_READ8(offset)) & (~mask))

#define USBPHY_READ16(offset)          musb_readw((void __iomem *)(((unsigned long)atc_musb->xceiv->io_priv)+0x800), offset)
#define USBPHY_WRITE16(offset, value)  musb_writew((void __iomem *)\
		(((unsigned long)atc_musb->xceiv->io_priv)+0x800), offset, value)
#define USBPHY_SET16(offset, mask)     USBPHY_WRITE16(offset, (USBPHY_READ16(offset)) | (mask))
#define USBPHY_CLR16(offset, mask)     USBPHY_WRITE16(offset, (USBPHY_READ16(offset)) & (~mask))

#define USBPHY_READ32(offset)          musb_readl((void __iomem *)(((unsigned long)atc_musb->xceiv->io_priv)+0x800), offset)
#define USBPHY_WRITE32(offset, value)  musb_writel((void __iomem *)\
		(((unsigned long)atc_musb->xceiv->io_priv)+0x800), offset, value)
#define USBPHY_SET32(offset, mask)     USBPHY_WRITE32(offset, (USBPHY_READ32(offset)) | (mask))
#define USBPHY_CLR32(offset, mask)     USBPHY_WRITE32(offset, (USBPHY_READ32(offset)) & (~mask))

#ifdef ATC_UART_USB_SWITCH
#define UART2_BASE 0x11003000
#endif

#else

#include <mach/ac_reg_base.h>
#define USBPHY_READ8(offset)          readb((void __iomem *)(USB_SIF_BASE+0x800+offset))
#define USBPHY_WRITE8(offset, value)  writeb(value, (void __iomem *)(USB_SIF_BASE+0x800+offset))
#define USBPHY_SET8(offset, mask)     USBPHY_WRITE8(offset, (USBPHY_READ8(offset)) | (mask))
#define USBPHY_CLR8(offset, mask)     USBPHY_WRITE8(offset, (USBPHY_READ8(offset)) & (~mask))

#define USBPHY_READ16(offset)          readw((void __iomem *)(USB_SIF_BASE+0x800+offset))
#define USBPHY_WRITE16(offset, value)  writew(value, (void __iomem *)(USB_SIF_BASE+0x800+offset))
#define USBPHY_SET16(offset, mask)     USBPHY_WRITE16(offset, (USBPHY_READ16(offset)) | (mask))
#define USBPHY_CLR16(offset, mask)     USBPHY_WRITE16(offset, (USBPHY_READ16(offset)) & (~mask))

#define USBPHY_READ32(offset)          readl((void __iomem *)(USB_SIF_BASE+0x800+offset))
#define USBPHY_WRITE32(offset, value)  writel(value, (void __iomem *)(USB_SIF_BASE+0x800+offset))
#define USBPHY_SET32(offset, mask)     USBPHY_WRITE32(offset, (USBPHY_READ32(offset)) | (mask))
#define USBPHY_CLR32(offset, mask)     USBPHY_WRITE32(offset, (USBPHY_READ32(offset)) & (~mask))

#endif

#else
#ifdef CONFIG_OF
extern struct musb *atc_musb;
#define USBPHY_READ8(offset)          musb_readb((void __iomem *)(((unsigned long)atc_musb->xceiv->io_priv)), offset)
#define USBPHY_WRITE8(offset, value)	musb_writeb((void __iomem *)\
		(((unsigned long)atc_musb->xceiv->io_priv)), offset, value);

#define USBPHY_SET8(offset, mask)     USBPHY_WRITE8(offset, (USBPHY_READ8(offset)) | (mask))
#define USBPHY_CLR8(offset, mask)     USBPHY_WRITE8(offset, (USBPHY_READ8(offset)) & (~mask))

#define USBPHY_READ16(offset)          musb_readw((void __iomem *)(((unsigned long)atc_musb->xceiv->io_priv)), offset)
#define USBPHY_WRITE16(offset, value)  musb_writew((void __iomem *)\
		(((unsigned long)atc_musb->xceiv->io_priv)), offset, value)
#define USBPHY_SET16(offset, mask)     USBPHY_WRITE16(offset, (USBPHY_READ16(offset)) | (mask))
#define USBPHY_CLR16(offset, mask)     USBPHY_WRITE16(offset, (USBPHY_READ16(offset)) & (~mask))

#define USBPHY_READ32(offset)          musb_readl((void __iomem *)(((unsigned long)atc_musb->xceiv->io_priv)), offset)
#define USBPHY_WRITE32(offset, value)  musb_writel((void __iomem *)\
		(((unsigned long)atc_musb->xceiv->io_priv)), offset, value)
#define USBPHY_SET32(offset, mask)     USBPHY_WRITE32(offset, (USBPHY_READ32(offset)) | (mask))
#define USBPHY_CLR32(offset, mask)     USBPHY_WRITE32(offset, (USBPHY_READ32(offset)) & (~mask))

#ifdef ATC_UART_USB_SWITCH
#define UART2_BASE 0x11003000
#endif

#endif
#endif
struct musb;

typedef enum {
	USB_SUSPEND = 0,
	USB_UNCONFIGURED,
	USB_CONFIGURED
} usb_state_enum;

/* USB phy and clock */
extern void usb_phy_poweron(void);
extern void usb_phy_recover(void);
extern void usb_phy_savecurrent(void);
extern void usb_phy_context_restore(void);
extern void usb_phy_context_save(void);
extern bool usb_enable_clock(bool enable);
extern void MUC_ResetPhy(void);
extern void phy_enable(void);
extern void usb_clock_enable(void);
extern void platform_musb_suspend(struct musb* );
extern void platform_musb_resume(struct musb* );

/* general USB */
extern bool ac_usb_is_device(void);
extern void ac_usb_connect(void);
extern void ac_usb_disconnect(void);
/* ALPS00775710 */
/* extern bool usb_iddig_state(void); */
/* ALPS00775710 */
extern bool usb_cable_connected(void);
extern void pmic_chrdet_int_en(int is_on);
extern void musb_platform_reset(struct musb *musb);
extern void musb_sync_with_bat(struct musb *musb, int usb_state);

extern bool is_saving_mode(void);

/* USB switch charger */
extern bool is_switch_charger(void);

/* host and otg */
extern void ac_usb_otg_init(struct musb *musb);
extern void ac_usb_init_drvvbus(void);
extern void ac_usb_set_vbus(struct musb *musb, int is_on);
extern int ac_usb_get_vbus_status(struct musb *musb);
extern void ac_usb_iddig_int(struct musb *musb);
extern void switch_int_to_device(struct musb *musb);
extern void switch_int_to_host(struct musb *musb);
extern void switch_int_to_host_and_mask(struct musb *musb);
extern void musb_session_restart(struct musb *musb);
extern void atc_musb_host_init(void);
extern void atc_musb_device_init(void);
extern void atc_musb_int_init(struct musb *musb);
#ifdef CONFIG_USB_OTG
extern bool musb_is_host(void);
extern void otg_a_device_init(void);
extern void otg_b_device_init(void);
#endif





#endif
