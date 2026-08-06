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
 
#ifndef BLUETOOTHCALLRECORDSBOOKSPAGE_H
#define BLUETOOTHCALLRECORDSBOOKSPAGE_H
#include <QObject>
#include <iostream>

#include "applog.h"
#include "bluetoothapi.h"
#include "bluetoothpbrecord.h"
#include "bluetoothpbapcallback.h"
#include "bluetoothphonebookmodel.h"
#include "bluetoothcallrecordsmodel.h"

#define PHONEBOOKTYPECOUNT        2
#define CALLRECORDSTYPECOUNT      4
#define COMBINRECORDSTYPEINDEX    0
#define INCOMINGRECORDSTYPEINDEX  1
#define OUTGOINGRECORDSTYPEINDEX  2
#define MISSINGRECORDSTYPEINDEX   3
#define STRING_DEFAULTNAME        CBluetoothCallRecordsBooksPage::trUtf8("unknown")

class CBluetoothCallRecordsBooksPage: public QObject
{
    Q_OBJECT
public:
    CBluetoothCallRecordsBooksPage();
    ~CBluetoothCallRecordsBooksPage();
    void initBluetoothCallRecordsBooksPage();
    void getPBAPInterface(IBluetoothPBAP *pbapInterface);

signals:
    void phoneBookDownloadStart();
    void phoneBookListSizeChanged(int m_phoneBookListSize);
    void phoneBookDownloadStop();
    void phoneBookDownloadFinish();
    void phoneBookIsDownloading();
    void phoneBookRefreshFinish();
    void phoneBookSearchResultEmpty();

    void callRecordsDownloadStart();
    void callRecordsListSizeChanged(int m_callRecordsListSize);
    void callRecordsDownloadStop();
    void callRecordsDownloadFinish();
    void callRecordsIsDownloading();
    void callRecordsRefreshFinish();
    void callRecordsStateChanged(bool m_callRecordsIncomingButtonState,
            bool m_callRecordsOutgoingButtonState, bool m_callRecordsMissingButtonState);

    void sigPhoneBookCallOutRequest(QString number);

public slots:
    QString getPhoneBookSearchString();
    bool getPhoneBooksUpdateState();
    int  getPhoneBookListSize();
    int getPhoneBookSize();
    bool getCallRecordsUpdateState();
    int  getCallRecordsListSize();
    bool getCallRecordsIncomingButtonState();
    bool getCallRecordsOutgoingButtonState();
    bool getCallRecordsMissingButtonState();

    void phoneBookSearchString(QString inputtedString);
    void phoneBookCallOutRequest(int index);
    void phoneBooksUpdatePauseRequest();
    void phoneBookListRefreshRequest();
    void phoneBookSearchByString(QString inputtedString);
    void phoneBookListShow();

    void callRecordsCallOutRequest(int index);
    void callRecordsUpdatePauseRequest();
    void callRecordsListRefreshRequest();
    void callRecordsIncomingListRequest();
    void callRecordsOutgoingListRequest();
    void callRecordsMissingListRequest();

    void doPhoneConnectState(bool phoneConnectState);
    void doPhoneBookDownloadStart();
    void doCallRecordsDownloadStart();
    void doPhoneBookDownloadRecord(int pbPathType, int pbCurrentIndex);
    void doCallRecordsDownloadRecord(int pbPathType, int pbCurrentIndex);
    void doPhoneBookDownloadStop();
    void doCallRecordsDownloadStop();
    void doPhoneBookDownloadFinish();
    void doCallRecordsDownloadFinish();

    void doCallRecordsState(bool incomingState, bool outgoingState, bool missingState);
    void doRestartDownload();

private:
    void initPhoneBookList();
    void initCallRecordsList();
    int getRecordsFromPBDatabase(int PathType, int currentIndex, int previousIndex);
    int  getSpecificRecord(int PathType, std::list<PBRecord> pbRecordList);
    void updatePhoneBookList(QString bt_phoneName, QString bt_phoneNumber, int bt_phoneType);
    void getPhoneBookSearchResult();
    void updateCallRecordsList(QString bt_phoneName,
            QString bt_phoneNumber, QString bt_callTime, int  bt_callType);
    QString intToQstring(int index);

    void needUpdateCallRecords();


    bool m_phoneBooksUpdateState;
    bool m_phoneBooksSearchState;
    bool m_phoneBooksSearchIsEmpty;
    bool m_phoneBooksPreviousUpdateState;
    bool m_callRecordsUpdateState;
    bool m_callRecordsPreviousUpdateState;
    bool m_callRecordsIncomingButtonState;
    bool m_callRecordsOutgoingButtonState;
    bool m_callRecordsMissingButtonState;

    int  m_phoneBookPathType[PHONEBOOKTYPECOUNT];
    int  m_phoneBookCurrentIndex[PHONEBOOKTYPECOUNT];
    int  m_phoneBookPreviousIndex[PHONEBOOKTYPECOUNT];
    int  m_phoneBookPreviousPathType;
    int  m_phoneBookUpdateStep;
    int  m_phoneBookListIndex;
    int  m_phoneBookPathCurrentIndex;
    int  m_phoneBookPathPreviousIndex;
    int  m_phoneBookListSize;
    int  m_phoneBookSearchResultListIndex;
    int  m_phoneBookSearchStep;
    QString m_phonebookSearchString;
    std::list<PBRecord> m_phoneBookSearchResultList;

    int  m_callRecordsPathType[CALLRECORDSTYPECOUNT];
    int  m_callRecordsCurrentIndex[CALLRECORDSTYPECOUNT];
    int  m_callRecordsPreviousIndex[CALLRECORDSTYPECOUNT];
    int  m_callRecordsPreviousPathType;
    int  m_callRecordsUpdateStep;
    int  m_callRecordsListIndex;
    int  m_callRecordsIncomingListIndex;
    int  m_callRecordsOutgoingListIndex;
    int  m_callRecordsMissingListIndex;
    int  m_callRecordsPathCurrentIndex;
    int  m_callRecordsPathPreviousIndex;
    int  m_callRecordsListSize;

    IBluetoothPBAP *m_pbapInterface;
    CBluetoothPBAPCallBack *m_bluetoothPBAPCallBack;
    CBluetoothPhoneBookModel *m_bluetoothPhoneBookModel;
    CBluetoothCallRecordsModel *m_bluetoothCallRecordsModel;
};

#endif
