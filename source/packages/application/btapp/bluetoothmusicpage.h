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
 
#ifndef BLUETOOTHMUSICPAGE_H
#define BLUETOOTHMUSICPAGE_H

#include <QObject>

#include "applog.h"
#include "bluetoothapi.h"
#include "bluetoothavrcp.h"
#include "bluetoothavrcpcallback.h"

#define STRING_DEFAULTID3 CBluetoothMusicPage::trUtf8("Unknown")
#define STRING_UNKNOWN    "Unknown"

class CBluetoothMusicPage : public QObject
{
    Q_OBJECT

public:
    CBluetoothMusicPage();
    ~CBluetoothMusicPage();
    void initBluetoothMusicPage();
    void getAVRCPInterface(IBluetoothAvrcp *avrcpInterface);

signals:
    void id3InfoChanged(QString m_title, QString m_artist, QString m_album);
    void totalTimeChanged(int m_totalTime);
    void currentTimeChanged(int m_currentTime);
    void a2dpStateChanged(bool m_a2dpState);
    void avrcpStateChanged(bool m_avrcpState);
    void musicStateChanged(bool m_musicState);
    void sigA2DPConnectRequest();
    void sigAVRCPConnectRequest();

public slots:
    QString getTitle();
    QString getArtist();
    QString getAlbum();
    bool getA2dpState();
    bool getAvrcpState();
    bool getMusicState();
    int getCurrentTime();
    int getTotalTime();

    void checkMediaAudioConnectState();
    void updatePlayStatusRequest();
    void musicPreviousRequest();
    void musicPausePlayRequest();
    void musicNextRequest();

    void doPlaybackDateUpdate(int musicState, int playingTime, int totalTime);
    void doMusicPositionChanged(int playingTime, int totalTime);
    void doPlayStateChanged(int musicState);
    void doMediaDateChangedResponse();
    void doAVRCPConnectState(bool avrcpConnectState);
    void doA2DPConnectState(bool a2dpConnectState);
    void doPauseMusic();

private:
    bool m_a2dpState;
    bool m_avrcpState;
    bool m_musicState;
    QString m_title;
    QString m_artist;
    QString m_album;
    int m_totalTime;
    int m_currentTime;
    IBluetoothAvrcp *m_avrcpInterface;
    CBluetoothAVRCPCallBack *m_bluetoothAVRCPCallBack;
};

#endif
