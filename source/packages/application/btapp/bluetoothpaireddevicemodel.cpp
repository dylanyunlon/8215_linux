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

#include "bluetoothpaireddevicemodel.h"

using namespace universal_utils;
static const char* const tag = "CBluetoothPairedDeviceModel";
template<> CBluetoothPairedDeviceModel* Singleton<CBluetoothPairedDeviceModel>::msSingleton = NULL;

CBluetoothPairedDevice::CBluetoothPairedDevice(const QString pairedDeviceName,
    const QString pairedDeviceAddress, const QString pairedDeviceState,
    const bool pairedDevicePhoneAudioState, const bool pairedDeviceMediaAudioState,
    const bool pairedDeviceA2DPState, const bool pairedDeviceAVRCPState)
    : m_pairedDeviceName(pairedDeviceName)
    , m_pairedDeviceAddress(pairedDeviceAddress)
    , m_pairedDeviceState(pairedDeviceState)
    , m_pairedDevicePhoneAudioState(pairedDevicePhoneAudioState)
    , m_pairedDeviceMediaAudioState(pairedDeviceMediaAudioState)
    , m_pairedDeviceA2DPState(pairedDeviceA2DPState)
    , m_pairedDeviceAVRCPState(pairedDeviceAVRCPState)
{

}

QString CBluetoothPairedDevice::pairedDeviceName() const
{
    return m_pairedDeviceName;
}

QString CBluetoothPairedDevice::pairedDeviceAddress() const
{
    return m_pairedDeviceAddress;
}

QString CBluetoothPairedDevice::pairedDeviceState() const
{
    return m_pairedDeviceState;
}

bool CBluetoothPairedDevice::pairedDevicePhoneAudioState() const
{
    return m_pairedDevicePhoneAudioState;
}

bool CBluetoothPairedDevice::pairedDeviceMediaAudioState() const
{
    return m_pairedDeviceMediaAudioState;
}

bool CBluetoothPairedDevice::pairedDeviceA2DPState() const
{
    return m_pairedDeviceA2DPState;
}

bool CBluetoothPairedDevice::pairedDeviceAVRCPState() const
{
    return m_pairedDeviceAVRCPState;
}

/////////////////////////////////////////////////////////////////////////////////

CBluetoothPairedDeviceModel::CBluetoothPairedDeviceModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

CBluetoothPairedDeviceModel::~CBluetoothPairedDeviceModel()
{
    LOGD(tag, "destructor\n");

    if (0 != m_pairedDevice.size()) {
        clearPairedDevice();
    }
}

void CBluetoothPairedDeviceModel::addPairedDevice(const CBluetoothPairedDevice &pairedDevice)
{
    LOGD(tag, "addPairedDevice\n");

   beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_pairedDevice << pairedDevice;
    endInsertRows();

}

void CBluetoothPairedDeviceModel::clearPairedDevice()
{
    beginRemoveRows(QModelIndex(), 0, rowCount()-1);
    m_pairedDevice.clear();
    endRemoveRows();
}

void CBluetoothPairedDeviceModel::searchPairedDeviceAddress(int index, QString &pairedDeviceAddress)
{
    LOGD(tag, "searchPairedDeviceAddress, index is %d\n", index);

    if (index >= 0 && index < m_pairedDevice.size()) {
        const CBluetoothPairedDevice &pairedDevice = m_pairedDevice.at(index);
        pairedDeviceAddress = pairedDevice.pairedDeviceAddress();
    } else {
        LOGD(tag, "searchPairedDeviceAddress, invalid index is %d, list size is %d\n", index, m_pairedDevice.size());
    }
}

void CBluetoothPairedDeviceModel::searchPairedDeviceSelectConnectState(int index,
    bool &pairedDevicePhoneAudioState, bool &pairedDeviceMediaAudioState)
{
    LOGD(tag, "searchPairedDeviceSelectConnectState, index is %d\n", index);

    if (index >= 0 && index < m_pairedDevice.size()) {
        const CBluetoothPairedDevice &pairedDevice = m_pairedDevice.at(index);
        pairedDevicePhoneAudioState = pairedDevice.pairedDevicePhoneAudioState();
        pairedDeviceMediaAudioState = pairedDevice.pairedDeviceMediaAudioState();
    } else {
        LOGD(tag, "searchPairedDeviceSelectConnectState, invalid index is %d, list size is %d\n",
                    index, m_pairedDevice.size());
    }
}

