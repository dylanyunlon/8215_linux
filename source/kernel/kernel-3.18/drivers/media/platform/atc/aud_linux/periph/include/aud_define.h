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

#ifndef _AUD_DEFINE_H_
#define _AUD_DEFINE_H_

#include <media/atc/aud_output.h>
#include <media/atc/drv_aud.h>



typedef struct 
{
    u32 u4Adr;
    u32 u4BitStart;
    u32 u4BitNum;
    s8   *pszRegName;
}AUD_IO_REG_CTL, *PAUD_IO_REG_CTL;


/**************************************************************
aout module related
**************************************************************/
typedef enum 
{
    AOUT_FS,
    AOUT_RS,
    AOUT_PATH_MAX,
}AUD_OUT_PATH_T;

typedef enum
{
    AUDID_AOUT1,
    AUDID_AOUT2,
    AUDID_AOUT_MAX,
}AUD_AOUT_DEVID;


typedef enum
{
    ASDATA_FLFR,   //front seat left channel, right channel
    ASDATA_RLRR,   //rear seat left channel, right channel
    ASDATA_CLFE,   //Center, LFE channel
    ASDATA_CH7_8,  //downmix channel 7 & 8
    ASDATA_CH9_10, //downmix channel 9 & 10
    ASDATA_CH11_12,
    ASDATA_SRC_MAX,
}ASDATA_PIN_SRC;

typedef enum
{
    ASDATA_PIN0,
    ASDATA_PIN1,
    ASDATA_PIN2,
    ASDATA_PIN3,
    ASDATA_PIN_MAX,
}ASDATA_PIN;

typedef enum
{
    SRC_AOUT1,
    SRC_AOUT2,
    SRC_DVD,
    SRC_MAX
}AUD_AOUT_SRC;


/**************************************************************
line in module related
**************************************************************/
typedef enum
{
    AUDID_LIN1,
    AUDID_LIN2,
    AUDID_LIN_MAX
}AUD_LIN_DEVID;

typedef enum
{
    INT_LINEIN,
    EXT_LINEIN,
    LIN_SRC_MAX,
}AUD_LIN_SRC;


typedef enum
{
    DRAM_DATA,
    DRAM_BYPASS,
    OUT_MODE_MAX,
}AUD_LIN_OUT_MODE;

typedef enum
{
    LIN_16,
    LIN_24,
}AUD_LIN_OUT_BITNUM;

typedef enum
{
    LALIGN_16BIT,  //16bits left alignment
    RALIGN_16BIT,  //16bits right alignment
}AUD_LIN_16BIT_FMT;

typedef enum
{
    LIN_CLK_MLIN,
    LIN_CLK_AOUT1,
    LIN_CLK_AOUT2,
    LIN_CLK_MAX,
}AUD_LIN_CLK_SRC;

typedef enum
{
    LIN_INT_OFF,
    LIN_INT_PERIOD_64DW,
    LIN_INT_PERIOD_128DW,
    LIN_INT_PERIOD_256DW,
}AUD_LIN_INT_PERIOD;

typedef enum
{
    BYPS_DST_AOUT1,
    BYPS_DST_AOUT2,
}AUD_BYPS_DST;

typedef enum
{
    BYPS_GAIN_SECTION_LINER,
    BYPS_GAIN_LINER,
}AUD_BYPS_GAIN_MODE;

typedef enum
{
    LIN_PIN_GROUP0, //ain0_l, ain0_r
    LIN_PIN_GROUP1, //ain1_l, ain1_r
    LIN_PIN_GROUP2, //ain2_l, ain2_r
    LIN_PIN_GROUP3, //ain3_l, ain3_r
    LIN_PIN_GROUP4, //ain4_l, ain4_r
    LIN_PIN_GROUP_MAX,
}AUD_LIN_PIN_IDX;


/**************************************************************
multi line in module related
**************************************************************/
typedef enum
{
    MLIN_SRC_AOUT,
    MLIN_SRC_LINE1,
    MLIN_SRC_HDMI_RX,
    MLIN_SRC_MAX,
}AUD_MLIN_SRC;

typedef enum
{
    MLIN_INTPERID_32DW,
    MLIN_INTPERID_64DW,
    MLIN_INTPERID_128DW,
    MLIN_INTPERID_256DW,
    MLIN_INTPERID_MAX,
}AUD_MLIN_INT_PERIOD;

