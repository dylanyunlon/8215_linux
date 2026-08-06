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

#ifndef __ARM2__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/videodev2.h>
#include <linux/mm.h>
#include <linux/time.h>
#include <asm/uaccess.h>
#include <media/atc/vdp_mdd.h>
#include <media/atc/display_inc.h>
#include <media/atc/pmx_hal.h>
#include <media/atc/drv_av_d.h>
#include <media/atc/display.h>
#include <media/atc/display_inc.h>
#include <media/atc/cp.h>
#include "windows.h"


#else
#include "x_types.h"
#include "vdp_mdd.h"
#include "display_inc.h"
#include "pmx_hal.h"
#include "drv_av_d.h"
#include "display.h"
#include "cp.h"
#include "errno-base.h"
#endif
#include "vdp_hal.h"
#include "drv_pmx.h"
#include "scl_hal.h"
#include "tve_hal.h"
#include "imgresz_hal_if.h"
#include "drv_imgresz.h"
#include "Surface.h"
#include "fbm.h"
#include "vdp.h"
#include "log.h"
#include "irtdma_drv.h"

extern __s32 TS_DirectScale(__u32 u4HwId, bool fg8Tap, VOID *prSrcImgInfo, VOID *prDestImgInfo);

/*#include "osd_inc.h"*/

/*#include <linux/spinlock.h>*/
/*#include "videodecoder.h"*/
#include "pulldown.h"

#define TOPLevel 0

#ifdef CONFIG_ATC_OS_linux
#define ENABLE_MM_DEINTERLACE     1
#define ENABLE_PD_DEINTERLACE     1
#define ENABLE_HD_DEINTERLACE     1
#define LINUX_FUN                 1
#else
#define ENABLE_MM_DEINTERLACE     0
#define ENABLE_PD_DEINTERLACE     0
#define ENABLE_HD_DEINTERLACE     0
#define LINUX_FUN                 0
#endif

#define IMG_RESZ_DST_BUFF_WIDTH   (_u4LCDWidth)      /* Max panel width*/
#ifdef CONFIG_ATC_OS_linux
#define IMG_RESZ_DST_BUFF_HEIGHT  (_u4LCDHeight << 1)  /* Max panel height * 2 for HD deinterlace source*/
#else
#define IMG_RESZ_DST_BUFF_HEIGHT  (_u4LCDHeight)  /* Max panel height * 2 for HD deinterlace source*/
#endif
#define IMG_RESZ_SD_BUFF_WIDTH    (720)      /* Max panel width*/
#define IMG_RESZ_SD_BUFF_HEIGHT   (480)      /* Max panel height * 2 for HD deinterlace source*/

#define VDP_BACKUP 2   /* Back up front video*/

#define VDP_ASSERT(argu) ASSERT(argu)

#define SIZE_ALIGN(x, n)  ((x + n - 1) / n * n)

#define ROUND_UP_COUNT(Count, Pow2)          (((Count)+(Pow2)-1) & (~(((LONG)(Pow2))-1)))

#define VALID_BUFF_COUNT(Cur, Disp, Count)   (((Cur - Disp + MAX_BUFF_CNT) % MAX_BUFF_CNT) > Count)

#define INVALID_BUFF_COUNT(Cur, Disp)        (((Disp + 1) % MAX_BUFF_CNT) == Cur)

#define CALCULATE_VSYNC_CNT(Duration, Vsync) ((Duration + Vsync / 2) / Vsync)

#define MONITOR_QBUF_TIME 0

#define SPIN_LOCK_SRC_ISRF   0
#define SPIN_LOCK_SRC_ISRR   1
#define SPIN_LOCK_SRC_IOCTL  2
#define SPIN_LOCK_SRC_RESIZE 3

#ifndef __ARM2__
static spinlock_t _vdp_lock;
#endif
static unsigned long _vdp_flags;
static __u32 u4VdpLockCnt;
static bool fgLockInit = FALSE;
static bool _fgInitVdpParam[2] = {FALSE, FALSE};
static bool _fgBypassFstVysnc[2] = {TRUE, TRUE};
#if TOPLevel
static bool _fgVDOTopLevel = FALSE; /* backcar video source show in front panel*/
#endif
static __u32 _u4VdpOutWidth;
static __u32 _u4VdpOutHeight;
VDP_PARAM rData[3];

static PMTKSurface pImgDstSurf[3][IMG_RESZ_BUFF_SIZE] = {
	{NULL, NULL, NULL},
	{NULL, NULL, NULL},
	{NULL, NULL, NULL},
};

static PMTKSurface pRotateSurf[2] = {NULL,NULL};

#if MONITOR_VDO_FPS
__u32 vdo_fps_en[2] = {0, 0};
__u32 vdo_flip_cnt[2] = {0, 0};
#endif

#if MONITOR_QBUF_TIME
struct timeval start_t[2];
static __u32 u4TmpCnt;
static bool fgStartCnt = FALSE;
#endif

u32 fr_follow = 0;
bool vdo_fr_on = FALSE;
#ifndef __ARM2__
extern int g_vdp_dump;
#endif
void SpinLock(__u32 u4LockSrc)
{
#ifndef __ARM2__
	spin_lock_irqsave(&_vdp_lock, _vdp_flags);
	u4VdpLockCnt++;

	if ((u4LockSrc == SPIN_LOCK_SRC_IOCTL) || (u4LockSrc == SPIN_LOCK_SRC_RESIZE)) {
		FB_PRINT(FB_LOG_LVL_IRQ, "VDP", "VDP Spinlock SRC =%d, count=%d, flag = %d\r\n"
			, (int)u4LockSrc, (int)u4VdpLockCnt, (int)_vdp_flags);
	} else if (u4VdpLockCnt > 1) {
		FB_PRINT(FB_LOG_LVL_IRQ, "VDP", "VDP Spinlock 111 SRC =%d, count=%d, flag = %d\r\n"
			, (int)u4LockSrc, (int)u4VdpLockCnt, (int)_vdp_flags);
	}
#endif
}

void SpinUnlock(__u32 u4LockSrc)
{
#ifndef __ARM2__
	u4VdpLockCnt--;
	spin_unlock_irqrestore(&_vdp_lock, _vdp_flags);

	if ((u4LockSrc == SPIN_LOCK_SRC_IOCTL) || (u4LockSrc == SPIN_LOCK_SRC_RESIZE)) {
		FB_PRINT(FB_LOG_LVL_IRQ, "VDP", "VDP SpinUnlock SRC =%d, count=%d, flag = %d\r\n"
			, (int)u4LockSrc, (int)u4VdpLockCnt, (int)_vdp_flags);
	} else if (u4VdpLockCnt) {
		FB_PRINT(FB_LOG_LVL_IRQ, "VDP", "VDP SpinUnlock 111 SRC =%d, count=%d, flag = %d\r\n"
			, (int)u4LockSrc, (int)u4VdpLockCnt, (int)_vdp_flags);
	}
#endif
}

bool fgResetParam(VDP_PARAM *prParam)
{
	memset(prParam, 0, sizeof(VDP_PARAM));
	prParam->u4ClearIdx = 0xFF;
	prParam->fgFirstField = TRUE;
	prParam->fgTopFiledFirst = TRUE;
	prParam->fgRepeatFirstField = TRUE;
#if MONITOR_QBUF_TIME
	u4TmpCnt = 0;
	fgStartCnt = FALSE;
#endif
	return TRUE;
}

#ifdef CONFIG_ATC_OS_linux
bool fgClearOneFrmBuff(VDP_PARAM *prParam, __u32 u4StartIdx, __u32 u4EndIdx)
{
	__u32 u4Idx = 0;

	if (u4StartIdx > u4EndIdx) {
		return FALSE;
	}

	for (u4Idx = u4StartIdx; u4Idx <= u4EndIdx; u4Idx++) {
		if (prParam->u4FrmBuffY[u4Idx] & prParam->u4FrmBuffC[u4Idx]) {
			/* clear buffer*/
			/*FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s Y/C 0x%x 0x%x\r\n", __func__
				, prParam->u4FrmBuffY[u4Idx], prParam->u4FrmBuffC[u4Idx]);*/
			prParam->u4FrmBuffY[u4Idx] = 0;
			prParam->u4FrmBuffC[u4Idx] = 0;
			prParam->u4Duration[u4Idx] = 0;
		}
		else {
			/*Clear Buffer is NULL*/
			return FALSE;
		}
	}

	return TRUE;
}
#endif
bool fgClearFrmBuff(VDP_PARAM *prParam, __u32 u4StartIdx, __u32 u4EndIdx)
{
	__u32 u4Idx = 0;

	if (u4StartIdx > u4EndIdx) {
		return FALSE;
	}

	for (u4Idx = u4StartIdx; u4Idx <= u4EndIdx; u4Idx++) {
		if (prParam->u4FrmBuffY[u4Idx] & prParam->u4FrmBuffC[u4Idx]) {
			/* clear buffer*/
			/*FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s Y/C 0x%x 0x%x\r\n", __func__
				, prParam->u4FrmBuffY[u4Idx], prParam->u4FrmBuffC[u4Idx]);*/
			prParam->u4FrmBuffY[u4Idx] = 0;
			prParam->u4FrmBuffC[u4Idx] = 0;
			prParam->u4Duration[u4Idx] = 0;
			prParam->u4VsyncCnt[u4Idx] = 0;
		}
	}

	return TRUE;
}

#ifndef __ARM2__
#define MAX_ISR_CNT    10
static struct atc_dispc_isr_data isr_data[MAX_ISR_CNT];
static bool register_isr[MAX_ISR_CNT];
int atc_dispc_register_isr(atc_dispc_isr_t isr, void *arg)
{
	u32 i = 0;

	if ((isr == NULL) || (arg == NULL)) {
		FB_PRINT(FB_LOG_LVL_ERR, "VDP", "atc_dispc_register_isr return EINVAL\r\n");

		return -EINVAL;
	}

	for (i = 0; i < MAX_ISR_CNT; i++) {
		if (isr_data[i].isr == NULL) {
			isr_data[i].isr = isr;
			isr_data[i].arg = arg;
			register_isr[i] = TRUE;
			break;
		} else if ((isr_data[i].isr == isr) && (isr_data[i].arg == arg)) {
			register_isr[i] = TRUE;
			break;
		}
	}

	if (i == MAX_ISR_CNT) {
		FB_PRINT(FB_LOG_LVL_ERR, "VDP", "atc_dispc_register_isr no free isr %d\r\n", i);

		return -EBUSY;
	}
	FB_PRINT(FB_LOG_LVL_INFO, "VDP", "atc_dispc_register_isr %d\r\n", i);

	return 0;
}
EXPORT_SYMBOL(atc_dispc_register_isr);

int atc_dispc_unregister_isr(atc_dispc_isr_t isr, void *arg)
{
	u32 i = 0;

	if ((isr == NULL) || (arg == NULL)) {
		FB_PRINT(FB_LOG_LVL_ERR, "VDP", "atc_dispc_unregister_isr return EINVAL\r\n");

		return -EINVAL;
	}

	for (i = 0; i < MAX_ISR_CNT; i++) {
		if ((isr_data[i].isr == isr) && (isr_data[i].arg == arg)) {
			isr_data[i].isr = NULL;
			isr_data[i].arg = NULL;
			register_isr[i] = FALSE;
			FB_PRINT(FB_LOG_LVL_INFO, "VDP", "atc_dispc_unregister_isr %d\r\n", i);

			return 0;
		}
	}
	FB_PRINT(FB_LOG_LVL_ERR, "VDP", "atc_dispc_unregister_isr no match isr %d\r\n", i);

	return -EBUSY;
}
EXPORT_SYMBOL(atc_dispc_unregister_isr);
#endif

void vDumpFrameBuffer(VDP_PARAM *prParam, bool fgFstField, __u32 *u4FBufferY)
{
	FB_PRINT(FB_LOG_LVL_TRACE, "VDP", "vDumpFrameBuffer: first field %d, id %d %d %d, buffer %x, %x, %x, %x\r\n"
		, (int)fgFstField, (int)prParam->u4PrevIdx, (int)prParam->u4DispIdx, (int)prParam->u4CurrIdx
		, (unsigned int)u4FBufferY[0], (unsigned int)u4FBufferY[1]
		, (unsigned int)u4FBufferY[2], (unsigned int)u4FBufferY[3]);
}

