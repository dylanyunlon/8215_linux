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
 * @file aud_reg_top_misc.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_REG_TOP_MISC_H
#define _AUD_REG_TOP_MISC_H
     
#ifdef __cplusplus
    extern "C"
    {
#endif



#define AUD_REG_TOP_MISC_BASE           (0x0000)

//audio peripher
#define AUD_REG_CLKGATE_CFG3            (AUD_REG_TOP_MISC_BASE + 0xA8)
    #define BIT_STR_DSPA_CLK_PD         0
    #define BIT_NUM_DSPA_CLK_PD         1

    #define BIT_STR_DSPB_CLK_PD         1
    #define BIT_NUM_DSPB_CLK_PD         1

    #define BIT_STR_FS_APLL_EN          2  //for pwm dac
    #define BIT_NUM_FS_APLL_EN          1

    #define BIT_STR_RS_APLL_EN          3  //for pwm dac
    #define BIT_NUM_RS_APLL_EN          1

    #define BIT_STR_RISCA_BCK_PD        4
    #define BIT_NUM_RISCA_BCK_PD        1

    #define BIT_STR_RISCB_BCK_PD        5
    #define BIT_NUM_RISCB_BCK_PD        1

    #define BIT_STR_DRAMA_CLK_PD        6
    #define BIT_NUM_DRAMA_CLK_PD        1

    #define BIT_STR_DRAMB_CLK_PD        7
    #define BIT_NUM_DRAMB_CLK_PD        1

    #define BIT_STR_MP_MLIN_MCLK_PD     8
    #define BIT_NUM_MP_MLIN_MCLK_PD     1

    #define BIT_STR_MLIN2_MCLK_PD       9
    #define BIT_NUM_MLIN2_MCLK_PD       1

    #define BIT_STR_AUD_IEC_CLK_PD      10
    #define BIT_NUM_AUD_IEC_CLK_PD      1

    #define BIT_STR_AUD2_CLK_PD         11
    #define BIT_NUM_AUD2_CLK_PD         1

    #define BIT_STR_APLL_CLK_AP_ASRC_PD 12
    #define BIT_NUM_APLL_CLK_AP_ASRC_PD 1
    
    #define BIT_STR_APLL_CLK_GPS_ASRC_PD 13
    #define BIT_NUM_APLL_CLK_GPS_ASRC_PD 1
    
    #define BIT_STR_AFE_26M_CLK_PD      14
    #define BIT_NUM_AFE_26M_CLK_PD      1

    #define ADSPA_CLK_PD ((1 << BIT_STR_DSPA_CLK_PD) | (1 << BIT_STR_RISCA_BCK_PD) | (1 << BIT_STR_DRAMA_CLK_PD))
    #define ADSPB_CLK_PD ((1 << BIT_STR_DSPB_CLK_PD) | (1 << BIT_STR_RISCB_BCK_PD) | (1 << BIT_STR_DRAMB_CLK_PD))
                         
//audio peripher sync reset
#define AUD_REG_SYNC_RESET_CFG3         (AUD_REG_TOP_MISC_BASE + 0xC4)
    #define BIT_STR_DSPA_RESET          0
    #define BIT_NUM_DSPA_RESET          1

    #define BIT_STR_DSPB_RESET          1
    #define BIT_NUM_DSPB_RESET          1

    #define BIT_STR_FS_PWMIP_RESET      2
    #define BIT_NUM_FS_PWMIP_RESET      1

    #define BIT_STR_RS_PWMIP_RESET      3
    #define BIT_NUM_RS_PWMIP_RESET      1

    
#ifdef __cplusplus
        }
#endif
                            
#endif // _AUD_REG_TOP_MISC_H
