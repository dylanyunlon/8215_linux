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
 * @file aud_reg_pwm.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_REG_PWM_H
#define _AUD_REG_PWM_H
     
#ifdef __cplusplus
    extern "C"
    {
#endif



#define AUD_REG_PWM_DAC_BASE        (0x600)
#define BIT_NUM_PWM_2CH_CTL 2

#define AUD_REG_PWM_CFG0        (AUD_REG_PWM_DAC_BASE + 0x28)
    #define BIT_STR_HB_CLK_294M_SEL     16 //(16 - 23)
    #define BIT_NUM_HB_CLK_294M_SEL     8  //pwm clock select
        #define PWM_294M    1
        #define PWM_270M    0

    #define BIT_STR_HB_RELATCH_SEL      0  //(0 - 7)   -> no need config,default 1:En
    #define BIT_NUM_HB_RELATCH_SEL      8  //clock relatch for pwm
        #define PWM_RELATCH_BYPASS  0
        #define PWM_RELATCH_ENABLE  1

#define AUD_REG_PWM_CFG1        (AUD_REG_PWM_DAC_BASE + 0x2C)
    #define BIT_STR_HB_PS_CLK_294M_SEL  16 //(16 - 23)
    #define BIT_NUM_HB_PS_CLK_294M_SEL  8  //Phase-Shift PWM Clock Select

#define AUD_REG_PWM_CFG2        (AUD_REG_PWM_DAC_BASE + 0x30)
    #define BIT_STR_HB_ENVO_CH3         29 //(29)
    #define BIT_NUM_HB_ENVO_CH3         1  //Activate Amplifier Mode of channel(X) amplifiter
        #define PWM_ENVO_OFF    0
        #define PWM_ENVO_ON     1

    #define BIT_STR_HB_ENVO_CH2         25 //(25)
    #define BIT_NUM_HB_ENVO_CH2         1  //Activate Amplifier Mode of channel(X) amplifiter

    #define BIT_STR_HB_ENVO_CH1         21 //(21)
    #define BIT_NUM_HB_ENVO_CH1         1  //Activate Amplifier Mode of channel(X) amplifiter

    #define BIT_STR_HB_ENVO_CH0         17 //(17)
    #define BIT_NUM_HB_ENVO_CH0         1  //Activate Amplifier Mode of channel(X) amplifiter

    #define BIT_STR_HB_DFC_CH           11 //(11 - 14)   ->default : 0
    #define BIT_NUM_HB_DFC_CH           4  //Enable damping factor control option for channel(X) amplifier

#define AUD_REG_PWM_CFG3        (AUD_REG_PWM_DAC_BASE + 0x34)
    #define BIT_STR_HB_ENPWRDET         21 //(21)        -> default : 0
    #define BIT_NUM_HB_ENPWRDET         1  //Power supply detecton for dc/ac off

#define AUD_REG_PWM_CFG4        (AUD_REG_PWM_DAC_BASE + 0x38)

#define AUD_REG_PWM_CFG5        (AUD_REG_PWM_DAC_BASE + 0x3C)
    #define BIT_STR_HB_REV0             16 //(16 - 31)   ->default : 0
    #define BIT_NUM_HB_REV0             16 //

    #define BIT_STR_GPIO_PWM_EN         8  //(8 - 15)
    #define BIT_NUM_GPIO_PWM_EN         8  //GPIO enable
        #define PWM_GPI_EN  0
        #define PWM_GPO_EN  1

#define AUD_REG_PWM_CFG6        (AUD_REG_PWM_DAC_BASE + 0x40)

#define AUD_REG_PWM_CFG7        (AUD_REG_PWM_DAC_BASE + 0x44)
    #define BIT_STR_GPIO_PWM_G          16 //(16 - 23)
    #define BIT_NUM_GPIO_PWM_G          8  //GPIO analog/gpio function control
        #define PWM_ANALOG_FUNCTON  0
        #define PWM_GPIO_FUNCTION   1

#define AUD_REG_PWM_CFG8        (AUD_REG_PWM_DAC_BASE + 0x48)
    #define BIT_STR_AUD_PWMDAC_REV0     16 //(16 - 31)    -> default : 0
    #define BIT_NUM_AUD_PWMDAC_REV0     16 //

    #define BIT_STR_AUD_PWMDAC_REV1     0  //(0 - 15)     -> default : 0
    #define BIT_NUM_AUD_PWMDAC_REV1     16 //

#define AUD_REG_PWM_CFG9        (AUD_REG_PWM_DAC_BASE + 0x4C)
    #define BIT_STR_ADAC_VCM_EN         7  //(7)
    #define BIT_NUM_ADAC_VCM_EN         1  //

#define AUD_REG_PWM_CFG10       (AUD_REG_PWM_DAC_BASE + 0x50)
    #define BIT_STR_HB_HIZ              24 //(24 - 31)   -> gpio: 1 , analog: 0
    #define BIT_NUM_HB_HIZ              8  //Enable high impedance mode

#define AUD_REG_PWM_CFG11       (AUD_REG_PWM_DAC_BASE + 0x54)


#ifdef __cplusplus
        }
#endif
                            
#endif // _AUD_REG_PWM_H