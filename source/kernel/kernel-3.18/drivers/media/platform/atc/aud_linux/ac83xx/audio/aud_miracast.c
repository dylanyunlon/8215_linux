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


/*-----------------------------------------------------------------------------
Include header files
-----------------------------------------------------------------------------*/
#include <linux/types.h>
#include "aud_debug.h"
#include "aud_drv.h"
#include "DspShm.h"
#include "drv_thread.h"
#include "aud_clock.h"
#include "DspFunc.h"
#ifdef __linux__
#include "winutil.h"
#include "drv_win32_if.h"
#endif
#include <media/atc/mm_common.h>


typedef enum
{
    Miracast_Normal,
    Miracast_AdjApll,
    Miracast_SkipData,
    Max_Status
}AdjMir_Status_E;



// Global variabl

static struct task_struct *g_hAudMiracast = NULL;
static struct task_struct *g_hThreadStopEvent = NULL;
static bool     _fgAudMiraState = FALSE;
static bool _fgMiraDspPtsSet = FALSE;
static bool _fgMiraStatus = FALSE;


s64 g_i8HighThrehold =90000;//1000*90 stc value to ms 
s64 g_i8LowThrehold  =36000;//400*90 

u32 g_u4AdjScale = 0xA7C56; //0x218DE;
u16 g_u2SleepTime = 1000;

extern u64 g_u8Pts;
extern u64 g_u8STC;

static s32 AUD_Miracast_Thread(void* pvArg)
{
    u64 u8AudPts = 0;
    u64 u8AudSTC = 0;    
    s64  i8STC2PTSLen = 0;
    s64  i8OldDiffLen = 0;
    AdjMir_Status_E eMiraStatus = Miracast_Normal;
    u16 u2ApllCount = 0;
        
    while(_fgAudMiraState)
    {
        u8AudPts = g_u8Pts;    
        u8AudSTC = g_u8STC;      
        
        i8OldDiffLen = i8STC2PTSLen;        
        i8STC2PTSLen = u8AudPts - u8AudSTC;
        
        if((_fgMiraDspPtsSet ==FALSE)||(u8AudPts == INVALID_TIMESTAMP)||
            (u8AudSTC == INVALID_TIMESTAMP)||((u8AudPts == 0)&&(i8STC2PTSLen < 0))) //Only audio
        {
            LOG(LOG_FAIL, TEXT("Not playing video or STC haven't Update.\r\n"));
        }
        else
        {            
            //detect the afifo data size
            if((i8STC2PTSLen > g_i8LowThrehold)&&(i8STC2PTSLen < g_i8HighThrehold))
            {
                //adjust APll
                if(eMiraStatus == Miracast_Normal)
                {                
                    Aud_ApllDirectAdjust(AUD_APLL_ADJ_UP,g_u4AdjScale);
                    u2ApllCount++;
                    eMiraStatus = Miracast_AdjApll;
                }
                else if(Miracast_SkipData == eMiraStatus)
                {
                    DspSetMiracast_skipDataFlag(0);
                    Aud_ApllDirectAdjust(AUD_APLL_ADJ_UP,g_u4AdjScale);
                    u2ApllCount++;
                    eMiraStatus = Miracast_AdjApll;                
                }
                else if(eMiraStatus == Miracast_AdjApll)
                {
                     if((i8STC2PTSLen >= i8OldDiffLen)&&(u2ApllCount < 3))
                     {
                         Aud_ApllDirectAdjust(AUD_APLL_ADJ_UP,g_u4AdjScale);                     
                         u2ApllCount++;
                         LOG(LOG_FAIL, TEXT("After Adjust Apll,Old PtsStc diff = 0x%x.\r\n"), (u32)i8OldDiffLen);
                         LOG(LOG_FAIL, TEXT("New PtsStc diff = 0x%x."), (u32)i8STC2PTSLen);
                     }
                }
            }
            else if(i8STC2PTSLen >=g_i8HighThrehold)
            {   
                //skip data
                if(eMiraStatus == Miracast_Normal)
                {
                    DspSetMiracast_skipDataFlag(0x100);
                    eMiraStatus = Miracast_SkipData;
                }
                else if(Miracast_AdjApll == eMiraStatus)
                {                
                    DspSetMiracast_skipDataFlag(0x100);
                    eMiraStatus = Miracast_SkipData;
                    Aud_ApllDirectAdjust(AUD_APLL_ADJ_NORMAL,0);
                    u2ApllCount = 0;
                }
            }
            else if(i8STC2PTSLen <= g_i8LowThrehold)
            {
                if(Miracast_AdjApll == eMiraStatus)
                { 
                    Aud_ApllDirectAdjust(AUD_APLL_ADJ_NORMAL,0);
                    u2ApllCount = 0;
                    eMiraStatus = Miracast_Normal;                   
                                    
                }
                else if(Miracast_SkipData == eMiraStatus)
                {                
                    DspSetMiracast_skipDataFlag(0);
                    eMiraStatus = Miracast_Normal;
                }
            }
        }
            
        LOG(LOG_DATAF, TEXT("STC val= %d.\r\n"), (u32)u8AudSTC);            
        LOG(LOG_DATAF, TEXT("PTS val= %d.\r\n"), (u32)u8AudPts);
        LOG(LOG_DATAF, TEXT("PTS - STC = %d.\r\n"), (u32)i8STC2PTSLen);

        LOG(LOG_DATAF, TEXT("STC time= %d.\r\n"), (u32)(u8AudSTC / 90));            
        LOG(LOG_DATAF, TEXT("PTS time= %d.\r\n"), (u32)(u8AudPts / 90));
        LOG(LOG_DATAF, TEXT("PTS - STC time = %d.\r\n"), (u32)(i8STC2PTSLen/90));

        LOG(LOG_DATAF, TEXT("Adjust Apll Size = %d(0.5/unit).\r\n"), u2ApllCount);
        Sleep(g_u2SleepTime);
    }
    Aud_ApllDirectAdjust(AUD_APLL_ADJ_NORMAL,0);
    DspSetMiracast_skipDataFlag(0);
    x_event_set(g_hThreadStopEvent);
    complete_and_exit(NULL, 0);       


    return 0;
}

