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

#ifndef GFX_HW_H
#define GFX_HW_H

#ifndef CONFIG_CHIP_VER_MT8560
#define CONFIG_CHIP_VER_MT8560 8560
#endif
#ifndef CONFIG_CHIP_VER_MT8580
#define CONFIG_CHIP_VER_MT8580 8580
#endif

#ifndef CONFIG_CHIP_VER_MT8561
#define CONFIG_CHIP_VER_MT8561 8561
#endif

#ifndef CONFIG_CHIP_VER_MT3363
#define CONFIG_CHIP_VER_MT3363 8563
#endif

//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------

#include "gfx_common.h"
#include "dram_model.h"
#include "drv_def.h"
#include "chip_ver.h"


#ifndef GL_XOR
#define GL_XOR
#endif


//---------------------------------------------------------------------------
// Configurations
//---------------------------------------------------------------------------
//#define GFX_PERFORMANCE_TEST
#ifdef GFX_PERFORMANCE_TEST
#define GFX_PERFORMANCE_TEST_SUPPORT 1
#else
#define GFX_PERFORMANCE_TEST_SUPPORT 0
#endif

//---------------------------------------------------------------------------
// Constant definitions
//---------------------------------------------------------------------------

/*! @name GFX Event Group Define */
/*! @{ */
#define GFX_HAL_EV_INITIAL           ((EV_GRP_EVENT_T) 0)
#define GFX_HAL_EV_PROC              ((EV_GRP_EVENT_T)(1) << 1)
#define GFX_HAL_EV_DONE_IRQ          ((EV_GRP_EVENT_T)(1) << 2)
#define GFX_HAL_EV_UNINIT            ((EV_GRP_EVENT_T)(1) << 3)
/*! @} */

#define GFX_ADDR_0                (IMAGE_BASE_VA)
#define GFX_ADDR_1                (IMAGE2_BASE_VA)
// must use IO_READ32 or IO_WRITE32 to control GFX HW registers
#define GFX_REG_G_CONFIG        (0x00)
#define GFX_REG_G_STATUS        (0x04)
#define GFX_REG_DRAMQ_STAD      (0x08)
#define GFX_REG_DRAMQ_LEN       (0x0C)
#define GFX_REG_G_MODE          (0x10)
#define GFX_REG_SRC_BSAD        (0x18)
#define GFX_REG_DST_BSAD        (0x1C)
#define GFX_REG_SRC_SIZE        (0x28)
#define GFX_REG_S_OSD_WIDTH     (0x2C)

#define GFX_REG_LEGAL_START     (0x90)
#define GFX_REG_LEGAL_END       (0x94)

#define GFX_REG_0x40E0          (0xE0)
#define GFX_REG_0x40E4          (0xE4)

#define GFX_REG_0x40EC          (0xEC)
#define GFX_REG_0x40F0          (0xF0)
#define GFX_REG_WAIT_MMU        (0xFC)

#define GFX_REG_ROP             (0xA0)
#define GFX_REG_IDX2DIR         (0xA4)
#define GFX_REG_MMU_SF          (0x104)
#define GFX_REG_G_256CPT        (0x108)
#define GFX_REG_G_PGIG          (0x124)
#define GFX_REG_SRC_RING        (0x130)
#define GFX_REG_DST_RING        (0x134)

//------------------------------------------------
// GFX HW registers' shift
//   [G_CONFIG] - 0x4000
#define GREG_SHT_G_RST          30
#define GREG_SHT_CQ_RST         28
#define GREG_SHT_SHORT_CMQ      25
#define GREG_SHT_DYN_HIGH       24
#define GREG_SHT_MMU_CMQ        23
#define GREG_SHT_ENG_LP         17
#define GREG_SHT_SRAM_LP        16
#define GREG_SHT_SDFIFO_THRS     9
#define GREG_SHT_DRAMQ_MODE      8
#define GREG_SHT_REQ_INTVAL      6
#define GREG_SHT_CMDFIFO_THRS    4
#define GREG_SHT_POST_THRS       2
#define GREG_SHT_INT_MASK        1
#define GREG_SHT_EN_DRAMQ        0
//   [G_STATUS] - 0x4004
#define GREG_SHT_VERSION_ID      8
#define GREG_SHT_IDLE            0
//   [DRAMQ_STAD] - 0x4008
#define GREG_SHT_CYC_SIZE       30
#define GREG_SHT_DRAMQ_BSAD      0

