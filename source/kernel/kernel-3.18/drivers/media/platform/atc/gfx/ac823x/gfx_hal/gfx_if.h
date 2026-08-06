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
    uint32_t                  u4CmdBufPoint;
    uint32_t                  fgUninit;
    uint32_t                  u4ThreadPC;
    EV_GRP_EVENT_T          u4WaitEvent;
    BOOL                    fgGfxHwActive;           ///< HW is in active state
    BOOL                    fgNeedCB;
} GFX_DRV_INST_T;

extern GFX_DRV_INST_T   _arGfxDrvInst[GFX_HAL_HW_INST_NUM];
extern BOOL _fgGfxDelay[2];
//---------------------------------------------------------------------------
// Inter-file functions
//---------------------------------------------------------------------------

//extern INT32 _GFX_IsFlushing(uint32_t u4GfxHwId);

//extern void _GFX_SetFlushStatus(uint32_t u4GfxHwId, INT32 i4Status);


//---------------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------------

extern INT32 GFX_CopyTopfieldToBotfield(uint32_t u4GfxHwId, UINT8 *pu1TopStartAddr,
    uint32_t u4TotalLines, uint32_t u4LineWidth);

extern INT32 GFX_Init(void);

extern INT32 i4GFXUninit(void);
extern BOOL GFX_IsInit(void);

extern INT32 GFX_Reset(uint32_t u4GfxHwId, uint32_t u4Reset);

extern INT32 GFX_ResetWT(uint32_t u4GfxHwId, uint32_t u4Reset);
extern INT32 GFX_ResetWTDst(uint32_t u4GfxHwId, uint32_t u4Reset);
extern INT32 GFX_ResetWTSRC(uint32_t u4GfxHwId, uint32_t u4Reset);
extern INT32 GFX_SetWTDst(uint32_t u4GfxHwId, uint32_t u4Reset);
extern INT32 GFX_SetWTSrc(uint32_t u4GfxHwId, uint32_t u4Reset);
extern INT32 GFX_WtYuvEn(uint32_t u4GfxHwId, uint32_t u4SrcEn,uint32_t u4DstEn);
extern INT32 GFX_WtYuvEnSrc(uint32_t u4GfxHwId, uint32_t u4SrcEn);
extern INT32 GFX_WtYuvEnDst(uint32_t u4GfxHwId, uint32_t u4DstEn);

extern void GFX_Wait(void);

extern INT32 GFX_MwFlush(uint32_t u4GfxHwId);

extern INT32 GFX_Flush(uint32_t u4GfxHwId);

extern void GFX_Lock(void);

extern INT32 GFX_TryLock(void);

extern void GFX_Unlock(void);

extern int GFX_LockCmdque(uint32_t u4GfxHwId);

extern void GFX_UnlockCmdque(uint32_t u4GfxHwId);

extern void GFX_LockResz(void);

extern void GFX_UnlockResz(void);

extern void GFX_LockBMP(void);

extern void GFX_UnlockBMP(void);

extern void GFX_LockBuffer(void);

extern void GFX_UnlockBuffer(void);

extern void GFX_LockHeight(void);

extern void GFX_UnlockHeight(void);

extern INT32 GFX_QueryHwIdle(uint32_t u4GfxHwId);

extern void GFX_SetNotify(uint32_t u4GfxHwId, void (*pfnNotifyFunc)(uint32_t));

extern INT32 GFX_SetDst(uint32_t u4GfxHwId, UINT8 *pu1Base, uint32_t u4ColorMode, uint32_t u4Pitch);

extern INT32 GFX_SetSrc(uint32_t u4GfxHwId, UINT8 *pu1Base, uint32_t u4ColorMode, uint32_t u4Pitch);
extern INT32 GFX_Set2ndSrc(uint32_t u4GfxHwId, UINT8 *pu1Base, uint32_t u4ColorMode, uint32_t u4Pitch);
extern INT32 GFX_Set2ndSrcEnable(uint32_t u4GfxHwId, uint32_t u4ColorMode);

extern INT32 GFX_SetCharSrcBase(uint32_t u4GfxHwId, UINT8 *pu1Base, uint32_t u4ColorMode, uint32_t u4Pitch);

extern INT32 GFX_SetAlpha(uint32_t u4GfxHwId, uint32_t u4Alpha);

extern INT32 GFX_24BPP24BPP_SetAlpha(uint32_t u4GfxHwId, uint32_t u4Alpha, uint32_t u4GAlphaEn);

extern INT32 GFX_SetColor(uint32_t u4GfxHwId, uint32_t u4Color);

#if CONFIG_DRV_3D_256_SUPPORT
extern INT32 GFX_InitMMUTable(uint32_t u4GfxHwId);
#endif
extern INT32 GFX_Fill(uint32_t u4GfxHwId, uint32_t u4X, uint32_t u4Y, uint32_t u4Width, uint32_t u4Height);
extern INT32 GFX_Draw(uint32_t u4GfxHwId, uint32_t u4X, uint32_t u4Y, uint32_t u4Width, uint32_t u4Height);
extern INT32 GFX_DrawCompose(uint32_t u4GfxHwId, uint32_t u4X, uint32_t u4Y, uint32_t u4Width, uint32_t u4Height,
                             uint32_t u4Ar, uint32_t u4OpCode, uint32_t u4RectSrc);
