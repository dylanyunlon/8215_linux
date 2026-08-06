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
 
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QObject>
#include <QQuickItem>
#include <QLabel>
#include <QQuickWindow>
#include <QQmlContext>
#include <QTranslator>
#include <5.6.3/QtGui/qpa/qplatformnativeinterface.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "applog.h"

#include "bluetoothapplication.h"
#include "bluetoothcallpage.h"
#include "bluetoothdialpage.h"
#include "bluetoothcallrecordsbookspage.h"
#include "bluetoothmusicpage.h"
#include "bluetoothpairrecordspage.h"
#include "bluetoothsettingpage.h"
#include "bluetoothpaireddevicemodel.h"
#include "bluetoothavailabledevicemodel.h"
#include "bluetoothcallrecordsmodel.h"
#include "bluetoothphonebookmodel.h"
#include "bluetoothcallmodel.h"

#include "bluetoothgapcallback.h"
#include "bluetoothhfpcallback.h"
#include "bluetoothpbapcallback.h"
#include "bluetoothavrcpcallback.h"
#include "bluetoothphoneapplication.h"
#include "bluetoothhidcallback.h"

using namespace std;
static const char* const tag = "BT_App";
#define MIN_BUF_LEN 0
#define MAX_BUF_LEN 255
#define RETRY_TIME  5
#define ABSOLUTE_PATH  "/data/bluetoothconfiguration.cfg"
#define PROC_SELF_PATH "/proc/self/exe"
#define RELATIVE_PATH  "bluetoothconfiguration.cfg"

//memory data
BluetoothMemData g_bluetoothMemData;
char g_filePath[MAX_BUF_LEN];

int saveBluetoothConfiguration()
{
    LOGD(tag, "saveBluetoothConfiguration\n");

    FILE *pFile = fopen(g_filePath, "wb");
    int writeLen = 0;
    int ret = -1;
    int i = 0;

    if (NULL != pFile) {
        while ((i < RETRY_TIME) && (1 != writeLen)) {
            writeLen = fwrite(&g_bluetoothMemData, sizeof(g_bluetoothMemData), 1, pFile);
            LOGI(tag, "g_filePath is %s, writeLen is %d\n", g_filePath, writeLen);

            ret = ferror(pFile);
            if (0 != ret) {
                LOGE(tag, "fwrite error, ret is %d\n", ret);
                clearerr(pFile);
            } else {
                break;
            }
            ++i;
        }

        fclose(pFile);

        LOGI(tag, "memAutoAnswer is %d\n", g_bluetoothMemData.memAutoAnswer);
        LOGI(tag, "memAutoConnect is %d\n", g_bluetoothMemData.memAutoConnect);
        LOGI(tag, "memBluetoothAddress is %lld\n", g_bluetoothMemData.memBluetoothAddress);
    } else {
        LOGE(tag, "open bluetoothconfiguration.cfg fail!\n");
    }

    return writeLen;
}

int loadBluetoothParameterFromCfgFile()
{
    memset(&g_filePath, 0, sizeof(g_filePath));
    strcpy(g_filePath, ABSOLUTE_PATH);

    FILE *pFile = fopen(g_filePath, "rb");
    if (NULL != pFile) {
        int readLen = -1;
        int ret = -1;
        int i = 0;
        int size = sizeof(g_bluetoothMemData);
        while((i < RETRY_TIME) && (size != readLen)) {
            readLen = fread(&g_bluetoothMemData, 1, size, pFile);
            if (size == readLen) {
                ret = ferror(pFile);
                if (0 != ret) {
                    LOGE(tag, "fread error, ret is %d\n", ret);
                    clearerr(pFile);
                } else {
                    break;
                }
            } else if (size != readLen) {
                LOGE(tag, "fread fail!\n");
                ret = ferror(pFile);
                if (0 != ret) {
                    LOGE(tag, "fread error, ret is %d\n", ret);
                    clearerr(pFile);
                }
            }
            i++;
        }

        if (size != readLen) {
            LOGE(tag, "read bluetoothconfiguration.cfg fail!\n");
            //use defalut
            memset(&g_bluetoothMemData, 0, sizeof(g_bluetoothMemData));
            g_bluetoothMemData.memAutoAnswer = false;
            g_bluetoothMemData.memAutoConnect = false;
            g_bluetoothMemData.memBluetoothAddress = 0;
        }
        fclose(pFile);
    } else {
        LOGE(tag, "open bluetoothconfiguration.cfg fail!\n");
        memset(&g_bluetoothMemData, 0, sizeof(g_bluetoothMemData));
        g_bluetoothMemData.memAutoAnswer = false;
        g_bluetoothMemData.memAutoConnect = false;
        g_bluetoothMemData.memBluetoothAddress = 0;
    }

	return 0;
}

