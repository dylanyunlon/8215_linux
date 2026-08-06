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
 
#ifndef CBLUETOOTHAPPLICATION_H
#define CBLUETOOTHAPPLICATION_H
#include <QObject>
#include <QQuickItem>
#include <QApplication>
#include <QTranslator>
#include <iostream>

#include "applog.h"
#include "qobjlistener.h"
#include "globalbus.h"
#include "ccmdtask.h"

#include "bluetoothcallpage.h"
#include "bluetoothdialpage.h"
#include "bluetoothcallrecordsbookspage.h"
#include "bluetoothmusicpage.h"
#include "bluetoothpairrecordspage.h"
#include "bluetoothsettingpage.h"

#include "bluetoothgapcallback.h"
#include "bluetoothhfpcallback.h"
#include "bluetoothpbapcallback.h"
#include "bluetoothavrcpcallback.h"
#include "bluetootha2dpcallback.h"
#include "bluetoothfilesync.h"
#include "bluetoothhidcallback.h"

class CBluetoothApplication : public CQObjListener
{
    Q_OBJECT
public:
    CBluetoothApplication();
    CBluetoothApplication(CBluetoothCallPage *bluetoothCallPage,
        CBluetoothDialPage *bluetoothDialPage,
        CBluetoothCallRecordsBooksPage *bluetoothCallRecordsBooksPage,
        CBluetoothMusicPage *bluetoothMusicPage,
        CBluetoothPairRecordsPage *bluetoothPairRecordsPage,
        CBluetoothSettingPage *bluetoothSettingPage);
    ~CBluetoothApplication();

    void initApplication();
    void initBluetooth();
    void initTranslator(QApplication *app, QTranslator *translator);
    void registerGAPCallBack();
    void registerHFPCallBack();
    void registerPBAPCallBack();
    void registerAVRCPCallBack();
    void registerA2DPCallBack();
    void checkClockState();
    void checkLanguageState();
    void onDialKeyEvent();
    void registerHIDCallBack();

    typedef enum {
        DIAL_PAGE_INDEX = 0,
        PHONEBOOK_PAGE_INDEX,
        CALLRECORD_PAGE_INDEX,
        MUSIC_PAGE_INDEX,
        PAIRRECORD_PAGE_INDEX,
        SETTING_PAGE_INDEX,
    }E_PAGE_INDEXS;

    typedef enum {
        BT_INVALID = 0,
        BT_MUSIC,
        BT_LOCAL_RING_TONE,
    }E_REQUEST_AUDIO_FOCUS_TYPE;

signals:
    void sigBluetoothPowerState(bool powerState);
    void popDialupInterface(bool m_acceptCallState);
    void phoneConnectStateChanged(bool m_phoneConnectState);
    void mediaConnectStateChanged(bool m_mediaConnectState);
    void clockStateChanged(bool m_clock24IsActive);
    void languageChanged ();
    void sigRestartDownload();
    void answerCallStateChanged(bool isAnswerCallNeeded);
    void enterPage(int index);
    void notifyToConnectPhone();
    void sigDialKeyEvent();
    void sigShowPhoneNoAnswer();
    void sigShowLinkLost();

public slots:
    bool getPhoneConnectState();
    bool getMediaConnectState();
    bool getClockState();

    bool goHome();
    bool goExit();

    void setPageIndex(int index);

    void doShowCallPage(bool isAnswerCallNeeded);
    void doCloseCallPage();
    void doCallStateChange(int callState);
    void doHFATNOAnswer();
    void doGapLinkStateInd();
    void doGetPhoneConnectState(bool phoneConnectState);
    void doGetMediaConnectState(bool mediaConnectState);
    void doAudioRequest();
    void doAudioRelease();

public slots:
    virtual int doExit (int param1, int param2);
    virtual int doShowFront (int param1, int param2);
    virtual int doHideFront (int param1, int param2);
    virtual int doAudioFocusChanged (CCtlListener::E_AVOUT aOut, CCtlListener::E_AUDIOFOCUS focus);
    virtual bool doKeyEvent (int key, int param1, int param2);

public:
    CBluetoothCallPage             *m_bluetoothCallPage;
    CBluetoothDialPage             *m_bluetoothDialPage;
    CBluetoothCallRecordsBooksPage *m_bluetoothCallRecordsBooksPage;
    CBluetoothMusicPage            *m_bluetoothMusicPage;
    CBluetoothPairRecordsPage      *m_bluetoothPairRecordsPage;
    CBluetoothSettingPage          *m_bluetoothSettingPage;

    CBluetoothGAPCallBack   *m_bluetoothGAPCallBack;
    CBluetoothHFPCallBack   *m_bluetoothHFPCallBack;
    CBluetoothPBAPCallBack  *m_bluetoothPBAPCallBack;
    CBluetoothAVRCPCallBack *m_bluetoothAVRCPCallBack;
    CBluetoothA2DPCallBack  *m_bluetoothA2DPCallBack;
    CBluetoothHIDCallBack   *m_bluetoothHIDCallBack;
    CBluetoothFileSync      *m_bluetoothFileSync;

private:
    QObject object;
    bool m_bluetoothCallPageIsShow;
    bool m_phoneConnectState;
    bool m_mediaConnectState;
    bool m_clock24IsActive;
    int  m_currentPageIndex;
    IBluetoothClient      *m_client;
    IBluetoothLocalDevice *m_localDevice;
    CBluetoothGAPCallBack *m_gapcallback;
    IBluetoothHandsfree   *m_hfpInterface;
    IBluetoothPBAP        *m_pbapInterface;
    IBluetoothAvrcp       *m_avrcpInterface;
    IBluetoothA2dp        *m_a2dpInterface;
    IBluetoothHid         *m_hidInterface;
    QApplication          *m_app;
    QTranslator           *m_translator;
    GlobalBus::E_LANGUAGE_STATE m_languageState;
};

typedef struct bluetoothMemData
{
    bool memAutoAnswer;
    bool memAutoConnect;
    long long memBluetoothAddress;
}BluetoothMemData;

#endif
