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

#ifndef CTLLISTENER_H
#define CTLLISTENER_H

class CCtlListener
{
public:
    //AV Output, front, rear or front & rear.
    //the front or back 
    typedef enum : unsigned int{
        AVOUT_F = 0,
        AVOUT_R = 1u << 30,
        AVOUT_FR = 2u << 30,
    } E_AVOUT;

    static const unsigned char AVOUT_MASK = 0xc0;

    typedef enum
    {
        //get focus;
        AUDIOFOCUS_GAIN = 0,

        AUDIOFOCUS_GAIN_TRANSIENT,

        //loss focus, it maybe loss for a long time,so not only to stop a playback,
        //the best release Media resources.
        AUDIOFOCUS_LOSS,

        //loss focus, will be get focus again soon. If can pause, then pause, can not, then stop
        AUDIOFOCUS_LOSS_TRANSIENT,

        //loss focus, but you can continue to play, but you better to lower the volume.
        AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK,

        AUDIOFOCUS_NONE,

        AUDIOFCOUS_REQUEST_FAILED
    } E_AUDIOFOCUS;

    typedef enum
    {
        //get focus;
        VIDEOFOCUS_GAIN = 0,

        //loss focus, it maybe loss for a long time,so not only to stop video path,
        //the best release Media resources.
        VIDEOFOCUS_LOSS,

        //loss focus, will be get focus again soon. If can pause, then pause, can not, then stop
        VIDEOFOCUS_LOSS_TRANSIENT,

        //loss focus, but you can continue to play, but you better to hight video.
        VIDEOFOCUS_LOSS_TRANSIENT_CAN_DUCK,
    } E_VIDEOFOCUS;

    typedef enum
    {
        VOLUME_MEDIA = 0,
        VOLUME_GIS,
        VOLUME_BT,
    } E_VOLUMETYPE;

    CCtlListener(void){}
    virtual ~CCtlListener(void){}

    virtual int onExit(int param1, int param2) = 0;
    virtual int onShowFrontUI(void) = 0;
    virtual int onHideFrontUI(void) = 0;
    virtual int onShowFront(int param1, int param2) = 0;
    virtual int onHideFront(int param1, int param2) = 0;
    virtual int onShowRear(int param1, int param2) = 0;
    virtual int onHideRear(int param1, int param2) = 0;
    virtual int onAudioFocusChanged(CCtlListener::E_AVOUT aOut,
                                        CCtlListener::E_AUDIOFOCUS focus) = 0;
    virtual int onVideoFocusChanged(CCtlListener::E_AVOUT vOut,
                                        CCtlListener::E_VIDEOFOCUS focus) = 0;
    virtual bool onKeyEvent(int key, int param1, int param2) = 0;
    static unsigned char getMainFunc(int key)
    {
        return (unsigned char)(key >> 16);
    }
    static unsigned char getSubFunc(int key)
    {
        return (unsigned char)(key & 0xff);
    }

    virtual int getWindowType() const
    {
        return 0;
    }

    static const char *decode(E_AVOUT code)
    {
        const char *ret;

        switch (code) {
        case AVOUT_F:
            ret = "AVOUT Front";
            break;
        case AVOUT_R:
            ret = "AVOUT Rear";
            break;
        default :
            ret = "No such E_AVOUT code!";
        }

        return ret;
    }

    static const char *decode(E_AUDIOFOCUS code)
    {
        const char *ret;

        switch (code) {
        case AUDIOFOCUS_GAIN:
            ret = "Audio gain focus";
            break;
        case AUDIOFOCUS_LOSS:
            ret = "Audio loss focus";
            break;
        case AUDIOFOCUS_LOSS_TRANSIENT:
            ret = "Audio loss transient";
            break;
        case AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
            ret = "Audio loss transient can duck";
            break;
        default :
            ret = "No such E_AUDIOFOCUS code!";
        }

        return ret;
    }

    static const char *decode(E_VIDEOFOCUS code)
    {
        const char *ret;

        switch (code) {
        case VIDEOFOCUS_GAIN:
            ret = "Video gain focus";
            break;
        case VIDEOFOCUS_LOSS:
            ret = "Video loss focus";
            break;
        case VIDEOFOCUS_LOSS_TRANSIENT:
            ret = "Video loss transient";
            break;
        case VIDEOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
            ret = "Video loss transient can duck";
            break;
        default :
            ret = "No such E_VIDEOFOCUS code!";
        }

        return ret;
    }
};

#endif // CTLLISTENER_H
