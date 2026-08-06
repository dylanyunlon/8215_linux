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

#include <algorithm>
#include <sys/time.h>
#include "ccmdtask.h"
#include "globalbus.h"
//#include "setting.h"
#include "applog.h"
#include "cavmutemanager.h"
//#include "AtcDisplaySettings.h"
#include "audiofocusrequestdata.h"

using namespace universal_utils;
using namespace GlobalBus;

#ifdef VIDEOBOOTANI_SUPPORT_EN
#include "atcbootanicom.h"
#else
//#include "DualarmDriver.h"
#endif /* VIDEOBOOTANI_SUPPORT_EN */

static const char TAG[] = "AVMuteManager";
static const char autotestPath[]  = "/usr/bin/autotestclient";

static const unsigned char INVALID_ID = 0;

static const string baseAPPFlag = "BaseAPP";
static const string audioFlag = "audioFlag";

pid_t CAVMuteManager::m_pid = -1;

CAVMuteManager::CAVMuteManager(CObjFactory *pFactory)
    : m_rearShowApp(INVALID_ID)
    , m_objFactory(pFactory)
    , m_preActiveApp(NULL)
{
    m_frontVideoRes.push_back(INVALID_ID);

    if (NULL == pFactory) {
        LOGE(TAG, "pFactory is NULL\n");
        throw pFactory;
    }

    m_bootList.push_back(CAPPBaseObj::APPID_CLUSTER);
// #ifdef CARPLAY_SUPPORT
//     m_bootList.push_back(CAPPBaseObj::APPID_CARPLAY_HELPER);
//     m_bootList.push_back(CAPPBaseObj::APPID_CARPLAY_APP);
// #endif

//#ifdef APP_SUPPORT
   // m_bootList.push_back(CAPPBaseObj::APPID_CARPLAY_APP);
//#endif

// #ifdef ATC_BT_SUPPORT
//     m_bootList.push_back(CAPPBaseObj::APPID_BT);
// #endif
    //m_bootList.push_back(CAPPBaseObj::APPID_SETTING);
    //m_bootList.push_back(CAPPBaseObj::APPID_RECOVERY);
// #ifdef WITH_SOAPP
//     m_bootList.push_back(CAPPBaseObj::APPID_BTPHONE);
// #endif

// #ifdef ANDROIDAUTO_SUPPORT
//     m_bootList.push_back(CAPPBaseObj::APPID_ANDROIDAUTO_APP);
// #endif

    // if (NULL != pFactory->getConf()) {
    //     m_watcher = new (std::nothrow)MemoryWatcherClient;
    //     if (NULL == m_watcher) {
    //         LOGE(TAG, "new MemoryWatcherClient fail\n");
    //         throw pFactory;
    //     }
    // } else
    //     m_watcher = NULL;

// #if 0
//     while (1) {
//         int ret = -1;
//         int appID = INVALID_ID;

//         ret = m_record.getApp(appID);
//         if (ret > 0) {
//             LOGD(TAG, "getApp %d\n", appID);
//             m_startList.push_back(appID);
//         } else
//             break;
//     }
// #endif
    // use analog ARM2_IN and OUT
    //initAnalogBackcarSendSignalThread();

}

void CAVMuteManager::initAnalogBackcarSendSignalThread(void) {
    m_initThread.init(this, &CAVMuteManager::analogBackcarSendSignalThread);
}

bool CAVMuteManager::analogBackcarSendSignalThread() {
    LOGD(TAG, "analogBackcarSendSignalThread enter");

    threadStart();
    return true;
}

unsigned long CAVMuteManager::threadRun() {
    sleep(1);

    // applyFor(ACTION_ARM2_BACKCAR_IN, CAPPBaseObj::APPID_BACKCAR, 0);
    // applyFor(ACTION_ARM2_BACKCAR_OUT, CAPPBaseObj::APPID_BACKCAR, 0);
}

CAVMuteManager::~CAVMuteManager()
{
    //SAFE_DELETE(m_objFactory);
    //SAFE_DELETE(m_watcher);
}

bool CAVMuteManager::judgeAppHide(CAPPControllerObj *operObj, CAPPControllerObj *activeObj)
{
    if ((operObj->getShareProcess().empty()
        || operObj->getShareProcess() != activeObj->getShareProcess())
            && (!operObj->isFloatingWindow()
            && operObj->getAppID() != CAPPBaseObj::APPID_RECOVERY
            && activeObj->getAppID() != CAPPBaseObj::APPID_BTPHONE)) {
        return true;
    }

    return false;
}

bool CAVMuteManager::start()
{
    bool ret = false;
    struct ipanic_header *iheader;

    // if (NULL != m_watcher) {
    //     // add by atc6129:When the backcar function is commplete,
    //     // it will be modified here
    //     // TODO: +1 is share process APP for BT
    //     //m_watcher->newFlag(baseAPPFlag,
    //         //m_bootList_be.size() + m_bootList_af.size() + 1);
    //     m_watcher->newFlag(baseAPPFlag, m_bootList.size() + 1);
    //     m_watcher->newFlag(audioFlag);
    // }

    while (!(ret = onStartRun(CAPPBaseObj::APPID_CLUSTER))) {
        sleep(2);
    }

    LOGI(TAG, "CAVMuteManager start leave1234:ret:%d\n", ret);
    return ret;
}

bool CAVMuteManager::onStartRun(unsigned char appID)
{
    LOGD(TAG, "onStartRun:%d\n", appID);

    bool ret = false;
    CAPPControllerObj *startRunObj = getAppObj(appID);
    static bool closeAnima = true;

    if (NULL == startRunObj) {
        startRunObj = newAppObj(appID);
    }

    do {
        if (startRunObj != NULL) {
            LOGD(TAG, "run %s at start\n", startRunObj->getAPPName());
            ret = startRunObj->run();
            if (!ret) {
                LOGE(TAG, "run %s fail!\n", startRunObj->getAPPName());
                releaseAppObj(startRunObj);
                break;
            } else {
                addApp(startRunObj);

                // if (NULL != m_watcher) {
                //     m_watcher->addProcess(appID);
                //     m_watcher->setProcessFlag(appID, baseAPPFlag);
                // }
            }
        } else {
            LOGE(TAG, "can't create app%d\n", appID);
            break;
        }

        if (m_bootList.front() == appID) {
            m_bootList.pop_front();
            if (!m_bootList.empty()) {
                GlobalBus::applyFor(GlobalBus::ACTION_RUN, m_bootList.front(), 0);
            }
        } else {
            LOGD(TAG, "show %s(%d) in advance\n",
                (NULL != startRunObj) ? startRunObj->getAPPName() : NULL, appID);
            std::list<unsigned char>::reverse_iterator rit =
                std::find(m_bootList.rbegin(), m_bootList.rend(), appID);
            if (rit != m_bootList.rend()) {
                m_bootList.erase(--rit.base());
            }

            onShowFront(appID, 0);
        }

        // it will be modified here
        if (m_bootList.empty()) {
            analogBackcarSendSignalThread();
        }

        if (m_bootList.empty() && closeAnima) {
            closeAnima = false;
#ifdef VIDEOBOOTANI_SUPPORT_EN
            /*communicate with bootanimation*/
            AniComm *aniComm = new AniComm();
            if (NULL != aniComm) {
                if (!aniComm->sendCommand(ANI_COMM_CMD_EXIT, true)) {
                    LOGE(TAG, "Notify Bootanimation to exit fail\n");
                }
                delete aniComm;
                aniComm = NULL;
            }
#else
            //int fd = OpenDualarmDriver();
            //SendDualarmMessage(fd);
            //CloseDualarmDriver(fd);
#endif /* VIDEOBOOTANI_SUPPORT_EN */
        }
    } while (0);

    LOGI(TAG, "onStartRun leave:ret=%d\n", ret);
    return ret;
}

bool CAVMuteManager::onDupRun(unsigned char appID, unsigned int data)
{
    bool ret = false;
    CAPPControllerObj *runObj = getAppObj(appID);

    if (NULL != runObj) {
        LOGW(TAG, "already has %s\n", runObj->getAPPName());
        return true;
    }

    runObj = newAppObj(appID);
    if (NULL == runObj) {
        LOGE(TAG, "new appobj %d fail! data(%d)\n",
                    appID, data);
        ret = false;
    } else {
        ret = runObj->run();
        if (!ret) {
            LOGE(TAG, "%s run fail!\n", runObj->getAPPName());
            releaseAppObj(runObj);
        } else {
            addApp(runObj);
            // if (NULL != m_watcher) {
            //     m_watcher->addProcess(appID);
            //     m_watcher->setProcessFlag(appID, baseAPPFlag);
            // }
        }
    }

    return ret;
}

