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
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/clk-private.h>
#include <asm/page.h>
#include "x_ver.h"
#include <linux/of_reserved_mem.h>
#include "drv_imgresz.h"
#include "drv_imgresz_errcode.h"
#include "imgresz_hal_if.h"
#include "imgresz_drv/imgresz_log.h"
//#include <io.h>
#include <linux/list.h>
#include <compat.h>
#include <linux/delay.h>

#define ATC_KERNEL_LINUX_LICENSE     "GPL"

#define MMISC_MODE_NAME         "IMGR"
#define MMISC_VER_MAJOR         01
#define MMISC_VER_MINOR         00
#define MMISC_VER_REV           00


struct Linebuf TempLine_Reserved ; // added by mtk68119
/*#define IMGZDEC_IOCT 10*/
/*#define IMGRESZ_YUV2YC 11*/
volatile int scale_done = 0;

#if CONFIG_COMPAT
IMGRESZ_DRV_TICKET_T global_ticket;

typedef struct _COMPAT_IMGRESZ_DRV_SRC_BUF_INFO_T
{
    IMGRESZ_DRV_SRC_COLOR_MODE  eSrcColorMode;  ///< The color mode of input source buffer.
    compat_uptr_t                    u4YBufAddr;     ///< In video mode, it means the Y buffer.
                                                ///< In graph mode, it means the graph buffer.
    compat_uptr_t                    u4CbBufAddr;    ///< Only used in video and jpeg mode.
    compat_uptr_t                    u4CrBufAddr;    ///< Only used in jpeg partial and picture mode.
    __u32                      u4BufWidth;     ///< The buffer width (pitch).
    __u32                      u4BufHeight;    ///< The buffer height.
    __u32                      u4PicPosX;      ///< The picture position X.
    __u32                      u4PicPosY;      ///< The picture position Y.
    __u32                      u4PicWidth;     ///< The picture width.
    __u32                      u4PicHeight;    ///< The picture height.
    bool                        fgInterlaced;   ///< Interlaced picture.
    bool                        fgTopField;     ///< Interlaced picture top field exist. (For interlaced video only)
    bool                        fgBottomField;  ///< Interlaced picture bottom field exist. (For interlaced video only)
    IMGRESZ_DRV_COMPONENT_FACTOR_T  rCompFactor;    ///< Used when eSrcColorMode == IMGRESZ_INPUT_COL_MD_JPG_DEF.
    compat_uptr_t                      u4ColorPalletSa; ///< Color Pallet.
    bool                        fgWTEnable;     ///< Wavelet transform compression.
} COMPAT_IMGRESZ_DRV_SRC_BUF_INFO_T;

typedef struct _COMPAT_IMGRESZ_DRV_DST_BUF_INFO_T
{
    IMGRESZ_DRV_DST_COLOR_MODE  eDstColorMode;  ///< The color mode of output destination buffer.
     compat_uptr_t                       u4YBufAddr;     ///< In video mode, it means the Y buffer.
                                                ///< In graph mode, it means the graph buffer.
     compat_uptr_t                       u4CBufAddr;     ///< Only used in video mode.
    __u32                      u4BufWidth;     ///< The buffer width (pitch).
    __u32                      u4BufHeight;    ///< The buffer height.
    __u32                      u4PicPosX;      ///< The picture position X.
    __u32                      u4PicPosY;      ///< The picture position Y.
    __u32                      u4PicWidth;     ///< The picture width.
    __u32                      u4PicHeight;    ///< The picture height.
    bool                        fgWTEnable;     ///< Wavelet transform compression.    
} COMPAT_IMGRESZ_DRV_DST_BUF_INFO_T;