typedef enum {
    MLIN_CHNUM_2,
    MLIN_CHNUM_4,
    MLIN_CHNUM_6,
    MLIN_CHNUM_8,
    MLIN_CHNUM_MAX,
} AUD_MLIN_CH_NUM_E;


/**************************************************************
multi line in module related
**************************************************************/
typedef enum
{
    INT_MICIN,
    EXT_MICIN,
    MIC_SRC_MAX,
}AUD_MIC_SRC;

typedef enum
{
    MIC_CLK_MPH,
    MIC_CLK_AOUT1,
    MIC_CLK_AOUT2,
    MIC_CLK_MAX,
}AUD_MIC_CLK_SRC;


/**************************************************************
pcm module related
**************************************************************/
typedef enum
{
    PCM_MSB_FIRST,
    PCM_LSB_FIRST,
}AUD_PCM_DATA_ORDER;

typedef enum
{
    PCM_BITS_16,
    PCM_BITS_15,
    PCM_BITS_14,
    PCM_BITS_13,
}AUD_PCM_BIT_NUM;

typedef enum
{
    PCM_LAW_8BIT,
    PCM_LINEAR_16BIT,
}AUD_PCM_BIT_MODE;

typedef enum
{
    PCM_CLK_LENGTH1,
    PCM_CLK_LENGTH2,
    PCM_CLK_LENGTH3,
    PCM_CLK_LENGTH4,
}AUD_PCM_SYNC_LENGTH;

typedef enum
{
    PCM_SHORT_MODE,
    PCM_LONG_MODE,
}AUD_PCM_SYNC_MODE;

typedef enum
{
    PCM_CLK_CYCLE_32,
    PCM_CLK_CYCLE_64,
}AUD_PCM_SYNC_CYCLE;

typedef enum
{
    PCM_MASTER,
    PCM_SLAVE,
}AUD_PCM_MODE;

typedef enum
{
    PCM,
    PCM_TX,
    PCM_RX,
}AUD_PCM_DEV_ID;





typedef enum
{
    PCM_NORMAL_MODE,
    PCM_LOOP_MODE, //internal , only for test
}AUD_PCM_HW_MODE;


/**************************************************************
adc module related
**************************************************************/
typedef enum {
    AUD_ADC1,
    AUD_ADC2,
    AUD_ADC_NON,
    AUD_ADC_MAX,
}AUD_ADC_ID;

typedef enum {
    AUD_IO_BUSY,
    AUD_IO_FREE,
}AUD_IO_STATUS;

typedef enum{
    AFE_CLK_AOUT1,
    AFE_CLK_AOUT2,
    AFE_CLK_MLIN,
    AFE_CLK_MPH,
    AFE_CLK_MLIN2
}AUD_AFE_CLK_SEL_E;

typedef enum
{
    ADC_SRC_MICIN,
    ADC_SRC_LIN1,
    ADC_SRC_LIN2,
    ADC_SRC_LIN3,
    ADC_SRC_LIN4,
    ADC_SRC_LIN5,
    ADC_SRC_NON,
    ADC_SRC_MAX,
}AUD_ADC_INPUT_SRC;

typedef enum
{
    LSBUFGAIN_3DB,
    LSBUFGAIN_0DB,
    LSBUFGAIN_MINUS_3DB,
    LSBUFGAIN_MINUS_6DB,
    LSBUFGAIN_MINUS_9DB,
    LSBUFGAIN_MINUS_12DB,
    LSBUFGAIN_MINUS_15DB,
    LSBUFGAIN_MINUS_18DB,
    LSBUFGAIN_MAX,
}AUD_LSBUF_GAIN;

/**************************************************************
dac module related
**************************************************************/
typedef enum
{
    PWM_SET0,
    PWM_SET1,
    PWM_SET2,
    PWM_SET3,
    PWM_SET_MAX,
}AUD_PWM_DAC_ID;


/**************************************************************
ckgen module related
**************************************************************/
typedef enum
{
    CKGEN_APPL1,
    CKGEN_APLL2,
}AUD_CKGEN_APLL;


#endif // #ifndef _AUD_DEFINE_H_
