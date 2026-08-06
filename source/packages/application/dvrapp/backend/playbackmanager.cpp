/*
 * Copyright (c) 2025 AutoChips Inc.
 *
 * PlaybackManager - Implementation
 */

#include "playbackmanager.h"
#include "dvrlog.h"
#include <QFileInfo>
#include <QMetaObject>
#include <QThread>

static const char* TAG = "PlaybackManager";

PlaybackManager::PlaybackManager(QObject *parent)
    : QObject(parent)
    , m_dvrPlayerInitialized(false)
    , m_position(0)
    , m_duration(0)
    , m_state(StoppedState)
    , m_seekInProgress(false)
    , m_isPrepared(false)
    , m_surfaceWidth(0)
    , m_surfaceHeight(0)
    , m_surfaceX(0)
    , m_surfaceY(0)
{
    LOGI(TAG, "PlaybackManager created");

    // Create position update timer
    m_positionUpdateTimer = new QTimer(this);
    m_positionUpdateTimer->setInterval(POSITION_UPDATE_INTERVAL);
    connect(m_positionUpdateTimer, &QTimer::timeout,
            this, &PlaybackManager::onPositionUpdateTimer);
}

PlaybackManager::~PlaybackManager()
{
    LOGI(TAG, "PlaybackManager destroying");

    // Stop timer
    if (m_positionUpdateTimer && m_positionUpdateTimer->isActive()) {
        m_positionUpdateTimer->stop();
    }

    // Deinitialize player if initialized
    if (m_dvrPlayerInitialized) {
        deinitializeDVRPlayer();
    }

    LOGI(TAG, "PlaybackManager destroyed");
}

bool PlaybackManager::initializeDVRPlayer()
{
    if (m_dvrPlayerInitialized) {
        LOGW(TAG, "DVR Player already initialized");
        return true;
    }

    LOGI(TAG, "[OK] Initializing DVR Player...");

    // Step 1: Initialize DVR Player library
    if (!DVR_PlayerInit(DVRPlayerCallback, this)) {
        LOGE(TAG, "[ERROR] DVR_PlayerInit failed");
        return false;
    }
    LOGI(TAG, "[OK] DVR_PlayerInit succeeded");

    // Step 2: Set video surface (adapt to screen resolution)
    // CRITICAL: Get actual screen size from Qt, DO NOT hardcode
    QScreen *screen = QApplication::primaryScreen();
    if (!screen) {
        LOGE(TAG, "[ERROR] Failed to get primary screen");
        DVR_PlayerDeinit();
        return false;
    }

    QRect screenGeometry = screen->geometry();
    const int surfaceWidth = screenGeometry.width();   // e.g., 1024 or 800
    const int surfaceHeight = screenGeometry.height(); // e.g., 600 or 480
    const int surfaceX = 0;
    const int surfaceY = 0;

    LOGI(TAG, "[OK] Detected screen resolution: %dx%d", surfaceWidth, surfaceHeight);

    // CRITICAL: Save surface parameters for replay re-initialization
    // After playback completion, MediaPlayer is fully deinitialized
    // Replay requires these parameters to recreate Surface with same dimensions
    m_surfaceWidth = surfaceWidth;
    m_surfaceHeight = surfaceHeight;
    m_surfaceX = surfaceX;
    m_surfaceY = surfaceY;

    if (!DVR_PlayerSetSurface(surfaceWidth, surfaceHeight, surfaceX, surfaceY)) {
        LOGE(TAG, "[ERROR] DVR_PlayerSetSurface failed");
        DVR_PlayerDeinit();
        return false;
    }
    LOGI(TAG, "[OK] DVR_PlayerSetSurface succeeded (surface params saved for replay)");

    m_dvrPlayerInitialized = true;
    LOGI(TAG, "[OK] DVR Player initialized successfully");
    return true;
}

void PlaybackManager::deinitializeDVRPlayer()
{
    if (!m_dvrPlayerInitialized) {
        return;
    }

    LOGI(TAG, "[OK] Deinitializing DVR Player...");
    LOGI(TAG, "    - This will release VDEC hardware");
    LOGI(TAG, "    - This will release VSINK resources");
    LOGI(TAG, "    - This will release IAtcSurface");
    LOGI(TAG, "    - This will delete MediaPlayer instance");

    DVR_PlayerDeinit();

    m_dvrPlayerInitialized = false;
    m_isPrepared = false;

    LOGI(TAG, "[OK] DVR Player deinitialized, all resources released");
}

