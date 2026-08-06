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
#include <media/atc/ac823x/pmx_hal.h>
#include <media/atc/display.h>
#include <media/atc/display_inc.h>
#include <media/atc/cp.h>
#include "windows.h"
#include "imgresz_hal_if.h"
#include "drv_imgresz.h"
#else
#include "x_types.h"
#include "vdp_mdd.h"
#include "display_inc.h"
#include "pmx_hal.h"
#include "display.h"
#include "errno-base.h"
#endif
#include "drv_pmx.h"
#include "scl_hal.h"
#include "vdp_hal.h"
#include "fbm.h"
#include "Surface.h"
#include "vdp.h"
#include "log.h"
#include "bringup.h"

#define ENABLE_MM_DEINTERLACE     0
#define ENABLE_PD_DEINTERLACE     0
#define ENABLE_HD_DEINTERLACE     0
#define LINUX_FUN                 0

#define IMG_RESZ_DST_BUFF_WIDTH   (_u4LCDWidth)         /* Max panel width*/
#define IMG_RESZ_DST_BUFF_HEIGHT  (_u4LCDHeight)        /* Max panel height * 2 for HD deinterlace source*/
#define IMG_RESZ_SD_BUFF_WIDTH    (720)                 /* Max panel width*/
#define IMG_RESZ_SD_BUFF_HEIGHT   (480)                 /* Max panel height * 2 for HD deinterlace source*/

#define VDP_BACKUP 2    /* Back up front video*/

#define VDP_ASSERT(argu) ASSERT(argu)

#define SIZE_ALIGN(x, n)  ((x + n - 1) / n * n)

#define ROUND_UP_COUNT(Count, Pow2)             (((Count)+(Pow2)-1) & (~(((LONG)(Pow2))-1)))

#define VALID_BUFF_COUNT(Cur, Disp, Count)      (((Cur - Disp + MAX_BUFF_CNT) % MAX_BUFF_CNT) > Count)

#define INVALID_BUFF_COUNT(Cur, Disp)           (((Disp + 1) % MAX_BUFF_CNT) == Cur)

#define CALCULATE_VSYNC_CNT(Duration, Vsync)    ((Duration + Vsync / 2) / Vsync)

#define MONITOR_QBUF_TIME 0

#define SPIN_LOCK_SRC_ISRF      0
#define SPIN_LOCK_SRC_ISRR      1
#define SPIN_LOCK_SRC_IOCTL     2
#define SPIN_LOCK_SRC_RESIZE    3

#ifndef __ARM2__
static spinlock_t _vdp_lock;
#endif
static unsigned long _vdp_flags;
static __u32 u4VdpLockCnt;
static bool fgLockInit = FALSE;
static bool _fgInitVdpParam[2] = {FALSE, FALSE};
static bool _fgBypassFstVysnc[2] = {TRUE, TRUE};
static __u32 _u4VdpOutWidth;
static __u32 _u4VdpOutHeight;
VDP_PARAM rData[3];

static PMTKSurface pImgDstSurf[2][IMG_RESZ_BUFF_SIZE] = {
        {NULL, NULL, NULL},
        {NULL, NULL, NULL},
};

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
__u32 _u4VIDEO_DBG_LVL = VDO_LOG_LVL_INFO;
__u8 *_pcVideoLogLevel[] = {
        "[VDO OFF]",
        "[VDO ERR]",
        "[VDO WARN]",
        "[VDO INFO]",
        "[VDO HAL]",
        "[VDO DBG]",
        "[VDO IRQ]",
        "[VDO REGRW]",
};

