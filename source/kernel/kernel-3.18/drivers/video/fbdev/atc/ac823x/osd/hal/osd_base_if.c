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

/** @file osd_base_if.c
*  This header file includes public function definitions of OSD base.
*/


/*-----------------------------------------------------------------------------*/
/* Include files*/
/*-----------------------------------------------------------------------------*/

#ifndef __ARM2__
#include <linux/module.h>
#include <media/atc/drv_osd_if.h>
#include <media/atc/panel.h>
//#include <media/atc/drv_av_d.h>
#include <media/atc/display_inc.h>
#include <media/atc/drv_osd_if.h>
#include "x_debug.h"
#include "x_printf.h"
#include "x_ioopt.h"
#else
#include "assert.h"
#include "drv_osd_if.h"
#include "panel.h"
//#include "drv_av_d.h"
#include "display_inc.h"
#include "drv_osd_if.h"
#endif
#include "osd_hw.h"
#include "bringup.h"

/*#include "osd_if.h"*/
/*#include "x_hal_8520.h"*/

/*#include "hw_scpos.h"*/
/*#include "scpos_reg.h"*/
#include "drv_env.h"
#include "drv_config.h"
#define DEFINE_IS_LOG   OSD_IsLog
#include "x_rtos.h"
/*#include <ceddk.h>*/
#include "log.h"
#include <generated/atc_project.h>