bool CAVMuteManager::onRun(unsigned char appID, unsigned int data)
{
    bool ret = false;
    CAPPControllerObj *activeObj = NULL;
    CAPPControllerObj *runObj = getAppObj(appID);
    LOGD(TAG, "onRun app(%d)\n", appID);

    if (NULL != runObj) {
        if (runObj->isRuning()) {
            LOGD(TAG, "%s(%d) is running, show it! param(%d)\n",
                        runObj->getAPPName(), appID, data);
            return onShowFront(appID, 0);
        }
    }

//#ifdef ATC_BT_SUPPORT
    if (CAPPBaseObj::APPID_BT == appID  // BT must use onStartRun
        ) {
        return onStartRun(appID);
    }
// #else
//     if (CAPPBaseObj::APPID_BT == appID) {
//         LOGD(TAG, "donot support BT, no run, param(%d)\n", appID);
//         return false;
//     }
// #endif

    if (m_bootList.end() != find(m_bootList.begin(), m_bootList.end(), appID)) {
        return onStartRun(appID);
    }

    do {
        runObj = newAppObj(appID);
        if (NULL == runObj) {
            LOGE(TAG, "new appobj %d fail! data(%d)\n", appID, data);
            ret = false;
            break;
        }

        // looking for active app, if none, get home
        activeObj = getActiveApp();
        if (NULL == activeObj) {
            activeObj = getAppObj(CAPPBaseObj::APPID_CLUSTER);
            if (NULL == activeObj) {
                LOGD(TAG, "can't get active APP or home!\n");
            }
        }
        

        /* check is higher priority app active.
         * for example: btphone or backcar is active,
         * can not run new app.
         */
        bool btphone = true;
#ifndef WITH_SOAPP
        if (activeObj && CAPPBaseObj::APPID_BTPHONE == activeObj->getAppID()) {
            btphone = false;
        }
#endif
        if (NULL != activeObj
                && activeObj->getPriority() < runObj->getPriority()
                && btphone) {
            LOGD(TAG, "priority higher app(%d) is active, can not run app(%d)!\n",
                        activeObj->getAppID(), appID);
            ret = false;
            break;
        }
#ifdef WITH_SOAPP
        if (NULL != activeObj && btphone) {
            activeObj->hideFrontUI();
            ret = activeObj->hideFront();
            if (!ret) {
                LOGE(TAG, "app %d hideFront fail!\n", activeObj->getAppID());
            }
        }
#endif
        ret = runObj->run(data);
        if (ret) {
            addApp(runObj);
            setActiveApp(appID);
            // if (NULL != m_watcher) {
            //     m_watcher->addProcess(appID);
            // }
        } else {
            LOGE(TAG, "run app %d fail!\n", appID);
            bool ret = false;

            ret = releaseAppObj(runObj);
            if (!ret) {
                LOGE(TAG, "releaseAppObj %d fail\n", appID);
            }

            if (NULL != activeObj) {
                ret = activeObj->showFront();
                if (!ret) {
                    LOGE(TAG, "previous app(%d) showFront fail\n",
                            activeObj->getAppID());
                    break;
                }
            }
        }
    } while (0);

    return ret;
}

bool CAVMuteManager::removeApp(unsigned char appID)
{
    bool ret = false;
    CAPPControllerObj *removeObj = getAppObj(appID);

    do {
        if (NULL == removeObj) {
            LOGE(TAG, "can not find appObj(%d)\n", appID);
            break;
        }

        if (removeObj->getShareProcess().empty()) {
            ret = rmActiveApp(appID);
            if (!ret) {
                LOGW(TAG, "rmActiveApp fail, can't find app %d\n", appID);
            }

            rmApp(appID);
            ret = releaseAppObj(removeObj);
            if (!ret) {
                LOGE(TAG, "releaseAppObj fail\n");
            }

            // if (NULL != m_watcher) {
            //     m_watcher->rmProcess(appID);
            // }
        } else {
            const std::string shareProcess = removeObj->getShareProcess();
            for (std::list<CAPPControllerObj *>::iterator iter = m_appObjList.begin();
                    iter != m_appObjList.end();) {
                if ((*iter)->getShareProcess() == shareProcess) {
                    unsigned char shareID = (*iter)->getAppID();

                    rmActiveApp(shareID);
                    ret = releaseAppObj(*iter);
                    iter = m_appObjList.erase(iter);

                    // if (NULL != m_watcher) {
                    //     m_watcher->rmProcess(shareID);
                    // }
                } else
                    iter++;
            }
        }
        //remove app in audiofocusStack
        auto it = std::find_if(m_audiofocusStack.begin(), m_audiofocusStack.end(),
            [appID](const AudiofocusStackItem& item){
                return item.appID == appID;
            });
        if (it == m_audiofocusStack.end()) {
            LOGD(TAG, "audiofocusStack is empty or appid doesn't exist");
            break;
        } else {
            m_audiofocusStack.remove_if([appID](const AudiofocusStackItem& item){
                return item.appID == appID;
            });
            if (!m_audiofocusStack.empty()) {
                getAppObj(m_audiofocusStack.back().appID)->sendAudioFocusResult(CCtlListener::AVOUT_F, CCtlListener::AUDIOFOCUS_GAIN);
            }
        }
    } while (0);

    return ret;
}

bool CAVMuteManager::onExit(unsigned char appID, unsigned int data)
{
    bool ret = false;
    CAPPControllerObj *exitObj = getAppObj(appID);
    bool showNext = false;
    LOGD(TAG, "onExit app(%d)\n", appID);

    do {
        if (NULL == exitObj) {
            LOGE(TAG, "can not find exit id(%d) data(%d)\r\n", appID, data);
            ret = false;
            break;
        }

        if (getActiveApp() == exitObj)
            showNext = true;

        // exit app
        //todo:
        ret = onAudioRel(appID, 0);
        //todo:
        if (m_frontVideoRes.has(appID)) {
            ret = onVideoRel(appID, 0);
            //if (!ret)
                //break;
        }

        ret = exitObj->exit();
        if (!ret) {
            LOGE(TAG, "app %d exit fail\n", appID);
            //break;
        }

        ret = removeApp(appID);
        if (!ret) {
            LOGE(TAG, "removeAPP %d fail\n", appID);
        }

        if (showNext) {
            // looking for next app, if none, get home
            CAPPControllerObj *nextActiveObj = getActiveApp();
            if (NULL == nextActiveObj) {
                nextActiveObj = getAppObj(CAPPBaseObj::APPID_CLUSTER);
                if (NULL == nextActiveObj) {
                    LOGE(TAG, "can't get active APP or home!\n");
                    break;
                }
            }

            nextActiveObj->showFrontUI();
            // if (NULL != m_watcher) {
            //     m_watcher->addProcess(nextActiveObj->getAppID());
            // }
            ret = nextActiveObj->showFront();
            if (!ret) {
                LOGE(TAG, "next app %d showFront fail\n", nextActiveObj->getAppID());
                break;
            }

            //if next app need video
            ret = resumeVideoApp(nextActiveObj->getAppID(), CAPPBaseObj::LEVEL_NORMAL);
            if (!ret) {
                LOGE(TAG, "%s resumeVideoApp fail\n", nextActiveObj->getAPPName());
                break;
            }
        }
    } while (0);

    return ret;
}

// for app is killed
bool CAVMuteManager::onRemove(unsigned char appID, unsigned int data)
{
    bool ret = false;
    CAPPControllerObj *removeObj = getAppObj(appID);
    bool showNext = false;
    LOGD(TAG, "onRemove app(%d)\n", appID);

    if (m_frontVideoRes.has(appID)) {
        m_frontVideoRes.remove(appID);
    }

    if (NULL == removeObj) {
        LOGE(TAG, "can not find remove id(%d) data(%d)\n", appID, data);
        ret = false;
    } else {
        ret = true;

        if (getActiveApp() == removeObj)
            showNext = true;

        ret = removeApp(appID);
        if (!ret) {
            LOGE(TAG, "removeAPP %d fail\n", appID);
        }
    }

    do {
        if (showNext) {
            // looking for next app, if none, get home
            CAPPControllerObj *nextActiveObj = getActiveApp();
            if (NULL == nextActiveObj) {
                nextActiveObj = getAppObj(CAPPBaseObj::APPID_CLUSTER);
                if (NULL == nextActiveObj) {
                    LOGE(TAG, "can't get active APP or home!\n");
                    break;
                }
            }

            nextActiveObj->showFrontUI();
            // if (NULL != m_watcher) {
            //     m_watcher->addProcess(nextActiveObj->getAppID());
            // }
            ret = nextActiveObj->showFront();
            if (!ret) {
                LOGE(TAG, "next app %d showFront fail\n", nextActiveObj->getAppID());
                break;
            }

            //if next app need video
            ret = resumeVideoApp(nextActiveObj->getAppID(), CAPPBaseObj::LEVEL_NORMAL);
            if (!ret) {
                LOGE(TAG, "%s resumeVideoApp fail\n", nextActiveObj->getAPPName());
                break;
            }
        }
    } while (0);

    return ret;
}

