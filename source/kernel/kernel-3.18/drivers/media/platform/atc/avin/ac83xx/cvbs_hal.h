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



#ifndef _CVBS_HAL_H_
#define _CVBS_HAL_H_


#include <linux/videodev2.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include "tvd_drv_if.h"
#include "wch_if.h"
#include "avin_base_def.h"


typedef enum {
	CVBS_TYPE_NORMAL = 0x01,
	CVBS_TYPE_CAMERA = 0x02,
	CVBS_TYPE_BACKCAR = 0x04,
	CVBS_TYPE_MAX
} CVBS_TYPE_ENUM;

extern struct v4l2_data g_cvbs_camera_data;

#ifdef CONFIG_AUDIO_ENABLE
int cvbs_init_audio(int index);
int cvbs_start_audio(int index);
int cvbs_stop_audio(int index);
int cvbs_select_audio(int index);
#endif

int tvdControl(TVD_APP_ID_ENUM cvbs_type, int CtrlCode);
int cvbs_init_video(int index);
int cvbs_start_video(CVBS_TYPE_ENUM cvbs_type, int index);
int cvbs_stop_video(CVBS_TYPE_ENUM cvbs_type, int index);
int cvbs_select_video(CVBS_TYPE_ENUM cvbs_type, int index);
u8 cvbs_get_di_flag(void);
u32 cvbs_set_mirror(CVBS_TYPE_ENUM cvbs_type, u32 mirror);
u32 cvbs_get_signal_status(__s32 *pStatus);
u32 cvbs_get_signal_std(v4l2_std_id *pstd_type);
int cvbs_init(void);
int cvbs_deinit(void);


#endif

