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

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/semaphore.h>

#include "x_os.h"
#include "x_assert.h"
/*#include <windows.h>*/
#include "x_rtos.h"
#include "x_printf.h"
#include "x_debug.h"
#include "drv_imgresz_errcode.h"
#include "imgresz_drv.h"
#include "imgresz_drv_inst.h"
#include "imgresz_log.h"
#include "drv_config.h"
/*#include "x_kmem.h"*/

#include "sys_config.h"
#include "imgresz_hal_if.h"
/*-----------------------------------------------------------------------------
data declarations
----------------------------------------------------------------------------*/
#define IMGRESZ_INST_LOG_NUM 256

struct semaphore h_imgresz_sema;
u32              _u4ImgreszScaleCount = 0;
u32              _u4ImgreszUnserviceCount = 0;
IMGRESZ_INST_T      _arImgreszInst[IMGRESZ_INST_NUM] = {
	{0},
	{0},
	{0},
	{0},
	{0},
	{0},
	{0},
	{0},
	{0},
	{0},
};
u32 _u4ImgreszInstLogIdx = 0;
u32 _au4ImgreszInstLog[IMGRESZ_INST_LOG_NUM] = {0};

void vImgReszInstLog(u32 u4Event, u32 u4Value)
{
	_au4ImgreszInstLog[_u4ImgreszInstLogIdx] = (u4Event << 16) | u4Value;

	_u4ImgreszInstLogIdx++;

	if (_u4ImgreszInstLogIdx == IMGRESZ_INST_LOG_NUM) {
		_u4ImgreszInstLogIdx = 0;
	}
}


void vImgResz_Inst_Init(void)
{
	s32 i4;

	vImgreszSetMemory(_arImgreszInst, 0, sizeof(_arImgreszInst));

	for (i4 = 0; i4 < IMGRESZ_INST_NUM; i4++) {
		_arImgreszInst[i4].u2ImgReszCompId = (u16)i4;
		_arImgreszInst[i4].eImgReszScaleMd = IMGRESZ_DRV_NONE_SCALE;
		_arImgreszInst[i4].eImgReszState = IMGRESZ_DRV_STATE_NONE;
		_arImgreszInst[i4].u4ImgReszPriority = 0;
		/*_arImgreszInst[i4].u4ImgReszComponents = IMGRESZ_INVALID32;*/
		/*_arImgreszInst[i4].u4ImgReszHwHandle = IMGRESZ_INVALID32;*/
	}

	_u4ImgreszScaleCount = 0;
	_u4ImgreszUnserviceCount = 0;

	/* create a semaphore*/
	sema_init(&h_imgresz_sema, 1);
}


void vImgResz_Inst_Uninit(void)
{
	/* Delete semaphore*/
	IMGR_LOG(IMGR_LOG_LVL_DBG, "vImgResz_Inst_Uninit !!!\r\n");
}


IMGRESZ_DRV_STATE eImgResz_Inst_GetState(u32 u4InstId)
{
	return _arImgreszInst[u4InstId].eImgReszState;
}

void vImgResz_Inst_SetState(u32 u4InstId, IMGRESZ_DRV_STATE eState)
{
	_arImgreszInst[u4InstId].eImgReszState = eState;
}


