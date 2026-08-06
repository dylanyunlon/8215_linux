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
#include "wi_end.h"

#include "common.h"

#if defined(CONFIG_ATC_WIFI_CHIP_SELF_ADAPTIVE)
	#define ATC_WIFI_CHIP_TYPE_DEFAULT  ATC_WIFI_CHIP_TYPE_UNKNOWN
#elif defined(CONFIG_ATC_WIFI_CHIP_MT6630_SDIO)
	#define ATC_WIFI_CHIP_TYPE_DEFAULT  ATC_WIFI_CHIP_TYPE_MT6630
#elif defined(CONFIG_ATC_WIFI_CHIP_CYPRESS_PCIE)
	#define ATC_WIFI_CHIP_TYPE_DEFAULT  ATC_WIFI_CHIP_TYPE_CYPRESS_PCIE
#elif defined(CONFIG_ATC_WIFI_CHIP_CYPRESS_SDIO)
	#define ATC_WIFI_CHIP_TYPE_DEFAULT  ATC_WIFI_CHIP_TYPE_CYPRESS_SDIO
#elif defined(CONFIG_ATC_WIFI_CHIP_QCA6595_PCIE)
	#define ATC_WIFI_CHIP_TYPE_DEFAULT  ATC_WIFI_CHIP_TYPE_QCA6595_PCIE
#elif defined(CONFIG_ATC_WIFI_CHIP_AIC8800_SDIO)
	#define ATC_WIFI_CHIP_TYPE_DEFAULT  ATC_WIFI_CHIP_TYPE_AIC8800_SDIO
#else
	#define ATC_WIFI_CHIP_TYPE_DEFAULT  ATC_WIFI_CHIP_TYPE_UNKNOWN
#endif

static int atc_combo_chip_type = ATC_WIFI_CHIP_TYPE_DEFAULT;

int atc_combo_get_chip_type(void)
{
	return atc_combo_chip_type;
}
EXPORT_SYMBOL(atc_combo_get_chip_type);

int atc_combo_set_chip_type(int type)
{
	atc_combo_chip_type = type;
	return 0;
}
EXPORT_SYMBOL(atc_combo_set_chip_type);

char *atc_combo_get_chip_type_str(void)
{
	switch (atc_combo_chip_type) {
	case ATC_WIFI_CHIP_TYPE_UNKNOWN:
		return ATC_WIFI_CHIP_TYPE_STR_UNKNOWN;
	case ATC_WIFI_CHIP_TYPE_MT6630:
		return ATC_WIFI_CHIP_TYPE_STR_MT6630;
	case ATC_WIFI_CHIP_TYPE_CYPRESS_PCIE:
		return ATC_WIFI_CHIP_TYPE_STR_CYPRESS_PCIE;
	case ATC_WIFI_CHIP_TYPE_CYPRESS_SDIO:
		return ATC_WIFI_CHIP_TYPE_STR_CYPRESS_SDIO;
	case ATC_WIFI_CHIP_TYPE_QCA6595_PCIE:
		return ATC_WIFI_CHIP_TYPE_STR_QCA6595_PCIE;
	case ATC_WIFI_CHIP_TYPE_AIC8800_SDIO:
		return ATC_WIFI_CHIP_TYPE_STR_AIC8800_SDIO;
	default:
		return ATC_WIFI_CHIP_TYPE_STR_UNKNOWN;
	}
}
EXPORT_SYMBOL(atc_combo_get_chip_type_str);

int atc_combo_set_chip_type_str(char *type_str)
{
	if (!type_str) {
		COMBO_ERR("NULL pointer\n");
		return -EINVAL;
	} else if (!strcmp(type_str, ATC_WIFI_CHIP_TYPE_STR_UNKNOWN)) {
		atc_combo_chip_type = ATC_WIFI_CHIP_TYPE_UNKNOWN;
	} else if (!strcmp(type_str, ATC_WIFI_CHIP_TYPE_STR_MT6630)) {
		atc_combo_chip_type = ATC_WIFI_CHIP_TYPE_MT6630;
	} else if (!strcmp(type_str, ATC_WIFI_CHIP_TYPE_STR_CYPRESS_PCIE)) {
		atc_combo_chip_type = ATC_WIFI_CHIP_TYPE_CYPRESS_PCIE;
	} else if (!strcmp(type_str, ATC_WIFI_CHIP_TYPE_STR_CYPRESS_SDIO)) {
		atc_combo_chip_type = ATC_WIFI_CHIP_TYPE_CYPRESS_SDIO;
	} else if (!strcmp(type_str, ATC_WIFI_CHIP_TYPE_STR_QCA6595_PCIE)) {
		atc_combo_chip_type = ATC_WIFI_CHIP_TYPE_QCA6595_PCIE;
	} else if (!strcmp(type_str, ATC_WIFI_CHIP_TYPE_STR_AIC8800_SDIO)) {
		atc_combo_chip_type = ATC_WIFI_CHIP_TYPE_AIC8800_SDIO;
	} else {
		COMBO_ERR("unknown type %s\n", type_str);
		return -EINVAL;
	}
	COMBO_INFO("%s\n", type_str);

	return 0;
}
EXPORT_SYMBOL(atc_combo_set_chip_type_str);

MODULE_DESCRIPTION("atc combo builtin");
MODULE_ALIAS("atc_combo:builtin");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Rocky Pan");
