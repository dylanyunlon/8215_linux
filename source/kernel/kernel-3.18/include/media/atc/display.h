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
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#ifndef __ARM2__
#include <linux/ioctl.h>
#endif
#include "types.h"
//#include <generated/atc_project.h>


#define MONITOR_VDO_FPS 1
#define KERNEL_STANDARD_API

#define VOUT_DI_FUN_EN            (1)
#define VOUT_DI_SRC_PROG          (0 << 1) /* progressive source*/
#define VOUT_DI_SRC_INTER_FRM     (1 << 1) /* interlace frame source*/
#define VOUT_DI_SRC_INTER         (1 << 2) /* interlace source*/
#define VOUT_DI_SRC_MASK          (3 << 1)
#define VOUT_DI_TOP_FLD_FIRST     (1 << 3) /* top filed first*/
#define VOUT_DI_RPT_FIRST_FLD     (1 << 4) /* repeat first filed*/
#define VOUT_DI_PROG_SEQ          (1 << 5) /* progressive sequece*/
#define VOUT_DI_SEEK_LOCATE       (1 << 6) /* seek locate*/
#define VOUT_DI_FF_RW             (1 << 7) /* fast forward*/
#define VOUT_DI_PD                (1 << 8)  /* pull down flag*/

#define VOUT_BG_OUTPUT_EN         (1 << 16)
#define  BOOT_ANIMATION_OSD_PLANE    OSD_PLANE_1

enum {
	SRC_COLOR_KEY,
	DST_COLOR_KEY,
};

struct OSD_INFO {
	__s32 posx;
	__s32 posy;
	__s32 width;
	__s32 height;
};

struct VDP_ALPHA {
	__s32 enable;
	__s32 inalpha;
	__s32 outalpha;
};

struct VOUT_PARAM {
	__u32 y_phy_addr;
	__u32 c_phy_addr;
	__u32 device_name;
	__u32 duration;
	__u32 disp_flags;
};

struct OSD_PARAM {
	__u32 buf_phy_addr;
};

struct OVERLAY_PARAM {
	__u32 u4Idx;
	__u32 u4SrcWidth;
	__u32 u4SrcHeight;
	RECT   rSrcRect;
	RECT   rDstRect;
	__u32 u4PhysicalAddressY;
	__u32 u4PhysicalAddressC;
	__u32 u4Status;
	__u32 u4Flags;
	__u32 device_name;
	/*De-interlace interface*/
	__u32 u4Duration;
	bool   fgProgSrc;
	bool   fgTopFiledFirst;
	bool   fgRepeatFirstField;/* first field 1, and second field 0*/
	bool   fgProgSeq;         /* Progressive sequece*/
	bool   fgPullDownFlagValid;
};

struct FRONT_REAR_PARAM {
	__u32 u4PreIdx;
	__u32 u4Idx;
	__u32 u4PreFlags;
	__u32 u4Flags;
	__u32 device_name;
};

struct FMT_BG_PARAM {
	__u32 u4Idx;
	__u32 u4Color;
	bool   fgEnable;
};

struct SCREEN_AREA {
	__u32 width;
	__u32 height;
};


typedef struct {
	__u32 u4TopLeftXMargin;
	__u32 u4TopLeftYMargin;
	__u32 u4BottomRightXMargin;
	__u32 u4BottomRightYMargin;
}CVBS_OUTPUT_MARGIN_T;

typedef struct _ACTIVE_OFFSET_VALUE
{
	__u32	   u4OffsetX;
	__u32	   u4OffsetY;
} ACTIVE_OFFSET_VALUE;