bool fgUpdateHwAddr(__u32 u4VdpIdx)
{
	VDP_PARAM *prParam = &rData[u4VdpIdx];
	__u32 u4PhyAddrY[4];
	__u32 u4PhyAddrC[4];
	__u32 u4Prev = prParam->u4PrevIdx;
	__u32 u4Disp = prParam->u4DispIdx;
	__u32 u4Next = (u4Disp + 1) % MAX_BUFF_CNT;
#ifdef CONFIG_ATC_OS_linux
	__u32 frmIdx = 0;
	__u32 frmValue = -1;
#endif
#ifdef MONITOR_VDO_FPS

	if (vdo_fps_en[u4VdpIdx]) {
		vdo_flip_cnt[u4VdpIdx]++;
	}

#endif

	if (prParam->u4DeintMode < VDP_DI_MA4F_MODE) {
		if (prParam->u4FrmBuffY[u4Disp] && prParam->u4FrmBuffC[u4Disp]) {
			vVdpHalSetYBufPtr(u4VdpIdx, prParam->u4FrmBuffY[u4Disp], prParam->u4FrmBuffC[u4Disp]);
			/*FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s: hw id %d, disp %d, cur %d, address %x, %x duration %d\r\n"
				, __func__, u4VdpIdx , u4Disp, prParam->u4CurrIdx, prParam->u4FrmBuffY[u4Disp]
				, prParam->u4FrmBuffC[u4Disp], prParam->u4Duration[u4Disp]);*/

#ifndef __ARM2__
			if (1 == g_vdp_dump) {
				vdpOutBuf.flag = 1;
				vdpOutBuf.u4YAddr[0] = prParam->u4FrmBuffY[u4Disp];
				vdpOutBuf.u4CAddr[0] = prParam->u4FrmBuffC[u4Disp];
			}
#endif
		} else {
			FB_PRINT(FB_LOG_LVL_ERR, "VDP", "fgUpdateHwAddr: error id %d, disp %d, address %x, %x\r\n"
				, (int)u4VdpIdx, (int)u4Disp, (unsigned int)prParam->u4FrmBuffY[u4Disp]
				, (unsigned int)prParam->u4FrmBuffC[u4Disp]);
			return FALSE;
		}
	} else {
#if 0
#ifdef CONFIG_ATC_OS_linux
		if (prParam->u4Flags & VDP_UPDATE_OVERLAY) {
				u4PhyAddrY[0] = prParam->u4FrmBuffY[u4Disp];
				u4PhyAddrY[1] = prParam->u4FrmBuffY[u4Disp];
				u4PhyAddrY[2] = prParam->u4FrmBuffY[u4Disp];
				u4PhyAddrY[3] = prParam->u4FrmBuffY[u4Disp];

				u4PhyAddrC[0] = prParam->u4FrmBuffC[u4Disp];
				u4PhyAddrC[1] = prParam->u4FrmBuffC[u4Disp];
				u4PhyAddrC[2] = prParam->u4FrmBuffC[u4Disp];
				u4PhyAddrC[3] = prParam->u4FrmBuffC[u4Disp];
		}
		else
#endif
#endif
		{
			if (prParam->fgFirstField) {
#ifdef CONFIG_ATC_OS_linux
#ifndef __ARM2__
				if(!prParam->u4FrmBuffY[u4Prev]&&prParam->u4FrmBuffY[u4Disp]) {
					u4Prev = u4Disp;
				} else if(prParam->u4FrmBuffY[u4Prev]&&!prParam->u4FrmBuffY[u4Disp]){
					u4Disp = u4Prev;
				}
#endif
#endif
				u4PhyAddrY[0] = prParam->u4FrmBuffY[u4Prev];
				u4PhyAddrY[1] = prParam->u4FrmBuffY[u4Prev];
				u4PhyAddrY[2] = prParam->u4FrmBuffY[u4Disp];
				u4PhyAddrY[3] = prParam->u4FrmBuffY[u4Disp];

				u4PhyAddrC[0] = prParam->u4FrmBuffC[u4Prev];
				u4PhyAddrC[1] = prParam->u4FrmBuffC[u4Prev];
				u4PhyAddrC[2] = prParam->u4FrmBuffC[u4Disp];
				u4PhyAddrC[3] = prParam->u4FrmBuffC[u4Disp];
			} else {
				u4PhyAddrY[0] = prParam->u4FrmBuffY[u4Prev];
				u4PhyAddrY[1] = prParam->u4FrmBuffY[u4Disp];
				u4PhyAddrY[2] = prParam->u4FrmBuffY[u4Disp];
				u4PhyAddrY[3] = prParam->u4FrmBuffY[u4Next];

				u4PhyAddrC[0] = prParam->u4FrmBuffC[u4Prev];
				u4PhyAddrC[1] = prParam->u4FrmBuffC[u4Disp];
				u4PhyAddrC[2] = prParam->u4FrmBuffC[u4Disp];
				u4PhyAddrC[3] = prParam->u4FrmBuffC[u4Next];
#ifdef CONFIG_ATC_OS_linux
#ifndef __ARM2__
				if(u4PhyAddrY[0]&&u4PhyAddrY[1] && u4PhyAddrY[3]) {
					/*Input Buffer is All valid*/
				} else {
					for(frmIdx = 3 ; frmIdx > -1;frmIdx--) {
						if(u4PhyAddrY[frmIdx]) {
							frmValue = frmIdx;
							break;
						}
					}
					for(frmIdx = 0 ; frmIdx <4 ; frmIdx++) {
						if(u4PhyAddrY[frmIdx] == NULL) {
							u4PhyAddrY[frmIdx] = u4PhyAddrY[frmValue];
							u4PhyAddrC[frmIdx] = u4PhyAddrC[frmValue];
						}
					}

				}
#endif
#endif
			}
		}
		if (u4PhyAddrY[0] && u4PhyAddrY[1] && u4PhyAddrY[2] && u4PhyAddrY[3] &&
		    u4PhyAddrC[0] && u4PhyAddrC[1] && u4PhyAddrC[2] && u4PhyAddrC[3]) {
			/*vDumpFrameBuffer(prParam, prParam->fgFirstField, u4PhyAddrY);*/
			vVdpHalSetDeintWXYZ(u4VdpIdx, u4PhyAddrY, u4PhyAddrC);
#ifndef __ARM2__
			if (1 == g_vdp_dump) {
				vdpOutBuf.flag = 0;
				vdpOutBuf.u4YAddr[0] = u4PhyAddrY[0];
				vdpOutBuf.u4YAddr[1] = u4PhyAddrY[1];
				vdpOutBuf.u4YAddr[2] = u4PhyAddrY[2];
				vdpOutBuf.u4YAddr[3] = u4PhyAddrY[3];
				vdpOutBuf.u4CAddr[0] = u4PhyAddrC[0];
				vdpOutBuf.u4CAddr[1] = u4PhyAddrC[1];
				vdpOutBuf.u4CAddr[2] = u4PhyAddrC[2];
				vdpOutBuf.u4CAddr[3] = u4PhyAddrC[3];
			}
#endif
		} else {
			vDumpFrameBuffer(prParam, prParam->fgFirstField, u4PhyAddrY);
			return FALSE;
		}
	}

	return TRUE;
}

void vUpdateDispIdx(__u32 u4VdpIdx)
{
	VDP_PARAM *prParam = &rData[u4VdpIdx];

	if (prParam->u4PrevIdx == prParam->u4DispIdx) {
		prParam->u4DispIdx = (prParam->u4DispIdx + 1) % MAX_BUFF_CNT;
		prParam->u4ClearIdx = 0xFF;
	} else {
		prParam->u4ClearIdx = prParam->u4PrevIdx;
		prParam->u4PrevIdx = (prParam->u4PrevIdx + 1) % MAX_BUFF_CNT;
		prParam->u4DispIdx = (prParam->u4DispIdx + 1) % MAX_BUFF_CNT;
	}
}

void vVdpIsr(__u32 u4VdpIdx)
{
	VDP_PARAM *prParam = NULL;
	__u32 u4DispIdx = 0;
	bool buffer_done = FALSE;

	if (u4VdpIdx > VDP_2) {
		FB_PRINT(FB_LOG_LVL_ERR, "VDP", "vdpisr not support hw id %d\r\n", u4VdpIdx);
		return;
	}

#ifndef __ARM2__
	if (!fgLockInit) {
		spin_lock_init(&_vdp_lock);
		fgLockInit = TRUE;
		FB_PRINT(FB_LOG_LVL_DBG, "VDP", "vdpisr spin lock init %x\r\n", (unsigned int)&_vdp_lock);
	}
#endif
	SpinLock(u4VdpIdx);
	prParam = &rData[u4VdpIdx];

	if ((prParam->u4NeedClrBuff & VDP_CLR_ONCE_BUF) && (prParam->u4ClearIdx != 0xFF)) {
		if (prParam->u4ClearIdx >= prParam->u4CurrIdx) {
			fgClearFrmBuff(prParam, prParam->u4CurrIdx, prParam->u4ClearIdx);
		} else {
			fgClearFrmBuff(prParam, prParam->u4CurrIdx, MAX_BUFF_CNT - 1);
			fgClearFrmBuff(prParam, 0, prParam->u4ClearIdx);
		}

		prParam->u4ClearIdx = 0xFF;
		prParam->u4NeedClrBuff &= ~VDP_CLR_ONCE_BUF;
		buffer_done = TRUE;
	}


#ifdef CONFIG_ATC_OS_linux
	/*Linux backcar pal-nstc-pal*/
	if (prParam->u4NeedClrBuff & VDP_CLR_REGU_BUF) {
		fgClearFrmBuff(prParam, prParam->u4ClearIdx, prParam->u4ClearIdx);
		prParam->u4ClearIdx = 0xFF;
		buffer_done = TRUE;
	}
#else
	if ((prParam->u4NeedClrBuff & VDP_CLR_REGU_BUF) && (prParam->u4ClearIdx != 0xFF)) {
		fgClearFrmBuff(prParam, prParam->u4ClearIdx, prParam->u4ClearIdx);
		prParam->u4ClearIdx = 0xFF;
		buffer_done = TRUE;
	}
#endif

	if ((prParam->u4DeintMode < VDP_DI_MA4F_MODE) || (prParam->u4VdpStatus == VDP_STATUS_BLACK)
		|| (prParam->u4VdpStatus == VDP_STATUS_SEEK)) {
		buffer_done = TRUE;
	}


	if (prParam->u4DeintMode >= VDP_DI_MA4F_MODE) {
		switch (prParam->u4VdpStatus) {
		case VDP_STATUS_PREPARE:
		case VDP_STATUS_SEEK: {
			if(prParam->u4VdpStatus == VDP_STATUS_PREPARE) {
					buffer_done = FALSE;
			}
			if (VALID_BUFF_COUNT(prParam->u4CurrIdx, prParam->u4DispIdx, 1)) {
				FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s hw %d ready to show %d, %d\r\n", __func__, u4VdpIdx
					, (int)prParam->u4DispIdx, (int)prParam->u4CurrIdx);
				prParam->u4VdpStatus = VDP_STATUS_SHOW;
				_fgBypassFstVysnc[u4VdpIdx] = TRUE;
			} else {
				FB_PRINT(FB_LOG_LVL_DBG, "VDP", "%s hw %d VDP_STATUS_PREPARE %d, %d\r\n", __func__, u4VdpIdx
					, (int)prParam->u4DispIdx, (int)prParam->u4CurrIdx);
				break;
			}
		}

		case VDP_STATUS_SHOW: {
			buffer_done = FALSE;
			if (INVALID_BUFF_COUNT(prParam->u4CurrIdx, prParam->u4DispIdx)) {
				/* buffer not enough to show*/
				/*FB_PRINT(FB_LOG_LVL_INFO, "VDP", "vVdpIsr no buffer %d, %d topfieldfirst %d\r\n"
					, (int)prParam->u4DispIdx, (int)prParam->u4CurrIdx
					, (int)prParam->fgTopFiledFirst);*/
			} else {
				if (_fgBypassFstVysnc[u4VdpIdx]) {
					/* First vsync not update address and motion and comb is invalid*/
					_fgBypassFstVysnc[u4VdpIdx] = FALSE;
				}

#if ENABLE_PD_DEINTERLACE
#ifdef CONFIG_ATC_OS_android
				else if (prParam->u4SrcType == USB) {
					vPullDownGetMotionComb(u4VdpIdx);
				}
#else
				else if (prParam->u4Flags & VDP_ENABLE_PDDI) {
					vPullDownGetMotionComb(u4VdpIdx);
				}

#endif
#endif
				u4DispIdx = prParam->u4DispIdx;

				if ((prParam->u4VsyncCnt[u4DispIdx] ==
					CALCULATE_VSYNC_CNT(prParam->u4Duration[u4DispIdx], VSYNC_PER_FIELD)) ||
					(prParam->u4VsyncCnt[u4DispIdx] ==
					CALCULATE_VSYNC_CNT(prParam->u4Duration[u4DispIdx], VSYNC_PER_FRAME))) {
					fgUpdateHwAddr(u4VdpIdx);
					vVdpHalSetFieldInfo(u4VdpIdx, prParam->fgTopFiledFirst, prParam->fgFirstField);

					/* fix bug AC8317M-10338 */
					if ((_u4LCDWidth != 1920) && (_u4LCDHeight != 720)) {
						prParam->fgFirstField = !prParam->fgFirstField;
					}

					if (!vPmxHalGetFmtEn(u4VdpIdx)) {
						/* Enable FMT to show video*/
						vPmxHalEnableFmt(u4VdpIdx);
						FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s hw %d enable fmt\r\n", __func__, u4VdpIdx);
					}

					if ((u4VdpIdx == VDP_1) && !fgPmxHalMixPlane(PMX_1, PMX_HW_PLANE_1)) {
						vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_1);
						#ifdef __ARM2__
						FB_PRINT(FB_LOG_LVL_INFO, "VDP", "enable video layer in isr and consume time %ums\r\n", GetBootTime());
						#else
						FB_PRINT(FB_LOG_LVL_INFO, "VDP", "enable video layer in isr\r\n");
						#endif
					}

#if ENABLE_PD_DEINTERLACE
#if !LINUX_FUN
					if ((prParam->u4SrcType == USB) &&
						(prParam->u4PullDownMode != PULLDOWN_MODE_UNKNOWN))
#else
					if ((prParam->u4Flags & VDP_ENABLE_PDDI) &&
						(prParam->u4PullDownMode != PULLDOWN_MODE_UNKNOWN))
#endif
					{
						vPullDownSetMergeInfo(u4VdpIdx, prParam->u4PullDownMode);
					}
#endif
				}
#if 0
#ifdef CONFIG_ATC_OS_linux
				if (prParam->u4Flags & VDP_UPDATE_OVERLAY) {
					prParam->u4CurrIdx--;
					if (prParam->u4CurrIdx == MAX_BUFF_CNT) {
						prParam->u4CurrIdx = 0;
					}
					prParam->u4VdpStatus =VDP_STATUS_PREPARE;
				}
#endif
#endif
				/*FB_PRINT(FB_LOG_LVL_IRQ, "VDP", "vVdpIsr count id %d, _gVsyncNs %d, disp %d %d %d\r\n"
					, (int)u4VdpIdx, (int)prParam->u4VsyncCnt[u4DispIdx], (int)prParam->u4PrevIdx
					, (int)prParam->u4DispIdx, (int)prParam->u4CurrIdx);*/
				prParam->u4VsyncCnt[u4DispIdx]--;

				if (prParam->u4VsyncCnt[u4DispIdx] <= 0) {

					vUpdateDispIdx(u4VdpIdx);
					buffer_done  = true;
				}
			}

			break;
		}

		case VDP_STATUS_FFRW: { /* fast forward or rew*/
			FB_PRINT(FB_LOG_LVL_IRQ, "VDP", "vVdpIsr VDP_STATUS_FFRW id %d, disp %d, prev %d, di %d\r\n"
				, (int)u4VdpIdx, (int)prParam->u4DispIdx, (int)prParam->u4PrevIdx
				, (int)prParam->u4DeintMode);

			if (fgUpdateHwAddr(u4VdpIdx)) {
				vUpdateDispIdx(u4VdpIdx);
			}

			break;
		}

		default: {
			break;
		}
		}
	} /*else if (prParam->u4DeintMode == VDP_DI_HD_MODE) {
		ToDo: Swap top / bottom filed in isr
	}*/

#if 0
	else if ((prParam->u4VdpStatus != VDP_STATUS_HIDE) && (prParam->u4NeedShowBuff)) {
		if (fgUpdateHwAddr(u4VdpIdx)) {
			vUpdateDispIdx(u4VdpIdx);

			if (prParam->u4VdpStatus == VDP_STATUS_PREPARE) {
				if (!vPmxHalGetFmtEn(u4VdpIdx)) {
					/* Enable FMT to show video*/
					vPmxHalEnableFmt(u4VdpIdx);
				}

				if ((u4VdpIdx == VDP_1) && !fgPmxHalMixPlane(PMX_1, PMX_HW_PLANE_1)) {
					vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_1);
				}

				prParam->u4VdpStatus = VDP_STATUS_SHOW;
			}

			prParam->u4NeedShowBuff--;
		}
	}

#endif

#ifndef __ARM2__
#if LINUX_FUN
	if (buffer_done) {
		for (u4DispIdx = 0; u4DispIdx < MAX_ISR_CNT; u4DispIdx++) {
			if (register_isr[u4DispIdx]) {
				if ((NULL != isr_data[u4DispIdx].isr) && (NULL != isr_data[u4DispIdx].arg)) {
					isr_data[u4DispIdx].isr(isr_data[u4DispIdx].arg, u4VdpIdx);
					/*FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s dev %d hw %d display %d status %d\r\n", __func__
						, u4DispIdx, u4VdpIdx, prParam->u4DispIdx, prParam->u4VdpStatus);*/
				} else {
					FB_PRINT(FB_LOG_LVL_ERR, "VDP", "vdpisr isr_data error dev %d\r\n", u4DispIdx);
				}
			}
		}
	}
#endif
#endif
	SpinUnlock(u4VdpIdx);
	vVdpHalIsr(u4VdpIdx);
}

