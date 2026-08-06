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

#include "aud_oal.h"
#include "aud_pwmdac.h"

// From inc
#include "drv_config.h"
#include <media/atc/drv_aud.h>
#include "aud_debug.h"
#include "aud_config.h"
#include "aud_3360_reg_misc.h"
#include "aud_3360_reg_rw.h"
#include "aud_hal.h"
#include "aud_if.h"
#include "util.h"
#include "x_ioopt.h"


static struct{
    u32 u4AddrReg;
    u32 u4DataReg;
} atPwmIndRegDesc[] = {
    {AUD_REG_RGBK2_INDRECT_FRNT_ADDR, AUD_REG_RGBK2_INDRECT_FRNT_DATA},
    {AUD_REG_RGBK2_INDRECT_GPS_ADDR, AUD_REG_RGBK2_INDRECT_GPS_DATA},
    {AUD_REG_RGBK2_INDRECT_REAR_ADDR, AUD_REG_RGBK2_INDRECT_REAR_DATA}
};

// THD+N = -64.5
u32 sg_au4yram_x4_20_ns40[AU4_YRAM_SIZE] = {
    0x3eb600, 0x2b9a00, 0x249f00, 0x0d9000, 0xfde900, 0x000000, 0x000000, 0x0d1a00, 0xf2e600, 0x100000,
    0x100000, 0x100000, 0x100000, 0x000000, 0x000000, 0x100000, 0x100000, 0x100000, 0xfee300, 0xfce300,
    0x000000, 0x0dc200, 0xfff900, 0xfff000, 0xffed00, 0xfff900, 0x001400, 0x002f00, 0x003400, 0x001400,
    0xffd500, 0xff9800, 0xff8d00, 0xffd200, 0x005300, 0x00c800, 0x00da00, 0x005900, 0xff7000, 0xfea200,
    0xfe8600, 0xff6400, 0x00ea00, 0x023e00, 0x026a00, 0x010300, 0xfe9400, 0xfc7d00, 0xfc3800, 0xfe6400,
    0x022500, 0x055f00, 0x05ce00, 0x028300, 0xfcc900, 0xf7d200, 0xf71000, 0xfc0b00, 0x04ec00, 0x0cdf00,
    0x0e6500, 0x06a000, 0xf7cd00, 0xe95700, 0xe50800, 0xf25f00, 0x124d00, 0x3d8f00, 0x66d700, 0x7fff00,
    0x0dc200, 0x0dc200, 0x0dc200, 0x0dc200, 0x0dc200
};
                                                                    
// THD+N = -64.5
u32 sg_au4yram_x2_20_ns40[AU4_YRAM_SIZE] = {
    0x3eb600, 0x2b9a00, 0x249f00, 0x0d9000, 0xfde900, 0x000000, 0x000000, 0x0d1a00, 0xf2e600, 0x100000,
    0x100000, 0x100000, 0x100000, 0x000000, 0x000000, 0x100000, 0x100000, 0x100000, 0xfee300, 0xfce300,
    0x000000, 0x0c0000, 0x000000, 0x000000, 0x000100, 0x000100, 0xfffa00, 0xfffa00, 0x001600, 0x001700,
    0xffc100, 0xffbd00, 0x009b00, 0x00a700, 0xfeae00, 0xfe8f00, 0x02a000, 0x02ee00, 0xfb1d00, 0xfa5f00,
    0x08d400, 0x0ac800, 0xef4f00, 0xe8a600, 0x2aad00, 0x7fff00, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
    0x0c0000, 0x0c0000, 0x0c0000, 0x0c0000, 0x0c0000
};