s32 i4ImgResz_Inst_SetLock(u32 u4InstId, bool fgLock)
{
	s32 i4;
	bool fgDoLock = FALSE;

	if (_arImgreszInst[u4InstId].fgLock == fgLock) {
		return 0;
	}

	if (_arImgreszInst[u4InstId].u4ImgReszPriority == 0) {
		return -1;
	}

	/* Get semaphore*/
	down(&h_imgresz_sema);

	vImgReszInstLog(0, u4InstId);
	vImgReszInstLog(1, fgLock);

	if (fgLock) {
		if (_arImgreszInst[u4InstId].eImgReszScaleMd == IMGRESZ_DRV_JPEG_PIC_SCALE) {
			u32 u4TargetHwId = _arImgreszInst[u4InstId].tImgReszJpegInfo.u4HwId;
			s32 i4Temp = 0;

			fgDoLock = TRUE;


			for (i4 = 0; i4 < IMGRESZ_INST_NUM; i4++) {
				if (_arImgreszInst[i4].fgLock && (_arImgreszInst[i4].u4HwInstId == u4TargetHwId) &&
				    (_arImgreszInst[u4InstId].u4ImgReszPriority <=
						_arImgreszInst[i4].u4ImgReszPriority)) {
					fgDoLock = FALSE;
				}

				if (_arImgreszInst[i4].fgLock && (_arImgreszInst[i4].u4HwInstId == u4TargetHwId) &&
				    (_arImgreszInst[u4InstId].u4ImgReszPriority >
						_arImgreszInst[i4].u4ImgReszPriority)) {
					i4Temp = i4;
				}
			}

			if (fgDoLock) {
				_arImgreszInst[u4InstId].u4HwInstId = u4TargetHwId;
				_arImgreszInst[i4Temp].fgLock = FALSE;
			}
		} else if ((_arImgreszInst[u4InstId].tImgReszSrcBufInfo.fgWTEnable) ||
			 (_arImgreszInst[u4InstId].tImgReszDstBufInfo.fgWTEnable)) {
			u32 u4TargetHwId = 1;

			fgDoLock = TRUE;

			for (i4 = 0; i4 < IMGRESZ_INST_NUM; i4++) {
				if (_arImgreszInst[i4].fgLock && (_arImgreszInst[i4].u4HwInstId == u4TargetHwId) &&
				    (_arImgreszInst[u4InstId].u4ImgReszPriority <=
						_arImgreszInst[i4].u4ImgReszPriority)) {
					fgDoLock = FALSE;
				}
			}

			if (fgDoLock) {
				_arImgreszInst[u4InstId].u4HwInstId = u4TargetHwId;
			}
		} else {
			u32 u4LockedNum = 0;

			/* Reach max lock num ?*/
			for (i4 = 0; i4 < IMGRESZ_INST_NUM; i4++) {
				if (_arImgreszInst[i4].fgLock) {
					u4LockedNum++;
				}
			}

			if (u4LockedNum == IMGRESZ_MAX_LOCK_HW_INST_NUM) {
				for (i4 = 0; i4 < IMGRESZ_INST_NUM; i4++) {
					if (_arImgreszInst[i4].fgLock && (_arImgreszInst[u4InstId].u4ImgReszPriority >
							_arImgreszInst[i4].u4ImgReszPriority)) {
						vImgReszInstLog(2, i4);

						/* Break previous lock instance.*/
						_arImgreszInst[i4].fgLock = FALSE;
						fgDoLock = TRUE;
						break;
					}
				}
			} else {
				fgDoLock = TRUE;
			}

			if (fgDoLock) {
				/* Decide locked hardware instance ID*/
				u32 u4LockedHwInst = 0;
				u32 u4OrderdHwInst = 0;
				u32 u4HwInstNum = i4ImgReszHwInstGetNum();
				s32 i4NonlockAndNonorderdHwInstId = -1;
				s32 i4NonlockAndNonactiveHwInstId = -1;
				s32 i4NonlockAndActiveHwInstId = -1;

				for (i4 = 0; i4 < IMGRESZ_INST_NUM; i4++) {
					if (_arImgreszInst[i4].fgLock) {
						u4LockedHwInst |= (1 << _arImgreszInst[i4].u4HwInstId);
					} else if (eImgResz_Inst_GetState(i4) == IMGRESZ_DRV_STATE_START) {
						u4OrderdHwInst |= (1 << _arImgreszInst[i4].u4HwInstId);
					}
				}

				for (i4 = 0; i4 < (s32)u4HwInstNum; i4++) {
					if ((u4LockedHwInst & (1 << i4)) == 0) {
						if (fgImgReszHwInstIsActive(i4)) {
							if (i4NonlockAndActiveHwInstId == -1) {
								i4NonlockAndActiveHwInstId = i4;
							}
						} else if ((u4OrderdHwInst & (1 << i4)) == 0) {
							if (i4NonlockAndNonorderdHwInstId == -1) {
								i4NonlockAndNonorderdHwInstId = i4;
							}
						} else {
							if (i4NonlockAndNonactiveHwInstId == -1) {
								i4NonlockAndNonactiveHwInstId = i4;
							}
						}
					}
				}

				if (i4NonlockAndNonorderdHwInstId != -1) {
					vImgReszInstLog(3, i4NonlockAndNonorderdHwInstId);

					_arImgreszInst[u4InstId].u4HwInstId = i4NonlockAndNonorderdHwInstId;
				} else if (i4NonlockAndNonactiveHwInstId != -1) {
					vImgReszInstLog(4, i4NonlockAndNonactiveHwInstId);

					_arImgreszInst[u4InstId].u4HwInstId = i4NonlockAndNonactiveHwInstId;
				} else if (i4NonlockAndActiveHwInstId != -1) {
					vImgReszInstLog(5, i4NonlockAndActiveHwInstId);

					_arImgreszInst[u4InstId].u4HwInstId = i4NonlockAndActiveHwInstId;
				} else {
					/* Why do not find any hardware instance?*/
					VERIFY(FALSE);
				}
			}
		}

		if (fgDoLock) {
			_arImgreszInst[u4InstId].fgLock = fgLock;
			i4ImgReszHwInstLockNotify(_arImgreszInst[u4InstId].u4HwInstId, fgLock);

			for (i4 = 0; i4 < IMGRESZ_INST_NUM; i4++) {
				if (_arImgreszInst[i4].fgLock && (i4 != u4InstId)) {
					if (_arImgreszInst[i4].u4HwInstId == _arImgreszInst[u4InstId].u4HwInstId) {
						VERIFY(FALSE);
					}
				}
			}
		}
	} else {
		vImgReszInstLog(2, _arImgreszInst[u4InstId].u4HwInstId);

		/* Unlock*/
		_arImgreszInst[u4InstId].fgLock = fgLock;
		i4ImgReszHwInstLockNotify(_arImgreszInst[u4InstId].u4HwInstId, fgLock);
	}

	/* Release semaphore*/
	up(&h_imgresz_sema);

	if (fgLock && !fgDoLock) {
		return E_IMGRESZ_DRV_LOCK_FAIL;
	} else {
		return 0;
	}
}


