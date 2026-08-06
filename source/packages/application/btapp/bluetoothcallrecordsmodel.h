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

#ifndef BLUETOOTHCALLRECORDSMODEL_H
#define BLUETOOTHCALLRECORDSMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include "../../connectivity/universal_utils/include/singleton.h"
#include "applog.h"

#define STRING_CALL_RECORDS_INDEX              "bluetoothCallRecordsIndex"
#define STRING_CALL_RECORDS_NAME               "bluetoothCallRecordsName"
#define STRING_CALL_RECORDS_NUMBER             "bluetoothCallRecordsNumber"
#define STRING_CALL_RECORDS_TYPE               "bluetoothCallRecordsType"

class CBluetoothCallRecords
{
public:
    CBluetoothCallRecords(const int &callRecordsIndex, const QString &callRecordsName,
        const QString callRecordsNumber, const QString callRecordsType);

    int callRecordsIndex() const;
    QString callRecordsName() const;
    QString callRecordsNumber() const;
    QString callRecordsType() const;

private:
    int m_callRecordsIndex;
    QString m_callRecordsName;
    QString m_callRecordsNumber;
    QString m_callRecordsType;
};

class CBluetoothCallRecordsModel
    : public QAbstractListModel
    , public universal_utils::Singleton<CBluetoothCallRecordsModel>
{
    Q_OBJECT

public:

    CBluetoothCallRecordsModel(QObject *parent = 0);
    ~CBluetoothCallRecordsModel();
    void addCallRecords(const CBluetoothCallRecords &callRecords);
    void addIncomingCallRecords(const CBluetoothCallRecords &callRecords);
    void addOutgoingCallRecords(const CBluetoothCallRecords &callRecords);
    void addMissingCallRecords(const CBluetoothCallRecords &callRecords);
    void insertIncomingCallRecords(const CBluetoothCallRecords &callRecords);
    void insertOutgoingCallRecords(const CBluetoothCallRecords &callRecords);
    void insertMissingCallRecords(const CBluetoothCallRecords &callRecords);
    void clearCallRecords();
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QList<CBluetoothCallRecords> getCallRecords();
    void getCallRecordsNumber(int index, QString &number);
    void showIncomingCallRecords();
    void showOutgoingCallRecords();
    void showMissingCallRecords();

    typedef enum {
        CALL_RECORDS_INDEX_ROLE = Qt::UserRole + 1,
        CALL_RECORDS_NAME_ROLE,
        CALL_RECORDS_NUMBER_ROLE,
        CALL_RECORDS_TYPE_ROLE,
    }E_CALL_RECORDS_ROLES;

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QList<CBluetoothCallRecords> m_callRecords;
    QList<CBluetoothCallRecords> m_incomingCallRecords;
    QList<CBluetoothCallRecords> m_outgoingCallRecords;
    QList<CBluetoothCallRecords> m_missingCallRecords;
};

#endif