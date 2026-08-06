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

#include <generated/atc_project.h>
#include "ybr_vga_common.h"

#define LOG_TAG "common"


#ifdef CONFIG_ATC_PLATFORM_ac823x
void HAL_GetTime(HAL_TIME_T* pTime)
{
	struct timeval time;

	if (!pTime) {
		YBRVGA_ERROR(LOG_TAG, "pTime is NULL!\n");
		return;
	}
	do_gettimeofday(&time);
	pTime->u4Seconds = time.tv_sec;
	pTime->u4Micros = time.tv_usec;
	//i_hal_t64b_get_time(pTime);
}

void HAL_GetDeltaTime(HAL_TIME_T* pResult, HAL_TIME_T* pT0,
	HAL_TIME_T* pT1)
{
	HAL_TIME_T* pNewer;
	HAL_TIME_T* pOlder;
	
	if ((!pResult) || (!pT0) || (!pT1)) {
		YBRVGA_ERROR(LOG_TAG, "pResult=0x%08x, pT0=0x%08x, pT1=0x%08x\n",
			(unsigned int)pResult, (unsigned int)pT0, (unsigned int)pT1);
		return;
	}

	if((pT0->u4Micros >= 1000000)||(pT1->u4Micros >= 1000000))
	{
		YBRVGA_ERROR(LOG_TAG, "pT0->u4Micros=0x%08x, pT1->u4Micros=0x%08x\n",
			pT1->u4Micros, pT1->u4Micros);
		return;
	}

	//Decide which one is newer
	if((pT0->u4Seconds > pT1->u4Seconds) ||
		((pT0->u4Seconds == pT1->u4Seconds) && (pT0->u4Micros > pT1->u4Micros))) {
		pNewer = pT0;
		pOlder = pT1;
	} else {
		pNewer = pT1;
		pOlder = pT0;
	}

	// count delta time
	pResult->u4Seconds = pNewer->u4Seconds - pOlder->u4Seconds;
	if(pNewer->u4Micros >= pOlder->u4Micros) {
		pResult->u4Micros = pNewer->u4Micros - pOlder->u4Micros;
	} else {
		pResult->u4Micros = 1000000 + pNewer->u4Micros - pOlder->u4Micros;
		pResult->u4Seconds--;
	}
	//i_hal_t64b_get_delta_time(pResult, pT0, pT1);
}
#endif

