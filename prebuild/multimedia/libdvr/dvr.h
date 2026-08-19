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
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */


/*******************************************************************************
*
* Filename:
* ---------
* file dvr.h
*
* Project:
* --------
*   CNB
*
* Description:
* ------------
*

*
*------------------------------------------------------------------------------
* $Revision: #1 $
* $Modtime:$
* $Log:$
*
*******************************************************************************/


#ifndef __DVR_H__
#define __DVR_H__

#include <linux/types.h>
#include <windows.h> //RECT
#include "dvr_types.h"
#include "dvr_mediascanner.h"

#include "atcsurface.h"
#if 0
#include "android_runtime/AndroidRuntime.h"
#include "android_runtime/android_view_Surface.h"

#include <gui/ISurfaceComposerClient.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/Surface.h>
#include <gui/ISurfaceTexture.h>
using namespace android;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define DVR_UI_MSG_START            0x100
#define ADAS_ONLY

// Follow Message Send to UI from Native lib.
typedef enum
{
    DVR_UI_MSG_S = DVR_UI_MSG_START,

    DVR_UI_MSG_INIT_FAILED,
    DVR_UI_MSG_DEINIT_FAILED,

    DVR_UI_MSG_REC_STARTED,
    DVR_UI_MSG_REC_STOPED,
    DVR_UI_MSG_START_REC_FAILED,
    DVR_UI_MSG_STOP_REC_FAILED,

    DVR_UI_MSG_PREV_STARTED,
    DVR_UI_MSG_PREV_STOPED,
    DVR_UI_MSG_START_PREV_FAILED,
    DVR_UI_MSG_STOP_PREV_FAILED,

    DVR_UI_MSG_SNAPSHOT_END,

    DVR_UI_MSG_SD_FULL,
    DVR_UI_MSG_URGENT_FILE,
    DVR_UI_MSG_FORMAT_INVALID,
    DVR_UI_MSG_CAM_ERROR,


    DVR_UI_MSG_NUM,
}DVR_MSG_T;


typedef enum
{
    E_DVR_SINK_TYPE_NONE = 0,
    E_DVR_SINK_TYPE_FRONT,
    E_DVR_SINK_TYPE_REAR,
    E_DVR_SINK_TYPE_FRONT_REAR,
}DVR_SINK_TYPE;

typedef struct
{
    DVR_SINK_TYPE eSinkType;
    RECT rDestRect;
    bool fgShow;
}DVR_SINK_INFO_T;

typedef struct
{
    __u16 u2Year;
    __u8 uMonth;
    __u8 uDay;
    __u8 uHour;
    __u8 uMinute;
    __u8 uSecond;
}DVR_TIME_T;

typedef struct
{
    __s16 i2Degree;
    __u8 bMinute;
    __u8 bSecond;
}DVR_OSD_COORDINATE_POS_T;

typedef struct
{
    DVR_OSD_COORDINATE_POS_T rLatitudePos;
    DVR_OSD_COORDINATE_POS_T rLongitudePos;
}DVR_OSD_COORDINATE_T;

typedef void (*AttachCurrentThread)(void);
typedef void (*DVR_callback)(__u32 u4Evt, __u32 u4Param1, __u32 u4Param2);
typedef void (*DetachCurrentThread)(void);


/**
*   @brief This method initializes DVR.
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
*   @note    must be called before DVR api used
*
**/
bool  DVR_Init(AttachCurrentThread AttachFunc, DVR_callback callback, DetachCurrentThread DetachFunc, DVR_VIDEO_INFO_T *prVideoInfo);

/**
*   @brief This method checks if DVR is fully initialized
*
*   @return  TRUE if DVR is initialized and ready, FALSE otherwise
*
*   @note   Use this to ensure DVR initialization is complete before starting preview
**/
bool  DVR_IsInitialized();

/**
*   @brief This method deinitializes DVR.
*
*   @return  TRUE indicate success, FALSE indicate Fail.
*
*   @note   must be called after DVR exit,sure that all resource be released
**/
bool  DVR_DeInit();


