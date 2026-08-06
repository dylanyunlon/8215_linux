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

#include <sstream>
#include <dlfcn.h>
#include "applog.h"
#include "apptype.h"
#include "cappobject.h"
#include "csync.h"


using namespace universal_utils;
typedef bool (*runQtSoFile)(const char *qtSOFile);

static const unsigned long APPCTLTIME_MS  = 20000;
static const char TAG[] = "CAPPObject";

CAPPObject::CAPPObject()
    : m_ctlSocket(NULL)
    , m_tagSocket(NULL)
    , m_ctlListener(NULL)
    , m_isOpenRear(false)
    , m_isVideoApp(false)
    , m_isAudioApp(false)
    , m_isAVApp(false)
    , m_isRunning(false)
    , m_priority(APP_PRIORITY_DEFAULT)
    , m_appID(0)
    , m_audioState(LEVEL_NORMAL)
{
    m_pthread_lock = new universal_utils::CMutexObject();
    if (NULL == m_pthread_lock) {
        LOGE(TAG, "new CMutexObject fail\n");
    }
}

CAPPObject::~CAPPObject()
{
    bool ret = false;

    ret = m_pthread_lock->lock();
    if (!ret) {
        LOGE(TAG, "m_pthread_lock->lock fail\n");
    }

    SAFE_DELETE(m_ctlSocket);
    SAFE_DELETE(m_tagSocket);

    ret = m_pthread_lock->unlock();
    if (!ret) {
        LOGE(TAG, "m_pthread_lock->unlock fail\n");
    }

    SAFE_DELETE(m_pthread_lock);
}

bool CAPPObject::notifyTarget(const APPCTLPacket &pack)
{
    bool ret = false;
    int len = -1;

    if (m_ctlSocket != NULL) {
        len = m_ctlSocket->write(&pack, sizeof(pack), m_tagAddrName);
        if (len >= (int)sizeof(pack)) {
            ret = true;
        } else {
            LOGE(TAG, "m_ctlSocket->write fail!\n");
            ret = false;
        }
    } else {
        LOGE(TAG, "m_ctlSocket is NULL\n");
        ret = false;
    }

    return ret;
}

bool CAPPObject::ackController(const APPCTLPacket &pack)
{
    bool ret = false;
    int len = -1;

    if (m_tagSocket != NULL) {
        len = m_tagSocket->write(&pack, sizeof(pack), m_ctlAddrName);
        if (len >= (int)sizeof(pack)) {
            ret = true;
        } else {
            LOGE(TAG, "m_tagSocket->write fail!\n");
            ret = false;
        }
    } else {
        LOGE(TAG, "m_tagSocket is NULL\n");
        ret = false;
    }

    return ret;
}

bool CAPPObject::waitAck(APPCTLPacket &pack)
{
    bool ret = false;
    std::string addr;
    int recvLen = -1;

    if (m_ctlSocket != NULL) {
        recvLen = m_ctlSocket->read(&pack, sizeof(pack), addr, APPCTLTIME_MS);
        if (recvLen >= (int)sizeof(pack)) {
            ret = pack.getParam2() >= 0;
        } else {
            LOGE(TAG, "waitAck from app%d error or time out!\n", getAppID());
            ret = false;
        }
    } else {
        LOGE(TAG, "m_ctlSocket is NULL\n");
        ret = false;
    }

    return ret;
}

bool CAPPObject::stopWaitAck()
{
    bool ret = false;
    int len = -1;
    APPCTLPacket pack;

    pack.ctlType = CTL_ACK;
    pack.setParam1(0);
    pack.setParam2(-1);

    if (m_ctlSocket != NULL) {
        len = m_ctlSocket->write(&pack, sizeof(pack), m_ctlAddrName);
        if (len >= (int)sizeof(pack)) {
            ret = true;
        } else {
            LOGE(TAG, "m_ctlSocket->write fail!\n");
            ret = false;
        }
    } else {
        LOGE(TAG, "m_ctlSocket is NULL\n");
        ret = false;
    }

    return ret;
}

bool CAPPObject::onSockRecv(CUDPSocket *socket)
{
    bool ret = false;
    int result = -1;
    APPCTLPacket pack;
    std::string addr;

    if (socket != NULL) {
        result = socket->read(&pack, sizeof(pack), addr);
        if (-1 == result) {
            LOGE(TAG, "socket->read fail\n");
            ret = false;
        } else {
            ret = true;
        }
    } else {
        LOGE(TAG, "socket is NULL\n");
        ret = false;
    }

    if (ret) {
        ret = onPacket(pack);
        if (!ret) {
            LOGE(TAG, "onPacket fail\n");
        }
    }

    return ret;
}

