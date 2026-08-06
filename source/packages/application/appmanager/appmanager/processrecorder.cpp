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

#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <algorithm>
#include "applog.h"
#include "processrecorder.h"

using namespace universal_utils;

static const char TAG[] = "ProcessRecord";

// Process mem func
int Process::setFlag(ProcessFlag *pFlag)
{
    int ret = -1;
    list<ProcessFlag *>::iterator ppFlag;

    ppFlag = find(m_flagList.begin(), m_flagList.end(), pFlag);
    if (ppFlag == m_flagList.end()) {
        if (0 == pFlag->setProcess(this)) {
            m_flagList.push_back(pFlag);
            ret = 0;
        }
    } else {
        LOGE(TAG, "Process %d already has flag %s\n", m_ID, pFlag->getName().c_str());
    }

    return ret;
}

int Process::rmFlag(ProcessFlag *pFlag)
{
    int ret = -1;
    list<ProcessFlag *>::iterator ppFlag;

    dumpFlag();

    ppFlag = find(m_flagList.begin(), m_flagList.end(), pFlag);
    if (ppFlag != m_flagList.end()) {
        m_flagList.erase(ppFlag);
        pFlag->rmProcess(this);
        ret = 0;
    }

    return ret;
}

int Process::rmFlag()
{
    int ret = m_flagList.empty() ? 0 : -1;
    list<ProcessFlag *>::iterator pFlag;

    while (!m_flagList.empty()) {
        ret = rmFlag(*m_flagList.begin());
    }

    return ret;
}

void Process::dumpFlag() const
{
    list<ProcessFlag *>::const_iterator pFlag;

    for (pFlag = m_flagList.begin(); pFlag != m_flagList.end(); pFlag++) {
        LOGD(TAG, "Process: %d has flag %s\n", m_ID, (*pFlag)->getName().c_str());
    }
}

// ProcessFlag mem func
int ProcessFlag::setProcess(Process *pPs)
{
    int ret = -1;
    list<Process *>::iterator ppProcess;

    ppProcess = find(m_processList.begin(), m_processList.end(), pPs);
    if (ppProcess == m_processList.end()) {
        if (m_processList.size() == m_maxCount) {
            dumpProcess();
            Process *outProcess = *(m_processList.begin());
            m_processList.pop_front();
            outProcess->rmFlag(this);
        }
        m_processList.push_back(pPs);
        ret = 0;
    } else {
        m_processList.erase(ppProcess);
        m_processList.push_back(pPs);
        ret = 0;
    }

    return ret;
}

int ProcessFlag::rmProcess(const Process *pPs)
{
    int ret = -1;
    list<Process *>::iterator ppProcess;

    ppProcess = find(m_processList.begin(), m_processList.end(), pPs);
    if (ppProcess == m_processList.end()) {
        LOGE(TAG, "Process %d doesn't has flag %s\n", pPs->getProcessID(), m_name.c_str());
    } else {
        m_processList.erase(ppProcess);
        ret = 0;
    }

    return ret;
}

int ProcessFlag::rmProcess()
{
    m_processList.clear();

    return 0;
}

void ProcessFlag::dumpProcess() const
{
    list<Process *>::const_iterator ppProcess;

    for (ppProcess = m_processList.begin(); ppProcess != m_processList.end(); ppProcess++) {
        LOGD(TAG, "ProcessFlag %s is hold by process %d\n", m_name.c_str(), (*ppProcess)->getProcessID());
    }
}

typedef enum {
    MSG_INVALID = 0,

    MSG_NEWFLAG,
    MSG_SETPROCESSFLAG,
    MSG_RMPROCESSFLAG,
    MSG_ADDPROCESS,
    MSG_RMPROCESS,

    MSG_GAP,

    MSG_NEWFLAG_ACK,
    MSG_SETPROCESSFLAG_ACK,
    MSG_RMPROCESSFLAG_ACK,
    MSG_ADDPROCESS_ACK,
    MSG_RMPROCESS_ACK,

    MSG_MAX,
} E_MSG;

// ProcessRecordClient mem func
ProcessRecordClient::ProcessRecordClient(const string &pathname, int proj_id)
    : m_msgQ(pathname, proj_id)
{
}

int ProcessRecordClient::getRet(long type)
{
    int result = m_msgQ.recvMsg(type + MSG_NEWFLAG_ACK - MSG_NEWFLAG);

    return result;
}

int ProcessRecordClient::newFlag(const string &name, unsigned int maxCount)
{
    int result = -1;
    long type = MSG_NEWFLAG;
    Message msg;
    msg.m_type = type;

    m_msgQ.sendMsg(&msg, 0, 0);
    m_msgQ.sendMsg(type, name);
    m_msgQ.sendMsg(type, maxCount);

    result = getRet(type);

    return result;
}

