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
#include "wi_end.h"

#include "common.h"

static int (*set_level_callback)(int level);
static int wlan_log_level = ATC_COMBO_WLAN_LOG_LEVEL_INFO;

int atc_combo_get_wlan_log_level(void)
{
	return wlan_log_level;
}

int atc_combo_set_wlan_log_level(int level)
{
	int ret = -1;

	if (!set_level_callback) {
		COMBO_ERR("wlan_log_level_callback not ready\n");
		return -ENOSYS;
	}
	ret = set_level_callback(level);
	if (ret) {
		COMBO_ERR("set_level_callback failed %d", ret);
		return -EINVAL;
	}
	wlan_log_level = level;

	return 0;
}

int atc_combo_wlan_log_level_register_callback(
	int def_level, int (*callback)(int type))
{
	set_level_callback = callback;
	if (callback) {
		wlan_log_level = def_level;
	}

	return 0;
}
EXPORT_SYMBOL(atc_combo_wlan_log_level_register_callback);

MODULE_DESCRIPTION("wlan log level");
MODULE_ALIAS("atc_combo:wlan_log_level");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Rocky Pan");