typedef struct _COMPAT_IMGRESZ_DRV_PARTIAL_INFO_T
{
    compat_uptr_t                      u4YBufAddr;     ///< In graph mode, it means the graph buffer.
    compat_uptr_t                       u4CbBufAddr;    ///< Only used in jpeg mode.
    compat_uptr_t                      u4CrBufAddr;    ///< Only used in jpeg mode.
    __u32                      u4YBufLine;     ///< Y buffer line number (if eSrcColorMode != IMGRESZ_INPUT_COL_MD_JPG_DEF).
    __u32                      u4CbBufLine;    ///< Cb buffer line number (if eSrcColorMode != IMGRESZ_INPUT_COL_MD_JPG_DEF).
    __u32                      u4CrBufLine;    ///< Cr buffer line number (if eSrcColorMode != IMGRESZ_INPUT_COL_MD_JPG_DEF).
    bool                        fgFirstRow;     ///< The First row.
    bool                        fgLastRow;      ///< The Last row.
} COMPAT_IMGRESZ_DRV_PARTIAL_INFO_T;


#endif

static long imgresz_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	void *private_data;
	
	bool bRet = true;
	long ret;
	IMGRESZ_DRV_SCALE_MODE scale_mode;
	bool lock_mode;
	IMGRESZ_DRV_DO_SCALE_T do_scale_t;
	int sleep_count = 0;
	IMGRESZ_DRV_RM_INFO_T rm_info;
	IMGRESZ_DRV_SRC_BUF_INFO_T src_buf;
	IMGRESZ_DRV_DST_BUF_INFO_T dst_buf;
	IMGRESZ_DRV_PARTIAL_INFO_T partical_buf;
	
	IMGR_LOG(IMGR_LOG_LVL_DBG, "enter ioctl,cmd is %d\n",cmd);

	private_data = filp->private_data;

	
	
	switch (cmd) {
			
		case IMGRESZ_GET_TICKET :
		ret=i4ImgResz_Drv_GetTicket(&global_ticket);
		if(ret == -1) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get imgresz ticket,return\n");
			return -1;
			}
		IMGR_LOG(IMGR_LOG_LVL_DBG, "get ticket %d\n",global_ticket.u4Ticket);
	    break;
		
		case IMGRESZ_RELEASE_TICKET:
			i4ImgResz_Drv_ReleaseTicket(&global_ticket);
			IMGR_LOG(IMGR_LOG_LVL_DBG, "release ticket %d\n",global_ticket.u4Ticket);
			break;
		
		case IMGRESZ_SET_PRORITY:
			i4ImgResz_Drv_SetPriority(&global_ticket,IMGRESZ_DRV_PRIORITY_HIGH);
			break;

		case IMGRESZ_SET_SCALEMODE:
			if (copy_from_user((void *)&scale_mode, (void *)arg, sizeof(IMGRESZ_DRV_SCALE_MODE))) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get scale_mode,return\n");
			return -1;
			}
			IMGR_LOG(IMGR_LOG_LVL_DBG, "scale_mode is %d\n",scale_mode);
			i4ImgResz_Drv_SetScaleMode(&global_ticket,scale_mode);
			break;
			
		case IMGRESZ_SET_LOCK:
			if (copy_from_user((void *)&lock_mode, (void *)arg, sizeof(bool))) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get lock_mode,return\n");
			return -1;
			}
			IMGR_LOG(IMGR_LOG_LVL_DBG, "lock_mode is %d\n",lock_mode);
			i4ImgResz_Drv_SetLock(&global_ticket,lock_mode);
			break;

		case IMGRESZ_DO_SCALE:
			if (copy_from_user((void *)&do_scale_t, (void *)arg, sizeof(IMGRESZ_DRV_DO_SCALE_T))) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get do_scale_t,return\n");
			return -1;
			}
			i4ImgResz_Drv_DoScale(&global_ticket,&do_scale_t);

			while(scale_done == 0) {

			sleep_count++;
			msleep(4);

			if(sleep_count > 250) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "do scale timeout,return\n");
			return -1;
			}
		}
			scale_done =0;
			IMGR_LOG(IMGR_LOG_LVL_DBG, "do scale done,return\n");
			break;

		case IMGRESZ_STOP_SCALE:
			i4ImgResz_Drv_StopScale(&global_ticket);
			break;

		case IMGRESZ_SET_RMINFO:
			if (copy_from_user((void *)&rm_info, (void *)arg, sizeof(IMGRESZ_DRV_RM_INFO_T))) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get rm_info,return\n");
			return -1;
			}

			i4ImgResz_Drv_SetRmInfo(&global_ticket,&rm_info);
			break;

		case IMGRESZ_SET_SRCBUF:
			if (copy_from_user((void *)&src_buf, (void *)arg, sizeof(IMGRESZ_DRV_SRC_BUF_INFO_T))) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get src_buf,return\n");
			return -1;
			}
			
			IMGR_LOG(IMGR_LOG_LVL_DBG, "src buf info %d %d,%d,%d,%d,%lx,%lx,%lx,%lx,%d\n",src_buf.eSrcColorMode,src_buf.u4BufWidth,src_buf.u4BufHeight,src_buf.u4PicWidth,src_buf.u4PicHeight,
			src_buf.u4YBufAddr,src_buf.u4CbBufAddr,src_buf.u4CrBufAddr,src_buf.u4ColorPalletSa,src_buf.fgWTEnable);
		
			i4ImgResz_Drv_SetSrcBufInfo(&global_ticket,&src_buf);
			break;

		case IMGRESZ_SET_DSTBUF:
			if (copy_from_user((void *)&dst_buf, (void *)arg, sizeof(IMGRESZ_DRV_DST_BUF_INFO_T))) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get dst_buf,return\n");
			return -1;
			}
			
			IMGR_LOG(IMGR_LOG_LVL_DBG, "dst buf info %d,%d,%d,%d,%d,%lx,%lx,%d\n",dst_buf.eDstColorMode,dst_buf.u4BufWidth,dst_buf.u4BufHeight,dst_buf.u4PicWidth,dst_buf.u4PicHeight,
			dst_buf.u4YBufAddr,dst_buf.u4CBufAddr,dst_buf.fgWTEnable);

			i4ImgResz_Drv_SetDstBufInfo(&global_ticket,&dst_buf);
			break;

		case IMGRESZ_SET_PARTICALBUF:
			if (copy_from_user((void *)&partical_buf, (void *)arg, sizeof(IMGRESZ_DRV_PARTIAL_INFO_T))) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get dst_buf,return\n");
			return -1;
			}
			
			i4ImgResz_Drv_SetPartialBufInfo(&global_ticket,&partical_buf);
			break;
	
		default:
			IMGR_LOG(IMGR_LOG_LVL_ERR, "unknown command\n");
			bRet=false;
			break;
	}
	if (!bRet) {
		return -1;
	}

	return 0;
}

