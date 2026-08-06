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

#include "volumeOverlay.h"
#include "applog.h"
//#include "settingcommon.h"
#include "ccmdtask.h"
#include "ctllistener.h"
#include <iostream>
#include <sstream>

static const char *TAG = "HOME_APP volumeOverlay";
#define AUTO_HIDE_TIMEOUT  (2000)

CvolumeOverlay::CvolumeOverlay(QQuickWindow *window)
: m_volumeWindow(window)
//, m_settingclient(NULL)
//, m_volume(NULL)
, m_autoHideTimer(NULL)
{
    if (m_autoHideTimer == NULL) {
        m_autoHideTimer = new QTimer(this);
    }

    if (m_autoHideTimer != NULL) {
        m_autoHideTimer->setSingleShot(true);
        LOGI(TAG, "connect autohide timer \n");
        QObject::connect(m_autoHideTimer, SIGNAL(timeout()),
            this, SLOT(doAutoHideTimeout()));

        QObject::connect(this, SIGNAL(volumeChange()),
            this, SLOT(doVolumeChanged()));
    } else {
        LOGE(TAG, "create autohidetimer fail\n");
    }

    creatVolumeMap();
}

CvolumeOverlay::~CvolumeOverlay()
{
    //m_volumeLock.lock();
    /*if (m_settingclient != NULL) {
        m_settingclient->releaseSettingInstance();
    }*/

    if (NULL != m_autoHideTimer) {
        delete  m_autoHideTimer;
        m_autoHideTimer = NULL;
    }
    //m_volumeLock.unlock();
}

bool CvolumeOverlay::doKeyEvent(int key, int param1, int param2)
{
    bool ret= false;

    switch (key) {
        case CCmdTask::SUBFUNC_KEY_FRONT_VOLUME_INC:
            {
                VOLUME_TYPE type = VOLUME_TYPE_MAX;
                switch (param1) {
                    case CCtlListener::VOLUME_MEDIA:
                        type = VOLUME_TYPE_MEDIA;
                        break;

                    case CCtlListener::VOLUME_GIS:
                        type = VOLUME_TYPE_GPS;
                        break;

                    case CCtlListener::VOLUME_BT:
                        type = VOLUME_TYPE_BT;
                        break;

                    default:
                        break;
                }

                if (type != VOLUME_TYPE_MAX) {
                    ret = incVolume(type);
                }
            }
            break;

        case CCmdTask::SUBFUNC_KEY_FRONT_VOLUME_DEC:
            {
                VOLUME_TYPE type = VOLUME_TYPE_MAX;
                switch (param1) {
                    case CCtlListener::VOLUME_MEDIA:
                        type = VOLUME_TYPE_MEDIA;
                        break;

                    case CCtlListener::VOLUME_GIS:
                        type = VOLUME_TYPE_GPS;
                        break;

                    case CCtlListener::VOLUME_BT:
                        type = VOLUME_TYPE_BT;
                        break;

                    default:
                        break;
                }

                if (type != VOLUME_TYPE_MAX) {
                    ret = decVolume(type);
                }
            }
            break;

        case CCmdTask::SUBFUNC_KEY_MUTE_UNMUTE:
            {
                VOLUME_TYPE type = VOLUME_TYPE_MAX;
                switch (param1) {
                    case CCtlListener::VOLUME_MEDIA:
                        type = VOLUME_TYPE_MEDIA;
                        break;

                    case CCtlListener::VOLUME_GIS:
                        type = VOLUME_TYPE_GPS;
                        break;

                    case CCtlListener::VOLUME_BT:
                        type = VOLUME_TYPE_BT;
                        break;

                    default:
                        break;
                }

                if (type != VOLUME_TYPE_MAX) {
                    ret = muteUnmute(type);
                }
            }
            break;

        default:
            break;
    }
    return ret;
}


void CvolumeOverlay::initObject()
{
    //ISetting* settingInst = NULL;

    LOGD(TAG, "initObject enter\n");
    //settingInst = ISetting::getSettingInstance();
    LOGD(TAG, "initObject getsetting client!!!\n");
    //m_volumeLock.lock();
    /*m_settingclient = settingInst;
    if (m_settingclient != NULL) {
        m_volume = m_settingclient->getSettingAdapter("volume");
    } else {
        LOGE(TAG, "cann't get volume server. \n");
    }*/

    /*if (m_volume != NULL) {
        LOGI(TAG, "registerCallBack this. \n");
        m_volume->registerCallBack(this);
    }*/
    //m_volumeLock.unlock();
    LOGD(TAG, "initObject leave\n");
}