void vImgResz_Inst_IncPriority(void)
{
	s32 i4 = 0;

	for (i4 = 0; i4 < IMGRESZ_INST_NUM; i4++) {
		if (eImgResz_Inst_GetState(i4) == IMGRESZ_DRV_STATE_START) {
			_arImgreszInst[i4].u4ImgReszCurrPriority++;
		}
	}
}


s32 i4ImgResz_Inst_GetInstanceObject(u32 u4InstId, IMGRESZ_INST_T **pprImgreszInst)
{
	*pprImgreszInst = &(_arImgreszInst[u4InstId]);

	return 0;
}


s32 i4ImgResz_Inst_GetInstance(u32 *pu4InstId)
{
	s32 i4Count = 0;

	/* Get semaphore*/
	down(&h_imgresz_sema);

	for (i4Count = 0; i4Count < IMGRESZ_INST_NUM; i4Count++) {
		if (eImgResz_Inst_GetState(i4Count) == IMGRESZ_DRV_STATE_NONE) {
			*pu4InstId = i4Count;
			vImgreszSetMemory(&(_arImgreszInst[*pu4InstId]), 0, sizeof(IMGRESZ_INST_T));
			_arImgreszInst[*pu4InstId].u2ImgReszCompId = (u16) * pu4InstId;
			_arImgreszInst[*pu4InstId].eImgReszScaleMd = IMGRESZ_DRV_NONE_SCALE;
			_arImgreszInst[*pu4InstId].u4ImgReszPriority = 0;

			_arImgreszInst[*pu4InstId].tImgReszSrcBufInfo.fgWTEnable = FALSE;
			_arImgreszInst[*pu4InstId].tImgReszDstBufInfo.fgWTEnable = FALSE;

			/*_arImgreszInst[*pu4InstId].u4ImgReszHwHandle = IMGRESZ_INVALID32;*/
			_arImgreszInst[*pu4InstId].tImgReszBldBufInfo.u1Alpha = 0xFF;
			_arImgreszInst[*pu4InstId].fg1To1Scale = FALSE;
			_arImgreszInst[*pu4InstId].fgYSrcOnly = FALSE;
			vImgResz_Inst_SetState(i4Count, IMGRESZ_DRV_STATE_IDLE);
			break;
		}
	}

	/* Release semaphore*/
	up(&h_imgresz_sema);

	if (i4Count == IMGRESZ_INST_NUM) {
		return -1;
	} else {
		return 0;
	}
}


