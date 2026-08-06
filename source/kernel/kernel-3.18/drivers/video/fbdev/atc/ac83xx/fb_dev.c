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
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
/*#include <linux/earlysuspend.h>*/
#include <linux/jiffies.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of.h>
#include <linux/of_reserved_mem.h>
#include <linux/of_fdt.h>
#include <linux/clk.h>
#include <linux/clk-private.h>
#include <linux/pinctrl/consumer.h>
#include <linux/time.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>
#include <media/atc/display_inc.h>
#include <media/atc/vdp_mdd.h>
#include <media/atc/display.h>
#include <media/atc/pmx_hal.h>
#include <media/atc/drv_av_d.h>
#include <media/atc/drv_osd_if.h>
#include "fb.h"
#include "vdp_hw.h"
#include "vdp_hal.h"
#include "scl_hal.h"
#include "tve_hal.h"
#include "vdp.h"
#include "tcon.h"
#include "osd_if_pdd.h"
 #include "x_ckgen.h"
#include "drv_imgresz.h"
#include "osd_inc.h"
#include "log.h"
#include "oal.h"
#include "x_ver.h"
#include "pmx/pmx_vfy_hal.h"
#include <sync.h>
#include <sw_sync.h>
#include <linux/file.h>
#include <asm/atomic.h>
#include <generated/atc_project.h>
#include "disp_assert_layer.h"
#include "metazone.h"
#include "metazone_ioctl.h"
#include "drv_dual.h"

#define MAX_TIMELINE_NAME_LEN 16
#define MTK_KERNEL_LINUX_LICENSE     "Proprietary"

#define MMISC_MODE_NAME                   "FB"
#define MMISC_VER_MAJOR                   01
#define MMISC_VER_MINOR                   00
#define MMISC_VER_REV                     00


/**/
#define DISPLAY_SET_LVDS_SSC                    (0x00020001)
#define DISPLAY_GET_DITHER                      (0x00020002)
#define DISPLAY_SET_DITHER_DISABLE              (0x00020003)
#define DISPLAY_SET_DITHER                      (0x00020004)
#define DISPLAY_SET_CONTRAST                    (0x00020005)
#define DISPLAY_SET_BRIGNTNESS                  (0x00020006)
#define DISPLAY_SET_SATURATION                  (0x00020007)
#define DISPLAY_SET_HUE                         (0x00020009)
#define DISPLAY_OSD8_MIX2DVD                    (0x0002000C)
#define DISPLAY_FR_FOLLOW                       (0x0002000D)
#define DISPLAY_GET_REAR_ON                     (0x0002000E)
#define DISPLAY_SET_YGAIN                       (0x0002001A)
#define DISPLAY_SET_UGAIN                       (0x0002001B)
#define DISPLAY_SET_VGAIN                       (0x0002001C)
#define DISPLAY_SET_GAMMA                       (0x0002001D)
#define DISPLAY_GET_CONTRAST                    (0x0002001E)
#define DISPLAY_GET_BRIGNTNESS                  (0x0002001F)
#define DISPLAY_GET_SATURATION                  (0x00020020)
#define DISPLAY_GET_HUE                         (0x00020022)
#define DISPLAY_GET_YGAIN                       (0x00020023)
#define DISPLAY_GET_UGAIN                       (0x00020024)
#define DISPLAY_GET_VGAIN                       (0x00020025)
#define DISPLAY_GET_GAMMA                       (0x00020026)
#define DISPLAY_GET_ROTATE                      (0x00020030)

/*#define ST_DEBUG*/

#define GPIO_LCM_PWDN 56
/*cont*/

#define SHOW_GIS_FPS 1


#define BUF_LEN  128
#define GAMMA_TABLE_64_SIZE 64
#ifdef UNUSED
#undef UNUSED
#define UNUSED(x) ((void)(x))
#else
#define UNUSED(x) ((void)(x))
#endif

struct display_gamma_deveice {
	uint8_t disp_gamma[64];
};

struct display_gamma_deveice disp_gamma_dev;

__u32 fb_log_lvl = FB_LOG_LVL_HAL;
__u8 *fb_lvl_str[] = {
	"OFF",
	"ERR",
	"WARN",
	"CLI",
	"INFO",
	"HAL",
	"IRQ",
	"TRACE",
	"DBG",
	"REGRW",
};

/*static struct fb_fix_screeninfo mtkfb_fix __devinitdata = {*/
static struct fb_fix_screeninfo mtkfb_fix = {
	.id          = "fb",
	.type        = FB_TYPE_PACKED_PIXELS,
	.visual      = FB_VISUAL_TRUECOLOR,
	.ypanstep    = 1,
	.accel       = FB_ACCEL_NONE
};

static int aRGBBitMask[][6] = {
	{11, 5, 5, 6, 0, 5  },
	{16, 8, 8, 8, 0, 8, },
};

typedef struct  _FB_SYNC_DATA
{
	UINT32  acq_fen_cnt;
	struct sync_fence *acq_fen[MAX_FENCE_CNT];
	struct sw_sync_timeline *timeline;
       int         timeline_value;
      atomic_t  commit_cnt;
       struct mutex sync_mutex;
}FB_SYNC_DATA_T;

static FB_SYNC_DATA_T  _rSyncData;
int aBitsPerPixel[] = {
	16, 32
};

struct LVDS_SSC {
	__u32 dir;
	__u32 freq;
	__u32 range;
};

FB_CONFIG_T g_rFBConfig;

/*static struct fb_var_screeninfo mtkfb_var __devinitdata = {*/
static struct fb_var_screeninfo mtkfb_var = {
	.xres           = PRIMARY_OSD_WIDTH,
	.yres           = PRIMARY_OSD_HEIGHT,
	.xres_virtual   = PRIMARY_OSD_WIDTH,
	.yres_virtual   = PRIMARY_OSD_HEIGHT * ANDROID_NUMBER_OF_BUFFERS,
	.activate       = FB_ACTIVATE_NOW,
	.width          = LCD_WIDTH_PHYSCIAL,
	.height         = LCD_HEIGHT_PHYSCIAL,

};

static struct fb_info *fb;

/*static __u32 _u4BacklightValue;*/
#ifdef CONFIG_HAS_EARLYSUSPEND
static struct early_suspend early_suspend;
#endif
void atc_fb_wait_for_fence(FB_SYNC_DATA_T *mfd);
int atc_fb_signal_timeline(FB_SYNC_DATA_T *mfd);

struct mtkfb {
	__u32 cmap[16];
};

#if SHOW_GIS_FPS
static int g_printk_gis;
static unsigned int g_count_gis;
static int32_t g_startTime, g_endTime;
#endif

int COLOR_DEPTH_32_BIT = 1;
module_param(COLOR_DEPTH_32_BIT, int, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH); /* rw-rw-r-- */

#ifdef CONFIG_ATC_OS_linux
int CLEAR_FB = 1;
#else
int CLEAR_FB = 0;
#endif
module_param(CLEAR_FB, int, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH); /* rw-rw-r-- */


__u32  LCD_GetScreenWidth(void)
{
	if (RESET_HW_ENGINE == FALSE) {
		__u32 u4Width;

		u4Width = (*((__u32 *) 0xFD02001C) >> 16) & 0x7FF;
		return u4Width;
	} else {
		return PRIMARY_OSD_WIDTH;
	}
}
EXPORT_SYMBOL(LCD_GetScreenWidth);

LVDS_OUTPUT_MODE_E LCD_GetLVDS_OutputMode(void)
{
	return ((*((__u32 *) 0xFD0a5010)  == 0) ? LVDS_OUTPUT_MODE_8BITS : LVDS_OUTPUT_MODE_6BITS);
}

__u32 LCD_GetScreenHeight(void)
{
	if (RESET_HW_ENGINE == FALSE) {
		__u32 u4Height = *((__u32 *) 0xFD02001C) & 0x7FF;

		return u4Height;
	} else {
		return PRIMARY_OSD_HEIGHT;
	}
}
EXPORT_SYMBOL(LCD_GetScreenHeight);


__u32 LCD_GetLcdType(void)
{
	__u32 LcdType = LCD_TYPE_LVDS; /*default is lvds in android*/

	LcdType = (*((__u32 *) 0xFD0a5090) & 0x1000000) >> 24;
	FB_PRINT(FB_LOG_LVL_DBG, "", "LCD_GetLcdType1: %d\n", (int)LcdType);

	return LcdType;
}

static void LCM_GetConfig(void)
{
	LVDS_OUTPUT_MODE_E  eLVDSMode;

	_u4LCDWidth = LCD_GetScreenWidth();
	_u4LCDHeight = LCD_GetScreenHeight();
	eLVDSMode = LCD_GetLVDS_OutputMode();
	_u4LCDType = LCD_GetLcdType();
	vSetLVDSOutputMode(eLVDSMode);
	vGetFBConfigFromShareMemory(&g_rFBConfig);
	_u4RearOutputMode = g_rFBConfig.u4VideoMode;
	_u4DispMode = 6;
	g_rPanelSetting = g_rFBConfig.rFBPanelSetting;
	g_rVdoWindowSetting = g_rFBConfig.rVdoWindowSetting;

	if (_u4RearOutputMode > RES_576P) {
		_u4RearOutputMode = RES_480P;
		FB_PRINT(FB_LOG_LVL_ERR, "", "[LCM_GetConfig] before %d, after %d\r\n"
			, (int)g_rFBConfig.u4VideoMode, (int)_u4RearOutputMode);
	}
	FB_PRINT(FB_LOG_LVL_INFO, "", "[DDI]LCM_GetConfig g_rFBConfig: u4TypeClock=%d,rSclHdShift=[0x%x,0x%x]\n",
		g_rFBConfig.rFBPanelSetting.u4TypeClock, g_rFBConfig.rFBPanelSetting.rSclHdShift);
	FB_PRINT(FB_LOG_LVL_INFO, "", "g_rFBConfig: FmtSdHV=[0x%x,0x%x],rFmtWindowSetting=[%d,%d,%d,%d,%d,%d]\n",
		g_rFBConfig.rVdoWindowSetting.u4FmtSdHstart,g_rFBConfig.rVdoWindowSetting.u4FmtSdVstart,
		g_rFBConfig.rVdoWindowSetting.rFmtWindowSetting);
	FB_PRINT(FB_LOG_LVL_INFO, "", "[DDI]LCM_GetConfig  @@g_rFBConfig: rSclWindowSetting=[%d,%d,%d,%d,%d,%d,]\n",
		g_rFBConfig.rVdoWindowSetting.rSclWindowSetting);

	FB_PRINT(FB_LOG_LVL_INFO, "", "_u4DispMode=%d,@g_rPanelSetting: u4TypeClock=%d,rSclSdShift=[0x%x,0x%x]\n",
		_u4DispMode, g_rPanelSetting.u4TypeClock, g_rPanelSetting.rSclSdShift);
	mtkfb_fix.line_length = _u4LCDWidth  * (aBitsPerPixel[COLOR_DEPTH_32_BIT] / 8);
	mtkfb_var.xres = _u4LCDWidth;
	mtkfb_var.yres = _u4LCDHeight;
	mtkfb_var.xres_virtual = _u4LCDWidth;
	mtkfb_var.yres_virtual = _u4LCDHeight * ANDROID_NUMBER_OF_BUFFERS;
	mtkfb_var.red.offset = aRGBBitMask[COLOR_DEPTH_32_BIT][0];
	mtkfb_var.red.length = aRGBBitMask[COLOR_DEPTH_32_BIT][1];
	mtkfb_var.green.offset = aRGBBitMask[COLOR_DEPTH_32_BIT][2];
	mtkfb_var.green.length = aRGBBitMask[COLOR_DEPTH_32_BIT][3];
	mtkfb_var.blue.offset = aRGBBitMask[COLOR_DEPTH_32_BIT][4];
	mtkfb_var.blue.length = aRGBBitMask[COLOR_DEPTH_32_BIT][5];
	mtkfb_var.bits_per_pixel = aBitsPerPixel[COLOR_DEPTH_32_BIT];
}
static inline __u32 convert_bitfield(int val, struct fb_bitfield *bf)
{
	unsigned int mask = (1 << bf->length) - 1;
	unsigned int ret = (val >> (16 - bf->length) & mask) << bf->offset;

	return ret;
}

