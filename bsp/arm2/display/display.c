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
#include <generated/atc_project.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "x_bim_83xx.h"
#include "x_types.h"
#include "drv_pmx.h"
#include "vdp_hal.h"
#include "tcon.h"
#include "scl_if.h"
#include "scl_hal.h"
#include "drv_osd_if.h"
#include "osd_if_pdd.h"
#include "backcar_cfg.h"
#include "display.h"
#include "mrf.h"
#include "osd_inc.h"
#include "vdp_mdd.h"
#include "cp.h"
#include "log.h"
#include "pwm_hal.h"
#include "ac83xx_gpio_pinmux.h"
#include "ac83xx_pinmux_table.h"
#include "pinmux.h"
#include "gpio.h"
#include "reserved_memory.h"
#include "metazone_inter.h"

#define  BACKCAR_UI_OSD_PLANE    OSD_PLANE_3

__u32 G_DISPLAY_MODE = RES_480P_800;
unsigned int G_OUTPUT_WIDTH = 800;
unsigned int G_OUTPUT_HEIGHT = 480;

#ifdef CONFIG_ATC_OS_linux
__u32 G_LOGO_BPP = 32;  //for 32/16 bpp logo config
#endif
/*HVTotal*/
#define PANEL_DCLK   512 /* lvds 51.2Mhz *10 for 1024x600 panel*/

extern BOOL fgIRTDone;
extern BOOL fgVDPNeedInit = TRUE;
static __u32 HTOTAL; /* ((__u32)(858 * G_OUTPUT_HEIGHT * PANEL_DCLK /10 /600 /27))*/
static __u32 VTOTAL; /* ((__u32)(525 * G_OUTPUT_HEIGHT /480))*/

/*MT3360 generate these fixed params for 800*480*/
#define PANEL_HS    0x86             /*Horizontal start,134*/
#define PANEL_HE    (PANEL_HS + 800) /*Horizontal end,934 =134+800*/
#define PANEL_VS    0x2D             /*Vertical Start ,45*/
#define PANEL_VE    (PANEL_VS + 480) /*Vertical end ,525 = 45+480*/

/*Get these params from panel Spec,e.g HSD070IDW1*/
#define PANEL_HBP   40   /*Horizontal Back Porch*/
#define PANEL_HPW   48   /*Horizontal Pulse Width*/
#define PANEL_VBP   29   /*Vertical Back Porch*/
#define PANEL_VPW   3    /*Vertical Pulse Width*/

#define ENABLE_LOAD_MRF    0

extern __u32 gu4CurTVMode;

__u32 fb_log_lvl = FB_LOG_LVL_HAL;

unsigned int osdf_reg = IO_BASE + 0x20000;
unsigned int osd1_reg = IO_BASE + 0x20100;
unsigned int osd2_reg = IO_BASE + 0x20200;
unsigned int osd3_reg = IO_BASE + 0x20300;
unsigned int osd4_reg = IO_BASE + 0x20a00;
unsigned int osd5_reg = IO_BASE + 0x20b00;
unsigned int osdr_reg = IO_BASE + 0xa3000;
unsigned int osdr1_reg = IO_BASE + 0xa3100;
unsigned int osdr2_reg = IO_BASE + 0xa3200;
unsigned int osdr3_reg = IO_BASE + 0xa3300;
unsigned int fmtf_reg = IO_BASE + 0x42000;
unsigned int vdof_reg = IO_BASE + 0x42100;
unsigned int fmtr_reg = IO_BASE + 0x43000;
unsigned int vdor_reg = IO_BASE + 0x43100;
unsigned int scl_reg = IO_BASE + 0xA4500;
unsigned int sclf_reg = IO_BASE + 0xA4600;
unsigned int mix_reg = IO_BASE + 0x1F000;
unsigned int tlcp_reg = IO_BASE + 0xA4400;
unsigned int togc_reg = IO_BASE + 0xA4700;
unsigned int tcon_reg = IO_BASE + 0xA4800;
unsigned int ddds_reg = IO_BASE + 0x52C00;
unsigned int lvds_reg = IO_BASE + 0xA5000;
unsigned int lvdsa_reg = IO_BASE + 0xA6000;
unsigned int tve_reg = IO_BASE + 0x2000;
unsigned int tvebk1_reg = IO_BASE + 0x2100;
unsigned int tvebk2_reg = IO_BASE + 0x2180;
unsigned int fbm_base = 0;
unsigned int fbm_size = 0;
unsigned int logo_base = 0;
unsigned int logo_size = 0;
void * fbm_va = NULL;
unsigned int vm_base = 0;
unsigned int vm_size = 0;
void * vm_va = NULL;
phys_addr_t wchReservebase = 0;
static int g_VbaInitFlag = 0;
/***************************/
/*DO NOT enable FPD TCON and HVSync,DE adjust  if Unneccessary,*/
/*Please set FALSE default*/
/***************************/
FB_CONFIG_T  g_rFBConfig = {
	REAR_OUTPUT_MODE,
	{
		0,
		0,
		0,
		0,
	},
	
	32,
	{0, 800,  480, 300, 928,  525, {0x59,0x15,0x15}, {0x65,0x16}, {0x10,0x17}, {0x10,0x17}, {0x66,0x2c}, {0x6a,0x16}},
	{0x68,  0x2c,  {-1, -1,  6,  0, -1,  1}, {-1, -1, -1,  0, -1, -1}}
	
};

static LCD_TCON_ARGS_T g_rTconTiming = {
	PANEL_DCLK,     /*DCLK setting*/
	0,         /*HTotal*/
	0,         /*VTotal*/
	TRUE,          /*FPD  tcon enable,set FALSE default*/
	{
		/*hsync*/
		FALSE,                           /*adjust enable,Do not change if Unneccessary*/
		FALSE,                           /*polarity invert,set FALSE default*/
		PANEL_HS - PANEL_HBP - PANEL_HPW,/*T2HS = T2HE - HPW*/
		PANEL_HS - PANEL_HBP,            /*T2HE = T8HS - HBP*/
		0x0,                             /*T2VS = 0*/
		0x0                              /*T2VE = 0*/
	},
	{
		/*vsync*/
		FALSE,                           /*adjust enable,Do not change if Unneccessary*/
		FALSE,                           /*polarity invert,set FALSE default*/
		PANEL_HS - PANEL_HBP - PANEL_HPW,/*T1HS =T2HS*/
		PANEL_HS - PANEL_HBP - PANEL_HPW,/*T1HE =T2HS*/
		PANEL_VS - PANEL_VBP - PANEL_VPW,/*T1VS = T1VE - VPW*/
		PANEL_VS - PANEL_VBP             /*T1VE = T8VS - VBP*/
	},
	{
		/*DE*/
		FALSE,     /*adjust enable,Do not change if Unneccessary*/
		FALSE,     /*not use*/
		PANEL_HS,
		PANEL_HE,
		PANEL_VS,
		PANEL_VE
	}
};

