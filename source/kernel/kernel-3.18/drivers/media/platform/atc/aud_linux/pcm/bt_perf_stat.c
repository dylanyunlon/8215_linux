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

/******************************************************************************
*
*[Description]
*	 source file for performance stat.
*
******************************************************************************/
#include "bt_osl.h"
#include "bt_perf_stat.h"
#include "aud_pcm_dbg.h"

#include "pcm_debug.h"
#define LOG_TAG "bt_perf"

#if (ENABLE_PERFORMANCE_STAT)

typedef struct _strucStatItem {
	char szName[40];			/* Name of funcitons */
	u32	u4Count;			/* Total invoked count of the funciotn */
	u64	u8LastStart;		/* Start time */
	u64	u8TotalTime;		/* Total time (us) */
	u32	u4MaxTime;			/* Max time (us) */
	u32	u4MinTime;			/* Minimal time (us) */
	u32	u4OverCount;		/* Count of time over the limitation. */
	u32	u4OverLimit;		/* Time of limitation */
} STATITEM_T, *PSTATITEM_T;


static STATITEM_T _prStatItem[] = {
	{TEXT("AEC+NDC"), 0, 0, 0, 0, 1000000, 0, AEC_NDC_LIMITATION},
	{TEXT("UL	  "), 0, 0, 0, 0, 1000000, 0, UL_LIMITATION},
	{TEXT("DL	  "), 0, 0, 0, 0, 1000000, 0, DL_LIMITATION},
	{TEXT("UL AEC "), 0, 0, 0, 0, 1000000, 0, UL_AEC_LIMITATION},
	{TEXT("UL NDC "), 0, 0, 0, 0, 1000000, 0, UL_NDC_LIMITATION},
	{TEXT("DL AEC "), 0, 0, 0, 0, 1000000, 0, DL_AEC_LIMITATION },
	{TEXT("DL NDC "), 0, 0, 0, 0, 1000000, 0, DL_NDC_LIMITATION },
	{TEXT("DL PLC "), 0, 0, 0, 0, 1000000, 0, DL_PLC_LIMITATION },
};

static u64 _u8HighPerFrequency = 1000U;


bool TimeStatInit(void)
{
	u32 i, u4Temp;

	_u8HighPerFrequency = BTGetSysFrequency();
	u4Temp = (u32)(_u8HighPerFrequency / 1000U);
	for (i = 0; i < STAT_IDX_MAX; i++) {
		_prStatItem[i].u4Count = 0;
		_prStatItem[i].u8LastStart = 0;
		_prStatItem[i].u8TotalTime = 0;
		_prStatItem[i].u4MaxTime = 0;
		_prStatItem[i].u4MinTime = 0xFFFFFFFF;
		_prStatItem[i].u4OverCount = 0;
		_prStatItem[i].u4OverLimit /= 1000;
		_prStatItem[i].u4OverLimit *= u4Temp;
	}

	return true;
}

bool TimeStatUnInit(void)
{
	u32 i, u4Temp;

	TimeStatOutput();
	u4Temp = (u32) (_u8HighPerFrequency / 1000U);
	if (0 == u4Temp) {
		PCM_ERROR(LOG_TAG, "TimeStatUnInit: error dwTemp = 0\r\n");
		return false;
	}
	for (i = 0; i < STAT_IDX_MAX; i++) {
		_prStatItem[i].u4OverLimit /= u4Temp;
		_prStatItem[i].u4OverLimit *= 1000;
	}

	return true;
}

bool TimeStatOutput(void)
{
	double dUsPerFrequency;
	u64 u8TotalTime;
	u32 i, u4MaxTime, u4MinTime;

	PCM_DEBUG(LOG_TAG, "TimeStatOutput Start >>>>>>>>>>>>>>>>>>>>>>>\r\n");
	PCM_DEBUG(LOG_TAG, "The frequency of the high performance clock: %d ticks/Sec\r\n",
		(s32)_u8HighPerFrequency);
	_prStatItem[STAT_IDX_AEC_NDC].u4MaxTime = _prStatItem[STAT_IDX_DL].u4MaxTime
		+ _prStatItem[STAT_IDX_UL].u4MaxTime;
	_prStatItem[STAT_IDX_AEC_NDC].u4MinTime = _prStatItem[STAT_IDX_DL].u4MinTime
		+ _prStatItem[STAT_IDX_UL].u4MinTime;
	dUsPerFrequency = (double)_u8HighPerFrequency;
	dUsPerFrequency = (double)MICROSECOND / dUsPerFrequency;

	PCM_DEBUG(LOG_TAG, "Name,  Max(us),  Avg(us),  Min(us),  Total(us),  Count, OverCount\r\n");
	for (i = 0; i < STAT_IDX_MAX; i++) {
		u8TotalTime = (u64)(_prStatItem[i].u8TotalTime * dUsPerFrequency);
		u4MaxTime	= (u32)(_prStatItem[i].u4MaxTime   * dUsPerFrequency);
		u4MinTime	= (u32)(_prStatItem[i].u4MinTime   * dUsPerFrequency);
		if (_prStatItem[i].u4Count) {
			s32 i4Avg = 0;

			if (STAT_IDX_AEC_NDC == i) {
				if (_prStatItem[STAT_IDX_UL].u4Count)
					i4Avg = (s32)(((u64)(_prStatItem[STAT_IDX_UL].u8TotalTime
						* dUsPerFrequency)) / _prStatItem[STAT_IDX_UL].u4Count);
				if (_prStatItem[STAT_IDX_DL].u4Count)
					i4Avg += (s32)(((u64)(_prStatItem[STAT_IDX_DL].u8TotalTime
						* dUsPerFrequency)) / _prStatItem[STAT_IDX_DL].u4Count);
			}
			PCM_DEBUG(LOG_TAG, " %s , %8d , %8d , %8d , %10d , %8d , %8d \r\n",
				_prStatItem[i].szName,
				(s32)u4MaxTime,
				(i == STAT_IDX_AEC_NDC) ? i4Avg : (s32)(u8TotalTime / _prStatItem[i].u4Count),
				(s32)u4MinTime,
				(s32)u8TotalTime,
				(s32)_prStatItem[i].u4Count,
				(s32)_prStatItem[i].u4OverCount);
		}
	}
	PCM_DEBUG(LOG_TAG, "TimeStatOutput End <<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n");

	return true;
}

void TimeStatEnter(u32 u4Index)
{
	_prStatItem[u4Index].u8LastStart = BTGetSysTick();
}

void TimeStatPause(u32 u4Index)
{
	_prStatItem[u4Index].u8LastStart = BTGetSysTick() - _prStatItem[u4Index].u8LastStart;
}

void TimeStatResume(u32 u4Index)
{
	_prStatItem[u4Index].u8LastStart = BTGetSysTick() - _prStatItem[u4Index].u8LastStart;
}

void TimeStatLeave(u32 u4Index)
{
	u64 u8Temp = BTGetSysTick() - _prStatItem[u4Index].u8LastStart;

	if (u8Temp > _prStatItem[u4Index].u4MaxTime) {
		_prStatItem[u4Index].u4MaxTime = (u32)u8Temp;
	}
	if (u8Temp <  _prStatItem[u4Index].u4MinTime) {
		_prStatItem[u4Index].u4MinTime = (u32)u8Temp;
	}
	if (u8Temp >  _prStatItem[u4Index].u4OverLimit) {
		_prStatItem[u4Index].u4OverCount++;
	}

	_prStatItem[u4Index].u8TotalTime += u8Temp;
	_prStatItem[u4Index].u4Count++;
	_prStatItem[u4Index].u8LastStart = 0;
}

#endif
