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

#ifndef BLUETOOTHPAIRRECORDSPAGE_H
#define BLUETOOTHPAIRRECORDSPAGE_H

#include <QObject>
#include <QList>
#include <QTimer>
#include <qvariant.h>
#include <string>
#include <stdio.h>

#include "applog.h"
#include "bluetoothapi.h"
#include "bluetoothavrcp.h"
#include "bluetootha2dp.h"
#include "../../connectivity/universal_utils/include/singleton.h"
#include "bluetoothpaireddevicemodel.h"
#include "bluetoothavailabledevicemodel.h"
#include "bluetoothgapcallback.h"
#include "bluetoothhfpcallback.h"
#include "bluetoothavrcpcallback.h"
#include "bluetoothhid.h"
#include "bluetoothhidcallback.h"

#define STRING_UNPAIR                           CBluetoothPairRecordsPage::trUtf8("No Pair")
#define STRING_PAIRED                           CBluetoothPairRecordsPage::trUtf8("Paired")
#define STRING_CONNECTING                       CBluetoothPairRecordsPage::trUtf8("Connecting...")
#define STRING_PHONEAUDIO_CONNECTING            CBluetoothPairRecordsPage::trUtf8("MediaAudio Connected and PhoneAudio Connecting...")
#define STRING_PHONEAUDIO_CONNECTED             CBluetoothPairRecordsPage::trUtf8("PhoneAudio Connected")
#define STRING_MEDIAAUDIO_CONNECTED             CBluetoothPairRecordsPage::trUtf8("MediaAudio Connected")
#define STRING_PHONEAUDIO_MEDIAAUDIO_CONNECTED  CBluetoothPairRecordsPage::trUtf8("PhoneAudio and MediaAudio Connected")
#define STRING_PHONEAUDIO_MEDIAAUDIO_HID_CONNECTED CBluetoothPairRecordsPage::trUtf8("PhoneAudio MediaAudio HID Connected")