s32 i4ImgResz_Inst_ReleaseInstance(u32 u4InstId)
{
	/* Get semaphore*/
	down(&h_imgresz_sema);

	switch (eImgResz_Inst_GetState(u4InstId)) {
	case IMGRESZ_DRV_STATE_START:
	case IMGRESZ_DRV_STATE_IDLE:
		vImgResz_Inst_SetState(u4InstId, IMGRESZ_DRV_STATE_NONE);
		break;

	case IMGRESZ_DRV_STATE_SCALING:
		/*break current scaling, use event*/
		ASSERT(0);
		/*vImgReszSetState(u4Ticket, IMGRESZ_DRV_STATE_NONE);*/
		break;

	default:
		ASSERT(0);
		break;
	}

	//if (_arImgreszInst[u4InstId].u4TempLineBufSa != 0) {
		//x_free_aligned_nc_mem((void *)_arImgreszInst[u4InstId].u4TempLineBufSa);
		//_arImgreszInst[u4InstId].u4TempLineBufSa = 0;
	//}
		memset(&(_arImgreszInst[u4InstId]),0,sizeof(_arImgreszInst[u4InstId]));
	/* Release semaphore*/
	up(&h_imgresz_sema);

	return 0;
}


void vImgResz_Inst_DispatchHw(u32 u4InstId)
{
	/* Get semaphore*/
	down(&h_imgresz_sema);

	vImgReszInstLog(0x10, u4InstId);

	_u4ImgreszScaleCount++;
	_u4ImgreszUnserviceCount++;
	vImgResz_Inst_SetState(u4InstId, IMGRESZ_DRV_STATE_START);

	if (_arImgreszInst[u4InstId].eImgReszScaleMd == IMGRESZ_DRV_JPEG_PIC_SCALE &&
			!_arImgreszInst[u4InstId].fgLock) {
		_arImgreszInst[u4InstId].u4HwInstId = _arImgreszInst[u4InstId].tImgReszJpegInfo.u4HwId;
		IMGR_LOG(IMGR_LOG_LVL_DBG,"enter jpeg fromat set event do scale\n");
		vImgReszHwInstScaleNotify(TRUE, &(_arImgreszInst[u4InstId].u4HwInstId));
	}

	/* Add for WT*/
	else if (_arImgreszInst[u4InstId].tImgReszSrcBufInfo.fgWTEnable ||
			(_arImgreszInst[u4InstId].tImgReszDstBufInfo.fgWTEnable)) {
		_arImgreszInst[u4InstId].u4HwInstId = 1;
		vImgReszHwInstScaleNotify(TRUE, &(_arImgreszInst[u4InstId].u4HwInstId));
	} else {
		vImgReszHwInstScaleNotify(_arImgreszInst[u4InstId].fgLock, &(_arImgreszInst[u4InstId].u4HwInstId));
	}

	/* Release semaphore*/
	up(&h_imgresz_sema);
}


