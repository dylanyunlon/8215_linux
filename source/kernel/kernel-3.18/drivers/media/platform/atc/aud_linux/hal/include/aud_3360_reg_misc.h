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




#ifndef _AUDIO_3360_REG_MISC_H_
#define _AUDIO_3360_REG_MISC_H_

/////////////////////////////////////////////////////////////////////////////
//         DSP Register
/////////////////////////////////////////////////////////////////////////////
// Should be define in DSP.


/////////////////////////////////////////////////////////////////////////////
//         RISC to DSP Register
/////////////////////////////////////////////////////////////////////////////
#define AUD_REG_R2D_BASE                     (0x100)

#define AUD_REG_R2D_ADR                      (AUD_REG_R2D_BASE + 0x80)
#define AUD_REG_R2D_SD                       (AUD_REG_R2D_BASE + 0x81)
#define AUD_REG_R2D_LDH                      (AUD_REG_R2D_BASE + 0x82)
#define AUD_REG_R2D_LDL                      (AUD_REG_R2D_BASE + 0x83)
#define AUD_REG_R2D_STA                      (AUD_REG_R2D_BASE + 0x84)
#define AUD_REG_R2D_RD                       (AUD_REG_R2D_BASE + 0x85)

#define AUD_REG_D2R_ADR                      (AUD_REG_R2D_BASE + 0x86)
#define AUD_REG_D2R_ADR_INTR                 (AUD_REG_R2D_BASE + 0x87)
#define AUD_REG_D2R_DATA                     (AUD_REG_R2D_BASE + 0x88)
#define AUD_REG_D2R_RD                       (AUD_REG_R2D_BASE + 0x89)
#define AUD_REG_D2R_STA                      (AUD_REG_R2D_BASE + 0x8A)

#define AUD_REG_R2D_WRITE_DRAM               (AUD_REG_R2D_BASE + 0x90)
#define AUD_REG_R2D_RW_DRAM_SBLK             (AUD_REG_R2D_BASE + 0x91)
#define AUD_REG_R2D_DRAM_CMPT_SBLK           (AUD_REG_R2D_BASE + 0x92)
#define AUD_REG_R2D_CMPT_STA_SAVE_RSTR       (AUD_REG_R2D_BASE + 0x93)
#define AUD_REG_R2D_DRAM_FLUSH               (AUD_REG_R2D_BASE + 0x94)
#define AUD_REG_R2D_DRAM_STA_SAVE_RSTR       (AUD_REG_R2D_BASE + 0x95)


/////////////////////////////////////////////////////////////////////////////
//         PWM Register
/////////////////////////////////////////////////////////////////////////////
#define AUD_REG_PWM_BASE                       (0x0)

#define AUD_REG_PWM_RAMPINFO0                  (AUD_REG_PWM_BASE + 0xE00)
#define AUD_REG_PWM_RAMPINFO1                  (AUD_REG_PWM_BASE + 0xE01)
#define AUD_REG_PWM_RAMPINFO2                  (AUD_REG_PWM_BASE + 0xE02)
#define AUD_REG_PWM_RAMPINFO3                  (AUD_REG_PWM_BASE + 0xE03)
#define AUD_REG_PWM_RAMPINFO4                  (AUD_REG_PWM_BASE + 0xE04)
#define AUD_REG_PWM_RAMPINFO5                  (AUD_REG_PWM_BASE + 0xE05)
#define AUD_REG_PWM_RAMPINFO6                  (AUD_REG_PWM_BASE + 0xE06)