bool PlaybackManager::initialize(const QString &filePath)
{
    LOGI(TAG, "========================================");
    LOGI(TAG, "initialize: Preparing to play: %s", qPrintable(filePath));
    LOGI(TAG, "========================================");

    // STEP 1: Validate file
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        LOGE(TAG, "[ERROR] File not found: %s", qPrintable(filePath));
        emit playbackError(tr("Video file not found"));
        return false;
    }
    LOGI(TAG, "[OK] File validation passed");

    // STEP 2: Stop current playback if any
    if (m_state != StoppedState) {
        LOGI(TAG, "STEP 2: Stopping current playback...");
        stop();
    }

    // STEP 3: Initialize DVR Player (Init + SetSurface)
    if (m_dvrPlayerInitialized) {
        LOGW(TAG, "STEP 3: Player already initialized, deinitializing first...");
        deinitializeDVRPlayer();
    }

    if (!initializeDVRPlayer()) {
        LOGE(TAG, "[ERROR] Failed to initialize DVR Player");
        emit playbackError(tr("Failed to initialize player"));
        return false;
    }
    LOGI(TAG, "[OK] DVR Player initialized (VDEC + Surface allocated)");

    // STEP 4: Save file path
    m_currentFilePath = filePath;
    emit currentFilePathChanged();

    // STEP 5: Prepare media file
    LOGI(TAG, "STEP 5: Preparing media file...");
    if (!DVR_PlayerPrepare(filePath.toUtf8().constData())) {
        LOGE(TAG, "[ERROR] DVR_PlayerPrepare failed");
        emit playbackError(tr("Failed to prepare video file"));
        deinitializeDVRPlayer();  // Cleanup on failure
        return false;
    }
    LOGI(TAG, "[OK] File prepared, waiting for onPlayerPrepared callback");

    // STEP 6: Playback will auto-start in onPlayerPrepared callback

    LOGI(TAG, "========================================");
    LOGI(TAG, "initialize completed successfully");
    LOGI(TAG, "========================================");
    return true;
}

void PlaybackManager::deinitialize()
{
    LOGI(TAG, "========================================");
    LOGI(TAG, "deinitialize: Cleaning up playback resources");
    LOGI(TAG, "========================================");

    // STEP 1: Stop playback if still playing
    if (m_state != StoppedState) {
        LOGI(TAG, "STEP 1: Stopping playback (current state: %d)", m_state);
        stop();
    } else {
        LOGI(TAG, "STEP 1: Already stopped, skipping stop()");
    }

    // STEP 2: Deinitialize DVR Player (releases VDEC, VSINK, IAtcSurface, /dev/video0)
    LOGI(TAG, "STEP 2: Deinitializing DVR Player to release all resources");
    if (m_dvrPlayerInitialized) {
        deinitializeDVRPlayer();
        LOGI(TAG, "STEP 2: DVR Player deinitialized, all resources released");
    } else {
        LOGI(TAG, "STEP 2: Player not initialized, skipping deinit");
    }

    // Clear file path
    m_currentFilePath.clear();
    emit currentFilePathChanged();

    LOGI(TAG, "========================================");
    LOGI(TAG, "deinitialize completed");
    LOGI(TAG, "========================================");
}

