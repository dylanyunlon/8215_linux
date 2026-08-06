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
#ifndef __ARM2__
#include <linux/kernel.h>
#else
#include "x_types.h"
#endif
#include "vdp.h"
#include "log.h"
#include "vdp_hal.h"
#include "pulldown.h"
#include "pattern.h"

#define MERGE_TABLE_SIZE                      (sizeof(_arMergeTable) / sizeof(_arMergeTable[0]))
#define MOTION_SEQ_LIST_CAPACITY              100
#define INVALID_STATE                         ((__u32)(-1))
#define COMBO_SEQ_LIST_CAPACITY               32
#define PATTEN_SEQ_REBUILD_SIZE               100
#define TOLERATE_ERR_CNT                      0
#define VDP_NUM                               2

static __u32 _u4PdCurState[VDP_NUM];
static __u32 _u4PassCnt[VDP_NUM]       = {
	2,
	2
};
static __u32 _u4ContinueUnMatchedCnt[VDP_NUM] = {
	0,
	0
};

static __u32 _au4MotionSeqList[VDP_NUM][MOTION_SEQ_LIST_CAPACITY];

TABLE_T _rMotionSeq[VDP_NUM] = {
	{
		_au4MotionSeqList[0],
		0,
	},
	{
		_au4MotionSeqList[1],
		0,
	},
};

static __u32 _au4ComboSeqList[VDP_NUM][COMBO_SEQ_LIST_CAPACITY];

TABLE_T _rComboSeq[VDP_NUM] = {
	{
		_au4ComboSeqList[0],
		0,
	},
	{
		_au4ComboSeqList[1],
		0,
	},
};

static void vResetStateMachine(__u32 u4VdpIdx)
{
	_rMotionSeq[u4VdpIdx].u4TableSize = 0;
	_rComboSeq[u4VdpIdx].u4TableSize = 0;
	_u4PassCnt[u4VdpIdx] = 2;
}

static __u32 GetNextState(__u32 u4Start, __u32 u4End, __u32 u4Cur)
{
	__u32 u4NextState = u4Cur;

	if (u4Cur >= u4End) {
		u4NextState = u4Start;
	} else {
		u4NextState = u4Cur + 1;
	}

	return u4NextState;
}

static void vPrintList(__u32 *pu4List, __u32 u4Size)
{

	__u32 i;

	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "[VDP_PD] dump list u4Size=%d <<<<\r\n", (int)u4Size);

	for (i = 0; i < u4Size; i++) {
		FB_PRINT(FB_LOG_LVL_DBG, "VDP", "%d ", (int)pu4List[i]);
	}

	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "[VDP_PD] dump list i=%d>>>>\r\n", (int)i);
}

PULLDOWN_MODE_T GetPulldownMode(__u32 u4VdpIdx)
{
	VDP_PARAM *prParam = &rData[u4VdpIdx];

	return prParam->u4PullDownMode;
}

void SetPulldownMode(__u32 u4VdpIdx, PULLDOWN_MODE_T eMode)
{
	VDP_PARAM *prParam = &rData[u4VdpIdx];

	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "[VDP_PD] set pull down id %d, mode %d\r\n", (int)u4VdpIdx, (int)eMode);
	prParam->u4PullDownMode = eMode;

	if (prParam->u4PullDownMode == PULLDOWN_MODE_UNKNOWN) {
		vVdpHalDisMergeZ(u4VdpIdx);
		vResetStateMachine(u4VdpIdx);
	}
}

__u32 u4CheckMotionInfo(__u32 u4VdpIdx, __u32 u4MotionWY, __u32 u4MotionXZ, FIELD_PRO_T eField)
{
	__u32 u4DiffData = 1;

	if (u4MotionWY < PULLDOWN_32_MOTION_CNT_STILL_THRESHOLD) {
		u4DiffData = 0;
	} else {
		if ((u4MotionWY * 5) < (u4MotionXZ * 2)) {
			u4DiffData = 0;
		}
	}

	return u4DiffData;
}

__u32 u4CheckCombInfo(__u32 u4VdpIdx, __u32 u4ComboWX, __u32 u4ComboYX, FIELD_PRO_T eField)
{
	__u32 u4Diff = 0xFF;
	/*Fix CNB00236973(8) change 0xFF to 0/1*/
	if (eField == FIELD_TOP) {
		u4Diff = (u4ComboYX > ((u4ComboWX + 1) * 2)) ? 1 : 0;
	} else {
		u4Diff = (u4ComboWX > ((u4ComboYX + 1) * 2)) ? 0 : 1;
	}

	return u4Diff;
}