#define AUD_REG_PWM_PDATA                      (AUD_REG_PWM_BASE + 0xF00)
#define AUD_REG_PWM_CTRL0                      (AUD_REG_PWM_BASE + 0xF01)
#define AUD_REG_PWM_CTRL1                      (AUD_REG_PWM_BASE + 0xF02)
#define AUD_REG_PWM_OUTPUT_EN                  (AUD_REG_PWM_BASE + 0xF04)
#define AUD_REG_PWM_INTR_CTRL                  (AUD_REG_PWM_BASE + 0xF05)
#define AUD_REG_PWM_STATUS                     (AUD_REG_PWM_BASE + 0xF06)
#define AUD_REG_PWM_GPIO                       (AUD_REG_PWM_BASE + 0xF07)
#define AUD_REG_PWM_COEF_ADDR                  (AUD_REG_PWM_BASE + 0xF08)
#define AUD_REG_PWM_COEF_DATA                  (AUD_REG_PWM_BASE + 0xF09)
#define AUD_REG_PWM_PIN0_MOD                   (AUD_REG_PWM_BASE + 0xF0A)
#define AUD_REG_PWM_PIN1_MOD                   (AUD_REG_PWM_BASE + 0xF0B)
#define AUD_REG_PWM_PIN2_MOD                   (AUD_REG_PWM_BASE + 0xF0C)
#define AUD_REG_PWM_PIN3_MOD                   (AUD_REG_PWM_BASE + 0xF0D)
#define AUD_REG_PWM_CH0_CTRL                   (AUD_REG_PWM_BASE + 0xF0E)
#define AUD_REG_PWM_CH1_CTRL                   (AUD_REG_PWM_BASE + 0xF0F)
#define AUD_REG_PWM_CH2_CTRL                   (AUD_REG_PWM_BASE + 0xF10)
#define AUD_REG_PWM_CH3_CTRL                   (AUD_REG_PWM_BASE + 0xF11)
#define AUD_REG_PWM_CH4_CTRL                   (AUD_REG_PWM_BASE + 0xF12)
#define AUD_REG_PWM_CH5_CTRL                   (AUD_REG_PWM_BASE + 0xF13)
#define AUD_REG_PWM_PIN4_MOD                   (AUD_REG_PWM_BASE + 0xF14)
#define AUD_REG_PWM_PIN5_MOD                   (AUD_REG_PWM_BASE + 0xF15)
#define AUD_REG_PWM_PIN6_MOD                   (AUD_REG_PWM_BASE + 0xF16)
#define AUD_REG_PWM_PIN7_MOD                   (AUD_REG_PWM_BASE + 0xF17)
#define AUD_REG_PWM_PIN8_MOD                   (AUD_REG_PWM_BASE + 0xF18)
#define AUD_REG_PWM_PIN9_MOD                   (AUD_REG_PWM_BASE + 0xF19)
#define AUD_REG_PWM_PIN10_MOD                  (AUD_REG_PWM_BASE + 0xF1A)
#define AUD_REG_PWM_PIN11_MOD                  (AUD_REG_PWM_BASE + 0xF1B)
#define AUD_REG_PWM_ERR_TST                    (AUD_REG_PWM_BASE + 0xF1C)
#define AUD_REG_PWM_OUTPUT_STATUS              (AUD_REG_PWM_BASE + 0xF1D)
#define AUD_REG_PWM_RAMP_FLAG                  (AUD_REG_PWM_BASE + 0xF1E)


/////////////////////////////////////////////////////////////////////////////
//         RGBK2 Register
/////////////////////////////////////////////////////////////////////////////
#define AUD_REG_RGBK2_BASE                     (0xA8000)

#define AUD_REG_RGBK2_PWMTOP_CFG               (AUD_REG_RGBK2_BASE + 0x00)
#define AUD_REG_RGBK2_FRNT_PWM_CFG1            (AUD_REG_RGBK2_BASE + 0x04)
    #define AUD_FRNT_PWM_IN_SEL_BIT_START      (6)
    #define AUD_FRNT_PWM_IN_SEL_BIT_NUM        (2)
