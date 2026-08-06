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
/*
 *Department: ATC-SD-SD2
 *
 *Aurthor: yezhiqiang(atc0064)
 *
 *Date: 2017-03-29
 */
#ifndef _CP_IF_H_
#define _CP_IF_H_

#ifndef __ARM2__
#include <linux/types.h>
#include <linux/miscdevice.h>

struct vcp_device{
	struct miscdevice cdev;   /* Char device structure */
	uint32_t dwVcpInst;
#ifdef CONFIG_PM
	struct device * dev;
#endif
};
#endif

extern unsigned long _IO_BASE_;

#define FRONT   0   /* refer to front row display */
#define REAR    1   /* refer to rear row display */

void VcpSetHue(u32 u4VcpIdx, u32 u4Hue);
void VcpSetYGain(u32 u4VcpIdx, u32 u4YGain);
void VcpSetUGain(u32 u4VcpIdx, u32 u4UGain);
void VcpSetVGain(u32 u4VcpIdx, u32 u4VGain);
void VcpSetContrast(u32 u4VcpIdx, u32 u4Contrast);
void VcpSetBrightness(u32 u4VcpIdx, u32 u4Brightness);
void VcpSetSaturation(u32 u4VcpIdx, u32 u4Saturation);

#endif
