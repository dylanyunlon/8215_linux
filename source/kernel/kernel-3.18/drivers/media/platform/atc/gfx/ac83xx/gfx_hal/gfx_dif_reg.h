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
    UINT32 u4_G_CONFIG;                 // 0x4000
    UINT32 u4_G_STATUS;                 // 0x4004
    UINT32 u4_DRAMQ_STAD;               // 0x4008
    UINT32 u4_DRAMQ_LEN;                // 0x400C
    UINT32 u4_G_MODE;                   // 0x4010
    UINT32 u4_RECT_COLOR;               // 0x4014
    UINT32 u4_SRC_BSAD;                 // 0x4018
    UINT32 u4_DST_BSAD;                 // 0x401C
    UINT32 u4_SRC_XY;                   // 0x4020
    UINT32 u4_DST_XY;                   // 0x4024
    UINT32 u4_SRC_SIZE;                 // 0x4028
    UINT32 u4_S_OSD_WIDTH;              // 0x402C
    UINT32 u4_CLIP_BR;                  // 0x4030
    UINT32 u4_CLIP_TL;                  // 0x4034
    UINT32 u4_GRAD_X_DELTA;             // 0x4038
    UINT32 u4_GRAD_Y_DELTA;             // 0x403C
    UINT32 u4_GRAD_XY_INC;              // 0x4040
    UINT32 u4_BITBLT_MODE;              // 0x4044
    UINT32 u4_KEY_DATA0;                // 0x4048
    UINT32 u4_KEY_DATA1;                // 0x404C
    UINT32 u4_SRCCBCR_BSA;              // 0x4050
    UINT32 u4_SRCCBCR_PITC;             // 0x4054
    UINT32 u4_DSTCBCR_BSA;              // 0x4058
    UINT32 u4_F_COLOR;                  // 0x405C
    UINT32 u4_B_COLOR;                  // 0x4060
    UINT32 u4_COL_TRAN0;                // 0x4064
    UINT32 u4_COL_TRAN1;                // 0x4068
    UINT32 u4_COL_TRAN2;                // 0x406C
    UINT32 u4_COL_TRAN3;                // 0x4070
    UINT32 u4_COL_TRAN4;                // 0x4074
    UINT32 u4_COL_TRAN5;                // 0x4078
    UINT32 u4_COL_TRAN6;                // 0x407C
    UINT32 u4_COL_TRAN7;                // 0x4080
    UINT32 u4_STR_BLT_H;                // 0x4084
    UINT32 u4_STR_BLT_V;                // 0x4088
    UINT32 u4_STR_DST_SIZE;             // 0x408C
    UINT32 u4_LEGAL_START_ADDR;         // 0x4090
    UINT32 u4_LEGAL_END_ADDR;           // 0x4094
    UINT32 u4_DUMMY;                    // 0x4098
    UINT32 u4_ALCOM_LOOP;               // 0x409C
    UINT32 u4_ROP;                      // 0x40A0
    UINT32 u4_IDX2DIR;                  // 0x40A4
    UINT32 u4_SRC_WBBSAD;               // 0x40A8
    UINT32 u4_DST_WBBSAD;               // 0x40AC
    UINT32 u4_SRCCBCR_WBBSAD;           // 0x40B0
    UINT32 u4_Res40B4;                  // 0x40B4
    UINT32 u4_VA_MSB;                   // 0x40B8
    UINT32 u4_RACING_MODE;              // 0x40BC
    UINT32 u4_BPCOMP_CFG;               // 0x40C0
    UINT32 u4_BPCOMP_AD_END;            // 0x40C4
    UINT32 u4_Res40C8;                  // 0x40C8
    UINT32 u4_Res40CC;                  // 0x40CC
    UINT32 u4_Res40D0;                  // 0x40D0
    UINT32 u4_Res40D4;                  // 0x40D4
    UINT32 u4_Res40D8;                  // 0x40D8
    UINT32 u4_Res40DC;                  // 0x40DC
    UINT32 u4_Res40E0;                  // 0x40E0
    UINT32 u4_Res40E4;                  // 0x40E4
    UINT32 u4_Res40E8;                  // 0x40E8
    UINT32 u4_Res40EC;                  // 0x40EC
    UINT32 u4_Res40F0;                  // 0x40F0
    UINT32 u4_Res40F4;                  // 0x40F4
    UINT32 u4_Res40F8;                  // 0x40F8
    UINT32 u4_Res40FC;                  // 0x40FC
    UINT32 u4_Res4100;                  // 0x4100
    UINT32 u4_BMP_STATUS;               // 0x4104
    UINT32 u4_G_256CPT;                 // 0x4108
    UINT32 u4_Res410C;                  // 0x410C
    UINT32 u4_Res4110;                  // 0x4110
    UINT32 u4_Res4114;                  // 0x4114
    UINT32 u4_Res4118;                  // 0x4118
    UINT32 u4_Res411C;                  // 0x411C
    UINT32 u4_Res4120;                  // 0x4120
    UINT32 u4_G_PGIG;                   // 0x4124
    UINT32 u4_BMP_SIZE;                 // 0x4128
    UINT32 u4_GREG_HDMV_IDX_256;        // 0x412C
    UINT32 u4_SRC_RING;                 // 0x4130
    UINT32 u4_DST_RING;                 // 0x4134
    UINT32 u4_CPT_ADDR_RD;              // 0x4138
    UINT32 u4_CPT_DATA_RD;              // 0x413C