#define AUD_REG_RGBK2_FRNT_PWM_CFG2            (AUD_REG_RGBK2_BASE + 0x08)
#define AUD_REG_RGBK2_FRNT_PWM_CFG3            (AUD_REG_RGBK2_BASE + 0x0C)
#define AUD_REG_RGBK2_FRNT_PWM_CFG4            (AUD_REG_RGBK2_BASE + 0x10)
#define AUD_REG_RGBK2_FRNT_PWM_CFG5            (AUD_REG_RGBK2_BASE + 0x14)
#define AUD_REG_RGBK2_FRNT_PWM_CFG6            (AUD_REG_RGBK2_BASE + 0x18)
#define AUD_REG_RGBK2_FRNT_PWM_CFG7            (AUD_REG_RGBK2_BASE + 0x1C)
#define AUD_REG_RGBK2_FRNT_PWM_CFG8            (AUD_REG_RGBK2_BASE + 0x20)
#define AUD_REG_RGBK2_FRNT_PWM_CFG9            (AUD_REG_RGBK2_BASE + 0x24)
#define AUD_REG_RGBK2_FRNT_PWM_CFG10           (AUD_REG_RGBK2_BASE + 0x28)
#define AUD_REG_RGBK2_REAR_PWM_CFG1            (AUD_REG_RGBK2_BASE + 0x2C)
    #define AUD_REAR_PWM_IN_SEL_BIT_START      (6)
    #define AUD_REAR_PWM_IN_SEL_BIT_NUM        (2)
#define AUD_REG_RGBK2_REAR_PWM_CFG2            (AUD_REG_RGBK2_BASE + 0x30)
#define AUD_REG_RGBK2_REAR_PWM_CFG3            (AUD_REG_RGBK2_BASE + 0x34)
#define AUD_REG_RGBK2_REAR_PWM_CFG4            (AUD_REG_RGBK2_BASE + 0x38)
#define AUD_REG_RGBK2_REAR_PWM_CFG5            (AUD_REG_RGBK2_BASE + 0x3C)
#define AUD_REG_RGBK2_REAR_PWM_CFG6            (AUD_REG_RGBK2_BASE + 0x40)

#define AUD_REG_RGBK2_ARM_MIC_SBLK             (AUD_REG_RGBK2_BASE + 0x40)
    #define AUD_MIC_SBLK_BIT_START             (0)
    #define AUD_MIC_SBLK_BIT_NUM               (16)

#define AUD_REG_RGBK2_REAR_PWM_CFG7            (AUD_REG_RGBK2_BASE + 0x44)
#define AUD_REG_RGBK2_REAR_PWM_CFG8            (AUD_REG_RGBK2_BASE + 0x48)
#define AUD_REG_RGBK2_REAR_PWM_CFG9            (AUD_REG_RGBK2_BASE + 0x4C)
#define AUD_REG_RGBK2_REAR_PWM_CFG10           (AUD_REG_RGBK2_BASE + 0x50)
#define AUD_REG_RGBK2_GPS_PWM_CFG1             (AUD_REG_RGBK2_BASE + 0x54)
#define AUD_REG_RGBK2_GPS_PWM_CFG2             (AUD_REG_RGBK2_BASE + 0x58)
#define AUD_REG_RGBK2_GPS_PWM_CFG3             (AUD_REG_RGBK2_BASE + 0x5C)
#define AUD_REG_RGBK2_GPS_PWM_CFG4             (AUD_REG_RGBK2_BASE + 0x60)
#define AUD_REG_RGBK2_GPS_PWM_CFG5             (AUD_REG_RGBK2_BASE + 0x64)
#define AUD_REG_RGBK2_GPS_PWM_CFG6             (AUD_REG_RGBK2_BASE + 0x68)
#define AUD_REG_RGBK2_GPS_PWM_CFG7             (AUD_REG_RGBK2_BASE + 0x6C)
#define AUD_REG_RGBK2_GPS_PWM_CFG8             (AUD_REG_RGBK2_BASE + 0x70)

#define AUD_GPS_AOUT_INTR_CLR_BIT_START        1
#define AUD_GPS_AOUT_INTR_CLR_BIT_NUM          1
#define AUD_GPS_OUT2_INTR_CLR_BIT_START        2
#define AUD_GPS_OUT2_INTR_CLR_BIT_NUM          1

