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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/slab.h>
#include "wi_end.h"

#include "common.h"

#define DRV_NAME	"atc_combo"

struct atc_combo_data {
	int test_val;
};

int atc_combo_int; // debug trigger
EXPORT_SYMBOL(atc_combo_int);

static ssize_t test_show(struct device *dev,
		struct device_attribute *attr __maybe_unused, char *buf)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct atc_combo_data *pdata = platform_get_drvdata(pdev);

	return sprintf(buf, "%d\n", pdata->test_val);
}

static ssize_t test_store(struct device *dev,
		struct device_attribute *attr __maybe_unused, const char *buf, size_t count)
{
	char *end = NULL;
	struct platform_device *pdev = to_platform_device(dev);
	struct atc_combo_data *pdata = platform_get_drvdata(pdev);
	int val = (int)simple_strtol(buf, &end, 0);

	if (end == buf) { // input non-number string
		COMBO_ERR("inalid data: %s\n", buf);
		return -EINVAL;
	}
	pdata->test_val = val;

	return (ssize_t)count;
}

static DEVICE_ATTR(test, S_IWUSR | S_IRUGO, test_show, test_store);

static ssize_t int_show(struct device *dev __maybe_unused,
		struct device_attribute *attr __maybe_unused, char *buf)
{
	return sprintf(buf, "%d\n", atc_combo_int);
}

static ssize_t int_store(struct device *dev __maybe_unused,
		struct device_attribute *attr __maybe_unused, const char *buf, size_t count)
{
	char *end = NULL;
	int val = (int)simple_strtol(buf, &end, 0);

	if (end == buf) { // input non-number string
		COMBO_ERR("inalid data: %s\n", buf);
		return -EINVAL;
	}
	atc_combo_int = val;

	return (ssize_t)count;
}

static DEVICE_ATTR(int, S_IWUSR | S_IRUGO, int_show, int_store);

static ssize_t bt_enable_show(struct device *dev __maybe_unused,
		struct device_attribute *attr __maybe_unused, char *buf)
{
	return sprintf(buf, "%d\n", atc_combo_bt_is_enabled());
}

static ssize_t bt_enable_store(struct device *dev __maybe_unused,
		struct device_attribute *attr __maybe_unused, const char *buf, size_t count)
{
	char *end = NULL;
	int val = (int)simple_strtol(buf, &end, 0);
	int ret = -1;

	if (end == buf) { // input non-number string
		COMBO_ERR("inalid data: %s\n", buf);
		return -EINVAL;
	}
	if (val != 0 && val != 1) {
		COMBO_ERR("inalid data: %s\n", buf);
		return -EINVAL;
	}
	ret = atc_combo_bt_enable(val);

	return ret < 0 ? ret : (ssize_t)count;
}

static DEVICE_ATTR(bt_enable, S_IWUSR | S_IRUGO,
		bt_enable_show, bt_enable_store);

static ssize_t wlan_enable_show(struct device *dev __maybe_unused,
		struct device_attribute *attr __maybe_unused, char *buf)
{
	return sprintf(buf, "%d\n", atc_combo_wlan_is_enabled());
}

static ssize_t wlan_enable_store(struct device *dev __maybe_unused,
		struct device_attribute *attr __maybe_unused, const char *buf, size_t count)
{
	char *end = NULL;
	int val = (int)simple_strtol(buf, &end, 0);
	int ret = -1;

	if (end == buf) { // input non-number string
		COMBO_ERR("inalid data: %s\n", buf);
		return -EINVAL;
	}
	if (val != 0 && val != 1) {
		COMBO_ERR("inalid data: %s\n", buf);
		return -EINVAL;
	}
	ret = atc_combo_wlan_enable(val);

	return ret < 0 ? ret : (ssize_t)count;
}

static DEVICE_ATTR(wlan_enable, S_IWUSR | S_IRUGO,
		wlan_enable_show, wlan_enable_store);

static ssize_t chip_reset_show(struct device *dev __maybe_unused,
		struct device_attribute *attr __maybe_unused, char *buf)
{
	return sprintf(buf, "write \"<type>\": chip_reset(type)\n");
}