bool PlaybackManager::playVideo(const QString &filePath)
{
    LOGI(TAG, "========================================");
    LOGI(TAG, "playVideo: %s", qPrintable(filePath));

    // Validate file path
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        LOGE(TAG, "[ERROR] File not found: %s", qPrintable(filePath));
        emit playbackError(tr("Video file not found"));
        return false;
    }
    LOGI(TAG, "[OK] File exists: %s", qPrintable(filePath));

    // Stop current playback if any
    if (m_state != StoppedState) {
        LOGI(TAG, "Stopping current playback...");
        stop();
    }

    // Initialize player if needed
    if (!m_dvrPlayerInitialized) {
        LOGI(TAG, "Player not initialized, initializing now...");
        if (!initializeDVRPlayer()) {
            LOGE(TAG, "[ERROR] Failed to initialize player");
            emit playbackError(tr("Failed to initialize player"));
            return false;
        }
    }

    // Save file path
    m_currentFilePath = filePath;
    emit currentFilePathChanged();

    // Prepare media file
    LOGI(TAG, "Calling DVR_PlayerPrepare...");
    if (!DVR_PlayerPrepare(filePath.toUtf8().constData())) {
        LOGE(TAG, "[ERROR] DVR_PlayerPrepare failed");
        emit playbackError(tr("Failed to prepare video file"));
        return false;
    }
    LOGI(TAG, "[OK] DVR_PlayerPrepare succeeded");
    LOGI(TAG, "Waiting for onPlayerPrepared callback...");

    // Note: Actual playback start happens in onPlayerPrepared callback
    // which is triggered by DVR_PLAYER_MSG_PREPARED message

    return true;
}

void PlaybackManager::play()
{
    LOGI(TAG, "play() called, current state: %d, initialized: %s",
         m_state, m_dvrPlayerInitialized ? "true" : "false");

    // Check if player needs re-initialization (after playback completion)
    if (!m_dvrPlayerInitialized) {
        LOGI(TAG, "========================================");
        LOGI(TAG, "REPLAY: Player not initialized - starting full re-initialization");
        LOGI(TAG, "REPLAY: This happens after natural playback completion");
        LOGI(TAG, "========================================");

        if (m_currentFilePath.isEmpty()) {
            LOGE(TAG, "[ERROR] No file loaded for replay");
            return;
        }

        // Full re-initialization: Init → SetSurface → Prepare → Play
        // This ensures clean hardware state (VDEC/VSINK/Surface)
        playVideo(m_currentFilePath);
        return;
    }

    // Normal play/pause toggle logic (when player is already initialized)
    if (m_state == PlayingState) {
        LOGW(TAG, "Already playing");
        return;
    }

    if (m_state == PausedState) {
        LOGI(TAG, "Resuming from pause");
        if (!DVR_PlayerPlay()) {
            LOGE(TAG, "[ERROR] DVR_PlayerPlay failed (resume)");
            return;
        }
        setState(PlayingState);
        m_positionUpdateTimer->start();
        LOGI(TAG, "[OK] Playback resumed");
    } else if (m_state == StoppedState && !m_currentFilePath.isEmpty()) {
        // This is replay case (user clicked play after natural completion)
        // But player is still initialized (contradiction with check above)
        // This shouldn't happen with proper state management
        LOGW(TAG, "Stopped state with initialized player - calling playVideo");
        playVideo(m_currentFilePath);
    }
}

void PlaybackManager::pause()
{
    if (m_state != PlayingState) {
        LOGW(TAG, "Cannot pause: not playing (state: %d)", m_state);
        return;
    }

    LOGI(TAG, "Pausing playback");
    if (!DVR_PlayerPause()) {
        LOGE(TAG, "[ERROR] DVR_PlayerPause failed");
        return;
    }

    setState(PausedState);
    m_positionUpdateTimer->stop();
    LOGI(TAG, "[OK] Playback paused");
}

void PlaybackManager::stop()
{
    if (m_state == StoppedState) {
        LOGW(TAG, "Already stopped");
        return;
    }

    LOGI(TAG, "Stopping playback");

    // Stop timer
    if (m_positionUpdateTimer->isActive()) {
        m_positionUpdateTimer->stop();
    }

    // Stop player
    if (m_dvrPlayerInitialized) {
        DVR_PlayerStop();
    }

    setState(StoppedState);
    m_position = 0;
    emit positionChanged();

    LOGI(TAG, "[OK] Playback stopped");
}