void vDumpVdpParam(int index)
{
	VDP_PARAM *prParam = &rData[index];

	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "##############VDP Param Data Start Vdp %d##############\r\n", index);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4SrcWidth: %d\r\n", (int)prParam->u4SrcWidth);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4SrcHeight: %d\r\n", (int)prParam->u4SrcHeight);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->rSrcRect: %d %d %d %d\r\n", (int)prParam->rSrcRect.left,
		(int)prParam->rSrcRect.top, (int)prParam->rSrcRect.right, (int)prParam->rSrcRect.bottom);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->rDstRect: %d %d %d %d\r\n", (int)prParam->rDstRect.left,
		(int)prParam->rDstRect.top, (int)prParam->rDstRect.right, (int)prParam->rDstRect.bottom);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4SrcType: %d\r\n", (int)prParam->u4SrcType);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4Flags: 0x%x\r\n", (unsigned int)prParam->u4Flags);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4FrmBuffY: 0x%x\r\n"
		, (unsigned int)prParam->u4FrmBuffY[prParam->u4DispIdx]);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4FrmBuffC: 0x%x\r\n"
		, (unsigned int)prParam->u4FrmBuffC[prParam->u4DispIdx]);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4Duration: %d\r\n", (int)prParam->u4Duration[prParam->u4DispIdx]);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4VsyncCnt: %d\r\n", (int)prParam->u4VsyncCnt[prParam->u4DispIdx]);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4VdpStatus: %d\r\n", (int)prParam->u4VdpStatus);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4VdpMode: 0x%x\r\n", (unsigned int)prParam->u4VdpMode);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4DeintMode: %d\r\n", (int)prParam->u4DeintMode);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4PullDownMode: %d\r\n", (int)prParam->u4PullDownMode);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4CurrIdx: %d\r\n", (int)prParam->u4CurrIdx);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4DispIdx: %d\r\n", (int)prParam->u4DispIdx);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4PrevIdx: %d\r\n", (int)prParam->u4PrevIdx);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4ClearIdx: %d\r\n", (int)prParam->u4ClearIdx);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->fgNeedClrBuff: %d\r\n", (int)prParam->u4NeedClrBuff);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->fgFirstField: %d\r\n", (int)prParam->fgFirstField);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->fgNeedResizer: %d\r\n", (int)prParam->fgNeedResizer);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->fgProgSrc: %d\r\n", (int)prParam->fgProgSrc);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->fgTopFiledFirst: %d\r\n", (int)prParam->fgTopFiledFirst);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->fgRepeatFirstField: %d\r\n", (int)prParam->fgRepeatFirstField);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->fgProgSeq: %d\r\n", (int)prParam->fgProgSeq);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->fgPullDownFlagValid: %d\r\n", (int)prParam->fgPullDownFlagValid);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "##############VDP Param Data End##############\r\n");
}

void vDumpParam(VDP_PARAM *prParam)
{
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "##############VDP Param Data Start Vdp 0x%x##############\r\n"
		, (unsigned int)prParam);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4SrcWidth: %d\r\n", (int)prParam->u4SrcWidth);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4SrcHeight: %d\r\n", (int)prParam->u4SrcHeight);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->rSrcRect: %d %d %d %d\r\n", (int)prParam->rSrcRect.left,
		(int)prParam->rSrcRect.top, (int)prParam->rSrcRect.right, (int)prParam->rSrcRect.bottom);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->rDstRect: %d %d %d %d\r\n", (int)prParam->rDstRect.left,
		(int)prParam->rDstRect.top, (int)prParam->rDstRect.right, (int)prParam->rDstRect.bottom);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4SrcType: %d\r\n", (int)prParam->u4SrcType);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4Flags: 0x%x\r\n", (unsigned int)prParam->u4Flags);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4FrmBuffY: 0x%x\r\n"
		, (unsigned int)prParam->u4FrmBuffY[prParam->u4DispIdx]);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4FrmBuffC: 0x%x\r\n"
		, (unsigned int)prParam->u4FrmBuffC[prParam->u4DispIdx]);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4Duration: %d\r\n", (int)prParam->u4Duration[prParam->u4DispIdx]);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4VsyncCnt: %d\r\n", (int)prParam->u4VsyncCnt[prParam->u4DispIdx]);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4VdpStatus: %d\r\n", (int)prParam->u4VdpStatus);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4VdpMode: 0x%x\r\n", (unsigned int)prParam->u4VdpMode);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4DeintMode: %d\r\n", (int)prParam->u4DeintMode);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4PullDownMode: %d\r\n", (int)prParam->u4PullDownMode);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4CurrIdx: %d\r\n", (int)prParam->u4CurrIdx);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4DispIdx: %d\r\n", (int)prParam->u4DispIdx);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4PrevIdx: %d\r\n", (int)prParam->u4PrevIdx);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->u4ClearIdx: %d\r\n", (int)prParam->u4ClearIdx);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->fgNeedClrBuff: %d\r\n", (int)prParam->u4NeedClrBuff);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->fgFirstField: %d\r\n", (int)prParam->fgFirstField);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->fgNeedResizer: %d\r\n", (int)prParam->fgNeedResizer);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->fgProgSrc: %d\r\n", (int)prParam->fgProgSrc);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->fgTopFiledFirst: %d\r\n", (int)prParam->fgTopFiledFirst);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->fgRepeatFirstField: %d\r\n", (int)prParam->fgRepeatFirstField);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->fgProgSeq: %d\r\n", (int)prParam->fgProgSeq);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "prParam->fgPullDownFlagValid: %d\r\n", (int)prParam->fgPullDownFlagValid);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "##############VDP Param Data End##############\r\n");
}

bool fgAddFrmBuff(__u32 u4VdpIdx, __u32 u4YAddr, __u32 u4CAddr, __u32 u4Duration)
{
	VDP_PARAM *prParam = &rData[u4VdpIdx];
	__u32 u4Idx = prParam->u4CurrIdx;
#if MONITOR_QBUF_TIME
	struct timeval end_t = {0, 0};
	long usec = 0;
#endif

	if ((u4YAddr & 0x7F) || (u4CAddr & 0x7F)) {
		FB_PRINT(FB_LOG_LVL_ERR, "VDP", "fgAddFrmBuff: error buffer is not 128 bytes alignment %x, %x, %d\r\n",
			(unsigned int)u4YAddr, (unsigned int)u4CAddr, (int)u4Duration);
		return FALSE;
	}

	if ((u4YAddr == prParam->u4FrmBuffY[(u4Idx - 1 + MAX_BUFF_CNT) % MAX_BUFF_CNT]) ||
	    (u4CAddr == prParam->u4FrmBuffY[(u4Idx - 1 + MAX_BUFF_CNT) % MAX_BUFF_CNT])) {
		FB_PRINT(FB_LOG_LVL_ERR, "VDP", "fgAddFrmBuff: error buffer addr is the same as before %x, %x, %d\r\n",
			(unsigned int)u4YAddr, (unsigned int)u4CAddr, (int)u4Duration);
		return FALSE;
	}

	prParam->u4FrmBuffY[u4Idx] = u4YAddr;
	prParam->u4FrmBuffC[u4Idx] = u4CAddr;
	prParam->u4Duration[u4Idx] = u4Duration ? u4Duration : VSYNC_PER_FRAME;
	/* fix bug AC8317M-10338 */
	if ((_u4LCDWidth != 1920) && (_u4LCDHeight != 720))
		prParam->u4VsyncCnt[u4Idx] = CALCULATE_VSYNC_CNT(prParam->u4Duration[u4Idx], VSYNC_PER_FIELD);
	else
		prParam->u4VsyncCnt[u4Idx] = CALCULATE_VSYNC_CNT(prParam->u4Duration[u4Idx], VSYNC_PER_FRAME);


#if MONITOR_QBUF_TIME
#if 1

	if ((u4TmpCnt == 0) && (fgStartCnt == FALSE)) {
		do_gettimeofday(&start_t[u4VdpIdx]);
		fgStartCnt = TRUE;
	}

	if (u4TmpCnt == 30) {
		do_gettimeofday(&end_t);
		usec = (end_t.tv_sec - start_t[u4VdpIdx].tv_sec) * 1000000
			+ (end_t.tv_usec - start_t[u4VdpIdx].tv_usec);
		FB_PRINT(FB_LOG_LVL_INFO, "VDP", "fgAddFrmBuff: 30 FPS id = %d usec = %d\r\n", (int)u4VdpIdx, (int)usec);
		do_gettimeofday(&start_t[u4VdpIdx]);
		u4TmpCnt = 0;
	} else {
		u4TmpCnt++;
	}

#else

	if (fgStartCnt == FALSE) {
		do_gettimeofday(&start_t[u4VdpIdx]);
		fgStartCnt = TRUE;
	}

	do_gettimeofday(&end_t);
	usec = (end_t.tv_sec - start_t[u4VdpIdx].tv_sec) * 1000000 + (end_t.tv_usec - start_t[u4VdpIdx].tv_usec);
	FB_PRINT(FB_LOG_LVL_INFO, "VDP", "fgAddFrmBuff: id = %d usec = %d~~~~~~~~\r\n", (int)u4VdpIdx, (int)usec);
	do_gettimeofday(&start_t[u4VdpIdx]);
#endif
#endif

	if ((prParam->u4DeintMode >= VDP_DI_MA4F_MODE) && (prParam->u4VsyncCnt[u4Idx] <= 0)) {
		FB_PRINT(FB_LOG_LVL_ERR, "VDP", "fgAddFrmBuff: Duration error idx %d, %x, %x, %d, %d, DI %d\r\n",
			(int)u4Idx, (unsigned int)u4YAddr, (unsigned int)u4CAddr, (int)u4Duration,
			(int)prParam->u4VsyncCnt[u4Idx], (int)prParam->u4DeintMode);
		return FALSE;
	}

	prParam->u4CurrIdx++;

	if (prParam->u4CurrIdx == MAX_BUFF_CNT) {
		prParam->u4CurrIdx = 0;
	}

	return TRUE;
}

__u32 RemapVdpRect(RECT *prDst, RECT *prObj, __u32 u4SrcWidth, __u32 u4SrcHeight
	, __u32 u4DstWidth, __u32 u4DstHeight)
{
	prObj->left = prDst->left * u4SrcWidth / u4DstWidth;
	prObj->right = prDst->right * u4SrcWidth / u4DstWidth;
	prObj->top = prDst->top * u4SrcHeight / u4DstHeight;
	prObj->bottom = prDst->bottom * u4SrcHeight / u4DstHeight;

	return TRUE;
}

u32 remap_rear_rect(RECT *prBefore)
{
	u32 rear_width, rear_height, front_width, front_height;

	rear_height = (g_rFBConfig.u4VideoMode == RES_480P) ? 480 : 576;
	rear_width = 720;
	front_width = _u4LCDWidth;
	front_height = _u4LCDHeight;

	prBefore->left = (prBefore->left * rear_width / front_width) + g_rFBConfig.rCVBSMargin.u4TopLeftXMargin;
	prBefore->top = (prBefore->top * rear_height / _u4LCDHeight) + g_rFBConfig.rCVBSMargin.u4TopLeftYMargin;

	prBefore->right = (prBefore->right * rear_width / front_width) - g_rFBConfig.rCVBSMargin.u4BottomRightXMargin;
	prBefore->bottom = (prBefore->bottom * rear_height / _u4LCDHeight) - g_rFBConfig.rCVBSMargin.u4BottomRightYMargin;

	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "%s remap rear rect %d %d %d %d\r\n", __func__, g_rFBConfig.rCVBSMargin.u4TopLeftXMargin
		, g_rFBConfig.rCVBSMargin.u4TopLeftYMargin, g_rFBConfig.rCVBSMargin.u4BottomRightXMargin
		, g_rFBConfig.rCVBSMargin.u4BottomRightYMargin);

	return TRUE;
}

bool is_rear_vdp_busy(void)
{
	bool ret = FALSE;

	if (!(fr_follow & FR_FOLLOW_VIDEO) && vPmxHalGetFmtEn(VDP_2)) {
		FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s vdp rear busy\r\n", __func__);
		ret = TRUE;
	} else {
		FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s vdp rear idle video mirror %d %d hw %d\r\n", __func__
			, fr_follow, vdo_fr_on, vPmxHalGetFmtEn(VDP_2));
	}

	return ret;
}

bool is_fr_follow_on(void)
{
	return fr_follow & FR_FOLLOW_MAX;
}
EXPORT_SYMBOL(is_fr_follow_on);

bool fgSetVideoInfo(__u32 u4VdpIdx, struct OVERLAY_PARAM *pOverlay, bool fgFlag)
{
	VDP_PARAM *prParam = &rData[u4VdpIdx];

#ifdef CONFIG_ATC_OS_linux
	/*If set src crop, src crop < src width and height*/
	if (((pOverlay->rSrcRect.right - pOverlay->rSrcRect.left) > pOverlay->u4SrcWidth) ||
		((pOverlay->rSrcRect.bottom - pOverlay->rSrcRect.top) > pOverlay->u4SrcHeight))
#else
	if ((pOverlay->device_name == USB) && (((pOverlay->rSrcRect.right - pOverlay->rSrcRect.left) != pOverlay->u4SrcWidth) ||
                        ((pOverlay->rSrcRect.bottom - pOverlay->rSrcRect.top) != pOverlay->u4SrcHeight)))
#endif
	{
		prParam->u4SrcWidth = SIZE_ALIGN(pOverlay->rSrcRect.right - pOverlay->rSrcRect.left, 16);
		prParam->u4SrcHeight = SIZE_ALIGN(pOverlay->rSrcRect.bottom - pOverlay->rSrcRect.top, 32);
#ifdef CONFIG_ATC_OS_linux
		FB_PRINT(FB_LOG_LVL_WARN, "VDP", "reconfig dev %d src width/height %d, %d\r\n", u4VdpIdx
			, prParam->u4SrcWidth, prParam->u4SrcHeight);
#endif
	} else {
		prParam->u4SrcWidth = pOverlay->u4SrcWidth;
		prParam->u4SrcHeight = pOverlay->u4SrcHeight;
	}

	prParam->rSrcRect.left = pOverlay->rSrcRect.left;
	prParam->rSrcRect.top = pOverlay->rSrcRect.top;
	prParam->rSrcRect.right = pOverlay->rSrcRect.right;
	prParam->rSrcRect.bottom = pOverlay->rSrcRect.bottom;

	if (u4VdpIdx == VDP_1) {
		prParam->rDstRect.left = pOverlay->rDstRect.left;
		prParam->rDstRect.top = pOverlay->rDstRect.top;
		prParam->rDstRect.right = pOverlay->rDstRect.right;
		prParam->rDstRect.bottom = pOverlay->rDstRect.bottom;
	} else if (u4VdpIdx == VDP_2) {
		if ((pOverlay->rDstRect.right > 720) || (pOverlay->rDstRect.bottom > 576) ||
		    (pOverlay->rDstRect.right == 0) || (pOverlay->rDstRect.bottom == 0)) {
			if (prParam->u4VdpMode == RES_480P) {
				prParam->rDstRect.bottom = 480;
			} else {
				prParam->rDstRect.bottom = 576;
			}

			prParam->rDstRect.left = 0;
			prParam->rDstRect.top = 0;
			prParam->rDstRect.right = 720;
			FB_PRINT(FB_LOG_LVL_WARN, "VDP", "Rear param error and reconfig to SD %d, %d, %d, %d vdp mode %d\r\n"
				, (int)pOverlay->rDstRect.top, (int)pOverlay->rDstRect.bottom
				, (int)pOverlay->rDstRect.left, (int)pOverlay->rDstRect.right
				, (int)prParam->u4VdpMode);
		} else {
			prParam->rDstRect.left = pOverlay->rDstRect.left;
			prParam->rDstRect.top = pOverlay->rDstRect.top;
			prParam->rDstRect.right = pOverlay->rDstRect.right;
			prParam->rDstRect.bottom = pOverlay->rDstRect.bottom;
		}
	}

	prParam->u4Flags = pOverlay->u4Flags;
	prParam->u4SrcType = pOverlay->device_name;
	prParam->fgProgSrc = pOverlay->fgProgSrc;

#if !LINUX_FUN
	if (prParam->u4SrcType == USB) {
#endif
		prParam->fgTopFiledFirst = pOverlay->fgTopFiledFirst;
#if !LINUX_FUN
	} else {
		prParam->fgTopFiledFirst = TRUE;
	}
#endif

	prParam->fgRepeatFirstField = pOverlay->fgRepeatFirstField;
	prParam->fgProgSeq = pOverlay->fgProgSeq;
	prParam->fgPullDownFlagValid = pOverlay->fgPullDownFlagValid;

	return TRUE;
}

