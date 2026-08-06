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

#include "bluetoothapplication.h"
#include <unistd.h>
#include "bluetoothhidcallback.h"

using namespace std;
static const char* const tag = "CBluetoothApplication";

CBluetoothApplication::CBluetoothApplication()
    : CQObjListener(CAPPBaseObj::APPID_BT)
    , m_bluetoothCallPage(NULL)
    , m_bluetoothDialPage(NULL)
    , m_bluetoothCallRecordsBooksPage(NULL)
    , m_bluetoothMusicPage(NULL)
    , m_bluetoothPairRecordsPage(NULL)
    , m_bluetoothSettingPage(NULL)
    , m_bluetoothGAPCallBack(NULL)
    , m_bluetoothHFPCallBack(NULL)
    , m_bluetoothPBAPCallBack(NULL)
    , m_bluetoothAVRCPCallBack(NULL)
    , m_bluetoothA2DPCallBack(NULL)
    , m_bluetoothFileSync(NULL)
    , m_bluetoothCallPageIsShow(false)
    , m_phoneConnectState(false)
    , m_mediaConnectState(false)
    , m_clock24IsActive(true)
    , m_currentPageIndex(PAIRRECORD_PAGE_INDEX)
    , m_client(NULL)
    , m_localDevice(NULL)
    , m_gapcallback(NULL)
    , m_hfpInterface(NULL)
    , m_pbapInterface(NULL)
    , m_avrcpInterface(NULL)
    , m_a2dpInterface(NULL)
    , m_app(NULL)
    , m_translator(NULL)
    , m_languageState(GlobalBus::INVALID_LANGUAGE)
    , m_bluetoothHIDCallBack(NULL)
    , m_hidInterface(NULL)
{

}

CBluetoothApplication::CBluetoothApplication(CBluetoothCallPage *bluetoothCallPage,
    CBluetoothDialPage *bluetoothDialPage,
    CBluetoothCallRecordsBooksPage *bluetoothCallRecordsBooksPage,
    CBluetoothMusicPage *bluetoothMusicPage,
    CBluetoothPairRecordsPage *bluetoothPairRecordsPage,
    CBluetoothSettingPage *bluetoothSettingPage)
    : CQObjListener(CAPPBaseObj::APPID_BT)
    , m_bluetoothCallPage(bluetoothCallPage)
    , m_bluetoothDialPage(bluetoothDialPage)
    , m_bluetoothCallRecordsBooksPage(bluetoothCallRecordsBooksPage)
    , m_bluetoothMusicPage(bluetoothMusicPage)
    , m_bluetoothPairRecordsPage(bluetoothPairRecordsPage)
    , m_bluetoothSettingPage(bluetoothSettingPage)
    , m_bluetoothGAPCallBack(NULL)
    , m_bluetoothHFPCallBack(NULL)
    , m_bluetoothPBAPCallBack(NULL)
    , m_bluetoothAVRCPCallBack(NULL)
    , m_bluetoothA2DPCallBack(NULL)
    , m_bluetoothFileSync(NULL)
    , m_bluetoothCallPageIsShow(false)
    , m_phoneConnectState(false)
    , m_mediaConnectState(false)
    , m_clock24IsActive(true)
    , m_currentPageIndex(PAIRRECORD_PAGE_INDEX)
    , m_client(NULL)
    , m_localDevice(NULL)
    , m_gapcallback(NULL)
    , m_hfpInterface(NULL)
    , m_pbapInterface(NULL)
    , m_avrcpInterface(NULL)
    , m_a2dpInterface(NULL)
    , m_app(NULL)
    , m_translator(NULL)
    , m_languageState(GlobalBus::INVALID_LANGUAGE)
{

}


CBluetoothApplication::~CBluetoothApplication()
{
    LOGD(tag, "[CBluetoothApplication] destructor\n");

    if (NULL != m_bluetoothGAPCallBack) {
        delete m_bluetoothGAPCallBack;
        m_bluetoothGAPCallBack = NULL;
    }
    if (NULL != m_bluetoothHFPCallBack) {
        delete m_bluetoothHFPCallBack;
        m_bluetoothHFPCallBack = NULL;
    }
    if (NULL != m_bluetoothPBAPCallBack) {
        delete m_bluetoothPBAPCallBack;
        m_bluetoothPBAPCallBack = NULL;
    }
    if (NULL != m_bluetoothAVRCPCallBack) {
        delete m_bluetoothAVRCPCallBack;
        m_bluetoothAVRCPCallBack = NULL;
    }
    if (NULL != m_bluetoothA2DPCallBack) {
        delete m_bluetoothA2DPCallBack;
        m_bluetoothA2DPCallBack = NULL;
    }
    if (NULL != m_bluetoothHIDCallBack) {
        delete m_bluetoothHIDCallBack;
        m_bluetoothHIDCallBack = NULL;
    }
    if (NULL != m_bluetoothFileSync) {
        delete m_bluetoothFileSync;
        m_bluetoothFileSync = NULL;
    }
    if (NULL != m_client) {
        releaseBluetoothClient();
        m_client = NULL;
    }

    if (m_app) {
        m_app->removeEventFilter(this);
    }
}

