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

#ifndef __ATC_DT_LOG_H__
#define __ATC_DT_LOG_H__

#ifndef LOG_TAG
#define LOG_TAG "DT"
#endif
#if 1
#include "mm_log.h"

#define PRINT_DEBUG(format, arg...)  MMLOG_DEBUG(LOG_MOD_DT, format, ##arg)
#define PRINT_TRACE(format, arg...)  MMLOG_TRACE(LOG_MOD_DT, format, ##arg)
#define PRINT_INFO(format, arg...)   MMLOG_INFO(LOG_MOD_DT, format, ##arg)
#define PRINT_ERROR(format, arg...)  MMLOG_ERROR(LOG_MOD_DT, format, ##arg)
#else
#include "mm_log.h"
#include "stdio.h"

#define PRINT_BASE(lvl, format, arg...) do {   \
    struct timespec ts;                             \
    struct tm tm;                                   \
    clock_gettime(CLOCK_REALTIME, &ts);             \
    localtime_r(&ts.tv_sec, &tm);                   \
    fprintf(stderr, "%02d:%02d:%02d.%03u " lvl "[MM]" LOG_TAG "[%s:%d] " format "\n", \
         tm.tm_hour, tm.tm_min, tm.tm_sec, (unsigned)(ts.tv_nsec / 1000000), \
         __FUNCTION__, __LINE__, ##arg); \
} while(0)

#define PRINT_ERROR(format, arg...)  PRINT_BASE("[E]", format, ##arg)
#define PRINT_INFO(format, arg...) PRINT_BASE("[I]", format, ##arg)
#define PRINT_TRACE(format, arg...) PRINT_BASE("[I]", format, ##arg)
#define PRINT_WARNING(format, arg...) PRINT_BASE("[W]", format, ##arg)
#define PRINT_DEBUG(format, arg...) PRINT_BASE("[D]", format, ##arg)
#endif

#endif /* __ATC_DT_LOG_H__ */

