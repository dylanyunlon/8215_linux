/*
 * Copyright (c) 2025 AutoChips Inc.
 *
 * FileManager - Video file scanning and management implementation
 */

#include "filemanager.h"
#include "recordmanager.h"
#include "dvrlog.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStorageInfo>

// Initialize static member
constexpr const char* FileManager::TAG;

// Static callback for DVR MediaScanner (called from DVR library thread)
static void mediaScannerCallback(const char* path, DVR_SCAN_STATE_E state,
                                 __u32 fileCount, void* userContext)
{
    // We use timer polling instead of callback, so this is just a placeholder
    Q_UNUSED(path);
    Q_UNUSED(state);
    Q_UNUSED(fileCount);
    Q_UNUSED(userContext);
}

FileManager::FileManager(RecordManager *recordManager, QObject *parent)
    : QObject(parent)
    , m_recordManager(recordManager)
    , m_scanning(false)
    , m_mediaScannerEnabled(false)
{
    // Create timer for scan progress polling
    m_scanTimer = new QTimer(this);
    m_scanTimer->setInterval(500);  // Poll every 500ms
    connect(m_scanTimer, &QTimer::timeout, this, &FileManager::onScanProgressUpdate);
    
    // Initialize DVR MediaScanner
    if (DVR_MediaScanner_Init()) {
        m_mediaScannerEnabled = true;
        LOGI(TAG, "DVR MediaScanner initialized successfully");
    } else {
        m_mediaScannerEnabled = false;
        LOGW(TAG, "DVR MediaScanner initialization failed, file scanning disabled");
    }
    
    LOGI(TAG, "FileManager created");
}

FileManager::~FileManager()
{
    // Stop timer if running
    if (m_scanTimer->isActive()) {
        m_scanTimer->stop();
    }
    
    // Deinitialize MediaScanner
    if (m_mediaScannerEnabled) {
        DVR_MediaScanner_Deinit();
        LOGI(TAG, "DVR MediaScanner deinitialized");
    }
    
    LOGI(TAG, "FileManager destroyed");
}

void FileManager::setRecordPath(const QString &path)
{
    if (m_recordPath != path) {
        m_recordPath = path;
        LOGI(TAG, "Record path set to: %s", qPrintable(path));
    }
}

void FileManager::requestFileList()
{
    LOGI(TAG, "File list requested");
    
    // Check if already scanning
    if (m_scanning) {
        LOGW(TAG, "Scan already in progress, ignoring request");
        return;
    }
    
    // Check if MediaScanner is available
    if (!m_mediaScannerEnabled) {
        LOGE(TAG, "MediaScanner not available");
        emit fileListError("File scanner not available");
        return;
    }
    
    // Re-fetch record path from RecordManager (triggers auto-detection if SD card changed)
    if (m_recordManager) {
        QString latestPath = m_recordManager->getRecordPath();
        if (latestPath != m_recordPath) {
            LOGI(TAG, "[SD_REFRESH] Record path updated: '%s' -> '%s'", 
                 qPrintable(m_recordPath), qPrintable(latestPath));
            m_recordPath = latestPath;
        }
    }
    
    // Check if record path is set
    if (m_recordPath.isEmpty()) {
        LOGE(TAG, "Record path is empty (SD card not mounted)");
        emit fileListError("SD card not inserted or not mounted");
        return;
    }
    
    // Additional check: Verify SD card is actually mounted (not tmpfs)
    QStorageInfo storage(m_recordPath);
    if (storage.isValid() && storage.fileSystemType() == "tmpfs") {
        LOGW(TAG, "Record path is on tmpfs, SD card not properly mounted");
        emit fileListError("SD card not properly mounted");
        return;
    }
    
    // RecordManager already returns the full path including /normal/ subdirectory
    LOGI(TAG, "Scanning video files in: %s", qPrintable(m_recordPath));
    
    // Check if directory exists
    QDir dir(m_recordPath);
    if (!dir.exists()) {
        LOGW(TAG, "Recording directory does not exist: %s", qPrintable(m_recordPath));
        // Return empty list instead of error (directory will be created when recording)
        emit fileListUpdated(QVariantList());
        return;
    }
    
    // Start async scan
    QByteArray pathBytes = m_recordPath.toUtf8();
    const char* path = pathBytes.constData();
    bool ret = DVR_MediaScanner_ScanDirectory(path, mediaScannerCallback, nullptr);
    
    if (!ret) {
        LOGE(TAG, "Failed to start directory scan");
        emit fileListError("Failed to start file scan");
        return;
    }
    
    // Start polling timer
    m_scanning = true;
    m_scanTimer->start();
    LOGI(TAG, "Directory scan started: %s", path);
}

void FileManager::onScanProgressUpdate()
{
    if (!m_scanning) {
        return;
    }
    
    // Get scan state (m_recordPath already includes /normal/)
    QByteArray pathBytes = m_recordPath.toUtf8();
    const char* path = pathBytes.constData();
    DVR_SCAN_STATE_E scanState = DVR_MediaScanner_GetScanState(path);
    
    switch (scanState) {
        case DVR_SCAN_STATE_IDLE:
            LOGW(TAG, "Scan state is IDLE (unexpected)");
            m_scanTimer->stop();
            m_scanning = false;
            emit fileListError("Scan state error");
            break;
            
        case DVR_SCAN_STATE_SCANNING:
            // Still scanning, keep polling
            LOGD(TAG, "Scan in progress...");
            break;
            
        case DVR_SCAN_STATE_COMPLETED:
            // Scan completed successfully
            LOGI(TAG, "Scan completed");
            m_scanTimer->stop();
            m_scanning = false;
            processScanResults();
            break;
            
        case DVR_SCAN_STATE_ERROR:
            // Scan failed
            LOGE(TAG, "Scan failed with error state");
            m_scanTimer->stop();
            m_scanning = false;
            emit fileListError("File scan failed");
            break;
            
        default:
            LOGW(TAG, "Unknown scan state: %d", scanState);
            break;
    }
}