bool vGetWidthHeight(__u32 u4Mode, __u32 *prWidth, __u32 *prHeight)
{
	switch (u4Mode) {
	case RES_480P:
	case RES_480I:
		*prWidth = 720;
		*prHeight = 480;
		break;

	case RES_576P:
	case RES_576I:
		*prWidth = 720;
		*prHeight = 576;
		break;

	/*case RES_480P_800:
	case RES_480P_800_50HZ:
		*prWidth = 800;
		*prHeight = 480;
		break;

	case RES_600P_800:
	case RES_600P_800_50HZ:
		*prWidth = 800;
		*prHeight = 600;
		break;

	case RES_600P_1024:
	case RES_600P_1024_50HZ:
		*prWidth = 1024;
		*prHeight = 600;
		break;

	case RES_768P_1024:
		*prWidth = 1024;
		*prHeight = 768;
		break;

	case RES_800P_1280:
	case RES_800P_1280_50HZ:
		*prWidth = 1280;
		*prHeight = 800;
		break;*/

	default:
		*prWidth = 720;
		*prHeight = 480;
		return FALSE;
	}

	return TRUE;
}


static bool fgConfigVdpHw(__u32 u4VdpIdx)
{
	VDP_PARAM *prParam = &rData[u4VdpIdx];
	__u32 u4ObjWidth, u4ObjHeight, u4CurIdx;
	RECT rObj;
	PRECT pRect = &prParam->rDstRect;
#ifdef VCP_FOR_ANDROID
	int SrcTypeNum;
#endif
	if (u4VdpIdx > VDP_2) {
		FB_PRINT(FB_LOG_LVL_ERR, "VDP", "fgConfigVdpHw: do not support u4VdpIdx = %d\r\n", (int)u4VdpIdx);
		return FALSE;
	}
#ifdef VCP_FOR_ANDROID
	if (!(prParam->u4Flags & VDP_TOP_LEVEL)) { /*arm1 vcp on/off controlled by vdo update overlay*/
		/*default vcp is off,if the srctype in vcp records,turn on*/
		vCPVideoOff();

		if (prParam->u4SrcType) {
			for (SrcTypeNum = 0; SrcTypeNum < srcMax; SrcTypeNum++) {
				if (prParam->u4SrcType == vCPGetSrcType(SrcTypeNum)) {
					break;
				}
			}

			if (SrcTypeNum < srcMax) {
				vCPVideoOn();
				vCPVideoSetVCP(prParam->u4SrcType);
			} else {
				FB_PRINT(FB_LOG_LVL_INFO, "VDP", "srctype:%x is not support\r\n"
					, (unsigned int)prParam->u4SrcType);
			}
		}
	}
#endif
	if (u4VdpIdx == VDP_1) {
		vSclHalSetMasterSrc(prParam->u4VdpMode);

		if (prParam->u4VdpMode <= RES_576P) { /* Front SD mode need setting panel scaler*/
			vGetWidthHeight(prParam->u4VdpMode, &_u4VdpOutWidth, &_u4VdpOutHeight);
			RemapVdpRect(&prParam->rDstRect, &rObj, _u4VdpOutWidth, _u4VdpOutHeight
				, _u4LCDWidth, _u4LCDHeight);
			FB_PRINT(FB_LOG_LVL_DBG, "VDP", "MidRgn top %d, left %d, bottom %d, right %d\r\n",
				(int)rObj.top, (int)rObj.left, (int)rObj.bottom, (int)rObj.right);
			if (prParam->u4DeintMode >= VDP_DI_MA4F_MODE) {
				u4ObjWidth = rObj.right - rObj.left + 16;
			} else {
				u4ObjWidth = rObj.right - rObj.left;
			}
			u4ObjHeight = rObj.bottom - rObj.top;
                        if(u4ObjWidth == 0 || u4ObjHeight == 0)
                        {
                                FB_PRINT(FB_LOG_LVL_ERR, "VDP", "SD Video remap rect 0 error,width:%d, height:%d !\r\n", u4ObjWidth, u4ObjHeight);
                                return FALSE;
                        }
			vVdpHalSetOutRegion(u4VdpIdx, rObj.left, rObj.top, u4ObjWidth, u4ObjHeight);
			vSclHalSetHScale((_u4VdpOutWidth << SCL_HSCALE_SHIFT) / _u4LCDWidth);
			vSclHalSetVScale((_u4VdpOutHeight << SCL_VSCALE_SHIFT) / _u4LCDHeight);
			vSclHalSetWinActive(pRect->top, pRect->bottom, pRect->left, pRect->right);
		} else {
			vVdpHalSetOutRegion(u4VdpIdx, pRect->left, pRect->top,
				pRect->right - pRect->left, pRect->bottom - pRect->top);
		}
	} else {
		vVdpHalSetOutRegion(u4VdpIdx, pRect->left, pRect->top
			, pRect->right - pRect->left, pRect->bottom - pRect->top);
	}

	pRect = &prParam->rSrcRect;
	vVdpHalSetSrcSize(u4VdpIdx, prParam->u4SrcWidth, prParam->u4SrcHeight);
	vVdpHalSetSrcRegion(u4VdpIdx, pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top);
	vVdpHalSetYuv422(u4VdpIdx, FALSE); /* default is YUV420 mode*/
	vVdpHalSetScanLine(u4VdpIdx, prParam->u4Flags & VDP_SCANLINE_MODE);
	vPmxHalSetMode(u4VdpIdx, prParam->u4VdpMode);

	if (prParam->u4VdpMode <= RES_576P) {
		vPmxHalSet709To601(u4VdpIdx, FALSE, FALSE);
	} else {
		vPmxHalSet709To601(u4VdpIdx, TRUE, TRUE);
	}

	vVdpHalSetMode(u4VdpIdx, prParam->u4VdpMode);
	vVdpHalSetFifo(u4VdpIdx);

	if (prParam->u4VdpStatus == VDP_STATUS_SHOW) {
		u4CurIdx = prParam->u4CurrIdx;
		/* Updateoverlay more than once, and need clear unused buffer if sd source*/
		if (!prParam->fgNeedResizer) {
#if LINUX_FUN
			/*Linux backcar pal-nstc-pal*/
			prParam->u4NeedClrBuff |= VDP_CLR_REGU_BUF;
#else
			prParam->u4NeedClrBuff |= VDP_CLR_ONCE_BUF;
#endif
			/* cur buffer is ready to write, cur - 1 is ready to show and cur - 2 is unsused buffer*/
			prParam->u4ClearIdx = (u4CurIdx - 2 + MAX_BUFF_CNT) % MAX_BUFF_CNT;
		}

		prParam->u4PrevIdx = (u4CurIdx - 1 + MAX_BUFF_CNT) % MAX_BUFF_CNT;
		prParam->u4DispIdx = (u4CurIdx - 1 + MAX_BUFF_CNT) % MAX_BUFF_CNT;
		prParam->fgFirstField = TRUE;
		prParam->u4NeedShowBuff = 0;
		FB_PRINT(FB_LOG_LVL_DBG, "VDP", "fgConfigVdpHw: u4VdpIdx = %d, Curr = %d, Disp = %d\r\n"
			, (int)u4VdpIdx, (int)u4CurIdx, (int)prParam->u4DispIdx);
	}

	prParam->u4VdpStatus = VDP_STATUS_PREPARE;

	if (prParam->u4DeintMode < VDP_DI_MA4F_MODE) { /* MA4F need 4 buffer to show 1 frame*/
		prParam->u4NeedShowBuff++;

		vVdpHalSetFieldInfo(u4VdpIdx, prParam->fgTopFiledFirst, prParam->fgFirstField);
#if 1
		if (fgUpdateHwAddr(u4VdpIdx)) {
			vUpdateDispIdx(u4VdpIdx);

			if (!vPmxHalGetFmtEn(u4VdpIdx)) {
				/* Enable FMT to show video*/
				vPmxHalEnableFmt(u4VdpIdx);
				vVdpHalReset(u4VdpIdx);
				FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s hw %d enable fmt\r\n", __func__, u4VdpIdx);
			}

			if ((u4VdpIdx == VDP_1) && !fgPmxHalMixPlane(PMX_1, PMX_HW_PLANE_1)) {
				/* Front need set mix layer and rear setting in tve module*/
				vPmxHalMixPlane(PMX_1, PMX_HW_PLANE_1);
				FB_PRINT(FB_LOG_LVL_INFO, "VDP", "enable video layer\r\n");
				vPmxHalNotMixPlaneDelay(PMX_1, PMX_HW_PLANE_3);
				FB_PRINT(FB_LOG_LVL_INFO, "VDP", "disable osd1 layer\r\n");
			}

			prParam->u4VdpStatus = VDP_STATUS_SHOW;
		}
#endif
	}
#if 0
#ifdef CONFIG_ATC_OS_linux
	else {
		prParam->u4CurrIdx++;
		if (prParam->u4CurrIdx == MAX_BUFF_CNT) {
			prParam->u4CurrIdx = 0;
		}
	}
#endif
#endif
	vDumpVdpParam(u4VdpIdx);

	return TRUE;
}

bool fgHDSource(VDP_PARAM *prParam)
{
	bool ret = ((prParam->u4SrcWidth > 720) || (prParam->u4SrcHeight > 576)) ? TRUE : FALSE;

	return ret;
}


bool fgNeedDeint(VDP_PARAM *prParam)
{
	bool fgResult = TRUE;

#ifdef CONFIG_ATC_OS_android
	if (prParam->fgProgSrc || !(prParam->u4Flags & VDP_ENABLE_DEINT) || fgHDSource(prParam)) {
			fgResult = FALSE;
	}
#else
	if (prParam->fgProgSrc || !(prParam->u4Flags & VDP_ENABLE_DEINT) ) {
		fgResult = FALSE;
	}
#endif

#if !ENABLE_MM_DEINTERLACE
	else if (prParam->u4SrcType == USB) {
		fgResult = FALSE;
	}
#endif

	return fgResult;
}

#define RECT_WIDTH(r)         ((r)->right - (r)->left)
#define RECT_HEIGHT(r)         ((r)->bottom - (r)->top)

bool fgDestRectMatchedHD(RECT *prRect)
{
	bool ret = ((RECT_WIDTH(prRect) == _u4LCDWidth) && (RECT_HEIGHT(prRect) > _u4LCDHeight / 2)) ? TRUE : FALSE;

	return ret;
}



void vSetVdpPath(__u32 u4VdpIdx, VDP_PARAM *prParam)
{
	if (u4VdpIdx == VDP_1) {
		if (fgHDSource(prParam)) {
			prParam->u4VdpMode = _u4DispMode;
		} else {
			prParam->u4VdpMode = ((_u4LCDWidth == 1920) && (_u4LCDHeight == 720)) ? RES_576P : RES_480P;
		}
	} else {
		if ((!prParam->fgNeedResizer) && (prParam->u4SrcHeight > 480)) { /* 720x480 < src < 720x576*/
			prParam->u4VdpMode = RES_576P;
		} else { /* (src < 720x480) or (src > 720x576 need to resizer to 720x480)*/
			prParam->u4VdpMode = RES_480P;
		}
	}
}

void vSetDeintMode(__u32 u4VdpIdx, VDP_PARAM *prParam)
{
	__u32 u4OldDeintMode = prParam->u4DeintMode;

#if ENABLE_HD_DEINTERLACE
	if (prParam->u4VdpMode == _u4DispMode) {
#if !LINUX_FUN
		if (fgNeedDeint(prParam) && (prParam->u4SrcType == USB))
#else
		if (fgNeedDeint(prParam))
#endif
		{
			prParam->u4DeintMode = VDP_DI_HD_MODE;
		}
	} else
#endif
	{
		#ifdef CONFIG_ATC_OS_linux
		if ((fgNeedDeint(prParam))&&(!prParam->fgNeedResizer)) {
				prParam->u4DeintMode = VDP_DI_MA4F_MODE;
		}
		#else
		if (fgNeedDeint(prParam)) {
			prParam->u4DeintMode = VDP_DI_MA4F_MODE;
		}
		else {
                        if(!(prParam->u4Flags & VDP_TOP_LEVEL))
			        prParam->u4DeintMode = VDP_DI_FRAME_MODE;
		}
		#endif
	}

	if ((prParam->u4DeintMode != u4OldDeintMode) || (prParam->u4Flags & VDP_UPDATE_OVERLAY)) {
		FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s: idx %d, old %d, new %d, vdp mode %d\r\n", __func__, (int)u4VdpIdx,
			(int)u4OldDeintMode, (int)prParam->u4DeintMode, (int)prParam->u4VdpMode);

		if (prParam->u4DeintMode >= VDP_DI_MA4F_MODE) {
#if LINUX_FUN
			/*Linux backcar pal-nstc-pal*/
			prParam->u4NeedClrBuff |= VDP_CLR_REGU_BUF;
#else
			prParam->u4NeedClrBuff |= VDP_CLR_ONCE_BUF;
#endif
			prParam->u4ClearIdx = (prParam->u4DispIdx - 1 + MAX_BUFF_CNT) % MAX_BUFF_CNT; /* unused idx*/
			prParam->u4PrevIdx = prParam->u4DispIdx;
			FB_PRINT(FB_LOG_LVL_DBG, "VDP", "vSetDeintMode: idx %d, prev %d, disp %d, status %d\r\n"
				, (int)u4VdpIdx, (int)prParam->u4PrevIdx
				, (int)prParam->u4DispIdx, (int)prParam->u4VdpStatus);
			SetPulldownMode(u4VdpIdx, PULLDOWN_MODE_UNKNOWN);
#if ENABLE_PD_DEINTERLACE

#if !LINUX_FUN
			if (prParam->u4SrcType == USB) {
#else
			if (prParam->u4Flags & VDP_ENABLE_PDDI) {
#endif
				vVdpHalSetPdDetect(u4VdpIdx, TRUE);

			} else {
				vVdpHalSetPdDetect(u4VdpIdx, FALSE);
			}


#else
			vVdpHalSetPdDetect(u4VdpIdx, FALSE);
#endif
		} else {
#if LINUX_FUN
			/*Linux backcar pal-nstc-pal*/
			prParam->u4NeedClrBuff |= VDP_CLR_REGU_BUF;
#else
			prParam->u4NeedClrBuff |= VDP_CLR_ONCE_BUF;
#endif
			prParam->u4ClearIdx = prParam->u4PrevIdx; /* unused buffer idx*/
			SetPulldownMode(u4VdpIdx, PULLDOWN_MODE_UNKNOWN);
			vVdpHalSetPdDetect(u4VdpIdx, FALSE);
		}

		vVdpHalSetDeintMode(u4VdpIdx, prParam->u4DeintMode);
	}
}