void HideBackVideo(void)
{
	i4OsdPlaneEnble(2, FALSE);
	vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_1);
	FB_PRINT(FB_LOG_LVL_INFO, "", "%s vPmxHalNotMixPlane video plane\n", __func__);
}
EXPORT_SYMBOL(HideBackVideo);

/* set the software color map.  Probably doesn't need modifying. */
static int
mtk_fb_setcolreg(unsigned int regno, unsigned int red, unsigned int green,
		 unsigned int blue, unsigned int transp, struct fb_info *info)
{
	struct mtkfb *mtkfb = info->par;

	if (regno < 16) {
		mtkfb->cmap[regno] = convert_bitfield(transp, &info->var.transp) |
				     convert_bitfield(blue, &info->var.blue) |
				     convert_bitfield(green, &info->var.green) |
				     convert_bitfield(red, &info->var.red);
		return 0;
	} else {
		return -EINVAL;
	}
}

/* check var to see if supported by this device.  Probably doesn't
 * need modifying.
 */
static int mtk_fb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	FB_PRINT(FB_LOG_LVL_DBG, "", "mtk_fb_check_var begin\n");

	if ((var->xres != info->var.xres) ||
	    (var->yres != info->var.yres) ||
	    (var->xres_virtual != info->var.xres) ||
	    (var->yres_virtual >
	     info->var.yres * ANDROID_NUMBER_OF_BUFFERS) ||
	    (var->yres_virtual < info->var.yres)) {
		return -EINVAL;
	}

	if ((var->xoffset != info->var.xoffset) ||
	    (var->bits_per_pixel != info->var.bits_per_pixel) ||
	    (var->grayscale != info->var.grayscale)) {
		return -EINVAL;
	}

	return 0;
}

/* Handles screen rotation if device supports it. */
static int mtk_fb_set_par(struct fb_info *info)
{
	return 0;
}

void exit_arm2_display(void)
{
	UINT32 user_buf[4] = {0x2 << 24, 1, 2, 3};
	HWSendMessage(user_buf[0], user_buf[1], user_buf[2], user_buf[3]);
}

extern void log_boot(char *str);
static int g_first_frame = 0;

/* Pan the display if device supports it. */
static int mtk_fb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
#if SHOW_GIS_FPS

	if (1 == g_printk_gis) {
		if (0 == g_count_gis) {
			g_startTime = jiffies_to_msecs(jiffies);
		}

		g_count_gis++;

		FB_PRINT(FB_LOG_LVL_ERR, "", "[GIS_FPS] count : %d\n", (int)g_count_gis);

		if (g_count_gis == 0xFFFFFFFF) {
			FB_PRINT(FB_LOG_LVL_ERR, "", "[GIS_FPS] count will overflow,reset to 0\n");
			g_count_gis = 0;
		}
	} else if (2 == g_printk_gis) {
		g_endTime = jiffies_to_msecs(jiffies);
		g_count_gis++;

		if (g_count_gis == 0xFFFFFFFF) {
			FB_PRINT(FB_LOG_LVL_ERR, "", "[GIS_FPS] count will overflow,reset to 0\n");
			g_count_gis = 0;
		}

		if ((g_endTime > g_startTime) && (g_count_gis != 0)) {
			int integer1 = g_count_gis / ((g_endTime - g_startTime) / 1000);
			int integer2 = (g_count_gis * 10 / ((g_endTime - g_startTime) / 1000)) % 10;
			int integer3 = (g_count_gis * 100 / ((g_endTime - g_startTime) / 1000)) % 10;

			FB_PRINT(FB_LOG_LVL_ERR, "", "[GIS_FPS] count : %d, costtime : %dms, fps : %d.%d%d\n",
				 (int)g_count_gis, g_endTime - g_startTime, integer1, integer2, integer3);
		} else {
			FB_PRINT(FB_LOG_LVL_ERR, "", "[GIS_FPS]count or jiffies overflow\n");
			FB_PRINT(FB_LOG_LVL_ERR, "", "[GIS_FPS]jiffies : %d, HZ :%d", (int)jiffies, (int)HZ);
			FB_PRINT(FB_LOG_LVL_ERR, "", "g_endTime : %dms, g_startTime : %dms, count : %d\n"
				, g_endTime, g_startTime, (int)g_count_gis);
			g_printk_gis = 0;
		}
	}

#endif
	display_flip(var);
	flush_cache_all();
	atc_fb_signal_timeline(&_rSyncData);
	if (!g_first_frame) {
		g_first_frame = 1;
		if (!fgPmxHalMixPlane(PMX_1, PMX_HW_PLANE_4)) {
			vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_4);  // enable osd2 layer
			FB_PRINT(FB_LOG_LVL_WARN, "", "open primary surface plane osd2\r\n");
		}
		if (fgPmxHalMixPlane(PMX_1, PMX_HW_PLANE_3)) {
			vPmxHalNotMixPlaneDelay(PMX_1, PMX_HW_PLANE_3);  // disable osd1 layer
			FB_PRINT(FB_LOG_LVL_WARN, "", "disable osd1 layer\r\n");
		}
		exit_arm2_display();
		log_boot("first frame pan display\n");
	}
	return 0;
}

UINT32 g_RearOSDRgn = 0;

static u32  RearOSDConfig(void)
{
    INT32  ret;
    UINT32 plane, rgn_list;
	UINT32 height,width;
	FB_CONFIG_T rFBConfig;
	unsigned int addr;

   // i4OsdVfyCreateSemaphores();
    plane = OSD_PLANE_8;
    vGetFBConfigFromShareMemory(&rFBConfig);
    height  = (rFBConfig.u4VideoMode == RES_480P) ? 480 : 576;
    width   = 720;
    addr = fbm_base;

    OSD_BASE_SetOsdPosition(plane, 0, 0);
    OSD_SC_Scale(plane, FALSE, width, height, width, height);
    OSD_RGN_LIST_Create(&rgn_list);
    rgn_list  = plane;
    OSD_RGN_Create(&g_RearOSDRgn, width, height, (void *)addr, OSD_CM_ARGB8888_DIRECT32, (width * 4), 0, 0,  width, height);

    //   OSD_RGN_Set(rgn, OSD_RGN_MIX_SEL, OSD_BM_PLANE);
    OSD_RGN_LIST_DetachAll(rgn_list);
    ret = OSD_RGN_Insert(g_RearOSDRgn, rgn_list);
    if (ret)
    {
        printk(KERN_ALERT "[FB1] OSD_RGN_Insert rgn failed: %d\n", (int)ret);
        return ret;
    }
    SetPlaneRgn(rgn_list, g_RearOSDRgn);
      printk("wts frds osd rgn %d  \n", g_RearOSDRgn);
    i4OsdPlaneFlipTo(plane, rgn_list);
    i4OsdPlaneEnble(plane, TRUE);

	 printk( "wyz RearOSDConfig width %d height %d\n", width,height);
    //TurnOnTve(plane, TRUE);

    return (TRUE);
}

static bool fginited = FALSE;
int FrontRearDisplayFollow(u32 fr_type)
{
	u32 plane = OSD_PLANE_8, u4Rgn;
         u32 prefollowtype = 0;

	if (is_rear_vdp_busy()) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "%s rear busy\r\n", __func__);
		return -EBUSY;
	}

	u4Rgn = GetPlaneRgn(plane);
	FB_PRINT(FB_LOG_LVL_INFO, "", "%s display follow %d plane %d init %d\r\n", __func__, fr_type, u4Rgn, fginited);
         prefollowtype = fr_follow;
	fr_follow = fr_type;
	if (fr_follow & FR_FOLLOW_UI) {
		if (fginited == FALSE) {
                           if (u4Rgn)
                           {
                                OSD_RGN_Delete(u4Rgn);
                                FB_PRINT(FB_LOG_LVL_INFO, "", "del on  = %d \n", u4Rgn);
                           }
			create_rear_osd_follow_with_front(plane, _u4LCDWidth, _u4LCDHeight, front_fb_current_addr);
			fginited = TRUE;
		} else {
			i4OsdPlaneEnble(plane, TRUE); //no need
		}
	} else {
		if (fginited == TRUE) {
			i4OsdPlaneEnble(plane, FALSE);
			SetPlaneRgn(plane, INVALID_RGN);
                           FB_PRINT(FB_LOG_LVL_INFO, "", "del off  = %d \n", u4Rgn);
                           if (u4Rgn)
                           {
                                OSD_RGN_Delete(u4Rgn);
                           }
                           
#if 0                           
                           if (g_RearOSDRgn)
                           {
                                OSD_RGN_Delete(g_RearOSDRgn); 
                                g_RearOSDRgn = 0;
                           }
#endif                           
                           RearOSDConfig();		
                           
			fginited = FALSE;
		}        
	}

	TurnOnTve(1, fr_follow ? TRUE : FALSE);

	return 0;
}

/*add by mtk94020 for vdp*/
static struct OSD_INFO osd_info;
static OSD_DATA_T _rData = {
	0
};
static unsigned long st_addr;
/*static struct VDP_ALPHA vdp_alpha;*/
int subHideFlag = 0;
/*#define g_u4stAddr (0x0C000000UL)*/
/*#define SUBTITLE_MEM_PA (0x0D000000UL)*/
/*#define SUBTITLE_MEM_SIZE (1024*1024/2UL)*/
struct FRONT_REAR_PARAM sub_fr_param = {0};

void vBufferFenceSync( FB_SYNC_T *prSync, FB_SYNC_DATA_T *mfd)
{
	int acq_fen_fd[MAX_FENCE_CNT];
	int i, ret =0;
	int release_fen_fd;
	struct sync_pt *release_sync_pt;
	struct sync_fence *release_fence;
	
	FB_SYNC_DATA_T	rSyncData;

	struct sync_fence *fence;

	//printk("[wts] fence cnt  %d \n", prSync->u4AcquireFenceCnt);

	//if (prSync->u4AcquireFenceCnt >0 )
	{
		//printk("wts fence id0 = %d \n", prSync->pru4AcquireFence[0] );
	}

	copy_from_user(acq_fen_fd, prSync->pru4AcquireFence, sizeof(int) * prSync->u4AcquireFenceCnt);

	mutex_lock(&mfd->sync_mutex);
	#if 0
	for (i = 0; i < prSync->u4AcquireFenceCnt; i++) {
		//printk("wts fence id %d \n",	acq_fen_fd[i] );
		fence = sync_fence_fdget(acq_fen_fd[i]);
		if (fence == NULL) {
			printk("%s: null fence! i=%d fd=%d\n", __func__, i, acq_fen_fd[i]);
			ret = -EINVAL;
			break;
		}
		mfd->acq_fen[i] = fence;
	}

	#endif

	mfd->acq_fen_cnt = 0;// i;


	release_fen_fd = get_unused_fd();
	if (release_fen_fd < 0) {
		printk("%s: get_unused_fd_flags failed", __func__);
		ret  = -EIO;
		goto buf_sync_err_1;
	}

	release_sync_pt = sw_sync_pt_create(mfd->timeline, mfd->timeline_value );
	release_fence = sync_fence_create("mdp-fence",	release_sync_pt);
	sync_fence_install(release_fence, release_fen_fd);

	ret = copy_to_user(prSync->pru4ReleaseFence, &release_fen_fd, sizeof(int));

	mutex_unlock(&mfd->sync_mutex);
	return ;

buf_sync_err_1:
	for (i = 0; i < mfd->acq_fen_cnt; i++)
		sync_fence_put(mfd->acq_fen[i]);
	mfd->acq_fen_cnt = 0;
	mutex_unlock(&mfd->sync_mutex);
	 
	return ; 
}

