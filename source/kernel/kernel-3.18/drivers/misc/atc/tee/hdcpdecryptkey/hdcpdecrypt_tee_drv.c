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

#include "types.h"

#include "../tee_drv.h"


struct smc_param *hdcp_param = NULL; 
extern spinlock_t tee_lock;

static void tz_hdcp2xDumpHex(const unsigned char *data, int len)
{
    int pos = 0;
#if 0
    if (!data || !len)
    {
        return;
    }
    //fprintf(stderr, "Dumping data\n");

    //if (hdcp2x_log_cli)
    {
        printk("\n------------------------------\n");
        while(pos < len /*len*/) // only print message ID
        {
            if (pos%8 == 0)
                printk("\n[%04x]|", pos);
            printk("0x%02x, ", data[pos]); // secure data, cannot show in mtktool
            pos ++;
        }
        printk("\n------------------------------\n");
    }
#endif
}


int LoadHDCPKeyToSRAM(unsigned char* data, unsigned int len)
{
    unsigned int ret = 0;
    unsigned long flags; 

    //struct smc_param *tee_param = NULL;

    spin_lock_irqsave(&tee_lock, flags);    

    hdcp_param = (struct smc_param*)kmalloc(sizeof(struct smc_param)+ len*sizeof(unsigned char), GFP_KERNEL); ;
    memset((void *)hdcp_param, 0, sizeof(struct smc_param)+ len*sizeof(unsigned char));   
    
    hdcp_param->cmd_id = TEE_SMC_CALL_HDCP_DECRYPT_KEY;
    hdcp_param->ret = 0xFF;

    memcpy(hdcp_param->data, data, len);
    //printk("\nTEE LoadHDCPKeyToSRAM\n");

    //memcpy((void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    //tee_smc_call(TZ_SHARED_DATA_RESERVED_MEM_PA);
    //memcpy((void*)hdcp_param, (void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    
    memcpy((void*)(tz_share_rsv_mem.virt_addr), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    tee_smc_call(tz_share_rsv_mem.base);
    memcpy((void*)hdcp_param, (void*)(tz_share_rsv_mem.virt_addr), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    ret = ((struct smc_param*)hdcp_param)->ret;
    
    kfree(hdcp_param);
    hdcp_param = NULL;
    spin_unlock_irqrestore(&tee_lock, flags);
    
    return ret;
}

EXPORT_SYMBOL(LoadHDCPKeyToSRAM);


int TZ_HDCP2_SetEncDcp2Key(unsigned char *penckey, unsigned int len)
{
    unsigned int ret = 0;
    unsigned long flags; 

    //struct smc_param *tee_param = NULL;

    spin_lock_irqsave(&tee_lock, flags);    

    hdcp_param = (struct smc_param*)kmalloc(sizeof(struct smc_param)+ len*sizeof(unsigned char), GFP_KERNEL);
    memset((void *)hdcp_param, 0, sizeof(struct smc_param)+ len*sizeof(unsigned char));   
    
    hdcp_param->cmd_id = TEE_SMC_CALL_HDCP2_SetEncDcp2Key;
    hdcp_param->ret = 0xFF;

    memcpy(hdcp_param->data, penckey, len);

    //memcpy((void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    //tee_smc_call(TZ_SHARED_DATA_RESERVED_MEM_PA);
    //memcpy((void*)hdcp_param, (void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    
    memcpy((void*)(tz_share_rsv_mem.virt_addr), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    tee_smc_call(tz_share_rsv_mem.base);
    memcpy((void*)hdcp_param, (void*)(tz_share_rsv_mem.virt_addr), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    
    ret = ((struct smc_param*)hdcp_param)->ret;
    
    kfree(hdcp_param);
    hdcp_param = NULL;
    spin_unlock_irqrestore(&tee_lock, flags);
    
    return ret;
}

EXPORT_SYMBOL(TZ_HDCP2_SetEncDcp2Key);

int TZ_HDCP2_GetCertInfo(unsigned char *pdata, unsigned int len)
{
    unsigned int ret = 0;
    unsigned long flags; 

    //struct smc_param *tee_param = NULL;

    spin_lock_irqsave(&tee_lock, flags);    

    hdcp_param = (struct smc_param*)kmalloc(sizeof(struct smc_param)+ len*sizeof(unsigned char), GFP_KERNEL);
    memset((void *)hdcp_param, 0, sizeof(struct smc_param)+ len*sizeof(unsigned char));   
    
    hdcp_param->cmd_id = TEE_SMC_CALL_HDCP2_GetCertInfo;
    hdcp_param->ret = 0xFF;

    memcpy(hdcp_param->data, pdata, len);
    //printk("\nTEE HDCP2_GetCertInfo %s\n", hdcp_param->data);
    //memcpy((void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    //tee_smc_call(TZ_SHARED_DATA_RESERVED_MEM_PA);
    //memcpy((void*)hdcp_param, (void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    
    memcpy((void*)(tz_share_rsv_mem.virt_addr), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    tee_smc_call(tz_share_rsv_mem.base);
    memcpy((void*)hdcp_param, (void*)(tz_share_rsv_mem.virt_addr), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    memcpy((void*)pdata, (void*)hdcp_param->data, len*sizeof(unsigned char));
    ret = ((struct smc_param*)hdcp_param)->ret;

    kfree(hdcp_param);
    hdcp_param = NULL;
    spin_unlock_irqrestore(&tee_lock, flags);
    
    return ret;
}

EXPORT_SYMBOL(TZ_HDCP2_GetCertInfo);

int TZ_HDCP2_GetKsXorLc128(unsigned char *pKsXorLc128, unsigned int len)
{
    unsigned int ret = 0;
    unsigned long flags; 

    //struct smc_param *tee_param = NULL;

    spin_lock_irqsave(&tee_lock, flags);    

    hdcp_param = (struct smc_param*)kmalloc(sizeof(struct smc_param)+ len*sizeof(unsigned char), GFP_KERNEL);
    memset((void *)hdcp_param, 0, sizeof(struct smc_param)+ len*sizeof(unsigned char));   
    
    hdcp_param->cmd_id = TEE_SMC_CALL_HDCP2_GetKsXorLc128;
    hdcp_param->ret = 0xFF;

    memcpy(hdcp_param->data, pKsXorLc128, len);
 
    //memcpy((void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    //tee_smc_call(TZ_SHARED_DATA_RESERVED_MEM_PA);
    //memcpy((void*)hdcp_param, (void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    
    memcpy((void*)(tz_share_rsv_mem.virt_addr), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    tee_smc_call(tz_share_rsv_mem.base);
    memcpy((void*)hdcp_param, (void*)(tz_share_rsv_mem.virt_addr), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    memcpy((void*)pKsXorLc128, (void*)hdcp_param->data, len*sizeof(unsigned char));
    ret = ((struct smc_param*)hdcp_param)->ret;
    
    kfree(hdcp_param);
    hdcp_param = NULL;
    spin_unlock_irqrestore(&tee_lock, flags);
    
    return ret;
}

EXPORT_SYMBOL(TZ_HDCP2_GetKsXorLc128);

int TZ_HDCP2_DecryptRSAESOAEP (unsigned char *pEkpub_km, unsigned int len)
{
    unsigned int ret = 0;
    unsigned long flags; 

    //struct smc_param *tee_param = NULL;

    spin_lock_irqsave(&tee_lock, flags);    

    hdcp_param = (struct smc_param*)kmalloc(sizeof(struct smc_param)+ len*sizeof(unsigned char), GFP_KERNEL);
    memset((void *)hdcp_param, 0, sizeof(struct smc_param)+ len*sizeof(unsigned char));   
    
    hdcp_param->cmd_id = TEE_SMC_CALL_HDCP2_DecryptRSAESOAEP;
    hdcp_param->ret = 0xFF;

    memcpy(hdcp_param->data, pEkpub_km, len);
    //memcpy((void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    //tee_smc_call(TZ_SHARED_DATA_RESERVED_MEM_PA);
    //memcpy((void*)hdcp_param, (void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    
    memcpy((void*)(tz_share_rsv_mem.virt_addr), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    tee_smc_call(tz_share_rsv_mem.base);
    memcpy((void*)hdcp_param, (void*)(tz_share_rsv_mem.virt_addr), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    ret = ((struct smc_param*)hdcp_param)->ret;
    kfree(hdcp_param);
    hdcp_param = NULL;
    spin_unlock_irqrestore(&tee_lock, flags);
    
    return ret;
}

EXPORT_SYMBOL(TZ_HDCP2_DecryptRSAESOAEP);

int TZ_HDCP2_2_KdKeyDerivation (unsigned char *pRtx, unsigned int Rtx_len, unsigned char *pRrx, unsigned int Rrx_len)
{
    unsigned int ret = 0;
    unsigned long flags; 
    unsigned char * data;

    //struct smc_param *tee_param = NULL;

    spin_lock_irqsave(&tee_lock, flags);    

    hdcp_param = (struct smc_param*)kmalloc(sizeof(struct smc_param)+ (Rtx_len+Rrx_len)*sizeof(unsigned char), GFP_KERNEL);
    memset((void *)hdcp_param, 0, sizeof(struct smc_param)+ (Rtx_len+Rrx_len)*sizeof(unsigned char));  

    data = (unsigned char*)kmalloc((Rtx_len+Rrx_len)*sizeof(unsigned char), GFP_KERNEL);
    memset((void *)data, 0, (Rtx_len+Rrx_len)*sizeof(unsigned char));
    memcpy(data, pRtx, Rtx_len);
    memcpy(data+Rtx_len, pRrx, Rrx_len);
    
    hdcp_param->cmd_id = TEE_SMC_CALL_HDCP2_2_KdKeyDerivation;
    hdcp_param->ret = 0xFF;

    memcpy(hdcp_param->data, data, (Rtx_len+Rrx_len));
 
    
    //memcpy((void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), (void*)hdcp_param, sizeof(struct smc_param)+ (Rtx_len+Rrx_len)*sizeof(unsigned char));
        
    //tee_smc_call(TZ_SHARED_DATA_RESERVED_MEM_PA);
    //memcpy((void*)hdcp_param, (void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), sizeof(struct smc_param)+ (Rtx_len+Rrx_len)*sizeof(unsigned char));
    
    memcpy((void*)(tz_share_rsv_mem.virt_addr), (void*)hdcp_param, sizeof(struct smc_param)+ (Rtx_len+Rrx_len)*sizeof(unsigned char));
        
    tee_smc_call(tz_share_rsv_mem.base);
    memcpy((void*)hdcp_param, (void*)(tz_share_rsv_mem.virt_addr), sizeof(struct smc_param)+ (Rtx_len+Rrx_len)*sizeof(unsigned char));
    ret = ((struct smc_param*)hdcp_param)->ret;
    kfree(hdcp_param);
    hdcp_param = NULL;
    spin_unlock_irqrestore(&tee_lock, flags);
    
    return ret;
}

EXPORT_SYMBOL(TZ_HDCP2_2_KdKeyDerivation);

int TZ_HDCP2_KdKeyDerivation (unsigned char *pRtx, unsigned int len)
{
    unsigned int ret = 0;
    unsigned long flags; 

    //struct smc_param *tee_param = NULL;

    spin_lock_irqsave(&tee_lock, flags);    

    hdcp_param = (struct smc_param*)kmalloc(sizeof(struct smc_param)+ len*sizeof(unsigned char), GFP_KERNEL); ;
    memset((void *)hdcp_param, 0, sizeof(struct smc_param)+ len*sizeof(unsigned char));   
    
    hdcp_param->cmd_id = TEE_SMC_CALL_HDCP2_KdKeyDerivation;
    hdcp_param->ret = 0xFF;

    memcpy(hdcp_param->data, pRtx, len);
    //printk("\nTEE HDCP2_KdKeyDerivation %s\n", hdcp_param->data);
    //memcpy((void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    //tee_smc_call(TZ_SHARED_DATA_RESERVED_MEM_PA);
    //memcpy((void*)hdcp_param, (void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    
    memcpy((void*)(tz_share_rsv_mem.virt_addr), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    tee_smc_call(tz_share_rsv_mem.base);
    memcpy((void*)hdcp_param, (void*)(tz_share_rsv_mem.virt_addr), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    ret = ((struct smc_param*)hdcp_param)->ret;
    kfree(hdcp_param);
    hdcp_param = NULL;
    spin_unlock_irqrestore(&tee_lock, flags);
    
    return ret;
}

EXPORT_SYMBOL(TZ_HDCP2_KdKeyDerivation);

int TZ_HDCP2_ComputeHprime (unsigned char* data, unsigned int len)
{
    unsigned int ret = 0;
    unsigned long flags; 

    //struct smc_param *tee_param = NULL;

    spin_lock_irqsave(&tee_lock, flags);    

    hdcp_param = (struct smc_param*)kmalloc(sizeof(struct smc_param)+ len*sizeof(unsigned char), GFP_KERNEL);
    memset((void *)hdcp_param, 0, sizeof(struct smc_param)+ len*sizeof(unsigned char));   
    
    hdcp_param->cmd_id = TEE_SMC_CALL_HDCP2_ComputeHprime;
    hdcp_param->ret = 0xFF;

    memcpy(hdcp_param->data, data, len);
    //printk("\nTEE HDCP2_ComputeHprime %s\n", hdcp_param->data);
    //memcpy((void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    //tee_smc_call(TZ_SHARED_DATA_RESERVED_MEM_PA);
    //memcpy((void*)hdcp_param, (void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    
    memcpy((void*)(tz_share_rsv_mem.virt_addr), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    tee_smc_call(tz_share_rsv_mem.base);
    memcpy((void*)hdcp_param, (void*)(tz_share_rsv_mem.virt_addr), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    memcpy((void*)data, (void*)hdcp_param->data, len*sizeof(unsigned char));
    ret = ((struct smc_param*)hdcp_param)->ret;
    kfree(hdcp_param);
    hdcp_param = NULL;
    spin_unlock_irqrestore(&tee_lock, flags);
    
    return ret;
}
EXPORT_SYMBOL(TZ_HDCP2_ComputeHprime);

int TZ_HDCP2_2_ComputeHprime (unsigned char* data, unsigned int len)
{
    unsigned int ret = 0;
    unsigned long flags; 

    //struct smc_param *tee_param = NULL;

    spin_lock_irqsave(&tee_lock, flags);    

    hdcp_param = (struct smc_param*)kmalloc(sizeof(struct smc_param)+ len*sizeof(unsigned char), GFP_KERNEL);
    memset((void *)hdcp_param, 0, sizeof(struct smc_param)+ len*sizeof(unsigned char));   
    
    hdcp_param->cmd_id = TEE_SMC_CALL_HDCP2_2_ComputeHprime;
    hdcp_param->ret = 0xFF;

    memcpy(hdcp_param->data, data, len);
    //printk("\nTEE HDCP2_2_ComputeHprime %s\n", hdcp_param->data);
    //memcpy((void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    //tee_smc_call(TZ_SHARED_DATA_RESERVED_MEM_PA);
    //memcpy((void*)hdcp_param, (void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    
    memcpy((void*)(tz_share_rsv_mem.virt_addr), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    tee_smc_call(tz_share_rsv_mem.base);
    memcpy((void*)hdcp_param, (void*)(tz_share_rsv_mem.virt_addr), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    memcpy((void*)data, (void*)hdcp_param->data, len*sizeof(unsigned char));
    ret = ((struct smc_param*)hdcp_param)->ret;
    kfree(hdcp_param);
    hdcp_param = NULL;
    spin_unlock_irqrestore(&tee_lock, flags);
    
    return ret;
}
EXPORT_SYMBOL(TZ_HDCP2_2_ComputeHprime);

int TZ_HDCP2_2_ComputeLprime (unsigned char* data, unsigned int len)
{
    unsigned int ret = 0;
    unsigned long flags; 

    //struct smc_param *tee_param = NULL;

    spin_lock_irqsave(&tee_lock, flags);    

    hdcp_param = (struct smc_param*)kmalloc(sizeof(struct smc_param)+ len*sizeof(unsigned char), GFP_KERNEL);
    memset((void *)hdcp_param, 0, sizeof(struct smc_param)+ len*sizeof(unsigned char));   
    
    hdcp_param->cmd_id = TEE_SMC_CALL_HDCP2_2_ComputeLprime ;
    hdcp_param->ret = 0xFF;

    memcpy(hdcp_param->data, data, len);
    //memcpy((void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    //tee_smc_call(TZ_SHARED_DATA_RESERVED_MEM_PA);
    //memcpy((void*)hdcp_param, (void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    
    memcpy((void*)(tz_share_rsv_mem.virt_addr), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    tee_smc_call(tz_share_rsv_mem.base);
    memcpy((void*)hdcp_param, (void*)(tz_share_rsv_mem.virt_addr), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    memcpy((void*)data, (void*)hdcp_param->data, len*sizeof(unsigned char));
    ret = ((struct smc_param*)hdcp_param)->ret;
    kfree(hdcp_param);
    hdcp_param = NULL;
    spin_unlock_irqrestore(&tee_lock, flags);
    
    return ret;
}

EXPORT_SYMBOL(TZ_HDCP2_2_ComputeLprime);

int TZ_HDCP2_ComputeLprime(unsigned char* data, unsigned int len)
{
    unsigned int ret = 0;
    unsigned long flags; 

    //struct smc_param *tee_param = NULL;

    spin_lock_irqsave(&tee_lock, flags);    

    hdcp_param = (struct smc_param*)kmalloc(sizeof(struct smc_param)+ len*sizeof(unsigned char), GFP_KERNEL);
    memset((void *)hdcp_param, 0, sizeof(struct smc_param)+ len*sizeof(unsigned char));   
    
    hdcp_param->cmd_id = TEE_SMC_CALL_HDCP2_ComputeLprime;
    hdcp_param->ret = 0xFF;

    memcpy(hdcp_param->data, data, len);
    //memcpy((void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    //tee_smc_call(TZ_SHARED_DATA_RESERVED_MEM_PA);
    //memcpy((void*)hdcp_param, (void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    
    memcpy((void*)(tz_share_rsv_mem.virt_addr), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    tee_smc_call(tz_share_rsv_mem.base);
    memcpy((void*)hdcp_param, (void*)(tz_share_rsv_mem.virt_addr), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    memcpy((void*)data, (void*)hdcp_param->data, len*sizeof(unsigned char));
    ret = ((struct smc_param*)hdcp_param)->ret;
    kfree(hdcp_param);
    hdcp_param = NULL;
    spin_unlock_irqrestore(&tee_lock, flags);
    
    return ret;
}
EXPORT_SYMBOL(TZ_HDCP2_ComputeLprime);

int TZ_HDCP2_ComputeKh (void)
{
    unsigned int ret = 0;
    unsigned int len = 0; 
    unsigned long flags;    

    //struct smc_param *tee_param = NULL;

    spin_lock_irqsave(&tee_lock, flags);    

    hdcp_param = (struct smc_param*)kmalloc(sizeof(struct smc_param)+ len*sizeof(unsigned char), GFP_KERNEL); ;
    memset((void *)hdcp_param, 0, sizeof(struct smc_param)+ len*sizeof(unsigned char));   
    
    hdcp_param->cmd_id = TEE_SMC_CALL_HDCP2_ComputeKh;
    hdcp_param->ret = 0xFF;

    //memcpy((void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    //tee_smc_call(TZ_SHARED_DATA_RESERVED_MEM_PA);
    //memcpy((void*)hdcp_param, (void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    
    memcpy((void*)(tz_share_rsv_mem.virt_addr), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));

    tee_smc_call(tz_share_rsv_mem.base);
    memcpy((void*)hdcp_param, (void*)(tz_share_rsv_mem.virt_addr), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    ret = ((struct smc_param*)hdcp_param)->ret;
    kfree(hdcp_param);
    hdcp_param = NULL;
    spin_unlock_irqrestore(&tee_lock, flags);
    
    return ret;
}
EXPORT_SYMBOL(TZ_HDCP2_ComputeKh);

int TZ_HDCP2_EncryptKmUsingKh (unsigned char *pEkh_km, 
                                      unsigned int pEkh_km_len,
                                      unsigned char *pM,
                                      unsigned int pM_len)
{
    unsigned int ret = 0;
    unsigned long flags; 
    unsigned char * data;

    //struct smc_param *tee_param = NULL;

    spin_lock_irqsave(&tee_lock, flags);    

    hdcp_param = (struct smc_param*)kmalloc(sizeof(struct smc_param)+ (pEkh_km_len+pM_len)*sizeof(unsigned char), GFP_KERNEL);
    memset((void *)hdcp_param, 0, sizeof(struct smc_param)+ (pEkh_km_len+pM_len)*sizeof(unsigned char));  

    data = (unsigned char*)kmalloc((pEkh_km_len+pM_len)*sizeof(unsigned char), GFP_KERNEL);
    memset((void *)data, 0, (pEkh_km_len+pM_len)*sizeof(unsigned char));
    memcpy(data, pEkh_km, pEkh_km_len);
    memcpy(data+pEkh_km_len, pM, pM_len);
    
    hdcp_param->cmd_id = TEE_SMC_CALL_HDCP2_EncryptKmUsingKh;
    hdcp_param->ret = 0xFF;

    memcpy(hdcp_param->data, data, (pEkh_km_len+pM_len));
    //memcpy((void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), (void*)hdcp_param, sizeof(struct smc_param)+ (pEkh_km_len+pM_len)*sizeof(unsigned char));
        
    //tee_smc_call(TZ_SHARED_DATA_RESERVED_MEM_PA);
    //memcpy((void*)hdcp_param, (void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), sizeof(struct smc_param)+ (pEkh_km_len+pM_len)*sizeof(unsigned char));
    
    memcpy((void*)(tz_share_rsv_mem.virt_addr), (void*)hdcp_param, sizeof(struct smc_param)+ (pEkh_km_len+pM_len)*sizeof(unsigned char));
    //printk("TZ_HDCP2_EncryptKmUsingKh\n");
    tee_smc_call(tz_share_rsv_mem.base);
    memcpy((void*)hdcp_param, (void*)(tz_share_rsv_mem.virt_addr), sizeof(struct smc_param)+ (pEkh_km_len+pM_len)*sizeof(unsigned char));
    memcpy(pEkh_km, (unsigned char*)hdcp_param->data, pEkh_km_len);
    //printk("Kernel dumping DATA\n");
    tz_hdcp2xDumpHex((unsigned char*)hdcp_param->data, pEkh_km_len);
    //printk("Kernel dumping pEkh_km\n");
    tz_hdcp2xDumpHex(pEkh_km, pEkh_km_len);
    ret = ((struct smc_param*)hdcp_param)->ret;
    kfree(hdcp_param);
    hdcp_param = NULL;
    spin_unlock_irqrestore(&tee_lock, flags);
    
    return ret;
}
EXPORT_SYMBOL(TZ_HDCP2_EncryptKmUsingKh);

int TZ_HDCP2_DecryptKmUsingKh (unsigned char *pM, 
                                      unsigned int pM_len,
                                      unsigned char *ekh_km,
                                      unsigned int ekh_km_len)
{
    unsigned int ret = 0;
    unsigned long flags; 
    unsigned char * data;

    //struct smc_param *tee_param = NULL;

    spin_lock_irqsave(&tee_lock, flags);    

    hdcp_param = (struct smc_param*)kmalloc(sizeof(struct smc_param)+ (pM_len+ekh_km_len)*sizeof(unsigned char), GFP_KERNEL);
    memset((void *)hdcp_param, 0, sizeof(struct smc_param)+ (pM_len+ekh_km_len)*sizeof(unsigned char));  

    data = (unsigned char*)kmalloc((pM_len+ekh_km_len)*sizeof(unsigned char), GFP_KERNEL);
    memset((void *)data, 0, (pM_len+ekh_km_len)*sizeof(unsigned char));
    memcpy(data, pM, pM_len);
    memcpy(data+pM_len, ekh_km, ekh_km_len);
    
    hdcp_param->cmd_id = TEE_SMC_CALL_HDCP2_DecryptKmUsingKh;
    hdcp_param->ret = 0xFF;

    memcpy(hdcp_param->data, data, (pM_len+ekh_km_len));
    //memcpy((void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), (void*)hdcp_param, sizeof(struct smc_param)+ (pM_len+ekh_km_len)*sizeof(unsigned char));
        
    //tee_smc_call(TZ_SHARED_DATA_RESERVED_MEM_PA);
    //memcpy((void*)hdcp_param, (void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), sizeof(struct smc_param)+ (pM_len+ekh_km_len)*sizeof(unsigned char));
    
    memcpy((void*)(tz_share_rsv_mem.virt_addr), (void*)hdcp_param, sizeof(struct smc_param)+ (pM_len+ekh_km_len)*sizeof(unsigned char));
        
    tee_smc_call(tz_share_rsv_mem.base);
    memcpy((void*)hdcp_param, (void*)(tz_share_rsv_mem.virt_addr), sizeof(struct smc_param)+ (pM_len+ekh_km_len)*sizeof(unsigned char));
    ret = ((struct smc_param*)hdcp_param)->ret;
    kfree(hdcp_param);
    hdcp_param = NULL;
    spin_unlock_irqrestore(&tee_lock, flags);
    
    return ret;
}
EXPORT_SYMBOL(TZ_HDCP2_DecryptKmUsingKh);

int TZ_HDCP2_DecryptEKs(unsigned char* data, unsigned int len)
{
    unsigned int ret = 0;
    unsigned long flags; 

    //struct smc_param *tee_param = NULL;

    spin_lock_irqsave(&tee_lock, flags);    

    hdcp_param = (struct smc_param*)kmalloc(sizeof(struct smc_param)+ len*sizeof(unsigned char), GFP_KERNEL);
    memset((void *)hdcp_param, 0, sizeof(struct smc_param)+ len*sizeof(unsigned char));   
    
    hdcp_param->cmd_id = TEE_SMC_CALL_HDCP2_DecryptEKs;
    hdcp_param->ret = 0xFF;

    memcpy(hdcp_param->data, data, len);
    //memcpy((void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    //tee_smc_call(TZ_SHARED_DATA_RESERVED_MEM_PA);
    //memcpy((void*)hdcp_param, (void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    
    memcpy((void*)(tz_share_rsv_mem.virt_addr), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    tee_smc_call(tz_share_rsv_mem.base);
    memcpy((void*)hdcp_param, (void*)(tz_share_rsv_mem.virt_addr), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    ret = ((struct smc_param*)hdcp_param)->ret;
    kfree(hdcp_param);
    hdcp_param = NULL;
    spin_unlock_irqrestore(&tee_lock, flags);
    
    return ret;
}
EXPORT_SYMBOL(TZ_HDCP2_DecryptEKs);

int TZ_HDCP2_2_DecryptEKs (unsigned char* data, unsigned int len)
{
    unsigned int ret = 0;
    unsigned long flags; 

    //struct smc_param *tee_param = NULL;

    spin_lock_irqsave(&tee_lock, flags);    

    hdcp_param = (struct smc_param*)kmalloc(sizeof(struct smc_param)+ len*sizeof(unsigned char), GFP_KERNEL);
    memset((void *)hdcp_param, 0, sizeof(struct smc_param)+ len*sizeof(unsigned char));   
    
    hdcp_param->cmd_id = TEE_SMC_CALL_HDCP2_2_DecryptEKs;
    hdcp_param->ret = 0xFF;

    memcpy(hdcp_param->data, data, len);
    //memcpy((void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    //tee_smc_call(TZ_SHARED_DATA_RESERVED_MEM_PA);
    //memcpy((void*)hdcp_param, (void*)(MEMRSV_PHY_TO_VIRT(TZ_SHARED_DATA_RESERVED_MEM_PA)), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    
    memcpy((void*)(tz_share_rsv_mem.virt_addr), (void*)hdcp_param, sizeof(struct smc_param)+ len*sizeof(unsigned char));
        
    tee_smc_call(tz_share_rsv_mem.base);
    memcpy((void*)hdcp_param, (void*)(tz_share_rsv_mem.virt_addr), sizeof(struct smc_param)+ len*sizeof(unsigned char));
    ret = ((struct smc_param*)hdcp_param)->ret;
    kfree(hdcp_param);
    hdcp_param = NULL;
    spin_unlock_irqrestore(&tee_lock, flags);
    
    return ret;
}
EXPORT_SYMBOL(TZ_HDCP2_2_DecryptEKs);
