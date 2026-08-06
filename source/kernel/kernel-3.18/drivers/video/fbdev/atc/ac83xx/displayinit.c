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
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/fb.h>

#include <media/atc/drv_osd_if.h>
#include <media/atc/drv_av_d.h>
#include <media/atc/display_inc.h>
#include <media/atc/display.h>
#include <media/atc/pmx_hal.h>
#include "x_os.h"
#include "drv_pmx.h"
#include "vdp_hal.h"
#include "tve_hal.h"
#include "scl_if.h"
#include "osd_if_pdd.h"
#include "osd_inc.h"
#include "drv_imgresz.h"
#include "tcon.h"
#include "log.h"
#include "osd_hw.h"
#include <generated/atc_project.h>
#include "soc_cfg.h"


typedef struct {
	__u32 u4Plane;
	bool   fgEnable;
} SURF_INTER_DATA_T;

static SURF_INTER_DATA_T _afgOSDPlaneState[4];


static __u32 BytesPerPixel(__u32 u4Format);


void DisplaySclInit(bool fgHwReset)
{
	SCL_Init(fgHwReset);

	if (fgHwReset) {
		FB_PRINT(FB_LOG_LVL_DBG, "", "[DDI]DisplaySclInit: _u4LCDWidth = %d, _u4LCDHeight = %d\n",
			(int)_u4LCDWidth, (int)_u4LCDHeight);
		SCL_Config();
	}
}

/*static __u32 rgn_list[2], rgn[2], plane;*/
static __u32 rgn_list, rgn, plane;
static unsigned int fb_addr1, fb_addr2;
/*add by mtk94020 for subtitle*/
static __u32 st_rgn_list, st_rgn, st_plane;
#define SUBTITLE_SURF_ID    OSD_PLANE_4
#define SUBTITLE_SURF_REAR_ID   OSD_PLANE_8

enum {
	OSD_OUTPUT_PATH_FRONT = 0x0,
	OSD_OUTPUT_PATH_REAR = 0x01,
};

void TurnOnTve(__u32 u4LayerID, bool fgOn)
{
	static __u32 u4BitMask;

	if (fgOn) {
		if (u4BitMask == 0) {
			vTveHalSetMode(_u4RearOutputMode);
			if(fgGetChipFeature(FEATURE_DVD) || fgGetChipFeature(FEATURE_CD)) {
				vTveHalEnable(TVE_NONE);
			} else {
				vTveHalEnable(TVE_AP);
			}
			/*vTveHalEnable(TVE_AP); //set default video source from ap*/
			/*vTveHalEnableCB(0);  //enable color bar for temp test*/
		}

		u4BitMask |= (1 << u4LayerID);
	} else {
		u4BitMask &= ~(1 << u4LayerID);

		if (u4BitMask == 0) {
			vTveHalDisable();
		}
	}
}
EXPORT_SYMBOL(TurnOnTve);


