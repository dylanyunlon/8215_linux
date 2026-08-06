#include "BtPtsHfpHandler.h"
#include <vector>
#include "clog.h"

using namespace universal_utils;

static const char* TAG = "BtPtsHfpHandler";
const map<string, HANDLE_FUN> BtPtsHfpHandler::HFP_HANDLE_MAP = {

    {CMD_PROFILE_CONNECT,     static_cast<HANDLE_FUN>(&BtPtsHfpHandler::connect)},
    {CMD_PROFILE_DISCONNECT,  static_cast<HANDLE_FUN>(&BtPtsHfpHandler::disconnect)},
    {CMD_HFP_DIAL,            static_cast<HANDLE_FUN>(&BtPtsHfpHandler::dial)},
    {CMD_HFP_ACCEPT,          static_cast<HANDLE_FUN>(&BtPtsHfpHandler::accept)},
    {CMD_HFP_TERMINATE,       static_cast<HANDLE_FUN>(&BtPtsHfpHandler::terminate)},
    {CMD_HFP_HOLD,            static_cast<HANDLE_FUN>(&BtPtsHfpHandler::hold)},
    {CMD_HFP_DTMF,            static_cast<HANDLE_FUN>(&BtPtsHfpHandler::dtmf)},
    {CMD_HFP_SWITCH,          static_cast<HANDLE_FUN>(&BtPtsHfpHandler::switchAudio)},
    {CMD_HFP_REDIAL,          static_cast<HANDLE_FUN>(&BtPtsHfpHandler::redial)},
    {CMD_HFP_SPEAKERGAIN,     static_cast<HANDLE_FUN>(&BtPtsHfpHandler::speakerGain)},
    {CMD_HFP_MICGAIN,         static_cast<HANDLE_FUN>(&BtPtsHfpHandler::micGain)}
};

BtPtsHfpHandler::BtPtsHfpHandler(IBluetoothClient *client)
    : BtPtsHandler(PROFILE_HFP)
    , m_hfpInterface(NULL)
    , m_hfpCallback(NULL)
{
    IBluetoothProfile *profile = NULL;
    client->getProfile(HANDSFREEPROFILENAME, &profile);
    m_hfpInterface = dynamic_cast<IBluetoothHandsfree*>(profile);
    EXPECT_NOT_NULL(m_hfpInterface, "m_hfpInterface");
    registerCallBack();
}

BtPtsHfpHandler::~BtPtsHfpHandler()
{
    deregisterCallBack();
}

bool BtPtsHfpHandler::isAutoConnected()
{
    return true;
}

const map<string, HANDLE_FUN>& BtPtsHfpHandler::getHandleFunMap()
{
    return HFP_HANDLE_MAP;
}

void BtPtsHfpHandler::registerCallBack()
{
    m_hfpCallback = new BtPtsHFPCallBack();
    m_hfpInterface->registerCallBack(*m_hfpCallback);
}

void BtPtsHfpHandler::deregisterCallBack()
{
    m_hfpInterface->deregisterCallBack(*m_hfpCallback);
    SAFE_DELETE(m_hfpCallback);
}


void BtPtsHfpHandler::sleepSecondsForRetry(const string &retryCmd)
{
    int waitSeconds = m_args.getIntExtra("int3", 0);
    if (waitSeconds > 0) {
        UTILS_LOGI(TAG, "sleep %d s retry %s... ", waitSeconds, retryCmd.c_str());
        sleep(waitSeconds);
    }
}