#define WAIT_FENCE_FIRST_TIMEOUT  (3  * MSEC_PER_SEC)

#define WAIT_FENCE_FINAL_TIMEOUT  ( 10 * MSEC_PER_SEC)

void atc_fb_wait_for_fence(FB_SYNC_DATA_T *mfd)
{
	int i, ret = 0;
	/* buf sync */
	for (i = 0; i < mfd->acq_fen_cnt; i++) {
		ret = sync_fence_wait(mfd->acq_fen[i], 	WAIT_FENCE_FIRST_TIMEOUT);
		if (ret == -ETIME) {
			printk("%s: sync_fence_wait timed out	Waiting %ld more seconds\n",__func__,WAIT_FENCE_FINAL_TIMEOUT/MSEC_PER_SEC);
			ret = sync_fence_wait(mfd->acq_fen[i],	WAIT_FENCE_FINAL_TIMEOUT);
		}
		if (ret < 0) {
			printk("%s: sync_fence_wait failed! ret = %x\n",	__func__, ret);
			break;
		}
		sync_fence_put(mfd->acq_fen[i]);
	}
	if (ret < 0) {
		while (i < mfd->acq_fen_cnt) {
			sync_fence_put(mfd->acq_fen[i]);
			i++;
		}
	}
	mfd->acq_fen_cnt = 0;
}

int atc_fb_signal_timeline(FB_SYNC_DATA_T *mfd)
{
	mutex_lock(&mfd->sync_mutex);
	if (mfd->timeline && !list_empty((const struct list_head *)
				(&(mfd->timeline->obj.active_list_head)))) {
		sw_sync_timeline_inc(mfd->timeline, 1);
		mfd->timeline_value++;
	}
    #if 0
	if (atomic_read(&mfd->commit_cnt) > 0)
	{
           atomic_dec(&mfd->commit_cnt);
	}
    #endif
	mutex_unlock(&mfd->sync_mutex);

	return 0;
}
#define GAMMA_LENGTH 64

static int mtk_fb_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int colorkey;
	int tconvalue = 0;
	int rotate = 0;
	#ifdef CONFIG_ATC_OS_linux
	int logo_bpp = 32;
	#endif
	__u8 PanelGamma[GAMMA_LENGTH] = {0};
	struct VDP_ALPHA vdpalpha;
	OSD_DATA_T  *prData;
	__u32 u4OSDIdx, u4Rgn, u4osdrRgn, u4Plane = PRIMARY_SURF_ID, u4osdrPlane3 = OSD_PLANE_8;
        FB_SYNC_T rSync;
	struct LVDS_SSC lvds_ssc_cfg;

	FB_PRINT(FB_LOG_LVL_DBG, "", "mtk_fb_ioctl begin %d\n", cmd);

	switch (cmd) {
	case FBIO_WAITFORVSYNC:
		vSclInVertBlank();
		break;

#ifndef MAINSURFACE_OSD2
	case STIOC_SET_DST_CK:
		if (copy_from_user(&colorkey, (int *)arg, sizeof(int))) {
			ret = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "copy_from_user return %d iocode %d\n", ret, cmd);
			break;
		}

		FB_PRINT(FB_LOG_LVL_DBG, "", "%s:%d:colorkey = %x\n", __func__, __LINE__, colorkey);

		if (COLOR_DEPTH_32_BIT) {
			colorkey = colorkey;
		} else {
			colorkey = (colorkey & 0x1f) | ((colorkey & 0x7e0) << 3) | ((colorkey & 0xf800) << 5);
		}

		if (colorkey) {
			OSD_PLA_SetDestColorKey(TRUE, (__u32)colorkey);
			vPmxHalSetPlaneDstColorKey(0, 1);
		} else {
			vPmxHalSetPlaneDstColorKey(0, 0);
		}

		FB_PRINT(FB_LOG_LVL_DBG, "", "%s:%d:colorkey = %x\n", __func__, __LINE__, colorkey);
		break;
#else
	case STIOC_SET_DST_CK:
		u4Rgn  = GetPlaneRgn(u4Plane);
                u4osdrRgn  = GetPlaneRgn(u4osdrPlane3);

		if (copy_from_user(&colorkey, (int *)arg, sizeof(int))) {
			ret = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "copy_from_user return %d iocode %d\n", ret, cmd);
			break;
		}
		if (colorkey) {
                        OSD_RGN_Set(u4Rgn, (__s32)OSD_RGN_COLOR_KEY, colorkey);
        	        OSD_RGN_Set(u4Rgn, (__s32)OSD_RGN_COLOR_KEY_EN, TRUE);
                        if(fr_follow & FR_FOLLOW_UI) {
                                OSD_RGN_Set(u4osdrRgn, (__s32)OSD_RGN_COLOR_KEY, colorkey);
        			OSD_RGN_Set(u4osdrRgn, (__s32)OSD_RGN_COLOR_KEY_EN, TRUE);
                        }
		} else {
			OSD_RGN_Set(u4Rgn, (__s32)OSD_RGN_COLOR_KEY_EN, FALSE);
                        if(fr_follow & FR_FOLLOW_UI) {
        			OSD_RGN_Set(u4osdrRgn, (__s32)OSD_RGN_COLOR_KEY_EN, FALSE);
                        }
		}
		FB_PRINT(FB_LOG_LVL_DBG, "", "%s:%d:colorkey = %x\n", __func__, __LINE__, colorkey);
		break;
