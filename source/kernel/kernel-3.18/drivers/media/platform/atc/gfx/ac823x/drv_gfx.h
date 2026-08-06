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
#if !defined(DRV_GFX_H)
#define DRV_GFX_H

#include "x_bim.h"
#include "x_gfx.h"
#include "drv_graphics.h"
#include <linux/slab.h>
//#define D_FPGA_GFX_TEST

#ifndef CONFIG_LOSSLESS_COMPRESS_AUTO_FLIP_MODE
#define CONFIG_LOSSLESS_COMPRESS_AUTO_FLIP_MODE 0
#endif

#ifndef GFX_IOMMU_WORKAROUND_BEFORE_ECO
#define GFX_IOMMU_WORKAROUND_BEFORE_ECO 0
#endif

#ifndef CONFIG_SYS_MEM_PHASE3
#define CONFIG_SYS_MEM_PHASE3 1
#endif
#ifndef CONFIG_SYS_MEM_PHASE2
#define CONFIG_SYS_MEM_PHASE2 0
#endif
#ifdef CONFIG_FATS_SUPPORT
#undef CONFIG_FATS_SUPPORT
#define CONFIG_FATS_SUPPORT 0
#else
#define CONFIG_FATS_SUPPORT 0
#endif

#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
//#include <asm/system.h>
#define GFX_MB mb

//#define GFXMMU			//MTK68024:enable gfx mmu when defined

//#define GFX_FLUSH_DCACHE() HalFlushInvalidateDCache();

typedef struct gfx_device {
		struct miscdevice cdev;   /* Char device structure */
		uint32_t dwGfxInst;
		#ifdef CONFIG_PM
		struct device * dev;
		#endif
}gfx_device;

//---------------------------------------------------------------------------
// Constant definitions
//---------------------------------------------------------------------------

#define D_GFXFLAG_TRANSPARENT   (1 << (INT32)E_GFXBLT_TRANSPARENT)
#define D_GFXFLAG_KEYNOT        (1 << (INT32)E_GFXBLT_KEYNOT)
#define D_GFXFLAG_COLORCHANGE   (1 << (INT32)E_GFXBLT_COLORCHANGE)
#define D_GFXFLAG_CLIP          (1 << (INT32)E_GFXBLT_CLIP)
#define D_GFXFLAG_CFMT_ENA      (1 << (INT32)E_GFXBLT_CFMT_ENA)
#define D_GFXFLAG_KEYSDSEL      (1 << (INT32)E_GFXBLT_KEYSDSEL)
#define D_GFX_24Bpp24BPP_PRCN_1555     (1 << (INT32)E_GFX_24Bpp24BPP_PRCN_1555)
#define D_GFX_24Bpp24BPP_PRCN_KEY_1555 (1 << (INT32)E_GFX_24Bpp24BPP_PRCN_KEY_1555)
#define D_GFX_24Bpp24BPP_ROUND_1555    (1 << (INT32)E_GFX_24Bpp24BPP_ROUND_1555)
#define D_GFX_24Bpp24BPP_ROUND_4444    (1 << (INT32)E_GFX_24Bpp24BPP_ROUND_4444)
#define D_GFX_24Bpp24BPP_ROUND_565    (1 << (INT32)E_GFX_24Bpp24BPP_ROUND_565)
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


//---------------------------------------------------------------------------
// Type definitions
//---------------------------------------------------------------------------

// GFX error mode
enum E_MI_GFX_ERR_CODE_T
{
    E_GFX_OK = 0,
    E_GFX_INV_ARG,
    E_GFX_OUT_OF_MEM,
    E_GFX_UNINIT,
    E_GFX_UNDEF_ERR,
    E_GFX_WOULD_BLOCK
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
	CM_RGB888_DIRECT24,
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
    E_AC_DST,
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
    E_GFXBLT_KEYSDSEL,
    E_GFX_24Bpp24BPP_PRCN_1555,
    E_GFX_24Bpp24BPP_PRCN_KEY_1555,
    E_GFX_24Bpp24BPP_ROUND_1555,
    E_GFX_24Bpp24BPP_ROUND_4444,
    E_GFX_24Bpp24BPP_ROUND_565
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

//#if (CONFIG_DRV_LINUX)
//ATC68024 --dont find symbol:x_alloc_aligned_dma_mem,x_free_aligned_dma_mem
//extern void* x_alloc_aligned_dma_mem(uint32_t u4Size, uint32_t u4Align);
//extern void x_free_aligned_dma_mem(void *pUser);

#define GFX_ALLOC_MEM(u4Size, u4Align)      kmalloc(u4Size, GFP_KERNEL)	//x_alloc_aligned_dma_mem(u4Size, u4Align)
#define GFX_FREE_MEM(p)                     if(p) {kfree((VOID*)(p)); p = NULL;} //if(p) {x_free_aligned_dma_mem((VOID*)(p)); p = NULL;}
#define GFX_ALLOC_ALGN_MEM(u4Size, u4Align)  kmalloc(u4Size, GFP_KERNEL) //x_alloc_aligned_mem(u4Size, u4Align)
#define GFX_FREE_ALGN_MEM(p)                 if(p) {kfree((VOID*)(p)); p = NULL;} //if(p) {x_free_aligned_mem((VOID*)(p)); p = NULL;}

//#define GFX_SEMA_LOCK_TIMEOUT(hSema, u4Time, i4Ret)     {i4Ret = x_sema_lock_timeout(hSema, u4Time);}
//#define GFX_SEMA_LOCK(hSema, eOpt, i4Ret)               {i4Ret = x_sema_lock(hSema, eOpt); VERIFY(i4Ret == OSR_OK);}
//#define GFX_SEMA_UNLOCK(hSema, i4Ret)                   {i4Ret = x_sema_unlock(hSema); VERIFY(i4Ret == OSR_OK);}


//extern void *GGT_get_vmem(uint32_t u4Size);
//extern void *IOMMU_get_fragment_vmem(uint32_t u4Size);

//extern void *x_mem_alloc(size_t u4Size);
//extern void x_mem_free(void *pUser);

//---------------------------------------------------------------------------
// Inter-file functions
//---------------------------------------------------------------------------

extern INT32 _GFX_IsFlushing(void);

extern void _GFX_SetFlushStatus(INT32 i4Status);


//---------------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------------
#if (CONFIG_DRV_LINUX)
extern void *addr_user_to_kernel(void *addr);
extern void *addr_kernel_to_user(void *addr);
#endif

#endif

