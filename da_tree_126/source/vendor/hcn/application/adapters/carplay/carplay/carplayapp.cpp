#include "carplayapp.h"
#include "carplayapp_if.h"
#include "carplayapp_ui.h"
#include "carplaysetting_ui.h"
#include "cp_event_queue.h"

#include <string.h>
#include <stdio.h>

#include "videofocusmanager.h"
#include "audiofocusmanager.h"

#include "policyEvent.h"

using vendor::autochips::carplay::CarplayBondState;
using vendor::autochips::carplay::CarplayConnectFailReason;
using vendor::autochips::carplay::CarplayDisconnectReason;

static void sendAudioPolicyEvent(int cy_type, bool start)
{
    lv_log("%s(cy_type:%d, start:%d)", __func__, cy_type, start);
    switch ((CarplayType)cy_type) {
        case CarplayType::CARPLAY_TELEPHONY:
            send_policy_event(TEL_CP_HANDLE, start ? AUDIO_STATE::START : AUDIO_STATE::STOP);
            break;
        case CarplayType::CARPLAY_SPEECHRECOGNIZE:
            send_policy_event(ASSISTANT_CP_HANDLE, start ? AUDIO_STATE::START : AUDIO_STATE::STOP);
            break;
        case CarplayType::CARPLAY_ALERT:
            send_policy_event(RING_CP_HANDLE, start ? AUDIO_STATE::START : AUDIO_STATE::STOP);
            break;
        case CarplayType::CARPLAY_MUSIC:
            send_policy_event(MM_CP_HANDLE, start ? AUDIO_STATE::START : AUDIO_STATE::STOP);
            break;
        default:
            lv_log("Unknown cy_type:%d", cy_type);
            break;
    }
    lv_log("%s end", __func__);
}

CarplayCallback::CarplayCallback(CarplayApp* outer)
{
    mOuter = outer;
}

void CarplayCallback::on_carplay_focus_changed(const audio_focus_change_info_t *info, void *user_data)
{
    lv_log("%s", __func__);

    if (user_data == NULL || ((CarplayApp*)user_data)->carplay_manager == NULL) {
        lv_log("user_data is NULL or carplay_manager is NULL");
        return;
    }

    switch (info->state) {
        case FOCUS_NONE:
            lv_log("FOCUS_NONE");
            ((CarplayApp*)user_data)->carplay_manager->audioFocusChanged(AudioFocusType::AUDIOFOCUS_NONE);
            break;
        case FOCUS_LOST:
            lv_log("FOCUS_LOST");
            ((CarplayApp*)user_data)->carplay_manager->audioFocusChanged(AudioFocusType::AUDIOFOCUS_LOSS);
            break;
        case FOCUS_LOST_TRANSIENT:
            lv_log("FOCUS_LOST_TRANSIENT");
            ((CarplayApp*)user_data)->carplay_manager->audioFocusChanged(AudioFocusType::AUDIOFOCUS_LOSS_TRANSIENT);
            break;
        case FOCUS_GRANTED:
            lv_log("FOCUS_GRANTED");
            ((CarplayApp*)user_data)->carplay_manager->audioFocusChanged(AudioFocusType::AUDIOFOCUS_GAIN);
            sendAudioPolicyEvent(((CarplayApp*)user_data)->cy_type, true);
            break;
        case FOCUS_GRANTED_TRANSIENT:
            lv_log("FOCUS_GRANTED_TRANSIENT");
            ((CarplayApp*)user_data)->carplay_manager->audioFocusChanged(AudioFocusType::AUDIOFOCUS_GAIN_TRANSIENT);
            sendAudioPolicyEvent(((CarplayApp*)user_data)->cy_type, true);
            break;

        default:
            break;
    }
}

