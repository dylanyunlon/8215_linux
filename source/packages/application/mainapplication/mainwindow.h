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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <QEvent>
#include <QApplication>
#include "csync.h"
#include "singleton.h"
#include "cautoudpsocket.h"
#include "apptype.h"


class MainWindow : public QObject, private CAutoUDPSocket, public universal_utils::Singleton<MainWindow>
{
    Q_OBJECT

public:
    explicit MainWindow(QObject *parent = 0);
    ~MainWindow();

    bool startListen();
    bool exitQtSoFile(void *handle, void *param);

private:
    int onReceive (); //running on thread;

    void customEvent(QEvent *e);
    bool runQtSoFile(const char *qtSOFile);
    LaunchPacket m_launchPacket;
    void        *m_soHandle;

    //QApplication &m_app;

    universal_utils::CMutexObject m_lockPacket;
};

#endif // MAINWINDOW_H
