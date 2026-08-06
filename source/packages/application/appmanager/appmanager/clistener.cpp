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

#include "clistener.h"
#include "cappmanager.h"
#include "applog.h"

static const char *TAG = "CListener";

CListener::CListener()
{

}

CListener::~CListener()
{

}

bool CListener::addTask(CMDPacket &pack)
{
    bool ret = false;

    CAppManager *appManager = CAppManager::getSingletonPtr();
    if (appManager != NULL) {
        ret = appManager->addTask(pack);
        if (!ret) {
            LOGE(TAG, "appManager->addTask fail\n");
        }
    } else {
        LOGE(TAG, "CAppManager::getSingletonPtr return NULL!\n");
    }

    return ret;
}

