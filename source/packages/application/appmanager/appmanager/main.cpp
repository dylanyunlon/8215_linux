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

#include <iostream>
#include <fstream>
#include "cautothread.h"
#include <csync.h>
#include <unistd.h>
#include "cappmanager.h"
#include "csocketlistener.h"
#include "memorywatcher.h"
#include "applog.h"
#include <dlfcn.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/resource.h>
#include "cobjfactory.h"
#include <QDateTime>
//#include "bootproflog.h" later see

static const char *TAG = "AppManager_App";
static const char APPXMLFILE[] = "/usr/bin/appobj.xml";
static const char WAYLANDXMLFILE[] = "/app/waylandappobj.xml";

//#define WITH_SOAPP
//using namespace std;
//const static unsigned long waitTimeOut = 0xffffffff;

int startAppManager(CAppManager *&manager, CObjFactory *pFactory)
{
    bool ret = false;

    do {
        if (NULL == pFactory) {
            LOGE(TAG, "invaild pFactory!\n");
            break;
        }

        if (NULL != manager) {
            LOGD(TAG, "CAppManager already exist\n");
            ret = manager->restartTaskWatcher();
            break;
        }

        manager = new CAppManager(pFactory);
        if (NULL == manager) {
            LOGE(TAG, "new CAppManager fail\n");
            break;
        }

        ret = manager->init();
        if (!ret) {
            LOGE(TAG, "manager->init fail\n");
            break;
        }

        ret = manager->startTaskWatcher();
        if (!ret) {
            LOGE(TAG, "manager->startTaskWatcher fail\n");
            break;
        }
    } while (0);

    return ret ? 0 : -1;
}

int startSocketListener(CSocketListener * &listener)
{
    bool ret = false;

    if (NULL == listener) {
        listener = new CSocketListener;
        if (NULL == listener) {
            LOGE(TAG, "new CSocketListener fail\n");
            ret = false;
        } else {
            ret = true;
        }

        if (ret) {
            ret = listener->startListen();
            if (!ret) {
                LOGE(TAG, "listener->startListen fail\n");
            }
        }
    } else {
        LOGD(TAG, "CSocketListener already exist\n");
        ret = true;
    }

    return ret ? 0 : -1;
}

int startMemoryWatcherService(MemoryWatcherService * &watcher, const APPConfig &conf)
{
    int ret = -1;

    if (NULL == watcher) {
        watcher = new MemoryWatcherService(conf);
        if (NULL == watcher) {
            LOGE(TAG, "new MemoryWatcherService fail\n");
        } else {
            ret = 0;
        }
    } else {
        LOGD(TAG, "MemoryWatcherService already exist\n");
    }

    return ret;
}

#ifdef WITH_SOAPP
static const char *MAINAPPLICATION_PATH = "/usr/bin/mainapplication";
static const char *MAINAPPLICATION_ARGV[] = {
    "./mainapplication",
    //"-plugin",
    //"libinput",
    "-platform",
    "eglfs",
    NULL,
};

