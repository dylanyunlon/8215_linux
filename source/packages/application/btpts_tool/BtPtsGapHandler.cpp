#include "BtPtsGapHandler.h"
using namespace universal_utils;

static const char* TAG = "BtPtsGapHandler";

const map<string, HANDLE_FUN> BtPtsGapHandler::GAP_HANDLE_MAP = {
    {CMD_GAP_OPEN,             static_cast<HANDLE_FUN>(&BtPtsGapHandler::open)},
    {CMD_GAP_CLOSE,            static_cast<HANDLE_FUN>(&BtPtsGapHandler::close)},
    {CMD_GAP_DISCOVERY,        static_cast<HANDLE_FUN>(&BtPtsGapHandler::discovery)},
    {CMD_GAP_CANCEL_DISCOVERY, static_cast<HANDLE_FUN>(&BtPtsGapHandler::cancelDiscovery)},
    {CMD_GAP_BOND,             static_cast<HANDLE_FUN>(&BtPtsGapHandler::bond)},
    {CMD_GAP_UNBOND,           static_cast<HANDLE_FUN>(&BtPtsGapHandler::unbond)},
    {CMD_GAP_SCANMODE,         static_cast<HANDLE_FUN>(&BtPtsGapHandler::scanMode)},
    {CMD_GAP_SETNAME,          static_cast<HANDLE_FUN>(&BtPtsGapHandler::setName)}
};

BtPtsGapHandler::BtPtsGapHandler(IBluetoothClient *client)
    : BtPtsHandler(PROFILE_GAP)
    , m_client(NULL)
    , m_localDevice(NULL)
    , m_remoteDevice(NULL)
    , m_gapCallback(NULL)
{
    m_client = client;
    client->getLocalDevice(&m_localDevice);
    EXPECT_NOT_NULL(m_localDevice, "m_localDevice");
    registerCallBack();
}

BtPtsGapHandler::~BtPtsGapHandler()
{
    deregisterCallBack();
    SAFE_DELETE(m_remoteDevice);
}
const map<string, HANDLE_FUN>& BtPtsGapHandler::getHandleFunMap()
{
    return GAP_HANDLE_MAP;
}

void BtPtsGapHandler::registerCallBack()
{
    m_gapCallback = new BtPtsGAPCallBack();
    m_client->registerCallBack(*m_gapCallback);
}
void BtPtsGapHandler::deregisterCallBack()
{
    m_client->deregisterCallBack(*m_gapCallback);
    SAFE_DELETE(m_gapCallback);
}

int BtPtsGapHandler::open()
{
    PRINTF_TO_CONSOLE(TAG, __FUNCTION__);

    int ret = BTPTS_ERROR;
    E_LOCAL_DEVICE_STATE state = GAP_STATE_INVALID;
    m_localDevice->getBluetoothState(state);

    if (GAP_STATE_POWEROFF == state) {
        ret = m_localDevice->open();
        EXPECT_EQ(BTPTS_OK, ret);
        EXPECT_EQ(true, m_gapCallback->waitCommandDone(GAP_POWERON_IND, WAIT_MAX_TIME));
        m_localDevice->getBluetoothState(state);
        EXPECT_EQ(GAP_STATE_POWERON, state);

    } else if ((GAP_STATE_POWERON == state) || (GAP_STATE_DISCOVERYING == state)) {
        PRINTF_TO_WARN(TAG, "m_localDevice is alreadly opened");
    }

    return ret;
}

int BtPtsGapHandler::close()
{
    PRINTF_TO_CONSOLE(TAG, __FUNCTION__);

    int ret = BTPTS_ERROR;
    E_LOCAL_DEVICE_STATE state = GAP_STATE_INVALID;
    m_localDevice->getBluetoothState(state);

    if (GAP_STATE_POWEROFF == state) {
        PRINTF_TO_WARN(TAG, "m_localDevice is alreadly closed");
    } else if ((GAP_STATE_POWERON == state) || (GAP_STATE_DISCOVERYING == state)) {
        ret = m_localDevice->close();
        EXPECT_EQ(BTPTS_OK, ret);
        EXPECT_EQ(true, m_gapCallback->waitCommandDone(GAP_POWEROFF_IND, WAIT_MAX_TIME));
        m_localDevice->getBluetoothState(state);
        EXPECT_EQ(GAP_STATE_POWEROFF, state);
    }

    return ret;
}

