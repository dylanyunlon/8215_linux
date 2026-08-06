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

#ifndef X_BIM_83XX_H
#define X_BIM_83XX_H

//============================================================================
// Include files
//============================================================================
#include "x_hal_ic.h"
#include "x_typedef.h"
#include "drv_config.h"

#if 0
//============================================================================
// Macros for register read/write
//============================================================================
#define BIM_READ8(offset)            IO_READ8(BIM_UCV_BASE, offset)
#define BIM_READ16(offset)           IO_READ16(BIM_UCV_BASE, offset)
#define BIM_READ32(offset)           IO_READ32(BIM_UCV_BASE, offset)

#define BIM_WRITE8(offset, value)    IO_WRITE8(BIM_UCV_BASE, offset, (value))
#define BIM_WRITE16(offset, value)   IO_WRITE16(BIM_UCV_BASE, offset, (value))
#define BIM_WRITE32(offset, value)   IO_WRITE32(BIM_UCV_BASE, offset, (value))

#define BIM_REG8(offset)             IO_REG8(BIM_UCV_BASE, offset)
#define BIM_REG16(offset)            IO_REG16(BIM_UCV_BASE, offset)
#define BIM_REG32(offset)            IO_REG32(BIM_UCV_BASE, offset)

#define BIM2_READ8(offset)           IO_READ8(BIM_1_UCV_BASE, offset)
#define BIM2_READ16(offset)          IO_READ16(BIM_1_UCV_BASE, offset)
#define BIM2_READ32(offset)          IO_READ32(BIM_1_UCV_BASE, offset)

#define BIM2_WRITE8(offset, value)   IO_WRITE8(BIM_1_UCV_BASE, offset, (value))
#define BIM2_WRITE16(offset, value)  IO_WRITE16(BIM_1_UCV_BASE, offset, (value))
#define BIM2_WRITE32(offset, value)  IO_WRITE32(BIM_1_UCV_BASE, offset, (value))

#define BIM2_REG8(offset)            IO_REG8(BIM_1_UCV_BASE, offset)
#define BIM2_REG16(offset)           IO_REG16(BIM_1_UCV_BASE, offset)
#define BIM2_REG32(offset)           IO_REG32(BIM_1_UCV_BASE, offset)

#define SBIM_READ8(offset)           IO_READ8(SBIM_UCV_BASE, offset)
#define SBIM_READ16(offset)          IO_READ16(SBIM_UCV_BASE, offset)
#define SBIM_READ32(offset)          IO_READ32(SBIM_UCV_BASE, offset)

#define SBIM_WRITE8(offset, value)   IO_WRITE8(SBIM_UCV_BASE, offset, (value))
#define SBIM_WRITE16(offset, value)  IO_WRITE16(SBIM_UCV_BASE, offset, (value))
#define SBIM_WRITE32(offset, value)  IO_WRITE32(SBIM_UCV_BASE, offset, (value))
//============================================================================
// Macros for memory read/write
//============================================================================
#define WRITEMEM8(Address, Value)       *(volatile UINT8 *)(Address) = Value
#define READMEM8(Address)               *(volatile UINT8 *)(Address)

#define WRITEMEM16(Address, Value)      *(volatile UINT16 *)(Address) = Value
#define READMEM16(Address)              *(volatile UINT16 *)(Address)

#define WRITEMEM32(Address, Value)      *(volatile UINT32 *)(Address) = Value
#define READMEM32(Address)              *(volatile UINT32 *)(Address)
#else
#endif

//============================================================================
// Boot Loader Version Define
//============================================================================
#define REG_RW_BOOTLOADER_VERSION       REG_RW_GPRB0
#define REG_RW_NE_UPG_VERSION           REG_RW_GPRB1

//============================================================================
// SBIM Registers
//============================================================================
#define REG_DMPROT_BGN                  0x0050
#define REG_DMPROT_END                  0x0054
#define REG_DMPROT_CTL                  0x0058
#define REG_CP15SDISABLE                0x0088
  #define DM1PROTCFGDISABLE               (1U << 17)
  #define DM0PROTCFGDISABLE               (1U << 16)
  #define ICE_DISABLE_RISC_1              (1U << 2)
  #define ICE_DISABLE_RISC_2              (1U << 3)

