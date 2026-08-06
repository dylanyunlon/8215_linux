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

#ifndef GFX_DIF_REG_H
#define GFX_DIF_REG_H
#ifndef CONFIG_CHIP_VER_MT8560
#define CONFIG_CHIP_VER_MT8560 8560
#endif


//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------

#include "gfx_common.h"


//---------------------------------------------------------------------------
// Configurations
//---------------------------------------------------------------------------
#ifndef CONFIG_CHIP_VER_MT8555
#define CONFIG_CHIP_VER_MT8555   8555
#endif

#ifndef CONFIG_CHIP_VER_MT3363
#define CONFIG_CHIP_VER_MT3363   8563
#endif

//---------------------------------------------------------------------------
// Constant definitions
//---------------------------------------------------------------------------

/** size of gfx register space (in unit sizeof(uint32))
 *
 */
#define GREG_FILE_SIZE          (INT32)(GREG_LAST)

#define GREG_LOSSLESS_FILE_SIZE (INT32)(GREG_LL_LAST)
#define GFX_ENABLE              1
#define GFX_DISABLE             0

#define GFX_FIELD_MODE          1
#define GFX_FRAME_MODE          0

#define GFX_CMD_BUF_CYLIC       0
#define GFX_CMD_BUF_SINGLE      1

#define GFX_SOFT_RESET          3

#define GFX_HW_IDLE             1
#define GFX_HW_BUSY             0

#define GFX_CMD_BUF_32KB        0
#define GFX_CMD_BUF_64KB        1
#define GFX_CMD_BUF_128KB       2
#define GFX_CMD_BUF_256KB       3

#define GFX_ENGINE_FIRE         1


//---------------------------------------------------------------------------
// Type definitions
//---------------------------------------------------------------------------

/** gfx hardware register enumeration
 *
 */
typedef enum _E_GFX_REG_T
{
    GREG_G_CONFIG           = 0x00,     // 0x4000
    GREG_G_STATUS           = 0x01,     // 0x4004
    GREG_DRAMQ_STAD         = 0x02,     // 0x4008
    GREG_DRAMQ_LEN          = 0x03,     // 0x400C
    GREG_G_MODE             = 0x04,     // 0x4010
    GREG_RECT_COLOR         = 0x05,     // 0x4014
    GREG_SRC_BSAD           = 0x06,     // 0x4018
    GREG_DST_BSAD           = 0x07,     // 0x401C
    GREG_SRC_XY             = 0x08,     // 0x4020
    GREG_DST_XY             = 0x09,     // 0x4024
    GREG_SRC_SIZE           = 0x0A,     // 0x4028
    GREG_S_OSD_WIDTH        = 0x0B,     // 0x402C
    GREG_CLIP_BR            = 0x0C,     // 0x4030
    GREG_CLIP_TL            = 0x0D,     // 0x4034
    GREG_GRAD_X_DELTA       = 0x0E,     // 0x4038
    GREG_GRAD_Y_DELTA       = 0x0F,     // 0x403C
    GREG_GRAD_XY_INC        = 0x10,     // 0x4040
    GREG_BITBLT_MODE        = 0x11,     // 0x4044
    GREG_KEY_DATA0          = 0x12,     // 0x4048
    GREG_KEY_DATA1          = 0x13,     // 0x404C
    GREG_SRCCBCR_BSAD       = 0x14,     // 0x4050
    GREG_SRCCBCR_PITCH      = 0x15,     // 0x4054
    GREG_DSTCBCR_BSAD       = 0x16,     // 0x4058
    GREG_F_COLOR            = 0x17,     // 0x405C
    GREG_B_COLOR            = 0x18,     // 0x4060
    GREG_COL_TRAN0          = 0x19,     // 0x4064
    GREG_COL_TRAN1          = 0x1A,     // 0x4068
    GREG_COL_TRAN2          = 0x1B,     // 0x406C
    GREG_COL_TRAN3          = 0x1C,     // 0x4070
    GREG_COL_TRAN4          = 0x1D,     // 0x4074
    GREG_COL_TRAN5          = 0x1E,     // 0x4078
    GREG_COL_TRAN6          = 0x1F,     // 0x407C
    GREG_COL_TRAN7          = 0x20,     // 0x4080
    GREG_STR_BLT_H          = 0x21,     // 0x4084
    GREG_STR_BLT_V          = 0x22,     // 0x4088
    GREG_STR_DST_SIZE       = 0x23,     // 0x408C
    GREG_LEGAL_START_ADDR   = 0x24,     // 0x4090
    GREG_LEGAL_END_ADDR     = 0x25,     // 0x4094
    GREG_DUMMY              = 0x26,     // 0x4098
    GREG_ALCOM_LOOP         = 0x27,     // 0x409C
    GREG_ROP                = 0x28,     // 0x40A0
    GREG_IDX2DIR            = 0x29,     // 0x40A4
    GREG_SRC_WBBSAD         = 0x2A,     // 0x40A8
    GREG_DST_WBBSAD         = 0x2B,     // 0x40AC
    GREG_SRCCBCR_WBBSAD     = 0x2C,     // 0x40B0
    GREG_XOR_COLOR          = 0x2D,     // 0x40B4
    GREG_0x40B8             = 0x2E,     // 0x40B8 //GREG_VA_MSB
    GREG_RACING_MODE        = 0x2F,     // 0x40BC
    GREG_BPCOMP_CFG         = 0x30,     // 0x40C0
    GREG_BPCOMP_AD_END      = 0x31,     // 0x40C4
    GREG_BPCOMP_DBG1        = 0x32,     // 0x40C8
    GREG_BPCOMP_CHKSUM      = 0x33,     // 0x40CC
    GREG_BPCOMP_STOP_ID_0   = 0x34,     // 0x40D0
    GREG_BPCOMP_STOP_ID_1   = 0x35,     // 0x40D4
    GREG_BPCOMP_STOP_ID_2   = 0x36,     // 0x40D8
    GREG_BPCOMP_STOP_ID_3   = 0x37,     // 0x40DC
    GREG_0x40E0             = 0x38,     // 0x40E0:BPCOMP_PACKET
    GREG_0x40E4             = 0x39,     // 0x40E4:BPCOMP_OFFSET1
    GREG_0x40E8             = 0x3A,     // 0x40E8:BPCOMP_OFFSET2
    GREG_0x40EC             = 0x3B,     // 0x40EC:BPCOMP_INDEX_BASD
    GREG_VA_MSB             = 0x3C,     // 0x40F0:reserved? //GREG_0x40F0
    GREG_0x40F4             = 0x3D,     // 0x40F4:reserved?
    GREG_0x40F8             = 0x3E,     // 0x40F8:reserved?
    GREG_0x40FC             = 0x3F,     // 0x40FC:reserved?
    GREG_0x4100             = 0x40,     // 0x4100:reserved?
    GREG_BMP_STATUS         = 0x41,     // 0x4104:reserved?
    GREG_G_256CPT           = 0x42,     // 0x4108
    GREG_0x410C             = 0x43,     // 0x410C:reserved?
    GREG_0x4110             = 0x44,     // 0x4110:reserved?
    GREG_0x4114             = 0x45,     // 0x4114:reserved?
    GREG_0x4118             = 0x46,     // 0x4118:reserved?
    GREG_0x411C             = 0x47,     // 0x411C:reserved?
    GREG_0x4120             = 0x48,     // 0x4120:reserved?
    GREG_G_PGIG             = 0x49,     // 0x4124
    GREG_BMP_SIZE           = 0x4A,     // 0x4128:reserved?
    GREG_HDMV_IDX_256       = 0x4B,     // 0x412C
    GREG_SRC_RING           = 0x4C,     // 0x4130:reserved?
    GREG_DST_RING           = 0x4D,     // 0x4134:reserved?
    GREG_CPT_ADDR_RD        = 0x4E,     // 0x4138
    GREG_CPT_DATA_RD        = 0x4F,     // 0x413C
#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8560)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    GREG_0x4140             = 0x50,     // 0x4140
    GREG_0x4144             = 0x51,     // 0x4144
    GREG_0x4148             = 0x52,     // 0x4148
    GREG_0x414C             = 0x53,     // 0x414C
    GREG_0x4150             = 0x54,     // 0x4150
    GREG_0x4154             = 0x55,     // 0x4154
    GREG_0x4158             = 0x56,     // 0x4158
    GREG_0x415C             = 0x57,     // 0x415C
    GREG_0x4160             = 0x58,     // 0x4160
    GREG_0x4164             = 0x59,     // 0x4164
    GREG_0x4168             = 0x5A,     // 0x4168
    GREG_0x416C             = 0x5B,     // 0x416C
    GREG_0x4170             = 0x5C,     // 0x4170
    GREG_0x4174             = 0x5D,     // 0x4174
    GREG_0x4178             = 0x5E,     // 0x4178
    GREG_0x417C             = 0x5F,     // 0x417C
    GREG_IOMMU_CFG0         = 0x60,     // 0x4180
    GREG_IOMMU_CFG1         = 0x61,     // 0x4184
    GREG_IOMMU_CFG2         = 0x62,     // 0x4188
    GREG_IOMMU_CFG3         = 0x63,     // 0x418C
    GREG_IOMMU_CFG4         = 0x64,     // 0x4190
    GREG_IOMMU_CFG5         = 0x65,     // 0x4194
    GREG_IOMMU_CFG6         = 0x66,     // 0x4198
    GREG_IOMMU_CFG7         = 0x67,     // 0x419C
    GREG_IOMMU_CFG8         = 0x68,     // 0x41A0
    GREG_IOMMU_CFG9         = 0x69,     // 0x41A4
    GREG_IOMMU_CFG10        = 0x6A,     // 0x41A8
    GREG_IOMMU_CFG11        = 0x6B,     // 0x41AC
    GREG_IOMMU_CFG12        = 0x6C,     // 0x41B0
    GREG_IOMMU_CFG13        = 0x6D,     // 0x41B4
    GREG_LAST               = 0x6E,     // 0x41B8
#else
    GREG_LAST               = 0x50      // 0x4140
    /* 0x4140 ~ 0x4258 are not listed. */
#endif    
} E_GFX_REG_T;


