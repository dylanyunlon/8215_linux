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
 
#include "bluetoothpairrecordspage.h"
#include "bluetoothapplication.h"
#include "bluetoothgapindications.h"
#include "bluetoothutils.h"

using namespace std;
using namespace universal_utils;
extern BluetoothMemData g_bluetoothMemData;
extern int saveBluetoothConfiguration();
static const char* const tag = "CBluetoothPairRecordsPage";
const static int AUTO_CONNECT_TIMEOUT = 10 * 1000;
const static int AUTO_CONNECT_MAX_TRY_TIME = 6;

template<> CBluetoothPairRecordsPage* Singleton<CBluetoothPairRecordsPage>::msSingleton = NULL;

CBluetoothPairRecordsPage::CBluetoothPairRecordsPage()
    : m_powerState(false)
    , m_scanState(false)
    , m_scanModeState(false)
    , m_phoneConnectState(false)
    , m_mediaConnectState(false)
    , m_a2dpConnectState(false)
    , m_avrcpConnectState(false)
    , m_autoConnectState(false)
    , m_maxRemoteDevice(15)
    , m_Timeoff(0)
    , m_autoConnectAddress("")
    , m_selectPairedAddress("")
    , m_localDevcieState(GAP_STATE_INVALID)
    , m_client(NULL)
    , m_localDevice(NULL)
    , m_remoteDevice(NULL)
    , m_hfpInterface(NULL)
    , m_avrcpInterface(NULL)
    , m_a2dpInterface(NULL)
    , m_bluetoothGAPCallBack(NULL)
    , m_bluetoothHFPCallBack(NULL)
    , m_bluetoothAVRCPCallBack(NULL)
    , m_bluetoothPairedDeviceModel(NULL)
    , m_bluetoothAvailableDeviceModel(NULL)
    , m_autoConnectTimer(NULL)
    , m_hidInterface(NULL)
    , m_bluetoothHIDCallBack(NULL)
{
    m_bluetoothPairedDeviceModel = CBluetoothPairedDeviceModel::getSingletonPtr();
    m_bluetoothAvailableDeviceModel = CBluetoothAvailableDeviceModel::getSingletonPtr();
    m_autoConnectTimer = new QTimer(this);
    QObject::connect(m_autoConnectTimer, SIGNAL(timeout()),
            this, SLOT(doAutoConnectTimeout()), Qt::QueuedConnection);
    m_initPairDeviceList.clear();
}

CBluetoothPairRecordsPage::~CBluetoothPairRecordsPage()
{
    LOGD(tag, "destructor\n");

    if (NULL !=  m_remoteDevice) {
        delete  m_remoteDevice;
        m_remoteDevice = NULL;
    }
    if (NULL != m_autoConnectTimer) {
        delete  m_autoConnectTimer;
        m_autoConnectTimer = NULL;
    }
}