// THD+N = -65.5
u32 sg_au4yram_x4_8_ns40[AU4_YRAM_SIZE] = {
    0x3eb600, 0x2b9a00, 0x249f00, 0x0d9000, 0xfde900, 0x000000, 0x000000, 0x0d1a00, 0xf2e600, 0x100000,
    0x100000, 0x100000, 0x100000, 0x000000, 0x000000, 0x100000, 0x100000, 0x100000, 0xfee300, 0xfce300,
    0x000000, 0x0a3000, 0x000100, 0x000300, 0x000500, 0x000600, 0x000400, 0xfffd00, 0xfff200, 0xffe600,
    0xffdf00, 0xffe500, 0xfffe00, 0x002700, 0x005500, 0x007300, 0x006a00, 0x002c00, 0xffbb00, 0xff3600,
    0xfed200, 0xfec900, 0xff4400, 0x003f00, 0x017c00, 0x028a00, 0x02e500, 0x022b00, 0x005000, 0xfdbd00,
    0xfb4900, 0xfa0000, 0xfac100, 0xfdd700, 0x02ba00, 0x07df00, 0x0b6800, 0x0b6b00, 0x06df00, 0xfe3000,
    0xf38100, 0xea5600, 0xe6c600, 0xec4300, 0xfc7400, 0x166700, 0x366f00, 0x56d900, 0x713700, 0x7fff00,
    0x0a3000, 0x0a3000, 0x0a3000, 0x0a3000, 0x0a3000
};

// THD+N = -31.5
u32 sg_au4yram_x4_16_ns40[AU4_YRAM_SIZE] = {
    0x3eb600, 0x2b9a00, 0x249f00, 0x0d9000, 0xfde900, 0x000000, 0x000000, 0x0d1a00, 0xf2e600, 0x100000,
    0x100000, 0x100000, 0x100000, 0x000000, 0x000000, 0x100000, 0x100000, 0x100000, 0xfee300, 0xfce300,
    0x000000, 0x0ccc00, 0xfe7d00, 0xfc8200, 0xfc7000, 0xfe6500, 0x014f00, 0x037a00, 0x039000, 0x016b00,
    0xfe3700, 0xfbd100, 0xfbb600, 0xfe1400, 0x01a400, 0x045200, 0x047400, 0x01cf00, 0xfdcd00, 0xfac300,
    0xfa9700, 0xfd9600, 0x022a00, 0x05ac00, 0x05e700, 0x027300, 0xfd1c00, 0xf8f600, 0xf8a400, 0xfcb600,
    0x031f00, 0x083300, 0x03b800, 0x08ad00, 0xfbb400, 0xf52b00, 0xf46000, 0xfab600, 0x056800, 0x0e9200,
    0x101f00, 0x076000, 0xf74b00, 0xe80900, 0xe3b500, 0xf19900, 0x122500, 0x3db700, 0x66f800, 0x7fff00,
    0x0ccc00, 0x0ccc00, 0x0ccc00, 0x0ccc00, 0x0ccc00
};

// THD+N = -65.5
u32 sg_u4YramData[AU4_YRAM_SIZE] = {
    0x3eb600, 0x2b9a00, 0x249f00, 0x0d9000, 0xfde900, 0x000000, 0x000000, 0x0d1a00, 0xf2e600, 0x100000,
    0x100000, 0x100000, 0x100000, 0x000000, 0x000000, 0x100000, 0x100000, 0x100000, 0xfee300, 0xfce300,
    0x000000, 0x1b8400, 0xfff900, 0xfff000, 0xffed00, 0xfff900, 0x001400, 0x002f00, 0x003400, 0x001400,
    0xffd500, 0xff9800, 0xff8d00, 0xffd200, 0x005300, 0x00c800, 0x00da00, 0x005900, 0xff7000, 0xfea200,
    0xfe8600, 0xff6400, 0x00ea00, 0x023e00, 0x026a00, 0x010300, 0xfe9400, 0xfc7d00, 0xfc3800, 0xfe6400,
    0x022500, 0x055f00, 0x05ce00, 0x028300, 0xfcc900, 0xf7d200, 0xf71000, 0xfc0b00, 0x04ec00, 0x0cdf00,
    0x0e6500, 0x06a000, 0xf7cd00, 0xe95700, 0xe50800, 0xf25f00, 0x124d00, 0x3d8f00, 0x66d700, 0x7fff00,
    0x1b8400, 0x1b8400, 0x1b8400, 0x1b8400, 0x1b8400
};


