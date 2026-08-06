#include "managerservice.h"
#include "carplaydevice.h"
#include "carplaycommon.h"
#include <algorithm>
#include <chrono>
#include <mutex>

using vendor::autochips::carlink::AccessoryCall;
using vendor::autochips::carplay::AudioFocusRequestResult;
using vendor::autochips::carplay::AudioFocusType;
using vendor::autochips::carplay::CarplayAccessoryURLIdentifier;
using vendor::autochips::carplay::CarplayDevice;
using vendor::autochips::carplay::CarplayDeviceURLIdentifier;
using vendor::autochips::carplay::CarplayStatus;
using vendor::autochips::carplay::CarplayScreenIndex;

static const char constexpr *TAG = "[CarplayApp]ManagerService";
static std::mutex startVideoMtx;

ManagerService::ManagerService()
    : CQObjListener(CAPPBaseObj::APPID_CARPLAY_APP)
{
}

ManagerService::~ManagerService()
{
    if (nullptr != mpCarplayClient) {
        mpCarplayClient->unregisterCallback(*mpCarplayCallback);
        vendor::autochips::carplay::service::releaseCarplayClient();
    }

    if (nullptr != mpAccessoryInfo) {
        mpAccessoryInfo->unregisterCallback();
        vendor::autochips::carlink::releaseAccessoryInfoClient();
    }

    if (mpSurface != NULL) {
        int err = IAtcSurface_hide(mpSurface);
        if (err < 0) {
            LOGD(TAG, "IAtcSurface_hide error: %d\n", err);
        }
        LOGD(TAG, "IAtcSurface_release\n");
        IAtcSurface_release(mpSurface);
        mpSurface = NULL;
    }
    if (mThreadCheckStream.joinable()) {
        mCheckStreamRunning = false;
        mThreadCheckStream.join();
    }
}

ManagerService* ManagerService::getInstance()
{
    static std::unique_ptr<ManagerService> pManagerService = std::unique_ptr<ManagerService>(new ManagerService());
    return pManagerService.get();
}

void ManagerService::onStart(soapp_exit_handler exit_handler, void *handle, void *param, QApplication *app, QQmlApplicationEngine *engine)
{
    mEngine = engine;
    initScreenValue();
    mEngine->load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    QObject *topLevel = mEngine->rootObjects().value(0);
    QQuickWindow *appWindow = qobject_cast<QQuickWindow *>(topLevel);
    appWindow->setFlags(appWindow->flags() | Qt::CoverWindow);
    if (!appWindow) {
        LOGE(TAG, "appWindow is null!");
    }
    bool ret = initListener(appWindow, exit_handler, handle, param);
    LOGD(TAG, "initListener ret = %s", ret ? "success" : "failed");

    QQmlContext *ctx = mEngine->rootContext();
    ctx->setContextProperty("managerservice", this);
    initClient();
    initCarplayStatus();
    mThreadCheckStream = std::thread(&ManagerService::periodicCheckStream, this);
    GlobalBus::applyFor(GlobalBus::ACTION_HIDEFRONT, CAPPBaseObj::APPID_CARPLAY_APP, 0);
}


void ManagerService::initClient()
{
    CarplayStatus ret;

    mpCarplayClient = vendor::autochips::carplay::service::getCarplayClient();
    if (nullptr == mpCarplayClient) {
        LOGE(TAG, "get ICarplayClient fail");
    }
    ret = mpCarplayClient->getCarplayManager(&mpCarplayManager);
    if (ret != CarplayStatus::CARPLAY_OK || nullptr == mpCarplayManager) {
        LOGE(TAG, "getCarplayManager fail");
    }

    int type = 0;
    type |= (int)AccessoryFunctionType::FUNCTION_TYPE_BT_GAP;
    type |= (int)AccessoryFunctionType::FUNCTION_TYPE_BT_AVRCP;
    type |= (int)AccessoryFunctionType::FUNCTION_TYPE_BT_HFP;
    type |= (int)AccessoryFunctionType::FUNCTION_TYPE_BT_A2DP;
    type |= (int)AccessoryFunctionType::FUNCTION_TYPE_RVC;
    mpAccessoryInfo = vendor::autochips::carlink::getAccessoryInfoClient();
    mpAccessoryCallback = std::unique_ptr<AccessoryInfoCallback>(new AccessoryInfoCallback());
    mpAccessoryInfo->enable(type);
    mpAccessoryInfo->registerCallback(*mpAccessoryCallback, type);

    mpCarplayCallback = std::unique_ptr<CarplayCallback>(new CarplayCallback(this));
    ret = mpCarplayClient->registerCallback(*mpCarplayCallback);
    if (ret != CarplayStatus::CARPLAY_OK) {
        LOGE(TAG, "register carplay callback fail");
    }

    //mVolumeClient = VolumeSetting::getInstance();
}

