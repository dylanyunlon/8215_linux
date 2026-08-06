#include "BtPtsAvrcpHandler.h"
#include "clog.h"

using namespace universal_utils;

static const char* TAG = "BtPtsAvrcpHandler";

const map<string, HANDLE_FUN> BtPtsAvrcpHandler::AVRCP_HANDLE_MAP = {
    {CMD_PROFILE_CONNECT,     static_cast<HANDLE_FUN>(&BtPtsAvrcpHandler::connect)},
    {CMD_PROFILE_DISCONNECT,  static_cast<HANDLE_FUN>(&BtPtsAvrcpHandler::disconnect)},
    {CMD_RC_PLAY,             static_cast<HANDLE_FUN>(&BtPtsAvrcpHandler::play)},
    {CMD_RC_PAUSE,            static_cast<HANDLE_FUN>(&BtPtsAvrcpHandler::pause)},
    {CMD_RC_STOP,             static_cast<HANDLE_FUN>(&BtPtsAvrcpHandler::stop)},
    {CMD_RC_PRE,              static_cast<HANDLE_FUN>(&BtPtsAvrcpHandler::pre)},
    {CMD_RC_NEXT,             static_cast<HANDLE_FUN>(&BtPtsAvrcpHandler::next)},
    {CMD_RC_UPPLAYSTATUS,     static_cast<HANDLE_FUN>(&BtPtsAvrcpHandler::updatePlayStatus)},
    {CMD_RC_UPID3,            static_cast<HANDLE_FUN>(&BtPtsAvrcpHandler::updateMediaInfo)},
    {CMD_RC_SET_VOL,          static_cast<HANDLE_FUN>(&BtPtsAvrcpHandler::setVolume)},
    {CMD_RC_REG_INTERIM_VOL,  static_cast<HANDLE_FUN>(&BtPtsAvrcpHandler::registerInterimVolume)},
};

BtPtsAvrcpHandler::BtPtsAvrcpHandler(IBluetoothClient *client)
    : BtPtsHandler(PROFILE_AVRCP)
    , m_a2dpInterface(NULL)
    , m_avrcpInterface(NULL)
    , m_a2dpCallback(NULL)
    , m_avrcpCallback(NULL)
{
    IBluetoothProfile *profile = NULL;
    client->getProfile(A2DPPROFILENAME, &profile);
    m_a2dpInterface = dynamic_cast<IBluetoothA2dp*>(profile);
    EXPECT_NOT_NULL(m_a2dpInterface, "m_a2dpInterface");
    client->getProfile(AVRCPPROFILENAME, &profile);
    m_avrcpInterface = dynamic_cast<IBluetoothAvrcp*>(profile);
    EXPECT_NOT_NULL(m_avrcpInterface, "m_avrcpInterface");
    registerCallBack();
}

BtPtsAvrcpHandler::~BtPtsAvrcpHandler()
{
    deregisterCallBack();
}

const map<string, HANDLE_FUN>& BtPtsAvrcpHandler::getHandleFunMap()
{
    return AVRCP_HANDLE_MAP;
}

bool BtPtsAvrcpHandler::isAutoConnected()
{
    return true;
}

void BtPtsAvrcpHandler::registerCallBack()
{
    m_a2dpCallback = new BtPtsA2DPCallBack();
    m_avrcpCallback = new BtPtsAVRCPCallBack();
    m_a2dpInterface->registerCallBack(*m_a2dpCallback);
    m_avrcpInterface->registerCallBack(*m_avrcpCallback);
}
void BtPtsAvrcpHandler::deregisterCallBack()
{
    m_a2dpInterface->deregisterCallBack(*m_a2dpCallback);
    m_avrcpInterface->deregisterCallBack(*m_avrcpCallback);
    SAFE_DELETE(m_a2dpCallback);
    SAFE_DELETE(m_avrcpCallback);
}