//void eglfs_main(void)
void eglfs_main(CObjFactory *pFactory)
{
    if (pFactory == NULL) {
        LOGE(TAG, "eglfs_main err,pFactory is NULL\n");
        return;
    }

    int ret = -1;
    CAppManager *manager = NULL;
    CSocketListener *listener = NULL;
    MemoryWatcherService *watcher = NULL;

    while (1) {
#if 1
        int rtn = -1;
        int pid = -1;

        pid = fork();
        if (pid < 0) {
            LOGE(TAG, "fork errorr: %s\n", strerror(errno));
        } else if (pid == 0) {
            ret = execv(MAINAPPLICATION_PATH,
                        const_cast<char *const *>(MAINAPPLICATION_ARGV));
            if (-1 == ret) {
                LOGE(TAG, "execl fail: %s\n", strerror(errno));
                exit(-1);
            }
        } else {
            ret = startAppManager(manager, pFactory);
            if (-1 == ret) {
                LOGE(TAG, "startAppManager fail\n");
                break;
            }

            ret = startSocketListener(listener);
            if (-1 == ret) {
                LOGE(TAG, "startSocketListener fail\n");
                break;
            }

            LOGD(TAG, "My pid is %d, fork %s pid(%d)!\r\n",
                        getpid(), MAINAPPLICATION_PATH, pid);

            const APPConfig *conf = pFactory->getConf();
            if (NULL != conf) {
                ret = startMemoryWatcherService(watcher, *conf);
                if (-1 == ret) {
                    LOGE(TAG, "startMemoryWatcherService fail\n");
                }
            }

            ret = waitpid(pid, &rtn, 0);// should not go out
            if (-1 == ret) {
                LOGE(TAG, "waitpid fail\n");
                break;
            }
            if (WIFEXITED(rtn)) {
                LOGE(TAG, "mainapplication program exits normally:%d\r\n", WEXITSTATUS(rtn));
            } else if (WIFSIGNALED(rtn)) {
                LOGE(TAG, "mainapplication program quits abnormally:%d\r\n", WTERMSIG(rtn));
            } else if (WIFSTOPPED(rtn)) {
                LOGE(TAG, "mainapplication STOPED:%d\r\n", WSTOPSIG(rtn));
            } else {
                LOGE(TAG, "other\r\n");
            }

            LOGE(TAG, "mainapplication process return:%d\r\n", rtn);
        }
#else
        if (manager == NULL) {
        manager = new CAppManager;
        manager->init();
        }

        if (listener == NULL) {
        listener = new CSocketListener;
        listener->startListen();
        }

        manager->startTaskWatcher();

        manager->waitTaskWatcherFinish(0xffffffff);
        sleep(1);
        manager->restartTaskWatcher();
        LOGD(TAG, "restart task!\r\n");
#endif
        system("kill -9 `ps |grep appmanager|grep -v grep|awk '{print $1}'`");
    }

    SAFE_DELETE(manager);
    SAFE_DELETE(listener);
}
#else
#include "globalbus.h"

void wayland_main(CObjFactory *pFactory)
{
    int ret = -1;
    CAppManager *manager = NULL;
    CSocketListener *listener = NULL;
    MemoryWatcherService *watcher = NULL;

    do {
        ret = startAppManager(manager, pFactory);
        if (-1 == ret) {
            LOGE(TAG, "startAppManager fail\n");
            break;
        }

        ret = startSocketListener(listener);
        if (-1 == ret) {
            LOGE(TAG, "startSocketListener fail\n");
            break;
        }

        if (NULL != pFactory->getConf()) {
            ret = startMemoryWatcherService(watcher, *pFactory->getConf());
            if (-1 == ret) {
                LOGE(TAG, "startMemoryWatcherService fail\n");
                break;
            }
        }

        GlobalBus::applyFor(GlobalBus::ACTION_MAINAPP_DONE, 0, 0);

        while (1)
            sleep(60);
    } while (0);
}
#endif

int main(int argc, char *argv[])
{
#if USE_SYSTEM_LOG
    OPENLOG(TAG);
#endif
    universal_utils::CLog::setLogLevel(UNIVERSAL_UTILE_LOG_LEVEL_DEBUG);

    LOGD(TAG, "main start! argc:%d\n", argc);
    //BootprofLog::writeLog("appmanager main start enter"); later see
    CObjFactory objFactory;
    bool res = false;
#ifdef WITH_SOAPP
    res = objFactory.init(APPXMLFILE);
#else
    res = objFactory.init(WAYLANDXMLFILE);
#endif
    if (!res) {
        LOGE(TAG, "m_objFactory->init fail\n");
    }

#ifdef WITH_SOAPP
    LOGD(TAG, "using %s\n", CAppManager::EGLFS);
    eglfs_main(&objFactory);
#else
    LOGD(TAG, "using %s\n", CAppManager::WAYLAND);
    wayland_main(&objFactory);
    exit(0);
#endif

    LOGE(TAG, "main end!\n");

    return -1;
}

