/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#ifndef _DRV_IF_SYNCCTRL_H_
#define _DRV_IF_SYNCCTRL_H_


#define SYNC_NONIPB2IPB_SMOOTHCHANGE 1
#define SYNC_FRAMEACCURATE 1
#define SYNC_HIGH_BITRATE_FILE_PLAY 1
#define SYNC_TWOPATH_AVSYNC 1
#define SYNC_FRAMEACCURATE_EOS_ERRORHANDLE 0     // error handle for EOS arrived earlier than end pts, if 1, error handle by decoder, else error handle by syncctrl


typedef enum
{
  SYNCCTRL_PIPELINE_MAIN = 0,
  SYNCCTRL_PIPELINE_SUB
} SYNCCTRL_PIPELINE;

typedef enum
{
  SYNCCTRL_CMD_NONE = 0,
  SYNCCTRL_CMD_PLAY,
  SYNCCTRL_CMD_STOP,
  SYNCCTRL_CMD_PAUSE,
  SYNCCTRL_CMD_FF,
  SYNCCTRL_CMD_FR,
  SYNCCTRL_CMD_SF,
  SYNCCTRL_CMD_SR,
  SYNCCTRL_CMD_STEPF,
  SYNCCTRL_CMD_STEPR
} SYNCCTRL_PLAYBACK_CMD;

typedef enum
{
  SYNCCTRL_StcLoadMode_LoadFromValue = 0,
  SYNCCTRL_StcLoadMode_LoadFromVideoPts
} SYNCCTRL_StcLoadMode;

typedef enum
{
  SYNCCTRL_VDEC_LAST_DROP_PTS,
  SYNCCTRL_VDEC_LAST_DEC_PTS,
  SYNCCTRL_VDEC_AU_INFO
} SYNCCTRL_Vdec_Update_Type;

typedef struct
{
  UINT32 u4ValidCount;
  UINT32 u4TotalCount;
} SYNCCTRL_AU_INFO_T;

#define STC_A_EQUAL_TO_B            0
#define STC_A_GREATER_THEN_B    1
#define STC_B_GREATER_THEN_A    (-1)

/// This interface represents IID_SyncCtrl interface.
/// This interface contains SyncCtrl functions.
typedef struct _ISyncCtrl
{
/*! \name Interface for all components used
* @{
*/
  /// Register Component
  /// \param pvTag [IN] the ISyncCtrl private data
  /// \param ePipeline [IN] the Path the component belongs to
  /// \param u2CompType [IN] the component type you want to register to SyncCtrl
  /// \param u2CompId [IN] the component id you want to register to SyncCtrl
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4RegisterComponent)(void *pvTag, SYNCCTRL_PIPELINE ePipeline, UINT16 u2CompType, UINT16 u2CompId);

  /// Unregister Component
  /// \param pvTag [IN] the ISyncCtrl private data
  /// \param ePipeline [IN] the Path the component belongs to
  /// \param u2CompType [IN] the component type you want to unregister to SyncCtrl
  /// \param u2CompId [IN] the component id you want to unregister to SyncCtrl
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4UnRegisterComponent)(void *pvTag, SYNCCTRL_PIPELINE ePipeline, UINT16 u2CompType, UINT16 u2CompId);

  /// Get STC time
  /// \Param pvTag [IN] the user private data
  /// \Param pu8Time [OUT] the STC time
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4GetSTC)(void *pvTag, UINT64 *pu8Time);

  /// Print Log
  /// \Param pvTag [IN] the user private data
  /// \param ePipeline [IN] the Path the component belongs to
  /// \param u2CompType [IN] the component type you want to register to SyncCtrl
  /// \Param szMessage [IN] the log string
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4PrintLog)(void *pvTag, SYNCCTRL_PIPELINE ePipeline, UINT16 u2CompType, const CHAR *szMessage);


  /// STC add
  /// \Param u8Stc [IN] stc value
  /// \Param u8Value [IN] Add value
  /// \Param pu8NewStc [IN] result stc
  /// \return None
  void (*pvStcAdd)(UINT64 u8Stc, UINT64 u8Value, UINT64 *pu8NewStc);

  /// STC subtract
  /// \Param u8Stc [IN] stc value
  /// \Param u8Value [IN] subtract value
  /// \Param pu8NewStc [IN] result stc
  /// \return None
  void (*pvStcSub)(UINT64 u8Stc, UINT64 u8Value, UINT64 * pu8NewStc);

  /// STC compare
  /// \Param u8A [IN] stc A value
  /// \Param u8B [IN] stc B value
  /// \return stc compare result (STC_A_EQUAL_TO_B/STC_A_GREATER_THEN_B/STC_B_GREATER_THEN_A)
  INT32 (*pi4StcCompare)(UINT64 u8A, UINT64 u8B);


