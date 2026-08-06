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
 
#ifndef CBUSOBJ_H
#define CBUSOBJ_H

#include "apptype.h"
#include "ccmdtask.h"
#include "cudpsocketproc.h"
#include "stddef.h"
#include "iostream"
#include "sharememory.h"
#include "globalbus.h"
#include "ccmdtask.h"

class CSysStatus
{
public:
    GlobalBus::E_BT_STATE           m_BtState;
    GlobalBus::E_LANGUAGE_STATE     m_Language;
    GlobalBus::E_CLOCK_STATE        m_Clock;
    GlobalBus::E_WIFI_STATE         m_WiFi;
    GlobalBus::E_IPOD_STATE         m_iPod;
};

class CBusObj
{
public:
    CBusObj();
    virtual ~CBusObj();
    bool applyFor (unsigned char action,
                        unsigned char appID,
                        unsigned int param) const;
    bool applyFor(const CMDPacket &pack) const;

    bool jumpTo(unsigned char fromAppID,
                unsigned char toAppID,
                unsigned char param) const;

    int getState (GlobalBus::E_STATE_TYPE type) const;
    bool setState(GlobalBus::E_STATE_TYPE type, int state) const;

protected:
    bool notifyAppManager (const CMDPacket &pack) const;

private:
    CUDPSocket *m_socket;
    std::string m_appManagerSocketAddr;
    CShareMemory<CSysStatus> *m_systemStatus;
    const char *m_shmemPath;
    const char m_shmemID;//for sharememory ftok, less than 255
};

#endif // CBUSOBJ_H
