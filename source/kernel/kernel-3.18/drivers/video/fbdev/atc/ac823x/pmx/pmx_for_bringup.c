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
#ifndef __ARM2__
#include <linux/printk.h>
#include <linux/dma-mapping.h>

#include "pmx_vfy_drv.h"

unsigned long IO_BASE_BRINGUP;

void getiobaseva(void)
{
	IO_BASE_BRINGUP = ioremap(0x10000000, 0x400000);
        printk("wts IO_BASE_BRINGUP = 0x%llx", IO_BASE_BRINGUP);
	if (IO_BASE_BRINGUP == 0)
		printk("wts IO_BASE_BRINGUP = 0x%llx", IO_BASE_BRINGUP);
}

extern void getpmxbaseaddr(void); //in pmx_hal.c

void bringupinit3365(void)
{
	getiobaseva();
	getpmxbaseaddr();
	PmxVerifyDrvInit(true);
	PmxVerifySetMode_OSD(0, 600, 600, 600, 600, 13, 1, 0, 0, 1, 0, FALSE);
}
#endif