/*! @} */


/*! \name Interface for DCC/Splitter components used
* @{
*/
  /// Set STC
  /// \param pvTag [IN] the ISyncCtrl private data
  /// \param ePipeline [IN] the Path that the component belongs to
  /// \param eLoadMode [IN] the STC Load mode
  /// \param u8Time [IN] the time you want to set
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4SetSTC)(void *pvTag, SYNCCTRL_PIPELINE ePipeline, SYNCCTRL_StcLoadMode eLoadMode, UINT64 u8Time);

  /// Enable AV no Sync mode for browsable slideshow
  /// \param pvTag [IN] the ISyncCtrl private data
  /// \param fgEnable [IN] TRUE: AV no sync mode, FALSE: AV Sync mode
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4SetAVNoSync)(void *pvTag, BOOL fgEnable);

#if (SYNC_HIGH_BITRATE_FILE_PLAY)
  /// DCC fifo is ready
  /// \param pvTag [IN] the ISyncCtrl private data
  /// \param ePipeline [IN] the Path that the component belongs to
  /// \param fgReady [IN] TRUE: fifo is ready; FALSE: fifo is not ready
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4DccFifoReady)(void *pvTag, SYNCCTRL_PIPELINE ePipeline, BOOL fgReady);
#endif

  /// Check whether STC is Valid
  /// \param pvTag [IN] the ISyncCtrl private data
  /// \param pfgValid [OUT] TRUE: STC is valid, FALSE: STC is not valid
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4StcIsValid)(void *pvTag, BOOL *pfgValid);

/*! @} */

/*! \name Interface for ADec used
* @{
*/
  /// Audio is ready to perform eCmd and sends the playback mode to SyncCtrl
  /// \param pvTag [IN] the ISyncCtrl private data
  /// \param ePipeline [IN] the Path the component belongs to
  /// \param eCmd [IN] the playback command
  /// \param u8APts [IN] the first Audio PTS
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4AudioReadyTo)(void *pvTag, SYNCCTRL_PIPELINE ePipeline, SYNCCTRL_PLAYBACK_CMD eCmd, UINT64 u8FirstAPts);

  /// Audio is already paused
  /// \param pvTag [IN] the ISyncCtrl private data
  /// \param ePipeline [IN] the Path the component belongs to
  /// \param u8PauseTime [IN] the audio paused time
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4AudioPaused)(void *pvTag, SYNCCTRL_PIPELINE ePipeline, UINT64 u8PauseTime);

  /// Audio is already stopped
  /// \param pvTag [IN] the ISyncCtrl private data
  /// \param ePipeline [IN] the Path the component belongs to
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4AudioStopped)(void *pvTag, SYNCCTRL_PIPELINE ePipeline);

  /// Audio skip done notify for fgAudDrvIf_SkipCmd
  /// \param pvTag    [IN] the user private data
  /// \param u8Pts     [IN] Audio current PTS after skip frame
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4AudioSkipDoneNotify)(void *pvTag, SYNCCTRL_PIPELINE ePipeline, UINT64 u8Pts);

  /// Audio EOS notify
  /// \param pvTag [IN] the ISyncCtrl private data
  /// \param ePipeline [IN] the Path the component belongs to
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4AudioEOSNotify)(void *pvTag, SYNCCTRL_PIPELINE ePipeline);

#if (SYNC_FRAMEACCURATE)
  /// Audio End Pts arrive notify for vAudDrvIf_SetEndPts
  /// \param pvTag     [IN] the user private data
  /// \param ePipeline [IN] the Path the component belongs to
  /// \Param u8EndPts  [IN] the end pts of frame accurate that SyncCtrl set to
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4AudioEndPtsDoneNotify)(void *pvTag, SYNCCTRL_PIPELINE ePipeline, UINT64 u8EndPts);
#endif
/*! @} */
  
