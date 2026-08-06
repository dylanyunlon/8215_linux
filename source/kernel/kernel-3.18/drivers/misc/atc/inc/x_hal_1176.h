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


#ifndef X_HAL_1176_H
#define X_HAL_1176_H

#include "x_typedef.h"
#include "x_lint.h"

//============================================================================
// IRQ/FIQ related functions
//============================================================================
externC void HalEnableIRQ(void);
externC void HalDisableIRQ(void);
externC void HalEnableFIQ(void);
externC void HalDisableFIQ(void);
externC UINT32 HalCriticalStart(void);
externC UINT32 HalCriticalSemiStart(void);
externC void HalCriticalEnd(UINT32 u4Flags);

//============================================================================
// Cache related functions
//============================================================================
externC BOOL HalIsICacheEnabled(void);
externC BOOL HalIsDCacheEnabled(void);

externC UINT32 HalGetICacheSize(void);
externC UINT32 HalGetDCacheSize(void);
externC UINT32 HalGetICacheLineSize(void);
externC UINT32 HalGetDCacheLineSize(void);

externC void HalEnableICache(void);
externC void HalDisableICache(void);

externC void HalEnableDCache(void);
externC void HalDisableDCache(void);

externC void HalEnableCaches(void);
externC void HalDisableCaches(void);

externC void HalInvalidateAllCaches(void);
externC void HalInvalidateICache(void);
externC void HalInvalidateDCacheCmd(void);
externC void HalInvalidateDCache(void);

externC void HalFlushDCacheCmd(void);
externC void HalFlushDCache(void);
externC void HalFlushInvalidateDCache(void);
externC void HalFlushWriteBuffer(void);
externC void HalInstructionMemoryBarrier(void);

/*
 * BSP_FlushDCacheRange(UINT32 u4Start, UINT32 u4End)
 *
 *    Clean and Invalidate Data Cache Range
 *    - u4Start : virtual start address (inclusive)
 *    - u4Len : length
 */
externC void BSP_FlushDCacheRange(UINT32 u4Start, UINT32 u4Len);


//============================================================================
// Branch Prediction related functions
//============================================================================
externC BOOL HalIsBranchPredictionEnabled(void);
externC void HalEnableBranchPrediction(void);
externC void HalDisableBranchPrediction(void);

//============================================================================
// Performance Monitor Register Operation
//============================================================================
externC void vHalCounterStart(void);
externC void vHalCounterStop(void);
externC void vHalCounterReset(void);

//============================================================================
// Cache Hit / Miss Rate Test
//============================================================================
externC void vHalDCacheHitRateStart(void);
externC void vHalICacheHitRateStart(void);
// Count Register 0
externC UINT32 u4HalGetCountRegister0(void);
externC void vHalSetCountRegister0(UINT32 u4Val);
externC BOOL fgHalCount0OverFlow(void);
// Count Register 1
externC UINT32 u4HalGetCountRegister1(void);
externC void vHalSetCountRegister1(UINT32 u4Val);
externC BOOL fgHalCount1OverFlow(void);

//============================================================================
// MMU related functions
//============================================================================
externC BOOL HalIsMMUEnabled(void);
externC void HalEnableMMU(void);
externC void HalDisableMMU(void);
externC void HalInvalidateTLB(void);
externC BOOL HalInitMMU(UINT32 u4Addr);
externC BOOL HalInitMMU_Protect_RO(UINT32 u4SectionAddr, UINT32 u4SmallAddr);
//============================================================================
// TCM mode
//============================================================================
externC BOOL HalITCMEnable(UINT32 u4BaseAddr);
externC BOOL HalITCMDisable(void);

externC BOOL HalDTCMEnable(UINT32 u4BaseAddr);
externC BOOL HalDTCMDisable(void);

externC BOOL HalITCMEnableEx(UINT32 u4Number, UINT32 u4Secure, UINT32 u4BaseAddr);
externC BOOL HalITCMDisableEx(UINT32 u4Number);

externC BOOL HalDTCMEnableEx(UINT32 u4Number, UINT32 u4Secure, UINT32 u4BaseAddr);
externC BOOL HalDTCMDisableEx(UINT32 u4Number);

externC BOOL HalDTCMDMA(UINT32 u4TCMBaseAddr, UINT32 u4DRAMBaseAddr, UINT32 u4Size, UINT32 u4Direction);
externC BOOL HalDTCMDMADone(void);

#define ITCM_SIZE       0x4000
#define DTCM_SIZE       0x4000

#define TCM_DMA_DIRECTION_TCM_TO_DRAM    0
#define TCM_DMA_DIRECTION_DRAM_TO_TCM    1

//============================================================================
// Sleep mode
//============================================================================
externC void HalWaitForInterrupt(void);

//============================================================================
// debug
//============================================================================
typedef struct
{
  UINT32              u4Addr;
  UINT32              u4Enable;
} ARM1176_HWBP_T;

typedef struct
{
  UINT32              u4Addr;
  UINT32              u4RWHalt;
  UINT32              u4ByteHalt;
  UINT32              u4Enable;
} ARM1176_WP_T;

#define READ_HALT			1
#define WRITE_HALT			2
#define READ_WRITE_HALT	3
#define BYTE0_HALT			(1<<0)
#define BYTE1_HALT			(2<<1)
#define BYTE2_HALT			(3<<2)
#define BYTE3_HALT			(4<<3)

externC void HalShowPoint(void);
externC void HalSetHWBreakPoint(UINT32 u4ID, ARM1176_HWBP_T *ptBP);
externC void HalSetWatchPoint(UINT32 u4ID, ARM1176_WP_T *ptWP);
externC BOOL HalIsDebugMode(void);
externC void HalBreakPoint(void);

//============================================================================
// Endian related
//============================================================================
UINT32 BYTESWAP32(UINT32 u4Value);
extern LINT_SUPPRESS_NEXT_EXPRESSION(129)
INLINE UINT16 BYTESWAP16(UINT16 u2Value);



// HTONL: Convert an UINT32 from host byte order to network byte order
// NTOHL: Convert an UINT32 from network byte order to host byte order
// HTONS: Convert an UINT16 from host byte order to network byte order
// NTOHS: Convert an UINT16 from network byte order to host byte order

#ifdef __BIG_ENDIAN

#define BIG_ENDIAN
#define HTONL(x)	(x)
#define NTOHL(x)	(x)
#define HTONS(x)	(x)
#define NTOHS(x)	(x)

#else	// __BIG_ENDIAN

#define LITTLE_ENDIAN
#define HTONL(x)	BYTESWAP32((x))
#define NTOHL(x)	BYTESWAP32((x))
#define HTONS(x)	BYTESWAP16((x))
#define NTOHS(x)	BYTESWAP16((x))

#endif	// __BIG_ENDIAN

#endif  // X_HAL_1176_H
