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
 * @file aud_io_pinmux_if.h include file
 * 
 * This file is global macros & definitions
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#ifndef _AUD_IO_PINMUX_IF_H
#define _AUD_IO_PINMUX_IF_H

#ifdef __cplusplus
    extern "C"
    {
#endif


/**********************************************************************************
*
*   macros
*
**********************************************************************************/


/**********************************************************************************
*
*   data type
*
**********************************************************************************/

typedef enum
{                                   //D0,           BCK,            MCLK,            LRCK
    PINMUX_I2SMICIN_DEFAULT,
    PINMUX_I2SMICIN_GROUP1,         //i2s_in1_d     i2s_in1_bck     i2s_in1_mclk     i2s_in1_lrck
    PINMUX_I2SMICIN_GROUP2,         //ain0_r        ain0_l          ain1_r           ain1_l
    PINMUX_I2SMICIN_GROUP3,         //ain2_r        ain2_l          ain3_r           ain3_l
    PINMUX_I2SMICIN_GROUP4,         //demod_rst     ts_d5           ts_d6            ts_d7
    PINMUX_I2SMICIN_GROUP5,         //vb4           vb5             vb6              vb7
    PINMUX_I2SMICIN_GROUP6,         //lvds_ao1p     lvds_ao1n       lvds_ao0p        lvds_ao0n
    PINMUX_I2SMICIN_GROUP7,         //ts_d1         ts_d2           ts_d3            ts_d4
    PINMUX_I2SMICIN_GROUP_MAX,
}AUD_PINMUX_I2SMICIN;

typedef enum
{                                    //D0            BCK             MCLK             LRCK
    PINMUX_I2SLIN0_DEFAULT,
    PINMUX_I2SLIN0_GROUP1,           //i2s_in1_d     i2s_in1_bck     i2s_in1_mclk     i2s_in1_lrck
    PINMUX_I2SLIN0_GROUP2,           //ain0_r        ain0_l          ain1_r           ain1_l
    PINMUX_I2SLIN0_GROUP3,           //ain2_r        ain2_l          ain3_r           ain3_l
    PINMUX_I2SLIN0_GROUP4,           //demod_rst     ts_d5           ts_d6            ts_d7
    PINMUX_I2SLIN0_GROUP5,           //vb4           vb5             vb6              vb7
    PINMUX_I2SLIN0_GROUP6,           //lvds_ao1p     lvds_ao1n       lvds_ao0p        lvds_ao0n
    PINMUX_I2SLIN0_GROUP7,           //ts_d1         ts_d2           ts_d3            ts_d4
    PINMUX_I2SLIN0_GROUP_MAX,
}AUD_PINMUX_I2SLIN0;

typedef enum
{                                    //D0            BCK             MCLK             LRCK
    PINMUX_I2SLIN1_DEFAULT,
    PINMUX_I2SLIN1_GROUP1,           //i2s_in1_d     i2s_in1_bck     i2s_in1_mclk     i2s_in1_lrck
    PINMUX_I2SLIN1_GROUP2,           //ain0_r        ain0_l          ain1_r           ain1_l
    PINMUX_I2SLIN1_GROUP3,           //ain2_r        ain2_l          ain3_r           ain3_l
    PINMUX_I2SLIN1_GROUP4,           //demod_rst     ts_d5           ts_d6            ts_d7
    PINMUX_I2SLIN1_GROUP5,           //vb4           vb5             vb6              vb7
    PINMUX_I2SLIN1_GROUP6,           //lvds_ao1p     lvds_ao1n       lvds_ao0p        lvds_ao0n
    PINMUX_I2SLIN1_GROUP7,           //ts_d1         ts_d2           ts_d3            ts_d4
    PINMUX_I2SLIN1_GROUP_MAX,
}AUD_PINMUX_I2SLIN1;

typedef enum
{                                    //ADC_CLK            ADC0_DIN             ADC1_DIN
    PINMUX_SGM_MICIN_DEFAULT,
    PINMUX_SGM_MICIN_GROUP1,         //ts_d5              ts_d6                ts_d7
    PINMUX_SGM_MICIN_GROUP2,         //ain4_r             ain4_l               ain3_r
    PINMUX_SGM_MICIN_GROUP3,         //i2s_in1_d          i2s_in1_bck          i2s_in1_mclk
    PINMUX_SGM_MICIN_GROUP_MAX,
}AUD_PINMUX_SGM_MICIN;

typedef enum
{                                   //D2              D1             D0            BCK            MCLK            LRCK
    PINMUX_FS_I2SOUT_DEFAULT,
    PINMUX_FS_I2SOUT_GROUP1,        //i2s_out0_d2     i2s_out0_d1    i2s_out0_d0   i2s_out0_bck   i2s_out0_mclk   i2s_out0_lrck
    PINMUX_FS_I2SOUT_GROUP2,        //ar2             al2            ar1           al1            ar0             al0
    PINMUX_FS_I2SOUT_GROUP_MAX,
}AUD_PINMUX_FS_I2SOUT;

typedef enum
{                                   //MCLK             LRCK           D0             BCK
    PINMUX_RS_I2SOUT_DEFAULT,
    PINMUX_RS_I2SOUT_GROUP1,        //i2s_in1_mclk     i2s_in1_lrck   i2s_in1_d     i2s_in1_bck
    PINMUX_RS_I2SOUT_GROUP2,        //i2s_out0_mclk    i2s_out0_lrck  i2s_out0_d0   i2s_out0_bck
    PINMUX_RS_I2SOUT_GROUP3,        //ar3              al3            ar2           al2
    PINMUX_RS_I2SOUT_GROUP_MAX,
}AUD_PINMUX_RS_I2SOUT;

typedef enum
{
    PINMUX_PCM_DEFAULT,
    PINMUX_PCM_GROUP1,
    PINMUX_PCM_MAX,
}AUD_PINMUX_PCM;

typedef enum
{
    PINMUX_SPDIF_DEFAULT,
    PINMUX_SPDIF_GROUP1,
    PINMUX_SPDIF_MAX,
}AUD_PINMUX_SPDIF;

typedef enum
{
    PINMUX_AMUTE_FRONT_DEFAULT,
    PINMUX_AMUTE_FRONT_GROUP1,
    PINMUX_AMUTE_FRONT_MAX,
}AUD_PINMUX_AMUTE_FRONT;

typedef enum
{
    PINMUX_AMUTE_REAR_DEFAULT,
    PINMUX_AMUTE_REAR_GROUP1,
    PINMUX_AMUTE_REAR_MAX,
}AUD_PINMUX_AMUTE_REAR;


/**********************************************************************************
*
*   For Debug Log
*
**********************************************************************************/


/**********************************************************************************
*
*   Functions 
*
**********************************************************************************/


extern void IoPinMux_SetI2sMicIn(AUD_PINMUX_I2SMICIN eI2sMicInPinSel);
extern void IoPinMux_SetI2sLin(AUD_PINMUX_I2SLIN0 eI2sLinPinSel);
extern void IoPinMux_SetI2sLin2(AUD_PINMUX_I2SLIN1 eI2sLin2PinSel);
extern void IoPinMux_SetI2sOutFs(AUD_PINMUX_FS_I2SOUT eI2sOutFsPinSel);
extern void IoPinMux_SetI2sOutRs(AUD_PINMUX_RS_I2SOUT eI2sOutRsPinSel);
extern void IoPinMux_SetPcm(AUD_PINMUX_PCM ePcmPinSel);
extern void IoPinMux_SetSpdif(AUD_PINMUX_SPDIF eSpdifPinSel);
extern void IoPinMux_SetAmuteFs(AUD_PINMUX_AMUTE_FRONT eAmuteFsPinSel);
extern void IoPinMux_SetAmuteRs(AUD_PINMUX_AMUTE_REAR eAmuteRsPinSel);
extern void IoPinMux_SetDefaultCfg(bool fgPowerOnByArm9);




#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_IO_PINMUX_IF_H