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

#ifndef _AC83XX_ISR_WATCH_H_
#define _AC83XX_ISR_WATCH_H_

//#include <x_typedef.h>
#include <mach/ac83xx_64b_timer.h>

/* define & config */
#define CONFIG_AC83XX_ISR_WATCH 0

#define ISRW_EVT_START 0
#define ISRW_EVT_END   1

/* struct */
typedef struct
{
	uint32_t u4Count;
	HAL_TIME_T rTotalTime;
	HAL_TIME_T rMaxTime;
	T64b_RAW_TIME_T _rt_start;
	T64b_RAW_TIME_T _rt_end;
}AC83XX_ISR_OVERHEAD_T;


/* global functions */
extern void ac83xx_isr_watch_init(void);
extern void ac83xx_isrw_proc_init(void);
extern void ac83xx_isr_watch_trigger(uint32_t u4Vector, uint32_t event);

#endif // _AC83XX_ISR_WATCH_H_
