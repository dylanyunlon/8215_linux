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

#ifndef GFX_IF_H
#define GFX_IF_H


//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------

//#include "x_common.h"
#include "x_typedef.h"
#include "x_timer.h"
#include "x_assert.h"
#include "x_os.h"
#include "x_bim.h"
#include "x_iommu.h"
#include "x_gfx.h"
#include "drv_gfx.h"
#include "gfx_hw.h"

//#include "gfx_drv_cmdbuf.h"
//---------------------------------------------------------------------------
// Configurations
//---------------------------------------------------------------------------
//#ifdef CONFIG_CHIP_VER_CURR
//#undef CONFIG_CHIP_VER_CURR
//#define CONFIG_CHIP_VER_CURR CONFIG_CHIP_VER_MT8563
//#endif



#if CONFIG_LOSSLESS_COMPRESS_AUTO_FLIP_MODE
#define GFX_LOSSLESS_CALLBACK_BDJ_FROM_GFX  1
#endif

#if defined(CC_DEBUG)   // debug mode
    #define GFX_DEBUG_MODE
#endif


#if defined(CC_CLI)     // for cli use
    #define GFX_CLI_USE
#endif


#if defined(CC_MINI_DRIVER)
    #define GFX_MINI_DRIVER
    //#define GFX_RISC_MODE
#endif

#define  GFX_RISC_MODE
#ifdef GFX_RISC_MODE
#define GFX_SYNC_OLD_METHOD   1
#else
#ifdef GFX_PERFORMANCE_TEST
#define GFX_SYNC_OLD_METHOD   0
#else
#define GFX_SYNC_OLD_METHOD   0
#endif
#endif
#if defined(GFX_DEBUG_MODE)
    #define GFX_ENABLE_SW_MODE
#endif
#define _u4GfxAllowedBusyTime 40 

//---------------------------------------------------------------------------
// Constant definitions
//---------------------------------------------------------------------------

#define D_GFXFLAG_TRANSPARENT   (1 << (INT32)E_GFXBLT_TRANSPARENT)
#define D_GFXFLAG_KEYNOT        (1 << (INT32)E_GFXBLT_KEYNOT)
#define D_GFXFLAG_COLORCHANGE   (1 << (INT32)E_GFXBLT_COLORCHANGE)
#define D_GFXFLAG_CLIP          (1 << (INT32)E_GFXBLT_CLIP)
#define D_GFXFLAG_CFMT_ENA      (1 << (INT32)E_GFXBLT_CFMT_ENA)
#define D_GFXFLAG_KEYSDSEL      (1 << (INT32)E_GFXBLT_KEYSDSEL)
#define D_GFXFLAG_NONE          (0)

#define GFX_FONT_1BIT           0
#define GFX_FONT_2BIT           1
#define GFX_FONT_4BIT           2
#define GFX_FONT_8BIT           3

#define GFX_HAVE_SW_MOD         (1 << (INT32)E_GFX_SW_MOD)
#define GFX_HAVE_HW_8520_MOD    (1 << (INT32)E_GFX_HW_8520_MOD)
#define GFX_HAVE_FB_MOD         (1 << (INT32)E_GFX_FB_MOD)

#define GFX_RESET_ENGINE        0xC0
#define GFX_RESET_CMDQUE        0x30
#define GFX_RESET_BOTH          0xF0
#define GFX_RESET_POWERDOWN  0x10

#define GFX_IDX2DIR_LN_ST_BYTE_AL   1
#define GFX_IDX2DIR_MSB_LEFT        1

#if 0
// GFX error mode
enum E_MI_GFX_ERR_CODE_T
{
    E_GFX_OK = 0,
    E_GFX_INV_ARG,
    E_GFX_OUT_OF_MEM,
    E_GFX_UNINIT,
    E_GFX_UNDEF_ERR,
    E_GFX_WOULD_BLOCK,
    E_GFX_EMPTY_BUFFER,
    E_GFX_BPCOMP_STOP
};

/* GFX color mode */

enum EGFX_COLOR_MODE_T
{
    	CM_YCbCr_CLUT2	= 0,
	CM_YCbCr_CLUT4,
	CM_YCbCr_CLUT8,
	CM_Reserved0,
	CM_CbYCrY422_DIRECT16,
	CM_YCbYCr422_DIRECT16,
	CM_AYCbCr8888_DIRECT32,
	CM_Reserved1,
	CM_RGB_CLUT2,
	CM_RGB_CLUT4,
	CM_RGB_CLUT8,
	CM_RGB565_DIRECT16,
	CM_ARGB1555_DIRECT16,
	CM_ARGB4444_DIRECT16,
	CM_ARGB8888_DIRECT32,
	CM_Reserved2,
	CM_SC_SINGLE
};

/* GFX alpha composition mode */
enum EGFX_AC_MODE_T
{
    E_AC_CLEAR = 0,
    E_AC_DST_IN,
    E_AC_DST_OUT,
    E_AC_DST_OVER,
    E_AC_SRC,
    E_AC_SRC_IN,
    E_AC_SRC_OUT,
    E_AC_SRC_OVER,
    E_AC_SRC_ATOP,
    E_AC_DST_ATOP,
    E_AC_XOR,
    E_AC_MAX
};


/* GFX bitblt option mode */
enum EGFX_BLT_OPT_T
{
    E_GFXBLT_TRANSPARENT = 0,
    E_GFXBLT_KEYNOT,
    E_GFXBLT_COLORCHANGE,
    E_GFXBLT_CLIP,
    E_GFXBLT_CFMT_ENA,
    E_GFXBLT_KEYSDSEL
};

/* GFX video standard mode */
enum EGFX_VIDSTD_T
{
    E_VSTD_BT601 = 0,
    E_VSTD_BT709
};


/* GFX video system mode */
enum EGFX_VSYS_T
{
    E_VSYS_VID = 0,
    E_VSYS_COMP
};

/** GFX swap mode
 *  swap mode of YCbCr to RGB
 */
enum EGFX_SWAP_MODE_T
{
    E_SWAP_0        = 0,
    E_SWAP_MERGETOP = 0,
    E_SWAP_1        = 1,
    E_SWAP_SWAP     = 1,
    E_SWAP_2        = 2,
    E_SWAP_BLOCK    = 2,
    E_SWAP_DEF      = 2     //MISRA rule, use 2 instead of E_SWAP_2 here
};


/** GFX YC format
 *  YC format of YCbCr to RGB
 */
enum EGFX_YCFMT_T
{
    E_YCFMT_420MB = 0,
    E_YCFMT_420LINEAR,
    E_YCFMT_422LINEAR,
    E_YCFMT_RES_3
};