#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8560)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    UINT32 u4_0x4140;                   // 0x4140
    UINT32 u4_0x4144;                   // 0x4144
    UINT32 u4_0x4148;                   // 0x4148
    UINT32 u4_0x414C;                   // 0x414C
    UINT32 u4_0x4150;                   // 0x4150
    UINT32 u4_0x4154;                   // 0x4154
    UINT32 u4_0x4158;                   // 0x4158
    UINT32 u4_0x415C;                   // 0x415C
    UINT32 u4_0x4160;                   // 0x4160
    UINT32 u4_0x4164;                   // 0x4164
    UINT32 u4_0x4168;                   // 0x4168
    UINT32 u4_0x416C;                   // 0x416C
    UINT32 u4_0x4170;                   // 0x4170
    UINT32 u4_0x4174;                   // 0x4174
    UINT32 u4_0x4178;                   // 0x4178
    UINT32 u4_0x417C;                   // 0x417C
    UINT32 u4_0x4180;                   // 0x4180
    UINT32 u4_0x4184;                   // 0x4184
    UINT32 u4_0x4188;                   // 0x4188
    UINT32 u4_0x418C;                   // 0x418C
    UINT32 u4_0x4190;                   // 0x4190
    UINT32 u4_0x4194;                   // 0x4194
    UINT32 u4_0x4198;                   // 0x4198
    UINT32 u4_0x419C;                   // 0x419C
    UINT32 u4_0x41A0;                   // 0x41A0
    UINT32 u4_0x41A4;                   // 0x41A4
    UINT32 u4_0x41A8;                   // 0x41A8
    UINT32 u4_0x41AC;                   // 0x41AC
    UINT32 u4_0x41B0;                   // 0x41BC
    UINT32 u4_LAST;                     // 0x41B0
#else
    UINT32 u4_LAST;                     // 0x4140
#endif
} MI_DIF_REG_T;


/** gfx hw register bitfield
 *
 */
