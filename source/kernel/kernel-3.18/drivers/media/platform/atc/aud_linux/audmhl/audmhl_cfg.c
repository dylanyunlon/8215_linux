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


#include "drv_config.h"

#if CONFIG_DRV_AUDIO_IN_SUPPORT

typedef signed long     s32;
typedef s8            s8;

/****************************************************************************
** Audio mhl In configure
****************************************************************************/

/* Declare the debug on/off/level and RegTest functions */

static s32 _AudInSetMicPort(s32 i4Argc, const s8 **szArgv)
{
    return 0;
}

static s32 _AudMicInInit(s32 i4Argc, const s8 ** szArgv)
{
    return 0;

}

static s32 _AudLineInInit(s32 i4Argc, const s8 ** szArgv)
{

    return 0;
}

static s32 _AudInInit(s32 i4Argc, const s8 ** szArgv)
{
    return 0;
}



static s32 _AudInWrite(s32 i4Argc, const s8 ** szArgv)
{
    return 0;
}



#endif