s32 i4ImgResz_Inst_GetUnservicedInstance(IMGRESZ_HW_INST_T *ptImgReszHwInst)
{
	s32 i4, i4RetValue = 0;
	s32 i4ImgReszInstId = -1;
	u32 u4PriorityTmp = 0;

	/* Get semaphore*/
	down(&h_imgresz_sema);

	vImgReszInstLog(0x20, ptImgReszHwInst->u2ImgReszHwId);

	if (_u4ImgreszUnserviceCount > 0) {
		vImgReszInstLog(0x21, ptImgReszHwInst->fgLock);

		if (ptImgReszHwInst->fgLock) {
			if (ptImgReszHwInst->u2ImgReszCompId > (IMGRESZ_INST_NUM - 1)) {
				ASSERT(0);
			}

			if (eImgResz_Inst_GetState(ptImgReszHwInst->u2ImgReszCompId) == IMGRESZ_DRV_STATE_START) {
				vImgReszInstLog(0x22, ptImgReszHwInst->u2ImgReszCompId);

				_u4ImgreszUnserviceCount--;
				ptImgReszHwInst->fgImgReszActive = TRUE;
				vImgResz_Inst_SetState(ptImgReszHwInst->u2ImgReszCompId, IMGRESZ_DRV_STATE_SCALING);
			} else {
				vImgReszInstLog(0x23, ptImgReszHwInst->u2ImgReszCompId);

				i4RetValue = -1;
			}
		} else {
			if (ptImgReszHwInst->u2ImgReszHwId == 0) {
				/* skip some instance with WT enalbe*/
				for (i4 = 0; i4 < IMGRESZ_INST_NUM; i4++) {
					if ((eImgResz_Inst_GetState(i4) == IMGRESZ_DRV_STATE_START) &&
							!_arImgreszInst[i4].fgLock) {
						if ((u4PriorityTmp < _arImgreszInst[i4].u4ImgReszCurrPriority) &&
						    ((!_arImgreszInst[i4].tImgReszSrcBufInfo.fgWTEnable) &&
						     (!_arImgreszInst[i4].tImgReszDstBufInfo.fgWTEnable))) {
							if ((_arImgreszInst[i4].eImgReszScaleMd != IMGRESZ_DRV_JPEG_PIC_SCALE) ||
							    (_arImgreszInst[i4].u4HwInstId == ptImgReszHwInst->u2ImgReszHwId)) {
								u4PriorityTmp = _arImgreszInst[i4].u4ImgReszCurrPriority;
								i4ImgReszInstId = i4;
							}
						}
					}
				}
			} else if (ptImgReszHwInst->u2ImgReszHwId == 1) {
				/* Search instance to do scale*/
				for (i4 = 0; i4 < IMGRESZ_INST_NUM; i4++) {
					if ((eImgResz_Inst_GetState(i4) == IMGRESZ_DRV_STATE_START) &&
							!_arImgreszInst[i4].fgLock) {
						if (u4PriorityTmp < _arImgreszInst[i4].u4ImgReszCurrPriority) {
							if ((_arImgreszInst[i4].eImgReszScaleMd != IMGRESZ_DRV_JPEG_PIC_SCALE) ||
							    (_arImgreszInst[i4].u4HwInstId == ptImgReszHwInst->u2ImgReszHwId)) {
								u4PriorityTmp = _arImgreszInst[i4].u4ImgReszCurrPriority;
								i4ImgReszInstId = i4;
							}
						}
					}
				}
			}

			if (i4ImgReszInstId == -1) {
				vImgReszInstLog(0x24, ptImgReszHwInst->u2ImgReszCompId);

				/* Do nothing, because the unserviced instance is locked.*/
				i4RetValue = -1;
			} else {
				vImgReszInstLog(0x25, i4ImgReszInstId);

				_u4ImgreszUnserviceCount--;
				ptImgReszHwInst->fgImgReszActive = TRUE;

				if (i4ImgReszInstId > (IMGRESZ_INST_NUM - 1)) {
					ASSERT(0);
				}

				_arImgreszInst[i4ImgReszInstId].u4ImgReszCurrPriority =
						_arImgreszInst[i4ImgReszInstId].u4ImgReszPriority;
				ptImgReszHwInst->u2ImgReszCompId = (u16)i4ImgReszInstId;
				_arImgreszInst[i4ImgReszInstId].u4HwInstId = (u32)ptImgReszHwInst->u2ImgReszHwId;
				vImgResz_Inst_SetState(i4ImgReszInstId, IMGRESZ_DRV_STATE_SCALING);
			}
		}
	} else {
		vImgReszInstLog(0x26, ptImgReszHwInst->u2ImgReszHwId);
		i4RetValue = -1;
	}

	/* Release semaphore*/
	up(&h_imgresz_sema);

	return i4RetValue;
}