OFFSET_TABLE_T rLocationOffset[RES_MODE_NUM] = {
	/* RESOLUTION_480I    0*/
#if OSD_CFG_IS_REAL_INTERLACE
	{
		{0xd1, 0xc, 0x113}, /* OSD5*/
		{0xd1, 0xc, 0x113}, /* OSD4*/
		{0xd1, 0xc, 0x113}, /* OSD3*/
		{0xd1, 0xc, 0x113}, /* OSD2*/
		{0xd1, 0xc, 0x113}, /* OSD1*/
		{0x63, 43, 43}, /* VDP2*/
		{0x63, 43, 43}, /* VDP1*/
	},
#else
	{
		{0xe9, 0x15, 0x15}, /* OSD5*/
		{0x69, 0x15, 0x15}, /* OSD4*/
		{0x66, 0x15, 0x15}, /* OSD3*/
		{0x67, 0x15, 0x15}, /* OSD2*/
		{0x67, 0x1d, 0x1d}, /* OSD1*/
		{0x63, 43, 43}, /* VDP2*/
		{0x63, 43, 43}, /* VDP1*/
	},
#endif

	/* RESOLUTION_576I    1*/
#if OSD_CFG_IS_REAL_INTERLACE
	{
		{0xd1, 0xc, 0x113}, /* OSD5*/
		{0xd1, 0xc, 0x113}, /* OSD4*/
		{0xd1, 0xc, 0x113}, /* OSD3*/
		{0xd1, 0xc, 0x113}, /* OSD2*/
		{0xd1, 0xc, 0x113}, /* OSD1*/
		{0x6D, 45, 45}, /* VDP2*/
		{0x6D, 45, 45}, /* VDP1*/
	},
#else
	{
		{0x68, 0x22, 0x2e}, /* OSD5*/
		{0x71, 0x22, 0x2e}, /* OSD4*/
		{0x6d, 0x22, 0x2e}, /* OSD3*/
		{0x6e, 0x22, 0x2e}, /* OSD2*/
		{0x70, 0x22, 0x2e}, /* OSD1 V start perhaps 21*/
		{0x6D, 45, 45}, /* VDP2*/
		{0x6D, 45, 45}, /* VDP1*/
	},
#endif

	/* RESOLUTION_480P    2*/
	{
		{0x57, 0x1d, 0x1d}, /* OSD5*/
		{0x5e, 0x1d, 0x1d}, /* OSD4*/
		{0x47, 0x2d, 0x1d}, /* OSD3*/
		{0x47, 0x2d, 0x1d}, /* OSD2*/
		{0x60, 0x2b, 0x1d}, /* OSD1*/
		{0x63, 43, 43}, /* VDP2*/
		{0x63, 43, 43}, /* VDP1*/
	},

	/* RESOLUTION_576P    3*/
	{
		{0x5e, 0x1f, 0x1f}, /* OSD5*/
		{0x65, 0x1f, 0x1f}, /* OSD4*/
		{0x6b, 0x1f, 0x1f}, /* OSD3*/
		{0x6c, 0x1f, 0x1f}, /* OSD2*/
		{0x67, 0x1f, 0x1f}, /* OSD*/
		{0x6D, 45, 45}, /* VDP2*/
		{0x6D, 45, 45}, /* VDP1*/
	},

	/* RESOLUTION_480P_1440    4*/
#if MASTER_MODE_ENABLE
	{
		{0x59, 0x15, 0x15}, /* OSD5 - remove for 8317*/
		{0x59, 0x15, 0x15}, /* OSD4*/
		{0x59, 0x15, 0x15}, /* OSD3*/
		{0x59, 0x15, 0x15}, /* OSD2*/
		{0x59, 0x15, 0x15}, /* OSD1*/
		{0x71, 0x2B, 0x2B}, /* VDP2*/
		{0x71, 0x2B, 0x2B}, /* VDP1*/
	},
#else
	{
		{0x2a, 0x2C, 0x2a}, /* OSD5*/
		{0x34, 0x2C, 0x2a}, /* OSD4*/
		{0x34, 0x2C, 0x2a}, /* OSD3*/
		{0x34, 0x2C, 0x2a}, /* OSD2*/
		{0x34, 0x2C, 0x2a}, /* OSD1*/
		{0x71, 0x2B, 0x2B}, /* VDP2*/
		{0x71, 0x2B, 0x2B},
	},
#endif
	/* RESOLUTION_600P_800    4               //add*/
	{
		{0x42, 0x11, 0x2a}, /* OSD5*/
		{0x4c, 0x11, 0x2a}, /* OSD4*/
		{0x4c, 0x11, 0x2a}, /* OSD3*/
		{0x4c, 0x11, 0x2a}, /* OSD2*/
		{0x4c, 0x11, 0x2a}, /* OSD1*/
		{0x71, 0x2B, 0x2B}, /* VDP2*/
		{0x71, 0x2B, 0x2B}, /* VDP1*/
	},
	/* RES_600P_1024    39                     //add  LVDS LCD*/
#if MASTER_MODE_ENABLE
	{
		{0xa7, 0x12, 0x12}, /* OSD5 - remove for 8317*/
		{0xa7, 0x12, 0x12}, /* OSD4*/
		{0xa7, 0x12, 0x12}, /* OSD3*/
		{0xa7, 0x12, 0x12}, /* OSD2*/
		{0xa7, 0x12, 0x12}, /* OSD1*/
		{0x71, 0x2B, 0x2B}, /* VDP2*/
		{0x71, 0x2B, 0x2B}, /* VDP1*/
	},
#else
	{
		{0x40, 0x0f, 0x0f}, /* OSD5*/
		{0x4d, 0x0f, 0x0f}, /* OSD4*/
		{0x4d, 0x0f, 0x0f}, /* OSD3*/
		{0x4d, 0x0f, 0x0f}, /* OSD2*/
		{0x4d, 0x0f, 0x0f}, /* OSD1*/
		{0x71, 0x2B, 0x2B}, /* VDP2*/
		{0x71, 0x2B, 0x2B}, /* VDP1*/
	},
#endif
	/* RESOLUTION_720P_1280    7*/
	{
		{0x68, 0x2e, 0x2e}, /* OSD5*/
		{0x71, 0x2e, 0x2e}, /* OSD4*/
		{0x6d, 0x2e, 0x2e}, /* OSD3*/
		{0x6e, 0x2e, 0x2e}, /* OSD2*/
		{0x70, 0x2e, 0x2e}, /* OSD1*/
		{0x71, 0x2B, 0x2B}, /* VDP2*/
		{0x71, 0x2B, 0x2B}, /* VDP1*/
	},
	/* RESOLUTION_800P_1280    8*/
	{
		{0x71, 0x14, 0x2a}, /* OSD5*/
		{0x71, 0x14, 0x2a}, /* OSD4*/
		{0x71, 0x14, 0x2a}, /* OSD3*/
		{0x71, 0x14, 0x2a}, /* OSD2*/
		{0x71, 0x14, 0x2a}, /* OSD1*/
		{0x71, 0x2B, 0x2B}, /* VDP2*/
		{0x71, 0x2B, 0x2B}, /* VDP1*/
	},
	/*resolution 1024x768   9*/
	{
		{0x57, 0x0e, 0x0e}, /* OSD5 - remove for 8317*/
		{0x57, 0x0e, 0x0e}, /* OSD4*/
		{0x57, 0x0e, 0x0e}, /* OSD3*/
		{0x57, 0x0e, 0x0e}, /* OSD2*/
		{0x57, 0x0e, 0x0e}, /* OSD1*/
		{0x71, 0x2B, 0x2B}, /* VDP2*/
		{0x71, 0x2B, 0x2B}, /* VDP1*/
	},
	/* RESOLUTION_576P_2880    9*/
	{
		{0x68, 0x2e, 0x2e}, /* OSD5*/
		{0x71, 0x2e, 0x2e}, /* OSD4*/
		{0x6d, 0x2e, 0x2e}, /* OSD3*/
		{0x6e, 0x2e, 0x2e}, /* OSD2*/
		{0x70, 0x2e, 0x2e}, /* OSD1*/
		{0x71, 0x2B, 0x2B}, /* VDP2*/
		{0x71, 0x2B, 0x2B}, /* VDP1*/
	},

	/* RESOLUTION_720P60HZ    8*/
	{
		{0xda, 0xc, 0xc}, /* OSD5  RESOLUTION_720P60HZ*/
		{0xde, 0xc, 0xc}, /* OSD4*/
		{0xe0, 0xc, 0xc}, /* OSD3*/
		{0xe2, 0xc, 0xc}, /* OSD2*/
		{0xe2, 0xc, 0xc}, /* OSD1*/
		{0xE0, 26, 26}, /* VDP2*/
		{0xE0, 26, 26}, /* VDP1*/
	},

	/* RESOLUTION_720P50HZ    9*/
	{
		{0xdb, 0xc, 0x18}, /* OSD5*/
		{0xe7, 0xc, 0x18}, /* OSD4*/
		{0xe1, 0xc, 0x18}, /* OSD3*/
		{0xe2, 0xc, 0x18}, /* OSD2*/
		{0xe9, 0xc, 0x18}, /* OSD1*/
		{0xE0, 26, 26}, /* VDP2*/
		{0xE0, 26, 26} /* VDP1*/
	},

	/* RESOLUTION_1080I60HZ    10*/
#if OSD_CFG_IS_REAL_INTERLACE
	{
		{0x9e, 0x0b, 0x23e}, /* OSD5*/
		{0xa3, 0x0b, 0x23e}, /* OSD4*/
		{0x9d, 0x0b, 0x23e}, /* OSD3*/
		{0xa5, 0x0b, 0x23e}, /* OSD2*/
		{0xa5, 0x0a, 0x23d}, /* OSD1*/
		{0x95, 41, 41}, /* VDP2*/
		{0x95, 41, 41}, /* VDP1*/
	},
#else
	{
		{0x98, 0x27, 0x27}, /* OSD5*/
		{0xa4, 0x27, 0x27}, /* OSD4*/
		{0x9b, 0x1c, 0x1c}, /* OSD3*/
		{0x9d, 0x1c, 0x1c}, /* OSD2*/
		{0x9f, 0x1c, 0x1c}, /* OSD1*/
		{0x95, 41, 41}, /* VDP2*/
		{0x95, 41, 41}, /* VDP1*/
	},
#endif

	/* RESOLUTION_1080I50HZ    11*/
#if OSD_CFG_IS_REAL_INTERLACE
	{
		{0x99, 0x27, 0x27}, /* OSD5*/
		{0xa1, 0x27, 0x27}, /* OSD4*/
		{0x9a, 0x27, 0x27}, /* OSD3*/
		{0x9b, 0x27, 0x27}, /* OSD2*/
		{0xa1, 0x27, 0x27}, /* OSD1*/
		{0x95, 41, 41}, /* VDP2*/
		{0x95, 41, 41}, /* VDP1*/
	},
#else
	{
		{0x99, 0x1b, 0x27}, /* OSD5*/
		{0xa1, 0x1b, 0x27}, /* OSD4*/
		{0x9d, 0x1b, 0x27}, /* OSD3*/
		{0x9f, 0x1b, 0x27}, /* OSD2*/
		{0xa1, 0x1b, 0x27}, /* OSD1*/
		{0x95, 41, 41}, /* VDP2*/
		{0x95, 41, 41}, /* VDP1*/
	},
#endif

	/* RESOLUTION_1080P60HZ    12*/
	{
		{0x91, 0x1b, 0x10f}, /* OSD5*/
		{0x95, 0x1b, 0x10f}, /* OSD4*/
		{0x97, 0x1b, 0x10f}, /* OSD3*/
		{0x99, 0x1b, 0x10f}, /* OSD2*/
		{0x9f, 0x1b, 0x10f}, /* OSD1*/
		{0x95, 41, 41}, /* VDP2*/
		{0x95, 41, 41}, /* VDP1*/
	},

	/* RESOLUTION_1080P50HZ    13*/
	{
		{0x9a, 0x1b, 0x28}, /* OSD5*/
		{0xa2, 0x1b, 0x28}, /* OSD4*/
		{0x98, 0x1b, 0x28}, /* OSD3*/
		{0x9a, 0x1b, 0x28}, /* OSD2*/
		{0xa2, 0x1b, 0x28}, /* OSD1*/
		{0x95, 41, 41}, /* VDP2*/
		{0x95, 41, 41}, /* VDP1*/
	},

	/* RESOLUTION_1080P30HZ    14*/
	{
		{0x98, 0x28, 0x28}, /* OSD5*/
		{0xa0, 0x28, 0x28}, /* OSD4*/
		{0x99, 0x28, 0x28}, /* OSD3*/
		{0x9a, 0x28, 0x28}, /* OSD2*/
		{0x9e, 0x28, 0x28}, /* OSD1*/
		{161, 42, 42}, /* VDP2*/
		{161, 42, 42}, /* VDP1*/
	},

	/* RESOLUTION_1080P25HZ    15*/
	{
		{0x98, 0x28, 0x28}, /* OSD5*/
		{0xa0, 0x28, 0x28}, /* OSD4*/
		{0x99, 0x28, 0x28}, /* OSD3*/
		{0x9a, 0x28, 0x28}, /* OSD2*/
		{0x9e, 0x28, 0x28}, /* OSD1*/
		{161, 42, 42}, /* VDP2*/
		{161, 42, 42}, /* VDP1*/
	},

	/* RESOLUTION_480I_2880    16*/
	{
		{0x61, 0x2a, 0x2a}, /* OSD5*/
		{0x69, 0x2a, 0x2a}, /* OSD4*/
		{0x65, 0x2a, 0x2a}, /* OSD3*/
		{0x66, 0x2a, 0x2a}, /* OSD2*/
		{0x68, 0x2a, 0x2a}, /* OSD1*/
		{0x71, 0x2B, 0x2B}, /* VDP2*/
		{0x71, 0x2B, 0x2B}, /* VDP1*/
	},

	/* RESOLUTION_576I_2880    17*/
	{
		{0x68, 0x2e, 0x2e}, /* OSD5*/
		{0x71, 0x2e, 0x2e}, /* OSD4*/
		{0x6d, 0x2e, 0x2e}, /* OSD3*/
		{0x6e, 0x2e, 0x2e}, /* OSD2*/
		{0x70, 0x2e, 0x2e}, /* OSD1*/
		{0x71, 0x2B, 0x2B}, /* VDP2*/
		{0x71, 0x2B, 0x2B}, /* VDP1*/
	},

	/* RESOLUTION_1080P24HZ    18*/
	{
		{0xa1, 0x29, 0x29}, /* OSD5*/
		{0xa9, 0x29, 0x29}, /* OSD4*/
		{0xa2, 0x29, 0x29}, /* OSD3*/
		{0xa3, 0x29, 0x29}, /* OSD2*/
		{0xa7, 0x29, 0x29}, /* OSD1*/
		{150, 42, 42}, /* VDP2*/
		{150, 42, 42}, /* VDP1*/
	},

	/* RESOLUTION_1080P23_976HZ    19*/
	{
		{0xa1, 0x29, 0x29}, /* OSD5*/
		{0xa9, 0x29, 0x29}, /* OSD4*/
		{0xa2, 0x29, 0x29}, /* OSD3*/
		{0xa3, 0x29, 0x29}, /* OSD2*/
		{0xa7, 0x29, 0x29}, /* OSD1*/
		{150, 42, 42}, /* VDP2*/
		{150, 42, 42}, /* VDP1*/
	},

	/* RES_1080P29_97HZ    20*/
	{
		{0xa1, 0x29, 0x29}, /* OSD5*/
		{0xa9, 0x29, 0x29}, /* OSD4*/
		{0xa2, 0x29, 0x29}, /* OSD3*/
		{0xa3, 0x29, 0x29}, /* OSD2*/
		{0xa7, 0x29, 0x29}, /* OSD1*/
		{161, 42, 42}, /* VDP2*/
		{161, 42, 42}, /* VDP1*/
	},

	/* RES_3D_1080P23HZ    21*/
	{
		{0xa1, 0x10, 0x29}, /* OSD5*/
		{0xa9, 0x10, 0x29}, /* OSD4*/
		{0x95, 0x10, 0x29}, /* OSD3*/
		{0x97, 0x10, 0x29}, /* OSD2*/
		{0xa7, 0x10, 0x29}, /* OSD1*/
		{150, 42, 42}, /* VDP2*/
		{150, 42, 42}, /* VDP1*/
	},

	/* RES_3D_1080P24HZ    22*/
	{
		{0xa1, 0x29, 0x29}, /* OSD5*/
		{0xa9, 0x29, 0x29}, /* OSD4*/
		{0xa2, 0x29, 0x29}, /* OSD3*/
		{0xa3, 0x29, 0x29}, /* OSD2*/
		{0xa7, 0x29, 0x29}, /* OSD1*/
		{150, 42, 42}, /* VDP2*/
		{150, 42, 42}, /* VDP1*/
	},

	/* RES_3D_720P60HZ    23*/
	{
		{0xdc, 0x18, 0x18}, /* OSD5*/
		{0xe8, 0x18, 0x18}, /* OSD4*/
		{0xe2, 0x18, 0x18}, /* OSD3*/
		{0xea, 0x18, 0x18}, /* OSD2*/
		{0xe9, 0x18, 0x18}, /* OSD1*/
		{229, 26, 26}, /* VDP2*/
		{229, 26, 26}, /* VDP1*/
	},

	/* RES_3D_720P50HZ    24*/
	{
		{0xdc, 0x18, 0x18}, /* OSD5*/
		{0xe8, 0x18, 0x18}, /* OSD4*/
		{0xe2, 0x18, 0x18}, /* OSD3*/
		{0xea, 0x18, 0x18}, /* OSD2*/
		{0xe9, 0x18, 0x18}, /* OSD1*/
		{229, 26, 26}, /* VDP2*/
		{229, 26, 26}, /* VDP1*/
	},

	/* RES_3D_720P30HZ    25*/
	{
		{0xdc, 0x18, 0x18}, /* OSD5*/
		{0xe8, 0x18, 0x18}, /* OSD4*/
		{0xe2, 0x18, 0x18}, /* OSD3*/
		{0xea, 0x18, 0x18}, /* OSD2*/
		{0xe9, 0x18, 0x18}, /* OSD1*/
		{229, 26, 26}, /* VDP2*/
		{229, 26, 26}, /* VDP1*/
	},

	/* RES_3D_720P25HZ    26*/
	{
		{0xdc, 0x18, 0x18}, /* OSD5*/
		{0xe8, 0x18, 0x18}, /* OSD4*/
		{0xe2, 0x18, 0x18}, /* OSD3*/
		{0xea, 0x18, 0x18}, /* OSD2*/
		{0xe9, 0x18, 0x18}, /* OSD1*/
		{229, 26, 26}, /* VDP2*/
		{229, 26, 26}, /* VDP1*/
	},

	/* RES_3D_576P50HZ    27*/
	{
		{0x68, 0x2c, 0x2c}, /* OSD5*/
		{0x71, 0x2c, 0x2c}, /* OSD4*/
		{0x6d, 0x2c, 0x2c}, /* OSD3*/
		{0x6e, 0x2c, 0x2c}, /* OSD2*/
		{0x70, 0x2c, 0x2c}, /* OSD*/
		{110, 45, 45}, /* VDP2*/
		{110, 45, 45}, /* VDP1*/
	},

	/* RES_3D_480P60HZ    28*/
	{
		{0x5c, 0x29, 0x29}, /* OSD5*/
		{0x67, 0x29, 0x29}, /* OSD4*/
		{0x64, 0x29, 0x29}, /* OSD3*/
		{0x68, 0x29, 0x29}, /* OSD2*/
		{0x66, 0x29, 0x29}, /* OSD1*/
		{100, 43, 43}, /* VDP2*/
		{102, 43, 43}, /* VDP1*/
	},

	/* RES_3D_1080I60HZ    29*/
	{
		{0x5c, 0x29, 0x29}, /* OSD5*/
		{0x67, 0x29, 0x29}, /* OSD4*/
		{0x64, 0x29, 0x29}, /* OSD3*/
		{0x68, 0x29, 0x29}, /* OSD2*/
		{0x66, 0x29, 0x29}, /* OSD1*/
		{0x9a, 0x15, 0x248}, /* VDP2*/
		{0x9a, 0x15, 0x248}, /* VDP1*/
	},

	/* RES_3D_1080I50HZ    30*/
	{
		{0x5c, 0x29, 0x29}, /* OSD5*/
		{0x67, 0x29, 0x29}, /* OSD4*/
		{0x64, 0x29, 0x29}, /* OSD3*/
		{0x68, 0x29, 0x29}, /* OSD2*/
		{0x66, 0x29, 0x29}, /* OSD1*/
		{0x9a, 0x15, 0x248}, /* VDP2*/
		{0x9a, 0x15, 0x248}, /* VDP1*/
	},

	/* RES_3D_1080I30HZ    31*/
	{
		{0x5c, 0x29, 0x29}, /* OSD5*/
		{0x67, 0x29, 0x29}, /* OSD4*/
		{0x64, 0x29, 0x29}, /* OSD3*/
		{0x68, 0x29, 0x29}, /* OSD2*/
		{0x66, 0x29, 0x29}, /* OSD1*/
		{0x9a, 0x15, 0x248}, /* VDP2*/
		{0x9a, 0x15, 0x248}, /* VDP1*/
	},

	/* RES_3D_1080I25HZ    32*/
	{
		{0x5c, 0x29, 0x29}, /* OSD5*/
		{0x67, 0x29, 0x29}, /* OSD4*/
		{0x64, 0x29, 0x29}, /* OSD3*/
		{0x68, 0x29, 0x29}, /* OSD2*/
		{0x66, 0x29, 0x29}, /* OSD1*/
		{0x9a, 0x15, 0x248}, /* VDP2*/
		{0x9a, 0x15, 0x248}, /* VDP1*/
	},

	/* RES_3D_576I25HZ    33*/
	{
		{0x5c, 0x29, 0x29}, /* OSD5*/
		{0x67, 0x29, 0x29}, /* OSD4*/
		{0x64, 0x29, 0x29}, /* OSD3*/
		{0x68, 0x29, 0x29}, /* OSD2*/
		{0x66, 0x29, 0x29}, /* OSD1*/
		{0xd1, 0x18, 0x151}, /* VDP2*/
		{0xd1, 0x18, 0x151}, /* VDP1*/
	},

	/* RES_3D_480I30HZ    34*/
	{
		{0x5c, 0x29, 0x29}, /* OSD5*/
		{0x67, 0x29, 0x29}, /* OSD4*/
		{0x64, 0x29, 0x29}, /* OSD3*/
		{0x68, 0x29, 0x29}, /* OSD2*/
		{0x66, 0x29, 0x29}, /* OSD1*/
		{0xe1, 0x13, 0x11a}, /* VDP2*/
		{0xe1, 0x13, 0x11a}, /* VDP1*/
	},

	/* RES_3D_576I50HZ    35*/
	{
		{0x5c, 0x29, 0x29}, /* OSD5*/
		{0x67, 0x29, 0x29}, /* OSD4*/
		{0x64, 0x29, 0x29}, /* OSD3*/
		{0x68, 0x29, 0x29}, /* OSD2*/
		{0x66, 0x29, 0x29}, /* OSD1*/
		{0xd1, 0x18, 0x151}, /* VDP2*/
		{0xd1, 0x18, 0x151}, /* VDP1*/
	},

	/* RES_3D_480I60HZ    36*/
	{
		{0x5c, 0x29, 0x29}, /* OSD5*/
		{0x67, 0x29, 0x29}, /* OSD4*/
		{0x64, 0x29, 0x29}, /* OSD3*/
		{0x68, 0x29, 0x29}, /* OSD2*/
		{0x66, 0x29, 0x29}, /* OSD1*/
		{0xe1, 0x13, 0x11a}, /* VDP2*/
		{0xe1, 0x13, 0x11a}, /* VDP1*/
	},

	/* RES_2D_480I60HZ    37*/
	{
		{0xe9, 0x15, 0x15}, /* OSD5*/
		{0x69, 0x15, 0x15}, /* OSD4*/
		{0x65, 0x15, 0x15}, /* OSD3*/
		{0x66, 0x15, 0x15}, /* OSD2*/
		{0x66, 0x15, 0x15}, /* OSD1*/
		{0xe1, 0x13, 0x11a}, /* VDP2*/
		{0xe1, 0x13, 0x11a}, /* VDP1*/
	},

	/* RES_2D_576I50HZ    38*/
	{
		{0xe9, 0x15, 0x15}, /* OSD5*/
		{0x69, 0x15, 0x15}, /* OSD4*/
		{0x65, 0x15, 0x15}, /* OSD3*/
		{0x66, 0x15, 0x15}, /* OSD2*/
		{0x66, 0x15, 0x15}, /* OSD1*/
		{0xe1, 0x13, 0x11a}, /* VDP2*/
		{0xe1, 0x13, 0x11a}, /* VDP1*/
	},

	/* RES_PANEL_AUO_B089AW01    39*/
	{
		{0x132, 0xc, 0xc}, /* OSD5*/
		{0x138, 0xc, 0xc}, /* OSD4*/
		{0x134, 0xc, 0xc}, /* OSD3*/
		{0x136, 0xc, 0xc}, /* OSD2*/
		{0x138, 0xc, 0xc}, /* OSD1*/
		{288, 26, 26}, /* VDP2*/
		{288, 26, 26}, /* VDP1*/
	}
};


/*-----------------------------------------------------------------------------*/
/* Static variables*/
/*-----------------------------------------------------------------------------*/
/**/

static __u32 _fgOsdInit;

