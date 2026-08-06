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
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include "wi_end.h"

#include "common.h"

static int (*wlan_reset_callback)(int type);
static int wlan_reset_type = 0;
static unsigned long wlan_reset_triggered = 0;
static struct task_struct *_wlan_reset_thread;

static int wlan_reset_thread(void *data __maybe_unused)
{
	int ret = -1;

	if (!wlan_reset_callback) {
		COMBO_ERR("wlan_reset_callback NULL\n");
		clear_bit(0, &wlan_reset_triggered);
		return -ENXIO;
	}
	COMBO_INFO("wlan_reset_callback(%d) start\n", wlan_reset_type);
	ret = wlan_reset_callback(wlan_reset_type);
	clear_bit(0, &wlan_reset_triggered);
	COMBO_INFO("wlan_reset_callback(%d) end ret %d\n", wlan_reset_type, ret);

	return ret;
}

int atc_combo_wlan_reset_register_callback(int (*callback)(int type))
{
	if (wlan_reset_callback != callback) {
		wlan_reset_callback = callback;
	}

	return 0;
}
EXPORT_SYMBOL(atc_combo_wlan_reset_register_callback);

int atc_combo_wlan_reset_trigger(int type)
{
	int ret = -1;

	COMBO_INFO("type %d\n", type);

	if (!wlan_reset_callback) {
		COMBO_ERR("wlan_reset_callback NULL\n");
		return -ENXIO;
	}
	if (test_and_set_bit(0, &wlan_reset_triggered)) {
		COMBO_INFO("already triggered type %d\n", wlan_reset_type);
		return 0;
	}
	dump_stack();
	if (in_atomic()) { // kthread_run maybe schedule
		COMBO_INFO("maybe scheduling while atomic, "
			"preempt_count 0x%08x, return fail\n", preempt_count());
		return -EFAULT;
	}
	wlan_reset_type = type;
	_wlan_reset_thread = kthread_run(
			wlan_reset_thread, NULL, "wlan_reset_thread");
	if (IS_ERR(_wlan_reset_thread)) {
		ret = (int)PTR_ERR(_wlan_reset_thread);
		COMBO_ERR("kthread_run failed %d\n", ret);
		clear_bit(0, &wlan_reset_triggered);
		_wlan_reset_thread = NULL;
		return ret;
	}

	return 0;
}
EXPORT_SYMBOL(atc_combo_wlan_reset_trigger);

bool atc_combo_wlan_reset_is_triggered(void)
{
	return test_bit(0, &wlan_reset_triggered);
}
EXPORT_SYMBOL(atc_combo_wlan_reset_is_triggered);

MODULE_DESCRIPTION("wlan reset");
MODULE_ALIAS("atc_combo:wlan_reset");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Rocky Pan");
