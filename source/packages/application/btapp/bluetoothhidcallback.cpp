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

#include "bluetoothhidcallback.h"

static const char* const tag = "CBluetoothHIDCallBack";
using namespace universal_utils;

template<> CBluetoothHIDCallBack* Singleton<CBluetoothHIDCallBack>::msSingleton = NULL;

CBluetoothHIDCallBack::CBluetoothHIDCallBack()
{

}

CBluetoothHIDCallBack::~CBluetoothHIDCallBack()
{
    LOGD(tag, "destrucrtor\n");

}

int CBluetoothHIDCallBack::onIndication(const CMessage &message)
{
    LOGI(tag, "incoming indication  is %d\n", message.what);

    switch (message.what) {
        case HID_IND_DEVICE_CONNECTED:{
            LOGD(tag, "HID_IND_DEVICE_CONNECTED\n");

            string address = message.getStringExtra(STRING_ADDRESS, " ");
            LOGD(tag, "address is %s\n", address.c_str());
        }
        break;

        case HID_IND_DEVICE_DISCONNECTED:{
            LOGD(tag, "HID_IND_DEVICE_DISCONNECTED\n");

            string address = message.getStringExtra(STRING_ADDRESS, " ");
            LOGD(tag, "address is %s\n", address.c_str());
        }
        break;

        default: {
            LOGD(tag, "default_IND\n");

        }
        break;
        }
    return 0;
}

