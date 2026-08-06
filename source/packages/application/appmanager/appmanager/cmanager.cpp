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

#include "applog.h"
#include "cmanager.h"

static const char TAG[] = "CManager";

CManager::CManager()
{

}

CManager::~CManager()
{

}

bool CManager::onDupRun(unsigned char appID, unsigned int data)
{
    LOGD(TAG, "%s: appID(%d) data(%d)\n", appID, data);
    return true;
}

bool CManager::processAutoTestCmd(const CCmdTask &cmdTask)
{
    LOGD(TAG, "CCmdTask: mainFunc(%d)\n", cmdTask.getMainFunc());
    return true;
}

bool CManager::onMiscRequest(const CCmdTask &cmdTask)
{
    LOGD(TAG, "CCmdTask: mainFunc(%d)\n", cmdTask.getMainFunc());
    return true;
}

