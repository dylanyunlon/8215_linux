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
#include "drv_osd_if.h"
#include "osd_if_pdd.h"
#include "display.h"
#include "drv_if_pmx.h"
#include "mrf.h"
#include "osd_inc.h"
#include "pmx_hal.h"
#include "log.h"
#include "reserve_memory.h"
#include <generated/atc_project.h>
#define  BACKCAR_UI_OSD_PLANE    OSD_PLANE_3

__u32 G_OUTPUT_WIDTH = 800;
__u32 G_OUTPUT_HEIGHT = 480;

static __u32 g_u4Rgn = -1;/*region for backcar UI*/
static __u32 g_u4LogoRgn = -1;/*region for logo*/
static __u32 _u4VdpOutWidth, _u4VdpOutHeight;
static struct OVERLAY_PARAM rVdpParam;

#define VDP_1                   0   /* VDP1*/
#define VDP_2                   1   /* VDP2*/
#define VSYNC_PER_FRAME    (3000)

extern __u32 gu4CurTVMode;
extern void memcpy(void *dest, void *src, unsigned int count);
static void vSetFBConfigToShareMemory(FB_CONFIG_T *prConfig);

#define  BOOT_ANIMATION_OSD_PLANE    OSD_PLANE_1

void  DisableDisplayOverlay(void)
{
#ifdef CONFIG_ATC_OS_linux
    vPmxNotMixPlane(PMX_1, PMX_HW_OSD1_MIX);/*disable boot animation osd */
    vSclHalIsr(0, 0);
#else
    vPmxNotMixPlane(PMX_1, PMX_HW_PLANE_4); /* disable android PRIMARY_SURF_ID layer for arm2 backcar*/
    vSclHalIsr(0, 0);
#endif
}

void  EnableDisplayOverlay(void)
{
#ifdef CONFIG_ATC_OS_linux
    vPmxMixPlane(PMX_1, PMX_HW_OSD1_MIX);/*enable boot animation osd */
#else
    vPmxMixPlane(PMX_1, PRIMARY_SURF_PLANE);/* enable PRIMARY_SURF_ID layer after arm2 backcar leave*/
    //vSclHalIsr(0, 0);
    vPmxHalMainIsr(45,NULL);
#endif
}

extern char backcar_data[];

HANDLE LoadMRF()
{
    void* lpBuf = backcar_data;
    return (HANDLE)lpBuf;
}

static bool VDPInit(__u32 u4YAddr, __u32 u4CAddr, __u32 u4Width, __u32 u4Height,
    RECT *prInRgn, RECT *prOutRgn, bool deint)
{
    memset(&rVdpParam, 0, sizeof(rVdpParam));
    rVdpParam.u4Idx = VDP_1;
    rVdpParam.u4SrcWidth = u4Width;
    rVdpParam.u4SrcHeight = u4Height;
    memcpy(&rVdpParam.rSrcRect, prInRgn, sizeof(RECT));
    memcpy(&rVdpParam.rDstRect, prOutRgn, sizeof(RECT));
    rVdpParam.u4PhysicalAddressY = u4YAddr;
    rVdpParam.u4PhysicalAddressC = u4CAddr;
    rVdpParam.u4Duration = VSYNC_PER_FRAME;
    rVdpParam.u4Status = 0;
    rVdpParam.device_name = BACKCAR;
    rVdpParam.fgProgSrc = FALSE;
    rVdpParam.fgTopFiledFirst = FALSE;

    if (deint) {
        rVdpParam.u4Flags = VDP_UPDATE_OVERLAY | VDP_TOP_LEVEL | VDP_SCANLINE_MODE | VDP_ENABLE_DEINT;
    } else {
        rVdpParam.u4Flags = VDP_UPDATE_OVERLAY | VDP_TOP_LEVEL | VDP_SCANLINE_MODE;
    }

    /*set vcp before qbuf in vdpinit*/
    /*vCPSetYUVGain(0xf0, 0xaa, 0x140);*/
    /*vCPOn(0);*/

    /*vPmxHalNotMixPlane(0, PMX_HW_PLANE_4); //for disable android osd*/

    if (VDP_IOControl(VIDIOC_QBUF, &rVdpParam, NULL)) {
        return FALSE;
    }

    return TRUE;
}

bool BackCarVdpFlip(__u32 u4YAddr, __u32 u4CAddr, bool deint)
{
    rVdpParam.u4PhysicalAddressY = u4YAddr;
    rVdpParam.u4PhysicalAddressC = u4CAddr;
    /*now only for tvd backcar 576 or 480, and this flag is for display needs*/
    if (1 == gu4CurTVMode) { /*pal*/
        rVdpParam.fgTopFiledFirst = 1;
    } else {
        rVdpParam.fgTopFiledFirst = 0;
    }

    /*Printf("Flip rVdpParam.fgTopFiledFirst %d, u4SrcWidth %d, u4SrcHeight %d\r\n", rVdpParam.fgTopFiledFirst
        , rVdpParam.u4SrcWidth, rVdpParam.u4SrcHeight);*/
    if (deint) {
        rVdpParam.u4Flags = VDP_TOP_LEVEL | VDP_SCANLINE_MODE | VDP_ENABLE_DEINT;
    } else {
        rVdpParam.u4Flags = VDP_TOP_LEVEL | VDP_SCANLINE_MODE;
    }

    if (VDP_IOControl(VIDIOC_QBUF, &rVdpParam, NULL)) {
        return FALSE;
    }

    return TRUE;
}

