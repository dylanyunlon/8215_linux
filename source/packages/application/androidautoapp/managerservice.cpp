#include "managerservice.h"

static const char constexpr *TAG = "[AAApp]ManagerService";

namespace {
constexpr int KEY_ENTER = 49;
constexpr int KEY_UP = 50;
constexpr int KEY_BACK = 51;
constexpr int KEY_LEFT = 52;
constexpr int KEY_DOWN = 53;
constexpr int KEY_RIGHT = 54;
constexpr int LONG_PRESS_THRESHOLD_MS = 500;
constexpr int ROTATE_DELTA_CLOCKWISE = 1;
constexpr int ROTATE_DELTA_COUNTERCLOCKWISE = -1;

bool isLeftRightKey(int key)
{
    return key == KEY_LEFT || key == KEY_RIGHT;
}

bool mapToAAKeyCode(int key, AAKeyCode &aaKeyCode)
{
    switch (key) {
        case KEY_LEFT:
            aaKeyCode = AAKeyCode::AAKEYCODE_DPAD_LEFT;
            break;
        case KEY_RIGHT:
            aaKeyCode = AAKeyCode::AAKEYCODE_DPAD_RIGHT;
            break;
        case KEY_UP:
            aaKeyCode = AAKeyCode::AAKEYCODE_DPAD_UP;
            break;
        case KEY_DOWN:
            aaKeyCode = AAKeyCode::AAKEYCODE_DPAD_DOWN;
            break;
        case KEY_ENTER:
            aaKeyCode = AAKeyCode::AAKEYCODE_DPAD_CENTER;
            break;
        case KEY_BACK:
            aaKeyCode = AAKeyCode::AAKEYCODE_BACK;
            break;
        default:
            return false;
    }

    return true;
}

int getRotateDelta(int key)
{
    return key == KEY_RIGHT ? ROTATE_DELTA_CLOCKWISE : ROTATE_DELTA_COUNTERCLOCKWISE;
}

}

ManagerService::ManagerService()
    : CQObjListener(CAPPBaseObj::APPID_ANDROIDAUTO_APP)
{
}

ManagerService::~ManagerService()
{
    LOGD(TAG, "~ManagerService");
    if (nullptr != mpAAClient) {
        IAndroidAutoClient::releaseAndroidAutoClient();
    }
    if (nullptr != mpAccessoryInfo) {
        mpAccessoryInfo->unregisterCallback();
        mpAccessoryInfo->disable();
        vendor::autochips::carlink::releaseAccessoryInfoClient();
    }

}

ManagerService* ManagerService::getInstance()
{
    static std::unique_ptr<ManagerService> pManagerService = std::unique_ptr<ManagerService>(new ManagerService());
    return pManagerService.get();
}

void ManagerService::onStart(soapp_exit_handler exit_handler, void *handle, void *param, QApplication *app, QQmlApplicationEngine *engine)
{
    mpEngine = engine;
    mpEngine->load(QUrl(QStringLiteral("qrc:/main.qml")));
    QObject *topLevel = mpEngine->rootObjects().value(0);
    QQuickWindow *appWindow = qobject_cast<QQuickWindow *>(topLevel);
    appWindow->setFlags(appWindow->flags() | Qt::CoverWindow);
    if (!appWindow) {
        LOGE(TAG, "appWindow is null!");
    }
    bool ret = initListener(appWindow, exit_handler, handle, param);
    LOGD(TAG, "initListener ret = %s", ret ? "success" : "failed");

    QQmlContext *ctx = mpEngine->rootContext();
    ctx->setContextProperty("managerservice", this);
    initClient();

    if (!hasConnectedDevice()) {
        std::vector<MobileDevice> devices;
        mpAAClient->getWaitingConfirmConnectDevices(devices);
        if (!devices.empty()) {
            addSupportedConfiguration();
            mConnectingDevice = devices.at(0);
            mpAAClient->confirmConnect(devices.at(0), true);
        }
    }
    // initBTClient();
    // initCarEventClient();
}

// void ManagerService::initConnection()
// {
//     QQmlContext *ctx = mpEngine->rootContext();
//     ctx->setContextProperty("managerservice", this);
// }

// void ManagerService::initBTClient()
// {
//     m_pBTClient = getBluetoothClient();
//     if (nullptr == m_pBTClient) {
//         LOGE(TAG, "m_pBTClient is empty!");
//     }

//     int ret = 0;
//     IBluetoothProfile *profile = nullptr;

//     ret = m_pBTClient->getProfile(HANDSFREEPROFILENAME, &profile);
//     if (0 <= ret) {
//         ;
//     } else {
//         LOGE(TAG, "get handsfree profile Fail!, profile is %p", profile);
//     }

