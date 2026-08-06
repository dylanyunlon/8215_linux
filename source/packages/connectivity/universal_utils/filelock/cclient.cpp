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
#include <signal.h>

#include "cclient.h"
//#include "log.h"

namespace universal_utils {

const static char *TAG = "CClient";

int CClient::m_monitorSignal = SIGUSR2;

CClient::CClient() :
    m_connection(NULL)
    , m_clientDisconnectFromServerDone(false)
    , m_threadStarted(false)
{

}

CClient::~CClient()
{
    if (!m_clientDisconnectFromServerDone) {
        disconnect();
    }
}

int CClient::connectServiceManager(const std::string &mangerName, unsigned long milliseconds)
{
    int pid = -1;

    m_managerLock.deinitLock();
    m_managerLock.initLock(mangerName);

    pid = waitServerPid(milliseconds);

    UTILS_LOGI(TAG, "connect to manger %s, pid = %d", mangerName.c_str(), pid);

    return pid;
}

bool CClient::connectService(const std::string &serverName, CServiceConnection *connection, unsigned long milliseconds)
{
    bool ready = false;

    m_serverLock.deinitLock();
    m_serverLock.initLock(serverName);

    ready = waitServerReady(milliseconds);

    m_connection = connection;

    UTILS_LOGI(TAG, "connect to server %s, ready = %s", serverName.c_str(), ready ? "true": "false");

    if (ready) {

        if (m_connection) {
            m_connection = connection;
            m_connection->onServiceConnected();
        }

        if (isTerminated()) {
            threadStart();
            m_threadStarted = true;
        }
    }

    return ready;
}

void CClient::disconnect()
{
    threadTerminated();
    if (m_threadStarted) {
        if (getThreadId() != 0) {
            pthread_kill(getThreadId(), m_monitorSignal);
        }
        m_threadStarted = false;
    }
    waitThreadComplete();
    m_serverLock.deinitLock();
    m_clientDisconnectFromServerDone = true;
    UTILS_LOGI(TAG, "client disconnect done");

}

unsigned int CClient::getServerPid()
{
    return m_serverLock.getLockerPid();
}

unsigned long CClient::threadRun()
{
    bool lockedStatus = false;
    bool isNotifyClientNeeded = true;

    UTILS_LOGD(TAG, "server state monitor start");
    if (installSignal(m_monitorSignal, signal_handler) < 0) {
        UTILS_LOGE(TAG, "installSignal(%d) with fail", m_monitorSignal);
    }

    while (false == isTerminated()) {
        lockedStatus = m_serverLock.lockForWriteWait();
        if (!lockedStatus && m_serverLock.getErrno() == EINTR) {
            // The lock was interruped by signal, we don't need notify client process
            isNotifyClientNeeded = false;
            UTILS_LOGW(TAG, "lock was interruped by signal");
        }
        m_serverLock.unlock();

        if (m_connection && isNotifyClientNeeded) {
            UTILS_LOGD(TAG,"server quit");
            m_connection->onServiceDisConnected();
        }
    }

    m_threadStarted = false;
    UTILS_LOGW(TAG, "server state monitor stop");

    return 0;
}

void CClient::wait(unsigned long milliseconds)
{
    struct timeval timeout;

    timeout.tv_usec = (milliseconds % 1000) * 1000;
    timeout.tv_sec = milliseconds / 1000;

    select(0, NULL, NULL, NULL, &timeout);
}

bool CClient::waitServerReady(unsigned long milliseconds)
{
    bool locked = false;
    int times = 0;
    int timeout = 0;

    if (milliseconds > 1000) {
        times = (milliseconds + 1000) / 1000;
        timeout = 1000;
    } else {
        times = 1;
        timeout = milliseconds;
    }

    while ((false == (locked = m_serverLock.isLocked())) && (times > 0)) {
        wait(timeout);
        times--;
    }

    return m_serverLock.isLocked();
}

int CClient::waitServerPid(unsigned long milliseconds)
{
    bool locked = false;
    int times = 0;
    int timeout = 0;

    if (milliseconds > 1000) {
        times = (milliseconds + 1000) / 1000;
        timeout = 1000;
    } else {
        times = 1;
        timeout = milliseconds;
    }

    while ((false == (locked = m_managerLock.isLocked())) && (times > 0)) {
        wait(timeout);
        times--;
    }

    return m_managerLock.getLockerPid();
}

int CClient::installSignal(int signum, void (*handler)(int))
{
    struct sigaction act = {0};
    sigset_t bset;

    sigemptyset(&bset);
    act.sa_mask = bset;
    act.sa_handler = handler;
    act.sa_restorer = NULL;
    act.sa_flags = 0;

    return sigaction(signum, &act, NULL);
}

void CClient::signal_handler(int sigNum)
{
    if (sigNum == m_monitorSignal) {
        UTILS_LOGI(TAG, "sig recv :%d, flock should unblocked", sigNum);
    }
}

}