__u32 BytesPerPixel(__u32 u4Format)
{
	__u32 u4BytesPerPixel = 0;

	switch (u4Format) {

	case OSD_CM_ARGB8888_DIRECT32:
		u4BytesPerPixel = 4;
		break;

	case OSD_CM_RGB565_DIRECT16:
		u4BytesPerPixel = 2;
		break;

	default:
		break;
	}

	return u4BytesPerPixel;

}
bool subtitle_osd_init(unsigned int addr, OSD_DATA_T *prData)
{
	__s32 ret;
	__u32 dstWidth;
	__u32 dstHeight;
	__u32 posx;
	__u32 posy;
	__u32 u4BytesPerPixel = BytesPerPixel(prData->u4PixelFormat);
	__u32 srcWidth, srcHeight;

	srcWidth  = prData->u4Width;
	srcHeight  = prData->u4Height;

	if (1) {
		dstWidth = prData->rDestRect.right - prData->rDestRect.left;
		dstHeight = prData->rDestRect.bottom - prData->rDestRect.top;
		posx    = prData->rDestRect.left;
		posy    = prData->rDestRect.top;
		FB_PRINT(FB_LOG_LVL_DBG, "", "using private dest rect\n");
	}

	if (u4BytesPerPixel == 0) {
		FB_PRINT(FB_LOG_LVL_DBG, "", "FAILED to get bytes per pixel %d\n", (int)prData->u4PixelFormat);

		return FALSE;
	}

	/*OSD_Init();*/
	st_plane = SUBTITLE_SURF_ID + prData->u4BlockID; /* OSD HW INDEX*/

	if (prData->u4Flags & OSD_DATA_OUPPUT_PATH) {
		if (prData->u4OutputPath  == OSD_OUTPUT_PATH_REAR) {
			st_plane = SUBTITLE_SURF_REAR_ID;
		}

	}

	st_rgn_list = st_plane;

	FB_PRINT(FB_LOG_LVL_DBG, "", "subtitle_osd_init plane id %d  rect  %d %d %d %d\n",
		(int)st_plane, (int)posx, (int)posy, (int)dstWidth, (int)dstHeight);

	if (!_afgOSDPlaneState[prData->u4BlockID].fgEnable) {
		_afgOSDPlaneState[prData->u4BlockID].u4Plane = st_plane;
		OSD_BASE_SetOsdPosition(st_plane, posx, posy);
		OSD_SC_Scale(st_plane, FALSE, dstWidth, dstHeight, dstWidth, dstHeight);
		OSD_RGN_LIST_DetachAll(st_rgn_list);
		OSD_RGN_Create(&st_rgn, srcWidth, srcHeight, (void *)addr, prData->u4PixelFormat,
			       (srcWidth * u4BytesPerPixel), 0, 0,  dstWidth, dstHeight);

		if (prData->u4Flags & OSD_DATA_SRC_CLR_KEY) {
			OSD_RGN_Set(st_rgn, (__s32)OSD_RGN_COLOR_KEY, prData->u4SrcColorKey);
			OSD_RGN_Set(st_rgn, (__s32)OSD_RGN_COLOR_KEY_EN, TRUE);
		}

		ret = OSD_RGN_Insert(st_rgn, st_rgn_list);

		if (ret) {
			FB_PRINT(FB_LOG_LVL_DBG, "", "OSD_RGN_Insert st_rgn_list failed: %d\n", (int)ret);
			return ret;
		}

		SetPlaneRgn(st_plane, st_rgn);
		i4OsdPlaneFlipTo(st_plane, st_rgn_list);
		i4OsdPlaneEnble(st_plane, TRUE);
		_afgOSDPlaneState[prData->u4BlockID].fgEnable = TRUE;
	} else {
		st_rgn  = GetPlaneRgn(st_plane);
		OSD_RGN_Set(st_rgn, (__s32)OSD_RGN_BMP_ADDR, addr);
	}

	/*if (st_plane > OSD_PLANE_6) {
		TurnOnTve(st_plane, TRUE);
	}*/

	return TRUE;

}

#define ROUND_UP_COUNT(Count, Pow2) (((Count) + (Pow2) - 1) & (~(((LONG)(Pow2)) - 1)))

