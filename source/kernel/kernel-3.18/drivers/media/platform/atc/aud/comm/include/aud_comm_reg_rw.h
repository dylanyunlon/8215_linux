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

#include "x_ckgen.h"
#include <mach/base_regs.h>
#include "aud_comm_log.h"


#define AUDREG_WRITE(addr, val)                (*((volatile u32*)(IO_BASE_VA + addr)) = (val))
#define AUDREG_READ(addr)                      (*((volatile u32*)(IO_BASE_VA + addr)))

#define AUDREG_MASK(start, bitNum)             (((1 << (bitNum)) - 1) << (start))


#define AUDREG_BITS_VAL(val, start, num)                           \
    ((val & AUDREG_MASK(start, num)) >> (start))

#define AUDREG_BITS_W(addr, start, bitNum, val)                \
    AUDREG_WRITE(addr, (AUDREG_READ(addr) & ~(AUDREG_MASK(start, bitNum))) | ((val) << start))

#define AUDREG_BITS_R(addr, start, bitNum)                      \
    ((AUDREG_READ(addr) & AUDREG_MASK(start, bitNum)) >> start)

//================================================================//


#define AUD_CKGEN_SETBITS(offset, dBit)        CKGEN_WRITE32(offset, CKGEN_READ32(offset) | (dBit))
#define AUD_CKGEN_CLRBITS(offset, dBit)        CKGEN_WRITE32(offset, CKGEN_READ32(offset) & (~(dBit)))

#define AUD_CKGEN_WRITE32_MASK(offset, val, mask)                       \
    CKGEN_WRITE32(offset, (CKGEN_READ32(offset) & (~(mask))) | (val))

#define AUD_CKGEN_BITS_W(addr, start, bitNum, val)                  \
    CKGEN_WRITE32(addr, (CKGEN_READ32(addr) & ~(AUDREG_MASK(start, bitNum))) | ((val) << start))

//================================================================//

#define AUDREG_READ_LOG(addr, len)                                          \
    {                                                                       \
        u32 i;                                                           \
        AUDLOG_NO_PREFIX(ALOG_INFO, T("[Read Reg] Start: "));                     \
        for (i = 0; i < len; i++) {                                         \
             if (i % 4 == 0) {                                              \
                AUDLOG_NO_PREFIX(ALOG_INFO, T("\n0x%6x : "), (u32)(addr+4*i));               \
             }                                                              \
             AUDLOG_NO_PREFIX(ALOG_INFO, T("0x%8x "), (u32)AUDREG_READ(addr+4*i));   \
        }                                                                   \
        AUDLOG_NO_PREFIX(ALOG_INFO, T("\n[Read Reg] End! \n"));                   \
    }


#define AUDREG_WRITE_LOG(addr, val)                                                             \
        AUDLOG_NO_PREFIX(ALOG_INFO, T("[Write Reg] 0x%6x: 0x%x -> "), (u32)addr, (u32)AUDREG_READ(addr));      \
        AUDREG_WRITE(addr,val);                                                                 \
        AUDLOG_NO_PREFIX(ALOG_INFO, T("0x%x  \n"), (u32)AUDREG_READ(addr));     \
        CKGEN_WRITE32(addr, val);


#endif // #ifndef _AUDIO_COMM_REG_RW_H_
