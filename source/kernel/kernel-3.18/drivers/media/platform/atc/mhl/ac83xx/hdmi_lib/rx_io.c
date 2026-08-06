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

#include "x_lint.h"
#include "x_typedef.h"
#include "x_os.h"
#include "x_printf.h"
/*#include "x_gpio.h"*/
/*#include "x_bim.h"*/
/*#include "x_printf.h"*/
#include "x_assert.h"
/*#include "drv_av_d.h"*/
/*#include "drv_sif_sw.h"*/
/*#include "drv_if_exthdmi.h"*/
/*#include "tx9134ctrl.h"*/
/*#include "tx9134hw.h"*/
#include "rx_io.h"
#include "hal_io.h"

#define M1_V1_BOARD


void Delay5MS(UINT32 count)
{
	UINT32 u4Index;

	for (u4Index = 0; u4Index < count; u4Index++) {
		msleep(1);
	}
}









