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

#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/machine.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/bitops.h>
#include <linux/mfd/syscon.h>
#include <linux/delay.h>
#include <asm/hardirq.h>
#include <linux/interrupt.h>


#include "../core.h"
#include "../pinconf.h"
#include "../pinctrl-utils.h"
#include "pinctrl-ac823x.h"

#include "../../gpio/gpio_ac823x_pinmux.h"
#include "../../gpio/gpio_ac823x_pinmux_table.h"
#include <linux/gpio.h>


static const struct mtk_pinctrl_devdata ac83xx_pinctrl_data = {
	.pins = mtk_pins_mt8163,//pin引脚定义
	.npins = ARRAY_SIZE(mtk_pins_mt8163),//pin引脚总数
	//.grp_desc = mt8163_drv_grp,
	//.n_grp_cls = ARRAY_SIZE(mt8163_drv_grp),
	//.pin_drv_grp = mt8163_pin_drv,
	//.n_pin_drv_grps = ARRAY_SIZE(mt8163_pin_drv),
	//.spec_pull_set = mt8163_spec_pull_set,
	//.spec_ies_smt_set = mt8163_ies_smt_set,
	.dir_offset = 0x0000,//寄存器偏移量
	.pullen_offset = 0x0100,
	.pullsel_offset = 0x0200,
	.dout_offset = 0x0400,
	.din_offset = 0x0500,
	.pinmux_offset = 0x0600,
	.type1_start = 155,
	.type1_end = 155,
	.port_shf = 4,//移动4位
	.port_mask = 0xf,//掩码f,也是4位
	.port_align = 4,
	/*.eint_offsets = {
		.name = "mt8163_eint",
		.stat      = 0x000,
		.ack       = 0x040,
		.mask      = 0x080,
		.mask_set  = 0x0c0,
		.mask_clr  = 0x100,
		.sens      = 0x140,
		.sens_set  = 0x180,
		.sens_clr  = 0x1c0,
		.soft      = 0x200,
		.soft_set  = 0x240,
		.soft_clr  = 0x280,
		.pol       = 0x300,
		.pol_set   = 0x340,
		.pol_clr   = 0x380,
		.dom_en    = 0x400,
		.dbnc_ctrl = 0x500,
		.dbnc_set  = 0x600,
		.dbnc_clr  = 0x700,
		.port_mask = 7,
		.ports     = 6,
	},*/
	.ap_num = 204,
	.db_cnt = 64,
};

#if 0
static const struct mtk_desc_function *mtk_pctrl_find_function_by_pin(
		struct mtk_pinctrl *pctl, u32 pin_num, u32 fnum)
{
	const struct mtk_desc_pin *pin = pctl->devdata->pins + pin_num;
	const struct mtk_desc_function *func = pin->functions;

	while (func && func->name) {
		if (func->muxval == fnum)
			return func;
		func++;
	}

	return NULL;
}
#endif

static bool mtk_pctrl_is_function_valid(struct mtk_pinctrl *pctl,
		u32 pin_num, u32 fnum)
{
	return true;
#if 0
	int i;
	for (i = 0; i < pctl->devdata->npins; i++) {
		const struct mtk_desc_pin *pin = pctl->devdata->pins + i;

		if (pin->pin.number == pin_num) {
			const struct mtk_desc_function *func =
					pin->functions;

			while (func && func->name) {
				if (func->muxval == fnum)
					return true;
				func++;
			}

			break;
		}
	}

	return false;
#endif
}



static int mtk_pmx_get_funcs_cnt(struct pinctrl_dev *pctldev)
{
	return ARRAY_SIZE(mtk_gpio_functions);
}

static const char *mtk_pmx_get_func_name(struct pinctrl_dev *pctldev,
					   unsigned selector)
{
	return mtk_gpio_functions[selector];
}

static int mtk_pmx_get_func_groups(struct pinctrl_dev *pctldev,
				     unsigned function,
				     const char * const **groups,
				     unsigned * const num_groups)
{
	struct mtk_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	*groups = pctl->grp_names;
	*num_groups = pctl->ngroups;

	return 0;
}

