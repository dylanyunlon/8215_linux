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




#include "x_bim.h"
#include "x_debug.h"
#include <mach/base_regs.h>
#ifdef __linux__
#include <mach/83xx_irqs_vector.h>
#endif
#include <media/atc/drv_aud.h>
#include "aud_if.h"

#include "aud_clock.h"
#include "aud_3360_reg_rw.h"
#include "aud_pwmdac.h"
#include "aud_extdac.h"
#include "util.h"
#include "aud_debug.h"
#include "aud_config.h"
#include "aud_reg_ckgen.h"

bool g_fgAPLLPowerOnByArm9       = FALSE;
static bool g_fgAPLL1PowerOn     = FALSE;       // 270M
static bool g_fgAPLL2PowerOn     = FALSE;       // 294M
static bool g_fgAudSysClkEnable  = FALSE;

static u32 Aud_APLLCalibration(APLL_DOMAIN eAPLLDomain)
{
    u32 failN = 0;
    u32 timeout;
    u32 kfail = 0;
    u32 u4PwdReg, u4AutOKReg, u4PcwReg;

    if (APLL_CLK270M == eAPLLDomain)
    {
        u4PwdReg   = ANA7_PLLGP_CFG14; // 0x2B8
        u4AutOKReg = ANA7_PLLGP_CFG15; // 0x2BC
        u4PcwReg   = ANA7_PLLGP_CFG16; // 0x2C0
    }
    else
    {
        u4PwdReg   = ANA7_PLLGP_CFG9;  // 0x2A4
        u4AutOKReg = ANA7_PLLGP_CFG10; // 0x2A8
        u4PcwReg   = ANA7_PLLGP_CFG11; // 0x2AC
    }
    CKGEN_SETBITS(u4PwdReg, BIT_RG_APLL_PWD);
    CKGEN_SETBITS(u4AutOKReg, BIT_RG_APLL_AUTOK_LOAD | BIT_RG_APLL_AUTOK_VCO);

    CKGEN_WRITE32_MASK(u4PwdReg, APLL_288MHz, PLL_SETTING_MASK);

    //failN = 0;
    do
    {
        CKGEN_CLRBITS(u4PwdReg, BIT_RG_APLL_PWD);
        timeout = 0;
        do
        {
            AudUtil_Delayus(200);
            if (CKGEN_READ32(MONITOR_PLLGP_STATUS) & BIT_AD_RGS_APLL294_VCOCAL_FAIL)
            {
                failN++;
                CKGEN_SETBITS(u4PwdReg, BIT_RG_APLL_PWD);
                kfail = 1;
                break;
            }
            else if (CKGEN_READ32(MONITOR_PLLGP_STATUS) & BIT_AD_RGS_APLL294_VCOCAL_CPLT)
            {
                kfail = 0;
                break;
            }
            timeout++;
        } while (timeout < APLL_CALIBRATION_TIMEOUT);

        if (timeout >= APLL_CALIBRATION_TIMEOUT)
        {
            kfail = 1;
            failN = 2;
        }

        if (kfail == 0)
        {
            break;
        }
    }while (failN < 2);

    if (kfail == 0)
    {
        CKGEN_CLRBITS(u4AutOKReg, BIT_RG_APLL_AUTOK_VCO);
        return (AUD_OK);
    }

    return (AUD_FAIL);
}


