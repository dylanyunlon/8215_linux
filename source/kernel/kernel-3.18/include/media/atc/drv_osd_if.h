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

/** @file osd_if.h
 *  This header file declares public function prototypes of OSD.
 */

#ifndef OSD_IF_H
#define OSD_IF_H

/*lint -e717 -e572*/

/*
e717 : do ... while(0)
e572 : Excessive shift value (precision Integer shifted right by Integer)
*/

/*-----------------------------------------------------------------------------*/
/* Include files*/
/*-----------------------------------------------------------------------------*/

#include "x_lint.h"

#ifdef __ARM2__
#include "x_types.h"
#endif
#include "x_typedef.h"
/*#include "x_assert.h"*/
/*#include "x_common.h"*/
#include "x_hal_ic.h"

#include "x_os.h"

#include "chip_ver.h"

#define x_memset memset

#include "display_inc.h"

#ifdef _RUN_ON_PC
#include "osd_pc.h"
#endif

#ifndef CONFIG_SYS_MEM_PHASE3
#define CONFIG_SYS_MEM_PHASE3 1
#endif
#ifndef CONFIG_SYS_MEM_PHASE2
#define CONFIG_SYS_MEM_PHASE2 0
#endif
/*#define SUPPORT_LUMA_601_709_CONVERT*/



#include "drv_config.h"
#include "sys_config.h"


#define SUPPORT_RGB_YUV_FULL_RANGE_CONVER

#define UI32BPPTEST    0

#define OSD_COLORSPACE_ADJUST   1

#define OSD_USE_NEW_DSPMODE

/*#define OSD_REGION_CLIP_TWO_SIDE*/

#define OSDDARMCHANNEL1  (1)
#define OSDDARMCHANNEL2  (2)


#define OSD_1_USE_CHANNEL    OSDDARMCHANNEL1
#define OSD_2_USE_CHANNEL    OSDDARMCHANNEL1
#define OSD_3_USE_CHANNEL    OSDDARMCHANNEL2
#define OSD_4_USE_CHANNEL    OSDDARMCHANNEL2

#define OSD_COLORSPACE_601               0
#define OSD_COLORSPACE_709               1

#define OSD_MAP_COMP_0          1
#define OSD_MAP_COMP_1          0
#define OSD_MAP_COMP_2          3
#define OSD_MAP_COMP_3          2
#define OSD_MAP_COMP_4          4

/*----------------------------------------------------------------
*  OSD Flip Config
------------------------------------------------------------------*/
/*added by msz00420*/
/*1:Reset IG/PG/UI in every VSYNC ISR. IG/PG/UI's flip and enable actions are not done in the VSYNC*/
/*2:not Reset IG/PG/UI in every VSYNC ISR, but only after flip. All the flip and enable actions are done in VSYNC ISR*/
/*3:The flip and enable action are not done in VSYNC period. It will cause the flickering of the display*/
/*       and some problem of OSD. This option is just for testing the old code.*/
#define OSD_FLIP_SCHEME  2


/*----------------------------------------------------------------
#define OSD_FLIP_SCHEME  2
*  OSD handle typedef
------------------------------------------------------------------*/
typedef void *OSD_HBITMAP_T;
typedef void *OSD_HPALETTE_T;
typedef __s32(*OSD_BASE_SET_FX)(__u32 u4Value);
typedef __s32(*OSD_BASE_GET_FX)(__u32 * pu4Value);
typedef __s32(*OSD_R_BASE_SET_FX)(__u32 u4Value);
typedef __s32(*OSD_R_BASE_GET_FX)(__u32 * pu4Value);

typedef enum {
	eScrnState_Enabled = 0,
	eScrnState_Enabling,
	eScrnState_Disabled,
	eScrnState_Disabling
} OSD_SCRN_STATE;

typedef struct _OSD_SCRN_FLAG {
	__u32         u4OsdPlane;
	OSD_SCRN_STATE eOsdScrnState;
	__u32       hSemOsdScrn;
} OSD_SCRN_FLAG;

#define CNB_OSD_R
/*-----------------------------------------------------------------------------*/
/* Constant definitions*/
/*-----------------------------------------------------------------------------*/

#define OSD_BASE_REG                   (IO_BASE_VA + 0x20000)
#define OSD_R_BASE_REG                 (IO_BASE_VA + 0xa3000)
#define OSD_REG_SOFT_RESET             (OSD_BASE_REG + 0x00004)
#define OSD_REG_SOFT_RESET_R           (OSD_BASE_REG + 0xa3004)

#define OSD_RESET_PLANE_MASK       ((__u32) 0xFFF0)


#define OSD_BASE_SKIP                  2
#define OSD_BASE_REG_NUM               40
#define OSD_R_BASE_REG_NUM             40
#define OSD_CORE_REG_NUM               8

#ifndef _RUN_ON_PC
/* register address defition */
#define OSD_REG_FMT_BASE              (OSD_BASE_REG + 0x00000)
#define OSD_REG_FMT_BASE_R            (OSD_BASE_REG + 0x83000)
#define OSD_REG_CORE_PLA1_BASE        (OSD_BASE_REG + 0x00100)
#define OSD_REG_CORE_PLA2_BASE        (OSD_BASE_REG + 0x00200)
#define OSD_REG_CORE_PLA3_BASE        (OSD_BASE_REG + 0x00300)
#define OSD_REG_CORE_PLA4_BASE        (OSD_BASE_REG + 0x00a00)
#define OSD_REG_CORE_PLA5_BASE        (OSD_BASE_REG + 0x00b00)
#define OSD_REG_CORE_PLA6_BASE        (OSD_BASE_REG + 0x83100)
#define OSD_REG_CORE_PLA7_BASE        (OSD_BASE_REG + 0x83200)
#define OSD_REG_CORE_PLA8_BASE        (OSD_BASE_REG + 0x83300)

#define OSD_REG_SCALER2_BASE          (OSD_BASE_REG + 0x00400)
#endif

#define OSD_PLA1_REG                   (OSD_BASE_REG + 0x00100)
#define OSD_PLA2_REG                   (OSD_BASE_REG + 0x00200)
#define OSD_PLA3_REG                   (OSD_BASE_REG + 0x00300)
#define OSD_PLA4_REG                   (OSD_BASE_REG + 0x00a00)
#define OSD_PLA5_REG                   (OSD_BASE_REG + 0x00b00)
#define OSD_PLA6_REG                   (OSD_BASE_REG + 0x83100)
#define OSD_PLA7_REG                   (OSD_BASE_REG + 0x83200)
#define OSD_PLA8_REG                   (OSD_BASE_REG + 0x83a00)

#define OSD_PLA_REG_NUM                4

#define OSD_SC1_REG                    (OSD_BASE_REG + 0x00c00)
#define OSD_SC2_REG                    (OSD_BASE_REG + 0x00400)
#define OSD_SC3_REG                    (OSD_BASE_REG + 0x00500)
#define OSD_SC4_REG                    (OSD_BASE_REG + 0x00d00)
#define OSD_SC5_REG                    (OSD_BASE_REG + 0x83c00)/*Mustn't use*/
#define OSD_SC6_REG                    (OSD_BASE_REG + 0x83c00)
#define OSD_SC7_REG                    (OSD_BASE_REG + 0x83400)
#define OSD_SC8_REG                    (OSD_BASE_REG + 0x83500)

#define OSD_SC_REG_NUM                 8

#define OSD_SC_STEP_BIT                14
#define OSD_SC_STEP_BASE               0x4000

#define OSD_SRAM_SHARED                0
#define OSD_SRAM_EXCLUSIVE_SC2         2
#define OSD_SRAM_EXCLUSIVE_SC3         3

#define OSD_LPF_PARAM_NUM              5
#define OSD_DEFAULT_LPF_C1             2
#define OSD_DEFAULT_LPF_C2             -3
#define OSD_DEFAULT_LPF_C3             -7
#define OSD_DEFAULT_LPF_C4             28
#define OSD_DEFAULT_LPF_C5             88

#define OSD_RGN_REG_NUM                8
#define OSD_MAX_NUM_RGN                (120*2)
#define OSD_MAX_NUM_RGN_LIST           (90*2)

#define OSD_CK_REG                     0x7000D20C
#define OSD_CK_XTAL                    0
#define OSD_CK_SYS                     1 /*4=dram clock,5=cpu clock*/
#define OSD_CK_OCLK                    2

#define OSD_FIELD_TOP                  1
#define OSD_FIELD_BOT                  0
#define OSD_FRAME_MODE                 2

#define GET_OSDDRV_HANDLE(XHANDLE) ((__u32)(XHANDLE) - 1)

