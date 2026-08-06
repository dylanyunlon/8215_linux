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
#include "appobj.h"
#include "audiofocusrequestdata.h"
#include "ctllistener.h"

AudioFocusRequestData::AudioFocusRequestData(unsigned int data)
    :m_data(data){}

unsigned int AudioFocusRequestData::getOutput()
{
    return m_data & 0xC0000000;
}

unsigned int AudioFocusRequestData::getFocusType()
{
    return m_data & 0x38000000;
}

unsigned int AudioFocusRequestData::getStreamType()
{
    return m_data & 0x07800000;
}

const char* AudioFocusRequestData::decodeOutput(unsigned int output)
{
    const char *ret;
    switch (output) {
        case CCtlListener::AVOUT_F:
            ret = "front";
            break;
        case CCtlListener::AVOUT_R:
            ret = "rear";
            break;
        case CCtlListener::AVOUT_FR:
            ret = "front and rear";
            break;
        default:
            ret = "unknow";
            break;
    }
    return ret;
}

const char* AudioFocusRequestData::decodeFocusType(unsigned int focusType)
{
    const char *ret;
    switch (focusType) {
        case CAPPBaseObj::LEVEL_NORMAL:
            ret = "normal";
            break;
        case CAPPBaseObj::LEVEL_TRANSIENT:
            ret = "transient";
            break;
        case CAPPBaseObj::LEVEL_TRANSIENT_CAN_DUCK:
            ret = "transient can duck";
            break;
        default:
            ret = "unknow";
            break;
    }
    return ret;
}

const char* AudioFocusRequestData::decodeStreamType(unsigned int streamtype)
{
    const char *ret;
    switch (streamtype) {
        case CAPPBaseObj::STREAM_MUSIC:
            ret = "music";
            break;
        case CAPPBaseObj::STREAM_VOICL_CALL:
            ret = "voice call";
            break;
        case CAPPBaseObj::STRAEM_NOTIFICATION_RINGTONE:
            ret = "notification ringtone";
            break;
        case CAPPBaseObj::STREAM_ASSISTANT:
            ret = "assistant";
            break;
        default:
            ret = "unknow";
            break;
    }
    return ret;
}