//     m_pHfpInterface = dynamic_cast<IBluetoothHandsfree*>(profile);
//     ret = m_pHfpInterface->registerCallBack(*BluetoothHFPCallBack::getInstance());
//     if (0 <= ret) {
//         ;
//     } else {
//         LOGE(TAG, "registerHFPCallBack Fail!");
//     }
// }

// void ManagerService::initCarEventClient()
// {
//     m_pCarEventMonitor = ICarEventMonitor::getInstance();
//     if (nullptr == m_pCarEventMonitor) {
//         LOGE(TAG, "get ICarEventMonitor fail");
//     }
//     bool ret = false;
//     ret = m_pCarEventMonitor->registerCallback(CarEventCallback::getInstance());
//     if (!ret) {
//         LOGE(TAG, "register careventcallback fail");
//     }
// }

void ManagerService::initClient()
{
    AndroidAutoReturnValue ret;

    mpAAClient = IAndroidAutoClient::getAndroidAutoClient();
    if (nullptr == mpAAClient) {
        LOGE(TAG, "get IAndroidAutoClient fail");
    }
    mpAACallback = std::unique_ptr<AACallback>(new AACallback());
    ret = mpAAClient->registerCallBack(*mpAACallback);
    if (ret != AndroidAutoReturnValue::ANDROID_AUTO_OK) {
        LOGE(TAG, "register androidauto callback fail");
    }

    int type = 0;
    type |= (int)AccessoryFunctionType::FUNCTION_TYPE_RVC;

    mpAccessoryInfo = vendor::autochips::carlink::getAccessoryInfoClient();
    mpAccessoryCallback = std::unique_ptr<AccessoryInfoCallback>(new AccessoryInfoCallback());
    mpAccessoryInfo->enable(type);
    mpAccessoryInfo->registerCallback(*mpAccessoryCallback, type);

    //mVolumeClient = VolumeSetting::getInstance();
}

bool ManagerService::hasConnectedDevice()
{
    bool ret = false;
    std::vector<MobileDevice> devices;
    mpAAClient->getConnectedDevices(devices);

    if (devices.empty()) {
        mActiveDevice = MobileDevice();
    } else {
        mActiveDevice = devices.at(0);
        ret = true;
    }

    return ret;
}

