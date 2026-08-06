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



#ifndef _AUD_HW_H_
#define _AUD_HW_H_

//#include "x_typedef.h"
#include <linux/types.h>
#include "x_bim.h"

// IC Type Distinguishing Config
#include "chip_ver.h"


// *********************************************************************
// DSP CLK Select
// *********************************************************************

#define CLK_SEL_4                           0x48

  #define PDN_DSP                           (1 << 7)  //turn off audio dsp clock
  #define ADSP_CLK_SEL_MASK                 (0x0f << 0)

// *********************************************************************
// PAD Select
// *********************************************************************

#define PAD_CFG_0                           0x58  //5351

  #define PAD_SPDIF_SEL                     (0x3 << 4)
  #define PAD_SPDIF                         (0x2 << 4)

#define PAD_CFG_1                           0x5C //5351

// *********************************************************************
// Audio Configuration
// *********************************************************************

#define AUD_ACFG          0x0C          // Audio Configuration
  // IEC output channel selection
  #define IEC_OUT_LR       (0 << 8)     // IEC output L/R channel
  #define IEC_OUT_LSRS     (1 << 8)     // IEC output LS/RS channel
  #define IEC_OUT_CLFE     (2 << 8)     // IEC output C/LFE channel
  #define IEC_OUT_7_8      (3 << 8)     // IEC output 7/8 channel
  #define IEC_OUT_SPDIF_IN (4 << 8)     // IEC output SPDIF/LineIn channel
  #define IEC_OUT_9_10     (5 << 8)     // IEC output 9/10 channel
  // IEC mute
  #define IEC_MUTE         (1 << 12)    // IEC mute

//#define AUD_AOUTCFG       0xB0          // Audio Output Configuration

#define AUD_SPLIN_CTL  0x18              // SPDIF/Line-In Control
  #define SPDIF_LINE_IN_ENABLE  (0x1 << 0)  // Enable SPDIF/Line-In buffering data to DRAM
  #define SPDIF_LINE_IN_DISABLE (0x0 << 0)  // Disable SPDIF/Line-In buffering data to DRAM
  #define DATA_16_BITS          (0x0 << 1)  // Store 16 bits data per sample
  #define DATA_24_BITS          (0x1 << 1)  // Store 24 bits data per sample
  #define DATA_SWAP             (0x1 << 3)  // Swap data
  #define DATA_NO_SWAP          (0x0 << 3)  // No swap data
  #define RISC_INT_PERIOD_NONE   (0x0 << 4)  // RISC interrupt generating period NONE
  #define RISC_INT_PERIOD_64     (0x1 << 4)  // RISC interrupt generating period per 64*4 bytes
  #define RISC_INT_PERIOD_128    (0x2 << 4)  // RISC interrupt generating period per 128*4 bytes
  #define RISC_INT_PERIOD_256    (0x3 << 4)  // RISC interrupt generating period per 256*4 bytes
  #define PSR_PTR_SELECT_RISC   (0x0 << 8)
  #define PSR_PTR_SELECT_SPDIF_LINE_IN   (0x1 << 8)

// *********************************************************************
// Audio Output Configuration
// *********************************************************************
// *********************************************************************
// Macros
// *********************************************************************


#define vWriteCKGen(dAddr, dVal)  *(volatile u32 *)(CKGEN_BASE + (dAddr)) = (dVal)
#define dReadCKGen(dAddr)         *(volatile u32 *)(CKGEN_BASE + (dAddr))
#define vWriteAUD(dAddr, dVal)    *(volatile u32 *)(AUD_BASE  + (dAddr)) = (dVal)
#define dReadAUD(dAddr)           *(volatile u32 *)(AUD_BASE  + (dAddr))

#define SetBitCKGen(Reg, Bit)    vWriteCKGen(Reg, dReadCKGen(Reg) | (Bit))
#define ClrBitCKGen(Reg, Bit)    vWriteCKGen(Reg, dReadCKGen(Reg) & (~(Bit)))
#define SetBitAUD(Reg, Bit)      vWriteAUD(Reg, dReadAUD(Reg) | (Bit))
#define ClrBitAUD(Reg, Bit)      vWriteAUD(Reg, dReadAUD(Reg) & (~(Bit)))

// *********************************************************************
// Export API
// *********************************************************************

#endif /* _AUD_HW_H_ */

