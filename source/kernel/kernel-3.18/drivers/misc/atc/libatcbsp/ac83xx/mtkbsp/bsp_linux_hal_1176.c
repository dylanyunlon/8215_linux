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

//============================================================================
// Include files
//============================================================================
#include <linux/module.h>      //Must be included header file
#include <mach/ac83xx_CS_watch.h>
#include "section.h"
//#include "drv_config_tz.h"
#include "x_typedef.h"
#include "x_hal_1176.h"
#include "x_assert.h"
#include "x_hal_ic.h"
#include <linux/spinlock_types.h>

#define CONFIG_DRV_TZ_SUPPORT 1
#define CONFIG_DRV_CONFIG_L2  0

static DEFINE_SPINLOCK(ac83xx_bsp_hal_1176_lock);

//#include "drv_config_l2.h"
//============================================================================
// Constant definitions
//============================================================================

//============================================================================
// IRQ/FIQ related definitions
//============================================================================
// PSR bit definitions
#define PSR_IRQ_DISABLE             (1 << 7)
#define PSR_FIQ_DISABLE             (1 << 6)

//============================================================================
// Cache related definitions
//============================================================================

//CP15 control register (c1) bit definitions
#define ICACHE_ENABLE               (1 << 12)
#define DCACHE_ENABLE               (1 << 2)
#define MMU_ENABLE                  (1 << 0)
#define SYS_PROTECT                 (1 << 8)
#define ROM_PROTECT                 (1 << 9)
#define BRANCH_PREDICTION_ENABLE    (1 << 11)

//CP15 cache type register (c0) bit definitions
#define CTYPE_MASK                  0xf
#define CTYPE_SHIFT                 0x25
#define S_MASK                      1
#define S_SHIFT                     24
#define DSIZE_MASK                  0xf
#define DSIZE_SHIFT                 (6 + 12)
#define ISIZE_MASK                  0xf
#define ISIZE_SHIFT                 6

//Translate VA (Virtual Address) to MVA (Modified Virtual Address)
#define MVA(addr)                   ((addr) & 0xffffffe0)

// Cache parameters (in bytes)
#define ICACHE_LINE_SIZE			32
#define DCACHE_LINE_SIZE			32

// Performance Monitor Control
#define PMC_EVT_COUNT_0 (20)
#define PMC_EVT_COUNT_1 (12)
#define PMC_CR_1                (1 << 9)
#define PMC_CR_0                (1 << 8)
#define PMC_EC_1                (1 << 5)
#define PMC_EC_0                (1 << 4)
#define PCM_P_RESET          (1 << 1)  // Reset Counter0, 1 to 0x0
#define PCM_E_ENABLE        (1 << 0)  // Enable Counter0, 1 to 0x0


#define EVT_NUM_DCACHE_MISS   (0xB)
#define EVT_NUM_DCACHE_ACCESS (0x9)
#define EVT_NUM_INSTRU_EXECED (0x7)
#define EVT_NUM_ICACHE_MISS   (0x0)

//============================================================================
// Page table related definitions
//============================================================================
//Domain access control definitions of CP15 register r3
enum DOMAIN_ACCESS
{
  NO_ACCESS = 0,
  CLIENT    = 1,
  RESERVED  = 2,
  MANAGER   = 3
};

enum PAGE_TABLE_ENTRY_TYPE
{
  INVALID     = 0,
  COARSE_PAGE = 1,
  SECTION     = 2,
  FINE_PAGE   = 3   //ARM v6 Remove FINE_PAGE
};

enum L2_ENTRY_TYPE
{
  LARGE       = 1,
  SMALL       = 2,
  TINY        = 3   //ARM v6 Remove TINY
};

//Bit definitions of page table
#define C_BIT                       (1 << 3)    //Cachable
#define B_BIT                       (1 << 2)    //Bufferable
#define U_BIT                       (1 << 4)    //Must be 1, for backward compatibility
#define NS_BIT                      (1 << 19)   //For TZ

//TCM Define
#define ITCM_ALIGNMENT  0x00000FFF
#define DTCM_ALIGNMENT  0x00000FFF

#define TCM_DMA_CONTROL_TR        (1U << 31)
#define TCM_DMA_CONTROL_DT        (1U << 30)
#define TCM_DMA_CONTROL_ST_MASK   0x000FFF00
#define TCM_DMA_CONTROL_ST_OFFSET 8
#define TCM_DMA_CONTROL_TS_MASK   0x00000003
#define TCM_DMA_CONTROL_TS_OFFSET 0
#define TCM_DMA_ENABLE            3
//============================================================================
// Macro definitions
//============================================================================


//============================================================================
// Public functions
//============================================================================

//============================================================================
// IRQ/FIQ related functions
//============================================================================



/*----------------------------------------------------------------------------
 * HalEnableIRQ() Enable IRQ
 *---------------------------------------------------------------------------*/
void HalEnableIRQ(void)
{
  UINT32 r = 0;
  UINT32 u4PsrIrqDisable = PSR_IRQ_DISABLE;

  __asm__ volatile(
    "    MRS     %0, CPSR \n"
    "    BIC     %0, %1, %2 \n"
    "    MSR     CPSR_c, %1 \n"
    : "=r" (r)
    : "0" (r), "r" (u4PsrIrqDisable)
  );
}
EXPORT_SYMBOL(HalEnableIRQ);

/*----------------------------------------------------------------------------
 * HalDisableIRQ() Disable IRQ
 *---------------------------------------------------------------------------*/
void HalDisableIRQ(void)
{
  UINT32 r = 0;
  UINT32 u4PsrIrqDisable = PSR_IRQ_DISABLE;

  __asm__ volatile(
    "    MRS     %0, CPSR \n"
    "    ORR     %0, %1, %2 \n"
    "    MSR     CPSR_c, %1 \n"
    : "=r" (r)
    : "0" (r), "r" (u4PsrIrqDisable)
  );
}
EXPORT_SYMBOL(HalDisableIRQ);

/*----------------------------------------------------------------------------
 * HalEnableFIQ() Enable FIQ
 *---------------------------------------------------------------------------*/
void HalEnableFIQ(void)
{
  UINT32 r = 0;
  UINT32 u4PsrFrqDisable = PSR_FIQ_DISABLE;
  
  __asm__ volatile(
    "    MRS     %0, CPSR \n"
    "    BIC     %0, %1, %2 \n"
    "    MSR     CPSR_c, %1 \n"
    : "=r" (r)
    : "0" (r), "r" (u4PsrFrqDisable)
  );
}
EXPORT_SYMBOL(HalEnableFIQ);

/*----------------------------------------------------------------------------
 * HalCriticalStart() Enter critical section, disable IRQ and FIQ
 *  @return The current processor status, which must be carried back to \n
 *               HalCritialEnd()
 *---------------------------------------------------------------------------*/
