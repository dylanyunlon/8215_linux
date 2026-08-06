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

#ifndef BLUETOOTHFILESYNC_H
#define BLUETOOTHFILESYNC_H

#include "../../connectivity/universal_utils/include/cautothread.h"
#include "../../connectivity/universal_utils/include/cconditionlock.h"
#include "../../connectivity/universal_utils/include/csync.h"

class CBluetoothFileSync: public universal_utils::CAutoThread
{
public:
    CBluetoothFileSync();
    ~CBluetoothFileSync();

    bool startSync();

protected:
    unsigned long threadRun();


private:
    universal_utils::CConditionLock m_conditionLock;
    int m_fileSyncTrigCond;
    universal_utils::CMutexObject* m_mutextSyncLock;
};


#endif