#define AUD_REG_RGBK2_GPS_PWM_CFG9             (AUD_REG_RGBK2_BASE + 0x74)
#define AUD_REG_RGBK2_GPS_PWM_CFG10            (AUD_REG_RGBK2_BASE + 0x78)
#define AUD_REG_RGBK2_CFG0                     (AUD_REG_RGBK2_BASE + 0x80)
#define AUD_REG_RGBK2_CFG1                     (AUD_REG_RGBK2_BASE + 0x84)
#define AUD_REG_RGBK2_CFG2                     (AUD_REG_RGBK2_BASE + 0x88)
#define AUD_REG_RGBK2_CFG3                     (AUD_REG_RGBK2_BASE + 0x8C)
#define AUD_REG_RGBK2_CFG4                     (AUD_REG_RGBK2_BASE + 0x90)
    #define AUD_LINEIN_CLK_FROM_MLINE_START         (4)
    #define AUD_LINEIN_CLK_FROM_MLINE_NUM           (1)
#define AUD_REG_RGBK2_AOUT_CFG0                (AUD_REG_RGBK2_BASE + 0xC0)
    #define AUD_GPS_AOUT_CYC_BIT_START         (0)
    #define AUD_GPS_AOUT_CYC_BIT_NUM           (2)
    #define AUD_GPS_AOUT_BCK_BIT_START         (16)
    #define AUD_GPS_AOUT_BCK_BIT_NUM           (4)
    #define AUD_GPS_AOUT_IWL_BIT_START         (23)
    #define AUD_GPS_AOUT_IWL_BIT_NUM           (6)
#define AUD_REG_RGBK2_AOUT_CFG1                (AUD_REG_RGBK2_BASE + 0xC4)
#define AUD_REG_RGBK2_AOUT_CFG2                (AUD_REG_RGBK2_BASE + 0xC8)
    #define AUD_FRNT_SRC_SEL_BIT_START         (16)
    #define AUD_FRNT_SRC_SEL_BIT_NUM           (2)
    #define AUD_REAR_SRC_SEL_BIT_START         (18)
    #define AUD_REAR_SRC_SEL_BIT_NUM           (2)
#define AUD_REG_RGBK2_AOUT_CFG3                (AUD_REG_RGBK2_BASE + 0xCC)
#define AUD_REG_RGBK2_AFE_TOP_CFG0             (AUD_REG_RGBK2_BASE + 0xD0)
    #define AUD_LINEIN_CLK_FROM_AOUT2_START        (14)
    #define AUD_LINEIN_CLK_FROM_AOUT2_NUM          (1)

#define AUD_REG_RGBK2_BYPS_VLUM_CFG0           (AUD_REG_RGBK2_BASE + 0xD4)
#define AUD_REG_RGBK2_BYPS_VLUM_CFG1           (AUD_REG_RGBK2_BASE + 0xD8)

#define AUD_MPHONE_SEL_ADC_START               (6)
#define AUD_MPHONE_SEL_ADC_NUM                 (1)