bool CAVMuteManager::onShowFront(unsigned char appID, unsigned int data)
{
    bool ret = false;
    CAPPControllerObj *showApp = NULL;
    CAPPControllerObj *activeObj = NULL;

    LOGD(TAG, "onShowFront app(%d)\n", appID);

    do {
        // looking for active app, if none, get home
        activeObj = getActiveApp();
        if (NULL == activeObj) {
            activeObj = getAppObj(CAPPBaseObj::APPID_CLUSTER);
            if (NULL == activeObj) {
                LOGE(TAG, "can't get active app or home!\n");
                ret = false;
                break;
            }
        }

        /***************************************************************************
         * Since the eglfs version does not support multiple windows, the floating window can not be
         * used properly when the UI is not in the Bluetooth Interface.So, here, taking into account
         * the above situation, we put out the whole Bluetooth APP show.
         */
#ifdef WITH_SOAPP
        if (!(activeObj == getAppObj(CAPPBaseObj::APPID_BT) && appID != CAPPBaseObj::APPID_BTPHONE)) {
            setPreActiveApp(activeObj);
        }

        if (appID == CAPPBaseObj::APPID_BTPHONE) {
            if (activeObj != getAppObj(CAPPBaseObj::APPID_BT)
                    && activeObj != getAppObj(CAPPBaseObj::APPID_CLUSTER)) {
                appID = CAPPBaseObj::APPID_BT;
            } else { // if active app is BT or cluster app, we do not need do anything.
                break;
            }
        }
#endif
        showApp = getAppObj(appID);
        if (NULL == showApp) {
            LOGE(TAG, "get app obj (%d) fail! data(%d)\r\n",
                        appID, data);
            ret = false;
            break;
        }

        if (showApp == activeObj) {
            LOGW(TAG, "APP %s already in front!\n", showApp->getAPPName());
            ret = false;
            break;
        }

        //check  priority.
        bool btphone = true;
#ifndef WITH_SOAPP
        if (CAPPBaseObj::APPID_BTPHONE == activeObj->getAppID()) {
            btphone = false;
        }
#endif
        if (activeObj->getPriority() < showApp->getPriority() && btphone) {
            LOGW(TAG, "priority higher %s(%d) is in front, can not show %s(%d)!\n",
                    activeObj->getAPPName(), activeObj->getAppID(),
                    showApp->getAPPName(), appID);
            if (showApp->getPriority() < CAPPBaseObj::APP_PRIORITY_DEFAULT) {
                setActiveAppInline(appID);
            }
            ret = false;
            break;
        }

        //if (!showApp->isFloatingWindow()) {
            CAPPControllerObj *active = activeObj;
            // while (active->isFloatingWindow()) {
            //     ret = active->hideFront();
            //     if (!ret) {
            //         LOGE(TAG, "app %d hideFront Fail!\n", active->getAppID());
            //     }
            //     active = getNextActiveApp(active->getAppID());
            // }
            ret = active->hideFrontUI();
            if (!ret) {
                LOGE(TAG, "app %d hideFront Fail!\n", active->getAppID());
            }
        //}

        setActiveApp(showApp->getAppID());

        btphone = false;
#ifndef WITH_SOAPP
        if (CAPPBaseObj::APPID_BTPHONE == activeObj->getAppID()
                || CAPPBaseObj::APPID_BTPHONE == showApp->getAppID()) {
            btphone = true;
        }
#endif

        //show it
        if (showApp->getShareProcess().empty()
            || showApp->getShareProcess() != activeObj->getShareProcess()
            || btphone) {
            ret = showApp->showFront(data);
            if (!ret) {
                LOGE(TAG, "app %d showFront fail!\n", appID);
            }
            showApp->showFrontUI();
        }

        // if (NULL != m_watcher) {
        //     m_watcher->addProcess(appID);
        // }

        //if is video app, resume it
        //todo:
        ret = resumeVideoApp(appID, 0);
        if (!ret) {
            LOGE(TAG, "app %d resumeVideoApp fail\n", appID);
            break;
        }
    } while (0);

    return ret;
}

bool CAVMuteManager::onHideFront(unsigned char appID, unsigned int data)
{
    bool ret = false;
    CAPPControllerObj *hideApp = NULL;
    CAPPControllerObj *nextActiveObj = NULL;
    LOGD(TAG, "onHideFront app(%d)\n", appID);

#ifdef WITH_SOAPP
    if (getPreActiveApp() != getAppObj(CAPPBaseObj::APPID_BT)) {
        if (appID == CAPPBaseObj::APPID_BTPHONE) {
            appID = CAPPBaseObj::APPID_BT;
        }
    }
#endif

    do {
        hideApp = getAppObj(appID);
        if (NULL == hideApp) {
            LOGE(TAG, "get app obj id(%d) fail!\n",
                        appID, data);
            ret = false;
            break;
        }

        ret = rmActiveApp(appID);
        if (!ret) {
            LOGE(TAG, "can't find active app(%d)\n", appID);
        }

        //get next app
        nextActiveObj = getActiveApp();
        if (NULL == nextActiveObj) {
            nextActiveObj = getAppObj(CAPPBaseObj::APPID_CLUSTER);
            if (NULL == nextActiveObj) {
                LOGE(TAG, "can't get active APP or home\n");
                ret = false;
                break;
            }
        }

//TODO:
#ifndef WITH_SOAPP
        if (!(!hideApp->getShareProcess().empty()
                && nextActiveObj->getShareProcess() == hideApp->getShareProcess())
                || CAPPBaseObj::APPID_BTPHONE == hideApp->getAppID()
                || CAPPBaseObj::APPID_BTPHONE == nextActiveObj->getAppID()) {

            LOGD(TAG, "hide app(%d)\n", appID);
            hideApp->hideFrontUI();
        }
#else
        hideApp->hideFrontUI();
#endif
        ret = hideApp->hideFront();
        if (!ret) {
            LOGE(TAG, "app %d hideFront fail!\n", appID);
        }

        //show next app
#ifndef WITH_SOAPP
        if (//  not share process app or has share process app but not next app
            (hideApp->getShareProcess().empty()
                || nextActiveObj->getShareProcess() != hideApp->getShareProcess())
            //  not floating window app
            //&& !hideApp->isFloatingWindow()
            //  next app not has share process app or has but not next active app
            && (nextActiveObj->getShareProcess().empty()
                || nextActiveObj->getShareProcess() != getNextActiveApp(nextActiveObj->getAppID())->getShareProcess())) {
            LOGD(TAG, "show ActiveApp\n");
            nextActiveObj->showFrontUI();
        }
#else
        nextActiveObj->showFrontUI();
#endif
        // if (NULL != m_watcher) {
        //     m_watcher->addProcess(nextActiveObj->getAppID());
        // }

        //if (!hideApp->isFloatingWindow()) {
            CAPPControllerObj *next = nextActiveObj;
            // while (next->isFloatingWindow()) {
            //     ret = next->showFront();
            //     if (!ret) {
            //         LOGE(TAG, "app %d showFront fail\n", next->getAppID());
            //     }
            //     next = getNextActiveApp(next->getAppID());
            // }
            ret = next->showFront();
            if (!ret) {
                LOGE(TAG, "app %d showFront fail\n", next->getAppID());
            }
        //}

        //if next app need video
        ret = resumeVideoApp(nextActiveObj->getAppID(), 0);
        if (!ret) {
            LOGE(TAG, "app %d resumeVideoApp fail\n", nextActiveObj->getAppID());
            break;
        }
    } while (0);

    return ret;
}

// TODO:
bool CAVMuteManager::onShowRear(unsigned char appID, unsigned int data)
{
    // bool ret = true;
    // CAPPControllerObj *showApp = getAppObj(appID);
    // CAPPControllerObj *rearObj = getAppObj(m_rearShowApp);

    // if (NULL == showApp) {
    //     ret = false;
    //     LOGE(TAG,"get app obj id(%d) fail! param(%d)\r\n",
    //                 appID, param);
    // }

    // //check  priority.
    // if (ret
    //     && NULL != rearObj
    //     && rearObj->getPriority() < showApp->getPriority()) {
    //     ret = false;
    //     LOGD(TAG, "priority higher id(%d) active, can not show id(%d)!\r\n",
    //                 rearObj->getAppID(), appID);
    // }

    // //rear app hide rear.
    // if (ret
    //     && NULL != rearObj) {
    //     rearObj->hideRear();
    // }

    // //show it rear
    // if (ret) {
    //     showApp->showRear();
    //     m_rearShowApp = showApp->getAppID();
    // }

    //return ret;
    return true;
}

// TODO:
bool CAVMuteManager::onHideRear(unsigned char appID, unsigned int param)
{
    bool ret = true;
    // CAPPControllerObj *hideApp = getAppObj(appID);

    // if (NULL == hideApp) {
    //     ret = false;
    //     LOGE(TAG, "get app obj id(%d) fail! param(%d)\r\n", appID, param);
    // }

    // if (ret) {
    //     hideApp->hideRear();
    //     m_rearShowApp = 0;
    // }

    return ret;
}

bool CAVMuteManager::onAudioReq(unsigned char appID, unsigned int data)
{
    bool ret = true;
    CAPPControllerObj *reqApp = getAppObj(appID);
    AudioFocusRequestData audioData(data);
    unsigned int output = audioData.getOutput();
    unsigned int focusType = audioData.getFocusType();
    unsigned int streamType = audioData.getStreamType();
    LOGD(TAG, "onAudioFocus request, app(%d), output(%s), focusType(%s), streamType(%s)\n", appID,
        AudioFocusRequestData::decodeOutput(output), AudioFocusRequestData::decodeFocusType(focusType), AudioFocusRequestData::decodeStreamType(streamType));
    do {
        if (NULL == reqApp) {
            LOGE(TAG, "%s: get app obj id(%d) fail! output(%d) focusType(%d)\n",
                    __func__, appID, output, focusType);
            ret = false;
            break;
        }
        if (!m_audiofocusStack.empty()) {
            if ((m_audiofocusStack.back().appID == appID && m_audiofocusStack.back().streamType == streamType)) {
                LOGE(TAG, "audiofcous request invalid, duplicate requests");
                ret = false;
                break;
            }
            if ((m_audiofocusStack.back().streamType == CAPPBaseObj::STREAM_VOICL_CALL || m_audiofocusStack.back().streamType == CAPPBaseObj::STREAM_ASSISTANT)
                    && m_audiofocusStack.back().streamType > streamType) {
                reqApp->sendAudioFocusResult(static_cast<CCtlListener::E_AVOUT>(output), CCtlListener::AUDIOFCOUS_REQUEST_FAILED);
                LOGE(TAG, "audiofcous request fail, higher priority focus uses");
                ret = false;
                break;
            }
            CAPPControllerObj *activeApp = getAppObj(m_audiofocusStack.back().appID);
            switch(focusType) {
                case CAPPBaseObj::LEVEL_TRANSIENT:
                    if (m_audiofocusStack.back().appID != appID) {
                        activeApp->sendAudioFocusResult(static_cast<CCtlListener::E_AVOUT>(output), CCtlListener::AUDIOFOCUS_LOSS_TRANSIENT);
                    }
                    break;
                case CAPPBaseObj::LEVEL_TRANSIENT_CAN_DUCK:
                    if (m_audiofocusStack.back().appID != appID) {
                        activeApp->sendAudioFocusResult(static_cast<CCtlListener::E_AVOUT>(output), CCtlListener::AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK);
                    }
                    break;
                default:
                    for (const auto& item : m_audiofocusStack){
                        if (item.appID != appID) {
                            getAppObj(item.appID)->sendAudioFocusResult(static_cast<CCtlListener::E_AVOUT>(output), CCtlListener::AUDIOFOCUS_LOSS);
                        }
                    }
                    m_audiofocusStack.clear();
            }
        }
        switch (focusType) {
            case CAPPBaseObj::LEVEL_TRANSIENT:
            case CAPPBaseObj::LEVEL_TRANSIENT_CAN_DUCK:
                reqApp->sendAudioFocusResult(static_cast<CCtlListener::E_AVOUT>(output), CCtlListener::AUDIOFOCUS_GAIN_TRANSIENT);
                break;
            default:
                reqApp->sendAudioFocusResult(static_cast<CCtlListener::E_AVOUT>(output), CCtlListener::AUDIOFOCUS_GAIN);
        }
        m_audiofocusStack.emplace_back(appID, streamType, focusType);
        // if (NULL != m_watcher) {
        //     m_watcher->setProcessFlag(appID, audioFlag);
        // }

        #ifdef CARPLAY_SUPPORT
        if (appID != CAPPBaseObj::APPID_CARPLAY_APP) {
            switch (focusType) {
                case CAPPBaseObj::LEVEL_TRANSIENT:
                    getAppObj(CAPPBaseObj::APPID_CARPLAY_APP)->sendAudioFocusResult(static_cast<CCtlListener::E_AVOUT>(output), CCtlListener::AUDIOFOCUS_LOSS_TRANSIENT);
                    break;
                case CAPPBaseObj::LEVEL_TRANSIENT_CAN_DUCK:
                    getAppObj(CAPPBaseObj::APPID_CARPLAY_APP)->sendAudioFocusResult(static_cast<CCtlListener::E_AVOUT>(output), CCtlListener::AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK);
                    break;
                default:
                    getAppObj(CAPPBaseObj::APPID_CARPLAY_APP)->sendAudioFocusResult(static_cast<CCtlListener::E_AVOUT>(output), CCtlListener::AUDIOFOCUS_LOSS);
            }
        }
        #endif
    } while (0);

    return ret;
}

