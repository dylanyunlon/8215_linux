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

#include "cconditionlock.h"
#include <algorithm>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <ctime>
#include "clog.h"

namespace universal_utils {
const static char* TAG = "ConditionLock";
CConditionLock::CConditionLock()
    : CMutexObject()
    , m_idPool(1)
{

}

CConditionLock::~CConditionLock()
{
    int err = -1;

    for (std::list<CondHandle>::iterator iter = m_condList.begin(); iter != m_condList.end(); ) {
        err = pthread_cond_destroy(&(iter->cond));
        if (0 == err) {
            iter = m_condList.erase(iter);
        } else {
            ++iter;
            UTILS_LOGE(TAG, "pthread_cond_destroy fail :%s\n", strerror(err));
        }
    }

    return;
}

int CConditionLock::newCondition() {
    int ret = -1;// -1 is fail for newCondition
    CondHandle condHandle;
    int err = 1;
    pthread_condattr_t pthreadAttr;

    pthread_condattr_init(&pthreadAttr);
    do {
        err = pthread_condattr_setclock(&pthreadAttr, CLOCK_MONOTONIC);
        if (err != 0) {
            UTILS_LOGE(TAG, "pthread_condattr_setclock error: %s\n", strerror(err));
            ret = -1;
            break;
        }

        err = pthread_cond_init(&(condHandle.cond), &pthreadAttr);
        if (0 == err) {
            condHandle.id = m_idPool;
            m_idPool++;
            m_condList.push_back(condHandle);
            ret = condHandle.id;
        } else {
            UTILS_LOGE(TAG, "pthread_cond_init fail: %s\n", strerror(err));
            ret = -1;
            break;
        }
    } while (0);
    pthread_condattr_destroy(&pthreadAttr);

    return ret;
}


bool CConditionLock::releaseCondition (int id) {
    int err = 1;
    std::list<CondHandle>::iterator iter = std::find(m_condList.begin(), m_condList.end(), id);
    if (iter != m_condList.end()) {
        err = pthread_cond_destroy(&(iter->cond));
        if (0 == err) {
            m_condList.erase(iter);
        }
    }

    return (0 == err);
}
bool CConditionLock::await (int id) {
    int err = -1;
    std::list<CondHandle>::iterator iter = std::find(m_condList.begin(), m_condList.end(), id);
    if (iter != m_condList.end()) {
        err = pthread_cond_wait(&(iter->cond), &m_lock);
        if (0 != err) {
            UTILS_LOGE(TAG, "pthread_cond_wait fail: %s\n", strerror(err));
        }
    }

    return (0 == err);
}
bool CConditionLock::await (int id, long milliSeconds) {
    const unsigned long INFINITE = 0xFFFFFFFF;

    const int MILLISECONDS_PER_SECOND = 1000;
    const int MICROSECONDS_PER_MILLISECOND = 1000;
    const int NANOSECONDS_PER_SECOND = 1000000000;
    const int NANOSECONDS_PER_MILLISECOND = 1000000;

    int err = -1;
    std::list<CondHandle>::iterator iter = std::find(m_condList.begin(), m_condList.end(), id);
    if (iter != m_condList.end()) {
        if (milliSeconds != INFINITE && milliSeconds) {
            timespec timeOut = {0,0};
            int ret = clock_gettime(CLOCK_MONOTONIC, &timeOut);
            if (ret == -1) {
                UTILS_LOGE(TAG, "gettime err:%s(%d)", strerror(errno), errno);
                return false;
            }
            timeOut.tv_sec += milliSeconds / MILLISECONDS_PER_SECOND;
            timeOut.tv_nsec += (milliSeconds - ((milliSeconds / MILLISECONDS_PER_SECOND) *
                MILLISECONDS_PER_SECOND)) * NANOSECONDS_PER_MILLISECOND;
            if (timeOut.tv_nsec >= NANOSECONDS_PER_SECOND) {
                timeOut.tv_sec += timeOut.tv_nsec / NANOSECONDS_PER_SECOND;
                timeOut.tv_nsec %= NANOSECONDS_PER_SECOND;
            }
            err = pthread_cond_timedwait(&(iter->cond), &m_lock, &timeOut);
        } else {
            err = pthread_cond_wait(&(iter->cond), &m_lock);
        }

    } else {
        UTILS_LOGE(TAG, "await not found in list");
    }
    return (0 == err);
}

bool CConditionLock::signal (int id) {
    int err = -1;
    std::list<CondHandle>::iterator iter = std::find(m_condList.begin(), m_condList.end(), id);
    if (iter != m_condList.end()) {
        err = pthread_cond_signal(&(iter->cond));
        if (0 != err) {
            UTILS_LOGE(TAG, "pthread_cond_signal fail: %s\n", strerror(err));
        }
    } else {
        UTILS_LOGE(TAG, "can't find CondHandle %d\n", id);
    }

    return (0 == err);
}
bool CConditionLock::signalAll (int id) {
    int err = -1;
    std::list<CondHandle>::iterator iter = std::find(m_condList.begin(), m_condList.end(), id);
    if (iter != m_condList.end()) {
        err = pthread_cond_broadcast(&(iter->cond));
        if (err > 0) {
            UTILS_LOGE(TAG, "pthread_cond_broadcast fail: %s\n", strerror(err));
        }
    } else {
        UTILS_LOGE(TAG, "can't find CondHandle %d\n", id);
    }

    return (0 == err);
}
}