//connnect the callback's signal and this slot function
void CBluetoothPairRecordsPage::initBluetoothPairRecordsPage()
{
    m_bluetoothGAPCallBack = CBluetoothGAPCallBack::getSingletonPtr();
    m_bluetoothHFPCallBack = CBluetoothHFPCallBack::getSingletonPtr();
    m_bluetoothAVRCPCallBack = CBluetoothAVRCPCallBack::getSingletonPtr();

    //connect GAPCallBack signal and tihs slot
    QObject::connect(m_bluetoothGAPCallBack, SIGNAL(sigPowerState(bool)),
                this, SLOT(doPowerState(bool)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothGAPCallBack, SIGNAL(sigScanModeActive(bool)),
                this, SLOT(doScanModeActive(bool)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothGAPCallBack, SIGNAL(sigScanStartResponse()),
                    this, SLOT(doScanStartResponse()), Qt::QueuedConnection);
    QObject::connect(m_bluetoothGAPCallBack, SIGNAL(sigScanStopResponse()),
                    this, SLOT(doScanStopResponse()), Qt::QueuedConnection);
    QObject::connect(m_bluetoothGAPCallBack, SIGNAL(sigScanResponse(QString, QString)),
                    this, SLOT(doScanResponse(QString, QString)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothGAPCallBack, SIGNAL(sigUpdateNameResponse(QString, QString)),
                    this, SLOT(doUpdateNameResponse(QString, QString)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothGAPCallBack, SIGNAL(sigSecureUserConfirmRequest(QString)),
                    this, SLOT(doSecureUserConfirmRequest(QString)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothGAPCallBack, SIGNAL(sigPairResponse(QString, QString, bool)),
                    this, SLOT(doPairResponse(QString, QString, bool)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothGAPCallBack, SIGNAL(sigPairRemoveResponse(QString, QString)),
                    this, SLOT(doPairRemoveResponse(QString, QString)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothGAPCallBack, SIGNAL(sigGapLinkStateErrorInd(QString)),
                    this, SLOT(doGapLinkStateInd(QString)), Qt::QueuedConnection);

    //connect HFPCallBack signal and tihs slot
    QObject::connect(m_bluetoothHFPCallBack, SIGNAL(sigHFConnectingResponse(QString)),
                this, SLOT(doHFConnectingResponse(QString)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothHFPCallBack, SIGNAL(sigHFConnectedResponse(QString, bool)),
                this, SLOT(doHFConnectedResponse(QString, bool)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothHFPCallBack, SIGNAL(sigHFDisconnectedResponse(QString)),
                this, SLOT(doHFDisconnectedResponse(QString)), Qt::QueuedConnection);

    //connect AVRCPCallBack signal and tihs slot
    QObject::connect(m_bluetoothAVRCPCallBack, SIGNAL(sigAVRCPConnectedResponse(QString, bool)),
                    this, SLOT(doAVRCPConnectedResponse(QString, bool)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothAVRCPCallBack, SIGNAL(sigAVRCPDisconnectedResponse(QString)),
                    this, SLOT(doAVRCPDisconnectedResponse(QString)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothGAPCallBack, SIGNAL(sigUuidsUpdate(QString, QString)),
                    this, SLOT(doUuidsUpdate(QString, QString)), Qt::QueuedConnection);
}

//get the bluetooth client
void CBluetoothPairRecordsPage::getBluetoothClient(IBluetoothClient *client)
{
    m_client = client;
    if(NULL != m_client ) {
        ;
    } else {
        LOGE(tag, "m_client is empty!\n");
    }
}

//get the bluetooth local device
void CBluetoothPairRecordsPage::getLocalDevice(IBluetoothLocalDevice *localDevice)
{
    LOGD(tag, "getLocalDevice\n");

    m_localDevice = localDevice;
    if(NULL != m_localDevice) {
        LOGI(tag, "m_localDevice is %p, this is %p\n", m_localDevice, this);
    } else {
        LOGE(tag, "m_localDevice is empty!\n");
    }
}

//get hfp interface
void CBluetoothPairRecordsPage::getHFPInterface(IBluetoothHandsfree  *hfpInterface)
{
    m_hfpInterface = hfpInterface;
    if(NULL != m_hfpInterface) {
        ;
    } else {
        LOGE(tag, "m_hfpInterface is empty!\n");
    }
}

//get avrcp interface
void CBluetoothPairRecordsPage::getAVRCPInterface(IBluetoothAvrcp *avrcpInterface)
{
    m_avrcpInterface = avrcpInterface;
    if(NULL != m_avrcpInterface) {
        ;
    } else {
        LOGE(tag, "m_avrcpInterface is empty!\n");
    }
}

//get a2dp interface
void CBluetoothPairRecordsPage::getA2DPInterface(IBluetoothA2dp *a2dpInterface)
{
    m_a2dpInterface = a2dpInterface;
    if(NULL != m_a2dpInterface) {
        ;
    } else {
        LOGE(tag, "m_a2dpInterface is empty!\n");
    }
}

// get hid interface
void CBluetoothPairRecordsPage::getHIDInterface(IBluetoothHid *hidInterface)
{
    m_hidInterface = hidInterface;
    if(NULL != m_hidInterface) {
        ;
    } else {
        LOGE(tag, "m_a2dpInterface is empty!\n");
    }
}


///////////////////////////////////////////// the functions are called by qml /////////////////////////////////////////

//the bluetoothPairedRecord UI get the scan state
bool CBluetoothPairRecordsPage::getScanState()
{
    LOGD(tag, "getScanState\n");

    return m_scanState;
}

//the local device send scanning request
void CBluetoothPairRecordsPage::scanningRequest()
{
    LOGD(tag, "scanningRequest\n");

    stopScanRequest();

    if (false == m_powerState) {
        //if power off, wait to power on
        emit waitToPower();
    } else {
        if (NULL != m_localDevice) {
            //before discovery, if A2DP is playing, pause it first, which avoid music noise
            emit sigPauseMusic();

            //if power on && scan mode is ready, send the scanning request
            LOGD(tag, "discovery\n");
            m_localDevice->discovery(m_maxRemoteDevice);
        } else {
            LOGE(tag, "m_localDevice is empty!\n");
        }
    }
}

void CBluetoothPairRecordsPage::stopScanRequest()
{
    if (NULL != m_localDevice) {
        m_localDevcieState = GAP_STATE_INVALID;
        m_localDevice->getBluetoothState(m_localDevcieState);
        if (GAP_STATE_DISCOVERYING == m_localDevcieState) {
            LOGD(tag, "cancelDiscovery\n");
            m_localDevice->cancelDiscovery();
        }
    }
}

//the local device send pairing request
void CBluetoothPairRecordsPage::pairingRequest(int index)
{
    LOGD(tag, "pairingRequest, index is %d\n", index);

    stopScanRequest();

    if ((NULL != m_bluetoothAvailableDeviceModel) && (-1 != index)) {
        //search the availableDeviceAddress from AvailableDeviceModel
        QString availableDeviceAddress= "";
        m_bluetoothAvailableDeviceModel->searchAvailableDeviceAddress(index, availableDeviceAddress);

        createRemoteDevice(availableDeviceAddress);

        if(NULL != m_remoteDevice && true == m_powerState) {
            //if power on, send the pairing request
            LOGI(tag, "m_remoteDevice is %p\n", m_remoteDevice);
            BluetoothAddress bt_address = m_remoteDevice->getAddress();
            string address = bt_address.toString();
            LOGI(tag, "address is %s\n", BluetoothUtils::StringForLog(address).c_str());

            LOGD(tag, "bond\n");
            bool ret = m_remoteDevice->bond();
            if (0 == ret) {
                LOGD(tag, "bond OK!\n");
            } else {
                LOGE(tag, "bond Fail!\n");
            }
            m_initPairDeviceList.append(availableDeviceAddress);
        } else {
            //if power off, wait to power on
            emit waitToPower();
        }
    }
}

//the local device send connect request
void CBluetoothPairRecordsPage::connectRequest(int index)
{
    LOGD(tag, "connectRequest, index is %d\n", index);

    if ((NULL != m_bluetoothPairedDeviceModel) && (-1 != index)) {
        //if the pairedDevice did not connect the whole profile, send the connect request
        if (false == m_bluetoothPairedDeviceModel->checkPairedDeviceState(index,
                STRING_PHONEAUDIO_MEDIAAUDIO_CONNECTED)) {

            stopScanRequest();

            //search the pairedDeviceAddress from PairedDeviceModel
            QString pairedDeviceAddress = "";
            m_bluetoothPairedDeviceModel->searchPairedDeviceAddress(index, pairedDeviceAddress);

            createRemoteDevice(pairedDeviceAddress);

            if((NULL != m_remoteDevice) && (true == m_powerState)) {
                //if power on, send the connect request
                LOGI(tag, "m_remoteDevice is %p\n", m_remoteDevice);
                E_GAP_PAIR_STATE state = BONDSTATE_UNBOND;
                m_remoteDevice->getPairState(state);
                LOGD(tag, "the pair state is %d\n", (int)state);

                if (BONDSTATE_UNBOND == state) {
                    doPairRemoveResponse(pairedDeviceAddress, "");
                    LOGD(tag, "bond\n");
                    bool ret = m_remoteDevice->bond();
                    if (0 == ret) {
                        m_initPairDeviceList.append(pairedDeviceAddress);
                        LOGD(tag, "bond OK!\n");
                    } else {
                        LOGE(tag, "bond Fail!\n");
                    }
                } else {
                    list<Uuid> uuids;
                    m_remoteDevice->getUUID(uuids);
                    if (uuids.empty()) {
                        LOGW(tag, "remote uuids not get, can not connect now!\n");
                    } else {
                        LOGD(tag, "connect %s.\n", BluetoothUtils::StringForLog(m_remoteDevice->getAddress().toString()).c_str());
                        for(list<Uuid>::iterator it = uuids.begin(); it != uuids.end(); it++) {
                            LOGD(tag, "uuids: %s\n", (*it).toString().c_str());
                        }
                        m_initPairDeviceList.removeAll(pairedDeviceAddress);
                        m_remoteDevice->connect();
                    }
                }
            } else {
                //if power off, wait to power on
                emit waitToPower();
            }
        }
    }
}

//the local device send disconnect request
void CBluetoothPairRecordsPage::disconnectRequest(int index)
{
    LOGD(tag, "disconnectRequest, index is %d\n", index);

    if ((NULL != m_bluetoothPairedDeviceModel) && (-1 != index)) {
        //if the pairedDevice did not disconnect the whole profile, send the disconnect request
        if (false == m_bluetoothPairedDeviceModel->checkPairedDeviceState(index, STRING_PAIRED)) {

            stopScanRequest();

            //search the pairedDeviceAddress from PairedDeviceModel
            QString pairedDeviceAddress = "";
            m_bluetoothPairedDeviceModel->searchPairedDeviceAddress(index, pairedDeviceAddress);

            createRemoteDevice(pairedDeviceAddress);

            if(NULL != m_remoteDevice && true == m_powerState) {
                //if power on, send the disconnect request
                LOGI(tag, "m_remoteDevice is %p\n", m_remoteDevice);
                BluetoothAddress bt_address = m_remoteDevice->getAddress();
                string address = bt_address.toString();
                LOGI(tag, "address is %s\n", BluetoothUtils::StringForLog(address).c_str());

                LOGD(tag, "disconnect\n");
                m_remoteDevice->disconnect();
            } else {
                //if power off, wait to power on
                emit waitToPower();
            }
        }
    }
}

//the local device send dispair request
void CBluetoothPairRecordsPage::dispairRequest(int index)
{
    LOGD(tag, "dispairRequest, index is %d\n", index);

    if ((NULL != m_bluetoothPairedDeviceModel) && (-1 != index)) {
        stopScanRequest();
        //search the pairedDeviceAddress from PairedDeviceModel
        QString pairedDeviceAddress = "";
        m_bluetoothPairedDeviceModel->searchPairedDeviceAddress(index, pairedDeviceAddress);

        if (true == m_powerState) {
            createRemoteDevice(pairedDeviceAddress);

            if(NULL != m_remoteDevice) {
                LOGI(tag, "m_remoteDevice is %p\n", m_remoteDevice);
                E_GAP_PAIR_STATE state = BONDSTATE_UNBOND;
                m_remoteDevice->getPairState(state);
                LOGD(tag, "the pair state is %d\n", (int)state);
                if (BONDSTATE_UNBOND == state) {
                    doPairRemoveResponse(pairedDeviceAddress, "");
                } else {
                    std::string address = pairedDeviceAddress.toStdString();
                    BluetoothAddress btAddress(address);
                    if (NULL != m_localDevice) {
                        LOGD(tag, "deleteTrust\n");
                        m_localDevice->deleteTrust(btAddress);
                    }
                }
            }
        } else {
            //if power off, wait to power on
            emit waitToPower();
        }
    }
}

void CBluetoothPairRecordsPage::openPairedItemSelectConnectBox(int index)
{
    LOGD(tag, "openPairedItemSelectConnectBox, index is %d\n", index);

    if ((NULL != m_bluetoothPairedDeviceModel) && (-1 != index)) {
        //search the pairedDeviceAddress from PairedDeviceModel
        m_bluetoothPairedDeviceModel->searchPairedDeviceAddress(index, m_selectPairedAddress);
    }

}

void CBluetoothPairRecordsPage::openAvailableItemSelectConnectBox(int index)
{
    LOGD(tag, "openAvailableItemSelectConnectBox, index is %d\n", index);

}

//set the pairedDevice's select connect state, and do the selectConnectRequest or selectDisconnectRequest
void CBluetoothPairRecordsPage::setPairedItemSelectConnectState(
    int index, bool phoneAudioState, bool mediaAudioState)
{
    LOGD(tag, "setPairedItemSelectConnectState, index is %d, phoneAudioState is %d, mediaAudioState is %d\n",
        index, phoneAudioState, mediaAudioState);
    m_selectPairedAddress = "";

    if ((NULL != m_bluetoothPairedDeviceModel) && (-1 < index)) {

        //update the PairedDeviceModel's select connect state
        m_bluetoothPairedDeviceModel->updatePairedDeviceSelectConnectState(
            index, phoneAudioState, mediaAudioState);

        if ((false == phoneAudioState) || (false == mediaAudioState)) {
            selectDisconnectRequest(index);
        }
        if ((true == phoneAudioState) || (true == mediaAudioState)) {
            selectConnectRequest(index);
        }
    }
}

//set the availableDevice's select connect state
void CBluetoothPairRecordsPage::setAvailableItemSelectConnectState(
    int index, bool phoneAudioState, bool mediaAudioState)
{
    LOGD(tag, "setAvailableItemSelectConnectState, index is %d, phoneAudioState is %d, mediaAudioState is %d\n",
        index, phoneAudioState, mediaAudioState);

    if((NULL != m_bluetoothAvailableDeviceModel) && (-1 < index)) {
        //update the AvailableDeviceModel's select connect state
        m_bluetoothAvailableDeviceModel->updateAvailableDeviceSelectConnectState(
            index, phoneAudioState, mediaAudioState);
    }
    pairingRequest(index);
}

/////////////////////////////////////////////// the slot function //////////////////////////////////////////////////////

//get the bluetooth power state form gapcallback
void CBluetoothPairRecordsPage::doPowerState(bool powerState)
{
    LOGD(tag, "doPowerState, powerState is %d\n", powerState);

    m_powerState = powerState;
    QString pairedDeviceState = STRING_PAIRED;
    m_initPairDeviceList.clear();

    if (m_powerState) {
        //bt power on.
        getPairedListFromStorage();
    } else {
        //bt power off.
        m_scanState = false;
        emit scanStateChanged(m_scanState);

        //clear the Paired devices.
        if (NULL != m_bluetoothPairedDeviceModel) {
            m_bluetoothPairedDeviceModel->clearPairedDevice();
        }
        //clear the Available devices.
        if (NULL != m_bluetoothAvailableDeviceModel) {
            m_bluetoothAvailableDeviceModel->clearAvailableDevice();
        }
    }

    //if the scanModeState is false,  set scan mode after power on
    if ((true == m_powerState) && (NULL != m_localDevice)) {
        LOGD(tag, "setScanMode\n");
        E_GAP_SCAN_MODE scan_mode = SCAN_MODE_PAGE_ON_INQUIRY_ON_LOW;
        m_localDevice->setScanMode(scan_mode);
    }

    //if the autoConnectState is true and the autoConnectAddress is not empty, do the auto connect
    if ((true == m_powerState) && (true == m_autoConnectState) &&
        ("00:00:00:00:00:00" != m_autoConnectAddress)) {

        checkProfileState(pairedDeviceState, m_autoConnectAddress);

        LOGD(tag, "doAutoConnect\n");
        createRemoteDevice(m_autoConnectAddress);
        if (NULL != m_remoteDevice) {
            E_GAP_PAIR_STATE state = BONDSTATE_UNBOND;
            m_remoteDevice->getPairState(state);
            if (state == BONDSTATE_BONDED) {
                //send the connect request
                LOGD(tag, "connect\n");
                m_remoteDevice->connect();
                startAutoConnectTimer();
            }
        }
    } else if ((true == m_powerState) && (false == m_autoConnectState)) {
        checkProfileState(pairedDeviceState, m_autoConnectAddress);
    }
}

//get the scan mode active result form gapcallback
void CBluetoothPairRecordsPage::doScanModeActive(bool scanModeState)
{
    LOGD(tag, "doScanModeActive, scanModeState is %d\n", scanModeState);

    m_scanModeState = scanModeState;
}

//get the scan start response form gapcallback
void CBluetoothPairRecordsPage::doScanStartResponse()
{
    LOGD(tag, "doScanStartResponse\n");

    m_scanState = true;
    //send the scan state to the bluetoothPairedRecord UI
    emit scanStateChanged(m_scanState);

    //clear the AvailableDeviceModel
    if (NULL != m_bluetoothAvailableDeviceModel) {
        m_bluetoothAvailableDeviceModel->clearAvailableDevice();
    }
}

//get the scan stop response form gapcallback
void CBluetoothPairRecordsPage::doScanStopResponse()
{
    LOGD(tag, "doScanStopResponse\n");

    m_scanState = false;

    emit scanStateChanged(m_scanState);
}

//get the scan response form gapcallback
void CBluetoothPairRecordsPage::doScanResponse(
    QString btdev_address, QString btdev_name)
{
    LOGD(tag, "doScanResponse, btdev_address is %s, btdev_name is %s\n",
        BluetoothUtils::StringForLog(btdev_address.toStdString()).c_str(),
        (btdev_name.toStdString()).c_str());

    if ("" == btdev_name) {
        LOGE(tag, "btname is invalid");
        return;
    }

    //check the device's name weather have new line ('\n')
    checkNewline(btdev_name);

    if ((NULL != m_bluetoothAvailableDeviceModel) && (NULL != m_bluetoothPairedDeviceModel)) {
        if ((false == m_bluetoothAvailableDeviceModel->isInAvailableDeviceModel(btdev_address)) &&
             (false == m_bluetoothPairedDeviceModel->isInPairedDeviceModel(btdev_address))) {
            //When the new device is not in availableDeviceModel && PairedDeviceModel, add it to availableDeviceModel
            m_bluetoothAvailableDeviceModel->addAvailableDevice(
                CBluetoothAvailableDevice(btdev_name, btdev_address, STRING_UNPAIR, false, false));
        }
    }
}

//get the update name response form gapcallback
void CBluetoothPairRecordsPage::doUpdateNameResponse(
    QString btdev_address, QString btdev_name)
{
    LOGD(tag, "doUpdateNameResponse, btdev_address is %s, btdev_name is %s\n",
        BluetoothUtils::StringForLog(btdev_address.toStdString()).c_str(),
        (btdev_name.toStdString()).c_str());

    if(("" == btdev_name) && ("" != btdev_address)) {
        btdev_name = btdev_address;
    }

    checkNewline(btdev_name);

    if ((NULL != m_bluetoothAvailableDeviceModel) && (NULL != m_bluetoothPairedDeviceModel)) {
        if (false == m_bluetoothPairedDeviceModel->isInPairedDeviceModel(btdev_address)) {
            //update the availableDevice's name
            m_bluetoothAvailableDeviceModel->updateAvailableDeviceName(btdev_name, btdev_address);
        }
    }
}

//get the secure user confirm request form gapcallback, and then send the secure user confirm fro true
void CBluetoothPairRecordsPage::doSecureUserConfirmRequest(QString btdev_address)
{
    LOGD(tag, "doSecureUserConfirmRequest, btdev_address is %s\n",
        BluetoothUtils::StringForLog(btdev_address.toStdString()).c_str());

    createRemoteDevice(btdev_address);

    if(NULL != m_remoteDevice) {
        LOGI(tag, "secureUserConfirm\n");
        m_remoteDevice->secureUserConfirm(true);
    }
}

//get the paired response form gapcallback
void CBluetoothPairRecordsPage::doPairResponse(
    QString btdev_address, QString btdev_name, bool pairState)
{
    LOGD(tag, "doPairResponse, btdev_address is %s, btdev_name is %s, pairState is %d, threadId:%u\n",
        BluetoothUtils::StringForLog(btdev_address.toStdString()).c_str(),
        (btdev_name.toStdString()).c_str(),
        pairState,
        (unsigned int)pthread_self());

    if(("" == btdev_name) && ("" != btdev_address)) {
        btdev_name = btdev_address;
    }

    checkNewline(btdev_name);
    bool phoneAudioState = false;
    bool mediaAudioState = false;
    bool A2DPState = false;
    bool AVRCPState = false;

    if ((true == pairState) && (NULL != m_bluetoothAvailableDeviceModel) &&
        (NULL != m_bluetoothPairedDeviceModel)) {
        if (true == m_bluetoothAvailableDeviceModel->isInAvailableDeviceModel(btdev_address)) {
            //search the select connect state
            m_bluetoothAvailableDeviceModel->searchAvailableDeviceSelectConnectState(
                btdev_address, phoneAudioState, mediaAudioState);
            LOGI(tag, "phoneAudioState is %d, mediaAudioState is %d\n",
                    phoneAudioState, mediaAudioState);

            //delete the device from avalableDeviceModel
            m_bluetoothAvailableDeviceModel->deleteAvailableDevice(btdev_address);
        }
        if (false == m_bluetoothPairedDeviceModel->isInPairedDeviceModel(btdev_address)) {
            //add the new device to pairedDeviceModel
            m_bluetoothPairedDeviceModel->addPairedDevice(
                CBluetoothPairedDevice(btdev_name, btdev_address, STRING_PAIRED,
                    phoneAudioState, mediaAudioState, A2DPState, AVRCPState));

            int index = m_bluetoothPairedDeviceModel->rowCount() - 1;
            LOGI(tag, "index is %d\n", index);

            if ((0 <= index) && (m_initPairDeviceList.indexOf(btdev_address) >= 0)) {
                if ((false == phoneAudioState) && (false == mediaAudioState)) {
                    //the local user do not select the needed profile, connect all profile
                    connectRequest(index);
                } else {
                    selectConnectRequest(index);
                }
            }
        }
        E_BLUETOOTH_PROFILE_STATE hfpConnectState = BLUETOOTH_PROFILE_IDLE;
        BluetoothAddress hfpConnectedAddress;
        m_hfpInterface->getState(hfpConnectState);
        m_hfpInterface->getConnectedDevice(hfpConnectedAddress);
        QString hfpAddrString = QString::fromStdString(hfpConnectedAddress.toString());

        E_BLUETOOTH_PROFILE_STATE a2dpConnectState = BLUETOOTH_PROFILE_IDLE;
        BluetoothAddress a2dpConnectedAddress;
        m_a2dpInterface->getState(a2dpConnectState);
        m_a2dpInterface->getConnectedDevice(a2dpConnectedAddress);
        QString a2dpAddrString = QString::fromStdString(a2dpConnectedAddress.toString());

        E_BLUETOOTH_PROFILE_STATE avrcpConnectState = BLUETOOTH_PROFILE_IDLE;
        BluetoothAddress avrcpConnectedAddress;
        m_avrcpInterface->getState(avrcpConnectState);
        m_avrcpInterface->getConnectedDevice(avrcpConnectedAddress);
        QString avrcpAddrString = QString::fromStdString(avrcpConnectedAddress.toString());

        if (hfpConnectState == BLUETOOTH_PROFILE_CONNECTED
            && hfpAddrString == btdev_address) {
            LOGI(tag, "hfp connected, doHfpConnectedResponse");
            doHFConnectedResponse(btdev_address, true);
        }
        if (a2dpConnectState == BLUETOOTH_PROFILE_CONNECTED
            && a2dpAddrString == btdev_address) {
            LOGI(tag, "a2dp connected, doA2dpConnectedResponse");
            doA2DPConnectedResponse(btdev_address, true);
        }
        if (avrcpConnectState == BLUETOOTH_PROFILE_CONNECTED
            && avrcpAddrString == btdev_address) {
            LOGI(tag, "avrcp connected, doAvrcpConnectedResponse");
            doAVRCPConnectedResponse(btdev_address, true);
        }

    } else if ((false == pairState) &&
        (NULL != m_bluetoothAvailableDeviceModel) && (NULL != m_bluetoothPairedDeviceModel)) {
        doPairRemoveResponse(btdev_address, btdev_name);
        emit remindPairFail();
        m_initPairDeviceList.removeAll(btdev_address);
    }
}

//get the paired remove response form gapcallback
void CBluetoothPairRecordsPage::doPairRemoveResponse(
    QString btdev_address, QString btdev_name)
{
    LOGD(tag, "doPairRemoveResponse, btdev_address is %s, btdev_name is %s\n",
        BluetoothUtils::StringForLog(btdev_address.toStdString()).c_str(),
        (btdev_name.toStdString()).c_str());

    if (btdev_address == m_autoConnectAddress) {
        g_bluetoothMemData.memBluetoothAddress = 0;
        int save_ret = saveBluetoothConfiguration();
        if (1 == save_ret) {
            LOGD(tag, "save BluetoothAddress OK!");
        } else if (0 == save_ret) {
            LOGE(tag, "save BluetoothAddress Fail!");
        }
    }

    if(("" == btdev_name) && ("" != btdev_address)) {
        btdev_name = btdev_address;
    }

    checkNewline(btdev_name);

    if ((NULL != m_bluetoothAvailableDeviceModel) && (NULL != m_bluetoothPairedDeviceModel) &&
        (true == m_bluetoothPairedDeviceModel->isInPairedDeviceModel(btdev_address))) {
        //delete the device from pairedDeviceModel
        m_bluetoothPairedDeviceModel->deletePairedDevice(btdev_address);

        if (false == m_bluetoothAvailableDeviceModel->isInAvailableDeviceModel(btdev_address)) {
            //add the device to availableDeviceModel
            m_bluetoothAvailableDeviceModel->addAvailableDevice(
                CBluetoothAvailableDevice(btdev_name, btdev_address,
                    STRING_UNPAIR, false, false));
        }
    }
}

void CBluetoothPairRecordsPage::doUuidsUpdate(QString btdev_address, QString btdev_name)
{
    bool phoneAudioState = false;
    bool mediaAudioState = false;

    if (NULL != m_bluetoothPairedDeviceModel &&
            m_bluetoothPairedDeviceModel->isInPairedDeviceModel(btdev_address)) {
        int index = m_bluetoothPairedDeviceModel->rowCount() - 1;

        m_bluetoothPairedDeviceModel->searchPairedDeviceSelectConnectState(btdev_address,
                phoneAudioState, mediaAudioState);

        if ((0 <= index) && (m_initPairDeviceList.indexOf(btdev_address) >= 0)) {
            if ((false == phoneAudioState) && (false == mediaAudioState)) {
                //the local user do not select the needed profile, connect all profile
                connectRequest(index);
            } else {
                selectConnectRequest(index);
            }
        }
    }

}

//try to reconnect within 1 min
void CBluetoothPairRecordsPage::doGapLinkStateInd(QString btdev_address)
{
    LOGI(tag, "doGapLinkStateInd, btdev_address is %s\n",
        BluetoothUtils::StringForLog(btdev_address.toStdString()).c_str());
    do {
        if (false == m_powerState || false == m_autoConnectState) {
            break;
        }

        if ("" == btdev_address || "00:00:00:00:00:00" == btdev_address) {
            break;
        }

        E_BLUETOOTH_PROFILE_STATE hfpState = BLUETOOTH_PROFILE_IDLE;
        E_BLUETOOTH_PROFILE_STATE a2dpState = BLUETOOTH_PROFILE_IDLE;
        int ret = 0;

        if (NULL != m_hfpInterface) {
            ret = m_hfpInterface->getState(hfpState);
        }

        if (NULL != m_a2dpInterface) {
            ret = m_a2dpInterface->getState(a2dpState);
        }
        LOGD(tag, "hfp state (%d) a2dp state(%d)\n", hfpState, a2dpState);

        if (BLUETOOTH_PROFILE_CONNECTED != hfpState
                && BLUETOOTH_PROFILE_CONNECTED != a2dpState) {
            //connect all profile
            LOGI(tag, "connecte address : %s\n",
                BluetoothUtils::StringForLog(btdev_address.toStdString()).c_str());
            startAutoConnectTimer();
            saveLatestConnectedAddress(btdev_address);
            reconnectProfile(btdev_address);
            break;
        }

        BluetoothAddress device;

        if (BLUETOOTH_PROFILE_CONNECTED == hfpState
                && BLUETOOTH_PROFILE_IDLE == a2dpState ) {
            ret = m_hfpInterface->getConnectedDevice(device);
            if ((BLUETOOTH_OK == ret) && (btdev_address != addressToQString(device))) {
                //connect a2dp profile
                createRemoteDevice(btdev_address);
                if (NULL == m_remoteDevice) {
                    break;
                }
                E_GAP_PAIR_STATE state = BONDSTATE_UNBOND;
                m_remoteDevice->getPairState(state);
                if (BONDSTATE_BONDED != state) {
                    //no pair, not connecte
                    break;
                }
                LOGI(tag, "only connecte a2dp : %s\n",
                    BluetoothUtils::StringForLog(btdev_address.toStdString()).c_str());
                m_remoteDevice->connect(A2DPPROFILENAME);
            }
            break;
        }

        if (BLUETOOTH_PROFILE_CONNECTED == a2dpState
                && BLUETOOTH_PROFILE_IDLE == hfpState ) {
            ret = m_a2dpInterface->getConnectedDevice(device);
            if ((BLUETOOTH_OK == ret) && (btdev_address != addressToQString(device))) {
                //connect hfp client profile
                createRemoteDevice(btdev_address);
                if (NULL == m_remoteDevice) {
                    break;
                }
                E_GAP_PAIR_STATE state = BONDSTATE_UNBOND;
                m_remoteDevice->getPairState(state);
                if (BONDSTATE_BONDED != state) {
                    //no pair, not connecte
                    break;
                }
                LOGI(tag, "only connecte hfp : %s\n",
                    BluetoothUtils::StringForLog(btdev_address.toStdString()).c_str());
                m_remoteDevice->connect(HANDSFREEPROFILENAME);
            }
            break;
        }

    } while (0);

}

//get the connecting response form hfpcallback
void CBluetoothPairRecordsPage::doHFConnectingResponse(QString btdev_address)
{
    LOGD(tag, "doHFConnectingResponse, btdev_address is %s\n",
        BluetoothUtils::StringForLog(btdev_address.toStdString()).c_str());

    if (NULL != m_bluetoothPairedDeviceModel) {
        //get the pairedDevice's state from PairedDeviceModel
        QString pairedDeviceState = STRING_PAIRED;
        m_bluetoothPairedDeviceModel->getPairedDeviceState(btdev_address, pairedDeviceState);

        if (STRING_PAIRED == pairedDeviceState) {
            pairedDeviceState = STRING_CONNECTING;
        } else if (STRING_MEDIAAUDIO_CONNECTED == pairedDeviceState) {
            pairedDeviceState = STRING_PHONEAUDIO_CONNECTING;
        }
        LOGI(tag, "pairedDeviceState is %s\n", (pairedDeviceState.toStdString()).c_str());

        if (true == m_bluetoothPairedDeviceModel->isInPairedDeviceModel(btdev_address)) {
            //update the pairedDevice's state
            m_bluetoothPairedDeviceModel->updatePairedDeviceStateString(btdev_address, pairedDeviceState);
        }
    }
}

bool CBluetoothPairRecordsPage::isRemoteDeviceBonded(QString btdev_address)
{
    E_GAP_PAIR_STATE state = BONDSTATE_UNBOND;
    createRemoteDevice(btdev_address);
    m_remoteDevice->getPairState(state);
    LOGD(tag, "isRemoteDeviceBonded::state is:%d", state);
    if (state != BONDSTATE_BONDED) {
        return false;
    } else {
        return true;
    }
}

//get the connected response form hfpcallback
void CBluetoothPairRecordsPage::doHFConnectedResponse(
    QString btdev_address, bool hfpConnectedResult)
{
    LOGD(tag, "doHFConnectedResponse, btdev_address is %s, hfpConnectedResult is %d\n",
        BluetoothUtils::StringForLog(btdev_address.toStdString()).c_str(),
        hfpConnectedResult, (unsigned int)pthread_self());
    if (!isRemoteDeviceBonded(btdev_address)) {
        return;
    }

    getPhoneAudioState();
    setBluetoothConnectState();

    if (isProfileConnected()) {
        stopAutoConnectTimer();
    }

    updateDeviceState(btdev_address);

    if (hfpConnectedResult) {
        saveLatestConnectedAddress(btdev_address);
    }

    if (btdev_address == m_selectPairedAddress) {
        emit updateCheckBoxPhoneConnectState(hfpConnectedResult);
    }
}

//get the disconnected response form hfpcallback
void CBluetoothPairRecordsPage::doHFDisconnectedResponse(QString btdev_address)
{
    LOGD(tag, "doHFDisconnectedResponse, btdev_address is %s\n",
        BluetoothUtils::StringForLog(btdev_address.toStdString()).c_str());

    getPhoneAudioState();
    setBluetoothConnectState();

    updateDeviceState(btdev_address);

    if (btdev_address == m_selectPairedAddress) {
        emit updateCheckBoxPhoneConnectState(false);
    }
}

//get the connected response form a2dpcallback
void CBluetoothPairRecordsPage::doA2DPConnectedResponse(
    QString btdev_address, bool a2dpConnectedResult)
{
    LOGD(tag, "doA2DPConnectedResponse, btdev_address is %s, a2dpConnectedResult is %d, threadId:%u\n",
        BluetoothUtils::StringForLog(btdev_address.toStdString()).c_str(),
        a2dpConnectedResult, (unsigned int)pthread_self());
    if (!isRemoteDeviceBonded(btdev_address)) {
        return;
    }

    doMediaConnectedResponse(btdev_address, a2dpConnectedResult);
}

//get the disconnected response form a2dpcallback
void CBluetoothPairRecordsPage::doA2DPDisconnectedResponse(QString btdev_address)
{
    LOGD(tag, "doA2DPDisconnectedResponse, btdev_address is %s\n",
        BluetoothUtils::StringForLog(btdev_address.toStdString()).c_str());

    doMediaDisconnectedResponse(btdev_address);
}

//get the connected response form avrcpcallback
void CBluetoothPairRecordsPage::doAVRCPConnectedResponse(
    QString btdev_address, bool avrcpConnectedResult)
{
    LOGD(tag, "doAVRCPConnectedResponse, btdev_address is %s, avrcpConnectedResult is %d\n",
        BluetoothUtils::StringForLog(btdev_address.toStdString()).c_str(), avrcpConnectedResult);
    if (!isRemoteDeviceBonded(btdev_address)) {
        return;
    }

    doMediaConnectedResponse(btdev_address, avrcpConnectedResult);
}

//get the disconnected response form avrcpcallback
void CBluetoothPairRecordsPage::doAVRCPDisconnectedResponse(QString btdev_address)
{
    LOGD(tag, "doAVRCPDisconnectedResponse, btdev_address is %s\n",
        BluetoothUtils::StringForLog(btdev_address.toStdString()).c_str());

    doMediaDisconnectedResponse(btdev_address);
}

//get the auto connect state from setting page or when first open the bluetooth app(the default state is false)
void CBluetoothPairRecordsPage::doAutoConnectStateChanged(bool autoConnectState)
{
    LOGD(tag, "doAutoConnectStateChanged, autoConnectState is %d\n", autoConnectState);

    m_autoConnectState = autoConnectState;
}

//when first open bluetooth app, get the auto connect address(the default address is 0)
void CBluetoothPairRecordsPage::doAutoConnectAddress(long long bluetoothAddress)
{
    BluetoothAddress bt_address = bluetoothAddress;
    string address = bt_address.toString();
    m_autoConnectAddress = QString::fromStdString(address);
    LOGI(tag, "doAutoConnectAddress, address is %s\n",
        BluetoothUtils::StringForLog(address).c_str());
}

void CBluetoothPairRecordsPage::doA2DPConnectRequest()
{
    LOGD(tag, "doA2DPConnectRequest\n");

    QString pairedDeviceAddress = "";
    bool ret = getPairedDeviceAddress(pairedDeviceAddress);
    if (true == ret) {
        createRemoteDevice(pairedDeviceAddress);
        if((NULL != m_remoteDevice) && (true == m_powerState)) {
            if (false == m_a2dpConnectState) {
                //connect a2dp
                LOGD(tag, "connect a2dp before\n");
                m_remoteDevice->connect(A2DPPROFILENAME);
                LOGD(tag, "connect a2dp after\n");
            }
        }
    }
}

void CBluetoothPairRecordsPage::doAVRCPConnectRequest()
{
    LOGD(tag, "doAVRCPConnectRequest\n");

    QString pairedDeviceAddress = "";
    bool ret = getPairedDeviceAddress(pairedDeviceAddress);
    if (true == ret) {
        createRemoteDevice(pairedDeviceAddress);
        if((NULL != m_remoteDevice) && (true == m_powerState)) {
            if (false == m_avrcpConnectState) {
                //connect avrcp
                LOGD(tag, "connect avrcp before\n");
                m_remoteDevice->connect(AVRCPPROFILENAME);
                LOGD(tag, "connect avrcp after\n");
            }
        }
    }
}

void CBluetoothPairRecordsPage::doLanguageChanged ()
{
    LOGI(tag, "doLanguageChanged\n");

    getPairedListFromStorage();

    QString pairedDeviceState = STRING_PAIRED;
    checkProfileState(pairedDeviceState, m_autoConnectAddress);

}

void CBluetoothPairRecordsPage::doAutoConnectTimeout()
{
    LOGD(tag, "doAutoConnectTimeout\n");

    bool isConnected = isProfileConnected();
    m_Timeoff += AUTO_CONNECT_TIMEOUT;
    LOGD(tag, "isConnected is %d\n", isConnected);

    if (isConnected || m_Timeoff > AUTO_CONNECT_MAX_TRY_TIME * AUTO_CONNECT_TIMEOUT) {
        stopAutoConnectTimer();
    } else if (false == isConnected) {
        LOGD(tag, "isConnected222 is %d\n", isConnected);
        reconnectProfile(m_autoConnectAddress);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//when first open bluetooth app, get the paired device list from storage
void CBluetoothPairRecordsPage::getPairedListFromStorage()
{
    vector<BluetoothRemoteDevice> m_bluetoothRemoteDevices;

    //clear the Paired devices.
    if (NULL != m_bluetoothPairedDeviceModel) {
        m_bluetoothPairedDeviceModel->clearPairedDevice();
    }

    if (NULL != m_localDevice) {
        m_localDevice->getBondedDevices(m_bluetoothRemoteDevices);
    }

    vector<BluetoothRemoteDevice>::iterator m_iterator;

    for (m_iterator = m_bluetoothRemoteDevices.begin();
        m_iterator != m_bluetoothRemoteDevices.end(); m_iterator++) {

        BluetoothRemoteDevice newDevice = *m_iterator;
        BluetoothAddress m_addr= newDevice.getAddress();
        string m_strAddr = m_addr.toString();
        string m_strName = newDevice.getName();

        QString btdev_address = QString::fromStdString(m_strAddr);
        QString btdev_name = QString::fromStdString(m_strName);
        if("" == btdev_name && "" != btdev_address) {
            btdev_name = btdev_address;
        }

        checkNewline(btdev_name);

        if (NULL != m_bluetoothPairedDeviceModel) {
            m_bluetoothPairedDeviceModel->addPairedDevice(
                CBluetoothPairedDevice(btdev_name, btdev_address,
                    STRING_PAIRED, false, false, false, false));
        }
    }
}

//after power on, check the needed profile state
bool CBluetoothPairRecordsPage::checkProfileState(
    QString &pairedDeviceState, QString autoConnectAddress)
{
    E_BLUETOOTH_PROFILE_STATE state = BLUETOOTH_PROFILE_IDLE;
    BluetoothAddress deviceHFP = "";
    BluetoothAddress deviceAVRCP = "";
    BluetoothAddress deviceA2DP = "";
    QString deviceHFPQString = "";
    QString deviceAVRCPQString = "";
    QString deviceA2DPQString = "";
    int ret = BLUETOOTH_ERROR;
    bool isAutoConnectAddress = false;
    if (NULL != m_hfpInterface) {
        m_hfpInterface->getState(state);
        ret = m_hfpInterface->getConnectedDevice(deviceHFP);

        deviceHFPQString = addressToQString(deviceHFP);

        if (autoConnectAddress == deviceHFPQString) {
            isAutoConnectAddress = true;
        }
        if ((0 <= ret) && (BLUETOOTH_PROFILE_CONNECTED == state)) {
            pairedDeviceState = STRING_PHONEAUDIO_CONNECTED;
            m_phoneConnectState = true;
            emit sigPhoneConnectState(m_phoneConnectState);

            setBluetoothConnectState();

            if ((NULL != m_bluetoothPairedDeviceModel) && (STRING_PAIRED != pairedDeviceState)) {
                //update the pairedDevice's state
                m_bluetoothPairedDeviceModel->updatePairedDeviceStateString(
                    deviceHFPQString, pairedDeviceState);
                m_bluetoothPairedDeviceModel->updatePairedDeviceHFPState(
                    deviceHFPQString, m_phoneConnectState);
            }

            int callState = m_hfpInterface->getCallState();
            emit sigCallStateChange(callState);
            if (HF_CALLSTATE_IDLE != callState) {
                string callNumber = "";
                ret = m_hfpInterface->getCallingNumber(callNumber);
                LOGI(tag, "callNumber is %s\n", callNumber.c_str());
                if (0 <= ret) {
                    emit sigCallNumber(QString::fromStdString(callNumber));
                }
            }
        }
    }
    if (NULL != m_avrcpInterface) {
        m_avrcpInterface->getState(state);
        ret = m_avrcpInterface->getConnectedDevice(deviceAVRCP);
        deviceAVRCPQString = addressToQString(deviceAVRCP);

        if (autoConnectAddress == deviceAVRCPQString) {
            isAutoConnectAddress = true;
        }
        if ((0 <= ret) && (BLUETOOTH_PROFILE_CONNECTED == state)) {
            m_avrcpConnectState = true;
            emit sigAVRCPConnectState(m_avrcpConnectState);

            checkMediaProfileState(pairedDeviceState, deviceHFPQString, deviceAVRCPQString);
        }
    }
    if (NULL != m_a2dpInterface) {
        m_a2dpInterface->getState(state);

        ret = m_a2dpInterface->getConnectedDevice(deviceA2DP);
        deviceA2DPQString = addressToQString(deviceA2DP);

        if (autoConnectAddress == deviceA2DPQString) {
            isAutoConnectAddress = true;
        }

        if ((0 <= ret) && (BLUETOOTH_PROFILE_CONNECTED == state)) {
            m_a2dpConnectState = true;
            emit sigA2DPConnectState(m_a2dpConnectState);

            checkMediaProfileState(pairedDeviceState, deviceHFPQString, deviceA2DPQString);
        }
    }

    return isAutoConnectAddress;
}

//check the a2dp/avrcp profile state
void CBluetoothPairRecordsPage::checkMediaProfileState(
    QString &pairedDeviceState, QString phoneAddress, QString mediaAddress)
{
    LOGD(tag, "checkMediaProfileState\n");

    checkMediaConnectState();
    setBluetoothConnectState();

    if ((phoneAddress == mediaAddress) &&
        STRING_PHONEAUDIO_CONNECTED == pairedDeviceState) {
        pairedDeviceState = STRING_PHONEAUDIO_MEDIAAUDIO_CONNECTED;
    } else if (phoneAddress != mediaAddress) {
        pairedDeviceState = STRING_MEDIAAUDIO_CONNECTED;
    }

    if ((NULL != m_bluetoothPairedDeviceModel) && (STRING_PAIRED != pairedDeviceState)) {
        //update the pairedDevice's state
        m_bluetoothPairedDeviceModel->updatePairedDeviceStateString(
            mediaAddress, pairedDeviceState);
        m_bluetoothPairedDeviceModel->updatePairedDeviceMediaAudioState(
            mediaAddress, m_mediaConnectState);
    }
}

//creat the remote device depend on address
void CBluetoothPairRecordsPage::createRemoteDevice(QString btdev_address)
{
    LOGD(tag, "createRemoteDevice\n");

    if (NULL !=  m_remoteDevice) {
        delete  m_remoteDevice;
        m_remoteDevice = NULL;
    }

    m_remoteDevice = new BluetoothRemoteDevice(btdev_address.toStdString());
    if(NULL != m_remoteDevice) {
        LOGD(tag, "new remotedevice OK, m_remoteDevice is %p\n", m_remoteDevice);
    } else {
        LOGD(tag, "new remotedevice FALL!\n");
    }
}

//check the name weather have new line ('\n')
void CBluetoothPairRecordsPage::checkNewline(QString &btdev_name)
{
    LOGD(tag, "checkNewline\n");

    for (int i = 0; i < btdev_name.size(); i++) {
        //if the name have new line, remove '\n'
        if ('\n' == btdev_name.at(i)) {
            btdev_name.remove(i, 1);
        }
    }
}

//add a new paired device to pairedDeviceModel
void CBluetoothPairRecordsPage::addNewPairedDevice(
    QString btdev_address, QString btdev_name, QString btdev_state)
{
    LOGD(tag, "addNewPairedDevice\n");

    bool phoneAudioState = false;
    bool mediaAudioState = false;
    bool A2DPState = false;
    bool AVRCPState = false;

    if (true == m_bluetoothAvailableDeviceModel->isInAvailableDeviceModel(btdev_address)) {
        //search the select connect state
        m_bluetoothAvailableDeviceModel->searchAvailableDeviceSelectConnectState(
            btdev_address, phoneAudioState, mediaAudioState);
        LOGI(tag, "phoneAudioState is %d, mediaAudioState is %d\n",
            phoneAudioState, mediaAudioState);

        //delete the device from availableDeviceModel
        m_bluetoothAvailableDeviceModel->deleteAvailableDevice(btdev_address);
    }
    //add the new device to pairedDeviceModel
    m_bluetoothPairedDeviceModel->addPairedDevice(
        CBluetoothPairedDevice(btdev_name, btdev_address, btdev_state,
        phoneAudioState, mediaAudioState, A2DPState, AVRCPState));
}

//the local device select the needed profile to connect
void CBluetoothPairRecordsPage::selectConnectRequest(int index)
{
    LOGD(tag, "selectConnectRequest, index is %d\n", index);

    if ((NULL != m_bluetoothPairedDeviceModel) && (-1 != index)) {
        bool phoneAudioState = false;
        bool mediaAudioState = false;

        if (false == m_bluetoothPairedDeviceModel->checkPairedDeviceState(index,
                STRING_PHONEAUDIO_MEDIAAUDIO_CONNECTED)) {

            stopScanRequest();

            QString pairedDeviceState = "";
            m_bluetoothPairedDeviceModel->getPairedDeviceState(index, pairedDeviceState);

            m_bluetoothPairedDeviceModel->searchPairedDeviceSelectConnectState(
                index, phoneAudioState, mediaAudioState);

            if ((true == phoneAudioState) || (true == mediaAudioState)) {
                QString pairedDeviceAddress = "";
                m_bluetoothPairedDeviceModel->searchPairedDeviceAddress(index, pairedDeviceAddress);

                createRemoteDevice(pairedDeviceAddress);

                if((NULL != m_remoteDevice) && (true == m_powerState)) {
                    LOGI(tag, "m_remoteDevice is %p\n", m_remoteDevice);
                    BluetoothAddress bt_address = m_remoteDevice->getAddress();
                    string address = bt_address.toString();
                    LOGI(tag, "address is %s\n", BluetoothUtils::StringForLog(address).c_str());

                    if (STRING_PAIRED == pairedDeviceState ||
                        STRING_CONNECTING == pairedDeviceState) {
                        if ((true == phoneAudioState) && (true == mediaAudioState)) {
                            //connect all profiles
                            LOGD(tag, "connect all profiles before\n");
                            m_remoteDevice->connect();
                            LOGD(tag, "connect all profiles after\n");
                        } else if (true == phoneAudioState) {
                            //connect hfp
                            LOGD(tag, "connect hfp before\n");
                            m_remoteDevice->connect(HANDSFREEPROFILENAME);
                            LOGD(tag, "connect hfp after\n");
                        } else if (true == mediaAudioState){
                            //connect a2dp
                            LOGD(tag, "connect a2dp before\n");
                            m_remoteDevice->connect(A2DPPROFILENAME);
                            LOGD(tag, "connect a2dp after\n");
                            //connect avrcp
                            LOGD(tag, "connect avrcp before\n");
                            m_remoteDevice->connect(AVRCPPROFILENAME);
                            LOGD(tag, "connect avrcp after\n");
                        }
                    } else if (STRING_PHONEAUDIO_CONNECTED == pairedDeviceState) {
                        if (true == mediaAudioState){
                            //connect a2dp
                            LOGD(tag, "connect a2dp before\n");
                            m_remoteDevice->connect(A2DPPROFILENAME);
                            LOGD(tag, "connect a2dp after\n");
                            //connect avrcp
                            LOGD(tag, "connect avrcp before\n");
                            m_remoteDevice->connect(AVRCPPROFILENAME);
                            LOGD(tag, "connect avrcp after\n");
                        }
                    } else if (STRING_MEDIAAUDIO_CONNECTED == pairedDeviceState) {
                        if (true == phoneAudioState){
                            //connect hfp
                            LOGD(tag, "connect hfp before\n");
                            m_remoteDevice->connect(HANDSFREEPROFILENAME);
                            LOGD(tag, "connect hfp after\n");
                        }
                    }
                } else {
                    emit waitToPower();
                }
            }
        }
    }
}

//the local device select the needed profile to disconnect
void CBluetoothPairRecordsPage::selectDisconnectRequest(int index)
{
    LOGD(tag, "selectDisconnectRequest, index is %d\n", index);

    if ((NULL != m_bluetoothPairedDeviceModel) && (-1 != index)) {
        bool phoneAudioState = false;
        bool mediaAudioState = false;

        if (false == m_bluetoothPairedDeviceModel->checkPairedDeviceState(
            index, STRING_PAIRED)) {

            stopScanRequest();

            QString pairedDeviceState = "";
            m_bluetoothPairedDeviceModel->getPairedDeviceState(index, pairedDeviceState);

            m_bluetoothPairedDeviceModel->searchPairedDeviceSelectConnectState(
                index, phoneAudioState, mediaAudioState);

            if ((false == phoneAudioState) || (false == mediaAudioState)) {
                QString pairedDeviceAddress = "";
                m_bluetoothPairedDeviceModel->searchPairedDeviceAddress(index, pairedDeviceAddress);

                createRemoteDevice(pairedDeviceAddress);

                if(NULL != m_remoteDevice && true == m_powerState) {
                    LOGI(tag, "m_remoteDevice is %p\n", m_remoteDevice);
                    BluetoothAddress bt_address = m_remoteDevice->getAddress();
                    string address = bt_address.toString();
                    LOGI(tag, "address is %s\n", BluetoothUtils::StringForLog(address).c_str());

                    if (STRING_PHONEAUDIO_MEDIAAUDIO_CONNECTED == pairedDeviceState) {
                        if ((false == phoneAudioState) && (false == mediaAudioState)) {
                            //disconnect all profiles
                            LOGD(tag, "disconnect all profiles before\n");
                            m_remoteDevice->disconnect();
                            LOGD(tag, "disconnect all profiles after\n");
                        } else if (false == phoneAudioState) {
                            //disconnect hfp
                            LOGD(tag, "disconnect hfp before\n");
                            m_remoteDevice->disconnect(HANDSFREEPROFILENAME);
                            LOGD(tag, "disconnect hfp after\n");
                        } else if (false == mediaAudioState) {
                            //disconnect a2dp
                            LOGD(tag, "disconnect a2dp before\n");
                            m_remoteDevice->disconnect(A2DPPROFILENAME);
                            LOGD(tag, "disconnect a2dp after\n");
                            //disconnect avrcp
                            LOGD(tag, "disconnect avrcp before\n");
                            m_remoteDevice->disconnect(AVRCPPROFILENAME);
                            LOGD(tag, "disconnect avrcp after\n");
                        }
                    } else if (STRING_PHONEAUDIO_CONNECTED == pairedDeviceState) {
                        if (false == phoneAudioState) {
                            //disconnect hfp
                            LOGD(tag, "disconnect hfp before\n");
                            m_remoteDevice->disconnect(HANDSFREEPROFILENAME);
                            LOGD(tag, "disconnect hfp after\n");
                        }
                    } else if (STRING_MEDIAAUDIO_CONNECTED == pairedDeviceState) {
                        if (false == mediaAudioState) {
                            //disconnect a2dp
                            LOGD(tag, "disconnect a2dp before\n");
                            m_remoteDevice->disconnect(A2DPPROFILENAME);
                            LOGD(tag, "disconnect a2dp after\n");
                            //disconnect avrcp
                            LOGD(tag, "disconnect avrcp before\n");
                            m_remoteDevice->disconnect(AVRCPPROFILENAME);
                            LOGD(tag, "disconnect avrcp after\n");
                        }
                    }
                } else {
                    emit waitToPower();
                }
            }
        }
    }
}

//save the latest connected address, used to auto connect after reset
void CBluetoothPairRecordsPage::saveLatestConnectedAddress(QString btdev_address)
{
    LOGD(tag, "saveLatestConnectedAddress, btdev_address is %s\n",
         BluetoothUtils::StringForLog(btdev_address.toStdString()).c_str());

    m_autoConnectAddress = btdev_address;
    BluetoothAddress bt_address = btdev_address.toStdString();
    g_bluetoothMemData.memBluetoothAddress = bt_address.toLong64();
    LOGI(tag, "bluetoothAddress is %lld\n", g_bluetoothMemData.memBluetoothAddress);

    int save_ret = saveBluetoothConfiguration();
    if (1 == save_ret) {
        LOGD(tag, "save BluetoothAddress OK!\n");
    } else if (0 == save_ret) {
        LOGE(tag, "save BluetoothAddress Fail!\n");
    }
}

//change BluetoothAddress to QString
QString CBluetoothPairRecordsPage::addressToQString(BluetoothAddress address)
{
    string addressString = "";
    QString addressQString = "";

    addressString = address.toString();
    addressQString = QString::fromStdString(addressString);

    return addressQString;
}

void CBluetoothPairRecordsPage::checkMediaConnectState()
{
    //check a2dp/avrcp state, only both of them are disconnected, the media is disconnected
    if ((false == m_mediaConnectState) &&
        ((true == m_a2dpConnectState))) {
        m_mediaConnectState = true;
        emit sigMediaConnectState(m_mediaConnectState);
    }
    //check a2dp/avrcp state, if either one is connected, the media is connected
    else if ((false == m_a2dpConnectState) && (false == m_avrcpConnectState)) {
        m_mediaConnectState = false;
        emit sigMediaConnectState(m_mediaConnectState);
    }
}

void CBluetoothPairRecordsPage::setBluetoothConnectState()
{
    bool ret = false;
    if (false == m_mediaConnectState && false == m_phoneConnectState) {
        ret = GlobalBus::setState(GlobalBus::STATE_BT, GlobalBus::BT_DISCONNECT);
        if (true == ret) {
            LOGD(tag, "setState BT_DISCONNECT OK!\n");
        } else {
            LOGE(tag, "setState BT_DISCONNECT Fail!\n");
        }
    } else {
        ret = GlobalBus::setState(GlobalBus::STATE_BT, GlobalBus::BT_CONNECT);
        if (true == ret) {
            LOGD(tag, "setState BT_CONNECT OK!\n");
        } else {
            LOGE(tag, "setState BT_CONNECT Fail!\n");
        }
    }
}

//do a2dp/avrcp connected response
void CBluetoothPairRecordsPage::doMediaConnectedResponse(
    QString btdev_address, bool connectedResult)
{
    LOGD(tag, "doMediaConnectedResponse, btdev_address is %s, connectedResult is %d\n",
        BluetoothUtils::StringForLog(btdev_address.toStdString()).c_str(), connectedResult);

    getMediaAudioState();
    checkMediaConnectState();
    setBluetoothConnectState();

    if (isProfileConnected()) {
        stopAutoConnectTimer();
    }

    updateDeviceState(btdev_address);

    if (btdev_address == m_selectPairedAddress) {
        bool mediaAudioState = false;
        m_bluetoothPairedDeviceModel->getPairedDeviceMediaAudioState(btdev_address, mediaAudioState);
        emit updateCheckBoxMediaConnectState(mediaAudioState);
    }

    if (connectedResult) {
        saveLatestConnectedAddress(btdev_address);
    }
}

//do a2dp/avrcp disconnected response
void CBluetoothPairRecordsPage::doMediaDisconnectedResponse(QString btdev_address)
{
    LOGD(tag, "doMediaDisconnectedResponse, btdev_address is %s\n",
        BluetoothUtils::StringForLog(btdev_address.toStdString()).c_str());

    getMediaAudioState();
    checkMediaConnectState();
    setBluetoothConnectState();

    updateDeviceState(btdev_address);

    if (btdev_address == m_selectPairedAddress) {
        bool mediaAudioState = false;
        m_bluetoothPairedDeviceModel->getPairedDeviceMediaAudioState(btdev_address, mediaAudioState);
        emit updateCheckBoxMediaConnectState(mediaAudioState);
    }
}

bool CBluetoothPairRecordsPage::getPairedDeviceAddress(QString &pairedDeviceAddress)
{
    int ret = BLUETOOTH_ERROR;
    E_BLUETOOTH_PROFILE_STATE state = BLUETOOTH_PROFILE_IDLE;
    BluetoothAddress device = "";

    if (NULL != m_a2dpInterface) {
        LOGD(tag, "a2dp getState\n");
        ret = m_a2dpInterface->getState(state);
        if ((0 <= ret) && (BLUETOOTH_PROFILE_CONNECTED == state)) {
            ret = m_a2dpInterface->getConnectedDevice(device);
            if (0 <= ret) {
                pairedDeviceAddress = addressToQString(device);
            }
        } else {
            LOGD(tag, "a2dp state is %d\n", (int)state);
        }
    }

    if ((ret < 0) && (NULL != m_avrcpInterface)) {
        LOGD(tag, "avrcp getState\n");
        ret = m_avrcpInterface->getState(state);
        if ((0 <= ret) && (BLUETOOTH_PROFILE_CONNECTED == state)) {
            ret = m_a2dpInterface->getConnectedDevice(device);
            if (0 <= ret) {
                pairedDeviceAddress = addressToQString(device);
            }
        } else {
            LOGD(tag, "avrcp state is %d\n", (int)state);
        }
    }

    return (0 <= ret) ? true : false;
}

void CBluetoothPairRecordsPage::startAutoConnectTimer()
{
    if (NULL != m_autoConnectTimer) {
        LOGD(tag, "startAutoConnectTimer\n");
        m_Timeoff = 0;
        m_autoConnectTimer->start(AUTO_CONNECT_TIMEOUT);
    } else {
        LOGE(tag, "m_autoConnectTimer is null\n");
    }
}

void CBluetoothPairRecordsPage::stopAutoConnectTimer()
{
    if (NULL != m_autoConnectTimer && m_autoConnectTimer->isActive()) {
        LOGD(tag, "stopAutoConnectTimer\n");
        m_autoConnectTimer->stop();
    } else if (false == m_autoConnectTimer->isActive()) {
        LOGD(tag, "m_autoConnectTimer already stop\n");
    } else {
        LOGE(tag, "m_autoConnectTimer is null\n");
    }
}

bool CBluetoothPairRecordsPage::isProfileConnected()
{
    LOGD(tag, "isProfileConnected\n");

    E_BLUETOOTH_PROFILE_STATE state = BLUETOOTH_PROFILE_IDLE;
    int ret = BLUETOOTH_ERROR;
    bool isHFPConnected = false;
    bool isA2DPConnected = false;
    bool isAVRCPConeccted = false;

    if (NULL != m_hfpInterface) {
        LOGD(tag, "hfp getState\n");
        ret = m_hfpInterface->getState(state);
        if ((0 <= ret) && (BLUETOOTH_PROFILE_CONNECTED == state)) {
            isHFPConnected = true;
        } else {
            LOGD(tag, "hfp state is %d\n", (int)state);
        }
    }
    if (NULL != m_avrcpInterface) {
        LOGD(tag, "avrcp getState\n");
        ret = m_avrcpInterface->getState(state);
        if ((0 <= ret) && (BLUETOOTH_PROFILE_CONNECTED == state)) {
            isAVRCPConeccted = true;
        } else {
            LOGD(tag, "avrcp state is %d\n", (int)state);
        }
    }
    if (NULL != m_a2dpInterface) {
        LOGD(tag, "a2dp getState\n");
        ret = m_a2dpInterface->getState(state);
        if ((0 <= ret) && (BLUETOOTH_PROFILE_CONNECTED == state)) {
            isA2DPConnected = true;
        } else {
            LOGD(tag, "a2dp state is %d\n", (int)state);
        }
    }

    return (isHFPConnected && isA2DPConnected && isAVRCPConeccted);
}

void CBluetoothPairRecordsPage::reconnectProfile(QString btdev_address)
{
    LOGD(tag, "reconnectProfile\n");

    if ((true == m_powerState) && (true == m_autoConnectState) &&
        ("00:00:00:00:00:00" != btdev_address)) {
        createRemoteDevice(btdev_address);

        if(NULL != m_remoteDevice) {
            LOGI(tag, "m_remoteDevice is %p\n", m_remoteDevice);
            E_GAP_PAIR_STATE state = BONDSTATE_UNBOND;
            m_remoteDevice->getPairState(state);
            LOGD(tag, "the pair state is %d\n", (int)state);

            if (BONDSTATE_BONDED == state) {
                //send the connect request
                LOGD(tag, "try connect again\n");
                m_remoteDevice->connect();
            }
        }
    }
}

bool CBluetoothPairRecordsPage::getPhoneAudioState()
{
    LOGD(tag, "getPhoneAudioState\n");

    E_BLUETOOTH_PROFILE_STATE state = BLUETOOTH_PROFILE_IDLE;
    int ret = BLUETOOTH_ERROR;
    bool isHFPConnected = false;

    if (NULL != m_hfpInterface) {
        LOGD(tag, "hfp getState\n");
        ret = m_hfpInterface->getState(state);
        if ((0 <= ret) && (BLUETOOTH_PROFILE_CONNECTED == state)) {
            isHFPConnected = true;
        } else {
            LOGD(tag, "hfp state is %d\n", (int)state);
        }
    }
    m_phoneConnectState = isHFPConnected;
    LOGD(tag, "m_phoneConnectState is %d\n", m_phoneConnectState);
    emit sigPhoneConnectState(m_phoneConnectState);

    return (0 <= ret) ? true : false;
}

bool CBluetoothPairRecordsPage::getMediaAudioState()
{
    LOGD(tag, "getMediaAudioState\n");

    E_BLUETOOTH_PROFILE_STATE state = BLUETOOTH_PROFILE_IDLE;
    int ret = BLUETOOTH_ERROR;
    bool isA2DPConnected = false;
    bool isAVRCPConeccted = false;

    if (NULL != m_avrcpInterface) {
        LOGD(tag, "avrcp getState\n");
        ret = m_avrcpInterface->getState(state);
        if ((0 <= ret) && (BLUETOOTH_PROFILE_CONNECTED == state)) {
            isAVRCPConeccted = true;
        } else {
            LOGD(tag, "avrcp state is %d\n", (int)state);
        }
    }
    m_avrcpConnectState = isAVRCPConeccted;
    LOGD(tag, "m_avrcpConnectState is %d\n", m_avrcpConnectState);
    emit sigAVRCPConnectState(m_avrcpConnectState);

    if (NULL != m_a2dpInterface) {
        LOGD(tag, "a2dp getState\n");
        ret = m_a2dpInterface->getState(state);
        if ((0 <= ret) && (BLUETOOTH_PROFILE_CONNECTED == state)) {
            isA2DPConnected = true;
        } else {
            LOGD(tag, "a2dp state is %d\n", (int)state);
        }
    }
    m_a2dpConnectState = isA2DPConnected;
    LOGD(tag, "m_a2dpConnectState is %d\n", m_a2dpConnectState);
    emit sigA2DPConnectState(m_a2dpConnectState);

    return (0 <= ret) ? true : false;
}

void CBluetoothPairRecordsPage::getDeviceState(
    QString btdev_address, bool &isHFPConnected, bool &isA2DPConnected, bool &isAVRCPConnected)
{
    LOGD(tag, "getDeviceState\n");
    E_BLUETOOTH_PROFILE_STATE state = BLUETOOTH_PROFILE_IDLE;
    int ret = BLUETOOTH_ERROR;
    BluetoothAddress device = "";
    isHFPConnected = false;
    isA2DPConnected = false;
    isAVRCPConnected = false;

    if (NULL != m_hfpInterface) {
        LOGD(tag, "hfp getState\n");
        ret = m_hfpInterface->getState(state);
        if ((0 <= ret) && (BLUETOOTH_PROFILE_CONNECTED == state)) {
            ret = m_hfpInterface->getConnectedDevice(device);
            if ((0 <= ret) && (btdev_address == addressToQString(device))) {
                isHFPConnected = true;
            }
        } else {
            LOGD(tag, "hfp state is %d\n", (int)state);
        }
    }

    if (NULL != m_a2dpInterface) {
        LOGD(tag, "a2dp getState\n");
        ret = m_a2dpInterface->getState(state);
        if ((0 <= ret) && (BLUETOOTH_PROFILE_CONNECTED == state)) {
            ret = m_a2dpInterface->getConnectedDevice(device);
            if ((0 <= ret) && (btdev_address == addressToQString(device))) {
                isA2DPConnected = true;
            }
        } else {
            LOGD(tag, "a2dp state is %d\n", (int)state);
        }
    }

    if (NULL != m_avrcpInterface) {
        LOGD(tag, "avrcp getState\n");
        ret = m_avrcpInterface->getState(state);
        if ((0 <= ret) && (BLUETOOTH_PROFILE_CONNECTED == state)) {
            ret = m_a2dpInterface->getConnectedDevice(device);
            if ((0 <= ret) && (btdev_address == addressToQString(device))) {
                isAVRCPConnected = true;
            }
        } else {
            LOGD(tag, "avrcp state is %d\n", (int)state);
        }
    }
}

void CBluetoothPairRecordsPage::getDeviceStateString(
    bool isHFPConnected, bool isA2DPConnected, bool isAVRCPConnected, QString &pairedDeviceStateString)
{
    pairedDeviceStateString = STRING_PAIRED;

    if ((isA2DPConnected) && isHFPConnected) {
        pairedDeviceStateString = STRING_PHONEAUDIO_MEDIAAUDIO_CONNECTED;
    } else if ((isA2DPConnected) && !isHFPConnected) {
        pairedDeviceStateString = STRING_MEDIAAUDIO_CONNECTED;
    } else if (isHFPConnected) {
        pairedDeviceStateString = STRING_PHONEAUDIO_CONNECTED;
    }
}

void CBluetoothPairRecordsPage::updateDeviceState(QString btdev_address)
{
    bool isHFPConnected = false;
    bool isA2DPConnected = false;
    bool isAVRCPConnected = false;
    QString pairedDeviceStateString = STRING_PAIRED;

    getDeviceState(btdev_address, isHFPConnected, isA2DPConnected, isAVRCPConnected);
    getDeviceStateString(isHFPConnected, isA2DPConnected, isAVRCPConnected, pairedDeviceStateString);

    if (NULL != m_bluetoothPairedDeviceModel) {
        if (true == m_bluetoothPairedDeviceModel->isInPairedDeviceModel(btdev_address)) {
            //update the pairedDevice's state
            m_bluetoothPairedDeviceModel->updatePairedDeviceHFPState(
                btdev_address, isHFPConnected);
            m_bluetoothPairedDeviceModel->updatePairedDeviceA2DPState(
                btdev_address, isA2DPConnected);
            m_bluetoothPairedDeviceModel->updatePairedDeviceAVRCPState(
                btdev_address, isAVRCPConnected);
            m_bluetoothPairedDeviceModel->updatePairedDeviceStateString(
                btdev_address, pairedDeviceStateString);
        }
    }

}