bool CAVMuteManager::onAudioRel(unsigned char appID, unsigned int data)
{
    bool ret = true;
    CAPPControllerObj *relApp = getAppObj(appID);
    AudioFocusRequestData audioData(data);
    unsigned int output = audioData.getOutput();
    unsigned int focusType = audioData.getFocusType();
    unsigned int streamType = audioData.getStreamType();
    LOGD(TAG, "onAudioFocus release, app(%d) output(%s), focusType(%s), streamType(%s)\n", appID,
            AudioFocusRequestData::decodeOutput(output), AudioFocusRequestData::decodeFocusType(focusType), AudioFocusRequestData::decodeStreamType(streamType));

    do {
        auto it = std::find_if(m_audiofocusStack.begin(), m_audiofocusStack.end(),
            [appID, streamType](const AudiofocusStackItem& item){
                return item.appID == appID && item.streamType == streamType;
            });
        if (it == m_audiofocusStack.end()) {
            LOGD(TAG, "audiofocusStack is empty or appid doesn't exist");
            ret = false;
            break;
        } else if (m_audiofocusStack.size() == 1) {
            m_audiofocusStack.clear();
            #ifdef CARPLAY_SUPPORT
            getAppObj(CAPPBaseObj::APPID_CARPLAY_APP)->sendAudioFocusResult(static_cast<CCtlListener::E_AVOUT>(output), CCtlListener::AUDIOFOCUS_NONE);
            #endif
            // if (NULL != m_watcher) {
            //     m_watcher->rmProcessFlag(appID, audioFlag);
            // }
            break;
        } else {
            // if (NULL != m_watcher) {
            //     m_watcher->rmProcessFlag(appID, audioFlag);
            // }
            m_audiofocusStack.remove_if([appID, streamType](const AudiofocusStackItem& item){
                return item.appID == appID && item.streamType == streamType;
            });
        }

        switch(m_audiofocusStack.back().focusType) {
            case CAPPBaseObj::LEVEL_TRANSIENT:
            case CAPPBaseObj::LEVEL_TRANSIENT_CAN_DUCK:
                getAppObj(m_audiofocusStack.back().appID)->sendAudioFocusResult(CCtlListener::AVOUT_F, CCtlListener::AUDIOFOCUS_GAIN_TRANSIENT);
                break;
            default:
                getAppObj(m_audiofocusStack.back().appID)->sendAudioFocusResult(CCtlListener::AVOUT_F, CCtlListener::AUDIOFOCUS_GAIN);
        }

        #ifdef CARPLAY_SUPPORT
        if (m_audiofocusStack.back().appID != CAPPBaseObj::APPID_CARPLAY_APP) {
            switch(m_audiofocusStack.back().focusType) {
                case CAPPBaseObj::LEVEL_TRANSIENT:
                case CAPPBaseObj::LEVEL_TRANSIENT_CAN_DUCK:
                    getAppObj(CAPPBaseObj::APPID_CARPLAY_APP)->sendAudioFocusResult(CCtlListener::AVOUT_F, CCtlListener::AUDIOFOCUS_LOSS_TRANSIENT);
                    break;
                default:
                    getAppObj(CAPPBaseObj::APPID_CARPLAY_APP)->sendAudioFocusResult(CCtlListener::AVOUT_F, CCtlListener::AUDIOFOCUS_LOSS);
            }
        }
        #endif
    } while (0);

    return ret;
}

bool CAVMuteManager::onVideoReq(unsigned char appID, unsigned int data)
{
    bool ret = false;
    CAPPControllerObj *reqApp = getAppObj(appID);
    CAPPControllerObj *videoApp = NULL;
    //todo:out and param should decorder from data,same as audiofocus request
    int vOut = CCtlListener::AVOUT_F;
    int param = 0;
    LOGD(TAG, "onVideoReq app(%d), param %d\n", appID, param);

    do {
        if (NULL == reqApp) {
            LOGE(TAG, "%s: get app obj id(%d) fail! vOut(%d) param(%d)\n",
                    __func__, appID, vOut, param);
            ret = false;
            break;
        }

        if (m_frontVideoRes.has(appID)) {
            LOGW(TAG, "%s already request video\n", reqApp->getAPPName());
            ret = false;
            break;
        }

        // get current video app.
        if (vOut == CCtlListener::AVOUT_F) {
            videoApp = getAppObj(m_frontVideoRes.back());
        } else if (vOut == CCtlListener::AVOUT_R) {
            // TODO:
        } else {
            LOGW(TAG, "tell %s no such vOut(%d)!\n", reqApp->getAPPName(), vOut);
            ret = false;
            break;
        }

        // process normal video
        if (NULL != videoApp) {
            //check  priority.
            if (videoApp->getPriority() < reqApp->getPriority()) {
                LOGW(TAG, "priority higher APP(%s) has video, %s can't req!\n",
                            videoApp->getAPPName(), reqApp->getAPPName());
                ret = false;
            } else if (videoApp->getPriority() == reqApp->getPriority()) {
                // only current active app can apply for video
                CAPPControllerObj *activeApp = getActiveApp();
                if (NULL != activeApp && reqApp != activeApp) {
                    LOGW(TAG, "only active APP(%s) can apply for video!\n",
                            activeApp->getAPPName());
                    ret = false;
                } else if (NULL == activeApp) {
                    ret = false;
                } else {
                    // NULL != activeApp && reqApp == activeApp
                    ret = true;
                }
            } else {
                ret = true;
            }

            if (!ret) {
                if (vOut == CCtlListener::AVOUT_F) {
                    m_frontVideoRes.push_back(appID, 1);
                } else {
                // TODO:
                }
                ret = true;
                break;
            }

            // check if need resume
            if (reqApp->needResume()) {
            }

            //current video loss foucs.
            ret = lossVideo(INVALID_ID, param);
        } else {
            // videoApp == NULL
        }

        //focus video.
        ret = gainVideo(appID, param);
    } while (0);

    return ret;
}

bool CAVMuteManager::onVideoRel(unsigned char appID, unsigned int data)
{
    bool ret = false;
    CAPPControllerObj *relApp = NULL;
    //todo:out and param should decorder from data,same as audiofocus request
    int aOut = CCtlListener::AVOUT_F;
    int param = 0;
    LOGD(TAG, "onVideoRel app(%d), param %d\n", appID, param);

    do {
        if (CAPPBaseObj::LEVEL_NORMAL != param) {
            LOGW(TAG, "param(%d) not supported\n", param);
        }

        if (!m_frontVideoRes.has(appID)) {
            LOGE(TAG, "app(%d) doesn't applyFor video yet!\n", appID);
            ret = false;
            break;
        }

        relApp = getAppObj(appID);
        if (NULL == relApp) {
            LOGE(TAG,"get app obj id(%d) fail!\n", appID);
            ret = false;
            break;
        }

        // video focus loss.
        ret = lossVideo(appID, param);
        if (ret) {
            if (aOut == CCtlListener::AVOUT_F) {
                if (m_frontVideoRes.back() == appID) {
                    m_frontVideoRes.remove(appID);
                    if (relApp->needResume()) {
                        unsigned char resumeID = m_frontVideoRes.back();
                        //LOGD(TAG, "m_frontVideoRes.getCurrentUser %d\n", m_frontVideoRes.back());
                        if (INVALID_ID != resumeID) {
                            ret = gainVideo(resumeID, param);
                        }
                    } else {
                        m_frontVideoRes.push_back(INVALID_ID);
                    }
                } else {
                    m_frontVideoRes.remove(appID);
                }
            } else if (aOut == CCtlListener::AVOUT_R) {
                // TODO:
            }
        }
    } while (0);

    return ret;
}

