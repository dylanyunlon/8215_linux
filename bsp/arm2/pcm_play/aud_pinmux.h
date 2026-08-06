/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */
#ifndef _AUD_IO_PINMUX_H
#define _AUD_IO_PINMUX_H

#include "aud_dac.h"

#ifdef __cplusplus
    extern "C"
    {
#endif

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


void IoPinMux_SetCfg(AUD_DAC_TYPE_T eDacType);


#ifdef __cplusplus
    }
#endif
            
            
#endif // _AUD_IO_PINMUX_H