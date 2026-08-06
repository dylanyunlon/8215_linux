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

//============================================================================
// Include files
//============================================================================
#include <linux/module.h>      //Must be included header file

#include "x_timer.h"
#include "x_bim.h"

/* functions from mt85xx_64b_timer in kernel */
typedef struct
{
  UINT32    u4L;                    //Number of timer interrupts from startup
  UINT32    u4H;                   //System cycles from last timer interrupt
} T64b_RAW_TIME_T;

extern int i_hal_t64b_get_raw_time(T64b_RAW_TIME_T* pRawTime);
extern int i_hal_t64b_raw_to_time(const T64b_RAW_TIME_T* pRawTime, HAL_TIME_T* pTime);
extern int i_hal_t64b_get_time(HAL_TIME_T* pTime);
extern int i_hal_t64b_get_delta_raw_time(T64b_RAW_TIME_T* pResult, const T64b_RAW_TIME_T* pT0, const T64b_RAW_TIME_T* pT1);
extern int i_hal_t64b_get_delta_time(HAL_TIME_T* pResult, HAL_TIME_T* pT0, HAL_TIME_T* pT1);

#define TIMER_LIMIT             0xffffffff



/*----------------------------------------------------------------------------
 * HAL_GetRawTime() Get system ticks and clock cycles since startup
 *  @param pRawTime [out] - A pointer to RAW_TIME_T to receive raw time
 *---------------------------------------------------------------------------*/
void HAL_GetRawTime(HAL_RAW_TIME_T* pRawTime)
{
    i_hal_t64b_get_raw_time((T64b_RAW_TIME_T*)pRawTime);
}
EXPORT_SYMBOL(HAL_GetRawTime);

/*----------------------------------------------------------------------------
 * HAL_RawToTime() Convert RAW_TIME_T to TIME_T
 *  @param pRawTime [in]  - Pointer to RAW_TIME_T, source
 *  @param pTime    [out] - Pointer to TIME_T, destination
 *---------------------------------------------------------------------------*/
void HAL_RawToTime(const HAL_RAW_TIME_T* pRawTime, HAL_TIME_T* pTime)
{
    i_hal_t64b_raw_to_time((T64b_RAW_TIME_T*) pRawTime, pTime);
}
EXPORT_SYMBOL(HAL_RawToTime);

/*----------------------------------------------------------------------------
 * HAL_GetTime() Get system time from startup
 *  @param pTime    [out] - Pointer to TIME_T to store system time
 *---------------------------------------------------------------------------*/
void HAL_GetTime(HAL_TIME_T* pTime)
{
    i_hal_t64b_get_time(pTime);
}
EXPORT_SYMBOL(HAL_GetTime);

/*----------------------------------------------------------------------------
 * HAL_GetDeltaTime() Get delta time of two time stamps
 *  @param pResult  [out] - The result
 *  @param pOlder   [in]  - The older time
 *  @param pNewer   [in]  - The newer time
 *---------------------------------------------------------------------------*/
void HAL_GetDeltaTime(HAL_TIME_T* pResult, HAL_TIME_T* pT0,
    HAL_TIME_T* pT1)
{
    i_hal_t64b_get_delta_time(pResult, pT0, pT1);
}
EXPORT_SYMBOL(HAL_GetDeltaTime);

/*----------------------------------------------------------------------------
 * HAL_GetDeltaRawTime() Get delta time of two time stamps
 *  @param pResult  [out] - The result
 *  @param pOlder   [in]  - The older time
 *  @param pNewer   [in]  - The newer time
 *---------------------------------------------------------------------------*/
void HAL_GetDeltaRawTime(HAL_RAW_TIME_T* pResult, const HAL_RAW_TIME_T* pT0,
    const HAL_RAW_TIME_T* pT1)
{
    i_hal_t64b_get_delta_raw_time((T64b_RAW_TIME_T*) pResult, (T64b_RAW_TIME_T*) pT0, (T64b_RAW_TIME_T*) pT1);
}
EXPORT_SYMBOL(HAL_GetDeltaRawTime);

/*----------------------------------------------------------------------------
 * HAL_Delay_us() delay X micro seconds
 *  @param u4Micro
 *---------------------------------------------------------------------------*/
void HAL_Delay_us(UINT32 u4Micros)
{
    HAL_TIME_T rOrgTime, rNewTime, rDiffTime;

    //This function cannot delay more than 10 seconds.
    //ASSERT(u4Micros < 10000000);

    if(u4Micros > 10000000)
    {
        u4Micros = 10000000;
    }

    HAL_GetTime(&rOrgTime);
    do
    {
        HAL_GetTime(&rNewTime);
        HAL_GetDeltaTime(&rDiffTime, &rOrgTime, &rNewTime);
    }while (((1000000*rDiffTime.u4Seconds)+rDiffTime.u4Micros) < u4Micros);
}
EXPORT_SYMBOL(HAL_Delay_us);

UINT32 HAL_GetFineTick(VOID)
{
    HAL_RAW_TIME_T t1;
    
    HAL_GetRawTime(&t1);

    return t1.u4L;
}
EXPORT_SYMBOL(HAL_GetFineTick);