static __u32 g_u4Rgn = -1;/*region for backcar UI*/
static __u32 g_u4LogoRgn = -1;/*region for logo*/
static __u32 _u4VdpOutWidth, _u4VdpOutHeight;
static struct OVERLAY_PARAM rVdpParam;
static struct OVERLAY_PARAM rVBAVdpParam;

__u32 g_vbaExitFlag = 0;

//config arm2 backcar rotate degree, one and only one of them must be 1 

#ifdef CONFIG_ATC_OS_android
#ifndef CONFIG_ATC_PLATFORM_ac823x

unsigned int screen_rotate =  0;

//unsigned int screen_rotate =  VDP_ROTATE_90;

//unsigned int screen_rotate = VDP_ROTATE_270;

#else
unsigned int screen_rotate =  0;


#endif

#else
unsigned int screen_rotate =  0;
unsigned int g_rotate_value =  0;

#endif
extern void memcpy(void *dest, void *src, unsigned int count);
static void vSetFBConfigToShareMemory(FB_CONFIG_T *prConfig);

#ifdef CONFIG_ATC_PLATFORM_ac83xx
#ifdef CONFIG_ATC_OS_android

#define DISABLE_ANIMATION_INIT_OSD2

#endif
#endif


#define DISPLAY_QUEUE_ITEMS_COUNT (1)
static QueueHandle_t xDisplayQueue = NULL;
//awtk
extern void LTDC_IRQHandler(void);
bool HideVBAOverlay();

//#define  BOOT_ANIMATION_OSD_PLANE    OSD_PLANE_1
bool BootAnimationOsdInit(__u32 u4DataPA,bool fgOsdInit)
{
	RECT    rRect = {0}, SrcRect = {0}, DstRect = {0};
	/*__u32 *pu4Header1 =  (__u32 *) 0xfd020304;*/
	__u32  u4RgnList;
	__u32  u4Width, u4Height;
	#ifdef CONFIG_ATC_OS_linux
	__u32  u4Plane = BOOT_ANIMATION_OSD_PLANE;
	#else
	__u32  u4Plane = PRIMARY_SURF_ID;
	#endif

	if(fgOsdInit) 
	{
#ifndef DISABLE_ANIMATION_INIT_OSD2

	rRect.left = 0;
	rRect.top = 0;
	rRect.right = _u4LCDWidth - rRect.left;
	rRect.bottom = _u4LCDHeight - rRect.top;

	OSD_BASE_SetOsdPosition(u4Plane, PRIMARY_OSD_X_OFFSET, PRIMARY_OSD_Y_OFFSET);

	OSD_SC_Scale(u4Plane, TRUE, _u4LCDWidth, _u4LCDHeight, G_OUTPUT_WIDTH, G_OUTPUT_HEIGHT);

	OSD_RGN_LIST_Create(&u4RgnList);
	u4RgnList  = u4Plane;
    if(G_LOGO_BPP == 32)
		OSD_RGN_Create(&g_u4LogoRgn, _u4LCDWidth, _u4LCDHeight, (void *)u4DataPA,
		       OSD_CM_ARGB8888_DIRECT32, (_u4LCDWidth * 4), 0, 0, _u4LCDWidth, _u4LCDHeight);
	else if(G_LOGO_BPP == 16)
		OSD_RGN_Create(&g_u4LogoRgn,_u4LCDWidth, _u4LCDHeight, (void *)u4DataPA,
				OSD_CM_RGB565_DIRECT16, (_u4LCDWidth * 2), 0, 0,  _u4LCDWidth, _u4LCDHeight);

	OSD_RGN_Create(&g_u4LogoRgn,_u4LCDWidth, _u4LCDHeight, (void *)u4DataPA,
				OSD_CM_RGB565_DIRECT16, (_u4LCDWidth * 2), 0, 0,  _u4LCDWidth, _u4LCDHeight);
#if 0
	OSD_RGN_Set(u4Rgn, (__s32)OSD_RGN_COLOR_KEY, 31);
	OSD_RGN_Set(u4Rgn, (__s32)OSD_RGN_COLOR_KEY_EN, TRUE);
#endif

	OSD_RGN_LIST_DetachAll(u4RgnList);
	OSD_RGN_Insert(g_u4LogoRgn, u4RgnList);
	SetPlaneRgn(u4RgnList, g_u4LogoRgn);
	i4OsdPlaneFlipTo(u4Plane, u4RgnList);
#endif
	}else {
	i4OsdPlaneEnble(u4Plane, TRUE);

#ifdef CONFIG_ATC_OS_linux
	vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_3);
	//vSclHalIsr(0, 0);
#endif	
	}
	return  TRUE;
}

bool AWtkOsdInit(__u32 u4DataPA, __u32 u4SrcWidth, __u32 u4SrcHeight, __u32 u4SrcX, __u32 u4SrcY, __u32 u4DstWidth, __u32 u4DstHeight, __u32 u4BitCount, __u8 carplay_flag)
{
	__u32  u4RgnList;
	__u32  u4Width, u4Height;
	__u32  u4Plane = OSD_PLANE_3;
	__u32  u4AwtkRgn = -1;/*region for awtk*/

	Printf("AWtkOsdInit in\n");
	if (carplay_flag) {
		/*v_disable_bim_irq(45);
		v_disable_bim_irq(VECTOR_VSYNC);
		v_disable_bim_irq(VECTOR_PANEL_SCALER);
		Printf("v_disable_bim_irq ok\n");*/
	} else {
		v_enable_bim_irq(VECTOR_VSYNC);
		v_enable_bim_irq(VECTOR_PANEL_SCALER);
		v_disable_bim_irq(VECTOR_TOCORISC);
		Printf("v_enable_bim_irq ok\n");
	}

	OSD_BASE_SetOsdPosition(u4Plane, u4SrcX, u4SrcY);

	OSD_SC_Scale(u4Plane, TRUE, u4SrcWidth, u4SrcHeight, u4DstWidth, u4DstHeight);

	OSD_RGN_LIST_Create(&u4RgnList);
	u4RgnList  = u4Plane;

	OSD_RGN_Create(&u4AwtkRgn,u4SrcWidth, u4SrcHeight, (void *)u4DataPA,
				u4BitCount, (u4SrcWidth * 2), u4SrcX, u4SrcY,  u4SrcWidth, u4SrcHeight);

#if 0
	OSD_RGN_Set(u4Rgn, (__s32)OSD_RGN_COLOR_KEY, 31);
	OSD_RGN_Set(u4Rgn, (__s32)OSD_RGN_COLOR_KEY_EN, TRUE);
#endif

	OSD_RGN_LIST_DetachAll(u4RgnList);
	OSD_RGN_Insert(u4AwtkRgn, u4RgnList);
	SetPlaneRgn(u4RgnList, u4AwtkRgn);
	i4OsdPlaneFlipTo(u4Plane, u4RgnList);

	//i4OsdPlaneEnble(u4Plane, TRUE);

	//vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_5);
	Printf("AWtkOsdInit out\n");
	return  TRUE;
}