/*#define OSD_BITMAP_MEM_POOL_SIZE (42*1024*1024)*/
#define OSD_BITMAP_MEM_POOL_NAME  "osdpool"


#define OSD_RGN_NUM_PER_MEMCHN   (OSD_MAX_NUM_RGN/2)

#define OSD_RGN_INDARMCHNANNEL1_START (0)
#define OSD_RGN_INDARMCHNANNEL2_START (OSD_RGN_NUM_PER_MEMCHN)

#define OSD_RGN_IS_IN_CHANNEL1(u4Rgn)  \
	((((__s32)(u4Rgn)  >= OSD_RGN_INDARMCHNANNEL1_START) &&  \
	((u4Rgn)  < (OSD_RGN_INDARMCHNANNEL1_START+OSD_RGN_NUM_PER_MEMCHN))) ? TRUE : FALSE)
#define OSD_RGN_IS_IN_CHANNEL2(u4Rgn)  \
	((((u4Rgn)  >= OSD_RGN_INDARMCHNANNEL2_START) &&  \
	((u4Rgn)  < (OSD_RGN_INDARMCHNANNEL2_START+OSD_RGN_NUM_PER_MEMCHN))) ? TRUE : FALSE)


/*-----------------------------------------------------------------------------*/
/* Type definitions*/
/*-----------------------------------------------------------------------------*/
typedef enum {
	OSD_BASE_MAIN,
	OSD_BASE_REAR,
	OSD_BASE_TOTAL
} OSD_BASE_T;

typedef enum {
	OSD_FMT_MAIN,
	OSD_FMT_AUX,
	OSD_FMT_MEG
} OSD_FMT_T;

/** OSD plane enum.
 */
typedef enum {
	OSD_PLANE_1,
	OSD_PLANE_2,
	OSD_PLANE_3,
	OSD_PLANE_4,
	OSD_PLANE_5,
	OSD_PLANE_6,
	OSD_PLANE_7,
	OSD_PLANE_8,
	OSD_PLANE_MAX_NUM
} OSD_PLANE_T;

#define OSD_PLANE_IG    OSD_PLANE_2
#define OSD_PLANE_PG    OSD_PLANE_3
#define OSD_PLANE_UI    OSD_PLANE_1

/** OSD scaler enum.
 */
typedef enum {
	OSD_SCALER_1,
	OSD_SCALER_2,
	OSD_SCALER_3,
	OSD_SCALER_4,
	OSD_SCALER_5,
	OSD_SCALER_6,
	OSD_SCALER_7,
	OSD_SCALER_8,
	OSD_SCALER_MAX_NUM
} OSD_SCALER_T;

/** OSD function return code.
 */
typedef enum {
	OSD_RET_OK,
	OSD_RET_INV_ARG,
	OSD_RET_OUT_OF_MEM,
	OSD_RET_OUT_OF_REGION,
	OSD_RET_OUT_OF_LIST,
	OSD_RET_UNINIT,
	OSD_RET_INV_PLANE,
	OSD_RET_INV_SCALER,
	OSD_RET_INV_REGION,
	OSD_RET_INV_BITMAP,
	OSD_RET_INV_LIST,
	OSD_RET_INV_DISPLAY_MODE,
	OSD_RET_REGION_COLLISION,
	OSD_RET_ERR_INTERNAL,
	OSD_RET_OUT_OF_LOSS_REGION,
	OSD_RET_OUT_OF_LOSS_LIST,
	OSD_RET_INV_LOSSLIST,
	OSD_RET_UNDEF_ERR
} OSD_RET_CODE_T;

/** OSD color mode.
 */
typedef enum {
	OSD_CM_YCBCR_CLUT2,
	OSD_CM_YCBCR_CLUT4,
	OSD_CM_YCBCR_CLUT8,
	OSD_CM_RESERVED_0,
	OSD_CM_CBYCR565_DIRECT16 = OSD_CM_RESERVED_0,
	OSD_CM_CBYCRY422_DIRECT16,
	OSD_CM_AYCBCR1555_DIRECT16 = OSD_CM_CBYCRY422_DIRECT16,
	OSD_CM_YCBYCR422_DIRECT16,
	OSD_CM_AYCBCR4444_DIRECT16 = OSD_CM_YCBYCR422_DIRECT16,
	OSD_CM_AYCBCR8888_DIRECT32,
	OSD_CM_RESERVED_1,
	OSD_CM_RGB_CLUT2,
	OSD_CM_RGB_CLUT4,
	OSD_CM_RGB_CLUT8,
	OSD_CM_RGB565_DIRECT16,
	OSD_CM_ARGB1555_DIRECT16,
	OSD_CM_ARGB4444_DIRECT16,
	OSD_CM_ARGB8888_DIRECT32,
	OSD_CM_RESERVED_2
} OSD_COLOR_MODE_T;

/** OSD display mode. for FPGA emulation
 */
typedef enum {
	OSD_DM_480I,
	OSD_DM_576I,
	OSD_DM_480P,
	OSD_DM_576P,
	OSD_DM_MAX_NUM
} OSD_DISPLAY_MODE_T;

/** OSD blending mode.
 */
typedef enum {
	OSD_BM_NONE,
	OSD_BM_PIXEL,
	OSD_BM_REGION,
	OSD_BM_PLANE
} OSD_BLEND_MODE_T;

/** OSD region list cmd.
 */
typedef enum {
	OSD_RGN_LIST_HEAD,
	OSD_RGN_LIST_TAIL,
	OSD_RGN_LIST_COUNT,
	OSD_LIST_FLAGS,
	OSD_RGN_LIST_COMPRESSED_FLAG
} OSD_RGN_LIST_CMD_T;

#ifdef GFX_SUPPORT_SINGLE_BUFFER
typedef enum {
	e2DVsync    = 0,
	eLeftVsync,
	eRightVsync,
} eVsyncMode;
#endif
/** OSD region cmd.
 */
typedef enum {
	OSD_RGN_ALLOC,
	OSD_RGN_PREV,
	OSD_RGN_NEXT,
	OSD_RGN_FLAGS,
	OSD_RGN_POS_X,
	OSD_RGN_POS_Y,
	OSD_RGN_BMP_W,
	OSD_RGN_BMP_H,
	OSD_RGN_DISP_W,
	OSD_RGN_DISP_H,
	OSD_RGN_OUT_W,
	OSD_RGN_OUT_H,
	OSD_RGN_COLORMODE,
	OSD_RGN_ALPHA,
	OSD_RGN_BMP_ADDR,
	OSD_RGN_BMP_PITCH,
	OSD_RGN_CLIP_V,
	OSD_RGN_CLIP_H,
	OSD_RGN_V_FLIP,
	OSD_RGN_H_MIRROR,
	OSD_RGN_PAL_LOAD,
	OSD_RGN_PAL_ADDR,
	OSD_RGN_PAL_LEN,
	OSD_RGN_STEP_V,
	OSD_RGN_STEP_H,
	OSD_RGN_COLOR_KEY,
	OSD_RGN_COLOR_KEY_EN,
	OSD_RGN_MIX_SEL,
	OSD_RGN_BIG_ENDIAN,
	OSD_RGN_ALPHA_SEL,
	OSD_RGN_YR_SEL,
	OSD_RGN_UG_SEL,
	OSD_RGN_VB_SEL,
	OSD_RGN_NEXT_EN,
	OSD_RGN_NEXT_HDR_ADDR,
	OSD_RGN_FIFO_EX,
	OSD_RGN_SELECT_BYTE_EN,
	OSD_RGN_RGB2YCBCR_EN,
	OSD_RGN_YCBCR709EN,
	OSD_RNG_XVYCC_EN,
	OSD_RGN_DECOMP_EN,
	OSD_RGN_DECOMP_LINE_BASED,
	OSD_RGN_WT_EN,
	OSD_RGN_DECOMP_MODE,
	OSD_RGN_PAL_PA,
} OSD_RGN_CMD_T;



/* mt5371 start----------------------------------------------------------------*/
/** OSD base register bit-field type.
 */
