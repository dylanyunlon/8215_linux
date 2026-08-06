
#ifndef __CACHE_OPERATION_H
#define __CACHE_OPERATION_H

/*
 * BSP_FlushDCacheRange(UINT32 u4Start, UINT32 u4End)
 *
 * Clean and Invalidate Data Cache Range
 * - u4Start : virtual start address (inclusive)
 * - u4Len : length
 */
#define BSP_CACHE_OPERATION_DEBUG	0
#define FLUSH_L1_RANGE_MAX	0x00008000
#define CLEAN_L1_RANGE_MAX	0x00008000
#define INV_L1_RANGE_MAX		0x00008000

#ifndef CONFIG_OUTER_CACHE
#define CORE_FLUSH_RANGE_MAX	0x00008000
#define CORE_CLEAN_RANGE_MAX	0x00008000
#define CORE_INV_RANGE_MAX		0x00008000
#else
#define FLUSH_L1L2_RANGE_MAX		0x00003800
#define CLEAN_L1L2_RANGE_MAX		0x00003000
#define INV_L1L2_RANGE_MAX		0x00003800
#define CORE_FLUSH_RANGE_MAX	0x00008000
#define CORE_CLEAN_RANGE_MAX	0x00008000
#define CORE_INV_RANGE_MAX		0x00008000
#endif

#define CACHE_LINE_UNIT          64  // 1 cache line = 32 byte

extern void Core_CleanDCacheRange(unsigned long start, unsigned long len); //DMA_TO_L2
extern void Core_InvDCacheRange(unsigned long start, unsigned long len); //DMA_FROM_L2
void Core_CleanOuterCacheRange(unsigned long start, unsigned long len);

enum data_direction {
	BIDIRECTIONAL = 0,
	TO_DEVICE = 1,
	FROM_DEVICE = 2
};
extern unsigned long BSP_dma_map_single(unsigned long start, unsigned long len, enum data_direction dir);
extern void BSP_dma_unmap_single(unsigned long start, unsigned long len, enum data_direction dir);
extern void BSP_dma_map_vaddr(unsigned long start, unsigned long len, enum data_direction dir);
extern void BSP_dma_unmap_vaddr(unsigned long start, unsigned long len, enum data_direction dir);
extern unsigned long bsp_vaddr_to_phys(unsigned long x);

#define VADDR_TO_PHYS(x) bsp_vaddr_to_phys(x)

#endif /* __CACHE_OPERATION_H */