void ManagerService::addSupportedConfiguration()
{
    std::vector<VideoConfiguration> videoConfiguration;
    {
        VideoConfiguration vc;
        vc.mCodecResolution = std::make_unique<VideoConfiguration::VideoCodecResolutionType>(VideoConfiguration::VIDEO_1280x720);
        vc.mFrameRate = std::make_unique<uint32_t>(60);
        vc.mWidthMargin = std::make_unique<uint32_t>(256);
        vc.mHeightMargin = std::make_unique<uint32_t>(120);
        vc.mDensity = std::make_unique<uint32_t>(213);
        vc.mDecoderAdditionalDepth = std::make_unique<uint32_t>(0);
        vc.mPixelAspectRatioE4 = std::make_unique<uint32_t>(10026);
        vc.mRealDensity = std::make_unique<uint32_t>(169);

        vc.mUiConfig = std::make_unique<VideoConfiguration::UiConfig>();
        vc.mUiConfig->mMargins = std::make_unique<VideoConfiguration::Insets>();
        vc.mUiConfig->mMargins->mTop = std::make_unique<uint32_t>(60);
        vc.mUiConfig->mMargins->mBottom = std::make_unique<uint32_t>(60);
        vc.mUiConfig->mMargins->mLeft = std::make_unique<uint32_t>(128);
        vc.mUiConfig->mMargins->mRight = std::make_unique<uint32_t>(128);

        vc.mUiConfig->mContentInsets = std::make_unique<VideoConfiguration::Insets>();
        vc.mUiConfig->mContentInsets->mTop = std::make_unique<uint32_t>(0);
        vc.mUiConfig->mContentInsets->mBottom = std::make_unique<uint32_t>(0);
        vc.mUiConfig->mContentInsets->mLeft = std::make_unique<uint32_t>(0);
        vc.mUiConfig->mContentInsets->mRight = std::make_unique<uint32_t>(0);

        vc.mUiConfig->mStableContentInsets = std::make_unique<VideoConfiguration::Insets>();
        vc.mUiConfig->mStableContentInsets->mTop = std::make_unique<uint32_t>(0);
        vc.mUiConfig->mStableContentInsets->mBottom = std::make_unique<uint32_t>(0);
        vc.mUiConfig->mStableContentInsets->mLeft = std::make_unique<uint32_t>(0);
        vc.mUiConfig->mStableContentInsets->mRight = std::make_unique<uint32_t>(0);

        vc.mUiConfig->mUiTheme = std::make_unique<VideoConfiguration::UiTheme>(VideoConfiguration::UI_THEME_AUTOMATIC);

        videoConfiguration.push_back(vc);
    }
    {
        VideoConfiguration vc;
        vc.mCodecResolution = std::make_unique<VideoConfiguration::VideoCodecResolutionType>(VideoConfiguration::VIDEO_800x480);
        vc.mFrameRate = std::make_unique<uint32_t>(60);
        vc.mWidthMargin = std::make_unique<uint32_t>(0);
        vc.mHeightMargin = std::make_unique<uint32_t>(12);
        vc.mDensity = std::make_unique<uint32_t>(166);
        vc.mDecoderAdditionalDepth = std::make_unique<uint32_t>(0);
        vc.mPixelAspectRatioE4 = std::make_unique<uint32_t>(10026);
        vc.mRealDensity = std::make_unique<uint32_t>(132);

        vc.mUiConfig = std::make_unique<VideoConfiguration::UiConfig>();
        vc.mUiConfig->mMargins = std::make_unique<VideoConfiguration::Insets>();
        vc.mUiConfig->mMargins->mTop = std::make_unique<uint32_t>(5);
        vc.mUiConfig->mMargins->mBottom = std::make_unique<uint32_t>(5);
        vc.mUiConfig->mMargins->mLeft = std::make_unique<uint32_t>(0);
        vc.mUiConfig->mMargins->mRight = std::make_unique<uint32_t>(0);

        vc.mUiConfig->mContentInsets = std::make_unique<VideoConfiguration::Insets>();
        vc.mUiConfig->mContentInsets->mTop = std::make_unique<uint32_t>(0);
        vc.mUiConfig->mContentInsets->mBottom = std::make_unique<uint32_t>(0);
        vc.mUiConfig->mContentInsets->mLeft = std::make_unique<uint32_t>(0);
        vc.mUiConfig->mContentInsets->mRight = std::make_unique<uint32_t>(0);

        vc.mUiConfig->mStableContentInsets = std::make_unique<VideoConfiguration::Insets>();
        vc.mUiConfig->mStableContentInsets->mTop = std::make_unique<uint32_t>(0);
        vc.mUiConfig->mStableContentInsets->mBottom = std::make_unique<uint32_t>(0);
        vc.mUiConfig->mStableContentInsets->mLeft = std::make_unique<uint32_t>(0);
        vc.mUiConfig->mStableContentInsets->mRight = std::make_unique<uint32_t>(0);

        vc.mUiConfig->mUiTheme = std::make_unique<VideoConfiguration::UiTheme>(VideoConfiguration::UI_THEME_AUTOMATIC);

        videoConfiguration.push_back(vc);
    }
    mpAAClient->addSupportedConfiguration(videoConfiguration);
}

void ManagerService::onUITouched(int action, int pointX, int pointY)
{
    if (mActiveDevice.getDeviceId() != "" || mActiveDevice.getBluetoothId() != "" || mActiveDevice.getUsbId() != "") {
        mpAAClient->sendTouchEvent(mActiveDevice, 1, new int[1]{pointX}, new int[1]{pointY}, new int[1]{0}, (AAPointerAction)action, 0);
    }
}

void ManagerService::onKeyEvent(bool pressed, int key)
{
    LOGD(TAG, "%s pressed(%d), key(%d)", __func__, pressed, key);

    if (mpAAClient == nullptr) {
        LOGE(TAG, "mpAAClient is null");
        return;
    }

    if (mActiveDevice.getDeviceId() == "" && mActiveDevice.getBluetoothId() == "" && mActiveDevice.getUsbId() == "") {
        return;
    }

    AAKeyCode aaKeyCode = AAKeyCode::AAKEYCODE_DPAD_CENTER;
    if (!mapToAAKeyCode(key, aaKeyCode)) {
        LOGW(TAG, "Unsupported key code: %d", key);
        return;
    }

    if (!isLeftRightKey(key)) {
        mpAAClient->sendShortPressKeyEvent(mActiveDevice, aaKeyCode, pressed);
        return;
    }

    if (pressed) {
        if (mPendingLeftRightKey == key) {
            LOGD(TAG, "Ignore repeated left/right key press, key(%d)", key);
            return;
        }

        if (mPendingLeftRightKey != -1) {
            LOGW(TAG, "Replace pending left/right key(%d) with key(%d)", mPendingLeftRightKey, key);
        }

        mPendingLeftRightKey = key;
        mPendingLeftRightKeyTimer.restart();
        return;
    }

    if (mPendingLeftRightKey != key || !mPendingLeftRightKeyTimer.isValid()) {
        LOGW(TAG, "Ignore left/right key release without matching press, key(%d), pending(%d)", key, mPendingLeftRightKey);
        return;
    }

    const long long durationMs = static_cast<long long>(mPendingLeftRightKeyTimer.elapsed());
    mPendingLeftRightKey = -1;
    mPendingLeftRightKeyTimer.invalidate();

    if (durationMs >= LONG_PRESS_THRESHOLD_MS) {
        const int delta = getRotateDelta(key);
        LOGD(TAG, "Send rotate event, key(%d), durationMs(%lld), delta(%d)", key, durationMs, delta);
        mpAAClient->sendRotateEvent(mActiveDevice, AAKeyCode::AAKEYCODE_ROTARY_CONTROLLER, delta);
        return;
    }

    LOGD(TAG, "Send short left/right key event, key(%d), durationMs(%lld)", key, durationMs);
    mpAAClient->sendShortPressKeyEvent(mActiveDevice, aaKeyCode, true);
    mpAAClient->sendShortPressKeyEvent(mActiveDevice, aaKeyCode, false);
}

