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

/******************************************************************************
*[File]                     aud_comm_reg_rw.h
*[Version]                  v1.0
*[Revision Date]            2014-03-10
*[Author]                   tongfa.luo@autochips.com 
*[Description]
*        
******************************************************************************/
#ifndef _AUDIO_COMM_REG_RW_H_
#define _AUDIO_COMM_REG_RW_H_


#if CONFIG_DRV_AUD_AC83XX

#include "x_ckgen.h"

#endif

#if CONFIG_DRV_AUD_AC83XX
#include <mach/base_regs.h>
#endif
#include "x_ioopt.h"

#include "aud_comm_log.h"

#if CONFIG_DRV_AUD_AC83XX

#define AUDREG_WRITE(addr, val)                (*((volatile u32*)(IO_BASE_VA + addr)) = (val))
#define AUDREG_READ(addr)                      (*((volatile u32*)(IO_BASE_VA + addr)))

#else
extern unsigned long io_v_base;
extern unsigned long io_base_ckgen;
#define AUDREG_WRITE(addr, val)                writel(val, io_v_base+addr)//(*((volatile u32*)(io_v_base + addr)) = (val))
#define AUDREG_READ(addr)                      readl(io_v_base + addr) //(*((volatile u32*)(io_v_base + addr)))
#endif
#define AUDREG_MASK(start, bitNum)             (((1L << (bitNum)) - 1) << (start))


#define AUDREG_BITS_VAL(val, start, num)                           \
    ((val & AUDREG_MASK(start, num)) >> (start))

#define AUDREG_BITS_W(addr, start, bitNum, val)                \
    AUDREG_WRITE(addr, (AUDREG_READ(addr) & ~(AUDREG_MASK(start, bitNum))) | ((val) << start))

#define AUDREG_BITS_R(addr, start, bitNum)                      \
    ((AUDREG_READ(addr) & AUDREG_MASK(start, bitNum)) >> start)

//================================================================//

#if CONFIG_DRV_AUD_AC83XX

#define AUD_CKGEN_SETBITS(offset, dBit)        CKGEN_WRITE32(offset, CKGEN_READ32(offset) | (dBit))
#define AUD_CKGEN_CLRBITS(offset, dBit)        CKGEN_WRITE32(offset, CKGEN_READ32(offset) & (~(dBit)))

#define AUD_CKGEN_WRITE32_MASK(offset, val, mask)                       \
    CKGEN_WRITE32(offset, (CKGEN_READ32(offset) & (~(mask))) | (val))

#define AUD_CKGEN_BITS_W(addr, start, bitNum, val)                  \
    CKGEN_WRITE32(addr, (CKGEN_READ32(addr) & ~(AUDREG_MASK(start, bitNum))) | ((val) << start))

#else;

#define AUD_CKGEN_SETBITS(offset, dBit)         writel(readl(io_v_base+offset) | (dBit), io_v_base+offset)
#define AUD_CKGEN_CLRBITS(offset, dBit)         writel(readl(io_v_base+offset) & (~(dBit)), io_v_base+offset)

#define AUD_CKGEN_WRITE32_MASK(offset, val, mask)                       \
    writel(readl(io_v_base+offset) & (~(mask)) | (val), io_v_base+offset)

#define AUD_CKGEN_BITS_W(addr, start, bitNum, val)                  \
    writel(readl(io_v_base+addr) & ~(AUD_REG_MASK(start, bitNum)) | ((val) << start), io_v_base+addr)
#endif
//================================================================//

#define AUDREG_READ_LOG(addr, len)                                          \
    {                                                                       \
        u32 i;                                                           \
        AUDLOG_NO_PREFIX(1, (T("[Read Reg] Start: ")));                     \
        for (i = 0; i < len; i++) {                                         \
             if (i % 4 == 0) {                                              \
                AUDLOG_NO_PREFIX(1, (T("\n0x%6x : "), (u32)(addr+4*i)));               \
             }                                                              \
             AUDLOG_NO_PREFIX(1, (T("0x%8x "), (u32)AUDREG_READ(addr+4*i)));   \
        }                                                                   \
        AUDLOG_NO_PREFIX(1, (T("\n[Read Reg] End! \n")));                   \
    }


#if CONFIG_DRV_AUD_AC83XX
#define AUDREG_WRITE_LOG(addr, val)                                                             \
        AUDLOG_NO_PREFIX(1, (T("[Write Reg] 0x%6x: 0x%x -> "), (u32)addr, (u32)AUDREG_READ(addr)));      \
        AUDREG_WRITE(addr,val);                                                                 \
        AUDLOG_NO_PREFIX(1, (T("0x%x  \n"), (u32)AUDREG_READ(addr)));     \
        CKGEN_WRITE32(addr, val);

#else
#define AUDREG_WRITE_LOG(addr, val)                                                             \
        AUDLOG_NO_PREFIX(1, (T("[Write Reg] 0x%6x: 0x%x -> "), (u32)addr, (u32)AUDREG_READ(addr)));      \
        AUDREG_WRITE(addr,val);                                                                 \
        AUDLOG_NO_PREFIX(1, (T("0x%x  \n"), (u32)AUDREG_READ(addr)));     \
        writel(val, io_v_base+addr);

#endif


#endif // #ifndef _AUDIO_COMM_REG_RW_H_
