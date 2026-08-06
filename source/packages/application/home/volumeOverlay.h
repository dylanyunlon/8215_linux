/*
* Copyright (c) 2016 AutoChips Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef CVOLUMEOVERLY_H
#define CVOLUMEOVERLY_H

#include <QQuickWindow>
#include <QObject>
#include <QTimer>
//#include "isettingadapter.h"
//#include "isetting.h"
//#include "isettingcallback.h"
//#include "csync.h"

//volume
#define VOLUMEMEDIA        "media"
#define VOLUMEGIS          "GIS"
#define VOLUMEBT           "BT"
#define VOLUMEMUTE         "mute"

//class CvolumeOverlay : public QObject, public ISettingCallBack
class CvolumeOverlay : public QObject
{
    Q_OBJECT

public:
    CvolumeOverlay(QQuickWindow *window = 0);
    virtual ~CvolumeOverlay();
    virtual int onSettingIndication(std::string cmd, std::string parame);
    bool doKeyEvent(int key, int param1, int param2);
    void initObject();

    typedef enum
    {
        VOLUME_TYPE_MEDIA = 0,
        VOLUME_TYPE_GPS,
        VOLUME_TYPE_BT,
        VOLUME_TYPE_MUTE,
        VOLUME_TYPE_MAX
    }VOLUME_TYPE;

signals:
    void broadcastVolumeValue(int type, int value);
    void showVolume(int type, int value);
    void volumeChange();

public slots:
    void doAutoHideTimeout();
    void doVolumeChanged();

private:
    int creatVolumeMap();
    int getVolume(VOLUME_TYPE type)const;
    bool setVolume(VOLUME_TYPE type, int value);
    void showVolumeOverlay(VOLUME_TYPE type, int value);
    void hideVolumeOverlay();
    bool isVolumeOverlayVisible();
    int stringToInt (const std::string& str)const;
    void startAutoHideTimer();
    void stopAutoHideTimer();
    bool incVolume(VOLUME_TYPE type);
    bool decVolume(VOLUME_TYPE type);
    bool muteUnmute(VOLUME_TYPE type);

private:
    std::map<VOLUME_TYPE, std::string> m_volumeTypeMap;
    QQuickWindow *m_volumeWindow;
    //ISetting *m_settingclient;
    //ISettingAdapter *m_volume;
    QTimer *m_autoHideTimer;
    //CMutexObject m_volumeLock;
};

#endif // CSETVIDEOPAGE_H