s32 i4ImgResz_Inst_SetServicingInstanceToUnserviced(IMGRESZ_HW_INST_T *ptImgReszHwInst)
{
	u32 u4InstId = ptImgReszHwInst->u2ImgReszCompId;

	/* Get semaphore*/
	down(&h_imgresz_sema);

	if (u4InstId > (IMGRESZ_INST_NUM - 1)) {
		ASSERT(0);
	}

	if (eImgResz_Inst_GetState(u4InstId) == IMGRESZ_DRV_STATE_SCALING) {
		_u4ImgreszUnserviceCount++;
		vImgResz_Inst_SetState(u4InstId, IMGRESZ_DRV_STATE_START);
	} else {
		VERIFY(FALSE);
	}

	/* Release semaphore*/
	up(&h_imgresz_sema);

	return 0;
}


void vImgResz_Inst_ReleaseServicedInstanceAndNotifyCallback(IMGRESZ_HW_INST_T *ptImgReszHwInst, bool fgCheckWaitStop,
							    s32 i4NotifyCallbackState)
{
	s32 i4Ret = 0;

	/* Get semaphore*/
	down(&h_imgresz_sema);

	vImgReszInstLog(0x30, ptImgReszHwInst->u2ImgReszHwId);
	vImgReszInstLog(0x31, ptImgReszHwInst->fgLock);
	vImgReszInstLog(0x32, ptImgReszHwInst->u2ImgReszCompId);

	if (ptImgReszHwInst->u2ImgReszCompId > (IMGRESZ_INST_NUM - 1)) {
		ASSERT(0);
	}

	if (fgCheckWaitStop && (eImgResz_Inst_GetState(ptImgReszHwInst->u2ImgReszCompId) ==
			IMGRESZ_DRV_STATE_WAIT_STOP)) {
		/* When decode finish, check wait lock.*/
		/* If wait lock, then wait lock event.*/
	} else {
		_u4ImgreszScaleCount--;
		ptImgReszHwInst->fgImgReszActive = FALSE;
		/*IMGR_LOG(IMGR_LOG_LVL_ERR, "vImgResz_Inst_ReleaseServicedInstanceAndNotifyCallback set
			inst%d state IDLE \r\n",ptImgReszHwInst->u2ImgReszCompId);*/
		vImgResz_Inst_SetState((u32)(ptImgReszHwInst->u2ImgReszCompId), IMGRESZ_DRV_STATE_IDLE);

		/* Notify callback*/
		i4ImgResz_Inst_NotifyCallbackProc(ptImgReszHwInst->u2ImgReszCompId, i4NotifyCallbackState);

		if ((_u4ImgreszUnserviceCount > 0) && !ptImgReszHwInst->fgLock) {
			/*ptImgReszHwInst->fgImgReszActive = TRUE;*/
			/*sent a event to related HW instance*/
			i4Ret = x_ev_group_set_event(ptImgReszHwInst->hEventHandle, IMGRESZ_EV_DO_SCALE, X_EV_OP_OR);
			VERIFY(i4Ret == OSR_OK);
		}
	}

	/* Release semaphore*/
	up(&h_imgresz_sema);
}


void vImgResz_Inst_NotifyCallback(IMGRESZ_HW_INST_T *ptImgReszHwInst, s32 i4NotifyCallbackState)
{
	/* Get semaphore*/
	down(&h_imgresz_sema);

	vImgReszInstLog(0x60, ptImgReszHwInst->u2ImgReszHwId);
	vImgReszInstLog(0x61, ptImgReszHwInst->fgLock);
	vImgReszInstLog(0x62, ptImgReszHwInst->u2ImgReszCompId);

	/* Notify callback*/
	if (ptImgReszHwInst->u2ImgReszCompId > ((IMGRESZ_INST_NUM - 1))) {
		ASSERT(0);
	}

	i4ImgResz_Inst_NotifyCallbackProc(ptImgReszHwInst->u2ImgReszCompId, i4NotifyCallbackState);

	/* Release semaphore*/
	up(&h_imgresz_sema);
}


