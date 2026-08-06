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

/*****************************************************************************
*  Driver Interterface: Exported interface for audio driver to audio submodules
*****************************************************************************/

#ifndef _AUD_DRV_SUB_INC_H_
#define _AUD_DRV_SUB_INC_H_

//#include "x_typedef.h"
#include <linux/types.h>
#include <media/atc/drv_aud.h>

// *********************************************************************
// Export API
// *********************************************************************
extern void AUD_AsvCommandDone(u8 u1DecId, u32 u4Command);

extern bool fgAudDrvIf_RequestOutputCfg(AUD_SOURCE_CFG_T *prSrcParam, AUD_OUTPUT_SETTING_CFG_T *prOutParam,
  AUD_OUTPUT_SETTING_CFG_T *prHdmiOutParam);
extern void vAudDrvIf_SetAout1Periph(void);
extern void vAudDrvIf_SetAout2Periph(void);
extern void vAudDrvIf_OutputCfgChanged(void);
extern void vAudDrvIf_DecOutputReady(u8 u1DecId);
extern void vAudDrvIf_DspStopDone(u8 u1DecId);
extern void vAudDrvIf_DspStepDone(u8 u1DecId);
extern void vAudDrvIf_DspBeginPTSDone(u8 u1DecId);
extern void vAudDrvIf_DspPauseBeginPTSDone(u8 u1DecId);
extern void vAudDrvIf_DspEndPTSDone(u8 u1DecId, bool fgNotifySyncctrl);
extern void vAudDrvIf_DspPauseDone(u8 u1DecId);
extern void vAudDrvIf_Resumed(u8 u1DecId);
extern void vAudDrvIf_DspGetPlayCmd(u8 u1DecId);
extern void vAudDspFrameAccurate(u8 u1DecId);
extern void vAudDspAConnectStatus(u8 u1DecId,bool fgConnect);
extern bool fgAudDspIf_SetAVDChStatusPcmMode(u8 * prLChStatus,u8 * prRChStatus);

#endif /* _AUD_DRVIF_H_ */

