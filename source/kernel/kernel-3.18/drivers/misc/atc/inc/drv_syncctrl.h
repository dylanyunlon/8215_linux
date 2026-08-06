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
#ifndef _DRV_SYNCCTRL_H_
#define _DRV_SYNCCTRL_H_

#include "x_sync_ctrl.h"
#include "drv_comp_id.h"
#include "drv_if_syncctrl.h"


/// Sync Ctrl initialization for x_drv_init()
/// \return If return value < 0, it's failed. Please reference drv_syncctrl_errcode.h.
INT32 i4SyncCtrl_Init(void);

/// Sync Ctrl uninitialization for x_drv_init()
/// \return If return value < 0, it's failed. Please reference drv_syncctrl_errcode.h.
INT32 i4SyncCtrl_Uninit(UINT32 u4Case);

/// Get SyncCtrl instance handle by component Id
/// \return SyncCtrl instance handle
UINT32 u4SyncCtrl_GetInstanceHandleFromCompId(
  UINT16 u2CompId             ///< [IN] SyncCtrl component Id
);


/// Sync Ctrl simple connnect. Called by MW_IF and Conn_Cmd
/// \return If return value < 0, it's failed. Please reference drv_syncctrl_errcode.h.
INT32 i4SyncCtrl_Connect(
  UINT16 u2ThisCompType,           ///< [IN] component type of SyncCtrl
  UINT16 u2ThisCompId,               ///< [IN] component Id of SyncCtrl
  UINT8  u1ThisPortId,            ///< [IN] Port Id
  UINT16 u2UpstreamCompType,   ///< [IN] component type of upper stream
  UINT16 u2UpstreamCompId       ///< [IN] component Id of upper stream
);


/// Sync Ctrl simple disconnnect. Called by MW_IF and Conn_Cmd
/// \return If return value < 0, it's failed. Please reference drv_syncctrl_errcode.h.
INT32 i4SyncCtrl_Disconnect(
  UINT16 u2ThisCompType,           ///< [IN] component type of SyncCtrl
  UINT16 u2ThisCompId,               ///< [IN] component Id of SyncCtrl
  UINT8  u1ThisPortId,            ///< [IN] Port Id
  UINT16 u2UpstreamCompType,   ///< [IN] component type of upper stream
  UINT16 u2UpstreamCompId       ///< [IN] component Id of upper stream
);


/// Sync Ctrl. Called by MW_IF
/// Provide a SET operation to accept a {type, id} of a DCC/Splitter
INT32 i4SyncCtrl_SetConnect(
  UINT32 u4Handle,             ///< [IN] SyncCtrl instance handle
  UINT16 u2UpstreamCompType,   ///< [IN] component type of upper stream
  UINT16 u2UpstreamCompId,       ///< [IN] component Id of upper stream
  BOOL fgConnect                          ///< [IN] TRUE: Connect, FALSE: disconnect
);


/// Get the presentation time
/// \return If return value < 0, it's failed. Please reference drv_syncctrl_errcode.h.
INT32 i4SyncCtrl_GetSTC(
  UINT32 u4Handle,             ///< [IN] SyncCtrl instance handle
  UINT64 *pu8Stc          ///< [OUT] the presentation time
);

/// Get the current system STC.
/// \return If return value < 0, it's failed. Please reference drv_syncctrl_errcode.h.
INT32 i4SyncCtrl_GetSystemSTC(
  UINT32 u4Handle,             ///< [IN] SyncCtrl instance handle
  BOOL   *pfgSTCValid,      ///[OUT] Current STC is valid or not. 
  UINT64 *pu8Stc          ///< [OUT] the current system stc.
);

/// Set the presentation time
/// \return If return value < 0, it's failed. Please reference drv_syncctrl_errcode.h.
INT32 i4SyncCtrl_SetSTC(
  UINT32 u4Handle,             ///< [IN] SyncCtrl instance handle
  SYNCCTRL_PIPELINE ePipeline,  ///< [IN] the Path that the component belongs to
  SYNCCTRL_StcLoadMode eLoadMode, ///< [IN] the STC Load mode
  UINT64 u8Time ///< [IN] the time you want to set
);

/// Middleware subscribe a callback function
/// \return If return value < 0, it's failed. Please reference drv_syncctrl_errcode.h.
INT32 i4SyncCtrl_SubscribeMwCb(
  UINT32 u4Handle,             ///< [IN] SyncCtrl instance handle
  void *prMwNfyFctInfo   ///< [IN] Callback function
);