int BtPtsHfpHandler::connect()
{
    int ret = BTPTS_ERROR;
    E_BLUETOOTH_PROFILE_STATE state = BLUETOOTH_PROFILE_IDLE;
    m_hfpInterface->getState(state);
    string address = m_args.getStringExtra("str1", "");
    if (address != "") m_address = address;
    int retryCnt = m_args.getIntExtra("int2", 0);
    bool isHfpConnected = false;

    do {
        if (state != BLUETOOTH_PROFILE_CONNECTED) {
            PRINTF_TO_CONSOLE(TAG, "connect address: %s, retryCnt: %d", m_address.toString().c_str(), retryCnt);
            ret = m_hfpInterface->connect(m_address);
            EXPECT_EQ(BTPTS_OK, ret);
            isHfpConnected = m_hfpCallback->waitCommandDone(HF_IND_DEVICE_CONNECTED, WAIT_MAX_TIME, BTPTS_OK);
        } else {
            PRINTF_TO_WARN(TAG, "hfp is already connected");
            break;
        }

        // check pass
        if (isHfpConnected) {
            UTILS_LOGI(TAG, "hfp connect address: %s done, retry time is %d", address.c_str(), retryCnt);
            break;
        }

        if (--retryCnt <= 0) {
            m_hfpInterface->getState(state);
            EXPECT_EQ(BLUETOOTH_PROFILE_CONNECTED, state);
        }

        sleepSecondsForRetry("hfp connect");

    } while (retryCnt > 0);

    return ret;
}

int BtPtsHfpHandler::disconnect()
{
    int ret = BTPTS_ERROR;

    BluetoothAddress address;
    m_hfpInterface->getConnectedDevice(address);
    if (address.isValid()) {
        PRINTF_TO_CONSOLE(TAG, "disconnect address: %s", address.toString().c_str());
        ret = m_hfpInterface->disconnect(address);
        EXPECT_EQ(BTPTS_OK, ret);
        EXPECT_EQ(true, m_hfpCallback->waitCommandDone(HF_IND_DEVICE_DISCONNECTED, WAIT_MAX_TIME));
    }

    return ret;
}

int BtPtsHfpHandler::dial()
{
    int ret = BTPTS_ERROR;
    string phoneNumber = m_args.getStringExtra("str1", "");
    PRINTF_TO_CONSOLE(TAG, "%s %s", __FUNCTION__, phoneNumber.c_str());
    vector<BluetoothHfClientCall> calls;
    m_hfpInterface->getCall(calls);
    for (BluetoothHfClientCall call : calls) {
        if (call.getNumber() == phoneNumber) {
            PRINTF_TO_WARN(TAG, "%s is already calling", phoneNumber.c_str());
            return ret;
        }
    }

    ret = m_hfpInterface->dialNumber(phoneNumber);
    EXPECT_EQ(BTPTS_OK, ret);
    EXPECT_EQ(true, m_hfpCallback->waitCommandDone(HF_IND_CALLSTATE, WAIT_TIME, HF_CALLSTATE_OUTGOING));

    return ret;
}

int BtPtsHfpHandler::accept()
{
    PRINTF_TO_CONSOLE(TAG, __FUNCTION__);

    int ret = BTPTS_ERROR;
    if (m_hfpInterface->getCallState() == HF_CALLSTATE_INCOMING || m_hfpInterface->getCallState() == HF_CALLSTATE_WAITING) {
        ret = m_hfpInterface->acceptIncommingCall();
        EXPECT_EQ(BTPTS_OK, ret);
        EXPECT_EQ(true, m_hfpCallback->waitCommandDone(HF_IND_CALLSTATE, WAIT_TIME));
    } else {
        PRINTF_TO_WARN(TAG, "no call need to accept");
    }

    return ret;
}

int BtPtsHfpHandler::terminate()
{
    PRINTF_TO_CONSOLE(TAG, __FUNCTION__);

    int ret = BTPTS_ERROR;
    vector<BluetoothHfClientCall> calls;
    m_hfpInterface->getCall(calls);
    if (calls.size() < 1) {
        PRINTF_TO_WARN(TAG, "no call need to termiante");
    } else {
        ret = m_hfpInterface->terminatePhoneCall();
        EXPECT_EQ(BTPTS_OK, ret);
        EXPECT_EQ(true, m_hfpCallback->waitCommandDone(HF_IND_CALLSTATE, WAIT_TIME));
    }

    return ret;
}

