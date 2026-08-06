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
#ifndef _PULL_DOWN_H

#define _PULL_DOWN_H


#define PULLDOWN_32_MOTION_CNT_STILL_THRESHOLD  0xFF /*0x200 */
#define PULLDOWN_22_COMB_CNT_STILL_THRESHOLD    0x10
#define PS_FLD_RPT_THD                          0x200
#define TABLE_SIZE(x)                           (sizeof(x) / sizeof(__u32))

typedef enum {
	FIELD_TOP,
	FIELD_BOTTOM,
} FIELD_PRO_T;

typedef enum {
	PULLDOWN_MODE_UNKNOWN,
	PULLDOWN_MODE_22,
	PULLDOWN_MODE_32,
	PULLDOWN_MODE_2332,   /*3*/
	PULLDOWN_MODE_64, /*4*/
	PULLDOWN_MODE_55, /*5*/
	PULLDOWN_MODE_2224,/*6*/
	PULLDOWN_MODE_32322,/*7*/
	PULLDOWN_MODE_87,/*8*/
} PULLDOWN_MODE_T;

typedef struct {
	__u32 *pu4TablebA;
	__u32  u4TableSize;
} TABLE_T;

typedef enum {
	STILL_MERGE_XY,
	STILL_MERGE_YZ,
} STILL_MERGE_TYPE;


extern PULLDOWN_MODE_T GetPulldownMode(__u32 u4VdpIdx);
extern void SetPulldownMode(__u32 u4VdpIdx, PULLDOWN_MODE_T eMode);
extern __u32 u4CheckMotionInfo(__u32 u4VdpIdx, __u32 u4MotionWY, __u32 u4MotionXZ, FIELD_PRO_T eField);
extern __u32 u4CheckCombInfo(__u32 u4VdpIdx, __u32 u4ComboWX, __u32 u4ComboYX, FIELD_PRO_T eField);
extern void vPullDownSetMergeInfo(__u32 u4VdpIdx, PULLDOWN_MODE_T eMode);
extern void vPullDownModeCheck(__u32 u4VdpIdx, __u32 u4DiffComb, __u32 u4DiffMotion, FIELD_PRO_T eField);
extern void vPullDownGetMotionComb(__u32 u4VdpIdx);


#endif