#if 0
static int mtk_pmx_set_mode(struct pinctrl_dev *pctldev,
		unsigned long pin, unsigned long mode)
{

    //For autochips Call 
	//GPIO_MultiFun_Set(int i4GpioNum,int i4FuncSel) instead

	unsigned int reg_addr;
	unsigned char bit;
	unsigned int val;
	int ret;
	unsigned int mask = (1L << GPIO_MODE_BITS) - 1;
	struct mtk_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	reg_addr = ((pin / MAX_GPIO_MODE_PER_REG) << pctl->devdata->port_shf)
			+ pctl->devdata->pinmux_offset;
	if (pctl->devdata->spec_set_gpio_mode) {
		ret = pctl->devdata->spec_set_gpio_mode(pin|0x80000000, mode);
		return 0;
	}

	bit = pin % MAX_GPIO_MODE_PER_REG;
	mask <<= (GPIO_MODE_BITS * bit);
	val = (mode << (GPIO_MODE_BITS * bit));


	return regmap_update_bits(mtk_get_regmap(pctl, pin),
			reg_addr, mask, val);
    
    return 0;
}
#endif

/*static const struct mtk_desc_pin *
mtk_find_pin_by_eint_num(struct mtk_pinctrl *pctl, unsigned int eint_num)
{
	int i;
	const struct mtk_desc_pin *pin;

	for (i = 0; i < pctl->devdata->npins; i++) {
		pin = pctl->devdata->pins + i;
		if (pin->eint.eintnum == eint_num)
			return pin;
	}

	return NULL;
}*/

static int mtk_pmx_set_mux(struct pinctrl_dev *pctldev,
			    unsigned function,
			    unsigned group)
{
	//bool ret;
	//const struct mtk_desc_function *desc;
	//struct mtk_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);
	//struct mtk_pinctrl_group *g = pctl->groups + group;

    printk("GPIO_MultiFun_Set is called from pinctrl group = %d function = %d\n",group,function);
	atc_set_gpio_pinmux(group,function);


#if 0
	ret = mtk_pctrl_is_function_valid(pctl, g->pin, function);
	if (!ret) {
		dev_err(pctl->dev, "invaild function %d on group %d .\n",
				function, group);
		return -EINVAL;
	}

	desc = mtk_pctrl_find_function_by_pin(pctl, g->pin, function);
	if (!desc)
		return -EINVAL;
	mtk_pmx_set_mode(pctldev, g->pin, desc->muxval);
#endif
	return 0;
}


static const struct pinmux_ops autochips_pmx_ops = {
	.get_functions_count	= mtk_pmx_get_funcs_cnt,
	.get_function_name	= mtk_pmx_get_func_name,
	.get_function_groups	= mtk_pmx_get_func_groups,
	.set_mux		= mtk_pmx_set_mux,
	//.gpio_set_direction	= mtk_pmx_gpio_set_direction,
};


#define MTK_PIN_NO(x) ((x) << 8)
#define MTK_GET_PIN_NO(x) ((x) >> 8)
#define MTK_GET_PIN_FUNC(x) ((x) & 0xff)


static struct mtk_pinctrl_group *
mtk_pctrl_find_group_by_pin(struct mtk_pinctrl *pctl, u32 pin)
{
	int i;

	for (i = 0; i < pctl->ngroups; i++) {
		struct mtk_pinctrl_group *grp = pctl->groups + i;

		if (grp->pin == pin)
			return grp;
	}

	return NULL;
}

static int mtk_pctrl_dt_node_to_map_func(struct mtk_pinctrl *pctl,
		u32 pin, u32 fnum, struct mtk_pinctrl_group *grp,
		struct pinctrl_map **map, unsigned *reserved_maps,
		unsigned *num_maps)
{
	bool ret;

	if (*num_maps == *reserved_maps)
		return -ENOSPC;

	(*map)[*num_maps].type = PIN_MAP_TYPE_MUX_GROUP;
	(*map)[*num_maps].data.mux.group = grp->name;

	ret = mtk_pctrl_is_function_valid(pctl, pin, fnum);
	if (!ret) {
		printk("invalid function %d on pin %d .\n",
				fnum, pin);
		return -EINVAL;
	}

	(*map)[*num_maps].data.mux.function = mtk_gpio_functions[fnum];
	(*num_maps)++;

	return 0;
}




