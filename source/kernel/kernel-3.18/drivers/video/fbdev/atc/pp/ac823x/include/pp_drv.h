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
/*
 *Department: ATC-SD-SD2
 *
 *Aurthor: yezhiqiang(atc0064)
 *
 *Date: 2017-02-27
 */
#ifndef _PP_DRV_H_
#define _PP_DRV_H_

typedef enum
{
    POST_VIDEO_CONTRAST = 0,
    POST_VIDEO_BRIGHTNESS,
    POST_VIDEO_HUE,
    POST_VIDEO_SATURATION,
    POST_VIDEO_SHARPNESS,		//SHARPNESS 4
    POST_VIDEO_CDS,
    POST_VIDEO_CTI,				//CTI 6
    POST_VIDEO_ADAPTIVE_LUMA_ONOFF,
    POST_VIDEO_SCE_ONOFF,
    
    POST_VIDEO_COLOR_RED_Y,
    POST_VIDEO_COLOR_RED_S,
    POST_VIDEO_COLOR_RED_H,
    POST_VIDEO_COLOR_GREEN_Y,
    POST_VIDEO_COLOR_GREEN_S,
    POST_VIDEO_COLOR_GREEN_H,
    POST_VIDEO_COLOR_BLUE_Y,
    POST_VIDEO_COLOR_BLUE_S,
    POST_VIDEO_COLOR_BLUE_H,
    POST_VIDEO_COLOR_YELLOW_Y,
    POST_VIDEO_COLOR_YELLOW_S,
    POST_VIDEO_COLOR_YELLOW_H,
    POST_VIDEO_COLOR_CYAN_Y,
    POST_VIDEO_COLOR_CYAN_S,
    POST_VIDEO_COLOR_CYAN_H,
    POST_VIDEO_COLOR_MAGENTA_Y,
    POST_VIDEO_COLOR_MAGENTA_S,
    POST_VIDEO_COLOR_MAGENTA_H,
  
}   POST_UI_ITEM_T;

typedef enum {
	RES_480P,
	RES_480I,
	RES_576P,
	RES_576I,
	RES_800X480,
	RES_800X600,
	RES_1024X600,
	RES_720P,
	RES_1080I,
	RES_1080P,
} PP_DISPLAY_MODE_E;

#define PP_FRONT 0
#define PP_REAR  1

#endif