#if CONFIG_COMPAT
long Imgresz_compat32_ioctl(struct file *file, unsigned int cmd,unsigned long arg)
{
	long ret = 0;
	void __user *up = compat_ptr(arg);
	IMGRESZ_DRV_SCALE_MODE scale_mode;
	bool lock_mode;
	IMGRESZ_DRV_DO_SCALE_T do_scale_t;
	int sleep_count = 0;
	IMGRESZ_DRV_RM_INFO_T rm_info;
	IMGRESZ_DRV_SRC_BUF_INFO_T src_buf;
	IMGRESZ_DRV_DST_BUF_INFO_T dst_buf;
	IMGRESZ_DRV_PARTIAL_INFO_T partical_buf;
	IMGRESZ_DRV_SCALE_MODE * scale_modearg2;
	bool* lock_modearg2;
	IMGRESZ_DRV_DO_SCALE_T* do_scale_targ2;
	IMGRESZ_DRV_RM_INFO_T * rm_infoarg2;
	COMPAT_IMGRESZ_DRV_SRC_BUF_INFO_T * src_bufarg2;
	COMPAT_IMGRESZ_DRV_DST_BUF_INFO_T *dst_bufarg2;
	COMPAT_IMGRESZ_DRV_PARTIAL_INFO_T *partical_bufarg2;

	
	
	if (!file->f_op->unlocked_ioctl) {
		    IMGR_LOG(IMGR_LOG_LVL_ERR, "file has no unlocked_ioctl,return\n");
			return ret;
		}

	switch (cmd) {
	case IMGRESZ_GET_TICKET :
		IMGR_LOG(IMGR_LOG_LVL_DBG, "call imgresz compat ioctl get ticket\n");
		ret=i4ImgResz_Drv_GetTicket(&global_ticket);
		if(ret == -1) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get imgresz ticket,return\n");
			return -1;
			}
		IMGR_LOG(IMGR_LOG_LVL_DBG, "get ticket %d\n",global_ticket.u4Ticket);
	    break;
	case IMGRESZ_RELEASE_TICKET:
		IMGR_LOG(IMGR_LOG_LVL_DBG, "call imgresz compat ioctl release ticket\n");
		i4ImgResz_Drv_ReleaseTicket(&global_ticket);
		IMGR_LOG(IMGR_LOG_LVL_DBG, "release ticket %d\n",global_ticket.u4Ticket);
		break;
		
	case IMGRESZ_SET_PRORITY:
		IMGR_LOG(IMGR_LOG_LVL_DBG, "call imgresz compat ioctl set priority\n");
		i4ImgResz_Drv_SetPriority(&global_ticket,IMGRESZ_DRV_PRIORITY_HIGH);
		break;
		
	case IMGRESZ_SET_SCALEMODE:
		IMGR_LOG(IMGR_LOG_LVL_DBG, "call imgresz compat ioctl set scale mode\n");
	     scale_modearg2 = (IMGRESZ_DRV_SCALE_MODE *)up;
		if (get_user(scale_mode, scale_modearg2)) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get scale_mode,return\n");
			return -1;
		}
		IMGR_LOG(IMGR_LOG_LVL_DBG, "scale_mode is %d\n",scale_mode);
		i4ImgResz_Drv_SetScaleMode(&global_ticket,scale_mode);
		break;
		
	case IMGRESZ_SET_LOCK:
		
		lock_modearg2 = (bool *)up;
		if (get_user(lock_mode, lock_modearg2)) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get scale_mode,return\n");
			return -1;
		}
		i4ImgResz_Drv_SetLock(&global_ticket,lock_mode);
		IMGR_LOG(IMGR_LOG_LVL_DBG, "call imgresz compat ioctl set lock %d\n",lock_mode);
		break;

	case IMGRESZ_DO_SCALE:
		IMGR_LOG(IMGR_LOG_LVL_DBG, "call imgresz compat ioctl do scale\n");
		do_scale_targ2 = (IMGRESZ_DRV_DO_SCALE_T *)up;
		if (get_user(do_scale_t.u4Unused, &(do_scale_targ2->u4Unused))) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get do_scale,return\n");
			return -1;
		}

		IMGR_LOG(IMGR_LOG_LVL_DBG, "in do scale ticket is %d\n",global_ticket.u4Ticket);
		i4ImgResz_Drv_DoScale(&global_ticket,&do_scale_t);
		//msleep(100);
		
		while(scale_done == 0) {

			sleep_count++;
			msleep(4);

			if(sleep_count > 250) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "do scale timeout,return\n");
			return -1;
			}
		}
		scale_done =0;
		msleep(1);
		IMGR_LOG(IMGR_LOG_LVL_DBG, "do scale done,return\n");
		break;
		
	case IMGRESZ_STOP_SCALE:
		IMGR_LOG(IMGR_LOG_LVL_DBG, "call imgresz compat ioctl stop scale\n");
		i4ImgResz_Drv_StopScale(&global_ticket);
		break;

	case IMGRESZ_SET_RMINFO:
		IMGR_LOG(IMGR_LOG_LVL_DBG, "call imgresz compat ioctl set rminfo\n");
		rm_infoarg2 = (IMGRESZ_DRV_RM_INFO_T*)up;
		if (get_user(rm_info.fgRacingMode, &(rm_infoarg2->fgRacingMode)) || 
			get_user(rm_info.fgRPRMode, &(rm_infoarg2->fgRPRMode))) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get rm_info,return\n");
			return -1;
		}
		IMGR_LOG(IMGR_LOG_LVL_DBG, "rminfo %d,%d\n",rm_info.fgRPRMode,rm_info.fgRacingMode);
		i4ImgResz_Drv_SetRmInfo(&global_ticket,&rm_info);
		break;
	case IMGRESZ_SET_SRCBUF:
		IMGR_LOG(IMGR_LOG_LVL_DBG, "call imgresz compat ioctl set srcbuf\n");
		 src_bufarg2 = (COMPAT_IMGRESZ_DRV_SRC_BUF_INFO_T *)up;

		if(get_user(src_buf.eSrcColorMode, &(src_bufarg2->eSrcColorMode)) ||
		    get_user(src_buf.u4BufWidth, &(src_bufarg2->u4BufWidth)) ||
			get_user(src_buf.u4BufHeight, &(src_bufarg2->u4BufHeight)) ||
			get_user(src_buf.u4PicPosX, &(src_bufarg2->u4PicPosX)) ||
			get_user(src_buf.u4PicPosY, &(src_bufarg2->u4PicPosX)) || 
			get_user(src_buf.u4PicWidth, &(src_bufarg2->u4PicWidth)) ||
			get_user(src_buf.u4PicHeight, &(src_bufarg2->u4PicHeight)) ||
			get_user(src_buf.fgInterlaced, &(src_bufarg2->fgInterlaced)) ||
			get_user(src_buf.fgTopField, &(src_bufarg2->fgTopField)) || 
			get_user(src_buf.fgBottomField, &(src_bufarg2->fgBottomField)) ||
			get_user(src_buf.fgWTEnable, &(src_bufarg2->fgWTEnable))) {
				IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get nomal arg of src_buf,return\n");
				return -1;
		}

		if(get_user(src_buf.u4YBufAddr, &(src_bufarg2->u4YBufAddr)) ||
			get_user(src_buf.u4CbBufAddr, &(src_bufarg2->u4CbBufAddr)) ||
			get_user(src_buf.u4CrBufAddr, &(src_bufarg2->u4CrBufAddr)) ||
			get_user(src_buf.u4ColorPalletSa, &(src_bufarg2->u4ColorPalletSa))) {
				IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get compat arg of src_buf,return\n");
				return -1;
		}

		if(get_user(src_buf.rCompFactor.u4Components, &(src_bufarg2->rCompFactor.u4Components)) ||
			get_user(src_buf.rCompFactor.u1YCompFactorH, &(src_bufarg2->rCompFactor.u1YCompFactorH)) ||
			get_user(src_buf.rCompFactor.u1YCompFactorV, &(src_bufarg2->rCompFactor.u1YCompFactorV) )||
			get_user(src_buf.rCompFactor.u1CbCompFactorH, &(src_bufarg2->rCompFactor.u1CbCompFactorH)) ||
			get_user(src_buf.rCompFactor.u1CbCompFactorV, &(src_bufarg2->rCompFactor.u1CbCompFactorV)) ||
			get_user(src_buf.rCompFactor.u1CrCompFactorH, &(src_bufarg2->rCompFactor.u1CrCompFactorH))||
			get_user(src_buf.rCompFactor.u1CrCompFactorV, &(src_bufarg2->rCompFactor.u1CrCompFactorV))){
				IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get struct arg of src_buf,return\n");
				return -1;
		}
		IMGR_LOG(IMGR_LOG_LVL_DBG, "src buf info %d %d,%d,%d,%d,%lx,%lx,%lx,%lx,%d\n",src_buf.eSrcColorMode,src_buf.u4BufWidth,src_buf.u4BufHeight,src_buf.u4PicWidth,src_buf.u4PicHeight,
			src_buf.u4YBufAddr,src_buf.u4CbBufAddr,src_buf.u4CrBufAddr,src_buf.u4ColorPalletSa,src_buf.fgWTEnable);
		
		i4ImgResz_Drv_SetSrcBufInfo(&global_ticket,&src_buf);
		break;

	case IMGRESZ_SET_DSTBUF:
		IMGR_LOG(IMGR_LOG_LVL_DBG, "call imgresz compat ioctl set dstbuf\n");
		dst_bufarg2 = (COMPAT_IMGRESZ_DRV_DST_BUF_INFO_T *)up;

		if(get_user(dst_buf.eDstColorMode, &(dst_bufarg2->eDstColorMode)) ||
			get_user(dst_buf.u4BufWidth, &(dst_bufarg2->u4BufWidth)) ||
			get_user(dst_buf.u4BufHeight, &(dst_bufarg2->u4BufHeight)) ||
			get_user(dst_buf.u4PicPosX, &(dst_bufarg2->u4PicPosX)) ||
			get_user(dst_buf.u4PicPosY, &(dst_bufarg2->u4PicPosY)) ||
			get_user(dst_buf.u4PicWidth, &(dst_bufarg2->u4PicWidth)) ||
			get_user(dst_buf.u4PicHeight, &(dst_bufarg2->u4PicHeight) )||
			get_user(dst_buf.fgWTEnable, &(dst_bufarg2->fgWTEnable))) {
				IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get nomal arg of dst_buf,return\n");
				return -1;

		}

		if(get_user(dst_buf.u4YBufAddr, &(dst_bufarg2->u4YBufAddr)) ||
			get_user(dst_buf.u4CBufAddr, &(dst_bufarg2->u4CBufAddr))) {
				IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get compat arg of dst_buf,return\n");
				return -1;
		}
		IMGR_LOG(IMGR_LOG_LVL_DBG, "dst buf info %d,%d,%d,%d,%d,%lx,%lx,%d\n",dst_buf.eDstColorMode,dst_buf.u4BufWidth,dst_buf.u4BufHeight,dst_buf.u4PicWidth,dst_buf.u4PicHeight,
			dst_buf.u4YBufAddr,dst_buf.u4CBufAddr,dst_buf.fgWTEnable);
		i4ImgResz_Drv_SetDstBufInfo(&global_ticket,&dst_buf);
		break;

	case IMGRESZ_SET_PARTICALBUF:
		IMGR_LOG(IMGR_LOG_LVL_DBG, "call imgresz compat ioctl set partical buf\n");
		partical_bufarg2 = (COMPAT_IMGRESZ_DRV_PARTIAL_INFO_T *)up;

		if(get_user(partical_buf.u4YBufLine, &(partical_bufarg2->u4YBufLine)) ||
			get_user(partical_buf.u4CbBufLine, &(partical_bufarg2->u4CbBufLine)) ||
			get_user(partical_buf.u4CrBufLine, &(partical_bufarg2->u4CrBufLine)) ||
			get_user(partical_buf.fgFirstRow, &(partical_bufarg2->fgFirstRow)) ||
			get_user(partical_buf.fgLastRow, &(partical_bufarg2->fgLastRow))){
				IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get nomal arg of partical_buf,return\n");
				return -1;

		}

		if(get_user(partical_buf.u4YBufAddr, &(partical_bufarg2->u4YBufAddr))||
			get_user(partical_buf.u4CbBufAddr, &(partical_bufarg2->u4CbBufAddr)) ||
			get_user(partical_buf.u4CrBufAddr, &(partical_bufarg2->u4CrBufAddr))) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "failed to get compat arg of partical_buf,return\n");
							return -1;

		}
		i4ImgResz_Drv_SetPartialBufInfo(&global_ticket,&partical_buf);
		break;

	default:
		IMGR_LOG(IMGR_LOG_LVL_ERR, "unknown command\n");
		return -1;

	return 0;	
	}

}