// GFX text and bitmap color mode
enum EGFX_BMP_COLOR_MODE_T
{
    E_BMP_CM_1BIT = 0,
    E_BMP_CM_2BIT,
    E_BMP_CM_4BIT,
    E_BMP_CM_8BIT
};


// GFX gradient mode
enum EGFX_GRAD_MODE_T
{
    E_GRAD_RESERVED = 0,
    E_GRAD_HOR,
    E_GRAD_VER,
    E_GRAD_BOTH
};


// GFX SW and HW module
enum E_GFX_MODULE
{
    E_GFX_SW_MOD = 0,
    E_GFX_HW_8520_MOD,
    E_GFX_FB_MOD,
    E_GFX_MODULE_LAST
};


// GFX rop mode
enum EGFX_ROP_MODE_T
{
    E_ROP_RESERVED0 = 0,
    E_ROP_RESERVED1,
    E_ROP_COLORIZE,
    E_ROP_RESERVED3,
    E_ROP_JAVA_XOR = E_ROP_RESERVED3,
    E_ROP_NOT_SRC,
    E_ROP_NOT_DST,
    E_ROP_SRC_XOR_DST,
    E_ROP_SRC_XNOR_DST,
    E_ROP_SRC_AND_DST,
    E_ROP_NOT_SRC_AND_DST,
    E_ROP_SRC_AND_NOT_DST,
    E_ROP_NOT_SRC_AND_NOT_DST,
    E_ROP_SRC_OR_DST,
    E_ROP_NOT_SRC_OR_DST,
    E_ROP_SRC_OR_NOT_DST,
    E_ROP_NOT_SRC_OR_NOT_DST,
    E_ROP_MAX
};


// GFX index to direct color bitblt option mode
enum EGFX_IDX2DIR_OPT_T
{
    E_IDX2DIR_LN_ST_BYTE_AL_OFF = 0,
    E_IDX2DIR_LN_ST_BYTE_AL_ON  = 1,
    E_IDX2DIR_MSB_LEFT_OFF      = 0,
    E_IDX2DIR_MSB_LEFT_ON       = 1
};
#endif

typedef enum {
    E_NOT_PREMULTIPLIED_2_PREMULTIPLIED = 0,
    E_PREMULTIPLIED_2_NON_PREMULTIPLIED = 1
}GFX_PREMULTIPLIED_CNV_TYPE;

typedef enum {
    E_DECODE_FULL_PITCH = 1,
    E_DECODE_MASK       = 1 << 1,
    E_DECODE_OFFSET     = 1 << 2
} GFX_DECODE_TYPE;

typedef enum
{
    GFX_RM_OSD3_NEXT_RIGHT_DRAW_CMD  = 1 << 0,
    GFX_RM_OSD3_NEXT_LEFT_DRAW_CMD   = 1 << 1,
    GFX_RM_OSD2_NEXT_RIGHT_DRAW_CMD  = 1 << 2,
    GFX_RM_OSD2_NEXT_LEFT_DRAW_CMD   = 1 << 3,
    GFX_RM_OSD3_RACING_RIGHT_COMP_EN = 1 << 4,
    GFX_RM_OSD3_RACING_LEFT_COMP_EN  = 1 << 5,
    GFX_RM_OSD2_RACING_RIGHT_COMP_EN = 1 << 6,
    GFX_RM_OSD2_RACING_LEFT_COMP_EN  = 1 << 7,
    GFX_RM_EN                        = 1 << 8,
    GFX_RM_TEST                      = 1 << 9,  /* Just for self test */
} GFX_RACING_MODE;

typedef enum
{
    GFX_MMU_COMPRESS = IOMMU_GFX_COMPRESSION,
    GFX_MMU_OTHER = IOMMU_GFX_OTHER,
} GFX_MMU_TYPE;

//---------------------------------------------------------------------------
// Public definition for driver layer
//---------------------------------------------------------------------------
/*! @name GFX Event Group Define */
/*! @{ */
#ifndef GFX_EV_INITIAL
#define GFX_EV_INITIAL               ((EV_GRP_EVENT_T) 0)
#endif
#ifndef GFX_EV_FLUSH
#define GFX_EV_FLUSH                 ((EV_GRP_EVENT_T)(1) << 1)
#endif
#ifndef GFX_EV_UNINIT
#define GFX_EV_UNINIT                ((EV_GRP_EVENT_T)(1) << 2)
#endif
#ifndef GFX_EV_DONE
#define GFX_EV_DONE                  ((EV_GRP_EVENT_T)(1) << 3)
#endif
#ifndef GFX_EV_HW_DONE
#define GFX_EV_HW_DONE               ((EV_GRP_EVENT_T)(1) << 4)
#endif
#ifndef GFX_EV_FLIP_DONE
#define GFX_EV_FLIP_DONE             ((EV_GRP_EVENT_T)(1) << 5)
#endif
//#define GFX_EV_NEXT_FLIP             ((EV_GRP_EVENT_T)(1) << 6)
/*! @} */

//---------------------------------------------------------------------------
// use to control fastlogo use gfx hw
//---------------------------------------------------------------------------
#define CONFIG_FASTLOGO_GFX 0
#define CONFIG_DRV_3D_256_SUPPORT 0

#if (CONFIG_FASTLOGO_GFX)
extern BOOL _fgFastLogoEnable;
#endif


typedef struct _GFX_DRV_INST_T
{
    UINT16                  u4GfxHwId;               ///< Hw id
    HANDLE_T                hEventHandle;           ///< Event Handle
    UINT32                  u4CmdBufPoint;
    UINT32                  fgUninit;
    UINT32                  u4ThreadPC;
    EV_GRP_EVENT_T          u4WaitEvent;
    BOOL                    fgGfxHwActive;           ///< HW is in active state
    BOOL                    fgNeedCB;
} GFX_DRV_INST_T;

extern GFX_DRV_INST_T   _arGfxDrvInst[GFX_HAL_HW_INST_NUM];
extern BOOL _fgGfxDelay[2];
//---------------------------------------------------------------------------
// Inter-file functions
//---------------------------------------------------------------------------

//extern INT32 _GFX_IsFlushing(UINT32 u4GfxHwId);

//extern void _GFX_SetFlushStatus(UINT32 u4GfxHwId, INT32 i4Status);


//---------------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------------

extern INT32 GFX_CopyTopfieldToBotfield(UINT32 u4GfxHwId, UINT8 *pu1TopStartAddr,
    UINT32 u4TotalLines, UINT32 u4LineWidth);

extern INT32 GFX_Init(void);

extern INT32 i4GFXUninit(void);
extern BOOL GFX_IsInit(void);

