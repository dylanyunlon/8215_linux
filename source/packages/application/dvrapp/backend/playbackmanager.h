/*
 * Copyright (c) 2025 AutoChips Inc.
 *
 * PlaybackManager - Video playback management for DVR file playback
 */

#ifndef PLAYBACKMANAGER_H
#define PLAYBACKMANAGER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QScreen>
#include <QApplication>
#include "dvr.h"

/**
 * @brief PlaybackManager - Manages video playback using DVR_Player library
 *
 * This class provides Qt/QML interface for playing back recorded DVR video files.
 * It wraps the DVR_Player C library and provides property bindings, signals,
 * and invokable methods for QML integration.
 *
 * Key Features:
 * - Fullscreen video playback with hardware decoding (VDEC)
 * - Progress tracking and seeking
 * - State management (Stopped/Playing/Paused/Error)
 * - Automatic screen resolution detection (1024x600 or 800x480)
 * - Complete resource cleanup on playback completion (follows original DVR app design)
 *
 * Resource Management:
 * - Uses full re-initialization on replay (deinit + init cycle)
 * - Prevents VDEC hardware conflicts on embedded platform
 * - Releases Surface and codec resources completely
 *
 * Thread Safety:
 * - DVR_Player callbacks use Qt::QueuedConnection via QMetaObject::invokeMethod
 * - All state changes are thread-safe
 */
class PlaybackManager : public QObject
{
    Q_OBJECT

    // Properties exposed to QML
    Q_PROPERTY(int position READ position NOTIFY positionChanged)
    Q_PROPERTY(int duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(PlaybackState state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString currentFilePath READ currentFilePath NOTIFY currentFilePathChanged)

public:
    enum PlaybackState {
        StoppedState = 0,
        PlayingState = 1,
        PausedState = 2,
        ErrorState = 3
    };
    Q_ENUM(PlaybackState)

    explicit PlaybackManager(QObject *parent = nullptr);
    ~PlaybackManager();

    // Property getters
    int position() const { return m_position; }
    int duration() const { return m_duration; }
    PlaybackState state() const { return m_state; }
    QString currentFilePath() const { return m_currentFilePath; }

    // Lifecycle management - pure playback resource management
    bool initialize(const QString &filePath);
    void deinitialize();

    // Invokable methods for QML
    Q_INVOKABLE bool playVideo(const QString &filePath);
    Q_INVOKABLE void play();       // Resume from pause or replay after completion
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void seek(int positionMs);
    Q_INVOKABLE void seekBackward(int deltaMs = 10000);  // Default -10s
    Q_INVOKABLE void seekForward(int deltaMs = 10000);   // Default +10s
    Q_INVOKABLE void skipToStart();
    Q_INVOKABLE void skipToEnd();

signals:
    void positionChanged();
    void durationChanged();
    void stateChanged();
    void currentFilePathChanged();
    void playbackFinished();  // Emitted when video reaches end naturally
    void playbackError(const QString &errorMsg);

private slots:
    void onPositionUpdateTimer();
    void onPlayerPrepared();
    void onSeekCompleted(quint32 positionMs);
    void onPlaybackFinished();

private:
    bool initializeDVRPlayer();
    void deinitializeDVRPlayer();
    void setState(PlaybackState newState);
    void updatePosition();
    QString formatTime(int ms) const;

    // DVR Player state
    bool m_dvrPlayerInitialized;
    QString m_currentFilePath;
    int m_position;          // Current position in milliseconds
    int m_duration;          // Total duration in milliseconds
    PlaybackState m_state;
    bool m_seekInProgress;
    bool m_isPrepared;       // Whether file is prepared and ready

    // Surface parameters (saved for replay re-initialization)
    // Required because full deinit releases Surface, but replay needs same dimensions
    int m_surfaceWidth;
    int m_surfaceHeight;
    int m_surfaceX;
    int m_surfaceY;

    // Update timer
    QTimer *m_positionUpdateTimer;

    // Constants
    static const int POSITION_UPDATE_INTERVAL = 100;  // 100ms = 10 updates/sec

    // Static callback for DVR_Player
    static void DVRPlayerCallback(__u32 msgType, __u32 param1, __u32 param2, void* userData);

    friend class PlaybackManagerTest;  // For unit testing
};

#endif // PLAYBACKMANAGER_H
