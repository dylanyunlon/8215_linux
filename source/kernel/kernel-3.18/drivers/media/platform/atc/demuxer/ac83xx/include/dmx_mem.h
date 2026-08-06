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


/*!
 * @file dmx_mem.h
 *
 * @par Project
 *
 *
 * @par Description
 *    Demuxer mem management Structure, Macro, interface declarations
 *
 * @par Author_Name
 *    Shuhui Zhang
 *
 */

#ifndef DMX_INTERNAL_MEM_H
#define DMX_INTERNAL_MEM_H

#include "x_os.h"
#include "x_typedef.h"
#include "chip_ver.h"
#ifdef __linux__
#include <linux/mm.h>
#include <linux/errno.h>
#include <media/atc/dmx_define.h>
#include <media/atc/drv_osd_if.h>
#include <media/atc/memdbg_c.h>
#include <media/atc/ose_mem.h>
#else
#include "drv_osd_if.h"
#include "memdbg_c.h"
#include "dmx_define.h"
#include "OSE_mem.h"
#endif	/* __linux__ */

#include "dmx_cli.h"

#ifdef __cplusplus
extern "C" {

#endif

/* ////////////////////////////////////////////////////////////////////////////// */
/* /////////////////   Memory Related Macros and Enumerations             /////// */
/* ////////////////////////////////////////////////////////////////////////////// */

#define DMX_HWMEM_ALL_USE_RSVMEM    1

#define DMX_MIN_MEMORY_ALIGNMENT                ((u32)32)

#ifndef NONCACHE
#define NONCACHE(addr)         (addr)
#endif

#ifndef CACHE
#define CACHE(addr)            (addr)
#endif

#define DMX_MEM_PRINT_LINE_FUNCTION	DMX_CHECK_MEM_VALIBILITY

#if DMX_MEM_PRINT_LINE_FUNCTION

#define DMX_VMALLOC_FROM_RESEVED_MEM   0

#define DMX_MEM_GUARD_PATTEN_CHECK     1

void dmx_memsetex(void *pvDst, u8 u1Value, u32 u4Size, const char *szFunc,
		s32 i4Line);
void dmx_memcpyex(void *pvDst, void *pvSrc, u32 u4Size, const char *szFunc,
		s32 i4Line);
#define dmx_memset(dst, value, size) dmx_memsetex((dst), (value), (size), __func__, __LINE__)
#define dmx_memcpy(dst, src, size)   dmx_memcpyex((dst), (src), (size), __func__, __LINE__)

#else	/* DMX_MEM_PRINT_LINE_FUNCTION */

#define DMX_VMALLOC_FROM_RESEVED_MEM   0

#define DMX_MEM_GUARD_PATTEN_CHECK     0
#define dmx_memset(dst, value, size) mm_memset((dst), (value), (size))
#define dmx_memcpy(dst, src, size)   mm_memcpy((dst), (src), (size))
#endif	/* DMX_MEM_PRINT_LINE_FUNCTION */

#if DMX_MEM_GUARD_PATTEN_CHECK
#define DMX_MEM_GUARD_PATTEN_LEN       ((u32)8)
#endif

typedef enum {
	SPT_MEM_TYPE_UNKNOWN,
	SPT_MEM_TYPE_OSEMALLOC,
	SPT_MEM_TYPE_OSEPHY,
	SPT_MEM_TYPE_EXT,
	MAX_OF_SPT_MEM_TYPE
} E_SPT_MEM_TYPE_T;

extern uintptr_t DMX_MEM_Align(uintptr_t ptrAddr, u32 u4Alignment);

#ifdef __linux__

#if DMX_MEM_PRINT_LINE_FUNCTION

#if DMX_MEM_GUARD_PATTEN_CHECK

#define dmx_alloc_np_mem_ex(pvMem, u4Size, szFunc, i4Line) do { \
	if (0 < (u4Size)) {\
		u32 u4AllocSize = DMX_MEM_Align(u4Size,\
			DMX_MIN_MEMORY_ALIGNMENT) + DMX_MIN_MEMORY_ALIGNMENT +\
			DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8) * 2;\
		pvMem = (__typeof__(pvMem)) MM_ALLOC(u4AllocSize);\
	} \
	else {\
		pvMem = (__typeof__(pvMem))NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail for u4Size == 0 %s, line %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
	} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT, \
		TEXT("[MEM] %s fail %s, line %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
		DMX_Dump_Mem();\
		DMX_ASSERT(FALSE);\
	} \
	else {\
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
		TEXT("[MEM] %s -- %s, line %d, alloc Size: %d, pvMem: 0x%08x\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line), (u4Size), (uintptr_t)(pvMem));\
		Add_VMemMap((void *)(pvMem), (u8)0x00, SPT_MEM_TYPE_OSEMALLOC, u4Size,\
			DMX_MIN_MEMORY_ALIGNMENT, (szFunc), (i4Line));\
		pvMem = (__typeof__(pvMem)) ((uintptr_t)(pvMem) + DMX_MIN_MEMORY_ALIGNMENT);\
	} \
} while (0)

/* The following function is used to alloc HW continous memory, the memory is 32-aligned */

#define dmx_alloc_cp_mem_ex(pvMem, u4Size, szFunc, i4Line)	do {\
	uintptr_t ptrPhysAddr;\
	if (0 < (u4Size)) { \
		u32 u4AllocSize =\
			DMX_MEM_Align((u4Size),\
			DMX_MIN_MEMORY_ALIGNMENT) + DMX_MIN_MEMORY_ALIGNMENT +\
			DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8) * 2;\
		pvMem =\
			(__typeof__(pvMem)) OSE_MemAllocCustom(OSE_DEMUXER, u4AllocSize,\
			DMX_MIN_MEMORY_ALIGNMENT, &ptrPhysAddr);\
	} \
	else {\
		pvMem = (__typeof__(pvMem)) NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail for u4Size == 0 %s, line %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
	} \
	if ((void *)NULL == (void *)(pvMem)) { \
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail %s, %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
		DmxCliDumpMemUsage();\
		DMX_ASSERT(FALSE);\
	} \
	else {\
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s -- %s, line %d, alloc Size: %d, pvMem: 0x%08x\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line), (u4Size), (uintptr_t)(pvMem));\
		Add_MemMap((void *)(pvMem), (void *)ptrPhysAddr, SPT_MEM_TYPE_OSEPHY,\
			u4Size, DMX_MIN_MEMORY_ALIGNMENT, (szFunc), (i4Line));\
		pvMem = (__typeof__(pvMem)) ((uintptr_t)(pvMem) + DMX_MIN_MEMORY_ALIGNMENT);\
		ptrPhysAddr += DMX_MIN_MEMORY_ALIGNMENT;\
	} \
} while (0)

