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
 
#ifndef CFILESYSWATCHER_H
#define CFILESYSWATCHER_H

#include <QObject>
#include <QStringList>
#include <QDebug>

//#include "cmddef.h"
#include "applog.h"

class AtcDeviceManager;

class CFileSysWatcher : public QObject
{
    Q_OBJECT
public:
    CFileSysWatcher();
    ~CFileSysWatcher();

private:
    bool initFileSystemWatcher(void);


signals:
    void sendDevChangedMsg(unsigned int msg, unsigned int wParam, unsigned int lParam);


public slots:
    void devChanged(const QString& mountpoint, const QString& action);


private:
    unsigned int m_msg;
    unsigned int m_wParam;
    unsigned int m_lParam;
	AtcDeviceManager *m_devManager;
};

#endif // CFILESYSWATCHER_H