int ProcessRecordClient::setProcessFlag(int id, const string &name)
{
    //int ret = -1;
    long type = MSG_SETPROCESSFLAG;
    Message msg;
    msg.m_type = type;

    m_msgQ.sendMsg(&msg, 0, 0);
    m_msgQ.sendMsg(type, id);
    m_msgQ.sendMsg(type, name);

    return getRet(type);
}

int ProcessRecordClient::rmProcessFlag(int id, const string &name)
{
    //int ret = -1;
    long type = MSG_RMPROCESSFLAG;
    Message msg;
    msg.m_type = type;

    m_msgQ.sendMsg(&msg, 0, 0);
    m_msgQ.sendMsg(type, id);
    m_msgQ.sendMsg(type, name);

    return getRet(type);
}

int ProcessRecordClient::addProcess(int id)
{
    //int ret = -1;
    long type = MSG_ADDPROCESS;
    Message msg;
    msg.m_type = type;

    m_msgQ.sendMsg(&msg, 0, 0);
    m_msgQ.sendMsg(type, id);

    return getRet(type);
}

int ProcessRecordClient::rmProcess(int id)
{
    //int ret = -1;
    long type = MSG_RMPROCESS;
    Message msg;
    msg.m_type = type;

    m_msgQ.sendMsg(&msg, 0, 0);
    m_msgQ.sendMsg(type, id);

    return getRet(type);
}

// ProcessRecordService mem func
ProcessRecordService::ProcessRecordService(const std::string &pathname, int proj_id)
    : m_msgQ(pathname, proj_id, MessageQueue::MSG_SERVICE)
{
    m_mainProc.init(this, &ProcessRecordService::mainProcess);
    m_mainProc.threadStart();

    m_msgProc.init(this, &ProcessRecordService::msgProcess);
    m_msgProc.threadStart();
}

ProcessRecordService::~ProcessRecordService()
{
    m_msgProc.threadStop();
    m_mainProc.threadStop();

    list<Process *>::iterator ppProcess;
    while (!m_processList.empty()) {
        ppProcess = m_processList.begin();
        delete (*ppProcess);
        m_processList.pop_front();
    }

    list<ProcessFlag *>::iterator ppFlag;
    while (!m_flagList.empty()) {
        ppFlag = m_flagList.begin();
        delete (*ppFlag);
        m_flagList.pop_front();
    }
}

int ProcessRecordService::getProcessCount()
{
    m_lock.lock();
    int ret = m_processList.size();
    m_lock.unlock();

    return ret;
}

int ProcessRecordService::newFlag(const std::string &name, unsigned int maxCount)
{
    int ret = -1;
    list<ProcessFlag *>::iterator ppFlag;

    LOGD(TAG, "%s: name %s, maxCount %d\n", __func__, name.c_str(), maxCount);

    m_lock.lock();

    for (ppFlag = m_flagList.begin(); ppFlag != m_flagList.end(); ppFlag++) {
        if ((*ppFlag)->getName() == name)
            break;
    }

    if (ppFlag == m_flagList.end()) {
        m_flagList.push_back(new ProcessFlag(name, maxCount));
        ret = 0;
    } else {
        LOGE(TAG, "already has flag %s\n", name.c_str());
        ret = -1;
    }

    m_lock.unlock();

    return ret;
}

int ProcessRecordService::setProcessFlag(int id, const string &name)
{
    int ret = -1;
    list<Process *>::iterator ppProcess;
    list<ProcessFlag *>::iterator ppFlag;

    LOGD(TAG, "%s: process %d, flag %s\n", __func__, id, name.c_str());

    m_lock.lock();

    do {
        for (ppProcess = m_processList.begin(); ppProcess != m_processList.end(); ppProcess++) {
            if ((*ppProcess)->getProcessID() == id)
                break;
        }
        if (ppProcess == m_processList.end()) {
            LOGE(TAG, "no Process: %d\n", id);
            ret = -1;
            break;
        }

        for (ppFlag = m_flagList.begin(); ppFlag != m_flagList.end(); ppFlag++) {
            if ((*ppFlag)->getName() == name)
                break;
        }
        if (ppFlag == m_flagList.end()) {
            LOGE(TAG, "no ProcessFlag: %s\n", name.c_str());
            ret = -1;
            break;
        }

        ret = (*ppProcess)->setFlag(*ppFlag);
    } while (0);

    m_lock.unlock();

    return ret;
}

int ProcessRecordService::rmProcessFlag(int id, const string &name)
{
    int ret = -1;
    list<Process *>::iterator ppProcess;
    list<ProcessFlag *>::iterator ppFlag;

    LOGD(TAG, "%s: process %d, flag %s\n", __func__, id, name.c_str());

    m_lock.lock();

    do {
        for (ppProcess = m_processList.begin(); ppProcess != m_processList.end(); ppProcess++) {
            if ((*ppProcess)->getProcessID() == id)
                break;
        }
        if (ppProcess == m_processList.end()) {
            LOGE(TAG, "no Process: %d\n", id);
            ret = -1;
            break;
        }

        for (ppFlag = m_flagList.begin(); ppFlag != m_flagList.end(); ppFlag++) {
            if ((*ppFlag)->getName() == name)
                break;
        }
        if (ppFlag == m_flagList.end()) {
            LOGE(TAG, "no ProcessFlag: %s\n", name.c_str());
            ret = -1;
            break;
        }

        ret = (*ppProcess)->rmFlag(*ppFlag);
    } while (0);

    m_lock.unlock();

    return ret;
}

