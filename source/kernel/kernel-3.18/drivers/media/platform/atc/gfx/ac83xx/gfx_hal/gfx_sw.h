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

#ifndef GFX_SW_H
#define GFX_SW_H


//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------

#include "gfx_if.h"


#if defined(GFX_ENABLE_SW_MODE)
//---------------------------------------------------------------------------
// Configurations
//---------------------------------------------------------------------------

//#define GFX_SW_FLOAT_VERSION  /* linux kernel should not use float */

#if 1//(CONFIG_DRV_LINUX)
#define  GFX_Alloc    GFX_ALLOC_MEM
#define  GFX_Free     GFX_FREE_MEM
#else
extern void* x_alloc_aligned_nc_mem(UINT32 u4Size, UINT32 u4Align);
extern void  x_free_aligned_nc_mem(void *pUser);
#define  GFX_Alloc    GFX_ALLOC_CH1_MEM
#define  GFX_Free     GFX_FREE_CH1_MEM
#endif

//---------------------------------------------------------------------------
// Type definitions
//---------------------------------------------------------------------------

typedef struct _GFX_YCBCR
{
    UINT8 y;
    UINT8 cb;
    UINT8 cr;
} GFX_YCBCR_T;


typedef struct _GFX_YCBCR2RGB_DATA_T
{
    UINT8 *pu1LumaBase;
    UINT8 *pu1ChromaBase;
    UINT8 *pu1DstBase;
    UINT32 u4LumaPitch;
    UINT32 u4ChromaPitch;
    UINT32 u4DstPitch;
    UINT32 u4DstCM;
    UINT32 u4YCFormat;
    UINT32 u4VideoStd;
    UINT32 u4VideoSys;
    UINT32 u4VideoClip;
    UINT32 u4PicSelect;
    UINT32 u4SwapMode;
    UINT32 u4Width;
    UINT32 u4Height;
} GFX_YCBCR2RGB_DATA_T;


typedef struct _GFX_GRADIENT_DATA_T
{
    UINT8 *pu1DstBase;
    UINT32 u4DstPitch;
    UINT32 u4DstCM;
    UINT32 u4X_Pix_Inc;
    UINT32 u4Y_Pix_Inc;
    UINT32 u4Width;
    UINT32 u4Height;
    UINT32 u4GradMode;
    UINT32 u4RectColor;
    UINT8 u1Delta_X_C0;
    UINT8 u1Delta_X_C1;
    UINT8 u1Delta_X_C2;
    UINT8 u1Delta_X_C3;
    UINT8 u1Delta_Y_C0;
    UINT8 u1Delta_Y_C1;
    UINT8 u1Delta_Y_C2;
    UINT8 u1Delta_Y_C3;
} GFX_GRADIENT_DATA_T;


typedef struct _GFX_BITBLT_DATA_T
{
    UINT8 *pu1SrcBase;
    UINT8 *pu1DstBase;
    UINT32 u4SrcPitch;
    UINT32 u4DstPitch;
    UINT32 u4SrcCM;
    UINT32 u4DstCM;
    UINT32 u4Width;
    UINT32 u4Height;
    UINT32 u4ColorMin;
    UINT32 u4ColorMax;
    UINT32 u4TransEn;
    UINT32 u4ColchgEn;
    UINT32 u4KeynotEn;
    UINT32 u4Color;
} GFX_BITBLT_DATA_T;


// alpha composition loop mode
typedef struct _GFX_ACLM_DATA_T
{
    UINT8 *pu1SrcBase;
    UINT8 *pu1DstBase;
    UINT32 u4SrcPitch;
    UINT32 u4DstPitch;
    UINT32 u4SrcCM;
    UINT32 u4DstCM;
    UINT32 u4Width;
    UINT32 u4Height;
    UINT32 u4AlComAr;
    UINT32 u4AlComOpCode;
    UINT32 u4AlComRectSrc;
    UINT32 u4RectColor;
    UINT32 u4GlobalAlpha;
    UINT32 u4AlcomNormal;

    UINT32 u4NonPre2PremultipliedEn;
    UINT32 u4Pre2NonPremultipliedEn;
} GFX_ACLM_DATA_T;


typedef struct _GFX_ROP_DATA_T
{
    UINT8 *pu1SrcBase;
    UINT8 *pu1DstBase;
    UINT32 u4SrcPitch;
    UINT32 u4DstPitch;
    UINT32 u4SrcCM;
    UINT32 u4DstCM;
    UINT32 u4Width;
    UINT32 u4Height;
    UINT32 u4RopOpCode;
    UINT32 u4RopNoWr;
} GFX_ROP_DATA_T;


typedef struct _GFX_IDX2DIR_DATA_T
{
    UINT8 *pu1SrcBase;
    UINT8 *pu1DstBase;
    UINT32 u4SrcPitch;
    UINT32 u4DstPitch;
    UINT32 u4CharCM;        // src color mode
    UINT32 u4DstCM;         // dst color mode
    UINT32 u4Width;
    UINT32 u4Height;
    UINT8 *pu1PaleBase;     // palette table address
    UINT32 u4MsbLeft;
    UINT32 u4ByteAlign;
    UINT32 u4SrcPitchEn;   // mt8550 new feature.
} GFX_IDX2DIR_DATA_T;


