#ifndef MANAGERSERVICE_H
#define MANAGERSERVICE_H

#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQmlContext>
#include <QElapsedTimer>
#include <memory>
#include <thread>

#include "applog.h"
#include "appobj.h"
#include "atcsurface.h"
#include "qobjlistener.h"
#include "singleton.h"
#include "globalbus.h"
#include "iaccessoryinfo.h"
#include "iandroidautocallback.h"
#include "iandroidautoclient.h"
//#include "VolumeSetting.h"

using vendor::autochips::carlink::IAccessoryInfo;
using vendor::autochips::carlink::IAccessoryInfoCallback;
using vendor::autochips::androidauto::client::IAndroidAutoCallBack;
using vendor::autochips::androidauto::client::IAndroidAutoClient;
using vendor::autochips::androidauto::common::MobileDevice;

class ManagerService
    : public CQObjListener
{
    Q_OBJECT
public:

    virtual ~ManagerService();
    static ManagerService* getInstance();
    // Entry used by AppManager in so loading mode.
    void onStart(soapp_exit_handler exit_handler, void *handle, void *param, QApplication *app, QQmlApplicationEngine *engine);


    Q_INVOKABLE void onUITouched(int action, int pointX, int pointY);
    Q_INVOKABLE void onKeyEvent(bool pressed, int key);

public slots:
    //override CQObjListener
    int doExit (int param1, int param2);
    int doShowFrontUI(void);
    int doShowFront (int param1, int param2);
    int doHideFront (int param1, int param2);
    int doAudioFocusChanged (CCtlListener::E_AVOUT aOut, CCtlListener::E_AUDIOFOCUS focus);

private:
    ManagerService();

    void initBTClient();
    void initCarEventClient();
    void initClient();
    bool hasConnectedDevice();
    void addSupportedConfiguration();

private:
    class AACallback: public IAndroidAutoCallBack {
    public:
        void onEnableStateChanged(const AAEnableState &enableState) override;
        void onAvailableDeviceAdded(MobileDevice device) override;
        void onConnectionStateChanged(MobileDevice device, const AAConnectionState connState) override;
        void onVideoFocusRequest(MobileDevice device, const AAVideoFocusMode videoRequestFocus) override;
        void onAudioFocusRequest(MobileDevice device, const AAAudioFocusRequestType audioRequestType) override;
        //void onAvailableDeviceAdded(MobileDevice device);
        void onNavigationFocusRequest(MobileDevice device, const AANavFocusType navFocusType) override;
        void onDeviceSelectResolutionCompleted(MobileDevice device, AAVideoSolution resolution) override;
        void onStreamStateChanged(MobileDevice device, const AAStreamType streamType, const AAStreamState streamState) override;
        void onVoiceSessionStatusChanged(MobileDevice device, const AAVoiceSessionStatus voiceSessionStatus) override;
    };

    class AccessoryInfoCallback : public IAccessoryInfoCallback {
    public:
        void onRvcStateChanged(bool on) override;
    };

private:
    static std::unique_ptr<ManagerService> mpManagerService;

    QQmlApplicationEngine *mpEngine;
    IAndroidAutoClient *mpAAClient = nullptr;
    IAccessoryInfo* mpAccessoryInfo = nullptr;
    std::unique_ptr<AACallback> mpAACallback = nullptr;
    std::unique_ptr<AccessoryInfoCallback> mpAccessoryCallback = nullptr;
    IAtcSurface* mpSurface = nullptr;
    //VolumeSetting *mVolumeClient;

    MobileDevice mActiveDevice = MobileDevice();
    MobileDevice mConnectingDevice = MobileDevice();

    bool mIsHoldAudioFocus = false;
    bool mIsAudioFocusTransientLoss = false;
    bool mIsRvcOn = false;
    int mPendingLeftRightKey = -1;
    QElapsedTimer mPendingLeftRightKeyTimer;
    AAAudioFocusRequestType mAAAudioFocusRequestType = AAAudioFocusRequestType::AAAUDIO_FOCUS_REQUEST_UNKNOWN;
};


#endif // MANAGERSERVICE_H
