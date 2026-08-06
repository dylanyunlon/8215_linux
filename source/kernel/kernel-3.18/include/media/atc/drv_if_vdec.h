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
#ifndef __DRV_IF_VDEC_H_
#define __DRV_IF_VDEC_H_


#include "drv_if.h"
//#include "drv_if_rc.h"
//#include "drv_if_ftr.h"
#include "drv_if_syncctrl.h"
#include "drv_if_vdp.h"
#include "drv_comp_id.h"

#include "drv_im.h"

/// This interface represents IIDVDec interface.
/// This interface contains VDec functions.
typedef struct _IVDec
{
  /// Get VDec current outputed PTS
  /// \param pvTag [IN] the VDec private data
  /// \return This function returns negative value if failed.
  UINT64 (*pu8GetVDecCurrPTS)(void *pvTag);
  UINT32 (*pu8GetVDecCurrFrameRate)(void *pvTag);
  
  /// Get VDec current decoded picture count
  /// \param pvTag [IN] the VDec private data
  /// \return This function returns 0 if failed.
  UINT32 (*pu4GetVDecDecPicCount)(void *pvTag);
  /// Get VDec current output picture count
  /// \param pvTag [IN] the VDec private data
  /// \return This function returns 0 if failed.
  UINT32 (*pu4GetVDecOutputPicCount)(void *pvTag);
  /// Get VDec current available AU count
  /// \param pvTag [IN] the VDec private data
  /// \return This function returns 0 if failed.
  UINT32 (*pu4GetVDecCurrAUCount)(void *pvTag);
  /// Get VDec current playback status
  /// \param pvTag [IN] the VDec private data
  /// \return the MPV status referred Drv_vdec.h
  UINT32 (*pu4GetVDecCurrStatus)(void *pvTag);
  /// Get VDec pts of latest picture AU
  /// \param pvTag [IN] the VDec private data
  /// \return This function returns invalid timestamp if failed.
  UINT64 (*pu8GetVDecLatestAUPts)(void *pvTag);
  /// Get VDec status of dropping decoding-error picture
  /// \param pvTag [IN] the VDec private data
  /// \return This function returns TRUE if decoding-error picture dropped.
  BOOL (*pfgVDecIsDecErrDropped)(void *pvTag);
  /// Set VDec status of de-blocking function
  /// \param pvTag [IN] the VDec private data
  /// \param fgDeblock [IN] turn on or turn off deblocking function
  void (*pvSetVDecDeblock)(void *pvTag, UINT32 u4BNRStrength, BOOL fgDeblockDemo);
  /// Get if VDec ready to disp
  /// \param pvTag [IN] the VDec private data
  /// \return This function returns FALSE, VDec dec less than 50%, TRUE, ready for disp.
  BOOL (*pfgVDecReadyToDisp)(void *pvTag);
} IVDec;

#endif
