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

#ifndef BLUETOOTHAVRCPCALLBACK_H
#define BLUETOOTHAVRCPCALLBACK_H

#include "bluetoothapi.h"
#include "bluetoothavrcpindications.h"
#include "bluetoothmessageextras.h"
#include "../../connectivity/universal_utils/include/singleton.h"
#include "applog.h"

#include <QObject>
#include <string>

class CBluetoothAVRCPCallBack
    : public QObject
    , public IBluetoothCallBack
    , public universal_utils::Singleton<CBluetoothAVRCPCallBack>
{
    Q_OBJECT

public:

    CBluetoothAVRCPCallBack ();

    virtual ~CBluetoothAVRCPCallBack();

    virtual int onIndication(const universal_utils::CMessage &message);

signals:
    void sigAVRCPConnectedResponse(QString btdev_address, bool avrcpConnectedResult);
    void sigAVRCPDisconnectedResponse(QString btdev_address);
    void sigPlaybackDateUpdate(int musicState, int playingTime, int totalTime);
    void sigMusicPositionChanged(int playingTime, int totalTime);
    void sigPlayStateChanged(int musicState);
    void sigMediaDateChangedResponse();

private:


};
#endif