//   [G_MODE] - 0x400C
#define GREG_SHT_SW_MODE             28

//   [G_MODE] - 0x4010
#define GREG_SHT_CM                  0
#define GREG_SHT_OP_MODE             4
#define GREG_SHT_FIRE               11
#define GREG_SHT_BURST_EN           12
#define GREG_SHT_BURST_MODE         13
#define GREG_SHT_SRCQ_FIRE          12
#define GREG_SHT_DSTQ_FIRE          13

//   [SRC_BSAD] - 0x4018
#define GREG_SHT_SRC_BSAD            0
#define GREG_SHT_CHAR_CM            30
//   [DST_BSAD] - 0x401C
#define GREG_SHT_DST_BSAD            0
#define GREG_SHT_DST_BSAD_WR        31
//   [SRC_SIZE] - 0x4028
#define GREG_SHT_SRC_WIDTH           0
#define GREG_SHT_RL_DEC             15
#define GREG_SHT_SRC_HEIGHT         16
#define GREG_SHT_SRC_CM             27
//   [S_OSD_WIDTH] - 0x402C
#define GREG_SHT_OSD_WIDTH           0
#define GREG_SHT_SRC_PITCH          16
//   [ROP] - 0x40A0
#define GREG_SHT_CMP_FLAG        9
//   [IDX2DIR] - 0x40A4
#define GREG_SHT_PAL_BSAD            0
#define GREG_SHT_MSB_LEFT           30
#define GREG_SHT_LN_ST_BYTE_AL      31
// [MMU Self Fire] - 0x40fc
#define GREG_SHT_CMQ_SELF_FIRE_CYC  0
#define GREG_SHT_CMQ_BYPASS_MMU     27
#define GREG_SHT_SELF_FIRE_EN       28
#define GREG_SHT_ORG_FIRE_EN        29
#define GREG_SHT_CMQ_SELF_FIRE_EN   30
#define GREG_SHT_HW_SELF_FIRE_EN    31

//   [G_256CPT] - 0x4108
#define GREG_SHT_256CPT_FIRE         0
#define GREG_SHT_USE_256CPT          1
#define GREG_SHT_CPT_DRAM_EN         2
#define GREG_SHT_IDX2DIR_EN          7
//   [G_PGIG] - 0x4124
#define GREG_SHT_PGIG_RLE_EN         0
#define GREG_SHT_PGIG_DIR_EN         1
#define GREG_SHT_HDMV_CPT_NUM        8
#define GREG_SHT_YR_SEL             16
#define GREG_SHT_UG_SEL             18
#define GREG_SHT_VB_SEL             20
#define GREG_SHT_A_SEL              22
#define GREG_SHT_SRCQ_RING_EN       24
#define GREG_SHT_DSTQ_RING_EN       25

//   [SRC_RING] - 0x4130
#define GREG_SHT_SRC_END_BSAD        0
#define GREG_SHT_SRC_SIZE_EN        30
#define GREG_SHT_SRC_BSAD_WR        31
//   [SRC_RING] - 0x4134
#define GREG_SHT_SRCQ_LEN             0
#define GREG_SHT_WDLE                1
#define GREG_SHT_DST_HEIGHT_RING    16


//------------------------------------------------
// GFX HW registers' mask
//[WT_CONFIG]- 0x40800
#define GREG_WT_HRRI_ENA         (UINT32)0x1;