#pragma pack(1)
typedef struct _OSD_BASE_FIELD_T {
	/* DWORD - 000 OSD_FMT_00 */
	__u32      fgUpdate            :  1;   /* write 1 to update shadow register's contents */
	__u32      fgAlwaysUpdate      :  1;   /*  */
	__u32                          : 30;

	/* DWORD - 004 OSD_FMT_04 */
	__u32      fgRstMainFmt        :  1;  /* write 1 to reset main path fmt */
	__u32      fgRstAuxFmt         :  1;  /* write 1 to reset aux path fmt, and it will be 0 autumnally */
	__u32      fgRstMegFmt         :  1;  /* write 1 to reset peg path fmt, and it will be 0 autumnally */
	__u32                          :  1;
	__u32      fgRstOsd1           :  2;  /* write 0 then write 1 to reset osd1 */
	__u32      fgRstOsd2           :  2;  /* write 0 then write 1 to reset osd2 */
	__u32      fgRstOsd3           :  2;  /* write 0 then write 1 to reset osd3 */
	__u32      fgRstOsd4           :  2;  /* write 0 then write 1 to reset osd4 */
	__u32      fgRstOsd5           :  2;  /* write 0 then write 1 to reset osd5 */
	__u32      fgRstCsr            :  2;  /* write 0 then write 1 to reset hardware cursor */

	__u32                          : 16;

	/* DWORD - 008 OSD_FMT_08 */
	__u32      fgHsEdge            :  1;  /* horizontal sync leading edge select */
	__u32      fgVsEdge            :  1;  /* vertical sync leading edge select */
	__u32      fgFldPol            :  1;  /* filed signal polarity */
	__u32      fgOsd1Prgs          :  1;  /* 0 output display interlaced, 1 output display progressive */
	__u32      fgOsd2Prgs          :  1;  /* 0 output display interlaced, 1 output display progressive */
	__u32      fgOsd3Prgs          :  1;  /* 0 output display interlaced, 1 output display progressive */
	__u32      fgOsd4Prgs          :  1;  /* 0 output display interlaced, 1 output display progressive */
	__u32      fgOsd5Prgs          :  1;  /* 0 output display interlaced, 1 output display progressive */
	__u32      fgCsrPrgs           :  1;  /* 0 output display interlaced, 1 output display progressive */
	__u32      fgOsd1Aux           :  1;  /* 0 main path, 1 aux path, 2 meg path */
	__u32      fgOsd2Aux           :  1;  /* 0 main path, 1 aux path, 2 meg path */
	__u32      fgOsd3Aux           :  1;  /* 0 main path, 1 aux path, 2 meg path */
	__u32      fgOsd4Aux           :  1;  /* 0 main path, 1 aux path, 2 meg path */
	__u32      fgOsd5Aux           :  2;  /* 0 main path, 1 aux path, 2 meg path */
	__u32      fgCsrAux            :  1;  /* 0 main path, 1 aux path, 2 meg path */
	__u32                          :  1;
	__u32      fgHsEdgeMeg         :  1;
	__u32      fgVsEdgeMeg         :  1;
	__u32      fgFldPolMeg         :  1;
	__u32      u4Osd1Dotctl        :  2;
	__u32      u4Osd2Dotctl        :  2;
	__u32      u4Osd3Dotctl        :  2;
	__u32      u4Osd4Dotctl        :  2;
	__u32      u4Osd5Dotctl        :  2;
	__u32      u4CsrDotctl         :  2;

	/* DWORD - 00C OSD_FMT_0C */

	__u32      u4OvtMain           : 11;  /* vertical sync width in line unit in main path */
	__u32                          :  5;
	__u32      u4VsWidthMain       :  8;   /* vertical total in line unit in main path */
	__u32      u4HsWidthMain       :  8;  /* horizontal sync width in pixel unit in main path */

	/* DWORD - 010 OSD_FMT_10 */
	/* active display area horizontal start position for OSD2 in pixel clock cycle unit */
	__u32      u4ScrnHStartOsd2    :  9;
	__u32                          :  7;
	/* active display area horizontal start position for OSD1 in pixel clock cycle unit */
	__u32      u4ScrnHStartOsd1    :  9;
	__u32                          :  7;

	/* DWORD - 014 OSD_FMT_14 */
	__u32      u4ScrnHStartCsr     :  9;
	__u32                          :  7;
	__u32      u4ScrnHStartOsd3    :  9;
	__u32                          :  7;

	/* DWORD - 018 OSD_FMT_18 */
	/* active display area vertical start position for bottom field in line unit in main path */
	__u32      u4ScrnVStartBotMain : 11;
	__u32                          :  5;
	/* active display area vertical start position for top field in line unit in main path */
	__u32      u4ScrnVStartTopMain :  8;
	__u32                          :  8;
	/* DWORD - 01C OSD_FMT_1C */
	__u32      u4ScrnVSizeMain     : 11;  /* active display area horizontal size in pixel unit in main path */
	__u32                          :  5;
	__u32      u4ScrnHSizeMain     : 11;  /* active display area vertical size in line unit in main path */
	__u32                          :  5;

	/* DWORD - 020 OSD_FMT_20*/
	__u32      u4Osd1VStart        : 11;  /* OSD1 vertical start position on display in pixel unit */
	__u32                          :  5;
	__u32      u4Osd1HStart        : 11;  /* OSD1 horizontal start position on display in pixel unit */
	__u32                          :  5;

	/* DWORD - 024 OSD_FMT_24 */
	__u32      u4Osd2VStart        : 11;  /* OSD2 vertical start position on display in pixel unit */
	__u32                          :  5;
	__u32      u4Osd2HStart        : 11;  /* OSD2 horizontal start position on display in pixel unit */
	__u32                          :  5;

	/* DWORD - 028 OSD_FMT_28 */
	__u32      u4Osd3VStart        : 11;  /* OSD3 vertical start position on display in pixel unit */
	__u32                          :  5;
	__u32      u4Osd3HStart        : 11;  /* OSD3 horizontal start position on display in pixel unit */
	__u32                          :  5;

	/* DWORD - 02C OSD_FMT_2C */
	__u32                          :  8;
	__u32      fgOsd12Ex           :  1;   /* moved to A4*//* exchange OSD1 and OSD2 */
	__u32      fgOsd34Ex           :  1;   /* moved to A4*//* exchange OSD3 and OSD4 */
	__u32                          :  22;

	/* DWORD - 030 OSD_FMT_30 */
	__u32                          : 32;

	/* DWORD - 034 OSD_FMT_34 */
	__u32                          : 32;

	/* DWORD - 038 OSD_FMT_38 */
	__u32                          : 32;

	/* DWORD - 03C OSD_FMT_3C */
	__u32                          : 32;

	/* DWORD - 040 OSD_FMT_40 */
	/* active display area horizontal start position for OSD4 in pixel clock cycle unit */
	__u32      u4ScrnHStartOsd4    :  9;
	__u32                          :  7;
	/* active display area horizontal start position for OSD5 in pixel clock cycle unit */
	__u32      u4ScrnHStartOsd5    :  9;
	__u32                          :  7;

	/* DWORD - 044 OSD_FMT_44 */
	__u32      u4Osd4VStart        : 11;  /* OSD4 vertical start position on display in pixel unit */
	__u32                          :  5;
	__u32      u4Osd4HStart        : 11;  /* OSD4 horizontal start position on display in pixel unit */
	__u32                          :  5;

	/* DWORD - 048 OSD_FMT_48 */
	__u32      u4Osd5VStart        : 11;  /* OSD5 vertical start position on display in pixel unit */
	__u32                          :  5;
	__u32      u4Osd5HStart        : 11;  /* OSD5 horizontal start position on display in pixel unit */
	__u32                          :  5;

	/* DWORD - 04C OSD_FMT_4C */
	__u32      u4OvtAux            : 11;  /* vertical total in line unit in aux path */
	__u32                          :  5;
	__u32      u4VsWidthAux        :  8;   /* vertical sync width in line unit in aux path */
	__u32      u4HsWidthAux        :  8;  /* horizontal sync width in line unit in aux path */

	/* DWORD - 050 OSD_FMT_50 */
	/* active display area vertical start position for bottom field in line unit in aux path */
	__u32      u4ScrnVStartBotAux  : 11;
	__u32                          :  5;
	/* active display area vertical start position for top field in line unit in aux path */
	__u32      u4ScrnVStartTopAux  :  8;
	__u32                          :  8;

	/* DWORD - 054 OSD_FMT_54 */
	__u32      u4ScrnVSizeAux      : 11;  /* active display area vertical size in line unit in aux path */
	__u32                          :  5;
	__u32      u4ScrnHSizeAux      : 11;  /* active display area horizontal size in pixel unit in aux path */
	__u32                          :  5;

	/* DWORD - 058 OSD_FMT_58 */
	__u32      u4OvtMeg            : 11;  /* vertical total in line unit in message path */
	__u32                          :  5;
	__u32      u4VsWidthMeg        :  8;   /* vertical sync width in line unit in message path */
	__u32      u4HsWidthMeg        :  8;   /* horizontal sync width in line unit in message path */

	/* DWORD - 05C OSD_FMT_5C */
	/* active display area vertical start position for bottom field in line unit in message path */
	__u32      u4ScrnVStartBotMeg  : 11;
	__u32                          :  5;
	/* active display area vertical start position for top field in line unit in message path */
	__u32      u4ScrnVStartTopMeg  :  8;
	__u32                          :  8;

	/* DWORD - 060 OSD_FMT_60 */
	__u32      u4ScrnVSizeMeg      : 11;  /* active display area vertical size in line unit in message path */
	__u32                          :  5;
	__u32      u4ScrnHSizeMeg      : 11;  /* active display area horizontal size in pixel unit in message path */
	__u32                          :  5;

	/* DWORD - 064 OSD_FMT_64 */
	__u32      u4OhtMain           : 12;  /* horizontal total in pixel unit in main path */
	__u32                          : 20;

	/* DWORD - 068 OSD_FMT_68 */
	__u32      u4OhtAux            : 12;  /* horizontal total in pixel unit in aux path */
	__u32                          :  4;
	__u32      u4OhtDgi            : 12;
	__u32                          :  4;

	/* DWORD - 06C OSD_FMT_6C */
	__u32      u4OhtMeg            : 12;  /* horizontal total in pixel unit in message path */
	__u32                          :  4;
	__u32      u4OhtDisp           : 12;
	__u32                          :  4;

	/* DWORD - 070 OSD_FMT_70 */
	__u32                          :  5;
	__u32     fgIntTGen            :  1;  /* hotizontal total in pixel unit in message path */
	/* Enable OSD embedded checksum calculator for DRAM data fetch stablility verification */
	__u32     fgCheckSumEn         :  1;
	__u32                          :  8;
	__u32     u4Sc1CheckSumSel     :  2;  /* */
	__u32     u4Sc2CheckSumSel     :  2;  /* */
	__u32     u4Sc3CheckSumSel     :  2;  /* */
	__u32     u4Sc4CheckSumSel     :  2;  /* */
	__u32                          :  9;

	/* DWORD - 074  */
	__u32                          : 32;

	/* DWORD - 078 OSD_FMT_78 */
	__u32     u4Osd1CheckSum       : 32;  /* OSD1 plane embedded checksum calculator result */

	/* DWORD - 07C OSD_FMT_7C */
	__u32     u4Osd1ScCheckSum     : 32;  /* OSD1 scaler embedded checksum calculator result */

	/* DWORD - 080 OSD_FMT_80 */
	__u32     u4Osd2CheckSum       : 32;  /* OSD2 plane embedded checksum calculator result */

	/* DWORD - 084 OSD_FMT_84 */
	__u32     u4Osd2ScCheckSum     : 32;  /* OSD2 scaler embedded checksum calculator result */

	/* DWORD - 088 OSD_FMT_88 */
	__u32     u4Osd3CheckSum       : 32;  /* OSD3 plane embedded checksum calculator result */

	/* DWORD - 08C OSD_FMT_8C */
	__u32     u4Osd3ScCheckSum     : 32;  /* OSD3 scaler embedded checksum calculator result */

	/* DWORD - 090 OSD_FMT_90 */
	__u32     u4Osd4CheckSum       : 32;  /* OSD4 plane embedded checksum calculator result */

	/* DWORD - 094 OSD_FMT_94 */
	__u32     u4Osd4ScCheckSum     : 32;  /* OSD4 scaler embedded checksum calculator result */

	/* DWORD - 098 OSD_FMT_98 */
	__u32     u4Osd5CheckSum       : 32;  /* OSD5 plane embedded checksum calculator result */

	/* DWORD - 09C OSD_FMT_9C */
	__u32     u4Osd5ScCheckSum     : 32;  /* Cursor embedded checksum calculator result */

	/* DWORD - 0A0 OSD_FMT_A0 */
	__u32     u4OSD5IntState       :  1;
	__u32     u4OSD4IntStatus      :  1;
	__u32     u4OSD3IntStatus      :  1;
	__u32     u4OSD2IntStatus      :  1;
	__u32     u4OSD1IntStatus      :  1;
	__u32                          :  27;

	/* DWORD - 0A4 OSD_FMT_A4 */
	__u32                          :  16;
	__u32      u4SRamType          :  4;
	__u32      u4AlphaSel          :  5;
	__u32                          :  1;
	__u32      u4IOMonSel          :  6;


} OSD_BASE_FIELD_T;

