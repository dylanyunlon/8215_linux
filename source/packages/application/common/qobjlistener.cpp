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

#include <stdio.h>
#include <QtWidgets/QApplication>
#include <QMouseEvent>
#include <QDesktopWidget>
#include "qobjlistener.h"
#include "applog.h"


extern "C" CAPPTargetObj *createAPPTargetObj(unsigned char m_appID);
extern "C" bool releaseAppObj(CAPPBaseObj *obj);

static const char TAG[] = "CQObjListener";

CQObjListener::CQObjListener(unsigned char appID)
    : m_appID(appID)
    , m_appExitProc(0)
    , m_appHandle(0)
    , m_appWindow(0)
    , m_appObj(0)
    , m_windowType(0)
    , m_floatingWindow(false)
{
    mNeedHideFrontWindow = true;
}

CQObjListener::~CQObjListener()
{
    LOGD(TAG, "++%s %d: releaseAppObj%p\n", __func__, m_appID, m_appObj);
    if (m_appObj) {
        releaseAppObj(m_appObj);
        m_appObj = 0;
    }
    LOGD(TAG, "--%s %d: releaseAppObj%p\n", __func__, m_appID, m_appObj);
}

bool CQObjListener::initListener(QWindow *topWindow,
                                soapp_exit_handler exit_handler,
                                void *handle,
                                void *param)
{
    bool ret = false;

    m_appHandle = handle;
    m_appExitProc = exit_handler;
    m_appWindow = topWindow;

    if (m_appID != 39) {
        QRect rect = topWindow->frameGeometry();
        QRect scrRect = QApplication::desktop()->screenGeometry();
        if ((rect.height() < scrRect.height() && rect.width() < scrRect.width()) ||
            m_floatingWindow) {
            m_windowType = 1;
        }
        LOGD(TAG, "initListener not cluster");
    } else {
        LOGD(TAG, "initListener cluster++++");
    }


    QObject::connect(this, SIGNAL(sigExit(int, int)), this, SLOT(doExit(int, int)),
                    Qt::BlockingQueuedConnection);
    QObject::connect(this, SIGNAL(sigShowFrontUI()), this, SLOT(doShowFrontUI()),
                    Qt::BlockingQueuedConnection);
    QObject::connect(this, SIGNAL(sigHideFrontUI()), this, SLOT(doHideFrontUI()),
                    Qt::BlockingQueuedConnection);
    QObject::connect(this, SIGNAL(sigShowFront(int, int)), this, SLOT(doShowFront(int, int)),
                    Qt::BlockingQueuedConnection);
    QObject::connect(this, SIGNAL(sigHideFront(int, int)), this, SLOT(doHideFront(int, int)),
                    Qt::BlockingQueuedConnection);
    QObject::connect(this, SIGNAL(sigShowRear(int, int)), this, SLOT(doShowRear(int, int)),
                    Qt::BlockingQueuedConnection);
    QObject::connect(this, SIGNAL(sigHideRear(int, int)), this, SLOT(doHideRear(int, int)),
                    Qt::BlockingQueuedConnection);
    QObject::connect(this,
                    SIGNAL(sigAudioFocusChanged(CCtlListener::E_AVOUT,
                                                CCtlListener::E_AUDIOFOCUS)),
                    this,
                    SLOT(doAudioFocusChanged(CCtlListener::E_AVOUT,
                                                CCtlListener::E_AUDIOFOCUS)),
                    Qt::BlockingQueuedConnection);
    QObject::connect(this,
                    SIGNAL(sigVideoFocusChanged(CCtlListener::E_AVOUT,
                                                CCtlListener::E_VIDEOFOCUS)),
                    this,
                    SLOT(doVideoFocusChanged(CCtlListener::E_AVOUT,
                                                CCtlListener::E_VIDEOFOCUS)),
                    Qt::BlockingQueuedConnection);
    QObject::connect(this, SIGNAL(sigKeyEvent(int, int, int)),
                    this, SLOT(doKeyEvent(int, int, int)),
                    Qt::BlockingQueuedConnection);

    m_appObj = createAPPTargetObj(m_appID);
    if (m_appObj != NULL) {
        LOGD(TAG, "%s %d: setCtlListener, param: %p\n",
                    __func__, m_appID, param);
        ret = m_appObj->setCtlListener(this);
        if (!ret) {
            LOGE(TAG, "m_appObj->setCtlListener fail\n");
            releaseAppObj(m_appObj);
        }
    } else {
        LOGE(TAG, "createAPPTargetObj fail\n");
        ret = false;
    }

    return ret;
}

