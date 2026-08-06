/*
 * Copyright (c) 2025 AutoChips Inc.
 *
 * RecordManager - Recording operations management
 * Handles per-camera recording control and DVR event monitoring
 */

#ifndef RECORDMANAGER_H
#define RECORDMANAGER_H

#include <QObject>
#include <QString>
#include <functional>
#include "dvr.h"  // For DVR_CAM_TYPE_E and DVR APIs

/**
 * @brief RecordManager - Manages video recording operations
 *
 * Responsibilities:
 * - Start/stop recording for specific camera via DVR_StartRecordByCamera()
 * - Track recording state per camera (independent front/rear)
 * - Monitor DVR events (REC_STARTED, REC_STOPPED, errors)
 * - Handle storage capacity management
 * - Emit signals for UI updates
 *
 * Design Pattern:
 * - Per-camera recording control (no dual recording API)
 * - Thread-safe event handling via onDvrEvent() callback
 * - State synchronization between DVR library and Qt layer
 * - Default settings: 3-minute files, 180-second duration
 *
 * Thread Safety:
 * - All public methods run in Qt main thread
 * - onDvrEvent() called from DVR library thread (via GlobalBus)
 * - DVR library handles internal thread synchronization
 */
class RecordManager : public QObject
{
    Q_OBJECT

public:
    explicit RecordManager(QObject *parent = nullptr);

    /**
     * @brief Set callback to get current camera from DVRBackend
     * This is used when DVR library events don't specify which camera
     * @param callback Function that returns current preview camera
     */
    void setCurrentCameraGetter(std::function<DVR_CAM_TYPE_E()> callback);
    ~RecordManager();

    /**
     * @brief Initialize recording manager
     * - Sets default recording path: /media/ext_sdcard2/dvr
     * - Configures default duration: 180 seconds (3 minutes)
     * - Sets capacity thresholds: 80% cleanup, 95% emergency stop
     * @return true if initialization succeeds
     */
    bool initialize();

    /**
     * @brief Shutdown recording manager
     * - Stops all active recordings
     * - Resets recording state
     */
    void shutdown();

    /**
     * @brief Start recording for specific camera
     * - Validates camera and storage state
     * - Calls DVR_StartRecordByCamera()
     * - Updates internal state on success
     * - Emits recordingStateChanged() signal
     * - Wait for DVR_UI_MSG_REC_STARTED callback for confirmation
     * @param camera Camera type (FRONT or REAR)
     * @return true if API call succeeds (actual start confirmed by event)
     */
    bool startRecording(DVR_CAM_TYPE_E camera);

    /**
     * @brief Stop recording for specific camera
     * - Calls DVR_StopRecordByCamera()
     * - Updates internal state on success
     * - Emits recordingStateChanged() signal
     * @param camera Camera type (FRONT or REAR)
     * @return true if stop succeeds
     */
    bool stopRecording(DVR_CAM_TYPE_E camera);

    /**
     * @brief Check if recording is active for specific camera
     * @param camera Camera type (FRONT or REAR)
     * @return true if recording is active
     */
    bool isRecordingActive(DVR_CAM_TYPE_E camera) const;

    /**
     * @brief Check if ANY camera is recording
     * @return true if front OR rear camera is recording
     */
    bool isAnyRecordingActive() const;

    /**
     * @brief Set recording file duration
     * @param durationSeconds Duration per file in seconds (180/300/600)
     * @return true if setting succeeds
     */
    bool setRecordDuration(uint32_t durationSeconds);

    /**
     * @brief Set recording path
     * @param path Storage path (e.g., "/media/ext_sdcard2/dvr")
     * @return true if path is valid and writable
     */
    bool setRecordPath(const QString &path);
    
    /**
     * @brief Get current recording path
     * Returns the path where recorded video files are stored
     * @return Recording path string (e.g., "/media/ext_sdcard2/dvr/Normal/")
     */
    QString getRecordPath() const;

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
     * - DVR_UI_MSG_REC_STARTED: Confirm recording started, emit recordingStarted()
     * - DVR_UI_MSG_REC_STOPED: Confirm recording stopped, emit recordingStopped()
     * - DVR_UI_MSG_START_REC_FAILED: Emit recordingError() with failure message
     * - DVR_UI_MSG_STOP_REC_FAILED: Emit recordingError() with failure message
     * - DVR_UI_MSG_SD_FULL: Auto-stop recording, emit storageFullWarning()
     * - DVR_UI_MSG_CAM_ERROR: Handle camera error, emit recordingError()
     *
     * @param eventType DVR event type (from DVR_UI_MSG_E enum)
     * @param param1 Event parameter 1 (typically camera type)
     * @param param2 Event parameter 2 (error code or reserved)
     */
    void onDvrEvent(uint eventType, uint param1, uint param2);

signals:
    /**
     * @brief Emitted when recording state changes
     * @param camera Camera type (FRONT or REAR)
     * @param active true if recording started, false if stopped
     */
    void recordingStateChanged(DVR_CAM_TYPE_E camera, bool active);

    /**
     * @brief Emitted when recording actually starts (confirmed by DVR lib)
     * This is the confirmation signal after successful startRecording() call
     * @param camera Camera type that started recording
     */
    void recordingStarted(DVR_CAM_TYPE_E camera);

    /**
     * @brief Emitted when recording stops
     * @param camera Camera type that stopped recording
     */
    void recordingStopped(DVR_CAM_TYPE_E camera);

    /**
     * @brief Emitted when recording error occurs
     * @param message Error description
     */
    void recordingError(const QString &message);

    /**
     * @brief Emitted when storage is nearly full (80% threshold)
     * UI should show warning but recording continues
     */
    void storageWarning();

    /**
     * @brief Emitted when storage is critically full (95% threshold)
     * Recording will be auto-stopped
     */
    void storageFullWarning();

private:
    /**
     * @brief Get camera name string for logging
     * @param camera Camera type
     * @return "Front" or "Rear"
     */
    QString getCameraName(DVR_CAM_TYPE_E camera) const;

    /**
     * @brief Validate storage is available and writable
     * @return true if storage is ready
     */
    bool validateStorage();

    /**
     * @brief Check if SD card is properly mounted (not tmpfs)
     * @return true if real SD card filesystem (vfat/exfat), false if tmpfs
     */
    bool checkSdCardMounted();

    // System-defined SD card mount point (fixed by system)
    static const QString SD_CARD_MOUNT_POINT;    // "/media/ext_sdcard2"
    
    // Application recording subdirectory name
    static const QString RECORDING_SUBDIR;        // "dvr"

    // Initialization state
    bool m_initialized;
    mutable bool m_sdCardReady;  // true if SD card is properly mounted (not tmpfs), mutable for lazy detection

    // Per-camera recording state (independent control)
    bool m_isFrontRecordingActive;
    bool m_isRearRecordingActive;

    // Callback to get current camera from DVRBackend
    // Used when DVR library events don't specify which camera triggered the event
    std::function<DVR_CAM_TYPE_E()> m_getCurrentCamera;

    // Recording configuration
    QString m_recordPath;            // Full recording path (SD_CARD_MOUNT_POINT + RECORDING_SUBDIR)
    uint32_t m_recordDuration;       // Duration per file in seconds (default: 180s = 3min)

    // Storage capacity management:
    // NOTE: Capacity checking is handled by DVR Library at runtime
    // - DVR Library uses absolute thresholds (100MB/50MB) in dvr_recorder.h
    // - App layer only validates SD card mount status, not capacity percentage
    // - App layer handles DVR_UI_MSG_SD_FULL notifications from DVR Library
};

#endif // RECORDMANAGER_H
