#include "gtest/gtest.h"
#include "cconditionlock.h"

/*hid function header file*/
#include "bluetoothhid.h"
#include "bluetoothapi.h"

IBluetoothClient *g_client = NULL;
IBluetoothHid *g_hid_interface = NULL;


/****************************************************
HID Callback function
connect/disconnect notify
********************************************************/
class TestHidCallBack : public IBluetoothCallBack
{
public:

    TestHidCallBack()
    {
        m_conditionId = m_condition.newCondition();
    }

    virtual ~TestHidCallBack()
    {

    }

    virtual int onIndication(const CMessage &message)
    {
        m_condition.lock();
        printf("Hid Test indication %d \n", message.what);

        switch (message.what) {
            case HID_IND_DEVICE_CONNECTED:{
                printf("HidTest ind connected");
            }
            break;

            case HID_IND_DEVICE_DISCONNECTED:{
                printf("HidTest ind disconnect");
            }
            break;

            default: {
            }
            break;
        }

        if (m_expectedIndication == message.what) {
            m_condition.signal(m_conditionId);
        }
        m_condition.unlock();

        return 0;
    }

    bool waitCommandDone(int expectedIndication, int timeout)
    {
        bool ret_ok;

        m_condition.lock();
        m_expectedIndication = expectedIndication;
        ret_ok = m_condition.await(m_conditionId, timeout);
        m_condition.unlock();

        return ret_ok;
    }

private:
    CConditionLock m_condition;
    int m_conditionId;

    int m_expectedIndication;
};


/*********************************************************************************
Get HID profile interface
to call hid send data function
*********************************************************************************/

TestHidCallBack *g_testHidCallBack = new TestHidCallBack;

TEST(TC_HID_PROFILE_FUNCTION, TEST_GET_HID_INTERFACE)
{
    int ret = 0;

    g_client = getBluetoothClient();

    IBluetoothProfile *profile = NULL;
    ret = g_client->getProfile(HIDPROFILENAME, &profile);
    printf("GET_HID_INTERFACE ret %d\n", ret);
    EXPECT_LE(0, ret);

    g_hid_interface = dynamic_cast<IBluetoothHid*>(profile);
    EXPECT_NE((IBluetoothProfile*)NULL, g_hid_interface);

    printf("GET_HID_INTERFACE registerCallback bf\n");
    ret = g_hid_interface->registerCallBack(*g_testHidCallBack);
    EXPECT_LE(0, ret);
}

/*********************************************************************************
Get HID current state
to check if the hid profile has been connected
*********************************************************************************/

TEST(TC_HID_PROFILE_FUNCTION, TEST_HID_GET_STATE)
{
    int ret = 0;

    printf("TEST GET STATE \n");
    E_BLUETOOTH_PROFILE_STATE curState = BLUETOOTH_PROFILE_IDLE;
    ret = g_hid_interface->getState(curState);
    printf("current state is %d\n", curState);
    if (curState == BLUETOOTH_PROFILE_CONNECTED) 
        printf("bluetooth hid state is connected\n");
    else 
        printf("bluetooth hid state is disconnected\n");
        
    EXPECT_LE(0, ret);
    sleep(1);
}

/*********************************************************************************
HID Send control data
*********************************************************************************/
TEST(TC_HID_PROFILE_FUNCTION, TEST_SEND_CONTROL_DATA)
{
    bool ret = false;
    ret = g_hid_interface->sendControlData(HID_CMD_HOME);
    EXPECT_LE(0, ret);
    sleep(1);

    ret = g_hid_interface->sendControlData(HID_CMD_VOLUME_DOWN);
    EXPECT_LE(0, ret);
    sleep(1);

    ret = g_hid_interface->sendControlData(HID_CMD_PREVIOS);
    EXPECT_LE(0, ret);
    sleep(1);

    ret = g_hid_interface->sendControlData(HID_CMD_VOLUME_UP);
    EXPECT_LE(0, ret);
    sleep(1);

    ret = g_hid_interface->sendControlData(HID_CMD_PLAY_PAUSE);
    EXPECT_LE(0, ret);
    sleep(2);

    ret = g_hid_interface->sendControlData(HID_CMD_PLAY_PAUSE);
    EXPECT_LE(0, ret);
    sleep(2);

    ret = g_hid_interface->sendControlData(HID_CMD_STOP);
    EXPECT_LE(0, ret);
    sleep(5);
}

/*********************************************************************************
HID Send keyboard data
*********************************************************************************/
TEST(TC_HID_PROFILE_FUNCTION, TEST_SEND_KEYBOARD_DATA)
{
    bool ret = false;
    ret = g_hid_interface->sendKeyboardData(65);
    EXPECT_LE(0, ret);
    sleep(2);

    ret = g_hid_interface->sendKeyboardData(68);
    EXPECT_LE(0, ret);
    sleep(2);

    ret = g_hid_interface->sendKeyboardData(30);
    EXPECT_LE(0, ret);
    sleep(5);
}

/*********************************************************************************
HID Send mouse data
*********************************************************************************/
TEST(TC_HID_PROFILE_FUNCTION, TEST_SEND_MOUSE_DATA)
{
    bool ret = false;

    ret = g_hid_interface->sendMouseData(false, -1024, -1024);
    EXPECT_LE(0, ret);
    usleep(100 * 1000);

    ret = g_hid_interface->sendMouseData(false, -1024, -1024);
    EXPECT_LE(0, ret);
    usleep(100 * 1000);

    ret = g_hid_interface->sendMouseData(false, 0, 0);
    EXPECT_LE(0, ret);
    usleep(100 * 1000);

    ret = g_hid_interface->sendMouseData(false, 0, 0);
    EXPECT_LE(0, ret);
    sleep(1);

    ret = g_hid_interface->sendMouseData(false, 0, 300);
    EXPECT_LE(0, ret);
    usleep(100 * 1000);

    ret = g_hid_interface->sendMouseData(false, 0, 0);
    EXPECT_LE(0, ret);
    sleep(1);

    ret = g_hid_interface->sendMouseData(false, 300, -300);
    EXPECT_LE(0, ret);
    usleep(100 * 1000);

    ret = g_hid_interface->sendMouseData(false, 0, 0);
    EXPECT_LE(0, ret);
    sleep(1);

    ret = g_hid_interface->sendMouseData(false, 0, 300);
    EXPECT_LE(0, ret);
    usleep(100 * 1000);

    ret = g_hid_interface->sendMouseData(false, 0, 0);
    EXPECT_LE(0, ret);
    sleep(2);

    ret = g_hid_interface->sendMouseData(false, -1024, -1024);
    EXPECT_LE(0, ret);
    usleep(100 * 1000);

    ret = g_hid_interface->sendMouseData(false, -1024, -1024);
    EXPECT_LE(0, ret);
    usleep(100 * 1000);

    ret = g_hid_interface->sendMouseData(false, 0, 0);
    EXPECT_LE(0, ret);
    sleep(5);
}

/*********************************************************************************/

int main(int argc , char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	(void)RUN_ALL_TESTS();
}

