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
#include <QQuickWindow>
#include <dlfcn.h>
#include <string>
#include <QDateTime>
#include <QTime>
#include <QTextCodec>
#include <QFontDatabase>
#include <appimageprovider.h>

#include <csync.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <dlfcn.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <QFileInfo>

#if QT_VERSION == QT_VERSION_CHECK(5, 6, 3)
#include <5.6.3/QtGui/qpa/qplatformnativeinterface.h>
#else
#include <5.10.1/QtGui/qpa/qplatformnativeinterface.h>
#endif

#include "csubwindowhome.h"
#include "cglobaldata.h"
//#include "bootproflog.h"


#define DEFAULT_FONT_POINT_SIZE     6
#define DEFAULT_COLOR_BUFFER_SIZE   8

typedef int (*systemConfig)();

#ifndef WITH_SOAPP
int main(int argc, char *argv[])
{
#ifdef USE_SYSTEM_LOG
    OPENLOG("Homw_App");
#endif
    universal_utils::CLog::setLogLevel(UNIVERSAL_UTILE_LOG_LEVEL_DEBUG);
    //BootprofLog::writeLog("home main start");

    QApplication app(argc, argv);
    QQmlApplicationEngine engine;

    LOGI(TAG_HOME, "[homeapp] wayland -> app prepare End\r\n");
/*
    QFont font  = app.font();
    LOGI(TAG_HOME, "[homeapp] so_main -> font.pointSize() = %d\r\n", font.pointSize());
    font.setPointSize(DEFAULT_FONT_POINT_SIZE);
    app.setFont(font);

    QFontDatabase  font_db;
    foreach (const QString &family, font_db.families()) {
        LOGI(TAG_HOME, "[homeapp] so_main -> font family : %s\r\n", (family.toLatin1()).data());
    }
*/
    QFont font2("Droid Sans Fallback", DEFAULT_FONT_POINT_SIZE);
    font2.setPointSize(DEFAULT_FONT_POINT_SIZE);
    app.setFont(font2);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF8"));

#if 1
    QSurfaceFormat fmt;

    fmt.setAlphaBufferSize(DEFAULT_COLOR_BUFFER_SIZE);
    fmt.setRedBufferSize(DEFAULT_COLOR_BUFFER_SIZE);
    fmt.setGreenBufferSize(DEFAULT_COLOR_BUFFER_SIZE);
    fmt.setBlueBufferSize(DEFAULT_COLOR_BUFFER_SIZE);
    QSurfaceFormat::setDefaultFormat(fmt);
#endif

    QQmlContext *context = engine.rootContext();
    if (context == NULL) {
        LOGI(TAG_HOME, "QQmlContext is NULL \r\n");
        return false;
    }

    QString appDirPath;
    if (argc >= 1) {
        LOGI(TAG_HOME, "[homeapp] wayland -> argv[0]  = %s\r\n", argv[0]);
        const QString app_name = QString::fromUtf8(argv[0]);
        QFileInfo file_info(app_name);
        appDirPath = file_info.path();
        LOGI(TAG_HOME, "[homeapp] wayland -> app file path = %s \r\n" ,(file_info.path().toLatin1()).data());
    } else {
        appDirPath = QCoreApplication::applicationDirPath();
        LOGI(TAG_HOME, "[homeapp] wayland -> home app argc = 0 \r\n");
    }

    CSubWindowHome *home = new CSubWindowHome;
    if (home == NULL || appDirPath.length() == 0) {
        LOGE(TAG_HOME, "wayland ->CSubWindowHome is NULL \r\n");
        return false;
    }
    home->initContext(context, appDirPath);

    //BootprofLog::writeLog("home qml load start");

    engine.addImageProvider(QLatin1String("AppImageProvider"), app_create_image_provider());
    engine.load(QUrl(QStringLiteral("qrc:/Res/home.qml")));
    LOGI(TAG_HOME, "[homeapp] main -> qml_engine load end\r\n");
    //BootprofLog::writeLog("home qml load finish");

    QQuickWindow *appWindow = qobject_cast<QQuickWindow *>(engine.rootObjects().value(0));
    if (appWindow == NULL) {
        LOGI(TAG_HOME, "wayland ->appWindow is NULL \r\n");
        return false;
    }
    appWindow->create();
    QPlatformNativeInterface *ni = app.platformNativeInterface();
    QPlatformWindow *nativeHandle = appWindow->handle();
    if (ni && nativeHandle) {
        LOGI(TAG_HOME, "[HomeApp] set home ui window type to home_ui -> before\n");
        ni->setWindowProperty(nativeHandle, "type", "home_ui");
        LOGI(TAG_HOME, "[HomeApp] set home ui window type to home_ui -> after\n");
    }

    appWindow->setFlags(Qt::Window | Qt::FramelessWindowHint);
    appWindow->show();

    LOGI(TAG_HOME, "[VolumeDemo] load volume_overlay.qml -> after, root object size = %d\n", engine.rootObjects().size());

    /*
    QQuickWindow *volume_overlay = qobject_cast<QQuickWindow *>(engine.rootObjects().value(1));
    if (volume_overlay) {
        QPlatformNativeInterface *ni = app.platformNativeInterface();
        QPlatformWindow *nativeHandle = volume_overlay->handle();
        if (ni && nativeHandle) {
            LOGI(TAG_HOME, "[VolumeDemo] set volume demo window type to volume overlay -> before\n");
            ni->setWindowProperty(nativeHandle, "type", "volume_overlay");
            LOGI(TAG_HOME, "[VolumeDemo] set volume demo window type to volume overlay -> after\n");
        }
    } else {
        LOGE(TAG_HOME, "could not found volume overlay");
    }
    */

    home->initObjects(appWindow);
    home->initListener(appWindow, NULL, NULL, NULL);
    //home->initVolumeWindow(volume_overlay);

    //BootprofLog::writeLog("home main finished");
    return app.exec();
}