void AwtkOsdEnable(int enable)
{
	if (enable) {
		i4OsdPlaneEnble(OSD_PLANE_3, TRUE);
		vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_5);
		HideVBAOverlay();
	} else {
		i4OsdPlaneEnble(OSD_PLANE_3, FALSE);
		vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_5);
	}
}

bool HideBootAnimationOsd()
{
	__u32  u4Plane = BOOT_ANIMATION_OSD_PLANE;

	i4OsdPlaneEnble(u4Plane, FALSE);
	i4OsdPlaneEnble(OSD_PLANE_3, FALSE);
#ifdef CONFIG_ATC_OS_linux 
	vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_4);		
	//vSclHalIsr(0, 0);
#else
	vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_4); 	
	vSclHalIsr(0, 0);
#endif
	vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_3);
	vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_5);

	g_vbaExitFlag = 1;
}
void  DisableDisplayOverlay(void)
{
#ifdef CONFIG_ATC_OS_linux
	vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_3);	/*disable boot animation osd */
	if(fgDualHALGetUpgradeMode() > 0) {
		i4OsdPlaneEnble(PRIMARY_SURF_ID, FALSE);
		vSclHalIsr(0, 0);
	}
#else
	vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_4);	/* disable android PRIMARY_SURF_ID layer for arm2 backcar*/
	vSclHalIsr(0, 0);
#endif

}
void  EnableDisplayOverlay(void)
{
#ifdef CONFIG_ATC_OS_linux
	vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_3);  /*enable boot animation osd */
	if(fgDualHALGetUpgradeMode() > 0) {
		i4OsdPlaneEnble(PRIMARY_SURF_ID, TRUE);
		vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_4);  /* enable PRIMARY_SURF_ID layer after arm2 backcar leave*/
		vSclHalIsr(0, 0);
	}
#else
	vPmxHalMixPlane(PMX_1, PRIMARY_SURF_PLANE);  /* enable PRIMARY_SURF_ID layer after arm2 backcar leave*/
	vSclHalIsr(0, 0);
#endif

}
VOID DisplaySclInit(bool fgHwReset)
{
	SCL_Init(fgHwReset);

	if (fgHwReset) {
		vSclHalSetHVTotal(1, HTOTAL, VTOTAL);
		SCL_Config();
	}
}
/*
static void VcpInit(void)
{
    unsigned int regval = 0;

    HAL_WRITE32(0xfd01f080, 0x1);

    regval = HAL_READ32(0xfd042630);
    regval &= 0xffffc0ff;
    regval |= (0x20 << 8);
    HAL_WRITE32(0xfd042630 ,regval);

    regval = HAL_READ32(0xfd042608);
    regval &= 0xffffe00f;
    regval |= (0x80 << 4);
    HAL_WRITE32(0xfd042608 ,regval);

    regval = HAL_READ32(0xfd042624);
    regval &= 0xe00fe00f;
    regval |= ((0x80 << 20) | (0x5b << 4));
    HAL_WRITE32(0xfd042624 ,regval);

    regval = HAL_READ32(0xfd04260c);
    regval &= 0xff00ff00;
    regval |= ((0x80 << 16) | (0x40 << 0));
    HAL_WRITE32(0xfd04260c ,regval);

    regval = HAL_READ32(0xfd04262c);
    regval &= 0xffffff00;
    regval |= (0x80 << 0);
    HAL_WRITE32(0xfd04262c ,regval);
}
*/
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
		rVdpParam.u4Flags = VDP_UPDATE_OVERLAY | VDP_TOP_LEVEL | VDP_SCANLINE_MODE | VDP_ENABLE_DEINT | screen_rotate;
	} else {
		rVdpParam.u4Flags = VDP_UPDATE_OVERLAY | VDP_TOP_LEVEL | VDP_SCANLINE_MODE | screen_rotate;
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
#ifdef CONFIG_ATC_OS_linux
		rVdpParam.fgTopFiledFirst = 1;
#else
		rVdpParam.fgTopFiledFirst = 0;
#endif
	}

	/*Printf("Flip rVdpParam.fgTopFiledFirst %d, u4SrcWidth %d, u4SrcHeight %d\r\n", rVdpParam.fgTopFiledFirst
		, rVdpParam.u4SrcWidth, rVdpParam.u4SrcHeight);*/
	if (deint) {
		rVdpParam.u4Flags = VDP_TOP_LEVEL | VDP_SCANLINE_MODE | VDP_ENABLE_DEINT | screen_rotate;
	} else {
		rVdpParam.u4Flags = VDP_TOP_LEVEL | VDP_SCANLINE_MODE | screen_rotate;
	}

	if (VDP_IOControl(VIDIOC_QBUF, &rVdpParam, NULL)) {
		return FALSE;
	}
	fgIRTDone = FALSE;

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
        #if USE_16BITS_TRACK_IMAGE
		OSD_RGN_Create(&g_u4Rgn, _u4LCDWidth, _u4LCDHeight, (void *)u4DataPA,
			OSD_CM_RGB565_DIRECT16, (_u4LCDWidth * 2), 0, 0, _u4LCDWidth, _u4LCDHeight);
        #else
		OSD_RGN_Create(&g_u4Rgn, _u4LCDWidth, _u4LCDHeight, (void *)u4DataPA,
			OSD_CM_ARGB8888_DIRECT32, (_u4LCDWidth * 4), 0, 0, _u4LCDWidth, _u4LCDHeight);
        #endif
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


bool VBAOverlayInit(__u32 u4VDPDstYPA, __u32 u4VDPDstCPA,
	__u32 width, __u32 height, RECT   *SrcRect)
{
	RECT   DstRect = {0};
    __u32  u4Plane = OSD_PLANE_3;

	g_VbaInitFlag = 1;
	DstRect.left = 0;
	DstRect.top = 0;
	DstRect.right = _u4LCDWidth;
	DstRect.bottom = _u4LCDHeight;

	memset(&rVBAVdpParam, 0, sizeof(rVBAVdpParam));
	rVBAVdpParam.u4Idx = VDP_1;
	rVBAVdpParam.u4SrcWidth = width;
	rVBAVdpParam.u4SrcHeight = height;
	memcpy(&rVBAVdpParam.rSrcRect, SrcRect, sizeof(RECT));
	memcpy(&rVBAVdpParam.rDstRect, &DstRect, sizeof(RECT));
	rVBAVdpParam.u4PhysicalAddressY = u4VDPDstYPA;
	rVBAVdpParam.u4PhysicalAddressC = u4VDPDstCPA;
	rVBAVdpParam.u4Duration = VSYNC_PER_FRAME;
	rVBAVdpParam.u4Status = 0;
	rVBAVdpParam.device_name = BACKCAR;
	rVBAVdpParam.fgProgSrc = FALSE;
	rVBAVdpParam.fgTopFiledFirst = FALSE;

	i4OsdPlaneEnble(u4Plane, FALSE);
	vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_5);

	rVBAVdpParam.u4Flags = VDP_UPDATE_OVERLAY | VDP_TOP_LEVEL | screen_rotate;

    if (!screen_rotate) {
        fgIRTDone = TRUE;
    }

	if (VDP_IOControl(VIDIOC_QBUF, &rVBAVdpParam, NULL)) {
		return FALSE;
	}

	return  TRUE;
}