/* tmp*/
#define PANEL_GetPanelWidth()     720
#define PANEL_GetPanelHeight()    480
#define bReadSCPOS(x)               0
/*#define bReadSCPOSMsk(x, y)         0*/
static __u32 _u4CurrentDispMode = 2;
__u32  u4DispModeScrnHStart[8] = {0x114, 0x89, 0x74, 0x90, 0xe8, 0, 0x98, 0x98};

OSD_BASE_CHANGE_RESOLUTION_FLAG_T _rOsdSetDispMdFlag;


/*-----------------------------------------------------------------------------*/
/* Static functions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** Brief
*  @param
*  @return
*/
/*-----------------------------------------------------------------------------*/
bool gOSD8Mix2DVD = FALSE;
void OSD_R_MIX2DVD(bool fgEnable)
{
	gOSD8Mix2DVD = fgEnable;
}

void OSD8_MIX2DVD(void)
{
	OFFSET_TABLE_T rOffset = rLocationOffset[RES_480P];

	if (gOSD8Mix2DVD) {
		IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd8(rOffset.rOSD3.i4XOffset + 0x13));
	} else {
		IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd8(rOffset.rOSD3.i4XOffset));
	}
}

/*-----------------------------------------------------------------------------*/
/* Inter-file functions*/
/*-----------------------------------------------------------------------------*/

bool fgOsdInit(void)
{
	return _fgOsdInit;
}


/*-----------------------------------------------------------------------------*/
/* Public functions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** Brief
*  @param
*  @return
*/
/*-----------------------------------------------------------------------------*/
__s32 OSD_Init(bool fgHwReset)
{
	int i;

	_prHwOsdBaseReg = (OSD_BASE_UNION_T *)osdf_reg;
	_prHwOsdRBaseReg = (OSD_BASE_UNION_T *)osdr_reg;
	FB_PRINT(FB_LOG_LVL_DBG, "[OSD_Reset] osd register %x, %x\r\n", (unsigned int)_prHwOsdBaseReg
		, (unsigned int)_prHwOsdRBaseReg);
	_prHwOsdPlaneCoreReg[0] = (OSD_PLA_CORE_UNION_T *)osd1_reg;
	_prHwOsdPlaneCoreReg[1] = (OSD_PLA_CORE_UNION_T *)osd2_reg;
	_prHwOsdPlaneCoreReg[2] = (OSD_PLA_CORE_UNION_T *)osd3_reg;
	_prHwOsdPlaneCoreReg[3] = (OSD_PLA_CORE_UNION_T *)osd4_reg;
	_prHwOsdPlaneCoreReg[4] = (OSD_PLA_CORE_UNION_T *)osd5_reg;
	_prHwOsdPlaneCoreReg[5] = (OSD_PLA_CORE_UNION_T *)osdr1_reg;
	_prHwOsdPlaneCoreReg[6] = (OSD_PLA_CORE_UNION_T *)osdr2_reg;
	_prHwOsdPlaneCoreReg[7] = (OSD_PLA_CORE_UNION_T *)osdr3_reg;

	_prHwOsdScalerReg[0] = (OSD_SC_UNION_T *)(osdf_reg + 0xc00);
	_prHwOsdScalerReg[1] = (OSD_SC_UNION_T *)(osdf_reg + 0x400);
	_prHwOsdScalerReg[2] = (OSD_SC_UNION_T *)(osdf_reg + 0x500);
	_prHwOsdScalerReg[3] = (OSD_SC_UNION_T *)(osdf_reg + 0xd00);
	_prHwOsdScalerReg[4] = (OSD_SC_UNION_T *)(osdr_reg + 0xc00); /* Mustn't use*/
	_prHwOsdScalerReg[5] = (OSD_SC_UNION_T *)(osdr_reg + 0xc00);
	_prHwOsdScalerReg[6] = (OSD_SC_UNION_T *)(osdr_reg + 0x400);
	_prHwOsdScalerReg[7] = (OSD_SC_UNION_T *)(osdr_reg + 0x500);

	if (_fgOsdInit == 0) {

		IGNORE_RET(OSD_Reset(fgHwReset));

		for (i = OSD_PLANE_1; i < OSD_PLANE_MAX_NUM; i++) {
			_rOsdCurRgn[i] = -1;

		}

		_fgOsdInit = 1;
	}

	return (__s32)OSD_RET_OK;
}
EXPORT_SYMBOL(OSD_Init);


/*-----------------------------------------------------------------------------*/
/* Public functions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** Brief
*  @param
*  @return
*/
/*-----------------------------------------------------------------------------*/
__s32 i4OSD_Uninit(void)
{
	if (_fgOsdInit == 1) {
		__u32  i;
		/* reset the whole OSD */
#ifdef OSD_SUPPORT_SUSPEND
		register_pm_ops(&osd_pm_ops);
#endif

		IGNORE_RET(_OSD_BASE_SetReg(NULL, FALSE));

		/* adjust scaler to fit new resolution and also reset LPF */
		/* default enable scaler for better bandwidth utilization */
		for (i = OSD_SCALER_1; i < OSD_SCALER_MAX_NUM; i++) {
			if ((i == OSD_SCALER_5) || (i == OSD_SCALER_6)) {
				continue;
			}

			IGNORE_RET(OSD_SC_Scale(i, FALSE, 0, 0, 0, 0));
			IGNORE_RET(OSD_SC_SetFormat16Bpp(i));
			IGNORE_RET(OSD_SC_SetLpf(i, FALSE));
		}



		_OSD_RGN_UninitApi();
		_OSD_PLA_Delete_Semaphores();

		IGNORE_RET(OSD_RGN_LIST_Init());
		IGNORE_RET(OSD_RGN_Init());

		_fgOsdInit = 0;
	}

	return (__s32)OSD_RET_OK;
}
EXPORT_SYMBOL(i4OSD_Uninit);


/*-----------------------------------------------------------------------------*/
/** Brief
*  @param
*  @return
*/
/*-----------------------------------------------------------------------------*/
#ifdef __ARM2__
extern unsigned int _u4LCDWidth;
extern unsigned int _u4LCDHeight;
#endif
__s32 OSD_Reset(bool fgHwReset)
{
	__u32  i;

	if (fgHwReset) {
		_OSD_AlwaysUpdateReg(FALSE);

		IO_WRITE32(IO_BASE_BRINGUP, 0xB4, (0x10 | IO_READ32(IO_BASE_BRINGUP, 0xB4))); /*osd_r_clk_en[4]     add*/
		IO_WRITE32(IO_BASE_BRINGUP, 0xD0, (0x10 | IO_READ32(IO_BASE_BRINGUP, 0xD0))); /*osd_r_rst_en[4]*/
		IO_WRITE32(IO_BASE_BRINGUP, 0xB4, (0x08 | IO_READ32(IO_BASE_BRINGUP, 0xB4))); /*osd_clk_en[3]*/
		IO_WRITE32(IO_BASE_BRINGUP, 0xD0, (0x08 | IO_READ32(IO_BASE_BRINGUP, 0xD0))); /*osd_rst_en[3]*/
#ifndef CONFIG_ATC_PRJ_ac823x_adas
		IO_WRITE32(IO_BASE_BRINGUP, 0xB4, (0xffff0000 | IO_READ32(IO_BASE_BRINGUP, 0xB4))); /*osd_clk_en[3]*/
		IO_WRITE32(IO_BASE_BRINGUP, 0xD0, (0xffff0000 | IO_READ32(IO_BASE_BRINGUP, 0xD0))); /*osd_rst_en[3]*/
#else
		IO_WRITE32(IO_BASE_BRINGUP, 0xB4, (0x1fe0000 | IO_READ32(IO_BASE_BRINGUP, 0xB4))); /*osd_clk_en[3]*/
		IO_WRITE32(IO_BASE_BRINGUP, 0xD0, (0x1fe0000 | IO_READ32(IO_BASE_BRINGUP, 0xD0))); /*osd_rst_en[3]*/
#endif
		IO_WRITE32(IO_BASE_BRINGUP, 0xD4, (0xffff0000 | IO_READ32(IO_BASE_BRINGUP, 0xD4))); /* osd pclk select panel*/
		if(768 == _u4LCDHeight || 800 == _u4LCDHeight) { /*osd oclk select 147 > pixel clock*/
			IO_WRITE32(IO_BASE_BRINGUP, 0x10, (0x50000000 | IO_READ32(IO_BASE_BRINGUP, 0x10)));
		} else { /*osd oclk select 64.8*/
			IO_WRITE32(IO_BASE_BRINGUP, 0x10, (0x60000000 | IO_READ32(IO_BASE_BRINGUP, 0x10)));
		}

		IO_WRITE32(osdf_reg, 4, 0xffff | IO_READ32(osdf_reg, 4));
		IO_WRITE32(osd1_reg, 0, 1);
		IO_WRITE32(osd1_reg, 0, 0);
	}

	_OSD_BASE_SetReg(NULL, fgHwReset);
	_OSD_RGN_InitApi();

	for (i = OSD_PLANE_1; i < OSD_PLANE_MAX_NUM; i++) {
		IGNORE_RET(OSD_PLA_Init(i));
	}

	_OSD_PLA_Create_Semaphores();

	IGNORE_RET(OSD_RGN_LIST_Init());
	IGNORE_RET(OSD_RGN_Init());

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/* Brief: to adjust timing register according to SCPOS's configuration*/
/*-----------------------------------------------------------------------------*/
/** Brief: exchange OSD plane 1 and 2, NOTE: it will reset OSD timing
*  @param
*  @return
*/
/*-----------------------------------------------------------------------------*/
__s32 OSD_ExchangeOsd12(bool fgValue)
{
	IGNORE_RET(_OSD_BASE_SetOsd12Ex(fgValue));
	return (__s32)OSD_RET_OK;
}

/*-----------------------------------------------------------------------------*/
/** Brief: exchange OSD plane 3 and 4, NOTE: it will reset OSD timing
*  @param
*  @return
*/
/*-----------------------------------------------------------------------------*/
__s32 OSD_ExchangeOsd34(bool fgValue)
{
	IGNORE_RET(_OSD_BASE_SetOsd34Ex(fgValue));
	return (__s32)OSD_RET_OK;
}

/*-----------------------------------------------------------------------------*/
/** Brief: for FPGA emulation
*  @param
*  @return
*/
/*-----------------------------------------------------------------------------*/
__u32 OSD_BASE_GetDisplayMode(void)
{
	return _u4CurrentDispMode;
}

void OSD_BASE_SetRES_480I(void)
{
	OFFSET_TABLE_T rOffset;
	PMX_RESOLUTION_MODE_T rDspMode;

	rDspMode = RES_480I;
#if OSD_CFG_IS_REAL_INTERLACE
	rOffset = rLocationOffset[RES_480I];
	/* 0x20008*/
	IGNORE_RET(_OSD_BASE_SetOsd1Prgs(0));
	IGNORE_RET(_OSD_BASE_SetOsd2Prgs(0));
	IGNORE_RET(_OSD_BASE_SetOsd3Prgs(0));
	IGNORE_RET(_OSD_BASE_SetOsd4Prgs(0));
	IGNORE_RET(_OSD_BASE_SetOsd5Prgs(0));
	IGNORE_RET(_OSD_BASE_SetCsrPrgs(0));
	IGNORE_RET(_OSD_BASE_SetFldPolMeg(1));
	IGNORE_RET(_OSD_BASE_SetFldPol(1));
	IGNORE_RET(_OSD_BASE_SetHsEdge(1));
	/* 0x20010*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd1(rOffset.rOSD1.i4XOffset));  /* 0x114 = 276*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd2(rOffset.rOSD2.i4XOffset));
	/* 0x20014*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartCsr(rOffset.rOSD1.i4XOffset));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd3(rOffset.rOSD3.i4XOffset));
	/* 0x20040*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd4(rOffset.rOSD4.i4XOffset));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd5(rOffset.rOSD5.i4XOffset));

	/* 0x2000C*/
	IGNORE_RET(_OSD_BASE_SetOvtMain(525));  /* 0x1E0 = 480*/
	IGNORE_RET(_OSD_BASE_SetVsWidthMain(6));

	IGNORE_RET(_OSD_BASE_SetHsWidthMain(1));   /* 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetHsWidthAux(1));   /* 1 = 0x01*/
	/*        IGNORE_RET(_OSD_BASE_SetHsWidthMeg(1));   // 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetOhtMain(1716));
	IGNORE_RET(_OSD_BASE_SetOhtAux(1716));
	/*        IGNORE_RET(_OSD_BASE_SetOhtMeg(1716));*/

	IGNORE_RET(_OSD_BASE_SetOvtAux(525));
	IGNORE_RET(_OSD_BASE_SetVsWidthAux(6));
	/*      IGNORE_RET(_OSD_BASE_SetOvtMeg(525));*/
	/*        IGNORE_RET(_OSD_BASE_SetVsWidthMeg(6));*/
	/*0x20018*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotMain(rOffset.rOSD1.i4EvenYOffset));   /* 0x11C = 284*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopMain(rOffset.rOSD1.i4YOffset));  /* 0x2C = 44*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotAux(rOffset.rOSD1.i4EvenYOffset));
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopAux(rOffset.rOSD1.i4YOffset));
	/*        IGNORE_RET(_OSD_BASE_SetScrnVStartBotMeg(rOffset.rOSD1.i4EvenYOffset));*/
	/*        IGNORE_RET(_OSD_BASE_SetScrnVStartTopMeg(rOffset.rOSD1.i4YOffset));*/
	/*0x2001C*/
	IGNORE_RET(_OSD_BASE_SetScrnHSizeMain(1440));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeMain(480));
	IGNORE_RET(_OSD_BASE_SetScrnHSizeAux(720));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeAux(480));
	/*        IGNORE_RET(_OSD_BASE_SetScrnHSizeMeg(720));*/
	/*        IGNORE_RET(_OSD_BASE_SetScrnVSizeMeg(480));*/

	IGNORE_RET(_OSD_BASE_SetOsd1Dotctl(3));
	IGNORE_RET(_OSD_BASE_SetOsd2Dotctl(3));
	IGNORE_RET(_OSD_BASE_SetOsd3Dotctl(3));
	IGNORE_RET(_OSD_BASE_SetOsd4Dotctl(3));
	/*        IGNORE_RET(_OSD_BASE_SetOsd5Dotctl(3));*/