typedef struct _PANEL_SETTING_ARGS_T
{
	__u32      u4PanelIDNum;
	__u32      u4PanelWidth;
	__u32      u4PanelHeight;
	__u32      u4TypeClock;  //from panel spec
	__u32      u4OsdHTotal;  //from panel spec
	__u32      u4OsdVTotal;  //from panel spec
	ACTIVE_OFFSET_VALUE     rOsdStart;
	ACTIVE_OFFSET_VALUE     rSclStart;
	ACTIVE_OFFSET_VALUE     rSclSdShift;
	ACTIVE_OFFSET_VALUE     rSclHdShift;
	ACTIVE_OFFSET_VALUE     rFmtStart;
	ACTIVE_OFFSET_VALUE     rTconOffset;
}PANEL_SETTING_ARGS_T;

typedef struct _WINDOW_SETTING
{
	__s32      i4H0EndMargin;
	__s32      i4V0EndMargin;
	__s32      i4H1StartShift;
	__s32      i4V1StartShift;
	__s32      i4H1EndMargin;
	__s32      i4V1EndMargin;
}WINDOW_SETTING_T;

typedef struct _VDO_WINDOW_SETTING
{
	__u32 u4FmtSdHstart;
	__u32 u4FmtSdVstart;
	WINDOW_SETTING_T rFmtWindowSetting;
	WINDOW_SETTING_T rSclWindowSetting;
}VDO_WINDOW_SETTING_T;
typedef struct _FB_CONFIG {
	__u32 u4VideoMode;
	//__u32 u4Tm070ddhg;/*used for checking wether current LCD is TM070DDHG LCD.1:yes,0:no.*/
	CVBS_OUTPUT_MARGIN_T rCVBSMargin;
	__u32 u4LogoBpp;
	PANEL_SETTING_ARGS_T rFBPanelSetting;
	VDO_WINDOW_SETTING_T rVdoWindowSetting;
} FB_CONFIG_T;

#ifdef __ARM2__
/*FIXME: we should not define variables in header file. just for ARM2.*/
static PANEL_SETTING_ARGS_T g_rLCDPanelSetting[] =
{
	//ID, width, height, clk, Htotal, Vtotal,   Osd{X,Y},    sclActive{X,Y,},    sclSD{X,Y,},   sclHD{X,Y,},    FmtHD{X,Y},     TCON{X,Y}
	{0, 800,  480, 300, 928,  525, {0x59,0x15}, {0x65,0x16}, {0x10,0x17}, {0x11,0x14}, {0x66,0x2c}, {0x6a,0x16}}, // 800*480
	{0, 1024, 600, 512, 1344, 635, {0x5a,0x0e}, {0x64,0x10}, {0x10,0x28}, {0xe,0x08}, {0x63,0x1a}, {0x6c,0x0f}}, // 1024*600
	{1, 1024, 600, 512, 1344, 635, {0x93,0x1a}, {0x9d,0x1a}, {0x10,0x28}, {0x10,0x08}, {0x9d,0x24}, {0xa2,0x1a}}, //  1024*600 TM
	{0, 1024, 768, 648, 2084, 800, {0x57,0x0e}, {0x65,0x10}, {0x10,0x38}, {0x10,0x0a}, {0x63,0x1a}, {0x68,0x0f}}, // 1024*768
	{0, 1280, 720, 673, 1426, 788, {0x52,0x1e}, {0x61,0x20}, {0x10,0x23}, {0x13,0x0b}, {0x61,0x2b}, {0x63,0x1f}}, // 1280*720
	{0, 1280, 800, 756, 1441, 875, {0x71,0x14}, {0x7d,0x15}, {0x10,0x35}, {0x33,0x04}, {0xa0,0x1c}, {0x82,0x17}}, // 1280*800
	{0, 800, 1280, 756, 930, 1356, {0x71,0x14}, {0x7d,0x15}, {0x10,0x35}, {0x15,0x04}, {0x88,0x3c}, {0x82,0x17}}, //800*1280---
	//{0, 1920, 720, 966, 2046, 788, {0x57,0x0c}, {0x65,0x0d}, {0x10,0x35}, {0x10,0x0b}, {0x63,0x1a}, {0x68,0x0d}}, //1920*720,CLK=966
	//{1, 1920, 720, 769, 2024, 759, {0x57,0x0c}, {0x65,0x0d}, {0x10,0x36}, {0x10,0x0b}, {0x63,0x1a}, {0x68,0x0d}}, //1920*720,CLK=769
	{0, 1920, 720, 800, 2048, 781, {0x57,0x0c}, {0x65,0x0d}, {0x10,0x2a}, {0x10,0x0b}, {0x63,0x1a}, {0x68,0x0d}}, //1920*720,CLK=800
};
#endif

