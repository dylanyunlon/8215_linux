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
#ifndef __DISP_ASSERT_LAYER_H__
#define __DISP_ASSERT_LAYER_H__

#include <linux/types.h>

#ifdef __cplusplus
extern "C" {
#endif
#define MAKE_DISP_FORMAT_ID(id, bpp)  (((id) << 8) | (bpp))

typedef enum {
	DISP_FORMAT_UNKNOWN = 0,

	DISP_FORMAT_RGB565 = MAKE_DISP_FORMAT_ID(1, 2),
	DISP_FORMAT_RGB888 = MAKE_DISP_FORMAT_ID(2, 3),
	DISP_FORMAT_BGR888 = MAKE_DISP_FORMAT_ID(3, 3),
	DISP_FORMAT_ARGB8888 = MAKE_DISP_FORMAT_ID(4, 4),
	DISP_FORMAT_ABGR8888 = MAKE_DISP_FORMAT_ID(5, 4),
	DISP_FORMAT_RGBA8888 = MAKE_DISP_FORMAT_ID(6, 4),
	DISP_FORMAT_BGRA8888 = MAKE_DISP_FORMAT_ID(7, 4),
	DISP_FORMAT_YUV422 = MAKE_DISP_FORMAT_ID(8, 2),
	DISP_FORMAT_XRGB8888 = MAKE_DISP_FORMAT_ID(9, 4),
	DISP_FORMAT_XBGR8888 = MAKE_DISP_FORMAT_ID(10, 4),
	DISP_FORMAT_RGBX8888 = MAKE_DISP_FORMAT_ID(11, 4),
	DISP_FORMAT_BGRX8888 = MAKE_DISP_FORMAT_ID(12, 4),
	DISP_FORMAT_UYVY = MAKE_DISP_FORMAT_ID(13, 2),
	DISP_FORMAT_YUV420_P = MAKE_DISP_FORMAT_ID(14, 2),
	DISP_FORMAT_YV12 = MAKE_DISP_FORMAT_ID(16, 1),	/* BPP = 1.5 */
	DISP_FORMAT_BPP_MASK = 0xFF,
} DISP_FORMAT;

typedef enum {
	DAL_STATUS_OK = 0,

	DAL_STATUS_NOT_READY = -1,
	DAL_STATUS_INVALID_ARGUMENT = -2,
	DAL_STATUS_LOCK_FAIL = -3,
	DAL_STATUS_LCD_IN_SUSPEND = -4,
	DAL_STATUS_FATAL_ERROR = -10,
} DAL_STATUS;


typedef enum {
	DAL_COLOR_BLACK = 0x000000,
	DAL_COLOR_WHITE = 0xFFFFFF,
	DAL_COLOR_RED = 0xFF0000,
	DAL_COLOR_GREEN = 0x00FF00,
	DAL_COLOR_BLUE = 0x0000FF,
	DAL_COLOR_TURQUOISE = (DAL_COLOR_GREEN | DAL_COLOR_BLUE),
	DAL_COLOR_YELLOW = (DAL_COLOR_RED | DAL_COLOR_GREEN),
	DAL_COLOR_PINK = (DAL_COLOR_RED | DAL_COLOR_BLUE),
} DAL_COLOR;


/* Display Assertion Layer API */
unsigned int DAL_GetLayerSize(void);
DAL_STATUS DAL_SetScreenColor(DAL_COLOR color);
DAL_STATUS DAL_Init(unsigned int layerVA, unsigned int layerPA, unsigned int u4Width, unsigned int u4Height);
DAL_STATUS DAL_SetColor(unsigned int fgColor, unsigned int bgColor);
DAL_STATUS DAL_Clean(void);
DAL_STATUS DAL_Printf(const char *fmt, ...);
DAL_STATUS DAL_OnDispPowerOn(void);
DAL_STATUS DAL_LowMemoryOn(void);
DAL_STATUS DAL_LowMemoryOff(void);


unsigned long get_Assert_Layer_PA(void);
int is_DAL_Enabled(void);
extern unsigned int isAEEEnabled;

extern int DAL_Clean(void);
extern int DAL_Printf(const char *fmt, ...);
extern struct semaphore dal_sem;
extern bool dal_shown;

#ifdef __cplusplus
}
#endif
#endif /* __DISP_ASSERT_LAYER_H__ */
