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
*[File]                     aud_comm_log.h
*[Version]                  v1.0
*[Revision Date]            2014-03-10
*[Author]                   tongfa.luo@autochips.com 
*[Description]
*        
*
******************************************************************************/
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
#define AUDRETAILMSG(fg, x... )  \
do { \
	if (fg) { \
		pr_info x ; \
	} \
} while ( 0 )

#define AUDLOG_NO_PREFIX(cond, exp)     AUDRETAILMSG(cond, exp)

#define AUDLOG(cond, prefix, exp)       AUDRETAILMSG(cond, prefix); AUDRETAILMSG(cond, exp);

#define AUDTEMP_LOG(cond, exp)          AUDRETAILMSG(cond, (T(["AUD_TEMP]"))); AUDRETAILMSG(cond, exp);


//========================================================================================================//


#define COMMLOG(cond, exp)              AUDLOG(cond, (T("[AUD_COMM]")), exp)
#define COMMLOG_ERR(exp)                AUDLOG(_u4CommLog & ALOG_ERR,  (T("<***AUD_COMM_ERR***>")), exp) 
#define COMMLOG_WARN(exp)               AUDLOG(_u4CommLog & ALOG_WARN, (T("<AUD_COMM_WARN>")), exp) 
#define COMMLOG_INFO(exp)               AUDLOG(_u4CommLog & ALOG_INFO, (T("[AUD_COMM]")), exp)
#define COMMLOG_CLI(exp)                AUDLOG(_u4CommLog & ALOG_INFO, (T("[AUD_COMM_CLI]")), exp)
#define COMMLOG_DBG(exp)                AUDLOG(_u4CommLog & ALOG_DBG,  (T("[AUD_COMM]")), exp)

#define COMMLOG_TEST(exp)               AUDLOG(_u4CommLog & ALOG_TEST,  (T("[AUD_COMM_TEST]")), exp)
#define COMMLOG_TEST_DT(exp)            AUDLOG(_u4CommLog & ALOG_TEST_DT,  (T("[AUD_COMM_TEST]")), exp)

#define COMMLOG_ERR_DBG(err, exp)    \
    if (err){ \
        COMMLOG_ERR(exp) \
    } else { \
        COMMLOG_DBG(exp) \
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