extern INT32 GFX_Reset(UINT32 u4GfxHwId, UINT32 u4Reset);

extern INT32 GFX_ResetWT(UINT32 u4GfxHwId, UINT32 u4Reset);
extern INT32 GFX_ResetWTDst(UINT32 u4GfxHwId, UINT32 u4Reset);
extern INT32 GFX_ResetWTSRC(UINT32 u4GfxHwId, UINT32 u4Reset);
extern INT32 GFX_SetWTDst(UINT32 u4GfxHwId, UINT32 u4Reset);
extern INT32 GFX_SetWTSrc(UINT32 u4GfxHwId, UINT32 u4Reset);
extern INT32 GFX_WtYuvEn(UINT32 u4GfxHwId, UINT32 u4SrcEn,UINT32 u4DstEn);
extern INT32 GFX_WtYuvEnSrc(UINT32 u4GfxHwId, UINT32 u4SrcEn);
extern INT32 GFX_WtYuvEnDst(UINT32 u4GfxHwId, UINT32 u4DstEn);

extern void GFX_Wait(void);

extern INT32 GFX_MwFlush(UINT32 u4GfxHwId);

extern INT32 GFX_Flush(UINT32 u4GfxHwId);

extern void GFX_Lock(void);

extern INT32 GFX_TryLock(void);

extern void GFX_Unlock(void);

extern void GFX_LockCmdque(UINT32 u4GfxHwId);

extern void GFX_UnlockCmdque(UINT32 u4GfxHwId);

extern void GFX_LockResz(void);

extern void GFX_UnlockResz(void);

extern void GFX_LockBMP(void);

extern void GFX_UnlockBMP(void);

extern void GFX_LockBuffer(void);

extern void GFX_UnlockBuffer(void);

extern void GFX_LockHeight(void);

extern void GFX_UnlockHeight(void);

extern INT32 GFX_QueryHwIdle(UINT32 u4GfxHwId);

extern void GFX_SetNotify(UINT32 u4GfxHwId, void (*pfnNotifyFunc)(UINT32));

extern INT32 GFX_SetDst(UINT32 u4GfxHwId, UINT8 *pu1Base, UINT32 u4ColorMode, UINT32 u4Pitch);

extern INT32 GFX_SetSrc(UINT32 u4GfxHwId, UINT8 *pu1Base, UINT32 u4ColorMode, UINT32 u4Pitch);
extern INT32 GFX_Set2ndSrc(UINT32 u4GfxHwId, UINT8 *pu1Base, UINT32 u4ColorMode, UINT32 u4Pitch);
extern INT32 GFX_Set2ndSrcEnable(UINT32 u4GfxHwId, UINT32 u4ColorMode);

extern INT32 GFX_SetCharSrcBase(UINT32 u4GfxHwId, UINT8 *pu1Base, UINT32 u4ColorMode, UINT32 u4Pitch);

extern INT32 GFX_SetAlpha(UINT32 u4GfxHwId, UINT32 u4Alpha);

extern INT32 GFX_24BPP24BPP_SetAlpha(UINT32 u4GfxHwId, UINT32 u4Alpha, UINT32 u4GAlphaEn);

extern INT32 GFX_SetColor(UINT32 u4GfxHwId, UINT32 u4Color);

#if CONFIG_DRV_3D_256_SUPPORT
extern INT32 GFX_InitMMUTable(UINT32 u4GfxHwId);
#endif
extern INT32 GFX_Fill(UINT32 u4GfxHwId, UINT32 u4X, UINT32 u4Y, UINT32 u4Width, UINT32 u4Height);
extern INT32 GFX_Draw(UINT32 u4GfxHwId, UINT32 u4X, UINT32 u4Y, UINT32 u4Width, UINT32 u4Height);
extern INT32 GFX_DrawCompose(UINT32 u4GfxHwId, UINT32 u4X, UINT32 u4Y, UINT32 u4Width, UINT32 u4Height,
                             UINT32 u4Ar, UINT32 u4OpCode, UINT32 u4RectSrc);
extern INT32 GFX_FillTriangle(UINT32 u4GfxHwId, UINT32 u4X1, UINT32 u4Y1, UINT32 u4X2, UINT32 u4Y2, UINT32 u4X3, UINT32 u4Y3);
extern INT32 GFX_FillTriangleCompose(UINT32 u4GfxHwId, UINT32 u4X1, UINT32 u4Y1, UINT32 u4X2, UINT32 u4Y2, UINT32 u4X3, UINT32 u4Y3,
                              UINT32 u4Ar, UINT32 u4OpCode, UINT32 u4RectSrc);

extern INT32 GFX_FillChar(void *pvBase, UINT8 u1Char, UINT32 u4NumWrd);
extern INT32 GFX_SetWTBase(UINT32 u4GfxHwId, UINT32 u4X, UINT32 u4Y);
extern INT32 GFX_WtBypass(UINT32 u4GfxHwId, UINT32 u4SrcBypass, UINT32 u4DstBypass);
extern INT32 GFX_SetCBCRWBBase(UINT32 u4GfxHwId, UINT8 *pu1SrcChroma);
extern INT32 GFX_HLine(UINT32 u4GfxHwId, UINT32 u4X, UINT32 u4Y, UINT32 u4Width);

extern INT32 GFX_VLine(UINT32 u4GfxHwId, UINT32 u4X, UINT32 u4Y, UINT32 u4Height);

extern INT32 GFX_ObliqueLine(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX, UINT32 u4DstY);
extern INT32 GFX_SetBltOptEx(UINT32 u4GfxHwId, UINT32 u4Switch, UINT32 u4DisSrcKey, UINT32 u4SrcKeyIn, UINT32 u4ColorMin, UINT32 u4ColorMax, UINT32 u4DisDstKey, UINT32 u4DstKeyIn, UINT32 u4DstColorMin, UINT32 u4DstColorMax);
extern INT32 GFX_SetBltOpt(UINT32 u4GfxHwId, UINT32 u4Switch, UINT32 u4ColorMin,
    UINT32 u4ColorMax);

extern INT32 GFX_SetBltOpt2(UINT32 u4GfxHwId, UINT32 u4Enable, UINT32 u4Y2R, UINT32 uEn709);
extern INT32 GFX_24Bpp24BPP_SetBltOpt(UINT32 u4GfxHwId, UINT32 u4Switch, UINT32 u4ColorMin, UINT32 u4ColorMax);

extern INT32 GFX_BitBlt(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX,
    UINT32 u4DstY, UINT32 u4Width, UINT32 u4Height);
extern INT32 GFX_24Bpp24BPP_BitBlt(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX,
    UINT32 u4DstY, UINT32 u4Width, UINT32 u4Height);
