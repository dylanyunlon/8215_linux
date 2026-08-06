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

#include "bluetoothphoneapplication.h"

CBluetoothPhoneApplication::CBluetoothPhoneApplication(QWindow *appWindow)
    : CQObjListener(CAPPBaseObj::APPID_BTPHONE)
    , m_btPhoneWindow(appWindow)
{
}

CBluetoothPhoneApplication::~CBluetoothPhoneApplication()
{
}

unsigned long CBluetoothPhoneApplication::threadRun()
{
    GlobalBus::applyFor(GlobalBus::ACTION_DUPRUN, CAPPBaseObj::APPID_BTPHONE, CAPPBaseObj::LEVEL_NORMAL);

    while (!initListener(m_btPhoneWindow, NULL, NULL, NULL, true)) {
        sleep(1);
    }

    return 0;
}

