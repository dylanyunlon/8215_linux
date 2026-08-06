/*
 * Copyright (C) 2016 Autochips.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 */

#include <linux/of.h>
#include <linux/clk.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/phy/phy.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/usb/xhci_pdriver.h>
#include <generated/atc_project.h>
#include "xhci.h"
#include "xhci-atc.h"

#define AC8237_IOBASE		(0x10000000)

#define ATC_USB30_DBG
#ifdef ATC_USB30_DBG
#define u30_dbg(dev, fmt, args...) \
	printk(KERN_INFO "%s:%s "fmt, dev_driver_string(dev), __func__, ##args)
#else
#define u30_dbg(dev, fmt, args...) \
do {  \
} while (0)
#endif

/* Device for a quirk */
const char hcd_name[] = "atc-xhci";
static const struct hc_driver xhci_atc_plat_hc_driver;
static struct kobject *atc_usb30_kobj;
static struct gpio_desc *vbus_gpio;


static ssize_t atc_usb30_vbus_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	unsigned long	val;
	void *addr;
	u32 value = 0;

	if (sscanf(buf, "%lu", &val) < 1) {
		dev_err(dev, "Invalid  value\n");
		return -EINVAL;
	}

	if (val)
		value = 1;

	gpiod_direction_output(vbus_gpio, value);
	return n;
}

static ssize_t atc_usb30_vbus_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int vbus = 0;
	void *addr;

	vbus = gpiod_get_value(vbus_gpio);
	return sprintf(buf, "Vbus %s\n",
			vbus ? "on" : "off");
}
static DEVICE_ATTR(vbus, 0644, atc_usb30_vbus_show, atc_usb30_vbus_store);


static struct attribute *atc_usb30_attributes[] = {
	&dev_attr_vbus.attr,
	NULL
};

static const struct attribute_group atc_usb30_attr_group = {
	.attrs = atc_usb30_attributes,
};


static void xhci_plat_quirks(struct device *dev, struct xhci_hcd *xhci)
{
	xhci->quirks |= XHCI_PLAT;
}

static void xhci_atc_hw_init(struct device *dev)
{
	void *addr;
	u32 val;
	struct mtk_u3h_hw *u3h = dev_get_platdata(dev);

	// enable ckgen & prst
	addr = devm_ioremap(dev, AC8237_IOBASE, 0x10000);
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

	gpiod_direction_output(u3h->vbus, 1);

	// turn on ssusb_xhci_int
	val = readl(addr + 0x8064);
	u30_dbg(dev, "ssusb inten = 0x%08x\n", val);
	val |= (1<<25);
	writel(val, (addr + 0x8064));

	//monitor bus setting
	//writel(0x3088e3, (addr+0x80a8));
	//writel(0x2f, (addr + 0x1a0));
	//writel(0x400, (addr + 0x60));
	//writel(0x40, (addr + 0x94));
	//writel(0x33c000, (addr + 0x308));

	u30_dbg(dev, "xhci_atc_hw_init end\n");
}

static inline struct mtk_u3h_hw *hcd_to_u3h(struct usb_hcd *hcd)
{
	return dev_get_platdata(hcd->self.controller);
}

static int _read_xhci(struct usb_hcd *hcd, unsigned long addr, u32 u4Len)
{
	u32 u4Idx;
	u32 temp1, temp2, temp3, temp4;

	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	struct device *dev = hcd->self.controller;

	//no operation is needed
	if (!u4Len)
		return 0;

	//the maximum number of bytes
	if (u4Len > 0x1000)
		u4Len = 0x1000;

	//sum together xhci register base address and offset
	addr += ((unsigned long)xhci->cap_regs);

	for (u4Idx = 0; u4Idx < u4Len; u4Idx += 16) {
		temp1 = xhci_read_64(xhci, (__u32 __iomem *)(addr + u4Idx + 0));
		temp2 = xhci_read_64(xhci, (__u32 __iomem *)(addr + u4Idx + 4));
		temp3 = xhci_read_64(xhci, (__u32 __iomem *)(addr + u4Idx + 8));
		temp4 = xhci_read_64(xhci, (__u32 __iomem *)(addr + u4Idx + 12));
		u30_dbg(dev, "0x%08lx | %08x %08x %08x %08x\n",
				(addr + u4Idx), temp1, temp2, temp3, temp4);
	}

	return 0;
}