void vAudDrvIf_SetMiracastOnOff(AUD_MIRACAST_CTRL_T eMiracastCtrl)
{
    u32 dwObject = 0;

    //set aac check frame size to 1.
    //set avsync restart when bistream underrun.
    switch (eMiracastCtrl)
    {
    case AUD_MIRACAST_OFF:
        {
            if (!_fgMiraStatus)
            {
                break;
            }
            _fgMiraStatus = FALSE;
            LOG(LOG_CTRLF, TEXT("[AUD] Set Miracast off\n"));
            _fgAudMiraState = FALSE;
            
            //wait detect thread exit, close the thread handle.
            dwObject = x_event_wait_for_objects(1, &g_hThreadStopEvent, FALSE, 2000);
            if(dwObject!= WAIT_OBJECT_0)
            {
                LOG(LOG_FAIL, TEXT("Aud_micracast:Wait MiraThdStopEvent Failed.\r\n"));
            }
            else
            {
                LOG(LOG_CTRLF, TEXT("Aud_micracast:Wait MiraThdStopEvent ok.\n"));
            }
            if (g_hThreadStopEvent) 
            {
                x_event_destroy(g_hThreadStopEvent);                
            }
            g_hThreadStopEvent = NULL;                
            g_hAudMiracast = NULL;
            vMira_SetDspPtsUpdate(FALSE);

            vWriteDspShmBYTE(B_AAC_CHK_FRM_NUM,1);
        }
        break;

    case AUD_MIRACAST_ON:
        {
            if (_fgMiraStatus)
            {
                break;
            }
            _fgMiraStatus = TRUE;
            LOG(LOG_CTRLF, TEXT("[AUD] Set Miracast on...\n"));
            
            _fgAudMiraState = TRUE;            
            
            //Create Miracast detect thread.  
            g_hAudMiracast = kthread_create(AUD_Miracast_Thread, (void *)NULL, MIRACAST_THREAD_NAME);
        	if (IS_ERR(g_hAudMiracast)) {
        		LOG(LOG_CTRLF, TEXT("[AUD_MIRACAST_ON]AUD_Miracast_Thread create fail \r\n"));
        		g_hAudMiracast = NULL;
        		return;
            }
            else
            {
                struct sched_param param;
                s32 ret;

                param.sched_priority = to_sched_priority(MIRACAST_THREAD_PRIORITY);
                ret = sched_setscheduler_nocheck(g_hAudMiracast, SCHED_RR, &param);
                ASSERT(ret == 0);
            }
        	wake_up_process(g_hAudMiracast);

        	g_hThreadStopEvent = x_event_create(NULL, false, false, TEXT("MiraThdStopEvent"));
            if (!g_hThreadStopEvent)
            {
                LOG(LOG_FAIL, TEXT("Aud_micracast:Create MiraThdStopEvent Failed.\r\n"));
                g_hThreadStopEvent = NULL;
            }
            
            vWriteDspShmBYTE(B_AAC_CHK_FRM_NUM,0);
        }
        break;

    default:
        LOG(LOG_CTRLF, TEXT("[AUD] Set Miracast Coeff error.\n"));
        break;
    }
}


