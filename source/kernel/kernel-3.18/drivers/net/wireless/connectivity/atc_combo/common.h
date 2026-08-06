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

#ifndef __ATC_COMBO_COMMON_H
#define __ATC_COMBO_COMMON_H

#include <linux/version.h>
#include "atc_combo.h"

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) "[atc_combo]" fmt

#define COMBO_DBG(fmt, args...) \
	pr_debug("[D]%s,%d: " fmt, __func__, __LINE__, ## args)

#define COMBO_INFO(fmt, args...) \
	pr_info("[I]%s,%d: " fmt, __func__, __LINE__, ## args)

#define COMBO_WARN(fmt, args...) \
	pr_warn("[W]%s,%d:WARN! " fmt, __func__, __LINE__, ## args)

#define COMBO_ERR(fmt, args...) \
	pr_err("[E]%s,%d:ERROR! " fmt, __func__, __LINE__, ## args)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0) \
		&& LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
#define WITHOUT_ATC_COMBO_DTS   1
#endif

#endif /* __ATC_COMBO_COMMON_H */