int ProcessRecordService::addProcess(int id)
{
    int ret = -1;
    list<Process *>::iterator ppProcess;

    LOGD(TAG, "%s %d", __func__, id);

    m_lock.lock();

    for (ppProcess = m_processList.begin(); ppProcess != m_processList.end(); ppProcess++) {
        if ((*ppProcess)->getProcessID() == id)
            break;
    }

    if (ppProcess == m_processList.end()) {
        //LOGD(TAG, "new process %d\n", id);
        m_processList.push_back(new Process(id));
        ret = 0;
    } else {
        m_processList.push_back(*ppProcess);
        m_processList.erase(ppProcess);
        ret = 0;
    }

    m_lock.unlock();

    return ret;
}

int ProcessRecordService::rmProcess(int id)
{
    int ret = -1;
    list<Process *>::iterator ppProcess;

    LOGD(TAG, "%s %d", __func__, id);

    m_lock.lock();

    for (ppProcess = m_processList.begin(); ppProcess != m_processList.end(); ppProcess++) {
        if ((*ppProcess)->getProcessID() == id)
            break;
    }

    if (ppProcess == m_processList.end()) {
        LOGE(TAG, "can't find process %d\n", id);
        ret = -1;
    } else {
        ret = (*ppProcess)->rmFlag();
        delete (*ppProcess);
        m_processList.erase(ppProcess);
    }

    m_lock.unlock();

    return ret;
}

bool ProcessRecordService::mainProcess()
{
    int ret = -1;

    sleep(2);

    if (checkCondition()) {
        LOGI(TAG, "condition satified\n");
        m_lock.lock();

        list<Process *>::iterator it;
        for (it = m_processList.begin(); it != m_processList.end(); it++) {
            LOGD(TAG, "process %d, flagCount %d\n", (*it)->getProcessID(), (*it)->getFlagCount());
            if (0 == (*it)->getFlagCount() && it != --m_processList.end()) {
                processExit((*it)->getProcessID());
                break;
            }
        }

        if (it == m_processList.end()) {
            LOGE(TAG, "can't find appropriate process!\n");
        } else {
            delete (*it);
            m_processList.erase(it);
        }

        m_lock.unlock();
    }

    return ret;
}

bool ProcessRecordService::msgProcess()
{
    int ret = -1;
    Message msg;

    do {
        ret = m_msgQ.recvMsg(&msg, 0, -MSG_GAP, 0);
        if (-1 == ret) {
            LOGE(TAG, "m_msgQ.recvMsg fail\n");
            break;
        }

        switch (msg.m_type) {
        case MSG_NEWFLAG:
        {
            string name;
            m_msgQ.recvMsg(msg.m_type, name);
            unsigned int maxCount = m_msgQ.recvMsg(msg.m_type);
            ret = newFlag(name, maxCount);
            break;
        }

        case MSG_SETPROCESSFLAG:
        {
            int id = m_msgQ.recvMsg(msg.m_type);
            string name;
            m_msgQ.recvMsg(msg.m_type, name);
            ret = setProcessFlag(id, name);
            break;
        }

        case MSG_RMPROCESSFLAG:
        {
            int id = m_msgQ.recvMsg(msg.m_type);
            string name;
            m_msgQ.recvMsg(msg.m_type, name);
            ret = rmProcessFlag(id, name);
            break;
        }

        case MSG_ADDPROCESS:
        {
            ret = m_msgQ.recvMsg(msg.m_type);
            if (-1 == ret) {
            } else {
                ret = addProcess(ret);
            }
            break;
        }

        case MSG_RMPROCESS:
        {
            ret = m_msgQ.recvMsg(msg.m_type);
            if (-1 == ret) {
            } else {
                ret = rmProcess(ret);
            }
            break;
        }

        default:
            LOGE(TAG, "not supported msg type: %d\n", msg.m_type);
            ret = -1;
            break;
        }

        if (msg.m_type > MSG_INVALID && msg.m_type < MSG_MAX) {
            sendRet(msg.m_type, ret);
            if (-1 == ret) {
                LOGE(TAG, "msg%d fail\n", msg.m_type);
            }
        }
    } while (0);

    return (ret >= 0);
}

bool ProcessRecordService::processExit(int ID)
{
    int ret = -1;

    ret = kill(ID, SIGKILL);
    return (ret == 0);
}

int ProcessRecordService::sendRet(long type, int ret)
{
    return m_msgQ.sendMsg(type + MSG_NEWFLAG_ACK - MSG_NEWFLAG, ret);
}


