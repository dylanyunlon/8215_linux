/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

#ifndef X_BIM_83XX_H
#define X_BIM_83XX_H

//============================================================================
// Include files
//============================================================================
#include "x_types.h"
#include "generated/atc_project.h"
//#include "drv_config.h"
#if defined(CONFIG_ATC_PLATFORM_ac83xx)
#include "ac83xx_evb.h"
#include "x_hal_83xx.h"
#elif defined(CONFIG_ATC_PLATFORM_ac823x)
#include "ac823x_evb.h"
#else
#error "no platform defined!"
#endif
//============================================================================
// Macros for register read/write
//============================================================================
#define BIM_READ8(offset)            IO_READ8(BIM_BASE, offset)
#define BIM_READ16(offset)           IO_READ16(BIM_BASE, offset)
#define BIM_READ32(offset)           IO_READ32(BIM_BASE, offset)

#define BIM_WRITE8(offset, value)    IO_WRITE8(BIM_BASE, offset, (value))
#define BIM_WRITE16(offset, value)   IO_WRITE16(BIM_BASE, offset, (value))
#define BIM_WRITE32(offset, value)   IO_WRITE32(BIM_BASE, offset, (value))

#define BIM_REG8(offset)             IO_REG8(BIM_BASE, offset)
#define BIM_REG16(offset)            IO_REG16(BIM_BASE, offset)
#define BIM_REG32(offset)            IO_REG32(BIM_BASE, offset)

#define BIM2_READ8(offset)           IO_READ8(BIM_1_BASE, offset)
#define BIM2_READ16(offset)          IO_READ16(BIM_1_BASE, offset)
#define BIM2_READ32(offset)          IO_READ32(BIM_1_BASE, offset)

#define BIM2_WRITE8(offset, value)   IO_WRITE8(BIM_1_BASE, offset, (value))
#define BIM2_WRITE16(offset, value)  IO_WRITE16(BIM_1_BASE, offset, (value))
#define BIM2_WRITE32(offset, value)  IO_WRITE32(BIM_1_BASE, offset, (value))

#define BIM2_REG8(offset)            IO_REG8(BIM_1_BASE, offset)
#define BIM2_REG16(offset)           IO_REG16(BIM_1_BASE, offset)
#define BIM2_REG32(offset)           IO_REG32(BIM_1_BASE, offset)

#define SBIM_READ8(offset)           IO_READ8(SBIM_BASE, offset)
#define SBIM_READ16(offset)          IO_READ16(SBIM_BASE, offset)
#define SBIM_READ32(offset)          IO_READ32(SBIM_BASE, offset)

#define SBIM_WRITE8(offset, value)   IO_WRITE8(SBIM_BASE, offset, (value))
#define SBIM_WRITE16(offset, value)  IO_WRITE16(SBIM_BASE, offset, (value))
#define SBIM_WRITE32(offset, value)  IO_WRITE32(SBIM_BASE, offset, (value))

//============================================================================
// Macros for memory read/write
//============================================================================
#define WRITEMEM8(Address, Value)    *(volatile UINT8 *)(Address) = Value
#define READMEM8(Address)            *(volatile UINT8 *)(Address)

#define WRITEMEM16(Address, Value)   *(volatile UINT16 *)(Address) = Value
#define READMEM16(Address)           *(volatile UINT16 *)(Address)

#define WRITEMEM32(Address, Value)   *(volatile UINT32 *)(Address) = Value
#define READMEM32(Address)           *(volatile UINT32 *)(Address)

//============================================================================
// Boot Loader Version Define
//============================================================================
#define REG_RW_BOOTLOADER_VERSION       REG_RW_GPRB0
#define REG_RW_NE_UPG_VERSION           REG_RW_GPRB1

//============================================================================
// SBIM Registers
//============================================================================
#define REG_DMPROT_BGN                  0x0000
#define REG_DMPROT_END                  0x0004
#define REG_DMPROT_CTL                  0x0008
#define REG_CP15SDISABLE                0x000C
  #define DM1PROTCFGDISABLE             (1U << 17)
  #define DM0PROTCFGDISABLE             (1U << 16)
  #define ICE_DISABLE_RISC_1            (1U << 3)
  #define ICE_DISABLE_RISC_2            (1U << 2)

