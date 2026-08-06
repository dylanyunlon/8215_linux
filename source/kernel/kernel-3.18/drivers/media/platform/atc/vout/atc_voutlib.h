/*
 * atc_voutlib.h
 *
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

#ifndef ATC_VOUTLIB_H
#define ATC_VOUTLIB_H

#include "atc_voutdef.h"

#define OSD_CM_INVALID_VALUE      ((u32)(-1))
#define INVALID_RGN     (-1)

void atc_vout_default_crop(struct v4l2_pix_format *pix, struct v4l2_rect *crop);
void atc_vout_new_format(struct v4l2_pix_format *pix,
		struct v4l2_framebuffer *fbuf, struct v4l2_rect *crop,
		struct v4l2_window *win, struct v4l2_window *extwin);
unsigned long atc_vout_alloc_vdo_buffer(u32 buf_size, u32 *phys_addr);
unsigned long atc_vout_alloc_osd_buffer(u32 buf_size, u32 *phys_addr);
void atc_vout_free_vdo_buffer(unsigned long virtaddr, u32 buf_size);
void atc_vout_free_osd_buffer(unsigned long virtaddr, u32 buf_size);
void atc_vout_reset_cp(struct atc_vout_device *vout, u32 idx);
enum v4l2_priority atc_vout_prio_max(struct atc_vout_device *vout);
u32 atc_vout_get_output(struct atc_vout_device *vout, bool show);
int atc_vout_ioctl(struct atc_vout_device *vout, u32 code);
int atc_vout_hide_video(struct atc_vout_device *vout);
int atc_vout_show_video(struct atc_vout_device *vout);
int atc_vout_osd_on(struct atc_vout_device *vout);
int atc_vout_osd_off(struct atc_vout_device *vout);
int atc_vout_qbuf(struct atc_vout_device *vout, u32 index);
int atc_vout_set_output(struct atc_vout_device *vout, u32 output);
bool atc_vout_can_use_hwdev(struct atc_vout_device *vout, enum vout_hw_ovls hwovl);
int atc_vout_get_backend_dev(struct atc_vout_device *vout, u32 context);

#endif	/* #ifndef ATC_VOUTLIB_H */

