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


#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include "ac823x_tvd/tvd_drv_if.h"
#include "ac823x/wch_if.h"


int cvbs_init_audio(int index);
int cvbs_start_audio(int index);
int cvbs_stop_audio(int index);
int cvbs_select_audio(int index);

int tvdControl(TVD_APP_ID_ENUM cvbs_type, int CtrlCode);
int cvbs_init_video(int index);
int cvbs_start_video(TVD_APP_ID_ENUM cvbs_type, int index);
int cvbs_stop_video(TVD_APP_ID_ENUM cvbs_type, int index);
int cvbs_select_video(TVD_APP_ID_ENUM cvbs_type, int index);
u8 cvbs_get_di_flag(TVD_CHANNEL_ID channel_id);
u32 cvbs_set_mirror(TVD_APP_ID_ENUM cvbs_type, u32 mirror);
u32 cvbs_get_signal_status(__s32 *pStatus);
int cvbs_init(void);


#endif