/**
*   @brief This method start DVR Preview
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_StartPreview();


/**
*   @brief This method stop DVR Preview
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_StopPreview();

/**
*   @brief This method start DVR Record
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_StartRecord();

/**
*   @brief This method stop DVR Record
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_StopRecord();

/**
*   @brief This method set DVR Record Path
*
*   @param[in]  pszRootPath: Path Full Name
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_SetRecordPath(TCHAR* pszRootPath);

/**
*   @brief This method set DVR Record Duration
*
*   @param[in]  u4Duration: Max Duration each Record File.(UINT: Second)
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_SetRecordDuration(__u32 u4Duration);

/**
*   @brief This method set DVR Record Capacity
*
*   @param[in]  u4NormalCapacity: Max Normal Video Capacity of total Record File.(in MByte)
*   @param[in]  u4UrgentCapacity: Max Urgent Video capacity of total Record File.(in MByte)
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_SetRecordCapacity(__u32 u4NormalCapacity, __u32 u4UrgentCapacity);

/**
*   @brief This method set DVR Video Info for Record/Preview
*
*   @param[in]  DVR_VIDEO_INFO_T: Video Format Info(such as Video Width & Height)
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_SetVideoInfo(DVR_VIDEO_INFO_T* prVidInfo);

/**
*   @brief This method set DVR Video Info for Snapshot
*
*   @param[in]  DVR_VIDEO_INFO_T: Video Format Info(such as Video Width & Height)
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool DVR_SnapShot(DVR_VIDEO_INFO_T* prVidInfo, __u32 num);
bool DVR_SetSnapShotVidInfo(DVR_VIDEO_INFO_T* prVidInfo);
/**
*   @brief This method set Preview Video Dest Rect,or Show/Hide Video
*
*   @param[in]  prSinkInfo: see DVR_SINK_INFO_T
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_SetSinkInfo(DVR_SINK_INFO_T* prSinkInfo);

/**
*   @brief This method set DVR Preview Dest Output
*
*   @param[in]  eSinkType: see DVR_SINK_TYPE
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_SetDestination(DVR_SINK_TYPE eSinkType);

/**
*   @brief This method set DVR Local Time.
*
*   @param[in]  prTimeInfo: see DVR_TIME_T
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_SetTime(DVR_TIME_T* prTimeInfo);


/**
*   @brief This method set GPS Coordinate(this will show in video OSD).
*
*   @param[in]  prCoordinate: see DVR_OSD_COORDINATE_T
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_SetGpsCoordinate(DVR_OSD_COORDINATE_T* prCoordinate);

/**
*   @brief This method set GPS Speed(this will show in video OSD)
*
*   @param[in]  u2Speed  GPS Speed(KM)
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_SetGpsSpeed(__u16 u2Speed);


bool DVR_SetAudioOn(bool fgOn);

void DVR_StorageMount(bool fgMount);

bool DVR_Urgent(void);

void DVR_SetSurface(IAtcSurface* pAtcSurface);

// ============================================================================
// Device Management APIs
// ============================================================================

/**
*   @brief This method rescan devices and update device status
*
*   @return TRUE indicate success, FALSE indicate Fail.
*   @note This API scans for available cameras and updates device manager state
*
**/
bool DVR_RescanDevices(void);

/**
*   @brief This method check if specific camera is connected
*
*   @param[in]  eCamType: Camera type (DVR_CAM_TYPE_FRONT/DVR_CAM_TYPE_REAR)
*   @return TRUE if camera is connected, FALSE otherwise
*
**/
bool DVR_CheckCameraConnected(DVR_CAM_TYPE_E eCamType);

/**
*   @brief This method get total number of connected cameras
*
*   @return Number of connected cameras
*
**/
int DVR_GetConnectedCameraCount(void);


bool DVRAV_RawData_Init(void);
void DVRAV_RawData_Deinit(void);
#if 0
bool DVRAV_RawData_Start(HANDLE hInst, const sp<ISurfaceTexture> &surface);
#endif
bool DVRAV_RawData_Stop(HANDLE hInst);

bool DVRAV_RawData_GetBuffer(HANDLE hInst, __u8 **ppData, __u32 * pu4Size);

// ============================================================================
// Dual Camera Independent Control APIs
// ============================================================================

/**
*   @brief This method initialize dual camera system
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_InitDualCamera();

/**
*   @brief This method initialize specific camera dynamically
*
*   @param[in] eCamType camera type to initialize
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_InitSingleCameraByType(DVR_CAM_TYPE_E eCamType);

/**
*   @brief Check if a specific camera type is already initialized
*
*   @param[in] eCamType camera type to check
*   @return TRUE if camera is initialized, FALSE otherwise
*
**/
bool  DVR_IsCameraInitialized(DVR_CAM_TYPE_E eCamType);

