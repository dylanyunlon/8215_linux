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

#include "mainwindow.h"
#include <sys/syscall.h>
#include <assert.h>
#include <pthread.h>
#include <QString>
#include "applog.h"
//#include "bootproflog.h"
using namespace universal_utils;

typedef struct _SOFILE_ITEM {
	char	so_file[50];
	void   *handle;
} SOFILE_ITEM;

static SOFILE_ITEM g_so_file_items[30];
static int g_so_file_count = 0;

extern const char* logtag_mainapp;

typedef void (*soapp_exit_handler)(void *handle, void *param);
typedef void (*soapp_close)(void);
typedef int (*soapp_main)(int argc, char *argv[], soapp_exit_handler exit_handler, void *handle, void *param);
typedef bool (*soapp_isOpenGLWindow)(void);
typedef bool (*soapp_isAvmShow)(void);
typedef void (*soapp_setAvmShow)(bool flag);


const QEvent::Type RUN_QT_SO_FILE = (QEvent::Type)5006;
const QEvent::Type EXIT_QT_SO_FILE = (QEvent::Type)5007;
extern "C" void soapp_exit(void *handle, void *param)
{
    if (MainWindow::getSingletonPtr())
    {
        LOGD(logtag_mainapp, "[MainWindow]soapp_exit:%d\n", (long)handle);
        MainWindow::getSingletonPtr()->exitQtSoFile(handle, param);
    }
}

#define MAIN_APPLICATION_SOCKET_ADDR  "/tmp/mainapplicationSocketAddr"

template<> MainWindow* Singleton<MainWindow>::msSingleton = 0;
static char *AVM_APP_PATH = "/usr/app/libavm_app.so";
MainWindow::MainWindow(QObject *parent)
: QObject(parent)
, m_soHandle(0)
{
    //BootprofLog::writeLog("MainWindow construct enter");
    // memset(&m_launchPacket, 0, sizeof(m_launchPacket));
    // strcpy(m_launchPacket.arg, AVM_APP_PATH);
    // m_launchPacket.cmd = 0;
    // m_launchPacket.size = 264;
    // runQtSoFile(NULL);
    //BootprofLog::writeLog("MainWindow construct leave");
}

MainWindow::~MainWindow()
{
}

bool MainWindow::exitQtSoFile(void *handle, void *param)
{
    m_lockPacket.lock();
    m_soHandle = handle;

    if (param) {
    }
    LOGD(logtag_mainapp, "[MainWindow]exitQtSoFile:%d\n", (long)m_soHandle);
    QApplication::postEvent(this, new QEvent(EXIT_QT_SO_FILE));

    m_lockPacket.unlock();

    return true;
}

void *get_so_handle(char *so_file) {
	for (int i = 0; i < g_so_file_count; i++) {
		if (!strcmp(so_file, g_so_file_items[i].so_file)) {
			return (g_so_file_items[i].handle);
		}
	}

	return (NULL);
}


