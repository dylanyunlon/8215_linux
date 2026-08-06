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

#ifndef CAPPOBJECT_H
#define CAPPOBJECT_H

#include <string>
#include <stddef.h>
#include <iostream>
#include "cudpsocketproc.h"
#include "csync.h"
#include "appobj.h"

#define APPCTLPACKETDATALEN 16

class CAPPObject : public CAPPControllerObj, public CAPPTargetObj
{
public:
    typedef enum
    {
        CTL_RUN = 1,
        CTL_EXIT,
        CTL_SHOWFRONTUI,
        CTL_HIDEFRONTUI,
        CTL_SHOWFRONT,
        CTL_HIDEFRONT,
        CTL_SHOWREAR,
        CTL_HIDEREAR,
        CTL_REQAUDIO,
        CTL_REQVIDEO,
        CTL_KEYEVENT,
        CTL_WINDOW,

        CTL_ACK,
    } E_CTLCMD;

    class APPCTLPacket
    {
    public:
        APPCTLPacket() : ctlType(0)
        {
            memset(data, 0, sizeof(data));
        }

        int getParam1() const
        {
            return ((data[3]<<24) | (data[2]<<16) | (data[1]<<8) | data[0]);
        }

        int getParam2() const
        {
            return ((data[7]<<24) | (data[6]<<16) | (data[5]<<8) | data[4]);
        }

        int getParam3() const
        {
            return ((data[11]<<24) | (data[10]<<16) | (data[9]<<8) | data[8]);
        }

        int getParam4() const
        {
            return ((data[15]<<24) | (data[14]<<16) | (data[13]<<8) | data[12]);
        }

        void setParam1(int param)
        {
            data[0] = param & 0xFF;
            data[1] = (param >> 8) & 0xFF;
            data[2] = (param >> 16) & 0xFF;
            data[3] = (param >> 24) & 0xFF;
        }

        void setParam2(int param)
        {
            data[4] = param & 0xFF;
            data[5] = (param >> 8) & 0xFF;
            data[6] = (param >> 16) & 0xFF;
            data[7] = (param >> 24) & 0xFF;
        }

        void setParam3(int param)
        {
            data[8] = param & 0xFF;
            data[9] = (param >> 8) & 0xFF;
            data[10] = (param >> 16) & 0xFF;
            data[11] = (param >> 24) & 0xFF;
        }

        void setParam4(int param)
        {
            data[12] = param & 0xFF;
            data[13] = (param >> 8) & 0xFF;
            data[14] = (param >> 16) & 0xFF;
            data[15] = (param >> 24) & 0xFF;
        }

        unsigned long ctlType;
        unsigned char data[APPCTLPACKETDATALEN];
    };

    CAPPObject();
    virtual ~CAPPObject();

    virtual unsigned char getAppID() const;
    virtual bool setAppID(unsigned char id);
    virtual const char *getAPPName(void) const;
    virtual bool setAPPName(const char *file);
    virtual const char *getExefile(void) const;
    virtual bool setExeFile(const char *file);

    virtual bool setCtlListener(CCtlListener *ctlListener);

    virtual bool run(int param = 0);
    virtual bool exit();
    virtual bool showFrontUI(void);
    virtual bool hideFrontUI(void);
    virtual bool showFront(int param);
    virtual bool hideFront();
    virtual bool showRear();
    virtual bool hideRear();
    virtual bool sendAudioFocusResult(CCtlListener::E_AVOUT aOut,
                                    CCtlListener::E_AUDIOFOCUS onFocue);
    virtual bool requestVideoFocus(CCtlListener::E_AVOUT vOut,
                                    CCtlListener::E_VIDEOFOCUS onFocue);
    virtual bool keyEvent(int key, int param1, int param2, bool sync);
    virtual bool isFloatingWindow();

    virtual bool isOpenRear() const;

    virtual bool isVideoApp() const;
    virtual bool setVideoApp(bool isVideo);
    virtual bool isAudioApp() const;
    virtual bool setAudioApp(bool isAudio);
    virtual bool isAVApp() const;
    virtual bool setAVApp(bool isAVApp);

    virtual bool needResume() const;
    virtual bool setNeedResume(bool needResume);

    virtual const std::string &getShareProcess() const;
    virtual bool setShareProcess(const std::string &name);

    virtual CAPPBaseObj::E_PARAMETER getAudioState() const;
    virtual bool setAudioState(E_PARAMETER audioState);

    virtual bool setPriority(int priority);
    virtual int getPriority() const;
    virtual bool isRuning() const;

protected:
    virtual bool runAPP(int param = 0);
    virtual bool notifyTarget(const APPCTLPacket &pack);
    virtual bool waitAck(APPCTLPacket &pack);
    virtual bool ackController(const APPCTLPacket &pack);
    virtual bool stopWaitAck();
    bool onPacket(const APPCTLPacket &pack);
    bool onSockRecv(CUDPSocket *socket);

protected:
    CUDPSocketProc<CAPPObject> *m_ctlSocket;
    CUDPSocketProc<CAPPObject> *m_tagSocket;
    std::string m_ctlAddrName;
    std::string m_tagAddrName;
    CCtlListener *m_ctlListener;
    bool m_isOpenRear;
    bool m_isVideoApp;
    bool m_isAudioApp;
    bool m_isAVApp;
    bool m_needResume;
    std::string m_shareProcess;

    bool m_isRunning;
    int m_priority;
    universal_utils::CMutexObject *m_pthread_lock;// make sure destructor run after exit

    unsigned char m_appID;
    std::string m_appName;
    std::string m_exeFile;
    E_PARAMETER m_audioState;
};

#endif // CAPPOBJECT_H
