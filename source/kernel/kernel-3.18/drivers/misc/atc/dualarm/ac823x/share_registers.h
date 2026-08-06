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

#ifndef SHARE_REGISTER_H
#define SHARE_REGISTER_H

#define REG_RW_REMAP                    0x001C        //Remap Register
  #define REMAP_BIT                       (1U << 0)   //Remap Bit
#define REG_RW_DRAMB_OFF                0x0020        //DRAM-B Offset Address Register

//----------------------------------------------------------------------------
// IRQ/FIQ
#define REG_ARM2_IRQST                       0x0100        //RISC L1 IRQ Status Register

#define REG_RW_TOCORISC                 0x0170        //To Co-RISC Interrupt Register
  #define TOCORISC_INTR                   (1U << 0)   //Interrupt To Co-RISC
  #define TOCORISC_INTR_CLR             (1U << 23)   //Interrupt Clear from Co-RISC //8555 remove from 8530

//----------------------------------------------------------------------------
// ARM2 SHARE INFO REGISTERS
#define REG_RW_SINFO0_REG   0x0150        //Dual Core Share Info Register 0
#define REG_RW_SINFO1_REG   0x0154        //Dual Core Share Info Register 1
#define REG_RW_SINFO2_REG   0x0158        //Dual Core Share Info Register 2
#define REG_RW_SINFO3_REG   0x015C        //Dual Core Share Info Register 3


#define REG_RW_SINFO4_REG   0x0160        //Dual Core Share Info Register 4
  #define FB_MEDIA_PRESENT    (1U << 0)     //[Fastboot]ARM2->ARM1 Detect Media Present
  #define FB_ARM2_ALIVE       (1U << 1)     //[Fastboot]ARM2->ARM1 Fastboot Function is Alive
  #define FB_ARM1_SATA_INIT   (1U << 2)     //[Fastboot]ARM1->ARM2 SATA Driver Init
  #define FB_ARM1_LIRC_INIT   (1U << 3)     //[Fastboot]ARM1->ARM2 LIRC Driver Init
  #define FB_ARM1_EJECT_KEY   (1U << 4)     //[Fastboot]ARM1->ARM2 Eject Key is Pressed
  #define FB_ARM1_IRRX_INIT   (1U << 5)     //[Fastboot]ARM1->ARM2 IRRX Driver Init

#define REG_RW_SINFO5_REG   0x0164        //Dual Core Share Info Register 5
#define REG_RW_SINFO6_REG   0x0168        //Dual Core Share Info Register 6
#define REG_RW_SINFO7_REG   0x016C        //Dual Core Share Info Register 7
#define REG_RW_SINFO8_REG   0x0170        //Dual Core Share Info Register 8
#define REG_RW_SINFO9_REG   0x0174        //Dual Core Share Info Register 9
#define REG_RW_SINFOA_REG   0x0178        //Dual Core Share Info Register A
#define REG_RW_SINFOB_REG   0x017C        //Dual Core Share Info Register B
#define REG_RW_SINFOC_REG   0x0180        //Dual Core Share Info Register C
#define REG_RW_SINFOD_REG   0x0184        //Dual Core Share Info Register D
#define REG_RW_SINFOE_REG   0x0188        //Dual Core Share Info Register E
#define REG_RW_SINFOF_REG   0x018C        //Dual Core Share Info Register F

#define REG_RW_RISCRST                  0x01B8        //RISC Reset Control Register
  #define RISCRST_RISC0_RESET             (1U << 0)   //Release Reset RISC 0
  #define RISCRST_RISC1_RESET             (1U << 1)   //Release Reset RISC 1
  #define RISCRST_PASSWD                  0x85208888  // Common Password that Force RISC into Reset State
  #define RISCRST_RISC0_PASSWD            0x16880001  // Password that Force RISC0 into Reset State
  #define RISCRST_RISC1_PASSWD            0x16880002  // Password that Force RISC0 into Reset State

//#define ARM2_REG_IRQ_STATUS  (*((volatile UINT32*)(BIM_1_UCV_BASE + REG_ARM2_IRQST)))
#endif  // SHARE_REGISTER_H