bool HideVBAOverlay()
{
	__u32 timeout = 0;
	__u32  u4Plane = BOOT_ANIMATION_OSD_PLANE;
	if (!g_VbaInitFlag)
		return TRUE;

	if (GetCarplayStatusUpdateFlag()) {
		//when using carplay, need to wait until carplay is activated before opening the awtk layer.
		while (1 != GetCarplayStatusUpdateFlag()) {
			vTaskDelay(pdMS_TO_TICKS(1));
			timeout++;
			if (timeout >= 1000) {
				Printf("wait carpaly start falg failed, GetCarplayStatusUpdateFlag: %d\n", GetCarplayStatusUpdateFlag());
				goto exit;
			}
		}
		if (GetCarplayStartFlag() && !GetAbnormalFlag()) {
			Printf("loki GetCarplayStartFlag is 1\n");
			i4OsdPlaneEnble(OSD_PLANE_3, TRUE);
			vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_5);

			vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_4);
		} else if (!GetCarplayStartFlag() && !GetAbnormalFlag() ) {
			Printf("loki GetCarplayStartFlag is 0\n");
			i4OsdPlaneEnble(OSD_PLANE_3, FALSE);
			vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_5);

			i4OsdPlaneEnble(OSD_PLANE_2, TRUE);
			vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_4);
		}
	} else {
		i4OsdPlaneEnble(OSD_PLANE_2, TRUE);
		vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_4);
	}

	g_VbaInitFlag = 0;
exit:
	if (VDP_IOControl(VIDIOC_STREAMOFF, &rVBAVdpParam, NULL)) {
		return FALSE;
	}
	g_vbaExitFlag = 1;

	return	TRUE;
}

/*
fgHideOverlay
true : close backcar UI osd and tream off VDP
false: only tream off VDP
*/
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

		/*add end*/
		/*vTconSetContrast(25);*/
		/*vTconSetSaturation(50);*/
		
	}

	/* this IOCTL will show primary surface ,when no signal we want black screen not primary surface*/
	if (VDP_IOControl(VIDIOC_STREAMOFF, &rVdpParam, NULL)) {
		return FALSE;
	}
	fgVDPNeedInit = TRUE;

	/*close vcp after streamoff vdp*/
	/*vCPOff(1);*/
	/*open the later code will fix primary surface shown issue when no signal*/
	/*
	if (!fgHideOverlay) {
	    //Printf(" HideBackCarOverlay when fgHideOverlay is False we dont show primary\r\n");
	    //add by 68028 for when backcar enter disable osd2
	    vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_4);  // disable android PRIMARY_SURF_ID layer for arm2 backcar
	}
	*/
	/*the end*/
	/*vPmxHalMixPlane(0, PRIMARY_SURF_PLANE); //for enable primary osd*/

#ifdef CONFIG_ATC_OS_linux
	/*if(fgDualHALGetUpgradeMode() > 0) {
		vPmxHalMixPlane(0, PRIMARY_SURF_PLANE);
		//Sleep(20); implementation of sleep() depens on timer irqs. calling this function if tvd signal lost irq service, will blocked here.
	}*/
	//vPmxHalMainIsr(0, 0);
	//vSclHalIsr(0, 0);
#else
	vPmxHalMainIsr(0, 0);
	vSclHalIsr(0, 0);
#endif

	return TRUE;
}

static BYTE g_pbPanelGamma[64] = {
	0, 1, 3, 5, 7, 9, 11, 13,
	15, 17, 20, 23, 25, 28, 31, 34,
	37, 40, 43, 47, 51, 56, 61, 66,
	71, 76, 82, 87, 92, 97, 102, 107,
	112, 118, 123, 128, 133, 138, 143, 148,
	153, 158, 163, 167, 172, 177, 182, 186,
	191, 195, 200, 204, 208, 212, 216, 220,
	224, 228, 232, 236, 240, 244, 248, 252
};

void vSetBacklightIntensity(__u32 u4Value)
{
	GPIO_MultiFun_Set(PIN_125_GPIO125, PINMUX_LEVEL_GPIO_END_FLAG);
	gpio_direction_output(PIN_125_GPIO125, 1);
	Printf("PIN_125_GPIO125 1\r\n");
	vSetPWM(u4Value);
}

