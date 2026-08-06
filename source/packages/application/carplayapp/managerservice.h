#ifndef MANAGERSERVICE_H
#define MANAGERSERVICE_H

#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQmlContext>
#include <memory>
#include <thread>

#include "applog.h"
#include "appobj.h"
#include "atcsurface.h"
#include "globalbus.h"
#include "icarplayclient.h"
#include "iaccessoryinfo.h"
#include "qobjlistener.h"
#include "singleton.h"
//#include "VolumeSetting.h"
#include "icarplaycallback.h"

using vendor::autochips::carlink::IAccessoryInfo;
using vendor::autochips::carlink::IAccessoryInfoCallback;
using vendor::autochips::carplay::AudioFocusType;
using vendor::autochips::carplay::service::ICarplay;
using vendor::autochips::carplay::service::ICarplayClient;
using vendor::autochips::carplay::CarplayAccessoryURLIdentifier;
using vendor::autochips::carplay::CarplayAudioType;
using vendor::autochips::carplay::CarplayBluetoothPairingStopReason;
using vendor::autochips::carplay::CarplayConnectionState;
using vendor::autochips::carplay::CarplayStreamID;
using vendor::autochips::carplay::CarplayStreamState;
using vendor::autochips::carplay::CarplayType;
using vendor::autochips::carplay::service::ICarplayCallback;


class ManagerService
    : public CQObjListener
{
    Q_OBJECT
public:

    virtual ~ManagerService();
    static ManagerService* getInstance();
    ManagerService(const ManagerService&) = delete;
    ManagerService& operator=(const ManagerService&) = delete;
    void onStart(soapp_exit_handler exit_handler, void *handle, void *param, QApplication *app, QQmlApplicationEngine *engine);


    Q_INVOKABLE void onUITouched(bool press, int pointX, int pointY);
    Q_INVOKABLE void onKeyEvent(bool pressed, int key);
    Q_INVOKABLE int getScreenWidth();
    Q_INVOKABLE int getScreenHeight();

public slots:
    //override CQObjListener
    int doExit (int param1, int param2);
    int doShowFrontUI(void);
    int doShowFront (int param1, int param2);
    int doHideFront (int param1, int param2);
    int doAudioFocusChanged (CCtlListener::E_AVOUT aOut, CCtlListener::E_AUDIOFOCUS focus);

private:
    ManagerService();

    void initClient();
    void initCarplayStatus();
    bool isCarplayShowLimit();
    void periodicCheckStream();
    void initScreenValue();
    unsigned int focusTypeConvert(AudioFocusType focusType);
    void startCarplayVideo();

private:
    class CarplayCallback : public ICarplayCallback {
    private:
        ManagerService* mOuter;
    public:
        CarplayCallback(ManagerService* outer);
        void onServiceDied() override;
        void onControllerStreamStateChanged(const std::string &deviceId, CarplayStreamID streamId,
            CarplayAudioType audioType, CarplayStreamState streamState) override;
        void onReceivedDuckCommand(double durationsSecs, double volume) override;
        void onReceivedUnduckCommand(double durationSecs) override;
        void onConnectionStateChanged(const std::string &deviceId, CarplayConnectionState state, int reason) override;
        void onRequestAccessoryUI(const std::string &deviceId, CarplayAccessoryURLIdentifier url) override;
        void onDisableBluetooth(const std::string &deviceId) override;
        void onConnectClassicBluetooth(const std::string &deviceId) override;
        void onAbandonAudioFocus(CarplayType cyType) override;
        void onAudioFocusRequest(const std::string &deviceId, CarplayType cyType, AudioFocusType focusType) override;
        void onBluetoothPairingStatus(const std::string &deviceId, bool isStarted, CarplayBluetoothPairingStopReason stopReason) override;
        void onCallbackRegistered() override;
    };
    class AccessoryInfoCallback : public IAccessoryInfoCallback {
    public:
        // void onBluetoothStateChanged(AccessoryBluetoothState state) override;
        // void onWifiAPStateChanged(AccessoryWifiAState state) override;
        // void onClientStateChanged(string stationMacAddr, bool connected, uint32_t reason) override;
    };

private:
    QQmlApplicationEngine *mEngine;
    std::thread mThreadCheckStream;
    ICarplayClient *mpCarplayClient = nullptr;
    ICarplay *mpCarplayManager = nullptr;
    IAccessoryInfo* mpAccessoryInfo = nullptr;
    std::unique_ptr<CarplayCallback> mpCarplayCallback = nullptr;
    std::unique_ptr<AccessoryInfoCallback> mpAccessoryCallback = nullptr;
    IAtcSurface *mpSurface = nullptr;
    //VolumeSetting *mVolumeClient;

    bool mIsAudioFocusTransientLoss = false;
    //bool mIsHoldAudioFocus = false;
    bool mCheckStreamRunning = true;
    bool mIsAudioFocusRequestResult = false;
    std::string mIdAudioFocusRequestDevice = "";
    int mScreenWidth = 0;
    int mScreenHeight = 0;
    bool mIsShowFront = false;
};


#endif // MANAGERSERVICE_H
