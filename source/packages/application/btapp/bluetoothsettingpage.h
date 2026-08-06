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
 
#ifndef BLUETOOTHSETTINGPAGE_H
#define BLUETOOTHSETTINGPAGE_H

#include <QObject>
#include <QTimer>
#include <string>
#include "bluetoothapi.h"
#include "applog.h"

class CBluetoothSettingPage : public QObject
{
    Q_OBJECT

public:
    CBluetoothSettingPage();
    ~CBluetoothSettingPage();
    void initBluetoothSettingPage();
    void getLocalDevice(IBluetoothLocalDevice *localDevice);
    void startBTSwitchTimer();
    void stopBTSwitchTimer();

signals:
    void autoAnswerStateChanged(bool m_autoAnswer);
    void autoConnectStateChanged(bool m_autoConnect);
    void bluetoothPowerStateChanged(int m_bluetoothPower);
    void bluetoothNameChanged(QString m_bluetoothName);
    void bluetoothPasswordChanged(QString m_bluetoothPassword);
    void notifyToPowerOn();
    void showNameDialog();
    void showPasswdDialog();

public slots:
    QString getBluetoothName();
    QString getBluetoothPassword();
    bool getAutoAnswerState();
    bool getAutoConnectState();
    int getPowerState();

    void clickNameButton();
    void clickPasswordButton();
    void setBluetoothName(QString name);
    void setBluetoothPassword(QString passwd);
    void setAutoAnswer();
    void setAutoConnect();
    void setBluetoothPower();

    void doBluetoothLocalName(QString bluetoothName);
    void doBluetoothAutoAnswer(bool autoAnswer);
    void doBluetoothAutoConnect(bool autoConnect);
    void doBluetoothPowerState(bool powerState);
    void doBluetoothSwitchTimeout();

private:
    void getValueFromStorage();
    bool isBluetoothPowerOn();

    bool m_autoAnswer;
    bool m_autoConnect;
    int m_bluetoothPower;
    QString m_bluetoothName;
    QString m_bluetoothPassword;
    IBluetoothLocalDevice *m_localDevice;
    QTimer *m_btSwitchTimer;


};

#endif