void ManagerService::initCarplayStatus()
{
    // vector<BluetoothHfClientCall> hfpCalls;
    // mpHfpInterface->getCall(hfpCalls);
    // CarEventInfo carEventInfo;
    // if (!mpCarEventMonitor->getCarEventInfo(carEventInfo)) {
    //     LOGE(TAG, "getCarEventInfo fail");
    // }
    // if (hfpCalls.size() > 0) {
    //     //todo
    //     //mpCarplayManager->notifyAccessoryPhoneCallStart();
    // }
    // if (carEventInfo.gearBoxInfo.gearBoxMode == GEARBOX_MODE_R) {
    //     //todo
    //     //mpCarplayManager->notifyAccessoryRvcStart();
    // }
}

void ManagerService::initScreenValue()
{
    mScreenWidth = 800;
    mScreenHeight = 360;
    //atc_getScreenResolution(&mScreenWidth, &mScreenHeight);
    LOGD(TAG, "screen width[%d]  screen height[%d]", mScreenWidth, mScreenHeight);
}

bool ManagerService::isCarplayShowLimit()
{
    bool ret = false;
    if (mpAccessoryInfo->isRvcOn()) {
        LOGD(TAG, "in rvc state");
        ret = true;
    }
    std::vector<AccessoryCall> accessorycalls = mpAccessoryInfo->getCurrentCalls();
    auto it = std::find_if(accessorycalls.begin(), accessorycalls.end(), [](AccessoryCall& call){
        return call.getCallState() == AccessoryCallState::ACCESSORY_CALL_STATE_OFFHOOK;
    });
    if (it != accessorycalls.end()) {
        LOGD(TAG, "in call state");
        ret = true;
    }


    return ret;
}

