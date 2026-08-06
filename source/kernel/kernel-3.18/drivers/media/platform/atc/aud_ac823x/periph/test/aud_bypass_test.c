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
 * @file aud_bypass_test.c source file
 * 
 * aud io bypass module test function
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */

#include "aud_bypass_hal_if.h"

PBYPS_HAL_CLS_PUB prByps = NULL;
extern void AudAout_ByPassMode(AUD_AOUT_DEVID eAoutId, bool fgByPass);

void AudBypsTestStart(AUD_BYPS_DST eDst, AUD_ADC_INPUT_SRC eGroup)
{
    BYPS_EXTPARAMS_T rExtCfg;

    BYPSLOG_TEST((T("AudBypsTestStart \r\n")));    
    AudAout_ByPassMode(eDst, TRUE);
    
    if (NULL == prByps)
    {
        prByps = BypsHal_New();
        

        rExtCfg.eDst = eDst;
        rExtCfg.eGainMode = BYPS_GAIN_LINER;
        rExtCfg.eGroup = eGroup;
        rExtCfg.u4Gain =0xffffff;
        rExtCfg.u4Scale =1;
        
        prByps->rHwIf.Setup(prByps, &rExtCfg);
    }

    prByps->rHwIf.Start(prByps, 0);
}

void AudBypsTestStop(void)
{
    BYPSLOG_TEST((T("AudBypsTestStop \r\n")));    
    if (NULL != prByps)
    {
        prByps->rHwIf.Stop(prByps, 0);
    }
    
    AudAout_ByPassMode(0, FALSE);
}

void AudBypsTest(u32 arg1, u32 arg2, u32 arg3)
{
    if (0 == arg1)
    {
       AudBypsTestStart(arg2, arg3);
    }
    else if (1 == arg1)
    {
       AudBypsTestStop();
    }
}

