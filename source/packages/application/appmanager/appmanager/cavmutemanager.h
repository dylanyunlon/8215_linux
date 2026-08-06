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

#ifndef CAVMUTEMANAGER_H
#define CAVMUTEMANAGER_H

#include <list>
#include <utility>
#include <unistd.h>
#include "cmanager.h"
#include "cobjfactory.h"
#include "unilist.h"
#include "memorywatcher.h"
//#include "apprecord.h"

class CAVMuteManager : public CManager, public universal_utils::CAutoThread
{
public:
    CAVMuteManager(CObjFactory *pFactory);
    ~CAVMuteManager();

    bool start();
    bool onDupRun(unsigned char appID, unsigned int data);
    bool onRun(unsigned char appID, unsigned int data);
    bool onExit(unsigned char appID, unsigned int data);
    bool onRemove(unsigned char appID, unsigned int data);
    bool onShowFront(unsigned char appID, unsigned int data);
    bool onHideFront(unsigned char appID, unsigned int data);
    bool onShowRear(unsigned char appID, unsigned int data);
    bool onHideRear(unsigned char appID, unsigned int data);
    bool onVideoReq(unsigned char appID, unsigned int data);
    bool onVideoRel(unsigned char appID, unsigned int data);
    bool onAudioReq(unsigned char appID, unsigned int data);
    bool onAudioRel(unsigned char appID, unsigned int data);
    bool onAVReq(unsigned char appID, unsigned int data);
    bool onAVRel(unsigned char appID, unsigned int data);
    bool onGoHome(unsigned char appID, unsigned int data);
    bool processAutoTestCmd(const CCmdTask &cmdTask);
    bool processAppJump(const CCmdTask &cmdTask);
    bool notifyAllApp(const CCmdTask &cmdTask);
    bool onMiscRequest(const CCmdTask &cmdTask);
    bool onKeyEvent(const CCmdTask &cmdTask);
    bool judgeAppHide(CAPPControllerObj *operObj, CAPPControllerObj *activeObj);

    struct AudiofocusStackItem {
        unsigned char appID = 0;
        unsigned int streamType = 0;
        unsigned int focusType = 0;
        AudiofocusStackItem(int appID, int streamType, int focusType) 
            : appID(appID), streamType(streamType), focusType(focusType) {}
    };

    protected:
    unsigned long threadRun();

private:
    bool onStartRun(unsigned char appID);
    bool sendCmdToAPP(CAPPControllerObj *targetAppObj,
                                        const CCmdTask &cmdTask);
    bool processAutoTestCmdBackCar(const CCmdTask &cmdTask);
    bool processSubFuncKey(const CCmdTask &cmdTask);

    bool removeApp(unsigned char appID);
    void exitAllApp();

    bool addApp(CAPPControllerObj *appObj);
    bool rmApp(unsigned char appID);

    CAPPControllerObj *getAppObj(unsigned char appID) const;  //get from m_appObjList.
    CAPPControllerObj *newAppObj(unsigned char appID) const;
    bool releaseAppObj(CAPPControllerObj *appObj);

    bool isRunning(unsigned char appID) const;

    bool setActiveApp(unsigned char appID);
    CAPPControllerObj *getActiveApp() const;
    CAPPControllerObj *getNoBtPhoneActiveApp() const;
    CAPPControllerObj *searchNoBtPhoneActiveApp(unsigned char &appID) const;
    CAPPControllerObj *getNextActiveApp(unsigned char appID);
    bool cleanActiveApp();
    bool setActiveAppInline(unsigned char appID);
    bool rmActiveApp(unsigned char appID);

    bool lossVideo(unsigned char appID, unsigned char param);
    bool gainVideo(unsigned char appID, unsigned char param);
    bool resumeVideoApp(unsigned char appID, unsigned char param);

    bool analogBackcarSendSignalThread();
    void initAnalogBackcarSendSignalThread(void);

    CAPPControllerObj *getPreActiveApp() const;
    void setPreActiveApp(CAPPControllerObj* preActiveApp);

    list<unsigned char> m_bootList_be;  // boot list for APPs before ACTION_ARM2_BACKCAR_OUT
    list<unsigned char> m_bootList_af;  // boot list for APPs after ACTION_ARM2_BACKCAR_OUT
    list<unsigned char> m_bootList;
    list<unsigned char> m_startList;
    list<CAPPControllerObj *> m_appObjList;
    list<unsigned char> m_actionList;  //current show app list.

    unsigned char m_rearShowApp;
    //unsigned char m_rearAudioApp;

    std::list<AudiofocusStackItem>m_audiofocusStack;

    Unilist<unsigned char> m_frontVideoRes;

    CObjFactory *m_objFactory;
    static pid_t m_pid;
    //int m_outsideAudioCount;
    const std::string m_mode;

    //MemoryWatcherClient *m_watcher;
    //AppRecord m_record;

    universal_utils::SingleThread<CAVMuteManager> m_initThread;

    CAPPControllerObj *m_preActiveApp;
};

#endif
