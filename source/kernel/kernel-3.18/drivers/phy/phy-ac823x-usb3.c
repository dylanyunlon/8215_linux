/*
 * Copyright (c) 2016 Autochips Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <dt-bindings/phy/phy.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>

#define PHY_USB30_DBG

#ifdef PHY_USB30_DBG
#define u3phy_dbg(dev, fmt, args...) \
	printk(KERN_INFO "%s:%s "fmt, dev_driver_string(dev), __func__, ##args)
#else
#define u3phy_dbg(dev, fmt, args...) \
do {  \
} while (0)
#endif


struct ac823x_phy_instance {
	struct phy *phy;
	void __iomem *port_base;
	u32 index;
	u8 type;
};

struct ac823x_u3phy {
	struct device *dev;
	void __iomem *sif_base;
	struct ac823x_phy_instance **phys;
	int nphys;
};

static void hs_slew_rate_calibrate(struct ac823x_u3phy *u3phy,
	struct ac823x_phy_instance *instance)
{

}

static void phy_instance_init(struct ac823x_u3phy *u3phy,
	struct ac823x_phy_instance *instance)
{
	void __iomem *port_base = instance->port_base;
	u32 index = instance->index;
	u32 tmp;

	u3phy_dbg(u3phy->dev, "(%d)\n", index);
}

static void phy_instance_power_on(struct ac823x_u3phy *u3phy,
	struct ac823x_phy_instance *instance)
{
	void __iomem *port_base = instance->port_base;
	u32 index = instance->index;
	u32 tmp;

	/*TODO: maybe need add later*/
	u3phy_dbg(u3phy->dev, "(%d)\n", index);
}

static void phy_instance_power_off(struct ac823x_u3phy *u3phy,
	struct ac823x_phy_instance *instance)
{
	void __iomem *port_base = instance->port_base;
	u32 index = instance->index;
	u32 tmp;

	/*TODO: maybe need add later*/
	u3phy_dbg(u3phy->dev, "(%d)\n", index);
}

static void phy_instance_exit(struct ac823x_u3phy *u3phy,
	struct ac823x_phy_instance *instance)
{
	void __iomem *port_base = instance->port_base;
	u32 index = instance->index;
	u32 tmp;

	/*TODO: maybe need add later*/
	u3phy_dbg(u3phy->dev, "\n");
}

static int ac823x_phy_init(struct phy *phy)
{
	struct ac823x_phy_instance *instance = phy_get_drvdata(phy);
	struct ac823x_u3phy *u3phy = dev_get_drvdata(phy->dev.parent);
	int ret;
	u32 data;

	u3phy_dbg(u3phy->dev, "type=%d\n", instance->type);

	if (instance->type == PHY_TYPE_USB2) {;
		//just turn on internal R
		data = readl(instance->port_base);
		writel(data | (1<<14), instance->port_base);
		u3phy_dbg(u3phy->dev, "u2phya@%p = 0x%08x\n",
	   			instance->port_base, readl(instance->port_base));

		phy_instance_init(u3phy, instance);
	}

	return 0;
}

static int ac823x_phy_power_on(struct phy *phy)
{
	struct ac823x_phy_instance *instance = phy_get_drvdata(phy);
	struct ac823x_u3phy *u3phy = dev_get_drvdata(phy->dev.parent);

	u3phy_dbg(u3phy->dev, "\n");

	phy_instance_power_on(u3phy, instance);
	hs_slew_rate_calibrate(u3phy, instance);
	return 0;
}

static int ac823x_phy_power_off(struct phy *phy)
{
	struct ac823x_phy_instance *instance = phy_get_drvdata(phy);
	struct ac823x_u3phy *u3phy = dev_get_drvdata(phy->dev.parent);

	u3phy_dbg(u3phy->dev, "\n");

	phy_instance_power_off(u3phy, instance);
	return 0;
}

static int ac823x_phy_exit(struct phy *phy)
{
	struct ac823x_phy_instance *instance = phy_get_drvdata(phy);
	struct ac823x_u3phy *u3phy = dev_get_drvdata(phy->dev.parent);

	u3phy_dbg(u3phy->dev, "\n");

	phy_instance_exit(u3phy, instance);
	return 0;
}

static struct phy *ac823x_phy_xlate(struct device *dev,
					struct of_phandle_args *args)
{
	struct ac823x_u3phy *u3phy = dev_get_drvdata(dev);
	struct ac823x_phy_instance *instance = NULL;
	struct device_node *phy_np = args->np;
	int index;

