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
#include <qqmlcontext.h>
#include <unistd.h>
#include "apptype.h"
#include "appobj.h"
#include "ctllistener.h"
#include "applog.h"
#include "qobjlistener.h"

#define TAG "BTPhone"

extern "C" bool isOpenGLWindow(void) {
    return (true);
}

QQmlApplicationEngine g_qml_engine_btphone;
typedef void (*soapp_exit_handler)(void *handle, void *arg);

CQObjListener g_qobjListener(CAPPBaseObj::APPID_BTPHONE);

extern "C" int so_main(int argc, char *argv[], soapp_exit_handler exit_handler, void *handle, void *param)
{
    LOGI(TAG, "enter so_main\n");
    int ret = 0;

    QQmlContext *ctx = g_qml_engine_btphone.rootContext();
    if (ctx == NULL) {
        LOGE(TAG, "ctx is NULL\n");
        ret = -1;
    }

    if (ret == 0) {
        g_qml_engine_btphone.load(QUrl(QStringLiteral("qrc:/new/prefix1/resource/btphone.qml")));

        QObject *toplevel = g_qml_engine_btphone.rootObjects().value(0);
        if (toplevel != NULL) {
            QQuickWindow *appWindow = qobject_cast<QQuickWindow *>(toplevel);
            if (appWindow != NULL) {
                g_qobjListener.initListener(appWindow, exit_handler, handle, param, true);
            }
        } else {
            LOGE(TAG, "ctx is NULL\n");
            ret = -1;
        }
    }
    return ret;
}