#endif

	case STIOC_SET_VDO_ALPHA:
		if (copy_from_user(&vdpalpha, (struct VDP_ALPHA *)arg, sizeof(struct VDP_ALPHA))) {
			ret = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "copy_from_user return %d iocode %d\n", ret, cmd);
			break;
		}

		FB_PRINT(FB_LOG_LVL_DBG, "", "%s:%d:addr = %x, inalpha = %x, outalpha = %x, enable = %d\n",
			 __func__, __LINE__, (unsigned int)arg, vdpalpha.inalpha, vdpalpha.outalpha, vdpalpha.enable);

		/*0x100 alpha default setting*/
		vPmxHalSetAlpha(PMX_1, vdpalpha.inalpha, vdpalpha.outalpha, vdpalpha.enable);
		break;

	case STIOC_TVD2TVE:
		FB_PRINT(FB_LOG_LVL_DBG, "", "STIOC_TVD2TVE arg = %d\n", (int)arg);

		if (arg) {
			vTveHalSetMode(_u4RearOutputMode);
			vTveHalEnable(TVE_TVD);
		} else {
			vTveHalDisable();
		}

		break;

	case STIOC_SETOSDINFO:
		if (copy_from_user(&osd_info, (struct OSD_INFO *)arg, sizeof(struct OSD_INFO))) {
			ret = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "copy_from_user return %d iocode %d\n", ret, cmd);
		}

		break;

	case STIOC_SET_DST_RECT:
		if (copy_from_user(&_rData.rDestRect, (RECT *)arg, sizeof(RECT))) {
			ret = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "copy_from_user return %d iocode %d\n", ret, cmd);
		}

		break;

	case STIOC_SET_SRC_CLR_KEY:
		_rData.u4Flags |= OSD_DATA_SRC_CLR_KEY;
		_rData.u4SrcColorKey = arg;
		break;

	case  STIOC_SETOSD_8_BIT_PALETTE: {
		if (!access_ok(VERIFY_READ, arg, sizeof(OSD_DATA_T))) {
			ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "access_ok return %d iocode %d\n", ret, cmd);
			break;
		}
		FB_PRINT(FB_LOG_LVL_DBG, "", "STIOC_SETOSD_8_BIT_PALETTE,arg = %p\n", (void *)arg);
		prData = (OSD_DATA_T *)arg;
		st_addr = prData->u4BitmapPA;

		if (prData->device_name == USB) {
			u4OSDIdx = sub_fr_param.u4Idx;

			if ((sub_fr_param.u4PreIdx != u4OSDIdx)
			    && (sub_fr_param.u4PreFlags == sub_fr_param.u4Flags)) {
				/*front to rear,need to free front vdp or rear to front, need to free rear vdp*/
				subtitle_osd_off(sub_fr_param.u4PreIdx);
				prData->u4OutputPath = u4OSDIdx;
				prData->u4BlockID = u4OSDIdx;

				if (subHideFlag && (prData->u4OutputPath == FRONT_VDO)) {
					subtitle_osd_off(prData->u4OutputPath);
				} else {
					subtitle_osd_palette_init(st_addr,  prData);
				}

			} else if ((sub_fr_param.u4PreIdx != u4OSDIdx)
				   && (sub_fr_param.u4PreFlags != sub_fr_param.u4Flags)
				   && (sub_fr_param.u4PreFlags == VDP_BLOCK_MODE)) {
				/*front to front_rear or rear to front_rear*/
				prData->u4OutputPath = sub_fr_param.u4PreIdx;
				prData->u4BlockID = sub_fr_param.u4PreIdx;

				if (subHideFlag && (prData->u4OutputPath == FRONT_VDO)) {
					subtitle_osd_off(prData->u4OutputPath);
				} else {
					subtitle_osd_palette_init(st_addr,  prData);
				}

				prData->u4OutputPath = u4OSDIdx;
				prData->u4BlockID = u4OSDIdx;

				if (subHideFlag && (prData->u4OutputPath == FRONT_VDO)) {
					subtitle_osd_off(prData->u4OutputPath);
				} else {
					subtitle_osd_palette_init(st_addr,  prData);
				}

			} else if ((sub_fr_param.u4PreIdx != u4OSDIdx)
				   && (sub_fr_param.u4PreFlags != sub_fr_param.u4Flags)
				   && (sub_fr_param.u4PreFlags == VDP_FRAMELOCK_MODE)) {
				/*front_rear to front or front_rear to rear*/
				subtitle_osd_off(sub_fr_param.u4PreIdx);

				flush_cache_all();
				prData->u4OutputPath = u4OSDIdx;
				prData->u4BlockID = u4OSDIdx;

				if (subHideFlag && (prData->u4OutputPath == FRONT_VDO)) {
					subtitle_osd_off(prData->u4OutputPath);
				} else {
					subtitle_osd_palette_init(st_addr,  prData);
				}
			} else if ((sub_fr_param.u4PreIdx == u4OSDIdx)
				   && (sub_fr_param.u4PreFlags == VDP_BLOCK_MODE)) {
				/*front,rear not change*/
				flush_cache_all();
				prData->u4OutputPath = u4OSDIdx;
				prData->u4BlockID = u4OSDIdx;

				if (subHideFlag && (prData->u4OutputPath == FRONT_VDO)) {
					subtitle_osd_off(prData->u4OutputPath);
				} else {
					subtitle_osd_palette_init(st_addr,  prData);
				}
			}
		} else {
			FB_PRINT(FB_LOG_LVL_DBG, "", "STIOC_SETOSD_8_BIT_PALETTE src : %d %d pa %p\n",
				 osd_info.width, osd_info.height , (void *)st_addr);
			subtitle_osd_palette_init(st_addr,  prData);
		}
	}
	break;

	case STIOC_SET_SUB_FLAG:
		if (copy_from_user(&subHideFlag, (int *)arg, sizeof(int))) {
			ret = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "copy_from_user return %d iocode %d\n", ret, cmd);
		}

		break;

	case  STIOC_SETOSD_HIDE:
		FB_PRINT(FB_LOG_LVL_DBG, "", "HIDE OSD  .......\n");
		if (!access_ok(VERIFY_READ, arg, sizeof(OSD_DATA_T))) {
			ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "access_ok return %d iocode %d\n", ret, cmd);
			break;
		}
		prData = (OSD_DATA_T *)arg;

		if (prData->device_name == USB) {
			if (sub_fr_param.u4Flags == VDP_FRAMELOCK_MODE) {
				subtitle_osd_off(sub_fr_param.u4PreIdx);
			}

			subtitle_osd_off(sub_fr_param.u4Idx);
		} else {
			subtitle_osd_off(prData->u4OutputPath);
		}

		break;

	/* display setting added by MTK*/
	case DISPLAY_GET_ROTATE:
		if (MZ_FAILURE == MetaZone_Read(MZ_DISPLAY_ROTATE, &rotate)) {
			FB_PRINT(FB_LOG_LVL_ERR, "", "Read MetaZone MZ_DISPLAY_ROTATE failed..\r\n");
			rotate = -1;
		}
		FB_PRINT(FB_LOG_LVL_INFO, "", "get display rotate is %d, (0:0, 1:90, 2:180, 3:270)\n", rotate);
		if (copy_to_user((int *)arg, &rotate, sizeof(int))) {
			ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "copy_to_user return %d iocode %d\n", ret, cmd);
		}
		break;
	case DISPLAY_SET_LVDS_SSC:
		if (copy_from_user(&lvds_ssc_cfg, (struct LVDS_SSC *)arg, sizeof(struct LVDS_SSC))) {
			ret = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "copy_from_user return %d iocode %d\n", ret, cmd);
			break;
		}
		LvdsSscConfig(lvds_ssc_cfg.dir, lvds_ssc_cfg.freq, lvds_ssc_cfg.range);
		FB_PRINT(FB_LOG_LVL_INFO, "", "LvdsSscConfig done, dir:%d, freq:%d, range:%d\n", lvds_ssc_cfg.dir, lvds_ssc_cfg.freq, lvds_ssc_cfg.range);
		break;
	case DISPLAY_GET_DITHER:
		tconvalue = TconGetDither();
		if (copy_to_user((int *)arg, &tconvalue, sizeof(int))) {
			ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "copy_to_user return %d iocode %d\n", ret, cmd);
		}
		break;

	case DISPLAY_SET_DITHER:
		TconSetDither(1, 2, arg);
		ret = MetaZone_Write(MZ_DISPLAY_DITHER, arg);
		if (ret) {
			FB_PRINT(FB_LOG_LVL_ERR, "", "MetaZone_Write MZ_DISPLAY_DITHER failed %d\n", arg);
		}
		break;

	case DISPLAY_SET_DITHER_DISABLE:
		TconSetDither(0, 2, 2);
		break;

	case DISPLAY_SET_CONTRAST:
		vTconSetContrast(arg);
		ret = MetaZone_Write(MZ_DISPLAY_CONTRAST, arg);
		if (ret) {
			FB_PRINT(FB_LOG_LVL_ERR, "", "MetaZone_Write MZ_DISPLAY_CONTRAST failed %d\n", arg);
		}
		break;

	case DISPLAY_SET_BRIGNTNESS:
		vTconSetBrightness(arg);
		FB_PRINT(FB_LOG_LVL_INFO, "", "arg = 0x%x  .......\n", (unsigned int)arg);
		ret = MetaZone_Write(MZ_DISPLAY_BRIGHTNESS, arg);
		if (ret) {
			FB_PRINT(FB_LOG_LVL_ERR, "", "MetaZone_Write MZ_DISPLAY_BRIGHTNESS failed %d\n", arg);
		}
		break;

	case DISPLAY_SET_SATURATION:
		vTconSetSaturation(arg);
		ret = MetaZone_Write(MZ_DISPLAY_SATURATION, arg);
		if (ret) {
			FB_PRINT(FB_LOG_LVL_ERR, "", "MetaZone_Write MZ_DISPLAY_SATURATION failed %d\n", arg);
		}
		break;

	case DISPLAY_SET_YGAIN:
		vTconSetYGain(arg);
		break;

	case DISPLAY_SET_UGAIN:
		vTconSetUGain(arg);
		break;

	case DISPLAY_SET_VGAIN:
		vTconSetVGain(arg);
		break;

	case DISPLAY_SET_GAMMA:
		if (!access_ok(VERIFY, arg, sizeof(__u32))) {
			ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "copy_from_user return %d iocode %d\n", ret, cmd);
			break;
		}
		vPanelSetGamma((__u8 *)arg);
		ret = MetaZone_WriteBinary(MZ_DISPLAY_GAMMA, (const char *)arg, 64);
		if (ret) {
			FB_PRINT(FB_LOG_LVL_ERR, "", "MetaZone_WriteBinary MZ_DISPLAY_GAMMA failed\n");
		}
		break;

	case DISPLAY_SET_HUE:
		vTconSetHue(arg);
		break;


	case DISPLAY_GET_CONTRAST:
		tconvalue = TconGetContrast();
		if (copy_to_user((int *)arg, &tconvalue, sizeof(int))) {
			ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "copy_to_user return %d iocode %d\n", ret, cmd);
		}
		break;

	case DISPLAY_GET_BRIGNTNESS:
		tconvalue = TconGetBrightness();
		if (copy_to_user((int *)arg, &tconvalue, sizeof(int))) {
			ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "copy_to_user return %d iocode %d\n", ret, cmd);
		}
        break;

	case DISPLAY_GET_SATURATION:
		tconvalue = TconGetSaturation();
		if (copy_to_user((int *)arg, &tconvalue, sizeof(int))) {
			ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "copy_to_user return %d iocode %d\n", ret, cmd);
		}
		break;

	case DISPLAY_GET_YGAIN:
		tconvalue = TconGetYGain();
		if (copy_to_user((int *)arg, &tconvalue, sizeof(int))) {
			ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "copy_to_user return %d iocode %d", ret, cmd);
		}
		break;

	case DISPLAY_GET_UGAIN:
		tconvalue = TconGetUGain();
		if (copy_to_user((int *)arg, &tconvalue, sizeof(int))) {
			ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "copy_to_user return %d iocode %d\n", ret, cmd);
		}
		break;

	case DISPLAY_GET_VGAIN:
		tconvalue = TconGetVGain();
		if (copy_to_user((int *)arg, &tconvalue, sizeof(int))) {
			ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "copy_to_user return %d iocode %d\n", ret, cmd);
		}
		break;

	case DISPLAY_GET_GAMMA:
		TconGetGamma(PanelGamma);
		if (copy_to_user((__u8 *)arg, PanelGamma, GAMMA_LENGTH)) {
			ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "copy_to_user return %d iocode %d\n", ret, cmd);
		}
		break;

	case DISPLAY_GET_HUE:
		tconvalue = TconGetHue();
		if (copy_to_user((int *)arg, &tconvalue, sizeof(int))) {
			ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "copy_to_user return %d iocode %d\n", ret, cmd);
		}
		break;

         case BUFFER_FENCE_SYNC:
            copy_from_user(&rSync, arg, sizeof(FB_SYNC_T));
            vBufferFenceSync(&rSync, &_rSyncData);
            copy_to_user(arg, &rSync, sizeof(FB_SYNC_T));
            break;




	case YUV420_BLOCK_TO_ARGB8888:
		if (!access_ok(VERIFY_READ, arg, sizeof(YUV420BLOCK_TO_ARGB8888_BUF_T))) {
			ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "access_ok return %d iocode %d\n", ret, cmd);
			break;
		}
		vYUV420_Block_TO_ARGB8888((void *)arg);
		break;
		
	case YUV420_BLOCK_TO_NV12:
		if (!access_ok(VERIFY_READ, arg, sizeof(YUV420BLOCK_TO_NV12_BUF_T))) {
			ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "access_ok return %d iocode %d\n", ret, cmd);
			break;
		}
		vYUV420_Block_TO_NV12((void *)arg);
		break;
		

	case STIOC_SET_PLANE_ORDER:
		if (!access_ok(VERIFY_READ, arg, sizeof(__u32))) {
			ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "access_ok return %d iocode %d\n", ret, cmd);
			break;
		}
		vPmxHalSetPlaneOrder(PMX_1, *(__u32 *)arg);
		break;

	case STIOC_RESET_PLANE_ORDER:
		vPmxHalSetPlaneOrder(PMX_1, PMX_PLANE_ORDER);
		break;

	case DISPLAY_OSD8_MIX2DVD: {
		bool karg;

		if (0 == get_user(karg, (bool __user *)arg)) {
			OSD_R_MIX2DVD(karg);
		}

		ret = -1;
		break;
	}

	case DISPLAY_FR_FOLLOW: {
		ret = FrontRearDisplayFollow(arg);
		break;
	}

	case DISPLAY_GET_REAR_ON: {
		int karg = 0;
		karg = (int)fgTveHalGetEn();
		if (copy_to_user((int *)arg, &karg, sizeof(int))) {
			ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "DISPLAY_GET_REAR_ON copy_to_user fail %d\n", karg);
		} else {
			FB_PRINT(FB_LOG_LVL_INFO, "", "DISPLAY_GET_REAR_ON %d\n", karg);
		}
		break;
	}
#ifdef CONFIG_ATC_OS_linux

	case DISPLAY_GET_LOGO_BPP:
		logo_bpp = g_rFBConfig.u4LogoBpp;
		if (copy_to_user((int *)arg, &logo_bpp, sizeof(int))) {
			ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "", "copy_to_user return %d iocode %d\n", ret, cmd);
		}
		FB_PRINT(FB_LOG_LVL_ERR, "", "mtkfb get logo_bpp is %d",logo_bpp);
		break;
#endif
	default:
		//FB_PRINT(FB_LOG_LVL_ERR, "", "not support cmd=%x\n", cmd);
		ret = VDP_IOControl(cmd , (void *)arg, (void *)arg); // do not use in linux project*/
		break;
	}

	return ret;
}
/*end*/
static struct fb_ops mtk_fb_ops = {
	.owner          = THIS_MODULE,
	.fb_check_var   = mtk_fb_check_var,
	.fb_set_par     = mtk_fb_set_par,
	.fb_setcolreg   = mtk_fb_setcolreg,
	.fb_pan_display = mtk_fb_pan_display,

	/* These are generic software based fb functions */
	/*    .fb_fillrect	= sys_fillrect, // marked by mtk94039*/
	.fb_copyarea	= sys_copyarea,
	.fb_imageblit	= sys_imageblit,
	/*add by mtk94020 for vdp*/
	.fb_ioctl		= mtk_fb_ioctl,
	/*.fb_mmap		= vdp_mmap,	//only for test*/
	/*end add*/
};

/**
 * mtk_fb_alloc_memory() - allocate display memory for framebuffer window
 *
 * Allocate memory for the given framebuffer.
 */
