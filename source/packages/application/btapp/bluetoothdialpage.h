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
 
#ifndef BLUETOOTHDIALPAGE_H
#define BLUETOOTHDIALPAGE_H

#include <QObject>
#include <iostream>
#include <string>
#include "bluetoothhandsfree.h"
#include "applog.h"

class CBluetoothDialPage: public QObject
{
    Q_OBJECT
public:
    CBluetoothDialPage();
    ~CBluetoothDialPage();
    void initBluetoothDialPage();
    void getHFPInterface(IBluetoothHandsfree  *hfpInterface);

signals:
    void phoneAudioDisconnect();
    void dialNumberEmpty();
    void redialNumberEmpty();

    void sigDialNumber(QString dial_number);

public slots:
    QString getDialNumber();
    QString getRedialNumber();
    void setDialNumber(QString number);
    void phoneCallRequest(QString dial_number);
    void phoneRecallRequest(QString redial_number);

    void doPhoneConnectState(bool phoneConnectState);
    void doDialKeyEvent();
private:
    QString m_dialNumber;
    QString m_redialNumber;
    bool m_phoneConnectState;
    IBluetoothHandsfree   *m_hfpInterface;
};

#endif