bool MainWindow::runQtSoFile(const char *qtSOFile)
{
    //LOGMASK(LOG_DEBUG | LOG_INFO  |LOG_WARNING | LOG_ERR);
    bool ret = false;
    LOGD(logtag_mainapp, "runQtSoFile:%s\n", m_launchPacket.arg);

    //void *handle = dlopen(m_launchPacket.arg, RTLD_LAZY);
    void *handle = get_so_handle(m_launchPacket.arg);
    if (handle) {
        LOGI(logtag_mainapp, "handle alreay exist -> file name is %s\n", (char *)m_launchPacket.arg);
    }

    int retry = 0;
    while (!handle) {
        LOGI(logtag_mainapp, "dlopen so file %s\n", (char *)m_launchPacket.arg);
        handle = dlopen(m_launchPacket.arg, RTLD_LAZY);
        if (handle) {
            strcpy(g_so_file_items[g_so_file_count].so_file, m_launchPacket.arg);
            g_so_file_items[g_so_file_count].handle = handle;
            g_so_file_count ++;
        } else {
            LOGE(logtag_mainapp, "dlopen error:%s\n", dlerror());
            if (retry >= 1)
                return ret;
            ++retry;
            usleep(100000);
        }
    }

    LOGD(logtag_mainapp, "runQtSoFile:%s\n", m_launchPacket.arg);
    if (handle) {
        soapp_main main = 0;
        soapp_isOpenGLWindow isOpenGLWindow = 0;

        //printf("[MAIN APP]runQtSoFile lwpid:%d\r\n", syscall(SYS_gettid));
        //printf("[MAIN APP]runQtSoFile tid:%d\r\n", pthread_self());

        isOpenGLWindow = (soapp_isOpenGLWindow)dlsym(handle, "isOpenGLWindow");
        main = (soapp_main)dlsym(handle, "so_main");
        if (main) {
            // if (QString(m_launchPacket.arg).endsWith("home.so")) {
            //     void *avmHandle = get_so_handle(AVM_APP_PATH);
            //     soapp_isAvmShow isAvmShow = 0;
            //     bool isAvmPreviewShow = false;
            //     if (avmHandle && (isAvmShow = (soapp_isAvmShow)dlsym(avmHandle, "isAvmPreviewShow"))) {
            //        soapp_setAvmShow setAvmPreviewShow = (soapp_setAvmShow)dlsym(handle, "setAvmPreviewShow");
            //        if (setAvmPreviewShow) {
            //            LOGD(logtag_mainapp, "setAvmPreviewShow");
            //            setAvmPreviewShow(isAvmShow());
            //        }
            //     } else {
            //         LOGD(logtag_mainapp, "avmhandle is null or soapp_isAvmShow is null");
            //     }
            // }

            main(0, NULL, soapp_exit, handle, &m_launchPacket);
            ret = true;
            LOGD(logtag_mainapp, "run QT so File Succes:%s\r\n", qtSOFile);
        } else {
            LOGD(logtag_mainapp, "no so_main in %s\r\n", qtSOFile);
        }

        if (isOpenGLWindow) {
        }
    } else {
        LOGD(logtag_mainapp, "runQtSoFile -> dlopen :%s failed, error is %s\r\n", m_launchPacket.arg, dlerror());
    }

    return ret;
}


bool MainWindow::startListen ()
{
    LOGD(logtag_mainapp, "[MainWindow::startListen]:%s\r\n", MAIN_APPLICATION_SOCKET_ADDR);
    bind(MAIN_APPLICATION_SOCKET_ADDR);
    startService();

    return true;
}

void MainWindow::customEvent(QEvent *e)
{
    LOGI(logtag_mainapp, "customEvent enter");
    if  (RUN_QT_SO_FILE == e->type()) {
        LOGD(logtag_mainapp, "[MainWindow::customEvent]:RUN_QT_SO_FILE->%s\r\n", m_launchPacket.arg);
        runQtSoFile(NULL);
    } else if (EXIT_QT_SO_FILE == e->type()) {
        m_lockPacket.lock();
        void *handle = m_soHandle;
        m_soHandle = 0;
        m_lockPacket.unlock();
        LOGD(logtag_mainapp, "[MainWindow::customEvent]:EXIT_QT_SO_FILE->%d\r\n", handle);
        if (handle) {
            soapp_close  so_close = NULL;
            so_close = (soapp_close)dlsym(handle, "so_close");
            LOGD(logtag_mainapp, "[MainWindow::customEvent]:EXIT_QT_SO_FILE -> so_close = %p\n", so_close);
            if (so_close) {
                so_close();
            }
            //dlclose(handle);
        }
    }
}

int MainWindow::onReceive()
{
    LOGI(logtag_mainapp, "MainWindow::onReceive enter");
    m_lockPacket.lock();
    int recvLen = -1;
    std::string addr;
    memset(&m_launchPacket, 0, sizeof(m_launchPacket));
    recvLen = read(&m_launchPacket, sizeof(m_launchPacket), addr);
    LOGI(logtag_mainapp, "[MainWindow::onReceive] size(%d) cmd(%d) arg(%s) from:%s\r\n",m_launchPacket.size, m_launchPacket.cmd, m_launchPacket.arg, addr.c_str());
    LOGI(logtag_mainapp, "from:%s\r\n", addr.c_str());
    QApplication::postEvent(this, new QEvent(RUN_QT_SO_FILE));
    m_lockPacket.unlock();

    return recvLen;
}