void vPushDiffDataToSeqList(__u32 u4VdpIdx, bool fgIsMotion, bool fgDiffData, bool fgFirstField)
{
	TABLE_T *prTable = NULL;
	__u32 u4MaxTableSize = 0;

	if (fgIsMotion) {
		prTable = &_rMotionSeq[u4VdpIdx];
		u4MaxTableSize = MOTION_SEQ_LIST_CAPACITY;
	} else {
		prTable = &_rComboSeq[u4VdpIdx];
		u4MaxTableSize = COMBO_SEQ_LIST_CAPACITY;
	}

	if (prTable->u4TableSize == 0) {
		if (!fgFirstField) {
			return; /* save diff data begin with first field*/
		}
		prTable->pu4TablebA[prTable->u4TableSize++] = fgDiffData;
	} else {
		if (prTable->u4TableSize >= u4MaxTableSize) {
			prTable->u4TableSize = 0;
		}

		prTable->pu4TablebA[prTable->u4TableSize++] = fgDiffData;
	}
}

void  vPushPdInfoToList(__u32 u4VdpIdx, __u32 u4DiffComb, __u32 u4DiffMotion, bool fgFirstField)
{
	if (_u4PassCnt[u4VdpIdx]) {
		_u4PassCnt[u4VdpIdx]--;
	} else {
		vPushDiffDataToSeqList(u4VdpIdx, FALSE, u4DiffComb, fgFirstField);
		vPushDiffDataToSeqList(u4VdpIdx, TRUE, u4DiffMotion, fgFirstField);
	}
}

__u32 u4SeqListMatchPatten(TABLE_T *prPattern, TABLE_T *prSeqList, __u32 u4NeedMatchCnt)
{
	__u32 u4PatternReBuild[PATTEN_SEQ_REBUILD_SIZE];
	__u32 u4ReBuildCnt = u4NeedMatchCnt, u4Temp = 0, u4Index = 0;
	__u32 u4Start = 0, u4CurState = 0;
	bool fgMatched = FALSE;

	if (u4NeedMatchCnt > PATTEN_SEQ_REBUILD_SIZE) {
		FB_PRINT(FB_LOG_LVL_ERR, "VDP", "[VDP_PD] need match cnt %d > 100 \r\n", (int)u4NeedMatchCnt);
		return INVALID_STATE;
	}

	if (prSeqList->u4TableSize < u4NeedMatchCnt) {
		return INVALID_STATE;
	}

	while (u4ReBuildCnt) {
		/* Set up goal pattern*/
		u4Temp = MIN(u4ReBuildCnt, prPattern->u4TableSize);
		memcpy(u4PatternReBuild + u4Index, prPattern->pu4TablebA, u4Temp * sizeof(__u32));
		u4Index += u4Temp;
		u4ReBuildCnt -= u4Temp;
	}

	u4Start = 0;

	while ((u4Start + u4NeedMatchCnt) <= prSeqList->u4TableSize) {
		u4Temp = memcmp(prSeqList->pu4TablebA + u4Start, u4PatternReBuild, u4NeedMatchCnt * sizeof(__u32));

		if (u4Temp) {
			u4Start += 2;  /*first filed alignment*/
		} else {
			fgMatched = TRUE;
			break;
		}
	}

	if (fgMatched) {
		u4CurState = (prSeqList->u4TableSize - 1 - u4Start) % prPattern->u4TableSize;
		FB_PRINT(FB_LOG_LVL_DBG, "VDP", "[VDP_PD] CurState %d start %d \r\n", (int)u4CurState, (int)u4Start);
		vPrintList(prSeqList->pu4TablebA , prSeqList->u4TableSize);
	} else {
		u4CurState = INVALID_STATE;
	}

	return u4CurState;
}

