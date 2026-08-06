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
#ifdef CONFIG_ATC_WIFI_CHIP_SELF_ADAPTIVE

#include "wi_begin.h"
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include "wi_end.h"

#include "common.h"

#define SDIO_VENDOR_ID_MT6630           0x037a
#define SDIO_VENDOR_ID_CYW4373          0x02d0
#define SDIO_VENDOR_ID_AIC8800D80       0xc8a1

#define SDIO_DEVICE_ID_MT6630           0x6630
#define SDIO_DEVICE_ID_CYW4373          0x4373
#define SDIO_DEVICE_ID_AIC8800D80       0x0082
#define SDIO_DEVICE_ID_AIC8800D80_F2    0x0182

#define WLAN_SLOT   1

extern int msdc_detect_change(u32 slot, u32 enable, u32 type);

static bool sdio_probe_done = false;

static const struct sdio_device_id atc_combo_sdio_detect_ids[] = {
	{ SDIO_DEVICE(SDIO_ANY_ID, SDIO_ANY_ID) },
	{}
};

static int atc_combo_sdio_detect_probe(struct sdio_func *func,
		const struct sdio_device_id *id)
{
	COMBO_INFO("sdio func %u class 0x%02x vendor 0x%04x device 0x%04x\n",
			func->num, func->class, func->vendor, func->device);
	if (func->num != 1) {
		return 0;
	}
	if (func->vendor == SDIO_VENDOR_ID_MT6630 && func->device == SDIO_DEVICE_ID_MT6630) {
		atc_combo_set_chip_type(ATC_WIFI_CHIP_TYPE_MT6630);
	} else if (func->vendor == SDIO_VENDOR_ID_CYW4373 && func->device == SDIO_DEVICE_ID_CYW4373) {
		atc_combo_set_chip_type(ATC_WIFI_CHIP_TYPE_CYPRESS_SDIO);
	} else if (func->vendor == SDIO_VENDOR_ID_AIC8800D80 && func->device == SDIO_DEVICE_ID_AIC8800D80) {
		atc_combo_set_chip_type(ATC_WIFI_CHIP_TYPE_AIC8800_SDIO);
	} else {
		atc_combo_set_chip_type(ATC_WIFI_CHIP_TYPE_UNKNOWN);
		COMBO_ERR("unknown sdio device\n");
	}
	sdio_probe_done = true;

	return 0;
}

static void atc_combo_sdio_detect_remove(struct sdio_func *func)
{
}

static struct sdio_driver atc_combo_sdio_detect_driver = {
	.name = "atc_combo_sdio_detect",
	.id_table = atc_combo_sdio_detect_ids,
	.probe = atc_combo_sdio_detect_probe,
	.remove = atc_combo_sdio_detect_remove,
};

static int atc_wlan_detect_change(int on)
{
    int ret = -1;

    ret = msdc_detect_change(WLAN_SLOT, on, 0);
    if (ret) {
        COMBO_ERR("%s(%d) failed %d\n", __func__, on, ret);
        return ret;
    }

    return 0;
}

static int detect_aic8800_cypress(struct platform_device *pdev)
{
	u32 wlan_enable_gpio = -1U;
	unsigned long timeout = 0;
	int ret = -1;

	COMBO_INFO("start\n");

	wlan_enable_gpio = (u32)of_get_named_gpio(pdev->dev.of_node, "wlan_enable-gpios", 0);
    if (!gpio_is_valid((int)wlan_enable_gpio)) {
        COMBO_INFO("no wlan_enable-gpios\n");
		ret = -EINVAL;
		goto out;
	}
	ret = gpio_request_one(wlan_enable_gpio, GPIOF_INIT_LOW, "wlan_enable");
	if (ret) {
		COMBO_INFO("gpio_request(%u) failed %d\n", wlan_enable_gpio, ret);
		ret = -EBUSY;
		goto out;
	}
	msleep(20);
	gpio_set_value(wlan_enable_gpio, 1);
	msleep(20);
	sdio_probe_done = false;
	ret = atc_wlan_detect_change(1);
	if (ret) {
		COMBO_INFO("sdio detect failed %d\n", ret);
		goto out_free;
	}
	timeout = jiffies + msecs_to_jiffies(200);
	while (time_is_after_jiffies(timeout)) {
		if (sdio_probe_done) {
			break;
		}
		msleep(10);
	}
	if (atc_combo_get_chip_type() == ATC_WIFI_CHIP_TYPE_UNKNOWN) {
		ret = -ENODEV;
	}

out_free:
	gpio_set_value(wlan_enable_gpio, 0);
	gpio_free(wlan_enable_gpio);
	msleep(20);
	atc_wlan_detect_change(0);
out:
	COMBO_INFO("detect chip type %d ret %d\n", atc_combo_get_chip_type(), ret);

	return ret;
}