void PlaybackManager::seek(int positionMs)
{
    LOGI(TAG, "[DEBUG_SEEK] seek called: input=%d ms, m_duration=%d ms, m_position=%d ms", 
         positionMs, m_duration, m_position);
    
    if (!m_dvrPlayerInitialized || m_state == StoppedState) {
        LOGW(TAG, "[DEBUG_SEEK] Cannot seek: player not ready (initialized=%d, state=%d)", 
             m_dvrPlayerInitialized, m_state);
        return;
    }

    // Clamp position to valid range
    int oldPosition = positionMs;
    positionMs = qBound(0, positionMs, m_duration);
    if (oldPosition != positionMs) {
        LOGI(TAG, "[DEBUG_SEEK] Position clamped: %d -> %d ms (duration=%d)", 
             oldPosition, positionMs, m_duration);
    }

    LOGI(TAG, "[DEBUG_SEEK] Final seek position: %d ms", positionMs);

    m_seekInProgress = true;
    if (!DVR_PlayerSeek(static_cast<__u32>(positionMs))) {
        LOGE(TAG, "[ERROR] DVR_PlayerSeek failed");
        m_seekInProgress = false;
        return;
    }

    // Position will be updated in onSeekCompleted callback
    LOGI(TAG, "[OK] Seek request sent, waiting for completion...");
}

void PlaybackManager::seekBackward(int deltaMs)
{
    LOGI(TAG, "[DEBUG_SEEK] seekBackward called: deltaMs=%d, current m_position=%d", deltaMs, m_position);
    int newPosition = m_position - deltaMs;
    LOGI(TAG, "[DEBUG_SEEK] seekBackward: %d - %d = %d ms", m_position, deltaMs, newPosition);
    seek(newPosition);
}

void PlaybackManager::seekForward(int deltaMs)
{
    LOGI(TAG, "[DEBUG_SEEK] seekForward called: deltaMs=%d, current m_position=%d", deltaMs, m_position);
    int newPosition = m_position + deltaMs;
    LOGI(TAG, "[DEBUG_SEEK] seekForward: %d + %d = %d ms", m_position, deltaMs, newPosition);
    seek(newPosition);
}

void PlaybackManager::skipToStart()
{
    seek(0);
}

void PlaybackManager::skipToEnd()
{
    if (m_duration > 1000) {
        seek(m_duration - 1000);  // 1 second before end
    }
}

void PlaybackManager::setState(PlaybackState newState)
{
    if (m_state != newState) {
        PlaybackState oldState = m_state;
        m_state = newState;
        LOGI(TAG, "State changed: %d -> %d", oldState, newState);
        emit stateChanged();
    }
}

void PlaybackManager::updatePosition()
{
    LOGI(TAG, "[DEBUG_SEEK] updatePosition called: initialized=%d, state=%d (Playing=%d)", 
         m_dvrPlayerInitialized, m_state, PlayingState);
    
    if (!m_dvrPlayerInitialized) {
        LOGW(TAG, "[DEBUG_SEEK] updatePosition skipped: Player not initialized");
        return;
    }
    
    if (m_state != PlayingState) {
        LOGW(TAG, "[DEBUG_SEEK] updatePosition skipped: State is %d, expected PlayingState(%d)", 
             m_state, PlayingState);
        return;
    }
    
    // Skip position updates during seek operation
    // During seek, decoder is flushing and repositioning,
    // DVR_PlayerGetPosition() returns unreliable old position which would
    // overwrite the target position set by onSeekCompleted callback
    if (m_seekInProgress) {
        LOGI(TAG, "[DEBUG_SEEK] updatePosition skipped: Seek in progress");
        return;
    }

    __u32 position = DVR_PlayerGetPosition();
    LOGI(TAG, "[DEBUG_SEEK] DVR_PlayerGetPosition returned: %u ms, current m_position: %d ms", 
         position, m_position);
    
    if (position != m_position) {
        int oldPosition = m_position;
        m_position = static_cast<int>(position);
        LOGI(TAG, "[DEBUG_SEEK] Position updated: %d -> %d ms", oldPosition, m_position);
        emit positionChanged();
    } else {
        LOGI(TAG, "[DEBUG_SEEK] Position unchanged: %d ms", m_position);
    }
}

QString PlaybackManager::formatTime(int ms) const
{
    int totalSeconds = ms / 1000;
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    if (hours > 0) {
        return QString("%1:%2:%3")
            .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2")
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    }
}

void PlaybackManager::onPositionUpdateTimer()
{
    updatePosition();
}