int ManagerService::doShowFrontUI(void)
{
    AAConnectionState connectState = AAConnectionState::AAUNKNOWN;
    if ((mActiveDevice.getDeviceId() != "" || mActiveDevice.getBluetoothId() != "" || mActiveDevice.getUsbId() != "")
            && (mpAAClient->getConnectionState(mActiveDevice, connectState), connectState == AAConnectionState::AACONNECTED)) {
        LOGD(TAG, "doShowFrontUI");
        CQObjListener::doShowFrontUI();
        mpAAClient->setVideoFocusMode(mActiveDevice, AAVideoFocusMode::AAVIDEO_FOCUS_PROJECTED, true);
    } else {
        GlobalBus::applyFor(GlobalBus::ACTION_HIDEFRONT, CAPPBaseObj::APPID_ANDROIDAUTO_APP, 0);
    }

    return 1;
}

int ManagerService::doShowFront (int param1, int param2)
{
    //LOGD(TAG, "doShowFront");
    CQObjListener::doShowFront(param1, param2);

    return 1;
}

int ManagerService::doHideFront (int param1, int param2)
{
    LOGD(TAG, "doHideFront");
    CQObjListener::doHideFront(param1, param2);

    return 1;
}

int ManagerService::doExit (int param1, int param2)
{
    LOGD(TAG, "doExit");
    CQObjListener::doExit(param1, param2);

    return 1;
}

int ManagerService::doAudioFocusChanged (CCtlListener::E_AVOUT aOut, CCtlListener::E_AUDIOFOCUS focus)
{
    LOGD(TAG, "doAudioFocusChanged, aout:[%d]; foucs: [%d]" ,(int)aOut, (int)focus);
    switch (focus) {
        case AUDIOFOCUS_LOSS: {
            mIsHoldAudioFocus = false;
            mIsAudioFocusTransientLoss = false;
            mpAAClient->setAudioFocusState(mActiveDevice, AAAudioFocusStateType::AAAUDIO_FOCUS_STATE_LOSS, true);
            break;
        }

        case AUDIOFOCUS_GAIN: {
            mIsHoldAudioFocus = true;
            mIsAudioFocusTransientLoss = false;
            // AAAudioFocusStateType currentState = AAAudioFocusStateType::AAAUDIO_FOCUS_STATE_UNKNOWN;
            // mpAAClient->getAudioFocusState(mActiveDevice, currentState);
            // if (mAAAudioFocusRequestType == AAAudioFocusRequestType::AAAUDIO_FOCUS_GAIN
            //                 || currentState == AAAudioFocusStateType::AAAUDIO_FOCUS_STATE_LOSS_TRANSIENT) {
            //     mpAAClient->setAudioFocusState(mActiveDevice, AAAudioFocusStateType::AAAUDIO_FOCUS_STATE_GAIN, false);
            // } else if (mAAAudioFocusRequestType == AAAudioFocusRequestType::AAAUDIO_FOCUS_GAIN_TRANSIENT
            //                 || mAAAudioFocusRequestType == AAAudioFocusRequestType::AAAUDIO_FOCUS_GAIN_TRANSIENT_MAY_DUCK){
            //     mpAAClient->setAudioFocusState(mActiveDevice, AAAudioFocusStateType::AAAUDIO_FOCUS_STATE_GAIN_TRANSIENT, false);
            // }
            mpAAClient->setAudioFocusState(mActiveDevice, AAAudioFocusStateType::AAAUDIO_FOCUS_STATE_GAIN, false);
            break;
        }

        case AUDIOFOCUS_GAIN_TRANSIENT: {
            mIsHoldAudioFocus = false;
            mIsAudioFocusTransientLoss = false;
            mpAAClient->setAudioFocusState(mActiveDevice, AAAudioFocusStateType::AAAUDIO_FOCUS_STATE_GAIN_TRANSIENT, false);
            break;
        }

        case AUDIOFOCUS_LOSS_TRANSIENT: {
            mIsHoldAudioFocus = false;
            mIsAudioFocusTransientLoss = true;
            mpAAClient->setAudioFocusState(mActiveDevice, AAAudioFocusStateType::AAAUDIO_FOCUS_STATE_LOSS_TRANSIENT, true);
            break;
        }

        case AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK: {
            mIsHoldAudioFocus = false;
            mIsAudioFocusTransientLoss = true;
            mpAAClient->setAudioFocusState(mActiveDevice, AAAudioFocusStateType::AAAUDIO_FOCUS_STATE_LOSS_TRANSIENT_CAN_DUCK, true);
            break;
        }

        case AUDIOFCOUS_REQUEST_FAILED: {
            mIsHoldAudioFocus = false;
            mpAAClient->setAudioFocusState(mActiveDevice, AAAudioFocusStateType::AAAUDIO_FOCUS_STATE_LOSS, false);
            break;
        }

        default:
            break;
    }

    return 1;
}

