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

/******************************************************************************
*[File]                aud_misc_reg.h
*[Author]              
*[Description]
*
*[Copyright]
*    
******************************************************************************/

#ifndef _AUD_MISC_REG_H_
#define _AUD_MISC_REG_H_

/////////////////////////////////////////////////////////////////////////////
//         DSP Register
/////////////////////////////////////////////////////////////////////////////
// Should be define in DSP.


/////////////////////////////////////////////////////////////////////////////
//         RISC to DSP Register
/////////////////////////////////////////////////////////////////////////////
#define AUDREG_R2D_BASE                     (0x100)

#define AUDREG_R2D_ADR                      (AUDREG_R2D_BASE + 0x80)
#define AUDREG_R2D_SD                       (AUDREG_R2D_BASE + 0x81)
#define AUDREG_R2D_LDH                      (AUDREG_R2D_BASE + 0x82)
#define AUDREG_R2D_LDL                      (AUDREG_R2D_BASE + 0x83)
#define AUDREG_R2D_STA                      (AUDREG_R2D_BASE + 0x84)
#define AUDREG_R2D_RD                       (AUDREG_R2D_BASE + 0x85)

#define AUDREG_D2R_ADR                      (AUDREG_R2D_BASE + 0x86)
#define AUDREG_D2R_ADR_INTR                 (AUDREG_R2D_BASE + 0x87)
#define AUDREG_D2R_DATA                     (AUDREG_R2D_BASE + 0x88)
#define AUDREG_D2R_RD                       (AUDREG_R2D_BASE + 0x89)
#define AUDREG_D2R_STA                      (AUDREG_R2D_BASE + 0x8A)

#define AUDREG_R2D_WRITE_DRAM               (AUDREG_R2D_BASE + 0x90)
#define AUDREG_R2D_RW_DRAM_SBLK             (AUDREG_R2D_BASE + 0x91)
#define AUDREG_R2D_DRAM_CMPT_SBLK           (AUDREG_R2D_BASE + 0x92)
#define AUDREG_R2D_CMPT_STA_SAVE_RSTR       (AUDREG_R2D_BASE + 0x93)
#define AUDREG_R2D_DRAM_FLUSH               (AUDREG_R2D_BASE + 0x94)
#define AUDREG_R2D_DRAM_STA_SAVE_RSTR       (AUDREG_R2D_BASE + 0x95)


/////////////////////////////////////////////////////////////////////////////
//         PWM Register
/////////////////////////////////////////////////////////////////////////////
#define AUDREG_PWM_BASE                       (0x0)

#define AUDREG_PWM_RAMPINFO0                  (AUDREG_PWM_BASE + 0xE00)
#define AUDREG_PWM_RAMPINFO1                  (AUDREG_PWM_BASE + 0xE01)
#define AUDREG_PWM_RAMPINFO2                  (AUDREG_PWM_BASE + 0xE02)
#define AUDREG_PWM_RAMPINFO3                  (AUDREG_PWM_BASE + 0xE03)
#define AUDREG_PWM_RAMPINFO4                  (AUDREG_PWM_BASE + 0xE04)
#define AUDREG_PWM_RAMPINFO5                  (AUDREG_PWM_BASE + 0xE05)
#define AUDREG_PWM_RAMPINFO6                  (AUDREG_PWM_BASE + 0xE06)

