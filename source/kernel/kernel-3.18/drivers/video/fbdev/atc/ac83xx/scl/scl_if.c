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
#include <linux/module.h>
#include <media/atc/display_inc.h>
#else
#include "display_inc.h"
#endif
#include "scl_hal.h"
#include "scl_if.h"
#include "log.h"

void SCL_Init(bool fgHwReset)
{
	vSclHalInit(fgHwReset);
}
EXPORT_SYMBOL(SCL_Init);

void  SCL_Config(void)
{
	vSclHalSetMode();
#if MASTER_MODE_ENABLE
	vSclHalSetMasterMode(TRUE);
#else
	vSclHalSetMasterMode(FALSE);
#endif
}

EXPORT_SYMBOL(SCL_Config);