void CBluetoothPairedDeviceModel::searchPairedDeviceSelectConnectState(QString pairedDeviceAddress,
    bool &pairedDevicePhoneAudioState, bool &pairedDeviceMediaAudioState)
{
    LOGD(tag, "searchPairedDeviceSelectConnectState, pairedDeviceAddress is %s\n",
        (pairedDeviceAddress.toStdString()).c_str());

    QList<CBluetoothPairedDevice>::iterator pairedDevice_iter = m_pairedDevice.begin();

    for (; pairedDevice_iter != m_pairedDevice.end(); pairedDevice_iter++) {
        const CBluetoothPairedDevice &pairedDevice = *pairedDevice_iter;
        if (pairedDeviceAddress == pairedDevice.pairedDeviceAddress()) {
            pairedDevicePhoneAudioState = pairedDevice.pairedDevicePhoneAudioState();
            pairedDeviceMediaAudioState = pairedDevice.pairedDeviceMediaAudioState();
            break;
        }
    }
}

bool CBluetoothPairedDeviceModel::isInPairedDeviceModel(QString pairedDeviceAddress)
{
    LOGD(tag, "isInPairedDeviceModel, pairedDeviceAddress is %s\n",
        (pairedDeviceAddress.toStdString()).c_str());

    bool b_isInPairedDeviceModel = false;
    QList<CBluetoothPairedDevice>::iterator pairedDevice_iter = m_pairedDevice.begin();

    for (; pairedDevice_iter != m_pairedDevice.end(); pairedDevice_iter++) {
        const CBluetoothPairedDevice &pairedDevice = *pairedDevice_iter;
        if (pairedDeviceAddress == pairedDevice.pairedDeviceAddress()) {
            b_isInPairedDeviceModel = true;
        }
    }

    return b_isInPairedDeviceModel;
}

void CBluetoothPairedDeviceModel::updatePairedDeviceStateString(
    QString pairedDeviceAddress, QString pairedDeviceState)
{
    LOGD(tag, "updatePairedDeviceStateString, pairedDeviceAddress is %s\n",
        (pairedDeviceAddress.toStdString()).c_str());

    int index = 0;
    QList<CBluetoothPairedDevice>::iterator pairedDevice_iter = m_pairedDevice.begin();

    for (; pairedDevice_iter != m_pairedDevice.end(); pairedDevice_iter++) {
        const CBluetoothPairedDevice &pairedDevice = *pairedDevice_iter;
        if (pairedDeviceAddress == pairedDevice.pairedDeviceAddress()) {
            QString pairedDeviceName = pairedDevice.pairedDeviceName();
            bool pairDevicePhoneAudioState = pairedDevice.pairedDevicePhoneAudioState();
            bool pairedDeviceMediaAudioState = pairedDevice.pairedDeviceMediaAudioState();
            bool pairedDeviceA2DPState = pairedDevice.pairedDeviceA2DPState();
            bool pairedDeviceAVRCPState = pairedDevice.pairedDeviceAVRCPState();

            const CBluetoothPairedDevice pairedDevice1(pairedDeviceName, pairedDeviceAddress,
                pairedDeviceState, pairDevicePhoneAudioState, pairedDeviceMediaAudioState,
                pairedDeviceA2DPState, pairedDeviceAVRCPState);
            m_pairedDevice.replace(index, pairedDevice1);
            emit dataChanged(this->createIndex(index, 1), this->createIndex(index, 1));
        }
        index++;
    }
}

void CBluetoothPairedDeviceModel::updatePairedDeviceSelectConnectState(
    int index, bool phoneAudioState, bool mediaAudioState)
{
    LOGD(tag, "updatePairedDeviceSelectConnectState\n");

    if (index >= 0 && index < m_pairedDevice.size()) {
        const CBluetoothPairedDevice &pairedDevice = m_pairedDevice.at(index);
        QString pairedDeviceName = pairedDevice.pairedDeviceName();
        QString pairedDeviceAddress = pairedDevice.pairedDeviceAddress();
        QString pairedDeviceState = pairedDevice.pairedDeviceState();
        bool pairedDeviceA2DPState = pairedDevice.pairedDeviceA2DPState();
        bool pairedDeviceAVRCPState = pairedDevice.pairedDeviceAVRCPState();

        const CBluetoothPairedDevice pairedDevice1(pairedDeviceName,
                        pairedDeviceAddress, pairedDeviceState,
                        phoneAudioState, mediaAudioState,
                        pairedDeviceA2DPState, pairedDeviceAVRCPState);

        m_pairedDevice.replace(index, pairedDevice1);
        emit dataChanged(this->createIndex(index, 1), this->createIndex(index, 1));
    } else {
        LOGD(tag, "updatePairedDeviceSelectConnectState, invalid index is %d, list size is %d\n",
                    index, m_pairedDevice.size());
    }
}