extern INT32 GFX_BitBlt2(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX, UINT32 u4DstY,
    UINT32 u4Width, UINT32 u4Height, UINT32 u4op);
extern INT32 GFX_BitBlt_NewMethod(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX, UINT32 u4DstY,
    UINT32 u4Width, UINT32 u4Height);
extern INT32 GFX_Blend(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX,
    UINT32 u4DstY, UINT32 u4Width, UINT32 u4Height);
extern INT32 GFX_MsBlend(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX,
    UINT32 u4DstY, UINT32 u4Width, UINT32 u4Height, UINT32 u4MsSel);  

extern INT32 GFX_SetBuf(UINT32 u4GfxHwId, UINT32 *pu4GfxWorkingBuf, UINT32 u4Size);


extern INT32 GFX_ComposeLoop(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX,
    UINT32 u4DstY, UINT32 u4Width, UINT32 u4Height, UINT32 u4Ar,
    UINT32 u4OpCode, UINT32 u4RectSrc);
extern INT32 GFX_NormalComposeLoop(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX,
    UINT32 u4DstY, UINT32 u4Width, UINT32 u4Height, UINT32 u4Ar,
    UINT32 u4OpCode, UINT32 u4RectSrc);
#if 0 //(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
extern INT32 GFX_FlashLiteNormalComposeBitblt(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX,
                                              UINT32 u4DstY, UINT32 u4Width, UINT32 u4Height,
                                              UINT32 u4PremultSrcRdEn, UINT32 u4PremultDstWrEn);
extern INT32 GFX_FlashLiteStretchBitblt(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                                 UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH, UINT32 u4PremultSrcRdEn,
                                 UINT32 u4PremultDstWrEn, UINT32 u4AlcomAr, UINT32 u4OpCode);
extern INT32 GFX_FlashLiteStretchCompose(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                             UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH, UINT32 u4PremultSrcRdEn,
                             UINT32 u4PremultDstRdEn, UINT32 u4PremultDstWrEn, UINT32 u4AlcomAr, UINT32 u4OpCode);
extern INT32 GFX_FlashLiteFillColor(UINT32 u4GfxHwId, UINT32 u4X, UINT32 u4Y, UINT32 u4Width, UINT32 u4Height, UINT32 u4PremultSrcRdEn);
extern INT32 GFX_FlashLiteComposeBitblt(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX, UINT32 u4DstY,
                                 UINT32 u4Width, UINT32 u4Height, UINT32 u4PremultSrcRdEn, UINT32 u4PremultDstRdEn,
                                 UINT32 u4PremultDstWrEn, UINT32 u4AlcomAr, UINT32 u4OpCode, UINT32 u4RectSrc);

extern INT32 GFX_2SrcBlendingBitblt(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX,
                      UINT32 u4DstY, UINT32 u4Width, UINT32 u4Height, UINT32 u4Ar,
                      UINT32 u4OpCode, UINT32 u4RectSrc, UINT32 u4Src2En);
extern INT32 GFX_BpCompressEncode(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                     UINT32 u4DstX, UINT32 u4DstY, UINT32 u4PixelWidth, UINT32 u4QulityMode,
                     UINT32 u4PixelSeparate, UINT8 *pu1DstIdxAddr);
extern INT32 GFX_BpCompressEncodeCheckEndAddr(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                     UINT32 u4DstX, UINT32 u4DstY, UINT32 u4PixelWidth, UINT32 u4QulityMode,
                     UINT32 u4PixelSeparate, UINT8 *pu1DstIdxAddr, UINT32 u4EndAddr);
extern INT32 GFX_BpDecode(UINT32 u4GfxHwId, UINT32 u4DecodeType,
                          UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                          UINT32 u4DstX, UINT32 u4DstY, UINT32 u4PicW, UINT32 u4PicH,
                          UINT32 u4PixelWidth, UINT32 u4PacketWidth, UINT32 u4QulityMode,
                          UINT8 *pu1DstIdxAddr);
extern INT32 GFX_I2DMFlipComposeLoop(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX, UINT32 u4DstY,
                UINT32 u4Width, UINT32 u4Height, UINT32 u4SrcPitchEn, UINT8 *pv_pal_base, UINT32 u4MsbLeft,
                UINT32 u4StartByteAligned, UINT32 u4AlComNormal, UINT32 u4Ar, UINT32 u4MFOp, UINT32 u4OpCode, UINT32 u4RectSrc);
extern INT32 GFX_I2DStretchMFlipComposeLoop(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH, UINT32 u4SrcPitchEn,
                UINT8 *pv_pal_base, UINT32 u4MsbLeft, UINT32 u4StartByteAligned,
                UINT32 u4AlComNormal, UINT32 u4Ar, UINT32 u4MFOp, UINT32 u4OpCode, UINT32 u4RectSrc);
extern INT32 GFX_I2DRotateMFlipComposeLoop(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX, UINT32 u4DstY, UINT32 u4SrcW, UINT32 u4SrcH, UINT32 u4SrcPitchEn,
                              UINT8 *pv_pal_base, UINT32 u4MsbLeft, UINT32 u4StartByteAligned,
                              UINT32 u4AlComNormal, UINT32 u4Ar, UINT32 u4Is90CCW, UINT32 u4MFOp, UINT32 u4OpCode, UINT32 u4RectSrc);
extern INT32 GFX_I2DStretchRotateMFlipComposeLoop(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
        UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH, UINT32 u4SrcPitchEn,
        UINT8 *pv_pal_base, UINT32 u4MsbLeft, UINT32 u4StartByteAligned,
        UINT32 u4AlComNormal, UINT32 u4Ar, UINT32 u4Is90CCW, UINT32 u4MFOp, UINT32 u4OpCode, UINT32 u4RectSrc);

#endif
extern INT32 GFX_ComposeLoopEx(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX,
    UINT32 u4DstY, UINT32 u4Width, UINT32 u4Height, UINT32 u4AlComNormal, UINT32 u4Ar,
    UINT32 u4OpCode, UINT32 u4RectSrc);
extern INT32 GFX_StretchRotate(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW,
    UINT32 u4SrcH, UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH, UINT32 u4Is90CCW);
extern INT32 GFX_StretchMFlip(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW,
    UINT32 u4SrcH, UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH, UINT32 u4MFOp);
extern INT32 GFX_StretchRotateMFlip(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW,
    UINT32 u4SrcH, UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH, UINT32 u4Is90CCW,
    UINT32 u4MFOp);
extern INT32 GFX_AlphaComposeLoopRotate(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY,
                                 UINT32 u4DstX, UINT32 u4DstY, UINT32 u4Width,
                                 UINT32 u4Height, UINT32 u4Ar, UINT32 u4OpCode,
                                 UINT32 u4RectSrc, UINT32 u4AlcomNormal,
                                 UINT32 u4Is90CCW);
