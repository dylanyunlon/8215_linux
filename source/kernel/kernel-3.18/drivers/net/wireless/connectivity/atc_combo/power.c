/*
* Copyright (c) 2023 AutoChips Inc.
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
#include "wi_begin.h"
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/of_gpio.h>
#include "wi_end.h"

#include "common.h"

#ifdef WITHOUT_ATC_COMBO_DTS
#ifdef CONFIG_ATC_WIFI_CHIP_CYPRESS_PCIE
#define WLAN_DEFAULT_STATE      0
#else
#define WLAN_DEFAULT_STATE      0
#endif
#endif
#define DEFAULT_POWER_DELAY_US  1000
#define DEFAULT_ENABLE_DELAY_US 10000

static bool is_power_init = false;

static u32 wcn_d3v3_ctrl_gpio = -1U;
static u32 wcn_d3v3_on_delay_us = DEFAULT_POWER_DELAY_US;
static u32 wcn_d3v3_off_delay_us = DEFAULT_POWER_DELAY_US;

static u32 bt_enable_gpio = -1U;
static u32 bt_enable_delay_us = DEFAULT_ENABLE_DELAY_US;

static u32 wlan_enable_gpio = -1U;
static u32 wlan_enable_delay_us = DEFAULT_ENABLE_DELAY_US;

static struct mutex power_lock;
static int bt_power = 0;
static int wlan_power = 0;

enum {
	BT_POWER,
	WLAN_POWER,
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
static inline void fsleep(unsigned long usecs)
{
	if (usecs <= 10) {
		udelay(usecs);
	} else if (usecs <= 20000) {
		usleep_range(usecs, 2 * usecs);
	} else {
		msleep(DIV_ROUND_UP(usecs, 1000));
	}
}
#endif

static int atc_combo_power(int type, int on)
{
	if (!gpio_is_valid((int)wcn_d3v3_ctrl_gpio)) {
		return 0;
	}
	mutex_lock(&power_lock);
	if (type == BT_POWER) {
		bt_power = on;
	} else if (type == WLAN_POWER) {
		wlan_power = on;
	} else {
		COMBO_ERR("invalid type %d\n", type);
		mutex_unlock(&power_lock);
		return -1;
	}
	if (on == gpio_get_value(wcn_d3v3_ctrl_gpio)) {
		mutex_unlock(&power_lock);
		return 0;
	}
	if (on) {
		COMBO_INFO("%s %d\n", __func__, on);
		gpio_set_value(wcn_d3v3_ctrl_gpio, 1);
		fsleep(wcn_d3v3_on_delay_us);
	} else if (!bt_power && !wlan_power) {
		COMBO_INFO("%s %d\n", __func__, on);
		fsleep(wcn_d3v3_off_delay_us);
		gpio_set_value(wcn_d3v3_ctrl_gpio, 0);
		fsleep(wcn_d3v3_off_delay_us);
	}
	mutex_unlock(&power_lock);

	return 0;
}

int atc_combo_bt_enable(int on)
{
	if (!gpio_is_valid((int)bt_enable_gpio)) {
		COMBO_ERR("invalid bt_enable_gpio %u\n", bt_enable_gpio);
		return -1;
	}
	if (on == gpio_get_value(bt_enable_gpio)) {
		COMBO_INFO("bt_enable %d\n", on);
		return 0;
	}
	if (on) {
		atc_combo_power(BT_POWER, 1);
	}
	COMBO_INFO("bt_enable %d\n", on);
	gpio_set_value(bt_enable_gpio, on);
	fsleep(bt_enable_delay_us);
	if (!on) {
		atc_combo_power(BT_POWER, 0);
	}

	return 0;
}
EXPORT_SYMBOL(atc_combo_bt_enable);

bool atc_combo_bt_is_enabled(void)
{
	if (!gpio_is_valid((int)bt_enable_gpio)) {
		COMBO_ERR("invalid bt_enable_gpio %u\n", bt_enable_gpio);
		return false;
	}

	return !!gpio_get_value(bt_enable_gpio);
}
EXPORT_SYMBOL(atc_combo_bt_is_enabled);

int atc_combo_wlan_enable(int on)
{
	if (!gpio_is_valid((int)wlan_enable_gpio)) {
		COMBO_ERR("invalid wlan_enable_gpio %u\n", wlan_enable_gpio);
		return -1;
	}
	if (on == gpio_get_value(wlan_enable_gpio)) {
		COMBO_INFO("wlan_enable %d\n", on);
		return 0;
	}
	if (on) {
		atc_combo_power(WLAN_POWER, 1);
	}
	COMBO_INFO("wlan_enable %d\n", on);
	gpio_set_value(wlan_enable_gpio, on);
	fsleep(wlan_enable_delay_us);
	if (!on) {
		atc_combo_power(WLAN_POWER, 0);
	}

	return 0;
}
EXPORT_SYMBOL(atc_combo_wlan_enable);

bool atc_combo_wlan_is_enabled(void)
{
	if (!gpio_is_valid((int)wlan_enable_gpio)) {
		COMBO_ERR("invalid wlan_enable_gpio %u\n", wlan_enable_gpio);
		return false;
	}

	return !!gpio_get_value(wlan_enable_gpio);
}
EXPORT_SYMBOL(atc_combo_wlan_is_enabled);

int atc_combo_chip_reset(int type)
{
	int ret = 0;
	int bt_enabled = bt_power;
	int wlan_enabled = wlan_power;

	if (type < COMBO_RESET_TYPE_OFF || type >= COMBO_RESET_TYPE_MAX) {
		COMBO_ERR("invalid type %d\n", type);
		return -1;
	}
	COMBO_INFO("chip_reset(%d) start\n", type);
	ret = atc_combo_bt_enable(0);
	if (ret) {
		COMBO_ERR("atc_combo_bt_enable(0) failed %d\n", ret);
		return ret;
	}
	ret = atc_combo_wlan_enable(0);
	if (ret) {
		COMBO_ERR("atc_combo_wlan_enable(0) failed %d\n", ret);
		return ret;
	}
	if (type == COMBO_RESET_TYPE_OFF) {
		COMBO_INFO("chip_reset(%d) end\n", type);
		return 0;
	}
	if (bt_enabled) {
		ret = atc_combo_bt_enable(1);
		if (ret) {
			COMBO_ERR("atc_combo_bt_enable(1) failed %d\n", ret);
			return ret;
		}
	}
	if (wlan_enabled) {
		ret = atc_combo_wlan_enable(1);
		if (ret) {
			COMBO_ERR("atc_combo_wlan_enable(0) failed %d\n", ret);
			return ret;
		}
	}
	if (type == COMBO_RESET_TYPE_RECOVER) {
		COMBO_INFO("chip_reset(%d) TODO: recover\n", type);
	}
	COMBO_INFO("chip_reset(%d) end\n", type);

	return 0;
}
EXPORT_SYMBOL(atc_combo_chip_reset);

int atc_combo_power_init(struct platform_device *pdev)
{
#if defined(WITHOUT_ATC_COMBO_DTS) && !defined(CONFIG_ATC_WIFI_CHIP_MT6630_SDIO)
	struct device_node *np = NULL;
	int ret = -1;
#else
	struct pinctrl *pinctrl = NULL;
	struct pinctrl_state *atc_combo_gpios = NULL;
	int ret = -1;
#endif

	COMBO_INFO("\n");
#ifdef CONFIG_ATC_WIFI_CHIP_SELF_ADAPTIVE
	if (atc_combo_get_chip_type() == ATC_WIFI_CHIP_TYPE_MT6630) {
		return 0;
	}
#endif
	mutex_init(&power_lock);
	is_power_init = true;

#if defined(WITHOUT_ATC_COMBO_DTS) && !defined(CONFIG_ATC_WIFI_CHIP_MT6630_SDIO)
	np = of_find_compatible_node(NULL, NULL, "Autochips,ac8x-ExtWiFi");
	if (!np) {
		COMBO_INFO("no wlan_enable_gpio\n");
    } else {
		wlan_enable_gpio = (u32)of_get_named_gpio(np, "extwifi-gpios", 0);
		if (!gpio_is_valid((int)wlan_enable_gpio)) {
			COMBO_ERR("no wlan_enable_gpio\n");
			return -ENODEV;
		}
		wlan_enable_delay_us = DEFAULT_ENABLE_DELAY_US;
		COMBO_INFO("wlan_enable gpio %u delay %u us\n",
				wlan_enable_gpio, wlan_enable_delay_us);

		ret = devm_gpio_request_one(&pdev->dev, wlan_enable_gpio,
				WLAN_DEFAULT_STATE ? GPIOF_INIT_HIGH : GPIOF_INIT_LOW, "wlan_enable");
		if (ret) {
			COMBO_ERR("gpio_request(%u) failed %d\n", wlan_enable_gpio, ret);
			return ret;
		}
		COMBO_INFO("wlan_enable %d\n", WLAN_DEFAULT_STATE);
	}
#else
	pinctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(pinctrl)) {
		COMBO_INFO("no pinctrl\n");
	} else {
		atc_combo_gpios = pinctrl_lookup_state(pinctrl, "atc_combo_gpios");
		if (IS_ERR(atc_combo_gpios)) {
			COMBO_INFO("no pinctrl\n");
		} else {
			ret = pinctrl_select_state(pinctrl, atc_combo_gpios);
			if (ret) {
				COMBO_ERR("select atc_combo_gpios failed %d\n", ret);
				return ret;
			}
		}
	}

	wcn_d3v3_ctrl_gpio = (u32)of_get_named_gpio(pdev->dev.of_node, "wcn_d3v3_ctrl-gpios", 0);
	if (!gpio_is_valid((int)wcn_d3v3_ctrl_gpio)) {
		COMBO_INFO("no wcn_d3v3_ctrl_gpio\n");
	} else {
		ret = of_property_read_u32(pdev->dev.of_node,
				"wcn_d3v3_on_delay_us", &wcn_d3v3_on_delay_us);
		if (ret) {
			wcn_d3v3_on_delay_us = DEFAULT_POWER_DELAY_US;
		}
		ret = of_property_read_u32(pdev->dev.of_node,
				"wcn_d3v3_off_delay_us", &wcn_d3v3_off_delay_us);
		if (ret) {
			wcn_d3v3_off_delay_us = DEFAULT_POWER_DELAY_US;
		}
		COMBO_INFO("wcn_d3v3_ctrl gpio %u on delay %u us off delay %u us\n",
				wcn_d3v3_ctrl_gpio, wcn_d3v3_on_delay_us, wcn_d3v3_off_delay_us);
	}

	bt_enable_gpio = (u32)of_get_named_gpio(pdev->dev.of_node, "bt_enable-gpios", 0);
	if (!gpio_is_valid((int)bt_enable_gpio)) {
		COMBO_INFO("no bt_enable_gpio\n");
	} else {
		ret = of_property_read_u32(pdev->dev.of_node,
				"bt_enable_delay_us", &bt_enable_delay_us);
		if (ret) {
			bt_enable_delay_us = DEFAULT_ENABLE_DELAY_US;
		}
		COMBO_INFO("bt_enable gpio %u delay %u us\n",
				bt_enable_gpio, bt_enable_delay_us);

		ret = devm_gpio_request_one(&pdev->dev, bt_enable_gpio,
				GPIOF_INIT_LOW, "bt_enable");
		if (ret) {
			COMBO_ERR("gpio_request(%u) failed %d\n", bt_enable_gpio, ret);
			return ret;
		}
		COMBO_INFO("bt_enable 0\n");
	}

	wlan_enable_gpio = (u32)of_get_named_gpio(pdev->dev.of_node, "wlan_enable-gpios", 0);
	if (!gpio_is_valid((int)wlan_enable_gpio)) {
		COMBO_INFO("no wlan_enable_gpio\n");
	} else {
		ret = of_property_read_u32(pdev->dev.of_node,
				"wlan_enable_delay_us", &wlan_enable_delay_us);
		if (ret) {
			wlan_enable_delay_us = DEFAULT_ENABLE_DELAY_US;
		}
		COMBO_INFO("wlan_enable gpio %u delay %u us\n",
				wlan_enable_gpio, wlan_enable_delay_us);

		ret = devm_gpio_request_one(&pdev->dev, wlan_enable_gpio,
				GPIOF_INIT_LOW, "wlan_enable");
		if (ret) {
			COMBO_ERR("gpio_request(%u) failed %d\n", wlan_enable_gpio, ret);
			return ret;
		}
		COMBO_INFO("wlan_enable 0\n");
	}

	fsleep(wcn_d3v3_off_delay_us);

	if (gpio_is_valid((int)wcn_d3v3_ctrl_gpio)) {
		ret = devm_gpio_request_one(&pdev->dev, wcn_d3v3_ctrl_gpio,
				GPIOF_INIT_LOW, "wcn_d3v3_ctrl");
		COMBO_INFO("wcn_d3v3_ctrl 0\n");
		fsleep(wcn_d3v3_off_delay_us);
	}
#endif

	return 0;
}

int atc_combo_power_deinit(struct platform_device *pdev __maybe_unused)
{
	COMBO_INFO("\n");
	if (!is_power_init) {
		return;
	}
	mutex_destroy(&power_lock);
	is_power_init = false;

	return 0;
}

int atc_combo_power_suspend(struct device *dev __maybe_unused)
{
	if (gpio_is_valid((int)wlan_enable_gpio) && gpio_get_value(wlan_enable_gpio)) {
		COMBO_INFO("power off wlan by suspend\n");
		atc_combo_wlan_enable(0);
	}
	if (gpio_is_valid((int)bt_enable_gpio) && gpio_get_value(bt_enable_gpio)) {
		COMBO_INFO("power off bt by suspend\n");
		atc_combo_bt_enable(0);
	}

	return 0;
}

int atc_combo_power_resume(struct device *dev __maybe_unused)
{
	return 0;
}

MODULE_DESCRIPTION("atc combo power");
MODULE_ALIAS("atc_combo:power");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Rocky Pan");
