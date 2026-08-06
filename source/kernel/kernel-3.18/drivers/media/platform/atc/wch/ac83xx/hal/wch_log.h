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

#ifndef _WCH_LOG_H_
#define _WCH_LOG_H_
#ifndef __ARM2__
#include "linux/kernel.h"
#endif
extern u32 _u4WCH_DBG_LVL;
extern u8 *_pcWchLogLevel[];

#define WCH_LOG_LVL_OFF                         0
#define WCH_LOG_LVL_ERR                         1
#define WCH_LOG_LVL_WARN                        2
#define WCH_LOG_LVL_INFO                        3
#define WCH_LOG_LVL_HAL                         4
#define WCH_LOG_LVL_DBG                         5
#define WCH_LOG_LVL_TRACE                       6
#define WCH_LOG_LVL_IRQ                         7
#define WCH_LOG_LVL_REGRW                       8
#ifdef __ARM2__
#define WCH_LOG(lvl, formatStr, ...)\
    do{ \
        if (lvl <= _u4WCH_DBG_LVL) {\
            printk("[ARM2 WCH]"formatStr, ##__VA_ARGS__);\
        }\
    } while (0)

#else
#define WCH_LOG(lvl, formatStr, ...)\
do { \
	if (lvl <= _u4WCH_DBG_LVL) {\
		if (lvl == WCH_LOG_LVL_ERR) {\
			pr_err("%s: "formatStr, _pcWchLogLevel[lvl], ##__VA_ARGS__); \
		} \
		else if (lvl == WCH_LOG_LVL_WARN) {\
			pr_warn("%s: "formatStr, _pcWchLogLevel[lvl], ##__VA_ARGS__); \
		} \
		else if (lvl == WCH_LOG_LVL_INFO) {\
			pr_info("%s: "formatStr, _pcWchLogLevel[lvl], ##__VA_ARGS__); \
		} \
		else if (lvl == WCH_LOG_LVL_HAL) {\
			pr_info("%s: "formatStr, _pcWchLogLevel[lvl], ##__VA_ARGS__); \
		} \
		else {\
			pr_debug("%s: "formatStr, _pcWchLogLevel[lvl], ##__VA_ARGS__); \
		} \
	} \
} while (0)
#endif
#endif
