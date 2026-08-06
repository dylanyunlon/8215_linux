/*
 * DVR QML Refactor - System Event Listener
 *
 * DVRQObjListener handles system-level events from AutoChips framework
 */

#ifndef DVRQOBJLISTENER_H
#define DVRQOBJLISTENER_H

#include <QObject>
#include "qobjlistener.h"  // AutoChips system listener base class (CQObjListener)

// Forward declaration
class DVRBackend;

/**
 * @brief DVR-specific system event listener
 *
 * DVRQObjListener inherits from CQObjListener (AutoChips system framework)
 * to handle system events:
 * - Front/Rear UI show/hide (app brought to foreground/background)
 * - Audio/Video focus changes (resource conflicts with other apps)
 * - Key events (hardware buttons)
 * - System exit requests
 *
 * All events are delegated to DVRBackend for actual handling logic.
 */
class DVRQObjListener : public CQObjListener {
public:
    explicit DVRQObjListener(DVRBackend *backend);
    virtual ~DVRQObjListener();

    // ===== UI Visibility Events =====

    /**
     * @brief Front UI show request (app brought to foreground)
     * @param param1 Reserved parameter
     * @param param2 Reserved parameter
     * @return 0 for success
     */
    virtual int doShowFront(int param1, int param2) override;

    /**
     * @brief Front UI hide request (app sent to background)
     * @param param1 Reserved parameter
     * @param param2 Reserved parameter
     * @return 0 for success
     */
    virtual int doHideFront(int param1, int param2) override;

    /**
     * @brief Front UI show implementation (QML window show)
     * @return 0 for success
     *
     * NOTE: This is NOT virtual - uses function hiding to intercept base class calls
     */
    int doShowFrontUI(void);

    /**
     * @brief Front UI hide implementation (QML window hide)
     * @return 0 for success
     *
     * CRITICAL: Properly destroy QWindow handle to avoid "second open black screen" issue.
     * NOTE: This is NOT virtual - uses function hiding to intercept base class calls
     */
    int doHideFrontUI(void);

    /**
     * @brief Rear UI show request (for rear-seat display, if supported)
     */
    virtual int doShowRear(int param1, int param2) override;

    /**
     * @brief Rear UI hide request
     */
    virtual int doHideRear(int param1, int param2) override;

    // ===== Focus Events =====

    /**
     * @brief Video focus change handler
     * @param vOut Video output type
     * @param focus Focus state (FOCUS_GAIN/FOCUS_LOSS)
     * @return 0 for success
     */
    virtual int doVideoFocusChanged(CCtlListener::E_AVOUT vOut,
                                    CCtlListener::E_VIDEOFOCUS focus) override;

    /**
     * @brief Audio focus change handler
     * @param aOut Audio output type
     * @param focus Focus state
     * @return 0 for success
     */
    virtual int doAudioFocusChanged(CCtlListener::E_AVOUT aOut,
                                    CCtlListener::E_AUDIOFOCUS focus) override;

    // ===== System Events =====

    /**
     * @brief System exit request
     * @return 0 for success
     */
    virtual int doExit(int param1, int param2) override;

    /**
     * @brief Hardware key event
     * @return false if handled, true to pass to system
     */
    virtual bool doKeyEvent(int key, int param1, int param2) override;

private:
    DVRBackend *m_backend;  // Not owned, just a reference
};

#endif // DVRQOBJLISTENER_H
