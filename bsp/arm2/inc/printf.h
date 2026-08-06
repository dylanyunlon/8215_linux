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

#ifndef _PRINTF_H__
#define _PRINTF_H__
#include "x_typedef.h"

INT32 Printf(const CHAR *format, ...);

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
extern const char *plevel[XLOG_LEVEL_MAX];

#define  xprint_with_tag(level,  tags,  fmt,  ...) \
do { \
    if (level <= def_dbg_level) { \
        if (level <= XLOG_ERR) \
            Printf("%s[%s] File:%s FUNC:%s Line:%d: " fmt, plevel[level], tags, __FILE__, __func__, __LINE__,  ##__VA_ARGS__); \
        else \
            Printf("%s[%s] " fmt, plevel[level], tags, ##__VA_ARGS__); \
    } \
} while (0)

#define pr_debug(fmt, ...)		xprint_with_tag(XLOG_DEBUG, TAGS, fmt, ##__VA_ARGS__)
#define pr_info(fmt, ...)		xprint_with_tag(XLOG_INFO, TAGS, fmt, ##__VA_ARGS__)
#define pr_warn(fmt, ...)		xprint_with_tag(XLOG_WARNING, TAGS, fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...)		xprint_with_tag(XLOG_ERR, TAGS, fmt, ##__VA_ARGS__)
#define pr_crit(fmt, ...)		xprint_with_tag(XLOG_CRIT, TAGS, fmt, ##__VA_ARGS__)

#endif  //_PRINTF_H__ 