#define dmx_alloc_aligned_cp_mem_ex(pvMem, u4Size, u4Align, szFunc, i4Line)	do {\
	uintptr_t ptrPhysAddr;\
	if (0 < (u4Size)) {\
		u32 u4AllocSize =\
			DMX_MEM_Align(u4Size, u4Align) + (u4Align) +\
			DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8) * 2;\
		pvMem =\
			(__typeof__(pvMem)) OSE_MemAllocCustom(OSE_DEMUXER, u4AllocSize,\
			u4Align, &ptrPhysAddr);\
	} \
	else{\
		pvMem = (__typeof__(pvMem)) NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail for u4Size == 0 %s, line %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
	} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail %s, %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
		DmxCliDumpMemUsage();\
		DMX_ASSERT(FALSE);\
	} \
	else{\
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT\
			("[MEM] %s -- %s, line %d, alloc Size: %d, pvMem: 0x%08x\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line), (u4Size), (uintptr_t)(pvMem));\
		Add_MemMap((void *)(pvMem), (void *)ptrPhysAddr, SPT_MEM_TYPE_OSEPHY,\
			(u4Size), (u4Align), (szFunc), (i4Line));\
		pvMem = (__typeof__(pvMem)) ((uintptr_t)(pvMem) + (u4Align));\
		ptrPhysAddr += u4Align;\
	} \
} while (0)

#define dmx_alloc_aligned_cpex_mem_ex(pvMem, u4Size, u4Align, szFunc, i4Line)	do {\
	uintptr_t ptrPhysAddr;\
	if (0 < (u4Size)) {\
		u32 u4AllocSize =\
			DMX_MEM_Align(((u4Size) + (u4Align)),\
			u4Align) + (u4Align) + DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8) * 2;\
			pvMem =\
				(__typeof__(pvMem)) OSE_MemAllocCustom(OSE_DEMUXER, u4AllocSize,\
				u4Align, &ptrPhysAddr);\
	} \
	else {\
		pvMem = (__typeof__(pvMem)) NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail for u4Size == 0 %s, line %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
	} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail %s, %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
		DmxCliDumpMemUsage();\
		DMX_ASSERT(FALSE);\
	} \
	else {\
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s -- %s, line %d, alloc Size: %d, pvMem: 0x%08x\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line), (u4Size), (uintptr_t)(pvMem));\
		Add_MemMap((void *)(pvMem), (void *)ptrPhysAddr, SPT_MEM_TYPE_OSEPHY,\
			((u4Size) + (u4Align)), u4Align, (szFunc), (i4Line));\
		pvMem = (__typeof__(pvMem)) ((uintptr_t)(pvMem) + (u4Align));\
		ptrPhysAddr += u4Align;\
	} \
} while (0)

#else				/* DMX_MEM_GUARD_PATTEN_CHECK */

#define dmx_alloc_np_mem_ex(pvMem, u4Size, szFunc, i4Line)	do {\
	if (0 < (u4Size)) {\
		pvMem = (__typeof__(pvMem)) MM_ALLOC(u4Size);\
	} \
	else {\
		pvMem = (__typeof__(pvMem)) NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail for u4Size == 0 %s, line %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
	} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail %s, %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
		DMX_Dump_Mem();\
		DMX_ASSERT(FALSE);\
	} \
	else {\
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s -- %s, %d, alloc Size: %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line), (u4Size));\
	} \
} while (0)

#define dmx_alloc_cp_mem_ex(pvMem, u4Size, szFunc, i4Line)	do {\
	uintptr_t ptrPhysAddr;\
	if (0 < (u4Size)) {\
		pvMem =\
		(__typeof__(pvMem)) OSE_MemAllocCustom(OSE_DEMUXER, (u4Size),\
			DMX_MIN_MEMORY_ALIGNMENT, &ptrPhysAddr);\
	} \
	else {\
		pvMem = (__typeof__(pvMem)) NULL;\
	DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
		TEXT("[MEM] %s fail for u4Size == 0 %s, line %d\r\n"),\
		DMX_FUNC_NAME, (szFunc), (i4Line));\
	} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail %s, %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
		DmxCliDumpMemUsage();\
		DMX_ASSERT(FALSE);\
	} \
	else {\
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s -- %s, %d, alloc Size: %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line), (u4Size));\
		Add_MemMap((void *)(pvMem), (void *)ptrPhysAddr, SPT_MEM_TYPE_OSEPHY,\
			u4Size, DMX_MIN_MEMORY_ALIGNMENT, (szFunc), (i4Line));\
	} \
} while (0)

