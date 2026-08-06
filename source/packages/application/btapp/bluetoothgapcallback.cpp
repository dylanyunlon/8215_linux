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
 
#include "bluetoothgapcallback.h"
#include "bluetoothutils.h"

using namespace universal_utils;
static const char* const tag = "CBluetoothGAPCallBack";
template<> CBluetoothGAPCallBack* Singleton<CBluetoothGAPCallBack>::msSingleton = NULL;

CBluetoothGAPCallBack::CBluetoothGAPCallBack()
{

}

CBluetoothGAPCallBack::~CBluetoothGAPCallBack()
{
    LOGD(tag, "destrucrtor\n");

}

int CBluetoothGAPCallBack::onIndication(const CMessage &message)
{
    //LOGD(tag, "onIndication\n");

    LOGI(tag, "incoming indication  is %d\n", message.what);

    switch (message.what) {
        case GAP_POWER_STATE_CHANGE_IND: {
            LOGD(tag, "GAP_POWER_STATE_CHANGE_IND\n");
            E_GAP_POWERSTATE powerState = (E_GAP_POWERSTATE)(message.getIntExtra(INT_STATE, 0));
            if (POWERSTATE_POWER_ON_IN_PROCESS == powerState) {
                LOGD(tag, "POWERSTATE_POWER_ON_IN_PROCESS\n");
            } else if (POWERSTATE_POWER_OFF_IN_PROCESS == powerState) {
                LOGD(tag, "POWERSTATE_POWER_OFF_IN_PROCESS\n");
            }
        }
        break;

        case GAP_POWERON_IND: {
            LOGD(tag, "GAP_POWERON_IND\n");

            emit sigPowerState(true);
        }
        break;

        case GAP_POWEROFF_IND: {
            LOGD(tag, "GAP_POWEROFF_IND\n");

            emit sigPowerState(false);
        }
        break;

        case GAP_LOCAL_NAME_IND: {
            LOGD(tag, "GAP_LOCAL_NAME_IND\n");

            string name = message.getStringExtra(STRING_NAME, "");
            LOGI(tag, "name is %s\n", name.c_str());

            emit sigBluetoothLocalName(QString::fromStdString(name));
        }
        break;

        case GAP_SCAN_MODE_IND: {
            LOGD(tag, "GAP_SCAN_MODE_IND\n");

            emit sigScanModeActive(true);
        }
            break;

        case GAP_DISCOVERY_START_IND:
            LOGD(tag, "GAP_DISCOVERY_START_IND\n");

            emit sigScanStartResponse();

            break;

        case GAP_DISCOVERY_STOP_IND:
            LOGD(tag, "GAP_DISCOVERY_STOP_IND\n");

            emit sigScanStopResponse();

            break;

        case GAP_DISCOVERY_RESULT_IND: {
            LOGD(tag, "GAP_DISCOVERY_RESULT_IND\n");

            string address = message.getStringExtra(STRING_ADDRESS, "");
            string name = message.getStringExtra(STRING_NAME, "");
            int cod = message.getIntExtra(INT_COD, 0);
            LOGI(tag, "address is %s, name is %s, cod is %d\n",
                BluetoothUtils::StringForLog(address).c_str(), name.c_str(), cod);

            emit sigScanResponse(QString::fromStdString(address), QString::fromStdString(name));
        }
        break;

        case GAP_DISCOVERY_UPDATE_IND: {
            LOGD(tag, "GAP_DISCOVERY_UPDATE_IND\n");

            string address = message.getStringExtra(STRING_ADDRESS, "");
            string name = message.getStringExtra(STRING_NAME, "");
            LOGI(tag, "address is %s, name is %s\n",
                BluetoothUtils::StringForLog(address).c_str(), name.c_str());

            emit sigUpdateNameResponse(QString::fromStdString(address), QString::fromStdString(name));

        }
        break;

        case GAP_SECURITY_USER_CONFIRM_IND: {
            LOGD(tag, "GAP_SECURITY_USER_CONFIRM_IND\n");

            string address = message.getStringExtra(STRING_ADDRESS, "");
            string passKey = message.getStringExtra(STRING_SSP_NUMERIC, "");
            string bluetoothName = message.getStringExtra(STRING_NAME, "");
            LOGI(tag, "address is %s, passKey is:%s, bluetoothName is: %s\n",
                BluetoothUtils::StringForLog(address).c_str(), passKey.c_str(), bluetoothName.c_str());

            emit sigSecureUserConfirmRequest(QString::fromStdString(address));
        }
        break;

        case GAP_BONDING_RESULT_IND: {
            LOGD(tag, "GAP_BONDING_RESULT_IND\n");

            BluetoothAddress bt_address = message.getStringExtra(STRING_ADDRESS, "");
            string address = bt_address.toString();
            string name = message.getStringExtra(STRING_NAME, "");
            int result = message.arg1;
            LOGI(tag, "address is %s, name is %s, result is %d\n",
                BluetoothUtils::StringForLog(address).c_str(), name.c_str(), result);

            if (0 == result) {
                emit sigPairResponse(QString::fromStdString(address), QString::fromStdString(name), true);
            } else {
                emit sigPairResponse(QString::fromStdString(address), QString::fromStdString(name), false);
            }

        }
        break;

        case GAP_BOND_REMOVED_IND: {
            LOGD(tag, "GAP_BOND_REMOVED_IND\n");

            BluetoothAddress bt_address = message.getStringExtra(STRING_ADDRESS, "");
            string address = bt_address.toString();
            string name = message.getStringExtra(STRING_NAME, "");
            LOGI(tag, "address is %s, name is %s\n",
                BluetoothUtils::StringForLog(address).c_str(), name.c_str());

            emit sigPairRemoveResponse(QString::fromStdString(address), QString::fromStdString(name));
        }
        break;

        case GAP_PIN_CODE_IND:{
            LOGD(tag, "GAP_PIN_CODE_IND\n");

            BluetoothAddress bt_address = message.getStringExtra(STRING_ADDRESS, "");
            string address = bt_address.toString();
            LOGI(tag, "address is %s\n",
                BluetoothUtils::StringForLog(address).c_str());
        }
        break;

        case GAP_LINK_STATE_IND:{
            LOGD(tag, "GAP_LINK_STATE_IND\n");

            string address = message.getStringExtra(STRING_ADDRESS, "");
            int currentNumber = message.getIntExtra(INT_CURRENT_NUMBER, 0);
            int errorCode = message.getIntExtra(INT_ERROR_CODE, 0);
            LOGI(tag, "address is %s, currentNumber is %d, errorCode is 0x%x\n",
                BluetoothUtils::StringForLog(address).c_str(), currentNumber, errorCode);
            if(HCI_ERR_CONNECTION_TIMEOUT == errorCode || HCI_ERR_LMP_RESPONSE_TIMEOUT == errorCode) {
                emit sigGapLinkStateErrorInd(QString::fromStdString(address));
            }

        }
        break;

        case GAP_REMOTE_UUID_IND:{
            LOGD(tag, "GAP_REMOTE_UUID_IND\n");

            BluetoothAddress bt_address = message.getStringExtra(STRING_ADDRESS, "");
            string address = bt_address.toString();
            string name = message.getStringExtra(STRING_NAME, "");

            emit sigUuidsUpdate(QString::fromStdString(address), QString::fromStdString(name));
        }

        default: {
            LOGD(tag, "default IND\n");
        }
        break;
    }

    return 0;
}

