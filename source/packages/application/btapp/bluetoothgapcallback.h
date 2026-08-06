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

#ifndef BLUETOOTHGAPCALLBACK_H
#define BLUETOOTHGAPCALLBACK_H

#include "bluetoothapi.h"
#include "cconditionlock.h"
#include "bluetoothgapindications.h"
#include "bluetoothhandsfree.h"
#include "bluetoothmessageextras.h"
#include "../../connectivity/universal_utils/include/singleton.h"
#include "applog.h"

#include <QObject>


class CBluetoothGAPCallBack
: public QObject
, public IBluetoothCallBack
, public universal_utils::Singleton<CBluetoothGAPCallBack>
{
    Q_OBJECT

public:

    CBluetoothGAPCallBack();

    virtual ~CBluetoothGAPCallBack();

    virtual int onIndication(const universal_utils::CMessage &message);

signals:
    void sigPowerState(bool b_powerState);
    void sigBluetoothLocalName(QString bt_name);
    void sigScanModeActive(bool m_scanModeState);
    void sigScanStartResponse();
    void sigScanStopResponse();
    void sigScanResponse(QString btdev_address, QString btdev_name);
    void sigUpdateNameResponse(QString btdev_address, QString btdev_name);
    void sigSecureUserConfirmRequest(QString btdev_address);
    void sigPairResponse(QString btdev_address, QString btdev_name, bool m_pairState);
    void sigPairRemoveResponse(QString btdev_address, QString btdev_name);
    void sigGapLinkStateErrorInd(QString btdev_address);
    void sigUuidsUpdate(QString btdev_address, QString btdev_name);

public slots:

private:

};
#endif