static int mtk_fb_alloc_memory(struct fb_info *fb)
{
	unsigned real_size, virt_size, size;

	FB_PRINT(FB_LOG_LVL_DBG, "", "allocating memory for display\n");

	real_size = fb->var.xres * fb->var.yres;
	virt_size = fb->var.xres_virtual * fb->var.yres_virtual;
	FB_PRINT(FB_LOG_LVL_DBG, "", "real_size=%u (%u.%u), virt_size=%u (%u.%u)\n",
		 real_size, fb->var.xres, fb->var.yres,
		 virt_size, fb->var.xres_virtual, fb->var.yres_virtual);

	size = (real_size > virt_size) ? real_size : virt_size;
	size *= fb->var.bits_per_pixel;
	size /= 8;

	fb->fix.smem_len = size;

	FB_PRINT(FB_LOG_LVL_DBG, "", "want %u(%x) bytes for display memory\n", size, size);

	fb->fix.smem_start = fbm_base;

	fb->screen_base = (char *)FB_PHYSICAL_TO_VIRTUAL(fb->fix.smem_start);

	if (!fb->screen_base) {
		FB_PRINT(FB_LOG_LVL_DBG, "", "can't alloc video memory start %x\n", (unsigned int)fb->fix.smem_start);
		return -ENOMEM;
	}

	FB_PRINT(FB_LOG_LVL_INFO, "", "framebuffer's pa=%x\n", (unsigned int)fbm_base);

	if (CLEAR_FB) {
		memset(fb->screen_base, 0x0, PAGE_ALIGN(size));
	}

	FB_PRINT(FB_LOG_LVL_DBG, "", "mapped %x to %p\n", (unsigned int)fb->fix.smem_start, fb->screen_base);
	display_init(fb->fix.smem_start, fb->fix.smem_start + PAGE_ALIGN(size) / 2, fb->var.xres, fb->var.yres);

	return 0;
}

static void mtk_fb_free_memory(struct fb_info *fb)
{
	display_uninit();

	if (fb->screen_base) {
		dma_free_writecombine(NULL, PAGE_ALIGN(fb->fix.smem_len), fb->screen_base
			, (dma_addr_t)fb->fix.smem_start);
	}
}

#if SHOW_GIS_FPS

static ssize_t fb_gisfps_show(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	ssize_t size;

	size = sprintf(buf, "%d %d", g_printk_gis, (int)g_count_gis);
	FB_PRINT(FB_LOG_LVL_INFO, "", "g_printk_gis=%d ,g_count_gis=%d\n", g_printk_gis, (int)g_count_gis);

	return size;
}

static ssize_t fb_gisfps_store(struct device *dev, struct device_attribute *attr,
			       const char *buf, size_t count)
{
	sscanf(buf, "%d %d", &g_printk_gis, &g_count_gis);
	FB_PRINT(FB_LOG_LVL_INFO, "", "g_printk_gis=%d, g_count_gis=%d\n", g_printk_gis, (int)g_count_gis);

	return count;
}

void gamma_info_dump(uint8_t *gamma_table)
{
	int i, len = 0;
	char buf[BUF_LEN] = {0};

	FB_PRINT(FB_LOG_LVL_INFO, "", "IN\n");

	for (i = 0; i < GAMMA_TABLE_64_SIZE; i++) {
		len += snprintf(buf + len, BUF_LEN, "%2x, ", gamma_table[i]);
		if ((i + 1) % 8 == 0) {
			FB_PRINT(FB_LOG_LVL_INFO, "", "[%d][%d]%s\n", i, i + 1, buf);
			memset(buf, 0, sizeof(buf));
			len = 0;
		}
	}
	FB_PRINT(FB_LOG_LVL_INFO, "", "OUT\n");
}

ssize_t show_gamma_table(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	ssize_t len = 0;
	unsigned int i = 0;

	FB_PRINT(FB_LOG_LVL_INFO, "", "IN\n");

	UNUSED(dev);
	UNUSED(attr);

	len += snprintf(buf, PAGE_SIZE, "%c", '{');
	for (i = 0; i < GAMMA_TABLE_64_SIZE; i++) {
		len += snprintf(buf + len, PAGE_SIZE, "%2x, ", disp_gamma_dev.disp_gamma[i]);
	}
	if (len > 2) {
		len = len - 2;
	}
	len += snprintf(buf + len, PAGE_SIZE, "%c", '}');

	gamma_info_dump(disp_gamma_dev.disp_gamma);
	FB_PRINT(FB_LOG_LVL_INFO, "", "OUT\n");

	return len;
}

static ssize_t store_gamma_table(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	char buf_tmp[10];
	ssize_t offset = 0;
	unsigned int i = 0;
	int ret = -EINVAL;

	UNUSED(dev);
	UNUSED(attr);

	FB_PRINT(FB_LOG_LVL_INFO, "", "[%s] buf len = %lu, buf:%s\n", __func__, strlen(buf), buf);

	for (i = 0; i < GAMMA_TABLE_64_SIZE; i++) {
		memset(buf_tmp, 0, sizeof(buf_tmp));

		memcpy(buf_tmp, buf + offset, 2);
		ret = sscanf(&buf_tmp[0], "%2x", &disp_gamma_dev.disp_gamma[i]);
		if (ret <= 0) {
			FB_PRINT(FB_LOG_LVL_ERR, "", "get gamma_table error!, i=%d, [%s], ret=%d\n", i, buf_tmp, ret);
			break;
		}
		offset += 2;

		if ((i != GAMMA_TABLE_64_SIZE) && (*(buf + offset) != ' ')) {
			FB_PRINT(FB_LOG_LVL_INFO, "", "input buf[%zd] = %c, buf[%zd] = %c\n", offset, *(buf + offset), offset + 1, *(buf + offset + 1));
			break;
		}
		offset += 1;
	}

	vPanelSetGamma(disp_gamma_dev.disp_gamma);

	ret = MetaZone_WriteBinary(MZ_DISPLAY_GAMMA, (const char *)disp_gamma_dev.disp_gamma, 64);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "MetaZone_WriteBinary MZ_DISPLAY_GAMMA failed\n");
	}

	return (ssize_t)count;
}


static DEVICE_ATTR(gisfps, S_IWUSR | S_IRUGO, fb_gisfps_show, fb_gisfps_store);
static DEVICE_ATTR(gamma_table, 0644, show_gamma_table, store_gamma_table);

#define FB_SYSFS_FLAG_GISFPS_ATTR 2
#endif

#if MONITOR_VDO_FPS
static struct timeval start_t[2];

static uint fb_vdofps_cal(uint idx)
{
	double vdofps = 0;
	struct timeval end_t = {0, 0};
	long usec = 0;

	if (idx > 2) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[VDO_FPS] fb_vdofps_cal error vdo id not support %d\n", idx);
		return vdofps;
	}

	if (start_t[idx].tv_sec && start_t[idx].tv_usec) {
		do_gettimeofday(&end_t);
		usec = (end_t.tv_sec - start_t[idx].tv_sec) * 1000000 + (end_t.tv_usec - start_t[idx].tv_usec);

		if (usec) {
			vdofps = (double)(vdo_flip_cnt[idx]) * 10000000.0 / (double)usec;
			FB_PRINT(FB_LOG_LVL_INFO, "", "[VDO_FPS] vdo %d Fps %f \r\n", (int)idx, vdofps);
		} else {
			FB_PRINT(FB_LOG_LVL_INFO, "", "[VDO_FPS] Error vdo %d usec = %d\n", (int)idx, (int) usec);
		}
	} else {
		FB_PRINT(FB_LOG_LVL_INFO, "", "[VDO_FPS] Pls enable vdo %d Fps first\r\n", (int)idx);
	}

	return vdofps;
}

static void fb_vdofps_ctl(uint idx)
{
	if (idx > 2) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[VDO_FPS] fb_vdofps_ctl error vdo id not support %d\n", (int)idx);
		return;
	}

	if (vdo_fps_en[idx] == 1) {
		vdo_flip_cnt[idx] = 0;
		do_gettimeofday(&start_t[idx]);
		FB_PRINT(FB_LOG_LVL_INFO, "", "[VDO_FPS] vdo=%d, vdo_fps_en=%d, vdo_flip_cnt=%d\n", (int)idx
			, (int)vdo_fps_en[idx], (int)vdo_flip_cnt[idx]);
	} else if (vdo_fps_en[idx] == 2) {
		fb_vdofps_cal(idx);
	} else {
		fb_vdofps_cal(idx);
		start_t[idx].tv_sec = 0;
		start_t[idx].tv_usec = 0;
	}
}

static ssize_t fb_vdoffps_show(struct device *dev, struct device_attribute *attr,
			       char *buf)
{
	ssize_t size;

	size = sprintf(buf, "vdo front %d fps\n", fb_vdofps_cal(0));

	return size;
}

static ssize_t fb_vdoffps_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	sscanf(buf, "%d", &vdo_fps_en[0]);
	fb_vdofps_ctl(0);

	return count;
}

static ssize_t fb_vdorfps_show(struct device *dev, struct device_attribute *attr,
			       char *buf)
{
	ssize_t size;

	size = sprintf(buf, "vdo rear %d fps\n", fb_vdofps_cal(1));

	return size;
}

static ssize_t fb_vdorfps_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	sscanf(buf, "%d", &vdo_fps_en[1]);
	fb_vdofps_ctl(1);

	return count;
}

static DEVICE_ATTR(vdoffps, S_IWUSR | S_IRUGO, fb_vdoffps_show, fb_vdoffps_store);
static DEVICE_ATTR(vdorfps, S_IWUSR | S_IRUGO, fb_vdorfps_show, fb_vdorfps_store);
#define FB_SYSFS_FLAG_VDOFFPS_ATTR 4
#define FB_SYSFS_FLAG_VDORFPS_ATTR 8
#endif

static ssize_t vdp_loglevel_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t size;

	size = sprintf(buf, "fb_log_lvl = %u\r\n", (unsigned int)fb_log_lvl);

	return size;
}
static ssize_t vdp_loglevel_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int nr = sscanf(buf, "%d", &fb_log_lvl);

	FB_PRINT(FB_LOG_LVL_INFO, "", "fb_loglevel_store: nr %d, level %d\r\n", nr, (int)fb_log_lvl);

	return (ssize_t)count;
}


static DEVICE_ATTR(loglevel, S_IWUSR | S_IRUGO, vdp_loglevel_show, vdp_loglevel_store);
#define FB_SYSFS_VDP_LOGLEVEL 16

void __iomem *osdf_reg;
void __iomem *osd1_reg;
void __iomem *osd2_reg;
void __iomem *osd3_reg;
void __iomem *osd4_reg;
void __iomem *osd5_reg;
void __iomem *osdr_reg;
void __iomem *osdr1_reg;
void __iomem *osdr2_reg;
void __iomem *osdr3_reg;
void __iomem *fmtf_reg;
void __iomem *vdof_reg;
void __iomem *fmtr_reg;
void __iomem *vdor_reg;
void __iomem *scl_reg;
void __iomem *sclf_reg;
void __iomem *mix_reg;
void __iomem *tlcp_reg;
void __iomem *togc_reg;
void __iomem *tcon_reg;
void __iomem *ddds_reg;
void __iomem *lvds_reg;
void __iomem *lvdsa_reg;
void __iomem *tve_reg;
void __iomem *tvebk1_reg;
void __iomem *tvebk2_reg;
unsigned int fmtf_irq;
unsigned int fmtr_irq;
unsigned int scl_irq;
struct clk *clk_imgrsz;
struct clk *clk_osdrsz;
struct clk *clk_lvds;
struct clk *clk_scl;
struct clk *clk_osdf;
struct clk *clk_osdr;
struct clk *clk_tcon;
struct clk *clk_fmtf;
struct clk *clk_fmtr;
struct clk *clk_tve;
struct clk *clk_dvd2ap;
struct clk *clk_osd1;
struct clk *clk_osd2;
struct clk *clk_osd3;
struct clk *clk_osd4;
struct clk *clk_osdr2;
struct clk *clk_osdr3;
struct clk *clk_scltg;
struct pinctrl *pinctrl_fb;
unsigned int fbm_base = 0;
#if defined(CONFIG_ATC_OS_android)
EXPORT_SYMBOL(fbm_base);
#endif
unsigned int fbm_size = 0;
void * fbm_va = NULL;
unsigned int g_imgreaz_base = 0;
unsigned int g_imgreaz_size = 0;

