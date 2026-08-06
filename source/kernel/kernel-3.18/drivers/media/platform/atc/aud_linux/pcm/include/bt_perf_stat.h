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



#ifndef PERFSTAT_H
#define PERFSTAT_H
#if defined(__cplusplus)
extern "C" {
#endif

enum {
	STAT_IDX_AEC_NDC = 0U,
	STAT_IDX_UL,
	STAT_IDX_DL,
	STAT_IDX_UL_AEC,
	STAT_IDX_UL_NDC,
	STAT_IDX_DL_AEC,
	STAT_IDX_DL_NDC,
	STAT_IDX_DL_PLC,
	STAT_IDX_MAX,
};

#if (ENABLE_PERFORMANCE_STAT)

#define EXPORT_LOG_PER_MINUTES		10U

#define AEC_NDC_LIMITATION		10000U		/* 10.0 ms */
#define UL_LIMITATION			7000U		/* 7.0 ms */
#define DL_LIMITATION			6000U		/* 6.0 ms */
#define UL_AEC_LIMITATION		4000U		/* 4.0 ms */
#define UL_NDC_LIMITATION		3000U		/* 3.0 ms */
#define DL_AEC_LIMITATION		1000U		/* 1.0 ms */
#define DL_NDC_LIMITATION		3000U		/* 3.0 ms */
#define DL_PLC_LIMITATION		2000U		/* 2.0 ms */


bool TimeStatInit(void);
bool TimeStatUnInit(void);
bool TimeStatOutput(void);
void TimeStatEnter(u32 u4Index);
void TimeStatPause(u32 u4Index);
void TimeStatResume(u32 u4Index);
void TimeStatLeave(u32 u4Index);

#else

#define TimeStatInit()
#define TimeStatUnInit()
#define TimeStatOutput()
#define TimeStatEnter(u4Index)
#define TimeStatPause(u4Index)
#define TimeStatResume(u4Index)
#define TimeStatLeave(u4Index)

#endif

#if defined(__cplusplus)
}
#endif

#endif
