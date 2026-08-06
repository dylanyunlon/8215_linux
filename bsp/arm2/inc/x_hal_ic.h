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

#ifndef X_HAL_ic_H
#define X_HAL_ic_H


#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_AC83XX)
#include "x_hal_83xx.h"
#endif

#if 0
#ifdef  __ARM2__
#include "x_hal_83xx.h"
#endif
#endif

#if 0
//============================================================================
// Macros for register read
//============================================================================
#define IO_READ8(base, offset)                          HAL_READ8((base) + (offset))
#define IO_READ16(base, offset)                         HAL_READ16((base) + (offset))
#define IO_READ32(base, offset)                         HAL_READ32((base) + (offset))

//============================================================================
// Macros for register write
//============================================================================
#define IO_WRITE8(base, offset, value)                  HAL_WRITE8((base) + (offset), (value))
#define IO_WRITE16(base, offset, value)                 HAL_WRITE16((base) + (offset), (value))
#define IO_WRITE32(base, offset, value)                 HAL_WRITE32((base) + (offset), (value))

//============================================================================
// Macros for register read/write access
//============================================================================
#define IO_REG8(base, offset)                           HAL_REG8((base) + (offset))
#define IO_REG16(base, offset)                          HAL_REG16((base) + (offset))
#define IO_REG32(base, offset)                          HAL_REG32((base) + (offset))
#else
//#include <mach/ac83xx_io_macros.h>
#endif

#define IO_REG8(base, offset)                           (*((volatile UINT8*)(base + offset)))
#define IO_REG16(base, offset)                          (*((volatile UINT16*)(base + offset)))
#define IO_REG32(base, offset)                          (*((volatile UINT32*)(base + offset)))


#include "x_dram_size.h"
#include "x_assert.h"
//============================================================================
// Macros for address translation
//============================================================================
#if 0  //now we use reserve memory from dts, MACROs here will not use anymore 
#if CONFIG_DRV_LINUX
#ifdef __KERNEL__
    //#include <asm/memory.h>
#if 1
	#include "x_dram_size.h"
	#include "x_assert.h"

	#define STATIC_MAPPING_PHYSICAL	RESV_MEM_IO_PHYS
	#define STATIC_MAPPING_VIRTUAL RESV_MEM_IO_VIRT
	#define STATIC_MAPPING_OFFSET	(STATIC_MAPPING_VIRTUAL-STATIC_MAPPING_PHYSICAL)
	#define STATIC_MAPPING_SIZE		RESV_MEM_IO_SIZE

    static inline UINT32 PHYSICAL(UINT32 addr)
	{
		ASSERT((STATIC_MAPPING_VIRTUAL<=addr) && (addr<(STATIC_MAPPING_VIRTUAL + STATIC_MAPPING_SIZE)));
		return (UINT32)(addr - STATIC_MAPPING_OFFSET);
	}
	static inline UINT32 VIRTUAL(UINT32 addr)
	{
		ASSERT((STATIC_MAPPING_PHYSICAL<=addr) && (addr<(STATIC_MAPPING_PHYSICAL + STATIC_MAPPING_SIZE)));
		return (UINT32)(addr + STATIC_MAPPING_OFFSET);
	}
	#define _PHYSICAL(addr)	(addr - STATIC_MAPPING_OFFSET)
	#define _VIRTUAL(addr)	(addr + STATIC_MAPPING_OFFSET)
#else
    #define PHYSICAL(addr)                                  __pa(addr)
    #define VIRTUAL(addr)                                   __va(addr)
#endif

#endif
#else
#ifdef WIN32

#if 0

#define PHYSICAL(addr)	( ((addr) - 0xAC000000) )
#define VIRTUAL(addr)	( ((addr) + 0xAC000000) )

#else
static UINT32 PHYSICAL(UINT32 addr)
{
	ASSERT((0xAC000000<=addr) && (addr<0xC0000000));
	//ASSERT((0x8C000000<=addr) && (addr<0xA0000000));
	return (UINT32)(addr - 0xAC000000);
}

static UINT32 VIRTUAL(UINT32 addr)
{
	ASSERT(addr<0x14000000);
	return (UINT32)(addr + 0xAC000000);
	//return (UINT32)(addr + 0x8C000000);
}

#endif

#else
	#define CACHE(addr) 									((addr) & 0x3fffffff)
	#define NONCACHE(addr)									(((addr) & 0x3fffffff) | 0xC0000000)
	#define PHYSICAL(addr)									((addr) & 0x3fffffff)
	#define VIRTUAL(addr)									(addr)
#endif
#endif
#endif
//============================================================================
// Macros for check if cache
//============================================================================
#define IFCACHEABLE(addr)                               (((addr) < (0x20000000)) ? (TRUE) : (FALSE))

//============================================================================
// Exported functions
//============================================================================
extern void* BSP_AllocateReserved(UINT32 u4Size);
extern void* BSP_AllocateReservedAlgin(UINT32 u4Size, UINT32 u4Align);
extern void BSP_FreeReserved(void* p);

extern UINT32 BSP_GetDRAMClock(void);
extern UINT32 BSP_GetRISCClock(void);
extern UINT32 BSP_GetBUSClock(void);

extern UINT32 BDP_GetBootLoaderVersion(void);
extern UINT32 BDP_GetNeUpgVersion(void);

extern UINT32 BSP_GetDRAMOffset(void);
extern UINT32 BSP_GetARM2DRAMOffset(void);
extern UINT32 BSP_GetDRAMSize(void);
extern UINT32 BSP_GetDRAMPhySize(void);
extern UINT32 BSP_GetDRAMCH1Size(void);

extern UINT32 BSP_GetROStart(void);
extern UINT32 BSP_GetROEnd(void);
extern UINT32 BSP_GetRWStart(void);
extern UINT32 BSP_GetCh2MemStart(void);
extern UINT32 BSP_GetDRAMCH2Size(void);

extern void BSP_HaltSystem(void);

#ifndef CHIP_VER_AC83XX
extern IC_VERSION_T BSP_GetIcVersion(void);
#endif

extern BOOL BSP_IsFPGA(void);

extern BOOL BSP_GetIcBounding(UINT32 u4PROT);
extern BOOL BSP_GetIcTrapping(UINT32 u4Trap);

#if CONFIG_DRV_LINUX
//----------------------------------------------------------------------------
// IRQ Priority
typedef enum {
    IRQ_PRIORITY_NONE = 0,
    IRQ_PRIORITY_LOW,
    IRQ_PRIORITY_MID,
    IRQ_PRIORITY_HIGH
}IRQ_PRIORITY;

extern IRQ_PRIORITY BSP_GetIrqPriority(UINT32 irq_num);
extern BOOL BSP_SetIrqPriority(UINT32 irq_num, IRQ_PRIORITY ePri);
#endif

//============================================================================
// Exported global variables
//============================================================================

#endif  // X_HAL_ic_H