// void ManagerService::onBackcarStateChange(bool enterBackcar)
// {
//     LOGD(TAG, "onBackcarStateChange [%s]", enterBackcar? "enter backcar" : "exit backcar");
//     if (enterBackcar) {
//         mpAAClient->notifyAccessoryRvcStart();
//     } else {
//         mpAAClient->notifyAccessoryRvcStop();
//     }
// }

// void ManagerService::onBtCallStateChange(bool comeInCall)
// {
//     LOGD(TAG, "onBtCallStateChange [%s]", comeInCall? "come in call" : "out call");
//     if (comeInCall) {
//         mpAAClient->notifyAccessoryPhoneCallStart();
//     } else {
//         mpAAClient->notifyAccessoryPhoneCallStop();
//     }
// }

void ManagerService::AACallback::onEnableStateChanged(const AAEnableState &enableState)
{
    LOGD(TAG, "onEnableStateChanged state:[%d]", int(enableState));
    MobileDevice &activeDevice = ManagerService::getInstance()->mActiveDevice;
    MobileDevice &connectingDevice = ManagerService::getInstance()->mConnectingDevice;

    if (enableState == AAEnableState::AASTATE_OFF && !activeDevice.getDeviceId().empty()) {
        activeDevice = MobileDevice();
        connectingDevice = MobileDevice();

        if (ManagerService::getInstance()->mpSurface != NULL) {
            int err = IAtcSurface_hide(ManagerService::getInstance()->mpSurface);
            if (err < 0) {
                LOGD(TAG, "IAtcSurface_hide error: %d\n", err);
            }
            LOGD(TAG, "IAtcSurface_release\n");
            IAtcSurface_release(ManagerService::getInstance()->mpSurface);
            ManagerService::getInstance()->mpSurface = NULL;
        }
        GlobalBus::applyFor(GlobalBus::ACTION_HIDEFRONT, CAPPBaseObj::APPID_ANDROIDAUTO_APP, 0);
        GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REL, CAPPBaseObj::APPID_ANDROIDAUTO_APP, CAPPBaseObj::STREAM_VOICL_CALL);
        GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REL, CAPPBaseObj::APPID_ANDROIDAUTO_APP, CAPPBaseObj::STREAM_ASSISTANT);
        GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REL, CAPPBaseObj::APPID_ANDROIDAUTO_APP, CAPPBaseObj::STRAEM_NOTIFICATION_RINGTONE);
        GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REL, CAPPBaseObj::APPID_ANDROIDAUTO_APP, CAPPBaseObj::STREAM_MUSIC);
    } else if (enableState == AAEnableState::AASTATE_OFF) {
        connectingDevice = MobileDevice();
    }
}

void ManagerService::AACallback::onAvailableDeviceAdded(MobileDevice device)
{
    ManagerService::getInstance()->mpAAClient->connect(device);
}