UINT32 HalCriticalStart(void)
{
  UINT32 r = 0, s = 0;
  UINT32 u4PsrIrqFiqDisable = (PSR_IRQ_DISABLE | PSR_FIQ_DISABLE);
  
  __asm__ volatile(
    "    MRS     %0, CPSR \n"
    "    ORR     %1, %2, %4 \n"
    "    MSR     CPSR_c, %3 \n"
    : "=r"(r), "=r"(s)
    : "0"(r), "1"(s), "r"(u4PsrIrqFiqDisable)
  );
#if CONFIG_AC83XX_CS_WATCH
  ac83xx_CS_watch_start(r);			    
#endif

  return r;
}
EXPORT_SYMBOL(HalCriticalStart);

/*----------------------------------------------------------------------------
 * HalCritialSemiStart() Enter critical section, disable IRQ (FIQ is still \n
 *                      enabled)
 *  @return The current processor status, which must be carried back to \n
 *               HalCritialEnd()
 *---------------------------------------------------------------------------*/
UINT32 HalCriticalSemiStart(void)
{
  UINT32 r = 0, s = 0;
  UINT32 u4PsrIrqDisable = PSR_IRQ_DISABLE;
  
  __asm__ volatile(
    "    MRS     %0, CPSR \n"
    "    ORR     %1, %2, %4 \n"
    "    MSR     CPSR_c, %3 \n"
    : "=r"(r), "=r"(s)
    : "0"(r), "1"(s), "r"(u4PsrIrqDisable)
  );
#if CONFIG_AC83XX_CS_WATCH
  ac83xx_CS_watch_start(r);			    
#endif

  return r;
}
EXPORT_SYMBOL(HalCriticalSemiStart);

/*----------------------------------------------------------------------------
 * HalCritialEnd() Leave critical section, restore IRQ and/or FIQ status
 *  @param u4Flags - The return value of the corresponding \n
 *                     HalCriticalStart() or HalCriticalSemiStart()
 *---------------------------------------------------------------------------*/
void HalCriticalEnd(UINT32 u4Flag)
{
#if CONFIG_AC83XX_CS_WATCH
  ac83xx_CS_watch_end(u4Flag);			    
#endif    
  __asm__ volatile(
    "    MSR     CPSR_c, %0 \n"
    :
    : "r" (u4Flag)
  );
#if CONFIG_AC83XX_CS_WATCH
  ac83xx_CS_watch_dump();		    
#endif     
}
EXPORT_SYMBOL(HalCriticalEnd);

/*----------------------------------------------------------------------------
 * HalDisableFIQ() Disable FIQ
 *---------------------------------------------------------------------------*/
void HalDisableFIQ(void)
{
  UINT32 r = 0;
  UINT32 u4PsrFrqDisable = PSR_FIQ_DISABLE;

  __asm__ volatile(
    "    MRS     %0, CPSR \n"
    "    ORR     %0, %1, %2 \n"
    "    MSR     CPSR_c, %1 \n"
    : "=r" (r)
    : "0" (r), "r" (u4PsrFrqDisable)
  );
}
EXPORT_SYMBOL(HalDisableFIQ);

//===========================================================================
// Cache related functions
//===========================================================================

/*----------------------------------------------------------------------------
 * HalIsICacheEnabled() Check if I-cache enabled
 * @return      : TRUE if I-cache enabled, FALSE if I-cache disabled
 *---------------------------------------------------------------------------*/
BOOL HalIsICacheEnabled(void)
{
  UINT32 r = 0;

  __asm__ volatile(
    "    MRC     p15, 0, %0, c1, c0, 0 \n"
    : "=r" (r)
  );

  return ((r & ICACHE_ENABLE) ? TRUE : FALSE);
}
EXPORT_SYMBOL(HalIsICacheEnabled);

/*----------------------------------------------------------------------------
 * HalIsDCacheEnabled() Check if D-cache enabled
 *  @return      : TRUE if D-cache enabled, FALSE if D-cache disabled
 *---------------------------------------------------------------------------*/
BOOL HalIsDCacheEnabled(void)
{
  UINT32 r = 0;

  __asm__ volatile(
    "    MRC     p15, 0, %0, c1, c0, 0 \n"
    : "=r" (r)
  );

  return ((r & DCACHE_ENABLE) ? TRUE : FALSE);
}
EXPORT_SYMBOL(HalIsDCacheEnabled);

/*----------------------------------------------------------------------------
 * HalGetICacheSize() Get I-cache size
 *  @return      : The size in byte of I-cache
 *---------------------------------------------------------------------------*/
UINT32 HalGetICacheSize(void)
{
  UINT32 r = 0;
  UINT32 u4SizeField, u4Size = 0;

  __asm__ volatile(
    "    MRC     p15, 0, %0, c0, c0, 1 \n"
    : "=r" (r)
  );

  u4SizeField = (r >> ISIZE_SHIFT) & ISIZE_MASK;

  switch (u4SizeField)
  {
    case 3:
      u4Size = 4;
      break;

    case 4:
      u4Size = 8;
      break;

    case 5:
      u4Size = 16;
      break;

    case 6:
      u4Size = 32;
      break;

    case 7:
      u4Size = 64;
      break;

    case 8:
      u4Size = 128;
      break;

    default:
      ASSERT(0);
      break;
  }

  return u4Size * 1024;
}
EXPORT_SYMBOL(HalGetICacheSize);

/*----------------------------------------------------------------------------
 * HalGetDCacheSize() Get D-cache size
 *  @return      : The size in byte of D-cache
 *---------------------------------------------------------------------------*/
UINT32 HalGetDCacheSize(void)
{
  UINT32 r = 0;
  UINT32 u4SizeField, u4Size = 0;

  __asm__ volatile(
    "    MRC     p15, 0, %0, c0, c0, 1 \n"
    : "=r" (r)
  );

  u4SizeField = (r >> DSIZE_SHIFT) & DSIZE_MASK;

  switch (u4SizeField)
  {
    case 3:
      u4Size = 4;
      break;

    case 4:
      u4Size = 8;
      break;

    case 5:
      u4Size = 16;
      break;

    case 6:
      u4Size = 32;
      break;

    case 7:
      u4Size = 64;
      break;

    case 8:
      u4Size = 128;
      break;

    default:
      ASSERT(0);
      break;
  }

  return u4Size * 1024;
}
EXPORT_SYMBOL(HalGetDCacheSize);

/*----------------------------------------------------------------------------
 * HalDisableICache() Disable I-cache
 *---------------------------------------------------------------------------*/