#if defined(CONFIG_ATC_OS_android)
__u32 _fb1Pa = 0, _fb1Va = 0, _fb1Size = 0;
EXPORT_SYMBOL(_fb1Pa);
EXPORT_SYMBOL(_fb1Va);
EXPORT_SYMBOL(_fb1Size);

static void set_fb1_param(__u32 base, __u32 va, __u32 size)
{
	_fb1Pa = base;
	_fb1Va = va;
	_fb1Size = size;
}
#endif

static int read_dts_data(struct platform_device *pdev)
{
	__u32 ret = -EINVAL;
        struct device_node *node;
        unsigned int property[2];
        unsigned int base_pa, base_va;
        unsigned int size;
	struct device_node *nd = pdev->dev.of_node;
	struct reserved_mem *fb_mem;

	osdf_reg = of_iomap(nd, 0);

	if (!osdf_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get osdf reg base address failed = %x \r\n"
			, (unsigned int)osdf_reg);
		goto err;
	}

	osd1_reg = of_iomap(nd, 1);

	if (!osd1_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get osd1 reg base address failed = %x \r\n"
			, (unsigned int)osd1_reg);
		goto err;
	}

	osd2_reg = of_iomap(nd, 2);

	if (!osd2_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get osd2 reg base address failed = %x \r\n"
			, (unsigned int)osd2_reg);
		goto err;
	}

	osd3_reg = of_iomap(nd, 3);

	if (!osd3_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get osd4 reg base address failed = %x \r\n"
			, (unsigned int)osd3_reg);
		goto err;
	}

	osd4_reg = of_iomap(nd, 4);

	if (!osd4_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get osdf reg base address failed = %x \r\n"
			, (unsigned int)osd4_reg);
		goto err;
	}

	osd5_reg = of_iomap(nd, 5);

	if (!osd5_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get osd5 reg base address failed = %x \r\n"
			, (unsigned int)osd5_reg);
		goto err;
	}

	osdr_reg = of_iomap(nd, 6);

	if (!osdr_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get osdr reg base address failed = %x \r\n"
			, (unsigned int)osdr_reg);
		goto err;
	}

	osdr1_reg = of_iomap(nd, 7);

	if (!osdr1_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get osdr1 reg base address failed = %x \r\n"
			, (unsigned int)osdr1_reg);
		goto err;
	}

	osdr2_reg = of_iomap(nd, 8);

	if (!osdr2_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get osdr2 reg base address failed = %x \r\n"
			, (unsigned int)osdr2_reg);
		goto err;
	}

	osdr3_reg = of_iomap(nd, 9);

	if (!osdr3_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get osdr3 reg base address failed = %x \r\n"
			, (unsigned int)osdr3_reg);
		goto err;
	}

	fmtf_reg = of_iomap(nd, 10);

	if (!fmtf_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get fmtf reg base address failed = %x \r\n"
			, (unsigned int)fmtf_reg);
		goto err;
	}

	vdof_reg = of_iomap(nd, 11);

	if (!vdof_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get vdof reg base address failed = %x \r\n"
			, (unsigned int)vdof_reg);
		goto err;
	}

	fmtr_reg = of_iomap(nd, 12);

	if (!fmtr_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get fmtr reg base address failed = %x \r\n"
			, (unsigned int)fmtr_reg);
		goto err;
	}

	vdor_reg = of_iomap(nd, 13);

	if (!vdor_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get vdor reg base address failed = %x \r\n"
			, (unsigned int)vdor_reg);
		goto err;
	}

	scl_reg = of_iomap(nd, 14);

	if (!scl_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get scl reg base address failed = %x \r\n"
			, (unsigned int)scl_reg);
		goto err;
	}

	sclf_reg = of_iomap(nd, 15);

	if (!sclf_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get sclf reg base address failed = %x \r\n"
			, (unsigned int)sclf_reg);
		goto err;
	}

	mix_reg = of_iomap(nd, 16);

	if (!mix_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get mix reg base address failed = %x \r\n"
			, (unsigned int)mix_reg);
		goto err;
	}

	tlcp_reg = of_iomap(nd, 17);

	if (!tlcp_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get tlcp reg base address failed = %x \r\n"
			, (unsigned int)tlcp_reg);
		goto err;
	}

	togc_reg = of_iomap(nd, 18);

	if (!togc_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get togc reg base address failed = %x \r\n"
			, (unsigned int)togc_reg);
		goto err;
	}

	tcon_reg = of_iomap(nd, 19);

	if (!tcon_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get tcon reg base address failed = %x \r\n"
			, (unsigned int)tcon_reg);
		goto err;
	}

	ddds_reg = of_iomap(nd, 20);

	if (!ddds_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get ddds reg base address failed = %x \r\n"
			, (unsigned int)ddds_reg);
		goto err;
	}

	lvds_reg = of_iomap(nd, 21);

	if (!lvds_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get lvds reg base address failed = %x \r\n"
			, (unsigned int)lvds_reg);
		goto err;
	}

	lvdsa_reg = of_iomap(nd, 22);

	if (!lvdsa_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get lvdsa reg base address failed = %x \r\n"
			, (unsigned int)lvdsa_reg);
		goto err;
	}

	tve_reg = of_iomap(nd, 23);

	if (!tve_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get tve reg base address failed = %x \r\n"
			, (unsigned int)tve_reg);
		goto err;
	}

	tvebk1_reg = of_iomap(nd, 24);

	if (!tvebk1_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get tve bank1 reg base address failed = %x \r\n"
			, (unsigned int)tvebk1_reg);
		goto err;
	}

	tvebk2_reg = of_iomap(nd, 25);

	if (!tvebk2_reg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get tve bank2 reg base address failed = %x \r\n"
			, (unsigned int)tvebk2_reg);
		goto err;
	}

	fmtf_irq = irq_of_parse_and_map(nd, 0);

	if (fmtf_irq == NO_IRQ) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get fmtf irq failed = %d \r\n", fmtf_irq);
		goto err;
	}

	fmtr_irq = irq_of_parse_and_map(nd, 1);

	if (fmtr_irq == NO_IRQ) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get fmtr irq failed = %d \r\n", fmtr_irq);
		goto err;
	}

	scl_irq = irq_of_parse_and_map(nd, 2);

	if (scl_irq == NO_IRQ) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "[read_dts_data] get scl irq failed = %d \r\n", scl_irq);
		goto err;
	}

	clk_imgrsz = devm_clk_get(&pdev->dev, "imgrsz-clk");

	if (!clk_imgrsz) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get imgrsz clk failed %x\r\n", (unsigned int)clk_imgrsz);
		goto err;
	}

	clk_osdrsz = devm_clk_get(&pdev->dev, "osdrsz-clk");

	if (!clk_osdrsz) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get osdrsz clk failed %x\r\n", (unsigned int)clk_osdrsz);
		goto err;
	}

	clk_lvds = devm_clk_get(&pdev->dev, "lvds-clk");

	if (!clk_lvds) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get lvds clk failed %x\r\n", (unsigned int)clk_lvds);
		goto err;
	}

	clk_scl = devm_clk_get(&pdev->dev, "scl-clk");

	if (!clk_scl) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get scl clk failed %x\r\n", (unsigned int)clk_scl);
		goto err;
	}

	clk_osdf = devm_clk_get(&pdev->dev, "osdf-clk");

	if (!clk_osdf) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get osd clk failed %x\r\n", (unsigned int)clk_osdf);
		goto err;
	}

	clk_osdr = devm_clk_get(&pdev->dev, "osdr-clk");

	if (!clk_osdr) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get scl osdr failed %x\r\n", (unsigned int)clk_osdr);
		goto err;
	}

	clk_tcon = devm_clk_get(&pdev->dev, "fpd-clk");

	if (!clk_tcon) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get tcon clk failed %x\r\n", (unsigned int)clk_tcon);
		goto err;
	}

	clk_fmtf = devm_clk_get(&pdev->dev, "fmtf-clk");

	if (!clk_fmtf) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get scl fmtf failed %x\r\n", (unsigned int)clk_fmtf);
		goto err;
	}

	clk_fmtr = devm_clk_get(&pdev->dev, "fmtr-clk");

	if (!clk_fmtr) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get fmtr clk failed %x\r\n", (unsigned int)clk_fmtr);
		goto err;
	}

	clk_tve = devm_clk_get(&pdev->dev, "tve-clk");

	if (!clk_tve) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get tve clk failed %x\r\n", (unsigned int)clk_tve);
		goto err;
	}

	clk_dvd2ap = devm_clk_get(&pdev->dev, "dvd2ap-clk");

	if (!clk_dvd2ap) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get dvd2ap clk failed %x\r\n", (unsigned int)clk_dvd2ap);
		goto err;
	}

	clk_osd1 = devm_clk_get(&pdev->dev, "osd1-clk");

	if (!clk_osd1) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get osd1 clk failed %x\r\n", (unsigned int)clk_osd1);
		goto err;
	}

	clk_osd2 = devm_clk_get(&pdev->dev, "osd2-clk");

	if (!clk_osd2) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get osd2 clk failed %x\r\n", (unsigned int)clk_osd2);
		goto err;
	}

	clk_osd3 = devm_clk_get(&pdev->dev, "osd3-clk");

	if (!clk_osd3) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get osd3 clk failed %x\r\n", (unsigned int)clk_osd3);
		goto err;
	}

	clk_osd4 = devm_clk_get(&pdev->dev, "osd4-clk");

	if (!clk_osd4) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get osd4 clk failed %x\r\n", (unsigned int)clk_osd4);
		goto err;
	}

	clk_osdr2 = devm_clk_get(&pdev->dev, "osdr2-clk");

	if (!clk_osdr2) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get osdr2 clk failed %x\r\n", (unsigned int)clk_osdr2);
		goto err;
	}

	clk_osdr3 = devm_clk_get(&pdev->dev, "osdr3-clk");

	if (!clk_osdr3) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get osdr3 clk failed %x\r\n", (unsigned int)clk_osdr3);
		goto err;
	}

	clk_scltg = devm_clk_get(&pdev->dev, "scl-tg-clk");

	if (!clk_scltg) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get scltg clk failed %x\r\n", (unsigned int)clk_scltg);
		goto err;
	}

	clk_prepare_enable(clk_osdf);
	clk_prepare_enable(clk_osdr);
	clk_prepare(clk_osd1);
	clk_prepare(clk_osd2);
	clk_prepare(clk_osd3);
	clk_prepare(clk_osd4);
	clk_prepare(clk_osdr2);
	clk_prepare(clk_osdr3);
	clk_prepare(clk_imgrsz);
	clk_prepare(clk_osdrsz);
	clk_prepare(clk_lvds);
	clk_prepare(clk_tcon);
	clk_prepare(clk_scltg);
	clk_prepare(clk_scl);
	clk_prepare(clk_fmtf);
	clk_prepare(clk_fmtr);
	clk_prepare(clk_tve);

	pinctrl_fb = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(pinctrl_fb)) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get pinctrl failed %x\r\n", (unsigned int)pinctrl_fb);
		goto err;
	}
	FB_PRINT(FB_LOG_LVL_INFO, "", "get pinctrl %x\r\n", (unsigned int)pinctrl_fb);

	of_reserved_mem_device_init( &(pdev->dev));
	fb_mem = (struct reserved_mem *)(pdev->dev.cma_area);
	if (!fb_mem) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get memory failed %x\r\n", (unsigned int)fb_mem);
		goto err;
	}
	fbm_base = fb_mem->base;
	fbm_size = fb_mem->size;
	if ((fbm_base == 0) || (fbm_size == 0)) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get memory failed base %x size %x\r\n", fbm_base, fbm_size);
		goto err;
	}
	fbm_va = ioremap(fbm_base, fbm_size);
	if (fbm_va == NULL) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "get ioremap va error, memory name %s, base 0x%x, size %d\r\n"
			, fb_mem->name, fbm_base, fbm_size / SZ_1M);
		goto err;
	}
	FB_PRINT(FB_LOG_LVL_INFO, "", "get memory name %s, base 0x%x, va 0x%x, size %d\r\n", fb_mem->name
		, fbm_base, (unsigned int)fbm_va, fbm_size / SZ_1M);


        node = of_find_compatible_node(NULL,NULL,"atc-imageresize");
	if (node)
	{
		/*base_va = (unsigned long)of_iomap(node, 0);
		if (0 == base_va) {
			FB_PRINT(FB_LOG_LVL_ERR, "", "imgresz buffer of_iomap fail\r\n");
			goto err;
		}*/
		if (of_property_read_u32_array(node, "reg", (u32 *)property, 2)) {
			FB_PRINT(FB_LOG_LVL_ERR, "", "get imgresz buffer reserved memory node reg info fail\r\n");
			goto err;
		}
		base_pa = property[0];
		size = property[1];
		g_imgreaz_base = base_pa;
		g_imgreaz_size = size;
		base_va = (unsigned long)phys_to_virt(base_pa);;
		if (0 == base_va) {
			FB_PRINT(FB_LOG_LVL_ERR, "", "imgresz buffer of_iomap fail\r\n");
			goto err;
		}
		FB_PRINT(FB_LOG_LVL_INFO, "", "imgresz buffer base va:%x, size:%x, pa:%x\n", base_va, size, base_pa);
		set_pool_param(base_pa, base_va, size);
	} else
	{
		FB_PRINT(FB_LOG_LVL_ERR, "", "can not find imgresz buffer reserved memory node!!\r\n");
		goto err;
	}