bool fgCheckVdpFlags(__u32 u4VdpIdx, VDP_PARAM *prParam)
{
	__u32 u4Idx = prParam->u4CurrIdx;

	if ((prParam->u4Flags & VDP_PLAY_FF_RW) && (prParam->u4VdpStatus != VDP_STATUS_FFRW)) { /* enter fast forward*/
		if (prParam->u4DeintMode >= VDP_DI_MA4F_MODE) {
			prParam->u4DeintMode = VDP_DI_FIELD_MODE; /* fast forward only used field mode*/
			prParam->u4NeedClrBuff |= VDP_CLR_ONCE_BUF;
			/* cur buffer is ready to write, cur - 1 is ready to show and cur - 2 is unsused buffer*/
			prParam->u4ClearIdx = (u4Idx - 2 + MAX_BUFF_CNT) % MAX_BUFF_CNT;
			vVdpHalSetDeintMode(u4VdpIdx, VDP_DI_FIELD_MODE);
		}

		prParam->u4VdpStatus = VDP_STATUS_FFRW;
		FB_PRINT(FB_LOG_LVL_INFO, "VDP", "fgCheckVdpFlags: enter fast forward flags %x, status %d, di %d\r\n",
			(unsigned int)prParam->u4Flags, (int)prParam->u4VdpStatus, (int)prParam->u4DeintMode);
	} else if (!(prParam->u4Flags & VDP_PLAY_FF_RW) && (prParam->u4VdpStatus == VDP_STATUS_FFRW)) { /*exit FF*/
		vSetVdpPath(u4VdpIdx, prParam);

		if (prParam->u4DeintMode >= VDP_DI_MA4F_MODE) {
			prParam->u4NeedClrBuff |= VDP_CLR_ONCE_BUF;
			/* cur buffer is ready to write, cur - 1 is ready to show and cur - 2 is unsused buffer*/
			prParam->u4ClearIdx = (u4Idx - 2 + MAX_BUFF_CNT) % MAX_BUFF_CNT;
			prParam->u4PrevIdx = (u4Idx - 1 + MAX_BUFF_CNT) % MAX_BUFF_CNT;
			prParam->u4DispIdx = (u4Idx - 1 + MAX_BUFF_CNT) % MAX_BUFF_CNT;
			prParam->fgFirstField = TRUE;
			prParam->u4NeedShowBuff = 0;
			prParam->u4VdpStatus = VDP_STATUS_PREPARE;
		} else {
			prParam->u4VdpStatus = VDP_STATUS_SHOW;
		}

		FB_PRINT(FB_LOG_LVL_INFO, "VDP", "fgCheckVdpFlags: exit fast forward flags %x, status %d, di %d\r\n",
			(unsigned int)prParam->u4Flags, (int)prParam->u4VdpStatus, (int)prParam->u4DeintMode);
	} else if (prParam->u4Flags & VDP_SEEK_LOCATE) {
		if (prParam->u4DeintMode >= VDP_DI_MA4F_MODE) {
			prParam->u4NeedClrBuff |= VDP_CLR_ONCE_BUF;
			/* cur buffer is ready to write, cur - 1 is ready to show and cur - 2 is unsused buffer*/
			prParam->u4ClearIdx = (u4Idx - 2 + MAX_BUFF_CNT) % MAX_BUFF_CNT;
			prParam->u4PrevIdx = (u4Idx - 1 + MAX_BUFF_CNT) % MAX_BUFF_CNT;
			prParam->u4DispIdx = (u4Idx - 1 + MAX_BUFF_CNT) % MAX_BUFF_CNT;
			prParam->fgFirstField = TRUE;
			prParam->u4NeedShowBuff = 0;
		}

		prParam->u4VdpStatus = VDP_STATUS_SEEK;
		FB_PRINT(FB_LOG_LVL_INFO, "VDP", "fgCheckVdpFlags: seek flags %x, status %d, di %d\r\n",
			(unsigned int)prParam->u4Flags, (int)prParam->u4VdpStatus, (int)prParam->u4DeintMode);
	}

	return TRUE;
}

bool fgNeedResizeBuf(VDP_PARAM *prParam)
{
	bool fgResult = FALSE;
	__u32 u4DstWidth = prParam->rDstRect.right - prParam->rDstRect.left;
	__u32 u4DstHeight = prParam->rDstRect.bottom - prParam->rDstRect.top;
	__u32 u4SrcWidth = prParam->rSrcRect.right - prParam->rSrcRect.left;
	__u32 u4SrcHeight = prParam->rSrcRect.bottom - prParam->rSrcRect.top;

	fgResult = fgHDSource(prParam);

	if (!fgResult) {
		/* HW limit: FMT H scaler down 1/2, X half H scaler down 1/4 in total; and use image resizer*/
		if (((prParam->u4SrcWidth >> 1) > u4DstWidth) || ((prParam->u4SrcHeight >> 2) > u4DstHeight) ||
		    ((prParam->u4SrcWidth << 2) < u4DstWidth) || ((prParam->u4SrcHeight << 2) < u4DstHeight)) {
			fgResult = TRUE;
		} else {
			fgResult = FALSE;
		}
	}
	if ((prParam->u4Flags & VDP_ROTATE_90) || (prParam->u4Flags & VDP_ROTATE_270)) {
		if ((u4SrcWidth == u4DstHeight) && (u4SrcHeight == u4DstWidth)) {
			fgResult = FALSE;
		}
	} else {
		if ((u4SrcWidth == u4DstWidth) && (u4SrcHeight == u4DstHeight)) {
			fgResult = FALSE;
		}
	}

	return fgResult;
}

bool fgClrSetHDFrmBuff(VDP_PARAM *prParam, __u32 u4BufIdx, __u32 u4YAddr, __u32 u4CAddr)
{
	/* Release buffer and set new buffer address*/
	fgClearFrmBuff(prParam, u4BufIdx, u4BufIdx);

	if ((u4YAddr & 0x7F) || (u4CAddr & 0x7F)) {
		FB_PRINT(FB_LOG_LVL_ERR, "VDP", "fgClrSetHDFrmBuff: error buffer is not 128 bytes alignment %x, %x\r\n",
			(unsigned int)u4YAddr, (unsigned int)u4CAddr);
		return FALSE;
	}

	prParam->u4FrmBuffY[u4BufIdx] = u4YAddr;
	prParam->u4FrmBuffC[u4BufIdx] = u4CAddr;

	if (prParam->u4DeintMode >= VDP_DI_MA4F_MODE) {
		prParam->u4Duration[u4BufIdx] = 3000;
	} else {
		prParam->u4Duration[u4BufIdx] = 0;
	}

	if (prParam->u4NeedClrBuff != VDP_CLR_NONE_BUF) {
		FB_PRINT(FB_LOG_LVL_DBG, "VDP", "fgClrSetHDFrmBuff: u4NeedClrBuff error id %d, flga %x\r\n",
			(int)u4BufIdx, (unsigned int)prParam->u4NeedClrBuff);
		prParam->u4NeedClrBuff = VDP_CLR_NONE_BUF;
	}

	return TRUE;
}

//use irt_dma to rotate bufferstatic
BOOL fgRotateBuffer(VDP_PARAM *prParam, PMTKSurface pDstSurf)
{
	UINT32 u4BufIdx = (prParam->u4CurrIdx - 1 + MAX_BUFF_CNT) % MAX_BUFF_CNT;
	UINT32 u4Size = ROUND_UP_COUNT(prParam->u4SrcWidth, 16) * ROUND_UP_COUNT(prParam->u4SrcHeight, 32);
	UINT32 u4YPA = pDstSurf->PhysicalAddress(pDstSurf);
	UINT32 u4CPA = pDstSurf->PhysicalAddress(pDstSurf) + u4Size;    //UINT32 u4Org_left, u4Org_right, u4Org_top, u4Org_bottom = 0;
	RECT rOrg_SrcRect;
	IRT_DMA_APP_INFO_T rIRDCtrlPara;
	memset(&rIRDCtrlPara,0,sizeof(rIRDCtrlPara));
	rIRDCtrlPara.pu4SrcYBufAddr = prParam->u4FrmBuffY[u4BufIdx];
	rIRDCtrlPara.pu4SrcCBufAddr = prParam->u4FrmBuffC[u4BufIdx];
	rIRDCtrlPara.pu4DstYBufAddr = u4YPA;
	rIRDCtrlPara.pu4DstCBufAddr = u4CPA;
	rIRDCtrlPara.u4FrameWidth   = prParam->u4SrcWidth;
	rIRDCtrlPara.u4FrameHeight  = prParam->u4SrcHeight;
	rIRDCtrlPara.fgScanLineMode = prParam->u4Flags & VDP_SCANLINE_MODE;
	rIRDCtrlPara.fg5351Mode     = FALSE;
	rIRDCtrlPara.fgBlockBurstRead = !(prParam->u4Flags & VDP_SCANLINE_MODE);
	rIRDCtrlPara.eModeOpt       = prParam->u4RotDegree;
	SpinUnlock(SPIN_LOCK_SRC_RESIZE); // Can not cost much time in spin lock case
	IrtDma_Rotate(&rIRDCtrlPara);
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "fgRotateBuffer: srcY:%x, srcC:%x, dstY:%x, dstC:%x width %d, height %d \n",
		rIRDCtrlPara.pu4SrcYBufAddr, rIRDCtrlPara.pu4SrcCBufAddr, rIRDCtrlPara.pu4DstYBufAddr, rIRDCtrlPara.pu4DstCBufAddr, (int)rIRDCtrlPara.u4FrameWidth, (int)rIRDCtrlPara.u4FrameHeight);
	SpinLock(SPIN_LOCK_SRC_RESIZE);
	prParam->u4Flags |= VDP_SCANLINE_MODE;
	memcpy(&rOrg_SrcRect, &prParam->rSrcRect, sizeof(RECT));
	if(prParam->u4RotDegree == IRT_DMA_MODE_ROTATE_270)
		{
		prParam->rSrcRect.left = rOrg_SrcRect.top;
		prParam->rSrcRect.right = rOrg_SrcRect.bottom;
		prParam->rSrcRect.top = rIRDCtrlPara.u4FrameWidth - rOrg_SrcRect.right;
		prParam->rSrcRect.bottom = rIRDCtrlPara.u4FrameWidth - rOrg_SrcRect.left;
		prParam->u4SrcWidth = ROUND_UP_COUNT(rIRDCtrlPara.u4FrameHeight, 32);
		prParam->u4SrcHeight = rIRDCtrlPara.u4FrameWidth;
		}
	else if (prParam->u4RotDegree == IRT_DMA_MODE_ROTATE_90)
		{
		prParam->u4SrcWidth = ROUND_UP_COUNT(rIRDCtrlPara.u4FrameHeight, 32);
		prParam->u4SrcHeight = rIRDCtrlPara.u4FrameWidth;
		prParam->rSrcRect.left = prParam->u4SrcWidth - rOrg_SrcRect.bottom;
		prParam->rSrcRect.right = prParam->u4SrcWidth - rOrg_SrcRect.top;
		prParam->rSrcRect.top = rOrg_SrcRect.left;
		prParam->rSrcRect.bottom = rOrg_SrcRect.right;
		}
	prParam->u4FrmBuffY[u4BufIdx] = u4YPA;
	prParam->u4FrmBuffC[u4BufIdx] = u4CPA;
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "loki new src: W:%d, H:%d, srcRect:(%d, %d, %d, %d)\n",prParam->u4SrcWidth, prParam->u4SrcHeight, prParam->rSrcRect.left, prParam->rSrcRect.top, prParam->rSrcRect.right,prParam->rSrcRect.bottom);
	return TRUE;
	}

static bool fgResizerBuffer(VDP_PARAM *prParam, PMTKSurface pDstSurf)
{
	IMGRESZ_HAL_IMG_INFO_T rSrcBufInfo;
	IMGRESZ_HAL_IMG_INFO_T rDstBufInfo;
	BOOL result = FALSE;
	__u32 u4BufIdx = (prParam->u4CurrIdx - 1 + MAX_BUFF_CNT) % MAX_BUFF_CNT;
	__u32 u4YAddr = pDstSurf->PhysicalAddress(pDstSurf);
	__u32 u4CAddr = pDstSurf->PhysicalAddress(pDstSurf) +
			 ROUND_UP_COUNT(pDstSurf->Width(pDstSurf), 16) * ROUND_UP_COUNT(pDstSurf->Height(pDstSurf), 32);
	bool fg8Tap = FALSE;

	memset(&rSrcBufInfo, 0, sizeof(rSrcBufInfo));
	rSrcBufInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER;
	rSrcBufInfo.rBufferFormat.eYUVFormat = IMGRESZ_HAL_IMG_YUV_FORMAT_420;
	rSrcBufInfo.rBufferFormat.fgBlockMode = !(prParam->u4Flags & VDP_SCANLINE_MODE);
	rSrcBufInfo.rBufferFormat.fgAddrSwap = TRUE;
//#ifdef CONFIG_ATC_OS_android
#if 1
	if(!prParam->fgNeedRotate)
    {
        rSrcBufInfo.u4BufWidth = ROUND_UP_COUNT(prParam->u4SrcWidth, 16);
        rSrcBufInfo.u4BufHeight = ROUND_UP_COUNT(prParam->u4SrcHeight, 32);
        rSrcBufInfo.u4ImgWidth = prParam->rSrcRect.right;
        rSrcBufInfo.u4ImgHeight = prParam->rSrcRect.bottom;
        rSrcBufInfo.u4ImgXOff = 0;
        rSrcBufInfo.u4ImgYOff = 0;
    }
    else
    {
        rSrcBufInfo.u4BufWidth = prParam->u4SrcWidth;
        rSrcBufInfo.u4BufHeight = prParam->u4SrcHeight;
        rSrcBufInfo.u4ImgWidth = prParam->rSrcRect.right - prParam->rSrcRect.left;
        rSrcBufInfo.u4ImgHeight = prParam->rSrcRect.bottom - prParam->rSrcRect.top;
        rSrcBufInfo.u4ImgXOff = prParam->rSrcRect.left;
        rSrcBufInfo.u4ImgYOff = prParam->rSrcRect.top;
    }
#else
	rSrcBufInfo.u4BufWidth = ROUND_UP_COUNT(prParam->u4SrcWidth, 16);
	rSrcBufInfo.u4BufHeight = ROUND_UP_COUNT(prParam->u4SrcHeight, 32);
	rSrcBufInfo.u4ImgWidth = prParam->rSrcRect.right;

	rSrcBufInfo.u4ImgXOff = 0;
	rSrcBufInfo.u4ImgYOff = 0;
#endif
	rSrcBufInfo.u4BufSA1 = prParam->u4FrmBuffY[u4BufIdx];
	rSrcBufInfo.u4BufSA2 = prParam->u4FrmBuffC[u4BufIdx];

	/* Set target buffer info*/
	memset(&rDstBufInfo, 0, sizeof(rSrcBufInfo));
	rDstBufInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER;
	rDstBufInfo.rBufferFormat.eYUVFormat = IMGRESZ_HAL_IMG_YUV_FORMAT_420;
	rDstBufInfo.rBufferFormat.fgBlockMode = !(prParam->u4Flags & VDP_SCANLINE_MODE);
	rDstBufInfo.rBufferFormat.fgAddrSwap = TRUE;
//#ifdef CONFIG_ATC_OS_android
#if 1
	if(!prParam->fgNeedRotate)
		{
			rDstBufInfo.u4BufWidth =  ROUND_UP_COUNT(pDstSurf->Width(pDstSurf), 16);
			rDstBufInfo.u4BufHeight = ROUND_UP_COUNT(pDstSurf->Height(pDstSurf), 32);
		}
	else
		{
			rDstBufInfo.u4BufWidth =  ROUND_UP_COUNT(pDstSurf->Width(pDstSurf), 16);
			rDstBufInfo.u4BufHeight = pDstSurf->Height(pDstSurf);
		}
#else
	rDstBufInfo.u4BufWidth =  ROUND_UP_COUNT(pDstSurf->Width(pDstSurf), 16);
	rDstBufInfo.u4BufHeight = ROUND_UP_COUNT(pDstSurf->Height(pDstSurf), 32);
#endif

	rDstBufInfo.u4ImgWidth = pDstSurf->Width(pDstSurf);

	rDstBufInfo.u4ImgXOff = 0;
	rDstBufInfo.u4ImgYOff = 0;
	rDstBufInfo.u4BufSA1 = u4YAddr;
	rDstBufInfo.u4BufSA2 = u4CAddr;

#if ENABLE_HD_DEINTERLACE
	if (prParam->u4DeintMode == VDP_DI_HD_MODE) {
		rSrcBufInfo.rBufferFormat.fgProgressiveFrame = FALSE;
		rSrcBufInfo.rBufferFormat.fgTopField = TRUE;
		rSrcBufInfo.u4ImgHeight = prParam->u4SrcHeight >> 1;

		rDstBufInfo.rBufferFormat.fgProgressiveFrame = FALSE;
		rDstBufInfo.rBufferFormat.fgTopField = TRUE;
		rDstBufInfo.u4ImgHeight = pDstSurf->Height(pDstSurf) >> 1;
	} else
#endif
	{
		rSrcBufInfo.rBufferFormat.fgProgressiveFrame = TRUE;
	#ifdef CONFIG_ATC_OS_linux
		rSrcBufInfo.u4ImgHeight = prParam->rSrcRect.bottom;
	#endif

		rDstBufInfo.rBufferFormat.fgProgressiveFrame = TRUE;
		rDstBufInfo.u4ImgHeight = pDstSurf->Height(pDstSurf);
	}

	SpinUnlock(SPIN_LOCK_SRC_RESIZE); /* Can not sleep in spin lock case*/

	/*if wch srctype ,both front and rear now, imgresz use H-8Tap method when > 720p*/
	if ((prParam->u4SrcType != USB) && (rData[0].fgNeedResizer && rData[1].fgNeedResizer)) {
		if (rSrcBufInfo.u4ImgWidth > 1280 || rSrcBufInfo.u4ImgHeight > 720) {
			fg8Tap = TRUE;
		}
	}

	result = TS_DirectScale(1, fg8Tap, &rSrcBufInfo, &rDstBufInfo);
	/*FB_PRINT(FB_LOG_LVL_DBG, "VDP", "fgResizerBuffer: 1 in src buf %d, %d, img %d, %d, dst buf %d, %d, img %d, %d\n"
		, (int)rSrcBufInfo.u4BufWidth, (int)rSrcBufInfo.u4BufHeight, (int)rSrcBufInfo.u4ImgWidth
		, (int)rSrcBufInfo.u4ImgHeight, (int)rDstBufInfo.u4BufWidth, (int)rDstBufInfo.u4BufHeight
		, (int)rDstBufInfo.u4ImgWidth, (int)rDstBufInfo.u4ImgHeight);*/

#if ENABLE_HD_DEINTERLACE
	if (prParam->u4DeintMode == VDP_DI_HD_MODE) {
		rSrcBufInfo.rBufferFormat.fgTopField = FALSE;
		rDstBufInfo.rBufferFormat.fgTopField = FALSE;
		result= TS_DirectScale(1, fg8Tap, &rSrcBufInfo, &rDstBufInfo);
		/*FB_PRINT(FB_LOG_LVL_DBG, "VDP", "fgResizerBuffer: 2 src %d, %d, img %d, %d, dst %d, %d, img %d, %d\n"
			, (int)rSrcBufInfo.u4BufWidth, (int)rSrcBufInfo.u4BufHeight
			, (int)rSrcBufInfo.u4ImgWidth, (int)rSrcBufInfo.u4ImgHeight
			, (int)rDstBufInfo.u4BufWidth, (int)rDstBufInfo.u4BufHeight
			, (int)rDstBufInfo.u4ImgWidth, (int)rDstBufInfo.u4ImgHeight);*/
	}
#endif
	SpinLock(SPIN_LOCK_SRC_RESIZE);
	if(result == FALSE)
		{
		   return FALSE;
		}

	if (!fgClrSetHDFrmBuff(prParam, u4BufIdx, u4YAddr, u4CAddr)) {
		return FALSE;
	}

	return TRUE;
}

