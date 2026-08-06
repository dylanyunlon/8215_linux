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
#define FB_PRINT(lvl, tag, format, ...) \
{ \
	if (lvl <= fb_log_lvl) { \
	} \
}
#else
#define FB_PRINT(lvl, tag, format, ...) \
{ \
	if (lvl <= fb_log_lvl) { \
                if(strcmp(tag,"") == 0) { \
                        if (lvl == FB_LOG_LVL_ERR) { \
        			pr_err("[FB] %s: "format, fb_lvl_str[lvl], ##__VA_ARGS__); \
        		} else if (lvl == FB_LOG_LVL_WARN) { \
        			pr_warn("[FB] %s: "format, fb_lvl_str[lvl], ##__VA_ARGS__); \
        		} else if (lvl == FB_LOG_LVL_INFO) { \
        			pr_info("[FB] %s: "format, fb_lvl_str[lvl], ##__VA_ARGS__); \
        		} else if (lvl == FB_LOG_LVL_DBG) { \
        			pr_debug("[FB] %s: "format, fb_lvl_str[lvl], ##__VA_ARGS__); \
        		} else { \
        			pr_debug("[FB] %s: "format, fb_lvl_str[lvl], ##__VA_ARGS__); \
        		} \
                } else { \        
        		if (lvl == FB_LOG_LVL_ERR) { \
        			pr_err("[FB][%s] %s: "format, tag, fb_lvl_str[lvl], ##__VA_ARGS__); \
        		} else if (lvl == FB_LOG_LVL_WARN) { \
        			pr_warn("[FB][%s] %s: "format, tag, fb_lvl_str[lvl], ##__VA_ARGS__); \
        		} else if (lvl == FB_LOG_LVL_INFO) { \
        			pr_info("[FB][%s] %s: "format, tag, fb_lvl_str[lvl], ##__VA_ARGS__); \
        		} else if (lvl == FB_LOG_LVL_DBG) { \
        			pr_debug("[FB][%s] %s: "format, tag, fb_lvl_str[lvl], ##__VA_ARGS__); \
        		} else { \
        			pr_debug("[FB][%s] %s: "format, tag, fb_lvl_str[lvl], ##__VA_ARGS__); \
        		} \
                } \
	} \
}
#endif

#endif