//============================================================================
// BIM Registers
//============================================================================
#define REG_RO_ICE                      0x0000        //RISC ICE Register
  #define ICE_STRAP                       (1U << 0)   //HDMI JTAG strapping and HDMI JTAG enable register.  //8555 new add
  #define ICE_ICE_1_STRAP                 (1U << 8)   //The 2nd RISC ICE mode Strapping                     //8555 new add
  #define ICE_BDOPT_ICE_1                 (1U << 9)   //Bonding decided ICE_1 strap value                   //8555 new add
  #define ICE_DBGACK_1                    (1U << 10)  //The 2nd RISC Debug Mode Acknowledge                 //8555 new add
  
#define REG_RW_ADDREN                   0x0004        //RISC Address Enable Register
  #define IOBASE_EN                       (1U << 0)   //IO Base Address Space Enable
  #define FIOBASE_EN                      (1U << 1)   //Fast IO Base Address Space Enable
  #define PBIB_EN                         (1U << 2)   //RISC PBI-B Base Address Space Enable
  #define PBIA_EN                         (1U << 3)   //RISC PBI-A Base Address Space Enable
  #define DRAMA_EN                        (1U << 4)   //RISC DRAM-A Base Address Space Enable
  #define DRAMB_EN                        (1U << 5)   //RISC DRAM-B Base Address Space Enable
  #define ROM_EN                          (1U << 6)   //RISC ROM Base Address Space Enable
#define REG_RW_PBIA_BA                  0x0008        //PBI-A Base Address Register
#define REG_RW_PBIB_BA                  0x000C        //PBI-B Base Address Register
#define REG_RW_DRAMA_BA                 0x0010        //DRAM-A Base Address Register
#define REG_RW_DRAMB_BA                 0x0014        //DRAM-B Base Address Register
#define REG_RW_IOBASE_BA                0x0018        //IO Base Address Register
#define REG_RW_REMAP                    0x001C        //Remap Register
  #define REMAP_BIT                       (1U << 0)   //Remap Bit
#define REG_RW_DRAMB_OFF                0x0020        //DRAM-B Offset Address Register
#define REG_RW_BUSTIME                  0x0024        //BUS Access Timing Register
#define REG_RO_CACHESIZE                0x002C        //Cache Size Register
#define REG_RW_MPPCFG                   0x004C        //Multi-Purpose SRAM Configuration Register

#define REG_RW_RST_CFG                  0x0048  //Reset Configuration Register //8555 change bonding read from ckgen to secure bim
  #define RST_CFG_TRAP_MAKS               0x03FF0000
  #define RST_CFG_TRAP_OFFSET             16
  #define RST_CFG_PROT_MASK               0x000003FF
  #define RST_CFG_PROT_OFFSET             0

//----------------------------------------------------------------------------
// IRQ/FIQ
#define REG_ARM2_IRQST                       0x0100        //RISC L1 IRQ Status Register

#define REG_IRQST                       0x0030        //RISC L1 IRQ Status Register
#define REG_IRQEN                       0x0034        //RISC L1 IRQ Enable Register
    #define EXT_INT_EN_                    (1<<3)     //External Interrupt Enable 

#define REG_IRQCL                       0x0038        //RISC L1 IRQ Clear Register
#define REG_FIQST                       0x003C        //RISC L1 FIQ Status Register
#define REG_FIQEN                       0x0040        //RISC L1 FIQ Enable Register
#define REG_FIQCL                       0x0044        //RISC L1 FIQ Clear Register

#define REG_IRQST2                      0x0138        //RISC L2 IRQ Status Register
#define REG_IRQEN2                      0x013C        //RISC L2 IRQ Enable Register
#define REG_IRQCL2                      0x0140        //RISC L2 IRQ Clear Register
#define REG_FIQST2                      0x0144        //RISC L2 FIQ Status Register
#define REG_FIQEN2                      0x0148        //RISC L2 FIQ Enable Register
#define REG_FIQCL2                      0x014C        //RISC L2 FIQ Clear Register

#define REG_IRQST3                      0x0154        //RISC L3 IRQ Status Register
#define REG_IRQEN3                      0x0158        //RISC L3 IRQ Enable Register
#define REG_IRQCL3                      0x015C        //RISC L3 IRQ Clear Register
#define REG_FIQST3                      0x0160        //RISC L3 FIQ Status Register
#define REG_FIQEN3                      0x0164        //RISC L3 FIQ Enable Register
 #define REG_FIQCL3                      0x0168        //RISC L3 FIQ Clear Register