void ManagerService::periodicCheckStream() {
    while (true) {
        for (int i = 0; i < 100 && mCheckStreamRunning; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        vector<CarplayDevice> carplayDevices;
        CarplayStreamState streamState = CarplayStreamState::STREAM_STATE_STOPPED;
        mpCarplayManager->getConnectedDevices(carplayDevices);
        if (carplayDevices.size() <= 0) {
            if (mIsShowFront) {
                LOGD(TAG, "periodicCheckStream: no device");
                GlobalBus::applyFor(GlobalBus::ACTION_HIDEFRONT, CAPPBaseObj::APPID_CARPLAY_APP, 0);
            }
        } else {
            mpCarplayManager->getStreamState(carplayDevices[0].getDeviceId(), CarplayStreamID::STREAM_ID_MAIN_SCREEN,
                        CarplayAudioType::AUDIO_TYPE_DEFAULT, streamState);
            if (streamState != CarplayStreamState::STREAM_STATE_STARTED && mIsShowFront) {
                LOGD(TAG, "periodicCheckStream: no stream");
                GlobalBus::applyFor(GlobalBus::ACTION_HIDEFRONT, CAPPBaseObj::APPID_CARPLAY_APP, 0);
            }
        }
    }
}

unsigned int ManagerService::focusTypeConvert(AudioFocusType focusType)
{
    unsigned int ret = 0;
    switch (focusType) {
        case AudioFocusType::AUDIOFOCUS_GAIN_TRANSIENT:
        case AudioFocusType::AUDIOFOCUS_LOSS_TRANSIENT:
            ret = CAPPBaseObj::LEVEL_TRANSIENT;
            break;
        case AudioFocusType::AUDIOFOCUS_GAIN:
        case AudioFocusType::AUDIOFOCUS_LOSS:
            ret = CAPPBaseObj::LEVEL_NORMAL;
            break;
    }
    return ret;
}

void ManagerService::startCarplayVideo() {
    std::lock_guard<std::mutex> lock(startVideoMtx);
    vector<CarplayDevice> carplayDevices;
    mpCarplayManager->getConnectedDevices(carplayDevices);
    if (mpSurface == NULL) {
        mpSurface = atc_createsurface(ATCSURF_TYPE_DEFAULT, mScreenWidth, mScreenHeight, ATC_PIX_FMT_NV12M_PRIVATE1);
        if(mpSurface != NULL) {
            IAtcSurface_setWindow(mpSurface, 0, 0, mScreenWidth, mScreenHeight);
            IAtcSurface_setLayerZOrder(mpSurface, 3);
            IAtcSurface_show(mpSurface);
            mpCarplayManager->startVideo(carplayDevices[0].getDeviceId(), CarplayScreenIndex::SCREEN_INDEX_MAIN, (void *)mpSurface);
        } else {
            LOGD(TAG, "create surface fail");
        }
    }
}

void ManagerService::onUITouched(bool press, int pointX, int pointY)
{
    vector<CarplayDevice> carplayDevices;
    mpCarplayManager->getConnectedDevices(carplayDevices);
    if (carplayDevices.size() > 0) {
        mpCarplayManager->updateTouchScreen(carplayDevices[0].getDeviceId(), press,
            (pointX * 800.0 / mScreenWidth) + 0.5, (pointY * 480.0 / mScreenHeight) + 0.5);
    }
}

void ManagerService::onKeyEvent(bool pressed, int key)
{
    LOGD(TAG, "%s pressed(%d), key(%d)", __func__, pressed, key);

    vector<CarplayDevice> carplayDevices;
    mpCarplayManager->getConnectedDevices(carplayDevices);
    if (carplayDevices.size() > 0) {
       switch (key) {
           //case Qt::Key_Left: {
           case 52: {
               LOGD(TAG, "%s Left", __func__);
               if (pressed) {
                   mpCarplayManager->updateKnob(carplayDevices[0].getDeviceId(), false, false, false, -90, 0, 0);
               } else {
                   mpCarplayManager->updateKnob(carplayDevices[0].getDeviceId(), false, false, false, 0, 0, 0);
               }
               break;
           }

           //case Qt::Key_Right: {
           case 54: {
               LOGD(TAG, "%s Right", __func__);
               if (pressed) {
                   mpCarplayManager->updateKnob(carplayDevices[0].getDeviceId(), false, false, false, 90, 0, 0);
               } else {
                   mpCarplayManager->updateKnob(carplayDevices[0].getDeviceId(), false, false, false, 0, 0, 0);
               }
               break;
           }

           //case Qt::Key_Up: {
           case 50: {
               LOGD(TAG, "%s Up", __func__);
               if (pressed) {
                   mpCarplayManager->updateKnob(carplayDevices[0].getDeviceId(), false, false, false, 0, -90, 0);
               } else {
                   mpCarplayManager->updateKnob(carplayDevices[0].getDeviceId(), false, false, false, 0, 0, 0);
               }
               break;
           }

           //case Qt::Key_Down: {
           case 53: {
               LOGD(TAG, "%s Down", __func__);
               if (pressed) {
                   mpCarplayManager->updateKnob(carplayDevices[0].getDeviceId(), false, false, false, 0, 90, 0);
               } else {
                   mpCarplayManager->updateKnob(carplayDevices[0].getDeviceId(), false, false, false, 0, 0, 0);
               }
               break;
           }

           //case Qt::Key_Select: {
           case 49: {
               LOGD(TAG, "%s Select", __func__);
               if (pressed) {
                   mpCarplayManager->updateKnob(carplayDevices[0].getDeviceId(), true, false, false, 0, 0, 0);
               } else {
                   mpCarplayManager->updateKnob(carplayDevices[0].getDeviceId(), false, false, false, 0, 0, 0);
               }
               break;
           }

           //case Qt::Key_Back: {
           case 51: {
               LOGD(TAG, "%s Back", __func__);
               if (pressed) {
                   mpCarplayManager->updateKnob(carplayDevices[0].getDeviceId(), false, false, true, 0, 0, 0);
               } else {
                   mpCarplayManager->updateKnob(carplayDevices[0].getDeviceId(), false, false, false, 0, 0, 0);
               }
               break;
           }
       }
    }
}

int ManagerService::getScreenWidth()
{
    return mScreenWidth;
}

int ManagerService::getScreenHeight()
{
    return mScreenHeight;
}

int ManagerService::doShowFrontUI(void)
{
    vector<CarplayDevice> carplayDevices;
    CarplayStreamState streamState = CarplayStreamState::STREAM_STATE_STOPPED;
    mpCarplayManager->getConnectedDevices(carplayDevices);
    if (!isCarplayShowLimit() && carplayDevices.size() > 0) {
        mpCarplayManager->getStreamState(carplayDevices[0].getDeviceId(), CarplayStreamID::STREAM_ID_MAIN_SCREEN,
                    CarplayAudioType::AUDIO_TYPE_DEFAULT, streamState);
        if (streamState == CarplayStreamState::STREAM_STATE_STARTED) {
            mIsShowFront = true;
            LOGD(TAG, "doShowFrontUI");
            CQObjListener::doShowFrontUI();
        } else {
            GlobalBus::applyFor(GlobalBus::ACTION_HIDEFRONT, CAPPBaseObj::APPID_CARPLAY_APP, 0);
            mpCarplayManager->requestDeviceUI(carplayDevices[0].getDeviceId(), CarplayDeviceURLIdentifier::URL_ID_EMPTY, "");
            startCarplayVideo();
        }
    } else {
        GlobalBus::applyFor(GlobalBus::ACTION_HIDEFRONT, CAPPBaseObj::APPID_CARPLAY_APP, 0);
    }

    return 1;
}

int ManagerService::doShowFront (int param1, int param2)
{
    CQObjListener::doShowFront(param1, param2);

    return 1;
}

int ManagerService::doHideFront (int param1, int param2)
{
    LOGD(TAG, "doHideFront");
    mIsShowFront = false;
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
        case CCtlListener::AUDIOFOCUS_NONE:
            mpCarplayManager->audioFocusChanged(AudioFocusType::AUDIOFOCUS_NONE);
            break;

        case CCtlListener::AUDIOFOCUS_LOSS_TRANSIENT:
            //mIsHoldAudioFocus = false;
            mpCarplayManager->audioFocusChanged(AudioFocusType::AUDIOFOCUS_LOSS_TRANSIENT);
            mIsAudioFocusTransientLoss = true;
            break;

        case CCtlListener::AUDIOFOCUS_LOSS:
            //mIsHoldAudioFocus = false;
            mpCarplayManager->audioFocusChanged(AudioFocusType::AUDIOFOCUS_LOSS);
            mIsAudioFocusTransientLoss = false;
            break;

        case CCtlListener::AUDIOFOCUS_GAIN:
            //mIsHoldAudioFocus = true;
            mpCarplayManager->audioFocusChanged(AudioFocusType::AUDIOFOCUS_GAIN);
            //mIsAudioFocusTransientLoss = false;
            if (mIsAudioFocusRequestResult) {
                mpCarplayManager->setAudioFocusResponse(mIdAudioFocusRequestDevice, AudioFocusRequestResult::AUDIOFOCUS_REQUEST_GRANTED);
                mIsAudioFocusRequestResult = false;
            }
            break;

        case CCtlListener::AUDIOFOCUS_GAIN_TRANSIENT:
            mpCarplayManager->audioFocusChanged(AudioFocusType::AUDIOFOCUS_GAIN_TRANSIENT);
            if (mIsAudioFocusRequestResult) {
                mpCarplayManager->setAudioFocusResponse(mIdAudioFocusRequestDevice, AudioFocusRequestResult::AUDIOFOCUS_REQUEST_GRANTED);
                mIsAudioFocusRequestResult = false;
            }
            break;

        case CCtlListener::AUDIOFCOUS_REQUEST_FAILED:
            if (mIsAudioFocusRequestResult) {
                mpCarplayManager->setAudioFocusResponse(mIdAudioFocusRequestDevice, AudioFocusRequestResult::AUDIOFOCUS_REQUEST_FAILED);
                mIsAudioFocusRequestResult = false;
            }
            break;
    }

    return 1;
}

