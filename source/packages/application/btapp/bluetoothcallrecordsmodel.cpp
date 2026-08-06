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
 
#include "bluetoothcallrecordsmodel.h"

using namespace universal_utils;

static const char* const tag = "CBluetoothCallRecordsModel";

template<> CBluetoothCallRecordsModel* Singleton<CBluetoothCallRecordsModel>::msSingleton = NULL;

CBluetoothCallRecords::CBluetoothCallRecords(const int &callRecordsIndex, const QString &callRecordsName,
    const QString callRecordsNumber, const QString callRecordsType)
    : m_callRecordsIndex(callRecordsIndex)
    , m_callRecordsName(callRecordsName)
    , m_callRecordsNumber(callRecordsNumber)
    , m_callRecordsType(callRecordsType)
{

}

int CBluetoothCallRecords::callRecordsIndex() const
{
    return m_callRecordsIndex;
}

QString CBluetoothCallRecords::callRecordsName() const
{
    return m_callRecordsName;
}

QString CBluetoothCallRecords::callRecordsNumber() const
{
    return m_callRecordsNumber;
}

QString CBluetoothCallRecords::callRecordsType() const
{
    return m_callRecordsType;
}

////////////////////////////////////////////////////////////////////////////////////


CBluetoothCallRecordsModel::CBluetoothCallRecordsModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

CBluetoothCallRecordsModel::~CBluetoothCallRecordsModel()
{
    LOGD(tag, "destructor\n");

    if (0 != m_callRecords.size() || 0 != m_incomingCallRecords.size() ||
        0 != m_outgoingCallRecords.size() || 0 != m_missingCallRecords.size()) {
        clearCallRecords();
    }
}

void CBluetoothCallRecordsModel::addCallRecords(
    const CBluetoothCallRecords &callRecords)
{
    LOGD(tag, "addCallRecords\n");

   beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_callRecords << callRecords;
    endInsertRows();
}

void CBluetoothCallRecordsModel::addIncomingCallRecords(
    const CBluetoothCallRecords &callRecords)
{
    LOGD(tag, "addIncomingCallRecords\n");

    m_incomingCallRecords << callRecords;

}

void CBluetoothCallRecordsModel::addOutgoingCallRecords(
    const CBluetoothCallRecords &callRecords)
{
    LOGD(tag, "addOutgoingCallRecords\n");

    m_outgoingCallRecords << callRecords;
}

void CBluetoothCallRecordsModel::addMissingCallRecords(
    const CBluetoothCallRecords &callRecords)
{
    LOGD(tag, "addMissingCallRecords\n");

    m_missingCallRecords << callRecords;
}

void CBluetoothCallRecordsModel::insertIncomingCallRecords(
    const CBluetoothCallRecords &callRecords)
{
    LOGD(tag, "insertIncomingCallRecords\n");

    m_incomingCallRecords.insert(0, callRecords);
}

void CBluetoothCallRecordsModel::insertOutgoingCallRecords(
    const CBluetoothCallRecords &callRecords)
{
    LOGD(tag, "insertOutgoingCallRecords\n");

    m_outgoingCallRecords.insert(0, callRecords);
}

void CBluetoothCallRecordsModel::insertMissingCallRecords(
    const CBluetoothCallRecords &callRecords)
{
    LOGD(tag, "insertMissingCallRecords\n");

    m_missingCallRecords.insert(0, callRecords);
}

void CBluetoothCallRecordsModel::clearCallRecords()
{
    LOGD(tag, "clearCallRecords\n");

    beginRemoveRows(QModelIndex(), 0, rowCount());
    m_callRecords.clear();
    endRemoveRows();

    m_incomingCallRecords.clear();
    m_outgoingCallRecords.clear();
    m_missingCallRecords.clear();
}

int CBluetoothCallRecordsModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return m_callRecords.count();
}

QVariant CBluetoothCallRecordsModel::data(const QModelIndex & index, int role) const
{
    if (index.row() < 0 || index.row() >= m_callRecords.count()) {
        return QVariant();
    }

    const CBluetoothCallRecords &callRecords = m_callRecords[index.row()];

    if (CALL_RECORDS_INDEX_ROLE == role) {
        return index.row()+1;
    } else if (CALL_RECORDS_NAME_ROLE == role) {
        return callRecords.callRecordsName();
    } else if (CALL_RECORDS_NUMBER_ROLE == role) {
        return callRecords.callRecordsNumber();
    } else if (CALL_RECORDS_TYPE_ROLE == role) {
        return callRecords.callRecordsType();
    }
    return QVariant();
}

QList<CBluetoothCallRecords> CBluetoothCallRecordsModel::getCallRecords()
{
    LOGD(tag, "getCallRecords\n");

    return m_callRecords;
}

void CBluetoothCallRecordsModel::getCallRecordsNumber(int index, QString &number)
{
    LOGD(tag, "getCallRecordsNumber, index is %d\n", index);

    if ((0 != m_callRecords.size()) && (-1 < index)) {
        const CBluetoothCallRecords &callrecords = m_callRecords.at(index);
        number = callrecords.callRecordsNumber();
    }
}

void CBluetoothCallRecordsModel::showIncomingCallRecords()
{
    LOGD(tag, "showIncomingCallRecords\n");

    beginRemoveRows(QModelIndex(), 0, rowCount());
    m_callRecords.clear();
    endRemoveRows();

    m_callRecords = m_incomingCallRecords;
    beginInsertRows(QModelIndex(), 0, rowCount()-1);
    endInsertRows();
}

void CBluetoothCallRecordsModel::showOutgoingCallRecords()
{
    LOGD(tag, "showOutgoingCallRecords\n");

    beginRemoveRows(QModelIndex(), 0, rowCount());
    m_callRecords.clear();
    endRemoveRows();

    m_callRecords = m_outgoingCallRecords;
    beginInsertRows(QModelIndex(), 0, rowCount()-1);
    endInsertRows();
}

void CBluetoothCallRecordsModel::showMissingCallRecords()
{
    LOGD(tag, "showMissingCallRecords\n");

    beginRemoveRows(QModelIndex(), 0, rowCount());
    m_callRecords.clear();
    endRemoveRows();

    m_callRecords = m_missingCallRecords;
    beginInsertRows(QModelIndex(), 0, rowCount()-1);
    endInsertRows();
}

QHash<int, QByteArray> CBluetoothCallRecordsModel::roleNames() const
{
    LOGD(tag, "roleNames\n");

    QHash<int, QByteArray> roles;
    roles[CALL_RECORDS_INDEX_ROLE]  = STRING_CALL_RECORDS_INDEX;
    roles[CALL_RECORDS_NAME_ROLE]   = STRING_CALL_RECORDS_NAME;
    roles[CALL_RECORDS_NUMBER_ROLE] = STRING_CALL_RECORDS_NUMBER;
    roles[CALL_RECORDS_TYPE_ROLE]   = STRING_CALL_RECORDS_TYPE;

    return roles;
}


