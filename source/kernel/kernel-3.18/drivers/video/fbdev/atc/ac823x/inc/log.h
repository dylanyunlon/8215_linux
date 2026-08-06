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
#ifndef _DEBUG_LOG_H

#define _DEBUG_LOG_H

extern __u32 fb_log_lvl;
extern __u8 *fb_lvl_str[];

#define FB_LOG_LVL_OFF                         0
#define FB_LOG_LVL_ERR                         1
#define FB_LOG_LVL_WARN                        2
#define FB_LOG_LVL_CLI                         3
#define FB_LOG_LVL_INFO                        4
#define FB_LOG_LVL_HAL                         5
#define FB_LOG_LVL_IRQ                         6
#define FB_LOG_LVL_TRACE                       7
#define FB_LOG_LVL_DBG                         8
#define FB_LOG_LVL_REGRW                       9

#ifdef __ARM2__
#define FB_PRINT(lvl, format, ...) \
{ \
	if (lvl <= fb_log_lvl) { \
		printk("[Display]: "format, ##__VA_ARGS__); \
	} \
}
#else
#define FB_PRINT(lvl, format, ...) \
{ \
	if (lvl <= fb_log_lvl) { \
		if (lvl == FB_LOG_LVL_ERR) { \
			pr_err("%s: "format, fb_lvl_str[lvl], ##__VA_ARGS__); \
		} else if (lvl == FB_LOG_LVL_WARN) { \
			pr_warn("%s: "format, fb_lvl_str[lvl], ##__VA_ARGS__); \
		} else if (lvl == FB_LOG_LVL_INFO) { \
			pr_info("%s: "format, fb_lvl_str[lvl], ##__VA_ARGS__); \
		} else if (lvl == FB_LOG_LVL_DBG) { \
			pr_debug("%s: "format, fb_lvl_str[lvl], ##__VA_ARGS__); \
		} else { \
			pr_debug("%s: "format, fb_lvl_str[lvl], ##__VA_ARGS__); \
		} \
	} \
}
#endif

extern __u32 _u4VIDEO_DBG_LVL;
extern __u8 *_pcVideoLogLevel[];

#define VDO_LOG_LVL_OFF                         0
#define VDO_LOG_LVL_ERR                         1
#define VDO_LOG_LVL_WARN                        2
#define VDO_LOG_LVL_INFO                        3
#define VDO_LOG_LVL_TRACE                       4
#define VDO_LOG_LVL_DBG                         5
#define VDO_LOG_LVL_IRQ                         6
#define VDO_LOG_LVL_REGRW                       7

#ifdef __ARM2__
#define VDO_LOG(lvl, format, ...) \
{ \
	if (lvl <= _u4VIDEO_DBG_LVL) { \
		printk("[Display]: "format, ##__VA_ARGS__); \
	} \
}
#else
#define VDO_LOG(lvl, format, ...) \
{ \
	if (lvl <= _u4VIDEO_DBG_LVL) { \
		if (lvl == VDO_LOG_LVL_ERR) { \
			pr_err("%s: "format, _pcVideoLogLevel[lvl], ##__VA_ARGS__); \
		} else if (lvl == VDO_LOG_LVL_WARN) { \
			pr_warn("%s: "format, _pcVideoLogLevel[lvl], ##__VA_ARGS__); \
		} else if (lvl == VDO_LOG_LVL_INFO) { \
			pr_info("%s: "format, _pcVideoLogLevel[lvl], ##__VA_ARGS__); \
		} else if (lvl <= VDO_LOG_LVL_DBG) { \
			pr_info("%s: "format, _pcVideoLogLevel[lvl], ##__VA_ARGS__); \
		} else { \
			pr_info("%s: "format, _pcVideoLogLevel[lvl], ##__VA_ARGS__); \
		} \
	} \
}
#endif

#endif