void ManagerService::CarplayCallback::onControllerStreamStateChanged(const std::string &deviceId, CarplayStreamID streamId,
        CarplayAudioType audioType, CarplayStreamState streamState)
{
    LOGD(TAG, "onControllerStreamStateChanged deviceId:[%s] streamId:[%d] audioType[%d] streamState[%d]",
            deviceId.c_str(), (int)streamId, (int)audioType, (int)streamState);
    int width = mOuter->mScreenWidth;
    int height = mOuter->mScreenHeight;
    switch (streamId) {
        case CarplayStreamID::STREAM_ID_MAIN_SCREEN:
            {
                if (streamState == CarplayStreamState::STREAM_STATE_STARTED) {
                    GlobalBus::applyFor(GlobalBus::ACTION_SHOWFRONT, CAPPBaseObj::APPID_CARPLAY_APP, 0);
                    mOuter->startCarplayVideo();
                } else if (streamState == CarplayStreamState::STREAM_STATE_STOPPED) {
                    GlobalBus::applyFor(GlobalBus::ACTION_HIDEFRONT, CAPPBaseObj::APPID_CARPLAY_APP, 0);
                    if (mOuter->mpSurface != NULL) {
                        IAtcSurface_setLayerZOrder(mOuter->mpSurface, 0);
                        int err = IAtcSurface_hide(mOuter->mpSurface);
                        if (err < 0) {
                            LOGD(TAG, "IAtcSurface_hide error: %d\n", err);
                        }
                        LOGD(TAG, "IAtcSurface_release\n");
                        IAtcSurface_release(mOuter->mpSurface);
                        mOuter->mpSurface = NULL;
                    }
                    mOuter->mpCarplayManager->stopVideo(deviceId, CarplayScreenIndex::SCREEN_INDEX_MAIN, false);
                }
            }
            break;

        default:
            break;
    }
}

