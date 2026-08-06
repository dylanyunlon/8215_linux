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
#include "tee_drv.h"
#include "efuse/efuse_tee_drv.h"
#include <asm/io.h>
#include <generated/atc_project.h>

DEFINE_SPINLOCK(tee_lock);
struct resvd_mem_info tz_share_rsv_mem;
extern int get_static_reserved_memory(const char *uname, phys_addr_t *base, phys_addr_t *size);

static int __init teedrv_init(void)  
{ 
	int ret = 0;

#ifndef CONFIG_ATC_PLATFORM_ac823x
	memset((void *)(&tz_share_rsv_mem), 0, sizeof(tz_share_rsv_mem));
	ret = get_static_reserved_memory("trustzoneshare", &(tz_share_rsv_mem.base), &(tz_share_rsv_mem.size));  
	if (ret)
	{
		pr_err("[TEE]can not find [trustzoneshare] node reserve memory!\r\n");
		return -1;
	}

	tz_share_rsv_mem.virt_addr = ioremap(tz_share_rsv_mem.base, tz_share_rsv_mem.size);
	if (NULL == tz_share_rsv_mem.virt_addr)
	{
		pr_err("[TEE]can not ioremap trustzone reserve mem\r\n");
		return -2;
	}
#endif

    return 0;  
}  
  
static void __exit teedrv_exit(void)  
{  
   	if (tz_share_rsv_mem.virt_addr)
   	{
		iounmap(tz_share_rsv_mem.virt_addr);
   	}
   	return; 
}  
  
module_init(teedrv_init);  
module_exit(teedrv_exit);  
MODULE_LICENSE("GPL");  
MODULE_AUTHOR("ATC");  