void vMira_SetHighThreshold(s64 i8Threshold)
{
    g_i8HighThrehold = i8Threshold * 90;
}

s64 i8Mira_GetHighThreshold()
{
    return (g_i8HighThrehold / 90);
}


void vMira_SetLowThreshold(s64 i8Threshold)
{
    g_i8LowThrehold = i8Threshold * 90;
}

s64 i8Mira_GetLowThreshold()
{
    return (g_i8LowThrehold / 90);
}

void vMira_SetApllScale(s16 i2Scale)
{
    const u32 u4ApllFactor = 0x218DE; //apll adjust 0.1%.
    g_u4AdjScale = i2Scale*u4ApllFactor;
}

s16 i2Mira_GetApllScale()
{
    const u32 u4ApllFactor = 0x218DE; //apll adjust 0.1%.
    s16 i2Scale = g_u4AdjScale /u4ApllFactor;
    return (i2Scale);
}

void vMira_SetSleepTime(u16 u2Time)
{
    g_u2SleepTime = u2Time;    
}


u16 u2Mira_GetSleepTime()
{
    return (g_u2SleepTime);
}

void vMira_SetDspPtsUpdate(bool fgPtsSet)
{
    _fgMiraDspPtsSet = fgPtsSet;    
}


void vMira_SetAdjustParam(AUD_MIRACAST_PARAM_T rMiraParam)
{
    AUD_MIRACAST_PARAM_E eParam = rMiraParam.e_ParamID;
    LOG(LOG_FEATURE, TEXT("Miracast Ctrl:%d.\r\n"), eParam); 

    switch(eParam)
    {
    case AUD_MIRA_LOWTH:
        g_i8LowThrehold = rMiraParam.uVal.i8LowThVal;
        LOG(LOG_FEATURE, TEXT("Miracast Param Value:%d.\r\n"), (u32)g_i8LowThrehold); 
        break;
    case AUD_MIRA_LIGHTH:
        g_i8HighThrehold = rMiraParam.uVal.i8HighThVal;
        LOG(LOG_FEATURE, TEXT("Miracast Param Value:%d.\r\n"), (u32)g_i8HighThrehold); 
        break;
    case AUD_MIRA_ADJSIZE:
        g_u4AdjScale = rMiraParam.uVal.u4AdjustSize;
        LOG(LOG_FEATURE, TEXT("Miracast Param Value:%d.\r\n"), g_u4AdjScale);
        break;
    case AUD_MIRA_SLEEPTIME:
        g_u2SleepTime = rMiraParam.uVal.u2SleepTime;
        LOG(LOG_FEATURE, TEXT("Miracast Param Value:%d.\r\n"), g_u2SleepTime);
        break;
    default:
        break;
    }
}


//add end







