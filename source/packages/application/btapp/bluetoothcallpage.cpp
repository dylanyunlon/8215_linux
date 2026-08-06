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
 
#include "bluetoothcallpage.h"
#include "bluetoothutils.h"

using namespace std;
static const char* const tag = "CBluetoothCallPage";

CBluetoothCallPage::CBluetoothCallPage()
    : m_callPageState(false)
    , m_audioSourceInHF(false)
    , m_autoListenState(false)
    , m_phoneBookState(false)
    , m_incomingCall(false)
    , m_outgoingCall(false)
    , m_missingCall(false)
    , m_audioState(false)
    , m_phoneFactroy("")
    , m_phoneSerial("")
    , m_dialNumber("")
    , m_hfpInterface(NULL)
    , m_pbapInterface(NULL)
    , m_bluetoothHFPCallBack(NULL)
    , m_bluetoothCallRecordsModel(NULL)
{
    m_internationalAreaCodeList.clear();
    m_internationalAreaCodeList.push_back("+");
    m_internationalAreaCodeList.push_back("+86");
    m_internationalAreaCodeList.push_back("86");
    m_internationalAreaCodeList.push_back("086");
    m_internationalAreaCodeList.push_back("0086");

    m_localAreaCodeList.clear();
    m_localAreaCodeList.push_back("0755");
    m_localAreaCodeList.push_back("0551");

    m_specialPhoneMap.clear();
    m_specialPhoneMap.insert("SAMSUNG", "SM-N9009");
}

CBluetoothCallPage::~CBluetoothCallPage()
{
    LOGD(tag, "[CBluetoothCallPage] destructor\n");

}

