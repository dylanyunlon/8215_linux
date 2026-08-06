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
 
#ifndef BLUETOOTHPHONEBOOKMODEL_H
#define BLUETOOTHPHONEBOOKMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include "singleton.h"
#include "applog.h"

#define STRING_PHONE_BOOK_INDEX              "bluetoothPhoneBookIndex"
#define STRING_PHONE_BOOK_NAME               "bluetoothPhoneBookName"
#define STRING_PHONE_BOOK_NUMBER             "bluetoothPhoneBookNumber"
#define STRING_PHONE_BOOK_TYPE               "bluetoothPhoneBookType"


class CBluetoothPhoneBook
{
public:
    CBluetoothPhoneBook(const QString &phoneBookIndex, const QString &phoneBookName,
        const QString phoneBookNumber, const QString phoneBookType);

    QString phoneBookIndex() const;
    QString phoneBookName() const;
    QString phoneBookNumber() const;
    QString phoneBookType() const;

private:
    QString m_phoneBookIndex;
    QString m_phoneBookName;
    QString m_phoneBookNumber;
    QString m_phoneBookType;
};

class CBluetoothPhoneBookModel
    : public QAbstractListModel
    , public universal_utils::Singleton<CBluetoothPhoneBookModel>
{
    Q_OBJECT

public:

    CBluetoothPhoneBookModel(QObject *parent = 0);
    ~CBluetoothPhoneBookModel();
    void addPhoneBookRecords(const CBluetoothPhoneBook &phoneBook);
    void addPhoneBookSearchResults(const CBluetoothPhoneBook &phoneBook);
    void clearPhoneBookRecords();
    void clearPhoneBookSearchResults();
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QList<CBluetoothPhoneBook> getPhoneBook();
    void getPhoneBookNumber(int index, QString &number);
    int getPhoneBookSearchResultsListSize();
    void showPhoneBookRecords();
    void showPhoneBookSearchResults();

    typedef enum {
        PHONE_BOOK_INDEX_ROLE = Qt::UserRole + 1,
        PHONE_BOOK_NAME_ROLE,
        PHONE_BOOK_NUMBER_ROLE,
        PHONE_BOOK_TYPE_ROLE,
    }E_PHONE_BOOK_ROLES;

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QList<CBluetoothPhoneBook> m_phoneBook;
    QList<CBluetoothPhoneBook> m_phoneBookSearchResult;
    QList<CBluetoothPhoneBook> m_phoneBookTemp;
};


#endif

