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
#include <linux/spinlock_types.h>
#include <linux/trusty/smcall.h>

#include "types.h"

#include "../tee_drv.h"


extern spinlock_t tee_lock;
extern s32 trusty_atc_std_call32(u32 smcnr, u32 a0, u32 a1, u32 a2);

bool fgGetChipFeature(unsigned int u4Feature)
{
	bool b_support = 0;
	unsigned long flags; 
	
	//spin_lock_irqsave(&tee_lock, flags);	

	b_support = trusty_atc_std_call32(ATC_GET_CHIP_FEATURE, u4Feature, 0xFFFF, 0xFFFF);
	//printk("[Trusty]fgGetChipFeature FEATURE: 0x%x, support: %d", u4Feature, b_support);
	
	//spin_unlock_irqrestore(&tee_lock, flags);
	return b_support; 
}
EXPORT_SYMBOL(fgGetChipFeature);

void featureInit(unsigned int u4Feature, unsigned int p2, unsigned int p3, unsigned int p4)
{
	unsigned long flags; 	
	int ret = 0;
	//spin_lock_irqsave(&tee_lock, flags);	
	
	ret = trusty_atc_std_call32(ATC_FEATURE_INIT, u4Feature, p2, p3);
	//printk("[Trusty]featureInit FEATURE: 0x%x, ret: %d", u4Feature, ret);
	//spin_unlock_irqrestore(&tee_lock, flags);
}

EXPORT_SYMBOL(featureInit);
