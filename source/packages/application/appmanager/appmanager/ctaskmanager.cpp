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

#include <unistd.h>
#include "ctaskmanager.h"
#include <pthread.h>
#include "globalbus.h"
#include "applog.h"

using namespace universal_utils;

static const char *TAG = "CTaskManager";

const unsigned long INFINITE = 0xffffffffL;

using std::list;

CTaskManager::CTaskManager()
    : m_taskRollTime(INFINITE)
    , m_isDoingTask(false)
{
    m_taskRunCondition = newCondition();
}

CTaskManager::~CTaskManager()
{
    releaseCondition(m_taskRunCondition);
}

bool CTaskManager::matchCmd(const CCmdTask &cmdTask)
{
    bool ret = false;

    if (CCmdTask::MAIN_FUNC_APP_ACTION == cmdTask.getMainFunc()) {
        if (cmdTask.getSubFunc() <= GlobalBus::ACTION_AV_REL
            && cmdTask.getSubFunc() % 2 == 1) {
            list<CCmdTask>::iterator it;
            unsigned char mainFunc = cmdTask.getMainFunc();
            unsigned char subFunc = cmdTask.getSubFunc();
            unsigned int data = cmdTask.getData();
            unsigned char appId = cmdTask.getAppID();

            for (it = m_taskList.begin(); it != m_taskList.end();) {
                if (it->getMainFunc() == mainFunc
                    && it->getSubFunc() == subFunc - 1
                    && it->getData() == data
                    && it->getAppID() == appId) {
                    LOGD(TAG, "match main(%d), sub(%d), data(%x)\n",
                        it->getMainFunc(), it->getSubFunc(), it->getData());
                    it = m_taskList.erase(it);
                } else
                    it++;
            }
        }
    } else if (CCmdTask::MAIN_FUNC_KEY == cmdTask.getMainFunc()) {
        if (cmdTask.getSubFunc() == CCmdTask::SUBFUNC_KEY_FRONT_VOLUME_INC
            || cmdTask.getSubFunc() == CCmdTask::SUBFUNC_KEY_FRONT_VOLUME_DEC) {
            list<CCmdTask>::iterator it;
            unsigned char mainFunc = cmdTask.getMainFunc();
            unsigned char subFunc = cmdTask.getSubFunc();
            int cmdcount = 0;

            for (it = m_taskList.begin(); it != m_taskList.end(); it++) {
                if (it->getMainFunc() == mainFunc
                    && it->getSubFunc() == subFunc) {
                    cmdcount++;

                    if (cmdcount > 3) {
                        LOGD(TAG, "too many mainfunc(%d) subfunc(%d)\n",
                            mainFunc, subFunc);
                        it = m_taskList.erase(it);
                        break;
                    }
                }
            }
        }
    } else {
        ret = false;
    }

    return ret;
}

bool CTaskManager::addTask(const CCmdTask &cmdTask)
{
    bool ret = false;

    if (lock()) {
        matchCmd(cmdTask);

        m_taskList.push_back(cmdTask);
        ret = true;

        LOGD(TAG, "addTask m_taskList.size = %d\n", m_taskList.size());
        if (m_taskList.size() == 1) {
            ret = signal(m_taskRunCondition);
            if (!ret) {
                LOGE(TAG, "signal fail\n");
            }
        }
        unlock();
    }

    return ret;
}

bool CTaskManager::clearTask()
{
    bool ret = false;

    if (lock()) {
        m_taskList.clear();
        ret = unlock();
    }

    return ret;
}

int CTaskManager::getCurTaskCount()
{
    int retCount = -1;

    if (lock()) {
        retCount = m_taskList.size();
        unlock();
    }

    return retCount;
}

bool CTaskManager::isDoingTask()
{
    bool ret = false;

    if (lock()) {
        ret = m_isDoingTask;
        unlock();
    }

    return ret;
}

bool CTaskManager::startTaskWatcher()
{
    bool ret = false;

    ret = stopTaskWatcher();
    if (!ret) {
        LOGE(TAG, "stopTaskWatcher fail\n");
    }

    ret = threadStart();
    if (!ret) {
        LOGE(TAG, "threadStart fail\n");
    }

    return ret;
}

bool CTaskManager::restartTaskWatcher()
{
    bool ret = false;

    if (isDoingTask()) {
        ret = forceTerminate();
        if (!ret) {
            LOGE(TAG, "forceTerminate fail\n");
        }

        m_taskList.clear();
        m_isDoingTask = false;

        if (ret) {
            ret = threadStart();
            if (!ret) {
                LOGE(TAG, "threadStart fail\n");
            }
        }
    } else {
        LOGD(TAG, "not doing task\n");
        ret = true;
    }

    return ret;
}

bool CTaskManager::stopTaskWatcher()
{
    bool ret = false;

    do {
        ret = threadTerminated();
        if (!ret) {
            LOGE(TAG, "threadTerminated fail\n");
            break;
        }

        ret = signalAll(m_taskRunCondition);
        if (!ret) {
            LOGE(TAG, "signalAll fail!\n");
            break;
        }

        ret = waitThreadComplete();
        if (!ret) {
            LOGE(TAG, "waitThreadComplete fail!\n");
            break;
        }

        m_taskList.clear();
        m_isDoingTask = false;
    } while(0);

    return ret;
}

bool CTaskManager::forceTerminateTaskWatcher()
{
    bool ret = false;

    ret = forceTerminate();
    if (!ret) {
        LOGE(TAG, "forceTerminate fail\n");
    }

    m_taskList.clear();
    m_isDoingTask = false;

    return ret;
}

bool CTaskManager::waitTaskWatcherFinish(unsigned long milliSeconds)
{
    bool ret = false;

    ret = waitThreadComplete(milliSeconds);

    return ret;
}

bool CTaskManager::cleanupLock()
{
    bool ret = unlock();
    LOGD(TAG, "cleanupLock\n");

    return ret;
}

void CTaskManager::clearAwait(void *arg)
{
    CTaskManager *target = (CTaskManager *)arg;
    target->cleanupLock();
}

unsigned long CTaskManager::threadRun()
{
    bool taskRet = false;
    CCmdTask *actionTask = NULL;

    // add by atc6129
    if (isTerminated() == true) {
        LOGW(TAG, "the Processing Threads is teminated\n");
    }
    while (!isTerminated()) {
        if (lock()) {
            if (m_taskList.size() == 0) {
                m_isDoingTask = false;

                pthread_cleanup_push(clearAwait, this);
                //LOGD(TAG, "about to await\n");
                await(m_taskRunCondition); //if list size is 0, block thread.
                //LOGD(TAG, "await done\n");
                pthread_cleanup_pop(0);
            }

            LOGD(TAG, "threadRun m_taskList.size = %d\n", m_taskList.size());
            if (m_taskList.size() > 0) {
                actionTask = new (std::nothrow)CCmdTask(m_taskList.front());
                if (NULL == actionTask) {
                    LOGE(TAG, "new CCmdTask fail\n");
                } else {
                    m_taskList.pop_front();
                    m_isDoingTask = true;
                }
            }
            unlock();
        }

        if (actionTask != NULL) {
            LOGD(TAG, "doTask begin\n");
            taskRet = doTask(*actionTask);
            LOGD(TAG, "doTask done\n");

            if (taskRet) {
                //task finished ok!
            } else {
                //task finished fail!
            }

            SAFE_DELETE(actionTask);
        } else {
            continue;
        }
    }

    //taskManager thread finished!
    return 0;
}