bool CAVMuteManager::onAVReq(unsigned char appID, unsigned int data)
{
    bool ret = false;
    CAPPControllerObj *reqApp = getAppObj(appID);
    CAPPControllerObj *videoApp = NULL;
    bool needLossVideo = false, needGainVideo = false;
    //todo:out and param should decorder from data,same as audiofocus request
    int out = CCtlListener::AVOUT_F;
    int param = 0;

    do {
        onAudioReq(appID, 0);
        if (NULL == reqApp) {
            LOGE(TAG, "%s: get app obj id(%d) fail! out(%d) param(%d)\n",
                    __func__, appID, out, param);
            ret = false;
            break;
        }

        if (m_frontVideoRes.has(appID)) {
            LOGW(TAG, "%s already request video + audio\n", reqApp->getAPPName());
            ret = false;
            break;
        }

        //get current audio & video app.
        if (out == CCtlListener::AVOUT_F) {
            videoApp = getAppObj(m_frontVideoRes.back());
        } else if (out == CCtlListener::AVOUT_R) {
            // TODO:
        } else {
            LOGW(TAG, "tell %s no such out(%d)!\n", reqApp->getAPPName(), out);
            ret = false;
            break;
        }

        if (NULL != videoApp) {
            //check  priority.
            if (videoApp->getPriority() < reqApp->getPriority()) {
                LOGW(TAG, "priority higher APP(%s) has video, %s can't req!\n",
                            videoApp->getAPPName(), reqApp->getAPPName());
                ret = false;
            } else if (videoApp->getPriority() == reqApp->getPriority()) {
                // only current active app can apply for video
                CAPPControllerObj *activeApp = getActiveApp();
                if (NULL != activeApp && reqApp != activeApp) {
                    LOGW(TAG, "only active APP(%s) can apply for video!\n",
                            activeApp->getAPPName());
                    ret = false;
                } else if (NULL == activeApp) {
                    ret = false;
                } else {
                    // NULL != activeApp && reqApp == activeApp
                    ret = true;
                }
            } else {
                ret = true;
            }

            if (!ret) {
                if (out == CCtlListener::AVOUT_F) {
                    m_frontVideoRes.push_back(appID, 1);
                } else {
                // TODO:
                }
                ret = true;
                needLossVideo = false;
                needGainVideo = false;
            } else {
                needLossVideo = true;
                needGainVideo = true;
            }
        } else {
            // videoApp == NULL
            needLossVideo = false;
            needGainVideo = true;
        }

        //current video loss foucs.
        if (needLossVideo)
            lossVideo(INVALID_ID, param);

        //focus video.
        if (needGainVideo)
            ret = gainVideo(appID, param);
        reqApp->setAVApp(true);
    } while (0);

    return ret;
}

bool CAVMuteManager::onAVRel(unsigned char appID, unsigned int data)
{
    bool ret = false;
    CAPPControllerObj *relApp = NULL;

    //todo:out and param should decorder from data,same as audiofocus request
    int aOut = CCtlListener::AVOUT_F;
    int param = 0;

    do {
        ret = onAudioRel(appID, 0);
        relApp = getAppObj(appID);
        if (NULL == relApp) {
            LOGE(TAG, "get app obj id(%d) fail!\n", appID);
            ret = false;
            break;
        }
        relApp->setAVApp(false);

        //todo:
        if (!ret) {
            break;
        }
        todo:
        ret = onVideoRel(appID, 0);
        if (!ret) {
            break;
        }
    } while (0);

    return ret;
}

/*
bool CAVMuteManager::onGoHome(unsigned char appID, unsigned char param)
{
    bool ret = false;
    CAPPControllerObj *hideApp = getAppObj(appID);
    CAPPControllerObj *activeObj = getActiveApp();
    LOGD(TAG, "onGoHome app(%d)\n", appID);

    do {
        if (NULL == hideApp) {
            LOGE(TAG,"get app obj id(%d) fail! param(%d)\r\n", appID, param);
            ret = false;
            break;
        }

        bool btphone = true;
#ifndef WITH_SOAPP
        if (activeObj && CAPPBaseObj::APPID_BTPHONE == activeObj->getAppID()) {
            btphone = false;
        }
#endif
        if (activeObj != hideApp && btphone) {
            LOGE(TAG,"active app not appID(%d)! param(%d)\r\n", appID, param);
            ret = false;
            break;
        }

        hideApp->hideFrontUI();
        ret = hideApp->hideFront();
        if (ret) {
            rmActiveApp(appID);
            if (NULL != activeObj && btphone) {
                activeObj->hideFrontUI();
            }
            //cleanActiveApp();
        } else {
            LOGE(TAG, "hideFront fail\n");
            break;
        }

        CAPPControllerObj *homeObj = getAppObj(CAPPBaseObj::APPID_HOME);
        if (homeObj != NULL && getActiveApp() == NULL) {
            homeObj->showFrontUI();

            if (NULL != m_watcher) {
                m_watcher->addProcess(CAPPBaseObj::APPID_HOME);
            }
            ret = homeObj->showFront();
            if (!ret) {
                LOGE(TAG, "%s showFront fail\n", homeObj->getAPPName());
                break;
            }
        } else {
            ret = false;
            break;
        }
    } while (0);

    return ret;
}
*/

bool CAVMuteManager::onGoHome(unsigned char appID, unsigned int data)
{
    bool ret = false;
    unsigned char hideID;

    CAPPControllerObj *hideApp = getAppObj(appID);
    LOGD(TAG, "onGoHome app(%d), hideID(%d)\n", appID, hideID);

    do {
        if (NULL == hideApp) {
            LOGE(TAG,"get app obj id(%d) fail! param(%d)\r\n", hideID, data);
            ret = false;
            break;
        }

        hideApp->hideFrontUI();
        ret = hideApp->hideFront();
        if (ret) {
            rmActiveApp(hideID);
        } else {
            LOGE(TAG, "hideFront fail\n");
            break;
        }

        onShowFront(CAPPBaseObj::APPID_HOME, 0);
    } while (0);

    return ret;
}

//todo:keyevent eg.volumekey
bool CAVMuteManager::onKeyEvent(const CCmdTask &cmdTask)
{
    // CAPPControllerObj *showApp = NULL;

    // LOGE(TAG, "---onKeyEvent param(%d)\n", param);
    // showApp = getAppObj(appID);
    // if (NULL == showApp) {
    //     LOGE(TAG, "get app obj (%d) fail! param(%d)\r\n",
    //                 appID, param);
    //     return false;
    // }

    /* CAPPControllerObj *activeObj = NULL;
    activeObj = getActiveApp();
    if (showApp == activeObj) {
        LOGI(TAG, "APP %s in front!\n", showApp->getAPPName());
        return showApp->keyEvent(event, param, param, 0);
    }

    return false; */

    //return showApp->keyEvent(event, param, param, 0);
    return true;
}


void CAVMuteManager::exitAllApp()
{
    bool ret = false;
    int res = -1;
    int appID = INVALID_ID;
    struct timeval tv_start = {0, 0}, tv_end = {0, 0};

    res = gettimeofday(&tv_start, NULL);
    if (-1 == res) {
        LOGE(TAG, "gettimeofday fail: %s\n", strerror(errno));
    }

    LOGD(TAG, "exit all APP start\n");
    struct timeval tv_app_start = tv_start, tv_app_end = tv_start;
    list<CAPPControllerObj *>::reverse_iterator rit;
    while (!m_appObjList.empty()) {
        tv_app_start = tv_app_end;

        rit = m_appObjList.rbegin();
        appID = (*rit)->getAppID();
        ret = (*rit)->exit();
        if (!ret) {
            LOGE(TAG, "app %s exit fail\n", (*rit)->getAPPName());
        }

        res = gettimeofday(&tv_app_end, NULL);
        if (-1 == res) {
            LOGE(TAG, "gettimeofday fail: %s\n", strerror(errno));
        } else {
            LOGD(TAG, "APP %s's exit take %ldus\n", (*rit)->getAPPName(),
                (tv_app_end.tv_sec - tv_app_start.tv_sec) * 1000000
                + tv_app_end.tv_usec - tv_app_start.tv_usec);
        }

        removeApp(appID);
    }

    res = gettimeofday(&tv_end, NULL);
    if (-1 == res) {
        LOGE(TAG, "gettimeofday fail: %s\n", strerror(errno));
    }

    LOGD(TAG, "exit all APP end, take %ldus in total\n",
        (tv_end.tv_sec - tv_start.tv_sec) * 1000000
        + tv_end.tv_usec - tv_start.tv_usec);

    system("systemctl poweroff");
}

bool CAVMuteManager::sendCmdToAPP(CAPPControllerObj *targetAppObj,
                                    const CCmdTask &cmdTask)
{
    bool ret = false;

    if (NULL != targetAppObj) {
        //todo:(cmdTask.getData() >> 24) change to get state()
        //oldparam 2 is unuse
        ret = targetAppObj->keyEvent(targetAppObj->makeKey(cmdTask.getMainFunc(), cmdTask.getSubFunc()),
                                        (cmdTask.getData() >> 24) & 0xFF,
                                        0,
                                        true);
    } else {
        LOGE(TAG, "No targetAppObj!\n");
        ret = false;
    }

    return ret;
}