s32 Aud_PWMDAC_MultiFuncSel(AUD_DAC_CLASS_T eDacType, bool fgSel)
{
    if (fgSel)
    {
        if (eDacType == AUD_FRONT_DAC)
        {
            // PWM Data Selection, 0--Other function, 1--PWM
            CKGEN_SETBITS(AUD_REG_RGBK2_FRNT_PWM_CFG7, 0x3F << 4);
            // GPIO_PWM, 0-- Analog function, 1--Digital function.
            CKGEN_SETBITS(AUD_REG_RGBK2_PWMANA_CFG5, 0x3F);
        }
        else if (eDacType == AUD_GPS_DAC)
        {
            // PWM Data Selection, 0--Other function, 1--PWM
            CKGEN_SETBITS(AUD_REG_RGBK2_GPS_PWM_CFG7, 0x3 << 4);
            // GPIO_PWM, 0-- Analog function, 1--Digital function.
            CKGEN_SETBITS(AUD_REG_RGBK2_PWMANA_CFG5, 0x3 << 6);
        }
        else if (eDacType == AUD_REAR_DAC)
        {
            // PWM Data Selection, 0--Other function, 1--PWM
            CKGEN_SETBITS(AUD_REG_RGBK2_REAR_PWM_CFG7, 0x3 << 4);
            // GPIO_PWM, 0-- Analog function, 1--Digital function.
            CKGEN_SETBITS(AUD_REG_RGBK2_PWMANA_CFG5, 0x3 << 8);
        }
    }
    else
    {
        if (eDacType == AUD_FRONT_DAC)
        {
            // PWM Data Selection, 0--Other function, 1--PWM
            CKGEN_CLRBITS(AUD_REG_RGBK2_FRNT_PWM_CFG7, 0x3F << 4);
            
            CKGEN_BITS_WRITE(AUD_REG_RGBK2_PWMANA_CFG2, 10, 6, 0);
            //CKGEN_BITS_WRITE(AUD_REG_RGBK2_PWMANA_CFG3, 0, 20, 0xFFFFF);
            CKGEN_BITS_WRITE(AUD_REG_RGBK2_PWMANA_CFG7, 20, 6, 0);
            // GPIO_PWM, 0-- Analog function, 1--Digital function.
            //CKGEN_CLRBITS(AUD_REG_RGBK2_PWMANA_CFG5, 0x3F);
        }
        else if (eDacType == AUD_GPS_DAC)
        {
            // PWM Data Selection, 0--Other function, 1--PWM
            CKGEN_CLRBITS(AUD_REG_RGBK2_GPS_PWM_CFG7, 0x3 << 4);
            CKGEN_BITS_WRITE(AUD_REG_RGBK2_PWMANA_CFG2, 16, 4, 0);
            //CKGEN_BITS_WRITE(AUD_REG_RGBK2_PWMANA_CFG3, 0, 20, 0xFFFFF);
            CKGEN_BITS_WRITE(AUD_REG_RGBK2_PWMANA_CFG7, 26, 4, 0);
            // GPIO_PWM, 0-- Analog function, 1--Digital function.
            //CKGEN_CLRBITS(AUD_REG_RGBK2_PWMANA_CFG5, 0x3 << 6);
        }
        else if (eDacType == AUD_REAR_DAC)
        {
            // PWM Data Selection, 0--Other function, 1--PWM
            CKGEN_CLRBITS(AUD_REG_RGBK2_REAR_PWM_CFG7, 0x3 << 4);
            CKGEN_BITS_WRITE(AUD_REG_RGBK2_PWMANA_CFG2, 16, 4, 0);
            //CKGEN_BITS_WRITE(AUD_REG_RGBK2_PWMANA_CFG3, 0, 20, 0xFFFFF);
            CKGEN_BITS_WRITE(AUD_REG_RGBK2_PWMANA_CFG7, 26, 4, 0);
            // GPIO_PWM, 0-- Analog function, 1--Digital function.
            //CKGEN_CLRBITS(AUD_REG_RGBK2_PWMANA_CFG5, 0x3 << 8);
        }

        //for enhance mclk output level
       
    }

    return (AUD_OK);
}


void AUD_IndReadPwmReg(AUD_DAC_CLASS_T ePwmDac, u32 u4Addr, u32 *pu4Val)
{
    AUD_REG_WRITE(atPwmIndRegDesc[ePwmDac].u4AddrReg, u4Addr);
    *pu4Val = AUD_REG_READ(atPwmIndRegDesc[ePwmDac].u4DataReg);
}