extern INT32 GFX_AlphaComposeLoopMFlip(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY,
                                UINT32 u4DstX, UINT32 u4DstY, UINT32 u4Width,
                                UINT32 u4Height, UINT32 u4Ar, UINT32 u4OpCode,
                                UINT32 u4RectSrc, UINT32 u4AlcomNormal,
                                UINT32 u4MFOp);
extern INT32 GFX_AlphaComposeLoopRotateMFlip(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY,
                                      UINT32 u4DstX, UINT32 u4DstY, UINT32 u4Width,
                                      UINT32 u4Height, UINT32 u4Ar, UINT32 u4OpCode,
                                      UINT32 u4RectSrc, UINT32 u4AlcomNormal,
                                      UINT32 u4Is90CCW, UINT32 u4MFOp);
extern INT32 GFX_ComposeLoop2ALU(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX,
    UINT32 u4DstY, UINT32 u4Width, UINT32 u4Height, UINT32 u4Ar,
    UINT32 u4OpCode, UINT32 u4RectSrc);
extern INT32 GFX_NormalComposeLoop2ALU(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX,
    UINT32 u4DstY, UINT32 u4Width, UINT32 u4Height, UINT32 u4Ar,
    UINT32 u4OpCode, UINT32 u4RectSrc);
extern INT32 GFX_Compose(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX,
    UINT32 u4DstY, UINT32 u4Width, UINT32 u4Height, UINT32 u4Ar, UINT32 u4Mode);

extern INT32 GFX_AlphaComposePass(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY,
    UINT32 u4DstX, UINT32 u4DstY, UINT32 u4Width, UINT32 u4Height,
    UINT32 u4Pass, UINT32 u4Param);
extern INT32 GFX_NormalComposePass(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY,
    UINT32 u4DstX, UINT32 u4DstY, UINT32 u4Width, UINT32 u4Height,
    UINT32 u4Pass, UINT32 u4Param);
extern INT32 GFX_SetColCnvFmt(UINT32 u4GfxHwId, UINT32 u4YCFmt, UINT32 u4SwapMode,
    UINT32 u4VidStd, UINT32 u4VidSys);

extern INT32 GFX_SetColCnvSrc(UINT32 u4GfxHwId, UINT8 *pu1SrcLuma, UINT32 u4SrcLumaPitch,
    UINT8 *pu1SrcChroma, UINT32 u4SrcChromaPitch, UINT32 u4FieldPic);

extern INT32 GFX_ColConv(UINT32 u4GfxHwId, UINT32 u4X, UINT32 u4Y, UINT32 u4Width,
    UINT32 u4Height);

extern INT32 GFX_StretchBlt(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW,
    UINT32 u4SrcH, UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH);

extern INT32 GFX_AlphaMapBitBlt(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY,
    UINT32 u4DstX, UINT32 u4DstY, UINT32 u4Width, UINT32 u4Height);

extern INT32 GFX_DMA(UINT32 *pu4Dst, UINT32 *pu4Src, UINT32 u4NumWrd);

extern INT32 GFX_SetBmp(UINT32 u4GfxHwId, UINT8 *puu1Base, UINT32 u4BmpColorMode, UINT32 u4Width,
    UINT32 u4Height, UINT32 u4RunLen);

extern INT32 GFX_ColorMap(UINT32 u4GfxHwId, UINT32 u4BmpCM, UINT32 u4CM,
    const UINT32 *psu4ColorMap);

extern INT32 GFX_Bmp(UINT32 u4GfxHwId, UINT32 u4X, UINT32 u4Y);

//extern INT32 GFX_SetPgigSrcQDecode(void *pvSrcQBase, UINT32 u4QLen, UINT32 u4QUint);
extern INT32 GFX_SetPgigDecodeSrcQ(UINT8 *pu1SrcQBase, UINT32 u4DataLen, UINT32 u4QLen, UINT32 u4QUnit);
extern INT32 GFX_SetPgigDstQDecode(void *pvSrcQBase, UINT32 u4QLen, UINT32 u4QUint);

//extern INT32 GFX_PgigDecode(UINT32 u4CharCm, VOID * pvSrc, UINT32 u4DstX, UINT32 u4DstY,
//    UINT32 u4Width, UINT32 u4Height);
extern INT32 GFX_SetPgigDecodePalette(void *pvPalBase, UINT32 u4PalSize);
extern INT32 GFX_SetPgigDecodePalette2(void *pvPalBase, UINT32 u4PalSize);
extern INT32 GFX_PgigDecode(UINT32 u4DstX, UINT32 u4DstY, UINT32 u4Width, UINT32 u4Height);

//extern INT32 GFX_PgigSrcQDecode(void);

extern INT32 GFX_RopBitblt(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX, UINT32 u4DstY,
    UINT32 u4Width, UINT32 u4Height, UINT32 u4RopCode);

extern INT32 GFX_Idx2DirBitblt(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX, UINT32 u4DstY,
    UINT32 u4Width, UINT32 u4Height, UINT8 *pv_pal_base,
    UINT32 u4MsbLeft, UINT32 u4StartByteAligned);
extern INT32 GFX_Idx2DirBitbltEx(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX, UINT32 u4DstY,
    UINT32 u4Width, UINT32 u4Height, UINT32 u4SrcPitchEn, UINT8 *pv_pal_base,
    UINT32 u4MsbLeft, UINT32 u4StartByteAligned);
extern INT32 GFX_SetHoriToVertLineOpt(UINT32 u4GfxHwId, UINT32 u4IsCounterClockWise);

extern INT32 GFX_HoriToVertLine(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX,
    UINT32 u4DstY, UINT32 u4HoriLineWidth, INT32 u4Is90CCW);

extern INT32 GFX_RotateBmp(UINT32 u4GfxHwId, UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
        UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX, UINT32 u4DstY,
    UINT32 u4CM, UINT32 u4SrcPitch, UINT32 u4DstPitch,
    UINT32 u4Width, UINT32 u4Height, UINT32 u4Is90CCW);
extern INT32 GFX_RotateMirrorFlipBmp(UINT32 u4GfxHwId, UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX, UINT32 u4DstY,
    UINT32 u4CM, UINT32 u4SrcPitch, UINT32 u4DstPitch,
    UINT32 u4Width, UINT32 u4Height, UINT32 u4Is90CCW,UINT8 u1AddOpt);

extern INT32 GFX_RotateBmpSw(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
        UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX, UINT32 u4DstY,
    UINT32 u4CM, UINT32 u4SrcPitch, UINT32 u4DstPitch,
    UINT32 u4Width, UINT32 u4Height, UINT32 u4Is90CCW);

