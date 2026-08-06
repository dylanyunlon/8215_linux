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

#ifndef X_TIMER_H
#define X_TIMER_H

#include "x_typedef.h"

//============================================================================
// Type definitions
//============================================================================

// Note: For a system with 100Hz timer tick, an UINT32 can only represent
// about 500 days. It may be insufficient for certain cases, but it's more
// convenient and efficient to process a 32-bit integer rather than a 64-
// bit integer.
//
typedef struct
{
  UINT32    u4L;                    //Low 32bits
  UINT32    u4H;                   //Hish 32bits
} HAL_RAW_TIME_T;

typedef struct
{
  UINT32    u4Ticks;                    //Number of timer interrupts from startup
  UINT32    u4Cycles;                   //System cycles from last timer interrupt
} HAL_RAW_TIME_32bit_T;

typedef struct
{
  UINT32    u4Seconds;                  //Number of seconds from startup
  UINT32    u4Micros;                   //Remainder in microsecond
} HAL_TIME_T;

//============================================================================
// Function prototypes
//============================================================================
extern BOOL HAL_InitTimer(void);
extern BOOL HAL_ResetTimer(void);
extern void HAL_GetRawTime(HAL_RAW_TIME_T* pRawTime);
extern void HAL_GetTime(HAL_TIME_T* pTime);
extern void HAL_RawToTime(const HAL_RAW_TIME_T* pRawTime, HAL_TIME_T* pTime);

extern void HAL_GetDeltaTime(HAL_TIME_T* pResult, HAL_TIME_T* pOlder,
    HAL_TIME_T* pNewer);

extern void HAL_GetDeltaRawTime(HAL_RAW_TIME_T* pResult,
    const HAL_RAW_TIME_T* pOlder, const HAL_RAW_TIME_T* pNewer);

extern void HAL_Delay_us(UINT32 u4Micros);

extern BOOL HAL_InitManualTimer(UINT32 u4TimerInterval);

extern UINT32 HAL_GetFineTick(void);
extern void HAL_GetSysUptime(UINT32 *pui4_sec, UINT32 *pui4_micro_sec);

#endif	// X_TIMER_H

