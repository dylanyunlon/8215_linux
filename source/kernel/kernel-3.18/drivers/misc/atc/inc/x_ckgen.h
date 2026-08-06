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

#ifndef X_CKGEN_H
#define X_CKGEN_H

//============================================================================
// Include files
//============================================================================
#include "x_typedef.h"
#include "x_hal_ic.h"
#include "drv_config.h"
#include "chip_ver.h"

#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_AC83XX)
#include "x_ckgen_8317.h"
#endif

#ifdef  __ARM2__
#include "x_ckgen_8317.h"
#endif

//============================================================================
// Macros for register read/write
//============================================================================
#if 0
#define CKGEN_READ8(offset)            IO_READ8(CKGEN_UCV_BASE, (offset))
#define CKGEN_READ16(offset)           IO_READ16(CKGEN_UCV_BASE, (offset))
#define CKGEN_READ32(offset)           IO_READ32(CKGEN_UCV_BASE, (offset))

#define CKGEN_WRITE8(offset, value)    IO_WRITE8(CKGEN_UCV_BASE, (offset), (value))
#define CKGEN_WRITE16(offset, value)   IO_WRITE16(CKGEN_UCV_BASE, (offset), (value))
#define CKGEN_WRITE32(offset, value)   IO_WRITE32(CKGEN_UCV_BASE, (offset), (value))

#define CKGEN_REG8(offset)             IO_REG8(CKGEN_UCV_BASE, (offset))
#define CKGEN_REG16(offset)            IO_REG16(CKGEN_UCV_BASE, (offset))
#define CKGEN_REG32(offset)            IO_REG32(CKGEN_UCV_BASE, (offset))

#define CKGEN_SETBIT(offset, dBit)        CKGEN_WRITE32(offset, CKGEN_READ32(offset) | (dBit))
#define CKGEN_CLRBIT(offset, dBit)        CKGEN_WRITE32(offset, CKGEN_READ32(offset) & (~(dBit)))
#else
#endif

extern bool BSP_Calibrate(SRC_CK_T eSource, unsigned int u4Clock);
extern unsigned int BSP_GetClock(SRC_CK_T eSource);
extern bool CKGEN_SetPLL(SRC_CK_T eSource, unsigned int u4Clock0, unsigned int u4Clock1);
extern bool CKGEN_AgtOnClk(e_CLK_T eAgt);
extern bool CKGEN_AgtOffClk(e_CLK_T eAgt);
extern bool CKGEN_AgtSelClk(e_CLK_SEL_T eAgt, unsigned int u4Sel);
extern unsigned int CKGEN_AgtGetClk(e_CLK_SEL_T eAgt);
extern bool CKGEN_AgtOnClk_NoReset(e_CLK_T eAgt);
extern bool CKGEN_AgtOffClk_NoReset(e_CLK_T eAgt);
extern bool Module_Reset_On(e_MODULE_T eAgt);
extern bool Module_Reset_Off(e_MODULE_T eAgt);
#endif  // X_CKGEN_H