extern INT32 GFX_FillTriangle(uint32_t u4GfxHwId, uint32_t u4X1, uint32_t u4Y1, uint32_t u4X2, uint32_t u4Y2, uint32_t u4X3, uint32_t u4Y3);
extern INT32 GFX_FillTriangleCompose(uint32_t u4GfxHwId, uint32_t u4X1, uint32_t u4Y1, uint32_t u4X2, uint32_t u4Y2, uint32_t u4X3, uint32_t u4Y3,
                              uint32_t u4Ar, uint32_t u4OpCode, uint32_t u4RectSrc);

extern INT32 GFX_FillChar(void *pvBase, UINT8 u1Char, uint32_t u4NumWrd);
extern INT32 GFX_SetWTBase(uint32_t u4GfxHwId, uint32_t u4X, uint32_t u4Y);
extern INT32 GFX_WtBypass(uint32_t u4GfxHwId, uint32_t u4SrcBypass, uint32_t u4DstBypass);
extern INT32 GFX_SetCBCRWBBase(uint32_t u4GfxHwId, UINT8 *pu1SrcChroma);
extern INT32 GFX_HLine(uint32_t u4GfxHwId, uint32_t u4X, uint32_t u4Y, uint32_t u4Width);

extern INT32 GFX_VLine(uint32_t u4GfxHwId, uint32_t u4X, uint32_t u4Y, uint32_t u4Height);

extern INT32 GFX_ObliqueLine(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX, uint32_t u4DstY);
extern INT32 GFX_SetBltOptEx(uint32_t u4GfxHwId, uint32_t u4Switch, uint32_t u4DisSrcKey, uint32_t u4SrcKeyIn, uint32_t u4ColorMin, uint32_t u4ColorMax, uint32_t u4DisDstKey, uint32_t u4DstKeyIn, uint32_t u4DstColorMin, uint32_t u4DstColorMax);
extern INT32 GFX_SetBltOpt(uint32_t u4GfxHwId, uint32_t u4Switch, uint32_t u4ColorMin,
    uint32_t u4ColorMax);

extern INT32 GFX_SetBltOpt2(uint32_t u4GfxHwId, uint32_t u4Enable, uint32_t u4Y2R, uint32_t uEn709);
extern INT32 GFX_24Bpp24BPP_SetBltOpt(uint32_t u4GfxHwId, uint32_t u4Switch, uint32_t u4ColorMin, uint32_t u4ColorMax);

extern INT32 GFX_BitBlt(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX,
    uint32_t u4DstY, uint32_t u4Width, uint32_t u4Height);
extern INT32 GFX_24Bpp24BPP_BitBlt(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX,
    uint32_t u4DstY, uint32_t u4Width, uint32_t u4Height);
extern INT32 GFX_BitBlt2(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX, uint32_t u4DstY,
    uint32_t u4Width, uint32_t u4Height, uint32_t u4op);
extern INT32 GFX_BitBlt_NewMethod(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX, uint32_t u4DstY,
    uint32_t u4Width, uint32_t u4Height);
extern INT32 GFX_Blend(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX,
    uint32_t u4DstY, uint32_t u4Width, uint32_t u4Height);
extern INT32 GFX_MsBlend(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX,
    uint32_t u4DstY, uint32_t u4Width, uint32_t u4Height, uint32_t u4MsSel);  

extern INT32 GFX_SetBuf(uint32_t u4GfxHwId, uint32_t *pu4GfxWorkingBuf, uint32_t u4Size);


extern INT32 GFX_ComposeLoop(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX,
    uint32_t u4DstY, uint32_t u4Width, uint32_t u4Height, uint32_t u4Ar,
    uint32_t u4OpCode, uint32_t u4RectSrc);
extern INT32 GFX_NormalComposeLoop(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX,
    uint32_t u4DstY, uint32_t u4Width, uint32_t u4Height, uint32_t u4Ar,
    uint32_t u4OpCode, uint32_t u4RectSrc);
#if 0 //(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
extern INT32 GFX_FlashLiteNormalComposeBitblt(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX,
                                              uint32_t u4DstY, uint32_t u4Width, uint32_t u4Height,
                                              uint32_t u4PremultSrcRdEn, uint32_t u4PremultDstWrEn);
extern INT32 GFX_FlashLiteStretchBitblt(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                                 uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH, uint32_t u4PremultSrcRdEn,
                                 uint32_t u4PremultDstWrEn, uint32_t u4AlcomAr, uint32_t u4OpCode);
extern INT32 GFX_FlashLiteStretchCompose(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                             uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH, uint32_t u4PremultSrcRdEn,
                             uint32_t u4PremultDstRdEn, uint32_t u4PremultDstWrEn, uint32_t u4AlcomAr, uint32_t u4OpCode);
