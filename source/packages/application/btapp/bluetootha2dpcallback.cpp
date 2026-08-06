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
 
#include "bluetootha2dpcallback.h"
#include "bluetoothutils.h"

static const char* const tag = "CBluetoothA2DPCallBack";
using namespace universal_utils;
template<> CBluetoothA2DPCallBack* Singleton<CBluetoothA2DPCallBack>::msSingleton = NULL;

CBluetoothA2DPCallBack::CBluetoothA2DPCallBack ()
{

}

CBluetoothA2DPCallBack::~CBluetoothA2DPCallBack()
{
    LOGD(tag, "destrucrtor\n");

}

int CBluetoothA2DPCallBack::onIndication(const CMessage &message)
{
    //LOGD(tag, "onIndication\n");

    LOGI(tag, "incoming indication %d\n", message.what);

    switch (message.what) {
        case A2DP_IND_DEVICE_CONNECTED: {
            LOGD(tag, "A2DP_IND_DEVICE_CONNECTED\n");
            string address = message.getStringExtra(STRING_ADDRESS, " ");
            int result = message.arg1;
            LOGI(tag, "address = %s, result is %d\n",
                BluetoothUtils::StringForLog(address).c_str(), result);

            if (0 == result) {
                emit sigA2DPConnectedResponse(QString::fromStdString(address), true);
            } else {
                emit sigA2DPConnectedResponse(QString::fromStdString(address), false);
            }
        }
        break;

        case A2DP_IND_DEVICE_DISCONNECTED: {
            LOGD(tag, "A2DP_IND_DEVICE_DISCONNECTED\n");
            string address = message.getStringExtra(STRING_ADDRESS, " ");
            LOGI(tag, "address = %s\n", BluetoothUtils::StringForLog(address).c_str());

            emit sigA2DPDisconnectedResponse(QString::fromStdString(address));
            emit sigAudioRelease();
        }
        break;

        case A2DP_IND_PLAY_START:
            LOGD(tag, "A2DP_IND_PLAY_START\n");
            emit sigAudioRequest();
            break;

        case A2DP_IND_PLAY_SUSPEND:
            LOGD(tag, "A2DP_IND_PLAY_SUSPEND\n");
            emit sigAudioRelease();
            break;

        case A2DP_IND_END:
            LOGD(tag, "A2DP_IND_END\n");
            break;

    }

    return 0;
}