void HalDisableICache(void)
{
  UINT32 r = 0;
  UINT32 u4ICacheEnable = ICACHE_ENABLE;

  __asm__ volatile(
    "    MRC     p15, 0, %0, c1, c0, 0 \n"
    "    BIC     %0, %1, %2 \n"
    "    MCR     p15, 0, %1, c1, c0, 0 \n"
    : "=r" (r)
    : "0" (r), "r" (u4ICacheEnable)
  );
}
EXPORT_SYMBOL(HalDisableICache);

/*----------------------------------------------------------------------------
 * HalEnableDCache() Enable D-cache
 *---------------------------------------------------------------------------*/
void HalEnableDCache(void)
{
  UINT32 r = 0;
  UINT32 u4DCacheEnable = DCACHE_ENABLE;

  HalInvalidateDCache();

  __asm__ volatile(
    "    MRC     p15, 0, %0, c1, c0, 0 \n"
    "    ORR     %0, %1, %2 \n"
    "    MCR     p15, 0, %1, c1, c0, 0 \n"
    "    NOP \n"
    "    NOP \n"
    "    NOP \n"
    "    NOP \n"
    "    NOP \n"
    : "=r" (r)
    : "0" (r), "r" (u4DCacheEnable)
  );
}
EXPORT_SYMBOL(HalEnableDCache);

/*----------------------------------------------------------------------------
 * HalDisableDCache() Disable D-cache
 *---------------------------------------------------------------------------*/
void HalDisableDCache(void)
{
  UINT32 r = 0;
  UINT32 u4DCacheEnable = DCACHE_ENABLE;

  //D-cache must be cleaned of dirty data before it is disabled
  HalFlushDCache();

  __asm__ volatile(
    "    MRC     p15, 0, %0, c1, c0, 0 \n"
    "    BIC     %0, %1, %2 \n"
    "    MCR     p15, 0, %1, c1, c0, 0 \n"
    : "=r" (r)
    : "0" (r), "r" (u4DCacheEnable)
  );
}
EXPORT_SYMBOL(HalDisableDCache);

/*----------------------------------------------------------------------------
 * HalEnableCaches() Enable both I-cache and D-cache
 *---------------------------------------------------------------------------*/
void HalEnableCaches(void)
{
  UINT32 r = 0;
  UINT32 u4ICacheEnable = ICACHE_ENABLE;
  UINT32 u4DCacheEnable = DCACHE_ENABLE;

  #if CONFIG_DRV_TZ_SUPPORT

  #else
    //Non-Secure can not use InvalidateAllCache
    HalInvalidateICache();
    HalInvalidateDCache();
  #endif

  __asm__ volatile(
    "    MRC     p15, 0, %0, c1, c0, 0 \n"
    "    ORR     %0, %1, %2 \n"
    "    ORR     %0, %1, %3 \n"
    "    MCR     p15, 0, %1, c1, c0, 0 \n"
    "    NOP \n"
    "    NOP \n"
    "    NOP \n"
    "    NOP \n"
    "    NOP \n"
    : "=r" (r)
    : "0" (r), "r" (u4ICacheEnable), "r" (u4DCacheEnable)
  );
}
EXPORT_SYMBOL(HalEnableCaches);

/*----------------------------------------------------------------------------
 * HalDisableCaches() Disable both I-cache and D-cache
 *---------------------------------------------------------------------------*/
void HalDisableCaches(void)
{
  UINT32 r = 0;
  UINT32 u4ICacheEnable = ICACHE_ENABLE;
  UINT32 u4DCacheEnable = DCACHE_ENABLE;

  //D-cache must be cleaned of dirty data before it is disabled
  HalFlushDCache();

  __asm__ volatile(
    "    MRC     p15, 0, %0, c1, c0, 0 \n"
    "    BIC     %0, %1, %2 \n"
    "    BIC     %0, %1, %3 \n"
    "    MCR     p15, 0, %1, c1, c0, 0 \n"
    : "=r" (r)
    : "0" (r), "r" (u4ICacheEnable), "r" (u4DCacheEnable)
  );
}
EXPORT_SYMBOL(HalDisableCaches);

/*----------------------------------------------------------------------------
 * HalInvalidateAllCaches() Invalidate I-cache and D-cache
 *---------------------------------------------------------------------------*/
void HalInvalidateAllCaches(void)
{
  UINT32 r = 0;

  __asm__ volatile(
    "    MCR     p15, 0, %0, c7, c7, 0 \n"
    :
    : "r" (r)
  );
}
EXPORT_SYMBOL(HalInvalidateAllCaches);

/*----------------------------------------------------------------------------
 * HalInvalidateICache() Invalidate entire I-cache
 *---------------------------------------------------------------------------*/
void HalInvalidateICache(void)
{
  UINT32 r = 0;

  __asm__ volatile(
    "    MCR     p15, 0, %0, c7, c5, 0 \n"
    :
    : "r" (r)
  );
}
EXPORT_SYMBOL(HalInvalidateICache);

/*----------------------------------------------------------------------------
 * HalInvalidateDCache() Invalidate entire D-cache
 *---------------------------------------------------------------------------*/
void HalInvalidateDCache(void)
{
  UINT32 r = 0;

  __asm__ volatile(
    "    MCR     p15, 0, %0, c7, c6, 0 \n"
    :
    : "r" (r)
  );
}
EXPORT_SYMBOL(HalInvalidateDCache);

/*----------------------------------------------------------------------------
 * HalFlushWriteBuffer() Flush write buffer
 *---------------------------------------------------------------------------*/
void HalFlushWriteBuffer(void)
{
  UINT32 r = 0;

  __asm__ volatile(
    "    MCR     p15, 0, %0, c7, c10, 4 \n"
    :
    : "r" (r)
  );
}
EXPORT_SYMBOL(HalFlushWriteBuffer);

//lint -save -e* Don't lint the following two embedded assembly functions

/*----------------------------------------------------------------------------
 * _FlushInvalidateDCache() Flush (clean) and invalidate entire D-cache
 *---------------------------------------------------------------------------*/
#if CONFIG_DRV_CONFIG_L2
 #define L2Reg_WRITE32(offset, value)   (*((volatile UINT32*)(L2_IO_VIRT + offset)) = (value))
 #define L2Reg_READ32(offset)   (*((volatile UINT32*)(L2_IO_VIRT + offset)))
