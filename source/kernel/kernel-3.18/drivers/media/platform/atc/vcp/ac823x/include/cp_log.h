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
/*
 *Department: ATC-SD-SD2
 *
 *Aurthor: yezhiqiang(atc0064)
 *
 *Date: 2017-03-29
 */
#ifndef _CP_LOG_H_
#define _CP_LOG_H_

#ifndef __ARM2__
#include "linux/kernel.h"
#endif
extern unsigned int _u4CP_DBG_LVL;
extern unsigned char *_pcCpLogLevel[];

#define CP_LOG_LVL_ERR                         0
#define CP_LOG_LVL_WARN                        1
#define CP_LOG_LVL_INFO                        2
#define CP_LOG_LVL_HAL                         3
#define CP_LOG_LVL_DBG                         4
#define CP_LOG_LVL_IRQ                         5

#ifdef __ARM2__
/* arm2 */
#define CP_LOG(lvl, formatStr, ...)\
    do{ \
        if (lvl <= _u4CP_DBG_LVL) {\
            printk("[ARM2 CP]"formatStr, ##__VA_ARGS__);\
        }\
    } while (0)

#else
/* kernel */
#define CP_LOG(lvl, formatStr, ...)\
do { \
	if (lvl <= _u4CP_DBG_LVL) {\
		if (lvl == CP_LOG_LVL_ERR) {\
			pr_err("%s: "formatStr, _pcCpLogLevel[lvl], ##__VA_ARGS__); \
		} \
		else if (lvl == CP_LOG_LVL_WARN) {\
			pr_warn("%s: "formatStr, _pcCpLogLevel[lvl], ##__VA_ARGS__); \
		} \
		else if (lvl == CP_LOG_LVL_INFO || lvl == CP_LOG_LVL_HAL) {\
			pr_info("%s: "formatStr, _pcCpLogLevel[lvl], ##__VA_ARGS__); \
		} \
		else {\
			pr_debug("%s: "formatStr, _pcCpLogLevel[lvl], ##__VA_ARGS__); \
		} \
	} \
} while (0)

#endif


#endif