void CarplayCallback::onControllerStreamStateChanged(const std::string &deviceId, CarplayStreamID streamId,
        CarplayAudioType audioType, CarplayStreamState streamState)
{
    lv_log("onControllerStreamStateChanged deviceId:[%s] streamId:[%d] audioType[%d] streamState[%d]",
            deviceId.c_str(), (int)streamId, (int)audioType, (int)streamState);

    switch (streamId) {
        case CarplayStreamID::STREAM_ID_MAIN_SCREEN:
            {
                if (streamState == CarplayStreamState::STREAM_STATE_STARTED) {
                    carplayapp_reset_video_stopped();
                    CpEvent event = {};
                    event.type = CP_EVENT_STREAM_STATE_CHANGED;
                    strncpy(event.device_id, deviceId.c_str(), CP_EVENT_DEVICE_ID_MAX_LEN - 1);
                    event.value = (int)streamId;
                    event.value2 = (int)streamState;
                    cp_event_post(&event);
                } else if (streamState == CarplayStreamState::STREAM_STATE_STOPPED) {
                    /* Surface release is handled in the UI thread via
                     * CP_EVENT_STREAM_STATE_CHANGED to avoid races between
                     * this callback thread and carplayapp_start_video/destroy. */
                    CpEvent event = {};
                    event.type = CP_EVENT_STREAM_STATE_CHANGED;
                    strncpy(event.device_id, deviceId.c_str(), CP_EVENT_DEVICE_ID_MAX_LEN - 1);
                    event.value = (int)streamId;
                    event.value2 = (int)streamState;
                    cp_event_post(&event);
                    mOuter->carplay_manager->stopVideo(deviceId, CarplayScreenIndex::SCREEN_INDEX_MAIN, false);
                    carplayapp_signal_video_stopped();
                }
            }
            break;

        default:
            break;
    }
}

void CarplayCallback::onConnectionStateChanged(const std::string &deviceId, CarplayConnectionState state, int reason)
{
    lv_log("onConnectionStateChanged deviceId:[%s] state[%d] reason[%d]", deviceId.c_str(), (int)state, (int)reason);
    switch (state) {
        case CarplayConnectionState::CONNECTION_STATE_CONNECTED:
        {
            strncpy(mOuter->device_id, deviceId.c_str(), 18);
            if (mOuter->connecting_mtx != NULL && mOuter->connecting_devices != NULL) {
                std::lock_guard<std::mutex> lk(*mOuter->connecting_mtx);
                mOuter->connecting_devices->erase(deviceId);
            }
            break;
        }

        case CarplayConnectionState::CONNECTION_STATE_DISCONNECTED:
        {
            if (mOuter->connecting_mtx != NULL && mOuter->connecting_devices != NULL) {
                std::lock_guard<std::mutex> lk(*mOuter->connecting_mtx);
                mOuter->connecting_devices->erase(deviceId);
            }
            /* Surface release is handled in the UI thread via
             * CP_EVENT_CONNECTION_STATE_CHANGED when no devices remain. */

            if (reason > (int)CarplayConnectFailReason::CONNECT_FAIL_REASON_USER_SELECT &&
                reason <= (int)CarplayConnectFailReason::CONNECT_FAIL_REASON_ILLEGAL_SESSION &&
                reason != (int)CarplayConnectFailReason::CONNECT_FAIL_REASON_USER_ACCEPT_CONNECT_TIMEOUT &&
                reason != (int)CarplayConnectFailReason::CONNECT_FAIL_REASON_USER_FORGET &&
                reason != (int)CarplayConnectFailReason::CONNECT_FAIL_REASON_SWITCH_DEVICE) {
                CpEvent event = {};
                event.type = CP_EVENT_CONNECTION_FAIL_REASON;
                strncpy(event.device_id, deviceId.c_str(), CP_EVENT_DEVICE_ID_MAX_LEN - 1);
                event.value = reason;
                cp_event_post(&event);
            }
            break;
        }
    }

    CpEvent event = {};
    event.type = CP_EVENT_CONNECTION_STATE_CHANGED;
    strncpy(event.device_id, deviceId.c_str(), CP_EVENT_DEVICE_ID_MAX_LEN - 1);
    event.value = (int)state;
    cp_event_post(&event);
}