//----------------------------------------------------------------------------
// IRQ Vectors
//----------------------------------------------------------------------------
#define REG_RW_BTIME_REG                0x0024        //RISC Timer 0 Limit Register
// Timer
#define REG_RW_TIMER0_LMT               0x0060        //RISC Timer 0 Limit Register
#define REG_T0LMT                       0x0060        //  The Same as above
#define REG_RW_TIMER0_COUNT             0x0064        //RISC Timer 0 Count Register
#define REG_T0                          0x0064        //  The Same as above
#define REG_RW_TIMER1_LMT               0x0068        //RISC Timer 1 Limit Register
#define REG_T1LMT                       0x0068        //  The Same as above
#define REG_RW_TIMER1_COUNT             0x006C        //RISC Timer 1 Count Register
#define REG_T1                          0x006C        //  The Same as above
#define REG_RW_TIMER2_LMT               0x0070        //RISC Timer 2 Limit Register
#define REG_T2LMT                       0x0070        //  The Same as above
#define REG_RW_TIMER2_COUNT             0x0074        //RISC Timer 2 Count Register
#define REG_T2                          0x0074        //  The Same as above

#define REG_RW_TIMER_CTRL               0x0078        //RISC Timer Control Register
  #define TMR0_CNTDWN_EN                  (1U << 0)   //Timer 0 Enable to Count Down
  #define TMR0_AUTOLD_EN                  (1U << 1)   //Timer 0 Auto-Load Enable
  #define TMR1_CNTDWN_EN                  (1U << 8)   //Timer 1 Enable to Count Down
  #define TMR1_AUTOLD_EN                  (1U << 9)   //Timer 1 Auto-Load Enable
  #define TMR2_CNTDWN_EN                  (1U << 16)  //Timer 2 Enable to Count Down
  #define TMR2_AUTOLD_EN                  (1U << 17)  //Timer 2 Auto-Load Enable

  #define TMR_CNTDWN_EN(x)                (1U << (x*8))
  #define TMR_AUTOLD_EN(x)                (1U << (1+(x*8)))

//  The Same as above
#define REG_TCTL                        0x78          // Timer control
  #define TCTL_T0EN                       (1 << 0)    // Timer 0 enable
  #define TCTL_T0AL                       (1 << 1)    // Timer 0 auto-load enable
  #define TCTL_T1EN                       (1 << 8)    // Timer 1 enable
  #define TCTL_T1AL                       (1 << 9)    // Timer 1 auto-load enable
  #define TCTL_T2EN                       (1 << 16)   // Timer 2 enable
  #define TCTL_T2AL                       (1 << 17)   // Timer 2 auto-load enable

//----------------------------------------------------------------------------
// Address SWAP
#define REG_RW_SWAP_RG0_BGN             0x0094        //Swap Region 0 Begin address register
#define REG_RW_SWAP_RG0_END             0x0098        //Swap Region 0 End address register
#define REG_RW_SWAP_RG1_BGN             0x009C        //Swap Region 1 Begin address register
#define REG_RW_SWAP_RG1_END             0x00A0        //Swap Region 1 End address register
#define REG_RW_SWAP_RG2_BGN             0x01D0        //Swap Region 2 Begin address register
#define REG_RW_SWAP_RG2_END             0x01D4        //Swap Region 2 End address register
#define REG_RW_SWAP_RG3_BGN             0x01D8        //Swap Region 3 Begin address register
#define REG_RW_SWAP_RG3_END             0x01DC        //Swap Region 3 End address register

#define REG_RW_SWAP_CTRL                0x00A4        //RISC Swap Control Register
  #define SWP_RG0_WREN                    (1U << 0)   //Region 0 address swap write enable
  #define SWP_RG0_RDEN                    (1U << 1)   //Region 0 address swap read enable
  #define SWP_RG1_WREN                    (1U << 2)   //Region 1 address swap write enable
  #define SWP_RG1_RDEN                    (1U << 3)   //Region 1 address swap read enable
  #define SWP_RG2_WREN                    (1U << 16)  //Region 2 address swap write enable
  #define SWP_RG2_RDEN                    (1U << 17)  //Region 2 address swap read enable
  #define SWP_RG3_WREN                    (1U << 18)  //Region 3 address swap write enable
  #define SWP_RG3_RDEN                    (1U << 19)  //Region 3 address swap read enable