int BtPtsAvrcpHandler::connect()
{
    int ret = BTPTS_ERROR;
    string address = m_args.getStringExtra("str1", "");
    if (address != "")
        m_address = address;

    E_BLUETOOTH_PROFILE_STATE state = BLUETOOTH_PROFILE_IDLE;
    m_a2dpInterface->getState(state);
    bool isA2dpCnnected = false;

    int retryCnt = m_args.getIntExtra("int2", 0);

    do {
        if (state != BLUETOOTH_PROFILE_CONNECTED) {
            PRINTF_TO_CONSOLE(TAG, "connect address: %s, retryCnt: %d", m_address.toString().c_str(), retryCnt);

            ret = m_a2dpInterface->connect(m_address);
            EXPECT_EQ(BTPTS_OK, ret);
            isA2dpCnnected = m_a2dpCallback->waitCommandDone(A2DP_IND_DEVICE_CONNECTED, WAIT_MAX_TIME, BTPTS_OK);
        } else {
            PRINTF_TO_WARN(TAG, "a2dp/avrcp is already connected");
            break;
        }

        // check pass
        if (isA2dpCnnected) {
            UTILS_LOGI(TAG, "a2dp connect address: %s done, retry time is %d", address.c_str(), retryCnt);
            break;
        }

        if (--retryCnt <= 0) {
            m_a2dpInterface->getState(state);
            EXPECT_EQ(BLUETOOTH_PROFILE_CONNECTED, state);
        }

        sleepSecondsForRetry("rc connect");

    } while (retryCnt > 0);

    return ret;
}