static int _read_ippc(struct mtk_u3h_hw * u3h, unsigned long addr, u32 u4Len)
{
	u32 u4Idx;
	u32 temp1, temp2, temp3, temp4;
	struct device *dev = u3h->dev;

	//no operation is needed
	if (!u4Len)
		return 0;

	//the maximum number of bytes
	if (u4Len > 0x800)
		u4Len = 0x800;

	//sum together ippc register base address and offset
	addr += u3h->ippc_virtual_base;

	for (u4Idx = 0; u4Idx < u4Len; u4Idx += 16) {
		temp1 = readl((__u32 __iomem *)(addr + u4Idx + 0));
		temp2 = readl((__u32 __iomem *)(addr + u4Idx + 4));
		temp3 = readl((__u32 __iomem *)(addr + u4Idx + 8));
		temp4 = readl((__u32 __iomem *)(addr + u4Idx + 12));
		u30_dbg(dev, "IPPC 0x%08lx | %08x %08x %08x %08x\n",
				(addr + u4Idx), temp1, temp2, temp3, temp4);
	}

	return 0;
}


static int xhci_atc_phy_init(struct mtk_u3h_hw *u3h)
{
	int i;
	int ret;

	for (i = 0; i < u3h->num_phys; i++) {
		ret = phy_init(u3h->phys[i]);
		if (ret)
			goto exit_phy;
	}
	return 0;

exit_phy:
	for (; i > 0; i--)
		phy_exit(u3h->phys[i - 1]);

	return ret;
}

static int xhci_atc_phy_exit(struct mtk_u3h_hw *u3h)
{
	int i;

	for (i = 0; i < u3h->num_phys; i++)
		phy_exit(u3h->phys[i]);

	return 0;
}

static int xhci_atc_phy_power_on(struct mtk_u3h_hw *u3h)
{
	int i;
	int ret;

	for (i = 0; i < u3h->num_phys; i++) {
		ret = phy_power_on(u3h->phys[i]);
		if (ret)
			goto power_off_phy;
	}
	return 0;

power_off_phy:
	for (; i > 0; i--)
		phy_power_off(u3h->phys[i - 1]);

	return ret;
}

static void xhci_atc_phy_power_off(struct mtk_u3h_hw *u3h)
{
	unsigned int i;

	for (i = 0; i < u3h->num_phys; i++)
		phy_power_off(u3h->phys[i]);
}

static int xhci_atc_host_enable(struct mtk_u3h_hw *u3h)
{
	struct device *dev = u3h->dev;
	int ret;

	xhci_atc_hw_init(dev);
	ret = xhci_atc_phy_init(u3h);
	if (ret) {
		u30_dbg(dev, "xhci_atc_phy_init fail ret =%d\n", ret);
		goto exit_phys;
	}
	ret = xhci_atc_phy_power_on(u3h);
	if (ret) {
		u30_dbg(dev, " xhci_atc_phy_power_on fail ret =%d\n", ret);
		goto power_off_phys;
	}
	reinitIP(u3h->dev);
	return ret;
	power_off_phys:
		xhci_atc_phy_power_off(u3h);

	exit_phys:
		xhci_atc_phy_exit(u3h);

	return ret;
}

static int xhci_atc_host_disable(struct mtk_u3h_hw *u3h)
{
	gpiod_direction_output(u3h->vbus, 0);
	xhci_atc_phy_power_off(u3h);
	xhci_atc_phy_exit(u3h);

	return 0;
}


