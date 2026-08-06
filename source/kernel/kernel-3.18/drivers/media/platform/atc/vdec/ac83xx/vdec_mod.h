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

#ifndef _VDEC_MOD_H
#define _VDEC_MOD_H

#include <linux/miscdevice.h>
#include <linux/clk.h>
#include <linux/clk-private.h>
#include <media/atc/vdec_init.h>

#define VIDEODECODER_DEVNAME "vdec"

// macro define
#define VDEC_MODE_NAME                   "VDEC"
#define VDEC_VER_MAJOR                   02
#define VDEC_VER_MINOR                   01
#define VDEC_VER_REV                     00

#define VDECRM_DRV_EV_SCALE_DONE           (1<<0)
#define VDECRM_DRV_EV_SCALE_ABORT         (1<<1)
#define VDECRM_DRV_EV_SCALE_ERR              (1<<2)
#define VDECRM_DRV_EV_SCALE_READY         (1<<3)
#define VDECRM_DRV_EV_SCALE_TIMEOUT         (1<<4)

#define  VDEC_EVENT_DEC_END  (1 << 2)

/* ------------------ */

typedef enum _VDEC_PM_STATE {
	INVALIED_PM_STATE = 0,
	PM_POWER_IDLE,
	PM_POWER_ON,
	PM_POWER_OFF,
} VDEC_PM_STATE;

#define VALID_DX(state) (((state) == PM_POWER_ON) || ((state) == PM_POWER_OFF))

// sturct
struct vdec_dev_info {
	struct miscdevice cdev;	/* Char device structure */
	struct device *dev;
	s32 vdful_irq;
	struct clk *vdec_topselect_clk_parent;
	struct clk *vdec_topselect_clk;
	struct clk *vdec_full_clk;
};

typedef struct _VDEC_DRV_INST{
    u32    u4HwId;
}VDEC_DRV_INST;

// interface
extern void ac83xx_mask_ack_bim_irq(u32 irq);



#endif //_VDEC_MOD_H
