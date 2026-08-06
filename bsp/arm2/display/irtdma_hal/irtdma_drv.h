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
#ifndef __IRT_DMA__H__
#define __IRT_DMA__H__

#include "chip_ver.h"
#include "types.h"
//Note:
//if you want to use irt_dma module,wr_channel must support rear(two wr_channel) and support ypbpr(buffer size is 0x600000/0x4)
//Resolution limitation is 1280 * 800
#define EV_IRT_MAX_WIDTH                (4*1024)
#define EV_IRT_MAX_HEIGHT               (2*1024)

typedef struct
{
    BOOL   fg5351Mode;
    UCHAR  uc5351ModeSel;
    BOOL   fgScanLineMode;
    UINT32 u4SrcWidthAlign;
    UINT32 u4SrcHeightAlign;
    BOOL   fgBlockBurstRead; //add@jgao
} IRT_DMA_SRC_MODE_T;

extern IRT_DMA_SRC_MODE_T   g_rIrtSrcMode;

#define EV_IRTDMA_W_ALIGN   (g_rIrtSrcMode.u4SrcWidthAlign)
#define EV_IRTDMA_H_ALIGN   (g_rIrtSrcMode.u4SrcHeightAlign)

typedef enum
{
    IRT_DMA_MODE_ROTATE_0 = 0,      //0, normal
    IRT_DMA_MODE_ROTATE_0_MIRROR,   //1, Rotate 0 + Mirror
    IRT_DMA_MODE_ROTATE_90,         //2, Rotate 90
    IRT_DMA_MODE_ROTATE_90_MIRROR,  //3, Rotate 90 + Mirror
    IRT_DMA_MODE_ROTATE_180,        //4, Rotate 180
    IRT_DMA_MODE_ROTATE_180_MIRROR, //5, Rotate 180 + Mirror
    IRT_DMA_MODE_ROTATE_270,        //6, Rotate 270
    IRT_DMA_MODE_ROTATE_270_MIRROR, //7, Rotate 270 + Mirror
    IRT_DMA_MODE_MAX,               //8
    IRT_DMA_MODE_CBCR_ALIGN,        //9 HW NOT supported
} IRT_DMA_MODE_T;

typedef enum
{
    IRT_DMA_ERR_NONE = 0,
    IRT_DMA_ERR_INV_ARG,
    IRT_DMA_ERR_OUT_OF_MEM,
    IRT_DMA_ERR_UNINIT,
    IRT_DMA_ERR_UNDEF_ERR,
    IRT_DMA_ERR_WOULD_BLOCK,
    IRT_DMA_ERR_CMP_FAIL
} IRT_DMA_ERR_CODE_T;

typedef struct
{
    UINT32        *pu4SrcYBufAddr;
    UINT32        *pu4SrcCBufAddr;
    UINT32        *pu4DstYBufAddr;
    UINT32        *pu4DstCBufAddr;
    UINT32         u4FrameWidth;
    UINT32         u4FrameHeight;
    IRT_DMA_MODE_T eModeOpt;
    BOOL           fgScanLineMode;
    BOOL           fg5351Mode;
    BOOL           fgBlockBurstRead; 
}IRT_DMA_APP_INFO_T, *PIRT_DMA_APP_INFO_T;

#define SCANLINE_MODE_WIDTH_AGLINE  16
#define SCANLINE_MODE_HEIGHT_AGLINE 32
#define BLOCK_MODE_WIDTH_AGLINE     16
#define BLOCK_MODE_HEIGHT_AGLINE    32
#define B5351_MODE_WIDTH_AGLINE     64
#define B5351_MODE_HEIGHT_AGLINE    32
#define IRT_DMA_MODE_AGLINE         16

#endif
