/*
 * Copyright (c) 2025 AutoChips Inc.
 *
 * PreviewManager - Preview stream and surface management implementation
 */

#include "previewmanager.h"
#include "dvrlog.h"
#include <QThread>

static const char* TAG = "PreviewManager";

PreviewManager::PreviewManager(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
    , m_sharedSurface(nullptr)
    , m_getCurrentCamera(nullptr)
    , m_surfaceWidth(0)
    , m_surfaceHeight(0)
{
    LOGI(TAG, "Created");
}

PreviewManager::~PreviewManager()
{
    LOGI(TAG, "Destroying");
    shutdown();
}

bool PreviewManager::initialize(int screenWidth, int screenHeight)
{
    if (m_initialized) {
        LOGW(TAG, "Already initialized");
        return true;
    }

    LOGI(TAG, "Initializing with screen resolution: %dx%d...", screenWidth, screenHeight);

    // Store screen dimensions
    m_surfaceWidth = screenWidth;
    m_surfaceHeight = screenHeight;

    // Create shared surface ONCE (original app pattern)
    // This surface will be reused by both cameras
    // CRITICAL: Surface size MUST match screen resolution to avoid scaling issues
    m_sharedSurface = atc_createsurface(
        ATCSURF_TYPE_DEFAULT,        // Surface type
        m_surfaceWidth,              // Width - matches screen
        m_surfaceHeight,             // Height - matches screen
        ATC_PIX_FMT_NV12M_PRIVATE1  // Format
    );

    if (!m_sharedSurface) {
        LOGE(TAG, "Failed to create shared surface");
        return false;
    }

    LOGI(TAG, "Shared surface created: %p, resolution: %dx%d",
         m_sharedSurface, m_surfaceWidth, m_surfaceHeight);

    // Set preview surface Z-order to bottom layer to prevent covering UI
    IAtcSurface_setLayerZOrder(m_sharedSurface, 0);
    LOGI(TAG, "Surface Z-order set to 0 (bottom layer)");

    // Set video surface to fullscreen display (like original app)
    IAtcSurface_setWindow(m_sharedSurface, 0, 0, m_surfaceWidth, m_surfaceHeight);
    LOGI(TAG, "Surface window set to fullscreen: 0,0 %dx%d", m_surfaceWidth, m_surfaceHeight);

    m_initialized = true;
    LOGI(TAG, "Initialization complete");

    return true;
}

void PreviewManager::shutdown()
{
    if (!m_initialized) {
        return;
    }

    LOGI(TAG, "Shutting down...");

    // Stop all active previews
    if (m_frontPreview.isActive) {
        stopPreview(DVR_CAM_TYPE_FRONT);
    }
    if (m_rearPreview.isActive) {
        stopPreview(DVR_CAM_TYPE_REAR);
    }

    // Release shared surface (only once at shutdown)
    if (m_sharedSurface) {
        LOGI(TAG, "Releasing shared surface: %p", m_sharedSurface);
        IAtcSurface_release(m_sharedSurface);
        m_sharedSurface = nullptr;
    }

    m_initialized = false;
    LOGI(TAG, "Shutdown complete");
}

bool PreviewManager::startPreview(DVR_CAM_TYPE_E camera)
{
    if (!m_initialized) {
        LOGE(TAG, "Not initialized");
        emit previewError(camera, "PreviewManager not initialized");
        return false;
    }

    CameraPreviewState *state = getPreviewState(camera);
    if (!state) {
        LOGE(TAG, "Invalid camera type: %d", camera);
        emit previewError(camera, "Invalid camera type");
        return false;
    }

    if (state->isActive) {
        LOGW(TAG, "Preview already active for camera %d", camera);
        return true;
    }

    LOGI(TAG, "Starting preview for camera %d", camera);

    // Step 1: Recreate surface if it was released (e.g., after file list view)
    if (!m_sharedSurface) {
        LOGI(TAG, "Surface not available, recreating...");

        m_sharedSurface = atc_createsurface(
            ATCSURF_TYPE_DEFAULT,
            m_surfaceWidth,
            m_surfaceHeight,
            ATC_PIX_FMT_NV12M_PRIVATE1
        );

        if (!m_sharedSurface) {
            LOGE(TAG, "Failed to create surface! Resolution: %dx%d",
                 m_surfaceWidth, m_surfaceHeight);
            emit previewError(camera, "Failed to create preview surface");
            return false;
        }

        LOGI(TAG, "Surface created: %p, resolution: %dx%d",
             m_sharedSurface, m_surfaceWidth, m_surfaceHeight);

        // Set Z-order and window
        IAtcSurface_setLayerZOrder(m_sharedSurface, 0);
        IAtcSurface_setWindow(m_sharedSurface, 0, 0, m_surfaceWidth, m_surfaceHeight);
        LOGI(TAG, "Surface configured: Z-order=0, fullscreen");
    }

    // Step 2: Ensure camera is initialized (on-demand initialization)
    if (!DVR_IsCameraInitialized(camera)) {
        LOGI(TAG, "Camera %d not initialized, initializing now", camera);
        if (!DVR_InitSingleCameraByType(camera)) {
            LOGE(TAG, "Failed to initialize camera %d", camera);
            emit previewError(camera, "Failed to initialize camera");
            return false;
        }
        // Give device time to stabilize
        QThread::msleep(200);
    }

    // Step 3: Bind surface to camera
    if (!DVR_SetPreviewSurface(camera, m_sharedSurface)) {
        LOGE(TAG, "DVR_SetPreviewSurface failed for camera %d", camera);
        emit previewError(camera, "Failed to set preview surface");
        return false;
    }

    // Step 4: Set video info (optional)
    DVR_VIDEO_INFO_T vidInfo;
    vidInfo.u4Width = m_surfaceWidth;
    vidInfo.u4Height = m_surfaceHeight;
    vidInfo.u4FrameRate = 30;

    if (!DVR_SetVideoInfoByCamera(camera, &vidInfo)) {
        LOGW(TAG, "DVR_SetVideoInfoByCamera failed, using defaults");
    }

    // Step 5: Start preview
    if (!DVR_StartPreviewByCamera(camera)) {
        LOGE(TAG, "DVR_StartPreviewByCamera failed for camera %d", camera);
        // Clear surface reference
        DVR_SetPreviewSurface(camera, NULL);
        emit previewError(camera, "Failed to start preview");
        return false;
    }

    state->isActive = true;
    emit previewStateChanged(camera, true);
    LOGI(TAG, "Preview started successfully for camera %d", camera);

    return true;
}

bool PreviewManager::stopPreview(DVR_CAM_TYPE_E camera, bool releaseSurface)
{
    CameraPreviewState *state = getPreviewState(camera);
    if (!state || !state->isActive) {
        LOGW(TAG, "Preview not active for camera %d", camera);
        
        // Even if not active, still release surface if requested
        if (releaseSurface && m_sharedSurface) {
            LOGI(TAG, "Releasing surface as requested (preview already stopped)");
            IAtcSurface_release(m_sharedSurface);
            m_sharedSurface = nullptr;
            LOGI(TAG, "Surface released, /dev/video0 freed");
        }
        
        return true;  // Already stopped, not an error
    }

    LOGI(TAG, "Stopping preview for camera %d (releaseSurface=%s)", 
         camera, releaseSurface ? "true" : "false");

    // Step 1: Stop preview stream
    DVR_StopPreviewByCamera(camera);

    // Step 2: Unbind surface from camera
    DVR_SetPreviewSurface(camera, NULL);

    state->isActive = false;
    emit previewStateChanged(camera, false);

    // Step 3: Release surface if requested (for file list view)
    if (releaseSurface && m_sharedSurface) {
        LOGI(TAG, "Releasing surface to free /dev/video0 for video player");
        IAtcSurface_release(m_sharedSurface);
        m_sharedSurface = nullptr;
        LOGI(TAG, "Surface released, /dev/video0 freed");
    }

    LOGI(TAG, "Preview stopped for camera %d", camera);

    return true;
}

bool PreviewManager::isPreviewActive(DVR_CAM_TYPE_E camera) const
{
    const CameraPreviewState *state = getPreviewState(camera);
    return state ? state->isActive : false;
}

PreviewManager::CameraPreviewState* PreviewManager::getPreviewState(DVR_CAM_TYPE_E camera)
{
    switch (camera) {
    case DVR_CAM_TYPE_FRONT:
        return &m_frontPreview;
    case DVR_CAM_TYPE_REAR:
        return &m_rearPreview;
    default:
        return nullptr;
    }
}

const PreviewManager::CameraPreviewState* PreviewManager::getPreviewState(DVR_CAM_TYPE_E camera) const
{
    switch (camera) {
    case DVR_CAM_TYPE_FRONT:
        return &m_frontPreview;
    case DVR_CAM_TYPE_REAR:
        return &m_rearPreview;
    default:
        return nullptr;
    }
}

void PreviewManager::setCurrentCameraGetter(std::function<DVR_CAM_TYPE_E()> callback)
{
    m_getCurrentCamera = callback;
    LOGI(TAG, "Current camera getter callback registered: %s",
         callback ? "valid" : "null");
}

void PreviewManager::onDvrEvent(uint eventType, uint param1, uint param2)
{
    // This method runs in Qt main thread (via Qt::QueuedConnection dispatch)
    // Safe to access member variables and emit signals

    // Lambda helper: Get current camera from callback or fallback to param1
    // NOTE: DVR library may not always provide camera type in param1, especially
    // for error events. We prefer using the current preview camera from DVRBackend.
    auto getCurrentCamera = [this, param1]() -> DVR_CAM_TYPE_E {
        if (m_getCurrentCamera) {
            return m_getCurrentCamera();
        } else {
            // Fallback: Try to use param1 if callback not set
            return static_cast<DVR_CAM_TYPE_E>(param1);
        }
    };

    DVR_CAM_TYPE_E camera = getCurrentCamera();
    const char* cameraName = (camera == DVR_CAM_TYPE_FRONT) ? "Front" : "Rear";

    switch (eventType) {
        case DVR_UI_MSG_PREV_STARTED:
            LOGI(TAG, "[DVR_EVENT] Preview started confirmed: camera %d (%s)",
                 camera, cameraName);
            // Update state if not already set
            {
                CameraPreviewState* state = getPreviewState(camera);
                if (state && !state->isActive) {
                    state->isActive = true;
                    emit previewStateChanged(camera, true);
                }
            }
            break;

        case DVR_UI_MSG_PREV_STOPED:
            LOGI(TAG, "[DVR_EVENT] Preview stopped confirmed: camera %d (%s)",
                 camera, cameraName);
            // Update state if not already cleared
            {
                CameraPreviewState* state = getPreviewState(camera);
                if (state && state->isActive) {
                    state->isActive = false;
                    emit previewStateChanged(camera, false);
                }
            }
            break;

        case DVR_UI_MSG_START_PREV_FAILED:
            LOGE(TAG, "[DVR_EVENT] Preview start failed: camera %d (%s), error=0x%X",
                 camera, cameraName, param2);
            {
                QString errorMsg = QString("Failed to start preview for %1 camera: error 0x%2")
                    .arg(cameraName)
                    .arg(param2, 0, 16);
                emit previewError(camera, errorMsg);
            }
            break;

        case DVR_UI_MSG_STOP_PREV_FAILED:
            LOGE(TAG, "[DVR_EVENT] Preview stop failed: camera %d (%s), error=0x%X",
                 camera, cameraName, param2);
            {
                QString errorMsg = QString("Failed to stop preview for %1 camera: error 0x%2")
                    .arg(cameraName)
                    .arg(param2, 0, 16);
                emit previewError(camera, errorMsg);
            }
            break;

        case DVR_UI_MSG_CAM_ERROR:
            LOGE(TAG, "[DVR_EVENT] Camera error: camera %d (%s), error=0x%X",
                 camera, cameraName, param2);
            {
                // Auto-stop preview on camera error
                CameraPreviewState* state = getPreviewState(camera);
                if (state && state->isActive) {
                    LOGW(TAG, "Auto-stopping preview due to camera error");
                    stopPreview(camera);
                }

                QString errorMsg = QString("%1 camera hardware error: 0x%2")
                    .arg(cameraName)
                    .arg(param2, 0, 16);
                emit previewError(camera, errorMsg);
            }
            break;

        default:
            LOGD(TAG, "[DVR_EVENT] Unhandled preview event: 0x%X for camera %d",
                 eventType, camera);
            break;
    }
}
