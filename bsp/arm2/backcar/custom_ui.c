/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * AutoChips Inc. (C) 2016. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

#include "x_types.h"
#include "BCLib.h"
#include <generated/atc_project.h>
#include "backcar_cfg.h"

#include "reserve_memory.h"
unsigned int backcarUI_base = 0;
unsigned int backcarUI_size = 0;


static HANDLE g_hMrf = NULL;
UINT32 *g_pFrameBuf = NULL;


int DIS_WIDTH = 0;
int DIS_HEIGHT = 0;

UINT32 ShowStopCarMsg(void);

BOOL BCAlphaBlend(int nXDest, int nYDest, int nWidth, int nHeight, UINT32 *pSrcData, int nXSrc, int nYSrc, BYTE Alpha)
{
    int i, j, cnt = 0;

    for (i = 0; i < nHeight; i++)
    {
        for (j = 0; j < nWidth; j++, cnt++)
        {
            g_pFrameBuf[(i + nYDest) * DIS_WIDTH + j + nXDest] = (pSrcData[cnt] & 0x00FFFFFF) | (Alpha << 24);
        }
    }

    return TRUE;
}

BOOL BCABitBlt(int nXDest, int nYDest, int nWidth, int nHeight, UINT32 *pSrcData, int nXSrc, int nYSrc, UINT32 BgColor, BYTE Alpha)
{
    int i, j, cnt = 0;
    register UINT32 u4ARGB = 0;

    for (i = 0; i < nHeight; i++)
    {
        for (j = 0; j < nWidth; j++, cnt++)
        {
            u4ARGB = (pSrcData[cnt] & 0x00FFFFFF);
            if (u4ARGB == BgColor)
            {
                g_pFrameBuf[(i + nYDest) * DIS_WIDTH + j + nXDest] = u4ARGB | (0);
            }
            else
            {
                g_pFrameBuf[(i + nYDest) * DIS_WIDTH + j + nXDest] = u4ARGB | (Alpha << 24);
            }
        }
    }

    return TRUE;
}

BOOL BCUIInvalidateRegion(INT32 x, INT32 y, UINT32 nWidth, UINT32 nHeight)
{
    int i, j, cnt = 0;

    for (i = 0; i < nHeight; i++)
    {
        for (j = 0; j < nWidth; j++, cnt++)
        {
            g_pFrameBuf[(i + y) * DIS_WIDTH + j + x] = RGB(0, 0, 0);
        }
    }

    return TRUE;
}

BOOL BufMemset()
{
    memset(g_pFrameBuf,0x0,DIS_WIDTH*DIS_HEIGHT*4);

    return TRUE;
}

extern _u4LCDWidth;
extern _u4LCDHeight;

BOOL CustomUIInit()
{
    BOOL bRet = FALSE;
    BITMAPOBJINFO BitmapInfo;


    backcarUI_base = READ64(ARM1PHY2ARM2UCV(ARM2_BC_UI_RSV_MEM_START_ADDR_4ARM2));
    backcarUI_size = READ64(ARM1PHY2ARM2UCV(ARM2_BC_UI_RSV_MEM_SIZE_4ARM2));
    Printf("[CustomUIInit]: backcarUI_base:0x%x, backcarUI_size:0x%x \r\n", backcarUI_base, backcarUI_size);

    g_hMrf = LoadMRF();
    if(0 != backcarUI_base){
        g_pFrameBuf = (UINT32 *)ARM1PHY2ARM2UCV(backcarUI_base);
    }else{
        Printf("error: [CustomUIInit]: backcarUI_base: is NULL\r\n");
        return FALSE;
    }

    DIS_WIDTH = _u4LCDWidth;
    DIS_HEIGHT = _u4LCDHeight;

    Printf("[CustomUIInit]: DIS_WIDTH = %d  DIS_HEIGHT = %d \n", DIS_WIDTH, DIS_HEIGHT);

    if (DIS_WIDTH * DIS_HEIGHT * 4 > backcarUI_size) {
        Printf("[CustomUIInit]: CustomUIInit Fail! Ui size larger than allocate size! \n");
        return FALSE;
    }

    #if USE_16BITS_TRACK_IMAGE
    // 16bits track image using color key(0xffff) to implemente alpha effect. so set it to 0xff.
    memset(g_pFrameBuf, 0xff, DIS_WIDTH * DIS_HEIGHT * 4);
    #else
    // 32 bits, when alpha ==0 , the pixel is transparent. so set it to 0.
    memset(g_pFrameBuf, 0, DIS_WIDTH * DIS_HEIGHT * 4);
    #endif

    // macro CAR_IMAGE_ON_UI can be enable in Makefile
    #if CAR_IMAGE_ON_UI
    bRet = GetBitmapInfo(g_hMrf, 14, &BitmapInfo);
    Printf("[CustomUIInit]: GetBitmapInfo: BitmapInfo.u4Width = %d  BitmapInfo.u4Height=%d \n", BitmapInfo.u4Width, BitmapInfo.u4Height);
    if (bRet)
    {
        BCABitBlt(0, 0, BitmapInfo.u4Width, BitmapInfo.u4Height,
            (UINT32 *)GetRCObjectMemAddr(g_hMrf, (RCOBJECT *)&BitmapInfo), 0, 0, RGB(0, 0, 0), 0x89);
    }
    #endif

    return TRUE;
}

