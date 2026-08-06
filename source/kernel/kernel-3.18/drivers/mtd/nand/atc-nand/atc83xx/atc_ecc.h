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
#ifndef _NFIECC2_H
#define _NFIECC2_H
#include <mach/ac83xx_basic.h>
#include "atc_nfi.h"
#define IO_VIRT 0xFD000000

//#define  USE_60BIT

#ifdef    USE_60BIT   
#define NFIECC_BASE_REG        (IO_VIRT+0x1E800) 
#else
#define NFIECC_BASE_REG        (IO_VIRT+0x1EC00) 
#endif

    #define ENC_EN      0x01
#ifdef USE_60BIT
    #define ENC_TNUM(x)			(((u32)x>24)?(((u32)x/4)+4):(((u32)x/2)-2))
    #define ENC_NFI_MODE		0x01 << 5
#else
    #define ENC_TNUM(x)                ((((u32) x / 2) - 2))
    #define ENC_NFI_MODE             0x01 << 4
#endif
    #define ENC_MS(x)                    (((u32) x &0x3FFF) << 16)
    #define ENC_IDLE                    0x01
    #define DEC_EN      0x01

#ifdef USE_60BIT
    #define DEC_TNUM(x)                (((u32)x>24)?(((u32)x/4)+4):(((u32)x/2)-2))
    #define DEC_NFI_MODE             0x01 << 5
#else
    #define DEC_TNUM(x)                (((((u32) x) / 2) - 2))
    #define DEC_NFI_MODE             0x01 << 4
#endif

    #define DEC_CON(x)                    (((u32) x &0x03) << 12)     
    #define DEC_CS(x)                    ((((u32) x )&0x3FFF) << 16)
    #define DEC_EMPTY_EN             0x80000000

#ifdef USE_60BIT
    #define DECENUM_MASK     0x3F
#else
    #define DECENUM_MASK	 0x1F
#endif

#define DEC_IDLE                    0x01
#define DEC_IRQEN                0x01
#define DEC_IRQSTA                0x01

typedef enum {
   ECC_4_BITS = 4,
   ECC_6_BITS = 6,
   ECC_8_BITS = 8,
   ECC_10_BITS = 10,
   ECC_12_BITS = 12,
   ECC_22_BITS = 22,
   ECC_24_BITS = 24,
#ifdef USE_60BIT
   ECC_28_BITS = 28,
   ECC_32_BITS = 32,
   ECC_36_BITS = 36,
   ECC_40_BITS = 40,
   ECC_44_BITS = 44,
   ECC_48_BITS = 48,
   ECC_52_BITS = 52,
   ECC_56_BITS = 56,
   ECC_60_BITS = 60
#endif
} ECC_Level_t;

typedef enum {
   ECC_DEC_NONE,
   ECC_DEC_DETECT,
   ECC_DEC_LOCATE,
   ECC_DEC_CORRECT
} ECC_Decode_Type_t;

#endif //_NFIECC_H