extern INT32 GFX_FlashLiteFillColor(uint32_t u4GfxHwId, uint32_t u4X, uint32_t u4Y, uint32_t u4Width, uint32_t u4Height, uint32_t u4PremultSrcRdEn);
extern INT32 GFX_FlashLiteComposeBitblt(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX, uint32_t u4DstY,
                                 uint32_t u4Width, uint32_t u4Height, uint32_t u4PremultSrcRdEn, uint32_t u4PremultDstRdEn,
                                 uint32_t u4PremultDstWrEn, uint32_t u4AlcomAr, uint32_t u4OpCode, uint32_t u4RectSrc);

extern INT32 GFX_2SrcBlendingBitblt(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX,
                      uint32_t u4DstY, uint32_t u4Width, uint32_t u4Height, uint32_t u4Ar,
                      uint32_t u4OpCode, uint32_t u4RectSrc, uint32_t u4Src2En);
extern INT32 GFX_BpCompressEncode(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                     uint32_t u4DstX, uint32_t u4DstY, uint32_t u4PixelWidth, uint32_t u4QulityMode,
                     uint32_t u4PixelSeparate, UINT8 *pu1DstIdxAddr);
extern INT32 GFX_BpCompressEncodeCheckEndAddr(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                     uint32_t u4DstX, uint32_t u4DstY, uint32_t u4PixelWidth, uint32_t u4QulityMode,
                     uint32_t u4PixelSeparate, UINT8 *pu1DstIdxAddr, uint32_t u4EndAddr);
extern INT32 GFX_BpDecode(uint32_t u4GfxHwId, uint32_t u4DecodeType,
                          uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                          uint32_t u4DstX, uint32_t u4DstY, uint32_t u4PicW, uint32_t u4PicH,
                          uint32_t u4PixelWidth, uint32_t u4PacketWidth, uint32_t u4QulityMode,
                          UINT8 *pu1DstIdxAddr);
extern INT32 GFX_I2DMFlipComposeLoop(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX, uint32_t u4DstY,
                uint32_t u4Width, uint32_t u4Height, uint32_t u4SrcPitchEn, UINT8 *pv_pal_base, uint32_t u4MsbLeft,
                uint32_t u4StartByteAligned, uint32_t u4AlComNormal, uint32_t u4Ar, uint32_t u4MFOp, uint32_t u4OpCode, uint32_t u4RectSrc);
extern INT32 GFX_I2DStretchMFlipComposeLoop(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH, uint32_t u4SrcPitchEn,
                UINT8 *pv_pal_base, uint32_t u4MsbLeft, uint32_t u4StartByteAligned,
                uint32_t u4AlComNormal, uint32_t u4Ar, uint32_t u4MFOp, uint32_t u4OpCode, uint32_t u4RectSrc);
extern INT32 GFX_I2DRotateMFlipComposeLoop(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX, uint32_t u4DstY, uint32_t u4SrcW, uint32_t u4SrcH, uint32_t u4SrcPitchEn,
                              UINT8 *pv_pal_base, uint32_t u4MsbLeft, uint32_t u4StartByteAligned,
                              uint32_t u4AlComNormal, uint32_t u4Ar, uint32_t u4Is90CCW, uint32_t u4MFOp, uint32_t u4OpCode, uint32_t u4RectSrc);
extern INT32 GFX_I2DStretchRotateMFlipComposeLoop(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
        uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH, uint32_t u4SrcPitchEn,
        UINT8 *pv_pal_base, uint32_t u4MsbLeft, uint32_t u4StartByteAligned,
        uint32_t u4AlComNormal, uint32_t u4Ar, uint32_t u4Is90CCW, uint32_t u4MFOp, uint32_t u4OpCode, uint32_t u4RectSrc);

#endif
extern INT32 GFX_ComposeLoopEx(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX,
    uint32_t u4DstY, uint32_t u4Width, uint32_t u4Height, uint32_t u4AlComNormal, uint32_t u4Ar,
    uint32_t u4OpCode, uint32_t u4RectSrc);
extern INT32 GFX_StretchRotate(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW,
    uint32_t u4SrcH, uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH, uint32_t u4Is90CCW);
extern INT32 GFX_StretchMFlip(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW,
    uint32_t u4SrcH, uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH, uint32_t u4MFOp);
extern INT32 GFX_StretchRotateMFlip(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW,
    uint32_t u4SrcH, uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH, uint32_t u4Is90CCW,
    uint32_t u4MFOp);
extern INT32 GFX_AlphaComposeLoopRotate(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY,
                                 uint32_t u4DstX, uint32_t u4DstY, uint32_t u4Width,
                                 uint32_t u4Height, uint32_t u4Ar, uint32_t u4OpCode,
                                 uint32_t u4RectSrc, uint32_t u4AlcomNormal,
                                 uint32_t u4Is90CCW);
extern INT32 GFX_AlphaComposeLoopMFlip(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY,
                                uint32_t u4DstX, uint32_t u4DstY, uint32_t u4Width,
                                uint32_t u4Height, uint32_t u4Ar, uint32_t u4OpCode,
                                uint32_t u4RectSrc, uint32_t u4AlcomNormal,
                                uint32_t u4MFOp);
extern INT32 GFX_AlphaComposeLoopRotateMFlip(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY,
                                      uint32_t u4DstX, uint32_t u4DstY, uint32_t u4Width,
                                      uint32_t u4Height, uint32_t u4Ar, uint32_t u4OpCode,
                                      uint32_t u4RectSrc, uint32_t u4AlcomNormal,
                                      uint32_t u4Is90CCW, uint32_t u4MFOp);
