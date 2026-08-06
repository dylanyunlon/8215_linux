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

#ifndef _AVM_HAL_H_
#define _AVM_HAL_H_
 
 
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include "ac823x_tvd/tvd_drv_if.h"
#include "ac823x/wch_if.h"

#define VIDEO_MAX_FRAME 5
#define AVM_MAX_VIDEO_DEVS 5

 typedef unsigned long	HANDLE_T;
#define NULL_HANDLE ((HANDLE_T)(NULL))
#define OSR_OK (unsigned int)(0)

 enum frame_status {
	 FRAME_EMPTY,
	 FRAME_READING,
	 FRAME_READY,
	 FRAME_ERROR
 };

 struct framebuf {
	 int idx;
	 int length;
	 volatile enum frame_status status;
	 u8 *data;
	 unsigned long userptr;
 };
 
 struct avmbuf {
	unsigned long yAddr;
	unsigned long cAddr;
	u32 width;
	u32 height;
 };
 
 struct avm_data {
	 int streaming;
	 struct v4l2_pix_format pix;
	 int memset_cnt;
	 int num_frames;
	 int cur_frames;
	 struct framebuf buffers[VIDEO_MAX_FRAME];
 };

 extern struct avm_data g_avm_data[AVM_MAX_VIDEO_DEVS];
 extern u32 BUFFER_WIDTH;
 extern u32 BUFFER_HEIGHT;

 int avm_tvd_control(TVD_CHANNEL_ID channel_id, int CtrlCode);
 int avm_init_video(int index);
 int avm_set_input(int index);
 int avm_get_std(uint8_t chid, v4l2_std_id *pstd_type);
 int avm_start_video(TVD_CHANNEL_ID channel_id);
 int avm_stop_video(TVD_CHANNEL_ID channel_id);
 int avm_init(void);
#endif