void AUD_IndWritePwmReg(AUD_DAC_CLASS_T ePwmDac, u32 u4Addr, u32 u4Val)
{
    AUD_REG_WRITE(atPwmIndRegDesc[ePwmDac].u4AddrReg, u4Addr);
    AUD_REG_WRITE(atPwmIndRegDesc[ePwmDac].u4DataReg, u4Val);

    // Check Write correct...
    if ((u4Addr != 0xf08) && (u4Addr != 0xf09))
    {
        u32 u4ReadVal = 0x12345678;
        AUD_IndReadPwmReg(ePwmDac, u4Addr, &u4ReadVal);
        if (u4ReadVal != u4Val)
        {
            LOG(LOG_FAIL, TEXT("[AUD]^^^Error to Write PWMDAC Register(0x%x : 0x%x : 0x%x).\n"), 
                             (u32)u4Addr, (u32)u4Val, (u32)u4ReadVal);
        }
    }
}


void vPwmHwEnable(AUD_DAC_CLASS_T ePwmDac)
{
    if (AUD_FRONT_DAC == ePwmDac)
    {
        CKGEN_SETBITS(AUD_REG_RGBK2_PWMTOP_CFG, 0x3);
    }
    else if (AUD_REAR_DAC == ePwmDac)
    {
        CKGEN_SETBITS(AUD_REG_RGBK2_PWMTOP_CFG, 0x5);
    }
    else if (AUD_GPS_DAC == ePwmDac)
    {
        CKGEN_SETBITS(AUD_REG_RGBK2_PWMTOP_CFG, 0x9);
    }
}


void vPwm_Apll_Select(AUD_DAC_CLASS_T ePwmType, APLL_DOMAIN eApllDomain)
{
    u32 u4RegAddr = AUD_REG_RGBK2_FRNT_PWM_CFG1;
    u32 u4AnaClk = 0x3F; // Front 6 channel

    u4RegAddr = (ePwmType == AUD_REAR_DAC) ? AUD_REG_RGBK2_REAR_PWM_CFG1 : u4RegAddr;
    u4RegAddr = (ePwmType == AUD_GPS_DAC) ? AUD_REG_RGBK2_GPS_PWM_CFG1 : u4RegAddr;

    u4AnaClk = (ePwmType == AUD_REAR_DAC) ? 0x300 : u4AnaClk;
    u4AnaClk = (ePwmType == AUD_GPS_DAC) ? 0xC0 : u4AnaClk;

    if (eApllDomain == APLL_CLK270M)
    {
        LOG(LOG_IO, TEXT("vPwm_Apll_Select APLL_CLK270M \n"));
        AUD_REG_CLRBIT(u4RegAddr, 0x1 << 0);
        AUD_REG_CLRBIT(AUD_REG_RGBK2_PWMANA_CFG0, u4AnaClk);
    }
    else if (eApllDomain == APLL_CLK294M)
    {
        LOG(LOG_IO, TEXT("vPwm_Apll_Select APLL_CLK294M \n"));
        AUD_REG_SETBIT(u4RegAddr, 0x1 << 0);
        AUD_REG_SETBIT(AUD_REG_RGBK2_PWMANA_CFG0, u4AnaClk);
    }
    else // 26M
    {
        LOG(LOG_IO, TEXT("vPwm_Apll_Select OTHER \n"));
        // TODO:
    }
}