void ManagerService::CarplayCallback::onConnectionStateChanged(const std::string &deviceId, CarplayConnectionState state, int reason)
{
    LOGD(TAG, "onConnectionStateChanged deviceId:[%s] state[%d] reason[%d]",
        deviceId.c_str(), (int)state, (int)reason);
    switch (state) {
        case CarplayConnectionState::CONNECTION_STATE_DISCONNECTED:
        {
            vector<CarplayDevice> deviceList;
            mOuter->mpCarplayManager->getConnectedDevices(deviceList);
            if (deviceList.size() <= 0) {
                GlobalBus::applyFor(GlobalBus::ACTION_HIDEFRONT, CAPPBaseObj::APPID_CARPLAY_APP, 0);
                if (mOuter->mpSurface != NULL) {
                    IAtcSurface_setLayerZOrder(mOuter->mpSurface, 0);
                    int err = IAtcSurface_hide(mOuter->mpSurface);
                    if (err < 0) {
                        LOGD(TAG, "IAtcSurface_hide error: %d\n", err);
                    }
                    LOGD(TAG, "IAtcSurface_release\n");
                    IAtcSurface_release(mOuter->mpSurface);
                    mOuter->mpSurface = NULL;
                }
            }
        }
        break;
    }
}

void ManagerService::CarplayCallback::onReceivedDuckCommand(double durationsSecs, double volume)
{
    LOGD(TAG, "onReceivedDuckCommand");
    //mOuter->mVolumeClient->setDuckControlState(true);
}

