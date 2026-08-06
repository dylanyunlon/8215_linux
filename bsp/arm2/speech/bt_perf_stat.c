/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

/******************************************************************************
*[File]             perfstat.c
*[Version]          v0.1
*[Revision Date]    2010-06-19
*[Author]           Zeng Zhang
*[Description]
*    source file for performance stat.
*
*
******************************************************************************/
#include "bt_osl.h"
#include "bt_perf_stat.h"


#if (ENABLE_PERFORMANCE_STAT)

extern BOOL g_u4SphLog;

typedef struct _strucStatItem {
    MCHAR   szName[40];          // Name of funcitons
    UINT32  u4Count;             // Total invoked count of the funciotn
    UINT64  u8LastStart;         // Start time
    UINT64  u8TotalTime;         // Total time (us)
    UINT32  u4MaxTime;           // Max time (us) 
    UINT32  u4MinTime;           // Minimal time (us)
    UINT32  u4OverCount;         // Count of time over the limitation.
    UINT32  u4OverLimit;         // Time of limitation
}STATITEM_T,* PSTATITEM_T;


static STATITEM_T _prStatItem[] = 
{ 
    {T("AEC+NDC"), 0, 0, 0, 0, 1000000, 0, AEC_NDC_LIMITATION},
    {T("UL     "), 0, 0, 0, 0, 1000000, 0, UL_LIMITATION},
    {T("DL     "), 0, 0, 0, 0, 1000000, 0, DL_LIMITATION},
    {T("UL AEC "), 0, 0, 0, 0, 1000000, 0, UL_AEC_LIMITATION},
    {T("UL NDC "), 0, 0, 0, 0, 1000000, 0, UL_NDC_LIMITATION},
    {T("DL AEC "), 0, 0, 0, 0, 1000000, 0, DL_AEC_LIMITATION },
    {T("DL NDC "), 0, 0, 0, 0, 1000000, 0, DL_NDC_LIMITATION },
    {T("DL PLC "), 0, 0, 0, 0, 1000000, 0, DL_PLC_LIMITATION },
};

static UINT64 _u8HighPerFrequency = 1000;


BOOL TimeStatInit()
{
    UINT32 i, u4Temp;

    _u8HighPerFrequency = BTGetSysFrequery();
    u4Temp = (UINT32)(_u8HighPerFrequency / 1000);
    for ( i = 0; i < STAT_IDX_MAX; i++)
    {
        _prStatItem[i].u4Count = 0;
        _prStatItem[i].u8LastStart = 0;
        _prStatItem[i].u8TotalTime = 0;
        _prStatItem[i].u4MaxTime = 0;
        _prStatItem[i].u4MinTime = 0xFFFFFFFF;
        _prStatItem[i].u4OverCount = 0;
        _prStatItem[i].u4OverLimit /= 1000;
        _prStatItem[i].u4OverLimit *= u4Temp;
    }

    return (TRUE);
}


BOOL TimeStatUnInit()
{
    UINT32 i, u4Temp;

    TimeStatOutput();
    u4Temp = (UINT32) (_u8HighPerFrequency / 1000);
    if (0 == u4Temp)
    {
        SPHLOG(1, (T("TimeStatUnInit error: dwTemp = 0\r\n")));
        return (FALSE);
    }
    for ( i = 0; i < STAT_IDX_MAX; i++)
    {
        _prStatItem[i].u4OverLimit /= u4Temp;
        _prStatItem[i].u4OverLimit *= 1000;
    }

    return (TRUE);
}


