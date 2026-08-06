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
 * @file aud_micin_test.c source file
 * 
 * aud io mic in module test function
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */

#include "aud_micin_hal_if.h"

PMIC_HAL_CLS_PUB prMic = NULL;

void AudMicTestStart(u32 u4SrcMode)
{
    MIC_EXTPARAMS_T rExtCfg;

    MICLOG_TEST((T("AudMicTestStart \r\n")));
    
    if (NULL == prMic)
    {
        prMic = MicHal_New();
        
        if (0 == u4SrcMode)
        {
            MICLOG_TEST((T(" AudMicTest : Int Mic \r\n")));

            rExtCfg.eSrc = INT_MICIN;
            rExtCfg.eFs = FS_8K;
            rExtCfg.u4SrcBitNum = 16;
        }else
        {
            MICLOG_TEST((T(" AudMicTest : Ext Mic \r\n")));
            
            rExtCfg.eSrc = EXT_MICIN;
            rExtCfg.eFs = FS_48K;
            rExtCfg.u4SrcBitNum = 24;
        }

        rExtCfg.u4MicGain = 14;
        rExtCfg.eOutBitNum = LIN_16;
        rExtCfg.u4BufPhyAdr = 0;
        rExtCfg.u4BufSz = 32000;

        prMic->rHwIf.Setup(prMic, &rExtCfg);
    }

    prMic->rHwIf.Start(prMic, 0);
}

void AudMicTestStop(void)
{
    MICLOG_TEST((T("AudMicTestStop \r\n")));
    
    if (NULL != prMic)
    {
        prMic->rHwIf.Stop(prMic, 0);
    }
}

void AudMicTest(u32 arg1, u32 arg2, u32 arg3)
{
    if (0 == arg1)
    {
       AudMicTestStart(arg2);
    }
    else if (1 == arg1)
    {
       AudMicTestStop();
    }
}