extern INT32 GFX_2DMemCompare(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX,
    UINT32 u4DstY, UINT32 u4Width, UINT32 u4Height);

extern INT32 GFX_GetMemCompareResult(UINT32 u4GfxHwId);

extern INT32 GFX_MemCompare(UINT32 u4GfxHwId, UINT8 *pu1SrcBase, UINT8 *pu1DstBase, UINT32 u4Size);
extern INT32 GFX_SetGradOpt(UINT32 u4GfxHwId, UINT32 u4IncX, UINT32 u4IncY,
  const UINT32 asu4DeltaX[4], const UINT32 asu4DeltaY[4]);
extern INT32 GFX_GradFill(UINT32 u4GfxHwId, UINT32 u4X, UINT32 u4Y, UINT32 u4Width, UINT32 u4Height,
    UINT32 u4Mode);
//#if (CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8550)
#if 1
INT32 GFX_SetClipOpt(UINT32 u4GfxHwId, UINT32 u4ClipEnMask, UINT32 u4DstX, UINT32 u4DstY, UINT32 u4ClipTop, UINT32 u4ClipBot, UINT32 u4ClipLeft, UINT32 u4ClipRight);

INT32 GFX_BurstEn(UINT32 u4GfxHwId, UINT32 u4BurstEn, UINT32 u4BurstMode);
INT32 GFX_StretchComposeLoop(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                             UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH,
                             UINT32 u4AlComNormal, UINT32 u4Ar, UINT32 u4OpCode, UINT32 u4RectSrc);
INT32 GFX_I2DComposeLoop(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX, UINT32 u4DstY, UINT32 u4Width, UINT32 u4Height, UINT32 u4SrcPitchEn,
                         UINT8 *pv_pal_base, UINT32 u4MsbLeft, UINT32 u4StartByteAligned,
                         UINT32 u4AlComNormal, UINT32 u4Ar, UINT32 u4OpCode, UINT32 u4RectSrc);
INT32 GFX_StretchRotateComposeLoop(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                                   UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH,
                                   UINT32 u4Is90CCW, UINT32 u4AlComNormal, UINT32 u4Ar, UINT32 u4OpCode, UINT32 u4RectSrc);
INT32 GFX_RopJavaXorMFilp(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                          UINT32 u4DstX, UINT32 u4DstY, UINT32 u4MFOp, UINT32 u4JavaXorClr);
INT32 GFX_StretchJavaXor(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                         UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH, UINT32 u4JavaXorClr);
INT32 GFX_StretchRotateJavaXor(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                               UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH, UINT32 u4Is90CCW, UINT32 u4JavaXorClr);
INT32 GFX_BpCompressEx(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                     UINT32 u4DstX, UINT32 u4DstY, UINT32 u4RollBackEn, UINT32 u4QulityMode, UINT32 u4LineSeparate, UINT32 u4Size);
INT32 GFX_BpCompress(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                     UINT32 u4DstX, UINT32 u4DstY, UINT32 u4RollBackEn, UINT32 u4QulityMode, UINT32 u4LineSeparate);
INT32 GFX_BpCompressBase(UINT32 u4GfxHwId, UINT32 u4SrcW, UINT32 u4RollBackEn, UINT32 u4QulityMode, UINT32 u4LineSeparate);
INT32 GFX_BpCompressSpecial(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY,
UINT32 u4DstX, UINT32 u4DstY, UINT32 u4SrcW, UINT32 u4SrcH);

INT32 GFX_PremultipliedConvert(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX, UINT32 u4DstY, 
                               UINT32 u4Width, UINT32 u4Height, GFX_PREMULTIPLIED_CNV_TYPE eCvtType);

INT32 GFX_RopBitbltEx(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX, UINT32 u4DstY,
                      UINT32 u4Width, UINT32 u4Height, UINT32 u4RopCode, UINT32 u4JavaXorClr);
INT32 GFX_StretchMFlipComposeLoop(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                                       UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH,
                                       UINT32 u4MFOp, UINT32 u4AlComNormal, UINT32 u4Ar, UINT32 u4OpCode, UINT32 u4RectSrc);
INT32 GFX_StretchMFlipJavaXor(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                              UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH,
                              UINT32 u4MFOp, UINT32 u4JavaXorClr);
INT32 GFX_StretchRotateMFlipJavaXor(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                                    UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH,
                                    UINT32 u4Is90CCW, UINT32 u4MFOp, UINT32 u4JavaXorClr);
INT32 GFX_StretchRotateMFlipComposeLoop(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                                        UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH,
                                        UINT32 u4Is90CCW, UINT32 u4MFOp, UINT32 u4AlComNormal, UINT32 u4Ar, UINT32 u4OpCode, UINT32 u4RectSrc);
#endif
INT32 GFX_YCbCr2RGBAlphaCompose(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX, UINT32 u4DstY, UINT32 u4Width, UINT32 u4Height,
            UINT8 *p1LumaSA, UINT32 u4SrcLumaPitch, UINT8 *p1ChromaSA, UINT32 u4SrcChromaPitch,
            UINT32 u4YCFmt, UINT32 u4SwapMode, UINT32 u4VidStd, UINT32 u4VidSys, UINT32 u4VsClip, UINT32 u4FldPic,
            UINT32 u4AlComNormal, UINT32 u4Ar, UINT32 u4OpCode, UINT32 u4RectSrc);
INT32 GFX_YCbCr2RGBStretchAlphaCompose(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4Width, UINT32 u4Height,
            UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstWidth, UINT32 u4DstHeight,
            UINT8 *p1LumaSA, UINT32 u4SrcLumaPitch, UINT8 *p1ChromaSA, UINT32 u4SrcChromaPitch,
            UINT32 u4YCFmt, UINT32 u4SwapMode, UINT32 u4VidStd, UINT32 u4VidSys, UINT32 u4VsClip, UINT32 u4FldPic,
            UINT32 u4AlComNormal, UINT32 u4Ar, UINT32 u4OpCode, UINT32 u4RectSrc);
INT32 GFX_YCbCr2RGBStretchMFlipAlphaCompose(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4Width, UINT32 u4Height,
            UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstWidth, UINT32 u4DstHeight,
            UINT8 *p1LumaSA, UINT32 u4SrcLumaPitch, UINT8 *p1ChromaSA, UINT32 u4SrcChromaPitch,
            UINT32 u4YCFmt, UINT32 u4SwapMode, UINT32 u4VidStd, UINT32 u4VidSys, UINT32 u4VsClip, UINT32 u4FldPic,
            UINT32 u4AlComNormal, UINT32 u4Ar, UINT32 u4MFOpt, UINT32 u4OpCode, UINT32 u4RectSrc);
