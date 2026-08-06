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

#include "ccmdtask.h"
#include "csocketlistener.h"
#include "globalbus.h"
#include "applog.h"

static const char* TAG = "socketListener";

CSocketListener::CSocketListener()
{

}

CSocketListener::~CSocketListener()
{

}

bool CSocketListener::startListen ()
{
    bool ret = false;
    int res = -1;

    do {
        res = bind(APPMANAGER_LISTENER_SOCKET_ADDR);
        if (-1 == res) {
            LOGE(TAG, "bind fail\n");
            ret = false;
            break;
        }

        res = startService();
        if (-1 == res) {
            LOGE(TAG, "startService fail\n");
            ret = false;
            break;
        }

        ret = true;
    } while (0);

    return ret;
}

int CSocketListener::onReceive()
{
    int recvLen = -1;
    CMDPacket pack;
    std::string addr;

    recvLen = read(&pack, sizeof(pack), addr);

    if (CCmdTask::MAIN_FUNC_APP_ACTION == pack.m_mainFunc) {
        LOGD(TAG, "receiver pack mainFunc %s(%d) subFunc %s(%d) appid(%d) data(0x%X) .\n",
                    CCmdTask::decode((CCmdTask::E_MAINFUNC)pack.m_mainFunc), pack.m_mainFunc,
                    GlobalBus::decode((GlobalBus::E_APPLY_ACTION)pack.m_subFunc), pack.m_subFunc,
                    pack.m_appID, pack.m_data);
    } else if (CCmdTask::MAIN_FUNC_APP_JUMP == pack.m_mainFunc) {
        LOGD(TAG, "receiver pack mainFunc %s(%d) from app(%d) to app(%x)\n",
                    CCmdTask::decode((CCmdTask::E_MAINFUNC)pack.m_mainFunc), pack.m_mainFunc,
                    pack.m_subFunc,
                    pack.m_data);
    } else {
        LOGD(TAG, "receiver pack mainFunc %s(%d) subFunc(%d) data(0x%X)\n",
                    CCmdTask::decode((CCmdTask::E_MAINFUNC)pack.m_mainFunc), pack.m_mainFunc,
                    pack.m_subFunc,
                    pack.m_data);
    }

    addTask(pack);

    return recvLen;
}