#else
	rOffset = rLocationOffset[RES_480I];
	/* 0x20008*/
	IGNORE_RET(_OSD_BASE_SetFldPolMeg(1));  /* ??*/
	IGNORE_RET(_OSD_BASE_SetOsd1Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd2Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd3Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd4Prgs(1));
	/*        IGNORE_RET(_OSD_BASE_SetOsd5Prgs(1));*/
	IGNORE_RET(_OSD_BASE_SetCsrPrgs(1));
	IGNORE_RET(_OSD_BASE_SetFldPol(1));
	IGNORE_RET(_OSD_BASE_SetVsEdge(1));
	IGNORE_RET(_OSD_BASE_SetHsEdge(0));

	/* 0x20010*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd1(rOffset.rOSD1.i4XOffset));/*rOffset.rOSD1.i4XOffset));  // 0x74 = 116*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd2(rOffset.rOSD2.i4XOffset));/*rOffset.rOSD2.i4XOffset));*/
	/* 0x20014*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartCsr(rOffset.rOSD1.i4XOffset));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd3(rOffset.rOSD3.i4XOffset));
	/* 0x20040*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd4(rOffset.rOSD4.i4XOffset));
	/*        IGNORE_RET(_OSD_BASE_SetScrnHStartOsd5(rOffset.rOSD5.i4XOffset));*/
	/* 0x2000C*/
	IGNORE_RET(_OSD_BASE_SetOvtMain(525));  /* 0x1E0 = 480*/
	IGNORE_RET(_OSD_BASE_SetVsWidthMain(6));

	IGNORE_RET(_OSD_BASE_SetHsWidthMain(1));   /* 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetHsWidthAux(1));   /* 1 = 0x01*/
	/*        IGNORE_RET(_OSD_BASE_SetHsWidthMeg(1));   // 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetOhtMain(858));
	IGNORE_RET(_OSD_BASE_SetOhtAux(858));
	/*        IGNORE_RET(_OSD_BASE_SetOhtMeg(858));*/

	IGNORE_RET(_OSD_BASE_SetOvtAux(525));
	IGNORE_RET(_OSD_BASE_SetVsWidthAux(6));
	/*      IGNORE_RET(_OSD_BASE_SetOvtMeg(525));*/
	/*        IGNORE_RET(_OSD_BASE_SetVsWidthMeg(6));*/
	/*0x20018*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotMain(rOffset.rOSD1.i4EvenYOffset));  /* 0x11C = 284->0x124*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopMain(rOffset.rOSD1.i4YOffset));   /* 0x2A*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotAux(rOffset.rOSD1.i4EvenYOffset));  /* 0x11C = 284*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopAux(rOffset.rOSD1.i4YOffset));   /* 0x2A*/
	/*        IGNORE_RET(_OSD_BASE_SetScrnVStartBotMeg(rOffset.rOSD1.i4EvenYOffset));  // 0x11C = 284*/
	/*        IGNORE_RET(_OSD_BASE_SetScrnVStartTopMeg(rOffset.rOSD1.i4YOffset));   // 0x2A*/
	/*0x2001C*/
	IGNORE_RET(_OSD_BASE_SetScrnHSizeMain(720));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeMain(480));
	IGNORE_RET(_OSD_BASE_SetScrnHSizeAux(720));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeAux(480));
	/*        IGNORE_RET(_OSD_BASE_SetScrnHSizeMeg(720));*/
	/*        IGNORE_RET(_OSD_BASE_SetScrnVSizeMeg(480));*/
#endif

}


void OSD_BASE_SetRES_480P(void)
{
	OFFSET_TABLE_T rOffset;
	PMX_RESOLUTION_MODE_T rDspMode;

	rDspMode = RES_480P;

	rOffset = rLocationOffset[RES_480P];
	/* 0x20008*/
	IGNORE_RET(_OSD_BASE_SetFldPolMeg(1));  /* ??*/
	IGNORE_RET(_OSD_BASE_SetOsd1Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd2Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd3Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd4Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd5Prgs(1));
	IGNORE_RET(_OSD_BASE_SetCsrPrgs(1));
	IGNORE_RET(_OSD_BASE_SetFldPol(1));
	IGNORE_RET(_OSD_BASE_SetVsEdge(1));
	IGNORE_RET(_OSD_BASE_SetHsEdge(0));

	/* 0x20010*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd1(rOffset.rOSD1.i4XOffset));/*rOffset.rOSD1.i4XOffset));  // 0x74 = 116*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd2(rOffset.rOSD2.i4XOffset));
	/* 0x20014*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartCsr(rOffset.rOSD1.i4XOffset));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd3(rOffset.rOSD3.i4XOffset));
	/* 0x20040*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd4(rOffset.rOSD4.i4XOffset));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd5(rOffset.rOSD5.i4XOffset));
	/* 0x2000C*/
	IGNORE_RET(_OSD_BASE_SetOvtMain(525));  /* 0x1E0 = 480*/
	IGNORE_RET(_OSD_BASE_SetVsWidthMain(0x2a));

	IGNORE_RET(_OSD_BASE_SetHsWidthMain(1));   /* 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetHsWidthAux(1));   /* 1 = 0x01*/
	/*    IGNORE_RET(_OSD_BASE_SetHsWidthMeg(1));   // 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetOhtMain(858));
	IGNORE_RET(_OSD_BASE_SetOhtAux(858));
	/*    IGNORE_RET(_OSD_BASE_SetOhtMeg(858));*/

	IGNORE_RET(_OSD_BASE_SetOvtAux(525));
	IGNORE_RET(_OSD_BASE_SetVsWidthAux(6));
	/*  IGNORE_RET(_OSD_BASE_SetOvtMeg(525));*/
	/*    IGNORE_RET(_OSD_BASE_SetVsWidthMeg(6));*/
	/*0x20018*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotMain(rOffset.rOSD1.i4EvenYOffset));  /* 0x11C = 284->0x10f*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopMain(rOffset.rOSD1.i4YOffset));   /* 0x2A*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotAux(rOffset.rOSD1.i4EvenYOffset));  /* 0x11C = 284*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopAux(rOffset.rOSD1.i4YOffset));   /* 0x2A*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVStartBotMeg(rOffset.rOSD1.i4EvenYOffset));  // 0x11C = 284*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVStartTopMeg(rOffset.rOSD1.i4YOffset));   // 0x2A*/
	/*0x2001C*/
	IGNORE_RET(_OSD_BASE_SetScrnHSizeMain(720));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeMain(480));
	IGNORE_RET(_OSD_BASE_SetScrnHSizeAux(720));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeAux(480));
	/*    IGNORE_RET(_OSD_BASE_SetScrnHSizeMeg(720));*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVSizeMeg(480));*/
	/*config Dgi & Disp path*/

}

void OSD_BASE_SetRES_480P_800(void)
{
	OFFSET_TABLE_T rOffset;
	PMX_RESOLUTION_MODE_T rDspMode;

	rDspMode = RES_480P_800;

	rOffset = rLocationOffset[RES_480P_800];
	/* 0x20008*/
	IGNORE_RET(_OSD_BASE_SetFldPolMeg(1));  /* ??*/
	IGNORE_RET(_OSD_BASE_SetOsd1Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd2Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd3Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd4Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd5Prgs(1));
	IGNORE_RET(_OSD_BASE_SetCsrPrgs(1));
	IGNORE_RET(_OSD_BASE_SetFldPol(1));
	IGNORE_RET(_OSD_BASE_SetVsEdge(1));
	IGNORE_RET(_OSD_BASE_SetHsEdge(0));

	/* 0x20010*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd1(rOffset.rOSD1.i4XOffset));/*rOffset.rOSD1.i4XOffset));  // 0x74 = 116*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd2(rOffset.rOSD2.i4XOffset));
	/* 0x20014*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartCsr(rOffset.rOSD1.i4XOffset));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd3(rOffset.rOSD3.i4XOffset));
	/* 0x20040*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd4(rOffset.rOSD4.i4XOffset));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd5(rOffset.rOSD5.i4XOffset));
	/* 0x2000C*/
	IGNORE_RET(_OSD_BASE_SetOvtMain(525));  /* 0x1E0 = 480*/
	IGNORE_RET(_OSD_BASE_SetVsWidthMain(0x0d));

	IGNORE_RET(_OSD_BASE_SetHsWidthMain(1));   /* 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetHsWidthAux(1));   /* 1 = 0x01*/
	/*    IGNORE_RET(_OSD_BASE_SetHsWidthMeg(1));   // 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetOhtMain(928));
	IGNORE_RET(_OSD_BASE_SetOhtAux(928));
	/*    IGNORE_RET(_OSD_BASE_SetOhtMeg(858));*/

	IGNORE_RET(_OSD_BASE_SetOvtAux(525));
	IGNORE_RET(_OSD_BASE_SetVsWidthAux(6));
	/*  IGNORE_RET(_OSD_BASE_SetOvtMeg(525));*/
	/*    IGNORE_RET(_OSD_BASE_SetVsWidthMeg(6));*/
	/*0x20018*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotMain(rOffset.rOSD1.i4EvenYOffset));  /* 0x11C = 284->0x10f*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopMain(rOffset.rOSD1.i4YOffset));   /* 0x2A*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotAux(rOffset.rOSD1.i4EvenYOffset));  /* 0x11C = 284*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopAux(rOffset.rOSD1.i4YOffset));   /* 0x2A*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVStartBotMeg(rOffset.rOSD1.i4EvenYOffset));  // 0x11C = 284*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVStartTopMeg(rOffset.rOSD1.i4YOffset));   // 0x2A*/
	/*0x2001C*/
	IGNORE_RET(_OSD_BASE_SetScrnHSizeMain(800));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeMain(480));
	IGNORE_RET(_OSD_BASE_SetScrnHSizeAux(800));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeAux(480));
	/*    IGNORE_RET(_OSD_BASE_SetScrnHSizeMeg(720));*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVSizeMeg(480));*/
	/*config Dgi & Disp path*/

}
void OSD_BASE_SetRES_576I(void)
{
	OFFSET_TABLE_T rOffset;
	PMX_RESOLUTION_MODE_T rDspMode;

	rDspMode = RES_576I;

#if OSD_CFG_IS_REAL_INTERLACE
	rOffset = rLocationOffset[RES_576I];
	/* 0x20008*/
	IGNORE_RET(_OSD_BASE_SetOsd1Prgs(0));
	IGNORE_RET(_OSD_BASE_SetOsd2Prgs(0));
	IGNORE_RET(_OSD_BASE_SetOsd3Prgs(0));
	IGNORE_RET(_OSD_BASE_SetOsd4Prgs(0));
	IGNORE_RET(_OSD_BASE_SetOsd5Prgs(0));
	IGNORE_RET(_OSD_BASE_SetCsrPrgs(0));
	IGNORE_RET(_OSD_BASE_SetFldPolMeg(1));
	IGNORE_RET(_OSD_BASE_SetHsEdge(1));
	/* 0x2000C*/
	IGNORE_RET(_OSD_BASE_SetOvtMain(625));  /* 576 = 0x240*/
	IGNORE_RET(_OSD_BASE_SetVsWidthMain(6));

	IGNORE_RET(_OSD_BASE_SetHsWidthMain(1));   /* 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetHsWidthAux(1));   /* 1 = 0x01*/
	/*    IGNORE_RET(_OSD_BASE_SetHsWidthMeg(1));   // 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetOhtMain(1728));
	IGNORE_RET(_OSD_BASE_SetOhtAux(1728));
	/*    IGNORE_RET(_OSD_BASE_SetOhtMeg(1728));*/

	IGNORE_RET(_OSD_BASE_SetOvtAux(625));
	IGNORE_RET(_OSD_BASE_SetVsWidthAux(6));
	/*  IGNORE_RET(_OSD_BASE_SetOvtMeg(625));*/
	/*    IGNORE_RET(_OSD_BASE_SetVsWidthMeg(6));*/
	/* 0x20010*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd1(rOffset.rOSD1.i4XOffset));  /* 137 = 0x89*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd2(rOffset.rOSD2.i4XOffset));
	/* 0x20014*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartCsr(rOffset.rOSD1.i4XOffset));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd3(rOffset.rOSD3.i4XOffset));
	/* 0x20040*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd4(rOffset.rOSD4.i4XOffset));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd5(rOffset.rOSD5.i4XOffset));
	/*0x20018*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotMain(rOffset.rOSD1.i4EvenYOffset));    /*  263 = 0x107*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopMain(rOffset.rOSD1.i4YOffset));     /*  22 = 0x16*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotAux(rOffset.rOSD1.i4EvenYOffset));
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopAux(rOffset.rOSD1.i4YOffset));
	/*    IGNORE_RET(_OSD_BASE_SetScrnVStartBotMeg(rOffset.rOSD1.i4EvenYOffset));*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVStartTopMeg(rOffset.rOSD1.i4YOffset));*/
	/*0x2001C*/
	IGNORE_RET(_OSD_BASE_SetScrnHSizeMain(1440));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeMain(576));
	IGNORE_RET(_OSD_BASE_SetScrnHSizeAux(720));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeAux(576));
	/*    IGNORE_RET(_OSD_BASE_SetScrnHSizeMeg(720));*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVSizeMeg(576));*/
	/*config Dgi & Disp path*/