/** gfx register name
 *
 */
typedef struct _MI_DIF_REG_T
{
    uint32_t u4_G_CONFIG;                 // 0x4000
    uint32_t u4_G_STATUS;                 // 0x4004
    uint32_t u4_DRAMQ_STAD;               // 0x4008
    uint32_t u4_DRAMQ_LEN;                // 0x400C
    uint32_t u4_G_MODE;                   // 0x4010
    uint32_t u4_RECT_COLOR;               // 0x4014
    uint32_t u4_SRC_BSAD;                 // 0x4018
    uint32_t u4_DST_BSAD;                 // 0x401C
    uint32_t u4_SRC_XY;                   // 0x4020
    uint32_t u4_DST_XY;                   // 0x4024
    uint32_t u4_SRC_SIZE;                 // 0x4028
    uint32_t u4_S_OSD_WIDTH;              // 0x402C
    uint32_t u4_CLIP_BR;                  // 0x4030
    uint32_t u4_CLIP_TL;                  // 0x4034
    uint32_t u4_GRAD_X_DELTA;             // 0x4038
    uint32_t u4_GRAD_Y_DELTA;             // 0x403C
    uint32_t u4_GRAD_XY_INC;              // 0x4040
    uint32_t u4_BITBLT_MODE;              // 0x4044
    uint32_t u4_KEY_DATA0;                // 0x4048
    uint32_t u4_KEY_DATA1;                // 0x404C
    uint32_t u4_SRCCBCR_BSA;              // 0x4050
    uint32_t u4_SRCCBCR_PITC;             // 0x4054
    uint32_t u4_DSTCBCR_BSA;              // 0x4058
    uint32_t u4_F_COLOR;                  // 0x405C
    uint32_t u4_B_COLOR;                  // 0x4060
    uint32_t u4_COL_TRAN0;                // 0x4064
    uint32_t u4_COL_TRAN1;                // 0x4068
    uint32_t u4_COL_TRAN2;                // 0x406C
    uint32_t u4_COL_TRAN3;                // 0x4070
    uint32_t u4_COL_TRAN4;                // 0x4074
    uint32_t u4_COL_TRAN5;                // 0x4078
    uint32_t u4_COL_TRAN6;                // 0x407C
    uint32_t u4_COL_TRAN7;                // 0x4080
    uint32_t u4_STR_BLT_H;                // 0x4084
    uint32_t u4_STR_BLT_V;                // 0x4088
    uint32_t u4_STR_DST_SIZE;             // 0x408C
    uint32_t u4_LEGAL_START_ADDR;         // 0x4090
    uint32_t u4_LEGAL_END_ADDR;           // 0x4094
    uint32_t u4_DUMMY;                    // 0x4098
    uint32_t u4_ALCOM_LOOP;               // 0x409C
    uint32_t u4_ROP;                      // 0x40A0
    uint32_t u4_IDX2DIR;                  // 0x40A4
    uint32_t u4_SRC_WBBSAD;               // 0x40A8
    uint32_t u4_DST_WBBSAD;               // 0x40AC
    uint32_t u4_SRCCBCR_WBBSAD;           // 0x40B0
    uint32_t u4_Res40B4;                  // 0x40B4
    uint32_t u4_VA_MSB;                   // 0x40B8
    uint32_t u4_RACING_MODE;              // 0x40BC
    uint32_t u4_BPCOMP_CFG;               // 0x40C0
    uint32_t u4_BPCOMP_AD_END;            // 0x40C4
    uint32_t u4_Res40C8;                  // 0x40C8
    uint32_t u4_Res40CC;                  // 0x40CC
    uint32_t u4_Res40D0;                  // 0x40D0
    uint32_t u4_Res40D4;                  // 0x40D4
    uint32_t u4_Res40D8;                  // 0x40D8
    uint32_t u4_Res40DC;                  // 0x40DC
    uint32_t u4_Res40E0;                  // 0x40E0
    uint32_t u4_Res40E4;                  // 0x40E4
    uint32_t u4_Res40E8;                  // 0x40E8
    uint32_t u4_Res40EC;                  // 0x40EC
    uint32_t u4_Res40F0;                  // 0x40F0
    uint32_t u4_Res40F4;                  // 0x40F4
    uint32_t u4_Res40F8;                  // 0x40F8
    uint32_t u4_Res40FC;                  // 0x40FC
    uint32_t u4_Res4100;                  // 0x4100
    uint32_t u4_BMP_STATUS;               // 0x4104
    uint32_t u4_G_256CPT;                 // 0x4108
    uint32_t u4_Res410C;                  // 0x410C
    uint32_t u4_Res4110;                  // 0x4110
    uint32_t u4_Res4114;                  // 0x4114
    uint32_t u4_Res4118;                  // 0x4118
    uint32_t u4_Res411C;                  // 0x411C
    uint32_t u4_Res4120;                  // 0x4120
    uint32_t u4_G_PGIG;                   // 0x4124
    uint32_t u4_BMP_SIZE;                 // 0x4128
    uint32_t u4_GREG_HDMV_IDX_256;        // 0x412C
    uint32_t u4_SRC_RING;                 // 0x4130
    uint32_t u4_DST_RING;                 // 0x4134
    uint32_t u4_CPT_ADDR_RD;              // 0x4138
    uint32_t u4_CPT_DATA_RD;              // 0x413C
#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8560)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    uint32_t u4_0x4140;                   // 0x4140
    uint32_t u4_0x4144;                   // 0x4144
    uint32_t u4_0x4148;                   // 0x4148
    uint32_t u4_0x414C;                   // 0x414C
    uint32_t u4_0x4150;                   // 0x4150
    uint32_t u4_0x4154;                   // 0x4154
    uint32_t u4_0x4158;                   // 0x4158
    uint32_t u4_0x415C;                   // 0x415C
    uint32_t u4_0x4160;                   // 0x4160
    uint32_t u4_0x4164;                   // 0x4164
    uint32_t u4_0x4168;                   // 0x4168
    uint32_t u4_0x416C;                   // 0x416C
    uint32_t u4_0x4170;                   // 0x4170
    uint32_t u4_0x4174;                   // 0x4174
    uint32_t u4_0x4178;                   // 0x4178
    uint32_t u4_0x417C;                   // 0x417C
    uint32_t u4_0x4180;                   // 0x4180
    uint32_t u4_0x4184;                   // 0x4184
    uint32_t u4_0x4188;                   // 0x4188
    uint32_t u4_0x418C;                   // 0x418C
    uint32_t u4_0x4190;                   // 0x4190
    uint32_t u4_0x4194;                   // 0x4194
    uint32_t u4_0x4198;                   // 0x4198
    uint32_t u4_0x419C;                   // 0x419C
    uint32_t u4_0x41A0;                   // 0x41A0
    uint32_t u4_0x41A4;                   // 0x41A4
    uint32_t u4_0x41A8;                   // 0x41A8
    uint32_t u4_0x41AC;                   // 0x41AC
    uint32_t u4_0x41B0;                   // 0x41BC
    uint32_t u4_LAST;                     // 0x41B0
#else
    uint32_t u4_LAST;                     // 0x4140
#endif
} MI_DIF_REG_T;


