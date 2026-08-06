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

#ifndef _WCH_DRV_H_
#define _WCH_DRV_H_

#include "../../../drivers/misc/atc/inc/x_typedef.h"
#ifndef __ARM2__
#include <linux/types.h>
#endif
#include <generated/atc_project.h>
/******************************************************************
*
*             STRUCT & ENUM & MACRO DECLARATION SECTION
*
*******************************************************************/

#define WCH_CFG_UNMIRROR        0
#define WCH_CFG_MIRROR_H        1
#define WCH_CFG_MIRROR_V        2
#define WCH_CFG_MIRROR_HV       (WCH_CFG_MIRROR_H | WCH_CFG_MIRROR_V)

#define WCH_Y_SEL_MASK          0x1
#define WCH_U_SEL_MASK          0x2
#define WCH_V_SEL_MASK          0x4
#define WCH_YUV_SEL_MASK        (WCH_Y_SEL_MASK | WCH_U_SEL_MASK | WCH_V_SEL_MASK)

#define WCH_UV_SWAP_MASK        0x8
#define WCH_YC_SWAP_MASK        0x10

#define WCH_DUMP_FRAMES_BY_ICE  0

#if WCH_DUMP_FRAMES_BY_ICE
#define WCH_SD_BUF_CNT          20
#else
#define WCH_SD_BUF_CNT          5 /* For SD interlace source (DI buffer count)*/
#endif

#define WCH_HD_BUF_CNT           3  /* For HD or SD progressive source*/
#define WCH_BUF_MAX_CNT          WCH_SD_BUF_CNT

#define WCH_STOP_IN_VSYNC        1
#define WCH_CLOSE_IN_VSYNC       2

#define WCH_SPECIAL_DGI_ENABLE      1		/* For avoiding ARM2 backcar write channel be graped by ARM1 DGI */

enum {
	WCH_SUCCESS = 0,
	WCH_CONTEXT_NULL = 0xF0000000,
	WCH_ACCESS_FAIL,
	WCH_COPYFROMUSER_FAIL,
	WCH_INVALID_APP_ID,
	WCH_INVALID_INPUT_PARAM,
	WCH_INVALID_OUTPUT_PARAM,
	WCH_INVALID_CTLCODE,
	WCH_NO_FREE_HW,
	WCH_ALLOC_BUF_FAIL,
	WCH_GET_ADDR_FAIL,
	WCH_COMMON_FAIL,
	WCH_BACKUP_SOURCE,
	WCH_NOT_START,
	WCH_ERROR_MAX
};

typedef enum  {
	SRC_APP_UNKNOWN = 0,
	SRC_APP_BACKCAR,
	SRC_APP_AVIN,
	SRC_APP_YPBPR,
	SRC_APP_VGA,
	SRC_APP_HDMI,
	SRC_APP_DGI,
	SRC_APP_DISPLAY,
#if WCH_SPECIAL_DGI_ENABLE
	SRC_APP_SPECIAL_DGI,     /* For avoiding ARM2 backcar write channel be graped by ARM1 DGI */
#endif
#ifndef __ARM2__
	SRC_APP_AVM,
#endif
	SRC_APP_MAX
} WCH_SRC_APP_ID_E;

typedef enum  {
	DATA_SRC_UNKNOWN = 0,
	DATA_SRC_HDMI,
	DATA_SRC_YPBPR,
	DATA_SRC_VGA,
	DATA_SRC_TVD,
	DATA_SRC_DGI,
	DATA_SRC_DVD,
	DATA_SRC_FMTR,
	DATA_SRC_FMTF,
	DATA_SRC_MIX,
} WCH_DATA_SRC_E;

typedef enum  {
	DATA_FMT_UNKNOWN = 0,
	DATA_FMT_YUV420,
	DATA_FMT_YUV422, /* HDMI */
	DATA_FMT_YUV444, /* TVD, YPbPr, VGA, Display, HDMI input */
	DATA_FMT_BT656,
	DATA_FMT_BT601,
} WCH_DATA_FMT_E;

typedef enum {
	SRC_FREQ_UNKNOWN = 0,
	SRC_FREQ_24HZ,
	SRC_FREQ_25HZ,
	SRC_FREQ_30HZ,
	SRC_FREQ_50HZ,
	SRC_FREQ_50HZ_V1250T,
	SRC_FREQ_60HZ,
	SRC_FREQ_100HZ,
	SRC_FREQ_120HZ,
	SRC_FREQ_200HZ,
	SRC_FREQ_240HZ,
} WCH_SRC_FREQ_E;

typedef enum {
	WCH_640_480P_60HZ = 0,
	WCH_1280_720P_60HZ,
	WCH_1920_1080I_60HZ,
	WCH_720_480P_60HZ,
	WCH_720_480I_60HZ, /* 720(1440) */
	WCH_1280_720P_50HZ,
	WCH_1920_1080I_50HZ, /* 1125 vertical total */
	WCH_720_576P_50HZ,
	WCH_720_576I_50HZ,
	WCH_720_240P_60HZ,

	WCH_2880_480I_60HZ,
	WCH_2880_240P_60HZ,
	WCH_1440_480P_60HZ,
	WCH_1920_1080P_60HZ,
	WCH_720_288P_50HZ,
	WCH_2880_576I_50HZ,
	WCH_2880_288P_50HZ,
	WCH_1440_576P_50HZ,
	WCH_1920_1080P_50HZ,
	WCH_1920_1080P_24HZ,

	WCH_1920_1080P_25HZ,
	WCH_1920_1080P_30HZ,
	WCH_2880_480P_60HZ,
	WCH_2880_576P_50HZ,
	WCH_1920_1080I_50HZ_1250T, /* 1250 vertical total */
	WCH_1920_1080I_100HZ,
	WCH_1280_720P_100HZ,
	WCH_720_576P_100HZ,
	WCH_720_576I_100HZ,
	WCH_1920_1080I_120HZ,

	WCH_1280_720P_120HZ,
	WCH_720_480P_120HZ,
	WCH_720_480I_120HZ,
	WCH_720_576P_200HZ,
	WCH_720_576I_200HZ,
	WCH_720_480P_240HZ,
	WCH_720_480I_240HZ,
	WCH_1440_576I_50HZ,
	WCH_MODE_NUM,          /* dummy mode, used to determine the last mode */
} WCH_TIMING_E;
typedef void (*get_wch_buffer_index)(u32 *bufindex);