#endif
struct file_operations const imgresz_fops = {
	.unlocked_ioctl = imgresz_ioctl,
	#if CONFIG_COMPAT
	.compat_ioctl = Imgresz_compat32_ioctl,
	#endif
};


static struct miscdevice imgresz_dev = {
	MISC_DYNAMIC_MINOR,
	"imgresz",
	&imgresz_fops
};

void __iomem *imgr0_sysreg_base = NULL;
void __iomem *imgr1_sysreg_base = NULL;
void __iomem *imgr2_sysreg_base = NULL;
void __iomem *imgr3_sysreg_base = NULL;
unsigned int imgr0irq = 0;
unsigned int imgr1irq = 0;
unsigned int imgr2irq = 0;
unsigned int imgr3irq = 0;
struct clk *clk_ac8317_imgr0 = NULL;
struct clk *clk_ac8317_imgr1 = NULL;
struct clk *clk_ac8317_imgr_top1 = NULL;
struct clk *clk_ac8317_imgr2 = NULL;
struct clk *clk_ac8317_imgr3 = NULL;
struct clk *clk_ac8317_imgr_top2 = NULL;

#define NO_IRQ ((unsigned int) -1)

unsigned long img_base_va = 0;
unsigned long img_base_pa = 0;
static int imgresz_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device_node *nd = pdev->dev.of_node;
	unsigned int property[4];
	unsigned long propertytmp;
	unsigned int size;
	struct device_node *node;

	imgr0_sysreg_base = of_iomap(nd, 0);

	if (!imgr0_sysreg_base) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz0 reg base address failed = %p \r\n",
			 imgr0_sysreg_base);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz0 reg base address = %p \r\n", imgr0_sysreg_base);

	imgr1_sysreg_base = of_iomap(nd, 1);

	if (!imgr1_sysreg_base) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz1 reg base address failed  = %p \r\n",
			 imgr1_sysreg_base);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz1 reg base address = %p \r\n", imgr1_sysreg_base);
	imgr2_sysreg_base = of_iomap(nd, 2);

	if (!imgr2_sysreg_base) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz2 reg base address failed = %p \r\n",
			 imgr2_sysreg_base);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz2 reg base address = %p \r\n", imgr2_sysreg_base);

	imgr3_sysreg_base = of_iomap(nd, 3);

	if (!imgr3_sysreg_base) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz3 reg base address failed  = %p \r\n",
			 imgr3_sysreg_base);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz3 reg base address = %p \r\n", imgr3_sysreg_base);

	imgr0irq = irq_of_parse_and_map(nd, 0);
	imgr0irq = imgr0irq -32;

	if (imgr0irq == NO_IRQ) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz0 irq failed = %d \r\n", imgr0irq);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz0 irq = %d \r\n", imgr0irq);

	imgr1irq = irq_of_parse_and_map(nd, 1);
	imgr1irq = imgr1irq -32;

	if (imgr1irq == NO_IRQ) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz1 irq failed = %d \r\n", imgr1irq);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz1 irq = %d \r\n", imgr1irq);

	imgr2irq = irq_of_parse_and_map(nd, 2);
	imgr2irq = imgr2irq -32;

	if (imgr2irq == NO_IRQ) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz2 irq failed = %d \r\n", imgr2irq);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz2 irq = %d \r\n", imgr2irq);

	imgr3irq = irq_of_parse_and_map(nd, 3);
	imgr3irq = imgr3irq -32;

	if (imgr3irq == NO_IRQ) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz3 irq failed = %d \r\n", imgr3irq);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz3 irq = %d \r\n", imgr3irq);

	ImgrGetHwRegAddress();

	clk_ac8317_imgr0 = devm_clk_get(&pdev->dev, "imgr0-device");

	if (!clk_ac8317_imgr0) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz0 clk failed %p\r\n", clk_ac8317_imgr0);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz0 clk success %p\r\n", clk_ac8317_imgr0);

	clk_ac8317_imgr1 = devm_clk_get(&pdev->dev, "imgr1-device");

	if (!clk_ac8317_imgr1) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz1 clk failed %p\r\n", clk_ac8317_imgr1);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz1 clk success %p\r\n", clk_ac8317_imgr1);


	clk_ac8317_imgr_top1= devm_clk_get(&pdev->dev, "imgr-clkselect1");

	if (!clk_ac8317_imgr_top1) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz select clk1 failed %p\r\n", clk_ac8317_imgr_top1);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz select clk1 success %p\r\n", clk_ac8317_imgr_top1);

	clk_ac8317_imgr2 = devm_clk_get(&pdev->dev, "imgr2-device");

	if (!clk_ac8317_imgr2) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz2 clk failed %p\r\n", clk_ac8317_imgr2);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz2 clk success %p\r\n", clk_ac8317_imgr2);

	clk_ac8317_imgr3 = devm_clk_get(&pdev->dev, "imgr3-device");

	if (!clk_ac8317_imgr3) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz3 clk failed %p\r\n", clk_ac8317_imgr3);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz3 clk success %p\r\n", clk_ac8317_imgr3);


	clk_ac8317_imgr_top2= devm_clk_get(&pdev->dev, "imgr-clkselect2");

	if (!clk_ac8317_imgr_top2) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get imgresz select2 clk failed %p\r\n", clk_ac8317_imgr_top2);
		return -1;
	}

	IMGR_LOG(IMGR_LOG_LVL_INFO, "get imgresz select2 clk success %p\r\n", clk_ac8317_imgr_top2);
	//added by mtk68119

	node = of_find_compatible_node(NULL,NULL,"atc-image-linebuf");
	if (node) {
		img_base_va = (unsigned long)of_iomap(node, 0);

		if (0 == img_base_va) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "img buffer of_iomap fail\r\n");
			return -1;
		}
		IMGR_LOG(IMGR_LOG_LVL_ERR, "get img_base_va %x\r\n",img_base_va);
		
		if (of_property_read_u32_array(node, "reg", (u32 *)property, 4)) {
			IMGR_LOG(IMGR_LOG_LVL_ERR, "get img buffer reserved memory node reg info fail\r\n");
			return -1;
		}
		img_base_pa = ((propertytmp = property[0]) << 32) | (unsigned long)property[1];
		size = ((propertytmp = property[2]) << 32) | (unsigned long)property[3];

		TempLine_Reserved.base = VIRT_TO_BUS(__va(img_base_pa));
		TempLine_Reserved.size = size;
		
		IMGR_LOG(IMGR_LOG_LVL_INFO, "img buffer base va:%lx, size:%x, pa:%lx\n", img_base_va, size, img_base_pa);
		IMGR_LOG(IMGR_LOG_LVL_INFO, "Templine buf base is %x,size is %x\n", TempLine_Reserved.base,TempLine_Reserved.size);
	} else {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "can not find img buffer reserved memory node!!\r\n");
		return -1;
	}

	//added by mtk68119 end
	ret = misc_register(&imgresz_dev);
	

	if (ret) {
		IMGR_LOG(IMGR_LOG_LVL_ERR, "imgr_probe: misc_register error %d\r\n", ret);
	}

	MOD_VERSION_INFO(MMISC_MODE_NAME, MMISC_VER_MAJOR, MMISC_VER_MINOR, MMISC_VER_REV);

	i4ImgResz_Drv_Init();

	IMGR_LOG(IMGR_LOG_LVL_INFO, "success to probe Imgresz \r\n");
	return ret;
}

