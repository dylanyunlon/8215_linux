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
 
#include "bluetoothmusicpage.h"

using namespace std;
static const char* const tag = "CBluetoothMusicPage";

CBluetoothMusicPage::CBluetoothMusicPage()
    : m_a2dpState(false)
    , m_avrcpState(false)
    , m_musicState(false)
    , m_title(STRING_DEFAULTID3)
    , m_artist(STRING_DEFAULTID3)
    , m_album(STRING_DEFAULTID3)
    , m_totalTime(300000)
    , m_currentTime(0)
    , m_avrcpInterface(NULL)
    , m_bluetoothAVRCPCallBack(NULL)
{

}

CBluetoothMusicPage::~CBluetoothMusicPage()
{
    LOGD(tag, "destructor\n");

}

//connect the avrcpcallback signal function and this slot function
void CBluetoothMusicPage::initBluetoothMusicPage()
{
    m_bluetoothAVRCPCallBack = CBluetoothAVRCPCallBack::getSingletonPtr();

    QObject::connect(m_bluetoothAVRCPCallBack, SIGNAL(sigPlaybackDateUpdate(int, int, int)),
                        this, SLOT(doPlaybackDateUpdate(int, int, int)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothAVRCPCallBack, SIGNAL(sigMusicPositionChanged(int, int)),
                            this, SLOT(doMusicPositionChanged(int, int)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothAVRCPCallBack, SIGNAL(sigPlayStateChanged(int)),
                                this, SLOT(doPlayStateChanged(int)), Qt::QueuedConnection);
    QObject::connect(m_bluetoothAVRCPCallBack, SIGNAL(sigMediaDateChangedResponse()),
                    this, SLOT(doMediaDateChangedResponse()), Qt::QueuedConnection);

}

//get the avrcp interface
void CBluetoothMusicPage::getAVRCPInterface(IBluetoothAvrcp *avrcpInterface)
{
    m_avrcpInterface = avrcpInterface;
    if(NULL != m_avrcpInterface ) {
        ;
    } else {
        LOGE(tag, "m_avrcpInterface is empty!\n");
    }
}

///////////////////////////////////////////// the slot function /////////////////////////////////////////////////

//bluetoothMusicPageView get the music's title
QString CBluetoothMusicPage::getTitle()
{
    LOGD(tag, "getTitle\n");

    return m_title;
}

//bluetoothMusicPageView get the music's artist
QString CBluetoothMusicPage::getArtist()
{
    LOGD(tag, "getArtist\n");

    return m_artist;
}

//bluetoothMusicPageView get the music's album
QString CBluetoothMusicPage::getAlbum()
{
    LOGD(tag, "getAlbum\n");

    return m_album;
}

//bluetoothMusicPageView get the a2dp state
bool CBluetoothMusicPage::getA2dpState()
{
    LOGD(tag, "getA2dpState\n");

    return m_a2dpState;
}

//bluetoothMusicPageView get the avrcp state
bool CBluetoothMusicPage::getAvrcpState()
{
    LOGD(tag, "getAvrcpState\n");

    return m_avrcpState;
}

//bluetoothMusicPageView get the music's playing sate
bool CBluetoothMusicPage::getMusicState()
{
    LOGD(tag, "getMusicState\n");

    return m_musicState;
}

//bluetoothMusicPageView get the music's current time
int CBluetoothMusicPage::getCurrentTime()
{
    LOGD(tag, "getCurrentTime\n");

    return m_currentTime;
}

//bluetoothMusicPageView get the music's total time
int CBluetoothMusicPage::getTotalTime()
{
    LOGD(tag, "getTotalTime\n");

    return m_totalTime;
}

void CBluetoothMusicPage::checkMediaAudioConnectState()
{
    LOGD(tag, "checkMediaAudioConnectState\n");

    if (false == m_a2dpState) {
        emit sigA2DPConnectRequest();
    }
    if (false == m_avrcpState) {
        emit sigAVRCPConnectRequest();
    }
}

//the local user send the update play status request in bluetoothMusicPageView
void CBluetoothMusicPage::updatePlayStatusRequest()
{
    LOGD(tag, "updatePlayStatusRequest\n");

    if (NULL != m_avrcpInterface) {
        LOGD(tag, "updatePlayStatus\n");
        m_avrcpInterface->updatePlayStatus();
        doMediaDateChangedResponse();
    } else {
        LOGE(tag, "m_avrcpInterface is empty!\n");
    }
}

//the local user send the previous request in bluetoothMusicPageView
void CBluetoothMusicPage::musicPreviousRequest()
{
    LOGD(tag, "musicPreviousRequest\n");

    if (NULL != m_avrcpInterface) {
        LOGD(tag, "prev\n");
        m_avrcpInterface->prev();
    } else {
        LOGE(tag, "m_avrcpInterface is empty!\n");
    }
}

//the local user send the play or pause request in bluetoothMusicPageView
void CBluetoothMusicPage::musicPausePlayRequest()
{
    LOGD(tag, "musicPausePlayRequest\n");

    if (NULL != m_avrcpInterface) {
        if (true == m_musicState) {
            LOGD(tag, "pause\n");
            m_avrcpInterface->pause();
        } else {
            LOGD(tag, "play\n");
            m_avrcpInterface->play();
        }
    } else {
        LOGE(tag, "m_avrcpInterface is empty!\n");
    }
}

//the local user send the next request in bluetoothMusicPageView
void CBluetoothMusicPage::musicNextRequest()
{
    LOGD(tag, "musicNextRequest\n");

    if (NULL != m_avrcpInterface) {
        LOGD(tag, "next\n");
        m_avrcpInterface->next();
    } else {
        LOGE(tag, "m_avrcpInterface is empty!\n");
    }
}

//get the music play back date from avrcpcallback
void CBluetoothMusicPage::doPlaybackDateUpdate(int musicState, int playingTime, int totalTime)
{
    LOGD(tag, "doPlaybackDateUpdate\n");
    LOGI(tag, "musicState is %d, playingTime is %d, totalTime is %d\n", musicState, playingTime, totalTime);

    if (PLAYING == musicState) {
        m_musicState = true;
    } else if (PAUSED == musicState || STOPPED == musicState) {
        m_musicState = false;
    }
    emit musicStateChanged(m_musicState);

    if (0 <= playingTime) {
        m_currentTime = playingTime;
    } else {
        m_currentTime = 0;
    }
    emit currentTimeChanged(m_currentTime);

    m_totalTime = totalTime;
    emit totalTimeChanged(m_totalTime);
}

//get the music position from avrcpcallback
void CBluetoothMusicPage::doMusicPositionChanged(int playingTime, int totalTime)
{
    LOGD(tag, "doMusicPositionChanged, playingTime is %d\n", playingTime);
    if (totalTime >0 && totalTime != m_totalTime) {
        m_totalTime = totalTime;
        emit totalTimeChanged(m_totalTime);
    }

    if (0 <= playingTime) {
        m_currentTime = playingTime;
    } else {
        m_currentTime = 0;
    }
    emit currentTimeChanged(m_currentTime);
}

//get the music play state from avrcpcallback
void CBluetoothMusicPage::doPlayStateChanged(int musicState)
{
    LOGD(tag, "doPlayStateChanged, musicState is %d\n", musicState);

    if (PLAYING == musicState) {
        m_musicState = true;
    } else if (PAUSED == musicState || STOPPED == musicState) {
        m_musicState = false;
    }
    emit musicStateChanged(m_musicState);
}

//get the media date changed response from avrcpcallback
void CBluetoothMusicPage::doMediaDateChangedResponse()
{
    LOGD(tag, "doMediaDateChangedResponse\n");

    string title = STRING_UNKNOWN;
    string artist = STRING_UNKNOWN;
    string album = STRING_UNKNOWN;
    int ret_title = 0;
    int ret_artist = 0;
    int ret_album = 0;

    if (NULL != m_avrcpInterface) {
        LOGD(tag, "getMediaTitle\n");
        ret_title = m_avrcpInterface->getMediaTitle(title);
        if (-1 == ret_title) {
            LOGE(tag, "getMediaTitle Fail!\n");
            title = STRING_UNKNOWN;
        }
        if ("" == title) {
            title = STRING_UNKNOWN;
        }

        LOGD(tag, "getMediaArtist\n");
        ret_artist = m_avrcpInterface->getMediaArtist(artist);
        if (-1 == ret_artist) {
            LOGE(tag, "getMediaArtist Fail!\n");
            artist = STRING_UNKNOWN;
        }
        if ("" == artist) {
            artist = STRING_UNKNOWN;
        }

        LOGD(tag, "getMediaTitle\n");
        ret_album = m_avrcpInterface->getMediaAlbum(album);
        if (-1 == ret_album) {
            LOGE(tag, "getMediaAlbum Fail!\n");
            album = STRING_UNKNOWN;
        }
        if ("" == album) {
            album = STRING_UNKNOWN;
        }
        LOGI(tag, "title is %s, artist is %s, album is %s\n",
            title.c_str(), artist.c_str(), album.c_str());

        if (STRING_UNKNOWN == title) {
            m_title = STRING_DEFAULTID3;
        } else {
            m_title = QString::fromStdString(title);
        }
        if (STRING_UNKNOWN == artist) {
            m_artist = STRING_DEFAULTID3;
        } else {
            m_artist = QString::fromStdString(artist);
        }
        if (STRING_UNKNOWN == album) {
            m_album = STRING_DEFAULTID3;
        } else {
            m_album = QString::fromStdString(album);
        }
        if ((0 <= ret_title) || (0 <= ret_artist) || (0 <= ret_album)) {
            emit id3InfoChanged(m_title, m_artist, m_album);
        }
    }

}

//get the avrcp connect state from bluetoothPairedRecordsPage
void CBluetoothMusicPage::doAVRCPConnectState(bool avrcpConnectState)
{
    LOGD(tag, "doMediaConnectState, avrcpConnectState is %d\n", avrcpConnectState);

    m_avrcpState = avrcpConnectState;

    emit avrcpStateChanged(m_avrcpState);
}

//get the a2dp connect state from bluetoothPairedRecordsPage
void CBluetoothMusicPage::doA2DPConnectState(bool a2dpConnectState)
{
    LOGD(tag, "doA2DPConnectState, a2dpConnectState is %d\n", a2dpConnectState);

    m_a2dpState = a2dpConnectState;

    emit a2dpStateChanged(m_a2dpState);
}

void CBluetoothMusicPage::doPauseMusic()
{
    LOGD(tag, "doPauseMusic\n");

    if (NULL != m_avrcpInterface) {
        if (m_avrcpState && m_musicState) {
            LOGD(tag, "pause\n");
            m_avrcpInterface->pause();
        }
    } else {
        LOGE(tag, "m_avrcpInterface is empty!\n");
    }
}