#endif
inline static void _FlushInvalidateDCache(void)
// Note: __asm function cannot be static
{
  UINT32 r1 = 0;
  UINT32 r2 = 0;
  UINT32 r3 = 0;
  UINT32 u4PsrIrqFiqDisable = (PSR_IRQ_DISABLE | PSR_FIQ_DISABLE);
#if !CONFIG_DRV_CONFIG_L2
  __asm__ volatile(
    "10: \n"
    "    MOV %0, #0 \n"
    "    MCR p15, 0, %3, c7, c14, 0  \n" // Clean (or Clean & Invalidate) Cache"
    "    MRS %1, CPSR \n"

    "    ORR %2, %4, %6 \n"
    "    MSR CPSR_c, %5 \n"

    "    MRC p15, 0, %0, c7, c10, 6  \n" // Read Cache Dirty Status Register \n"
    "    ANDS %0, %3, #1            \n"  // Check if it is clean \n"
    "    BEQ 20f \n"
    "    MSR     CPSR_c, %4          \n" // Re-enable interrupts \n"
    "    B 10b                     \n" // - clean the cache again \n"
    "20: \n"
    "    MSR     CPSR_c, %4       \n" // Re-enable interrupts \n"
    : "=r" (r1), "=r" (r2), "=r" (r3)
    : "0" (r1), "1" (r2), "2" (r3), "r" (u4PsrIrqFiqDisable)
  );
#else
  __asm__ volatile(
    "10: \n"
    "    MOV %0, #0 \n"
    "    MCR p15, 0, %3, c7, c14, 0  \n" // Clean (or Clean & Invalidate) Cache"
    "    MRS %1, CPSR \n"

    "    ORR %2, %4, %6 \n"
    "    MSR CPSR_c, %5 \n"

    "    MRC p15, 0, %0, c7, c10, 6  \n" // Read Cache Dirty Status Register \n"
    "    ANDS %0, %3, #1            \n"  // Check if it is clean \n"
    "    BEQ 20f \n"
    "    MSR     CPSR_c, %4          \n" // Re-enable interrupts \n"
    "    B 10b                     \n" // - clean the cache again \n"
    "20: \n"
    : "=r" (r1), "=r" (r2), "=r" (r3)
    : "0" (r1), "1" (r2), "2" (r3), "r" (u4PsrIrqFiqDisable)
  );
  if(L2Reg_READ32(L2_CTRL) & 0x01)
	  {	
    while(L2Reg_READ32(L2_CLEAN_INV_WAY));        //wait for done to prevent SLVERR
		L2Reg_WRITE32(L2_CLEAN_INV_WAY, 0x0000FFFF);  //clean & invalidate all
	  while(L2Reg_READ32(L2_CLEAN_INV_WAY));        //wait for done
	  }
	__asm__ volatile(  
    "    MSR     CPSR_c, %4       \n" // Re-enable interrupts \n"
    : "=r" (r1), "=r" (r2), "=r" (r3)
    : "0" (r1), "1" (r2), "2" (r3), "r" (u4PsrIrqFiqDisable)
  );
 #endif
}

/*----------------------------------------------------------------------------
 * HalFlushDCache() Flush (clean) entire D-cache
 *---------------------------------------------------------------------------*/
void HalFlushDCache(void)
{
  _FlushInvalidateDCache();
  HalFlushWriteBuffer();
}
EXPORT_SYMBOL(HalFlushDCache);

/*----------------------------------------------------------------------------
 * HalFlushInvalidateDCache() Flush (clean) and invalidate entire D-cache
 *---------------------------------------------------------------------------*/
void HalFlushInvalidateDCache(void)
{
  //unsigned long flags;

  spin_lock(&ac83xx_bsp_hal_1176_lock);
  _FlushInvalidateDCache();
  HalFlushWriteBuffer();
  spin_unlock(&ac83xx_bsp_hal_1176_lock);
}
EXPORT_SYMBOL(HalFlushInvalidateDCache);

//lint -restore Re-enable linting

/*----------------------------------------------------------------------------
 * HalWaitForInterrupt() Put the CPU into a low-power sleep state until an
 *                          interrupt
 *---------------------------------------------------------------------------*/
void HalWaitForInterrupt(void)
{
  UINT32 r = 0;

  __asm__ volatile(
    "    MCR     p15, 0, %0, c7, c0, 4 \n"
    :
    : "r" (r)
  );
}
EXPORT_SYMBOL(HalWaitForInterrupt);

//============================================================================
// debug
//============================================================================
/*----------------------------------------------------------------------------
 * HalIsDebugMode() Is debug mode
 *---------------------------------------------------------------------------*/
BOOL HalIsDebugMode(void)
{
  UINT32 r = 0;

  __asm__ volatile(
    "    MRC     p14, 0, %0, c0, c1, 0 \n"
    : "=r" (r)
  );

  return ((r & (1 << 14)) != 0);
}
EXPORT_SYMBOL(HalIsDebugMode);

/*----------------------------------------------------------------------------
 * HalBreakPoint() Breakpoint
 *---------------------------------------------------------------------------*/
void HalBreakPoint(void)
{
  // here is a software breakpoint.
  //__breakpoint(0xF02C);
  while(1);

  // if you want to leave this break function, set PC to here.
}
EXPORT_SYMBOL(HalBreakPoint);

/*----------------------------------------------------------------------------
 * HalInstructionMemoryBarrier() IMB that ensures consistency between the
 *                                  data and instruction streams
 *---------------------------------------------------------------------------*/
void HalInstructionMemoryBarrier(void)
{
  volatile UINT32* p;
  UINT32 i;

  //1. Clean D-cache and drain write buffer
  HalFlushDCache();

  //2. Synchronize data and instruction streams in level two AHB subsystems
  //   by using a nonbuffered store (STR) or a noncached load (LDR)
  p = (volatile UINT32*)0x20000000;        // DRAM-A, noncached address
  i = *p;
  UNUSED(i);

  //3. Invalidate the I-cache
  HalInvalidateICache();
}
EXPORT_SYMBOL(HalInstructionMemoryBarrier);


//============================================================================
// Branch Prediction related functions
//============================================================================
/*----------------------------------------------------------------------------
 * HalIsBranchPredictionEnabled() Check if Branch Prediction enabled
 *  @return TRUE if Branch Prediction enabled, FALSE if Branch Prediction disabled
 *---------------------------------------------------------------------------*/
BOOL HalIsBranchPredictionEnabled(void)
{
  UINT32 r = 0;

  __asm__ volatile(
    "    MRC     p15, 0, %0, c1, c0, 0 \n"
    : "=r" (r)
  );

  return ((r & BRANCH_PREDICTION_ENABLE) ? TRUE : FALSE);
}
EXPORT_SYMBOL(HalIsBranchPredictionEnabled);


/*----------------------------------------------------------------------------
 * HalEnableBranchPrediction() Enable Branch Prediction
 *---------------------------------------------------------------------------*/
void HalEnableBranchPrediction(void)
{
  UINT32 r = 0;
  UINT32 BranchPrediction = BRANCH_PREDICTION_ENABLE;
  __asm__ volatile(
    "    MRC     p15, 0, %0, c1, c0, 0 \n"
    "    ORR     %0, %1, %2 \n"
    "    MCR     p15, 0, %1, c1, c0, 0 \n"
    "    NOP \n"
    "    NOP \n"
    "    NOP \n"
    "    NOP \n"
    "    NOP \n"
    : "=r" (r)
    : "0" (r), "r"(BranchPrediction)
  );
}
EXPORT_SYMBOL(HalEnableBranchPrediction);


