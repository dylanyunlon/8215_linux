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
 
#include "bluetoothphonebookmodel.h"
using namespace universal_utils;

static const char* const tag = "CBluetoothPhoneBookModel";
template<> CBluetoothPhoneBookModel* Singleton<CBluetoothPhoneBookModel>::msSingleton = NULL;

CBluetoothPhoneBook::CBluetoothPhoneBook(const QString &phoneBookIndex,
    const QString &phoneBookName, const QString phoneBookNumber, const QString phoneBookType)
    : m_phoneBookIndex(phoneBookIndex)
    , m_phoneBookName(phoneBookName)
    , m_phoneBookNumber(phoneBookNumber)
    , m_phoneBookType(phoneBookType)
{

}

QString CBluetoothPhoneBook::phoneBookIndex() const
{
    return m_phoneBookIndex;
}

QString CBluetoothPhoneBook::phoneBookName() const
{
    return m_phoneBookName;
}

QString CBluetoothPhoneBook::phoneBookNumber() const
{
    return m_phoneBookNumber;
}

QString CBluetoothPhoneBook::phoneBookType() const
{
    return m_phoneBookType;
}


CBluetoothPhoneBookModel::CBluetoothPhoneBookModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

CBluetoothPhoneBookModel::~CBluetoothPhoneBookModel()
{
    LOGD(tag, "destructor\n");

    if (0 != m_phoneBook.size() || 0 != m_phoneBookTemp.size()) {
        clearPhoneBookRecords();
    }
    if (0 != m_phoneBookSearchResult.size()) {
        clearPhoneBookSearchResults();
    }
}

void CBluetoothPhoneBookModel::addPhoneBookRecords(
    const CBluetoothPhoneBook &phoneBook)
{
    LOGD(tag, "addPhoneBookRecords\n");

   beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_phoneBook << phoneBook;
    m_phoneBookTemp << phoneBook;
    endInsertRows();
}

void CBluetoothPhoneBookModel::addPhoneBookSearchResults(
    const CBluetoothPhoneBook &phoneBook)
{
    LOGD(tag, "addPhoneBookRecords\n");

    m_phoneBookSearchResult << phoneBook;
}

void CBluetoothPhoneBookModel::clearPhoneBookRecords()
{
    LOGD(tag, "clearPhoneBookRecords\n");

    beginRemoveRows(QModelIndex(), 0, rowCount());
    m_phoneBook.clear();
    m_phoneBookTemp.clear();
    endRemoveRows();
}

void CBluetoothPhoneBookModel::clearPhoneBookSearchResults()
{
    LOGD(tag, "clearPhoneBookSearchResults\n");

    m_phoneBookSearchResult.clear();
}

int CBluetoothPhoneBookModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return m_phoneBook.count();
}

QVariant CBluetoothPhoneBookModel::data(const QModelIndex & index, int role) const
{
    if (index.row() < 0 || index.row() >= m_phoneBook.count()) {
        return QVariant();
    }

    const CBluetoothPhoneBook &phoneBook = m_phoneBook[index.row()];
    if (PHONE_BOOK_INDEX_ROLE == role) {
        return phoneBook.phoneBookIndex();
    } else if (PHONE_BOOK_NAME_ROLE == role) {
        return phoneBook.phoneBookName();
    } else if (PHONE_BOOK_NUMBER_ROLE == role) {
        return phoneBook.phoneBookNumber();
    } else if (PHONE_BOOK_TYPE_ROLE == role) {
        return phoneBook.phoneBookType();
    }

    return QVariant();
}

QList<CBluetoothPhoneBook> CBluetoothPhoneBookModel::getPhoneBook()
{
    LOGD(tag, "getPhoneBook\n");

    return m_phoneBook;
}

void CBluetoothPhoneBookModel::getPhoneBookNumber(int index, QString &number)
{
    LOGD(tag, "getPhoneBookNumber, index is %d\n", index);

    if ((0 != m_phoneBook.size()) && (-1 < index)) {
        const CBluetoothPhoneBook &phoneBook = m_phoneBook.at(index);
        number = phoneBook.phoneBookNumber();
    }
}

int CBluetoothPhoneBookModel::getPhoneBookSearchResultsListSize()
{
    LOGD(tag, "getPhoneBookSearchResultsListSize\n");

    return m_phoneBookSearchResult.size();
}

void CBluetoothPhoneBookModel::showPhoneBookRecords()
{
    LOGD(tag, "showPhoneBookRecords\n");

    beginRemoveRows(QModelIndex(), 0, rowCount());
    m_phoneBook.clear();
    endRemoveRows();

    m_phoneBook = m_phoneBookTemp;
    beginInsertRows(QModelIndex(), 0, rowCount()-1);
    endInsertRows();
}

void CBluetoothPhoneBookModel::showPhoneBookSearchResults()
{
    LOGD(tag, "showPhoneBookSearchResults\n");

    beginRemoveRows(QModelIndex(), 0, rowCount());
    m_phoneBook.clear();
    endRemoveRows();

    m_phoneBook = m_phoneBookSearchResult;
    beginInsertRows(QModelIndex(), 0, rowCount()-1);
    endInsertRows();
}

QHash<int, QByteArray> CBluetoothPhoneBookModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[PHONE_BOOK_INDEX_ROLE]  = STRING_PHONE_BOOK_INDEX;
    roles[PHONE_BOOK_NAME_ROLE]   = STRING_PHONE_BOOK_NAME;
    roles[PHONE_BOOK_NUMBER_ROLE] = STRING_PHONE_BOOK_NUMBER;
    roles[PHONE_BOOK_TYPE_ROLE]   = STRING_PHONE_BOOK_TYPE;

    return roles;
}