static int detect_mt6630(struct platform_device *pdev)
{
	u32 mt6630_pwn_gpio = -1U;
	u32 mt6630_rst_gpio = -1U;
	unsigned long timeout = 0;
	int ret = -1;

	COMBO_INFO("start\n");

	mt6630_rst_gpio = (u32)of_get_named_gpio(pdev->dev.of_node, "mt6630_rst-gpios", 0);
    if (!gpio_is_valid((int)mt6630_rst_gpio)) {
        COMBO_INFO("no mt6630_rst-gpios\n");
		ret = -EINVAL;
		goto out;
	}
	ret = gpio_request_one(mt6630_rst_gpio, GPIOF_INIT_LOW, "mt6630_rst");
	if (ret) {
		COMBO_INFO("gpio_request(%u) failed %d\n", mt6630_rst_gpio, ret);
		ret = -EBUSY;
		goto out;
	}
	mt6630_pwn_gpio = (u32)of_get_named_gpio(pdev->dev.of_node, "mt6630_pwn-gpios", 0);
    if (!gpio_is_valid((int)mt6630_pwn_gpio)) {
        COMBO_INFO("no mt6630_pwn-gpios\n");
		gpio_free(mt6630_rst_gpio);
		ret = -EINVAL;
		goto out;
	}
	ret = gpio_request_one(mt6630_pwn_gpio, GPIOF_INIT_LOW, "mt6630_pwn");
	if (ret) {
		COMBO_INFO("gpio_request(%u) failed %d\n", mt6630_rst_gpio, ret);
		gpio_free(mt6630_rst_gpio);
		ret = -EBUSY;
		goto out;
	}
	msleep(20);
	gpio_set_value(mt6630_pwn_gpio, 1);
	msleep(30);
	gpio_set_value(mt6630_rst_gpio, 1);
	msleep(100);
	sdio_probe_done = false;
	ret = atc_wlan_detect_change(1);
	if (ret) {
		COMBO_INFO("sdio detect failed %d\n", ret);
		goto out_free;
	}
	timeout = jiffies + msecs_to_jiffies(200);
	while (time_is_after_jiffies(timeout)) {
		if (sdio_probe_done) {
			break;
		}
		msleep(10);
	}
	if (atc_combo_get_chip_type() == ATC_WIFI_CHIP_TYPE_UNKNOWN) {
		ret = -ENODEV;
	}

out_free:
	gpio_set_value(mt6630_rst_gpio, 0);
	gpio_free(mt6630_rst_gpio);
	gpio_set_value(mt6630_pwn_gpio, 0);
	gpio_free(mt6630_pwn_gpio);
	msleep(20);
	atc_wlan_detect_change(0);
out:
	COMBO_INFO("detect chip type %d ret %d\n", atc_combo_get_chip_type(), ret);

	return ret;
}

int atc_combo_chip_type_detect(struct platform_device *pdev)
{
	int ret = -1;

	COMBO_INFO("start\n");

	ret = sdio_register_driver(&atc_combo_sdio_detect_driver);
	if (ret) {
		COMBO_ERR("sdio_register_driver failed %d\n", ret);
		return ret;
	}
	ret = detect_aic8800_cypress(pdev);
	if (ret) {
		ret = detect_mt6630(pdev);
	}
	sdio_unregister_driver(&atc_combo_sdio_detect_driver);

	if (ret) {
		COMBO_INFO("detect chip type failed %d\n", ret);
	} else {
		COMBO_INFO("detect chip type: %s\n", atc_combo_get_chip_type_str());
	}

	return ret;
}

MODULE_DESCRIPTION("atc combo detect");
MODULE_ALIAS("atc_combo:detect");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Rocky Pan");

#endif // CONFIG_ATC_WIFI_CHIP_SELF_ADAPTIVE