extern INT32 GFX_ComposeLoop2ALU(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX,
    uint32_t u4DstY, uint32_t u4Width, uint32_t u4Height, uint32_t u4Ar,
    uint32_t u4OpCode, uint32_t u4RectSrc);
extern INT32 GFX_NormalComposeLoop2ALU(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX,
    uint32_t u4DstY, uint32_t u4Width, uint32_t u4Height, uint32_t u4Ar,
    uint32_t u4OpCode, uint32_t u4RectSrc);
extern INT32 GFX_Compose(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX,
    uint32_t u4DstY, uint32_t u4Width, uint32_t u4Height, uint32_t u4Ar, uint32_t u4Mode);

extern INT32 GFX_AlphaComposePass(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY,
    uint32_t u4DstX, uint32_t u4DstY, uint32_t u4Width, uint32_t u4Height,
    uint32_t u4Pass, uint32_t u4Param);
extern INT32 GFX_NormalComposePass(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY,
    uint32_t u4DstX, uint32_t u4DstY, uint32_t u4Width, uint32_t u4Height,
    uint32_t u4Pass, uint32_t u4Param);
extern INT32 GFX_SetColCnvFmt(uint32_t u4GfxHwId, uint32_t u4YCFmt, uint32_t u4SwapMode,
    uint32_t u4VidStd, uint32_t u4VidSys);

extern INT32 GFX_SetColCnvSrc(uint32_t u4GfxHwId, UINT8 *pu1SrcLuma, uint32_t u4SrcLumaPitch,
    UINT8 *pu1SrcChroma, uint32_t u4SrcChromaPitch, uint32_t u4FieldPic);

extern INT32 GFX_ColConv(uint32_t u4GfxHwId, uint32_t u4X, uint32_t u4Y, uint32_t u4Width,
    uint32_t u4Height);

extern INT32 GFX_StretchBlt(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW,
    uint32_t u4SrcH, uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH);

extern INT32 GFX_AlphaMapBitBlt(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY,
    uint32_t u4DstX, uint32_t u4DstY, uint32_t u4Width, uint32_t u4Height);

extern INT32 GFX_DMA(uint32_t *pu4Dst, uint32_t *pu4Src, uint32_t u4NumWrd);

extern INT32 GFX_SetBmp(uint32_t u4GfxHwId, UINT8 *puu1Base, uint32_t u4BmpColorMode, uint32_t u4Width,
    uint32_t u4Height, uint32_t u4RunLen);

extern INT32 GFX_ColorMap(uint32_t u4GfxHwId, uint32_t u4BmpCM, uint32_t u4CM,
    const uint32_t *psu4ColorMap);

extern INT32 GFX_Bmp(uint32_t u4GfxHwId, uint32_t u4X, uint32_t u4Y);

//extern INT32 GFX_SetPgigSrcQDecode(void *pvSrcQBase, uint32_t u4QLen, uint32_t u4QUint);
extern INT32 GFX_SetPgigDecodeSrcQ(UINT8 *pu1SrcQBase, uint32_t u4DataLen, uint32_t u4QLen, uint32_t u4QUnit);
extern INT32 GFX_SetPgigDstQDecode(void *pvSrcQBase, uint32_t u4QLen, uint32_t u4QUint);

//extern INT32 GFX_PgigDecode(uint32_t u4CharCm, VOID * pvSrc, uint32_t u4DstX, uint32_t u4DstY,
//    uint32_t u4Width, uint32_t u4Height);
extern INT32 GFX_SetPgigDecodePalette(void *pvPalBase, uint32_t u4PalSize);
extern INT32 GFX_SetPgigDecodePalette2(void *pvPalBase, uint32_t u4PalSize);
extern INT32 GFX_PgigDecode(uint32_t u4DstX, uint32_t u4DstY, uint32_t u4Width, uint32_t u4Height);

//extern INT32 GFX_PgigSrcQDecode(void);

extern INT32 GFX_RopBitblt(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX, uint32_t u4DstY,
    uint32_t u4Width, uint32_t u4Height, uint32_t u4RopCode);

extern INT32 GFX_Idx2DirBitblt(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX, uint32_t u4DstY,
    uint32_t u4Width, uint32_t u4Height, UINT8 *pv_pal_base,
    uint32_t u4MsbLeft, uint32_t u4StartByteAligned);
extern INT32 GFX_Idx2DirBitbltEx(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX, uint32_t u4DstY,
    uint32_t u4Width, uint32_t u4Height, uint32_t u4SrcPitchEn, UINT8 *pv_pal_base,
    uint32_t u4MsbLeft, uint32_t u4StartByteAligned);
extern INT32 GFX_SetHoriToVertLineOpt(uint32_t u4GfxHwId, uint32_t u4IsCounterClockWise);

extern INT32 GFX_HoriToVertLine(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX,
    uint32_t u4DstY, uint32_t u4HoriLineWidth, INT32 u4Is90CCW);