#if defined(CONFIG_ATC_OS_android)
	node = of_find_compatible_node(NULL,NULL,"atc-fb1reserved");
	if (node)
	{
		base_va = (unsigned long)of_iomap(node, 0);
		if (0 == base_va) {
			FB_PRINT(FB_LOG_LVL_ERR, "", "fb1 buffer of_iomap fail\r\n");
			goto err;
		}
		if (of_property_read_u32_array(node, "reg", (u32 *)property, 2)) {
			FB_PRINT(FB_LOG_LVL_ERR, "", "get fb1 buffer reserved memory node reg info fail\r\n");
			goto err;
		}
		base_pa = property[0];
		size = property[1];
		FB_PRINT(FB_LOG_LVL_INFO, "", "fb1 buffer base va:%x, size:%x, pa:%x\n", base_va, size, base_pa);
		set_fb1_param(base_pa, base_va, size);
	} else
	{
		FB_PRINT(FB_LOG_LVL_ERR, "", "can not find fb1 buffer reserved memory node!!\r\n");
		goto err;
	}
#endif
	ret = 0;
err:
	return ret;
}

#if defined(CONFIG_ATC_OS_android)
#define DUMP_DAL_MEM
#endif
#ifdef DUMP_DAL_MEM
static ssize_t fb_dal_show(struct device *dev, struct device_attribute *attr,
			      char *buf)
{

	return 0;
}

static ssize_t fb_dal_store(struct device *dev, struct device_attribute *attr,
			       const char *buf, size_t count)
{
	unsigned char string[20] = "";
	void *dal_va = NULL;
	unsigned int dal_size = 1024*600*2;
	struct file *fp = NULL;
	mm_segment_t old_fs = 0;
	loff_t pos = 0;
	unsigned char szDumpFile[256] = "";
	int ret = 0;

	dal_va = fbm_va + (LCD_GetScreenWidth() * LCD_GetScreenHeight() * 4 * 12);
        FB_PRINT(FB_LOG_LVL_INFO, "", "dal_va:0x%x, fbm_va:0x%x\n", dal_va, fbm_va);
	sscanf(buf, "%s", string);
        FB_PRINT(FB_LOG_LVL_INFO, "", "string:%s\n", string);

	if (strcmp(string, "dumpdal") == 0) {
		old_fs = get_fs();
		set_fs(KERNEL_DS);

		sprintf(szDumpFile, "/storage/ext_sdcard2/dal.bin");
		fp = filp_open(szDumpFile, O_CREAT | O_RDWR, 0777);
		if (IS_ERR(fp)) {
			ret = PTR_ERR(fp);
			FB_PRINT(FB_LOG_LVL_INFO, "", "dump dal memory open file %s error!\n", szDumpFile);
			return false;
		}

		pos = 0;

		ret = vfs_write(fp, (char __user *)dal_va, dal_size, &pos);
		filp_close(fp, NULL);
		set_fs(old_fs);

		FB_PRINT(FB_LOG_LVL_INFO, "", "dump dal memory done\n");
	} else {
                FB_PRINT(FB_LOG_LVL_ERR, "", "dump dal memory CMD error:%s\n", string);
	}

	return count;
}

static DEVICE_ATTR(dal, S_IWUSR | S_IRUGO, fb_dal_show, fb_dal_store);
#define FB_SYSFS_FLAG_DALDUMP_ATTR 32
#endif

//share global variable
extern int g_vdp_dump;
static ssize_t fb_vdp_show(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	return 0;
}

static ssize_t fb_vdp_store(struct device *dev, struct device_attribute *attr,
			       const char *buf, size_t count)
{
	unsigned char cmd[20] = "";
	unsigned char path[40] = "";
	unsigned int num = 0;
	unsigned int io = 0;
	int ret = 0;

	sscanf(buf, "%s %s %d %d", (unsigned char *)&cmd, (unsigned char *)&path, (unsigned int *)&num, (unsigned int *)&io);
	if (strcmp(cmd, "setDumpVdp=1") == 0) {
		g_vdp_dump = 1;
		FB_PRINT(FB_LOG_LVL_INFO, "VDP", "dump function open now\n");
	} else if (strcmp(cmd, "setDumpVdp=0") == 0) {
		g_vdp_dump = 0;
		FB_PRINT(FB_LOG_LVL_INFO, "VDP", "dump function close now\n");
	} else if ((strcmp(cmd, "dumpvdp") == 0) && (io == VDP_DUMP_IN || io == VDP_DUMP_OUT || io == VDP_DUMP_IO)) {
		if (0 == g_vdp_dump) {
			FB_PRINT(FB_LOG_LVL_ERR, "VDP", "dump function not open, please open first!!\n");
		} else {
			FB_PRINT(FB_LOG_LVL_INFO, "VDP", "*****************Dump Buffer Start*****************\n");
			if (io == VDP_DUMP_IO) {
				vVdpDumpTest(path, num, VDP_DUMP_IN);
				vVdpDumpTest(path, num, VDP_DUMP_OUT);
			} else {
				vVdpDumpTest(path, num, io);
			}
			FB_PRINT(FB_LOG_LVL_INFO, "VDP", "*****************Dump Buffer End*****************\n");
		}
	} else {
		FB_PRINT(FB_LOG_LVL_ERR, "VDP", "[fb_vdp_store] enter cmd error,cmd:%s \r\n", cmd);
	}

	return count;
}
static DEVICE_ATTR(vdp4dump, S_IWUSR | S_IRUGO, fb_vdp_show, fb_vdp_store);

extern char *saved_command_line;
void check_recovery_mode(void)
{
	//FB_PRINT(FB_LOG_LVL_INFO, "","check_recovery_mode read start \n");
	char * cmdline = saved_command_line;
	if(NULL == cmdline) {
		//FB_PRINT(FB_LOG_LVL_ERR,"", "check_recovery_mode null cmd line \n");
		return;
	}
	if(strstr(cmdline,"boot_device=2")) {
		//FB_PRINT(FB_LOG_LVL_INFO, ,"","check_recovery_mode read sucess \n");
		CLEAR_FB = 0;
	} else {
		//FB_PRINT(FB_LOG_LVL_INFO,"", "check_recovery_mode read fail \n");
	}
	return;
}

#define ROUND_UP_COUNT(Count, Pow2) (((Count) + (Pow2) - 1) & (~(((LONG)(Pow2)) - 1)))
extern int free_memblock_runtime(phys_addr_t phy_addr,phys_addr_t size);

/*static   int __devinit mtk_fb_probe(struct platform_device *pdev)*/
static   int mtk_fb_probe(struct platform_device *pdev)
{
	int ret = -ENOMEM;
	struct mtkfb *mtkfb;
	FB_SYNC_DATA_T *    mfd = &_rSyncData;
	char timeline_name[MAX_TIMELINE_NAME_LEN];
	__u32 imgresz_size = 0;
	__u32 release_size = 0;
	__u32 release_base = 0;
	static uint8_t panel_gamma[64] = {
		0, 1, 3, 5, 7, 9, 11, 13,
		15, 17, 20, 23, 25, 28, 31, 34,
		37, 40, 43, 47, 51, 56, 61, 66,
		71, 76, 82, 87, 92, 97, 102, 107,
		112, 118, 123, 128, 133, 138, 143, 148,
		153, 158, 163, 167, 172, 177, 182, 186,
		191, 195, 200, 204, 208, 212, 216, 220,
		224, 228, 232, 236, 240, 244, 248, 252
	};

	FB_PRINT(FB_LOG_LVL_INFO, "", "[wts] mtk_fb_probe color fmt %d\n", (int)COLOR_DEPTH_32_BIT);

	memcpy(disp_gamma_dev.disp_gamma, panel_gamma, sizeof(uint8_t) * GAMMA_TABLE_64_SIZE);
	MOD_VERSION_INFO(MMISC_MODE_NAME, MMISC_VER_MAJOR, MMISC_VER_MINOR, MMISC_VER_REV);
	check_recovery_mode();
	read_dts_data(pdev);

	LCM_GetConfig();

	imgresz_size = ROUND_UP_COUNT(_u4LCDWidth, 16) * ROUND_UP_COUNT(_u4LCDHeight << 1, 32) * 3 / 2;
	imgresz_size = ROUND_UP_COUNT(imgresz_size, 0x400);
	imgresz_size = imgresz_size * 3;
	release_base = g_imgreaz_base + imgresz_size;
	release_base = ROUND_UP_COUNT(release_base, 0x1000);
	release_size = g_imgreaz_base + g_imgreaz_size - release_base;
	FB_PRINT(FB_LOG_LVL_INFO, "", "[wts]  imgresz_size: 0x%x, g_imgreaz_base: 0x%x, g_imgreaz_size: 0x%x, release_size: 0x%x, release_base: 0x%x\n", imgresz_size, g_imgreaz_base, g_imgreaz_size, release_size, release_base);
	if ((g_imgreaz_base  != 0) && (g_imgreaz_size != 0) && (release_size > 0)) {
		//free_memblock_runtime((phys_addr_t)release_base, (phys_addr_t)release_size);
		FB_PRINT(FB_LOG_LVL_INFO, "", "[wts]  release base: 0x%x, release size: 0x%x\n", release_base, release_size);
	}

	fb = framebuffer_alloc(sizeof(struct mtkfb), NULL);

	if (!fb) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "Failed to allocate framebuffer device\n");
		ret = -ENOMEM;
		goto err_fb_alloc_failed;
	}

	mtkfb = fb->par;

	fb->fbops = &mtk_fb_ops;
	fb->flags = FBINFO_FLAG_DEFAULT;
	fb->pseudo_palette = mtkfb->cmap;
	fb->var = mtkfb_var;
	fb->fix = mtkfb_fix;

	ret = mtk_fb_alloc_memory(fb);

	if (ret) {
		goto err_fb_alloc_failed;
	}

	ret = fb_set_var(fb, &fb->var);

	if (ret) {
		goto err_fb_set_var_failed;
	}

	ret = register_framebuffer(fb);

	if (ret) {
		goto err_register_framebuffer_failed;
	}

	ret = os_device_create_file(fb->dev, &dev_attr_gamma_table);
	if (ret) {
		FB_PRINT(FB_LOG_LVL_ERR, "", "cannot create gamma_table dev file\n");
	}