#pragma pack()


/** OSD plane register bit-field type.
 */

typedef struct _OSD_PLA_CORE_FIELD_T {
	/* DWORD - 000 */
	__u32      fgOsdEn             :   1;   /* OSD enable */
	__u32                          :   1;   /* skip */
	__u32      fgFakeHdr           :   1;   /* enable osd register based header */
	__u32      fgPrngEn            :   1;   /* Enable OSD embedded peeudo random number generator */
	__u32                          :   1;   /* skip  */
	__u32      fgOsdDbg            :   1;   /* skip  */
	__u32                          :   1;   /* skip  */
	__u32      fgAlphaZeroBlack    :   1;   /* Enable ouput black color value when alpha is zero */
	__u32      fgOutRngColorMode   :   1;   /* Enable ouput black color value when out of osd region */
	__u32                          :  23;   /* skip */

	/* DWORD - 004 *//* OSD first header address pointer. Unit: byte Bit3~Bit0 will be 0 for 16-byte alignment */
	__u32      u4HeaderAddr        :  26;
	/* shadowed register field */
	__u32                          :   6;  /* skip */

	/* DWORD - 008 */
	__u32      u4GobalBlending     :   8;   /* OSD global blending ratio */
	__u32      u4FadingRatio       :   8;   /* OSD fading blending ratio */
	__u32      fgHFilter           :   1;   /* OSD horizontal interpolation enable */
	__u32      fgColorExpSel       :   1;   /* OSD color expansion mode for direct color */
	__u32      fgAlphaRatioEn      :   1;   /* OSD color expansion mode for alpha */
	__u32                          :   4;  /* skip */
	__u32      fgHMirrorEn         :   1; /* Horizontal Region display mirror enable switch */
	__u32      fgVFlipEn           :   1; /* Vertical region display flip ebable switch */
	__u32      fgRgb2YcbrbEn       :   1;
	__u32      fgXVYCCEn           :   1; /* Full Range YCC Color Conversion */
	__u32      fgYCbCr709En        :   1;
	__u32      fgOsd5FifoWrapEn    :   1;
	__u32                          :   3;  /* skip */

	/* DWORD - 00C */
	__u32      u4ContReqLmt        :   6; /* continuous reuqest limit for fifo fill */
	__u32      u4FifoSize          :   6; /* OSD FIFO logical size */
	__u32      u4PauseCnt          :   4; /* Number of cycles between two successice DRAM read request */
	__u32      u4ContReqLmt0       :   6; /* continuous reuqest limit for first time fifo fill */
	__u32      fgBurstDis          :   1; /* DRAM read burst control */
	__u32      fgRgbMode           :   1; /* OSD WB is RGB / YCbCr */
	__u32      u4VacancyThr        :   4; /* OSD FIFO vacancy threshold */
	__u32                          :   4; /**/


	/* DWORD - 110 */
	__u32      fgOsd1ArbRgnEn      :   1; /* OSD arbitrary region enable */
	__u32                          :  31;

	__u32  u4Occupy[19];

	/* 0x160 */
	__u32      u4DstKeyEnable: 1;
	__u32      u4DstKeySel : 1;
	__u32              : 30;

	__u32     u4DstKeyLow;
	__u32     u4DstKeyUp;
} OSD_PLA_CORE_FIELD_T;



/** OSD scaler register bit-field type.
 */