INT32 GFX_YCbCr2RGBStretchJavaXor(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4Width, UINT32 u4Height,
            UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstWidth, UINT32 u4DstHeight,
            UINT8 *p1LumaSA, UINT32 u4SrcLumaPitch, UINT8 *p1ChromaSA, UINT32 u4SrcChromaPitch,
            UINT32 u4YCFmt, UINT32 u4SwapMode, UINT32 u4VidStd, UINT32 u4VidSys, UINT32 u4VsClip, UINT32 u4FldPic,
            UINT32 u4JavaXorClr);
INT32 GFX_LosslessDecodeComposeLoop(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY,
    UINT32 u4DstX, UINT32 u4DstY, UINT32 u4Width, UINT32 u4Height, UINT32 u4Ar, UINT32 u4OpCode,
    UINT32 u4RectSrc, UINT32 u4PixelWidth, UINT32 u4PacketWidth, UINT32 u4QulityMode, UINT8 *p1IdxSA);
INT32 GFX_LosslessDecodeStretchBitblt(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                                              UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH, UINT32 u4PixelWidth,
                                              UINT32 u4PacketWidth, UINT32 u4QulityMode, UINT8 *p1IdxSA);
INT32 GFX_LosslessDecodeStretchCompose(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                                       UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH, UINT32 u4AlcomNormal, UINT32 u4Ar, UINT32 u4OpCode,
                                       UINT32 u4RectSrc, UINT32 u4PicWidth, UINT32 u4PicHeight, UINT32 u4PixelWidth,
                                       UINT32 u4PacketWidth, UINT32 u4QulityMode, UINT8 *p1IdxSA);
INT32 GFX_LosslessDecodeStretchFlipCompose(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                                           UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH, UINT32 u4AlcomNormal, UINT32 u4Ar, UINT32 u4OpCode,
                                           UINT32 u4RectSrc, UINT32 u4PicWidth, UINT32 u4PicHeight, UINT32 u4PixelWidth,
                                           UINT32 u4PacketWidth, UINT32 u4QulityMode, UINT8 *p1IdxSA);
INT32 GFX_LosslessDecodeStretchRotateCompose(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                                             UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH, UINT32 u4AlcomNormal, UINT32 u4Ar, UINT32 u4OpCode,
                                             UINT32 u4RectSrc, UINT32 u4Is90CCW, UINT32 u4PicWidth, UINT32 u4PicHeight, UINT32 u4PixelWidth,
                                             UINT32 u4PacketWidth, UINT32 u4QulityMode, UINT8 *p1IdxSA);
INT32 GFX_LosslessDecodeStretchRotateFlipCompose(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                                                 UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH, UINT32 u4AlcomNormal, UINT32 u4Ar, UINT32 u4OpCode,
                                                 UINT32 u4RectSrc, UINT32 u4Is90CCW, UINT32 u4PicWidth, UINT32 u4PicHeight, UINT32 u4PixelWidth,
                                                 UINT32 u4PacketWidth, UINT32 u4QulityMode, UINT8 *p1IdxSA);
INT32 GFX_LosslessDecodeStretchJavaXor(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                                       UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH, UINT32 u4JavaXorColor,
                                       UINT32 u4PicWidth, UINT32 u4PicHeight, UINT32 u4PixelWidth,
                                       UINT32 u4PacketWidth, UINT32 u4QulityMode, UINT8 *p1IdxSA);
INT32 GFX_LosslessDecodeStretchFlipJavaXor(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                                           UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH, UINT32 u4JavaXorColor,
                                           UINT32 u4PicWidth, UINT32 u4PicHeight, UINT32 u4PixelWidth,
                                           UINT32 u4PacketWidth, UINT32 u4QulityMode, UINT8 *p1IdxSA);
INT32 GFX_LosslessDecodeStretchRotateJavaXor(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                                             UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH,
                                             UINT32 u4JavaXorColor, UINT32 u4Is90CCW,
                                             UINT32 u4PicWidth, UINT32 u4PicHeight, UINT32 u4PixelWidth,
                                             UINT32 u4PacketWidth, UINT32 u4QulityMode, UINT8 *p1IdxSA);
INT32 GFX_LosslessDecodeStretchRotateFlipJavaXor(UINT32 u4GfxHwId, UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4SrcW, UINT32 u4SrcH,
                                                 UINT32 u4DstX, UINT32 u4DstY, UINT32 u4DstW, UINT32 u4DstH,
                                                 UINT32 u4JavaXorColor, UINT32 u4Is90CCW,
                                                 UINT32 u4PicWidth, UINT32 u4PicHeight, UINT32 u4PixelWidth,
                                                 UINT32 u4PacketWidth, UINT32 u4QulityMode, UINT8 *p1IdxSA);
INT32 GFX_SetRacingMode(UINT32 u4GfxHwId, UINT32 u4Mode);
INT32 GFX_MMUEnable(UINT32 u4GfxHwId, UINT32 u4Enable);
INT32 GFX_MMUCfgA0(UINT32 u4GfxHwId, UINT32 u4En, UINT32 u4Mid, UINT32 u4Prefetch, UINT32 u42DPrefetch,
                   UINT32 u4PrefetchDec, UINT32 u4TwoWay, UINT32 u4Mid2nd);
INT32 GFX_MMUCfgA1(UINT32 u4GfxHwId, UINT32 u4En, UINT32 u4Mid, UINT32 u4Prefetch, UINT32 u42DPrefetch,
                   UINT32 u4PrefetchDec, UINT32 u4TwoWay, UINT32 u4Mid2nd);
INT32 GFX_MMUCfgA2(UINT32 u4GfxHwId, UINT32 u4En, UINT32 u4Mid, UINT32 u4Prefetch, UINT32 u42DPrefetch,
                   UINT32 u4PrefetchDec, UINT32 u4TwoWay, UINT32 u4Mid2nd);
INT32 GFX_MMUCfgA3(UINT32 u4GfxHwId, UINT32 u4En, UINT32 u4Mid, UINT32 u4Prefetch, UINT32 u42DPrefetch,
                   UINT32 u4PrefetchDec, UINT32 u4TwoWay, UINT32 u4Mid2nd);