bool CAPPObject::onPacket(const APPCTLPacket &pack)
{
    bool ret = false;

    if (m_ctlListener != NULL) {
        int ack = -1;
        APPCTLPacket ackPack;

        switch (pack.ctlType) {
        case CTL_EXIT:
            ret = m_pthread_lock->lock();
            if (!ret) {
                LOGE(TAG, "m_pthread_lock->lock fail\n");
            }

            ack = m_ctlListener->onExit(pack.getParam1(),
                                        pack.getParam2());

            ret = m_pthread_lock->unlock();
            if (!ret) {
                LOGE(TAG, "m_pthread_lock->lock fail\n");
            }
            break;
        case CTL_SHOWFRONTUI:
            ack = m_ctlListener->onShowFrontUI();
            break;

        case CTL_HIDEFRONTUI:
            ack = m_ctlListener->onHideFrontUI();
            break;

        case CTL_SHOWFRONT:
            ack = m_ctlListener->onShowFront(pack.getParam1(),
                                            pack.getParam2());
            break;

        case CTL_HIDEFRONT:
            ack = m_ctlListener->onHideFront(pack.getParam1(),
                                            pack.getParam2());
            break;

        case CTL_SHOWREAR:
            ack = m_ctlListener->onShowRear(pack.getParam1(),
                                            pack.getParam2());
            break;

        case CTL_HIDEREAR:
            ack = m_ctlListener->onHideRear(pack.getParam1(),
                                            pack.getParam2());
            break;

        case CTL_REQAUDIO:
            ack = m_ctlListener->onAudioFocusChanged(
                                (CCtlListener::E_AVOUT)pack.getParam1(),
                                (CCtlListener::E_AUDIOFOCUS)pack.getParam2());
            break;

        case CTL_REQVIDEO:
            ack = m_ctlListener->onVideoFocusChanged(
                                (CCtlListener::E_AVOUT)pack.getParam1(),
                                (CCtlListener::E_VIDEOFOCUS)pack.getParam2());
            break;

        case CTL_KEYEVENT:
            ack = (m_ctlListener->onKeyEvent(pack.getParam1(),
                                            pack.getParam2(),
                                            pack.getParam3())) ? 0 : -1;
            break;

        case CTL_WINDOW:
            ack = m_ctlListener->getWindowType();
            break;

        default:
            break;
        }

        //ack ctl
        ackPack.ctlType = CTL_ACK;
        ackPack.setParam1((int)pack.ctlType);
        ackPack.setParam2(ack);
        ret = ackController(ackPack);
        if (!ret) {
            LOGE(TAG, "ackController fail\n");
        }
    } else {
        LOGE(TAG, "m_ctlListener is NULL\n");
        ret = false;
    }

    return ret;
}

bool CAPPObject::setCtlListener(CCtlListener *ctlListener)
{
    bool ret = false;
    int result = -1;

    if (NULL != ctlListener) {
        m_ctlListener = ctlListener;
        SAFE_DELETE(m_tagSocket);
        ret = true;
    } else {
        LOGE(TAG, "ctlListener is NULL\n");
        ret = false;
    }

    if (ret) {
        if (m_tagAddrName.empty()) {
            LOGE(TAG, "m_tagAddrName is empty\n");
            ret = false;
        }

        m_tagSocket = new CUDPSocketProc<CAPPObject>;
        if (NULL == m_tagSocket) {
            LOGE(TAG, "new CUDPSocketProc<CAPPObject> fail\n");
            ret = false;
        }
    }

    if (ret) {
        result = m_tagSocket->bind(m_tagAddrName);
        if (-1 == result) {
            LOGE(TAG, "m_tagSocket->bind fail\n");
            ret = false;
        }

        m_tagSocket->setListener(this, &CAPPObject::onSockRecv);

        result = m_tagSocket->startService();
        if (-1 == result) {
            LOGE(TAG, "m_tagSocket->startService fail\n");
            ret = false;
        }
    }

    //ack ctl
    if (ret) {
        APPCTLPacket ackPack;

        ackPack.ctlType = CTL_ACK;
        ackPack.setParam1((int)CTL_RUN);
        ackPack.setParam2(0);
        ret = ackController(ackPack);
        if (!ret) {
            LOGE(TAG, "ackController fail\n");
            m_tagSocket->stopService();
        }
    }

    return ret;
}