//============================================================================
// BIM Registers
//============================================================================
#define REG_RO_ICE          0x0000        //RISC ICE Register
#define REG_RW_ADDREN       0x0004        //RISC Address Enable Register
  #define IOBASE_EN           (1U << 0)   //IO Base Address Space Enable
  #define FIOBASE_EN          (1U << 1)   //Fast IO Base Address Space Enable
  #define PBIB_EN             (1U << 2)   //RISC PBI-B Base Address Space Enable
  #define PBIA_EN             (1U << 3)   //RISC PBI-A Base Address Space Enable
  #define DRAMA_EN            (1U << 4)   //RISC DRAM-A Base Address Space Enable
  #define DRAMB_EN            (1U << 5)   //RISC DRAM-B Base Address Space Enable
  #define ROM_EN              (1U << 6)   //RISC ROM Base Address Space Enable
#define REG_RW_PBIA_BA      0x0008        //PBI-A Base Address Register
#define REG_RW_PBIB_BA      0x000C        //PBI-B Base Address Register
#define REG_RW_DRAMA_BA     0x0010        //DRAM-A Base Address Register
#define REG_RW_DRAMB_BA     0x0014        //DRAM-B Base Address Register
#define REG_RW_IOBASE_BA    0x0018        //IO Base Address Register
#define REG_RW_REMAP        0x001C        //Remap Register
  #define REMAP_BIT           (1U << 0)   //Remap Bit
#define REG_RW_DRAMB_OFF    0x0020        //DRAM-B Offset Address Register
#define REG_RW_BUSTIME      0x0024        //BUS Access Timing Register

//----------------------------------------------------------------------------
// IRQ/FIQ
#define REG_IRQST           0x0100        //RISC L1 IRQ Status Register
#define REG_IRQEN           0x0104        //RISC L1 IRQ Enable Register
#define REG_FIQST           0x010C        //RISC L1 FIQ Status Register
#define REG_FIQEN           0x0100        //RISC L1 FIQ Enable Register

#define REG_IRQST2          0x0110        //RISC L2 IRQ Status Register
#define REG_IRQEN2          0x0114        //RISC L2 IRQ Enable Register
#define REG_FIQST2          0x0118        //RISC L2 FIQ Status Register
#define REG_FIQEN2          0x011c        //RISC L2 FIQ Enable Register


#define REG_IRQST3          0x0120        //RISC L3 IRQ Status Register
#define REG_IRQEN3          0x0124        //RISC L3 IRQ Enable Register
#define REG_FIQST3          0x0128        //RISC L3 FIQ Status Register
#define REG_FIQEN3          0x012c        //RISC L3 FIQ Enable Register


#define REG_IRQST4          0x0130        //RISC L4 IRQ Status Register
#define REG_IRQEN4          0x0134        //RISC L4 IRQ Enable Register
#define REG_FIQST4          0x0138        //RISC L4 FIQ Status Register
#define REG_FIQEN4          0x013c        //RISC L4 FIQ Enable Register

#define REG_IRQCL           0x0038        //RISC L1 IRQ Clear Register
#define REG_FIQCL           0x0044        //RISC L1 FIQ Clear Register
#define REG_IRQCL2          0x0050        //RISC L2 IRQ Clear Register
#define REG_FIQCL2          0x014C        //RISC L2 FIQ Clear Register
#define REG_IRQCL3          0x0068        //RISC L3 IRQ Clear Register
#define REG_FIQCL3          0x0168        //RISC L3 FIQ Clear Register
#define REG_IRQCL4          0x0238        //RISC L3 IRQ Clear Register
#define REG_FIQCL4          0x0244        //RISC L3 FIQ Clear Register