/** gfx hw register bitfield
 *
 */
typedef struct _MI_DIF_FIELD_T
{
    // DWORD - G_CONFIG         (4000h)
    uint32_t fg_EN_DRAMQ          :  1;
    uint32_t fg_INT_MASK          :  1;
    uint32_t fg_POST_THRS         :  2;
    uint32_t fg_CMDFIFO_THRS      :  2;
    uint32_t fg_REQ_INTVAL        :  2;
    uint32_t fg_DRAMQ_MODE        :  1;
    uint32_t fg_SDFIFO_THRS       :  2;
    uint32_t                      :  5;
    uint32_t fg_SRAM_LP           :  1;
    uint32_t fg_ENG_LP            :  1;
    uint32_t                      :  3;
	uint32_t fg_MMU_RST			:  2;
    uint32_t fg_MMU_CMDQ_EN		:  1;          
    uint32_t fg_DYNAMIC_HIGH_PRIORITY : 1;
    uint32_t fg_SHORT_CMDQ        :  1;
    uint32_t                      :  2;
    uint32_t fg_CQ_RST            :  2;
    uint32_t fg_G_RST             :  2;

    // DWORD - G_STATUS         (4004h)
    uint32_t fg_IDLE              :  1;
    uint32_t fg_HWQ_LEN           :  5;
    uint32_t                      :  2;
    uint32_t fg_VERSION_ID        :  8;
    uint32_t fg_CURR_Y_LINE       : 11;
    uint32_t                      :  5;

    // DWORD - DRAMQ_STAD       (4008h)
    uint32_t fg_DRAMQ_BSAD        : 30;
    uint32_t fg_CYC_SIZE          :  2;

    // DWORD - DRAMQ_LEN        (400Ch)
    uint32_t fg_DRAMQ_LEN         : 18;
    uint32_t                      : 6;
	uint32_t fg_IO_MON_SEL		: 4;
    uint32_t fg_NEW_SW            : 1;
    uint32_t                      : 3;

    // DWORD - G_MODE           (4010h)
    uint32_t fg_CM                :  4;
    uint32_t fg_OP_MODE           :  5;
	uint32_t fg_SRC_CM_24BPP		:  1;
    uint32_t                      :  1;
    uint32_t fg_FIRE              :  1;
    uint32_t fg_BURST_EN          :  1;
    uint32_t fg_BURST_MODE        :  2;
    uint32_t                      :  1;
    uint32_t fg_DSTOWN            :  1;
    uint32_t fg_SRCOWN            :  1;
    uint32_t                      : 2;
    uint32_t  fg_SRC_CM           : 4;
    uint32_t                      : 1;
	uint32_t  fg_MMU_CLK_OFF		: 1;
    uint32_t  fg_STATIC_HIGH_PRIORITY : 1;
    uint32_t  fg_BURST_PROTECT_DIS    : 1;
    uint32_t  fg_DST_YUV_EN           : 1;
    uint32_t  fg_SRC_YUV_EN           : 1;
    uint32_t  fg_DST_WT_EN            : 1;
    uint32_t  fg_SRC_WT_EN            : 1;

    // DWORD - RECT_COLOR       (4014h)
    uint32_t fg_RECT_COLOR        : 32;

    // DWORD - SRC_BSAD         (4018h)
    uint32_t fg_SRC_BSAD          : 30;
    uint32_t fg_CHAR_CM           :  2;

    // DWORD - DST_BSAD         (401Ch)
    uint32_t fg_DST_BSAD          : 30;
    uint32_t                      :  1;
    uint32_t fg_DST_BSAD_WR       :  1;

    // DWORD - SRC_XY           (4020h)
    uint32_t fg_SRCX              : 15;
    uint32_t                      :  1;
    uint32_t fg_SRCY              : 11;
    uint32_t                      :  5;

    // DWORD - DST_XY           (4024h)
    uint32_t fg_DSTX              : 15;
    uint32_t                      :  1;
    uint32_t fg_DSTY              : 11;
    uint32_t                      :  5;

    // DWORD - SRC_SIZE         (4028h)  
    uint32_t fg_SRC_WIDTH         : 15;
    uint32_t fg_RL_DEC            :  1;
#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    uint32_t fg_SRC_HEIGHT        : 11;
    uint32_t fg_SRC_CM_2            : 4; //Source color mode selection flag
                                    // if new_sw: 0=> SRC_CM[3:0], 1=>SRC_HEIGHT[13:11]
    uint32_t fg_SRC_HEIGHT_15     : 1; // if new_sw: 0=> SRC_HEIGHT[15], 1=>no used
#else
    uint32_t fg_SRC_HEIGHT        : 16;
#endif

    // DWORD - S_OSD_WIDTH      (402Ch)
    uint32_t fg_OSD_WIDTH         : 16;
    uint32_t fg_SRC_PITCH         : 16;

    // DWORD - CLIP_BR          (4030h)
    uint32_t fg_CLIP_RIGHT        : 15;
    uint32_t fg_CLR_ENA           :  1;
    uint32_t fg_CLIP_BOT          : 14;
    uint32_t                      :  1;
    uint32_t fg_CLB_ENA           :  1;

    // DWORD - CLIP_TL          (4034h)
    uint32_t fg_CLIP_LEFT         : 15;
    uint32_t fg_CLL_ENA           :  1;
    uint32_t fg_CLIP_TOP          : 14;
    uint32_t                      :  1;
    uint32_t fg_CLT_ENA           :  1;

    // DWORD - GRAD_X_DELTA     (4038h)
    uint32_t fg_DELTA_X_C0        :  8;
    uint32_t fg_DELTA_X_C1        :  8;
    uint32_t fg_DELTA_X_C2        :  8;
    uint32_t fg_DELTA_X_C3        :  8;

    // DWORD - GRAD_Y_DELTA     (403Ch)
    uint32_t fg_DELTA_Y_C0        :  8;
    uint32_t fg_DELTA_Y_C1        :  8;
    uint32_t fg_DELTA_Y_C2        :  8;
    uint32_t fg_DELTA_Y_C3        :  8;

    // DWORD - GRAD_XY_INC      (4040h)
    uint32_t fg_GRAD_X_PIX_INC    : 11;
    uint32_t                      :  5;
    uint32_t fg_GRAD_Y_PIX_INC    : 11;
    uint32_t                      :  3;
    uint32_t fg_GRAD_MODE         :  2;

    // DWORD - BITBLT_MODE      (4044h)
    uint32_t fg_TRANS_ENA         :  1;
    uint32_t fg_KEYNOT_ENA        :  1;
    uint32_t fg_COLCHG_ENA        :  1;
#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    uint32_t fg_BITBLT_CLIP_ENA   :  1;
#else
    uint32_t                      :  1; 
#endif
    uint32_t fg_CFMT_ENA          :  1;
    uint32_t fg_KEYSDSEL          :  1;
    uint32_t fg_PRCN_OPT          :  1;
    uint32_t fg_CLIP_ENA          :  1;
    uint32_t fg_ALPHA_VALUE       :  8;
    uint32_t fg_ALCOM_PASS        :  3;
	uint32_t fg_SRC_PREMULT       :  1;
#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8560)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
#if 0
	
	uint32_t						:  3;
	uint32_t fg_DIFF_CM			:  1;
#else
    uint32_t fg_DST_KEY_IN        :  1;
    uint32_t fg_DIS_DST_KEY       :  1;
    uint32_t fg_SRC_KEY_IN        :  1;
    uint32_t fg_DIS_SRC_KEY       :  1;
#endif
#else
    uint32_t                      :  5;
#endif
    uint32_t fg_DSTPITCH_DEC      :  1;
    uint32_t fg_DST_MIRR_OR    :  1;
    uint32_t fg_SRCPITCH_DEC   :  1;
    uint32_t fg_SRC_MIRR_OR    :  1;
#if 0 //(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)
    uint32_t fg_DST_WR_ROTATE  : 1;
    uint32_t fg_DST_RD_ROTATE  : 1;
#else
    uint32_t fg_DST_ROTATE        : 1;
    uint32_t                      : 1;
#endif
    uint32_t  fg_BARREL_ENA       :  1;
    uint32_t  fg_PASS_ALU_ENA     :  1;

    // DWORD - KEY_DATA0        (4048h)
    uint32_t fg_COLOR_KEY_MIN     : 32;

    // DWORD - KEY_DATA1        (404Ch)
    uint32_t fg_COLOR_KEY_MAX     : 32;

    // DWORD - SRCCBCR_BSA      (4050h)
    uint32_t fg_SRCCBCR_BSAD      : 30;
    uint32_t fg_YC_FMT            :  2;

    // DWORD - SRCCBCR_PITCH    (4054h)
    uint32_t fg_SRCCBCR_PITCH     : 16;
    uint32_t fg_VSTD              :  1;
    uint32_t fg_VSYS              :  1;
    uint32_t fg_VSCLIP            :  1;
    uint32_t fg_FLD_PIC           :  1;
    uint32_t fg_SWAP_MODE         :  2;
#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    uint32_t fg_SWAP_NEW_MODE     : 2;
    uint32_t fg_SRC2_BSAD_ENA     : 1;
    uint32_t                      : 3;
    uint32_t fg_SRC2_CM           : 4;
#else
    uint32_t                      : 10;
#endif

    // DWORD - DSTCBCR_BSAD     (4058h)
    uint32_t fg_DSTCBCR_BSAD      : 28;
    uint32_t                      :  4;

    // DWORD - F_COLOR (1)      (405Ch)
    uint32_t fg_FORE_COLOR        : 32;

    // DWORD - B_COLOR (0)      (4060h)
    uint32_t fg_BACK_COLOR        : 32;

    // DWORD - COL_TRAN0        (4064h)
    uint32_t fg_COLOR_TRANS0      : 32;

    // DWORD - COL_TRAN1        (4068h)
    uint32_t fg_COLOR_TRANS1      : 32;

    // DWORD - COL_TRAN2        (406Ch)
    uint32_t fg_COLOR_TRANS2      : 32;

    // DWORD - COL_TRAN3        (4070h)
    uint32_t fg_COLOR_TRANS3      : 32;

    // DWORD - COL_TRAN4        (4074h)
    uint32_t fg_COLOR_TRANS4      : 32;

    // DWORD - COL_TRAN5        (4078h)
    uint32_t fg_COLOR_TRANS5      : 32;

    // DWORD - COL_TRAN6        (407Ch)
    uint32_t fg_COLOR_TRANS6      : 32;

    // DWORD - COL_TRAN7        (4080h)
    uint32_t fg_COLOR_TRANS7      : 32;

    // DWORD - STR_BLT_H        (4084h)
    uint32_t fg_STR_BLT_H         : 24;
    uint32_t                      :  8;

    // DWORD - STR_BLT_V        (4088h)
    uint32_t fg_STR_BLT_V         : 24;
    uint32_t                      :  8;

    // DWORD - STR_DST_SIZE     (408Ch)
    uint32_t fg_STR_DST_WIDTH     : 15;
    uint32_t                      :  1;
    uint32_t fg_STR_DST_HEIGHT    : 11;
    uint32_t                      :  5;

    // DWORD - LEGAL_START_ADDR (4090h) default value:0x80000000
    uint32_t fg_LEGAL_AD_START    : 30;
    uint32_t                      :  1;
    uint32_t fg_WR_PROT_EN        :  1;

    // DWORD - LEGAL_END_ADDR   (4094h)  default value:0x3FFFFFFF
    uint32_t fg_LEGAL_AD_END      : 30;
    uint32_t                      :  2;

    // DWORD - DUMMY            (4098h)
    uint32_t fg_DUMMY             : 32;

    // DWORD - ALCOM_LOOP       (409C)
    uint32_t fg_ALCOM_AR          : 8;
    uint32_t fg_ALCOM_OPCODE      : 4;
    uint32_t                      : 4;
    uint32_t fg_ALCOM_RECT_SRC    : 1;
    uint32_t fg_ALCOM_NORMAL      : 1;
    uint32_t fg_PRE2NONPREMUTLI_ENA : 1;
    uint32_t fg_NONPRE2PREMUTLI_ENA : 1;
    uint32_t fg_SRC_OVERFLOW_ENA  : 1;
    uint32_t                      : 11;

    // DWORD - ROP              (40A0)
    uint32_t fg_ROP_OPCODE        : 4;
#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    uint32_t fg_SRCALPHA_CHECK    : 1; /*JavaXor-default : 1 -- else default 0*/
    uint32_t                      : 3;
#else
    uint32_t                      : 4;
#endif
    uint32_t fg_NO_WR             : 1;
    uint32_t fg_CMP_FLAG          : 1;
    uint32_t                      : 6; /**/
    uint32_t fg_YUVRGB_MODE       : 3;
#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8561)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    uint32_t                      : 11;
    uint32_t fg_COLORIZE_REP      : 1;
    uint32_t fg_PRE_COLORIZE      : 1;
#else
    uint32_t                      : 13;
#endif

    // DWORD - IDX2DIR          (40A4)
    uint32_t fg_PAL_BSAD          : 30;
    uint32_t fg_MSB_LEFT          : 1;
    uint32_t fg_LN_ST_BYTE_AL     : 1;

    // DWORD - SRC_WBBSAD       (40A8)
    uint32_t fg_SRC_WBBSAD        : 30;
    uint32_t                      : 2;

    // DWORD - DST_WBBSAD       (40AC)
    uint32_t fg_DST_WBBSAD        : 30;
    uint32_t                      : 2;

    // DWORD - SRCCBCR_WBBSAD   (40B0)
    uint32_t fg_SRCCBCR_WBBSAD    : 30;
    uint32_t                      : 2;

    // DWORD - XOR COLOR        (40B4)
    /** 8560:
     * ROP mode:3: xor color in set XOR mode.
     * ROP mode:2: [23:16]-Ar, [15:8]-Ag, [7:0]-Ab
     */
    uint32_t fg_JAVA_XOR_COLOR    : 32;

#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8560)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)

	#if 0
    // DWORD - DRAMQ_STADMSB -- Virtual Address MSB  (40B8)
    uint32_t                      : 6;
    uint32_t fg_INDEX_BASD_H      : 2;
    uint32_t fg_BPCOMP_AD_END_H   : 2;
    uint32_t fg_SRCCBCR_WBBSAD_H  : 2;
    uint32_t fg_DST_WBBSAD_H      : 2;
    uint32_t fg_SRC_WBBSAD_H      : 2;
    uint32_t fg_PAL_BSAD_H        : 2;
    uint32_t fg_DSTCBCR_BSAD_H    : 2;
    uint32_t fg_SRCCBCR_BSAD_H    : 2;
    uint32_t fg_DST_BSAD_H        : 2;
    uint32_t fg_SRC_BSAD_H        : 2;
    uint32_t fg_LEGAL_AD_END_H    : 2;
    uint32_t fg_LEGAL_AD_START_H  : 2;
    uint32_t fg_DRAMQ_BSAD_H      : 2;
	#else
	uint32_t fg_Res40B8           : 32;
	#endif
	
    // DWORD - RACING_MODE_REGISTER  (40BC)
    uint32_t                              : 18;
    uint32_t fg_RACING_MODE_TEST          : 1;
    uint32_t fg_OSD3_RIGHT_FLIP_TEST      : 1;
    uint32_t fg_OSD3_LEFT_FLIP_TEST       : 1;
    uint32_t fg_OSD2_RIGHT_FLIP_TEST      : 1;
    uint32_t fg_OSD2_LEFT_FLIP_TEST       : 1;
    uint32_t fg_OSD3_NEXT_RIGHT_DRAW_CMD  : 1;
    uint32_t fg_OSD3_NEXT_LEFT_DRAW_CMD   : 1;
    uint32_t fg_OSD2_NEXT_RIGHT_DRAW_CMD  : 1;
    uint32_t fg_OSD2_NEXT_LEFT_DRAW_CMD   : 1;
    uint32_t fg_OSD3_RACING_RIGHT_COMP_EN : 1;
    uint32_t fg_OSD3_RACING_LEFT_COMP_EN  : 1;
    uint32_t fg_OSD2_RACING_RIGHT_COMP_EN : 1;
    uint32_t fg_OSD2_RACING_LEFT_COMP_EN  : 1;
    uint32_t fg_RACING_EN                 : 1;
#else
    uint32_t fg_Res40B8           : 32;
    uint32_t fg_Res40BC           : 32;
#endif

    // DWORD - BPCOMP_CFG       (40C0)
    uint32_t fg_ROLL_BACK_EN      : 1;
    uint32_t fg_QUALITY_MODE      : 2;
    uint32_t fg_LINE_SEPRATE      : 1;
#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8560)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    uint32_t fg_PIXEL_SEPARATE    : 1;
    uint32_t fg_DEC_ENA           : 1;
    uint32_t fg_DEC_MODE          : 2;
    uint32_t                      : 24;
#elif 0//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)
    uint32_t fg_PIXEL_SEPARATE    : 1;
    uint32_t fg_DEC_ENA           : 1;
    uint32_t fg_MASK_DEC_ENA      : 1;
    uint32_t fg_OFFSET_DEC_ENA    : 1;
    uint32_t                      : 24;
#else
    uint32_t fg_Res40C0           : 28;
#endif

    // DWORD - BPCOMP_AD_END    (40C4)    
    uint32_t fg_BPCOMP_AD_END     : 30;
    uint32_t fg_Res40C4           :  2;
    
    // DWORD - BPCOMP_DBG1      (40C8)    
    uint32_t fg_BPCOMP_NIPPLE     : 26;
    uint32_t                      : 5;
    uint32_t fg_BPCOMP_STPP       : 1;

    // DWORD - BPCOMP_DBG2      (40CC)
    uint32_t fg_BPCOMP_CHKSUM      : 32;

    // DWORD - BPCOMP_DBG3_0    (40D0)
    uint32_t fg_BPCOMP_STOP_ID0    : 32;

    // DWORD - BPCOMP_DBG3_1    (40D4)
    uint32_t fg_BPCOMP_STOP_ID1   : 32;

    // DWORD - BPCOMP_DBG3_2    (40D8)
    uint32_t fg_BPCOMP_STOP_ID2   : 32;

    // DWORD - BPCOMP_DBG3_3    (40DC)
    uint32_t fg_BPCOMP_STOP_ID3   : 32;

#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    // BPCOMP_PACKET            (40E0)
    uint32_t fg_PACKET_WIDTH      : 16;
    uint32_t fg_PIXEL_WIDTH       : 8;
    uint32_t                      : 8;
    // BPCOMP_OFFSET1           (40E4)
    uint32_t fg_PACKET_ACTIVE_WIDTH : 15;
    uint32_t                        : 1;
    uint32_t fg_PACKET_X_START      : 15;
    uint32_t                        : 1;
    // BPCOMP_OFFSET2           (40E8)
    //uint32_t fg_LINE_WIDTH        : 15;
    //uint32_t                      : 17;
    uint32_t fg_DST_COLOR_KEY_MIN :32;
    // BPCOMP_INDEX_BASD        (40EC)
    //uint32_t fg_INDEX_BASD        : 30;
    //uint32_t                      : 2;
    uint32_t fg_DST_COLOR_KEY_MAX :32;
#else
    uint32_t fg_Res40E0           : 32;
    uint32_t fg_Res40E4           : 32;
    uint32_t fg_Res40E8           : 32;
    uint32_t fg_Res40EC           : 32;
#endif

	#if 0
    uint32_t fg_Res40F0           : 32;
	#else
	// DWORD - DRAMQ_STADMSB -- Virtual Address MSB  (40F0)
    uint32_t                      : 6;
    uint32_t fg_INDEX_BASD_H      : 2;
    uint32_t fg_SRC_24BPP_BSAD_H   : 2;
    uint32_t fg_SRCCBCR_WBBSAD_H  : 2;
    uint32_t fg_DST_WBBSAD_H      : 2;
    uint32_t fg_SRC_WBBSAD_H      : 2;
    uint32_t fg_PAL_BSAD_H        : 2;
    uint32_t fg_DSTCBCR_BSAD_H    : 2;
    uint32_t fg_SRCCBCR_BSAD_H    : 2;
    uint32_t fg_DST_BSAD_H        : 2;
    uint32_t fg_SRC_BSAD_H        : 2;
    uint32_t fg_LEGAL_AD_END_H    : 2;
    uint32_t fg_LEGAL_AD_START_H  : 2;
    uint32_t fg_DRAMQ_BSAD_H      : 2;
	#endif
    uint32_t fg_Res40F4           : 32;
    uint32_t fg_Res40F8           : 32;
    uint32_t fg_Res40FC           : 32;

    uint32_t fg_Res4100           : 32;

    // DWORD - BMP_STATUSDX2DIR (4104)
#if 1//(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8561) || (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8563)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    uint32_t fg_MULTREG_ENA       : 1;
    uint32_t fg_Res4104           : 31;
#else
    uint32_t fg_Res4104           : 32;
#endif

    // DWRD G_256CPT            (4108)
    uint32_t fg_256CPT_FIRE       : 1;
    uint32_t fg_USE_256CPT        : 1;
    uint32_t fg_CPT_DRAM_EN       : 1;
    uint32_t                      : 3;
    uint32_t fg_ALU_ENB           : 1;
    uint32_t fg_IDX2DIR_EN        : 1;
    uint32_t                      : 11;
    uint32_t fg_SRC_PITCH_ENA     : 1;
    uint32_t                      : 2;
	uint32_t fg_GLOABLE_ALPHA_EN  : 1;
    uint32_t fg_ALPHA_EN          : 1;
	uint32_t fg_ROUNDING_1555     : 1;
	uint32_t fg_PRCN_OPTIONG_1555 : 1;
	uint32_t fg_PRCN_KEY_1555     : 1;
	uint32_t fg_ROUNDING_565      : 1;
	uint32_t fg_ROUNDING_4444     : 1;
	uint32_t						: 3;

	
    uint32_t fg_Res410C           : 32;
    uint32_t fg_Res4110           : 32;
    uint32_t fg_Res4114           : 32;
    uint32_t fg_Res4118           : 32;
    uint32_t fg_Res411C           : 32;
    uint32_t fg_Res4120           : 32;
	
    // DWRD G_PGIG              (4124)
#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    uint32_t fg_SRC_HEIGHT_19_16  : 4;
    uint32_t                      : 12;
#else
    uint32_t                      : 16;
#endif
    uint32_t fg_YR_SEL            : 2;
    uint32_t fg_UG_SEL            : 2;
    uint32_t fg_VB_SEL            : 2;
    uint32_t fg_A_SEL             : 2;
    uint32_t                      : 8;
	
    uint32_t fg_Res4128           : 32;


    // DWRD HDMV_IDX_256        (412C)
    uint32_t fg_DMV_IDX_256       : 32;

    uint32_t fg_Res4130           : 32;

    //                          (4134)

    uint32_t fg_Res4134         : 32;

    // DWRD CPT_ADDR_RD         (4138)
    uint32_t fg_CPT_RD_ADDR       : 8;
    uint32_t                      : 8;
    uint32_t fg_CPT_RD_EN         : 1;
    uint32_t                      : 15;

    // DWRD CPT_DATA_RD         (413C)
    uint32_t fg_CPT_DATA_RD       : 32;

#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8560)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    uint32_t fg_Reg4140           : 32;
    uint32_t fg_Reg4144           : 32;
    uint32_t fg_Reg4148           : 32;
    uint32_t fg_Reg414C           : 32;
    uint32_t fg_Reg4150           : 32;
    //uint32_t fg_Reg4154           : 32;
    //DWORD --24bpp src bsad		    (4154)	
    uint32_t fg_SRC_BSAD_24BPP    : 30;
    uint32_t 				        :  2;
    
    //uint32_t fg_Reg4158           : 32;
    //DWORD --24bpp SRC_SIZE          (4158)
    uint32_t fg_SRC_WIDTH_24BPP   : 15;
	uint32_t 						: 1;
    uint32_t fg_SRC_HEIGHT_24BPP  : 14;
	uint32_t						: 2;

    uint32_t fg_Reg415C           : 32;
    uint32_t fg_Reg4160           : 32;
    uint32_t fg_Reg4164           : 32;
    uint32_t fg_Reg4168           : 32;
    uint32_t fg_Reg416C           : 32;
    uint32_t fg_Reg4170           : 32;
    uint32_t fg_Reg4174           : 32;
    uint32_t fg_Reg4178           : 32;
    uint32_t fg_Reg417C           : 32;

    // mmu mapping register (5f040 ~ 5f06c, 5f070)
    // DWRD  IOMMU_CFG0         (4180)
    uint32_t fg_IOMMU_EN          : 1;
    uint32_t fg_TLBLAST           : 1;
    uint32_t fg_AUTODISCARD       : 1;
    uint32_t fg_MIF_ENTRY         : 1;
    uint32_t fg_SMT_RESZ          : 1;
    uint32_t fg_CFG0_BIT5         : 1;
    uint32_t fg_CFG0_BIT6         : 1;
    uint32_t fg_CFG0_BIT7         : 1;
    uint32_t fg_Reg4180           : 24;

    // DWRD IOMMU_CFG1          (4184)
    uint32_t fg_RG_TTB            : 32;

    // DWRD IOMMU_CFG2          (4188)
    uint32_t fg_A0_MMU_EN         : 1;
    uint32_t fg_A0_MID            : 3;
    uint32_t fg_A0_PREFETCH       : 1;
    uint32_t fg_A0_2D_PREFETCH    : 1;
    uint32_t fg_A0_PREFETCH_DEC   : 1;
    uint32_t                      : 1;
    uint32_t fg_A0_TWO_WAY        : 1;
    uint32_t fg_A0_MID_2ND        : 3;
    uint32_t                      : 4;
    uint32_t fg_A1_MMU_EN         : 1;
    uint32_t fg_A1_MID            : 3;
    uint32_t fg_A1_PREFETCH       : 1;
    uint32_t fg_A1_2D_PREFETCH    : 1;
    uint32_t fg_A1_PREFETCH_DEC   : 1;
    uint32_t                      : 1;
    uint32_t fg_A1_TWO_WAY        : 1;
    uint32_t fg_A1_MID_2ND        : 3;
    uint32_t                      : 4;

    // DWORD - IOMMU_CFG3       (418ch)
    uint32_t fg_A2_MMU_EN         : 1;
    uint32_t fg_A2_MID            : 3;
    uint32_t fg_A2_PREFETCH       : 1;
    uint32_t fg_A2_2D_PREFETCH    : 1;
    uint32_t fg_A2_PREFETCH_DEC   : 1;
    uint32_t                      : 1;
    uint32_t fg_A2_TWO_WAY        : 1;
    uint32_t fg_A2_MID_2ND        : 3;
    uint32_t                      : 4;
    uint32_t fg_A3_MMU_EN         : 1;
    uint32_t fg_A3_MID            : 3;
    uint32_t fg_A3_PREFETCH       : 1;
    uint32_t fg_A3_2D_PREFETCH    : 1;
    uint32_t fg_A3_PREFETCH_DEC   : 1;
    uint32_t                      : 1;
    uint32_t fg_A3_TWO_WAY        : 1;
    uint32_t fg_A3_MID_2ND        : 3;
    uint32_t                      : 4;

    // DWORD - IOMMU_CFG4       (4190h)
    uint32_t fg_IOMMU_CFG4        : 31;
    uint32_t fg_IOMMU_FIRE        :  1;

    // DWORD - IOMMU_CFG5       (4194h)
    uint32_t fg_IOMMU_CFG5        : 32;

    // DWORD - IOMMU_CFG6       (4198h)
    uint32_t fg_IOMMU_CFG6        : 32;

    // DWORD - IOMMU_CFG7       (419ch)
    uint32_t fg_IOMMU_CFG7        : 32;

    // DWORD - IOMMU_CFG8       (41a0h)
    uint32_t fg_IOMMU_CFG8        : 32;    

    // DWORD - IOMMU_CFG9       (41a4h)
    uint32_t fg_IOMMU_CFG9        : 32;

    // DWORD - IOMMU_CFG10      (41a8h)
    uint32_t fg_IOMMU_CFG10       : 32;

    // DWORD - IOMMU_CFG11       (41a4h)
    uint32_t fg_IOMMU_CFG11       : 32;
    
    // DWORD - IOMMU_CFG12      (41a8h)
    uint32_t fg_IOMMU_CFG12       : 32;

    // DWORD - IOMMU_CFG13      (41ach)
    uint32_t fg_IOMMU_CFG13       : 32;
	
    // DWORD - LAST            (41b0h)
    uint32_t fg_LAST              : 32;
#else
    // DWORD - LAST            (4140h)
    uint32_t fg_LAST              : 32;
#endif
} MI_DIF_FIELD_T;


