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
#include <linux/of.h>
#include <linux/of_reserved_mem.h>
#include <asm/uaccess.h>
#include "FB1.h"
#include "media/atc/display_fb1_inc.h"
#include "media/atc/display.h"
#include "media/atc/pmx_hal.h"
#include "../ac83xx/inc/tve_hal.h"
#include "media/atc/drv_av_d.h"
#include "media/atc/drv_osd_if.h"
#include <asm/cacheflush.h>
#include "../ac83xx/inc/vdp.h"
#include "../ac83xx/inc/tcon.h"
#include "../ac83xx/inc/osd_if_pdd.h"
#include <x_typedef.h>
#include "drv_imgresz.h"
#include "../ac83xx/inc/osd_inc.h"
#include "../ac83xx/inc/log.h"
#include "x_ver.h"
#include <generated/atc_project.h>
#include <linux/of_fdt.h>

#define MTK_KERNEL_LINUX_LICENSE     "Proprietary"

#define MMISC_MODE_NAME                   "FB1"
#define MMISC_VER_MAJOR                   01
#define MMISC_VER_MINOR                   00
#define MMISC_VER_REV                        00

#define DISPLAY_SET_CONTRAST                   (0x00020005)
#define DISPLAY_SET_BRIGNTNESS               (0x00020006)
#define DISPLAY_SET_SATURATION                (0x00020007)
#define DISPLAY_SET_BKL_INTENSITY           (0x00020008)
#define DISPLAY_SET_HUE                              (0x00020009)

#define GPIO_LCM_PWDN 56

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

static u32 rear_base, rear_size;
void * rear_base_va = NULL;
static unsigned int fb_addr1;
static bool external_osd_init(unsigned int addr1, int width, int height);
extern int VDP_IOControl(__u32 dwCode, void *pBufIn, void *pBufOut);
extern bool is_fr_follow_on(void);

static struct fb_fix_screeninfo mtkfb_fix = {
	.id          = "ext_fb",
	.type        = FB_TYPE_PACKED_PIXELS,
	.visual      = FB_VISUAL_TRUECOLOR,
	.line_length = EXTERNAL_OSD_WIDTH * (EXTERNAL_ANDROID_BITS_PER_PIXEL / 8),
	.ypanstep    = 1,
	.accel       = FB_ACCEL_NONE
};

static struct fb_var_screeninfo mtkfb_var = {
	.xres		= EXTERNAL_OSD_WIDTH,
	.yres		= EXTERNAL_OSD_HEIGHT,
	.xres_virtual	= EXTERNAL_OSD_WIDTH,
	.yres_virtual	= EXTERNAL_OSD_HEIGHT * EXTERNAL_ANDROID_NUMBER_OF_BUFFERS,
	.bits_per_pixel	= EXTERNAL_ANDROID_BITS_PER_PIXEL,
	.activate	= FB_ACTIVATE_NOW,
	.width		= EXTERNAL_DISPLAY_DEVICE_WIDTH_PHYSCIAL,
	.height		= EXTERNAL_DISPLAY_DEVICE_HEIGHT_PHYSCIAL,
#if EXTERNAL_COLOR_DEPTH_32_BIT
	.red.offset	= 16,
	.red.length	= 8,
	.green.offset	= 8,
	.green.length	= 8,
	.blue.offset	= 0,
	.blue.length	= 8
#else
	.red.offset	= 11,
	.red.length	= 5,
	.green.offset	= 5,
	.green.length	= 6,
	.blue.offset	= 0,
	.blue.length	= 5
#endif
};

static struct fb_info *fb;

struct mtkfb {
    u32 cmap[16];
};


static inline u32 convert_bitfield(int val, struct fb_bitfield *bf)
{
	unsigned int mask = (1 << bf->length) - 1;

	return (val >> (16 - bf->length) & mask) << bf->offset;
}

/* set the software color map.  Probably doesn't need modifying. */
static int fb1_setcolreg(unsigned int regno, unsigned int red, unsigned int green,
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
static int fb1_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	FB_PRINT(FB_LOG_LVL_DBG, "FB1", "fb_check_var begin\n");

	if((var->xres != info->var.xres) || (var->yres != info->var.yres) ||
		(var->xres_virtual != info->var.xres) || (var->yres_virtual < info->var.yres ) ||
		(var->yres_virtual > info->var.yres * EXTERNAL_ANDROID_NUMBER_OF_BUFFERS)) {
		return -EINVAL;
	}

	if((var->xoffset != info->var.xoffset) ||
		(var->bits_per_pixel != info->var.bits_per_pixel) ||
		(var->grayscale != info->var.grayscale)) {
		return -EINVAL;
	}

	return 0;
}

