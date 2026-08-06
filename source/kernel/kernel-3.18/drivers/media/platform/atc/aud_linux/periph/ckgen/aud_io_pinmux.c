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
 * @file aud_io_pinmux.c source file
 * 
 * aud io pin mux module driver
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */

#include "aud_io_pinmux.h"
#include "aud_io_pinmux_if.h"
#include "aud_adc_hal_if.h"

#ifdef __linux__
#include <mach/pinmux.h>
#include <mach/ac83xx_gpio_pinmux.h>
#endif


//==============================================//
#define CodeSight_PinMux_Mphon_Fun
//==============================================//

/**
 * function : set i2s mic in pin
 *
 * @param [in]  eI2sMicInPinSel : is2 mic in pin
 * @param [out] 
 *
 * @return
 */
void IoPinMux_SetI2sMicIn(AUD_PINMUX_I2SMICIN eI2sMicInPinSel)
{
    switch (eI2sMicInPinSel)    
    {
        case PINMUX_I2SMICIN_GROUP2:
            AdcHal_SetInputPinGpioFun(LIN_PIN_GROUP0, TRUE);
            AdcHal_SetInputPinGpioFun(LIN_PIN_GROUP1, TRUE);
            break;

        case PINMUX_I2SMICIN_GROUP3:
            AdcHal_SetInputPinGpioFun(LIN_PIN_GROUP2, TRUE);
            AdcHal_SetInputPinGpioFun(LIN_PIN_GROUP3, TRUE);
            break;

        case PINMUX_I2SMICIN_GROUP6:
            //need to config gpio mode
            break;

        case PINMUX_I2SMICIN_DEFAULT:
        case PINMUX_I2SMICIN_GROUP1:
        case PINMUX_I2SMICIN_GROUP4:
        case PINMUX_I2SMICIN_GROUP5:
        case PINMUX_I2SMICIN_GROUP7:
            break;

        default:
            break;

    }

#ifndef __linux__
    AUDREG_BITS_W(AUD_REG_PAD_MUX0, BIT_STR_I2S_MIC_IN_SEL, BIT_NUM_I2S_MIC_IN_SEL, eI2sMicInPinSel);
#else
    bsp_pinset(I2S_MIC_IN_SEL, eI2sMicInPinSel);
#endif
}

/**
 * function : set sgm mic in pin
 *
 * @param [in]  eSgmMicInPinSel : sgm mic in pin
 * @param [out] 
 *
 * @return
 */
void IoPinMux_SetSgmMicIn(AUD_PINMUX_SGM_MICIN eSgmMicInPinSel)
{
    switch (eSgmMicInPinSel)    
    {
        case PINMUX_SGM_MICIN_GROUP2:
            AdcHal_SetInputPinGpioFun(LIN_PIN_GROUP3, TRUE);
            AdcHal_SetInputPinGpioFun(LIN_PIN_GROUP4, TRUE);
            break;

        case PINMUX_SGM_MICIN_DEFAULT:
        case PINMUX_SGM_MICIN_GROUP1:
        case PINMUX_SGM_MICIN_GROUP3:
            break;

        default:
            break;

    }

#ifndef __linux__
    AUDREG_BITS_W(AUD_REG_PAD_MUX1, BIT_STR_SGM_MIC_IN_SEL, BIT_NUM_SGM_MIC_IN_SEL, eSgmMicInPinSel);
#else
    bsp_pinset(SGM_MIC_IN_SEL, eSgmMicInPinSel);
#endif
}



//==============================================//
#define CodeSight_PinMux_Lin_Fun
//==============================================//
/**
 * function : set i2s lin pin
 *
 * @param [in]  eI2sLinPinSel : is2 lin pin
 * @param [out] 
 *
 * @return
 */
