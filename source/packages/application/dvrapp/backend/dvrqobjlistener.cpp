/*
 * DVR QML Refactor - System Event Listener Implementation
 */

#include "dvrqobjlistener.h"
#include "dvrbackend.h"
#include "appobj.h"
#include <QDebug>
#include <QGuiApplication>

DVRQObjListener::DVRQObjListener(DVRBackend *backend)
    : CQObjListener(CAPPBaseObj::APPID_DVR)  // APPID_DVR = 19
    , m_backend(backend)
{
    qDebug() << "[DVRQObjListener] Constructor called, APPID_DVR = 19";
}

DVRQObjListener::~DVRQObjListener()
{
    qDebug() << "[DVRQObjListener] Destructor called";
}

// ===== UI Visibility Events =====

int DVRQObjListener::doShowFront(int param1, int param2)
{
    qDebug() << "[DVRQObjListener] doShowFront called, param1:" << param1
             << "param2:" << param2;

    // Call base class implementation first
    CQObjListener::doShowFront(param1, param2);

    // Delegate to backend
    if (m_backend) {
        m_backend->handleShowFront(param1, param2);
    }

    return 0;
}

int DVRQObjListener::doHideFront(int param1, int param2)
{
    qDebug() << "[DVRQObjListener] doHideFront called, param1:" << param1
             << "param2:" << param2;

    // Call base class implementation first
    CQObjListener::doHideFront(param1, param2);

    // Delegate to backend
    if (m_backend) {
        m_backend->handleHideFront(param1, param2);
    }

    return 0;
}

int DVRQObjListener::doShowFrontUI(void)
{
    qDebug() << "[DVRQObjListener] doShowFrontUI called - delegate to base class";

    // Base class CQObjListener handles window show/hide/create/destroy operations
    // Since we use QQuickWindow (which inherits QWindow), base class can handle it directly
    return CQObjListener::doShowFrontUI();
}

int DVRQObjListener::doHideFrontUI(void)
{
    qDebug() << "[DVRQObjListener] doHideFrontUI called - delegate to base class";

    // Base class CQObjListener handles window show/hide/create/destroy operations
    // Since we use QQuickWindow (which inherits QWindow), base class can handle it directly
    // Base class will properly call hide() and destroy() to prevent black screen on next open
    return CQObjListener::doHideFrontUI();
}

int DVRQObjListener::doShowRear(int param1, int param2)
{
    qDebug() << "[DVRQObjListener] doShowRear called (not supported in DVR)";

    // Call base class implementation
    CQObjListener::doShowRear(param1, param2);

    // DVR app doesn't have rear display support
    return 0;
}

int DVRQObjListener::doHideRear(int param1, int param2)
{
    qDebug() << "[DVRQObjListener] doHideRear called (not supported in DVR)";

    // Call base class implementation
    CQObjListener::doHideRear(param1, param2);

    // DVR app doesn't have rear display support
    return 0;
}

// ===== Focus Events =====

int DVRQObjListener::doVideoFocusChanged(CCtlListener::E_AVOUT vOut,
                                         CCtlListener::E_VIDEOFOCUS focus)
{
    qDebug() << "[DVRQObjListener] doVideoFocusChanged, vOut:" << vOut
             << "focus:" << focus;

    // Call base class implementation
    CQObjListener::doVideoFocusChanged(vOut, focus);

    // Delegate to backend
    if (m_backend) {
        m_backend->handleVideoFocusChanged(vOut, focus);
    }

    return 0;
}

int DVRQObjListener::doAudioFocusChanged(CCtlListener::E_AVOUT aOut,
                                         CCtlListener::E_AUDIOFOCUS focus)
{
    qDebug() << "[DVRQObjListener] doAudioFocusChanged, aOut:" << aOut
             << "focus:" << focus;

    // Call base class implementation
    CQObjListener::doAudioFocusChanged(aOut, focus);

    // Delegate to backend
    if (m_backend) {
        m_backend->handleAudioFocusChanged(aOut, focus);
    }

    return 0;
}

// ===== System Events =====

int DVRQObjListener::doExit(int param1, int param2)
{
    qDebug() << "[DVRQObjListener] doExit called, param1:" << param1
             << "param2:" << param2;

    // Call base class implementation
    CQObjListener::doExit(param1, param2);

    // Quit the application
    qDebug() << "[DVRQObjListener] Quitting application";
    QGuiApplication::quit();

    return 0;
}

bool DVRQObjListener::doKeyEvent(int key, int param1, int param2)
{
    qDebug() << "[DVRQObjListener] doKeyEvent, key:" << key
             << "param1:" << param1 << "param2:" << param2;

    // Call base class implementation
    CQObjListener::doKeyEvent(key, param1, param2);

    // For now, let system handle all keys
    // Return true = pass to system
    // Return false = event handled, don't pass to system
    return true;
}