static int mtk_pctrl_dt_subnode_to_map(struct pinctrl_dev *pctldev,
				      struct device_node *node,
				      struct pinctrl_map **map,
				      unsigned *reserved_maps,
				      unsigned *num_maps)
{
	struct property *pins;
	u32 pinfunc, pin, func;
	int num_pins, num_funcs, maps_per_pin;
	unsigned long *configs;
	unsigned int num_configs;
	bool has_config = 0;
	int i, err;
	unsigned reserve = 0;
	struct mtk_pinctrl_group *grp;
	struct mtk_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);


	pins = of_find_property(node, "pinmux", NULL);
	if (!pins) {
		pins = of_find_property(node, "pins", NULL);
		if (!pins) {
			dev_err(pctl->dev, "missing pins property in node %s .\n",
					node->name);
			return -EINVAL;
		}
	}
	err = pinconf_generic_parse_dt_config(node, &configs, &num_configs);
	
	if (num_configs) 
		has_config = 1;

	num_pins = pins->length / sizeof(u32);
	num_funcs = num_pins;
	maps_per_pin = 0;
	if (num_funcs)
		maps_per_pin++;
	if (has_config && num_pins >= 1)
		//maps_per_pin++;
		maps_per_pin = maps_per_pin + num_configs;

	if (!num_pins || !maps_per_pin)
	{
		return -EINVAL;
	}

	reserve = num_pins * maps_per_pin;

	err = pinctrl_utils_reserve_map(pctldev, map,
			reserved_maps, num_maps, reserve);
	if (err < 0)
	{
		goto fail;
	}

	

	for (i = 0; i < num_pins; i++) {
		err = of_property_read_u32_index(node, "pinmux",
				i, &pinfunc);
		if (err) {
			err = of_property_read_u32_index(node, "pins",
				i, &pinfunc);
			if (err)
				goto fail;
			
		}

		pin = MTK_GET_PIN_NO(pinfunc);
		func = MTK_GET_PIN_FUNC(pinfunc);
		

		if (pin >= pctl->devdata->npins ||
				func >= ARRAY_SIZE(mtk_gpio_functions)) {
			printk("invalid pins value.\n");
			err = -EINVAL;
			goto fail;
		}

		grp = mtk_pctrl_find_group_by_pin(pctl, pin);
		if (!grp) {
			printk("unable to match pin %d to group\n",
					pin);
			return -EINVAL;
		}

		err = mtk_pctrl_dt_node_to_map_func(pctl, pin, func, grp, map,
				reserved_maps, num_maps);
		if (err < 0)
			goto fail;

		if (has_config) {
			err = pinctrl_utils_add_map_configs(pctldev, map,
					reserved_maps, num_maps, grp->name,
					configs, num_configs,
					PIN_MAP_TYPE_CONFIGS_GROUP);
			if (err < 0)
				goto fail;
		}
	}

	return 0;

fail:
	return err;
}




static int mtk_pctrl_dt_node_to_map(struct pinctrl_dev *pctldev,
				 struct device_node *np_config,
				 struct pinctrl_map **map, unsigned *num_maps)
{


	struct device_node *np;
	unsigned reserved_maps;
	int ret;

	*map = NULL;
	*num_maps = 0;
	reserved_maps = 0;

	for_each_child_of_node(np_config, np) {
		ret = mtk_pctrl_dt_subnode_to_map(pctldev, np, map,
				&reserved_maps, num_maps);
		if (ret < 0) {
			pinctrl_utils_dt_free_map(pctldev, *map, *num_maps);
			return ret;
		}
	}

	return 0;
}



static int mtk_pctrl_get_groups_count(struct pinctrl_dev *pctldev)
{
	struct mtk_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	return pctl->ngroups;
}

static const char *mtk_pctrl_get_group_name(struct pinctrl_dev *pctldev,
					      unsigned group)
{
	struct mtk_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	return pctl->groups[group].name;
}