/** gfx hw register name/bitfield union
 *
 */
typedef union _MI_DIF_UNION_T
{
    MI_DIF_REG_T   rReg;
    MI_DIF_FIELD_T rField;
} MI_DIF_UNION_T;

typedef enum _E_GFX_LOSSLESS_REG_T
{
    GREG_REGION_H_0 = 0x0,  // 4300
    GREG_REGION_H_1,
    GREG_REGION_H_2,
    GREG_REGION_H_3,
    GREG_REGION_H_4,
    GREG_REGION_H_5,
    GREG_REGION_H_6,
    GREG_REGION_H_7,
    GREG_REGION_H_8,
    GREG_REGION_H_9,
    GREG_REGION_H_10,
    GREG_REGION_H_11,
    GREG_REGION_H_12,
    GREG_REGION_H_13,
    GREG_REGION_H_14,
    GREG_REGION_H_15,
    GREG_REGION_H_16,
    GREG_REGION_H_17,
    GREG_REGION_H_18,
    GREG_REGION_H_19,
    GREG_REGION_H_20,
    GREG_REGION_H_21,
    GREG_REGION_H_22,
    GREG_REGION_H_23,
    GREG_REGION_H_24,
    GREG_REGION_H_25,
    GREG_REGION_H_26,
    GREG_REGION_H_27,
    GREG_REGION_H_28,
    GREG_REGION_H_29,
    GREG_REGION_H_30,
    GREG_REGION_H_31,
    GREG_REGION_ADDR_0,   // 4380
    GREG_REGION_ADDR_1,
    GREG_REGION_ADDR_2,
    GREG_REGION_ADDR_3,
    GREG_REGION_ADDR_4,
    GREG_REGION_ADDR_5,
    GREG_REGION_ADDR_6,
    GREG_REGION_ADDR_7,
    GREG_REGION_ADDR_8,
    GREG_REGION_ADDR_9,
    GREG_REGION_ADDR_10,
    GREG_REGION_ADDR_11,
    GREG_REGION_ADDR_12,
    GREG_REGION_ADDR_13,
    GREG_REGION_ADDR_14,
    GREG_REGION_ADDR_15,
    GREG_REGION_ADDR_16,
    GREG_REGION_ADDR_17,
    GREG_REGION_ADDR_18,
    GREG_REGION_ADDR_19,
    GREG_REGION_ADDR_20,
    GREG_REGION_ADDR_21,
    GREG_REGION_ADDR_22,
    GREG_REGION_ADDR_23,
    GREG_REGION_ADDR_24,
    GREG_REGION_ADDR_25,
    GREG_REGION_ADDR_26,
    GREG_REGION_ADDR_27,
    GREG_REGION_ADDR_28,
    GREG_REGION_ADDR_29,
    GREG_REGION_ADDR_30,
    GREG_REGION_ADDR_31,
    GREG_REGION_ADDR_32,
    GREG_REGION_ADDR_33,
    GREG_REGION_ADDR_34,
    GREG_REGION_ADDR_35,
    GREG_REGION_ADDR_36,
    GREG_REGION_ADDR_37,
    GREG_REGION_ADDR_38,
    GREG_REGION_ADDR_39,
    GREG_REGION_ADDR_40,
    GREG_REGION_ADDR_41,
    GREG_REGION_ADDR_42,
    GREG_REGION_ADDR_43,
    GREG_REGION_ADDR_44,
    GREG_REGION_ADDR_45,
    GREG_REGION_ADDR_46,
    GREG_REGION_ADDR_47,
    GREG_REGION_ADDR_48,
    GREG_REGION_ADDR_49,
    GREG_REGION_ADDR_50,
    GREG_REGION_ADDR_51,
    GREG_REGION_ADDR_52,
    GREG_REGION_ADDR_53,
    GREG_REGION_ADDR_54,
    GREG_REGION_ADDR_55,
    GREG_REGION_ADDR_56,
    GREG_REGION_ADDR_57,
    GREG_REGION_ADDR_58,
    GREG_REGION_ADDR_59,
    GREG_REGION_ADDR_60,
    GREG_REGION_ADDR_61,
    GREG_REGION_ADDR_62,
    GREG_REGION_ADDR_63,
    GREG_REGION_ENA_1,     // 4480
    GREG_REGION_ENA_2,     // 4484
    GREG_LL_LAST
} E_GFX_LOSSLESS_REG_T;