extern INT32 GFX_RotateBmp(uint32_t u4GfxHwId, UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
        uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX, uint32_t u4DstY,
    uint32_t u4CM, uint32_t u4SrcPitch, uint32_t u4DstPitch,
    uint32_t u4Width, uint32_t u4Height, uint32_t u4Is90CCW);
extern INT32 GFX_RotateMirrorFlipBmp(uint32_t u4GfxHwId, UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX, uint32_t u4DstY,
    uint32_t u4CM, uint32_t u4SrcPitch, uint32_t u4DstPitch,
    uint32_t u4Width, uint32_t u4Height, uint32_t u4Is90CCW,UINT8 u1AddOpt);

extern INT32 GFX_RotateBmpSw(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
        uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX, uint32_t u4DstY,
    uint32_t u4CM, uint32_t u4SrcPitch, uint32_t u4DstPitch,
    uint32_t u4Width, uint32_t u4Height, uint32_t u4Is90CCW);

extern INT32 GFX_2DMemCompare(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX,
    uint32_t u4DstY, uint32_t u4Width, uint32_t u4Height);

extern INT32 GFX_GetMemCompareResult(uint32_t u4GfxHwId);

extern INT32 GFX_MemCompare(uint32_t u4GfxHwId, UINT8 *pu1SrcBase, UINT8 *pu1DstBase, uint32_t u4Size);
extern INT32 GFX_SetGradOpt(uint32_t u4GfxHwId, uint32_t u4IncX, uint32_t u4IncY,
  const uint32_t asu4DeltaX[4], const uint32_t asu4DeltaY[4]);
extern INT32 GFX_GradFill(uint32_t u4GfxHwId, uint32_t u4X, uint32_t u4Y, uint32_t u4Width, uint32_t u4Height,
    uint32_t u4Mode);
//#if (CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8550)
#if 1
INT32 GFX_SetClipOpt(uint32_t u4GfxHwId, uint32_t u4ClipEnMask, uint32_t u4DstX, uint32_t u4DstY, uint32_t u4ClipTop, uint32_t u4ClipBot, uint32_t u4ClipLeft, uint32_t u4ClipRight);

INT32 GFX_BurstEn(uint32_t u4GfxHwId, uint32_t u4BurstEn, uint32_t u4BurstMode);
INT32 GFX_StretchComposeLoop(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                             uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH,
                             uint32_t u4AlComNormal, uint32_t u4Ar, uint32_t u4OpCode, uint32_t u4RectSrc);
INT32 GFX_I2DComposeLoop(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX, uint32_t u4DstY, uint32_t u4Width, uint32_t u4Height, uint32_t u4SrcPitchEn,
                         UINT8 *pv_pal_base, uint32_t u4MsbLeft, uint32_t u4StartByteAligned,
                         uint32_t u4AlComNormal, uint32_t u4Ar, uint32_t u4OpCode, uint32_t u4RectSrc);
INT32 GFX_StretchRotateComposeLoop(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                                   uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH,
                                   uint32_t u4Is90CCW, uint32_t u4AlComNormal, uint32_t u4Ar, uint32_t u4OpCode, uint32_t u4RectSrc);
INT32 GFX_RopJavaXorMFilp(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                          uint32_t u4DstX, uint32_t u4DstY, uint32_t u4MFOp, uint32_t u4JavaXorClr);
INT32 GFX_StretchJavaXor(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                         uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH, uint32_t u4JavaXorClr);
INT32 GFX_StretchRotateJavaXor(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                               uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH, uint32_t u4Is90CCW, uint32_t u4JavaXorClr);
INT32 GFX_BpCompressEx(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                     uint32_t u4DstX, uint32_t u4DstY, uint32_t u4RollBackEn, uint32_t u4QulityMode, uint32_t u4LineSeparate, uint32_t u4Size);
INT32 GFX_BpCompress(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                     uint32_t u4DstX, uint32_t u4DstY, uint32_t u4RollBackEn, uint32_t u4QulityMode, uint32_t u4LineSeparate);
INT32 GFX_BpCompressBase(uint32_t u4GfxHwId, uint32_t u4SrcW, uint32_t u4RollBackEn, uint32_t u4QulityMode, uint32_t u4LineSeparate);
INT32 GFX_BpCompressSpecial(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY,
uint32_t u4DstX, uint32_t u4DstY, uint32_t u4SrcW, uint32_t u4SrcH);

INT32 GFX_PremultipliedConvert(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX, uint32_t u4DstY, 
                               uint32_t u4Width, uint32_t u4Height, GFX_PREMULTIPLIED_CNV_TYPE eCvtType);

INT32 GFX_RopBitbltEx(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX, uint32_t u4DstY,
                      uint32_t u4Width, uint32_t u4Height, uint32_t u4RopCode, uint32_t u4JavaXorClr);
INT32 GFX_StretchMFlipComposeLoop(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                                       uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH,
                                       uint32_t u4MFOp, uint32_t u4AlComNormal, uint32_t u4Ar, uint32_t u4OpCode, uint32_t u4RectSrc);
INT32 GFX_StretchMFlipJavaXor(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                              uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH,
                              uint32_t u4MFOp, uint32_t u4JavaXorClr);
INT32 GFX_StretchRotateMFlipJavaXor(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                                    uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH,
                                    uint32_t u4Is90CCW, uint32_t u4MFOp, uint32_t u4JavaXorClr);