//   [G_CONFIG] - 0x4000
#define GREG_MSK_G_RST          ((UINT32)0x3 << GREG_SHT_G_RST)
#define GREG_MSK_CQ_RST         ((UINT32)0x3 << GREG_SHT_CQ_RST)
#define GREG_MSK_SHORT_CMQ      ((UINT32)0x1 << GREG_SHT_SHORT_CMQ)
#define GREG_MSK_DYN_HIGH       ((UINT32)0x1 << GREG_SHT_DYN_HIGH)
#define GREG_MSK_MMU_CMQ        ((UINT32)0x1 << GREG_SHT_MMU_CMQ)
#define GREG_MSK_ENG_LP         ((UINT32)0x1 << GREG_SHT_ENG_LP)
#define GREG_MSK_SRAM_LP         ((UINT32)0x1 << GREG_SHT_SRAM_LP)
#define GREG_MSK_SDFIFO_THRS    ((UINT32)0x3 << GREG_SHT_SDFIFO_THRS)
#define GREG_MSK_DRAMQ_MODE     ((UINT32)0x1 << GREG_SHT_DRAMQ_MODE)
#define GREG_MSK_REQ_INTVAL     ((UINT32)0x3 << GREG_SHT_REQ_INTVAL)
#define GREG_MSK_CMDFIFO_THRS   ((UINT32)0x3 << GREG_SHT_CMDFIFO_THRS)
#define GREG_MSK_POST_THRS      ((UINT32)0x3 << GREG_SHT_POST_THRS)
#define GREG_MSK_INT_MASK       ((UINT32)0x1 << GREG_SHT_INT_MASK)
#define GREG_MSK_EN_DRAMQ       ((UINT32)0x1 << GREG_SHT_EN_DRAMQ)
//   [G_STATUS] - 0x4004
#define GREG_MSK_VERSION_ID     ((UINT32)0xFF << GREG_SHT_VERSION_ID)
#define GREG_MSK_IDLE           ((UINT32)0x1 << GREG_SHT_IDLE)
//   [DRAMQ_STAD] - 0x4008
#define GREG_MSK_CYC_SIZE       ((UINT32)0x3 << GREG_SHT_CYC_SIZE)
#define GREG_MSK_DRAMQ_BSAD     ((UINT32)0xFFFFFFF << GREG_SHT_DRAMQ_BSAD)
//   [SRC_BSAD] - 0x4018
#define GREG_MSK_SRC_BSAD           ((UINT32)0x3FFFFFFF << GREG_SHT_SRC_BSAD)
#define GREG_MSK_CHAR_CM            ((UINT32)0x3 << GREG_SHT_CHAR_CM)

//   [G_MODE] - 0x4010
#define GREG_MSK_CM                 ((UINT32)0xF << GREG_SHT_CM)
#define GREG_MSK_OP_MODE            ((UINT32)0x1F << GREG_SHT_OP_MODE)
#define GREG_MSK_FIRE               ((UINT32)0x1 << GREG_SHT_FIRE)
#define GREG_MSK_BURST_EN           ((UINT32)0x1 << GREG_SHT_BURST_EN)
#define GREG_MSK_BURST_MODE         ((UINT32)0x3 << GREG_SHT_BURST_MODE)
#define GREG_MSK_SRCQ_FIRE          ((UINT32)0x1 << GREG_SHT_SRCQ_FIRE)
#define GREG_MSK_DSTQ_FIRE          ((UINT32)0x1 << GREG_SHT_DSTQ_FIRE)

//   [DST_BSAD] - 0x401C
#define GREG_MSK_DST_BSAD           ((UINT32)0x3FFFFFFF << GREG_SHT_DST_BSAD)
#define GREG_MSK_DST_BSAD_WR        ((UINT32)0x1 << GREG_SHT_DST_BSAD_WR)
//   [SRC_SIZE] - 0x4028
#define GREG_MSK_SRC_WIDTH          ((UINT32)0x7FFF << GREG_SHT_SRC_WIDTH)
#define GREG_MSK_RL_DEC             ((UINT32)0x1 << GREG_SHT_RL_DEC)
#define GREG_MSK_SRC_HEIGHT         ((UINT32)0x7FFF << GREG_SHT_SRC_HEIGHT)
#define GREG_MSK_SRC_CM             ((UINT32)0xF << GREG_SHT_SRC_CM)

