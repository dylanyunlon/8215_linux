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
 
#include "bluetoothhfpcallback.h"
#include "bluetoothutils.h"

using namespace universal_utils;
static const char* const tag = "CBluetoothHFPCallBack";
template<> CBluetoothHFPCallBack* Singleton<CBluetoothHFPCallBack>::msSingleton = NULL;

CBluetoothHFPCallBack::CBluetoothHFPCallBack()
{

}

CBluetoothHFPCallBack::~CBluetoothHFPCallBack()
{
    LOGD(tag, "destrucrtor\n");

}

int CBluetoothHFPCallBack::onIndication(const CMessage &message)
{
    //LOGD(tag, "onIndication\n");

    LOGI(tag, "incoming indication  is %d\n", message.what);

    switch (message.what) {
        case HF_IND_DEVICE_CONNECTED:{
            string address = message.getStringExtra(STRING_ADDRESS, " ");
            LOGI(tag, "HF_IND_DEVICE_CONNECTED address is %s.\n", BluetoothUtils::StringForLog(address).c_str());

            emit sigHFConnectedResponse(QString::fromStdString(address), true);
        }
        break;

        case HF_IND_DEVICE_DISCONNECTED:{
            LOGD(tag, "HF_IND_DEVICE_DISCONNECTED\n");

            string address = message.getStringExtra(STRING_ADDRESS, " ");
            LOGI(tag, "address is %s\n", BluetoothUtils::StringForLog(address).c_str());

            emit sigHFDisconnectedResponse(QString::fromStdString(address));
        }
        break;

        case HF_IND_DEVICE_CONNECTING:{
            LOGD(tag, "HF_IND_DEVICE_CONNECTING\n");

            string address = message.getStringExtra(STRING_ADDRESS, " ");
            LOGI(tag, "address is %s\n", BluetoothUtils::StringForLog(address).c_str());

            emit sigHFConnectingResponse(QString::fromStdString(address));
        }
        break;

        case HF_IND_CALLSTATE:{
            LOGD(tag, "HF_IND_CALLSTATE\n");

            int callState = message.getIntExtra(INT_HF_CALLSTATE, 0);
            LOGI(tag, "callState:%d, IDLE(%d), incoming(%d), outgoing(%d),alerting(%d), speaking(%d), waiting(%d), held(%d)\n",
                callState,
                HF_CALLSTATE_IDLE,
                HF_CALLSTATE_INCOMING,
                HF_CALLSTATE_OUTGOING,
                HF_CALLSTATE_ALERTING,
                HF_CALLSTATE_SPEAKING,
                HF_CALLSTATE_WAITING,
                HF_CALLSTATE_HELD);
            emit sigCallStateChange(callState);
        }
        break;

        case HF_IND_INBANDRING_PROVIDE_STATE:{
            LOGD(tag, "HF_IND_INBANDRING_PROVIDE_STATE\n");

            int supportInBandRing = message.getIntExtra(INT_HF_INBANDRING_PROVIDE, 0);
            LOGI(tag, "supportInBandRing is %d\n", supportInBandRing);

            emit sigSupportInBandRing(supportInBandRing);
        }
        break;

        case HF_IND_CALLNUMBER:{
            LOGD(tag, "HF_IND_CALLNUMBER\n");

            string callNumber = message.getStringExtra(STRING_HF_CALLNUMBER, " ");
            LOGI(tag, "callNumber is %s\n", callNumber.c_str());

            emit sigCallNumber(QString::fromStdString(callNumber));
        }
        break;

        case HF_IND_CALLWAITINGNUMBER:{
            LOGD(tag, "HF_IND_CALLWAITINGNUMBER\n");

            string callWaitingNumber = message.getStringExtra(STRING_HF_CALLWAITINGNUMBER, " ");
            LOGI(tag, "callWaitingNumber is %s\n", callWaitingNumber.c_str());
        }
        break;

        case HF_IND_CALLCHANGE: {     //CallList changed
            LOGD(tag, "HF_IND_CALLCHANGE\n");
            emit sigCallListChange();
        }
        break;
        case HF_IND_CALLAUDIOTRANSFER:{
            LOGD(tag, "HF_IND_CALLAUDIOTRANSFER\n");

            int scoState = message.getIntExtra(STRING_HF_AUDIOTOWARDS, 0);
            LOGI(tag, "scoState is %d\n", scoState);
            if (HF_AUDIOTOWARDS_HF == scoState) {
                emit sigScoState(true);
            } else {
                emit sigScoState(false);
            }
        }
        break;

        case HF_IND_PHONEFACTORY:{
            LOGD(tag, "HF_IND_PHONEFACTORY\n");

            std::string phoneFactroy = message.getStringExtra(STRING_HF_PHONEFACTORY, " ");
            LOGI(tag, "phoneFactroy is %s\n", phoneFactroy.c_str());
            emit sigPhoneFactroy(QString::fromStdString(phoneFactroy));
        }
        break;

        case HF_IND_PHONESERIAL:{
            LOGD(tag, "HF_IND_PHONESERIAL\n");

            std::string phoneSerial = message.getStringExtra(STRING_HF_PHONESERIAL, " ");
            LOGI(tag, "phoneSerial is %s\n", phoneSerial.c_str());
            emit sigPhoneSerial(QString::fromStdString(phoneSerial));
        }
        break;

        case HF_IND_PHONEINDICATION: {
            LOGD(tag, "HF_IND_PHONEINDICATION\n");

            int service = message.getIntExtra(INT_HF_CIEV_SERVICE, 0);
            int signal = message.getIntExtra(INT_HF_CIEV_SIGNAL, 0);
            int battchg = message.getIntExtra(INT_HF_CIEV_BATTCHG, 0);
            int roam = message.getIntExtra(INT_HF_CIEV_ROAM, 0);
            LOGI(tag, "service is %d, signal is %d, battchg is %d, roam is %d\n", service, signal, battchg, roam);

        }
        break;

        case HF_IND_RINGTONE: {
            LOGD(tag, "HF_IND_RINGTONE\n");
        }
        break;

        case HF_IND_PHONESPEAKERGAIN: {
            LOGD(tag, "HF_IND_PHONESPEAKERGAIN\n");

            int agSpeakerGain = message.getIntExtra(INT_HF_AGSPEAKERGAIN, 0);
            LOGI(tag, "agSpeakerGain is %d\n", agSpeakerGain);
            emit sigUpdateSpeakerGain(agSpeakerGain);
        }
        break;

        case HF_IND_PHONEMICGAIN:{
            LOGD(tag, "HF_IND_PHONEMICGAIN\n");

            int agMicGain = message.getIntExtra(INT_HF_AGMICGAIN, 0);
            LOGI(tag, "agMicGain is %d\n", agMicGain);
        }
        break;

        case HF_IND_ATRESULT:{
            LOGD(tag, "HF_IND_ATRESULT\n");

            int resultCode = message.getIntExtra(INT_HF_RESULT_CODE, 0);
            if(HF_AT_RESULT_ERROR_NO_ANSWER == resultCode) {
                LOGI(tag, "resultCode is %d\n", resultCode);
                emit sigATNOAnswer();
            } else if (BTA_HF_CLIENT_AT_RESULT_UNKNWON_AT_EVENT == resultCode) {
                LOGI(tag, "resultCode is :%d\n", resultCode);
            }
        }
        break;

        default: {
            LOGD(tag, "default_IND\n");

        }
        break;
        }
    return 0;
}