static ssize_t chip_reset_store(struct device *dev __maybe_unused,
		struct device_attribute *attr __maybe_unused, const char *buf, size_t count)
{
	char *end = NULL;
	int val = (int)simple_strtol(buf, &end, 0);
	int ret = -1;

	if (end == buf) { // input non-number string
		COMBO_ERR("inalid data: %s\n", buf);
		return -EINVAL;
	}
	if (val < COMBO_RESET_TYPE_OFF
			|| val >= COMBO_RESET_TYPE_MAX) {
		COMBO_ERR("inalid data: %s\n", buf);
		return -EINVAL;
	}
	ret = atc_combo_chip_reset(val);

	return ret < 0 ? ret : (ssize_t)count;
}

static DEVICE_ATTR(chip_reset, S_IWUSR | S_IRUGO,
		chip_reset_show, chip_reset_store);

extern int atc_combo_wlan_reset_trigger(int type);

static ssize_t wlan_reset_show(struct device *dev __maybe_unused,
		struct device_attribute *attr __maybe_unused, char *buf)
{
	return sprintf(buf, "write(type): atc_combo_wlan_reset_trigger(type)\n");
}

static ssize_t wlan_reset_store(struct device *dev __maybe_unused,
		struct device_attribute *attr __maybe_unused, const char *buf, size_t count)
{
	char *end = NULL;
	int val = (int)simple_strtol(buf, &end, 0);
	int ret = -1;

	if (end == buf) { // input non-number string
		COMBO_ERR("inalid data: %s\n", buf);
		return -EINVAL;
	}
	ret = atc_combo_wlan_reset_trigger(val);

	return ret < 0 ? ret : (ssize_t)count;
}

static DEVICE_ATTR(wlan_reset, S_IWUSR | S_IRUGO,
		wlan_reset_show, wlan_reset_store);

extern int atc_combo_get_wlan_log_level(void);
extern int atc_combo_set_wlan_log_level(int level);

static ssize_t wlan_log_level_show(struct device *dev __maybe_unused,
		struct device_attribute *attr __maybe_unused, char *buf)
{
	int level = atc_combo_get_wlan_log_level();

	if (ATC_COMBO_WLAN_LOG_LEVEL_INFO == level) {
		return sprintf(buf, "info\n");

	} else if (ATC_COMBO_WLAN_LOG_LEVEL_DEBUG == level) {
		return sprintf(buf, "debug\n");

	} else if (ATC_COMBO_WLAN_LOG_LEVEL_LOUD == level) {
		return sprintf(buf, "loud\n");

	} else {
		COMBO_ERR("unknown log level %d\n", level);
		return sprintf(buf, "unknown\n");
	}
}

static ssize_t wlan_log_level_store(struct device *dev __maybe_unused,
		struct device_attribute *attr __maybe_unused, const char *buf, size_t count)
{
	int ret = -1;
	int level = 0;

	if (!strncmp(buf, "info", strlen("info"))) {
		level = ATC_COMBO_WLAN_LOG_LEVEL_INFO;

	} else if (!strncmp(buf, "debug", strlen("debug"))) {
		level = ATC_COMBO_WLAN_LOG_LEVEL_DEBUG;

	} else if (!strncmp(buf, "loud", strlen("loud"))) {
		level = ATC_COMBO_WLAN_LOG_LEVEL_LOUD;

	} else {
		COMBO_ERR("invalid level %s, available: info debug loud\n", buf);
		return -EINVAL;
	}
	ret = atc_combo_set_wlan_log_level(level);

	return ret < 0 ? ret : (ssize_t)count;
}

static DEVICE_ATTR(wlan_log_level, S_IWUSR | S_IRUGO,
		wlan_log_level_show, wlan_log_level_store);

extern int atc_combo_tcpdump_enable;

static ssize_t tcpdump_enable_show(struct device *dev __maybe_unused,
		struct device_attribute *attr __maybe_unused, char *buf)
{
	return sprintf(buf,
			"UDP|TCP|ICMP\n"
			"bit2|bit1|bit0\n"
			"enable: 0x%x\n", atc_combo_tcpdump_enable);
}

