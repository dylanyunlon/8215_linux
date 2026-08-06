/*
 * Copyright (c) 2025 AutoChips Inc.
 *
 * DVRBackend - Main coordinator implementation
 */

#include "dvrbackend.h"
#include "dvrqobjlistener.h"
#include "cameramanager.h"
#include "previewmanager.h"
#include "recordmanager.h"
#include "playbackmanager.h"
#include "filemanager.h"
#include "settingsmanager.h"
#include "dvrlog.h"
#include "globalbus.h"      // For GlobalBus::applyFor

// System headers for device availability detection
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "appobj.h"         // For CAPPBaseObj::APPID_*
#include "dvr.h"            // For DVR_Init and other DVR APIs
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QGuiApplication>
#include <QScreen>
#include <cstring>          // For memset

static const char* TAG = "DVRBackend";

// Global pointer to DVRBackend instance for event callback
// This is needed because DVR_Init() requires a C-style callback function
static DVRBackend* g_dvrBackendInstance = nullptr;

// Global callback functions for DVR_Init (following original app pattern)
static void dvrAttachThreadCallback()
{
    // Empty - DVR library may use this for JNI thread management
    // In Qt application, we don't need explicit thread attach/detach
}

static void dvrEventCallback(__u32 event, __u32 param1, __u32 param2)
{
    // DVR library event notification
    LOGI(TAG, "[DVR_EVENT] event=0x%X, param1=%u, param2=%u", event, param1, param2);

    // Forward event to DVRBackend instance if available
    if (g_dvrBackendInstance) {
        g_dvrBackendInstance->handleDvrEvent(event, param1, param2);
    } else {
        LOGW(TAG, "[DVR_EVENT] No DVRBackend instance to handle event");
    }
}

static void dvrDetachThreadCallback()
{
    // Empty - DVR library thread cleanup
}

DVRBackend::DVRBackend(QObject *parent)
    : QObject(parent)
    , m_cameraManager(nullptr)
    , m_previewManager(nullptr)
    , m_recordManager(nullptr)
    , m_playbackManager(nullptr)
    , m_fileManager(nullptr)
    , m_settingsManager(nullptr)
    , m_qobjListener(nullptr)
    , m_isFrontCamera(true)     // Default to front camera
    , m_isDualCameraMode(false)
    , m_initialized(false)
{
    LOGI(TAG, "Constructor");

    // Set global instance pointer for DVR event callback
    g_dvrBackendInstance = this;
}

DVRBackend::~DVRBackend()
{
    LOGI(TAG, "Destructor - start cleanup");

    // Clear global instance pointer
    g_dvrBackendInstance = nullptr;

    shutdown();
    LOGI(TAG, "Destructor - cleanup complete");
}