typedef struct _MI_DIF_LOSSLESS_REG_T
{
    uint32_t u4_REGION_H_0 ;  // 4300
    uint32_t u4_REGION_H_1 ;
    uint32_t u4_REGION_H_2 ;
    uint32_t u4_REGION_H_3 ;
    uint32_t u4_REGION_H_4 ;
    uint32_t u4_REGION_H_5 ;
    uint32_t u4_REGION_H_6 ;
    uint32_t u4_REGION_H_7 ;
    uint32_t u4_REGION_H_8 ;
    uint32_t u4_REGION_H_9 ;
    uint32_t u4_REGION_H_10;
    uint32_t u4_REGION_H_11;
    uint32_t u4_REGION_H_12;
    uint32_t u4_REGION_H_13;
    uint32_t u4_REGION_H_14;
    uint32_t u4_REGION_H_15;
    uint32_t u4_REGION_H_16;
    uint32_t u4_REGION_H_17;
    uint32_t u4_REGION_H_18;
    uint32_t u4_REGION_H_19;
    uint32_t u4_REGION_H_20;
    uint32_t u4_REGION_H_21;
    uint32_t u4_REGION_H_22;
    uint32_t u4_REGION_H_23;
    uint32_t u4_REGION_H_24;
    uint32_t u4_REGION_H_25;
    uint32_t u4_REGION_H_26;
    uint32_t u4_REGION_H_27;
    uint32_t u4_REGION_H_28;
    uint32_t u4_REGION_H_29;
    uint32_t u4_REGION_H_30;
    uint32_t u4_REGION_H_31;
    uint32_t u4_REGION_ADDR_0 ;   // 4380
    uint32_t u4_REGION_ADDR_1 ;
    uint32_t u4_REGION_ADDR_2 ;
    uint32_t u4_REGION_ADDR_3 ;
    uint32_t u4_REGION_ADDR_4 ;
    uint32_t u4_REGION_ADDR_5 ;
    uint32_t u4_REGION_ADDR_6 ;
    uint32_t u4_REGION_ADDR_7 ;
    uint32_t u4_REGION_ADDR_8 ;
    uint32_t u4_REGION_ADDR_9 ;
    uint32_t u4_REGION_ADDR_10;
    uint32_t u4_REGION_ADDR_11;
    uint32_t u4_REGION_ADDR_12;
    uint32_t u4_REGION_ADDR_13;
    uint32_t u4_REGION_ADDR_14;
    uint32_t u4_REGION_ADDR_15;
    uint32_t u4_REGION_ADDR_16;
    uint32_t u4_REGION_ADDR_17;
    uint32_t u4_REGION_ADDR_18;
    uint32_t u4_REGION_ADDR_19;
    uint32_t u4_REGION_ADDR_20;
    uint32_t u4_REGION_ADDR_21;
    uint32_t u4_REGION_ADDR_22;
    uint32_t u4_REGION_ADDR_23;
    uint32_t u4_REGION_ADDR_24;
    uint32_t u4_REGION_ADDR_25;
    uint32_t u4_REGION_ADDR_26;
    uint32_t u4_REGION_ADDR_27;
    uint32_t u4_REGION_ADDR_28;
    uint32_t u4_REGION_ADDR_29;
    uint32_t u4_REGION_ADDR_30;
    uint32_t u4_REGION_ADDR_31;
    uint32_t u4_REGION_ADDR_32;
    uint32_t u4_REGION_ADDR_33;
    uint32_t u4_REGION_ADDR_34;
    uint32_t u4_REGION_ADDR_35;
    uint32_t u4_REGION_ADDR_36;
    uint32_t u4_REGION_ADDR_37;
    uint32_t u4_REGION_ADDR_38;
    uint32_t u4_REGION_ADDR_39;
    uint32_t u4_REGION_ADDR_40;
    uint32_t u4_REGION_ADDR_41;
    uint32_t u4_REGION_ADDR_42;
    uint32_t u4_REGION_ADDR_43;
    uint32_t u4_REGION_ADDR_44;
    uint32_t u4_REGION_ADDR_45;
    uint32_t u4_REGION_ADDR_46;
    uint32_t u4_REGION_ADDR_47;
    uint32_t u4_REGION_ADDR_48;
    uint32_t u4_REGION_ADDR_49;
    uint32_t u4_REGION_ADDR_50;
    uint32_t u4_REGION_ADDR_51;
    uint32_t u4_REGION_ADDR_52;
    uint32_t u4_REGION_ADDR_53;
    uint32_t u4_REGION_ADDR_54;
    uint32_t u4_REGION_ADDR_55;
    uint32_t u4_REGION_ADDR_56;
    uint32_t u4_REGION_ADDR_57;
    uint32_t u4_REGION_ADDR_58;
    uint32_t u4_REGION_ADDR_59;
    uint32_t u4_REGION_ADDR_60;
    uint32_t u4_REGION_ADDR_61;
    uint32_t u4_REGION_ADDR_62;
    uint32_t u4_REGION_ADDR_63;
    uint32_t u4_REGION_ENA_1;     // 4480
    uint32_t u4_REGION_ENA_2;     // 4484
    uint32_t u4_LL_LAST;          // 4488
} MI_DIF_LOSSLESS_REG_T;