void ManagerService::CarplayCallback::onReceivedUnduckCommand(double durationsSecs)
{
    LOGD(TAG, "onReceivedUnduckCommand");
    //mOuter->mVolumeClient->setDuckControlState(false);
}

void ManagerService::CarplayCallback::onAudioFocusRequest(const std::string &deviceId, CarplayType cyType, AudioFocusType focusType)
{
    LOGD(TAG, "onAudioFocusRequest CarplayType:[%d] focusType[%d]", (int)cyType, (int)focusType);
    mOuter->mIsAudioFocusRequestResult = true;
    mOuter->mIdAudioFocusRequestDevice = deviceId;
    switch (cyType) {
        case CarplayType::CARPLAY_TELEPHONY:
            GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REQ, CAPPBaseObj::APPID_CARPLAY_APP,
                mOuter->focusTypeConvert(focusType) | CAPPBaseObj::STREAM_VOICL_CALL);
            break;
        case CarplayType::CARPLAY_SPEECHRECOGNIZE:
            GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REQ, CAPPBaseObj::APPID_CARPLAY_APP,
                mOuter->focusTypeConvert(focusType) | CAPPBaseObj::STREAM_ASSISTANT);
            break;
        case CarplayType::CARPLAY_ALERT:
            GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REQ, CAPPBaseObj::APPID_CARPLAY_APP,
                mOuter->focusTypeConvert(focusType) | CAPPBaseObj::STRAEM_NOTIFICATION_RINGTONE);
            break;
        case CarplayType::CARPLAY_MUSIC:
        case CarplayType::CARPLAY_CAPATIBILITY:
        case CarplayType::CARPLAY_DEFAULT:
            GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REQ, CAPPBaseObj::APPID_CARPLAY_APP,
                mOuter->focusTypeConvert(focusType) | CAPPBaseObj::STREAM_MUSIC);
            break;
    }
}

void ManagerService::CarplayCallback::onAbandonAudioFocus(CarplayType cyType)
{
    LOGD(TAG, "onAbandonAudioFocus CarplayType:[%d]", (int)cyType);

    switch (cyType) {
        case CarplayType::CARPLAY_TELEPHONY:
            GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REL, CAPPBaseObj::APPID_CARPLAY_APP, CAPPBaseObj::STREAM_VOICL_CALL);
            break;
        case CarplayType::CARPLAY_SPEECHRECOGNIZE:
            GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REL, CAPPBaseObj::APPID_CARPLAY_APP, CAPPBaseObj::STREAM_ASSISTANT);
            break;
        case CarplayType::CARPLAY_ALERT:
            GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REL, CAPPBaseObj::APPID_CARPLAY_APP, CAPPBaseObj::STRAEM_NOTIFICATION_RINGTONE);
            break;
        case CarplayType::CARPLAY_MUSIC:
        case CarplayType::CARPLAY_CAPATIBILITY:
        case CarplayType::CARPLAY_DEFAULT:
            GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REL, CAPPBaseObj::APPID_CARPLAY_APP, CAPPBaseObj::STREAM_MUSIC);
            break;
    }
}

void ManagerService::CarplayCallback::onRequestAccessoryUI(const std::string &deviceId, CarplayAccessoryURLIdentifier url)
{
    LOGD(TAG, "onRequestAccessoryUI deviceId:[%s] url[%d]", deviceId.c_str(), (int)url);
    if (url == CarplayAccessoryURLIdentifier ::URL_ID_EMPTY || url == CarplayAccessoryURLIdentifier::URL_ID_OEM_BACK) {
        GlobalBus::applyFor(GlobalBus::ACTION_HIDEFRONT, CAPPBaseObj::APPID_CARPLAY_APP, 0);
        mOuter->mpCarplayManager->stopVideo(deviceId, CarplayScreenIndex::SCREEN_INDEX_MAIN, true);
    }
}