typedef struct _GFX_HORI2VERT_DATA_T
{
    UINT8 *pu1SrcBase;
    UINT8 *pu1DstBase;
    UINT32 u4SrcPitch;
    UINT32 u4DstPitch;
    UINT32 u4SrcCM;
    UINT32 u4DstCM;
    UINT32 u4Width;
    UINT32 u4Height;
    UINT32 u4DstPitchDec;
    UINT32 u4StrDstWidth;
    UINT32 u4StrDstHeight;
} GFX_HORI2VERT_DATA_T;


//---------------------------------------------------------------------------
// Inter-file functions
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------------

extern INT32 GFX_SwInit(void);

extern INT32 GFX_SwGetRegBase(UINT32 u4GfxHwId, UINT32 **ppu4RegBase);

extern void GFX_SwISR(void);

extern INT32 GFX_SwSetCallBack(void (*pfnCallBack)(void *pvTag), void *pvTag);

extern UINT32 GFX_SwGetOpCount(void);

extern INT32 GFX_SwAction(UINT32 u4GfxHwId);

extern void GFX_SwSetColorComponent(UINT8 **ppu1DestFb, UINT32 u4CM,
    const INT32 *pi4sColorComponent);

extern void GFX_SwGetColorComponent(UINT8 **ppu1DestFb, UINT32 u4CM,
    INT32 *pi4ColorComponent);

#if 1
extern INT32 GFX_SwBitBlt(UINT8 *pu1SrcBase, UINT8 *pu1DstBase, UINT32 u4SrcPitch,
        UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM, UINT32 u4Width,
        UINT32 u4Height, UINT32 u4ColorMin, UINT32 u4ColorMax, UINT32 u4DstColorMin,
        UINT32 u4DstColorMax, UINT32 u4TransEn,    UINT32 u4ColchgEn, UINT32 u4DisSrcKey, 
        UINT32 u4SrcKeyIn, UINT32 u4DisDstKey, UINT32 u4DstKeyIn, UINT32 u4KeynotEn, UINT32 u4Color, UINT32 u4Alpha);
#else
extern INT32 GFX_SwBitBlt(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    UINT32 u4SrcPitch, UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM,
    UINT32 u4Width, UINT32 u4Height, UINT32 u4ColorMin, UINT32 u4ColorMax,
    UINT32 u4TransEn, UINT32 u4ColchgEn, UINT32 u4KeynotEn, UINT32 u4Color);
#endif
extern INT32 GFX_24Bpp_SwBitBlt(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    UINT32 u4SrcPitch, UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM,
    UINT32 u4Width, UINT32 u4Height, UINT32 u4ColorMin, UINT32 u4ColorMax,
    UINT32 u4TransEn, UINT32 u4ColchgEn, UINT32 u4KeynotEn, UINT32 u4Color, UINT32 u4GAlphaEn, UINT32 u4GAlpha);

extern INT32 GFX_SwBitBlt_NewMethod(UINT8 *pu1SrcBase, UINT8 *pu1DstBase, UINT32 u4SrcPitch,
    UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM, UINT32 u4Width,
    UINT32 u4Height, UINT32 u4ColorMin, UINT32 u4ColorMax, UINT32 u4TransEn,
    UINT32 u4ColchgEn, UINT32 u4KeynotEn, UINT32 u4Color);
extern INT32 GFX_SwBitBlt_DstFlip(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    UINT32 u4SrcPitch, UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM,
    UINT32 u4Width, UINT32 u4Height, UINT32 u4ColorMin, UINT32 u4ColorMax,
    UINT32 u4TransEn, UINT32 u4ColchgEn, UINT32 u4KeynotEn, UINT32 u4Color,UINT32 u4mirror,UINT32 u4flip);
extern INT32 GFX_SwAlphaMapBitBlt(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    UINT32 u4SrcPitch, UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM,
    UINT32 u4Width, UINT32 u4Height);

extern INT32 GFX_SwAlphaComposePass(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    UINT32 u4SrcPitch, UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM,
    UINT32 u4Width, UINT32 u4Height, UINT32 u4AlphaValue, UINT32 u4AlcomPass);

extern INT32 GFX_SwBlock2Linear(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    INT32 i4Width, INT32 i4Height, INT32 i4MBWidth, INT32 i4MBHeight);

extern INT32 GFX_SwBlock2Swap(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    INT32 i4Width, INT32 i4Height, INT32 i4MBWidth, INT32 i4MBHeight);

extern INT32 GFX_SwBlock2Mergetop(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    INT32 i4Width, INT32 i4Height, INT32 i4MBWidth, INT32 i4MBHeight);

extern INT32 GFX_SwYCbCr420LinearScan(const GFX_YCBCR2RGB_DATA_T *prsData);