int BtPtsHfpHandler::hold()
{
    int ret = BTPTS_ERROR;

    int callAction = m_args.getIntExtra("int1", 0);
    PRINTF_TO_CONSOLE(TAG, "%s %d", __FUNCTION__, callAction);
    ret = m_hfpInterface->callHold((E_HF_CALL_ACTION)callAction);
    EXPECT_EQ(BTPTS_OK, ret);

    return ret;
}

int BtPtsHfpHandler::dtmf()
{
    int ret = BTPTS_ERROR;

    string dtmfCode = m_args.getStringExtra("str1", "");
    PRINTF_TO_CONSOLE(TAG, "%s %s", __FUNCTION__, dtmfCode.c_str());
    ret = m_hfpInterface->sendDTMFCode(dtmfCode);
    EXPECT_EQ(BTPTS_OK, ret);

    return ret;
}

int BtPtsHfpHandler::switchAudio()
{
    int ret = BTPTS_ERROR;

    E_HF_AUDIOTOWARDS toward = m_hfpInterface->getAudioTransferTowards();
    if (toward == HF_AUDIOTOWARDS_AG) {
        toward = HF_AUDIOTOWARDS_HF;
    } else {
        toward = HF_AUDIOTOWARDS_AG;
    }
    PRINTF_TO_CONSOLE(TAG, "%s %d", __FUNCTION__, toward);
    ret = m_hfpInterface->swithAudioTransferTowards(toward);
    EXPECT_EQ(BTPTS_OK, ret);
    EXPECT_EQ(true, m_hfpCallback->waitCommandDone(HF_IND_CALLAUDIOTRANSFER, WAIT_TIME, toward));
    EXPECT_EQ(toward, m_hfpInterface->getAudioTransferTowards());

    return ret;
}

int BtPtsHfpHandler::redial()
{
    PRINTF_TO_CONSOLE(TAG, __FUNCTION__);

    int ret = BTPTS_ERROR;
    ret = m_hfpInterface->redial();
    EXPECT_EQ(BTPTS_OK, ret);
    EXPECT_EQ(true, m_hfpCallback->waitCommandDone(HF_IND_CALLSTATE, WAIT_TIME, HF_CALLSTATE_OUTGOING));

    return ret;
}

int BtPtsHfpHandler::speakerGain()
{
    int ret = BTPTS_ERROR;

    int gain = m_args.getIntExtra("int1", 0);
    PRINTF_TO_CONSOLE(TAG, "%s %d", __FUNCTION__, gain);
    ret = m_hfpInterface->setPhoneSpeakerGain(gain);
    EXPECT_EQ(BTPTS_OK, ret);

    return ret;
}

int BtPtsHfpHandler::micGain()
{
    int ret = BTPTS_ERROR;

    int gain = m_args.getIntExtra("int1", 0);
    PRINTF_TO_CONSOLE(TAG, "micGain gain: %d\n", gain);
    ret =  m_hfpInterface->setPhoneMicGain(gain);
    EXPECT_EQ(BTPTS_OK, ret);

    return ret;
}

bool BtPtsHfpHandler::dump(string &dumpInfo)
{
    E_BLUETOOTH_PROFILE_STATE state = BLUETOOTH_PROFILE_IDLE;

    m_hfpInterface->getState(state);
    dumpInfo.append("hfp state: " + connectionStateToString(state) + "\n");
    if (state == BLUETOOTH_PROFILE_CONNECTED) {
        BluetoothAddress device;
        m_hfpInterface->getConnectedDevice(device);
        dumpInfo.append("hfp device: " + device.toString() + "\n");
    }
}