void IoPinMux_SetI2sLin(AUD_PINMUX_I2SLIN0 eI2sLinPinSel)
{
    switch (eI2sLinPinSel)
    {
        case PINMUX_I2SLIN0_GROUP2:
            AdcHal_SetInputPinGpioFun(LIN_PIN_GROUP0, TRUE);
            AdcHal_SetInputPinGpioFun(LIN_PIN_GROUP1, TRUE);
            break;

        case PINMUX_I2SLIN0_GROUP3:
            AdcHal_SetInputPinGpioFun(LIN_PIN_GROUP2, TRUE);
            AdcHal_SetInputPinGpioFun(LIN_PIN_GROUP3, TRUE);
            break;

        case PINMUX_I2SLIN0_GROUP6:
            //set to digital gpio pin first
            break;

        case PINMUX_I2SLIN0_DEFAULT:
        case PINMUX_I2SLIN0_GROUP1:
        case PINMUX_I2SLIN0_GROUP4:
        case PINMUX_I2SLIN0_GROUP5:
        case PINMUX_I2SLIN0_GROUP7:
            break;
 
        default:
            break;
    }
#ifndef __linux__
    AUDREG_BITS_W(AUD_REG_PAD_MUX7, BIT_STR_I2S_LINE0_IN_SEL, BIT_NUM_I2S_LINE0_IN_SEL, eI2sLinPinSel);
#else
    bsp_pinset(I2S_LINE0_IN_SEL, eI2sLinPinSel);
#endif
}

/**
 * function : set i2s lin2 pin
 *
 * @param [in]  eI2sLinPinSel : is2 lin pin
 * @param [out] 
 *
 * @return
 */
void IoPinMux_SetI2sLin2(AUD_PINMUX_I2SLIN1 eI2sLin2PinSel)
{
    switch (eI2sLin2PinSel)
    {
        case PINMUX_I2SLIN1_GROUP2:
            AdcHal_SetInputPinGpioFun(LIN_PIN_GROUP0, TRUE);
            AdcHal_SetInputPinGpioFun(LIN_PIN_GROUP1, TRUE);
            break;

        case PINMUX_I2SLIN1_GROUP3:
            AdcHal_SetInputPinGpioFun(LIN_PIN_GROUP2, TRUE);
            AdcHal_SetInputPinGpioFun(LIN_PIN_GROUP3, TRUE);
            break;

        case PINMUX_I2SLIN1_GROUP6:
            //set to digital gpio pin first
            break;

        case PINMUX_I2SLIN1_DEFAULT:
        case PINMUX_I2SLIN1_GROUP1:
        case PINMUX_I2SLIN1_GROUP4:
        case PINMUX_I2SLIN1_GROUP5:
        case PINMUX_I2SLIN1_GROUP7:
            break;
 
        default:
            break;
    }
#ifndef __linux__
    AUDREG_BITS_W(AUD_REG_PAD_MUX7, BIT_STR_I2S_LINE1_IN_SEL, BIT_NUM_I2S_LINE1_IN_SEL, eI2sLin2PinSel);
#else
    bsp_pinset(I2S_LINE1_IN_SEL, eI2sLin2PinSel);
#endif
}


//==============================================//
#define CodeSight_PinMux_I2s_out_Fun
//==============================================//
/**
 * function : set i2s out front seat pin
 *
 * @param [in]  eI2sOutFsPinSel : front seat i2s out pin
 * @param [out] 
 *
 * @return
 */
void IoPinMux_SetI2sOutFs(AUD_PINMUX_FS_I2SOUT eI2sOutFsPinSel)
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
#ifndef __linux__
    AUDREG_BITS_W(AUD_REG_PAD_MUX3, BIT_STR_I2S_OUT0_SEL, BIT_NUM_I2S_OUT0_SEL, eI2sOutFsPinSel);
#else
    bsp_pinset(I2S_OUT0_SEL, eI2sOutFsPinSel);
#endif
}

/**
 * function : set i2s out rear seat pin
 *
 * @param [in]  eI2sOutRsPinSel : rear seat i2s out pin
 * @param [out] 
 *
 * @return
 */
void IoPinMux_SetI2sOutRs(AUD_PINMUX_RS_I2SOUT eI2sOutRsPinSel)
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
#ifndef __linux__
    AUDREG_BITS_W(AUD_REG_PAD_MUX6, BIT_STR_I2S_OUT1_SEL, BIT_NUM_I2S_OUT1_SEL, eI2sOutRsPinSel);
#else
    bsp_pinset(I2S_OUT1_SEL, eI2sOutRsPinSel);
#endif
}


