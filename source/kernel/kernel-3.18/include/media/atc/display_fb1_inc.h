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
#ifndef _DISPLAY_FB1_INC_H
#define _DISPLAY_FB1_INC_H

#include "x_os.h"
#include <media/atc/drv_osd_if.h>
#include "chip_ver.h"
#include "media/atc/display.h"
#include <generated/atc_project.h>


#ifdef __cplusplus

extern "C" {            /* Assume C declarations for C++ */
#endif	/* __cplusplus */

extern  __u32  _u4DispMode;
extern  __u32  _u4LCDWidth;
extern  __u32  _u4LCDHeight;



#ifdef __cplusplus
}
#endif	/* __cplusplus */
#define VIDEO_MODE                            RES_480P

#if defined(CONFIG_ATC_OS_android)

extern __u32 _fb1Pa;
extern __u32 _fb1Va;
extern __u32 _fb1Size;
extern unsigned int fbm_base;

#define DISPLAY_MODE	RES_480P_800
#define OUTPUT_WIDTH	720
#define OUTPUT_HEIGHT	480

#define EXTERNAL_OSD_WIDTH     720
#define EXTERNAL_OSD_HEIGHT    480
#elif defined(CONFIG_ATC_OS_linux)
#define EXTERNAL_OSD_WIDTH     1024 /*720*/
#define EXTERNAL_OSD_HEIGHT    600 /*480*/
#endif

#define EXTERNAL_COLOR_DEPTH_32_BIT          1

#define EXTERNAL_DISPLAY_DEVICE_WIDTH_PHYSCIAL         400 /*(mm)*/
#define EXTERNAL_DISPLAY_DEVICE_HEIGHT_PHYSCIAL        300  /*(mm)*/

#define EXTERNAL_PRIMARY_PLANE_ID                             (OSD_PLANE_8)

#if EXTERNAL_COLOR_DEPTH_32_BIT
#define EXTERNAL_ANDROID_BITS_PER_PIXEL 32  /*argb8888*/
#else
#define EXTERNAL_ANDROID_BITS_PER_PIXEL 16 /*rgb565 */
#endif
#define  EXTERNAL_ANDROID_NUMBER_OF_BUFFERS  3


extern bool subtitle_osd_init(unsigned int addr,  OSD_DATA_T *prData);
extern bool subtitle_osd_palette_init(unsigned int addr, OSD_DATA_T *prData);
extern void mt3360_display_init(unsigned int, unsigned int, unsigned int, unsigned int);
extern void mt3360_display_uninit(void);
extern void mt3360_display_flip(struct fb_var_screeninfo *);
extern void TurnOnTve(__u32 u4LayerID, bool fgOn);
extern void  vGetFBConfigFromShareMemory(FB_CONFIG_T *prFBConfig);


#endif