static void DisplayInit(__u32 u4DataPA, __s32 i4Width, __s32 i4Height, __u32 u4BitCount, __u32 u4LcdType)
{
	RECT    rRect = {0}, SrcRect = {0},
	rDstRect = {
		PRIMARY_OSD_X_OFFSET,
		PRIMARY_OSD_Y_OFFSET,
		G_OUTPUT_WIDTH - PRIMARY_OSD_RIGHT_CORDER_X_MARGIN,
		G_OUTPUT_HEIGHT - PRIMARY_OSD_RIGHT_CORDER_Y_MARGIN
	};
	__u32 u4RgnList, u4Rgn, u4Plane;
	__s32 i;
	__u32 u4Cnt = 0, u4PanelID = 0;//get ID from customer,for 1024*600 solution ID 1 is TM panel
	__u32 u4LCDHStart = 0, u4LCDVStart = 0;
	__u32 u4Idx;

	__u32 regvalue;

	for(u4Cnt = 0; u4Cnt < sizeof(g_rLCDPanelSetting) / sizeof(g_rLCDPanelSetting[0]); u4Cnt++)
	{
		if((g_rLCDPanelSetting[u4Cnt].u4PanelWidth == i4Width)
			&& (g_rLCDPanelSetting[u4Cnt].u4PanelHeight == i4Height))
		{
			if((g_rLCDPanelSetting[u4Cnt+u4PanelID].u4PanelIDNum == u4PanelID)
				&&(g_rLCDPanelSetting[u4Cnt+u4PanelID].u4PanelWidth == i4Width)
				&&(g_rLCDPanelSetting[u4Cnt+u4PanelID].u4PanelHeight == i4Height))
			{
				u4Cnt = u4Cnt + u4PanelID;
			}
			else
			{
				Printf("[arm2]u4PanelID(%d) is incompatite with W*H!\r\n", u4PanelID);
			}
			g_rTconTiming.u4Clock = g_rLCDPanelSetting[u4Cnt].u4TypeClock;
			u4LCDHStart = g_rLCDPanelSetting[u4Cnt].rTconOffset.u4OffsetX;
			u4LCDVStart = g_rLCDPanelSetting[u4Cnt].rTconOffset.u4OffsetY;
			break;
		}
	}
	if(u4Cnt >= sizeof(g_rLCDPanelSetting) / sizeof(g_rLCDPanelSetting[0]))
	{
		//not suport this (i4Width*i4Height) solution now, need add setting @g_rLCDPanelSetting
		Printf("[@arm2][error]not suport this (i4Width*i4Height) solution now \r\n");
		g_rTconTiming.u4Clock = ((i4Width + 60)/10)*((i4Height + 50)/10)*6/100;
		u4LCDHStart = 104;
		u4LCDVStart = 15;
		u4Cnt = 4;
	}
	Printf("[@arm2]panel support u4Cnt,u4PanelID,u4Clock=%d,%d,%d;u4LCDH&VStart=0x%x,0x%x\r\n",
		u4Cnt,u4PanelID, g_rTconTiming.u4Clock,u4LCDHStart,u4LCDVStart);

	if((i4Width == 800)&&(i4Height == 1280))
	{
		HTOTAL = 930;
		VTOTAL = 1356;
	}
    else if((i4Width == 1920)&&(i4Height == 720))
	{
		 //formular from DE
		HTOTAL = (float)864 * 576 * g_rTconTiming.u4Clock /10 /G_OUTPUT_HEIGHT /27; //2048 for 80MHZ
		VTOTAL = (float)625 * G_OUTPUT_HEIGHT /576; //781 for 80MHZ
	}
	else {
		 //formular from DE
		HTOTAL = (float)858 * 480 * g_rTconTiming.u4Clock /10 /G_OUTPUT_HEIGHT /27;
		VTOTAL = (float)525 * G_OUTPUT_HEIGHT /480;
	}
	g_rTconTiming.u4HTotal = HTOTAL;
	g_rTconTiming.u4VTotal = VTOTAL;
	Printf("g_rTconTiming [i4Width,i4Height]=[%d,%d];HTotal:%d, VTotal:%d \r\n",
		i4Width,i4Height,HTOTAL, VTOTAL);
	_u4LCDWidth = i4Width;
	_u4LCDHeight = i4Height;
	_u4DispMode = G_DISPLAY_MODE =6;//no delete
	_u4LCDType = u4LcdType;/*transfor lcd type from eboot*/
	_u4RearOutputMode = g_rFBConfig.u4VideoMode;
	g_rPanelSetting = g_rLCDPanelSetting[u4Cnt];
	g_rFBConfig.rFBPanelSetting = g_rPanelSetting;
	g_rVdoWindowSetting = g_rFBConfig.rVdoWindowSetting;
	Printf("[@arm2] G_DISPLAY_MODE=%d,g_rPanelSetting clk:%d, tcon offsetHV:0x%x,0x%x \r\n",
		G_DISPLAY_MODE, g_rPanelSetting.u4TypeClock, g_rPanelSetting.rTconOffset.u4OffsetX,
		g_rPanelSetting.rTconOffset.u4OffsetY);
	Printf("g_rTconTiming [_u4LCDWidth,_u4LCDWidth]=[%d,%d]; \r\n",
		_u4LCDWidth,_u4LCDHeight);

	vSetFBConfigToShareMemory(&g_rFBConfig);/*very important    can not removed*/

	rRect.left = 0;
	rRect.top = 0;
	rRect.right = i4Width - rRect.left;
	rRect.bottom = i4Height - rRect.top;

	if (LCD_TYPE_LVDS == u4LcdType) {
		vSetLVDSOutputMode(LVDS_OUTPUT_MODE);
	}
	/*if((i4Width == 800)&&(i4Height == 480)) {
		GPIO_MultiFun_Set(PIN_125_GPIO125, PINMUX_LEVEL_GPIO_END_FLAG);
		gpio_direction_output(PIN_125_GPIO125, 1);
	}*/

	/*Demo board panel VGH and VGL power on, or change HW R352 always on*/
	/*GPIO_MultiFun_Set(PIN_125_GPIO125, PINMUX_LEVEL_GPIO_END_FLAG);
	gpio_direction_output(PIN_125_GPIO125, 0);*/
	PMX_Init(TRUE);
	/*vPmxHalLayerBgEn(0x4060E0); //for debug*/
	vPmxHalSetVHTotal(PMX_1, FALSE, HTOTAL, VTOTAL);
	vTconSetTiming(u4LcdType, g_rTconTiming);
	DisplaySclInit(TRUE);


#if MASTER_MODE_ENABLE
	Printf("[DDI]display_init: master mode\n");
	vTconTimingInput(TRUE, u4LCDHStart, u4LCDHStart + _u4LCDWidth, u4LCDVStart, u4LCDVStart + _u4LCDHeight);
	vPmxHalSetMasterMode(TRUE);
#else
	Printf("[DDI]display_init: slave mode\n");
	vTconTimingInput(FALSE, u4LCDHStart, u4LCDHStart + _u4LCDWidth, u4LCDVStart, u4LCDVStart + _u4LCDHeight);
	vPmxHalSetMasterMode(FALSE);
#endif
#ifndef VCP_FOR_ANDROID
	VcpInit(0);
#endif

	vVdpHalInit(VDP_1, TRUE);
	vPmxHalAuxIsr(0, 0);
#ifdef CONFIG_ATC_OS_linux 
	//vPmxHalMainIsr(0, 0);
	//vSclHalIsr(0, 0);
#else
	vPmxHalMainIsr(0, 0);
	vSclHalIsr(0, 0);
#endif

	OSD_Init(TRUE);

#ifdef DISABLE_ANIMATION_INIT_OSD2
	u4Plane = PRIMARY_SURF_ID;
	i4OsdSetDisplayMode(u4Plane, G_DISPLAY_MODE);
	OSD_BASE_SetOsdPosition(u4Plane, PRIMARY_OSD_X_OFFSET, PRIMARY_OSD_Y_OFFSET);
	OSD_SC_Scale(u4Plane, TRUE, i4Width, i4Height
		, G_OUTPUT_WIDTH - PRIMARY_OSD_X_OFFSET - PRIMARY_OSD_RIGHT_CORDER_X_MARGIN
		, G_OUTPUT_HEIGHT - PRIMARY_OSD_Y_OFFSET - PRIMARY_OSD_RIGHT_CORDER_Y_MARGIN);
	OSD_RGN_LIST_Create(&u4RgnList);
	u4RgnList  = u4Plane;

	if (16 == u4BitCount) {
		OSD_RGN_Create(&u4Rgn, i4Width, i4Height, (void *)u4DataPA, OSD_CM_ARGB8888_DIRECT32
			, (i4Width * 4), 0, 0,  i4Width, i4Height);
	} else if (32 == u4BitCount) {
		OSD_RGN_Create(&u4Rgn, i4Width, i4Height, (void *)u4DataPA, OSD_CM_ARGB8888_DIRECT32
			, (i4Width * 4), 0, 0, i4Width, i4Height);
	}
	OSD_RGN_Create(&u4Rgn, i4Width, i4Height, (void *)u4DataPA, OSD_CM_ARGB8888_DIRECT32
			, (i4Width * 4), 0, 0, i4Width, i4Height);

#if  0
	OSD_RGN_Set(u4Rgn, (__s32)OSD_RGN_COLOR_KEY, 31);
	OSD_RGN_Set(u4Rgn, (__s32)OSD_RGN_COLOR_KEY_EN, TRUE);
#endif
	OSD_RGN_Set(u4Rgn, OSD_RGN_MIX_SEL, OSD_BM_PLANE);
	OSD_RGN_LIST_DetachAll(u4RgnList);
	OSD_RGN_Insert(u4Rgn, u4RgnList);
	SetPlaneRgn(u4RgnList, u4Rgn);
	i4OsdPlaneFlipTo(u4Plane, u4RgnList);
	Printf("primary surface inital \r\n");
#else
	i4OsdSetDisplayMode(1, G_DISPLAY_MODE);
#endif

#ifdef CONFIG_ATC_OS_linux
		/*Recover Mode inital primary surface */
		if(fgDualHALGetUpgradeMode() == 2) 
		{
			u4Plane = PRIMARY_SURF_ID;
			i4OsdSetDisplayMode(u4Plane, G_DISPLAY_MODE);
			OSD_BASE_SetOsdPosition(u4Plane, PRIMARY_OSD_X_OFFSET, PRIMARY_OSD_Y_OFFSET);
			OSD_SC_Scale(u4Plane, TRUE, i4Width, i4Height
				, G_OUTPUT_WIDTH - PRIMARY_OSD_X_OFFSET - PRIMARY_OSD_RIGHT_CORDER_X_MARGIN
				, G_OUTPUT_HEIGHT - PRIMARY_OSD_Y_OFFSET - PRIMARY_OSD_RIGHT_CORDER_Y_MARGIN);
			OSD_RGN_LIST_Create(&u4RgnList);
			u4RgnList  = u4Plane;
		
		
			OSD_RGN_Create(&u4Rgn, i4Width, i4Height, (void *)u4DataPA, OSD_CM_ARGB8888_DIRECT32
					, (i4Width * 4), 0, 0, i4Width, i4Height);
			OSD_RGN_Set(u4Rgn, OSD_RGN_MIX_SEL, OSD_BM_PLANE);
			OSD_RGN_LIST_DetachAll(u4RgnList);
			OSD_RGN_Insert(u4Rgn, u4RgnList);
			SetPlaneRgn(u4RgnList, u4Rgn);
			i4OsdPlaneFlipTo(u4Plane, u4RgnList);
			#ifdef CONFIG_ATC_OS_linux
			i4OsdPlaneEnble(u4Plane, TRUE);
			#endif
			Printf("primary surface inital \r\n");
		}
		else 
		{
			i4OsdSetDisplayMode(1, G_DISPLAY_MODE);
		}
#endif
	i4OsdPlaneEnble(OSD_PLANE_2, TRUE);
	vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_4);
	Printf("primary OSD_PLANE_2 open \r\n");
	/*Reset OSD HW*/
	u4Idx = HAL_READ32(osdf_reg + 4);
	u4Idx |= OSD_RESET_PLANE_MASK;
	HAL_WRITE32(osdf_reg + 4, u4Idx);
	vTaskDelay(pdMS_TO_TICKS(5));
	u4Idx &= (~OSD_RESET_PLANE_MASK);
	u4Idx |= 0x01F00000;
	HAL_WRITE32(osdf_reg + 4, u4Idx);
	
	WriteREG(0xa504c,0x0000000f); //reset lvds
	vTaskDelay(pdMS_TO_TICKS(10));
	WriteREG(0xa504c,0x00000000);
	#if 0
	//6014
	regvalue = 0xfa54cb68;

	HAL_WRITE32(0xfd006014,regvalue);
	//606c
	regvalue = HAL_READ32(0xfd00606c);

	regvalue |= 0x1;

	HAL_WRITE32(0xfd00606c,regvalue);
	#endif

	memcpy((void *)ARM1PHY2ARM2UCV(ARM2_FBDRV_SHARE_GAMMA_PA), (void *)g_pbPanelGamma, 64);
}