//----------------------------------------------------------------------------
// General Purpose Register
#define REG_RW_GPRB0                    0x00E0        //RISC Byte General Purpose Register 0
                                                      // boot loader version
#define REG_RW_GPRB1                    0x00E4        //RISC Byte General Purpose Register 1
                                                      // ne upg version
#define REG_RW_GPRB2                    0x00E8        //RISC Byte General Purpose Register 2
#define REG_RW_GPRB3                    0x00EC        //RISC Byte General Purpose Register 3
#define REG_RW_GPRB4                    0x00F0        //RISC Byte General Purpose Register 4
#define REG_RW_GPRB5                    0x00F4        //RISC Byte General Purpose Register 5
#define REG_RW_GPRB6                    0x00F8        //RISC Byte General Purpose Register 6
#define REG_RW_GPRB7                    0x00FC        //RISC Byte General Purpose Register 7
#define REG_RW_GPRDW0                   0x0100        //RISC Double Word General Purpose Register 0
#define REG_RW_GPRDW1                   0x0104        //RISC Double Word General Purpose Register 1
#define REG_RW_LED                      0x010C        //Seven segment display
#define REG_RW_GPRDW2                   0x0120        //RISC Double Word General Purpose Register 2
#define REG_RW_GPRDW3                   0x0124        //RISC Double Word General Purpose Register 3
#define REG_RW_GPRDW4                   0x0110        //RISC Double Word General Purpose Register 4
#define REG_RW_GPRDW5                   0x0114        //RISC Double Word General Purpose Register 5
#define REG_RW_GPRDW6                   0x0118        //RISC Double Word General Purpose Register 6
#define REG_RW_GPRDW7                   0x011C        //RISC Double Word General Purpose Register 7
//Secure General Purpose Register
#define REG_RW_SGPRBDW0                 0x00B0        //Secure General Purpose Register 
#define REG_RW_SGPRBDW1                 0x00B4        //Secure General Purpose Register
#define REG_RW_SGPRBDW2                 0x00B8        //Secure General Purpose Register
#define REG_RW_SGPRBDW3                 0x00BC        //Secure General Purpose Register
//----------------------------------------------------------------------------
// MISC Reg
#define EXT_INT_CTRL_                   0x00A8
  #define TRST_SEL0                       (1<<0)      /* TRST of JTAG Selection for the 1st ARM */   // 8555 new add
  #define EXT_INT1_TRIGGER_POLARITY_H_    (1<<1)      /* active high or rising edge */
  #define EXT_INT1_LEVEL_TRIGGER_         (1<<2)      /* triggered by level */
  #define EXT_INT4_TRIGGER_POLARITY_H_    (1<<3)      /* active high or rising edge */
  #define EXT_INT4_LEVEL_TRIGGER_         (1<<4)      /* triggered by level */
  #define TRST_SEL0_SW                    (1<<5)      /* TRST of JTAG for the 1st ARM */             // 8555 new add
  #define TRST_SEL1                       (1<<6)      /* TRST of JTAG Selection for the 2st ARM */   // 8555 new add
  #define TRST_SEL1_SW                    (1<<7)      /* TRST of JTAG for the 2st ARM */             // 8555 new add
  #define EXT_INT1_SAMPLE_EN_             (1<<20)     /* sample enable */
  //#define EXT_INT2_CLEAR_                 (1<<18)     /* INT clear => non-useful */
  #define EXT_INT2_TRIGGER_POLARITY_H_    (1<<16)     /* active high or rising edge */
  #define EXT_INT2_LEVEL_TRIGGER_         (1<<17)     /* triggered by level */

//AC8317
#define REG_DEGCK_CFG                   0x740
  #define EINT_PRE_DIV_EN                    (1 << 12)
  #define EINT_PRE_DIV_MASK                  0xFFF
