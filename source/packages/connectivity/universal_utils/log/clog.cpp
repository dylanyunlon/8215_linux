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

#include "clog.h"
#include <iostream>
#include <cstdarg>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/syscall.h>
#include <sys/types.h>

#define CLOG_LEN    2048

namespace universal_utils {

int CLog::mLogLevel = UNIVERSAL_UTILE_LOG_LEVEL_INFO;
int CLog::mLogFlag = UNIVERSAL_UTILS_LOG_FLAG_PID
    | UNIVERSAL_UTILS_LOG_FLAG_TID;

CLog::CLog()
{

}

CLog::~CLog()
{

}

void CLog::printLogEx(const char *level, const char *tag,
        const char *file, const char *func, int line,
        const char *fmt, ...)
{
    /* use local variable for lock-free
     * don't need memset before snprintf
     */
    char buf[CLOG_LEN] = {0};
    int n = 0;
    int syslogLevel = 0;
    va_list args;

    if (nullptr == level || nullptr == fmt) {
        return;
    }
    switch (*level) {
    case 'E':
        if (mLogLevel < UNIVERSAL_UTILE_LOG_LEVEL_ERROR) {
            return;
        }
        syslogLevel = LOG_ERR;
        break;
    case 'W':
        if (mLogLevel < UNIVERSAL_UTILE_LOG_LEVEL_WARNING) {
            return;
        }
        syslogLevel = LOG_WARNING;
        break;
    case 'I':
        if (mLogLevel < UNIVERSAL_UTILE_LOG_LEVEL_INFO) {
            return;
        }
        syslogLevel = LOG_INFO;
        break;
    case 'D':
        if (mLogLevel < UNIVERSAL_UTILE_LOG_LEVEL_DEBUG) {
            return;
        }
        syslogLevel = LOG_DEBUG;
        break;
    default:
        return;
    }
    n += snprintf(buf, CLOG_LEN - n, "[%s]", level);
    if (mLogFlag & UNIVERSAL_UTILS_LOG_FLAG_PID) {
        n += snprintf(buf + n, CLOG_LEN - n, "[%d]", getPid());
    }
    if (mLogFlag & UNIVERSAL_UTILS_LOG_FLAG_TID) {
        n += snprintf(buf + n, CLOG_LEN - n, "[%d]", getTid());
    }
    if (tag) {
        n += snprintf(buf + n, CLOG_LEN - n, "[%s] ", tag);
    } else {
        n += snprintf(buf + n, CLOG_LEN - n, " ");
    }
    if (mLogFlag & UNIVERSAL_UTILS_LOG_FLAG_FILE) {
        n += snprintf(buf + n, CLOG_LEN - n, "%s:", file);
    }
    if (mLogFlag & UNIVERSAL_UTILS_LOG_FLAG_FUNC
            || (mLogFlag & UNIVERSAL_UTILS_LOG_FLAG_EW
                && (*level == 'E' || *level == 'W'))) {
        n += snprintf(buf + n, CLOG_LEN - n, "%s():", func);
    }
    if (mLogFlag & UNIVERSAL_UTILS_LOG_FLAG_LINE
            || (mLogFlag & UNIVERSAL_UTILS_LOG_FLAG_EW
                && (*level == 'E' || *level == 'W'))) {
        n += snprintf(buf + n, CLOG_LEN - n, "%d ", line);
    }

    va_start(args, fmt);
    n += vsnprintf(buf + n, CLOG_LEN - n, fmt, args);
    va_end(args);

#if USE_SYSTEM_LOG
    syslog(syslogLevel, "%s", buf);
#else
    printf("%s\n", buf);
#endif
}

void CLog::dump(const char *data, unsigned int length)
{
    char buf[CLOG_LEN] = {0};
    int n = 0;
    unsigned int i = 0;

    if (mLogLevel < UNIVERSAL_UTILE_LOG_LEVEL_DEBUG) {
        return;
    }
    n += snprintf(buf + n, CLOG_LEN - n, "[D][dump]\n");
    while (i < length) {
        n += snprintf(buf + n, CLOG_LEN - n, "0x%x ", data[i++]);
        if ((i % 16) == 0 && n < CLOG_LEN - 1) {
            n += snprintf(buf + n, CLOG_LEN - n, "\n");
        }
        if (n >= CLOG_LEN) {
            break;
        }
    }
#if USE_SYSTEM_LOG
    syslog(LOG_DEBUG, "%s", buf);
#else
    printf("%s\n", buf);
#endif
}

void CLog::setLogLevel(int logLevel)
{
    mLogLevel = logLevel;
}

int CLog::getLogLevel()
{
    return mLogLevel;
}

void CLog::setLogFlag(int flag)
{
    mLogFlag = flag;
}

int CLog::getLogFlag()
{
    return mLogFlag;
}

pid_t CLog::getPid()
{
    static pid_t pid = -1;

    if (-1 == pid) {
        pid = getpid();
    }

    return pid;
}

pid_t CLog::getTid()
{
    pid_t tid = syscall(__NR_gettid);

    return tid;
}

} // namespace universal_utils