typedef struct _MI_DIF_FIELD_T
{
    // DWORD - G_CONFIG         (4000h)
    UINT32 fg_EN_DRAMQ          :  1;
    UINT32 fg_INT_MASK          :  1;
    UINT32 fg_POST_THRS         :  2;
    UINT32 fg_CMDFIFO_THRS      :  2;
    UINT32 fg_REQ_INTVAL        :  2;
    UINT32 fg_DRAMQ_MODE        :  1;
    UINT32 fg_SDFIFO_THRS       :  2;
    UINT32                      :  5;
    UINT32 fg_SRAM_LP           :  1;
    UINT32 fg_ENG_LP            :  1;
    UINT32                      :  3;
	UINT32 fg_MMU_RST			:  2;
    UINT32 fg_MMU_CMDQ_EN		:  1;          
    UINT32 fg_DYNAMIC_HIGH_PRIORITY : 1;
    UINT32 fg_SHORT_CMDQ        :  1;
    UINT32                      :  2;
    UINT32 fg_CQ_RST            :  2;
    UINT32 fg_G_RST             :  2;

    // DWORD - G_STATUS         (4004h)
    UINT32 fg_IDLE              :  1;
    UINT32 fg_HWQ_LEN           :  5;
    UINT32                      :  2;
    UINT32 fg_VERSION_ID        :  8;
    UINT32 fg_CURR_Y_LINE       : 11;
    UINT32                      :  5;

    // DWORD - DRAMQ_STAD       (4008h)
    UINT32 fg_DRAMQ_BSAD        : 30;
    UINT32 fg_CYC_SIZE          :  2;

    // DWORD - DRAMQ_LEN        (400Ch)
    UINT32 fg_DRAMQ_LEN         : 18;
    UINT32                      : 6;
	UINT32 fg_IO_MON_SEL		: 4;
    UINT32 fg_NEW_SW            : 1;
    UINT32                      : 3;

    // DWORD - G_MODE           (4010h)
    UINT32 fg_CM                :  4;
    UINT32 fg_OP_MODE           :  5;
	UINT32 fg_SRC_CM_24BPP		:  1;
    UINT32                      :  1;
    UINT32 fg_FIRE              :  1;
    UINT32 fg_BURST_EN          :  1;
    UINT32 fg_BURST_MODE        :  2;
    UINT32                      :  1;
    UINT32 fg_DSTOWN            :  1;
    UINT32 fg_SRCOWN            :  1;
    UINT32                      : 2;
    UINT32  fg_SRC_CM           : 4;
    UINT32                      : 1;
	UINT32  fg_MMU_CLK_OFF		: 1;
    UINT32  fg_STATIC_HIGH_PRIORITY : 1;
    UINT32  fg_BURST_PROTECT_DIS    : 1;
    UINT32  fg_DST_YUV_EN           : 1;
    UINT32  fg_SRC_YUV_EN           : 1;
    UINT32  fg_DST_WT_EN            : 1;
    UINT32  fg_SRC_WT_EN            : 1;

    // DWORD - RECT_COLOR       (4014h)
    UINT32 fg_RECT_COLOR        : 32;

    // DWORD - SRC_BSAD         (4018h)
    UINT32 fg_SRC_BSAD          : 30;
    UINT32 fg_CHAR_CM           :  2;

    // DWORD - DST_BSAD         (401Ch)
    UINT32 fg_DST_BSAD          : 30;
    UINT32                      :  1;
    UINT32 fg_DST_BSAD_WR       :  1;

    // DWORD - SRC_XY           (4020h)
    UINT32 fg_SRCX              : 15;
    UINT32                      :  1;
    UINT32 fg_SRCY              : 11;
    UINT32                      :  5;

    // DWORD - DST_XY           (4024h)
    UINT32 fg_DSTX              : 15;
    UINT32                      :  1;
    UINT32 fg_DSTY              : 11;
    UINT32                      :  5;

    // DWORD - SRC_SIZE         (4028h)  
    UINT32 fg_SRC_WIDTH         : 15;
    UINT32 fg_RL_DEC            :  1;
#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    UINT32 fg_SRC_HEIGHT        : 11;
    UINT32 fg_SRC_CM_2            : 4; //Source color mode selection flag
                                    // if new_sw: 0=> SRC_CM[3:0], 1=>SRC_HEIGHT[13:11]
    UINT32 fg_SRC_HEIGHT_15     : 1; // if new_sw: 0=> SRC_HEIGHT[15], 1=>no used
#else
    UINT32 fg_SRC_HEIGHT        : 16;
#endif

    // DWORD - S_OSD_WIDTH      (402Ch)
    UINT32 fg_OSD_WIDTH         : 16;
    UINT32 fg_SRC_PITCH         : 16;

    // DWORD - CLIP_BR          (4030h)
    UINT32 fg_CLIP_RIGHT        : 15;
    UINT32 fg_CLR_ENA           :  1;
    UINT32 fg_CLIP_BOT          : 14;
    UINT32                      :  1;
    UINT32 fg_CLB_ENA           :  1;

    // DWORD - CLIP_TL          (4034h)
    UINT32 fg_CLIP_LEFT         : 15;
    UINT32 fg_CLL_ENA           :  1;
    UINT32 fg_CLIP_TOP          : 14;
    UINT32                      :  1;
    UINT32 fg_CLT_ENA           :  1;

    // DWORD - GRAD_X_DELTA     (4038h)
    UINT32 fg_DELTA_X_C0        :  8;
    UINT32 fg_DELTA_X_C1        :  8;
    UINT32 fg_DELTA_X_C2        :  8;
    UINT32 fg_DELTA_X_C3        :  8;

    // DWORD - GRAD_Y_DELTA     (403Ch)
    UINT32 fg_DELTA_Y_C0        :  8;
    UINT32 fg_DELTA_Y_C1        :  8;
    UINT32 fg_DELTA_Y_C2        :  8;
    UINT32 fg_DELTA_Y_C3        :  8;

    // DWORD - GRAD_XY_INC      (4040h)
    UINT32 fg_GRAD_X_PIX_INC    : 11;
    UINT32                      :  5;
    UINT32 fg_GRAD_Y_PIX_INC    : 11;
    UINT32                      :  3;
    UINT32 fg_GRAD_MODE         :  2;

    // DWORD - BITBLT_MODE      (4044h)
    UINT32 fg_TRANS_ENA         :  1;
    UINT32 fg_KEYNOT_ENA        :  1;
    UINT32 fg_COLCHG_ENA        :  1;
#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    UINT32 fg_BITBLT_CLIP_ENA   :  1;
#else
    UINT32                      :  1; 
#endif
    UINT32 fg_CFMT_ENA          :  1;
    UINT32 fg_KEYSDSEL          :  1;
    UINT32 fg_PRCN_OPT          :  1;
    UINT32 fg_CLIP_ENA          :  1;
    UINT32 fg_ALPHA_VALUE       :  8;
    UINT32 fg_ALCOM_PASS        :  3;
	UINT32 fg_SRC_PREMULT       :  1;
#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8560)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
#if 0
	
	UINT32						:  3;
	UINT32 fg_DIFF_CM			:  1;
#else
    UINT32 fg_DST_KEY_IN        :  1;
    UINT32 fg_DIS_DST_KEY       :  1;
    UINT32 fg_SRC_KEY_IN        :  1;
    UINT32 fg_DIS_SRC_KEY       :  1;
#endif
#else
    UINT32                      :  5;
#endif
    UINT32 fg_DSTPITCH_DEC      :  1;
    UINT32 fg_DST_MIRR_OR    :  1;
    UINT32 fg_SRCPITCH_DEC   :  1;
    UINT32 fg_SRC_MIRR_OR    :  1;
#if 0 //(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)
    UINT32 fg_DST_WR_ROTATE  : 1;
    UINT32 fg_DST_RD_ROTATE  : 1;
#else
    UINT32 fg_DST_ROTATE        : 1;
    UINT32                      : 1;
#endif
    UINT32  fg_BARREL_ENA       :  1;
    UINT32  fg_PASS_ALU_ENA     :  1;

    // DWORD - KEY_DATA0        (4048h)
    UINT32 fg_COLOR_KEY_MIN     : 32;

    // DWORD - KEY_DATA1        (404Ch)
    UINT32 fg_COLOR_KEY_MAX     : 32;

    // DWORD - SRCCBCR_BSA      (4050h)
    UINT32 fg_SRCCBCR_BSAD      : 30;
    UINT32 fg_YC_FMT            :  2;

    // DWORD - SRCCBCR_PITCH    (4054h)
    UINT32 fg_SRCCBCR_PITCH     : 16;
    UINT32 fg_VSTD              :  1;
    UINT32 fg_VSYS              :  1;
    UINT32 fg_VSCLIP            :  1;
    UINT32 fg_FLD_PIC           :  1;
    UINT32 fg_SWAP_MODE         :  2;
#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    UINT32 fg_SWAP_NEW_MODE     : 2;
    UINT32 fg_SRC2_BSAD_ENA     : 1;
    UINT32                      : 3;
    UINT32 fg_SRC2_CM           : 4;
#else
    UINT32                      : 10;
#endif

    // DWORD - DSTCBCR_BSAD     (4058h)
    UINT32 fg_DSTCBCR_BSAD      : 28;
    UINT32                      :  4;

    // DWORD - F_COLOR (1)      (405Ch)
    UINT32 fg_FORE_COLOR        : 32;

    // DWORD - B_COLOR (0)      (4060h)
    UINT32 fg_BACK_COLOR        : 32;

    // DWORD - COL_TRAN0        (4064h)
    UINT32 fg_COLOR_TRANS0      : 32;

    // DWORD - COL_TRAN1        (4068h)
    UINT32 fg_COLOR_TRANS1      : 32;

    // DWORD - COL_TRAN2        (406Ch)
    UINT32 fg_COLOR_TRANS2      : 32;

    // DWORD - COL_TRAN3        (4070h)
    UINT32 fg_COLOR_TRANS3      : 32;

    // DWORD - COL_TRAN4        (4074h)
    UINT32 fg_COLOR_TRANS4      : 32;

    // DWORD - COL_TRAN5        (4078h)
    UINT32 fg_COLOR_TRANS5      : 32;

    // DWORD - COL_TRAN6        (407Ch)
    UINT32 fg_COLOR_TRANS6      : 32;

    // DWORD - COL_TRAN7        (4080h)
    UINT32 fg_COLOR_TRANS7      : 32;

    // DWORD - STR_BLT_H        (4084h)
    UINT32 fg_STR_BLT_H         : 24;
    UINT32                      :  8;

    // DWORD - STR_BLT_V        (4088h)
    UINT32 fg_STR_BLT_V         : 24;
    UINT32                      :  8;

    // DWORD - STR_DST_SIZE     (408Ch)
    UINT32 fg_STR_DST_WIDTH     : 15;
    UINT32                      :  1;
    UINT32 fg_STR_DST_HEIGHT    : 11;
    UINT32                      :  5;

    // DWORD - LEGAL_START_ADDR (4090h) default value:0x80000000
    UINT32 fg_LEGAL_AD_START    : 30;
    UINT32                      :  1;
    UINT32 fg_WR_PROT_EN        :  1;

    // DWORD - LEGAL_END_ADDR   (4094h)  default value:0x3FFFFFFF
    UINT32 fg_LEGAL_AD_END      : 30;
    UINT32                      :  2;

    // DWORD - DUMMY            (4098h)
    UINT32 fg_DUMMY             : 32;

    // DWORD - ALCOM_LOOP       (409C)
    UINT32 fg_ALCOM_AR          : 8;
    UINT32 fg_ALCOM_OPCODE      : 4;
    UINT32                      : 4;
    UINT32 fg_ALCOM_RECT_SRC    : 1;
    UINT32 fg_ALCOM_NORMAL      : 1;
    UINT32 fg_PRE2NONPREMUTLI_ENA : 1;
    UINT32 fg_NONPRE2PREMUTLI_ENA : 1;
    UINT32 fg_SRC_OVERFLOW_ENA  : 1;
    UINT32                      : 11;

    // DWORD - ROP              (40A0)
    UINT32 fg_ROP_OPCODE        : 4;
#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    UINT32 fg_SRCALPHA_CHECK    : 1; /*JavaXor-default : 1 -- else default 0*/
    UINT32                      : 3;
#else
    UINT32                      : 4;
#endif
    UINT32 fg_NO_WR             : 1;
    UINT32 fg_CMP_FLAG          : 1;
    UINT32                      : 6; /**/
    UINT32 fg_YUVRGB_MODE       : 3;
#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8561)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    UINT32                      : 11;
    UINT32 fg_COLORIZE_REP      : 1;
    UINT32 fg_PRE_COLORIZE      : 1;
#else
    UINT32                      : 13;
#endif

    // DWORD - IDX2DIR          (40A4)
    UINT32 fg_PAL_BSAD          : 30;
    UINT32 fg_MSB_LEFT          : 1;
    UINT32 fg_LN_ST_BYTE_AL     : 1;

    // DWORD - SRC_WBBSAD       (40A8)
    UINT32 fg_SRC_WBBSAD        : 30;
    UINT32                      : 2;

    // DWORD - DST_WBBSAD       (40AC)
    UINT32 fg_DST_WBBSAD        : 30;
    UINT32                      : 2;

    // DWORD - SRCCBCR_WBBSAD   (40B0)
    UINT32 fg_SRCCBCR_WBBSAD    : 30;
    UINT32                      : 2;

    // DWORD - XOR COLOR        (40B4)
    /** 8560:
     * ROP mode:3: xor color in set XOR mode.
     * ROP mode:2: [23:16]-Ar, [15:8]-Ag, [7:0]-Ab
     */
    UINT32 fg_JAVA_XOR_COLOR    : 32;

#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8560)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)

	#if 0
    // DWORD - DRAMQ_STADMSB -- Virtual Address MSB  (40B8)
    UINT32                      : 6;
    UINT32 fg_INDEX_BASD_H      : 2;
    UINT32 fg_BPCOMP_AD_END_H   : 2;
    UINT32 fg_SRCCBCR_WBBSAD_H  : 2;
    UINT32 fg_DST_WBBSAD_H      : 2;
    UINT32 fg_SRC_WBBSAD_H      : 2;
    UINT32 fg_PAL_BSAD_H        : 2;
    UINT32 fg_DSTCBCR_BSAD_H    : 2;
    UINT32 fg_SRCCBCR_BSAD_H    : 2;
    UINT32 fg_DST_BSAD_H        : 2;
    UINT32 fg_SRC_BSAD_H        : 2;
    UINT32 fg_LEGAL_AD_END_H    : 2;
    UINT32 fg_LEGAL_AD_START_H  : 2;
    UINT32 fg_DRAMQ_BSAD_H      : 2;
	#else
	UINT32 fg_Res40B8           : 32;
	#endif
	
    // DWORD - RACING_MODE_REGISTER  (40BC)
    UINT32                              : 18;
    UINT32 fg_RACING_MODE_TEST          : 1;
    UINT32 fg_OSD3_RIGHT_FLIP_TEST      : 1;
    UINT32 fg_OSD3_LEFT_FLIP_TEST       : 1;
    UINT32 fg_OSD2_RIGHT_FLIP_TEST      : 1;
    UINT32 fg_OSD2_LEFT_FLIP_TEST       : 1;
    UINT32 fg_OSD3_NEXT_RIGHT_DRAW_CMD  : 1;
    UINT32 fg_OSD3_NEXT_LEFT_DRAW_CMD   : 1;
    UINT32 fg_OSD2_NEXT_RIGHT_DRAW_CMD  : 1;
    UINT32 fg_OSD2_NEXT_LEFT_DRAW_CMD   : 1;
    UINT32 fg_OSD3_RACING_RIGHT_COMP_EN : 1;
    UINT32 fg_OSD3_RACING_LEFT_COMP_EN  : 1;
    UINT32 fg_OSD2_RACING_RIGHT_COMP_EN : 1;
    UINT32 fg_OSD2_RACING_LEFT_COMP_EN  : 1;
    UINT32 fg_RACING_EN                 : 1;
#else
    UINT32 fg_Res40B8           : 32;
    UINT32 fg_Res40BC           : 32;
#endif

    // DWORD - BPCOMP_CFG       (40C0)
    UINT32 fg_ROLL_BACK_EN      : 1;
    UINT32 fg_QUALITY_MODE      : 2;
    UINT32 fg_LINE_SEPRATE      : 1;
#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8560)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    UINT32 fg_PIXEL_SEPARATE    : 1;
    UINT32 fg_DEC_ENA           : 1;
    UINT32 fg_DEC_MODE          : 2;
    UINT32                      : 24;
#elif 0//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)
    UINT32 fg_PIXEL_SEPARATE    : 1;
    UINT32 fg_DEC_ENA           : 1;
    UINT32 fg_MASK_DEC_ENA      : 1;
    UINT32 fg_OFFSET_DEC_ENA    : 1;
    UINT32                      : 24;
#else
    UINT32 fg_Res40C0           : 28;
#endif

    // DWORD - BPCOMP_AD_END    (40C4)    
    UINT32 fg_BPCOMP_AD_END     : 30;
    UINT32 fg_Res40C4           :  2;
    
    // DWORD - BPCOMP_DBG1      (40C8)    
    UINT32 fg_BPCOMP_NIPPLE     : 26;
    UINT32                      : 5;
    UINT32 fg_BPCOMP_STPP       : 1;

    // DWORD - BPCOMP_DBG2      (40CC)
    UINT32 fg_BPCOMP_CHKSUM      : 32;

    // DWORD - BPCOMP_DBG3_0    (40D0)
    UINT32 fg_BPCOMP_STOP_ID0    : 32;

    // DWORD - BPCOMP_DBG3_1    (40D4)
    UINT32 fg_BPCOMP_STOP_ID1   : 32;

    // DWORD - BPCOMP_DBG3_2    (40D8)
    UINT32 fg_BPCOMP_STOP_ID2   : 32;

    // DWORD - BPCOMP_DBG3_3    (40DC)
    UINT32 fg_BPCOMP_STOP_ID3   : 32;

#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    // BPCOMP_PACKET            (40E0)
    UINT32 fg_PACKET_WIDTH      : 16;
    UINT32 fg_PIXEL_WIDTH       : 8;
    UINT32                      : 8;
    // BPCOMP_OFFSET1           (40E4)
    UINT32 fg_PACKET_ACTIVE_WIDTH : 15;
    UINT32                        : 1;
    UINT32 fg_PACKET_X_START      : 15;
    UINT32                        : 1;
    // BPCOMP_OFFSET2           (40E8)
    //UINT32 fg_LINE_WIDTH        : 15;
    //UINT32                      : 17;
    UINT32 fg_DST_COLOR_KEY_MIN :32;
    // BPCOMP_INDEX_BASD        (40EC)
    //UINT32 fg_INDEX_BASD        : 30;
    //UINT32                      : 2;
    UINT32 fg_DST_COLOR_KEY_MAX :32;
#else
    UINT32 fg_Res40E0           : 32;
    UINT32 fg_Res40E4           : 32;
    UINT32 fg_Res40E8           : 32;
    UINT32 fg_Res40EC           : 32;
#endif

	#if 0
    UINT32 fg_Res40F0           : 32;
	#else
	// DWORD - DRAMQ_STADMSB -- Virtual Address MSB  (40F0)
    UINT32                      : 6;
    UINT32 fg_INDEX_BASD_H      : 2;
    UINT32 fg_SRC_24BPP_BSAD_H   : 2;
    UINT32 fg_SRCCBCR_WBBSAD_H  : 2;
    UINT32 fg_DST_WBBSAD_H      : 2;
    UINT32 fg_SRC_WBBSAD_H      : 2;
    UINT32 fg_PAL_BSAD_H        : 2;
    UINT32 fg_DSTCBCR_BSAD_H    : 2;
    UINT32 fg_SRCCBCR_BSAD_H    : 2;
    UINT32 fg_DST_BSAD_H        : 2;
    UINT32 fg_SRC_BSAD_H        : 2;
    UINT32 fg_LEGAL_AD_END_H    : 2;
    UINT32 fg_LEGAL_AD_START_H  : 2;
    UINT32 fg_DRAMQ_BSAD_H      : 2;
	#endif
    UINT32 fg_Res40F4           : 32;
    UINT32 fg_Res40F8           : 32;
    UINT32 fg_Res40FC           : 32;

    UINT32 fg_Res4100           : 32;

    // DWORD - BMP_STATUSDX2DIR (4104)
#if 1//(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8561) || (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8563)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    UINT32 fg_MULTREG_ENA       : 1;
    UINT32 fg_Res4104           : 31;
#else
    UINT32 fg_Res4104           : 32;
#endif

    // DWRD G_256CPT            (4108)
    UINT32 fg_256CPT_FIRE       : 1;
    UINT32 fg_USE_256CPT        : 1;
    UINT32 fg_CPT_DRAM_EN       : 1;
    UINT32                      : 3;
    UINT32 fg_ALU_ENB           : 1;
    UINT32 fg_IDX2DIR_EN        : 1;
    UINT32                      : 11;
    UINT32 fg_SRC_PITCH_ENA     : 1;
    UINT32                      : 2;
	UINT32 fg_GLOABLE_ALPHA_EN  : 1;
    UINT32 fg_ALPHA_EN          : 1;
	UINT32 fg_ROUNDING_1555     : 1;
	UINT32 fg_PRCN_OPTIONG_1555 : 1;
	UINT32 fg_PRCN_KEY_1555     : 1;
	UINT32 fg_ROUNDING_565      : 1;
	UINT32 fg_ROUNDING_4444     : 1;
	UINT32						: 3;

	
    UINT32 fg_Res410C           : 32;
    UINT32 fg_Res4110           : 32;
    UINT32 fg_Res4114           : 32;
    UINT32 fg_Res4118           : 32;
    UINT32 fg_Res411C           : 32;
    UINT32 fg_Res4120           : 32;
	
    // DWRD G_PGIG              (4124)
#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    UINT32 fg_SRC_HEIGHT_19_16  : 4;
    UINT32                      : 12;
#else
    UINT32                      : 16;
#endif
    UINT32 fg_YR_SEL            : 2;
    UINT32 fg_UG_SEL            : 2;
    UINT32 fg_VB_SEL            : 2;
    UINT32 fg_A_SEL             : 2;
    UINT32                      : 8;
	
    UINT32 fg_Res4128           : 32;


    // DWRD HDMV_IDX_256        (412C)
    UINT32 fg_DMV_IDX_256       : 32;

    UINT32 fg_Res4130           : 32;

    //                          (4134)

    UINT32 fg_Res4134         : 32;

    // DWRD CPT_ADDR_RD         (4138)
    UINT32 fg_CPT_RD_ADDR       : 8;
    UINT32                      : 8;
    UINT32 fg_CPT_RD_EN         : 1;
    UINT32                      : 15;

    // DWRD CPT_DATA_RD         (413C)
    UINT32 fg_CPT_DATA_RD       : 32;

#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8560)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    UINT32 fg_Reg4140           : 32;
    UINT32 fg_Reg4144           : 32;
    UINT32 fg_Reg4148           : 32;
    UINT32 fg_Reg414C           : 32;
    UINT32 fg_Reg4150           : 32;
    //UINT32 fg_Reg4154           : 32;
    //DWORD --24bpp src bsad		    (4154)	
    UINT32 fg_SRC_BSAD_24BPP    : 30;
    UINT32 				        :  2;
    
    //UINT32 fg_Reg4158           : 32;
    //DWORD --24bpp SRC_SIZE          (4158)
    UINT32 fg_SRC_WIDTH_24BPP   : 15;
	UINT32 						: 1;
    UINT32 fg_SRC_HEIGHT_24BPP  : 14;
	UINT32						: 2;

    UINT32 fg_Reg415C           : 32;
    UINT32 fg_Reg4160           : 32;
    UINT32 fg_Reg4164           : 32;
    UINT32 fg_Reg4168           : 32;
    UINT32 fg_Reg416C           : 32;
    UINT32 fg_Reg4170           : 32;
    UINT32 fg_Reg4174           : 32;
    UINT32 fg_Reg4178           : 32;
    UINT32 fg_Reg417C           : 32;

    // mmu mapping register (5f040 ~ 5f06c, 5f070)
    // DWRD  IOMMU_CFG0         (4180)
    UINT32 fg_IOMMU_EN          : 1;
    UINT32 fg_TLBLAST           : 1;
    UINT32 fg_AUTODISCARD       : 1;
    UINT32 fg_MIF_ENTRY         : 1;
    UINT32 fg_SMT_RESZ          : 1;
    UINT32 fg_CFG0_BIT5         : 1;
    UINT32 fg_CFG0_BIT6         : 1;
    UINT32 fg_CFG0_BIT7         : 1;
    UINT32 fg_Reg4180           : 24;

    // DWRD IOMMU_CFG1          (4184)
    UINT32 fg_RG_TTB            : 32;

    // DWRD IOMMU_CFG2          (4188)
    UINT32 fg_A0_MMU_EN         : 1;
    UINT32 fg_A0_MID            : 3;
    UINT32 fg_A0_PREFETCH       : 1;
    UINT32 fg_A0_2D_PREFETCH    : 1;
    UINT32 fg_A0_PREFETCH_DEC   : 1;
    UINT32                      : 1;
    UINT32 fg_A0_TWO_WAY        : 1;
    UINT32 fg_A0_MID_2ND        : 3;
    UINT32                      : 4;
    UINT32 fg_A1_MMU_EN         : 1;
    UINT32 fg_A1_MID            : 3;
    UINT32 fg_A1_PREFETCH       : 1;
    UINT32 fg_A1_2D_PREFETCH    : 1;
    UINT32 fg_A1_PREFETCH_DEC   : 1;
    UINT32                      : 1;
    UINT32 fg_A1_TWO_WAY        : 1;
    UINT32 fg_A1_MID_2ND        : 3;
    UINT32                      : 4;

    // DWORD - IOMMU_CFG3       (418ch)
    UINT32 fg_A2_MMU_EN         : 1;
    UINT32 fg_A2_MID            : 3;
    UINT32 fg_A2_PREFETCH       : 1;
    UINT32 fg_A2_2D_PREFETCH    : 1;
    UINT32 fg_A2_PREFETCH_DEC   : 1;
    UINT32                      : 1;
    UINT32 fg_A2_TWO_WAY        : 1;
    UINT32 fg_A2_MID_2ND        : 3;
    UINT32                      : 4;
    UINT32 fg_A3_MMU_EN         : 1;
    UINT32 fg_A3_MID            : 3;
    UINT32 fg_A3_PREFETCH       : 1;
    UINT32 fg_A3_2D_PREFETCH    : 1;
    UINT32 fg_A3_PREFETCH_DEC   : 1;
    UINT32                      : 1;
    UINT32 fg_A3_TWO_WAY        : 1;
    UINT32 fg_A3_MID_2ND        : 3;
    UINT32                      : 4;

    // DWORD - IOMMU_CFG4       (4190h)
    UINT32 fg_IOMMU_CFG4        : 31;
    UINT32 fg_IOMMU_FIRE        :  1;

    // DWORD - IOMMU_CFG5       (4194h)
    UINT32 fg_IOMMU_CFG5        : 32;

    // DWORD - IOMMU_CFG6       (4198h)
    UINT32 fg_IOMMU_CFG6        : 32;

    // DWORD - IOMMU_CFG7       (419ch)
    UINT32 fg_IOMMU_CFG7        : 32;

    // DWORD - IOMMU_CFG8       (41a0h)
    UINT32 fg_IOMMU_CFG8        : 32;    

    // DWORD - IOMMU_CFG9       (41a4h)
    UINT32 fg_IOMMU_CFG9        : 32;

    // DWORD - IOMMU_CFG10      (41a8h)
    UINT32 fg_IOMMU_CFG10       : 32;

    // DWORD - IOMMU_CFG11       (41a4h)
    UINT32 fg_IOMMU_CFG11       : 32;
    
    // DWORD - IOMMU_CFG12      (41a8h)
    UINT32 fg_IOMMU_CFG12       : 32;

    // DWORD - IOMMU_CFG13      (41ach)
    UINT32 fg_IOMMU_CFG13       : 32;
	
    // DWORD - LAST            (41b0h)
    UINT32 fg_LAST              : 32;
#else
    // DWORD - LAST            (4140h)
    UINT32 fg_LAST              : 32;
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
    UINT32 u4_REGION_H_0 ;  // 4300
    UINT32 u4_REGION_H_1 ;
    UINT32 u4_REGION_H_2 ;
    UINT32 u4_REGION_H_3 ;
    UINT32 u4_REGION_H_4 ;
    UINT32 u4_REGION_H_5 ;
    UINT32 u4_REGION_H_6 ;
    UINT32 u4_REGION_H_7 ;
    UINT32 u4_REGION_H_8 ;
    UINT32 u4_REGION_H_9 ;
    UINT32 u4_REGION_H_10;
    UINT32 u4_REGION_H_11;
    UINT32 u4_REGION_H_12;
    UINT32 u4_REGION_H_13;
    UINT32 u4_REGION_H_14;
    UINT32 u4_REGION_H_15;
    UINT32 u4_REGION_H_16;
    UINT32 u4_REGION_H_17;
    UINT32 u4_REGION_H_18;
    UINT32 u4_REGION_H_19;
    UINT32 u4_REGION_H_20;
    UINT32 u4_REGION_H_21;
    UINT32 u4_REGION_H_22;
    UINT32 u4_REGION_H_23;
    UINT32 u4_REGION_H_24;
    UINT32 u4_REGION_H_25;
    UINT32 u4_REGION_H_26;
    UINT32 u4_REGION_H_27;
    UINT32 u4_REGION_H_28;
    UINT32 u4_REGION_H_29;
    UINT32 u4_REGION_H_30;
    UINT32 u4_REGION_H_31;
    UINT32 u4_REGION_ADDR_0 ;   // 4380
    UINT32 u4_REGION_ADDR_1 ;
    UINT32 u4_REGION_ADDR_2 ;
    UINT32 u4_REGION_ADDR_3 ;
    UINT32 u4_REGION_ADDR_4 ;
    UINT32 u4_REGION_ADDR_5 ;
    UINT32 u4_REGION_ADDR_6 ;
    UINT32 u4_REGION_ADDR_7 ;
    UINT32 u4_REGION_ADDR_8 ;
    UINT32 u4_REGION_ADDR_9 ;
    UINT32 u4_REGION_ADDR_10;
    UINT32 u4_REGION_ADDR_11;
    UINT32 u4_REGION_ADDR_12;
    UINT32 u4_REGION_ADDR_13;
    UINT32 u4_REGION_ADDR_14;
    UINT32 u4_REGION_ADDR_15;
    UINT32 u4_REGION_ADDR_16;
    UINT32 u4_REGION_ADDR_17;
    UINT32 u4_REGION_ADDR_18;
    UINT32 u4_REGION_ADDR_19;
    UINT32 u4_REGION_ADDR_20;
    UINT32 u4_REGION_ADDR_21;
    UINT32 u4_REGION_ADDR_22;
    UINT32 u4_REGION_ADDR_23;
    UINT32 u4_REGION_ADDR_24;
    UINT32 u4_REGION_ADDR_25;
    UINT32 u4_REGION_ADDR_26;
    UINT32 u4_REGION_ADDR_27;
    UINT32 u4_REGION_ADDR_28;
    UINT32 u4_REGION_ADDR_29;
    UINT32 u4_REGION_ADDR_30;
    UINT32 u4_REGION_ADDR_31;
    UINT32 u4_REGION_ADDR_32;
    UINT32 u4_REGION_ADDR_33;
    UINT32 u4_REGION_ADDR_34;
    UINT32 u4_REGION_ADDR_35;
    UINT32 u4_REGION_ADDR_36;
    UINT32 u4_REGION_ADDR_37;
    UINT32 u4_REGION_ADDR_38;
    UINT32 u4_REGION_ADDR_39;
    UINT32 u4_REGION_ADDR_40;
    UINT32 u4_REGION_ADDR_41;
    UINT32 u4_REGION_ADDR_42;
    UINT32 u4_REGION_ADDR_43;
    UINT32 u4_REGION_ADDR_44;
    UINT32 u4_REGION_ADDR_45;
    UINT32 u4_REGION_ADDR_46;
    UINT32 u4_REGION_ADDR_47;
    UINT32 u4_REGION_ADDR_48;
    UINT32 u4_REGION_ADDR_49;
    UINT32 u4_REGION_ADDR_50;
    UINT32 u4_REGION_ADDR_51;
    UINT32 u4_REGION_ADDR_52;
    UINT32 u4_REGION_ADDR_53;
    UINT32 u4_REGION_ADDR_54;
    UINT32 u4_REGION_ADDR_55;
    UINT32 u4_REGION_ADDR_56;
    UINT32 u4_REGION_ADDR_57;
    UINT32 u4_REGION_ADDR_58;
    UINT32 u4_REGION_ADDR_59;
    UINT32 u4_REGION_ADDR_60;
    UINT32 u4_REGION_ADDR_61;
    UINT32 u4_REGION_ADDR_62;
    UINT32 u4_REGION_ADDR_63;
    UINT32 u4_REGION_ENA_1;     // 4480
    UINT32 u4_REGION_ENA_2;     // 4484
    UINT32 u4_LL_LAST;          // 4488
} MI_DIF_LOSSLESS_REG_T;

