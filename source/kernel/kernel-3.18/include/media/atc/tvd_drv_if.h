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
#ifndef TVD_DRV_IF_H
#define TVD_DRV_IF_H
#ifdef  __ARM2__
#include "x_types.h"
#else
#include <linux/types.h>
#include <linux/irqreturn.h>
#endif
//#include "tvd_cfg.h"
#include "tvd_data_struct.h"
//#include "tvd_internal_data_struct.h"



/*  TVD Driver control code    */
#define TVD_CONTROL_CODE_INIT          0x0001U
#define TVD_CONTROL_CODE_CONFIG        0x0002U
#define TVD_CONTROL_CODE_START         0x0004U
#define TVD_CONTROL_CODE_STOP          0x0008U
#ifdef INIT_TVD_BEFORE_ARM2_START
#define OPTIMIZATION_TVD               0x0009U
#endif



#if defined(__ARM2__)
#define TVD_CONTROL_CODE_SETMODE       		0x0010
#define TVD_ATV_SIGNAL_FLAG            		0x00100000
#define TVD_DBG_CTRL_CODE_CVBS_SIG_STAT     0x0002
#endif


#define TVD_CONTROL_CODE_INIT_EX       0x0080
#define GET_DI_FLAG                    0x0800
#define GET_SIGNAL_STATE               0x0100
#define TVD_CONTROL_CODE_GET_INFO      0x8000




#define TVD_DI_ON_FLAG              (0x01000000)
#define TVD_SIG_ON_FLAG             (0X10000000)
#define TVD_SIG_OFF_FLAG            (0X20000000)
#define TVD_MODE_CHANGE_START_FLAG  (0X80000000)
#define TVD_MODE_CHANGE_DONE_FLAG   (0X40000000)
#define TVD_SIG_STATE_MASK     		(0XF0000000)
#define TVD_IDX_FLAG_MASK          	(0xf)









typedef enum {
	TVD_SUSPEND_STATE = 0,
	TVD_IDLE_STATE,
	TVD_STANDBY_STATE,
	TVD_INIT_STATE,
	TVD_CONFIG_STATE,
	TVD_RUN_STATE,
	TVD_PAUSE_STATE,
	TVD_STOP_STATE,
	TVD_STATE_MAX
} TVD_OPERATION_STATE_ENUM;








#if defined(__ARM2__)
#define TVD_EVT_NAME_SIG_STATE_BC      "TVDSIGSTATE_BC"
#elif defined(__UBOOT__)
/*for uboot*/
#else
/*for arm1*/
#ifdef __linux__
#define TVD_EVT_NAME_SIG_STATE_AVIN      "TVDSIGSTATE_VIN"
/*#define TVD_EVT_NAME_FRAME_DONE_AVIN     "TVDFRMDONE_VIN" */
#define TVD_EVT_NAME_SIG_STATE_BC        "TVDSIGSTATE_BC"
/*#define TVD_EVT_NAME_FRAME_DONE_BC       "TVDFRMDONE_BC"  */
/*#define TVD_EVT_NAME_FRAME_DONE_SYNC     "TVDFRMDONE_SYNC"*/
/*Add for linux event api bug*/

/*#define TVD_EVT_NAME_SIG_STATE_AVIN1      "TVDSIGSTATE_VIN1" */
/*#define TVD_EVT_NAME_FRAME_DONE_AVIN1     "TVDFRMDONE_VIN1" */
/*#define TVD_EVT_NAME_SIG_STATE_BC1        "TVDSIGSTATE_BC1"  */
/*#define TVD_EVT_NAME_FRAME_DONE_BC1       "TVDFRMDONE_BC1"  */
#else
/*#define TVD_EVT_NAME_IMG_LOSS       _T("LossImgSig") // Loss image signal*/
/*#define TVD_EVT_NAME_IMG_GET        _T("GetImgSig")  // Get image signal*/
/*#define TVD_EVT_NAME_SYS_CHANGE     _T("ImgSysChange") // image system change (PAL <-> NTSC <-> SECAM)*/

#define TVD_EVT_NAME_SIG_STATE_AVIN      "TVDSIGSTATE_VIN"
/*#define TVD_EVT_NAME_FRAME_DONE_AVIN     _T("TVDFRMDONE_VIN") */
#define TVD_EVT_NAME_SIG_STATE_BC      "TVDSIGSTATE_BC"
/*#define TVD_EVT_NAME_FRAME_DONE_BC     _T("TVDFRMDONE_BC") */
#endif

#endif

#ifdef __ARM2__
typedef void (*PTVD_IRQ_PROCE) (s32  irq);
#else
typedef irqreturn_t  (*PTVD_IRQ_PROCE)(s32 irq, void *dev_id);
#endif

extern u32 tvd_irq;
extern u32 tvd_internal_ioctl(u32 cmd, void *in_buf, void *out_buf);
extern s8  tvd_register_notify(tvd_notify notify_fun, void *arg);
extern void tvd_unregister_notify(void);
u32 tvd_power_mgr(u32 u4PowerStateValue);


//#endif
extern bool   gfgNeedTurnClamp;


#endif