u32 Aud_APLLPowerOn(APLL_DOMAIN eDomain)
{
    if (!g_fgAPLL1PowerOn && (APLL_CLK270M == eDomain))
    {
        CKGEN_WRITE32_MASK(ANA7_PLLGP_CFG19, (1 << 2), BIT_RG_PLL_RESERVE_MASK);
        // Power On 270M
        if(Aud_APLLCalibration(APLL_CLK270M) != AUD_OK)
        {
            LOG(LOG_FAIL, TEXT("Error: Failed to Calibration 270M.\n"));
            return (AUD_FAIL);
        }
        AudUtil_Delayus(20);
        CKGEN_SETBITS(ANA7_PLLGP_CFG15, BIT_RG_APLL_DDS_RSTB | BIT_RG_APLL_DDS_PWDB);
        AudUtil_Delayus(40);
        CKGEN_SETBITS(ANA7_PLLGP_CFG15, BIT_RG_APLL_PCW_NCPO_CHG);
        CKGEN_SETBITS(ANA7_PLLGP_CFG17, BIT_RG_APLL_DDS_CLK_PH_INV);
        AudUtil_Delayus(40);
        CKGEN_SETBITS(ANA7_PLLGP_CFG17, BIT_RG_APLL_FIFO_START_MAN | BIT_RG_APLL_DDS_NCPO_EN);
        AudUtil_Delayus(10);
        CKGEN_SETBITS(ANA7_PLLGP_CFG15, BIT_RG_APLL_DDSEN);
        g_fgAPLL1PowerOn = TRUE;
    }
    else if (!g_fgAPLLPowerOnByArm9 && !g_fgAPLL2PowerOn && (APLL_CLK294M == eDomain))
    {
        // Power On 294M
        CKGEN_WRITE32_MASK(ANA7_PLLGP_CFG19, (1 << 2), BIT_RG_PLL_RESERVE_MASK);
        if (Aud_APLLCalibration(APLL_CLK294M) != AUD_OK)
        {
            LOG(LOG_FAIL, TEXT("Error: Failed to Calibration 294M.\n"));
            return (AUD_FAIL);
        }
        AudUtil_Delayus(20);
        CKGEN_SETBITS(ANA7_PLLGP_CFG10, BIT_RG_APLL_DDS_RSTB | BIT_RG_APLL_DDS_PWDB);
        AudUtil_Delayus(40);
        CKGEN_SETBITS(ANA7_PLLGP_CFG10, BIT_RG_APLL_PCW_NCPO_CHG);
        CKGEN_SETBITS(ANA7_PLLGP_CFG12, BIT_RG_APLL_DDS_CLK_PH_INV);
        AudUtil_Delayus(40);
        CKGEN_SETBITS(ANA7_PLLGP_CFG12, BIT_RG_APLL_FIFO_START_MAN | BIT_RG_APLL_DDS_NCPO_EN);
        AudUtil_Delayus(10);
        CKGEN_SETBITS(ANA7_PLLGP_CFG10, BIT_RG_APLL_DDSEN);
        g_fgAPLL2PowerOn = TRUE;
    }

    if (!g_fgAPLLPowerOnByArm9 && !g_fgAudSysClkEnable)
    {
        // Enable Audio System Clock.
      
        CKGEN_SETBITS(AUD_REG_RST_1, 0x3FF); //clock enable
        CKGEN_CLRBITS(AUD_REG_RST_2, 0x7);  //clear reset bit first
        AudUtil_Delayus(1);
        CKGEN_SETBITS(AUD_REG_RST_2, 0x7);  //AOUT HW reset        

        g_fgAudSysClkEnable = TRUE;
    }
    // Auto Traceing: Select 294M for ADC 48K SRC to Aout 48k
    CKGEN_SETBITS(REG_AP_REG3, 0x1 << 31);

    return (AUD_OK);
}

u32 Aud_APLLPowerDown(void)
{
    g_fgAPLL1PowerOn = FALSE;
    g_fgAPLL2PowerOn = FALSE;
    g_fgAudSysClkEnable = FALSE;
  
    return (AUD_OK);
}

void Aud_ApllDirectAdjust(AUD_APLL_ADJ_MODE_E eAdjMode, u32 u4StepNum)
{
    u32 u4PcwVal;

    u4PcwVal = AUD_REG_BITS_READ(AUD_REG_PLLGP_APLL_CFG9, BIT_STR_PCW_NCPO, BIT_NUM_PCW_NCPO);

    switch (eAdjMode)
    {
    case AUD_APLL_ADJ_NORMAL:
        LOG(LOG_DAC, TEXT("APLL ADJUST NORMAL \n"));
        u4PcwVal = 0x73126E9;
        break;

    case AUD_APLL_ADJ_UP:
        LOG(LOG_DAC, TEXT("APLL ADJUST UP \n"));
        u4PcwVal += (u4StepNum * APLL_ADJ_STEP);
        break;

    case AUD_APLL_ADJ_DOWN:
        LOG(LOG_DAC, TEXT("APLL ADJUST DOWN \n"));
        u4PcwVal -= (u4StepNum * APLL_ADJ_STEP);
        break;

    default:
        LOG(LOG_DAC, TEXT("APLL ADJUST NORMAL \n"));
        u4PcwVal = 0x73126E9;
        break;
    }
    LOG(LOG_DAC, TEXT("APLL ADJUST RESULT: [normal]:[294.912 M] -VS- [current]:[0x%x M] \n"), (36*(1 + (u4PcwVal >> 24))));

    AUD_REG_BITS_WRITE(AUD_REG_PLLGP_APLL_CFG8, BIT_STR_PCW_NCPO_CHG, BIT_NUM_PCW_NCPO_CHG, 0);
    AUD_REG_BITS_WRITE(AUD_REG_PLLGP_APLL_CFG9, BIT_STR_PCW_NCPO, BIT_NUM_PCW_NCPO, u4PcwVal);
    AUD_REG_BITS_WRITE(AUD_REG_PLLGP_APLL_CFG8, BIT_STR_PCW_NCPO_CHG, BIT_NUM_PCW_NCPO_CHG, 1);
}

void vDspSetClock(void)
{
    AUD_REG_WRITE(0xc, (((AUD_REG_READ(0xc)&0xfffffff0)|0x6)));
}