void SpinLock(__u32 u4LockSrc)
{
#ifndef __ARM2__
        spin_lock_irqsave(&_vdp_lock, _vdp_flags);
        u4VdpLockCnt++;

        if ((u4LockSrc == SPIN_LOCK_SRC_IOCTL) || (u4LockSrc == SPIN_LOCK_SRC_RESIZE)) {
                VDO_LOG(VDO_LOG_LVL_IRQ, "VDP Spinlock SRC =%d, count=%d, flag = %d\r\n"
                        , (int)u4LockSrc, (int)u4VdpLockCnt, (int)_vdp_flags);
        } else if (u4VdpLockCnt > 1) {
                VDO_LOG(VDO_LOG_LVL_IRQ, "VDP Spinlock 111 SRC =%d, count=%d, flag = %d\r\n"
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
                VDO_LOG(VDO_LOG_LVL_IRQ, "VDP SpinUnlock SRC =%d, count=%d, flag = %d\r\n"
                        , (int)u4LockSrc, (int)u4VdpLockCnt, (int)_vdp_flags);
        } else if (u4VdpLockCnt) {
                VDO_LOG(VDO_LOG_LVL_IRQ, "VDP SpinUnlock 111 SRC =%d, count=%d, flag = %d\r\n"
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

bool fgClearFrmBuff(VDP_PARAM *prParam, __u32 u4StartIdx, __u32 u4EndIdx)
{
        __u32 u4Idx = 0;

        if (u4StartIdx > u4EndIdx) {
                return FALSE;
        }

        for (u4Idx = u4StartIdx; u4Idx <= u4EndIdx; u4Idx++) {
                if (prParam->u4FrmBuffY[u4Idx] & prParam->u4FrmBuffC[u4Idx]) {
                        /* clear buffer*/
                        /*VDO_LOG(VDO_LOG_LVL_INFO, "%s Y/C 0x%x 0x%x\r\n", __func__
                                , prParam->u4FrmBuffY[u4Idx], prParam->u4FrmBuffC[u4Idx]);*/
                        prParam->u4FrmBuffY[u4Idx] = 0;
                        prParam->u4FrmBuffC[u4Idx] = 0;
                        prParam->u4Duration[u4Idx] = 0;
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
                VDO_LOG(VDO_LOG_LVL_ERR, "atc_dispc_register_isr return EINVAL\r\n");

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
                VDO_LOG(VDO_LOG_LVL_ERR, "atc_dispc_register_isr no free isr %d\r\n", i);

                return -EBUSY;
        }
        VDO_LOG(VDO_LOG_LVL_INFO, "atc_dispc_register_isr %d\r\n", i);

        return 0;
}
EXPORT_SYMBOL(atc_dispc_register_isr);

int atc_dispc_unregister_isr(atc_dispc_isr_t isr, void *arg)
{
        u32 i = 0;

        if ((isr == NULL) || (arg == NULL)) {
                VDO_LOG(VDO_LOG_LVL_ERR, "atc_dispc_unregister_isr return EINVAL\r\n");

                return -EINVAL;
        }

        for (i = 0; i < MAX_ISR_CNT; i++) {
                if ((isr_data[i].isr == isr) && (isr_data[i].arg == arg)) {
                        isr_data[i].isr = NULL;
                        isr_data[i].arg = NULL;
                        register_isr[i] = FALSE;
                        VDO_LOG(VDO_LOG_LVL_INFO, "atc_dispc_unregister_isr %d\r\n", i);

                        return 0;
                }
        }
        VDO_LOG(VDO_LOG_LVL_ERR, "atc_dispc_unregister_isr no match isr %d\r\n", i);

        return -EBUSY;
}
EXPORT_SYMBOL(atc_dispc_unregister_isr);
#endif

void vDumpFrameBuffer(VDP_PARAM *prParam, bool fgFstField, __u32 *u4FBufferY)
{
        VDO_LOG(VDO_LOG_LVL_TRACE, "[VDP]vDumpFrameBuffer: first field %d, id %d %d %d, buffer %x, %x, %x, %x\r\n"
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

#ifdef MONITOR_VDO_FPS
        if (vdo_fps_en[u4VdpIdx]) {
                vdo_flip_cnt[u4VdpIdx]++;
        }
#endif

        if (prParam->u4DeintMode < VDP_DI_MA4F_MODE) {
                if (prParam->u4FrmBuffY[u4Disp] && prParam->u4FrmBuffC[u4Disp]) {
                        vVdpHalSetYBufPtr(u4VdpIdx, prParam->u4FrmBuffY[u4Disp], prParam->u4FrmBuffC[u4Disp]);
                        /*VDO_LOG(VDO_LOG_LVL_INFO, "%s: hw id %d, disp %d, cur %d, address %x, %x duration %d\r\n"
                                , __func__, u4VdpIdx , u4Disp, prParam->u4CurrIdx, prParam->u4FrmBuffY[u4Disp]
                                , prParam->u4FrmBuffC[u4Disp], prParam->u4Duration[u4Disp]);*/
                } else {
                        VDO_LOG(VDO_LOG_LVL_ERR, "[VDP]fgUpdateHwAddr: error id %d, disp %d, address %x, %x\r\n"
                                , (int)u4VdpIdx, (int)u4Disp, (unsigned int)prParam->u4FrmBuffY[u4Disp]
                                , (unsigned int)prParam->u4FrmBuffC[u4Disp]);
                        return FALSE;
                }
        } else {
                if (prParam->fgFirstField) {
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
                }

                if (u4PhyAddrY[0] && u4PhyAddrY[1] && u4PhyAddrY[2] && u4PhyAddrY[3] &&
                    u4PhyAddrC[0] && u4PhyAddrC[1] && u4PhyAddrC[2] && u4PhyAddrC[3]) {
                        /*vDumpFrameBuffer(prParam, prParam->fgFirstField, u4PhyAddrY);*/
                        vVdpHalSetDeintWXYZ(u4VdpIdx, u4PhyAddrY, u4PhyAddrC);
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
        
        if (u4VdpIdx > VDP_2) {
                VDO_LOG(VDO_LOG_LVL_ERR, "vdpisr not support hw id %d\r\n", u4VdpIdx);
                return;
        }

#ifndef __ARM2__
        if (!fgLockInit) {
                spin_lock_init(&_vdp_lock);
                fgLockInit = TRUE;
                VDO_LOG(VDO_LOG_LVL_DBG, "vdpisr spin lock init %x\r\n", (unsigned int)&_vdp_lock);
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
        }

        if ((prParam->u4NeedClrBuff & VDP_CLR_REGU_BUF) && (prParam->u4ClearIdx != 0xFF))
        {
                fgClearFrmBuff(prParam, prParam->u4ClearIdx, prParam->u4ClearIdx);
                prParam->u4ClearIdx = 0xFF;
        }

        if (prParam->u4DeintMode == VDP_DI_MA4F_MODE) {
                switch (prParam->u4VdpStatus) {
                case VDP_STATUS_PREPARE:
                case VDP_STATUS_SEEK: {
                        if (VALID_BUFF_COUNT(prParam->u4CurrIdx, prParam->u4DispIdx, 1)) {
                                FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s hw %d ready to show %d, %d\r\n", __func__,
                                        u4VdpIdx, (int)prParam->u4DispIdx, (int)prParam->u4CurrIdx);
                                prParam->u4VdpStatus = VDP_STATUS_SHOW;
                                _fgBypassFstVysnc[u4VdpIdx] = TRUE;
                        } else {
                                FB_PRINT(FB_LOG_LVL_DBG, "VDP", "%s hw %d VDP_STATUS_PREPARE %d, %d\r\n", __func__,
                                        u4VdpIdx, (int)prParam->u4DispIdx, (int)prParam->u4CurrIdx);
                                break;
                        }
                }

                case VDP_STATUS_SHOW: {
                        if (INVALID_BUFF_COUNT(prParam->u4CurrIdx, prParam->u4DispIdx) &&
                            (prParam->fgFirstField == FALSE)) {
                                /* buffer not enough to show*/
                                /*FB_PRINT(FB_LOG_LVL_INFO, "VDP", "vVdpIsr no buffer %d, %d topfieldfirst %d\r\n"
                                        , (int)prParam->u4DispIdx, (int)prParam->u4CurrIdx
                                        , (int)prParam->fgTopFiledFirst);*/
                        } else {
                                if (_fgBypassFstVysnc[u4VdpIdx]) {
                                        /* First vsync not update address and motion and comb is invalid*/
                                        _fgBypassFstVysnc[u4VdpIdx] = FALSE;
                                }

                                u4DispIdx = prParam->u4DispIdx;

                                if ((prParam->u4VsyncCnt[u4DispIdx] ==
                                        CALCULATE_VSYNC_CNT(prParam->u4Duration[u4DispIdx], VSYNC_PER_FIELD)) ||
                                        (prParam->u4VsyncCnt[u4DispIdx] ==
                                        CALCULATE_VSYNC_CNT(prParam->u4Duration[u4DispIdx], VSYNC_PER_FRAME))) {
                                        fgUpdateHwAddr(u4VdpIdx);
                                        vVdpHalSetFieldInfo(u4VdpIdx, prParam->fgTopFiledFirst, prParam->fgFirstField);
                                        prParam->fgFirstField = !prParam->fgFirstField;

                                        if (!vPmxHalGetFmtEn(u4VdpIdx)) {
                                                /* Enable FMT to show video*/
                                                vPmxHalEnableFmt(u4VdpIdx);
                                                FB_PRINT(FB_LOG_LVL_INFO, "VDP", "%s hw %d enable fmt\r\n",
                                                        __func__, u4VdpIdx);
                                        }

                                        if ((u4VdpIdx == VDP_1) && !fgPmxHalMixPlane(PMX_1, PMX_HW_PLANE_1)) {
                                                vPmxMixPlane(PMX_1, PMX_HW_PLANE_1);
                                                FB_PRINT(FB_LOG_LVL_INFO, "VDP", "enable video layer in isr\r\n");
                                        }

                                }

                                /*FB_PRINT(FB_LOG_LVL_IRQ, "VDP", "vVdpIsr count id %d, _gVsyncNs %d, disp %d %d %d\r\n"
                                        , (int)u4VdpIdx, (int)prParam->u4VsyncCnt[u4DispIdx], (int)prParam->u4PrevIdx
                                        , (int)prParam->u4DispIdx, (int)prParam->u4CurrIdx);*/
                                prParam->u4VsyncCnt[u4DispIdx]--;

                                if (prParam->u4VsyncCnt[u4DispIdx] <= 0) {
                                        vUpdateDispIdx(u4VdpIdx);
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
        }

        SpinUnlock(u4VdpIdx);
        vVdpHalIsr(u4VdpIdx);
}

void vDumpVdpParam(int index)
{
        VDP_PARAM *prParam = &rData[index];

        VDO_LOG(VDO_LOG_LVL_DBG, "##############VDP Param Data Start Vdp %d##############\r\n", index);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4SrcWidth: %d\r\n", (int)prParam->u4SrcWidth);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4SrcHeight: %d\r\n", (int)prParam->u4SrcHeight);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->rSrcRect: %d %d %d %d\r\n", (int)prParam->rSrcRect.left,
                (int)prParam->rSrcRect.top, (int)prParam->rSrcRect.right, (int)prParam->rSrcRect.bottom);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->rDstRect: %d %d %d %d\r\n", (int)prParam->rDstRect.left,
                (int)prParam->rDstRect.top, (int)prParam->rDstRect.right, (int)prParam->rDstRect.bottom);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4SrcType: %d\r\n", (int)prParam->u4SrcType);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4Flags: 0x%x\r\n", (unsigned int)prParam->u4Flags);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4FrmBuffY: 0x%x\r\n"
                , (unsigned int)prParam->u4FrmBuffY[prParam->u4DispIdx]);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4FrmBuffC: 0x%x\r\n"
                , (unsigned int)prParam->u4FrmBuffC[prParam->u4DispIdx]);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4Duration: %d\r\n", (int)prParam->u4Duration[prParam->u4DispIdx]);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4VsyncCnt: %d\r\n", (int)prParam->u4VsyncCnt[prParam->u4DispIdx]);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4VdpStatus: %d\r\n", (int)prParam->u4VdpStatus);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4VdpMode: 0x%x\r\n", (unsigned int)prParam->u4VdpMode);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4DeintMode: %d\r\n", (int)prParam->u4DeintMode);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4PullDownMode: %d\r\n", (int)prParam->u4PullDownMode);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4CurrIdx: %d\r\n", (int)prParam->u4CurrIdx);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4DispIdx: %d\r\n", (int)prParam->u4DispIdx);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4PrevIdx: %d\r\n", (int)prParam->u4PrevIdx);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4ClearIdx: %d\r\n", (int)prParam->u4ClearIdx);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->fgNeedClrBuff: %d\r\n", (int)prParam->u4NeedClrBuff);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->fgFirstField: %d\r\n", (int)prParam->fgFirstField);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->fgNeedResizer: %d\r\n", (int)prParam->fgNeedResizer);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->fgProgSrc: %d\r\n", (int)prParam->fgProgSrc);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->fgTopFiledFirst: %d\r\n", (int)prParam->fgTopFiledFirst);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->fgRepeatFirstField: %d\r\n", (int)prParam->fgRepeatFirstField);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->fgProgSeq: %d\r\n", (int)prParam->fgProgSeq);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->fgPullDownFlagValid: %d\r\n", (int)prParam->fgPullDownFlagValid);
        VDO_LOG(VDO_LOG_LVL_DBG, "##############VDP Param Data End##############\r\n");
}

