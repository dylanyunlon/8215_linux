#include "BtPtsHidHandler.h"
using namespace universal_utils;

static const char* TAG = "BtPtsHidHandler";
const map<string, HANDLE_FUN> BtPtsHidHandler::HID_HANDLE_MAP = {

    {CMD_PROFILE_CONNECT,      static_cast<HANDLE_FUN>(&BtPtsHidHandler::connect)},
    {CMD_PROFILE_DISCONNECT,   static_cast<HANDLE_FUN>(&BtPtsHidHandler::disconnect)},
    {CMD_HID_SENDMOUSEDATA,    static_cast<HANDLE_FUN>(&BtPtsHidHandler::sendMouseData)},
    {CMD_HID_SENDKEYBOARDDATA, static_cast<HANDLE_FUN>(&BtPtsHidHandler::sendKeyboardData)},
    {CMD_HID_SENDCONTROLDATA,  static_cast<HANDLE_FUN>(&BtPtsHidHandler::sendControlData)},
};

BtPtsHidHandler::BtPtsHidHandler(IBluetoothClient *client)
    : BtPtsHandler(PROFILE_HID)
    , m_hidInterface(NULL)
    , m_hidCallback(NULL)
{
    IBluetoothProfile *profile = NULL;
    client->getProfile(HIDPROFILENAME, &profile);
    m_hidInterface = dynamic_cast<IBluetoothHid*>(profile);
    EXPECT_NOT_NULL(m_hidInterface, "m_hidInterface");
    registerCallBack();
}

BtPtsHidHandler::~BtPtsHidHandler()
{
    deregisterCallBack();
}

const map<string, HANDLE_FUN>& BtPtsHidHandler::getHandleFunMap()
{
    return HID_HANDLE_MAP;
}

void BtPtsHidHandler::registerCallBack()
{
    m_hidCallback = new BtPtsHidCallBack();
    m_hidInterface->registerCallBack(*m_hidCallback);
}

void BtPtsHidHandler::deregisterCallBack()
{
    m_hidInterface->deregisterCallBack(*m_hidCallback);
    SAFE_DELETE(m_hidCallback);
}

int BtPtsHidHandler::connect()
{
    int ret = BTPTS_ERROR;

    string address = m_args.getStringExtra("str1", "");
    if (address != "")
        m_address = address;
    E_BLUETOOTH_PROFILE_STATE state = BLUETOOTH_PROFILE_IDLE;
    m_hidInterface->getState(state);
    if (state == BLUETOOTH_PROFILE_IDLE || state == BLUETOOTH_PROFILE_DISCONNECTING) {
        PRINTF_TO_CONSOLE(TAG, "connect address: %s", m_address.toString().c_str());
        ret = m_hidInterface->connect(m_address);
        EXPECT_EQ(BTPTS_OK, ret);
        EXPECT_EQ(true, m_hidCallback->waitCommandDone(HID_IND_DEVICE_CONNECTED, WAIT_MAX_TIME));
    }

    return ret;
}

int BtPtsHidHandler::disconnect()
{
    int ret = BTPTS_ERROR;

    BluetoothAddress address;
    m_hidInterface->getConnectedDevice(address);
    if (address.isValid()) {
        PRINTF_TO_CONSOLE(TAG, "disconnect address: %s", m_address.toString().c_str());
        ret = m_hidInterface->disconnect(address);
        EXPECT_EQ(BTPTS_OK, ret);
        EXPECT_EQ(true, m_hidCallback->waitCommandDone(HID_IND_DEVICE_DISCONNECTED, WAIT_MAX_TIME));
    }

    return ret;
}

int BtPtsHidHandler::sendMouseData()
{
    int ret = BTPTS_ERROR;

    bool buttonDown = (bool)m_args.getIntExtra("int1", 0);
    int relativeX = m_args.getIntExtra("int2", 0);
    int relativeY = m_args.getIntExtra("int3", 0);
    PRINTF_TO_CONSOLE(TAG, "%s %d %d %d",  __FUNCTION__, buttonDown, relativeX, relativeY);
    ret = m_hidInterface->sendMouseData(buttonDown, relativeX, relativeY);
    EXPECT_EQ(BTPTS_OK, ret);

    return ret;
}

int BtPtsHidHandler::sendKeyboardData()
{
    int ret = BTPTS_ERROR;

    char keyCode = m_args.getCharExtra("char1", 0);
    PRINTF_TO_CONSOLE(TAG, "%s %c", __FUNCTION__, keyCode);
    ret = m_hidInterface->sendKeyboardData(keyCode);
    EXPECT_EQ(BTPTS_OK, ret);

    return ret;
}

int BtPtsHidHandler::sendControlData()
{
    int ret = BTPTS_ERROR;

    int cmdCode = m_args.getIntExtra("int1", 0);
    PRINTF_TO_CONSOLE(TAG, "%s %d", __FUNCTION__, cmdCode);
    ret = m_hidInterface->sendControlData(cmdCode);
    EXPECT_EQ(BTPTS_OK, ret);

    return ret;
}

template<> BtPtsHidCallBack* Singleton<BtPtsHidCallBack>::msSingleton = NULL;
int BtPtsHidCallBack::onIndication(const CMessage &message)
{

    switch (message.what) {
        case HID_IND_DEVICE_CONNECTED: {
            string address = message.getStringExtra(STRING_ADDRESS, " ");
            PRINTF_TO_CONSOLE(TAG, "indication connected: %s ", address.c_str());
        }
        break;

        case HID_IND_DEVICE_DISCONNECTED:{
            string address = message.getStringExtra(STRING_ADDRESS, " ");
            PRINTF_TO_CONSOLE(TAG, "indication disconnect: %s ", address.c_str());
        }
        break;
    }

    checkIndication(message.what);
    return 0;
}


