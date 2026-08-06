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
 
#include "cfilesyswatcher.h"
#include <QObject>
#include "applog.h"
//#include "storage_atc.h"

static const char* const TAG = "CFileSysWatcher";


#define DEV_ROOT_PATH "/media"

CFileSysWatcher::CFileSysWatcher()
    : QObject(NULL)
    , m_msg(CMD_DEV_CHANGED)
    , m_wParam(0)
    , m_lParam(0)
    , m_devManager(NULL)
{
    initFileSystemWatcher();
}

CFileSysWatcher::~CFileSysWatcher()
{
    if (NULL != m_devManager) {
        //delete m_devManager;
        QObject::disconnect(m_devManager, SIGNAL(deviceChanged(const QString&, const QString&)),
                 this, SLOT(devChanged(const QString&, const QString&)));
        m_devManager = NULL;
    }
}

bool CFileSysWatcher::initFileSystemWatcher()
{
    bool ret = false;
    if (NULL == m_devManager) {
        m_devManager = AtcDeviceManager::getInstance();;
    }

    if (NULL != m_devManager) {
        QObject::connect(m_devManager, SIGNAL(deviceChanged(const QString&, const QString&)),
                 this, SLOT(devChanged(const QString&, const QString&)));
        m_devManager->start();
        ret = true;
    }
    else {
        LOGE(TAG, "initFileSystemWatcher fail..\r\n");
    }

    return ret;
}

void CFileSysWatcher::devChanged(const QString& mountpoint, const QString& action)
{
    std::string devpath = mountpoint.toStdString();
    std::string devactive = action.toStdString();
    LOGD(TAG, "devChanged: [%s] [%s]\r\n",devactive.c_str(),
        devpath.c_str());
/*
    std::string temppath;
    std::string mediapath = DEV_ROOT_PATH;
    std::size_t pos = devpath.find(mediapath);

    if (pos == 0) {
        temppath = devpath.substr(mediapath.size());
        m_wParam = (int)(temppath.c_str());
    } else {
        m_wParam = (int)(devpath.c_str());
    }*/
    m_wParam = (int)(devpath.c_str());

    if (action.compare(MOUNTED) == 0) {
        m_lParam = EXIST;
    }
    else if (action.compare(EJECT) == 0){
        m_lParam = NOT_EXIST;
    }
    else {
        LOGE(TAG, "unkown active.\r\n");
        return;
    }


    emit sendDevChangedMsg(CMD_DEV_CHANGED, m_wParam, m_lParam);
}

