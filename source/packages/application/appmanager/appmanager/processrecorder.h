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

#ifndef PROCESS_RECORDER_H
#define PROCESS_RECORDER_H

#include <list>
#include <string>
#include "threadproc.h"
#include "messagequeue.h"

using std::list;
using std::string;

class ProcessFlag;

class Process
{
public:
    explicit Process(int id)
        : m_ID(id)
    {
    }
    virtual ~Process()
    {
    }

    int setFlag(ProcessFlag *pFlag);
    int rmFlag(ProcessFlag *pFlag);
    int rmFlag();

    int getFlagCount()
    {
        return m_flagList.size();
    }

    int getProcessID() const
    {
        return m_ID;
    }
/*
    bool operator ==(const Process ps) const
    {
        return (m_ID == ps.getProcessID());
    }

    bool operator ==(int id) const
    {
        return (m_ID == id);
    }
*/
    void dumpFlag() const;

private:
    int m_ID;
    list<ProcessFlag *> m_flagList;
};

class ProcessFlag
{
public:
    explicit ProcessFlag(const string &name, unsigned int maxCount = 1)
        : m_name(name)
        , m_maxCount(maxCount)
    {
    }
    virtual ~ProcessFlag()
    {
    }

    const string &getName() const
    {
        return m_name;
    }

    int setProcess(Process *pPs);
    int rmProcess(const Process *pPs);
    int rmProcess();
/*
    bool operator ==(const ProcessFlag &flag2) const
    {
        return (m_name == flag2.getName());
    }

    bool operator ==(const string &name2) const
    {
        return (m_name == name2);
    }
*/
    void dumpProcess() const;

private:
    const string m_name;
    unsigned int m_maxCount;
    list<Process *> m_processList;
};

class ProcessRecordClient
{
public:
    ProcessRecordClient(const string &pathname, int proj_id);
    virtual ~ProcessRecordClient()
    {
    }

    // must have service, or memfunc will hang
    int newFlag(const std::string &name, unsigned int maxCount = 1);
    int setProcessFlag(int id, const string &name);
    int rmProcessFlag(int id, const string &name);
    int addProcess(int id);
    int rmProcess(int id);

private:
    int getRet(long type);

    MessageQueue m_msgQ;
};

class ProcessRecordService
{
public:
    ProcessRecordService(const string &pathname, int proj_id);
    virtual ~ProcessRecordService();

    int getProcessCount();

    virtual bool checkCondition() = 0;
    virtual bool processExit(int ID);

protected:
    bool mainProcess();
    bool msgProcess();

protected:
    MessageQueue m_msgQ;

private:
    int newFlag(const std::string &name, unsigned int maxCount = 1);
    int setProcessFlag(int id, const string &name);
    int rmProcessFlag(int id, const string &name);
    int addProcess(int id);
    int rmProcess(int id);

    int sendRet(long type, int ret);

    int m_qid;
    list<Process *> m_processList;
    list<ProcessFlag *> m_flagList;
    universal_utils::LoopThread<ProcessRecordService> m_msgProc;
    universal_utils::LoopThread<ProcessRecordService> m_mainProc;
    universal_utils::CMutexObject m_lock;
};

#endif