void CarplayCallback::onReceivedDuckCommand(double durationsSecs, double volume)
{
    lv_log("onReceivedDuckCommand");
    (void)durationsSecs;
    (void)volume;

    std::vector<CarplayDevice> devices;
    CarplayStreamState streamState = CarplayStreamState::STREAM_STATE_STOPPED;

    mOuter->carplay_manager->getConnectedDevices(devices);
    mOuter->carplay_manager->getStreamState(devices.at(0).getDeviceId(),
        CarplayStreamID::STREAM_ID_AUX_OUT_AUDIO,
        CarplayAudioType::AUDIO_TYPE_SPEECHRECOGNITION,
        streamState);
    if (streamState == CarplayStreamState::STREAM_STATE_STARTED) {
        lv_log("send_policy_event, ESIRI_CP_HANDLE, start");
        send_policy_event(ESIRI_CP_HANDLE, AUDIO_STATE::START);
        lv_log("send_policy_event, ESIRI_CP_HANDLE, start end");
    } else {
        lv_log("send_policy_event, ALTERNATE_CP_HANDLE, start");
        send_policy_event(ALTERNATE_CP_HANDLE, AUDIO_STATE::START);
        lv_log("send_policy_event, ALTERNATE_CP_HANDLE, start end");
    }
}

void CarplayCallback::onReceivedUnduckCommand(double durationsSecs)
{
    lv_log("onReceivedUnduckCommand");
    (void)durationsSecs;

    std::vector<CarplayDevice> devices;
    CarplayStreamState streamState = CarplayStreamState::STREAM_STATE_STOPPED;

    mOuter->carplay_manager->getConnectedDevices(devices);
    mOuter->carplay_manager->getStreamState(devices.at(0).getDeviceId(),
        CarplayStreamID::STREAM_ID_AUX_OUT_AUDIO,
        CarplayAudioType::AUDIO_TYPE_SPEECHRECOGNITION,
        streamState);
    if (streamState == CarplayStreamState::STREAM_STATE_STARTED) {
        lv_log("send_policy_event, ESIRI_CP_HANDLE, stop");
        send_policy_event(ESIRI_CP_HANDLE, AUDIO_STATE::STOP);
        lv_log("send_policy_event, ESIRI_CP_HANDLE, stop end");
    } else {
        lv_log("send_policy_event, ALTERNATE_CP_HANDLE, stop");
        send_policy_event(ALTERNATE_CP_HANDLE, AUDIO_STATE::STOP);
        lv_log("send_policy_event, ALTERNATE_CP_HANDLE, stop end");
    }
}

void CarplayCallback::onAudioFocusRequest(const std::string &deviceId, CarplayType cyType, AudioFocusType focusType)
{
    lv_log("onAudioFocusRequest CarplayType:[%d] focusType[%d]", (int)cyType, (int)focusType);

    audio_focus_type_t type = FOCUS_MUSIC;
    switch (cyType) {
        case CarplayType::CARPLAY_TELEPHONY:
            mOuter->cy_type = (int)CarplayType::CARPLAY_TELEPHONY;
            type = FOCUS_CALL;
            break;
        case CarplayType::CARPLAY_SPEECHRECOGNIZE:
            mOuter->cy_type = (int)CarplayType::CARPLAY_SPEECHRECOGNIZE;
            type = FOCUS_SPEECH;
            break;
        case CarplayType::CARPLAY_ALERT:
            mOuter->cy_type = (int)CarplayType::CARPLAY_ALERT;
            type = FOCUS_NOTIFICATION;
            break;
        case CarplayType::CARPLAY_MUSIC:
        case CarplayType::CARPLAY_CAPATIBILITY:
        case CarplayType::CARPLAY_DEFAULT:
            mOuter->cy_type = (int)CarplayType::CARPLAY_MUSIC;
            type = FOCUS_MUSIC;
            break;
    }

    audio_focus_state_t state = FOCUS_GRANTED;
    switch (focusType) {
        case AudioFocusType::AUDIOFOCUS_GAIN_TRANSIENT:
            state = FOCUS_GRANTED_TRANSIENT;
            break;
        case AudioFocusType::AUDIOFOCUS_GAIN:
            state = FOCUS_GRANTED;
            break;
    }

    audiofocusmanager_register_focus_callback(APP_CARPLAY, on_carplay_focus_changed, (void *)mOuter);
    audio_focus_request_t request;
    request.id = APP_CARPLAY;
    request.focus_type = type;
    request.state = state;

    int ret = audiofocusmanager_request_focus(&request);
    if (ret == AUDIO_OK) {
        lv_log("success to request audio focus");
        mOuter->carplay_manager->setAudioFocusResponse(deviceId, vendor::autochips::carplay::AudioFocusRequestResult::AUDIOFOCUS_REQUEST_GRANTED);
    } else {
        lv_log("failed to request audio focus, ret: %d", ret);
        mOuter->carplay_manager->setAudioFocusResponse(deviceId, vendor::autochips::carplay::AudioFocusRequestResult::AUDIOFOCUS_REQUEST_FAILED);
    }
}