bool CAVMuteManager::processSubFuncKey(const CCmdTask &cmdTask)
{
    // bool ret = false;
    // CAPPControllerObj *targetAPPObj = NULL;
    // CCmdTask::E_SUBFUNC_KEY key = (CCmdTask::E_SUBFUNC_KEY)cmdTask.getSubFunc();
    // unsigned char appID = INVALID_ID;

    // switch (key) {
    // case CCmdTask::SUBFUNC_KEY_MENU:
    // case CCmdTask::SUBFUNC_KEY_HOME:
    //     targetAPPObj = getActiveApp();
    //     if (NULL == targetAPPObj)
    //         ret = true;
    //     else if (targetAPPObj->getPriority() < CAPPBaseObj::APP_PRIORITY_DEFAULT) {
    //         LOGW(TAG, "high priority app(%s) is in front, should not go home\n",
    //             targetAPPObj->getAPPName());
    //         ret = true;
    //     } else
    //         ret = onGoHome(targetAPPObj->getAppID(), CAPPBaseObj::LEVEL_NORMAL);

    //     break;

    // case CCmdTask::SUBFUNC_KEY_PREVIOUS:
    // case CCmdTask::SUBFUNC_KEY_NEXT:
    // case CCmdTask::SUBFUNC_KEY_FF:
    // case CCmdTask::SUBFUNC_KEY_FW:
    // case CCmdTask::SUBFUNC_KEY_PLAYPAUSE:
    // case CCmdTask::SUBFUNC_KEY_STOP:
    //     targetAPPObj = getAppObj(m_frontAudioRes.back());
    //     if (NULL == targetAPPObj) {
    //          LOGE(TAG, "no front audio app\n");
    //     }
    //     else {
    //         ret = sendCmdToAPP(targetAPPObj, cmdTask);
    //         if (!ret) {
    //             LOGE(TAG, "sendAutoTestCmdTo FrontAudioApp %s ERROR!\n", targetAPPObj->getAPPName());
    //         }
    //     }
    //     break;

    // case CCmdTask::SUBFUNC_KEY_MODE:
    // case CCmdTask::SUBFUNC_KEY_GPS:
    //     LOGE(TAG, "We don't support SubFunc_key %d yet!\n", key);
    //     break;
    // case CCmdTask::SUBFUNC_KEY_DVD:
    //     ret = onRun(CAPPBaseObj::APPID_DVP, 0);
    //     break;
    // case CCmdTask::SUBFUNC_KEY_FMBAND:
    // case CCmdTask::SUBFUNC_KEY_EJECT:
    // case CCmdTask::SUBFUNC_KEY_SD:
    // case CCmdTask::SUBFUNC_KEY_USB:
    //     LOGE(TAG, "We don't support SubFunc_key %d yet!\n", key);
    //     break;

    // case CCmdTask::SUBFUNC_KEY_BT:
    //     targetAPPObj = getActiveApp();
    //     if (NULL != targetAPPObj
    //         && (targetAPPObj->getAppID() == CAPPBaseObj::APPID_BT
    //             || targetAPPObj->getAppID() == CAPPBaseObj::APPID_BTPHONE)) {
    //         CMDPacket cmd(cmdTask.getSyncCount(),
    //                     CCmdTask::MAIN_FUNC_BT,
    //                     CCmdTask::SUBFUNC_BT_DIALING,
    //                     0,
    //                     0,
    //                     cmdTask.getRetType());
    //         CCmdTask task(cmd);

    //         targetAPPObj = getAppObj(CAPPBaseObj::APPID_BT);
    //         ret = sendCmdToAPP(targetAPPObj, task);
    //         if (!ret) {
    //             LOGI(TAG, "%s don't process this cmd\n", targetAPPObj->getAPPName());
    //         }
    //     } else {
    //         // bt is not in front, just show bt
    //         ret = onRun(CAPPBaseObj::APPID_BT, 0);
    //     }
    //     break;

    // case CCmdTask::SUBFUNC_KEY_AVIN:
    //     ret = onRun(CAPPBaseObj::APPID_AVIN, 0);
    //     break;

    // case CCmdTask::SUBFUNC_KEY_SETTING:
    //     appID = CAPPBaseObj::APPID_SETTING;
    //     targetAPPObj = getActiveApp();
    //     if (NULL == targetAPPObj || appID != targetAPPObj->getAppID())
    //         ret = onRun(appID, 0);
    //     else
    //         ret = onHideFront(appID, 0);
    //     break;

    // case CCmdTask::SUBFUNC_KEY_CMMB:
    // case CCmdTask::SUBFUNC_KEY_CALC:
    //     LOGE(TAG, "We don't support SubFunc_key %d yet!\n", key);
    //     break;

    // case CCmdTask::SUBFUNC_KEY_BACK:
    //     targetAPPObj = getActiveApp();
    //     if (NULL != targetAPPObj) {
    //         ret = sendCmdToAPP(targetAPPObj, cmdTask);
    //         if (!ret) {
    //             LOGI(TAG, "%s don't support SUBFUNC_KEY_BACK\n", targetAPPObj->getAPPName());

    //             if (!(targetAPPObj->getPriority() < CAPPBaseObj::APP_PRIORITY_DEFAULT)) {
    //                 ret = onGoHome(targetAPPObj->getAppID(), 0);
    //             } else {
    //                 LOGW(TAG, "high priority app(%s) is in front, should not back\n",
    //                     targetAPPObj->getAPPName());
    //                 ret = true;
    //             }
    //         }
    //     }

    //     break;

    // case CCmdTask::SUBFUNC_KEY_CLOSE:
    //     targetAPPObj = getActiveApp();
    //     if (NULL != targetAPPObj) {
    //         if (!(targetAPPObj->getPriority() < CAPPBaseObj::APP_PRIORITY_DEFAULT)) {
    //             ret = onExit(targetAPPObj->getAppID(), 0);
    //         } else {
    //             LOGW(TAG, "high priority app(%s) is in front, should not close\n",
    //                 targetAPPObj->getAPPName());
    //             ret = true;
    //         }
    //     }
    //     break;

    // case CCmdTask::SUBFUNC_KEY_ACCOFF:
    // case CCmdTask::SUBFUNC_KEY_ACCON:
    // case CCmdTask::SUBFUNC_KEY_CDC:
    //     LOGE(TAG, "We don't support SubFunc_key %d yet!\n", key);
    //     break;

    // case CCmdTask::SUBFUNC_KEY_IPOD:
    //     ret = onRun(CAPPBaseObj::APPID_IPOD, 0);
    //     break;
    // case CCmdTask::SUBFUNC_KEY_MHL:
    //     ret = onRun(CAPPBaseObj::APPID_MHL, 0);
    //     break;
    //  case CCmdTask::SUBFUNC_KEY_BTEM:
    //     ret = onRun(CAPPBaseObj::APPID_BTEM, 0);
    //     break;
    // case CCmdTask::SUBFUNC_KEY_VIDEO:
    //     ret = onRun(CAPPBaseObj::APPID_MMP_VIDEO, 0);
    //     break;
    // case CCmdTask::SUBFUNC_KEY_AUDIO:
    //     ret = onRun(CAPPBaseObj::APPID_MMP_AUDIO, 0);
    //     break;
    // case CCmdTask::SUBFUNC_KEY_PICTURE:
    //     ret = onRun(CAPPBaseObj::APPID_MMP_PIC, 0);
    //     break;

    // case CCmdTask::SUBFUNC_KEY_LAMPON:
    // case CCmdTask::SUBFUNC_KEY_LAMPOFF:
    // // move to below
    // //case CCmdTask::SUBFUNC_KEY_MUTE_UNMUTE:
    // case CCmdTask::SUBFUNC_KEY_EQ:
    // case CCmdTask::SUBFUNC_KEY_BRAKE_ON:
    // case CCmdTask::SUBFUNC_KEY_BRAKE_OFF:
    // case CCmdTask::SUBFUNC_KEY_TURN_ON_SCREEN:
    // case CCmdTask::SUBFUNC_KEY_TURN_OFF_SCREEN:
    //     LOGE(TAG, "We don't support SubFunc_key %d yet!\n", key);
    //     break;

    // case CCmdTask::SUBFUNC_KEY_POWER_OFF:
    //     /*ret = SetBklShutDown(1);
    //     if (0 != ret) {
    //         LOGE(TAG, "SetBklShutDown fail\n");
    //     }
    //     exitAllApp();*/
    //     ret = true;
    //     break;
    // //move to menu
    // //case CCmdTask::SUBFUNC_KEY_HOME:
    // case CCmdTask::SUBFUNC_KEY_MUTE_UNMUTE:
    // case CCmdTask::SUBFUNC_KEY_FRONT_VOLUME_INC:
    // case CCmdTask::SUBFUNC_KEY_FRONT_VOLUME_DEC: {
    //     targetAPPObj = getActiveApp();
    //     if (NULL == targetAPPObj) {
    //         targetAPPObj = getAppObj(CAPPBaseObj::APPID_HOME);
    //     }

    //     if (targetAPPObj->getAppID() == CAPPBaseObj::APPID_BACKCAR) {
    //         LOGD(TAG, "%s is active, don't process volume %d\n",
    //             targetAPPObj->getAPPName(), key);
    //         ret = true;
    //         break;
    //     }

    //     CMDPacket cmd(cmdTask.getSyncCount(),
    //                     cmdTask.getMainFunc(),
    //                     cmdTask.getSubFunc(),
    //                     cmdTask.getAppID(),
    //                     cmdTask.getData(),
    //                     cmdTask.getRetType());
    //     switch (targetAPPObj->getAppID()) {
    //         case CAPPBaseObj::APPID_BTPHONE:
    //             cmd.m_param1 = CCtlListener::VOLUME_BT;
    //             break;
    //         case CAPPBaseObj::APPID_NAVI:
    //             cmd.m_param1 = CCtlListener::VOLUME_GIS;
    //             break;
    //         default:
    //             cmd.m_param1 = CCtlListener::VOLUME_MEDIA;
    //             break;
    //     }

    //     CCmdTask task(cmd);

    //     ret = sendCmdToAPP(targetAPPObj, task);
    //     if (!ret && targetAPPObj->getAppID() != CAPPBaseObj::APPID_HOME) {
    //         targetAPPObj = getAppObj(CAPPBaseObj::APPID_HOME);
    //         ret = sendCmdToAPP(targetAPPObj, task);
    //     }
    //     break;
    // }

    // case CCmdTask::SUBFUNC_KEY_WIPER_FAST:
    // case CCmdTask::SUBFUNC_KEY_WIPER_MIDDLE:
    // case CCmdTask::SUBFUNC_KEY_WIPER_CLOSE:
    // case CCmdTask::SUBFUNC_KEY_WINDOW_LEFT_OPEN:
    // case CCmdTask::SUBFUNC_KEY_WINDOW_LEFT_CLOSE:
    // case CCmdTask::SUBFUNC_KEY_WINDOW_RIGHT_OPEN:
    // case CCmdTask::SUBFUNC_KEY_WINDOW_RIGHT_CLOSE:
    // case CCmdTask::SUBFUNC_KEY_HAZARD_LIGHT_OPEN:
    // case CCmdTask::SUBFUNC_KEY_HAZARD_LIGHT_CLOSE:
    // case CCmdTask::SUBFUNC_KEY_TURN_LEFT_SIGNAL_OPEN:
    // case CCmdTask::SUBFUNC_KEY_TURN_RIGHT_SIGNAL_OPEN:
    // case CCmdTask::SUBFUNC_KEY_HAZARD_LIGHT_FLICKER:
    //     targetAPPObj = getAppObj(CAPPBaseObj::APPID_BCM);
    //     if (NULL != targetAPPObj) {
    //         ret = sendCmdToAPP(targetAPPObj, cmdTask);
    //     } else
    //         LOGW(TAG, "APP BCM is not running\n");
    //     break;

    // default:
    //     LOGE(TAG, "we don't support SubFuncKey %d yet!\n", key);
    //     break;
    // }

    // return ret;
    return true;
}