__u32 floatToInt(float f)
{
	__u32 i = 0;

	if (f > 0) {
		i = (f * 10 + 5) / 10;
	} else if (f <= 0) {
		Printf("error H/VTotal \r\n");
	}

	return i;
}

void  DisplayLighten(void)
{
    static BOOL s_fgFirstInit = FALSE;
	__u32 brightness_val = 55;
	__u32 contrast_val = 21;
	__u32 saturation_val = 50;
	__u32 dither_val = 2;
	__u32 backlight_val = 64;
	UINT32 ret = 0;

	if(!s_fgFirstInit)
	{
		ret = _MetaZone_Read(MZ_DISPLAY_BRIGHTNESS, &brightness_val);
		if (ret) {
			Printf("_MetaZone_Read MZ_DISPLAY_BRIGHTNESS Failed\r\n");
		} else {
			Printf("_MetaZone_Read MZ_DISPLAY_BRIGHTNESS %d successed\r\n", brightness_val);
		}
		ret = _MetaZone_Read(MZ_DISPLAY_CONTRAST, &contrast_val);
		if (ret) {
			Printf("_MetaZone_Read MZ_DISPLAY_CONTRAST Failed\r\n");
		} else {
			Printf("_MetaZone_Read MZ_DISPLAY_CONTRAST %d successed\r\n", contrast_val);
		}
		ret = _MetaZone_Read(MZ_DISPLAY_SATURATION, &saturation_val);
		if (ret) {
			Printf("_MetaZone_Read MZ_DISPLAY_SATURATION Failed\r\n");
		} else {
			Printf("_MetaZone_Read MZ_DISPLAY_SATURATION %d successed\r\n", saturation_val);
		}
		ret = _MetaZone_Read(MZ_DISPLAY_DITHER, &dither_val);
		if (ret) {
			Printf("_MetaZone_Read MZ_DISPLAY_DITHER Failed\r\n");
		} else {
			Printf("_MetaZone_Read MZ_DISPLAY_DITHER %d successed\r\n", dither_val);
		}
		ret = _MetaZone_Read(MZ_DISPLAY_BACKLIGHT, &backlight_val);
		if (ret) {
			Printf("_MetaZone_Read MZ_DISPLAY_BACKLIGHT Failed\r\n");
		} else {
			Printf("_MetaZone_Read MZ_DISPLAY_BACKLIGHT %d successed\r\n", backlight_val);
		}
		ret = _MetaZone_ReadBinary(MZ_DISPLAY_GAMMA, (BYTE *)g_pbPanelGamma, 64);
		if (ret == MZ_FAILURE) {
			Printf("_MetaZone_Read MZ_DISPLAY_GAMMA Failed\r\n");
		} else {
			Printf("_MetaZone_Read MZ_DISPLAY_GAMMA successed\r\n");
		}
		vTconSetBrightness(brightness_val);
		vTconSetContrast(contrast_val);
		vTconSetHue(50);
		vTconSetSaturation(saturation_val);
		vPanelSetGamma(g_pbPanelGamma);
		TconSetDither(1, 2, dither_val);

		if (800 == G_OUTPUT_WIDTH && 480 == G_OUTPUT_HEIGHT) {
			if (PANEL_DCLK >= 300) {
				Printf("set current 6! \r\n");
				DisplaySetDrvAbility(6);
			}
		}
		vTaskDelay(pdMS_TO_TICKS(50)); /*wait LVDS data ready to show*/
        vSetBacklightIntensity(backlight_val);

		s_fgFirstInit = TRUE;
	} else {
		ret = _MetaZone_Read(MZ_DISPLAY_BACKLIGHT, &backlight_val);
		vTaskDelay(pdMS_TO_TICKS(30)); /*wait LVDS data ready to show*/
		if (ret) {
			Printf("_MetaZone_Read MZ_DISPLAY_BACKLIGHT Failed\r\n");
		} else {
			Printf("_MetaZone_Read MZ_DISPLAY_BACKLIGHT %d successed\r\n", backlight_val);
		}
		vSetBacklightIntensity(backlight_val);
	}
}