/**
*   @brief This method deinitialize dual camera system
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_DeinitDualCamera();

// ----------------------------------------------------------------------------
// Independent Preview Control for Each Camera
// ----------------------------------------------------------------------------

/**
*   @brief This method start preview for specific camera
*
*   @param[in]  eCamType: Camera type (DVR_CAM_TYPE_FRONT/DVR_CAM_TYPE_REAR)
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_StartPreviewByCamera(DVR_CAM_TYPE_E eCamType);

/**
*   @brief This method stop preview for specific camera
*
*   @param[in]  eCamType: Camera type (DVR_CAM_TYPE_FRONT/DVR_CAM_TYPE_REAR)
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_StopPreviewByCamera(DVR_CAM_TYPE_E eCamType);

/**
*   @brief This method check if preview is active for specific camera
*
*   @param[in]  eCamType: Camera type (DVR_CAM_TYPE_FRONT/DVR_CAM_TYPE_REAR)
*   @return TRUE if preview is active, FALSE otherwise
*
**/
bool  DVR_IsPreviewActive(DVR_CAM_TYPE_E eCamType);

/**
*   @brief This method set preview surface for specific camera
*
*   @param[in]  eCamType: Camera type (DVR_CAM_TYPE_FRONT/DVR_CAM_TYPE_REAR)
*   @param[in]  pSurface: Pointer to IAtcSurface for preview display
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_SetPreviewSurface(DVR_CAM_TYPE_E eCamType, IAtcSurface* pSurface);

// ----------------------------------------------------------------------------
// Independent Recording Control for Each Camera
// ----------------------------------------------------------------------------

/**
*   @brief This method start recording for specific camera
*
*   @param[in]  eCamType: Camera type (DVR_CAM_TYPE_FRONT/DVR_CAM_TYPE_REAR)
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_StartRecordByCamera(DVR_CAM_TYPE_E eCamType);

/**
*   @brief This method stop recording for specific camera
*
*   @param[in]  eCamType: Camera type (DVR_CAM_TYPE_FRONT/DVR_CAM_TYPE_REAR)
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_StopRecordByCamera(DVR_CAM_TYPE_E eCamType);

/**
*   @brief This method check if recording is active for specific camera
*
*   @param[in]  eCamType: Camera type (DVR_CAM_TYPE_FRONT/DVR_CAM_TYPE_REAR)
*   @return TRUE if recording is active, FALSE otherwise
*
**/
bool  DVR_IsRecordingActive(DVR_CAM_TYPE_E eCamType);

// ----------------------------------------------------------------------------
// Camera Configuration and Control
// ----------------------------------------------------------------------------

/**
*   @brief This method set video info for specific camera
*
*   @param[in]  eCamType: Camera type (DVR_CAM_TYPE_FRONT/DVR_CAM_TYPE_REAR)
*   @param[in]  prVidInfo: Video format information
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_SetVideoInfoByCamera(DVR_CAM_TYPE_E eCamType, DVR_VIDEO_INFO_T* prVidInfo);

/**
*   @brief This method set recording path for specific camera
*
*   @param[in]  eCamType: Camera type (DVR_CAM_TYPE_FRONT/DVR_CAM_TYPE_REAR)
*   @param[in]  pszRootPath: Recording path
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_SetRecordPathByCamera(DVR_CAM_TYPE_E eCamType, TCHAR* pszRootPath);

/**
*   @brief This method take snapshot for specific camera
*
*   @param[in]  eCamType: Camera type (DVR_CAM_TYPE_FRONT/DVR_CAM_TYPE_REAR)
*   @param[in]  prVidInfo: Video info for snapshot
*   @param[in]  num: Number of snapshots
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_SnapShotByCamera(DVR_CAM_TYPE_E eCamType, DVR_VIDEO_INFO_T* prVidInfo, __u32 num);

// ----------------------------------------------------------------------------
// Convenience APIs for Dual Camera Operation
// ----------------------------------------------------------------------------

/**
*   @brief This method start preview for both cameras
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_StartDualPreview();

/**
*   @brief This method stop preview for both cameras
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_StopDualPreview();

/**
*   @brief This method start recording for both cameras
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_StartDualRecord();

/**
*   @brief This method stop recording for both cameras
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool  DVR_StopDualRecord();

// ----------------------------------------------------------------------------
// Legacy Compatibility APIs (maintained for backward compatibility)
// ----------------------------------------------------------------------------

/**
*   @brief This method switch between front and rear camera (legacy)
*
*   @return TRUE indicate success, FALSE indicate Fail.
*   @note This API is maintained for backward compatibility
*
**/
bool  DVR_SwitchCamera();

