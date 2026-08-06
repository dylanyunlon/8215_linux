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

#ifndef BLUETOOTHHFPCALLBACK_H
#define BLUETOOTHHFPCALLBACK_H

#include "bluetoothapi.h"
#include "bluetoothhandsfreeindications.h"
#include "bluetoothhandsfree.h"
#include "bluetoothmessageextras.h"
#include "../../connectivity/universal_utils/include/singleton.h"
#include "applog.h"

#include <QObject>


class CBluetoothHFPCallBack
    : public QObject, public IBluetoothCallBack
    , public universal_utils::Singleton<CBluetoothHFPCallBack>
{
    Q_OBJECT

public:

    CBluetoothHFPCallBack();

    virtual ~CBluetoothHFPCallBack();

    virtual int onIndication(const universal_utils::CMessage &message);

signals:
    void sigHFConnectingResponse(QString btdev_address);
    void sigHFConnectedResponse(QString btdev_address, bool hfpConnectedResult);
    void sigHFDisconnectedResponse(QString btdev_address);
    void sigCallStateChange(int callState);
    void sigCallNumber(QString callNuber);
    void sigCallListChange();
    void sigScoState(bool audioSourceInHF);
    void sigPhoneFactroy(QString phoneFactroy);
    void sigPhoneSerial(QString phoneSerial);
    void sigATNOAnswer();
    void sigSupportInBandRing(int supportInBandRing);
    void sigUpdateSpeakerGain(int agSpeakerGain);


public slots:


private:

};
#endif