void CarplayCallback::onAbandonAudioFocus(CarplayType cyType)
{
    lv_log("onAbandonAudioFocus CarplayType:[%d]", (int)cyType);

    switch (cyType) {
        case CarplayType::CARPLAY_TELEPHONY:
            break;
        case CarplayType::CARPLAY_SPEECHRECOGNIZE:
            break;
        case CarplayType::CARPLAY_ALERT:
            break;
        case CarplayType::CARPLAY_MUSIC:
        case CarplayType::CARPLAY_CAPATIBILITY:
        case CarplayType::CARPLAY_DEFAULT:
            break;
    }

    sendAudioPolicyEvent((int)cyType, false);

    if (mOuter->cy_type == (int)CarplayType::CARPLAY_TELEPHONY && cyType == CarplayType::CARPLAY_ALERT) {
        lv_log("ignore abandon audio focus");
    } else {
        lv_log("release audio focus");
        audiofocusmanager_release_focus(APP_CARPLAY);
        mOuter->cy_type = (int)CarplayType::CARPLAY_DEFAULT;
    }
}

void CarplayCallback::onRequestAccessoryUI(const std::string &deviceId, CarplayAccessoryURLIdentifier url)
{
    lv_log("onRequestAccessoryUI deviceId:[%s] url[%d]", deviceId.c_str(), (int)url);
    if (url == CarplayAccessoryURLIdentifier::URL_ID_EMPTY || url == CarplayAccessoryURLIdentifier::URL_ID_OEM_BACK) {
        CpEvent event = {};
        event.type = CP_EVENT_REQUEST_ACCESSORY_UI;
        strncpy(event.device_id, deviceId.c_str(), CP_EVENT_DEVICE_ID_MAX_LEN - 1);
        event.value = (int)url;
        cp_event_post(&event);
        mOuter->carplay_manager->stopVideo(deviceId, CarplayScreenIndex::SCREEN_INDEX_MAIN, true);
    }
}

void CarplayCallback::onDisableBluetooth(const std::string &deviceId)
{
    lv_log("onDisableBluetooth");
    mOuter->accessory_info->disconnectBluetoothProfile(deviceId, AccessoryBluetoothProfile::ACCESSORY_BLUETOOTH_PROFILE_A2DP);
    mOuter->accessory_info->disconnectBluetoothProfile(deviceId, AccessoryBluetoothProfile::ACCESSORY_BLUETOOTH_PROFILE_HFP);
}

void CarplayCallback::onConnectClassicBluetooth(const std::string &deviceId)
{
    lv_log("onConnectClassicBluetooth");
    mOuter->accessory_info->connectClassicBluetooth(deviceId);
}

void CarplayCallback::onBluetoothPairingStatus(const std::string &deviceId, bool isStarted, CarplayBluetoothPairingStopReason stopReason)
{
    lv_log("onBluetoothPairingStatus deviceId:[%s] isStarted[%d] stopReason[%d]", deviceId.c_str(), (int)isStarted, (int)stopReason);
    if (isStarted)
        mOuter->carplay_manager->confirmOOBPairing(deviceId, true);
}

void CarplayCallback::onMainBufferedAudioSetRate(const std::string &deviceId, int rate)
{
    lv_log("onMainBufferedAudioSetRate deviceId:[%s] rate[%d]", deviceId.c_str(), rate);

    if (rate > 0) {
        /* No local action. */
    } else if (rate == 0) {
        /* No local action. */
    }
}