/**
*   @brief This method switch recording camera (legacy)
*
*   @return TRUE indicate success, FALSE indicate Fail.
*   @note This API is maintained for backward compatibility
*
**/
bool  DVR_SwitchRecordCamera();

/**
*   @brief This method get current active camera type (legacy)
*
*   @return DVR_CAM_TYPE_E Current camera type
*   @note This API is maintained for backward compatibility
*
**/
__u32  DVR_GetCurrentCamera();

/**
*   @brief This method get current recording camera type (legacy)
*
*   @return DVR_CAM_TYPE_E Current recording camera type
*   @note This API is maintained for backward compatibility
*
**/
__u32  DVR_GetRecordingCamera();

// ============================================================================
// Media Player APIs
// ============================================================================

typedef enum
{
    DVR_PLAYER_STATE_STOPPED = 0,
    DVR_PLAYER_STATE_PLAYING,
    DVR_PLAYER_STATE_PAUSED,
    DVR_PLAYER_STATE_ERROR
} DVR_PLAYER_STATE_E;

typedef enum
{
    DVR_PLAYER_MSG_PREPARED = 0,
    DVR_PLAYER_MSG_PLAYBACK_COMPLETE,
    DVR_PLAYER_MSG_ERROR,
    DVR_PLAYER_MSG_SEEK_COMPLETE,
    DVR_PLAYER_MSG_PROGRESS_UPDATE
} DVR_PLAYER_MSG_E;

typedef struct
{
    __u32 u4Position;      // Current position in milliseconds
    __u32 u4Duration;      // Total duration in milliseconds
} DVR_PLAYER_PROGRESS_T;

typedef void (*DVR_PlayerCallback)(__u32 u4MsgType, __u32 u4Param1, __u32 u4Param2, void* pvUserData);

/**
*   @brief This method initialize media player
*
*   @param[in]  callback: Callback function for player events
*   @param[in]  pvUserData: User data passed to callback
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool DVR_PlayerInit(DVR_PlayerCallback callback, void* pvUserData);

/**
*   @brief This method deinitialize media player
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool DVR_PlayerDeinit(void);

/**
*   @brief This method setup video surface for player
*
*   @param[in]  u4Width: Surface width
*   @param[in]  u4Height: Surface height
*   @return TRUE indicate success, FALSE indicate Fail.
*   @note IAtcSurface is created internally by MediaPlayer
*
**/
bool DVR_PlayerSetSurface(__u32 u4Width, __u32 u4Height, __u32 u4X = 0, __u32 u4Y = 0);

/**
*   @brief This method load and prepare media file for playback
*
*   @param[in]  pszFilePath: Full path to media file
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool DVR_PlayerPrepare(const char* pszFilePath);

/**
*   @brief This method start or resume playback
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool DVR_PlayerPlay(void);

/**
*   @brief This method pause playback
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool DVR_PlayerPause(void);

/**
*   @brief This method stop playback
*
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool DVR_PlayerStop(void);

/**
*   @brief This method seek to specific position
*
*   @param[in]  u4PositionMs: Target position in milliseconds
*   @return TRUE indicate success, FALSE indicate Fail.
*
**/
bool DVR_PlayerSeek(__u32 u4PositionMs);

/**
*   @brief This method get current playback position
*
*   @return Current position in milliseconds, 0 if error
*
**/
__u32 DVR_PlayerGetPosition(void);

/**
*   @brief This method get total duration of current media
*
*   @return Total duration in milliseconds, 0 if error or unknown
*
**/
__u32 DVR_PlayerGetDuration(void);

/**
*   @brief This method get current player state
*
*   @return DVR_PLAYER_STATE_E Current player state
*
**/
DVR_PLAYER_STATE_E DVR_PlayerGetState(void);

/**
*   @brief This method check if player is ready for playback
*
*   @return TRUE if player is prepared and ready, FALSE otherwise
*
**/
bool DVR_PlayerIsReady(void);

#ifdef __cplusplus
}
#endif

#endif //_MSDK_AVIN_H_
