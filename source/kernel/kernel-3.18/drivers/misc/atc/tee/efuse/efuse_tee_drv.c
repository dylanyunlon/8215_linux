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

#include <asm/memory.h>

#include "types.h"

#include "../tee_drv.h"

static struct smc_param *tee_param = NULL; 
extern spinlock_t tee_lock;

bool fgGetChipFeature(unsigned int u4Feature)
{
#ifndef CONFIG_ATC_OS_android
    unsigned int b_support = 0;
    unsigned long flags; 

    //struct smc_param *tee_param = NULL;

    spin_lock_irqsave(&tee_lock, flags);    

    tee_param = (struct smc_param*)kmalloc(sizeof(struct smc_param)+ sizeof(unsigned int), GFP_KERNEL); ;
    memset((void *)tee_param, 0, sizeof(struct smc_param)+ sizeof(unsigned int));   
    
    tee_param->cmd_id = TEE_SMC_CALL_Efuse_GetChipFeature;
    tee_param->ret = 0xFF;
    tee_param->data[0]= u4Feature;

    //memcpy((void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), (void*)tee_param, sizeof(struct smc_param)+ sizeof(unsigned int));
    memcpy((void*)(tz_share_rsv_mem.virt_addr), (void*)tee_param, sizeof(struct smc_param)+ sizeof(unsigned int));
        
    //tee_smc_call(TZ_SHARED_DATA_RESERVED_MEM_PA);
    tee_smc_call(tz_share_rsv_mem.base);
    //memcpy((void*)tee_param, (void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), sizeof(struct smc_param)+ sizeof(unsigned int));
    memcpy((void*)tee_param, (void*)(tz_share_rsv_mem.virt_addr), sizeof(struct smc_param)+ sizeof(unsigned int));
    b_support = ((struct smc_param*)tee_param)->ret;
    //printk("\nTEE DRV fgGetChipFeature: 0x%x\n", b_support);
    kfree(tee_param);
    tee_param = NULL;
    spin_unlock_irqrestore(&tee_lock, flags);
    
    return b_support;
#else
	return 1;
#endif 
	
}
EXPORT_SYMBOL(fgGetChipFeature);

void featureInit(unsigned int u4Feature, unsigned int p2, unsigned int p3, unsigned int p4)
{

    unsigned long flags;    
    //printk("\nTEE DRV featureInit: 0x%x, 0x%x, 0x%x, 0x%x\n", u4Feature, p2, p3, p4);

    spin_lock_irqsave(&tee_lock, flags); 
    tee_param = (struct smc_param*)kmalloc(sizeof(struct smc_param)+ 4*sizeof(unsigned int), GFP_KERNEL);    
    memset((void *)tee_param, 0, sizeof(struct smc_param)+ 4*sizeof(unsigned int));   
    
    tee_param->cmd_id = TEE_SMC_CALL_Efuse_FeatureInit;
    tee_param->ret = 0xFF;
    tee_param->data[0] = u4Feature;
    tee_param->data[1] = p2;
    tee_param->data[2] = p3;
    tee_param->data[3] = p4;
    //memcpy((void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), (void*)tee_param, sizeof(struct smc_param)+ 4*sizeof(unsigned int));
        
    //tee_smc_call(TZ_SHARED_DATA_RESERVED_MEM_PA);
    //memcpy((void*)tee_param, (void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), sizeof(struct smc_param)+ 4*sizeof(unsigned int));
    
    memcpy((void*)(tz_share_rsv_mem.virt_addr), (void*)tee_param, sizeof(struct smc_param)+ 4*sizeof(unsigned int));
        
    tee_smc_call(tz_share_rsv_mem.base);
    memcpy((void*)tee_param, (void*)(tz_share_rsv_mem.virt_addr), sizeof(struct smc_param)+ 4*sizeof(unsigned int));
    
    kfree(tee_param);
    tee_param = NULL;
    spin_unlock_irqrestore(&tee_lock, flags);

	return;	
}
EXPORT_SYMBOL(featureInit);