/* Handles screen rotation if device supports it. */
static int fb1_set_par(struct fb_info *info)
{
	return 0;
}


static void external_display_flip(struct fb_var_screeninfo *var)
{
	u32 u4Addr, u4Rgn;
	u32 u4Plane = EXTERNAL_PRIMARY_PLANE_ID;

	u4Addr = fb_addr1 +  var->yoffset * EXTERNAL_OSD_WIDTH * (EXTERNAL_ANDROID_BITS_PER_PIXEL / 8);
	u4Rgn = GetPlaneRgn(u4Plane);
	OSD_RGN_Set(u4Rgn, (u32)OSD_RGN_BMP_ADDR, u4Addr);

        FB_PRINT(FB_LOG_LVL_INFO, "FB1", "wts external flip to %x %x \n", fb_addr1, u4Addr);

	FB_PRINT(FB_LOG_LVL_DBG, "FB1", "%s rgn %d flip to yoffset %d addr 0x%x\n", __func__, u4Rgn, var->yoffset, u4Addr);
}


/* Pan the display if device supports it. */
static int fb1_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	if (!is_fr_follow_on()) {//mirror priority is larger than the one of frds
		external_display_flip(var);
	} else {
		FB_PRINT(FB_LOG_LVL_INFO, "FB1", "%s rear mirror on not flip\n", __func__);
	}

	return 0;
}

static int fb1_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	u32 u4Offset;

	FB_PRINT(FB_LOG_LVL_DBG, "FB1", "mtk_fb_ioctl begin %d\n", cmd);
         if (is_fr_follow_on())
         {
            return (-EINVAL);
         }

	switch(cmd) {
	case 0x1001: {
		u32 u4Addr, u4Rgn;
		u32 u4Plane = EXTERNAL_PRIMARY_PLANE_ID;

		u4Offset = arg;
		FB_PRINT(FB_LOG_LVL_DBG, "FB1", "flip to gralloc fb buffer pa= %x\n", u4Offset);
#if defined(CONFIG_ATC_OS_android)
		u4Addr = fbm_base + u4Offset;
#elif defined(CONFIG_ATC_OS_linux)
		u4Addr = rear_base + u4Offset;
#endif
		u4Rgn = GetPlaneRgn(u4Plane);
		OSD_RGN_Set(u4Rgn, (u32)OSD_RGN_BMP_ADDR, u4Addr);
		break;
	}
	case TVE_TURNON:
		TurnOnTve(1, arg);
		break;
	default:
		ret = VDP_IOControl(cmd , (void *)arg, (void*)arg);
		break;
	}


	if ( ret == 0) {
		return 0;
	} else {
		return -EINVAL;
	}
}

static struct fb_ops mtk_fb_ops = {
            .owner		= THIS_MODULE,
            .fb_check_var	= fb1_check_var,
            .fb_set_par		= fb1_set_par,
            .fb_setcolreg	= fb1_setcolreg,
            .fb_pan_display	= fb1_pan_display,
            .fb_copyarea	= sys_copyarea,
            .fb_imageblit	= sys_imageblit,
            .fb_ioctl		= fb1_ioctl,
};

int get_fb_rear_param(struct platform_device *pdev)
{
	u32 ret = -EINVAL;
	struct reserved_mem *fb_mem;

	of_reserved_mem_device_init(&(pdev->dev));
	fb_mem = (struct reserved_mem *)(pdev->dev.cma_area);
	if (!fb_mem) {
		FB_PRINT(FB_LOG_LVL_ERR, "FB1", "get memory failed %x\r\n", (unsigned int)fb_mem);
		goto err;
	}
	rear_base = fb_mem->base;
	rear_size = fb_mem->size;
	if ((rear_base == 0) || (rear_size == 0)) {
		FB_PRINT(FB_LOG_LVL_ERR, "FB1", "get memory failed base %x size %x\r\n", rear_base, rear_size);
		goto err;
	}
	rear_base_va = ioremap(rear_base, rear_size);
	if (rear_base_va == NULL) {
		FB_PRINT(FB_LOG_LVL_ERR, "FB1", "get ioremap va error, memory name %s, base 0x%x, size %d\r\n"
			, fb_mem->name, rear_base, rear_size / SZ_1M);
		goto err;
	}
	FB_PRINT(FB_LOG_LVL_INFO, "FB1", "get memory name %s, base 0x%x, va 0x%x, size %d\r\n", fb_mem->name
		, rear_base, (unsigned int)rear_base_va, rear_size / SZ_1M);
	ret = 0;
err:

	return ret;
}
/**
 * mtk_fb_alloc_memory() - allocate display memory for framebuffer window
 *
 * Allocate memory for the given framebuffer.
 */