#define REG_EXTINT0_CFG                 0x744
#define REG_EXTINT1_CFG                 0x748
#define REG_EXTINT2_CFG                 0x74c
#define REG_EXTINT3_CFG                 0x750
#define REG_EXTINT4_CFG                 0x754
#define REG_EXTINT5_CFG                 0x758
#define REG_EXTINT6_CFG                 0x75c
#define REG_EXTINT7_CFG                 0x760
#define REG_EXTINT_CFG(a)              (REG_EXTINT0_CFG + (a) * 4)
  #define EINT_BYPASS_PRE_DIV           (1 << 14)
  #define EINT_EN                       (1 << 13)
  #define EINT_TYPE_MASK                (0x7 << 10)
  #define EINT_TYPE_POSEDGE             (0 << 10)
  #define EINT_TYPE_NEGEDGE             (1 << 10)
  #define EINT_TYPE_HIGHLEVEL             (2 << 10)
  #define EINT_TYPE_LOWLEVEL             (3 << 10)
  #define EINT_TYPE_DUALEDGE             (4 << 10)
  #define EINT_BYPASS_DEG                (1 << 9)
  #define EINT_POST_DIV_EN               (1 << 8)
  #define EINT_POST_DIV_MASK             0xFF


//----------------------------------------------------------------------------
#if 1 // no arm2
// DUAL CORE
#define REG_RW_MISC2                    0x00AC        //MISC2
  #define MISC2_RISC1_IO_ASYNC            (1U << 30)  //RISC1 IO ASYNC
  #define MISC2_JTAG_CFG_MASK	          0x3
  #define MISC2_JTAG_CFG_ICE2vsICE1	      3
  #define MISC2_JTAG_CFG_ICE1vsICE2	      2
  #define MISC2_JTAG_CFG_ICE2	          1
  #define MISC2_JTAG_CFG_ICE1	          0

#define REG_RW_TOCORISC                 0x0170        //To Co-RISC Interrupt Register
  #define TOCORISC_INTR                   (1U << 0)   //Interrupt To Co-RISC
  #define TOCORISC_INTR_CLR             (1U << 23)   //Interrupt Clear from Co-RISC //8555 remove from 8530

#define REG_RW_HSMPHE                   0x01B4        //Hardware Semaphore Register
  #define HSMPHE_UART1                    (1U << 0)   //Hardware Semaphore 0
  #define HSMPHE_DC_SMPHE                 (1U << 1)   //Hardware Semaphore 1 (for Dual Communication Semaphore)
  #define HSMPHE_NAND                     (1U << 2)   // NAND semaphore
  #define HSMPHE_SPEECH                   (1U << 3)   // Speech semaphore
  #define HSMPHE_SINFO4                   (1U << 4)   // SINFO4_REG semaphore
  #define HSMPHE_PNG                      (1U << 5)   // PNG semaphore
  #define HSMPHE_PWR_UP_CFG               (1U << 6)   // POWER UP REG semaphore
  #define HWSMPHE_MUTEX4USB0              (1U << 7)
  #define HWSMPHE_MUTEX4USB1              (1U << 8)
  #define HWSMPHE_MUTEX4MSDC0             (1U << 9)
  #define HWSMPHE_MUTEX4MSDC1             (1U << 10)
  #define HWSMPHE_MUTEX4MSDC2             (1U << 11)
  #define HWSMPHE_AUD_POWER_ON            (1U << 12)
  #define HWSMPHE_GPSAOUT                 (1U << 13)

#define REG_RW_RISCRST                  0x01B8        //RISC Reset Control Register
  #define RISCRST_RISC0_RESET             (1U << 0)   //Release Reset RISC 0
  #define RISCRST_RISC1_RESET             (1U << 1)   //Release Reset RISC 1
  #define RISCRST_PASSWD                  0x85208888  // Common Password that Force RISC into Reset State
  #define RISCRST_RISC0_PASSWD            0x16880001  // Password that Force RISC0 into Reset State
  #define RISCRST_RISC1_PASSWD            0x16880002  // Password that Force RISC0 into Reset State

#define REG_RO_WALE                     0x01BC        //WALE Status Register
  #define WALE_DWBUSY                     (1U << 0)   //Write Data is Busy
  #define WALE_DMAWBUSY                   (1U << 1)   //Write DMA is Busy
#endif

