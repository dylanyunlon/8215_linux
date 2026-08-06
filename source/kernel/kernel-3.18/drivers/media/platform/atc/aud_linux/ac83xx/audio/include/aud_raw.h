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
*  Audio Driver: Exported interfaces for dspctrl and other modules
*****************************************************************************/

#ifndef _AUD_RAW_H_
#define _AUD_RAW_H_

#define AUDESM_RAW_MAKER 0
#define PASS2MATPACKER 0

#if AUDESM_RAW_MAKER   

typedef void (*DspSetHDbypassMode)(bool* pfgHDBybass, bool* pfgAC3Bypass);

extern void vAudWrapInit(u8 u1DecId);
extern void vAudWrapUninit(u8 u1DecId);
extern void vAudWrapSetMode(u8 u1DecId, bool fgHDByPass, bool fgAC3ByPass);

extern void vAudWrapMakeInterface(u8 u1DecId);
extern void vAudWrapSendPTSQueue(u8 u1DecId, u32 u4PtsAddr, u64 u8Pts);
extern void vAudWrapSetBufWPts(u8 u1DecId, u8 u1BufType, u32 u4WPtrVal);
extern void vAudWrapState(void);
extern void vAudWrapUpdateAllDspRPtr(void);
extern u32 u4AudWrapGetBufRPtr(u8 u1DecId, u8 u1BufType);
extern u32 u4AudWrapResetWrite(u8 u1DecId, u8 u1BufType);
extern void vAudWrapResetAFIFO(u8 u1DecId, u8 u1BufType, u32 u4Start, u32 u4End);
extern void vAudWrapResetAUIdx(u8 u1DecId);
//extern void vAudWrapTimerTrigger(void);
extern void vAudWrapDestoryFifo(void);
extern void vAudWrapADecNotifyStop(u8 u1DecId);
extern void vAudWrapADecNotifyPlay(u8 u1DecId);
extern void vAudWrapSetStartRdPtr(u8 u1DecId, u32 u4Addr);
extern bool fgAudWrapFIFOEmpty(void);
extern void vAudWrapDumpStatus(void);
#endif

#endif
