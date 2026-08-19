/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 */

#ifndef __DVR_MEDIASCANNER_H__
#define __DVR_MEDIASCANNER_H__

#include <linux/types.h>
#include "dvr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// DVR Media Scanner Types and Structures
// ============================================================================

/**
 * @brief Video file information structure
 */
typedef struct {
    char filename[256];     // Video file name
    char duration[16];      // Duration in HH:MM:SS format
    __u64 fileSize;         // File size in bytes
    __u64 modifyTime;       // Last modification time
} DVR_VIDEO_FILE_INFO_T;

/**
 * @brief Media scanner states
 */
typedef enum {
    DVR_SCAN_STATE_IDLE = 0,        // Scanner is idle
    DVR_SCAN_STATE_SCANNING,        // Currently scanning
    DVR_SCAN_STATE_COMPLETED,       // Scan completed successfully
    DVR_SCAN_STATE_ERROR            // Error occurred during scanning
} DVR_SCAN_STATE_E;

/**
 * @brief Media scanner callback function type
 * @param[in] path          Directory path being scanned
 * @param[in] state         Current scanner state
 * @param[in] fileCount     Number of files found (valid when state is SCANNING or COMPLETED)
 * @param[in] userContext   User-provided context pointer
 */
typedef void (*DVR_MediaScanner_Callback)(const char* path, DVR_SCAN_STATE_E state, 
                                         __u32 fileCount, void* userContext);

// ============================================================================
// DVR Media Scanner APIs
// ============================================================================

/**
 * @brief Initialize DVR media scanner
 * 
 * @return TRUE indicate success, FALSE indicate Fail
 * @note Must be called before using any other media scanner APIs
 */
bool DVR_MediaScanner_Init(void);

/**
 * @brief Deinitialize DVR media scanner
 * 
 * @return TRUE indicate success, FALSE indicate Fail
 * @note Should be called when media scanner is no longer needed
 */
bool DVR_MediaScanner_Deinit(void);

/**
 * @brief Start asynchronous directory scan
 * 
 * @param[in] directoryPath     Path to directory to scan
 * @param[in] callback          Optional callback for scan progress (can be NULL)
 * @param[in] userContext       User context passed to callback (can be NULL)
 * @return TRUE indicate success, FALSE indicate Fail
 * @note Scan is performed asynchronously. Use DVR_MediaScanner_GetScanState to check progress
 */
bool DVR_MediaScanner_ScanDirectory(const char* directoryPath, 
                                   DVR_MediaScanner_Callback callback,
                                   void* userContext);

/**
 * @brief Get current scan state for a directory
 * 
 * @param[in] directoryPath     Path to directory
 * @return DVR_SCAN_STATE_E     Current scan state
 */
DVR_SCAN_STATE_E DVR_MediaScanner_GetScanState(const char* directoryPath);

/**
 * @brief Get scanned video file list for a directory
 * 
 * @param[in] directoryPath     Path to directory
 * @param[out] fileList         Array to store video file information
 * @param[in] maxFiles          Maximum number of files to retrieve
 * @param[out] actualFiles      Actual number of files retrieved
 * @return TRUE indicate success, FALSE indicate Fail
 * @note fileList must be allocated by caller with size maxFiles * sizeof(DVR_VIDEO_FILE_INFO_T)
 */
bool DVR_MediaScanner_GetVideoList(const char* directoryPath,
                                  DVR_VIDEO_FILE_INFO_T* fileList,
                                  __u32 maxFiles,
                                  __u32* actualFiles);

/**
 * @brief Get total number of video files in scanned directory
 * 
 * @param[in] directoryPath     Path to directory
 * @return Number of video files found, 0 if directory not scanned or error
 */
__u32 DVR_MediaScanner_GetVideoCount(const char* directoryPath);

/**
 * @brief Add new video files to scanner database
 * 
 * @param[in] directoryPath     Path to directory
 * @param[in] fileNames         Array of file names to add
 * @param[in] fileCount         Number of files to add
 * @return TRUE indicate success, FALSE indicate Fail
 * @note This is a synchronous operation
 */
bool DVR_MediaScanner_AddVideoFiles(const char* directoryPath,
                                   const char** fileNames,
                                   __u32 fileCount);

/**
 * @brief Remove video files from scanner database
 * 
 * @param[in] directoryPath     Path to directory
 * @param[in] fileNames         Array of file names to remove
 * @param[in] fileCount         Number of files to remove
 * @return TRUE indicate success, FALSE indicate Fail
 * @note This is a synchronous operation
 */
bool DVR_MediaScanner_RemoveVideoFiles(const char* directoryPath,
                                      const char** fileNames,
                                      __u32 fileCount);

/**
 * @brief Get video file duration
 * 
 * @param[in] filePath          Full path to video file
 * @param[out] duration         Buffer to store duration string (HH:MM:SS format)
 * @param[in] bufferSize        Size of duration buffer (should be at least 16 bytes)
 * @return TRUE indicate success, FALSE indicate Fail
 */
bool DVR_MediaScanner_GetVideoDuration(const char* filePath,
                                      char* duration,
                                      __u32 bufferSize);

/**
 * @brief Check if a file is a supported video format
 * 
 * @param[in] filename          File name to check
 * @return TRUE if supported video format, FALSE otherwise
 */
bool DVR_MediaScanner_IsVideoFile(const char* filename);

/**
 * @brief Refresh scanner database for a directory
 * 
 * @param[in] directoryPath     Path to directory to refresh
 * @return TRUE indicate success, FALSE indicate Fail
 * @note This will trigger a new scan even if directory was previously scanned
 */
bool DVR_MediaScanner_RefreshDirectory(const char* directoryPath);

// ============================================================================
// Convenience APIs for DVR Integration
// ============================================================================

/**
 * @brief Scan default DVR recording directories
 * 
 * @param[in] callback          Optional callback for scan progress (can be NULL)
 * @param[in] userContext       User context passed to callback (can be NULL)
 * @return TRUE indicate success, FALSE indicate Fail
 * @note Scans both normal and urgent recording directories
 */
bool DVR_MediaScanner_ScanRecordingDirectories(DVR_MediaScanner_Callback callback,
                                              void* userContext);

/**
 * @brief Get video files from default normal recording directory
 * 
 * @param[out] fileList         Array to store video file information
 * @param[in] maxFiles          Maximum number of files to retrieve
 * @param[out] actualFiles      Actual number of files retrieved
 * @return TRUE indicate success, FALSE indicate Fail
 */
bool DVR_MediaScanner_GetNormalRecordingFiles(DVR_VIDEO_FILE_INFO_T* fileList,
                                             __u32 maxFiles,
                                             __u32* actualFiles);

/**
 * @brief Get video files from default urgent recording directory
 * 
 * @param[out] fileList         Array to store video file information
 * @param[in] maxFiles          Maximum number of files to retrieve
 * @param[out] actualFiles      Actual number of files retrieved
 * @return TRUE indicate success, FALSE indicate Fail
 */
bool DVR_MediaScanner_GetUrgentRecordingFiles(DVR_VIDEO_FILE_INFO_T* fileList,
                                             __u32 maxFiles,
                                             __u32* actualFiles);

#ifdef __cplusplus
}
#endif

#endif // __DVR_MEDIASCANNER_H__