bool CQObjListener::initListener(QWindow *topWindow,
                                soapp_exit_handler exit_handler,
                                void *handle, void *param,
                                bool isFloatingWindow) {
    m_floatingWindow = isFloatingWindow;
    return initListener(topWindow, exit_handler, handle, param);
}

int CQObjListener::onExit(int param1, int param2)
{
    int ret = -1;
    LOGD(TAG, "++%s %d: param1(%d), param2(%d)\n",
                __func__, m_appID, param1, param2);
    ret = emit sigExit(param1, param2);
    LOGD(TAG, "--%s %d\n", __func__, m_appID);
    return ret;
}

int CQObjListener::onHideFrontUI(void)
{
    int ret = -1;
    ret = emit sigHideFrontUI();
    return ret;
}

int CQObjListener::onShowFrontUI(void)
{
    int ret = -1;
    ret = emit sigShowFrontUI();
    return ret;
}

int CQObjListener::onHideFront(int param1, int param2)
{
    int ret = -1;
    LOGD(TAG, "++%s %d: param1(%d), param2(%d)\n",
                __func__, m_appID, param1, param2);
    ret = emit sigHideFront(param1, param2);
    LOGD(TAG, "--%s %d\n", __func__, m_appID);
    return ret;
}

int CQObjListener::onShowFront(int param1, int param2)
{
    int ret = -1;
    LOGD(TAG, "++%s %d: param1(%d), param2(%d)\n",
                __func__, m_appID, param1, param2);
    ret = emit sigShowFront(param1, param2);
    LOGD(TAG, "--%s %d\n", __func__, m_appID);
    return ret;
}

int CQObjListener::onShowRear(int param1, int param2)
{
    int ret = -1;
    LOGD(TAG, "++%s %d: param1(%d), param2(%d)\n",
                __func__, m_appID, param1, param2);
    ret = emit sigShowRear(param1, param2);
    LOGD(TAG, "--%s %d\n", __func__, m_appID);
    return ret;
}

int CQObjListener::onHideRear(int param1, int param2)
{
    int ret = -1;
    LOGD(TAG, "++%s %d: param1(%d), param2(%d)\n",
                __func__, m_appID, param1, param1);
    ret = emit sigHideRear(param1, param2);
    LOGD(TAG, "--%s %d\n", __func__, m_appID);
    return ret;
}
int CQObjListener::onAudioFocusChanged(CCtlListener::E_AVOUT aOut,
                                        CCtlListener::E_AUDIOFOCUS focus)
{
    int ret = -1;
    LOGD(TAG, "++%s %d: aOut(%d), focus(%d)\n", __func__, m_appID, aOut, focus);
    ret = emit sigAudioFocusChanged(aOut, focus);
    LOGD(TAG, "--%s %d\n", __func__, m_appID);
    return ret;
}

int CQObjListener::onVideoFocusChanged(CCtlListener::E_AVOUT vOut,
                                        CCtlListener::E_VIDEOFOCUS focus)
{
    int ret = -1;
    LOGD(TAG, "++%s %d: vOut(%d), focus(%d)\n",
                __func__, m_appID, vOut, focus);
    ret = emit sigVideoFocusChanged(vOut, focus);
    LOGD(TAG, "--%s %d\n", __func__, m_appID);
    return ret;
}

bool CQObjListener::onKeyEvent(int key, int param1, int param2)
{
    bool ret = false;
    LOGD(TAG, "++%s %d: key(%x), param1(%d), param2(%d)\n",
                __func__, m_appID, key, param1, param2);
    ret = emit sigKeyEvent(key, param1, param2);
    LOGD(TAG, "--%s %d\n", __func__, m_appID);
    return ret;
}

int CQObjListener::getWindowType() const
{
    LOGD(TAG, "%s %d: window type: %d\n", __func__, m_appID, m_windowType);
    return m_windowType;
}