#if SHOW_GIS_FPS
	fb->class_flag |= FB_SYSFS_FLAG_GISFPS_ATTR;
	ret = os_device_create_file(fb->dev, &dev_attr_gisfps);

	if (ret) {
		fb->class_flag &= ~FB_SYSFS_FLAG_GISFPS_ATTR;
		FB_PRINT(FB_LOG_LVL_ERR, "", "cannot create dev file\n");
	}
#endif

#if MONITOR_VDO_FPS
	fb->class_flag |= FB_SYSFS_FLAG_VDOFFPS_ATTR;
	ret = os_device_create_file(fb->dev, &dev_attr_vdoffps);

	if (ret) {
		fb->class_flag &= ~FB_SYSFS_FLAG_VDOFFPS_ATTR;
		FB_PRINT(FB_LOG_LVL_ERR, "", "cannot create dev file\n");
	}

	fb->class_flag |= FB_SYSFS_FLAG_VDORFPS_ATTR;
	ret = os_device_create_file(fb->dev, &dev_attr_vdorfps);

	if (ret) {
		fb->class_flag &= ~FB_SYSFS_FLAG_VDORFPS_ATTR;
		FB_PRINT(FB_LOG_LVL_ERR, "", "cannot create dev file\n");
	}

#endif
	fb->class_flag |= FB_SYSFS_VDP_LOGLEVEL;
	ret = os_device_create_file(fb->dev, &dev_attr_loglevel);

	if (ret) {
		fb->class_flag &= ~FB_SYSFS_VDP_LOGLEVEL;
		FB_PRINT(FB_LOG_LVL_ERR, "", "cannot create dev file\n");
	}
#ifdef DUMP_DAL_MEM
        fb->class_flag |= FB_SYSFS_FLAG_DALDUMP_ATTR;
	ret = os_device_create_file(fb->dev, &dev_attr_dal);

	if (ret) {
	        fb->class_flag &= ~FB_SYSFS_FLAG_DALDUMP_ATTR;
	        FB_PRINT(FB_LOG_LVL_ERR, "", "cannot create dal dev file\n");
	}
#endif
	//create vdp4dump node for dump
		ret = os_device_create_file(fb->dev, &dev_attr_vdp4dump);

		if (ret) {
				FB_PRINT(FB_LOG_LVL_ERR, "", "cannot create vdp4dump dev file\n");
		}

    //    
    snprintf(timeline_name, sizeof(timeline_name),  	"atc_fb_%d", 1);
    mfd->timeline = sw_sync_timeline_create(timeline_name);
    if (mfd->timeline == NULL) 
    {
    	FB_PRINT(FB_LOG_LVL_INFO, "", "%s: cannot create time line", __func__);
    	return -ENOMEM;
    } 
    else 
    {
    	mfd->timeline_value = 0;
    }

    mutex_init(&mfd->sync_mutex);
	FB_PRINT(FB_LOG_LVL_DBG, "", "probe successful\n");
	return 0;

err_register_framebuffer_failed:
err_fb_set_var_failed:
	framebuffer_release(fb);
err_fb_alloc_failed:
	FB_PRINT(FB_LOG_LVL_ERR, "", "probe failed %d\n", ret);
	return ret;
}

/*static int __devexit mtk_fb_remove(struct platform_device *pdev)*/
static int mtk_fb_remove(struct platform_device *pdev)
{
	if (fb) {
		os_device_remove_file(fb->dev, &dev_attr_gamma_table);

#if SHOW_GIS_FPS

		if (fb->class_flag & FB_SYSFS_FLAG_GISFPS_ATTR) {
			os_device_remove_file(fb->dev, &dev_attr_gisfps);
			fb->class_flag &= ~FB_SYSFS_FLAG_GISFPS_ATTR;
		}

#endif

#if MONITOR_VDO_FPS

		if (fb->class_flag & FB_SYSFS_FLAG_VDOFFPS_ATTR) {
			os_device_remove_file(fb->dev, &dev_attr_vdoffps);
			fb->class_flag &= ~FB_SYSFS_FLAG_VDOFFPS_ATTR;
		}

		if (fb->class_flag & FB_SYSFS_FLAG_VDORFPS_ATTR) {
			os_device_remove_file(fb->dev, &dev_attr_vdorfps);
			fb->class_flag &= ~FB_SYSFS_FLAG_VDORFPS_ATTR;
		}

#endif

		if (fb->class_flag & FB_SYSFS_VDP_LOGLEVEL) {
			os_device_remove_file(fb->dev, &dev_attr_loglevel);
			fb->class_flag &= ~FB_SYSFS_VDP_LOGLEVEL;
		}
		//remove vdp4dump node
		os_device_remove_file(fb->dev, &dev_attr_vdp4dump);

		unregister_framebuffer(fb);
		mtk_fb_free_memory(fb);
		framebuffer_release(fb);
	}

	return 0;
}

static int  mtkfb_suspend(struct device *dev)
{
	FB_PRINT(FB_LOG_LVL_INFO, "", "mtkfb_suspend begin\n");
	/*pause lcm power*/

	/*use 2mA current*/
	/*	(*((volatile __u32 *)((__u32)0xFD000504))) = 0x00;*/
	/*    (*((volatile __u32 *)((__u32)0xFD000518))) = 0x00;*/
	vSclHalIsrStop(1);
	vPmxHalIsrStop(1);
	vSetBacklightIntensity(0);
	vTconSupend();
	tve_suspend();
	/*  Osd_PustReset_Plane(OSD_PLANE_MAX_NUM);*/
	/*  IO_WRITE32(VDP_HAL_VDO_F_REG, 0x3C, 0xFF);*/
	clk_unprepare(clk_osd1);
	clk_unprepare(clk_osd2);
	clk_unprepare(clk_osd3);
	clk_unprepare(clk_osd4);
	clk_unprepare(clk_osdr2);
	clk_unprepare(clk_osdr3);
	clk_disable_unprepare(clk_osdf);
	clk_disable_unprepare(clk_osdr);	
	//clk_unprepare(clk_osdf);
	//clk_unprepare(clk_osdr);
	clk_unprepare(clk_imgrsz);
	clk_unprepare(clk_osdrsz);
	clk_unprepare(clk_lvds);
	clk_unprepare(clk_scl);
	clk_unprepare(clk_scltg);
	clk_unprepare(clk_fmtf);
	clk_unprepare(clk_fmtr);
	clk_unprepare(clk_tcon);
	clk_unprepare(clk_tve);
	FB_PRINT(FB_LOG_LVL_INFO, "", "mtkfb_suspend end\n");

	return 0;
}

void DelayMs(unsigned int n)
{
	volatile unsigned int i, j;

	for (i = 0; i < n; i++)
		for (j = 0; j < 800000; j++) { /*800Mhz*/
			;
		}
}

#define WriteREG(arg, val) (*(volatile __u32 *)(IO_BASE_VA + (arg)) = val)

static int  mtkfb_resume(struct device *dev)
{
	FB_PRINT(FB_LOG_LVL_INFO, "", "mtkfb_late_resume begin\n");
	/*resume lcm power*/

	/*use 8mA current*/
	/*	(*((volatile __u32 *)((__u32)0xFD000504))) = 0xFFFFFFFF;*/
	/*    (*((volatile __u32 *)((__u32)0xFD000518))) = 0xFFFFFFFF;*/

	clk_prepare_enable(clk_osdf);
	clk_prepare_enable(clk_osdr);
	clk_prepare(clk_osd1);
	clk_prepare(clk_osd2);
	clk_prepare(clk_osd3);
	clk_prepare(clk_osd4);
	clk_prepare(clk_osdr2);
	clk_prepare(clk_osdr3);
	//clk_prepare(clk_osdf);
	//clk_prepare(clk_osdr);
	clk_prepare(clk_imgrsz);
	clk_prepare(clk_osdrsz);
	clk_prepare(clk_lvds);
	clk_prepare(clk_tcon);
	clk_prepare(clk_scltg);
	clk_prepare(clk_scl);
	clk_prepare(clk_fmtf);
	clk_prepare(clk_fmtr);
	clk_prepare(clk_tve);

	vSclHalResume();
	vPmxHalResume(PMX_1);
	vPmxHalResume(PMX_2);
	vTconResume();

	/* vPmxReOpenVOPLL();*/
	/*  Osd_ReleaseReset_Plane();*/
	/*   vVdpHalReset(VDP_1);*/
	/*DelayMs(100);   //must not use Sleep function;*/
	i4OSDRestoreHwReg();
	/* vSetBacklightIntensity(_u4BacklightValue);*/
	vSclHalIsrInit();
	vPmxHalIsrInit();
	tve_resume();
	WriteREG(0xa504c, 0x0000000f);
	msleep(20);
	WriteREG(0xa504c, 0x00000000);

	FB_PRINT(FB_LOG_LVL_INFO, "", "mtkfb_late_resume end\n");
	return 0;
}

void vGetFBConfigFromShareMemory(FB_CONFIG_T *prFBConfig)
{
	memcpy((void *)prFBConfig, (const void *)FB_PHYSICAL_TO_VIRTUAL(ARM2_FBDRV_SHARE_MEMORY_PA)
		, sizeof(FB_CONFIG_T));
}
EXPORT_SYMBOL(vGetFBConfigFromShareMemory);

static const struct dev_pm_ops  mtkfb_dev_pm_ops = {

	SET_SYSTEM_SLEEP_PM_OPS(mtkfb_suspend, mtkfb_resume)
};

static const struct of_device_id fb_of_match[] = {
	{.compatible = "Autochips,framebuffer",},
	{}
};

static void fb_shutdown(struct platform_device *dev)
{
	TurnOnTve(1, FALSE);
}

/* Currenlty no platform devices registered */
static struct platform_driver mtk_fb_driver = {
	.probe        = mtk_fb_probe,
	/*.remove       = __devexit_p(mtk_fb_remove),*/
	.remove       = mtk_fb_remove,
	.driver = {
		.name = "mtk_fb",
		.owner = THIS_MODULE,
		.pm = &mtkfb_dev_pm_ops,
		.of_match_table = fb_of_match,
	},
	#ifdef CONFIG_ATC_OS_android
        .shutdown = fb_shutdown
        #endif
};

static int __init mtk_fb_init(void)
{
	platform_driver_register(&mtk_fb_driver);

	return 0;
}

static void __exit mtk_fb_exit(void)
{
	if (fbm_va) {
		iounmap(fbm_va);
	}
	platform_driver_unregister(&mtk_fb_driver);
}

module_init(mtk_fb_init);
module_exit(mtk_fb_exit);

/*MODULE_LICENSE(MTK_KERNEL_LINUX_LICENSE);*/
MODULE_LICENSE("GPL");