bool CAPPObject::runAPP(int param)
{
    bool ret = false;

    CUDPSocket *applicationSocket = new CUDPSocket;
    if (NULL != applicationSocket) {
        LaunchPacket pack;
        int len = -1;
        std::string addr = MAIN_APPLICATION_SOCKET_ADDR;
        memset(&pack, 0, sizeof(pack));

        pack.size = sizeof(pack);
        pack.cmd = param;
        strcpy(pack.arg, m_exeFile.c_str());

        len = applicationSocket->write(&pack, sizeof(pack), addr);
        if (len >= (int)sizeof(pack)) {
            ret = true;
        } else {
            LOGE(TAG, "socket send fail! len(%d)\n", len);
            ret = false;
        }

        SAFE_DELETE(applicationSocket);
    } else {
        LOGE(TAG, "new CUDPSocket fail\n");
        ret = false;
    }

    return ret;
}

bool CAPPObject::run(int param)
{
    bool ret = false;

    LOGD(TAG, "appobj(%d) run!\r\n", m_appID);

    if (!isRuning()) {
        SAFE_DELETE(m_ctlSocket);
        ret = true;
    } else {
        LOGE(TAG, "app is running, do not run again!\r\n");
        ret = false;
    }

    if (ret) {
        m_ctlSocket = new CUDPSocketProc<CAPPObject>;
        if (NULL == m_ctlSocket) {
            LOGE(TAG, "new CUDPSocketProc<CAPPObject> fail\n");
            ret = false;
        }
    }

    if (ret) {
        int result = m_ctlSocket->bind(m_ctlAddrName);
        if (0 != result) {
            LOGE(TAG, "m_ctlSocket->bind fail\n");
            ret = false;
        }
    }

    if (ret) {
        ret = runAPP(param);
        if (!ret) {
            LOGE(TAG, "runAPP fail\n");
        }
    }

    if (ret) {
        APPCTLPacket ackPack;

        bool ack = waitAck(ackPack);
        if (!ack) {
            LOGE(TAG, "run app %d waitAck fail\n", m_appID);
        } else {
            m_isRunning = true;
        }
    }

    return ret;
}

unsigned char CAPPObject::getAppID() const
{
    return m_appID;
}

bool CAPPObject::setAppID(unsigned char id)
{
    std::stringstream str;
    m_appID = id;

    str << "/tmp/ctlappobjId" << (int)id;
    m_ctlAddrName = str.str();
    str.str("");
    str.clear();
    str << "/tmp/tagappobjId" << (int)id;
    m_tagAddrName = str.str();

    return true;
}

const char *CAPPObject::getExefile(void) const
{
    return m_exeFile.c_str();
}

bool CAPPObject::setExeFile(const char *file)
{
    bool ret = false;

    if (file != NULL) {
        m_exeFile = file;
        ret = true;
    }

    return ret;
}

bool CAPPObject::setAPPName(const char *file)
{
    bool ret = false;

    if (NULL != file) {
        m_appName = file;
        ret = true;
    }

    return ret;
}

const char *CAPPObject::getAPPName(void) const
{
    return m_appName.c_str();
}

bool CAPPObject::exit()
{
    bool ret = false;
    APPCTLPacket pack;

    LOGD(TAG, "APP %s exit\n", m_appName.c_str());

    pack.ctlType = CTL_EXIT;
    ret = notifyTarget(pack);

    if (ret) {
        ret = waitAck(pack);
        if (ret) {
            m_isRunning = false;
        } else {
            LOGE(TAG, "exit app %d waitAck fail\n", m_appID);
        }
    }

    return ret;
}

bool CAPPObject::showFrontUI()
{
    bool ret = false;
    APPCTLPacket pack;

    LOGD(TAG, "%s showFrontUI\n", m_appName.c_str());

    pack.ctlType = CTL_SHOWFRONTUI;
    ret = notifyTarget(pack);

    if (ret) {
        ret = waitAck(pack);
    }

    return ret;
}

bool CAPPObject::hideFrontUI()
{
    bool ret = false;
    APPCTLPacket pack;

    LOGD(TAG, "%s hideFrontUI\n", m_appName.c_str());

    pack.ctlType = CTL_HIDEFRONTUI;
    ret = notifyTarget(pack);

    if (ret) {
        ret = waitAck(pack);
    }

    return ret;
}

bool CAPPObject::showFront(int param)
{
    bool ret = false;
    APPCTLPacket pack;
    LOGD(TAG, "%s param(%d) showFront!\r\n", m_appName.c_str(), param);

    pack.ctlType = CTL_SHOWFRONT;
    pack.setParam1(param);
    ret = notifyTarget(pack);

    if (ret) {
        ret = waitAck(pack);
    }

    return ret;
}

bool CAPPObject::hideFront()
{
    bool ret = false;
    APPCTLPacket pack;

    LOGD(TAG, "%s hideFront!\r\n", m_appName.c_str());

    pack.ctlType = CTL_HIDEFRONT;
    ret = notifyTarget(pack);

    if (ret) {
        ret = waitAck(pack);
    }

    return ret;
}

