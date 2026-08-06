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

#ifndef X_IOMMU_3363_H
#define X_IOMMU_3363_H

//============================================================================
// Include files
//============================================================================
#include "x_hal_ic.h"
#include "x_typedef.h"
#include "drv_config.h"
#include "x_hal_83xx.h"

#define IOMMU_BASE_VA (IO_BASE_VA + 0x09000)
//============================================================================
// Macros for register read/write
//============================================================================
#define IOMMU_READ32(offset, bank)          IO_READ32(IOMMU_BASE_VA+bank, offset)
#define IOMMU_WRITE32(offset, bank, value)  IO_WRITE32(IOMMU_BASE_VA+bank, offset, (value))
#define IOMMU_CHKSUM_READ32(offset)         IO_READ32(IO_BASE_VA, offset)
#define IOMMU_CHKSUM_WRITE32(offset, value) IO_WRITE32(IO_BASE_VA, offset, (value))
//============================================================================
// Macros for memory read/write
//============================================================================
#define WRITEMEM8(Address, Value)           *(volatile UINT8 *)(Address) = Value
#define READMEM8(Address)                   *(volatile UINT8 *)(Address)

#define WRITEMEM16(Address, Value)          *(volatile UINT16 *)(Address) = Value
#define READMEM16(Address)                  *(volatile UINT16 *)(Address)

#define WRITEMEM32(Address, Value)          *(volatile UINT32 *)(Address) = Value
#define READMEM32(Address)                  *(volatile UINT32 *)(Address)

//============================================================================
// IOMMU Registers
//============================================================================
//  #define IOMMU_BASE                                      (IO_BASE + 0x09000)

//module bank
//#define IOMMU_GCPU                          0x0000
#define IOMMU_GFX                           0x0000
#define IOMMU_IMG_RESZ                      0x0100
#define IOMMU_PNG							0x0200
#define IOMMU_JPG                           0x0300
#define IOMMU_GIF                           0x0400
#define IOMMU_OSD_RESZ						0x0500
#define IOMMU_RESZ	IOMMU_IMG_RESZ
#define IOMMU_RESZ1	IOMMU_OSD_RESZ
//#define IOMMU_RLE                           0x1140


#define REG_RW_IOMMU_CFG0                   0x000           // basic setting
#define REG_RW_IOMMU_CFG1                   0x004           // page table index
#define REG_RW_IOMMU_CFG2                   0x008           // agnet_0~1 setting
#define REG_RW_IOMMU_CFG3                   0x00C           // agnet_2~3 setting
#define REG_RW_IOMMU_CFG4                   0x010           // interrupt, monitor and debug
#define REG_RW_IOMMU_CFG5                   0x014           // perfomance meter
#define REG_RW_IOMMU_CFG6                   0x018           // monitor result
#define REG_RW_IOMMU_CFG7                   0x01C           // monitor result
#define REG_RW_IOMMU_CFG8                   0x020           // monitor result
// special setting *********************
#define REG_RW_IOMMU_CFG9                   0x024           // over read protection
#define REG_RW_IOMMU_CFGA                   0x028           // over read protection
#define REG_RW_IOMMU_CFGB                   0x02C           // over read protection
#define REG_RW_IOMMU_CFGC                   0x030           // over read protection
#define REG_RW_IOMMU_CFGD                   0x034           // over read protection
// special setting &&&&&&&&&&&&&&&&&&&&&


#define IOMMU_GFX_OTHER                     0x2040          // for OverRead used only
#define IOMMU_GFX_COMPRESSION               0x3040          // for OverRead used only

#define MON_START                           0
#define MON_DATA                            1
#define CHK_SUM_START                       0
#define CHK_SUM_RESULT                      1

#define REG_IOMMU_CFG0_DEF                  0x000000FE

#define REG_IOMMU_CFG4_EN                   0x0410090A
#define REG_IOMMU_CFG4_EN_60B               0x0010090A
#define REG_IOMMU_CFG4_CLR                  0x04100905
#define REG_IOMMU_CFG4_MUTE                 0x04100900
#define REG_IOMMU_CFG4_RST_TLB              0x06100905
#define REG_IOMMU_CFG4_RST_ALL              0x07100905
#define REG_IOMMU_CFG4_DEBUG                0x0410010A

#define REG_IOMMU_CHK_SUM_RESET             0x40840         //
#define REG_IOMMU_CHK_SUM_SEL               0x40844         //
  #define AG_GFX                            0x0
  #define AG_GIF                            0x1
  #define AG_JPG                            0x2
  #define AG_PNG                            0x3
  #define AG_RESZ                           0x4
  #define AG_RLE                            0x5
#define REG_IOMMU_IADR_WRITE                0x40848         //
#define REG_IOMMU_IADR_READ                 0x4084C         //
#define REG_IOMMU_MADR_WRITE                0x40850         //
#define REG_IOMMU_MADR_READ                 0x40854         //
#define REG_IOMMU_IWDAT3                    0x40858         //
#define REG_IOMMU_IWDAT2                    0x4085C         //
#define REG_IOMMU_IWDAT1                    0x40860         //
#define REG_IOMMU_IWDAT0                    0x40864         //
#define REG_IOMMU_MWDAT3                    0x40868         //
#define REG_IOMMU_MWDAT2                    0x4086C         //
#define REG_IOMMU_MWDAT1                    0x40870         //
#define REG_IOMMU_MWDAT0                    0x40874         //

//============================================================================
// Public functions
//============================================================================
EXTERN UINT32 u4HalGetTTB0(VOID);
EXTERN UINT32 u4HalGetTTB1(VOID);
EXTERN UINT32 u4HalGetTTB(UINT32 ui4_mem_ptr);
EXTERN VOID vIOMMU_Performance(UINT32 ui4_type, UINT32 ui4_step);
EXTERN VOID vIOMMU_CheckSum(UINT32 ui4_step, UINT32 ui4_ag, UINT32 ui4_id);

#endif  // X_IOMMU_8563_H
