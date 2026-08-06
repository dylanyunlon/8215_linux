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
 
#include "bluetoothcallrecordsbookspage.h"
#include <algorithm>

using namespace std;
static const char* const tag = "CBluetoothCallRecordsBooksPage";


CBluetoothCallRecordsBooksPage::CBluetoothCallRecordsBooksPage()
    : m_phoneBooksUpdateState(false)
    , m_phoneBooksSearchState(false)
    , m_phoneBooksSearchIsEmpty(false)
    , m_phoneBooksPreviousUpdateState(false)
    , m_callRecordsUpdateState(false)
    , m_callRecordsPreviousUpdateState(false)
    , m_callRecordsIncomingButtonState(false)
    , m_callRecordsOutgoingButtonState(false)
    , m_callRecordsMissingButtonState(false)
    , m_phoneBookPreviousPathType(0)
    , m_phoneBookUpdateStep(5)
    , m_phoneBookListIndex(0)
    , m_phoneBookPathCurrentIndex(0)
    , m_phoneBookPathPreviousIndex(0)
    , m_phoneBookListSize(0)
    , m_phoneBookSearchResultListIndex(0)
    , m_phoneBookSearchStep(10)
    , m_phonebookSearchString("")
    , m_callRecordsPreviousPathType(0)
    , m_callRecordsUpdateStep(5)
    , m_callRecordsListIndex(0)
    , m_callRecordsIncomingListIndex(0)
    , m_callRecordsOutgoingListIndex(0)
    , m_callRecordsMissingListIndex(0)
    , m_callRecordsPathCurrentIndex(0)
    , m_callRecordsPathPreviousIndex(0)
    , m_callRecordsListSize(0)
    , m_pbapInterface(NULL)
    , m_bluetoothPBAPCallBack(NULL)
    , m_bluetoothPhoneBookModel(NULL)
    , m_bluetoothCallRecordsModel(NULL)
{
    memset(m_phoneBookPathType, 0, sizeof(m_phoneBookPathType));
    memset(m_phoneBookCurrentIndex, 0, sizeof(m_phoneBookCurrentIndex));
    memset(m_phoneBookPreviousIndex, 0, sizeof(m_phoneBookPreviousIndex));
    memset(m_callRecordsPathType, 0, sizeof(m_callRecordsPathType));
    memset(m_callRecordsCurrentIndex, 0, sizeof(m_callRecordsCurrentIndex));
    memset(m_callRecordsPreviousIndex, 0, sizeof(m_callRecordsPreviousIndex));

    m_bluetoothPhoneBookModel = CBluetoothPhoneBookModel::getSingletonPtr();

    m_bluetoothCallRecordsModel = CBluetoothCallRecordsModel::getSingletonPtr();

}

CBluetoothCallRecordsBooksPage::~CBluetoothCallRecordsBooksPage()
{
    LOGD(tag, "destructor\n");

}

