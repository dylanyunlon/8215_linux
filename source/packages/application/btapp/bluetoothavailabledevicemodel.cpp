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

#include "bluetoothavailabledevicemodel.h"
#include "bluetoothutils.h"

using namespace universal_utils;

static const char* const tag = "CBluetoothAvailableDeviceModel";
template<> CBluetoothAvailableDeviceModel* Singleton<CBluetoothAvailableDeviceModel>::msSingleton = NULL;

CBluetoothAvailableDevice::CBluetoothAvailableDevice(const QString availableDeviceName,
    const QString availableDeviceAddress, const QString availableDeviceState,
    const bool availableDevicePhoneAudioState, const bool availableDeviceMediaAudioState)
    : m_availableDeviceName(availableDeviceName)
    , m_availableDeviceAddress(availableDeviceAddress)
    , m_availableDeviceState(availableDeviceState)
    , m_availableDevicePhoneAudioState(availableDevicePhoneAudioState)
    , m_availableDeviceMediaAudioState(availableDeviceMediaAudioState)
{

}

QString CBluetoothAvailableDevice::availableDeviceName() const
{
    return m_availableDeviceName;
}

QString CBluetoothAvailableDevice::availableDeviceAddress() const
{
    return m_availableDeviceAddress;
}

QString CBluetoothAvailableDevice::availableDeviceState() const
{
    return m_availableDeviceState;
}

bool CBluetoothAvailableDevice::availableDevicePhoneAudioState() const
{
    return m_availableDevicePhoneAudioState;
}

bool CBluetoothAvailableDevice::availableDeviceMediaAudioState() const
{
    return m_availableDeviceMediaAudioState;
}

////////////////////////////////////////////////////////////////////////////////////////

CBluetoothAvailableDeviceModel::CBluetoothAvailableDeviceModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

CBluetoothAvailableDeviceModel::~CBluetoothAvailableDeviceModel()
{
    LOGD(tag, "destructor\n");

    if (0 != m_availableDevice.size()) {
        clearAvailableDevice();
    }
}

void CBluetoothAvailableDeviceModel::addAvailableDevice(const CBluetoothAvailableDevice &availableDevice)
{
    LOGD(tag, "addAvailableDevice\n");

   beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_availableDevice << availableDevice;
    endInsertRows();
}

void CBluetoothAvailableDeviceModel::clearAvailableDevice()
{
    LOGD(tag, "clearAvailableDevice\n");

    beginRemoveRows(QModelIndex(), 0, rowCount());
    m_availableDevice.clear();
    endRemoveRows();
}

void CBluetoothAvailableDeviceModel::searchAvailableDeviceAddress(int index, QString &availableDeviceAddress)
{
    LOGD(tag, "searchAvailableDeviceAddress, index = %d\n", index);

    if (index >= 0 && index < m_availableDevice.size()) {
        const CBluetoothAvailableDevice &availableDevice = m_availableDevice.at(index);
        availableDeviceAddress = availableDevice.availableDeviceAddress();
    } else {
        LOGD(tag, "searchAvailableDeviceAddress, invalid index is %d, list size is %d\n",
                    index, m_availableDevice.size());
    }
}

void CBluetoothAvailableDeviceModel::searchAvailableDeviceSelectConnectState(QString availableDeviceAddress,
        bool &availableDevicePhoneAudioState, bool &availableDeviceMediaAudioState)
{
    LOGD(tag, "searchAvailableDeviceSelectConnectState, availableDeviceAddress = %s\n",
        BluetoothUtils::StringForLog(availableDeviceAddress.toStdString()).c_str());

    QList<CBluetoothAvailableDevice>::iterator availableDevice_iter = m_availableDevice.begin();

    for (; availableDevice_iter != m_availableDevice.end(); availableDevice_iter++) {
        const CBluetoothAvailableDevice &availableDevice = *availableDevice_iter;
        if (availableDeviceAddress == availableDevice.availableDeviceAddress()) {
            availableDevicePhoneAudioState = availableDevice.availableDevicePhoneAudioState();
            availableDeviceMediaAudioState = availableDevice.availableDeviceMediaAudioState();
        }
    }
}

void CBluetoothAvailableDeviceModel::searchAvailableDeviceSelectConnectState(int index,
    bool &availableDevicePhoneAudioState, bool &availableDeviceMediaAudioState)
{
    LOGD(tag, "searchAvailableDeviceSelectConnectState, index = %d\n", index);

    if (index >= 0 && index < m_availableDevice.size()) {
        const CBluetoothAvailableDevice &availableDevice = m_availableDevice.at(index);
        availableDevicePhoneAudioState = availableDevice.availableDevicePhoneAudioState();
        availableDeviceMediaAudioState = availableDevice.availableDeviceMediaAudioState();
    } else {
        LOGD(tag, "searchAvailableDeviceSelectConnectState, invalid index is %d, list size is %d\n",
                    index, m_availableDevice.size());
    }
}

