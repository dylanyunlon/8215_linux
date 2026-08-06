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
 * @file aud_mline_test.c source file
 * 
 * aud io multi line in module test function
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */

#include "aud_mline_hal_if.h"


PMLIN_HAL_CLS_PUB prMlin = NULL;


void AudMlinTestIsr(u32 u4VectorId)
{
    if (NULL == prMlin)
    {
        MLINLOG_ERR((T("AudMlinTestIsr Mlin Obj IS NULL !!!  \r\n")));
        return;        
    }
    
    MLINLOG_TEST((T("AudMlinTestIsr \r\n")));
}

void AudMlinTestStart(u32 u4SrcBitNum,  u32 u4SrcChNum)
{
    MLIN_EXTPARAMS_T rExtCfg;


    MLINLOG_TEST((T("AudAoutTestStart \r\n")));
    
    if (NULL == prMlin)
    {
        prMlin = MlinHal_New();

        if (NULL == prMlin)
        {
            MLINLOG_ERR((T("AudMlinTestStart New Mlin Obj Fail !!!  \r\n")));
            return;
        }

        rExtCfg.u4SrcBitNum = u4SrcBitNum;
        rExtCfg.eOutBitNum = 24;
        rExtCfg.eDataFmt = AUDFMT_IIS;
        rExtCfg.eMlinChNum = u4SrcChNum;
        rExtCfg.eCycle = AUD_LRCK_CYC_32;
        rExtCfg.u4BufPhyAdr = 0;
        rExtCfg.u4BufSz = 0x100;

        rExtCfg.eIntPeriod = MLIN_INTPERID_128DW;
        rExtCfg.PFN_ISR_CB = AudMlinTestIsr;
        
        prMlin->rHwIf.Setup(prMlin, &rExtCfg);
    }

    prMlin->rHwIf.Start(prMlin, 0);
}

void AudMlinTestStop(void)
{
    MLINLOG_TEST((T("AudMlinTestStop \r\n")));
    
    if (NULL != prMlin)
    {
        prMlin->rHwIf.Stop(prMlin, 0);
    }
}

void AudMlinTest(u32 arg1, u32 arg2, u32 arg3)
{
    u32 u4Wp;
    MLIN_SPDIF_INFO_T rSpdifInfo;
    
    if (0 == arg1)
    {
       AudMlinTestStart(arg2, arg3);
    }
    else if (1 == arg1)
    {
       AudMlinTestStop();
    }
    else if (100 == arg1)
    {
        if (NULL != prMlin)
        {
            MLINLOG_TEST((T("AudMlinTest : Mlin OBJ DELETE \r\n")));
            prMlin->Delete(prMlin);
            prMlin = NULL;
        }
    }
    else
    {
        if (NULL != prMlin)
        {
            u4Wp = prMlin->rHwIf.GetPoint(prMlin);
            prMlin->GetSpdifType(prMlin, &rSpdifInfo);

            MLINLOG_TEST((T("AudMlinTest : u4Wp(0x%x) \r\n"), (u32)u4Wp));
            MLINLOG_TEST((T("AudMlinTest : rSpdifInfo(0x%x, 0x%x, 0x%x, 0x%x) \r\n"), rSpdifInfo.DETAIL, rSpdifInfo.BSNUM, rSpdifInfo.ROUGH, rSpdifInfo.DEC));
        }
    }
}

