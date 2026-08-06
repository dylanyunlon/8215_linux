/*
 * Copyright (c) 2025 AutoChips Inc.
 *
 * CameraManager - Camera hardware lifecycle management implementation
 */

#include "cameramanager.h"
#include "dvrlog.h"
#include <QDebug>

static const char* TAG = "CameraManager";

CameraManager::CameraManager(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
    , m_frontCameraConnected(false)
    , m_rearCameraConnected(false)
{
    LOGI(TAG, "Created");
}

CameraManager::~CameraManager()
{
    LOGI(TAG, "Destroying");
    shutdown();
}

bool CameraManager::initialize()
{
    if (m_initialized) {
        LOGW(TAG, "Already initialized");
        return true;
    }

    LOGI(TAG, "Initializing...");

    // Scan for connected devices (dvr.h API)
    if (!DVR_RescanDevices()) {
        LOGE(TAG, "DVR_RescanDevices() failed");
        return false;
    }

    // Update cached connection status
    updateCameraStatus();

    m_initialized = true;
    LOGI(TAG, "Initialization complete - Front: %s, Rear: %s",
         m_frontCameraConnected ? "Connected" : "Not connected",
         m_rearCameraConnected ? "Connected" : "Not connected");

    return true;
}

void CameraManager::shutdown()
{
    if (!m_initialized) {
        return;
    }

    LOGI(TAG, "Shutting down");

    // Clear cached state
    m_frontCameraConnected = false;
    m_rearCameraConnected = false;
    m_initialized = false;

    LOGI(TAG, "Shutdown complete");
}

bool CameraManager::isCameraConnected(DVR_CAM_TYPE_E camera) const
{
    if (!m_initialized) {
        LOGW(TAG, "Not initialized, returning false");
        return false;
    }

    switch (camera) {
    case DVR_CAM_TYPE_FRONT:
        return m_frontCameraConnected;
    case DVR_CAM_TYPE_REAR:
        return m_rearCameraConnected;
    default:
        LOGW(TAG, "Invalid camera type: %d", camera);
        return false;
    }
}

bool CameraManager::rescanDevices()
{
    if (!m_initialized) {
        LOGW(TAG, "Not initialized");
        return false;
    }

    LOGI(TAG, "Rescanning devices...");

    // Store old status to detect changes
    bool oldFrontStatus = m_frontCameraConnected;
    bool oldRearStatus = m_rearCameraConnected;

    // Rescan devices (dvr.h API)
    if (!DVR_RescanDevices()) {
        LOGE(TAG, "DVR_RescanDevices() failed");
        return false;
    }

    // Update cached status
    updateCameraStatus();

    // Emit signals if status changed
    if (m_frontCameraConnected != oldFrontStatus) {
        LOGI(TAG, "Front camera status changed: %s",
             m_frontCameraConnected ? "Connected" : "Disconnected");
        emit cameraAvailabilityChanged(DVR_CAM_TYPE_FRONT, m_frontCameraConnected);
    }

    if (m_rearCameraConnected != oldRearStatus) {
        LOGI(TAG, "Rear camera status changed: %s",
             m_rearCameraConnected ? "Connected" : "Disconnected");
        emit cameraAvailabilityChanged(DVR_CAM_TYPE_REAR, m_rearCameraConnected);
    }

    return true;
}

void CameraManager::updateCameraStatus()
{
    // Query connection status from DVR library (dvr.h API)
    m_frontCameraConnected = DVR_CheckCameraConnected(DVR_CAM_TYPE_FRONT);
    m_rearCameraConnected = DVR_CheckCameraConnected(DVR_CAM_TYPE_REAR);

    LOGD(TAG, "Status updated - Front: %d, Rear: %d",
         m_frontCameraConnected, m_rearCameraConnected);
}
