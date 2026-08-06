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

#include "apptype.h"
#include "cappmanager.h"
#include "globalbus.h"
#include "applog.h"
#include "cavmutemanager.h"

using namespace universal_utils;

static const char *TAG = "AppManager_App";

template <> CAppManager* Singleton<CAppManager>::msSingleton = 0;

const char *CAppManager::EGLFS = "eglfs";
const char *CAppManager::WAYLAND = "wayland";

using namespace GlobalBus;

CAppManager::CAppManager(CObjFactory *pFactory)
    : m_factory(pFactory)
//    , m_MCUProxy(NULL)
{
    m_manager = new CAVMuteManager(pFactory);
    if (NULL == m_manager) {
        LOGE(TAG, "new CAVMuteManager false\n");
        throw m_manager;
    }
}

CAppManager::~CAppManager()
{
//    SAFE_DELETE(m_MCUProxy);
    SAFE_DELETE(m_manager);
}

bool CAppManager::init()
{
    //init app manager.
    return true;
}

bool CAppManager::startTaskWatcher()
{
    bool ret = false;

    ret = CTaskManager::startTaskWatcher();

    return ret;
}

bool CAppManager::restartTaskWatcher()
{
    bool ret = false;

    if (isDoingTask()) {
        ret = CTaskManager::forceTerminateTaskWatcher();
        if (!ret) {
            LOGE(TAG, "forceTerminateTaskWatcher false\n");
        }
    } else {
        ret = CTaskManager::stopTaskWatcher();
        if (!ret) {
            LOGE(TAG, "stopTaskWatcher false\n");
        }
    }

    SAFE_DELETE(m_manager);

    m_manager = new CAVMuteManager(m_factory);
    if (NULL == m_manager) {
        LOGE(TAG, "new CAVMuteManager fail\n");
        ret = false;
    }

    if (ret) {
        ret = startTaskWatcher();
        if (!ret) {
            LOGE(TAG, "startTaskWatcher fail\n");
        }
    }

    return ret;
}

bool CAppManager::doTask(const CCmdTask &cmdTask)
{
    bool ret = false;
    CCmdTask::E_MAINFUNC mainFunc = (CCmdTask::E_MAINFUNC)cmdTask.getMainFunc();
    LOGI(TAG, "CAppManager doTask begin mainFunc:%d\n", mainFunc);

    if (NULL == m_manager) {
        LOGE(TAG, "m_manager is NULL\n");
        ret = false;
    } else {
        ret = true;
    }

    if (ret) {
        switch (mainFunc) {
        case CCmdTask::MAIN_FUNC_APP_ACTION:
            ret = appActionProc(cmdTask);
            break;
        case CCmdTask::MAIN_FUNC_APP_JUMP:
            ret = m_manager->processAppJump(cmdTask);
            if (!ret) {
                LOGE(TAG, "m_manager->processAppJump false!\n");
            }
            break;
        default:
            ret = m_manager->processAutoTestCmd(cmdTask);
            if (!ret) {
                LOGE(TAG, "m_manager->processAutoTestCmd false!\n");
            }
            break;
        }
    }

    return ret;
}

