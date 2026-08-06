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

#include "cautothread.h"
#include <execinfo.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>


namespace universal_utils
{

static const char* const autoThreadtag = "CAutoThread";
static const bool DBG = false;

CAutoThread::CAutoThread(const char *name, unsigned long threadTerminateTime)
    : m_terminated(true)
    , m_threadId(0)
    , m_exitCode(0)
    , m_threadTerminatedTime(threadTerminateTime)
{
    unsigned int nameLen = 0;

    if (name) {
        nameLen = strlen(name);
        nameLen = (nameLen >= MAX_TRHEAD_NAME_LENGTH) ? (MAX_TRHEAD_NAME_LENGTH - 1) : nameLen;
        strncpy(m_name, name, nameLen);
    }

    m_name[nameLen] = 0;
}

CAutoThread::~CAutoThread()
{
    if (DBG) UTILS_LOGD(autoThreadtag, "~CAutoThread leave\n");
}

bool CAutoThread::threadStart()
{
    int err = -1;

    threadStop();
    if (0 == m_threadId) {
        m_terminated = false;
        err = pthread_create(&m_threadId, NULL, CAutoThread::threadProc, (void *)this);
        if (0 == err) {
            if (DBG) UTILS_LOGD(autoThreadtag, "[%s]thread start thread id = %u", m_name, m_threadId);
            pthread_setname_np(m_threadId, m_name);
        } else {
            if (DBG) UTILS_LOGE("", "pthread_create fail: %s\n", strerror(err));
        }
    }

    return (0 == err);
}

bool CAutoThread::threadStop()
{
    if (DBG) UTILS_LOGD(autoThreadtag, "[%s]threadStop enter\n", m_name);
    threadTerminated();
    waitThreadComplete(m_threadTerminatedTime);
    if (DBG) UTILS_LOGD(autoThreadtag, "[%s]threadStop leave\n", m_name);

    return (0 == m_threadId);
}

bool CAutoThread::setPriority(int priority)
{
    int ret = -1;

    if (priority >= -20 && priority <= 19) {
        ret = setpriority(PRIO_PROCESS, 0, priority);
    }

    return (0 == ret);
}

bool CAutoThread::threadTerminated()
{
    m_terminated = true;

    return true;
}

bool CAutoThread::waitThreadComplete(unsigned long milliSeconds)
{
    int err = 0;

    if (0 != m_threadId) {
        if (DBG) UTILS_LOGD("", "waitThreadComplete tid(%d) enter.", m_threadId);
        err = pthread_join(m_threadId, NULL);
        if (0 != err) {
            UTILS_LOGE("", "pthread_join fail:%s.", strerror(err));
        }
        if (DBG) UTILS_LOGD("", "waitThreadComplete tid(%d) leave.", m_threadId);

        if (0 == err) {
            m_threadId = 0;
        }
    } else {
        //UTILS_LOGD("", "m_threadId = 0\n");
    }

    return (0 == err);
}

bool CAutoThread::forceTerminate()
{
    bool ret = false;

    threadTerminated();
    ret = waitThreadComplete();

    return ret;
}

bool CAutoThread::isTerminated() const {
    return m_terminated;
}

bool CAutoThread::getExitCodeThread(void *exitCode)
{
    if (NULL != exitCode) {
        *((unsigned long*)exitCode) = m_exitCode;
    }

    return true;
}

pthread_t CAutoThread::getThreadId() const
{
    return m_threadId;
}

void* CAutoThread::threadProc(void *arg)
{
    CAutoThread* threadPtr = ( CAutoThread* ) arg;
    threadPtr->m_exitCode = threadPtr->threadRun();
    return (void *)&threadPtr->m_exitCode;
}

}