static int mtk_fb_alloc_memory(struct fb_info *fb)
{
	u32 real_size, virt_size, size;

	real_size = fb->var.xres * fb->var.yres;
	virt_size = fb->var.xres_virtual * fb->var.yres_virtual;
	FB_PRINT(FB_LOG_LVL_INFO, "FB1", "real_size=%u (%u x %u), virt_size=%u (%u x%u)\n",
		real_size, fb->var.xres, fb->var.yres,
		virt_size, fb->var.xres_virtual, fb->var.yres_virtual);

	size = (real_size > virt_size) ? real_size : virt_size;
	size *= fb->var.bits_per_pixel;
	size /= 8;
	fb->fix.smem_len = size;

#if defined(CONFIG_ATC_OS_android)
	fb->fix.smem_start = _fb1Pa;
	fb->screen_base = (char *)_fb1Va;
	FB_PRINT(FB_LOG_LVL_INFO, "FB1", "%s, pa 0x%x, va 0x%x, size 0x%x\n", __func__, fb->fix.smem_start
		, fb->screen_base, fb->fix.smem_len);
#elif defined(CONFIG_ATC_OS_linux)
	FB_PRINT(FB_LOG_LVL_INFO, "FB1", "%s, pa 0x%x, va 0x%x, size 0x%x\n", __func__, rear_base
		, (unsigned int)rear_base_va, fb->fix.smem_len);
	fb->fix.smem_start = rear_base;
	fb->screen_base = (char *)rear_base_va;
#endif
	if (!fb->screen_base) {
		FB_PRINT(FB_LOG_LVL_ERR, "FB1", "can't alloc video memory\n");
		return -ENOMEM;
	}
#if defined(CONFIG_ATC_OS_android)
	memset(fb->screen_base, 0xff , PAGE_ALIGN(size));
#elif defined(CONFIG_ATC_OS_linux)
	memset(rear_base_va, 0 , size);
#endif
	external_osd_init(fb->fix.smem_start, fb->var.xres, fb->var.yres);

	return 0;
}

static bool  external_osd_init(unsigned int addr1, int width, int height)
{
	s32 ret;
	u32 plane, rgn_list, rgn;
#if defined(CONFIG_ATC_OS_linux)
	u32 out_width = 720, out_height = (VIDEO_MODE == RES_480P) ? 480 : 576;
#endif

	i4OsdVfyCreateSemaphores();
	plane = EXTERNAL_PRIMARY_PLANE_ID;

	fb_addr1 = addr1;
	OSD_BASE_SetOsdPosition(plane, 0, 0);
#if defined(CONFIG_ATC_OS_linux)
	OSD_SC_Scale(plane, FALSE, out_width, out_height, out_width, out_height);
#elif defined(CONFIG_ATC_OS_android)
	OSD_SC_Scale(plane, FALSE, width, height, width, height);
#endif
	OSD_RGN_LIST_Create(&rgn_list);
	rgn_list  = plane;

#if defined(CONFIG_ATC_OS_linux)

#if EXTERNAL_COLOR_DEPTH_32_BIT
	OSD_RGN_Create(&rgn, width, height, (void *)addr1, OSD_CM_ARGB8888_DIRECT32, (width * 4), 0, 0,  out_width, out_height);
#else
	OSD_RGN_Create(&rgn, width, height, (void *)addr1, OSD_CM_RGB565_DIRECT16, (width * 2), 0, 0,  out_width, out_height);
#endif

#elif defined(CONFIG_ATC_OS_android)

#if EXTERNAL_COLOR_DEPTH_32_BIT
	OSD_RGN_Create(&rgn, width, height, (void *)addr1, OSD_CM_ARGB8888_DIRECT32, (width * 4), 0, 0,  width, height);
#else
	OSD_RGN_Create(&rgn, width, height, (void *)addr1, OSD_CM_RGB565_DIRECT16, (width * 2), 0, 0,  width, height);
#endif

#endif

	OSD_RGN_LIST_DetachAll(rgn_list);
	ret = OSD_RGN_Insert(rgn, rgn_list);
	if (ret) {
		printk(KERN_ALERT "[FB1] OSD_RGN_Insert rgn failed: %d\n", (int)ret);
		return ret;
	}
	SetPlaneRgn(rgn_list, rgn);
	i4OsdPlaneFlipTo(plane, rgn_list);
	i4OsdPlaneEnble(plane, TRUE);
#if defined(CONFIG_ATC_OS_linux)
	TurnOnTve(plane, TRUE);
#endif

	return (TRUE);
}