INT32 GFX_MMUCfg(UINT32 u4GfxHwId, UINT32 u4Enable,	UINT32 u4SrcRead, UINT32 u4DstRead, UINT32 u4DstWrite);
INT32 GFX_MMUCfgFill(UINT32 u4GfxHwId, UINT32 u4Enable,	UINT32 u4SrcRead, UINT32 u4DstRead, UINT32 u4DstWrite);
INT32 GFX_MMUCfgEx(UINT32 u4GfxHwId, UINT32 u4Enable,	UINT32 u4SrcRead, UINT32 u4DstRead, UINT32 u4DstWrite);
INT32 GFX_MMUCfgForCompressFunc(UINT32 u4GfxHwId, UINT32 u4Enable, UINT32 u4SrcRead, UINT32 u4DstRead, UINT32 u4DstWrite);
INT32 GFX_MMUOverReadProtection(UINT32 u4GfxHwId, UINT32 u4Enable, GFX_MMU_TYPE eType, UINT32 u4Addr1, UINT32 u4Size1, UINT32 u4Addr2, UINT32 u4Size2);
INT32 GFX_MMUOverReadProtectionEx(UINT32 u4GfxHwId, UINT32 u4Enalbe, GFX_MMU_TYPE eType, UINT32 u4Addr1, UINT32 u4Size1, UINT32 u4Addr2, UINT32 u4Size2);
INT32 GFX_MMUCfgForY2RFunc(UINT32 u4GfxHwId, UINT32 u4Enable, UINT32 u4SrcYRead, UINT32 u4SrcCRead, UINT32 u4DstWrite);
//extern INT32 u4NewGfxTask(void);

// for debug use
//#if defined(GFX_DEBUG_MODE)
#if 1
extern INT32 GFX_Reset_Engine(UINT32 u4GfxHwId);

extern INT32 GFX_Reset_CmdQue(UINT32 u4GfxHwId);

extern void GFX_DumpDebugInfo(void);

extern void GFX_QueryCmdQueInfo(UINT32 u4GfxHwId);

extern INT32 GFX_SetLegalAddress(UINT32 u4GfxHwId, UINT32 u4Start, UINT32 u4End);
#endif // #if defined(GFX_DEBUG_MODE)


extern INT32 GFX_Memset(UINT32 u4GfxHwId, UINT8 *pu1Addr, UINT32 u4Size, UINT32 u4Color);

extern UINT32 GFX_QueryFlushCount(UINT32 u4GfxHwId);

extern UINT32 GFX_QueryHwInterruptCount(UINT32 u4GfxHwId);

extern void GFX_SetCqCapacity(UINT32 u4GfxHwId, INT32 i4Capacity);
#ifdef GL_XOR

#if CONFIG_SYS_MEM_PHASE3
HANDLE_T GFX_GetWorkingBufferMemHandle(UINT32 u4GfxHwId);
HANDLE_T GFX_GetTopWorkingBufferMemHandle(UINT32 u4GfxHwId);
HANDLE_T GFX_GetBottomWorkingBufferMemHandle(UINT32 u4GfxHwId);
#endif

void GFX_GetWorkingBufferHeight(UINT32 u4GfxHwId, void** ppvBuffer, UINT32 ui4_pitch,UINT32 *pui4_height);
void GFX_GetWorkingBufferWidth(UINT32 u4GfxHwId, UINT32 ui4_height,UINT32 *pui4_width);
void GFX_GetTopWorkingBufferHeight(UINT32 u4GfxHwId, void** ppvBuffer, UINT32 ui4_pitch,UINT32 *pui4_height);
void GFX_GetTopWorkingBufferWidth(UINT32 u4GfxHwId, UINT32 ui4_height,UINT32 *pui4_width);
void GFX_GetBottomWorkingBufferHeight(UINT32 u4GfxHwId,void * * ppvBuffer,UINT32 ui4_pitch,UINT32 * pui4_height);
void GFX_GetBottomWorkingBufferWidth(UINT32 u4GfxHwId, UINT32 ui4_height,UINT32 *pui4_width);
//void GFX_GetWorkingBuffer2Height(UINT32 u4GfxHwId, void** ppvBuffer, UINT32 ui4_pitch,UINT32 *pui4_height);
//void GFX_GetWorkingBuffer2Width(UINT32 u4GfxHwId, UINT32 ui4_height,UINT32 *pui4_width);
INT32 GFX_MirrorFlip(UINT32 u4GfxHwId,
    UINT32 u4SrcX,
    UINT32 u4SrcY,
    UINT32 u4DstX,
    UINT32 u4DstY,
    UINT32 u4Width,
    UINT32 u4Height,
    EGFX_BITBLT_OPT_T u4Opt,
    UINT8 u1AddOpt);
INT32 GFX_RotateMirrorFlip(UINT32 u4GfxHwId, UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    UINT32 u4SrcX, UINT32 u4SrcY, UINT32 u4DstX, UINT32 u4DstY,
    UINT32 u4CM, UINT32 u4SrcPitch, UINT32 u4DstPitch,
    UINT32 u4Width, UINT32 u4Height, UINT32 u4Is90CCW,EGFX_BITBLT_OPT_T u4Opt);


#endif
INT32 GFX_MMUCfgEx(UINT32 u4GfxHwId, UINT32 u4Enable, UINT32 u4SrcRead, UINT32 u4DstRead, UINT32 u4DstWrite);
INT32 vGfxSetMMU(BOOL fgMMUEnble);
VOID vIIOMMU_PrintAllPageTables(UINT32 ui4_vmem, UINT32 ui4_mem_sz);

INT32 i4GfxSetMmuWaitMode(UINT32 u4Mode);
INT32 i4GfxSetMmuGlbBypss(UINT32 u4Mode);
INT32 GFX_SetMmuWaitMode(UINT32 u4GfxHwId, UINT32 u4Mode);
INT32 i4GfxSetMmuSelfFire(UINT32 u4GfxHwId);
INT32 GFX_SetPreColorize(UINT32 u4GfxHwId, UINT32 u4PreColorize, UINT32 u4ColorRepEn, UINT32 u4ColorRep);
//#endif // GFX_IF_H ---???not the file end

extern INT32 i4GfxDrvInstInit(void);

INT32 GFX_SetLosslessMultiRegionEnable(UINT32 u4GfxHwId, UINT32 u4Enable);
INT32 GFX_SetLosslessRegionDisableAll(UINT32 u4GfxHwId);
INT32 GFX_SetLosslessDecSettings(UINT32 u4GfxHwId, UINT32 u4Y, UINT32 u4RegionCount, UINT32 *pu4Height, UINT32 *pu4Addr);
INT32 GFX_GetStartRegion(UINT32 *pu4Height, UINT32 u4Y, UINT32 u4RegionCnt);
INT32 GFX_SetLosslessDecSettings_new(UINT32 u4GfxHwId, UINT32 u4Y, UINT32 u4RegionCount, UINT32 *pu4Height, UINT32 *pu4Addr);
#endif // GFX_IF_H