#define AUD_REG_RGBK2_BT_PCM_BLK_CFG           (AUD_REG_RGBK2_BASE + 0xE0)
#define AUD_REG_RGBK2_AFE_DBG_ASDR             (AUD_REG_RGBK2_BASE + 0xE4)
#define AUD_REG_RGBK2_AFE_DBG_EADR             (AUD_REG_RGBK2_BASE + 0xE8)
#define AUD_REG_RGBK2_PWMANA_CFG0              (AUD_REG_RGBK2_BASE + 0x1C0)  // PWMDAC Register
#define AUD_REG_RGBK2_PWMANA_CFG1              (AUD_REG_RGBK2_BASE + 0x1C4)
#define AUD_REG_RGBK2_PWMANA_CFG2              (AUD_REG_RGBK2_BASE + 0x1C8)
#define AUD_REG_RGBK2_PWMANA_CFG3              (AUD_REG_RGBK2_BASE + 0x1CC)
#define AUD_REG_RGBK2_PWMANA_CFG4              (AUD_REG_RGBK2_BASE + 0x1D0)
#define AUD_REG_RGBK2_PWMANA_CFG5              (AUD_REG_RGBK2_BASE + 0x1D4)
#define AUD_REG_RGBK2_PWMANA_CFG6              (AUD_REG_RGBK2_BASE + 0x1D8)
#define AUD_REG_RGBK2_PWMANA_CFG7              (AUD_REG_RGBK2_BASE + 0x1DC)
#define AUD_REG_RGBK2_APLL_ADJ_CFG0            (AUD_REG_RGBK2_BASE + 0x1E0)
#define AUD_REG_RGBK2_APLL_ADJ_CFG1            (AUD_REG_RGBK2_BASE + 0x1E4)
#define AUD_REG_RGBK2_APLL_ADJ_STATUS0         (AUD_REG_RGBK2_BASE + 0x1E8)
#define AUD_REG_RGBK2_APLL_ADJ_STATUS1         (AUD_REG_RGBK2_BASE + 0x1EC)
#define AUD_REG_RGBK2_APLL_ADJ_STATUS2         (AUD_REG_RGBK2_BASE + 0x1F0)


#define AUD_REG_RGBK2_INDRECT_FRNT_ADDR        (AUD_REG_RGBK2_BASE + 0x240)
#define AUD_REG_RGBK2_INDRECT_FRNT_DATA        (AUD_REG_RGBK2_BASE + 0x244)
#define AUD_REG_RGBK2_INDRECT_REAR_ADDR        (AUD_REG_RGBK2_BASE + 0x248)
#define AUD_REG_RGBK2_INDRECT_REAR_DATA        (AUD_REG_RGBK2_BASE + 0x24C)
#define AUD_REG_RGBK2_INDRECT_GPS_ADDR         (AUD_REG_RGBK2_BASE + 0x250)
#define AUD_REG_RGBK2_INDRECT_GPS_DATA         (AUD_REG_RGBK2_BASE + 0x254)


/////////////////////////////////////////////////////////////////////////////
//         GPS AOUT Register
/////////////////////////////////////////////////////////////////////////////
#define AUD_REG_GPS_AOUT_BASE           (0xA8400)

#define AUD_REG_GOUT_CH1_BUF_SADR         (AUD_REG_GPS_AOUT_BASE + 0x00)
#define AUD_REG_GOUT_CH1_BUF_SIZE         (AUD_REG_GPS_AOUT_BASE + 0x04)
#define AUD_REG_GOUT_CH1_NSADR            (AUD_REG_GPS_AOUT_BASE + 0x08)
#define AUD_REG_GPS_AOUT_SN               (AUD_REG_GPS_AOUT_BASE + 0x38)
#define AUD_REG_GPS_AOUT_INTR_NUM         (AUD_REG_GPS_AOUT_BASE + 0x3c)
#define AUD_REG_GPS_AOUT_CTRL             (AUD_REG_GPS_AOUT_BASE + 0x40)
#define AUD_REG_GPS_AOUT_CH_CFG           (AUD_REG_GPS_AOUT_BASE + 0x44)
#define AUD_REG_GPS_AOUT_CH_NUM           (AUD_REG_GPS_AOUT_BASE + 0x50)


/////////////////////////////////////////////////////////////////////////////
//         AOUT2 Register
/////////////////////////////////////////////////////////////////////////////
#define AUD_REG_AOUT2_BASE                     (0xA8500)

