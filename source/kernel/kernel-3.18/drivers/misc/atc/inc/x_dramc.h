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

#ifndef X_DRAMC_H
#define X_DRAMC_H

//============================================================================
// Include files
//============================================================================
#include "x_hal_ic.h"
#include "x_typedef.h"

//============================================================================
// Constant definitions
//============================================================================
#define DRAM_CHANNEL_1					        0x1
#define DRAM_CHANNEL_2					        0x2

#define DRAM_ARM1	                      		0x0
#define DRAM_ARM2     					        0x1

/* HW-Dynamic Priority Change: starve ID */
#define DRAMC_STRV0					            0
#define DRAMC_STRV1					            1
#define DRAMC_STRV2					            2
#define DRAMC_STRV3					            3
#define DRAMC_STRV4					            4
#define DRAMC_STRV5					            5
#define DRAMC_STRV6					            6
#define DRAMC_STRV7					            7
#define DRAMC_STRV8					            8
#define DRAMC_STRV9					            9
#define DRAMC_STRV10					        10
#define DRAMC_STRV11					        11
#define DRAMC_STRV12					        12
#define DRAMC_STRV13					        13
#define DRAMC_STRV14					        14
#define DRAMC_STRV15					        15
/* Code Protect : 0~2=CPU/Agent,  3~4=Read only */
#define DRAMC_PROTC0					        0
#define DRAMC_PROTC1					        1
#define DRAMC_PROTC2					        2
#define DRAMC_PROTC3					        3
#define DRAMC_PROTC4					        4
/* Priority Group1>Group2>Group3.  Group1:16 agnts, set priority, Group2 and Group3: 8 agents  round-robin priority */
#define DRAMC_GROUP1					        1
#define DRAMC_GROUP2					        2
#define DRAMC_GROUP3					        3

// TEST switch
//#define DRAMC_TEST_DLL

//============================================================================
// Macros for register read/write
//============================================================================
#define LARB_READ8(offset)                      IO_READ8(LARB_IPYA_BASE, offset)
#define LARB_READ16(offset)                     IO_READ16(LARB_IPYA_BASE, offset)
#define LARB_READ32(offset)                     IO_READ32(LARB_IPYA_BASE, offset)

#define LARB_WRITE8(offset, value)              IO_WRITE8(LARB_IPYA_BASE, offset, (value))
#define LARB_WRITE16(offset, value)             IO_WRITE16(LARB_IPYA_BASE, offset, (value))
#define LARB_WRITE32(offset, value)             IO_WRITE32(LARB_IPYA_BASE, offset, (value))

#define REG_RW_ARB04				            0x0000
#define REG_RW_ARB05				            0x0020
#define REG_RW_ARB06				            0x0040
#define REG_RW_ARB09				            0x0060
#define REG_RW_ARB09_0				            0x0080
#define REG_RW_ARB09_1				            0x00A0
#define REG_RW_ARB011				            0x00C0
#define REG_RW_ARB012				            0x00E0
#define REG_RW_ARB013				            0x0100
#define REG_RW_ARB014				            0x0120
#define REG_RW_ARB016				            0x0140
#define REG_RW_ARB016_4				            0x0160
#define REG_RW_ARB017				            0x0180
#define REG_RW_ARB018				            0x01A0

#define REG_RW_ARB_EN				            0x4
#define REG_RW_ARB_CYC				            0x8
#define REG_RW_ARB_LEN				            0xC

#define REG_RW_ARB_SUBID_MASK		            0xF
#define REG_RW_ARB_BEG				            0x10
#define REG_RW_ARB_END				            0x20


//============================================================================
// Macros for register read/write
//============================================================================
#define DRAMC_READ8(offset)                     IO_READ8(DRAM_UCV_BASE, offset)
#define DRAMC_READ16(offset)                    IO_READ16(DRAM_UCV_BASE, offset)
#define DRAMC_READ32(offset)                    IO_READ32(DRAM_UCV_BASE, offset)

#define DRAMC_WRITE8(offset, value)             IO_WRITE8(DRAM_UCV_BASE, offset, (value))
#define DRAMC_WRITE16(offset, value)            IO_WRITE16(DRAM_UCV_BASE, offset, (value))
#define DRAMC_WRITE32(offset, value)            IO_WRITE32(DRAM_UCV_BASE, offset, (value))

#define DRAMC_REG8(offset)                      IO_REG8(DRAM_UCV_BASE, offset)
#define DRAMC_REG16(offset)                     IO_REG16(DRAM_UCV_BASE, offset)
#define DRAMC_REG32(offset)                     IO_REG32(DRAM_UCV_BASE, offset)

#define DRAMC2_READ8(offset)                    IO_READ8(DRAM1_UCV_BASE, offset)
#define DRAMC2_READ16(offset)                   IO_READ16(DRAM1_UCV_BASE, offset)
#define DRAMC2_READ32(offset)                   IO_READ32(DRAM1_UCV_BASE, offset)