//   [S_OSD_WIDTH] - 0x402C
#define GREG_MSK_OSD_WIDTH          ((UINT32)0xFFFF << GREG_SHT_OSD_WIDTH)
#define GREG_MSK_SRC_PITCH          ((UINT32)0xFFFF << GREG_SHT_SRC_PITCH)
//   [ROP] - 0x40A0
#define GREG_MSK_CMP_FLAG       ((UINT32)0x1 << GREG_SHT_CMP_FLAG)
//   [IDX2DIR] - 0x40A4
#define GREG_MSK_PAL_BSAD           ((UINT32)0x3FFFFFFF << GREG_SHT_PAL_BSAD)
#define GREG_MSK_MSB_LEFT           ((UINT32)0x1 << GREG_SHT_MSB_LEFT)
#define GREG_MSK_LN_ST_BYTE_AL      ((UINT32)0x1 << GREG_SHT_LN_ST_BYTE_AL)
//   [G_256CPT] - 0x4108
#define GREG_MSK_256CPT_FIRE         ((UINT32)0x1 << GREG_SHT_256CPT_FIRE)
#define GREG_MSK_USE_256CPT          ((UINT32)0x1 << GREG_SHT_USE_256CPT)
#define GREG_MSK_CPT_DRAM_EN         ((UINT32)0x1 << GREG_SHT_CPT_DRAM_EN)
#define GREG_MSK_IDX2DIR_EN          ((UINT32)0x1 << GREG_SHT_IDX2DIR_EN)

//   [G_PGIG] - 0x4124
#define GREG_MSK_PGIG_RLE_EN        ((UINT32)0x1 << GREG_SHT_PGIG_RLE_EN)
#define GREG_MSK_PGIG_DIR_EN        ((UINT32)0x1 << GREG_SHT_PGIG_DIR_EN)
#define GREG_MSK_HDMV_CPT_NUM       ((UINT32)0xFF << GREG_SHT_HDMV_CPT_NUM)
#define GREG_MSK_YR_SEL             ((UINT32)0x3 << GREG_SHT_YR_SEL)
#define GREG_MSK_UG_SEL             ((UINT32)0x3 << GREG_SHT_UG_SEL)
#define GREG_MSK_VB_SEL             ((UINT32)0x3 << GREG_SHT_VB_SEL)
#define GREG_MSK_A_SEL              ((UINT32)0x3 << GREG_SHT_A_SEL)


#define GREG_MSK_SRCQ_RING_EN       ((UINT32)0x1 << GREG_SHT_SRCQ_RING_EN)
#define GREG_MSK_DSTQ_RING_EN       ((UINT32)0x1 << GREG_SHT_DSTQ_RING_EN)
//   [SRC_RING] - 0x4130
#define GREG_MSK_SRC_END_BSAD       ((UINT32)0x3FFFFFFF << GREG_SHT_SRC_END_BSAD)
#define GREG_MSK_SRC_SIZE_EN        ((UINT32)0x1 << GREG_SHT_SRC_SIZE_EN)
#define GREG_MSK_SRC_BSAD_WR        ((UINT32)0x1 << GREG_SHT_SRC_BSAD_WR)
//   [SRC_RING] - 0x4134
#define GREG_MSK_SRCQ_LEN           ((UINT32)0xFFFF << GREG_SHT_SRCQ_LEN)
#define GREG_MSK_DST_HEIGHT_RING    ((UINT32) 0x7FF << GREG_SHT_DST_HEIGHT_RING)
#define GREG_MSK_WDLE               ((UINT32)0x1    << GREG_SHT_WDLE)
#define GFX_G_RST_READY         ((UINT32)0x80000000)
#define GFX_CQ_RST_READY        ((UINT32)0x20000000)
extern unsigned int gfx_base;

//------------------------------------------------
// GFX HW registers' field
//   [G_MODE] - 0x4010
#define GREG_FLD_CM(x)              (((UINT32)(x) << GREG_SHT_CM) & GREG_MSK_CM)
#define GREG_FLD_OP_MODE(x)         (((UINT32)(x) << GREG_SHT_OP_MODE) & GREG_MSK_OP_MODE)
#define GREG_FLD_BURST_EN(x)        (((UINT32)(x) << GREG_SHT_BURST_EN) & GREG_MSK_BURST_EN)
#define GREG_FLD_BURST_MODE(x)      (((UINT32)(x) << GREG_SHT_BURST_MODE) & GREG_MSK_BURST_MODE)