/*----------------------------------------------------------------------------
 * HalDisableBranchPrediction() Disable Branch Prediction
 *---------------------------------------------------------------------------*/
void HalDisableBranchPrediction(void)
{
  UINT32 r = 0;
  UINT32 BranchPrediction = BRANCH_PREDICTION_ENABLE;

  __asm__ volatile(
    "    MRC     p15, 0, %0, c1, c0, 0 \n"
    "    BIC     %0, %1, %2 \n"
    "    MCR     p15, 0, %1, c1, c0, 0 \n"
    : "=r" (r)
    : "0" (r), "r"(BranchPrediction)
  );
}
EXPORT_SYMBOL(HalDisableBranchPrediction);

//============================================================================
// Performance Monitor Control Register related functions
//============================================================================
// Performance Monitor Control Register
UINT32 _u4HalGetPerfMonitorControl(void)
{
  UINT32 r = 0;

  __asm__ volatile(
    "    MRC p15, 0, %0, c15, c12, 0 \n"
    : "=r" (r)
  );

  return r;
}

void _vHalSetPerfMonitorControl(UINT32 u4Val)
{
  __asm__ volatile(
    "    MCR p15, 0, %0, c15, c12, 0 \n"
    :
    : "r" (u4Val)
  );
}

// Count Register 0
UINT32 u4HalGetCountRegister0(void)
{
  UINT32 r = 0;

  __asm__ volatile(
    "    MRC p15, 0, %0, c15, c12, 2 \n"
    : "=r" (r)
  );

  return r;
}
EXPORT_SYMBOL(u4HalGetCountRegister0);

void vHalSetCountRegister0(UINT32 u4Val)
{
  __asm__ volatile(
    "    MCR p15, 0, %0, c15, c12, 2 \n"
    :
    : "r" (u4Val)
  );
}
EXPORT_SYMBOL(vHalSetCountRegister0);

BOOL fgHalCount0OverFlow(void)
{
  if(_u4HalGetPerfMonitorControl() & PMC_CR_0)
  {
    return TRUE;
  }
  return FALSE;
}
EXPORT_SYMBOL(fgHalCount0OverFlow);

// Count Register 1
UINT32 u4HalGetCountRegister1(void)
{
  UINT32 r = 0;

  __asm__ volatile(
    "    MRC p15, 0, %0, c15, c12, 3 \n"
    : "=r" (r)
  );

  return r;
}
EXPORT_SYMBOL(u4HalGetCountRegister1);

void vHalSetCountRegister1(UINT32 u4Val)
{
  __asm__ volatile(
    "    MCR p15, 0, %0, c15, c12, 3 \n"
    :
    : "r" (u4Val)
  );
}
EXPORT_SYMBOL(vHalSetCountRegister1);

BOOL fgHalCount1OverFlow(void)
{
  if(_u4HalGetPerfMonitorControl() & PMC_CR_1)
  {
    return TRUE;
  }
  return FALSE;
}
EXPORT_SYMBOL(fgHalCount1OverFlow);

// Performance Monitor Register Operation
void vHalCounterStart(void)
{
  UINT32 u4PMC;
  u4PMC = _u4HalGetPerfMonitorControl();
  u4PMC = u4PMC | PCM_E_ENABLE;
  _vHalSetPerfMonitorControl(u4PMC);
}
EXPORT_SYMBOL(vHalCounterStart);

void vHalCounterStop(void)
{
  UINT32 u4PMC;
  u4PMC = _u4HalGetPerfMonitorControl();
  u4PMC = u4PMC & (~PCM_E_ENABLE);
  _vHalSetPerfMonitorControl(u4PMC);
}
EXPORT_SYMBOL(vHalCounterStop);

void vHalCounterReset(void)
{
  UINT32 u4PMC;
  u4PMC = _u4HalGetPerfMonitorControl();
  u4PMC = u4PMC | PCM_P_RESET;
  _vHalSetPerfMonitorControl(u4PMC);
}
EXPORT_SYMBOL(vHalCounterReset);

// Data Cache Hit / Miss Rate Test
void vHalDCacheHitRateStart(void)
{
  UINT32 u4PMC;

  //u4PMC = _u4HalGetPerfMonitorControl();
  u4PMC = 0x0;
  u4PMC = u4PMC | (EVT_NUM_DCACHE_MISS << PMC_EVT_COUNT_0)      // counter0: dcache miss
  	        | (EVT_NUM_DCACHE_ACCESS << PMC_EVT_COUNT_1);   // counter1: dcache access
  _vHalSetPerfMonitorControl(u4PMC);

  vHalCounterReset();
  vHalCounterStart();
}
EXPORT_SYMBOL(vHalDCacheHitRateStart);

void vHalICacheHitRateStart(void)
{
  UINT32 u4PMC;

  //u4PMC = _u4HalGetPerfMonitorControl();
  u4PMC = 0x0;
  u4PMC = u4PMC | (EVT_NUM_ICACHE_MISS << PMC_EVT_COUNT_0)      // counter0: icache miss
  	        | (EVT_NUM_INSTRU_EXECED << PMC_EVT_COUNT_1);   // counter1: icache access
  _vHalSetPerfMonitorControl(u4PMC);

  vHalCounterReset();
  vHalCounterStart();
}
EXPORT_SYMBOL(vHalICacheHitRateStart);

//============================================================================
// MMU related functions
//============================================================================


/*----------------------------------------------------------------------------
 * HalInvalidateTLB() Invalidate entire TLB
 *---------------------------------------------------------------------------*/
void HalInvalidateTLB(void)
{
  UINT32 r = 0;

  __asm__ volatile(
    "    MCR     p15, 0, %0, c8, c7, 0 \n"
    :
    : "r" (r)
  );
}
EXPORT_SYMBOL(HalInvalidateTLB);

/*----------------------------------------------------------------------------
 * HalGetTTB0() to get TTB0 for user mode
 *---------------------------------------------------------------------------*/
UINT32 u4HalGetTTB0(void)
{
  UINT32 r = 0;

  __asm__ volatile(
    "    MRC p15, 0, %0, c2, c0, 0 \n"
    : "=r" (r)
  );

  r = r & 0xFFFFC000;

  return r;
}
EXPORT_SYMBOL(u4HalGetTTB0);

/*----------------------------------------------------------------------------
 * HalGetTTB1() to get TTB1 for kernal mode
 *---------------------------------------------------------------------------*/