void CBluetoothPairedDeviceModel::updatePairedDeviceHFPState(
    QString pairedDeviceAddress, bool HFPState)
{
    LOGD(tag, "updatePairedDeviceHFPState\n");

    int index = 0;
    QList<CBluetoothPairedDevice>::iterator pairedDevice_iter = m_pairedDevice.begin();

    for (; pairedDevice_iter != m_pairedDevice.end(); pairedDevice_iter++) {
        const CBluetoothPairedDevice &pairedDevice = *pairedDevice_iter;
        if (pairedDeviceAddress == pairedDevice.pairedDeviceAddress()) {
            QString pairedDeviceName = pairedDevice.pairedDeviceName();
            QString pairedDeviceAddress = pairedDevice.pairedDeviceAddress();
            QString pairedDeviceState = pairedDevice.pairedDeviceState();
            bool pairedDeviceMediaAudioState = pairedDevice.pairedDeviceMediaAudioState();
            bool pairedDeviceA2DPState = pairedDevice.pairedDeviceA2DPState();
            bool pairedDeviceAVRCPState = pairedDevice.pairedDeviceAVRCPState();

            const CBluetoothPairedDevice pairedDevice1(pairedDeviceName, pairedDeviceAddress,
                pairedDeviceState, HFPState, pairedDeviceMediaAudioState,
                pairedDeviceA2DPState, pairedDeviceAVRCPState);
            m_pairedDevice.replace(index, pairedDevice1);
            emit dataChanged(this->createIndex(index, 1), this->createIndex(index, 1));
            break;
        }
        index++;
    }
}

void CBluetoothPairedDeviceModel::updatePairedDeviceA2DPState(
    QString pairedDeviceAddress, bool A2DPState)
{
    LOGD(tag, "updatePairedDeviceA2DPState\n");

    int index = 0;
    QList<CBluetoothPairedDevice>::iterator pairedDevice_iter = m_pairedDevice.begin();

    for (; pairedDevice_iter != m_pairedDevice.end(); pairedDevice_iter++) {
        const CBluetoothPairedDevice &pairedDevice = *pairedDevice_iter;
        if (pairedDeviceAddress == pairedDevice.pairedDeviceAddress()) {
            QString pairedDeviceName = pairedDevice.pairedDeviceName();
            QString pairedDeviceAddress = pairedDevice.pairedDeviceAddress();
            QString pairedDeviceState = pairedDevice.pairedDeviceState();
            bool pairedDevicePhoneAudioState = pairedDevice.pairedDevicePhoneAudioState();
            bool pairedDeviceAVRCPState = pairedDevice.pairedDeviceAVRCPState();
            bool pairedDeviceMediaAudioState = pairedDeviceAVRCPState || A2DPState;

            const CBluetoothPairedDevice pairedDevice1(pairedDeviceName, pairedDeviceAddress,
                pairedDeviceState, pairedDevicePhoneAudioState, pairedDeviceMediaAudioState,
                A2DPState, pairedDeviceAVRCPState);
            m_pairedDevice.replace(index, pairedDevice1);
            emit dataChanged(this->createIndex(index, 1), this->createIndex(index, 1));
            break;
        }
        index++;
    }
}

