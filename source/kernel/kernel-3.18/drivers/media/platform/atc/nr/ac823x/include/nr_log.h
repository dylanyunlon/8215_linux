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
 *Date: 2017-03-10
 */

#ifndef _NR_LOG_H_
#define _NR_LOG_H_


#ifndef __ARM2__
#include "linux/kernel.h"
#endif
extern unsigned int _u4NR_DBG_LVL;
extern unsigned char *_pcNrLogLevel[];

#define NR_LOG_LVL_ERR                         0
#define NR_LOG_LVL_WARN                        1
#define NR_LOG_LVL_INFO                        2
#define NR_LOG_LVL_HAL                         3
#define NR_LOG_LVL_DBG                         4
#define NR_LOG_LVL_IRQ                         5

#ifdef __ARM2__
/* arm2 */
#define NR_LOG(lvl, formatStr, ...)\
		 do{ \
			 if (lvl <= _u4NR_DBG_LVL) {\
				 printk("[ARM2 NR]"formatStr, ##__VA_ARGS__);\
			 }\
		 } while (0)
#else
/* kernel */
#define NR_LOG(lvl, formatStr, ...)\
	 do { \
		 if (lvl <= _u4NR_DBG_LVL) {\
			 if (lvl == NR_LOG_LVL_ERR) {\
				 pr_err("%s: "formatStr, _pcNrLogLevel[lvl], ##__VA_ARGS__); \
			 } \
			 else if (lvl == NR_LOG_LVL_WARN) {\
				 pr_warn("%s: "formatStr, _pcNrLogLevel[lvl], ##__VA_ARGS__); \
			 } \
			 else if (lvl == NR_LOG_LVL_INFO || lvl == NR_LOG_LVL_HAL) {\
				 pr_info("%s: "formatStr, _pcNrLogLevel[lvl], ##__VA_ARGS__); \
			 } \
			 else {\
				 pr_debug("%s: "formatStr, _pcNrLogLevel[lvl], ##__VA_ARGS__); \
			 } \
		 } \
	 } while (0)

#endif

#endif