typedef struct _MI_DIF_LOSSLESS_FIELD_T
{
    // DWRD region height         (4300h ~ 437ch)
    uint32_t fg_REGION_H_0        : 16;
    uint32_t fg_REGION_H_1        : 16;
    uint32_t fg_REGION_H_2        : 16;
    uint32_t fg_REGION_H_3        : 16;
    uint32_t fg_REGION_H_4        : 16;
    uint32_t fg_REGION_H_5        : 16;
    uint32_t fg_REGION_H_6        : 16;
    uint32_t fg_REGION_H_7        : 16;
    uint32_t fg_REGION_H_8        : 16;
    uint32_t fg_REGION_H_9        : 16;
    uint32_t fg_REGION_H_10       : 16;
    uint32_t fg_REGION_H_11       : 16;
    uint32_t fg_REGION_H_12       : 16;
    uint32_t fg_REGION_H_13       : 16;
    uint32_t fg_REGION_H_14       : 16;
    uint32_t fg_REGION_H_15       : 16;
    uint32_t fg_REGION_H_16       : 16;
    uint32_t fg_REGION_H_17       : 16;
    uint32_t fg_REGION_H_18       : 16;
    uint32_t fg_REGION_H_19       : 16;
    uint32_t fg_REGION_H_20       : 16;
    uint32_t fg_REGION_H_21       : 16;
    uint32_t fg_REGION_H_22       : 16;
    uint32_t fg_REGION_H_23       : 16;
    uint32_t fg_REGION_H_24       : 16;
    uint32_t fg_REGION_H_25       : 16;
    uint32_t fg_REGION_H_26       : 16;
    uint32_t fg_REGION_H_27       : 16;
    uint32_t fg_REGION_H_28       : 16;
    uint32_t fg_REGION_H_29       : 16;
    uint32_t fg_REGION_H_30       : 16;
    uint32_t fg_REGION_H_31       : 16;
    uint32_t fg_REGION_H_32       : 16;
    uint32_t fg_REGION_H_33       : 16;
    uint32_t fg_REGION_H_34       : 16;
    uint32_t fg_REGION_H_35       : 16;
    uint32_t fg_REGION_H_36       : 16;
    uint32_t fg_REGION_H_37       : 16;
    uint32_t fg_REGION_H_38       : 16;
    uint32_t fg_REGION_H_39       : 16;
    uint32_t fg_REGION_H_40       : 16;
    uint32_t fg_REGION_H_41       : 16;
    uint32_t fg_REGION_H_42       : 16;
    uint32_t fg_REGION_H_43       : 16;
    uint32_t fg_REGION_H_44       : 16;
    uint32_t fg_REGION_H_45       : 16;
    uint32_t fg_REGION_H_46       : 16;
    uint32_t fg_REGION_H_47       : 16;
    uint32_t fg_REGION_H_48       : 16;
    uint32_t fg_REGION_H_49       : 16;
    uint32_t fg_REGION_H_50       : 16;
    uint32_t fg_REGION_H_51       : 16;
    uint32_t fg_REGION_H_52       : 16;
    uint32_t fg_REGION_H_53       : 16;
    uint32_t fg_REGION_H_54       : 16;
    uint32_t fg_REGION_H_55       : 16;
    uint32_t fg_REGION_H_56       : 16;
    uint32_t fg_REGION_H_57       : 16;
    uint32_t fg_REGION_H_58       : 16;
    uint32_t fg_REGION_H_59       : 16;
    uint32_t fg_REGION_H_60       : 16;
    uint32_t fg_REGION_H_61       : 16;
    uint32_t fg_REGION_H_62       : 16;
    uint32_t fg_REGION_H_63       : 16;

    // DWRD region end address         (4380h ~ 447ch)
    uint32_t fg_REGION_ADDR_0        : 32;
    uint32_t fg_REGION_ADDR_1        : 32;
    uint32_t fg_REGION_ADDR_2        : 32;
    uint32_t fg_REGION_ADDR_3        : 32;
    uint32_t fg_REGION_ADDR_4        : 32;
    uint32_t fg_REGION_ADDR_5        : 32;
    uint32_t fg_REGION_ADDR_6        : 32;
    uint32_t fg_REGION_ADDR_7        : 32;
    uint32_t fg_REGION_ADDR_8        : 32;
    uint32_t fg_REGION_ADDR_9        : 32;
    uint32_t fg_REGION_ADDR_10       : 32;
    uint32_t fg_REGION_ADDR_11       : 32;
    uint32_t fg_REGION_ADDR_12       : 32;
    uint32_t fg_REGION_ADDR_13       : 32;
    uint32_t fg_REGION_ADDR_14       : 32;
    uint32_t fg_REGION_ADDR_15       : 32;
    uint32_t fg_REGION_ADDR_16       : 32;
    uint32_t fg_REGION_ADDR_17       : 32;
    uint32_t fg_REGION_ADDR_18       : 32;
    uint32_t fg_REGION_ADDR_19       : 32;
    uint32_t fg_REGION_ADDR_20       : 32;
    uint32_t fg_REGION_ADDR_21       : 32;
    uint32_t fg_REGION_ADDR_22       : 32;
    uint32_t fg_REGION_ADDR_23       : 32;
    uint32_t fg_REGION_ADDR_24       : 32;
    uint32_t fg_REGION_ADDR_25       : 32;
    uint32_t fg_REGION_ADDR_26       : 32;
    uint32_t fg_REGION_ADDR_27       : 32;
    uint32_t fg_REGION_ADDR_28       : 32;
    uint32_t fg_REGION_ADDR_29       : 32;
    uint32_t fg_REGION_ADDR_30       : 32;
    uint32_t fg_REGION_ADDR_31       : 32;
    uint32_t fg_REGION_ADDR_32       : 32;
    uint32_t fg_REGION_ADDR_33       : 32;
    uint32_t fg_REGION_ADDR_34       : 32;
    uint32_t fg_REGION_ADDR_35       : 32;
    uint32_t fg_REGION_ADDR_36       : 32;
    uint32_t fg_REGION_ADDR_37       : 32;
    uint32_t fg_REGION_ADDR_38       : 32;
    uint32_t fg_REGION_ADDR_39       : 32;
    uint32_t fg_REGION_ADDR_40       : 32;
    uint32_t fg_REGION_ADDR_41       : 32;
    uint32_t fg_REGION_ADDR_42       : 32;
    uint32_t fg_REGION_ADDR_43       : 32;
    uint32_t fg_REGION_ADDR_44       : 32;
    uint32_t fg_REGION_ADDR_45       : 32;
    uint32_t fg_REGION_ADDR_46       : 32;
    uint32_t fg_REGION_ADDR_47       : 32;
    uint32_t fg_REGION_ADDR_48       : 32;
    uint32_t fg_REGION_ADDR_49       : 32;
    uint32_t fg_REGION_ADDR_50       : 32;
    uint32_t fg_REGION_ADDR_51       : 32;
    uint32_t fg_REGION_ADDR_52       : 32;
    uint32_t fg_REGION_ADDR_53       : 32;
    uint32_t fg_REGION_ADDR_54       : 32;
    uint32_t fg_REGION_ADDR_55       : 32;
    uint32_t fg_REGION_ADDR_56       : 32;
    uint32_t fg_REGION_ADDR_57       : 32;
    uint32_t fg_REGION_ADDR_58       : 32;
    uint32_t fg_REGION_ADDR_59       : 32;
    uint32_t fg_REGION_ADDR_60       : 32;
    uint32_t fg_REGION_ADDR_61       : 32;
    uint32_t fg_REGION_ADDR_62       : 32;
    uint32_t fg_REGION_ADDR_63       : 32;

    // DWRD region enable         (4480h)
    uint32_t fg_REGION_ENA_1         : 32;

    // DWRD region enable         (4484h)
    uint32_t fg_REGION_ENA_2         : 32;

    // DWORD - LAST            (4488h)
    uint32_t fg_LL_LAST              : 32;
} MI_DIF_LOSSLESS_FIELD_T;

