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

#ifndef _DSP_DRV_INC_H_
#define _DSP_DRV_INC_H_

//#include "x_typedef.h"
#include <linux/types.h>
#include "DspUop.h"
#include "DspShm.h"
#include "DspConst.h"
#include "DspStruct.h"
#include <media/atc/drv_aud.h>
#ifdef __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

//====================================================
// Variable definition
// note that these variables should be access with function
//====================================================
extern uintptr_t g_u4DspDramBuf[8];
extern uintptr_t g_u4DspDramBufEx[4];
extern const u32 LRMIX[LRMIX_MAX] ;
//====================================================
// Function definition
//====================================================
extern u32 u4DspGetBufStartAddr(u8 u1DecId, u8 u1BufType);
extern u32 u4DspGetBufEndAddr(u8 u1DecId, u8 u1BufType);
extern void vDspSetPRSAfifoAddr(void);
//extern void vDspInitPtsQueue(u8 u1DecId);

extern void vDspState (void);
void vDspCmd (u32 u4Cmd);
extern void vDspPowerOff (void);

//AC3_OUTMODE, Orho
extern s16 i2AC3SpkCfgTblToOutMode(u32 u4Spkcfg1, u32 u4Spkcfg2);
extern s16 i2AC3SpkCfgTblToLFEMode(u32 u4Spkcfg1, u32 u4Spkcfg2);
extern void vDspDSPSamplingRateTransform (u32* u4DspSF,AUDIO_SAMPLING_T u4DspToAvdSF);

// UI_default
extern void vDspSyncCtrlSetStepTargetPts(u32  u4PtsVal);
extern u64 i8DspGetDspEndPts(u8 u1DecId) ;
extern u64 i8DspGetDspPts(u8 u1DecId) ;
extern void vDspSyncCtrlSetFirstTargetPts(u32  u4PtsVal,u8 u1DecId);
extern void vDspSyncCtrlSetEndPts(u32  u4PtsVal, u8 u1DecId);
extern void vDspSetEosPts(u32  u4PtsVal, u8 u1DecId) ;
extern u32 u4DspGetPlaybackTime(void);

extern void vDspUpdateSpkCfg(AUD_CH_NUM_T  eAout2ChCnt,bool fgAoutSelect);
extern void vDspUpdateAout2SamplingFreq(AUDIO_SAMPLING_T  eAout2SamplingRate);
extern void vAudUpdateIecCfg(bool fgHbr4sd,AUD_IEC_CFG_T eAout2IecCfg,bool fgAoutSelect);
extern void vAudSetChannelStatus(AUDIO_SAMPLING_T u1SamplingRate);
extern void vAudUpdateChDelay(AUD_CH_DELAY_SETTING_T  rAout2ChDelay,bool fgAoutSelect);


#ifdef __cplusplus
}
#endif                          /* __cplusplus */

#endif //DRV_ADSP_H

