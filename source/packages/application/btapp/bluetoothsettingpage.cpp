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
 
#include "bluetoothsettingpage.h"
#include "bluetoothapplication.h"

using namespace std;
extern BluetoothMemData g_bluetoothMemData;
extern int saveBluetoothConfiguration();
static const char* const tag = "CBluetoothSettingPage";
const static int BLUETOOTH_SWITCH_TIMEOUT = 5 * 1000;
static const int POWER_OFF = 0;
static const int POWER_ON = 1;
static const int POWER_OPENING_OR_CLOSING = 2;

CBluetoothSettingPage::CBluetoothSettingPage()
    : m_autoAnswer(false)
    , m_autoConnect(false)
    , m_bluetoothPower(0)
    , m_bluetoothName("Linux BT")
    , m_bluetoothPassword("1234")
    , m_localDevice(NULL)
    , m_btSwitchTimer(NULL)
{
    m_btSwitchTimer = new QTimer(this);
    QObject::connect(m_btSwitchTimer, SIGNAL(timeout()),
            this, SLOT(doBluetoothSwitchTimeout()), Qt::QueuedConnection);

}

CBluetoothSettingPage::~CBluetoothSettingPage()
{
    LOGD(tag, "destructor\n");
    if (NULL != m_btSwitchTimer) {
        delete  m_btSwitchTimer;
        m_btSwitchTimer = NULL;
    }

}

void CBluetoothSettingPage::startBTSwitchTimer()
{
    LOGD(tag, "startBTSwitchTimer\n");
    if (NULL != m_btSwitchTimer) {
        LOGD(tag, "startbtSwitchTimer\n");
        m_btSwitchTimer->start(BLUETOOTH_SWITCH_TIMEOUT);
    } else {
        LOGE(tag, "m_btSwitchTimer is null\n");
    }
}

void CBluetoothSettingPage::stopBTSwitchTimer()
{
    LOGD(tag, "stopBTSwitchTimer\n");
    if (NULL != m_btSwitchTimer && m_btSwitchTimer->isActive()) {
        LOGD(tag, "stopbtSwitchTimer\n");
        m_btSwitchTimer->stop();
    } else if (false == m_btSwitchTimer->isActive()) {
        LOGD(tag, "m_btSwitchTimer already stop\n");
    } else {
        LOGE(tag, "m_btSwitchTimer is null\n");
    }
}


void CBluetoothSettingPage::doBluetoothSwitchTimeout()
{
    LOGD(tag, "doBluetoothSwitchTimeout\n");

    if (isBluetoothPowerOn()) {
        m_bluetoothPower = POWER_ON; // power on
    } else {
        m_bluetoothPower = POWER_OFF; // power off
    }
    emit bluetoothPowerStateChanged(m_bluetoothPower);
}


void CBluetoothSettingPage::initBluetoothSettingPage()
{

}

//get the local device
void CBluetoothSettingPage::getLocalDevice(IBluetoothLocalDevice *localDevice)
{
    m_localDevice = localDevice;
    if(NULL != m_localDevice) {
        ;
    } else {
        LOGE(tag, "m_localDevice is empty!\n");
    }

    getValueFromStorage();

}

///////////////////////////////////////// the slot function ////////////////////////////////////////////

//bluetoothSettingPageView get the bluetooth name
QString CBluetoothSettingPage::getBluetoothName()
{
    LOGD(tag, "getBluetoothName\n");

    if(NULL != m_localDevice) {
        string name = m_localDevice->getName();
        m_bluetoothName = QString::fromStdString(name);
    }

    return m_bluetoothName;
}

//bluetoothSettingPageView get the bluetooth password
QString CBluetoothSettingPage::getBluetoothPassword()
{
    LOGD(tag, "getBluetoothPassword\n");

    if(NULL != m_localDevice) {
        string pin_code;
        m_localDevice->getPinCode(pin_code);
        m_bluetoothPassword = QString::fromStdString(pin_code);
    }

    return m_bluetoothPassword;
}

//bluetoothSettingPageView get the auto answer state
bool CBluetoothSettingPage::getAutoAnswerState()
{
    LOGD(tag, "getAutoAnswerState\n");

    return m_autoAnswer;
}

//bluetoothSettingPageView get the auto connect state
bool CBluetoothSettingPage::getAutoConnectState()
{
    LOGD(tag, "getAutoConnectState\n");

    return m_autoConnect;
}

//bluetoothSettingPageView get the power state
int CBluetoothSettingPage::getPowerState()
{
    LOGD(tag, "getPowerState\n");

    return m_bluetoothPower;
}

void CBluetoothSettingPage::clickNameButton()
{
    LOGD(tag, "clickNameButton\n");

    if (isBluetoothPowerOn()) {
        emit showNameDialog();
    } else {
        emit notifyToPowerOn();
    }
}

void CBluetoothSettingPage::clickPasswordButton()
{
    LOGD(tag, "clickPasswordButton\n");

    if (isBluetoothPowerOn()) {
        emit showPasswdDialog();
    } else {
        emit notifyToPowerOn();
    }
}

//the local user set bluetooth name in bluetoothSettingPageView
void CBluetoothSettingPage::setBluetoothName(QString name_text)
{
    LOGD(tag, "setBluetoothName\n");

    if (NULL != m_localDevice) {
        LOGI(tag, "m_bluetoothName is %s\n", (name_text.toStdString()).c_str());
        m_localDevice->setName(name_text.toStdString());
    }
}

