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

#ifndef __ATC_COMBO_H
#define __ATC_COMBO_H

#include "wi_begin.h"
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/mmc/sdio_func.h>
#include "wi_end.h"

/* debug trigger */
extern int atc_combo_int;

/* task dump api */
struct task_struct *atc_combo_find_task_by_name(const char *name);
void atc_combo_dump_single_task(struct task_struct *tsk);
void atc_combo_dump_task_by_name(const char *name);
void atc_combo_dump_cpu_task_simple(void);
void atc_combo_dump_cpu_task_trigger(bool wait);

/* time check api */
#define atc_combo_time_check(end) \
	_atc_combo_time_check(__func__, __LINE__, end)
void _atc_combo_time_check(const char *func, int line, bool end);

/* irq dump api */
void atc_combo_dump_irq(unsigned int irq);

/* chip type api */
#define ATC_WIFI_CHIP_TYPE_STR_UNKNOWN          "unknown"
#define ATC_WIFI_CHIP_TYPE_STR_MT6630           "mt6630"
#define ATC_WIFI_CHIP_TYPE_STR_CYPRESS_PCIE     "cypress_pcie"
#define ATC_WIFI_CHIP_TYPE_STR_CYPRESS_SDIO     "cypress_sdio"
#define ATC_WIFI_CHIP_TYPE_STR_QCA6595_PCIE     "qca6595_pcie"
#define  ATC_WIFI_CHIP_TYPE_STR_AIC8800_SDIO    "aic8800_sdio"
enum {
	ATC_WIFI_CHIP_TYPE_UNKNOWN      = 0,
	ATC_WIFI_CHIP_TYPE_MT6630       = 1,
	ATC_WIFI_CHIP_TYPE_CYPRESS_PCIE = 2,
	ATC_WIFI_CHIP_TYPE_CYPRESS_SDIO = 3,
	ATC_WIFI_CHIP_TYPE_QCA6595_PCIE = 4,
	ATC_WIFI_CHIP_TYPE_AIC8800_SDIO = 5,
	ATC_WIFI_CHIP_TYPE_MAX
};
/* builtin api */
#if IS_ENABLED(CONFIG_ATC_COMBO)
int atc_combo_get_chip_type(void);
int atc_combo_set_chip_type(int type);
char *atc_combo_get_chip_type_str(void);
int atc_combo_set_chip_type_str(char *type_str);
#else
static inline int atc_combo_get_chip_type(void)
{
	return ATC_WIFI_CHIP_TYPE_UNKNOWN;
}

static inline int atc_combo_set_chip_type(int type)
{
	return -EOPNOTSUPP;
}

static inline char *atc_combo_get_chip_type_str(void)
{
	return ATC_WIFI_CHIP_TYPE_STR_UNKNOWN;
}

static inline int atc_combo_set_chip_type_str(char *type_str)
{
	return -EOPNOTSUPP;
}
#endif

/* bt wlan power api */
enum {
	COMBO_RESET_TYPE_OFF     = 0, // power off
	COMBO_RESET_TYPE_RESET   = 1, // power off + re-enable
	COMBO_RESET_TYPE_RECOVER = 2, // TODO: reset + driver flow
	COMBO_RESET_TYPE_MAX
};
int atc_combo_bt_enable(int on);
bool atc_combo_bt_is_enabled(void);
int atc_combo_wlan_enable(int on);
bool atc_combo_wlan_is_enabled(void);
int atc_combo_chip_reset(int type);

/* wlan driver reset api */
int atc_combo_wlan_reset_register_callback(int (*callback)(int type));
int atc_combo_wlan_reset_trigger(int type);
bool atc_combo_wlan_reset_is_triggered(void);

/* wlan log level api */
enum {
	ATC_COMBO_WLAN_LOG_LEVEL_INFO  = 0,
	ATC_COMBO_WLAN_LOG_LEVEL_DEBUG = 1,
	ATC_COMBO_WLAN_LOG_LEVEL_LOUD  = 2,
};
int atc_combo_wlan_log_level_register_callback(
		int def_level, int (*callback)(int type));

/* dump network package */
bool atc_combo_tcpdump(void *ethhdr, bool tx, u32 seq);

/* sdio extented/debug api */

/**
 * @Description: timeout version of sdio_claim_host
 * @Param: @func          [IN] sdio_func
 *         @timeout_msecs [IN]
 *             -1       - direct call sdio_claim_host
 *             0        - try claim and don't wait
 *             positive - wait as long as timeout_msecs
 * @Return: Return 0 on success, or negtive on fail.
 */
int atc_combo_sdio_claim_host_timeout(struct sdio_func *func, int timeout_msecs);

#endif /* __ATC_COMBO_H */