template<> BtPtsHFPCallBack* Singleton<BtPtsHFPCallBack>::msSingleton = NULL;
int BtPtsHFPCallBack::onIndication(const CMessage &message)
{
    int expectedValue = 0;

    switch (message.what) {
        case HF_IND_DEVICE_CONNECTED:{

            string address = message.getStringExtra(STRING_ADDRESS, " ");
            PRINTF_TO_CONSOLE(TAG, "indication connect address is %s, connectResult is %d", address.c_str(), message.arg1);
            expectedValue = message.arg1;
        }
        break;

        case HF_IND_DEVICE_DISCONNECTED:{

            string address = message.getStringExtra(STRING_ADDRESS, " ");
            PRINTF_TO_CONSOLE(TAG, "indication disconnect address is %s", address.c_str());
        }
        break;

        case HF_IND_DEVICE_CONNECTING:
        break;

        case HF_IND_CALLSTATE:{
            int callState = message.getIntExtra(INT_HF_CALLSTATE, 0);
            PRINTF_TO_CONSOLE(TAG, "indication callState:%d, IDLE(%d), incoming(%d), outgoing(%d), speaking(%d), waiting(%d), held(%d)",
                callState,
                HF_CALLSTATE_IDLE,
                HF_CALLSTATE_INCOMING,
                HF_CALLSTATE_OUTGOING,
                HF_CALLSTATE_SPEAKING,
                HF_CALLSTATE_WAITING,
                HF_CALLSTATE_HELD);
            if (callState == HF_CALLSTATE_INCOMING || callState == HF_CALLSTATE_OUTGOING)
                expectedValue = callState;
        }
        break;

        case HF_IND_CALLNUMBER:{
            string callNumber = message.getStringExtra(STRING_HF_CALLNUMBER, " ");
            PRINTF_TO_CONSOLE(TAG, "indication callNumber is %s", callNumber.c_str());

        }
        break;

        case HF_IND_CALLWAITINGNUMBER:{
            string callWaitingNumber = message.getStringExtra(STRING_HF_CALLWAITINGNUMBER, " ");
            PRINTF_TO_CONSOLE(TAG, "indication callWaitingNumber is %s", callWaitingNumber.c_str());
        }
        break;

        case HF_IND_CALLAUDIOTRANSFER:{

            int scoState = message.getIntExtra(STRING_HF_AUDIOTOWARDS, 0);
            PRINTF_TO_CONSOLE(TAG, "indication scoState is %d\n", scoState);
            expectedValue = scoState;
        }
        break;

        case HF_IND_PHONEFACTORY:
        break;

        case HF_IND_PHONESERIAL:
        break;

        case HF_IND_PHONEINDICATION: {

            int service = message.getIntExtra(INT_HF_CIEV_SERVICE, 0);
            int signal = message.getIntExtra(INT_HF_CIEV_SIGNAL, 0);
            int battchg = message.getIntExtra(INT_HF_CIEV_BATTCHG, 0);
            int roam = message.getIntExtra(INT_HF_CIEV_ROAM, 0);
            PRINTF_TO_CONSOLE(TAG, "indication service is %d, signal is %d, battchg is %d, roam is %d", service, signal, battchg, roam);

        }
        break;

        case HF_IND_RINGTONE: {
            PRINTF_TO_CONSOLE(TAG, "indication ring");
        }
        break;

        case HF_IND_PHONESPEAKERGAIN: {

            int agSpeakerGain = message.getIntExtra(INT_HF_AGSPEAKERGAIN, 0);
            PRINTF_TO_CONSOLE(TAG, "indication agSpeakerGain is %d", agSpeakerGain);
        }
        break;

        case HF_IND_PHONEMICGAIN:{
            int agMicGain = message.getIntExtra(INT_HF_AGMICGAIN, 0);
            PRINTF_TO_CONSOLE(TAG, "indication agMicGain is %d", agMicGain);
        }
        break;

        case HF_IND_ATRESULT:{
            int resultCode = message.getIntExtra(INT_HF_RESULT_CODE, 0);
            if(HF_AT_RESULT_ERROR_NO_ANSWER == resultCode) {
                PRINTF_TO_CONSOLE(TAG, "AT resultCode is %d", resultCode);
            }
        }
        break;
        }

    checkIndication(message.what, expectedValue);

    return 0;
}

