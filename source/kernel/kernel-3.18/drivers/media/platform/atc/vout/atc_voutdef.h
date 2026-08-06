/*
 * atc_voutdef.h
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

#ifndef ATC_VOUTDEF_H
#define ATC_VOUTDEF_H

#include <media/atc/display.h>

#define YC420_BPP	2
#define RGB565_BPP	2
#define RGB24_BPP	3
#define RGB32_BPP	4
#define TILE_SIZE	32
#define YUYV_VRFB_BPP	2
#define RGB_VRFB_BPP	1
#define MAX_CID		3
#define MAX_VOUT_DEV	10
#define MAX_HW_OVLS	7
#define MAX_DISPLAYS	2
#define MAX_MANAGERS	3
#define MAX_OBUF_CNT	6
#define INVALID_DEV	0xFF
#define LOW_PRIO	0
#define HIGH_PRIO	1

#define VOUT_DEV_BASE   0

#define DEFAULT_WIDTH		720
#define DEFAULT_HEIGHT		480

#define DEFAULT_OUT_WIDTH		1024
#define DEFAULT_OUT_HEIGHT		600

#define DEFAULT_BUF_NUM     3
#define DEFAULT_BUF_SIZE     0x1000

/* Max Resolution supported by the driver */
#define VID_MAX_WIDTH		1280	/* Largest width */
#define VID_MAX_HEIGHT		800	/* Largest height */

/* Mimimum requirement is 2x2 for DSS */
#define VID_MIN_WIDTH		2
#define VID_MIN_HEIGHT		2

/* 2048 x 2048 is max res supported by atc display controller */
#define MAX_PIXELS_PER_LINE     2048

#define VRFB_TX_TIMEOUT         1000
#define VRFB_NUM_BUFS		4

/* Max buffer size tobe allocated during init */
#define ATC_VOUT_MAX_BUF_SIZE (DEFAULT_OUT_WIDTH*DEFAULT_OUT_HEIGHT*4)

#define V4L2_CID_OUTPUTBLACK        (V4L2_CID_PRIVATE_BASE + 0)
#define V4L2_CID_ZORDER             (V4L2_CID_PRIVATE_BASE + 1)
#define V4L2_CID_YUVGAIN            (V4L2_CID_PRIVATE_BASE + 2)
#define V4L2_CID_CONTEXT            (V4L2_CID_PRIVATE_BASE + 3)
#define V4L2_CID_OUTPUT             (V4L2_CID_PRIVATE_BASE + 4)
#define V4L2_CID_COLOR_RANGE        (V4L2_CID_PRIVATE_BASE + 5)
#define V4L2_CID_COLOR_ENCODEING    (V4L2_CID_PRIVATE_BASE + 6)

#define VIDIOC_QUERY_VALID_DATA_RECT    _IOWR('V', BASE_VIDIOC_PRIVATE, struct v4l2_rect)
#define VIDIOC_SCREEN_PHY_SIZE	        _IOR('V', BASE_VIDIOC_PRIVATE + 1, unsigned int)

#define VOUT_OUTPUT_NONE		(0)
#define VOUT_OUTPUT_FRONT		(1)
#define VOUT_OUTPUT_REAR		(2)
#define VOUT_OUTPUT_FRONTREAR		(3)

#define VOUT_FREE			(0)
#define VOUT_OPEN			(1)
#define VOUT_PREPARE			(2)
#define VOUT_SHOW			(3)
#define VOUT_BACKEND			(4)
#define VOUT_HIDE			(5)

#define VOUT_FMT_UNKNOWN		(0)
#define VOUT_FMT_VIDEO			(1)
#define VOUT_FMT_OSD			(2)

#define VOUT_SET_SRC_PARAMS		(1)
#define VOUT_SET_FRONT_DST_PARAMS	(2)
#define VOUT_SET_REAR_DST_PARAMS	(4)
#define VOUT_SET_PARAMS_MASK		(7)
#define VOUT_SET_BG_OUTPUT		(8)

#define VOUT_BRIGHTNESS_MAX             (255)
#define VOUT_CONTRAST_MAX               (255)
#define VOUT_SATURATION_MAX             (255)
#define VOUT_HUE_MAX                    (255)
#define VOUT_GAIN_MAX                   (511)

#define VOUT_Y_GAIN_OFFSET              (0)
#define VOUT_U_GAIN_OFFSET              (10)
#define VOUT_V_GAIN_OFFSET              (20)
#define VOUT_GET_Y_GAIN(x)              (x & (0x1FF << VOUT_Y_GAIN_OFFSET))
#define VOUT_GET_U_GAIN(x)              (x & (0x1FF << VOUT_U_GAIN_OFFSET))
#define VOUT_GET_V_GAIN(x)              (x & (0x1FF << VOUT_V_GAIN_OFFSET))
#define VOUT_MERGE_YUV_GAIN(y, u, v)    ((y << VOUT_Y_GAIN_OFFSET) | \
					(u << VOUT_U_GAIN_OFFSET) | (v << VOUT_V_GAIN_OFFSET))

#define VOUT_OSD_BUF_SIZE		(0) /*0xC00000*/
/*#define VOUT_OSD_BUF_SIZE		(0xC00000)*/ /*0xC00000*/

#define VOUT_LOG_LVL_OFF                         0
#define VOUT_LOG_LVL_ERR                         1
#define VOUT_LOG_LVL_WARN                        2
#define VOUT_LOG_LVL_INFO                        3
#define VOUT_LOG_LVL_TRACE                       4
#define VOUT_LOG_LVL_DBG                         5
#define VOUT_LOG_LVL_IRQ                         6