bool CAPPObject::showRear()
{
    bool ret = false;
    APPCTLPacket pack;

    LOGD(TAG, "appobj(%d) hideFront!\r\n", m_appID);

    pack.ctlType = CTL_SHOWREAR;
    ret = notifyTarget(pack);

    if (ret) {
        ret = waitAck(pack);
    }

    if (ret) {
        m_isOpenRear = true;
    }

    return ret;
}

bool CAPPObject::hideRear()
{
    bool ret = false;
    APPCTLPacket pack;

    LOGD(TAG, "appobj(%d) hideRear!\r\n", m_appID);

    pack.ctlType = CTL_HIDEREAR;
    ret = notifyTarget(pack);

    if (ret) {
        ret = waitAck(pack);
    }

    if (ret) {
        m_isOpenRear = false;
    }

    return ret;
}

bool CAPPObject::sendAudioFocusResult(CCtlListener::E_AVOUT aOut,
                                        CCtlListener::E_AUDIOFOCUS onFocue)
{
    bool ret = false;
    APPCTLPacket pack;

    LOGD(TAG, "appobj(%d) sendAudioFocusResult aOut(%s) Focuse(%s)!\r\n",
            m_appID,
            CCtlListener::decode(aOut),
            CCtlListener::decode(onFocue));

    pack.ctlType = CTL_REQAUDIO;
    pack.setParam1((int)aOut);
    pack.setParam2((int)onFocue);
    ret = notifyTarget(pack);

    if (ret) {
        ret = waitAck(pack);
    }

    return ret;
}

//todo:set data and get data use different decorder from different messages 
bool CAPPObject::requestVideoFocus(CCtlListener::E_AVOUT vOut,
                                        CCtlListener::E_VIDEOFOCUS onFocue)
{
    bool ret = false;
    APPCTLPacket pack;

    LOGD(TAG, "appobj(%d) requestVideoFocus vOut(%s) Focuse(%s)!\r\n",
            m_appID,
            CCtlListener::decode(vOut),
            CCtlListener::decode(onFocue));

    pack.ctlType = CTL_REQVIDEO;
    pack.setParam1((int)vOut);
    pack.setParam2((int)onFocue);
    ret = notifyTarget(pack);

    if (ret) {
        ret = waitAck(pack);
    }

    return ret;
}

bool CAPPObject::keyEvent(int key, int param1, int param2, bool sync)
{
    bool ret = false;
    APPCTLPacket pack;
    LOGD(TAG, "appobj(%d) keyEvent key(%x) param1(%d) param2(%d) sync(%d)!\r\n",
        m_appID,
        key,
        param1,
        param2,
        sync);

    pack.ctlType = CTL_KEYEVENT;
    pack.setParam1(key);
    pack.setParam2(param1);
    pack.setParam3(param2);
    ret = notifyTarget(pack);

    if (ret) {
        ret = waitAck(pack);
    }

    return ret;
}

bool CAPPObject::isFloatingWindow()
{
    bool ret = false;
    APPCTLPacket pack;

    pack.ctlType = CTL_WINDOW;
    ret = notifyTarget(pack);

    if (ret) {
        ret = waitAck(pack);
    }

    return (ret ? (pack.getParam2() == 1) : ret);
}

bool CAPPObject::isOpenRear() const
{
    return m_isOpenRear;
}

bool CAPPObject::isVideoApp() const
{
    return m_isVideoApp;
}

bool CAPPObject::setVideoApp(bool isVideo)
{
    m_isVideoApp = isVideo;

    return true;
}

bool CAPPObject::isAudioApp() const
{
    return m_isAudioApp;
}

bool CAPPObject::setAudioApp(bool isAudio)
{
    m_isAudioApp = isAudio;

    return true;
}

bool CAPPObject::isAVApp() const
{
    return m_isAVApp;
}

bool CAPPObject::setAVApp(bool isAVApp)
{
    m_isAVApp = isAVApp;

    return true;
}

bool CAPPObject::needResume() const
{
    return m_needResume;
}

bool CAPPObject::setNeedResume(bool needResume)
{
    m_needResume = needResume;
    return true;
}

const std::string &CAPPObject::getShareProcess() const
{
    return m_shareProcess;
}

bool CAPPObject::setShareProcess(const std::string &name)
{
    m_shareProcess = name;

    return true;
}

CAPPObject::E_PARAMETER CAPPObject::getAudioState() const
{
    return m_audioState;
}

bool CAPPObject::setAudioState(E_PARAMETER audioState)
{
    m_audioState = audioState;
    return true;
}

bool CAPPObject::setPriority(int priority)
{
    m_priority = priority;

    return true;
}

int CAPPObject::getPriority() const
{
    return m_priority;
}

bool CAPPObject::isRuning() const
{
    return m_isRunning;
}

