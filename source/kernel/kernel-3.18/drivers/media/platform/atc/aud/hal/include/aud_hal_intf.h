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

#ifndef _AUD_HAL_INTF_H_
#define _AUD_HAL_INTF_H_

//#include "x_typedef.h"
#include <linux/types.h>
#include "drv_config.h"
#ifdef CONFIG_DRV_AUD_AC83XX

#include <mach/base_regs.h>
#endif

/*****************Audio HAL Data Structino Define******************************/
typedef enum
{
    AUD_HAL_DOWNSAMPLE_UNKNOW = 0,
    AUD_HAL_DOWNSAMPLE_WITHOUT,         // without downsample
    AUD_HAL_DOWNSAMPLE_HALF,            // 1/2 downsample
    AUD_HAL_DOWNSAMPLE_QUARTER          // 1/4 downsample
}AUD_HAL_DOWNSAMPLE;

typedef enum
{
    AUD_HAL_IEC_OUTPUT_L_R = 0,
    AUD_HAL_IEC_OUTPUT_LS_RS,
    AUD_HAL_IEC_OUTPUT_C_LFE,
    AUD_HAL_IEC_OUTPUT_CH7_CH8,
    AUD_HAL_IEC_OUTPUT_LINE_IN,
    AUD_HAL_IEC_OUTPUT_CH9_CH10,
    AUD_HAL_IEC_OUTPUT_CH11_CH12
}AUD_HAL_IEC_OUTPUT_SEL;

typedef enum
{
    AUD_HAL_IEC_BITNS_16 = 0,
    AUD_HAL_IEC_BITNS_20,
    AUD_HAL_IEC_BITNS_24

}AUD_HAL_IEC_BIT_NUMBER;

typedef enum
{
    AUD_HAL_DRAM_BLOCK0 = 0,
    AUD_HAL_DRAM_BLOCK1,
    AUD_HAL_DRAM_BLOCK2,
    AUD_HAL_DRAM_BLOCK3,
    AUD_HAL_DRAM_BLOCK4,
    AUD_HAL_DRAM_BLOCK5,
    AUD_HAL_DRAM_BLOCK6,
    AUD_HAL_DRAM_BLOCK7,
    AUD_HAL_DRAM_BLOCK8,
    AUD_HAL_DRAM_BLOCK9,
    AUD_HAL_DRAM_BLOCK10,
    AUD_HAL_DRAM_BLOCK11

}AUD_HAL_DRAM_BLOCK;

typedef enum
{
    AUD_HAL_DSP_ID_A = 0,
    AUD_HAL_DSP_ID_B,
    AUD_HAL_DSP_ID_C
}AUD_HAL_DSP_ID;

#define WriteREG(addr,value)        IO_WRITE32(AUD_UCV_BASE, (addr << 2), value)
#define ReadREG(addr)               IO_READ32(AUD_UCV_BASE, (addr << 2))
#define WriteREGC(addr,value)       IO_WRITE32(ADSP3_UCV_BASE, (addr << 2), value)
#define ReadREGC(addr)              IO_READ32(ADSP3_UCV_BASE, (addr << 2))