#define AUDREG_PWM_PDATA                      (AUDREG_PWM_BASE + 0xF00)
#define AUDREG_PWM_CTRL0                      (AUDREG_PWM_BASE + 0xF01)
#define AUDREG_PWM_CTRL1                      (AUDREG_PWM_BASE + 0xF02)
#define AUDREG_PWM_OUTPUT_EN                  (AUDREG_PWM_BASE + 0xF04)
#define AUDREG_PWM_INTR_CTRL                  (AUDREG_PWM_BASE + 0xF05)
#define AUDREG_PWM_STATUS                     (AUDREG_PWM_BASE + 0xF06)
#define AUDREG_PWM_GPIO                       (AUDREG_PWM_BASE + 0xF07)
#define AUDREG_PWM_COEF_ADDR                  (AUDREG_PWM_BASE + 0xF08)
#define AUDREG_PWM_COEF_DATA                  (AUDREG_PWM_BASE + 0xF09)
#define AUDREG_PWM_PIN0_MOD                   (AUDREG_PWM_BASE + 0xF0A)
#define AUDREG_PWM_PIN1_MOD                   (AUDREG_PWM_BASE + 0xF0B)
#define AUDREG_PWM_PIN2_MOD                   (AUDREG_PWM_BASE + 0xF0C)
#define AUDREG_PWM_PIN3_MOD                   (AUDREG_PWM_BASE + 0xF0D)
#define AUDREG_PWM_CH0_CTRL                   (AUDREG_PWM_BASE + 0xF0E)
#define AUDREG_PWM_CH1_CTRL                   (AUDREG_PWM_BASE + 0xF0F)
#define AUDREG_PWM_CH2_CTRL                   (AUDREG_PWM_BASE + 0xF10)
#define AUDREG_PWM_CH3_CTRL                   (AUDREG_PWM_BASE + 0xF11)
#define AUDREG_PWM_CH4_CTRL                   (AUDREG_PWM_BASE + 0xF12)
#define AUDREG_PWM_CH5_CTRL                   (AUDREG_PWM_BASE + 0xF13)
#define AUDREG_PWM_PIN4_MOD                   (AUDREG_PWM_BASE + 0xF14)
#define AUDREG_PWM_PIN5_MOD                   (AUDREG_PWM_BASE + 0xF15)
#define AUDREG_PWM_PIN6_MOD                   (AUDREG_PWM_BASE + 0xF16)
#define AUDREG_PWM_PIN7_MOD                   (AUDREG_PWM_BASE + 0xF17)
#define AUDREG_PWM_PIN8_MOD                   (AUDREG_PWM_BASE + 0xF18)
#define AUDREG_PWM_PIN9_MOD                   (AUDREG_PWM_BASE + 0xF19)
#define AUDREG_PWM_PIN10_MOD                  (AUDREG_PWM_BASE + 0xF1A)
#define AUDREG_PWM_PIN11_MOD                  (AUDREG_PWM_BASE + 0xF1B)
#define AUDREG_PWM_ERR_TST                    (AUDREG_PWM_BASE + 0xF1C)
#define AUDREG_PWM_OUTPUT_STATUS              (AUDREG_PWM_BASE + 0xF1D)
#define AUDREG_PWM_RAMP_FLAG                  (AUDREG_PWM_BASE + 0xF1E)


/////////////////////////////////////////////////////////////////////////////
//         RGBK2 Register
/////////////////////////////////////////////////////////////////////////////
#define AUDREG_RGBK2_BASE                     (0xA8000)

#define AUDREG_RGBK2_PWMTOP_CFG               (AUDREG_RGBK2_BASE + 0x00)
#define AUDREG_RGBK2_FRNT_PWM_CFG1            (AUDREG_RGBK2_BASE + 0x04)
    #define AUD_FRNT_PWM_IN_SEL_BIT_START      (6)
    #define AUD_FRNT_PWM_IN_SEL_BIT_NUM        (2)
#define AUDREG_RGBK2_FRNT_PWM_CFG2            (AUDREG_RGBK2_BASE + 0x08)
#define AUDREG_RGBK2_FRNT_PWM_CFG3            (AUDREG_RGBK2_BASE + 0x0C)
#define AUDREG_RGBK2_FRNT_PWM_CFG4            (AUDREG_RGBK2_BASE + 0x10)
#define AUDREG_RGBK2_FRNT_PWM_CFG5            (AUDREG_RGBK2_BASE + 0x14)
#define AUDREG_RGBK2_FRNT_PWM_CFG6            (AUDREG_RGBK2_BASE + 0x18)
#define AUDREG_RGBK2_FRNT_PWM_CFG7            (AUDREG_RGBK2_BASE + 0x1C)
#define AUDREG_RGBK2_FRNT_PWM_CFG8            (AUDREG_RGBK2_BASE + 0x20)
#define AUDREG_RGBK2_FRNT_PWM_CFG9            (AUDREG_RGBK2_BASE + 0x24)
#define AUDREG_RGBK2_FRNT_PWM_CFG10           (AUDREG_RGBK2_BASE + 0x28)
#define AUDREG_RGBK2_REAR_PWM_CFG1            (AUDREG_RGBK2_BASE + 0x2C)
    #define AUD_REAR_PWM_IN_SEL_BIT_START      (6)
    #define AUD_REAR_PWM_IN_SEL_BIT_NUM        (2)