static s32 AudVfy_PwmBasicSetting(AUD_DAC_CLASS_T ePwmDac, u32 u4YramDataType)
{
    u32 i;
    u32 *pu4YramData;
    u32 u4YramDataNum;

    switch (u4YramDataType)
    {
    case 0:
        pu4YramData = sg_au4yram_x4_20_ns40;
        u4YramDataNum = AU4_YRAM_SIZE;
        break;

    case 1:
        pu4YramData = sg_au4yram_x4_16_ns40;
        u4YramDataNum = AU4_YRAM_SIZE;
        break;

    case 2:
        pu4YramData = sg_au4yram_x4_8_ns40;
        u4YramDataNum = AU4_YRAM_SIZE;
        break;

    case 3:
        pu4YramData = sg_au4yram_x2_20_ns40;
        u4YramDataNum = AU4_YRAM_SIZE;
        break;

    case 4:
        pu4YramData = sg_u4YramData;
        u4YramDataNum = AU4_YRAM_SIZE;
        break;

    default:
        LOG(LOG_FAIL, TEXT("[AUD]yram type err\n"));
        return AUD_FAIL;
    }
    AUD_IndWritePwmReg(ePwmDac, 0xf0a, 0x507);
    AUD_IndWritePwmReg(ePwmDac, 0xf0b, 0x600);
    AUD_IndWritePwmReg(ePwmDac, 0xf0c, 0x500);
    AUD_IndWritePwmReg(ePwmDac, 0xf0d, 0x600);

    AUD_IndWritePwmReg(ePwmDac, 0xf04, 0x3);

    AUD_IndWritePwmReg(ePwmDac, 0xf02, 0x36ff9c);
    AUD_IndWritePwmReg(ePwmDac, 0xf01, 0x640fe);

    AUD_IndWritePwmReg(ePwmDac, 0xf05, 1);

    AUD_IndWritePwmReg(ePwmDac, 0xf08, 0);/*begin to write xram data*/
    for (i = 0; i < u4YramDataNum; i++)
    {
        AUD_IndWritePwmReg(ePwmDac, 0xf09, 0);
    }
    Sleep(100);

    AUD_IndWritePwmReg(ePwmDac, 0xf08, 0x100);/*begin to write yram data*/
    for (i = 0; i < u4YramDataNum; i++)
    {
        AUD_IndWritePwmReg(ePwmDac, 0xf09, pu4YramData[i]);
    }
    Sleep(100);

    AUD_IndWritePwmReg(ePwmDac, 0xf01, 0x640FF);
    AUD_IndWritePwmReg(ePwmDac, 0xf00, 0);
    AUD_IndWritePwmReg(ePwmDac, 0xf00, 0);

    return AUD_OK;
}


static void Aud_Pwm_ForceReady(AUD_DAC_CLASS_T eDacCls)
{
    if (AUD_FRONT_DAC == eDacCls)
    {
        AUD_REG_SETBIT(AUD_REG_RGBK2_FRNT_PWM_CFG5, 0x1 << 30);
    }
    else if (AUD_REAR_DAC == eDacCls)
    {
        AUD_REG_SETBIT(AUD_REG_RGBK2_REAR_PWM_CFG5, 0x1 << 30);
    }
    else if (AUD_GPS_DAC == eDacCls)
    {
        AUD_REG_SETBIT(AUD_REG_RGBK2_GPS_PWM_CFG5, 0x1 << 30);
    }
}

void Aud_Pwm_Output_Zero(AUD_DAC_CLASS_T ePwmDac, bool fgEnable)
{
    u32 u4ReadVal;
    
    AUD_IndReadPwmReg(ePwmDac, 0xf04, &u4ReadVal);

    if(fgEnable)
    {
      u4ReadVal |= 0x30;
    }
    else
    {
      u4ReadVal &= 0xffffffcf;
    }
    
    AUD_IndWritePwmReg(ePwmDac, 0xf04, u4ReadVal);
}

static void Aud_Pwm_Enable(AUD_DAC_CLASS_T eDacCls)
{
    if (eDacCls == AUD_FRONT_DAC)
    {
        // PWM Front Pin Mux
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_FRNT_PWM_CFG1, 16, 16, 0x1010);
    }
    else if (eDacCls == AUD_REAR_DAC)
    {
        // PWM Rear Pin Mux
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_REAR_PWM_CFG1, 16, 16, 0x1010);
    }
    else if (eDacCls == AUD_GPS_DAC)
    {
        // PWM GPS Pin Mux
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_GPS_PWM_CFG1, 16, 16, 0x1010);
    }
}


