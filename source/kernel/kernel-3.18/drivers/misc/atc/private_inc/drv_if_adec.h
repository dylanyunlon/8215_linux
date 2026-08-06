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

#ifndef __DRV_IF_ADEC_H
#define __DRV_IF_ADEC_H


#include "drv_if.h"

// Check disc type
typedef enum
{
   DRV_ADEC_DISC_TYPE_NONE = 0, ///< Reserved
   DRV_ADEC_DISC_TYPE_LEG,        /// for LEG
   DRV_ADEC_DISC_TYPE_BD,        /// for BD LPCM   
} DRV_ADEC_DISC_TYPE_T;

typedef struct _DRV_ADEC_LPCM_PARM_T
{
   UINT8 u1ChannelAssignment;
   UINT8 u1SamplingFrequency;
   UINT8 u1BitsPerSample;
   BOOL fgDeEmphasis;
   DRV_ADEC_DISC_TYPE_T tDiscType;
} DRV_ADEC_LPCM_PARM_T;

/// This interface represents IID_IADec interface.
/// This interface is used for DSP functions.
typedef struct _IADec
{
  /// Set Sync Ctrl interface
  /// - pvTag                    [IN] the user private data
  /// - u2SyncCtrlType      [IN] Sync ctrl type
  /// - u2SyncCtrlId          [IN] Sync ctrl id
  /// .
  INT32 (*pi4AudSetLpcmParam)(void *pvUserPrivate, DRV_ADEC_LPCM_PARM_T *tAudLpcmParam);
  void (*pvAudSetDeEmphasis)(BOOL *fgFlag);
  void (*vAudDrv_SetBeginPts)(void *pvTag,UINT64 u8BeginPTS);
  void (*AudDrv_SetEndPts)(void *pvTag,UINT64 u8EndPTS);
  void (*vAudDrv_StcValid)(void *pvTag,UINT64 u8FirstPTS);
  INT32 (*i4AudDrv_SetNoSyncMode)(void *pvTag);
  BOOL (*fgAudDrv_SkipCmd)(void *pvTag,UINT64 u8TargetPTS);  
  UINT64 (*u8AudDrv_GetAudPts)(void *pvTag);
  void (*vAudDrvIf_SkipToEnd)(void *pvTag);
  void (*vAudDrv_InitStcBase)(void *pvTag,UINT64 u8StcBase);
  void (*pvAudSetLpcmDRC)(UINT8 u1DynRngCtrl);          //get DRC value
  void (*pvAudSetLpcmBitShiftGr2)(UINT8 u1BitShftGr2);  //bit_shift_of_channel_gr2
} IADec;


#endif // __DRV_IF_ADEC_H