#define AUD_REG_AOUT2_CH1_SADR                 (AUD_REG_AOUT2_BASE + 0x00)
#define AUD_REG_AOUT2_CH1_SIZE                 (AUD_REG_AOUT2_BASE + 0x04)
#define AUD_REG_AOUT2_CH1_NSADR                (AUD_REG_AOUT2_BASE + 0x08)
#define AUD_REG_AOUT2_CH2_NSADR                (AUD_REG_AOUT2_BASE + 0x0C)
#define AUD_REG_AOUT2_CH3_NSADR                (AUD_REG_AOUT2_BASE + 0x10)
#define AUD_REG_AOUT2_CH4_NSADR                (AUD_REG_AOUT2_BASE + 0x14)
#define AUD_REG_AOUT2_CH5_NSADR                (AUD_REG_AOUT2_BASE + 0x18)
#define AUD_REG_AOUT2_CH6_NSADR                (AUD_REG_AOUT2_BASE + 0x1C)
#define AUD_REG_AOUT2_CH7_NSADR                (AUD_REG_AOUT2_BASE + 0x20)
#define AUD_REG_AOUT2_CH8_NSADR                (AUD_REG_AOUT2_BASE + 0x24)
#define AUD_REG_AOUT2_CH9_NSADR                (AUD_REG_AOUT2_BASE + 0x28)
#define AUD_REG_AOUT2_CH10_NSADR               (AUD_REG_AOUT2_BASE + 0x2C)
#define AUD_REG_AOUT2_CH11_NSADR               (AUD_REG_AOUT2_BASE + 0x30)
#define AUD_REG_AOUT2_CH12_NSADR               (AUD_REG_AOUT2_BASE + 0x34)
#define AUD_REG_AOUT2_NSNUM                    (AUD_REG_AOUT2_BASE + 0x38)
#define AUD_REG_AOUT2_INTR_SIZE                (AUD_REG_AOUT2_BASE + 0x3C)
#define AUD_REG_AOUT2_CTRL                     (AUD_REG_AOUT2_BASE + 0x40)
#define AUD_REG_AOUT2_CFG0                     (AUD_REG_AOUT2_BASE + 0x44)
#define AUD_REG_AOUT2_CFG1                     (AUD_REG_AOUT2_BASE + 0x48)
#define AUD_REG_AOUT2_CFG2                     (AUD_REG_AOUT2_BASE + 0x4C)
#define AUD_REG_AOUT2_CH_NUM                   (AUD_REG_AOUT2_BASE + 0x50)
#define AUD_REG_AOUT2_SWITCH                   (AUD_REG_AOUT2_BASE + 0x54)
#define AUD_REG_AOUT2_LEV_DET_CFG              (AUD_REG_AOUT2_BASE + 0x58)
#define AUD_REG_AOUT2_LEVEL                    (AUD_REG_AOUT2_BASE + 0x5C)
#define AUD_REG_AOUT2_BUF_MIX_CFG              (AUD_REG_AOUT2_BASE + 0x60)
#define AUD_REG_AOUT2_MIX1_NSADR               (AUD_REG_AOUT2_BASE + 0x64)
#define AUD_REG_AOUT2_MIX2_NSADR               (AUD_REG_AOUT2_BASE + 0x68)
#define AUD_REG_AOUT2_LRCK_CNT                 (AUD_REG_AOUT2_BASE + 0x6C)
//#define AUD_REG_AOUT2_SACD_L_VOLUME            (AUD_REG_AOUT2_BASE + 0x70)
//#define AUD_REG_AOUT2_SACD_R_VOLUME            (AUD_REG_AOUT2_BASE + 0x74)
//#define AUD_REG_AOUT2_SACD_SL_VOLUME           (AUD_REG_AOUT2_BASE + 0x78)
//#define AUD_REG_AOUT2_SACD_SR_VOLUME           (AUD_REG_AOUT2_BASE + 0x7C)
//#define AUD_REG_AOUT2_SACD_C_VOLUME            (AUD_REG_AOUT2_BASE + 0x80)
//#define AUD_REG_AOUT2_SACD_LFE_VOLUME          (AUD_REG_AOUT2_BASE + 0x84)
//#define AUD_REG_AOUT2_SACD_FADER_CTRL          (AUD_REG_AOUT2_BASE + 0x88)
//#define AUD_REG_AOUT2_SACD_FADER_MONITOR       (AUD_REG_AOUT2_BASE + 0x8C)
//#define AUD_REG_AOUT2_SACD_FADER_FLAG          (AUD_REG_AOUT2_BASE + 0x90)
//#define AUD_REG_AOUT2_SACD_ANCILLARY           (AUD_REG_AOUT2_BASE + 0x94)
//#define AUD_REG_AOUT2_SACD_FRAME               (AUD_REG_AOUT2_BASE + 0x98)
//#define AUD_REG_AOUT2_SACD_FCK_CFG             (AUD_REG_AOUT2_BASE + 0x9C)
#define AUD_REG_AOUT2_IIR_SEL                  (AUD_REG_AOUT2_BASE + 0xA0)
#define AUD_REG_AOUT2_IIR_COEF                 (AUD_REG_AOUT2_BASE + 0xA4)


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
#define AUD_REG_MMR                            (0x00)

