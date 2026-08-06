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

#ifndef _WCH_IF_H_
#define _WCH_IF_H_

#include "media/atc/wch_drv.h"
#ifndef __ARM2__
#include <linux/interrupt.h>
#endif
#include <generated/atc_project.h>

#define WCH_HDMI_TIMING_EN       1   /* 1-use write channel timing; 0-use hdmi source timing */
#define WCH_SKIP_BUFF_CNT        4
#define WCH_GRAB_FUN_EN          1   /* 1-support backup grab write channel source, and enable need to change WCH_MAX */

#define MONITOR_UPDATEBUF_TIME   0
#define WCH_TEST_THREAD          0
#define WCH_DUMP_BUFF            0

#define WCH_DUMP_BUFFER_ATTR	 1

#define WCH_HW_FREE              0
#define WCH_HW_READY             1
#define WCH_HW_CONFIG            2
#define WCH_HW_START             3
#define WCH_HW_STOP              4
#define WCH_HW_PENDING           5  /* For wait write channel hw free*/

#define WCH_SD_YBUF_SIZE         (720 * 576)
#define WCH_SD_CBUF_SIZE         (720 * 576 / 2)
/* #define WCH_VGA_YBUF_SIZE        (800 * 608) */
/* #define WCH_VGA_CBUF_SIZE        (800 * 608 / 2) */
#define WCH_VGA_YBUF_SIZE        (1280 * 768)
#define WCH_VGA_CBUF_SIZE        (1280 * 768 / 2)

#define WCH_HDMI_YBUF_SIZE       (1920 * 1088)
#define WCH_HDMI_CBUF_SIZE       (1920 * 1088 / 2)
#define WCH_DISP_YBUF_SIZE       (1280 * 800)
#define WCH_DISP_CBUF_SIZE       (1280 * 800 / 2)

#define WCH_MEM_TVD_SIZE          (0x00300000) /* 5*720*576*3/2  */
#define WCH_MEM_YPBPR_VGA_SIZE    (0x00800000) /* 5*720*576*3/2 YPBPR; 3*800*600*3/2 VGA */
//#define WCH_MEM_HDMI_SIZE         (0x00900000) /* 3*1920*1080*3/2 HD; 5*720*576*3/2 SD */
#define WCH_MEM_HDMI_SIZE         (0x00f00000) /* 5*1920*1088*3/2 HD*/
#define WCH_MEM_DISP_SIZE         (0x00480000) /* 3*1280*800*3/2 FRONT; 3*720*576*3/2 REAR */
#if defined(CONFIG_ATC_OS_android)
#define WCH_MEM_DGI_SIZE          (0x00300000)//(0x00300000) /* 5*720*576*3/2 */
#elif defined(CONFIG_ATC_OS_linux)
#define WCH_MEM_DGI_SIZE          (0x00300000) /* 5*720*576*3/2 */
#endif
#define WCH_AVM_YBUF_SIZE	(368*576*4)
#define WCH_AVM_CBUF_SIZE	(368*576*4)

#define WCH_WIDTH_ALIGN          16
#define WCH_WIDTH_ALIGN_SHIFT    4
#define WCH_HEIGHT_ALIGN         32
#define WCH_HEIGHT_ALIGN_SHIFT   5

#define IsAlign(length, shift)   ((length - ((length >> shift) << shift)) ? 0 : 1)

#define WCH_BUFFER_MASK          0xFF

typedef struct {
	u32           u4Status;      /* specify write channel hw status*/
	u32           u4BufIdx;
	WCH_BUFF_INFO_T  tWchBuf;
	WCH_SRC_APP_ID_E eWchSrcId;
	WCH_CFG_T        tWchCfg;
} WCH_IF_PARAM_T, *PWCH_IF_PARAM_T;

typedef struct WCHTIMING {
	WCH_TIMING_E eTiming; /* Timing Mode */
	u16 u2HsyncInv;   /* Horizontal active in hsync low range, otherwise invert hsync */
	u16 u2HPixel;     /* Hsync falling edge to H active start pixel count */
	u16 u2HActive;    /* H active */
	u16 u2VsyncInv;   /* Vertical active in vsync low range, otherwise invert vsync */
	u16 u2VTopLine;   /* Vsync falling edge to V active start line count */
	u16 u2VBotLine;   /* Vsync falling edge to V active start line count, and Interlace timing use */
	u16 u2VActive;    /* V active -1 */
} WCH_TIMING_PARAM_T;

#if !NEW_WCH_EVENT_NAME
extern bool WchCreateEvent(void);
extern bool WchCloseEvent(void);
#endif
extern u32 WchIoControl(u32 u4Context, u32 u4CtlCode, u8 *pInBuffer, u32 u4InSize,
			u8 *pOutBuffer, u32 u4OutSize, u32 *pOutSize);
extern u32 _u4WCH_DBG_LVL;
extern void wchGetHwRegAddress(void);

#if WCH_DUMP_BUFFER_ATTR
extern WCH_IF_PARAM_T _gWchParam[3];
extern u32 u4DumpFrameCnt[2];
extern u32 u4DumpBufIdxCnt[2];
#ifndef __ARM2__
extern bool WchDumpBuffer(u32 u1WchId, u32 u4YAddr, u32 u4CAddr, u32 u4Src);
bool WchCaptureScreen(void);
#endif
#endif

#ifndef __ARM2__
extern struct task_struct *hWchInst[2];
int WchGetBuffIdxThread(void *data);
void WchCreateKthread(int (*threadfn)(void *data), const char *namefmt1, const char *namefmt2);
void WchEventThreadInit(void);
void WchCloseThreadEvent(void);
#endif

HANDLE X_CreateEvent(LPTSTR lpName);
bool   X_DestroyEvent(HANDLE hEvent);
bool   X_SetEvent(HANDLE hEvent);
bool   X_SetEventData(HANDLE hEvent, DWORD dwData);
void WchEnabelClk(u8 u1WchId);
void WchDisabelClk(u8 u1WchId);
void WchSetPinmux(void);
#ifdef __ARM2__
typedef int irqreturn_t;
#define IRQ_HANDLED 0x01

unsigned int WchRequstIrq(u8 u1WchId, irqreturn_t (*isr)(int u2Vector, void *dev_id));
#else
unsigned int WchRequstIrq(u8 u1WchId, irq_handler_t isr);
#endif
void WchSetSourceBaseAddr(phys_addr_t wchReservebase);
bool GetWchIndexFromIsrId(u16 u2Vector, u32 *u4Idx);

u32 WchIoControl(u32 u4Context, u32 u4CtlCode, u8 *pInBuffer, u32 u4InSize,
u8 *pOutBuffer, u32 u4OutSize, u32 *pOutSize);
u32 WchStopByInputSrc(WCH_SRC_APP_ID_E eWchSrcId);
bool WchCaptureScreen(void);

//#if defined(CONFIG_ATC_OS_android)
u32 ConfigWch(PWCH_CTL_PARAM_T pWchCtlParam);
u32 StartWch(WCH_SRC_APP_ID_E eWchSrcId);
u32 StopWch(WCH_SRC_APP_ID_E eWchSrcId);
u32 CloseWch(WCH_SRC_APP_ID_E eWchSrcId);
u32 WchGetBufferAddress(PWCH_BUF_T pWchGetBuf);
bool WchSetMirror(PWCH_CTL_PARAM_T pWchCtlParam);
//#endif

#endif