UINT32 u4HalGetTTB1(void)
{
  UINT32 r = 0;

  __asm__ volatile(
    "    MRC p15, 0, %0, c2, c0, 1 \n"
    : "=r" (r)
  );

  r = r & 0xFFFFC000;

  return r;
}
EXPORT_SYMBOL(u4HalGetTTB1);

/*----------------------------------------------------------------------------
 * HalGetTTB() to get TTB
 *---------------------------------------------------------------------------*/
UINT32 u4HalGetTTB(UINT32 ui4_mem_ptr)
{
    UINT32 r = 0;

    if( 0xC0000000 > ui4_mem_ptr )
    {   //user mode
      __asm__ volatile(
        "    MRC p15, 0, %0, c2, c0, 1 \n"
        : "=r" (r)
      );
    }
    else
    {   //kernal mode
      __asm__ volatile(
        "    MRC p15, 0, %0, c2, c0, 1 \n"
        : "=r" (r)
      );
    }

    r = r & 0xFFFFC000;

    return r;
}
EXPORT_SYMBOL(u4HalGetTTB);

/*----------------------------------------------------------------------------
 * BYTESWAP32() Swap bytes in a word (32-bit)
 *  @param u4Value[in] - The word to be swapped
 *  @return      : The swapped word
 *---------------------------------------------------------------------------*/
UINT32 BYTESWAP32(UINT32 u4Value)
{
  //lint --e{*} Don't lint this function
  UINT32 r1 = 0;

  // r0 = a  , b  , c  , d
  // r1 = a^c, b^d, c^a, d^b
  // r1 = a^c, 0  , c^a, d^b 
  // r0 = d  , a  , b  , c
  // r0 = d  , c  , b  , a
  __asm__ volatile(
    "    EOR     %0, %3, %3, ROR #16 \n"
    "    BIC     %0, %2, #0xff0000 \n"
    "    MOV     %1, %3, ROR #8 \n"
    "    EOR     %1, %3, %2, LSR #8 \n"
    : "=r" (r1), "=r" (u4Value)
    : "0" (r1), "1" (u4Value)
  );
  return u4Value;
}
EXPORT_SYMBOL(BYTESWAP32);

//---------------------------------------------------------------------
// Function    : HalGetICacheLineSize
// Description : Get I-cache line size
// Parameter   : None
// Return      : The size in byte
//---------------------------------------------------------------------
UINT32 HalGetICacheLineSize(void)
{
  return ICACHE_LINE_SIZE;
}
EXPORT_SYMBOL(HalGetICacheLineSize);

//---------------------------------------------------------------------
// Function    : HalGetDCacheLineSize
// Description : Get D-cache line size
// Parameter   : None
// Return      : The size in byte
//---------------------------------------------------------------------
UINT32 HalGetDCacheLineSize(void)
{
  return DCACHE_LINE_SIZE;
}
EXPORT_SYMBOL(HalGetDCacheLineSize);

BOOL HalITCMEnable(UINT32 u4BaseAddr)
{
  UINT32 r0 = 0, r1 = 0, r2 = 0xFFF;
  UINT32 u4Address = 0;

  if((u4BaseAddr & ITCM_ALIGNMENT) != 0)
  {
    return FALSE;
  }

  //AC83XX ITCM = 16KB, it need set two ITCM section, we put them together

  u4Address = u4BaseAddr;

  r0 = 0x0;
  __asm__ volatile(
    //Set section 0
    "    MCR p15, 0, %0, c9, c2, 0 \n"
    :
    : "r" (r0)
  );

  r0 = 0x0;
  r1 = u4Address;
  __asm__ volatile(
    //Set Address
    "    MRC p15, 0, %0, c9, c1, 1 \n"
    "    AND %0, %2, %4 \n"
    "    ORR %0, %2, %3 \n"
    "    MCR p15, 0, %2, c9, c1, 1 \n"
    : "=r" (r0), "=r" (r1)
    : "0" (r0), "1" (r1), "r" (r2)
  );

  r0 = 0x0;
  r1 = 0x1;
  __asm__ volatile(
    //Enable ITCM
    "    MRC p15, 0, %0, c9, c1, 1 \n"
    "    ORR %0, %2, %3 \n"
    "    MCR p15, 0, %2, c9, c1, 1 \n"
    : "=r" (r0), "=r" (r1)
    : "0" (r0), "1" (r1)
  );

  u4Address = u4Address + ITCM_SIZE/2;

  r0 = 0x1;
  __asm__ volatile(
    //Set section 1
    "    MCR p15, 0, %0, c9, c2, 0 \n"
    :
    : "r" (r0)
  );

  r0 = 0x0;
  r1 = u4Address;
  __asm__ volatile(
    //Set Address
    "    MRC p15, 0, %0, c9, c1, 1 \n"
    "    AND %0, %2, %4 \n"
    "    ORR %0, %2, %3 \n"
    "    MCR p15, 0, %2, c9, c1, 1 \n"
    : "=r" (r0), "=r" (r1)
    : "0" (r0), "1" (r1), "r" (r2)
  );

  r0 = 0x0;
  r1 = 0x1;
  __asm__ volatile(
    //Enable ITCM
    "    MRC p15, 0, %0, c9, c1, 1 \n"
    "    ORR %0, %2, %3 \n"
    "    MCR p15, 0, %2, c9, c1, 1 \n"
    : "=r" (r0), "=r" (r1)
    : "0" (r0), "1" (r1)
  );

  return TRUE;
}
EXPORT_SYMBOL(HalITCMEnable);

BOOL HalITCMEnableEx(UINT32 u4Number, UINT32 u4Secure, UINT32 u4BaseAddr)
{
  UINT32 r0 = 0, r1 = 0, r2 =0xFFF;
  UINT32 u4Address = 0;

  if((u4BaseAddr & ITCM_ALIGNMENT) != 0 || u4Number > 2)
  {
    return FALSE;
  }

  u4Address = u4BaseAddr;

  r0 = u4Number;
  __asm__ volatile(
    //Set section
    "    MCR p15, 0, %0, c9, c2, 0 \n"
    :
    : "r" (r0)
  );

  if(u4Secure == 0)
  {
    //Non-secure TCM
    r0 = 1;
  }
  else
  {
    //Secure TCM
    r0 = 0;
  }

  __asm__ volatile(
    //Set NS Bit
    "    MCR p15, 0, %0, c9, c1, 3 \n"
    :
    : "r" (r0)
  );

  r0 = 0;
  r1 = u4Address;
  __asm__ volatile(
    //Set Address
    "    MRC p15, 0, %0, c9, c1, 1 \n"
    "    AND %0, %2, %4 \n"
    "    ORR %0, %2, %3 \n"
    "    MCR p15, 0, %2, c9, c1, 1 \n"
    : "=r" (r0), "=r" (r1)
    : "0" (r0), "1" (r1), "r" (r2)
  );

  r0 = 0x0;
  r1 = 0x1;
  __asm__ volatile(
    //Enable DTCM
    "    MRC p15, 0, %0, c9, c1, 1 \n"
    "    ORR %0, %2, %3 \n"
    "    MCR p15, 0, %2, c9, c1, 1 \n"
    : "=r" (r0), "=r" (r1)
    : "0" (r0), "1" (r1)
  );

  return TRUE;
}
EXPORT_SYMBOL(HalITCMEnableEx);