bool subtitle_osd_palette_init(unsigned int addr, OSD_DATA_T *prData)
{
	__s32 ret;
	__u32 u4PalettePa;
	__u32 dstWidth;
	__u32 dstHeight;
	__u32 posx;
	__u32 posy;
	__u32 srcWidth, srcHeight;

	srcWidth  = prData->u4Width;
	srcHeight  = prData->u4Height;

	u4PalettePa = addr + (srcWidth * srcHeight);
	u4PalettePa = ROUND_UP_COUNT(u4PalettePa, 16);

	if (1) {
		dstWidth = prData->rDestRect.right - prData->rDestRect.left;
		dstHeight = prData->rDestRect.bottom - prData->rDestRect.top;
		posx    = prData->rDestRect.left;
		posy    = prData->rDestRect.top;
		FB_PRINT(FB_LOG_LVL_DBG, "", "using private dest rect\n");
	}

	st_plane =  SUBTITLE_SURF_ID + prData->u4BlockID;

	if (prData->u4Flags & OSD_DATA_OUPPUT_PATH) {
		if (prData->u4OutputPath  == OSD_OUTPUT_PATH_REAR) {
			st_plane = SUBTITLE_SURF_REAR_ID;
		}
	}

	st_rgn_list = st_plane;
	FB_PRINT(FB_LOG_LVL_DBG, "", "subtitle_osd_palette_init %d %d %d %d\n",
		(int)posx, (int)posy, (int)dstWidth, (int)dstHeight);

	if (!_afgOSDPlaneState[prData->u4BlockID].fgEnable) {
		_afgOSDPlaneState[prData->u4BlockID].u4Plane = st_plane;
		OSD_BASE_SetOsdPosition(st_plane, posx, posy);
		OSD_SC_Scale(st_plane, FALSE, dstWidth, dstHeight, dstWidth, dstHeight);
		OSD_RGN_LIST_DetachAll(st_rgn_list);
		OSD_RGN_Create(&st_rgn, srcWidth, srcHeight, (void *)addr, OSD_CM_RGB_CLUT8
			, srcWidth, 0, 0,  dstWidth, dstHeight);
		OSD_RGN_Set(st_rgn, (__s32)OSD_RGN_PAL_PA,   u4PalettePa);

		if (prData->u4Flags & OSD_DATA_SRC_CLR_KEY) {
			OSD_RGN_Set(st_rgn, (__s32)OSD_RGN_COLOR_KEY, prData->u4SrcColorKey);
			OSD_RGN_Set(st_rgn, (__s32)OSD_RGN_COLOR_KEY_EN, TRUE);
		}

		ret = OSD_RGN_Insert(st_rgn, st_rgn_list);

		if (ret) {
			FB_PRINT(FB_LOG_LVL_DBG, "", "OSD_RGN_Insert st_rgn_list failed: %d\n", (int)ret);
			return ret;
		}

		SetPlaneRgn(st_plane, st_rgn);
		i4OsdPlaneFlipTo(st_plane, st_rgn_list);
		i4OsdPlaneEnble(st_plane, TRUE);
		_afgOSDPlaneState[prData->u4BlockID].fgEnable = TRUE;
	} else {
		st_rgn  = GetPlaneRgn(st_plane);
		OSD_RGN_Set(st_rgn, (__s32)OSD_RGN_BMP_ADDR, addr);
		OSD_RGN_Set(st_rgn, (__s32)OSD_RGN_PAL_PA, u4PalettePa);
	}

	/*if (st_plane > OSD_PLANE_6) {
		TurnOnTve(st_plane, TRUE);
	}*/

	return TRUE;
}


void subtitle_osd_off(unsigned int u4Plane)
{
	unsigned int layerId = _afgOSDPlaneState[u4Plane].u4Plane;
	unsigned int u4Rgn;

	if (_afgOSDPlaneState[u4Plane].fgEnable) {
		FB_PRINT(FB_LOG_LVL_DBG, "", "disable osd   %d.......\n", (int)layerId);
		i4OsdPlaneEnble(layerId, FALSE);
		u4Rgn  = GetPlaneRgn(layerId);

		if (u4Rgn < OSD_MAX_NUM_RGN) {
			FB_PRINT(FB_LOG_LVL_DBG, "", "rgn delete  .......\n");
			OSD_RGN_Delete(u4Rgn);
			SetPlaneRgn(layerId, INVALID_RGN);
		}

		_afgOSDPlaneState[u4Plane].fgEnable = FALSE;
	}

	/*if (st_plane > OSD_PLANE_6) {
		TurnOnTve(st_plane, FALSE);
	}*/
}
static int aColorMode[] = {
	OSD_CM_RGB565_DIRECT16,
	OSD_CM_ARGB8888_DIRECT32
};

static int _aBytesPerPixel[] = {
	2,
	4
};


static bool osd_init(unsigned int addr1, unsigned int addr2, int width, int height)
{
#ifdef FB_DEBUG
	unsigned char *pBuf = (unsigned char *)__va(addr2);

	memset(pBuf, 0xf800, width * height * 2);
	int cnt;

	for (cnt = 0; cnt < 10; cnt++) {
		FB_PRINT(FB_LOG_LVL_DBG, "", "err, %d=0x%x\n", (int)cnt, (unsigned int)*pBuf);
		pBuf++;
	}

#endif

	__s32  ret;

	i4OsdVfyCreateSemaphores();
	plane = PRIMARY_SURF_ID;
	i4OsdSetDisplayMode(plane, _u4DispMode);
	i4OsdSetDisplayMode(OSD_PLANE_7, _u4RearOutputMode);
	OSD_BASE_SetOsdPosition(plane, PRIMARY_OSD_X_OFFSET, PRIMARY_OSD_Y_OFFSET);
	OSD_SC_Scale(plane, TRUE, width, height, width, height);


	OSD_RGN_LIST_Create(&rgn_list);
	rgn_list  = plane;

	OSD_RGN_Create(&rgn, width, height, (void *)addr1, aColorMode[COLOR_DEPTH_32_BIT],
		       width * _aBytesPerPixel[COLOR_DEPTH_32_BIT], 0, 0,  width, height);
	/*   OSD_RGN_Set(rgn, OSD_RGN_MIX_SEL, OSD_BM_PLANE);*/
	OSD_RGN_LIST_DetachAll(rgn_list);
	ret = OSD_RGN_Insert(rgn, rgn_list);

	if (ret) {
		FB_PRINT(FB_LOG_LVL_DBG, "", "OSD_RGN_Insert rgn failed: %d\n", (int)ret);
		return ret;
	}

	SetPlaneRgn(rgn_list, rgn);
	i4OsdPlaneFlipTo(plane, rgn_list);
	i4OsdPlaneEnble(plane, TRUE);

	return TRUE;
}