bool fgHideVdp(__u32 u4VdpIdx)
{
	VDP_PARAM *prParam = &rData[u4VdpIdx];
#ifdef	CONFIG_ATC_OS_linux
#ifdef __ARM2__
	if (u4VdpIdx == VDP_1) {
		vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_1);
		FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s vPmxHalNotMixPlane video plane\r\n", __func__);
	}

	vPmxHalDisableFmt(u4VdpIdx);
#endif
#else
	if (u4VdpIdx == VDP_1) {
		vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_1);
		FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s vPmxHalNotMixPlane video plane\r\n", __func__);
	}

	vPmxHalDisableFmt(u4VdpIdx);
#endif

	if (prParam->u4NeedClrBuff == VDP_CLR_NONE_BUF) { /* MA4F need 4 buffer to show 1 frame*/
		fgResetParam(prParam);
	} else {
		fgClearFrmBuff(prParam, 0, MAX_BUFF_CNT - 1);
		fgResetParam(prParam);
	}
	FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s dev %d status %d\r\n", __func__, u4VdpIdx, prParam->u4VdpStatus);

	return TRUE;
}

static bool Hide_Video(__u32 u4VdpIdx)
{
#ifdef CONFIG_ATC_OS_linux
#ifdef __ARM2__
	fgHideVdp(u4VdpIdx);
#else
	/*Don't reset param in streamoff just set flag*/
	_fgInitVdpParam[u4VdpIdx] = FALSE;
#endif
#else
	fgHideVdp(u4VdpIdx);
#endif
	FBM_Uninit(u4VdpIdx);
	return TRUE;
}

void vGetDestBufferResolution(VDP_PARAM *prParam, __u32 *pu4Width, __u32 *pu4Height)
{
    //if DstWidth is an odd value, the last pixel  of ImgRsz is garbage
    __u32 u4Width  = SIZE_ALIGN((prParam->rDstRect.right - prParam->rDstRect.left),2);
	__u32 u4Height = prParam->rDstRect.bottom - prParam->rDstRect.top;

        #ifdef CONFIG_ATC_OS_linux
        if (((u4Width > 720) || (u4Height > 576)) && (prParam->u4VdpMode == RES_480P)) {
		/* SD mode not support HD width/height*/
                *pu4Width   = IMG_RESZ_SD_BUFF_WIDTH;
                *pu4Height  = IMG_RESZ_SD_BUFF_HEIGHT;
        }
        #else
        if (prParam->u4VdpMode <= RES_576P){
                if (u4Width > 720)
                        *pu4Width = IMG_RESZ_SD_BUFF_WIDTH;
                else
                        *pu4Width = u4Width;

                if (u4Height > 576)
                        *pu4Height = IMG_RESZ_SD_BUFF_HEIGHT;
                else
                        *pu4Height = u4Height;
        }
        #endif

#if ENABLE_HD_DEINTERLACE
	else if (prParam->u4DeintMode == VDP_DI_HD_MODE) {
		*pu4Width   = u4Width;
		*pu4Height  = u4Height << 1;
	}
#endif
	else {
	    if (u4Width >= 1840)
	    {
	    	*pu4Width   = 1840;
			prParam->rDstRect.right -= 40;
			prParam->rDstRect.left += 40;

	    	printk("[wts] set pic size %d \n",(int)*pu4Width);
	    }
	    else
	    {
	      *pu4Width   = u4Width;
	    }
		//*pu4Width   = u4Width;
		*pu4Height  = u4Height;
	}
}

void RemapSrcRect(RECT *prRect, PMTKSurface pDstSurf)
{
	__u32 u4NewWidth, u4NewHeight;
	__u32 u4OrignalAlignWidth, u4OrignalAlignHeight;
	__u32 u4OrignalWidth, u4OrignalHeight;

	u4NewWidth  =  ROUND_UP_COUNT(pDstSurf->Width(pDstSurf), 16);
	u4NewHeight  = ROUND_UP_COUNT(pDstSurf->Height(pDstSurf), 32);

	u4OrignalWidth =  prRect->right - prRect->left;
	u4OrignalHeight = prRect->bottom - prRect->top;

	u4OrignalAlignWidth =  SIZE_ALIGN(prRect->right - prRect->left, 16);
	u4OrignalAlignHeight = SIZE_ALIGN(prRect->bottom - prRect->top, 32);

	prRect->right     = prRect->left + u4OrignalWidth * u4NewWidth / u4OrignalAlignWidth;
	prRect->bottom = prRect->top  + u4OrignalHeight * u4NewHeight / u4OrignalAlignHeight;
}
static bool Show_Video(__u32 u4VdpIdx)
{
	VDP_PARAM *prParam = &rData[u4VdpIdx];
	bool fgNeedResizer = prParam->fgNeedResizer;

	if(((prParam->u4Flags & VDP_ROTATE_90) || (prParam->u4Flags & VDP_ROTATE_180) || (prParam->u4Flags & VDP_ROTATE_270)) && (u4VdpIdx == VDP_1))
    {
        PMTKSurface pBuffer = NULL;

        prParam->fgNeedRotate = TRUE;
        if(prParam->u4Flags & VDP_ROTATE_90) {
            prParam->u4RotDegree = IRT_DMA_MODE_ROTATE_90;
        } else if(prParam->u4Flags & VDP_ROTATE_180) {
            prParam->u4RotDegree = IRT_DMA_MODE_ROTATE_180;
        } else if(prParam->u4Flags & VDP_ROTATE_270) {
            prParam->u4RotDegree = IRT_DMA_MODE_ROTATE_270;
        } else {
            FB_PRINT(FB_LOG_LVL_ERR, "VDP", "vdo dont support rotate except 90/180/270 degree.\r\n");
        }

        if(pRotateSurf[u4VdpIdx] == NULL)
        {
            pRotateSurf[u4VdpIdx] = AllocRoateSurface(1088,1920);//width:32, height:16 alignment for irtdma
        }

        pBuffer = pRotateSurf[u4VdpIdx];

        if (prParam->u4Flags & VDP_UPDATE_OVERLAY)
        {
            UINT32 u4RotateDstW, u4RotateDstH;
			if (prParam->u4RotDegree & IRT_DMA_MODE_ROTATE_180) {
				u4RotateDstH = prParam->u4SrcHeight;
	            u4RotateDstW = prParam->u4SrcWidth;
			} else {
	            u4RotateDstH = prParam->u4SrcWidth;
	            u4RotateDstW = ROUND_UP_COUNT(prParam->u4SrcHeight, 32);
			}

            pBuffer->SetPicSize(pBuffer, u4RotateDstW, u4RotateDstH);
            FB_PRINT(FB_LOG_LVL_INFO, "VDP", "set pic size %d %d\n", (int)u4RotateDstW, (int)u4RotateDstH);
        }
        fgRotateBuffer(prParam, pBuffer);
    }

	if (prParam->u4SrcType == USB) {
		prParam->fgNeedResizer = 1;
	} else {
		prParam->fgNeedResizer = fgNeedResizeBuf(prParam);
	}

	if ((fgNeedResizer != prParam->fgNeedResizer) && !(prParam->u4Flags & VDP_UPDATE_OVERLAY)) {
		/* change buffer size need to reconfig vdp param*/
		FB_PRINT(FB_LOG_LVL_INFO, "VDP", "[Show_Video] hw id %d fgNeedResizer flag change to %d\r\n"
			, (int)u4VdpIdx, (int)prParam->fgNeedResizer);
		prParam->u4Flags |= VDP_UPDATE_OVERLAY;
	}

	if (prParam->u4Flags & VDP_UPDATE_OVERLAY) {
		vSetVdpPath(u4VdpIdx, prParam);
	}

	vSetDeintMode(u4VdpIdx, prParam);
	fgCheckVdpFlags(u4VdpIdx, prParam);
#ifdef CONFIG_ATC_OS_android
    if(prParam->fgNeedResizer&&(u4VdpIdx == VDP_2))
    {
        prParam->u4DeintMode = VDP_DI_FRAME_MODE;
	}
#endif
	//FB_PRINT(FB_LOG_LVL_INFO, "VDP", "Show_Video prParam->fgNeedResizer 0x%x\n", (unsigned int)prParam->fgNeedResizer);

	if (prParam->fgNeedResizer) {
		PMTKSurface pDstSurf = NULL;
		__u32 j = 0;
		PMTKSurface pBuffer = NULL;

		if (pImgDstSurf[u4VdpIdx][0] == NULL) { /* firstly allocate buffer, no need secondly*/
			for (j = 0; j < IMG_RESZ_BUFF_SIZE; j++) {
				/* Ying-ToDo: Alloc surface dynamic and get panel width and height*/
				pBuffer = AllocSurface(
					prParam->u4SrcWidth > IMG_RESZ_DST_BUFF_WIDTH ? prParam->u4SrcWidth : IMG_RESZ_DST_BUFF_WIDTH,
					prParam->u4SrcHeight > IMG_RESZ_DST_BUFF_HEIGHT ? prParam->u4SrcHeight : IMG_RESZ_DST_BUFF_HEIGHT);

				if (pBuffer == NULL) {
					FB_PRINT(FB_LOG_LVL_ERR, "VDP", "Can not allocate surface\r\n ");
				}

				pImgDstSurf[u4VdpIdx][j] = pBuffer;
			}
		}

		if (FBM_IsNotEmpty(u4VdpIdx) == FALSE) {
			FBM_Init(u4VdpIdx, (LPVOID *)pImgDstSurf[u4VdpIdx], IMG_RESZ_BUFF_SIZE);
		}

		if (prParam->u4Flags & VDP_UPDATE_OVERLAY) {
			__u32 u4DstWidth, u4DstHeight;

			vGetDestBufferResolution(prParam, &u4DstWidth, &u4DstHeight);

			for (j = 0; j < IMG_RESZ_BUFF_SIZE; j++) {
				pBuffer =  pImgDstSurf[u4VdpIdx][j];
				pBuffer->SetPicSize(pBuffer, u4DstWidth, u4DstHeight);
			}

			FB_PRINT(FB_LOG_LVL_DBG, "VDP", "set pic size %d %d\n", (int)u4DstWidth, (int)u4DstHeight);
		}

		pDstSurf = (PMTKSurface)FBM_Lock(u4VdpIdx);
		if(fgResizerBuffer(prParam, pDstSurf) == FALSE) {
			FB_PRINT(FB_LOG_LVL_DBG, "VDP", "resize buffer return false\n");
			return ;
		}
		FBM_Flip(u4VdpIdx);

		if (prParam->u4Flags & VDP_UPDATE_OVERLAY) {
			//#ifdef CONFIG_ATC_OS_android
			#if 1
			if(!prParam->fgNeedRotate)
            {
                prParam->rSrcRect.right = prParam->rSrcRect.left + pDstSurf->Width(pDstSurf);
                prParam->rSrcRect.bottom = prParam->rSrcRect.top + pDstSurf->Height(pDstSurf);
                prParam->u4SrcWidth = ROUND_UP_COUNT(pDstSurf->Width(pDstSurf), 16);
                prParam->u4SrcHeight = pDstSurf->Height(pDstSurf);//ROUND_UP_COUNT(pDstSurf->Height(pDstSurf), 32);
            }
            else
            {
                prParam->rSrcRect.left = 0;
                prParam->rSrcRect.top = 0;
                prParam->rSrcRect.right = pDstSurf->Width(pDstSurf);
                prParam->rSrcRect.bottom = pDstSurf->Height(pDstSurf);
                prParam->u4SrcWidth = ROUND_UP_COUNT(pDstSurf->Width(pDstSurf), 16);
                prParam->u4SrcHeight = pDstSurf->Height(pDstSurf);
            }
			#else
			prParam->rSrcRect.right = prParam->rSrcRect.left + pDstSurf->Width(pDstSurf);
			prParam->rSrcRect.bottom = prParam->rSrcRect.top + pDstSurf->Height(pDstSurf);

			/*RemapSrcRect(& prParam->rSrcRect, pDstSurf);*/

			prParam->u4SrcWidth = ROUND_UP_COUNT(pDstSurf->Width(pDstSurf), 16);
			prParam->u4SrcHeight = pDstSurf->Height(pDstSurf);//ROUND_UP_COUNT(pDstSurf->Height(pDstSurf), 32);
			#endif
		}


		prParam->u4NeedClrBuff = VDP_CLR_NONE_BUF;
	} else {
		prParam->u4NeedClrBuff = 0;
	}

	//FB_PRINT(FB_LOG_LVL_INFO, "VDP", "Show_Video prParam->u4Flags 0x%x\n", (unsigned int)prParam->u4Flags);
	if (prParam->u4Flags & VDP_UPDATE_OVERLAY) {
		fgConfigVdpHw(u4VdpIdx);
	} else if (prParam->u4DeintMode < VDP_DI_MA4F_MODE) {
		prParam->u4NeedShowBuff++;
#if 1

		if (fgUpdateHwAddr(u4VdpIdx)) {
			vUpdateDispIdx(u4VdpIdx);
		}

#endif
	}

	return TRUE;
}

