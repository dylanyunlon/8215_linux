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
 * @file aud_linein_test.c source file
 * 
 * aud io line in module test function
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */

#include "aud_linein_hal_if.h"

PLIN_HAL_CLS_PUB prLin = NULL;
u32 u4LinIntCnt = 0;


void AudLineInTestIsr(u32 u4Param)
{
    LINLOG_INFO(T(" AudLineInTestIsr Run \r\n"));
    u4LinIntCnt ++;
}


void AudLinTestStart(u32 u4SrcMode, AUD_ADC_INPUT_SRC eLinGroup)
{
    LIN_EXTPARAMS_T rExtCfg;

    LINLOG_TEST(T("AudLinTestStart \r\n"));
    
    if (NULL == prLin)
    {
        prLin = LinHal_New(AUDID_LIN1);
        
        if (0 == u4SrcMode)
        {
            LINLOG_TEST(T(" AudLinTest : Internal LineIn \r\n"));

            rExtCfg.eSrc = INT_LINEIN;
            rExtCfg.eIntClkSrc = LIN_CLK_MLIN;
            rExtCfg.eGroup = eLinGroup;
        }
        else
        {
            LINLOG_TEST(T(" AudLinTest : External LineIn \r\n"));
            
        }

        rExtCfg.u4BufPhyAdr = 0;
        rExtCfg.u4BufSz = 200 * 1024;

        rExtCfg.rIntCfg.fgOn = FALSE;
        rExtCfg.rIntCfg.eIntPeriod = LIN_INT_PERIOD_128DW;
        rExtCfg.rIntCfg.PFN_ISR_CB = NULL;
        
        prLin->rHwIf.Setup(prLin, &rExtCfg);
    }

    prLin->rHwIf.Start(prLin, 0);
}

void AudLinTestStop(void)
{
    LINLOG_TEST(T("AudLinTestStop \r\n"));
    
    if (NULL != prLin)
    {
        prLin->rHwIf.Stop(prLin, 0);
    }
}

void AudLinTest(u32 arg1, u32 arg2, u32 arg3)
{
    if (0 == arg1)
    {
       AudLinTestStart(arg2, arg3);
    }
    else if (1 == arg1)
    {
       AudLinTestStop();
    }
}