#define dmx_alloc_aligned_cp_mem_ex(pvMem, u4Size, u4Align, szFunc, i4Line)	do {\
	uintptr_t ptrPhysAddr;\
	if (0 < (u4Size)) {\
		pvMem =\
			(__typeof__(pvMem)) OSE_MemAllocCustom(OSE_DEMUXER, (u4Size),\
			(u4Align), &ptrPhysAddr);\
	} \
	else {\
		pvMem = (__typeof__(pvMem)) NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail for u4Size == 0 %s, line %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
	} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail %s, %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
		DmxCliDumpMemUsage();\
		DMX_ASSERT(FALSE);\
	} \
	else {\
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s -- %s, %d, alloc Size: %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line), (u4Size));\
		Add_MemMap((void *)(pvMem), (void *)ptrPhysAddr, SPT_MEM_TYPE_OSEPHY,\
			(u4Size), (u4Align), (szFunc), (i4Line));\
	} \
} while (0)

#define dmx_alloc_aligned_cpex_mem_ex(pvMem, u4Size, u4Align, szFunc, i4Line)	do {\
	u32 ptrPhysAddr;\
	if (0 < (u4Size)) {\
		pvMem =\
			(__typeof__(pvMem)) OSE_MemAllocCustom(OSE_DEMUXER, (u4Size) + (u4Align),\
			(u4Align), &ptrPhysAddr);\
	} \
	else {\
		pvMem = (__typeof__(pvMem)) NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			      TEXT("[MEM] %s fail for u4Size == 0 %s, line %d\r\n"),\
			      DMX_FUNC_NAME, (szFunc), (i4Line));\
	} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail %s, %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
		DmxCliDumpMemUsage();\
		DMX_ASSERT(FALSE);\
	} \
	else {\
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s -- %s, %d, alloc Size: %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line), ((u4Size) + (u4Align)));\
		Add_MemMap((void *)(pvMem), (void *)ptrPhysAddr, SPT_MEM_TYPE_OSEPHY,\
			((u4Size) + (u4Align)), (u4Align), (szFunc), (i4Line));\
	} \
} while (0)

#endif	/* DMX_MEM_GUARD_PATTEN_CHECK */

/* #define dmx_mem_alloc(pvMem, size)	dmx_alloc_np_mem_ex(pvMem, (size), __FUNCTION__, __LINE__) */
#define dmx_alloc_cp_mem(pvMem, size)\
			dmx_alloc_cp_mem_ex((pvMem), (size), __func__, __LINE__)
#define dmx_alloc_aligned_cp_mem(pvMem, size, align)\
			dmx_alloc_aligned_cp_mem_ex((pvMem), (size), (align), __func__, __LINE__)
#define dmx_alloc_aligned_cpex_mem(pvMem, size, align)\
			dmx_alloc_aligned_cpex_mem_ex((pvMem), (size), (align), __func__, __LINE__)

#if DMX_VMALLOC_FROM_RESEVED_MEM
#define dmx_mem_alloc(pvMem, size)\
			dmx_alloc_cp_mem_ex(pvMem, (size), __func__, __LINE__)
#define dmx_free_np_mem(pvMem)	do {\
	if ((void *)NULL != (void *)(pvMem)) {\
		Del_MemMap((void *)(pvMem));\
	} \
} while (0)
#else
#define dmx_mem_alloc(pvMem, size)\
			dmx_alloc_np_mem_ex(pvMem, (size), __func__, __LINE__)
#define dmx_free_np_mem(pvMem)	do {\
	if ((void *)NULL != (void *)(pvMem)) {\
		Del_VMemMap((void *)(pvMem));\
	} \
} while (0)
#endif	/* #if DMX_VMALLOC_FROM_RESEVED_MEM */

#define  dmx_free_cp_mem(pvMem)	do {\
	if ((void *)NULL != (void *)(pvMem)) {\
		Del_MemMap((void *)(pvMem));\
	} \
} while (0)

#define  dmx_free_aligned_cp_mem(pvMem)	do {\
	if ((void *)NULL != (void *)(pvMem)) {\
		Del_MemMap((void *)(pvMem));\
	} \
} while (0)

#define dmx_free_aligned_cpex_mem(pvMem)	do {\
	if ((void *)NULL != (void *)(pvMem)) {\
		Del_MemMap((void *)(pvMem));\
	} \
} while (0)
#else	/* DMX_MEM_PRINT_LINE_FUNCTION */

