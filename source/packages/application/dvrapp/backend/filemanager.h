/*
 * Copyright (c) 2025 AutoChips Inc.
 *
 * FileManager - Video file scanning and management
 * Uses DVR MediaScanner library for async file operations
 */

#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QTimer>
#include "dvr.h"  // DVR library APIs (includes MediaScanner)

// Forward declaration
class RecordManager;

/**
 * FileManager - File scanning and management backend
 * 
 * Responsibilities:
 * - Scan video files using DVR MediaScanner library
 * - Extract metadata (duration, size, camera type)
 * - Delete files and update MediaScanner database
 * - Provide file list to QML layer
 * 
 * Threading:
 * - DVR_MediaScanner_ScanDirectory() is asynchronous
 * - Timer polls scan progress every 500ms
 * - All signals are emitted on main thread
 */
class FileManager : public QObject
{
    Q_OBJECT
    
public:
    explicit FileManager(RecordManager *recordManager, QObject *parent = nullptr);
    ~FileManager();
    
    // Initialize with recording path
    void setRecordPath(const QString &path);
    
    // Get current recording path
    QString recordPath() const { return m_recordPath; }
    
public slots:
    /**
     * Request file list scan
     * Triggers async scan via DVR MediaScanner
     * Emits: fileListUpdated() on success, fileListError() on failure
     */
    void requestFileList();
    
    /**
     * Delete a video file
     * @param filePath - Full path to file
     * @return true if deletion initiated
     * Emits: fileDeleted(filePath, success)
     */
    bool deleteFile(const QString &filePath);
    
signals:
    /**
     * Emitted when file list is ready
     * @param fileList - Array of file info maps with keys:
     *   - filename (QString)
     *   - filePath (QString)
     *   - duration (QString) - HH:MM:SS format
     *   - fileSize (qint64) - bytes
     *   - modifyTime (qint64) - Unix timestamp
     *   - cameraType (QString) - "Front", "Rear", "Unknown"
     *   - cameraLabel (QString) - "F", "R", "?"
     *   - cameraColor (QString) - "rgba(...)" color string
     */
    void fileListUpdated(const QVariantList &fileList);
    
    /**
     * Emitted when file list scan fails
     * @param errorMsg - Human-readable error message
     */
    void fileListError(const QString &errorMsg);
    
    /**
     * Emitted when file deletion completes
     * @param filePath - Path to deleted file
     * @param success - true if deletion succeeded
     */
    void fileDeleted(const QString &filePath, bool success);
    
    // Legacy signal for compatibility (can be removed later)
    void fileListChanged();
    void fileError(const QString &message);
    
private slots:
    /**
     * Timer callback to poll scan progress
     * Called every 500ms while scanning
     */
    void onScanProgressUpdate();
    
private:
    /**
     * Process scan results and emit fileListUpdated
     * Called when scan state becomes SCANDONE
     */
    void processScanResults();
    
    /**
     * Create file info map from DVR_VIDEO_FILE_INFO_T
     * Extracts all metadata including camera type from filename
     */
    QVariantMap createFileInfo(const DVR_VIDEO_FILE_INFO_T &videoInfo);
    
    /**
     * Parse camera type from filename
     * Pattern: *_F.ts = Front, *_R.ts = Rear
     */
    QString parseCameraType(const QString &filename);
    
private:
    RecordManager *m_recordManager; // RecordManager reference for path updates
    QString m_recordPath;           // Recording directory path
    QTimer *m_scanTimer;            // Progress polling timer
    bool m_scanning;                // Scan in progress flag
    bool m_mediaScannerEnabled;     // MediaScanner available flag
    
    // Logging tag
    static constexpr const char* TAG = "[FileManager]";
};

#endif // FILEMANAGER_H