void PlaybackManager::onPlayerPrepared()
{
    LOGI(TAG, "========================================");
    LOGI(TAG, "CALLBACK: onPlayerPrepared");

    m_isPrepared = true;

    // Get duration from DVR_PlayerGetDuration first
    __u32 duration = DVR_PlayerGetDuration();
    if (duration > 0) {
        m_duration = static_cast<int>(duration);
        emit durationChanged();
        LOGI(TAG, "[OK] Duration from DVR_PlayerGetDuration: %d ms (%s)", m_duration, qPrintable(formatTime(m_duration)));
    } else {
        LOGW(TAG, "[WARNING] DVR_PlayerGetDuration returned 0, attempting to get from MediaScanner");
        
        // Try to get duration from MediaScanner (same approach as original app)
        char durationStr[16] = {0};
        if (DVR_MediaScanner_GetVideoDuration(m_currentFilePath.toUtf8().constData(), durationStr, sizeof(durationStr))) {
            QString durationString = QString::fromUtf8(durationStr);
            if (!durationString.isEmpty() && durationString != "ERROR" && durationString != "00:00:00") {
                LOGI(TAG, "[OK] MediaScanner duration string: %s", durationStr);
                
                // Convert HH:MM:SS to milliseconds
                QStringList parts = durationString.split(':');
                if (parts.size() == 3) {
                    int hours = parts[0].toInt();
                    int minutes = parts[1].toInt();
                    int seconds = parts[2].toInt();
                    m_duration = (hours * 3600 + minutes * 60 + seconds) * 1000;
                    emit durationChanged();
                    LOGI(TAG, "[OK] Duration from MediaScanner: %d ms (%s)", m_duration, qPrintable(formatTime(m_duration)));
                } else if (parts.size() == 2) {
                    // MM:SS format
                    int minutes = parts[0].toInt();
                    int seconds = parts[1].toInt();
                    m_duration = (minutes * 60 + seconds) * 1000;
                    emit durationChanged();
                    LOGI(TAG, "[OK] Duration from MediaScanner: %d ms (%s)", m_duration, qPrintable(formatTime(m_duration)));
                } else {
                    LOGW(TAG, "[WARNING] Invalid duration format: %s", durationStr);
                    m_duration = 0;
                }
            } else {
                LOGW(TAG, "[WARNING] MediaScanner returned invalid duration: %s", durationStr);
                m_duration = 0;
            }
        } else {
            LOGW(TAG, "[WARNING] DVR_MediaScanner_GetVideoDuration failed");
            m_duration = 0;
        }
    }

    // Start playback
    LOGI(TAG, "Calling DVR_PlayerPlay to start playback...");
    if (!DVR_PlayerPlay()) {
        LOGE(TAG, "[ERROR] DVR_PlayerPlay failed");
        emit playbackError(tr("Failed to start playback"));
        return;
    }

    setState(PlayingState);
    m_position = 0;
    emit positionChanged();

    // Start position update timer
    m_positionUpdateTimer->start();

    LOGI(TAG, "[OK] Playback started successfully");
    LOGI(TAG, "========================================");
}

void PlaybackManager::onSeekCompleted(quint32 positionMs)
{
    LOGI(TAG, "CALLBACK: onSeekCompleted - position: %u ms (seek command processed)", positionMs);

    // Do NOT update m_position here - it should only be updated by updatePosition()
    // from DVR_PlayerGetPosition() which returns the actual decoder output position
    // 
    // The onSeekCompleted callback is fired immediately after seek command is queued,
    // but decoder needs time to: flush buffers (50ms) + file seek (100ms) + decode I-frame (50ms)
    // Total delay: ~200ms before decoder actually outputs frames at the new position
    //
    // Delay clearing m_seekInProgress flag to avoid updatePosition() reading stale position
    // from DVR_PlayerGetPosition() during the transition period
    QTimer::singleShot(200, this, [this]() {
        m_seekInProgress = false;
        LOGI(TAG, "[OK] Seek stabilization complete (200ms elapsed), resume position updates");
    });
    
    LOGI(TAG, "[OK] Seek command acknowledged, waiting 200ms for decoder stabilization");
}

