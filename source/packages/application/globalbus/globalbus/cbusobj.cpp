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

#include <sys/types.h>
#include <sys/ipc.h>
#include "cbusobj.h"
#include "applog.h"
#include "utils.h"
#include "globalbus.h"

static const char TAG[] = "CBusObj";
static const char SHMEM_PATH[] = "/usr/lib";
static const char SHMEN_ID = 215;


CBusObj::CBusObj()
    : m_appManagerSocketAddr(APPMANAGER_LISTENER_SOCKET_ADDR)
    , m_shmemPath(SHMEM_PATH)
    , m_shmemID(SHMEN_ID)
{
    m_socket = new CUDPSocket();
    if (NULL == m_socket) {
        LOGE(TAG, "new CUDPSocket fail\n");
        throw m_socket;
    }

    key_t key = 0;

    key = ftok(m_shmemPath, m_shmemID);
    m_systemStatus = new CShareMemory<CSysStatus>(key);
    if (NULL == m_systemStatus) {
        LOGE(TAG, "new CShareMemory<CSysStatus>(%d) fail\n", key);
        throw m_systemStatus;
    }
}

CBusObj::~CBusObj()
{
    SAFE_DELETE(m_socket);
    SAFE_DELETE(m_systemStatus);
}

bool CBusObj::applyFor (unsigned char action,
                            unsigned char appID,
                            unsigned int param) const
{
    bool ret = false;
    CMDPacket pack;

    if (isMonkeySingleAppTesting()) {
        if (GlobalBus::ACTION_EXIT == action || GlobalBus::ACTION_GOHOME == action) {
            LOGD(TAG, "block %s in MonkeySingleAppTesting!\n",
                GlobalBus::decode((GlobalBus::E_APPLY_ACTION)action));
            ret = true;
        }
    } else {
        pack.m_mainFunc = CCmdTask::MAIN_FUNC_APP_ACTION;
        pack.m_subFunc = action;
        pack.m_appID = appID;
        pack.m_data = param;
        ret = notifyAppManager(pack);
    }

    return ret;
}

bool CBusObj::applyFor(const CMDPacket &pack) const
{
    bool ret = false;

    ret = notifyAppManager(pack);

    return ret;
}

//only use to jump home clock to setting
bool CBusObj::jumpTo(unsigned char fromAppID,
                    unsigned char toAppID,
                    unsigned char param) const
{
    // bool ret = false;
    // CMDPacket pack;

    // pack.m_mainFunc = CCmdTask::MAIN_FUNC_APP_JUMP;
    // pack.m_subFunc = fromAppID;
    // pack.m_data = (toAppID << 8) | (param);
    // ret = notifyAppManager(pack);

    // return ret;
    return true;
}

bool CBusObj::notifyAppManager(const CMDPacket &pack) const
{
    bool ret = false;
    int len = -1;

    len = m_socket->write((const void *)&pack, sizeof(pack),
                            m_appManagerSocketAddr);

    if (len >= (int)sizeof(pack)) {
        ret = true;
    } else {
        LOGE(TAG, "socket send fail! len(%d)\n", len);
        ret = false;
    }

    return ret;
}

int CBusObj::getState(GlobalBus::E_STATE_TYPE type) const
{
    int result = -1;
    CSysStatus *sysStatus = m_systemStatus->getMemDataLock();

    if (NULL != sysStatus) {
        switch (type) {
        case GlobalBus::STATE_BT:
            result = sysStatus->m_BtState;
            break;
        case GlobalBus::STATE_CLOCK:
            result = sysStatus->m_Clock;
            break;
        case GlobalBus::STATE_LANGUAGE:
            result = sysStatus->m_Language;
            break;
        case GlobalBus::STATE_WIFI:
            result = sysStatus->m_WiFi;
            break;
        case GlobalBus::STATE_IPOD:
            result = sysStatus->m_iPod;
            break;

        default:
            LOGE(TAG, "No such Status type!\n");
            break;
        }

        m_systemStatus->memDataUnLock();
    } else {
        LOGE(TAG, "can't get MemData!\n");
    }

    return result;
}

bool CBusObj::setState(GlobalBus::E_STATE_TYPE type, int state) const
{
    bool ret = false;
    CSysStatus *sysStatus = m_systemStatus->getMemDataLock();
    GlobalBus::E_STATE_TYPE stateType = GlobalBus::INVALID_STATE_TYPE;

    if (NULL != sysStatus) {
        switch (type) {
        case GlobalBus::STATE_BT:
            sysStatus->m_BtState = (GlobalBus::E_BT_STATE)state;
            stateType = type;
            break;
        case GlobalBus::STATE_CLOCK:
            sysStatus->m_Clock = (GlobalBus::E_CLOCK_STATE)state;
            stateType = type;
            break;
        case GlobalBus::STATE_LANGUAGE:
            sysStatus->m_Language = (GlobalBus::E_LANGUAGE_STATE)state;
            stateType = type;
            break;
        case GlobalBus::STATE_WIFI:
            sysStatus->m_WiFi = (GlobalBus::E_WIFI_STATE)state;
            stateType = type;
            break;
        case GlobalBus::STATE_IPOD:
            sysStatus->m_iPod = (GlobalBus::E_IPOD_STATE)state;
            stateType = type;
            break;

        default:
            LOGE(TAG, "No such Status type!\n");
            break;
        }

        m_systemStatus->memDataUnLock();

        CMDPacket pack;

        pack.m_mainFunc = CCmdTask::MAIN_FUNC_APP_ACTION;
        pack.m_subFunc = GlobalBus::ACTION_STATUS_CHANGED;
        pack.m_data = stateType << 24;
        ret = notifyAppManager(pack);
    } else {
        LOGE(TAG, "can't get MemData!\n");
    }

    return ret;
}