/* ////////////////////////////////////////////////////////////////////////////// */
/* ///////////////////////Memory alloc and free////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////////// */
#define dmx_alloc_np_mem(pvMem, u4Size)	do {\
	if (0 < (u4Size)) {\
		pvMem = (__typeof__(pvMem)) MM_ALLOC(u4Size);\
	} \
	else {\
		pvMem = (__typeof__(pvMem)) NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s line %d fail for u4Size == 0\r\n"),\
			DMX_FUNC_NAME, DMX_LINE_NO);\
	} \
} while (0)

#define dmx_alloc_cp_mem(pvMem, u4Size)	do {\
	uintptr_t ptrPhysAddr = 0;\
	if (0 < (u4Size)) {\
		pvMem =\
			(__typeof__(pvMem)) OSE_MemAllocCustom(OSE_DEMUXER, (u4Size)\
				DMX_MIN_MEMORY_ALIGNMENT, &ptrPhysAddr);\
	} \
	else {\
		pvMem = (__typeof__(pvMem)) NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s line %d fail for u4Size == 0\r\n"),\
			DMX_FUNC_NAME, DMX_LINE_NO);\
	} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DMX_ASSERT(FALSE);\
	} \
	else {\
		Add_MemMap((void *)(pvMem), (void *)ptrPhysAddr, SPT_MEM_TYPE_OSEPHY,\
			(u4Size));\
	} \
} while (0)

#define dmx_alloc_aligned_cp_mem(pvMem, u4Size, u4Align)	do {\
	uintptr_t ptrPhysAddr = 0;\
	if (0 < (u4Size)) {\
		pvMem =\
			(__typeof__(pvMem)) OSE_MemAllocCustom(OSE_DEMUXER, u4Size,\
			(u4Align), &ptrPhysAddr);\
	} \
	else {\
		pvMem = (__typeof__(pvMem)) NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s line %d fail for u4Size == 0\r\n"),\
			DMX_FUNC_NAME, DMX_LINE_NO);\
	} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DMX_ASSERT(FALSE);\
	} \
	else {\
		Add_MemMap((void *)(pvMem), (void *)ptrPhysAddr, SPT_MEM_TYPE_OSEPHY,\
			u4Size);\
	} \
} while (0)

#define dmx_alloc_aligned_cpex_mem(pvMem, u4Size, u4Align)	do {\
	uintptr_t ptrPhysAddr;\
	if (0 < (u4Size)) {\
		pvMem =\
			(__typeof__(pvMem)) OSE_MemAllocCustom(OSE_DEMUXER, (u4Size) + (u4Align),\
				(u4Align), &ptrPhysAddr);\
	} \
	else {\
		pvMem = (__typeof__(pvMem)) NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s line %d fail for u4Size == 0\r\n"),\
			DMX_FUNC_NAME, DMX_LINE_NO);\
	} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DMX_Dump_Mem();\
		DMX_ASSERT(FALSE);\
	} \
	else {\
		Add_MemMap((void *)(pvMem), (void *)ptrPhysAddr, SPT_MEM_TYPE_OSEPHY,\
			((u4Size) + (u4Align)));\
	} \
} while (0)

#define dmx_mem_alloc(pvMem, size)      dmx_alloc_np_mem((pvMem), (size))

#define  dmx_free_np_mem(pvMem)	do {\
	if ((void *)NULL != (void *)(pvMem)) {\
		MM_FREE((void *)(pvMem));\
	} \
} while (0)

#define  dmx_free_cp_mem(pvMem)	do {\
	if ((void *)NULL != (void *)(pvMem)) {\
		Del_MemMap((void *)(pvMem));\
		OSE_MemFreeCustom(OSE_DEMUXER, (void *)(pvMem));\
	} \
} while (0)

#define  dmx_free_aligned_cp_mem(pvMem)	do {\
	if ((void *)NULL != (void *)(pvMem)) {\
		Del_MemMap((void *)(pvMem));\
		OSE_MemFreeCustom(OSE_DEMUXER, (void *)(pvMem));\
	} \
} while (0)

#define dmx_free_aligned_cpex_mem(pvMem)	do {\
	if ((void *)NULL != (void *)(pvMem)) {\
		Del_MemMap((void *)(pvMem));\
		OSE_MemFreeCustom(OSE_DEMUXER, (void *)(pvMem));\
	} \
} while (0)

#endif				/* DMX_MEM_PRINT_LINE_FUNCTION */

/*!
 * @brief Allocate heap memory
 *
 * This function is used to allocate heap memory, it equals to MM_ALLOC
 *
 * @retval if success, return E_DMX_OK.
 *         otherwise, return the corresponding errcode
 */
#define DMX_NewMemory(u4Sz, pvMem)	do {\
	if ((void *)NULL == (void *)(pvMem)) {\
		dmx_mem_alloc((pvMem), (u32)(u4Sz));\
		if ((void *)(pvMem) != (void *)NULL) {\
			dmx_memset((void *)(pvMem), (u8)0x00, (u32)(u4Sz));\
		} \
	} \
	else {\
		DMX_ASSERT(FALSE);\
	} \
} while (0)

/*!
 * @brief Allocate Physical-Continuously Memory
 *
 * This function is used to allocate hysical-Continuously memory from MM
 * reserved Memory area.
 *
 * @retval if success, return E_DMX_OK.
 *         otherwise, return the corresponding errcode
 */
#define DMX_NewHwMemory(u4Sz, pvMem)	do {\
	if ((void *)NULL == (void *)(pvMem)) {\
		dmx_alloc_cp_mem((pvMem), (u4Sz));\
		if ((void *)(pvMem) != (void *)NULL)\
			dmx_memset((void *)(pvMem), (u8)0x00, (u32)(u4Sz));\
		} \
	else {\
		DMX_ASSERT(FALSE);\
	} \
} while (0)

#define DMX_NewHwAlignMemory(u4Sz, u4Align, pvMem)	do {\
	if ((void *)NULL == (void *)(pvMem)) {\
		dmx_alloc_aligned_cp_mem((pvMem), (u4Sz), (u4Align));\
		if ((void *)(pvMem) != (void *)NULL)\
			dmx_memset((void *)(pvMem), (u8)0x00, (u32)(u4Sz));\
	} \
	else {\
		DMX_ASSERT(FALSE);\
	} \
} while (0)

