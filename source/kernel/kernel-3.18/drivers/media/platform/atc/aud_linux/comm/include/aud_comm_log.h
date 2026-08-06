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



#ifndef _AUD_COMM_LOG_H_
#define _AUD_COMM_LOG_H_

#include "aud_comm_macros.h"
#include "aud_comm_datatype.h"


//=========================================================================================================//


#define ALOG_ERR        (1 << 0)
#define ALOG_WARN       (1 << 1)
#define ALOG_INFO       (1 << 2)
#define ALOG_CLI        (1 << 3)

#define ALOG_DBG        (1 << 4)

#define ALOG_BIT8       (1 << 8)
#define ALOG_BIT9       (1 << 9)
#define ALOG_BIT10      (1 << 10)
#define ALOG_BIT11      (1 << 11)

#define ALOG_BIT12      (1 << 12)
#define ALOG_BIT13      (1 << 13)
#define ALOG_BIT14      (1 << 14)
#define ALOG_BIT15      (1 << 15)

#define ALOG_TEST       (1 << 16)
#define ALOG_TEST_DT    (1 << 17)

#define ALOG_DEFAULT    (ALOG_ERR | ALOG_WARN | ALOG_INFO | ALOG_CLI)


//========================================================================================================//
#define AUDRETAILMSG(fg, x, exp, ...)  \
do { \
	if (fg == ALOG_INFO || fg == ALOG_CLI) { \
		pr_info(x exp, ##__VA_ARGS__) ; \
	} else if (fg == ALOG_WARN) { \
		pr_warn(x exp, ##__VA_ARGS__); \
	} else if (fg == ALOG_ERR) { \
		pr_err(x "%s:%d: " exp, FILE_ONLY, __LINE__, ##__VA_ARGS__); \
	} else if (fg == ALOG_DBG) { \
		pr_debug(x exp, ##__VA_ARGS__); \
	} \
} while ( 0 )


#define AUDLOG_NO_PREFIX(cond, exp, ...)     AUDLOG(cond, "[AUD]", exp, ##__VA_ARGS__)
	
#define AUDLOG(cond, prefix, exp, ...)       AUDRETAILMSG(cond, prefix, exp, ##__VA_ARGS__)
	
#define AUDTEMP_LOG(cond, exp, ...)          AUDRETAILMSG(cond, T(["AUD][TEMP]"), exp, ##__VA_ARGS__)


//========================================================================================================//


#define COMMLOG(cond, exp, ...)              AUDLOG(cond, T("[AUD][COMM]"), exp, ##__VA_ARGS__)
#define COMMLOG_ERR(exp, ...)                AUDLOG(_u4CommLog & ALOG_ERR,  T("[AUD][COMM]"), exp, ##__VA_ARGS__) 
#define COMMLOG_WARN(exp, ...)               AUDLOG(_u4CommLog & ALOG_WARN, T("[AUD][COMM]"), exp, ##__VA_ARGS__) 
#define COMMLOG_INFO(exp, ...)               AUDLOG(_u4CommLog & ALOG_INFO, T("[AUD][COMM]"), exp, ##__VA_ARGS__)
#define COMMLOG_CLI(exp, ...)                AUDLOG(_u4CommLog & ALOG_INFO, T("[AUD][COMM][CLI]"), exp, ##__VA_ARGS__)
#define COMMLOG_DBG(exp, ...)                AUDLOG(_u4CommLog & ALOG_DBG,  T("[AUD][COMM]"), exp, ##__VA_ARGS__)

#define COMMLOG_TEST(exp, ...)               AUDLOG(_u4CommLog & ALOG_TEST,  T("[AUD][COMM][TEST]"), exp, ##__VA_ARGS__)
#define COMMLOG_TEST_DT(exp, ...)            AUDLOG(_u4CommLog & ALOG_TEST_DT,  T("[AUD][COMM][TEST]"), exp, ##__VA_ARGS__)

#define COMMLOG_ERR_DBG(err, exp, ...)    \
    if (err){ \
        COMMLOG_ERR(exp, ##__VA_ARGS__); \
    } else { \
        COMMLOG_DBG(exp, ##__VA_ARGS__); \
    }


//========================================================================================================//


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


extern u32 _u4CommLog;

void AudLog_SetLog(u32 u4Log);
u32 AudLog_GetLog(void);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif  //_AUD_COMM_LOG_H_

