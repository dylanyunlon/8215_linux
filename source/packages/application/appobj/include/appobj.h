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

#ifndef APPOBJ_H
#define APPOBJ_H

#include <string>
#include "ctllistener.h"

class CAPPBaseObj
{
public:
    //type define.
    typedef enum
    {
        APPID_INVALID = 0,
        APPID_HOME = 1,
        APPID_MMP_VIDEO = 2,
        APPID_MMP_AUDIO = 3,
        APPID_MMP_PIC = 4,
        APPID_AVIN = 5,
        APPID_BT = 6,
        APPID_DVP = 7,
        APPID_BACKCAR = 8,
        APPID_MHL = 9,
        APPID_RAIDO = 10,
        APPID_SETTING = 11,
        APPID_FILE_BROWSER = 12,
        APPID_NAVI          = 15,
        APPID_IPOD          = 17,
        APPID_MIRACAST      = 18,
        APPID_DVR           = 19,
        APPID_GPS           = 20,
        APPID_BTSPP         = 22,
        APPID_MMP_AUDIO2    = 25,
        APPID_MMP_VIDEO2    = 26,
        APPID_BCM           = 27,
        APPID_RECOVERY   = 30,
        APPID_CARPLAY_SETTINGS = 34,
        APPID_CARPLAY_APP   = 35,
        APPID_ANDROIDAUTO_SETTINGS = 36,
        APPID_ANDROIDAUTO_APP = 37,
        APPID_CARPLAY_HELPER = 38,
        APPID_CLUSTER = 39,
        APPID_CARBIT_APP = 40,
        APPID_BTPHONE = 106,
        APPID_BTEM,
    } E_APPID;

    typedef enum
    {
        APP_PRIORITY_MAX = 0,
        APP_PRIORITY_DEFAULT = 10,
    } E_APP_PRIORITY;

    typedef enum: unsigned int {
        // for audio,the higher 3 to 5 bits of auido data(32-bit)are used to indicate the focus type 
        LEVEL_NORMAL = 0,
        LEVEL_TRANSIENT = 1u << 27,
        LEVEL_TRANSIENT_CAN_DUCK = 2u << 27,
        // for show page
        PAGE1 = 3,
        PAGE2,
        PAGE3,
        PAGE4,
        PAGE5,
        PAGE6,
        //for ketValue
        LEVEL_VOLUMEKEY
    } E_PARAMETER;

    typedef enum: unsigned int {
        // for audio,the higher 6 to 9 bits of auido data(32-bit)are used to indicate the stream type 
        STREAM_MUSIC = 0,
        STREAM_VOICL_CALL = 3u << 23,
        STRAEM_NOTIFICATION_RINGTONE = 1u << 23,
        STREAM_ASSISTANT = 2u << 23
    }E_STREAMTYPE;

    //interface.
    CAPPBaseObj(void){}
    virtual ~CAPPBaseObj(void){}
    virtual unsigned char getAppID(void) const = 0;
    virtual bool setAppID(unsigned char id) = 0;
    virtual const char *getExefile(void) const = 0;
    virtual bool setExeFile(const char *file) = 0;
    virtual bool setAPPName(const char *file) = 0;
    virtual const char *getAPPName(void) const = 0;
};

class CAPPTargetObj : public CAPPBaseObj
{
public:
    CAPPTargetObj(void){}
    virtual ~CAPPTargetObj(void){}
    virtual bool setCtlListener(CCtlListener *ctlListener) = 0;
};

class CAPPControllerObj : public CAPPBaseObj
{
public:
    //type define

    //interface.
    CAPPControllerObj(void){}
    virtual ~CAPPControllerObj(void){}
    virtual bool exit() = 0;
    virtual bool showFrontUI(void) = 0;
    virtual bool hideFrontUI(void) = 0;
    virtual bool showFront(int param = LEVEL_NORMAL) = 0;
    virtual bool hideFront() = 0;
    virtual bool showRear() = 0;
    virtual bool hideRear() = 0;
    virtual bool sendAudioFocusResult(CCtlListener::E_AVOUT aOut,
                                        CCtlListener::E_AUDIOFOCUS onFocue) = 0;
    virtual bool requestVideoFocus(CCtlListener::E_AVOUT vOut,
                                        CCtlListener::E_VIDEOFOCUS onFocue) = 0;
    virtual bool keyEvent(int key, int param1, int param2, bool sync) = 0;
    static int makeKey(unsigned char mainFunc, unsigned char subFunc)
    {
        return ((int)mainFunc << 16 | subFunc);
    }
    virtual bool isFloatingWindow() = 0;

    virtual bool run(int param = 0) = 0;
    virtual bool isOpenRear() const = 0;

    virtual bool isVideoApp() const = 0;
    virtual bool setVideoApp(bool isVideo) = 0;
    virtual bool isAudioApp() const = 0;
    virtual bool setAudioApp(bool isAudio) = 0;
    virtual bool isAVApp() const = 0;
    virtual bool setAVApp(bool isAVApp) = 0;

    virtual bool needResume() const = 0;
    virtual bool setNeedResume(bool needResume) = 0;

    virtual const std::string &getShareProcess() const= 0;
    virtual bool setShareProcess(const std::string &name) = 0;

    virtual CAPPBaseObj::E_PARAMETER getAudioState() const = 0;
    virtual bool setAudioState(E_PARAMETER audioState) = 0;

    virtual bool setPriority(int priority) = 0;
    virtual int getPriority() const = 0;

    virtual bool isRuning() const = 0;
};

#endif // APPOBJ_H