#define MAX_FENCE_CNT   10
typedef struct _FB_SYNC
{
    UINT32 u4AcquireFenceCnt;
    int *pru4AcquireFence;
    int *pru4ReleaseFence;
}FB_SYNC_T;

typedef struct _OSD_DATA_T {
	__u32 u4Flags;
	__u32 u4SrcColorKey;
	RECT   rDestRect;
	__u32 u4PixelFormat;
	__u32 u4OutputPath;
	__u32 u4BitmapPA;/*phiscial address*/
	__u32 u4BlockID;
	__u32 u4Width;
	__u32 u4Height;
	__u32 device_name;
} OSD_DATA_T;

typedef struct {
	void *srcdata;
	void *dstdata;
	__s32 srcWidth;
	__s32 srcHeight;
	__s32 dstWidth;
	__s32 dstHeight;
	__u32 ycbuf[2];
} imgrez_data;

typedef struct YUV420BLOCK_TO_ARGB8888_BUF_T {
	__u32 ycbuf[2];
	void *vaddr;
	__s32 bufwidth;
	__s32 bufheight;
	__s32 picwidth;
	__s32 picheight;
	__s32 dstWidth;
	__s32 dstHeight;
	__u32 u4DestARGB8888Pa;
        bool fgBlock;
} YUV420BLOCK_TO_ARGB8888_BUF_T;

typedef struct YUV420BLOCK_TO_NV12_BUF_T {
	__u32 srcbuf[2];
	__s32 srcwidth;
	__s32 srcheight;
	__s32 srcbufwidth;
	__s32 srcbufheight;
	__u32 dstbuf[2];
	__s32 dstwidth;
	__s32 dstheight;
	__s32 dstbufwidth;
	__s32 dstbufheight;
}YUV420BLOCK_TO_NV12_BUF_T;

enum {
	FRONT_VDO = 0x00,
	REAR_VDO   = 0x01,
	BACKCAR_VDO = 0x02,
};

typedef enum {
	USB     = 0xa,
	DVD     = 0xb,
	AVIN_1  = 0xc,
	BACKCAR = 0xd,
	YPBPR   = 0xe,
	VGA     = 0xf,
	DGI     = 0x10,
	VDO_HDMI    = 0x11,
	AVIN_2  = 0x12,
	AVIN_3  = 0x13,
	AVIN_4  = 0x14,
	AVIN_5  = 0x15,
} VIDEO_SRC_TYPE;

#define srcMax 12 /*the number of VIDEO_SRC_TYPE's members*/

typedef struct _CP_CONFIG {
	VIDEO_SRC_TYPE SrcType;
	__s32 i4GHue;
	__s32 i4YGain;
	__s32 i4UGain;
	__s32 i4VGain;
	__s32 i4Contr;
	__s32 i4Brit;
	__s32 i4Satr;
} CP_CONFIG;

#define VDP_BLOCK_MODE       0x0
#define VDP_SCANLINE_MODE    0x1 /* 1 is scanline mode and 0 is block mode */
#define VDP_FRAMELOCK_MODE   0x2
#define VDP_TOP_LEVEL        0x04
#define VDP_ENABLE_DEINT     0x1000
#define VDP_PLAY_FF_RW       0x2000
#define VDP_SEEK_LOCATE      0x4000
#define VDP_ENABLE_PDDI      0x8000
#define VDP_UPDATE_OVERLAY   0x80000
#define VDP_FLIP_ADDRESS     0x40000