void CBluetoothPairedDeviceModel::updatePairedDeviceAVRCPState(
    QString pairedDeviceAddress, bool AVRCPState)
{
    LOGD(tag, "updatePairedDeviceAVRCPState\n");

    int index = 0;
    QList<CBluetoothPairedDevice>::iterator pairedDevice_iter = m_pairedDevice.begin();

    for (; pairedDevice_iter != m_pairedDevice.end(); pairedDevice_iter++) {
        const CBluetoothPairedDevice &pairedDevice = *pairedDevice_iter;
        if (pairedDeviceAddress == pairedDevice.pairedDeviceAddress()) {
            QString pairedDeviceName = pairedDevice.pairedDeviceName();
            QString pairedDeviceAddress = pairedDevice.pairedDeviceAddress();
            QString pairedDeviceState = pairedDevice.pairedDeviceState();
            bool pairedDevicePhoneAudioState = pairedDevice.pairedDevicePhoneAudioState();
            bool pairedDeviceA2DPState = pairedDevice.pairedDeviceA2DPState();
            bool pairedDeviceMediaAudioState = pairedDeviceA2DPState || AVRCPState;

            const CBluetoothPairedDevice pairedDevice1(pairedDeviceName, pairedDeviceAddress,
                pairedDeviceState, pairedDevicePhoneAudioState, pairedDeviceMediaAudioState,
                pairedDeviceA2DPState, AVRCPState);
            m_pairedDevice.replace(index, pairedDevice1);
            emit dataChanged(this->createIndex(index, 1), this->createIndex(index, 1));
            break;
        }
        index++;
    }
}

void CBluetoothPairedDeviceModel::updatePairedDeviceMediaAudioState(
    QString pairedDeviceAddress, bool mediaAudioState)
{
    LOGD(tag, "updatePairedDeviceMediaAudioState\n");

    int index = 0;
    QList<CBluetoothPairedDevice>::iterator pairedDevice_iter = m_pairedDevice.begin();

    for (; pairedDevice_iter != m_pairedDevice.end(); pairedDevice_iter++) {
        const CBluetoothPairedDevice &pairedDevice = *pairedDevice_iter;
        if (pairedDeviceAddress == pairedDevice.pairedDeviceAddress()) {
            QString pairedDeviceName = pairedDevice.pairedDeviceName();
            QString pairedDeviceAddress = pairedDevice.pairedDeviceAddress();
            QString pairedDeviceState = pairedDevice.pairedDeviceState();
            bool pairedDevicePhoneAudioState = pairedDevice.pairedDevicePhoneAudioState();
            bool pairedDeviceA2DPState = pairedDevice.pairedDeviceA2DPState();
            bool pairedDeviceAVRCPState = pairedDevice.pairedDeviceAVRCPState();

            const CBluetoothPairedDevice pairedDevice1(pairedDeviceName, pairedDeviceAddress,
                pairedDeviceState, pairedDevicePhoneAudioState, mediaAudioState,
                pairedDeviceA2DPState, pairedDeviceAVRCPState);
            m_pairedDevice.replace(index, pairedDevice1);
            emit dataChanged(this->createIndex(index, 1), this->createIndex(index, 1));
            break;
        }
        index++;
    }
}

void CBluetoothPairedDeviceModel::getPairedDeviceAddress(
    QString pairedDeviceState, QString &pairedDeviceAddress)
{
    LOGD(tag, "getPairedDeviceAddress, pairedDeviceState is %s\n",
            (pairedDeviceState.toStdString()).c_str());

    QList<CBluetoothPairedDevice>::iterator pairedDevice_iter = m_pairedDevice.begin();

    for (; pairedDevice_iter != m_pairedDevice.end(); pairedDevice_iter++) {
        const CBluetoothPairedDevice &pairedDevice = *pairedDevice_iter;
        if (pairedDeviceState == pairedDevice.pairedDeviceState()) {
            pairedDeviceAddress = pairedDevice.pairedDeviceAddress();
            break;
        }
    }
}

void CBluetoothPairedDeviceModel::getPairedDeviceState(
    QString pairedDeviceAddress, QString &pairedDeviceState)
{
    LOGD(tag, "getPairedDeviceState, pairedDeviceAddress is %s\n",
        (pairedDeviceAddress.toStdString()).c_str());

    QList<CBluetoothPairedDevice>::iterator pairedDevice_iter = m_pairedDevice.begin();

    for (; pairedDevice_iter != m_pairedDevice.end(); pairedDevice_iter++) {
        const CBluetoothPairedDevice &pairedDevice = *pairedDevice_iter;
        if (pairedDeviceAddress == pairedDevice.pairedDeviceAddress()) {
            pairedDeviceState = pairedDevice.pairedDeviceState();
            break;
        }
    }
}

void CBluetoothPairedDeviceModel::getPairedDeviceState(
    int index, QString &pairedDeviceState)
{
    LOGD(tag, "getPairedDeviceState, index is %d\n", index);

    if (index >= 0 && index < m_pairedDevice.size()) {
        const CBluetoothPairedDevice &pairedDevice = m_pairedDevice.at(index);
        pairedDeviceState = pairedDevice.pairedDeviceState();
    } else {
        LOGD(tag, "getPairedDeviceState, invalid index is %d, list size is %d\n", index, m_pairedDevice.size());
    }
}