bool DVRBackend::initialize()
{
    if (m_initialized) {
        LOGW(TAG, "Already initialized");
        return true;
    }

    LOGI(TAG, "Initializing...");

    // ========================================================================
    // CRITICAL: Initialize DVR library global instance FIRST
    // This must be called before any DVR_InitSingleCameraByType() or other
    // camera operations. Without this, DVR APIs will fail with
    // "DVRAV instance not initialized" error.
    // ========================================================================

    LOGI(TAG, "Step 1: Initializing DVR library global instance...");

    // Prepare default video info structure
    DVR_VIDEO_INFO_T videoInfo;
    memset(&videoInfo, 0, sizeof(DVR_VIDEO_INFO_T));
    videoInfo.u4Width = 1280;       // Default preview resolution
    videoInfo.u4Height = 720;
    videoInfo.u4FrameRate = 30;     // 30 fps

    // Initialize DVR library with callbacks
    // Note: Callbacks follow original app pattern (dvrdemo.cpp:1371)
    bool dvrInitResult = DVR_Init(
        dvrAttachThreadCallback,    // Thread attach (for JNI compatibility)
        dvrEventCallback,           // Event notification callback
        dvrDetachThreadCallback,    // Thread detach cleanup
        &videoInfo                  // Default video configuration
    );

    if (!dvrInitResult) {
        LOGE(TAG, "DVR_Init() failed! Cannot proceed without DVR library");
        return false;
    }

    LOGI(TAG, "DVR_Init() succeeded - DVR library ready");

    // Configure video settings (following original app startup sequence)
    DVR_SetVideoInfo(&videoInfo);
    LOGI(TAG, "DVR video info configured: %dx%d @%dfps",
         videoInfo.u4Width, videoInfo.u4Height, videoInfo.u4FrameRate);

    // Note: Recording path/duration/capacity are configured by RecordManager::initialize()
    // No need to configure them here to maintain separation of concerns

    LOGI(TAG, "Step 1 complete: DVR library initialized and configured");

    // ========================================================================
    // Step 2: Create application managers (now safe to use DVR APIs)
    // ========================================================================

    // Create DVRQObjListener to handle system events
    m_qobjListener = new DVRQObjListener(this);
    LOGI(TAG, "Created DVRQObjListener (APPID_DVR)");

    // Create CameraManager
    m_cameraManager = new CameraManager(this);
    connect(m_cameraManager, &CameraManager::cameraAvailabilityChanged,
            this, &DVRBackend::onCameraAvailabilityChanged);

    if (!m_cameraManager->initialize()) {
        LOGE(TAG, "Failed to initialize CameraManager");
        return false;
    }
    LOGI(TAG, "CameraManager initialized");

    // Create PreviewManager
    m_previewManager = new PreviewManager(this);
    connect(m_previewManager, &PreviewManager::previewStateChanged,
            this, &DVRBackend::onPreviewStateChanged);
    connect(m_previewManager, &PreviewManager::previewError,
            this, &DVRBackend::onPreviewError);

    // Register callback for PreviewManager to get current preview camera
    // This enables PreviewManager to determine which camera to operate on
    // when DVR library events don't include camera type information
    m_previewManager->setCurrentCameraGetter([this]() {
        return getCurrentCamera();
    });
    LOGI(TAG, "PreviewManager current camera callback registered");

    // Get actual screen resolution for surface creation
    // CRITICAL: Surface size MUST match screen resolution to avoid scaling artifacts
    int screenWidth = 1024;   // Default fallback for large screen
    int screenHeight = 600;
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect geometry = screen->geometry();
        screenWidth = geometry.width();
        screenHeight = geometry.height();
        LOGI(TAG, "[OK] Detected screen resolution: %dx%d", screenWidth, screenHeight);
    } else {
        LOGW(TAG, "Could not get screen info, using default: %dx%d", screenWidth, screenHeight);
    }

    if (!m_previewManager->initialize(screenWidth, screenHeight)) {
        LOGE(TAG, "Failed to initialize PreviewManager");
        return false;
    }
    LOGI(TAG, "[OK] PreviewManager initialized with screen resolution: %dx%d", screenWidth, screenHeight);

    // Create RecordManager
    m_recordManager = new RecordManager(this);
    connect(m_recordManager, &RecordManager::recordingStateChanged,
            this, &DVRBackend::onRecordingStateChanged);
    connect(m_recordManager, &RecordManager::recordingStarted,
            this, &DVRBackend::onRecordingStarted);
    connect(m_recordManager, &RecordManager::recordingStopped,
            this, &DVRBackend::onRecordingStopped);
    connect(m_recordManager, &RecordManager::recordingError,
            this, &DVRBackend::onRecordingError);
    connect(m_recordManager, &RecordManager::storageFullWarning,
            this, &DVRBackend::onStorageFullWarning);

    // Register callback for RecordManager to get current preview camera
    // This enables RecordManager to determine which camera to operate on
    // when DVR library events don't include camera type information
    m_recordManager->setCurrentCameraGetter([this]() {
        return getCurrentCamera();
    });
    LOGI(TAG, "RecordManager current camera callback registered");

    if (!m_recordManager->initialize()) {
        LOGE(TAG, "Failed to initialize RecordManager");
        return false;
    }
    LOGI(TAG, "RecordManager initialized");

    // Create SettingsManager
    m_settingsManager = new SettingsManager(this);
    LOGI(TAG, "SettingsManager created (managed by DVRBackend)");
    
    // Create FileManager
    m_fileManager = new FileManager(m_recordManager, this);
    // Get recording path from RecordManager (it knows the actual path used by DVR library)
    QString recordPath = m_recordManager->getRecordPath();
    m_fileManager->setRecordPath(recordPath);
    LOGI(TAG, "FileManager created with path from RecordManager: %s", qPrintable(recordPath));
    
    // Create PlaybackManager
    m_playbackManager = new PlaybackManager(this);
    LOGI(TAG, "PlaybackManager created (managed by DVRBackend)");

    m_initialized = true;
    LOGI(TAG, "Initialization complete");

    // Schedule auto-preview with delay (gives DVR lib time to initialize)
    // KEY: Use QTimer for non-blocking delayed start
    LOGI(TAG, "Scheduling auto-preview in %d ms", AUTO_PREVIEW_DELAY_MS);
    QTimer::singleShot(AUTO_PREVIEW_DELAY_MS, this, [this]() {
        tryAutoStartPreview(0);  // Start with retryCount = 0
    });

    return true;
}

