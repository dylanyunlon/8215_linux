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

#include "cmessagehandler.h"
#include <errno.h>
#include <time.h>
#include "clog.h"

namespace universal_utils
{

static const char *TAG = "CMessageHandler";
static const bool DBG = false;

CMessageHandler::CMessageHandler(const char *name) :
    CAutoThread(name),
    m_messageCond(NULL),
    m_messageCondIndex(0),
    m_enableDebug(false)
{
    m_messageCond = new CConditionLock();
    if (NULL == m_messageCond) {
        throw std::bad_alloc();
    }

    m_messageCond->lock();
    m_messageCondIndex = m_messageCond->newCondition();
    m_messageCond->unlock();
}

CMessageHandler::~CMessageHandler()
{
    if (m_messageCond) {
        m_messageCond->releaseCondition(m_messageCondIndex);
        delete m_messageCond;
        m_messageCond = NULL;
    }
}

int CMessageHandler::handleMessage(const CMessage &message)
{
    return 0;
}

int CMessageHandler::sendMessage(const CMessage& message)
{
    m_messageCond->lock();
    m_messages.push_back(message);
    m_messageCond->signal(m_messageCondIndex);
    m_messageCond->unlock();

    return 0;
}

int CMessageHandler::sendMessageAtFrontOfQueue(const CMessage& message)
{
    m_messageCond->lock();
    m_messages.push_front(message);
    m_messageCond->signal(m_messageCondIndex);
    m_messageCond->unlock();

    return 0;

}

int CMessageHandler::sendMessageDelayed(const CMessage &message, unsigned long delayMillis)
{
    CMessage delayMsg = message;

    delayMsg.when = nowMilliSecond() + delayMillis;

    m_messageCond->lock();
    m_delayMessages.push_back(delayMsg);
    m_messageCond->signal(m_messageCondIndex);
    m_messageCond->unlock();

    return 0;
}

void CMessageHandler::startProcess()
{
    if (DBG) UTILS_LOGD(TAG, "start message thread in");

    //m_messageCond->lock();
    CAutoThread::threadStart();
    //m_messageCond->unlock();

    if (DBG) UTILS_LOGD(TAG, "start message thread out, thread id = %d", getThreadId());
}

void CMessageHandler::stopProcess()
{
    if (DBG) UTILS_LOGD(TAG, "stop message thread in");

    m_messageCond->lock();

    if (false == isTerminated()) {
        m_messages.clear();
        m_delayMessages.clear();
        threadTerminated();
        m_messageCond->signal(m_messageCondIndex);
        m_messageCond->unlock();
        waitThreadComplete(5000);
    } else {
        m_messageCond->unlock();
    }

    if (DBG) UTILS_LOGD(TAG, "stop message thread out");

}

void CMessageHandler::flushMessage()
{
    m_messageCond->lock();
    m_messages.clear();
    m_messageCond->unlock();
}

void CMessageHandler::removeMessages(int what)
{
    m_messageCond->lock();

    std::list<CMessage>::iterator it = m_delayMessages.begin();
    while (it != m_delayMessages.end()) {
        if (it->what == what) {
            it = m_delayMessages.erase(it);
        } else {
            ++it;
        }
    }

    it = m_messages.begin();
    while (it != m_messages.end()) {
        if (it->what == what) {
            it = m_messages.erase(it);
        } else {
            ++it;
        }
    }

    m_messageCond->unlock();
}

bool CMessageHandler::hasMessages(int what)
{
    bool ret = false;

    m_messageCond->lock();

    std::list<CMessage>::iterator it = m_delayMessages.begin();
    while (it != m_delayMessages.end()) {
        if (it->what == what) {
            ret = true;
            break;
        } else {
            ++it;
        }
    }

    it = m_messages.begin();
    while (it != m_messages.end()) {
        if (it->what == what) {
            ret = true;
            break;
        } else {
            ++it;
        }
    }

    m_messageCond->unlock();

    return ret;
}

unsigned long CMessageHandler::threadRun()
{
    if (DBG) UTILS_LOGD(TAG, "message thread start");

    while (false == isTerminated()) {
       m_messageCond->lock();
        while (m_messages.empty()) {
            if (isTerminated()) {
                m_messageCond->unlock();
                if (DBG) UTILS_LOGD(TAG, "message thread get teminated signal");
                return 0;
            }

            m_messageCond->await(m_messageCondIndex, DELAYED_MESSAGE_CHECK_MILLIS);

            checkDelayedMessages();
        }

        CMessage message = m_messages.front();
        m_messages.pop_front();


        m_messageCond->unlock();

        handleMessage(message);
    }

    if (DBG) UTILS_LOGD(TAG, "message thread stopped");

    return 0;
}

bool CMessageHandler::threadStart()
{
    startProcess();

    return true;
}

bool CMessageHandler::threadStop()
{
    stopProcess();

    return true;
}

void CMessageHandler::checkDelayedMessages()
{
    long long now = nowMilliSecond();

    std::list<CMessage>::iterator it = m_delayMessages.begin();
    while (it != m_delayMessages.end()) {
        if (it->when < now) {
            m_messages.push_front(*it);
            it = m_delayMessages.erase(it);
        } else {
            ++it;
        }
    }
}

long long CMessageHandler::nowMilliSecond() const
{
    struct timespec res = {0, 0};
    long long now = 0;

    int ret = clock_gettime(CLOCK_MONOTONIC, &res);
    if (ret == -1) {
        UTILS_LOGE(TAG, "clock_gettime error:%s", strerror(errno));
        return -1;
    }

    now = (res.tv_sec * 1000);
    now += (res.tv_nsec / 1000000);

    return now;
}

}
