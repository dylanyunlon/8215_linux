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

#ifndef YBR_VGA_UTIL_H__
#define YBR_VGA_UTIL_H__


#include <generated/atc_project.h>
#include "ybr_vga_hw_reg.h"
#include "video_timing.h"
#include "x_typedef.h"
#include "x_os.h"
#ifdef CONFIG_ATC_PLATFORM_ac83xx
#include "x_timer.h"
#endif
#include "autocolor_table.h"
//#include "x_debug.h"
#include "vga_debug.h"
#include "vga_hal_api.h"
//#include "ac83xx_irqs_vector.h"
#include "ybr_vga_oal.h"
#include "x_ckgen.h"

/*#include "x_bim.h"*/

// *********************************************************************
// Video state machine define
// *********************************************************************
  // define the auto state machine
enum
{
    VDO_AUTO_NOT_BEGIN,
    VDO_AUTO_POSITION_START,
    VDO_AUTO_POSITION_1_START,
    VDO_AUTO_POSITION_SET,
    VDO_AUTO_PHASE_START,
    VDO_AUTO_PHASE_1_START,
    VDO_AUTO_CLOCK_START,
    VDO_AUTO_CLOCK_1_START,
    VDO_AUTO_CLOCK_2_START
};

enum
{
    INT_HDTV,
    INT_VGA,
    INT_SCART,
    INT_VGA_COMPOENT,
    EXT_HDTV,
    EXT_VGA
};


#endif
