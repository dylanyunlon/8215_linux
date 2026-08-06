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
 
#include "bluetoothpbapcallback.h"
using namespace universal_utils;

static const char* const tag = "CBluetoothPBAPCallBack";
template<> CBluetoothPBAPCallBack* Singleton<CBluetoothPBAPCallBack>::msSingleton = NULL;

CBluetoothPBAPCallBack::CBluetoothPBAPCallBack()
{

}

CBluetoothPBAPCallBack::~CBluetoothPBAPCallBack()
{
    LOGD(tag, "destrucrtor\n");

}

int CBluetoothPBAPCallBack::onIndication(const CMessage &message)
{
    //LOGD(tag, "onIndication\n");

    LOGI(tag, " incoming indication %d\n", message.what);

    switch (message.what) {
        case PBAP_IND_DOWNLOAD_ONESTEP: {
            LOGD(tag, "PBAP_IND_DOWNLOAD_ONESTEP\n");

            int pbPathType = message.getIntExtra(INT_PBAP_DOWNLOAD_PATH, 0);
            int pbCurrentIndex = message.getIntExtra(INT_PBAP_DOWNLOAD_INDEX, 0);
            LOGI(tag, " pbPathType is %d, pbCurrentIndex is %d\n", pbPathType, pbCurrentIndex);

            if ((PBMGR_PHONEBOOK == (PBType)pbPathType) | (PBMGR_SIM_PHONEBOOK == (PBType)pbPathType)) {
                emit sigPhoneBookDownloadRecord(pbPathType, pbCurrentIndex);
            } else if ((PBMGR_INCOMING_CALLS_HISTORY == (PBType)pbPathType) |
                        (PBMGR_SIM_INCOMING_CALLS_HISTORY == (PBType)pbPathType) |
                        (PBMGR_OUTGOING_CALLS_HISTORY == (PBType)pbPathType) |
                        (PBMGR_SIM_OUTGONING_CALLS_HISTORY == (PBType)pbPathType) |
                        (PBMGR_MISSED_CALLS_HISTORY == (PBType)pbPathType) |
                        (PBMGR_SIM_MISSED_CALLS_HISTORY == (PBType)pbPathType) |
                        (PBMGR_COMBINED_CALLED_HISTORY == (PBType)pbPathType)) {
                emit sigCallRecordsDownloadRecord(pbPathType, pbCurrentIndex);
            }
        }
        break;

        case PBAP_IND_DOWNLOAD_FINISH: {
            LOGD(tag, "PBAP_IND_DOWNLOAD_FINISH\n");

            int pbPathType = message.getIntExtra(INT_PBAP_DOWNLOAD_PATH, 0);
            LOGI(tag, " pbPathType is %d\n", pbPathType);

            if ((PBMGR_PHONEBOOK | PBMGR_SIM_PHONEBOOK) == (PBType)pbPathType) {
                emit sigPhoneBookDownloadFinish();
            } else if ((PBMGR_INCOMING_CALLS_HISTORY | PBMGR_OUTGOING_CALLS_HISTORY |
                PBMGR_MISSED_CALLS_HISTORY) == (PBType)pbPathType) {
                emit sigCallRecordsDownloadFinish();
            }
        }
        break;

        case PBAP_IND_DOWNLOAD_STOP: {
            LOGD(tag, "PBAP_IND_DOWNLOAD_STOP\n");

            int pbPathType = message.getIntExtra(INT_PBAP_DOWNLOAD_PATH, 0);
            LOGI(tag, " pbPathType is %d\n", pbPathType);

            if ((PBMGR_PHONEBOOK | PBMGR_SIM_PHONEBOOK) == (PBType)pbPathType) {
                emit sigPhoneBookDownloadStop();
            } else if ((PBMGR_INCOMING_CALLS_HISTORY | PBMGR_OUTGOING_CALLS_HISTORY |
                PBMGR_MISSED_CALLS_HISTORY) == (PBType)pbPathType) {
                emit sigCallRecordsDownloadStop();
            }
        }
        break;

        case PBAP_IND_DOWNLOAD_ERROR: {
            LOGD(tag, "PBAP_IND_DOWNLOAD_ERROR\n");

            int pbPathType = message.getIntExtra(INT_PBAP_DOWNLOAD_PATH, 0);
            LOGI(tag, " pbPathType is %d\n", pbPathType);

            if ((PBMGR_PHONEBOOK | PBMGR_SIM_PHONEBOOK) == (PBType)pbPathType) {
                emit sigPhoneBookDownloadStop();
            } else if ((PBMGR_INCOMING_CALLS_HISTORY | PBMGR_OUTGOING_CALLS_HISTORY |
                PBMGR_MISSED_CALLS_HISTORY) == (PBType)pbPathType) {
                emit sigCallRecordsDownloadStop();
            }
        }
        break;

        case PBAP_IND_DOWNLOAD_START: {
            LOGD(tag, "PBAP_IND_DOWNLOAD_START\n");

            int pbPathType = message.getIntExtra(INT_PBAP_DOWNLOAD_PATH, 0);
            LOGI(tag, " pbPathType is %d\n", pbPathType);

            if ((PBMGR_PHONEBOOK | PBMGR_SIM_PHONEBOOK) == (PBType)pbPathType) {
                emit sigPhoneBookDownloadStart();
            } else if ((PBMGR_INCOMING_CALLS_HISTORY | PBMGR_OUTGOING_CALLS_HISTORY |
                PBMGR_MISSED_CALLS_HISTORY) == (PBType)pbPathType) {
                emit sigCallRecordsDownloadStart();
            }
        }
        break;

        default:
            LOGD(tag, "default IND!\n");

            break;

    }

    return 0;
}

