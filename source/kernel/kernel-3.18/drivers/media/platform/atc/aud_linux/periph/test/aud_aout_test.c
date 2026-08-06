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
 * @file aud_aout_test.c source file
 * 
 * aud io aout module test function
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */

#include "aud_aout_hal_if.h"

typedef struct
{
   u32 u4BufSz;
   u32 u4PhyAdr;
   u32 u4VirAdr;
   u32 u4ChNum;
   u32 u4ChSz;
   u32 u4Bps;
   u32 u4IntBankNum;
   u32 u4IntBankSampleNum;//sample
   u32 u4IntBankSz;  //Byte
}AOUT_TEST_CFG_T, *PAOUT_TEST_CFG_T;

PAOUT_HAL_CLS_PUB prAout = NULL;
AOUT_TEST_CFG_T rTestCfg;


void AudAoutTestIsr(u32 u4AoutId)
{
    u32 u4Rp, u4Wp;

    if (NULL == prAout)
    {
        AOUTLOG_ERR(T("AudAoutTestIsr Aout Obj IS NULL !!!  \r\n"));
        return;        
    }
    
    u4Rp = prAout->rHwIf.GetPoint(prAout);
    
    if (u4Rp > rTestCfg.u4IntBankSz){
        u4Wp = u4Rp - rTestCfg.u4IntBankSz;
    }else{
        u4Wp = rTestCfg.u4ChSz - rTestCfg.u4IntBankSz;
    }

    prAout->rHwIf.SetPoint(prAout, u4Wp);

   // AOUTLOG_TEST(T("AudAoutTestIsr u4Rp(0x%x), u4Wp(0x%x) \r\n"), u4Rp, u4Wp);
}

void AudAoutTestStart(u32 u4AoutId, AUD_OUT_PATH_T eAoutPath)
{
    AOUT_EXTPARAMS_T rExtCfg;

    u32 u4Tmp1, u4Tmp2, u4Tmp3, u4Tmp4, u4Tmp5;
    u8 *prBuf;

    AOUTLOG_TEST(T("AudAoutTestStart aout(%d), Seat(%d) \r\n"), (s32)u4AoutId, eAoutPath);
    
    if (NULL == prAout)
    {
        prAout = AoutHal_New((0 == u4AoutId) ? AUDID_AOUT1 : AUDID_AOUT2);

        if (NULL == prAout)
        {
            AOUTLOG_TEST(T("AudAoutTestStart New Aout Obj Fail !!!  \r\n"));
            return;
        }

        rTestCfg.u4Bps = 24;
        rTestCfg.u4ChNum = 2;
        rTestCfg.u4IntBankNum = 4;
        rTestCfg.u4IntBankSampleNum = 0xF0;
        rTestCfg.u4IntBankSz = rTestCfg.u4IntBankSampleNum * rTestCfg.u4Bps / 8;
        rTestCfg.u4ChSz = rTestCfg.u4IntBankSz * rTestCfg.u4IntBankNum;
        rTestCfg.u4BufSz = rTestCfg.u4ChSz * rTestCfg.u4ChNum; 
        rTestCfg.u4VirAdr = AudOS_Memory_Alloc(rTestCfg.u4BufSz, 4, &rTestCfg.u4PhyAdr);

        prBuf = (u8 *)rTestCfg.u4VirAdr;

        //memory init
        for (u4Tmp5 = 0; u4Tmp5 < rTestCfg.u4ChNum; u4Tmp5 ++)
        {
            u4Tmp2 = 0;
            u4Tmp3 = 0x2000;
            
            for (u4Tmp1 = 0; u4Tmp1 < rTestCfg.u4IntBankNum; u4Tmp1 ++)
            {
                for (u4Tmp4 = 0; u4Tmp4 < rTestCfg.u4IntBankSampleNum/2; u4Tmp4 ++)
                {
                    u4Tmp2 += u4Tmp3;
                    *prBuf++ = u4Tmp2 & 0xff;
                    *prBuf++ = (u4Tmp2 & 0xff00) >> 8;
                    *prBuf++ = (u4Tmp2 & 0xff0000) >> 16;
                }

                for (u4Tmp4 = 0; u4Tmp4 < rTestCfg.u4IntBankSampleNum/2; u4Tmp4 ++)
                {
                    u4Tmp2 -= u4Tmp3;
                    *prBuf++ = u4Tmp2 & 0xff;
                    *prBuf++ = (u4Tmp2 & 0xff00) >> 8;
                    *prBuf++ = (u4Tmp2 & 0xff0000) >> 16;
                }

                u4Tmp3 += 0x2000;
            }
        }

        rExtCfg.eOutPath = eAoutPath;
        rExtCfg.fgAdcBypasMode = FALSE;
        rExtCfg.eFs = FS_48K;
        rExtCfg.u4Bps = rTestCfg.u4Bps;

        rExtCfg.u4ChCfg0 = 0xFF2100;
        rExtCfg.u4ChCfg1 = 0xFFFFFF;
        rExtCfg.u4ChCfg2 = 0xFFFFFF;
        
        rExtCfg.u4ChNum = rTestCfg.u4ChNum;   
        rExtCfg.u4BufPhyAdr = rTestCfg.u4PhyAdr;
        rExtCfg.u4BufSz = rTestCfg.u4BufSz;

        rExtCfg.rIntCfg.u4NSNum = rTestCfg.u4IntBankSampleNum;
        rExtCfg.rIntCfg.u4IntrSize = rTestCfg.u4IntBankSampleNum / 2;
        rExtCfg.rIntCfg.PFN_ISR_CB = AudAoutTestIsr;
        
        prAout->rHwIf.Setup(prAout, &rExtCfg);
    }

    prAout->rHwIf.Start(prAout, 0);
}

void AudAoutTestStop(void)
{
    AOUTLOG_TEST(T("AudAoutTestStop \r\n"));
    
    if (NULL != prAout)
    {
        prAout->rHwIf.Stop(prAout, 0);
    }
}

void AudAoutTest(u32 arg1, u32 arg2, u32 arg3)
{
    if (0 == arg1)
    {
       AudAoutTestStart(arg2, arg3);
    }
    else if (1 == arg1)
    {
       AudAoutTestStop();
    }
}

