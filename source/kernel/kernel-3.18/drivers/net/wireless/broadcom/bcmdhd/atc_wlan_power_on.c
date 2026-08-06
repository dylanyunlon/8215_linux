/*
* Copyright (c) 2025 AutoChips Inc.
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
#include <linux/module.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#include <../../atc_modules/connectivity/atc_combo/atc_combo.h>
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
#include <../drivers/soc/autochips/connectivity/atc_combo/atc_combo.h>
#else // 3.18
#include <../drivers/net/wireless/connectivity/atc_combo/atc_combo.h>
#endif

#ifdef CONFIG_ATC_WLAN_TRANSMISSION_MODE_SDIO
	#ifdef CONFIG_ARCH_AC83XX
		#define WLAN_SLOT   1
		#ifdef CONFIG_ATC_OS_VERSION_JB2
		extern void msdc_detect_change(u32 slot, u32 enable, u32 type);
		#else // linux
		extern int msdc_detect_change(u32 slot, u32 enable, u32 type);
		#endif

	#elif defined(CONFIG_ARCH_AC8X)
		#define WLAN_SLOT   1 // Demo: sd1 EVB: sd2
		extern int sdhci_cadence_detect_change(u32 slot, u32 enable);

	#elif defined(CONFIG_ATC_8025)
		#define WLAN_SLOT   2
		extern int sdhci_cadence_detect_change(u32 slot, u32 enable);
	#endif

#elif defined(CONFIG_ATC_WLAN_TRANSMISSION_MODE_PCIE)
	#define WLAN_SLOT   1
	int pcie_host_detect_change(u32 slot, u32 enable);
#endif

static int atc_wlan_detect_change(int on)
{
	int ret = -1;

#ifdef CONFIG_ATC_WLAN_TRANSMISSION_MODE_SDIO
#ifdef CONFIG_ARCH_AC83XX
	#ifdef CONFIG_ATC_OS_VERSION_JB2
	msdc_detect_change(WLAN_SLOT, on, 0);
	ret = 0;
	#else
	ret = msdc_detect_change(WLAN_SLOT, on, 0);
	#endif
#elif defined(CONFIG_ARCH_AC8X) || defined(CONFIG_ATC_8025)
	ret = sdhci_cadence_detect_change(WLAN_SLOT, on);
#else
	pr_err("%s not implemented\n", __func__);
	ret = -ENOSYS;
#endif
#elif defined(CONFIG_ATC_WLAN_TRANSMISSION_MODE_PCIE)
	ret = pcie_host_detect_change(WLAN_SLOT, on);
#endif
	if (ret) {
		pr_err("%s(%d) failed %d\n", __func__, on, ret);
		return ret;
	}

	return 0;
}

int atc_wlan_power_on(int on)
{
	int ret = -1;

	pr_info("%s(%d) slot %d\n", __func__, on, WLAN_SLOT);

#ifdef CONFIG_ATC_WLAN_TRANSMISSION_MODE_PCIE
	if (!on) {
		ret = atc_wlan_detect_change(0);
		if (ret) {
			pr_err("atc_wlan_detect_change failed %d\n", ret);
			return ret;
		}
	}
#endif
	ret = atc_combo_wlan_enable(on);
	if (ret) {
		pr_err("atc_combo_wlan_enable(%d) failed %d\n", on, ret);
		return ret;
	}
#ifdef CONFIG_ATC_WLAN_TRANSMISSION_MODE_SDIO
	ret = atc_wlan_detect_change(on);
	if (ret) {
		pr_err("atc_wlan_detect_change failed %d\n", ret);
		return ret;
	}
#elif defined(CONFIG_ATC_WLAN_TRANSMISSION_MODE_PCIE)
	if (on) {
		ret = atc_wlan_detect_change(1);
		if (ret) {
			atc_combo_wlan_enable(0);
			atc_combo_wlan_enable(1);
			ret = atc_wlan_detect_change(1);
			if (ret) {
				atc_combo_wlan_enable(0);
				pr_err("atc_wlan_detect_change failed %d\n", ret);
				return ret;
			}
		}
	}
#endif

	pr_info("%s(%d) ok\n", __func__, on);

	return 0;
}