void CBluetoothApplication::initApplication()
{
    LOGD(tag, "initApplication\n");

    m_bluetoothGAPCallBack = new CBluetoothGAPCallBack();
    if (NULL == m_bluetoothGAPCallBack) {
        LOGE(tag, "m_bluetoothGAPCallBack is empty!\n");
    }

    m_bluetoothHFPCallBack = new CBluetoothHFPCallBack();
    if (NULL == m_bluetoothHFPCallBack) {
        LOGE(tag, "m_bluetoothHFPCallBack is empty!\n");
    }

    m_bluetoothPBAPCallBack = new CBluetoothPBAPCallBack();
    if (NULL == m_bluetoothPBAPCallBack) {
        LOGE(tag, "m_bluetoothPBAPCallBack is empty!\n");
    }

    m_bluetoothAVRCPCallBack = new CBluetoothAVRCPCallBack();
    if (NULL == m_bluetoothAVRCPCallBack) {
        LOGE(tag, "m_bluetoothAVRCPCallBack is empty!\n");
    }

    m_bluetoothA2DPCallBack = new CBluetoothA2DPCallBack();
    if (NULL == m_bluetoothA2DPCallBack) {
        LOGE(tag, "m_bluetoothA2DPCallBack is empty!\n");
    }

    m_bluetoothFileSync = new CBluetoothFileSync();
    if (NULL == m_bluetoothFileSync) {
        LOGE(tag, "m_bluetoothFileSync is empty!\n");
    }

    m_bluetoothHIDCallBack = new CBluetoothHIDCallBack();
    if (NULL == m_bluetoothHIDCallBack) {
        LOGE(tag, "m_bluetoothHIDCallBack is empty!\n");
    }

    //connect m_bluetoothGAPCallBack's signal and m_bluetoothSettingPage's slot function
    QObject::connect(m_bluetoothGAPCallBack, SIGNAL(sigBluetoothLocalName(QString)),
            m_bluetoothSettingPage, SLOT(doBluetoothLocalName(QString)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothGAPCallBack, SIGNAL(sigPowerState(bool)),
            m_bluetoothSettingPage, SLOT(doBluetoothPowerState(bool)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothGAPCallBack, SIGNAL(sigGapLinkStateErrorInd(QString)),
                this, SLOT(doGapLinkStateInd()), Qt::QueuedConnection);

    //connect m_bluetoothHFPCallBack's signal and others slot function
    QObject::connect(m_bluetoothHFPCallBack, SIGNAL(sigCallStateChange(int)),
                this, SLOT(doCallStateChange(int)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothHFPCallBack, SIGNAL(sigATNOAnswer()),
                this, SLOT(doHFATNOAnswer()), Qt::QueuedConnection);

    //connect m_bluetoothA2DPCallBack's signal and m_bluetoothMusicPage's slot function
    QObject::connect(m_bluetoothA2DPCallBack, SIGNAL(sigA2DPConnectedResponse(QString, bool)),
            m_bluetoothPairRecordsPage, SLOT(doA2DPConnectedResponse(QString, bool)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothA2DPCallBack, SIGNAL(sigA2DPDisconnectedResponse(QString)),
            m_bluetoothPairRecordsPage, SLOT(doA2DPDisconnectedResponse(QString)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothA2DPCallBack, SIGNAL(sigAudioRequest()),
            this, SLOT(doAudioRequest()), Qt::QueuedConnection);
    QObject::connect(m_bluetoothA2DPCallBack, SIGNAL(sigAudioRelease()),
            this, SLOT(doAudioRelease()), Qt::QueuedConnection);

    //connect CallBack's signal and Page's slot function
    m_bluetoothCallPage->initBluetoothCallPage();
    m_bluetoothDialPage->initBluetoothDialPage();
    m_bluetoothCallRecordsBooksPage->initBluetoothCallRecordsBooksPage();
    m_bluetoothMusicPage->initBluetoothMusicPage();
    m_bluetoothPairRecordsPage->initBluetoothPairRecordsPage();
    m_bluetoothSettingPage->initBluetoothSettingPage();

    //connect this signal and others slot function
    QObject::connect(this, SIGNAL(sigBluetoothPowerState(bool)),
            m_bluetoothPairRecordsPage, SLOT(doPowerState(bool)), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(sigBluetoothPowerState(bool)),
            m_bluetoothSettingPage, SLOT(doBluetoothPowerState(bool)), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(languageChanged()),
            m_bluetoothPairRecordsPage, SLOT(doLanguageChanged()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(sigRestartDownload()),
            m_bluetoothCallRecordsBooksPage, SLOT(doRestartDownload()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(sigDialKeyEvent()),
            m_bluetoothDialPage, SLOT(doDialKeyEvent()), Qt::QueuedConnection);

    //connect m_bluetoothSettingPage's signal and others slot function
    QObject::connect(m_bluetoothSettingPage, SIGNAL(autoConnectStateChanged(bool)),
            m_bluetoothPairRecordsPage, SLOT(doAutoConnectStateChanged(bool)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothSettingPage, SIGNAL(autoAnswerStateChanged(bool)),
            m_bluetoothCallPage, SLOT(doAutoListenStateChanged(bool)), Qt::QueuedConnection);

    //connect m_bluetoothPairRecordsPage's signal and others slot function
    QObject::connect(m_bluetoothPairRecordsPage, SIGNAL(sigPhoneConnectState(bool)),
            m_bluetoothDialPage, SLOT(doPhoneConnectState(bool)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothPairRecordsPage, SIGNAL(sigPhoneConnectState(bool)),
            m_bluetoothCallRecordsBooksPage, SLOT(doPhoneConnectState(bool)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothPairRecordsPage, SIGNAL(sigPhoneConnectState(bool)),
            this, SLOT(doGetPhoneConnectState(bool)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothPairRecordsPage, SIGNAL(sigAVRCPConnectState(bool)),
            m_bluetoothMusicPage, SLOT(doAVRCPConnectState(bool)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothPairRecordsPage, SIGNAL(sigMediaConnectState(bool)),
            this, SLOT(doGetMediaConnectState(bool)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothPairRecordsPage, SIGNAL(sigA2DPConnectState(bool)),
            m_bluetoothMusicPage, SLOT(doA2DPConnectState(bool)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothPairRecordsPage, SIGNAL(sigPauseMusic()),
            m_bluetoothMusicPage, SLOT(doPauseMusic()), Qt::QueuedConnection);
    QObject::connect(m_bluetoothPairRecordsPage, SIGNAL(sigCallStateChange(int)),
                this, SLOT(doCallStateChange(int)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothPairRecordsPage, SIGNAL(sigCallStateChange(int)),
                m_bluetoothCallPage, SLOT(doCallStateChange(int)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothPairRecordsPage, SIGNAL(sigCallNumber(QString)),
                m_bluetoothCallPage, SLOT(doCallNumber(QString)), Qt::QueuedConnection);

    //connect m_bluetoothCallRecordsBooksPage's signal and others slot function
    QObject::connect(m_bluetoothCallRecordsBooksPage, SIGNAL(sigPhoneBookCallOutRequest(QString)),
            m_bluetoothDialPage, SLOT(phoneCallRequest(QString)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothCallRecordsBooksPage, SIGNAL(phoneBookDownloadStart()),
            m_bluetoothCallPage, SLOT(doGetPhoneBookState()), Qt::QueuedConnection);

    //connect m_bluetoothCallPage's signal and m_bluetoothCallRecordsBooksPage's slot function
    QObject::connect(m_bluetoothCallPage, SIGNAL(sigCallRecordsState(bool, bool, bool)),
            m_bluetoothCallRecordsBooksPage, SLOT(doCallRecordsState(bool, bool, bool)), Qt::QueuedConnection);

    //connect m_bluetoothMusicPage's signal and m_bluetoothPairRecordsPage's slot function
    QObject::connect(m_bluetoothMusicPage, SIGNAL(sigA2DPConnectRequest()),
            m_bluetoothPairRecordsPage, SLOT(doA2DPConnectRequest()), Qt::QueuedConnection);
    QObject::connect(m_bluetoothMusicPage, SIGNAL(sigAVRCPConnectRequest()),
            m_bluetoothPairRecordsPage, SLOT(doAVRCPConnectRequest()), Qt::QueuedConnection);

    //connect m_bluetoothDialPage's signal and m_bluetoothCallPage's slot function
    QObject::connect(m_bluetoothDialPage, SIGNAL(sigDialNumber(QString)),
            m_bluetoothCallPage, SLOT(doDialNumber(QString)), Qt::QueuedConnection);

    initBluetooth();

}

void CBluetoothApplication::initBluetooth()
{
    checkClockState();
    checkLanguageState();

    m_client = getBluetoothClient();
    if (NULL != m_client) {
        ;
    } else {
        LOGE(tag, "m_client is empty!\n");
    }

    registerGAPCallBack();
    registerHFPCallBack();
    registerPBAPCallBack();
    registerAVRCPCallBack();
    registerA2DPCallBack();
    registerHIDCallBack();

    if (NULL != m_client) {
        m_client->getLocalDevice(&m_localDevice);
        if (NULL != m_localDevice) {
            E_LOCAL_DEVICE_STATE power_state = GAP_STATE_INVALID;
            m_localDevice->getBluetoothState(power_state);
            LOGD(tag, "power_state is 0x%x\n", power_state);
            if (GAP_STATE_POWERON == power_state){
                LOGD(tag, "bluetooth already power on!\n");
                emit sigBluetoothPowerState(true);
            }

            LOGD(tag, "setScanMode\n");
            E_GAP_SCAN_MODE scan_mode = SCAN_MODE_PAGE_ON_INQUIRY_ON_LOW;
            m_localDevice->setScanMode(scan_mode);

            if (NULL != m_bluetoothPairRecordsPage) {
                m_bluetoothPairRecordsPage->getBluetoothClient(m_client);
                m_bluetoothPairRecordsPage->getLocalDevice(m_localDevice);
            } else {
                LOGE(tag, "m_bluetoothPairRecordsPage is empty!\n");
            }

            if (NULL != m_bluetoothSettingPage) {
                m_bluetoothSettingPage->getLocalDevice(m_localDevice);
            } else {
                LOGE(tag, "m_bluetoothSettingPage is empty!\n");
            }
        } else {
            LOGE(tag, "m_localDevice is empty!\n");
        }
    }
}

void CBluetoothApplication::initTranslator(QApplication *app, QTranslator *translator)
{
    m_app = app;
    m_translator = translator;
    if (m_app) {
        m_app->installEventFilter(this);
    }
}

void CBluetoothApplication::registerGAPCallBack()
{
    if (NULL != m_client) {
        int ret = m_client->registerCallBack(*m_bluetoothGAPCallBack);
        if (0 == ret) {
            ;
        } else {
            LOGE(tag, "registerGAPCallBack Fail!\n");
        }
    } else {
        LOGE(tag, "m_client is empty!\n");
    }
}

void CBluetoothApplication::registerHFPCallBack()
{
    int ret = 0;
    IBluetoothProfile *profile = NULL;

    if (NULL != m_client) {
        ret = m_client->getProfile(HANDSFREEPROFILENAME, &profile);
        if (0 <= ret) {
            ;
        } else {
            LOGE(tag, "get handsfree profile Fail!, profile is %p\n", profile);
        }
    }

    m_hfpInterface = dynamic_cast<IBluetoothHandsfree*>(profile);
    if (NULL != m_hfpInterface) {

        ret = m_hfpInterface->registerCallBack(*m_bluetoothHFPCallBack);
        if (0 <= ret) {
            ;
        } else {
            LOGE(tag, "registerHFPCallBack Fail!\n");
        }

        if (NULL != m_bluetoothDialPage) {
            m_bluetoothDialPage->getHFPInterface(m_hfpInterface);
            ;
        } else {
            LOGE(tag, "m_bluetoothDialPage is empty!\n");
        }

        if (NULL != m_bluetoothCallPage) {
            m_bluetoothCallPage->getHFPInterface(m_hfpInterface);
        } else {
            LOGE(tag, "m_bluetoothCallPage is empty!\n");
        }

        if (NULL != m_bluetoothPairRecordsPage) {
            m_bluetoothPairRecordsPage->getHFPInterface(m_hfpInterface);
        } else {
            LOGE(tag, "m_bluetoothPairRecordsPage is empty!\n");
        }
    } else {
        LOGE(tag, "m_hfpInterface is empty!\n");
    }
}

void CBluetoothApplication::registerPBAPCallBack()
{
    int ret = 0;
    IBluetoothProfile *profile = NULL;

    if (NULL != m_client) {
        ret = m_client->getProfile(PBAPPROFILENAME, &profile);
        if (0 <= ret) {
            ;
        } else {
            LOGE(tag, "get pbap profile Fail!, profile is %p\n", profile);
        }
    }

    m_pbapInterface = dynamic_cast<IBluetoothPBAP*>(profile);
    if (NULL != m_pbapInterface) {
        ret = m_pbapInterface->registerCallBack(*m_bluetoothPBAPCallBack);
        if (0 <= ret) {
            ;
        } else {
            LOGE(tag, "registerPBAPCallBack Fail!\n");
        }

        if (NULL != m_bluetoothCallRecordsBooksPage) {
            m_bluetoothCallRecordsBooksPage->getPBAPInterface(m_pbapInterface);
        } else {
            LOGE(tag, "m_bluetoothCallRecordsBooksPage is empty!\n");
        }

        if (NULL != m_bluetoothCallPage) {
            m_bluetoothCallPage->getPBAPInterface(m_pbapInterface);
        } else {
            LOGE(tag, "m_bluetoothCallPage is empty!\n");
        }
    } else {
        LOGE(tag, "m_pbapInterface is empty!\n");
    }
}

void CBluetoothApplication::registerAVRCPCallBack()
{
    int ret = 0;
    IBluetoothProfile *profile = NULL;
    if (NULL != m_client) {
        ret = m_client->getProfile(AVRCPPROFILENAME, &profile);
        if (0 <= ret) {
            ;
        } else {
            LOGE(tag, "get avrcp profile Fail!, profile is %p\n", profile);
        }
    } else {
        LOGE(tag, "m_client is empty!\n");
    }

    m_avrcpInterface = dynamic_cast<IBluetoothAvrcp*>(profile);
    if (NULL != m_avrcpInterface) {
        ret = m_avrcpInterface->registerCallBack(*m_bluetoothAVRCPCallBack);
        if (0 <= ret) {
            m_avrcpInterface->updatePlayStatus();
            m_avrcpInterface->updateMediaInfo();

        } else {
            LOGE(tag, "registerAVRCPCallBack Fail!\n");
        }

        if (NULL != m_bluetoothMusicPage) {
            m_bluetoothMusicPage->getAVRCPInterface(m_avrcpInterface);
        } else {
            LOGE(tag, "m_bluetoothMusicPage is empty!\n");
        }

        if (NULL != m_bluetoothPairRecordsPage) {
            m_bluetoothPairRecordsPage->getAVRCPInterface(m_avrcpInterface);
        } else {
            LOGE(tag, "m_bluetoothPairRecordsPage is empty!\n");
        }
    } else {
        LOGE(tag, "m_avrcpInterface is empty!\n");
    }


}

void CBluetoothApplication::registerA2DPCallBack()
{
    int ret = 0;
    IBluetoothProfile *profile = NULL;

    if (NULL != m_client) {
        ret = m_client->getProfile(A2DPPROFILENAME, &profile);
        if (0 <= ret) {
            ;
        } else {
            LOGE(tag, "get a2dp profile Fail!, profile is %p\n", profile);
        }
    } else {
        LOGE(tag, "m_client is empty!\n");
    }

    m_a2dpInterface = dynamic_cast<IBluetoothA2dp*>(profile);
    if (NULL != m_a2dpInterface) {
        ret = m_a2dpInterface->registerCallBack(*m_bluetoothA2DPCallBack);
        if (0 <= ret) {
            ;
        } else {
            LOGE(tag, "registerA2DPCallBack Fail!\n");
        }

        if (NULL != m_bluetoothPairRecordsPage) {
            m_bluetoothPairRecordsPage->getA2DPInterface(m_a2dpInterface);
        } else {
            LOGE(tag, "m_bluetoothPairRecordsPage is empty!\n");
        }
    } else {
        LOGE(tag, "m_a2dpInterface is empty!\n");
    }
}

void CBluetoothApplication::registerHIDCallBack()
{
    int ret = 0;
    IBluetoothProfile *profile = NULL;

    if (NULL != m_client) {
        ret = m_client->getProfile(HIDPROFILENAME, &profile);
        if (0 <= ret) {
            ;
        } else {
            LOGE(tag, "get hid profile Fail!, profile is %p\n", profile);
        }
    } else {
        LOGE(tag, "m_client is empty!\n");
    }

    m_hidInterface = dynamic_cast<IBluetoothHid*>(profile);
    if (NULL != m_hidInterface) {
        ret = m_hidInterface->registerCallBack(*m_bluetoothHIDCallBack);
        if (0 <= ret) {
            ;
        } else {
            LOGE(tag, "registerHIDCallBack Fail!\n");
        }

        if (NULL != m_bluetoothPairRecordsPage) {
            m_bluetoothPairRecordsPage->getHIDInterface(m_hidInterface);
        } else {
            LOGE(tag, "m_bluetoothPairRecordsPage is empty!\n");
        }
    } else {
        LOGE(tag, "m_hidInterface is empty!\n");
    }
}

void CBluetoothApplication::checkClockState()
{
    GlobalBus::E_CLOCK_STATE clockState =
        (GlobalBus::E_CLOCK_STATE)GlobalBus::getState(GlobalBus::STATE_CLOCK);
    if (GlobalBus::CLOCK_12 == clockState) {
        m_clock24IsActive = false;
        emit clockStateChanged(m_clock24IsActive);
    } else if (GlobalBus::CLOCK_24 == clockState) {
        m_clock24IsActive = true;
        emit clockStateChanged(m_clock24IsActive);
    }
}

void CBluetoothApplication::checkLanguageState()
{
    GlobalBus::E_LANGUAGE_STATE languageState =
        (GlobalBus::E_LANGUAGE_STATE)GlobalBus::getState(GlobalBus::STATE_LANGUAGE);

    if (!m_app || !m_translator) {
        return;
    }
    if (languageState == m_languageState) {
        LOGD(tag, "the language no change\n");
        return;
    }
    if (GlobalBus::ENGLISH == languageState) {
        m_languageState = GlobalBus::ENGLISH;
        m_translator->load("");
        m_app->removeTranslator(m_translator);
    } else if (GlobalBus::CHINESE == languageState) {
        if (m_translator->load("/app/bluetooth_chs.qm")) {
            m_languageState = GlobalBus::CHINESE;
            m_app->installTranslator(m_translator);
        } else {
            LOGE(tag, "installTranslator CHINESE FAIL\n");
        }
    } else if (GlobalBus::CHINESE_TW == languageState){
        if (m_translator->load("/app/bluetooth_cht.qm")) {
            m_languageState = GlobalBus::CHINESE_TW;
            m_app->installTranslator(m_translator);
        } else {
            LOGE(tag, "installTranslator CHINESE_TW FAIL\n");
        }
    }
    emit languageChanged();
}

void CBluetoothApplication::onDialKeyEvent()
{
    LOGD(tag, "onDialKeyEvent\n");

    bool ret = false;
    E_BLUETOOTH_PROFILE_STATE state = BLUETOOTH_PROFILE_IDLE;

    if (NULL != m_hfpInterface) {

        ret = m_hfpInterface->getState(state);
        if ((0 <= ret) && (BLUETOOTH_PROFILE_CONNECTED == state)) {
            //hfp connected
            int callState = m_hfpInterface->getCallState();
            LOGD(tag, "hfp callState is %d\n", callState);
            if (HF_CALLSTATE_IDLE == callState) {
                if (m_currentPageIndex != DIAL_PAGE_INDEX) {
                    //enter dial page
                    LOGD(tag, "enter dial Page\n");
                    emit enterPage(DIAL_PAGE_INDEX);
                } else {
                    //do dial
                    LOGD(tag, "sigDialKeyEvent\n");
                    emit sigDialKeyEvent();

                }
            } else if (HF_CALLSTATE_INCOMING  == callState) {
                //do answer
                LOGD(tag, "acceptIncommingCall\n");
                m_hfpInterface->acceptIncommingCall();
            }
        } else {
            LOGD(tag, "hfp state is %d\n", (int)state);
            //notify "please connect hfp"
            emit notifyToConnectPhone();
        }
    }

}


/////////////////////////////the function invoke by bluetoothApplicationPageView//////////////////////////////
bool CBluetoothApplication::getPhoneConnectState()
{
    return m_phoneConnectState;
}

bool CBluetoothApplication::getMediaConnectState()
{
    return m_mediaConnectState;
}

bool CBluetoothApplication::getClockState()
{
    LOGD(tag, "getClockState\n");

    return m_clock24IsActive;
}

bool CBluetoothApplication::goHome()
{
    LOGD(tag, "goHome\n");

    GlobalBus::applyFor(GlobalBus::ACTION_RUN, CAPPBaseObj::APPID_CLUSTER, CAPPBaseObj::LEVEL_NORMAL);
    if(NULL != m_bluetoothFileSync) {
        m_bluetoothFileSync->startSync();
    }

    return (true);
}

bool CBluetoothApplication::goExit()
{
    LOGD(tag, "goExit\n");

    GlobalBus::applyFor(GlobalBus::ACTION_HIDEFRONT, CAPPBaseObj::APPID_BT, CAPPBaseObj::LEVEL_NORMAL);
     if(NULL != m_bluetoothFileSync) {
        m_bluetoothFileSync->startSync();
    }

    return (true);
}

void CBluetoothApplication::setPageIndex(int index)
{
    if (m_currentPageIndex != index) {
        LOGD(tag, "setPageIndex, index = %d\n", index);
        m_currentPageIndex = index;
    }
}

/////////////////////////////////////// the slot function /////////////////////////////////////
void CBluetoothApplication::doShowCallPage(bool isAnswerCallNeeded)
{
    LOGD(tag, "doShowCallPage\n");

    if (false == m_bluetoothCallPageIsShow) {
        LOGD(tag, "applyFor ACTION_SHOWFRONT\n");
        GlobalBus::applyFor(GlobalBus::ACTION_SHOWFRONT, CAPPBaseObj::APPID_BTPHONE, CAPPBaseObj::LEVEL_NORMAL);
        GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REQ, CAPPBaseObj::APPID_BTPHONE, CCtlListener::AVOUT_F | CAPPBaseObj::LEVEL_TRANSIENT_CAN_DUCK);

        emit popDialupInterface(isAnswerCallNeeded);
        m_bluetoothCallPageIsShow = true;
    }
}

void CBluetoothApplication::doCloseCallPage()
{
    LOGD(tag, "doCloseCallPage\n");

    m_bluetoothCallPageIsShow = false;

    LOGD(tag, "applyFor ACTION_HIDEFRONT\n");
    GlobalBus::applyFor(GlobalBus::ACTION_HIDEFRONT, CAPPBaseObj::APPID_BTPHONE, CAPPBaseObj::LEVEL_NORMAL);
    GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REL, CAPPBaseObj::APPID_BTPHONE, CCtlListener::AVOUT_F | CAPPBaseObj::LEVEL_TRANSIENT_CAN_DUCK);
}

void CBluetoothApplication::doCallStateChange(int callState)
{
    LOGD(tag, "doCallStateChange\n");
    bool isAnswerCallNeeded = false;

    if (HF_CALLSTATE_INCOMING == callState) {
        LOGD(tag, "HF_CALLSTATE_INCOMING\n");
        isAnswerCallNeeded = true;
        doShowCallPage(isAnswerCallNeeded);
    } else if (HF_CALLSTATE_OUTGOING == callState) {
        LOGD(tag, "HF_CALLSTATE_OUTGOING\n");
        isAnswerCallNeeded = false;
        doShowCallPage(isAnswerCallNeeded);
    } else if (HF_CALLSTATE_ALERTING == callState) {
        LOGD(tag, "HF_CALLSTATE_ALERTING\n");
        isAnswerCallNeeded = false;
        doShowCallPage(isAnswerCallNeeded);
    } else if (HF_CALLSTATE_SPEAKING == callState) {
        LOGD(tag, "HF_CALLSTATE_SPEAKING\n");
        isAnswerCallNeeded = false;
        doShowCallPage(isAnswerCallNeeded);
    } else if (HF_CALLSTATE_IDLE == callState){
        LOGD(tag, "HF_CALLSTATE_IDLE\n");
        doCloseCallPage();
        emit sigRestartDownload();
    }
    emit answerCallStateChanged(isAnswerCallNeeded);
}

void CBluetoothApplication::doHFATNOAnswer()
{
    LOGI(tag, "doHFATNOAnswer\n");

    emit sigShowPhoneNoAnswer();
}

void CBluetoothApplication::doGapLinkStateInd()
{
    LOGI(tag, "doGapLinkStateInd\n");

    emit sigShowLinkLost();
}

void CBluetoothApplication::doGetPhoneConnectState(bool phoneConnectState)
{
    LOGD(tag, "doGetPhoneConnectState\n");

    m_phoneConnectState = phoneConnectState;
    emit phoneConnectStateChanged(m_phoneConnectState);

    if (false == m_phoneConnectState && true == m_bluetoothCallPageIsShow) {
        doCloseCallPage();
    }
}

void CBluetoothApplication::doGetMediaConnectState(bool mediaConnectState)
{
    LOGD(tag, "doGetMediaConnectState\n");

    m_mediaConnectState = mediaConnectState;
    emit mediaConnectStateChanged(m_mediaConnectState);
}

void CBluetoothApplication::doAudioRequest()
{
    LOGD(tag, "doAudioRequest\n");

    LOGD(tag, "applyFor ACTION_AUDIO_REQ\n");
    GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REQ, CAPPBaseObj::APPID_BT, CAPPBaseObj::LEVEL_NORMAL);
}

void CBluetoothApplication::doAudioRelease()
{
    LOGD(tag, "doAudioRelease\n");

    LOGD(tag, "applyFor ACTION_AUDIO_REL\n");
    GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REL, CAPPBaseObj::APPID_BT, CAPPBaseObj::LEVEL_NORMAL);
}

int CBluetoothApplication::doExit (int param1, int param2)
{
    LOGD(tag, "doExit\n");

    CQObjListener::doExit(param1, param2);

    return 1;
}

int CBluetoothApplication::doShowFront (int param1, int param2)
{
    LOGD(tag, "doShowFront\n");

    CQObjListener::doShowFront(param1, param2);

    checkClockState();
    checkLanguageState();

    return 1;
}

int CBluetoothApplication::doHideFront (int param1, int param2)
{
    LOGD(tag, "doHideFront\n");

    CQObjListener::doHideFront(param1, param2);

    return 1;
}

int CBluetoothApplication::doAudioFocusChanged (
    CCtlListener::E_AVOUT aOut, CCtlListener::E_AUDIOFOCUS focus)
{
    LOGD(tag, "doAudioFocusChanged, aOut is %d, focus is %d\n", (int)aOut, (int)focus);

    switch (focus){
        case CCtlListener::AUDIOFOCUS_GAIN:
            LOGD(tag, "AUDIOFOCUS_GAIN\n");
            //open A2DP audio
            if (NULL != m_a2dpInterface) {
                LOGD(tag, "enableAudio -> true\n");
                m_a2dpInterface->enableAudio(true);
            }
            break;

        case CCtlListener::AUDIOFOCUS_LOSS:
        case CCtlListener::AUDIOFOCUS_LOSS_TRANSIENT:
        case CCtlListener::AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK: {
            LOGD(tag, "AUDIOFOCUS_LOSS\n");
            E_LOCAL_DEVICE_STATE power_state;
            if (NULL != m_localDevice) {
                m_localDevice->getBluetoothState(power_state);
                LOGD(tag, "power_state = %d\n", power_state);
            }
            if ((GAP_STATE_POWEROFF != power_state) &&
                (GAP_STATE_POWERSWTICHING != power_state)) {
                //release A2DP audio
                if (NULL != m_a2dpInterface) {
                    LOGD(tag, "enableAudio -> false\n");
                    m_a2dpInterface->enableAudio(false);
                }
            }
            break;
            }
        }

    return 1;
}

bool CBluetoothApplication::doKeyEvent (int key, int param1, int param2)
{
    bool ret = false;
    LOGD(tag, "doKeyEvent, key is %d, param1 is %d, param2 is %d\n", key, param1, param2);

    GlobalBus::E_STATE_TYPE stateType = (GlobalBus::E_STATE_TYPE) param1;
    unsigned char mainfunc = getMainFunc(key);
    unsigned char subfunc = getSubFunc(key);

    switch (mainfunc) {
    case CCmdTask::MAIN_FUNC_KEY:
        switch (subfunc) {
        case CCmdTask::SUBFUNC_KEY_BACK:
            goExit();
            ret = true;
            break;
        default:
            break;
        }
        break;

    case CCmdTask::MAIN_FUNC_APP_ACTION:
        switch (subfunc) {
        case GlobalBus::ACTION_STATUS_CHANGED:
            switch (stateType) {
            case GlobalBus::INVALID_STATE_TYPE: {
                LOGD(tag, "INVALID_STATE_TYPE\n");
                break;
            }

            case GlobalBus::STATE_BT: {
                GlobalBus::E_BT_STATE btState = (GlobalBus::E_BT_STATE) GlobalBus::getState(
                        stateType);
                LOGD(tag, "btState is %d, BT_CONNET(1), BT_DISCONNET(2)\n", btState);
                ret = true;
                break;
            }

            case GlobalBus::STATE_CLOCK: {
                checkClockState();
                ret = true;
            }
                break;

            case GlobalBus::STATE_LANGUAGE: {
                checkLanguageState();
                ret = true;
                break;
            }

            case GlobalBus::STATE_WIFI: {
                LOGD(tag, "STATE_WIFI\n");
                break;
            }

            default:
                LOGD(tag, "stateType: %d\n", stateType);
                break;
            }
            break;
        }
        break;

        case CCmdTask::MAIN_FUNC_BT:
            switch (subfunc) {
            case CCmdTask::SUBFUNC_BT_DIALING:
                onDialKeyEvent();
                break;
            default:
                break;
            }
            break;

        default:
            break;

    }

    return ret;
}

