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




#ifndef _H_ASVAUDDRV
#define _H_ASVAUDDRV

#include "AsvDef.h"
#include "AsvTrigger.h"
#include "aud_drv.h"

#ifdef __cplusplus
extern "C"
{
#endif

// receive command
extern bool  fgASVCheckInit(void);
extern u8 u1AsvPlayCmd(u8 u1DecId);
extern u8 u1AsvStopCmd(u8 u1DecId);
extern u8 u1AsvPauseCmd(u8 u1DecId);
extern u8 u1AsvResumeCmd(u8 u1DecId);
extern u8 u1AsvFlushCmd(u8 ucDecId);
extern u8 u1AsvDspBFlushDone(u8 u1DecId);
extern void  vAsvPeriphDone(void);

extern u8 u1AudDspGetState(void);
extern u8 u1DspAGetState(void);
extern u8 u1DspBDec1GetState(void);
extern u8 u1DspAoutGetState(void);

// for notify ASH real play state
extern void AUD_RealPlayNotify(u8 u1DecId,  AUD_DRV_CMD_T eAudDecCmd);
extern void AUD_InbandCmdNotify(u8 u1DecId,  u32 u4IbcId);
extern void AUD_HdcdTrkStmChg_Notify(u8 u1DecId, bool isHdcdTrk);
extern void AUD_Ch_Cfg_Notify(u8 u1DecId, AUD_DRV_AUD_TYPE_T eAudChCfg);
extern void AUD_AsvCommandDone(u8 u1DecId, u32 u4Command);
extern void AUD_WaitAsvCommandDone(u8 u1DecId, u32 u4Command);

// for aout2 cmd
extern u8 u1AsvDspAAoutOff(void);
extern u8 u1AsvDspAAout2Off(void);
extern u8 u1AsvDspAAout2On(void);

extern void vAsvNotifyStepOK(u8 u1DecId);
extern void vAsvNotifyBeginPtsDone(u8 u1DecId);
extern void vAsvNotifyPauseBeginPts(u8 u1DecId);
extern void vAudDrvIf_DspEosNotify(u8 u1DecId);

// for Reencode cmd
extern u8 u1AsvReencStartCmd(u8 ucDecId);
extern u8 u1AsvReencStarted(u8 ucDecId);
extern u8 u1AsvReencStopCmd(u8 ucDecId);
extern u8 u1AsvReencStopped(u8 ucDecId);

#ifdef __cplusplus
}
#endif

#endif