/* called during probe() after chip reset completes */
static int xhci_plat_setup(struct usb_hcd *hcd)
{
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	struct mtk_u3h_hw *u3h_hw = hcd_to_u3h(hcd);
	struct device *dev = u3h_hw->dev;
	int ret;
	u32 val;
	void *addr;
	int i = 0;

	if (usb_hcd_is_primary_hcd(hcd)) {
		xhci_atc_hw_init(dev);
		ret = xhci_atc_phy_init(u3h_hw);
		if (ret) {
			u30_dbg(dev, "xhci_atc_phy_init fail ret =%d\n", ret);
			goto exit_phys;
		}
		ret = xhci_atc_phy_power_on(u3h_hw);
		if (ret) {
			u30_dbg(dev, " xhci_atc_phy_power_on fail ret =%d\n", ret);
			goto power_off_phys;
		}
		reinitIP(u3h_hw->dev);
	}

	return xhci_gen_setup(hcd, xhci_plat_quirks);

	power_off_phys:
		xhci_atc_phy_power_off(u3h_hw);
		//device_init_wakeup(dev, false);

	exit_phys:
		xhci_atc_phy_exit(u3h_hw);
	return ret;
}

static void xhci_check_usb_speed(struct mtk_u3h_hw *u3h)
{
	if (strstr(boot_command_line, "xh=full"))
		u3h->speed = USB11;
	else if (strstr(boot_command_line, "xh=high"))
		u3h->speed = USB20;
	else
		u3h->speed = USB30;
}

/**
 * usb_hcd_atc_plat_probe - initialize TI-based HCDs
 *
 * Allocates basic resources for this USB host controller, and
 * then invokes the start() method for the HCD associated with it
 * through the hotplug entry's driver_data.
 */
