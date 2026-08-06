#include "xhci-atc.h"
#include "xhci-atc-power.h"
#include "xhci-atc-scheduler.h"
#include <linux/kernel.h>       /* printk() */
#include <linux/slab.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/jiffies.h>

static const char hcd_name[] = "xhci-hcd";

int get_xhci_u3_port_num(struct device *dev){
	struct mtk_u3h_hw *u3h_hw;
	__u32 __iomem *addr;
	u32 data;
	int u3_port_num;

	u3h_hw = dev->platform_data;

	addr = (u32 __iomem *)(u3h_hw->ippc_virtual_base + U3H_SSUSB_IP_XHCI_CAP);
	data = readl(addr);
	u3_port_num = data & SSUSB_IP_XHCI_U3_PORT_NO;

	return u3_port_num;
}

int get_xhci_u2_port_num(struct device *dev){
	struct mtk_u3h_hw *u3h_hw;
	__u32 __iomem *addr;
	u32 data;
	int u2_port_num;

	u3h_hw = dev->platform_data;

	addr = (u32 __iomem *)(u3h_hw->ippc_virtual_base + U3H_SSUSB_IP_XHCI_CAP);
	data = readl(addr);

	u2_port_num = (data & SSUSB_IP_XHCI_U2_PORT_NO) >> 8;

	return u2_port_num;
}

void set_frame_cnt_ck(struct device *dev){}
void setInitialReg(struct device *dev){
	int i;
	struct mtk_u3h_hw *u3h_hw;
	__u32 __iomem *usb3_csr_base;
	__u32 __iomem *usb2_csr_base;

	__u32 __iomem *addr;
	u32 data;
	int u3_port_num,u2_port_num;

	u3h_hw = dev->platform_data;
	usb3_csr_base = (u32 __iomem *)(u3h_hw->u3h_virtual_base + SSUSB_USB3_CSR_OFFSET);
	usb2_csr_base = (u32 __iomem *)(u3h_hw->u3h_virtual_base + SSUSB_USB2_CSR_OFFSET);

	//get u3 & u2 port num
	u3_port_num = get_xhci_u3_port_num(dev);
	u2_port_num = get_xhci_u2_port_num(dev);
	//u3_port_num = 0;
	if (u3h_hw->speed == USB30) {
		for(i = 0; i < u3_port_num; i++){
			//set MAC reference clock speed
			addr = usb3_csr_base + U3H_UX_EXIT_LFPS_TIMING_PARAMETER;
			data = ((300*U3_REF_CK_VAL + (1000-1)) / 1000);
			u3h_writelmsk(addr,data,RX_UX_EXIT_LFPS_REF);

			addr = usb3_csr_base + U3H_REF_CK_PARAMETER;
			data = U3_REF_CK_VAL;
			u3h_writelmsk(addr,data,REF_1000NS);

			//set SYS_CK
			addr = usb3_csr_base + U3H_TIMING_PULSE_CTRL;
			data = U3_SYS_CK_VAL;
			u3h_writelmsk(addr,data,CNT_1US_VALUE);
		}
	}
	for(i=0; i<u2_port_num; i++){
		addr = usb2_csr_base + U3H_USB20_TIMING_PARAMETER;
		data = U3_SYS_CK_VAL;
		u3h_writelmsk(addr,data,TIME_VALUE_1US);
	}
}

static void mt3365_phy_init(struct device *dev)
{
	struct mtk_u3h_hw *u3h_hw;
	void __iomem *addr, *base;
	u32 data;

	pr_info("%s\n", __func__);

	u3h_hw = dev->platform_data;
	pr_info("IPPC %p, U3VIR %p\n", u3h_hw->ippc_virtual_base, u3h_hw->u3h_virtual_base);
	return;

	base = u3h_hw->u3phy_virtual_base + 0x800;//U2 PHY BASE
	addr = base;
	//just turn on internal R
	data = readl(addr);
	writel(data | (1<<14), addr);
	pr_info("u2phya@%p = 0x%08x\n", addr, readl(addr));

	/*
	// USBPLL_FBDIV[6:0], BGR_DIV[1:0]
	data = readl(addr);
	data &= ~(0x7f<<16) ;
	data |= ((0x13<<16) | (0x3<<2));
	writel(data, addr);
	addr = base + 0x68;
	data = readl(addr);
	//force_uart_en, 0x68[26]
	writel(data & ~(0x1<<26), addr);
	addr = base + 0x6C;
	data = readl(addr);
	//force_uart_en, 0x6c[16]
	writel(data & ~(0x1<<16), addr);
	addr = base + 0x20;
	data = readl(addr);
	//rg_usb20_gpio_ctl, usb20_gpio_mode
	writel(data & ~(0x3<<8), addr);
	addr = base + 0x1c;
	data = readl(addr);
	//RG_USB20_PHY_REV[7]
	writel(data & ~(0x1<<7), addr);
	addr = base + 0x68;
	data = readl(addr);
	//rg_suspendm:0x68[3], force_suspendm:0x68[18]
	pr_err("U2PHYDTM0@%p=0x%08x\n", addr, data);
	writel(data & ~((0x1<<3) | (0x1<<18)), addr);
	data = readl(addr);
	pr_err("U2PHYDTM0=0x%08x\n", data);
	*/

}
void reinitIP(struct device *dev){

	mt3365_phy_init(dev);
	enableAllClockPower(dev);
	setInitialReg(dev);
	mtk_xhci_scheduler_init(dev);
}

/* return code */
#define RET_SUCCESS 0
#define RET_FAIL -1

int chk_frmcnt_clk(struct usb_hcd *hcd){ return 0;}

