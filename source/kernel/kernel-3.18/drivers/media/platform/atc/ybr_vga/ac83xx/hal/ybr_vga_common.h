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


#ifndef YBR_VGA_COMMON_H_
#define YBR_VGA_COMMON_H_


#include <generated/atc_project.h>
#include <linux/types.h>
#include "x_os.h"
#ifdef CONFIG_ATC_PLATFORM_ac823x
#include <linux/time.h>
#include "mt33xx_64b_timer.h"
#endif

#define YBRVGA_TAG "[YBR_VGA]"

#define LOG_FATAL		0U
#define LOG_ERROR		1U
#define LOG_WARNING		2U
#define LOG_INFO		8U
#define LOG_DEBUG		9U


#define YBRVGA_LOG(ftag, level, sFmt, ...)			\
	do {											\
		if (level == LOG_FATAL) {					\
			pr_err(YBRVGA_TAG "["ftag"]" "%s:%s():%d: " sFmt, FILE_ONLY, __func__, __LINE__, ##__VA_ARGS__);	\
		} else if (level == LOG_ERROR) {			\
			pr_err(YBRVGA_TAG "["ftag"]" "%s:%s():%d: " sFmt, FILE_ONLY, __func__, __LINE__, ##__VA_ARGS__);	\
		} else if (level == LOG_WARNING) {			\
			pr_warn(YBRVGA_TAG "["ftag"]" "%s:%s():%d: " sFmt, FILE_ONLY, __func__, __LINE__, ##__VA_ARGS__);	\
		} else if (level == LOG_INFO) {				\
			pr_info(YBRVGA_TAG "["ftag"]" "%s(): " sFmt, __func__, ##__VA_ARGS__);		\
		} else if (level == LOG_DEBUG) {			\
			pr_debug(YBRVGA_TAG "["ftag"]" "%s(): " sFmt, __func__, ##__VA_ARGS__);		\
		}											\
	} while(0)

#define YBRVGA_DEBUG(ftag, sFmt, ...)				\
	YBRVGA_LOG(ftag, LOG_DEBUG, sFmt, ##__VA_ARGS__)
#define YBRVGA_INFO(ftag, sFmt, ...)				\
	YBRVGA_LOG(ftag, LOG_INFO, sFmt, ##__VA_ARGS__)
#define YBRVGA_WARN(ftag, sFmt, ...)				\
	YBRVGA_LOG(ftag, LOG_WARNING, sFmt, ##__VA_ARGS__)
#define YBRVGA_ERROR(ftag, sFmt, ...)				\
	YBRVGA_LOG(ftag, LOG_ERROR, sFmt, ##__VA_ARGS__)
#define YBRVGA_FATAL(ftag, sFmt, ...)				\
	YBRVGA_LOG(ftag, LOG_FATAL, sFmt, ##__VA_ARGS__)

#ifdef CONFIG_ATC_PLATFORM_ac823x
void HAL_GetTime(HAL_TIME_T* pTime);
void HAL_GetDeltaTime(HAL_TIME_T* pResult, HAL_TIME_T* pT0,
	HAL_TIME_T* pT1);
#endif

#endif