#define AUDREG_RGBK2_REAR_PWM_CFG2            (AUDREG_RGBK2_BASE + 0x30)
#define AUDREG_RGBK2_REAR_PWM_CFG3            (AUDREG_RGBK2_BASE + 0x34)
#define AUDREG_RGBK2_REAR_PWM_CFG4            (AUDREG_RGBK2_BASE + 0x38)
#define AUDREG_RGBK2_REAR_PWM_CFG5            (AUDREG_RGBK2_BASE + 0x3C)
#define AUDREG_RGBK2_REAR_PWM_CFG6            (AUDREG_RGBK2_BASE + 0x40)

#define AUDREG_RGBK2_ARM_MIC_SBLK             (AUDREG_RGBK2_BASE + 0x40)
    #define AUD_MIC_SBLK_BIT_START             (0)
    #define AUD_MIC_SBLK_BIT_NUM               (16)

#define AUDREG_RGBK2_REAR_PWM_CFG7            (AUDREG_RGBK2_BASE + 0x44)
#define AUDREG_RGBK2_REAR_PWM_CFG8            (AUDREG_RGBK2_BASE + 0x48)
#define AUDREG_RGBK2_REAR_PWM_CFG9            (AUDREG_RGBK2_BASE + 0x4C)
#define AUDREG_RGBK2_REAR_PWM_CFG10           (AUDREG_RGBK2_BASE + 0x50)
#define AUDREG_RGBK2_GPS_PWM_CFG1             (AUDREG_RGBK2_BASE + 0x54)
#define AUDREG_RGBK2_GPS_PWM_CFG2             (AUDREG_RGBK2_BASE + 0x58)
#define AUDREG_RGBK2_GPS_PWM_CFG3             (AUDREG_RGBK2_BASE + 0x5C)
#define AUDREG_RGBK2_GPS_PWM_CFG4             (AUDREG_RGBK2_BASE + 0x60)
#define AUDREG_RGBK2_GPS_PWM_CFG5             (AUDREG_RGBK2_BASE + 0x64)
#define AUDREG_RGBK2_GPS_PWM_CFG6             (AUDREG_RGBK2_BASE + 0x68)
#define AUDREG_RGBK2_GPS_PWM_CFG7             (AUDREG_RGBK2_BASE + 0x6C)
#define AUDREG_RGBK2_GPS_PWM_CFG8             (AUDREG_RGBK2_BASE + 0x70)

#define AUD_GPS_AOUT_INTR_CLR_BIT_START        1
#define AUD_GPS_AOUT_INTR_CLR_BIT_NUM          1
#define AUD_GPS_OUT2_INTR_CLR_BIT_START        2
#define AUD_GPS_OUT2_INTR_CLR_BIT_NUM          1

#define AUDREG_RGBK2_GPS_PWM_CFG9             (AUDREG_RGBK2_BASE + 0x74)
#define AUDREG_RGBK2_GPS_PWM_CFG10            (AUDREG_RGBK2_BASE + 0x78)
#define AUDREG_RGBK2_CFG0                     (AUDREG_RGBK2_BASE + 0x80)
#define AUDREG_RGBK2_CFG1                     (AUDREG_RGBK2_BASE + 0x84)
#define AUDREG_RGBK2_CFG2                     (AUDREG_RGBK2_BASE + 0x88)
#define AUDREG_RGBK2_CFG3                     (AUDREG_RGBK2_BASE + 0x8C)
#define AUDREG_RGBK2_CFG4                     (AUDREG_RGBK2_BASE + 0x90)
    #define AUD_LINEIN_CLK_FROM_MLINE_START         (4)
    #define AUD_LINEIN_CLK_FROM_MLINE_NUM           (1)
#define AUDREG_RGBK2_AOUT_CFG0                (AUDREG_RGBK2_BASE + 0xC0)
    #define AUD_GPS_AOUT_CYC_BIT_START         (0)
    #define AUD_GPS_AOUT_CYC_BIT_NUM           (2)
    #define AUD_GPS_AOUT_BCK_BIT_START         (16)
    #define AUD_GPS_AOUT_BCK_BIT_NUM           (4)
    #define AUD_GPS_AOUT_IWL_BIT_START         (23)
    #define AUD_GPS_AOUT_IWL_BIT_NUM           (6)