//   [SRC_BSAD] - 0x4018
#define GREG_FLD_SRC_BSAD(x)        (((UINT32)(x) << GREG_SHT_SRC_BSAD) & GREG_MSK_SRC_BSAD)
#define GREG_FLD_CHAR_CM(x)         (((UINT32)(x) << GREG_SHT_CHAR_CM) & GREG_MSK_CHAR_CM)
    #define GREG_FLD_CHAR_CM_1BIT   GREG_FLD_CHAR_CM(0)
    #define GREG_FLD_CHAR_CM_2BIT   GREG_FLD_CHAR_CM(1)
    #define GREG_FLD_CHAR_CM_4BIT   GREG_FLD_CHAR_CM(2)
    #define GREG_FLD_CHAR_CM_8BIT   GREG_FLD_CHAR_CM(3)
//   [DST_BSAD] - 0x401C
#define GREG_FLD_DST_BSAD(x)        (((UINT32)(x) << GREG_SHT_DST_BSAD) & GREG_MSK_DST_BSAD)

//   [SRC_SIZE] - 0x4028
#define GREG_FLD_SRC_WIDTH(x)       (((UINT32)(x) << GREG_SHT_SRC_WIDTH) & GREG_MSK_SRC_WIDTH)
#define GREG_FLD_SRC_HEIGHT(x)      (((UINT32)(x-1) << GREG_SHT_SRC_HEIGHT) & GREG_MSK_SRC_HEIGHT)


//   [S_OSD_WIDTH] - 0x402C
#define GREG_FLD_OSD_WIDTH(x)        (((UINT32)(x) << GREG_SHT_OSD_WIDTH) & GREG_MSK_OSD_WIDTH)
#define GREG_FLD_SRC_PITCH(x)        (((UINT32)(x) << GREG_SHT_SRC_PITCH) & GREG_MSK_SRC_PITCH)
//   [IDX2DIR] - 0x40A4
#define GREG_FLD_PAL_BSAD(x)         (((UINT32)(x) << GREG_SHT_PAL_BSAD) & GREG_MSK_PAL_BSAD)

//   [G_PGIG] - 0x4124
#define GREG_FLD_HDMV_CPT_NUM(x)     (((UINT32)(x) << GREG_SHT_HDMV_CPT_NUM) & GREG_MSK_HDMV_CPT_NUM)
#define GREG_FLD_YR_SEL(x)           (((UINT32)(x) << GREG_SHT_YR_SEL) & GREG_MSK_YR_SEL)
#define GREG_FLD_UG_SEL(x)           (((UINT32)(x) << GREG_SHT_UG_SEL) & GREG_MSK_UG_SEL)
#define GREG_FLD_VB_SEL(x)           (((UINT32)(x) << GREG_SHT_VB_SEL) & GREG_MSK_VB_SEL)
#define GREG_FLD_A_SEL(x)            (((UINT32)(x) << GREG_SHT_A_SEL)  & GREG_MSK_A_SEL)

//   [SRC_RING] - 0x4130
#define GREG_FLD_SRC_END_BSAD(x)     (((UINT32)(x) << GREG_SHT_SRC_END_BSAD) & GREG_MSK_SRC_END_BSAD)

//   [DST_RING] - 0x4134
#define GREG_FLD_SRCQ_LEN(x)         (((UINT32)(x) << GREG_SHT_SRCQ_LEN) & GREG_MSK_SRCQ_LEN)
#define GREG_FLD_DST_HEIGHT_RING(x)  (((UINT32)(x) << GREG_SHT_DST_HEIGHT_RING) & GREG_MSK_DST_HEIGHT_RING)

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

/// Read 32 bits data from GFX HW registers.
#define GFX_READ32(u4GfxHwId, offset)           IO_READ32(gfx_base, (offset))

#ifdef GFX_RISC_MODE  // risc mode
/// Write 32 bits data into GFX HW registers.
#define GFX_WRITE32(u4GfxHwId, offset, value)   IO_WRITE32(gfx_base, (offset), (value)); \
                                                if (GFX_READ32(u4GfxHwId, offset) != value) \
                                                { \
                                                    EvPrintf("[Gfx][Ev][Warning] Register Writer Fail @ 0x%x \n", offset); \
                                                } 