typedef union _MI_DIF_LOSSLESS_UNION_T
{
    MI_DIF_LOSSLESS_REG_T   rReg;
    MI_DIF_LOSSLESS_FIELD_T rField;
} MI_DIF_LOSSLESS_UNION_T;


/** gfx operation
 1 - text/bitmap mapping
 2 - rectangle fill
 3 - draw point/horizontal line
 4 - draw vertical line mode
 5 - gradient fill
 6 - normal bitblt
 7 - 1-D bitblt
 8 - alpha blending bitblt
 9 - alpha composition bitblt
 10 - YCbCr to RGB conversion
 11 - stretch bitblt
 12 - alpha map bitblt
 13 - loop mode alpha composition
 14 - ROP bitblt
 15 - index to direct mode bitblt
 16 - horizontal line to vertical line
 17 - oblinque line
 18 - graphics bit-plane compression & ms alpha blending bitblt 
 19 - stretch bitblt & loop mode aplpha composition
 20 - stretch bitblt & rotate & loop mode alpha composition
 21 - stretch bitblt & java xor
 22 - stretch bitblt & rotate & java xor
 23 - index to direct mode bitblt & loop mode alpha compostion
 24 - stretch bitblt & index to direct mode bitblt & loop mode alpha compostion
 */
enum EGFX_OP_MODE_T
{
    OP_TEXT_BLT             = 1,
    OP_BITMAP_BLT           = 1,