void FileManager::processScanResults()
{
    // Build scan path (base + /normal)
    // Query scanned video files (m_recordPath already includes /normal/)
    QByteArray pathBytes = m_recordPath.toUtf8();
    const char* path = pathBytes.constData();
    
    // Get video count
    __u32 count = DVR_MediaScanner_GetVideoCount(path);
    LOGI(TAG, "Found %u video files in %s", count, path);
    
    if (count <= 0) {
        // No files found, return empty list
        emit fileListUpdated(QVariantList());
        return;
    }
    
    // Allocate buffer for video list
    DVR_VIDEO_FILE_INFO_T* videoList = new DVR_VIDEO_FILE_INFO_T[count];
    __u32 actualCount = 0;
    
    // Get video list
    bool ret = DVR_MediaScanner_GetVideoList(path, videoList, count, &actualCount);
    if (!ret) {
        LOGE(TAG, "Failed to get video list");
        delete[] videoList;
        emit fileListError("Failed to retrieve file list");
        return;
    }
    
    // Convert to QVariantList
    QVariantList fileList;
    for (__u32 i = 0; i < actualCount; i++) {
        QVariantMap fileInfo = createFileInfo(videoList[i]);
        fileList.append(fileInfo);
    }
    
    // Clean up
    delete[] videoList;
    
    LOGI(TAG, "Processed %u video files", actualCount);
    emit fileListUpdated(fileList);
}

QVariantMap FileManager::createFileInfo(const DVR_VIDEO_FILE_INFO_T &videoInfo)
{
    QVariantMap fileInfo;
    
    // Filename
    QString filename = QString::fromUtf8(videoInfo.filename);
    fileInfo["filename"] = filename;
    
    // Full path (m_recordPath already includes /normal/, just append filename)
    QString filePath = m_recordPath;
    if (!filePath.endsWith('/')) {
        filePath += '/';
    }
    filePath += filename;
    fileInfo["filePath"] = filePath;
    
    // Duration (HH:MM:SS format)
    QString durationStr = QString::fromUtf8(videoInfo.duration);
    fileInfo["duration"] = durationStr;
    
    // File size (bytes)
    fileInfo["fileSize"] = static_cast<qint64>(videoInfo.fileSize);
    
    // Modification time (timestamp)
    fileInfo["modifyTime"] = static_cast<qint64>(videoInfo.modifyTime);
    
    // Camera type (parsed from filename pattern: *_F or *_R)
    if (filename.contains("_F.")) {
        fileInfo["cameraType"] = "Front";
        fileInfo["cameraLabel"] = "F";
        fileInfo["cameraColor"] = "#c80096ff";  // Blue for front (rgba 0,150,255,200)
    } else if (filename.contains("_R.")) {
        fileInfo["cameraType"] = "Rear";
        fileInfo["cameraLabel"] = "R";
        fileInfo["cameraColor"] = "#c8ff6400";  // Orange for rear (rgba 255,100,0,200)
    } else {
        fileInfo["cameraType"] = "Unknown";
        fileInfo["cameraLabel"] = "?";
        fileInfo["cameraColor"] = "#c8646464";  // Gray for unknown (rgba 100,100,100,200)
    }
    
    LOGD(TAG, "File: %s, Duration: %s, Size: %lld bytes, Camera: %s",
         qPrintable(filename),
         videoInfo.duration,
         videoInfo.fileSize,
         qPrintable(fileInfo["cameraType"].toString()));
    
    return fileInfo;
}

bool FileManager::deleteFile(const QString &filePath)
{
    LOGI(TAG, "Delete requested: %s", qPrintable(filePath));
    
    QFile file(filePath);
    if (!file.exists()) {
        LOGW(TAG, "File does not exist: %s", qPrintable(filePath));
        emit fileDeleted(filePath, false);
        return false;
    }
    
    // Delete file
    bool success = file.remove();
    
    if (success) {
        LOGI(TAG, "File deleted successfully");
        
        // Update MediaScanner database
        if (m_mediaScannerEnabled) {
            QFileInfo fileInfo(filePath);
            QString filename = fileInfo.fileName();
            
            // Use m_recordPath directly (already includes /normal/)
            QByteArray pathBytes = m_recordPath.toUtf8();
            QByteArray nameBytes = filename.toUtf8();
            const char* path = pathBytes.constData();
            const char* fname = nameBytes.constData();
            
            // Remove from MediaScanner database
            // Note: This is incremental update, not full rescan
            DVR_MediaScanner_RemoveVideoFiles(path, &fname, 1);
            LOGI(TAG, "MediaScanner database updated");
        }
        
        emit fileDeleted(filePath, true);
    } else {
        LOGE(TAG, "Failed to delete file: %s", qPrintable(file.errorString()));
        emit fileDeleted(filePath, false);
    }
    
    return success;
}