#else
	rOffset = rLocationOffset[RES_576I];
	/* 0x20008*/
	IGNORE_RET(_OSD_BASE_SetOsd1Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd2Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd3Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd4Prgs(1));
	/*    IGNORE_RET(_OSD_BASE_SetOsd5Prgs(1));*/
	IGNORE_RET(_OSD_BASE_SetCsrPrgs(1));
	IGNORE_RET(_OSD_BASE_SetFldPolMeg(1));
	IGNORE_RET(_OSD_BASE_SetHsEdge(1));
	/* 0x2000C*/
	IGNORE_RET(_OSD_BASE_SetOvtMain(625));  /* 576 = 0x240*/
	IGNORE_RET(_OSD_BASE_SetVsWidthMain(6));

	IGNORE_RET(_OSD_BASE_SetHsWidthMain(1));   /* 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetHsWidthAux(1));   /* 1 = 0x01*/
	/*    IGNORE_RET(_OSD_BASE_SetHsWidthMeg(1));   // 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetOhtMain(864));
	IGNORE_RET(_OSD_BASE_SetOhtAux(864));
	/*    IGNORE_RET(_OSD_BASE_SetOhtMeg(864));*/

	IGNORE_RET(_OSD_BASE_SetOvtAux(625));
	IGNORE_RET(_OSD_BASE_SetVsWidthAux(6));
	/*  IGNORE_RET(_OSD_BASE_SetOvtMeg(625));*/
	/*    IGNORE_RET(_OSD_BASE_SetVsWidthMeg(6));*/
	/* 0x20010*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd1(rOffset.rOSD1.i4XOffset));  /* 144 = 0x90*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd2(rOffset.rOSD2.i4XOffset));
	/* 0x20014*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartCsr(rOffset.rOSD1.i4XOffset));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd3(rOffset.rOSD3.i4XOffset));
	/* 0x20040*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd4(rOffset.rOSD4.i4XOffset));
	/*    IGNORE_RET(_OSD_BASE_SetScrnHStartOsd5(rOffset.rOSD5.i4XOffset));*/
	/*0x20018*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotMain(0x110));    /* 285 = 0x11D->0x110*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopMain(rOffset.rOSD1.i4YOffset));     /* 48 = 0x30*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotAux(0x110));
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopAux(rOffset.rOSD1.i4YOffset));

	/*0x2001C*/
	IGNORE_RET(_OSD_BASE_SetScrnHSizeMain(720));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeMain(576));
	IGNORE_RET(_OSD_BASE_SetScrnHSizeAux(720));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeAux(576));


#endif
}

void OSD_BASE_SetRES_576P(void)
{
	OFFSET_TABLE_T rOffset;
	PMX_RESOLUTION_MODE_T rDspMode;

	rDspMode = RES_576P;
	rOffset = rLocationOffset[RES_576P];
	/* 0x20008*/
	IGNORE_RET(_OSD_BASE_SetOsd1Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd2Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd3Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd4Prgs(1));
	/*    IGNORE_RET(_OSD_BASE_SetOsd5Prgs(1));*/
	IGNORE_RET(_OSD_BASE_SetCsrPrgs(1));
	IGNORE_RET(_OSD_BASE_SetFldPolMeg(1));
	IGNORE_RET(_OSD_BASE_SetHsEdge(1));
	/* 0x2000C*/
	IGNORE_RET(_OSD_BASE_SetOvtMain(625));  /* 576 = 0x240*/
	IGNORE_RET(_OSD_BASE_SetVsWidthMain(6));

	IGNORE_RET(_OSD_BASE_SetHsWidthMain(1));   /* 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetHsWidthAux(1));   /* 1 = 0x01*/
	/*    IGNORE_RET(_OSD_BASE_SetHsWidthMeg(1));   // 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetOhtMain(864));
	IGNORE_RET(_OSD_BASE_SetOhtAux(864));
	/*    IGNORE_RET(_OSD_BASE_SetOhtMeg(864));*/

	IGNORE_RET(_OSD_BASE_SetOvtAux(625));
	IGNORE_RET(_OSD_BASE_SetVsWidthAux(6));
	/*  IGNORE_RET(_OSD_BASE_SetOvtMeg(625));*/
	/*    IGNORE_RET(_OSD_BASE_SetVsWidthMeg(6));*/
	/* 0x20010*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd1(rOffset.rOSD1.i4XOffset));  /* 144 = 0x90*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd2(rOffset.rOSD2.i4XOffset));
	/* 0x20014*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartCsr(rOffset.rOSD1.i4XOffset));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd3(rOffset.rOSD3.i4XOffset));
	/* 0x20040*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd4(rOffset.rOSD4.i4XOffset));
	/*    IGNORE_RET(_OSD_BASE_SetScrnHStartOsd5(rOffset.rOSD5.i4XOffset));*/
	/*0x20018*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotMain(0x110));    /* 285 = 0x11D->0x10f*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopMain(rOffset.rOSD1.i4YOffset));     /* 48 = 0x30*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotAux(0x110));
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopAux(rOffset.rOSD1.i4YOffset));
	/*    IGNORE_RET(_OSD_BASE_SetScrnVStartBotMeg(0x110));*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVStartTopMeg(0x1f));//rOffset.rOSD1.i4YOffset));*/
	/*0x2001C*/
	IGNORE_RET(_OSD_BASE_SetScrnHSizeMain(720));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeMain(576));
	IGNORE_RET(_OSD_BASE_SetScrnHSizeAux(720));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeAux(576));
	/*    IGNORE_RET(_OSD_BASE_SetScrnHSizeMeg(720));*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVSizeMeg(576));*/
	/*config Dgi & Disp path*/

}


void OSD_BASE_SetRES_600P_800(void)  /*add*/
{
	OFFSET_TABLE_T rOffset;
	PMX_RESOLUTION_MODE_T rDspMode;

	rDspMode = RES_600P_800;

	rOffset = rLocationOffset[RES_600P_800];
	/* 0x20008*/
	IGNORE_RET(_OSD_BASE_SetFldPolMeg(1));  /* ??*/
	IGNORE_RET(_OSD_BASE_SetOsd1Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd2Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd3Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd4Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd5Prgs(1));
	IGNORE_RET(_OSD_BASE_SetCsrPrgs(1));
	IGNORE_RET(_OSD_BASE_SetFldPol(1));
	IGNORE_RET(_OSD_BASE_SetVsEdge(1));
	IGNORE_RET(_OSD_BASE_SetHsEdge(0));

	/* 0x20010*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd1(rOffset.rOSD1.i4XOffset));/*rOffset.rOSD1.i4XOffset));  // 0x74 = 116*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd2(rOffset.rOSD2.i4XOffset));
	/* 0x20014*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartCsr(rOffset.rOSD1.i4XOffset));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd3(rOffset.rOSD3.i4XOffset));
	/* 0x20040*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd4(rOffset.rOSD4.i4XOffset));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd5(rOffset.rOSD5.i4XOffset));
	/* 0x2000C*/
	IGNORE_RET(_OSD_BASE_SetOvtMain(635));  /* 0x1E0 = 480*/
	IGNORE_RET(_OSD_BASE_SetVsWidthMain(0x01));

	IGNORE_RET(_OSD_BASE_SetHsWidthMain(1));   /* 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetHsWidthAux(1));   /* 1 = 0x01*/
	/*    IGNORE_RET(_OSD_BASE_SetHsWidthMeg(1));   // 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetOhtMain(928));
	IGNORE_RET(_OSD_BASE_SetOhtAux(928));
	/*    IGNORE_RET(_OSD_BASE_SetOhtMeg(858));*/

	IGNORE_RET(_OSD_BASE_SetOvtAux(635));
	IGNORE_RET(_OSD_BASE_SetVsWidthAux(6));
	/*  IGNORE_RET(_OSD_BASE_SetOvtMeg(525));*/
	/*    IGNORE_RET(_OSD_BASE_SetVsWidthMeg(6));*/
	/*0x20018*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotMain(rOffset.rOSD1.i4EvenYOffset));  /* 0x11C = 284->0x10f*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopMain(rOffset.rOSD1.i4YOffset));   /* 0x2A*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotAux(rOffset.rOSD1.i4EvenYOffset));  /* 0x11C = 284*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopAux(rOffset.rOSD1.i4YOffset));   /* 0x2A*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVStartBotMeg(rOffset.rOSD1.i4EvenYOffset));  // 0x11C = 284*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVStartTopMeg(rOffset.rOSD1.i4YOffset));   // 0x2A*/
	/*0x2001C*/
	IGNORE_RET(_OSD_BASE_SetScrnHSizeMain(800));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeMain(600));
	IGNORE_RET(_OSD_BASE_SetScrnHSizeAux(800));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeAux(600));
	/*    IGNORE_RET(_OSD_BASE_SetScrnHSizeMeg(720));*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVSizeMeg(480));*/
	/*config Dgi & Disp path*/

}
OFFSET_TABLE_T _rTm070ddhgOffset = {
	{0x93, 0x1a, 0x0e}, /* OSD5 - remove for 8317*/
	{0x93, 0x1a, 0x0e}, /* OSD4*/
	{0x93, 0x1a, 0x0e}, /* OSD3*/
	{0x93, 0x1a, 0x0e}, /* OSD2*/
	{0x93, 0x1a, 0x0e}, /* OSD1*/
	{0x71, 0x2B, 0x2B}, /* VDP2*/
	{0x71, 0x2B, 0x2B}, /* VDP1*/
};

void OSD_BASE_SetRES_600P_1024(void)  /*add*/
{
	OFFSET_TABLE_T rOffset;
	PMX_RESOLUTION_MODE_T rDspMode;

	rDspMode = RES_600P_1024;

	//if (1 == _u4Tm070ddhg) {
	//	rOffset = _rTm070ddhgOffset;
	//} else {
		rOffset = rLocationOffset[RES_600P_1024];
	//}

	/* 0x20008*/
	IGNORE_RET(_OSD_BASE_SetFldPolMeg(1));  /* ??*/
	IGNORE_RET(_OSD_BASE_SetOsd1Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd2Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd3Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd4Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd5Prgs(1));
	IGNORE_RET(_OSD_BASE_SetCsrPrgs(1));
	IGNORE_RET(_OSD_BASE_SetFldPol(1));
	IGNORE_RET(_OSD_BASE_SetVsEdge(1));
	IGNORE_RET(_OSD_BASE_SetHsEdge(0));

	/* 0x20010*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd1(rOffset.rOSD1.i4XOffset));/*rOffset.rOSD1.i4XOffset));  // 0x74 = 116*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd2(rOffset.rOSD2.i4XOffset));
	/* 0x20014*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartCsr(rOffset.rOSD1.i4XOffset));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd3(rOffset.rOSD3.i4XOffset));
	/* 0x20040*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd4(rOffset.rOSD4.i4XOffset));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd5(rOffset.rOSD5.i4XOffset));
	/* 0x2000C*/
	IGNORE_RET(_OSD_BASE_SetOvtMain(635));  /* 0x1E0 = 480*/
	IGNORE_RET(_OSD_BASE_SetVsWidthMain(0x01));



	IGNORE_RET(_OSD_BASE_SetHsWidthMain(1));   /* 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetHsWidthAux(1));   /* 1 = 0x01*/
	/*    IGNORE_RET(_OSD_BASE_SetHsWidthMeg(1));   // 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetOhtMain(1344));
	IGNORE_RET(_OSD_BASE_SetOhtAux(1344));
	/*    IGNORE_RET(_OSD_BASE_SetOhtMeg(858));*/

	IGNORE_RET(_OSD_BASE_SetOvtAux(635));
	IGNORE_RET(_OSD_BASE_SetVsWidthAux(6));
	/*  IGNORE_RET(_OSD_BASE_SetOvtMeg(525));*/
	/*    IGNORE_RET(_OSD_BASE_SetVsWidthMeg(6));*/
	/*0x20018*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotMain(rOffset.rOSD1.i4EvenYOffset));  /* 0x11C = 284->0x10f*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopMain(rOffset.rOSD1.i4YOffset));   /* 0x2A*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotAux(rOffset.rOSD1.i4EvenYOffset));  /* 0x11C = 284*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopAux(rOffset.rOSD1.i4YOffset));   /* 0x2A*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVStartBotMeg(rOffset.rOSD1.i4EvenYOffset));  // 0x11C = 284*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVStartTopMeg(rOffset.rOSD1.i4YOffset));   // 0x2A*/
	/*0x2001C*/
	IGNORE_RET(_OSD_BASE_SetScrnHSizeMain(1024));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeMain(600));
	IGNORE_RET(_OSD_BASE_SetScrnHSizeAux(1024));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeAux(600));
	/*    IGNORE_RET(_OSD_BASE_SetScrnHSizeMeg(720));*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVSizeMeg(480));*/
	/*config Dgi & Disp path*/

}