/*! \name Interface for VDP used
* @{
*/
  /// Video is ready to perform eCmd and sends the playback mode to SyncCtrl
  /// \param pvTag [IN] the ISyncCtrl private data
  /// \param ePipeline [IN] the Path the component belongs to
  /// \param eCmd [IN] the playback command
  /// \param u8APts [IN] the first Audio PTS
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4VideoReadyTo)(void *pvTag, SYNCCTRL_PIPELINE ePipeline, SYNCCTRL_PLAYBACK_CMD eCmd, UINT64 u8FirstVPts);

  /// Video is already paused
  /// \param pvTag [IN] the ISyncCtrl private data
  /// \param ePipeline [IN] the Path the component belongs to
  /// \param u8PauseTime [IN] the video paused time
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4VideoPaused)(void *pvTag, SYNCCTRL_PIPELINE ePipeline, UINT64 u8PauseTime);

  /// Video is already stopped
  /// \param pvTag [IN] the ISyncCtrl private data
  /// \param ePipeline [IN] the Path the component belongs to
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4VideoStopped)(void *pvTag, SYNCCTRL_PIPELINE ePipeline);

  /// Video updates picture time
  /// \param pvTag [IN] the ISyncCtrl private data
  /// \param ePipeline [IN] the Path the component belongs to
  /// \param u8PictureTime [IN] the Picture PTS
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4VideoInfoUpdate)(void *pvTag, SYNCCTRL_PIPELINE ePipeline, UINT64 u8PicturePts);

  /// Video EOS notify
  /// \param pvTag [IN] the ISyncCtrl private data
  /// \param ePipeline [IN] the Path the component belongs to
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4VideoEOSNotify)(void *pvTag, SYNCCTRL_PIPELINE ePipeline);

  /// Video updates current display picture information
  /// \param pvTag [IN] the ISyncCtrl private data
  /// \param ePipeline [IN] the path the component belongs to
  /// \param u8PictureTime [IN] the current display picture PTS
  /// \param fgSlideshow [IN] the current display picture whether is slideshow
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4VideoCurrDispPicInfoUpdate)(void *pvTag, SYNCCTRL_PIPELINE ePipeline, UINT64 u8PicturePts, BOOL fgSlideShow);
  
  /// Video decoder updates  information
  /// \param pvTag [IN] the ISyncCtrl private data
  /// \param ePipeline [IN] the path the component belongs to
  /// \param eType [IN] the update information type
  /// \param pvInfo [IN] Point to the the information
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4VdecInfoUpdate)(void *pvTag, SYNCCTRL_PIPELINE ePipeline, SYNCCTRL_Vdec_Update_Type eType, void *pvInfo);
                                 
#if (SYNC_FRAMEACCURATE)
  /// Video frame accurate done notify
  /// \param pvTag     [IN] the user private data
  /// \param ePipeline [IN] the Path the component belongs to
  /// \Param u8EndPts  [IN] the end pts of frame accurate that SyncCtrl set to
  /// \return This function returns negative value if failed. Please reference drv_syncctrl_errcode.h.
  INT32 (*pi4VideoFrameAccurateDoneNotify)(void *pvTag, SYNCCTRL_PIPELINE ePipeline, UINT64 u8EndPts);
#endif

/*! @} */
} ISyncCtrl;

/// This interface represents IID_SyncCtrl_User interface.
/// This interface contains SyncCtrl functions.
typedef struct _ISyncCtrlUser
{
  /// Set Sync Ctrl interface
  /// \param pvTag [IN] the ISyncCtrlUser private data
  /// \param u2SyncCtrlType [IN] Sync ctrl type
  /// \param u2SyncCtrlId [IN] Sync ctrl id
  /// \return This function returns negative value if failed.
  INT32 (*pi4SetSyncCtrl)(void *pvTag, UINT16 u2SyncCtrlType, UINT16 u2SyncCtrlId);
} ISyncCtrlUser;

extern void SYNCCTRL_Isr(void);

#endif //_DRV_IF_SYNCCTRL_H_