/// Request a presentation time trigger
/// \return If return value < 0, it's failed. Please reference drv_syncctrl_errcode.h.
INT32 i4SyncCtrl_RequestTimeTrigger(
  UINT32 u4Handle,             ///< [IN] SyncCtrl instance handle
  SYNC_CTRL_TIME_CTRL_T *prTimerInfo   ///< [IN] Trigger Timer information
);


/// Cancel the presentation time trigger
/// \return If return value < 0, it's failed. Please reference drv_syncctrl_errcode.h.
INT32 i4SyncCtrl_CancelTimeTrigger(
  UINT32 u4Handle,             ///< [IN] SyncCtrl instance handle
  SYNC_CTRL_TIME_CTRL_T *prTimerInfo   ///< [IN] Trigger Timer information
);

/// Set Master Port
/// \return If return value < 0, it's failed. Please reference drv_syncctrl_errcode.h.
INT32 i4SyncCtrl_SetMasterPort(
  UINT32 u4Handle,             ///< [IN] SyncCtrl instance handle
  UINT32 u4PortId     ///< [IN] Port Id
);

/// Print Log for AV Sync
/// \return If return value < 0, it's failed. Please reference drv_syncctrl_errcode.h.
INT32 i4SyncCtrl_PrintLog(
  UINT32 u4Handle,             ///< [IN] SyncCtrl instance handle
  SYNCCTRL_PIPELINE ePipeline, 
  UINT16 u2CompType,
  const CHAR *szMessage
);

/// SyncCtrl timer 
/// \return None.
void vSyncCtrl_SoftTimer(UINT8 u1PMXMode);

/// Status report
/// \return None.
void vSyncCtrl_StatusReport(
  UINT32 u4Handle             ///< [IN] SyncCtrl instance handle
);

#if (SYNC_FRAMEACCURATE)
/// Set frame accurate pts information
/// \return If return value < 0, it's failed. Please reference drv_syncctrl_errcode.h.
INT32 i4SyncCtrl_SetFrameAccuratePTS(
  UINT32 u4Handle,             ///< [IN] SyncCtrl instance handle
  SYNC_CTRL_FMACCU_PTS_T *prPtsInfo                  ///< [IN] PTS
);

/// Set frame accurate done notify function
/// \return If return value < 0, it's failed. Please reference drv_syncctrl_errcode.h.
INT32 i4SyncCtrl_SetFrameAccurateDoneNfyInfo(
  UINT32 u4Handle,             ///< [IN] SyncCtrl instance handle
  void *prNfyInfo                  ///< [IN] Notify function information
);
#endif

/// Set AV no Sync mode
/// \return If return value < 0, it's failed. Please reference drv_syncctrl_errcode.h.
INT32 i4SyncCtrl_SetAVNoSync(
  UINT32 u4Handle,             ///< [IN] SyncCtrl instance handle
  BOOL fgEnable                  ///< [IN] TRUE: AV no sync; FALSE: AV sync
);

/// Set Video Output Early or not.
/// \return If return value < 0, it's failed. Please reference drv_syncctrl_errcode.h.
INT32 i4SyncCtrl_SetVideoOutputEarlier(
  UINT32 u4Handle,             ///< [IN] SyncCtrl instance handle
  BOOL fgVideoOutputEarlier     ///< [IN] TRUE: Video Output Earlier; FALSE: According A/U PTS.
);

/// Set information for STC update frequency
/// \return If return value < 0, it's failed. Please reference drv_syncctrl_errcode.h.
INT32 i4SyncCtrl_SetSTCUpdateFrequency(
  UINT32 u4Handle,             ///< [IN] SyncCtrl instance handle
  SYNC_CTRL_STCUPDATE_FREQUENCY_T *prInfo           ///< [IN] STC update frequency information
);

/// Set STC update one step
/// \return If return value < 0, it's failed. Please reference drv_syncctrl_errcode.h.
INT32 i4SyncCtrl_SetSTCUpdateOneStep(
  UINT32 u4Handle,             ///< [IN] SyncCtrl instance handle
  BOOL fgForward               ///< [IN] TRUE: forward, FALSE: reverse
);

/// Get STC from syncctrl. for other driver module.
/// \return If return value < 0, it's failed. Please reference drv_syncctrl_errcode.h.
INT32 i4SyncCtrl_DrvGetStc(
  UINT64 *pu8Stc              ///< [Out] Get current STC for other driver module.
);

#endif // _DRV_SYNCCTRL_H_