typedef struct _MI_DIF_LOSSLESS_FIELD_T
{
    // DWRD region height         (4300h ~ 437ch)
    UINT32 fg_REGION_H_0        : 16;
    UINT32 fg_REGION_H_1        : 16;
    UINT32 fg_REGION_H_2        : 16;
    UINT32 fg_REGION_H_3        : 16;
    UINT32 fg_REGION_H_4        : 16;
    UINT32 fg_REGION_H_5        : 16;
    UINT32 fg_REGION_H_6        : 16;
    UINT32 fg_REGION_H_7        : 16;
    UINT32 fg_REGION_H_8        : 16;
    UINT32 fg_REGION_H_9        : 16;
    UINT32 fg_REGION_H_10       : 16;
    UINT32 fg_REGION_H_11       : 16;
    UINT32 fg_REGION_H_12       : 16;
    UINT32 fg_REGION_H_13       : 16;
    UINT32 fg_REGION_H_14       : 16;
    UINT32 fg_REGION_H_15       : 16;
    UINT32 fg_REGION_H_16       : 16;
    UINT32 fg_REGION_H_17       : 16;
    UINT32 fg_REGION_H_18       : 16;
    UINT32 fg_REGION_H_19       : 16;
    UINT32 fg_REGION_H_20       : 16;
    UINT32 fg_REGION_H_21       : 16;
    UINT32 fg_REGION_H_22       : 16;
    UINT32 fg_REGION_H_23       : 16;
    UINT32 fg_REGION_H_24       : 16;
    UINT32 fg_REGION_H_25       : 16;
    UINT32 fg_REGION_H_26       : 16;
    UINT32 fg_REGION_H_27       : 16;
    UINT32 fg_REGION_H_28       : 16;
    UINT32 fg_REGION_H_29       : 16;
    UINT32 fg_REGION_H_30       : 16;
    UINT32 fg_REGION_H_31       : 16;
    UINT32 fg_REGION_H_32       : 16;
    UINT32 fg_REGION_H_33       : 16;
    UINT32 fg_REGION_H_34       : 16;
    UINT32 fg_REGION_H_35       : 16;
    UINT32 fg_REGION_H_36       : 16;
    UINT32 fg_REGION_H_37       : 16;
    UINT32 fg_REGION_H_38       : 16;
    UINT32 fg_REGION_H_39       : 16;
    UINT32 fg_REGION_H_40       : 16;
    UINT32 fg_REGION_H_41       : 16;
    UINT32 fg_REGION_H_42       : 16;
    UINT32 fg_REGION_H_43       : 16;
    UINT32 fg_REGION_H_44       : 16;
    UINT32 fg_REGION_H_45       : 16;
    UINT32 fg_REGION_H_46       : 16;
    UINT32 fg_REGION_H_47       : 16;
    UINT32 fg_REGION_H_48       : 16;
    UINT32 fg_REGION_H_49       : 16;
    UINT32 fg_REGION_H_50       : 16;
    UINT32 fg_REGION_H_51       : 16;
    UINT32 fg_REGION_H_52       : 16;
    UINT32 fg_REGION_H_53       : 16;
    UINT32 fg_REGION_H_54       : 16;
    UINT32 fg_REGION_H_55       : 16;
    UINT32 fg_REGION_H_56       : 16;
    UINT32 fg_REGION_H_57       : 16;
    UINT32 fg_REGION_H_58       : 16;
    UINT32 fg_REGION_H_59       : 16;
    UINT32 fg_REGION_H_60       : 16;
    UINT32 fg_REGION_H_61       : 16;
    UINT32 fg_REGION_H_62       : 16;
    UINT32 fg_REGION_H_63       : 16;

    // DWRD region end address         (4380h ~ 447ch)
    UINT32 fg_REGION_ADDR_0        : 32;
    UINT32 fg_REGION_ADDR_1        : 32;
    UINT32 fg_REGION_ADDR_2        : 32;
    UINT32 fg_REGION_ADDR_3        : 32;
    UINT32 fg_REGION_ADDR_4        : 32;
    UINT32 fg_REGION_ADDR_5        : 32;
    UINT32 fg_REGION_ADDR_6        : 32;
    UINT32 fg_REGION_ADDR_7        : 32;
    UINT32 fg_REGION_ADDR_8        : 32;
    UINT32 fg_REGION_ADDR_9        : 32;
    UINT32 fg_REGION_ADDR_10       : 32;
    UINT32 fg_REGION_ADDR_11       : 32;
    UINT32 fg_REGION_ADDR_12       : 32;
    UINT32 fg_REGION_ADDR_13       : 32;
    UINT32 fg_REGION_ADDR_14       : 32;
    UINT32 fg_REGION_ADDR_15       : 32;
    UINT32 fg_REGION_ADDR_16       : 32;
    UINT32 fg_REGION_ADDR_17       : 32;
    UINT32 fg_REGION_ADDR_18       : 32;
    UINT32 fg_REGION_ADDR_19       : 32;
    UINT32 fg_REGION_ADDR_20       : 32;
    UINT32 fg_REGION_ADDR_21       : 32;
    UINT32 fg_REGION_ADDR_22       : 32;
    UINT32 fg_REGION_ADDR_23       : 32;
    UINT32 fg_REGION_ADDR_24       : 32;
    UINT32 fg_REGION_ADDR_25       : 32;
    UINT32 fg_REGION_ADDR_26       : 32;
    UINT32 fg_REGION_ADDR_27       : 32;
    UINT32 fg_REGION_ADDR_28       : 32;
    UINT32 fg_REGION_ADDR_29       : 32;
    UINT32 fg_REGION_ADDR_30       : 32;
    UINT32 fg_REGION_ADDR_31       : 32;
    UINT32 fg_REGION_ADDR_32       : 32;
    UINT32 fg_REGION_ADDR_33       : 32;
    UINT32 fg_REGION_ADDR_34       : 32;
    UINT32 fg_REGION_ADDR_35       : 32;
    UINT32 fg_REGION_ADDR_36       : 32;
    UINT32 fg_REGION_ADDR_37       : 32;
    UINT32 fg_REGION_ADDR_38       : 32;
    UINT32 fg_REGION_ADDR_39       : 32;
    UINT32 fg_REGION_ADDR_40       : 32;
    UINT32 fg_REGION_ADDR_41       : 32;
    UINT32 fg_REGION_ADDR_42       : 32;
    UINT32 fg_REGION_ADDR_43       : 32;
    UINT32 fg_REGION_ADDR_44       : 32;
    UINT32 fg_REGION_ADDR_45       : 32;
    UINT32 fg_REGION_ADDR_46       : 32;
    UINT32 fg_REGION_ADDR_47       : 32;
    UINT32 fg_REGION_ADDR_48       : 32;
    UINT32 fg_REGION_ADDR_49       : 32;
    UINT32 fg_REGION_ADDR_50       : 32;
    UINT32 fg_REGION_ADDR_51       : 32;
    UINT32 fg_REGION_ADDR_52       : 32;
    UINT32 fg_REGION_ADDR_53       : 32;
    UINT32 fg_REGION_ADDR_54       : 32;
    UINT32 fg_REGION_ADDR_55       : 32;
    UINT32 fg_REGION_ADDR_56       : 32;
    UINT32 fg_REGION_ADDR_57       : 32;
    UINT32 fg_REGION_ADDR_58       : 32;
    UINT32 fg_REGION_ADDR_59       : 32;
    UINT32 fg_REGION_ADDR_60       : 32;
    UINT32 fg_REGION_ADDR_61       : 32;
    UINT32 fg_REGION_ADDR_62       : 32;
    UINT32 fg_REGION_ADDR_63       : 32;

    // DWRD region enable         (4480h)
    UINT32 fg_REGION_ENA_1         : 32;

    // DWRD region enable         (4484h)
    UINT32 fg_REGION_ENA_2         : 32;

    // DWORD - LAST            (4488h)
    UINT32 fg_LL_LAST              : 32;
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


