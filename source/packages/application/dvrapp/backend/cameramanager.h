/*
 * Copyright (c) 2025 AutoChips Inc.
 *
 * CameraManager - Camera hardware lifecycle management
 * Handles camera connection detection and on-demand initialization
 */

#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include <QObject>
#include "dvr.h"  // For DVR_CAM_TYPE_E and DVR APIs

/**
 * @brief CameraManager - Manages camera hardware lifecycle
 *
 * Responsibilities:
 * - Detect connected cameras via DVR_CheckCameraConnected()
 * - Initialize cameras on-demand via DVR_InitSingleCameraByType()
 * - Track camera initialization state
 * - Provide connection status query for other components
 *
 * Design:
 * - Cameras are NOT initialized at startup (on-demand initialization)
 * - Uses dvr.h public APIs only (no internal APIs)
 * - Caches connection status to avoid repeated API calls
 */
class CameraManager : public QObject
{
    Q_OBJECT

public:
    explicit CameraManager(QObject *parent = nullptr);
    ~CameraManager();

    /**
     * @brief Initialize camera manager
     * - Calls DVR_RescanDevices() to detect cameras
     * - Updates initial connection status cache
     * - Does NOT initialize cameras (deferred until needed)
     * @return true if initialization succeeds
     */
    bool initialize();

    /**
     * @brief Shutdown camera manager
     * - Clears cached state
     */
    void shutdown();

    /**
     * @brief Check if camera is physically connected
     * - Uses cached status (updated by initialize() and rescanDevices())
     * @param camera Camera type (FRONT or REAR)
     * @return true if camera is connected
     */
    bool isCameraConnected(DVR_CAM_TYPE_E camera) const;

    /**
     * @brief Rescan devices to update connection status
     * - Calls DVR_RescanDevices()
     * - Updates cached connection status
     * - Emits cameraAvailabilityChanged if status changes
     * @return true if scan succeeds
     */
    bool rescanDevices();

signals:
    /**
     * @brief Emitted when camera connection status changes
     * @param camera Camera type (FRONT or REAR)
     * @param available true if camera is now connected
     */
    void cameraAvailabilityChanged(DVR_CAM_TYPE_E camera, bool available);

    /**
     * @brief Emitted when camera error occurs
     * @param camera Camera type (FRONT or REAR)
     * @param message Error description
     */
    void cameraError(DVR_CAM_TYPE_E camera, const QString &message);

private:
    /**
     * @brief Update cached connection status from DVR library
     * - Calls DVR_CheckCameraConnected() for each camera
     * - Emits signals if status changed
     */
    void updateCameraStatus();

    // Initialization state
    bool m_initialized;

    // Cached connection status (updated by rescan)
    bool m_frontCameraConnected;
    bool m_rearCameraConnected;
};

#endif // CAMERAMANAGER_H