int BtPtsAvrcpHandler::disconnect()
{
    int ret = BTPTS_ERROR;

    E_BLUETOOTH_PROFILE_STATE state = BLUETOOTH_PROFILE_IDLE;
    m_a2dpInterface->getState(state);
    if (state == BLUETOOTH_PROFILE_CONNECTED) {
        BluetoothAddress address;
        m_a2dpInterface->getConnectedDevice(address);
        PRINTF_TO_CONSOLE(TAG, "disconnect address: %s", address.toString().c_str());
        ret = m_a2dpInterface->disconnect(address);
        EXPECT_EQ(BTPTS_OK, ret);
        EXPECT_EQ(true, m_a2dpCallback->waitCommandDone(A2DP_IND_DEVICE_DISCONNECTED, WAIT_MAX_TIME));
    }

    return ret;
}
int BtPtsAvrcpHandler::play()
{
    PRINTF_TO_CONSOLE(TAG, __FUNCTION__);

    int ret = BTPTS_ERROR;
    updatePlayStatus();
    if (m_avrcpCallback->getMusicState()!= PLAYING) {
        ret = m_avrcpInterface->play();
        EXPECT_EQ(BTPTS_OK, ret);
        ret = m_avrcpCallback->waitCommandDone(AVRCP_IND_PLAY_STATUS_CHANGED, WAIT_TIME, PLAYING);
        if (!ret)
            EXPECT_EQ(PLAYING, m_avrcpCallback->getMusicState());

    } else {
        PRINTF_TO_WARN(TAG, "rc is already playing");
    }

    return ret;
}
int BtPtsAvrcpHandler::pause()
{
    PRINTF_TO_CONSOLE(TAG, __FUNCTION__);

    int ret = BTPTS_ERROR;
    updatePlayStatus();
    if (m_avrcpCallback->getMusicState() == PLAYING) {
        ret = m_avrcpInterface->pause();
        EXPECT_EQ(BTPTS_OK, ret);
        ret = m_avrcpCallback->waitCommandDone(AVRCP_IND_PLAY_STATUS_CHANGED, WAIT_TIME, PAUSED);
        if (!ret)
            EXPECT_EQ(PAUSED, m_avrcpCallback->getMusicState());

    } else {
        PRINTF_TO_WARN(TAG, "rc is not playing");
    }

    return ret;
}
int BtPtsAvrcpHandler::stop()
{
    PRINTF_TO_CONSOLE(TAG, __FUNCTION__);
    int ret = BTPTS_ERROR;
    updatePlayStatus();
    if (m_avrcpCallback->getMusicState() == PLAYING) {
        ret = m_avrcpInterface->stop();
        EXPECT_EQ(BTPTS_OK, ret);
        ret = m_avrcpCallback->waitCommandDone(AVRCP_IND_PLAY_STATUS_CHANGED, WAIT_TIME, STOPPED);
        if (!ret)
            EXPECT_EQ(STOPPED, m_avrcpCallback->getMusicState());
    } else {
        PRINTF_TO_WARN(TAG, "rc is not playing");
    }

    return ret;
}
int BtPtsAvrcpHandler::pre()
{
    PRINTF_TO_CONSOLE(TAG, __FUNCTION__);

    int ret = BTPTS_ERROR;
    ret = m_avrcpInterface->prev();
    EXPECT_EQ(BTPTS_OK, ret);
    EXPECT_EQ(true, m_avrcpCallback->waitCommandDone(AVRCP_IND_MEDIA_DATA_CHANGED, WAIT_MAX_TIME));

    return ret;
}
int BtPtsAvrcpHandler::next()
{
    PRINTF_TO_CONSOLE(TAG, __FUNCTION__);

    int ret = BTPTS_ERROR;
    ret = m_avrcpInterface->next();
    EXPECT_EQ(BTPTS_OK, ret);
    EXPECT_EQ(true, m_avrcpCallback->waitCommandDone(AVRCP_IND_MEDIA_DATA_CHANGED, WAIT_MAX_TIME));

    return ret;
}
int BtPtsAvrcpHandler::updatePlayStatus()
{
    PRINTF_TO_CONSOLE(TAG, __FUNCTION__);

    int ret = BTPTS_ERROR;
    ret = m_avrcpInterface->updatePlayStatus();
    EXPECT_EQ(BTPTS_OK, ret);
    EXPECT_EQ(true, m_avrcpCallback->waitCommandDone(AVRCP_IND_PLAYBACK_DATA_UPDATE, WAIT_MAX_TIME));

    return ret;
}
int BtPtsAvrcpHandler::updateMediaInfo()
{
    PRINTF_TO_CONSOLE(TAG, __FUNCTION__);

    int ret = BTPTS_ERROR;
    ret = m_avrcpInterface->updateMediaInfo();
    EXPECT_EQ(BTPTS_OK, ret);
    EXPECT_EQ(true, m_avrcpCallback->waitCommandDone(AVRCP_IND_MEDIA_DATA_CHANGED, WAIT_MAX_TIME));
    string title = "unknown";
    string artist = "unknown";
    string album = "unknown";
    string handle = "unknown";
    int getId3 = 0;
    getId3 = m_avrcpInterface->getMediaTitle(title);
    if (-1 == getId3 || title == "") {
        title = "unknown";
    }
    getId3 = m_avrcpInterface->getMediaArtist(artist);
    if (-1 == getId3 || artist == "") {
        artist = "unknown";
    }
    getId3 = m_avrcpInterface->getMediaAlbum(album);
    if (-1 == getId3 || album == "") {
        album = "unknown";
    }
    getId3 = m_avrcpInterface->getMediaAlbumHandle(handle);
    if (-1 == getId3 || album == "") {
        handle = "unknown";
    }

    PRINTF_TO_CONSOLE(TAG, "updateMediaInfo title is %s, artist is %s, album is %s, handle is %s",
        title.c_str(), artist.c_str(), album.c_str(), handle.c_str());

    sleep(2);

    // just test getMediaAlbumImgProperty & getMediaAlbumImgData interface
    // when writing code, call getMediaAlbumImgProperty() upon receiving message AVRCP_IND_MEDIA_DATA_IMG_PROPERTY_CHANGED,
    // call getMediaAlbumImgData upon receiving message AVRCP_IND_MEDIA_DATA_IMG_DATA_CHANGED
    vector<BluetoothBipImageProperties> imgProperty;
    getId3 = m_avrcpInterface->getMediaAlbumImgProperty(imgProperty);
    if (-1 == getId3) {
        PRINTF_TO_CONSOLE(TAG, "updateMediaInfo imgProperty is null");
    }
    for (BluetoothBipImageProperties imgP : imgProperty) {
        PRINTF_TO_CONSOLE(TAG, "updateMediaInfo imgProperty: imgpropertyhandle is %s, format is %s, encoding is %s,  img_width is %d, img_height is %d",
            imgP.getImgPropertyHandle().c_str(),
            imgP.getImgPropertyFormat().c_str(),
            imgP.getImgPropertyEncoding().c_str(),
            imgP.getImgWidth(),
            imgP.getImgHeight());
    }

    string imghandle = "unknown";
    vector<unsigned char> imgdata;
    getId3 = m_avrcpInterface->getMediaAlbumImgData(imghandle, imgdata);
    if (-1 == getId3) {
        PRINTF_TO_CONSOLE(TAG, "updateMediaInfo imgData is null");
    }
#if 1
    else {
        string filename = "/data/misc/bluetooth/cover_art_" + imghandle + ".jpg";

        FILE* file = fopen(filename.c_str(), "wb");
        if (file != NULL) {
            size_t written = fwrite(imgdata.data(), sizeof(unsigned char), imgdata.size(), file);
            fclose(file);

            if (written == imgdata.size()) {
                PRINTF_TO_CONSOLE(TAG, "Successfully saved cover art to %s, size: %zu bytes", 
                                  filename.c_str(), imgdata.size());
            } else {
                PRINTF_TO_CONSOLE(TAG, "Failed to write complete image data to %s", 
                                  filename.c_str());
            }
        } else {
            PRINTF_TO_CONSOLE(TAG, "Failed to open file for writing: %s", 
                              filename.c_str());
        }
    }
#endif
    PRINTF_TO_CONSOLE(TAG, "updateMediaInfo imghandle is %s, imgdata size is %d",
        imghandle.c_str(), imgdata.size());

    return BTPTS_OK;
}

