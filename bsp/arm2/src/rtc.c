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
#include "rtc_hw.h"
#include "printf.h"

int ac83xx_rtc_readtime(rtc_time_t *tm)
{
	int rc = -1;

	if (!tm) {
		Printf("[RTC] got NULL tm\n");
		goto cleanUp;
	}

	tm->tm_min = RTC_READ32(RTC_TC_MIN);
	tm->tm_min = tm->tm_min & 0x3F;

	tm->tm_hour = RTC_READ32(RTC_TC_HOU);
	tm->tm_hour = tm->tm_hour & 0x1F;
#if 0
	tm->tm_mday = RTC_READ32(RTC_TC_DOM);
	tm->tm_mday = tm->tm_mday & 0x1F;

	tm->tm_mon  = RTC_READ32(RTC_TC_MTH);
	tm->tm_mon = tm->tm_mon & 0x0F;
	tm->tm_mon = tm->tm_mon - 1;

	/* Hardware DayOfWeek is 1~7 but WinCE is 0~6, 0 is Sunday.*/
	tm->tm_wday = RTC_READ32(RTC_TC_DOW);
	tm->tm_wday = tm->tm_wday & 0x07;
	tm->tm_wday = tm->tm_wday % 7;

	tm->tm_year = RTC_READ32(RTC_TC_YEA);
	tm->tm_year = tm->tm_year & 0x7F;
	tm->tm_year = tm->tm_year + 2000 - 1900;


	tm->tm_yday = rtc_year_days(tm->tm_mday, tm->tm_mon, tm->tm_year);
#endif
	if (tm != NULL) {
		Printf("[RTC] read time %02d:%02d\n", tm->tm_hour, tm->tm_min);
	}

	rc = 0;

cleanUp:
	return rc;
}