#define VDP_ROTATE_90		 0x040
#define VDP_ROTATE_180		 0x020
#define VDP_ROTATE_270		 0x080


#define SUBTITLE_IOC_BASE 'w'
#define OSD_BASE      'O'

#ifndef VIDIOC_QBUF
#ifdef __ARM2__
#define VIDIOC_QBUF                     (0xC044560F) /*_IOWR('V', 15, struct OVERLAY_PARAM)*/
#endif
#endif
#ifndef VIDIOC_STREAMON
#define VIDIOC_STREAMON               	(0x40045612) //_IOWR('V', 18, struct OVERLAY_PARAM)
#endif
#ifndef VIDIOC_STREAMOFF
#define VIDIOC_STREAMOFF                (0x40045613) /*_IOWR('V', 19, struct OVERLAY_PARAM)*/
#endif

#ifdef CONFIG_ATC_OS_linux
#ifndef VIDIOC_OVERLAY
#define VIDIOC_OVERLAY                (0x40045614) /*_IOWR('V', 20, struct OVERLAY_PARAM)*/
#endif
#endif

#ifndef __ARM2__
#define STIOC_SETOSDINFO                _IOWR(SUBTITLE_IOC_BASE, 0, struct OSD_INFO)
#define STIOC_SETOSD                    _IOWR(SUBTITLE_IOC_BASE, 1, unsigned long)
#define STIOC_SET_DST_CK                _IOWR(SUBTITLE_IOC_BASE, 2, __s32)
#define STIOC_SET_VDO_ALPHA             _IOWR(SUBTITLE_IOC_BASE, 3, struct VDP_ALPHA)
#define STIOC_SETOSD_8_BIT_PALETTE      _IOWR(SUBTITLE_IOC_BASE, 4, unsigned long)
#define STIOC_SETOSD_HIDE               _IOWR(SUBTITLE_IOC_BASE, 5, unsigned long)
#define STIOC_SET_FMT_BLACK             _IOWR(SUBTITLE_IOC_BASE, 6, unsigned long)
#define STIOC_SET_SRC_CLR_KEY           _IOWR(SUBTITLE_IOC_BASE, 7, unsigned long)
#define STIOC_TVD2TVE                   _IOWR(SUBTITLE_IOC_BASE, 8, __s32)
#define STIOC_SET_DST_RECT              _IOWR(SUBTITLE_IOC_BASE, 9, unsigned long)
#define STIOC_SET_VDP_PARAM             _IOWR(SUBTITLE_IOC_BASE, 10, struct OVERLAY_PARAM)
#define VIDIOC_QBUF                     _IOWR(SUBTITLE_IOC_BASE, 17, struct OVERLAY_PARAM)
#define YUV420_TO_YC_IOCTL              _IOWR(SUBTITLE_IOC_BASE, 11, imgrez_data)
#define STIOC_GET_VDP_SYNC_STATUS       _IOWR(SUBTITLE_IOC_BASE, 12, __u32)/*mtk94106 add*/
#define STIOC_SET_PLANE_ORDER           _IOWR(SUBTITLE_IOC_BASE, 13, unsigned long)
#define STIOC_RESET_PLANE_ORDER         _IOWR(SUBTITLE_IOC_BASE, 14, unsigned long)
#define STIOC_SET_FRONT_REAR_PARAM      _IOWR(SUBTITLE_IOC_BASE, 15, struct FRONT_REAR_PARAM) /*mtk94098*/
#define STIOC_SET_SUB_FLAG              _IOWR(SUBTITLE_IOC_BASE, 16, __s32) /*mtk94020*/
#define STIOC_SET_COLOR_RANGE           _IOWR(SUBTITLE_IOC_BASE, 17, __u32)
#define STIOC_SET_COLOR_ENCODING        _IOWR(SUBTITLE_IOC_BASE, 18, __u32)
#define STIOC_GET_PHY_ACTIVE            _IOWR(SUBTITLE_IOC_BASE, 19, struct SCREEN_AREA)