static ssize_t tcpdump_enable_store(struct device *dev __maybe_unused,
		struct device_attribute *attr __maybe_unused, const char *buf, size_t count)
{
	char *end = NULL;
	int val = (int)simple_strtol(buf, &end, 0);

	if (end == buf) { // input non-number string
		COMBO_ERR("inalid data: %s\n", buf);
		return -EINVAL;
	}
	atc_combo_tcpdump_enable = val;

	return (ssize_t)count;
}

static DEVICE_ATTR(tcpdump_enable, S_IWUSR | S_IRUGO,
		tcpdump_enable_show, tcpdump_enable_store);

static ssize_t chip_type_show(struct device *dev __maybe_unused,
		struct device_attribute *attr __maybe_unused, char *buf)
{
	return sprintf(buf, "%s\n", atc_combo_get_chip_type_str());
}

int atc_combo_set_chip_type_str(char *type_str);

static ssize_t chip_type_store(struct device *dev __maybe_unused,
		struct device_attribute *attr __maybe_unused, const char *buf, size_t count)
{
	int ret = -1;

	ret = atc_combo_set_chip_type_str(buf);

	return ret < 0 ? ret : (ssize_t)count;
}

static DEVICE_ATTR(chip_type, S_IWUSR | S_IRUGO,
		chip_type_show, chip_type_store);

#ifdef CONFIG_ATC_WIFI_CHIP_SELF_ADAPTIVE

static ssize_t detect_show(struct device *dev __maybe_unused,
		struct device_attribute *attr __maybe_unused, char *buf)
{
	return sprintf(buf, "write 1 to detect chip type\n");
}

int atc_combo_chip_type_detect(struct platform_device *pdev);

static ssize_t detect_store(struct device *dev,
		struct device_attribute *attr __maybe_unused, const char *buf, size_t count)
{
	struct platform_device *pdev = to_platform_device(dev);
	char *end = NULL;
	int val = (int)simple_strtol(buf, &end, 0);
	int ret = -1;

	if (end == buf) { // input non-number string
		COMBO_ERR("inalid data: %s\n", buf);
		return -EINVAL;
	}
	if (val) {
		ret = atc_combo_chip_type_detect(pdev);
	}

	return ret < 0 ? ret : (ssize_t)count;
}

static DEVICE_ATTR(detect, S_IWUSR | S_IRUGO,
		detect_show, detect_store);

static ssize_t power_init_show(struct device *dev __maybe_unused,
		struct device_attribute *attr __maybe_unused, char *buf)
{
	return sprintf(buf, "1: init 0: deinit\n");
}

int atc_combo_power_init(struct platform_device *pdev);
int atc_combo_power_deinit(struct platform_device *pdev);

static ssize_t power_init_store(struct device *dev,
		struct device_attribute *attr __maybe_unused, const char *buf, size_t count)
{
	struct platform_device *pdev = to_platform_device(dev);
	char *end = NULL;
	int val = (int)simple_strtol(buf, &end, 0);
	int ret = -1;

	if (end == buf) { // input non-number string
		COMBO_ERR("inalid data: %s\n", buf);
		return -EINVAL;
	}
	if (val) {
		ret = atc_combo_power_init(pdev);
	} else {
		ret = atc_combo_power_deinit(pdev);
	}

	return ret < 0 ? ret : (ssize_t)count;
}

static DEVICE_ATTR(power_init, S_IWUSR | S_IRUGO,
		power_init_show, power_init_store);

#endif // CONFIG_ATC_WIFI_CHIP_SELF_ADAPTIVE

static struct attribute *atc_combo_attrs[] = {
	&dev_attr_test.attr,
	&dev_attr_int.attr,
	&dev_attr_bt_enable.attr,
	&dev_attr_wlan_enable.attr,
	&dev_attr_chip_reset.attr,
	&dev_attr_wlan_reset.attr,
	&dev_attr_wlan_log_level.attr,
	&dev_attr_tcpdump_enable.attr,
	&dev_attr_chip_type.attr,
#ifdef CONFIG_ATC_WIFI_CHIP_SELF_ADAPTIVE
	&dev_attr_detect.attr,
	&dev_attr_power_init.attr,
#endif
	NULL
};