int BtPtsAvrcpHandler::setVolume()
{
    int ret = BTPTS_ERROR;
    int volume = m_args.getIntExtra("int1", 0);
    PRINTF_TO_CONSOLE(TAG, "volume is %d", volume);

    ret = m_avrcpInterface->setVolume(volume);
    EXPECT_EQ(BTPTS_OK, ret);

    EXPECT_EQ(true, m_avrcpCallback->waitCommandDone(AVRCP_IND_REGISTER_ABS_VOL_NOTIFICATION, WAIT_MIN_TIME));

    PRINTF_TO_CONSOLE(TAG, "receive phone notify vol change");
    ret = m_avrcpInterface->registerInterimVolume(volume);
    EXPECT_EQ(BTPTS_OK, ret);
    PRINTF_TO_CONSOLE(TAG, "carkit send interm");

    return ret;
}

int BtPtsAvrcpHandler::registerInterimVolume()
{
    int ret = BTPTS_ERROR;
    int volume = m_args.getIntExtra("int1", 0);
    PRINTF_TO_CONSOLE(TAG, "volume is %d", volume);

    ret = m_avrcpInterface->registerInterimVolume(volume);
    EXPECT_EQ(BTPTS_OK, ret);

    return ret;
}

void BtPtsAvrcpHandler::sleepSecondsForRetry(const string &retryCmd)
{
    int waitSeconds = m_args.getIntExtra("int2", 0);
    if (retryCmd.find("connect") != std::string::npos) {
        waitSeconds = m_args.getIntExtra("int3", 0);
    }
    if (waitSeconds > 0) {
        UTILS_LOGI(TAG, "sleep %d s retry %s... ", waitSeconds, retryCmd.c_str());
        sleep(waitSeconds);
    }
}

bool BtPtsAvrcpHandler::dump(string &dumpInfo)
{
    E_BLUETOOTH_PROFILE_STATE state = BLUETOOTH_PROFILE_IDLE;

    m_a2dpInterface->getState(state);
    dumpInfo.append("a2dp state: " + connectionStateToString(state) + "\n");
    if (state == BLUETOOTH_PROFILE_CONNECTED) {
        BluetoothAddress device;
        m_a2dpInterface->getConnectedDevice(device);
        dumpInfo.append("a2dp device: " + device.toString() + "\n");
    }

    m_avrcpInterface->getState(state);
    dumpInfo.append("avrcp state: " + connectionStateToString(state) + "\n");
    if (state == BLUETOOTH_PROFILE_CONNECTED) {
        BluetoothAddress device;
        m_avrcpInterface->getConnectedDevice(device);
        dumpInfo.append("avrcp device: " + device.toString() + "\n");
    }
}

template<> BtPtsA2DPCallBack* Singleton<BtPtsA2DPCallBack>::msSingleton = NULL;
int BtPtsA2DPCallBack::onIndication(const CMessage &message)
{
    int expectedValue = 0;
    switch (message.what) {
        case A2DP_IND_DEVICE_CONNECTED: {
            string address = message.getStringExtra(STRING_ADDRESS, " ");
            int result = message.arg1;
            expectedValue = result;
            PRINTF_TO_CONSOLE(TAG, "indication: a2dp connected, address = %s, result is %d", address.c_str(), result);
        }
        break;

        case A2DP_IND_DEVICE_DISCONNECTED: {
            string address = message.getStringExtra(STRING_ADDRESS, " ");
            PRINTF_TO_CONSOLE(TAG, "indication: a2dp disconnected, address = %s\n", address.c_str());

        }
        break;

        case A2DP_IND_PLAY_START:
            PRINTF_TO_CONSOLE(TAG, "indication: a2dp play start");
            break;

        case A2DP_IND_PLAY_SUSPEND:
            PRINTF_TO_CONSOLE(TAG, "indication: a2dp play suspend");
            break;

        case A2DP_IND_END:
            PRINTF_TO_CONSOLE(TAG, "indication: a2dp ind end");
            break;

    }
    checkIndication(message.what);
    return 0;
}