#else
/// Write 32 bits data into GFX HW registers.
#define GFX_WRITE32(u4GfxHwId, offset, value)   IO_WRITE32(gfx_base, (offset), (value))
#endif

//---------------------------------------------------------------------------
// Type definitions
//---------------------------------------------------------------------------
typedef struct _GFX_HAL_INST_T
{
    UINT32      u4HwInstId;
    HANDLE_T    hEvent;
}GFX_HAL_INST_T;

typedef struct {
    UINT32 u4StopID[4];
} GFX_BPCOMP_STOP_ID_T;

typedef enum {
    WT_CF_WEIGHT_PAD        = 1 << 0,
    WT_CF_EDGE57_BYP        = 1 << 1,
    WT_CF_FIFO_LEVEL        = 1 << 2,
    WT_CF_PQ_RGB_PROC       = 1 << 6,
    WT_CF_PQ_EDGE_PROC      = 1 << 7,
    WT_CF_CONT_BE           = 1 << 8,
    WT_CF_PQ_SMART_MODE_A   = 1 << 9,
    WT_CF_EDGE_THR          = 1 << 11,
    WT_CF_EDGE_DIFF         = 1 << 13,
    WT_CF_BYPASS            = 1 << 16,
    WT_CF_EDGE_THR2         = 1 << 17,
    WT_CF_BURST             = 1 << 19,
    WT_CF_CHKSUM_SHFT       = 1 << 20,
    WT_CF_COMPACT           = 1 << 21,
    WT_CF_ARGB4888          = 1 << 22,
    WT_CF_MON_SEL           = 1 << 23,
    WT_CF_PQ_PRE            = 1 << 24,
    WT_CF_EDGE_MEAN         = 1 << 26,
    WT_CF_SE_THR            = 1 << 28,
    WT_CF_SW_DRAM_RSTB      = 1 << 30,
    WT_CF_SW_RSTB           = 1 << 31
} WT_CONFIG_FLAG_T;

typedef struct
{
    UINT32 fg_WT_WEIGHT_PAD   : 1; /* weight_padding (PRE_PROC), 1'b1:enable */
    UINT32 fg_WT_EDGE57_BYP   : 1; /* bypass edge 57(two-edges), 1'b1: bypass */
    UINT32 fg_WT_FIFO_LEVEL   : 4; /* WT request FIFO level (default: 16) */
    UINT32 fg_PQ_RGB_PROC     : 1; /* RGB_BYPASS,1'b1: disable */
    UINT32 fg_PQ_EDGE_PROC    : 1; /* EDGE_BYPASS, 1'b1: disable */
    UINT32 fg_WT_CONT_BE      : 1; /* continuous byte enable merge, 1: merge */
    UINT32 fg_PQ_SMART_MODE_A : 1; /* smart_mode_a, 1'b1: enable */
    UINT32                    : 1;
    UINT32 fg_EDGE_THR        : 2; /* Alpha EDGE_PROC threshold(0~3) */
    UINT32 fg_EDGE_DIFF       : 3; /* EDGE_PROC difference threshold (0~7) */
    UINT32 fg_WT_BYPASS       : 1; /* bypass WT, 1'b1: disable WT */
    UINT32 fg_EDGE_THR2       : 2; /* Luma EDGE_PROC threshold (0~3) */
    UINT32 fg_WT_BURST        : 1; /* burst read enable */
    UINT32 fg_WT_CHKSUM_SHFT  : 1; /* checksum shift mode, 0:w/o shift, 1: w/shift */
    UINT32 fg_WT_COMPACT      : 1; /* compact mode for saving bandwidth, 1: enable */
    UINT32 fg_WT_ARGB4888     : 1; /* WT ARGB4888 (alpha 4-b bypass), 1: enable ARGB4888 */
    UINT32 fg_WT_MON_SEL      : 1; /* monitor selection */
    UINT32 fg_PQ_PRE          : 2; /* PQ Alpha Preprocessing, 2b'1x: bypass, 2'b00: enable with shift 3, 2'b01: enable with shift 0 */
    UINT32 fg_EDGE_MEAN       : 2; /* EDGE_PROC(set to 2'b10) */
    UINT32 fg_SE_THR          : 2; /* GRANDIENT_PROC(set to 2'b10) */
    UINT32 fg_WT_SW_DRAM_RSTB : 1; /* WT SW DRAM I/F reset (negative), 0: reset */
    UINT32 fg_WT_SW_RSTB      : 1; /* WT SW reset (negetive), 0: reset */
} WT_CONFIG_REG_T;