INT32 GFX_StretchRotateMFlipComposeLoop(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                                        uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH,
                                        uint32_t u4Is90CCW, uint32_t u4MFOp, uint32_t u4AlComNormal, uint32_t u4Ar, uint32_t u4OpCode, uint32_t u4RectSrc);
#endif
INT32 GFX_YCbCr2RGBAlphaCompose(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX, uint32_t u4DstY, uint32_t u4Width, uint32_t u4Height,
            UINT8 *p1LumaSA, uint32_t u4SrcLumaPitch, UINT8 *p1ChromaSA, uint32_t u4SrcChromaPitch,
            uint32_t u4YCFmt, uint32_t u4SwapMode, uint32_t u4VidStd, uint32_t u4VidSys, uint32_t u4VsClip, uint32_t u4FldPic,
            uint32_t u4AlComNormal, uint32_t u4Ar, uint32_t u4OpCode, uint32_t u4RectSrc);
INT32 GFX_YCbCr2RGBStretchAlphaCompose(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4Width, uint32_t u4Height,
            uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstWidth, uint32_t u4DstHeight,
            UINT8 *p1LumaSA, uint32_t u4SrcLumaPitch, UINT8 *p1ChromaSA, uint32_t u4SrcChromaPitch,
            uint32_t u4YCFmt, uint32_t u4SwapMode, uint32_t u4VidStd, uint32_t u4VidSys, uint32_t u4VsClip, uint32_t u4FldPic,
            uint32_t u4AlComNormal, uint32_t u4Ar, uint32_t u4OpCode, uint32_t u4RectSrc);
INT32 GFX_YCbCr2RGBStretchMFlipAlphaCompose(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4Width, uint32_t u4Height,
            uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstWidth, uint32_t u4DstHeight,
            UINT8 *p1LumaSA, uint32_t u4SrcLumaPitch, UINT8 *p1ChromaSA, uint32_t u4SrcChromaPitch,
            uint32_t u4YCFmt, uint32_t u4SwapMode, uint32_t u4VidStd, uint32_t u4VidSys, uint32_t u4VsClip, uint32_t u4FldPic,
            uint32_t u4AlComNormal, uint32_t u4Ar, uint32_t u4MFOpt, uint32_t u4OpCode, uint32_t u4RectSrc);
INT32 GFX_YCbCr2RGBStretchJavaXor(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4Width, uint32_t u4Height,
            uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstWidth, uint32_t u4DstHeight,
            UINT8 *p1LumaSA, uint32_t u4SrcLumaPitch, UINT8 *p1ChromaSA, uint32_t u4SrcChromaPitch,
            uint32_t u4YCFmt, uint32_t u4SwapMode, uint32_t u4VidStd, uint32_t u4VidSys, uint32_t u4VsClip, uint32_t u4FldPic,
            uint32_t u4JavaXorClr);
INT32 GFX_LosslessDecodeComposeLoop(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY,
    uint32_t u4DstX, uint32_t u4DstY, uint32_t u4Width, uint32_t u4Height, uint32_t u4Ar, uint32_t u4OpCode,
    uint32_t u4RectSrc, uint32_t u4PixelWidth, uint32_t u4PacketWidth, uint32_t u4QulityMode, UINT8 *p1IdxSA);
INT32 GFX_LosslessDecodeStretchBitblt(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                                              uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH, uint32_t u4PixelWidth,
                                              uint32_t u4PacketWidth, uint32_t u4QulityMode, UINT8 *p1IdxSA);
INT32 GFX_LosslessDecodeStretchCompose(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                                       uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH, uint32_t u4AlcomNormal, uint32_t u4Ar, uint32_t u4OpCode,
                                       uint32_t u4RectSrc, uint32_t u4PicWidth, uint32_t u4PicHeight, uint32_t u4PixelWidth,
                                       uint32_t u4PacketWidth, uint32_t u4QulityMode, UINT8 *p1IdxSA);
INT32 GFX_LosslessDecodeStretchFlipCompose(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                                           uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH, uint32_t u4AlcomNormal, uint32_t u4Ar, uint32_t u4OpCode,
                                           uint32_t u4RectSrc, uint32_t u4PicWidth, uint32_t u4PicHeight, uint32_t u4PixelWidth,
                                           uint32_t u4PacketWidth, uint32_t u4QulityMode, UINT8 *p1IdxSA);
INT32 GFX_LosslessDecodeStretchRotateCompose(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                                             uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH, uint32_t u4AlcomNormal, uint32_t u4Ar, uint32_t u4OpCode,
                                             uint32_t u4RectSrc, uint32_t u4Is90CCW, uint32_t u4PicWidth, uint32_t u4PicHeight, uint32_t u4PixelWidth,
                                             uint32_t u4PacketWidth, uint32_t u4QulityMode, UINT8 *p1IdxSA);
INT32 GFX_LosslessDecodeStretchRotateFlipCompose(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                                                 uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH, uint32_t u4AlcomNormal, uint32_t u4Ar, uint32_t u4OpCode,
                                                 uint32_t u4RectSrc, uint32_t u4Is90CCW, uint32_t u4PicWidth, uint32_t u4PicHeight, uint32_t u4PixelWidth,
                                                 uint32_t u4PacketWidth, uint32_t u4QulityMode, UINT8 *p1IdxSA);