#define AUDREG_RGBK2_AOUT_CFG1                (AUDREG_RGBK2_BASE + 0xC4)
#define AUDREG_RGBK2_AOUT_CFG2                (AUDREG_RGBK2_BASE + 0xC8)
    #define AUD_FRNT_SRC_SEL_BIT_START         (16)
    #define AUD_FRNT_SRC_SEL_BIT_NUM           (2)
    #define AUD_REAR_SRC_SEL_BIT_START         (18)
    #define AUD_REAR_SRC_SEL_BIT_NUM           (2)
#define AUDREG_RGBK2_AOUT_CFG3                (AUDREG_RGBK2_BASE + 0xCC)
#define AUDREG_RGBK2_AFE_TOP_CFG0             (AUDREG_RGBK2_BASE + 0xD0)
    #define AUD_LINEIN_CLK_FROM_AOUT2_START        (14)
    #define AUD_LINEIN_CLK_FROM_AOUT2_NUM          (1)
    
#define AUDREG_RGBK2_BYPS_VLUM_CFG0           (AUDREG_RGBK2_BASE + 0xD4)
#define AUDREG_RGBK2_BYPS_VLUM_CFG1           (AUDREG_RGBK2_BASE + 0xD8)

#define AUD_MPHONE_SEL_ADC_START              (6)
#define AUD_MPHONE_SEL_ADC_NUM                   (1)

#define AUDREG_RGBK2_BT_PCM_BLK_CFG           (AUDREG_RGBK2_BASE + 0xE0)
#define AUDREG_RGBK2_AFE_DBG_ASDR             (AUDREG_RGBK2_BASE + 0xE4)
#define AUDREG_RGBK2_AFE_DBG_EADR             (AUDREG_RGBK2_BASE + 0xE8)
#define AUDREG_RGBK2_PWMANA_CFG0              (AUDREG_RGBK2_BASE + 0x1C0)  // PWMDAC Register
#define AUDREG_RGBK2_PWMANA_CFG1              (AUDREG_RGBK2_BASE + 0x1C4)
#define AUDREG_RGBK2_PWMANA_CFG2              (AUDREG_RGBK2_BASE + 0x1C8)
#define AUDREG_RGBK2_PWMANA_CFG3              (AUDREG_RGBK2_BASE + 0x1CC)
#define AUDREG_RGBK2_PWMANA_CFG4              (AUDREG_RGBK2_BASE + 0x1D0)
#define AUDREG_RGBK2_PWMANA_CFG5              (AUDREG_RGBK2_BASE + 0x1D4)
#define AUDREG_RGBK2_PWMANA_CFG6              (AUDREG_RGBK2_BASE + 0x1D8)
#define AUDREG_RGBK2_PWMANA_CFG7              (AUDREG_RGBK2_BASE + 0x1DC)
#define AUDREG_RGBK2_APLL_ADJ_CFG0            (AUDREG_RGBK2_BASE + 0x1E0)
#define AUDREG_RGBK2_APLL_ADJ_CFG1            (AUDREG_RGBK2_BASE + 0x1E4)
#define AUDREG_RGBK2_APLL_ADJ_STATUS0         (AUDREG_RGBK2_BASE + 0x1E8)
#define AUDREG_RGBK2_APLL_ADJ_STATUS1         (AUDREG_RGBK2_BASE + 0x1EC)
#define AUDREG_RGBK2_APLL_ADJ_STATUS2         (AUDREG_RGBK2_BASE + 0x1F0)


#define AUDREG_RGBK2_INDRECT_FRNT_ADDR        (AUDREG_RGBK2_BASE + 0x240)
#define AUDREG_RGBK2_INDRECT_FRNT_DATA        (AUDREG_RGBK2_BASE + 0x244)
#define AUDREG_RGBK2_INDRECT_REAR_ADDR        (AUDREG_RGBK2_BASE + 0x248)
#define AUDREG_RGBK2_INDRECT_REAR_DATA        (AUDREG_RGBK2_BASE + 0x24C)
#define AUDREG_RGBK2_INDRECT_GPS_ADDR         (AUDREG_RGBK2_BASE + 0x250)
#define AUDREG_RGBK2_INDRECT_GPS_DATA         (AUDREG_RGBK2_BASE + 0x254)


/////////////////////////////////////////////////////////////////////////////
//         GPS AOUT Register
/////////////////////////////////////////////////////////////////////////////
#define AUDREG_GPS_AOUT_BASE           (0xA8400)