#pragma pack(1)
typedef struct _OSD_SC_FIELD_T {
	/* DWORD - 000  OSD_SC_00 */
	__u32                          :  2;   /* */

	__u32      fgVuscEn            :  1;   /* vertical up scaler enable switch */
	__u32      fgVdscEn            :  1;   /* vertical down scaler enable switch */
	__u32      fgHuscEn            :  1;   /* horizontal up scaler enable switch */
	__u32      fgHdscEn            :  1;   /* horizontal down scaler enable switch */
	__u32      fgScLpfEn           :  1;   /* anti-aliasing filter enable switch */
	__u32      fgScEn              :  1;   /* scaler enable switch */

	__u32                          :  4;   /* */
	__u32      fgVuscColorEdgeOnly :  1;  /* When VUSC_ALPHA=1, only color apply alpha edge sharpness */
	/* Vetical up scaler apply alpha edge sharpness according to alpha edge */
	__u32      fgVuscAlphaEdgeEn   :  1;
	__u32      fgVdscColorEdgeOnly :  1;  /* When VDSC_ALPHA=1, only color apply alpha edge sharpness */
	/* Vetical down scaler apply alpha edge sharpness according to alpha edge */
	__u32      fgVdscAlphaEdgeEn   :  1;
	__u32      fgHuscColorEdgeOnly :  1;  /* When HUSC_ALPHA=1, only color apply alpha edge sharpness */
	/* Horizontal up scaler apply alpha edge sharpness according to alpha edge */
	__u32      fgHuscAlphaEdgeEn   :  1;
	__u32      fgHdscColorEdgeOnly :  1;  /* When HDSC_ALPHA=1, only color apply alpha edge sharpness */
	/* Horizontal down scaler apply alpha edge sharpness according to alpha edge */
	__u32      fgHdscAlphaEdgeEn   :  1;
	__u32                          : 12;   /* */

	/* DWORD - 004  OSD_SC_04 */
	__u32      u4SrcVSize          : 11;  /* Original image height before scaling in line unit */
	__u32                          :  5;
	__u32      u4SrcHSize          : 11;  /* Original image width before scaling in line unit */
	__u32                          :  5;

	/* DWORD - 008 OSD_SC_08 */
	__u32      u4DstVSize          : 11;  /* Desired image height after scaling in line unit */
	__u32                          :  5;
	__u32      u4DstHSize          : 11;  /* Desired image width after scaling in pixel unit */
	__u32                          :  5;

	/* DWORD - 00C OSD_SC_0C */
	__u32      u4VscHSize          : 11;  /* vertical scaling processing width in pixel unit */
	__u32                          : 21;

	/* DWORD - 010 OSD_SC_10 */
	__u32      u4HdscStep          : 14;  /* horizontal down scaler phase accumulator increment step size */
	__u32                          :  2;
	__u32      u4HdscOfst          : 14;  /* horizontal down scaler initial phase offset */
	__u32                          :  2;

	/* DWORD - 014 */
	__u32      u4HuscStep          : 14;  /* horizontal up scaler phase accumulator increment step size */
	__u32                          :  2;
	__u32      u4HuscOfst          : 14;  /* horizontal up scaler initial phase offset */
	__u32                          :  2;

	/* DWORD - 018 */
	__u32      u4VscOfstBot        : 14;  /* vertical scaler initial phase offset for bottom field */
	__u32                          :  2;
	__u32      u4VscOfstTop        : 14;  /* vertical scaler initial phase offset for top field */
	__u32                          :  2;

	/* DWORD - 01C */
	__u32      u4VscStep           : 14;  /* vertical scaler phase accumulator increment step size */
	__u32                          : 18;

	/* DWORD - 020 Anti-Aliasing Filter configuration Register OSD_SC_20 */
	__u32      u4ScLpfC5           :  7;   /* anti-aliasing filter coefficient 5*/
	__u32                          :  1;
	__u32      u4ScLpfC4           :  7;   /* anti-aliasing filter coefficient 4*/
	__u32                          :  1;
	__u32      u4ScLpfC3           :  6;   /* anti-aliasing filter coefficient 3*/
	__u32                          : 10;



	/*LINT_SUPPRESS_NEXT_EXPRESSION(950)*/
}  OSD_SC_FIELD_T;

#pragma pack()


/** OSD region register bit-field type.
 */
#pragma pack(1)
typedef struct _OSD_RGN_FIELD_T {
	/* DWORD - 000 */
	__u32      u4NextOsdAddr       : 26;   /* next OSD address 16-byte align */
	__u32      fgFifoEx            :  1;
	__u32      fgNextOsdEn         :  1;
	__u32      u4ColorMode         :  4;

	/* DWORD - 004 */
	__u32      u4DataAddr          : 24;
	__u32      u4MixWeight         :  8;

	/* DWORD - 008 */
	__u32      u4HClip             : 12;
	__u32      u4VClip             : 11;
	__u32      fgLineSize8         :  1;
	__u32      u4VbSel             :  2;
	__u32      u4UgSel             :  2;
	__u32      u4YrSel             :  2;
	__u32      u4AlphaSel          :  2;

	/* DWORD - 00C */
	__u32      u4PaletteAddr       : 26;
	__u32      u4PaletteLen        :  2;
	__u32      fgNewPalette        :  1;
	__u32      fgDeCompEn          :  1;
	__u32      fgDeCompLineBased   :  1;
	__u32      fgSelectByteEn      :  1;

	/* DWORD - 010 */
	__u32      u4Ihw               : 12;
	__u32      u4Ivw               : 11;
	__u32      u4LineSize          :  9;

	/* DWORD - 014 */
	__u32      u4HStep             : 16;
	__u32      u4VStep             : 16;

	/* DWORD - 018 */
	__u32      u4Ovw               : 11;
	__u32                          :  1;
	__u32      u4Ovs               : 11;
	__u32      fgColorKeyEn        :  1;
	__u32      u4ColorKey          :  8;

	/* DWORD - 01C */
	__u32      u4Ohw               : 11;
	__u32      fgWTEn              :  1;
	__u32      u4Ohs               : 11;
	__u32      u4DataAddrHI        :  2;
	__u32      u4DeCompMode        :  2;
	__u32      fgAcsFrame          :  1;
	__u32      fgAcsAuto           :  1;
	__u32      fgAcsTop            :  1;
	__u32      u4MixSel            :  2;
	/*LINT_SUPPRESS_NEXT_EXPRESSION(950)*/
} OSD_RGN_FIELD_T;

#pragma pack()


/** OSD cursor register bit-field type.
 */

/** OSD base register union type.
 */
/*lint -save -e960 */
typedef union _OSD_BASE_UNION_T {
	__u32               au4Reg[OSD_BASE_REG_NUM];
	OSD_BASE_FIELD_T     rField;
} OSD_BASE_UNION_T;
/*lint -restore */

/** OSD plane register union type.
 */
typedef union _OSD_PLA_CORE_UNION_T {
	__u32               au4Reg[OSD_CORE_REG_NUM];
	OSD_PLA_CORE_FIELD_T rField;
} OSD_PLA_CORE_UNION_T;

/** OSD plane scaler union type.
 */
/*lint -save -e960 */
typedef union _OSD_SC_UNION_T {
	__u32             au4Reg[OSD_SC_REG_NUM];
	OSD_SC_FIELD_T     rField;
} OSD_SC_UNION_T;
/*lint -restore */

/** OSD region register union type.
 */
/*lint -save -e960 */
typedef union _OSD_RGN_UNION_T {
	__u32             au4Reg[OSD_RGN_REG_NUM];
	OSD_RGN_FIELD_T    rField;
} OSD_RGN_UNION_T;
/*lint -restore */

/** OSD cursor register union type.
 */

/* this struct is uesd to create palette*/
typedef struct _OSD_ARGB_T {
	__u8 u1Alpha;
	__u8 u1Red;
	__u8 u1Green;
	__u8 u1Blue;
} OSD_ARGB_T;

typedef struct _OSD_AYCBCR_T {
	__u8 u1Alpha;
	__u8 u1Y;
	__u8 u1Cb;
	__u8 u1Cr;
} OSD_AYCBCR_T;

/*-----------------------------------------------------------------------------*/
/* Macro definitions*/
/*-----------------------------------------------------------------------------*/

#define OSD_CHECK_NULL(X) \
	do { \
		if ((X) == NULL) { \
			return -(__s32)OSD_RET_INV_ARG; \
		} \
	} while (0)
#define OSD_VERIFY_PLANE(X) \
	do { \
		if (((__u32)(X) >= (__u32)OSD_PLANE_MAX_NUM) || ((__u32)(X) == (__u32)OSD_PLANE_6)) { \
			return -(__s32)OSD_RET_INV_PLANE; \
		} \
	} while (0)


#define OSD_VERIFY_SCALER(X) \
	do { \
		if ((__u32)(X) >= (__u32)OSD_SCALER_MAX_NUM) { \
			return -(__s32)OSD_RET_INV_SCALER; \
		} \
	} while (0)

#define OSD_VERIFY_REGION(X) \
	do { \
		if ((X) >= (__u32)OSD_MAX_NUM_RGN) { \
			return -(__s32)OSD_RET_INV_REGION; \
		} \
	} while (0)

#define OSD_VERIFY_RGNLIST(X) \
	do { \
		if ((__u32)(X) >= (__u32)OSD_MAX_NUM_RGN_LIST) { \
			return -(__s32)OSD_RET_INV_LIST; \
		} \
	} while (0)
#ifdef OSD_USE_NEW_DSPMODE
#define OSD_VERIFY_DISPLAY_MODE(X) \
	do { \
		if ((__u32)(X) >= (__u32)RES_MODE_NUM) { \
			return -(__s32)OSD_RET_INV_DISPLAY_MODE; \
		} \
	} while (0)
