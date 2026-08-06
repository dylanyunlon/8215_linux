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
//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------
#include "drv_config.h"
#include <linux/string.h>
/*lint -save -e961 -e971 -e10 -e18 -e19 */
//#include "gfx_common.h"
#include "gfx_if.h"
#include "gfx_sw.h"
#include "gfx_dif.h"
//#include "x_dbg.h"
#include "gfx_hw.h"
/*lint -restore */
#include "x_printf.h"
#include "x_debug.h"
#include "x_rtos.h"

#include "sys_config.h"

#if CONFIG_SYS_MEM_PHASE2
#include "x_mem_phase2.h"
#elif CONFIG_SYS_MEM_PHASE3
#include "x_kmem.h"
#endif

//#ifdef CONFIG_CHIP_VER_CURR
//#undef CONFIG_CHIP_VER_CURR
//#define CONFIG_CHIP_VER_CURR CONFIG_CHIP_VER_MT8563
//#endif

#define x_dbg_stmt(argu)

#if defined(GFX_ENABLE_SW_MODE)
//---------------------------------------------------------------------------
// Constant definitions
//---------------------------------------------------------------------------
#define DSTPOTCHDEC 0

//---------------------------------------------------------------------------
// Type definitions
//---------------------------------------------------------------------------
//extern void   HalFlushDCache(void);
//extern void   HalInvalidateDCache(void);
//extern void     BIM_WAIT_WALE(void);
extern void HalFlushInvalidateDCache(void);
//---------------------------------------------------------------------------
// Macro definitions
//---------------------------------------------------------------------------
#define VERIFY_ALCOM   1

//---------------------------------------------------------------------------
// Imported variables
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Imported functions
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Static function forward declarations
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Static variables
//---------------------------------------------------------------------------

static void *_pvGfxSwCallBackTag;

static UINT32 _au4GfxSwReg[GFX_HAL_HW_INST_NUM][GREG_FILE_SIZE];

static UINT32 _u4GfxSwActionCount;

static const UINT8 _au1sPixelLgDwn[] =
{
    // (pixel_perl_dw)
    4,    //CM_YCbCr_CLUT2
    3,    //CM_YCbCr_CLUT4
    2,    //CM_YCbCr_CLUT8
    0,    //CM_Reserved0
    1,    //CM_CbYCrY422_DIRECT16
    1,    //CM_YCbYCr422_DIRECT16
    0,    //CM_AYCbCr8888_DIRECT32
    0,    //CM_Reserved1
    4,    //CM_RGB_CLUT2
    3,    //CM_RGB_CLUT4
    2,    //CM_RGB_CLUT8
    1,    //CM_RGB565_DIRECT16
    1,    //CM_ARGB1555_DIRECT16
    1,    //CM_ARGB4444_DIRECT16
    0,    //CM_ARGB8888_DIRECT32
    0    //CM_Reserved2
};

/** division reference table
 *  for alpha composition use
 *
 */
static const INT32 _i4sAlphaInvTbl[] =
{
       0,    2048,    1024,     683,     512,     410,     341,     293,
     256,     228,     205,     186,     171,     158,     146,     137,
     128,     120,     114,     108,     102,      98,      93,      89,
      85,      82,      79,      76,      73,      71,      68,      66,
      64,      62,      60,      59,      57,      55,      54,      53,
      51,      50,      49,      48,      47,      46,      45,      44,
      43,      42,      41,      40,      39,      39,      38,      37,
      37,      36,      35,      35,      34,      34,      33,      33,
      32,      32,      31,      31,      30,      30,      29,      29,
      28,      28,      28,      27,      27,      27,      26,      26,
      26,      25,      25,      25,      24,      24,      24,      24,
      23,      23,      23,      23,      22,      22,      22,      22,
      21,      21,      21,      21,      20,      20,      20,      20,
      20,      20,      19,      19,      19,      19,      19,      18,
      18,      18,      18,      18,      18,      18,      17,      17,
      17,      17,      17,      17,      17,      16,      16,      16,
      16,      16,      16,      16,      16,      15,      15,      15,
      15,      15,      15,      15,      15,      15,      14,      14,
      14,      14,      14,      14,      14,      14,      14,      14,
      13,      13,      13,      13,      13,      13,      13,      13,
      13,      13,      13,      13,      12,      12,      12,      12,
      12,      12,      12,      12,      12,      12,      12,      12,
      12,      12,      12,      11,      11,      11,      11,      11,
      11,      11,      11,      11,      11,      11,      11,      11,
      11,      11,      11,      11,      10,      10,      10,      10,
      10,      10,      10,      10,      10,      10,      10,      10,
      10,      10,      10,      10,      10,      10,      10,      10,
       9,       9,       9,       9,       9,       9,       9,       9,
       9,       9,       9,       9,       9,       9,       9,       9,
       9,       9,       9,       9,       9,       9,       9,       9,
       9,       8,       8,       8,       8,       8,       8,       8,
       8,       8,       8,       8,       8,       8,       8,       8
};

#if defined(GFX_SW_FLOAT_VERSION)
    static UINT32 _u4YCbCr2Rgb_RedErrorCount = 0;
    static UINT32 _u4YCbCr2Rgb_GreenErrorCount = 0;
    static UINT32 _u4YCbCr2Rgb_BlueErrorCount = 0;
#endif

#if (CONFIG_DRV_LINUX)
#if 0//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8560)
UINT32 VIRADDR(const MI_DIF_FIELD_T *prGfxReg, UINT32 u4Addr)
{
    UINT32 u4Rst;

    if (u4Addr == prGfxReg->fg_SRC_BSAD)
    {
        u4Rst = prGfxReg->fg_SRC_BSAD_H;
    }
    else if (u4Addr == prGfxReg->fg_DST_BSAD)
    {
        u4Rst = prGfxReg->fg_DST_BSAD_H;
    }
    else if (u4Addr == prGfxReg->fg_SRCCBCR_BSAD)
    {
        u4Rst = prGfxReg->fg_SRCCBCR_BSAD_H;
    }
    else if (u4Addr == prGfxReg->fg_PAL_BSAD)
    {
        u4Rst = prGfxReg->fg_PAL_BSAD_H;
    }
    else if (u4Addr == prGfxReg->fg_INDEX_BASD)
    {
        u4Rst = prGfxReg->fg_INDEX_BASD_H;
    }
    else if (u4Addr == prGfxReg->fg_BPCOMP_AD_END)
    {
        u4Rst = prGfxReg->fg_BPCOMP_AD_END_H;
    }
    else if (u4Addr == prGfxReg->fg_SRCCBCR_WBBSAD)
    {
        u4Rst = prGfxReg->fg_SRCCBCR_WBBSAD_H;
    }
    else if (u4Addr == prGfxReg->fg_DST_WBBSAD)
    {
        u4Rst = prGfxReg->fg_DST_WBBSAD_H;
    }
    else if (u4Addr == prGfxReg->fg_SRC_WBBSAD)
    {
        u4Rst = prGfxReg->fg_SRC_WBBSAD_H;
    }
    else if (u4Addr == prGfxReg->fg_DSTCBCR_BSAD)
    {
        u4Rst = prGfxReg->fg_DSTCBCR_BSAD_H;
    }
    else if (u4Addr == prGfxReg->fg_LEGAL_AD_END)
    {
        u4Rst = prGfxReg->fg_LEGAL_AD_END_H;
    }
    else if (u4Addr == prGfxReg->fg_LEGAL_AD_START)
    {
        u4Rst = prGfxReg->fg_LEGAL_AD_START_H;
    }
    else if (u4Addr == prGfxReg->fg_DRAMQ_BSAD)
    {
        u4Rst = prGfxReg->fg_DRAMQ_BSAD_H;
    }
    else
    {
        UTIL_Printf("....VIRADDR...Addr wrong...\n");
        u4Rst = u4Addr >> 30;
    }

    u4Rst <<= 30;
    u4Rst |= u4Addr;

    return (u4Rst);
}
#else
#define VIRADDR(prGfxReg, addr)   (__va(addr))
#endif
#else
#define VIRADDR(prGfxReg, addr)    (addr)
#endif

/* 8555 or newer IC, and 8550 ECO IC, use the following variables */
static UINT32 _u4RGB565Alpha = 0;
static UINT32 _u4UseRGB565Alpha = 0;
UINT32 g_u4I2DAlcom = 0; /* Just for I2D + Alcom + XXXX(Mirror/Flip/Stretch) */
//---------------------------------------------------------------------------
// Static functions
//---------------------------------------------------------------------------


//-------------------------------------------------------------------------
/** _pfnGfxSwCallBack
 *
 */
//-------------------------------------------------------------------------
static void (*_pfnGfxSwCallBack)(void *pvTag);


//-------------------------------------------------------------------------
/** _GfxSwColorExpansion
 *
 */
//-------------------------------------------------------------------------
static UINT32 _GfxSwColorExpansion(UINT32 u4Color, UINT32 u4ColorMode)
{
    switch (_au1sPixelLgDwn[u4ColorMode])
    {
    case 1:
        u4Color = (u4Color << 16) | (u4Color & 0xffff);
        break;

    case 2:
        u4Color = (u4Color <<  8) | (u4Color & 0xff);
        u4Color = (u4Color << 16) | (u4Color & 0xffff);
        break;

    case 3:
        u4Color = (u4Color <<  4) | (u4Color & 0xf);
        u4Color = (u4Color <<  8) | (u4Color & 0xff);
        u4Color = (u4Color << 16) | (u4Color & 0xffff);
        break;

    case 4:
        u4Color = (u4Color <<  2) | (u4Color & 0x3);
        u4Color = (u4Color <<  4) | (u4Color & 0xf);
        u4Color = (u4Color <<  8) | (u4Color & 0xff);
        u4Color = (u4Color << 16) | (u4Color & 0xffff);
        break;

    default:
        break;
    }

    return u4Color;
}


typedef struct _GFX_CLIP_PARA_T{
    UINT32 u4ClipEn;
    UINT32 u4DstX;
    UINT32 u4DstY;
    UINT32 u4ClipTopEn;
    UINT32 u4ClipTop;
    UINT32 u4ClipBotEn;
    UINT32 u4ClipBot;
    UINT32 u4ClipLeftEn;
    UINT32 u4ClipLeft;
    UINT32 u4ClipRightEn;
    UINT32 u4ClipRight;
}GFX_CLIP_PARA_T, *PGFX_CLIP_PARA_T;

static BOOL  fgGfxWithinClip(UINT32 u4CurX, UINT32 u4CurY, GFX_CLIP_PARA_T *prClipPara)
{
    BOOL fgWithinClip = TRUE;

    if (((prClipPara->u4ClipTopEn)   && (u4CurY <= (prClipPara->u4DstY + prClipPara->u4ClipTop)))  ||
        ((prClipPara->u4ClipBotEn)   && (u4CurY >  (prClipPara->u4DstY + prClipPara->u4ClipBot)))  ||
        ((prClipPara->u4ClipLeftEn)  && (u4CurX <= (prClipPara->u4DstX + prClipPara->u4ClipLeft))) ||
        ((prClipPara->u4ClipRightEn) && (u4CurX >= (prClipPara->u4DstX + prClipPara->u4ClipRight))) )
    {
        fgWithinClip = FALSE;
    }

    return (fgWithinClip);
}
INT32 GFX_SwBitBlt_Clip(UINT8 *pu1SrcBase, UINT8 *pu1DstBase, UINT32 u4SrcPitch,
    UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM, UINT32 u4Width,
    UINT32 u4Height, UINT32 u4ColorMin, UINT32 u4ColorMax, UINT32 u4TransEn,
    UINT32 u4ColchgEn, UINT32 u4KeynotEn, UINT32 u4Color, GFX_CLIP_PARA_T *prClipPara);

//-------------------------------------------------------------------------
/** _GfxSwTextBlt
 *
 */
//-------------------------------------------------------------------------
/*
static INT32 _GfxSwTextBlt(const MI_DIF_FIELD_T *prsGfxReg)
{
    UNUSED(prsGfxReg);

    return -(INT32)E_GFX_INV_ARG;

}
*/

//-------------------------------------------------------------------------
/** _GfxSwRectFill
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e613 -e740 */
static INT32 _GfxSwRectFill(const MI_DIF_FIELD_T *prsGfxReg)
{
    UINT8 *pu1DstBase, *pu1Write, *pu1DstLine;
    UINT32 *pu4Color;
    UINT32 u4Width, u4Height, u4DstPitch, u4Color, u4DstCM;
    UINT32 x, y;
    INT32 ai4RectColor[4] = {0};
    UINT32 ui4Temp = 0;

    GFX_SW_FX_ENTRY

    switch (prsGfxReg->fg_OP_MODE)
    {
    case OP_DRAW_HLINE:
        u4Width  = prsGfxReg->fg_SRC_WIDTH;
        u4Height = 1;
        break;

    case OP_DRAW_VLINE:
        u4Width  = 1;
        u4Height = prsGfxReg->fg_SRC_HEIGHT + 1;
        break;

    default:
        u4Width  = prsGfxReg->fg_SRC_WIDTH;
        u4Height = prsGfxReg->fg_SRC_HEIGHT + 1;
        break;
    }

    u4DstPitch = prsGfxReg->fg_OSD_WIDTH << 4;    // 128bit aligned
    u4DstCM    = prsGfxReg->fg_CM;
    u4Color    = prsGfxReg->fg_RECT_COLOR;
    ui4Temp    = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_DST_BSAD);

    pu1DstBase = (UINT8 *)ui4Temp;

    pu1Write    = pu1DstBase;
    pu1DstLine  = pu1DstBase;

    pu4Color = &u4Color;
    u4Color  = _GfxSwColorExpansion(u4Color, u4DstCM);
    GFX_SwGetColorComponent((UINT8 **)&pu4Color, u4DstCM, ai4RectColor);

    for (y = 0; y < u4Height; y++)
    {
        pu1Write = pu1DstLine;

        for (x = 0; x < u4Width; x++)
        {
            // write rect color to dst
            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM, ai4RectColor);
        }

        pu1DstLine += u4DstPitch;
    }

    return (INT32)E_GFX_OK;
}
/*lint -restore */


//-------------------------------------------------------------------------
/** _GfxSwDrawHline
 *
 */
//-------------------------------------------------------------------------
static INT32 _GfxSwDrawHline(const MI_DIF_FIELD_T *prsGfxReg)
{
    GFX_SW_FX_ENTRY

    return _GfxSwRectFill(prsGfxReg);
}


//-------------------------------------------------------------------------
/** _GfxSwDrawVline
 *
 */
//-------------------------------------------------------------------------
static INT32 _GfxSwDrawVline(const MI_DIF_FIELD_T *prsGfxReg)
{
    GFX_SW_FX_ENTRY

    return _GfxSwRectFill(prsGfxReg);
}


//-------------------------------------------------------------------------
/** _GfxSwDrawObliqueLine
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e613 -e740 */
static INT32 _GfxSwDrawObliqueLine(const MI_DIF_FIELD_T *prsGfxReg)
{
    UINT8 *pu1DstBase, *pu1Write; //, *pu1DstLine;
    UINT32 *pu4Color;
    //UINT32 u4Width, u4Height;
    UINT32 u4DstPitch, u4Color, u4DstCM;
    UINT32  u4SrcX, u4SrcY, u4DstX, u4DstY;
    //UINT32 x, y;
    INT32 ai4RectColor[4] = {0};
    UINT32  u4Bpp;

    INT32  i;
    INT32  dx, dy, x_inc, y_inc;
    INT32  x_major;
    INT32  mm, MM, K1, K2, Error, Delta, xpos, ypos; //, color;
    UINT32  ui4Temp = 0;

    GFX_SW_FX_ENTRY

    //u4Width  = prsGfxReg->fg_SRC_WIDTH;
    //u4Height = prsGfxReg->fg_SRC_HEIGHT + 1;

    u4DstPitch = prsGfxReg->fg_OSD_WIDTH << 4;    // 128bit aligned
    u4DstCM    = prsGfxReg->fg_CM;
    u4Color    = prsGfxReg->fg_RECT_COLOR;

    u4SrcX = prsGfxReg->fg_SRCX;
    u4SrcY = prsGfxReg->fg_SRCY;
    u4DstX = prsGfxReg->fg_DSTX;
    u4DstY = prsGfxReg->fg_DSTY;

    //pu1DstBase = (UINT8 *)(prsGfxReg->fg_DST_BSAD);
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_DST_BSAD);

    pu1DstBase = (UINT8 *)ui4Temp;

    //pu1Write    = pu1DstBase;
    //pu1DstLine  = pu1DstBase;

    pu4Color = &u4Color;
    u4Color  = _GfxSwColorExpansion(u4Color, u4DstCM);
    GFX_SwGetColorComponent((UINT8 **)&pu4Color, u4DstCM, ai4RectColor);

    //for (y = 0; y < u4Height; y++)
    //{
    //    pu1Write = pu1DstLine;
    //
    //    for (x = 0; x < u4Width; x++)
    //    {
    //        // write rect color to dst
    //        GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM, ai4RectColor);
    //    }
    //
    //    pu1DstLine += u4DstPitch;
    //}

    switch (u4DstCM)
    {
    case CM_RGB_CLUT8:
        u4Bpp = 1;
        break;
    case CM_RGB565_DIRECT16:
    case CM_ARGB1555_DIRECT16:
    case CM_ARGB4444_DIRECT16:
        u4Bpp = 2;
        break;
    case CM_ARGB8888_DIRECT32:
    default:
        u4Bpp = 4;
        break;
    }

    //dx = (u4X2 > u4X1) ? u4X2 - u4X1 : u4X1 - u4X2;
    //dy = (u4Y2 > u4Y1) ? u4Y2 - u4Y1 : u4Y1 - u4Y2;
    dx = (u4DstX > u4SrcX) ? u4DstX - u4SrcX : u4SrcX - u4DstX;
    dy = (u4DstY > u4SrcY) ? u4DstY - u4SrcY : u4SrcY - u4DstY;

    //x_inc = (u4X2 > u4X1) ? 1 : 0;
    //y_inc = (u4Y2 > u4Y1) ? 1 : 0;
    x_inc = (u4DstX > u4SrcX) ? 1 : 0;
    y_inc = (u4DstY > u4SrcY) ? 1 : 0;

    if (dx >= dy)
    {
        x_major = 1;
        mm = dy;
        MM = dx;
    }
    else
    {
        x_major = 0;
        mm = dx;
        MM = dy;
    }

    K1=2*mm;
    K2=2*(mm-MM);
    Error=2*mm-MM;
    //Delta=Error;   xpos=u4X1;   ypos=u4Y1;
    Delta=Error;   xpos=u4SrcX;   ypos=u4SrcY;
    //vDrawPixel(xpos, ypos, color);
    pu1Write = pu1DstBase + (ypos * u4DstPitch + xpos * u4Bpp);
    GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM, ai4RectColor);
    if (x_major != 0)
    {
        for (i=1; i<dx; i++)
        {
            if ( x_inc!=0 )
                xpos = xpos + 1;
            else
                xpos = xpos - 1;

            if ( Delta<=0 )
                Delta = Delta + K1;
            else
            {
                if ( y_inc!=0 )
                    ypos = ypos + 1;
                else
                    ypos = ypos - 1;
                Delta = Delta + K2;
            }
            //vDrawPixel(xpos, ypos, color);
            pu1Write = pu1DstBase + (ypos * u4DstPitch + xpos * u4Bpp);
            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM, ai4RectColor);
        }
    }
    else
    {
        for (i=1; i<dy; i++)
        {
            if ( y_inc!=0 )
                ypos = ypos + 1;
            else
                ypos = ypos - 1;

            if ( Delta<=0 )
                Delta = Delta + K1;
            else
          {
              if ( x_inc!=0 )
                  xpos = xpos + 1;
              else
                  xpos = xpos - 1;
                Delta=Delta+K2;
          }
            //vDrawPixel(xpos, ypos, color);
            pu1Write = pu1DstBase + (ypos * u4DstPitch + xpos * u4Bpp);
            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM, ai4RectColor);
        }
    }

    return (INT32)E_GFX_OK;
}
/*lint -restore */

//-------------------------------------------------------------------------
/** _GfxSwGradFill
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e613 */
static INT32 _GfxSwGradFill(const MI_DIF_FIELD_T *prsGfxReg)
{
    GFX_GRADIENT_DATA_T rData;
    UINT32 ui4Temp = 0;

    GFX_SW_FX_ENTRY

    rData.u4DstPitch  = prsGfxReg->fg_OSD_WIDTH << 4;    // 128bit aligned
    rData.u4DstCM     = prsGfxReg->fg_CM;
    rData.u4RectColor = prsGfxReg->fg_RECT_COLOR;

    //rData.pu1DstBase  = (UINT8 *)(prsGfxReg->fg_DST_BSAD);
    //for warning
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_DST_BSAD);
    rData.pu1DstBase = (UINT8 *)ui4Temp;

    rData.u4Width     = prsGfxReg->fg_SRC_WIDTH;
    rData.u4Height    = prsGfxReg->fg_SRC_HEIGHT + 1;
    rData.u4X_Pix_Inc = prsGfxReg->fg_GRAD_X_PIX_INC;
    rData.u4Y_Pix_Inc = prsGfxReg->fg_GRAD_Y_PIX_INC;
    rData.u4GradMode  = prsGfxReg->fg_GRAD_MODE;

    rData.u1Delta_X_C0 = prsGfxReg->fg_DELTA_X_C0;
    rData.u1Delta_X_C1 = prsGfxReg->fg_DELTA_X_C1;
    rData.u1Delta_X_C2 = prsGfxReg->fg_DELTA_X_C2;
    rData.u1Delta_X_C3 = prsGfxReg->fg_DELTA_X_C3;

    rData.u1Delta_Y_C0 = prsGfxReg->fg_DELTA_Y_C0;
    rData.u1Delta_Y_C1 = prsGfxReg->fg_DELTA_Y_C1;
    rData.u1Delta_Y_C2 = prsGfxReg->fg_DELTA_Y_C2;
    rData.u1Delta_Y_C3 = prsGfxReg->fg_DELTA_Y_C3;

    return GFX_SwGradientFill(&rData);
}
/*lint -restore */

INT32 GFX_SwBitBlt_R2Y(UINT8 *pu1SrcBase, UINT8 *pu1DstBase, UINT32 u4SrcPitch,
    UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM, UINT32 u4Width,
    UINT32 u4Height, UINT32 u4ColorMin, UINT32 u4ColorMax, UINT32 u4TransEn,
    UINT32 u4ColchgEn, UINT32 u4KeynotEn, UINT32 u4Color,UINT32 u4R2YY2REnable);
//-------------------------------------------------------------------------
/** _GfxSwBitBlt
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e613 */
#if 1
static INT32 _GfxSwBitBlt(const MI_DIF_FIELD_T *prsGfxReg)
{
    UINT8 *pu1SrcBase, *pu1DstBase;
    UINT32 u4SrcPitch, u4DstPitch;
    UINT32 u4SrcCM, u4DstCM;
    UINT32 u4Color;
    UINT32 u4Width, u4Height;
    UINT32 u4TransEn, u4ColchgEn, u4KeynotEn;
    UINT32 u4DstFlip = 0;
    UINT32 u4DstMirror = 0;
    UINT32 u4SrcFlip = 0;
    UINT32 u4SrcMirror = 0;
    UINT32 u4R2YY2REnable = 0;
    UINT32 u4Flip = 0;
    UINT32 u4Mirror = 0;
    UINT32 u4SrcFlag=0;
    UINT32 u4DstFlag=0;
    UINT32 ui4Temp = 0;
    
    UINT32 u4SrcColorMin;
    UINT32 u4SrcColorMax;
    UINT32 u4DstColorMin;
    UINT32 u4DstColorMax;
    
    UINT32 u4DisSrcKey;
    UINT32 u4DisDstKey;
    UINT32 u4SrcKeyIn;
    UINT32 u4DstKeyIn;
    UINT32 u4Alpha;
    // Clip Opt

    // get source & destination info
    //pu1SrcBase = (UINT8 *)(prsGfxReg->fg_SRC_BSAD);
    //for warning
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_SRC_BSAD);
    pu1SrcBase = (UINT8 *)ui4Temp;

    u4SrcPitch = prsGfxReg->fg_SRC_PITCH << 4;
    u4SrcCM    = prsGfxReg->fg_SRC_CM;
    //pu1DstBase = (UINT8 *)(prsGfxReg->fg_DST_BSAD);

    ui4Temp = 0;
    //for warning
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_DST_BSAD);
    pu1DstBase = (UINT8 *)ui4Temp;

    u4DstPitch = prsGfxReg->fg_OSD_WIDTH << 4;
    u4DstCM    = prsGfxReg->fg_CM;
    u4Width    = prsGfxReg->fg_SRC_WIDTH;
    u4Height   = prsGfxReg->fg_SRC_HEIGHT + 1;
    u4TransEn  = prsGfxReg->fg_TRANS_ENA;
    u4ColchgEn = prsGfxReg->fg_COLCHG_ENA;
    u4KeynotEn = prsGfxReg->fg_KEYNOT_ENA;
    u4Color    = prsGfxReg->fg_RECT_COLOR;
    u4SrcColorMin = prsGfxReg->fg_COLOR_KEY_MIN;
    u4SrcColorMax = prsGfxReg->fg_COLOR_KEY_MAX;
    u4DstColorMin = prsGfxReg->fg_DST_COLOR_KEY_MIN;
    u4DstColorMax = prsGfxReg->fg_DST_COLOR_KEY_MAX;
    
    u4DisSrcKey   = prsGfxReg->fg_DIS_SRC_KEY;
    u4DisDstKey   = prsGfxReg->fg_DIS_DST_KEY;
    u4SrcKeyIn    = prsGfxReg->fg_SRC_KEY_IN;
    u4DstKeyIn    = prsGfxReg->fg_DST_KEY_IN;
    u4Alpha       = prsGfxReg->fg_ALPHA_VALUE;

    u4DstFlip = prsGfxReg->fg_DSTPITCH_DEC;
    u4DstMirror = prsGfxReg->fg_DST_MIRR_OR;
    u4SrcFlip = prsGfxReg->fg_SRCPITCH_DEC;
    u4SrcMirror = prsGfxReg->fg_SRC_MIRR_OR;
    u4R2YY2REnable = prsGfxReg->fg_YUVRGB_MODE;
    u4Flip = (u4DstFlip&(~u4SrcFlip))|((~u4DstFlip)&u4SrcFlip);
    u4Mirror = (u4DstMirror&(~u4SrcMirror))|((~u4DstMirror)&u4SrcMirror);

    u4SrcFlag=u4SrcFlip|(u4SrcMirror<<1);
    u4DstFlag=u4DstFlip|(u4DstMirror<<1);
    if( u4Flip|u4Mirror )  //dst flip and mirror
    {
        return GFX_SwBitBlt_DstFlip(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch,
            u4SrcCM, u4DstCM, u4Width, u4Height, u4SrcColorMin, u4SrcColorMax,
            u4TransEn, u4ColchgEn, u4KeynotEn, u4Color,u4SrcFlag, u4DstFlag);
    }
    if(u4R2YY2REnable/4) //rgb2yuv and yuv2rgb
    {
        return GFX_SwBitBlt_R2Y(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch,
            u4SrcCM, u4DstCM, u4Width, u4Height, u4SrcColorMin, u4SrcColorMax,
            u4TransEn, u4ColchgEn, u4KeynotEn, u4Color,u4R2YY2REnable);
    }

    return GFX_SwBitBlt(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch,
            u4SrcCM, u4DstCM, u4Width, u4Height, u4SrcColorMin, u4SrcColorMax, u4DstColorMin, u4DstColorMax,
            u4TransEn, u4ColchgEn, u4DisSrcKey, u4SrcKeyIn, u4DisDstKey, u4DstKeyIn, u4KeynotEn, u4Color, u4Alpha);
}
/*lint -restore */

#else
static INT32 _GfxSwBitBlt(const MI_DIF_FIELD_T *prsGfxReg)
{
    UINT8 *pu1SrcBase, *pu1DstBase;
    UINT32 u4SrcPitch, u4DstPitch;
    UINT32 u4SrcCM, u4DstCM;
    UINT32 u4Color, u4ColorMin, u4ColorMax;
    UINT32 u4Width, u4Height;
    UINT32 u4TransEn, u4ColchgEn, u4KeynotEn;
    UINT32 u4DstFlip = 0;
    UINT32 u4DstMirror = 0;
    UINT32 u4SrcFlip = 0;
    UINT32 u4SrcMirror = 0;
    UINT32 u4R2YY2REnable = 0;
    UINT32 u4Flip = 0;
    UINT32 u4Mirror = 0;
    UINT32 u4SrcFlag=0;
    UINT32 u4DstFlag=0;
    UINT32 ui4Temp = 0;
    // Clip Opt
    GFX_CLIP_PARA_T rClipPara = {0};

    // get source & destination info
    //pu1SrcBase = (UINT8 *)(prsGfxReg->fg_SRC_BSAD);
    //for warning
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_SRC_BSAD);
    pu1SrcBase = (UINT8 *)ui4Temp;

    u4SrcPitch = prsGfxReg->fg_SRC_PITCH << 4;
    u4SrcCM    = prsGfxReg->fg_SRC_CM;
    //pu1DstBase = (UINT8 *)(prsGfxReg->fg_DST_BSAD);

    ui4Temp = 0;
    //for warning
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_DST_BSAD);
    pu1DstBase = (UINT8 *)ui4Temp;

    u4DstPitch = prsGfxReg->fg_OSD_WIDTH << 4;
    u4DstCM    = prsGfxReg->fg_CM;
    u4Width    = prsGfxReg->fg_SRC_WIDTH;
    u4Height   = prsGfxReg->fg_SRC_HEIGHT + 1;
    u4TransEn  = prsGfxReg->fg_TRANS_ENA;
    u4ColchgEn = prsGfxReg->fg_COLCHG_ENA;
    u4KeynotEn = prsGfxReg->fg_KEYNOT_ENA;
    u4Color    = prsGfxReg->fg_RECT_COLOR;
    u4ColorMin = prsGfxReg->fg_COLOR_KEY_MIN;
    u4ColorMax = prsGfxReg->fg_COLOR_KEY_MAX;

    u4DstFlip = prsGfxReg->fg_DSTPITCH_DEC;
    u4DstMirror = prsGfxReg->fg_DST_MIRR_OR;
    u4SrcFlip = prsGfxReg->fg_SRCPITCH_DEC;
    u4SrcMirror = prsGfxReg->fg_SRC_MIRR_OR;
    u4R2YY2REnable = prsGfxReg->fg_YUVRGB_MODE;
    u4Flip = (u4DstFlip&(~u4SrcFlip))|((~u4DstFlip)&u4SrcFlip);
    u4Mirror = (u4DstMirror&(~u4SrcMirror))|((~u4DstMirror)&u4SrcMirror);

    rClipPara.u4ClipEn      = prsGfxReg->fg_CLIP_ENA;
    rClipPara.u4DstX        = prsGfxReg->fg_DSTX;
    rClipPara.u4DstY        = prsGfxReg->fg_DSTY;
    rClipPara.u4ClipTopEn   = prsGfxReg->fg_CLT_ENA;
    rClipPara.u4ClipTop     = prsGfxReg->fg_CLIP_TOP;
    rClipPara.u4ClipBotEn   = prsGfxReg->fg_CLB_ENA;
    rClipPara.u4ClipBot     = prsGfxReg->fg_CLIP_BOT;
    rClipPara.u4ClipLeftEn  = prsGfxReg->fg_CLL_ENA;
    rClipPara.u4ClipLeft    = prsGfxReg->fg_CLIP_LEFT;
    rClipPara.u4ClipRightEn = prsGfxReg->fg_CLR_ENA;
    rClipPara.u4ClipRight   = prsGfxReg->fg_CLIP_RIGHT;

    u4SrcFlag=u4SrcFlip|(u4SrcMirror<<1);
    u4DstFlag=u4DstFlip|(u4DstMirror<<1);
    if( u4Flip|u4Mirror )  //dst flip and mirror
    {
        return GFX_SwBitBlt_DstFlip(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch,
            u4SrcCM, u4DstCM, u4Width, u4Height, u4ColorMin, u4ColorMax,
            u4TransEn, u4ColchgEn, u4KeynotEn, u4Color,u4SrcFlag, u4DstFlag);
    }
    if(u4R2YY2REnable/4) //rgb2yuv and yuv2rgb
    {
        return GFX_SwBitBlt_R2Y(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch,
            u4SrcCM, u4DstCM, u4Width, u4Height, u4ColorMin, u4ColorMax,
            u4TransEn, u4ColchgEn, u4KeynotEn, u4Color,u4R2YY2REnable);
    }

    if (rClipPara.u4ClipEn)
    {
        return GFX_SwBitBlt_Clip(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch,
            u4SrcCM, u4DstCM, u4Width, u4Height, u4ColorMin, u4ColorMax,
            u4TransEn, u4ColchgEn, u4KeynotEn, u4Color, &rClipPara);
    }

#if (CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8561)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8563)
    return GFX_SwBitBlt_NewMethod(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch,
            u4SrcCM, u4DstCM, u4Width, u4Height, u4ColorMin, u4ColorMax,
            u4TransEn, u4ColchgEn, u4KeynotEn, u4Color);
#else
    return GFX_SwBitBlt(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch,
            u4SrcCM, u4DstCM, u4Width, u4Height, u4ColorMin, u4ColorMax,
            u4TransEn, u4ColchgEn, u4KeynotEn, u4Color);
#endif
}
/*lint -restore */
#endif



//-------------------------------------------------------------------------
/** _Gfx24BppSwBitBlt
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e613 */
static INT32 _Gfx24BppSwBitBlt(const MI_DIF_FIELD_T *prsGfx24BppReg)
{
    UINT8 *pu1SrcBase, *pu1DstBase;
    UINT32 u4SrcPitch, u4DstPitch;
    UINT32 u4SrcCM, u4DstCM;
    UINT32 u4Color, u4ColorMin, u4ColorMax;
    UINT32 u4Width, u4Height;
    UINT32 u4TransEn, u4ColchgEn, u4KeynotEn;
    UINT32 u4DstFlip = 0;
    UINT32 u4DstMirror = 0;
    UINT32 u4SrcFlip = 0;
    UINT32 u4SrcMirror = 0;
    UINT32 u4R2YY2REnable = 0;
    UINT32 u4Flip = 0;
    UINT32 u4Mirror = 0;
    UINT32 u4SrcFlag=0;
    UINT32 u4DstFlag=0;
    UINT32 ui4Temp = 0;
    UINT32 u4GAlphaEn, u4GAlpha;
    // Clip Opt
    GFX_CLIP_PARA_T rClipPara = {0};

    // get source & destination info
    //pu1SrcBase = (UINT8 *)(prsGfx24BppReg->fg_SRC_BSAD);
    //for warning
	
    ui4Temp = VIRADDR(prsGfx24BppReg, (UINT32)prsGfx24BppReg->fg_SRC_BSAD_24BPP);
    pu1SrcBase = (UINT8 *)ui4Temp;

    u4SrcPitch = prsGfx24BppReg->fg_SRC_PITCH << 4;
    u4SrcCM    = prsGfx24BppReg->fg_SRC_CM;
    //pu1DstBase = (UINT8 *)(prsGfx24BppReg->fg_DST_BSAD);

    ui4Temp = 0;
    //for warning
    ui4Temp = VIRADDR(prsGfx24BppReg, (UINT32)prsGfx24BppReg->fg_DST_BSAD);
    pu1DstBase = (UINT8 *)ui4Temp;

    u4DstPitch = prsGfx24BppReg->fg_OSD_WIDTH << 4;
    u4DstCM    = prsGfx24BppReg->fg_CM;
    u4Width    = prsGfx24BppReg->fg_SRC_WIDTH_24BPP;
    u4Height   = prsGfx24BppReg->fg_SRC_HEIGHT_24BPP + 1;
    u4TransEn  = prsGfx24BppReg->fg_TRANS_ENA;
    u4ColchgEn = prsGfx24BppReg->fg_COLCHG_ENA;
    u4KeynotEn = prsGfx24BppReg->fg_KEYNOT_ENA;
    u4Color    = prsGfx24BppReg->fg_RECT_COLOR;
    u4ColorMin = prsGfx24BppReg->fg_COLOR_KEY_MIN;
    u4ColorMax = prsGfx24BppReg->fg_COLOR_KEY_MAX;
    u4GAlphaEn = prsGfx24BppReg->fg_GLOABLE_ALPHA_EN;
	UTIL_Printf("[GFX_SwBitBlt] u4GAlphaEn = %d !!!\n",u4GAlphaEn);
    u4GAlpha   = prsGfx24BppReg->fg_ALPHA_VALUE;
    u4DstFlip = prsGfx24BppReg->fg_DSTPITCH_DEC;
    u4DstMirror = prsGfx24BppReg->fg_DST_MIRR_OR;
    u4SrcFlip = prsGfx24BppReg->fg_SRCPITCH_DEC;
    u4SrcMirror = prsGfx24BppReg->fg_SRC_MIRR_OR;
    u4R2YY2REnable = prsGfx24BppReg->fg_YUVRGB_MODE;
    u4Flip = (u4DstFlip&(~u4SrcFlip))|((~u4DstFlip)&u4SrcFlip);
    u4Mirror = (u4DstMirror&(~u4SrcMirror))|((~u4DstMirror)&u4SrcMirror);

    rClipPara.u4ClipEn      = prsGfx24BppReg->fg_CLIP_ENA;
    rClipPara.u4DstX        = prsGfx24BppReg->fg_DSTX;
    rClipPara.u4DstY        = prsGfx24BppReg->fg_DSTY;
    rClipPara.u4ClipTopEn   = prsGfx24BppReg->fg_CLT_ENA;
    rClipPara.u4ClipTop     = prsGfx24BppReg->fg_CLIP_TOP;
    rClipPara.u4ClipBotEn   = prsGfx24BppReg->fg_CLB_ENA;
    rClipPara.u4ClipBot     = prsGfx24BppReg->fg_CLIP_BOT;
    rClipPara.u4ClipLeftEn  = prsGfx24BppReg->fg_CLL_ENA;
    rClipPara.u4ClipLeft    = prsGfx24BppReg->fg_CLIP_LEFT;
    rClipPara.u4ClipRightEn = prsGfx24BppReg->fg_CLR_ENA;
    rClipPara.u4ClipRight   = prsGfx24BppReg->fg_CLIP_RIGHT;

    u4SrcFlag=u4SrcFlip|(u4SrcMirror<<1);
    u4DstFlag=u4DstFlip|(u4DstMirror<<1);

    return GFX_24Bpp_SwBitBlt(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch,
            u4SrcCM, u4DstCM, u4Width, u4Height, u4ColorMin, u4ColorMax,
            u4TransEn, u4ColchgEn, u4KeynotEn, u4Color, u4GAlphaEn, u4GAlpha);//
}
/*lint -restore */


//-------------------------------------------------------------------------
/** _GfxDma
 *  1-D bitblt
 */
//-------------------------------------------------------------------------
/*lint -save -e613 */
static INT32 _GfxDma(const MI_DIF_FIELD_T *prsGfxReg)
{
    UINT32 *pu4Src, *pu4Dst;
    UINT32 u4NDW;
    UINT32 ui4Temp = 0;

    //pu4Src = (UINT32 *)(prsGfxReg->fg_SRC_BSAD);
    //pu4Dst = (UINT32 *)(prsGfxReg->fg_DST_BSAD);

    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_SRC_BSAD);
    pu4Src = (UINT32 *)ui4Temp;

    ui4Temp = 0;
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_DST_BSAD);
    pu4Dst = (UINT32 *)ui4Temp;

    u4NDW = (prsGfxReg->fg_SRC_HEIGHT << 11) | prsGfxReg->fg_SRC_WIDTH;

    while (u4NDW-- > 0)
    {
        *pu4Dst++ = *pu4Src++;
    }

    return (INT32)E_GFX_OK;
}
/*lint -restore */


//-------------------------------------------------------------------------
/** _GfxAlphaBitBlt
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e613 */
static INT32 _GfxAlphaBitBlt(const MI_DIF_FIELD_T *prsGfxReg)
{
    UINT8 *pu1SrcBase, *pu1DstBase;
    UINT32 u4SrcPitch, u4DstPitch;
    UINT32 u4DstCM, u4Alpha;
    UINT32 u4Width, u4Height;
    UINT32 ui4Temp = 0;

    // get source & destination info
    //for warning
    //pu1SrcBase = (UINT8 *)(prsGfxReg->fg_SRC_BSAD);

    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_SRC_BSAD);
    pu1SrcBase = (UINT8 *)ui4Temp;

    u4SrcPitch = prsGfxReg->fg_SRC_PITCH << 4;
    //pu1DstBase = (UINT8 *)(prsGfxReg->fg_DST_BSAD);
    ui4Temp = 0;
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_DST_BSAD);
    pu1DstBase = (UINT8 *)ui4Temp;

    u4DstPitch = prsGfxReg->fg_OSD_WIDTH << 4;
    u4DstCM    = prsGfxReg->fg_CM;
    u4Alpha    = prsGfxReg->fg_ALPHA_VALUE;
    u4Width    = prsGfxReg->fg_SRC_WIDTH;
    u4Height   = prsGfxReg->fg_SRC_HEIGHT + 1;

    return GFX_SwAlphaBlending(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch,
            u4DstCM, u4Alpha, u4Width, u4Height);
}
/*lint -restore */

/** _GfxMsAlphaBitBlt
 *
 */
static INT32 _GfxMsAlphaBitBlt(const MI_DIF_FIELD_T *prsGfxReg)
{
    UINT8 *pu1SrcBase, *pu1DstBase;
	UINT32 u4SrcPitch, u4DstPitch;
	UINT32 u4DstCM, u4Alpha, u4SrcPremult;
	UINT32 u4Width, u4Height;
	UINT32 ui4Temp;

	//get source & destination info 
	ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_SRC_BSAD);
	pu1SrcBase = (UINT8 *)ui4Temp;

	u4SrcPitch = prsGfxReg->fg_SRC_PITCH << 4;

    ui4Temp =  VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_DST_BSAD);
	pu1DstBase = (UINT8 *)ui4Temp;

	u4DstPitch   = prsGfxReg->fg_OSD_WIDTH << 4;
    u4DstCM      = prsGfxReg->fg_CM;
	u4Alpha      = prsGfxReg->fg_ALPHA_VALUE;
	u4SrcPremult = prsGfxReg->fg_SRC_PREMULT;
	u4Width      = prsGfxReg->fg_SRC_WIDTH;
	u4Height     = prsGfxReg->fg_SRC_HEIGHT + 1;
	
    return GFX_SwMsAlphaBlending(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch,
		u4DstCM, u4Alpha, u4SrcPremult, u4Width, u4Height);	
	
}


//-------------------------------------------------------------------------
/** _GfxAlphaComposition
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e613 */
static INT32 _GfxAlphaComposition(const MI_DIF_FIELD_T *prsGfxReg)
{
    UINT8 *pu1SrcBase, *pu1DstBase;
    UINT32 u4SrcPitch, u4DstPitch;
    UINT32 u4DstCM, u4Alpha;
    UINT32 u4Width, u4Height, u4Alcom;
    UINT32 ui4Temp = 0;

    // get source & destination info
    //pu1SrcBase = (UINT8 *)(prsGfxReg->fg_SRC_BSAD);
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_SRC_BSAD);
    pu1SrcBase = (UINT8 *)ui4Temp;

    u4SrcPitch = prsGfxReg->fg_SRC_PITCH << 4;
    //pu1DstBase = (UINT8 *)(prsGfxReg->fg_DST_BSAD);
    ui4Temp = 0;
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_DST_BSAD);
    pu1DstBase = (UINT8 *)ui4Temp;

    u4DstPitch = prsGfxReg->fg_OSD_WIDTH << 4;
    u4DstCM    = prsGfxReg->fg_CM;
    u4Alpha    = prsGfxReg->fg_ALPHA_VALUE;
    u4Width    = prsGfxReg->fg_SRC_WIDTH;
    u4Height   = prsGfxReg->fg_SRC_HEIGHT + 1;
    u4Alcom    = prsGfxReg->fg_ALCOM_PASS;

    return GFX_SwAlphaComposePass(pu1SrcBase, pu1DstBase, u4SrcPitch,
            u4DstPitch, u4DstCM, u4DstCM, u4Width, u4Height, u4Alpha, u4Alcom);
}
/*lint -restore */


//-------------------------------------------------------------------------
/** _GfxYCrCb2Rgb
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e613 */
static INT32 _GfxYCrCb2Rgb(const MI_DIF_FIELD_T *prsGfxReg)
{
    GFX_YCBCR2RGB_DATA_T rData;
    UINT32 ui4Temp = 0;

    // get source & destination info
    //
    //rData.pu1LumaBase   = (UINT8 *)(prsGfxReg->fg_SRC_BSAD);
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_SRC_BSAD);
    rData.pu1LumaBase = (UINT8 *)ui4Temp;

    rData.u4LumaPitch   = prsGfxReg->fg_SRC_PITCH << 4;
    //rData.pu1ChromaBase = (UINT8 *)(prsGfxReg->fg_SRCCBCR_BSAD);
    ui4Temp = 0;
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_SRCCBCR_BSAD);
    rData.pu1ChromaBase = (UINT8 *)ui4Temp;

    rData.u4ChromaPitch = prsGfxReg->fg_SRCCBCR_PITCH << 4;
    //rData.pu1DstBase    = (UINT8 *)(prsGfxReg->fg_DST_BSAD);
    ui4Temp = 0;
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_DST_BSAD);
    rData.pu1DstBase = (UINT8 *)ui4Temp;

    rData.u4DstPitch    = prsGfxReg->fg_OSD_WIDTH << 4;
    rData.u4DstCM       = prsGfxReg->fg_CM;
    rData.u4Width       = prsGfxReg->fg_SRC_WIDTH;
    rData.u4Height      = prsGfxReg->fg_SRC_HEIGHT + 1;
    rData.u4YCFormat    = prsGfxReg->fg_YC_FMT;
    rData.u4VideoStd    = prsGfxReg->fg_VSTD;
    rData.u4VideoSys    = prsGfxReg->fg_VSYS;
    rData.u4VideoClip   = prsGfxReg->fg_VSCLIP;
    rData.u4PicSelect   = prsGfxReg->fg_FLD_PIC;
    rData.u4SwapMode    = prsGfxReg->fg_SWAP_MODE;

    return GFX_SwYCbCr2RGB(&rData);
}
/*lint -restore */


//-------------------------------------------------------------------------
/** _GfxAlphaMapBitBlt
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e613 */
static INT32 _GfxAlphaMapBitBlt(const MI_DIF_FIELD_T *prsGfxReg)
{
    UINT8 *pu1SrcBase, *pu1DstBase;
    UINT32 u4SrcPitch, u4DstPitch;
    UINT32 u4SrcCM, u4DstCM;
    UINT32 u4Width, u4Height;
    UINT32 ui4Temp = 0;

    // get source & destination info
    //pu1SrcBase = (UINT8 *)(prsGfxReg->fg_SRC_BSAD);
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_SRC_BSAD);
    pu1SrcBase = (UINT8 *)ui4Temp;

    u4SrcPitch = prsGfxReg->fg_SRC_PITCH << 4;
    u4SrcCM    = prsGfxReg->fg_SRC_CM;
    //pu1DstBase = (UINT8 *)(prsGfxReg->fg_DST_BSAD);
    ui4Temp = 0;
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_DST_BSAD);
    pu1DstBase = (UINT8 *)ui4Temp;

    u4DstPitch = prsGfxReg->fg_OSD_WIDTH << 4;
    u4DstCM    = prsGfxReg->fg_CM;
    u4Width    = prsGfxReg->fg_SRC_WIDTH;
    u4Height   = prsGfxReg->fg_SRC_HEIGHT + 1;

    return GFX_SwAlphaMapBitBlt(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch,
            u4SrcCM, u4DstCM, u4Width, u4Height);
}
/*lint -restore */

/*lint -save -e613 */
static INT32  _GfxSwRopBitBlt(const MI_DIF_FIELD_T *prsGfxReg)
{
    UINT8 *pu1SrcBase, *pu1DstBase;
    UINT8  *pu1DstX, *pu1DstY, *pu1CmpNowr;
    UINT32 u4SrcPitch, u4DstPitch;
    UINT32 u4SrcCM, u4DstCM;
    UINT32 u4Width, u4Height;
    UINT32 u4RopOpCode, u4nowr, u4cmpflag, u4JavaXorClr;
    UINT32 ui4Temp = 0;
    UINT32 u4ColorRepEn;
    /* 8555 or newer IC, and 8550 ECO IC, use the following variables */
    UINT32 u4SrcAlphaCheck;
    // get source & destination info
    //pu1SrcBase = (UINT8 *)(prsGfxReg->fg_SRC_BSAD);
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_SRC_BSAD);
    pu1SrcBase = (UINT8 *)ui4Temp;

    u4SrcPitch = prsGfxReg->fg_SRC_PITCH << 4;
    u4SrcCM    = prsGfxReg->fg_SRC_CM;
    //pu1DstBase = (UINT8 *)(prsGfxReg->fg_DST_BSAD);

    ui4Temp = 0;
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_DST_BSAD);
    pu1DstBase = (UINT8 *)ui4Temp;

    u4DstPitch = prsGfxReg->fg_OSD_WIDTH << 4;
    u4DstCM    = prsGfxReg->fg_CM;
    u4Width    = prsGfxReg->fg_SRC_WIDTH;
    u4Height   = prsGfxReg->fg_SRC_HEIGHT + 1;
    u4RopOpCode  = prsGfxReg->fg_ROP_OPCODE;
    u4nowr       = prsGfxReg->fg_NO_WR;
    u4cmpflag    = prsGfxReg->fg_CMP_FLAG;
    //pu1DstX     = (UINT8 *)(prsGfxReg->fg_DSTX);
    ui4Temp = 0;
    ui4Temp = (UINT32)prsGfxReg->fg_DSTX;
    pu1DstX = (UINT8 *)ui4Temp;

    //pu1DstY     = (UINT8 *)(prsGfxReg->fg_DSTY);
    ui4Temp = 0;
    ui4Temp = (UINT32)prsGfxReg->fg_DSTY;
    pu1DstY = (UINT8 *)ui4Temp;

    //pu1CmpNowr  = (UINT8 *)(prsGfxReg->fg_NO_WR);
    ui4Temp = 0;
    ui4Temp = (UINT32)prsGfxReg->fg_NO_WR;
    pu1CmpNowr = (UINT8 *)ui4Temp;

    u4JavaXorClr = prsGfxReg->fg_JAVA_XOR_COLOR;
#if 1//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8561)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    u4ColorRepEn = prsGfxReg->fg_COLORIZE_REP;
        u4SrcAlphaCheck = prsGfxReg->fg_SRCALPHA_CHECK;
        _u4RGB565Alpha = prsGfxReg->fg_ALPHA_VALUE;
        _u4UseRGB565Alpha = TRUE;

        ui4Temp = GFX_SwRopBitBlt8580(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch,
            u4SrcCM, u4DstCM, u4Width, u4Height, u4RopOpCode, u4nowr, u4cmpflag,
            pu1DstX, pu1DstY,pu1CmpNowr, u4JavaXorClr, u4SrcAlphaCheck, u4ColorRepEn);

        _u4UseRGB565Alpha = FALSE;

        return (ui4Temp);

#elif 0//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)
    u4SrcAlphaCheck = prsGfxReg->fg_SRCALPHA_CHECK;
    _u4RGB565Alpha = prsGfxReg->fg_ALPHA_VALUE;
    _u4UseRGB565Alpha = TRUE;

    ui4Temp = GFX_SwRopBitBlt8555(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch,
        u4SrcCM, u4DstCM, u4Width, u4Height, u4RopOpCode, u4nowr, u4cmpflag,
        pu1DstX, pu1DstY,pu1CmpNowr, u4JavaXorClr, u4SrcAlphaCheck);

    _u4UseRGB565Alpha = FALSE;

    return (ui4Temp);
#elif 0//(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8550)
    if (BSP_GetIcVersion() >= IC_8550_VER_A)
    { // 8550 ECO IC: Same as 8555
        u4SrcAlphaCheck = (prsGfxReg->fg_Res4134 >> 7) & 0x01;
        _u4RGB565Alpha = prsGfxReg->fg_ALPHA_VALUE;
        _u4UseRGB565Alpha = TRUE;

        ui4Temp = GFX_SwRopBitBlt8555(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch,
            u4SrcCM, u4DstCM, u4Width, u4Height, u4RopOpCode, u4nowr, u4cmpflag,
            pu1DstX, pu1DstY,pu1CmpNowr, u4JavaXorClr, u4SrcAlphaCheck);

        _u4UseRGB565Alpha = FALSE;

        return (ui4Temp);
    }
    else
    {
        return GFX_SwRopBitBlt(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch,
            u4SrcCM, u4DstCM, u4Width, u4Height, u4RopOpCode, u4nowr, u4cmpflag,
            pu1DstX, pu1DstY,pu1CmpNowr, u4JavaXorClr);
    }
#else
    return GFX_SwRopBitBlt(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch,
        u4SrcCM, u4DstCM, u4Width, u4Height, u4RopOpCode, u4nowr, u4cmpflag,
        pu1DstX, pu1DstY,pu1CmpNowr, u4JavaXorClr);
#endif
}
/*lint -restore */

/*lint -save -e613 */
#if 0
static INT32  _GfxSwInx2DirBitBlt(const MI_DIF_FIELD_T *prsGfxReg)
{
  UINT8 *pu1SrcBase, *pu1DstBase;
  UINT8 *pu1PalBase;
    UINT32 u4SrcPitch, u4DstPitch;
    UINT32 u4CharCM, u4DstCM;
    UINT32 u4Width, u4Height;
  UINT32 u4LnStByeAl, u4MsbLeft;

    // get source & destination info
    pu1SrcBase = (UINT8 *)prsGfxReg->fg_SRC_BSAD;
    u4SrcPitch = prsGfxReg->fg_SRC_PITCH << 4;
    pu1DstBase = (UINT8 *)prsGfxReg->fg_DST_BSAD;
    u4DstPitch = prsGfxReg->fg_OSD_WIDTH << 4;
    u4DstCM    = prsGfxReg->fg_CM;
    u4Width    = prsGfxReg->fg_SRC_WIDTH;
    u4Height   = prsGfxReg->fg_SRC_HEIGHT + 1;
  u4CharCM   = prsGfxReg->fg_CHAR_CM;
  u4MsbLeft  = prsGfxReg->fg_MSB_LEFT;
  pu1PalBase = (UINT8 *)prsGfxReg->fg_PAL_BSAD;
    u4LnStByeAl = prsGfxReg->fg_LN_ST_BYTE_AL;


    return GFX_SwInx2DirBitBlt(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch,
      u4CharCM, u4DstCM, u4Width, u4Height, u4LnStByeAl, u4MsbLeft,
      pu1PalBase);
}
/*lint -restore */
#endif
/*lint -save -e613 */
static INT32  _GfxSwH2VLine(const MI_DIF_FIELD_T *prsGfxReg)
{
    UINT8 *pu1SrcBase, *pu1DstBase;
    UINT32 u4DstCM;
  UINT32 u4DstPitch;
  UINT32 u4Alpha;
    UINT32 u4SrcWidth, u4SrcHeight, u4StrDstWidth, u4StrDstHeight;
    UINT32 u4wise=0;
    UINT32 ui4Temp = 0;

    // get source & destination info
    //pu1SrcBase = (UINT8 *)(prsGfxReg->fg_SRC_BSAD);
    //pu1DstBase = (UINT8 *)(prsGfxReg->fg_DST_BSAD);

    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_SRC_BSAD);
    pu1SrcBase = (UINT8 *)ui4Temp;

    ui4Temp = 0;
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_DST_BSAD);
    pu1DstBase = (UINT8 *)ui4Temp;

    u4DstPitch = prsGfxReg->fg_OSD_WIDTH << 4;
    u4DstCM    = prsGfxReg->fg_CM;
    u4SrcWidth    = prsGfxReg->fg_SRC_WIDTH;
    u4SrcHeight   = prsGfxReg->fg_SRC_HEIGHT + 1;
  u4StrDstWidth    = prsGfxReg->fg_STR_DST_WIDTH;
  u4StrDstHeight   = prsGfxReg->fg_STR_DST_HEIGHT+1;
      u4wise=prsGfxReg->fg_DSTPITCH_DEC;
    u4Alpha = prsGfxReg->fg_ALPHA_VALUE;

    return GFX_SwH2VLine(pu1SrcBase, pu1DstBase, u4DstPitch, u4DstCM,
    u4SrcWidth, u4SrcHeight, u4StrDstWidth, u4StrDstHeight, u4Alpha, u4wise);
}
/*lint -restore */

/*lint -save -e613 */
static INT32  _GfxSwPgigDecode(const MI_DIF_FIELD_T *prsGfxReg)
{
    UINT8 *pu1SrcBase, *pu1DstBase, *pu1PalBase;
    UINT32 u4DstCM, u4DstPitch;
    UINT32 u4Width, u4Height;
    UINT32 ui4Temp = 0;


    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_SRC_BSAD);
    pu1SrcBase = (UINT8 *)ui4Temp;

    ui4Temp = 0;
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_DST_BSAD);
    pu1DstBase = (UINT8 *)ui4Temp;

    ui4Temp = 0;
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_PAL_BSAD);
    pu1PalBase = (UINT8 *)ui4Temp;

    u4DstPitch = prsGfxReg->fg_OSD_WIDTH << 4;
    u4DstCM    = prsGfxReg->fg_CM;

    u4Width    = prsGfxReg->fg_SRC_WIDTH;
    u4Height   = prsGfxReg->fg_SRC_HEIGHT + 1;

    return GFX_SwPgigDecode(pu1SrcBase, pu1DstBase, pu1PalBase,
                            u4Width, u4Height, u4DstPitch, u4DstCM);
}
/*lint -restore */
/*lint -restore */


//-------------------------------------------------------------------------
/** _GfxSwAlphaCompositionLoop
 *  Alpha composition loop mode
 */
//-------------------------------------------------------------------------
/*lint -save -e613 */
/*lint -save -e704 -e613 */

// for Alpha Composition Loop Mode use
#define COMP_NUM            4   // A,R,G,B
#define DIV_SHT_1           4
#if (VERIFY_ALCOM == 1)
#define TABLE_SIZE          4096
#else
#define TABLE_SIZE          1
#endif
#define PASS3_SHT_NUM_1     12
#define ALPHA_SHT           8
#define ALPHA_SHT_16BPP     (ALPHA_SHT+4)
#define ALPHA_SHT_16BPP_565   (ALPHA_SHT+2)
#define ALPHA_SHT_16BPP_1555   (ALPHA_SHT+3)

static UINT32 _u4IsTableExist = (UINT32)FALSE;

static unsigned long _inverse_table_value[TABLE_SIZE+1];
static unsigned long _inverse_table_slope[TABLE_SIZE+1];
static unsigned long _inverse_table_shift[TABLE_SIZE+1];
//static double _alpha_error_distr[TABLE_SIZE+1];

//static INT32 _i4MemCompFlag = 0;
//#define VIRTUAL(addr)               (addr)
/*
static const INT32 _i4sAlphaInvTbl[] =
{
       0,    2048,    2048,    1365,    2048,    1638,    1365,    1170,
    2048,    1820,    1638,    1489,    1365,    1260,    1170,    1092,
    2048,    1928,    1820,    1725,    1638,    1560,    1489,    1425,
    1365,    1311,    1260,    1214,    1170,    1130,    1092,    1057,
    2048,    1986,    1928,    1872,    1820,    1771,    1725,    1680,
    1638,    1598,    1560,    1524,    1489,    1456,    1425,    1394,
    1365,    1337,    1311,    1285,    1260,    1237,    1214,    1192,
    1170,    1150,    1130,    1111,    1092,    1074,    1057,    1040,
    2048,    2016,    1986,    1956,    1928,    1900,    1872,    1846,
    1820,    1796,    1771,    1748,    1725,    1702,    1680,    1659,
    1638,    1618,    1598,    1579,    1560,    1542,    1524,    1507,
    1489,    1473,    1456,    1440,    1425,    1409,    1394,    1380,
    1365,    1351,    1337,    1324,    1311,    1298,    1285,    1273,
    1260,    1248,    1237,    1225,    1214,    1202,    1192,    1181,
    1170,    1160,    1150,    1140,    1130,    1120,    1111,    1101,
    1092,    1083,    1074,    1066,    1057,    1049,    1040,    1032,
    2048,    2032,    2016,    2001,    1986,    1971,    1956,    1942,
    1928,    1913,    1900,    1886,    1872,    1859,    1846,    1833,
    1820,    1808,    1796,    1783,    1771,    1759,    1748,    1736,
    1725,    1713,    1702,    1691,    1680,    1670,    1659,    1649,
    1638,    1628,    1618,    1608,    1598,    1589,    1579,    1570,
    1560,    1551,    1542,    1533,    1524,    1515,    1507,    1498,
    1489,    1481,    1473,    1464,    1456,    1448,    1440,    1432,
    1425,    1417,    1409,    1402,    1394,    1387,    1380,    1372,
    1365,    1358,    1351,    1344,    1337,    1331,    1324,    1317,
    1311,    1304,    1298,    1291,    1285,    1279,    1273,    1266,
    1260,    1254,    1248,    1242,    1237,    1231,    1225,    1219,
    1214,    1208,    1202,    1197,    1192,    1186,    1181,    1176,
    1170,    1165,    1160,    1155,    1150,    1145,    1140,    1135,
    1130,    1125,    1120,    1116,    1111,    1106,    1101,    1097,
    1092,    1088,    1083,    1079,    1074,    1070,    1066,    1061,
    1057,    1053,    1049,    1044,    1040,    1036,    1032,    1028
};
*/
/** division reference table (shift)
 *  for alpha composition use
 *
 */
 /*
static const INT32 _i4sAlphaInvShtTbl[] =
{
    0,    0,    1,    1,    2,    2,    2,    2,
    3,    3,    3,    3,    3,    3,    3,    3,
    4,    4,    4,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,
    5,    5,    5,    5,    5,    5,    5,    5,
    5,    5,    5,    5,    5,    5,    5,    5,
    5,    5,    5,    5,    5,    5,    5,    5,
    5,    5,    5,    5,    5,    5,    5,    5,
    6,    6,    6,    6,    6,    6,    6,    6,
    6,    6,    6,    6,    6,    6,    6,    6,
    6,    6,    6,    6,    6,    6,    6,    6,
    6,    6,    6,    6,    6,    6,    6,    6,
    6,    6,    6,    6,    6,    6,    6,    6,
    6,    6,    6,    6,    6,    6,    6,    6,
    6,    6,    6,    6,    6,    6,    6,    6,
    6,    6,    6,    6,    6,    6,    6,    6,
    7,    7,    7,    7,    7,    7,    7,    7,
    7,    7,    7,    7,    7,    7,    7,    7,
    7,    7,    7,    7,    7,    7,    7,    7,
    7,    7,    7,    7,    7,    7,    7,    7,
    7,    7,    7,    7,    7,    7,    7,    7,
    7,    7,    7,    7,    7,    7,    7,    7,
    7,    7,    7,    7,    7,    7,    7,    7,
    7,    7,    7,    7,    7,    7,    7,    7,
    7,    7,    7,    7,    7,    7,    7,    7,
    7,    7,    7,    7,    7,    7,    7,    7,
    7,    7,    7,    7,    7,    7,    7,    7,
    7,    7,    7,    7,    7,    7,    7,    7,
    7,    7,    7,    7,    7,    7,    7,    7,
    7,    7,    7,    7,    7,    7,    7,    7,
    7,    7,    7,    7,    7,    7,    7,    7,
    7,    7,    7,    7,    7,    7,    7,    7
};*/
//-------------------------------------------------------------------------
/** round_u_long
 *  rounding
 *
 */
//-------------------------------------------------------------------------
unsigned long round_u_long(unsigned long i, int sht)
{
    unsigned long result;
    result = i;
    result = result >> (sht-1);
    if ((result % 2) == 1)
    {
        result = (result/2) + 1;
    }
    else
    {
        result = (result/2);
    }

    return result;
}

UINT64 round_u_long_long(UINT64 i, int sht)
{
    UINT64 result;
    result = i;
    result = result >> (sht - 1);
    if ((result % 2) == 1)
    {
        result = (result/2) + 1;
    }
    else
    {
        result = (result/2);
    }

    return result;
}

/*lint -save -e704 -e613 */
INT32 GFX_SwAlphaCompositionLoop(const GFX_ACLM_DATA_T *prsData)
{
    int i, h, w, m;
    int op_code;
    int pass1_sht_num = 8;
    int pass2_sht_num = 16;
    int height, width;
    int src_p_zero = 0;
    int dst_p_zero = 0;
    int color_mode;
    int S_D_SCAN_SHT;
    int PASS3_SHT_NUM_REAL;
    int PASS3_ROUND_LEVEL;
    int ALPHA_SHT_LEVEL;

    unsigned long    tmp1, tmp3, tmp4, tmp5, tmp6;
    unsigned long    pixel_s[4], pixel_d[4];
    unsigned long    csn_pass1[3], cdn_pass1[3];
    unsigned long    As, Ad;
    unsigned long    Ar = 65;    //setting range in 0~256
    unsigned long    Fs = 0, Fd = 0;
    unsigned long    F_1 = 65536;
    unsigned long    A_1 = 256;
    unsigned long    An_long;
    unsigned long    An_long_index;

    unsigned long    pixel_res[4];
    unsigned long    MAX_ALPHA_VALUE, MAX_SRC_ALPHA_VALUE;
    unsigned long    An_step;
    unsigned long    CLAMP_LEVEL;

    unsigned long long  cdn_pass2[3];
    unsigned long long  cdn_pass3[3];
    unsigned long long  An;
    unsigned long long  tmp2, An_approxi, tmp_cdn_pass2[3];

    UINT32 u4SrcPitch, u4DstPitch;
    UINT32 u4SrcCM, u4DstCM;
    UINT32 u4RectSrc, u4RectColor;
    INT32 ai4SrcColor[COMP_NUM];
    INT32 ai4DstColor[COMP_NUM];
    INT32 ai4RectColor[COMP_NUM];
    UINT8 *pu1SrcBase = NULL,*pu1DstBase = NULL;
    UINT8 *pu1SrcLine, *pu1DstLine;
    UINT8 *pu1Read, *pu1Write, *pu1Dst;
    UINT32 *pu4Color;
    UINT32 u4GlobalAlpha;

#if 0//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8561)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    UINT32 u4PreColorize, u4ColorRepEn, u4JavaXorColor;
    unsigned long    pixel_p[4];

#endif
    pu1SrcBase  = prsData->pu1SrcBase;
    u4SrcPitch  = prsData->u4SrcPitch;
    pu1DstBase  = prsData->pu1DstBase;
    u4DstPitch  = prsData->u4DstPitch;
    u4SrcCM     = prsData->u4SrcCM;
    u4DstCM     = prsData->u4DstCM;
    width       = prsData->u4Width;
    height      = prsData->u4Height;
    Ar          = prsData->u4AlComAr;
    op_code     = prsData->u4AlComOpCode;
    u4RectSrc   = prsData->u4AlComRectSrc;
    u4RectColor = prsData->u4RectColor;
    u4GlobalAlpha   = prsData->u4GlobalAlpha;
#if 0//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8561)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    u4PreColorize   = prsData->u4PreColorize;
    u4ColorRepEn    = prsData->u4ColorRepEn;
    u4JavaXorColor  = prsData->u4JavaXorColor;
#endif

    pu1Read    = pu1SrcBase;
    pu1SrcLine = pu1SrcBase;
    pu1Write   = pu1DstBase;
    pu1DstLine = pu1DstBase;

    // --------------- linear approximation start ---------------
    if (_u4IsTableExist == (UINT32)FALSE)
    {
        _inverse_table_value[0] = 0;
        _inverse_table_slope[0] = 0;
        _inverse_table_shift[0] = PASS3_SHT_NUM_1;
      //  _alpha_error_distr[0] = 0.0;

        for (m = 1; m < (TABLE_SIZE+1); m++)
        {
           // _alpha_error_distr[m] = 0.0;
            i = 0;
            tmp1 = F_1 / m;
            tmp4 = F_1;
            while (tmp1 <= F_1/2)
            {
                tmp4 = tmp4 << 1;
                tmp1 = tmp4 / m;
                i++;
            }

        #if 0
            //=============================================
            tmp4 = (F_1<<(i+1))/m;
            _inverse_table_value[m] = round_u_long(tmp4, 1);
            _inverse_table_shift[m] = i+PASS3_SHT_NUM_1;
            tmp5 = (F_1<<(i+1))/m;
            tmp6 = (F_1<<(i+1))/(m+1);
            tmp4 = (tmp5-tmp6)>>4;
            _inverse_table_slope[m] =  round_u_long(tmp4, 1);
            //=============================================
        #else
            //=============================================
            tmp4 = (F_1 << (i+2)) / m;    //one more bit
            _inverse_table_value[m] = round_u_long(tmp4, 1);
            _inverse_table_shift[m] = i + PASS3_SHT_NUM_1;
            tmp5 = (F_1 << (i+2)) / m;
            tmp6 = (F_1 << (i+2)) / (m+1);
            tmp4 = (tmp5 - tmp6) >> 4;
            _inverse_table_slope[m] = round_u_long(tmp4, 1);
            //=============================================
        #endif
        }

        _u4IsTableExist = (UINT32)TRUE;
    }
    // --------------- linear approximation end ---------------
    if (u4SrcCM == CM_ARGB8888_DIRECT32)
    {
        MAX_SRC_ALPHA_VALUE = 255;
    }
    else if(u4SrcCM == CM_ARGB4444_DIRECT16)
    {
        MAX_SRC_ALPHA_VALUE = 255;
    }
    else if(u4SrcCM == CM_RGB565_DIRECT16)
    {
        MAX_SRC_ALPHA_VALUE = 255;
    }
    else
    {
        MAX_SRC_ALPHA_VALUE = 255;//127;
    }

    if (u4DstCM == CM_ARGB8888_DIRECT32)
    {
        color_mode = 0;
    }
    else if(u4DstCM == CM_ARGB4444_DIRECT16)
    {
        color_mode = 1;
    }
    else if(u4DstCM == CM_RGB565_DIRECT16)
    {
        color_mode = 2;
    }
    else
    {
        color_mode = 3;
    }

    if (color_mode == 0)     //32bpp
    {
        S_D_SCAN_SHT = 0;
        MAX_ALPHA_VALUE = 255;
        PASS3_SHT_NUM_REAL = PASS3_SHT_NUM_1+12;
        PASS3_ROUND_LEVEL = 0;
        ALPHA_SHT_LEVEL = ALPHA_SHT;
        CLAMP_LEVEL = 255;
    }
    else if(color_mode == 1)
    {
        S_D_SCAN_SHT = 4;
        MAX_ALPHA_VALUE = 240;
        PASS3_SHT_NUM_REAL = PASS3_SHT_NUM_1+16;
        PASS3_ROUND_LEVEL = 4;
        ALPHA_SHT_LEVEL = ALPHA_SHT_16BPP;
        CLAMP_LEVEL = 15;
    }
    else if (color_mode == 2)     // 565(6666) : 6bit
    {
        S_D_SCAN_SHT = 0;
        MAX_ALPHA_VALUE = 255;
        PASS3_SHT_NUM_REAL = PASS3_SHT_NUM_1+12;
        PASS3_ROUND_LEVEL = 0;
        ALPHA_SHT_LEVEL = ALPHA_SHT;
        CLAMP_LEVEL = 255;
    }
    else     //  1555 : 5bit
    {
        S_D_SCAN_SHT = 0;//3;
        MAX_ALPHA_VALUE = 255;
        PASS3_SHT_NUM_REAL = PASS3_SHT_NUM_1+12;//PASS3_SHT_NUM_1+15;
        PASS3_ROUND_LEVEL = 0;//3;
        ALPHA_SHT_LEVEL = ALPHA_SHT;//ALPHA_SHT_16BPP_1555;
        CLAMP_LEVEL = 255;//31;
    }

    if (1 == g_u4I2DAlcom)
    {
        MAX_SRC_ALPHA_VALUE = 255; /* Just for I2D + Alcom + XXXX(Mirror/Flip/Stretch) */
        //if (color_mode == 3)MAX_ALPHA_VALUE = 0;
    }

    if (Ar == 255)
    {
        Ar = 256;
    }

    if (u4RectSrc)  // use Rect_Color[31:0] as Src pixel values
    {
        pu4Color = &(u4RectColor);
        GFX_SwGetColorComponent_5381((UINT8 **)&pu4Color, u4DstCM, ai4RectColor);
    }

    for (h = 0; h < height; h++)
    {
        for (w = 0; w < width; w++)
        {
            pu1Dst = pu1Write;

            if (u4RectSrc)
            {
                // use Rect Color as Src pixel values
                pixel_s[3] = (UINT32)(ai4RectColor[0]);
                pixel_s[2] = (UINT32)(ai4RectColor[1]);
                pixel_s[1] = (UINT32)(ai4RectColor[2]);
                pixel_s[0] = (UINT32)(ai4RectColor[3]);
            }
            else
            {
                // Src pixel values
                GFX_SwGetColorComponent_5381(&pu1Read, u4SrcCM, ai4SrcColor);
                pixel_s[3] = (UINT32)(ai4SrcColor[0]);
                pixel_s[2] = (UINT32)(ai4SrcColor[1]);
                pixel_s[1] = (UINT32)(ai4SrcColor[2]);
                pixel_s[0] = (UINT32)(ai4SrcColor[3]);

                if(((u4SrcCM == CM_ARGB1555_DIRECT16) && (0 == pixel_s[3]))||
                    (u4SrcCM == CM_RGB565_DIRECT16))
                {
                    pixel_s[3] = u4GlobalAlpha;
                }
            }

            // Dst pixel values
            GFX_SwGetColorComponent_5381(&pu1Dst, u4DstCM, ai4DstColor);
            pixel_d[3] = (UINT32)(ai4DstColor[0]);
            pixel_d[2] = (UINT32)(ai4DstColor[1]);
            pixel_d[1] = (UINT32)(ai4DstColor[2]);
            pixel_d[0] = (UINT32)(ai4DstColor[3]);

            if(((u4DstCM == CM_ARGB1555_DIRECT16)&& (0 == pixel_d[3]))||
                (u4DstCM == CM_RGB565_DIRECT16))
            {
                pixel_d[3] = u4GlobalAlpha;
            }

            #if 0//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8561)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)

            pixel_p[3] = u4JavaXorColor >> 24;
            pixel_p[2] = (u4JavaXorColor >> 16) & 0xff;
            pixel_p[1] = (u4JavaXorColor >>  8) & 0xff;
            pixel_p[0] = u4JavaXorColor & 0xff;

            if (u4PreColorize)
            {
                pixel_s[2] = pixel_s[2]*(pixel_p[2]+1)/256;
                pixel_s[1] = pixel_s[1]*(pixel_p[1]+1)/256;
                pixel_s[0] = pixel_s[0]*(pixel_p[0]+1)/256;
            }
            if (u4ColorRepEn)
            {
                pixel_s[3] = pixel_p[3];//
            }
            else
            {
                pixel_s[3] = pixel_s[3];//
            }
            #endif

            if (pixel_s[3] >= MAX_SRC_ALPHA_VALUE)
            {
                pixel_s[3] = A_1;    //only alpha pushed to 1.0
            }

            if (pixel_d[3] >= MAX_ALPHA_VALUE)
            {
                pixel_d[3] = A_1;    //only alpha pushed to 1.0
            }

            // Common operation
            As = pixel_s[3];
            Ad = pixel_d[3];

            // pass 0, As*Ar
            As = As * Ar;  // 17-bit data
            Ad = Ad * A_1; //9x9, 17-bit data, no loss

            if (As == 0)
            {
                src_p_zero = 1;
            }
            else
            {
                src_p_zero = 0;    //src zero flag
            }

            if (Ad == 0)
            {
                dst_p_zero = 1;
            }
            else
            {
                dst_p_zero = 0; //dst zero flag
            }

            // pass 1, color * Fs, Fd
            ////////////////////////////////
            ///CLEAR
            if (op_code == 0)
            {
                Fs = 0;
                Fd = 0;
            } //end of op_code

            ////////////////////////////////
            ///DST_IN
            if (op_code == 1)
            {
                Fs = 0;
                Fd = As;
            } //end of op_code

            ////////////////////////////////
            ///DST_OUT
            if (op_code == 2)
            {
                Fs = 0;
                Fd = F_1 - As;
            } //end of op_code

            ////////////////////////////////
            ///DST_OVER
            if (op_code == 3)
            {
                Fs = F_1 - Ad;
                Fd = F_1;
            } //end of op_code

            ////////////////////////////////
            ///SRC
            if (op_code == 4)
            {
                Fs = F_1;
                Fd = 0;
            } //end of op_code

            ////////////////////////////////
            ///SRC_IN
            if (op_code == 5)
            {
                Fs = Ad;
                Fd = 0;
            } //end of op_code

            ////////////////////////////////
            ///SRC_OUT
            if (op_code == 6)
            {
                Fs = F_1 - Ad;
                Fd = 0;
            } //end of op_code

            ////////////////////////////////
            ///SRC_OVER
            if (op_code == 7)
            {
                Fs = F_1;
                Fd = F_1 - As;
            } //end of op_code

            ////////////////////////////////
            ///DST
            if (op_code == 8)
            {
                Fs = 0;
                Fd = F_1;
            } //end of op_code

            ////////////////////////////////
            ///SRC_ATOP
            if (op_code == 9)
            {
                Fs = Ad;
                Fd = F_1 - As;
            } //end of op_code

            ////////////////////////////////
            ///DST_ATOP
            if (op_code == 10)
            {
                Fs = F_1 - Ad;
                Fd = As;
            } //end of op_code

            ////////////////////////////////
            ///XOR
            if (op_code == 11)
            {
                Fs = F_1 - Ad;
                Fd = F_1 - As;
            } //end of op_code

            ///////////////////////////////
            ///NONE
            if (op_code == 12)
            {
                Fs = As;
                Fd = F_1 - As;
            } //end of op_code

            ///////////////////////////////
            ///ADD
            if (op_code == 13)
            {
                Fs = F_1;
                Fd = F_1;
            } //end of op_code
            
            if (Fs == 0)
            {
                src_p_zero = 0x1 | src_p_zero;    //src zero flag
            }

            if (Fd == 0)
            {
                dst_p_zero = 0x1 | dst_p_zero; //dst zero flag
            }

            for (i = 0; i < 3; i++)
            {
                if (dst_p_zero)
                {
                    tmp1 = F_1 * pixel_s[i];    //17x9, 25-bit data
                }
                else
                {
                    tmp1 = Fs * pixel_s[i];    //17x9, 25-bit data
                }
                csn_pass1[i] = round_u_long(tmp1, pass1_sht_num);    //25->17bit

                if (src_p_zero)
                {
                    tmp1 = F_1 * pixel_d[i];    //17x9, 25-bit data
                }
                else
                {
                    tmp1 = Fd * pixel_d[i];    //17x9, 25-bit data
                }
                cdn_pass1[i] = round_u_long(tmp1, pass1_sht_num);    //25->17bit

            }

            // pass 2, color tmp * alpha tmp
            An = (unsigned long long) Fs*As + (unsigned long long) Fd*Ad;    //17x17, 33+1 bit
            An = An >> pass2_sht_num;    //34(33) bit->17bit

            for (i = 0; i < 3; i++)
            {
                if (dst_p_zero)
                {
                    tmp1 = F_1;
                }
                else
                {
                    tmp1 = As;
                }

                if (src_p_zero)
                {
                    tmp3 = F_1;
                }
                else
                {
                    tmp3 = Ad;
                }
                tmp_cdn_pass2[i] = (unsigned long long) tmp1*csn_pass1[i] + (unsigned long long) tmp3*cdn_pass1[i]; //17x17, 33+1 bit

                cdn_pass2[i] = round_u_long_long(tmp_cdn_pass2[i], pass2_sht_num); //34->17bit
            }

            // pass 3
            An_long = An;    //17-bit
            An_step = An_long & 0xf;    //last 4-bit
            An_long_index = An_long >> DIV_SHT_1;

            if (An_long_index >= TABLE_SIZE)
            {
                An_long_index = TABLE_SIZE;
            }

            for (i = 0; i < 3; i++)
            {
                if (src_p_zero || dst_p_zero)
                {
                    if (An_long_index == 0)
                    {
                        tmp2 = 0; //17x17, 33 bit
                    }
                    else
                    {
                        tmp2 = F_1 * cdn_pass2[i]; //17x17, 33 bit
                    }
                }
                else
                {
                    An_approxi = (_inverse_table_value[An_long_index]-_inverse_table_slope[An_long_index]*An_step);
                    An_approxi = An_approxi >> 1;
                    tmp2 = An_approxi * cdn_pass2[i]; //17x17, 33 bit
                }

                if (src_p_zero || dst_p_zero)
                {
                    cdn_pass3[i] = tmp2 >> PASS3_SHT_NUM_REAL;
                }
                else
                {
                    cdn_pass3[i] = round_u_long_long(tmp2, _inverse_table_shift[An_long_index]+PASS3_ROUND_LEVEL); //33->9bit
                }
            }

            An_long = round_u_long(An_long, ALPHA_SHT_LEVEL);
            if (An_long >= CLAMP_LEVEL)
            {
                An_long = CLAMP_LEVEL;
            }

            pixel_res[3] = An_long;

            for (i = 0; i < 3; i++)
            {
                pixel_res[i] = cdn_pass3[i];
                if (pixel_res[i] >= CLAMP_LEVEL)
                {
                    pixel_res[i] = CLAMP_LEVEL;
                }
            }

            if (pixel_res[3] == 0)
            {
                pixel_res[2] = 0;
                pixel_res[1] = 0;
                pixel_res[0] = 0;
            }

            // Dst pixel values
            ai4DstColor[0] = (INT32)(pixel_res[3] << S_D_SCAN_SHT);
            ai4DstColor[1] = (INT32)(pixel_res[2] << S_D_SCAN_SHT);
            ai4DstColor[2] = (INT32)(pixel_res[1] << S_D_SCAN_SHT);
            ai4DstColor[3] = (INT32)(pixel_res[0] << S_D_SCAN_SHT);

            GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4DstColor);

        } // ~for

        pu1SrcLine += u4SrcPitch;
        pu1DstLine += u4DstPitch;
        pu1Read     = pu1SrcLine;
        pu1Write    = pu1DstLine;

    } //~for

    g_u4I2DAlcom = 0; /* Just for I2D + Alcom + XXXX(Mirror/Flip/Stretch) */

    return (INT32)E_GFX_OK;
}

#if 0//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
INT32 GFX_SwAlphaCompositionLoop2Src(const GFX_ACLM_DATA_T *prsData)
{
    int i, h, w, m;
    int op_code;
    int pass1_sht_num = 8;
    int pass2_sht_num = 16;
    int height, width;
    int src_p_zero = 0;
    int dst_p_zero = 0;
    int color_mode;
    int S_D_SCAN_SHT;
    int PASS3_SHT_NUM_REAL;
    int PASS3_ROUND_LEVEL;
    int ALPHA_SHT_LEVEL;

    unsigned long    tmp1, tmp3, tmp4, tmp5, tmp6;
    unsigned long    pixel_s[4], pixel_d[4];
    unsigned long    csn_pass1[3], cdn_pass1[3];
    unsigned long    As, Ad;
    unsigned long    Ar = 65;    //setting range in 0~256
    unsigned long    Fs = 0, Fd = 0;
    unsigned long    F_1 = 65536;
    unsigned long    A_1 = 256;
    unsigned long    An_long;
    unsigned long    An_long_index;

    unsigned long    pixel_res[4];
    unsigned long    MAX_ALPHA_VALUE, MAX_SRC_ALPHA_VALUE;
    unsigned long    An_step;
    unsigned long    CLAMP_LEVEL;

    unsigned long long  cdn_pass2[3];
    unsigned long long  cdn_pass3[3];
    unsigned long long  An;
    unsigned long long  tmp2, An_approxi, tmp_cdn_pass2[3];

    UINT32 u4SrcPitch, u4DstPitch, u4Src2Pitch;
    UINT32 u4SrcCM, u4DstCM;
    UINT32 u4RectSrc, u4RectColor;
    INT32 ai4SrcColor[COMP_NUM];
    INT32 ai4DstColor[COMP_NUM];
    INT32 ai4RectColor[COMP_NUM];
    UINT8 *pu1SrcBase = NULL,*pu1DstBase = NULL,*pu1Src2Base = NULL;
    UINT8 *pu1SrcLine, *pu1DstLine;
    UINT8 *pu1Src2Line;
    UINT8 *pu1Read, *pu1Write, *pu1Dst;
    UINT8 *pu1Read2;
    UINT32 *pu4Color;
    UINT32 u4GlobalAlpha;

#if (CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8561)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    UINT32 u4PreColorize, u4ColorRepEn, u4JavaXorColor;
    unsigned long    pixel_p[4];
#endif

    pu1SrcBase  = prsData->pu1SrcBase;
    u4SrcPitch  = prsData->u4SrcPitch;
    pu1Src2Base = prsData->pu12ndSrcBase;
    u4Src2Pitch = prsData->u42ndSrcPitch;
    pu1DstBase  = prsData->pu1DstBase;
    u4DstPitch  = prsData->u4DstPitch;
    u4SrcCM     = prsData->u4SrcCM;
    u4DstCM     = prsData->u4DstCM;
    width       = prsData->u4Width;
    height      = prsData->u4Height;
    Ar          = prsData->u4AlComAr;
    op_code     = prsData->u4AlComOpCode;
    u4RectSrc   = prsData->u4AlComRectSrc;
    u4RectColor = prsData->u4RectColor;
    u4GlobalAlpha   = prsData->u4GlobalAlpha;

#if (CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8561)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    u4PreColorize   = prsData->u4PreColorize;
    u4ColorRepEn    = prsData->u4ColorRepEn;
    u4JavaXorColor  = prsData->u4JavaXorColor;
#endif

    pu1Read    = pu1SrcBase;
    pu1SrcLine = pu1SrcBase;
    pu1Read2   = pu1Src2Base;
    pu1Src2Line = pu1Src2Base;
    pu1Write   = pu1DstBase;
    pu1DstLine = pu1DstBase;

    // --------------- linear approximation start ---------------
    if (_u4IsTableExist == (UINT32)FALSE)
    {
        _inverse_table_value[0] = 0;
        _inverse_table_slope[0] = 0;
        _inverse_table_shift[0] = PASS3_SHT_NUM_1;
        //  _alpha_error_distr[0] = 0.0;

        for (m = 1; m < (TABLE_SIZE+1); m++)
        {
            // _alpha_error_distr[m] = 0.0;
            i = 0;
            tmp1 = F_1 / m;
            tmp4 = F_1;
            while (tmp1 <= F_1/2)
            {
                tmp4 = tmp4 << 1;
                tmp1 = tmp4 / m;
                i++;
            }

#if 0
            //=============================================
            tmp4 = (F_1<<(i+1))/m;
            _inverse_table_value[m] = round_u_long(tmp4, 1);
            _inverse_table_shift[m] = i+PASS3_SHT_NUM_1;
            tmp5 = (F_1<<(i+1))/m;
            tmp6 = (F_1<<(i+1))/(m+1);
            tmp4 = (tmp5-tmp6)>>4;
            _inverse_table_slope[m] =  round_u_long(tmp4, 1);
            //=============================================
#else
            //=============================================
            tmp4 = (F_1 << (i+2)) / m;    //one more bit
            _inverse_table_value[m] = round_u_long(tmp4, 1);
            _inverse_table_shift[m] = i + PASS3_SHT_NUM_1;
            tmp5 = (F_1 << (i+2)) / m;
            tmp6 = (F_1 << (i+2)) / (m+1);
            tmp4 = (tmp5 - tmp6) >> 4;
            _inverse_table_slope[m] = round_u_long(tmp4, 1);
            //=============================================
#endif
        }

        _u4IsTableExist = (UINT32)TRUE;
    }
    // --------------- linear approximation end ---------------

    // --------------- linear approximation end ---------------
    if(u4SrcCM == CM_ARGB4444_DIRECT16)
    {
        MAX_SRC_ALPHA_VALUE = 240;
    }
    else
    {
        MAX_SRC_ALPHA_VALUE = 255;//127;
    }

    if (u4DstCM == CM_ARGB8888_DIRECT32)
    {
        color_mode = 0;
    }
    else
    {
        color_mode = 1;
    }

    if (color_mode == 0)     //32bpp
    {
        S_D_SCAN_SHT = 0;
        MAX_ALPHA_VALUE = 255;
        PASS3_SHT_NUM_REAL = PASS3_SHT_NUM_1+12;
        PASS3_ROUND_LEVEL = 0;
        ALPHA_SHT_LEVEL = ALPHA_SHT;
        CLAMP_LEVEL = 255;
    }

    if (color_mode == 1)     //16bpp
    {
        S_D_SCAN_SHT = 4;
        MAX_ALPHA_VALUE = 240;
        PASS3_SHT_NUM_REAL = PASS3_SHT_NUM_1+16;
        PASS3_ROUND_LEVEL = 4;
        ALPHA_SHT_LEVEL = ALPHA_SHT_16BPP;
        CLAMP_LEVEL = 15;
    }

    if (Ar == 255)
    {
        Ar = 256;
    }

    if (u4RectSrc)  // use Rect_Color[31:0] as Src pixel values
    {
        pu4Color = &(u4RectColor);
        GFX_SwGetColorComponent_5381((UINT8 **)&pu4Color, u4DstCM, ai4RectColor);
    }

    for (h = 0; h < height; h++)
    {
        for (w = 0; w < width; w++)
        {
            pu1Dst  = pu1Write;

            if (u4RectSrc)
            {
                // use Rect Color as Src pixel values
                pixel_s[3] = (UINT32)(ai4RectColor[0]);
                pixel_s[2] = (UINT32)(ai4RectColor[1]);
                pixel_s[1] = (UINT32)(ai4RectColor[2]);
                pixel_s[0] = (UINT32)(ai4RectColor[3]);
                // use Rect Color as Src2 pixel values
                pixel_d[3] = (UINT32)(ai4RectColor[0]);
                pixel_d[2] = (UINT32)(ai4RectColor[1]);
                pixel_d[1] = (UINT32)(ai4RectColor[2]);
                pixel_d[0] = (UINT32)(ai4RectColor[3]);
            }
            else
            {
                // Src pixel values
                GFX_SwGetColorComponent_5381(&pu1Read, u4SrcCM, ai4SrcColor);
                pixel_s[3] = (UINT32)(ai4SrcColor[0]);
                pixel_s[2] = (UINT32)(ai4SrcColor[1]);
                pixel_s[1] = (UINT32)(ai4SrcColor[2]);
                pixel_s[0] = (UINT32)(ai4SrcColor[3]);

                // src2 pixel values
                GFX_SwGetColorComponent_5381(&pu1Read2, u4SrcCM, ai4DstColor);
                pixel_d[3] = (UINT32)(ai4DstColor[0]);
                pixel_d[2] = (UINT32)(ai4DstColor[1]);
                pixel_d[1] = (UINT32)(ai4DstColor[2]);
                pixel_d[0] = (UINT32)(ai4DstColor[3]);
            }

            if(((u4DstCM == CM_ARGB1555_DIRECT16)&& (0 == pixel_d[3]))||
                (u4DstCM == CM_RGB565_DIRECT16))
            {
                pixel_d[3] = u4GlobalAlpha;
            }

        #if (CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8561)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)

            pixel_p[3] = u4JavaXorColor >> 24;
            pixel_p[2] = (u4JavaXorColor >> 16) & 0xff;
            pixel_p[1] = (u4JavaXorColor >>  8) & 0xff;
            pixel_p[0] = u4JavaXorColor & 0xff;

            if (u4PreColorize)
            {
                pixel_s[2] = pixel_s[2]*(pixel_p[2]+1)/256;
                pixel_s[1] = pixel_s[1]*(pixel_p[1]+1)/256;
                pixel_s[0] = pixel_s[0]*(pixel_p[0]+1)/256;
            }
            if (u4ColorRepEn)
            {
                pixel_s[3] = pixel_p[3];//
            }
            else
            {
                pixel_s[3] = pixel_s[3];//
            }
        #endif

            if (pixel_s[3] >= MAX_SRC_ALPHA_VALUE)
            {
                pixel_s[3] = A_1;    //only alpha pushed to 1.0
            }

            if (pixel_d[3] >= MAX_ALPHA_VALUE)
            {
                pixel_d[3] = A_1;    //only alpha pushed to 1.0
            }

            // Common operation
            As = pixel_s[3];
            Ad = pixel_d[3];

            // pass 0, As*Ar
            As = As * Ar;
            Ad = Ad * A_1; //9x9, 17-bit data, no loss

            if (As == 0)
            {
                src_p_zero = 1;
            }
            else
            {
                src_p_zero = 0;    //src zero flag
            }

            if (Ad == 0)
            {
                dst_p_zero = 1;
            }
            else
            {
                dst_p_zero = 0; //dst zero flag
            }

            // pass 1, color * Fs, Fd
            ////////////////////////////////
            ///CLEAR
            if (op_code == 0)
            {
                Fs = 0;
                Fd = 0;
            } //end of op_code

            ////////////////////////////////
            ///DST_IN
            if (op_code == 1)
            {
                Fs = 0;
                Fd = As;
            } //end of op_code

            ////////////////////////////////
            ///DST_OUT
            if (op_code == 2)
            {
                Fs = 0;
                Fd = F_1 - As;
            } //end of op_code

            ////////////////////////////////
            ///DST_OVER
            if (op_code == 3)
            {
                Fs = F_1 - Ad;
                Fd = F_1;
            } //end of op_code

            ////////////////////////////////
            ///SRC
            if (op_code == 4)
            {
                Fs = F_1;
                Fd = 0;
            } //end of op_code

            ////////////////////////////////
            ///SRC_IN
            if (op_code == 5)
            {
                Fs = Ad;
                Fd = 0;
            } //end of op_code

            ////////////////////////////////
            ///SRC_OUT
            if (op_code == 6)
            {
                Fs = F_1 - Ad;
                Fd = 0;
            } //end of op_code

            ////////////////////////////////
            ///SRC_OVER
            if (op_code == 7)
            {
                Fs = F_1;
                Fd = F_1 - As;
            } //end of op_code

            ////////////////////////////////
            ///DST
            if (op_code == 8)
            {
                Fs = 0;
                Fd = F_1;
            } //end of op_code

            ////////////////////////////////
            ///SRC_ATOP
            if (op_code == 9)
            {
                Fs = Ad;
                Fd = F_1 - As;
            } //end of op_code

            ////////////////////////////////
            ///DST_ATOP
            if (op_code == 10)
            {
                Fs = F_1 - Ad;
                Fd = As;
            } //end of op_code

            ////////////////////////////////
            ///XOR
            if (op_code == 11)
            {
                Fs = F_1 - Ad;
                Fd = F_1 - As;
            } //end of op_code

            ///////////////////////////////
            ///NONE
            if (op_code == 12)
            {
                Fs = As;
                Fd = F_1 - As;
            } //end of op_code

            ///////////////////////////////
            ///ADD
            if (op_code == 13)
            {
                Fs = F_1;
                Fd = F_1;
            } //end of op_code

            if (Fs == 0)
            {
                src_p_zero = 0x1 | src_p_zero;    //src zero flag
            }

            if (Fd == 0)
            {
                dst_p_zero = 0x1 | dst_p_zero; //dst zero flag
            }

            for (i = 0; i < 3; i++)
            {
                if (dst_p_zero)
                {
                    tmp1 = F_1 * pixel_s[i];    //17x9, 25-bit data
                }
                else
                {
                    tmp1 = Fs * pixel_s[i];    //17x9, 25-bit data
                }
                csn_pass1[i] = round_u_long(tmp1, pass1_sht_num);    //25->17bit

                if (src_p_zero)
                {
                    tmp1 = F_1 * pixel_d[i];    //17x9, 25-bit data
                }
                else
                {
                    tmp1 = Fd * pixel_d[i];    //17x9, 25-bit data
                }
                cdn_pass1[i] = round_u_long(tmp1, pass1_sht_num);    //25->17bit

            }

            // pass 2, color tmp * alpha tmp
            An = (unsigned long long) Fs*As + (unsigned long long) Fd*Ad;    //17x17, 33+1 bit
            An = An >> pass2_sht_num;    //34(33) bit->17bit

            for (i = 0; i < 3; i++)
            {
                if (dst_p_zero)
                {
                    tmp1 = F_1;
                }
                else
                {
                    tmp1 = As;
                }

                if (src_p_zero)
                {
                    tmp3 = F_1;
                }
                else
                {
                    tmp3 = Ad;
                }
                tmp_cdn_pass2[i] = (unsigned long long) tmp1*csn_pass1[i] + (unsigned long long) tmp3*cdn_pass1[i]; //17x17, 33+1 bit

                cdn_pass2[i] = round_u_long_long(tmp_cdn_pass2[i], pass2_sht_num); //34->17bit
            }

            // pass 3
            An_long = An;    //17-bit
            An_step = An_long & 0xf;    //last 4-bit
            An_long_index = An_long >> DIV_SHT_1;

            if (An_long_index >= TABLE_SIZE)
            {
                An_long_index = TABLE_SIZE;
            }

            for (i = 0; i < 3; i++)
            {
                if (src_p_zero || dst_p_zero)
                {
                    if (An_long_index == 0)
                    {
                        tmp2 = 0; //17x17, 33 bit
                    }
                    else
                    {
                        tmp2 = F_1 * cdn_pass2[i]; //17x17, 33 bit
                    }
                }
                else
                {
                    An_approxi = (_inverse_table_value[An_long_index]-_inverse_table_slope[An_long_index]*An_step);
                    An_approxi = An_approxi >> 1;
                    tmp2 = An_approxi * cdn_pass2[i]; //17x17, 33 bit
                }

                if (src_p_zero || dst_p_zero)
                {
                    cdn_pass3[i] = tmp2 >> PASS3_SHT_NUM_REAL;
                }
                else
                {
                    cdn_pass3[i] = round_u_long_long(tmp2, _inverse_table_shift[An_long_index]+PASS3_ROUND_LEVEL); //33->9bit
                }
            }

            An_long = round_u_long(An_long, ALPHA_SHT_LEVEL);
            if (An_long >= CLAMP_LEVEL)
            {
                An_long = CLAMP_LEVEL;
            }

            pixel_res[3] = An_long;

            for (i = 0; i < 3; i++)
            {
                pixel_res[i] = cdn_pass3[i];
                if (pixel_res[i] >= CLAMP_LEVEL)
                {
                    pixel_res[i] = CLAMP_LEVEL;
                }
            }

            if (pixel_res[3] == 0)
            {
                pixel_res[2] = 0;
                pixel_res[1] = 0;
                pixel_res[0] = 0;
            }

            // Dst pixel values
            ai4DstColor[0] = (INT32)(pixel_res[3] << S_D_SCAN_SHT);
            ai4DstColor[1] = (INT32)(pixel_res[2] << S_D_SCAN_SHT);
            ai4DstColor[2] = (INT32)(pixel_res[1] << S_D_SCAN_SHT);
            ai4DstColor[3] = (INT32)(pixel_res[0] << S_D_SCAN_SHT);

            GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4DstColor);

        } // ~for

        pu1SrcLine  += u4SrcPitch;
        pu1Src2Line += u4Src2Pitch;
        pu1DstLine  += u4DstPitch;
        pu1Read      = pu1SrcLine;
        pu1Read2     = pu1Src2Line;
        pu1Write     = pu1DstLine;

    } //~for

    return (INT32)E_GFX_OK;
}
#endif

INT32 GFX_SwPremultipliedCvnBitblt(const GFX_ACLM_DATA_T *prsData);
#if 1
static INT32 _GfxSwAlphaCompositionLoop(const MI_DIF_FIELD_T *prsGfxReg)
{
    GFX_ACLM_DATA_T rData;
    UINT32 u4Temp = 0;

    // get source & destination info
    u4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_SRC_BSAD);
    rData.pu1SrcBase = (UINT8 *)u4Temp;

    rData.u4SrcPitch = prsGfxReg->fg_SRC_PITCH << 4;    // 128bit aligned
    u4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_DST_BSAD);
    rData.pu1DstBase = (UINT8 *)u4Temp;

    rData.u4DstPitch = prsGfxReg->fg_OSD_WIDTH << 4;    // 128bit aligned
    rData.u4SrcCM    = prsGfxReg->fg_SRC_CM;
    rData.u4DstCM    = prsGfxReg->fg_CM;
    rData.u4Width    = prsGfxReg->fg_SRC_WIDTH;
    rData.u4Height   = prsGfxReg->fg_SRC_HEIGHT + 1;

    rData.u4AlComAr      = prsGfxReg->fg_ALCOM_AR;
    rData.u4AlComOpCode  = prsGfxReg->fg_ALCOM_OPCODE;
    rData.u4AlComRectSrc = prsGfxReg->fg_ALCOM_RECT_SRC;
    rData.u4RectColor    = prsGfxReg->fg_RECT_COLOR;

    rData.u4GlobalAlpha  = prsGfxReg->fg_ALPHA_VALUE;
    rData.u4AlcomNormal  = prsGfxReg->fg_ALCOM_NORMAL;

    rData.u4NonPre2PremultipliedEn = prsGfxReg->fg_NONPRE2PREMUTLI_ENA;
    rData.u4Pre2NonPremultipliedEn = prsGfxReg->fg_PRE2NONPREMUTLI_ENA;


    if ((rData.u4NonPre2PremultipliedEn) || (rData.u4Pre2NonPremultipliedEn))
    {
        return GFX_SwPremultipliedCvnBitblt(&rData);
    }
    else
    {
        return GFX_SwAlphaCompositionLoop(&rData);
    }
}

#else
INT32 GFX_SwFlashLiteBitblt(const GFX_ACLM_DATA_T *prsData);
static INT32 _GfxSwAlphaCompositionLoop(const MI_DIF_FIELD_T *prsGfxReg)
{
    GFX_ACLM_DATA_T rData;
    UINT32 u4Temp = 0;

    // get source & destination info
    u4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_SRC_BSAD);
    rData.pu1SrcBase = (UINT8 *)u4Temp;

    rData.u4SrcPitch = prsGfxReg->fg_SRC_PITCH << 4;    // 128bit aligned
    u4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_DST_BSAD);
    rData.pu1DstBase = (UINT8 *)u4Temp;

    rData.u4DstPitch = prsGfxReg->fg_OSD_WIDTH << 4;    // 128bit aligned
    rData.u4SrcCM    = prsGfxReg->fg_SRC_CM;
    rData.u4DstCM    = prsGfxReg->fg_CM;
    rData.u4Width    = prsGfxReg->fg_SRC_WIDTH;
    rData.u4Height   = prsGfxReg->fg_SRC_HEIGHT + 1;

    rData.u4AlComAr      = prsGfxReg->fg_ALCOM_AR;
    rData.u4AlComOpCode  = prsGfxReg->fg_ALCOM_OPCODE;
    rData.u4AlComRectSrc = prsGfxReg->fg_ALCOM_RECT_SRC;
    rData.u4RectColor    = prsGfxReg->fg_RECT_COLOR;

    rData.u4GlobalAlpha  = prsGfxReg->fg_ALPHA_VALUE;
    rData.u4AlcomNormal  = prsGfxReg->fg_ALCOM_NORMAL;

#if (CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8561)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)

    rData.u4SrcPremultipliedRdEn = prsGfxReg->fg_PREMULT_SRCRD_ENA;
    rData.u4DstPremultipliedRdEn = prsGfxReg->fg_PREMULT_DSTRD_ENA;
    rData.u4DstPremultipliedWrEn = prsGfxReg->fg_PREMULT_DSTWR_ENA;
    rData.u4SrcOverflowEn        = prsGfxReg->fg_SRC_OVERFLOW_ENA;

    rData.u42ndSrcEna    = prsGfxReg->fg_SRC2_BSAD_ENA;
    rData.pu12ndSrcBase  = (UINT8 *)VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_SRCCBCR_BSAD);
    rData.u42ndSrcPitch  = prsGfxReg->fg_SRCCBCR_PITCH << 4;     // 128bit aligned

    rData.u4PreColorize   = prsGfxReg->fg_PRE_COLORIZE;
    rData.u4ColorRepEn    = prsGfxReg->fg_COLORIZE_REP;
    rData.u4JavaXorColor      = prsGfxReg->fg_JAVA_XOR_COLOR;
#elif 0//(CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)
    rData.u4SrcPremultipliedRdEn = prsGfxReg->fg_PREMULT_SRCRD_ENA;
    rData.u4DstPremultipliedRdEn = prsGfxReg->fg_PREMULT_DSTRD_ENA;
    rData.u4DstPremultipliedWrEn = prsGfxReg->fg_PREMULT_DSTWR_ENA;
    rData.u4SrcOverflowEn        = prsGfxReg->fg_SRC_OVERFLOW_ENA;

    rData.u42ndSrcEna    = prsGfxReg->fg_SRC2_BSAD_ENA;
    rData.pu12ndSrcBase  = (UINT8 *)VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_SRCCBCR_BSAD);
    rData.u42ndSrcPitch  = prsGfxReg->fg_SRCCBCR_PITCH << 4;     // 128bit aligned
#else
    rData.u4NonPre2PremultipliedEn = prsGfxReg->fg_NONPRE2PREMUTLI_ENA;
    rData.u4Pre2NonPremultipliedEn = prsGfxReg->fg_PRE2NONPREMUTLI_ENA;
#endif

#if (CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
    if (rData.u4SrcPremultipliedRdEn || rData.u4DstPremultipliedRdEn || rData.u4DstPremultipliedWrEn)
    {
        return GFX_SwFlashLiteBitblt(&rData);
    }
    else if (rData.u42ndSrcEna)
    {
        return GFX_SwAlphaCompositionLoop2Src(&rData);
    }
#else
    if ((rData.u4NonPre2PremultipliedEn) || (rData.u4Pre2NonPremultipliedEn))
    {
        return GFX_SwPremultipliedCvnBitblt(&rData);
    }
#endif
    else
    {
        return GFX_SwAlphaCompositionLoop(&rData);
    }
}
#endif

/*lint -restore */


//-------------------------------------------------------------------------
/** GFX_SwSetIdx2DirComp
 *  for index to direct color bitblt use
 */
//-------------------------------------------------------------------------
void GFX_SwSetIdx2DirComp(UINT8 **ppu1DestFb, UINT32 u4DstCM,
    const UINT8 *pu1PaleBase, UINT32 u4PaleIdx)
{
    UINT32 *pu4PaleTable;
    UINT16 *pu2PaleTable;
    UINT32 u4Color;
    UINT16 u2Color;

    ASSERT(ppu1DestFb != NULL);
    ASSERT(pu1PaleBase != NULL);

    switch (u4DstCM)
    {
    case CM_RGB565_DIRECT16:
    case CM_ARGB1555_DIRECT16:
    case CM_ARGB4444_DIRECT16:
        pu2PaleTable = (UINT16 *)pu1PaleBase;
        u2Color = pu2PaleTable[u4PaleIdx];
        *((UINT16 *)*ppu1DestFb) = u2Color;
        *ppu1DestFb += 2;
        break;

    case CM_ARGB8888_DIRECT32:
        pu4PaleTable = (UINT32 *)pu1PaleBase;
        u4Color = pu4PaleTable[u4PaleIdx];
        *((UINT32 *)*ppu1DestFb) = u4Color;
        *ppu1DestFb += 4;
        break;

    default:
        return;
    }

    return;
}


//-------------------------------------------------------------------------
/** GFX_SwGetIdx2DirComp
 *  for index to direct color bitblt use
 */
//-------------------------------------------------------------------------
/*lint -save -e704 -e826 -e961 */
void GFX_SwGetIdx2DirComp(UINT8 **ppu1DestFb, UINT8 *pu1SrcByte)
{
    ASSERT(ppu1DestFb != NULL);
    ASSERT(pu1SrcByte != NULL);

    *pu1SrcByte = *((UINT8 *)*ppu1DestFb);
    *ppu1DestFb += 1;

    return;
}
/*lint -restore */


//-------------------------------------------------------------------------
/** GFX_SwGetPaleIdx
 *  get palette table index
 *  for index to direct color bitblt use
 */
//-------------------------------------------------------------------------
UINT32 GFX_SwGetPaleIdx(UINT32 u4CharCM, UINT32 u4ShiftBit, UINT8 u1SrcByte)
{
    UINT32 u4Mask;
    UINT32 u4PaleIdx;

    switch (u4CharCM)
    {
        // src_cm = 1 bpp
        case E_BMP_CM_1BIT:
            u4Mask = 0x1;
            break;

        // src_cm = 2 bpp
        case E_BMP_CM_2BIT:
            u4Mask = 0x3;
            break;

        // src_cm = 4 bpp
        case E_BMP_CM_4BIT:
            u4Mask = 0xF;
            break;

        // src_cm = 8 bpp
        case E_BMP_CM_8BIT:
            u4Mask = 0xFF;
            break;

        default:
            return 0xFFFFFFFF;
    } // ~switch

    u4PaleIdx = (((UINT32)u1SrcByte >> u4ShiftBit) & u4Mask);

    // for debug use
    switch (u4CharCM)
    {
        // src_cm = 1 bpp
        case E_BMP_CM_1BIT:
            if (u4PaleIdx >= 2)
            {
       //         LOG(5, "Error: (u4PaleIdx >= 2)\n");
            }
            break;

        // src_cm = 2 bpp
        case E_BMP_CM_2BIT:
            if (u4PaleIdx >= 4)
            {
        //        LOG(5, "Error: (u4PaleIdx >= 4)\n");
            }
            break;

        // src_cm = 4 bpp
        case E_BMP_CM_4BIT:
            if (u4PaleIdx >= 16)
            {
          //      LOG(5, "Error: (u4PaleIdx >= 16)\n");
            }
            break;

        // src_cm = 8 bpp
        case E_BMP_CM_8BIT:
            if (u4PaleIdx >= 256)
            {
          //      LOG(5, "Error: (u4PaleIdx >= 256)\n");
            }
            break;

        default:
            return 0xFFFFFFFF;
    } // ~switch

    return u4PaleIdx;
}

//-------------------------------------------------------------------------
/** _GfxSwIndexToDirectBitBlt
 *  Raster OPeration bitblt
 */
//-------------------------------------------------------------------------
/*lint -save -e613 */


//-------------------------------------------------------------------------
/** GFX_SwIndexToDirectBitBlt
 *  Index to Direct Color Bitblt
 *  CharCM = 1, 2, 4, 8 bpp
 *  DstCM  = 16, 32 bpp
 */
//-------------------------------------------------------------------------
/*lint -save -e704 -e613 */
INT32 GFX_SwIndexToDirectBitBlt(const GFX_IDX2DIR_DATA_T *prsData)
{
    UINT8 *pu1SrcBase, *pu1DstBase, *pu1PaleBase;
    UINT32/*  u4SrcPitch,*/u4DstPitch;
    UINT32 u4CharCM, u4DstCM;
    UINT32 u4Width, u4Height;
    UINT32 u4ByteAlign, u4MsbLeft;
    UINT32 i, y, g, sht;
    UINT8 *pu1DstLine= NULL;//*pu1SrcLine = NULL,
    UINT8 *pu1Read, *pu1Write;
    UINT32 u4BitNumPerByte, u4BitNumPerPix;
    UINT32 u4PixPerByte, u4PixGroup, u4Remainder;
    UINT32 u4PaleIdx;
    UINT8 u1SrcByte;

    // check parameters' error
    VERIFY(prsData != NULL);
    VERIFY(prsData->pu1SrcBase != NULL);
    VERIFY(prsData->pu1DstBase != NULL);


    pu1SrcBase = prsData->pu1SrcBase;
    //u4SrcPitch = prsData->u4SrcPitch;   // Ignore it.
    pu1DstBase = prsData->pu1DstBase;
    u4DstPitch = prsData->u4DstPitch;
    u4CharCM   = prsData->u4CharCM;
    u4DstCM    = prsData->u4DstCM;
    u4Width    = prsData->u4Width;
    u4Height   = prsData->u4Height;

    pu1PaleBase = prsData->pu1PaleBase;
    u4MsbLeft   = prsData->u4MsbLeft;
    u4ByteAlign = prsData->u4ByteAlign; // LN_ST_BYTE_AL

    u4BitNumPerByte = 8;            // bit_num / byte

    switch (prsData->u4CharCM)
    {
        // src_cm = 1 bpp
        case E_BMP_CM_1BIT:
            u4BitNumPerPix = 1;     // bit_num / pix
            break;

        // src_cm = 2 bpp
        case E_BMP_CM_2BIT:
            u4BitNumPerPix = 2;     // bit_num / pix
            break;

        // src_cm = 4 bpp
        case E_BMP_CM_4BIT:
            u4BitNumPerPix = 4;     // bit_num / pix
            break;

        // src_cm = 8 bpp
        case E_BMP_CM_8BIT:
            u4BitNumPerPix = 8;     // bit_num / pix
            break;

        default:
            return (INT32)E_GFX_INV_ARG;
    } // ~switch

    // pixels / byte = (bit_num/byte) / (bit_num/pix)
    u4PixPerByte = u4BitNumPerByte / u4BitNumPerPix;

    u4Remainder = 0;
    u4Remainder = (u4Width % u4PixPerByte);
    u4PixGroup  = (u4Width / u4PixPerByte);

    if (u4Remainder > 0)
    {
        if (u4ByteAlign != 1)
        {
     //       LOG(5, "Error: (u4Remainder > 0), but (u4ByteAlign != 1)\n");
        }
    }

    pu1Read    = pu1SrcBase;
    pu1Write   = pu1DstBase;
    pu1DstLine = pu1DstBase;

    if (u4ByteAlign == 1)
    {
        for (y = 0; y < u4Height; y++)
        {
            for (g = 0; g < u4PixGroup; g++)
            {
                GFX_SwGetIdx2DirComp(&pu1Read, &u1SrcByte);

                if (u4MsbLeft == 1)
                {
                    for (i = 0, sht = u4BitNumPerByte - u4BitNumPerPix;
                         i < u4PixPerByte;
                         i++, sht -= u4BitNumPerPix)
                    {
                        u4PaleIdx = GFX_SwGetPaleIdx(u4CharCM, sht, u1SrcByte);

                        GFX_SwSetIdx2DirComp(&pu1Write, u4DstCM, pu1PaleBase, u4PaleIdx);
                    } // ~for
                }
                else
                {
                    for (i = 0, sht = 0;
                         i < u4PixPerByte;
                         i++, sht += u4BitNumPerPix)
                    {
                        u4PaleIdx = GFX_SwGetPaleIdx(u4CharCM, sht, u1SrcByte);

                        GFX_SwSetIdx2DirComp(&pu1Write, u4DstCM, pu1PaleBase, u4PaleIdx);
                    } // ~for
                }
            } // ~for

            if (u4Remainder != 0)
            {
                
                //if (u4ByteAlign == 1) u4Remainder = u4PixPerByte;
                GFX_SwGetIdx2DirComp(&pu1Read, &u1SrcByte);

                // deal with the remainder pixels
                if (u4MsbLeft == 1)
                {
                    for (i = 0, sht = u4BitNumPerByte - u4BitNumPerPix;
                         i < u4Remainder;
                         i++, sht -= u4BitNumPerPix)
                    {
                        u4PaleIdx = GFX_SwGetPaleIdx(u4CharCM, sht, u1SrcByte);

                        GFX_SwSetIdx2DirComp(&pu1Write, u4DstCM, pu1PaleBase, u4PaleIdx);
                    } // ~for
                }
                else
                {
                    for (i = 0, sht = 0;
                         i < u4Remainder;
                         i++, sht += u4BitNumPerPix)
                    {
                        u4PaleIdx = GFX_SwGetPaleIdx(u4CharCM, sht, u1SrcByte);

                        GFX_SwSetIdx2DirComp(&pu1Write, u4DstCM, pu1PaleBase, u4PaleIdx);
                    } // ~for
                }
                
            }

            pu1DstLine += u4DstPitch;
            pu1Write    = pu1DstLine;
        } // ~for
    }
    else
    {
        g   = 0;
        y   = 0;
        do
        {
            GFX_SwGetIdx2DirComp(&pu1Read, &u1SrcByte);
            
            if (u4MsbLeft == 1)
            {
                for (i = 0, sht = u4BitNumPerByte - u4BitNumPerPix;
                     i < u4PixPerByte;
                     i++, sht -= u4BitNumPerPix)
                {
                    u4PaleIdx = GFX_SwGetPaleIdx(u4CharCM, sht, u1SrcByte);
            
                    GFX_SwSetIdx2DirComp(&pu1Write, u4DstCM, pu1PaleBase, u4PaleIdx);
                    g++;
                    if (g>=u4Width)
                    {
                        pu1DstLine += u4DstPitch;
                        pu1Write    = pu1DstLine;
                        y++;
                        g           = 0;
                        if (y >= u4Height)return (INT32)E_GFX_OK;
                    }
                } // ~for
            }
            else
            {
                for (i = 0, sht = 0;
                     i < u4PixPerByte;
                     i++, sht += u4BitNumPerPix)
                {
                    u4PaleIdx = GFX_SwGetPaleIdx(u4CharCM, sht, u1SrcByte);
            
                    GFX_SwSetIdx2DirComp(&pu1Write, u4DstCM, pu1PaleBase, u4PaleIdx);
                    g++;
                    if (g>=u4Width)
                    {
                        pu1DstLine += u4DstPitch;
                        pu1Write    = pu1DstLine;
                        y++;
                        g           = 0;
                        if (y >= u4Height)return (INT32)E_GFX_OK;
                    }
                } // ~for
            }
        }
        while(y < u4Height);
    }
    return (INT32)E_GFX_OK;
}

//-------------------------------------------------------------------------
/** _GfxSwIndexToDirectBitBlt
 *  Raster OPeration bitblt
 */
//-------------------------------------------------------------------------
/*lint -save -e613 */


//-------------------------------------------------------------------------
/** GFX_SwIndexToDirectBitBlt
 *  Index to Direct Color Bitblt
 *  CharCM = 1, 2, 4, 8 bpp
 *  DstCM  = 16, 32 bpp
 */
//-------------------------------------------------------------------------
/*lint -save -e704 -e613 */
INT32 GFX_SwIndexToDirectBitBltEx(const GFX_IDX2DIR_DATA_T *prsData)
{
    UINT8 *pu1SrcBase, *pu1DstBase, *pu1PaleBase;
    UINT32 u4SrcPitch, u4DstPitch;
    UINT32 u4CharCM, u4DstCM;
    UINT32 u4Width, u4Height;
    UINT32 u4ByteAlign, u4MsbLeft;
    UINT32 i, y, g, sht;
    UINT8 *pu1SrcLine= NULL;//*pu1SrcLine = NULL,
    UINT8 *pu1DstLine= NULL;//*pu1SrcLine = NULL,
    UINT8 *pu1Read, *pu1Write;
    UINT32 u4BitNumPerByte, u4BitNumPerPix;
    UINT32 u4PixPerByte, u4PixGroup, u4Remainder;
    UINT32 u4PaleIdx;
    UINT8  u1SrcByte;

    // check parameters' error
    VERIFY(prsData != NULL);
    VERIFY(prsData->pu1SrcBase != NULL);
    VERIFY(prsData->pu1DstBase != NULL);


    pu1SrcBase = prsData->pu1SrcBase;
    u4SrcPitch = prsData->u4SrcPitch;   // Ignore it.
    pu1DstBase = prsData->pu1DstBase;
    u4DstPitch = prsData->u4DstPitch;
    u4CharCM   = prsData->u4CharCM;
    u4DstCM    = prsData->u4DstCM;
    u4Width    = prsData->u4Width;
    u4Height   = prsData->u4Height;

    pu1PaleBase = prsData->pu1PaleBase;
    u4MsbLeft   = prsData->u4MsbLeft;
    u4ByteAlign = prsData->u4ByteAlign; // LN_ST_BYTE_AL

    u4BitNumPerByte = 8;            // bit_num / byte

    switch (prsData->u4CharCM)
    {
        // src_cm = 1 bpp
        case E_BMP_CM_1BIT:
            u4BitNumPerPix = 1;     // bit_num / pix
            break;

        // src_cm = 2 bpp
        case E_BMP_CM_2BIT:
            u4BitNumPerPix = 2;     // bit_num / pix
            break;

        // src_cm = 4 bpp
        case E_BMP_CM_4BIT:
            u4BitNumPerPix = 4;     // bit_num / pix
            break;

        // src_cm = 8 bpp
        case E_BMP_CM_8BIT:
            u4BitNumPerPix = 8;     // bit_num / pix
            break;

        default:
            return (INT32)E_GFX_INV_ARG;
    } // ~switch

    // pixels / byte = (bit_num/byte) / (bit_num/pix)
    u4PixPerByte = u4BitNumPerByte / u4BitNumPerPix;

    u4Remainder = 0;
    u4Remainder = (u4Width % u4PixPerByte);
    u4PixGroup  = (u4Width / u4PixPerByte);

    if (u4Remainder > 0)
    {
        if (u4ByteAlign != 1)
        {
     //       LOG(5, "Error: (u4Remainder > 0), but (u4ByteAlign != 1)\n");
        }
    }

    pu1Read    = pu1SrcBase;
    pu1Write   = pu1DstBase;
    pu1SrcLine = pu1SrcBase;
    pu1DstLine = pu1DstBase;

    for (y = 0; y < u4Height; y++)
    {
        for (g = 0; g < u4PixGroup; g++)
        {
            GFX_SwGetIdx2DirComp(&pu1Read, &u1SrcByte);

            if (u4MsbLeft == 1)
            {
                for (i = 0, sht = u4BitNumPerByte - u4BitNumPerPix;
                     i < u4PixPerByte;
                     i++, sht -= u4BitNumPerPix)
                {
                    u4PaleIdx = GFX_SwGetPaleIdx(u4CharCM, sht, u1SrcByte);

                    GFX_SwSetIdx2DirComp(&pu1Write, u4DstCM, pu1PaleBase, u4PaleIdx);
                } // ~for
            }
            else
            {
                for (i = 0, sht = 0;
                     i < u4PixPerByte;
                     i++, sht += u4BitNumPerPix)
                {
                    u4PaleIdx = GFX_SwGetPaleIdx(u4CharCM, sht, u1SrcByte);

                    GFX_SwSetIdx2DirComp(&pu1Write, u4DstCM, pu1PaleBase, u4PaleIdx);
                } // ~for
            }
        } // ~for

        if (u4Remainder != 0)
        {
            
            //if (u4ByteAlign == 1) u4Remainder = u4PixPerByte;
            GFX_SwGetIdx2DirComp(&pu1Read, &u1SrcByte);

            // deal with the remainder pixels
            if (u4MsbLeft == 1)
            {
                for (i = 0, sht = u4BitNumPerByte - u4BitNumPerPix;
                     i < u4Remainder;
                     i++, sht -= u4BitNumPerPix)
                {
                    u4PaleIdx = GFX_SwGetPaleIdx(u4CharCM, sht, u1SrcByte);

                    GFX_SwSetIdx2DirComp(&pu1Write, u4DstCM, pu1PaleBase, u4PaleIdx);
                } // ~for
            }
            else
            {
                for (i = 0, sht = 0;
                     i < u4Remainder;
                     i++, sht += u4BitNumPerPix)
                {
                    u4PaleIdx = GFX_SwGetPaleIdx(u4CharCM, sht, u1SrcByte);

                    GFX_SwSetIdx2DirComp(&pu1Write, u4DstCM, pu1PaleBase, u4PaleIdx);
                } // ~for
            }
        }
        pu1SrcLine += u4SrcPitch;
        pu1Read     = pu1SrcLine;
        pu1DstLine += u4DstPitch;
        pu1Write    = pu1DstLine;
    } // ~for

    return (INT32)E_GFX_OK;
}

static INT32 _GfxSwIndexToDirectBitBlt(const MI_DIF_FIELD_T *prsGfxReg)
{
    GFX_IDX2DIR_DATA_T rData;
    UINT32 ui4Temp = 0;

    // get source & destination info
    //rData.pu1SrcBase = (UINT8 *)(prsGfxReg->fg_SRC_BSAD);
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_SRC_BSAD);
    rData.pu1SrcBase = (UINT8 *)ui4Temp;

    rData.u4SrcPitch = prsGfxReg->fg_SRC_PITCH << 4;    // 128bit aligned
    //rData.pu1DstBase = (UINT8 *)(prsGfxReg->fg_DST_BSAD);
    ui4Temp = 0;
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_DST_BSAD);
    rData.pu1DstBase = (UINT8 *)ui4Temp;

    rData.u4DstPitch = prsGfxReg->fg_OSD_WIDTH << 4;    // 128bit aligned
    rData.u4CharCM   = prsGfxReg->fg_CHAR_CM;
    rData.u4DstCM    = prsGfxReg->fg_CM;
    rData.u4Width    = prsGfxReg->fg_SRC_WIDTH;
    rData.u4Height   = prsGfxReg->fg_SRC_HEIGHT + 1;

    //rData.pu1PaleBase = (UINT8 *)(prsGfxReg->fg_PAL_BSAD);
    ui4Temp = 0;
    ui4Temp = VIRADDR(prsGfxReg, (UINT32)prsGfxReg->fg_PAL_BSAD);
    rData.pu1PaleBase = (UINT8 *)ui4Temp;

    rData.u4MsbLeft   = prsGfxReg->fg_MSB_LEFT;
    rData.u4ByteAlign = prsGfxReg->fg_LN_ST_BYTE_AL;

    rData.u4SrcPitchEn = prsGfxReg->fg_SRC_PITCH_ENA;

    // virtual address
    rData.pu1SrcBase = (rData.pu1SrcBase);
    // virtual address
    rData.pu1DstBase = (rData.pu1DstBase);
    // virtual address
    rData.pu1PaleBase = (rData.pu1PaleBase);

    if (rData.u4SrcPitchEn)
    {
        return GFX_SwIndexToDirectBitBltEx(&rData);
    }
    else
    {
        return GFX_SwIndexToDirectBitBlt(&rData);
    }
}

//---------------------------------------------------------------------------
// Inter-file functions
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------------


//-------------------------------------------------------------------------
/** GFX_SwInit
 *
 */
//-------------------------------------------------------------------------
INT32 GFX_SwInit(void)
{
    GFX_SW_FX_ENTRY

    _pfnGfxSwCallBack   = NULL;
    _pvGfxSwCallBackTag = NULL;
    _u4GfxSwActionCount = 0;

    GFX_UNUSED_RET(x_memset(_au4GfxSwReg, 0, sizeof(_au4GfxSwReg)))

    return (INT32)E_GFX_OK;
}


//-------------------------------------------------------------------------
/** GFX_SwGetRegBase
 *
 */
//-------------------------------------------------------------------------
INT32 GFX_SwGetRegBase(UINT32 u4GfxHwId, UINT32 **ppu4RegBase)
{
    if ((ppu4RegBase != NULL) && (*ppu4RegBase != NULL))
    {
        *ppu4RegBase = (UINT32 *)&_au4GfxSwReg[u4GfxHwId][0];
        return (INT32)E_GFX_OK;
    }

    return -(INT32)E_GFX_INV_ARG;
}


//-------------------------------------------------------------------------
/** GFX_SwISR
 *
 */
//-------------------------------------------------------------------------
void GFX_SwISR(void)
{
    if (_pfnGfxSwCallBack != NULL)
    {
        _pfnGfxSwCallBack(_pvGfxSwCallBackTag);
    }
}


//-------------------------------------------------------------------------
/** GFX_SwSetCallBack
 *
 */
//-------------------------------------------------------------------------
INT32 GFX_SwSetCallBack(void (*pfnCallBack)(void *pvTag), void *pvTag)
{
    if (pfnCallBack != NULL)
    {
        _pfnGfxSwCallBack   = pfnCallBack;
        _pvGfxSwCallBackTag = pvTag;
        return (INT32)E_GFX_OK;
    }

    return -(INT32)E_GFX_INV_ARG;
}


//-------------------------------------------------------------------------
/** GFX_SwGetOpCount
 *
 */
//-------------------------------------------------------------------------
UINT32 GFX_SwGetOpCount(void)
{
    return _u4GfxSwActionCount;
}


//-------------------------------------------------------------------------
/** GFX_SwAction
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e740 -e826 */
INT32 GFX_SwAction(UINT32 u4GfxHwId)
{
    INT32 i4Ret = (INT32)E_GFX_OK;
    MI_DIF_FIELD_T *prGfxReg;

    prGfxReg = (MI_DIF_FIELD_T *)&_au4GfxSwReg[u4GfxHwId][0];

    switch (prGfxReg->fg_OP_MODE)
    {
    case OP_TEXT_BLT:
        //i4Ret = _GfxSwTextBlt(prGfxReg);
        i4Ret = _GfxSwPgigDecode(prGfxReg);
        break;

    case OP_RECT_FILL:
        i4Ret = _GfxSwRectFill(prGfxReg);
        break;

    case OP_DRAW_HLINE:
        i4Ret = _GfxSwDrawHline(prGfxReg);
        break;

    case OP_DRAW_VLINE:
        i4Ret = _GfxSwDrawVline(prGfxReg);
        break;

    case OP_GRAD_FILL:
        i4Ret = _GfxSwGradFill(prGfxReg);
        break;

    case OP_BITBLT:
        
		if(!prGfxReg->fg_SRC_CM_24BPP)
		{
			i4Ret = _GfxSwBitBlt(prGfxReg);
		}
		else
		{
			i4Ret = _Gfx24BppSwBitBlt(prGfxReg);
		}
        break;

    case OP_DMA:
        i4Ret = _GfxDma(prGfxReg);
        break;

    case OP_ALPHA_BITBLT:
        i4Ret = _GfxAlphaBitBlt(prGfxReg);
        break;

	case OP_MS_ALPHA_BITBLT:
		i4Ret = _GfxMsAlphaBitBlt(prGfxReg);
		break;	

    case OP_ALPHA_COMPOS_BITBLT:
        i4Ret = _GfxAlphaComposition(prGfxReg);
        break;

    case OP_YCRCB_RGB_CNV:
        i4Ret = _GfxYCrCb2Rgb(prGfxReg);
        break;

    case OP_ALPHA_MAP_BITBLT:
        i4Ret = _GfxAlphaMapBitBlt(prGfxReg);
        break;

//add by msz00441 for rop & index2direct & hline2vline
    case OP_ROP_BITBLT:
        i4Ret = _GfxSwRopBitBlt(prGfxReg);
        break;
  case OP_IDX2DIR_BITBLT:
        #if 0
        i4Ret = _GfxSwInx2DirBitBlt(prGfxReg);
        #else
         i4Ret = _GfxSwIndexToDirectBitBlt(prGfxReg);

        #endif
        break;
  case OP_H2V_LINE:
        i4Ret = _GfxSwH2VLine(prGfxReg);
        break;
//add end
//add by msz00441 080603 for loop mode
    case OP_LOOP_ALPAH_COMPOS:
        i4Ret = _GfxSwAlphaCompositionLoop(prGfxReg);
        break;

    case OP_OBLIQUE_LINE:
        i4Ret = _GfxSwDrawObliqueLine(prGfxReg);
        break;
    default:
        return -(INT32)E_GFX_INV_ARG;
    }

    return i4Ret;
}
/*lint -restore */


//-------------------------------------------------------------------------
/** GFX_SwSetColorComponent
 *  gfx alpha-map bitblt
 *  set color components to frame buffer,
 *  advance frame buffer point for 1 pixel
 */
//-------------------------------------------------------------------------
/*lint -save -e703 -e704 -e826 -e961 */
void GFX_SwSetColorComponent(UINT8 **ppu1DestFb, UINT32 u4CM,
    const INT32 *pi4sColorComponent)
{
    INT32  i4Q, i4W, i4E, i4R;
    UINT32 u4Color;
    UINT16 u2Color;
    UINT8  u1Color;

    if ((ppu1DestFb == NULL) || (pi4sColorComponent == NULL))
    {
        return;
    }

    i4Q = pi4sColorComponent[0];
    i4W = pi4sColorComponent[1];
    i4E = pi4sColorComponent[2];
    i4R = pi4sColorComponent[3];

    i4Q = (i4Q > 255) ? 255 :
          (i4Q <   0) ?   0 : i4Q;

    i4W = (i4W > 255) ? 255 :
          (i4W <   0) ?   0 : i4W;

    i4E = (i4E > 255) ? 255 :
          (i4E <   0) ?   0 : i4E;

    i4R = (i4R > 255) ? 255 :
          (i4R <   0) ?   0 : i4R;

    switch (u4CM)
    {
    case CM_RGB_CLUT8:
        u1Color = i4R & 0xff;
        *((UINT8 *)*ppu1DestFb) = u1Color;
        *ppu1DestFb += 1;
        break;

    case CM_RGB565_DIRECT16:
        u2Color = ((i4W & 0xf8) << 8)   |
                  ((i4E & 0xfc) << 3)   |
                  (i4R >> 3);
        *((UINT16 *)*ppu1DestFb) = u2Color;
        *ppu1DestFb += 2;
        break;

    case CM_ARGB1555_DIRECT16:
        u2Color = (i4Q ? 0x8000 : 0)     |
                  ((i4W & 0xf8) << 7)    |
                  ((i4E & 0xf8) << 2)    |
                  (i4R >> 3);
        *((UINT16 *)*ppu1DestFb) = u2Color;
        *ppu1DestFb += 2;
        break;

    case CM_ARGB4444_DIRECT16:
        u2Color = ((i4Q & 0xf0) << 8)   |
                  ((i4W & 0xf0) << 4)   |
                  (i4E & 0xf0)          |
                  (i4R >> 4);
        *((UINT16 *)*ppu1DestFb) = u2Color;
        *ppu1DestFb += 2;
        break;

    case CM_ARGB8888_DIRECT32:
        u4Color = (i4Q << 24) |
                  (i4W << 16) |
                  (i4E <<  8) |
                  i4R;
        *((UINT32 *)(*ppu1DestFb)) = u4Color;
        *ppu1DestFb += 4;
        break;

    default:
        return;
    }

    return;
}
/*lint -restore */


//-------------------------------------------------------------------------
/** GFX_SwSetColorComponent
 *  gfx alpha-map bitblt
 *  set color components to frame buffer,
 *  advance frame buffer point for 1 pixel
 */
//-------------------------------------------------------------------------
/*lint -save -e703 -e704 -e826 -e961 */
void GFX_SwSetColorComponentEx(UINT8 **ppu1DestFb, UINT32 u4CM, const INT32 *pi4sColorComponent)
{
    INT32  i4Q, i4W, i4E, i4R;
    UINT32 u4Color;
    UINT16 u2Color;
    UINT8  u1Color;

    if ((ppu1DestFb == NULL) || (pi4sColorComponent == NULL))
    {
        return;
    }

    i4Q = pi4sColorComponent[0];
    i4W = pi4sColorComponent[1];
    i4E = pi4sColorComponent[2];
    i4R = pi4sColorComponent[3];

    i4Q = (i4Q > 255) ? 255 :
          (i4Q <   0) ?   0 : i4Q;

    i4W = (i4W > 255) ? 255 :
          (i4W <   0) ?   0 : i4W;

    i4E = (i4E > 255) ? 255 :
          (i4E <   0) ?   0 : i4E;

    i4R = (i4R > 255) ? 255 :
          (i4R <   0) ?   0 : i4R;

    switch (u4CM)
    {
    case CM_RGB_CLUT8:
        u1Color = i4R & 0xff;
        *((UINT8 *)*ppu1DestFb) = u1Color;
        *ppu1DestFb += 1;
        break;

    case CM_RGB565_DIRECT16:
        u2Color = ((i4W & 0x1f) << 11)   |
                  ((i4E & 0x3f) << 5)   |
                  (i4R & 0x1f);
        *((UINT16 *)*ppu1DestFb) = u2Color;
        *ppu1DestFb += 2;
        break;

    case CM_ARGB1555_DIRECT16:
        u2Color = ((i4Q & 0x1) ? 0x8000 : 0)     |
                  ((i4W & 0x1f) << 10)    |
                  ((i4E & 0x1f) << 5)    |
                  (i4R & 0x1f);
        *((UINT16 *)*ppu1DestFb) = u2Color;
        *ppu1DestFb += 2;
        break;

    case CM_ARGB4444_DIRECT16:
        u2Color = ((i4Q & 0xf) << 12)  |
                  ((i4W & 0xf) << 8)   |
                  ((i4E & 0xf) << 4)   |
                  (i4R & 0xf);
        *((UINT16 *)*ppu1DestFb) = u2Color;
        *ppu1DestFb += 2;
        break;

    case CM_ARGB8888_DIRECT32:
        u4Color = ((i4Q & 0xff) << 24) |
                  ((i4W & 0xff) << 16) |
                  ((i4E & 0xff) <<  8) |
                  (i4R & 0xff);
        *((UINT32 *)(*ppu1DestFb)) = u4Color;
        *ppu1DestFb += 4;
        break;

    default:
        return;
    }

    return;
}
/*lint -restore */


//-------------------------------------------------------------------------
/** GFX_SwSetColorComponent
 *  gfx alpha-map bitblt
 *  set color components to frame buffer,
 *  advance frame buffer point for 1 pixel
 */
//-------------------------------------------------------------------------
/*lint -save -e703 -e704 -e826 -e961 */
void GFX_SwSetColorComponent_Mirror(UINT8 **ppu1DestFb, UINT32 u4CM,
    const INT32 *pi4sColorComponent)
{
    INT32  i4Q, i4W, i4E, i4R;
    UINT32 u4Color;
    UINT16 u2Color;
    UINT8  u1Color;

    if ((ppu1DestFb == NULL) || (pi4sColorComponent == NULL))
    {
        return;
    }

    i4Q = pi4sColorComponent[0];
    i4W = pi4sColorComponent[1];
    i4E = pi4sColorComponent[2];
    i4R = pi4sColorComponent[3];

    i4Q = (i4Q > 255) ? 255 :
          (i4Q <   0) ?   0 : i4Q;

    i4W = (i4W > 255) ? 255 :
          (i4W <   0) ?   0 : i4W;

    i4E = (i4E > 255) ? 255 :
          (i4E <   0) ?   0 : i4E;

    i4R = (i4R > 255) ? 255 :
          (i4R <   0) ?   0 : i4R;

    switch (u4CM)
    {
    case CM_RGB_CLUT8:
        u1Color = i4R & 0xff;
        *((UINT8 *)*ppu1DestFb) = u1Color;
        *ppu1DestFb -= 1;
        break;

    case CM_RGB565_DIRECT16:
        u2Color = ((i4W & 0xf8) << 8)   |
                  ((i4E & 0xfc) << 3)   |
                  (i4R >> 3);
        *((UINT16 *)*ppu1DestFb) = u2Color;
        *ppu1DestFb -= 2;
        break;

    case CM_ARGB1555_DIRECT16:
        u2Color = (i4Q ? 0x8000 : 0)     |
                  ((i4W & 0xf8) << 7)    |
                  ((i4E & 0xf8) << 2)    |
                  (i4R >> 3);
        *((UINT16 *)*ppu1DestFb) = u2Color;
        *ppu1DestFb -= 2;
        break;

    case CM_ARGB4444_DIRECT16:
        u2Color = ((i4Q & 0xf0) << 8)   |
                  ((i4W & 0xf0) << 4)   |
                  (i4E & 0xf0)          |
                  (i4R >> 4);
        *((UINT16 *)*ppu1DestFb) = u2Color;
        *ppu1DestFb -= 2;
        break;

    case CM_ARGB8888_DIRECT32:
        u4Color = (i4Q << 24) |
                  (i4W << 16) |
                  (i4E <<  8) |
                  i4R;
        *((UINT32 *)*ppu1DestFb) = u4Color;
        *ppu1DestFb -= 4;
        break;

    default:
        return;
    }

    return;
}

/*lint -restore */


//-------------------------------------------------------------------------
/** GFX_SwGetColorComponent
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e704 -e826 -e961 */
void GFX_SwGetColorComponent(UINT8 **ppu1DestFb, UINT32 u4CM,
    INT32 *pi4ColorComponent)
{
    // get color components from frame buffer, advance frame buffer point for 1 pixel
    INT32  i4Q, i4W, i4E, i4R;
    UINT32 u4Color;
    UINT16 u2Color;
    UINT8  u1Color;

    if ((ppu1DestFb == NULL) || (pi4ColorComponent == NULL))
    {
        return;
    }

    switch (u4CM)
    {
    case CM_RGB_CLUT8:
        u1Color = *((UINT8 *)*ppu1DestFb);
        i4Q = u1Color & 0xff;
        i4W = u1Color & 0xff;
        i4E = u1Color & 0xff;
        i4R = u1Color & 0xff;
        *ppu1DestFb += 1;
        break;

    case CM_RGB565_DIRECT16:
        u2Color = *((UINT16 *)*ppu1DestFb);
        if (TRUE == _u4UseRGB565Alpha)
        {
            i4Q = _u4RGB565Alpha;
        }
        else
        {
            i4Q = 0xff;
        }

        i4W = (u2Color >> 8) & 0xf8;
        i4E = (u2Color >> 3) & 0xfc;
        i4R = (u2Color << 3) & 0xf8;

        i4W = i4W | (i4W >> 5);
        i4E = i4E | (i4E >> 6);
        i4R = i4R | (i4R >> 5);

        *ppu1DestFb += 2;
        break;

    case CM_ARGB1555_DIRECT16:
        u2Color = *((UINT16 *)*ppu1DestFb);
        i4Q = (u2Color & 0x8000) ? 0xff : 0;
        i4W = (u2Color >> 7) & 0xf8;
        i4E = (u2Color >> 2) & 0xf8;
        i4R = (u2Color << 3) & 0xf8;

        i4W = i4W | (i4W >> 5);
        i4E = i4E | (i4E >> 5);
        i4R = i4R | (i4R >> 5);

        *ppu1DestFb += 2;
        break;

    case CM_ARGB4444_DIRECT16:
        u2Color = *((UINT16 *)*ppu1DestFb);
        i4Q = (u2Color >> 8) & 0xf0;
        i4W = (u2Color >> 4) & 0xf0;
        i4E = u2Color & 0xf0;
        i4R = (u2Color << 4) & 0xf0;

        i4Q = i4Q | (i4Q >> 4);
        i4W = i4W | (i4W >> 4);
        i4E = i4E | (i4E >> 4);
        i4R = i4R | (i4R >> 4);

        *ppu1DestFb += 2;
        break;

    case CM_ARGB8888_DIRECT32:
        u4Color = *((UINT32 *)*ppu1DestFb);
        i4Q = u4Color >> 24;
        i4W = (u4Color >> 16) & 0xff;
        i4E = (u4Color >>  8) & 0xff;
        i4R = u4Color & 0xff;
        *ppu1DestFb += 4;
        break;
		
	case CM_RGB888_DIRECT24:
        i4Q = 0xff;
        u4Color = *((UINT8 *)*ppu1DestFb);
        i4R = u4Color & 0xff;
        *ppu1DestFb += 1;
        u4Color = *((UINT8 *)*ppu1DestFb);
        i4E = u4Color & 0xff;
        *ppu1DestFb += 1;
        u4Color = *((UINT8 *)*ppu1DestFb);
        i4W = u4Color & 0xff;
        *ppu1DestFb += 1;
        break;
		
    default:
        return;
    }

    pi4ColorComponent[0] = i4Q;
    pi4ColorComponent[1] = i4W;
    pi4ColorComponent[2] = i4E;
    pi4ColorComponent[3] = i4R;

    return;
}

/*lint -restore */


//-------------------------------------------------------------------------
/** GFX_SwGetColorComponent
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e704 -e826 -e961 */
void GFX_SwGetColorComponentEx(UINT8 **ppu1DestFb, UINT32 u4CM, INT32 *pi4ColorComponent)
{
    // get color components from frame buffer, advance frame buffer point for 1 pixel
    INT32  i4Q, i4W, i4E, i4R;
    UINT32 u4Color;
    UINT16 u2Color;
    UINT8  u1Color;

    if ((ppu1DestFb == NULL) || (pi4ColorComponent == NULL))
    {
        return;
    }

    switch (u4CM)
    {
    case CM_RGB_CLUT8:
        u1Color = *((UINT8 *)*ppu1DestFb);
        i4Q = u1Color & 0xff;
        i4W = u1Color & 0xff;
        i4E = u1Color & 0xff;
        i4R = u1Color & 0xff;
        *ppu1DestFb += 1;
        break;

    case CM_RGB565_DIRECT16:
        u2Color = *((UINT16 *)*ppu1DestFb);

        i4Q = 0xff;
        i4W = (u2Color >> 11) & 0x1f;
        i4E = (u2Color >> 5) & 0x3f;
        i4R = u2Color & 0x1f;

        *ppu1DestFb += 2;
        break;

    case CM_ARGB1555_DIRECT16:
        u2Color = *((UINT16 *)*ppu1DestFb);
        i4Q = (u2Color & 0x8000) ? 0x1 : 0;
        i4W = (u2Color >> 10) & 0x1f;
        i4E = (u2Color >> 5) & 0x1f;
        i4R = u2Color & 0x1f;

        *ppu1DestFb += 2;
        break;

    case CM_ARGB4444_DIRECT16:
        u2Color = *((UINT16 *)*ppu1DestFb);
        i4Q = (u2Color >> 12) & 0xf;
        i4W = (u2Color >> 8) & 0xf;
        i4E = (u2Color >> 4) & 0xf;
        i4R = u2Color & 0xf;

        *ppu1DestFb += 2;
        break;

    case CM_ARGB8888_DIRECT32:
        u4Color = *((UINT32 *)*ppu1DestFb);
        i4Q = u4Color >> 24;
        i4W = (u4Color >> 16) & 0xff;
        i4E = (u4Color >>  8) & 0xff;
        i4R = u4Color & 0xff;
        *ppu1DestFb += 4;
        break;

    default:
        return;
    }

    pi4ColorComponent[0] = i4Q;
    pi4ColorComponent[1] = i4W;
    pi4ColorComponent[2] = i4E;
    pi4ColorComponent[3] = i4R;

    return;
}
/*lint -restore */

/*lint -restore */


//-------------------------------------------------------------------------
/** GFX_SwGetColorComponent
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e704 -e826 -e961 */
void GFX_SwGetColorComponent_Mirror(UINT8 **ppu1DestFb, UINT32 u4CM,
    INT32 *pi4ColorComponent)
{
    // get color components from frame buffer, advance frame buffer point for 1 pixel
    INT32  i4Q, i4W, i4E, i4R;
    UINT32 u4Color;
    UINT16 u2Color;
    UINT8  u1Color;

    if ((ppu1DestFb == NULL) || (pi4ColorComponent == NULL))
    {
        return;
    }

    switch (u4CM)
    {
    case CM_RGB_CLUT8:
        u1Color = *((UINT8 *)*ppu1DestFb);
        i4Q = u1Color & 0xff;
        i4W = u1Color & 0xff;
        i4E = u1Color & 0xff;
        i4R = u1Color & 0xff;
        *ppu1DestFb -= 1;
        break;

    case CM_RGB565_DIRECT16:
        u2Color = *((UINT16 *)*ppu1DestFb);
        i4Q = 0xff;
        i4W = (u2Color >> 8) & 0xf8;
        i4E = (u2Color >> 3) & 0xfc;
        i4R = (u2Color << 3) & 0xf8;

        i4W = i4W | (i4W >> 5);
        i4E = i4E | (i4E >> 6);
        i4R = i4R | (i4R >> 5);

        *ppu1DestFb -= 2;
        break;

    case CM_ARGB1555_DIRECT16:
        u2Color = *((UINT16 *)*ppu1DestFb);
        i4Q = (u2Color & 0x8000) ? 0xff : 0;
        i4W = (u2Color >> 7) & 0xf8;
        i4E = (u2Color >> 2) & 0xf8;
        i4R = (u2Color << 3) & 0xf8;

        i4W = i4W | (i4W >> 5);
        i4E = i4E | (i4E >> 5);
        i4R = i4R | (i4R >> 5);

        *ppu1DestFb -= 2;
        break;

    case CM_ARGB4444_DIRECT16:
        u2Color = *((UINT16 *)*ppu1DestFb);
        i4Q = (u2Color >> 8) & 0xf0;
        i4W = (u2Color >> 4) & 0xf0;
        i4E = u2Color & 0xf0;
        i4R = (u2Color << 4) & 0xf0;

        i4Q = i4Q | (i4Q >> 4);
        i4W = i4W | (i4W >> 4);
        i4E = i4E | (i4E >> 4);
        i4R = i4R | (i4R >> 4);

        *ppu1DestFb -= 2;
        break;

    case CM_ARGB8888_DIRECT32:
        u4Color = *((UINT32 *)*ppu1DestFb);
        i4Q = u4Color >> 24;
        i4W = (u4Color >> 16) & 0xff;
        i4E = (u4Color >>  8) & 0xff;
        i4R = u4Color & 0xff;
        *ppu1DestFb -= 4;
        break;

    default:
        return;
    }

    pi4ColorComponent[0] = i4Q;
    pi4ColorComponent[1] = i4W;
    pi4ColorComponent[2] = i4E;
    pi4ColorComponent[3] = i4R;

    return;
}


//-------------------------------------------------------------------------
/** GFX_SwGetColorComponent_5381
 *  only for "Alpha-Composition" and "Alpha-Blending"
 */
//-------------------------------------------------------------------------
/*lint -save -e704 -e826 -e961 */
void GFX_SwGetColorComponent_5381(UINT8 **ppu1DestFb, UINT32 u4CM,
    INT32 *pi4ColorComponent)
{
    // get color components from frame buffer, advance frame buffer point for 1 pixel
    INT32  i4Q, i4W, i4E, i4R;
    UINT32 u4Color;
    UINT16 u2Color;
    UINT8  u1Color;

    if ((ppu1DestFb == NULL) || (pi4ColorComponent == NULL))
    {
        return;
    }

    switch (u4CM)
    {
    case CM_RGB_CLUT8:
        u1Color = *((UINT8 *)*ppu1DestFb);
        i4Q = u1Color & 0xff;
        i4W = u1Color & 0xff;
        i4E = u1Color & 0xff;
        i4R = u1Color & 0xff;
        *ppu1DestFb += 1;
        break;

    case CM_RGB565_DIRECT16:
        u2Color = *((UINT16 *)*ppu1DestFb);
        i4Q = 0xff;
        i4W = (u2Color >> 8) & 0xf8;
        i4E = (u2Color >> 3) & 0xfc;
        i4R = (u2Color << 3) & 0xf8;

        i4W = i4W | (i4W >> 5);
        i4E = i4E | (i4E >> 6);
        i4R = i4R | (i4R >> 5);

        *ppu1DestFb += 2;
        break;

    case CM_ARGB1555_DIRECT16:
        u2Color = *((UINT16 *)*ppu1DestFb);
        i4Q = (u2Color & 0x8000) ? 0xff : 0;
        i4W = (u2Color >> 7) & 0xf8;
        i4E = (u2Color >> 2) & 0xf8;
        i4R = (u2Color << 3) & 0xf8;

        i4W = i4W | (i4W >> 5);
        i4E = i4E | (i4E >> 5);
        i4R = i4R | (i4R >> 5);

        *ppu1DestFb += 2;
        break;

    case CM_ARGB4444_DIRECT16:
        u2Color = *((UINT16 *)*ppu1DestFb);
        i4Q = (u2Color >> 8) & 0xf0;
        i4W = (u2Color >> 4) & 0xf0;
        i4E = u2Color & 0xf0;
        i4R = (u2Color << 4) & 0xf0;

        // if (As or Ad == 15) repeat MSB, else no repeat
        // Cs and Cd, no repeat
        if (i4Q == 0xF0)
        {
            i4Q = i4Q | (i4Q >> 4);
        }

        *ppu1DestFb += 2;
        break;

    case CM_ARGB8888_DIRECT32:
        u4Color = *((UINT32 *)*ppu1DestFb);
        i4Q = u4Color >> 24;
        i4W = (u4Color >> 16) & 0xff;
        i4E = (u4Color >>  8) & 0xff;
        i4R = u4Color & 0xff;
        *ppu1DestFb += 4;
        break;

    default:
        return;
    }

    pi4ColorComponent[0] = i4Q;
    pi4ColorComponent[1] = i4W;
    pi4ColorComponent[2] = i4E;
    pi4ColorComponent[3] = i4R;

    return;
}

//-------------------------------------------------------------------------
/** GFX_SwBitBlt
 *  gfx normal bitblt
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e740 -e613 */
#if 1
//-------------------------------------------------------------------------
/** GFX_SwBitBlt
 *  gfx normal bitblt
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e740 -e613 */
INT32 GFX_SwBitBlt(UINT8 *pu1SrcBase, UINT8 *pu1DstBase, UINT32 u4SrcPitch,
    UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM, UINT32 u4Width,
    UINT32 u4Height, UINT32 u4ColorMin, UINT32 u4ColorMax, UINT32 u4DstColorMin,
    UINT32 u4DstColorMax, UINT32 u4TransEn,    UINT32 u4ColchgEn, UINT32 u4DisSrcKey, 
    UINT32 u4SrcKeyIn, UINT32 u4DisDstKey, UINT32 u4DstKeyIn, UINT32 u4KeynotEn, UINT32 u4Color, UINT32 u4Alpha)
{
    INT32 ai4SrcColorCompMin[4] = {0};
    INT32 ai4DstColorCompMin[4] = {0};
    INT32 ai4SrcColorCompMax[4] = {0};
    INT32 ai4DstColorCompMax[4] = {0};
    INT32 ai4SrcColorComp[4] = {0};
    INT32 ai4DstColorComp[4] = {0};
    INT32 ai4RectColor[4] = {0};
    UINT32 *pu4Color;
    UINT32 x, y;
    UINT8 *pu1SrcLine, *pu1DstLine;
    UINT8 *pu1Read, *pu1Write;

    // protection
    if (u4TransEn && u4ColchgEn)
    {
        return -(INT32)E_GFX_INV_ARG;
    }
    //HalFlushDCache();
    //HalInvalidateDCache();
    UTIL_Printf("[GFX_SwBitBlt] Enter !!!\n");
	
    HalFlushInvalidateDCache();
   // BIM_WAIT_WALE();
    pu1Read     = pu1SrcBase;
    pu1SrcLine  = pu1SrcBase;
    pu1Write    = pu1DstBase;
    pu1DstLine  = pu1DstBase;

    if (u4TransEn || u4ColchgEn)
    {
        ai4SrcColorCompMin[0] = (INT32) (u4ColorMin >> 24);
        ai4SrcColorCompMin[1] = (INT32)((u4ColorMin >> 16) & 0xFF);
        ai4SrcColorCompMin[2] = (INT32)((u4ColorMin >>  8) & 0xFF);
        ai4SrcColorCompMin[3] = (INT32)( u4ColorMin        & 0xFF);

        ai4SrcColorCompMax[0] = (INT32) (u4ColorMax >> 24);
        ai4SrcColorCompMax[1] = (INT32)((u4ColorMax >> 16) & 0xFF);
        ai4SrcColorCompMax[2] = (INT32)((u4ColorMax >>  8) & 0xFF);
        ai4SrcColorCompMax[3] = (INT32)( u4ColorMax        & 0xFF);

        ai4DstColorCompMin[0] = (INT32) (u4DstColorMin >> 24);
        ai4DstColorCompMin[1] = (INT32)((u4DstColorMin >> 16) & 0xFF);
        ai4DstColorCompMin[2] = (INT32)((u4DstColorMin >>  8) & 0xFF);
        ai4DstColorCompMin[3] = (INT32)( u4DstColorMin        & 0xFF);

        ai4DstColorCompMax[0] = (INT32) (u4DstColorMax >> 24);
        ai4DstColorCompMax[1] = (INT32)((u4DstColorMax >> 16) & 0xFF);
        ai4DstColorCompMax[2] = (INT32)((u4DstColorMax >>  8) & 0xFF);
        ai4DstColorCompMax[3] = (INT32)( u4DstColorMax        & 0xFF);

        pu4Color = &u4Color;
        u4Color  = _GfxSwColorExpansion(u4Color, u4DstCM);

        GFX_SwGetColorComponent((UINT8 **)&pu4Color, u4DstCM, ai4RectColor);
    }

    for (y = 0; y < u4Height; y++)
    {
        pu1Read  = pu1SrcLine;
        pu1Write = pu1DstLine;

        for (x = 0; x < u4Width; x++)
        {
            GFX_SwGetColorComponent(&pu1Read,  u4SrcCM, ai4SrcColorComp);
            GFX_SwGetColorComponent(&pu1Write, u4DstCM, ai4DstColorComp);
            
            if (CM_RGB_CLUT8 == u4DstCM)
            {
                pu1Write -= 1;
            }
            else if (CM_ARGB8888_DIRECT32 == u4DstCM)
            {
                pu1Write -= 4;
            }
            else
            {
                pu1Write -= 2;
            }
            
            if ((CM_ARGB1555_DIRECT16 == u4DstCM) && (ai4DstColorComp[0] == 0))
            {
                ai4DstColorComp[0] = u4Alpha;
            }
            
            if ((CM_ARGB1555_DIRECT16 == u4SrcCM) && (ai4SrcColorComp[0] == 0))
            {
                ai4DstColorComp[0] = u4Alpha;
            }
            // the document says COLCHA_ENA and TRANS_ENA should not
            // set at the same time, but if we set them both, which
            // one would process by hardware ? Check it.
            //
            // hardware can NOT work, software must keep it off
            //(if ((ai4DstColorComp[0] == ai4DstColorCompMin[3])&&(ai4SrcColorComp[0] == ai4SrcColorCompMin[3]))
            
            if ((u4SrcCM == u4DstCM) && ((UINT32)CM_RGB_CLUT8 == u4SrcCM))
            {
                if (u4TransEn)
                {
                    if (!u4DisDstKey && !u4DisSrcKey)//Dst Color Key
                    {
                        if (u4DstKeyIn && u4SrcKeyIn) //u4DstKeyIn==1 && u4SrcKeyIn==1
                        {
                            if ((ai4DstColorComp[0] == ai4DstColorCompMin[3])&&
                               (ai4SrcColorComp[0] == ai4SrcColorCompMin[3]))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                pu1Write += 1;
                            }
                        }
                        else if (u4DstKeyIn)//u4DstKeyIn==1 && u4SrcKeyIn==0
                        {
                            if ((ai4DstColorComp[0] == ai4DstColorCompMin[3])&&
                               !(ai4SrcColorComp[0] == ai4SrcColorCompMin[3]))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // keep dst color
                                pu1Write += 1;
                            }
                        }
                        else if (u4SrcKeyIn)//u4SrcKeyIn == 1 && u4DstKeyIn == 0
                        {
                            if (!(ai4DstColorComp[0] == ai4DstColorCompMin[3])&&
                               (ai4SrcColorComp[0] == ai4SrcColorCompMin[3]))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // keep dst color
                                pu1Write += 1;
                            }
                        }
                        else//u4SrcKeyIn == 0 && u4DstKeyIn == 0
                        {
                            if (!(ai4DstColorComp[0] == ai4DstColorCompMin[3])&&
                               !(ai4SrcColorComp[0] == ai4SrcColorCompMin[3]))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // keep dst color
                                pu1Write += 1;
                            }
                        }
                    }
                    else if (!u4DisDstKey)//u4DisDstKey==0 && u4DisSrcKey==1
                    {
                        if (u4DstKeyIn)
                        {
                            if (ai4DstColorComp[0] == ai4DstColorCompMin[3])
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // keep dst color
                                pu1Write += 1;
                            }
                            
                        }
                        else 
                        {
                            if (ai4DstColorComp[0] == ai4DstColorCompMin[3])
                            {
                                // keep dst color
                                pu1Write += 1;
                            }
                            else
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            
                        }
                    }
                    else if (!u4DisSrcKey)//u4DisDstKey==1 && u4DisSrcKey==0
                    {
                        if (u4SrcKeyIn)
                        {
                            if (ai4SrcColorComp[0] == ai4SrcColorCompMin[3])
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // keep dst color
                                pu1Write += 1;
                            }
                        }
                        else    // u4SrcKeyIn = 0
                        {
                            if (ai4SrcColorComp[0] == ai4SrcColorCompMin[3])
                            {
                                pu1Write += 1;
                            }
                            else
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                        }
                    }
                    else//u4DisSrcKey== 1 && u4DisDstKey == 1
                    {
                        GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4SrcColorComp);
                    }
                }
                else if (u4ColchgEn)
                {
                    if (!u4DisDstKey && !u4DisSrcKey)//Dst Color Key
                    {
                        if (u4DstKeyIn && u4SrcKeyIn) //u4DstKeyIn==1 && u4SrcKeyIn==1
                        {
                            if ((ai4DstColorComp[0] == ai4DstColorCompMin[3])&&
                               (ai4SrcColorComp[0] == ai4SrcColorCompMin[3]))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                        }
                        else if (u4DstKeyIn)//u4DstKeyIn==1 && u4SrcKeyIn==0
                        {
                            if ((ai4DstColorComp[0] == ai4DstColorCompMin[3])&&
                               !(ai4SrcColorComp[0] == ai4SrcColorCompMin[3]))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                        }
                        else if (u4SrcKeyIn)//u4SrcKeyIn == 1 && u4DstKeyIn == 0
                        {
                            if (!(ai4DstColorComp[0] == ai4DstColorCompMin[3])&&
                               (ai4SrcColorComp[0] == ai4SrcColorCompMin[3]))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                        }
                        else//u4SrcKeyIn == 0 && u4DstKeyIn == 0
                        {
                            if (!(ai4DstColorComp[0] == ai4DstColorCompMin[3])&&
                               !(ai4SrcColorComp[0] == ai4SrcColorCompMin[3]))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                        }
                    }
                    else if (!u4DisDstKey)//u4DisDstKey==0 && u4DisSrcKey==1
                    {
                        if (u4DstKeyIn)
                        {
                            if (ai4DstColorComp[0] == ai4DstColorCompMin[3])
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                            
                        }
                        else 
                        {
                            if (ai4DstColorComp[0] == ai4DstColorCompMin[3])
                            
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                            else
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            
                        }
                    }
                    else if (!u4DisSrcKey)//u4DisDstKey==1 && u4DisSrcKey==0
                    {
                        if (u4SrcKeyIn)
                        {
                            if (ai4SrcColorComp[0] == ai4SrcColorCompMin[3])
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                        }
                        else    // u4SrcKeyIn = 0
                        {
                            if (ai4SrcColorComp[0] == ai4SrcColorCompMin[3])
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                            else
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                        }
                    }
                    else//u4DisSrcKey== 1 && u4DisDstKey == 1
                    {
                        GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4SrcColorComp);
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4SrcColorComp);
                }
            }    
            else if((u4SrcCM == u4DstCM) && ((UINT32)CM_RGB565_DIRECT16 == u4SrcCM))// 32bpp and 16bpp
            {
                if (u4TransEn)
                {
                    if (!u4DisDstKey && !u4DisSrcKey)//Dst Color Key
                    {
                        if (u4DstKeyIn && u4SrcKeyIn) //u4DstKeyIn==1 && u4SrcKeyIn==1
                        {
                            if ((
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))&&
                               (
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3])))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                            // keep dst color
                                if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                                {
                                    pu1Write += 4;
                                }
                                else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                                {
                                    pu1Write += 2;
                                }
                                else    // for lint happy
                                {
                                    ;
                                }
                            }
                        }
                        else if (u4DstKeyIn)//u4DstKeyIn==1 && u4SrcKeyIn==0
                        {
                            if ((
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))&&
                               !(
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3])))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                            // keep dst color
                                if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                                {
                                    pu1Write += 4;
                                }
                                else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                                {
                                    pu1Write += 2;
                                }
                                else    // for lint happy
                                {
                                    ;
                                }
                            }
                        }
                        else if (u4SrcKeyIn)//u4SrcKeyIn == 1 && u4DstKeyIn == 0
                        {
                            if (!(
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))&&
                               (
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3])))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                            // keep dst color
                                if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                                {
                                    pu1Write += 4;
                                }
                                else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                                {
                                    pu1Write += 2;
                                }
                                else    // for lint happy
                                {
                                    ;
                                }
                            }
                        }
                        else//u4SrcKeyIn == 0 && u4DstKeyIn == 0
                        {
                            if (!(
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))&&
                               !(
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3])))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                            // keep dst color
                                if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                                {
                                    pu1Write += 4;
                                }
                                else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                                {
                                    pu1Write += 2;
                                }
                                else    // for lint happy
                                {
                                    ;
                                }
                            }
                        }
                    }
                    else if (!u4DisDstKey)//u4DisDstKey==0 && u4DisSrcKey==1
                    {
                        if (u4DstKeyIn)
                        {
                            if (
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // keep dst color
                                if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                                {
                                    pu1Write += 4;
                                }
                                else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                                {
                                    pu1Write += 2;
                                }
                                else    // for lint happy
                                {
                                    ;
                                }
                            }
                            
                        }
                        else 
                        {
                            if (
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))
                            {
                                // keep dst color
                                if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                                {
                                    pu1Write += 4;
                                }
                                else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                                {
                                    pu1Write += 2;
                                }
                                else    // for lint happy
                                {
                                    ;
                                }
                            }
                            else
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            
                        }
                    }
                    else if (!u4DisSrcKey)//u4DisDstKey==1 && u4DisSrcKey==0
                    {
                        if (u4SrcKeyIn)
                        {
                            if (
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3]))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // keep dst color
                                if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                                {
                                    pu1Write += 4;
                                }
                                else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                                {
                                    pu1Write += 2;
                                }
                                else    // for lint happy
                                {
                                    ;
                                }
                            }
                        }
                        else    // u4SrcKeyIn = 0
                        {
                            if (
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3]))
                            {
                                // keep dst color
                                if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                                {
                                    pu1Write += 4;
                                }
                                else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                                {
                                    pu1Write += 2;
                                }
                                else    // for lint happy
                                {
                                    ;
                                }
                            }
                            else
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                        }
                    }
                    else//u4DisSrcKey== 1 && u4DisDstKey == 1
                    {
                        GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4SrcColorComp);
                    }
                }
                else if (u4ColchgEn)
                {
                    if (!u4DisDstKey && !u4DisSrcKey)//Dst Color Key
                    {
                        if (u4DstKeyIn && u4SrcKeyIn) //u4DstKeyIn==1 && u4SrcKeyIn==1
                        {
                            if ((
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))&&
                               (

                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3])))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                        }
                        else if (u4DstKeyIn)//u4DstKeyIn==1 && u4SrcKeyIn==0
                        {
                            if ((
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))&&
                               !(
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3])))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                        }
                        else if (u4SrcKeyIn)//u4SrcKeyIn == 1 && u4DstKeyIn == 0
                        {
                            if (!(
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))&&
                               (
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3])))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                        }
                        else//u4SrcKeyIn == 0 && u4DstKeyIn == 0
                        {
                            if (!(
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))&&
                               !(
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3])))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                        }
                    }
                    else if (!u4DisDstKey)//u4DisDstKey==0 && u4DisSrcKey==1
                    {
                        if (u4DstKeyIn)
                        {
                            if (
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                            
                        }
                        else 
                        {
                            if (
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))
                            
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                            else
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            
                        }
                    }
                    else if (!u4DisSrcKey)//u4DisDstKey==1 && u4DisSrcKey==0
                    {
                        if (u4SrcKeyIn)
                        {
                            if (
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3]))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                        }
                        else    // u4SrcKeyIn = 0
                        {
                            if (
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3]))
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                            else
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                        }
                    }
                    else//u4DisSrcKey== 1 && u4DisDstKey == 1
                    {
                        GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4SrcColorComp);
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4SrcColorComp);
                }
            }
            else // 32bpp and 16bpp
            {
                if (u4TransEn)
                {
                    if (!u4DisDstKey && !u4DisSrcKey)//Dst Color Key
                    {
                        if (u4DstKeyIn && u4SrcKeyIn) //u4DstKeyIn==1 && u4SrcKeyIn==1
                        {
                            if ((
                                (ai4DstColorComp[0] >= ai4DstColorCompMin[0]) &&
                                (ai4DstColorComp[0] <= ai4DstColorCompMax[0]) &&
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))&&
                               ((ai4SrcColorComp[0] >= ai4SrcColorCompMin[0]) &&
                                (ai4SrcColorComp[0] <= ai4SrcColorCompMax[0]) &&
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3])))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                            // keep dst color
                                if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                                {
                                    pu1Write += 4;
                                }
                                else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                                {
                                    pu1Write += 2;
                                }
                                else    // for lint happy
                                {
                                    ;
                                }
                            }
                        }
                        else if (u4DstKeyIn)//u4DstKeyIn==1 && u4SrcKeyIn==0
                        {
                            if ((
                                (ai4DstColorComp[0] >= ai4DstColorCompMin[0]) &&
                                (ai4DstColorComp[0] <= ai4DstColorCompMax[0]) &&
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))&&
                               !((ai4SrcColorComp[0] >= ai4SrcColorCompMin[0]) &&
                                (ai4SrcColorComp[0] <= ai4SrcColorCompMax[0]) &&
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3])))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                            // keep dst color
                                if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                                {
                                    pu1Write += 4;
                                }
                                else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                                {
                                    pu1Write += 2;
                                }
                                else    // for lint happy
                                {
                                    ;
                                }
                            }
                        }
                        else if (u4SrcKeyIn)//u4SrcKeyIn == 1 && u4DstKeyIn == 0
                        {
                            if (!(
                                (ai4DstColorComp[0] >= ai4DstColorCompMin[0]) &&
                                (ai4DstColorComp[0] <= ai4DstColorCompMax[0]) &&
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))&&
                               ((ai4SrcColorComp[0] >= ai4SrcColorCompMin[0]) &&
                                (ai4SrcColorComp[0] <= ai4SrcColorCompMax[0]) &&
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3])))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                            // keep dst color
                                if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                                {
                                    pu1Write += 4;
                                }
                                else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                                {
                                    pu1Write += 2;
                                }
                                else    // for lint happy
                                {
                                    ;
                                }
                            }
                        }
                        else//u4SrcKeyIn == 0 && u4DstKeyIn == 0
                        {
                            if (!(
                                (ai4DstColorComp[0] >= ai4DstColorCompMin[0]) &&
                                (ai4DstColorComp[0] <= ai4DstColorCompMax[0]) &&
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))&&
                               !((ai4SrcColorComp[0] >= ai4SrcColorCompMin[0]) &&
                                (ai4SrcColorComp[0] <= ai4SrcColorCompMax[0]) &&
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3])))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                            // keep dst color
                                if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                                {
                                    pu1Write += 4;
                                }
                                else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                                {
                                    pu1Write += 2;
                                }
                                else    // for lint happy
                                {
                                    ;
                                }
                            }
                        }
                    }
                    else if (!u4DisDstKey)//u4DisDstKey==0 && u4DisSrcKey==1
                    {
                        if (u4DstKeyIn)
                        {
                            if (
                                (ai4DstColorComp[0] >= ai4DstColorCompMin[0]) &&
                                (ai4DstColorComp[0] <= ai4DstColorCompMax[0]) &&
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // keep dst color
                                if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                                {
                                    pu1Write += 4;
                                }
                                else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                                {
                                    pu1Write += 2;
                                }
                                else    // for lint happy
                                {
                                    ;
                                }
                            }
                            
                        }
                        else 
                        {
                            if (
                                (ai4DstColorComp[0] >= ai4DstColorCompMin[0]) &&
                                (ai4DstColorComp[0] <= ai4DstColorCompMax[0]) &&
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))
                            {
                                // keep dst color
                                if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                                {
                                    pu1Write += 4;
                                }
                                else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                                {
                                    pu1Write += 2;
                                }
                                else    // for lint happy
                                {
                                    ;
                                }
                            }
                            else
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            
                        }
                    }
                    else if (!u4DisSrcKey)//u4DisDstKey==1 && u4DisSrcKey==0
                    {
                        if (u4SrcKeyIn)
                        {
                            if ((ai4SrcColorComp[0] >= ai4SrcColorCompMin[0]) &&
                                (ai4SrcColorComp[0] <= ai4SrcColorCompMax[0]) &&
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3]))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // keep dst color
                                if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                                {
                                    pu1Write += 4;
                                }
                                else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                                {
                                    pu1Write += 2;
                                }
                                else    // for lint happy
                                {
                                    ;
                                }
                            }
                        }
                        else    // u4SrcKeyIn = 0
                        {
                            if ((ai4SrcColorComp[0] >= ai4SrcColorCompMin[0]) &&
                                (ai4SrcColorComp[0] <= ai4SrcColorCompMax[0]) &&
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3]))
                            {
                                // keep dst color
                                if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                                {
                                    pu1Write += 4;
                                }
                                else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                         ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                                {
                                    pu1Write += 2;
                                }
                                else    // for lint happy
                                {
                                    ;
                                }
                            }
                            else
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                        }
                    }
                    else//u4DisSrcKey== 1 && u4DisDstKey == 1
                    {
                        GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4SrcColorComp);
                    }
                }
                else if (u4ColchgEn)
                {
                    if (!u4DisDstKey && !u4DisSrcKey)//Dst Color Key
                    {
                        if (u4DstKeyIn && u4SrcKeyIn) //u4DstKeyIn==1 && u4SrcKeyIn==1
                        {
                            if ((
                                (ai4DstColorComp[0] >= ai4DstColorCompMin[0]) &&
                                (ai4DstColorComp[0] <= ai4DstColorCompMax[0]) &&
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))&&
                               ((ai4SrcColorComp[0] >= ai4SrcColorCompMin[0]) &&
                                (ai4SrcColorComp[0] <= ai4SrcColorCompMax[0]) &&
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3])))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                        }
                        else if (u4DstKeyIn)//u4DstKeyIn==1 && u4SrcKeyIn==0
                        {
                            if ((
                                (ai4DstColorComp[0] >= ai4DstColorCompMin[0]) &&
                                (ai4DstColorComp[0] <= ai4DstColorCompMax[0]) &&
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))&&
                               !((ai4SrcColorComp[0] >= ai4SrcColorCompMin[0]) &&
                                (ai4SrcColorComp[0] <= ai4SrcColorCompMax[0]) &&
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3])))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                        }
                        else if (u4SrcKeyIn)//u4SrcKeyIn == 1 && u4DstKeyIn == 0
                        {
                            if (!(
                                (ai4DstColorComp[0] >= ai4DstColorCompMin[0]) &&
                                (ai4DstColorComp[0] <= ai4DstColorCompMax[0]) &&
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))&&
                               ((ai4SrcColorComp[0] >= ai4SrcColorCompMin[0]) &&
                                (ai4SrcColorComp[0] <= ai4SrcColorCompMax[0]) &&
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3])))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                        }
                        else//u4SrcKeyIn == 0 && u4DstKeyIn == 0
                        {
                            if (!(
                                (ai4DstColorComp[0] >= ai4DstColorCompMin[0]) &&
                                (ai4DstColorComp[0] <= ai4DstColorCompMax[0]) &&
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))&&
                               !((ai4SrcColorComp[0] >= ai4SrcColorCompMin[0]) &&
                                (ai4SrcColorComp[0] <= ai4SrcColorCompMax[0]) &&
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3])))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                        }
                    }
                    else if (!u4DisDstKey)//u4DisDstKey==0 && u4DisSrcKey==1
                    {
                        if (u4DstKeyIn)
                        {
                            if (
                                (ai4DstColorComp[0] >= ai4DstColorCompMin[0]) &&
                                (ai4DstColorComp[0] <= ai4DstColorCompMax[0]) &&
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                            
                        }
                        else 
                        {
                            if (
                                (ai4DstColorComp[0] >= ai4DstColorCompMin[0]) &&
                                (ai4DstColorComp[0] <= ai4DstColorCompMax[0]) &&
                                (ai4DstColorComp[1] >= ai4DstColorCompMin[1]) &&
                                (ai4DstColorComp[1] <= ai4DstColorCompMax[1]) &&
                                (ai4DstColorComp[2] >= ai4DstColorCompMin[2]) &&
                                (ai4DstColorComp[2] <= ai4DstColorCompMax[2]) &&
                                (ai4DstColorComp[3] >= ai4DstColorCompMin[3]) &&
                                (ai4DstColorComp[3] <= ai4DstColorCompMax[3]))
                            
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                            else
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            
                        }
                    }
                    else if (!u4DisSrcKey)//u4DisDstKey==1 && u4DisSrcKey==0
                    {
                        if (u4SrcKeyIn)
                        {
                            if ((ai4SrcColorComp[0] >= ai4SrcColorCompMin[0]) &&
                                (ai4SrcColorComp[0] <= ai4SrcColorCompMax[0]) &&
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3]))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                            else
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                        }
                        else    // u4SrcKeyIn = 0
                        {
                            if ((ai4SrcColorComp[0] >= ai4SrcColorCompMin[0]) &&
                                (ai4SrcColorComp[0] <= ai4SrcColorCompMax[0]) &&
                                (ai4SrcColorComp[1] >= ai4SrcColorCompMin[1]) &&
                                (ai4SrcColorComp[1] <= ai4SrcColorCompMax[1]) &&
                                (ai4SrcColorComp[2] >= ai4SrcColorCompMin[2]) &&
                                (ai4SrcColorComp[2] <= ai4SrcColorCompMax[2]) &&
                                (ai4SrcColorComp[3] >= ai4SrcColorCompMin[3]) &&
                                (ai4SrcColorComp[3] <= ai4SrcColorCompMax[3]))
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                            else
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4SrcColorComp);
                            }
                        }
                    }
                    else//u4DisSrcKey== 1 && u4DisDstKey == 1
                    {
                        GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4SrcColorComp);
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4SrcColorComp);
                }
            }
        }

        pu1SrcLine += u4SrcPitch;
        pu1DstLine += u4DstPitch;
    }

    return (INT32)E_GFX_OK;
}

#else
INT32 GFX_SwBitBlt(UINT8 *pu1SrcBase, UINT8 *pu1DstBase, UINT32 u4SrcPitch,
    UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM, UINT32 u4Width,
    UINT32 u4Height, UINT32 u4ColorMin, UINT32 u4ColorMax, UINT32 u4TransEn,
    UINT32 u4ColchgEn, UINT32 u4KeynotEn, UINT32 u4Color)
{
    INT32 ai4ColorCompMin[4] = {0};
    INT32 ai4ColorCompMax[4] = {0};
    INT32 ai4ColorComp[4] = {0};
    INT32 ai4RectColor[4] = {0};
    UINT32 *pu4Color;
    UINT32 x, y;
    UINT8 *pu1SrcLine, *pu1DstLine;
    UINT8 *pu1Read, *pu1Write;

    // protection
    if (u4TransEn && u4ColchgEn)
    {
        return -(INT32)E_GFX_INV_ARG;
    }
    //HalFlushDCache();
    //HalInvalidateDCache();
    ////HalFlushInvalidateDCache();
   // BIM_WAIT_WALE();
    pu1Read     = pu1SrcBase;
    pu1SrcLine  = pu1SrcBase;
    pu1Write    = pu1DstBase;
    pu1DstLine  = pu1DstBase;

    if (u4TransEn || u4ColchgEn)
    {
        ai4ColorCompMin[0] = (INT32) (u4ColorMin >> 24);
        ai4ColorCompMin[1] = (INT32)((u4ColorMin >> 16) & 0xFF);
        ai4ColorCompMin[2] = (INT32)((u4ColorMin >>  8) & 0xFF);
        ai4ColorCompMin[3] = (INT32)( u4ColorMin        & 0xFF);

        ai4ColorCompMax[0] = (INT32) (u4ColorMax >> 24);
        ai4ColorCompMax[1] = (INT32)((u4ColorMax >> 16) & 0xFF);
        ai4ColorCompMax[2] = (INT32)((u4ColorMax >>  8) & 0xFF);
        ai4ColorCompMax[3] = (INT32)( u4ColorMax        & 0xFF);

        pu4Color = &u4Color;
        u4Color  = _GfxSwColorExpansion(u4Color, u4DstCM);

        GFX_SwGetColorComponent((UINT8 **)&pu4Color, u4DstCM, ai4RectColor);
    }

    for (y = 0; y < u4Height; y++)
    {
        pu1Read  = pu1SrcLine;
        pu1Write = pu1DstLine;

        for (x = 0; x < u4Width; x++)
        {
            GFX_SwGetColorComponent(&pu1Read, u4SrcCM, ai4ColorComp);

            // the document says COLCHA_ENA and TRANS_ENA should not
            // set at the same time, but if we set them both, which
            // one would process by hardware ? Check it.
            //
            // hardware can NOT work, software must keep it off
            //
            if ((u4SrcCM == u4DstCM) && ((UINT32)CM_RGB_CLUT8 == u4SrcCM))
            {
                if (u4TransEn)
                {
                    if (u4KeynotEn)
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // keep dst color
                            pu1Write += 1;
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // keep dst color
                            pu1Write += 1;
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else if (u4ColchgEn)
                {
                    if (u4KeynotEn)
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4ColorComp);
                }

            }
            else if((u4SrcCM == u4DstCM) && ((UINT32)CM_RGB565_DIRECT16 == u4SrcCM))// 32bpp and 16bpp
            {
                if (u4TransEn)
                {
                    if (u4KeynotEn)
                    {
                        if (
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                pu1Write += 4;
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                pu1Write += 2;
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        if ((ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                pu1Write += 4;
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                pu1Write += 2;
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else if (u4ColchgEn)
                {
                    if (u4KeynotEn)
                    {
                        if ((ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        if ((ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4ColorComp);
                }
            }
            else // 32bpp and 16bpp
            {
                if (u4TransEn)
                {
                    if (u4KeynotEn)
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                pu1Write += 4;
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                pu1Write += 2;
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                pu1Write += 4;
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                pu1Write += 2;
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else if (u4ColchgEn)
                {
                    if (u4KeynotEn)
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4ColorComp);
                }
            }
        }

        pu1SrcLine += u4SrcPitch;
        pu1DstLine += u4DstPitch;
    }

    return (INT32)E_GFX_OK;
}
/*lint -restore */
//add by msz00441 080604
#endif


//-------------------------------------------------------------------------
/** GFX_24Bpp_SwBitBlt
 *  gfx_24bpp normal bitblt
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e740 -e613 */
INT32 GFX_24Bpp_SwBitBlt(UINT8 *pu1SrcBase, UINT8 *pu1DstBase, UINT32 u4SrcPitch,
    UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM, UINT32 u4Width,
    UINT32 u4Height, UINT32 u4ColorMin, UINT32 u4ColorMax, UINT32 u4TransEn,
    UINT32 u4ColchgEn, UINT32 u4SrcKeyNotIn, UINT32 u4Color, UINT32 u4GAlphaEn, UINT32 u4GAlpha)
{
    INT32 ai4ColorCompMin[4] = {0};
    INT32 ai4ColorCompMax[4] = {0};
    INT32 ai4ColorComp[4] = {0};
    INT32 ai4RectColor[4] = {0};
    UINT32 *pu4Color;
    UINT32 x, y;
    UINT8 *pu1SrcLine, *pu1DstLine;
    UINT8 *pu1Read, *pu1Write;
    //, UINT32 u4GAlphaEn, UINT32 u4GAlpha
    // protection
    if (u4TransEn && u4ColchgEn)
    {
        return -(INT32)E_GFX_INV_ARG;
    }
    //HalFlushDCache();
    //HalInvalidateDCache();
    HalFlushInvalidateDCache();
    // BIM_WAIT_WALE();
    pu1Read     = pu1SrcBase;
    pu1SrcLine  = pu1SrcBase;
    pu1Write    = pu1DstBase;
    pu1DstLine  = pu1DstBase;

    if (u4TransEn || u4ColchgEn)
    {
        ai4ColorCompMin[0] = (INT32) (u4ColorMin >> 24);
        ai4ColorCompMin[1] = (INT32)((u4ColorMin >> 16) & 0xFF);
        ai4ColorCompMin[2] = (INT32)((u4ColorMin >>  8) & 0xFF);
        ai4ColorCompMin[3] = (INT32)( u4ColorMin        & 0xFF);

        ai4ColorCompMax[0] = (INT32) (u4ColorMax >> 24);
        ai4ColorCompMax[1] = (INT32)((u4ColorMax >> 16) & 0xFF);
        ai4ColorCompMax[2] = (INT32)((u4ColorMax >>  8) & 0xFF);
        ai4ColorCompMax[3] = (INT32)( u4ColorMax        & 0xFF);

        pu4Color = &u4Color;
        u4Color  = _GfxSwColorExpansion(u4Color, u4DstCM);

        GFX_SwGetColorComponent((UINT8 **)&pu4Color, u4DstCM, ai4RectColor);
    }

    for (y = 0; y < u4Height; y++)
    {
        pu1Read  = pu1SrcLine;
        pu1Write = pu1DstLine;

        for (x = 0; x < u4Width; x++)
        {
            GFX_SwGetColorComponent(&pu1Read, u4SrcCM, ai4ColorComp);

            // the document says COLCHA_ENA and TRANS_ENA should not
            // set at the same time, but if we set them both, which
            // one would process by hardware ? Check it.
            //
            // hardware can NOT work, software must keep it off
            //
            if ((u4SrcCM == CM_RGB888_DIRECT24) && (u4GAlphaEn))
            {
                ai4ColorComp[0] = u4GAlpha;
                ai4RectColor[0] = u4GAlpha;
            }
            
            {
                if (u4TransEn)
                {
                    if (u4SrcKeyNotIn) // Key In
                    {
                        if ((ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write src color to dst
                            
                            if ((u4SrcCM == CM_RGB888_DIRECT24) && (u4GAlphaEn))
                            {
                                ai4ColorComp[0] = u4GAlpha;
                            }
                            GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                pu1Write += 4;
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                pu1Write += 2;
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                    }
                    else    // !u4SrcKeyNotIn = 0
                    {
                        if ((ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3])) // Key In
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                pu1Write += 4;
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                pu1Write += 2;
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                        else
                        {
                            
                            if ((u4SrcCM == CM_RGB888_DIRECT24) && (u4GAlphaEn))
                            {
                                ai4ColorComp[0] = u4GAlpha;
                            }
                            // write src color to dst
                            GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else if (u4ColchgEn)
                {
                    if (u4SrcKeyNotIn)
                    {
                        if ((ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            
                            if ((u4SrcCM == CM_RGB888_DIRECT24) && (u4GAlphaEn))
                            {
                                ai4ColorComp[0] = u4GAlpha;
                            }
                            // write src color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            
                            if ((u4SrcCM == CM_RGB888_DIRECT24) && (u4GAlphaEn))
                            {
                                ai4RectColor[0] = u4GAlpha;
                            }
                            // write rect color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                    }
                    else    // !u4SrcKeyNotIn = 0
                    {
                        if ((ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            if ((u4SrcCM == CM_RGB888_DIRECT24) && (u4GAlphaEn))
                            {
                                ai4RectColor[0] = u4GAlpha;
                            }
                            // write rect color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                        else
                        {
                            
                            if ((u4SrcCM == CM_RGB888_DIRECT24) && (u4GAlphaEn))
                            {
                                ai4ColorComp[0] = u4GAlpha;
                            }
                            // write src color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    
                    if ((u4SrcCM == CM_RGB888_DIRECT24) && (u4GAlphaEn))
                    {
                        ai4ColorComp[0] = u4GAlpha;
                    }
                    GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4ColorComp);
                }
            }
        }

        pu1SrcLine += u4SrcPitch;
        pu1DstLine += u4DstPitch;
    }

    return (INT32)E_GFX_OK;
}
/*lint -restore */


#if 1//(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8561)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT8563)||(CONFIG_CHIP_VER_CURR == CONFIG_CHIP_VER_MT3363)
//-------------------------------------------------------------------------
/** GFX_SwBitBlt_NewMethod
 *  gfx normal bitblt
 *
 */
//-------------------------------------------------------------------------
INT32 GFX_SwBitBlt_NewMethod(UINT8 *pu1SrcBase, UINT8 *pu1DstBase, UINT32 u4SrcPitch,
    UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM, UINT32 u4Width,
    UINT32 u4Height, UINT32 u4ColorMin, UINT32 u4ColorMax, UINT32 u4TransEn,
    UINT32 u4ColchgEn, UINT32 u4KeynotEn, UINT32 u4Color)
{
    INT32 ai4ColorCompMin[4] = {0};
    INT32 ai4ColorCompMax[4] = {0};
    INT32 ai4ColorComp[4] = {0};
    INT32 ai4RectColor[4] = {0};
    UINT32 x, y;
    UINT8 *pu1SrcLine, *pu1DstLine;
    UINT8 *pu1Read, *pu1Write;

    // protection
    if (u4TransEn && u4ColchgEn)
    {
        return -(INT32)E_GFX_INV_ARG;
    }
    if (u4ColorMin != u4ColorMax && CM_RGB_CLUT8 == u4SrcCM)
    {
        UTIL_Printf("[GFX_SwBitBlt_NewMethod] ERROR: colormin != colormax in CLUT8 mode\n");
        return -(INT32)E_GFX_INV_ARG;
    }

    pu1Read     = pu1SrcBase;
    pu1SrcLine  = pu1SrcBase;
    pu1Write    = pu1DstBase;
    pu1DstLine  = pu1DstBase;

    if (u4TransEn || u4ColchgEn)
    {
        switch (u4SrcCM)
        {
            case CM_RGB_CLUT8:
                ai4ColorCompMin[0] = ai4ColorCompMax[0] = u4ColorMin & 0xff;
                ai4ColorCompMin[1] = ai4ColorCompMax[1] = u4ColorMin & 0xff;
                ai4ColorCompMin[2] = ai4ColorCompMax[2] = u4ColorMin & 0xff;
                ai4ColorCompMin[3] = ai4ColorCompMax[3] = u4ColorMin & 0xff;

                ai4RectColor[0] = ai4RectColor[1] = ai4RectColor[2] = ai4RectColor[3] = u4Color & 0xff;
                break;

            case CM_RGB565_DIRECT16:
                ai4ColorCompMin[0] = 0xff;                      // A0
                ai4ColorCompMin[1] = (u4ColorMin >> 11) & 0x1f; // R5
                ai4ColorCompMin[2] = (u4ColorMin >> 5) & 0x3f;  // G6
                ai4ColorCompMin[3] = u4ColorMin & 0x1f;         // B5

                ai4ColorCompMax[0] = 0xff;                      // A0
                ai4ColorCompMax[1] = (u4ColorMax >> 11) & 0x1f; // R5
                ai4ColorCompMax[2] = (u4ColorMax >> 5) & 0x3f;  // G6
                ai4ColorCompMax[3] = u4ColorMax & 0x1f;         // B5

                ai4RectColor[0] = 0xff;
                ai4RectColor[1] = (u4Color >> 11) & 0x1f; // R5
                ai4RectColor[2] = (u4Color >> 5) & 0x3f;  // G6
                ai4RectColor[3] = u4Color & 0x1f;         // B5
                break;

            case CM_ARGB1555_DIRECT16:
                ai4ColorCompMin[0] = (u4ColorMin >> 15) & 0x1;  // A1
                ai4ColorCompMin[1] = (u4ColorMin >> 10) & 0x1f; // R5
                ai4ColorCompMin[2] = (u4ColorMin >> 5) & 0x1f;  // G5
                ai4ColorCompMin[3] = u4ColorMin & 0x1f;         // B5

                ai4ColorCompMax[0] = (u4ColorMax >> 15) & 0x1;  // A1
                ai4ColorCompMax[1] = (u4ColorMax >> 10) & 0x1f; // R5
                ai4ColorCompMax[2] = (u4ColorMax >> 5) & 0x1f;  // G5
                ai4ColorCompMax[3] = u4ColorMax & 0x1f;         // B5

                ai4RectColor[0] = (u4Color >> 15) & 0x1;  // A1
                ai4RectColor[1] = (u4Color >> 10) & 0x1f; // R5
                ai4RectColor[2] = (u4Color >> 5) & 0x1f;  // G5
                ai4RectColor[3] = u4Color & 0x1f;         // B5
                break;

            case CM_ARGB4444_DIRECT16:
                ai4ColorCompMin[0] = (u4ColorMin >> 12) & 0xf;  // A4
                ai4ColorCompMin[1] = (u4ColorMin >> 8) & 0xf;   // R4
                ai4ColorCompMin[2] = (u4ColorMin >> 4) & 0xf;   // G4
                ai4ColorCompMin[3] = u4ColorMin & 0xf;          // B4

                ai4ColorCompMax[0] = (u4ColorMax >> 12) & 0xf;  // A4
                ai4ColorCompMax[1] = (u4ColorMax >> 8) & 0xf;   // R4
                ai4ColorCompMax[2] = (u4ColorMax >> 4) & 0xf;   // G4
                ai4ColorCompMax[3] = u4ColorMax & 0xf;          // B4

                ai4RectColor[0] = (u4Color >> 12) & 0xf;  // A4
                ai4RectColor[1] = (u4Color >> 8) & 0xf;   // R4
                ai4RectColor[2] = (u4Color >> 4) & 0xf;   // G4
                ai4RectColor[3] = u4Color & 0xf;          // B4
                break;

            case CM_ARGB8888_DIRECT32:
                ai4ColorCompMin[0] = (u4ColorMin >> 24);
                ai4ColorCompMin[1] = ((u4ColorMin >> 16) & 0xff);
                ai4ColorCompMin[2] = ((u4ColorMin >>  8) & 0xff);
                ai4ColorCompMin[3] = ( u4ColorMin        & 0xff);

                ai4ColorCompMax[0] = (u4ColorMax >> 24);
                ai4ColorCompMax[1] = ((u4ColorMax >> 16) & 0xff);
                ai4ColorCompMax[2] = ((u4ColorMax >>  8) & 0xff);
                ai4ColorCompMax[3] = ( u4ColorMax        & 0xff);

                ai4RectColor[0] = (u4Color >> 24);
                ai4RectColor[1] = ((u4Color >> 16) & 0xff);
                ai4RectColor[2] = ((u4Color >>  8) & 0xff);
                ai4RectColor[3] = ( u4Color        & 0xff);
                break;

            default:
                return -(INT32)E_GFX_INV_ARG;
        }
    }
    UTIL_Printf("ai4ColorCompMin{0x%x,0x%x,0x%x,0x%x}ai4ColorCompMax{0x%x,0x%x,0x%x,0x%x}ai4RectColor{0x%x,0x%x,0x%x,0x%x}\n",
                ai4ColorCompMin[0],ai4ColorCompMin[1],ai4ColorCompMin[2],ai4ColorCompMin[3],
                ai4ColorCompMax[0],ai4ColorCompMax[1],ai4ColorCompMax[2],ai4ColorCompMax[3],
                ai4RectColor[0],ai4RectColor[1],ai4RectColor[2],ai4RectColor[3]);

    for (y = 0; y < u4Height; y++)
    {
        pu1Read  = pu1SrcLine;
        pu1Write = pu1DstLine;

        for (x = 0; x < u4Width; x++)
        {
            GFX_SwGetColorComponentEx(&pu1Read, u4SrcCM, ai4ColorComp);

            // the document says COLCHA_ENA and TRANS_ENA should not
            // set at the same time, but if we set them both, which
            // one would process by hardware ? Check it.
            //
            // hardware can NOT work, software must keep it off
            //
            if ((u4SrcCM == u4DstCM) && ((UINT32)CM_RGB_CLUT8 == u4SrcCM))
            {
                if (u4TransEn)
                {
                    if (u4KeynotEn)
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[3] == ai4ColorCompMin[3])
                        {
                            // write src color to dst
                            GFX_SwSetColorComponentEx(&pu1Write, u4DstCM, ai4ColorComp);
                        }
                        else
                        {
                            // keep dst color
                            pu1Write += 1;
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[3] == ai4ColorCompMin[3])
                        {
                            // keep dst color
                            pu1Write += 1;
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponentEx(&pu1Write, u4DstCM, ai4ColorComp);
                        }
                    }
                }
                else if (u4ColchgEn)
                {
                    if (u4KeynotEn)
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[3] == ai4ColorCompMin[3])
                        {
                            // write src color to dst
                            GFX_SwSetColorComponentEx((UINT8 **)&pu1Write, u4DstCM, ai4ColorComp);
                        }
                        else
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponentEx((UINT8 **)&pu1Write, u4DstCM, ai4RectColor);
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[3] == ai4ColorCompMin[3])
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponentEx((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponentEx((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    GFX_SwSetColorComponentEx(&pu1Write, u4DstCM, ai4ColorComp);
                }

            }
            else if((u4SrcCM == u4DstCM) && ((UINT32)CM_RGB565_DIRECT16 == u4SrcCM))// 32bpp and 16bpp
            {
                if (u4TransEn)
                {
                    if (u4KeynotEn)
                    {
                        if (
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write src color to dst
                            GFX_SwSetColorComponentEx(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                pu1Write += 4;
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                pu1Write += 2;
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        if ((ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                pu1Write += 4;
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                pu1Write += 2;
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponentEx(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else if (u4ColchgEn)
                {
                    if (u4KeynotEn)
                    {
                        if ((ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write src color to dst
                            GFX_SwSetColorComponentEx((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponentEx((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        if ((ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponentEx((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponentEx((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    GFX_SwSetColorComponentEx(&pu1Write, u4DstCM, ai4ColorComp);
                }
            }
            else // 32bpp and 16bpp
            {
                if (u4TransEn)
                {
                    if (u4KeynotEn)
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write src color to dst
                            GFX_SwSetColorComponentEx(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                pu1Write += 4;
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                pu1Write += 2;
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                pu1Write += 4;
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                pu1Write += 2;
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponentEx(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else if (u4ColchgEn)
                {
                    if (u4KeynotEn)
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write src color to dst
                            GFX_SwSetColorComponentEx((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponentEx((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponentEx((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponentEx((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    GFX_SwSetColorComponentEx(&pu1Write, u4DstCM, ai4ColorComp);
                }
            }
        }

        pu1SrcLine += u4SrcPitch;
        pu1DstLine += u4DstPitch;
    }

    return (INT32)E_GFX_OK;
}
#endif

//-------------------------------------------------------------------------
/** GFX_SwBitBlt
 *  gfx normal bitblt
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e740 -e613 */
INT32 GFX_SwBitBlt_Clip(UINT8 *pu1SrcBase, UINT8 *pu1DstBase, UINT32 u4SrcPitch,
    UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM, UINT32 u4Width,
    UINT32 u4Height, UINT32 u4ColorMin, UINT32 u4ColorMax, UINT32 u4TransEn,
    UINT32 u4ColchgEn, UINT32 u4KeynotEn, UINT32 u4Color, GFX_CLIP_PARA_T *prClipPara)
{
    INT32 ai4ColorCompMin[4] = {0};
    INT32 ai4ColorCompMax[4] = {0};
    INT32 ai4ColorComp[4] = {0};
    INT32 ai4RectColor[4] = {0};
    UINT32 *pu4Color;
    UINT32 x, y;
    UINT8 *pu1SrcLine, *pu1DstLine;
    UINT8 *pu1Read, *pu1Write;

    // protection
    if (u4TransEn && u4ColchgEn)
    {
        return -(INT32)E_GFX_INV_ARG;
    }
    //HalFlushDCache();
    //HalInvalidateDCache();
    //HalFlushInvalidateDCache();
   // BIM_WAIT_WALE();
    pu1Read     = pu1SrcBase;
    pu1SrcLine  = pu1SrcBase;
    pu1Write    = pu1DstBase;
    pu1DstLine  = pu1DstBase;

    if (u4TransEn || u4ColchgEn)
    {
        ai4ColorCompMin[0] = (INT32) (u4ColorMin >> 24);
        ai4ColorCompMin[1] = (INT32)((u4ColorMin >> 16) & 0xFF);
        ai4ColorCompMin[2] = (INT32)((u4ColorMin >>  8) & 0xFF);
        ai4ColorCompMin[3] = (INT32)( u4ColorMin        & 0xFF);

        ai4ColorCompMax[0] = (INT32) (u4ColorMax >> 24);
        ai4ColorCompMax[1] = (INT32)((u4ColorMax >> 16) & 0xFF);
        ai4ColorCompMax[2] = (INT32)((u4ColorMax >>  8) & 0xFF);
        ai4ColorCompMax[3] = (INT32)( u4ColorMax        & 0xFF);

        pu4Color = &u4Color;
        u4Color  = _GfxSwColorExpansion(u4Color, u4DstCM);

        GFX_SwGetColorComponent((UINT8 **)&pu4Color, u4DstCM, ai4RectColor);
    }

    for (y = 0; y < u4Height; y++)
    {
        pu1Read  = pu1SrcLine;
        pu1Write = pu1DstLine;

        for (x = 0; x < u4Width; x++)
        {
            GFX_SwGetColorComponent(&pu1Read, u4SrcCM, ai4ColorComp);

            // the document says COLCHA_ENA and TRANS_ENA should not
            // set at the same time, but if we set them both, which
            // one would process by hardware ? Check it.
            //
            // hardware can NOT work, software must keep it off
            //
            if ((u4SrcCM == u4DstCM) && ((UINT32)CM_RGB_CLUT8 == u4SrcCM))
            {
                if (u4TransEn)
                {
                    if (u4KeynotEn)
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // write src color to dst
                            if (fgGfxWithinClip(x, y, prClipPara))
                            {
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4ColorComp);
                            }
                            else
                            {
                                pu1Write += 1;
                            }
                        }
                        else
                        {
                            // keep dst color
                            pu1Write += 1;
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // keep dst color
                            pu1Write += 1;
                        }
                        else
                        {
                            // write src color to dst
                            if (fgGfxWithinClip(x, y, prClipPara))
                            {
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4ColorComp);
                            }
                            else
                            {
                                pu1Write += 1;
                            }
                        }
                    }
                }
                else if (u4ColchgEn)
                {
                    if (u4KeynotEn)
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // write src color to dst
                            if (fgGfxWithinClip(x, y, prClipPara))
                            {
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4ColorComp);
                            }
                            else
                            {
                                pu1Write += 1;
                            }
                        }
                        else
                        {
                            if (fgGfxWithinClip(x, y, prClipPara))
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                            else
                            {
                                pu1Write += 1;
                            }
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            if (fgGfxWithinClip(x, y, prClipPara))
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                            else
                            {
                                pu1Write += 1;
                            }
                        }
                        else
                        {
                            if (fgGfxWithinClip(x, y, prClipPara))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4ColorComp);
                            }
                            else
                            {
                                pu1Write += 1;
                            }
                        }
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    if (fgGfxWithinClip(x, y, prClipPara))
                    {
                        GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4ColorComp);
                    }
                    else
                    {
                        pu1Write += 1;
                    }
                }

            }
            else if((u4SrcCM == u4DstCM) && ((UINT32)CM_RGB565_DIRECT16 == u4SrcCM))// 32bpp and 16bpp
            {
                if (u4TransEn)
                {
                    if (u4KeynotEn)
                    {
                        if (
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            if (fgGfxWithinClip(x, y, prClipPara))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4ColorComp);
                            }
                            else
                            {
                                pu1Write += 2;
                            }
                        }
                        else
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                pu1Write += 4;
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                pu1Write += 2;
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        if ((ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                pu1Write += 4;
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                pu1Write += 2;
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                        else
                        {
                            if (fgGfxWithinClip(x, y, prClipPara))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4ColorComp);
                            }
                            else
                            {
                                pu1Write += 2;
                            }
                        }
                    }
                }
                else if (u4ColchgEn)
                {
                    if (u4KeynotEn)
                    {
                        if ((ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            if (fgGfxWithinClip(x, y, prClipPara))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4ColorComp);
                            }
                            else
                            {
                                pu1Write += 2;
                            }
                        }
                        else
                        {
                            if (fgGfxWithinClip(x, y, prClipPara))
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                            else
                            {
                                pu1Write += 2;
                            }
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        if ((ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            if (fgGfxWithinClip(x, y, prClipPara))
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                            else
                            {
                                pu1Write += 2;
                            }
                        }
                        else
                        {
                            if (fgGfxWithinClip(x, y, prClipPara))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4ColorComp);
                            }
                            else
                            {
                                pu1Write += 2;
                            }
                        }
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    if (fgGfxWithinClip(x, y, prClipPara))
                    {
                        GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4ColorComp);
                    }
                    else
                    {
                        pu1Write += 2;
                    }
                }
            }
            else // 32bpp and 16bpp
            {
                if (u4TransEn)
                {
                    if (u4KeynotEn)
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            if (fgGfxWithinClip(x, y, prClipPara))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4ColorComp);
                            }
                            else
                            {
                                pu1Write += ((UINT32)CM_ARGB8888_DIRECT32 == u4SrcCM) ? 4 : 2;
                            }
                        }
                        else
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                pu1Write += 4;
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                pu1Write += 2;
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                pu1Write += 4;
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                pu1Write += 2;
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                        else
                        {
                            if (fgGfxWithinClip(x, y, prClipPara))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                    ai4ColorComp);
                            }
                            else
                            {
                                pu1Write += ((UINT32)CM_ARGB8888_DIRECT32 == u4SrcCM) ? 4 : 2;
                            }
                        }
                    }
                }
                else if (u4ColchgEn)
                {
                    if (u4KeynotEn)
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            if (fgGfxWithinClip(x, y, prClipPara))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4ColorComp);
                            }
                            else
                            {
                                pu1Write += ((UINT32)CM_ARGB8888_DIRECT32 == u4SrcCM) ? 4 : 2;
                            }
                        }
                        else
                        {
                            if (fgGfxWithinClip(x, y, prClipPara))
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                            }
                            else
                            {
                                pu1Write += ((UINT32)CM_ARGB8888_DIRECT32 == u4SrcCM) ? 4 : 2;
                            }
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            if (fgGfxWithinClip(x, y, prClipPara))
                            {
                                // write rect color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4RectColor);
                            }
                            else
                            {
                                pu1Write += ((UINT32)CM_ARGB8888_DIRECT32 == u4SrcCM) ? 4 : 2;
                            }
                        }
                        else
                        {
                            if (fgGfxWithinClip(x, y, prClipPara))
                            {
                                // write src color to dst
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                    ai4ColorComp);
                            }
                            else
                            {
                                pu1Write += ((UINT32)CM_ARGB8888_DIRECT32 == u4SrcCM) ? 4 : 2;
                            }
                        }
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    if (fgGfxWithinClip(x, y, prClipPara))
                    {
                        GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4ColorComp);
                    }
                    else
                    {
                        pu1Write += ((UINT32)CM_ARGB8888_DIRECT32 == u4SrcCM) ? 4 : 2;
                    }
                }
            }
        }

        pu1SrcLine += u4SrcPitch;
        pu1DstLine += u4DstPitch;
    }

    return (INT32)E_GFX_OK;
}


void  GFX_SwRGB2YCbCr601(INT32 *pi4sColorComponent)
{

    INT32 Y,Cb,Cr;
    INT32 R,G,B;
    B = (INT32)pi4sColorComponent[3];
    G = (INT32)pi4sColorComponent[2];
    R = (INT32)pi4sColorComponent[1];

    Y=(INT32)((16*64+16*R+32*G+6*B)/64);
    Cb= (INT32)((128*64-9*R-19*G+28*B)/64);
    Cr= (INT32)((128*64+28*R-24*G-5*B)/64);

    pi4sColorComponent[1]=  (Y > 255) ? 255 :
                           (Y <   0) ?   0 : Y;
    pi4sColorComponent[2]=  (Cb > 255) ? 255 :
                           (Cb <   0) ?   0 : Cb;
    pi4sColorComponent[3]= (Cr > 255) ? 255 :
                           (Cr <   0) ?   0 : Cr;
}
void GFX_SwRGB2YCbCr709(INT32 *pi4sColorComponent)
{
    INT32 Y,Cb,Cr;
    INT32 R,G,B;
    B = (INT32)pi4sColorComponent[3];
    G = (INT32)pi4sColorComponent[2];
    R = (INT32)pi4sColorComponent[1];

    Y= (INT32)((16*64+11*R+39*G+4*B)/64);
    Cb= (INT32)((128*64-6*R-22*G+28*B)/64);
    Cr= (INT32)((128*64+28*R-26*G-3*B)/64);


    pi4sColorComponent[1]=  (Y > 255) ? 255 :
                           (Y <   0) ?   0 : Y;
    pi4sColorComponent[2]=  (Cb > 255) ? 255 :
                           (Cb <   0) ?   0 : Cb;
    pi4sColorComponent[3]= (Cr > 255) ? 255 :
                           (Cr <   0) ?   0 : Cr;
}

void GFX_SwYCbCr6012RGB(INT32 *pi4sColorComponent)
{
    INT32 Y,Cb,Cr;
    INT32 R,G,B;
    Cr= (INT32)pi4sColorComponent[3];
    Cb = (INT32)pi4sColorComponent[2];
    Y = (INT32)pi4sColorComponent[1];

    R= (INT32)((75*Y-Y/2+102*Cr)/64-223);
    G= (INT32)((75*Y-Y/2-52*Cr-25*Cb)/64+135);
    B= (INT32)((75*Y-Y/2+129*Cb)/64-277);


    pi4sColorComponent[1]=  (R > 255) ? 255 :
                           (R <   0) ?   0 : R;
    pi4sColorComponent[2]=  (G> 255) ? 255 :
                           (G <   0) ?   0 : G;
    pi4sColorComponent[3]= (B > 255) ? 255 :
                           (B <   0) ?   0 : B;
}
void GFX_SwYCbCr7092RGB(INT32 *pi4sColorComponent)
{
    INT32 Y,Cb,Cr;
    INT32 R,G,B;
    Cr= (INT32)pi4sColorComponent[3];
    Cb = (INT32)pi4sColorComponent[2];
    Y = (INT32)pi4sColorComponent[1];

    R= (INT32)((75*Y-Y/2+115*Cr)/64-248);
    G= (INT32)((75*Y-Y/2-34*Cr-14*Cb)/64+77);
    B= (INT32)((75*Y-Y/2+135*Cb)/64-289);

   pi4sColorComponent[1]=  (R > 255) ? 255 :
                           (R <   0) ?   0 : R;
    pi4sColorComponent[2]=  (G> 255) ? 255 :
                           (G <   0) ?   0 : G;
    pi4sColorComponent[3]= (B > 255) ? 255 :
                           (B <   0) ?   0 : B;
}

//-------------------------------------------------------------------------
/** GFX_SwBitBlt
 *  gfx normal bitblt
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e740 -e613 */
INT32 GFX_SwBitBlt_R2Y(UINT8 *pu1SrcBase, UINT8 *pu1DstBase, UINT32 u4SrcPitch,
    UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM, UINT32 u4Width,
    UINT32 u4Height, UINT32 u4ColorMin, UINT32 u4ColorMax, UINT32 u4TransEn,
    UINT32 u4ColchgEn, UINT32 u4KeynotEn, UINT32 u4Color,UINT32 u4R2YY2REnable)
{
//    INT32 ai4ColorCompMin[4] = {0};
//    INT32 ai4ColorCompMax[4] = {0};
       INT32 ai4ColorComp[4] = {0};
//    INT32 ai4RectColor[4] = {0};
//    UINT32 *pu4Color;
    UINT32 x, y;
    UINT8 *pu1SrcLine, *pu1DstLine;
    UINT8 *pu1Read, *pu1Write;
    // HalFlushDCache();
    //HalInvalidateDCache();
    //HalFlushInvalidateDCache();
    // BIM_WAIT_WALE();
    // protection
    if (u4TransEn && u4ColchgEn)
    {
        return -(INT32)E_GFX_INV_ARG;
    }

    pu1Read     = pu1SrcBase;
    pu1SrcLine  = pu1SrcBase;
    pu1Write    = pu1DstBase;
    pu1DstLine  = pu1DstBase;
    for (y = 0; y < u4Height; y++)
    {
        pu1Read  = pu1SrcLine;
        pu1Write = pu1DstLine;

        for (x = 0; x < u4Width; x++)
        {
            GFX_SwGetColorComponent(&pu1Read, u4SrcCM, ai4ColorComp);
            if((u4R2YY2REnable%4)==0)
            {
                GFX_SwRGB2YCbCr601(ai4ColorComp);
            }
            else if((u4R2YY2REnable%4)==1)
            {
                GFX_SwRGB2YCbCr709(ai4ColorComp);
            }
            else if((u4R2YY2REnable%4)==2)
            {
                GFX_SwYCbCr6012RGB(ai4ColorComp);
            }
            else
            {
                GFX_SwYCbCr7092RGB(ai4ColorComp);
            }


            GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4ColorComp);


        }

        pu1SrcLine += u4SrcPitch;
        pu1DstLine += u4DstPitch;
    }

    return (INT32)E_GFX_OK;
}

//-------------------------------------------------------------------------
/** GFX_SwBitBlt
 *  gfx normal bitblt
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e740 -e613 */
INT32 GFX_SwBitBlt_DstFlip(UINT8 *pu1SrcBase, UINT8 *pu1DstBase, UINT32 u4SrcPitch,
    UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM, UINT32 u4Width,
    UINT32 u4Height, UINT32 u4ColorMin, UINT32 u4ColorMax, UINT32 u4TransEn,
    UINT32 u4ColchgEn, UINT32 u4KeynotEn, UINT32 u4Color, UINT32 u4SrcFlag, UINT32 u4DstFlag )
{
    INT32 ai4ColorCompMin[4] = {0};
    INT32 ai4ColorCompMax[4] = {0};
    INT32 ai4ColorComp[4] = {0};
    INT32 ai4RectColor[4] = {0};
    UINT32 *pu4Color;
    UINT32 x, y;
    UINT8 *pu1SrcLine, *pu1DstLine;
    UINT8 *pu1Read, *pu1Write;
//    UINT32 u4PiexlLg/*,u4ByteNum*/;
    UINT32 u4DstFlip=0;
    UINT32 u4SrcFlip=0;
    UINT32 u4DstMirror=0;
    UINT32 u4SrcMirror=0;
    // protection
    if (u4TransEn && u4ColchgEn)
    {
        return -(INT32)E_GFX_INV_ARG;
    }
  //  u4PiexlLg = _au1sPixelLgDwn[u4SrcCM];
   // u4ByteNum = (u4PiexlLg == 0 ? 4 : (u4PiexlLg == 1 ? 2 : 1 ));
    if(u4SrcFlag==1) //src flip
    {
        u4SrcFlip=1;
        pu1SrcLine  = pu1SrcBase;//u4DstPitch*(u4Height-1);
    }
    else if(u4SrcFlag==2)//src mirror
    {
        u4SrcMirror=1;
        pu1SrcLine  = pu1SrcBase;//(u4Width-1)*u4ByteNum;
    }
    else if(u4SrcFlag==3)//src flip+mirror
    {
        u4SrcFlip=1;
        u4SrcMirror=1;
        pu1SrcLine  = pu1SrcBase;//+u4DstPitch*(u4Height-1)+(u4Width-1)*u4ByteNum;
    }
    else
    {
        pu1SrcLine  = pu1SrcBase;
    }

    if(u4DstFlag==1)//dst for flip
    {
        u4DstFlip=1;
        pu1DstLine  = pu1DstBase;//u4DstPitch*(u4Height-1);
    }
    else if(u4DstFlag==2)
    {
        u4DstMirror=1;
        pu1DstLine  = pu1DstBase;//(u4Width-1)*u4ByteNum;
    }
    else if(u4DstFlag==3)
    {
        u4DstFlip=1;
        u4DstMirror=1;
        pu1DstLine  = pu1DstBase;//u4DstPitch*(u4Height-1)+(u4Width-1)*u4ByteNum;
    }
    else
    {
        pu1DstLine  = pu1DstBase;
    }
    if (u4TransEn || u4ColchgEn)
    {
        ai4ColorCompMin[0] = (INT32) (u4ColorMin >> 24);
        ai4ColorCompMin[1] = (INT32)((u4ColorMin >> 16) & 0xFF);
        ai4ColorCompMin[2] = (INT32)((u4ColorMin >>  8) & 0xFF);
        ai4ColorCompMin[3] = (INT32)( u4ColorMin        & 0xFF);

        ai4ColorCompMax[0] = (INT32) (u4ColorMax >> 24);
        ai4ColorCompMax[1] = (INT32)((u4ColorMax >> 16) & 0xFF);
        ai4ColorCompMax[2] = (INT32)((u4ColorMax >>  8) & 0xFF);
        ai4ColorCompMax[3] = (INT32)( u4ColorMax        & 0xFF);

        pu4Color = &u4Color;
        u4Color  = _GfxSwColorExpansion(u4Color, u4DstCM);

        GFX_SwGetColorComponent((UINT8 **)&pu4Color, u4DstCM, ai4RectColor);
    }

    for (y = 0; y < u4Height; y++)
    {
        pu1Read  = pu1SrcLine;
        pu1Write = pu1DstLine;

        for (x = 0; x < u4Width; x++)
        {
            if(u4SrcMirror==1)
            {
                GFX_SwGetColorComponent_Mirror(&pu1Read, u4SrcCM, ai4ColorComp);
            }
            else
            {
                GFX_SwGetColorComponent(&pu1Read, u4SrcCM, ai4ColorComp);
            }


            // the document says COLCHA_ENA and TRANS_ENA should not
            // set at the same time, but if we set them both, which
            // one would process by hardware ? Check it.
            //
            // hardware can NOT work, software must keep it off
            //
            if ((u4SrcCM == u4DstCM) && ((UINT32)CM_RGB_CLUT8 == u4SrcCM))
            {
                if (u4TransEn)
                {
                    if (u4KeynotEn)
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // write src color to dst
                            if(u4DstMirror)
                            {
                                GFX_SwSetColorComponent_Mirror(&pu1Write, u4DstCM,
                                ai4ColorComp);
                            }
                            else
                            {
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                            }
                        }
                        else
                        {
                            // keep dst color
                            if(u4DstMirror)
                            {
                                pu1Write -= 1;
                            }
                            else
                            {
                                pu1Write += 1;
                            }
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // keep dst color
                             if(u4DstMirror)
                            {
                                pu1Write -= 1;
                            }
                            else
                            {
                                pu1Write += 1;
                            }
                        }
                        else
                        {
                            // write src color to dst
                             if(u4DstMirror)
                            {
                                GFX_SwSetColorComponent_Mirror(&pu1Write, u4DstCM,
                                ai4ColorComp);
                            }
                            else
                            {
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                            }
                        }
                    }
                }
                else if (u4ColchgEn)
                {
                    if (u4KeynotEn)
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // write src color to dst
                            if(u4DstMirror)
                            {
                                GFX_SwSetColorComponent_Mirror(&pu1Write, u4DstCM,
                                ai4ColorComp);
                            }
                            else
                            {
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                            }
                        }
                        else
                        {
                            // write rect color to dst
                            if(u4DstMirror)
                            {
                                GFX_SwSetColorComponent_Mirror(&pu1Write, u4DstCM,
                                ai4RectColor);
                            }
                            else
                            {
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4RectColor);
                            }
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // write rect color to dst
                            if(u4DstMirror)
                            {
                                GFX_SwSetColorComponent_Mirror(&pu1Write, u4DstCM,
                                ai4RectColor);
                            }
                            else
                            {
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4RectColor);
                            }

                        }
                        else
                        {
                            // write src color to dst
                            if(u4DstMirror)
                            {
                                GFX_SwSetColorComponent_Mirror(&pu1Write, u4DstCM,
                                ai4ColorComp);
                            }
                            else
                            {
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                            }
                        }
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    if(u4DstMirror)
                    {
                        GFX_SwSetColorComponent_Mirror(&pu1Write, u4DstCM,
                                ai4ColorComp);
                    }
                    else
                    {
                        GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                    }
                }

            }
            else    // 32bpp and 16bpp
            {
                if (u4TransEn)
                {
                    if (u4KeynotEn)
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write src color to dst
                            if(u4DstMirror)
                            {
                                GFX_SwSetColorComponent_Mirror(&pu1Write, u4DstCM,
                                        ai4ColorComp);
                            }
                            else
                            {
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                        ai4ColorComp);
                            }
                        }
                        else
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                if(u4DstMirror)
                                {
                                    pu1Write -= 4;
                                }
                                else
                                {
                                    pu1Write += 4;
                                }
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                if(u4DstMirror)
                                {
                                    pu1Write -= 2;
                                }
                                else
                                {
                                    pu1Write += 2;
                                }
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                if(u4DstMirror)
                                {
                                    pu1Write -= 4;
                                }
                                else
                                {
                                    pu1Write += 4;
                                }
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                if(u4DstMirror)
                                {
                                    pu1Write -= 2;
                                }
                                else
                                {
                                    pu1Write += 2;
                                }
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                        else
                        {
                            // write src color to dst
                             if(u4DstMirror)
                            {
                                GFX_SwSetColorComponent_Mirror(&pu1Write, u4DstCM,
                                        ai4ColorComp);
                            }
                            else
                            {
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                        ai4ColorComp);
                            }
                        }
                    }
                }
                else if (u4ColchgEn)
                {
                    if (u4KeynotEn)
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write src color to dst
                             if(u4DstMirror)
                            {
                                GFX_SwSetColorComponent_Mirror(&pu1Write, u4DstCM,
                                        ai4ColorComp);
                            }
                            else
                            {
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                        ai4ColorComp);
                            }
                        }
                        else
                        {
                            // write rect color to dst
                             if(u4DstMirror)
                            {
                                GFX_SwSetColorComponent_Mirror(&pu1Write, u4DstCM,
                                        ai4RectColor);
                            }
                            else
                            {
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                        ai4RectColor);
                            }
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write rect color to dst
                             if(u4DstMirror)
                            {
                                GFX_SwSetColorComponent_Mirror(&pu1Write, u4DstCM,
                                        ai4RectColor);
                            }
                            else
                            {
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                        ai4RectColor);
                            }
                        }
                        else
                        {
                            // write src color to dst
                            if(u4DstMirror)
                            {
                                GFX_SwSetColorComponent_Mirror(&pu1Write, u4DstCM,
                                        ai4ColorComp);
                            }
                            else
                            {
                                GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                        ai4ColorComp);
                            }
                        }
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    if(u4DstMirror)
                    {
                        GFX_SwSetColorComponent_Mirror(&pu1Write, u4DstCM,
                                        ai4ColorComp);
                    }
                    else
                    {
                        GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                        ai4ColorComp);
                    }
                }
            }
        }
        if(u4SrcFlip==1)
        {
            pu1SrcLine -= u4SrcPitch;//for flip
        }
        else
        {
            pu1SrcLine += u4SrcPitch;
        }
        if(u4DstFlip==1)
        {
            pu1DstLine -= u4DstPitch;
        }
        else
        {
            pu1DstLine += u4DstPitch;
        }
    }

    return (INT32)E_GFX_OK;
}


//-------------------------------------------------------------------------
/** GFX_SwBitBlt
 *  gfx normal bitblt
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e740 -e613 */
#if 0
INT32 GFX_SwBitBlt_DstMirr(UINT8 *pu1SrcBase, UINT8 *pu1DstBase, UINT32 u4SrcPitch,
    UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM, UINT32 u4Width,
    UINT32 u4Height, UINT32 u4ColorMin, UINT32 u4ColorMax, UINT32 u4TransEn,
    UINT32 u4ColchgEn, UINT32 u4KeynotEn, UINT32 u4Color)
{
    INT32 ai4ColorCompMin[4] = {0};
    INT32 ai4ColorCompMax[4] = {0};
    INT32 ai4ColorComp[4] = {0};
    INT32 ai4RectColor[4] = {0};
    UINT32 *pu4Color;
    UINT32 x, y;
    UINT8 *pu1SrcLine, *pu1DstLine;
    UINT8 *pu1Read, *pu1Write;

    // protection
    if (u4TransEn && u4ColchgEn)
    {
        return -(INT32)E_GFX_INV_ARG;
    }

    pu1Read     = pu1SrcBase;
    pu1SrcLine  = pu1SrcBase;
    pu1Write    = pu1DstBase;
    pu1DstLine  = pu1DstBase;

    if (u4TransEn || u4ColchgEn)
    {
        ai4ColorCompMin[0] = (INT32) (u4ColorMin >> 24);
        ai4ColorCompMin[1] = (INT32)((u4ColorMin >> 16) & 0xFF);
        ai4ColorCompMin[2] = (INT32)((u4ColorMin >>  8) & 0xFF);
        ai4ColorCompMin[3] = (INT32)( u4ColorMin        & 0xFF);

        ai4ColorCompMax[0] = (INT32) (u4ColorMax >> 24);
        ai4ColorCompMax[1] = (INT32)((u4ColorMax >> 16) & 0xFF);
        ai4ColorCompMax[2] = (INT32)((u4ColorMax >>  8) & 0xFF);
        ai4ColorCompMax[3] = (INT32)( u4ColorMax        & 0xFF);

        pu4Color = &u4Color;
        u4Color  = _GfxSwColorExpansion(u4Color, u4DstCM);

        GFX_SwGetColorComponent((UINT8 **)&pu4Color, u4DstCM, ai4RectColor);
    }

    for (y = 0; y < u4Height; y++)
    {
        pu1Read  = pu1SrcLine;
        pu1Write = pu1DstLine;

        for (x = 0; x < u4Width; x++)
        {
            GFX_SwGetColorComponent(&pu1Read, u4SrcCM, ai4ColorComp);

            // the document says COLCHA_ENA and TRANS_ENA should not
            // set at the same time, but if we set them both, which
            // one would process by hardware ? Check it.
            //
            // hardware can NOT work, software must keep it off
            //
            if ((u4SrcCM == u4DstCM) && ((UINT32)CM_RGB_CLUT8 == u4SrcCM))
            {
                if (u4TransEn)
                {
                    if (u4KeynotEn)
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // keep dst color
                            pu1Write += 1;
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // keep dst color
                            pu1Write += 1;
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else if (u4ColchgEn)
                {
                    if (u4KeynotEn)
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4ColorComp);
                }

            }
            else    // 32bpp and 16bpp
            {
                if (u4TransEn)
                {
                    if (u4KeynotEn)
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                pu1Write += 4;
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                pu1Write += 2;
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                pu1Write += 4;
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                pu1Write += 2;
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else if (u4ColchgEn)
                {
                    if (u4KeynotEn)
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4ColorComp);
                }
            }
        }

        pu1SrcLine += u4SrcPitch;
        pu1DstLine += u4DstPitch;
    }

    return (INT32)E_GFX_OK;
}
/*lint -restore */

//-------------------------------------------------------------------------
/** GFX_SwBitBlt
 *  gfx normal bitblt
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e740 -e613 */
INT32 GFX_SwBitBlt_SrcFlip(UINT8 *pu1SrcBase, UINT8 *pu1DstBase, UINT32 u4SrcPitch,
    UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM, UINT32 u4Width,
    UINT32 u4Height, UINT32 u4ColorMin, UINT32 u4ColorMax, UINT32 u4TransEn,
    UINT32 u4ColchgEn, UINT32 u4KeynotEn, UINT32 u4Color)
{
    INT32 ai4ColorCompMin[4] = {0};
    INT32 ai4ColorCompMax[4] = {0};
    INT32 ai4ColorComp[4] = {0};
    INT32 ai4RectColor[4] = {0};
    UINT32 *pu4Color;
    UINT32 x, y;
    UINT8 *pu1SrcLine, *pu1DstLine;
    UINT8 *pu1Read, *pu1Write;

    // protection
    if (u4TransEn && u4ColchgEn)
    {
        return -(INT32)E_GFX_INV_ARG;
    }

    pu1Read     = pu1SrcBase;
    pu1SrcLine  = pu1SrcBase;
    pu1Write    = pu1DstBase;
    pu1DstLine  = pu1DstBase;

    if (u4TransEn || u4ColchgEn)
    {
        ai4ColorCompMin[0] = (INT32) (u4ColorMin >> 24);
        ai4ColorCompMin[1] = (INT32)((u4ColorMin >> 16) & 0xFF);
        ai4ColorCompMin[2] = (INT32)((u4ColorMin >>  8) & 0xFF);
        ai4ColorCompMin[3] = (INT32)( u4ColorMin        & 0xFF);

        ai4ColorCompMax[0] = (INT32) (u4ColorMax >> 24);
        ai4ColorCompMax[1] = (INT32)((u4ColorMax >> 16) & 0xFF);
        ai4ColorCompMax[2] = (INT32)((u4ColorMax >>  8) & 0xFF);
        ai4ColorCompMax[3] = (INT32)( u4ColorMax        & 0xFF);

        pu4Color = &u4Color;
        u4Color  = _GfxSwColorExpansion(u4Color, u4DstCM);

        GFX_SwGetColorComponent((UINT8 **)&pu4Color, u4DstCM, ai4RectColor);
    }

    for (y = 0; y < u4Height; y++)
    {
        pu1Read  = pu1SrcLine;
        pu1Write = pu1DstLine;

        for (x = 0; x < u4Width; x++)
        {
            GFX_SwGetColorComponent(&pu1Read, u4SrcCM, ai4ColorComp);

            // the document says COLCHA_ENA and TRANS_ENA should not
            // set at the same time, but if we set them both, which
            // one would process by hardware ? Check it.
            //
            // hardware can NOT work, software must keep it off
            //
            if ((u4SrcCM == u4DstCM) && ((UINT32)CM_RGB_CLUT8 == u4SrcCM))
            {
                if (u4TransEn)
                {
                    if (u4KeynotEn)
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // keep dst color
                            pu1Write += 1;
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // keep dst color
                            pu1Write += 1;
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else if (u4ColchgEn)
                {
                    if (u4KeynotEn)
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4ColorComp);
                }

            }
            else    // 32bpp and 16bpp
            {
                if (u4TransEn)
                {
                    if (u4KeynotEn)
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                pu1Write += 4;
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                pu1Write += 2;
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                pu1Write += 4;
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                pu1Write += 2;
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else if (u4ColchgEn)
                {
                    if (u4KeynotEn)
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4ColorComp);
                }
            }
        }

        pu1SrcLine += u4SrcPitch;
        pu1DstLine += u4DstPitch;
    }

    return (INT32)E_GFX_OK;
}

//-------------------------------------------------------------------------
/** GFX_SwBitBlt
 *  gfx normal bitblt
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e740 -e613 */
INT32 GFX_SwBitBlt_SrcMirr(UINT8 *pu1SrcBase, UINT8 *pu1DstBase, UINT32 u4SrcPitch,
    UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM, UINT32 u4Width,
    UINT32 u4Height, UINT32 u4ColorMin, UINT32 u4ColorMax, UINT32 u4TransEn,
    UINT32 u4ColchgEn, UINT32 u4KeynotEn, UINT32 u4Color)
{
    INT32 ai4ColorCompMin[4] = {0};
    INT32 ai4ColorCompMax[4] = {0};
    INT32 ai4ColorComp[4] = {0};
    INT32 ai4RectColor[4] = {0};
    UINT32 *pu4Color;
    UINT32 x, y;
    UINT8 *pu1SrcLine, *pu1DstLine;
    UINT8 *pu1Read, *pu1Write;

    // protection
    if (u4TransEn && u4ColchgEn)
    {
        return -(INT32)E_GFX_INV_ARG;
    }

    pu1Read     = pu1SrcBase;
    pu1SrcLine  = pu1SrcBase;
    pu1Write    = pu1DstBase;
    pu1DstLine  = pu1DstBase;

    if (u4TransEn || u4ColchgEn)
    {
        ai4ColorCompMin[0] = (INT32) (u4ColorMin >> 24);
        ai4ColorCompMin[1] = (INT32)((u4ColorMin >> 16) & 0xFF);
        ai4ColorCompMin[2] = (INT32)((u4ColorMin >>  8) & 0xFF);
        ai4ColorCompMin[3] = (INT32)( u4ColorMin        & 0xFF);

        ai4ColorCompMax[0] = (INT32) (u4ColorMax >> 24);
        ai4ColorCompMax[1] = (INT32)((u4ColorMax >> 16) & 0xFF);
        ai4ColorCompMax[2] = (INT32)((u4ColorMax >>  8) & 0xFF);
        ai4ColorCompMax[3] = (INT32)( u4ColorMax        & 0xFF);

        pu4Color = &u4Color;
        u4Color  = _GfxSwColorExpansion(u4Color, u4DstCM);

        GFX_SwGetColorComponent((UINT8 **)&pu4Color, u4DstCM, ai4RectColor);
    }

    for (y = 0; y < u4Height; y++)
    {
        pu1Read  = pu1SrcLine;
        pu1Write =  pu1DstLine;

        for (x = 0; x < u4Width; x++)
        {
            GFX_SwGetColorComponent(&pu1Read, u4SrcCM, ai4ColorComp);

            // the document says COLCHA_ENA and TRANS_ENA should not
            // set at the same time, but if we set them both, which
            // one would process by hardware ? Check it.
            //
            // hardware can NOT work, software must keep it off
            //
            if ((u4SrcCM == u4DstCM) && ((UINT32)CM_RGB_CLUT8 == u4SrcCM))
            {
                if (u4TransEn)
                {
                    if (u4KeynotEn)
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // keep dst color
                            pu1Write += 1;
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // keep dst color
                            pu1Write += 1;
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else if (u4ColchgEn)
                {
                    if (u4KeynotEn)
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        // exactly equal color_key_min[7:0]
                        if (ai4ColorComp[0] == ai4ColorCompMin[3])
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4ColorComp);
                }

            }
            else    // 32bpp and 16bpp
            {
                if (u4TransEn)
                {
                    if (u4KeynotEn)
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                pu1Write += 4;
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                pu1Write += 2;
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // keep dst color
                            if ((UINT32)CM_ARGB8888_DIRECT32 == u4DstCM)
                            {
                                pu1Write += 4;
                            }
                            else if (((UINT32)CM_ARGB4444_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_ARGB1555_DIRECT16 == u4DstCM) ||
                                     ((UINT32)CM_RGB565_DIRECT16   == u4DstCM))
                            {
                                pu1Write += 2;
                            }
                            else    // for lint happy
                            {
                                ;
                            }
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent(&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else if (u4ColchgEn)
                {
                    if (u4KeynotEn)
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                        else
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                    }
                    else    // u4KeynotEn = 0
                    {
                        if ((ai4ColorComp[0] >= ai4ColorCompMin[0]) &&
                            (ai4ColorComp[0] <= ai4ColorCompMax[0]) &&
                            (ai4ColorComp[1] >= ai4ColorCompMin[1]) &&
                            (ai4ColorComp[1] <= ai4ColorCompMax[1]) &&
                            (ai4ColorComp[2] >= ai4ColorCompMin[2]) &&
                            (ai4ColorComp[2] <= ai4ColorCompMax[2]) &&
                            (ai4ColorComp[3] >= ai4ColorCompMin[3]) &&
                            (ai4ColorComp[3] <= ai4ColorCompMax[3]))
                        {
                            // write rect color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4RectColor);
                        }
                        else
                        {
                            // write src color to dst
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM,
                                ai4ColorComp);
                        }
                    }
                }
                else        // u4TransEn = 0 and u4ColchgEn = 0
                {
                    GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4ColorComp);
                }
            }
        }

        pu1SrcLine += u4SrcPitch;
        pu1DstLine += u4DstPitch;
    }

    return (INT32)E_GFX_OK;
}
#endif
//-------------------------------------------------------------------------
/** GFX_SwAlphaMapBitBlt
 *  SRC CM = RGB8 (4'b1010)
 *  DST CM = ARGB8888/ARGB4444/ARGB1555
 */
//-------------------------------------------------------------------------
/*lint -save -e613 */
INT32 GFX_SwAlphaMapBitBlt(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    UINT32 u4SrcPitch, UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM,
    UINT32 u4Width, UINT32 u4Height)
{
    UINT8 u1Color, u1Alpha;
    UINT8 *pu1SrcPtr, *pu1DstPtr;
    UINT32 u4DstPW;                 // dst pixel width
    UINT32 i, j;

    // check parameters' error
    VERIFY(pu1SrcBase != NULL);
    VERIFY(pu1DstBase != NULL);

    if (u4SrcCM != (UINT32)CM_RGB_CLUT8)
    {
        return -(INT32)E_GFX_INV_ARG;
    }
    //HalFlushDCache();
    // HalInvalidateDCache();
    HalFlushInvalidateDCache();
    // BIM_WAIT_WALE();
    switch (u4DstCM)
    {
    case CM_ARGB8888_DIRECT32:
        u4DstPW = 4;                // 1 pixel = 4 bytes
        for (j = 0; j < u4Height; j++)
        {
            pu1SrcPtr = (UINT8 *)(pu1SrcBase + (j * u4SrcPitch));
            pu1DstPtr = (UINT8 *)(pu1DstBase + (j * u4DstPitch));

            for (i = 0; i < u4Width; i++)
            {
                *(pu1DstPtr + (i * u4DstPW) + 3) = *(pu1SrcPtr + i);
            }
        }
        break;

    case CM_ARGB4444_DIRECT16:
        u4DstPW = 2;                // 1 pixel = 2 bytes
        for (j = 0; j < u4Height; j++)
        {
            pu1SrcPtr = (UINT8 *)(pu1SrcBase + (j * u4SrcPitch));
            pu1DstPtr = (UINT8 *)(pu1DstBase + (j * u4DstPitch));

            for (i = 0; i < u4Width; i++)
            {
                // preserve MSB 4-bit
                u1Alpha = *(pu1SrcPtr + i) & 0xF0;
                u1Color = *(pu1DstPtr + (i * u4DstPW) + 1) & 0x0F;
                *(pu1DstPtr + (i * u4DstPW) + 1) = (u1Alpha | u1Color);
            }
        }
        break;

    case CM_ARGB1555_DIRECT16:
        u4DstPW = 2;                // 1 pixel = 2 bytes
        for (j = 0; j < u4Height; j++)
        {
            pu1SrcPtr = (UINT8 *)(pu1SrcBase + (j * u4SrcPitch));
            pu1DstPtr = (UINT8 *)(pu1DstBase + (j * u4DstPitch));

            for (i = 0; i < u4Width; i++)
            {
                u1Alpha = (*(pu1SrcPtr + i) == 0) ? 0x0 : 0x80;
                u1Color = *(pu1DstPtr + (i * u4DstPW) + 1) & 0x7F;
                *(pu1DstPtr + (i * u4DstPW) + 1) = (u1Alpha | u1Color);
            }
        }
        break;

    default:
        return -(INT32)E_GFX_INV_ARG;

    } // end of switch (u4DstCM)

    return (INT32)E_GFX_OK;
}
/*lint -restore */


//-------------------------------------------------------------------------
/** GFX_SwAlphaComposePass
 *  gfx alpha composition
 *  Pass0 ~ Pass3
 */
//-------------------------------------------------------------------------
/*lint -save -e704 -e613 */
INT32 GFX_SwAlphaComposePass(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    UINT32 u4SrcPitch, UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM,
    UINT32 u4Width, UINT32 u4Height, UINT32 u4AlphaValue, UINT32 u4AlcomPass)
{
    UINT32 u4Alpha, u4Fs, u4Fd, x, y;
    UINT8 *pu1SrcLine, *pu1DstLine;
    UINT8 *pu1Read, *pu1Write, *pu1Dst;
    INT32 ai4SrcColor[4], ai4DstColor[4], i4Alpha, i;

    UNUSED(u4SrcCM);

    u4Alpha = u4AlphaValue;     // Ar
    u4Alpha = (u4Alpha == 255) ? 256 : u4Alpha;

    pu1Read    = pu1SrcBase;
    pu1SrcLine = pu1SrcBase;
    pu1Write   = pu1DstBase;
    pu1DstLine = pu1DstBase;

    switch (u4AlcomPass)
    {
    case 0:     // Pass 0 (As * Ar)
        for (y = 0; y < u4Height; y++)
        {
            for (x = 0; x < u4Width; x++)
            {
                GFX_SwGetColorComponent(&pu1Read, u4DstCM, ai4SrcColor);

                ai4DstColor[0] = ai4SrcColor[0] * (INT32)u4Alpha;  // As * Ar
                ai4DstColor[1] = ai4SrcColor[1];
                ai4DstColor[2] = ai4SrcColor[2];
                ai4DstColor[3] = ai4SrcColor[3];

                // Rounding
                if ((ai4DstColor[0] >> 7) & 1)
                {
                    ai4DstColor[0] = (ai4DstColor[0] >> 8) + 1;
                }
                else
                {
                    ai4DstColor[0] = (ai4DstColor[0] >> 8);
                }
                GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4DstColor);
            }

            pu1SrcLine += u4SrcPitch;
            pu1DstLine += u4DstPitch;
            pu1Read     = pu1SrcLine;
            pu1Write    = pu1DstLine;
        }
        break;

    case 1:     // Pass 1
        for (y = 0; y < u4Height; y++)
        {
            for (x = 0; x < u4Width; x++)
            {
                GFX_SwGetColorComponent(&pu1Read, u4DstCM, ai4SrcColor);

                i4Alpha        = ai4SrcColor[0];
                ai4DstColor[0] = ai4SrcColor[0];

                i4Alpha = (i4Alpha == 255) ? 256 : i4Alpha;

                for (i = 1; i < 4; i++)
                {
                    // As' * Cs
                    ai4DstColor[i] = ai4SrcColor[i] * i4Alpha;

                    // Rounding
                    if ((ai4DstColor[i] >> 7) & 1)
                    {
                        ai4DstColor[i] = (ai4DstColor[i] >> 8) + 1;
                    }
                    else
                    {
                        ai4DstColor[i] = (ai4DstColor[i] >> 8);
                    }
                }
                GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4DstColor);
            }

            pu1SrcLine += u4SrcPitch;
            pu1DstLine += u4DstPitch;
            pu1Read     = pu1SrcLine;
            pu1Write    = pu1DstLine;
        }
        break;

    case 2:     // Pass 2
        for (y = 0; y < u4Height; y++)
        {
            for (x = 0; x < u4Width; x++)
            {
                pu1Dst = pu1Write;

                GFX_SwGetColorComponent(&pu1Read, u4DstCM, ai4SrcColor);
                GFX_SwGetColorComponent(&pu1Dst, u4DstCM, ai4DstColor);

                switch (u4Alpha)
                {
                case E_AC_CLEAR:    // 0
                    u4Fs = 0;
                    u4Fd = 0;
                    break;

                case E_AC_DST_IN:   // 1
                    u4Fs = 0;
                    u4Fd = (UINT32)(ai4SrcColor[0]);
                    break;

                case E_AC_DST_OUT:  // 2
                    u4Fs = 0;
                    u4Fd = (UINT32)(256 - ai4SrcColor[0]);
                    break;

                case E_AC_DST_OVER: // 3
                    u4Fs = (UINT32)(256 - ai4DstColor[0]);
                    u4Fd = 256;
                    break;

                case E_AC_SRC:      // 4
                    u4Fs = 256;
                    u4Fd = 0;
                    break;

                case E_AC_SRC_IN:   // 5
                    u4Fs = (UINT32)(ai4DstColor[0]);
                    u4Fd = 0;
                    break;

                case E_AC_SRC_OUT:  // 6
                    u4Fs = (UINT32)(256 - ai4DstColor[0]);
                    u4Fd = 0;
                    break;

                case E_AC_SRC_OVER: // 7
                    u4Fs = 256;
                    u4Fd = (UINT32)(256 - ai4SrcColor[0]);
                    break;
                    //8-10 added by msz00441 080531
                case E_AC_SRC_ATOP: // 8
                    u4Fs = ai4DstColor[0];
                    u4Fd = (UINT32)(256 - ai4SrcColor[0]);
                    break;
                case E_AC_DST_ATOP: // 9
                    u4Fs = (UINT32)(256 - ai4DstColor[0]);
                    u4Fd = 256;
                    break;
                case E_AC_XOR: // 10
                    u4Fs = (UINT32)(256 - ai4DstColor[0]);;
                    u4Fd = (UINT32)(256 - ai4SrcColor[0]);
                    break;

                default:
                    return -(INT32)E_GFX_INV_ARG;
                }

                for(i = 0; i < 4; i++)
                {
                    ai4DstColor[i] = ((INT32)u4Fs * ai4SrcColor[i]) +
                                     ((INT32)u4Fd * ai4DstColor[i]);
                    // Rounding
                    if ((ai4DstColor[i] >> 7) & 1)
                    {
                        ai4DstColor[i] = (ai4DstColor[i] >> 8) + 1;
                    }
                    else
                    {
                        ai4DstColor[i] = (ai4DstColor[i] >> 8);
                    }
                }
                GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4DstColor);
            }

            pu1SrcLine += u4SrcPitch;
            pu1DstLine += u4DstPitch;
            pu1Read     = pu1SrcLine;
            pu1Write    = pu1DstLine;
        }
        break;

    case 3:     // Pass 3
        for (y = 0; y < u4Height; y++)
        {
            for (x = 0; x < u4Width; x++)
            {
                GFX_SwGetColorComponent(&pu1Read, u4DstCM, ai4SrcColor);

                ai4DstColor[0] = ai4SrcColor[0];    // Ad = As
                u4Alpha = (UINT32)ai4SrcColor[0];
                for (i = 1; i < 4; i++)
                {
                    if (u4Alpha)
                    {
                        ai4DstColor[i] =
                            (ai4SrcColor[i] * _i4sAlphaInvTbl[u4Alpha]);

                        // Rounding
                        if ((ai4DstColor[i] >> 2) & 1)
                        {
                            ai4DstColor[i] = (ai4DstColor[i] >> 3) + 1;
                        }
                        else
                        {
                            ai4DstColor[i] = (ai4DstColor[i] >> 3);
                        }
                    }
                    else
                    {
                        ai4DstColor[i] = 0;
                    }
                }
                GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4DstColor);
            }

            pu1SrcLine += u4SrcPitch;
            pu1DstLine += u4DstPitch;
            pu1Read     = pu1SrcLine;
            pu1Write    = pu1DstLine;
        }
        break;

    case 4: // Pass 4   (Alpha-BitBlt)
        for (y = 0; y < u4Height; y++)
        {
            for (x = 0; x < u4Width; x++)
            {
                pu1Dst = pu1Write;

                GFX_SwGetColorComponent(&pu1Read, u4DstCM, ai4SrcColor);
                GFX_SwGetColorComponent(&pu1Dst, u4DstCM, ai4DstColor);

                ai4DstColor[0] = ai4SrcColor[0];    // Ad = As

                GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4DstColor);
            }

            pu1SrcLine += u4SrcPitch;
            pu1DstLine += u4DstPitch;
            pu1Read     = pu1SrcLine;
            pu1Write    = pu1DstLine;
        }
        break;

    case 5: // Pass 5   (Color-BitBlt)
        for (y = 0; y < u4Height; y++)
        {
            for (x = 0; x < u4Width; x++)
            {
                pu1Dst = pu1Write;

                GFX_SwGetColorComponent(&pu1Read, u4DstCM, ai4SrcColor);
                GFX_SwGetColorComponent(&pu1Dst, u4DstCM, ai4DstColor);

                for (i = 1; i < 4; i++)
                {
                    ai4DstColor[i] = ai4SrcColor[i];    // Cd = Cs
                }

                GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4DstColor);
            }

            pu1SrcLine += u4SrcPitch;
            pu1DstLine += u4DstPitch;
            pu1Read     = pu1SrcLine;
            pu1Write    = pu1DstLine;
        }
        break;

    default:
        return -(INT32)E_GFX_INV_ARG;
    }

    return (INT32)E_GFX_OK;
}

//add by msz00441 9/18 for rop & inx2dir & h2vline
static INT32 GFX_SwRopBitBlt_0(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    UINT32 u4SrcPitch, UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM,
    UINT32 u4Width, UINT32 u4Height, UINT32 u4RopOpCode, UINT32 u4nowr,
    UINT32 u4cmpflag, UINT8 *pu1DstX, UINT8 *pu1DstY, UINT8 *pu1CmpNowr, UINT32 u4JavaXorClr,
    UINT32 u4SrcAlphaCheck, UINT32 u4ColorRepEn)
{
    UINT8 *pu1SrcLine, *pu1DstLine;
    UINT8 *pu1Read, *pu1Write;
    UINT32 x, y, i;
    UINT32 u4ByteNum = 0, u4PiexlLg = 0;
    UINT32 u4BreakFlag = 0;

    //x_dbg_stmt( "GFX_SwRopBitBlt \n");

    pu1SrcLine = pu1SrcBase;
    pu1DstLine = pu1DstBase;

    if(u4SrcCM != u4DstCM)   //check CM
    {
        return -(INT32)E_GFX_INV_ARG;
    }
    else
    {
        u4PiexlLg = _au1sPixelLgDwn[u4SrcCM];
        u4ByteNum = (u4PiexlLg == 0 ? 4 : (u4PiexlLg == 1 ? 2 : 1 ));
    }

    if( u4nowr==1 && 6==u4RopOpCode ) //to compare
    {
        for(y = 0; y < u4Height ; y++)
        {
            pu1Read  = pu1SrcLine;
            pu1Write = pu1DstLine;

            for(x = 0; x < u4Width; x++)
            {
                for(i = 0; i < u4ByteNum; i++)
                {
                    if(*(pu1Write) !=(*(pu1Read)))
                    {
                        *pu1CmpNowr = *pu1CmpNowr|0x2;
                        *pu1DstX = x;
                        *pu1DstX = y;
                        u4BreakFlag = 1;
                        break;
                    }
                    pu1Write++;
                    pu1Read++;
                }
                if(u4BreakFlag == 1)
                {
                    break;
                }
            }
            if(u4BreakFlag == 1)
            {
                break;
            }
            pu1SrcLine += u4SrcPitch;
            pu1DstLine += u4DstPitch;
        }

    }
    else if(u4nowr != 1)      //bit operation
    {
        switch (u4RopOpCode)
        {
        case E_ROP_COLORIZE: // mt8560 new added
            for(y = 0; y < u4Height ; y++)
            {
                pu1Read  = pu1SrcLine;
                pu1Write = pu1DstLine;

                for(x = 0; x < u4Width; x++)
                {
                    INT32 ai4SrcColorComp[4]  = {0};
                    INT32 ai4DstColorComp[4]  = {0};
                    INT32 ai4RectColorComp[4] = {0};

                    ai4RectColorComp[0] = (INT32)((u4JavaXorClr&0xFF000000) >> 24);
                    ai4RectColorComp[1] = (INT32)((u4JavaXorClr&0x00FF0000) >> 16); // Ar
                    ai4RectColorComp[1] += 1;
                    ai4RectColorComp[2] = (INT32)((u4JavaXorClr&0x0000FF00) >>  8); // Ag
                    ai4RectColorComp[2] += 1;
                    ai4RectColorComp[3] = (INT32)((u4JavaXorClr&0x000000FF) >>  0); // Ab
                    ai4RectColorComp[3] += 1;
                    GFX_SwGetColorComponent(&pu1Read, u4SrcCM, ai4SrcColorComp);

                    /*
                    dst a = src a
                    dst r= Ar * src r
                    dst g= Ag * src g
                    dst b= Ab * src b
                    Ar Ag Ab are 8-bit register specified by software (42b4) [23:16] [15:8] [7:0]
                    */
                    if (u4ColorRepEn)
                    {
                        ai4DstColorComp[0] = ai4SrcColorComp[0] * ai4SrcColorComp[0];
                        ai4DstColorComp[0] >>= 8;
                    }
                    else
                    {
                        ai4DstColorComp[0] = ai4SrcColorComp[0];
                    }
                    ai4DstColorComp[1] = ai4RectColorComp[1] * ai4SrcColorComp[1];
					ai4DstColorComp[1] >>= 8;
                    ai4DstColorComp[2] = ai4RectColorComp[2] * ai4SrcColorComp[2];
                    ai4DstColorComp[2] >>= 8;
                    ai4DstColorComp[3] = ai4RectColorComp[3] * ai4SrcColorComp[3];
					ai4DstColorComp[3] >>= 8;

                    //pu1Write -= u4ByteNum;
                    GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4DstColorComp);
                }

                pu1SrcLine += u4SrcPitch;
                pu1DstLine += u4DstPitch;
            }
            break;
        case E_ROP_JAVA_XOR:       //src xor JAVA_XORCOLOR xor dst
            for(y = 0; y < u4Height ; y++)
            {
                pu1Read  = pu1SrcLine;
                pu1Write = pu1DstLine;
                for(x = 0; x < u4Width; x++)
                {
                    INT32 ai4SrcColorComp[4]  = {0};
                    INT32 ai4DstColorComp[4]  = {0};
                    INT32 ai4RectColorComp[4] = {0};

                    ai4RectColorComp[0] = (INT32)((u4JavaXorClr&0xFF000000) >> 24);
                    ai4RectColorComp[1] = (INT32)((u4JavaXorClr&0x00FF0000) >> 16);
                    ai4RectColorComp[2] = (INT32)((u4JavaXorClr&0x0000FF00) >>  8);
                    ai4RectColorComp[3] = (INT32)((u4JavaXorClr&0x000000FF) >>  0);
                    GFX_SwGetColorComponent(&pu1Read, u4SrcCM, ai4SrcColorComp);
                    GFX_SwGetColorComponent(&pu1Write, u4DstCM, ai4DstColorComp);
                   if ((1 == u4SrcAlphaCheck && ai4SrcColorComp[0] >= 0x80) || 0 == u4SrcAlphaCheck)
                   {
                        ai4RectColorComp[1] = ( ~(ai4SrcColorComp[1] ) &(ai4RectColorComp[1])) | ( ~(ai4RectColorComp[1]) &(ai4SrcColorComp[1] ));
                        ai4DstColorComp[1]  = ( ~(ai4RectColorComp[1]) &(ai4DstColorComp[1] )) | ( ~(ai4DstColorComp[1] ) &(ai4RectColorComp[1]));
                        ai4RectColorComp[2] = ( ~(ai4SrcColorComp[2] ) &(ai4RectColorComp[2])) | ( ~(ai4RectColorComp[2]) &(ai4SrcColorComp[2] ));
                        ai4DstColorComp[2]  = ( ~(ai4RectColorComp[2]) &(ai4DstColorComp[2] )) | ( ~(ai4DstColorComp[2] ) &(ai4RectColorComp[2]));
                        ai4RectColorComp[3] = ( ~(ai4SrcColorComp[3] ) &(ai4RectColorComp[3])) | ( ~(ai4RectColorComp[3]) &(ai4SrcColorComp[3] ));
                        ai4DstColorComp[3]  = ( ~(ai4RectColorComp[3]) &(ai4DstColorComp[3] )) | ( ~(ai4DstColorComp[3] ) &(ai4RectColorComp[3]));
                    }

                    pu1Write -= u4ByteNum;

                    GFX_SwSetColorComponent(&pu1Write, u4DstCM, ai4DstColorComp);
                }

                pu1SrcLine += u4SrcPitch;
                pu1DstLine += u4DstPitch;
            }
            break;
        case E_ROP_NOT_SRC:        //~src
            for(y = 0; y < u4Height ; y++)
            {
                pu1Read  = pu1SrcLine;
                pu1Write = pu1DstLine;

                for(x = 0; x < u4Width; x++)
                {
                    for(i = 0; i < u4ByteNum; i++)
                    {
                        *(pu1Write) = ~(*(pu1Read));
                        pu1Write++;
                        pu1Read++;
                    }

                }

                pu1SrcLine += u4SrcPitch;
                pu1DstLine += u4DstPitch;
            }
            break;
        case E_ROP_NOT_DST:        //~dst
            for(y = 0; y < u4Height ; y++)
            {
                pu1Read  = pu1SrcLine;
                pu1Write = pu1DstLine;

                for(x = 0; x < u4Width; x++)
                {
                    for(i = 0; i < u4ByteNum; i++)
                    {
                        *(pu1Write) = ~(*(pu1Write));
                        pu1Write++;
                        pu1Read++;
                    }
                }

                pu1SrcLine += u4SrcPitch;
                pu1DstLine += u4DstPitch;
            }
            break;
        case E_ROP_SRC_XOR_DST:       //src xor dst
            for(y = 0; y < u4Height ; y++)
            {
                pu1Read  = pu1SrcLine;
                pu1Write = pu1DstLine;

                for(x = 0; x < u4Width; x++)
                {
                    for(i = 0; i < u4ByteNum; i++)
                    {
                        *(pu1Write) = ( ~(*(pu1Read))&(*(pu1Write))) | ( ~(*(pu1Write))&(*(pu1Read)));
                        pu1Write++;
                        pu1Read++;
                    }
                }

                pu1SrcLine += u4SrcPitch;
                pu1DstLine += u4DstPitch;
            }
            break;

        case E_ROP_SRC_XNOR_DST:      //src xnor dst
            for(y = 0; y < u4Height ; y++)
            {
                pu1Read  = pu1SrcLine;
                pu1Write = pu1DstLine;

                for(x = 0; x < u4Width; x++)
                {
                    for(i = 0; i < u4ByteNum; i++)
                    {
                        *(pu1Write) = ((*(pu1Read))&(*(pu1Write))) | (( ~(*(pu1Write)))&(~(*(pu1Read))));
                        pu1Write++;
                        pu1Read++;
                    }
                }

                pu1SrcLine += u4SrcPitch;
                pu1DstLine += u4DstPitch;
            }
            break;

        case E_ROP_SRC_AND_DST:       //src & dst
            for(y = 0; y < u4Height ; y++)
            {
                pu1Read  = pu1SrcLine;
                pu1Write = pu1DstLine;

                for(x = 0; x < u4Width; x++)
                {
                    for(i = 0; i < u4ByteNum; i++)
                    {
                        *(pu1Write) = (*(pu1Write)) & (*(pu1Read));
                        pu1Write++;
                        pu1Read++;
                    }
                }

                pu1SrcLine += u4SrcPitch;
                pu1DstLine += u4DstPitch;
            }
            break;
        case E_ROP_NOT_SRC_AND_DST:        //~src & dst
            for(y = 0; y < u4Height ; y++)
            {
                pu1Read  = pu1SrcLine;
                pu1Write = pu1DstLine;

                for(x = 0; x < u4Width; x++)
                {
                    for(i = 0; i < u4ByteNum; i++)
                    {
                        *(pu1Write) = (~(*(pu1Read))) & (*(pu1Write));
                        pu1Write++;
                        pu1Read++;
                    }
                }

                pu1SrcLine += u4SrcPitch;
                pu1DstLine += u4DstPitch;
            }
            break;

        case E_ROP_SRC_AND_NOT_DST:         //src & ~dst
            for(y = 0; y < u4Height ; y++)
            {
                pu1Read  = pu1SrcLine;
                pu1Write = pu1DstLine;

                for(x = 0; x < u4Width; x++)
                {
                    for(i = 0; i < u4ByteNum; i++)
                    {
                        *(pu1Write) = (*(pu1Read)) & (~(*(pu1Write)));
                        pu1Write++;
                        pu1Read++;
                    }
                }

                pu1SrcLine += u4SrcPitch;
                pu1DstLine += u4DstPitch;
            }
            break;

        case E_ROP_NOT_SRC_AND_NOT_DST:      //~src & ~dst
            for(y = 0; y < u4Height ; y++)
            {
                pu1Read  = pu1SrcLine;
                pu1Write = pu1DstLine;

                for(x = 0; x < u4Width; x++)
                {
                    for(i = 0; i < u4ByteNum; i++)
                    {
                        *(pu1Write) = (~(*(pu1Read))) & (~(*(pu1Write)));
                        pu1Write++;
                        pu1Read++;
                    }
                }

                pu1SrcLine += u4SrcPitch;
                pu1DstLine += u4DstPitch;
            }
            break;

        case E_ROP_SRC_OR_DST:       //src | dst
            for(y = 0; y < u4Height ; y++)
            {
                pu1Read  = pu1SrcLine;
                pu1Write = pu1DstLine;

                for(x = 0; x < u4Width; x++)
                {
                    for(i = 0; i < u4ByteNum; i++)
                    {
                        *(pu1Write) = (*(pu1Write)) | (*(pu1Read));
                        pu1Write++;
                        pu1Read++;
                    }
                }

                pu1SrcLine += u4SrcPitch;
                pu1DstLine += u4DstPitch;

            }
            break;

        case E_ROP_NOT_SRC_OR_DST:      //~src | dst
            for(y = 0; y < u4Height ; y++)
            {
                pu1Read  = pu1SrcLine;
                pu1Write = pu1DstLine;

                for(x = 0; x < u4Width; x++)
                {
                    for(i = 0; i < u4ByteNum; i++)
                    {
                        *(pu1Write) = (~(*(pu1Read))) | (*(pu1Write));
                        pu1Write++;
                        pu1Read++;
                    }
                }

                pu1SrcLine += u4SrcPitch;
                pu1DstLine += u4DstPitch;
            }
            break;

        case E_ROP_SRC_OR_NOT_DST:       //src | ~dst
            for (y = 0; y < u4Height ; y++)
            {
                pu1Read  = pu1SrcLine;
                pu1Write = pu1DstLine;

                for (x = 0; x < u4Width; x++)
                {
                    for (i = 0; i < u4ByteNum; i++)
                    {
                        *(pu1Write) = (*(pu1Read)) | (~(*(pu1Write)));
                        pu1Write++;
                        pu1Read++;
                    }

                }

                pu1SrcLine += u4SrcPitch;
                pu1DstLine += u4DstPitch;
            }
            break;

        case E_ROP_NOT_SRC_OR_NOT_DST:       //~src | ~dst
            for(y = 0; y < u4Height ; y++)
            {
                pu1Read  = pu1SrcLine;
                pu1Write = pu1DstLine;

                for (x = 0; x < u4Width; x++)
                {
                    for(i = 0; i < u4ByteNum; i++)
                    {
                        *(pu1Write) = (~(*(pu1Read))) | (~(*(pu1Write)));
                        pu1Write++;
                        pu1Read++;
                    }

                }

                pu1SrcLine += u4SrcPitch;
                pu1DstLine += u4DstPitch;
            }
            break;

        default:
            return -(INT32)E_GFX_INV_ARG;
        }

    }
    return (INT32)E_GFX_OK;
}

INT32 GFX_SwRopBitBlt(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
                      UINT32 u4SrcPitch, UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM,
                      UINT32 u4Width, UINT32 u4Height, UINT32 u4RopOpCode, UINT32 u4nowr,
                      UINT32 u4cmpflag, UINT8 *pu1DstX, UINT8 *pu1DstY, UINT8 *pu1CmpNowr, UINT32 u4JavaXorClr)
{
    return GFX_SwRopBitBlt_0(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch, u4SrcCM, u4DstCM,
        u4Width, u4Height, u4RopOpCode, u4nowr, u4cmpflag, pu1DstX, pu1DstY, pu1CmpNowr, u4JavaXorClr, 0, 0);
}

// 8555 or newer Ic, or 8550 ECO IC
INT32 GFX_SwRopBitBlt8555(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
                      UINT32 u4SrcPitch, UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM,
                      UINT32 u4Width, UINT32 u4Height, UINT32 u4RopOpCode, UINT32 u4nowr,
                      UINT32 u4cmpflag, UINT8 *pu1DstX, UINT8 *pu1DstY, UINT8 *pu1CmpNowr, UINT32 u4JavaXorClr,
                      UINT32 u4SrcAlphaCheck)
{
    return GFX_SwRopBitBlt_0(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch, u4SrcCM, u4DstCM,
        u4Width, u4Height, u4RopOpCode, u4nowr, u4cmpflag, pu1DstX, pu1DstY, pu1CmpNowr, u4JavaXorClr, u4SrcAlphaCheck, 0);
}

// 8555 or newer Ic, or 8550 ECO IC
INT32 GFX_SwRopBitBlt8580(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
                      UINT32 u4SrcPitch, UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM,
                      UINT32 u4Width, UINT32 u4Height, UINT32 u4RopOpCode, UINT32 u4nowr,
                      UINT32 u4cmpflag, UINT8 *pu1DstX, UINT8 *pu1DstY, UINT8 *pu1CmpNowr, UINT32 u4JavaXorClr,
                      UINT32 u4SrcAlphaCheck, UINT32 u4ColorRepEn)
{
    return GFX_SwRopBitBlt_0(pu1SrcBase, pu1DstBase, u4SrcPitch, u4DstPitch, u4SrcCM, u4DstCM,
        u4Width, u4Height, u4RopOpCode, u4nowr, u4cmpflag, pu1DstX, pu1DstY, pu1CmpNowr, u4JavaXorClr, u4SrcAlphaCheck, u4ColorRepEn);
}


//add for inx2dir
INT32 GFX_SwInx2DirBitBlt (UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
  UINT32 u4SrcPitch,UINT32 u4DstPitch,UINT32 u4CharCM, UINT32 u4DstCM,
  UINT32 u4Width, UINT32 u4Height, UINT32 u4LnStByeAl, UINT32 u4MsbLeft,
  UINT8 *pu1PalBase)
{
    UINT8 *pu1SrcLine, *pu1DstLine;
    //UINT8 *pu1Pal;
    UINT8 *pu1Read, *pu1Write;
    UINT32 x, y, i;
    INT32 ai4RectColor[4];

    // check parameters' error
    VERIFY(pu1SrcBase != NULL);
    VERIFY(pu1DstBase != NULL);

    pu1SrcLine = pu1SrcBase;
    pu1DstLine = pu1DstBase;
    ////x_dbg_stmt( "GFX_SwInx2DirBitBlt \n");
    if (u4CharCM ==3)
    {
        /*x_dbg_stmt( "GFX_SwInx2DirBitBlt enter\n");
        x_dbg_stmt( "u4DstCM = %d\n",u4DstCM);
        x_dbg_stmt( "u4SrcPitch = %d\n",u4SrcPitch);
        x_dbg_stmt( "u4DstPitch = %d\n",u4DstPitch);
        x_dbg_stmt( "u4CharCM = %d\n",u4CharCM);
        x_dbg_stmt( "u4Width = %d\n",u4Width);
        x_dbg_stmt( "u4Height = %d\n",u4Height);
        x_dbg_stmt( "pu1SrcBase = %x\n",pu1SrcBase);
        x_dbg_stmt( "pu1DstBase = %x\n",pu1DstBase);
        */
        if(u4MsbLeft == 1)
        {
            for(y = 0; y < u4Height ; y++)
            {
                pu1Read  = pu1SrcLine;
                pu1Write = pu1DstLine;
                for (x = 0; x < u4Width; x++)
                {
                    //pu1Pal = pu1Write;
                    //x_dbg_stmt( "*pu1Read = %d\n",*pu1Read);
                    for(i = 0; i < 4; i++)
                    {
                        ai4RectColor[3-i] = *(pu1PalBase+(*pu1Read)*4+i);
                        //x_dbg_stmt( "*ai4RectColor[3-%d] = %d\n",i, ai4RectColor[3-i]);
                    }
                    pu1Read++;
                    GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM, ai4RectColor);
                     /*x_dbg_stmt( "*pu1Write-1 = %d\n",*(pu1Write-1));
                     x_dbg_stmt( "*pu1Write-2 = %d\n",*(pu1Write-2));
                     x_dbg_stmt( "*pu1Write-3 = %d\n",*(pu1Write-3));
                     x_dbg_stmt( "*pu1Write-4 = %d\n",*(pu1Write-4));

                     x_dbg_stmt( "pu1Write = %d\n", pu1Write);
                     */
                }
                pu1SrcLine += u4SrcPitch;
                pu1DstLine += u4DstPitch;


            }
        }
        else
        {
            for(y = 0; y < u4Height ; y++)
            {
                pu1Read  = pu1SrcLine;
                pu1Write = pu1DstLine;
                for (x = 0; x < u4Width; x++)
                {
                    //pu1Pal = pu1Write;
                    //x_dbg_stmt( "*pu1Read = %d\n",*pu1Read);
                    for(i = 0; i < 4; i++)
                    {
                        ai4RectColor[i] = *(pu1PalBase+(*pu1Read)*4+i);
                        //x_dbg_stmt( "*ai4RectColor[3-%d] = %d\n",i, ai4RectColor[3-i]);
                    }
                    pu1Read++;
                    GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM, ai4RectColor);
                     /*x_dbg_stmt( "*pu1Write-1 = %d\n",*(pu1Write-1));
                     x_dbg_stmt( "*pu1Write-2 = %d\n",*(pu1Write-2));
                     x_dbg_stmt( "*pu1Write-3 = %d\n",*(pu1Write-3));
                     x_dbg_stmt( "*pu1Write-4 = %d\n",*(pu1Write-4));

                     x_dbg_stmt( "pu1Write = %d\n", pu1Write);
                     */
                }
                pu1SrcLine += u4SrcPitch;
                pu1DstLine += u4DstPitch;
        }

        }
    }
    return (INT32)E_GFX_OK;
}

//add for h2vline
INT32 GFX_SwH2VLine(UINT8 *pu1SrcBase, UINT8 *pu1DstBase, UINT32 u4DstPitch,
                    UINT32 u4DstCM, UINT32 u4SrcWidth, UINT32 u4SrcHeight,
                    UINT32 u4StrDstWidth, UINT32 u4StrDstHeight, UINT32 u4Alpha, UINT32 u4wise)
{
    volatile UINT8 *pu1DstLine;
    volatile UINT8 *pu1Read, *pu1Write;
    volatile UINT32 x, i;
    volatile UINT32 u4ByteNum, u4PiexlLg;
  //x_dbg_stmt( "GFX_SwH2VLine \n");
  // check parameters' error
    VERIFY(pu1SrcBase != NULL);
    VERIFY(pu1DstBase != NULL);
  VERIFY(u4SrcWidth == u4StrDstHeight);
  VERIFY(u4SrcHeight == 1);
  VERIFY(u4StrDstWidth == 1);
  pu1Read = pu1SrcBase;
  pu1DstLine = pu1DstBase;
  u4PiexlLg = _au1sPixelLgDwn[u4DstCM];
  u4ByteNum = (u4PiexlLg == 0 ? 4 : (u4PiexlLg == 1 ? 2 : 1 ));
  if (u4wise ==0)
  {
    //x_dbg_stmt( "GFX_SwH2VLine clockwise \n");
    for (x = 0; x < u4SrcWidth; x++)
    {
      pu1Write = pu1DstLine;

      if (CM_ARGB1555_DIRECT16 == u4DstCM)
      {
          *pu1Write++ = *pu1Read++;
          *pu1Write = (0 == u4Alpha) ? *pu1Read : (*pu1Read | 0x80);
          pu1Read++;
          pu1Write++;
      }
      else
      {
      for(i = 0; i <u4ByteNum; i++)
      {
        *pu1Write = *pu1Read;
        pu1Read++;
        pu1Write++;
      }
      }
      pu1DstLine += u4DstPitch;
    }
  }
  if (u4wise ==1)
  {
      //x_dbg_stmt( "GFX_SwH2VLine counterclockwise \n");
    for (x = 0; x < u4SrcWidth; x++)
    {
      pu1Write = pu1DstLine;
      for(i = 0; i <u4ByteNum; i++)
      {
        *pu1Write = *pu1Read;
        pu1Read++;
        pu1Write++;
      }
      pu1DstLine -= u4DstPitch;
    }
  }
  return (INT32)E_GFX_OK;
}

//end of add by msz00441 9/18 for rop & inx2dir & h2vline

/*lint -restore */


//-------------------------------------------------------------------------
/** GFX_SwBlock2Linear
 *  re-arrange memory position from block mode to linear mode
 *  "Linear" means YC 420 separate raster
 */
//-------------------------------------------------------------------------
/*lint -save -e613 */
INT32 GFX_SwBlock2Linear(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    INT32 i4Width, INT32 i4Height, INT32 i4MBWidth, INT32 i4MBHeight)
{
    UINT8 *pu1SrcPtr, *pu1DstPtr, *pu1Anchor;
    INT32 i4MBSize, i4Vert_MB_Num, i4Hori_MB_Num;
    INT32 i, j, l;

    // check parameters' error
    VERIFY(pu1SrcBase != NULL);
    VERIFY(pu1DstBase != NULL);

    i4MBSize      = i4MBWidth * i4MBHeight;
    i4Hori_MB_Num = i4Width / i4MBWidth;
    i4Vert_MB_Num = i4Height / i4MBHeight;
    pu1DstPtr     = pu1DstBase;

    for (j = 0; j < i4Vert_MB_Num; j++)
    {
        pu1Anchor = pu1SrcBase + (j * i4Hori_MB_Num * i4MBSize);

        for(l = 0; l < i4MBHeight; l++)
        {
            pu1SrcPtr = pu1Anchor + (l * i4MBWidth);

            for (i = 0; i < i4Hori_MB_Num; i++)
            {
                GFX_UNUSED_RET(x_memcpy(pu1DstPtr, pu1SrcPtr,
                    (SIZE_T)i4MBWidth))

                pu1DstPtr += i4MBWidth;
                pu1SrcPtr += i4MBSize;
            }
        }
    } // ~for (j = 0; j < i4Vert_MB_Num; j++)

    return (INT32)E_GFX_OK;
}
/*lint -restore */


//-------------------------------------------------------------------------
/** GFX_SwBlock2Swap
 *  re-arrange memory position from block mode to swap mode
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e613 */
INT32 GFX_SwBlock2Swap(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    INT32 i4Width, INT32 i4Height, INT32 i4MBWidth, INT32 i4MBHeight)
{
    INT32 i, j;
    INT32 i4MBSize, i4Vert_MB_Num, i4Hori_MB_Num;
    UINT8 *pu1SrcPtr, *pu1DstPtr, *pu1Anchor;

    // check parameters' error
    VERIFY(pu1SrcBase != NULL);
    VERIFY(pu1DstBase != NULL);

    i4MBSize      = i4MBWidth * i4MBHeight;
    i4Hori_MB_Num = i4Width / i4MBWidth;
    i4Vert_MB_Num = i4Height / i4MBHeight;
    pu1DstPtr     = pu1DstBase;

    for (j = 0; j < i4Vert_MB_Num; j++)
    {
        pu1Anchor = pu1SrcBase + (j * i4Hori_MB_Num * i4MBSize);

        for (i = 0; i < i4Hori_MB_Num; i+=2)
        {
            pu1SrcPtr = pu1Anchor + ((i + 1) * i4MBSize);
            GFX_UNUSED_RET(x_memcpy(pu1DstPtr, pu1SrcPtr, (SIZE_T)i4MBSize))
            pu1DstPtr += i4MBSize;

            pu1SrcPtr = pu1Anchor + ((i + 0) * i4MBSize);
            GFX_UNUSED_RET(x_memcpy(pu1DstPtr, pu1SrcPtr, (SIZE_T)i4MBSize))
            pu1DstPtr += i4MBSize;
        }
    }

    return (INT32)E_GFX_OK;
}
/*lint -restore */


//-------------------------------------------------------------------------
/** GFX_SwBlock2Mergetop
 *  re-arrange memory position from block mode to mergetop mode
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e613 -e794 */
INT32 GFX_SwBlock2Mergetop(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    INT32 i4Width, INT32 i4Height, INT32 i4MBWidth, INT32 i4MBHeight)
{
    INT32 i, j, l;
    INT32 i4MBSize, i4Vert_MB_Num, i4Hori_MB_Num;
    UINT8 *pu1SrcPtr, *pu1DstPtr, *pu1Anchor, *pu1TempPtr;

    // check parameters' error
    VERIFY(pu1SrcBase != NULL);
    VERIFY(pu1DstBase != NULL);

    i4MBSize      = i4MBWidth * i4MBHeight;
    i4Hori_MB_Num = i4Width / i4MBWidth;
    i4Vert_MB_Num = i4Height / i4MBHeight;
    pu1DstPtr     = pu1DstBase;

    for (j = 0; j < i4Vert_MB_Num; j++)
    {
        pu1Anchor = pu1SrcBase + (j * i4Hori_MB_Num * i4MBSize);

        for (i = 0; i < i4Hori_MB_Num; i++)
        {
            pu1SrcPtr = pu1Anchor + (i * i4MBSize);

            // process odd lines
            for (l = 0; l < i4MBHeight; l+=2)
            {
                pu1TempPtr = pu1SrcPtr + (l * i4MBWidth);

                GFX_UNUSED_RET(x_memcpy(pu1DstPtr, pu1TempPtr,
                    (SIZE_T)i4MBWidth))

                pu1DstPtr += i4MBWidth;
            }

            // process even lines
            for (l = 1; l < i4MBHeight; l+=2)
            {
                pu1TempPtr = pu1SrcPtr + (l * i4MBWidth);

                GFX_UNUSED_RET(x_memcpy(pu1DstPtr, pu1TempPtr,
                    (SIZE_T)i4MBWidth))

                pu1DstPtr += i4MBWidth;
            }
        }
    }

    return (INT32)E_GFX_OK;
}
/*lint -restore */


//-------------------------------------------------------------------------
/** GFX_SwYCbCr420LinearScan
 *  do YCbCr420 linear scan
 *  (Y, Cb, Cr range from 16 to 235)
 */
//-------------------------------------------------------------------------
/*lint -save -e522 -e613 -e961 */
INT32 GFX_SwYCbCr420LinearScan(const GFX_YCBCR2RGB_DATA_T *prsData)
{
    UINT8 *pLumaBase, *pLumaPtr;
    UINT8 *pChromaBase, *pChromaPtr;
    UINT8 *pDstBase, *pDstPtr;
    UINT32 u4LumaPitch, u4ChromaPitch, u4DstPitch;
    UINT32 u4DstCM, i, j;
    UINT32 u4Width, u4Height;
    INT32 ai4Argb8888[4];
    INT32 (*pfnEquation)(const GFX_YCBCR_T *prsSrc, INT32 *pi4ColorComponent);
    GFX_YCBCR_T ycbcr1, ycbcr2;

    pLumaBase     = prsData->pu1LumaBase;
    pChromaBase   = prsData->pu1ChromaBase;
    pDstBase      = prsData->pu1DstBase;
    u4LumaPitch   = prsData->u4LumaPitch;
    u4ChromaPitch = prsData->u4ChromaPitch;
    u4DstPitch    = prsData->u4DstPitch;
    u4DstCM       = prsData->u4DstCM;
    u4Width       = prsData->u4Width;
    u4Height      = prsData->u4Height;

    // check parameters' error
    VERIFY(pLumaBase != NULL);
    VERIFY(pChromaBase != NULL);
    VERIFY(pDstBase != NULL);
    VERIFY(u4LumaPitch == u4ChromaPitch);

	UTIL_Printf("GFX_SW y2r !\n");
	
    // choose one equation
    if ((UINT32)E_VSTD_BT601 == prsData->u4VideoStd)
    {
        if ((UINT32)E_VSYS_VID == prsData->u4VideoSys)
        {
            pfnEquation = GFX_SwYCbCr601toArgbVideoEqu;
        }
        else if ((UINT32)E_VSYS_COMP == prsData->u4VideoSys)
        {
            pfnEquation = GFX_SwYCbCr601toArgbCompEqu;
        }
        else
        {
            return -(INT32)E_GFX_INV_ARG;
        }
    }
    else if ((UINT32)E_VSTD_BT709 == prsData->u4VideoStd)
    {
        if ((UINT32)E_VSYS_VID == prsData->u4VideoSys)
        {
            pfnEquation = GFX_SwYCbCr709toArgbVideoEqu;
        }
        else if ((UINT32)E_VSYS_COMP == prsData->u4VideoSys)
        {
            pfnEquation = GFX_SwYCbCr709toArgbCompEqu;
        }
        else
        {
            return -(INT32)E_GFX_INV_ARG;
        }
    }
    else
    {
        return -(INT32)E_GFX_INV_ARG;
    }


#if defined(GFX_SW_FLOAT_VERSION)
    // Error-rate statistic
    _u4YCbCr2Rgb_RedErrorCount = 0;
    _u4YCbCr2Rgb_GreenErrorCount = 0;
    _u4YCbCr2Rgb_BlueErrorCount = 0;
#endif

    for (j = 0; j < u4Height; j++)
    {
        pLumaPtr   = pLumaBase + (j * u4LumaPitch);
        pChromaPtr = pChromaBase + ((j/2) * u4ChromaPitch);
        pDstPtr    = pDstBase + (j * u4DstPitch);

        for (i = 0; i < u4Width; i += 2)
        {
            ycbcr2.cb = ycbcr1.cb = *(pChromaPtr + i + 0);
            ycbcr2.cr = ycbcr1.cr = *(pChromaPtr + i + 1);

            ycbcr1.y = *(pLumaPtr + i + 0);
            ycbcr2.y = *(pLumaPtr + i + 1);

            if (GFX_ENABLE == prsData->u4VideoClip)
            {
                // Y1
                ycbcr1.y  = (ycbcr1.y  > 235) ? 235 :
                            (ycbcr1.y  <  16) ?  16 : ycbcr1.y;
                ycbcr1.cb = (ycbcr1.cb > 240) ? 240 :
                            (ycbcr1.cb <  16) ?  16 : ycbcr1.cb;
                ycbcr1.cr = (ycbcr1.cr > 240) ? 240 :
                            (ycbcr1.cr <  16) ?  16 : ycbcr1.cr;
                // Y2
                ycbcr2.y  = (ycbcr2.y  > 235) ? 235 :
                            (ycbcr2.y  <  16) ?  16 : ycbcr2.y;
                ycbcr2.cb = (ycbcr2.cb > 240) ? 240 :
                            (ycbcr2.cb <  16) ?  16 : ycbcr2.cb;
                ycbcr2.cr = (ycbcr2.cr > 240) ? 240 :
                            (ycbcr2.cr <  16) ?  16 : ycbcr2.cr;
            }
            else if ((UINT32)E_VSYS_COMP == prsData->u4VideoSys)
            {
                // Y1
                //ycbcr1.y  = (ycbcr1.y  <  16) ? 16 : ycbcr1.y;
                if (ycbcr1.y  <  16)
                {
                    ycbcr1.y = 16;
                }

                // Y2
                //ycbcr2.y  = (ycbcr2.y  <  16) ? 16 : ycbcr2.y;
                if (ycbcr2.y  <  16)
                {
                    ycbcr2.y = 16;
                }
            }
            else    // for lint happy
            {
                ;
            }

            (*pfnEquation)(&ycbcr1, ai4Argb8888);
            GFX_SwSetColorComponent(&pDstPtr, u4DstCM, ai4Argb8888);

            if (i != u4Width-1)
            {
            (*pfnEquation)(&ycbcr2, ai4Argb8888);
            GFX_SwSetColorComponent(&pDstPtr, u4DstCM, ai4Argb8888);
            }
        }
    }

    return (INT32)E_GFX_OK;
}
/*lint -restore */


//-------------------------------------------------------------------------
/** GFX_SwYCbCr601toArgbVideoEqu
 *  Y(601)CbCr -> ARGB video system equation
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e704 -e834 -e961 -e613 */
INT32 GFX_SwYCbCr601toArgbVideoEqu(const GFX_YCBCR_T *prsSrc,
    INT32 *pi4ColorComponent)
{
    INT32 i4Temp = 0;
    INT32 i4Y, i4Cb, i4Cr;

	//UTIL_Printf("GFX_SW y2r _601 video equ!\n");

    i4Y  = (INT32)prsSrc->y;
    i4Cb = (INT32)prsSrc->cb;
    i4Cr = (INT32)prsSrc->cr;
    pi4ColorComponent[0] = (INT32)0xFF;

    // Integer Version (like HW)
    i4Temp = (128 * i4Y) + (175 * (i4Cr - 128));
    i4Temp = i4Temp >> 7;       // i4Temp / 128
    pi4ColorComponent[1] = (i4Temp > 255) ? 255 :
                           (i4Temp <   0) ?   0 : i4Temp;

    i4Temp = (128 * i4Y) - (43 * (i4Cb - 128)) - (89 * (i4Cr - 128));
    i4Temp = i4Temp >> 7;       // i4Temp / 128
    pi4ColorComponent[2] = (i4Temp > 255) ? 255 :
                           (i4Temp <   0) ?   0 : i4Temp;

    i4Temp = (128 * i4Y) + (222 * (i4Cb - 128));
    i4Temp = i4Temp >> 7;       // i4temp / 128
    pi4ColorComponent[3] = (i4Temp > 255) ? 255 :
                           (i4Temp <   0) ?   0 : i4Temp;

#if defined(GFX_SW_FLOAT_VERSION)
{
    INT32 ai4Array[4];

    i4Temp = (INT32)(i4Y + (1.371 * (i4Cr - 128)));

    ai4Array[1] = (i4Temp > 255) ? 255 :
                  (i4Temp <   0) ?   0 : i4Temp;

    i4Temp = (INT32)(i4Y - (0.698 * (i4Cr - 128)) - (0.336 * (i4Cb - 128)));

    ai4Array[2] = (i4Temp > 255) ? 255 :
                  (i4Temp <   0) ?   0 : i4Temp;

    i4Temp = (INT32)(i4Y + (1.732 * (i4Cb - 128)));

    ai4Array[3] = (i4Temp > 255) ? 255 :
                  (i4Temp <   0) ?   0 : i4Temp;
    // Compare
    if (ABS(pi4ColorComponent[1] - ai4Array[1]) > 1)
    {
        _u4YCbCr2Rgb_RedErrorCount++;
        UNUSED(_u4YCbCr2Rgb_RedErrorCount);
    }

    if (ABS(pi4ColorComponent[2] - ai4Array[2]) > 1)
    {
        _u4YCbCr2Rgb_GreenErrorCount++;
        UNUSED(_u4YCbCr2Rgb_GreenErrorCount);
    }

    if (ABS(pi4ColorComponent[3] - ai4Array[3]) > 1)
    {
        _u4YCbCr2Rgb_BlueErrorCount++;
        UNUSED(_u4YCbCr2Rgb_BlueErrorCount);
    }
}
#endif

    return (INT32)E_GFX_OK;
}
/*lint -restore */


//-------------------------------------------------------------------------
/** GFX_SwYCbCr601toArgbCompEqu
 *  Y(601)CbCr -> ARGB computer system equation
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e704 -e834 -e961 -e613 */
INT32 GFX_SwYCbCr601toArgbCompEqu(const GFX_YCBCR_T *prsSrc,
    INT32 *pi4ColorComponent)
{
    INT32 i4Temp = 0;
    INT32 i4Y, i4Cb, i4Cr;

	//UTIL_Printf("GFX_SW y2r 601 comp equ!\n");
	
    i4Y  = (INT32)prsSrc->y;
    i4Cb = (INT32)prsSrc->cb;
    i4Cr = (INT32)prsSrc->cr;
    pi4ColorComponent[0] = (INT32)0xFF;

    // Integer Version (like HW)
    i4Temp = (149 * (i4Y - 16)) + (204 * (i4Cr - 128));
    i4Temp = i4Temp >> 7;       // i4Temp / 128
    pi4ColorComponent[1] = (i4Temp > 255) ? 255 :
                           (i4Temp <   0) ?   0 : i4Temp;

    i4Temp = (149 * (i4Y - 16)) - (50 * (i4Cb - 128)) - (104 * (i4Cr - 128));
    i4Temp = i4Temp >> 7;       // i4Temp / 128
    pi4ColorComponent[2] = (i4Temp > 255) ? 255 :
                           (i4Temp <   0) ?   0 : i4Temp;

    i4Temp = (149 * (i4Y - 16)) + (258 * (i4Cb - 128));
    i4Temp = i4Temp >> 7;       // i4Temp / 128
    pi4ColorComponent[3] = (i4Temp > 255) ? 255 :
                           (i4Temp <   0) ?   0 : i4Temp;

#if defined(GFX_SW_FLOAT_VERSION)
{
    INT32 ai4Array[4];

    // Floating-Point Version
/* maked by Victor Lin, 20101128, for fix build fail, kernel should not use float
    i4Temp = (INT32)((1.164 * (i4Y - 16)) + (1.596 * (i4Cr - 128)));
*/
    ai4Array[1] = (i4Temp > 255) ? 255 :
                  (i4Temp <   0) ?   0 : i4Temp;

/* maked by Victor Lin, 20101128, for fix build fail, kernel should not use float
    i4Temp = (INT32)((1.164 * (i4Y - 16)) -
                    (0.813 * (i4Cr - 128)) - (0.391 * (i4Cb - 128)));
*/
    ai4Array[2] = (i4Temp > 255) ? 255 :
                  (i4Temp <   0) ?   0 : i4Temp;

/* maked by Victor Lin, 20101128, for fix build fail, kernel should not use float
    i4Temp = (INT32)((1.164 * (i4Y - 16)) + (2.018 * (i4Cb - 128)));
*/
    ai4Array[3] = (i4Temp > 255) ? 255 :
                  (i4Temp <   0) ?   0 : i4Temp;
    // Compare
    if (ABS(pi4ColorComponent[1] - ai4Array[1]) > 1)
    {
        _u4YCbCr2Rgb_RedErrorCount++;
    }

    if (ABS(pi4ColorComponent[2] - ai4Array[2]) > 1)
    {
        _u4YCbCr2Rgb_GreenErrorCount++;
    }

    if (ABS(pi4ColorComponent[3] - ai4Array[3]) > 1)
    {
        _u4YCbCr2Rgb_BlueErrorCount++;
    }
}
#endif

    return (INT32)E_GFX_OK;
}
/*lint -restore */


//-------------------------------------------------------------------------
/** GFX_SwYCbCr709toArgbVideoEqu
 *  Y(709)CbCr -> ARGB video system equation
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e704 -e834 -e961 -e613 */
INT32 GFX_SwYCbCr709toArgbVideoEqu(const GFX_YCBCR_T *prsSrc,
    INT32 *pi4ColorComponent)
{
    INT32 i4Temp = 0;
    INT32 i4Y, i4Cb, i4Cr;

	//UTIL_Printf("GFX_SW y2r 709 video equ!\n");
	
    i4Y  = (INT32)prsSrc->y;
    i4Cb = (INT32)prsSrc->cb;
    i4Cr = (INT32)prsSrc->cr;
    pi4ColorComponent[0] = (INT32)0xFF;

    // Integer Version (like HW)
    i4Temp = (128 * i4Y) + (197 * (i4Cr - 128));
    i4Temp = i4Temp >> 7;       // i4Temp / 128
    pi4ColorComponent[1] = (i4Temp > 255) ? 255 :
                           (i4Temp <   0) ?   0 : i4Temp;

    i4Temp = (128 * i4Y) - (23 * (i4Cb - 128)) - (59 * (i4Cr - 128));
    i4Temp = i4Temp >> 7;       // i4Temp / 128
    pi4ColorComponent[2] = (i4Temp > 255) ? 255 :
                           (i4Temp <   0) ?   0 : i4Temp;

    i4Temp = (128 * i4Y) + (232 * (i4Cb - 128));
    i4Temp = i4Temp >> 7;       // i4Temp / 128
    pi4ColorComponent[3] = (i4Temp > 255) ? 255 :
                           (i4Temp <   0) ?   0 : i4Temp;

#if defined(GFX_SW_FLOAT_VERSION)
{
    INT32 ai4Array[4];

    // Floating-Point Version
/* maked by Victor Lin, 20101128, for fix build fail, kernel should not use float
    i4Temp = (INT32)(i4Y + (1.54 * (i4Cr - 128)));
*/
    ai4Array[1] = (i4Temp > 255) ? 255 :
                  (i4Temp <   0) ?   0 : i4Temp;

/* maked by Victor Lin, 20101128, for fix build fail, kernel should not use float
    i4Temp = (INT32)(i4Y - (0.459 * (i4Cr - 128)) - (0.183 * (i4Cb - 128)));
*/
    ai4Array[2] = (i4Temp > 255) ? 255 :
                  (i4Temp <   0) ?   0 : i4Temp;

/* maked by Victor Lin, 20101128, for fix build fail, kernel should not use float
    i4Temp = (INT32)(i4Y + (1.816 * (i4Cb - 128)));
*/
    ai4Array[3] = (i4Temp > 255) ? 255 :
                  (i4Temp <   0) ?   0 : i4Temp;
    // Compare
    if (ABS(pi4ColorComponent[1] - ai4Array[1]) > 1)
    {
        _u4YCbCr2Rgb_RedErrorCount++;
    }

    if (ABS(pi4ColorComponent[2] - ai4Array[2]) > 1)
    {
        _u4YCbCr2Rgb_GreenErrorCount++;
    }

    if (ABS(pi4ColorComponent[3] - ai4Array[3]) > 1)
    {
        _u4YCbCr2Rgb_BlueErrorCount++;
    }
}
#endif

    return (INT32)E_GFX_OK;
}
/*lint -restore */


//-------------------------------------------------------------------------
/** GFX_SwYCbCr709toArgbCompEqu
 *  Y(709)CbCr -> ARGB computer system equation
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e704 -e834 -e961 -e613 */
INT32 GFX_SwYCbCr709toArgbCompEqu(const GFX_YCBCR_T *prsSrc,
    INT32 *pi4ColorComponent)
{
    INT32 i4Temp = 0;
    INT32 i4Y, i4Cb, i4Cr;

	//UTIL_Printf("GFX_SW y2r 709 comp equ!\n");
	
    i4Y  = (INT32)prsSrc->y;
    i4Cb = (INT32)prsSrc->cb;
    i4Cr = (INT32)prsSrc->cr;
    pi4ColorComponent[0] = (INT32)0xFF;

    // Integer Version (like HW)
    i4Temp = (149 * (i4Y - 16)) + (230 * (i4Cr - 128));
    i4Temp = i4Temp >> 7;       // ftemp / 128
    pi4ColorComponent[1] = (i4Temp > 255) ? 255 :
                           (i4Temp <   0) ?   0 : i4Temp;

    i4Temp = (149 * (i4Y - 16)) - (27 * (i4Cb - 128)) - (68 * (i4Cr - 128));
    i4Temp = i4Temp >> 7;       // ftemp / 128
    pi4ColorComponent[2] = (i4Temp > 255) ? 255 :
                           (i4Temp <   0) ?   0 : i4Temp;

    i4Temp = (149 * (i4Y - 16)) + (271 * (i4Cb - 128));
    i4Temp = i4Temp >> 7;       // ftemp / 128
    pi4ColorComponent[3] = (i4Temp > 255) ? 255 :
                           (i4Temp <   0) ?   0 : i4Temp;

#if defined(GFX_SW_FLOAT_VERSION)
{
    INT32 ai4Array[4];

    // Floating-Point Version
/* maked by Victor Lin, 20101128, for fix build fail, kernel should not use float
    i4Temp = (INT32)((1.164 * (i4Y - 16)) + (1.793 * (i4Cr - 128)));
*/
    ai4Array[1] = (i4Temp > 255) ? 255 :
                  (i4Temp <   0) ?   0 : i4Temp;

/* maked by Victor Lin, 20101128, for fix build fail, kernel should not use float
    i4Temp = (INT32)((1.164 * (i4Y - 16)) -
                    (0.534 * (i4Cr - 128)) - (0.213 * (i4Cb - 128)));
*/
    ai4Array[2] = (i4Temp > 255) ? 255 :
                  (i4Temp <   0) ?   0 : i4Temp;

/* maked by Victor Lin, 20101128, for fix build fail, kernel should not use float
    i4Temp = (INT32)((1.164 * (i4Y - 16)) + (2.115 * (i4Cb - 128)));
*/
    ai4Array[3] = (i4Temp > 255) ? 255 :
                  (i4Temp <   0) ?   0 : i4Temp;
    // Compare
    if (ABS(pi4ColorComponent[1] - ai4Array[1]) > 1)
    {
        _u4YCbCr2Rgb_RedErrorCount++;
    }

    if (ABS(pi4ColorComponent[2] - ai4Array[2]) > 1)
    {
        _u4YCbCr2Rgb_GreenErrorCount++;
    }

    if (ABS(pi4ColorComponent[3] - ai4Array[3]) > 1)
    {
        _u4YCbCr2Rgb_BlueErrorCount++;
    }
}
#endif  //#if defined(GFX_SW_FLOAT_VERSION)

    return (INT32)E_GFX_OK;
}
/*lint -restore */


//-------------------------------------------------------------------------
/** GFX_SwYCbCr2RGB
 *  do YCbCr to RGB
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e613 */
INT32 GFX_SwYCbCr2RGB(const GFX_YCBCR2RGB_DATA_T *prsData)
{
    // check parameters' error
    VERIFY(prsData != NULL);
    VERIFY(prsData->pu1LumaBase != NULL);
    VERIFY(prsData->pu1ChromaBase != NULL);
    VERIFY(prsData->pu1DstBase != NULL);

    if (prsData->u4YCFormat >= (UINT32)E_YCFMT_422LINEAR)
    {
        // MT5371 does NOT support 422 Linear Scan
        return -(INT32)E_GFX_INV_ARG;
    }

    if (prsData->u4YCFormat == (UINT32)E_YCFMT_420LINEAR)
    {
        GFX_UNUSED_RET(GFX_SwYCbCr420LinearScan(prsData))
    }
    else if (prsData->u4YCFormat == (UINT32)E_YCFMT_420MB)
    {
        ;
    }
    else            // for lint happy
    {
        ;
    }

    return (INT32)E_GFX_OK;
}
/*lint -restore */


//**************************************************************************
// *GFX_SwMsAlphaBlending
//* By Sxj
//*20120203
// **************************************************************************
INT32 GFX_SwMsAlphaBlending(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    UINT32 u4SrcPitch, UINT32 u4DstPitch, UINT32 u4Dst_CM, UINT32 u4Alpha,
    UINT32 u4SrcPremult, UINT32 u4Width, UINT32 u4Height)
{
    UINT8 *pu1SrcLine, *pu1DstLine;
	UINT8 *pu1Read, *pu1Write, *pu1Dst;
	UINT32 u4Alpha0, u4Alpha1, x, y;
	INT32 ai4SrcColor[4], ai4DstColor[4], i;

	//get source & destination info
	u4Alpha = (u4Alpha==255) ? 256 : u4Alpha;
	pu1SrcLine = pu1SrcBase;
	pu1Read    = pu1SrcBase;
	pu1DstLine = pu1DstBase;
	pu1Write   = pu1DstBase;
	
    VERIFY(pu1SrcBase != NULL);
    VERIFY(pu1DstBase != NULL);
    //HalFlushDCache();
    // HalInvalidateDCache();
    HalFlushInvalidateDCache();

	if(u4SrcPremult)  //
	{
	     for(y=0; y<u4Height; y++)
	     {
	         for(x=0; x<u4Width; x++)
	         {
	            pu1Dst = pu1Write;
			    GFX_SwGetColorComponent(&pu1Read, u4Dst_CM, ai4SrcColor);
				GFX_SwGetColorComponent(&pu1Write, u4Dst_CM, ai4DstColor);

				ai4SrcColor[0] = (ai4SrcColor[0] == 255) ? 256 : ai4SrcColor[0];
				ai4DstColor[0] = (ai4DstColor[0] == 255) ? 256 : ai4DstColor[0];
				for(i=0; i<4; i++)
				{
				    ai4SrcColor[i] = (ai4SrcColor[i]*(INT32)u4Alpha);
					u4Alpha0 = ai4SrcColor[0];
					u4Alpha1 = u4Alpha0 ? (65536 - u4Alpha0) : 65536;
					ai4DstColor[i] = ai4DstColor[i]*u4Alpha1;
					ai4DstColor[i] += ai4SrcColor[i]*256;
					
					//rounding
					if((ai4DstColor[i] >> 15) & 1)
					{
					    ai4DstColor[i] = (ai4DstColor[i] >> 16) + 1;
					}
					else
					{
					    ai4DstColor[i] = ai4DstColor[i] >> 16;
					}
				}
				GFX_SwSetColorComponent(&pu1Dst, u4Dst_CM, ai4DstColor);
	         }
			 pu1SrcLine += u4SrcPitch;
			 pu1DstLine += u4DstPitch;
			 pu1Read     = pu1SrcLine;
			 pu1Write    = pu1DstLine;
		 }
	}
	else
	{
		for(y=0; y<u4Height; y++)
				{
					for(x=0; x<u4Width; x++)
					{
					   pu1Dst = pu1Write;
					   GFX_SwGetColorComponent(&pu1Read, u4Dst_CM, ai4SrcColor);
					   GFX_SwGetColorComponent(&pu1Write, u4Dst_CM, ai4DstColor);
		
					   ai4SrcColor[0] = (ai4SrcColor[0] == 255) ? 256 : ai4SrcColor[0];
					   ai4DstColor[0] = (ai4DstColor[0] == 255) ? 256 : ai4DstColor[0];
					   for(i=0; i<4; i++)
					   {
						   ai4SrcColor[i] = (ai4SrcColor[i]*256);
						   u4Alpha0 = ai4SrcColor[0];
						   u4Alpha1 = u4Alpha0 ? (65536 - u4Alpha0) : 65536;
						   ai4DstColor[i] = ai4DstColor[i]*u4Alpha1;
						   ai4DstColor[i] += ai4SrcColor[i]*256;
						   
						   //rounding
						   if((ai4DstColor[i] >> 15) & 1)
						   {
							   ai4DstColor[i] = (ai4DstColor[i] >> 16) + 1;
						   }
						   else
						   {
							   ai4DstColor[i] = ai4DstColor[i] >> 16;
						   }
					   }
					   GFX_SwSetColorComponent(&pu1Dst, u4Dst_CM, ai4DstColor);
					}
					pu1SrcLine += u4SrcPitch;
					pu1DstLine += u4DstPitch;
					pu1Read 	= pu1SrcLine;
					pu1Write	= pu1DstLine;
				}

	}
	return (INT32)E_GFX_OK;
    
}


//-------------------------------------------------------------------------
/** GFX_SwAlphaBlending
 *
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e704 -e613 */
INT32 GFX_SwAlphaBlending(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    UINT32 u4SrcPitch, UINT32 u4DstPitch, UINT32 u4Dst_CM, UINT32 u4Alpha,
    UINT32 u4Width, UINT32 u4Height)
{
    UINT8 *pu1SrcLine, *pu1DstLine;
    UINT8 *pu1Read, *pu1Write, *pu1Dst;
    UINT32 u4Alpha1, x, y;
    INT32 ai4SrcColor[4], ai4DstColor[4], i;

    // get source & destination info
    u4Alpha  = (u4Alpha == 255) ? 256 : u4Alpha;   // Ar
    u4Alpha1 = (u4Alpha) ? (256 - u4Alpha) : 256;  // (1-Ar)

    pu1Read    = pu1SrcBase;
    pu1SrcLine = pu1SrcBase;
    pu1Write   = pu1DstBase;
    pu1DstLine = pu1DstBase;

    VERIFY(pu1SrcBase != NULL);
    VERIFY(pu1DstBase != NULL);
    //HalFlushDCache();
   // HalInvalidateDCache();
    HalFlushInvalidateDCache();
   // BIM_WAIT_WALE();
    for (y = 0; y < u4Height; y++)
    {
        for (x = 0; x < u4Width; x++)
        {
            pu1Dst = pu1Write;

            GFX_SwGetColorComponent(&pu1Read, u4Dst_CM, ai4SrcColor);
            GFX_SwGetColorComponent(&pu1Dst,  u4Dst_CM, ai4DstColor);

            ai4SrcColor[0] = (ai4SrcColor[0] == 255) ? 256 : ai4SrcColor[0];
            ai4DstColor[0] = (ai4DstColor[0] == 255) ? 256 : ai4DstColor[0];

            for (i = 0; i < 4; i++)
            {
                ai4SrcColor[i]  = (ai4SrcColor[i] * (INT32)u4Alpha);
                ai4DstColor[i]  = (ai4DstColor[i] * (INT32)u4Alpha1);
                ai4DstColor[i] += ai4SrcColor[i];

                // Rounding
                if ((ai4DstColor[i] >> 7) & 1)
                {
                    ai4DstColor[i] = (ai4DstColor[i] >> 8) + 1;
                }
                else
                {
                    ai4DstColor[i] = (ai4DstColor[i] >> 8);
                }
            }

            GFX_SwSetColorComponent(&pu1Write, u4Dst_CM, ai4DstColor);
        }

        pu1SrcLine += u4SrcPitch;
        pu1DstLine += u4DstPitch;
        pu1Read     = pu1SrcLine;
        pu1Write    = pu1DstLine;
    }

    return (INT32)E_GFX_OK;
}
/*lint -restore */

#if 0
//-------------------------------------------------------------------------
/** GFX_SwBigEndianAndLittleEndianConversion
 *
 *
 */
//-------------------------------------------------------------------------
/*lint -save -e826 -e613 */
INT32 GFX_SwBigEndianAndLittleEndianConversion(UINT8 *pu1DstAddr,
    UINT8 *pu1SrcAddr, UINT32 u4TotalPixels, UINT32 u4ColorMode)
{
    UINT32 p, u4Color;
    UINT32 u4Byte0, u4Byte1, u4Byte2, u4Byte3;
    UINT32 *pu4DstPtr, *pu4SrcPtr;

    UINT16 u2Color;
    UINT16 *pu2DstPtr, *pu2SrcPtr;

    switch (u4ColorMode)
    {
    case CM_RGB565_DIRECT16:
        break;

    case CM_ARGB1555_DIRECT16:
        break;

    case CM_ARGB4444_DIRECT16:
        pu2DstPtr = ((UINT16 *)pu1DstAddr);
        pu2SrcPtr = ((UINT16 *)pu1SrcAddr);

        for (p = 0; p < u4TotalPixels; p++)
        {
            u2Color = *pu2SrcPtr;

            // Seperate
            u4Byte0 = u2Color >> 12;
            u4Byte1 = (u2Color >> 8) & 0xf;
            u4Byte2 = (u2Color >> 4) & 0xf;
            u4Byte3 = u2Color & 0xf;

            // Merge
            u2Color = ((u4Byte3 << 12) | (u4Byte2 << 8) |
                       (u4Byte1 <<  4) | (u4Byte0));

            *pu2DstPtr = u2Color;

            pu2DstPtr++;
            pu2SrcPtr++;
        }
        break;

    case CM_ARGB8888_DIRECT32:
        pu4DstPtr = ((UINT32 *)pu1DstAddr);
        pu4SrcPtr = ((UINT32 *)pu1SrcAddr);

        for (p = 0; p < u4TotalPixels; p++)
        {
            u4Color = *pu4SrcPtr;

            // Seperate
            u4Byte0 = u4Color >> 24;
            u4Byte1 = (u4Color >> 16) & 0xff;
            u4Byte2 = (u4Color >>  8) & 0xff;
            u4Byte3 = u4Color & 0xff;

            // Merge
            u4Color = ((u4Byte3 << 24) | (u4Byte2 << 16) |
                       (u4Byte1 <<  8) | (u4Byte0));

            *pu4DstPtr = u4Color;

            pu4DstPtr++;
            pu4SrcPtr++;
        }
        break;

    default:
        return -(INT32)E_GFX_INV_ARG;
    }

    return (INT32)E_GFX_OK;
}
/*lint -restore */
#endif

//-------------------------------------------------------------------------
/** GFX_SwGradientFill
 *
 *
 */
//-------------------------------------------------------------------------
#if (!CONFIG_DRV_LINUX)

FLOAT GFX_SwFpS61toFloat(INT8 i1Value);
FLOAT GFX_SwClipping(FLOAT fValue);

/*lint -save -e613 -e740 */
INT32 GFX_SwGradientFill(const GFX_GRADIENT_DATA_T *prsData)
{
    UINT8 *pu1Write;
    UINT32 *pu4Color;
    UINT32 u4XInc, u4YInc;
    UINT32 u4Width, u4Height;
    UINT32 u4DstCM, u4RectColor, u4GradMode;
    INT32 i4ColorComponent[4], i4TempColor[4];
    FLOAT f4DeltaX[4], f4DeltaY[4], f4Temp[4];
    FLOAT f4TempColor[4];
    UINT32 i, j, xinc = 0, yinc = 0;

    // check parameters' error
    VERIFY(prsData != NULL);
    VERIFY(prsData->pu1DstBase != NULL);

    pu1Write    = prsData->pu1DstBase;
    u4XInc      = prsData->u4X_Pix_Inc;
    u4YInc      = prsData->u4Y_Pix_Inc;
    u4Width     = prsData->u4Width;
    u4Height    = prsData->u4Height;
    u4DstCM     = prsData->u4DstCM;
    u4RectColor = prsData->u4RectColor;
    u4GradMode  = prsData->u4GradMode;

    pu4Color    = &u4RectColor;
    u4RectColor = _GfxSwColorExpansion(u4RectColor, u4DstCM);

    GFX_SwGetColorComponent((UINT8 **)&pu4Color, u4DstCM, i4ColorComponent);

    f4DeltaX[0] = GFX_SwFpS61toFloat((INT8)prsData->u1Delta_X_C0); // B
    f4DeltaX[1] = GFX_SwFpS61toFloat((INT8)prsData->u1Delta_X_C1); // G
    f4DeltaX[2] = GFX_SwFpS61toFloat((INT8)prsData->u1Delta_X_C2); // R
    f4DeltaX[3] = GFX_SwFpS61toFloat((INT8)prsData->u1Delta_X_C3); // A

    f4DeltaY[0] = GFX_SwFpS61toFloat((INT8)prsData->u1Delta_Y_C0); // B
    f4DeltaY[1] = GFX_SwFpS61toFloat((INT8)prsData->u1Delta_Y_C1); // G
    f4DeltaY[2] = GFX_SwFpS61toFloat((INT8)prsData->u1Delta_Y_C2); // R
    f4DeltaY[3] = GFX_SwFpS61toFloat((INT8)prsData->u1Delta_Y_C3); // A

    //HalFlushDCache();
    //HalInvalidateDCache();
     HalFlushInvalidateDCache();
   // BIM_WAIT_WALE();
    for (j = 0; j < u4Height; j++)
    {
        yinc = (j / u4YInc);
        pu1Write = prsData->pu1DstBase + (j * prsData->u4DstPitch);

        for (i = 0; i < u4Width; i++)
        {
            xinc = (i / u4XInc);

            f4TempColor[0] = (FLOAT) i4ColorComponent[0];
            f4TempColor[1] = (FLOAT) i4ColorComponent[1];
            f4TempColor[2] = (FLOAT) i4ColorComponent[2];
            f4TempColor[3] = (FLOAT) i4ColorComponent[3];

            // calculate vertical direction first
            if (u4GradMode & (UINT32)E_GRAD_VER)
            {
                f4Temp[3] = (f4DeltaY[3] * (FLOAT)yinc); // A
                f4Temp[2] = (f4DeltaY[2] * (FLOAT)yinc); // R
                f4Temp[1] = (f4DeltaY[1] * (FLOAT)yinc); // G
                f4Temp[0] = (f4DeltaY[0] * (FLOAT)yinc); // B

                f4TempColor[0] = GFX_SwClipping(f4TempColor[0] + f4Temp[3]);
                f4TempColor[1] = GFX_SwClipping(f4TempColor[1] + f4Temp[2]);
                f4TempColor[2] = GFX_SwClipping(f4TempColor[2] + f4Temp[1]);
                f4TempColor[3] = GFX_SwClipping(f4TempColor[3] + f4Temp[0]);
            }

            if (u4GradMode & (UINT32)E_GRAD_HOR)
            {
                f4Temp[3] = (f4DeltaX[3] * (FLOAT)xinc); // A
                f4Temp[2] = (f4DeltaX[2] * (FLOAT)xinc); // R
                f4Temp[1] = (f4DeltaX[1] * (FLOAT)xinc); // G
                f4Temp[0] = (f4DeltaX[0] * (FLOAT)xinc); // B

                f4TempColor[0] = GFX_SwClipping(f4TempColor[0] + f4Temp[3]);
                f4TempColor[1] = GFX_SwClipping(f4TempColor[1] + f4Temp[2]);
                f4TempColor[2] = GFX_SwClipping(f4TempColor[2] + f4Temp[1]);
                f4TempColor[3] = GFX_SwClipping(f4TempColor[3] + f4Temp[0]);
            }

            // translate floating-point to integer
            i4TempColor[0] = (INT32) f4TempColor[0]; // A
            i4TempColor[1] = (INT32) f4TempColor[1]; // R
            i4TempColor[2] = (INT32) f4TempColor[2]; // G
            i4TempColor[3] = (INT32) f4TempColor[3]; // B

            // write value to dst
            GFX_SwSetColorComponent(&pu1Write, u4DstCM, i4TempColor);
        }
    }

    return (INT32)E_GFX_OK;
}
/*lint -restore */
//-------------------------------------------------------------------------
/** GFX_SwFpS61toFloat
 *  Translate fixed-point s6.1 to floating-point
 *
 */
//-------------------------------------------------------------------------
FLOAT GFX_SwFpS61toFloat(INT8 i1Value)
{
    FLOAT fResult = (FLOAT)i1Value / (FLOAT)2;

    return fResult;
}


//-------------------------------------------------------------------------
/** GFX_SwClipping
 *  Clip value within 0 ~ 255
 *
 */
//-------------------------------------------------------------------------
FLOAT GFX_SwClipping(FLOAT fValue)
{
    if (fValue > 255.0)
    {
        return 255.0;
    }
    else if (fValue < 0.0)
    {
        return 0.0;
    }
    else
    {
        return fValue;
    }
}
#else

#define GF_MULTI    30

INT64 GFX_SwFpS61toFloat(INT64 i1Value)
    {
    INT64 fResult = i1Value >> 1;

    return fResult;
    }

INT64 GFX_SwClipping(INT64 i4Value)
{
    if (i4Value < 0)
    {
        return 0;
    }
    else if (i4Value > ((INT64)(255) << GF_MULTI))
    {
        return ((INT64)(255) << GF_MULTI);
    }
    else
    {
        return i4Value;
    }
}

INT32 GFX_SwGradientFill(const GFX_GRADIENT_DATA_T *prsData)
{
    UINT8 *pu1Write;
    UINT32 *pu4Color;
    UINT32 u4XInc, u4YInc;
    UINT32 u4Width, u4Height;
    UINT32 u4DstCM, u4RectColor, u4GradMode;
    INT32 i4ColorComponent[4], i4TempColor[4];
    INT64 f4DeltaX[4], f4DeltaY[4], f4Temp[4];
    INT64 f4TempColor[4];
    UINT32 i, j, xinc = 0, yinc = 0;

    // check parameters' error
    VERIFY(prsData != NULL);
    VERIFY(prsData->pu1DstBase != NULL);

    pu1Write    = prsData->pu1DstBase;
    u4XInc      = prsData->u4X_Pix_Inc;
    u4YInc      = prsData->u4Y_Pix_Inc;
    u4Width     = prsData->u4Width;
    u4Height    = prsData->u4Height;
    u4DstCM     = prsData->u4DstCM;
    u4RectColor = prsData->u4RectColor;
    u4GradMode  = prsData->u4GradMode;

    pu4Color    = &u4RectColor;
    u4RectColor = _GfxSwColorExpansion(u4RectColor, u4DstCM);

    GFX_SwGetColorComponent((UINT8 **)&pu4Color, u4DstCM, i4ColorComponent);

    f4DeltaX[0] = (INT8)prsData->u1Delta_X_C0; // B
    f4DeltaX[1] = (INT8)prsData->u1Delta_X_C1; // G
    f4DeltaX[2] = (INT8)prsData->u1Delta_X_C2; // R
    f4DeltaX[3] = (INT8)prsData->u1Delta_X_C3; // A

    f4DeltaY[0] = (INT8)prsData->u1Delta_Y_C0; // B
    f4DeltaY[1] = (INT8)prsData->u1Delta_Y_C1; // G
    f4DeltaY[2] = (INT8)prsData->u1Delta_Y_C2; // R
    f4DeltaY[3] = (INT8)prsData->u1Delta_Y_C3; // A

    f4DeltaX[0] <<= GF_MULTI; // B
    f4DeltaX[1] <<= GF_MULTI; // G
    f4DeltaX[2] <<= GF_MULTI; // R
    f4DeltaX[3] <<= GF_MULTI; // A

    f4DeltaY[0] <<= GF_MULTI; // B
    f4DeltaY[1] <<= GF_MULTI; // G
    f4DeltaY[2] <<= GF_MULTI; // R
    f4DeltaY[3] <<= GF_MULTI; // A

    f4DeltaX[0] = GFX_SwFpS61toFloat(f4DeltaX[0]); // B
    f4DeltaX[1] = GFX_SwFpS61toFloat(f4DeltaX[1]); // G
    f4DeltaX[2] = GFX_SwFpS61toFloat(f4DeltaX[2]); // R
    f4DeltaX[3] = GFX_SwFpS61toFloat(f4DeltaX[3]); // A

    f4DeltaY[0] = GFX_SwFpS61toFloat(f4DeltaY[0]); // B
    f4DeltaY[1] = GFX_SwFpS61toFloat(f4DeltaY[1]); // G
    f4DeltaY[2] = GFX_SwFpS61toFloat(f4DeltaY[2]); // R
    f4DeltaY[3] = GFX_SwFpS61toFloat(f4DeltaY[3]); // A

    //HalFlushDCache();
    //HalInvalidateDCache();
    HalFlushInvalidateDCache();
    // BIM_WAIT_WALE();
    for (j = 0; j < u4Height; j++)
    {
        yinc = (j / u4YInc);
        pu1Write = prsData->pu1DstBase + (j * prsData->u4DstPitch);

        for (i = 0; i < u4Width; i++)
        {
            xinc = (i / u4XInc);

            f4TempColor[0] = i4ColorComponent[0];
            f4TempColor[1] = i4ColorComponent[1];
            f4TempColor[2] = i4ColorComponent[2];
            f4TempColor[3] = i4ColorComponent[3];

            f4TempColor[0] <<= GF_MULTI;
            f4TempColor[1] <<= GF_MULTI;
            f4TempColor[2] <<= GF_MULTI;
            f4TempColor[3] <<= GF_MULTI;

            // calculate vertical direction first
            if (u4GradMode & (UINT32)E_GRAD_VER)
            {
                f4Temp[3] = (f4DeltaY[3] * yinc); // A
                f4Temp[2] = (f4DeltaY[2] * yinc); // R
                f4Temp[1] = (f4DeltaY[1] * yinc); // G
                f4Temp[0] = (f4DeltaY[0] * yinc); // B

                f4TempColor[0] += f4Temp[3];
                f4TempColor[1] += f4Temp[2];
                f4TempColor[2] += f4Temp[1];
                f4TempColor[3] += f4Temp[0];

                f4TempColor[0] = GFX_SwClipping(f4TempColor[0]);
                f4TempColor[1] = GFX_SwClipping(f4TempColor[1]);
                f4TempColor[2] = GFX_SwClipping(f4TempColor[2]);
                f4TempColor[3] = GFX_SwClipping(f4TempColor[3]);
            }

            if (u4GradMode & (UINT32)E_GRAD_HOR)
            {
                f4Temp[3] = (f4DeltaX[3] * xinc); // A
                f4Temp[2] = (f4DeltaX[2] * xinc); // R
                f4Temp[1] = (f4DeltaX[1] * xinc); // G
                f4Temp[0] = (f4DeltaX[0] * xinc); // B

                f4TempColor[0] += f4Temp[3];
                f4TempColor[1] += f4Temp[2];
                f4TempColor[2] += f4Temp[1];
                f4TempColor[3] += f4Temp[0];

                f4TempColor[0] = GFX_SwClipping(f4TempColor[0]);
                f4TempColor[1] = GFX_SwClipping(f4TempColor[1]);
                f4TempColor[2] = GFX_SwClipping(f4TempColor[2]);
                f4TempColor[3] = GFX_SwClipping(f4TempColor[3]);
            }

            f4TempColor[0] >>= GF_MULTI;
            f4TempColor[1] >>= GF_MULTI;
            f4TempColor[2] >>= GF_MULTI;
            f4TempColor[3] >>= GF_MULTI;

            // translate floating-point to integer
            i4TempColor[0] = f4TempColor[0]; // A
            i4TempColor[1] = f4TempColor[1]; // R
            i4TempColor[2] = f4TempColor[2]; // G
            i4TempColor[3] = f4TempColor[3]; // B

            // write value to dst
            GFX_SwSetColorComponent(&pu1Write, u4DstCM, i4TempColor);
        }
    }

    return (INT32)E_GFX_OK;
}
#endif

//static INT32 _ai4Palette[256][4];

/*lint -save -e613 -e740 */
INT32 GFX_SwPgigDecode(UINT8 *pu1SrcBase, UINT8 *pu1DstBase, UINT8 *pu1PalBase,
        UINT32 u4Width, UINT32 u4Height, UINT32 u4DstPitch, UINT32 u4DstCM)
{
#if 0
    UINT32  u4Val, u4SrcIdx, u4Line, i;
    BOOL    fgData, fgLong, fgLineEnd;
    //INT32   *pi4Color;
    UINT8   *pu1Write;


    for (i = 0; i < 256; i++)
    {
        _ai4Palette[i][0] = pu1PalBase[i * 5 + 4];
        if (CM_RGB565_DIRECT16 == u4DstCM)
        {
            _ai4Palette[i][1] = pu1PalBase[i * 5 + 3];
            _ai4Palette[i][2] = pu1PalBase[i * 5 + 1];
            _ai4Palette[i][3] = pu1PalBase[i * 5 + 2];
        }
        else
        {
            _ai4Palette[i][1] = pu1PalBase[i * 5 + 1];
            _ai4Palette[i][2] = pu1PalBase[i * 5 + 2];
            _ai4Palette[i][3] = pu1PalBase[i * 5 + 3];
        }
    }

    u4SrcIdx = 0;
    u4Line = 0;
    while (u4Line < u4Height)
    {
        fgLineEnd = FALSE;
        pu1Write = pu1DstBase + u4Line * u4DstPitch;

        while (!fgLineEnd)
        {
            u4Val = pu1SrcBase[u4SrcIdx++];
            if (u4Val == 0x25)
            {
                i = 0x25;
            }
            if (u4Val != 0)
            {
                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM, _ai4Palette[(BYTE) u4Val]);
            }
            else
            {
                u4Val = pu1SrcBase[u4SrcIdx++];
                if (u4Val == 0x25)
                {
                    i = 0x25;
                }
                fgData = (u4Val & 0x80) ? TRUE : FALSE;
                fgLong = (u4Val & 0x40) ? TRUE : FALSE;
                u4Val &= 0x3F;
                if (!fgData)  // switch_1
                {
                    if (!fgLong)  // switch_2
                    {
                        if (u4Val != 0)
                        {
                            //while (u4Val--)   pu2Dst[u4DstIdx++] = pu2Plt[0x0000];
                            while (u4Val--)
                                GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM, _ai4Palette[0]);
                        }
                        else
                        {
                            fgLineEnd = TRUE;
                    //if (u4DstIdx % 2)
                        //    u4DstIdx++;
                        }
                    }
                    else
                    {
                        //u4Val = (u4Val << 8) | pbSrc[u4SrcIdx++];
                        u4Val = (u4Val << 8) | pu1SrcBase[u4SrcIdx++];
                        //while (u4Val--)   pu2Dst[u4DstIdx++] = pu2Plt[0x0000];
                        while (u4Val--)
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM, _ai4Palette[0]);
                    }
                }
                else  // fgData/switch_1 == 1
                {
                    if (!fgLong)
                    {
                        if (u4Val < 3)
                        {
                            // error
                        }
                        //while (u4Val --)
                        //    pu2Dst[u4DstIdx++] = pu2Plt[pbSrc[u4SrcIdx]];
                        //i = pu1SrcBase[u4SrcIdx];
                        while (u4Val--)
                        {
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM, _ai4Palette[pu1SrcBase[u4SrcIdx]]);
                        }
                        u4SrcIdx++;
                    }
                    else
                    {
                        //u4Val = (u4Val << 8) | pbSrc[u4SrcIdx++];
                        u4Val = (u4Val << 8) | pu1SrcBase[u4SrcIdx++];
                        //while (u4Val --)
                        //    pu2Dst[u4DstIdx++] = pu2Plt[pbSrc[u4SrcIdx]];
                        i = pu1SrcBase[u4SrcIdx];
                        while (u4Val--)
                            GFX_SwSetColorComponent((UINT8 **)&pu1Write, u4DstCM, _ai4Palette[pu1SrcBase[u4SrcIdx]]);
                        u4SrcIdx++;
                    }
                }
            }
        }
        u4Line++;
  }
#endif

    return (INT32)E_GFX_OK;
}
/*lint -restore */


// Non-Premultiplied to Premultiplied bitblt
// Premultiplied to Non-Premultiplied bitblt
typedef struct
{
    unsigned int a;
    unsigned int r;
    unsigned int g;
    unsigned int b;
} pixel;

const int ARGB8bpp      = 10;
const int RGB565        = 11;
const int ARGB1555      = 12;
const int ARGB4444      = 13;
const int ARGB8888      = 14;

pixel expand_color( pixel read_pixel, int color_mode, int global_alpha )
{
    pixel pixel_8888 = {0, 0, 0, 0};

    if ( color_mode == ARGB8888 || color_mode == ARGB8bpp )  //32bpp, 8bpp
    {
        pixel_8888.a = read_pixel.a;
        pixel_8888.r = read_pixel.r;
        pixel_8888.g = read_pixel.g;
        pixel_8888.b = read_pixel.b;
    }
    else  //16bpp
    {
        if ( color_mode == ARGB4444 )
        {
            pixel_8888.a = ( read_pixel.a == 0xf ) ? ( read_pixel.a << 4 ) + read_pixel.a : read_pixel.a << 4;
            pixel_8888.r = read_pixel.r << 4;
            pixel_8888.g = read_pixel.g << 4;
            pixel_8888.b = read_pixel.b << 4;
        }
        else if ( color_mode == ARGB1555 )
        {
            pixel_8888.a = ( read_pixel.a == 1 ) ? 255 : 0;
            pixel_8888.r = ( read_pixel.r << 3 ) + ( read_pixel.r >> 2 );
            pixel_8888.g = ( read_pixel.g << 3 ) + ( read_pixel.g >> 2 );
            pixel_8888.b = ( read_pixel.b << 3 ) + ( read_pixel.b >> 2 );
        }
        else if ( color_mode == RGB565 )
        {
            pixel_8888.a = global_alpha;
            pixel_8888.r = ( read_pixel.r << 3 ) + ( read_pixel.r >> 2 );
            pixel_8888.g = ( read_pixel.g << 2 ) + ( read_pixel.g >> 4 );
            pixel_8888.b = ( read_pixel.b << 3 ) + ( read_pixel.b >> 2 );
        }
    }

    return pixel_8888;
}

pixel shrink_color( pixel pixel_8888, int color_mode, int global_alpha )
{
    pixel write_pixel = {0, 0, 0, 0};

    if ( color_mode == ARGB8888 )  //32bpp
    {
        write_pixel.a = pixel_8888.a;
        write_pixel.r = pixel_8888.r;
        write_pixel.g = pixel_8888.g;
        write_pixel.b = pixel_8888.b;
    }
    else  //16bpp
    {
        if ( color_mode == ARGB4444 )
        {
            write_pixel.a = pixel_8888.a + ( ( pixel_8888.a & ( 1 << 3 ) ) ? ( 1 << 4 ) : 0 );
            write_pixel.r = pixel_8888.r + ( ( pixel_8888.r & ( 1 << 3 ) ) ? ( 1 << 4 ) : 0 );
            write_pixel.g = pixel_8888.g + ( ( pixel_8888.g & ( 1 << 3 ) ) ? ( 1 << 4 ) : 0 );
            write_pixel.b = pixel_8888.b + ( ( pixel_8888.b & ( 1 << 3 ) ) ? ( 1 << 4 ) : 0 );

            write_pixel.a = ( write_pixel.a > 255 ) ? 255 : write_pixel.a;
            write_pixel.r = ( write_pixel.r > 255 ) ? 255 : write_pixel.r;
            write_pixel.g = ( write_pixel.g > 255 ) ? 255 : write_pixel.g;
            write_pixel.b = ( write_pixel.b > 255 ) ? 255 : write_pixel.b;

            write_pixel.a = write_pixel.a >> 4;
            write_pixel.r = write_pixel.r >> 4;
            write_pixel.g = write_pixel.g >> 4;
            write_pixel.b = write_pixel.b >> 4;
        }
        else if ( color_mode == ARGB1555 )
        {
            write_pixel.a = ( pixel_8888.a > 0 ) ? 1 : 0;
            write_pixel.r = pixel_8888.r >> 3;
            write_pixel.g = pixel_8888.g >> 3;
            write_pixel.b = pixel_8888.b >> 3;
        }
        else if ( color_mode == RGB565 )
        {
            write_pixel.a = 0;
            write_pixel.r = pixel_8888.r >> 3;
            write_pixel.g = pixel_8888.g >> 2;
            write_pixel.b = pixel_8888.b >> 3;
        }
    }

    return write_pixel;
}

//#define   DIV_SHT 7
//#define   TABLE_SIZE 512
//#define   PASS3_SHT_NUM 15

//#define   DIV_SHT 6
//#define   TABLE_SIZE 1024
//#define   PASS3_SHT_NUM 14

//#define   DIV_SHT_1 5
//#define   TABLE_SIZE 2048
//#define   PASS3_SHT_NUM_1 11

#define DIV_SHT_1 4
#if (VERIFY_ALCOM == 1)
#define TABLE_SIZE          4096
#else
#define TABLE_SIZE          1
#endif
#define PASS3_SHT_NUM_1 12

//#define DIV_SHT_1 2
//#define TABLE_SIZE 16384
//#define PASS3_SHT_NUM_1 18

//#define TABLE_SIZE_1 1024

void alcom_core_NonPre2Premultiplied( pixel **src, pixel **dst, int width, int height, int dst_color_mode, int blend_mode, int blend_normal, int blend_ar )
{
    /*float Ey, Epb, Epr, EG, EB, ER;*/
    /*FILE *RAMfileY, *RAMfileC, *BMPfile;*/
    /*unsigned int  Ydata[500000], CRdata[250000], CBdata[250000], temp, temp2;*/
    int i, h, w, m;
    unsigned long tmp1, tmp3, tmp4, tmp5, tmp6;
    unsigned long   long tmp2, An_approxi, tmp_cdn_pass2[3];
    unsigned long pixel_s[4], pixel_d[4];
    //    long    height, width;
    unsigned long csn_pass1[3], cdn_pass1[3];
    unsigned long long  cdn_pass2[3];
    unsigned long long  cdn_pass3[3];
    unsigned long long  An;
    unsigned long As, Ad;
    //unsigned long Ar = 65;  //setting range in 0~256
    //unsigned long Ar = 1; //setting range in 0~256
    unsigned long Ar = 256;
    unsigned long Fs = 0;
    unsigned long   Fd = 0;
    int op_code;
    int   pass1_sht_num = 8;
    int   pass2_sht_num = 16;
    unsigned long F_1 = 65536;
    unsigned long A_1 = 256;
    //unsigned long long F_1_SQUARE = (unsigned long long) F_1*F_1;
    unsigned long An_long;
    unsigned long An_long_index;
    //  unsigned long _inverse_table_value[TABLE_SIZE+1];
    //  unsigned long _inverse_table_slope[TABLE_SIZE+1];
    //  unsigned long _inverse_table_shift[TABLE_SIZE+1];
    unsigned long pixel_res[4];
    int   src_p_zero = 0;
    int   dst_p_zero = 0;
    int   color_mode;
    //double  alpha_error_distr[TABLE_SIZE+1];

    //int S_D_SCAN_SHT;
    unsigned long MAX_ALPHA_VALUE;
    int PASS3_SHT_NUM_REAL;
    //int PASS3_ROUND_LEVEL;
    int ALPHA_SHT_LEVEL;
    unsigned long CLAMP_LEVEL;
    //int OVERFLOW_LEVEL;
    unsigned long An_step;

    int an_equalto_zero;

    if ( blend_normal == 0 )  //use Ar
    {
        Ar = blend_ar;
    }
    else  //not use Ar
    {
        Ar = 255;
    }

    if ( Ar == 255 )
    {
        Ar = 256;
    }

    /*if (argc<5) {
    UTIL_Printf("\nUsage : alcom_hw width(height) op_code src_file dst_file gold_file color_mode\n");
    UTIL_Printf("\nColor_mode 0: 32bpp, 1: 16bpp\n");
    exit(0);
    }

    width=atoi(argv[1]);
    height=width;
    UTIL_Printf("Running %d x %d samples\n", width, height);*/

    op_code = blend_mode;
    /*op_code=atoi(argv[2]);
    if (op_code==0) UTIL_Printf("op_code = %d, perform CLEAR\n", op_code);
    if (op_code==1) UTIL_Printf("op_code = %d, perform DST_IN\n", op_code);
    if (op_code==2) UTIL_Printf("op_code = %d, perform DST_OUT\n", op_code);
    if (op_code==3) UTIL_Printf("op_code = %d, perform DST_OVER\n", op_code);
    if (op_code==4) UTIL_Printf("op_code = %d, perform SRC\n", op_code);
    if (op_code==5) UTIL_Printf("op_code = %d, perform SRC_IN\n", op_code);
    if (op_code==6) UTIL_Printf("op_code = %d, perform SRC_OUT\n", op_code);
    if (op_code==7) UTIL_Printf("op_code = %d, perform SRC_OVER\n", op_code);*/

    /*strncpy(file_name, argv[3], 80);
    if ((SRC_FILE=fopen(file_name,"r"))==NULL) {
    UTIL_Printf("Can't open src_file \"%s\"\n",file_name);
    exit(1);
    }

    strncpy(file_name, argv[4], 80);
    if ((DST_FILE=fopen(file_name,"r"))==NULL) {
    UTIL_Printf("Can't open dst_file \"%s\"\n",file_name);
    exit(1);
    }

    strncpy(file_name, argv[5], 80);
    if ((GOLD_FILE=fopen(file_name,"r"))==NULL) {
    UTIL_Printf("Can't open dst_file \"%s\"\n",file_name);
    exit(1);
    }*/

    color_mode = 0;
    /*color_mode=atoi(argv[6]);
    strncpy(file_name, "RESULT_FILE", 40);
    if ((RESULT_FILE=fopen(file_name,"wb"))==NULL) {
    UTIL_Printf("Can't open file \"%s\"\n",file_name);
    exit(1);
    }

    strncpy(file_name, "TABLE_FILE_F_3", 40);
    if ((TABLE_FILE=fopen(file_name,"wb"))==NULL) {
    UTIL_Printf("Can't open file \"%s\"\n",file_name);
    exit(1);
    }

    strncpy(file_name, "TABLE_FILE_FOR_RTL", 40);
    if ((TABLE_FILE_FOR_RTL=fopen(file_name,"wb"))==NULL) {
    UTIL_Printf("Can't open file \"%s\"\n",file_name);
    exit(1);
    }*/


    //linear approximation
    _inverse_table_value[0] = 0;
    _inverse_table_slope[0] = 0;
    _inverse_table_shift[0] = PASS3_SHT_NUM_1;
    //alpha_error_distr[0] = 0.0;
    //fPrintf(TABLE_FILE_FOR_RTL, "%6d %6d %6d %6d\n", 0, _inverse_table_value[0], _inverse_table_shift[0], _inverse_table_slope[0]);
    for (m=1; m<TABLE_SIZE+1; m++){
        //alpha_error_distr[m] = 0.0;
        i = 0;
        tmp1 = F_1/m;
        tmp4 = F_1;
        while (tmp1<=F_1/2) {
            tmp4 = tmp4<<1;
            tmp1 = tmp4/m;
            i++;
        }
        /**************************************/
        /*tmp4 = (F_1<<(i+1))/m;
        _inverse_table_value[m] = round_u_long(tmp4, 1);
        _inverse_table_shift[m] = i+PASS3_SHT_NUM_1;
        tmp5 = (F_1<<(i+1))/m;
        tmp6 = (F_1<<(i+1))/(m+1);
        tmp4 = (tmp5-tmp6)>>4;
        _inverse_table_slope[m] =  round_u_long(tmp4, 1);*/
        /**************************************/
        tmp4 = (F_1<<(i+2))/m;  //one more bit
        _inverse_table_value[m] = round_u_long(tmp4, 1);
        _inverse_table_shift[m] = i+PASS3_SHT_NUM_1;
        tmp5 = (F_1<<(i+2))/m;
        tmp6 = (F_1<<(i+2))/(m+1);
        tmp4 = (tmp5-tmp6)>>4;
        /*tmp1 =  round_u_long(tmp4, 1);
        if ((tmp5-tmp6) < (tmp1<<5))
        {_inverse_table_slope[m] =  tmp1-1;
        }
        else
        _inverse_table_slope[m] =  tmp1 ; */
        _inverse_table_slope[m] =  round_u_long(tmp4, 1);
        /**************************************/
        //UTIL_Printf("index = %d, _inverse_table_value[]= %d, _inverse_table_shift[]= %d\n", m, _inverse_table_value[m], _inverse_table_shift[m]);
        //getchar();
        //fPrintf(TABLE_FILE, "index = %d, _inverse_table_value[]= %d, _inverse_table_shift[]= %d, _inverse_table_slope[]= %d\n", m, _inverse_table_value[m], _inverse_table_shift[m], _inverse_table_slope[m]);
        //if (m!=TABLE_SIZE)
        //  fPrintf(TABLE_FILE_FOR_RTL, "%6d %6d %6d %6d\n", m, _inverse_table_value[m], _inverse_table_shift[m], _inverse_table_slope[m]);
    }
    //fclose(TABLE_FILE);
    //fclose(TABLE_FILE_FOR_RTL);
    //getchar();

    if (color_mode==0) {  //32bpp
        //S_D_SCAN_SHT = 0;
        MAX_ALPHA_VALUE = 255;
        PASS3_SHT_NUM_REAL = PASS3_SHT_NUM_1+12;
        //PASS3_ROUND_LEVEL = 0;
        ALPHA_SHT_LEVEL = ALPHA_SHT;
        CLAMP_LEVEL = 255;
        //OVERFLOW_LEVEL = 256;
    }
    if (color_mode==1) {  //16bpp
        //S_D_SCAN_SHT = 4;
        MAX_ALPHA_VALUE = 240;
        PASS3_SHT_NUM_REAL = PASS3_SHT_NUM_1+16;
        //PASS3_ROUND_LEVEL = 4;
        ALPHA_SHT_LEVEL = ALPHA_SHT_16BPP;
        CLAMP_LEVEL = 15;
        //OVERFLOW_LEVEL = 16;
    }


    for (h=0; h<height; h++) {
        for (w=0; w<width; w++){
            dst_p_zero = 0;

            //read src and dst pixels
            pixel_s[3] = src[w][h].a;
            pixel_s[2] = src[w][h].r;
            pixel_s[1] = src[w][h].g;
            pixel_s[0] = src[w][h].b;

            pixel_d[3] = dst[w][h].a;
            pixel_d[2] = dst[w][h].r;
            pixel_d[1] = dst[w][h].g;
            pixel_d[0] = dst[w][h].b;
            /*for (i=0; i<4; i++){
            fscanf(SRC_FILE, "%x", &tmp1);
            pixel_s[i] = tmp1 << S_D_SCAN_SHT;
            //UTIL_Printf("pixel_s %d\n", pixel_s[i]);
            fscanf(DST_FILE, "%x", &tmp1);
            pixel_d[i] = tmp1 << S_D_SCAN_SHT;
            //UTIL_Printf("pixel_d %d\n", pixel_d[i]);
            }*/


            /*for (i=3; i>=0; i--){
            if (pixel_s[i]==255) pixel_s[i]=256;
            if (pixel_d[i]==255) pixel_d[i]=256;
            }*/
            if (pixel_s[3]==MAX_ALPHA_VALUE) pixel_s[3]=A_1;  //only alpha pushed to 1.0
            if (pixel_d[3]==MAX_ALPHA_VALUE) pixel_d[3]=A_1;  //only alpha pushed to 1.0

            ///Common operation
            As=pixel_s[3];
            Ad=pixel_d[3];

            ///pass 0, As*Ar
            As=As*Ar; Ad=Ad*A_1; //9x9, 17-bit data, no loss
            if (As==0) src_p_zero = 1; else src_p_zero = 0; //src zero flag
            //if (Ad==0) dst_p_zero = 1; else dst_p_zero = 0; //dst zero flag

            //UTIL_Printf("As %d, Ad %d\n", As, Ad);
            ///pass 1, color * Fs, Fd
            ////////////////////////////////
            ///CLEAR
            if (op_code==0){
                Fs = 0;
                Fd = 0;
            } //end of op_code


            ////////////////////////////////
            ///DST_IN
            if (op_code==1){
                Fs = 0;
                Fd = As;
            } //end of op_code

            ////////////////////////////////
            ///DST_OUT
            if (op_code==2){
                Fs = 0;
                Fd = F_1 - As;
            } //end of op_code

            ////////////////////////////////
            ///DST_OVER
            if (op_code==3){
                Fs = F_1 - Ad;
                Fd = F_1;
            } //end of op_code

            ////////////////////////////////
            ///SRC
            if (op_code==4){
                Fs = F_1;
                Fd = 0;
            } //end of op_code

            ////////////////////////////////
            ///SRC_IN
            if (op_code==5){
                Fs = Ad;
                Fd = 0;
            } //end of op_code

            ////////////////////////////////
            ///SRC_OUT
            if (op_code==6){
                Fs = F_1 - Ad;
                Fd = 0;
            } //end of op_code

            ////////////////////////////////
            ///SRC_OVER
            if (op_code==7){
                Fs = F_1;
                Fd = F_1 - As;
            } //end of op_code

            if (Fs==0) src_p_zero = 0x1|src_p_zero; //src zero flag
            //if (Fd==0) dst_p_zero = 0x1|dst_p_zero; //dst zero flag

            for (i=0; i<3; i++){
                if (dst_p_zero)
                    tmp1 = F_1*pixel_s[i];  //17x9, 25-bit data
                else
                    tmp1 = Fs*pixel_s[i]; //17x9, 25-bit data
                //UTIL_Printf("src tmp1 %d\n", tmp1);
                csn_pass1[i] = round_u_long(tmp1, pass1_sht_num); //25->17bit
                //if (csn_pass1[i]>F_1) UTIL_Printf("csn_pass1 data overflow !!\n\n");
                if (src_p_zero)
                    tmp1 = F_1*pixel_d[i];  //17x9, 25-bit data
                else
                    tmp1 = Fd*pixel_d[i]; //17x9, 25-bit data
                cdn_pass1[i] = round_u_long(tmp1, pass1_sht_num); //25->17bit
                //if (csn_pass1[i]>F_1) UTIL_Printf("cdn_pass1 data overflow !!\n\n");
                //cdn_pass1[i] = tmp1;  //17bit
                //UTIL_Printf("dst tmp1 [%d]= %d\n", i, tmp1);
            }

            ///pass 2, color tmp * alpha tmp
            An = (unsigned long long) Fs*As + (unsigned long long) Fd*Ad; //17x17, 33+1 bit
            //UTIL_Printf("after pass 2 An=%llu before shift\n", An);
            //An = round_u_long_long(An, pass2_sht_num);  //26 bit->17bit
            An = An>>pass2_sht_num; //34(33) bit->17bit
            //UTIL_Printf("after pass 2 An=%llu after shift\n", An);
            //if (Fs==0) src_p_zero = 1|src_p_zero;
            //if (Fd==0) dst_p_zero = 1|dst_p_zero;
            for (i=0; i<3; i++){
                if (dst_p_zero)
                    tmp1 = F_1;
                else
                    tmp1 = As;
                if (src_p_zero)
                    tmp3 = F_1;
                else
                    tmp3 = Ad;
                tmp_cdn_pass2[i] = (unsigned long long) tmp1*csn_pass1[i] + (unsigned long long) tmp3*cdn_pass1[i]; //17x17, 33+1 bit
                //UTIL_Printf("after pass 2 tmp2=%llu\n", tmp2);
                cdn_pass2[i] = round_u_long_long(tmp_cdn_pass2[i], pass2_sht_num); //34->17bit
                //if (cdn_pass2[i] > (F_1 & (!(src_p_zero & dst_p_zero))))
                //{UTIL_Printf("\n pass 2 result overflow!!\n", cdn_pass2[i]);
                //UTIL_Printf("src_p_zero=%d, dst_p_zero=%d\n", src_p_zero, dst_p_zero);
                //UTIL_Printf("tmp1=%d, csn_pass1=%d, tmp3=%d, cdn_pass1=%d, cdn_pass2=%llu!!\n\n", tmp1, csn_pass1[i], tmp3, cdn_pass1[i], cdn_pass2[i]);}
            }


            ///pass 3
            //int an_equalto_zero;

            An_long = An; //17-bit
            An_step = An_long & 0xf;  //last 4-bit
            An_long_index = An_long >> DIV_SHT_1;
            //An_long_index = round_u_long(An_long , DIV_SHT_1);
            if (An_long_index>=TABLE_SIZE) An_long_index=TABLE_SIZE;
            for (i=0; i<3; i++){
                //tmp2 = _inverse_table_value[An_long_index]*cdn_pass2[i]; //17x17, 33 bit
                if ( dst_color_mode == ARGB4444 )
                {
                    an_equalto_zero = ( ( An_long_index >> 7 ) == 0 ) ? 1 : 0;
                }
                else
                {
                    an_equalto_zero = ( ( An_long_index >> 3 ) == 0 ) ? 1 : 0;
                }

                if ( ( src_p_zero & dst_p_zero ) | an_equalto_zero )
                {
                    tmp2 = 0; //17x17, 33 bit
                }
                else if (src_p_zero||dst_p_zero||1)
                {
                    tmp2 = F_1*cdn_pass2[i]; //17x17, 33 bit
                }
                else {
                    //tmp2 = _inverse_table_value[An_long_index]*cdn_pass2[i]; //17x17, 33 bit
                    //An_approxi = (_inverse_table_value[An_long_index]-_inverse_table_slope[An_long_index]*An_step);
                    An_approxi = (_inverse_table_value[An_long_index]-_inverse_table_slope[An_long_index]*An_step);
                    //An_approxi = round_u_long_long(An_approxi, 1);
                    An_approxi = An_approxi >> 1;
                    tmp2 = An_approxi*cdn_pass2[i]; //17x17, 33 bit
                    //if (tmp2 >= F_1_SQUARE)
                    //{UTIL_Printf("\npass 3 result overflow !!\n");
                    //UTIL_Printf("An_long_index = %d, F_1_SQUARE=%llu\n", An_long_index, F_1_SQUARE);
                    //UTIL_Printf("An_approxi = %llu, shift=%d, cdn_pass2=%llu, tmp2 = %llu\n", An_approxi, _inverse_table_shift[An_long_index], cdn_pass2[i], tmp2);
                    //}
                }
                if (src_p_zero||dst_p_zero||1){
                    //cdn_pass3[i] = tmp2 >> PASS3_SHT_NUM_REAL;
                    if ( dst_color_mode == ARGB8888 )
                    {
                        cdn_pass3[i] = round_u_long_long(tmp2, PASS3_SHT_NUM_REAL);
                    }
                    else
                    {
                        cdn_pass3[i] = tmp2 >> PASS3_SHT_NUM_REAL;
                    }
                }
                else {
                    if ( dst_color_mode == ARGB8888 )
                    {
                        cdn_pass3[i] = round_u_long_long(tmp2, _inverse_table_shift[An_long_index]); //33->9bit
                    }
                    else
                    {
                        cdn_pass3[i] = tmp2 >> _inverse_table_shift[An_long_index];
                    }
                }
            }

            if ( dst_color_mode == ARGB8888 )
            {
                An_long = round_u_long(An_long, ALPHA_SHT_LEVEL);
            }
            else
            {
                An_long = An_long >> ALPHA_SHT_LEVEL;
            }
            //if (An_long>OVERFLOW_LEVEL) UTIL_Printf("final alpha data overflow !!\n\n");
            if (An_long>=CLAMP_LEVEL) An_long=CLAMP_LEVEL;

            //if (cdn_pass3[2]>OVERFLOW_LEVEL) UTIL_Printf("final pass3 data overflow !!\n\n");
            //if (cdn_pass3[1]>OVERFLOW_LEVEL) UTIL_Printf("final pass3 data overflow !!\n\n");
            //if (cdn_pass3[0]>OVERFLOW_LEVEL) UTIL_Printf("final pass3 data overflow !!\n\n");

            pixel_res[3] = An_long;
            pixel_res[2] = cdn_pass3[2]; if (pixel_res[2]>=CLAMP_LEVEL) pixel_res[2]=CLAMP_LEVEL;
            pixel_res[1] = cdn_pass3[1]; if (pixel_res[1]>=CLAMP_LEVEL) pixel_res[1]=CLAMP_LEVEL;
            pixel_res[0] = cdn_pass3[0]; if (pixel_res[0]>=CLAMP_LEVEL) pixel_res[0]=CLAMP_LEVEL;
            //if (pixel_res[3]==0) pixel_res[2]=0;
            //if (pixel_res[3]==0) pixel_res[1]=0;
            //if (pixel_res[3]==0) pixel_res[0]=0;

            dst[w][h].a = pixel_res[3];
            dst[w][h].r = pixel_res[2];
            dst[w][h].g = pixel_res[1];
            dst[w][h].b = pixel_res[0];
        } //end of for n
    } //end of for m
}

void alcom_core_Pre2NonPremultiplied( pixel **src, pixel **dst, int width, int height, int dst_color_mode, int blend_mode, int blend_normal, int blend_ar )
{
    /*float Ey, Epb, Epr, EG, EB, ER;*/
    /*FILE *RAMfileY, *RAMfileC, *BMPfile;*/
    /*unsigned int  Ydata[500000], CRdata[250000], CBdata[250000], temp, temp2;*/
    int     i, h, w, m;
    unsigned long tmp1, tmp3, tmp4, tmp5, tmp6;
    unsigned long   long tmp2, An_approxi, tmp_cdn_pass2[3];
    unsigned long pixel_s[4], pixel_d[4];
    unsigned long csn_pass1[3], cdn_pass1[3];
    unsigned long long  cdn_pass2[3];
    unsigned long long  cdn_pass3[3];
    unsigned long long  An;
    unsigned long As, Ad;
    unsigned long Ar = 256;
    unsigned long Fs = 0;
    unsigned long   Fd = 0;
    int   op_code;
    int   pass1_sht_num = 8;
    int   pass2_sht_num = 16;
    unsigned long F_1 = 65536;
    unsigned long A_1 = 256;
//    unsigned long long F_1_SQUARE = (unsigned long long) F_1*F_1;
    unsigned long An_long;
    unsigned long An_long_index;
    //  unsigned long _inverse_table_value[TABLE_SIZE+1];
    //  unsigned long _inverse_table_slope[TABLE_SIZE+1];
    //  unsigned long _inverse_table_shift[TABLE_SIZE+1];
    unsigned long pixel_res[4];
    int   src_p_zero = 0;
    int   dst_p_zero = 0;
    int   color_mode;
    unsigned long MAX_ALPHA_VALUE;
    int   PASS3_SHT_NUM_REAL;
    int   ALPHA_SHT_LEVEL;
    unsigned long CLAMP_LEVEL;
    //int     OVERFLOW_LEVEL;
    unsigned long An_step;
    //double  alpha_error_distr[TABLE_SIZE+1];
    //int   S_D_SCAN_SHT;
    //int   PASS3_ROUND_LEVEL;
    int an_equalto_zero;

    if ( blend_normal == 0 )  //use Ar
    {
        Ar = blend_ar;
    }
    else  //not use Ar
    {
        Ar = 255;
    }

    if ( Ar == 255 )
    {
        Ar = 256;
    }

    /*if (argc<5) {
    UTIL_Printf("\nUsage : alcom_hw width(height) op_code src_file dst_file gold_file color_mode\n");
    UTIL_Printf("\nColor_mode 0: 32bpp, 1: 16bpp\n");
    exit(0);
    }

    width=atoi(argv[1]);
    height=width;
    UTIL_Printf("Running %d x %d samples\n", width, height);*/

    op_code = blend_mode;
    /*op_code=atoi(argv[2]);
    if (op_code==0) UTIL_Printf("op_code = %d, perform CLEAR\n", op_code);
    if (op_code==1) UTIL_Printf("op_code = %d, perform DST_IN\n", op_code);
    if (op_code==2) UTIL_Printf("op_code = %d, perform DST_OUT\n", op_code);
    if (op_code==3) UTIL_Printf("op_code = %d, perform DST_OVER\n", op_code);
    if (op_code==4) UTIL_Printf("op_code = %d, perform SRC\n", op_code);
    if (op_code==5) UTIL_Printf("op_code = %d, perform SRC_IN\n", op_code);
    if (op_code==6) UTIL_Printf("op_code = %d, perform SRC_OUT\n", op_code);
    if (op_code==7) UTIL_Printf("op_code = %d, perform SRC_OVER\n", op_code);*/

    /*strncpy(file_name, argv[3], 80);
    if ((SRC_FILE=fopen(file_name,"r"))==NULL) {
    UTIL_Printf("Can't open src_file \"%s\"\n",file_name);
    exit(1);
    }

    strncpy(file_name, argv[4], 80);
    if ((DST_FILE=fopen(file_name,"r"))==NULL) {
    UTIL_Printf("Can't open dst_file \"%s\"\n",file_name);
    exit(1);
    }

    strncpy(file_name, argv[5], 80);
    if ((GOLD_FILE=fopen(file_name,"r"))==NULL) {
    UTIL_Printf("Can't open dst_file \"%s\"\n",file_name);
    exit(1);
    }*/

    color_mode = 0;
    /*color_mode=atoi(argv[6]);
    strncpy(file_name, "RESULT_FILE", 40);
    if ((RESULT_FILE=fopen(file_name,"wb"))==NULL) {
    UTIL_Printf("Can't open file \"%s\"\n",file_name);
    exit(1);
    }

    strncpy(file_name, "TABLE_FILE_F_3", 40);
    if ((TABLE_FILE=fopen(file_name,"wb"))==NULL) {
    UTIL_Printf("Can't open file \"%s\"\n",file_name);
    exit(1);
    }

    strncpy(file_name, "TABLE_FILE_FOR_RTL", 40);
    if ((TABLE_FILE_FOR_RTL=fopen(file_name,"wb"))==NULL) {
    UTIL_Printf("Can't open file \"%s\"\n",file_name);
    exit(1);
    }*/


    //linear approximation
    _inverse_table_value[0] = 0;
    _inverse_table_slope[0] = 0;
    _inverse_table_shift[0] = PASS3_SHT_NUM_1;
    //alpha_error_distr[0] = 0.0;
    //fPrintf(TABLE_FILE_FOR_RTL, "%6d %6d %6d %6d\n", 0, _inverse_table_value[0], _inverse_table_shift[0], _inverse_table_slope[0]);
    for (m=1; m<TABLE_SIZE+1; m++){
        //alpha_error_distr[m] = 0.0;
        i = 0;
        tmp1 = F_1/m;
        tmp4 = F_1;
        while (tmp1<=F_1/2) {
            tmp4 = tmp4<<1;
            tmp1 = tmp4/m;
            i++;
        }
        /**************************************/
        /*tmp4 = (F_1<<(i+1))/m;
        _inverse_table_value[m] = round_u_long(tmp4, 1);
        _inverse_table_shift[m] = i+PASS3_SHT_NUM_1;
        tmp5 = (F_1<<(i+1))/m;
        tmp6 = (F_1<<(i+1))/(m+1);
        tmp4 = (tmp5-tmp6)>>4;
        _inverse_table_slope[m] =  round_u_long(tmp4, 1);*/
        /**************************************/
        tmp4 = (F_1<<(i+2))/m;  //one more bit
        _inverse_table_value[m] = round_u_long(tmp4, 1);
        _inverse_table_shift[m] = i+PASS3_SHT_NUM_1;
        tmp5 = (F_1<<(i+2))/m;
        tmp6 = (F_1<<(i+2))/(m+1);
        tmp4 = (tmp5-tmp6)>>4;
        /*tmp1 =  round_u_long(tmp4, 1);
        if ((tmp5-tmp6) < (tmp1<<5))
        {_inverse_table_slope[m] =  tmp1-1;
        }
        else
        _inverse_table_slope[m] =  tmp1 ; */
        _inverse_table_slope[m] =  round_u_long(tmp4, 1);
        /**************************************/
        //UTIL_Printf("index = %d, _inverse_table_value[]= %d, _inverse_table_shift[]= %d\n", m, _inverse_table_value[m], _inverse_table_shift[m]);
        //getchar();
        //fPrintf(TABLE_FILE, "index = %d, _inverse_table_value[]= %d, _inverse_table_shift[]= %d, _inverse_table_slope[]= %d\n", m, _inverse_table_value[m], _inverse_table_shift[m], _inverse_table_slope[m]);
        //if (m!=TABLE_SIZE)
        //  fPrintf(TABLE_FILE_FOR_RTL, "%6d %6d %6d %6d\n", m, _inverse_table_value[m], _inverse_table_shift[m], _inverse_table_slope[m]);
    }
    //fclose(TABLE_FILE);
    //fclose(TABLE_FILE_FOR_RTL);
    //getchar();

    if (color_mode==0) {  //32bpp
        //S_D_SCAN_SHT = 0;
        MAX_ALPHA_VALUE = 255;
        PASS3_SHT_NUM_REAL = PASS3_SHT_NUM_1+12;
        //PASS3_ROUND_LEVEL = 0;
        ALPHA_SHT_LEVEL = ALPHA_SHT;
        CLAMP_LEVEL = 255;
        //OVERFLOW_LEVEL = 256;
    }
    if (color_mode==1) {  //16bpp
        //S_D_SCAN_SHT = 4;
        MAX_ALPHA_VALUE = 240;
        PASS3_SHT_NUM_REAL = PASS3_SHT_NUM_1+16;
        //PASS3_ROUND_LEVEL = 4;
        ALPHA_SHT_LEVEL = ALPHA_SHT_16BPP;
        CLAMP_LEVEL = 15;
        //OVERFLOW_LEVEL = 16;
    }


    for (h=0; h<height; h++) {
        for (w=0; w<width; w++){

            //read src and dst pixels
            pixel_s[3] = src[w][h].a;
            pixel_s[2] = src[w][h].r;
            pixel_s[1] = src[w][h].g;
            pixel_s[0] = src[w][h].b;

            pixel_d[3] = dst[w][h].a;
            pixel_d[2] = dst[w][h].r;
            pixel_d[1] = dst[w][h].g;
            pixel_d[0] = dst[w][h].b;
            /*for (i=0; i<4; i++){
            fscanf(SRC_FILE, "%x", &tmp1);
            pixel_s[i] = tmp1 << S_D_SCAN_SHT;
            //UTIL_Printf("pixel_s %d\n", pixel_s[i]);
            fscanf(DST_FILE, "%x", &tmp1);
            pixel_d[i] = tmp1 << S_D_SCAN_SHT;
            //UTIL_Printf("pixel_d %d\n", pixel_d[i]);
            }*/


            /*for (i=3; i>=0; i--){
            if (pixel_s[i]==255) pixel_s[i]=256;
            if (pixel_d[i]==255) pixel_d[i]=256;
            }*/
            if (pixel_s[3]==MAX_ALPHA_VALUE) pixel_s[3]=A_1;  //only alpha pushed to 1.0
            if (pixel_d[3]==MAX_ALPHA_VALUE) pixel_d[3]=A_1;  //only alpha pushed to 1.0

            ///Common operation
            As=pixel_s[3];
            Ad=pixel_d[3];

            ///pass 0, As*Ar
            As=As*Ar; Ad=Ad*A_1; //9x9, 17-bit data, no loss
            if (As==0) src_p_zero = 1; else src_p_zero = 0; //src zero flag
            //if (Ad==0) dst_p_zero = 1; else dst_p_zero = 0; //dst zero flag

            //UTIL_Printf("As %d, Ad %d\n", As, Ad);
            ///pass 1, color * Fs, Fd
            ////////////////////////////////
            ///CLEAR
            if (op_code==0){
                Fs = 0;
                Fd = 0;
            } //end of op_code


            ////////////////////////////////
            ///DST_IN
            if (op_code==1){
                Fs = 0;
                Fd = As;
            } //end of op_code

            ////////////////////////////////
            ///DST_OUT
            if (op_code==2){
                Fs = 0;
                Fd = F_1 - As;
            } //end of op_code

            ////////////////////////////////
            ///DST_OVER
            if (op_code==3){
                Fs = F_1 - Ad;
                Fd = F_1;
            } //end of op_code

            ////////////////////////////////
            ///SRC
            if (op_code==4){
                Fs = F_1;
                Fd = 0;
            } //end of op_code

            ////////////////////////////////
            ///SRC_IN
            if (op_code==5){
                Fs = Ad;
                Fd = 0;
            } //end of op_code

            ////////////////////////////////
            ///SRC_OUT
            if (op_code==6){
                Fs = F_1 - Ad;
                Fd = 0;
            } //end of op_code

            ////////////////////////////////
            ///SRC_OVER
            if (op_code==7){
                Fs = F_1;
                Fd = F_1 - As;
            } //end of op_code

            if (Fs==0) src_p_zero = 0x1|src_p_zero; //src zero flag
            //if (Fd==0) dst_p_zero = 0x1|dst_p_zero; //dst zero flag

            for (i=0; i<3; i++){
                if (dst_p_zero)
                    tmp1 = F_1*pixel_s[i];  //17x9, 25-bit data
                else
                    tmp1 = Fs*pixel_s[i]; //17x9, 25-bit data
                //UTIL_Printf("src tmp1 %d\n", tmp1);
                csn_pass1[i] = round_u_long(tmp1, pass1_sht_num); //25->17bit
                //if (csn_pass1[i]>F_1) UTIL_Printf("csn_pass1 data overflow !!\n\n");
                if (src_p_zero)
                    tmp1 = F_1*pixel_d[i];  //17x9, 25-bit data
                else
                    tmp1 = Fd*pixel_d[i]; //17x9, 25-bit data
                cdn_pass1[i] = round_u_long(tmp1, pass1_sht_num); //25->17bit
                //if (csn_pass1[i]>F_1) UTIL_Printf("cdn_pass1 data overflow !!\n\n");
                //cdn_pass1[i] = tmp1;  //17bit
                //UTIL_Printf("dst tmp1 [%d]= %d\n", i, tmp1);
            }

            ///pass 2, color tmp * alpha tmp
            An = (unsigned long long) Fs*As + (unsigned long long) Fd*Ad; //17x17, 33+1 bit
            //UTIL_Printf("after pass 2 An=%llu before shift\n", An);
            //An = round_u_long_long(An, pass2_sht_num);  //26 bit->17bit
            An = An>>pass2_sht_num; //34(33) bit->17bit
            //UTIL_Printf("after pass 2 An=%llu after shift\n", An);
            //if (Fs==0) src_p_zero = 1|src_p_zero;
            //if (Fd==0) dst_p_zero = 1|dst_p_zero;
            for (i=0; i<3; i++){
                if (dst_p_zero)
                    tmp1 = F_1;
                else
                    tmp1 = As;
                if (src_p_zero)
                    tmp3 = F_1;
                else
                    tmp3 = Ad;
                tmp_cdn_pass2[i] = (unsigned long long) Ar*A_1*csn_pass1[i] + (unsigned long long) tmp3*cdn_pass1[i]; //17x17, 33+1 bit
                //UTIL_Printf("after pass 2 tmp2=%llu\n", tmp2);
                cdn_pass2[i] = round_u_long_long(tmp_cdn_pass2[i], pass2_sht_num); //34->17bit
                //if (cdn_pass2[i] > (F_1 & (!(src_p_zero & dst_p_zero))))
                //{UTIL_Printf("\n pass 2 result overflow!!\n", cdn_pass2[i]);
                //UTIL_Printf("src_p_zero=%d, dst_p_zero=%d\n", src_p_zero, dst_p_zero);
                //UTIL_Printf("tmp1=%d, csn_pass1=%d, tmp3=%d, cdn_pass1=%d, cdn_pass2=%llu!!\n\n", tmp1, csn_pass1[i], tmp3, cdn_pass1[i], cdn_pass2[i]);}
            }


            ///pass 3
            //int an_equalto_zero;

            An_long = An; //17-bit
            An_step = An_long & 0xf;  //last 4-bit
            An_long_index = An_long >> DIV_SHT_1;
            //An_long_index = round_u_long(An_long , DIV_SHT_1);
            if (An_long_index>=TABLE_SIZE) An_long_index=TABLE_SIZE;
            for (i=0; i<3; i++){
                //tmp2 = _inverse_table_value[An_long_index]*cdn_pass2[i]; //17x17, 33 bit
                if ( dst_color_mode == ARGB4444 )
                {
                    an_equalto_zero = ( ( An_long_index >> 7 ) == 0 ) ? 1 : 0;
                }
                else
                {
                    an_equalto_zero = ( ( An_long_index >> 3 ) == 0 ) ? 1 : 0;
                }

                if ( ( src_p_zero & dst_p_zero ) | an_equalto_zero )
                {
                    tmp2 = 0; //17x17, 33 bit
                }
                else if (src_p_zero||dst_p_zero)
                {
                    tmp2 = F_1*cdn_pass2[i]; //17x17, 33 bit
                }
                else {
                    //tmp2 = _inverse_table_value[An_long_index]*cdn_pass2[i]; //17x17, 33 bit
                    //An_approxi = (_inverse_table_value[An_long_index]-_inverse_table_slope[An_long_index]*An_step);
                    An_approxi = (_inverse_table_value[An_long_index]-_inverse_table_slope[An_long_index]*An_step);
                    //An_approxi = round_u_long_long(An_approxi, 1);
                    An_approxi = An_approxi >> 1;
                    tmp2 = An_approxi*cdn_pass2[i]; //17x17, 33 bit
                    //if (tmp2 >= F_1_SQUARE)
                    //{UTIL_Printf("\npass 3 result overflow !!\n");
                    //UTIL_Printf("An_long_index = %d, F_1_SQUARE=%llu\n", An_long_index, F_1_SQUARE);
                    //UTIL_Printf("An_approxi = %llu, shift=%d, cdn_pass2=%llu, tmp2 = %llu\n", An_approxi, _inverse_table_shift[An_long_index], cdn_pass2[i], tmp2);
                    //}
                }
                if (src_p_zero||dst_p_zero){
                    //cdn_pass3[i] = tmp2 >> PASS3_SHT_NUM_REAL;
                    if ( dst_color_mode == ARGB8888 )
                    {
                        cdn_pass3[i] = round_u_long_long(tmp2, PASS3_SHT_NUM_REAL);
                    }
                    else
                    {
                        cdn_pass3[i] = tmp2 >> PASS3_SHT_NUM_REAL;
                    }
                }
                else {
                    if ( dst_color_mode == ARGB8888 )
                    {
                        cdn_pass3[i] = round_u_long_long(tmp2, _inverse_table_shift[An_long_index]); //33->9bit
                    }
                    else
                    {
                        cdn_pass3[i] = tmp2 >> _inverse_table_shift[An_long_index];
                    }
                }
            }

            if ( dst_color_mode == ARGB8888 )
            {
                An_long = round_u_long(An_long, ALPHA_SHT_LEVEL);
            }
            else
            {
                An_long = An_long >> ALPHA_SHT_LEVEL;
            }
            //if (An_long>OVERFLOW_LEVEL) UTIL_Printf("final alpha data overflow !!\n\n");
            if (An_long>=CLAMP_LEVEL) An_long=CLAMP_LEVEL;

            //if (cdn_pass3[2]>OVERFLOW_LEVEL) UTIL_Printf("final pass3 data overflow !!\n\n");
            //if (cdn_pass3[1]>OVERFLOW_LEVEL) UTIL_Printf("final pass3 data overflow !!\n\n");
            //if (cdn_pass3[0]>OVERFLOW_LEVEL) UTIL_Printf("final pass3 data overflow !!\n\n");

            pixel_res[3] = An_long;
            pixel_res[2] = cdn_pass3[2]; if (pixel_res[2]>=CLAMP_LEVEL) pixel_res[2]=CLAMP_LEVEL;
            pixel_res[1] = cdn_pass3[1]; if (pixel_res[1]>=CLAMP_LEVEL) pixel_res[1]=CLAMP_LEVEL;
            pixel_res[0] = cdn_pass3[0]; if (pixel_res[0]>=CLAMP_LEVEL) pixel_res[0]=CLAMP_LEVEL;
            //if (pixel_res[3]==0) pixel_res[2]=0;
            //if (pixel_res[3]==0) pixel_res[1]=0;
            //if (pixel_res[3]==0) pixel_res[0]=0;

            dst[w][h].a = pixel_res[3];
            dst[w][h].r = pixel_res[2];
            dst[w][h].g = pixel_res[1];
            dst[w][h].b = pixel_res[0];
        } //end of for n
    } //end of for m
}


INT32 GFX_SwPremultipliedCvnBitblt(const GFX_ACLM_DATA_T *prsData)
{
    int src_color_mode;
    int dst_color_mode;
    int width;
    int height;
    int global_alpha;
    //int src_global_alpha;
    //int dst_global_alpha;
    int blend_mode;
    int blend_normal;
    int blend_ar;

    UINT32 u4SrcPitch, u4DstPitch;
    UINT8 *pu1SrcLine, *pu1DstLine;
    UINT8 *pu1Read, *pu1Write, *pu1Src, *pu1Dst;
    UINT32 u4NonPre2PremultipliedEn, u4Pre2NonPremultipliedEn;
    int x, y;
    int byte_index;
    pixel **src;
    pixel **dst;
    UINT8 *pu1SrcBase = NULL,*pu1DstBase = NULL;

    pu1SrcBase  = prsData->pu1SrcBase;
    u4SrcPitch  = prsData->u4SrcPitch;
    pu1DstBase  = prsData->pu1DstBase;
    u4DstPitch  = prsData->u4DstPitch;
    dst_color_mode = prsData->u4DstCM;
    width       = prsData->u4Width;
    height      = prsData->u4Height;
    blend_ar    = prsData->u4AlComAr;
    blend_mode  = prsData->u4AlComOpCode;
    global_alpha = prsData->u4GlobalAlpha;
    blend_normal = prsData->u4AlcomNormal;
    u4NonPre2PremultipliedEn = prsData->u4NonPre2PremultipliedEn;
    u4Pre2NonPremultipliedEn = prsData->u4Pre2NonPremultipliedEn;

    if ((!u4NonPre2PremultipliedEn) && (!u4Pre2NonPremultipliedEn))
    {
        return 0;
    }
    else if ((u4NonPre2PremultipliedEn) && (u4Pre2NonPremultipliedEn))
    {
        return 0;
    }

    src_color_mode = dst_color_mode;

    pu1Read    = pu1SrcBase;
    pu1SrcLine = pu1SrcBase;
    src = (pixel **) GFX_Alloc( width * sizeof(pixel *), 32);
    if (NULL == src)
    {
        return 0;
    }

    for ( x = 0; x < width; x++ )
    {
        src[x] = (pixel *) GFX_Alloc( height * sizeof(pixel), 32);
        if (NULL == src[x])
        {
            return 0;
        }
    }

    for (y = 0; y < height; y++)
    {
        byte_index = 0;
        for (x = 0; x < width; x++)
        {
            pu1Src     = pu1Read;
            if ( src_color_mode == ARGB8888 )  //32bpp
            {
                src[x][y].a = pu1Src[byte_index + 3];
                src[x][y].r = pu1Src[byte_index + 2];
                src[x][y].g = pu1Src[byte_index + 1];
                src[x][y].b = pu1Src[byte_index + 0];

                byte_index += 4;
            }
            else  //16bpp
            {
                if ( src_color_mode == ARGB4444 )
                {
                    src[x][y].a = pu1Src[byte_index + 1] >> 4;
                    src[x][y].r = pu1Src[byte_index + 1] & 0xF;
                    src[x][y].g = pu1Src[byte_index + 0] >> 4;
                    src[x][y].b = pu1Src[byte_index + 0] & 0xF;
                }
                else if ( src_color_mode == ARGB1555 )
                {
                    src[x][y].a = pu1Src[byte_index + 1] >> 7;
                    src[x][y].r = ( pu1Src[byte_index + 1] >> 2 ) & 0x1F;
                    src[x][y].g = ( ( pu1Src[byte_index + 1] & 0x3 ) << 3 ) + ( pu1Src[byte_index + 0] >> 5 );
                    src[x][y].b = pu1Src[byte_index + 0] & 0x1F;
                }
                else if ( src_color_mode == RGB565 )
                {
                    src[x][y].a = 0;
                    src[x][y].r = pu1Src[byte_index + 1] >> 3;
                    src[x][y].g = ( ( pu1Src[byte_index + 1] & 0x7 ) << 3 ) + ( pu1Src[byte_index + 0] >> 5 );
                    src[x][y].b = pu1Src[byte_index + 0] & 0x1F;
                }

                byte_index += 2;
            }
        }

        pu1SrcLine += u4SrcPitch;
        pu1Read     = pu1SrcLine;
    } //~for

    pu1Write   = pu1DstBase;
    pu1DstLine = pu1DstBase;
    dst = (pixel **) GFX_Alloc( width * sizeof(pixel *), 32);
    if (NULL == dst)
    {
        return 0;
    }

    for ( x = 0; x < width; x++ )
    {
        dst[x] = (pixel *) GFX_Alloc( height * sizeof(pixel), 32 );
        if (NULL == dst[x])
        {
            return 0;
        }
    }

    for (y = 0; y < height; y++)
    {
        byte_index = 0;
        for (x = 0; x < width; x++)
        {
            pu1Dst     = pu1Write;
            if ( dst_color_mode == ARGB8888 )  //32bpp
            {
                dst[x][y].a = pu1Dst[byte_index + 3];
                dst[x][y].r = pu1Dst[byte_index + 2];
                dst[x][y].g = pu1Dst[byte_index + 1];
                dst[x][y].b = pu1Dst[byte_index + 0];

                byte_index += 4;
            }
            else  //16bpp
            {
                if ( dst_color_mode == ARGB4444 )
                {
                    dst[x][y].a = pu1Dst[byte_index + 1] >> 4;
                    dst[x][y].r = pu1Dst[byte_index + 1] & 0xF;
                    dst[x][y].g = pu1Dst[byte_index + 0] >> 4;
                    dst[x][y].b = pu1Dst[byte_index + 0] & 0xF;
                }
                else if ( dst_color_mode == ARGB1555 )
                {
                    dst[x][y].a = pu1Dst[byte_index + 1] >> 7;
                    dst[x][y].r = ( pu1Dst[byte_index + 1] >> 2 ) & 0x1F;
                    dst[x][y].g = ( ( pu1Dst[byte_index + 1] & 0x3 ) << 3 ) + ( pu1Dst[byte_index + 0] >> 5 );
                    dst[x][y].b = pu1Dst[byte_index + 0] & 0x1F;
                }
                else if ( dst_color_mode == RGB565 )
                {
                    dst[x][y].a = 0;
                    dst[x][y].r = pu1Dst[byte_index + 1] >> 3;
                    dst[x][y].g = ( ( pu1Dst[byte_index + 1] & 0x7 ) << 3 ) + ( pu1Dst[byte_index + 0] >> 5 );
                    dst[x][y].b = pu1Dst[byte_index + 0] & 0x1F;
                }

                byte_index += 2;
            }
        }

        pu1DstLine += u4DstPitch;
        pu1Write    = pu1DstLine;
    } //~for

    //expand color
    //src_global_alpha = global_alpha;
    //dst_global_alpha = global_alpha;

    for ( y = 0; y < height; y++ )
    {
        for ( x = 0; x < width; x++ )
        {
            src[x][y] = expand_color( src[x][y], src_color_mode, global_alpha );
            dst[x][y] = expand_color( dst[x][y], dst_color_mode, global_alpha );
        }
    }

    if (prsData->u4NonPre2PremultipliedEn)
    {
        alcom_core_NonPre2Premultiplied( src, dst, width, height, dst_color_mode, blend_mode, blend_normal, blend_ar );
    }
    else
    {
        alcom_core_Pre2NonPremultiplied( src, dst, width, height, dst_color_mode, blend_mode, blend_normal, blend_ar );
    }

    for ( x = 0; x < width; x++ )
    {
        GFX_Free( src[x] );
    }

    GFX_Free( src );

    pu1Read    = pu1SrcBase;
    pu1SrcLine = pu1SrcBase;
    pu1Write   = pu1DstBase;
    pu1DstLine = pu1DstBase;

    //shrink color
    for ( y = 0; y < height; y++ )
    {
        for ( x = 0; x < width; x++ )
        {
            dst[x][y] = shrink_color( dst[x][y], dst_color_mode, 0 );
        }
    }

    for ( y = 0; y < height; y++ )
    {
        byte_index = 0;
        pu1Dst     = pu1Write;
        for ( x = 0; x < width; x++ )
        {
            if ( dst_color_mode == ARGB8888 )  //32bpp
            {
                pu1Dst[byte_index + 0] = dst[x][y].b;
                pu1Dst[byte_index + 1] = dst[x][y].g;
                pu1Dst[byte_index + 2] = dst[x][y].r;
                pu1Dst[byte_index + 3] = dst[x][y].a;

                byte_index += 4;
            }
            else  //16bpp
            {
                if ( dst_color_mode == ARGB4444 )
                {
                    pu1Dst[byte_index + 0] = ( dst[x][y].g << 4 ) + dst[x][y].b;
                    pu1Dst[byte_index + 1] = ( dst[x][y].a << 4 ) + dst[x][y].r;
                }
                else if ( dst_color_mode == ARGB1555 )
                {
                    pu1Dst[byte_index + 0] = ( ( dst[x][y].g & 0x7 ) << 5 ) + dst[x][y].b;
                    pu1Dst[byte_index + 1] = ( dst[x][y].a << 7 ) + ( dst[x][y].r << 2 ) + ( dst[x][y].g >> 3 );
                }
                else if ( dst_color_mode == RGB565 )
                {
                    pu1Dst[byte_index + 0] = ( ( dst[x][y].g & 0x7 ) << 5 ) + dst[x][y].b;
                    pu1Dst[byte_index + 1] = ( dst[x][y].r << 3 ) + ( dst[x][y].g >> 3 );
                }

                byte_index += 2;
            }
        }
        pu1DstLine += u4DstPitch;
        pu1Write    = pu1DstLine;
    }

    for ( x = 0; x < width; x++ )
    {
        GFX_Free( dst[x] );
    }

    GFX_Free( dst );

    return 0;
}

#endif  //#if defined(GFX_ENABLE_SW_MODE)