void DVRBackend::shutdown()
{
    if (!m_initialized) {
        return;
    }

    LOGI(TAG, "Shutting down...");

    // Shutdown RecordManager (stop all recordings first)
    if (m_recordManager) {
        m_recordManager->shutdown();
        delete m_recordManager;
        m_recordManager = nullptr;
        LOGI(TAG, "RecordManager destroyed");
    }

    // Shutdown PreviewManager
    if (m_previewManager) {
        m_previewManager->shutdown();
        delete m_previewManager;
        m_previewManager = nullptr;
        LOGI(TAG, "PreviewManager destroyed");
    }

    // Shutdown CameraManager
    if (m_cameraManager) {
        m_cameraManager->shutdown();
        delete m_cameraManager;
        m_cameraManager = nullptr;
        LOGI(TAG, "CameraManager destroyed");
    }
    
    // Destroy FileManager
    if (m_fileManager) {
        delete m_fileManager;
        m_fileManager = nullptr;
        LOGI(TAG, "FileManager destroyed");
    }
    
    // Destroy PlaybackManager
    if (m_playbackManager) {
        delete m_playbackManager;
        m_playbackManager = nullptr;
        LOGI(TAG, "PlaybackManager destroyed");
    }
    
    // Destroy SettingsManager
    if (m_settingsManager) {
        delete m_settingsManager;
        m_settingsManager = nullptr;
        LOGI(TAG, "SettingsManager destroyed");
    }

    // Clean up system event listener
    if (m_qobjListener) {
        delete m_qobjListener;
        m_qobjListener = nullptr;
        LOGI(TAG, "DVRQObjListener destroyed");
    }

    // Deinitialize DVR library (last step, after all managers destroyed)
    LOGI(TAG, "Calling DVR_DeInit() to cleanup DVR library...");
    DVR_DeInit();
    LOGI(TAG, "DVR_DeInit() completed - DVR library cleaned up");

    m_initialized = false;
    LOGI(TAG, "Shutdown complete");
}

bool DVRBackend::isPreviewing() const
{
    if (!m_previewManager) {
        return false;
    }

    // Check if current camera has active preview
    DVR_CAM_TYPE_E currentCamera = getCurrentCamera();
    return m_previewManager->isPreviewActive(currentCamera);
}

bool DVRBackend::isRecording() const
{
    if (!m_recordManager) {
        return false;
    }

    // Check if CURRENT camera is recording (reflects current preview camera state)
    DVR_CAM_TYPE_E currentCamera = getCurrentCamera();
    return m_recordManager->isRecordingActive(currentCamera);
}

void DVRBackend::setFrontCamera(bool front)
{
    if (m_isFrontCamera == front) {
        return;
    }

    LOGI(TAG, "Front camera property changed: %s", front ? "true" : "false");
    m_isFrontCamera = front;
    emit frontCameraChanged(front);
}

void DVRBackend::setDualCameraMode(bool dual)
{
    if (m_isDualCameraMode == dual) {
        return;
    }

    LOGI(TAG, "Dual camera mode changed: %s", dual ? "true" : "false");
    m_isDualCameraMode = dual;
    emit dualCameraModeChanged(dual);
}

void DVRBackend::switchCamera()
{
    if (!m_initialized) {
        LOGW(TAG, "Not initialized, ignoring switch request");
        return;
    }

    // Rescan devices to update CameraManager state after USB mode changes
    LOGI(TAG, "Rescanning devices before camera switch");
    m_cameraManager->rescanDevices();

    DVR_CAM_TYPE_E oldCamera = getCurrentCamera();
    DVR_CAM_TYPE_E newCamera = (oldCamera == DVR_CAM_TYPE_FRONT) ?
                                DVR_CAM_TYPE_REAR : DVR_CAM_TYPE_FRONT;

    LOGI(TAG, "Switching from camera %d to camera %d", oldCamera, newCamera);

    // Check if new camera is connected
    if (!m_cameraManager->isCameraConnected(newCamera)) {
        LOGW(TAG, "New camera %d not connected", newCamera);
        emit cameraConnectionError(tr("Camera not connected"));
        return;
    }

    // Stop old camera preview
    if (m_previewManager->isPreviewActive(oldCamera)) {
        LOGI(TAG, "Stopping old camera %d preview", oldCamera);
        m_previewManager->stopPreview(oldCamera);
    }

    // Initialize new camera if needed
    if (!DVR_IsCameraInitialized(newCamera)) {
        LOGI(TAG, "Initializing new camera %d", newCamera);
        if (!DVR_InitSingleCameraByType(newCamera)) {
            LOGE(TAG, "Failed to initialize new camera %d", newCamera);
            emit cameraConnectionError(tr("Failed to initialize camera"));

            // Restart old camera preview as fallback
            LOGI(TAG, "Restarting old camera %d preview as fallback", oldCamera);
            m_previewManager->startPreview(oldCamera);
            return;
        }

        // Give device time to stabilize
        QThread::msleep(200);
    }

    // Start new camera preview FIRST (before updating state)
    LOGI(TAG, "Starting new camera %d preview", newCamera);
    if (!m_previewManager->startPreview(newCamera)) {
        LOGE(TAG, "Failed to start new camera %d preview", newCamera);
        emit cameraConnectionError(tr("Failed to start preview"));

        // Restart old camera (state unchanged, no need to revert)
        LOGW(TAG, "Restarting old camera %d", oldCamera);
        m_previewManager->startPreview(oldCamera);
        return;
    }

    // Update button state ONLY AFTER preview starts successfully
    // This prevents UI icon from changing if switch fails
    m_isFrontCamera = (newCamera == DVR_CAM_TYPE_FRONT);
    emit frontCameraChanged(m_isFrontCamera);

    // IMPORTANT: Emit state change signals to update button states for new camera
    // - isRecording() and isPreviewing() depend on getCurrentCamera() which just changed
    // - QML bindings need notification to re-evaluate button icons
    // - Each camera has independent recording/preview state
    // - Example: Front recording ??switch to rear ??button should show "start" (rear not recording)
    emit recordingChanged(isRecording());
    emit previewingChanged(isPreviewing());

    LOGI(TAG, "Camera switch completed successfully");
}

