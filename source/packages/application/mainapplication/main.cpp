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
#include <QApplication>
#include <sys/syscall.h>
#include <assert.h>
#include <pthread.h>

#include <QDebug>
#include <QWindow>
#include <QTextCodec>
#include <QFontDatabase>

#include "globalbus.h"
//#include "client/linux/handler/exception_handler.h"
#include "applog.h"

const char* logtag_mainapp = "main_application_app";

extern "C" void load_mmpctrl_library(void);
extern "C" void dummy_load_connman_library(void);

#ifndef WITH_SOAPP
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    universal_utils::CLog::setLogLevel(UNIVERSAL_UTILE_LOG_LEVEL_DEBUG);

    int ret = 0;

    OPENLOG("main_application");

    //LOGMASK(LOG_DEBUG | LOG_INFO  |LOG_WARNING | LOG_ERR);
    LOGD(logtag_mainapp, "******** [MainWindow]main ********\n");
    /*google_breakpad::MinidumpDescriptor  descriptor("/tmp");
    google_breakpad::ExceptionHandler eh(descriptor,
                                            NULL,
                                            NULL,
                                            NULL,
                                            true,
                                            -1);*/

    /*
    QFont font  = a .font();
    font.setPointSize(6);
    a.setFont(font);

    QFontDatabase  font_db;
    std::string fontName;
    foreach (const QString &family, font_db.families()) {
        fontName = family.toStdString();
        LOGD(logtag_mainapp, "font family ++++++++++++++++: (%s)\n", fontName.c_str());
    }
    */

    QFont font2("Droid Sans Fallback", 6);
    font2.setPointSize(6);
    a.setFont(font2);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF8"));
    QSurfaceFormat fmt;
    fmt.setAlphaBufferSize(8);
    fmt.setRedBufferSize(8);
    fmt.setGreenBufferSize(8);
    fmt.setBlueBufferSize(8);
    QSurfaceFormat::setDefaultFormat(fmt);
//    load_mmpctrl_library();
#ifndef WITH_ATC_BUILD
//    dummy_load_connman_library();
#endif

    MainWindow w;
    w.startListen();
    GlobalBus::applyFor(GlobalBus::ACTION_MAINAPP_DONE, 0, 0);
    ret = a.exec();
    LOGD(logtag_mainapp, "******** [MainWindow]main leav, ret(0x%x)********\n", ret);
    return ret;
}
#else

#include <QDebug>
#include <QFileInfo>

typedef void (*soapp_exit_handler)(void *handle, void *arg);

extern "C" bool isOpenGLWindow(void) {
    return (true);
}


MainWindow g_mainApp;
extern "C" int so_main(int argc, char *argv[], soapp_exit_handler exit_handler, void *handle, void *param)
{
    LOGD(logtag_mainapp, "******** [MainWindow]so_main enter, argc(%d), exit_handler(%d), handle(%d)********\n", argc, (long)exit_handler, (long)handle);

    if ((argc > 0) && (argv != NULL)) {
        LOGD(logtag_mainapp, "******** [MainWindow]so_main, argv[0](%s)********\n", argv[0]);
    }

    if (param != NULL) {
    }
    load_mmpctrl_library();
    /*google_breakpad::MinidumpDescriptor  descriptor("/tmp");
    google_breakpad::ExceptionHandler eh(descriptor,
                                            NULL,
                                            NULL,
                                            NULL,
                                            true,
                                            -1);*/
    g_mainApp.startListen();
    LOGD(logtag_mainapp, "******** [MAIN APP]so_main leave ********\n");
    return (0);
}
#endif