#define AUDREG_GOUT_CH1_BUF_SADR         (AUDREG_GPS_AOUT_BASE + 0x00)
#define AUDREG_GOUT_CH1_BUF_SIZE         (AUDREG_GPS_AOUT_BASE + 0x04)
#define AUDREG_GOUT_CH1_NSADR            (AUDREG_GPS_AOUT_BASE + 0x08)
#define AUDREG_GPS_AOUT_SN               (AUDREG_GPS_AOUT_BASE + 0x38)
#define AUDREG_GPS_AOUT_INTR_NUM         (AUDREG_GPS_AOUT_BASE + 0x3c)
#define AUDREG_GPS_AOUT_CTRL             (AUDREG_GPS_AOUT_BASE + 0x40)
#define AUDREG_GPS_AOUT_CH_CFG           (AUDREG_GPS_AOUT_BASE + 0x44)
#define AUDREG_GPS_AOUT_CH_NUM           (AUDREG_GPS_AOUT_BASE + 0x50)


/////////////////////////////////////////////////////////////////////////////
//         AOUT2 Register
/////////////////////////////////////////////////////////////////////////////
#define AUDREG_AOUT2_BASE                     (0xA8500)

#define AUDREG_AOUT2_CH1_SADR                 (AUDREG_AOUT2_BASE + 0x00)
#define AUDREG_AOUT2_CH1_SIZE                 (AUDREG_AOUT2_BASE + 0x04)
#define AUDREG_AOUT2_CH1_NSADR                (AUDREG_AOUT2_BASE + 0x08)
#define AUDREG_AOUT2_CH2_NSADR                (AUDREG_AOUT2_BASE + 0x0C)
#define AUDREG_AOUT2_CH3_NSADR                (AUDREG_AOUT2_BASE + 0x10)
#define AUDREG_AOUT2_CH4_NSADR                (AUDREG_AOUT2_BASE + 0x14)
#define AUDREG_AOUT2_CH5_NSADR                (AUDREG_AOUT2_BASE + 0x18)
#define AUDREG_AOUT2_CH6_NSADR                (AUDREG_AOUT2_BASE + 0x1C)
#define AUDREG_AOUT2_CH7_NSADR                (AUDREG_AOUT2_BASE + 0x20)
#define AUDREG_AOUT2_CH8_NSADR                (AUDREG_AOUT2_BASE + 0x24)
#define AUDREG_AOUT2_CH9_NSADR                (AUDREG_AOUT2_BASE + 0x28)
#define AUDREG_AOUT2_CH10_NSADR               (AUDREG_AOUT2_BASE + 0x2C)
#define AUDREG_AOUT2_CH11_NSADR               (AUDREG_AOUT2_BASE + 0x30)
#define AUDREG_AOUT2_CH12_NSADR               (AUDREG_AOUT2_BASE + 0x34)
#define AUDREG_AOUT2_NSNUM                    (AUDREG_AOUT2_BASE + 0x38)
#define AUDREG_AOUT2_INTR_SIZE                (AUDREG_AOUT2_BASE + 0x3C)
#define AUDREG_AOUT2_CTRL                     (AUDREG_AOUT2_BASE + 0x40)
#define AUDREG_AOUT2_CFG0                     (AUDREG_AOUT2_BASE + 0x44)
#define AUDREG_AOUT2_CFG1                     (AUDREG_AOUT2_BASE + 0x48)
#define AUDREG_AOUT2_CFG2                     (AUDREG_AOUT2_BASE + 0x4C)
#define AUDREG_AOUT2_CH_NUM                   (AUDREG_AOUT2_BASE + 0x50)
#define AUDREG_AOUT2_SWITCH                   (AUDREG_AOUT2_BASE + 0x54)
#define AUDREG_AOUT2_LEV_DET_CFG              (AUDREG_AOUT2_BASE + 0x58)
#define AUDREG_AOUT2_LEVEL                    (AUDREG_AOUT2_BASE + 0x5C)
#define AUDREG_AOUT2_BUF_MIX_CFG              (AUDREG_AOUT2_BASE + 0x60)
#define AUDREG_AOUT2_MIX1_NSADR               (AUDREG_AOUT2_BASE + 0x64)
#define AUDREG_AOUT2_MIX2_NSADR               (AUDREG_AOUT2_BASE + 0x68)
#define AUDREG_AOUT2_LRCK_CNT                 (AUDREG_AOUT2_BASE + 0x6C)
//#define AUDREG_AOUT2_SACD_L_VOLUME            (AUDREG_AOUT2_BASE + 0x70)
//#define AUDREG_AOUT2_SACD_R_VOLUME            (AUDREG_AOUT2_BASE + 0x74)
//#define AUDREG_AOUT2_SACD_SL_VOLUME           (AUDREG_AOUT2_BASE + 0x78)
//#define AUDREG_AOUT2_SACD_SR_VOLUME           (AUDREG_AOUT2_BASE + 0x7C)
//#define AUDREG_AOUT2_SACD_C_VOLUME            (AUDREG_AOUT2_BASE + 0x80)
//#define AUDREG_AOUT2_SACD_LFE_VOLUME          (AUDREG_AOUT2_BASE + 0x84)
//#define AUDREG_AOUT2_SACD_FADER_CTRL          (AUDREG_AOUT2_BASE + 0x88)
//#define AUDREG_AOUT2_SACD_FADER_MONITOR       (AUDREG_AOUT2_BASE + 0x8C)
//#define AUDREG_AOUT2_SACD_FADER_FLAG          (AUDREG_AOUT2_BASE + 0x90)
//#define AUDREG_AOUT2_SACD_ANCILLARY           (AUDREG_AOUT2_BASE + 0x94)
//#define AUDREG_AOUT2_SACD_FRAME               (AUDREG_AOUT2_BASE + 0x98)
//#define AUDREG_AOUT2_SACD_FCK_CFG             (AUDREG_AOUT2_BASE + 0x9C)
#define AUDREG_AOUT2_IIR_SEL                  (AUDREG_AOUT2_BASE + 0xA0)
#define AUDREG_AOUT2_IIR_COEF                 (AUDREG_AOUT2_BASE + 0xA4)