//==============================================//
#define CodeSight_PinMux_Pcm_Module_Fun
//==============================================//
/**
 * function : set pcm module pin
 *
 * @param [in]  eI2sOutRsPinSel : rear seat i2s out pin
 * @param [out] 
 *
 * @return
 */
void IoPinMux_SetPcm(AUD_PINMUX_PCM ePcmPinSel)
{
#ifndef __linux__
    AUDREG_BITS_W(AUD_REG_PAD_MUX1, BIT_STR_PCM_SEL, BIT_NUM_PCM_SEL, ePcmPinSel);
#else
    bsp_pinset(PCM_SEL, ePcmPinSel);
#endif
}


//==============================================//
#define CodeSight_PinMux_Spdif_module_Fun
//==============================================//
/**
 * function : set spdif module pin
 *
 * @param [in]  eSpdifPinSel : spdif pin
 * @param [out] 
 *
 * @return
 */
void IoPinMux_SetSpdif(AUD_PINMUX_SPDIF eSpdifPinSel)
{
#ifndef __linux__
    AUDREG_BITS_W(AUD_REG_PAD_MUX2, BIT_STR_SPDIF_SEL, BIT_NUM_SPDIF_SEL, eSpdifPinSel);
#else
    bsp_pinset(SPDIF_SEL, eSpdifPinSel);
#endif
}


//==============================================//
#define CodeSight_PinMux_Amute_Fun
//==============================================//

/**
 * function : set amute front seat pin
 *
 * @param [in]  eAmuteFsPinSel : amute front seat pin
 * @param [out] 
 *
 * @return
 */
void IoPinMux_SetAmuteFs(AUD_PINMUX_AMUTE_FRONT eAmuteFsPinSel)
{
#ifndef __linux__
    AUDREG_BITS_W(AUD_REG_PAD_MUX3, BIT_STR_AMUTE_F_SEL, BIT_NUM_AMUTE_F_SEL, eAmuteFsPinSel);
#else
    bsp_pinset(AMUTE_F_SEL, eAmuteFsPinSel);
#endif
}

/**
 * function : set amute rear seat pin
 *
 * @param [in]  eAmuteRsPinSel : amute rear seat pin
 * @param [out] 
 *
 * @return
 */
void IoPinMux_SetAmuteRs(AUD_PINMUX_AMUTE_REAR eAmuteRsPinSel)
{
#ifndef __linux__
    AUDREG_BITS_W(AUD_REG_PAD_MUX2, BIT_STR_AMUTE_R_SEL, BIT_NUM_AMUTE_R_SEL, eAmuteRsPinSel);
#else
    bsp_pinset(AMUTE_R_SEL, eAmuteRsPinSel);
#endif
}


//==============================================//
#define CodeSight_PinMux_Cmm_fun
//==============================================//
/**
 * function : set audio io pin mux default config
 *
 * @param [in]
 * @param [out] 
 *
 * @return
 */
void IoPinMux_SetDefaultCfg(bool fgPowerOnByArm9)
{
    // Mic in pinmux select
    IoPinMux_SetI2sMicIn(PINMUX_I2SMICIN_DEFAULT);
    IoPinMux_SetSgmMicIn(PINMUX_SGM_MICIN_DEFAULT);

    // Line in pinmux select
    IoPinMux_SetI2sLin(PINMUX_I2SLIN0_DEFAULT);
    IoPinMux_SetI2sLin2(PINMUX_I2SLIN1_DEFAULT);

    // PCM in pinmux select
    IoPinMux_SetPcm(PINMUX_PCM_DEFAULT);

    // SPDIF pinmux select
    IoPinMux_SetSpdif(PINMUX_SPDIF_DEFAULT);

#ifndef __linux__
    // Rear aout(aout2) pinmux select
    IoPinMux_SetAmuteRs(PINMUX_AMUTE_REAR_GROUP1);
    IoPinMux_SetI2sOutRs(PINMUX_RS_I2SOUT_DEFAULT);

    // Front aout(aout1) pinmux select
    if (!fgPowerOnByArm9) {
        IoPinMux_SetI2sOutFs(PINMUX_FS_I2SOUT_DEFAULT);
        IoPinMux_SetAmuteFs(PINMUX_AMUTE_REAR_GROUP1);
    }
#endif
}