//connect the pbapcallback signal function and this slot function
void CBluetoothCallRecordsBooksPage::initBluetoothCallRecordsBooksPage()
{
    m_bluetoothPBAPCallBack = CBluetoothPBAPCallBack::getSingletonPtr();

    QObject::connect(m_bluetoothPBAPCallBack, SIGNAL(sigPhoneBookDownloadStart()),
                    this, SLOT(doPhoneBookDownloadStart()), Qt::QueuedConnection);
    QObject::connect(m_bluetoothPBAPCallBack, SIGNAL(sigCallRecordsDownloadStart()),
                    this, SLOT(doCallRecordsDownloadStart()), Qt::QueuedConnection);
    QObject::connect(m_bluetoothPBAPCallBack, SIGNAL(sigPhoneBookDownloadRecord(int, int)),
                    this, SLOT(doPhoneBookDownloadRecord(int, int)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothPBAPCallBack, SIGNAL(sigCallRecordsDownloadRecord(int, int)),
                    this, SLOT(doCallRecordsDownloadRecord(int, int)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothPBAPCallBack, SIGNAL(sigPhoneBookDownloadStop()),
                    this, SLOT(doPhoneBookDownloadStop()), Qt::QueuedConnection);
    QObject::connect(m_bluetoothPBAPCallBack, SIGNAL(sigCallRecordsDownloadStop()),
                    this, SLOT(doCallRecordsDownloadStop()), Qt::QueuedConnection);
    QObject::connect(m_bluetoothPBAPCallBack, SIGNAL(sigPhoneBookDownloadFinish()),
                    this, SLOT(doPhoneBookDownloadFinish()), Qt::QueuedConnection);
    QObject::connect(m_bluetoothPBAPCallBack, SIGNAL(sigCallRecordsDownloadFinish()),
                    this, SLOT(doCallRecordsDownloadFinish()), Qt::QueuedConnection);
}

//get the pbap interface
void CBluetoothCallRecordsBooksPage::getPBAPInterface(IBluetoothPBAP *pbapInterface)
{
    m_pbapInterface = pbapInterface;
    if(NULL != m_pbapInterface ) {
        ;
    } else {
        LOGE(tag, "m_pbapInterface is empty!\n");
    }
}

////////////////////////////////////////////////// the slot function /////////////////////////////////////////////////////

//bluetoothPhoneBookPageView get the search number
QString CBluetoothCallRecordsBooksPage::getPhoneBookSearchString()
{
    LOGD(tag, "getPhoneBookSearchString\n");

    return m_phonebookSearchString;
}

//bluetoothPhoneBookPageView get the phone book update state
bool CBluetoothCallRecordsBooksPage::getPhoneBooksUpdateState()
{
    LOGD(tag, "getPhoneBooksUpdateState\n");

    return m_phoneBooksUpdateState;
}

//bluetoothPhoneBookPageView get the phone book list's size
int CBluetoothCallRecordsBooksPage::getPhoneBookListSize()
{
    LOGD(tag, "getPhoneBookListSize\n");

    return m_phoneBookListSize;
}

//bluetoothCallRecordsPageView get the phone book's size in db
int CBluetoothCallRecordsBooksPage::getPhoneBookSize()
{
    LOGD(tag, "getPhoneBookSize\n");

    int count = 0;

    if(NULL != m_pbapInterface )
        m_pbapInterface->getRecordCount((E_PBType)(PBMGR_PHONEBOOK | PBMGR_SIM_PHONEBOOK), count);
    LOGD(tag, "count = %d\n", count);
    return count;
}

//bluetoothCallRecordsPageView get the call records update state
bool CBluetoothCallRecordsBooksPage::getCallRecordsUpdateState()
{
    LOGD(tag, "getCallRecordsUpdateState\n");

    return m_callRecordsUpdateState;
}

//bluetoothCallRecordsPageView get the call records list's size
int CBluetoothCallRecordsBooksPage::getCallRecordsListSize()
{
    LOGD(tag, "getCallRecordsListSize\n");

    return m_callRecordsListSize;
}

//bluetoothCallRecordsPageView get the call records incoming button state
bool CBluetoothCallRecordsBooksPage::getCallRecordsIncomingButtonState()
{
    LOGD(tag, "getCallRecordsIncomingButtonState\n");

    return m_callRecordsIncomingButtonState;
}

//bluetoothCallRecordsPageView get the call records outgoing button state
bool CBluetoothCallRecordsBooksPage::getCallRecordsOutgoingButtonState()
{
    LOGD(tag, "getCallRecordsOutgoingButtonState\n");

    return m_callRecordsOutgoingButtonState;
}

//bluetoothCallRecordsPageView get the call records missing button state
bool CBluetoothCallRecordsBooksPage::getCallRecordsMissingButtonState()
{
    LOGD(tag, "getCallRecordsMissingButtonState\n");

    return m_callRecordsMissingButtonState;
}

//bluetoothPhoneBookPageView set the phone book search string
void CBluetoothCallRecordsBooksPage::phoneBookSearchString(QString inputtedString)
{
    LOGD(tag, "phoneBookSearchString\n");

    m_phonebookSearchString = inputtedString;
}

//the local user send the call out request in bluetoothPhoneBookPageView
void CBluetoothCallRecordsBooksPage::phoneBookCallOutRequest(int index)
{
    LOGD(tag, "phoneBookCallOutRequest, index = %d\n", index);

    if (NULL != m_bluetoothPhoneBookModel && -1 < index) {
        //get the number from PhoneBookModel
        QString number = "";
        m_bluetoothPhoneBookModel->getPhoneBookNumber(index, number);

        emit sigPhoneBookCallOutRequest(number);
    }
}

//the local user send the update or pause request in bluetoothPhoneBookPageView
void CBluetoothCallRecordsBooksPage::phoneBooksUpdatePauseRequest()
{
    LOGD(tag, "phoneBooksUpdatePauseRequest\n");

    if (NULL != m_pbapInterface) {
        //when the call records is not downloading, handle the phone book update or pause request
        if (false == m_callRecordsUpdateState) {
            if (false == m_phoneBooksUpdateState) {
                LOGD(tag, "download\n");
                m_pbapInterface->download((E_PBType)(PBMGR_PHONEBOOK | PBMGR_SIM_PHONEBOOK));
            } else {
                LOGD(tag, "stopDownload\n");
                m_pbapInterface->stopDownload();
            }
        } else {
            emit callRecordsIsDownloading();
        }
    }
}

//the local user click the screen to send the refresh request in bluetoothPhoneBookPageView
void CBluetoothCallRecordsBooksPage::phoneBookListRefreshRequest()
{
    LOGD(tag, "phoneBookListRefreshRequest\n");

    //refresh the whole phoneBook
    if (false == m_phoneBooksSearchState && false == m_phoneBooksSearchIsEmpty) {
        LOGD(tag, "refresh the whole phoneBook\n");
        // PBAP v1.2.3, Sec 3.1.5. The first contact in pb is owner card 0.vcf, which we do not want to download.
        if ((m_phoneBookPreviousIndex[m_phoneBookPathPreviousIndex] ==
                m_phoneBookCurrentIndex[m_phoneBookPathPreviousIndex] - 1) &&
                (m_phoneBookPathPreviousIndex < m_phoneBookPathCurrentIndex)) {
            m_phoneBookPathPreviousIndex++;
        }

        LOGI(tag, "m_phoneBookPathPreviousIndex is %d, m_phoneBookPathCurrentIndex is %d\n",
            m_phoneBookPathPreviousIndex, m_phoneBookPathCurrentIndex);

        if ((m_phoneBookPathPreviousIndex <= m_phoneBookPathCurrentIndex) &&
                (m_phoneBookPreviousIndex[m_phoneBookPathPreviousIndex] <
                m_phoneBookCurrentIndex[m_phoneBookPathPreviousIndex])) {
            LOGI(tag, "m_phoneBookPathType is %d, m_phoneBookCurrentIndex is %d, m_phoneBookPreviousIndex is %d\n",
                m_phoneBookPathType[m_phoneBookPathPreviousIndex],
                m_phoneBookCurrentIndex[m_phoneBookPathPreviousIndex],
                m_phoneBookPreviousIndex[m_phoneBookPathPreviousIndex]);

            m_phoneBookPreviousIndex[m_phoneBookPathPreviousIndex] = getRecordsFromPBDatabase(m_phoneBookPathType[m_phoneBookPathPreviousIndex],
                    m_phoneBookCurrentIndex[m_phoneBookPathPreviousIndex],
                    m_phoneBookPreviousIndex[m_phoneBookPathPreviousIndex]);
        } else {
            E_PBState downloadState = PBDOWNLOADING;
            m_pbapInterface->getState(downloadState);
            if(PBIDLE == downloadState) {
                LOGD(tag, "phoneBookRefreshFinish\n");
                emit phoneBookRefreshFinish();
            }
        }
    }
    //if search phonebook, but the result is empty, notify that the search result is empty
    else if (false == m_phoneBooksSearchState && true == m_phoneBooksSearchIsEmpty) {
        emit phoneBookSearchResultEmpty();
    }
    // refresh the searched phoneBook
    else if (true == m_phoneBooksSearchState){
        LOGD(tag, "refresh the searched phoneBook\n");
        if (NULL != m_pbapInterface && NULL != m_bluetoothPhoneBookModel) {
            //search the records depend on number from pbDateBase
            m_phoneBookSearchResultList.clear();

            LOGD(tag, "searchByString\n");
            int ret = m_pbapInterface->searchByString((E_PBType)(PBMGR_PHONEBOOK| PBMGR_SIM_PHONEBOOK),
                m_phonebookSearchString.toStdString(), m_phoneBookSearchResultListIndex,
                m_phoneBookSearchStep,m_phoneBookSearchResultList);

            LOGI(tag, "ret = %d, size = %d\n", ret, m_phoneBookSearchResultList.size());

            m_phoneBooksSearchState = true;

            if (0 != m_phoneBookSearchResultList.size()) {
                getPhoneBookSearchResult();
            } else {
                emit phoneBookRefreshFinish();
            }
        }
    }
}

//the local user send the search request in bluetoothPhoneBookPageView
void CBluetoothCallRecordsBooksPage::phoneBookSearchByString(QString inputtedString)
{
    LOGD(tag, "phoneBookSearchByString\n");

    string searchString = m_phonebookSearchString.toStdString();
    string inputString = inputtedString.toStdString();
    LOGD(tag, "searchString = %s, inputString = %s\n", searchString.c_str(), inputString.c_str());

    int searchStringLen = searchString.length();
    int inputStringLen = inputString.length();
    int ret = 0;
    if (searchStringLen == inputStringLen) {
        ret = inputString.compare(0, inputStringLen, searchString);
    } else {
        ret = 1;
    }

    if (0 != ret) {
        m_phonebookSearchString = inputtedString;

        if (NULL != m_pbapInterface && NULL != m_bluetoothPhoneBookModel) {
            m_phoneBookSearchResultList.clear();
            m_phoneBookSearchResultListIndex = 0;

            //search the records depend on number from pbDateBase
            LOGD(tag, "searchByString\n");
            int ret = m_pbapInterface->searchByString((E_PBType)(PBMGR_PHONEBOOK| PBMGR_SIM_PHONEBOOK),
                m_phonebookSearchString.toStdString(), m_phoneBookSearchResultListIndex,
                m_phoneBookSearchStep, m_phoneBookSearchResultList);

            LOGI(tag, "ret = %d, size = %d\n", ret, m_phoneBookSearchResultList.size());

            m_phoneBooksSearchState = true;

            if (0 != m_phoneBookSearchResultList.size()) {
                m_bluetoothPhoneBookModel->clearPhoneBookSearchResults();
                getPhoneBookSearchResult();
            } else {
                //cannot find the phone number
                emit phoneBookSearchResultEmpty();
                m_phoneBooksSearchState = false;
                m_phoneBooksSearchIsEmpty = true;
                m_bluetoothPhoneBookModel->clearPhoneBookSearchResults();
                getPhoneBookSearchResult();
            }
        }
    }
}

//when the search number is empty, show the previous phoneBookList
void CBluetoothCallRecordsBooksPage::phoneBookListShow()
{
    LOGD(tag, "phoneBookListShow\n");

    m_phoneBooksSearchState = false;
    m_phoneBooksSearchIsEmpty = false;
    m_phonebookSearchString = "";
    m_phoneBookSearchResultListIndex = 0;

    if (NULL != m_bluetoothPhoneBookModel) {
        int searchResultsListSize = m_bluetoothPhoneBookModel->getPhoneBookSearchResultsListSize();
        LOGI(tag, "searchResultsListSize= %d\n", searchResultsListSize);
        if (/*0 != searchResultsListSize*/true) {
            m_bluetoothPhoneBookModel->clearPhoneBookSearchResults();
            m_bluetoothPhoneBookModel->showPhoneBookRecords();
        }
    }
}

//the local user send the call out request in bluetoothCallRecordsPageView
void CBluetoothCallRecordsBooksPage::callRecordsCallOutRequest(int index)
{
    LOGD(tag, "callRecordsCallOutRequest, index = %d\n", index);

    if (NULL != m_bluetoothCallRecordsModel && -1 < index) {
        //get the number from CallRecordsModel
        QString number = "";
        m_bluetoothCallRecordsModel->getCallRecordsNumber(index, number);

        emit sigPhoneBookCallOutRequest(number);
    }
}

//the local user send the update or pause request in bluetoothCallRecordsPageView
void CBluetoothCallRecordsBooksPage::callRecordsUpdatePauseRequest()
{
    LOGD(tag, "callRecordsUpdatePauseRequest\n");

    if (NULL != m_pbapInterface) {
        //when the phone book is not downloading, handle the call records update or pause request
        if (false == m_phoneBooksUpdateState) {
            if (false == m_callRecordsUpdateState) {
                LOGD(tag, "download\n");
                m_pbapInterface->download((E_PBType)(PBMGR_INCOMING_CALLS_HISTORY |
                    PBMGR_OUTGOING_CALLS_HISTORY | PBMGR_MISSED_CALLS_HISTORY));
            } else {
                LOGD(tag, "stopDownload\n");
                m_pbapInterface->stopDownload();
            }
        } else {
            emit phoneBookIsDownloading();
        }
    }
}

//the local user click the screen to send the refresh request in bluetoothCallRecordsPageView
void CBluetoothCallRecordsBooksPage::callRecordsListRefreshRequest()
{
    LOGD(tag, "callRecordsListRefreshRequest\n");

    //refresh the whole callRecords
    if ((false == m_callRecordsIncomingButtonState) &&
        (false == m_callRecordsOutgoingButtonState) &&
        (false == m_callRecordsMissingButtonState)) {
        if ((m_callRecordsPreviousIndex[m_callRecordsPathPreviousIndex] ==
             m_callRecordsCurrentIndex[m_callRecordsPathPreviousIndex]) &&
            (m_callRecordsPathPreviousIndex < m_callRecordsPathCurrentIndex)) {
            m_callRecordsPathPreviousIndex++;
        }

        LOGI(tag, "m_callRecordsPathPreviousIndex is %d, m_callRecordsPathCurrentIndex is %d\n",
            m_callRecordsPathPreviousIndex, m_callRecordsPathCurrentIndex);

        if ((m_callRecordsPathPreviousIndex <= m_callRecordsPathCurrentIndex) &&
            (m_callRecordsPreviousIndex[m_callRecordsPathPreviousIndex] <
             m_callRecordsCurrentIndex[m_callRecordsPathPreviousIndex])) {
            LOGI(tag, "m_callRecordsPathType is %d, m_callRecordsCurrentIndex is %d, m_callRecordsPreviousIndex is %d\n",
                m_callRecordsPathType[m_callRecordsPathPreviousIndex],
                m_callRecordsCurrentIndex[m_callRecordsPathPreviousIndex],
                m_callRecordsPreviousIndex[m_callRecordsPathPreviousIndex]);
            getRecordsFromPBDatabase(m_callRecordsPathType[m_callRecordsPathPreviousIndex],
                                     m_callRecordsCurrentIndex[m_callRecordsPathPreviousIndex],
                                     m_callRecordsPreviousIndex[m_callRecordsPathPreviousIndex]);
        } else {
            LOGD(tag, "callRecordsRefreshFinish\n");
            emit callRecordsRefreshFinish();
        }
    }
    //refresh the incoming callRecords
    else if ((true == m_callRecordsIncomingButtonState) &&
             (false == m_callRecordsOutgoingButtonState) &&
             (false == m_callRecordsMissingButtonState)) {
        //get the incoming records from pbDataBase
        if (m_callRecordsPreviousIndex[INCOMINGRECORDSTYPEINDEX] <
            m_callRecordsCurrentIndex[INCOMINGRECORDSTYPEINDEX]) {
            getRecordsFromPBDatabase(m_callRecordsPathType[INCOMINGRECORDSTYPEINDEX],
                                     m_callRecordsCurrentIndex[INCOMINGRECORDSTYPEINDEX],
                                     m_callRecordsPreviousIndex[INCOMINGRECORDSTYPEINDEX]);
        } else {
            emit callRecordsRefreshFinish();
        }
    }
    //refresh the outgoing callRecords
    else if ((false == m_callRecordsIncomingButtonState) &&
             (true == m_callRecordsOutgoingButtonState) &&
             (false == m_callRecordsMissingButtonState)) {
        //get the outgoing records from pbDataBase
        if (m_callRecordsPreviousIndex[OUTGOINGRECORDSTYPEINDEX] <
            m_callRecordsCurrentIndex[OUTGOINGRECORDSTYPEINDEX]) {
            getRecordsFromPBDatabase(m_callRecordsPathType[OUTGOINGRECORDSTYPEINDEX],
                                     m_callRecordsCurrentIndex[OUTGOINGRECORDSTYPEINDEX],
                                     m_callRecordsPreviousIndex[OUTGOINGRECORDSTYPEINDEX]);
        } else {
            emit callRecordsRefreshFinish();
        }
    }
    //refresh the missing callRecords
    else if ((false == m_callRecordsIncomingButtonState) &&
             (false == m_callRecordsOutgoingButtonState) &&
             (true == m_callRecordsMissingButtonState)) {
        if (m_callRecordsPreviousIndex[MISSINGRECORDSTYPEINDEX] <
            m_callRecordsCurrentIndex[MISSINGRECORDSTYPEINDEX]) {
            //get the missing records from pbDataBase
            getRecordsFromPBDatabase(m_callRecordsPathType[MISSINGRECORDSTYPEINDEX],
                                     m_callRecordsCurrentIndex[MISSINGRECORDSTYPEINDEX],
                                     m_callRecordsPreviousIndex[MISSINGRECORDSTYPEINDEX]);
        } else {
            emit callRecordsRefreshFinish();
        }
    }
}

//the local user click the incoming button to send the request in bluetoothCallRecordsPageView
void CBluetoothCallRecordsBooksPage::callRecordsIncomingListRequest()
{
    LOGD(tag, "callRecordsIncomingListRequest\n");

    m_callRecordsIncomingButtonState = true;
    m_callRecordsOutgoingButtonState = false;
    m_callRecordsMissingButtonState = false;

    //get the incoming records from pbDataBase
    if (m_callRecordsPreviousIndex[INCOMINGRECORDSTYPEINDEX] <
        m_callRecordsCurrentIndex[INCOMINGRECORDSTYPEINDEX]) {
        getRecordsFromPBDatabase(m_callRecordsPathType[INCOMINGRECORDSTYPEINDEX],
                                 m_callRecordsCurrentIndex[INCOMINGRECORDSTYPEINDEX],
                                 m_callRecordsPreviousIndex[INCOMINGRECORDSTYPEINDEX]);
    }

    if (NULL != m_bluetoothCallRecordsModel) {
        LOGD(tag, "showIncomingCallRecords\n");
        m_bluetoothCallRecordsModel->showIncomingCallRecords();
    }
}

//the local user click the outgoing button to send the request in bluetoothCallRecordsPageView
void CBluetoothCallRecordsBooksPage::callRecordsOutgoingListRequest()
{
    LOGD(tag, "callRecordsOutgoingListRequest\n");

    m_callRecordsIncomingButtonState = false;
    m_callRecordsOutgoingButtonState = true;
    m_callRecordsMissingButtonState = false;

    //get the outgoing records from pbDataBase
    if (m_callRecordsPreviousIndex[OUTGOINGRECORDSTYPEINDEX] <
        m_callRecordsCurrentIndex[OUTGOINGRECORDSTYPEINDEX]) {
        getRecordsFromPBDatabase(m_callRecordsPathType[OUTGOINGRECORDSTYPEINDEX],
                                 m_callRecordsCurrentIndex[OUTGOINGRECORDSTYPEINDEX],
                                 m_callRecordsPreviousIndex[OUTGOINGRECORDSTYPEINDEX]);
    }

    if (NULL != m_bluetoothCallRecordsModel) {
        LOGD(tag, "showOutgoingCallRecords\n");
        m_bluetoothCallRecordsModel->showOutgoingCallRecords();
    }
}

//the local user click the missing button to send the request in bluetoothCallRecordsPageView
void CBluetoothCallRecordsBooksPage::callRecordsMissingListRequest()
{
    LOGD(tag, "callRecordsMissingListRequest\n");

    m_callRecordsIncomingButtonState = false;
    m_callRecordsOutgoingButtonState = false;
    m_callRecordsMissingButtonState = true;

    //get the missing records from pbDataBase
    if (m_callRecordsPreviousIndex[MISSINGRECORDSTYPEINDEX] <
        m_callRecordsCurrentIndex[MISSINGRECORDSTYPEINDEX]) {
        getRecordsFromPBDatabase(m_callRecordsPathType[MISSINGRECORDSTYPEINDEX],
                                 m_callRecordsCurrentIndex[MISSINGRECORDSTYPEINDEX],
                                 m_callRecordsPreviousIndex[MISSINGRECORDSTYPEINDEX]);
    }

    if (NULL != m_bluetoothCallRecordsModel) {
        LOGD(tag, "showMissingCallRecords\n");
        m_bluetoothCallRecordsModel->showMissingCallRecords();
    }
}

//get the hfp connect state from bluetoothPairedRecordsPage
void CBluetoothCallRecordsBooksPage::doPhoneConnectState(bool phoneConnectState)
{
    LOGD(tag, "doPhoneConnectState, phoneConnectState = %d\n", phoneConnectState);

    initPhoneBookList();
    doPhoneBookDownloadStop();
    initCallRecordsList();
    doCallRecordsDownloadStop();
}

//get the phone book download start indication from pbapcallback
void CBluetoothCallRecordsBooksPage::doPhoneBookDownloadStart()
{
    LOGD(tag, "doPhoneBookDownloadStart\n");

    initPhoneBookList();
    m_phoneBooksUpdateState = true;

    emit phoneBookDownloadStart();
}

//get the call records download start indication from pbapcallback
void CBluetoothCallRecordsBooksPage::doCallRecordsDownloadStart()
{
    LOGD(tag, "doCallRecordsDownloadStart\n");

    initCallRecordsList();
    m_callRecordsUpdateState = true;

    emit callRecordsDownloadStart();
}

//get the phone book download records indication from pbapcallback
void CBluetoothCallRecordsBooksPage::doPhoneBookDownloadRecord(int pbPathType, int pbCurrentIndex)
{
    LOGD(tag, "doPhoneBookDownloadRecord, pbPathType is %d, pbCurrentIndex is %d\n",
        pbPathType, pbCurrentIndex);

    int oneStepCount = 0;

    if ((0 == m_phoneBookPreviousPathType) || (pbPathType == m_phoneBookPreviousPathType)) {
        m_phoneBookPathType[m_phoneBookPathCurrentIndex] = pbPathType;
        oneStepCount = pbCurrentIndex - m_phoneBookCurrentIndex[m_phoneBookPathCurrentIndex];
        m_phoneBookCurrentIndex[m_phoneBookPathCurrentIndex] = pbCurrentIndex;
    } else {
        m_phoneBookPathCurrentIndex++;
        m_phoneBookPathType[m_phoneBookPathCurrentIndex] = pbPathType;
        oneStepCount = pbCurrentIndex - m_phoneBookCurrentIndex[m_phoneBookPathCurrentIndex];
        m_phoneBookCurrentIndex[m_phoneBookPathCurrentIndex] = pbCurrentIndex;
        m_phoneBookPreviousIndex[m_phoneBookPathCurrentIndex] = 0;
    }
    m_phoneBookPreviousPathType = pbPathType;
    m_phoneBookListSize = m_phoneBookListSize + oneStepCount;
    emit phoneBookListSizeChanged(m_phoneBookListSize);

    if (false == m_phoneBooksSearchState) {
        phoneBookListRefreshRequest();
    }
}

//get the call records download records indication from pbapcallback
void CBluetoothCallRecordsBooksPage::doCallRecordsDownloadRecord(int pbPathType, int pbCurrentIndex)
{
    LOGD(tag, "doCallRecordsDownloadRecord, pbPathType is %d, pbCurrentIndex is %d\n",
        pbPathType, pbCurrentIndex);

    int oneStepCount = 0;

    if ((0 == m_callRecordsPreviousPathType) || (pbPathType == m_callRecordsPreviousPathType)) {
        m_callRecordsPathType[m_callRecordsPathCurrentIndex] = pbPathType;
        oneStepCount = pbCurrentIndex - m_callRecordsCurrentIndex[m_callRecordsPathCurrentIndex];
        m_callRecordsCurrentIndex[m_callRecordsPathCurrentIndex] = pbCurrentIndex;
    } else {
        m_callRecordsPathCurrentIndex++;
        m_callRecordsPathType[m_callRecordsPathCurrentIndex] = pbPathType;
        oneStepCount = pbCurrentIndex - m_callRecordsCurrentIndex[m_callRecordsPathCurrentIndex];
        m_callRecordsCurrentIndex[m_callRecordsPathCurrentIndex] = pbCurrentIndex;
        m_callRecordsPreviousIndex[m_callRecordsPathCurrentIndex] = 0;
    }
    m_callRecordsPreviousPathType = pbPathType;
    m_callRecordsListSize = m_callRecordsListSize + oneStepCount;
    emit callRecordsListSizeChanged(m_callRecordsListSize);

    if ((m_callRecordsListIndex >= 0) && (m_callRecordsListIndex < 10)) {
        if ((false == m_callRecordsIncomingButtonState) &&
        (false == m_callRecordsOutgoingButtonState) &&
        (false == m_callRecordsMissingButtonState)) {
            callRecordsListRefreshRequest();
        }
    }
}

//get the phone book download stop indication from pbapcallback
void CBluetoothCallRecordsBooksPage::doPhoneBookDownloadStop()
{
    LOGD(tag, "doPhoneBookDownloadStop\n");

    m_phoneBooksUpdateState = false;

    emit phoneBookDownloadStop();
}

//get the call records download stop indication from pbapcallback
void CBluetoothCallRecordsBooksPage::doCallRecordsDownloadStop()
{
    LOGD(tag, "doCallRecordsDownloadStop\n");

    m_callRecordsUpdateState = false;
    if (NULL != m_pbapInterface) {
        m_callRecordsPathType[INCOMINGRECORDSTYPEINDEX] = PBMGR_INCOMING_CALLS_HISTORY;
        m_callRecordsPathType[OUTGOINGRECORDSTYPEINDEX] = PBMGR_OUTGOING_CALLS_HISTORY;
        m_callRecordsPathType[MISSINGRECORDSTYPEINDEX] = PBMGR_MISSED_CALLS_HISTORY;

        m_pbapInterface->getRecordCount(PBMGR_INCOMING_CALLS_HISTORY, m_callRecordsCurrentIndex[INCOMINGRECORDSTYPEINDEX]);
        m_pbapInterface->getRecordCount(PBMGR_OUTGOING_CALLS_HISTORY, m_callRecordsCurrentIndex[OUTGOINGRECORDSTYPEINDEX]);
        m_pbapInterface->getRecordCount(PBMGR_MISSED_CALLS_HISTORY, m_callRecordsCurrentIndex[MISSINGRECORDSTYPEINDEX]);
    }

    emit callRecordsDownloadStop();

    needUpdateCallRecords();
}

//get the phone book download finish indication from pbapcallback
void CBluetoothCallRecordsBooksPage::doPhoneBookDownloadFinish()
{
    LOGD(tag, "doPhoneBookDownloadFinish\n");

    m_phoneBooksUpdateState = false;

    emit phoneBookDownloadFinish();
}

void CBluetoothCallRecordsBooksPage::needUpdateCallRecords()
{
    if (m_callRecordsIncomingButtonState) {
        doCallRecordsState(true, false, false);
    } else if (m_callRecordsOutgoingButtonState) {
        doCallRecordsState(false, true, false);
    } else if (m_callRecordsMissingButtonState) {
        doCallRecordsState(false, false, true);
    } else {
        // nothing
    }
}

//get the call records download finish indication from pbapcallback
void CBluetoothCallRecordsBooksPage::doCallRecordsDownloadFinish()
{
    LOGD(tag, "doCallRecordsDownloadFinish\n");

    m_callRecordsUpdateState = false;
    if (NULL != m_pbapInterface) {
        m_callRecordsPathType[INCOMINGRECORDSTYPEINDEX] = PBMGR_INCOMING_CALLS_HISTORY;
        m_callRecordsPathType[OUTGOINGRECORDSTYPEINDEX] = PBMGR_OUTGOING_CALLS_HISTORY;
        m_callRecordsPathType[MISSINGRECORDSTYPEINDEX] = PBMGR_MISSED_CALLS_HISTORY;

        m_pbapInterface->getRecordCount(PBMGR_INCOMING_CALLS_HISTORY, m_callRecordsCurrentIndex[INCOMINGRECORDSTYPEINDEX]);
        m_pbapInterface->getRecordCount(PBMGR_OUTGOING_CALLS_HISTORY, m_callRecordsCurrentIndex[OUTGOINGRECORDSTYPEINDEX]);
        m_pbapInterface->getRecordCount(PBMGR_MISSED_CALLS_HISTORY, m_callRecordsCurrentIndex[MISSINGRECORDSTYPEINDEX]);
    }

    emit callRecordsDownloadFinish();

    needUpdateCallRecords();
}

//get the new call records sate from bluetoothCallPage
void CBluetoothCallRecordsBooksPage::doCallRecordsState(
    bool incomingState, bool outgoingState, bool missingState)
{
    LOGD(tag, "doCallRecordsState\n");

    m_callRecordsIncomingButtonState = incomingState;
    m_callRecordsOutgoingButtonState = outgoingState;
    m_callRecordsMissingButtonState = missingState;

    emit callRecordsStateChanged(m_callRecordsIncomingButtonState,
        m_callRecordsOutgoingButtonState, m_callRecordsMissingButtonState);

    if (true == m_callRecordsIncomingButtonState) {
        callRecordsIncomingListRequest();
    } else if (true == m_callRecordsOutgoingButtonState) {
        callRecordsOutgoingListRequest();
    } else if (true == m_callRecordsMissingButtonState) {
        callRecordsMissingListRequest();
    }
}

//after a call finish, restart a download which was active previous
void CBluetoothCallRecordsBooksPage::doRestartDownload()
{
    LOGD(tag, "doRestartDownload\n");

    if (NULL != m_pbapInterface) {
        if (true == m_phoneBooksPreviousUpdateState) {
            LOGD(tag, "download phonebook\n");
            m_pbapInterface->download((E_PBType)(PBMGR_PHONEBOOK | PBMGR_SIM_PHONEBOOK));
            m_phoneBooksPreviousUpdateState = false;
        } else if (true == m_callRecordsPreviousUpdateState) {
            LOGD(tag, "download callrecords\n");
            m_pbapInterface->download((E_PBType)(PBMGR_INCOMING_CALLS_HISTORY |
                PBMGR_OUTGOING_CALLS_HISTORY | PBMGR_MISSED_CALLS_HISTORY));
            m_callRecordsPreviousUpdateState = false;
        }
    }
}

void CBluetoothCallRecordsBooksPage::initPhoneBookList()
{
    LOGD(tag, "initPhoneBookList\n");

    m_phoneBooksSearchState = false;
    m_phoneBooksSearchIsEmpty = false;
    m_phoneBooksPreviousUpdateState = false;
    m_phoneBookPreviousPathType = 0;
    m_phoneBookListIndex = 0;
    m_phoneBookPathCurrentIndex = 0;
    m_phoneBookPathPreviousIndex = 0;
    m_phoneBookListSize = 0;
    m_phoneBookSearchResultListIndex = 0;
    m_phonebookSearchString = "";

    for (int i = 0; i < PHONEBOOKTYPECOUNT; i++) {
        m_phoneBookPathType[i] = 0;
        m_phoneBookCurrentIndex[i] = 0;
        m_phoneBookPreviousIndex[i] = 0;
    }

    if (NULL != m_bluetoothPhoneBookModel) {
        m_bluetoothPhoneBookModel->clearPhoneBookRecords();
    }

}

void CBluetoothCallRecordsBooksPage::initCallRecordsList()
{
    LOGD(tag, "initCallRecordsList\n");

    m_callRecordsPreviousUpdateState = false;
    m_callRecordsIncomingButtonState = false;
    m_callRecordsOutgoingButtonState = false;
    m_callRecordsMissingButtonState = false;
    m_callRecordsPreviousPathType =0;
    m_callRecordsPathPreviousIndex = 0;
    m_callRecordsPathCurrentIndex = 0;
    m_callRecordsListIndex = 0;
    m_callRecordsIncomingListIndex = 0;
    m_callRecordsOutgoingListIndex = 0;
    m_callRecordsMissingListIndex = 0;
    m_callRecordsListSize = 0;

    emit callRecordsStateChanged(m_callRecordsIncomingButtonState,
        m_callRecordsOutgoingButtonState, m_callRecordsMissingButtonState);

    for (int i = 0; i < CALLRECORDSTYPECOUNT; i++) {
        m_callRecordsPathType[i] = 0;
        m_callRecordsCurrentIndex[i] = 0;
        m_callRecordsPreviousIndex[i] = 0;
    }

    if (NULL != m_bluetoothCallRecordsModel) {
        m_bluetoothCallRecordsModel->clearCallRecords();
    }
}

//get rcords from pbDataBase
int  CBluetoothCallRecordsBooksPage::getRecordsFromPBDatabase(
    int PathType, int currentIndex, int previousIndex)
{
    LOGD(tag, "getRecordsFromPBDatabase\n");

    if (NULL != m_pbapInterface) {
        LOGI(tag, "previousIndex is %d, currentIndex is %d\n", previousIndex, currentIndex);

        std::list<PBRecord> pbRecordList;
        int m_downloadCount1 = 0;
        int m_thresholdCount1 = 0;
        int m_downloadCount2 = 0;
        int m_thresholdCount2 = 0;
        int m_downloadSum = 0;

        while ((previousIndex + m_phoneBookUpdateStep <= currentIndex) && (m_downloadSum < 10)) {
            pbRecordList.clear();
            m_pbapInterface->getRecord((E_PBType)PathType,
                previousIndex, m_phoneBookUpdateStep, pbRecordList);

            m_downloadCount1 = getSpecificRecord(PathType, pbRecordList);
            m_downloadSum = m_downloadSum + m_downloadCount1;

            previousIndex = previousIndex + m_downloadCount1;
            LOGI(tag, "previousIndex is %d, m_downloadCount1 is %d\n", previousIndex, m_downloadCount1);

            if (0 == m_downloadCount1) {
                m_thresholdCount1++;
            }
            if (2 == m_thresholdCount1) {
                break;
            }
        }

        m_downloadSum = 0;
        while ((previousIndex + m_phoneBookUpdateStep > currentIndex) &&
                (previousIndex < currentIndex) &&
                (m_downloadSum <= 10)) {
            pbRecordList.clear();
            m_pbapInterface->getRecord((E_PBType)PathType,
                previousIndex, (currentIndex - previousIndex), pbRecordList);

            m_downloadCount2 = getSpecificRecord(PathType, pbRecordList);
            m_downloadSum = m_downloadSum + m_downloadCount2;

            previousIndex = previousIndex + m_downloadCount2;
            LOGI(tag, "previousIndex is %d, m_downloadCount2 is %d\n", previousIndex, m_downloadCount2);

            if (0 == m_downloadCount2) {
                m_thresholdCount2++;
            }
            if (2 == m_thresholdCount2) {
                if (m_callRecordsPathPreviousIndex < m_callRecordsPathCurrentIndex) {
                    m_callRecordsPathPreviousIndex++;
                }
                break;
            }
        }
    }

    return previousIndex;
}

//get the specific records
int CBluetoothCallRecordsBooksPage::getSpecificRecord(int PathType, std::list<PBRecord> pbRecordList)
{
    LOGD(tag, "getSpecificRecord\n");

    list<PBRecord>::iterator m_iterator;

    int downloadCount = 0;
    for (m_iterator = pbRecordList.begin(); m_iterator != pbRecordList.end(); m_iterator++) {
        PBRecord newPBRecord = *m_iterator;

        string firstName = newPBRecord.getFirstName();
        string middleName = newPBRecord.getMiddleName();
        string givenName = newPBRecord.getGivenName();
        string formattedName = newPBRecord.getFormattedName();
        BluetoothAddress btAddr = newPBRecord.getRemoteAddress();

        PathType = newPBRecord.getType();

        if (" " == firstName) {
            firstName = "";
        }
        if (" " == middleName) {
            middleName = "";
        }
        if (" " == givenName) {
            givenName = "";
        }
        if (" " == formattedName) {
            formattedName = "";
        }
        LOGI(tag, "firstName is %s, middleName is %s, givenName is %s, formatName:%s\n",
            firstName.c_str(), middleName.c_str(), givenName.c_str(), formattedName.c_str());

        string pbName;
        if (formattedName != "") {
            pbName = formattedName;
        } else {
            pbName = firstName + middleName + givenName;
        }
        QString bt_phoneName = STRING_DEFAULTNAME;

        if ("" == pbName || " " == pbName || "  " == pbName || "   " == pbName) {
            pbName = "unknown";
        } else {
            bt_phoneName = QString::fromStdString(pbName);
        }

        string callTime = newPBRecord.getCallTime();
        QString bt_callTime = QString::fromStdString(callTime);
        //LOGI(tag, "pbName is %s, callTime is %s\n", pbName.c_str(), callTime.c_str());

        std::list<PBRecord::PBTel> phoneNumberList;
        phoneNumberList.clear();
        newPBRecord.getTelList(phoneNumberList);
        LOGI(tag, "phoneNumberList.size is %d\n", phoneNumberList.size());

        if (0 != phoneNumberList.size()) {
            int phoneNumberCount = 0;
            list<PBRecord::PBTel>::iterator m_phoneNumberIterator;
            for (m_phoneNumberIterator = phoneNumberList.begin();
                (m_phoneNumberIterator != phoneNumberList.end()) && (phoneNumberCount < 10);
                 m_phoneNumberIterator++) {
                PBRecord::PBTel newPhoneNumber = *m_phoneNumberIterator;
                string phoneNumber = newPhoneNumber.m_telNumber;
                QString bt_phoneNumber = QString::fromStdString(phoneNumber);
                //LOGI(tag, "phoneNumber is %s\n", phoneNumber.c_str());

                if ((PBMGR_PHONEBOOK == (E_PBType)PathType) ||
                    (PBMGR_SIM_PHONEBOOK == (E_PBType)PathType)) {
                    updatePhoneBookList(bt_phoneName, bt_phoneNumber, PathType);
                } else if ((PBMGR_INCOMING_CALLS_HISTORY == (E_PBType)PathType) ||
                            (PBMGR_OUTGOING_CALLS_HISTORY == (E_PBType)PathType) ||
                            (PBMGR_MISSED_CALLS_HISTORY == (E_PBType)PathType)) {
                        //if the callrecords name is empty, search the name from the phone book
                        if ("" == firstName + middleName + givenName) {
                            std::list<PBRecord> pbRecordListSearchFromPhoneBook;
                            int startIdx = 0;
                            int searchCount = 1;
                            if (NULL != m_pbapInterface) {
                                m_pbapInterface->getRecordByNumber(
                                    (E_PBType)(PBMGR_PHONEBOOK | PBMGR_SIM_PHONEBOOK),
                                    phoneNumber, startIdx, searchCount,
                                    pbRecordListSearchFromPhoneBook);
                            }
                            int ret = pbRecordListSearchFromPhoneBook.size();
                            LOGI(tag, "pbRecordListSearchFromPhoneBook.size is %d!\n", ret);
                            if (0 != pbRecordListSearchFromPhoneBook.size()) {
                                list<PBRecord>::iterator m_iterator;
                                for (m_iterator = pbRecordListSearchFromPhoneBook.begin();
                                    m_iterator != pbRecordListSearchFromPhoneBook.end();
                                    m_iterator++) {
                                    PBRecord newPBRecord = *m_iterator;
                                    BluetoothAddress pbAddr = newPBRecord.getRemoteAddress();

                                    if (pbAddr == btAddr) {
                                        firstName = newPBRecord.getFirstName();
                                        middleName = newPBRecord.getMiddleName();
                                        givenName = newPBRecord.getGivenName();
                                        formattedName = newPBRecord.getFormattedName();

                                        if (formattedName != "") {
                                            pbName = formattedName;
                                        } else {
                                            pbName = firstName + middleName + givenName;
                                        }
                                        bt_phoneName = STRING_DEFAULTNAME;

                                        if ("" == pbName || " " == pbName ||
                                            "  " == pbName || "   " == pbName) {
                                            pbName = "unknown";
                                        } else {
                                            bt_phoneName = QString::fromStdString(pbName);
                                        }
                                    } else {
                                        LOGD(tag, "the remote user of phonebook is not the remote user of callrecords!\n");

                                    }
                                }
                            }
                        }
                    updateCallRecordsList(bt_phoneName, bt_phoneNumber, bt_callTime, PathType);
                }
                phoneNumberCount++;
            }
        } else {
            //if the TelList is empty, show the empty phone number
            QString bt_phoneNumber = "";
            if(bt_phoneNumber != "" || pbName != "unknown") {
                if ((PBMGR_PHONEBOOK == (E_PBType)PathType) ||
                    (PBMGR_SIM_PHONEBOOK == (E_PBType)PathType)) {
                    updatePhoneBookList(bt_phoneName, bt_phoneNumber, PathType);
                } else if ((PBMGR_INCOMING_CALLS_HISTORY == (E_PBType)PathType) ||
                            (PBMGR_OUTGOING_CALLS_HISTORY == (E_PBType)PathType) ||
                            (PBMGR_MISSED_CALLS_HISTORY == (E_PBType)PathType)) {
                    updateCallRecordsList(bt_phoneName, bt_phoneNumber, bt_callTime, PathType);
                }
            } else {
                LOGD(tag, "empty name and empty number!\n");
                --m_phoneBookListSize;
                emit phoneBookListSizeChanged(m_phoneBookListSize);
            }
        }
        downloadCount++;
    }

    return downloadCount;
}

void CBluetoothCallRecordsBooksPage::updatePhoneBookList(
    QString bt_phoneName, QString bt_phoneNumber,  int bt_phoneType)
{
    LOGD(tag, "updatePhoneBookList\n");

    QString bt_phoneIndex = intToQstring(m_phoneBookListIndex);
    m_phoneBookListIndex++;

    if (NULL != m_bluetoothPhoneBookModel) {
        QString bt_phoneBookType = "";
        if (PBMGR_PHONEBOOK == (E_PBType)bt_phoneType) {
            bt_phoneBookType = QString::fromStdString("phone");
        } else if (PBMGR_SIM_PHONEBOOK == (E_PBType)bt_phoneType) {
            bt_phoneBookType = QString::fromStdString("SIM");
        }
        //add PhoneBookRecords
        m_bluetoothPhoneBookModel->addPhoneBookRecords(
            CBluetoothPhoneBook(bt_phoneIndex, bt_phoneName, bt_phoneNumber, bt_phoneBookType));
    } else {
        LOGE(tag,"m_bluetoothPhoneBookModel is empty!");
    }
}

//get the phone book search result form pbDataBase
void CBluetoothCallRecordsBooksPage::getPhoneBookSearchResult()
{
    LOGD(tag, "getPhoneBookSearchResult, m_phoneBookSearchResultListIndex is %d\n",
        m_phoneBookSearchResultListIndex);

    list<PBRecord>::iterator m_iterator= m_phoneBookSearchResultList.begin();
    int m_downloadSum = 0;

    for (; m_iterator != m_phoneBookSearchResultList.end() && m_downloadSum < 10; m_iterator++) {
        PBRecord newPBRecord = *m_iterator;

        int phoneType = newPBRecord.getType();
        QString bt_phoneBookType = "";
        if (PBMGR_PHONEBOOK == (E_PBType)phoneType) {
            bt_phoneBookType = QString::fromStdString("phone");
        } else if (PBMGR_SIM_PHONEBOOK == (E_PBType)phoneType) {
            bt_phoneBookType = QString::fromStdString("SIM");
        }

        string firstName = newPBRecord.getFirstName();
        string middleName = newPBRecord.getMiddleName();
        string givenName = newPBRecord.getGivenName();
        string formattedName = newPBRecord.getFormattedName();

        if (" " == firstName) {
            firstName = "";
        }
        if (" " == middleName) {
            middleName = "";
        }
        if (" " == givenName) {
            givenName = "";
        }
        if (" " == formattedName) {
            formattedName = "";
        }

        string pbName;
        if (formattedName != "") {
            pbName = formattedName;
        } else {
            pbName = firstName + middleName + givenName;
        }
        QString bt_phoneName = STRING_DEFAULTNAME;

        if ("" == pbName || " " == pbName || "  " == pbName || "   " == pbName) {
            pbName = "unknown";
        } else {
            bt_phoneName = QString::fromStdString(pbName);
        }
        LOGI(tag, "pbName is %s\n", pbName.c_str());
        string searchString = m_phonebookSearchString.toStdString();
        transform(pbName.begin(), pbName.end(), pbName.begin(), ::toupper);
        transform(searchString.begin(), searchString.end(), searchString.begin(), ::toupper);


        int name_ret = -1;
        const char *pstr;
        pstr = strstr(pbName.c_str(), searchString.c_str());
        if (NULL != pstr) {
            name_ret = 0;
        }

        std::list<PBRecord::PBTel> phoneNumberList;
        phoneNumberList.clear();
        newPBRecord.getTelList(phoneNumberList);
        int telListSize = phoneNumberList.size();
        LOGI(tag, "telListSize is %d\n", telListSize);

        if (0 < telListSize) {
            int number_ret = -1;
            list<PBRecord::PBTel>::iterator m_phoneNumberIterator;
            for (m_phoneNumberIterator = phoneNumberList.begin();
                    m_phoneNumberIterator != phoneNumberList.end() ;
                    m_phoneNumberIterator++) {
                PBRecord::PBTel newPhoneNumber = *m_phoneNumberIterator;
                string phoneNumber = newPhoneNumber.m_telNumber;

                if (0 != name_ret) {
                    //screen out the adaptive number
                    string searchNumber = m_phonebookSearchString.toStdString();
                    int len = searchNumber.length();
                    number_ret = phoneNumber.compare(0, len, searchNumber);
                } else {
                    //if the name is match, do not compare the number
                    number_ret = 0;
                }
                LOGI(tag, "name_ret is %d, number_ret is %d\n", name_ret, number_ret);

                if (0 == number_ret) {
                    QString bt_phoneNumber = QString::fromStdString(phoneNumber);

                    if (NULL != m_bluetoothPhoneBookModel) {
                        QString bt_phoneBookSearchResultListIndex = intToQstring(m_phoneBookSearchResultListIndex);
                        m_bluetoothPhoneBookModel->addPhoneBookSearchResults(
                            CBluetoothPhoneBook(bt_phoneBookSearchResultListIndex,
                            bt_phoneName, bt_phoneNumber, bt_phoneBookType));
                    }
                    m_phoneBookSearchResultListIndex++;
                }
            }
            m_downloadSum++;
        } else if (0  == telListSize) {
            QString bt_phoneNumber = QString::fromStdString("");

            if (NULL != m_bluetoothPhoneBookModel) {
                QString bt_phoneBookSearchResultListIndex = intToQstring(m_phoneBookSearchResultListIndex);
                m_bluetoothPhoneBookModel->addPhoneBookSearchResults(
                    CBluetoothPhoneBook(bt_phoneBookSearchResultListIndex,
                    bt_phoneName, bt_phoneNumber, bt_phoneBookType));
            }
            m_phoneBookSearchResultListIndex++;
        }
    }
    if (NULL != m_bluetoothPhoneBookModel) {
        m_bluetoothPhoneBookModel->showPhoneBookSearchResults();
    }
}

void CBluetoothCallRecordsBooksPage::updateCallRecordsList(
    QString bt_phoneName, QString bt_phoneNumber, QString bt_callTime, int bt_callType)
{
    LOGD(tag, "updateCallRecordsList, bt_callTime is %s\n",
        (bt_callTime.toStdString()).c_str());

    if (NULL != m_bluetoothCallRecordsModel) {
        QString bt_callRecordsType = "";
        if (PBMGR_INCOMING_CALLS_HISTORY == (E_PBType)bt_callType) {
            bt_callRecordsType = QString::fromStdString("incoming");
            //add Incoming CallRecords
            m_bluetoothCallRecordsModel->addIncomingCallRecords(
                CBluetoothCallRecords(m_callRecordsIncomingListIndex,
                    bt_phoneName, bt_phoneNumber, bt_callRecordsType));
            m_callRecordsIncomingListIndex++;
            m_callRecordsPreviousIndex[INCOMINGRECORDSTYPEINDEX]++;
        } else if (PBMGR_OUTGOING_CALLS_HISTORY == (E_PBType)bt_callType){
            bt_callRecordsType = QString::fromStdString("outgoing");
            //add Outgoing CallRecords
            m_bluetoothCallRecordsModel->addOutgoingCallRecords(
                CBluetoothCallRecords(m_callRecordsOutgoingListIndex,
                    bt_phoneName, bt_phoneNumber, bt_callRecordsType));
            m_callRecordsOutgoingListIndex++;
            m_callRecordsPreviousIndex[OUTGOINGRECORDSTYPEINDEX]++;
        } else if (PBMGR_MISSED_CALLS_HISTORY == (E_PBType)bt_callType){
            bt_callRecordsType = QString::fromStdString("missing");
            //add Missing CallRecords
            m_bluetoothCallRecordsModel->addMissingCallRecords(
                CBluetoothCallRecords(m_callRecordsMissingListIndex,
                    bt_phoneName, bt_phoneNumber, bt_callRecordsType));
            m_callRecordsMissingListIndex++;
            m_callRecordsPreviousIndex[MISSINGRECORDSTYPEINDEX]++;
        }
        //add CallRecords
        m_bluetoothCallRecordsModel->addCallRecords(
            CBluetoothCallRecords(m_callRecordsListIndex,
                bt_phoneName, bt_phoneNumber, bt_callRecordsType));
        m_callRecordsListIndex++;
        m_callRecordsPreviousIndex[COMBINRECORDSTYPEINDEX]++;
    } else {
        LOGE(tag, "m_bluetoothCallRecordsModel is empty!");
    }
}

QString CBluetoothCallRecordsBooksPage::intToQstring(int index)
{
    //LOGD(tag, "intToQstring\n");

    char inttochar[256];
    string chartostring;
    string IndexString;

    memset(inttochar, 0, 256);
    sprintf(inttochar , "%d", index+1);
    IndexString = inttochar;
    QString IndexQString = QString::fromStdString(IndexString);

    return IndexQString;
}