__u32 u4PulldownDetect(__u32 u4VdpIdx)
{
	TABLE_T *prComboSeq = &_rComboSeq[u4VdpIdx];
	TABLE_T *prMotionSeq = &_rMotionSeq[u4VdpIdx];
	PULLDOWN_MODE_T eMode;

	
	_u4PdCurState[u4VdpIdx] = u4SeqListMatchPatten(&_r32Patern, prMotionSeq, 20);
	if (_u4PdCurState[u4VdpIdx] != INVALID_STATE) {
		eMode = PULLDOWN_MODE_32;
		goto MATCHED;
	}

	_u4PdCurState[u4VdpIdx] = u4SeqListMatchPatten(&_r2332Patern, prMotionSeq, 20);
	if (_u4PdCurState[u4VdpIdx] != INVALID_STATE) {
		eMode = PULLDOWN_MODE_2332;
		goto MATCHED;
	}

	_u4PdCurState[u4VdpIdx] = u4SeqListMatchPatten(&_r64Patern, prMotionSeq, 20);
	if (_u4PdCurState[u4VdpIdx] != INVALID_STATE) {
		eMode = PULLDOWN_MODE_64;
		goto MATCHED;
	}

	_u4PdCurState[u4VdpIdx] = u4SeqListMatchPatten(&_r55Patern, prMotionSeq, 20);
	if (_u4PdCurState[u4VdpIdx] != INVALID_STATE) {
		eMode = PULLDOWN_MODE_55;
		goto MATCHED;
	}

	_u4PdCurState[u4VdpIdx] = u4SeqListMatchPatten(&_r2224Patern, prMotionSeq, 20);
	if (_u4PdCurState[u4VdpIdx] != INVALID_STATE) {
		eMode = PULLDOWN_MODE_2224;
		goto MATCHED;
	}

	_u4PdCurState[u4VdpIdx] = u4SeqListMatchPatten(&_r32322Patern, prMotionSeq, 24);
	if (_u4PdCurState[u4VdpIdx] != INVALID_STATE) {
		eMode = PULLDOWN_MODE_32322;
		goto MATCHED;
	}

	_u4PdCurState[u4VdpIdx] = u4SeqListMatchPatten(&_r87Patern, prMotionSeq, 25);
	if (_u4PdCurState[u4VdpIdx] != INVALID_STATE) {
		eMode = PULLDOWN_MODE_87;
		goto MATCHED;
	}
	
	_u4PdCurState[u4VdpIdx] = u4SeqListMatchPatten(&_r22Patern, prComboSeq, 28);
	if (_u4PdCurState[u4VdpIdx] != INVALID_STATE) {
			eMode = PULLDOWN_MODE_22;
			goto MATCHED;
	}

	/*FB_PRINT(FB_LOG_LVL_DBG, "VDP", "[VDP_PD] vdp %d unmatch mode %d state %d\r\n"
		, (int)u4VdpIdx, (int)eMode, (int)_u4PdCurState[u4VdpIdx]);*/
	return FALSE;

MATCHED:

	SetPulldownMode(u4VdpIdx, eMode);
	_u4ContinueUnMatchedCnt[u4VdpIdx] = 0;
	FB_PRINT(FB_LOG_LVL_INFO, "VDP", "[VDP_PD] vdp %d match pull down mode %d\r\n", (int)u4VdpIdx, (int)eMode);

	return TRUE;
}

bool fgIsMatchCurMode(__u32 u4VdpIdx, __u32 u4Diff, PULLDOWN_MODE_T eMode)
{
	TABLE_T *prPattern = NULL;
	__u32 u4CurState = 0;
	bool ret;

	if (eMode >= MERGE_TABLE_SIZE) {
		return FALSE;
	}

	prPattern = _aprPattern[eMode];

	if (prPattern->pu4TablebA == NULL) {
		return FALSE;
	}

	u4CurState = _u4PdCurState[u4VdpIdx];

	if (u4Diff != prPattern->pu4TablebA[u4CurState]) {
		_u4ContinueUnMatchedCnt[u4VdpIdx]++;
	} else {
		_u4ContinueUnMatchedCnt[u4VdpIdx] = 0;
	}

	FB_PRINT(FB_LOG_LVL_DBG, "VDP", "[VDP_PD] u4CurState %d u4Diff %d, %d\r\n"
		, (int)u4CurState, (int)u4Diff, (int)prPattern->pu4TablebA[u4CurState]);
	ret = _u4ContinueUnMatchedCnt[u4VdpIdx] <= TOLERATE_ERR_CNT;

	return ret;
}

void vPullDownSetMergeInfo(__u32 u4VdpIdx, PULLDOWN_MODE_T eMode)
{
	__u32 u4NextState;
	TABLE_T *prMergeTable = NULL;
	STILL_MERGE_TYPE eMergeType = STILL_MERGE_YZ;

	if (eMode >= MERGE_TABLE_SIZE) {
		return;
	}

	prMergeTable = &_arMergeTable[eMode];

	if (prMergeTable->pu4TablebA == NULL) {
		return;
	}

	u4NextState = GetNextState(0, prMergeTable->u4TableSize - 1, _u4PdCurState[u4VdpIdx]);
	eMergeType  = prMergeTable->pu4TablebA[u4NextState];
	/*FB_PRINT(FB_LOG_LVL_DBG, "VDP", "[VDP_PD] next %d mergeZ %x\r\n", (int)u4NextState, (unsigned int)eMergeType);*/
	vVdpHalSetMergeZ(u4VdpIdx, eMergeType);
	_u4PdCurState[u4VdpIdx] = u4NextState;
}

