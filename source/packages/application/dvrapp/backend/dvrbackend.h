/*
 * Copyright (c) 2025 AutoChips Inc.
 *
 * DVRBackend - Main coordinator for DVR application
 * Refactored architecture with separated concerns
 */

#ifndef DVRBACKEND_H
#define DVRBACKEND_H

#include <QObject>
#include "qobjlistener.h"  // For E_VIDEOFOCUS, E_AUDIOFOCUS enums
#include "dvr.h"          // For DVR_CAM_TYPE_E and DVR API functions

// Forward declarations
class CameraManager;
class PreviewManager;
class RecordManager;
class PlaybackManager;
class FileManager;
class SettingsManager;
class DVRQObjListener;

/**
 * @brief DVRBackend - Main coordinator class
 *
 * Responsibilities:
 * - Coordinate between different managers
 * - Handle system events delegated from DVRQObjListener
 * - Expose properties and methods to QML layer
 * - Manage application lifecycle
 *
 * Note: DVRBackend does NOT inherit CQObjListener directly.
 * Instead, DVRQObjListener handles system events and delegates to this class.
 */
class DVRBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isFrontCamera READ isFrontCamera WRITE setFrontCamera NOTIFY frontCameraChanged)
    Q_PROPERTY(bool isDualCameraMode READ isDualCameraMode WRITE setDualCameraMode NOTIFY dualCameraModeChanged)
    Q_PROPERTY(bool isPreviewing READ isPreviewing NOTIFY previewingChanged)
    Q_PROPERTY(bool isRecording READ isRecording NOTIFY recordingChanged)
    Q_PROPERTY(SettingsManager* settingsManager READ getSettingsManager CONSTANT)
    Q_PROPERTY(FileManager* fileManager READ getFileManager CONSTANT)
    Q_PROPERTY(PlaybackManager* playbackManager READ getPlaybackManager CONSTANT)

public:
    // Timing constants for auto-preview startup and retry mechanism
    static constexpr int AUTO_PREVIEW_DELAY_MS = 500;      // Initial delay before auto-preview
    static constexpr int PREVIEW_RETRY_INTERVAL_MS = 200;  // Retry interval
    static constexpr int PREVIEW_MAX_RETRY_COUNT = 10;     // Max retry attempts (2 seconds total)
    explicit DVRBackend(QObject *parent = nullptr);
    virtual ~DVRBackend();

    // Full initialization: Initialize DVR library and all managers
    // Can be called before QML loads - only initListener() requires window object
    Q_INVOKABLE bool initialize();
    Q_INVOKABLE void shutdown();

    // Get QObjListener for main.cpp to call initListener
    DVRQObjListener* getQObjListener() const { return m_qobjListener; }

    // Get SettingsManager for QML access
    SettingsManager* getSettingsManager() const { return m_settingsManager; }
    
    // Get FileManager for QML access
    FileManager* getFileManager() const { return m_fileManager; }
    
    // Get PlaybackManager for QML access
    PlaybackManager* getPlaybackManager() const { return m_playbackManager; }

    // Navigation and Exit
    Q_INVOKABLE void goHome();
    Q_INVOKABLE void goExit();
    
    // File list navigation
    Q_INVOKABLE void showFileList();  // Stop preview, show file list
    Q_INVOKABLE void hideFileList();  // Hide file list, restart preview
    
    // Video player navigation
    Q_INVOKABLE void showVideoPlayer(const QString &filePath);  // Stop preview, play video
    Q_INVOKABLE void hideVideoPlayer();  // Stop playback, restart preview

    // Camera control
    Q_INVOKABLE void switchCamera();  // Called when camera switch button clicked

    // Recording control (exposed to QML)
    Q_INVOKABLE bool startRecording();     // Start recording current camera
    Q_INVOKABLE bool stopRecording();      // Stop recording current camera
    Q_INVOKABLE bool toggleRecording();    // Toggle recording state for current camera

    // Property getters
    bool isFrontCamera() const { return m_isFrontCamera; }
    bool isDualCameraMode() const { return m_isDualCameraMode; }
    bool isPreviewing() const;
    bool isRecording() const;

    // Property setters
    void setFrontCamera(bool front);
    void setDualCameraMode(bool dual);

    // System event handlers (called by DVRQObjListener)
    void handleShowFront(int param1, int param2);
    void handleHideFront(int param1, int param2);
    void handleVideoFocusChanged(CCtlListener::E_AVOUT vOut, CCtlListener::E_VIDEOFOCUS focus);
    void handleAudioFocusChanged(CCtlListener::E_AVOUT aOut, CCtlListener::E_AUDIOFOCUS focus);

    // DVR event handler (called by dvrEventCallback from DVR library)
    void handleDvrEvent(uint32_t event, uint32_t param1, uint32_t param2);

signals:
    void frontCameraChanged(bool isFront);
    void dualCameraModeChanged(bool dual);
    void previewingChanged(bool previewing);
    void recordingChanged(bool recording);
    void cameraConnectionError(const QString &message);
    void errorOccurred(const QString &message);
    void videoFocusChanged(CCtlListener::E_AVOUT vOut, CCtlListener::E_VIDEOFOCUS focus);
    void audioFocusChanged(CCtlListener::E_AVOUT aOut, CCtlListener::E_AUDIOFOCUS focus);
    
    // File list navigation signals
    void navigateToFileList();
    void navigateBackFromFileList();
    
    // Video player navigation signals
    void videoPlayerRequested(const QString &filePath);
    void videoPlayerClosed();

private slots:
    // Connected to PreviewManager signals
    void onPreviewStateChanged(DVR_CAM_TYPE_E camera, bool active);
    void onPreviewError(DVR_CAM_TYPE_E camera, const QString &error);

    // Connected to CameraManager signals
    void onCameraAvailabilityChanged(DVR_CAM_TYPE_E camera, bool available);

    // Connected to RecordManager signals
    void onRecordingStateChanged(DVR_CAM_TYPE_E camera, bool active);
    void onRecordingStarted(DVR_CAM_TYPE_E camera);
    void onRecordingStopped(DVR_CAM_TYPE_E camera);
    void onRecordingError(const QString &message);
    void onStorageFullWarning();

private:
    // Auto-preview with retry mechanism (inspired by original app)
    void tryAutoStartPreview(int retryCount = 0);

    // Get current camera based on QML button state
    DVR_CAM_TYPE_E getCurrentCamera() const;

    /**
     * @brief Wait for /dev/video0 to become available after preview stop
     * Polls the device by attempting to open it in non-blocking mode
     * @param timeoutMs Maximum wait time in milliseconds
     * @return true if device becomes available within timeout, false otherwise
     */
    bool waitForDeviceReady(int timeoutMs = 500);

    // Manager instances
    CameraManager *m_cameraManager;
    PreviewManager *m_previewManager;
    RecordManager *m_recordManager;
    PlaybackManager *m_playbackManager;
    FileManager *m_fileManager;
    SettingsManager *m_settingsManager;

    // System event listener (owns this)
    DVRQObjListener *m_qobjListener;

    // State
    bool m_isFrontCamera;        // Synced with QML switch button
    bool m_isDualCameraMode;
    bool m_initialized;
};

#endif // DVRBACKEND_H
