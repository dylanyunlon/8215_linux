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

#ifndef _IMGRESZ_LOG_H_
#define _IMGRESZ_LOG_H_

//#include "linux/kernel.h"
#include "types.h"

extern u32 _u4IMGR_DBG_LVL;
extern u8 *_pcImgreszLogLevel[];

#define IMGR_LOG_LVL_OFF			 0
#define IMGR_LOG_LVL_ERR			 1
#define IMGR_LOG_LVL_WARN			 2
#define IMGR_LOG_LVL_CLI			 3
#define IMGR_LOG_LVL_INFO			 4
#define IMGR_LOG_LVL_HAL			 5
#define IMGR_LOG_LVL_IRQ			 6
#define IMGR_LOG_LVL_TRACE			 7
#define IMGR_LOG_LVL_DBG			 8
#define IMGR_LOG_LVL_REGRW			 9
#define IMGR_LOG_TAG            "[IMGRSZ] "

#define IMGR_LOG(lvl, formatStr, args...)\
{\
	if (lvl <= _u4IMGR_DBG_LVL) {\
		if (lvl == IMGR_LOG_LVL_ERR) {\
			printk("[IMGRSZ] "formatStr, ##args);\
		} else if (lvl == IMGR_LOG_LVL_WARN) {\
			printk("[IMGRSZ] "formatStr, ##args);\
		} else if (lvl == IMGR_LOG_LVL_INFO) {\
			printk("[IMGRSZ] "formatStr, ##args);\
		} else if (lvl == IMGR_LOG_LVL_DBG) {\
			printk("[IMGRSZ] "formatStr, ##args);\
		} else {\
		} \
	} \
}

#define X_Printf(...)  printk(__VA_ARGS__)

#endif


