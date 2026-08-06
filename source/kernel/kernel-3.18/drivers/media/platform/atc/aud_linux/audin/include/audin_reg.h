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




#if 0

#ifndef __AUD_IN_REG_H
#define __AUD_IN_REG_H
    
#include "x_aud_dec.h"
#include "x_ckgen.h"

    
#ifdef __cplusplus
    extern "C"
    {
#endif


/*******************************************
 *                    setting              *
 *******************************************/
#define AUDIN_ENABLE                      (1)
#define AUDIN_DISABLE                     (0)

#define AUD_LINE_IN_ADDR_START_POS             (8)
#define AUD_LINE_IN_ADDR_LEN                   (16)
#define AUD_LINE_IN_BANK_ADDR_START_POS        (20)
#define AUD_LINE_IN_BANK_ADDR_LEN              (10)
#define AUD_LINE_IN_BLK_START_POS              (0)
#define AUD_LINE_IN_BLK_ADDR_LEN               (0)



#define AUD_LINE_IN_BUF_SIZE          (0x6000)
#define AUD_LINE_IN_ALN               (256)

/*audio_intf*/
//audio base addr
#define AUD_INTF_BASE_ADDR                   (0x5000)

#define AUD_SPLIN_BLK_ADDR              (AUD_INTF_BASE_ADDR + 0x1c)
#define AUD_SPLIN_END_BIT_START         (0)
#define AUD_SPLIN_END_BIT_NUM           (16)
#define AUD_SPLIN_START_BIT_START       (16)
#define AUD_SPLIN_START_BIT_NUM         (16)

//spdif/line in ctrol
#define AUD_SPLIN_CTRL_ADDR             (AUD_INTF_BASE_ADDR + 0x20)
    #define AUD_LINE_EN_BIT_START            (0)
    #define AUD_LINE_EN_BIT_NUM              (1)

    #define AUD_SPLIN_CTRL_BIT24_BIT_START   (1)
    #define AUD_SPLIN_CTRL_BIT24_BIT_NUM     (1)


    #define AUD_SPLIN_CTRL_INTPRD_START      (4)
    #define AUD_SPLIN_CTRL_INTPRD_NUM        (2)
    #define AUD_SPLIN_DATALEN_INTR_DISABLE   (0)    
    #define AUD_SPLIN_DATALEN_INTR_64DW      (1)
    #define AUD_SPLIN_DATALEN_INTR_128DW     (2)
    #define AUD_SPLIN_DATALEN_INTR_256DW     (3)

#define AUD_AEVN_BAK_ADDR               (AUD_INTF_BASE_ADDR + 0x2c)
    #define AUD_AEVN_INV_BCK_BIT_START      (31)
    #define AUD_AEVN_INV_BCK_BIT_NUM        (1)
    
#define AUD_LINE_IN_BANK_ADDR           (AUD_INTF_BASE_ADDR + 0x3a0)
#define AUD_LINE_IN_BANK_BIT_START         (22)
#define AUD_LINE_IN_BANK_BIT_NUM           (10)

//ext dsp on, arm program dsp reg must on
#define AUD_RISC_REG_EXT_ON             (AUD_INTF_BASE_ADDR + 0x3e0)
#define AUD_RISC_EXT_DSP_ON                    (0x6a)

#define AUD_RISC_REG_EXT_BANK           (AUD_INTF_BASE_ADDR + 0x3e4)
#define AUD_RISC_DSP_ENABLE                    (1)


#define AUD_AIN_CFG_ADDR                (AUD_INTF_BASE_ADDR + 0xc4)    
    #define AUD_SPL_BNUM_BIT_START          (8)
    #define AUD_SPL_BNUM_BIT_NUM            (5)
    #define AUD_LEFT_ALN_DLY_BIT_START      (13)
    #define AUD_LEFT_ALN_DLY_BIT_NUM        (2)
    #define AUD_RIGHT_JUSTIFIED             (0)
    #define AUD_LEFT_JUSTIFIED              (1)
    #define AUD_IIS_INTF                    (3)

    #define AUD_LRCK_CYCLE_BIT_START        (16)
    #define AUD_LRCK_CYCLE_BIT_NUM          (2)
    
    #define AUD_LINEIN_ACK_SEL_START        (20)
    #define AUD_LINEIN_ACK_SEL_BIT_NUM      (1)
    #define AUD_LINEIN_INTERNAL_MODE        (0)
    #define AUD_SPDIFIN_EXT_MODE            (1)



#define AUD_AOUT_CFG_ADDR               (AUD_INTF_BASE_ADDR + 0xc0)
#define AUD_AOUT_LRCK_CYC_BIT_START     (0)
#define AUD_AOUT_LRCK_CYC_BIT_NUM       (2)

#define AUD_MISC_CTRL_ADDR              (AUD_INTF_BASE_ADDR + 0xcc)


#define AUD_REG_ABUF0_PNT               (AUD_INTF_BASE_ADDR + (0xB0 ))



#if 0

/*PWM_REG*/
#define AUD_PWM_BASE               (0xa8000)

#define AUD_BCK_DIVIDER            (AUD_PWM_BASE + 0x90)
#define AUD_BCK_DIV_BIT_START           (0)
#define AUD_BCK_DIV_BIT_NUM             (4)






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
#define AUD_REG_RGBK2_GPS_PWM_CFG9             (AUD_REG_RGBK2_BASE + 0x74)
#define AUD_REG_RGBK2_GPS_PWM_CFG10            (AUD_REG_RGBK2_BASE + 0x78)
#define AUD_REG_RGBK2_CFG0                     (AUD_REG_RGBK2_BASE + 0x80)
#define AUD_REG_RGBK2_CFG1                     (AUD_REG_RGBK2_BASE + 0x84)
#define AUD_REG_RGBK2_CFG2                     (AUD_REG_RGBK2_BASE + 0x88)
#define AUD_REG_RGBK2_CFG3                     (AUD_REG_RGBK2_BASE + 0x8C)
//lin clk use inter mlineckgen clk
#define AUD_REG_RGBK2_CFG4                     (AUD_REG_RGBK2_BASE + 0x90)
    #define AUD_LINEIN_CLK_FROM_MLINE_START         (4)
    #define AUD_LINEIN_CLK_FROM_MLINE_NUM           (1)
    #define AUD_LINEIN_CLK_FROM_MLINE_ENABLE        (1)

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
    #define AUD_AFE1_SRC_CLK_SEL_START             (2)
    #define AUD_AFE1_SRC_CLK_SEL_NUM               (2)
     
    #define AUD_AFE2_SRC_CLK_SEL_START             (4)
    #define AUD_AFE2_SRC_CLK_SEL_NUM               (2)

    #define AUD_MLINE_SEL_ADC_START                (7)
    #define AUD_MLINE_SEL_ADC_NUM                   (1)

    #define AUD_LINEIN_CLK_FROM_AOUT2_START        (14)
    #define AUD_LINEIN_CLK_FROM_AOUT2_NUM          (1)
    
    #define AUD_AFE1_DST_CLK_SEL_START             (16)
    #define AUD_AFE1_DST_CLK_SEL_NUM               (2)
    
    #define AUD_AFE2_DST_CLK_SEL_START             (18)
    #define AUD_AFE2_DST_CLK_SEL_NUM               (2)
    
    #define AUD_AFE_CLK_AOUT1                      (0)
    #define AUD_AFE_CLK_AOUT2                      (1)
    #define AUD_AFE_CLK_MLINE                      (2)
    #define AUD_AFE_CLK_MPHONE                     (3)
    
#define AUD_REG_RGBK2_BYPS_VLUM_CFG0           (AUD_REG_RGBK2_BASE + 0xD4)
#define AUD_REG_RGBK2_BYPS_VLUM_CFG1           (AUD_REG_RGBK2_BASE + 0xD8)
  #define AUD_LINEIN_LINEIN_AFE_MODE_START          (4)
  #define AUD_LINEIN_LINEIN_AFE_MODE_NUM          (1)
  #define AUD_LINEIN_LINEIN_INTENAL_ADC_MODE      (1)
  #define AUD_LINEIN_LINEIN_EXTENAL_ADC_MODE      (0)


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


#define AUD_REG_RGBK2_AFE1_SET_ADDR            (AUD_REG_RGBK2_BASE + 0x2D4)
#define AUD_REG_RGBK2_AFE2_SET_ADDR            (AUD_REG_RGBK2_BASE + 0x354)


/////////////////////////////////////////////////////////////////////////////
//         HardWare Reset Register
/////////////////////////////////////////////////////////////////////////////
#define REG_AUDHW_RST_I                     0x00A8
#define REG_AUDHW_RST_II                    0x00C4


/////////////////////////////////////////////////////////////////////////////
//         clock set  Register
/////////////////////////////////////////////////////////////////////////////


#define REG_AP_REG4                         0x001C
    #define AUD_REG_CFG_K3_START            (16)
    #define AUD_REG_CFG_K3_NUM              (8)


#define REG_AP_REG6                         0x0024
    #define AUD_REG_SEL_APLL2_K3_START      (14)
    #define AUD_REG_SEL_APLL2_K3_NUM        (1)

#define REG_AP_REG9                         0x0030
    #define AUD_MLINE_CLK_SEL_START        (26)
    #define AUD_MLINE_CLK_SEL_NUM          (1)

    /////////////////////////////////////////////////////////////////////////////
    //         EXT LIN TEST
    /////////////////////////////////////////////////////////////////////////////

#define REG_AP_REG2                         0x0014
#define REG_AP_REG3                         0x0018

#define AUD_AOUT_A2BCKX_BIT_START       (16)
#define AUD_AOUT_A2BCKX_BIT_NUM         (4)
#define AUD_AOUT2_LRCK_CYC_BIT_START       (17)
#define AUD_AOUT2_LRCK_CYC_BIT_NUM         (2)

#define AUD_AOUT2_A2BCKX_BIT_START         (12)
#define AUD_AOUT2_A2BCKX_BIT_NUM           (4)

#define AUD_INV_BCK_BIT_START           (6)
#define AUD_INV_BCK_BIT_NUM             (1)

#define AUD_AOUT2_BCK_INV_START            (21)
 #define AUD_AOUT2_BCK_INV_BIT_NUMBER       (1)
#endif 
#define ECO_AUD_LINE_IN_TEST_BUF_SIZE                                           (0xc000)
#define ECO_AUD_LINE_IN_ALN                                                     (256)
/////////////////////////////////////////////////////////////////////////////
//                   AFE1 Register
/////////////////////////////////////////////////////////////////////////////
#define ECO_AUD_REG_AFE1_BASE                                                  (0xA8200)

#define ECO_AUD_REG_AFE1_CFG19                                                 (ECO_AUD_REG_AFE1_BASE + 0xD0)
    #define ECO_AUD_CH2_MAUNAL_GAIN_SEL_START                                  (15)
    #define ECO_AUD_CH2_MAUNAL_GAIN_SEL_BIT_NUM                                (1)
    #define ECO_AUD_CH2_MAUNAL_GAIN_SEL_MODE                                   (1)//0 auto_gain,1:menu_gain

    #define ECO_AUD_CH1_MAUNAL_GAIN_SEL_START                                  (16)
    #define ECO_AUD_CH1_MAUNAL_GAIN_SEL_BIT_NUM                                (1)
    #define ECO_AUD_CH1_MAUNAL_GAIN_SEL_MODE                                   (1)//0 auto_gain,1:menu_gain
    
    #define ECO_AUD_CH2_MAUNAL_GAIN_START                                      (17)
    #define ECO_AUD_CH2_MAUNAL_GAIN_BIT_NUM                                    (6)
    #define ECO_AUD_CH2_MAUNAL_GAIN_VALUE                                      (14)//0 auto_gain,1:menu_gain

    #define ECO_AUD_CH1_MAUNAL_GAIN_START                                      (23)
    #define ECO_AUD_CH1_MAUNAL_GAIN_BIT_NUM                                    (6)
    #define ECO_AUD_CH1_MAUNAL_GAIN_VALUE                                      (14)//0 auto_gain,1:menu_gain

/////////////////////////////////////////////////////////////////////////////
//              AADC Regs        
/////////////////////////////////////////////////////////////////////////////
#define ECO_AADC_REG_BASE                                                       (0x300)

#define ECO_AUD_ANALOG_AADC_CFG0                                                (ECO_AADC_REG_BASE + 0x00)
    #define ECO_AUD_FRONT_LEFT_PREAMP_CTRL_START                                (0)    
    #define ECO_AUD_FRONT_LEFT_PREAMP_CTRL_BIT_NUM                              (1)
    #define ECO_AUD_FRONT_LEFT_PREAMP_CTRL_ENABLE                               (1)

    #define ECO_AUD_FRONT_LEFT_PREAMP_INPUT_SEL_START                           (2)    
    #define ECO_AUD_FRONT_LEFT_PREAMP_INPUT_SEL_BIT_NUM                         (3)   
    #define ECO_AUD_FRONT_LEFT_PREAMP_INPUT_SEL_VALUE                           (1) //001:AIN1, 010:AN2,011:AIN3

    #define ECO_AUD_FRONT_RIGHT_PREAMP_CTRL_START                               (8)    
    #define ECO_AUD_FRONT_RIGHT_PREAMP_CTRL_BIT_NUM                             (1)
    #define ECO_AUD_FRONT_RIGHT_PREAMP_CTRL_ENABLE                              (1) 

    #define ECO_AUD_FRONT_RIGHT_PREAMP_INPUT_SEL_START                           (10)    
    #define ECO_AUD_FRONT_RIGHT_PREAMP_INPUT_SEL_BIT_NUM                         (3)   
    #define ECO_AUD_FRONT_RIGHT_PREAMP_INPUT_SEL_VALUE                           (1) //001:AIN1, 010:AN2,011:AIN3


#define ECO_AUD_ANALOG_AADC_CFG1                                                (ECO_AADC_REG_BASE + 0x04)
    #define ECO_AADC_LIN_ADC_POWER_CTRL_START                                   (16)
    #define ECO_AADC_LIN_ADC_POWER_CTRL_BIT_NUM                                 (4)
    #define ECO_AADC_LIN_ADC_POWER_CTRL_VALUE                                   (0xF)


#define ECO_AUD_ANALOG_AADC_CFG3                                                (ECO_AADC_REG_BASE + 0x0C)
    #define ECO_AADC_LIN_FRNT_LEFT_ADC_INPUT_SEL_START                          (2)
    #define ECO_AADC_LIN_FRNT_LEFT_ADC_INPUT_SEL_BIT_NUM                        (3)
    #define ECO_AADC_MIN_FRNT_LEFT_ADC_INPUT_SEL_RIGHT_PREAMP_VALUE             (4)
    #define ECO_AADC_LIN_FRNT_LEFT_ADC_INPUT_SEL_VALUE                          (5)

    #define ECO_AADC_LIN_REAR_LEFT_ADC_INPUT_SEL_START                          (5)
    #define ECO_AADC_LIN_REAR_LEFT_ADC_INPUT_SEL_BIT_NUM                        (3)
    #define ECO_AADC_LIN_REAR_LEFT_ADC_INPUT_SEL_VALUE                          (5)

    #define ECO_AADC_LIN_FRNT_RIGHT_ADC_INPUT_SEL_START                         (13)
    #define ECO_AADC_LIN_FRNT_RIGHT_ADC_INPUT_SEL_BIT_NUM                       (3)
    #define ECO_AADC_MIN_FRNT_LEFT_ADC_INPUT_SEL_RIGHT_PREAMP_VALUE             (4)
    #define ECO_AADC_LIN_FRNT_RIGHT_ADC_INPUT_SEL_VALUE                         (5)

    #define ECO_AADC_LIN_REAR_RIGHT_ADC_INPUT_SEL_START                         (21)
    #define ECO_AADC_LIN_REAR_RIGHT_ADC_INPUT_SEL_BIT_NUM                       (3)
    #define ECO_AADC_LIN_REAR_RIGHT_ADC_INPUT_SEL_VALUE                         (5)
    
#define ECO_AUD_ANALOG_AADC_CFG5                                                (ECO_AADC_REG_BASE + 0x14)
    #define ECO_AADC_LIN_FORNT_INPUT_AMP_DECREASE_START                         (0)
    #define ECO_AADC_LIN_FORNT_INPUT_AMP_DECREASE_BIT_NUM                        (2)
    #define ECO_AADC_LIN_FORNT_INPUT_AMP_DECREASE_VALUE                         (3)
    #define ECO_AADC_LIN_REAR_INPUT_AMP_DECREASE_START                             (8)
    #define ECO_AADC_LIN_REAR_INPUT_AMP_DECREASE_BIT_NUM                        (2)
    #define ECO_AADC_LIN_REAR_INPUT_AMP_DECREASE_VALUE                             (3)
#define ECO_AUD_ANALOG_AADC_CFG7                                                (ECO_AADC_REG_BASE + 0x1C)
    #define ECO_AADC_FRONT_MIN_BIAS_POWER_CTRL_START                            (28)
    #define ECO_AADC_FRONT_MIN_BIAS_POWER_CTRL_BIT_NUM                          (1)
    #define ECO_AADC_FRONT_MIN_BIAS_POWER_CTRL_VALUE                            (1)//0:POWER DOWN,1:POWER ON


#define ECO_AUD_ANALOG_AADC_CFG10                                               (ECO_AADC_REG_BASE + 0x28)
    #define ECO_AADC_LIN_FRNT_LEFT_BUFFER_GAIN_START                            (8)
    #define ECO_AADC_LIN_FRNT_LEFT_BUFFER_GAIN_BIT_NUM                          (6)
    #define ECO_AADC_LIN_FRNT_LEFT_BUFFER_GAIN_VALUE                            (3)

    #define ECO_AADC_LIN_REAR_LEFT_BUFFER_GAIN_START                            (14)
    #define ECO_AADC_LIN_REAR_LEFT_BUFFER_GAIN_BIT_NUM                          (6)
    #define ECO_AADC_LIN_REAR_LEFT_BUFFER_GAIN_VALUE                            (3)

    #define ECO_AADC_LIN_FRNT_RIGHT_BUFFER_GAIN_START                           (20)
    #define ECO_AADC_LIN_FRNT_RIGHT_BUFFER_GAIN_BIT_NUM                         (6)
    #define ECO_AADC_LIN_FRNT_RIGHT_BUFFER_GAIN_VALUE                           (3)

    #define ECO_AADC_LIN_REAR_RIGHT_BUFFER_GAIN_START                           (26)
    #define ECO_AADC_LIN_REAR_RIGHT_BUFFER_GAIN_BIT_NUM                         (6)
    #define ECO_AADC_LIN_REAR_RIGHT_BUFFER_GAIN_VALUE                           (3)


#define ECO_AUD_ANALOG_AADC_CFG11                                               (ECO_AADC_REG_BASE + 0x2c)
    #define ECO_AADC_LIN_BUFFER_POWER_CTRL_START                                (0)
    #define ECO_AADC_LIN_BUFFER_POWER_CTRL_BIT_NUM                              (4)
    #define ECO_AADC_LIN_BUFFER_POWER_CTRL_VALUE                                (0xF)

    #define ECO_AADC_LIN_FRNT_LEFT_BUFFER_INPUT_SEL_START                       (6)
    #define ECO_AADC_LIN_FRNT_LEFT_BUFFER_INPUT_SEL_BIT_NUM                     (3)
   
    #define ECO_AADC_LIN_REAR_LEFT_BUFFER_INPUT_SEL_START                       (9)
    #define ECO_AADC_LIN_REAR_LEFT_BUFFER_INPUT_SEL_BIT_NUM                     (3)

    #define ECO_AADC_LIN_FRNT_RIGHT_BUFFER_INPUT_SEL_START                      (12)
    #define ECO_AADC_LIN_FRNT_RIGHT_BUFFER_INPUT_SEL_BIT_NUM                    (3)
    
    #define ECO_AADC_LIN_REAR_RIGHT_BUFFER_INPUT_SEL_START                      (15)
    #define ECO_AADC_LIN_REAR_RIGHT_BUFFER_INPUT_SEL_BIT_NUM                    (3)

    
#define ECO_AUD_ANALOG_AADC_CFG12                                               (ECO_AADC_REG_BASE + 0x30)
    #define ECO_AADC_RCH_GPIO_ENABLE_CTRL_START                                 (8)
    #define ECO_AADC_RCH_GPIO_ENABLE_CTRL_BIT_NUM                               (5)
    #define ECO_AADC_RCH_GPIO_ENABLE_CTRL_ANALOG_MODE                           (0)


#define ECO_AUD_ANALOG_AADC_CFG14                                               (ECO_AADC_REG_BASE + 0x38)
    #define ECO_AADC_LCH_GPIO_ENABLE_CTRL_START                                 (8)
    #define ECO_AADC_LCH_GPIO_ENABLE_CTRL_BIT_NUM                               (5)
    #define ECO_AADC_LCH_GPIO_ENABLE_CTRL_ANALOG_MODE                           (0)


#define ECO_AUD_ANALOG_AADC_CFG16                                               (ECO_AADC_REG_BASE + 0x40)
    #define ECO_AADC_GLOBE_BIAS_POWER_CTRL_START                                (14) 
    #define ECO_AADC_GLOBE_BIAS_POWER_CTRL_BIT_NUM                              (1) 
    #define ECO_AADC_GLOBE_BIAS_POWER_CTRL_VALUE                                (0) //0:POWER ON,1:POWER DOWN

    #define ECO_AADC_RESET_CLK_CTRL_START                                       (22)
    #define ECO_AADC_RESET_CLK_CTRL_BIT_NUM                                     (1)
    #define ECO_AADC_RESET_CLK_CTRL_VALUE                                       (1) //0:RESET 1:NOT RESET

    #define ECO_AADC_SEL_CLK_FREQ_CTRL_START                                    (23)
    #define ECO_AADC_SEL_CLK_FREQ_CTRL_BIT_NUM                                  (1)
    #define ECO_AADC_SEL_CLK_FREQ_CTRL_VALUE                                    (0)//0:13m 1:26m


/////////////////////////////////////////////////////////////////////////////
//             PLLPG Regs        
/////////////////////////////////////////////////////////////////////////////
#define ECO_PLL_PG_REG_BASE                                                     (0x280)
    #define ECO_ANAALOG_PLLGP_CFG25                                                 (ECO_PLL_PG_REG_BASE + 0x68)

/////////////////////////////////////////////////////////////////////////////
//         CKgen Regs         
/////////////////////////////////////////////////////////////////////////////
#define ECO_CKGEN_REG_BASE                                                      (0x0)

#define ECO_CKGEN_AP_REG2                                                       (ECO_CKGEN_REG_BASE + 0x14)
    #define ECO_AOUT1_CLK_K2_START                                                (28)
    #define ECO_AOUT1_CLK_K2_BIT_NUM                                            (1)
    #define ECO_AOUT1_CLK_K2_ENABLE                                                (1)

#define ECO_CKGEN_AP_REG3                                                       (ECO_CKGEN_REG_BASE + 0x18) 
    #define ECO_AOUT2_CLK_K4_START                                                (0)
    #define ECO_AOUT2_CLK_K4_BIT_NUM                                             (2)
    #define ECO_AOUT2_CLK_K4_ENABLE                                                (1)

    #define ECO_MPHONE_SELF_CLK_SEL_START                                       (6)
    #define ECO_MPHONE_SELF_CLK_SEL_BIT_NUM                                     (2)
    #define ECO_MPHONE_SELF_CLK_SEL_VALUE                                       (1)//0:27m,1:ack_k6,2:mphone_in,3:spmclk_in



#define ECO_CKGEN_AP_REG4                                                       (ECO_CKGEN_REG_BASE + 0x1C)
    #define ECO_MCLK_DIVD_RATIO_K2_AOUT1_START                                  (8)
    #define ECO_MCLK_DIVD_RATIO_K2_AOUT1_BIT_NUM                                (8)

    #define ECO_MCLK_DIVD_RATIO_K4_AOUT2_START                                  (24)
    #define ECO_MCLK_DIVD_RATIO_K4_AOUT2_BIT_NUM                                (8)

#define ECO_CKGEN_AP_REG5                                                       (ECO_CKGEN_REG_BASE + 0x20)
    #define ECO_MCLK_DIVD_RATIO_K6_MPHONE_START                                  (8)
    #define ECO_MCLK_DIVD_RATIO_K6_MPHONE_BIT_NUM                                (8)

#define ECO_CKGEN_AP_REG6                                                       (ECO_CKGEN_REG_BASE + 0x24)
    #define ECO_APLL_K2_FOR_AOUT1_START                                         (13)
    #define ECO_APLL_K2_FOR_AOUT1_BIT_NUM                                       (1)
    #define ECO_APLL_K2_FOR_AOUT1_VALUE                                         (1) //0:APLL1,1:APLL2
    
    #define ECO_APLL_K4_FOR_AOUT2_START                                         (15)
    #define ECO_APLL_K4_FOR_AOUT2_BIT_NUM                                       (1)
    #define ECO_APLL_K4_FOR_AOUT2_VALUE                                         (1) //0:APLL1,1:APLL2

    #define ECO_APLL_K6_FOR_MPHONE_START                                        (17)
    #define ECO_APLL_K6_FOR_MPHONE_BIT_NUM                                      (1)
    #define ECO_APLL_K6_FOR_MPHONE_VALUE                                        (1) //0:APLL1,1:APLL2

/////////////////////////////////////////////////////////////////////////////
//         RISC Regs   
/////////////////////////////////////////////////////////////////////////////
#define ECO_AUD_RISC_REG_BASE                                                   (0x5000)


#define ECO_AUD_SPLIN_CTRL_ADDR                                                 (ECO_AUD_RISC_REG_BASE + 0x20)
    #define ECO_AUD_LINE_EN_START                                               (0)
    #define ECO_AUD_LINE_EN_BIT_NUM                                             (1)

    #define ECO_AUD_SPLIN_CTRL_SAVE_DATA_MODE_START                             (1)
    #define EC0_AUD_SPLIN_CTRL_SAVE_DATA_MODE_BIT_NUM                           (1)    
    #define ECO_AUD_SPLIN_CTRL_SAVE_DATA_MODE_BIT16                             (0)
    #define ECO_AUD_SPLIN_CTRL_SAVE_DATA_MODE_BIT24                             (1)


#define ECO_AUD_AOUT1_CFG_ADDR                                                  (ECO_AUD_RISC_REG_BASE+ 0xc0)
    #define ECO_AUD_AOUT1_LRCK_RATIO_START                                      (0)
    #define ECO_AUD_AOUT1_LRCK_RATIO_BIT_NUM                                    (2)

    #define ECO_AUD_AOUT1_BCK_RATIO_START                                       (16)
    #define ECO_AUD_AOUT1_BCK_RATIO_BIT_NUM                                     (4)


#define ECO_AUD_INPUT_HW_CFG_ADDR                                               (ECO_AUD_RISC_REG_BASE+ 0xc4)
    #define ECO_AUD_LRCK_CYCLE_BIT_START                                        (16)
    #define ECO_AUD_LRCK_CYCLE_BIT_NUM                                          (2)

    #define ECO_AUD_MIN_SRC_SWITCH1_START                                        (19)
    #define ECO_AUD_MIN_SRC_SWITCH1_BIT_NUM                                      (1)   
    #define ECO_AUD_MIN_SRC_SWITCH1_MPHONE                                       (1)
    #define ECO_AUD_MIN_SRC_SWITCH1_AOUT1                                        (0)
    
    #define ECO_AUD_LIN_CLK_MODE_SEL_START                                      (20)
    #define ECO_AUD_LIN_CLK_MODE_SEL_BIT_NUM                                    (1)
    #define ECO_AUD_LIN_INTERNAL_MODE                                           (0)
    #define ECO_AUD_SPDIFIN_EXT_MODE                                            (1)


#define ECO_AUD_AOUT2_CFG_ADDR                                                  (ECO_AUD_RISC_REG_BASE+ 0xcc)
    #define ECO_AUD_MPHONE_LRCK_DIV_START                                       (4)
    #define ECO_AUD_MPHONE_LRCK_DIV_BIT_NUM                                     (2)
    #define ECO_AUD_MPHONE_LRCK_DIV_VALUE                                       (2)

    #define ECO_AUD_MPHONE_BCK_DIV_START                                     (0)
    #define ECO_AUD_MPHONE_BCK_DIV_BIT_NUM                                   (4)
    #define ECO_AUD_MPHONE_BCK_DIV_VALUE                                     (2)
    
    #define ECO_AUD_AOUT2_LRCK_RATIO_START                                      (17)
    #define ECO_AUD_AOUT2_LRCK_RATIO_BIT_NUM                                    (2)

    #define ECO_AUD_AOUT2_BCK_RATIO_START                                       (12)
    #define ECO_AUD_AOUT2_BCK_RATIO_BIT_NUM                                     (4)   

/////////////////////////////////////////////////////////////////////////////
//         RGBK2 Register
/////////////////////////////////////////////////////////////////////////////
#define ECO_AUD_RGBK2_REG_BASE                                                  (0xA8000)


#define ECO_AUD_REG_ARM_CTRL_MIN_SBLK                                           (ECO_AUD_RGBK2_REG_BASE + 0x40) 
    #define ECO_AUD_REG_ARM_CTRL_MIN_SBLK_ADDR_START                            (0)
    #define ECO_AUD_REG_ARM_CTRL_MIN_SBLK_BIT_NUM                               (16)

#define ECO_AUD_REG_RGBK2_REAR_PWM_CFG8                                         (ECO_AUD_RGBK2_REG_BASE + 0x48)

    #define ECO_AUD_REG_AFE_MIN_SEL_START                                           (0)
    #define ECO_AUD_REG_AFE_MIN_SEL_BIT_NUM                                         (1)

    #define ECO_AUD_REG_AFE_LIN_SEL_START                                           (3)
    #define ECO_AUD_REG_AFE_LIN_SEL_BIT_NUM                                         (1)
    
#define ECO_AUD_REG_RGBK2_CFG1                                                  (ECO_AUD_RGBK2_REG_BASE + 0x84)
    #define ECO_AUD_MIN_ARM_CTRL_START                                          (1)
    #define ECO_AUD_MIN_ARM_CTRL_BIT_NUM                                        (1)        
    #define ECO_AUD_MIN_ARM_CTRL_ENABLE                                         (1)

#define ECO_AUD_REG_RGBK2_CFG2                                                  (ECO_AUD_RGBK2_REG_BASE + 0x88)
    
#define ECO_AUD_REG_RGBK2_CFG3                                                  (ECO_AUD_RGBK2_REG_BASE + 0x8C)
    #define ECO_AUD_RGBK2_R_MPBUF3_SADR_START                                   (0)
    #define EC0_AUD_RGBK2_R_MPBUF3_SADR_BIT_NUM                                 (16)
        
    #define ECO_AUD_REG_MIC_IN_ENABLE_START                                     (16)        
    #define ECO_AUD_REG_MIC_IN_ENABLE_BIT_NUM                                   (1)
    
#define ECO_AUD_REG_RGBK2_CFG4                                                  (ECO_AUD_RGBK2_REG_BASE + 0x90)
    #define ECO_AUD_BCK_RATIO_START                                             (0)
    #define EC0_AUD_BCK_RATIO_BIT_NUM                                           (4)

    #define ECO_LIN_HW_SEL1_CLK_START                                           (4)
    #define ECO_LIN_HW_SEL1_CLK_BIT_NUM                                         (1)
    #define ECO_LIN_HW_SEL1_CLK_ENABLE                                          (1)
    #define ECO_LIN_HW_SEL1_CLK_DISABLE                                         (0)

    #define ECO_AUD_MPHONE_BANK_ADDR_START                                      (8)
    #define ECO_AUD_MPHONE_BANK_ADDR_BIT_NUM                                    (10)

    //with ECO_AUD_REG_RGBK2_CFG4[4]:00 aout1 timing,01:Mlin Timing,10:aout2 timing
#define ECO_AUD_REG_RGBK2_AFE_TOP_CFG0                                          (ECO_AUD_RGBK2_REG_BASE + 0xD0)
    #define ECO_AUD_MIN_SAVE_DATA_BIT_MODE_START                                (1)    
    #define ECO_AUD_MIN_SAVE_DATA_BIT_MODE_BIT_NUM                              (1)
    #define ECO_AUD_MIN_SAVE_DATA_BIT_MODE_24BIT                                (1)

    #define ECO_AUD_AFE1_SRC_CLK_SEL_START                                      (2)
    #define ECO_AUD_AFE1_SRC_CLK_SEL_NUM                                        (2)
    
    #define ECO_AUD_AFE2_SRC_CLK_SEL_START                                      (4)
    #define ECO_AUD_AFE2_SRC_CLK_SEL_NUM                                        (2)

    #define ECO_AUD_AFE_MIN_SEL_START                                           (6)
    #define ECO_AUD_AFE_MIN_SEL_BIT_NUM                                         (1)
    
    #define ECO_AUD_AFE_LIN_SEL_START                                           (7)
    #define ECO_AUD_AFE_LIN_SEL_BIT_NUM                                         (1)

    #define ECO_AUD_AOUT1_BYASS_MODE_SEL_START                                  (8)
    #define ECO_AUD_AOUT1_BYASS_MODE_SEL_BIT_NUM                                (1)
    #define ECO_AUD_AOUT1_BYASS_MODE_SEL_ADC                                    (1)

    #define ECO_AUD_AOUT2_BYASS_MODE_SEL_START                                    (9)
    #define ECO_AUD_AOUT2_BYASS_MODE_SEL_BIT_NUM                                (1)
    #define ECO_AUD_AOUT2_BYASS_MODE_SEL_ADC                                    (1)
    //with 50c4[19]
    #define ECO_AUD_MIN_SRC_SWITCH2_START                                       (13)
    #define ECO_AUD_MIN_SRC_SWITCH2_BIT_NUM                                     (1)
    #define ECO_AUD_MIN_SRC_SWITCH2_MPHONE                                      (0)
    #define ECO_AUD_MIN_SRC_SWITCH2_AOUT2                                       (1)    

    #define ECO_LIN_HW_SEL2_CLK_START                                           (14)
    #define ECO_LIN_HW_SEL2_CLK_BIT_NUM                                         (1)
    #define ECO_LIN_HW_SEL2_CLK_ENABLE                                          (1)
    #define ECO_LIN_HW_SEL2_CLK_DISABLE                                         (0)
    
    #define ECO_AUD_AFE1_DST_CLK_SEL_START                                      (16)
    #define ECO_AUD_AFE1_DST_CLK_SEL_NUM                                        (2)
   
    #define ECO_AUD_AFE2_DST_CLK_SEL_START                                      (18)
    #define ECO_AUD_AFE2_DST_CLK_SEL_NUM                                        (2)


#define ECO_AUD_REG_RGBK2_BYPS_VLUM_CFG0                                        (ECO_AUD_RGBK2_REG_BASE + 0xD4)
    #define ECO_AUD_ADC_BYPASS_VOL_GAIN_START                                   (0)
    #define ECO_AUD_ADC_BYPASS_VOL_GAIN_BIT_NUM                                 (24)

    #define ECO_AUD_ADC_BYPASS_VOL_CTRL_START                                   (24)
    #define ECO_AUD_ADC_BYPASS_VOL_CTRL_BIT_NUM                                 (1)
    #define ECO_AUD_ADC_BYPASS_VOL_CTRL_ENABLE                                  (1)
    #define ECO_AUD_ADC_BYPASS_VOL_CTRL_DISABLE                                 (0)

    #define ECO_AUD_ADC_BYPASS_VOL_FADEIN_CTRL_START                            (25)
    #define ECO_AUD_ADC_BYPASS_VOL_FADEIN_CTRL_BIT_NUM                          (1)
    #define ECO_AUD_ADC_BYPASS_VOL_FADEIN_CTRL_ENABLE                           (1)
    #define ECO_AUD_ADC_BYPASS_VOL_FADEIN_CTRL_DISABLE                          (0)
           
    #define ECO_AUD_ADC_BYPASS_VOL_FADEOUT_CTRL_START                           (26)
    #define ECO_AUD_ADC_BYPASS_VOL_FADEOUT_CTRL_BIT_NUM                         (1)
    #define ECO_AUD_ADC_BYPASS_VOL_FADEOUT_CTRL_ENABLE                          (1)
    #define ECO_AUD_ADC_BYPASS_VOL_FADEOUT_CTRL_DISABLE                         (0)

    #define ECO_AUD_ADC_BYPASS_VOL_MODE_SEL_START                               (27)
    #define ECO_AUD_ADC_BYPASS_VOL_MODE_SEL_BIT_NUM                             (1)
    #define ECO_AUD_ADC_BYPASS_VOL_MODE_SEL_LINER                               (1)
    #define ECO_AUD_ADC_BYPASS_VOL_MODE_SEL_NOLINER                             (0)

    #define ECO_AUD_SEL_AFE_START                                               (28)
    #define ECO_AUD_SEL_AFE_BIT_NUM                                             (1)   

    
#define ECO_AUD_REG_RGBK2_BYPS_VLUM_CFG1                                        (ECO_AUD_RGBK2_REG_BASE + 0xD8)
    #define ECO_AUD_LIN_AFE_MODE_START                                            (4)
    #define ECO_AUD_LIN_AFE_MODE_BIT_NUM                                              (1)
    #define ECO_AUD_LIN_INTENAL_ADC_MODE                                          (1)
    #define ECO_AUD_LIN_EXTENAL_ADC_MODE                                          (0)

    #define ECO_AUD_LIN_DATA_FORMAT_SEL_START                                      (7)
    #define ECO_AUD_LIN_DATA_FORMAT_SEL_BIT_NUM                                     (1)
    #define ECO_AUD_LIN_DATA_FORMAT_SEL_LEFT_ALIGNMENT                             (0)
    #define ECO_AUD_LIN_DATA_FORMAT_SEL_RIGHT_ALIGNMENT                             (1)

    #define ECO_AUD_MIN_INTERNAL_ADC_MODE_START                                   (8)
    #define ECO_AUD_MIN_INTERNAL_ADC_MODE_BIT_NUM                                 (1)
    #define ECO_AUD_MIN_INTERNAL_ADC_MODE_ENABLE                                  (0)
    
    #define ECO_AUD_MIN_WE_SEL_START                                              (9)
    #define ECO_AUD_MIN_WE_SEL_BIT_NUM                                            (1)
    #define ECO_AUD_MIN_WE_SEL_VALUE                                              (1)

#define ECO_AUD_SPLIN2_CTRL_ADDR 0x5268

/////////////////////////////////////////////////////////////////////////////
//        AFE1&2 Register
/////////////////////////////////////////////////////////////////////////////
#define ECO_AUD_AFE_REG_BASE                                                  (0xA8200)

#define ECO_AUD_REG_AFE1_CFG20                                                 (ECO_AUD_AFE_REG_BASE + 0xD4)
    #define ECO_AUD_REG_AFE1_CTRL_START                                        (0)
    #define ECO_AUD_REG_AFE1_CTRL_BIT_NUM                                      (1)
    #define ECO_AUD_REG_AFE1_CTRL_ENABLE                                       (1)
    
    #define ECO_AUD_REG_AFE1_SRC_CTRL_START                                    (6)
    #define ECO_AUD_REG_AFE1_SRC_CTRL_BIT_NUM                                  (1)
    #define ECO_AUD_REG_AFE1_SRC_CTRL_ENABLE                                   (1)

    
#define ECO_AUD_REG_AFE2_CFG20                                                 (ECO_AUD_AFE_REG_BASE + 0x154)
    #define ECO_AUD_REG_AFE2_CTRL_START                                        (0)
    #define ECO_AUD_REG_AFE2_CTRL_BIT_NUM                                      (1)
    #define ECO_AUD_REG_AFE2_CTRL_ENABLE                                       (1)

    #define ECO_AUD_REG_AFE2_SRC_CTRL_START                                    (6)
    #define ECO_AUD_REG_AFE2_SRC_CTRL_BIT_NUM                                  (1)
    #define ECO_AUD_REG_AFE2_SRC_CTRL_ENABLE                                   (1)

#endif

#endif