static void Aud_Pwm_Reset(AUD_DAC_CLASS_T eDacCls)
{
    if (AUD_FRONT_DAC == eDacCls)
    {
        CKGEN_SETBITS(AUD_REG_RGBK2_PWMTOP_CFG, 0x3);
    }
    else if (AUD_REAR_DAC == eDacCls)
    {
        CKGEN_SETBITS(AUD_REG_RGBK2_PWMTOP_CFG, 0x5);
    }
    else
    {
        CKGEN_SETBITS(AUD_REG_RGBK2_PWMTOP_CFG, 0x9);
    }
}

static s32 Aud_Pwm_Relatch(AUD_DAC_CLASS_T eDacType)
{
    if (eDacType == AUD_FRONT_DAC)
    {
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_FRNT_PWM_CFG7, 16, 6, 0x3F);
    }
    else if (eDacType == AUD_REAR_DAC)
    {
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_REAR_PWM_CFG7, 8, 2, 0x3);
    }
    else
    {
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_GPS_PWM_CFG7, 8, 2, 0x3);
    }

    return (AUD_OK);
}

s32 Aud_PwmDacSourceSwitch(AUD_DAC_CLASS_T eDacCls,
                             AUD_OUT_TYPE_T   eSource)
{
    s32 i4RetVal = AUD_OK;

    if (eDacCls == AUD_REAR_DAC)
    {
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_REAR_PWM_CFG1,
                           AUD_REAR_PWM_IN_SEL_BIT_START,
                           AUD_REAR_PWM_IN_SEL_BIT_NUM,
                           eSource);
        if (eSource == AUD_AOUT1)
        {
            LOG(LOG_DAC, TEXT("[AUD_CFG] Rear PWM DAC select Aout1. \n"));
        }
        else if (eSource == AUD_AOUT2)
        {
            LOG(LOG_DAC, TEXT("[AUD_CFG] Rear PWM DAC select Aout2. \n"));
        }
        else if (eSource == AUD_DVD_OUT)
        {
            LOG(LOG_DAC, TEXT("[AUD_CFG] Rear PWM DAC select DVD. \n"));
        }
    }
    else if (eDacCls == AUD_FRONT_DAC)
    {
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_FRNT_PWM_CFG1,
                           AUD_FRNT_PWM_IN_SEL_BIT_START,
                           AUD_FRNT_PWM_IN_SEL_BIT_NUM,
                           eSource);
        if (eSource == AUD_AOUT1)
        {
            LOG(LOG_DAC, TEXT("[AUD_CFG] Front PWM DAC select Aout1. \n"));
        }
        else if (eSource == AUD_AOUT2)
        {
            LOG(LOG_DAC, TEXT("[AUD_CFG] Front PWM DAC select Aout2. \n"));
        }
        else if (eSource == AUD_DVD_OUT)
        {
            LOG(LOG_DAC, TEXT("[AUD_CFG] Front PWM DAC select DVD. \n"));
        }
    }

    return i4RetVal;
}

void AudCfg_UseExtLdo(bool fgUseExtLdo, AUD_DAC_CLASS_T eDacCls)
{
    if (fgUseExtLdo)
    {
        if (AUD_FRONT_DAC == eDacCls)
        {
            AUD_REG_CLRBIT(AUD_REG_RGBK2_PWMANA_CFG2, 0x3F << 10);
        }
        else if (AUD_GPS_DAC == eDacCls)
        {
            AUD_REG_CLRBIT(AUD_REG_RGBK2_PWMANA_CFG2, 0x3 << 16);
        }
        else
        {
            AUD_REG_CLRBIT(AUD_REG_RGBK2_PWMANA_CFG2, 0x3 << 18);
        }
        LOG(LOG_DAC, TEXT("[AUD_CFG] Aud_PWMDAC Use External LDO. \n"));
    }
    else
    {
        if (AUD_FRONT_DAC == eDacCls)
        {
            AUD_REG_SETBIT(AUD_REG_RGBK2_PWMANA_CFG2, 0x3F << 10);
        }
        else if (AUD_GPS_DAC == eDacCls)
        {
            AUD_REG_SETBIT(AUD_REG_RGBK2_PWMANA_CFG2, 0x3 << 16);
        }
        else
        {
            AUD_REG_SETBIT(AUD_REG_RGBK2_PWMANA_CFG2, 0x3 << 18);
        }
        LOG(LOG_DAC, TEXT("[AUD_CFG] Aud_PWMDAC Use Internal LDO. \n"));
    }
}