/////////////////////////////////////////////////////////////////////////////
//                   RISC Interface Register
/////////////////////////////////////////////////////////////////////////////
/* This register should be define already at other place.  */


/////////////////////////////////////////////////////////////////////////////
//                   RISC Interface MIN Register
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//                   Memory Map Register
/////////////////////////////////////////////////////////////////////////////
#define AUDREG_MMR                            (0x00)

//bit0~19: write compact data address
#define AUDREG_SET_WDCMPT_ADDR                (AUDREG_MMR + 0x51)
//bit0~19: read compact data address
#define AUDREG_SET_RDCMPT_ADDR                (AUDREG_MMR + 0x52)
//bit0~1: compact write data bit width selection
//bit2~3: compact read data bit width selection
//0 is 24bits, 1 is 16bits, 2 is 8bits, 3 is 4bits.
#define AUDREG_CMPT_BWSEL                     (AUDREG_MMR + 0x53)
//0~2 ADSP read/write DRAM cache page selection
#define AUDREG_SET_DRAM_PAGE                  (AUDREG_MMR + 0x54)
//0~2 ADSP compact read/write DRAM cache page selection
#define AUDREG_SET_CMPT_PAGE                  (AUDREG_MMR + 0x55)
//0~2 ADSP read DMA cache page selection
#define AUDREG_SET_WDMA_PAGE                  (AUDREG_MMR + 0x56)
//0~2 ADSP write DMA cache page selection
#define AUDREG_SET_RDMA_PAGE                  (AUDREG_MMR + 0x57)
//0~19 ADSP read DMA DRAM address
#define AUDREG_SET_RDMA_ADDR                  (AUDREG_MMR + 0x58)
//0~19 ADSP write DMA DRAM address
#define AUDREG_SET_WDMA_ADDR                  (AUDREG_MMR + 0x59)

/////////////////////////////////////////////////////////////////////////////
//                   Audio Interface Register
/////////////////////////////////////////////////////////////////////////////
#define AUD_INTF_BASE_ADDR                     (0x5000)

#define AUD_AOUT_CFG_ADDR                      (AUD_INTF_BASE_ADDR + 0xc0)
    #define AUD_AOUT_LRCK_CYC_BIT_START        (0)
    #define AUD_AOUT_LRCK_CYC_BIT_NUM          (2)
    #define AUD_AOUT_DAT_DLY_BIT_START         (4)
    #define AUD_AOUT_DAT_DLY_BIT_NUM           (1)
    #define AUD_AOUT_LEFT_ALN_BIT_START        (5)
    #define AUD_AOUT_LEFT_ALN_BIT_NUM          (1)
    #define AUD_INV_BCK_BIT_START              (6)
    #define AUD_INV_BCK_BIT_NUM                (1)
    #define AUD_AOUT_DA_BNUM_BIT_START         (8)
    #define AUD_AOUT_DA_BNUM_BIT_NUM           (6)
    #define AUD_AOUT_A2BCKX_BIT_START          (16)
    #define AUD_AOUT_A2BCKX_BIT_NUM            (4)

