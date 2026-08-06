/*
 * Copyright (c) 2025 AutoChips Inc.
 *
 * RecordManager - Implementation
 * Handles per-camera recording control with DVR library integration
 */

#include "recordmanager.h"
#include "dvrlog.h"
#include <QDir>
#include <QFileInfo>
#include <QStorageInfo>

static const char* TAG = "RecordManager";

// System-defined SD card mount point (fixed)
const QString RecordManager::SD_CARD_MOUNT_POINT = "/media/ext_sdcard2";

// Application recording subdirectory name
const QString RecordManager::RECORDING_SUBDIR = "dvr";

RecordManager::RecordManager(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
    , m_isFrontRecordingActive(false)
    , m_isRearRecordingActive(false)
    , m_getCurrentCamera(nullptr)
    , m_recordPath(SD_CARD_MOUNT_POINT + "/" + RECORDING_SUBDIR)  // "/media/ext_sdcard2/dvr"
    , m_recordDuration(180)  // Default: 3 minutes
    , m_sdCardReady(false)
{
    LOGI(TAG, "RecordManager created");
    LOGI(TAG, "SD card mount point: %s", qPrintable(SD_CARD_MOUNT_POINT));
    LOGI(TAG, "Recording subdirectory: %s", qPrintable(RECORDING_SUBDIR));
    LOGI(TAG, "Full recording path: %s", qPrintable(m_recordPath));
}

void RecordManager::setCurrentCameraGetter(std::function<DVR_CAM_TYPE_E()> callback)
{
    m_getCurrentCamera = callback;
    LOGI(TAG, "Current camera getter callback registered");
}

RecordManager::~RecordManager()
{
    LOGI(TAG, "RecordManager destructor called");

    // Stop all active recordings before destruction
    if (m_isFrontRecordingActive) {
        LOGW(TAG, "Front camera still recording in destructor, stopping...");
        stopRecording(DVR_CAM_TYPE_FRONT);
    }

    if (m_isRearRecordingActive) {
        LOGW(TAG, "Rear camera still recording in destructor, stopping...");
        stopRecording(DVR_CAM_TYPE_REAR);
    }

    LOGI(TAG, "RecordManager destroyed");
}

bool RecordManager::initialize()
{
    if (m_initialized) {
        LOGW(TAG, "RecordManager already initialized");
        return true;
    }

    LOGI(TAG, "Initializing RecordManager...");

    // Step 1: Check if SD card is properly mounted
    if (!checkSdCardMounted()) {
        LOGW(TAG, "SD card not mounted or on tmpfs, entering STANDBY mode");
        LOGW(TAG, "Recording will be unavailable until SD card is properly inserted");
        m_initialized = true;  // Allow app to start
        m_sdCardReady = false;
        return true;  // Return true to allow startup, but mark SD as not ready
    }

    m_sdCardReady = true;
    LOGI(TAG, "SD card properly mounted, proceeding with full initialization");

    // Step 2: Validate and create recording directory
    QDir recordDir(m_recordPath);
    if (!recordDir.exists()) {
        LOGI(TAG, "Recording directory does not exist, creating: %s",
             qPrintable(m_recordPath));

        if (!recordDir.mkpath(m_recordPath)) {
            LOGE(TAG, "Failed to create recording directory: %s",
                 qPrintable(m_recordPath));
            m_sdCardReady = false;
            return false;
        }
    }

    // Step 3: Validate storage is writable
    QFileInfo pathInfo(m_recordPath);
    if (!pathInfo.isWritable()) {
        LOGE(TAG, "Recording path is not writable: %s", qPrintable(m_recordPath));
        m_sdCardReady = false;
        return false;
    }

    // Step 4: Log storage status (for informational purposes only)
    QStorageInfo storage(m_recordPath);
    if (storage.isValid()) {
        qint64 availableBytes = storage.bytesAvailable();
        LOGI(TAG, "Storage status: %lld MB available",
             availableBytes / (1024 * 1024));
    }

    // NOTE: Storage capacity checking is handled by DVR Library at runtime:
    // - Circular recording triggers at 100MB threshold (auto-delete old files)
    // - DVR_UI_MSG_SD_FULL notification sent at 50MB threshold
    // App layer should not duplicate this logic to avoid conflicts
    // See: src/lib/dvr_filemgr.cpp and src/lib/dvr_recorder.h

    // Step 5: Configure DVR library recording settings
    LOGI(TAG, "Configuring DVR recording settings:");
    LOGI(TAG, "  - Path: %s", qPrintable(m_recordPath));
    LOGI(TAG, "  - Duration: %u seconds", m_recordDuration);

    // Set recording path (DVR library function)
    // Note: DVR_SetRecordPath expects TCHAR* (non-const), so we need a mutable buffer
    QByteArray pathBytes = m_recordPath.toUtf8();
    if (!DVR_SetRecordPath(pathBytes.data())) {
        LOGE(TAG, "Failed to set recording path in DVR library");
        return false;
    }

    // Set recording duration (DVR library function)
    if (!DVR_SetRecordDuration(m_recordDuration)) {
        LOGE(TAG, "Failed to set recording duration in DVR library");
        return false;
    }

    m_initialized = true;
    LOGI(TAG, "RecordManager initialized successfully");
    return true;
}

void RecordManager::shutdown()
{
    if (!m_initialized) {
        LOGD(TAG, "RecordManager not initialized, nothing to shutdown");
        return;
    }

    LOGI(TAG, "Shutting down RecordManager...");

    // Stop all active recordings
    if (m_isFrontRecordingActive) {
        LOGI(TAG, "Stopping front camera recording...");
        stopRecording(DVR_CAM_TYPE_FRONT);
    }

    if (m_isRearRecordingActive) {
        LOGI(TAG, "Stopping rear camera recording...");
        stopRecording(DVR_CAM_TYPE_REAR);
    }

    m_initialized = false;
    LOGI(TAG, "RecordManager shutdown complete");
}

bool RecordManager::startRecording(DVR_CAM_TYPE_E camera)
{
    QString cameraName = getCameraName(camera);

    LOGI(TAG, "[DEBUG_RECORDING] startRecording() called for %s camera",
         qPrintable(cameraName));

    // Step 1: Validate manager is initialized
    if (!m_initialized) {
        LOGE(TAG, "RecordManager not initialized, cannot start recording");
        emit recordingError("Recording system not initialized");
        return false;
    }

    // Step 2: Check SD card readiness (on-demand detection)
    if (!m_sdCardReady) {
        LOGW(TAG, "SD card not ready, attempting re-initialization...");
        
        // Try to re-initialize (check if SD card is now mounted)
        if (!checkSdCardMounted()) {
            LOGE(TAG, "SD card still not mounted, cannot start recording");
            emit recordingError("SD card not inserted or not mounted");
            return false;
        }
        
        // SD card is now available, perform full initialization
        LOGI(TAG, "SD card now available, performing full initialization");
        m_initialized = false;  // Reset to force re-init
        if (!initialize()) {
            LOGE(TAG, "Re-initialization failed");
            emit recordingError("Failed to initialize recording system");
            return false;
        }
        
        LOGI(TAG, "Re-initialization successful, SD card ready");
    }

    // Step 3: Check if already recording
    if (isRecordingActive(camera)) {
        LOGW(TAG, "%s camera is already recording", qPrintable(cameraName));
        return true;  // Not an error, just already running
    }

    // Step 4: Re-check SD card mount status (in case card was removed after initialization)
    if (!checkSdCardMounted()) {
        LOGE(TAG, "SD card no longer mounted, cannot start recording");
        m_sdCardReady = false;
        emit recordingError("SD card removed, please re-insert");
        return false;
    }

    // Step 5: Call DVR library to start recording
    // NOTE: Storage capacity is checked by DVR Library (100MB/50MB thresholds)
    LOGI(TAG, "Calling DVR_StartRecordByCamera(%s)...", qPrintable(cameraName));

    bool success = DVR_StartRecordByCamera(camera);

    if (success) {
        // Update internal state AFTER successful API call
        if (camera == DVR_CAM_TYPE_FRONT) {
            m_isFrontRecordingActive = true;
        } else {
            m_isRearRecordingActive = true;
        }

        LOGI(TAG, "[OK] %s camera recording started successfully",
             qPrintable(cameraName));

        // Emit state change signal immediately
        emit recordingStateChanged(camera, true);

        // Note: recordingStarted() signal will be emitted when we receive
        // DVR_UI_MSG_REC_STARTED event from DVR library

    } else {
        LOGE(TAG, "[ERROR] Failed to start %s camera recording",
             qPrintable(cameraName));
        emit recordingError(QString("Failed to start %1 camera recording").arg(cameraName));
    }

    return success;
}

bool RecordManager::stopRecording(DVR_CAM_TYPE_E camera)
{
    QString cameraName = getCameraName(camera);

    LOGI(TAG, "[DEBUG_RECORDING] stopRecording() called for %s camera",
         qPrintable(cameraName));

    // Step 1: Check if actually recording
    if (!isRecordingActive(camera)) {
        LOGD(TAG, "%s camera is not recording, nothing to stop",
             qPrintable(cameraName));
        return true;  // Not an error
    }

    // Step 2: Call DVR library to stop recording
    LOGI(TAG, "Calling DVR_StopRecordByCamera(%s)...", qPrintable(cameraName));

    bool success = DVR_StopRecordByCamera(camera);

    if (success) {
        // Update internal state AFTER successful API call
        if (camera == DVR_CAM_TYPE_FRONT) {
            m_isFrontRecordingActive = false;
        } else {
            m_isRearRecordingActive = false;
        }

        LOGI(TAG, "[OK] %s camera recording stopped successfully",
             qPrintable(cameraName));

        // Emit state change signal
        emit recordingStateChanged(camera, false);
        emit recordingStopped(camera);

    } else {
        LOGE(TAG, "[ERROR] Failed to stop %s camera recording",
             qPrintable(cameraName));
        emit recordingError(QString("Failed to stop %1 camera recording").arg(cameraName));
    }

    return success;
}

bool RecordManager::isRecordingActive(DVR_CAM_TYPE_E camera) const
{
    if (camera == DVR_CAM_TYPE_FRONT) {
        return m_isFrontRecordingActive;
    } else {
        return m_isRearRecordingActive;
    }
}

bool RecordManager::isAnyRecordingActive() const
{
    return m_isFrontRecordingActive || m_isRearRecordingActive;
}

bool RecordManager::setRecordDuration(uint32_t durationSeconds)
{
    LOGI(TAG, "Setting record duration to %u seconds", durationSeconds);

    // Validate duration (3/5/10 minutes = 180/300/600 seconds)
    if (durationSeconds != 180 && durationSeconds != 300 && durationSeconds != 600) {
        LOGW(TAG, "Invalid duration: %u seconds. Valid options: 180, 300, 600",
             durationSeconds);
        return false;
    }

    m_recordDuration = durationSeconds;

    // Update DVR library setting
    if (m_initialized) {
        if (!DVR_SetRecordDuration(m_recordDuration)) {
            LOGE(TAG, "Failed to update recording duration in DVR library");
            return false;
        }
    }

    LOGI(TAG, "Record duration set to %u seconds successfully", durationSeconds);
    return true;
}

bool RecordManager::setRecordPath(const QString &path)
{
    LOGI(TAG, "Setting record path to: %s", qPrintable(path));

    // Validate path exists and is writable
    QFileInfo pathInfo(path);
    if (!pathInfo.exists()) {
        LOGE(TAG, "Path does not exist: %s", qPrintable(path));
        return false;
    }

    if (!pathInfo.isWritable()) {
        LOGE(TAG, "Path is not writable: %s", qPrintable(path));
        return false;
    }

    m_recordPath = path;

    // Update DVR library setting
    if (m_initialized) {
        // Note: DVR_SetRecordPath expects TCHAR* (non-const), so we need a mutable buffer
        QByteArray pathBytes = m_recordPath.toUtf8();
        if (!DVR_SetRecordPath(pathBytes.data())) {
            LOGE(TAG, "Failed to update recording path in DVR library");
            return false;
        }
    }

    LOGI(TAG, "Record path set successfully");
    return true;
}

QString RecordManager::getRecordPath() const
{
    // Auto-detect SD card if not ready yet (lazy detection)
    if (!m_sdCardReady) {
        LOGD(TAG, "[RECORD_PATH] SD card not ready, attempting auto-detection...");
        const_cast<RecordManager*>(this)->checkSdCardMounted();
    }
    
    // Return empty string if SD card is still not ready after detection
    // This prevents FileManager from accessing tmpfs paths
    if (!m_sdCardReady) {
        LOGD(TAG, "[RECORD_PATH] SD card not ready after detection, returning empty path");
        return QString();
    }
    
    // Return the actual path where files are stored
    // DVR library appends "normal/" subdirectory (lowercase)
    QString path = m_recordPath;
    
    // Ensure path ends with separator
    if (!path.endsWith('/')) {
        path += '/';
    }
    
    // DVR library uses normal/ subdirectory for normal recordings
    // See: dvr_recorder.cpp line 1401: wcscat(pCameraState->szNmlPath, TEXT("normal/"));
    QString normalPath = path + "normal/";
    QDir normalDir(normalPath);
    if (normalDir.exists()) {
        LOGD(TAG, "[RECORD_PATH] Using normal subdirectory: %s", qPrintable(normalPath));
        return normalPath;
    }
    
    LOGD(TAG, "[RECORD_PATH] Using base path: %s", qPrintable(path));
    return path;
}

void RecordManager::onDvrEvent(uint eventType, uint param1, uint param2)
{
    // THREAD SAFETY: This method runs in Qt main thread (via Qt::QueuedConnection)
    // It is invoked via QMetaObject::invokeMethod from DVRBackend::handleDvrEvent()
    // Safe to access member variables and emit signals without additional locking

    LOGD(TAG, "[DVR_EVENT] onDvrEvent: type=0x%X, param1=%u, param2=%u",
         eventType, param1, param2);

    // Helper: Get current preview camera from DVRBackend
    // DVR library events don't specify which camera, so we use current preview camera
    auto getCurrentCamera = [this]() -> DVR_CAM_TYPE_E {
        if (m_getCurrentCamera) {
            return m_getCurrentCamera();
        }
        // Fallback: default to front camera if callback not set
        LOGW(TAG, "[DVR_EVENT] Current camera getter not set, defaulting to FRONT");
        return DVR_CAM_TYPE_FRONT;
    };

    switch (eventType) {
        case DVR_UI_MSG_REC_STARTED:
        {
            // Recording confirmed started by DVR library
            // Note: DVR library doesn't pass camera type in param1, use current preview camera
            DVR_CAM_TYPE_E camera = getCurrentCamera();

            LOGI(TAG, "[OK] Recording confirmed started for %s camera (using current preview camera)",
                 qPrintable(getCameraName(camera)));

            // Emit confirmation signal
            emit recordingStarted(camera);
            break;
        }

        case DVR_UI_MSG_REC_STOPED:
        {
            // Recording stopped (normal completion or user request)
            // Note: DVR library doesn't pass camera type in param1, use current preview camera
            DVR_CAM_TYPE_E camera = getCurrentCamera();

            LOGI(TAG, "[OK] Recording stopped for %s camera (using current preview camera)",
                 qPrintable(getCameraName(camera)));

            // State should already be updated in stopRecording(), but ensure consistency
            if (camera == DVR_CAM_TYPE_FRONT) {
                m_isFrontRecordingActive = false;
            } else {
                m_isRearRecordingActive = false;
            }

            emit recordingStateChanged(camera, false);
            emit recordingStopped(camera);
            break;
        }

        case DVR_UI_MSG_START_REC_FAILED:
        {
            // Recording start failed
            // Note: DVR library doesn't specify which camera failed, use current preview camera
            DVR_CAM_TYPE_E camera = getCurrentCamera();

            LOGE(TAG, "[ERROR] Recording start failed for %s camera (using current preview camera)",
                 qPrintable(getCameraName(camera)));

            // Rollback state for the camera that failed
            if (camera == DVR_CAM_TYPE_FRONT) {
                m_isFrontRecordingActive = false;
            } else {
                m_isRearRecordingActive = false;
            }

            emit recordingError(QString("Failed to start %1 camera recording").arg(getCameraName(camera)));
            break;
        }

        case DVR_UI_MSG_SD_FULL:
        {
            // Storage full - auto-stop all recordings
            LOGE(TAG, "[WARNING] Storage full, stopping all recordings");

            emit storageFullWarning();
            emit recordingError("Storage full, recording stopped");

            // Auto-stop both cameras if recording
            if (m_isFrontRecordingActive) {
                stopRecording(DVR_CAM_TYPE_FRONT);
            }
            if (m_isRearRecordingActive) {
                stopRecording(DVR_CAM_TYPE_REAR);
            }
            break;
        }

        case DVR_UI_MSG_CAM_ERROR:
        {
            // Camera error during recording
            // Note: DVR library may not pass camera type reliably, use current preview camera
            DVR_CAM_TYPE_E camera = getCurrentCamera();

            LOGE(TAG, "[ERROR] Camera error for %s camera during recording (using current preview camera)",
                 qPrintable(getCameraName(camera)));

            // Update state for the affected camera
            if (camera == DVR_CAM_TYPE_FRONT) {
                m_isFrontRecordingActive = false;
            } else {
                m_isRearRecordingActive = false;
            }

            emit recordingStateChanged(camera, false);
            emit recordingError(QString("%1 camera error during recording")
                               .arg(getCameraName(camera)));
            break;
        }

        default:
            // Ignore other events
            break;
    }
}

QString RecordManager::getCameraName(DVR_CAM_TYPE_E camera) const
{
    return (camera == DVR_CAM_TYPE_FRONT) ? "Front" : "Rear";
}

bool RecordManager::validateStorage()
{
    // Check storage availability only (no capacity percentage check)
    // Capacity management is handled by DVR Library with absolute thresholds (100MB/50MB)
    QStorageInfo storage(m_recordPath);

    if (!storage.isValid()) {
        LOGE(TAG, "Storage is not valid");
        return false;
    }

    if (!storage.isReady()) {
        LOGE(TAG, "Storage is not ready");
        return false;
    }

    qint64 totalBytes = storage.bytesTotal();

    if (totalBytes <= 0) {
        LOGE(TAG, "Invalid storage size");
        return false;
    }

    // Log available space for debugging (no threshold check)
    qint64 availableBytes = storage.bytesAvailable();
    LOGD(TAG, "Storage check: %lld MB available (capacity checking delegated to DVR Library)",
         availableBytes / (1024 * 1024));

    return true;
}

bool RecordManager::checkSdCardMounted()
{
    LOGI(TAG, "Checking SD card mount status at: %s", qPrintable(SD_CARD_MOUNT_POINT));
    
    // Check mount point directly (not the full recording path which may not exist yet)
    QStorageInfo storage(SD_CARD_MOUNT_POINT);

    if (!storage.isValid()) {
        LOGW(TAG, "Storage info invalid for mount point %s, SD card likely not mounted", 
             qPrintable(SD_CARD_MOUNT_POINT));
        return false;
    }

    QString fsType = storage.fileSystemType();
    LOGI(TAG, "Filesystem type at %s: %s", qPrintable(SD_CARD_MOUNT_POINT), qPrintable(fsType));

    // Check if filesystem is tmpfs (memory filesystem, not real SD card)
    if (fsType == "tmpfs") {
        LOGW(TAG, "Path is on tmpfs (memory filesystem), not a real SD card");
        return false;
    }

    // Check for typical SD card filesystems
    if (fsType == "vfat" || fsType == "exfat" || fsType == "ext4") {
        LOGI(TAG, "[OK] Valid SD card filesystem detected: %s", qPrintable(fsType));
        return true;
    }

    // Unknown filesystem type, log warning but allow (might be valid)
    LOGW(TAG, "Unknown filesystem type: %s (allowing, but may not be SD card)", qPrintable(fsType));
    return true;
}