static const struct attribute_group atc_combo_group = {
	.attrs = atc_combo_attrs,
};

int atc_combo_power_init(struct platform_device *pdev);
int atc_combo_power_deinit(struct platform_device *pdev);
int atc_combo_power_suspend(struct device *dev __maybe_unused);
int atc_combo_power_resume(struct device *dev __maybe_unused);

static int atc_combo_probe(struct platform_device *pdev)
{
	struct atc_combo_data *pdata = NULL;
	int ret = 0;

#ifndef CONFIG_ATC_WIFI_CHIP_SELF_ADAPTIVE
	ret = atc_combo_power_init(pdev);
	if (ret) {
		COMBO_ERR("atc_combo_power_init failed(%d)\n", ret);
		return ret;
	}
#endif

	pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		COMBO_ERR("kmalloc failed\n");
		return -ENOMEM;
	}
	platform_set_drvdata(pdev, pdata);

	ret = sysfs_create_group(&pdev->dev.kobj, &atc_combo_group);
	if (ret) {
		COMBO_ERR("sysfs_create_group failed(%d)\n", ret);
		platform_set_drvdata(pdev, NULL);
		kfree(pdata);
#ifndef CONFIG_ATC_WIFI_CHIP_SELF_ADAPTIVE
		atc_combo_power_deinit(pdev);
#endif
		return ret;
	}

	return 0;
}

static int atc_combo_remove(struct platform_device *pdev)
{
	struct atc_combo_data *pdata = platform_get_drvdata(pdev);

	sysfs_remove_group(&pdev->dev.kobj, &atc_combo_group);
	platform_set_drvdata(pdev, NULL);
	kfree(pdata);
	atc_combo_power_deinit(pdev);

	return 0;
}

static int atc_combo_suspend(struct device *dev)
{
	return atc_combo_power_suspend(dev);
}

static int atc_combo_resume(struct device *dev)
{
	return atc_combo_power_resume(dev);
}

static const struct dev_pm_ops atc_combo_pm_ops = {
	.suspend = atc_combo_suspend,
	.resume = atc_combo_resume,
};

#ifdef WITHOUT_ATC_COMBO_DTS
static struct platform_device *atc_combo_device = NULL;
#endif

static const struct of_device_id atc_combo_of_match[] = {
	{ .compatible = "autochips,atc_combo", },
	{}
};
MODULE_DEVICE_TABLE(of, atc_combo_of_match);

static struct platform_driver atc_combo_driver = {
	.probe = atc_combo_probe,
	.remove = atc_combo_remove,
	.driver = {
		.name = DRV_NAME,
		.of_match_table = atc_combo_of_match,
		.pm = &atc_combo_pm_ops,
	},
};

static int __init atc_combo_init(void)
{
	int ret = -1;

#ifdef WITHOUT_ATC_COMBO_DTS
	atc_combo_device = platform_device_alloc(DRV_NAME, -1);
	if (!atc_combo_device) {
		COMBO_ERR("platform_device_alloc failed\n");
		return -ENOMEM;
	}

	ret = platform_device_add(atc_combo_device);
	if (ret) {
		COMBO_ERR("platform_device_add failed(%d)\n", ret);
		goto err_free_dev;
	}
#endif
	ret = platform_driver_register(&atc_combo_driver);
	if (ret) {
		COMBO_ERR("platform_driver_register failed(%d)\n", ret);
		goto err_delete_dev;
	}

	return 0;

err_delete_dev:
#ifdef WITHOUT_ATC_COMBO_DTS
	platform_device_del(atc_combo_device);

err_free_dev:
	platform_device_put(atc_combo_device);
	atc_combo_device = NULL;
#endif

	return ret;
}

static void __exit atc_combo_exit(void)
{
	platform_driver_unregister(&atc_combo_driver);
#ifdef WITHOUT_ATC_COMBO_DTS
	if (atc_combo_device) {
		platform_device_unregister(atc_combo_device);
		atc_combo_device = NULL;
	}
#endif
}

module_init(atc_combo_init);
module_exit(atc_combo_exit);

MODULE_DESCRIPTION("atc_combo platform device driver");
MODULE_ALIAS("atc_combo:main");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Rocky Pan");