void ManagerService::CarplayCallback::onDisableBluetooth(const std::string &deviceId)
{
    LOGD(TAG, "onDisableBluetooth");
    mOuter->mpAccessoryInfo->disconnectBluetoothProfile(deviceId, AccessoryBluetoothProfile::ACCESSORY_BLUETOOTH_PROFILE_A2DP);
    mOuter->mpAccessoryInfo->disconnectBluetoothProfile(deviceId, AccessoryBluetoothProfile::ACCESSORY_BLUETOOTH_PROFILE_HFP);
}

void ManagerService::CarplayCallback::onConnectClassicBluetooth(const std::string &deviceId)
{
    LOGD(TAG, "onConnectClassicBluetooth");
    mOuter->mpAccessoryInfo->connectClassicBluetooth(deviceId);
}

void ManagerService::CarplayCallback::onBluetoothPairingStatus(const std::string &deviceId, bool isStarted, CarplayBluetoothPairingStopReason stopReason)
{
    LOGD(TAG, "onBluetoothPairingStatus deviceId:[%s] isStarted[%d] stopReason[%d]", deviceId.c_str(), (int)isStarted, (int)stopReason);
    if (isStarted)
        mOuter->mpCarplayManager->confirmOOBPairing(deviceId, true);
}

void ManagerService::CarplayCallback::onCallbackRegistered()
{
    LOGD(TAG, "onCallbackRegistered");
    vector<CarplayDevice> carplayDevices;
    CarplayStreamState streamState = CarplayStreamState::STREAM_STATE_STOPPED;
    mOuter->mpCarplayManager->getConnectedDevices(carplayDevices);
    if (!mOuter->isCarplayShowLimit() && carplayDevices.size() > 0) {
        mOuter->mpAccessoryInfo->disconnectBluetoothProfile(carplayDevices[0].getDeviceId(), AccessoryBluetoothProfile::ACCESSORY_BLUETOOTH_PROFILE_A2DP);
        mOuter->mpAccessoryInfo->disconnectBluetoothProfile(carplayDevices[0].getDeviceId(), AccessoryBluetoothProfile::ACCESSORY_BLUETOOTH_PROFILE_HFP);
        mOuter->mpCarplayManager->getStreamState(carplayDevices[0].getDeviceId(), CarplayStreamID::STREAM_ID_MAIN_SCREEN,
                    CarplayAudioType::AUDIO_TYPE_DEFAULT, streamState);
        if (streamState == CarplayStreamState::STREAM_STATE_STARTED) {
            GlobalBus::applyFor(GlobalBus::ACTION_SHOWFRONT, CAPPBaseObj::APPID_CARPLAY_APP, 0);
            mOuter->startCarplayVideo();
        }
    }
}

ManagerService::CarplayCallback::CarplayCallback(ManagerService* outer)
{
    mOuter = outer;
}

void ManagerService::CarplayCallback::onServiceDied()
{
    LOGD(TAG, "onServiceDied");
    GlobalBus::applyFor(GlobalBus::ACTION_HIDEFRONT, CAPPBaseObj::APPID_CARPLAY_APP, 0);
    GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REL, CAPPBaseObj::APPID_CARPLAY_APP, CAPPBaseObj::STREAM_VOICL_CALL);
    GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REL, CAPPBaseObj::APPID_CARPLAY_APP, CAPPBaseObj::STREAM_ASSISTANT);
    GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REL, CAPPBaseObj::APPID_CARPLAY_APP, CAPPBaseObj::STRAEM_NOTIFICATION_RINGTONE);
    GlobalBus::applyFor(GlobalBus::ACTION_AUDIO_REL, CAPPBaseObj::APPID_CARPLAY_APP, CAPPBaseObj::STREAM_MUSIC);
    if (mOuter->mpSurface != NULL) {
        IAtcSurface_setLayerZOrder(mOuter->mpSurface, 0);
        int err = IAtcSurface_hide(mOuter->mpSurface);
        if (err < 0) {
            LOGD(TAG, "IAtcSurface_hide error: %d\n", err);
        }
        LOGD(TAG, "IAtcSurface_release\n");
        IAtcSurface_release(mOuter->mpSurface);
        mOuter->mpSurface = NULL;
    }
}