bool CBluetoothAvailableDeviceModel::isInAvailableDeviceModel(QString availableDeviceAddress)
{
    LOGD(tag, "isInAvailableDeviceModel, availableDeviceAddress = %s\n",
        BluetoothUtils::StringForLog(availableDeviceAddress.toStdString()).c_str());

    bool b_isInAvailableDeviceModel = false;
    QList<CBluetoothAvailableDevice>::iterator availableDevice_iter = m_availableDevice.begin();

    for (; availableDevice_iter != m_availableDevice.end(); availableDevice_iter++) {
        const CBluetoothAvailableDevice &availableDevice = *availableDevice_iter;
        if (availableDeviceAddress == availableDevice.availableDeviceAddress()) {
            b_isInAvailableDeviceModel = true;
        }
    }

    return b_isInAvailableDeviceModel;
}

void CBluetoothAvailableDeviceModel::updateAvailableDeviceName(
    QString availableDeviceName, QString availableDeviceAddress)
{
    LOGD(tag, "updateAvailableDeviceName\n");

    int index = 0;
    QList<CBluetoothAvailableDevice>::iterator availableDevice_iter = m_availableDevice.begin();

    for (; availableDevice_iter != m_availableDevice.end(); availableDevice_iter++) {
        const CBluetoothAvailableDevice &availableDevice = *availableDevice_iter;
        if (availableDeviceAddress == availableDevice.availableDeviceAddress()) {
            QString availableDeviceState = availableDevice.availableDeviceState();

            const CBluetoothAvailableDevice availableDevice1(availableDeviceName,
                availableDeviceAddress, availableDeviceState, true, true);
            m_availableDevice.replace(index, availableDevice1);
            emit dataChanged(this->createIndex(index, 1), this->createIndex(index, 1));
        }
        index++;
    }
}

void CBluetoothAvailableDeviceModel::updateAvailableDeviceSelectConnectState(
        int index, bool phoneAudioState, bool mediaAudioState)
{
    LOGD(tag, "updateAvailableDeviceSelectConnectState\n");

    if (index >= 0 && index < m_availableDevice.size()) {
        const CBluetoothAvailableDevice &availableDevice = m_availableDevice.at(index);
        QString availableDeviceName = availableDevice.availableDeviceName();
        QString availableDeviceAddress = availableDevice.availableDeviceAddress();
        QString availableDeviceState = availableDevice.availableDeviceState();

        const CBluetoothAvailableDevice availableDevice1(availableDeviceName,
                        availableDeviceAddress, availableDeviceState, phoneAudioState, mediaAudioState);

        m_availableDevice.replace(index, availableDevice1);
        emit dataChanged(this->createIndex(index, 1), this->createIndex(index, 1));
    } else {
        LOGD(tag, "updateAvailableDeviceSelectConnectState, invalid index is %d, list size is %d\n",
                    index, m_availableDevice.size());
    }
}

void CBluetoothAvailableDeviceModel::deleteAvailableDevice(QString availableDeviceAddress)
{
    LOGD(tag, "isInAvailableDeviceModel, deleteAvailableDevice = %s\n",
            BluetoothUtils::StringForLog(availableDeviceAddress.toStdString()).c_str());

    int index = 0;
    QList<CBluetoothAvailableDevice>::iterator availableDevice_iter = m_availableDevice.begin();

    for (; availableDevice_iter != m_availableDevice.end(); ) {
        const CBluetoothAvailableDevice &availableDevice = *availableDevice_iter;
        if (availableDeviceAddress == availableDevice.availableDeviceAddress()) {
            beginRemoveRows(QModelIndex(), index, index);
            availableDevice_iter = m_availableDevice.erase(availableDevice_iter);
            endRemoveRows();
        } else {
            availableDevice_iter++;
        }
        index++;
    }
}

int CBluetoothAvailableDeviceModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return m_availableDevice.count();
}

QVariant CBluetoothAvailableDeviceModel::data(const QModelIndex & index, int role) const
{
    if (index.row() < 0 || index.row() >= m_availableDevice.count()) {
        return QVariant();
    }

    const CBluetoothAvailableDevice &availableDevice = m_availableDevice[index.row()];

    if (AVAILABLE_DEVICE_NAME_ROLE == role) {
        return availableDevice.availableDeviceName();
    } else if (AVAILABLE_DEVICE_ADDRESS_ROLE == role) {
        return availableDevice.availableDeviceAddress();
    } else if (AVAILABLE_DEVICE_STATE_ROLE == role) {
        return availableDevice.availableDeviceState();
    } else if (AVAILABLE_DEVICE_PHONE_AUDIO_STATE_ROLE == role) {
        return availableDevice.availableDevicePhoneAudioState();
    } else if (AVAILABLE_DEVICE_MEDIA_AUDIO_STATE_ROLE == role) {
        return availableDevice.availableDeviceMediaAudioState();
    }

    return QVariant();
}

QHash<int, QByteArray> CBluetoothAvailableDeviceModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[AVAILABLE_DEVICE_NAME_ROLE]  = STRING_AVAILABLE_DEVICE_NAME;
    roles[AVAILABLE_DEVICE_ADDRESS_ROLE]   = STRING_AVAILABLE_DEVICE_ADDRESS;
    roles[AVAILABLE_DEVICE_STATE_ROLE] = STRING_AVAILABLE_DEVICE_STATE;
    roles[AVAILABLE_DEVICE_PHONE_AUDIO_STATE_ROLE] = STRING_AVAILABLE_PHONE_AUDIO_DEVICE_STATE;
    roles[AVAILABLE_DEVICE_MEDIA_AUDIO_STATE_ROLE] = STRING_AVAILABLE_MEDIA_AUDIO_DEVICE_STATE;

    return roles;
}