#if 1
//----------------------------------------------------------------------------
// IRQ Vectors
  #define   VECTOR_GRPC                 0
  #define   VECTOR_GRPB                 1
  #define   VECTOR_GRPD           2
  #define   VECTOR_EXT                  3
  #define   VECTOR_FLASHCARD            4
  #define   VECTOR_EXT2                 5
  #define   VECTOR_VDOIN                6   //VECTOR_DEMUX in 83xx
  #define   VECTOR_GRAPH                7
  #define   VECTOR_JPGDEC               8
  //#define   VECTOR_FONT                 9
  //#define   VECTOR_IOMMU                10
  #define   VECTOR_DDMANEW              11
  #define   VECTOR_JAVA                 12
  #define   VECTOR_RLE                  13
  #define   VECTOR_RS232_1              14   //83xx
  #define   VECTOR_DSP                  15   //83xx DSPA2RC
  #define   VECTOR_SPD                  16   //83xx SPDF_RC
  //#define   VECTOR_DISP_VSYNC           17
  #define   VECTOR_VDOUTREAR            17
  #define   VECTOR_SVO_IFINT            18
  #define   VECTOR_SVOIF                18
  #define   VECTOR_SFDMAI               19
  #define   VECTOR_DDMAI                20
  #define   VECTOR_PL310                21
  #define   VECTOR_USB                  22
  #define   VECTOR_TOCORISC             23
  #define   VECTOR_AXI64_WR             24
  #define   VECTOR_T2                   25
  #define   VECTOR_T1                   26
  #define   VECTOR_T0                   27
  #define   VECTOR_DSPC                 28    //83xx DSPC2RC
 // #define   VECTOR_SVO_FE1INT           29
  #define VECTOR_YPBPRINT    29
  #define   VECTOR_SVO_FE0INT           30
  #define   VECTOR_VSYNC                31
  //===============================================
  #define   VECTOR_GCPU                 32
  #define   VECTOR_NFI                  33
  #define   VECTOR_PWMIP_2              34
  #define   VECTOR_PWMIP_1              35
  #define   VECTOR_SPDIF                36
  #define   VECTOR_NR                   37
  #define   VECTOR_CEC                  38  //(VECTOR_CEC=VECTOR_AVLNK)
  #define   VECTOR_EXT4                 39
  #define   VECTOR_DRAMC                40
  #define   VECTOR_RESIZER2             41
  #define   VECTOR_RESIZER1             42
  #define   VECTOR_RESIZER0             43
  #define   VECTOR_HDMI                 44
  #define   VECTOR_PANEL_SCALER         45
  //#define   VECTOR_DISP_VSYNC           46  //83xx change to VECTOR 17
  #define   VECTOR_SPIINT               47
  #define   VECTOR_DDIINT               48
  #define   VECTOR_VDOUTAUX0            49
  #define   VECTOR_DMXINT               50
  #define   VECTOR_WCHNL                51
  #define   VECTOR_PWDNC                52
  #define   VECTOR_ATA1                 53   //There is only one ATA in 83xx
  //#define   VECTOR_(Reserved)           54
  #define   VECTOR_WCHNL2               55
  #define   VECTOR_SPU                  56
  #define   VECTOR_PVR                  57   //VECTOR_PVR=VECTOR_DEMUX
  #define   VECTOR_TSMUX                58
  #define   VECTOR_FMC                  59
  #define   VECTOR_FLASH                60
  #define   VECTOR_UART_3				62 	 //VECTOR_RS232_4              62   //83xx
  #define   VECTOR_UART_4 				61   //VECTOR_RS232_5              61   //83xx
  #define   VECTOR_LZHS                 63
  //===============================================
  #define   VECTOR_UART_5 				64    //VECTOR_RS232_6              64    //83xx
  #define   VECTOR_UART_2 				65    //VECTOR_RS232_3              65    //83xx
  #define   VECTOR_RS232_3 				65    
  #define   VECTOR_PARSER1              66
  #define   VECTOR_PARSER               67
  #define   VECTOR_DISP_END2            68
  #define   VECTOR_DRAMC1               69
  #define   VECTOR_UART_1 				70    //VECTOR_RS232_2              70    //83xx
  #define   VECTOR_RS232_2 				70    
  #define   VECTOR_PNG2                 71
  #define   VECTOR_PNG1                 72
  #define   VECTOR_GIF2                 73
  #define   VECTOR_GIF1                 74
  #define   VECTOR_PSTQ                 75
  #define   VECTOR_SRCQ                 76
  #define   VECTOR_OSD5_UNFLW           77
  #define   VECTOR_OSD4_UNFLW           78
  #define   VECTOR_OSD3_UNFLW           79
  #define   VECTOR_OSD2_UNFLW           80
  #define   VECTOR_OSD1_UNFLW           81
  #define   VECTOR_USB3                 82
  #define   VECTOR_DISP_END             83
  #define   VECTOR_TCPIP                84
  #define   VECTOR_USB2                 85
  #define   VECTOR_EPHY                 86
  #define   VECTOR_UART_6 				87    //VECTOR_RS232_7              87    //83xx
  #define   VECTOR_DISPVDO2_UNDERRUN    88
  #define   VECTOR_ENET                 89
  #define   VECTOR_VDLIT                90
  #define   VECTOR_VDFUL                91
  #define   VECTOR_DSPB2RISC            92       //83xx DSPB2RC
  #define   VECTOR_DISPVDO_UNDERRUN     93
  #define   VECTOR_PNG                  94
  #define   VECTOR_GIF                  95
  //===============================================
  //#define   VECTOR_EXT2                 96
  //#define   VECTOR_EXT4                 97
  #define   VECTOR_EXT5                 98  
  #define   VECTOR_EXT6                 99
  #define   VECTOR_EXT7                 100
  #define   VECTOR_PT110AP0             101
  #define   VECTOR_PT110AP1             102
  #define   VECTOR_PT110AP2             103
  #define   VECTOR_PT110AP3             104
  #define   VECTOR_UP2AP0               105
  #define   VECTOR_PWNFB                106
  #define   VECTOR_KPI                  107
  #define   VECTOR_TSI                  108
  #define   VECTOR_M3D                  109
  #define   VECTOR_RTC                  110
  #define   VECTOR_SPI1                 111
  #define   VECTOR_ASRC_AP              112  //83xx
  #define   VECTOR_ASRC_GPS             113  //83xx
  #define   VECTOR_PWM_REAR             114  //83xx
  #define   VECTOR_PWM_FRONT1           115  //83xx
  #define   VECTOR_PWM_FRONT2           116  //83xx
  #define   VECTOR_PWM_FRONT3	          117  //83xx
  #define   VECTOR_PWM_GPS              118  //83xx
  #define   VECTOR_AOUT_GPS_RC          119  //83xx
  #define   VECTOR_AOUT_2ND_RC          120  //83xx
  #define   VECTOR_MIC_RC               121  //83xx
  #define   VECTOR_AOUT_BT_RC           122  //83xx
  #define   VECTOR_MSDC0                123
  #define   VECTOR_MSDC1                124
  #define   VECTOR_MSDC2                125
  #define   VECTOR_LVDSTOP              126
  //#define   VECTOR_(Reserved)           127
  //#define   VECTOR_(Reserved)           128
  //===============================================
  #define   VECTOR_INT_P_GPIO0          128  //83xx
  #define   VECTOR_INT_P_GPIO1          129  //83xx
  #define   VECTOR_INT_P_GPIO2          130  //83xx
  #define   VECTOR_INT_P_GPIO3          131  //83xx
  #define   VECTOR_INT_P_GPIO4          132  //83xx
  #define   VECTOR_INT_P_GPIO5          133  //83xx
  #define   VECTOR_INT_P_GPIO6          134  //83xx
  #define   VECTOR_INT_P_GPIO7          135  //83xx
  //#define   VECTOR_INT_P_xx             136  
  #define   VECTOR_INT_P_DBG_UART       137  //83xx
  #define   VECTOR_INT_P_SIFS           138  //83xx
  #define   VECTOR_INT_P_CEC            139  //83xx
  #define   VECTOR_INT_P_ETNET          140  //83xx
  #define   VECTOR_INT_P_IR             141  //83xx
  #define   VECTOR_INT_P_SIFM           142  //83xx
  #define   VECTOR_INT_P_DDCCI          143  //83xx