int BtPtsGapHandler::discovery()
{
    PRINTF_TO_CONSOLE(TAG, __FUNCTION__);

    int ret = BTPTS_ERROR;
    E_LOCAL_DEVICE_STATE state = GAP_STATE_INVALID;
    m_localDevice->getBluetoothState(state);
    if (GAP_STATE_DISCOVERYING == state) {
        PRINTF_TO_WARN(TAG, "m_localDevice is alreadly discovering");
    } else {
        ret = m_localDevice->discovery(0);
        EXPECT_EQ(BTPTS_OK, ret);
        EXPECT_EQ(true, m_gapCallback->waitCommandDone(GAP_DISCOVERY_STOP_IND, WAIT_MAX_DISC_TIME));
    }

    return ret;
}

int BtPtsGapHandler::cancelDiscovery()
{
    PRINTF_TO_CONSOLE(TAG, __FUNCTION__);

    int ret = BTPTS_ERROR;
    E_LOCAL_DEVICE_STATE state = GAP_STATE_INVALID;
    m_localDevice->getBluetoothState(state);
    if (GAP_STATE_DISCOVERYING == state) {
        ret = m_localDevice->cancelDiscovery();
        EXPECT_EQ(BTPTS_OK, ret);
        EXPECT_EQ(true, m_gapCallback->waitCommandDone(GAP_DISCOVERY_STOP_IND, WAIT_MAX_TIME));

    } else {
        PRINTF_TO_WARN(TAG, "m_remoteDevice is undiscovery state");
    }

    return ret;
}

int BtPtsGapHandler::bond()
{
    int ret = BTPTS_ERROR;

    string address = m_args.getStringExtra("str1", "");
    if (address != "") m_address = address;

    if (m_address.isValid()) {
        PRINTF_TO_CONSOLE(TAG, "bond address: %s", m_address.toString().c_str());
        SAFE_DELETE(m_remoteDevice);
        m_remoteDevice = new BluetoothRemoteDevice(m_address.toString());
        E_GAP_PAIR_STATE state = BONDSTATE_UNBOND;
        m_remoteDevice->getPairState(state);
        if (state == BONDSTATE_UNBOND) {
            ret = m_remoteDevice->bond();
            EXPECT_EQ(BTPTS_OK, ret);
            EXPECT_EQ(true, m_gapCallback->waitCommandDone(GAP_SECURITY_USER_CONFIRM_IND, WAIT_MAX_TIME));
            ret = m_remoteDevice->secureUserConfirm(true);
            EXPECT_EQ(BTPTS_OK, ret);
            EXPECT_EQ(true, m_gapCallback->waitCommandDone(GAP_BONDING_RESULT_IND, WAIT_MAX_TIME));
            m_remoteDevice->getPairState(state);
            EXPECT_EQ(BONDSTATE_BONDED, state);
        } else {
            PRINTF_TO_WARN(TAG, "m_remoteDevice is alreadly bond");
        }

    } else {
        PRINTF_TO_WARN(TAG, "m_remoteDevice bond address is invalid: %s", m_address.toString().c_str());
    }

    return  ret;
}

int BtPtsGapHandler::unbond()
{
    int ret = BTPTS_ERROR;

    string address = m_args.getStringExtra("str1", "");
    if (address != "") m_address = address;

    if (m_address.isValid()) {
        PRINTF_TO_CONSOLE(TAG, "unbond address: %s", m_address.toString().c_str());
        SAFE_DELETE(m_remoteDevice);
        m_remoteDevice = new BluetoothRemoteDevice(m_address.toString());
        E_GAP_PAIR_STATE state = BONDSTATE_UNBOND;
        m_remoteDevice->getPairState(state);

        if (state == BONDSTATE_BONDED) {
            ret = m_remoteDevice->removeBond();
            EXPECT_EQ(BTPTS_OK, ret);
            EXPECT_EQ(true, m_gapCallback->waitCommandDone(GAP_BOND_REMOVED_IND, WAIT_MIN_TIME));
            m_remoteDevice->getPairState(state);
            EXPECT_EQ(BONDSTATE_UNBOND, state);
        } else {
            PRINTF_TO_WARN(TAG, "m_remoteDevice is alreadly unbond");
        }
    } else {
        PRINTF_TO_WARN(TAG, "m_remoteDevice is alreadly bond");
    }

    return BTPTS_OK;
}

