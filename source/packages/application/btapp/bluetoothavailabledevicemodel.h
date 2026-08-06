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

#ifndef BLUETOOTHAVAILABLEDEVICEMODEL_H
#define BLUETOOTHAVAILABLEDEVICEMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include "../../connectivity/universal_utils/include/singleton.h"
#include "applog.h"

#define STRING_AVAILABLE_DEVICE_NAME               "bluetoothAvailableDeviceName"
#define STRING_AVAILABLE_DEVICE_ADDRESS            "bluetoothAvailableDeviceAddress"
#define STRING_AVAILABLE_DEVICE_STATE              "bluetoothAvailableDeviceState"
#define STRING_AVAILABLE_PHONE_AUDIO_DEVICE_STATE  "bluetoothAvailableDevicePhoneAudioState"
#define STRING_AVAILABLE_MEDIA_AUDIO_DEVICE_STATE  "bluetoothAvailableDeviceMediaAudioState"

class CBluetoothAvailableDevice
{
public:
    CBluetoothAvailableDevice(const QString availableDeviceName,
        const QString availableDeviceAddress, const QString availableDeviceState,
        const bool availableDevicePhoneAudioState, const bool availableDeviceMediaAudioState);

    QString availableDeviceName() const;
    QString availableDeviceAddress() const;
    QString availableDeviceState() const;
    bool    availableDevicePhoneAudioState() const;
    bool    availableDeviceMediaAudioState() const;

private:
    QString m_availableDeviceName;
    QString m_availableDeviceAddress;
    QString m_availableDeviceState;
    bool    m_availableDevicePhoneAudioState;
    bool    m_availableDeviceMediaAudioState;
};

class CBluetoothAvailableDeviceModel
    : public QAbstractListModel
    , public universal_utils::Singleton<CBluetoothAvailableDeviceModel>
{
    Q_OBJECT

public:

    CBluetoothAvailableDeviceModel(QObject *parent = 0);
    ~CBluetoothAvailableDeviceModel();
    bool isInAvailableDeviceModel(QString availableDeviceAddress);
    void addAvailableDevice(const CBluetoothAvailableDevice &availableDevice);
    void clearAvailableDevice();
    void searchAvailableDeviceAddress(int index, QString &availableDeviceAddress);
    void searchAvailableDeviceSelectConnectState(QString availableDeviceAddress,
        bool &availableDevicePhoneAudioState, bool &availableDeviceMediaAudioState);
    void searchAvailableDeviceSelectConnectState(int index,
        bool &availableDevicePhoneAudioState, bool &availableDeviceMediaAudioState);
    void updateAvailableDeviceName(QString availableDeviceName, QString availableDeviceAddress);
    void updateAvailableDeviceSelectConnectState(int index, bool phoneAudioState, bool mediaAudioState);
    void deleteAvailableDevice(QString availableDeviceAddress);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    typedef enum {
        AVAILABLE_DEVICE_NAME_ROLE = Qt::UserRole + 1,
        AVAILABLE_DEVICE_ADDRESS_ROLE,
        AVAILABLE_DEVICE_STATE_ROLE,
        AVAILABLE_DEVICE_PHONE_AUDIO_STATE_ROLE,
        AVAILABLE_DEVICE_MEDIA_AUDIO_STATE_ROLE,
    }E_AVAILABLE_DEVICE_ROLDES;

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QList<CBluetoothAvailableDevice> m_availableDevice;
};

#endif