void vDumpParam(VDP_PARAM *prParam)
{
        VDO_LOG(VDO_LOG_LVL_DBG, "##############VDP Param Data Start Vdp 0x%x##############\r\n"
                , (unsigned int)prParam);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4SrcWidth: %d\r\n", (int)prParam->u4SrcWidth);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4SrcHeight: %d\r\n", (int)prParam->u4SrcHeight);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->rSrcRect: %d %d %d %d\r\n", (int)prParam->rSrcRect.left,
                (int)prParam->rSrcRect.top, (int)prParam->rSrcRect.right, (int)prParam->rSrcRect.bottom);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->rDstRect: %d %d %d %d\r\n", (int)prParam->rDstRect.left,
                (int)prParam->rDstRect.top, (int)prParam->rDstRect.right, (int)prParam->rDstRect.bottom);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4SrcType: %d\r\n", (int)prParam->u4SrcType);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4Flags: 0x%x\r\n", (unsigned int)prParam->u4Flags);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4FrmBuffY: 0x%x\r\n"
                , (unsigned int)prParam->u4FrmBuffY[prParam->u4DispIdx]);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4FrmBuffC: 0x%x\r\n"
                , (unsigned int)prParam->u4FrmBuffC[prParam->u4DispIdx]);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4Duration: %d\r\n", (int)prParam->u4Duration[prParam->u4DispIdx]);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4VsyncCnt: %d\r\n", (int)prParam->u4VsyncCnt[prParam->u4DispIdx]);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4VdpStatus: %d\r\n", (int)prParam->u4VdpStatus);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4VdpMode: 0x%x\r\n", (unsigned int)prParam->u4VdpMode);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4DeintMode: %d\r\n", (int)prParam->u4DeintMode);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4PullDownMode: %d\r\n", (int)prParam->u4PullDownMode);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4CurrIdx: %d\r\n", (int)prParam->u4CurrIdx);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4DispIdx: %d\r\n", (int)prParam->u4DispIdx);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4PrevIdx: %d\r\n", (int)prParam->u4PrevIdx);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->u4ClearIdx: %d\r\n", (int)prParam->u4ClearIdx);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->fgNeedClrBuff: %d\r\n", (int)prParam->u4NeedClrBuff);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->fgFirstField: %d\r\n", (int)prParam->fgFirstField);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->fgNeedResizer: %d\r\n", (int)prParam->fgNeedResizer);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->fgProgSrc: %d\r\n", (int)prParam->fgProgSrc);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->fgTopFiledFirst: %d\r\n", (int)prParam->fgTopFiledFirst);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->fgRepeatFirstField: %d\r\n", (int)prParam->fgRepeatFirstField);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->fgProgSeq: %d\r\n", (int)prParam->fgProgSeq);
        VDO_LOG(VDO_LOG_LVL_DBG, "prParam->fgPullDownFlagValid: %d\r\n", (int)prParam->fgPullDownFlagValid);
        VDO_LOG(VDO_LOG_LVL_DBG, "##############VDP Param Data End##############\r\n");
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
                VDO_LOG(VDO_LOG_LVL_ERR, "[VDP]fgAddFrmBuff: error buffer is not 128 bytes alignment %x, %x, %d\r\n",
                        (unsigned int)u4YAddr, (unsigned int)u4CAddr, (int)u4Duration);
                return FALSE;
        }

        if ((u4YAddr == prParam->u4FrmBuffY[(u4Idx - 1 + MAX_BUFF_CNT) % MAX_BUFF_CNT]) ||
            (u4CAddr == prParam->u4FrmBuffY[(u4Idx - 1 + MAX_BUFF_CNT) % MAX_BUFF_CNT])) {
                VDO_LOG(VDO_LOG_LVL_ERR, "[VDP]fgAddFrmBuff: error buffer addr is the same as before %x, %x, %d\r\n",
                        (unsigned int)u4YAddr, (unsigned int)u4CAddr, (int)u4Duration);
                return FALSE;
        }

        prParam->u4FrmBuffY[u4Idx] = u4YAddr;
        prParam->u4FrmBuffC[u4Idx] = u4CAddr;
        prParam->u4Duration[u4Idx] = u4Duration ? u4Duration : VSYNC_PER_FRAME;
        prParam->u4VsyncCnt[u4Idx] = CALCULATE_VSYNC_CNT(prParam->u4Duration[u4Idx], VSYNC_PER_FIELD);

