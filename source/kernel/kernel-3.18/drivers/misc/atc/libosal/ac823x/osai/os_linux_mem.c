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
#include <linux/slab.h>
#include "x_os.h"
#include "sys_config.h"
#include "drv_config.h"
#include <linux/types.h>

void* x_mem_alloc (size_t  u4Size)
{
#ifdef CONFIG_SYS_MEM_WASTE_WARNING
    if (u4Size >= PAGE_SIZE)
    {
        if((u4Size * 4) < (((1 << get_order(u4Size)) << PAGE_SHIFT) * 3))
            printk("[%s] size = %d, usage less than 3/4, caller address = 0x%08X\r\n", __FUNCTION__, u4Size, (u32)__builtin_return_address(0));
        if((u4Size + (256*1024)) < ((1 << get_order(u4Size)) << PAGE_SHIFT))
            printk("[%s] size = %d, over 256K bytes is unused, caller address = 0x%08X\r\n", __FUNCTION__, u4Size, (u32)__builtin_return_address(0));
    }
#endif

    return kzalloc(u4Size, GFP_KERNEL);
}

void* x_mem_alloc_ret_phy_addr (size_t  z_size, __u32 *pu4PhyAddr )
{

    void *va = kmalloc(z_size, GFP_KERNEL);

	if (va == NULL) {
		*pu4PhyAddr = 0;
		return NULL;
	}
    *pu4PhyAddr = __pa(va);

     return (va);
}

void  x_mem_free_ret_phy_addr (void*  pv_mem_block )
{
	if (pv_mem_block != NULL) {
	  kfree(pv_mem_block);
	}
}

void* x_mem_calloc (__u32  ui4_num_element,
                    __u32  z_size_element)
{
    return kcalloc(ui4_num_element, z_size_element, GFP_KERNEL);
}


void* x_mem_realloc (const void*  pv_mem_block,
                     __u32 z_new_size)
{
    return krealloc(pv_mem_block, z_new_size, GFP_KERNEL);
}


void x_mem_free (void*  pv_mem_block)
{
    kfree(pv_mem_block);
}


void * x_mem_ch2_alloc(size_t u4Size)
{
    return kmalloc(u4Size, GFP_KERNEL);
}


void* x_mem_ch2_calloc (__u32  ui4_num_element, __u32  z_size_element)
{
    return kcalloc((size_t)ui4_num_element, (size_t)z_size_element, GFP_KERNEL);
}


void* x_mem_ch2_realloc (const void*  pv_mem_block, __u32 z_new_size)
{
    return krealloc(pv_mem_block, z_new_size, GFP_KERNEL);
}


__s32 x_mem_part_create (__u32*    ph_part_hdl,
                         const char*  ps_name,
                         void*        pv_addr,
                         __u32       z_size,
                         __u32       z_alloc_size)
{
    return OSR_OK;
}


__s32 x_mem_part_delete (__u32 h_part_hdl)
{
    return OSR_OK;
}


__s32 x_mem_part_attach (__u32*    ph_part_hdl,
                         const char*  ps_name)
{
    return OSR_OK;
}


void* x_mem_part_alloc (__u32  h_part_hdl,
                        __u32    z_size)
{
    return kmalloc(z_size, GFP_KERNEL);
}


void* x_mem_part_calloc (__u32  h_part_hdl,
                         __u32    ui4_num_element,
                         __u32    z_size_element)
{
    return kcalloc((size_t)ui4_num_element, (size_t)z_size_element, GFP_KERNEL);
}


void* x_mem_part_realloc (__u32     h_part_hdl,
                          const void*        pv_mem_block,
                          __u32       z_new_size)
{
    return krealloc(pv_mem_block, z_new_size, GFP_KERNEL);
}

#if !CONFIG_SYS_MEM_PHASE3
void x_mem_fill_address(void *pv_mem, void *address)
{
}
#endif

void* x_mem_aligned_alloc(size_t z_size, __u32 u4Align)
{
	return (void *)(((__u32)kmalloc((size_t)(z_size + u4Align), GFP_KERNEL) + u4Align - (__u32)1) &
			~(u4Align - (__u32)1));
}

EXPORT_SYMBOL(x_mem_alloc);
EXPORT_SYMBOL(x_mem_alloc_ret_phy_addr);
EXPORT_SYMBOL( x_mem_free_ret_phy_addr);
EXPORT_SYMBOL(x_mem_calloc);
EXPORT_SYMBOL(x_mem_realloc);
EXPORT_SYMBOL(x_mem_free);
EXPORT_SYMBOL(x_mem_ch2_alloc);
EXPORT_SYMBOL(x_mem_ch2_calloc);
EXPORT_SYMBOL(x_mem_ch2_realloc);
EXPORT_SYMBOL(x_mem_part_create);
EXPORT_SYMBOL(x_mem_part_delete);
EXPORT_SYMBOL(x_mem_part_attach);
EXPORT_SYMBOL(x_mem_part_alloc);
EXPORT_SYMBOL(x_mem_part_calloc);
EXPORT_SYMBOL(x_mem_part_realloc);
#if !CONFIG_SYS_MEM_PHASE3
EXPORT_SYMBOL(x_mem_fill_address);
#endif
EXPORT_SYMBOL(x_mem_aligned_alloc);


