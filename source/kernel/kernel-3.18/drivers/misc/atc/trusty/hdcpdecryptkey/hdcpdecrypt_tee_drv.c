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

#include <linux/module.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <asm/memory.h>
#include <linux/spinlock_types.h>
#include <linux/trusty/smcall.h>

#include "types.h"

#include "../tee_drv.h"

extern spinlock_t tee_lock;

extern s32 trusty_atc_std_call32(u32 smcnr, u32 a0, u32 a1, u32 a2);

int LoadHDCPKeyToSRAM(unsigned char* data, unsigned int len)
{
	unsigned int ret = 0;
	unsigned long flags; 

	spin_lock_irqsave(&tee_lock, flags);	

	memcpy((void*)(tz_share_rsv_mem.virt_addr), (void*)data, len*sizeof(unsigned char));
		
	trusty_atc_std_call32(ATC_HDCP_SAVE2SRAM, tz_share_rsv_mem.base, len, 0xFFFF);

	spin_unlock_irqrestore(&tee_lock, flags);
	
	return ret;
}


EXPORT_SYMBOL(LoadHDCPKeyToSRAM);