static int usb_hcd_atc_plat_probe(struct platform_device *pdev)
{
	const struct hc_driver *driver;
	struct device *dev = &pdev->dev;
	struct usb_hcd *hcd;
	struct resource *res;
	struct device_node *node = dev->of_node;
	struct xhci_hcd *xhci;
	struct clk *clk;
	struct phy *phy;
	struct mtk_u3h_hw *u3h_hw;
	int irq;
	int ret;
	int phy_num;
	int i, u3_port_num = 1;
	int status;
	u32 port_id, temp;
	u32 __iomem *addr;

	ret = -ENODEV;

#ifdef CONFIG_ATC_PRJ_ac823x_adas
	void *address;
	u32 val;
	// disable ckgen & prst
	address = devm_ioremap(dev, AC8237_IOBASE, 0x100);
	val = readl(address + 0xa0);
	val &= ~(0x3<<21);
	writel(val, (address + 0xa0));

	val = readl(address + 0xbc);
	val &= ~(1<<21);
	writel(val, (address + 0xbc));

	return ret;
#endif
	u30_dbg(dev, "probe is called\n");

	if (usb_disabled())
		return -ENODEV;

	u3h_hw = devm_kzalloc(dev, sizeof(*u3h_hw), GFP_KERNEL);
	if (!u3h_hw)
		return -ENOMEM;

	driver = &xhci_atc_plat_hc_driver;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return -ENODEV;

	pdev->dev.platform_data = u3h_hw;
	u3h_hw->dev = dev;
	u3h_hw->num_phys = of_count_phandle_with_args(node,
			"phys", "#phy-cells");
	if (u3h_hw->num_phys > 0) {
		u3h_hw->phys = devm_kcalloc(dev, u3h_hw->num_phys,
					sizeof(*u3h_hw->phys), GFP_KERNEL);
		if (!u3h_hw->phys)
			return -ENOMEM;
	} else {
		u3h_hw->num_phys = 0;
	}

	for (phy_num = 0; phy_num < u3h_hw->num_phys; phy_num++) {
		phy = devm_of_phy_get_by_index(dev, node, phy_num);
		if (IS_ERR(phy)) {
			ret = PTR_ERR(phy);
		}
		u3h_hw->phys[phy_num] = phy;
	}

	pm_runtime_enable(dev);
	pm_runtime_get_sync(dev);

	/* Initialize dma_mask and coherent_dma_mask to 32-bits */
	ret = dma_set_coherent_mask(dev, DMA_BIT_MASK(32));
	if (ret)
		return ret;
	if (!dev->dma_mask)
		dev->dma_mask = &dev->coherent_dma_mask;
	else
		dma_set_mask(dev, DMA_BIT_MASK(32));

	u3h_hw->vbus = __gpiod_get(dev, "u30vbus", GPIOD_ASIS);
	if (IS_ERR(u3h_hw->vbus )) {
		u30_dbg(dev, "failed to get vbus gpio\n");
		return -EINVAL;
	}
	vbus_gpio = u3h_hw->vbus;

	hcd = usb_create_hcd(driver, dev, dev_name(dev));
	if (!hcd) {
		u30_dbg(dev, "failed to create hcd with err %d\n", ret);
		ret = -ENOMEM;
		goto disable_pm;
	}
	u3h_hw->hcd = hcd;
	u30_dbg(dev, "Creat HCD success!dev name =%s\n", dev_name(dev));

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	hcd->regs = devm_ioremap_resource(dev, res);
	if (IS_ERR(hcd->regs)) {
		ret = PTR_ERR(hcd->regs);
		goto put_usb2_hcd;
	}
	hcd->rsrc_start = res->start;
	hcd->rsrc_len = resource_size(res);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	u3h_hw->ippc_virtual_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(u3h_hw->ippc_virtual_base)) {
		ret = PTR_ERR(u3h_hw->ippc_virtual_base);
		goto put_usb2_hcd;
	}

	u30_dbg(dev, "u3h ippc base=0x%p\n", u3h_hw->ippc_virtual_base);

	/*
	* Not all platforms have a clk so it is not an error if the
	* clock does not exists.
	*/
	clk = devm_clk_get(&pdev->dev, NULL);
	if (!IS_ERR(clk)) {
		ret = clk_prepare_enable(clk);
		if (ret)
			goto put_usb2_hcd;
	}

	u3h_hw->u3h_virtual_base = hcd->regs;
	u30_dbg(dev, "IPPC %p, U3VIR %p\n",
		u3h_hw->ippc_virtual_base, u3h_hw->u3h_virtual_base);

	xhci_check_usb_speed(u3h_hw);

	hcd->self.sg_tablesize = TRBS_PER_SEGMENT - 2;
	hcd->self.uses_dma = 1;

	ret = usb_add_hcd(hcd, irq, IRQF_SHARED);
	if (ret) {
		dev_dbg(&pdev->dev, "failed to add hcd with err %d\n", ret);
		goto disable_clk;
	}

	/* USB 2.0 roothub is stored in the platform_device now. */
	xhci = hcd_to_xhci(hcd);
	xhci->clk = clk;
	xhci->shared_hcd = usb_create_shared_hcd(driver, dev,
			dev_name(dev), hcd);
	if (!xhci->shared_hcd) {
		ret = -ENOMEM;
		goto dealloc_usb2_hcd;
	}

	/* we know this is the memory we want, no need to ioremap again */
	*((struct xhci_hcd **) xhci->shared_hcd->hcd_priv) = xhci;

	if (HCC_MAX_PSA(xhci->hcc_params) >= 4)
		xhci->shared_hcd->can_do_streams = 1;

	ret = usb_add_hcd(xhci->shared_hcd, irq, IRQF_SHARED);
	if (ret)
		goto put_usb3_hcd;


	if (u3h_hw->speed != USB30) {
		u3_port_num = get_xhci_u3_port_num(hcd->self.controller);
		for(i=1; i<=u3_port_num; i++){
				addr = &xhci->op_regs->port_status_base + NUM_PORT_REGS*((i - 1) & 0xff);
				temp = readl(addr);
				temp = xhci_port_state_to_neutral(temp);
				temp &= ~PORT_POWER;
				writel( temp, addr);
			}
	}

	if (u3h_hw->speed == USB11) {
		u32 val;
		val = readl (u3h_hw->u3h_virtual_base + 0x3404);
		val &= (~0x20);
		writel(val, u3h_hw->u3h_virtual_base + 0x3404);
	}
	atc_usb30_kobj = kobject_create_and_add("atcxhci", NULL);
	if (atc_usb30_kobj) {
		status = sysfs_create_group(atc_usb30_kobj, &atc_usb30_attr_group);
		if (status < 0)
			u30_dbg(dev, "create_group fail status %d\n", status);
	}
	//_read_xhci(hcd, 0, 0x1000);
	//_read_ippc(u3h_hw, 0, 0x800);

	return 0;

	put_usb3_hcd:
		usb_put_hcd(xhci->shared_hcd);

	dealloc_usb2_hcd:
		usb_remove_hcd(hcd);

	disable_clk:
		if (!IS_ERR(clk))
			clk_disable_unprepare(clk);

	put_usb2_hcd:
		usb_put_hcd(hcd);
		if (!IS_ERR(u3h_hw->vbus ))
			gpiod_put(u3h_hw->vbus);

	disable_pm:
		pm_runtime_put_sync(dev);
		pm_runtime_disable(dev);

	return ret;
}