void ManagerService::AACallback::onConnectionStateChanged(MobileDevice device, const AAConnectionState connState)
{
    LOGD(TAG, "onConnectionStateChanged deviceId:[%s] state:[%d]", device.getDeviceId().c_str(), int(connState));
    MobileDevice &activeDevice = ManagerService::getInstance()->mActiveDevice;
    MobileDevice &connectingDevice = ManagerService::getInstance()->mConnectingDevice;
    IAndroidAutoClient *aaClient = ManagerService::getInstance()->mpAAClient;

    if (connState == AAConnectionState::AACONNECTED) {
        activeDevice = device;

        if (connectingDevice == device) {
            connectingDevice = MobileDevice();
        }
    } else if (connState == AAConnectionState::AACONNECTING
            || connState == AAConnectionState::AACONNECTING_AUTOMATIC_RESTART) {
        ManagerService::getInstance()->addSupportedConfiguration();

        if (connectingDevice.getUsbId().empty()
                && connectingDevice.getBluetoothId().empty()
                && connectingDevice.getDeviceId().empty()
                && activeDevice.getUsbId().empty()
                && activeDevice.getBluetoothId().empty()
                && activeDevice.getDeviceId().empty()) {
            connectingDevice = device;
            aaClient->confirmConnect(device, true);
        } else {
            aaClient->confirmConnect(device, false);
        }
    } else if (connState == AAConnectionState::AADISCONNECTED) {
        if (device == activeDevice || device.getDeviceId() == activeDevice.getDeviceId()) {
            activeDevice = MobileDevice();
            GlobalBus::applyFor(GlobalBus::ACTION_HIDEFRONT, CAPPBaseObj::APPID_ANDROIDAUTO_APP, 0);
            if (ManagerService::getInstance()->mpSurface != NULL) {
                int err = IAtcSurface_hide(ManagerService::getInstance()->mpSurface);
                if (err < 0) {
                    LOGD(TAG, "IAtcSurface_hide error: %d\n", err);
                }
                LOGD(TAG, "IAtcSurface_release\n");
                IAtcSurface_release(ManagerService::getInstance()->mpSurface);
                ManagerService::getInstance()->mpSurface = NULL;
            }
        }

        if (connectingDevice == device) {
            connectingDevice = MobileDevice();
        }
    }
}

void ManagerService::AACallback::onVideoFocusRequest(MobileDevice device, const AAVideoFocusMode videoRequestFocus){
    LOGD(TAG, "onVideoFocusRequest deviceId:[%s] mode:[%d]", device.getDeviceId().c_str(), int(videoRequestFocus));
    AAVideoFocusMode focusMode = AAVideoFocusMode::AAVIDEO_FOCUS_UNKNOWN;
    IAndroidAutoClient *aaClient = ManagerService::getInstance()->mpAAClient;
    MobileDevice &activeDevice = ManagerService::getInstance()->mActiveDevice;
    IAccessoryInfo *accessoryInfo = ManagerService::getInstance()->mpAccessoryInfo;

    if ((aaClient->getVideoFocusMode(activeDevice, focusMode), AAVideoFocusMode::AAVIDEO_FOCUS_PROJECTED == focusMode)
            && videoRequestFocus == AAVideoFocusMode::AAVIDEO_FOCUS_PROJECTED) {
        aaClient->setVideoFocusMode(activeDevice, AAVideoFocusMode::AAVIDEO_FOCUS_PROJECTED, false);
    } else if (videoRequestFocus == AAVideoFocusMode::AAVIDEO_FOCUS_PROJECTED){
        if (accessoryInfo->isRvcOn()) {
            ManagerService::getInstance()->mIsRvcOn = true;
        } else {
            GlobalBus::applyFor(GlobalBus::ACTION_SHOWFRONT, CAPPBaseObj::APPID_ANDROIDAUTO_APP, 0);
        }
    } else {
        aaClient->setVideoFocusMode(activeDevice, AAVideoFocusMode::AAVIDEO_FOCUS_NATIVE, false);
        GlobalBus::applyFor(GlobalBus::ACTION_HIDEFRONT, CAPPBaseObj::APPID_ANDROIDAUTO_APP, 0);
        if (ManagerService::getInstance()->mpSurface != NULL) {
            int err = IAtcSurface_hide(ManagerService::getInstance()->mpSurface);
            if (err < 0) {
                LOGD(TAG, "IAtcSurface_hide error: %d\n", err);
            }
            LOGD(TAG, "IAtcSurface_release\n");
            IAtcSurface_release(ManagerService::getInstance()->mpSurface);
            ManagerService::getInstance()->mpSurface = NULL;
        }
    }
}