#endif
//----------------------------------------------------------------------------
#define REG_RW_BTIME_REG   0x0024        //RISC Timer 0 Limit Register
// Timer
#define REG_RW_TIMER0_LMT   0x0060        //RISC Timer 0 Limit Register
#define REG_T0LMT           0x0060        //  The Same as above
#define REG_RW_TIMER0_COUNT 0x0064        //RISC Timer 0 Count Register
#define REG_T0              0x0064        //  The Same as above
#define REG_RW_TIMER1_LMT   0x0068        //RISC Timer 1 Limit Register
#define REG_T1LMT           0x0068        //  The Same as above
#define REG_RW_TIMER1_COUNT 0x006C        //RISC Timer 1 Count Register
#define REG_T1              0x006C        //  The Same as above
#define REG_RW_TIMER2_LMT   0x0070        //RISC Timer 2 Limit Register
#define REG_T2LMT           0x0070        //  The Same as above
#define REG_RW_TIMER2_COUNT 0x0074        //RISC Timer 2 Count Register
#define REG_T2              0x0074        //  The Same as above

#define REG_RW_TIMER_CTRL   0x0078        //RISC Timer Control Register
  #define TMR0_CNTDWN_EN      (1U << 0)   //Timer 0 Enable to Count Down
  #define TMR0_AUTOLD_EN      (1U << 1)   //Timer 0 Auto-Load Enable
  #define TMR1_CNTDWN_EN      (1U << 8)   //Timer 1 Enable to Count Down
  #define TMR1_AUTOLD_EN      (1U << 9)   //Timer 1 Auto-Load Enable
  #define TMR2_CNTDWN_EN      (1U << 16)  //Timer 2 Enable to Count Down
  #define TMR2_AUTOLD_EN      (1U << 17)  //Timer 2 Auto-Load Enable

  #define TMR_CNTDWN_EN(x)    (1U << (x*8))
  #define TMR_AUTOLD_EN(x)    (1U << (1+(x*8)))