void ResizeAndUpdateVDOHw(UINT32 u4VdpIdx)
{
    VDP_PARAM * prParam = &rData[u4VdpIdx];
	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "call ResizeAndUpdateVDOHw.\r\n");

    if (prParam->fgNeedResizer)
    {
    	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "fgNeedResizer is  true.\r\n");
        PMTKSurface pDstSurf = NULL;
        UINT32 j = 0;
        PMTKSurface pBuffer = NULL;
        if (pImgDstSurf[u4VdpIdx][0] == NULL)  // firstly allocate buffer, no need secondly
        {
            for (j=0; j < IMG_RESZ_BUFF_SIZE; j++)
            {
                // Ying-ToDo: Alloc surface dynamic and get panel width and height
                pBuffer = AllocSurface(IMG_RESZ_DST_BUFF_WIDTH, IMG_RESZ_DST_BUFF_HEIGHT);
                if (pBuffer == NULL)
                {
					 FB_PRINT(FB_LOG_LVL_ERR, "VDP", "Can not allocate surface.\r\n");
                }
                pImgDstSurf[u4VdpIdx][j] = pBuffer;
            }
        }
        if (FBM_IsNotEmpty(u4VdpIdx) == FALSE)
        {
            FBM_Init(u4VdpIdx, (LPVOID *)pImgDstSurf[u4VdpIdx], IMG_RESZ_BUFF_SIZE);
        }
        if (prParam->u4Flags & VDP_UPDATE_OVERLAY)
        {
            UINT32 u4DstWidth, u4DstHeight;

            vGetDestBufferResolution(prParam, &u4DstWidth, &u4DstHeight);

            for (j=0; j < IMG_RESZ_BUFF_SIZE; j++)
            {
                pBuffer =  pImgDstSurf[u4VdpIdx][j];
                pBuffer->SetPicSize(pBuffer, u4DstWidth, u4DstHeight);
            }
            printk("[resize] set pic size %d %d\n", (int)u4DstWidth, (int)u4DstHeight);
        }
        pDstSurf =(PMTKSurface)FBM_Lock(u4VdpIdx);
        if(fgResizerBuffer(prParam, pDstSurf)== FALSE)
        {
           FB_PRINT(FB_LOG_LVL_DBG, "VDP", "resize buffer return false\n");
            return;
        }
        FBM_Flip(u4VdpIdx);

        if (prParam->u4Flags & VDP_UPDATE_OVERLAY) {
            prParam->rSrcRect.left = 0;
            prParam->rSrcRect.top = 0;
            prParam->rSrcRect.right = pDstSurf->Width(pDstSurf);
            prParam->rSrcRect.bottom = pDstSurf->Height(pDstSurf);
            prParam->u4SrcWidth = ROUND_UP_COUNT(pDstSurf->Width(pDstSurf), 16);
            prParam->u4SrcHeight = pDstSurf->Height(pDstSurf);
            //RemapSrcRect(& prParam->rSrcRect, pDstSurf);
        }
        prParam->u4NeedClrBuff = VDP_CLR_NONE_BUF;
    }
    else
    {
        prParam->u4NeedClrBuff |= VDP_CLR_REGU_BUF;
    }

    //VDP_LOG(VDP_LOG_LVL_DBG, "[VDP]Show_Video prParam->u4Flags 0x%x\n", (unsigned int)prParam->u4Flags);
    if (prParam->u4Flags & VDP_UPDATE_OVERLAY) {
        //prParam->fgFirstField = TRUE;
        fgConfigVdpHw(u4VdpIdx);
    }
    else if (prParam->u4DeintMode < VDP_DI_MA4F_MODE)
    {
        prParam->u4NeedShowBuff++;
        #if 1
        if (fgUpdateHwAddr(u4VdpIdx))
        {
            vUpdateDispIdx(u4VdpIdx);
        }
        #endif
    }
}

int VDP_IOControl(__u32 dwCode, void *pBufIn,  void *pBufOut)
{
	__u32 u4Ret = 0, u4VdpIdx = 0, i, loop_cnt;
	bool ret = FALSE;
#ifdef CONFIG_ATC_OS_linux
	__u32 addrList[6];
#endif
	struct OVERLAY_PARAM rear_follow_with_front_vdp_param;
	struct OVERLAY_PARAM *pOverlay = NULL;
	VDP_PARAM *prParam = NULL;
#if TOPLevel
	VDP_PARAM *prBackuprParam = &rData[VDP_BACKUP];
#endif

	if (!pBufIn) {
		FB_PRINT(FB_LOG_LVL_ERR, "VDP", "VDP_IOControl: input buffer is null code = 0x%x\n", dwCode);
		return -EINVAL;
	}

	loop_cnt = (fr_follow & FR_FOLLOW_VIDEO) ? 2 : 1;

	switch (dwCode) {
	case VIDIOC_STREAMOFF: {
		/*if (!access_ok(VERIFY_READ, pBufIn, sizeof(struct OVERLAY_PARAM))) {
			u4Ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "VDP", "VIDIOC_STREAMOFF return %d\r\n", u4Ret);
			break;
		}*/
		pOverlay = (struct OVERLAY_PARAM *)pBufIn;
		FB_PRINT(FB_LOG_LVL_INFO, "VDP", "VDP_IOControl: VIDIOC_STREAMOFF cmd 0x%x id %d\n", dwCode, pOverlay->u4Idx);

		for (i = 0; i < loop_cnt; i++) {
			SpinLock(SPIN_LOCK_SRC_IOCTL);
			if ((i == VDP_2) && (fr_follow & FR_FOLLOW_VIDEO)) {
				if (vdo_fr_on) {
					FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s id from %d to %d\n", __func__, pOverlay->u4Idx, i);
					pOverlay->u4Idx = i;
					vdo_fr_on = FALSE;
				} else {
					FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s vdo fr off and do nothing\n", __func__);
				}
			}
#if TOPLevel
			if (_fgVDOTopLevel && (pOverlay->device_name != BACKCAR) && (pOverlay->u4Idx == VDP_1)) {
				FB_PRINT(FB_LOG_LVL_INFO, "VDP", "Hide backup video, do not update hw registers\r\n");
				fgResetParam(prBackuprParam);
			} else {
				u4VdpIdx = pOverlay->u4Idx;
				Hide_Video(u4VdpIdx);
			}

			if ((pOverlay->device_name == BACKCAR) && (u4VdpIdx == VDP_1)) {
				if (!fgPmxHalMixPlane(PMX_1, PRIMARY_SURF_PLANE)) {
					/*vPmxHalMixPlane(PMX_1, PRIMARY_SURF_PLANE);  // enable PRIMARY_SURF_ID layer*/
					FB_PRINT(FB_LOG_LVL_INFO, "VDP", "vdp mix primary surface plane %d\r\n", PRIMARY_SURF_PLANE);
				}

				/* Recover back up video if need*/
				if (prBackuprParam->u4VdpStatus != VDP_STATUS_HIDE) {
					FB_PRINT(FB_LOG_LVL_INFO, "VDP", "Recover backup video after hide backcar\r\n");
					prParam = &rData[u4VdpIdx];
					memcpy(prParam, prBackuprParam, sizeof(VDP_PARAM));
					fgResetParam(prBackuprParam);
					Show_Video(u4VdpIdx);
				}

				if (_fgVDOTopLevel) {
					_fgVDOTopLevel = FALSE;
				}
			}
#else
			if (!fgPmxHalMixPlane(PMX_1, PRIMARY_SURF_PLANE)) {
				vPmxHalMixPlane(PMX_1, PRIMARY_SURF_PLANE);  // enable PRIMARY_SURF_ID layer
				FB_PRINT(FB_LOG_LVL_WARN, "VDP", "vdp mix primary surface plane %d\r\n", PRIMARY_SURF_PLANE);
			}

			u4VdpIdx = pOverlay->u4Idx;
			Hide_Video(u4VdpIdx);
#endif
			SpinUnlock(SPIN_LOCK_SRC_IOCTL);
		}
		break;
	}
#ifdef CONFIG_ATC_OS_linux
#ifndef __ARM2__
	case VIDIOC_OVERLAY: {
			pOverlay = (struct OVERLAY_PARAM *)pBufIn;
			FB_PRINT(FB_LOG_LVL_INFO, "VDP", "VDP_IOControl: VIDIOC_OVERLAY cmd 0x%x id %d\n", dwCode, pOverlay->u4Idx);

			for (i = 0; i < loop_cnt; i++) {
				SpinLock(SPIN_LOCK_SRC_IOCTL);
				if ((i == VDP_2) && (fr_follow & FR_FOLLOW_VIDEO)) {
					if (vdo_fr_on) {
						FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s id from %d to %d\n", __func__, pOverlay->u4Idx, i);
						pOverlay->u4Idx = i;
						vdo_fr_on = FALSE;
					} else {
						FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s vdo fr off and do nothing\n", __func__);
					}
				}


				u4VdpIdx = pOverlay->u4Idx;
				prParam = &rData[u4VdpIdx];

				if(prParam->u4DeintMode >= VDP_DI_MA4F_MODE) {
					FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s set Hide Status u4DisplayIdx = %d u4CurrentIdx = %d u4ClearIdx = %d di mode = %d\r\n", __func__
							,prParam->u4DispIdx,prParam->u4CurrIdx,prParam->u4ClearIdx,prParam->u4DeintMode);
					addrList[0] = prParam->u4FrmBuffY[(prParam->u4CurrIdx - 2+MAX_BUFF_CNT)%MAX_BUFF_CNT];
					addrList[1] = prParam->u4FrmBuffC[(prParam->u4CurrIdx - 2+MAX_BUFF_CNT)%MAX_BUFF_CNT];
					addrList[2] = prParam->u4FrmBuffY[(prParam->u4CurrIdx - 3+MAX_BUFF_CNT)%MAX_BUFF_CNT];
					addrList[3] = prParam->u4FrmBuffC[(prParam->u4CurrIdx - 3+MAX_BUFF_CNT)%MAX_BUFF_CNT];
					addrList[4] = prParam->u4Duration[(prParam->u4CurrIdx - 2+MAX_BUFF_CNT)%MAX_BUFF_CNT];
					addrList[5] = prParam->u4Duration[(prParam->u4CurrIdx - 3+MAX_BUFF_CNT)%MAX_BUFF_CNT];

					fgClearFrmBuff(prParam, 0, MAX_BUFF_CNT-1);
					prParam->u4FrmBuffY[(prParam->u4CurrIdx - 1+MAX_BUFF_CNT)%MAX_BUFF_CNT] = addrList[0];
					prParam->u4FrmBuffC[(prParam->u4CurrIdx - 1+MAX_BUFF_CNT)%MAX_BUFF_CNT] = addrList[1];
					prParam->u4FrmBuffY[(prParam->u4CurrIdx - 2+MAX_BUFF_CNT)%MAX_BUFF_CNT] = addrList[2];
					prParam->u4FrmBuffC[(prParam->u4CurrIdx - 2+MAX_BUFF_CNT)%MAX_BUFF_CNT] = addrList[3];

					prParam->u4DispIdx = (prParam->u4CurrIdx - 1+MAX_BUFF_CNT)%MAX_BUFF_CNT;
					prParam->u4PrevIdx = (prParam->u4CurrIdx - 2+MAX_BUFF_CNT)%MAX_BUFF_CNT;
				} else {
					if (prParam->u4NeedClrBuff != VDP_CLR_NONE_BUF) {
						fgClearFrmBuff(prParam, 0, MAX_BUFF_CNT - 1);
					}
				}

				prParam->u4VdpStatus = VDP_STATUS_HIDE;
				prParam->u4DeintMode = 0;
				prParam->fgFirstField = TRUE;
				prParam->fgTopFiledFirst = TRUE;
				prParam->fgRepeatFirstField = TRUE;
				prParam->u4Flags = 0;
				FB_PRINT(FB_LOG_LVL_DBG, "VDP", "%s set Hide Status u4DisplayIdx = %d u4CurrentIdx = %d u4ClearIdx = %d addrList[0] = %x\r\n", __func__
						,prParam->u4DispIdx,prParam->u4CurrIdx,prParam->u4ClearIdx,addrList[0]);
				if (u4VdpIdx == VDP_1) {
					if (!fgPmxHalMixPlane(PMX_1, PRIMARY_SURF_PLANE)) {
						vPmxHalMixPlane(PMX_1, PRIMARY_SURF_PLANE);  // enable PRIMARY_SURF_ID layer
						FB_PRINT(FB_LOG_LVL_WARN, "VDP", "vdp mix primary surface plane %d\r\n", PRIMARY_SURF_PLANE);
					}
					vPmxHalNotMixPlaneDelay(PMX_1, PMX_HW_PLANE_1);
					FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s vPmxHalNotMixPlane video plane\r\n", __func__);
				}
				vPmxHalDisableFmt(u4VdpIdx);
				SpinUnlock(SPIN_LOCK_SRC_IOCTL);
			}
			break;
		}
#endif
#endif
	case VIDIOC_QBUF: {
		/*if (!access_ok(VERIFY_READ, pBufIn, sizeof(struct OVERLAY_PARAM))) {
			u4Ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "VDP", "VIDIOC_QBUF return %d\r\n", u4Ret);
			break;
		}*/

		pOverlay = (struct OVERLAY_PARAM *)pBufIn;
		if ((loop_cnt == 1) && vdo_fr_on) {
			FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s fr_follow %d vdo fr %d and hide rear\n", __func__
				, fr_follow, (u32)vdo_fr_on);
			Hide_Video(VDP_2);
			vdo_fr_on = FALSE;
		}

		for (i = 0; i < loop_cnt ; i++) {
			if ((i == VDP_2) && (fr_follow & FR_FOLLOW_VIDEO)) {
				pOverlay  = &rear_follow_with_front_vdp_param;
				memcpy(pOverlay, pBufIn, sizeof(struct OVERLAY_PARAM));
				remap_rear_rect(&pOverlay->rDstRect);
				if (!vdo_fr_on) {
					FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s id from %d to %d vdo fr %d\n", __func__
						, pOverlay->u4Idx, i, (u32)vdo_fr_on);
					pOverlay->u4Flags |= VDP_UPDATE_OVERLAY;
					vdo_fr_on = TRUE;
				}
				pOverlay->u4Idx = i;
			}
			u4VdpIdx = pOverlay->u4Idx;
			prParam = &rData[u4VdpIdx];

			FB_PRINT(FB_LOG_LVL_DBG, "VDP", "VDP_IOControl: VIDIOC_QBUF cmd 0x%x\n", dwCode);
			if (pOverlay->u4Flags & VDP_UPDATE_OVERLAY) {
				FB_PRINT(FB_LOG_LVL_INFO, "VDP", "VIDIOC_QBUF param 0x%x id=%d, src %d,%d, {%d,%d,%d,%d}\r\n"
					, (unsigned int)pBufIn, (int)pOverlay->u4Idx
					, (int)pOverlay->u4SrcWidth, (int)pOverlay->u4SrcHeight
					, (int)pOverlay->rSrcRect.left, (int)pOverlay->rSrcRect.top
					, (int)pOverlay->rSrcRect.right, (int)pOverlay->rSrcRect.bottom);
				FB_PRINT(FB_LOG_LVL_INFO, "VDP", "VIDIOC_QBUF dst {%d,%d,%d,%d} Addr=%x,%x, stutas=%d, flags=%x\r\n"
					, (int)pOverlay->rDstRect.left, (int)pOverlay->rDstRect.top
					, (int)pOverlay->rDstRect.right, (int)pOverlay->rDstRect.bottom
					, (unsigned int)pOverlay->u4PhysicalAddressY
					, (unsigned int)pOverlay->u4PhysicalAddressC
					, (int)pOverlay->u4Status, (unsigned int)pOverlay->u4Flags);
#ifndef __ARM2__
				if (1 == g_vdp_dump) {//In_Buf:420
					vdpInBuf.u4Width = pOverlay->u4SrcWidth;
					vdpInBuf.u4Height = pOverlay->u4SrcHeight;
					vdpInBuf.u4YSize = pOverlay->u4SrcWidth * pOverlay->u4SrcHeight;
					vdpInBuf.u4CSize = vdpInBuf.u4YSize / 2;
					vdpInBuf.u4YAddr[0] = pOverlay->u4PhysicalAddressY;
					vdpInBuf.u4CAddr[0] = pOverlay->u4PhysicalAddressC;
				}
#endif
			}

			if (!_fgInitVdpParam[u4VdpIdx]) {
				fgResetParam(prParam);
				_fgInitVdpParam[u4VdpIdx] = TRUE;

#ifdef	CONFIG_ATC_OS_linux
#ifndef __ARM2__
				/*If Flag is true reset param in En-Queue*/
				fgHideVdp(u4VdpIdx);
				pOverlay->u4Flags |= VDP_UPDATE_OVERLAY;
#endif
#endif
			}

			SpinLock(SPIN_LOCK_SRC_IOCTL);
#if TOPLevel
			if ((_fgVDOTopLevel == FALSE) && (u4VdpIdx == VDP_1) && (pOverlay->device_name == BACKCAR)) {
				_fgVDOTopLevel = TRUE;

				if (pOverlay->u4Flags & VDP_TOP_LEVEL) {
					/* disable android PRIMARY_SURF_ID layer for arm2 backcar*/
					vPmxHalNotMixPlane(PMX_1, PRIMARY_SURF_PLANE);
					FB_PRINT(FB_LOG_LVL_INFO, "VDP", "vdp not mix primary surface plane %d\r\n", PRIMARY_SURF_PLANE);
				}

				/* Other video is show which need back up param and release vdp hw for backcar*/
				if ((prParam->u4SrcType != BACKCAR) && (prParam->u4VdpStatus != VDP_STATUS_HIDE)) {
					FB_PRINT(FB_LOG_LVL_INFO, "VDP", "Backup %d video source for backcar source enter\r\n"
						, prParam->u4SrcType);
					memcpy(prBackuprParam, prParam, sizeof(VDP_PARAM));
					fgResetParam(prParam);
					vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_1);
					FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s vPmxHalNotMixPlane video plane\r\n", __func__);
				} else {
					FB_PRINT(FB_LOG_LVL_INFO, "VDP", "Backcar source and do not need backup other video\r\n");
				}
			}

			if (_fgVDOTopLevel && (u4VdpIdx == VDP_1) && (pOverlay->device_name != BACKCAR)) {
				FB_PRINT(FB_LOG_LVL_DBG, "VDP", "Update backup video param, but not update hw register\r\n");
				ret = fgAddFrmBuff(VDP_BACKUP, pOverlay->u4PhysicalAddressY, pOverlay->u4PhysicalAddressC
					, pOverlay->u4Duration);
				if (ret) {
					fgSetVideoInfo(VDP_BACKUP, pOverlay, TRUE);
				} else {
					u4Ret = -EIO;
				}
			} else {
				ret = fgAddFrmBuff(u4VdpIdx, pOverlay->u4PhysicalAddressY, pOverlay->u4PhysicalAddressC
					, pOverlay->u4Duration);
				if (ret) {
					fgSetVideoInfo(u4VdpIdx, pOverlay, TRUE);
					Show_Video(u4VdpIdx);
				} else {
					u4Ret = -EIO;
				}
			}