    OP_RECT_FILL            = 2,
    OP_DRAW_HLINE           = 3,
    OP_DRAW_POINT           = 3,

    OP_DRAW_VLINE           = 4,
    OP_GRAD_FILL            = 5,
    OP_BITBLT               = 6,
    OP_DMA                  = 7,
    OP_1D_BITBLT            = 7,

    OP_ALPHA_BITBLT         = 8,
    OP_ALPHA_COMPOS_BITBLT  = 9,
    OP_YCRCB_RGB_CNV        = 10,
    OP_STRETCH_BITBLT       = 11,
    OP_ALPHA_MAP_BITBLT     = 12,
    OP_LOOP_ALPAH_COMPOS    = 13,
    OP_ROP_BITBLT           = 14,
    OP_IDX2DIR_BITBLT       = 15,
    OP_H2V_LINE             = 16,
    OP_OBLIQUE_LINE         = 17,
    OP_BPCOMP                           = 18,
    OP_MS_ALPHA_BITBLT                  = 18,  //by sxj 
    OP_STRETCH_LOOP_ALPHA_COMPOS        = 19,
    OP_STRETCH_ROTATE_LOOP_ALPHA_COMPOS = 20,
    OP_STRETCH_JAVA_XOR                 = 21,
    OP_STRETCH_ROTATE_JAVA_XOR          = 22,
    OP_IDX2DIR_LOOP_ALPHA_COMPOS        = 23,
    OP_STRETCH_IDX2DIR_LOOP_ALPHA_COMPOS = 24,
    OP_YCBCR2RGB_ALCOM              = 25,
    OP_STRETCH_YCBCR2RGB_ALCOM      = 26,
    OP_STRETCH_YCBCR2RGB_JAVAXOR    = 27,
};


#endif // GFX_DIF_REG_H


/*lint -restore */


