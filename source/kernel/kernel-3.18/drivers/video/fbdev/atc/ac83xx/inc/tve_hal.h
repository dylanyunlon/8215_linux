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
       
#ifndef _TVE_HAL_H_
#define _TVE_HAL_H_

#define TVE_DVP                      (0)
#define TVE_AP                       (1)
#define TVE_TVD                      (2)
#define TVE_NONE                     (3)

extern void vTveHalClockEn(void);
extern void vTveHalInit(void);
extern void vTveHalEnable(__u32 u4TveSource);
extern void vTveHalDisable(void);
extern void tve_suspend(void);
extern void tve_resume(void);
extern void vTveHalSetMode(__u32 u4Fmt);
extern void vTveHalEnableCB(__u32 u4CBType);
extern void vTveHalDisableCB(void);
extern void vTveHalMixPlane(__u32 u4Plane);
extern void vTveHalNotMixPlane(__u32 u4Plane);
extern __u32 dwTveHalGetTveSrc(void);
extern bool IsTVE_SRC_AP(void);
extern bool fgTveHalGetEn(void);
extern void vTveHalSetMv(__u32 dwType);
extern void vTveHalSetVbi(__u32 dRegVal);
extern __u32 dwTveGet525VbiData(void);
extern __u32 dwTveGet625VbiData(void);
extern __u32 dwTveGetVbiCtrl(void);
extern void vTveHalSetCc(__u8 bHi, __u8 bLo, __u32 dFld);
extern bool fgTveCheckCCDummy(void);
extern bool fgTveGetTvField(void);

#endif /*_TVE_HAL_H_*/