#define AUD_MISC_CTRL_ADDR                     (AUD_INTF_BASE_ADDR + 0xcc)
    #define AUD_K_DIV_BIT_START                (0)
    #define AUD_K_DIV_BIT_NUM                  (4)
    #define AUD_CK_DIV_SEL_BIT_START           (4)
    #define AUD_CK_DIV_SEL_BIT_NUM             (2)
    #define AUD_AOUT2_A2BCKX_BIT_START         (12)
    #define AUD_AOUT2_A2BCKX_BIT_NUM           (4)
    #define AUD_AOUT2_LRCK_CYC_BIT_START       (17)
    #define AUD_AOUT2_LRCK_CYC_BIT_NUM         (2)
    #define AUD_AOUT2_DELAY_BIT_START          (19)
    #define AUD_AOUT2_DELAY_BIT_NUM            (1)
    #define AUD_AOUT2_LEFT_BIT_START           (20)
    #define AUD_AOUT2_LEFT_BIT_NUM             (1)
    #define AUD_AOUT2_BCK_INV_START            (21)
    #define AUD_AOUT2_BCK_INV_BIT_NUMBER       (1)
    #define AUD_AOUT2_DA_BNUM_BIT_START        (23)
    #define AUD_AOUT2_DA_BNUM_BIT_NUM          (6)


/////////////////////////////////////////////////////////////////////////////
//                   AFE1 Register

/////////////////////////////////////////////////////////////////////////////
#define AUDREG_AFE1_BASE                      (0xA8200)
    
#define AUDREG_AFE1_CFG0                      (AUDREG_AFE1_BASE + 0x84)
#define AUDREG_AFE1_CFG1                      (AUDREG_AFE1_BASE + 0x88)
#define AUDREG_AFE1_CFG2                      (AUDREG_AFE1_BASE + 0x8C)
#define AUDREG_AFE1_CFG3                      (AUDREG_AFE1_BASE + 0x90)
#define AUDREG_AFE1_CFG4                      (AUDREG_AFE1_BASE + 0x94)
#define AUDREG_AFE1_CFG5                      (AUDREG_AFE1_BASE + 0x98)
#define AUDREG_AFE1_CFG6                      (AUDREG_AFE1_BASE + 0x9C)
#define AUDREG_AFE1_CFG7                      (AUDREG_AFE1_BASE + 0xA0)
#define AUDREG_AFE1_CFG8                      (AUDREG_AFE1_BASE + 0xA4)
#define AUDREG_AFE1_CFG9                      (AUDREG_AFE1_BASE + 0xA8)
#define AUDREG_AFE1_CFG10                     (AUDREG_AFE1_BASE + 0xAC)
#define AUDREG_AFE1_CFG11                     (AUDREG_AFE1_BASE + 0xB0)
#define AUDREG_AFE1_CFG12                     (AUDREG_AFE1_BASE + 0xB4)
#define AUDREG_AFE1_CFG13                     (AUDREG_AFE1_BASE + 0xB8)
#define AUDREG_AFE1_CFG14                     (AUDREG_AFE1_BASE + 0xBC)
#define AUDREG_AFE1_CFG15                     (AUDREG_AFE1_BASE + 0xC0)
#define AUDREG_AFE1_CFG16                     (AUDREG_AFE1_BASE + 0xC4)
#define AUDREG_AFE1_CFG17                     (AUDREG_AFE1_BASE + 0xC8)
#define AUDREG_AFE1_CFG18                     (AUDREG_AFE1_BASE + 0xCC)
#define AUDREG_AFE1_CFG19                     (AUDREG_AFE1_BASE + 0xD0)
#define AUDREG_AFE1_CFG20                     (AUDREG_AFE1_BASE + 0xD4)
#define AUDREG_AFE1_CFG21                     (AUDREG_AFE1_BASE + 0xD8)
#define AUDREG_AFE1_CFG22                     (AUDREG_AFE1_BASE + 0xDC)
    
    /////////////////////////////////////////////////////////////////////////////
    //                   AFE2 Register
    /////////////////////////////////////////////////////////////////////////////
#define AUDREG_AFE2_BASE                      (0xA8300)
    
