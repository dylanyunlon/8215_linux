/*
 *Department: ATC-SD-SD2
 *
 *Aurthor: yezhiqiang(atc0064)
 *
 *Date: 2016-12-05
 */

#ifndef _WCH_LOG_H_
#define _WCH_LOG_H_


#ifndef __ARM2__
#include "linux/kernel.h"
#endif
extern unsigned int _u4WCH_DBG_LVL;
extern unsigned char *_pcWchLogLevel[];

#define WCH_LOG_LVL_ERR                         0
#define WCH_LOG_LVL_WARN                        1
#define WCH_LOG_LVL_INFO                        2
#define WCH_LOG_LVL_HAL                         3
#define WCH_LOG_LVL_DBG                         4
#define WCH_LOG_LVL_IRQ                         5

#ifdef __ARM2__
/* arm2 */
#define WCH_LOG(lvl, formatStr, ...)\
    do{ \
        if (lvl <= _u4WCH_DBG_LVL) {\
            printk("[ARM2 WCH]"formatStr, ##__VA_ARGS__);\
        }\
    } while (0)

#else
/* kernel */
#define WCH_LOG(lvl, formatStr, ...)\
do { \
	if (lvl <= _u4WCH_DBG_LVL) {\
		if (lvl == WCH_LOG_LVL_ERR) {\
			pr_err("%s: "formatStr, _pcWchLogLevel[lvl], ##__VA_ARGS__); \
		} \
		else if (lvl == WCH_LOG_LVL_WARN) {\
			pr_warn("%s: "formatStr, _pcWchLogLevel[lvl], ##__VA_ARGS__); \
		} \
		else if (lvl == WCH_LOG_LVL_INFO || \
			lvl == WCH_LOG_LVL_HAL || \
			lvl == WCH_LOG_LVL_DBG ) {\
			pr_info("%s: "formatStr, _pcWchLogLevel[lvl], ##__VA_ARGS__); \
		} \
		else {\
			pr_debug("%s: "formatStr, _pcWchLogLevel[lvl], ##__VA_ARGS__); \
		} \
	} \
} while (0)

#endif

#endif