#if CFG_DEV_U3H0
static struct resource mtk_resource_u3h0[] = {
	[0] = {
		    .start = U3H_IRQ0,
		    .end   = U3H_IRQ0,
		    .flags = IORESOURCE_IRQ,
	},
	[1] = {
            .name = "u3h",
			 /*physical address*/
		    .start = U3H_BASE0,
		    .end   = U3H_BASE0 + MTK_U3H_SIZE - 1,
		    .flags = IORESOURCE_MEM,
	},
	[2] = {
            .name = "ippc",
			 /*physical address*/
		    .start = IPPC_BASE0,
		    .end   = IPPC_BASE0 + MTK_IPPC_SIZE - 1,
		    .flags = IORESOURCE_MEM,
	},
	[3] = {
			.name = "u3phy",
			 /*physical address*/
			.start = U3PHY_BASE0,
			.end   = U3PHY_BASE0 + MTK_U3PHY_SIZE - 1,
			.flags = IORESOURCE_MEM,
	},

};
#endif

#if CFG_DEV_U3H1
static struct resource mtk_resource_u3h1[] = {
	[0] = {
		    .start = U3H_IRQ1,
		    .end   = U3H_IRQ1,
		    .flags = IORESOURCE_IRQ,
	},
	[1] = {
            .name = "u3h",
			 /*physical address*/
		    .start = U3H_BASE1,
		    .end   = U3H_BASE1 + MTK_U3H_SIZE - 1,
		    .flags = IORESOURCE_MEM,
	},
	[2] = {
            .name = "ippc",
			 /*physical address*/
		    .start = IPPC_BASE1,
		    .end   = IPPC_BASE1 + MTK_IPPC_SIZE - 1,
		    .flags = IORESOURCE_MEM,
	},

};
#endif

#if CFG_DEV_U3H0
struct mtk_u3h_hw u3h_hw0;
#endif

#if CFG_DEV_U3H1
struct mtk_u3h_hw u3h_hw1;
#endif

static u64 mtk_u3h_dma_mask = 0xffffffffffffffffULL;
//static u64 mtk_u3h_dma_mask = 0x1fffffffUL;

static struct platform_device mtk_device_u3h[] = {
#if CFG_DEV_U3H0
	{
	    .name          = hcd_name,
	    .id            = 0,
	    .resource      = mtk_resource_u3h0,
	    .num_resources = ARRAY_SIZE(mtk_resource_u3h0),
	    .dev           = {
                            .platform_data = &u3h_hw0,
						    .dma_mask = &mtk_u3h_dma_mask,
				            .coherent_dma_mask = 0xffffffffffffffffULL,
                         },
     },
#endif
#if CFG_DEV_U3H1
	{
	    .name          = hcd_name,
	    .id            = 1,
	    .resource      = mtk_resource_u3h1,
	    .num_resources = ARRAY_SIZE(mtk_resource_u3h1),
	    .dev           = {
                            .platform_data = &u3h_hw1,
						    .dma_mask = &mtk_u3h_dma_mask,
				            .coherent_dma_mask = 0xffffffffUL,
                         },
     },
#endif

};

struct platform_device * get_mtk_device_u3h(u32 id){
	return &mtk_device_u3h[id];
}

static 	int __init mtk_u3h_init(void)
{
	int ret;
	int i;
	int u3h_dev_num;
	u32 val;
	void *addr;

	return;
	pr_info("mtk_u3h_init start\n");

	u3h_dev_num = sizeof(mtk_device_u3h) / sizeof(mtk_device_u3h[0]);
	for (i = 0; i < u3h_dev_num; i++){
	        ret = platform_device_register(&mtk_device_u3h[i]);
	        if (ret != 0){
			return ret;
	        }
	}

#ifdef PRJ_MT3365
	//writel(VAL_SSUSB_RST, (void *) (CKGEN_BASE + 0xbc));
	//writel(0x500, (void *) (CKGEN_BASE + 0x30));
#define MT3365_IOBASE	0x10000000
	// enable ckgen & prst
	addr = ioremap(MT3365_IOBASE, 0x10000);
	val = readl(addr + 0xa0);
	val |= (0x3<<21);
	writel(val, (addr + 0xa0));
	val = readl(addr + 0xbc);
	val |= (1<<21);
	writel(val, (addr + 0xbc));
	// switch clk mux to 108MHz
	val = readl(addr + 0x18);
	val |= 0x400;
	writel(val, (addr + 0x18));
	// turn on vbus, GPIO3
	val = readl(addr + 0x74);
	val |= (1<<3);
	writel(val, (addr + 0x74));
	val = readl(addr + 0xe0);
	val |= (1<<3);
	writel(val, (addr + 0xe0));
	// turn on ssusb_xhci_int
	val = readl(addr + 0x8064);
	pr_info("ssusb inten = 0x%08x\n", val);
	val |= (1<<25);
	writel(val, (addr + 0x8064));
	//monitor bus setting
	writel(0x3088e3, (addr+0x80a8));
	writel(0x2f, (addr + 0x1a0));
	writel(0x400, (addr + 0x60));
	writel(0x40, (addr + 0x94));
	writel(0x33c000, (addr + 0x308));
	pr_info("mtk_u3h_init end\n");
#endif

	return ret;
}

static void __exit mtk_u3h_cleanup(void)
{
	int u3h_dev_num;
	int i;

	return;
	u3h_dev_num = sizeof(mtk_device_u3h) / sizeof(mtk_device_u3h[0]);
	for (i = 0; i < u3h_dev_num; i++)
		platform_device_unregister(&mtk_device_u3h[i]);
}

rootfs_initcall(mtk_u3h_init);
module_exit(mtk_u3h_cleanup);
MODULE_LICENSE("GPL");