void ManagerService::AACallback::onAudioFocusRequest(MobileDevice device, const AAAudioFocusRequestType audioRequestType){
    LOGD(TAG, "onAudioFocusRequest deviceId:[%s] type:[%d]", device.getDeviceId().c_str(), int(audioRequestType));
    bool isHoldAudioFocus = ManagerService::getInstance()->mIsHoldAudioFocus;
    IAndroidAutoClient *aaClient = ManagerService::getInstance()->mpAAClient;
    MobileDevice &activeDevice = ManagerService::getInstance()->mActiveDevice;
    AAAudioFocusRequestType aaAudioFocusRequestType = ManagerService::getInstance()->mAAAudioFocusRequestType;

    if (device.getDeviceId() != activeDevice.getDeviceId()) {
        return;
    }

    switch (audioRequestType) {
        aaAudioFocusRequestType = audioRequestType;
        case AAAudioFocusRequestType::AAAUDIO_FOCUS_GAIN:{
            if (isHoldAudioFocus) {
                aaClient->setAudioFocusState(activeDevice, AAAudioFocusStateType::AAAUDIO_FOCUS_STATE_GAIN, false);
            } else {
                GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REQ, CAPPBaseObj::APPID_ANDROIDAUTO_APP, CAPPBaseObj::LEVEL_NORMAL | CAPPBaseObj::STREAM_MUSIC);
            }
            break;
        }

        case AAAudioFocusRequestType::AAAUDIO_FOCUS_GAIN_TRANSIENT:{
            if (isHoldAudioFocus) {
                aaClient->setAudioFocusState(activeDevice, AAAudioFocusStateType::AAAUDIO_FOCUS_STATE_GAIN_TRANSIENT, false);
            } else {
                GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REQ, CAPPBaseObj::APPID_ANDROIDAUTO_APP, CAPPBaseObj::LEVEL_TRANSIENT | CAPPBaseObj::STREAM_ASSISTANT);
            }
            break;
        }

        case AAAudioFocusRequestType::AAAUDIO_FOCUS_GAIN_TRANSIENT_MAY_DUCK:{
            aaClient->setAudioFocusState(activeDevice, AAAudioFocusStateType::AAAUDIO_FOCUS_STATE_GAIN_TRANSIENT, false);
            break;
        }

        case AAAudioFocusRequestType::AAAUDIO_FOCUS_RELEASE:{
            ManagerService::getInstance()->mIsAudioFocusTransientLoss = false;
            ManagerService::getInstance()->mIsHoldAudioFocus = false;
            aaClient->setAudioFocusState(activeDevice, AAAudioFocusStateType::AAAUDIO_FOCUS_STATE_LOSS, false);
            GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REL, CAPPBaseObj::APPID_ANDROIDAUTO_APP, CAPPBaseObj::LEVEL_NORMAL | CAPPBaseObj::STREAM_MUSIC);
            GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REL, CAPPBaseObj::APPID_ANDROIDAUTO_APP, CAPPBaseObj::LEVEL_TRANSIENT | CAPPBaseObj::STREAM_ASSISTANT);
            break;
        }

        default:
            break;
    }
}

void ManagerService::AACallback::onNavigationFocusRequest(MobileDevice device, const AANavFocusType navFocusType){
    LOGD(TAG, "onNavigationFocusRequest deviceId:[%s] type:[%d]",  device.toString().c_str(), (int)navFocusType);
    IAndroidAutoClient *aaClient = ManagerService::getInstance()->mpAAClient;
    //if (mpAAClient) {
        //if Hu do not have native navigations, should set projected always.
        //mAAutoManager.setNavigationFocus(mobileDevice, navFocusType);
        aaClient->setNavigationFocus(device, navFocusType);
    //}
}

// void ManagerService::AACallback::onAvailableDeviceAdded(MobileDevice device){
//     LOGD(TAG, "onAvailableDeviceAdded device:[%s]", device.toString().c_str());
// }

void ManagerService::AACallback::onDeviceSelectResolutionCompleted(MobileDevice device, AAVideoSolution resolution){
    LOGD(TAG, "onDeviceSelectResolutionCompleted deviceId:[%s] solution[%d]", device.toString().c_str(), (int)resolution);
    IAndroidAutoClient *aaClient = ManagerService::getInstance()->mpAAClient;

    if (ManagerService::getInstance()->mpSurface == NULL) {
        ManagerService::getInstance()->mpSurface = atc_createsurface(ATCSURF_TYPE_DEFAULT, 1280, 720, ATC_PIX_FMT_NV12M_PRIVATE1);
    }
    if(ManagerService::getInstance()->mpSurface != NULL) {
        IAtcSurface_setCrop(ManagerService::getInstance()->mpSurface, 128, 60, 1024, 600);
        IAtcSurface_setWindow(ManagerService::getInstance()->mpSurface, 0, 0, 1024, 600);
        IAtcSurface_setLayerZOrder(ManagerService::getInstance()->mpSurface, 0);
        IAtcSurface_show(ManagerService::getInstance()->mpSurface);
    } else {
        LOGD(TAG, "create surface fail");
    }
    aaClient->updateSurface(device, ManagerService::getInstance()->mpSurface);
}