void OSD_BASE_SetRES_768P_1024(void)  /*add*/
{
	OFFSET_TABLE_T rOffset;
	PMX_RESOLUTION_MODE_T rDspMode;

	rDspMode = RES_768P_1024;

	rOffset = rLocationOffset[RES_768P_1024];
	/* 0x20008*/
	IGNORE_RET(_OSD_BASE_SetFldPolMeg(1));  /* ??*/
	IGNORE_RET(_OSD_BASE_SetOsd1Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd2Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd3Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd4Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd5Prgs(1));
	IGNORE_RET(_OSD_BASE_SetCsrPrgs(1));
	IGNORE_RET(_OSD_BASE_SetFldPol(1));
	IGNORE_RET(_OSD_BASE_SetVsEdge(1));
	IGNORE_RET(_OSD_BASE_SetHsEdge(0));

	/* 0x20010*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd1(rOffset.rOSD1.i4XOffset));/*rOffset.rOSD1.i4XOffset));  // 0x74 = 116*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd2(rOffset.rOSD2.i4XOffset));
	/* 0x20014*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartCsr(rOffset.rOSD1.i4XOffset));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd3(rOffset.rOSD3.i4XOffset));
	/* 0x20040*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd4(rOffset.rOSD4.i4XOffset));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd5(rOffset.rOSD5.i4XOffset));
	/* 0x2000C*/
	IGNORE_RET(_OSD_BASE_SetOvtMain(800));  /* 0x1E0 = 480*/
	IGNORE_RET(_OSD_BASE_SetVsWidthMain(0x01));



	IGNORE_RET(_OSD_BASE_SetHsWidthMain(1));   /* 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetHsWidthAux(1));   /* 1 = 0x01*/
	/*    IGNORE_RET(_OSD_BASE_SetHsWidthMeg(1));   // 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetOhtMain(2084));
	IGNORE_RET(_OSD_BASE_SetOhtAux(2084));
	/*    IGNORE_RET(_OSD_BASE_SetOhtMeg(858));*/

	IGNORE_RET(_OSD_BASE_SetOvtAux(800));
	IGNORE_RET(_OSD_BASE_SetVsWidthAux(6));
	/*  IGNORE_RET(_OSD_BASE_SetOvtMeg(525));*/
	/*    IGNORE_RET(_OSD_BASE_SetVsWidthMeg(6));*/
	/*0x20018*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotMain(rOffset.rOSD1.i4EvenYOffset));  /* 0x11C = 284->0x10f*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopMain(rOffset.rOSD1.i4YOffset));   /* 0x2A*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotAux(rOffset.rOSD1.i4EvenYOffset));  /* 0x11C = 284*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopAux(rOffset.rOSD1.i4YOffset));   /* 0x2A*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVStartBotMeg(rOffset.rOSD1.i4EvenYOffset));  // 0x11C = 284*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVStartTopMeg(rOffset.rOSD1.i4YOffset));   // 0x2A*/
	/*0x2001C*/
	IGNORE_RET(_OSD_BASE_SetScrnHSizeMain(1024));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeMain(768));
	IGNORE_RET(_OSD_BASE_SetScrnHSizeAux(1024));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeAux(768));
	/*    IGNORE_RET(_OSD_BASE_SetScrnHSizeMeg(720));*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVSizeMeg(480));*/
	/*config Dgi & Disp path*/

}


void OSD_BASE_SetRES_800P_1280(void)
{
	OFFSET_TABLE_T rOffset;
	PMX_RESOLUTION_MODE_T rDspMode;

	rDspMode = RES_800P_1280;

	rOffset = rLocationOffset[RES_800P_1280];
	/* 0x20008*/
	IGNORE_RET(_OSD_BASE_SetFldPolMeg(1));  /* ??*/
	IGNORE_RET(_OSD_BASE_SetOsd1Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd2Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd3Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd4Prgs(1));
	IGNORE_RET(_OSD_BASE_SetOsd5Prgs(1));
	IGNORE_RET(_OSD_BASE_SetCsrPrgs(1));
	IGNORE_RET(_OSD_BASE_SetFldPol(1));
	IGNORE_RET(_OSD_BASE_SetVsEdge(1));
	IGNORE_RET(_OSD_BASE_SetHsEdge(0));

	/* 0x20010*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd1(rOffset.rOSD1.i4XOffset));/*rOffset.rOSD1.i4XOffset));  // 0x74 = 116*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd2(rOffset.rOSD2.i4XOffset));
	/* 0x20014*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartCsr(rOffset.rOSD1.i4XOffset));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd3(rOffset.rOSD3.i4XOffset));
	/* 0x20040*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd4(rOffset.rOSD4.i4XOffset));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd5(rOffset.rOSD5.i4XOffset));
	/* 0x2000C*/
	IGNORE_RET(_OSD_BASE_SetOvtMain(875));  /* 0x1E0 = 480*/
	IGNORE_RET(_OSD_BASE_SetVsWidthMain(0x06));
#ifdef CONFIG_IC_MT8530

	IGNORE_RET(_OSD_BASE_SetHsWidthMain(1));   /* 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetHsWidthAux(1));   /* 1 = 0x01*/
	/*    IGNORE_RET(_OSD_BASE_SetHsWidthMeg(1));   // 1 = 0x01*/
	IGNORE_RET(_OSD_BASE_SetOhtMain(1380));
	IGNORE_RET(_OSD_BASE_SetOhtAux(1380));
	/*    IGNORE_RET(_OSD_BASE_SetOhtMeg(858));*/
#endif
	IGNORE_RET(_OSD_BASE_SetOvtAux(875));
	IGNORE_RET(_OSD_BASE_SetVsWidthAux(6));
	/*  IGNORE_RET(_OSD_BASE_SetOvtMeg(525));*/
	/*    IGNORE_RET(_OSD_BASE_SetVsWidthMeg(6));*/
	/*0x20018*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotMain(rOffset.rOSD1.i4EvenYOffset));  /* 0x11C = 284->0x10f*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopMain(rOffset.rOSD1.i4YOffset));   /* 0x2A*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartBotAux(rOffset.rOSD1.i4EvenYOffset));  /* 0x11C = 284*/
	IGNORE_RET(_OSD_BASE_SetScrnVStartTopAux(rOffset.rOSD1.i4YOffset));   /* 0x2A*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVStartBotMeg(rOffset.rOSD1.i4EvenYOffset));  // 0x11C = 284*/
	/*    IGNORE_RET(_OSD_BASE_SetScrnVStartTopMeg(rOffset.rOSD1.i4YOffset));   // 0x2A*/
	/*0x2001C*/
	IGNORE_RET(_OSD_BASE_SetScrnHSizeMain(1280));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeMain(800));
	IGNORE_RET(_OSD_BASE_SetScrnHSizeAux(1280));
	IGNORE_RET(_OSD_BASE_SetScrnVSizeAux(800));

}

/*-----------------------------------------------------------------------------*/
/** Brief: for FPGA emulation
*  @param
*  @return
*/
/*-----------------------------------------------------------------------------*/
__s32 OSD_BASE_SetDisplayMode_InVsync(__u32 u4DisplayMode)
{
	/*    __u32  i;*/
	/*OFFSET_TABLE_T rOffset;*/
	/*PMX_RESOLUTION_MODE_T rDspMode;*/

	_u4CurrentDispMode = u4DisplayMode;

	FB_PRINT(FB_LOG_LVL_INFO, "[osd_base_if] rDspMode =%d.\n", (int)u4DisplayMode);

	switch (u4DisplayMode) {
	case   RES_480I:
		OSD_BASE_SetRES_480I();
		break;

	case   RES_480P:
		OSD_BASE_SetRES_480P();
		break;

	case   RES_480P_800:
		OSD_BASE_SetRES_480P_800();
		break;

	case   RES_576I:
		OSD_BASE_SetRES_576I();
		break;

	case   RES_576P:
		OSD_BASE_SetRES_576P();
		break;

	case   RES_600P_800:         /* add*/
		OSD_BASE_SetRES_600P_800();
		break;

	case   RES_600P_1024:         /* add*/
		OSD_BASE_SetRES_600P_1024();
		break;

	case RES_768P_1024:
		OSD_BASE_SetRES_768P_1024();
		break;

	case RES_800P_1280:
		OSD_BASE_SetRES_800P_1280();
		break;

	default:
		break;
	}

	IGNORE_RET(_OSD_BASE_SetVsEdge(1));

	/*    IGNORE_RET(_OSD_BASE_SetScrnVSizeMeg(1080));*/


	return (__s32)OSD_RET_OK;
}

__s32 OSD_BASE_SetDisplayMode(__u32 u4DisplayMode)
{
	/*__s32 i4Ret;*/
	/*_rOsdSetDispMdFlag.fgNeedChangeResolution = TRUE;*/
	/*_rOsdSetDispMdFlag.i4DisplayMode = (__s32) u4DisplayMode;*/
	OSD_BASE_SetDisplayMode_InVsync(u4DisplayMode);

	return (__s32)OSD_RET_OK;
}


/*This function is not used*/
__s32 OSD_BASE_SetOSDOrder(__u32 u4OSDOrder[6])
{
	__u32 u4OsdOrder[6];
	__u32 u4i;
	__u32 u4iDefaultValue = 0x98;

	for (u4i = 0; u4i < 6; u4i++) {
		u4OsdOrder[u4i] = u4OSDOrder[u4i];

		if (u4OsdOrder[u4i] > 5) {
			return OSD_RET_INV_PLANE;
		}
	}

	/*get display mode Default Value*/
	u4iDefaultValue = u4DispModeScrnHStart[_u4CurrentDispMode];
	/* 0x20010*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd1(u4iDefaultValue - u4OsdOrder[1])); /* 0x74 = 116*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd2((u4iDefaultValue - u4OsdOrder[2])));
	/* 0x20014*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartCsr((u4iDefaultValue - u4OsdOrder[0])));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd3((u4iDefaultValue - u4OsdOrder[3])));
	/* 0x20040*/
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd4((u4iDefaultValue - u4OsdOrder[4])));
	IGNORE_RET(_OSD_BASE_SetScrnHStartOsd5((u4iDefaultValue - u4OsdOrder[5])));
	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
*  @param
*  @return
*/
/*-----------------------------------------------------------------------------*/
__s32 OSD_BASE_SetOsdPosition(__u32 u4Plane, __u32 u4X, __u32 u4Y)
{

	OSD_VERIFY_PLANE(u4Plane);

	if (u4Plane == (__u32)OSD_PLANE_1) {
		IGNORE_RET(_OSD_BASE_SetOsd1HStart(u4X));
		IGNORE_RET(_OSD_BASE_SetOsd1VStart(u4Y));
		/*        OSD_SC_UpdateDstSize((__u32)OSD_SCALER_1);*/
	} else if (u4Plane == (__u32)OSD_PLANE_2) {
		IGNORE_RET(_OSD_BASE_SetOsd2HStart(u4X));
		IGNORE_RET(_OSD_BASE_SetOsd2VStart(u4Y));
		/*        OSD_SC_UpdateDstSize((__u32)OSD_SCALER_2);*/
	}

	else if (u4Plane == (__u32)OSD_PLANE_3) {
		IGNORE_RET(_OSD_BASE_SetOsd3HStart(u4X));
		IGNORE_RET(_OSD_BASE_SetOsd3VStart(u4Y));
		/*        OSD_SC_UpdateDstSize((__u32)OSD_SCALER_3);*/
	} else if (u4Plane == (__u32)OSD_PLANE_4) {
		IGNORE_RET(_OSD_BASE_SetOsd4HStart(u4X));
		IGNORE_RET(_OSD_BASE_SetOsd4VStart(u4Y));
		/*        OSD_SC_UpdateDstSize((__u32)OSD_SCALER_4);*/

	} else if (u4Plane == (__u32)OSD_PLANE_5) {
		IGNORE_RET(_OSD_BASE_SetOsd5HStart(u4X));
		IGNORE_RET(_OSD_BASE_SetOsd5VStart(u4Y));
	} else if (u4Plane == (__u32)OSD_PLANE_6) {
		IGNORE_RET(_OSD_BASE_SetOsd2HStart(u4X));
		IGNORE_RET(_OSD_BASE_SetOsd2VStart(u4Y));
		/*        OSD_SC_UpdateDstSize((__u32)OSD_SCALER_6);*/
	} else if (u4Plane == (__u32)OSD_PLANE_7) {
		IGNORE_RET(_OSD_R_BASE_SetOsd7HStart(u4X));
		IGNORE_RET(_OSD_R_BASE_SetOsd7VStart(u4Y));
		/*       OSD_SC_UpdateDstSize((__u32)OSD_SCALER_7);*/
	} else if (u4Plane == (__u32)OSD_PLANE_8) {
		IGNORE_RET(_OSD_R_BASE_SetOsd8HStart(u4X));
		IGNORE_RET(_OSD_R_BASE_SetOsd8VStart(u4Y));
		/*       OSD_SC_UpdateDstSize((__u32)OSD_SCALER_8);*/

	}

	if (u4Plane < OSD_PLANE_6) {
		VERIFY((__s32)OSD_RET_OK == _OSD_BASE_UpdateHwReg());
	} else {
		VERIFY((__s32)OSD_RET_OK == _OSD_R_BASE_UpdateHwReg());
	}


	return (__s32)OSD_RET_OK;
}
EXPORT_SYMBOL(OSD_BASE_SetOsdPosition);


/*-----------------------------------------------------------------------------*/
/** Brief
*  @param
*  @return
*/
/*-----------------------------------------------------------------------------*/
__s32 OSD_BASE_GetOsdPosition(__u32 u4Plane, __u32 *pu4X, __u32 *pu4Y)
{
	OSD_VERIFY_PLANE(u4Plane);

	if (u4Plane == (__u32)OSD_PLANE_1) {
		IGNORE_RET(_OSD_BASE_GetOsd1HStart(pu4X));
		IGNORE_RET(_OSD_BASE_GetOsd1VStart(pu4Y));
	} else if (u4Plane == (__u32)OSD_PLANE_2) {
		IGNORE_RET(_OSD_BASE_GetOsd2HStart(pu4X));
		IGNORE_RET(_OSD_BASE_GetOsd2VStart(pu4Y));
	}

	else if (u4Plane == (__u32)OSD_PLANE_3) {
		IGNORE_RET(_OSD_BASE_GetOsd3HStart(pu4X));
		IGNORE_RET(_OSD_BASE_GetOsd3VStart(pu4Y));
	} else if (u4Plane == (__u32)OSD_PLANE_5) {
		IGNORE_RET(_OSD_BASE_GetOsd5HStart(pu4X));
		IGNORE_RET(_OSD_BASE_GetOsd5VStart(pu4Y));
	}


	return (__s32)OSD_RET_OK;
}

