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

#ifndef CLOG_H
#define CLOG_H

#include <unistd.h>
#include <syslog.h>

#define USE_SYSTEM_LOG  1
#define DEBUG           false

#if USE_SYSTEM_LOG

#ifndef OPENLOG
#define OPENLOG(name)   openlog(name, \
        LOG_CONS | LOG_PERROR, LOG_USER)
#endif

#ifndef CLOSELOG
#define CLOSELOG    closelog
#endif

#ifndef LOGMASK
#define LOGMASK     setlogmask
#endif

#endif // USE_SYSTEM_LOG

#ifndef LOGE
#define LOGE        UTILS_LOGE
#endif

#ifndef LOGW
#define LOGW        UTILS_LOGW
#endif

#ifndef LOGI
#define LOGI        UTILS_LOGI
#endif

#ifndef LOGD
#define LOGD        UTILS_LOGD
#endif

#define UTILS_LOGE(tag, fmt, args...) \
    cluster_utils::CLog::printLogEx("E", tag, \
            __FILE__, __func__, __LINE__, fmt, ##args)

#define UTILS_LOGW(tag, fmt, args...) \
    cluster_utils::CLog::printLogEx("W", tag, \
            __FILE__, __func__, __LINE__, fmt, ##args)

#define UTILS_LOGI(tag, fmt, args...) \
    cluster_utils::CLog::printLogEx("I", tag, \
            __FILE__, __func__, __LINE__, fmt, ##args)

#define UTILS_LOGD(tag, fmt, args...) \
    cluster_utils::CLog::printLogEx("D", tag, \
            __FILE__, __func__, __LINE__, fmt, ##args)

#define DUMP(data, length) \
    cluster_utils::CLog::dump(data, length)

#define TRACE_CALL_IN \
    cluster_utils::CLog::printLogEx("I", NULL, \
            __FILE__, __func__, __LINE__, \
            "%s %s", __func__, "call in")

#define TRACE_CALL_OUT \
    cluster_utils::CLog::printLogEx("I", NULL, \
            __FILE__, __func__, __LINE__, \
            "%s %s", __func__, "call out")

#define TRACE_FUNCTION \
    cluster_utils::CLog::printLogEx("I", NULL, \
            __FILE__, __func__, __LINE__, \
            "%s %s", __func__, "trace")

/* No trace messages to be generated    */
#define UNIVERSAL_UTILE_LOG_LEVEL_NONE    0
/* Error condition trace messages       */
#define UNIVERSAL_UTILE_LOG_LEVEL_ERROR   1
/* Warning condition trace messages     */
#define UNIVERSAL_UTILE_LOG_LEVEL_WARNING 2
/* Debug messages for events            */
#define UNIVERSAL_UTILE_LOG_LEVEL_INFO    3
/* Full debug messages                  */
#define UNIVERSAL_UTILE_LOG_LEVEL_DEBUG   4

/* format: [pid][tid][level][tag] file:func():line log
 * format: [1234][5678][I][FOO] foo.c:foo():123 demo log
 */
#define UNIVERSAL_UTILS_LOG_FLAG_PID    (1 << 0)
#define UNIVERSAL_UTILS_LOG_FLAG_TID    (1 << 1)
#define UNIVERSAL_UTILS_LOG_FLAG_FILE   (1 << 2)
#define UNIVERSAL_UTILS_LOG_FLAG_FUNC   (1 << 3)
#define UNIVERSAL_UTILS_LOG_FLAG_LINE   (1 << 4)
/* LOGW/LOGE print func:line */
#define UNIVERSAL_UTILS_LOG_FLAG_EW     (1 << 5)

namespace cluster_utils {

class CLog
{
public:
    CLog();
    ~CLog();

    static void printLogEx(const char *level, const char *tag,
            const char *file, const char *func, int line,
            const char *log, ...);

    static void dump(const char *data, unsigned int length);

    static void setLogLevel(int logLevel);
    static int getLogLevel();
    static void setLogFlag(int flag);
    static int getLogFlag();

private:
    static pid_t getPid();
    static pid_t getTid();

    static int mLogLevel;
    static int mLogFlag;
};

} // namespace cluster_utils

#endif // CLOG_H