void CBluetoothPairedDeviceModel::getPairedDeviceMediaAudioState(
    QString pairedDeviceAddress, bool &mediaAudioState)
{
    LOGD(tag, "getPairedDeviceMediaAudioState, pairedDeviceAddress is %s\n",
            (pairedDeviceAddress.toStdString()).c_str());

    QList<CBluetoothPairedDevice>::iterator pairedDevice_iter = m_pairedDevice.begin();

    for (; pairedDevice_iter != m_pairedDevice.end(); pairedDevice_iter++) {
        const CBluetoothPairedDevice &pairedDevice = *pairedDevice_iter;
        if (pairedDeviceAddress == pairedDevice.pairedDeviceAddress()) {
            mediaAudioState = pairedDevice.pairedDeviceMediaAudioState();
            break;
        }
    }
}

bool CBluetoothPairedDeviceModel::checkPairedDeviceState(
    int index, QString pairedDeviceState)
{
    LOGD(tag, "checkPairedDeviceState\n");

    bool b_pairedDeviceState = false;
    if (index >= 0 && index < m_pairedDevice.size()) {
        const CBluetoothPairedDevice &pairedDevice = m_pairedDevice.at(index);
        if (pairedDeviceState == pairedDevice.pairedDeviceState()) {
            b_pairedDeviceState = true;
        }
    } else {
        LOGD(tag, "checkPairedDeviceState, invalid index is %d, list size is %d\n", index, m_pairedDevice.size());
    }

    return b_pairedDeviceState;
}


void CBluetoothPairedDeviceModel::deletePairedDevice(QString pairedDeviceAddress)
{
    LOGD(tag, "deletePairedDevice\n");

    int index = 0;
    QList<CBluetoothPairedDevice>::iterator pairedDevice_iter = m_pairedDevice.begin();

    for (; pairedDevice_iter != m_pairedDevice.end();) {
        const CBluetoothPairedDevice &pairedDevice = *pairedDevice_iter;
        if (pairedDeviceAddress == pairedDevice.pairedDeviceAddress()) {
            beginRemoveRows(QModelIndex(), index, index);
            pairedDevice_iter = m_pairedDevice.erase(pairedDevice_iter);
            endRemoveRows();
            break;
        } else {
            pairedDevice_iter++;
        }
        index++;
    }
}

int CBluetoothPairedDeviceModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return m_pairedDevice.count();

}

QVariant CBluetoothPairedDeviceModel::data(const QModelIndex & index, int role) const
{
    if (index.row() < 0 || index.row() >= m_pairedDevice.count()) {
        return QVariant();
    }

    const CBluetoothPairedDevice &pairedDevice = m_pairedDevice[index.row()];

    if (PAIRED_DEVICE_NAME_ROLE == role) {
        return pairedDevice.pairedDeviceName();
    } else if (PAIRED_DEVICE_ADDRESS_ROLE == role) {
        return pairedDevice.pairedDeviceAddress();
    } else if (PAIRED_DEVICE_STATE_ROLE == role) {
        return pairedDevice.pairedDeviceState();
    } else if (PAIRED_DEVICE_PHONE_AUDIO_STATE_ROLE == role) {
        return pairedDevice.pairedDevicePhoneAudioState();
    } else if (PAIRED_DEVICE_MEDIA_AUDIO_STATE_ROLE == role) {
        return pairedDevice.pairedDeviceMediaAudioState();
    }
    return QVariant();

}

QHash<int, QByteArray> CBluetoothPairedDeviceModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[PAIRED_DEVICE_NAME_ROLE]  = STRING_PAIRED_DEVICE_NAME;
    roles[PAIRED_DEVICE_ADDRESS_ROLE] = STRING_PAIRED_DEVICE_ADDRESS;
    roles[PAIRED_DEVICE_STATE_ROLE] = STRING_PAIRED_DEVICE_STATE;
    roles[PAIRED_DEVICE_PHONE_AUDIO_STATE_ROLE] = STRING_PAIRED_PHONE_AUDIO_DEVICE_STATE;
    roles[PAIRED_DEVICE_MEDIA_AUDIO_STATE_ROLE] = STRING_PAIRED_MEDIA_AUDIO_DEVICE_STATE;

    return roles;
}