static int usb_hcd_atc_plat_remove(struct platform_device *dev)
{
	struct mtk_u3h_hw *u3h = dev_get_platdata(&dev->dev);
	struct usb_hcd *hcd = u3h->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);

	if (atc_usb30_kobj)
		sysfs_remove_group(atc_usb30_kobj, &atc_usb30_attr_group);

	usb_remove_hcd(xhci->shared_hcd);
	//xhci_atc_phy_power_off(u3h);
	//xhci_atc_phy_exit(u3h);
	//device_init_wakeup(&dev->dev, false);

	usb_remove_hcd(hcd);
	usb_put_hcd(xhci->shared_hcd);
	usb_put_hcd(hcd);

	if (!IS_ERR(u3h->vbus ))
		gpiod_put(u3h->vbus);

	pm_runtime_put_sync(&dev->dev);
	pm_runtime_disable(&dev->dev);

	return 0;
}

static int __maybe_unused xhci_atc_suspend(struct device *dev)
{
	struct mtk_u3h_hw *u3h = dev_get_platdata(dev);
	struct usb_hcd *hcd = u3h->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	int ret;

	ret = xhci_suspend(xhci, device_may_wakeup(dev));
	if (ret ) {
		u30_dbg(dev, "%s: fail (%d)\n", __func__, ret);
		return ret;
	}

	ret = xhci_atc_host_disable(u3h);
	if (ret)
		return ret;

	return 0;
}

static int __maybe_unused xhci_atc_resume(struct device *dev)
{
	struct mtk_u3h_hw *u3h = dev_get_platdata(dev);
	struct usb_hcd *hcd = u3h->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	int ret;

	ret = xhci_atc_host_enable(u3h);
	if (ret)
		return ret;

	return xhci_resume(xhci, 1);
}

static const struct dev_pm_ops xhci_atc_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(xhci_atc_suspend, xhci_atc_resume)
};
#define DEV_PM_OPS IS_ENABLED(CONFIG_PM) ? &xhci_atc_pm_ops : NULL

static void usb_hcd_atc_plat_shutdown(struct platform_device *dev)
{
	struct mtk_u3h_hw *u3h = dev_get_platdata(&dev->dev);
	struct usb_hcd *hcd = u3h->hcd;

	u30_dbg(&dev->dev, "%s\n", __func__);

	hcd->driver->shutdown(hcd);
	gpiod_direction_output(vbus_gpio, 0);
	return;
}

static const struct of_device_id atc_xhci_of_match[] = {
	{ .compatible = "autochips,ac823x-xhci"},
	{ },
};

MODULE_DEVICE_TABLE(of, atc_xhci_of_match);

static struct platform_driver xhci_atc_plat_driver = {

	.probe =	usb_hcd_atc_plat_probe,
	.remove =	usb_hcd_atc_plat_remove,
	.shutdown = 	usb_hcd_atc_plat_shutdown,

	.driver = {
		.name = (char *) hcd_name,
		.pm = DEV_PM_OPS,
		.of_match_table = of_match_ptr(atc_xhci_of_match),
	}
};

static int __init xhci_atc_init(void)
{
	xhci_init_driver(&xhci_atc_plat_hc_driver, xhci_plat_setup);
	return platform_driver_register(&xhci_atc_plat_driver);
}
module_init(xhci_atc_init);

static void __exit xhci_atc_exit(void)
{
	platform_driver_unregister(&xhci_atc_plat_driver);
}
module_exit(xhci_atc_exit);