void Aud_PWMDAC_Setting(AUD_DAC_CLASS_T ePwmType)
{
    AudCfg_UseExtLdo(FALSE, ePwmType); // Use Internal LDO

    Aud_Pwm_ForceReady(ePwmType);

    Aud_Pwm_Enable(ePwmType);

    AUD_IndWritePwmReg(ePwmType, 0xf01, 0x0);

    // Set Default Clock for PWMDAC Register write.
    Aud_Pwm_Apll_Select(ePwmType, APLL_CLK294M);

    Aud_Pwm_Reset(ePwmType);

    Aud_Pwm_Relatch(ePwmType);
    
    switch(ePwmType)
    {
    case AUD_FRONT_DAC:
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_PWMTOP_CFG, 1, 1, 1);//front bit 1
        AUD_REG_WRITE(AUD_REG_RGBK2_FRNT_PWM_CFG3, 0xf0000100);//front pwm dac config
        AUD_REG_WRITE(AUD_REG_RGBK2_FRNT_PWM_CFG3, 0xfa029000);
        break;

    case AUD_GPS_DAC:
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_PWMTOP_CFG, 3, 1, 1);//gps bit 3
        AUD_REG_WRITE(AUD_REG_RGBK2_GPS_PWM_CFG3, 0xf0000100);//pwm dac config
        //AUD_REG_WRITE(AUD_REG_RGBK2_GPS_PWM_CFG3, 0xfa029000);
        AUD_REG_WRITE(AUD_REG_RGBK2_GPS_PWM_CFG3, 0x55029000);
        break;

    case AUD_REAR_DAC:
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_PWMTOP_CFG, 2, 1, 1);//rear bit 2
        AUD_REG_WRITE(AUD_REG_RGBK2_REAR_PWM_CFG3, 0xf0000100);//rear pwm dac config
        AUD_REG_WRITE(AUD_REG_RGBK2_REAR_PWM_CFG3, 0xfa029000);
        break;

    default:
        LOG(LOG_FAIL, TEXT("Aud_PWMDAC_Setting type err\n"));
        return ;
    }

    AudVfy_PwmBasicSetting(ePwmType, AUDVFY_PWM_YRAM_IDX);
}


///////////////////////////////////////////////////////////////////////////////
// Global Function.
///////////////////////////////////////////////////////////////////////////////
void Aud_Pwm_Apll_Select(AUD_DAC_CLASS_T eDacCls, APLL_DOMAIN eApllDomain)
{
    u32 u4RegAddr = AUD_REG_RGBK2_FRNT_PWM_CFG1;
    u32 u4AnaClk = 0x3F; // Front 6 channel

    u4RegAddr = (eDacCls == AUD_REAR_DAC) ? AUD_REG_RGBK2_REAR_PWM_CFG1 : u4RegAddr;
    u4RegAddr = (eDacCls == AUD_GPS_DAC) ? AUD_REG_RGBK2_GPS_PWM_CFG1 : u4RegAddr;

    u4AnaClk = (eDacCls == AUD_REAR_DAC) ? 0x300 : u4AnaClk;
    u4AnaClk = (eDacCls == AUD_GPS_DAC) ? 0xC0 : u4AnaClk;

    if (eApllDomain == APLL_CLK270M)
    {
        AUD_REG_CLRBIT(u4RegAddr, 0x1 << 0);
        AUD_REG_CLRBIT(AUD_REG_RGBK2_PWMANA_CFG0, u4AnaClk);
    }
    else if (eApllDomain == APLL_CLK294M)
    {
        AUD_REG_SETBIT(u4RegAddr, 0x1 << 0);
        AUD_REG_SETBIT(AUD_REG_RGBK2_PWMANA_CFG0, u4AnaClk);
    }
    else // 26M
    {
        // TODO:
    }
}