int CvolumeOverlay::onSettingIndication(std::string cmd, std::string param)
{
    std::map<VOLUME_TYPE, std::string>::const_iterator iter;
    int value = 0;

    LOGD(TAG, "indication %s, %s enter\n", cmd.c_str(), param.c_str());
    iter = m_volumeTypeMap.begin();
    while (iter != m_volumeTypeMap.end()) {
        if (cmd.compare(iter->second) == 0) {
            break;
        }
        iter++;
    }

    if (iter == m_volumeTypeMap.end()) {
        LOGE(TAG, "onSettingIndication not found index inmap, %s, %s\n", cmd.c_str(), param.c_str());
        return 0;
    }

    value = stringToInt(param);
    //LOGD(TAG, "indication %s, send volume change event \n", iter->second.c_str());
    emit broadcastVolumeValue(iter->first, value);
    //LOGD(TAG, "indication %s, %s leave\n", cmd.c_str(), param.c_str());
    return 0;
}

void CvolumeOverlay::doAutoHideTimeout()
{
    LOGD(TAG, "doAutoHideTimeout\n");
    hideVolumeOverlay();
}

void CvolumeOverlay::doVolumeChanged()
{
    LOGD(TAG, "doVolumeChanged\n");
    if (m_volumeWindow != NULL) {
        m_volumeWindow->show();
    }
    stopAutoHideTimer();
    startAutoHideTimer();
}

int CvolumeOverlay::creatVolumeMap()
{
    m_volumeTypeMap.clear();
    m_volumeTypeMap.insert(std::make_pair(VOLUME_TYPE_MEDIA, VOLUMEMEDIA));
    m_volumeTypeMap.insert(std::make_pair(VOLUME_TYPE_GPS, VOLUMEGIS));
    m_volumeTypeMap.insert(std::make_pair(VOLUME_TYPE_BT, VOLUMEBT));
    m_volumeTypeMap.insert(std::make_pair(VOLUME_TYPE_MUTE, VOLUMEMUTE));

    LOGI(TAG, "creatVolumeMap OK.\n");

    return 0;
}

int CvolumeOverlay::getVolume(VOLUME_TYPE volumeType) const
{
    std::string volumeValue;
    int ret = 0;
    std::map<VOLUME_TYPE, std::string>::const_iterator iter;
    iter = m_volumeTypeMap.find(volumeType);
    if (iter == m_volumeTypeMap.end()) {
        return 0;
    }
    /*if (m_volume != NULL) {
        ret = m_volume->getValue(iter->second, volumeValue);
    }*/
    if (ret != 0) {
        LOGE(TAG, "getValue error.\n");
    }

    return stringToInt(volumeValue);
}

bool CvolumeOverlay::setVolume(VOLUME_TYPE volumeType, int value)
{
    std::map<VOLUME_TYPE, std::string>::iterator iter;
    std::string str;
    std::stringstream instream;
    bool ret = false;

    instream<<value;
    iter = m_volumeTypeMap.find(volumeType);
    if (iter == m_volumeTypeMap.end()) {
        LOGE(TAG, "onSettingIndication not found index inmap, type(%d)", volumeType);
        return false;
    }

    str = instream.str();
    /*if (m_volume != NULL) {
        m_volume->setValue(iter->second, str);
        ret = true;
    }*/

    return ret;
}


void CvolumeOverlay::showVolumeOverlay(VOLUME_TYPE type, int value)
{
    LOGD(TAG, "showVolumeOverlay type:%d \n", type);
    emit showVolume(type, value);
    emit volumeChange();
}

void CvolumeOverlay::hideVolumeOverlay()
{
    if (m_volumeWindow != NULL) {
        m_volumeWindow->hide();
    }
    LOGD(TAG, "hideVolumeOverlay\n");
}

bool CvolumeOverlay::isVolumeOverlayVisible()
{
    if (m_volumeWindow != NULL) {
        return m_volumeWindow->isVisible();
    }

    return false;
}

int CvolumeOverlay::stringToInt (const std::string& str)const
{
    int value = 0;
    std::stringstream ss;
    ss<<str.c_str();
    ss>>value;
    return value;
}