#else
#define OSD_VERIFY_DISPLAY_MODE(X) \
	do { \
		if ((__u32)(X) >= (__u32)OSD_DM_MAX_NUM) { \
			return -(__s32)OSD_RET_INV_DISPLAY_MODE; \
		} \
	} while (0)
#endif
/*changed by msz00441 for 8520*/
#define OSD_PLANE_TO_SCALER(X) ((X))
#define OSD_SCALER_TO_PLANE(X) ((X))
/*#define OSD_PLANE_TO_SCALER(X) ((X)-1)*/
/*#define OSD_SCALER_TO_PLANE(X) ((X)+1)*/

#define IGNORE_RET(X) \
	do { \
		__s32 i4Ignore; \
		i4Ignore = (__s32)(X); \
		UNUSED(i4Ignore); \
	} while (0)

#define OSD_GET_LPF_SIGN_NUM(NUM, SIGN, DST) \
	do { \
		if ((NUM) > (SIGN)) { \
			(DST) = (SIGN) - (NUM); \
		} \
		else { \
			(DST) = (NUM); \
		} \
	} while (0)

#define OSD_SET_LPF_SIGN_NUM(NUM, SIGN, SRC) \
	do { \
		if (SRC < 0) { \
			(NUM) = (UINT16)((SIGN) - SRC); \
		} else { \
			(NUM) = (UINT16)SRC; \
		} \
		if ((NUM) >= ((SIGN) << 1)) { \
			return -(__s32)OSD_RET_INV_ARG; \
		} \
	} while (0)

#define OSD_BYTE_PER_PIXEL(CM, BPP) \
	do { \
		switch ((CM)) { \
		case OSD_CM_CBYCRY422_DIRECT16: \
		case OSD_CM_YCBYCR422_DIRECT16: \
		case OSD_CM_RGB565_DIRECT16: \
		case OSD_CM_ARGB1555_DIRECT16: \
		case OSD_CM_ARGB4444_DIRECT16: \
			(BPP) = 2; \
			break; \
		case OSD_CM_AYCBCR8888_DIRECT32: \
		case OSD_CM_ARGB8888_DIRECT32: \
			(BPP) = 4; \
			break; \
		default: \
			(BPP) = 1; \
		} \
	} while (0)


#define OSD_GET_PITCH_SIZE(CM, WIDTH, SIZE) \
	do { \
		switch ((CM)) { \
		case OSD_CM_CBYCRY422_DIRECT16: \
		case OSD_CM_YCBYCR422_DIRECT16: \
		case OSD_CM_RGB565_DIRECT16: \
		case OSD_CM_ARGB1555_DIRECT16: \
		case OSD_CM_ARGB4444_DIRECT16: \
			(SIZE) = (WIDTH) * 2; \
			break; \
		case OSD_CM_AYCBCR8888_DIRECT32: \
		case OSD_CM_ARGB8888_DIRECT32: \
			(SIZE) = (WIDTH) * 4; \
			break; \
		case OSD_CM_YCBCR_CLUT8: \
		case OSD_CM_RGB_CLUT8: \
			(SIZE) = (WIDTH); \
			break; \
		case OSD_CM_YCBCR_CLUT4: \
		case OSD_CM_RGB_CLUT4: \
			(SIZE) = (WIDTH) >> 1; \
			break; \
		case OSD_CM_YCBCR_CLUT2: \
		case OSD_CM_RGB_CLUT2: \
			(SIZE) = (WIDTH) >> 2; \
			break; \
		default: \
			ASSERT(0); \
		} \
	} while (0)


/*-----------------------------------------------------------------------------*/
/* Public functions*/
/*-----------------------------------------------------------------------------*/
extern volatile OSD_BASE_UNION_T *_prHwOsdBaseReg;
extern volatile OSD_BASE_UNION_T *_prHwOsdRBaseReg;
extern volatile OSD_PLA_CORE_UNION_T * _prHwOsdPlaneCoreReg[OSD_CORE_REG_NUM];
extern volatile OSD_SC_UNION_T * _prHwOsdScalerReg[OSD_CORE_REG_NUM];

extern __s32 _rOsdCurRgn[OSD_PLANE_MAX_NUM];
extern __s32 _i4Region_ScrnSvr;
extern void *_pvScrnSvr_Bitmap;
extern OSD_SCRN_FLAG _rOsdScrnFlag;

#if CONFIG_SUSPEND_TO_DRAM
#define OSD_SUPPORT_SUSPEND 1
#endif

#ifdef OSD_SUPPORT_SUSPEND
extern bool _fgIsInSuspend;
#endif

#ifdef GFX_SUPPORT_DOUBLE_BUFFER
#ifdef GFX_SUPPORT_SINGLE_BUFFER
extern __u32 _hSemCallBackToGfx;
extern bool _fgCallBackToGFXDrv;
extern void _vGfxSetSingleBufEvent(void);
#endif
#endif

#ifdef GFX_SUPPORT_3D_DOUBLE_BUFFER
extern x_gfx_flip_cb_fct	pf3DNotifyGFXDrv;
extern OSD_BASE_CHANGE_RESOLUTION_FLAG_T _rOsdSetDispMdFlag;
#endif

extern __s32 OSD_BASE_SetDisplayMode_InVsync(__u32 u4DisplayMode);

#if CONFIG_DRV_3D_SUPPORT
extern EV_GRP_EVENT_T OSD_EV_3D_PTS_PLANE_FLIP[2];
extern OSD_3D_SET_PTS_PAL_ENTIES _rOsd3DSetPtsPalEntries;
#endif

#ifdef GFX_SUPPORT_DOUBLE_BUFFER
extern x_gfx_flip_cb_fct	  pfNotifyGFXDrv;
#endif


EXTERN __s32 OSD_Init(bool fgHwReset);
EXTERN __s32 i4OSD_Uninit(void);
EXTERN __s32 OSD_Reset(bool fgHwReset);

EXTERN __s32 OSDSetColorSpace(__u8 ui1ClrSpa);

EXTERN __s32 OSD_ExchangeOsd12(bool fgValue);
EXTERN __u32 OSD_BASE_GetDisplayMode(void);
EXTERN __s32 OSD_BASE_SetDisplayMode(__u32 u4DisplayMode);

/**/
/*this api is added by msz00441 07-12-3, for Plane Mixer to change OSD plane order,the SCRN_HSTART of*/
/*each OSD Plane will be changed ,and the array is for CS and 5 OSD,the content is the orders of each Plane*/
EXTERN __s32 OSD_BASE_SetOSDOrder(__u32 u4OSDOrder[6]);

EXTERN __s32 OSD_BASE_SetPreMulArea(__u32 u4Target, __u32 u4X, __u32 u4Y,
				    __u32 u4W, __u32 u4H);
EXTERN __s32 OSD_BASE_GetPreMulArea(__u32 *pu4Target, __u32 *pu4X,
				    __u32 *pu4Y, __u32 *pu4W, __u32 *pu4H);
EXTERN __s32 OSD_BASE_SetOsdPosition(__u32 u4Plane, __u32 u4X, __u32 u4Y);
EXTERN __s32 OSD_BASE_GetOsdPosition(__u32 u4Plane, __u32 *pu4X,
				     __u32 *pu4Y);
EXTERN __s32 OSD_BASE_SetClock(__u32 u4Clock);
EXTERN __s32 OSD_BASE_GetPrgs(__u32 u4Plane, __u32 *puPrgs);
EXTERN __s32 _OSD_BASE_GetScrnVSizeMain(__u32 *pu4ScrnVSizeMain);
EXTERN __s32 _OSD_BASE_GetScrnHSizeMain(__u32 *pu4ScrnHSizeMain);



EXTERN __u32 OSD_R_BASE_GetDisplayMode(void);
EXTERN __s32 OSD_R_BASE_SetDisplayMode(__u32 u4DisplayMode);
EXTERN __s32 OSD_R_BASE_SetOsd5DisplayMode(__u32 u4DisplayMode);
#if CONFIG_DRV_ALTHD_SUPPORT
EXTERN __s32 OSD_R_BASE_SetDisplayMode_AltHD(__u32 u4DisplayMode, bool b_is_alternative_HD_output);
#endif

/**/
/*this api is added by msz00441 07-12-3, for Plane Mixer to change OSD plane order,the SCRN_HSTART of*/
/*each OSD Plane will be changed ,and the array is for CS and 5 OSD,the content is the orders of each Plane*/
EXTERN __s32 OSD_R_BASE_SetOSDOrder(__u32 u4OSDOrder[6]);

EXTERN __s32 OSD_R_BASE_SetPreMulArea(__u32 u4Target, __u32 u4X, __u32 u4Y,
				      __u32 u4W, __u32 u4H);
