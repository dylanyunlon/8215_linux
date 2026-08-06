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

#ifndef BLUETOOTHA2DPCALLBACK_H
#define BLUETOOTHA2DPCALLBACK_H

#include "bluetoothapi.h"
#include "bluetootha2dpindications.h"
#include "bluetoothmessageextras.h"
#include "bluetootha2dp.h"
#include "../../connectivity/universal_utils/include/singleton.h"
#include "applog.h"

#include <QObject>
#include <string>

class CBluetoothA2DPCallBack
    : public QObject, public IBluetoothCallBack
    , public universal_utils::Singleton<CBluetoothA2DPCallBack>
{
    Q_OBJECT

public:

    CBluetoothA2DPCallBack ();

    virtual ~CBluetoothA2DPCallBack();

    virtual int onIndication(const universal_utils::CMessage &message);

signals:
    void sigA2DPConnectedResponse(QString btdev_address, bool a2dpConnectedResult);
    void sigA2DPDisconnectedResponse(QString btdev_address);
    void sigAudioRequest();
    void sigAudioRelease();

private:


};
#endif

