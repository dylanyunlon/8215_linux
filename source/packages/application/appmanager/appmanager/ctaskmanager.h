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

#ifndef CTASKMANAGER_H
#define CTASKMANAGER_H

#include <list>
#include "cautothread.h"
#include "cconditionlock.h"
#include "ccmdtask.h"

class CTaskManager
    : private universal_utils::CAutoThread
    , protected universal_utils::CConditionLock
{
public:
    CTaskManager();
    ~CTaskManager();

    virtual bool addTask(const CCmdTask &cmdTask);
    virtual bool clearTask();

    int getCurTaskCount();
    bool isDoingTask();

    virtual bool startTaskWatcher();
    virtual bool restartTaskWatcher();
    virtual bool stopTaskWatcher();
    virtual bool forceTerminateTaskWatcher();
    virtual bool waitTaskWatcherFinish(unsigned long milliSeconds);

protected:
    std::list<CCmdTask> m_taskList;

private:
    bool matchCmd(const CCmdTask &cmdTask);

    bool cleanupLock();
    static void clearAwait(void *arg);

    unsigned long threadRun();
    virtual bool doTask (const CCmdTask &cmdTask) = 0;

    CTaskManager(const CTaskManager &rhs);
    const CTaskManager& operator = (const CTaskManager &rhs);

    unsigned long m_taskRollTime;
    bool m_isDoingTask;
    int m_taskRunCondition;
};

#endif // CTASKMANAGER_H