	u3phy_dbg(u3phy->dev, "\n");

	if (args->args_count != 1) {
		dev_err(dev, "invalid number of cells in 'phy' property\n");
		return ERR_PTR(-EINVAL);
	}

	for (index = 0; index < u3phy->nphys; index++)
		if (phy_np == u3phy->phys[index]->phy->dev.of_node) {
			instance = u3phy->phys[index];
			break;
		}

	if (!instance) {
		dev_err(dev, "failed to find appropriate phy\n");
		return ERR_PTR(-EINVAL);
	}

	instance->type = args->args[0];

	if (!(instance->type == PHY_TYPE_USB2 ||
	      instance->type == PHY_TYPE_USB3)) {
		dev_err(dev, "unsupported device type: %d\n", instance->type);
		return ERR_PTR(-EINVAL);
	}

	return instance->phy;
}

static struct phy_ops ac823x_u3phy_ops = {
	.init		= ac823x_phy_init,
	.exit		= ac823x_phy_exit,
	.power_on	= ac823x_phy_power_on,
	.power_off	= ac823x_phy_power_off,
	.owner		= THIS_MODULE,
};

static int ac823x_u3phy_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct device_node *child_np;
	struct phy_provider *provider;
	struct resource *sif_res;
	struct ac823x_u3phy *u3phy;
	struct resource res;
	int port, retval;

	u3phy_dbg(dev, "\n");

	u3phy = devm_kzalloc(dev, sizeof(*u3phy), GFP_KERNEL);
	if (!u3phy)
		return -ENOMEM;

	u3phy->nphys = of_get_child_count(np);
	u3phy->phys = devm_kcalloc(dev, u3phy->nphys,
				       sizeof(*u3phy->phys), GFP_KERNEL);
	if (!u3phy->phys)
		return -ENOMEM;

	u3phy->dev = dev;
	platform_set_drvdata(pdev, u3phy);

	sif_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	u3phy->sif_base = devm_ioremap_resource(dev, sif_res);
	u3phy_dbg(dev, "sif_res=0x%p, u3phy->sif_base=0x%p",
		sif_res->start, u3phy->sif_base);
	if (IS_ERR(u3phy->sif_base)) {
		dev_err(dev, "failed to remap sif regs\n");
		return PTR_ERR(u3phy->sif_base);
	}

	port = 0;
	for_each_child_of_node(np, child_np) {
		struct ac823x_phy_instance *instance;
		struct phy *phy;

		instance = devm_kzalloc(dev, sizeof(*instance), GFP_KERNEL);
		if (!instance) {
			retval = -ENOMEM;
			goto put_child;
		}

		u3phy->phys[port] = instance;

		phy = devm_phy_create(dev, child_np, &ac823x_u3phy_ops, NULL);
		if (IS_ERR(phy)) {
			dev_err(dev, "failed to create phy\n");
			retval = PTR_ERR(phy);
			goto put_child;
		}

		retval = of_address_to_resource(child_np, 0, &res);
		if (retval) {
			dev_err(dev, "failed to get address resource(id-%d)\n",
				port);
			goto put_child;
		}

		instance->port_base = devm_ioremap_resource(&phy->dev, &res);

		u3phy_dbg(dev, "port_base=0x%p\n", instance->port_base);

		if (IS_ERR(instance->port_base)) {
			dev_err(dev, "failed to remap phy regs\n");
			retval = PTR_ERR(instance->port_base);
			goto put_child;
		}

		instance->phy = phy;
		instance->index = port;
		phy_set_drvdata(phy, instance);
		port++;
	}

	provider = devm_of_phy_provider_register(dev, ac823x_phy_xlate);

	u3phy_dbg(dev, "end\n");

	return PTR_ERR_OR_ZERO(provider);
put_child:
	of_node_put(child_np);
	return retval;
}

static const struct of_device_id ac823x_u3phy_id_table[] = {
	{ .compatible = "autochips,ac823x-u3phy", },
	{ },
};
MODULE_DEVICE_TABLE(of, ac823x_u3phy_id_table);

static struct platform_driver ac823x_u3phy_driver = {
	.probe		= ac823x_u3phy_probe,
	.driver		= {
		.name	= "ac823x-u3phy",
		.of_match_table = ac823x_u3phy_id_table,
	},
};

module_platform_driver(ac823x_u3phy_driver);

MODULE_DESCRIPTION("ac823x USB PHY driver");
MODULE_LICENSE("GPL v2");