EXTERN __s32 OSD_R_BASE_GetPreMulArea(__u32 *pu4Target, __u32 *pu4X,
				      __u32 *pu4Y, __u32 *pu4W, __u32 *pu4H);
EXTERN __s32 OSD_R_BASE_SetOsdPosition(__u32 u4Plane, __u32 u4X, __u32 u4Y);
EXTERN __s32 OSD_R_BASE_GetOsdPosition(__u32 u4Plane, __u32 *pu4X,
				       __u32 *pu4Y);
EXTERN __s32 OSD_R_BASE_SetClock(__u32 u4Clock);
EXTERN __s32 OSD_R_BASE_GetPrgs(__u32 u4Plane, __u32 *puPrgs);
EXTERN __s32 _OSD_R_BASE_GetScrnVSizeMain(__u32 *pu4ScrnVSizeMain);
EXTERN __s32 _OSD_R_BASE_GetScrnHSizeMain(__u32 *pu4ScrnHSizeMain);
EXTERN void OSD_R_MIX2DVD(bool fgEnable);
/*add by msz00441 for vsync 08-1-7*/
EXTERN __s32 OSD_Vsync(void/*__u8 ucIsAbleToFlip*/);

EXTERN __s32 OSD_PLA_Init(__u32 u4Plane);
EXTERN void OSD_Flip_Flag_Init(void);

EXTERN __u32 OSD_PLA_Map(__u32 u4Plane);

EXTERN __s32 Osd_PustReset_Plane(__u32 u4Plane);
EXTERN __s32 Osd_ReleaseReset_Plane(void);
EXTERN __s32 OSD_PLA_Reset(__u32 u4Plane);
EXTERN __s32 OSD_PLA_Enable(__u32 u4Plane, bool fgEnable);
EXTERN __s32 OSD_PLA_FlipTo(__u32 u4Plane, __u32 u4RgnList);
EXTERN __s32 OSD_PLA_FlipToNone(__u32 u4Plane);
EXTERN __s32 OSD_PLA_Reflip(__u32 u4RgnList);
EXTERN __s32 OSD_PLA_GetBlendLevel(__u32 u4Plane, __u8 *pu1BlendLevel);
EXTERN __s32 OSD_PLA_SetBlendLevel(__u32 u4Plane, __u8 u1BlendLevel);
EXTERN __s32 OSD_PLA_GetFading(__u32 u4Plane, __u8 *pu1Fading);
EXTERN __s32 OSD_PLA_SetFading(__u32 u4Plane, __u8 u1Fading);
EXTERN __s32 OSD_PLA_SetHFilter(__u32 u4Plane, bool fgEnable);
EXTERN __s32 OSD_PLA_SetFifo(__u32 u4Plane, bool fgFastReq, __u8 u1ExVacThr,
			     __u8 u1VacThr, __u8 u1FullThr);
EXTERN __s32 OSD_PLA_SetGlobeEnable(__u32 u4Plane, bool fgGlobeEnable);
EXTERN void OSD_PLA_Mute(void);
EXTERN void OSD_PLA_Unmute(void);
EXTERN __s32 OSD_PLA_Dump(__u32 u4Plane);
EXTERN void OSD_PLA_SetDestColorKey(bool fgEnable, __u32 u4ColorKey);
EXTERN __s32 OSD_PLA_GetFirstRegion(__u32 u4Plane, __u32 *pu4Region);
EXTERN __s32 OsdFlipPlaneInVsync(__u32 u4Plane, bool fgValidReg, __u32 u4List);

/*add by mtk94020*/
EXTERN __s32 OsdFlipPlane(__u32 u4Plane, bool fgValidReg, __u32 addr);

EXTERN __s32 OsdFlipPlaneTryLockInVsyncNowait(__u32 u4Plane, bool fgValidReg, __u32 u4List);
EXTERN __s32 OsdFlipPlaneInVsyncNowait(__u32 u4Plane, bool fgValidReg, __u32 u4List, bool b_force);


EXTERN __s32 i43DNotifyOsd_LockScreen(bool *pfgLocked);
EXTERN __s32 OsdFlipPlaneRightNow(__u32 u4Plane, bool fgValidReg, __u32 u4List);
EXTERN __s32 OSD_PLA_Enable_In_VSYNC(__u32 u4Plane, bool fgEnable);
EXTERN __s32 OSD_PLA_Enable_In_VSYNC_NoWait(__u32 u4Plane, bool fgEnable);

EXTERN __s32 OSD_DRV_GetMemChannelEx(__u32 u4Plane);
EXTERN __s32 OSD_DRV_GetMemChannel(OSD_COLOR_MODE_T eClrMode);

/* region list related APIs */
EXTERN __s32 OSD_RGN_LIST_Set(__u32 u4RgnList, __s32 i4Cmd, __u32 u4Value);
EXTERN __s32 OSD_RGN_LIST_Get(__u32 u4RgnList, __s32 i4Cmd, __u32 *pu4Value);
EXTERN __s32 OSD_RGN_LIST_Create(__u32 *pu4RgnList);
EXTERN __s32 OSD_RGN_LIST_Create_Ex(__u32 *pu4RgnList, bool b_compressed);
EXTERN __s32 OSD_RGN_LIST_Delete(__u32 u4RgnList);
EXTERN __s32 OSD_RGN_LIST_GetHead(__u32 u4RgnList, __s32 *phHeadRegion);
EXTERN __s32 OSD_RGN_LIST_GetNext(__u32 u4RgnList, __s32 hRgnCurr,
				  __s32 *phRgnNext);
EXTERN __s32 OSD_RGN_LIST_DetachAll(__u32 u4RgnList);
EXTERN __s32 OSD_RGN_LIST_GetCount(__u32 u4RgnList, __s32 *pi4Count);
EXTERN __s32 OSD_RGN_LIST_Init(void);

/* Region related APIs */
EXTERN __s32 OSD_RGN_Init(void);
EXTERN __s32 OSD_RGN_Get(__u32 u4Region, __s32 i4Cmd, __u32 *pu4Value);

#ifdef AC823X_CONFIG
EXTERN __s32 OSD_RGN_Set(__u32 u4Region, __s32 i4Cmd, unsigned long u4Value);
#else
EXTERN __s32 OSD_RGN_Set(__u32 u4Region, __s32 i4Cmd, __u32 u4Value);
#endif

EXTERN __s32 OSD_RGN_Create(__u32 *pu4Region, __u32 u4BmpWidth,
			    __u32 u4BmpHeight, void *pvBitmap,
			    __u32 eColorMode, __u32 u4BmpPitch,
			    __u32 u4DispX, __u32 u4DispY, __u32 u4DispW,
			    __u32 u4DispH);
EXTERN __s32 OSD_RGN_Create_EX(__u32 *pu4Region, __u32 u4BmpWidth, __u32 u4BmpHeight,
			       void *pvBitmap, __u32 eColorMode, __u32 u4BmpPitch,
			       __u32 u4DispX, __u32 u4DispY,
			       __u32 u4DispW, __u32 u4DispH, __u32 ui4_plane);
EXTERN __s32 OSD_ARB_RGN_Create(__u32 *pu4Region, __u32 u4BmpWidth, __u32 u4BmpHeight,
				void *pvBitmap, __u32 eColorMode, __u32 u4BmpPitch,
				__u32 u4DispX, __u32 u4DispY,
				__u32 u4DispW, __u32 u4DispH, __u32 ui4Plane);
EXTERN __s32 OSD_RGN_Delete(__u32 u4Region);
EXTERN __s32 OSD_RGN_AttachTail(__u32 u4Region, __u32 u4RgnList);
EXTERN __s32 OSD_RGN_Insert(__u32 u4Region, __u32 u4RgnList);
EXTERN __s32 OSD_ARB_RGN_Insert(__u32 u4Region, __u32 u4RgnList);
EXTERN __s32 OSD_RGN_Detach(__u32 u4Region, __u32 u4RgnList);
EXTERN __s32 OSD_RGN_SetDisplayWidth(__u32 u4Region, __u32 u4Width);
EXTERN __s32 OSD_RGN_SetDisplayHeight(__u32 u4Region, __u32 u4Height);
EXTERN __s32 OSD_RGN_SetBigEndian(__u32 u4Region, bool fgBE);
EXTERN __s32 OSD_RGN_Dump(__u32 u4Region);
#ifdef SUPPORT_RGB_YUV_FULL_RANGE_CONVER
EXTERN __s32 OSD_RNG_Set_Xvycc_En(bool fgEnXvycc);
EXTERN __s32 OSD_RNG_List_Set_Xvycc_En(__u32 u4RgnList);
#endif

