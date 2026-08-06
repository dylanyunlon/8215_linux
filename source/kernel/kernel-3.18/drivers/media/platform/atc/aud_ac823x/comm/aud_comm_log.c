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
*[File]                     aud_comm_log.c
*[Author]                   tongfa.luo@autochips.com
*[Description]
*
*[Copyright]
*
******************************************************************************/

#include "aud_comm_log.h"
#include "aud_oal.h"

u32 _u4CommLog = ALOG_DEFAULT;

u32 g_u4AudLogLevel = 5;


s32 Aud_snprintf(s8 *ps_str, u32 z_size, const s8 *ps_format, ...)
{
    s32 i4_len;
    va_list t_ap;

    va_start(t_ap, ps_format);
    i4_len = vsnprintf((s8 *)ps_str, (u32)z_size, (const s8 *)ps_format, t_ap);
    va_end(t_ap);

    return (s32)i4_len;
}

void AudLog_SetLog(u32 u4Log)
{
    COMMLOG_INFO((T("Set Comm Log: 0x%x -> 0x%x \n"), (u32)_u4CommLog, (u32)u4Log));
    _u4CommLog = u4Log;
}


u32 AudLog_GetLog(void)
{
    COMMLOG_INFO((T("Get Comm Log: 0x%x \n"), (u32)_u4CommLog));
    return (_u4CommLog);
}