void CarplayCallback::onCallbackRegistered()
{
    lv_log("onCallbackRegistered");
    std::vector<CarplayDevice> carplay_devices;
    CarplayStreamState stream_state = CarplayStreamState::STREAM_STATE_STOPPED;
    mOuter->carplay_manager->getConnectedDevices(carplay_devices);
    if (!carplayapp_is_show_limit(mOuter) && carplay_devices.size() > 0) {
        mOuter->accessory_info->disconnectBluetoothProfile(carplay_devices[0].getDeviceId(),
                    AccessoryBluetoothProfile::ACCESSORY_BLUETOOTH_PROFILE_A2DP);
        mOuter->accessory_info->disconnectBluetoothProfile(carplay_devices[0].getDeviceId(),
                    AccessoryBluetoothProfile::ACCESSORY_BLUETOOTH_PROFILE_HFP);
        mOuter->carplay_manager->getStreamState(carplay_devices[0].getDeviceId(),
                    CarplayStreamID::STREAM_ID_MAIN_SCREEN,
                    CarplayAudioType::AUDIO_TYPE_DEFAULT, stream_state);
        if (stream_state == CarplayStreamState::STREAM_STATE_STARTED) {
            CpEvent event = {};
            event.type = CP_EVENT_CALLBACK_REGISTERED;
            strncpy(event.device_id, carplay_devices[0].getDeviceId().c_str(), CP_EVENT_DEVICE_ID_MAX_LEN - 1);
            cp_event_post(&event);
        }
    }
}

void CarplayCallback::onServiceDied()
{
    lv_log("onServiceDied");
    /* Surface release is handled in the UI thread via CP_EVENT_SERVICE_DIED
     * to avoid races with carplayapp_start_video/destroy. */
    CpEvent event = {};
    event.type = CP_EVENT_SERVICE_DIED;
    cp_event_post(&event);

    if (mOuter->connecting_mtx != NULL && mOuter->connecting_devices != NULL) {
        std::lock_guard<std::mutex> lk(*mOuter->connecting_mtx);
        mOuter->connecting_devices->clear();
    }
}

void CarplayCallback::onWirelessAvailableDeviceAdded(const std::string& deviceId, const std::string& name)
{
    lv_log("%s(%s)", __func__, name.c_str());
    CpEvent event = {};
    event.type = CP_EVENT_AVAILABLE_DEVICE_ADDED;
    strncpy(event.device_id, deviceId.c_str(), CP_EVENT_DEVICE_ID_MAX_LEN - 1);
    strncpy(event.device_name, name.c_str(), CP_EVENT_DEVICE_NAME_MAX_LEN - 1);
    cp_event_post(&event);
}


void CarplayCallback::onEnableStateChanged(CarplayEnableState preState, CarplayEnableState newState)
{
    lv_log("%s(%d)", __func__, (int)newState);
    CpEvent event = {};
    event.type = CP_EVENT_ENABLE_STATE_CHANGED;
    event.value = (int)newState;
    cp_event_post(&event);
}
void CarplayCallback::onFirstConnectRequest(const std::string& deviceId)
{
    lv_log("%s(%s)", __func__, deviceId.c_str());
    CarplayDevice device(deviceId);
    CpEvent event = {};
    event.type = CP_EVENT_FIRST_CONNECT_REQUEST;
    strncpy(event.device_id, deviceId.c_str(), CP_EVENT_DEVICE_ID_MAX_LEN - 1);
    strncpy(event.device_name, device.getName().c_str(), CP_EVENT_DEVICE_NAME_MAX_LEN - 1);
    cp_event_post(&event);
}

void CarplayCallback::onPassiveConnectRequest(const std::string& deviceId)
{
    lv_log("%s(%s)", __func__, deviceId.c_str());
    CarplayDevice device(deviceId);
    CpEvent event = {};
    event.type = CP_EVENT_PASSIVE_CONNECT_REQUEST;
    strncpy(event.device_id, deviceId.c_str(), CP_EVENT_DEVICE_ID_MAX_LEN - 1);
    strncpy(event.device_name, device.getName().c_str(), CP_EVENT_DEVICE_NAME_MAX_LEN - 1);
    cp_event_post(&event);
}

