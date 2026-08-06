/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */


#ifndef X_PRINTF_H
#define X_PRINTF_H

#include "x_typedef.h"
#include "Simulation_log.h"

//extern INT32 Printf(const CHAR *format, ...);
//extern INT32 SPrintf(CHAR *out, const CHAR *format, ...);
#define XLOG_EMERG      0
#define XLOG_ALERT      1
#define XLOG_CRIT       2
#define XLOG_ERR        3
#define XLOG_WARNING    4
#define XLOG_NOTICE     5
#define XLOG_INFO       6
#define XLOG_DEBUG      7
#define XLOG_LEVEL_MAX  8
extern unsigned def_dbg_level;
#if(UART_ENABLE)

#ifndef printf
#define printf      UART_Printf      //lint !e683 function #define'd
#endif

#define Printf  UART_Printf
#define getc   UART_GetChar
#define getnum UART_GetNum
#define getstring UART_GetString
#elif(SIMULATION_LOG)

#define printf  SIM_PRINTF
#define Printf  SIM_PRINTF

#else

//#ifndef sprintf
//#define sprintf     SPrintf     //lint !e683 function #define'd
//#endif
#define Printf
#define getnum
#define getstring

#endif
#if 0
#ifdef CONFIG_ATC_USER
#define  xprint_with_tag(level,  tags,  fmt,  ...) \
do { \
    if (level <= def_dbg_level) { \
        if (level <= XLOG_ERR) \
            Printf("%s[%s] File:%s FUNC:%s Line:%d: " fmt, plevel[level], tags, __FILE__, __func__, __LINE__,  ##__VA_ARGS__); \
        else \
            Printf("%s[%s] " fmt, plevel[level], tags, ##__VA_ARGS__); \
    } \
} while (0)
#else
#define  xprint_with_tag(level,  tags,  fmt,  ...) \
do { \
    if (level <= def_dbg_level) { \
        if (level <= XLOG_ERR) \
            Printf("%s[%s] File:%s FUNC:%s Line:%d: " fmt, plevel[level], tags, __FILE__, __func__, __LINE__,  ##__VA_ARGS__); \
        else \
            Printf("%s[%s] " fmt, plevel[level], tags, ##__VA_ARGS__); \
    } \
} while (0)
#endif //CONFIG_ATC_OS_USER

#define pr_debug(fmt, ...)		xprint_with_tag(XLOG_DEBUG, TAGS, fmt, ##__VA_ARGS__)
#define pr_info(fmt, ...)		xprint_with_tag(XLOG_INFO, TAGS, fmt, ##__VA_ARGS__)
#define pr_notice(fmt, ...)		xprint_with_tag(XLOG_NOTICE, TAGS, fmt, ##__VA_ARGS__)
#define pr_warning(fmt, ...)		xprint_with_tag(XLOG_WARNING, TAGS, fmt, ##__VA_ARGS__)
#define pr_warn pr_warning
#define pr_err(fmt, ...)		xprint_with_tag(XLOG_ERR, TAGS, fmt, ##__VA_ARGS__)
#define pr_crit(fmt, ...)		xprint_with_tag(XLOG_CRIT, TAGS, fmt, ##__VA_ARGS__)
#define pr_alert(fmt, ...)		xprint_with_tag(XLOG_ALERT, TAGS, fmt, ##__VA_ARGS__)
#define pr_emery(fmt, ...)		xprint_with_tag(XLOG_EMERG, TAGS, fmt, ##__VA_ARGS__)
#define pr_time(fmt, ...)   		xprint_with_tag(XLOG_INFO, TAGS, fmt, ##__VA_ARGS__)

#define printf_debug(fmt, ...)	pr_debug(fmt, ##__VA_ARGS__)
#define printf_info(fmt, ...)	pr_info(fmt, ##__VA_ARGS__)
#define printf_warn(fmt, ...)	pr_warning(fmt, ##__VA_ARGS__)
#define printf_error(fmt, ...)	pr_err(fmt, ##__VA_ARGS__)
#endif




#endif  // X_PRINTF_H