/*****************Audio HAL interface dalaration***********************************/
extern void vAudHalGetDownsample(AUD_HAL_DOWNSAMPLE * peDownSample);
extern void vAudHalSetIecMute(void);
extern void vAudHalSetIec2Mute(void);
extern void vAudHalSetIecUnmute(void);
extern void vAudHalSetIec2Unmute(void);
extern void vAudHalSetIecDownSample(AUD_HAL_DOWNSAMPLE eDownSample);
extern void vAudHalSetIec2DownSample(AUD_HAL_DOWNSAMPLE eDownSample);
extern void vAudHalSetIecChannle(AUD_HAL_IEC_OUTPUT_SEL eIecOutputSel);
extern void vAudHalSetIecBitNs(AUD_HAL_IEC_BIT_NUMBER eIecBitsNum);
extern void vAudHalSetIecClock(void);
extern void vAudHalSetCacheSize(u8 bSize);
extern void vAudHalSetCacheInstLen(u8 bSize);
extern void vAudHalResetAndHoldDSP(void);
extern void vAudHalSetDspBootUpAddress(void);
extern void vAudHalSetDspFlushCache(void);
extern void vAudHalSetDspBootFromRam(void);
extern void vAudHalTriggerDSP(void);
extern void vAudHalSetGetBitsNotHold(bool fgGetBitsNotHold);
extern bool fgAudHalDspABusy(void);
extern bool fgAudHalDspBBusy(void);
extern bool fgAudHalDspCBusy(void);
extern void vAudHalSendDSPAInt(u32 IntAddr ,u32 IntData);
extern void vAudHalSendDSPBInt(u32 IntAddr ,u32 IntData);
extern void vAudHalSendDSPCInt(u32 IntAddr ,u32 IntData);
extern void vAudHalSendDSPALInt(u32 IntAddr ,u32 IntData);
extern void vAudHalSendDSPBLInt(u32 IntAddr ,u32 IntData);
extern void vAudHalSendDSPCLInt(u32 IntAddr ,u32 IntData);
extern void vAudHalWriteDSPAIntLD(u32 u4LongData);
extern void vAudHalWriteDSPBIntLD(u32 u4LongData);
extern void vAudHalWriteDSPCIntLD(u32 u4LongData);
extern u32 u4AudHalGetBufRPtr(u8 u1DecId, u8 u1BufType);
extern void vAudHalSetBufWPtr(u8 u1DecId, u8 u1BufType, u32 u4WPtrVal);
extern void vAudHalResetBufWPtr(u8 u1DecId);
extern void vAudHalSetBufEndAddr(u8 u1DecId, u8 u1BufType, u32 u4Addr);
extern void vAudHalSetBufStartAddr(u8 u1DecId, u8 u1BufType, u32 u4Addr);
extern void vAudHalSetDspBlkRegInit(void);
extern void vAudHalGetBlkAddr(AUD_HAL_DRAM_BLOCK eBlockNum,uintptr_t* pu4Addr);
extern u32 u4AudHalGetDspLongData(AUD_HAL_DSP_ID eDspId);
extern void vAudHalClearInterupt(AUD_HAL_DSP_ID eDspId);
extern u8 u1AudHalGetDspIntAddr(AUD_HAL_DSP_ID eDspId);
extern u32 u4AudHalGetDspIntShortData(AUD_HAL_DSP_ID eDspId);
extern void vAudHalSetDspHwRegInit(void);
extern void vAudHalSetDspWorkingDramBlock(uintptr_t u4DspStartAddr);
extern void vAudHalSetDspLoadTable(u8 ucType, u32 u4size, const u32 *u4pra);
extern void vAudHalSetPrsDmaDestAfifo(void);
extern void vAudHalSetPrsDmaDestWorkingBuffer(void);
extern void vAudHalTriggerDSP(void);
extern void vAudHalSetDspBlkRegInitBank(u32 u4bank);
extern void vAudHalSetAdspCOff(bool fgSCOEnable);
extern u32 u4AudHalGetPbsBufSize(void);
extern u32 u4AudHalGetPbsBufDspOffset(u32 u4Addr);
extern void u4AudHalSetPbsBufRPtr(u32 u4RPtr);
extern u32 u4AudHalGetPbsBufWPtr(void);
extern u32 u4AudHalGetPbsBufRPtr(void);
extern u32 u4AudHalGetPbsBufEndAddr(void);
extern u32 u4AudHalGetPbsBufStartAddr(void);
extern u32 u4AudHalGetPbsBufBaseAddr(void);

extern uintptr_t u4AudHalGetAFIFOBaseAddr(u32 u4DecId);     // add by water -- (2010-3-19) for AFIFO monitor cli debug used
extern uintptr_t u4AudHalGetAFIFOWPtr(u32 u4DecId);
extern uintptr_t u4AudHalGetAFIFOStartAddr(u32 u4DecId);
extern uintptr_t u4AudHalGetAFIFOEndAddr(u32 u4DecId);

extern uintptr_t u4AudHalGetDSPDataPageStartAddr(u32 u4PageId); // add by water -- (2010-3-19) for AOUT monitor cli debug used
extern void vAudHalSetAsdataCh(u32 u4Ch);
extern u32 vAudHalGetAsdataCh(void);
extern void vAudHalSet128bitMode(u32 u4Switch);
#endif