int CQObjListener::doExit(int param1, int param2)
{
    LOGD(TAG, "++%s %d: param1(%d), param2(%d), m_appExitProc(%p)\n",
                __func__, m_appID, param1, param2, m_appExitProc);
    if (m_appWindow) {
        LOGD(TAG, "%s: ++hide\n", __func__);
        m_appWindow->hide();
        LOGD(TAG, "%s: --hide\n", __func__);
#ifdef WITH_SOAPP
        m_appWindow->destroy();
        LOGD(TAG, "%s: --destroy\n", __func__);
#endif
    }

    if (m_appExitProc) {
        LOGD(TAG, "%s: ++%pf\n", __func__, m_appExitProc);
        m_appExitProc(m_appHandle, 0);
        m_appHandle = 0;
        m_appExitProc = 0;
        LOGD(TAG, "%s: --%pf\n", __func__, m_appExitProc);
    }

#ifndef WITH_SOAPP
    if (QApplication::instance() != NULL) {
        LOGD(TAG, "%s: quit()\n", __func__);
        QApplication::instance()->quit();
    }
#endif
    LOGD(TAG, "--%s %d\n", __func__, m_appID);
    return 0;
}

int CQObjListener::doHideFrontUI(void)
{
    LOGD(TAG, "++%s %d: m_appWindow(%p)\n",
                __func__, m_appID, m_appWindow);
    if (m_appWindow) {
        QPointF localPoint(QCursor::pos().x(), QCursor::pos().y());
        //QPointF screenPoint(0, 0);
        QMouseEvent event(QEvent::MouseButtonRelease,
                            localPoint,
                            Qt::NoButton,
                            Qt::NoButton,
                            Qt::NoModifier);
        QCoreApplication::instance()->sendEvent(m_appWindow, &event);
        LOGD(TAG, "%s: ++hide\n", __func__);

        if (mNeedHideFrontWindow) {
            m_appWindow->hide();
        }
#ifdef WITH_SOAPP
        m_appWindow->destroy();
        LOGD(TAG, "%s: --destroy\n", __func__);
#endif
    }
    LOGD(TAG, "--%s %d\n", __func__, m_appID);
    return 0;
}

int CQObjListener::doShowFrontUI(void)
{
    LOGD(TAG, "++%s %d: m_appWindow(%p)\n",
                __func__, m_appID, m_appWindow);
    if (m_appWindow) {
#ifdef WITH_SOAPP
        LOGD(TAG, "doShowFrontUI::m_appWindow->create\n");
        m_appWindow->create();
        LOGD(TAG, "%s: --create\n", __func__);
#endif
        LOGD(TAG, "%s: ++show\n", __func__);
        m_appWindow->show();
        LOGD(TAG, "%s: --show\n", __func__);
    }
    LOGD(TAG, "--%s %d\n", __func__, m_appID);
    return 0;
}

int CQObjListener::doHideFront(int param1, int param2)
{
    LOGD(TAG, "%s %d: param1(%d), param2(%d)\n",
                __func__, m_appID, param1, param2);
    return 0;
}

int CQObjListener::doShowFront(int param1, int param2)
{
    LOGD(TAG, "%s %d: param1(%d), param2(%d)\n",
                __func__, m_appID, param1, param2);
    return 0;
}

int CQObjListener::doShowRear(int param1, int param2)
{
    LOGD(TAG, "%s: param1(%d), param2(%d)", __func__, param1, param2);
    return 0;
}

int CQObjListener::doHideRear(int param1, int param2)
{
    LOGD(TAG, "%s: param1(%d), param2(%d)", __func__, param1, param2);
    return 0;
}

int CQObjListener::doAudioFocusChanged(CCtlListener::E_AVOUT aOut,
                                        CCtlListener::E_AUDIOFOCUS focus)
{
    LOGD(TAG, "%s: aOut(%d), focus(%d)", __func__, aOut, focus);
    return 0;
}

int CQObjListener::doVideoFocusChanged(CCtlListener::E_AVOUT vOut,
                                        CCtlListener::E_VIDEOFOCUS focus)
{
    LOGD(TAG, "%s: vOut(%d), focus(%d)", __func__, vOut, focus);
    return 0;
}

bool CQObjListener::doKeyEvent(int key, int param1, int param2)
{
    LOGD(TAG, "%s: key(%d), param1(%d), param2(%d)",
            __func__, key, param1, param2);
    return false;
}