#if MONITOR_QBUF_TIME
        if ((u4TmpCnt == 0) && (fgStartCnt == FALSE)) {
                do_gettimeofday(&start_t[u4VdpIdx]);
                fgStartCnt = TRUE;
        }

        if (u4TmpCnt == 30) {
                do_gettimeofday(&end_t);
                usec = (end_t.tv_sec - start_t[u4VdpIdx].tv_sec) * 1000000
                        + (end_t.tv_usec - start_t[u4VdpIdx].tv_usec);
                VDO_LOG(VDO_LOG_LVL_INFO, "[VDP]fgAddFrmBuff: 30 FPS id = %d usec = %d\r\n", (int)u4VdpIdx, (int)usec);
                do_gettimeofday(&start_t[u4VdpIdx]);
                u4TmpCnt = 0;
        } else {
                u4TmpCnt++;
        }
#endif

        if ((prParam->u4DeintMode >= VDP_DI_MA4F_MODE) && (prParam->u4VsyncCnt[u4Idx] <= 0)) {
                VDO_LOG(VDO_LOG_LVL_ERR, "[VDP]fgAddFrmBuff: Duration error idx %d, %x, %x, %d, %d, DI %d\r\n",
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
        prBefore->bottom = (prBefore->bottom * rear_height / _u4LCDHeight) -
                g_rFBConfig.rCVBSMargin.u4BottomRightYMargin;

        VDO_LOG(VDO_LOG_LVL_DBG, "%s remap rear rect %d %d %d %d\r\n", __func__
                , g_rFBConfig.rCVBSMargin.u4TopLeftXMargin, g_rFBConfig.rCVBSMargin.u4TopLeftYMargin
                , g_rFBConfig.rCVBSMargin.u4BottomRightXMargin, g_rFBConfig.rCVBSMargin.u4BottomRightYMargin);

        return TRUE;
}

bool is_rear_vdp_busy(void)
{
        bool ret = FALSE;

        if (!(fr_follow & FR_FOLLOW_VIDEO) && vPmxHalGetFmtEn(VDP_2)) {
                VDO_LOG(VDO_LOG_LVL_INFO, "%s vdp rear busy\r\n", __func__);
                ret = TRUE;
        } else {
                VDO_LOG(VDO_LOG_LVL_INFO, "%s vdp rear idle video mirror %d %d hw %d\r\n", __func__
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

        prParam->u4SrcWidth = pOverlay->u4SrcWidth;
        prParam->u4SrcHeight = pOverlay->u4SrcHeight;

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
                        VDO_LOG(VDO_LOG_LVL_WARN, "Rear param error and reconfig to SD %d, %d, %d, %d vdp mode %d\r\n"
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
        
        if (prParam->u4SrcType == USB) {
                prParam->fgTopFiledFirst = pOverlay->fgTopFiledFirst;
        } else {
        #if 1
                if(prParam->u4SrcType == 0x11)
                        prParam->fgTopFiledFirst = TRUE;
                else
                        prParam->fgTopFiledFirst = FALSE;
        #else
                prParam->fgTopFiledFirst = TRUE;
                if(prParam->rSrcRect.top & 1) {
                        prParam->fgFirstField = FALSE;
                }
                else {
                        prParam->fgFirstField = TRUE;
                }
        #endif
        }

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

        case RES_480P_800:
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
                break;

        default:
                *prWidth = 720;
                *prHeight = 480;
                return FALSE;
        }

        return TRUE;
}

void vSclMasterSwitchSrc(__u32 u4VdpMode, __u32 u4LcdWidth, __u32 u4LcdHeight)
{
#if MASTER_MODE_ENABLE
        __u32 u4SclInput = 0, u4SclOutput = 0;

        switch (u4VdpMode) {
        case RES_480P:
                u4SclInput = SCL_IN_480P;
                break;

        case RES_480P_800:
        case RES_480P_800_50HZ:
                u4SclInput = SCL_IN_480P_800;
                break;

        case RES_600P_800:
        case RES_600P_800_50HZ:
                u4SclInput = SCL_IN_600P_800;
                break;

        case RES_600P_1024:
        case RES_600P_1024_50HZ:
                u4SclInput = SCL_IN_600P_1024;
                break;

        case RES_720P_1280:
        case RES_720P_1280_50HZ:
                u4SclInput = SCL_IN_720P_1280;
                break;

        case RES_800P_1280:
        case RES_800P_1280_50HZ:
                u4SclInput = SCL_IN_800P_1280;
                break;

        case RES_768P_1024:
                u4SclInput = SCL_IN_768P_1024;
                break;
        }

        if ((u4LcdWidth == 800) && (u4LcdHeight == 480)) {
                u4SclOutput = SCL_OUT_800_480;
        } else if ((u4LcdWidth == 1024) && (u4LcdHeight == 600)) {
                u4SclOutput = SCL_OUT_1024_600;
        } else if ((u4LcdWidth == 1280) && (u4LcdHeight == 720)) {
                u4SclOutput = SCL_OUT_1280_720;
        } else if ((u4LcdWidth == 1280) && (u4LcdHeight == 800)) {
                u4SclOutput = SCL_OUT_1280_800;
        } else if ((u4LcdWidth == 1024) && (u4LcdHeight == 768)) {
                u4SclOutput = SCL_OUT_1024_768;
        }

        VDO_LOG(VDO_LOG_LVL_DBG, "vSclMasterSwitchSrc: u4SclInput %d, u4SclOutput %d\r\n"
                , (int)u4SclInput, (int)u4SclOutput);
        vSclHalSetMasterSrc(u4SclInput, u4SclOutput);
#endif
}

static bool fgConfigVdpHw(__u32 u4VdpIdx)
{
        VDP_PARAM *prParam = &rData[u4VdpIdx];
        __u32 u4CurIdx;
        PRECT pRect = &prParam->rDstRect;
        
        VDO_LOG(VDO_LOG_LVL_DBG, "enter %s\r\n", __func__);
        vDumpVdpParam(u4VdpIdx);
               
        prParam->u4DispIdx = (prParam->u4CurrIdx - 1 + MAX_BUFF_CNT) % MAX_BUFF_CNT;

        vVdpHalSetOutRegion(u4VdpIdx, pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top);

        vVdpHalSetSrcSize(u4VdpIdx, prParam->u4SrcWidth, prParam->u4SrcHeight);
        pRect = &prParam->rSrcRect;
        vVdpHalSetSrcRegion(u4VdpIdx, pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top);
        vVdpHalSetYuv422(u4VdpIdx, FALSE); /* default is YUV420 mode*/
        vVdpHalSetScanLine(u4VdpIdx, prParam->u4Flags & VDP_SCANLINE_MODE);
        //vPmxHalDispFmtHFilter(u4VdpIdx, 3, 2);
        PMX_HalSetMode(u4VdpIdx, RES_600P_1024); //now ,fixed in panel size:1024*600
        vVdpHalSetMode(u4VdpIdx, RES_600P_1024);
        vPmxHalSetFullRange(u4VdpIdx, TRUE, TRUE); /* video normal range and convert to full range*/
        vVdpHalSetFifo(u4VdpIdx);

        if (prParam->u4VdpStatus == VDP_STATUS_SHOW) {
                u4CurIdx = prParam->u4CurrIdx;
                /* Updateoverlay more than once, and need clear unused buffer if sd source*/
                if (!prParam->fgNeedResizer) {
                        prParam->u4NeedClrBuff |= VDP_CLR_ONCE_BUF;
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

        if (prParam->u4DeintMode < VDP_DI_MA4F_MODE) {
                vVdpHalSetFieldInfo(u4VdpIdx, prParam->fgTopFiledFirst, prParam->fgFirstField); //should config filedinfo?
                if (fgUpdateHwAddr(u4VdpIdx)) {
                        vUpdateDispIdx(u4VdpIdx);

                        if (!vPmxHalGetFmtEn(u4VdpIdx)) {
                                /* Enable FMT to show video*/
                                vPmxHalEnableFmt(u4VdpIdx);
                                vVdpHalReset(u4VdpIdx);
                                VDO_LOG(VDO_LOG_LVL_DBG, "%s hw %d enable fmt\r\n", __func__, u4VdpIdx);
                        }

                        if (u4VdpIdx == VDP_1) {
                                /* Front need set mix layer and rear setting in tve module*/
                                vPmxMixPlane(PMX_1, PMX_HW_PLANE_1);
                                VDO_LOG(VDO_LOG_LVL_DBG, "enable video layer\r\n");
                        }

                        prParam->u4VdpStatus = VDP_STATUS_SHOW;
                }
        }

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

        if (prParam->fgProgSrc || !(prParam->u4Flags & VDP_ENABLE_DEINT) || fgHDSource(prParam)) {
                fgResult = FALSE;
        }
#if !ENABLE_MM_DEINTERLACE
        else if (prParam->u4SrcType == USB) {
                fgResult = FALSE;
        }
#endif

        return fgResult;
}

#define RECT_WIDTH(r)           ((r)->right - (r)->left)
#define RECT_HEIGHT(r)          ((r)->bottom - (r)->top)

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
                        prParam->u4VdpMode = RES_480P;
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
        
        if (fgNeedDeint(prParam)) {
                prParam->u4DeintMode = VDP_DI_MA4F_MODE;
        } else {
                if(!(prParam->u4Flags & VDP_TOP_LEVEL)) {
                        if(prParam->u4SrcType == 0x11) {
                                prParam->u4DeintMode = VDP_DI_FIELD_MODE;
                        } else {
                                prParam->u4DeintMode = VDP_DI_FRAME_MODE;
                        }
                }
        }


        if ((prParam->u4DeintMode != u4OldDeintMode) || (prParam->u4Flags & VDP_UPDATE_OVERLAY)) {
                FB_PRINT(FB_LOG_LVL_DBG, "VDP", "%s: idx %d, old %d, new %d, vdp mode %d\r\n", __func__,
                        (int)u4VdpIdx, (int)u4OldDeintMode, (int)prParam->u4DeintMode, (int)prParam->u4VdpMode);

                if (prParam->u4DeintMode == VDP_DI_MA4F_MODE) {
                        prParam->u4NeedClrBuff |= VDP_CLR_ONCE_BUF;
                        prParam->u4ClearIdx = (prParam->u4DispIdx - 1 + MAX_BUFF_CNT) % MAX_BUFF_CNT; /* unused idx*/
                        prParam->u4PrevIdx = prParam->u4DispIdx;
                        FB_PRINT(FB_LOG_LVL_DBG, "VDP", "vSetDeintMode: idx %d, prev %d, disp %d, status %d\r\n"
                                , (int)u4VdpIdx, (int)prParam->u4PrevIdx
                                , (int)prParam->u4DispIdx, (int)prParam->u4VdpStatus);
                } else {
                        prParam->u4NeedClrBuff |= VDP_CLR_ONCE_BUF;
                        prParam->u4ClearIdx = prParam->u4PrevIdx; /* unused buffer idx*/
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
                VDO_LOG(VDO_LOG_LVL_INFO, "fgCheckVdpFlags: enter fast forward flags %x, status %d, di %d\r\n",
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

                VDO_LOG(VDO_LOG_LVL_INFO, "fgCheckVdpFlags: exit fast forward flags %x, status %d, di %d\r\n",
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
                VDO_LOG(VDO_LOG_LVL_INFO, "fgCheckVdpFlags: seek flags %x, status %d, di %d\r\n",
                        (unsigned int)prParam->u4Flags, (int)prParam->u4VdpStatus, (int)prParam->u4DeintMode);
        }

        return TRUE;
}

bool fgClrSetHDFrmBuff(VDP_PARAM *prParam, __u32 u4BufIdx, __u32 u4YAddr, __u32 u4CAddr)
{
        /* Release buffer and set new buffer address*/
        fgClearFrmBuff(prParam, u4BufIdx, u4BufIdx);

        if ((u4YAddr & 0x7F) || (u4CAddr & 0x7F)) {
                VDO_LOG(VDO_LOG_LVL_ERR, "fgClrSetHDFrmBuff: error buffer is not 128 bytes alignment %x, %x\r\n",
                        (unsigned int)u4YAddr, (unsigned int)u4CAddr);
                return FALSE;
        }

        prParam->u4FrmBuffY[u4BufIdx] = u4YAddr;
        prParam->u4FrmBuffC[u4BufIdx] = u4CAddr;

        if (prParam->u4NeedClrBuff != VDP_CLR_NONE_BUF) {
                VDO_LOG(VDO_LOG_LVL_DBG, "fgClrSetHDFrmBuff: u4NeedClrBuff error id %d, flga %x\r\n",
                        (int)u4BufIdx, (unsigned int)prParam->u4NeedClrBuff);
                prParam->u4NeedClrBuff = VDP_CLR_NONE_BUF;
        }

        return TRUE;
}

bool fgHideVdp(__u32 u4VdpIdx)
{
        VDP_PARAM *prParam = &rData[u4VdpIdx];

        if (u4VdpIdx == VDP_1) {
                vPmxNotMixPlane(PMX_1, PMX_HW_PLANE_1);
                VDO_LOG(VDO_LOG_LVL_INFO, "%s vPmxNotMixPlane video plane\r\n", __func__);
        }

        vPmxHalDisableFmt(u4VdpIdx);

        if (prParam->u4NeedClrBuff == VDP_CLR_NONE_BUF) { /* MA4F need 4 buffer to show 1 frame*/
                fgResetParam(prParam);
        } else {
                fgClearFrmBuff(prParam, 0, MAX_BUFF_CNT - 1);
                fgResetParam(prParam);
        }
        VDO_LOG(VDO_LOG_LVL_INFO, "%s dev %d status %d\r\n", __func__, u4VdpIdx, prParam->u4VdpStatus);

        return TRUE;
}

static bool Hide_Video(__u32 u4VdpIdx)
{
        fgHideVdp(u4VdpIdx);
        FBM_Uninit(u4VdpIdx);
        return TRUE;
}

static bool fgResizerBuffer(VDP_PARAM *prParam, PMTKSurface pDstSurf)
{
#ifndef __ARM2__
        IMGRESZ_HAL_IMG_INFO_T rSrcBufInfo;
        IMGRESZ_HAL_IMG_INFO_T rDstBufInfo;
        __u32 u4BufIdx = (prParam->u4CurrIdx - 1 + MAX_BUFF_CNT) % MAX_BUFF_CNT;
        __u64 u4YAddr = pDstSurf->PhysicalAddress(pDstSurf);
        __u64 u4CAddr = pDstSurf->PhysicalAddress(pDstSurf) +
                ROUND_UP_COUNT(pDstSurf->Width(pDstSurf), 16) * ROUND_UP_COUNT(pDstSurf->Height(pDstSurf), 32);
        bool fg8Tap = FALSE;

        memset(&rSrcBufInfo, 0, sizeof(rSrcBufInfo));
        rSrcBufInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER;
        rSrcBufInfo.rBufferFormat.eYUVFormat = IMGRESZ_HAL_IMG_YUV_FORMAT_420;
        rSrcBufInfo.rBufferFormat.fgBlockMode = !(prParam->u4Flags & VDP_SCANLINE_MODE);
        rSrcBufInfo.rBufferFormat.fgAddrSwap = FALSE;//TRUE;

        if (!prParam->fgNeedRotate) {
                rSrcBufInfo.u4BufWidth = ROUND_UP_COUNT(prParam->u4SrcWidth, 16);
                rSrcBufInfo.u4BufHeight = ROUND_UP_COUNT(prParam->u4SrcHeight, 32);
                rSrcBufInfo.u4ImgWidth = prParam->rSrcRect.right;
                rSrcBufInfo.u4ImgHeight = prParam->rSrcRect.bottom;
                rSrcBufInfo.u4ImgXOff = 0;
                rSrcBufInfo.u4ImgYOff = 0;
        } else {
                rSrcBufInfo.u4BufWidth = prParam->u4SrcWidth;
                rSrcBufInfo.u4BufHeight = prParam->u4SrcHeight;
                rSrcBufInfo.u4ImgWidth = prParam->rSrcRect.right - prParam->rSrcRect.left;
                rSrcBufInfo.u4ImgHeight = prParam->rSrcRect.bottom - prParam->rSrcRect.top;
                rSrcBufInfo.u4ImgXOff = prParam->rSrcRect.left;
                rSrcBufInfo.u4ImgYOff = prParam->rSrcRect.top;
        }

        rSrcBufInfo.u4BufSA1 = (unsigned long)prParam->u4FrmBuffY[u4BufIdx];
        rSrcBufInfo.u4BufSA2 = (unsigned long)prParam->u4FrmBuffC[u4BufIdx];

        /* Set target buffer info*/
        memset(&rDstBufInfo, 0, sizeof(rSrcBufInfo));
        rDstBufInfo.rBufferFormat.eBufferMainFormat = IMGRESZ_HAL_IMG_BUF_MAIN_FORMAT_Y_C_BUFFER;
        rDstBufInfo.rBufferFormat.eYUVFormat = IMGRESZ_HAL_IMG_YUV_FORMAT_420;
        rDstBufInfo.rBufferFormat.fgBlockMode = !(prParam->u4Flags & VDP_SCANLINE_MODE);
        rDstBufInfo.rBufferFormat.fgAddrSwap = FALSE;//TRUE;

        if (!prParam->fgNeedRotate) {
                rDstBufInfo.u4BufWidth =  ROUND_UP_COUNT(pDstSurf->Width(pDstSurf), 16);
                rDstBufInfo.u4BufHeight = ROUND_UP_COUNT(pDstSurf->Height(pDstSurf), 32);
        } else {
                rDstBufInfo.u4BufWidth =  ROUND_UP_COUNT(pDstSurf->Width(pDstSurf), 16);
                rDstBufInfo.u4BufHeight = pDstSurf->Height(pDstSurf);
        }

        rDstBufInfo.u4ImgWidth = ROUND_UP_COUNT(pDstSurf->Width(pDstSurf), 2);//prParam->rSrcRect.right;
        rDstBufInfo.u4ImgXOff = 0;
        rDstBufInfo.u4ImgYOff = 0;
        rDstBufInfo.u4BufSA1 = u4YAddr;
        rDstBufInfo.u4BufSA2 = u4CAddr;

        rSrcBufInfo.rBufferFormat.fgProgressiveFrame = TRUE;
        rSrcBufInfo.u4ImgHeight = prParam->rSrcRect.bottom;

        rDstBufInfo.rBufferFormat.fgProgressiveFrame = TRUE;
        rDstBufInfo.u4ImgHeight = pDstSurf->Height(pDstSurf);//prParam->rSrcRect.bottom;

        SpinUnlock(SPIN_LOCK_SRC_RESIZE); /* Can not sleep in spin lock case*/

        #if 0
        /*if wch srctype ,both front and rear now, imgresz use H-8Tap method when > 720p*/
        if ((prParam->u4SrcType != USB) && (rData[0].fgNeedResizer && rData[1].fgNeedResizer)) {
                if (rSrcBufInfo.u4ImgWidth > 1280 || rSrcBufInfo.u4ImgHeight > 720) {
                        fg8Tap = TRUE;
                }
        }
        #endif

        TS_DirectScale(1, fg8Tap, &rSrcBufInfo, &rDstBufInfo);
        /*VDO_LOG(VDO_LOG_LVL_DBG, "fgResizerBuffer: 1 in src buf %d, %d, img %d, %d, dst buf %d, %d, img %d, %d\n"
                , (int)rSrcBufInfo.u4BufWidth, (int)rSrcBufInfo.u4BufHeight, (int)rSrcBufInfo.u4ImgWidth
                , (int)rSrcBufInfo.u4ImgHeight, (int)rDstBufInfo.u4BufWidth, (int)rDstBufInfo.u4BufHeight
                , (int)rDstBufInfo.u4ImgWidth, (int)rDstBufInfo.u4ImgHeight);*/

        SpinLock(SPIN_LOCK_SRC_RESIZE);

        if (!fgClrSetHDFrmBuff(prParam, u4BufIdx, (unsigned int)u4YAddr, (unsigned int)u4CAddr)) {
                return FALSE;
        }
#endif
        return TRUE;
}

#define CALC_IMGRESZ_TIME 0
static bool Show_Video(__u32 u4VdpIdx)
{
        VDP_PARAM *prParam = &rData[u4VdpIdx];
        bool fgNeedResizer = prParam->fgNeedResizer;
        #if CALC_IMGRESZ_TIME
        struct timeval begin_t = {0, 0};
        struct timeval end_t = {0, 0};
        long usec = 0;
        #endif

        if (prParam->u4SrcType == USB) {
                prParam->fgNeedResizer = 1;
        }

        if ((fgNeedResizer != prParam->fgNeedResizer) && !(prParam->u4Flags & VDP_UPDATE_OVERLAY)) {
                /* change buffer size need to reconfig vdp param*/
                VDO_LOG(VDO_LOG_LVL_DBG, "[Show_Video] hw id %d fgNeedResizer flag change to %d\r\n"
                        , (int)u4VdpIdx, (int)prParam->fgNeedResizer);
                prParam->u4Flags |= VDP_UPDATE_OVERLAY;
        }

        vSetDeintMode(u4VdpIdx, prParam);
        fgCheckVdpFlags(u4VdpIdx, prParam);

        if (prParam->fgNeedResizer) {
                PMTKSurface pDstSurf = NULL, pBuffer = NULL;
                unsigned int j = 0;

                if (pImgDstSurf[u4VdpIdx][0] == NULL) { /* firstly allocate buffer, no need secondly*/
                        for (j = 0; j < IMG_RESZ_BUFF_SIZE; j++) {
                                /* Ying-ToDo: Alloc surface dynamic and get panel width and height*/
                                pBuffer = AllocSurface(1920, 1080);

                                if (pBuffer == NULL) {
                                        VDO_LOG(VDO_LOG_LVL_ERR, "[ddi] Can not allocate surface\r\n ");
                                }

                                pImgDstSurf[u4VdpIdx][j] = pBuffer;
                        }
                }

                if (FBM_IsNotEmpty(u4VdpIdx) == FALSE) {
                        FBM_Init(u4VdpIdx, (LPVOID *)pImgDstSurf[u4VdpIdx], IMG_RESZ_BUFF_SIZE);
                }
                if (prParam->u4Flags & VDP_UPDATE_OVERLAY) {
                        unsigned int u4Width, u4Height;

                        u4Width = prParam->rDstRect.right - prParam->rDstRect.left;
                        u4Height = prParam->rDstRect.bottom - prParam->rDstRect.top;

                        for (j = 0; j < IMG_RESZ_BUFF_SIZE; j++) {
                                pBuffer =  pImgDstSurf[u4VdpIdx][j];
                                pBuffer->SetPicSize(pBuffer, u4Width, u4Height);
                        }
                }

                pDstSurf = (PMTKSurface)FBM_Lock(u4VdpIdx);
                #if CALC_IMGRESZ_TIME
                do_gettimeofday(&begin_t);
                #endif
                fgResizerBuffer(prParam, pDstSurf);
                #if CALC_IMGRESZ_TIME
                do_gettimeofday(&end_t);
                usec = (end_t.tv_sec - begin_t.tv_sec) * 1000000 + (end_t.tv_usec - begin_t.tv_usec);
                VDO_LOG(VDO_LOG_LVL_DBG, "[xzr] resize cost time:%dusec\r\n ", usec); 
                #endif
                FBM_Flip(u4VdpIdx);

                if (prParam->u4Flags & VDP_UPDATE_OVERLAY) {
                        prParam->rSrcRect.left = 0;
                        prParam->rSrcRect.top = 0;
                        prParam->rSrcRect.right = pDstSurf->Width(pDstSurf);
                        prParam->rSrcRect.bottom = pDstSurf->Height(pDstSurf);
                        prParam->u4SrcWidth = ROUND_UP_COUNT(pDstSurf->Width(pDstSurf), 16);
                        prParam->u4SrcHeight = pDstSurf->Height(pDstSurf);
                }
                prParam->u4NeedClrBuff = VDP_CLR_NONE_BUF;
        }else {
                prParam->u4NeedClrBuff |= VDP_CLR_REGU_BUF;
        }

        /*VDO_LOG(VDO_LOG_LVL_DBG, "[VDP]Show_Video prParam->u4Flags 0x%x\n", (unsigned int)prParam->u4Flags);*/
        if (prParam->u4Flags & VDP_UPDATE_OVERLAY) {
                fgConfigVdpHw(u4VdpIdx);
        } else if (prParam->u4DeintMode < VDP_DI_MA4F_MODE) {
                prParam->u4NeedShowBuff++;
                if (fgUpdateHwAddr(u4VdpIdx)) {
                        vUpdateDispIdx(u4VdpIdx);
                }
        }

        return TRUE;
}

int VDP_IOControl(__u32 dwCode, void *pBufIn, void *pBufOut)
{
        __u32 u4Ret = 0, u4VdpIdx = 0, i, loop_cnt;
        bool ret = FALSE;
        struct OVERLAY_PARAM rear_follow_with_front_vdp_param;
        struct OVERLAY_PARAM *pOverlay = NULL;
        VDP_PARAM *prParam = NULL;
#ifndef __ARM2__
        if (!fgLockInit) {
                spin_lock_init(&_vdp_lock);
                fgLockInit = TRUE;
                VDO_LOG(VDO_LOG_LVL_DBG, "vdpisr spin lock init %x\r\n", (unsigned int)&_vdp_lock);
        }
#endif
        if (!pBufIn) {
                VDO_LOG(VDO_LOG_LVL_ERR, "VDP_IOControl: input buffer is null code = 0x%x\n", dwCode);
                return -EINVAL;
        }

        loop_cnt = (fr_follow & FR_FOLLOW_VIDEO) ? 2 : 1;

        switch (dwCode) {
        case VIDIOC_STREAMOFF: {
                /*if (!access_ok(VERIFY_READ, pBufIn, sizeof(struct OVERLAY_PARAM))) {
                        u4Ret  = -EINVAL;
                        VDO_LOG(VDO_LOG_LVL_ERR, "VIDIOC_STREAMOFF return %d\r\n", u4Ret);
                        break;
                }*/
                pOverlay = (struct OVERLAY_PARAM *)pBufIn;
                VDO_LOG(VDO_LOG_LVL_INFO, "VDP_IOControl: VIDIOC_STREAMOFF cmd 0x%x id %d\n", dwCode, pOverlay->u4Idx);

                for (i = 0; i < loop_cnt; i++) {
                        SpinLock(SPIN_LOCK_SRC_IOCTL);
                        if ((i == VDP_2) && (fr_follow & FR_FOLLOW_VIDEO)) {
                                if (vdo_fr_on) {
                                        VDO_LOG(VDO_LOG_LVL_INFO, "%s id from %d to %d\n",
                                                __func__, pOverlay->u4Idx, i);
                                        pOverlay->u4Idx = i;
                                        vdo_fr_on = FALSE;
                                } else {
                                        VDO_LOG(VDO_LOG_LVL_INFO, "%s vdo fr off and do nothing\n", __func__);
                                }
                        }
                        
                        u4VdpIdx = pOverlay->u4Idx;
                        Hide_Video(u4VdpIdx);
                        SpinUnlock(SPIN_LOCK_SRC_IOCTL);
                }
                break;
        }

        case VIDIOC_QBUF: {
                /*if (!access_ok(VERIFY_READ, pBufIn, sizeof(struct OVERLAY_PARAM))) {
                        u4Ret  = -EINVAL;
                        VDO_LOG(VDO_LOG_LVL_ERR, "VIDIOC_QBUF return %d\r\n", u4Ret);
                        break;
                }*/
                pOverlay = (struct OVERLAY_PARAM *)pBufIn;

                if ((loop_cnt == 1) && vdo_fr_on) {
                        VDO_LOG(VDO_LOG_LVL_INFO, "%s fr_follow %d vdo fr %d and hide rear\n", __func__
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
                                        VDO_LOG(VDO_LOG_LVL_INFO, "%s id from %d to %d vdo fr %d\n", __func__
                                                , pOverlay->u4Idx, i, (u32)vdo_fr_on);
                                        pOverlay->u4Flags |= VDP_UPDATE_OVERLAY;
                                        vdo_fr_on = TRUE;
                                }
                                pOverlay->u4Idx = i;
                        }
                        u4VdpIdx = pOverlay->u4Idx;
                        prParam = &rData[u4VdpIdx];

                        VDO_LOG(VDO_LOG_LVL_DBG, "VDP_IOControl: VIDIOC_QBUF cmd 0x%x\n", dwCode);
                        if (pOverlay->u4Flags & VDP_UPDATE_OVERLAY) {
                                VDO_LOG(VDO_LOG_LVL_INFO, "VIDIOC_QBUF param 0x%x id=%d, src %d,%d, {%d,%d,%d,%d}\r\n"
                                        , (unsigned int)pBufIn, (int)pOverlay->u4Idx
                                        , (int)pOverlay->u4SrcWidth, (int)pOverlay->u4SrcHeight
                                        , (int)pOverlay->rSrcRect.left, (int)pOverlay->rSrcRect.top
                                        , (int)pOverlay->rSrcRect.right, (int)pOverlay->rSrcRect.bottom);
                                VDO_LOG(VDO_LOG_LVL_INFO
                                        , "VIDIOC_QBUF dst {%d,%d,%d,%d} Addr=%x,%x, stutas=%d, flags=%x\r\n"
                                        , (int)pOverlay->rDstRect.left, (int)pOverlay->rDstRect.top
                                        , (int)pOverlay->rDstRect.right, (int)pOverlay->rDstRect.bottom
                                        , (unsigned int)pOverlay->u4PhysicalAddressY
                                        , (unsigned int)pOverlay->u4PhysicalAddressC
                                        , (int)pOverlay->u4Status, (unsigned int)pOverlay->u4Flags);
                        }

                        if (!_fgInitVdpParam[u4VdpIdx]) {
                                fgResetParam(prParam);
                                _fgInitVdpParam[u4VdpIdx] = TRUE;
                        }

                        SpinLock(SPIN_LOCK_SRC_IOCTL);

                        if (pOverlay->u4Flags & VDP_TOP_LEVEL) {
                                if (fgPmxHalMixPlane(PMX_1, PRIMARY_SURF_PLANE)) {
                                        vPmxNotMixPlane(PMX_1, PRIMARY_SURF_PLANE);
                                        VDO_LOG(VDO_LOG_LVL_DBG, "vdp not mix primary surface plane %d\r\n", PRIMARY_SURF_PLANE);
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
                        
                        SpinUnlock(SPIN_LOCK_SRC_IOCTL);
                }
                break;
        }
#ifndef __ARM2__
        case STIOC_SET_VDP_PARAM: {
                /*if (!access_ok(VERIFY_READ, pBufIn, sizeof(struct OVERLAY_PARAM))) {
                        u4Ret  = -EINVAL;
                        VDO_LOG(VDO_LOG_LVL_ERR, "STIOC_SET_VDP_PARAM return %d\r\n", u4Ret);
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
                        VDO_LOG(VDO_LOG_LVL_DBG, "VDP_IOControl: STIOC_SET_VDP_PARAM cmd 0x%x\n", dwCode);
                        VDO_LOG(VDO_LOG_LVL_INFO, "STIOC_SET_VDP_PARAM param 0x%x id=%d, src %d,%d, {%d,%d,%d,%d}\r\n"
                                , (unsigned int)pBufIn, (int)pOverlay->u4Idx
                                , (int)pOverlay->u4SrcWidth, (int)pOverlay->u4SrcHeight
                                , (int)pOverlay->rSrcRect.left, (int)pOverlay->rSrcRect.top
                                , (int)pOverlay->rSrcRect.right, (int)pOverlay->rSrcRect.bottom);
                        VDO_LOG(VDO_LOG_LVL_INFO
                                , "STIOC_SET_VDP_PARAM dst {%d,%d,%d,%d} Addr=%x,%x, stutas=%d, flags=%x\r\n"
                                , (int)pOverlay->rDstRect.left, (int)pOverlay->rDstRect.top
                                , (int)pOverlay->rDstRect.right, (int)pOverlay->rDstRect.bottom
                                , (unsigned int)pOverlay->u4PhysicalAddressY
                                , (unsigned int)pOverlay->u4PhysicalAddressC
                                , (int)pOverlay->u4Status, (unsigned int)pOverlay->u4Flags);
                        /* Other video is show which need back up param and release vdp hw for backcar*/

                        prParam = &rData[u4VdpIdx];

                        if ((pOverlay->device_name == BACKCAR) && (pOverlay->device_name != prParam->u4SrcType)) {
                                Hide_Video(u4VdpIdx);
                        }
                        fgSetVideoInfo(u4VdpIdx, pOverlay, TRUE);
                        SpinUnlock(SPIN_LOCK_SRC_IOCTL);
                }
                break;
        }

        case STIOC_SET_FMT_BLACK: {
                struct FMT_BG_PARAM *pFmtBG = NULL;

                /*if (!access_ok(VERIFY_READ, pBufIn, sizeof(struct FMT_BG_PARAM))) {
                        u4Ret  = -EINVAL;
                        VDO_LOG(VDO_LOG_LVL_ERR, "STIOC_SET_FMT_BLACK return %d\r\n", u4Ret);
                        break;
                }*/
                pFmtBG = (struct FMT_BG_PARAM *)pBufIn;
                prParam = &rData[pFmtBG->u4Idx];
                SpinLock(SPIN_LOCK_SRC_IOCTL);
                if (pFmtBG->fgEnable) {
                        //vPmxHalSetBg(pFmtBG->u4Idx, pFmtBG->u4Color);
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
                VDO_LOG(VDO_LOG_LVL_INFO, "VDP_IOControl: set black %d status %d and video layer %d\n"
                        , pFmtBG->fgEnable, prParam->u4VdpStatus, fgPmxHalMixPlane(0, PMX_HW_PLANE_1));
                SpinUnlock(SPIN_LOCK_SRC_IOCTL);
                break;
        }
#endif
        default:
                VDO_LOG(VDO_LOG_LVL_DBG, "VDP_IOControl: Unsupported cmd 0x%x\n", dwCode);
                break;
        }

        return u4Ret;
}
EXPORT_SYMBOL(VDP_IOControl);




