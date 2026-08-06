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

#ifndef __IMGRESZ_H
#define __IMGRESZ_H

#include "x_common.h"
#include "drv_imgresz.h"
#include "drv_config.h"

/*Some new features switch for MT8560*/
#define CONFIG_SYS_MEM_PHASE2   0
#define CONFIG_SYS_MEM_PHASE3   0
#define CONFIG_SUSPEND_TO_DRAM  0


/* Work around for hardware destination width can not reach 4096 limit*/

/********************************
*
* mtk40136 marked to  further modify
*
* Remark: there are physical address and virtual address issues
*/
#define IMGRESZ_HW_DEST_WIDTH_4096_LIMIT 0
#define IMGRESZ_HW_SOURCE_WIDTH_2_LIMIT 0
#define IMGRESZ_HW_SOURCE_HEIGHT_2_LIMIT 0
#define IMGRESZ_HW_TARGET_HEIGHT_2_LIMIT 0

/*----------------------------------------------------------*/


#if !CONFIG_DRV_VERIFY_SUPPORT
#define IMGRESZ_INST_NUM        10
#else
#define IMGRESZ_INST_NUM        0
#endif
#define IMGRESZ_HW_INST_NUM     2
#define IMGRESZ_MAX_LOCK_HW_INST_NUM 1

#define IMGRESZ_MEMORY_DYNAMIC_ALLOC 0

/*! @name Image Resizer Event Group Define */
/*! @{ */

#define IMGRESZ_EV_INITIAL          ((EV_GRP_EVENT_T) 0)
#define IMGRESZ_EV_FINISH           ((EV_GRP_EVENT_T)(1) << 0)
#define IMGRESZ_EV_DO_SCALE         ((EV_GRP_EVENT_T)(1) << 4)
#define IMGRESZ_EV_STOP_SCALE       ((EV_GRP_EVENT_T)(1) << 2)
#define IMGRESZ_EV_FINISH_SCALE     ((EV_GRP_EVENT_T)(1) << 3)
#define IMGRESZ_EV_LOCK             ((EV_GRP_EVENT_T)(1) << 1)
#define IMGRESZ_EV_UNLOCK           ((EV_GRP_EVENT_T)(1) << 5)

/*! @} */

#define IMGRESZ_INVALID64   0xFFFFFFFFFFFFFFFF
#define IMGRESZ_INVALID32   0xFFFFFFFF
#define IMGRESZ_INVALID16   0xFFFF
#define IMGRESZ_INVALID8    0xFF
#if CONFIG_SUSPEND_TO_DRAM
struct pm_operations {
	int (*suspend)(void *param);
	int (*resume)(void *param);
};


void vPmInit(int argc, void *argv);
void vPmUninit(int argc, void *argv);
int register_pm_ops(struct pm_operations *ops);
int unregister_pm_ops(struct pm_operations *ops);
#endif


#if IMGRESZ_HW_DEST_WIDTH_4096_LIMIT
typedef struct _IMGRESZ_YCBCR_TO_AYUV_WORK_AROUND {
	BOOL                    fgYCbCrToAYUVWorkAround;
	BOOL                    fgCbTo444;
	BOOL                    fgCrTo444;
	u32                  u4Cb444Sa;
	u32                  u4Cr444Sa;
	BOOL                    fgCbTo444Done;
	BOOL                    fgCrTo444Done;
	u32                  u4LineCnt;
	/*IMGRESZ_INST_T          rImgreszInst;*/
} IMGRESZ_YCBCR_TO_AYUV_WORK_AROUND;


typedef struct _IMGRESZ_AYUV_TO_ARGB_WORK_AROUND {
	BOOL                    fgAYUVToARGBWorkAround;
	u32                  u4Step;
	u32                  u4TotalStep;
} IMGRESZ_AYUV_TO_ARGB_WORK_AROUND;
#endif


#if IMGRESZ_HW_SOURCE_HEIGHT_2_LIMIT
typedef struct _IMGRESZ_SORUCE_HEIGHT_2_WORK_AROUND {
	u32                  u4SA1;
	u32                  u4SA2;
	u32                  u4SA3;
} IMGRESZ_SORUCE_HEIGHT_2_WORK_AROUND;
#endif

#if IMGRESZ_HW_TARGET_HEIGHT_2_LIMIT
typedef struct _IMGRESZ_TARGET_HEIGHT_2_WORK_AROUND {
	u32                  u4SA1;
	u32                  u4SA2;
	u32                  u4SA3;
} IMGRESZ_TARGET_HEIGHT_2_WORK_AROUND;
#endif


typedef struct _IMGRESZ_HW_INST_T {
	u16                     u2ImgReszHwId;          /*/< Hw id*/
	u16                     u2ImgReszCompId;        /*/< HW is working in which component*/
	HANDLE_T                  hEventHandle;           /*/< Event Handle*/
	BOOL                    fgThreadFinish;         /*/< Thread finish*/
	BOOL                    fgWaitThreadFinish;     /*/< Wait thread finish*/
	BOOL                    fgImgReszActive;        /*/< HW is in active state*/
	BOOL                    fgInterlaced;           /*/< Interlaced scaling mode.*/
	BOOL                    fgCurrTopField;         /*/< Current scale top field.*/
	BOOL                    fgLock;                 /*/< Locked*/
	BOOL                    fgWaitLock;             /*/< Wait to be locked.*/
	u32                  u4WaitLockInstId;       /*/< Wait to be locked instance id.*/
#if IMGRESZ_HW_DEST_WIDTH_4096_LIMIT
	IMGRESZ_YCBCR_TO_AYUV_WORK_AROUND rYCbCrToAYUVWorkAround; /*/< YCbCr to AYUV work around*/
	IMGRESZ_AYUV_TO_ARGB_WORK_AROUND rAYUVToARGBWorkAround;   /*/< AYUV to ARGB work around*/
#endif
#if IMGRESZ_HW_SOURCE_HEIGHT_2_LIMIT
	IMGRESZ_SORUCE_HEIGHT_2_WORK_AROUND rSourceHeight2WorkAround;  /*/ Source height 2 limit work around*/
#endif

#if IMGRESZ_HW_TARGET_HEIGHT_2_LIMIT
	IMGRESZ_TARGET_HEIGHT_2_WORK_AROUND rTargetHeight2WorkAround; /*/ Target height 2 limit work around*/
#endif
} IMGRESZ_HW_INST_T;

#endif /* __IMGRESZ_H*/