__s32 OSD_BASE_GetPrgs(__u32 u4Plane, __u32 *puPrgs)
{
	OSD_BASE_GET_FX  afnOsdGetPrgs[5] = {
		_OSD_BASE_GetOsd1Prgs,
		_OSD_BASE_GetOsd2Prgs,
		_OSD_BASE_GetOsd3Prgs,
		_OSD_BASE_GetOsd4Prgs,
		_OSD_BASE_GetOsd5Prgs
	};

	/*OSD_VERIFY_PLANE(u4Plane);*/

	if (u4Plane >= OSD_PLANE_6) {
		return -(__s32)OSD_RET_INV_PLANE;
	}

	(*afnOsdGetPrgs[u4Plane])(puPrgs);

	return (__s32)OSD_RET_OK;
}



/*/////////////////////////////////////////////////////////////////////*/
/**/
/**/
/**/
/**/
/**/
/*/////////////////////////////////////////////////////////////////////*/

/*-----------------------------------------------------------------------------*/
/** Brief
*  @param
*  @return
*/
/*-----------------------------------------------------------------------------*/
#if 1

static __u32 _u4OsdRCurrentDispMode = 2;

/*-----------------------------------------------------------------------------*/
/** Brief: for FPGA emulation
*  @param
*  @return
*/
/*-----------------------------------------------------------------------------*/
__u32 OSD_R_BASE_GetDisplayMode(void)
{
	return _u4OsdRCurrentDispMode;
}

void OSD_R_BASE_SetRES_480I(void)
{
	OFFSET_TABLE_T rOffset;
	PMX_RESOLUTION_MODE_T rDspMode;

	rDspMode = RES_480I;
#if OSD_CFG_IS_REAL_INTERLACE
	rOffset = rLocationOffset[RES_480I];
	/* 0x20008*/
	IGNORE_RET(_OSD_R_BASE_SetOsd6Prgs(0));
	IGNORE_RET(_OSD_R_BASE_SetOsd7Prgs(0));
	IGNORE_RET(_OSD_R_BASE_SetOsd8Prgs(0));
	IGNORE_RET(_OSD_R_BASE_SetFldPolMeg(1));
	IGNORE_RET(_OSD_R_BASE_SetFldPol(1));
	IGNORE_RET(_OSD_R_BASE_SetHsEdge(1));
	/* 0x20010*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd6(rOffset.rOSD1.i4XOffset));  /* 0x114 = 276*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd7(rOffset.rOSD2.i4XOffset));
	/* 0x20014*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd8(rOffset.rOSD3.i4XOffset));
	/* 0x20040*/

	/* 0x2000C*/
	IGNORE_RET(_OSD_R_BASE_SetOvtMain(525));  /* 0x1E0 = 480*/
	IGNORE_RET(_OSD_R_BASE_SetVsWidthMain(6));
	IGNORE_RET(_OSD_R_BASE_SetHsWidthMain(1));   /* 1 = 0x01*/
	IGNORE_RET(_OSD_R_BASE_SetHsWidthAux(1));   /* 1 = 0x01*/
	IGNORE_RET(_OSD_R_BASE_SetOhtMain(1716));
	IGNORE_RET(_OSD_R_BASE_SetOhtAux(1716));
	IGNORE_RET(_OSD_R_BASE_SetOvtAux(525));
	IGNORE_RET(_OSD_R_BASE_SetVsWidthAux(6));
	/*0x20018*/
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartBotMain(rOffset.rOSD1.i4EvenYOffset));   /* 0x11C = 284*/
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartTopMain(rOffset.rOSD1.i4YOffset));  /* 0x2C = 44*/
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartBotAux(rOffset.rOSD1.i4EvenYOffset));
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartTopAux(rOffset.rOSD1.i4YOffset));
	/*0x2001C*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHSizeMain(1440));
	IGNORE_RET(_OSD_R_BASE_SetScrnVSizeMain(480));
	IGNORE_RET(_OSD_R_BASE_SetScrnHSizeAux(1440));
	IGNORE_RET(_OSD_R_BASE_SetScrnVSizeAux(480));

	IGNORE_RET(_OSD_R_BASE_SetOsd6Dotctl(3));
	IGNORE_RET(_OSD_R_BASE_SetOsd7Dotctl(3));
	IGNORE_RET(_OSD_R_BASE_SetOsd8Dotctl(3));
	IGNORE_RET(_OSD_R_BASE_SetOsd4Dotctl(3));
#else
	rOffset = rLocationOffset[RES_480I];
	/* 0x20008*/
	IGNORE_RET(_OSD_R_BASE_SetFldPolMeg(1));  /* ??*/
	IGNORE_RET(_OSD_R_BASE_SetOsd6Prgs(1));
	IGNORE_RET(_OSD_R_BASE_SetOsd7Prgs(1));
	IGNORE_RET(_OSD_R_BASE_SetOsd8Prgs(1));
	IGNORE_RET(_OSD_R_BASE_SetFldPol(1));
	IGNORE_RET(_OSD_R_BASE_SetVsEdge(1));
	IGNORE_RET(_OSD_R_BASE_SetHsEdge(0));

	/* 0x20010*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd6(rOffset.rOSD1.i4XOffset));/*rOffset.rOSD1.i4XOffset));  // 0x74 = 116*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd7(rOffset.rOSD2.i4XOffset));/*rOffset.rOSD2.i4XOffset));*/
	/* 0x20014*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd8(rOffset.rOSD3.i4XOffset));
	/* 0x20040*/
	/* 0x2000C*/
	IGNORE_RET(_OSD_R_BASE_SetOvtMain(525));  /* 0x1E0 = 480*/
	IGNORE_RET(_OSD_R_BASE_SetVsWidthMain(6));
	IGNORE_RET(_OSD_R_BASE_SetHsWidthMain(1));   /* 1 = 0x01*/
	IGNORE_RET(_OSD_R_BASE_SetHsWidthAux(1));   /* 1 = 0x01*/
	/*        IGNORE_RET(_OSD_R_BASE_SetHsWidthMeg(1));   // 1 = 0x01*/
	IGNORE_RET(_OSD_R_BASE_SetOhtMain(858));
	IGNORE_RET(_OSD_R_BASE_SetOhtAux(858));
	/*        IGNORE_RET(_OSD_R_BASE_SetOhtMeg(858));*/
	IGNORE_RET(_OSD_R_BASE_SetOvtAux(525));
	IGNORE_RET(_OSD_R_BASE_SetVsWidthAux(6));
	/*      IGNORE_RET(_OSD_R_BASE_SetOvtMeg(525));*/
	/*        IGNORE_RET(_OSD_R_BASE_SetVsWidthMeg(6));*/
	/*0x20018*/
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartBotMain(rOffset.rOSD1.i4EvenYOffset));  /* 0x11C = 284->0x124*/
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartTopMain(rOffset.rOSD1.i4YOffset));   /* 0x2A*/
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartBotAux(rOffset.rOSD1.i4EvenYOffset));  /* 0x11C = 284*/
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartTopAux(rOffset.rOSD1.i4YOffset));   /* 0x2A*/
	/*        IGNORE_RET(_OSD_R_BASE_SetScrnVStartBotMeg(rOffset.rOSD1.i4EvenYOffset));  // 0x11C = 284*/
	/*        IGNORE_RET(_OSD_R_BASE_SetScrnVStartTopMeg(rOffset.rOSD1.i4YOffset));   // 0x2A*/
	/*0x2001C*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHSizeMain(720));
	IGNORE_RET(_OSD_R_BASE_SetScrnVSizeMain(480));
	IGNORE_RET(_OSD_R_BASE_SetScrnHSizeAux(720));
	IGNORE_RET(_OSD_R_BASE_SetScrnVSizeAux(480));
	/*        IGNORE_RET(_OSD_R_BASE_SetScrnHSizeMeg(720));*/
	/*        IGNORE_RET(_OSD_R_BASE_SetScrnVSizeMeg(480));*/
#endif

}


void OSD_R_BASE_SetRES_480P(void)
{
	OFFSET_TABLE_T rOffset;
	PMX_RESOLUTION_MODE_T rDspMode;

	rDspMode = RES_480P;

	rOffset = rLocationOffset[RES_480P];
	/* 0x20008*/
	IGNORE_RET(_OSD_R_BASE_SetFldPolMeg(1));  /* ??*/
	IGNORE_RET(_OSD_R_BASE_SetOsd6Prgs(1));
	IGNORE_RET(_OSD_R_BASE_SetOsd7Prgs(1));
	IGNORE_RET(_OSD_R_BASE_SetOsd8Prgs(1));
	IGNORE_RET(_OSD_R_BASE_SetFldPol(1));
	IGNORE_RET(_OSD_R_BASE_SetVsEdge(1));
	IGNORE_RET(_OSD_R_BASE_SetHsEdge(0));

	/* 0x20010*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd6(rOffset.rOSD1.i4XOffset));/*rOffset.rOSD1.i4XOffset));  // 0x74 = 116*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd7(rOffset.rOSD2.i4XOffset));
	/* 0x20014*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd8(rOffset.rOSD3.i4XOffset));
	/* 0x20040*/
	/*    IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd5(rOffset.rOSD5.i4XOffset));*/
	/* 0x2000C*/
	IGNORE_RET(_OSD_R_BASE_SetOvtMain(525));  /* 0x1E0 = 480*/
	IGNORE_RET(_OSD_R_BASE_SetVsWidthMain(0x1a));
	IGNORE_RET(_OSD_R_BASE_SetHsWidthMain(1));   /* 1 = 0x01*/
	IGNORE_RET(_OSD_R_BASE_SetHsWidthAux(1));   /* 1 = 0x01*/
	/*    IGNORE_RET(_OSD_R_BASE_SetHsWidthMeg(1));   // 1 = 0x01*/
	IGNORE_RET(_OSD_R_BASE_SetOhtMain(858));
	IGNORE_RET(_OSD_R_BASE_SetOhtAux(858));
	/*    IGNORE_RET(_OSD_R_BASE_SetOhtMeg(858));*/
	IGNORE_RET(_OSD_R_BASE_SetOvtAux(525));
	IGNORE_RET(_OSD_R_BASE_SetVsWidthAux(6));
	/*  IGNORE_RET(_OSD_R_BASE_SetOvtMeg(525));*/
	/*    IGNORE_RET(_OSD_R_BASE_SetVsWidthMeg(6));*/
	/*0x20018*/
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartBotMain(rOffset.rOSD1.i4EvenYOffset));  /* 0x11C = 284->0x10f*/
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartTopMain(rOffset.rOSD1.i4YOffset));   /* 0x2A*/
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartBotAux(rOffset.rOSD1.i4EvenYOffset));  /* 0x11C = 284*/
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartTopAux(rOffset.rOSD1.i4YOffset));   /* 0x2A*/
	/*    IGNORE_RET(_OSD_R_BASE_SetScrnVStartBotMeg(rOffset.rOSD1.i4EvenYOffset));  // 0x11C = 284*/
	/*    IGNORE_RET(_OSD_R_BASE_SetScrnVStartTopMeg(rOffset.rOSD1.i4YOffset));   // 0x2A*/
	/*0x2001C*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHSizeMain(720));
	IGNORE_RET(_OSD_R_BASE_SetScrnVSizeMain(480));
	IGNORE_RET(_OSD_R_BASE_SetScrnHSizeAux(720));
	IGNORE_RET(_OSD_R_BASE_SetScrnVSizeAux(480));
	/*    IGNORE_RET(_OSD_R_BASE_SetScrnHSizeMeg(720));*/
	/*    IGNORE_RET(_OSD_R_BASE_SetScrnVSizeMeg(480));*/
	/*config Dgi & Disp path*/

}

void OSD_R_BASE_SetRES_576I(void)
{
	OFFSET_TABLE_T rOffset;
	PMX_RESOLUTION_MODE_T rDspMode;

	rDspMode = RES_576I;

#if OSD_CFG_IS_REAL_INTERLACE
	rOffset = rLocationOffset[RES_576I];
	/* 0x20008*/
	IGNORE_RET(_OSD_R_BASE_SetOsd6Prgs(0));
	IGNORE_RET(_OSD_R_BASE_SetOsd7Prgs(0));
	IGNORE_RET(_OSD_R_BASE_SetOsd8Prgs(0));
	IGNORE_RET(_OSD_R_BASE_SetFldPolMeg(1));
	IGNORE_RET(_OSD_R_BASE_SetHsEdge(1));
	/* 0x2000C*/
	IGNORE_RET(_OSD_R_BASE_SetOvtMain(625));  /* 576 = 0x240*/
	IGNORE_RET(_OSD_R_BASE_SetVsWidthMain(6));
	IGNORE_RET(_OSD_R_BASE_SetHsWidthMain(1));   /* 1 = 0x01*/
	IGNORE_RET(_OSD_R_BASE_SetHsWidthAux(1));   /* 1 = 0x01*/
	/*    IGNORE_RET(_OSD_R_BASE_SetHsWidthMeg(1));   // 1 = 0x01*/
	IGNORE_RET(_OSD_R_BASE_SetOhtMain(1728));
	IGNORE_RET(_OSD_R_BASE_SetOhtAux(1728));
	/*    IGNORE_RET(_OSD_R_BASE_SetOhtMeg(1728));*/
	IGNORE_RET(_OSD_R_BASE_SetOvtAux(625));
	IGNORE_RET(_OSD_R_BASE_SetVsWidthAux(6));
	/* 0x20010*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd6(rOffset.rOSD1.i4XOffset));  /* 137 = 0x89*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd7(rOffset.rOSD2.i4XOffset));
	/* 0x20014*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd8(rOffset.rOSD3.i4XOffset));
	/* 0x20040*/
	/*0x20018*/
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartBotMain(rOffset.rOSD1.i4EvenYOffset));    /*  263 = 0x107*/
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartTopMain(rOffset.rOSD1.i4YOffset));     /*  22 = 0x16*/
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartBotAux(rOffset.rOSD1.i4EvenYOffset));
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartTopAux(rOffset.rOSD1.i4YOffset));
	/*0x2001C*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHSizeMain(1440));
	IGNORE_RET(_OSD_R_BASE_SetScrnVSizeMain(576));
	IGNORE_RET(_OSD_R_BASE_SetScrnHSizeAux(720));
	IGNORE_RET(_OSD_R_BASE_SetScrnVSizeAux(576));
	/*config Dgi & Disp path*/