// Video player navigation methods
void DVRBackend::showVideoPlayer(const QString &filePath)
{
    LOGI(TAG, "========================================");
    LOGI(TAG, "showVideoPlayer: Switching to video player");
    LOGI(TAG, "File: %s", qPrintable(filePath));
    LOGI(TAG, "========================================");

    // STEP 1: Stop preview and release Surface to free /dev/video0
    DVR_CAM_TYPE_E currentCamera = getCurrentCamera();
    if (m_previewManager) {
        LOGI(TAG, "STEP 1: Stopping preview for camera %d and releasing Surface", currentCamera);
        
        // CRITICAL: Pass releaseSurface=true to release IAtcSurface
        // This decrements Surface reference count to 0, triggering close(dev_fd)
        // Without this, /dev/video0 remains held by preview Surface (EBUSY)
        m_previewManager->stopPreview(currentCamera, true);  // ¡û releaseSurface=true

        // STEP 1.5: Wait for device to become available (adaptive polling)
        // Should succeed quickly now that Surface is released
        LOGI(TAG, "STEP 1.5: Polling /dev/video0 availability (max 500ms)...");
        if (waitForDeviceReady(500)) {
            LOGI(TAG, "[OK] STEP 1: /dev/video0 is now available for playback");
        } else {
            LOGE(TAG, "[ERROR] STEP 1: /dev/video0 still busy after 500ms timeout");
            LOGW(TAG, "[WARNING] Proceeding with playback anyway (may fallback to /dev/video1)");
        }
    } else {
        LOGW(TAG, "STEP 1: PreviewManager is NULL, skipping preview stop");
    }

    // STEP 2: Initialize playback (pure playback logic)
    if (!m_playbackManager) {
        LOGE(TAG, "[ERROR] PlaybackManager is NULL, cannot start playback");
        emit errorOccurred("Playback manager not available");
        
        // Rollback: Restart preview on failure
        if (m_previewManager) {
            LOGI(TAG, "[ROLLBACK] Restarting preview after NULL manager check");
            QTimer::singleShot(200, this, [this, currentCamera]() {
                if (m_previewManager) {
                    m_previewManager->startPreview(currentCamera);
                }
            });
        }
        return;
    }

    if (!m_playbackManager->initialize(filePath)) {
        LOGE(TAG, "[ERROR] Failed to initialize playback");
        
        // Rollback: Restart preview on failure
        if (m_previewManager) {
            LOGI(TAG, "[ROLLBACK] Restarting preview after initialization failure");
            QTimer::singleShot(200, this, [this, currentCamera]() {
                if (m_previewManager) {
                    m_previewManager->startPreview(currentCamera);
                }
            });
        }
        
        emit errorOccurred("Failed to initialize video player");
        return;
    }
    LOGI(TAG, "STEP 2: Playback initialized successfully");

    // STEP 3: Navigate to video player UI
    LOGI(TAG, "STEP 3: Emitting videoPlayerRequested signal");
    emit videoPlayerRequested(filePath);

    LOGI(TAG, "========================================");
}

