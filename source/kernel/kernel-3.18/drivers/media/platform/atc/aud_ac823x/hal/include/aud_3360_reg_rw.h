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

#if CONFIG_DRV_AUD_AC83XX
#include "x_ckgen.h"
#endif

#include "aud_3360_reg_afe.h"
#include "aud_3360_reg_asrc.h"
#include "aud_3360_reg_ckgen.h"
#include "aud_3360_reg_misc.h"
#include "aud_3360_reg_bt.h"

#include "x_ioopt.h"

#ifndef _AUDIO_3360_REG_RW_H_
#define _AUDIO_3360_REG_RW_H_

#define AUD_STATUS_OK                           ((s32) 0)
#define AUD_STATUS_ERR                          ((s32) -1)

#if 1

#if CONFIG_DRV_AUD_AC83XX

#define AUD_REG_WRITE(addr, val)            HAL_WRITE32(IO_BASE_VA + addr, val)
#else
extern unsigned long io_v_base;
#define AUD_REG_WRITE(addr, val)            HAL_WRITE32(io_v_base + addr, val)

#endif
#else
#define AUD_REG_WRITE(addr, val)                                    \
{                                                                   \
    HAL_WRITE32(IO_BASE + addr, val);                               \
    if ((addr == AUD_REG_RGBK2_INDRECT_FRNT_ADDR) ||                \
        (addr == AUD_REG_RGBK2_INDRECT_FRNT_DATA) ||                \
        (addr == AUD_REG_RGBK2_INDRECT_REAR_ADDR) ||                \
        (addr == AUD_REG_RGBK2_INDRECT_REAR_DATA) ||                \
        (addr == AUD_REG_RGBK2_INDRECT_GPS_ADDR) ||                 \
        (addr == AUD_REG_RGBK2_INDRECT_GPS_DATA))                   \
    {                                                               \
        AUD_PRINTF("Write Ret Addr 0x%08x=0x%x.\n", addr, val);     \
    }                                                               \
    else                                                            \
    {                                                               \
        AUD_PRINTF("Write Ret Addr 0x%08x=0x%x.\n", addr, AUD_REG_READ(addr));  \
    }                                                               \
}
#endif

#if CONFIG_DRV_AUD_AC83XX
#define AUD_REG_READ(addr)                  HAL_READ32(IO_BASE_VA + addr)
#else
extern unsigned long io_v_base;
#define AUD_REG_READ(addr)                  HAL_READ32(io_v_base + addr)

#endif

#define AUD_REG_MASK(start, bitNum)         ((((u32)(1 << (bitNum))) - 1) << start)


#define AUD_GET_BITS_VAL(val, start, num)                           \
    ((val & AUD_REG_MASK(start, num)) >> (start))

#define AUD_REG_BITS_WRITE(addr, start, bitNum, val)                \
    AUD_REG_WRITE(addr, (AUD_REG_READ(addr) & ~(AUD_REG_MASK(start, bitNum))) | ((val) << start))

#define AUD_REG_BITS_READ(addr, start, bitNum)                      \
    ((AUD_REG_READ(addr) & AUD_REG_MASK(start, bitNum)) >> start)

#define AUD_REG_CLRBIT(addr, val)                                   \
    AUD_REG_WRITE(addr, AUD_REG_READ(addr) & (~(val)))

#define AUD_REG_SETBIT(addr, val)                                   \
    AUD_REG_WRITE(addr, AUD_REG_READ(addr) | (val))

#if CONFIG_DRV_AUD_AC83XX

#define CKGEN_SETBITS(offset, dBit)        CKGEN_WRITE32(offset, CKGEN_READ32(offset) | (dBit))
#define CKGEN_CLRBITS(offset, dBit)        CKGEN_WRITE32(offset, CKGEN_READ32(offset) & (~(dBit)))

#define CKGEN_WRITE32_MASK(offset, val, mask)                       \
    CKGEN_WRITE32(offset, (CKGEN_READ32(offset) & (~(mask))) | (val))

#define CKGEN_BITS_WRITE(addr, start, bitNum, val)                  \
    CKGEN_WRITE32(addr, (CKGEN_READ32(addr) & ~(AUD_REG_MASK(start, bitNum))) | ((val) << start))

#else
extern unsigned long io_v_base;

#define CKGEN_SETBITS(offset, dBit)        writel(readl(io_v_base+offset) | (dBit), io_v_base+offset)// IO_WRITE32(io_base_ckgen, (offset), (IO_READ32(io_base_ckgen, (offset)) | (dBit)))
#define CKGEN_CLRBITS(offset, dBit)        writel(readl(io_v_base+offset) & (~(dBit)), io_v_base+offset)// IO_WRITE32(io_base_ckgen, (offset), (IO_READ32(io_base_ckgen, (offset)) &  (~dBit)))

#define CKGEN_WRITE32_MASK(offset, val, mask)                       \
    writel(readl(io_v_base+offset) & (~(mask)) | (val), io_v_base+offset)// IO_WRITE32(io_base_ckgen, offset, (IO_READ32(io_base_ckgen, (offset)) & (~(mask))) | (val))

#define CKGEN_BITS_WRITE(addr, start, bitNum, val)                  \
    writel(readl(io_v_base+addr) & ~(AUD_REG_MASK(start, bitNum)) | ((val) << start), io_v_base+addr)
#endif
#endif // #ifndef _AUDIO_3360_REG_RW_H_