BOOL HalITCMDisable(void)
{
  UINT32 r0 = 0, r1 = 0;

  //AC83XX ITCM = 16KB, it need set two ITCM section, we put them together

  r0 = 0x0;
  __asm__ volatile(
    //Set section 0
    "    MCR p15, 0, %0, c9, c2, 0 \n"
    :
    : "r" (r0)
  );

  r0 = 0x0;
  r1 = 0xFFFFFFFE;
  __asm__ volatile(
    //Dsiable ITCM
    "    MRC p15, 0, %0, c9, c1, 1 \n"
    "    AND %0, %2, %3 \n"
    "    MCR p15, 0, %2, c9, c1, 1 \n"
    : "=r" (r0), "=r" (r1)
    : "0" (r0), "1" (r1)
  );

  r0 = 0x1;
  __asm__ volatile(
    //Set section 1
    "    MCR p15, 0, %0, c9, c2, 0 \n"
    :
    : "r" (r0)
  );

  r0 = 0x0;
  r1 = 0xFFFFFFFE;
  __asm__ volatile(
    //Dsiable ITCM
    "    MRC p15, 0, %0, c9, c1, 1 \n"
    "    AND %0, %2, %3 \n"
    "    MCR p15, 0, %2, c9, c1, 1 \n"
    : "=r" (r0), "=r" (r1)
    : "0" (r0), "1" (r1)
  );

  return TRUE;
}
EXPORT_SYMBOL(HalITCMDisable);

BOOL HalITCMDisableEx(UINT32 u4Number)
{
  UINT32 r0 = 0, r1 = 0;

  r0 = u4Number;
  __asm__ volatile(
    //Set section 0
    "    MCR p15, 0, %0, c9, c2, 0 \n"
    :
    : "r" (r0)
  );

  r0 = 0x0;
  r1 = 0xFFFFFFFE;
  __asm__ volatile(
    //Dsiable DTCM
    "    MRC p15, 0, %0, c9, c1, 1 \n"
    "    AND %0, %2, %3 \n"
    "    MCR p15, 0, %2, c9, c1, 1 \n"
    : "=r" (r0), "=r" (r1)
    : "0" (r0), "1" (r1)
  );

  return TRUE;
}
EXPORT_SYMBOL(HalITCMDisableEx);

BOOL HalDTCMEnable(UINT32 u4BaseAddr)
{
  UINT32 r0 = 0, r1 = 0, r2=0xFFF;
  UINT32 u4Address = 0;

  if((u4BaseAddr & DTCM_ALIGNMENT) != 0)
  {
    return FALSE;
  }

  //AC83XX DTCM = 16KB, it need set two DTCM section, we put them together

  u4Address = u4BaseAddr;

  r0 = 0x0;
  __asm__ volatile(
    //Set section 0
    "    MCR p15, 0, %0, c9, c2, 0 \n"
    :
    : "r" (r0)
  );

  r0 = 0x0;
  r1 = u4Address;
  __asm__ volatile(
    //Set Address
    "    MRC p15, 0, %0, c9, c1, 0 \n"
    "    AND %0, %2, %4 \n"
    "    ORR %0, %2, %3 \n"
    "    MCR p15, 0, %2, c9, c1, 0 \n"
    : "=r" (r0), "=r" (r1)
    : "0" (r0), "1" (r1), "r" (r2)
  );

  r0 = 0x0;
  r1 = 0x1;
  __asm__ volatile(
    //Enable ITCM
    "    MRC p15, 0, %0, c9, c1, 0 \n"
    "    ORR %0, %2, %3 \n"
    "    MCR p15, 0, %2, c9, c1, 0 \n"
    : "=r" (r0), "=r" (r1)
    : "0" (r0), "1" (r1)
  );

  u4Address = u4Address + DTCM_SIZE/2;

  r0 = 0x1;
  __asm__ volatile(
    //Set section 1
    "    MCR p15, 0, %0, c9, c2, 0 \n"
    :
    : "r" (r0)
  );

  r0 = 0x0;
  r1 = u4Address;
  __asm__ volatile(
    //Set Address
    "    MRC p15, 0, %0, c9, c1, 0 \n"
    "    AND %0, %2, %4 \n"
    "    ORR %0, %2, %3 \n"
    "    MCR p15, 0, %2, c9, c1, 0 \n"
    : "=r" (r0), "=r" (r1)
    : "0" (r0), "1" (r1), "r" (r2)
  );

  r0 = 0x0;
  r1 = 0x1;
  __asm__ volatile(
    //Enable ITCM
    "    MRC p15, 0, %0, c9, c1, 0 \n"
    "    ORR %0, %2, %3 \n"
    "    MCR p15, 0, %2, c9, c1, 0 \n"
    : "=r" (r0), "=r" (r1)
    : "0" (r0), "1" (r1)
  );

  return TRUE;
}
EXPORT_SYMBOL(HalDTCMEnable);

BOOL HalDTCMEnableEx(UINT32 u4Number, UINT32 u4Secure, UINT32 u4BaseAddr)
{
  UINT32 r0 = 0, r1 = 0, r2=0xFFF;
  UINT32 u4Address = 0;

  if((u4BaseAddr & DTCM_ALIGNMENT) != 0 || u4Number > 2)
  {
    return FALSE;
  }

  u4Address = u4BaseAddr;

  r0 = u4Number;
  __asm__ volatile(
    //Set section
    "    MCR p15, 0, %0, c9, c2, 0 \n"
    :
    : "r" (r0)
  );

  if(u4Secure == 0)
  {
    //Non-secure TCM
    r0 = 1;
  }
  else
  {
    //Secure TCM
    r0 = 0;
  }

  __asm__ volatile(
    //Set NS Bit
    "    MCR p15, 0, %0, c9, c1, 2 \n"
    :
    : "r" (r0)
  );

  r0 = 0;
  r1 = u4Address;
  __asm__ volatile(
    //Set Address
    "    MRC p15, 0, %0, c9, c1, 0 \n"
    "    AND %0, %2, %4 \n"
    "    ORR %0, %2, %3 \n"
    "    MCR p15, 0, %2, c9, c1, 0 \n"
    : "=r" (r0), "=r" (r1)
    : "0" (r0), "1" (r1), "r" (r2)
  );

  r0 = 0x0;
  r1 = 0x1;
  __asm__ volatile(
    //Enable DTCM
    "    MRC p15, 0, %0, c9, c1, 0 \n"
    "    ORR %0, %2, %3 \n"
    "    MCR p15, 0, %2, c9, c1, 0 \n"
    : "=r" (r0), "=r" (r1)
    : "0" (r0), "1" (r1)
  );

  return TRUE;
}
EXPORT_SYMBOL(HalDTCMEnableEx);