typedef struct
{
    UINT32 u4Flag; /* Values of WT_CONFIG_FLAG_T, connected by OR */
    WT_CONFIG_REG_T rReg;
} WT_CONFIG_T;
//---------------------------------------------------------------------------
// Inter-file functions
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------------

extern INT32 GFX_HwInit(void);

extern INT32 GFX_HwUninit(void);

extern INT32 GFX_HwInit2(void);

extern INT32 GFX_HwGetRegBase(UINT32 u4GfxHwId, UINT32 **ppu4RegBase);

extern void GFX_HwISR(UINT32 u4GfxHwId);

extern void GFX_HwWait(void);

extern INT32 GFX_HwReset(UINT32 u4GfxHwId, UINT32 u4Reset);

extern INT32 GFX_HwGetIdle(UINT32 u4GfxHwId);

extern INT32 GFX_HwAction(UINT32 u4GfxHwId);

extern void GFX_HwSetRiscMode(UINT32 u4GfxHwId);

extern void GFX_HwSetCmdQMode(UINT32 u4GfxHwId);

extern void GFX_HwCrashRecover(void);

extern void GFX_HwSetReqInterval(UINT32 u4GfxHwId, UINT32 u4ReqInterval);
extern UINT32 _GFX_GetInterruptCount(UINT32 u4GfxHwId);
extern INT32 GFX_HwGetMemCompareResult(UINT32 u4GfxHwId);
extern INT32 i4GfxHwProc(UINT32 u4HwInstId);
extern INT32 i4GfxHwWait(UINT32 u4HwInstId);

extern INT32 GFX_HwResetEngine(UINT32 u4GfxHwId);
extern INT32 GFX_HwResetCmdQue(UINT32 u4GfxHwId);
extern INT32 GFX_HwResetWT(UINT32 u4GfxHwId,UINT32 u4Reset);
#ifdef GL_XOR
void GFX_Hal_SetCallback(UINT32 u4GfxHwId, BOOL bEnableCallback);
void GFX_Hal_SetSwISR(UINT32 u4GfxHwId);
#endif

extern INT32 GFX_HwGetBpCompStopID(UINT32 u4GfxHwId, GFX_BPCOMP_STOP_ID_T *prBpCompStopID);
INT32 GFX_HwSetHighPriorityMode(UINT32 u4GfxHwId, UINT32 u4HighPriority);
// for debug use
#if defined(GFX_DEBUG_MODE)

extern INT32 GFX_HwGetHwVersion(UINT32 u4GfxHwId, UINT32 *pu4HwVersion);

extern void GFX_HwEnableLowPowerMode(UINT32 u4GfxHwId);

extern void GFX_HwDisableLowPowerMode(UINT32 u4GfxHwId);

extern void GFX_HwSetSDFifoThreshold(UINT32 u4GfxHwId, UINT32 u4Value);

extern void GFX_HwSetCMDFifoThreshold(UINT32 u4GfxHwId, UINT32 u4Value);

extern void GFX_HwSetPOSTFifoThreshold(UINT32 u4GfxHwId, UINT32 u4Value);

#endif // #if defined(GFX_DEBUG_MODE)

#ifdef GFX_PERFORMANCE_TEST
#include "x_timer.h"

extern HAL_TIME_T g_rGfxStartTime, g_rGfxEndTime, g_rGfxResultTime;
extern BOOL       g_fgGfxPfmTesting;
void  vGfxStartPfmTest(void);
void vGfxEndPfmTest(void);
UINT32 u4GetAverageMicrosPerLoop(UINT32 u4Loop);
#endif

#endif // GFX_HW_H


