/*
* Copyright (c) 2016 AutoChips Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
 
#include "cautoudpsocket.h"
#include "apptype.h"
#include "applog.h"
using namespace universal_utils;

static const char TAG[] = "CAutoUDPSocket";

CAutoUDPSocket::CAutoUDPSocket()
    : m_threadProc(NULL)
{
}

CAutoUDPSocket::~CAutoUDPSocket()
{
    SAFE_DELETE(m_threadProc);
}

int CAutoUDPSocket::startService()
{
    int ret = -1;
    bool result = false;

    do {
        m_threadProc = new CThreadProc<CAutoUDPSocket>;
        if (NULL == m_threadProc) {
            LOGE(TAG, "new CThreadProc<CAutoUDPSocket> fail\n");
            ret = -1;
            break;
        }

        result = m_threadProc->init(this, &CAutoUDPSocket::recveiveProc);
        if (!result) {
            LOGE(TAG, "m_threadProc->init fail\n");
            ret = -1;
            break;
        }

        result = m_threadProc->threadStart();
        if (!result) {
            LOGE(TAG, "m_threadProc->threadStart fail\n");
            ret = -1;
            break;
        }

        result = m_threadProc->triggerProc();
        if (!result) {
            LOGE(TAG, "m_threadProc->triggerProc fail\n");
            ret = -1;
            break;
        }

        ret = 0;
    } while(0);

    return ret;
}

int CAutoUDPSocket::stopService()
{
    int ret = -1;

    if (NULL != m_threadProc)
        ret = m_threadProc->forceTerminate() ? 0 : -1;

    return ret;
}

bool CAutoUDPSocket::recveiveProc() //running on thread;
{
    int res = -1;

    res = select();
    if (res > 0) {
        this->onReceive();
        m_threadProc->triggerProc();
    }
    else if (res == 0) {
        //time out.
        LOGE(TAG, "select timeout\n");
        m_threadProc->triggerProc();
    }
    else {
        LOGE(TAG, "select fail\n");
    }

    return true;
}

