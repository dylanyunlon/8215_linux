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
*[File]                DspAsvInc.h      
*[Author]              
*[Description]
* This files contains the syntax to exporte interface from dspctrl to Asv
******************************************************************************/
#ifndef _AUD_DEBUG_H
#define _AUD_DEBUG_H
#include <windows.h>
#include <stdarg.h>

#include "x_debug.h"

#ifdef LOG
#undef LOG
#endif



typedef struct _LOG_PARAM_
{
    bool     fgHasP;
    bool    fgType;
    union
    {
        s8     *chParam;
        u32     u4Param;
    }param;
}Log_Param;

s32 Aud_snprintf(s8 *ps_str, u32 z_size, const s8 *ps_format, ...);

//linux system
#define SYSTEM_LOG_LEVEL_ERR       0
#define SYSTEM_LOG_LEVEL_WARN      1
#define SYSTEM_LOG_LEVEL_INFO      2
#define SYSTEM_LOG_LEVEL_DEBUG     3

//driver use
#define LOG_ADSP_ERR        SYSTEM_LOG_LEVEL_ERR
#define LOG_ADSP_WARN       SYSTEM_LOG_LEVEL_WARN
#define LOG_ADSP_INFO       SYSTEM_LOG_LEVEL_INFO
#define LOG_ADSP_DEBUG      SYSTEM_LOG_LEVEL_DEBUG

//traditional
#define LOG_CTRLF           LOG_ADSP_INFO
#define LOG_DATAF           LOG_ADSP_DEBUG
#define LOG_DECINFO         LOG_ADSP_DEBUG
#define LOG_DAC             LOG_ADSP_DEBUG
#define LOG_IO              LOG_ADSP_DEBUG
#define LOG_DUALCTRL        LOG_ADSP_DEBUG
#define LOG_FEATURE         LOG_ADSP_DEBUG
#define LOG_MHL             LOG_ADSP_DEBUG
#define LOG_OTHER           LOG_ADSP_DEBUG
#define LOG_FEWUSED         LOG_ADSP_DEBUG
#define LOG_FAIL            LOG_ADSP_ERR
#define LOG_POWER           LOG_ADSP_INFO

extern u32 g_u4AudLogLevel;

#define LOG(level, sFmt, ...)  \
do { \
    if (9 == g_u4AudLogLevel) \
       pr_info("[AUD]"sFmt, ##__VA_ARGS__); \
	else if (LOG_ADSP_ERR == level) \
		pr_err("[AUD]"sFmt, ##__VA_ARGS__); \
	else if (LOG_ADSP_WARN == level) \
		pr_warn("[AUD]"sFmt, ##__VA_ARGS__); \
    else if (LOG_ADSP_INFO == level) \
		pr_info("[AUD]"sFmt, ##__VA_ARGS__); \
    else \
        pr_debug("[AUD]"sFmt, ##__VA_ARGS__); \
} while (0)


#ifndef __linux__
#define DEFINE_IS_LOG    UTIL_Printf
#else
#define DEFINE_IS_LOG    AUD_IsLog
#endif

#endif