#define REG_L2_MON          0x01E0
#define REG_L2_INT          0x01E4
//----------------------------------------------------------------------------
// DEBUG
#define REG_RO_DBGDO                    0x007C        //Debug Data Output Register
#define REG_RO_DBGDOSEL                 0x0080        //Debug Data Select Register
#define REG_RW_INTCFG                   0x01E0        //Interrupt configuration Register
#define REG_RO_EDEG_CNTR                0x01E4        //Edge Counter Register

//#define REG_RW_IABRT0ADR                0x0188        //Instruction Abort Address 0 Register
//#define REG_RW_IABRT1ADR                0x018C        //Instruction Abort Address 1 Register
#define REG_RW_DWABRT0ADR               0x0190        //Data Write Abort Address 0 Register
#define REG_RW_DRABRT0ADR               0x0198        //Data Read Abort Address 0 Register
#define REG_RW_DRABRT1ADR               0x019C        //Data Read Abort Address 1 Register

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

//----------------------------------------------------------------------------
#define REG_RW_DSL                      0x0184        //Delay Select Register

#define REG_RW_LZHS_CTL                 0x01C0        //LZHS Control Register
#define REG_RW_LZHS_ADR                 0x01C4        //LZHS Read Source Address Register
#define REG_RW_WDRAM_ADR                0x01C8        //LZHS Write DRAM Address Register
#define REG_RW_WDRAM_END                0x01CC        //LZHS Write DRAM End Address Register
//#define REG_RO_BIMDM_BEG                0x01F0        //Local Arbiter Begin
//#define REG_RO_BIMDM_TM0                0x01F4        //Arbitration Timer
//#define REG_RO_BIMDM_LEN_CHA            0x01F8        //Monitored Agent Bandwidth
//#define REG_RO_BIMDM_CYC_CHA            0x01FC        //Monitored Cycles
//#define REG_RO_BIMDM_LEN_CHB            0x0200        //Monitored Agent Bandwidth
//#define REG_RO_BIMDM_CYC_CHB            0x0204        //Monitored Cycles
#define REG_RO_LSS_BEG                  0x0208        //Local Arbiter Begin
#define REG_RO_LSS_TM0                  0x020C        //Arbitration Timer
#define REG_RO_LSS_LEN                  0x0210        //Monitored Agent Bandwidth
#define REG_RO_LSS_CYC                  0x0214        //Monitored Cycles
#define REG_RO_BIMIO_BEG                0x0218        //Local Arbiter Begin
#define REG_RO_BIMIO_TM0                0x021C        //Arbitration Timer
#define REG_RO_BIMIO_LEN                0x0220        //Monitored Agent Bandwidth
#define REG_RO_BIMIO_CYC                0x0224        //Monitored Cycles

//----------------------------------------------------------------------------
// Flash DMA
#define REG_RW_FDMACTL                  0x0718
  #define FDMACTL_HWSET_MODE              (1U << 4)
  #define FDMACTL_SOFT_RESET              (1U << 1)
  #define FDMACTL_DMA_TRIGGER             (1U << 0)
#define REG_RW_FDMADADR                 0x0720
#define REG_RW_FDMADENDADR              0x0724

//----------------------------------------------------------------------------
// 64b_TIMER
#define BIM_64b_TIMER_NUM               2
#define REG_RW_T64b_LO_0                0x0728
#define REG_RW_T64b_HI_0                0x072C
#define REG_RW_T64b_EN_0                0x0730
#define REG_RW_T64b_LO_1                0x0734
#define REG_RW_T64b_HI_1                0x0738
#define REG_RW_T64b_EN_1                0x073C

//----------------------------------------------------------------------------
// Secure BIM
#define DMPROT_BGN                      0x0050
#define DMPROT_END                      0x0054
#define DMPROT_CTL                      0x0058
  #define WDM_SECURE_EN                   0x01
  #define RDM_SECURE_EN                   0x02
  #define WDM_SVC_EN                      0x04
  #define RDM_SVC_EN                      0x08

#define REG_RW_SEC_MISC2                0x0088

