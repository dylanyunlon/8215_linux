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

#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <linux/timer.h>
#include <linux/module.h>


/* #ifdef CC_UP8032_ATV */
/* #include "general_mt82.h" */
/* #endif */
/* #include "general.h" */
#include "x_typedef.h"
#include "x_os.h"
#include "x_printf.h"
#include "x_stl_lib.h"
#include "x_assert.h"
#include "x_bim.h"
#include "drv_thread.h"
/* #include "drv_av_d.h" */
#include "x_timer.h"
#include "vga_table.h"
#include "video_timing.h"
#include "atc_vga.h"

#define _VGA_TABLE_C_
#define VGAMODE_OFFSET 0
#define VGACAPTURE_OFFSET (VGAMODE_OFFSET + (UINT16)MAX_TIMING_FORMAT * sizeof(VGAMODE))
/* above,  79 * 14 = 1106 */

#define ADCPLL_WORKAROUND   1
#define OVERSAMPLE_THRESHOLD   250 /*  250//350 //25MHz */

#if SUPPORT_VGA_USERMODE
VGA_USRMODE rVgaUsrEEP[USERMODE_TIMING];        /* both  EEP & RAM */
VGA_USRMODE_EXT rVgaUsrExt[USERMODE_TIMING]; /* only on RAM */
#endif


/* const UINT8 bHdtvTimings = HDTV_TIMING_NUM; */
/* extern const UINT8 bHdtvTimings; */
const UINT8 bUserHDMITimings = USERMODE_TIMING;
const UINT8 bAllHDMITimings = (UINT8)(sizeof(HDMITIMING_TABLE) / sizeof(VGAMODE));
const UINT8 bHDMITimings = (ALL_TIMING_NUM - HDTV_TIMING_NUM - USERMODE_TIMING);
const UINT8 bUserHDMITimingBegin = (ALL_TIMING_NUM - USERMODE_TIMING);


UINT16 Get_HDMIMODE_IHF(UINT8 mode)
{
	return HDMITIMING_TABLE[mode].IHF;
}

UINT8 Get_HDMIMODE_IVF(UINT8 mode)
{
	return HDMITIMING_TABLE[mode].IVF;
}

UINT16 Get_HDMIMODE_ICLK(UINT8 mode)
{
#if ADCPLL_WORKAROUND

	if (mode == 0xFF)
		return 0;

	if (fgIsVgaTiming(mode) && (HDMITIMING_TABLE[mode].ICLK < OVERSAMPLE_THRESHOLD))
		return (HDMITIMING_TABLE[mode].ICLK * 2);
#endif
	return HDMITIMING_TABLE[mode].ICLK;
}

UINT16 Get_HDMIMODE_IHTOTAL(UINT8 mode)
{
#if ADCPLL_WORKAROUND

	if (mode == 0xFF)
		return 0;

	if (fgIsVgaTiming(mode) && (HDMITIMING_TABLE[mode].ICLK < OVERSAMPLE_THRESHOLD))
		return (HDMITIMING_TABLE[mode].IHTOTAL * 2);
#endif
	return HDMITIMING_TABLE[mode].IHTOTAL;
}

UINT16 Get_HDMIMODE_IVTOTAL(UINT8 mode)
{
	return HDMITIMING_TABLE[mode].IVTOTAL;
}

UINT16 Get_HDMIMODE_IPH_STA(UINT8 mode)
{
#if ADCPLL_WORKAROUND

	if (mode == 0xFF)
		return 0;

	if (fgIsVgaTiming(mode) && (HDMITIMING_TABLE[mode].ICLK < OVERSAMPLE_THRESHOLD))
		return ((HDMITIMING_TABLE[mode].IPH_BP + HDMITIMING_TABLE[mode].IPH_SYNCW) * 2);
#endif
	return (HDMITIMING_TABLE[mode].IPH_BP + HDMITIMING_TABLE[mode].IPH_SYNCW);
}

UINT16 Get_HDMIMODE_IPH_WID(UINT8 mode)
{
#if ADCPLL_WORKAROUND

	if (mode == 0xFF)
		return 0;

	if (fgIsVgaTiming(mode) && (HDMITIMING_TABLE[mode].ICLK < OVERSAMPLE_THRESHOLD))
		return (HDMITIMING_TABLE[mode].IPH_WID * 2);
#endif
	return HDMITIMING_TABLE[mode].IPH_WID;
}

UINT16 Get_HDMIMODE_IPH_SYNCW(UINT8 mode)
{
#if ADCPLL_WORKAROUND

	if (mode == 0xFF)
		return 0;

	if (fgIsVgaTiming(mode) && (HDMITIMING_TABLE[mode].ICLK < OVERSAMPLE_THRESHOLD))
		return (HDMITIMING_TABLE[mode].IPH_SYNCW * 2);
#endif
	return HDMITIMING_TABLE[mode].IPH_SYNCW;
}

UINT16 Get_HDMIMODE_IPH_BP(UINT8 mode)
{
#if ADCPLL_WORKAROUND

	if (mode == 0xFF)
		return 0;

	if (fgIsVgaTiming(mode) && (HDMITIMING_TABLE[mode].ICLK < OVERSAMPLE_THRESHOLD))
		return (HDMITIMING_TABLE[mode].IPH_BP * 2);
#endif
	return HDMITIMING_TABLE[mode].IPH_BP;
}

UINT8 Get_HDMIMODE_IPV_STA(UINT8 mode)
{
	return HDMITIMING_TABLE[mode].IPV_STA; /* -Displaymode_delay; //Modify for Disaplymode delay */
}

UINT16 Get_HDMIMODE_IPV_LEN(UINT8 mode)
{
	return HDMITIMING_TABLE[mode].IPV_LEN;
}

UINT16 Get_HDMIMODE_COMBINE(UINT8 mode)
{
	return HDMITIMING_TABLE[mode].COMBINE;
}

UINT8 Get_HDMIMODE_OverSample(UINT8 mode)
{
#if ADCPLL_WORKAROUND

	if (mode == 0xFF)
		return 0;

	if (fgIsVgaTiming(mode) && (HDMITIMING_TABLE[mode].ICLK < OVERSAMPLE_THRESHOLD)) {
		return 1;        /* alwasy oversample */
	}
#endif
	return (HDMITIMING_TABLE[mode].COMBINE & 0x01);
}