void CarplayCallback::onDeviceBondStateChanged(const std::string& deviceId, int bondState)
{
    lv_log("%s(%s, %d)", __func__, deviceId.c_str(), bondState);
    CarplayDevice device(deviceId);
    CpEvent event = {};
    event.type = CP_EVENT_DEVICE_BOND_STATE_CHANGED;
    strncpy(event.device_id, deviceId.c_str(), CP_EVENT_DEVICE_ID_MAX_LEN - 1);
    strncpy(event.device_name, device.getName().c_str(), CP_EVENT_DEVICE_NAME_MAX_LEN - 1);
    event.value = bondState;
    cp_event_post(&event);
}

void CarplayCallback::onDeviceNameUpdate(const std::string& deviceId, const std::string& name)
{
    lv_log("%s(%s, %s)", __func__, deviceId.c_str(), name.c_str());
    CpEvent event = {};
    event.type = CP_EVENT_DEVICE_NAME_UPDATE;
    strncpy(event.device_id, deviceId.c_str(), CP_EVENT_DEVICE_ID_MAX_LEN - 1);
    strncpy(event.device_name, name.c_str(), CP_EVENT_DEVICE_NAME_MAX_LEN - 1);
    cp_event_post(&event);
}

AccessoryInfoCallback::AccessoryInfoCallback(CarplayApp* outer)
{
    mOuter = outer;
    mExpectedEnableType = 0;
    mPendingEnableType = 0;
}

void AccessoryInfoCallback::setExpectedEnableType(int type)
{
    mExpectedEnableType = type;
    mPendingEnableType = type;
}

void AccessoryInfoCallback::onBluetoothScanModeChanged(AccessoryBluetoothScanMode mode)
{
    lv_log("%s", __func__);
    CpEvent event = {};
    event.type = CP_EVENT_BLUETOOTH_SCAN_MODE_CHANGED;
    event.value = (int)mode;
    cp_event_post(&event);
}

void AccessoryInfoCallback::onAccessoryReady(int type, AccessoryStage stage, AccessoryStatus status)
{
    const char *stageText = (stage == ACCESSORY_STAGE_ENABLE) ? "ENABLE" :
                            (stage == ACCESSORY_STAGE_REGISTER) ? "REGISTER" : "UNKNOWN";
    lv_log("%s(type:0x%x, stage:%s, status:%d)", __func__, type, stageText, (int)status);

    if (stage != ACCESSORY_STAGE_ENABLE) {
        return;
    }

    if (status != ACCESSORY_OK) {
        lv_log("%s enable failed for type:0x%x, skip registerCallbackAsync", __func__, type);
        mPendingEnableType = 0;
        return;
    }

    mPendingEnableType &= ~type;
    if (mPendingEnableType == 0) {
        if (mOuter == NULL || mOuter->accessory_info == NULL) {
            lv_log("%s accessory_info is NULL, cannot registerCallbackAsync", __func__);
            return;
        }
        lv_log("%s all enable ready, registerCallbackAsync(type:0x%x)", __func__, mExpectedEnableType);
        mOuter->accessory_info->registerCallbackAsync(*this, mExpectedEnableType);
    }
}

void AccessoryInfoCallback::onClientStateChanged(string stationMacAddr, bool connected, uint32_t reason)
{
    lv_log("%s, stationMacAddr:%s, connected:%s, reason:%u", __func__, stationMacAddr.c_str(), connected ? "true" : "false", reason);

    string addr = stationMacAddr;
    std::transform(addr.begin(), addr.end(), addr.begin(), [](char c) {
            return std::toupper(c);
    });

    CarplayDevice carplaydevice = CarplayDevice(mOuter->device_id);
    string clientWifiMacAddress = carplaydevice.getClientIfAddr();

    if (carplaydevice.isConnected() && clientWifiMacAddress == addr) {
        if (!connected) {
            if (reason == 8) {
                CpEvent event = {};
                event.type = CP_EVENT_CONNECTION_FAIL_REASON;
                strncpy(event.device_id, mOuter->device_id, CP_EVENT_DEVICE_ID_MAX_LEN - 1);
                event.value = (uint32_t)CarplayDisconnectReason::DISCONNECT_REASON_NET_IFACE_CLIENT_DISCONNECTED;
                cp_event_post(&event);
            }
        }
    }
}