int BtPtsGapHandler::scanMode()
{
    int ret = BTPTS_ERROR;

    int mode = m_args.getIntExtra("int1", 0);
    PRINTF_TO_CONSOLE(TAG, "%s %d", __FUNCTION__, mode);
    if (mode >= SCAN_MODE_PAGE_OFF_INQUIRY_OFF && mode <= SCAN_MODE_PAGE_ON_INQUIRY_ON) {
        ret = m_localDevice->setScanMode((E_GAP_SCAN_MODE)mode);
    }
    EXPECT_EQ(BTPTS_OK, ret);

    return BTPTS_OK;
}

int BtPtsGapHandler::setName()
{
    PRINTF_TO_CONSOLE(TAG, __FUNCTION__);

    int ret = BTPTS_ERROR;

    string name = m_args.getStringExtra("str1", "");
    PRINTF_TO_CONSOLE(TAG, "%s %s", __FUNCTION__, name.c_str());
    ret = m_localDevice->setName(name);

    EXPECT_EQ(BTPTS_OK, ret);
    EXPECT_EQ(true, m_gapCallback->waitCommandDone(GAP_LOCAL_NAME_IND, WAIT_MIN_TIME));

    EXPECT_EQ(name == m_localDevice->getName(), true);

    return BTPTS_OK;
}

bool BtPtsGapHandler::dump(string &dumpInfo)
{
    string tempStr = "";
    E_GAP_SCAN_MODE scanMode;
    E_LOCAL_DEVICE_STATE localState;

    dumpInfo.append("local address:" + m_localDevice->getAddress().toString() + "\n");
    dumpInfo.append("local name:" + m_localDevice->getName() + "\n");
    m_localDevice->getBluetoothState(localState);
    dumpInfo.append("local state:" + localStateToString(localState) + "\n");
    m_localDevice->getPinCode(tempStr);
    dumpInfo.append("pin code:" + tempStr + "\n");
    m_localDevice->getScanMode(scanMode);
    dumpInfo.append("scan mode: " + scanModeToString(scanMode) + "\n");

    return true;
}

string BtPtsGapHandler::localStateToString(E_LOCAL_DEVICE_STATE state)
{
    string stateStr = "";
    switch(state) {
    case GAP_STATE_POWEROFF:
        stateStr = "STATE_POWEROFF";
        break;

    case GAP_STATE_POWERSWTICHING:
        stateStr = "STATE_POWERSWTICHING";
        break;

    case GAP_STATE_POWERON:
        stateStr = "STATE_POWERON";
        break;

    case GAP_STATE_DISCOVERYING:
        stateStr = "STATE_DISCOVERYING";
        break;

    default:
        stateStr = "STATE_INVALID";
    }

    return stateStr;
}

string BtPtsGapHandler::scanModeToString(E_GAP_SCAN_MODE mode)
{
    string modeStr = "";
    switch(mode) {
    case SCAN_MODE_PAGE_OFF_INQUIRY_OFF:
        modeStr = "SCAN_MODE_PAGE_OFF_INQUIRY_OFF";
        break;

    case SCAN_MODE_PAGE_OFF_INQUIRY_ON:
        modeStr = "SCAN_MODE_PAGE_OFF_INQUIRY_ON";
        break;

    case SCAN_MODE_PAGE_ON_INQUIRY_OFF:
        modeStr = "SCAN_MODE_PAGE_ON_INQUIRY_OFF";
        break;

    case SCAN_MODE_PAGE_ON_INQUIRY_ON:
        modeStr = "SCAN_MODE_PAGE_ON_INQUIRY_ON";
        break;

    case SCAN_MODE_PAGE_ON_INQUIRY_ON_LOW:
        modeStr = "SCAN_MODE_PAGE_ON_INQUIRY_ON_LOW";
        break;

    default:
        modeStr = "SCAN_MODE_INVALID";
    }

    return modeStr;
}