#define DMX_NewHwAlignMemoryEx(u4Sz, u4Align, pvMem)	do {\
	if ((void *)NULL == (void *)(pvMem)) {\
		dmx_alloc_aligned_cpex_mem((pvMem), (u4Sz), (u4Align));\
		if ((void *)(pvMem) != (void *)NULL)\
			dmx_memset((void *)(pvMem), (u8)0x00, (u32)(u4Sz));\
	} \
	else {\
		DMX_ASSERT(FALSE);\
	} \
} while (0)

#define DMX_FreeMemory(pvMem)	do {\
	if ((void *)(pvMem) != (void *)NULL) {\
		dmx_free_np_mem(pvMem);\
		pvMem = (__typeof__(pvMem)) NULL;\
	} \
} while (0)

#define DMX_FreeHwMemory(pvMem)  dmx_free_cp_mem(pvMem)

#else				/* #ifdef __linux__ */

#if DMX_MEM_PRINT_LINE_FUNCTION

#if DMX_MEM_GUARD_PATTEN_CHECK

#define dmx_alloc_np_mem_ex(pvMem, u4Size, szFunc, i4Line)	do {\
	if (0 < (u4Size)) {\
		u32 u4AllocSize =\
			DMX_MEM_Align((u4Size),\
				DMX_MIN_MEMORY_ALIGNMENT) + DMX_MIN_MEMORY_ALIGNMENT +\
				DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8) * 2;\
		pvMem = MM_ALLOC(u4AllocSize);\
	} \
	else {\
		pvMem = NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail for u4Size == 0 %s, line %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
	} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail %s, line %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
		DMX_Dump_Mem();\
		DMX_ASSERT(FALSE);\
	} \
	else {\
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s -- %s, line %d, alloc Size: %d, pvMem: 0x%08x\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line), (u4Size), (u32)(pvMem));\
		Add_VMemMap((void *)(pvMem), (u8)0x00, SPT_MEM_TYPE_OSEMALLOC, u4Size,\
			DMX_MIN_MEMORY_ALIGNMENT, (szFunc), (i4Line));\
		pvMem = ((u32)(pvMem) + DMX_MIN_MEMORY_ALIGNMENT);\
	} \
} while (0)

/* The following function is used to alloc HW continous memory, the memory is 32-aligned */

#define dmx_alloc_cp_mem_ex(pvMem, u4Size, szFunc, i4Line)	do {\
	uintptr_t ptrPhysAddr;\
	if (0 < (u4Size)) {\
		u32 u4AllocSize =\
			DMX_MEM_Align((u4Size), DMX_MIN_MEMORY_ALIGNMENT)\
			+ DMX_MIN_MEMORY_ALIGNMENT +\
			DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8) * 2;\
		pvMem =\
			OSE_MemAllocCustom(OSE_DEMUXER, u4AllocSize, DMX_MIN_MEMORY_ALIGNMENT,\
			&ptrPhysAddr);\
	} \
	else {\
		pvMem = NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail for u4Size == 0 %s, line %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
	} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail %s, %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
		DmxCliDumpMemUsage();\
		DMX_ASSERT(FALSE);\
	} \
	else {\
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s -- %s, line %d, alloc Size: %d, pvMem: 0x%08x\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line), (u4Size), (uintptr_t)(pvMem));\
		Add_MemMap((void *)(pvMem), (void *)ptrPhysAddr, SPT_MEM_TYPE_OSEPHY,\
			(u4Size), DMX_MIN_MEMORY_ALIGNMENT, (szFunc), (i4Line));\
		pvMem = ((uintptr_t)(pvMem) + DMX_MIN_MEMORY_ALIGNMENT);\
		ptrPhysAddr += DMX_MIN_MEMORY_ALIGNMENT;\
	} \
} while (0)

#define dmx_alloc_aligned_cp_mem_ex(pvMem, u4Size, u4Align, szFunc, i4Line)	do {\
	uintptr_t ptrPhysAddr;\
	if (0 < (u4Size)) {\
		u32 u4AllocSize =\
			DMX_MEM_Align((u4Size), (u4Align)) + (u4Align) +\
			DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8) * 2;\
		pvMem =\
			OSE_MemAllocCustom(OSE_DEMUXER, u4AllocSize, (u4Align), &ptrPhysAddr);\
	} \
	else {\
		pvMem = NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail for u4Size == 0 %s, line %d\r\n"),\
		DMX_FUNC_NAME, (szFunc), (i4Line));\
	} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail %s, %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
		DmxCliDumpMemUsage();\
		DMX_ASSERT(FALSE);\
	} \
	else {\
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s -- %s, line %d, alloc Size: %d, pvMem: 0x%08x\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line), (u4Size), (uintptr_t)(pvMem));\
		Add_MemMap((void *)(pvMem), (void *)ptrPhysAddr, SPT_MEM_TYPE_OSEPHY,\
			(u4Size), (u4Align), (szFunc), (i4Line));\
		pvMem = ((uintptr_t)(pvMem) + (u4Align));\
		ptrPhysAddr += u4Align;\
	} \
} while (0)

