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
#ifndef ___BDP_DRv_IF_VDP_H_
#define ___BDP_DRv_IF_VDP_H_

#include <media/atc/drv_vdec.h>
#include "drv_if_syncctrl.h"
/*#include "drv_if_pmx.h"*/

#define STC_VALID_WITH_PTS

typedef enum {
	SYNC_VIDEO_SLAVE = 0,
	SYNC_VIDEO_MASTER,
} SYNC_MASTER_MODE_T;

typedef enum {
	DRV_PMX_CS_YCBCR_601 = 0,
	DRV_PMX_CS_YCBCR_709,
	DRV_PMX_CS_RGB
} DRV_PMX_COLOR_SPACE_T;

#define PTS_A_EQUAL_TO_B        0
#define PTS_A_GREATER_THEN_B    1
#define PTS_B_GREATER_THEN_A    (-1)

typedef struct {
	__u32(*pu4AssignSyncCtrl)(void *pvUserPrivate, __u16 eType, __u16 u2Id);
	__u32(*pu4StcIsValid)(void *pvUserPrivate, SYNC_MASTER_MODE_T e_mode, __u64 ValidPts);
	__u32(*pu4SetFrameAccurateStartPts)(void *pvUserPrivate, __u64 u8Pts);
	__u32(*pu4SetFrameAccurateEndPts)(void *pvUserPrivate, __u64 u8Pts);

	__u32(*pu4Flush)(void *pvUserPrivate);
#if UNIFORM_DRV_CALLBACK
	__u32(*pu4AssignVdecCallback)(void *pvUserPrivate, const DRV_CB_REG_INFO_T * pCallback);
#else
	__u32(*pu4AssignVdecCallback)(void *pvUserPrivate, const VID_DEC_NFY_INFO_T * pCallback);
#endif
	__u32(*pu4DesiredResolution)(void *pvUserPrivate, VDSCL_INFO_T * prVdesclInfo);
	__u32(*pu4SetColorMode)(void *pvUserPrivate, DRV_PMX_COLOR_SPACE_T eColorMode);
	__u32(*pu4GetCurrPictureInfo)(void *pvUserPrivate, __u64 * pPts, __u8 * pbIsOpenB);
	__u32(*pu4PictureSent)(void *pvUserPrivate);
} IVdp;

__u32 u4VDP_AssignSyncCtrl(void *pvUserPrivate, __u16 eType, __u16 u2Id);
__u32 u4VDP_StcIsValid(void *pvUserPrivate, SYNC_MASTER_MODE_T e_mode, __u64 ValidPts);
__u32 u4VDP_Flush(void *pvUserPrivate);
#if UNIFORM_DRV_CALLBACK
__u32 u4VDP_AssignVdecCallback(void *pvUserPrivate, const DRV_CB_REG_INFO_T *pCallback);
#else
__u32 u4VDP_AssignVdecCallback(void *pvUserPrivate, const VID_DEC_NFY_INFO_T *pCallback);
#endif
__u32 u4VDP_DesiredResolution(void *pvUserPrivate, VDSCL_INFO_T *prVdesclInfo);
__u32 u4VDP_SetFrameAccurateStartPts(void *pvUserPrivate, __u64 u8Pts);
__u32 u4VDP_SetFrameAccurateEndPts(void *pvUserPrivate, __u64 u8Pts);
__u32 u4VDP_SetColorMode(void *pvUserPrivate, DRV_PMX_COLOR_SPACE_T eColorMode);
__u32 u4VDP_GetCurrPictureInfo(void *pthis, __u64 *pPts, __u8 *pbIsOpenB);
__u32 u4VDP_PictureSent(void *pthis);

void   vVDP_InbandCallback(__s32 i4IBCId, void *pthis);

#endif