static int imgresz_remove(struct platform_device *pdev)
{
	of_reserved_mem_device_release(&(pdev->dev)); //added by mtk68119
	i4ImgResz_Drv_Uninit();

	misc_deregister(&(imgresz_dev));

	return 0;
}

static const struct of_device_id imgr_of_ids[] = {
	{.compatible = "Autochips,ac83xx-imgresz",},
	{}
};

static struct platform_driver imgr_plt_drv = {
	.driver = {
		.name = "ac83xx-imgresz",
		.owner = THIS_MODULE,
		.of_match_table = imgr_of_ids,
	},
	.probe = imgresz_probe,
	.remove = imgresz_remove,
};


static int __init imgresz_init(void)
{
	int ret;

	pr_info("imgresz_init--->\n");
	ret = platform_driver_register(&imgr_plt_drv);

	if (ret) {
		pr_err("[imgresz]: %s: register  driver failed\n", __func__);
	}

	return ret;

}

static void __exit imgresz_exit(void)
{
	pr_info("imgresz_exit--->\n");
	platform_driver_unregister(&imgr_plt_drv);
}
module_init(imgresz_init);
module_exit(imgresz_exit);


MODULE_AUTHOR("mtk68024");
MODULE_DESCRIPTION("Imgresz driver");
MODULE_LICENSE(ATC_KERNEL_LINUX_LICENSE); /*TODO:*/