#define dmx_alloc_aligned_cpex_mem_ex(pvMem, u4Size, u4Align, szFunc, i4Line)	do {\
	uintptr_t ptrPhysAddr;\
	if (0 < (u4Size)) {\
		u32 u4AllocSize =\
			DMX_MEM_Align(((u4Size) + (u4Align)), (u4Align)) + (u4Align) +\
			DMX_MEM_GUARD_PATTEN_LEN * sizeof(u8) * 2;\
		pvMem =\
			OSE_MemAllocCustom(OSE_DEMUXER, u4AllocSize, (u4Align), &ptrPhysAddr);\
	} \
	else {\
		pvMem = NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail for u4Size == 0 %s, line %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
	} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail %s, %d\r\n"), DMX_FUNC_NAME,\
			(szFunc), (i4Line));\
		DmxCliDumpMemUsage();\
		DMX_ASSERT(FALSE);\
	} \
	else {\
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s -- %s, line %d, alloc Size: %d, pvMem: 0x%08x\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line), (u4Size), (uintptr_t)(pvMem));\
		Add_MemMap((void *)(pvMem), (void *)ptrPhysAddr, SPT_MEM_TYPE_OSEPHY,\
			((u4Size) + (u4Align)), (u4Align), (szFunc), (i4Line));\
		pvMem = ((uintptr_t)(pvMem) + (u4Align));\
		ptrPhysAddr += u4Align;\
	} \
} while (0)

#else				/* DMX_MEM_GUARD_PATTEN_CHECK */

#define dmx_alloc_np_mem_ex(pvMem, u4Size, szFunc, i4Line)	do {\
	if (0 < (u4Size)) {\
		pvMem = MM_ALLOC(u4Size);\
	} \
	else {\
		pvMem = NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail for u4Size == 0 %s, line %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
	} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail %s, %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
		DMX_Dump_Mem();\
		DMX_ASSERT(FALSE);\
	} \
	else {\
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s -- %s, %d, alloc Size: %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line), (u4Size));\
	} \
} while (0)

#define dmx_alloc_cp_mem_ex(pvMem, u4Size, szFunc, i4Line)	do {\
	uintptr_t ptrPhysAddr;\
	if (0 < (u4Size)) {\
		pvMem =\
			OSE_MemAllocCustom(OSE_DEMUXER, (u4Size), DMX_MIN_MEMORY_ALIGNMENT,\
			&ptrPhysAddr);\
	} \
	else {\
		pvMem = NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail for u4Size == 0 %s, line %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
	} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail %s, %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
		DmxCliDumpMemUsage();\
		DMX_ASSERT(FALSE);\
	} \
	else {\
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s -- %s, %d, alloc Size: %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line), (u4Size));\
		Add_MemMap((void *)(pvMem), (void *)ptrPhysAddr, SPT_MEM_TYPE_OSEPHY,\
			(u4Size), DMX_MIN_MEMORY_ALIGNMENT, (szFunc), (i4Line));\
	} \
} while (0)

#define dmx_alloc_aligned_cp_mem_ex(pvMem, u4Size, u4Align, szFunc, i4Line)	do {\
	uintptr_t ptrPhysAddr;\
	if (0 < (u4Size)) {\
		pvMem = OSE_MemAllocCustom(OSE_DEMUXER, (u4Size), (u4Align), &ptrPhysAddr);\
	} \
	else {\
		pvMem = NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail for u4Size == 0 %s, line %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
	} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail %s, %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
		DmxCliDumpMemUsage();\
		DMX_ASSERT(FALSE);\
	} \
	else {\
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s -- %s, %d, alloc Size: %d\r\n"),\
			DMX_FUNC_NAME, szFunc, i4Line, (u4Size));\
		Add_MemMap((void *)(pvMem), (void *)ptrPhysAddr, SPT_MEM_TYPE_OSEPHY,\
			(u4Size), (u4Align), (szFunc), (i4Line));\
	} \
} while (0)

#define dmx_alloc_aligned_cpex_mem_ex(pvMem, u4Size, u4Align, szFunc, i4Line)	do {\
	uintptr_t ptrPhysAddr;\
	if (0 < (u4Size)) {\
		pvMem = OSE_MemAllocCustom(OSE_DEMUXER, (u4Size) + (u4Align),\
			(u4Align), &ptrPhysAddr);\
		} \
	else{\
		pvMem = NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail for u4Size == 0 %s, line %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
		} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s fail %s, %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line));\
		DmxCliDumpMemUsage();\
		DMX_ASSERT(FALSE);\
	} \
	else{\
		DmxLogD(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s -- %s, %d, alloc Size: %d\r\n"),\
			DMX_FUNC_NAME, (szFunc), (i4Line), ((u4Size) + (u4Align)));\
		Add_MemMap((void *)(pvMem), (void *)ptrPhysAddr, SPT_MEM_TYPE_OSEPHY,\
			((u4Size) + (u4Align)), (u4Align), (szFunc), (i4Line));\
		} \
} while (0)

#endif				/* DMX_MEM_GUARD_PATTEN_CHECK */

/* #define dmx_mem_alloc(pvMem, size)*/
		/*dmx_alloc_np_mem_ex(pvMem, (size), __FUNCTION__, __LINE__) */
#define dmx_alloc_cp_mem(pvMem, size)\
			dmx_alloc_cp_mem_ex((pvMem), (size), __func__, __LINE__)
#define dmx_alloc_aligned_cp_mem(pvMem, size, align)\
			dmx_alloc_aligned_cp_mem_ex((pvMem), (size), (align), __func__, __LINE__)
#define dmx_alloc_aligned_cpex_mem(pvMem, size, align)\
			dmx_alloc_aligned_cpex_mem_ex((pvMem), (size), (align), __func__, __LINE__)

#if DMX_VMALLOC_FROM_RESEVED_MEM
#define dmx_mem_alloc(pvMem, size)\
			dmx_alloc_cp_mem_ex((pvMem), (size), __func__, __LINE__)
#define dmx_free_np_mem(pvMem)	do {\
	if ((void *)NULL != (void *)(pvMem)) {\
		Del_MemMap((void *)(pvMem));\
	} \
} while (0)

#else

#define dmx_mem_alloc(pvMem, size)	dmx_alloc_np_mem_ex((pvMem), (size), __func__, __LINE__)
#define dmx_free_np_mem(pvMem)	do {\
	if ((void *)NULL != (void *)(pvMem)) {\
		Del_VMemMap((void *)(pvMem));\
	} \
} while (0)