void DVRBackend::hideVideoPlayer()
{
    LOGI(TAG, "========================================");
    LOGI(TAG, "hideVideoPlayer: Returning from video player");
    LOGI(TAG, "========================================");

    // STEP 1: Deinitialize playback (pure playback cleanup)
    if (m_playbackManager) {
        m_playbackManager->deinitialize();
        LOGI(TAG, "STEP 1: Playback deinitialized, resources released");
    } else {
        LOGW(TAG, "STEP 1: PlaybackManager is NULL, skipping deinitialize");
    }

    // STEP 2: Wait for device to be released, then restart preview
    // File list shows preview in background for smooth transition
    DVR_CAM_TYPE_E currentCamera = getCurrentCamera();
    if (m_previewManager) {
        LOGI(TAG, "STEP 2: Preparing to restart preview for file list background");

        // CRITICAL: Wait for /dev/video0 to be released by playback before preview
        // Playback uses /dev/video0 (hardware), must fully release before preview can use it
        QTimer::singleShot(50, this, [this, currentCamera]() {
            if (m_previewManager) {
                // STEP 2.1: Poll for device availability
                LOGI(TAG, "[PREVIEW_RESTART] Polling /dev/video0 availability (max 500ms)...");
                if (waitForDeviceReady(500)) {
                    LOGI(TAG, "[PREVIEW_RESTART] [OK] /dev/video0 is now available for preview");
                } else {
                    LOGE(TAG, "[PREVIEW_RESTART] [ERROR] /dev/video0 still busy after 500ms timeout");
                    LOGW(TAG, "[PREVIEW_RESTART] [WARNING] Attempting preview anyway (may fail)");
                }

                // STEP 2.2: Start preview
                LOGI(TAG, "[PREVIEW_RESTART] Starting preview for camera %d", currentCamera);
                if (!m_previewManager->startPreview(currentCamera)) {
                    LOGE(TAG, "[PREVIEW_RESTART] Failed to restart preview");
                } else {
                    LOGI(TAG, "[PREVIEW_RESTART] Preview restarted successfully");
                }
            }
        });
    } else {
        LOGW(TAG, "STEP 2: PreviewManager is NULL, skipping preview restart");
    }

    // STEP 3: Navigate back to file list
    LOGI(TAG, "STEP 3: Emitting videoPlayerClosed signal");
    emit videoPlayerClosed();

    LOGI(TAG, "========================================");
}

// Recording control methods
bool DVRBackend::startRecording()
{
    if (!m_initialized || !m_recordManager) {
        LOGW(TAG, "Not initialized or RecordManager unavailable, cannot start recording");
        return false;
    }

    DVR_CAM_TYPE_E currentCamera = getCurrentCamera();
    LOGI(TAG, "[DEBUG_RECORDING] startRecording() called for camera %d", currentCamera);

    // Check if already recording
    if (m_recordManager->isRecordingActive(currentCamera)) {
        LOGW(TAG, "Camera %d already recording", currentCamera);
        return true;  // Not an error
    }

    // Start recording for current camera
    bool success = m_recordManager->startRecording(currentCamera);

    if (success) {
        LOGI(TAG, "[OK] Recording start requested for camera %d", currentCamera);
    } else {
        LOGE(TAG, "[ERROR] Failed to start recording for camera %d", currentCamera);
    }

    return success;
}

bool DVRBackend::stopRecording()
{
    if (!m_initialized || !m_recordManager) {
        LOGW(TAG, "Not initialized or RecordManager unavailable, cannot stop recording");
        return false;
    }

    DVR_CAM_TYPE_E currentCamera = getCurrentCamera();
    LOGI(TAG, "[DEBUG_RECORDING] stopRecording() called for camera %d", currentCamera);

    // Check if actually recording
    if (!m_recordManager->isRecordingActive(currentCamera)) {
        LOGD(TAG, "Camera %d not recording, nothing to stop", currentCamera);
        return true;  // Not an error
    }

    // Stop recording for current camera
    bool success = m_recordManager->stopRecording(currentCamera);

    if (success) {
        LOGI(TAG, "[OK] Recording stop requested for camera %d", currentCamera);
    } else {
        LOGE(TAG, "[ERROR] Failed to stop recording for camera %d", currentCamera);
    }

    return success;
}

bool DVRBackend::toggleRecording()
{
    if (!m_initialized || !m_recordManager) {
        LOGW(TAG, "Not initialized or RecordManager unavailable, cannot toggle recording");
        return false;
    }

    DVR_CAM_TYPE_E currentCamera = getCurrentCamera();
    bool isCurrentlyRecording = m_recordManager->isRecordingActive(currentCamera);

    LOGI(TAG, "[DEBUG_RECORDING] toggleRecording() called, current state: %s",
         isCurrentlyRecording ? "recording" : "stopped");

    if (isCurrentlyRecording) {
        return stopRecording();
    } else {
        return startRecording();
    }
}

DVR_CAM_TYPE_E DVRBackend::getCurrentCamera() const
{
    return m_isFrontCamera ? DVR_CAM_TYPE_FRONT : DVR_CAM_TYPE_REAR;
}

