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


/**
 * @file aud_reg_ckgen.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_REG_CKGEN_H
#define _AUD_REG_CKGEN_H
     
#ifdef __cplusplus
    extern "C"
    {
#endif

#define AUD_REG_CKGEN_BASE          (0x0000)

#define AUD_REG_AP_RISC_CFG0        (AUD_REG_CKGEN_BASE + 0x0004)

#define AUD_REG_DVD_REG1            (AUD_REG_CKGEN_BASE + 0x0008)
    #define BIT_STR_AUD_DVD_SEL         8 //(8 - 9)
    #define BIT_NUM_AUD_DVD_SEL         2 //select dvd audio clock
        #define AUD_DVD_27M     0
        #define AUD_DVD_K7      1
        #define AUD_DVD_ACLK_IN 2 //(PAD)
        #define AUD_DVD_I2S_OUT2_MCLK   3

#define AUD_REG_AP_REG0             (AUD_REG_CKGEN_BASE + 0x000C)
    #define BIT_STR_DSP_AP_SEL          0 //(0 - 2)
    #define BIT_NUM_DSP_AP_SEL          3 //dsp clock select
        #define DSP_AP_CLK27M       0
        #define DSP_AP_SYSPLL_D3    1
        #define DSP_AP_ARMPLL2_CK   2
        #define DSP_AP_USBPLL_D2    3
        #define DSP_AP_CLK_APLL2    4
        #define DSP_AP_DMPLL_CK     5
        #define DSP_AP_SYSPLL_D2    6
        #define DSP_AP_SYSPLL_D4    7

    #define BIT_STR_CFG_REG_K10          16//(16 - 23)
    #define BIT_NUM_CFG_REG_K10          8 //audio k10 clock divider ratio


#define AUD_REG_AP_REG1             (AUD_REG_CKGEN_BASE + 0x0010)

#define AUD_REG_AP_REG2             (AUD_REG_CKGEN_BASE + 0x0014)
    #define BIT_STR_AUD_AP_SEL          28//(28 - 29)
    #define BIT_NUM_AUD_AP_SEL          2 //aud clock select
        #define AUD_AP_CLK27M       0
        #define AUD_AP_ACLK_K2      1
        #define AUD_AP_I2S_OUT0_MCLK_INT_IN 2 //MULTI FUNCTION
        #define AUD_AP_I2S_OUT1_MCLK_INT_IN 3 //MULTI FUNCTION

#define AUD_REG_AP_REG3             (AUD_REG_CKGEN_BASE + 0x0018)
    #define BIT_STR_BT_PCM_SEL          18//(18 - 19)
    #define BIT_NUM_BT_PCM_SEL          2 //bt pcm clock select
        #define BT_PCM_CLK27M       0
        #define BT_PCM_ACK_K9       1
        #define BT_PCM_BT_MIC_IN    2
        #define BT_PCM_MPHON_IN     3

    #define BIT_STR_MPHONE_AP_SEL       6 //(6 - 7)
    #define BIT_NUM_MPHONE_AP_SEL       2 //mphone self mclk select
        #define MPHONE_CLK27M       0
        #define MPHONE_ACK_K6       1
        #define MPHONE_MPHONE_IN    2
        #define MPHONE_SPMCLK_IN    3

    #define BIT_STR_AUD2_AP_SEL         0 //(0 - 1)
    #define BIT_NUM_AUD2_AP_SEL         2 //aud2 clock select
        #define AUD2_AP_CLK27M      0
        #define AUD2_AP_ACLK_K4     1
        #define AUD2_AP_I2S_OUT0_MCLK_INT_IN 2 //MULTI FUNCTION
        #define AUD2_AP_I2S_OUT1_MCLK_INT_IN 3 //MULTI FUNCTION

#define AUD_REG_AP_REG4             (AUD_REG_CKGEN_BASE + 0x001C)
    #define BIT_STR_CFG_REG_K4          24//(24 - 31)
    #define BIT_NUM_CFG_REG_K4          8 //audio k4 clock divider ratio

    #define BIT_STR_CFG_REG_K3          16//(16 - 23)
    #define BIT_NUM_CFG_REG_K3          8 //audio k3 clock divider ratio

    #define BIT_STR_CFG_REG_K2          8 //(8 - 15)
    #define BIT_NUM_CFG_REG_K2          8 //audio k2 clock divider ratio

    #define BIT_STR_CFG_REG_K1          0 //(0 - 7)
    #define BIT_NUM_CFG_REG_K1          8 //audio k1 clock divider ratio

#define AUD_REG_AP_REG5             (AUD_REG_CKGEN_BASE + 0x0020)
    #define BIT_STR_CFG_REG_K8          24//(24 - 31)
    #define BIT_NUM_CFG_REG_K8          8 //audio k8 clock divider ratio

    #define BIT_STR_CFG_REG_K7          16//(16 - 23)
    #define BIT_NUM_CFG_REG_K7          8 //audio k7 clock divider ratio

    #define BIT_STR_CFG_REG_K6          8 //(8 - 15)
    #define BIT_NUM_CFG_REG_K6          8 //audio k6 clock divider ratio

    #define BIT_STR_CFG_REG_K5          0 //(0 - 7)
    #define BIT_NUM_CFG_REG_K5          8 //audio k5 clock divider ratio


#define AUD_REG_AP_REG6             (AUD_REG_CKGEN_BASE + 0x0024)
    #define BIT_STR_CFG_REG_K12         24//(24 - 31)
    #define BIT_NUM_CFG_REG_K12         8 //audio k12 clock divider ratio

    #define BIT_STR_CFG_REG_K11         16//(16 - 23)
    #define BIT_NUM_CFG_REG_K11         8 //audio k11 clock divider ratio
    
    #define BIT_STR_CFG_REG_A3          14//(14)
    #define BIT_NUM_CFG_REG_A3          1 //audio A3 clock divider ratio

    #define BIT_STR_CFG_REG_A2          13//(13)
    #define BIT_NUM_CFG_REG_A2          1 //audio A2 clock divider ratio

    #define BIT_STR_CFG_REG_A1          12//(12)
    #define BIT_NUM_CFG_REG_A1          1 //audio A1 clock divider ratio

    #define BIT_STR_CFG_REG_K9          0 //(0 - 11)
    #define BIT_NUM_CFG_REG_K9          12//audio K9 clock divider ratio


#define AUD_REG_AP_REG7             (AUD_REG_CKGEN_BASE + 0x0028)
    #define BIT_STR_SEL_APLL_K8         30 //(30 - 31)
    #define BIT_NUM_SEL_APLL_K8         2 //select apll1 or apll2 as k8 divider source
        #define KX_SEL_APLL1    0
        #define KX_SEL_APLL2    1
        #define KX_SEL_HADDS    2
        #define KX_SEL_SYSPLL_D2 3

    #define BIT_STR_SEL_APLL_K7         28 //(28 - 29)
    #define BIT_NUM_SEL_APLL_K7         2 //select apll1 or apll2 as k7 divider source
    
    #define BIT_STR_SEL_APLL_K6         26 //(26 - 27)
    #define BIT_NUM_SEL_APLL_K6         2 //select apll1 or apll2 as k6 divider source

    #define BIT_STR_SEL_APLL_K5         24 //(24 - 25)
    #define BIT_NUM_SEL_APLL_K5         2 //select apll1 or apll2 as k5 divider source

    #define BIT_STR_SEL_APLL_K4         22 //(22 - 23)
    #define BIT_NUM_SEL_APLL_K4         2 //select apll1 or apll2 as k4 divider source

    #define BIT_STR_SEL_APLL_K3         20 //(20 - 21)
    #define BIT_NUM_SEL_APLL_K3         2 //select apll1 or apll2 as k3 divider source

    #define BIT_STR_SEL_APLL_K2         18 //(18 - 19)
    #define BIT_NUM_SEL_APLL_K2         2 //select apll1 or apll2 as k2 divider source

    #define BIT_STR_SEL_APLL_K1         16 //(16 - 17)
    #define BIT_NUM_SEL_APLL_K1         2 //select apll1 or apll2 as k1 divider source

    #define BIT_STR_CFG_REG_K14         8 //(8 - 15)
    #define BIT_NUM_CFG_REG_K14         8 //audio k14 clock divider ratio

    #define BIT_STR_CFG_REG_K13         0 //(0 - 7)
    #define BIT_NUM_CFG_REG_K13         8 //audio k13 clock divider ratio

#define AUD_REG_AP_REG8             (AUD_REG_CKGEN_BASE + 0x002C)
    #define BIT_STR_MLIN_SEL            29 //(29 - 30)
    #define BIT_NUM_MLIN_SEL            2 //mlin clock select
        #define MLIN_CLK27M         0
        #define MLIN_ACK_K3         1
        #define MLIN_SPMCK2_IN      2
        #define MLIN_SPMCK_IN       3

    #define BIT_STR_MPH_SEL             27 //(27 - 28)
    #define BIT_NUM_MPH_SEL             2 //mphone clock select  

    #define BIT_STR_MLIN2_SEL           25 //(25 - 26)
    #define BIT_NUM_MLIN2_SEL           2 //mlin2 clock select
        #define MLIN2_CLK27M        0
        #define MLIN2_ACK_K12       1
        #define MLIN2_SPMCK_IN      2
        #define MLIN2_SPMCK2_IN     3

    #define BIT_STR_PWM_SEL             23 //(23 - 24)
    #define BIT_NUM_PWM_SEL             2 //pwm clock select

    #define BIT_STR_ADC_SEL             21 //(21 - 22)
    #define BIT_NUM_ADC_SEL             2 //adc clock select
        #define ADC_CLK27M          0
        #define ADC_ACK_K10         1
        #define ADC_ACK_K5          2
        #define ADC_SPMCK_IN        3

    #define BIT_STR_SEL_APLL_A3         16//(16 - 17)
    #define BIT_NUM_SEL_APLL_A3         2 //select apll1 or apll2 as A3 divider source

    #define BIT_STR_SEL_APLL_A2         14//(14 - 15)
    #define BIT_NUM_SEL_APLL_A2         2 //select apll1 or apll2 as A2 divider source

    #define BIT_STR_SEL_APLL_A1         12//(12 - 13)
    #define BIT_NUM_SEL_APLL_A1         2 //select apll1 or apll2 as A1 divider source

    #define BIT_STR_SEL_APLL_K14        10//(10)
    #define BIT_NUM_SEL_APLL_K14        1 //select apll1 or apll2 as k14 divider source
        #define K4_SEL_26M_APLL     0
        #define k4_SEL_SPMCK_IN     1

    #define BIT_STR_SEL_APLL_K13        8  //(8 - 9)
    #define BIT_NUM_SEL_APLL_K13        2 //select apll1 or apll2 as k13 divider source

    #define BIT_STR_SEL_APLL_K12        6  //(6 - 7)
    #define BIT_NUM_SEL_APLL_K12        2 //select apll1 or apll2 as k12 divider source

    #define BIT_STR_SEL_APLL_K11        4  //(4 - 5)
    #define BIT_NUM_SEL_APLL_K11        2 //select apll1 or apll2 as k11 divider source

    #define BIT_STR_SEL_APLL_K10        2  //(2 - 3)
    #define BIT_NUM_SEL_APLL_K10        2 //select apll1 or apll2 as k10 divider source
        #define K10_SEL_26M_APLL    0
        #define k10_SEL_APLL2       1

    #define BIT_STR_SEL_APLL_K9         0  //(0 - 1)
    #define BIT_NUM_SEL_APLL_K9         2 //select apll1 or apll2 as k9 divider source

#define AUD_REG_AP_REG9             (AUD_REG_CKGEN_BASE + 0x0030)
    #define BIT_STR_AP_ASRC_CLI_SEL     28//(28)
    #define BIT_NUM_AP_ASRC_CLI_SEL     1 //ap asrc clock select
        #define AP_ASRC_CLI_ACLK_A2  0
        #define AP_ASRC_CLI_MPHON_IN 1

    #define BIT_STR_GPS_ASRC_CLI_SEL    27//(27)
    #define BIT_NUM_GPS_ASRC_CLI_SEL    1 //gps asrc clock select
        #define AP_ASRC_CLI_ACLK_A1  0
        #define AP_ASRC_CLI_MPHON_IN 1


#define AUD_REG_PAD_MUX0            (AUD_REG_CKGEN_BASE + 0x0054)
    #define BIT_STR_I2S_MIC_IN_SEL      0 //(0 - 2)
    #define BIT_NUM_I2S_MIC_IN_SEL      3 //i2s mic in pad select

#define AUD_REG_PAD_MUX1            (AUD_REG_CKGEN_BASE + 0x0058)
    #define BIT_STR_SGM_MIC_IN_SEL      0 //(0 - 1)
    #define BIT_NUM_SGM_MIC_IN_SEL      2 //sgm mic pad select

    #define BIT_STR_PCM_SEL             24//(24)
    #define BIT_NUM_PCM_SEL             1 //bt pcm pad select
    
#define AUD_REG_PAD_MUX2            (AUD_REG_CKGEN_BASE + 0x005C)
    #define BIT_STR_AMUTE_R_SEL         31//(31)
    #define BIT_NUM_AMUTE_R_SEL         1 //amute rear pad select

    #define BIT_STR_SPDIF_SEL           30//(30)
    #define BIT_NUM_SPDIF_SEL           1 //spdif pad select

#define AUD_REG_PAD_MUX3            (AUD_REG_CKGEN_BASE + 0x0060)
    #define BIT_STR_I2S_OUT0_SEL        8 //(8 - 9)
    #define BIT_NUM_I2S_OUT0_SEL        2 //i2s out pad select

    #define BIT_STR_AMUTE_F_SEL         0 //(0)
    #define BIT_NUM_AMUTE_F_SEL         1 //amute front pad select

#define AUD_REG_PAD_MUX4            (AUD_REG_CKGEN_BASE + 0x0064)

#define AUD_REG_PAD_MUX6            (AUD_REG_CKGEN_BASE + 0x006C)
    #define BIT_STR_I2S_OUT1_SEL        3 //(3 - 4)
    #define BIT_NUM_I2S_OUT1_SEL        2 //i2s out1 pad select

#define AUD_REG_PAD_MUX7            (AUD_REG_CKGEN_BASE + 0x0070)
    #define BIT_STR_I2S_LINE1_IN_SEL    20//(20 - 22)
    #define BIT_NUM_I2S_LINE1_IN_SEL    3 //i2s line1 pad select

    #define BIT_STR_I2S_LINE0_IN_SEL    16//(16 - 18)
    #define BIT_NUM_I2S_LINE0_IN_SEL    3 //i2s line0 pad select

#define AUD_REG_PLLGP_APLL_CFG8     (AUD_REG_CKGEN_BASE + 0x05A0)
    #define BIT_STR_PCW_NCPO_CHG        5
    #define BIT_NUM_PCW_NCPO_CHG        1

#define AUD_REG_PLLGP_APLL_CFG9     (AUD_REG_CKGEN_BASE + 0x05A4)
    #define BIT_STR_PCW_NCPO            1
    #define BIT_NUM_PCW_NCPO            31



#ifdef __cplusplus
    }
#endif
                        
#endif // _AUD_REG_CKGEN_H