#define AUDREG_AFE2_CFG0                      (AUDREG_AFE2_BASE + 0x04)
#define AUDREG_AFE2_CFG1                      (AUDREG_AFE2_BASE + 0x08)
#define AUDREG_AFE2_CFG2                      (AUDREG_AFE2_BASE + 0x0C)
#define AUDREG_AFE2_CFG3                      (AUDREG_AFE2_BASE + 0x10)
#define AUDREG_AFE2_CFG4                      (AUDREG_AFE2_BASE + 0x14)
#define AUDREG_AFE2_CFG5                      (AUDREG_AFE2_BASE + 0x18)
#define AUDREG_AFE2_CFG6                      (AUDREG_AFE2_BASE + 0x1C)
#define AUDREG_AFE2_CFG7                      (AUDREG_AFE2_BASE + 0x20)
#define AUDREG_AFE2_CFG8                      (AUDREG_AFE2_BASE + 0x24)
#define AUDREG_AFE2_CFG9                      (AUDREG_AFE2_BASE + 0x28)
#define AUDREG_AFE2_CFG10                     (AUDREG_AFE2_BASE + 0x2C)
#define AUDREG_AFE2_CFG11                     (AUDREG_AFE2_BASE + 0x30)
#define AUDREG_AFE2_CFG12                     (AUDREG_AFE2_BASE + 0x34)
#define AUDREG_AFE2_CFG13                     (AUDREG_AFE2_BASE + 0x38)
#define AUDREG_AFE2_CFG14                     (AUDREG_AFE2_BASE + 0x3C)
#define AUDREG_AFE2_CFG15                     (AUDREG_AFE2_BASE + 0x40)
#define AUDREG_AFE2_CFG16                     (AUDREG_AFE2_BASE + 0x44)
#define AUDREG_AFE2_CFG17                     (AUDREG_AFE2_BASE + 0x48)
#define AUDREG_AFE2_CFG18                     (AUDREG_AFE2_BASE + 0x4C)
#define AUDREG_AFE2_CFG19                     (AUDREG_AFE2_BASE + 0x50)
#define AUDREG_AFE2_CFG20                     (AUDREG_AFE2_BASE + 0x54)
#define AUDREG_AFE2_CFG21                     (AUDREG_AFE2_BASE + 0x58)
#define AUDREG_AFE2_CFG22                     (AUDREG_AFE2_BASE + 0x5C)
    
    /////////////////////////////////////////////////////////////////////////////
    //                   AUADC Register
    /////////////////////////////////////////////////////////////////////////////
#define AUDREG_AUADC_BASE                     (0x300)
    
#define AUDREG_AADC_CFG0                      (AUDREG_AUADC_BASE + 0x00)
#define AUDREG_AADC_CFG1                      (AUDREG_AUADC_BASE + 0x04)
#define AUDREG_AADC_CFG2                      (AUDREG_AUADC_BASE + 0x08)
#define AUDREG_AADC_CFG3                      (AUDREG_AUADC_BASE + 0x0C)
#define AUDREG_AADC_CFG4                      (AUDREG_AUADC_BASE + 0x10)
#define AUDREG_AADC_CFG5                      (AUDREG_AUADC_BASE + 0x14)
#define AUDREG_AADC_CFG6                      (AUDREG_AUADC_BASE + 0x18)
#define AUDREG_AADC_CFG7                      (AUDREG_AUADC_BASE + 0x1C)
#define AUDREG_AADC_CFG8                      (AUDREG_AUADC_BASE + 0x20)
#define AUDREG_AADC_CFG9                      (AUDREG_AUADC_BASE + 0x24)
#define AUDREG_AADC_CFG10                     (AUDREG_AUADC_BASE + 0x28)
#define AUDREG_AADC_CFG11                     (AUDREG_AUADC_BASE + 0x2C)
#define AUDREG_AADC_CFG12                     (AUDREG_AUADC_BASE + 0x30)
#define AUDREG_AADC_CFG13                     (AUDREG_AUADC_BASE + 0x34)
#define AUDREG_AADC_CFG14                     (AUDREG_AUADC_BASE + 0x38)
#define AUDREG_AADC_CFG15                     (AUDREG_AUADC_BASE + 0x3C)
#define AUDREG_AADC_CFG16                     (AUDREG_AUADC_BASE + 0x40)
#define AUDREG_AADC_RO                        (AUDREG_AUADC_BASE + 0x44)
    
    


#endif // #ifndef _AUD_MISC_REG_H_

