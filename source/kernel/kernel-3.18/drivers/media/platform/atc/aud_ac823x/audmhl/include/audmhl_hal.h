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
/*----------------------------------------------------------------------------*
 * $RCSfile: audmhl_hal.h,v $
 * $Revision: #2 $
 * $Date: 2016/03/01 $
 * $Author: lingbo.liu $
 * Description: This header file contains hal define.
 *---------------------------------------------------------------------------*/

#ifndef _AUDMHL_HAL_H_
#define _AUDMHL_HAL_H_

#if CONFIG_DRV_HDMI_RX

#ifndef AUD_REG_OFST
#define AUD_REG_OFST    0x00   //0x000 >>2
#endif
#define AUDIN_REG_OFST  0x80   //0x200 >> 2
#define AUDIN2_REG_OFST 0xC0   //0x300 >> 2
//
// Spdif in H/W Register
//0x5018: SPDIF Sample Frequency Detection
#define RW_SPDIFIN_FS_DET           (AUD_REG_OFST+ (0x18>>2))
//0x501C : SPDIF/Line In Buffer Block
#define RW_SPDIFIN_BLK              (AUD_REG_OFST+ (0x1C>>2))
//0x5020 : SPDIF/Line In Control
#define RW_SPDIFIN_CTRL             (AUD_REG_OFST+ (0x20>>2))
// 0x5024 : SPDIF Type Detection
#define RW_SPDIFIN_TYPE             (AUD_REG_OFST+ (0x24>>2))
// 0x50C4 : Audio Input Hardware Configuration
#define RW_AUDIN_CFG                (AUD_REG_OFST+ (0xC4>>2))


//Select the  interface for spdifin or linein
#define SPL_SEL_MASK                ((u32)0x1 << 20)
// the interface is line in
#define LINEIN_SEL                  ((u32)0x0 << 20)
// the interface is spdif in
#define SPD_SEL                     ((u32)0x1 << 20)

//0x50B0 : Audio Buffer 0 Pointer or SPDIF/Line Buffer Pointer
//These registers specify a 24-bit byte address of audio buffer 
//pointer or SPDIF/Line-in buffer pointer.Writing 1 to bit 0, RISC will 
//read SPDIF/Line-in buffer pointer, else parser buffer pointer.
#ifndef RW_ABUF0_PNT
#define RW_ABUF0_PNT        (AUD_REG_OFST+ (0xB0>>2))
#endif

//5080
#define RW_DSP_SW_BS0_SBLK (AUD_REG_OFST + (0x80>>2))

//(0x1<<31) //HW mode: 0, SW mode: 1 //irlian: don't use 0x1<<31 to avoid warning
#define BS0_HW_SW_MOD_SEL     0x80000000 
//5084
#define RW_DSP_SW_BS0_EBLK    (AUD_REG_OFST + (0x84>>2))
//5088 : Bit Stream Buffer 0 Parser Pointer
#define RW_DSP_SW_BS0_PPNT    (AUD_REG_OFST + (0x88>>2))
//502C
#define RW_ENV_BAK            (AUD_REG_OFST + (0x2C >> 2))

// Multiple Line In H/W Register
// 0x5230 :
#define RW_MULTI_LIN_BANK (AUDIN_REG_OFST+ (0x30>>2))
//#define SP_LINEIN_BANK   0x3F

#define SPDF_DETECTION_MSK        ((u32)0x1 << 0)
#define SPDF_DETECTION_SHORT      ((u32)0x0 << 0)
//SPDF_DETECTION_LONG for EAC-3
#define SPDF_DETECTION_LONG       ((u32)0x1 << 0)


#define MUTLILINE_DETECTION_MSK   ((u32)0x1 << 1)
#define MUTLILINE_DETECTION_SHORT ((u32)0x0 << 1)

//MUTLILINE_DETECTION_LONG for EAC-3
#define MUTLILINE_DETECTION_LONG  ((u32)0x1 << 1) 

#define RW_SPDIFIN_CFG1               (AUDIN_REG_OFST+ (0x84>>2))
#define MULTI_SEL_DATA_MASK            ((u32)0x1 << 2)
#define MULTI_SEL_DATA_LINE            ((u32)0x0 << 2)
#define MULTI_SEL_DATA_INTERNAL_SPDF   ((u32)0x1 << 2)
#define MULTI_SEL_CLOCK_MASK           ((u32)0x1 << 3)
#define MULTI_SEL_CLOCK_LINE           ((u32)0x0 << 3)
#define MULTI_SEL_CLOCK_INTERNAL_SPDF  ((u32)0x1 << 3)

