#include "BtPtsPbapHandler.h"
using namespace universal_utils;

static const char* TAG = "BtPtsPbapHandler";
const map<string, HANDLE_FUN> BtPtsPbapHandler::PBAP_HANDLE_MAP = {

    {CMD_PROFILE_CONNECT,     static_cast<HANDLE_FUN>(&BtPtsPbapHandler::connect)},
    {CMD_PROFILE_DISCONNECT,  static_cast<HANDLE_FUN>(&BtPtsPbapHandler::disconnect)},
    {CMD_PBAP_DOWNLOAD,       static_cast<HANDLE_FUN>(&BtPtsPbapHandler::download)},
    {CMD_PBAP_STOPDOWNLOAD,   static_cast<HANDLE_FUN>(&BtPtsPbapHandler::stopdownload)},
};

BtPtsPbapHandler::BtPtsPbapHandler(IBluetoothClient *client)
    : BtPtsHandler(PROFILE_PBAP)
    , m_pbapInterface(NULL)
    , m_pbapCallback(NULL)
{
    IBluetoothProfile *profile = NULL;
    client->getProfile(PBAPPROFILENAME, &profile);
    m_pbapInterface = dynamic_cast<IBluetoothPBAP*>(profile);
    EXPECT_NOT_NULL(m_pbapInterface, "m_pbapInterface");
    registerCallBack();
}

BtPtsPbapHandler::~BtPtsPbapHandler()
{
    deregisterCallBack();
}

const map<string, HANDLE_FUN>& BtPtsPbapHandler::getHandleFunMap()
{
    return PBAP_HANDLE_MAP;
}

void BtPtsPbapHandler::registerCallBack()
{
    m_pbapCallback = new BtPtsPBAPCallBack();
    m_pbapInterface->registerCallBack(*m_pbapCallback);
}

void BtPtsPbapHandler::deregisterCallBack()
{
    m_pbapInterface->deregisterCallBack(*m_pbapCallback);
    SAFE_DELETE(m_pbapCallback);
}

int BtPtsPbapHandler::connect()
{
    PRINTF_TO_CONSOLE(TAG, __FUNCTION__);

    int ret = BTPTS_ERROR;

    string address = m_args.getStringExtra("str1", "");
    if (address != "")
        m_address = address;
    PRINTF_TO_CONSOLE(TAG, "connect address: %s", m_address.toString().c_str());
    ret = m_pbapInterface->connect(m_address);
    EXPECT_EQ(BTPTS_OK, ret);
    EXPECT_EQ(true, m_pbapCallback->waitCommandDone(PBAP_IND_DEVICE_CONNECTED, WAIT_TIME));

    return ret;
}

int BtPtsPbapHandler::disconnect()
{
    PRINTF_TO_CONSOLE(TAG, __FUNCTION__);

    int ret = BTPTS_ERROR;

    BluetoothAddress address;
    m_pbapInterface->getConnectedDevice(address);
    if (address.isValid()) {
        PRINTF_TO_CONSOLE(TAG, "disconnect address: %s", address.toString().c_str());
        ret = m_pbapInterface->disconnect(address);
        EXPECT_EQ(BTPTS_OK, ret);
        EXPECT_EQ(true, m_pbapCallback->waitCommandDone(PBAP_IND_DEVICE_DISCONNECTED, WAIT_TIME));
    }

    return ret;
}

