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
#include <linux/types.h>
#include "disp_assert_layer.h"
#include <linux/semaphore.h>
#include <linux/mutex.h>

/* /common part */
#define DAL_BPP             (2)
#define DAL_WIDTH           (1024)
#define DAL_HEIGHT          (600)

/* #ifdef CONFIG_MTK_FB_SUPPORT_ASSERTION_LAYER */

#include <linux/string.h>
#include <linux/semaphore.h>
#include <asm/cacheflush.h>
#include <linux/module.h>
#include "mtkfb_console.h"
#include <media/atc/display_inc.h>
#include <media/atc/drv_osd_if.h>

/* --------------------------------------------------------------------------- */
#define DAL_FORMAT          (DISP_FORMAT_RGB565)
#define DAL_BG_COLOR        (dal_bg_color)
#define DAL_FG_COLOR        (dal_fg_color)
#define RGB888_To_RGB565(x) ((((x) & 0xF80000) >> 8) |                      \
			     (((x) & 0x00FC00) >> 5) |                      \
			     (((x) & 0x0000F8) >> 3))
#define MAKE_TWO_RGB565_COLOR(high, low)  (((low) << 16) | (high))
#define DAL_LOG(fmt, arg...)	pr_debug("[DISP/DAL] " fmt, ##arg)
/* --------------------------------------------------------------------------- */
static MFC_HANDLE mfc_handle;
static void *dal_fb_addr;
static unsigned int dal_fb_pa;
unsigned int isAEEEnabled = 0;
bool dal_shown = false;
static unsigned int dal_fg_color = RGB888_To_RGB565(DAL_COLOR_WHITE);
static unsigned int dal_bg_color = RGB888_To_RGB565(DAL_COLOR_RED);
DEFINE_SEMAPHORE(dal_sem);
static char dal_print_buffer[1024];

unsigned int g_lcm_width;
unsigned int g_lcm_height;

/* --------------------------------------------------------------------------- */
#define DIPSLAY_FUNC() pr_debug("[YZQ/DAL] %s, %d \n", __FUNCTION__, __LINE__)
extern __s32 i4OsdPlaneEnbleDAL(__u32 u4Plane, __u32 fgEnble);
uint32_t DAL_GetLayerSize(void)
{
	DIPSLAY_FUNC();
	return g_lcm_width * g_lcm_height * DAL_BPP + 4096;
}

DAL_STATUS DAL_SetScreenColor(DAL_COLOR color)
{
	uint32_t i;
	uint32_t size;
	uint32_t BG_COLOR;
	MFC_CONTEXT *ctxt = NULL;
	uint32_t offset;
	unsigned int *addr;

	color = RGB888_To_RGB565(color);
	BG_COLOR = MAKE_TWO_RGB565_COLOR(color, color);
	DIPSLAY_FUNC();

	ctxt = (MFC_CONTEXT *) mfc_handle;
	if (!ctxt)
		return DAL_STATUS_FATAL_ERROR;
	if (ctxt->screen_color == color)
		return DAL_STATUS_OK;

	offset = MFC_Get_Cursor_Offset(mfc_handle);
	addr = (unsigned int *)(ctxt->fb_addr + offset);

	size = DAL_GetLayerSize() - offset;
	for (i = 0; i < size / sizeof(uint32_t); ++i)
		*addr++ = BG_COLOR;

	ctxt->screen_color = color;

	return DAL_STATUS_OK;
}
EXPORT_SYMBOL(DAL_SetScreenColor);

DAL_STATUS DAL_Init(unsigned int layerVA, unsigned int layerPA, unsigned int u4Width, unsigned int u4Height)
{
	MFC_STATUS ret;
	DIPSLAY_FUNC();
	//char *c = "ATC-SD-SD2 yzq!";
	DAL_LOG("%s, layerVA=0x%lx, layerPA=0x%lx\n", __func__, layerVA, layerPA);

	g_lcm_width = u4Width;
	g_lcm_height = u4Height;
	dal_fb_addr = (void *)layerVA;
	dal_fb_pa = layerPA;

	ret = MFC_Open(&mfc_handle, dal_fb_addr, g_lcm_width, g_lcm_height, DAL_BPP, DAL_FG_COLOR, DAL_BG_COLOR);
	if (MFC_STATUS_OK != ret) {
		pr_err("DISP/DAL: Warning: call MFC_XXX function failed in %s(), line: %d, ret: %x\n",
			__func__, __LINE__, ret);
		return ret;
	}

	/* DAL_Clean(); */
	DAL_SetScreenColor(DAL_COLOR_RED);
	//DAL_Printf(c);

	return DAL_STATUS_OK;
}

