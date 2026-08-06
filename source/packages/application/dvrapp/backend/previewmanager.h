/*
 * Copyright (c) 2025 AutoChips Inc.
 *
 * PreviewManager - Preview stream and surface management
 * Implements single shared surface pattern (matches original app)
 */

#ifndef PREVIEWMANAGER_H
#define PREVIEWMANAGER_H

#include <QObject>
#include <functional>  // For std::function callback
#include "dvr.h"  // For DVR_CAM_TYPE_E and DVR APIs
#include "atcsurface.h"  // For IAtcSurface

/**
 * @brief PreviewManager - Manages preview streams and display surface
 *
 * Responsibilities:
 * - Create and manage shared IAtcSurface for preview display
 * - Start/stop preview for specific camera via DVR_StartPreviewByCamera()
 * - Track preview active state per camera
 * - Handle preview errors and notify via signals
 *
 * Design Pattern (Single Shared Surface):
 * - ONE surface created at initialization, reused for all cameras
 * - Camera switch = rebind same surface to different camera
 * - Surface released only at shutdown (not during camera switch)
 * - Based on original app pattern (dvrdemo.cpp line 1416, ~960)
 *
 * Thread Safety:
 * - All methods run in Qt main thread
 * - DVR lib handles internal thread synchronization
 */
class PreviewManager : public QObject
{
    Q_OBJECT

public:
    explicit PreviewManager(QObject *parent = nullptr);
    ~PreviewManager();

    /**
     * @brief Initialize preview manager
     * - Creates shared IAtcSurface with specified resolution (NV12 format)
     * - Surface is reused across all camera switches
     * @param screenWidth Screen width (e.g., 800 or 1024)
     * @param screenHeight Screen height (e.g., 480 or 600)
     * @return true if initialization succeeds
     */
    bool initialize(int screenWidth, int screenHeight);

    /**
     * @brief Shutdown preview manager
     * - Stops all active previews
     * - Releases shared surface
     */
    void shutdown();

    /**
     * @brief Stop preview for specific camera
     * - Stops preview stream (DVR_StopPreviewByCamera)
     * - Unbinds surface from camera (DVR_SetPreviewSurface with NULL)
     * - Optionally releases surface completely (for file list view)
     * @param camera Camera type (FRONT or REAR)
     * @param releaseSurface If true, completely release surface to free /dev/video0
     * @return true if preview stops successfully
     */
    bool stopPreview(DVR_CAM_TYPE_E camera, bool releaseSurface = false);

    /**
     * @brief Start preview for specific camera
     * - Recreates surface if needed (after being released)
     * - Initializes camera if not already initialized (DVR_InitSingleCameraByType)
     * - Binds surface to camera (DVR_SetPreviewSurface)
     * - Starts preview stream (DVR_StartPreviewByCamera)
     * @param camera Camera type (FRONT or REAR)
     * @return true if preview starts successfully
     */
    bool startPreview(DVR_CAM_TYPE_E camera);

    /**
     * @brief Check if preview is active for specific camera
     * @param camera Camera type (FRONT or REAR)
     * @return true if preview is running
     */
    bool isPreviewActive(DVR_CAM_TYPE_E camera) const;

    /**
     * @brief Set callback to get current preview camera
     * - Used when DVR library events don't include camera type in param1
     * - Enables PreviewManager to determine which camera to operate on
     * @param callback Function that returns current preview camera type
     */
    void setCurrentCameraGetter(std::function<DVR_CAM_TYPE_E()> callback);

public slots:
    /**
     * @brief Handle DVR library events (called via Qt::QueuedConnection from DVR thread)
     *
     * THREAD SAFETY: This slot is invoked via QMetaObject::invokeMethod with
     * Qt::QueuedConnection from DVRBackend::handleDvrEvent(). The DVR library
     * event callback runs in DVR internal thread, but this method executes in
     * Qt main thread, making it safe to access member variables and emit signals.
     *
     * Events handled:
     * - DVR_UI_MSG_PREV_STARTED: Confirm preview started, emit previewStateChanged(true)
     * - DVR_UI_MSG_PREV_STOPED: Confirm preview stopped, emit previewStateChanged(false)
     * - DVR_UI_MSG_START_PREV_FAILED: Emit previewError() with failure message
     * - DVR_UI_MSG_STOP_PREV_FAILED: Emit previewError() with failure message
     * - DVR_UI_MSG_CAM_ERROR: Auto-stop preview, emit previewError() with camera error
     *
     * @param eventType DVR event type (from DVR_UI_MSG_E enum)
     * @param param1 Event parameter 1 (typically camera type)
     * @param param2 Event parameter 2 (error code or reserved)
     */
    void onDvrEvent(uint eventType, uint param1, uint param2);

signals:
    /**
     * @brief Emitted when preview state changes
     * @param camera Camera type (FRONT or REAR)
     * @param active true if preview started, false if stopped
     */
    void previewStateChanged(DVR_CAM_TYPE_E camera, bool active);

    /**
     * @brief Emitted when preview error occurs
     * @param camera Camera type (FRONT or REAR)
     * @param message Error description
     */
    void previewError(DVR_CAM_TYPE_E camera, const QString &message);

private:
    /**
     * @brief Per-camera preview state tracking
     */
    struct CameraPreviewState {
        bool isActive;  // Is preview currently running

        CameraPreviewState() : isActive(false) {}
    };

    /**
     * @brief Get preview state for specific camera
     * @param camera Camera type
     * @return Pointer to state struct, or nullptr if invalid
     */
    CameraPreviewState* getPreviewState(DVR_CAM_TYPE_E camera);
    const CameraPreviewState* getPreviewState(DVR_CAM_TYPE_E camera) const;

    // Initialization state
    bool m_initialized;

    // Shared surface for all cameras (single surface pattern)
    IAtcSurface *m_sharedSurface;

    // Per-camera preview state
    CameraPreviewState m_frontPreview;
    CameraPreviewState m_rearPreview;

    // Callback to get current preview camera (for DVR event handling)
    std::function<DVR_CAM_TYPE_E()> m_getCurrentCamera;

    // Surface configuration (dynamically set based on screen resolution)
    // These dimensions are set during initialize() to match actual screen size
    int m_surfaceWidth;
    int m_surfaceHeight;
};

#endif // PREVIEWMANAGER_H