#endif				/* #if DMX_VMALLOC_FROM_RESEVED_MEM */

#define  dmx_free_cp_mem(pvMem)	do {\
	if ((void *)NULL != (void *)(pvMem)) {\
		Del_MemMap((void *)(pvMem));\
	} \
} while (0)

#define  dmx_free_aligned_cp_mem(pvMem)	do {\
	if ((void *)NULL != (void *)(pvMem)) {\
		Del_MemMap((void *)(pvMem));\
	} \
} while (0)

#define dmx_free_aligned_cpex_mem(pvMem)	do {\
	if ((void *)NULL != (void *)(pvMem)) {\
		Del_MemMap((void *)(pvMem));\
	} \
} while (0)

#else				/* DMX_MEM_PRINT_LINE_FUNCTION */

/* ////////////////////////////////////////////////////////////////////////////// */
/* //////////////////Memory alloc and free////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////////// */
#define dmx_alloc_np_mem(pvMem, u4Size)	do {\
	if (0 < (u4Size)) {\
		pvMem = MM_ALLOC(u4Size);\
	} \
	else {\
		pvMem = NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s line %d fail for u4Size == 0\r\n"),\
			DMX_FUNC_NAME, DMX_LINE_NO);\
	} \
} while (0)

#define dmx_alloc_cp_mem(pvMem, u4Size)	do {\
	uintptr_t ptrPhysAddr = 0;\
	if (0 < (u4Size)) {\
		pvMem =\
			OSE_MemAllocCustom(OSE_DEMUXER, (u4Size), DMX_MIN_MEMORY_ALIGNMENT,\
			&ptrPhysAddr);\
	} \
	else { \
		pvMem = NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s line %d fail for u4Size == 0\r\n"),\
			DMX_FUNC_NAME, DMX_LINE_NO);\
		} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DMX_ASSERT(FALSE);\
	} \
	else {\
		Add_MemMap((void *)(pvMem), (void *)ptrPhysAddr, SPT_MEM_TYPE_OSEPHY,\
			u4Size);\
	} \
} while (0)

#define dmx_alloc_aligned_cp_mem(pvMem, u4Size, u4Align)	do {\
	uintptr_t ptrPhysAddr = 0;\
	if (0 < (u4Size)) {\
		pvMem = OSE_MemAllocCustom(OSE_DEMUXER, (u4Size), (u4Align), &ptrPhysAddr);\
	} \
	else{\
		pvMem = NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s line %d fail for u4Size == 0\r\n"),\
			DMX_FUNC_NAME, DMX_LINE_NO);\
	} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DMX_ASSERT(FALSE);\
	} \
	else {\
		Add_MemMap((void *)(pvMem), (void *)ptrPhysAddr, SPT_MEM_TYPE_OSEPHY,\
			(u4Size));\
	} \
} while (0)

#define dmx_alloc_aligned_cpex_mem(pvMem, u4Size, u4Align)	do {\
	uintptr_t ptrPhysAddr;\
	if (0 < (u4Size)) {\
		pvMem = OSE_MemAllocCustom(OSE_DEMUXER, (u4Size) + (u4Align),\
			(u4Align), &ptrPhysAddr);\
	} \
	else {\
		pvMem = NULL;\
		DmxLogE(DMX_MOD_MEM, DMX_MOD_MEM_LOGLVL_DEFAULT,\
			TEXT("[MEM] %s line %d fail for u4Size == 0\r\n"),\
			DMX_FUNC_NAME, DMX_LINE_NO);\
	} \
	if ((void *)NULL == (void *)(pvMem)) {\
		DMX_Dump_Mem();\
		DMX_ASSERT(FALSE);\
	} \
	else {\
		Add_MemMap((void *)(pvMem), (void *)ptrPhysAddr, SPT_MEM_TYPE_OSEPHY,\
			((u4Size) + (u4Align)));\
	} \
} while (0)

#define dmx_mem_alloc(pvMem, size)      dmx_alloc_np_mem((pvMem), (size))

#define  dmx_free_np_mem(pvMem)	do {\
	if ((void *)NULL != (void *)(pvMem)) {\
		MM_FREE((void *)(pvMem));\
	} \
} while (0)

#define  dmx_free_cp_mem(pvMem)do {\
	if ((void *)NULL != (void *)(pvMem)) {\
		Del_MemMap((void *)(pvMem));\
		OSE_MemFreeCustom(OSE_DEMUXER, (void *)(pvMem));\
	} \
} while (0)

#define  dmx_free_aligned_cp_mem(pvMem)	do {\
	if ((void *)NULL != (void *)(pvMem)) {\
		Del_MemMap((void *)(pvMem));\
		OSE_MemFreeCustom(OSE_DEMUXER, (void *)(pvMem));\
	} \
} while (0)

#define dmx_free_aligned_cpex_mem(pvMem)	do {\
	if ((void *)NULL != (void *)(pvMem)) {\
		Del_MemMap((void *)(pvMem));\
		OSE_MemFreeCustom(OSE_DEMUXER, (void *)(pvMem));\
	} \
} while (0)

#endif				/* DMX_MEM_PRINT_LINE_FUNCTION */

/*!
 * @brief Allocate heap memory
 *
 * This function is used to allocate heap memory, it equals to MM_ALLOC
 *
 * @retval if success, return E_DMX_OK.
 *         otherwise, return the corresponding errcode
 */