#ifndef WITH_SOAPP
int main(int argc, char *argv[])
{
#ifdef USE_SYSTEM_LOG
    OPENLOG("BT_App");
#endif
    universal_utils::CLog::setLogLevel(UNIVERSAL_UTILE_LOG_LEVEL_DEBUG);

    QApplication app(argc, argv);
	QTranslator  translator;

    CBluetoothPairedDeviceModel m_bluetoothPairedDeviceModel;
    CBluetoothAvailableDeviceModel m_bluetoothAvailableDeviceModel;
    CBluetoothPhoneBookModel   m_bluetoothPhoneBookModel;
    CBluetoothCallRecordsModel m_bluetoothCallRecordsModel;
    CBluetoothCallListModel m_bluetoothCallListModel;
    //new page
    CBluetoothCallPage m_bluetoothCallPage;
    CBluetoothDialPage m_bluetoothDialPage;
    CBluetoothCallRecordsBooksPage m_bluetoothCallRecordsBooksPage;
    CBluetoothMusicPage m_bluetoothMusicPage;
    CBluetoothPairRecordsPage m_bluetoothPairRecordsPage;
    CBluetoothSettingPage m_bluetoothSettingPage;

    CBluetoothApplication m_bluetoothApplication(
        &m_bluetoothCallPage, &m_bluetoothDialPage, &m_bluetoothCallRecordsBooksPage,
        &m_bluetoothMusicPage, &m_bluetoothPairRecordsPage, &m_bluetoothSettingPage);

    QQmlApplicationEngine bluetooth_engine;
    QQmlContext *ctx = bluetooth_engine.rootContext();
    if (NULL != ctx) {
        ctx->setContextProperty("bluetoothApplication",         &m_bluetoothApplication);
        ctx->setContextProperty("bluetoothCallPage",            &m_bluetoothCallPage);
        ctx->setContextProperty("bluetoothDialPage",            &m_bluetoothDialPage);
        ctx->setContextProperty("bluetoothCallRecordsBooksPage",&m_bluetoothCallRecordsBooksPage);
        ctx->setContextProperty("bluetoothMusicPage",           &m_bluetoothMusicPage);
        ctx->setContextProperty("bluetoothPairRecordsPage",     &m_bluetoothPairRecordsPage);
        ctx->setContextProperty("bluetoothSettingPage",         &m_bluetoothSettingPage);
        ctx->setContextProperty("bluetoothPairedDeviceModel",   &m_bluetoothPairedDeviceModel);
        ctx->setContextProperty("bluetoothAvailableDeviceModel",&m_bluetoothAvailableDeviceModel);
        ctx->setContextProperty("bluetoothPhoneBookModel",      &m_bluetoothPhoneBookModel);
        ctx->setContextProperty("bluetoothCallRecordsModel",    &m_bluetoothCallRecordsModel);
        ctx->setContextProperty("bluetoothCallListModel",       &m_bluetoothCallListModel);
    }

    loadBluetoothParameterFromCfgFile();
    m_bluetoothCallPage.doAutoListenStateChanged(g_bluetoothMemData.memAutoAnswer);
    m_bluetoothSettingPage.doBluetoothAutoAnswer(g_bluetoothMemData.memAutoAnswer);
    m_bluetoothSettingPage.doBluetoothAutoConnect(g_bluetoothMemData.memAutoConnect);
    m_bluetoothPairRecordsPage.doAutoConnectStateChanged(g_bluetoothMemData.memAutoConnect);
    m_bluetoothPairRecordsPage.doAutoConnectAddress(g_bluetoothMemData.memBluetoothAddress);

    bluetooth_engine.load(QUrl(QStringLiteral("qrc:/bluetooth.qml")));
    QObject *toplevel = bluetooth_engine.rootObjects().value(0);
    QQuickWindow *appWindow = qobject_cast<QQuickWindow *>(toplevel);
    if (NULL != appWindow) {
        appWindow->setFlags(Qt::Window | Qt::FramelessWindowHint);
    } else {
        LOGE(tag, "appWindow is null!\n");
    }

	m_bluetoothApplication.initTranslator(&app, &translator);
    if (NULL != appWindow) {
        m_bluetoothApplication.initListener(appWindow, NULL, NULL, NULL);
    }

    bluetooth_engine.load(QUrl(QStringLiteral("qrc:/qml/bluetoothcallnavigation.qml")));
    LOGD(tag, "load bluetoothcallnavigation.qml -> after, root object size = %d\n", bluetooth_engine.rootObjects().size());
    QObject *navigaitonlevel = bluetooth_engine.rootObjects().value(1);
    QQuickWindow *btCallWindow = qobject_cast<QQuickWindow *>(navigaitonlevel);
    if (btCallWindow) {
        QPlatformNativeInterface *ni = app.platformNativeInterface();
        QPlatformWindow *nativeHandle = btCallWindow->handle();
        LOGD(tag, "nativeHandle is %p\n", nativeHandle);
        if (!nativeHandle) {
            btCallWindow->create();
            nativeHandle = btCallWindow->handle();
        }
        if (ni && nativeHandle) {
            ni->setWindowProperty(nativeHandle, "type", "panel");
        }
    }

    CBluetoothPhoneApplication bluetoothPhoneApplication(btCallWindow);
    bluetoothPhoneApplication.threadStart();

    m_bluetoothApplication.initApplication();

    return app.exec();
}