//bit0~19: write compact data address
#define AUD_REG_SET_WDCMPT_ADDR                (AUD_REG_MMR + 0x51)
//bit0~19: read compact data address
#define AUD_REG_SET_RDCMPT_ADDR                (AUD_REG_MMR + 0x52)
//bit0~1: compact write data bit width selection
//bit2~3: compact read data bit width selection
//0 is 24bits, 1 is 16bits, 2 is 8bits, 3 is 4bits.
#define AUD_REG_CMPT_BWSEL                     (AUD_REG_MMR + 0x53)
//0~2 ADSP read/write DRAM cache page selection
#define AUD_REG_SET_DRAM_PAGE                  (AUD_REG_MMR + 0x54)
//0~2 ADSP compact read/write DRAM cache page selection
#define AUD_REG_SET_CMPT_PAGE                  (AUD_REG_MMR + 0x55)
//0~2 ADSP read DMA cache page selection
#define AUD_REG_SET_WDMA_PAGE                  (AUD_REG_MMR + 0x56)
//0~2 ADSP write DMA cache page selection
#define AUD_REG_SET_RDMA_PAGE                  (AUD_REG_MMR + 0x57)
//0~19 ADSP read DMA DRAM address
#define AUD_REG_SET_RDMA_ADDR                  (AUD_REG_MMR + 0x58)
//0~19 ADSP write DMA DRAM address
#define AUD_REG_SET_WDMA_ADDR                  (AUD_REG_MMR + 0x59)

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



//afifo register

#define AUD_REG_OFST     (0x00)
//5080
#define RW_DSP_SW_BS0_SBLK    (AUD_REG_OFST + (0x80>>2))
#define BS0_HW_SW_MOD_SEL      0x80000000 //(0x1<<31) //HW mode: 0, SW mode: 1 //irlian: don't use 0x1<<31 to avoid warning
//5084
#define RW_DSP_SW_BS0_EBLK    (AUD_REG_OFST + (0x84>>2))
//5088
#define RW_DSP_SW_BS0_PPNT    (AUD_REG_OFST + (0x88>>2))
//508C
#define RW_DSP_SW_BS1_SBLK    (AUD_REG_OFST + (0x8C>>2))
#define BS1_HW_SW_MOD_SEL     0x80000000 //(0x1<<31)
//5090
#define RW_DSP_SW_BS1_EBLK    (AUD_REG_OFST + (0x90>>2))
//5094
#define RW_DSP_SW_BS1_PPNT    (AUD_REG_OFST + (0x94>>2))

//50B0
#define RW_ABUF0_PNT          (AUD_REG_OFST + (0xB0 >> 2))
                              //Bit0 [W]: 0: read audio buffer pointer
                              //        1: read SPDIF/LineIn buffer pointer
                              //Bit0~31 [R]: buffer pointer pointer value according to bit0
//50B4
#define RW_ABUF1_PNT          (AUD_REG_OFST + (0xB4 >> 2))
                              //Bit0 [W]: 0: read audio buffer pointer
                              //        1: read SPDIF/LineIn buffer pointer
                              //Bit0~31 [R]: buffer pointer pointer value according to bit0



#endif // #ifndef _AUDIO_3360_REG_MISC_H_