void vPullDownModeCheck(__u32 u4VdpIdx, __u32 u4DiffComb, __u32 u4DiffMotion, FIELD_PRO_T eField)
{
	VDP_PARAM *prParam = &rData[u4VdpIdx];
	/* first field flag changed in last vsync, and need to roll back for motion detect*/
	bool fgFirstField = !prParam->fgFirstField;

	/*FB_PRINT(FB_LOG_LVL_DBG, "VDP", "[VDP_PD] VdpIdx %d, u4DiffComb %d, u4DiffMotion %d, eField %d, fgFirstField %d\r\n"
		, (int)u4VdpIdx, (int)u4DiffComb, (int)u4DiffMotion, (int)eField, (int)fgFirstField);*/

	switch (prParam->u4PullDownMode) {
	case PULLDOWN_MODE_UNKNOWN:
		vPushPdInfoToList(u4VdpIdx, u4DiffComb, u4DiffMotion, fgFirstField);
		u4PulldownDetect(u4VdpIdx);
		break;



	case PULLDOWN_MODE_32:
	case PULLDOWN_MODE_2332:
	case PULLDOWN_MODE_64:
	case PULLDOWN_MODE_55:
	case PULLDOWN_MODE_2224:
	case PULLDOWN_MODE_32322:
	case PULLDOWN_MODE_87:
		if (fgIsMatchCurMode(u4VdpIdx, u4DiffMotion, prParam->u4PullDownMode) == FALSE) {
			FB_PRINT(FB_LOG_LVL_INFO, "VDP", "[VDP_PD]vPullDownModeCheck unmatch exit mode %d....\r\n"
				, (int)prParam->u4PullDownMode);
			/*exit specific mode ,then enter unknown mode*/
			SetPulldownMode(u4VdpIdx, PULLDOWN_MODE_UNKNOWN);
		}

		break;

	case PULLDOWN_MODE_22:
		if (fgIsMatchCurMode(u4VdpIdx, u4DiffComb, prParam->u4PullDownMode) == FALSE) {
			FB_PRINT(FB_LOG_LVL_INFO, "VDP", "[VDP_PD] unmatch exit mode 22 id=%d\r\n", (int)u4VdpIdx);
			/*exit specific mode ,then enter unknown mode*/
			SetPulldownMode(u4VdpIdx, PULLDOWN_MODE_UNKNOWN);
		}

		break;


	default:
		FB_PRINT(FB_LOG_LVL_WARN, "VDP", "[VDP_PD]vPullDownModeCheck not support pull down mode %d\r\n"
			, (int)prParam->u4PullDownMode);
		break;
	}
}

void vPullDownGetMotionComb(__u32 u4VdpIdx)
{
	__u32 u4CombXY = 0, u4CombWX = 0, u4MotionWY = 0, u4MotionXZ = 0;
	__u32 u4DiffComb = 0, u4DiffMotion = 0;
	FIELD_PRO_T eField;

	/* Get Motion, Comb info and push into list*/
	vVdpHalGetMotionComb(u4VdpIdx, &u4CombXY, &u4CombWX, &u4MotionWY, &u4MotionXZ);
	vVdpHalGetFieldInfo(u4VdpIdx, (__u32 *)&eField);
	u4DiffComb = u4CheckCombInfo(u4VdpIdx, u4CombWX, u4CombXY, eField);
	u4DiffMotion = u4CheckMotionInfo(u4VdpIdx, u4MotionWY, u4MotionXZ, eField);
	vPullDownModeCheck(u4VdpIdx, u4DiffComb, u4DiffMotion, eField);
	/*FB_PRINT(FB_LOG_LVL_DBG, "VDP", "[VDP_PD] XY %x, WX %x, WY %x, XZ %x, comb %d, motion %d, Field %d %d\r\n"
		, (unsigned int)u4CombXY, (unsigned int)u4CombWX, (unsigned int)u4MotionWY
		, (unsigned int)u4MotionXZ, (int)u4DiffComb, (int)u4DiffMotion
		, (int)eField, (int)!rData[u4VdpIdx].fgFirstField);*/
}




