#ifndef __MUSB_ATC_MUSB_H__
#define __MUSB_ATC_MUSB_H__


#include <generated/atc_project.h>
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#ifdef CONFIG_OF


#define USBPHY_READ8(offset)          musb_readb((void __iomem *)(((unsigned long)h_atc_musb->phy_io_priv)+0x900), offset)
#define USBPHY_WRITE8(offset, value)	musb_writeb((void __iomem *)\
		(((unsigned long)h_atc_musb->phy_io_priv)+0x900), offset, value);

#define USBPHY_SET8(offset, mask)     USBPHY_WRITE8(offset, (USBPHY_READ8(offset)) | (mask))
#define USBPHY_CLR8(offset, mask)     USBPHY_WRITE8(offset, (USBPHY_READ8(offset)) & (~mask))

#define USBPHY_READ16(offset)          musb_readw((void __iomem *)(((unsigned long)h_atc_musb->phy_io_priv)+0x900), offset)
#define USBPHY_WRITE16(offset, value)  musb_writew((void __iomem *)\
		(((unsigned long)h_atc_musb->phy_io_priv)+0x900), offset, value)
#define USBPHY_SET16(offset, mask)     USBPHY_WRITE16(offset, (USBPHY_READ16(offset)) | (mask))
#define USBPHY_CLR16(offset, mask)     USBPHY_WRITE16(offset, (USBPHY_READ16(offset)) & (~mask))

#define USBPHY_READ32(offset)          musb_readl((void __iomem *)(((unsigned long)h_atc_musb->phy_io_priv)+0x900), offset)
#define USBPHY_WRITE32(offset, value)  musb_writel((void __iomem *)\
		(((unsigned long)h_atc_musb->phy_io_priv)+0x900), offset, value)
#define USBPHY_SET32(offset, mask)     USBPHY_WRITE32(offset, (USBPHY_READ32(offset)) | (mask))
#define USBPHY_CLR32(offset, mask)     USBPHY_WRITE32(offset, (USBPHY_READ32(offset)) & (~mask))

#ifdef ATC_UART_USB_SWITCH
#define UART2_BASE 0x11003000
#endif

#else

#include <mach/ac_reg_base.h>
#define USBPHY_READ8(offset)          readb((void __iomem *)(USB_SIF_BASE+0x900+offset))
#define USBPHY_WRITE8(offset, value)  writeb(value, (void __iomem *)(USB_SIF_BASE+0x900+offset))
#define USBPHY_SET8(offset, mask)     USBPHY_WRITE8(offset, (USBPHY_READ8(offset)) | (mask))
#define USBPHY_CLR8(offset, mask)     USBPHY_WRITE8(offset, (USBPHY_READ8(offset)) & (~mask))

#define USBPHY_READ16(offset)          readw((void __iomem *)(USB_SIF_BASE+0x900+offset))
#define USBPHY_WRITE16(offset, value)  writew(value, (void __iomem *)(USB_SIF_BASE+0x900+offset))
#define USBPHY_SET16(offset, mask)     USBPHY_WRITE16(offset, (USBPHY_READ16(offset)) | (mask))
#define USBPHY_CLR16(offset, mask)     USBPHY_WRITE16(offset, (USBPHY_READ16(offset)) & (~mask))

#define USBPHY_READ32(offset)          readl((void __iomem *)(USB_SIF_BASE+0x900+offset))
#define USBPHY_WRITE32(offset, value)  writel(value, (void __iomem *)(USB_SIF_BASE+0x900+offset))
#define USBPHY_SET32(offset, mask)     USBPHY_WRITE32(offset, (USBPHY_READ32(offset)) | (mask))
#define USBPHY_CLR32(offset, mask)     USBPHY_WRITE32(offset, (USBPHY_READ32(offset)) & (~mask))

#endif

#else
#ifdef CONFIG_OF

#define MUSB_ASSERT(x)   if (!(x)) BUG();

#define USBPHY_READ8(offset)          musb_readb((void __iomem *)(((unsigned long)h_atc_musb->phy_io_priv)), offset)
#define USBPHY_WRITE8(offset, value)	musb_writeb((void __iomem *)\
		(((unsigned long)h_atc_musb->phy_io_priv)), offset, value);

#define USBPHY_SET8(offset, mask)     USBPHY_WRITE8(offset, (USBPHY_READ8(offset)) | (mask))
#define USBPHY_CLR8(offset, mask)     USBPHY_WRITE8(offset, (USBPHY_READ8(offset)) & (~mask))

#define USBPHY_READ16(offset)          musb_readw((void __iomem *)(((unsigned long)h_atc_musb->phy_io_priv)), offset)
#define USBPHY_WRITE16(offset, value)  musb_writew((void __iomem *)\
		(((unsigned long)atc_musb->xceiv->io_priv)), offset, value)
#define USBPHY_SET16(offset, mask)     USBPHY_WRITE16(offset, (USBPHY_READ16(offset)) | (mask))
#define USBPHY_CLR16(offset, mask)     USBPHY_WRITE16(offset, (USBPHY_READ16(offset)) & (~mask))

#define USBPHY_READ32(offset)          musb_readl((void __iomem *)(((unsigned long)h_atc_musb->phy_io_priv)), offset)
#define USBPHY_WRITE32(offset, value)  musb_writel((void __iomem *)\
		(((unsigned long)h_atc_musb->phy_io_priv)), offset, value)
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
extern void h_usb_phy_poweron(void);
extern void h_usb_phy_recover(void);
extern void h_usb_phy_savecurrent(void);
extern void h_usb_phy_context_restore(void);
extern void h_usb_phy_context_save(void);

extern void h_platform_musb_suspend(struct musb* );
extern void h_platform_musb_resume(struct musb* );
extern void h_usb_clock_enable(void);

/* host and otg */
extern void h_ac_usb_otg_init(struct musb *musb);
extern void h_ac_usb_iddig_int(struct musb *musb);
extern void h_musb_session_restart(struct musb *musb);
extern void atc_h_musb_host_init(void);
extern void atc_h_musb_int_init(struct musb *musb);

#endif