typedef struct {
	WCH_DATA_SRC_E   eInputSrc;       /* specify the vdo in data source*/
	WCH_DATA_FMT_E   eInputFmt;
	u32           u4SrcWidth;      /* specify the src video Width of one filed */
	u32           u4SrcHeight;     /* specify the src video height of one filed */
	u32           u4SrcStartX;     /* specify the start pixel position of active video in horizontal direction */
	u32           u4SrcStartYTop;  /* specify the start position of active video in top field */
	u32           u4SrcStartYBot;  /* specify the start position of active video in bottom field */

	WCH_DATA_FMT_E   eOutputFmt;
	u32           u4DstWidth;      /* specify the dst video Width of one filed */
	u32           u4DstHeight;     /* specify the dst video height of one filed */
	u32           u4Mirror;
	u8            fgProgressive;
	u8            fgCanNotGrabbed; /* specify output to TV and backcar source can not grab*/

	u8            fgVSyncPolarity; /* TRUE is positive */
	u8            fgHSyncPolarity; /* TRUE is positive */
	u8            fgBotFieldFirst; /* TRUE is bottom first */

	u8            u1UVSwap;        /* Cr buffer connet with C need set 1*/
	u8            u1YUVMask;
	u8            u1YSel;
	u8            u1USel;
	u8            u1VSel;
	WCH_TIMING_E     eTiming;
	get_wch_buffer_index GetWchBufIndx;
} WCH_CFG_T, *PWCH_CFG_T;

typedef struct {
	WCH_SRC_APP_ID_E	eSrcId;
	WCH_CFG_T	 tWchCfg;
} WCH_CTL_PARAM_T, *PWCH_CTL_PARAM_T;

typedef struct {
	WCH_SRC_APP_ID_E	eSrcId;
	u8	_fgCanNotGrabbed;
} WCH_CTL_PAR_T, *PWCH_CTL_PAR_T;

typedef struct {
	u32	u4BufCnt;
	u32	u4YBuf[WCH_BUF_MAX_CNT];
	u32	u4CBuf[WCH_BUF_MAX_CNT];
} WCH_BUFF_INFO_T, *PWCH_BUFF_INFO_T;
//#if defined(CONFIG_ATC_OS_android)
typedef struct {
    WCH_SRC_APP_ID_E eSrcId;
    WCH_BUFF_INFO_T tWchBuf;
} WCH_BUF_T,*PWCH_BUF_T;
//#endif

#if defined(CONFIG_ATC_OS_linux)
#ifndef __ARM2__
struct _wchBuffer {
	dma_addr_t phyaddr;
	void *viraddr;
	unsigned int size;
};
#endif
#endif
/******************************************************************
*
*    EXPORT APIS DECLARATION
*
******************************************************************/

#define WCH_INVALID                       0xFF
#define WCH_1                             0
#define WCH_2                             1
#define WCH_BACKUP                        (WCH_2 + 1)
#define WCH_MAX                           (WCH_BACKUP + 1) /* (WCH_BACKUP + 1) */

/************  [ IOCTL Code ]  ***************/

#define IOCTL_WCH_OPEN        (0x80000001)
#define IOCTL_WCH_CLOSE       (0x80000002)
#define IOCTL_WCH_START       (0x80000003)
#define IOCTL_WCH_STOP        (0x80000004)
#define IOCTL_WCH_CONFIG      (0x80000005)
#define IOCTL_WCH_GET_ADDR    (0x80000006)
#define IOCTL_WCH_SET_MIRROR  (0x80000007)
#define IOCTL_WCH_SET_REARGRAB (0x80000008)
#if defined(CONFIG_ATC_OS_linux)
#define IOCTL_WCH_GET_VIRTUAL_ADDR    (0x8000000F)
#endif
/************  [ EVENT DATA ]  ***************/
#define NEW_WCH_EVENT_NAME     1
#if NEW_WCH_EVENT_NAME
#define WCH_BACKCAR_EVENT      "WCH_BACKCAR_FRMDONE"
#define WCH_AVIN_EVENT         "WCH_AVIN_FRMDONE"
#define WCH_YPBPR_EVENT        "WCH_YPBPR_FRMDONE"
#define WCH_VGA_EVENT          "WCH_VGA_FRMDONE"
#define WCH_HDMI_EVENT         "WCH_HDMI_FRMDONE"
#define WCH_DGI_EVENT          "WCH_DGI_FRMDONE"
#define WCH_DVD_EVENT          "WCH_DVD_FRMDONE"
#else
#define WCH1_FRAME_DONE_EVENT  "WCH1_FRM_DONE"
#define WCH2_FRAME_DONE_EVENT  "WCH2_FRM_DONE"
#endif
#endif