template<> BtPtsGAPCallBack* Singleton<BtPtsGAPCallBack>::msSingleton = NULL;
int BtPtsGAPCallBack::onIndication(const CMessage &message)
{
    switch (message.what) {
        case GAP_POWER_STATE_CHANGE_IND:
            break;

        case GAP_POWERON_IND: {
            PRINTF_TO_CONSOLE(TAG, "indication power on");
        }
        break;

        case GAP_POWEROFF_IND: {
            PRINTF_TO_CONSOLE(TAG, "indication power off");
        }
        break;

        case GAP_LOCAL_NAME_IND: {

            string name = message.getStringExtra(STRING_NAME, "");
            PRINTF_TO_CONSOLE(TAG, "indication local name is %s", name.c_str());
        }
        break;

        case GAP_DISCOVERY_START_IND:
            PRINTF_TO_CONSOLE(TAG, "indication discovery start");
            break;

        case GAP_DISCOVERY_STOP_IND:
            PRINTF_TO_CONSOLE(TAG, "indication discovery stop");
            break;

        case GAP_DISCOVERY_RESULT_IND: {
            string address = message.getStringExtra(STRING_ADDRESS, "");
            string name = message.getStringExtra(STRING_NAME, "");
            int cod = message.getIntExtra(INT_COD, 0);
            int rssi = message.getIntExtra(INT_RSSI, 0);

            PRINTF_TO_CONSOLE(TAG, "indication dicovery result: address is %s, name is %s, cod is %d, rssi is %d", address.c_str(), name.c_str(), cod, rssi);
        }
        break;

        case GAP_DISCOVERY_UPDATE_IND: {
            string address = message.getStringExtra(STRING_ADDRESS, "");
            string name = message.getStringExtra(STRING_NAME, "");
            PRINTF_TO_CONSOLE(TAG, "indication discovery update address is %s, name is %s", address.c_str(), name.c_str());
        }
        break;

        case GAP_SECURITY_USER_CONFIRM_IND: {
            string address = message.getStringExtra(STRING_ADDRESS, "");
            PRINTF_TO_CONSOLE(TAG, "indication confirm address is %s", address.c_str());
        }
        break;

        case GAP_BONDING_RESULT_IND: {

            BluetoothAddress bt_address = message.getStringExtra(STRING_ADDRESS, "");
            string address = bt_address.toString();
            string name = message.getStringExtra(STRING_NAME, "");
            int result = message.arg1;
            PRINTF_TO_CONSOLE(TAG, "indication bond address is %s, name is %s, result is %d", address.c_str(), name.c_str(), result);
        }
        break;

        case GAP_BOND_REMOVED_IND: {

            BluetoothAddress bt_address = message.getStringExtra(STRING_ADDRESS, "");
            string address = bt_address.toString();
            string name = message.getStringExtra(STRING_NAME, "");
            PRINTF_TO_CONSOLE(TAG, "indication remove bond address is %s, name is %s", address.c_str(), name.c_str());
        }
        break;

        case GAP_PIN_CODE_IND:{

            BluetoothAddress bt_address = message.getStringExtra(STRING_ADDRESS, "");
            string address = bt_address.toString();
            PRINTF_TO_CONSOLE(TAG, "indication pin code address is %s", address.c_str());
        }
        break;

        case GAP_LINK_STATE_IND:{

            string address = message.getStringExtra(STRING_ADDRESS, "");
            int currentNumber = message.getIntExtra(INT_CURRENT_NUMBER, 0);
            int errorCode = message.getIntExtra(INT_ERROR_CODE, 0);
            PRINTF_TO_CONSOLE(TAG, "indication address is %s, currentNumber is %d, errorCode is 0x%x", address.c_str(), currentNumber, errorCode);

        }
        break;

    }

    checkIndication(message.what);

    return 0;
}

