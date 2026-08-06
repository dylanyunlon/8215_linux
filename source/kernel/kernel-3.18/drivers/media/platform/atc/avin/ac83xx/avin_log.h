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

#ifndef _AVIN_LOG_H_
#define _AVIN_LOG_H_

#include <linux/types.h>
#include "x_os.h"

#define AVIN_TAG "[AVIN]"

#define LOG_FATAL		0U
#define LOG_ERROR		1U
#define LOG_WARNING		2U
#define LOG_INFO		8U
#define LOG_DEBUG		9U


#define AVIN_LOG(ftag, level, sFmt, ...)			\
	do {											\
		if (level == LOG_FATAL) {					\
			pr_err(AVIN_TAG "["ftag"]" "%s:%s():%d: " sFmt, FILE_ONLY, __func__, __LINE__, ##__VA_ARGS__);	\
		} else if (level == LOG_ERROR) {			\
			pr_err(AVIN_TAG "["ftag"]" "%s:%s():%d: " sFmt, FILE_ONLY, __func__, __LINE__, ##__VA_ARGS__);	\
		} else if (level == LOG_WARNING) {			\
			pr_warn(AVIN_TAG "["ftag"]" "%s:%s():%d: " sFmt, FILE_ONLY, __func__, __LINE__, ##__VA_ARGS__);	\
		} else if (level == LOG_INFO) {				\
			pr_info(AVIN_TAG "["ftag"]" "%s(): " sFmt, __func__, ##__VA_ARGS__);		\
		} else if (level == LOG_DEBUG) {			\
			pr_debug(AVIN_TAG "["ftag"]" "%s(): " sFmt, __func__, ##__VA_ARGS__);		\
		}											\
	} while(0)

#define AVIN_DEBUG(ftag, sFmt, ...)				\
	AVIN_LOG(ftag, LOG_DEBUG, sFmt, ##__VA_ARGS__)
#define AVIN_INFO(ftag, sFmt, ...)				\
	AVIN_LOG(ftag, LOG_INFO, sFmt, ##__VA_ARGS__)
#define AVIN_WARN(ftag, sFmt, ...)				\
	AVIN_LOG(ftag, LOG_WARNING, sFmt, ##__VA_ARGS__)
#define AVIN_ERROR(ftag, sFmt, ...)				\
	AVIN_LOG(ftag, LOG_ERROR, sFmt, ##__VA_ARGS__)
#define AVIN_FATAL(ftag, sFmt, ...)				\
	AVIN_LOG(ftag, LOG_FATAL, sFmt, ##__VA_ARGS__)




#endif /*_AVIN_LOG_H_*/