//connect the hfp callback signal function and this lot function
void CBluetoothCallPage::initBluetoothCallPage()
{
    m_bluetoothHFPCallBack = CBluetoothHFPCallBack::getSingletonPtr();
    if (NULL == m_bluetoothHFPCallBack) {
        LOGE(tag, "m_bluetoothHFPCallBack is empty!\n");
    }

    QObject::connect(m_bluetoothHFPCallBack, SIGNAL(sigCallStateChange(int)),
        this, SLOT(doCallStateChange(int)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothHFPCallBack, SIGNAL(sigCallListChange()),
        this, SLOT(doCallListChange()), Qt::QueuedConnection);
    QObject::connect(m_bluetoothHFPCallBack, SIGNAL(sigHFDisconnectedResponse(QString)),
        this, SLOT(doHFDisconnectedResponse(QString)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothHFPCallBack, SIGNAL(sigScoState(bool)),
        this, SLOT(doScoState(bool)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothHFPCallBack, SIGNAL(sigPhoneFactroy(QString)),
        this, SLOT(doPhoneFactroy(QString)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothHFPCallBack, SIGNAL(sigPhoneSerial(QString)),
        this, SLOT(doPhoneSerial(QString)), Qt::QueuedConnection);
}

//get the hfp interface
void CBluetoothCallPage::getHFPInterface(IBluetoothHandsfree *hfpInterface)
{
    // bt app may be loss callstate/list changed call back, do getCallState & List;
    int currentCallState = HF_CALLSTATE_IDLE;

    m_hfpInterface = hfpInterface;
    if(NULL != m_hfpInterface ) {
        currentCallState = m_hfpInterface->getCallState();
        if (currentCallState != HF_CALLSTATE_IDLE) {
            E_HF_AUDIOTOWARDS sco = m_hfpInterface->getAudioTransferTowards();
            doCallListChange();
            doScoState((sco == HF_AUDIOTOWARDS_HF ? true : false));
        }
    } else {
        LOGE(tag, "m_hfpInterface is empty!\n");
    }
}

//get the pbap interface
void CBluetoothCallPage::getPBAPInterface(IBluetoothPBAP *pbapInterface)
{
    m_pbapInterface = pbapInterface;
    if(NULL != m_pbapInterface ) {
        ;
    } else {
        LOGE(tag, "m_pbapInterface is empty!\n");
    }
}
QString CBluetoothCallPage::getPhoneName(const string &callNumber)
{
    bool ret = false;
    int comp = -1;
    std::string areaCode;
    std::string strTmp = "";
    std::list<std::string>::iterator iter;
    QString retName = STRING_CALLNAME;
    string stCallNumber = callNumber;
    ret = getRecordByNumber(stCallNumber, retName);
    if (false == ret) {
        LOGD(tag, "fix internationalAreaCodeList\n");
        for (iter = m_internationalAreaCodeList.begin();
             iter != m_internationalAreaCodeList.end();
             iter++) {
            areaCode = *iter;
            comp = stCallNumber.compare(0, areaCode.length(), areaCode);
            if (0 == comp) {
                strTmp = stCallNumber.substr(areaCode.length());
                LOGD(tag, "strTmp is %s\n", strTmp.c_str());
                break;
            } else {
                LOGD(tag, "comp is %d\n", comp);
            }
        }
        if ("" != strTmp) {
            stCallNumber = strTmp;
            ret = getRecordByNumber(stCallNumber, retName);
        }
    }
    if (false == ret) {
        LOGD(tag, "fix localAreaCodeList\n");
        for (iter = m_localAreaCodeList.begin();
             iter != m_localAreaCodeList.end();
             iter++) {
            areaCode = *iter;
            comp = stCallNumber.compare(0, areaCode.length(), areaCode);
            if (0 == comp) {
                strTmp = stCallNumber.substr(areaCode.length());
                LOGD(tag, "strTmp is %s\n", strTmp.c_str());
                break;
            } else {
                LOGD(tag, "comp is %d\n", comp);
            }
        }
         if ("" != strTmp) {
            stCallNumber = strTmp;
            getRecordByNumber(stCallNumber, retName);
        }
    }
    return retName;
}

//the bluetoothCallPageView get the audio source state
bool CBluetoothCallPage::getAudioSourceInHFState()
{
    LOGD(tag, "getAudioSourceInHFState\n");

    return m_audioSourceInHF;
}

//the local user switch the audio source
void CBluetoothCallPage::switchAudioSource(bool audioSourceInHF)
{
    LOGD(tag, "switchAudioSource\n");

    if(NULL != m_hfpInterface ) {
        if (true == audioSourceInHF) {
            LOGD(tag, "swithAudioTransferTowards -> HF \n");
            m_hfpInterface->swithAudioTransferTowards(HF_AUDIOTOWARDS_HF);
        } else {
            LOGD(tag, "swithAudioTransferTowards -> AG \n");
            m_hfpInterface->swithAudioTransferTowards(HF_AUDIOTOWARDS_AG);
        }
    } else {
        LOGE(tag, "m_hfpInterface is empty!\n");
    }
}

//the local user answer the incoming phone call
void CBluetoothCallPage::acceptPhoneCall()
{
    LOGD(tag, "acceptPhoneCall\n");

    if(NULL != m_hfpInterface ) {
        LOGD(tag, "acceptIncommingCall\n");
        m_hfpInterface->acceptIncommingCall();
    } else {
        LOGE(tag, "m_hfpInterface is empty!\n");
    }
}

//the local user terminate the speaking phone call
void CBluetoothCallPage::terminatePhoneCall()
{
    LOGD(tag, "terminatePhoneCall\n");

    if(NULL != m_hfpInterface ) {
        m_hfpInterface->terminatePhoneCall();
    } else {
        LOGE(tag, "m_hfpInterface is empty!\n");
    }
}

void CBluetoothCallPage::holdPhoneCall(int action)
{
    LOGD(tag, "holdPhoneCall\n");
    if(NULL != m_hfpInterface) {
        m_hfpInterface->callHold((E_HF_CALL_ACTION)action);
    } else {
        LOGE(tag, "m_hfpInterface is empty!\n");
    }
}
//the local user input the DTMF code
void CBluetoothCallPage::inputDTMFCode(QString dtmfCode)
{
    LOGD(tag, "inputDTMFCode\n");

    if(NULL != m_hfpInterface ) {
        LOGD(tag, "sendDTMFCode, m_DTMFCode is %s\n",(dtmfCode.toStdString()).c_str());
        m_hfpInterface->sendDTMFCode(dtmfCode.toStdString());
    } else {
        LOGE(tag, "m_hfpInterface is empty!\n");
    }
}

//if auto answer is active, answer the incoming call after the timer stop(5s)
void CBluetoothCallPage::autoAnswerTimerStop()
{
    LOGD(tag, "autoAnswerTimerStop\n");

    if ((true == m_autoListenState) && (true == m_callPageState)) {
        LOGD(tag, "auto answer phone call!\n");
        acceptPhoneCall();
    }
}

void CBluetoothCallPage::doCallStateChange(int callState)
{
    LOGD(tag, "doCallStateChange callState is %d \n", callState);

    if (HF_CALLSTATE_INCOMING == callState) {
        LOGD(tag, "HF_CALLSTATE_INCOMING\n");
        onIncomingCallState();
    } else if (HF_CALLSTATE_OUTGOING == callState) {
        LOGD(tag, "HF_CALLSTATE_OUTGOING\n");
        onOutgoingCallState();
    } else if (HF_CALLSTATE_ALERTING == callState) {
        LOGD(tag, "HF_CALLSTATE_ALERTING\n");
        onOutgoingCallState();
    } else if (HF_CALLSTATE_SPEAKING == callState) {
        LOGD(tag, "HF_CALLSTATE_SPEAKING\n");
        onSpeakingCallState();
    } else if (HF_CALLSTATE_IDLE == callState){
        LOGD(tag, "HF_CALLSTATE_IDLE\n");
        onIdleCallState();
    }
}
void CBluetoothCallPage::doCallListChange()
{
    LOGD(tag, "doCallListChange\n");
    CBluetoothCallListModel *callListMode = CBluetoothCallListModel::getSingletonPtr();
    vector<BluetoothHfClientCall> hfpCalls;
    m_hfpInterface->getCall(hfpCalls);
    callListMode->clearCalls();
    for (BluetoothHfClientCall call : hfpCalls) {
        QString phoneName = getPhoneName(call.getNumber());
        CBluetoothCall btCall(call, phoneName);
        callListMode->addCall(btCall);
        addCallRecordList(btCall);
    }

    emit callListChanged();
}

int CBluetoothCallPage::convertCallState(int state) {
    int callstateCluster = 0;
    return callstateCluster;
}

void CBluetoothCallPage::doPhoneFactroy(QString phoneFactroy)
{
    LOGD(tag, "doPhoneFactroy\n");

    m_phoneFactroy = phoneFactroy;
}

void CBluetoothCallPage::doPhoneSerial(QString phoneSerial)
{
    LOGD(tag, "doPhoneSerial\n");

    m_phoneSerial = phoneSerial;

//get the call number from hfpcallback
}

//get sco state from hfpcallback
void CBluetoothCallPage::doScoState(bool audioSourceInHF)
{
    LOGD(tag, "doScoState\n");

    m_audioSourceInHF = audioSourceInHF;
    LOGD(tag, "m_audioSourceInHF is %d\n", m_audioSourceInHF);

    emit scoStateChanged(m_audioSourceInHF);
}

//get the hpf disconnect state from bluetoothPairedRecordsPage
void CBluetoothCallPage::doHFDisconnectedResponse(QString btdev_address)
{
    LOGD(tag, "doHFDisconnectedResponse, btdev_address is %s\n",
        BluetoothUtils::StringForLog(btdev_address.toStdString()).c_str());

    m_phoneBookState = false;
    m_callPageState = false;
    m_phoneFactroy = "";
    m_phoneSerial= "";

    enableCallAudio(false);

    emit bluetoothCallPageChanged(m_callPageState);
}

//get the auto answer state from bluetoothSettingPage or when it first open bluetooth APP (the default state is false)
void CBluetoothCallPage::doAutoListenStateChanged(bool autoListenState)
{
    m_autoListenState = autoListenState;
    LOGD(tag, "m_autoListenState is %d\n", m_autoListenState);
}

//get the phone book state from bluetoothCallRecordsBooksPage
void CBluetoothCallPage::doGetPhoneBookState()
{
    LOGD(tag, "doGetPhoneBookState\n");

    m_phoneBookState = true;
}

void CBluetoothCallPage::doDialNumber(QString dialNumber)
{
    LOGD(tag, "doDialNumber\n");

    m_dialNumber = dialNumber;
}


bool CBluetoothCallPage::getRecordByNumber(std::string callNumber, QString &phoneName)
{
    bool ret = false;

    std::list<PBRecord> pbRecordList;
    int startIdx = 0;
    int searchCount = 1;

    //when the phone book have been download
    if (true == m_phoneBookState && NULL != m_pbapInterface) {
        //search name depend on number from pbDataBase
        pbRecordList.clear();
        LOGD(tag, "getRecordByNumber\n");
        m_pbapInterface->getRecordByNumber((E_PBType)(PBMGR_PHONEBOOK | PBMGR_SIM_PHONEBOOK),
            callNumber, startIdx, searchCount, pbRecordList);
        LOGI(tag, "pbRecordList size is %d\n", pbRecordList.size());

        //have been searched the name
        if (0 != pbRecordList.size()) {
            list<PBRecord>::iterator pbIterator = pbRecordList.begin();
            PBRecord newPBRecord = *pbIterator;
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

                QString m_phoneName = STRING_CALLNAME;

                if ("" == pbName || " " == pbName || "  " == pbName || "   " == pbName) {
                    pbName = "unknown";
                } else {
                    m_phoneName = QString::fromStdString(pbName);
            }
            phoneName = m_phoneName;
            ret = true;
        } else {
            phoneName = STRING_CALLNAME;
        }
    } else {
        phoneName = STRING_CALLNAME;
    }

    return ret;
}

void CBluetoothCallPage::addCallRecordList(const CBluetoothCall &call)
{
    list<CBluetoothCall>::iterator it = m_callRecordList.begin();
    for (; it != m_callRecordList.end(); ++it) {
        if ((it->getCall()).getNumber() == (call.getCall()).getNumber()) {
            if (it->getCallRecordStatus() == CALL_MISSING && (call.getCall()).getState() == HF_CALLSTATE_SPEAKING)
                it->setCallRecordStatus(CALL_INCOMING);
            break;
        }
    }

    if (it == m_callRecordList.end()) {
        CBluetoothCall addCall = call;
        switch ((addCall.getCall()).getState()) {
            case HF_CALLSTATE_INCOMING:
                addCall.setCallRecordStatus(CALL_MISSING);
                m_callRecordList.push_back(addCall);
                break;

            case HF_CALLSTATE_OUTGOING:
            case HF_CALLSTATE_ALERTING:
                addCall.setCallRecordStatus(CALL_OUTGOING);
                m_callRecordList.push_back(addCall);
                break;
        }
    }
}

//when the call is finish, put the new call record to CallRecordsList
void CBluetoothCallPage::updateCallRecordsList()
{
    m_bluetoothCallRecordsModel = CBluetoothCallRecordsModel::getSingletonPtr();
    LOGD(tag, "m_bluetoothCallRecordsModel is %p\n", m_bluetoothCallRecordsModel);

    list<CBluetoothCall>::iterator it = m_callRecordList.begin();
    for (; it != m_callRecordList.end(); ++it) {
        switch (it->getCallRecordStatus()) {
            case CALL_INCOMING:
                m_bluetoothCallRecordsModel->insertIncomingCallRecords(
                CBluetoothCallRecords(0, it->getName(), QString::fromStdString((it->getCall()).getNumber()), QString::fromStdString("incoming")));
                emit sigCallRecordsState(true, false, false);
                break;

            case CALL_OUTGOING:
                m_bluetoothCallRecordsModel->insertOutgoingCallRecords(
                CBluetoothCallRecords(0, it->getName(), QString::fromStdString((it->getCall()).getNumber()), QString::fromStdString("outgoing")));
                emit sigCallRecordsState(false, true, false);
                break;

            case CALL_MISSING:
                m_bluetoothCallRecordsModel->insertMissingCallRecords(
                CBluetoothCallRecords(0, it->getName(), QString::fromStdString((it->getCall()).getNumber()), QString::fromStdString("missing")));
                emit sigCallRecordsState(false, false, true);
                break;
        }
    }

    m_callRecordList.clear();
}

void CBluetoothCallPage::onIncomingCallState()
{
    LOGD(tag, "onIncomingCallState\n");

    m_incomingCall = true;

    if (true == m_autoListenState) {
        emit triggerAutoAnswerTimer();
    }
    enableCallPageState(true);
}

void CBluetoothCallPage::onOutgoingCallState()
{
    LOGD(tag, "onOutgoingCallState\n");

    m_outgoingCall = true;
    enableCallAudio(true);
    enableCallPageState(true);
}
void CBluetoothCallPage::onSpeakingCallState()
{
    LOGD(tag, "onSpeakingCallState\n");

    enableCallAudio(true);
    enableCallPageState(true);

}

void CBluetoothCallPage::onIdleCallState()
{
    LOGD(tag, "onIdleCallState\n");

    updateCallRecordsList();
    enableCallAudio(false);
    enableCallPageState(false);

    m_dialNumber = "";
    m_incomingCall = false;
    m_outgoingCall = false;

}

void CBluetoothCallPage::enableCallAudio(bool enable)
{
    LOGD(tag, "enableCallAudio, enable is %d\n", enable);

    m_audioState = enable;
}

void CBluetoothCallPage::enableCallPageState(bool enable)
{
    LOGD(tag, "enableCallPageState, enable is %d\n", enable);

    if (!m_callPageState && enable) {
        m_callPageState = enable;
        emit bluetoothCallPageChanged(m_callPageState);
    } else if (!enable) {
        m_callPageState = enable;
        emit bluetoothCallPageChanged(m_callPageState);
    }
}