void ManagerService::AACallback::onStreamStateChanged(MobileDevice device, const AAStreamType streamType, const AAStreamState streamState)
{
    LOGD(TAG, "onStreamStateChanged streamType:[%d] streamState:[%d]", (int)streamType, (int)streamState);
    if (streamType == AASTREAM_TYPE_AUDIO_MEDIA ) {
        if (streamState == AAStreamState::AASTREAM_STATE_START) {
            GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REQ, CAPPBaseObj::APPID_ANDROIDAUTO_APP, CAPPBaseObj::LEVEL_NORMAL | CAPPBaseObj::STREAM_MUSIC);
        } else if (streamState == AAStreamState::AASTREAM_STATE_STOP && !ManagerService::getInstance()->mIsAudioFocusTransientLoss) {
            ManagerService::getInstance()->mIsHoldAudioFocus = false;
            GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REL, CAPPBaseObj::APPID_ANDROIDAUTO_APP, CAPPBaseObj::LEVEL_NORMAL | CAPPBaseObj::STREAM_MUSIC);
            ManagerService::getInstance()->mpAAClient->setAudioFocusState(
                ManagerService::getInstance()->mActiveDevice, AAAudioFocusStateType::AAAUDIO_FOCUS_STATE_LOSS, true);
        }
    }
    if (streamType == AASTREAM_TYPE_AUDIO_GUIDANCE) {
        if (streamState == AAStreamState::AASTREAM_STATE_START) {
            //ManagerService::getInstance()->mVolumeClient->setDuckControlState(true);
        } else if (streamState == AAStreamState::AASTREAM_STATE_STOP) {
            //ManagerService::getInstance()->mVolumeClient->setDuckControlState(false);
        }
    }
}

void ManagerService::AACallback::onVoiceSessionStatusChanged(MobileDevice device, const AAVoiceSessionStatus voiceSessionStatus)
{
    LOGD(TAG, "onVoiceSessionStatusChanged voiceSessionStatus:[%d]", (int)voiceSessionStatus);
    if (voiceSessionStatus == AAVoiceSessionStatus::AAVOICE_SESSION_START) {
        GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REQ, CAPPBaseObj::APPID_ANDROIDAUTO_APP, CAPPBaseObj::LEVEL_TRANSIENT | CAPPBaseObj::STREAM_ASSISTANT);
    } else if (voiceSessionStatus == AAVoiceSessionStatus::AAVOICE_SESSION_END) {
        ManagerService::getInstance()->mIsAudioFocusTransientLoss = false;
        GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REL, CAPPBaseObj::APPID_ANDROIDAUTO_APP, CAPPBaseObj::LEVEL_TRANSIENT | CAPPBaseObj::STREAM_ASSISTANT);
    }
}

void ManagerService::AccessoryInfoCallback::onRvcStateChanged(bool on)
{
    LOGD(TAG, "%s mIsRvcOn[%d] on[%d]", __func__, ManagerService::getInstance()->mIsRvcOn, on);
    IAndroidAutoClient *aaClient = ManagerService::getInstance()->mpAAClient;
    MobileDevice &activeDevice = ManagerService::getInstance()->mActiveDevice;
    bool &isRvcOn = ManagerService::getInstance()->mIsRvcOn;
    AAConnectionState connectState = AAConnectionState::AAUNKNOWN;
    AAVideoFocusMode focusMode = AAVideoFocusMode::AAVIDEO_FOCUS_UNKNOWN;

    if (on) {
        if (activeDevice.getDeviceId() != ""
                && (aaClient->getConnectionState(activeDevice, connectState), connectState == AAConnectionState::AACONNECTED)
                && (aaClient->getVideoFocusMode(activeDevice, focusMode), AAVideoFocusMode::AAVIDEO_FOCUS_PROJECTED == focusMode)) {
            isRvcOn = true;
            aaClient->setVideoFocusMode(activeDevice, AAVideoFocusMode::AAVIDEO_FOCUS_NATIVE, true);
            GlobalBus::applyFor(GlobalBus::ACTION_HIDEFRONT, CAPPBaseObj::APPID_ANDROIDAUTO_APP, 0);
        }
    } else {
        if (isRvcOn
                && activeDevice.getDeviceId() != ""
                && (aaClient->getConnectionState(activeDevice, connectState), connectState == AAConnectionState::AACONNECTED)) {
            GlobalBus::applyFor(GlobalBus::ACTION_SHOWFRONT, CAPPBaseObj::APPID_ANDROIDAUTO_APP, 0);
        }
        isRvcOn = false;
    }
}