template<> BtPtsAVRCPCallBack* Singleton<BtPtsAVRCPCallBack>::msSingleton = NULL;

BtPtsAVRCPCallBack::BtPtsAVRCPCallBack() : m_musicState(STOPPED)
{

}

int BtPtsAVRCPCallBack::getMusicState()
{
    return m_musicState;
}

int BtPtsAVRCPCallBack::onIndication(const CMessage &message)
{
    int expectedValue = 0;

    switch (message.what) {
        case AVRCP_IND_DEVICE_CONNECTED: {
            string address = message.getStringExtra(STRING_ADDRESS, " ");
            int result = message.arg1;
            expectedValue = result;
            PRINTF_TO_CONSOLE(TAG, "indication: avrcp connected, address = %s, result is %d", address.c_str(), result);
        }
        break;

        case AVRCP_IND_DEVICE_DISCONNECTED: {

            string address = message.getStringExtra(STRING_ADDRESS, " ");
            PRINTF_TO_CONSOLE(TAG, "indication: avrcp disconnected, address = %s", address.c_str());
        }
        break;

        case AVRCP_IND_PLAYBACK_DATA_UPDATE: {

            int musicState = message.getIntExtra(BYTE_PLAYBACK_STATUS, 0);
            PRINTF_TO_CONSOLE(TAG, "indication: avrcp update musicState:%d, stop(%d), playing(%d), pause(%d)",
                musicState,
                STOPPED,
                PLAYING,
                PAUSED);
            m_musicState = musicState;

            int playingTime = message.getIntExtra(INT_PLAYING_TIME, 0);
            int totalTime = message.getIntExtra(INT_TOTAL_TIME, 0);
            PRINTF_TO_CONSOLE(TAG, "indication:AVRCP_IND_PLAYBACK_DATA_UPDATE playingTime = %d, totalTime = %d", playingTime, totalTime);
        }
        break;

        case AVRCP_IND_SONG_POSITION_CHANGED: {
            int playingTime = message.getIntExtra(INT_PLAYING_TIME, 0);
            int totalTime = message.getIntExtra(INT_TOTAL_TIME, 0);
            PRINTF_TO_CONSOLE(TAG, "indication:AVRCP_IND_SONG_POSITION_CHANGED playingTime = %d, totalTime = %d", playingTime, totalTime);
        }
        break;

        case AVRCP_IND_PLAY_STATUS_CHANGED: {
            int musicState = message.getIntExtra(BYTE_PLAYBACK_STATUS, 0);
            PRINTF_TO_CONSOLE(TAG, "indication: avrcp play status change:%d, stop(%d), playing(%d), pause(%d), fwd_seek(%d), rev_seek(%d), error(%d)\n",
                musicState,
                STOPPED,
                PLAYING,
                PAUSED,
                FWD_SEEK,
                REV_SEEK,
                ERROR);
            expectedValue = musicState;
            }
            break;

        case AVRCP_IND_MEDIA_DATA_CHANGED: {
            string address = message.getStringExtra(STRING_ADDRESS, " ");
            PRINTF_TO_CONSOLE(TAG, "indication: avrcp id3 update, address = %s", address.c_str());
        }
        break;

        case AVRCP_IND_SET_ABS_VOL: {
            string address = message.getStringExtra(STRING_ADDRESS, " ");
            int vol = message.arg1;
            int label = message.arg2;
            PRINTF_TO_CONSOLE(TAG, "indication: avrcp phone set abs vol, address = %s, vol(%d), label(%d)", address.c_str(), vol, label);
        }
        break;

        // When receive AVRCP_IND_REGISTER_ABS_VOL_NOTIFICATION callback, must call registerInterimVolume(int abs_vol) interface
        case AVRCP_IND_REGISTER_ABS_VOL_NOTIFICATION: {
            string address = message.getStringExtra(STRING_ADDRESS, " ");
            int label = message.arg1;
            PRINTF_TO_CONSOLE(TAG, "indication: avrcp phone notify abs vol, address = %s, label(%d)", address.c_str(), label);
        }
        break;

        case AVRCP_IND_MEDIA_DATA_IMG_PROPERTY_CHANGED:
            PRINTF_TO_CONSOLE(TAG, "indication: avrcp img property update");
            break;

        case AVRCP_IND_MEDIA_DATA_IMG_DATA_CHANGED:
            PRINTF_TO_CONSOLE(TAG, "indication: avrcp img data update");
            break;

    }

    checkIndication(message.what, expectedValue);
    return 0;
}