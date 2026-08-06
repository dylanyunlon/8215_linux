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
 
#ifndef GLOBALBUS_H
#define GLOBALBUS_H

#include "ccmdtask.h"

namespace GlobalBus
{
typedef enum {
    ACTION_RUN = 0,
    ACTION_EXIT,
    ACTION_SHOWFRONT,
    ACTION_HIDEFRONT,
    ACTION_SHOWREAR,
    ACTION_HIDEREAR,
    ACTION_AUDIO_REQ,  // audio request.
    ACTION_AUDIO_REL,  // audio release.
    ACTION_VIDEO_REQ,
    ACTION_VIDEO_REL,
    ACTION_AV_REQ,
    ACTION_AV_REL,
    ACTION_GOHOME,

    ACTION_DUPRUN,
    ACTION_OUTSIDE_AUDIO_REQ,   // notify appmanager outside audio
    ACTION_OUTSIDE_AUDIO_REL,

    ACTION_MAINAPP_DONE = 100,
    ACTION_STATUS_CHANGED,
    ACTION_REMOVE,
    ACTION_LAST_APP_DONE,
    ACTION_ARM2_BACKCAR_IN,
    ACTION_ARM2_BACKCAR_OUT,

    ACTION_VOLUMEKEY
} E_APPLY_ACTION;

typedef enum {
    INVALID_STATE_TYPE = 0,
    STATE_BT,
    STATE_CLOCK,
    STATE_LANGUAGE,
    STATE_WIFI,
    STATE_IPOD,
} E_STATE_TYPE;

typedef enum {
    INVALID_BT_STATE = 0,
    BT_CONNECT,
    BT_DISCONNECT,
    BT_ENABLE,
    BT_DISABLE,
} E_BT_STATE;

typedef enum {
    INVALID_CLOCK = 0,
    CLOCK_12,
    CLOCK_24,
} E_CLOCK_STATE;

typedef enum {
    INVALID_LANGUAGE = 0,
    ENGLISH,
    CHINESE,
    CHINESE_TW,
} E_LANGUAGE_STATE;

typedef enum {
    INVALID_IPOD_STATE,
    IPOD_CONNECT,
    IPOD_DISCONNECT,
} E_IPOD_STATE;

typedef enum {
    INVALID_WIFI_STATE = 0,
    WIFI_DISABLE,
    WIFI_ENABLE,
    WIFI_CONNECT,
    WIFI_SIGLV_0,
    WIFI_SIGLV_1,
    WIFI_SIGLV_2,
    WIFI_SIGLV_3,
    WIFI_SIGLV_4,
    WIFI_SIGLV_5,
    WIFI_SIGLV_6,
    WIFI_SIGLV_7,
    WIFI_SIGLV_8,
    WIFI_SIGLV_9,
} E_WIFI_STATE;

bool applyFor (E_APPLY_ACTION action,
                unsigned char appID,
                unsigned int param);
bool applyFor(const CMDPacket &pack);

bool jumpTo(unsigned char fromAppID,
            unsigned char toAppID,
            unsigned char param);

int getState (GlobalBus::E_STATE_TYPE type);
bool setState(E_STATE_TYPE type, int state);

const char *decode(E_APPLY_ACTION action);
}

#endif // GLOBALBUS_H