BOOL HalDTCMDisable(void)
{
  UINT32 r0 = 0, r1 = 0;

  //AC83XX DTCM = 16KB, it need set two DTCM section, we put them together

  r0 = 0x0;
  __asm__ volatile(
    //Set section 0
    "    MCR p15, 0, %0, c9, c2, 0 \n"
    :
    : "r" (r0)
  );

  r0 = 0x0;
  r1 = 0xFFFFFFFE;
  __asm__ volatile(
    //Dsiable ITCM
    "    MRC p15, 0, %0, c9, c1, 0 \n"
    "    AND %0, %2, %3 \n"
    "    MCR p15, 0, %2, c9, c1, 0 \n"
    : "=r" (r0), "=r" (r1)
    : "0" (r0), "1" (r1)
  );

  r0 = 0x1;
  __asm__ volatile(
    //Set section 1
    "    MCR p15, 0, %0, c9, c2, 0 \n"
    :
    : "r" (r0)
  );

  r0 = 0x0;
  r1 = 0xFFFFFFFE;
  __asm__ volatile(
    //Dsiable ITCM
    "    MRC p15, 0, %0, c9, c1, 0 \n"
    "    AND %0, %2, %3 \n"
    "    MCR p15, 0, %2, c9, c1, 0 \n"
    : "=r" (r0), "=r" (r1)
    : "0" (r0), "1" (r1)
  );

  return TRUE;
}
EXPORT_SYMBOL(HalDTCMDisable);

BOOL HalDTCMDisableEx(UINT32 u4Number)
{
  UINT32 r0 = 0, r1 = 0;

  r0 = u4Number;
  __asm__ volatile(
    //Set section 0
    "    MCR p15, 0, %0, c9, c2, 0 \n"
    :
    : "r" (r0)
  );

  r0 = 0x0;
  r1 = 0xFFFFFFFE;
  __asm__ volatile(
    //Dsiable DTCM
    "    MRC p15, 0, %0, c9, c1, 0 \n"
    "    AND %0, %2, %3 \n"
    "    MCR p15, 0, %2, c9, c1, 0 \n"
    : "=r" (r0), "=r" (r1)
    : "0" (r0), "1" (r1)
  );

  return TRUE;
}
EXPORT_SYMBOL(HalDTCMDisableEx);

BOOL HalDTCMDMA(UINT32 u4TCMBaseAddr, UINT32 u4DRAMBaseAddr, UINT32 u4Size, UINT32 u4Direction)
{
  UINT32 r0 = 0;

  r0 = TCM_DMA_ENABLE;

  __asm__ volatile(
    //Clear First
    "    MCR p15, 0, %0, c11, c3, 2 \n"
    :
    : "r" (r0)
  );

  r0 = u4TCMBaseAddr;
  __asm__ volatile(
    //Set DTCM Start Address
    "    MCR p15, 0, %0, c11, c5, 0 \n"
    :
    : "r" (r0)
  );

  r0 = u4TCMBaseAddr + u4Size;
  __asm__ volatile(
    //Set DTCM End Address
    "    MCR p15, 0, %0, c11, c7, 0 \n"
    :
    : "r" (r0)
  );

  r0 = u4DRAMBaseAddr;
  __asm__ volatile(
    //Set Memory Start Address
    "    MCR p15, 0, %0, c11, c6, 0 \n"
    :
    : "r" (r0)
  );

  r0 = 0x0;

  __asm__ volatile(
    //Read DMA Control
    "    MRC p15, 0, %0, c11, c4, 0 \n"
    : "=r" (r0)
  );

  //Set D-TCM
  r0 = r0 & (~TCM_DMA_CONTROL_TR);

  //Set Direction
  if(u4Direction == TCM_DMA_DIRECTION_TCM_TO_DRAM)
  {
    r0 = r0 | TCM_DMA_CONTROL_DT;
  }
  else
  {
    r0 = r0 & (~TCM_DMA_CONTROL_DT);
  }

  //Set Increment : 4Bytes a time, Bit[19:8] = 4
  r0 = r0 & (~TCM_DMA_CONTROL_ST_MASK);
  r0 = r0 | (0x4<<TCM_DMA_CONTROL_ST_OFFSET);

  //Set Size : 4Bytes a time, Bit[1:0] = 2
  r0 = r0 & (~TCM_DMA_CONTROL_TS_MASK);
  r0 = r0 | (0x2<<TCM_DMA_CONTROL_TS_OFFSET);

  __asm__ volatile(
    //Write DMA Control
    "    MCR p15, 0, %0, c11, c4, 0 \n"
    :
    : "r" (r0)
  );

  r0 = TCM_DMA_ENABLE;

  __asm__ volatile(
    //Enable DMA
    "    MCR p15, 0, %0, c11, c3, 1 \n"
    :
    : "r" (r0)
  );

  return TRUE;
}
EXPORT_SYMBOL(HalDTCMDMA);

LINT_SUPPRESS_NEXT_EXPRESSION(129)
INLINE UINT16 BYTESWAP16(UINT16 u2Value)
{
	return (UINT16)((u2Value >> 8) | (u2Value << 8));
}
EXPORT_SYMBOL(BYTESWAP16);

//---------------------------------------------------------------------
void ConfigPerformanceMonitorControlReg(UINT32 value)
{
  UINT32 r0;

  r0 = value;
  __asm__ volatile(
    //Set Memory Start Address
    "    MCR p15, 0, %0, c15, c12, 0 \n"
    :
    : "r" (r0)
  );
}
EXPORT_SYMBOL(ConfigPerformanceMonitorControlReg);


UINT32 ReadARMCycleCounter(void)
{
  UINT32 r0;

  r0 = 0x0;

  __asm__ volatile(
    "    MRC p15, 0, %0, c15, c12, 1 \n"
    : "=r" (r0)
  );

  return (r0);
}
EXPORT_SYMBOL(ReadARMCycleCounter);


BOOL IsARMCycleCounterOverflow(void)
{
  UINT32 r0;
  UINT32 overflow_bit;

  __asm__ volatile(
    "    MRC p15, 0, %0, c15, c12, 0 \n"
    : "=r" (r0)
  );

  overflow_bit = (r0>>10) & 0x0001;
  return ((overflow_bit == 1));

}
EXPORT_SYMBOL(IsARMCycleCounterOverflow);

