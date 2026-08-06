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
 
#ifndef BLUETOOTHCALLPAGE_H
#define BLUETOOTHCALLPAGE_H

#include <QObject>
#include <iostream>
#include "bluetoothhfpcallback.h"
#include "bluetoothcallrecordsmodel.h"
#include "applog.h"
#include "bluetoothcallmodel.h"

#define STRING_CALLNAME CBluetoothCallPage::trUtf8("Unknown")

class CBluetoothCallPage : public QObject
{
    Q_OBJECT
public:
    CBluetoothCallPage();
    ~CBluetoothCallPage();
    void initBluetoothCallPage();
    void getHFPInterface(IBluetoothHandsfree  *hfpInterface);
    void getPBAPInterface(IBluetoothPBAP *pbapInterface);
    QString getPhoneName(const string &callNumber);

signals:
    void bluetoothCallPageChanged(bool m_callPageState);
    void updateCallNumber(QString m_callNuber);
    void updatePhoneName(QString m_phoneName);
    void scoStateChanged(bool m_audioSourceInHF);
    void callListChanged();
    void triggerAutoAnswerTimer();

    void sigCallRecordsState(bool incomingState, bool outgoingState, bool missingState);

public slots:
    bool getAudioSourceInHFState();
    void switchAudioSource(bool audioSourceInHF);
    void acceptPhoneCall();
    void terminatePhoneCall();
    void holdPhoneCall(int action);
    void inputDTMFCode(QString dtmfCode);
    void autoAnswerTimerStop();

    void doCallStateChange(int callState);
    void doCallListChange();
    int convertCallState(int state);
    void doPhoneFactroy(QString phoneFactroy);
    void doPhoneSerial(QString phoneSerial);
    void doScoState(bool audioSourceInHF);
    void doHFDisconnectedResponse(QString btdev_address);
    void doAutoListenStateChanged(bool autoListenState);
    void doGetPhoneBookState();
    void doDialNumber(QString dialNumber);

private:
    bool getRecordByNumber(std::string callNumber,  QString &phoneName);
    void addCallRecordList(const CBluetoothCall &call);
    void updateCallRecordsList();
    void onIncomingCallState();
    void onOutgoingCallState();
    void onSpeakingCallState();
    void onIdleCallState();
    void enableCallAudio(bool enable);
    void enableCallPageState(bool enable);

private:
    bool m_callPageState;
    bool m_audioSourceInHF;
    bool m_autoListenState;
    bool m_phoneBookState;
    bool m_incomingCall;
    bool m_outgoingCall;
    bool m_missingCall;
    bool m_audioState;
    QString m_phoneFactroy;
    QString m_phoneSerial;
    QString m_dialNumber;
    std::list<CBluetoothCall> m_callRecordList;
    std::list<std::string> m_internationalAreaCodeList;
    std::list<std::string> m_localAreaCodeList;
    QMap<QString, QString> m_specialPhoneMap;

    IBluetoothHandsfree   *m_hfpInterface;
    IBluetoothPBAP        *m_pbapInterface;
    CBluetoothHFPCallBack *m_bluetoothHFPCallBack;
    CBluetoothCallRecordsModel *m_bluetoothCallRecordsModel;
};

#endif