VOID BackCarSetVdpRect(bool fgPal, __u32 *pu4Width, __u32 *pu4Height, RECT *SrcRect, RECT *DstRect)
{
    if (fgPal) {
        *pu4Width = 720;
        *pu4Height = 576;

        SrcRect->left = 0;
        SrcRect->top = 0;
        SrcRect->right = 720;
        SrcRect->bottom = 576;

        DstRect->left = 0;
        DstRect->top = 0;
        DstRect->right = _u4LCDWidth;
        DstRect->bottom = _u4LCDHeight;
    } else {
        *pu4Width = 720;
        *pu4Height = 480;

        SrcRect->left = 0;
        SrcRect->top = 0;
        SrcRect->right = 720;
        SrcRect->bottom = 480;

        DstRect->left = 0;
        DstRect->top = 0;
        DstRect->right = _u4LCDWidth;
        DstRect->bottom = _u4LCDHeight;
    }
}

bool BackCarOverlayInit(__u32 u4DataPA, __u32 u4VDPDstYPA, __u32 u4VDPDstCPA,
    bool fgOverlayInit, bool fgPal, bool deint)
{
    RECT    rRect = {0}, SrcRect = {0}, DstRect = {0};
    /*__u32 *pu4Header1 =  (__u32 *) 0xfd020304;*/
    __u32  u4RgnList;
    __u32  u4Width, u4Height;
    __u32  u4Plane = BACKCAR_UI_OSD_PLANE;
    if (FALSE == fgOverlayInit) {
        /*_u4LCDWidth = i4Width;*/
        /*_u4LCDHeight = i4Height;*/
        /*_u4DispMode = G_DISPLAY_MODE;*/
        rRect.left = 0;
        rRect.top = 0;
        rRect.right = _u4LCDWidth - rRect.left;
        rRect.bottom = _u4LCDHeight - rRect.top;

        OSD_BASE_SetOsdPosition(u4Plane, PRIMARY_OSD_X_OFFSET, PRIMARY_OSD_Y_OFFSET);

        OSD_SC_Scale(u4Plane, TRUE, _u4LCDWidth, _u4LCDHeight, G_OUTPUT_WIDTH, G_OUTPUT_HEIGHT);

        OSD_RGN_LIST_Create(&u4RgnList);
        u4RgnList  = u4Plane;

        OSD_RGN_Create(&g_u4Rgn, _u4LCDWidth, _u4LCDHeight, (void *)u4DataPA,
                   OSD_CM_ARGB8888_DIRECT32, (_u4LCDWidth * 4), 0, 0, _u4LCDWidth, _u4LCDHeight);

#if 0
        OSD_RGN_Set(u4Rgn, (__s32)OSD_RGN_COLOR_KEY, 31);
        OSD_RGN_Set(u4Rgn, (__s32)OSD_RGN_COLOR_KEY_EN, TRUE);
#endif

        OSD_RGN_LIST_DetachAll(u4RgnList);
        OSD_RGN_Insert(g_u4Rgn, u4RgnList);
        SetPlaneRgn(u4RgnList, g_u4Rgn);
        i4OsdPlaneFlipTo(u4Plane, u4RgnList);
        /*enable backcar osd*/
        i4OsdPlaneEnble(u4Plane, TRUE);

        DisableDisplayOverlay();

        /* *pu4Header1 &= 0xf0000000;*/
        /* *pu4Header1 = *pu4Header1 | ((PA_START  + 0x20)>> 4);*/
    } else {
        BackCarSetVdpRect(fgPal, &u4Width, &u4Height, &SrcRect, &DstRect);

        VDPInit(u4VDPDstYPA, u4VDPDstCPA, u4Width, u4Height, &SrcRect, &DstRect, deint);

        /*Set brightness if you need*/
        /*vTconSetBrightness(40);*/
        /*vTconSetContrast(25);*/
        /*vTconSetSaturation(50);*/
    }

    return  TRUE;
}

bool HideBackCarOverlay(bool fgHideOverlay)
{
    __u32  u4Plane = BACKCAR_UI_OSD_PLANE;

    if (fgHideOverlay) {
        i4OsdPlaneEnble(u4Plane, FALSE);

        EnableDisplayOverlay();

        if (g_u4Rgn != -1) {
            Printf("[HideBackCarOverlay] when backcar exit delete region \r\n");
            OSD_RGN_Delete(g_u4Rgn);
            g_u4Rgn = -1;
        }
    }

    /* this IOCTL will show primary surface ,when no signal we want black screen not primary surface*/
    if (VDP_IOControl(VIDIOC_STREAMOFF, &rVdpParam, NULL)) {
      return FALSE;
    }

#ifdef CONFIG_ATC_OS_linux
    //vPmxHalMainIsr(0, 0);
    //vSclHalIsr(0, 0);
#else
  //vPmxHalMainIsr(0, 0);
  //vSclHalIsr(0, 0);
#endif
    vPmxHalMainIsr(45,NULL);

    return TRUE;
}