int BtPtsPbapHandler::download()
{
    PRINTF_TO_CONSOLE(TAG, __FUNCTION__);
    string pathStr = m_args.getStringExtra("str1", "");
    int ret = BTPTS_ERROR;
    E_PBType path = (E_PBType)PBMGR_PHONEBOOK;
    if ("pb" == pathStr) {
        path = (E_PBType)PBMGR_PHONEBOOK;
    } else if ("calllog" == pathStr) {
        path = (E_PBType)(PBMGR_INCOMING_CALLS_HISTORY | PBMGR_OUTGOING_CALLS_HISTORY | PBMGR_MISSED_CALLS_HISTORY);
    } else {
        PRINTF_TO_WARN(TAG, "The download path is expected to be either 'pb' or 'calllog' ");
        return ret;
    }
    ret = m_pbapInterface->download(path, 0, 100);
    EXPECT_EQ(BTPTS_OK, ret);
    EXPECT_EQ(true, m_pbapCallback->waitCommandDone(PBAP_IND_DOWNLOAD_START, WAIT_TIME));
    EXPECT_EQ(true, m_pbapCallback->waitCommandDone(PBAP_IND_DOWNLOAD_ONESTEP, WAIT_TIME));
    EXPECT_EQ(true, m_pbapCallback->waitCommandDone(PBAP_IND_DOWNLOAD_FINISH, WAIT_MAX_DISC_TIME));

    int pbCount = 0;
    ret = m_pbapInterface->getRecordCount(path, pbCount);

    std::list<PBRecord> pbRecordList;
    m_pbapInterface->getRecord(path, 0, pbCount, pbRecordList);

    list<PBRecord>::iterator iterator;
    for (iterator = pbRecordList.begin(); iterator != pbRecordList.end(); iterator++) {
        PBRecord newPBRecord = *iterator;

        string firstName = newPBRecord.getFirstName();
        string middleName = newPBRecord.getMiddleName();
        string givenName = newPBRecord.getGivenName();
        string formattedName = newPBRecord.getFormattedName();

        std::list<PBRecord::PBTel> phoneNumberList;
        newPBRecord.getTelList(phoneNumberList);

        if (!phoneNumberList.empty()) {
            PBRecord::PBTel firstTel = phoneNumberList.front();
            if ("pb" == pathStr) {
                PRINTF_TO_CONSOLE(TAG, "show phone book: firstName(%s) middleName(%s) givenName(%s) formattedName(%s) telnum(%s)",
                    firstName.c_str(), middleName.c_str(), givenName.c_str(), formattedName.c_str(), firstTel.m_telNumber.c_str());
            }
        }

        if ("calllog" == pathStr) {
            PRINTF_TO_CONSOLE(TAG, "show call log: firstName(%s) middleName(%s) givenName(%s) formattedName(%s) telnum(%s) calltype(%s)",
                    firstName.c_str(), middleName.c_str(), givenName.c_str(), formattedName.c_str(), newPBRecord.getNumber().c_str(), newPBRecord.getCallType().c_str());
        }
    }

    return ret;
}

int BtPtsPbapHandler::stopdownload()
{
    PRINTF_TO_CONSOLE(TAG, __FUNCTION__);

    int ret = BTPTS_ERROR;
    ret =  m_pbapInterface->stopDownload();
    EXPECT_EQ(BTPTS_OK, ret);
    EXPECT_EQ(true, m_pbapCallback->waitCommandDone(PBAP_IND_DOWNLOAD_STOP, WAIT_TIME));

    return ret;
}

template<> BtPtsPBAPCallBack* Singleton<BtPtsPBAPCallBack>::msSingleton = NULL;
int BtPtsPBAPCallBack::onIndication(const CMessage &message)
{
    switch (message.what) {

        case PBAP_IND_DOWNLOAD_ONESTEP:
            PRINTF_TO_CONSOLE(TAG, "indication download onestep");
        break;

        case PBAP_IND_DOWNLOAD_FINISH: {
            int pbPathType = message.getIntExtra(INT_PBAP_DOWNLOAD_PATH, 0);
            PRINTF_TO_CONSOLE(TAG, "indication download finish, pbPathType is %d", pbPathType);
        }
        break;

        case PBAP_IND_DOWNLOAD_STOP: {
            int pbPathType = message.getIntExtra(INT_PBAP_DOWNLOAD_PATH, 0);
            PRINTF_TO_CONSOLE(TAG, "indication download stop, pbPathType is %d", pbPathType);
        }
        break;

        case PBAP_IND_DOWNLOAD_ERROR: {
            int pbPathType = message.getIntExtra(INT_PBAP_DOWNLOAD_PATH, 0);
            PRINTF_TO_CONSOLE(TAG, "indication download error, pbPathType is %d", pbPathType);
        }
        break;

        case PBAP_IND_DOWNLOAD_START: {
            int pbPathType = message.getIntExtra(INT_PBAP_DOWNLOAD_PATH, 0);
            PRINTF_TO_CONSOLE(TAG, "indication download start, pbPathType is %d", pbPathType);
        }
        break;

        case  PBAP_IND_DEVICE_DISCONNECTED: {
            int result = message.arg1;
            PRINTF_TO_CONSOLE(TAG, "indication pbap disconnected, result is %d", result);
        }
        break;

        case PBAP_IND_DEVICE_CONNECTED: {
            int result = message.arg1;
            PRINTF_TO_CONSOLE(TAG, "indication pbap connected, result is %d", result);
        }
        break;
    }
    checkIndication(message.what);
    return 0;
}


