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
 
#include "bluetoothfilesync.h"
#include "applog.h"
#include <QThread>

using namespace universal_utils;

#define SLEEP_TIME_TO_WAIT              10*1000
#define TAG                             "CBluetoothFileSync"

CBluetoothFileSync::CBluetoothFileSync()
    : CAutoThread()
    , m_fileSyncTrigCond(0)
    , m_mutextSyncLock(NULL)

{

    m_mutextSyncLock = new CMutexObject;
    m_fileSyncTrigCond = m_conditionLock.newCondition();
    this->threadStart();
    QThread::usleep(SLEEP_TIME_TO_WAIT);

}

CBluetoothFileSync::~CBluetoothFileSync()
{
    LOGD(TAG, "destructor\n");
    bool ret = false;

    do {
        ret = threadTerminated();
        if (!ret) {
            LOGE(TAG, "threadTerminated fail\n");
            break;
        }
        ret = m_conditionLock.signalAll(m_fileSyncTrigCond);
        if (!ret) {
            LOGE(TAG, "signalAll fail!\n");
            break;
        }
        ret = waitThreadComplete();
        if (!ret) {
            LOGE(TAG, "waitThreadComplete fail!\n");
            break;
        }
    } while(0);

    if (NULL != m_mutextSyncLock) {
        delete m_mutextSyncLock;
        m_mutextSyncLock = NULL;
    }

    m_conditionLock.releaseCondition(m_fileSyncTrigCond);

}

bool CBluetoothFileSync::startSync()
{
    LOGD(TAG, "startSync\n");

    bool ret = false;

    if (NULL == m_mutextSyncLock) {
        LOGD(TAG, "m_mutextSyncLock is empty\n");
    } else {
        m_conditionLock.lock();
        m_conditionLock.signal(m_fileSyncTrigCond);
        m_conditionLock.unlock();
        ret = true;
    }

    return ret;
}

unsigned long CBluetoothFileSync::threadRun()
{
    bool ret = false;
    
    while (false == isTerminated()) {
        m_conditionLock.lock();
        m_conditionLock.await(m_fileSyncTrigCond);
        m_conditionLock.unlock();

        if(NULL != m_mutextSyncLock){
            m_mutextSyncLock->lock();
            sync();
            m_mutextSyncLock->unlock();
            ret = true;
        }
    }
    
    return ret;
}