void DVRBackend::tryAutoStartPreview(int retryCount)
{
    LOGI(TAG, "tryAutoStartPreview attempt %d of %d",
         retryCount + 1, PREVIEW_MAX_RETRY_COUNT);

    // Check retry limit
    if (retryCount >= PREVIEW_MAX_RETRY_COUNT) {
        LOGE(TAG, "Auto-preview timeout after %d retries", PREVIEW_MAX_RETRY_COUNT);
        emit cameraConnectionError(tr("Failed to start preview - DVR not ready after %1 seconds")
                                   .arg((AUTO_PREVIEW_DELAY_MS + PREVIEW_MAX_RETRY_COUNT * PREVIEW_RETRY_INTERVAL_MS) / 1000.0));
        return;
    }

    // Get current camera from QML button state
    DVR_CAM_TYPE_E currentCamera = getCurrentCamera();
    LOGI(TAG, "Current camera: %d (FRONT=0, REAR=1)", currentCamera);

    // Check if camera is connected
    if (!m_cameraManager->isCameraConnected(currentCamera)) {
        LOGW(TAG, "Camera %d not connected, checking alternative", currentCamera);

        // Try alternative camera
        DVR_CAM_TYPE_E altCamera = (currentCamera == DVR_CAM_TYPE_FRONT) ?
                                    DVR_CAM_TYPE_REAR : DVR_CAM_TYPE_FRONT;

        if (m_cameraManager->isCameraConnected(altCamera)) {
            LOGI(TAG, "Alternative camera %d available, switching", altCamera);
            // Update button state
            m_isFrontCamera = (altCamera == DVR_CAM_TYPE_FRONT);
            emit frontCameraChanged(m_isFrontCamera);
            currentCamera = altCamera;
        } else {
            LOGW(TAG, "No camera connected, retry in %d ms", PREVIEW_RETRY_INTERVAL_MS);
            // Retry after interval
            QTimer::singleShot(PREVIEW_RETRY_INTERVAL_MS, this, [this, retryCount]() {
                tryAutoStartPreview(retryCount + 1);
            });
            return;
        }
    }

    // Check if camera is initialized
    if (!DVR_IsCameraInitialized(currentCamera)) {
        LOGI(TAG, "Camera %d not initialized, attempting DVR_InitSingleCameraByType", currentCamera);

        if (!DVR_InitSingleCameraByType(currentCamera)) {
            LOGW(TAG, "Camera initialization failed, retry in %d ms", PREVIEW_RETRY_INTERVAL_MS);
            // Retry after interval
            QTimer::singleShot(PREVIEW_RETRY_INTERVAL_MS, this, [this, retryCount]() {
                tryAutoStartPreview(retryCount + 1);
            });
            return;
        }

        LOGI(TAG, "Camera %d initialized successfully", currentCamera);
    }

    // Try to start preview
    LOGI(TAG, "Attempting to start preview for camera %d", currentCamera);
    if (m_previewManager->startPreview(currentCamera)) {
        LOGI(TAG, "Auto-preview started successfully after %d attempts", retryCount + 1);
        emit previewingChanged(true);
    } else {
        LOGW(TAG, "Preview start failed, retry %d of %d in %d ms",
             retryCount + 1, PREVIEW_MAX_RETRY_COUNT, PREVIEW_RETRY_INTERVAL_MS);

        // Retry after interval
        QTimer::singleShot(PREVIEW_RETRY_INTERVAL_MS, this, [this, retryCount]() {
            tryAutoStartPreview(retryCount + 1);
        });
    }
}

// Slots for manager signals
void DVRBackend::onPreviewStateChanged(DVR_CAM_TYPE_E camera, bool active)
{
    LOGI(TAG, "Preview state changed for camera %d: %s", camera, active ? "active" : "inactive");

    // Only emit signal if it's the current camera
    if (camera == getCurrentCamera()) {
        emit previewingChanged(active);
    }
}

void DVRBackend::onPreviewError(DVR_CAM_TYPE_E camera, const QString &error)
{
    LOGE(TAG, "Preview error for camera %d: %s", camera, error.toUtf8().constData());
    emit errorOccurred(error);
}

void DVRBackend::onCameraAvailabilityChanged(DVR_CAM_TYPE_E camera, bool available)
{
    LOGI(TAG, "Camera %d availability changed: %s", camera, available ? "available" : "unavailable");

    // TODO: Handle camera hot-plug
    // If current camera disconnected, may need to switch to other camera
}

// System event handlers (called by DVRQObjListener)
void DVRBackend::handleShowFront(int param1, int param2)
{
    LOGI(TAG, "handleShowFront called, param1=%d, param2=%d", param1, param2);

    // Phase 1: Just log
    // Phase 2+: Will resume preview/recording if they were active
}

void DVRBackend::handleHideFront(int param1, int param2)
{
    LOGI(TAG, "handleHideFront called, param1=%d, param2=%d", param1, param2);

    // Phase 1: Just log
    // Phase 2+: Will pause preview/recording to save resources
}