class CBluetoothPairRecordsPage
    : public QObject
    , public universal_utils::Singleton<CBluetoothPairRecordsPage>
{
    Q_OBJECT

public:
    CBluetoothPairRecordsPage();
    ~CBluetoothPairRecordsPage();
    void initBluetoothPairRecordsPage();
    void getBluetoothClient(IBluetoothClient *client);
    void getLocalDevice(IBluetoothLocalDevice *localDevice);
    void getHFPInterface(IBluetoothHandsfree  *hfpInterface);
    void getAVRCPInterface(IBluetoothAvrcp *avrcpInterface);
    void getA2DPInterface(IBluetoothA2dp *a2dpInterface);
    void getHIDInterface(IBluetoothHid *hidInterface);

signals:
    void waitToScan();
    void waitToPower();
    void remindPairFail();
    void scanStateChanged(bool m_scanState);

    void sigPhoneConnectState(bool m_phoneConnectState);
    void sigMediaConnectState(bool m_mediaConnectState);
    void sigA2DPConnectState(bool m_a2dpConnectState);
    void sigAVRCPConnectState(bool m_avrcpConnectState);
    void sigPauseMusic();
    void sigCallStateChange(int callState);
    void sigCallNumber(QString callNuber);

    void updateCheckBoxPhoneConnectState(bool isConnected);
    void updateCheckBoxMediaConnectState(bool isConnected);

public slots:
    bool getScanState();

    void scanningRequest();
    void stopScanRequest();
    void pairingRequest(int index);
    void connectRequest(int index);
    void disconnectRequest(int index);
    void dispairRequest(int index);
    void openPairedItemSelectConnectBox(int index);
    void openAvailableItemSelectConnectBox(int index);
    void setPairedItemSelectConnectState(int index, bool phoneAudioState, bool mediaAudioState);
    void setAvailableItemSelectConnectState(int index, bool phoneAudioState, bool mediaAudioState);

    void doPowerState(bool powerState);
    void doScanModeActive(bool scanModeState);
    void doScanStartResponse();
    void doScanStopResponse();
    void doScanResponse(QString btdev_address, QString btdev_name);
    void doUpdateNameResponse(QString btdev_address, QString btdev_name);
    void doSecureUserConfirmRequest(QString btdev_address);
    void doPairResponse(QString btdev_address, QString btdev_name, bool m_pairState);
    void doPairRemoveResponse(QString btdev_address, QString btdev_name);
    void doGapLinkStateInd(QString btdev_address);
    void doHFConnectingResponse(QString btdev_address);
    void doHFConnectedResponse(QString btdev_address, bool hfpConnectedResult);
    void doHFDisconnectedResponse(QString btdev_address);
    void doA2DPConnectedResponse(QString btdev_address, bool a2dpConnectedResult);
    void doA2DPDisconnectedResponse(QString btdev_address);
    void doAVRCPConnectedResponse(QString btdev_address, bool avrcpConnectedResult);
    void doAVRCPDisconnectedResponse(QString btdev_address);
    void doAutoConnectStateChanged(bool autoConnectState);
    void doAutoConnectAddress(long long bluetoothAddress);
    void doA2DPConnectRequest();
    void doAVRCPConnectRequest();
    void doLanguageChanged ();
    void doAutoConnectTimeout();
    void doUuidsUpdate(QString btdev_address, QString btdev_name);

public:
    void getPairedListFromStorage();
    bool checkProfileState(QString &pairedDeviceState, QString autoConnectAddress);
    void checkMediaProfileState(QString &pairedDeviceState, QString phoneAddress, QString mediaAddress);
    void createRemoteDevice(QString btdev_address);
    void checkNewline(QString &btdev_name);
    void addNewPairedDevice(QString btdev_address, QString btdev_name, QString btdev_state);
    void selectConnectRequest(int index);
    void selectDisconnectRequest(int index);
    void saveLatestConnectedAddress(QString btdev_address);
    QString addressToQString(BluetoothAddress address);
    void checkMediaConnectState();
    void setBluetoothConnectState();
    void doMediaConnectedResponse(QString btdev_address, bool connectedResult);
    void doMediaDisconnectedResponse(QString btdev_address);
    bool getPairedDeviceAddress(QString &pairedDeviceAddress);
    void startAutoConnectTimer();
    void stopAutoConnectTimer();
    bool isProfileConnected();
    void reconnectProfile(QString btdev_address);
    bool getPhoneAudioState();
    bool getMediaAudioState();
    void getDeviceState(QString btdev_address, bool &isHFPConnected, bool &isA2DPConnected, bool &isAVRCPConnected);
    void getDeviceStateString(bool isHFPConnected, bool isA2DPConnected, bool isAVRCPConnected, QString &pairedDeviceState);
    void updateDeviceState(QString btdev_address);

private:
        bool isRemoteDeviceBonded(QString btdev_address);

private:
    bool m_powerState;
    bool m_scanState;
    bool m_scanModeState;
    bool m_phoneConnectState;
    bool m_mediaConnectState;
    bool m_a2dpConnectState;
    bool m_avrcpConnectState;
    bool m_autoConnectState;
    int  m_maxRemoteDevice;
    int  m_Timeoff;
    QString m_autoConnectAddress;
    QString m_selectPairedAddress;
    QList<QString> m_initPairDeviceList;
    E_LOCAL_DEVICE_STATE m_localDevcieState;

    IBluetoothClient      *m_client;
    IBluetoothLocalDevice *m_localDevice;
    BluetoothRemoteDevice *m_remoteDevice;
    IBluetoothHandsfree   *m_hfpInterface;
    IBluetoothAvrcp       *m_avrcpInterface;
    IBluetoothA2dp        *m_a2dpInterface;
    IBluetoothHid         *m_hidInterface;

    CBluetoothGAPCallBack   *m_bluetoothGAPCallBack;
    CBluetoothHFPCallBack   *m_bluetoothHFPCallBack;
    CBluetoothAVRCPCallBack *m_bluetoothAVRCPCallBack;
    CBluetoothPairedDeviceModel *m_bluetoothPairedDeviceModel;
    CBluetoothAvailableDeviceModel *m_bluetoothAvailableDeviceModel;
    QTimer *m_autoConnectTimer;
    CBluetoothHIDCallBack *m_bluetoothHIDCallBack;
};

#endif
