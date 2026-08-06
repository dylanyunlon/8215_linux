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
 * @file aud_reg_adc.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_REG_ADC_H
#define _AUD_REG_ADC_H
     
#ifdef __cplusplus
    extern "C"
    {
#endif

#define AUD_REG_ADC_BASE        (0x600)

#define AUD_REG_ADC_CFG0        (AUD_REG_ADC_BASE + 0x60)
    #define BIT_STR_PREAMPL1ON          31 //(31)
    #define BIT_NUM_PREAMPL1ON          1  //front audio left preanplifiter enable
      #define PREAMP_RESET      0
      #define PREAMP_NORMAL     1

    #define BIT_STR_PREAMPL2ON          30 //(30)
    #define BIT_NUM_PREAMPL2ON          1  //rear audio left preamplifiter enable

    #define BIT_STR_PREAMPLINPUTSEL1    27 //(27 - 29)
    #define BIT_NUM_PREAMPLINPUTSEL1    3  //front audio left preamplifiter intput selection
        #define PREAMP_AIN1     1

    #define BIT_STR_PREAMPLINPUTSEL2    24 //(24 - 26) 
    #define BIT_NUM_PREAMPLINPUTSEL2    3  //rear audio left preamplifiter input selection

    #define BIT_STR_PREAMPR1ON          23 //(23)
    #define BIT_NUM_PREAMPR1ON          1  //front audio right preanplifiter enable

    #define BIT_STR_PREAMPR2ON          22 //(22) 
    #define BIT_NUM_PREAMPR2ON          1  //rear audio right preamplifiter enable

    #define BIT_STR_PREAMPRINPUTSEL1    19 //(19 - 21)
    #define BIT_NUM_PREAMPRINPUTSEL1    3  //front audio right preamplifiter intput selection

    #define BIT_STR_PREAMPRINPUTSEL2    16 //(16 - 18)
    #define BIT_NUM_PREAMPRINPUTSEL2    3  //rear audio left preamplifiter input selection

#define AUD_REG_ADC_CFG1        (AUD_REG_ADC_BASE + 0x64)
    #define BIT_STR_ADCLPWRUP           15 //(15)
    #define BIT_NUM_ADCLPWRUP           1  //front audio left adc power up
        #define ADC_PWD     0
        #define ADC_PWP     1

    #define BIT_STR_ADCLPWRUP2          14 //(14)
    #define BIT_NUM_ADCLPWRUP2          1  //rear audio left adc power up

    #define BIT_STR_ADCRPWRUP           13 //(13)
    #define BIT_NUM_ADCRPWRUP           1  //front audio right adc power up

    #define BIT_STR_ADCRPWRUP2          12 //(12) 
    #define BIT_NUM_ADCRPWRUP2          1  //rear audio right adc power up

#define AUD_REG_ADC_CFG2        (AUD_REG_ADC_BASE + 0x68)

#define AUD_REG_ADC_CFG3        (AUD_REG_ADC_BASE + 0x6C)
    #define BIT_STR_ADCLINPUTSEL1       27 //(27 - 29)
    #define BIT_NUM_ADCLINPUTSEL1       3  //front audio left adc input selction
        #define ADC_IN_PREAMP   4
        #define ADC_IN_LSBUF    5

    #define BIT_STR_ADCLINPUTSEL2       24 //(24 - 26) 
    #define BIT_NUM_ADCLINPUTSEL2       3  //rear audio left adc input selction

    #define BIT_STR_ADCRINPUTSEL1       16 //(16 - 18) 
    #define BIT_NUM_ADCRINPUTSEL1       3  //front audio right adc input selction

    #define BIT_STR_ADCRINPUTSEL2       8  //(8 - 10)
    #define BIT_NUM_ADCRINPUTSEL2       3  //rear audio right adc input selction

#define AUD_REG_ADC_CFG4        (AUD_REG_ADC_BASE + 0x70)

#define AUD_REG_ADC_CFG5        (AUD_REG_ADC_BASE + 0x74)
    #define BIT_STR_ADCLSBUFLCLAMP1_EN  31 //(31)
    #define BIT_NUM_ADCLSBUFLCLAMP1_EN  1  //front spare control bits for AVDD25 voltage domain

    #define BIT_STR_ADCLSBUFRCLAMP1_EN  30 //(30)
    #define BIT_NUM_ADCLSBUFRCLAMP1_EN  1  //

    #define BIT_STR_ADCLSBUFLCLAMP2_EN  23 //(23)
    #define BIT_NUM_ADCLSBUFLCLAMP2_EN  1  //

    #define BIT_STR_ADCLSBUFRCLAMP2_EN  22 //(22)
    #define BIT_NUM_ADCLSBUFRCLAMP2_EN  1  //

#define AUD_REG_ADC_CFG6        (AUD_REG_ADC_BASE + 0x78)

#define AUD_REG_ADC_CFG7        (AUD_REG_ADC_BASE + 0x7C)
    #define BIT_STR_PWDB_MBIAS1         3  //(3)
    #define BIT_NUM_PWDB_MBIAS1         1  //front mic bias power down
        #define MBIAS_PWD   0
        #define MBIAS_PWP   1

    #define BIT_STR_PWDB_MBIAS2         2  //(2)
    #define BIT_NUM_PWDB_MBIAS2         1  //rear mic bias power down


#define AUD_REG_ADC_CFG8        (AUD_REG_ADC_BASE + 0x80)

#define AUD_REG_ADC_CFG9        (AUD_REG_ADC_BASE + 0x84)

#define AUD_REG_ADC_CFG10       (AUD_REG_ADC_BASE + 0x88)
    #define BIT_STR_LSBUFLGAIN1         18 //(18 - 23)
    #define BIT_NUM_LSBUFLGAIN1         6  //front audio left buffer(level shifted)gain setting
        
    #define BIT_STR_LSBUFLGAIN2         12 //(12 - 17)
    #define BIT_NUM_LSBUFLGAIN2         6  //rear audio left buffer(level shifted)gain setting

    #define BIT_STR_LSBUFRGAIN1         6  //(6 - 11)
    #define BIT_NUM_LSBUFRGAIN1         6  //front audio right buffer(level shifted)gain setting

    #define BIT_STR_LSBUFRGAIN2         0  //(0 - 5)
    #define BIT_NUM_LSBUFRGAIN2         6  //rear audio right buffer(level shifted)gain setting

#define AUD_REG_ADC_CFG11       (AUD_REG_ADC_BASE + 0x8C)
    #define BIT_STR_LSBUFLPWRUP1        31 //(31)
    #define BIT_NUM_LSBUFLPWRUP1        1  //front audio left line in buffer power up (level shifted)
        #define LSBUF_PWD   0
        #define LSBUF_PWP   1

    #define BIT_STR_LSBUFLPWRUP2        30 //(30)
    #define BIT_NUM_LSBUFLPWRUP2        1  //rear audio left line in buffer power up (level shifted)

    #define BIT_STR_LSBUFRPWRUP1        29 //(29)
    #define BIT_NUM_LSBUFRPWRUP1        1  //front audio right line in buffer power up (level shifted)

    #define BIT_STR_LSBUFRPWRUP2        28 //(28) 
    #define BIT_NUM_LSBUFRPWRUP2        1  //rear audio right line in buffer power up (level shifted)
    
    #define BIT_STR_LSBUFLINPUTSEL1     23 //(23 - 25)
    #define BIT_NUM_LSBUFLINPUTSEL1     3  //front audio left level shifted buffer input selection, positive/negative pins

    #define BIT_STR_LSBUFLINPUTSEL2     20 //(20 - 22)
    #define BIT_NUM_LSBUFLINPUTSEL2     3  //rear audio left level shifted buffer input selection, positive/negative pins

    #define BIT_STR_LSBUFRINPUTSEL1     17 //(17 - 19) 
    #define BIT_NUM_LSBUFRINPUTSEL1     3  //front audio right level shifted buffer input selection, positive/negative pins

    #define BIT_STR_LSBUFRINPUTSEL2     14 //(14 - 16)
    #define BIT_NUM_LSBUFRINPUTSEL2     3  //rear audio right level shifted buffer input selection, positive/negative pins

#define AUD_REG_ADC_CFG12       (AUD_REG_ADC_BASE + 0x90)
    #define BIT_STR_RCH_GPIO_PIN_CFG    19 //(19 - 23)
    #define BIT_NUM_RCH_GPIO_PIN_CFG    5  //audio rch pin gpio enable control
        #define PIN_AIN0    0
        #define PIN_AIN1    1
        #define PIN_AIN2    2
        #define PIN_AIN3    3
        #define PIN_AIN4    4
        //1 : enable , 0 : disable

    #define BIT_STR_RCH_GPIO_PIN_CTL    11 //(11 - 15)
    #define BIT_NUM_RCH_GPIO_PIN_CTL    5  //audio rch input gpio control
        #define DIGITAL_INPUT   0
        #define DIGITAL_OUTPUT  1

#define AUD_REG_ADC_CFG13       (AUD_REG_ADC_BASE + 0x94)

#define AUD_REG_ADC_CFG14       (AUD_REG_ADC_BASE + 0x98)
    #define BIT_STR_LCH_GPIO_PIN_CFG    19 //(19 - 23)
    #define BIT_NUM_LCH_GPIO_PIN_CFG    5  //audio lch pin gpio enable control
        //1 : enable , 0 : disable

    #define BIT_STR_LCH_GPIO_PIN_CTL    11 //(11 - 15)
    #define BIT_NUM_LCH_GPIO_PIN_CTL    5  //audio rch input gpio control


#define AUD_REG_ADC_CFG15       (AUD_REG_ADC_BASE + 0x9C)

#define AUD_REG_ADC_CFG16       (AUD_REG_ADC_BASE + 0xA0)
    #define BIT_STR_GLB_PWD             17 //(17)
    #define BIT_NUM_GLB_PWD             1  //global bias power down reigster
        #define GLB_PWD     1
        #define GLB_PWP     0

    #define BIT_STR_RESET_CLK           9  //(9)
    #define BIT_NUM_RESET_CLK           1  //reset clk to audac
        #define CLK_RESET_EN    0
        #define CLK_NO_RESET    1
    
    #define BIT_STR_SEL_CLK_FREQ        8  //(8)
    #define BIT_NUM_SEL_CLK_FREQ        1  //clock freq selection
        #define CLK_13M     0
        #define CLK_26M     1

#define AUD_REG_ADC_DEBUG       (AUD_REG_ADC_BASE + 0xA4)



#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_REG_ADC_H