//the local user set bluetooth password in bluetoothSettingPageView
void CBluetoothSettingPage::setBluetoothPassword(QString passwd_text)
{
    LOGD(tag, "setBluetoothPassword\n");

    m_bluetoothPassword = passwd_text;
    emit bluetoothPasswordChanged(m_bluetoothPassword);

    if (NULL != m_localDevice) {
        LOGI(tag, "m_bluetoothPassword is %s\n", (passwd_text.toStdString()).c_str());
        m_localDevice->setPinCode(m_bluetoothPassword.toStdString());
    }
}

//the local user set bluetooth auto answer state in bluetoothSettingPageView
void CBluetoothSettingPage::setAutoAnswer()
{
    LOGD(tag, "setAutoAnswer\n");

    m_autoAnswer = !m_autoAnswer;
    emit autoAnswerStateChanged(m_autoAnswer);

    g_bluetoothMemData.memAutoAnswer = m_autoAnswer;
    int ret = saveBluetoothConfiguration();
    if (1 == ret) {
        LOGD(tag, " save AutoAnswer OK!\n");
    } else if (0 == ret) {
        LOGE(tag, " save AutoAnswer Fail!\n");
    }
}

//the local user set bluetooth auto connect state in bluetoothSettingPageView
void CBluetoothSettingPage::setAutoConnect()
{
    LOGD(tag, "setAutoConnect\n");

    m_autoConnect = !m_autoConnect;
    emit autoConnectStateChanged(m_autoConnect);

    g_bluetoothMemData.memAutoConnect = m_autoConnect;
    int ret = saveBluetoothConfiguration();
    if (1 == ret) {
        LOGD(tag, " save AutoConnect OK!\n");
    } else if (0 == ret) {
        LOGE(tag, " save AutoConnect Fail!\n");
    }
}

//the local user set bluetooth power state in bluetoothSettingPageView
void CBluetoothSettingPage::setBluetoothPower()
{
    LOGD(tag, "setBluetoothPower\n");

    if (NULL != m_localDevice) {
        E_LOCAL_DEVICE_STATE local_device_state = GAP_STATE_INVALID;
        m_localDevice->getBluetoothState(local_device_state);
        LOGD(tag, "local_device_state is 0x%x\n", local_device_state);
        if (GAP_STATE_POWEROFF == local_device_state){
            LOGD(tag, "m_localDevice->open\n");
            m_localDevice->open();
            m_bluetoothPower = POWER_OPENING_OR_CLOSING;
            startBTSwitchTimer();
            emit bluetoothPowerStateChanged(m_bluetoothPower);
        } else if ((GAP_STATE_POWERON == local_device_state) || (GAP_STATE_DISCOVERYING == local_device_state)) {
            LOGD(tag, "m_localDevice->close\n");
            m_localDevice->close();
            m_bluetoothPower = POWER_OPENING_OR_CLOSING;
            startBTSwitchTimer();
            emit bluetoothPowerStateChanged(m_bluetoothPower);
        }
    }
}

//get the bluetooth local name from gapcallback or when first open bluetooth APP
void CBluetoothSettingPage::doBluetoothLocalName(QString bluetoothName)
{
    LOGD(tag, "doBluetoothLocalName, m_bluetoothName is %s\n",
        (m_bluetoothName.toStdString()).c_str());

    m_bluetoothName = bluetoothName;
    emit bluetoothNameChanged(m_bluetoothName);
}

//get the bluetooth auto answer state when first open bluetooth APP
void CBluetoothSettingPage::doBluetoothAutoAnswer(bool autoAnswer)
{
    LOGD(tag, "doBluetoothAutoAnswer, autoAnswer is %d\n", autoAnswer);

    m_autoAnswer = autoAnswer;
    emit autoAnswerStateChanged(m_autoAnswer);
}

//get the bluetooth auto connect state when first open bluetooth APP
void CBluetoothSettingPage::doBluetoothAutoConnect(bool autoConnect)
{
    LOGD(tag, "doBluetoothAutoConnect, autoConnect is %d\n", autoConnect);

    m_autoConnect = autoConnect;
    emit autoConnectStateChanged(m_autoConnect);
}

//get the bluetooth power state from gapcallback or  when first open bluetooth APP
void CBluetoothSettingPage::doBluetoothPowerState(bool powerState)
{
    LOGD(tag, "doBluetoothPowerState, powerState is %d\n", powerState);
    if (powerState) {
        m_bluetoothPower = POWER_ON; // power on
    } else {
        m_bluetoothPower = POWER_OFF; // power off
    }
    stopBTSwitchTimer();
    emit bluetoothPowerStateChanged(m_bluetoothPower);
}

//get the bluetooth name and password from storage when first open bluetooth APP
void CBluetoothSettingPage::getValueFromStorage()
{
    if(NULL != m_localDevice) {
        string name = m_localDevice->getName();
        m_bluetoothName = QString::fromStdString(name);

        string pin_code;
        m_localDevice->getPinCode(pin_code);
        m_bluetoothPassword = QString::fromStdString(pin_code);
    }
}

bool CBluetoothSettingPage::isBluetoothPowerOn()
{
    bool isPowerOn = false;
    E_LOCAL_DEVICE_STATE local_device_state = GAP_STATE_INVALID;

    if (NULL != m_localDevice) {
        m_localDevice->getBluetoothState(local_device_state);
        LOGD(tag, "local_device_state = %d\n", local_device_state);
    }

    switch (local_device_state) {
        case GAP_STATE_POWEROFF:
        case GAP_STATE_POWERSWTICHING:
            isPowerOn = false;
            break;

        case GAP_STATE_POWERON:
        case GAP_STATE_DISCOVERYING:
            isPowerOn = true;
            break;

        default:
            break;
    }

    return isPowerOn;
}