#endif
#define  BUFFER_FENCE_SYNC             0x1008

#define OSD_DATA_SRC_CLR_KEY            0x01
#define OSD_DATA_ALPHA_BLENDING         0x02
#define OSD_DATA_OUPPUT_PATH            0x04
#define OSD_DATA_BITMAP_PA              0x08
#define OSD_DATA_SET_DEST_RECT          0x10
#define YUV420_BLOCK_TO_ARGB8888        0x0002000a
#define YUV420_BLOCK_TO_NV12			0x0002000b
#define TVE_TURNON                      0x1002


enum {
	OSD_PIXEL_FORMAT_RGB565   = 0xb,
	OSD_PIXEL_FORMAT_ARGB8888 = 0xe,
};



/* OSD Memory info*/

#define OSD_BLOCK_SIZE   0x100000

#define OVERLAY_OSD_SET_SRC_CLRKEY      _IOWR(OSD_BASE, 0, __s32)
#define OVERLAY_OSD_SET_ALPHA_BLENDING  _IOWR(OSD_BASE, 1, __s32)
#define OVERLAY_OSD_SHOW                _IOWR(OSD_BASE, 2, __s32)
#define OVERLAY_OSD_HIDE                _IOWR(OSD_BASE, 3, __s32)
#define OVERLAY_OSD_SET_SRCRECT         _IOWR(OSD_BASE, 4, __s32)
#define OVERLAY_OSD_GET_SRCRECT         _IOWR(OSD_BASE, 5, __s32)
#define OVERLAY_OSD_SET_OVERLAY_PARAM   _IOWR(OSD_BASE, 6, __s32)
#define OVERLAY_OSD_SET_DSTRECT         _IOWR(OSD_BASE, 7, __s32)
#define OVERLAY_OSD_GET_DSTRECT         _IOWR(OSD_BASE, 8, __s32)
#define OVERLAY_OSD_REQUEST_LAYER       _IOWR(OSD_BASE, 9, __s32)
#define OVERLAY_OSD_RELEASE_LAYER       _IOWR(OSD_BASE, 10, __s32)



typedef struct _OSD_OVERLAY_PARAM_T {
	__s32 i4PixelFormat;
	__s32 i4Width;
	__s32 i4Height;
} OSD_OVERLAY_PARAM_T;


typedef struct _OSD_CTL {
	__s32  i4Plane;
	__s32  i4Value;
} OSD_CTL_T;
/*setParameter  param define*/
#define SET_DST_COLOR_KEY               0x01
#define SET_VIDEO_ALPHA                 0x02
#define SET_OSD_HIDE                    0x03
#define SET_SRC_CLK_KEY                 0x04
#define SET_TVD_TO_TVE                  0x05
#define SET_OSD_OUTPUT_PATH             0x06
#define SET_OSD_USING_PRIVATE_DEST_RECT   0x07

#define OVERLAY_FLAG_YUV_MASK					(3 << 1)
#define OVERLAY_FLAG_YUV_BT601					(0 << 1)
#define OVERLAY_FLAG_YUV_BT709					(1 << 1)
#define OVERLAY_FLAG_YUV_NARROW					(0 << 2)
#define OVERLAY_FLAG_YUV_WIDE					(1 << 2)


#define DISPLAY_GET_LOGO_BPP                         (0x00020027) //for linux logo bpp

extern void set_colorkey(__u32 type, __u32 colorkey, __u32 enable);
extern void vSetBacklightIntensity(__u32 u4Value);
#ifdef AC823X_CONFIG
extern void set_pool_param(__u64 base, __u64 va, __u32 size);
#else
extern void set_pool_param(__u32 base, __u32 va, __u32 size);
#endif
#ifdef __ARM2__
typedef u32 phys_addr_t;
extern void WchSetSourceBaseAddr(phys_addr_t wchReservebase);
#endif

#endif