bool CAVMuteManager::processAutoTestCmdBackCar(const CCmdTask &cmdTask)
{
    // bool ret = false;
    // CCmdTask::E_SUBFUNC_BACKCAR subFunc = (CCmdTask::E_SUBFUNC_BACKCAR)
    //                                             cmdTask.getSubFunc();
    // CAPPControllerObj *backcarObj = getAppObj(CAPPBaseObj::APPID_BACKCAR);
    // if (NULL == backcarObj) {
    //     LOGE(TAG, "no backcar appobj\n");
    // }
    // else {
    //     switch (subFunc) {
    //     case CCmdTask::SUBFUNC_BACKCAR_START:
    //         ret = onRun(CAPPBaseObj::APPID_BACKCAR, 0);
    //         ret = onVideoReq(CAPPBaseObj::APPID_BACKCAR, 0, 0);
    //         ret = onAudioReq(CAPPBaseObj::APPID_BACKCAR, 0, CAPPBaseObj::LEVEL_TRANSIENT);
    //         break;
    //     case CCmdTask::SUBFUNC_BACKCAR_STOP:
    //         ret = onHideFront(CAPPBaseObj::APPID_BACKCAR, 0);
    //         ret = onVideoRel(CAPPBaseObj::APPID_BACKCAR, 0, 0);
    //         ret = onAudioRel(CAPPBaseObj::APPID_BACKCAR, 0, CAPPBaseObj::LEVEL_TRANSIENT);
    //         break;
    //     case CCmdTask::SUBFUNC_BACKCAR_DISTANCE:
    //     case CCmdTask::SUBFUNC_BAKCCAR_TURNLEFT:
    //     case CCmdTask::SUBFUNC_BAKCCAR_TURNRIGHT:
    //         ret = sendCmdToAPP(backcarObj, cmdTask);
    //         if (!ret) {
    //             LOGE(TAG, "sendCmdToAPP BACKCAR ERROR!\n");
    //         }
    //         break;
    //     default:
    //         ret = false;
    //         LOGE(TAG, "No backcar subfunc %d!\n", subFunc);
    //         break;
    //     }
    // }

    // return ret;
    return true;
}

bool CAVMuteManager::processAutoTestCmd(const CCmdTask &cmdTask)
{
    // bool ret = false;
    // CAPPControllerObj *targetAPPObj = NULL;
    // const int mainFunc = cmdTask.getMainFunc();
    // unsigned char appID = INVALID_ID;

    // switch (mainFunc) {
    // case CCmdTask::MAIN_FUNC_CDDVD:
    // case CCmdTask::MAIN_FUNC_USB:
    // case CCmdTask::MAIN_FUNC_SD:
    // case CCmdTask::MAIN_FUNC_FM:
    // case CCmdTask::MAIN_FUNC_AM:
    // case CCmdTask::MAIN_FUNC_CMMB:
    //     LOGE(TAG, "We don't support MainFunc%d yet!\n", mainFunc);
    //     break;

    // case CCmdTask::MAIN_FUNC_AVIN:
    //     targetAPPObj = getAppObj(CAPPBaseObj::APPID_AVIN);
    //     ret = sendCmdToAPP(targetAPPObj, cmdTask);
    //     if (!ret) {
    //         LOGE(TAG, "sendCmdToAPP AVIN ERROR!\n");
    //     }
    //     break;

    // case CCmdTask::MAIN_FUNC_CDC:
    // case CCmdTask::MAIN_FUNC_IPOD:
    //     LOGE(TAG, "We don't support MainFunc%d yet!\n", mainFunc);
    //     break;

    // case CCmdTask::MAIN_FUNC_BT:
    //     targetAPPObj = getAppObj(CAPPBaseObj::APPID_BT);
    //     ret = sendCmdToAPP(targetAPPObj, cmdTask);
    //     if (!ret) {
    //         LOGE(TAG, "sendCmdToAPP BT ERROR!\n");
    //     }
    //     break;

    // case CCmdTask::MAIN_FUNC_NAVI_FUNC:
    // case CCmdTask::MAIN_FUNC_STEERING_WHEEL:
    //     LOGE(TAG, "We don't support MainFunc%d yet!\n", mainFunc);
    //     break;

    // case CCmdTask::MAIN_FUNC_BACKCAR:
    //     ret = processAutoTestCmdBackCar(cmdTask);
    //     if (!ret) {
    //         LOGE(TAG, "processAutoTestCmdBackCar ERROR!\n");
    //     }
    //     break;

    // case CCmdTask::MAIN_FUNC_TIRE_PRESSURE:
    // case CCmdTask::MAIN_FUNC_MAIN_VOLUME:
    // case CCmdTask::MAIN_FUNC_SOUND_EFFECT:
    // case CCmdTask::MAIN_FUNC_DISPLAY:
    // case CCmdTask::MAIN_FUNC_FACTORY:
    // case CCmdTask::MAIN_FUNC_PUSH_TO_TALK:
    // case CCmdTask::MAIN_FUNC_SPEECH_RECOGNITION:
    // case CCmdTask::MAIN_FUNC_RESET:
    // case CCmdTask::MAIN_FUNC_CALIBRATE:
    //     LOGE(TAG, "We don't support MainFunc%d yet!\n", mainFunc);
    //     break;

    // case CCmdTask::MAIN_FUNC_GPS:
    //     appID = CAPPBaseObj::APPID_NAVI;
    //     targetAPPObj = getActiveApp();
    //     if (NULL == targetAPPObj || appID != targetAPPObj->getAppID())
    //         ret = onRun(appID, 0);
    //     else
    //         ret = onHideFront(appID, 0);

    //     break;

    // case CCmdTask::MAIN_FUNC_STEERING_WHEEL_STUDY:
    // case CCmdTask::MAIN_FUNC_BACKCAR_LOCUS:
    //     LOGE(TAG, "We don't support MainFunc%d yet!\n", mainFunc);
    //     break;

    // case CCmdTask::MAIN_FUNC_KEY:
    //     ret = processSubFuncKey(cmdTask);
    //     if (!ret) {
    //         LOGE(TAG, "processSubFuncKey ERROR!\n");
    //     }
    //     break;

    // default:
    //     LOGE(TAG, "We have not defined MainFunc %d yet!\n", mainFunc);
    //     break;
    // }

    // return ret;
    return true;
}

bool CAVMuteManager::notifyAllApp(const CCmdTask &cmdTask)
{
    bool ret = true;

    for (std::list<CAPPControllerObj *>::const_iterator iter = m_appObjList.begin();
            iter != m_appObjList.end();
            iter++) {
        ret = sendCmdToAPP(*iter, cmdTask);
        if (!ret) {
            LOGI(TAG, "APP %s don't care this message\n", (*iter)->getAPPName());
            //break;
        }
    }

    return true;
}

//only use to jump home clock to setting
bool CAVMuteManager::processAppJump(const CCmdTask &cmdTask)
{
    // bool ret = false;

    // ret = onRun(cmdTask.getDataParam1(), cmdTask.getDataParam2());
    // if (!ret) {
    //     LOGE(TAG, "app(%d) jump to app(%d) fail\n",
    //             cmdTask.getSubFunc(), cmdTask.getDataParam1());
    // }

    // return ret;
    return true;
}

bool CAVMuteManager::onMiscRequest(const CCmdTask &cmdTask)
{
    bool ret = false;
    static bool arm2Backcar = false;

    switch (cmdTask.getSubFunc()) {
        case GlobalBus::ACTION_ARM2_BACKCAR_IN:
            if (!arm2Backcar) {
                arm2Backcar = true;
                //todo:
                ret = onAudioReq(CAPPBaseObj::APPID_BACKCAR, 0);
                ret = onVideoReq(CAPPBaseObj::APPID_BACKCAR, 0);
            }
            break;

        case GlobalBus::ACTION_ARM2_BACKCAR_OUT: {
            if (arm2Backcar) {
                arm2Backcar = false;
                //todo:
                ret = onAudioRel(CAPPBaseObj::APPID_BACKCAR, 0);
                ret = onVideoRel(CAPPBaseObj::APPID_BACKCAR, 0);

                if (-1 == m_pid) {
                    m_pid = fork();
                    if (-1 == m_pid) {
                        LOGE(TAG, "Fork ERROR!\n");
                    } else if (0 == m_pid) {
                        if (execl(autotestPath, "autotestclient", NULL) < 0) {
                            LOGE(TAG, "execl %s fail : %s\n", autotestPath, strerror(errno));
                            exit(-1);
                        }
                    }
                } else {
                    LOGD(TAG, "autotestclient already exist\n");
                }

                // run boot start app after arm2 backcar
                m_bootList.insert(m_bootList.begin(), m_bootList_af.begin(), m_bootList_af.end());
                GlobalBus::applyFor(GlobalBus::ACTION_RUN, m_bootList_af.front(), 0);

                // send arm2 backcar close msg to home for OT
                CAPPControllerObj *homeobj = getAppObj(CAPPBaseObj::APPID_HOME);
                ret = sendCmdToAPP(homeobj, cmdTask);
            }
            break;
        }

        default:
            LOGE(TAG, "unsupported cmd\n");
            break;
    }

    return ret;
}

