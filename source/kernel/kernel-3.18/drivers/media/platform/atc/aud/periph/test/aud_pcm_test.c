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
 * @file aud_pcm_test.c source file
 * 
 * aud io pcm module test function
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */

#include "aud_pcm_hal_if.h"

PPCM_HAL_CLS_PUB prPcm = NULL;
u32 u4PcmIntCnt = 0;


void AudPcmTestTxIsr(u32 u4Param)
{
    //PCMLOG_INFO(T(" AudPcmTestTxIsr Run \r\n"));
    u4PcmIntCnt ++;
}


void AudPcmTestStart(u32 u4HwMode)
{
    PCM_EXTPARAMS_T rExtCfg;
    AUD_DATA_BUF_T rDataBuf;
    u16 *prAddr;
    u16 u2Val, u2DataNum;

    PCMLOG_INFO(T("AudPcmTestStart \r\n"));
    
    if (NULL == prPcm)
    {   
        //creat pcm hal obj
        prPcm = PcmHal_New();

        //config pcm hal
        if (0 == u4HwMode)
        {
            PCMLOG_INFO(T(" AudPcmTest : Normal Mode \r\n"));

            rExtCfg.eHwMode = PCM_NORMAL_MODE;
        }else
        {
            PCMLOG_INFO(T(" AudPcmTest : Loop Mode \r\n"));
            
            rExtCfg.eHwMode = PCM_LOOP_MODE;
        }

        rExtCfg.eSyncMode = PCM_LONG_MODE;
        rExtCfg.eSyncCycle = PCM_CLK_CYCLE_32;
        
        rExtCfg.rIntCfg.PFN_ISR_CB = AudPcmTestTxIsr;
        rExtCfg.rIntCfg.u4IntSz = 200;
        rExtCfg.rIntCfg.u4IntBurstTime = 100;

        rExtCfg.u4BufPhyAdr = 0;
        rExtCfg.u4RxBufSz = 200 * 8;
        rExtCfg.u4TxBufSz = 200 * 4;

        prPcm->rRxHwIf.Setup(prPcm, &rExtCfg);

        //init tx buffer
        prPcm->rTxHwIf.GetBuf(prPcm, &rDataBuf);

        prAddr = (u16 *)(rDataBuf.u4VirSAdr);
        u2DataNum = (u16)(rExtCfg.rIntCfg.u4IntSz * 2);

        for (u2Val = 0; u2Val < u2DataNum; u2Val ++)
        {
            *prAddr = u2Val;
            prAddr ++;
        }
    }

    prPcm->PcmCtrl(prPcm, TRUE);
    prPcm->rTxHwIf.Start(prPcm, 0);
    prPcm->rRxHwIf.Start(prPcm, 0);
}

void AudPcmTestStop(void)
{
    PCMLOG_INFO(T("AudPcmTestStop \r\n"));
    
    if (NULL != prPcm)
    {
        prPcm->rTxHwIf.Stop(prPcm, 0);
        prPcm->rRxHwIf.Stop(prPcm, 0);
        prPcm->PcmCtrl(prPcm, FALSE);
    }
}

void AudPcmTest(u32 arg1, u32 arg2, u32 arg3)
{
    if (0 == arg1)
    {
       AudPcmTestStart(arg2);
    }
    else if (1 == arg1)
    {
       AudPcmTestStop();
    }
}

