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

#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_reserved_mem.h>
#include <linux/device.h>

#include <linux/libfdt.h>



/*
* add API to get static reserve memory info for drivers
* return 0:success  other value:fail
*/
int get_static_reserved_memory(const char *uname, phys_addr_t *base, phys_addr_t *size)
{
	int offset;
	const char *p;
	int len;
	const void *fdt = initial_boot_params;
	char full_node_name[60]="/reserved-memory/";
	
	//pr_err("[Libatcbsp] fdt addr= %pa\n", fdt);
	if (NULL == fdt || NULL == uname || '\0' == *uname)
		return -ENOENT;
	
	strcat(full_node_name, uname);	

	offset = fdt_path_offset(fdt, full_node_name);
	if (offset < 0)
		return -ENOENT;
    //pr_err("[Libatcbsp] node[%s] found\r\n", full_node_name);
	p = fdt_getprop(fdt, offset, "reg", &len);

	if (!p || !len)
		return -ENOENT;
    //pr_err("[Libatcbsp] reg found, len= %d\r\n", len);
	if (8 == len)        //32bit *2
	{
		const u32 *data32 = (u32 *)p;
		*base = (phys_addr_t)fdt32_to_cpu(data32[0]);
		*size = (phys_addr_t)fdt32_to_cpu(data32[1]);
	}
#ifdef CONFIG_PHYS_ADDR_T_64BIT
	else if (16 == len) //64bit * 2
	{
		u64 *data64 = (u64 *)p;
		*base = fdt64_to_cpu(data64[0]);
		*size = fdt64_to_cpu(data64[1]);
	}
#endif
	else	
		return -ENODEV;
		
#ifdef CONFIG_PHYS_ADDR_T_64BIT
	pr_info("[Libatcbsp] [%s][%d] bsp static rsv mem node[%s]: base = 0x%llx, size = 0x%llx\n", __func__, __LINE__, uname, *base, *size);
#else
	pr_info("[Libatcbsp] [%s][%d] bsp static rsv mem node[%s]: base = 0x%x, size = 0x%x\n", __func__, __LINE__, uname, *base, *size);
#endif	

	return 0;
}
EXPORT_SYMBOL(get_static_reserved_memory);


#if 0
void test_static_rsv_mem(void)
{
	phys_addr_t base, size;
	int ret = 0;
	ret = get_static_reserved_memory("trustzone", &base, &size);
	if (0 != ret)
		pr_err("[Libatcbsp] node not exist or not a static reserved memory node\n");
		
	ret = get_static_reserved_memory("trustzoneshare", &base, &size);
	if (0 != ret)
		pr_err("[Libatcbsp] node not exist or not a static reserved memory node\n");
	
	ret = get_static_reserved_memory("dvp", &base, &size);
	if (0 != ret)
		pr_err("[Libatcbsp] node not exist or not a static reserved memory node\n");
	
	ret = get_static_reserved_memory("dsp", &base, &size);
	if (0 != ret)
		pr_err("[Libatcbsp] node not exist or not a static reserved memory node\n");
		
	ret = get_static_reserved_memory("framebuffer", &base, &size);
	if (0 != ret)
		pr_err("[Libatcbsp] node not exist or not a static reserved memory node\n");
	
	ret = get_static_reserved_memory("imageresize", &base, &size);
	if (0 != ret)
		pr_err("[Libatcbsp] node not exist or not a static reserved memory node\n");
		
	ret = get_static_reserved_memory("wch_rsv", &base, &size);
	if (0 != ret)
		pr_err("[Libatcbsp] node not exist or not a static reserved memory node\n");
		
	ret = get_static_reserved_memory("arm2", &base, &size);
	if (0 != ret)
		pr_err("[Libatcbsp] node not exist or not a static reserved memory node\n");
		
	ret = get_static_reserved_memory("xx", &base, &size);
	if (0 != ret)
		pr_err("[Libatcbsp] node not exist or not a static reserved memory node\n");
	
	ret = get_static_reserved_memory("yy", &base, &size);
	if (0 != ret)
		pr_err("[Libatcbsp] node not exist or not a static reserved memory node\n");
		
	
}
#endif

#if 0
static inline void dev_set_cma_area(struct device *dev, struct cma *cma)
{
	if (dev)
		dev->cma_area = cma;
}
#endif

static inline void dev_set_cma_area(struct device *dev, struct reserved_mem *rmem)
{
	if (dev) {
		pr_info("[Libatcbsp] [%s][%d] bsp : rmem = %pa, dev = %pa/n", __func__, __LINE__, rmem, dev);
		dev->cma_area = (struct cma *)rmem;
	}
}

static int rmem_cma_device_init(struct reserved_mem *rmem, struct device *dev)
{
	//dev_set_cma_area(dev, rmem->priv);
	dev_set_cma_area(dev, rmem);
	return 0;
}

static void rmem_cma_device_release(struct reserved_mem *rmem,
				    struct device *dev)
{
	dev_set_cma_area(dev, NULL);
}

static const struct reserved_mem_ops rmem_cma_ops = {
	.device_init	= rmem_cma_device_init,
	.device_release = rmem_cma_device_release,
};

static int __init rmem_bsp_setup(struct reserved_mem *rmem)
{
#if 0
	phys_addr_t align = PAGE_SIZE << max(MAX_ORDER - 1, pageblock_order);
	phys_addr_t mask = align - 1;
	unsigned long node = rmem->fdt_node;
	struct cma *cma;
	int err;

	if (!of_get_flat_dt_prop(node, "reusable", NULL) ||
	    of_get_flat_dt_prop(node, "no-map", NULL))
		return -EINVAL;

	if ((rmem->base & mask) || (rmem->size & mask)) {
		pr_err("[Libatcbsp] Reserved memory: incorrect alignment of CMA region\n");
		return -EINVAL;
	}

	err = cma_init_reserved_mem(rmem->base, rmem->size, 0, &cma);
	if (err) {
		pr_err("[Libatcbsp] Reserved memory: unable to setup CMA region\n");
		return err;
	}
	/* Architecture specific contiguous memory fixup. */
	dma_contiguous_early_fixup(rmem->base, rmem->size);

	if (of_get_flat_dt_prop(node, "linux,cma-default", NULL))
		dma_contiguous_set_default(cma);
#endif
	rmem->ops = &rmem_cma_ops;
	//rmem->priv = cma;

	pr_info("[Libatcbsp] [%s][%d] Reserved memory: created bsp memory at %pa, size %ld MiB\n", __func__, __LINE__,
		&rmem->base, (unsigned long)rmem->size / SZ_1M);

	return 0;
}

RESERVEDMEM_OF_DECLARE(cma, "shared-bsp", rmem_bsp_setup);
RESERVEDMEM_OF_DECLARE(fbm, "atc-framebuffer", rmem_bsp_setup);
RESERVEDMEM_OF_DECLARE(extdisfb, "atc-extdisfb", rmem_bsp_setup);
RESERVEDMEM_OF_DECLARE(vm, "atc-imageresize", rmem_bsp_setup);
RESERVEDMEM_OF_DECLARE(wch, "atc-wch", rmem_bsp_setup);
RESERVEDMEM_OF_DECLARE(mmisc, "atc-multimedia", rmem_bsp_setup);
RESERVEDMEM_OF_DECLARE(afifo, "atc-audio-fifo", rmem_bsp_setup);
RESERVEDMEM_OF_DECLARE(metazone, "atc-metazone-mem", rmem_bsp_setup);
RESERVEDMEM_OF_DECLARE(linebuf, "atc-image-linebuf", rmem_bsp_setup);//added by mtk68119

