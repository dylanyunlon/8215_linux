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
 * @file aud_io_test.c source file
 * 
 * aud io module test function
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */

#include "drv_thread.h"
#include "aud_oal.h"

#include "aud_io_test.h"
#include <linux/semaphore.h>


//==========================================================//
    #define CodeSight_MicToAout_Test
//==========================================================//

static MIC_TO_AOUT_TEST_T _rMicToAout = {NULL, NULL};
  

static void MicToAout_CB(u32 u4AoutId)
{
    if (_rMicToAout.hSema) {
        up(_rMicToAout.hSema);
    }   
}

static void MicToAout_Open(void)
{
    COMMLOG_CLI(T("MicToAout_Open. \r\n"));
    
    if (NULL == _rMicToAout.prMic)
    {
        MIC_EXTPARAMS_T rExtCfg;
        _rMicToAout.prMic = MicHal_New();
        
        rExtCfg.eSrc = INT_MICIN;
        rExtCfg.eFs = FS_48K;
        rExtCfg.u4SrcBitNum = 16;
        rExtCfg.u4MicGain = 14;
        rExtCfg.eOutBitNum = LIN_16;
        rExtCfg.u4BufPhyAdr = 0;
        rExtCfg.u4BufSz = 32000;

        if (_rMicToAout.prMic) {
            _rMicToAout.prMic->rHwIf.Setup(_rMicToAout.prMic, &rExtCfg);
        }
    }

    if (NULL == _rMicToAout.prAout)
    {
        AOUT_EXTPARAMS_T rExtCfg;
        _rMicToAout.prAout = AoutHal_New(AUDID_AOUT2);
        if (NULL == _rMicToAout.prAout)
        {
            COMMLOG_CLI(T("MicToAout_Open New Aout Obj Fail !!!  \r\n"));
            return;
        }

        rExtCfg.eOutPath = AOUT_RS;
        rExtCfg.eFs = FS_48K;
        rExtCfg.u4Bps = 24;

        rExtCfg.u4ChCfg0 = 0xFF2100;
        rExtCfg.u4ChCfg1 = 0xFFFFFF;
        rExtCfg.u4ChCfg2 = 0xFFFFFF;
        
        rExtCfg.u4ChNum = 2;   
        rExtCfg.u4BufPhyAdr = 0;
        rExtCfg.u4BufSz = AOUT_INT_SAMPLE_NUM * AOUT_INT_BANK_NUM * 3 * rExtCfg.u4ChNum;  // 3 sample->bytes

        rExtCfg.rIntCfg.u4NSNum = 0xF0; 
        rExtCfg.rIntCfg.u4IntrSize = rExtCfg.rIntCfg.u4NSNum >> 1;
        rExtCfg.rIntCfg.PFN_ISR_CB = MicToAout_CB;
        
        if (_rMicToAout.prAout) {
            _rMicToAout.prAout->rHwIf.Setup(_rMicToAout.prAout, &rExtCfg);
        }
    }
}


static void MicToAout_Close(void)
{   
    if (_rMicToAout.prMic)
    {
        _rMicToAout.prMic->Delete(_rMicToAout.prMic);
        _rMicToAout.prMic = NULL;
    }

    if (_rMicToAout.prAout)
    {
         _rMicToAout.prAout->Delete(_rMicToAout.prAout);
        _rMicToAout.prAout = NULL;
    }
}

static void MicToAout_Start(void)
{
    PMIC_HAL_CLS_PUB prMic = _rMicToAout.prMic;
    PAOUT_HAL_CLS_PUB prAout = _rMicToAout.prAout;
    u32 u4MicWP = 0;
    
    if (!prMic || !prAout) {
        MicToAout_Open();
    }

    if (prMic && prAout)
    {
        _rMicToAout.fgThreadEn = TRUE;
        _rMicToAout.hSema = (struct semaphore *)kmalloc(sizeof(struct semaphore), GFP_KERNEL);
        sema_init(_rMicToAout.hSema, 1);
        #ifndef __linux__
        _rMicToAout.hThread = CreateThread(NULL, 0, MicToAout_Thread, &_rMicToAout, 0, NULL);
        #else
        //temp close for warning cancel
        ////s8 t_name[20] = "Mic2Aout_Test";
        //VERIFY(x_thread_create(&_rMicToAout.hThread, t_name, AUD_DRV_THREAD_STACK_SIZE,
                //AUD_DRV_THREAD_PRIORITY, MicToAout_Thread, 0, &_rMicToAout) == OSR_OK);        
        #endif
    
        prMic->rHwIf.Start(prMic, 0);
        prMic->rHwIf.GetBuf(prMic, &_rMicToAout.rMicBuf);
        _rMicToAout.u4MicRP = prMic->rHwIf.GetPoint(prMic);    
        while (u4MicWP < (AOUT_INT_SAMPLE_NUM << 2))  //4: 2 bytes per sample and 2 banks 
        {
            u4MicWP = prMic->rHwIf.GetPoint(prMic);
        }

        prAout->rHwIf.Start(prAout, 0);
        prAout->rHwIf.GetBuf(prAout, &_rMicToAout.rAoutBuf);
    }
}


static void MicToAout_Stop(void)
{    
    _rMicToAout.prMic->rHwIf.Stop(_rMicToAout.prMic, 0);
    _rMicToAout.prAout->rHwIf.Stop(_rMicToAout.prAout, 0);

    kfree(_rMicToAout.hSema);
    _rMicToAout.fgThreadEn = FALSE;
    MicToAout_CB(0);
}


static void MicToAout_TestCmd(u32 u4Arg1, u32 u4Arg2)
{
    COMMLOG_CLI(T("MicToAout_TestCmd: 0x%x, 0x%x \n"), (u32)u4Arg1, (u32)u4Arg2);
    
    switch(u4Arg1)
    {
    case 0:
        MicToAout_Open();
        break;

    case 1:
        MicToAout_Start();
        break;

    case 2:
        MicToAout_Stop();
        break;

    case 3:
        MicToAout_Close();
        break;

    default:
        break;
    }

}

//==========================================================//
    #define CodeSight_IOTest_Cmd
//==========================================================//

void AudIOTest(u32 u4TestType, u32 u4Arg1, u32 u4Arg2)
{
    switch(u4TestType)
    {
    case 0:
       MicToAout_TestCmd(u4Arg1, u4Arg2);
       break;

    default:
        break;
    }
}