//0x5020 b16-b26
#define RW_DSP_SP_MULTI_BANK  (AUD_REG_OFST+ (0x20>>2))

//0x53E8 : line in multi cfg for Start/End Address
#ifndef RW_DSP_MULTI_LIN_BLK
#define RW_DSP_MULTI_LIN_BLK (AUDIN2_REG_OFST+ (0xE8>>2))
#endif

//0x53EC : line in multi control
#ifndef RW_DSP_MULTI_LIN_CTL
#define RW_DSP_MULTI_LIN_CTL (AUDIN2_REG_OFST+ (0xEC>>2))
#endif
#define BCLK_PAD_VOUTHSYNC             ((u32)0x1 << 10)
#define SDATA_PAD_VOUTD0               ((u32)0x1 << 11)
#define MCLK_PAD_VOUTCLK1              ((u32)0x1 << 12)
#define BCKLRCK_SEL_0_MASK             ((u32)0x1 << 12)
#define BCKLRCK_DECIDED_BY_BIT10       ((u32)0x0 << 12)
#define BCKLRCK_AOUT2_OR_ISPDIFRX      ((u32)0x1 << 12)
#define SDATA0_SEL_0_MASK              ((u32)0x1 << 13)
#define BCKLRCK_DECIDED_BY_BIT11       ((u32)0x0 << 13)
#define BCKLRCK_LIN_SDATA              ((u32)0x1 << 13)
#define SPLIN_MULTI_AO_LOOP_SEL        ((u32)0x1 << 15)

//0x53F0 : Multiple Line In Hardware Configuration
#ifndef RW_DSP_AIN_ACK_CFG
#define RW_DSP_AIN_ACK_CFG  (AUDIN2_REG_OFST+(0xF0>>2))
#endif

#define AINACK_CFG_ADDR_UPDATE_MASK ((u32)0x1<< 5)
#define AINACK_CFG_DRAM_ALE         ((u32)0x0<< 5)    // 0 : with DRAM ALE
#define AINACK_CFG_SMP_CNT          ((u32)0x1<< 5)    // 1 : with input sample count


//0: no effect (SPDIF), 1: interrupt select multi
#define AINACK_CFG_INT_MASK         ((u32)0x1<< 6)
//SPDIF INT
#define AINACK_CFG_INT_SPD          ((u32)0x0<< 6)
//Multiline INT
#define AINACK_CFG_INT_MUTLI        ((u32)0x1<< 6)    

#define AINACK_HBRMOD_MASK          ((u32)0x1<< 18)   //HBR Mode Mask
#define AINACK_NONHBR_MODE          ((u32)0x0<< 18)   //non-HBR Mode : L0L1L2L3R0R1R2R3
#define AINACK_HBR_MODE             ((u32)0x1<< 18)   //HBR Mode : L0R0L1R1L2R2L3R3
#define AINACK_CFG_IN_MODE          ((u32)0x1<< 20)   //Input Mode Selection
#define AINACK_CFG_PCM_MODE         ((u32)0x0<< 20)   //PCM Mode
#define AINACK_CFG_DSD_MODE         ((u32)0x1<< 20)   //DSD Mode

#define RW_MULTILINE_SPDIF_TYPE         (AUDIN2_REG_OFST+(0xF4>>2))
#define MULTILINE_SPDIF_TYPE_DEC_MASK   ((u32)0x1<< 10)
#define MULTILINE_SPDIF_TYPE_DEC_RST    ((u32)0x0<< 10)


#define AUDIO_IN_BUF_SIZE      (2*1024*1024)
#define MULTILINEIN_INT_VECTOR  VECTOR_SPD

