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

#ifndef __CSYNC_H
#define __CSYNC_H
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 255
#endif

namespace universal_utils
{

class CMutexObject
{
public:
    CMutexObject() {
        pthread_mutexattr_init(&m_attr);
        pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_RECURSIVE);

        pthread_mutex_init(&m_lock, &m_attr);
    }

    virtual ~CMutexObject() {
        pthread_mutex_destroy(&m_lock);
    }

    bool lock (long milliSeconds = -1) {
        int err = 1;
        if (milliSeconds < 0) {
            err = pthread_mutex_lock(&m_lock);
        }else {
            timespec timeOut = {0,0};
            clock_gettime(CLOCK_MONOTONIC, &timeOut);
            timeOut.tv_sec += milliSeconds/1000;
            timeOut.tv_nsec += ((milliSeconds%1000)*1000000);
            err = pthread_mutex_timedlock(&m_lock, &timeOut);
        }

        return  (0 == err);
    }

    bool unlock () {
        int err = 1;
        err = pthread_mutex_unlock(&m_lock);

        return (0 == err);
    }
protected:
    pthread_mutex_t m_lock;
    pthread_mutexattr_t m_attr;
};

}

namespace universal_utils
{
class CRWLock {

public:
    CRWLock()
    {
        pthread_rwlock_init(&mLock, NULL);
    }
    ~CRWLock()
    {
        pthread_rwlock_destroy(&mLock);
    }

    int rlock()
    {
        return pthread_rwlock_rdlock(&mLock);
    }
    int wlock()
    {
        return pthread_rwlock_wrlock(&mLock);
    }
    int unlock()
    {
        return pthread_rwlock_unlock(&mLock);
    }

private:
    pthread_rwlock_t mLock;
};

}
#endif // CSYNC_H

