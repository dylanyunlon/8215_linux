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

#ifndef __IMGRESZ_INST_H
#define __IMGRESZ_INST_H

#include <linux/semaphore.h>
#include "drv_imgresz.h"
#include "imgresz_drv.h"
#include "chip_ver.h"

typedef struct _IMGRESZ_INST_T {
	u16                  u2ImgReszCompId;        /*/< the ImgResz Component Id*/
	IMGRESZ_DRV_SCALE_MODE      eImgReszScaleMd;        /*/< scale mode*/
	IMGRESZ_DRV_STATE           eImgReszState;          /*/< scale state*/
#if IMGRESZ_MEMORY_DYNAMIC_ALLOC
	IMGRESZ_DRV_SRC_BUF_INFO_T  *ptImgReszSrcBufInfo;     /*/< source buffer information*/
	IMGRESZ_DRV_DST_BUF_INFO_T  *ptImgReszDstBufInfo;     /*/< destination buffer information*/
	IMGRESZ_DRV_BLD_BUF_INFO_T  *ptImgReszBldBufInfo;     /*/< blending buffer information*/
	IMGRESZ_DRV_PARTIAL_INFO_T  *ptImgReszPartialBufInfo; /*/< Partial buffer information*/
	IMGRESZ_DRV_JPEG_INFO_T     *ptImgReszJpegInfo;       /*/< Jpeg information*/
	IMGRESZ_DRV_RM_INFO_T       *ptImgReszRmInfo;
#else
	IMGRESZ_DRV_SRC_BUF_INFO_T  tImgReszSrcBufInfo;     /*/< source buffer information*/
	IMGRESZ_DRV_DST_BUF_INFO_T  tImgReszDstBufInfo;     /*/< destination buffer information*/
	IMGRESZ_DRV_BLD_BUF_INFO_T  tImgReszBldBufInfo;     /*/< blending buffer information*/
	IMGRESZ_DRV_PARTIAL_INFO_T  tImgReszPartialBufInfo; /*/< Partial buffer information*/
	IMGRESZ_DRV_JPEG_INFO_T     tImgReszJpegInfo;       /*/< Jpeg information*/
	IMGRESZ_DRV_RM_INFO_T       tImgReszRmInfo;
#endif
	u32                  u4PrevRowBufSa1;        /*/< Previous partial row buffer 1 start address*/
	u32                  u4PrevRowBufSa2;        /*/< Previous partial row buffer 2 start address*/
	u32                  u4PrevRowBufSa3;        /*/< Previous partial row buffer 3 start address*/
	/*u32                  u4ImgReszComponents;    ///< current scale components*/
	/*u32                  u4ImgReszHwHandle;      ///< ImgResz HW Handle*/
	u32                  u4ImgReszPriority;      /*/< priority*/
	u32                  u4ImgReszCurrPriority;  /*/< current priority (priority will increase if not serviced)*/
	bool                    fgDoLock;               /*/< Ready to do lock*/
	bool                    fgLock;                 /*/< Lock hardware instance.*/
	u32                  u4HwInstId;             /*/< Locked hardware instance id.*/
	IMGRESZ_DRV_NOTIFY_CB_REG_T rNotifyCallbackReg; /*/< notify callback function*/
	u32                  u4TempLineBufSa;        /*/< Temporary line buffer start address.*/
	u32                  u4PartialRowCnt;        /*/< Partial mode row count.*/
	BYTE                    abColorPallet[256][4];  /*/< Color pallet*/
	bool                    fgLumaKeyEnable;        /*/< Luma key enalbe or not*/
	u8                   u1LumaKey;              /*/< Luma key value*/
	bool                    fg1To1Scale;            /*/< 1:1 Scale for venc path*/
	bool                    fgYSrcOnly;             /*/< Only has Y component in src/dst*/
	u32					u4tableaddr;			/*/mmu table addr from caller*/
} IMGRESZ_INST_T;
extern IMGRESZ_INST_T      _arImgreszInst[IMGRESZ_INST_NUM];
extern struct mutex g_ImgReszMutex;

extern void vImgResz_Inst_Init(void);
extern void vImgResz_Inst_Uninit(void);
extern IMGRESZ_DRV_STATE eImgResz_Inst_GetState(u32 u4InstId);
extern void vImgResz_Inst_SetState(u32 u4InstId, IMGRESZ_DRV_STATE eState);
extern s32 i4ImgResz_Inst_SetLock(u32 u4InstId, bool fgLock);
extern void vImgResz_Inst_IncPriority(void);
extern s32 i4ImgResz_Inst_GetInstanceObject(u32 u4InstId, IMGRESZ_INST_T **pprImgreszInst);
extern s32 i4ImgResz_Inst_GetInstance(u32 *pu4InstId);
extern s32 i4ImgResz_Inst_ReleaseInstance(u32 u4InstId);
extern void vImgResz_Inst_DispatchHw(u32 u4Ticket);
extern s32 i4ImgResz_Inst_GetUnservicedInstance(IMGRESZ_HW_INST_T *ptImgReszHwInst);
extern s32 i4ImgResz_Inst_SetServicingInstanceToUnserviced(IMGRESZ_HW_INST_T *ptImgReszHwInst);
extern void vImgResz_Inst_ReleaseServicedInstanceAndNotifyCallback(
			IMGRESZ_HW_INST_T *ptImgReszHwInst, bool fgCheckWaitStop,
			s32 i4NotifyCallbackState);
extern void vImgResz_Inst_NotifyCallback(IMGRESZ_HW_INST_T *ptImgReszHwInst, s32 i4NotifyCallbackState);
extern s32 i4ImgResz_Inst_GetLockInstance(IMGRESZ_HW_INST_T *ptImgReszHwInst, bool *pfgLock, u32 *pu4InstId);
extern s32 i4ImgResz_Inst_StopResize(u32 u4InstId);
extern s32 i4ImgResz_Inst_NotifyCallbackProc(u32 u4InstId, s32 i4State);
extern void vImgReszInstLog(u32 u4Event, u32 u4Value);

#endif /* __IMGRESZ_H*/


