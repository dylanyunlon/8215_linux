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

#ifndef _QOBJLISTENER_H
#define _QOBJLISTENER_H

#include "apptype.h"
#include "ctllistener.h"
#include "globalbus.h"
#include "appobj.h"
#include <QWindow>
#include <QObject>
#include <QVariant>

typedef void (*soapp_exit_handler)(void *handle, void *arg);
#define MAX_LISTENER_NAME_LEN 128

class CQObjListener : public QObject, public CCtlListener
{
    Q_OBJECT
public:
    CQObjListener(unsigned char appID);
    virtual ~CQObjListener();

    bool initListener(QWindow *topWindow,
                        soapp_exit_handler exit_handler,
                        void *handle, void *param);
    bool initListener(QWindow *topWindow,
                                soapp_exit_handler exit_handler,
                                void *handle, void *param,
                                bool isFloatingWindow);

    int onExit(int param1, int param2);
    int onShowFrontUI(void);
    int onHideFrontUI(void);
    int onShowFront(int param1, int param2);
    int onHideFront(int param1, int param2);
    int onShowRear(int param1, int param2);

    int onHideRear(int param1, int param2);
    int onAudioFocusChanged(CCtlListener::E_AVOUT aOut,
                                CCtlListener::E_AUDIOFOCUS focus);
    int onVideoFocusChanged(CCtlListener::E_AVOUT vOut,
                                CCtlListener::E_VIDEOFOCUS focus);
    bool onKeyEvent(int key, int param1, int param2);
    int getWindowType() const;

    bool mNeedHideFrontWindow;

signals:
    int sigExit(int param1, int param2);
    int sigShowFrontUI(void);
    int sigHideFrontUI(void);
    int sigShowFront(int param1, int param2);
    int sigHideFront(int param1, int param2);
    int sigShowRear(int param1, int param2);
    int sigHideRear(int param1, int param2);
    int sigAudioFocusChanged(CCtlListener::E_AVOUT aOut,
                            CCtlListener::E_AUDIOFOCUS focus);
    int sigVideoFocusChanged(CCtlListener::E_AVOUT vOut,
                            CCtlListener::E_VIDEOFOCUS focus);
    bool sigKeyEvent(int key, int param1, int param2);

private slots:


public slots:
	int doShowFrontUI(void);
    int doHideFrontUI(void);
    virtual int doExit(int param1, int param2);
    virtual int doShowFront(int param1, int param2);
    virtual int doHideFront(int param1, int param2);
    virtual int doShowRear(int param1, int param2);
    virtual int doHideRear(int param1, int param2);
    virtual int doAudioFocusChanged(CCtlListener::E_AVOUT aOut,
                                    CCtlListener::E_AUDIOFOCUS focus);
    virtual int doVideoFocusChanged(CCtlListener::E_AVOUT vOut,
                                    CCtlListener::E_VIDEOFOCUS focus);
    virtual bool doKeyEvent(int key, int param1, int param2);

protected:
    QWindow *m_appWindow;

private:
    unsigned char m_appID;
    char m_listenerName[MAX_LISTENER_NAME_LEN];
    soapp_exit_handler m_appExitProc;
    void *m_appHandle;
    CAPPTargetObj *m_appObj;
    int m_windowType;
    bool m_floatingWindow;
};

#endif