#else
QQmlApplicationEngine bluetoothqml_engine;
typedef void (*soapp_exit_handler)(void *handle, void *arg);

extern "C" bool isOpenGLWindow(void) {
	return (true);
}

//new model
CBluetoothPairedDeviceModel g_bluetoothPairedDeviceModel;
CBluetoothAvailableDeviceModel g_bluetoothAvailableDeviceModel;
CBluetoothPhoneBookModel   g_bluetoothPhoneBookModel;
CBluetoothCallRecordsModel g_bluetoothCallRecordsModel;
CBluetoothCallListModel g_bluetoothCallListModel;
//new page
CBluetoothCallPage g_bluetoothCallPage;
CBluetoothDialPage g_bluetoothDialPage;
CBluetoothCallRecordsBooksPage g_bluetoothCallRecordsBooksPage;
CBluetoothMusicPage g_bluetoothMusicPage;
CBluetoothPairRecordsPage g_bluetoothPairRecordsPage;
CBluetoothSettingPage g_bluetoothSettingPage;
QTranslator  g_translator;

CBluetoothApplication g_bluetoothApplication(
    &g_bluetoothCallPage, &g_bluetoothDialPage, &g_bluetoothCallRecordsBooksPage,
    &g_bluetoothMusicPage, &g_bluetoothPairRecordsPage, &g_bluetoothSettingPage);


int loadBluetoothCfg()
{
    LOGD(tag, "loadBluetoothCfg\n");

    loadBluetoothParameterFromCfgFile();

    g_bluetoothCallPage.doAutoListenStateChanged(g_bluetoothMemData.memAutoAnswer);
    g_bluetoothSettingPage.doBluetoothAutoAnswer(g_bluetoothMemData.memAutoAnswer);
    g_bluetoothSettingPage.doBluetoothAutoConnect(g_bluetoothMemData.memAutoConnect);
    g_bluetoothPairRecordsPage.doAutoConnectStateChanged(g_bluetoothMemData.memAutoConnect);
    g_bluetoothPairRecordsPage.doAutoConnectAddress(g_bluetoothMemData.memBluetoothAddress);

    return 0;
}

extern "C" int loadBluetoothConfiguration()
{
    LOGD(tag, "loadBluetoothConfiguration\n");

    loadBluetoothCfg();

    return 0;
}

extern "C" int so_main(int argc, char *argv[], soapp_exit_handler exit_handler, void *handle, void *param)
{
    LOGD(tag, "so_main, argc is %d, argv is %p\n", argc, argv);

    QQmlContext *ctx = bluetoothqml_engine.rootContext();
    ctx->setContextProperty("bluetoothApplication",         &g_bluetoothApplication);
    ctx->setContextProperty("bluetoothCallPage",            &g_bluetoothCallPage);
    ctx->setContextProperty("bluetoothDialPage",            &g_bluetoothDialPage);
    ctx->setContextProperty("bluetoothCallRecordsBooksPage",&g_bluetoothCallRecordsBooksPage);
    ctx->setContextProperty("bluetoothMusicPage",           &g_bluetoothMusicPage);
    ctx->setContextProperty("bluetoothPairRecordsPage",     &g_bluetoothPairRecordsPage);
    ctx->setContextProperty("bluetoothSettingPage",         &g_bluetoothSettingPage);
    ctx->setContextProperty("bluetoothPairedDeviceModel",   &g_bluetoothPairedDeviceModel);
    ctx->setContextProperty("bluetoothAvailableDeviceModel",&g_bluetoothAvailableDeviceModel);
    ctx->setContextProperty("bluetoothPhoneBookModel",      &g_bluetoothPhoneBookModel);
    ctx->setContextProperty("bluetoothCallRecordsModel",    &g_bluetoothCallRecordsModel);
    ctx->setContextProperty("bluetoothCallListModel",       &g_bluetoothCallListModel);

    loadBluetoothCfg();

    bluetoothqml_engine.load(QUrl(QStringLiteral("qrc:/bluetooth.qml")));
    QObject *toplevel = bluetoothqml_engine.rootObjects().value(0);
    QQuickWindow *appWindow = qobject_cast<QQuickWindow *>(toplevel);
    if (!appWindow) {
        LOGE(tag, "appWindow is null!\n");
    }
    //appWindow->show();
    if (QApplication::instance() != NULL) {
        g_bluetoothApplication.initTranslator((QApplication *)QApplication::instance(), &g_translator);
    } else {
        LOGE(tag, "QApplication is null!\n");
    }

    g_bluetoothApplication.initListener(appWindow, exit_handler, handle, param);
    g_bluetoothApplication.initApplication();

    return(0);
}
#endif