//  The Same as above
#define REG_TCTL            0x78          // Timer control
  #define TCTL_T0EN           (1 << 0)    // Timer 0 enable
  #define TCTL_T0AL           (1 << 1)    // Timer 0 auto-load enable
  #define TCTL_T1EN           (1 << 8)    // Timer 1 enable
  #define TCTL_T1AL           (1 << 9)    // Timer 1 auto-load enable
  #define TCTL_T2EN           (1 << 16)   // Timer 2 enable
  #define TCTL_T2AL           (1 << 17)   // Timer 2 auto-load enable

//----------------------------------------------------------------------------
// Address SWAP
#define REG_RW_SWAP_RG0_BGN 0x0094        //Swap Region 0 Begin address register
#define REG_RW_SWAP_RG0_END 0x0098        //Swap Region 0 End address register
#define REG_RW_SWAP_RG1_BGN 0x009C        //Swap Region 1 Begin address register
#define REG_RW_SWAP_RG1_END 0x00A0        //Swap Region 1 End address register
#define REG_RW_SWAP_RG2_BGN 0x01D0        //Swap Region 2 Begin address register
#define REG_RW_SWAP_RG2_END 0x01D4        //Swap Region 2 End address register
#define REG_RW_SWAP_RG3_BGN 0x01D8        //Swap Region 3 Begin address register
#define REG_RW_SWAP_RG3_END 0x01DC        //Swap Region 3 End address register