extern INT32 GFX_SwYCbCr601toArgbVideoEqu(const GFX_YCBCR_T *prsSrc,
    INT32 *pi4ColorComponent);

extern INT32 GFX_SwYCbCr601toArgbCompEqu(const GFX_YCBCR_T *prsSrc,
    INT32 *pi4ColorComponent);

extern INT32 GFX_SwYCbCr709toArgbVideoEqu(const GFX_YCBCR_T *prsSrc,
    INT32 *pi4ColorComponent);

extern INT32 GFX_SwYCbCr709toArgbCompEqu(const GFX_YCBCR_T *prsSrc,
    INT32 *pi4ColorComponent);

extern INT32 GFX_SwYCbCr2RGB(const GFX_YCBCR2RGB_DATA_T *prsData);

extern INT32 GFX_SwAlphaBlending(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    UINT32 u4SrcPitch, UINT32 u4DstPitch, UINT32 u4Dst_CM, UINT32 u4Alpha,
    UINT32 u4Width, UINT32 u4Height);

extern INT32 GFX_SwMsAlphaBlending(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    UINT32 u4SrcPitch, UINT32 u4DstPitch, UINT32 u4Dst_CM, UINT32 u4Alpha,
    UINT32 u4SrcPremult, UINT32 u4Width, UINT32 u4Height);

#if 0
extern INT32 GFX_SwBigEndianAndLittleEndianConversion(UINT8 *pu1DstAddr, 
    UINT8 *pu1SrcAddr, UINT32 u4TotalPixels, UINT32 u4ColorMode);
#endif
extern INT32 GFX_SwGradientFill(const GFX_GRADIENT_DATA_T *prsData);
#if (!CONFIG_DRV_LINUX)
extern FLOAT GFX_SwFpS61toFloat(INT8 i1Value);

extern FLOAT GFX_SwClipping(FLOAT fValue);
#endif
//add by msz00441 for rop & inx2dir &h2vline
extern INT32 GFX_SwRopBitBlt(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
    UINT32 u4SrcPitch, UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM,
    UINT32 u4Width, UINT32 u4Height, UINT32 u4RopOpCode, UINT32 u4nowr,
    UINT32 u4cmpflag, UINT8 *pu1DstX, UINT8 *pu1DstY, UINT8 *pu1CmpNowr, UINT32 u4JavaXorClr);
// 8555 or newer Ic, or 8550 ECO IC
INT32 GFX_SwRopBitBlt8555(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
                      UINT32 u4SrcPitch, UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM,
                      UINT32 u4Width, UINT32 u4Height, UINT32 u4RopOpCode, UINT32 u4nowr,
                      UINT32 u4cmpflag, UINT8 *pu1DstX, UINT8 *pu1DstY, UINT8 *pu1CmpNowr, UINT32 u4JavaXorClr,
                      UINT32 u4SrcAlphaCheck);
// 8555 or newer Ic, or 8550 ECO IC
INT32 GFX_SwRopBitBlt8580(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
                      UINT32 u4SrcPitch, UINT32 u4DstPitch, UINT32 u4SrcCM, UINT32 u4DstCM,
                      UINT32 u4Width, UINT32 u4Height, UINT32 u4RopOpCode, UINT32 u4nowr,
                      UINT32 u4cmpflag, UINT8 *pu1DstX, UINT8 *pu1DstY, UINT8 *pu1CmpNowr, UINT32 u4JavaXorClr,
                      UINT32 u4SrcAlphaCheck, UINT32 u4ColorRepEn);


extern INT32 GFX_SwInx2DirBitBlt(UINT8 *pu1SrcBase, UINT8 *pu1DstBase,
	UINT32 u4SrcPitch,UINT32 u4DstPitch,UINT32 u4CharCM, UINT32 u4DstCM,
	UINT32 u4Width, UINT32 u4Height, UINT32 u4LnStByeAl, UINT32 u4MsbLeft,
	UINT8 *pu1PalBase);
extern INT32 GFX_SwH2VLine(UINT8 *pu1SrcBase, UINT8 *pu1DstBase, UINT32 u4DstPitch,
                    UINT32 u4DstCM, UINT32 u4SrcWidth, UINT32 u4SrcHeight,
                    UINT32 u4StrDstWidth, UINT32 u4StrDstHeight, UINT32 u4Alpha, UINT32 u4wise);
//end of add by msz00441 for rop & inx2dir &h2vline

extern INT32 GFX_SwPgigDecode(UINT8 *pu1SrcBase, UINT8 *pu1DstBase, UINT8 *pu1PalBase,
        UINT32 u4Width, UINT32 u4Height, UINT32 u4DstPitch, UINT32 u4DstCM);

extern void GFX_SwGetColorComponent_5381(UINT8 **ppu1DestFb, UINT32 u4CM,
    INT32 *pi4ColorComponent);

#endif //#if defined(GFX_ENABLE_SW_MODE)


#endif // GFX_SW_H