#else
			if (pOverlay->u4Flags & VDP_TOP_LEVEL) {
				if (fgPmxHalMixPlane(PMX_1, PRIMARY_SURF_PLANE)) {
					vPmxHalNotMixPlane(PMX_1, PRIMARY_SURF_PLANE);
					FB_PRINT(FB_LOG_LVL_INFO, "VDP", "vdp not mix primary surface plane %d\r\n", PRIMARY_SURF_PLANE);
				}
			} else {
				if (!fgPmxHalMixPlane(PMX_1, PRIMARY_SURF_PLANE)) {
					vPmxHalMixPlane(PMX_1, PRIMARY_SURF_PLANE);
					FB_PRINT(FB_LOG_LVL_INFO, "VDP", "vdp enable mix primary surface plane %d\r\n", PRIMARY_SURF_PLANE);
				}
			}

			ret = fgAddFrmBuff(u4VdpIdx, pOverlay->u4PhysicalAddressY, pOverlay->u4PhysicalAddressC
				, pOverlay->u4Duration);
			if (ret) {
				fgSetVideoInfo(u4VdpIdx, pOverlay, TRUE);
				Show_Video(u4VdpIdx);
			} else {
				u4Ret = -EIO;
			}
#endif
			SpinUnlock(SPIN_LOCK_SRC_IOCTL);
		}
		break;
	}
#ifndef __ARM2__
	case STIOC_SET_VDP_PARAM: {
		/*if (!access_ok(VERIFY_READ, pBufIn, sizeof(struct OVERLAY_PARAM))) {
			u4Ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "VDP", "STIOC_SET_VDP_PARAM return %d\r\n", u4Ret);
			break;
		}*/

		pOverlay = (struct OVERLAY_PARAM *)pBufIn;
		for (i = 0; i < loop_cnt ; i++) {
			if (i == VDP_2) {
				pOverlay  = &rear_follow_with_front_vdp_param;
				memcpy(pOverlay, pBufIn, sizeof(struct OVERLAY_PARAM));
				remap_rear_rect(&pOverlay->rDstRect);
			}
			SpinLock(SPIN_LOCK_SRC_IOCTL);
			if (fr_follow & FR_FOLLOW_VIDEO) {
				pOverlay->u4Idx = i;
			}
			u4VdpIdx = pOverlay->u4Idx;
			FB_PRINT(FB_LOG_LVL_DBG, "VDP", "VDP_IOControl: STIOC_SET_VDP_PARAM cmd 0x%x\n", dwCode);
			FB_PRINT(FB_LOG_LVL_INFO, "VDP", "STIOC_SET_VDP_PARAM param 0x%x id=%d, src %d,%d, {%d,%d,%d,%d}\r\n"
				, (unsigned int)pBufIn, (int)pOverlay->u4Idx
				, (int)pOverlay->u4SrcWidth, (int)pOverlay->u4SrcHeight
				, (int)pOverlay->rSrcRect.left, (int)pOverlay->rSrcRect.top
				, (int)pOverlay->rSrcRect.right, (int)pOverlay->rSrcRect.bottom);
			FB_PRINT(FB_LOG_LVL_INFO, "VDP", "STIOC_SET_VDP_PARAM dst {%d,%d,%d,%d} Addr=%x,%x, stutas=%d, flags=%x\r\n"
				, (int)pOverlay->rDstRect.left, (int)pOverlay->rDstRect.top
				, (int)pOverlay->rDstRect.right, (int)pOverlay->rDstRect.bottom
				, (unsigned int)pOverlay->u4PhysicalAddressY
				, (unsigned int)pOverlay->u4PhysicalAddressC
				, (int)pOverlay->u4Status, (unsigned int)pOverlay->u4Flags);
			/* Other video is show which need back up param and release vdp hw for backcar*/
#if TOPLevel
			if (!_fgVDOTopLevel && (u4VdpIdx == VDP_1) && (pOverlay->device_name == BACKCAR)) {
				_fgVDOTopLevel = TRUE;
				prParam = &rData[u4VdpIdx];

				if ((prParam->u4SrcType != BACKCAR) && (prParam->u4VdpStatus != VDP_STATUS_HIDE)) {
					/* Other video is show which need back up param and release vdp hw for backcar*/
					FB_PRINT(FB_LOG_LVL_INFO, "VDP", "STIOC_SET_VDP_PARAM: backup %d video for backcar enter\r\n"
						, prParam->u4SrcType);
					memcpy(prBackuprParam, prParam, sizeof(VDP_PARAM));
					fgResetParam(prParam);
					vPmxHalNotMixPlane(PMX_1, PMX_HW_PLANE_1);
				} else {
					FB_PRINT(FB_LOG_LVL_INFO, "VDP", "STIOC_SET_VDP_PARAM: do not need backup other video\r\n");
				}
			}

			if (_fgVDOTopLevel && (u4VdpIdx == VDP_1) && (pOverlay->device_name != BACKCAR)) {
				FB_PRINT(FB_LOG_LVL_DBG, "VDP", "STIOC_SET_VDP_PARAM: update backup video param\r\n");
				u4VdpIdx = VDP_BACKUP;
			} else {
				u4VdpIdx = pOverlay->u4Idx;
			}
#else
			prParam = &rData[u4VdpIdx];

			if ((pOverlay->device_name == BACKCAR) && (pOverlay->device_name != prParam->u4SrcType)) {
				Hide_Video(u4VdpIdx);
			}
#endif
			fgSetVideoInfo(u4VdpIdx, pOverlay, TRUE);
			SpinUnlock(SPIN_LOCK_SRC_IOCTL);
		}
		break;
	}

	case STIOC_SET_FMT_BLACK: {
		struct FMT_BG_PARAM *pFmtBG = NULL;

		/*if (!access_ok(VERIFY_READ, pBufIn, sizeof(struct FMT_BG_PARAM))) {
			u4Ret  = -EINVAL;
			FB_PRINT(FB_LOG_LVL_ERR, "VDP", "STIOC_SET_FMT_BLACK return %d\r\n", u4Ret);
			break;
		}*/
		pFmtBG = (struct FMT_BG_PARAM *)pBufIn;
		prParam = &rData[pFmtBG->u4Idx];
		SpinLock(SPIN_LOCK_SRC_IOCTL);
		if (pFmtBG->fgEnable) {
			vPmxHalSetBg(pFmtBG->u4Idx, pFmtBG->u4Color);
			vPmxHalDisableFmt(pFmtBG->u4Idx);
			prParam->u4VdpStatus = VDP_STATUS_BLACK;
		} else {
			u32 u4Idx = prParam->u4CurrIdx;

			vPmxHalEnableFmt(pFmtBG->u4Idx);
			if (prParam->u4DeintMode >= VDP_DI_MA4F_MODE) {
				prParam->u4NeedClrBuff |= VDP_CLR_ONCE_BUF;
				/* cur buffer is ready to write, cur - 1 is ready to show and cur - 2 is unsused buffer*/
				prParam->u4ClearIdx = (u4Idx - 2 + MAX_BUFF_CNT) % MAX_BUFF_CNT;
				prParam->u4PrevIdx = (u4Idx - 1 + MAX_BUFF_CNT) % MAX_BUFF_CNT;
				prParam->u4DispIdx = (u4Idx - 1 + MAX_BUFF_CNT) % MAX_BUFF_CNT;
				prParam->fgFirstField = TRUE;
				prParam->u4NeedShowBuff = 0;
				prParam->u4VdpStatus = VDP_STATUS_PREPARE;
			} else {
				prParam->u4VdpStatus = VDP_STATUS_SHOW;
			}
		}
		FB_PRINT(FB_LOG_LVL_INFO, "VDP", "VDP_IOControl: set black %d status %d and video layer %d\n", pFmtBG->fgEnable
			, prParam->u4VdpStatus, fgPmxHalMixPlane(0, PMX_HW_PLANE_1));
		SpinUnlock(SPIN_LOCK_SRC_IOCTL);
		break;
	}
	case STIOC_SET_COLOR_RANGE: {
		__u32 *format = NULL;
		__u32 u4Idx = 0;

		if (!pBufIn) {
			FB_PRINT(FB_LOG_LVL_ERR, "VDP", "VDP_IOControl: STIOC_SET_COLOR pBufIn is NULL\n");
			break;
		}
		format = (__u32 *)pBufIn;

		FB_PRINT(FB_LOG_LVL_INFO, "VDP", "VDP_IOControl: set format 0x%x\n", *format);
		switch (*format) {
		case (OVERLAY_FLAG_YUV_BT601):
				for (u4Idx = PMX_1; u4Idx <= PMX_2; u4Idx++) {
					vPmxHalSet709To601(u4Idx, TRUE, TRUE);
				}
				break;
		case (OVERLAY_FLAG_YUV_BT709):
				for (u4Idx = PMX_1; u4Idx <= PMX_2; u4Idx++) {
					vPmxHalSet709To601(u4Idx, FALSE, FALSE);
				}
				break;
		default:
				FB_PRINT(FB_LOG_LVL_INFO, "VDP", " bug here, please check it!!!!!, set format is 0x%x\n", *format);
		}
		break;
	}
	case STIOC_SET_COLOR_ENCODING: {
		__u32 *format = NULL;
		__u32 u4Idx = 0;

		if (!pBufIn) {
			FB_PRINT(FB_LOG_LVL_ERR, "VDP", "VDP_IOControl: STIOC_SET_COLOR pBufIn is NULL\n");
			break;
		}
		format = (__u32 *)pBufIn;

		FB_PRINT(FB_LOG_LVL_INFO, "VDP", "VDP_IOControl: set format 0x%x\n", *format);
		switch (*format) {
		case (OVERLAY_FLAG_YUV_NARROW):
				for (u4Idx = PMX_1; u4Idx <= PMX_2; u4Idx++) {
					vPmxHalSetFullRange(u4Idx, FALSE, FALSE);
				}
				break;
		case (OVERLAY_FLAG_YUV_WIDE):
				for (u4Idx = PMX_1; u4Idx <= PMX_2; u4Idx++) {
					vPmxHalSetFullRange(u4Idx, TRUE, TRUE);
				}
				break;
		default:
				FB_PRINT(FB_LOG_LVL_INFO, "VDP", " bug here, please check it!!!!!, set format is 0x%x\n", *format);
		}
		break;
	}
	case STIOC_GET_PHY_ACTIVE: {
		struct SCREEN_AREA *phy_resolution = NULL;
		if (!pBufOut) {
			FB_PRINT(FB_LOG_LVL_ERR, "VDP", "VDP_IOControl: STIOC_GET_PHY_ACTIVE pBufOut is NULL\n");
			break;
		}

		phy_resolution = (struct SCREEN_AREA *)pBufOut;
		phy_resolution->height = 91;
		phy_resolution->width = 152;
		FB_PRINT(FB_LOG_LVL_INFO, "VDP", "VDP_IOControl: get phy_resolution: %dx%d\n", phy_resolution->width, phy_resolution->height);

		break;
	}
#endif
	default:
		FB_PRINT(FB_LOG_LVL_DBG, "VDP", "VDP_IOControl: Unsupported cmd 0x%x\n", dwCode);
		break;
	}

	return u4Ret;
}
EXPORT_SYMBOL(VDP_IOControl);