/* scaler related APIs */
EXTERN __s32 OSD_SC_Scale(__u32 u4Scaler, __u32 u4Enable,
			  __u32 u4SrcWidth, __u32 u4SrcHeight,
			  __u32 u4DstWidth, __u32 u4DstHeight);
/*#ifdef CC_DEBUG*/
EXTERN __s32 OSD_SC_HDown(__u32 u4Scaler, __u32 u4SrcWidth, __u32 u4Step);
EXTERN __s32 OSD_SC_HUp(__u32 u4Scaler, __u32 u4SrcWidth, __u32 u4Step);
EXTERN __s32 OSD_SC_VDown(__u32 u4Scaler, __u32 u4SrcHeight, __u32 u4Step);
EXTERN __s32 OSD_SC_VUp(__u32 u4Scaler, __u32 u4SrcHeight, __u32 u4Step);
EXTERN __s32 OSD_SC_GetScalerInfo(__u32 u4Scaler, __u32 *pu4Enable,
				  __u32 *pu4SrcW, __u32 *pu4SrcH,
				  __u32 *pu4DstW, __u32 *pu4DstH,
				  __u32 *pu4Is16Bpp);
EXTERN __s32 OSD_SC_SetLpfInfo(__u32 u4Scaler, __u32 u4Enable, INT16 i2C1,
			       INT16 i2C2, INT16 i2C3, INT16 i2C4, INT16 i2C5);
EXTERN __s32 OSD_SC_SetLpf(__u32 u4Scaler, __u32 u4Enable);
EXTERN __s32 OSD_SC_GetLpfInfo(__u32 u4Scaler, __u32 *pu4Enable, INT16 *pi2C1,
			       INT16 *pi2C2, INT16 *pi2C3,
			       INT16 *pi2C4, INT16 *pi2C5);
EXTERN __s32 OSD_SC_SetFormat16Bpp(__u32 u4Scaler);
EXTERN __s32 OSD_SC_SetFormat32Bpp(__u32 u4Scaler);
EXTERN void OSD_SC_UpdateDstSize(__u32 u4Scaler);
EXTERN __s32 OSD_SC_CheckCapability(__u32 u4SrcW, __u32 u4SrcH,
				    __u32 u4DstW, __u32 u4DstH);
EXTERN __s32 OSD_SC_SetSramConfiguration(__u32 u4Mode);
EXTERN __s32 OSD_SC_GetSramConfiguration(void);
EXTERN __s32 OSD_SC_SetSlackEn(__u32 u4SlackEn);

/*#ifdef CC_DEBUG*/
#if 1
EXTERN void OSD_DrawVLine(__u8 *pu1Canvas, __u32 u4Width, __u32 u4Height,
			  __u32 u4ColorMode);
EXTERN void OSD_DrawHLine(__u8 *pu1Canvas, __u32 u4Width, __u32 u4Height,
			  __u32 u4ColorMode);
EXTERN void OSD_DrawBorder(__u8 *pu1Canvas, __u32 u4Width, __u32 u4Height,
			   __u32 u4ColorMode);
EXTERN void OSD_DrawColorbar(__u8 *pu1Canvas, __u32 u4Width, __u32 u4Height,
			     __u32 u4ColorMode);
EXTERN void OSD_DrawSlt(__u32 u4GfxHwId, __u8 *pu1Canvas, __u32 u4Width, __u32 u4Height,
			__u32 u4ColorMode);
EXTERN void OSD_DrawRamp(__u8 *pu1Canvas, __u32 u4Width, __u32 u4Height,
			 __u32 u4ColorMode);
EXTERN void OSD_CreatePaleTable(OSD_ARGB_T *prPaleTable);

EXTERN void OSD_CreatePaletteTable(OSD_ARGB_T *prPaleTable, __u32 u4Cm);

EXTERN __s32 _i4OSD_SetPosOffset(__s32 *pi4OsdPosOfst);
EXTERN __s32 OSD_Slt(void);
EXTERN __s32 _i4Osd5AuxOut(bool fgOsd5OnlyOutCvbs);
EXTERN __s32 _i4Osd5Out(bool fgOsd5OnlyOutCvbs, __u32 eResMode);
EXTERN __s32 _i4Osd5OutMsgPath(bool fgOsd5OnlyOutCvbs, __u32 u4DispMode);
EXTERN __s32 _i4Osd5Out_CVBSOnly(bool fgOsd5OnlyOutCvbs, __u32 eResMode);

#endif


#ifdef SUPPORT_LUMA_601_709_CONVERT
extern __s32 _OSD_InitPaletteCovArray(void);
extern void _OSD_UninitPaletteCovArray(void);
extern void _OSD_Set_Pal_Color_Mode(__u32 u4Entry, bool fgYcbcr709);

extern void  _OSD_ColorConvert709to601(__u32 *pu4OldPal, __u32 *pu4NewPal, __u32 u4Start);

extern void  _OSD_ColorConvert601to709(__u32 *pu4OldPal, __u32 *pu4NewPal, __u32 u4Start);
extern __u32  *_OSD_Get_Pal_To_Hw(__u32 u4CurPal, bool fgYCbCr709);

extern __u32 *_OSD_Get_Current_Pal(__u32 *pu4OldPal, __u32 *pu4Entry);

extern __u32 *_OSD_Get_Unused_Pal(__u32 u4Plane, __u32 *pu4OldPal);


extern __s32 _OSD_Set_Unused_Pal(__u32 *pu4OldPal, __u32 u4Plane);
#endif

extern bool fgOsdInit(void);
/*Move from osd_inc.h*/
extern void SetPlaneRgn(__u32 u4Plane, __u32 u4Rgn);
extern __u32 GetPlaneRgn(__u32 u4Plane);
/*Move from osd_if_pdd.h*/
extern __s32 i4OsdPlaneEnble(__u32 u4Plane, __u32 fgEnble);
extern __s32 i4OsdPlaneFlipTo(__u32 u4Plane, __u32 u4RgnList);

#ifdef _RUN_ON_PC
extern OSD_PLA_CORE_UNION_T   gunion_pla_core_osd1;
extern OSD_PLA_CORE_UNION_T   gunion_pla_core_osd2;
extern OSD_PLA_CORE_UNION_T   gunion_pla_core_osd3;
extern OSD_PLA_CORE_UNION_T   gunion_pla_core_osd4;
extern OSD_PLA_CORE_UNION_T   gunion_pla_core_osd5;
extern OSD_PLA_CORE_UNION_T   gunion_pla_core_osd6;
extern OSD_PLA_CORE_UNION_T   gunion_pla_core_osd7;
extern OSD_PLA_CORE_UNION_T   gunion_pla_core_osd8;


#define OSD_REG_FMT_BASE              (OSD_BASE_REG + 0x00000)
#define OSD_REG_FMT_BASE_R            (OSD_BASE_REG + 0x83000)
#define OSD_REG_CORE_PLA1_BASE        (&gunion_pla_core_osd1)
#define OSD_REG_CORE_PLA2_BASE        (&gunion_pla_core_osd2)
#define OSD_REG_CORE_PLA3_BASE        (&gunion_pla_core_osd3)
#define OSD_REG_CORE_PLA4_BASE        (&gunion_pla_core_osd4)
#define OSD_REG_CORE_PLA5_BASE        (&gunion_pla_core_osd5)
#define OSD_REG_CORE_PLA6_BASE        (&gunion_pla_core_osd6)
#define OSD_REG_CORE_PLA7_BASE        (&gunion_pla_core_osd7)
#define OSD_REG_CORE_PLA8_BASE        (&gunion_pla_core_osd8)

#define OSD_REG_SCALER2_BASE          (OSD_BASE_REG + 0x00400)

#define OSD_REG_CUR_CTRL_BASE         (OSD_BASE_REG + 0x00800)
#define OSD_REG_CUR_DATA_BASE         (OSD_BASE_REG + 0x00900)   /* same with OSD_REG_CORE_PLA4_BASE */
#endif
/*-----------------------------------------------------------------------------*/
/* Global Variables*/
/*-----------------------------------------------------------------------------*/

#ifdef OSD_MEM_POOL_STATIC
extern __u8  _pbOsdBitmapMemPool[];
extern bool fgOsdBitmapMemPoolNeedInit;
extern __u32 HOsdMemPool;
#endif
typedef struct _OSD_BASE_CHANGE_RESOLUTION_FLAG_T {
	bool fgNeedChangeResolution;
	__s32 i4DisplayMode;
} OSD_BASE_CHANGE_RESOLUTION_FLAG_T;

#endif  /*DRV_OSD_IF_H*/