#define REG_RW_SWAP_CTRL    0x00A4        //RISC Swap Control Register
  #define SWP_RG0_WREN        (1U << 0)   //Region 0 address swap write enable
  #define SWP_RG0_RDEN        (1U << 1)   //Region 0 address swap read enable
  #define SWP_RG1_WREN        (1U << 2)   //Region 1 address swap write enable
  #define SWP_RG1_RDEN        (1U << 3)   //Region 1 address swap read enable
  #define SWP_RG2_WREN        (1U << 16)  //Region 2 address swap write enable
  #define SWP_RG2_RDEN        (1U << 17)  //Region 2 address swap read enable
  #define SWP_RG3_WREN        (1U << 18)  //Region 3 address swap write enable
  #define SWP_RG3_RDEN        (1U << 19)  //Region 3 address swap read enable

//----------------------------------------------------------------------------
// General Purpose Register
#define REG_RW_GPRB0        0x00E0        //RISC Byte General Purpose Register 0
                                          // boot loader version
#define REG_RW_GPRB1        0x00E4        //RISC Byte General Purpose Register 1
                                          // ne upg version
#define REG_RW_GPRB2        0x00E8        //RISC Byte General Purpose Register 2
                                          // print on/off
#define REG_RW_GPRB3        0x00EC        //RISC Byte General Purpose Register 3
#define REG_RW_GPRB4        0x00F0        //RISC Byte General Purpose Register 4
#define REG_RW_GPRB5        0x00F4        //RISC Byte General Purpose Register 5
#define REG_RW_GPRB6        0x00F8        //RISC Byte General Purpose Register 6
#define REG_RW_GPRB7        0x00FC        //RISC Byte General Purpose Register 7
#define REG_RW_GPRDW0       0x0100        //RISC Double Word General Purpose Register 0
#define REG_RW_GPRDW1       0x0104        //RISC Double Word General Purpose Register 1
#define REG_RW_GPRDW2       0x0120        //RISC Double Word General Purpose Register 2
#define REG_RW_GPRDW3       0x0124        //RISC Double Word General Purpose Register 3
#define REG_RW_GPRDW4       0x0110        //RISC Double Word General Purpose Register 4
#define REG_RW_GPRDW5       0x0114        //RISC Double Word General Purpose Register 5
#define REG_RW_GPRDW6       0x0118        //RISC Double Word General Purpose Register 6
#define REG_RW_GPRDW7       0x011C        //RISC Double Word General Purpose Register 7

//----------------------------------------------------------------------------
// MISC Reg
#define EXT_INT_CTRL_                        0x00A8
    #define EXT_INT1_TRIGGER_POLARITY_H_     (1<<1)  /* active high or rising edge */
    #define EXT_INT1_LEVEL_TRIGGER_          (1<<2)  /* triggered by level */
    #define EXT_INT1_SAMPLE_EN_              (1<<23) /* sample enable */
    //#define EXT_INT2_CLEAR_                  (1<<18) /* INT clear => non-useful */
    #define EXT_INT2_TRIGGER_POLARITY_H_     (1<<16)  /* active high or rising edge */
    #define EXT_INT2_LEVEL_TRIGGER_          (1<<17)  /* triggered by level */

//----------------------------------------------------------------------------
// DUAL CORE
#define REG_RW_MISC2        0x00AC        //MISC2
  #define MISC2_RISC1_IO_ASYNC (1U << 30) //RISC1 IO ASYNC
  #define MISC2_JTAG_CFG_MASK	0x3
  #define MISC2_JTAG_CFG_ICE2vsICE1	3
  #define MISC2_JTAG_CFG_ICE1vsICE2	2
  #define MISC2_JTAG_CFG_ICE2	1
  #define MISC2_JTAG_CFG_ICE1	0

#define REG_RW_TOCORISC     0x0098        //To Co-RISC Interrupt Register
  #define TOCORISC_INTR       (1U << 4)   //Interrupt To Co-RISC
#define REG_RW_IRQCLR       0x0140
  #define TOCORISC_INTR_CLR   (1U << 23)   //Interrupt Clear from Co-RISC
  #define T0C_INTR_CLR   (1U << 27)
  #define T1C_INTR_CLR   (1U << 26)
  #define T2C_INTR_CLR   (1U << 25)