extern u32 osd_buf_pa;
extern u32 osd_buf_va;

extern u32 vout_log_lvl;
extern u8 *vout_lvl_str[];

extern struct mutex interface_lock;
#define VOUT_PRINT(lvl, format, ...) \
{ \
	if (lvl <= vout_log_lvl) { \
		if (lvl == VOUT_LOG_LVL_ERR) { \
			pr_err("[VOUT] %s: "format, vout_lvl_str[lvl], ##__VA_ARGS__); \
		} else if (lvl == VOUT_LOG_LVL_WARN) { \
			pr_warn("[VOUT] %s: "format, vout_lvl_str[lvl], ##__VA_ARGS__); \
		} else if (lvl <= VOUT_LOG_LVL_TRACE) { \
			pr_info("[VOUT] %s: "format, vout_lvl_str[lvl], ##__VA_ARGS__); \
		} else { \
			pr_debug("[VOUT] %s: "format, vout_lvl_str[lvl], ##__VA_ARGS__); \
		} \
	} \
}

enum vout_hw_ovls {
	VOUT_HW_VDP1,
	VOUT_HW_VDP2,
	VOUT_HW_OSDF1,
	VOUT_HW_OSDF2,
	VOUT_HW_OSDF3,
	VOUT_HW_OSDR1,
	VOUT_HW_OSDR2,
};

enum vout_context {
	VOUT_CONTEXT_FRONT,
	VOUT_CONTEXT_REAR,
};

enum dma_channel_state {
	DMA_CHAN_NOT_ALLOTED,
	DMA_CHAN_ALLOTED,
};

enum color_encoding {
    COLOR_YCBCR_BT601,
    COLOR_YCBCR_BT709,
};

enum color_range {
    COLOR_YCBCR_LIMITED_RANGE,
    COLOR_YCBCR_FULL_RANGE,
};

struct atcvideo_info {
	int id;
	int num_overlays;
};

struct atcvideo_device {
	struct mutex  mtx;
	struct v4l2_device v4l2_dev;
	struct atc_vout_device *vouts[MAX_VOUT_DEV];
	struct atc_vout_device *hwdevs[MAX_HW_OVLS];
};

/* per-device data structure */
struct atc_vout_device {

	struct video_device *vfd;
	struct atcvideo_device *vid_dev;
	int vid;
	int opened;

	enum v4l2_priority prio; /*Overlay priority*/
	u32 format; /*Overlay format is video or OSD*/
	u32 output; /*Output to front, rear or front & rear*/
	u32 orgout; /*Last output*/
	u32 context; /*Set param for front or rear*/
	u32 qbufcnt; /*Overlay qbuf count*/
	u32 dqbufcnt; /*Overlay dqbuf count*/
	u32 stateflags; /*Overlay set flags*/
	u32 bgcolor[MAX_DISPLAYS];
	u32 brightness[MAX_DISPLAYS];
	u32 contrast[MAX_DISPLAYS];
	u32 saturation[MAX_DISPLAYS];
	u32 hue[MAX_DISPLAYS];
	u32 y_gain[MAX_DISPLAYS];
	u32 u_gain[MAX_DISPLAYS];
	u32 v_gain[MAX_DISPLAYS];
	struct OVERLAY_PARAM param;
	struct SCREEN_AREA screen_size;
	OSD_DATA_T osd_param;

	/* we don't allow to change image fmt/size once buffer has
	 * been allocated
	 */
	int buffer_allocated;
	/* allow to reuse previously allocated buffer which is big enough */
	int buffer_size;
	/* keep buffer info across opens */
	unsigned long buf_virt_addr[VIDEO_MAX_FRAME];
	unsigned long buf_phy_addr[VIDEO_MAX_FRAME];

	/* we don't allow to request new buffer when old buffers are
	 * still mmaped
	 */
	int mmap_count;

	spinlock_t vbq_lock;		/* spinlock for videobuf queues */
	unsigned long field_count;	/* field counter for videobuf_buffer */

	/* non-NULL means streaming is in progress. */
	bool streaming;
	bool enable;
	bool datavalid;

	struct v4l2_pix_format pix;
	struct v4l2_rect crop;
	struct v4l2_window win;
	struct v4l2_window extwin;
	struct v4l2_framebuffer fbuf;

	enum color_encoding encoding;
	enum color_range range;

	/* Lock to protect the shared data structures in ioctl */
	struct mutex lock;

	/* V4L2 control structure for different control id */
	struct v4l2_control control[MAX_CID];
	bool mirror;
	int flicker_filter;
	/* V4L2 control structure for different control id */

	int bpp; /* bytes per pixel */

	unsigned int smsshado_size;
	unsigned char pos;

	int ps, vr_ps, line_length, first_int, field_id;
	enum v4l2_memory memory;
	struct videobuf_buffer *cur_frm, *next_frm;
	struct list_head dma_queue;
	u8 *queued_buf_addr[VIDEO_MAX_FRAME];
	s32 tv_field1_offset;
	void *isr_handle;

	/* Buffer queue variables */
	struct atc_vout_device *vout;
	enum v4l2_buf_type type;
	struct videobuf_queue vbq;
	int io_allowed;

};

void atc_vout_free_buffers(struct atc_vout_device *vout);
int vidioc_overlay(struct file *file, void *fh, unsigned int on);

#endif	/* ifndef ATC_VOUTDEF_H */
