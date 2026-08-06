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


#ifndef AVIN_COM_STRUCT_
#define AVIN_COM_STRUCT_


enum avin_signal_status {
	SIGNAL_NONE = 0,
	SIGNAL_READY,
	SIGNAL_LOST,
	SIGNAL_CHANGE_START,
	SIGNAL_CHANGE_DONE,
	SIGNAL_CONNECTING,
	SIGNAL_IDLE
};

enum hdmi_device_type {
	DEVICE_TYPE_NULL,
	DEVICE_TYPE_MHL = 0x16,
	DEVICE_TYPE_HDMI = 0x17
};


enum avin_device_type {
	AVIN_TYPE_CVBS_VIDEO = 0,
	AVIN_TYPE_CVBS_AUDIO,
	AVIN_TYPE_DIGITAL_AUDIO,
	AVIN_TYPE_DIGITAL_VIDEO,
	AVIN_TYPE_YPBPR,
	AVIN_TYPE_HDMI,
	AVIN_TYPE_BACKCAR,
#ifdef CONFIG_CVBS_CAMERA_ENABLE
	AVIN_TYPE_CVBS_CAMERA,
#endif
#ifdef CONFIG_AVM_ENABLE
	AVIN_TYPE_AVM_FRONT,
	AVIN_TYPE_AVM_REAR,
	AVIN_TYPE_AVM_LEFT,
	AVIN_TYPE_AVM_RIGHT,
	AVIN_TYPE_AVM_SIGVIEW,
#endif
	AVIN_TYPE_MAX
};

struct yc_addr_t {
	unsigned int y;
	unsigned int c;
};

struct capture_priv {
	struct yc_addr_t        ycaddr;
	unsigned int            buf_width;
	unsigned int            buf_height;
	unsigned int            di_flags;
	bool                    need_hide;
	enum avin_signal_status signal_status;
	enum hdmi_device_type   hdmi_dev_type;
};

typedef struct _SIGNAL_MESSAGE {
	bool signal_status;
	int  format;
} SIGNAL_MESSAGE;


/*The command to set parameter*/
#define V4L2_CID_SET_MIRROR		(V4L2_CID_PRIVATE_BASE + 0)
#define V4L2_CID_SET_FMT		(V4L2_CID_PRIVATE_BASE + 10)


/*The command to get parameter*/
#define V4L2_CID_GET_SIGNAL_MESSAGE     (V4L2_CID_PRIVATE_BASE + 50)
#define V4L2_CID_GET_HDMI_MHL_DEVICE    (V4L2_CID_PRIVATE_BASE + 100)
#define V4L2_CID_GET_HDMI_MHL_VDORECT   (V4L2_CID_PRIVATE_BASE + 101)
#define V4L2_CID_GET_ORIENTATION        (V4L2_CID_PRIVATE_BASE + 102)
#define V4L2_CID_GET_HDMI_MHL_VDORECT0  (V4L2_CID_PRIVATE_BASE + 103)
#define V4L2_CID_GET_HDMI_MHL_VDORECT1  (V4L2_CID_PRIVATE_BASE + 104)
#define V4L2_CID_GET_HDMI_MHL_VDORECT2  (V4L2_CID_PRIVATE_BASE + 105)
#define V4L2_CID_GET_HDMI_MHL_VDORECT3  (V4L2_CID_PRIVATE_BASE + 106)
#define V4L2_CID_GET_HDMI_MHL_SIGNAL    (V4L2_CID_PRIVATE_BASE + 107)


#endif
