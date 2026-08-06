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
 *Date: 2016-12-05
 */
 
/* wch data path:
			         
	BT656/601		         
	BT1120     ----------------->WCH8

	BT656/601----------------->WCH1
			
	TVD0---------------------->WCH1
	
	TVD1---------------------->WCH2
	
	TVD2---------------------->WCH3
	
	TVD3---------------------->WCH4

	TVD0/TVD1
	TVD2/TVD3----------------->WCH5

	YPBPR/VGA----------------->WCH6

	HDMI---------------------->WCH7

	VOUT---------------------->WCH9

*/
#ifndef _WCH_DRV_H_
#define _WCH_DRV_H_
#ifdef __ARM2__
#include "x_bim_83xx.h"
#else
#include <linux/spinlock.h>
#include <linux/spinlock_types.h>
#endif

#define WCH_SYNC_BUFFER 0

#define WCH_DUMP_FIRST_20_FRAMES_BY_ICE 	0
#define WCH_OPEN_PATTEN				0

#define WCH_SUPPORT_AVM_480P 	0

#define WCH_NUM		9

#define VIRT_TO_BUS(vaddr)	(((unsigned long)__pa(vaddr)) - 0x100000000L)
#define BUS_TO_VIRT(baddr)	__va((unsigned long)(baddr) + 0x100000000L)

#ifdef __ARM2__

#define NULL (void *)(0)

#endif


typedef struct _wchLock {
#ifndef __ARM2__
	spinlock_t lock;
#else
	unsigned int lock;
#endif
	unsigned long flags;
	unsigned int u4LockCnt;
} wchLock;

#define WCH_1	0 /*tvd0&dgi_2*/
#define WCH_2	1 /*tvd1*/
#define WCH_3	2 /*tvd2*/
#define WCH_4	3 /*tvd3*/
#define WCH_5	4 /*tvd0&tvd1&tvd2&tvd3*/
#define WCH_6	5 /*vga*/
#define WCH_7	6 /*hdmi*/
#define WCH_8	7 /*dgi_1*/
#define WCH_9	8 /*vout*/

typedef enum  {
	DATA_SRC_UNKNOWN = 0,
	DATA_SRC_BT656_601,
	DATA_SRC_BT1120,
	DATA_SRC_TVD0,
	DATA_SRC_TVD1,
	DATA_SRC_TVD2,
	DATA_SRC_TVD3,
	DATA_SRC_YPBPR,
	DATA_SRC_VGA,
	DATA_SRC_HDMI,
	DATA_SRC_VOUT,
	DATA_SRC_MIX,
} WCH_DATA_SRC_E;

typedef enum  {
	DATA_FMT_UNKNOWN = 0,
	DATA_FMT_YUV420,
	DATA_FMT_YUV422,
	DATA_FMT_YUV444,
	DATA_FMT_BT656,
	DATA_FMT_BT601,
	DATA_FMT_BT1120,
} WCH_DATA_FMT_E;


#if WCH_DUMP_FIRST_20_FRAMES_BY_ICE
#define WCH_BUF_MAX_CNT		20
#else
#define WCH_BUF_MAX_CNT		5
#endif //WCH_DUMP_FIRST_20_FRAMES_BY_ICE

typedef struct {
	unsigned int u4BufCnt;
#if WCH_SYNC_BUFFER
	unsigned int u4BufFlag;
	bool fgUseBackupBuffer;
#endif
	unsigned long u4YBuf[WCH_BUF_MAX_CNT];
	unsigned long u4CBuf[WCH_BUF_MAX_CNT];
} WCH_BUFF_INFO_T, *PWCH_BUFF_INFO_T;


typedef enum  {
	SRC_APP_UNKNOWN = 0,
	SRC_APP_AVM_WCH1,
	SRC_APP_AVM_WCH2,
	SRC_APP_AVM_WCH3,
	SRC_APP_AVM_WCH4,
	SRC_APP_AVM_WCH5,
	SRC_APP_BACKCAR_WCH1,
	SRC_APP_BACKCAR_WCH5,
	SRC_APP_AVIN_WCH1,
	SRC_APP_AVIN_WCH5,
	SRC_APP_YPBPR,
	SRC_APP_VGA,
	SRC_APP_HDMI,
	SRC_APP_DGI656_WCH1,
	SRC_APP_DGI656_WCH8,
	SRC_APP_DGI601_WCH1,
	SRC_APP_DGI601_WCH8,
	SRC_APP_DGI1120,
	SRC_APP_VDO,
	SRC_APP_MAX
} WCH_SRC_APP_ID_E;


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

typedef void (*get_wch_buffer_index)(unsigned int *bufindex);
typedef struct {
	unsigned char u1WchId;
	WCH_SRC_APP_ID_E eSrcId;
	WCH_DATA_SRC_E eInputSrc;       /* specify the vdo in data source*/
	WCH_DATA_FMT_E eInputFmt;
	unsigned int u4SrcWidth;      /* specify the src video Width of one filed */
	unsigned int u4SrcHeight;     /* specify the src video height of one filed */
	unsigned int u4SrcStartX;     /* specify the start pixel position of active video in horizontal direction */
	unsigned int u4SrcStartYTop;  /* specify the start position of active video in top field */
	unsigned int u4SrcStartYBot;  /* specify the start position of active video in bottom field */

	WCH_DATA_FMT_E eOutputFmt;
	unsigned int u4DstWidth;      /* specify the dst video Width of one filed */
	unsigned int u4DstHeight;     /* specify the dst video height of one filed */
	unsigned int u4Mirror;
	unsigned int u4ScanLineMode;
	unsigned int u4HalfSample;
	unsigned char fgProgressive;
	unsigned char fgCanNotGrabbed; /* specify output to TV and backcar source can not grab*/

	unsigned char fgVSyncPolarity; /* TRUE is positive */
	unsigned char fgHSyncPolarity; /* TRUE is positive */
	unsigned char fgBotFieldFirst; /* TRUE is bottom first */

	unsigned char u1UVSwap;        /* Cr buffer connet with C need set 1*/
	unsigned char u1YUVMask;
	unsigned char u1YSel;
	unsigned char u1USel;
	unsigned char u1VSel;
	unsigned char u1CInDelay;
	get_wch_buffer_index GetWchBufIndx;
	WCH_TIMING_E     eTiming;
	
#if WCH_SUPPORT_AVM_480P
	bool fgSupportAVM480P;
#endif
} WCH_CFG_T, *PWCH_CFG_T;

/*
typedef struct {
	WCH_SRC_APP_ID_E eSrcId;
	WCH_CFG_T tWchCfg;
} WCH_CTL_PARAM_T, *PWCH_CTL_PARAM_T;
*/

typedef struct {
    unsigned char u1WchId;
    WCH_BUFF_INFO_T tWchBuf;
} WCH_BUF_T,*PWCH_BUF_T;

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
	WCH_WRONG_ID,
	WCH_ALLOC_BUF_FAIL,
	WCH_STOP_BY_OTHER,
	WCH_GET_ADDR_FAIL,
	WCH_CONFIG_FAIL,
	WCH_START_FAIL,
	WCH_STOP_FAIL,
	WCH_CLOSE_FAIL,
	WCH_COMMON_FAIL,
	WCH_BACKUP_SOURCE,
	WCH_NOT_START,
	WCH_ERROR_MAX
};

#endif