void PlaybackManager::onPlaybackFinished()
{
    LOGI(TAG, "========================================");
    LOGI(TAG, "PLAYBACK_COMPLETION: Video playback finished naturally");
    LOGI(TAG, "========================================");

    // Stop position updates immediately
    if (m_positionUpdateTimer && m_positionUpdateTimer->isActive()) {
        m_positionUpdateTimer->stop();
        LOGI(TAG, "PLAYBACK_COMPLETION: Position update timer stopped");
    }

    // CRITICAL: Completely deinitialize MediaPlayer to release all resources
    // This prevents VDEC hardware resource conflicts when replaying
    // Root cause: MediaPlayer::setup() being called multiple times without releasing old AVCodec
    // Solution: Deinit/Init cycle ensures clean state for replay
    LOGI(TAG, "PLAYBACK_COMPLETION: Calling deinitializeDVRPlayer to fully release resources");
    LOGI(TAG, "PLAYBACK_COMPLETION: This will trigger:");
    LOGI(TAG, "PLAYBACK_COMPLETION:   - codec->stop() (stop message_handler thread)");
    LOGI(TAG, "PLAYBACK_COMPLETION:   - codec->release() (release VDEC/VSINK hardware)");
    LOGI(TAG, "PLAYBACK_COMPLETION:   - IAtcSurface_release() (release V4L2 surface)");
    LOGI(TAG, "PLAYBACK_COMPLETION:   - delete MediaPlayer (free memory)");

    deinitializeDVRPlayer();

    LOGI(TAG, "PLAYBACK_COMPLETION: MediaPlayer deinitialized successfully");
    LOGI(TAG, "PLAYBACK_COMPLETION: All hardware resources released");

    // Update state
    setState(StoppedState);

    // Keep position at end for UI display
    m_position = m_duration;
    emit positionChanged();

    // Notify QML
    emit playbackFinished();

    LOGI(TAG, "PLAYBACK_COMPLETION: UI updated - ready for replay");
    LOGI(TAG, "PLAYBACK_COMPLETION: Next play will trigger full re-initialization:");
    LOGI(TAG, "PLAYBACK_COMPLETION:   1. DVR_PlayerInit (create new MediaPlayer)");
    LOGI(TAG, "PLAYBACK_COMPLETION:   2. DVR_PlayerSetSurface (allocate VDEC, create Surface)");
    LOGI(TAG, "PLAYBACK_COMPLETION:   3. DVR_PlayerPrepare (setup codec, parse file)");
    LOGI(TAG, "PLAYBACK_COMPLETION:   4. DVR_PlayerPlay (start playback)");
    LOGI(TAG, "========================================");
}

void PlaybackManager::DVRPlayerCallback(__u32 msgType, __u32 param1, __u32 param2, void* userData)
{
    PlaybackManager* manager = static_cast<PlaybackManager*>(userData);
    if (!manager) {
        LOGE(TAG, "[ERROR] DVRPlayerCallback: manager is NULL");
        return;
    }

    // Use Qt's thread-safe mechanism to invoke slots
    switch (msgType) {
        case DVR_PLAYER_MSG_PREPARED:
            LOGI(TAG, "CALLBACK: DVR_PLAYER_MSG_PREPARED received");
            QMetaObject::invokeMethod(manager, "onPlayerPrepared", Qt::QueuedConnection);
            break;

        case DVR_PLAYER_MSG_SEEK_COMPLETE:
            LOGI(TAG, "CALLBACK: DVR_PLAYER_MSG_SEEK_COMPLETE received (position: %u)", param1);
            QMetaObject::invokeMethod(manager, "onSeekCompleted", Qt::QueuedConnection,
                                     Q_ARG(quint32, param1));
            break;

        case DVR_PLAYER_MSG_PLAYBACK_COMPLETE:
            LOGI(TAG, "CALLBACK: DVR_PLAYER_MSG_PLAYBACK_COMPLETE received");
            QMetaObject::invokeMethod(manager, "onPlaybackFinished", Qt::QueuedConnection);
            break;

        case DVR_PLAYER_MSG_ERROR:
            LOGE(TAG, "CALLBACK: DVR_PLAYER_MSG_ERROR received (error code: %u)", param1);
            QMetaObject::invokeMethod(manager, "playbackError", Qt::QueuedConnection,
                                     Q_ARG(QString, QString("Playback error: %1").arg(param1)));
            break;

        case DVR_PLAYER_MSG_PROGRESS_UPDATE:
            // Don't log this, too frequent
            break;

        default:
            LOGW(TAG, "CALLBACK: Unknown message type: %u", msgType);
            break;
    }
}