bool  BL_DisplayInit(void)
{
#if ENABLE_LOAD_MRF
	HANDLE  hMrf = NULL;
#endif
	void    *lpFB = NULL;
	bool    bRet = FALSE;
	BITMAPOBJINFO   bitinfo;
	__u32 u4LcdType = LCD_TYPE_LVDS; /*default is lvds in android*/
	unsigned int u4DisplayBitCnt = 32;
	__u32 piccount;
	__u32 picbpp;
	__u32 get_resolution, ret;
	__u32 u4Idx;
	RSV_MEM_T *fb = NULL;
	RSV_MEM_T *ani = NULL;
	RSV_MEM_T *im = NULL;
	RSV_MEM_T *wch = NULL;
	fb = get_rsv_mem_by_name("framebuffer");
	ani = get_rsv_mem_by_name("animation");
	im = get_rsv_mem_by_name("imageresize");
	wch = get_rsv_mem_by_name("wch_rsv");
	if ((NULL == fb) || (NULL == ani) || (NULL == im) || (NULL == wch))
		return FALSE;
	fbm_base = (UINT32)(fb->start_addr);
	fbm_size = (UINT32)(fb->size);
	#ifdef CONFIG_ATC_OS_linux
	logo_base = (UINT32)(ani->start_addr);
	logo_size = (UINT32)(ani->size);
	#else
	logo_base = fbm_base;
	logo_size = fbm_size;
	#endif
	fbm_va = fbm_base;
	vm_base = (UINT32)(im->start_addr);
	vm_size = (UINT32)(im->size);
	vm_va = vm_base;
	set_pool_param(vm_base, vm_va, vm_size);
	wchReservebase = (UINT32)(wch->start_addr);
	WchSetSourceBaseAddr(wchReservebase);
	Printf("[ARM2] video base:0x%x, size:0x%x, fb base 0x%x, size 0x%x\r\n"
		, vm_base, vm_size, fbm_base, fbm_size);
	MMInit();

#if ENABLE_LOAD_MRF
	hMrf = LoadLogoMRF();
	MRFHEADER     *pMrfHeader = (MRFHEADER *)hMrf;
	u4DisplayBitCnt = pMrfHeader->u4bitcount;
	Printf("mrf reserve count is %d \r\n",u4DisplayBitCnt);
	bRet = GetBitmapInfo(hMrf, 2, &bitinfo);
	if (bRet) {
		lpFB = GetRCObjectMemAddr(hMrf, &bitinfo);
	#ifdef CONFIG_ATC_OS_linux
	    G_LOGO_BPP = u4DisplayBitCnt;
	    g_rFBConfig.u4LogoBpp = G_LOGO_BPP;
		if(fgDualHALGetUpgradeMode() == 1)
		{
	   		memcpy((void *)(ARM1PHY2ARM2UCV(logo_base)), (const void *)lpFB,bitinfo.u4Height*bitinfo.u4Width*bitinfo.u4BitCount/8);
		}
		else if(fgDualHALGetUpgradeMode() == 2)
		{
			if(u4DisplayBitCnt == 16) {
				piccount = pMrfHeader->u4picturecount;
				bRet = GetBitmapInfo(hMrf, piccount-2, &bitinfo);
				lpFB = GetRCObjectMemAddr(hMrf, &bitinfo);
				memcpy((void *)(ARM1PHY2ARM2UCV(fbm_base)), (const void *)lpFB,bitinfo.u4Height*bitinfo.u4Width*bitinfo.u4BitCount/8);
			}
			else {
				memcpy((void *)(ARM1PHY2ARM2UCV(fbm_base)), (const void *)lpFB,bitinfo.u4Height*bitinfo.u4Width*bitinfo.u4BitCount/8);
			}
		}
	#else
		if(fgDualHALGetUpgradeMode() == 0)
		{

			memcpy((void *)ARM1PHY2ARM2UCV(logo_base), lpFB,
		       		bitinfo.u4Height * bitinfo.u4Width * bitinfo.u4BitCount / 8);
		}
	#endif
	}

	Printf("bitinfo.u4Height(%d) bitinfo.u4Width(%d) bitinfo.u4BitCount(%d)\r\n",
	       bitinfo.u4Height, bitinfo.u4Width, bitinfo.u4BitCount);
	G_OUTPUT_WIDTH = bitinfo.u4Width;
	G_OUTPUT_HEIGHT = bitinfo.u4Height;
#else
		ret = _MetaZone_Read(MZ_DISPLAY_RESOLUTION, &get_resolution);
		if (ret) {
			Printf("_MetaZone_Read failed, use default resolution:: 1024x600\r\n");
			get_resolution = 0;
		} else {
			Printf("_MetaZone_Read successed, get_resolution: %d\r\n", get_resolution);
		}
		ret = _MetaZone_Read(MZ_DISPLAY_ROTATE, &screen_rotate);
		if (ret) {
			Printf("_MetaZone_Read failed, use default screen_rotate: 0\r\n");
			screen_rotate = 0;
			g_rotate_value = 0;
		} else {
			g_rotate_value = screen_rotate;
			Printf("_MetaZone_Read successed, screen_rotate: %d\r\n", screen_rotate);
			switch (screen_rotate) {
			case 0:
				break;
			case 1:
				screen_rotate = VDP_ROTATE_90;
				break;
			case 2:
				screen_rotate = VDP_ROTATE_180;
				break;
			case 3:
				screen_rotate = VDP_ROTATE_270;
				break;
			};
			Printf("screen_rotate: 0x%x\r\n", screen_rotate);
		}
		if (1 == get_resolution) {
			G_OUTPUT_WIDTH = 800;
			G_OUTPUT_HEIGHT = 480;
		} else if (2 == get_resolution) {
			G_OUTPUT_WIDTH = 1920;
			G_OUTPUT_HEIGHT = 720;
		} else {
			G_OUTPUT_WIDTH = 1024;
			G_OUTPUT_HEIGHT = 600;
		}
		if(fgDualHALGetUpgradeMode() == 1) {
			memset((void *)(ARM1PHY2ARM2UCV(logo_base)), 0xff, G_OUTPUT_WIDTH * G_OUTPUT_HEIGHT * u4DisplayBitCnt / 8);
		} else if(fgDualHALGetUpgradeMode() == 2) {
			memset((void *)(ARM1PHY2ARM2UCV(fbm_base)), 0xff, G_OUTPUT_WIDTH * G_OUTPUT_HEIGHT * u4DisplayBitCnt / 8);
		}
#endif

	if (800 == G_OUTPUT_WIDTH && 480 == G_OUTPUT_HEIGHT)
		u4LcdType = LCD_TYPE_TTL;

#if defined(CONFIG_ATC_PLATFORM_ac823x)
	return TRUE;
#endif
	Printf("u4Height(%d) u4Width(%d) u4BitCount(%d)\r\n",
	       G_OUTPUT_HEIGHT, G_OUTPUT_WIDTH, u4DisplayBitCnt);
	DisplayInit(fbm_base, G_OUTPUT_WIDTH, G_OUTPUT_HEIGHT, u4DisplayBitCnt, u4LcdType);

#ifdef CONFIG_ATC_OS_linux 
	BootAnimationOsdInit(logo_base,TRUE);
#endif
#ifdef LCD_POWER_CONTROL_USEGIO29
	HAL_WRITE32(0xa0000068, HAL_READ32(0xa0000068) & (~(7U << 28))); /*hbs lcd power.*/
	GPIO_Config(29, OUTPUT, HIGH);
	GPIO_Config(28, OUTPUT, HIGH);
#endif

	return TRUE;
}