#define REG_RW_HSMPHE       0x01B4        //Hardware Semaphore Register
  #define HSMPHE_UART1        (1U << 0)   //Hardware Semaphore 0
  #define HSMPHE_DC_SMPHE     (1U << 1)   //Hardware Semaphore 1 (for Dual Communication Semaphore)
  #define HSMPHE_NAND         (1U << 2)   // NAND semaphore
  #define HSMPHE_SPEECH       (1U << 3)   // Speech semaphore
  #define HSMPHE_SINFO4       (1U << 4)   // SINFO4_REG semaphore

  #define HWSMPHE_AUD_POWER_ON            (1U << 12)
  #define HWSMPHE_GPSAOUT                 (1U << 13)

#define REG_RW_RISCRST      0x01B8        //RISC Reset Control Register
  #define RISCRST_RISC0_RESET (1U << 0)   //Release Reset RISC 0
  #define RISCRST_RISC1_RESET (1U << 1)   //Release Reset RISC 1
  #define RISCRST_PASSWD    0x85208888    // Common Password that Force RISC into Reset State
  #define RISCRST_RISC0_PASSWD    0x16880001    // Password that Force RISC0 into Reset State
  #define RISCRST_RISC1_PASSWD    0x16880002    // Password that Force RISC0 into Reset State

#define REG_RO_WALE         0x01BC        //WALE Status Register
  #define WALE_DWBUSY         (1U << 0)   //Write Data is Busy
  #define WALE_DMAWBUSY       (1U << 1)   //Write DMA is Busy

//----------------------------------------------------------------------------
// DEBUG
#define REG_RW_INTCFG       0x01E0        //Interrupt configuration Register
#define REG_RO_EDEG_CNTR    0x01E4        //Edge Counter Register

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
// Flash DMA
#define REG_RW_FDMACTL      0x0718
  #define FDMACTL_HWSET_MODE  (1U << 2)
  #define FDMACTL_SOFT_RESET  (1U << 1)
  #define FDMACTL_DMA_TRIGGER (1U << 0)
#define REG_RW_FDMADADR     0x0720
#define REG_RW_FDMADENDADR  0x0724

//----------------------------------------------------------------------------
// 64b_TIMER
#define BIM_64b_TIMER_NUM  2
#define REG_RW_T64b_LO_0   0x0728
#define REG_RW_T64b_HI_0   0x072C
#define REG_RW_T64b_EN_0   0x0730
#define REG_RW_T64b_LO_1   0x0734
#define REG_RW_T64b_HI_1   0x0738
#define REG_RW_T64b_EN_1   0x073C

//----------------------------------------------------------------------------
// Secure BIM
#define DMPROT_BGN          0x0000
#define DMPROT_END          0x0004
#define DMPROT_CTL          0x0008
  #define WDM_SECURE_EN       0x01
  #define RDM_SECURE_EN       0x02
  #define WDM_SVC_EN          0x04
  #define RDM_SVC_EN          0x08

#define T64B_GET_LOW()	BIM_READ32(REG_RW_T64b_LO_0)
#define T64B_GET_HIGH()	BIM_READ32(REG_RW_T64b_HI_0)  

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

#define RESET_MODE_NONE 0     // no reset, while loop
#define RESET_MODE_AUTO 1     // Automatically Reset
#define RESET_MODE_IR   2     // IR Power Key Reset

EXTERN void BIM_SetSysHaltResetMode(UINT32 u4Mode); // 0: no reset, while loop. 1: Automatically Reset. 2: IR Power Key Reset
EXTERN UINT32 BIM_GetSysHaltResetMode(void);

EXTERN BOOL BIM_AddrSwap(
	UINT32 u4Region,    ///< [IN] address swap region: 0~3
       UINT32 u4BeginAddr, ///< [IN] address swap begin addrss
       UINT32 u4EndAddr,   ///< [IN] address swap end address
       UINT32 u4SwapMode   ///< [IN] address swap mode: 0/1/2, 0: Off
       );

EXTERN UINT32 BIM_WatchCounter(void);
EXTERN void BIM_WatchDog(UINT32 u4Val);

#endif  // X_BIM_83XX_H