void display_flip(struct fb_var_screeninfo *var)
{
	/*int u4Rgn;*/
	__u32 u4Addr;
	__u32   u4Rgn;
	__u32  u4Plane = PRIMARY_SURF_ID;

	u4Addr = fb_addr1 + var->yoffset * _u4LCDWidth * (aBitsPerPixel[COLOR_DEPTH_32_BIT] / 8);
	front_fb_current_addr = u4Addr;
	u4Rgn = GetPlaneRgn(u4Plane);
	OSD_RGN_Set(u4Rgn, (__s32)OSD_RGN_BMP_ADDR, u4Addr);
	if (fr_follow & FR_FOLLOW_UI) {
	    rear_osd_follow_with_front_flip(OSD_PLANE_8, u4Addr);
	}
}

void display_init(unsigned int addr1, unsigned int addr2, unsigned int width, unsigned int height)
{
	__u32 *pu4OSDReset;
	__u32 u4LCDHStart = 0, u4LCDVStart = 0;

	PMX_Init(RESET_HW_ENGINE);
	vPmxHalSetVHTotal(PMX_1, FALSE, 0, 0);
	DisplaySclInit(RESET_HW_ENGINE);

	if (RESET_HW_ENGINE) {
		if ((width == 1024) && (height == 600)) {
			u4LCDHStart = MASTER_1024_600_HSTART;
			u4LCDVStart = MASTER_1024_600_VSTART;
		} else if ((width == 1024) && (height == 768)) {
			u4LCDHStart = MASTER_1024_768_HSTART;
			u4LCDVStart = MASTER_1024_768_VSTART;
		} else {
			u4LCDHStart = MASTER_800_480_HSTART;
			u4LCDVStart = MASTER_800_480_VSTART;
		}

#if MASTER_MODE_ENABLE
		FB_PRINT(FB_LOG_LVL_DBG, "", "display_init: master mode %d\n", RESET_HW_ENGINE);
		vTconTimingInput(TRUE, u4LCDHStart, u4LCDHStart + width, u4LCDVStart, u4LCDVStart + height);
		vPmxHalSetMasterMode(TRUE);
#else
		FB_PRINT(FB_LOG_LVL_DBG, "", "display_init: slave mode %d\n", RESET_HW_ENGINE);
		vTconTimingInput(FALSE, u4LCDHStart, u4LCDHStart + width, u4LCDVStart, u4LCDVStart + height);
		vPmxHalSetMasterMode(FALSE);
#endif
	}

	/*vLoadCMMSetting();*/
	OSD_Init(RESET_HW_ENGINE);
	fb_addr1 = addr1;
	fb_addr2 = addr2;
	osd_init(addr1, addr2, width, height);

	vVdpHalInit(VDP_1, RESET_HW_ENGINE);
	vVdpHalInit(VDP_2, TRUE);
	vTveHalInit();

	/*vTCONReset(TRUE, TRUE, RESET_HW_ENGINE);*/

	if (RESET_HW_ENGINE) {
		*((__u32 *)0xFD000504)	=  0x0;
		*((__u32 *)0xFD000518)	=  0xFFFFFFFF;
		pu4OSDReset = (__u32 *)(osdf_reg + 0x04);
		*pu4OSDReset |= 0xff0;
		*pu4OSDReset &= ~0xff0;
		msleep(300);
		vSetBacklightIntensity(76);
	}

	/*i4ImgResz_Drv_Init();*/
}

