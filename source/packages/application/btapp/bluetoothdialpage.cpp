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
 
#include "bluetoothdialpage.h"

using namespace std;
static const char* const tag = "CBluetoothDialPage";

CBluetoothDialPage::CBluetoothDialPage()
    : m_dialNumber("")
    , m_redialNumber("")
    , m_phoneConnectState(false)
    , m_hfpInterface(NULL)
{

}

CBluetoothDialPage::~CBluetoothDialPage()
{
    LOGD(tag, "[CBluetoothDialPage] destructor\n");

}

void CBluetoothDialPage::initBluetoothDialPage()
{

}

//get hfp interface
void CBluetoothDialPage::getHFPInterface(IBluetoothHandsfree  *hfpInterface)
{
    m_hfpInterface = hfpInterface;
    if(NULL != m_hfpInterface ) {
        ;
    } else {
        LOGE(tag, "m_hfpInterface is empty!\n");
    }
}

////////////////////////////////////////the slot function ///////////////////////////////////////////////////

//the bluetoothDial UI get the dial number
QString CBluetoothDialPage::getDialNumber()
{
    LOGD(tag, "getDialNumber\n");

    return m_dialNumber;
}

//the bluetoothDial UI get the redial number
QString CBluetoothDialPage::getRedialNumber()
{
    LOGD(tag, "getRedialNumber\n");

    return m_redialNumber;
}

//the bluetoothDial UI set the dial number
void CBluetoothDialPage::setDialNumber(QString number)
{
    LOGD(tag, "setDialNumber\n");

    m_dialNumber = number;
    string dial_num = m_dialNumber.toStdString();
    LOGI(tag, "dial_number is %s\n", dial_num.c_str());

    return;
}

//the local user send the phone call request
void CBluetoothDialPage::phoneCallRequest(QString dial_number)
{
    LOGD(tag, "phoneCallRequest\n");

    m_dialNumber = dial_number;
    if ("" != m_dialNumber) {
        m_redialNumber = dial_number;
    }
    if ("" != m_dialNumber && NULL != m_hfpInterface) {
        emit sigDialNumber(dial_number);

        string dial_num = m_dialNumber.toStdString();
        LOGI(tag, "dial_number is %s\n", dial_num.c_str());
        m_hfpInterface->dialNumber(dial_num);
    }
    if (false == m_phoneConnectState) {
        emit phoneAudioDisconnect();
    } else if ("" == m_dialNumber) {
        emit dialNumberEmpty();
    }
}

//the local user send the phone recall request
void CBluetoothDialPage::phoneRecallRequest(QString redial_number)
{
    LOGD(tag, "phoneRecallRequest\n");

    m_redialNumber = redial_number;
    if (NULL != m_hfpInterface) {
        m_hfpInterface->redial();
    }

    if (false == m_phoneConnectState) {
        emit phoneAudioDisconnect();
    }
}

//get the hfp connect state from bluetoothPairedRecordPage
void CBluetoothDialPage::doPhoneConnectState(bool phoneConnectState)
{
    LOGD(tag, "doPhoneConnectState\n");

    m_phoneConnectState = phoneConnectState;
}

void CBluetoothDialPage::doDialKeyEvent()
{
    LOGD(tag, "doDialKeyEvent\n");

    phoneCallRequest(m_dialNumber);
}