void DVRBackend::handleVideoFocusChanged(CCtlListener::E_AVOUT vOut, CCtlListener::E_VIDEOFOCUS focus)
{
    LOGI(TAG, "handleVideoFocusChanged, vOut=%d, focus=%d", vOut, focus);
    emit videoFocusChanged(vOut, focus);

    // Phase 1: Just log and emit signal
    // Phase 2+: Will handle preview pause/resume based on focus
    // FOCUS_LOSS: Pause preview, release video resources
    // FOCUS_GAIN: Resume preview
}

void DVRBackend::handleAudioFocusChanged(CCtlListener::E_AVOUT aOut, CCtlListener::E_AUDIOFOCUS focus)
{
    LOGI(TAG, "handleAudioFocusChanged, aOut=%d, focus=%d", aOut, focus);
    emit audioFocusChanged(aOut, focus);

    // Phase 1: Just log and emit signal
    // Phase 2+: Will handle audio mute/unmute based on focus
    // FOCUS_LOSS: Mute audio
    // FOCUS_GAIN: Unmute audio
}

// Navigation and Exit (Phase 1)
void DVRBackend::goHome()
{
    LOGI(TAG, "goHome: Home button clicked - switching to Cluster main screen");

    // Switch to Cluster application (main screen) like btapp does
    // Reference: btapp/bluetoothapplication.cpp goHome()
    GlobalBus::applyFor(GlobalBus::ACTION_RUN, CAPPBaseObj::APPID_CLUSTER, CAPPBaseObj::LEVEL_NORMAL);

    LOGI(TAG, "goHome: Sent ACTION_RUN to APPID_CLUSTER");
}

void DVRBackend::goExit()
{
    LOGI(TAG, "goExit: Exit button clicked - hiding DVR front UI");

    // Hide DVR front UI, app keeps running in background like btapp does
    // Reference: original dvrdemo.cpp goExit()
    GlobalBus::applyFor(GlobalBus::ACTION_HIDEFRONT, CAPPBaseObj::APPID_DVR, CAPPBaseObj::LEVEL_NORMAL);

    LOGI(TAG, "goExit: Sent ACTION_HIDEFRONT to APPID_DVR");
}

void DVRBackend::showFileList()
{
    LOGI(TAG, "========================================");
    LOGI(TAG, "showFileList: Switching to file list view");
    LOGI(TAG, "========================================");

    // NOTE: Preview continues running in background
    // There's no resource conflict between file list and preview
    // Preview will be stopped only when user actually clicks a file to play
    // This allows instant preview restoration if user returns without playing
    LOGI(TAG, "Preview continues running (no conflict with file list UI)");

    // Emit navigation signal to QML
    emit navigateToFileList();

    LOGI(TAG, "========================================");
}

void DVRBackend::hideFileList()
{
    LOGI(TAG, "========================================");
    LOGI(TAG, "hideFileList: Returning from file list to main view");
    LOGI(TAG, "========================================");

    // Emit navigation signal to QML first
    emit navigateBackFromFileList();

    // Check if preview needs to be restarted
    // Case 1: User never played video - preview still running, do nothing ?
    // Case 2: User played video and returned - preview stopped, need restart ?
    if (m_previewManager) {
        DVR_CAM_TYPE_E camera = getCurrentCamera();
        
        if (!m_previewManager->isPreviewActive(camera)) {
            LOGI(TAG, "Preview is not active, restarting after short delay");
            
            // Restart preview after short delay (gives QML time to hide file list)
            QTimer::singleShot(200, this, [this, camera]() {
                if (m_previewManager) {
                    if (!m_previewManager->startPreview(camera)) {
                        LOGE(TAG, "Failed to restart preview for camera %d", camera);
                        emit errorOccurred("Failed to restart preview");
                        return;
                    }
                    LOGI(TAG, "Preview restarted successfully after file list");
                }
            });
        } else {
            LOGI(TAG, "Preview already active, no restart needed (user didn't play video)");
        }
    }

    LOGI(TAG, "========================================");
}

// Recording event handlers (connected to RecordManager signals)
void DVRBackend::onRecordingStateChanged(DVR_CAM_TYPE_E camera, bool active)
{
    LOGI(TAG, "Recording state changed for camera %d: %s",
         camera, active ? "active" : "stopped");

    // Emit general recording state change signal for QML
    emit recordingChanged(isRecording());
}

void DVRBackend::onRecordingStarted(DVR_CAM_TYPE_E camera)
{
    LOGI(TAG, "Recording confirmed started for camera %d", camera);

    // Emit state change signal
    emit recordingChanged(true);
}