void display_uninit(void)
{
	i4OSD_Uninit();
	i4OsdVfyDeleteSemaphores();
	/*i4ImgResz_Drv_Uninit();*/
}
EXPORT_SYMBOL(display_uninit);

void set_colorkey(__u32 type, __u32 colorkey, __u32 enable)
{
	__u32 u4Rgn, color;
	__u32 u4Plane = PRIMARY_SURF_ID;

	u4Rgn  = GetPlaneRgn(u4Plane);

	if (COLOR_DEPTH_32_BIT) {
		color = colorkey;
	} else {
		color = (colorkey & 0x1f) | ((colorkey & 0x7e0) << 3) | ((colorkey & 0xf800) << 5);
	}

	if (enable) {
		if (type == SRC_COLOR_KEY) {
			OSD_RGN_Set(u4Rgn, (__s32)OSD_RGN_COLOR_KEY, color);
			OSD_RGN_Set(u4Rgn, (__s32)OSD_RGN_COLOR_KEY_EN, TRUE);
		} else {
			OSD_PLA_SetDestColorKey(TRUE, color);
			vPmxHalSetPlaneDstColorKey(0, 1);
		}
	} else {
		if (type == SRC_COLOR_KEY) {
			OSD_RGN_Set(u4Rgn, (__s32)OSD_RGN_COLOR_KEY_EN, FALSE);
		} else {
			vPmxHalSetPlaneDstColorKey(0, 0);
		}
	}
}
EXPORT_SYMBOL(set_colorkey);

u32 front_fb_current_addr = 0;
u32 create_rear_osd_follow_with_front(u32 plane, u32 src_width, u32 src_height, u32 addr)
{
	u32 rgn_list, rgn;
	FB_CONFIG_T rFBConfig;
	u32 output_height, output_width;
	u32 ret = 0;

	vGetFBConfigFromShareMemory(&rFBConfig);
	output_height  = (rFBConfig.u4VideoMode == RES_480P) ? 480 : 576;
	output_width   = 720;

	//i4OsdSetDisplayMode(OSD_PLANE_7, VIDEO_MODE);
	OSD_BASE_SetOsdPosition(plane, g_rFBConfig.rCVBSMargin.u4TopLeftXMargin, g_rFBConfig.rCVBSMargin.u4TopLeftYMargin);
	OSD_SC_Scale(plane, FALSE, output_width, output_height, output_width, output_height);
	OSD_RGN_LIST_Create(&rgn_list);
	rgn_list  = plane;

	OSD_RGN_Create(&rgn, src_width, src_height, (void *)addr, OSD_CM_ARGB8888_DIRECT32, (src_width * 4), 0, 0,
		output_width - g_rFBConfig.rCVBSMargin.u4TopLeftXMargin - g_rFBConfig.rCVBSMargin.u4BottomRightXMargin,
		output_height - g_rFBConfig.rCVBSMargin.u4TopLeftYMargin - g_rFBConfig.rCVBSMargin.u4BottomRightYMargin);
	//OSD_RGN_Set(rgn, OSD_RGN_MIX_SEL, OSD_BM_PLANE);
	OSD_RGN_LIST_DetachAll(rgn_list);
	ret = OSD_RGN_Insert(rgn, rgn_list);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "%s OSD_RGN_Insert rgn %d failed: %d\n", __func__, rgn, (int)ret);
		return ret;
	}
	SetPlaneRgn(rgn_list, rgn);
        FB_PRINT(FB_LOG_LVL_INFO, "", "create follow osd rgn %d  \n", rgn);
	i4OsdPlaneFlipTo(plane, rgn_list);
	i4OsdPlaneEnble(plane, TRUE);

	FB_PRINT(FB_LOG_LVL_INFO, "", "%s rgn %d src w %d h %d output w %d %d ret %d\n", __func__, rgn, src_width
		, src_height, output_width, output_height, ret);

	return (ret);
}

void rear_osd_follow_with_front_flip(u32 u4Plane, u32 u4Addr)
{
	u32 u4Rgn;

	u4Rgn = GetPlaneRgn(u4Plane);
	OSD_RGN_Set(u4Rgn, (u32)OSD_RGN_BMP_ADDR, u4Addr);
}