//----------------------------------------------------------------------------
// IOMMU
#define REG_RW_IOMMU_CFG0               0x000        // basic setting
#define REG_RW_IOMMU_CFG1               0x004        // page table index
#define REG_RW_IOMMU_CFG2               0x008        // agnet_0~1 setting
#define REG_RW_IOMMU_CFG3               0x00C        // agnet_2~3 setting
#define REG_RW_IOMMU_CFG4               0x010        // interrupt, monitor and debug
#define REG_RW_IOMMU_CFG5               0x014        // perfomance meter
#define REG_RW_IOMMU_CFG6               0x018        // monitor result
#define REG_RW_IOMMU_CFG7               0x01C        // monitor result
#define REG_RW_IOMMU_CFG8               0x020        // monitor result
// special setting *********************
#define REG_RW_IOMMU_CFG9               0x024        // over read protection
#define REG_RW_IOMMU_CFGA               0x028        // over read protection
#define REG_RW_IOMMU_CFGB               0x02C        // over read protection
#define REG_RW_IOMMU_CFGC               0x030        // over read protection
// special setting &&&&&&&&&&&&&&&&&&&&&


//============================================================================
// Type definitions
//============================================================================

//============================================================================
// Public functions
//============================================================================
EXTERN BOOL BIM_EnableIrq(UINT32 u4Vector);
EXTERN BOOL BIM_DisableIrq(UINT32 u4Vector);
EXTERN void BIM_Workaround(UINT32 u4Vector);
EXTERN BOOL BIM_IsIrqEnabled(UINT32 u4Vector);
EXTERN BOOL BIM_IsIrqPending(UINT32 u4Vector);
EXTERN BOOL BIM_ClearIrq(UINT32 u4Vector);

EXTERN BOOL BIM_EnableFiq(UINT32 u4Vector);
EXTERN BOOL BIM_DisableFiq(UINT32 u4Vector);
EXTERN BOOL BIM_IsFiqEnabled(UINT32 u4Vector);
EXTERN BOOL BIM_IsFiqPending(UINT32 u4Vector);
EXTERN BOOL BIM_ClearFiq(UINT32 u4Vector);

EXTERN BOOL BIM_GETHWSemaphore(UINT32 u4Number, UINT32 u4TimeOut);
EXTERN BOOL BIM_ReleaseHWSemaphore(UINT32 u4Number);

EXTERN void BIM_ClearIntFromARM1(void);
EXTERN void BIM_ClearIntFromARM2(void);


EXTERN void BIM_RegResetIrKey(UINT32 u4ResetIrM, UINT32 u4ResetIrL);
EXTERN void BIM_GetResetIrKey(UINT32* pu4ResetIrM, UINT32* pu4ResetIrL);
EXTERN void BIM_WatchDogIrReset(UINT32 u4Val);

#define RESET_MODE_NONE                 0            // no reset, while loop
#define RESET_MODE_AUTO                 1            // Automatically Reset
#define RESET_MODE_IR                   2            // IR Power Key Reset

EXTERN void BIM_SetSysHaltResetMode(UINT32 u4Mode);  // 0: no reset, while loop. 1: Automatically Reset. 2: IR Power Key Reset
EXTERN UINT32 BIM_GetSysHaltResetMode(void);

EXTERN BOOL BIM_AddrSwap(
    UINT32 u4Region,       ///< [IN] address swap region: 0~3
    UINT32 u4BeginAddr,    ///< [IN] address swap begin addrss
    UINT32 u4EndAddr,      ///< [IN] address swap end address
    UINT32 u4SwapMode      ///< [IN] address swap mode: 0/1/2, 0: Off
       );

EXTERN UINT32 BIM_WatchCounter(void);
EXTERN void BIM_WatchDog(UINT32 u4Val);

EXTERN UINT32 u4HalGetTTB0(void);
EXTERN UINT32 u4HalGetTTB1(void);
EXTERN UINT32 u4HalGetTTB(UINT32 ui4_mem_ptr);
EXTERN VOID vIOMMU_Performance(UINT32 ui4_type, UINT32 ui4_step);
EXTERN VOID vIOMMU_CheckSum(UINT32 ui4_step, UINT32 ui4_ag, UINT32 ui4_id);

EXTERN VOID BIM_SetEInt(UINT32 EIntNumber, UINT32 type, UINT32 debunceTime);
EXTERN VOID BIM_EnableEInt(UINT32 EIntNumber);
EXTERN VOID BIM_DisableEInt(UINT32 EIntNumber);

#endif  // X_BIM_83XX_H