static int mtk_pctrl_get_group_pins(struct pinctrl_dev *pctldev,
				      unsigned group,
				      const unsigned **pins,
				      unsigned *num_pins)
{
	struct mtk_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	*pins = (unsigned *)&pctl->groups[group].pin;
	*num_pins = 1;

	return 0;
}



static const struct pinctrl_ops mtk_pctrl_ops = {
	.dt_node_to_map		= mtk_pctrl_dt_node_to_map,
	.dt_free_map		= pinctrl_utils_dt_free_map,
	.get_groups_count	= mtk_pctrl_get_groups_count,
	.get_group_name		= mtk_pctrl_get_group_name,
	.get_group_pins		= mtk_pctrl_get_group_pins,
};



static int mtk_pconf_group_get(struct pinctrl_dev *pctldev,
				 unsigned group,
				 unsigned long *config)
{
#if 0
	struct mtk_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	*config = pctl->groups[group].config;
#endif
	return 0;
}


static int mtk_pconf_parse_conf(struct pinctrl_dev *pctldev,
		unsigned int pin, enum pin_config_param param,
		enum pin_config_param arg)
{
	int ret = 0;
	//struct mtk_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
		//ret = mtk_pconf_set_pull_select(pctl, pin, false, false, arg);
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		//ret = mtk_pconf_set_pull_select(pctl, pin, true, true, arg);
		//GPIO_Pull_UpDown(pin,PULLUP);
		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		//ret = mtk_pconf_set_pull_select(pctl, pin, true, false, arg);
	    //GPIO_Pull_UpDown(pin,PULLDOWN);
		break;
	case PIN_CONFIG_INPUT_ENABLE:
		//ret = mtk_pconf_set_ies_smt(pctl, pin, arg, param);
		break;
	case PIN_CONFIG_OUTPUT:
		//mtk_gpio_set(pctl->chip, pin, arg);
		//ret = mtk_pmx_gpio_set_direction(pctldev, NULL, pin, false);
		break;
	case PIN_CONFIG_INPUT_SCHMITT_ENABLE:
		//ret = mtk_pconf_set_ies_smt(pctl, pin, arg, param);
		break;
	case PIN_CONFIG_DRIVE_STRENGTH:
		//ret = mtk_pconf_set_driving(pctl, pin, arg);
		//GPIO_DriveCurrent_Set(pin,arg);
		break;
	case PIN_CONFIG_SLEW_RATE:
		//mtk_pconf_set_direction(pctl, pin, arg, param);
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}


static int mtk_pconf_group_set(struct pinctrl_dev *pctldev, unsigned group,
				 unsigned long *configs, unsigned num_configs)
{
#if 1
	struct mtk_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);
	struct mtk_pinctrl_group *g = &pctl->groups[group];

	int i, ret;

	for (i = 0; i < num_configs; i++) {
		ret = mtk_pconf_parse_conf(pctldev, g->pin,
			pinconf_to_config_param(configs[i]),
			pinconf_to_config_argument(configs[i]));
		if (ret < 0)
			return ret;

		g->config = configs[i];
	}
#endif
	return 0;
}


static const struct pinconf_ops mtk_pconf_ops = {
	.pin_config_group_get	= mtk_pconf_group_get,
	.pin_config_group_set	= mtk_pconf_group_set,
};


static struct pinctrl_desc mtk_pctrl_desc = {
	.confops	= &mtk_pconf_ops,
	.pctlops	= &mtk_pctrl_ops,
	.pmxops		= &autochips_pmx_ops,
};

#if 0
static int ac83xx_gpio_request(struct gpio_chip *chip, unsigned offset)
{
	return pinctrl_request_gpio(chip->base + offset);
}

static void ac83xx_gpio_free(struct gpio_chip *chip, unsigned offset)
{
	pinctrl_free_gpio(chip->base + offset);
}
#endif