bool CAppManager::appActionProc(const CCmdTask &cmdTask)
{
// #ifndef APP_SUPPORT
//     if (cmdTask.getAppID() != CAPPBaseObj::APPID_CLUSTER) {
//         LOGD(TAG, "APP not support");
//         return true;
//     }
// #endif
    bool ret = false;
    LOGI(TAG, "appActionProc subFunc:%d\n", cmdTask.getSubFunc());
    LOGI(TAG, "appID:%d, data:%d\n", cmdTask.getAppID(), cmdTask.getData());
    switch (cmdTask.getSubFunc()) {
    case ACTION_RUN:
        ret = m_manager->onRun(cmdTask.getAppID(), cmdTask.getData());
        if (!ret)
            LOGE(TAG, "m_manager->onRun ERROR!\n");
        break;

    case ACTION_EXIT:
        ret = m_manager->onExit(cmdTask.getAppID(), cmdTask.getData());
        if (!ret)
            LOGE(TAG, "m_manager->onExit ERROR!\n");
        break;

    case ACTION_REMOVE:
        ret = m_manager->onRemove(cmdTask.getAppID(), cmdTask.getData());
        if (!ret)
            LOGE(TAG, "m_manager->onRemove ERROR!\n");
        break;

    case ACTION_SHOWFRONT:
        ret = m_manager->onShowFront(cmdTask.getAppID(), cmdTask.getData());
        if (!ret)
            LOGE(TAG, "m_manager->onShowFront ERROR!\n");
        break;

    case ACTION_HIDEFRONT:
        ret = m_manager->onHideFront(cmdTask.getAppID(), cmdTask.getData());
        if (!ret)
            LOGE(TAG, "m_manager->onHideFront ERROR!\n");
        break;

    case ACTION_SHOWREAR:
        ret = m_manager->onShowRear(cmdTask.getAppID(), cmdTask.getData());
        if (!ret)
            LOGE(TAG, "m_manager->onShowRear ERROR!\n");
        break;

    case ACTION_HIDEREAR:
        ret = m_manager->onHideRear(cmdTask.getAppID(), cmdTask.getData());
        if (!ret)
            LOGE(TAG, "m_manager->onHideRear ERROR!\n");
        break;

    case ACTION_DUPRUN:
        ret = m_manager->onDupRun(cmdTask.getAppID(), cmdTask.getData());
        if (!ret)
            LOGE(TAG, "m_manager->onDupRun ERROR!\n");
        break;

    // process audio resource
    case ACTION_AUDIO_REQ:
        ret = m_manager->onAudioReq(cmdTask.getAppID(), cmdTask.getData());
        if (!ret)
            LOGE(TAG, "m_manager->onAudioReq ERROR!\n");
        break;
    case ACTION_AUDIO_REL:
        ret = m_manager->onAudioRel(cmdTask.getAppID(), cmdTask.getData());
        if (!ret)
            LOGE(TAG, "m_manager->onAudioRel ERROR!\n");
        break;

    // process video resource
    case ACTION_VIDEO_REQ:
        ret = m_manager->onVideoReq(cmdTask.getAppID(), cmdTask.getData());
        if (!ret)
            LOGE(TAG, "m_manager->onVideoReq ERROR!\n");
        break;
    case ACTION_VIDEO_REL:
        ret = m_manager->onVideoRel(cmdTask.getAppID(), cmdTask.getData());
        if (!ret)
            LOGE(TAG, "m_manager->onVideoRel ERROR!\n");
        break;

    // process audio&video resource
    case ACTION_AV_REQ:
        ret = m_manager->onAVReq(cmdTask.getAppID(), cmdTask.getData());
        if (!ret)
            LOGE(TAG, "m_manager->onAVReq ERROR!\n");
        break;
    case ACTION_AV_REL:
        ret = m_manager->onAVRel(cmdTask.getAppID(), cmdTask.getData());
        if (!ret)
            LOGE(TAG, "m_manager->onAVRel ERROR!\n");
        break;

    case ACTION_GOHOME:
        ret = m_manager->onGoHome(cmdTask.getAppID(), cmdTask.getData());
        if (!ret)
            LOGE(TAG, "m_manager->onGoHome ERROR!\n");
        break;
/*
    case ACTION_OUTSIDE_AUDIO_REQ:
        ret = m_manager->onAudioReq(cmdTask.getDataParam1(), cmdTask.getDataParam2(), CAPPBaseObj::LEVEL_TRANSIENT);
        if (!ret)
            LOGE(TAG, "m_manager->onAudioReq ERROR!\n");
        break;
    case ACTION_OUTSIDE_AUDIO_REL:
        ret = m_manager->onAudioRel(cmdTask.getDataParam1(), cmdTask.getDataParam2(), CAPPBaseObj::LEVEL_TRANSIENT);
        if (!ret)
            LOGE(TAG, "m_manager->onAudioRel ERROR!\n");
        break;
*/
    case ACTION_MAINAPP_DONE:
        ret = m_manager->start();
        if (!ret)
            LOGE(TAG, "m_manager->start ERROR!\n");

//        m_MCUProxy = new MCUProxy;
        break;


    case ACTION_STATUS_CHANGED:
        ret = m_manager->notifyAllApp(cmdTask);
        if (!ret)
            LOGE(TAG, "m_manager->notifyAllApp ERROR!\n");
        break;

    case ACTION_ARM2_BACKCAR_IN:
    case ACTION_ARM2_BACKCAR_OUT:
        ret = m_manager->onMiscRequest(cmdTask);
        if (!ret)
            LOGE(TAG, "m_manager->onMiscRequest ERROR!\n");
        break;

    case ACTION_VOLUMEKEY:
        ret = m_manager->onKeyEvent(cmdTask);
        if (!ret)
            LOGE(TAG, "m_manager->onKeyEvent ERROR!\n");
        break;

    default:
        LOGE(TAG, "NO such E_APPLY_ACTION!\n");
        ret = false;
        break;
    }

    return ret;
}

