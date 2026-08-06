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

#ifndef _DRV_STC_H_
#define _DRV_STC_H_

#include "x_typedef.h"

extern void vSTC_SetStc(UINT64 u8Stc);
extern UINT64 u8STC_GetStc(void);

#if 1   /* Most of the following should be removed in MT8520 */

/* STC ID */
#define STC_SRC_A1					0
#define STC_SRC_A2					1
#define STC_SRC_V1					2
#define STC_SRC_V2					3
#define STC_SRC_NS					4

/* AV SYNC Mode */
#define AV_SYNC_MODE_NONE			0
#define AV_SYNC_MODE_SLAVE			1
#define AV_SYNC_MODE_MASTER			2
#define AV_SYNC_MODE_NS				3

/* AV Sync Target */
#define AV_SYNC_TARGET_AUD			0
#define AV_SYNC_TARGET_VDO			1
#define AV_SYNC_TARGET_NS			2

/* MAX Device ID */
#define MAX_DEVICE_ID				0xFF

/* STC API Return Value */
#define STC_SET_OK					0
#define STC_SET_FAIL				-1
#define STC_VALID					0
#define STC_INVALID					-1

/* STC Time Structure */
typedef struct _STC_CLOCK
{
	UINT64			u8Base;  
	UCHAR			  ucBaseHi;
	UINT16			u2Ext;
} STC_CLOCK;

/******************************************************************************
* STC API
******************************************************************************/
extern INT32 DMX_SetStcSrc(UCHAR ucPidIdx, UCHAR ucStcId);
extern INT32 STC_GetSrc(UCHAR ucStcId, STC_CLOCK* prStc);
extern INT32 STC_SetSrc(UCHAR ucStcId, STC_CLOCK* prStc);

extern INT32 STC_SetSyncTarget(UCHAR ucType, UCHAR ucStcId, UCHAR ucTargetId);
extern UCHAR STC_GetSyncTarget(UCHAR ucType, UCHAR ucStcId);
extern INT32 STC_SetStartPts(UCHAR ucType, UCHAR ucStcId, UINT32 u4Pts);

extern INT32 STC_SetPtsDrift(UCHAR ucStcId, INT32 i4Drift);
extern INT32 STC_GetPtsDrift(UCHAR ucStcId);

extern INT32 STC_GetPtsDriftByAudDeviceId(UCHAR ucDevId);
extern INT32 STC_GetPtsDriftByVdoDeviceId(UCHAR ucDevId);

extern void STC_RxEsPkt(UCHAR ucType, UCHAR ucDevId, UINT32 u4Pts);
extern UINT32 STC_GetFastChangeChangeAhead(UCHAR ucStcId);

#define STC_SetAudDeviceId(x, y)		((void)0)
#define STC_SetVdoDeviceId(x, y)		((void)0)
 /* marked by Victor Lin, 20070828, for remove 5351 parser driver	
#define STC_GetVdoDeviceId(x)			STC_GetSyncTarget(AV_SYNC_TARGET_VDO, x)
*/
#endif /* #if 1 */

#endif /* _DRV_STC_H_ */