INT32 GFX_LosslessDecodeStretchJavaXor(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                                       uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH, uint32_t u4JavaXorColor,
                                       uint32_t u4PicWidth, uint32_t u4PicHeight, uint32_t u4PixelWidth,
                                       uint32_t u4PacketWidth, uint32_t u4QulityMode, UINT8 *p1IdxSA);
INT32 GFX_LosslessDecodeStretchFlipJavaXor(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                                           uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH, uint32_t u4JavaXorColor,
                                           uint32_t u4PicWidth, uint32_t u4PicHeight, uint32_t u4PixelWidth,
                                           uint32_t u4PacketWidth, uint32_t u4QulityMode, UINT8 *p1IdxSA);
INT32 GFX_LosslessDecodeStretchRotateJavaXor(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                                             uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH,
                                             uint32_t u4JavaXorColor, uint32_t u4Is90CCW,
                                             uint32_t u4PicWidth, uint32_t u4PicHeight, uint32_t u4PixelWidth,
                                             uint32_t u4PacketWidth, uint32_t u4QulityMode, UINT8 *p1IdxSA);
INT32 GFX_LosslessDecodeStretchRotateFlipJavaXor(uint32_t u4GfxHwId, uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4SrcW, uint32_t u4SrcH,
                                                 uint32_t u4DstX, uint32_t u4DstY, uint32_t u4DstW, uint32_t u4DstH,
                                                 uint32_t u4JavaXorColor, uint32_t u4Is90CCW,
                                                 uint32_t u4PicWidth, uint32_t u4PicHeight, uint32_t u4PixelWidth,
                                                 uint32_t u4PacketWidth, uint32_t u4QulityMode, UINT8 *p1IdxSA);
INT32 GFX_SetRacingMode(uint32_t u4GfxHwId, uint32_t u4Mode);
INT32 GFX_MMUEnable(uint32_t u4GfxHwId, uint32_t u4Enable);
INT32 GFX_MMUCfgA0(uint32_t u4GfxHwId, uint32_t u4En, uint32_t u4Mid, uint32_t u4Prefetch, uint32_t u42DPrefetch,
                   uint32_t u4PrefetchDec, uint32_t u4TwoWay, uint32_t u4Mid2nd);
INT32 GFX_MMUCfgA1(uint32_t u4GfxHwId, uint32_t u4En, uint32_t u4Mid, uint32_t u4Prefetch, uint32_t u42DPrefetch,
                   uint32_t u4PrefetchDec, uint32_t u4TwoWay, uint32_t u4Mid2nd);
INT32 GFX_MMUCfgA2(uint32_t u4GfxHwId, uint32_t u4En, uint32_t u4Mid, uint32_t u4Prefetch, uint32_t u42DPrefetch,
                   uint32_t u4PrefetchDec, uint32_t u4TwoWay, uint32_t u4Mid2nd);
INT32 GFX_MMUCfgA3(uint32_t u4GfxHwId, uint32_t u4En, uint32_t u4Mid, uint32_t u4Prefetch, uint32_t u42DPrefetch,
                   uint32_t u4PrefetchDec, uint32_t u4TwoWay, uint32_t u4Mid2nd);
INT32 GFX_MMUCfg(uint32_t u4GfxHwId, uint32_t u4Enable,	uint32_t u4SrcRead, uint32_t u4DstRead, uint32_t u4DstWrite);
INT32 GFX_MMUCfgFill(uint32_t u4GfxHwId, uint32_t u4Enable,	uint32_t u4SrcRead, uint32_t u4DstRead, uint32_t u4DstWrite);
INT32 GFX_MMUCfgEx(uint32_t u4GfxHwId, uint32_t u4Enable,	uint32_t u4SrcRead, uint32_t u4DstRead, uint32_t u4DstWrite);
INT32 GFX_MMUCfgForCompressFunc(uint32_t u4GfxHwId, uint32_t u4Enable, uint32_t u4SrcRead, uint32_t u4DstRead, uint32_t u4DstWrite);
INT32 GFX_MMUOverReadProtection(uint32_t u4GfxHwId, uint32_t u4Enable, GFX_MMU_TYPE eType, uint32_t u4Addr1, uint32_t u4Size1, uint32_t u4Addr2, uint32_t u4Size2);
INT32 GFX_MMUOverReadProtectionEx(uint32_t u4GfxHwId, uint32_t u4Enalbe, GFX_MMU_TYPE eType, uint32_t u4Addr1, uint32_t u4Size1, uint32_t u4Addr2, uint32_t u4Size2);
INT32 GFX_MMUCfgForY2RFunc(uint32_t u4GfxHwId, uint32_t u4Enable, uint32_t u4SrcYRead, uint32_t u4SrcCRead, uint32_t u4DstWrite);
//extern INT32 u4NewGfxTask(void);

// for debug use
//#if defined(GFX_DEBUG_MODE)
#if 1
extern INT32 GFX_Reset_Engine(uint32_t u4GfxHwId);

extern INT32 GFX_Reset_CmdQue(uint32_t u4GfxHwId);

