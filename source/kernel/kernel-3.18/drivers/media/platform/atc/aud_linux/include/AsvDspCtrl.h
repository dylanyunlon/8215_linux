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


#include  "AsvDef.h"
#include "AsvTrigger.h"
#include  "aud_drv_config.h"
#ifdef __cplusplus
extern "C"
{
#endif

// init
extern void vAudStateInit(void);
extern void vAudStateReset(void);

extern u8 u1AsvDspInit(void);
extern u8 u1AsvDspAInit(void);
extern u8 u1AsvDspBInit(void);
extern u8 u1AsvDspReencInit(void);

extern u8 u1AsvDspReady(void);
extern u8 u1AsvDspAReady(void);
extern u8 u1AsvDspBReady(void);
extern u8 u1AsvDspReencReady(void);
extern u8 u1AsvDspEncoderReady(void);        // -- Water (AUD_RIPPING)

extern u8 u1AsvDspAAoutOn(void);
extern u8 u1AsvDspAAout2On(void);
extern void vAsvDspAAoutResume(void);
extern u8 u1AsvDspAAoutOff(void);
extern u8 u1AsvDspAAout2Off(void);
extern u8 u1AsvDspBReceiveCfg(bool fgCfgchanged);
extern u8 u1AsvDspAAoutStopped(void);
extern u8 u1AsvDspAAout2Stopped(void);
extern u8 u1AsvDspAConnected(u8 u1DecId);
extern u8 u1AsvDspADisconnected(u8 u1DecId);
extern u8 u1AsvDspAAoutStarted(void);
extern u8 u1AsvDspAAout2Started(void);
extern u8 u1AsvDspAStepDone(u8 u1DecId);
extern u8 u1AsvReencStarted(u8 ucDecId);
extern u8 u1AsvReencStopped(u8 ucDecId);

extern u8 u1AsvDspBSendCfg(u8 u1DecId);
extern u8 u1AsvDspBDecReady(u8 u1DecId);
extern u8 u1AsvDspBDecStopped(u8 u1DecId);
extern u8 u1AsvDspBFlushCmd(u8 u1DecId);
extern void vAsvDspDecReadyTrigger(u8 u1DecId);

extern u8 u1DspAoutGetState(void);
extern u8 u1AudDspGetState(void);
extern u8 u1DspAGetState(void);
extern u8 u1DspBDec1GetState(void);
extern u8 u1DspBDec2GetState(void);
extern u8 u1DspBDec3GetState(void);

extern u8 u1DspReencGetState(void);
extern u8 u1DspEncoderGetState(void);        // -- Water (AUD_RIPPING)
extern bool fgAsvQueryAVD(void * rAudSrcCfg, void * rAudOutputCfg, void* rAudHdmiOutputCfg);
extern void vAsvNotifyPlayCmdGot(u8 u1DecId);
extern u8 u1AsvDspResumed(u8 u1DecId);
extern u8 u1AsvDspIbcNotify(u8 ucDecId, s32 i4IbcId);
extern u8 u1AsvDsp_Hdcd_Trk_Stm_Chg(u8 u1DecId,bool isHdcdTrk);
extern void vAsvNotifyBeginPtsDone(u8 u1DecId);
extern void vAsvNotifyPauseBeginPts(u8 u1DecId);
extern void vAsvNotifyEndPtsDone(u8 u1DecId, bool fgNotifySyncctrl);
extern u8 u1AsvDspBFlushDone(u8 u1DecId);
extern void vAsvDspAStepCmd (u8 u1DecId);
extern void vAsvDspAStepCancelDone(u8 u1DecId);
extern bool fgAsvNotifyAVDChStatus(u8 * prLChStatus,u8 * prRChStatus);
extern s32 i4AudLockAoutReset(void);
extern s32 i4AudUnlockAoutReset(void);
extern s32 i4AudLockAout2Reset(void);
extern s32 i4AudUnlockAout2Reset(void);

extern void i4AsvGpsMixDspNotifyPlayCmdDone(void);
extern void i4AsvGpsMixDspNotifyStopCmdDone(void);
extern void i4AsvGpsMixDspNotifyPauseCmdDone(void);
extern void i4AsvGpsMixDspNotifyResumeCmdDone(void);
extern u32 i4AsvGpsMixDspNotifyConsumedData(void);



#ifdef __cplusplus
}
#endif