s32 i4ImgResz_Inst_GetLockInstance(IMGRESZ_HW_INST_T *ptImgReszHwInst, bool *pfgLock, u32 *pu4InstId)
{
	s32 i4;

	/* Get semaphore*/
	down(&h_imgresz_sema);

	vImgReszInstLog(0x40, ptImgReszHwInst->u2ImgReszHwId);

	/* Search lock instance*/
	for (i4 = 0; i4 < IMGRESZ_INST_NUM; i4++) {
		if (_arImgreszInst[i4].fgLock && (_arImgreszInst[i4].u4HwInstId == ptImgReszHwInst->u2ImgReszHwId)) {
			break;
		}
	}

	if (i4 == IMGRESZ_INST_NUM) { /* Get lock instance fail.*/
		vImgReszInstLog(0x42, ptImgReszHwInst->u2ImgReszHwId);

		*pfgLock = FALSE;
	} else { /* Lock instance found.*/
		vImgReszInstLog(0x41, i4);

		*pu4InstId = i4;
		*pfgLock = TRUE;
	}

	/* Release semaphore*/
	up(&h_imgresz_sema);

	return 0;
}


s32 i4ImgResz_Inst_StopResize(u32 u4InstId)
{
	/* Get semaphore*/
	down(&h_imgresz_sema);

	vImgReszInstLog(0x50, u4InstId);
	vImgReszInstLog(0x51, (u32)eImgResz_Inst_GetState(u4InstId));
	vImgReszInstLog(0x52, _arImgreszInst[u4InstId].u4HwInstId);

	switch (eImgResz_Inst_GetState(u4InstId)) {
	case IMGRESZ_DRV_STATE_START:
		IMGR_LOG(IMGR_LOG_LVL_DBG, "i4ImgResz_Drv_StopScale IMGRESZ_DRV_STATE_START !!!\r\n");
		vImgResz_Inst_SetState(u4InstId, IMGRESZ_DRV_STATE_IDLE);
		i4ImgResz_Inst_NotifyCallbackProc(u4InstId, S_IMGRESZ_DRV_RESIZE_STOP);
		break;

	case IMGRESZ_DRV_STATE_IDLE:
		IMGR_LOG(IMGR_LOG_LVL_DBG, "i4ImgResz_Drv_StopScale IMGRESZ_DRV_STATE_IDLE !!!\r\n");
		i4ImgResz_Inst_NotifyCallbackProc(u4InstId, S_IMGRESZ_DRV_RESIZE_STOP);
		break;

	case IMGRESZ_DRV_STATE_SCALING:
		IMGR_LOG(IMGR_LOG_LVL_DBG, "i4ImgResz_Drv_StopScale IMGRESZ_DRV_STATE_SCALING !!!\r\n");
		/*IMGR_LOG(IMGR_LOG_LVL_ERR, "Set inst%d to IMGRESZ_DRV_STATE_WAIT_STOP \r\n",u4InstId);*/
		vImgResz_Inst_SetState(u4InstId, IMGRESZ_DRV_STATE_WAIT_STOP);
		i4ImgReszHwInstStopNotify(_arImgreszInst[u4InstId].u4HwInstId);
		break;

	default:
		//ASSERT(0);
		break;
	}

	/* Release semaphore*/
	up(&h_imgresz_sema);

	return S_IMGRESZ_DRV_OK;
}


s32 i4ImgResz_Inst_NotifyCallbackProc(u32 u4InstId, s32 i4State)
{
	vImgReszInstLog(0x70, u4InstId);
	vImgReszInstLog(0x71, i4State);

	if (_arImgreszInst[u4InstId].rNotifyCallbackReg.pvCallBackFunc != NULL)
		_arImgreszInst[u4InstId].rNotifyCallbackReg.pvCallBackFunc(i4State,
			_arImgreszInst[u4InstId].rNotifyCallbackReg.pvPrivData);

	return 0;
}


