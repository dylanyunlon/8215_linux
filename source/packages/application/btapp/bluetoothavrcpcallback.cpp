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

#include "bluetoothavrcpcallback.h"
#include "bluetoothutils.h"

using namespace universal_utils;
static const char* const tag = "CBluetoothAVRCPCallBack";

template<> CBluetoothAVRCPCallBack* Singleton<CBluetoothAVRCPCallBack>::msSingleton = NULL;

CBluetoothAVRCPCallBack::CBluetoothAVRCPCallBack()
{

}

CBluetoothAVRCPCallBack::~CBluetoothAVRCPCallBack()
{
    LOGD(tag, "destrucrtor\n");

}

int CBluetoothAVRCPCallBack::onIndication(const CMessage &message)
{
    //LOGD(tag, "onIndication\n");

    LOGI(tag, "incoming indication %d\n", message.what);

    switch (message.what) {
        case AVRCP_IND_DEVICE_CONNECTED: {
            LOGD(tag, "AVRCP_IND_DEVICE_CONNECTED\n");

            string address = message.getStringExtra(STRING_ADDRESS, " ");
            int result = message.arg1;
            LOGI(tag, "address = %s, result is %d\n",
                BluetoothUtils::StringForLog(address).c_str(), result);

            if (0 == result) {
                emit sigAVRCPConnectedResponse(QString::fromStdString(address), true);
            } else {
                emit sigAVRCPConnectedResponse(QString::fromStdString(address), false);
            }
        }
        break;

        case AVRCP_IND_DEVICE_DISCONNECTED: {
            LOGD(tag, "AVRCP_IND_DEVICE_DISCONNECTED\n");

            string address = message.getStringExtra(STRING_ADDRESS, " ");
            LOGI(tag, "address = %s\n", BluetoothUtils::StringForLog(address).c_str());

            emit sigAVRCPDisconnectedResponse(QString::fromStdString(address));
        }
        break;

        case AVRCP_IND_PLAYBACK_DATA_UPDATE: {
            LOGD(tag, "AVRCP_IND_PLAYBACK_DATA_UPDATE\n");

            int musicState = message.getIntExtra(BYTE_PLAYBACK_STATUS, 0);
            LOGI(tag, "musicState:%d, stop(%d), playing(%d), pause(%d)\n",
                musicState,
                STOPPED,
                PLAYING,
                PAUSED);
            int playingTime = message.getIntExtra(INT_PLAYING_TIME, 0);
            int totalTime = message.getIntExtra(INT_TOTAL_TIME, 0);
            LOGI(tag, "playingTime is %d, totalTime is %d\n", playingTime, totalTime);

            emit sigPlaybackDateUpdate(musicState, playingTime, totalTime);
            }
            break;

        case AVRCP_IND_SONG_POSITION_CHANGED: {
            LOGD(tag, "AVRCP_IND_SONG_POSITION_CHANGED\n");

            int playingTime = message.getIntExtra(INT_PLAYING_TIME, 0);
            int totalTime = message.getIntExtra(INT_TOTAL_TIME, 0);
            LOGI(tag, "playingTime is %d\n", playingTime);

            emit sigMusicPositionChanged(playingTime, totalTime);
            }
            break;

        case AVRCP_IND_PLAY_STATUS_CHANGED: {
            LOGD(tag, "AVRCP_IND_PLAY_STATUS_CHANGED\n");

            int musicState = message.getIntExtra(BYTE_PLAYBACK_STATUS, 0);
            LOGI(tag, "musicState:%d, stop(%d), playing(%d), pause(%d), fwd_seek(%d), rev_seek(%d), error(%d)\n",
                musicState,
                STOPPED,
                PLAYING,
                PAUSED,
                FWD_SEEK,
                REV_SEEK,
                ERROR);

            emit sigPlayStateChanged(musicState);
            }
            break;

        case AVRCP_IND_MEDIA_DATA_CHANGED: {
            LOGD(tag, "AVRCP_IND_MEDIA_DATA_CHANGED\n");

            emit sigMediaDateChangedResponse();
            }
            break;

        default:
            LOGD(tag, "DEFALUT_IND\n");

            break;
    }

    return 0;
}