void CvolumeOverlay::startAutoHideTimer()
{
    if (NULL != m_autoHideTimer) {
        LOGD(TAG, "startAutoHideTimer\n");
        m_autoHideTimer->start(AUTO_HIDE_TIMEOUT);
    } else {
        LOGE(TAG, "startAutoHideTimer is null\n");
    }
}

void CvolumeOverlay::stopAutoHideTimer()
{
    if (NULL != m_autoHideTimer && m_autoHideTimer->isActive()) {
        LOGD(TAG, "stopAutoHideTimer\n");
        m_autoHideTimer->stop();
    } else if (false == m_autoHideTimer->isActive()) {
        LOGD(TAG, "m_autoHideTimer already stop\n");
    } else {
        LOGE(TAG, "m_autoHideTimer is null\n");
    }
}

bool CvolumeOverlay::incVolume(VOLUME_TYPE type)
{
    int volume = 0;
    bool ret = false;
    bool needUnmute = false;
    bool muted = false;

    LOGD(TAG, "incVolume, type(%d)\n", type);
    //m_volumeLock.lock();
    switch (type)
    {
        case VOLUME_TYPE_MEDIA:
        case VOLUME_TYPE_GPS:
            // current is mute state
            if (getVolume(VOLUME_TYPE_MUTE) != 0) {
                needUnmute = true;
            }
            break;

        case VOLUME_TYPE_BT:
            break;

        default:
            break;
    }

    // if current is mute and volumeoverlay not visible, so show mute state
    if (needUnmute && !isVolumeOverlayVisible()) {
        LOGD(TAG, "incVolume, type(%d), cur is mute, so show mute\n", type);
        showVolumeOverlay(VOLUME_TYPE_MUTE, 1);
        ret = true;
    } else {
        volume = getVolume(type);
        if (setVolume(type, volume + 1)) {
            if (needUnmute) {
                setVolume(VOLUME_TYPE_MUTE, 0);
            }
            volume = getVolume(type);
            showVolumeOverlay(type, volume);
            ret = true;
        }
    }
    //m_volumeLock.unlock();

    return ret;
}

bool CvolumeOverlay::decVolume(VOLUME_TYPE type)
{
    int volume = 0;
    bool ret = false;
    bool needUnmute = false;
    bool muted = false;

    LOGD(TAG, "decVolume, type(%d)\n", type);
    //m_volumeLock.lock();
    switch (type)
    {
        case VOLUME_TYPE_MEDIA:
        case VOLUME_TYPE_GPS:
            // current is mute state
            if (getVolume(VOLUME_TYPE_MUTE) != 0) {
                needUnmute = true;
            }
            break;

        case VOLUME_TYPE_BT:
            break;

        default:
            break;
    }
    volume = getVolume(type);

    // if current is mute and volumeoverlay not visible, so show mute state
    if (needUnmute && !isVolumeOverlayVisible()) {
        LOGD(TAG, "decVolume, type(%d), cur is mute, so show mute\n", type);
        showVolumeOverlay(VOLUME_TYPE_MUTE, 1);
        ret = true;
    } else {
        if (volume > 0) {
            volume--;
            if (setVolume(type, volume)) {
                if (needUnmute) {
                    setVolume(VOLUME_TYPE_MUTE, 0);
                }
                volume = getVolume(type);
                showVolumeOverlay(type, volume);
                ret = true;
            }
        } else {
            showVolumeOverlay(type, 0);
            ret = true;
        }
    }

    //m_volumeLock.unlock();
    return ret;

}

bool CvolumeOverlay::muteUnmute(VOLUME_TYPE type)
{
    int volume = 0;
    bool ret = false;
    bool volumeMuted = false;
    bool muted = false;

    LOGD(TAG, "muteUnmute, type(%d)\n", type);
    //m_volumeLock.lock();

    // current is mute state
    if (getVolume(VOLUME_TYPE_MUTE) != 0) {
        volumeMuted = true;
    }

    if (volumeMuted) {
        setVolume(VOLUME_TYPE_MUTE, 0);
        volume = getVolume(type);
        showVolumeOverlay(type, volume);
    } else {
        setVolume(VOLUME_TYPE_MUTE, 1);
        showVolumeOverlay(VOLUME_TYPE_MUTE, 1);
    }
    ret = true;
    //m_volumeLock.unlock();

    return ret;

}