DAL_STATUS DAL_SetColor(unsigned int fgColor, unsigned int bgColor)
{
	MFC_STATUS ret;

	if (NULL == mfc_handle)
		return DAL_STATUS_NOT_READY;

	if (down_interruptible(&dal_sem)) {
		pr_err("DISP/DAL " "Can't get semaphore in %s()\n", __func__);
		return DAL_STATUS_LOCK_FAIL;
	}

	dal_fg_color = RGB888_To_RGB565(fgColor);
	dal_bg_color = RGB888_To_RGB565(bgColor);
	DIPSLAY_FUNC();

	ret = MFC_SetColor(mfc_handle, dal_fg_color, dal_bg_color);
	if (MFC_STATUS_OK != ret) {
		pr_err("DISP/DAL: Warning: call MFC_XXX function failed in %s(), line: %d, ret: %x\n",
			__func__, __LINE__, ret);
		return ret;
	}

	up(&dal_sem);

	return DAL_STATUS_OK;
}
EXPORT_SYMBOL(DAL_SetColor);

DAL_STATUS DAL_Clean(void)
{
	DAL_STATUS ret = DAL_STATUS_OK;
	MFC_STATUS r;
	static int dal_clean_cnt;
	MFC_CONTEXT *ctxt = (MFC_CONTEXT *) mfc_handle;
	__u32 plane = OSD_PLANE_4;

	if (NULL == mfc_handle)
		return DAL_STATUS_NOT_READY;

	if (down_interruptible(&dal_sem)) {
		pr_err("DISP/DAL " "Can't get semaphore in %s()\n", __func__);
		return DAL_STATUS_LOCK_FAIL;
	}
	DIPSLAY_FUNC();

	r = MFC_ResetCursor(mfc_handle);
	if (MFC_STATUS_OK != r) {
		pr_err("DISP/DAL: Warning: call MFC_XXX function failed in %s(), line: %d, ret: %x\n",
			__func__, __LINE__, r);
		return r;
	}

	ctxt->screen_color = 0;
	DAL_SetScreenColor(DAL_COLOR_RED);

	if (isAEEEnabled == 1) {
		pr_info("isAEEEnabled from 1 to 0, %d\n",
			dal_clean_cnt++);
		i4OsdPlaneEnbleDAL(plane, FALSE);
		isAEEEnabled = 0;
	}

	up(&dal_sem);

	return ret;
}
EXPORT_SYMBOL(DAL_Clean);

int is_DAL_Enabled(void)
{
	int ret = 0;
	DIPSLAY_FUNC();

	if (down_interruptible(&dal_sem)) {
		pr_err("DISP/DAL " "Can't get semaphore in %s()\n", __func__);
		return DAL_STATUS_LOCK_FAIL;
	}

	ret = isAEEEnabled;

	up(&dal_sem);

	return ret;
}

unsigned long get_Assert_Layer_PA(void)
{
	DIPSLAY_FUNC();
	return dal_fb_pa;
}

DAL_STATUS DAL_Printf(const char *fmt, ...)
{
	va_list args;
	uint i;
	DAL_STATUS ret = DAL_STATUS_OK;
	MFC_STATUS r;
	__u32 plane = OSD_PLANE_4;

	DIPSLAY_FUNC();

	if (NULL == mfc_handle)
		return DAL_STATUS_NOT_READY;

	if (NULL == fmt)
		return DAL_STATUS_INVALID_ARGUMENT;

	if (down_interruptible(&dal_sem)) {
		pr_err("DISP/DAL " "Can't get semaphore in %s()\n",  __func__);
		return DAL_STATUS_LOCK_FAIL;
	}

	va_start(args, fmt);
	i = vsprintf(dal_print_buffer, fmt, args);
	va_end(args);

	r = MFC_Print(mfc_handle, dal_print_buffer);
	if (MFC_STATUS_OK != r) {
		pr_err("DISP/DAL: Warning: call MFC_XXX function failed in %s(), line: %d, ret: %x\n",
			__func__, __LINE__, r);
		return r;
	}

	flush_cache_all();
	if (!dal_shown)
		dal_shown = true;

	if (isAEEEnabled == 0) {
		isAEEEnabled = 1;
		i4OsdPlaneEnbleDAL(plane, TRUE);
	}

	up(&dal_sem);

	return ret;
}
EXPORT_SYMBOL(DAL_Printf);

DAL_STATUS DAL_OnDispPowerOn(void)
{
	DIPSLAY_FUNC();
	return DAL_STATUS_OK;
}
