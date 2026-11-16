#ifndef CARPLAYAPP_H
#define CARPLAYAPP_H

#include "atcsurface.h"

#ifdef __cplusplus

#include <mutex>
#include <set>
#include <string>

#include "icarplayclient.h"
#include "icarplaycallback.h"
#include "iaccessoryinfo.h"
#include "iaccessorycommon.h"
#include "audiofocusmanager.h"

using vendor::autochips::carlink::IAccessoryInfo;
using vendor::autochips::carlink::IAccessoryInfoCallback;
using vendor::autochips::carplay::AudioFocusRequestResult;
using vendor::autochips::carplay::AudioFocusType;
using vendor::autochips::carplay::service::ICarplay;
using vendor::autochips::carplay::service::ICarplayClient;
using vendor::autochips::carplay::CarplayAccessoryURLIdentifier;
using vendor::autochips::carplay::CarplayAudioType;
using vendor::autochips::carplay::CarplayBluetoothPairingStopReason;
using vendor::autochips::carplay::CarplayConnectionState;
using vendor::autochips::carplay::CarplayDevice;
using vendor::autochips::carplay::CarplayScreenIndex;
using vendor::autochips::carplay::CarplayStatus;
using vendor::autochips::carplay::CarplayStreamID;
using vendor::autochips::carplay::CarplayStreamState;
using vendor::autochips::carplay::CarplayType;
using vendor::autochips::carplay::CarplayEnableState;
using vendor::autochips::carplay::service::ICarplayCallback;

struct CarplayApp {
    ICarplayClient* carplay_client;
    ICarplay* carplay_manager;
    IAccessoryInfo* accessory_info;

    void* carplay_callback;
    void* accessory_callback;

    IAtcSurface* surface;

    int screen_width;
    int screen_height;
    int cy_type;

    char device_id[18];

    std::mutex* connecting_mtx;
    std::set<std::string>* connecting_devices;
};

class CarplayCallback : public ICarplayCallback {
public:
    CarplayCallback(CarplayApp* outer);
    static void on_carplay_focus_changed(const audio_focus_change_info_t* info, void* user_data);

    void onControllerStreamStateChanged(const std::string& deviceId, CarplayStreamID streamId,
        CarplayAudioType audioType, CarplayStreamState streamState) override;
    void onReceivedDuckCommand(double durationsSecs, double volume) override;
    void onReceivedUnduckCommand(double durationSecs) override;
    void onConnectionStateChanged(const std::string& deviceId, CarplayConnectionState state, int reason) override;
    void onRequestAccessoryUI(const std::string& deviceId, CarplayAccessoryURLIdentifier url) override;
    void onDisableBluetooth(const std::string& deviceId) override;
    void onConnectClassicBluetooth(const std::string& deviceId) override;
    void onAbandonAudioFocus(CarplayType cyType) override;
    void onAudioFocusRequest(const std::string& deviceId, CarplayType cyType, AudioFocusType focusType) override;
    void onBluetoothPairingStatus(const std::string& deviceId, bool isStarted, CarplayBluetoothPairingStopReason stopReason) override;
    void onMainBufferedAudioSetRate(const std::string& deviceId, int rate) override;
    void onCallbackRegistered() override;
    void onServiceDied() override;

    void onWirelessAvailableDeviceAdded(const std::string& deviceId, const std::string& name) override;
    void onEnableStateChanged(CarplayEnableState preState, CarplayEnableState newState) override;
    void onFirstConnectRequest(const std::string& deviceId) override;
    void onPassiveConnectRequest(const std::string& deviceId) override;
    void onDeviceBondStateChanged(const std::string& deviceId, int bondState) override;
    void onDeviceNameUpdate(const std::string& deviceId, const std::string& name) override;

private:
    CarplayApp* mOuter;
};

class AccessoryInfoCallback : public IAccessoryInfoCallback {
public:
    AccessoryInfoCallback(CarplayApp* outer);
    void setExpectedEnableType(int type);
    void onBluetoothScanModeChanged(AccessoryBluetoothScanMode mode) override;
    void onClientStateChanged(std::string stationMacAddr, bool connected, uint32_t reason) override;
    void onAccessoryReady(int type, AccessoryStage stage, AccessoryStatus status) override;

private:
    CarplayApp* mOuter;
    int mExpectedEnableType;
    int mPendingEnableType;
};

#endif

#endif /* CARPLAYAPP_H */