void vSetFBConfigToShareMemory(FB_CONFIG_T *prConfig)
{
	memcpy(ARM1PHY2ARM2UCV(ARM2_FBDRV_SHARE_MEMORY_PA), prConfig, sizeof(FB_CONFIG_T));
}

void vSetPWM(__u32 dty_cyc)
{
	__u32 u4PwmRsn = 0xFFF;
	__u32 u4PwmP = 0x4;
	__u32 u4PwmH = 0x800;
	__u32 u4PwmH_FB = 0x800;

	pwm_config_code pwm_bl[2] = {0};

	u4PwmRsn = (269) & 0xFFF;
	dty_cyc  = (dty_cyc > 100) ? 100 : dty_cyc;
	u4PwmH = (dty_cyc * u4PwmRsn / 100) & 0xFFF;
	u4PwmH_FB = ((100 - dty_cyc) * u4PwmRsn / 100) & 0xFFF;

	/*pwm_bl en setting*/
	pwm_bl[0].clk_id = CLKSRC_27M,
		  pwm_bl[0].pin_id = 0; /*GPIO 125 output en*/
	pwm_bl[0].pwm_en = 0;
	pwm_bl[0].pwm_high = u4PwmH;
	pwm_bl[0].pwm_prescale = u4PwmP;
	pwm_bl[0].pwm_rsn = u4PwmRsn;
	pwm_bl[0].pwm_mode = SYNC_TRI_MODE;

	/*pwm_bl fb setting*/
	pwm_bl[1].clk_id = CLKSRC_27M,
		  pwm_bl[1].pin_id = 1; /*GPIO 150 output en*/
	pwm_bl[1].pwm_en = 1;
	pwm_bl[1].pwm_high = u4PwmH_FB;
	pwm_bl[1].pwm_prescale = u4PwmP;
	pwm_bl[1].pwm_rsn = u4PwmRsn;
	pwm_bl[1].pwm_mode = SYNC_TRI_MODE;

	pwm_hal_config(3, &(pwm_bl[0]));
	pwm_hal_config(1, &(pwm_bl[1]));

	Printf("[wts][arm2]set bk light %d, time %d\r\n", dty_cyc, GetBootTime());
}

void vDisableBacklight()
{
	GPIO_MultiFun_Set(PIN_125_GPIO125, PINMUX_LEVEL_GPIO_END_FLAG);
	gpio_direction_output(PIN_125_GPIO125, 0);
	Printf("[bkl][arm2]vDisableBacklight\r\n");
}

unsigned int vGetRotateValue(void)
{
    return g_rotate_value;
}

void SendVsync(int event)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (errQUEUE_FULL == xQueueSendFromISR(xDisplayQueue, &event, &xHigherPriorityTaskWoken)) {
		Printf("queue full, lost event %d\n", event);
	}

	portYIELD_FROM_ISR();
}
int arm1VsyncReadyFlag = 0;
void SetArm1VsyncReady(int flag)
{
	arm1VsyncReadyFlag = flag;
}

int GetArm1VsyncReady(void)
{
	return arm1VsyncReadyFlag;
}
static void VsyncCallback( void * parameters )
{
	int event = 0;

	while (1) {
		xQueueReceive(xDisplayQueue, &event, portMAX_DELAY);
		if (!GetArm1VsyncReady()) {
			LTDC_IRQHandler();
		}
	}
}

void DisplayVsyncInit(void)
{
	BaseType_t xReturn = pdFAIL;

	xDisplayQueue = xQueueCreate(DISPLAY_QUEUE_ITEMS_COUNT, sizeof(int));
	if (!xDisplayQueue)
	{
		Printf("Create Queue failed.\n");
		return;
	}

	xReturn = xTaskCreate(VsyncCallback, "Vsync", 1000, NULL, 3U, NULL);
	if (xReturn == pdPASS)
		Printf("VsyncCallback create pass\n");
}

