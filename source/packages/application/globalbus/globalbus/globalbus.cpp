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

#include "globalbus.h"
#include "cbusobj.h"

static CBusObj g_busObj;
//define for ipod global bus
#define IPOD_STATUS_DISCONNECT 1
#define IPOD_STATUS_CONNECTED 2
namespace GlobalBus
{
    bool applyFor(E_APPLY_ACTION action,
                    unsigned char appID,
                    unsigned int param)
    {
        return g_busObj.applyFor((unsigned char)action, appID, param);
    }

    bool applyFor(const CMDPacket &pack)
    {
         return g_busObj.applyFor(pack);
    }

    bool jumpTo(unsigned char fromAppID,
                unsigned char toAppID,
                unsigned char param)
    {
        return g_busObj.jumpTo(fromAppID, toAppID, param);
    }

    int getState(GlobalBus::E_STATE_TYPE type)
    {
        return g_busObj.getState(type);
    }

    bool setState(E_STATE_TYPE type, int state)
    {
        return g_busObj.setState(type, state);
    }

    const char *decode(E_APPLY_ACTION action)
    {
        const char *res = NULL;

        switch (action) {
        case ACTION_RUN:
            res = "ACTION_RUN";
            break;
        case ACTION_EXIT:
            res = "ACTION_EXIT";
            break;
        case ACTION_SHOWFRONT:
            res = "ACTION_SHOWFRONT";
            break;
        case ACTION_HIDEFRONT:
            res = "ACTION_HIDEFRONT";
            break;
        case ACTION_SHOWREAR:
            res = "ACTION_SHOWREAR";
            break;
        case ACTION_HIDEREAR:
            res = "ACTION_HIDEREAR";
            break;
        case ACTION_AUDIO_REQ:
            res = "ACTION_AUDIO_REQ";
            break;
        case ACTION_AUDIO_REL:
            res = "ACTION_AUDIO_REL";
            break;
        case ACTION_VIDEO_REQ:
            res = "ACTION_VIDEO_REQ";
            break;
        case ACTION_VIDEO_REL:
            res = "ACTION_VIDEO_REL";
            break;
        case ACTION_AV_REQ:
            res = "ACTION_AV_REQ";
            break;
        case ACTION_AV_REL:
            res = "ACTION_AV_REL";
            break;
        case ACTION_GOHOME:
            res = "ACTION_GOHOME";
            break;

        case ACTION_DUPRUN:
            res = "ACTION_DUPRUN";
            break;
        case ACTION_OUTSIDE_AUDIO_REQ:
            res = "ACTION_TRANSIENT_AUDIO_REQ";
            break;
        case ACTION_OUTSIDE_AUDIO_REL:
            res = "ACTION_TRANSIENT_AUDIO_REL";
            break;

        case ACTION_MAINAPP_DONE:
            res = "ACTION_MAINAPP_DONE";
            break;
        case ACTION_STATUS_CHANGED:
            res = "ACTION_STATUS_CHANGED";
            break;
        case ACTION_REMOVE:
            res = "ACTION_REMOVE";
            break;
        default:
            res = "not support";
            break;
        }

        return res;
    }
}

extern "C" void setIPodbarState(int state)
{
    if(state == IPOD_STATUS_CONNECTED) {
        GlobalBus::setState(GlobalBus::STATE_IPOD, GlobalBus::IPOD_CONNECT);
    } else if (state == IPOD_STATUS_DISCONNECT){
        GlobalBus::setState(GlobalBus::STATE_IPOD, GlobalBus::IPOD_DISCONNECT);
    }else {
        //Do not care
    }
}
