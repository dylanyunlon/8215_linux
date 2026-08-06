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

#ifndef __ARM2__
#include "linux/types.h"
#else
#include "x_types.h"
#endif
#include "aud_pinmux.h"


static void IoPinMux_SetI2sOutFs(AUD_PINMUX_FS_I2SOUT eI2sOutFsPinSel)
{
    switch (eI2sOutFsPinSel)
    {
        case PINMUX_FS_I2SOUT_GROUP2:
            DacHal_SetPwmAnaGpioFun(PWM_SET0, TRUE);
            DacHal_SetPwmAnaGpioFun(PWM_SET1, TRUE);
            DacHal_SetPwmAnaGpioFun(PWM_SET2, TRUE);
            break;

        case PINMUX_FS_I2SOUT_DEFAULT:
        case PINMUX_FS_I2SOUT_GROUP1:
            break;
 
        default:
            break;
    }

    AUDREG_BITS_W(AUD_REG_PAD_MUX3, BIT_STR_I2S_OUT0_SEL, BIT_NUM_I2S_OUT0_SEL, eI2sOutFsPinSel);
}


static void IoPinMux_SetI2sOutRs(AUD_PINMUX_RS_I2SOUT eI2sOutRsPinSel)
{
    switch (eI2sOutRsPinSel)
    {
        case PINMUX_RS_I2SOUT_GROUP3:
            DacHal_SetPwmAnaGpioFun(PWM_SET2, TRUE);
            DacHal_SetPwmAnaGpioFun(PWM_SET3, TRUE);
            break;

        case PINMUX_RS_I2SOUT_DEFAULT:
        case PINMUX_RS_I2SOUT_GROUP1:
        case PINMUX_RS_I2SOUT_GROUP2:
            break;
 
        default:
            break;
    }

    AUDREG_BITS_W(AUD_REG_PAD_MUX6, BIT_STR_I2S_OUT1_SEL, BIT_NUM_I2S_OUT1_SEL, eI2sOutRsPinSel);
}


static void IoPinMux_SetSpdif(AUD_PINMUX_SPDIF eSpdifPinSel)
{
    AUDREG_BITS_W(AUD_REG_PAD_MUX2, BIT_STR_SPDIF_SEL, BIT_NUM_SPDIF_SEL, eSpdifPinSel);
}


static void IoPinMux_SetAmuteFs(AUD_PINMUX_AMUTE_FRONT eAmuteFsPinSel)
{
    AUDREG_BITS_W(AUD_REG_PAD_MUX3, BIT_STR_AMUTE_F_SEL, BIT_NUM_AMUTE_F_SEL, eAmuteFsPinSel);
}


static void IoPinMux_SetAmuteRs(AUD_PINMUX_AMUTE_REAR eAmuteRsPinSel)
{
    AUDREG_BITS_W(AUD_REG_PAD_MUX2, BIT_STR_AMUTE_R_SEL, BIT_NUM_AMUTE_R_SEL, eAmuteRsPinSel);
}


//==============================================//
#define CodeSight_PinMux_Cmm_fun
//==============================================//

void IoPinMux_SetCfg(AUD_DAC_TYPE_T eDacType)
{
    if (eDacType == AUD_DAC_EXT){
        IoPinMux_SetI2sOutFs(PINMUX_FS_I2SOUT_GROUP2);
        IoPinMux_SetI2sOutRs(PINMUX_RS_I2SOUT_DEFAULT);
    }else { 
        IoPinMux_SetI2sOutFs(PINMUX_FS_I2SOUT_DEFAULT);
        IoPinMux_SetI2sOutRs(PINMUX_RS_I2SOUT_DEFAULT);
    }

    IoPinMux_SetAmuteFs(PINMUX_AMUTE_REAR_GROUP1);
}