#else
	rOffset = rLocationOffset[RES_576I];
	/* 0x20008*/
	IGNORE_RET(_OSD_R_BASE_SetOsd6Prgs(1));
	IGNORE_RET(_OSD_R_BASE_SetOsd7Prgs(1));
	IGNORE_RET(_OSD_R_BASE_SetOsd8Prgs(1));
	IGNORE_RET(_OSD_R_BASE_SetFldPolMeg(1));
	IGNORE_RET(_OSD_R_BASE_SetHsEdge(1));
	/* 0x2000C*/
	IGNORE_RET(_OSD_R_BASE_SetOvtMain(625));  /* 576 = 0x240*/
	IGNORE_RET(_OSD_R_BASE_SetVsWidthMain(6));
	IGNORE_RET(_OSD_R_BASE_SetHsWidthMain(1));   /* 1 = 0x01*/
	IGNORE_RET(_OSD_R_BASE_SetHsWidthAux(1));   /* 1 = 0x01*/
	/*    IGNORE_RET(_OSD_R_BASE_SetHsWidthMeg(1));   // 1 = 0x01*/
	IGNORE_RET(_OSD_R_BASE_SetOhtMain(864));
	IGNORE_RET(_OSD_R_BASE_SetOhtAux(864));
	/*    IGNORE_RET(_OSD_R_BASE_SetOhtMeg(864));*/
	IGNORE_RET(_OSD_R_BASE_SetOvtAux(625));
	IGNORE_RET(_OSD_R_BASE_SetVsWidthAux(6));
	/*  IGNORE_RET(_OSD_R_BASE_SetOvtMeg(625));*/
	/*    IGNORE_RET(_OSD_R_BASE_SetVsWidthMeg(6));*/
	/* 0x20010*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd6(rOffset.rOSD1.i4XOffset));  /* 144 = 0x90*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd7(rOffset.rOSD2.i4XOffset));
	/* 0x20014*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd8(rOffset.rOSD3.i4XOffset));
	/* 0x20040*/
	/*0x20018*/
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartBotMain(0x110));    /* 285 = 0x11D->0x110*/
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartTopMain(rOffset.rOSD1.i4YOffset));     /* 48 = 0x30*/
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartBotAux(0x110));
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartTopAux(rOffset.rOSD1.i4YOffset));
	/*    IGNORE_RET(_OSD_R_BASE_SetScrnVStartBotMeg(0x110));*/
	/*    IGNORE_RET(_OSD_R_BASE_SetScrnVStartTopMeg(rOffset.rOSD1.i4YOffset));*/
	/*0x2001C*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHSizeMain(720));
	IGNORE_RET(_OSD_R_BASE_SetScrnVSizeMain(576));
	IGNORE_RET(_OSD_R_BASE_SetScrnHSizeAux(720));
	IGNORE_RET(_OSD_R_BASE_SetScrnVSizeAux(576));
	/*    IGNORE_RET(_OSD_R_BASE_SetScrnHSizeMeg(720));*/
	/*    IGNORE_RET(_OSD_R_BASE_SetScrnVSizeMeg(576));*/
	/*config Dgi & Disp path*/

#endif
}

void OSD_R_BASE_SetRES_576P(void)
{
	OFFSET_TABLE_T rOffset;
	PMX_RESOLUTION_MODE_T rDspMode;

	rDspMode = RES_576P;
	rOffset = rLocationOffset[RES_576P];
	/* 0x20008*/
	IGNORE_RET(_OSD_R_BASE_SetOsd6Prgs(1));
	IGNORE_RET(_OSD_R_BASE_SetOsd7Prgs(1));
	IGNORE_RET(_OSD_R_BASE_SetOsd8Prgs(1));
	IGNORE_RET(_OSD_R_BASE_SetFldPolMeg(1));
	IGNORE_RET(_OSD_R_BASE_SetHsEdge(1));
	/* 0x2000C*/
	IGNORE_RET(_OSD_R_BASE_SetOvtMain(625));  /* 576 = 0x240*/
	IGNORE_RET(_OSD_R_BASE_SetVsWidthMain(6));
	IGNORE_RET(_OSD_R_BASE_SetHsWidthMain(1));   /* 1 = 0x01*/
	IGNORE_RET(_OSD_R_BASE_SetHsWidthAux(1));   /* 1 = 0x01*/
	/*    IGNORE_RET(_OSD_R_BASE_SetHsWidthMeg(1));   // 1 = 0x01*/
	IGNORE_RET(_OSD_R_BASE_SetOhtMain(864));
	IGNORE_RET(_OSD_R_BASE_SetOhtAux(864));
	/*    IGNORE_RET(_OSD_R_BASE_SetOhtMeg(864));*/
	IGNORE_RET(_OSD_R_BASE_SetOvtAux(625));
	IGNORE_RET(_OSD_R_BASE_SetVsWidthAux(6));
	/*  IGNORE_RET(_OSD_R_BASE_SetOvtMeg(625));*/
	/*    IGNORE_RET(_OSD_R_BASE_SetVsWidthMeg(6));*/
	/* 0x20010*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd6(rOffset.rOSD1.i4XOffset));  /* 144 = 0x90*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd7(rOffset.rOSD2.i4XOffset));
	/* 0x20014*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd8(rOffset.rOSD3.i4XOffset));
	/* 0x20040*/
	/*0x20018*/
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartBotMain(0x110));    /* 285 = 0x11D->0x10f*/
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartTopMain(rOffset.rOSD1.i4YOffset));     /* 48 = 0x30*/
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartBotAux(0x110));
	IGNORE_RET(_OSD_R_BASE_SetScrnVStartTopAux(rOffset.rOSD1.i4YOffset));
	/*    IGNORE_RET(_OSD_R_BASE_SetScrnVStartBotMeg(0x110));*/
	/*    IGNORE_RET(_OSD_R_BASE_SetScrnVStartTopMeg(0x1f));//rOffset.rOSD1.i4YOffset));*/
	/*0x2001C*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHSizeMain(720));
	IGNORE_RET(_OSD_R_BASE_SetScrnVSizeMain(576));
	IGNORE_RET(_OSD_R_BASE_SetScrnHSizeAux(720));
	IGNORE_RET(_OSD_R_BASE_SetScrnVSizeAux(576));
	/*    IGNORE_RET(_OSD_R_BASE_SetScrnHSizeMeg(720));*/
	/*    IGNORE_RET(_OSD_R_BASE_SetScrnVSizeMeg(576));*/
	/*config Dgi & Disp path*/

}




/*-----------------------------------------------------------------------------*/
/** Brief: for FPGA emulation
*  @param
*  @return
*/
/*-----------------------------------------------------------------------------*/
__s32 OSD_R_BASE_SetDisplayMode_InVsync(__u32 u4DisplayMode)
{
	/*    __u32  i;*/
	/*OFFSET_TABLE_T rOffset;*/
	PMX_RESOLUTION_MODE_T rDspMode;

	rDspMode = (PMX_RESOLUTION_MODE_T)u4DisplayMode;

	if (RES_MODE_NUM <= rDspMode) {
		return  -(__s32)OSD_RET_INV_DISPLAY_MODE;
	}

	_u4CurrentDispMode = u4DisplayMode;

	FB_PRINT(FB_LOG_LVL_INFO, "[osd_base_if] rDspMode =%d.\n", rDspMode);

	switch (rDspMode) {
	case   RES_480I:
		OSD_R_BASE_SetRES_480I();
		break;

	case   RES_480P:
		OSD_R_BASE_SetRES_480P();
		break;

	case   RES_576I:
		OSD_R_BASE_SetRES_576I();
		break;

	case   RES_576P:
		OSD_R_BASE_SetRES_576P();
		break;

	default:
		break;
	}

	IGNORE_RET(_OSD_R_BASE_SetVsEdge(1));


	return (__s32)OSD_RET_OK;
}

__s32 OSD_R_BASE_SetDisplayMode(__u32 u4DisplayMode)
{
	/*__s32 i4Ret;*/
	/*_rOsdSetDispMdFlag.fgNeedChangeResolution = TRUE;*/
	/*_rOsdSetDispMdFlag.i4DisplayMode = (__s32) u4DisplayMode;*/
	OSD_R_BASE_SetDisplayMode_InVsync(u4DisplayMode);

	return (__s32)OSD_RET_OK;
}


/*This function is not used*/
__s32 OSD_R_BASE_SetOSDOrder(__u32 u4OSDOrder[6])
{
	__u32 u4OsdOrder[6];
	__u32 u4i;
	__u32 u4iDefaultValue = 0x98;

	for (u4i = 0; u4i < 6; u4i++) {
		u4OsdOrder[u4i] = u4OSDOrder[u4i];

		if (u4OsdOrder[u4i] > 5) {
			return OSD_RET_INV_PLANE;
		}
	}

	/*get display mode Default Value*/
	u4iDefaultValue = u4DispModeScrnHStart[_u4CurrentDispMode];
	/* 0x20010*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd6(u4iDefaultValue - u4OsdOrder[1])); /* 0x74 = 116*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd7((u4iDefaultValue - u4OsdOrder[2])));
	/* 0x20014*/
	IGNORE_RET(_OSD_R_BASE_SetScrnHStartOsd8((u4iDefaultValue - u4OsdOrder[3])));
	/* 0x20040*/
	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
*  @param
*  @return
*/
/*-----------------------------------------------------------------------------*/
__s32 OSD_R_BASE_SetOsdPosition(__u32 u4Plane, __u32 u4X, __u32 u4Y)
{

	OSD_VERIFY_PLANE(u4Plane);

	if (u4Plane == (__u32)OSD_PLANE_6) {
		IGNORE_RET(_OSD_R_BASE_SetOsd6HStart(u4X));
		IGNORE_RET(_OSD_R_BASE_SetOsd6VStart(u4Y));
	} else if (u4Plane == (__u32)OSD_PLANE_7) {
		IGNORE_RET(_OSD_R_BASE_SetOsd7HStart(u4X));
		IGNORE_RET(_OSD_R_BASE_SetOsd7VStart(u4Y));
	}

	else if (u4Plane == (__u32)OSD_PLANE_8) {
		IGNORE_RET(_OSD_R_BASE_SetOsd8HStart(u4X));
		IGNORE_RET(_OSD_R_BASE_SetOsd8VStart(u4Y));
	}

	VERIFY((__s32)OSD_RET_OK == _OSD_R_BASE_UpdateHwReg());

	return (__s32)OSD_RET_OK;
}


/*-----------------------------------------------------------------------------*/
/** Brief
*  @param
*  @return
*/
/*-----------------------------------------------------------------------------*/
__s32 OSD_R_BASE_GetOsdPosition(__u32 u4Plane, __u32 *pu4X, __u32 *pu4Y)
{
	OSD_VERIFY_PLANE(u4Plane);

	if (u4Plane == (__u32)OSD_PLANE_6) {
		IGNORE_RET(_OSD_R_BASE_GetOsd6HStart(pu4X));
		IGNORE_RET(_OSD_R_BASE_GetOsd6VStart(pu4Y));
	} else if (u4Plane == (__u32)OSD_PLANE_7) {
		IGNORE_RET(_OSD_R_BASE_GetOsd7HStart(pu4X));
		IGNORE_RET(_OSD_R_BASE_GetOsd7VStart(pu4Y));
	}

	else if (u4Plane == (__u32)OSD_PLANE_8) {
		IGNORE_RET(_OSD_R_BASE_GetOsd8HStart(pu4X));
		IGNORE_RET(_OSD_R_BASE_GetOsd8VStart(pu4Y));
	}

	return (__s32)OSD_RET_OK;
}

__s32 OSD_R_BASE_GetPrgs(__u32 u4Plane, __u32 *puPrgs)
{
	OSD_R_BASE_GET_FX  afnOsdGetPrgs[3] = {
		_OSD_R_BASE_GetOsd6Prgs,
		_OSD_R_BASE_GetOsd7Prgs,
		_OSD_R_BASE_GetOsd8Prgs,
	};

	/*OSD_VERIFY_PLANE(u4Plane);*/

	if (u4Plane >= (OSD_PLANE_MAX_NUM - OSD_PLANE_6)) {
		return -(__s32)OSD_RET_INV_PLANE;
	}

	(*afnOsdGetPrgs[u4Plane])(puPrgs);

	return (__s32)OSD_RET_OK;
}
#endif