//=============== definition for multiple line in ================
// for 0x5020h and 0x53ECh
#define HW_EN_MASK          ((u32)0x1 << 0)
#define HW_EN               ((u32)0x1 << 0)
#define HW_DISABLE          ((u32)0x0 << 0)
#define DATA_BIT_MASK       ((u32)0x1<< 1)  // bit 1 : 0 : 16-bit, 1 : 24-bit
#define BIT_16              ((u32)0x0<< 1)  // 16-bit
#define BIT_24              ((u32)0x1<< 1)  // 24-bit
#define INTR_PERIOD_MASK    ((u32)0x3<< 4)  //  Interrupt period
#define INTR_PERIOD_DISABLE ((u32)0x0<< 4)  //0<<4  : SPDIF In , disable
#define INTR_PERIOD_32      ((u32)0x0<< 4)  //0<<4  : Multiple Line In , 32 double words
#define INTR_PERIOD_64      ((u32)0x1<< 4) // 1<<4
#define INTR_PERIOD_128     ((u32)0x2<< 4) // 2<<4
#define INTR_PERIOD_256     ((u32)0x3<< 4) // 3<<4
#define DATA_SWAP_MASK      ((u32)0x1<< 3)// 1<<3
#define DATA_NON_SWAP       ((u32)0x0<< 3)// 0<<3
#define DATA_SWAP           ((u32)0x1<< 3)// 1<<3
// 1 <<6: Multiple Line In
#define SPDIF_PRE_DETECT    ((u32)0x1<< 6)
// 1 <<8: SPDIF In Select SPDIF/Line-in write pointer as getbs parser pointer
#define AUDIO_PRS_SEL       ((u32)0x1<< 8)  
//  Note: SPLIN_BLK should be equal to bit stream start and end blocks.
#define PNT_SEL0            ((u32)0x1<< 8) // 1 <<8
#define PNT_SEL1            ((u32)0x1<< 9)// 1 <<9




// for 0x5024h and 0x53F4h
#define SPDIFIN_DETAIL                ((u32)0x1F<<0)    // Detail type for IEC61937 RAW data
#define SPDIFIN_BSNUM                 ((u32)0x7<<5)    // Bit stream number for IEC61937 RAW data
#define SPDIFIN_ROUGH                 ((u32)0x3<<8)    //Rough type of the SPDIF input bit stream  0  PCM. 1  RAW (encoded) data. Detail types will be in bits 4~0.
//  2  The type is 16-bit DTS-CD.  3   The type is 14-bit DTS-CD.
#define SPDIFIN_TYPE_DEC              ((u32)0x1<<10)  //  SPDIF bit stream type decided or not.

// Burst Info Pc :
#define SPDIF_RAW_NULL                       0
#define SPDIF_RAW_AC3                        1
#define SPDIF_RAW_TIMESTAMP                  2
#define SPDIF_RAW_MP1L1                      4
#define SPDIF_RAW_MP1L23_MP2_WOEXT           5
#define SPDIF_RAW_MP2_WEXT                   6
#define SPDIF_RAW_MP2_AAC1                   7 // Reserved
#define SPDIF_RAW_MP2L1_LSF                  8
#define SPDIF_RAW_MP2L23_LSF                 9
#define SPDIF_RAW_DTS_I                      11
#define SPDIF_RAW_DTS_II                     12
#define SPDIF_RAW_DTS_III                    13
#define SPDIF_RAW_DTS_IV                     17
#define SPDIF_RAW_EAC3                       21
#define SPDIF_RAW_MP4_AAC                    27
#define SPDIF_RAW_MP2_AAC                    28
// HBR RAW, DolbyTrueHD RAW 0x16
#define HBR_RAW_HBR_MAT                      22 

//Use multiple line in H/W module
#define AudmhlGetMLinSPDType()       ReadREG(RW_MULTILINE_SPDIF_TYPE)

//Use SPDIF/Line In H/W module
#define AudmhlGetSPDIFInType()       ReadREG(RW_SPDIFIN_TYPE)
#define AudmhlSetMLinCtrl(u4Data)    WriteREG(RW_DSP_MULTI_LIN_CTL,(u4Data))
#define AudmhlSetSPDIFInCtrl(u4Data) WriteREG(RW_SPDIFIN_CTRL,(u4Data))
#define AudmhlSetMLinBLK(dwBlk)      WriteREG(RW_DSP_MULTI_LIN_BLK,(dwBlk))
#define AudmhlSetSPDIFInWPtr()       WriteREG(RW_ABUF0_PNT,0x01)

#define AudmhlSetMLinWPtr()          WriteREG(RW_ABUF0_PNT,0x02)

//#define u4GetMultiLineInCtrl()       ReadREG(RW_DSP_MULTI_LIN_CTL)
//#define u4GetSPDIFInCtrl()           ReadREG(RW_SPDIFIN_CTRL)
//#define u4GetAinAckCfg(u4Addr)       ReadREG(u4Addr)
//#define vSetAinAckCfg(u4Addr,u4Data) WriteREG(u4Addr,u4Data)
//#define vSetSPDIFInBLK(dwBlk)        WriteREG(RW_SPDIFIN_BLK,(dwBlk))

#define vAinRegSet(addr,mask,value)  WriteREG((addr),((ReadREG(addr)& (~(mask))) | ((value)&(mask))))
#define vAinRegAnd(addr,value,mask)  WriteREG(addr,((ReadREG(addr)& (~mask)) | value))

#endif

#endif