s32 Aud_PWMDAC_LRChannelSelect(AUD_DAC_CLASS_T eDacCls)
{
    if (eDacCls == AUD_FRONT_DAC)
    {
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_FRNT_PWM_CFG1, 16, 4, 1); // For R
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_FRNT_PWM_CFG1, 20, 4, 0); // For L
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_FRNT_PWM_CFG8, 0, 4, 1); // For Rs
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_FRNT_PWM_CFG8, 4, 4, 0); // For Ls
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_FRNT_PWM_CFG8, 16, 4, 1); // For LFE
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_FRNT_PWM_CFG8, 20, 4, 0); // For C
    }
    else if (eDacCls == AUD_REAR_DAC)
    {
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_REAR_PWM_CFG1, 16, 4, 1);
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_REAR_PWM_CFG1, 20, 4, 0);
    }
    else if (eDacCls == AUD_GPS_DAC)
    {
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_GPS_PWM_CFG1, 16, 4, 1);
        AUD_REG_BITS_WRITE(AUD_REG_RGBK2_GPS_PWM_CFG1, 20, 4, 0);
    }
    else
    {
        LOG(LOG_FAIL, TEXT("Aud_PWMDAC_LRChannelSelect failed.\n"));
        return (AUD_FAIL);
    }

    return (AUD_OK);
}


s32 Aud_PWMDAC_SDATASelect(AUD_DAC_CLASS_T eDacCls, AUD_OUT_TYPE_T eSource)
{
    if (eDacCls == AUD_FRONT_DAC)
    {
        if (eSource == AUD_AOUT1)
        {
            AUD_REG_BITS_WRITE(0x5240, 0, 4, AUD_CH_1_2);
            AUD_REG_BITS_WRITE(0x5240, 4, 4, AUD_CH_3_4);
            AUD_REG_BITS_WRITE(0x5240, 8, 4, AUD_CH_5_6);
            //Follow three lines for verify test.
            //AUD_REG_BITS_WRITE(0x5240, 4, 4, AUD_CH_1_2);
            //AUD_REG_BITS_WRITE(0x5240, 8, 4, AUD_CH_1_2);
            //AUD_REG_BITS_WRITE(0x5240, 12, 4, AUD_CH_1_2);
            
            LOG(LOG_DAC, TEXT("[AUD_CFG] Aout1 SDATA to Front. \n"));
        }
        else if (eSource == AUD_AOUT2)
        {
            AUD_REG_BITS_WRITE(0x5260, 0, 4, AUD_CH_1_2);
            AUD_REG_BITS_WRITE(0x5260, 4, 4, AUD_CH_3_4);
            AUD_REG_BITS_WRITE(0x5260, 8, 4, AUD_CH_5_6);
            LOG(LOG_DAC, TEXT("[AUD_CFG] Aout2 SDATA to Front. \n"));
        }
    }
    else if (eDacCls == AUD_REAR_DAC)
    {
        if (eSource == AUD_AOUT1)
        {
            AUD_REG_BITS_WRITE(0x5240, 12, 4, AUD_CH_1_2);
            LOG(LOG_DAC, TEXT("[AUD_CFG] Aout1 SDATA to Rear. \n"));
        }
        else if (eSource == AUD_AOUT2)
        {
            AUD_REG_BITS_WRITE(0x5260, 12, 4, AUD_CH_1_2);
            LOG(LOG_DAC, TEXT("[AUD_CFG] Aout2 SDATA to Rear. \n"));
        }
    }   

    return (AUD_OK);
}


s32 Aud_PWMDAC_Init(AUD_DAC_CLASS_T eDacCls)
{
    // PWMDAC analog Setting
    // PWMDAC bias enable
    AUD_REG_SETBIT(AUD_REG_RGBK2_PWMANA_CFG0, 0x1 << 30);
    // 1.2v regulator power on: 16-F, 17-G, 18-R.
    AUD_REG_SETBIT(AUD_REG_RGBK2_PWMANA_CFG7, 0x7 << 16);
    // AUADC BGR global bias on
    AUD_REG_CLRBIT(0x340, 0x1 << 14);

    // Add for after MP ECO IC.
    AUD_REG_SETBIT(AUD_REG_RGBK2_PWMANA_CFG7, 0x3FF << 20);

    Aud_PWMDAC_Setting(eDacCls);

    return (AUD_OK);
}