#define DMX_NewMemory(u4Sz, pvMem)	do {\
	if ((void *)NULL == (void *)(pvMem)) {\
		dmx_mem_alloc(pvMem, (u32)(u4Sz));\
		if ((void *)(pvMem) != (void *)NULL) {\
			dmx_memset((void *)(pvMem), (u8)0x00, (u32)(u4Sz));\
		} \
	} \
	else {\
		DMX_ASSERT(FALSE);\
	} \
} while (0)

/*!
 * @brief Allocate Physical-Continuously Memory
 *
 * This function is used to allocate hysical-Continuously memory from MM
 * reserved Memory area.
 *
 * @retval if success, return E_DMX_OK.
 *         otherwise, return the corresponding errcode
 */
#define DMX_NewHwMemory(u4Sz, pvMem)	do {\
	if ((void *)NULL == (void *)(pvMem)) {\
		dmx_alloc_cp_mem((pvMem), (u4Sz));\
		if ((void *)(pvMem) != (void *)NULL)\
			dmx_memset((void *)(pvMem), (u8)0x00, (u32)(u4Sz));\
	} \
	else {\
		DMX_ASSERT(FALSE);\
	} \
} while (0)

#define DMX_NewHwAlignMemory(u4Sz, u4Align, pvMem)	do {\
	if ((void *)NULL == (void *)(pvMem)) {\
		dmx_alloc_aligned_cp_mem(pvMem, u4Sz, u4Align);\
		if ((void *)(pvMem) != (void *)NULL)\
			dmx_memset((void *)(pvMem), (u8)0x00, (u32)(u4Sz));\
		} \
	else {\
		DMX_ASSERT(FALSE);\
		} \
} while (0)

#define DMX_NewHwAlignMemoryEx(u4Sz, u4Align, pvMem)	do {\
	if ((void *)NULL == (void *)(pvMem)) {\
		dmx_alloc_aligned_cpex_mem((pvMem), (u4Sz), (u4Align));\
		if ((void *)(pvMem) != (void *)NULL)\
			dmx_memset((void *)(pvMem), (u8)0x00, (u32)(u4Sz));\
	} \
	else {\
		DMX_ASSERT(FALSE);\
		} \
} while (0)

#define DMX_FreeMemory(pvMem)	do { \
	if ((void *)(pvMem) != (void *)NULL) {\
		pvMem = NULL;\
	} \
} while (0)

#define DMX_FreeHwMemory(pvMem)  dmx_free_cp_mem(pvMem)

#endif				/* #ifdef __linux__ */

/* ////////////////////////////////////////////////////////////////////////////// */
/* /////////////////        Memory Related Functions             //////////////// */
/* ////////////////////////////////////////////////////////////////////////////// */
void DMX_Dump_Mem(void);
void DMX_CheckMemList(void);

/* Initialize Splitter memory management */
MRESULT dmx_Mem_Init(void);

/* DeInitialize Splitter memory management */
void dmx_Mem_Uninit(void);
bool DMX_CheckPesHdrMemGuard(uintptr_t ptrPhyPesHdrSa);

/* Add HW Physical Address and its Virtual Memory Address mapping entry */
/*  into Splitter memory management */
#if DMX_MEM_PRINT_LINE_FUNCTION
MRESULT Add_MemMap(void *pvVMem, void *pvPMem, E_SPT_MEM_TYPE_T eType,
	u32 u4Size, u32 u4Align, const char *szFunc, s32 i4Line);

#if DMX_MEM_GUARD_PATTEN_CHECK
MRESULT Add_VMemMap(void *pvVMem, void *pvPMem, E_SPT_MEM_TYPE_T eType,
			u32 u4Size, u32 u4Align, const char *szFunc, s32 i4Line);
void Del_VMemMap(void *pvVMem);

#endif				/* DMX_MEM_GUARD_PATTEN_CHECK */
#else				/* DMX_MEM_PRINT_LINE_FUNCTION */
MRESULT Add_MemMap(void *pvVMem, void *pvPMem, E_SPT_MEM_TYPE_T eType,
			u32 u4Size);

#endif				/* DMX_MEM_PRINT_LINE_FUNCTION */

/* Delete HW Physical Address and its Virtual Memory Address mapping */
/* entry into Splitter memory management */
void Del_MemMap(void *pvVMem);

/* Get  designated Virtual Address's HW Physical Address, */
#if DMX_MEM_PRINT_LINE_FUNCTION
uintptr_t DMX_PHYSICALEX(uintptr_t ptrVMem, const char *szFunc, s32 i4Line);

#define DMX_PHYSICAL(u4VMem) DMX_PHYSICALEX(u4VMem, __func__, __LINE__)
uintptr_t DMX_NONCACHEEX(uintptr_t ptrPMem, const char *szFunc, s32 i4Line);

#define DMX_NONCACHE(ptrPMem) DMX_NONCACHEEX(ptrPMem, __func__, __LINE__)
#else
u32 DMX_PHYSICAL(uintptr_t ptrVMem);

/* Get  designated HW Physical Address's mapping Virtual Address */
u32 DMX_NONCACHE(uintptr_t ptrPMem);

#endif	/* #if DMX_MEM_PRINT_LINE_FUNCTION */
bool Dmx_RingMemCpy(u8 *pu1DstBufSA, u8 *pu1DstBufEA, u8 *pu1DstAddr,
		u8 *pu1SrcAddr, u32 u4Size);

#ifdef __cplusplus
}


#endif

#endif	/* #ifndef DMX_INTERNAL_MEM_H */