#if 0
static struct gpio_chip mtk_gpio_chip = {
	.owner			= THIS_MODULE,
	.request		= ac83xx_gpio_request,
	.free			= ac83xx_gpio_free,
	.direction_input	= ac83xx_gpio_direction_input,
	.direction_output	= ac83xx_gpio_direction_output,
	.get			= ac83xx_gpio_get_value,
	.set			= ac83xx_gpio_set_value,
	.to_irq			= ac83xx_gpio_to_irq,
	//.set_debounce		= mtk_gpio_set_debounce,
	.of_gpio_n_cells	= 1,
};
#endif


//#########step 3#################

static int mtk_pctrl_build_state(struct platform_device *pdev)
{
	struct mtk_pinctrl *pctl = platform_get_drvdata(pdev);
	int i;

	pctl->ngroups = pctl->devdata->npins;

	/* Allocate groups */
	pctl->groups = devm_kcalloc(&pdev->dev, pctl->ngroups,
				    sizeof(*pctl->groups), GFP_KERNEL);
	if (!pctl->groups)
		return -ENOMEM;

	/* We assume that one pin is one group, use pin name as group name. */
	pctl->grp_names = devm_kcalloc(&pdev->dev, pctl->ngroups,
				       sizeof(*pctl->grp_names), GFP_KERNEL);
	if (!pctl->grp_names)
		return -ENOMEM;

	for (i = 0; i < pctl->devdata->npins; i++) {
		const struct mtk_desc_pin *pin = pctl->devdata->pins + i;
		struct mtk_pinctrl_group *group = pctl->groups + i;

		group->name = pin->pin.name;
		group->pin = pin->pin.number;

		pctl->grp_names[i] = pin->pin.name;
	}

	return 0;
}



//##################Step2#######################


int mtk_pctrl_init(struct platform_device *pdev,
		const struct mtk_pinctrl_devdata *data,
		struct regmap *regmap)
{
	struct pinctrl_pin_desc *pins;
	struct mtk_pinctrl *pctl;
	struct device_node *np = pdev->dev.of_node;
	struct property *prop;
	int i, ret;

	pctl = devm_kzalloc(&pdev->dev, sizeof(*pctl), GFP_KERNEL);
	if (!pctl)
		return -ENOMEM;

	printk("#######atc 823x pinctrl init \n");

	platform_set_drvdata(pdev, pctl);

	prop = of_find_property(np, "pins-are-numbered", NULL);
	if (!prop) {
		dev_err(&pdev->dev, "only support pins-are-numbered format\n");
		return -EINVAL;
	}

#if 0
	node = of_parse_phandle(np, "atc,pctl-regmap", 0);
	if (node) {
		pctl->regmap1 = syscon_node_to_regmap(node);
		if (IS_ERR(pctl->regmap1))
			return PTR_ERR(pctl->regmap1);
	} else if (regmap) {
		pctl->regmap1  = regmap;
	} else {
		dev_err(&pdev->dev, "Pinctrl node has not register regmap.\n");
		return -EINVAL;
	}

	/* Only 8135 has two base addr, other SoCs have only one. */
	node = of_parse_phandle(np, "atc,pctl-regmap", 1);

	if (node) {
		pctl->regmap2 = syscon_node_to_regmap(node);
		if (IS_ERR(pctl->regmap2))
			return PTR_ERR(pctl->regmap2);
	}
#endif
	pctl->devdata = data;
	ret = mtk_pctrl_build_state(pdev);

	if (ret) {
		dev_err(&pdev->dev, "build state failed: %d\n", ret);
		return -EINVAL;
	}

	pins = devm_kzalloc(&pdev->dev,
			    pctl->devdata->npins * sizeof(*pins),
			    GFP_KERNEL);
	if (!pins)
		return -ENOMEM;

	for (i = 0; i < pctl->devdata->npins; i++)
		pins[i] = pctl->devdata->pins[i].pin;
	mtk_pctrl_desc.name = dev_name(&pdev->dev);
	mtk_pctrl_desc.owner = THIS_MODULE;
	mtk_pctrl_desc.pins = pins;
	mtk_pctrl_desc.npins = pctl->devdata->npins;
	pctl->dev = &pdev->dev;
	printk("[###pinctl]111\n");
	pctl->pctl_dev = pinctrl_register(&mtk_pctrl_desc, &pdev->dev, pctl);
	printk("[###pinctl]222\n");

