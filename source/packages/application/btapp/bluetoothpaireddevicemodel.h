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

#ifndef BLUETOOTHPAIREDDEVICEMODEL_H
#define BLUETOOTHPAIREDDEVICEMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include <QtDebug>
#include "../../connectivity/universal_utils/include/singleton.h"
#include "applog.h"

#define STRING_PAIRED_DEVICE_NAME               "bluetoothPairedDeviceName"
#define STRING_PAIRED_DEVICE_ADDRESS            "bluetoothPairedDeviceAddress"
#define STRING_PAIRED_DEVICE_STATE              "bluetoothPairedDeviceState"
#define STRING_PAIRED_PHONE_AUDIO_DEVICE_STATE  "bluetoothPairedDevicePhoneAudioState"
#define STRING_PAIRED_MEDIA_AUDIO_DEVICE_STATE  "bluetoothPairedDeviceMediaAudioState"

class CBluetoothPairedDevice
{
public:
    CBluetoothPairedDevice(const QString pairedDeviceName,
        const QString pairedDeviceAddress, const QString pairedDeviceState,
        const bool pairedDevicePhoneAudioState, const bool pairedDeviceMediaAudioState,
        const bool pairedDeviceA2DPState, const bool pairedDeviceAVRCPState);

    QString pairedDeviceName() const;
    QString pairedDeviceAddress() const;
    QString pairedDeviceState() const;
    bool    pairedDevicePhoneAudioState() const;
    bool    pairedDeviceMediaAudioState() const;
    bool    pairedDeviceA2DPState() const;
    bool    pairedDeviceAVRCPState() const;

private:
    QString m_pairedDeviceName;
    QString m_pairedDeviceAddress;
    QString m_pairedDeviceState;
    bool    m_pairedDevicePhoneAudioState;
    bool    m_pairedDeviceMediaAudioState;
    bool    m_pairedDeviceA2DPState;
    bool    m_pairedDeviceAVRCPState;
};

class CBluetoothPairedDeviceModel
    : public QAbstractListModel
    , public universal_utils::Singleton<CBluetoothPairedDeviceModel>
{
    Q_OBJECT

public:

    CBluetoothPairedDeviceModel(QObject *parent = 0);
    ~CBluetoothPairedDeviceModel();
    void addPairedDevice(const CBluetoothPairedDevice &pairedDevice);
    void clearPairedDevice();
    void searchPairedDeviceAddress(int index, QString &pairedDeviceAddress);
    void searchPairedDeviceSelectConnectState(int index,
        bool &pairedDevicePhoneAudioState, bool &pairedDeviceMediaAudioState);
    void searchPairedDeviceSelectConnectState(QString pairedDeviceAddress,
        bool &pairedDevicePhoneAudioState, bool &pairedDeviceMediaAudioState);
    bool isInPairedDeviceModel(QString pairedDeviceAddress);
    void updatePairedDeviceStateString(QString pairedDeviceAddress, QString pairedDeviceState);
    void updatePairedDeviceSelectConnectState(int index, bool phoneAudioState, bool mediaAudioState);
    void updatePairedDeviceHFPState(QString pairedDeviceAddress, bool HFPState);
    void updatePairedDeviceA2DPState(QString pairedDeviceAddress, bool A2DPState);
    void updatePairedDeviceAVRCPState(QString pairedDeviceAddress, bool AVRCPState);
    void updatePairedDeviceMediaAudioState(QString pairedDeviceAddress, bool mediaAudioState);
    void getPairedDeviceAddress(QString pairedDeviceState, QString &pairedDeviceAddress);
    void getPairedDeviceState(QString pairedDeviceAddress, QString &pairedDeviceState);
    void getPairedDeviceState(int index, QString &pairedDeviceState);
    void getPairedDeviceMediaAudioState(QString pairedDeviceAddress, bool &mediaAudioState);
    bool checkPairedDeviceState(int index, QString pairedDeviceState);
    void deletePairedDevice(QString pairedDeviceAddress);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    typedef enum {
        PAIRED_DEVICE_NAME_ROLE = Qt::UserRole + 1,
        PAIRED_DEVICE_ADDRESS_ROLE,
        PAIRED_DEVICE_STATE_ROLE,
        PAIRED_DEVICE_PHONE_AUDIO_STATE_ROLE,
        PAIRED_DEVICE_MEDIA_AUDIO_STATE_ROLE,
    }E_PAIRED_DEVICE_ROLES;

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QList<CBluetoothPairedDevice> m_pairedDevice;
};

#endif