BOOL PlayToneSound()
{
    BOOL bRet = FALSE;
    WAVOBJINFO WavInfo;

    bRet = GetWavResnfo(g_hMrf, 18, &WavInfo);
    if (bRet)
    {
        BCPlaySound(GetRCObjectMemAddr(g_hMrf, (RCOBJECT *)&WavInfo), WavInfo.u4SizeImage);
    }

    return TRUE;
}
UINT32 ShowDistance(UINT32 u4Dis)
{
    BOOL bRet = FALSE;
    BITMAPOBJINFO BitmapInfo;

    if (0 == u4Dis)
    {
        return ShowStopCarMsg(); 
    }
    else if (1 == u4Dis)
    {
        PlayToneSound();
    }

    bRet = GetBitmapInfo(g_hMrf, u4Dis, &BitmapInfo);   
    BCUIInvalidateRegion(0, 430, 240, 50);
    if (bRet)
    {
        BCABitBlt(90, 430, BitmapInfo.u4Width, BitmapInfo.u4Height, 
            (UINT32 *)GetRCObjectMemAddr(g_hMrf, (RCOBJECT *)&BitmapInfo), 0, 0, RGB( 0, 0, 0), 0xFF);
    }

    bRet = GetBitmapInfo(g_hMrf, 17, &BitmapInfo);
    if (bRet)
    {
        BCABitBlt(120, 430, BitmapInfo.u4Width, BitmapInfo.u4Height, 
            (UINT32 *)GetRCObjectMemAddr(g_hMrf, (RCOBJECT *)&BitmapInfo), 0, 0, RGB(0, 0, 0), 0xFF);
    }

    return TRUE;
}

UINT32 ShowStopCarMsg(void)
{
    BITMAPOBJINFO BitmapInfo;
    BOOL bRet = FALSE;

    BCUIInvalidateRegion(0, 430, 240, 50);

    bRet = GetBitmapInfo(g_hMrf, 11, &BitmapInfo);
    if (bRet)
    {
        BCABitBlt(40, 430, BitmapInfo.u4Width, BitmapInfo.u4Height,
            (UINT32 *)GetRCObjectMemAddr(g_hMrf, (RCOBJECT *)&BitmapInfo), 0, 0, RGB(0, 0, 0), 0XFF);
    }

    bRet = GetBitmapInfo(g_hMrf, 13, &BitmapInfo);
    if (bRet)
    {
        BCABitBlt(90, 430, BitmapInfo.u4Width, BitmapInfo.u4Height,
            (UINT32 *)GetRCObjectMemAddr(g_hMrf, (RCOBJECT *)&BitmapInfo), 0, 0, RGB(0, 0, 0), 0XFF);
    }

    bRet = GetBitmapInfo(g_hMrf, 12, &BitmapInfo);
    if (bRet)
    {
        BCABitBlt(140, 430, BitmapInfo.u4Width, BitmapInfo.u4Height,
            (UINT32 *)GetRCObjectMemAddr(g_hMrf, (RCOBJECT *)&BitmapInfo), 0, 0, RGB(0, 0, 0), 0XFF);
    }

    bRet = GetBitmapInfo(g_hMrf, 16, &BitmapInfo);
    if (bRet)
    {
        BCABitBlt(190, 430, BitmapInfo.u4Width, BitmapInfo.u4Height,
            (UINT32 *)GetRCObjectMemAddr(g_hMrf, (RCOBJECT *)&BitmapInfo), 0, 0, RGB(0, 0, 0), 0XFF);
    }

    return TRUE;
}