extern void GFX_DumpDebugInfo(void);

extern void GFX_QueryCmdQueInfo(uint32_t u4GfxHwId);

extern INT32 GFX_SetLegalAddress(uint32_t u4GfxHwId, uint32_t u4Start, uint32_t u4End);
#endif // #if defined(GFX_DEBUG_MODE)


extern INT32 GFX_Memset(uint32_t u4GfxHwId, UINT8 *pu1Addr, uint32_t u4Size, uint32_t u4Color);

extern uint32_t GFX_QueryFlushCount(uint32_t u4GfxHwId);

extern uint32_t GFX_QueryHwInterruptCount(uint32_t u4GfxHwId);

extern void GFX_SetCqCapacity(uint32_t u4GfxHwId, INT32 i4Capacity);
#ifdef GL_XOR

#if CONFIG_SYS_MEM_PHASE3
HANDLE_T GFX_GetWorkingBufferMemHandle(uint32_t u4GfxHwId);
HANDLE_T GFX_GetTopWorkingBufferMemHandle(uint32_t u4GfxHwId);
HANDLE_T GFX_GetBottomWorkingBufferMemHandle(uint32_t u4GfxHwId);
#endif

void GFX_GetWorkingBufferHeight(uint32_t u4GfxHwId, void** ppvBuffer, uint32_t ui4_pitch,uint32_t *pui4_height);
void GFX_GetWorkingBufferWidth(uint32_t u4GfxHwId, uint32_t ui4_height,uint32_t *pui4_width);
void GFX_GetTopWorkingBufferHeight(uint32_t u4GfxHwId, void** ppvBuffer, uint32_t ui4_pitch,uint32_t *pui4_height);
void GFX_GetTopWorkingBufferWidth(uint32_t u4GfxHwId, uint32_t ui4_height,uint32_t *pui4_width);
void GFX_GetBottomWorkingBufferHeight(uint32_t u4GfxHwId,void * * ppvBuffer,uint32_t ui4_pitch,uint32_t * pui4_height);
void GFX_GetBottomWorkingBufferWidth(uint32_t u4GfxHwId, uint32_t ui4_height,uint32_t *pui4_width);
//void GFX_GetWorkingBuffer2Height(uint32_t u4GfxHwId, void** ppvBuffer, uint32_t ui4_pitch,uint32_t *pui4_height);
//void GFX_GetWorkingBuffer2Width(uint32_t u4GfxHwId, uint32_t ui4_height,uint32_t *pui4_width);
INT32 GFX_MirrorFlip(uint32_t u4GfxHwId,
    uint32_t u4SrcX,
    uint32_t u4SrcY,
    uint32_t u4DstX,
    uint32_t u4DstY,
    uint32_t u4Width,
    uint32_t u4Height,
    EGFX_BITBLT_OPT_T u4Opt,
    UINT8 u1AddOpt);
INT32 GFX_RotateMirrorFlip(uint32_t u4GfxHwId, UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    uint32_t u4SrcX, uint32_t u4SrcY, uint32_t u4DstX, uint32_t u4DstY,
    uint32_t u4CM, uint32_t u4SrcPitch, uint32_t u4DstPitch,
    uint32_t u4Width, uint32_t u4Height, uint32_t u4Is90CCW,EGFX_BITBLT_OPT_T u4Opt);


#endif
INT32 GFX_MMUCfgEx(uint32_t u4GfxHwId, uint32_t u4Enable, uint32_t u4SrcRead, uint32_t u4DstRead, uint32_t u4DstWrite);
INT32 vGfxSetMMU(BOOL fgMMUEnble);
VOID vIIOMMU_PrintAllPageTables(uint32_t ui4_vmem, uint32_t ui4_mem_sz);

INT32 i4GfxSetMmuWaitMode(uint32_t u4Mode);
INT32 i4GfxSetMmuGlbBypss(uint32_t u4Mode);
INT32 GFX_SetMmuWaitMode(uint32_t u4GfxHwId, uint32_t u4Mode);
INT32 i4GfxSetMmuSelfFire(uint32_t u4GfxHwId);
INT32 GFX_SetPreColorize(uint32_t u4GfxHwId, uint32_t u4PreColorize, uint32_t u4ColorRepEn, uint32_t u4ColorRep);
//#endif // GFX_IF_H ---???not the file end

extern INT32 i4GfxDrvInstInit(void);

INT32 GFX_SetLosslessMultiRegionEnable(uint32_t u4GfxHwId, uint32_t u4Enable);
INT32 GFX_SetLosslessRegionDisableAll(uint32_t u4GfxHwId);
INT32 GFX_SetLosslessDecSettings(uint32_t u4GfxHwId, uint32_t u4Y, uint32_t u4RegionCount, uint32_t *pu4Height, uint32_t *pu4Addr);
INT32 GFX_GetStartRegion(uint32_t *pu4Height, uint32_t u4Y, uint32_t u4RegionCnt);
INT32 GFX_SetLosslessDecSettings_new(uint32_t u4GfxHwId, uint32_t u4Y, uint32_t u4RegionCount, uint32_t *pu4Height, uint32_t *pu4Addr);
#endif // GFX_IF_H