	if (!pctl->pctl_dev) {
		dev_err(&pdev->dev, "couldn't register pinctrl driver\n");
		return -EINVAL;
	}

#if 0
	pctl->chip = devm_kzalloc(&pdev->dev, sizeof(*pctl->chip), GFP_KERNEL);

	if (!pctl->chip) {
		ret = -ENOMEM;
		goto pctrl_error;
	}

	*pctl->chip = mtk_gpio_chip;
	pctl->chip->ngpio = pctl->devdata->npins;
	pctl->chip->label = dev_name(&pdev->dev);
	pctl->chip->dev = &pdev->dev;
	pctl->chip->base = -1;

	ret = gpiochip_add(pctl->chip);

	if (ret) {
		ret = -EINVAL;
		goto pctrl_error;
	}

	/* Register the GPIO to pin mappings. */
	ret = gpiochip_add_pin_range(pctl->chip, dev_name(&pdev->dev),
			0, 0, pctl->devdata->npins);
	if (ret) {
		ret = -EINVAL;
		goto chip_error;
	}
#endif	
	printk("atc pinctrl init end\n");

#if 0   //暂时不做外部中断功能
	if (!of_property_read_bool(np, "interrupt-controller"))
		return 0;

	/* Get EINT register base from dts. */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "Unable to get Pinctrl resource\n");
		ret = -EINVAL;
		goto chip_error;
	}

	pctl->eint_reg_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(pctl->eint_reg_base)) {
		ret = -EINVAL;
		goto chip_error;
	}

	pctl->eint_dual_edges = devm_kcalloc(&pdev->dev, pctl->devdata->ap_num,
						 sizeof(int), GFP_KERNEL);
	if (!pctl->eint_dual_edges) {
		ret = -ENOMEM;
		goto chip_error;
	}

	irq = irq_of_parse_and_map(np, 0);
	if (!irq) {
		dev_err(&pdev->dev, "couldn't parse and map irq\n");
		ret = -EINVAL;
		goto chip_error;
	}

	pctl->domain = irq_domain_add_linear(np,
		pctl->devdata->ap_num, &irq_domain_simple_ops, NULL);
	if (!pctl->domain) {
		dev_err(&pdev->dev, "Couldn't register IRQ domain\n");
		ret = -ENOMEM;
		goto chip_error;
	}

	mtk_eint_init(pctl);
	for (i = 0; i < pctl->devdata->ap_num; i++) {
		int virq = irq_create_mapping(pctl->domain, i);

		irq_set_chip_and_handler(virq, &mtk_pinctrl_irq_chip,
			handle_level_irq);
		irq_set_chip_data(virq, pctl);
		set_irq_flags(virq, IRQF_VALID);
	};

	irq_set_chained_handler(irq, mtk_eint_irq_handler);
	irq_set_handler_data(irq, pctl);
	set_irq_flags(irq, IRQF_VALID);
#endif
	return 0;

#if 0
chip_error:
	gpiochip_remove(pctl->chip);
pctrl_error:
	pinctrl_unregister(pctl->pctl_dev);
	return ret;
#endif
}




//###################Step1################

static int ac83xx_pinctrl_probe(struct platform_device *pdev)
{
	return mtk_pctrl_init(pdev, &ac83xx_pinctrl_data, NULL);
}

static const struct of_device_id ac83xx_pctrl_match[] = {
	{ .compatible = "autochips,ac823x-pinctrl", },
	{ }
};
MODULE_DEVICE_TABLE(of, ac83xx_pctrl_match);

static struct platform_driver mtk_pinctrl_driver = {
	.probe = ac83xx_pinctrl_probe,
	.driver = {
		.name = "autochips-ac83xx-pinctrl",
		.owner = THIS_MODULE,
		.of_match_table = ac83xx_pctrl_match,
	},
};

static int __init mtk_pinctrl_init(void)
{
	return platform_driver_register(&mtk_pinctrl_driver);
}

subsys_initcall(mtk_pinctrl_init);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("atc Pinctrl Driver");
MODULE_AUTHOR("atc");