BOOL TimeStatOutput(VOID)
{
    double dUsPerFrequency;
    UINT64 u8TotalTime;
    UINT32 i, u4MaxTime, u4MinTime;

    SPHLOG_INFO((T("[TimeStatOutput] Start >>>>>>>>>>>>>>>>>>>>>>> \r\n")));
    SPHLOG_INFO((T("The frequency of the high performance clock: %d ticks/Sec \r\n"), (UINT32)_u8HighPerFrequency));

    _prStatItem[STAT_IDX_AEC_NDC].u4MaxTime = _prStatItem[STAT_IDX_DL].u4MaxTime + _prStatItem[STAT_IDX_UL].u4MaxTime;
    dUsPerFrequency = (double)_u8HighPerFrequency;
    dUsPerFrequency = MICROSECOND / dUsPerFrequency;
    
    SPHLOG_INFO((T("   Name  , Max(us) , Avg(us)  , Min(us) , Total(us)  , Count   , OverCount \r\n")));
    for ( i = 0; i < STAT_IDX_MAX; i++)
    {
        u8TotalTime = (UINT64)(_prStatItem[i].u8TotalTime * dUsPerFrequency);
        u4MaxTime   = (UINT32)(_prStatItem[i].u4MaxTime   * dUsPerFrequency);
        u4MinTime   = (UINT32)(_prStatItem[i].u4MinTime   * dUsPerFrequency);
        if (_prStatItem[i].u4Count)
        {
            SPHLOG_INFO((T(" %s , %8d , %8d , %8d , %10d , %8d , %8d \r\n"), 
                _prStatItem[i].szName, 
                u4MaxTime,
                (UINT32)(u8TotalTime / _prStatItem[i].u4Count),
                u4MinTime,
                (UINT32)u8TotalTime,
                _prStatItem[i].u4Count,
                _prStatItem[i].u4OverCount));
        }
    }
    SPHLOG_INFO((T("[TimeStatOutput] End <<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n")));

#if ENABLE_CSV_OUTPUT
{
    FILE *fp = fopen("\\windows\\perf.csv" ,"wb" ); 
    if (fp)
    {
        char cBuffer[256];
        sprintf(cBuffer, "    Name, Count  , Total(us)  , Avg(us), Max(us), Min(us), OverCount \r\n");
        fwrite(cBuffer, 1, strlen(cBuffer), fp);
        for ( i = 0; i < STAT_IDX_MAX; i++)
        {
            char szName[50];
            wcstombs(szName, _prStatItem[i].szName, 50);
            if (_prStatItem[i].u4Count)
            {
                sprintf(cBuffer,"%8s,%8d,%12I64u,%8d,%8d,%8d, %d\r\n", 
                    szName, 
                    _prStatItem[i].u4Count,
                    _prStatItem[i].u8TotalTime,
                    (UINT32)(_prStatItem[i].u8TotalTime/_prStatItem[i].u4Count),
                    _prStatItem[i].u4MaxTime,
                    _prStatItem[i].u4MinTime,
                    _prStatItem[i].u4OverCount );
            }   
            fwrite(cBuffer, 1, strlen(cBuffer), fp);

        }
        fclose(fp);
        SPHLOG_INFO((T("[TimeStatOutput] output to file perf.csv\r\n")));
    }
}
#endif
    return (TRUE);
}


VOID TimeStatEnter(UINT32 u4Index)
{
    _prStatItem[u4Index].u8LastStart = BTGetSysTick();
}


VOID TimeStatPause(UINT32 u4Index)
{
    _prStatItem[u4Index].u8LastStart = BTGetSysTick() - _prStatItem[u4Index].u8LastStart;
}


VOID TimeStatResume(UINT32 u4Index)
{
    _prStatItem[u4Index].u8LastStart = BTGetSysTick() - _prStatItem[u4Index].u8LastStart;
}


VOID TimeStatLeave(UINT32 u4Index)
{
    UINT64 u8Temp = BTGetSysTick() - _prStatItem[u4Index].u8LastStart;
    if (u8Temp > _prStatItem[u4Index].u4MaxTime)
    {
        _prStatItem[u4Index].u4MaxTime = (UINT32)u8Temp;
    }
    if (u8Temp <  _prStatItem[u4Index].u4MinTime)
    {
        _prStatItem[u4Index].u4MinTime = (UINT32)u8Temp;
    }
    if (u8Temp >  _prStatItem[u4Index].u4OverLimit)
    {
        _prStatItem[u4Index].u4OverCount ++;
    }
    _prStatItem[u4Index].u8TotalTime += u8Temp;
    _prStatItem[u4Index].u4Count ++;
    _prStatItem[u4Index].u8LastStart = 0;
}

#endif