CAPPControllerObj *CAVMuteManager::newAppObj(unsigned char appID) const
{
    CAPPControllerObj* obj = NULL;

    if (m_objFactory != NULL) {
        obj = m_objFactory->createObj(appID);
        if (NULL == obj) {
            LOGE(TAG, "createObj %d, return NULL\n", appID);
        }
    }

    return obj;
}

bool CAVMuteManager::releaseAppObj(CAPPControllerObj* appObj)
{
    bool ret = false;

    if (NULL == appObj) {
        LOGE(TAG,"releaseAppObj appObj()!\r\n");
    } else {
        ret = true;
    }

    if (ret && m_objFactory != NULL) {
        ret = m_objFactory->destroyObj(appObj);
        if (!ret) {
            LOGE(TAG, "destroyObj %d fail\n", appObj->getAppID());
        }
    }

    return ret;
}

bool CAVMuteManager::isRunning(unsigned char appID) const
{
    bool ret = false;
    CAPPControllerObj *obj = getAppObj(appID);

    if (NULL != obj) {
        ret = obj->isRuning();
    } else {
        LOGE(TAG, "getAppObj(%d) return NULL\n", appID);
    }

    return ret;
}

bool CAVMuteManager::addApp(CAPPControllerObj *appObj)
{
    bool ret = false;

    if (NULL != appObj) {
        m_appObjList.push_back(appObj);
        ret = true;
    } else {
        LOGE(TAG, "addApp fail! appObj is NULL\n");
    }

    return ret;
}

CAPPControllerObj *CAVMuteManager::getAppObj(unsigned char appID) const
{
    CAPPControllerObj *obj = NULL;

    for (std::list<CAPPControllerObj*>::const_iterator iter = m_appObjList.begin();
            iter != m_appObjList.end();
            iter++) {
        if ((*iter)->getAppID() == appID) {
            obj = (*iter);
            break;
        }
    }

    return obj;
}

bool CAVMuteManager::rmApp(unsigned char appID)
{
    bool ret = false;

    for (std::list<CAPPControllerObj*>::iterator iter = m_appObjList.begin();
            iter != m_appObjList.end();
            iter++) {
        if ((*iter)->getAppID() == appID) {
            m_appObjList.erase(iter);
            ret = true;
            break;
        }
    }

    return ret;
}

CAPPControllerObj *CAVMuteManager::getPreActiveApp() const
{
        return m_preActiveApp;
}

void CAVMuteManager::setPreActiveApp(CAPPControllerObj *preActiveApp)
{
    m_preActiveApp = preActiveApp;
}

CAPPControllerObj *CAVMuteManager::getActiveApp() const
{
    CAPPControllerObj *obj = NULL;
    unsigned char appID = 0;

    if (m_actionList.size() != 0) {
        std::list<unsigned char>::const_reverse_iterator iter = m_actionList.rbegin();
        appID = (*iter);
        obj = getAppObj(appID);
    } else {
        LOGD(TAG, "no active APP now\n");
    }

    return obj;
}

CAPPControllerObj *CAVMuteManager::getNoBtPhoneActiveApp() const
{
    CAPPControllerObj *obj = NULL;
    unsigned char appID = 0;

    if (m_actionList.size() != 0) {
        std::list<unsigned char>::const_reverse_iterator iter = m_actionList.rbegin();
        if (CAPPBaseObj::APPID_BTPHONE != (*iter)) {
            appID = (*iter);
            obj = getAppObj(appID);
        } else {
            iter++;
            if (iter != m_actionList.rend()) {
                appID = (*iter);
                obj = getAppObj(appID);
            }
        }
    } else {
        LOGD(TAG, "no active APP now\n");
    }

    return obj;
}

CAPPControllerObj *CAVMuteManager::searchNoBtPhoneActiveApp(unsigned char &appID) const
{
    CAPPControllerObj *obj = NULL;

    if (m_actionList.size() != 0) {
        std::list<unsigned char>::const_iterator iter = m_actionList.begin();
        if (CAPPBaseObj::APPID_BTPHONE != (*iter)) {
            appID = (*iter);
            obj = getAppObj(appID);
        } else {
            iter++;
            if (iter != m_actionList.end()) {
                appID = (*iter);
                obj = getAppObj(appID);
            }
        }
    } else {
        LOGD(TAG, "no active APP now\n");
    }

    return obj;
}

CAPPControllerObj *CAVMuteManager::getNextActiveApp(unsigned char appID)
{
    CAPPControllerObj *obj = NULL;
    list<unsigned char>::reverse_iterator rit;

    for (rit = m_actionList.rbegin(); rit != m_actionList.rend(); rit++) {
        if (*rit == appID) {
            break;
        }
    }

    if (rit != m_actionList.rend()) {
        rit++;
        if (rit != m_actionList.rend()) {
            obj = getAppObj(*rit);
        } else {
            LOGD(TAG, "no active app after app%d\n", appID);
            obj = getAppObj(CAPPBaseObj::APPID_HOME);
        }
    } else {
        LOGE(TAG, "app%d is not active\n", appID);
    }

    return obj;
}

bool CAVMuteManager::cleanActiveApp()
{
    m_actionList.clear();

    return true;
}

bool CAVMuteManager::setActiveApp(unsigned char appID)
{
    for (std::list<unsigned char>::iterator iter = m_actionList.begin();
            iter != m_actionList.end();
            iter++) {
        if ((*iter) == appID) {
            m_actionList.erase(iter);
            break;
        }
    }

    m_actionList.push_back(appID);

    return true;
}

// only for high priority app
bool CAVMuteManager::setActiveAppInline(unsigned char appID)
{
    bool ret = false;
    CAPPControllerObj *reqAPPObj = getAppObj(appID);

    if (NULL == reqAPPObj) {
        LOGE(TAG, "can't get appObj of appID %d!\n", appID);
        return false;
    } else
        ret = true;

    std::list<unsigned char>::reverse_iterator rit;
    for (rit = m_actionList.rbegin(); rit != m_actionList.rend(); rit++) {
        if (*rit == appID)
            break;

        CAPPControllerObj *ritAPPObj = getAppObj(*rit);
        if (NULL != ritAPPObj) {
            if (reqAPPObj->getPriority() < ritAPPObj->getPriority()) {
                m_actionList.insert(rit.base(), appID);
                ret = true;
                break;
            }
        } else {
            LOGE(TAG, "can't get appObj of appID %d in actionList!\n", *rit);
            break;
        }
    }

    if (rit == m_actionList.rend()) {
        m_actionList.push_front(appID);
        ret = true;
    }

    return ret;
}

bool CAVMuteManager::rmActiveApp(unsigned char appID)
{
    bool ret = false;

    for (std::list<unsigned char>::reverse_iterator iter = m_actionList.rbegin();
            iter != m_actionList.rend();
            iter++) {
        if ((*iter) == appID) {
            m_actionList.erase(--iter.base()); //remove
            ret = true;
            break;
        }
    }

    return ret;
}

bool CAVMuteManager::lossVideo(unsigned char appID, unsigned char param)
{
    bool ret = false;
    CAPPControllerObj *lossApp = NULL;

    if (INVALID_ID == appID) {
        appID = m_frontVideoRes.back();
        if (INVALID_ID == appID) {
            return true;
        }
    }

    lossApp = getAppObj(appID);
    if (NULL == lossApp) {
        LOGE(TAG, "can't find app %d to loss video\n", appID);
        ret = false;
    } else {
        ret = true;
    }

    if (ret
        && m_frontVideoRes.back() == appID) {
        CCtlListener::E_VIDEOFOCUS onFocue;

        if (param == CAPPBaseObj::LEVEL_TRANSIENT) {
            onFocue = CCtlListener::VIDEOFOCUS_LOSS_TRANSIENT;
        } else if (param == CAPPBaseObj::LEVEL_TRANSIENT_CAN_DUCK) {
            onFocue = CCtlListener::VIDEOFOCUS_LOSS_TRANSIENT_CAN_DUCK;
        } else {
            onFocue = CCtlListener::VIDEOFOCUS_LOSS;
            //m_frontVideoRes.push_back(INVALID_ID);
        }

        ret = lossApp->requestVideoFocus(CCtlListener::AVOUT_F, onFocue);
        if (!ret) {
            LOGE(TAG, "%s video loss fail!\n", lossApp->getAPPName());
        }
    }

    return ret;
}

bool CAVMuteManager::gainVideo(unsigned char appID, unsigned char param)
{
    bool ret = false;
    CAPPControllerObj *gainApp = getAppObj(appID);

    if (NULL == gainApp) {
        LOGE(TAG, "can't find app %d to gain video\n", appID);
        ret = false;
    } else {
        ret = true;
    }

    if (ret) {
        ret = gainApp->requestVideoFocus(CCtlListener::AVOUT_F,
                                        CCtlListener::VIDEOFOCUS_GAIN);
        if (!ret) {
            LOGE(TAG, "%s video gain fail!\n", gainApp->getAPPName());
        } else {
            m_frontVideoRes.push_back(appID);
        }
    }

    return ret;
}

bool CAVMuteManager::resumeVideoApp(unsigned char appID, unsigned char param)
{
    bool ret = false;

    if (m_frontVideoRes.has(appID)
        && m_frontVideoRes.back() != appID) {
        ret = lossVideo(INVALID_ID, param);
        ret = gainVideo(appID, param);
    } else {
        ret = true;
    }

    return ret;
}