BOOL BC_DrawPoint(UINT32 x, UINT32 y, UINT32 u4RGB, UINT32 BgColor, BYTE Alpha)
{
    UINT32 u4ARGB;

    if ((x > DIS_WIDTH) || (y > DIS_HEIGHT))
    {
        return FALSE;
    }

    u4ARGB = (u4RGB & 0x00FFFFFF);
    if (u4ARGB == BgColor)
    {
        g_pFrameBuf[y * DIS_WIDTH + x] = u4ARGB | (0);
    }
    else
    {
        g_pFrameBuf[y * DIS_WIDTH + x] = u4ARGB | (Alpha << 24);
    }

    return TRUE;
}

static int BC_Sing(float d)
{
    int ret;
    if  (d > 0)
    {
        ret = 1;
    }
    else if  (d < 0)
    {
        ret = -1;
    }
    else
    {
        ret = 0;
    }
    
    return ret;
}

BOOL BC_DrawLine(POINT Start, POINT End, UINT32 Color)
{
    UINT32  u4Length;
    float   stepX, stepY, x, y;
    INT32   i, dx, dy;

    if ((ABS(Start.x) > DIS_WIDTH) || 
        (ABS(Start.y) > DIS_HEIGHT) || 
        (ABS(End.x) > DIS_WIDTH) || 
        (ABS(End.y) > DIS_HEIGHT))
    {
        return FALSE;
    }

    dx = End.x - Start.x; //need type cast: UINT32 -> INT32, otherwise the minus result will occur error.
    dy = End.y - Start.y;
    u4Length = MAX(ABS(dx), ABS(dy));

    stepX = ((float)dx) / u4Length;
    stepY = ((float)dy) / u4Length;

    x = (float)Start.x + 0.5 * BC_Sing(stepX);
    y = (float)Start.y + 0.5 * BC_Sing(stepY);

    for (i = 0; i < u4Length; i++)
    {
        BC_DrawPoint((UINT32)x, (UINT32)y, Color, RGB(0, 0, 0), 0xFF);
        x = x + stepX;
        y = y + stepY;
    }

    return TRUE;
}

BOOL BC_DrawArc(POINT Dot,UINT32 r,POINT Start,POINT End,UINT32 Color)
{
    INT32   x, y, z1, z2;
    UINT32  r0;
    LONG    strx, endx, stry, endy;

    if((ABS(Start.x) > DIS_WIDTH) ||
       (ABS(Start.y) > DIS_HEIGHT) || 
       (ABS(End.x) > DIS_WIDTH) || 
       (ABS(End.y) > DIS_HEIGHT))
    {
        return FALSE;
    }

    r0 = r * r;
    strx = (Start.x < End.x) ? Start.x : End.x;
    endx = (Start.x > End.x) ? Start.x : End.x;
    stry = (Start.y < End.y) ? Start.y : End.y;
    endy = (Start.y > End.y) ? Start.y : End.y;

    for (x = strx; x <= endx; x++)
    {
        for (y = stry; y <= endy; y++)
        {
            z1 = (x - Dot.x) * (x - Dot.x);
            z2 = (y - Dot.y) * (y - Dot.y);
            if (((z1 + z2) < (r0 + r)) && ((z1 + z2) > (r0 - r)))
            {
                BC_DrawPoint((UINT32)x, (UINT32)y, Color, RGB(0, 0, 0), 0xff);
            }
        }
    }

    return TRUE;
}