void DVRBackend::onRecordingStopped(DVR_CAM_TYPE_E camera)
{
    LOGI(TAG, "Recording stopped for camera %d", camera);

    // Check if any camera is still recording
    bool anyRecording = isRecording();
    emit recordingChanged(anyRecording);
}

void DVRBackend::onRecordingError(const QString &message)
{
    LOGE(TAG, "Recording error: %s", qPrintable(message));

    // Forward error to QML
    emit errorOccurred(message);
}

void DVRBackend::onStorageFullWarning()
{
    LOGW(TAG, "Storage full warning received");

    // Emit error to show toast notification
    emit errorOccurred("Storage full, recording stopped");
}

// DVR event handler (called from dvrEventCallback via global pointer)
void DVRBackend::handleDvrEvent(uint32_t event, uint32_t param1, uint32_t param2)
{
    // CRITICAL: This method is called from DVR library internal thread!
    // Must use Qt::QueuedConnection to dispatch events to managers in main thread

    LOGD(TAG, "[DVR_EVENT] handleDvrEvent called: event=0x%X, param1=%u, param2=%u",
         event, param1, param2);

    // Route events to appropriate managers using Qt::QueuedConnection
    // This ensures manager event handlers run in Qt main thread, not DVR thread

    switch (event) {
        // Recording events ??RecordManager
        case DVR_UI_MSG_REC_STARTED:
        case DVR_UI_MSG_REC_STOPED:
        case DVR_UI_MSG_START_REC_FAILED:
        case DVR_UI_MSG_STOP_REC_FAILED:
        case DVR_UI_MSG_SD_FULL:
            if (m_recordManager) {
                LOGD(TAG, "[DVR_EVENT] Routing recording event 0x%X to RecordManager (queued)", event);
                QMetaObject::invokeMethod(m_recordManager, "onDvrEvent",
                    Qt::QueuedConnection,
                    Q_ARG(uint, event),
                    Q_ARG(uint, param1),
                    Q_ARG(uint, param2));
            } else {
                LOGW(TAG, "[DVR_EVENT] RecordManager not available, cannot route event 0x%X", event);
            }
            break;

        // Preview events ??PreviewManager
        case DVR_UI_MSG_PREV_STARTED:
        case DVR_UI_MSG_PREV_STOPED:
        case DVR_UI_MSG_START_PREV_FAILED:
        case DVR_UI_MSG_STOP_PREV_FAILED:
        case DVR_UI_MSG_CAM_ERROR:
            if (m_previewManager) {
                LOGD(TAG, "[DVR_EVENT] Routing preview event 0x%X to PreviewManager (queued)", event);
                QMetaObject::invokeMethod(m_previewManager, "onDvrEvent",
                    Qt::QueuedConnection,
                    Q_ARG(uint, event),
                    Q_ARG(uint, param1),
                    Q_ARG(uint, param2));
            } else {
                LOGW(TAG, "[DVR_EVENT] PreviewManager not available, cannot route event 0x%X", event);
            }
            break;

        default:
            LOGD(TAG, "[DVR_EVENT] Unhandled event type: 0x%X", event);
            break;
    }
}

bool DVRBackend::waitForDeviceReady(int timeoutMs)
{
    const char* devicePath = "/dev/video0";
    int pollIntervalMs = 50;  // Poll every 50ms
    int elapsed = 0;

    LOGI(TAG, "[DEVICE_POLL] Starting device availability check: %s (timeout=%dms)",
         devicePath, timeoutMs);

    while (elapsed < timeoutMs) {
        // Try to open device in read-write non-blocking mode
        int fd = open(devicePath, O_RDWR | O_NONBLOCK);

        if (fd >= 0) {
            // Device is available, close and return success
            close(fd);
            LOGI(TAG, "[DEVICE_POLL] [OK] Device %s available after %dms",
                 devicePath, elapsed);
            return true;
        }

        // Device busy or error, check errno
        int err = errno;
        if (err == EBUSY) {
            // Device is busy (still held by preview), continue polling
            LOGD(TAG, "[DEVICE_POLL] Device %s busy (EBUSY), retrying... (elapsed=%dms)",
                 devicePath, elapsed);
        } else if (err == ENOENT) {
            // Device doesn't exist - this is a fatal error
            LOGE(TAG, "[DEVICE_POLL] [ERROR] Device %s does not exist (ENOENT)",
                 devicePath);
            return false;
        } else {
            // Other error (EACCES, etc.)
            LOGW(TAG, "[DEVICE_POLL] Device %s open failed with errno=%d, retrying...",
                 devicePath, err);
        }

        // Wait before next poll
        QThread::msleep(pollIntervalMs);
        elapsed += pollIntervalMs;
    }

    // Timeout reached
    LOGE(TAG, "[DEVICE_POLL] [TIMEOUT] Device %s still not available after %dms",
         devicePath, elapsed);
    return false;
}