#else

QQmlApplicationEngine qml_engine;

extern "C" bool isOpenGLWindow(void) {
    return (true);
}


CSubWindowHome g_WindowHome;
static bool g_avmPreviewShow = false;
extern "C" void setAvmPreviewShow(bool flag)
{
    g_avmPreviewShow = flag;
    LOGI(TAG_HOME, "[homeapp] setAvmPreviewShow: %d", flag);
}

extern "C" int so_main(int argc, char *argv[], soapp_exit_handler exit_handler, void *handle, void *param)
{
    LOGI(TAG_HOME, "[homeapp] enter so_main\r\n");
    //BootprofLog::writeLog("home so_main start");

    LOGI(TAG_HOME, "[homeapp] so_main -> init context start\r\n");
    QQmlContext *context = qml_engine.rootContext();
    if (context == NULL) {
        LOGI(TAG_HOME, "QQmlContext is NULL \r\n");
        return false;
    }

    QString appDirPath;
    if (argc >= 1) {
        LOGI(TAG_HOME, "[homeapp] so_main -> argv[0]  = %s\n", argv[0]);
        const QString app_name = QString::fromUtf8(argv[0]);
        QFileInfo file_info(app_name);
        appDirPath = file_info.path();
        LOGI(TAG_HOME, "[homeapp] so_main -> app file path = %s \r\n" ,(file_info.path().toLatin1()).data());
    } else {
        appDirPath = QCoreApplication::applicationDirPath();
        LOGI(TAG_HOME, "[homeapp] so_main -> home app argc = 0 \r\n");
    }

    if (appDirPath.length() == 0) {
        LOGI(TAG_HOME, "appDirPath.length() == 0\r\n");
        return false;
    }

    g_WindowHome.initContext(context, appDirPath);

    QString settingQtName = "/libsettings_app.so";
    settingQtName.prepend(appDirPath);
    std::string fileName = settingQtName.toStdString();

    //BootprofLog::writeLog("home load qml start");

    LOGI(TAG_HOME, "[homeapp] so_main -> qml_engine load\r\n");
    qml_engine.addImageProvider(QLatin1String("AppImageProvider"), app_create_image_provider());
    qml_engine.load(QUrl(QStringLiteral("qrc:/Res/home.qml")));
    LOGI(TAG_HOME, "[homeapp] so_main -> qml_engine load\r\n");

    //BootprofLog::writeLog("home load qml finished");

    QQuickWindow *appWindow = qobject_cast<QQuickWindow *>(qml_engine.rootObjects().value(0));
    if (appWindow == NULL) {
        LOGI(TAG_HOME, "appWindow is NULL \r\n");
        return false;
    }

    if (!g_avmPreviewShow) {
        appWindow->show();
        //BootprofLog::writeLog("home show finished");
    }
    appWindow->setWindowState(Qt::WindowFullScreen);
    appWindow->setFlags(Qt::Window | Qt::FramelessWindowHint);

    g_WindowHome.initObjects(appWindow);

    LOGI(TAG_HOME, "[homeapp] end initObjects\r\n");

    g_WindowHome.initListener(appWindow, exit_handler, handle, param);
    /*
    void *settinghandle = dlopen(fileName.c_str(), RTLD_LAZY);
    if (settinghandle) {
        systemConfig sysConfig = NULL;
        sysConfig = (systemConfig)dlsym(settinghandle, "loadSettingConfig");
        if (sysConfig) {
            sysConfig();
            LOGI(TAG_HOME, "[homeapp] so_main -> resume last system config success\r\n");
        } else {
            LOGI(TAG_HOME, "[homeapp] so_main -> no loadSettingConfig in %s\r\n", fileName.c_str());
        }
        dlclose(settinghandle);
    }
    else {
        LOGI(TAG_HOME, "[homeapp] so_main -> dlopen :%s failed\r\n", fileName.c_str());
    }*/
    LOGI(TAG_HOME, "[homeapp] end so_main\r\n");
    //BootprofLog::writeLog("home so_main finished");

    return (0);
}
#endif