#define DRAMC2_WRITE8(offset, value)            IO_WRITE8(DRAM1_UCV_BASE, offset, (value))
#define DRAMC2_WRITE16(offset, value)           IO_WRITE16(DRAM1_UCV_BASE, offset, (value))
#define DRAMC2_WRITE32(offset, value)           IO_WRITE32(DRAM1_UCV_BASE, offset, (value))

#define DRAMC2_REG8(offset)                     IO_REG8(DRAM1_UCV_BASE, offset)
#define DRAMC2_REG16(offset)                    IO_REG16(DRAM1_UCV_BASE, offset)
#define DRAMC2_REG32(offset)                    IO_REG32(DRAM1_UCV_BASE, offset)

#error "chip version error"

//============================================================================
// Public functions
//============================================================================
DRAM_MODE_T DRAMcGetDRAMMode(void);
void vDRAMcShowWindow(void);
/* DRAM Monitor Function */
EXTERN void DRAMcShowBM_PercentList(void);
EXTERN UINT32 DRAMcShowBM_Counter3(void);
EXTERN UINT32 DRAMcShowBM_Counter2(void);
EXTERN UINT32 DRAMcShowBM_Counter1(void);
EXTERN UINT32 DRAMcShowBM_AgtID(UINT32 u4GPID);
EXTERN BOOL DRAMcHoldBM(void);
EXTERN BOOL DRAMcDisBM(void);
EXTERN BOOL DRAMcEnBM(UINT32 u4GpID);
EXTERN BOOL DRAMcDumpBMConfig(void);
EXTERN BOOL DRAMcSetBM_REQALE_Dly(UINT32 u4AgtID, UINT32 u4Period);
EXTERN BOOL DRAMcSetBM_Active(UINT32 u4AgtID, UINT32 u4Period);
EXTERN BOOL DRAMcSetBM_Precharge(UINT32 u4AgtID, UINT32 u4Period);
EXTERN BOOL DRAMcSetBM_REQALE(UINT32 u4AgtID, UINT32 u4Period);
EXTERN BOOL DRAMcSetBM_GP3(UINT32 u4AgtID, UINT32 u4Period);
EXTERN BOOL DRAMcSetBM_GP2(UINT32 u4AgtID, UINT32 u4Period);
EXTERN BOOL DRAMcSetBM_GP1(UINT32 u4AgtID, UINT32 u4Period);

/* DRAM Code Protect */
EXTERN BOOL DRAMcShowCodePtc(void);
EXTERN BOOL DRAMcDisCodePtc(UINT32 u4PtcID);
EXTERN BOOL DRAMcEnCodePtc(UINT32 u4PtcID);
EXTERN BOOL DRAMcDumpCodePtcConfig(void);
EXTERN BOOL DRAMcSetCodePtc(UINT32 u4PtcID, UINT32 u4AgtID, UINT32 u4UpBnd, UINT32 u4LwBnd);
EXTERN BOOL DRAMcSetCodePtcMode(UINT32 u4PtcID, UINT32 u4AgtID, UINT32 u4ModeSel, UINT32 CUPYes, UINT32 CUPSel);
EXTERN BOOL DRAMcSetCodePtcBnd(UINT32 u4PtcID, UINT32 u4UpBnd, UINT32 u4LwBnd);

/* DRAM HW-Dynamic Priority Change */
EXTERN BOOL DRAMcShowSTRVAgtCnt(void);
EXTERN BOOL DRAMcFreezeSTRVAgtCnt(void);
EXTERN BOOL DRAMcDisSTRVAgtCnt(void);
EXTERN BOOL DRAMcEnSTRVAgtCnt(void);
EXTERN BOOL DRAMcSetAgtID(UINT32 u4GpID, UINT32 u4AgtID);
EXTERN BOOL DRAMcShowSTRVstate(void);
EXTERN BOOL DRAMcDumpSTRVConfig(void);
EXTERN BOOL DRAMcSetSTRV(UINT32 u4StrvID, UINT32 u4Ind, UINT32 u4Dec, UINT32 u4Inc);

/* DRAM HW Calibration */
EXTERN BOOL DRAMcDisCa(void);
EXTERN BOOL DRAMcEnCa(void);

EXTERN BOOL DRAMcChkPtcAgt(void);
EXTERN BOOL DRAMc2ChkPtcAgt(void);
EXTERN void DRAMcShowAgtID(UINT32 u4AgtID, UINT32 u4SubID);

EXTERN BOOL DRAMcCheckHWCal(void);
EXTERN BOOL DRAMcShowHWCal(UINT32 channel, BOOL bLog);
EXTERN void _DRAMcResetHWCal(UINT32 channel);
EXTERN BOOL DRAMcEnableHWCal(UINT32 channel);
EXTERN BOOL DRAMcHWBIST(UINT32 channel);
EXTERN INT32 DRAMcCalShow(UINT32 u4Period);
EXTERN void vDRAMcDLLEnable(void);
EXTERN void vDRAMcDLLGet(UINT32 channel, UINT32 *dlls);
EXTERN BOOL DRAMcCheckDLL(void);
#endif  // X_DRAMC_H