static void mtk_fb_free_memory(struct fb_info *fb)
{
	if (fb->screen_base) {
		dma_free_writecombine(NULL, PAGE_ALIGN(fb->fix.smem_len)
			, fb->screen_base, (dma_addr_t)fb->fix.smem_start);
	}
}

static void vSetFBVARByConfig(void)
{
	FB_CONFIG_T rFBConfig;
	u32 u4Height;

	vGetFBConfigFromShareMemory(&rFBConfig);
	u4Height = (rFBConfig.u4VideoMode == RES_480P) ? 480 : 576;
	mtkfb_var.yres = u4Height;
	mtkfb_var.yres_virtual = u4Height * EXTERNAL_ANDROID_NUMBER_OF_BUFFERS;
}
#if defined(CONFIG_ATC_OS_android)
static inline int mtk_fb1_probe(void)
#elif defined(CONFIG_ATC_OS_linux)
static inline int mtk_fb1_probe(struct platform_device *pdev)
#endif
{
	int ret = -ENOMEM;
	struct mtkfb *mtkfb;

	FB_PRINT(FB_LOG_LVL_INFO, "FB1", "mtk_fb_probe\n");
	MOD_VERSION_INFO(MMISC_MODE_NAME, MMISC_VER_MAJOR, MMISC_VER_MINOR, MMISC_VER_REV);

#if defined(CONFIG_ATC_OS_android)
	vSetFBVARByConfig();
#elif defined(CONFIG_ATC_OS_linux)
	get_fb_rear_param(pdev);
#endif

	fb = framebuffer_alloc(sizeof(struct mtkfb), NULL);
	if (!fb) {
		FB_PRINT(FB_LOG_LVL_ERR, "FB1", "Failed to allocate framebuffer device\n");
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

	//end add
	ret = fb_set_var(fb, &fb->var);
	if (ret) {
		goto err_fb_set_var_failed;
	}
	ret = register_framebuffer(fb);
	if (ret) {
		goto err_register_framebuffer_failed;
	}

	FB_PRINT(FB_LOG_LVL_INFO, "FB1", "probe successful\n");
	return 0;

err_register_framebuffer_failed:
err_fb_set_var_failed:
	framebuffer_release(fb);
err_fb_alloc_failed:
	FB_PRINT(FB_LOG_LVL_ERR, "FB1", "probe failed\n");

	return ret;
}

#if defined(CONFIG_ATC_OS_android)
static int mtk_fb1_remove(void)
#elif defined(CONFIG_ATC_OS_linux)
static int mtk_fb1_remove(struct platform_device *pdev)
#endif
{
	if (fb) {
		unregister_framebuffer(fb);
		mtk_fb_free_memory(fb);
		framebuffer_release(fb);
	}

	return 0;
}

static const struct of_device_id fb1_of_match[] = {
	{.compatible = "Autochips,extframebuffer",},
	{}
};

/* Currenlty no platform devices registered */
static struct platform_driver mtk_fb1_driver = {
	.probe        = mtk_fb1_probe,
	.remove       = mtk_fb1_remove,
	.driver = {
		.name = "mtk_fb1",
		.of_match_table = fb1_of_match,
	}
};

static int __init mtk_fb_init(void)
{
	FB_PRINT(FB_LOG_LVL_DBG, "FB1", "mtk_fb_init begin\n");
#if defined(CONFIG_ATC_OS_android)
	return mtk_fb1_probe();
#elif defined(CONFIG_ATC_OS_linux)
	int ret = -ENOMEM;

	ret = platform_driver_register(&mtk_fb1_driver);
	if (ret) {
	    FB_PRINT(FB_LOG_LVL_ERR, "FB1", "%s: register driver failed %d\r\n", __func__, ret);
	}

	return ret;
#endif
}

static void __exit mtk_fb_exit(void)
{
	FB_PRINT(FB_LOG_LVL_DBG, "FB1", "mtk_fb_exit begin\n");
#if defined(CONFIG_ATC_OS_android)
	mtk_fb1_remove();
#elif defined(CONFIG_ATC_OS_linux)
	if (rear_base_va) {
		iounmap(rear_base_va);
	}
	platform_driver_unregister(&mtk_fb1_driver);
#endif
}

module_init(mtk_fb_init);
module_exit(mtk_fb_exit);

MODULE_LICENSE("GPL");


