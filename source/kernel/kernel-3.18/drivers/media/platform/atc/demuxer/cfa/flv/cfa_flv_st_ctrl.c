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



#include "windows.h"
#include <media/atc/dmx_define.h>
/* #include <media/atc/mm_debug.h> */
#include <media/atc/memdbg_c.h>

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_spt.h"
#include "cfa_macro.h"
#include "cfa_flv.h"
#include "cfa_flv_st_ctrl.h"

#define CFA_FLV_GET_DELTA_VAL(val, a, b)	  \
		do {								  \
			if ((a) >= (b)) {                 \
				(val) = (a) - (b);              \
			}                                 \
			else {                            \
				(val) = (b) - (a);              \
			}								  \
		} while (0)

#define MAX_VALID_TIMESTAMP (24*60*60*1000)


static MRESULT CfaFlvTxAVCmdQ(void *pvSptHdl, CfaFlvInst_T *prCfaFlv);

#if CONFIG_CFA_FLV_FOR_ERR_JUMP
static bool CfaFlvNotiDmxChunkError(void *pvSptHdl, CfaFlvInst_T *prCfaFlv, u32 u4Status);
#endif

void CfaFlvQuickSort(u32 *pu4SortElems, s32 i4Begin, s32 i4End)
{
	if ((i4Begin < i4End) && (i4Begin >= 0) && (i4End >= 0)) {
		u32 u4Tmp = 0, u4Tmpt = pu4SortElems[i4Begin];
		s32 i4Low = i4Begin, i4High = i4End;

		while (i4Low < i4High) {
			while (pu4SortElems[i4Low] <= u4Tmpt)
				i4Low++;

			while ((0 < i4High) && (u4Tmpt <= pu4SortElems[i4High]))
				i4High--;

			if (i4Low < i4High) {
				u4Tmp = pu4SortElems[i4Low];
				pu4SortElems[i4Low] = pu4SortElems[i4High];
				pu4SortElems[i4High] = u4Tmp;
			} else {
				return;
			}

			if (i4Low + 1 == i4High)
				return;
		}

		CfaFlvQuickSort(pu4SortElems, i4Begin, i4Low);

		CfaFlvQuickSort(pu4SortElems, i4Low + 1, i4End);
	}
}


u32 CfaFlvGetMinDelta(u32 *pu4SortElems, u32 u4Begin, u32 u4End)
{
	u32 u4Idx = 0;
	u32 u4MinVal = DMX_INVALID_UINT32;
	u32 u4Tmp = 0;

	if (u4Begin >= u4End)
		return 0;

	for (u4Idx = u4Begin; u4Idx < u4End - (u32)1; u4Idx++) {
		CFA_FLV_GET_DELTA_VAL(u4Tmp, pu4SortElems[u4Idx], pu4SortElems[u4Idx + 1]);
		if (u4Tmp < u4MinVal)
			u4MinVal = u4Tmp;
	}

	return u4MinVal;
}

static void CfaFlvClrTimeRefineParams(FLV_TIMESTAMP_MAN_T *prTimeMan)
{
	prTimeMan->u4OldNormnalTime = 0;
	prTimeMan->u4FirstAbnormalTime = 0;
	prTimeMan->u4TimeDecCnt = 0;
	prTimeMan->u4TimeIncCnt = 0;
	prTimeMan->fgNeedCheckTimeDec = FALSE;
	prTimeMan->fgNeedCheckTimeInc = FALSE;

	dmx_memset(prTimeMan->au4Rec4CorrectTime, 0xFF,
		sizeof(u32) * CFA_FLV_MAX_PTS_RECORD_LEN);
}

static void CfaFlvCalcDeltaTime(E_SPT_DATA_TYPE_T eStmType,
				void *pvSptHdl, CfaFlvInst_T *prCfaFlv, const u32 *pu4CurTime)
{
	u32 u4Idx = 0;
	u32 u4DeltaVal = 0;
	FLV_TIMESTAMP_MAN_T *prTimeMan = NULL;

	if ((NULL == pu4CurTime) || (NULL == prCfaFlv))
		return;

	if (SPT_DATA_A == eStmType)
		prTimeMan = &(prCfaFlv->rATimeStampMan);
	else
		prTimeMan = &(prCfaFlv->rVTimeStampMan);

	for (u4Idx = (u32)0; u4Idx < (u32)CFA_FLV_MAX_PTS_RECORD_LEN; u4Idx++) {
		if (DMX_INVALID_UINT32 == prTimeMan->au4TimeRecord[u4Idx]) {
			prTimeMan->au4TimeRecord[u4Idx] = *pu4CurTime;
			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("[CFA_FLV]%s line %d -- (%s) prTimeMan->au4TimeRecord[%d]: %d \r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				(SPT_DATA_A == eStmType) ? TEXT("Audio") : TEXT("Video"),
				u4Idx, (prTimeMan->au4TimeRecord[u4Idx]));
			break;
		}
	}

	if (SPT_DATA_A == eStmType) {
		if (((u32)0 < u4Idx) && (u4Idx < (u32)CFA_FLV_MAX_PTS_RECORD_LEN)) {
			CFA_FLV_GET_DELTA_VAL(u4DeltaVal, prTimeMan->au4TimeRecord[u4Idx],
					      prTimeMan->au4TimeRecord[u4Idx - (u32)1]);
			if ((u32)CFA_FLV_ERR_TIMESTAMP_AUD_DELTA_MS < u4DeltaVal) {
				/* 15fps*/
				for (u4Idx = (u32)0; u4Idx < (u32)CFA_FLV_MAX_PTS_RECORD_LEN; u4Idx++)
					prTimeMan->au4TimeRecord[u4Idx] = DMX_INVALID_UINT32;

				prTimeMan->u4DeltaTime = DMX_INVALID_UINT32;
#if CFA_FLV_DBG_AUD_PTS
				DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					TEXT("[CFA_FLV] %s line %d -- Audio (DeltaVal(%d) > ")
					TEXT("ErrDeltaMs(%d)), Set DeltaTime to be -1\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4DeltaVal,
					CFA_FLV_ERR_TIMESTAMP_AUD_DELTA_MS);
#endif				/*CFA_FLV_DBG_VID_PTS*/

				return;
			}
		}

		if ((u32)CFA_FLV_MAX_PTS_RECORD_LEN - (u32)1 == u4Idx) {
			u32 u4TotalDeltaTime = 0;
			s32 i4Low1 = 0, i4Low2 = 0;
			u32 au4TimeRecord[CFA_FLV_MAX_PTS_RECORD_LEN];

			CfaFlvQuickSort(prTimeMan->au4TimeRecord, (s32)0,
				CFA_FLV_MAX_PTS_RECORD_LEN - 1);

			i4Low2 = 0;
			au4TimeRecord[0] = prTimeMan->au4TimeRecord[0];
			for (i4Low1 = 0; i4Low1 < CFA_FLV_MAX_PTS_RECORD_LEN; i4Low1++) {
				if ((i4Low2 + 1 < CFA_FLV_MAX_PTS_RECORD_LEN) &&
				    (prTimeMan->au4TimeRecord[i4Low1] != au4TimeRecord[i4Low2])) {
					i4Low2++;
					au4TimeRecord[i4Low2] = prTimeMan->au4TimeRecord[i4Low1];
				}
			}

			if (i4Low2 < CFA_FLV_MAX_PTS_RECORD_LEN) {
				CFA_FLV_GET_DELTA_VAL(u4TotalDeltaTime, au4TimeRecord[0],
						      au4TimeRecord[i4Low2]);
			} else {
				CFA_FLV_GET_DELTA_VAL(u4TotalDeltaTime, au4TimeRecord[0],
						      au4TimeRecord[CFA_FLV_MAX_PTS_RECORD_LEN -
								    1]);
			}

			if (i4Low2 > 0) {
				prTimeMan->u4DeltaTime = u4TotalDeltaTime / (u32)i4Low2;
			} else {
				prTimeMan->u4DeltaTime = 0;
				for (u4Idx = (u32)0; u4Idx < (u32)CFA_FLV_MAX_PTS_RECORD_LEN; u4Idx++)
					prTimeMan->au4TimeRecord[u4Idx] = DMX_INVALID_UINT32;

				prTimeMan->u4DeltaTime = DMX_INVALID_UINT32;
				return;
			}

#if CFA_FLV_DBG_AUD_PTS
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("[CFA_FLV] %s line %d -- Audio DeltaTime: %d, BaseTime: %d\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prTimeMan->u4DeltaTime,
				prTimeMan->u4BaseTime);
#endif				/* CFA_FLV_DBG_VID_PTS*/
			if ((0 != prTimeMan->u4DeltaTime) &&
			    (DMX_INVALID_UINT32 != prTimeMan->u4DeltaTime)) {
				prTimeMan->fgNeedGetDeltaTime = FALSE;
			} else {
				/* this should not come in*/
				for (u4Idx = (u32)0; u4Idx < (u32)CFA_FLV_MAX_PTS_RECORD_LEN; u4Idx++)
					prTimeMan->au4TimeRecord[u4Idx] = DMX_INVALID_UINT32;
			}
			return;
		}
	} else {
		if (((u32)0 < u4Idx) && (u4Idx < (u32)CFA_FLV_MAX_PTS_RECORD_LEN)) {
			CFA_FLV_GET_DELTA_VAL(u4DeltaVal, prTimeMan->au4TimeRecord[u4Idx],
					      prTimeMan->au4TimeRecord[u4Idx - 1]);
			if (0x1800 < u4DeltaVal) {
				/* 15fps*/
				for (u4Idx = 0; u4Idx < CFA_FLV_MAX_PTS_RECORD_LEN; u4Idx++)
					prTimeMan->au4TimeRecord[u4Idx] = DMX_INVALID_UINT32;

				prTimeMan->u4DeltaTime = DMX_INVALID_UINT32;

				return;
			}
		}

		if (1 == u4Idx) {
			prTimeMan->u4DeltaTime = u4DeltaVal;

			if (0x1800 < u4DeltaVal) {
				for (u4Idx = 0; u4Idx < CFA_FLV_MAX_PTS_RECORD_LEN; u4Idx++)
					prTimeMan->au4TimeRecord[u4Idx] = DMX_INVALID_UINT32;

				prTimeMan->u4DeltaTime = DMX_INVALID_UINT32;

				return;
			}
		} else if ((1 < u4Idx)
			&& (CFA_FLV_MAX_PTS_RECORD_LEN - 1 == u4Idx)) {
			if ((0 != prTimeMan->u4DeltaTime) &&
			    (DMX_INVALID_UINT32 != prTimeMan->u4DeltaTime)) {
				u32 u4TotalDeltaTime = 0;
				u32 u4MinDeltaTime = 0;
				s32 i4Low1 = 0, i4Low2 = 0;

				u32 au4TimeRecord[CFA_FLV_MAX_PTS_RECORD_LEN] = { 0 };

#if CFA_FLV_DBG_VID_PTS
				DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] %s line %d -- To get delta time\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO);
#endif				/*CFA_FLV_DBG_VID_PTS*/

				CfaFlvQuickSort(prTimeMan->au4TimeRecord, 0,
						CFA_FLV_MAX_PTS_RECORD_LEN - 1);

				i4Low2 = 0;
				au4TimeRecord[i4Low2] = prTimeMan->au4TimeRecord[0];
				for (i4Low1 = 0; i4Low1 < CFA_FLV_MAX_PTS_RECORD_LEN;
				     i4Low1++) {
					if ((i4Low2 + 1 < CFA_FLV_MAX_PTS_RECORD_LEN)
					    && (prTimeMan->au4TimeRecord[i4Low1] !=
						au4TimeRecord[i4Low2])) {
						i4Low2++;
						au4TimeRecord[i4Low2] =
						    prTimeMan->au4TimeRecord[i4Low1];
					}
				}

				if (i4Low2 < CFA_FLV_MAX_PTS_RECORD_LEN) {
					CFA_FLV_GET_DELTA_VAL(u4TotalDeltaTime,
							      au4TimeRecord[0],
							      au4TimeRecord[i4Low2]);
				} else {
					CFA_FLV_GET_DELTA_VAL(u4TotalDeltaTime,
							      au4TimeRecord[0],
							      au4TimeRecord
							      [CFA_FLV_MAX_PTS_RECORD_LEN -
							       1]);
				}

				if (i4Low2 > 0) {
					prTimeMan->u4DeltaTime = u4TotalDeltaTime / i4Low2;
				} else {
					prTimeMan->u4DeltaTime = 0;
					for (u4Idx = 0; u4Idx < CFA_FLV_MAX_PTS_RECORD_LEN;
					     u4Idx++) {
						prTimeMan->au4TimeRecord[u4Idx] =
						    DMX_INVALID_UINT32;
					}

					prTimeMan->u4DeltaTime = DMX_INVALID_UINT32;
					return;
				}

				prTimeMan->fgNeedGetDeltaTime = FALSE;

				if (i4Low2 < CFA_FLV_MAX_PTS_RECORD_LEN) {
					u4MinDeltaTime =
					    CfaFlvGetMinDelta(prTimeMan->au4TimeRecord, 0,
							      i4Low2);
				} else {
					u4MinDeltaTime =
					    CfaFlvGetMinDelta(prTimeMan->au4TimeRecord, 0,
							      CFA_FLV_MAX_PTS_RECORD_LEN -
							      1);
				}

				if (u4MinDeltaTime < prTimeMan->u4DeltaTime)
					prTimeMan->u4DeltaTime = u4MinDeltaTime;
				if (prTimeMan->u4DeltaTime <
				    CFA_FLV_MAX_60FPS_FRAME_DURATION) {
					prTimeMan->u4DeltaTime =
					    CFA_FLV_60FPS_FRAME_DURATION;
				} else if (prTimeMan->u4DeltaTime <
					   CFA_FLV_MAX_30FPS_FRAME_DURATION) {
					prTimeMan->u4DeltaTime =
					    CFA_FLV_30FPS_FRAME_DURATION;
				} else if (prTimeMan->u4DeltaTime <
					   CFA_FLV_MAX_25FPS_FRAME_DURATION) {
					prTimeMan->u4DeltaTime =
					    CFA_FLV_25FPS_FRAME_DURATION;
				} else if (prTimeMan->u4DeltaTime <
					   CFA_FLV_MAX_20FPS_FRAME_DURATION) {
					prTimeMan->u4DeltaTime =
					    CFA_FLV_20FPS_FRAME_DURATION;
				} else {
					prTimeMan->u4DeltaTime =
					    CFA_FLV_15FPS_FRAME_DURATION;
				}

#if CFA_FLV_DBG_VID_PTS
				DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					TEXT("[CFA_FLV] %s line %d -- Video DeltaTime: %d, BaseTime: %d\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					prTimeMan->u4DeltaTime, prTimeMan->u4BaseTime);
#endif				/* CFA_FLV_DBG_VID_PTS*/
			} else {
				for (u4Idx = 0; u4Idx < CFA_FLV_MAX_PTS_RECORD_LEN; u4Idx++) {
					prTimeMan->au4TimeRecord[u4Idx] =
					    DMX_INVALID_UINT32;
				}
			}

			return;
		} else if (1 < u4Idx) {
			prTimeMan->u4DeltaTime = u4DeltaVal;
		} else {
			/*do nothing*/
		}
	}

}

static void CfaFlvSetTimeState(E_SPT_DATA_TYPE_T eStmType,
			       void *pvSptHdl, CfaFlvInst_T *prCfaFlv, const u32 *pu4CurTime)
{
	FLV_TIMESTAMP_MAN_T *prTimeMan = NULL;
	u32 u4JumpErrDeltaTime = 0;

	if ((NULL == pu4CurTime) || (NULL == prCfaFlv))
		return;

	prTimeMan = &(prCfaFlv->rVTimeStampMan);
	u4JumpErrDeltaTime = (u32)CFA_FLV_ERR_TIMESTAMP_VID_DELTA_MS;

	if (SPT_DATA_A == eStmType) {
		prTimeMan = &(prCfaFlv->rATimeStampMan);
		u4JumpErrDeltaTime = (u32)CFA_FLV_ERR_TIMESTAMP_AUD_DELTA_MS;
	}

	prTimeMan->u4LastTime = prTimeMan->u4CurTime;
	prTimeMan->u4CurTime = *pu4CurTime;

	if (prTimeMan->fgFirstGetTime) {
		prTimeMan->eState = CFA_FLV_TIMESTAMP_OK;

		prTimeMan->fgFirstGetTime = FALSE;

		prTimeMan->u4BaseTime = *pu4CurTime;
		prTimeMan->u4CurTime = *pu4CurTime;

		if (prTimeMan->u4BaseTime >= prCfaFlv->rRange.u4AV1stTime) {
			prTimeMan->u4SeekTime =
			    prTimeMan->u4BaseTime - prCfaFlv->rRange.u4AV1stTime;
		} else {
			prTimeMan->u4SeekTime = (u32) (prCfaFlv->rRange.u8SeekTime);
		}

#if CFA_FLV_DBG_VID_PTS
		if (SPT_DATA_V == eStmType) {
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("[CFA_FLV] %s line %d -- Video BaseTime: %d, SeekTime: %d, RangeSeekTime: %d\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prTimeMan->u4BaseTime,
				prTimeMan->u4SeekTime, prCfaFlv->rRange.u8SeekTime);
		}
#endif				/* CFA_FLV_DBG_VID_PTS*/
#if CFA_FLV_DBG_AUD_PTS
		if (SPT_DATA_A == eStmType) {
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("[CFA_FLV] %s line %d -- Audio BaseTime: %d, SeekTime: %d, RangeSeekTime: %d\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prTimeMan->u4BaseTime,
				prTimeMan->u4SeekTime, prCfaFlv->rRange.u8SeekTime);
		}
#endif				/* CFA_FLV_DBG_AUD_PTS*/

		return;
	}
	if (0 < prTimeMan->u4FirstAbnormalTime) {
		s64 i4DeltaTime1 = 0;
		s64 i4DeltaTime2 = 0;
		u64 u4DeltaTime1 = 0;
		u64 u4DeltaTime2 = 0;

		prTimeMan->eState = CFA_FLV_TIMESTAMP_ERROR;

		CFA_FLV_GET_DELTA_VAL(u4DeltaTime1, prTimeMan->u4CurTime,
				      prTimeMan->u4LastTime);
		i4DeltaTime1 = prTimeMan->u4CurTime - prTimeMan->u4LastTime;
		CFA_FLV_GET_DELTA_VAL(u4DeltaTime2, prTimeMan->u4CurTime,
				      prTimeMan->u4FirstAbnormalTime);
		i4DeltaTime2 = prTimeMan->u4CurTime - prTimeMan->u4FirstAbnormalTime;

		if (prTimeMan->fgNeedCheckTimeInc) {
			if ((u4DeltaTime1 < u4JumpErrDeltaTime) &&
			    (u4DeltaTime2 < u4JumpErrDeltaTime)) {
				if (prTimeMan->u4TimeIncCnt < CFA_FLV_MAX_PTS_RECORD_LEN) {
					prTimeMan->au4Rec4CorrectTime[prTimeMan->
								      u4TimeIncCnt] =
					    prTimeMan->u4CurTime;
				}

				prTimeMan->u4TimeIncCnt += 1;

				if (prTimeMan->u4TimeIncCnt >= CFA_FLV_MAX_PTS_RECORD_LEN) {
					prTimeMan->eState = CFA_FLV_TIMESTAMP_BIG_CHG;
					CfaFlvClrTimeRefineParams(prTimeMan);
					prTimeMan->u4FirstAbnormalTime = 0;
					prTimeMan->u4OldNormnalTime = 0;
#if CFA_FLV_DBG_VID_PTS
					if (SPT_DATA_V == eStmType) {
						DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
							TEXT("[CFA_FLV] %s line %d -- ")
							TEXT("Set Video Time --> BIG_CHANGE\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
					}
#endif				/* CFA_FLV_DBG_VID_PTS*/
#if CFA_FLV_DBG_AUD_PTS
					if (SPT_DATA_A == eStmType) {
						DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
							TEXT("[CFA_FLV] %s line %d -- ")
							TEXT("Set Audio Time --> BIG_CHANGE\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO);
					}
#endif				/* CFA_FLV_DBG_AUD_PTS*/
				} else {
#if CFA_FLV_DBG_VID_PTS
					if (SPT_DATA_V == eStmType) {
						DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
							TEXT("[CFA_FLV] %s line %d -- Set Video Time")
							TEXT(" --> TIME_ERROR, CurTime: %d\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							*pu4CurTime);
					}
#endif				/* CFA_FLV_DBG_VID_PTS*/
#if CFA_FLV_DBG_AUD_PTS
					if (SPT_DATA_A == eStmType) {
						DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
							TEXT("[CFA_FLV] %s line %d -- Set Audio Time")
							(" --> TIME_ERROR, CurTime: %d\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							*pu4CurTime);
					}
#endif				/* CFA_FLV_DBG_AUD_PTS*/
					prTimeMan->eState = CFA_FLV_TIMESTAMP_ERROR;
				}
			} else {
				prTimeMan->eState = CFA_FLV_TIMESTAMP_ERROR;
				CfaFlvClrTimeRefineParams(prTimeMan);
				prTimeMan->u4FirstAbnormalTime = 0;
				prTimeMan->u4OldNormnalTime = 0;
#if CFA_FLV_DBG_VID_PTS
				if (SPT_DATA_V == eStmType) {
					DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						TEXT("[CFA_FLV] %s line %d -- Set Video Time ")
						TEXT("--> TIME_ERROR, CurTime: %d\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						*pu4CurTime);
				}
#endif				/* CFA_FLV_DBG_VID_PTS*/
#if CFA_FLV_DBG_AUD_PTS
				if (SPT_DATA_A == eStmType) {
					DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						TEXT("[CFA_FLV] %s line %d -- Set Audio Time ")
						TEXT("--> TIME_ERROR, CurTime: %d\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						*pu4CurTime);
				}
#endif				/* CFA_FLV_DBG_AUD_PTS*/
			}
		}

		if (prTimeMan->fgNeedCheckTimeDec) {
			if ((u4DeltaTime1 < u4JumpErrDeltaTime) &&
			    (u4DeltaTime2 < u4JumpErrDeltaTime)) {
				if (prTimeMan->u4TimeDecCnt < CFA_FLV_MAX_PTS_RECORD_LEN) {
					prTimeMan->au4Rec4CorrectTime[prTimeMan->
								      u4TimeDecCnt] =
					    prTimeMan->u4CurTime;
				}

				prTimeMan->u4TimeDecCnt += 1;

				if (prTimeMan->u4TimeDecCnt >= CFA_FLV_MAX_PTS_RECORD_LEN) {
					prTimeMan->eState = CFA_FLV_TIMESTAMP_BIG_CHG;
					CfaFlvClrTimeRefineParams(prTimeMan);
					prTimeMan->u4FirstAbnormalTime = 0;
					prTimeMan->u4OldNormnalTime = 0;
#if CFA_FLV_DBG_VID_PTS
					if (SPT_DATA_V == eStmType) {
						DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
							TEXT("[CFA_FLV] %s line %d -- Set Video Time ")
							TEXT("--> BIG_CHANGE, CurTime: %d\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							*pu4CurTime);
					}
#endif				/* CFA_FLV_DBG_VID_PTS*/
#if CFA_FLV_DBG_AUD_PTS
					if (SPT_DATA_A == eStmType) {
						DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
							TEXT("[CFA_FLV] %s line %d -- Set Audio Time ")
							TEXT("--> BIG_CHANGE, CurTime: %d\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							*pu4CurTime);
					}
#endif				/* CFA_FLV_DBG_AUD_PTS*/
				} else {
					prTimeMan->eState = CFA_FLV_TIMESTAMP_ERROR;
#if CFA_FLV_DBG_VID_PTS
					if (SPT_DATA_V == eStmType) {
						DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
							TEXT("[CFA_FLV] %s line %d -- Set Video Time ")
							TEXT("--> TIME_ERROR, CurTime: %d\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							*pu4CurTime);
					}
#endif				/* CFA_FLV_DBG_VID_PTS*/
#if CFA_FLV_DBG_AUD_PTS
					if (SPT_DATA_A == eStmType) {
						DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
							TEXT("[CFA_FLV] %s line %d -- Set Audio Time ")
							TEXT("--> TIME_ERROR, CurTime: %d\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO,
							*pu4CurTime);
					}
#endif				/* CFA_FLV_DBG_AUD_PTS*/
				}
			} else {
				prTimeMan->eState = CFA_FLV_TIMESTAMP_ERROR;
				CfaFlvClrTimeRefineParams(prTimeMan);
				prTimeMan->u4FirstAbnormalTime = 0;
				prTimeMan->u4OldNormnalTime = 0;
#if CFA_FLV_DBG_VID_PTS
				if (SPT_DATA_V == eStmType) {
					DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						TEXT("[CFA_FLV] %s line %d -- Set Video Time")
						TEXT(" --> TIME_ERROR, CurTime: %d\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						*pu4CurTime);
				}
#endif				/* CFA_FLV_DBG_VID_PTS*/
#if CFA_FLV_DBG_AUD_PTS
				if (SPT_DATA_A == eStmType) {
					DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						TEXT("[CFA_FLV] %s line %d -- Set Audio Time ")
						TEXT("--> TIME_ERROR, CurTime: %d\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						*pu4CurTime);
				}
#endif				/* CFA_FLV_DBG_AUD_PTS*/
			}
		}
	} else {
		/*0 == prTimeMan->u4FirstAbnormalTime*/
		s32 i4DeltaTime = 0;
		u32 u4DeltaTime = 0;

		CFA_FLV_GET_DELTA_VAL(u4DeltaTime, prTimeMan->u4CurTime,
				      prTimeMan->u4LastTime);
		i4DeltaTime = prTimeMan->u4CurTime - prTimeMan->u4LastTime;
		if (u4JumpErrDeltaTime <= u4DeltaTime) {
			prTimeMan->eState = CFA_FLV_TIMESTAMP_ERROR;

			CfaFlvClrTimeRefineParams(prTimeMan);

			if (prTimeMan->fgNeedGetDeltaTime) {
				u32 u4Idx = 0;

				prTimeMan->u4LastTime = 0;
				prTimeMan->u4CurTime = *pu4CurTime;
				prTimeMan->u4BaseTime = *pu4CurTime;
				for (u4Idx = (u32)0; u4Idx < (u32)CFA_FLV_MAX_PTS_RECORD_LEN; u4Idx++) {
					prTimeMan->au4TimeRecord[u4Idx] =
					    DMX_INVALID_UINT32;
				}
				prTimeMan->au4TimeRecord[0] = *pu4CurTime;
#if CFA_FLV_DBG_VID_PTS
				if (SPT_DATA_V == eStmType) {
					DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						TEXT("[CFA_FLV] %s line %d -- Set Video Time ")
						TEXT("--> TIME_ERROR, Change BaseTime to be: %d\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						*pu4CurTime);
				}
#endif				/* CFA_FLV_DBG_VID_PTS*/
#if CFA_FLV_DBG_AUD_PTS
				if (SPT_DATA_A == eStmType) {
					DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						TEXT("[CFA_FLV] %s line %d -- Set Audio Time ")
						TEXT("--> TIME_ERROR, Change BaseTime to be: %d\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						*pu4CurTime);
				}
#endif				/* CFA_FLV_DBG_AUD_PTS*/
				return;
			}

			prTimeMan->u4FirstAbnormalTime = prTimeMan->u4CurTime;
			prTimeMan->u4OldNormnalTime = prTimeMan->u4LastTime;

			if (0 < i4DeltaTime) {
				prTimeMan->fgNeedCheckTimeInc = TRUE;
				prTimeMan->fgNeedCheckTimeDec = FALSE;
			} else {
				prTimeMan->fgNeedCheckTimeDec = TRUE;
				prTimeMan->fgNeedCheckTimeInc = FALSE;
			}

#if CFA_FLV_DBG_VID_PTS
			if (SPT_DATA_V == eStmType) {
				DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					TEXT("[CFA_FLV] %s line %d -- Set Video Time --> TIME_ERROR, CurTime: %d\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, *pu4CurTime);
			}
#endif				/* CFA_FLV_DBG_VID_PTS*/
#if CFA_FLV_DBG_AUD_PTS
			if (SPT_DATA_A == eStmType) {
				DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					TEXT("[CFA_FLV] %s line %d -- Set Audio Time --> TIME_ERROR, CurTime: %d\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, *pu4CurTime);
			}
#endif				/* CFA_FLV_DBG_AUD_PTS*/
		} else {
			prTimeMan->eState = CFA_FLV_TIMESTAMP_OK;
		}
	}
}

static void CfaFlvReAdjustAUPts(E_SPT_DATA_TYPE_T eStmType,
				void *pvSptHdl, CfaFlvInst_T *prCfaFlv, u32 *pu4CurTime)
{
	FLV_TIMESTAMP_MAN_T *prVTimeMan = NULL, *prATimeMan = NULL;
	u32 u4PtsChangeCnt = 0;

	prATimeMan = &(prCfaFlv->rATimeStampMan);

	prVTimeMan = &(prCfaFlv->rVTimeStampMan);

	if (SPT_DATA_V == eStmType) {
		if (CFA_FLV_TIMESTAMP_BIG_CHG == prVTimeMan->eState) {
			if (CFA_FLV_MAX_PTS_RECORD_LEN <= prVTimeMan->u4TimeDecCnt)
				u4PtsChangeCnt = prVTimeMan->u4TimeDecCnt;
		} else {
			return;
		}

		if ((prVTimeMan->u4CurTime >
		     (prATimeMan->u4CurTime + CFA_FLV_VID_AUD_TIME_MAX_DIFF))
		    || ((prVTimeMan->u4CurTime + CFA_FLV_VID_AUD_TIME_MAX_DIFF) <
			prATimeMan->u4CurTime)) {
			*pu4CurTime = prATimeMan->u4BaseTime;
			prVTimeMan->u4CurTime = *pu4CurTime;
#if CFA_FLV_DBG_VID_PTS
			if (SPT_DATA_V == eStmType) {
				DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_PTS_V,
					    TEXT("[CFA_FLV] %s line %d -- Video CurTime: %d\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, *pu4CurTime);
			}
#endif				/* CFA_FLV_DBG_VID_PTS*/
#if CFA_FLV_DBG_AUD_PTS
			if (SPT_DATA_A == eStmType) {
				DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_PTS_A,
					    TEXT("[CFA_FLV] %s line %d -- Audio CurTime: %d\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, *pu4CurTime);
			}
#endif				/* CFA_FLV_DBG_AUD_PTS*/
			return;
		}

		if (u4PtsChangeCnt > 0)
			prVTimeMan->u4CurTime = *pu4CurTime;
	}

}


static MRESULT CfaFlvCorrectAUPts(E_SPT_DATA_TYPE_T eStmType,
				  void *pvSptHdl, CfaFlvInst_T *prCfaFlv, u32 *pu4CurTime)
{
	FLV_TIMESTAMP_MAN_T *prVTimeMan = NULL;
	FLV_TIMESTAMP_MAN_T *prATimeMan = NULL;
	FLV_TIMESTAMP_MAN_T *prTimeMan = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prCfaFlv) || (NULL == pu4CurTime)) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("[CFA_FLV] failed for invalid args\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prATimeMan = &(prCfaFlv->rATimeStampMan);
	prVTimeMan = &(prCfaFlv->rVTimeStampMan);

	if (DMX_INVALID_UINT32 == *pu4CurTime) {
		prVTimeMan->eState = CFA_FLV_TIMESTAMP_ERROR;
		MM_RETURN(RET_DMX_OK);
	}

	prTimeMan = prVTimeMan;
	if (SPT_DATA_A == eStmType)
		prTimeMan = prATimeMan;
	/* get delta video Pes Pts*/
	if (prTimeMan->fgNeedGetDeltaTime)
		CfaFlvCalcDeltaTime(eStmType, pvSptHdl, prCfaFlv, pu4CurTime);
	/* check pts state*/
	CfaFlvSetTimeState(eStmType, pvSptHdl, prCfaFlv, pu4CurTime);

	switch (prTimeMan->eState) {
	case CFA_FLV_TIMESTAMP_ERROR:
		*pu4CurTime = (u32) INVALID_TIMESTAMP;
		break;

	case CFA_FLV_TIMESTAMP_BIG_CHG:
		CfaFlvReAdjustAUPts(eStmType, pvSptHdl, prCfaFlv, pu4CurTime);

		if (SPT_DATA_V == eStmType) {
			prVTimeMan->u4BaseTime = prVTimeMan->u4CurTime;
			prVTimeMan->u4TimeDecCnt = 0;
			prVTimeMan->u4TimeIncCnt = 0;
			prVTimeMan->u4FirstAbnormalTime = 0;
			prVTimeMan->eState = CFA_FLV_TIMESTAMP_OK;
#if CFA_FLV_DBG_VID_PTS
			if (SPT_DATA_V == eStmType) {
				DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_PTS_V,
					    TEXT("[CFA_FLV] %s line %d -- (BIG_CHANGE) BaseTime: %d\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, prVTimeMan->u4BaseTime);
			}
#endif				/*CFA_FLV_DBG_VID_PTS*/
		} else {
			if (DMX_INVALID_UINT32 == prATimeMan->u4DeltaTime) {
				prATimeMan->u4TimeIncrement +=
				    prATimeMan->u4LastTime + prATimeMan->u4DeltaTime -
				    prATimeMan->u4BaseTime;
			} else {
				prATimeMan->u4TimeIncrement +=
				    prATimeMan->u4LastTime - prATimeMan->u4BaseTime;
			}

			prATimeMan->u4BaseTime = *pu4CurTime;

			CfaFlvClrTimeRefineParams(prATimeMan);
#if CFA_FLV_DBG_AUD_PTS
			if (SPT_DATA_A == eStmType) {
				DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_PTS_A,
					    TEXT("[CFA_FLV] %s line %d -- ")
					    TEXT("(BIG_CHANGE) BaseTime: %d, Increment: %d\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, prATimeMan->u4BaseTime,
					    prATimeMan->u4TimeIncrement);
			}
#endif				/* CFA_FLV_DBG_AUD_PTS*/
		}
		break;
	case CFA_FLV_TIMESTAMP_OK:
	default:
		break;
	}

	MM_RETURN(mrRet);
}

/*Description: When flv cfa finished parsing, call this function.*/
/*@return none*/
static void CfaFlvFinishPrs(void *pvSptHdl, CfaFlvInst_T *prCfaFlv, u32 u4Status)
{
	u64 u8Ea = 0;
	MRESULT mrRet = RET_DMX_OK;

    if(NULL == prCfaFlv)
    {
        DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] prCfaFlv is NULL!\r\n"));
		Spt4CfaFinishedEx(pvSptHdl, 0, FALSE, u4Status);
		return;
    }

	MMATE_CHECK_POINTER(prCfaFlv);
	MMATE_CHECK_STRUCT(prCfaFlv->rRange);
	MMATE_CHECK_STRUCT(prCfaFlv->rDecoderCfgInfo);

	if (((NULL != prCfaFlv->prAudCmdQsInfo) &&
	     (0 < prCfaFlv->prAudCmdQsInfo->u4EntryCnt)) ||
	    ((NULL != prCfaFlv->prVidCmdQsInfo) && (0 < prCfaFlv->prVidCmdQsInfo->u4EntryCnt))) {
		prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_TX_AV_CMDQ;
		mrRet = CfaFlvTxAVCmdQ(pvSptHdl, prCfaFlv);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] fail in CfaFlvTxAVCmdQ, mrRet: 0x%x.\r\n"),
				    mrRet);
			u8Ea = prCfaFlv->rFileInfo.u8FileSize;
			prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_IDLE;

			Spt4CfaFinishedEx(pvSptHdl, u8Ea, FALSE, u4Status);
		}
		return;
	}

	if (fgIsCfaStmToPlay(prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_AUD))
		u8Ea = prCfaFlv->rRange.u8AudEa;

	if (DMX_INVALID_UINT64 == u8Ea)
		u8Ea = prCfaFlv->rFileInfo.u8FileSize;

	prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_IDLE;

	DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		    TEXT("[CFA_FLV] %s line %d -- Finished parsing for current range!\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO);

	if (DMX_IS_FF_PLAY(pvSptHdl) && (!prCfaFlv->rRange.fgHasVid)) {
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			    TEXT("[CFA_FLV] %s line %d -- Pure Audio Send EOS for Fast Forward!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		mrRet = Spt4CfaFinishedEx(pvSptHdl, u8Ea, TRUE, u4Status);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] fail in Spt4CfaFinishedEx(TRUE), mrRet: 0x%x.\r\n"),
				    mrRet);
		}
		return;
	}

	mrRet = Spt4CfaFinishedEx(pvSptHdl, u8Ea, FALSE, u4Status);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] fail in Spt4CfaFinishedEx, mrRet: 0x%x.\r\n"),
			mrRet);
	}

}

/* Description: Set picture tx mode in terms of video codec type*/
/* @Return: cfa picture tx mode. 04/10/2008*/
static CfaApiPicTxMode CfaFlvSetPicTxMode(CfaFlvInst_T *prCfaFlv)
{
	u32 u4Offset = (u32)8;

	switch (prCfaFlv->eVidCodecType) {
	case CFA_VID_DIVX3:
		if (prCfaFlv->fgExistCompressData) {
			if ((u8)DIVX311_PVOP == (prCfaFlv->u1FirstBytePay >> 6U))
				return CFA_PTM_ONE_PIC_DX3_P;
			return CFA_PTM_ONE_PIC_DX3_I;
		}

		if (prCfaFlv->fgKeyFrame)
			return CFA_PTM_ONE_PIC_DX3_I;
		return CFA_PTM_ONE_PIC_DX3_P;

	case CFA_VID_H263_SORENSON:
		if (FLV_FRM_TYPE_I == prCfaFlv->rVideoInfo.eFrmType)
			return CFA_PTM_H263_SORENSON_I;
		else if (FLV_FRM_TYPE_P == prCfaFlv->rVideoInfo.eFrmType)
			return CFA_PTM_H263_SORENSON_P;
		else {
			/*do nothing*/
		}

		DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("[CFA_FLV] %s -- Frame Type(%d) is error!\r\n"),
			    DMX_FUNC_NAME, prCfaFlv->rVideoInfo.eFrmType);
		return CFA_PTM_H263_SORENSON_P;

	case CFA_VID_WMV9:
		if (prCfaFlv->fgPrsSeqFrameInterpolation)
			u4Offset--;
		u4Offset -= (u32)2;
		if (prCfaFlv->fgPrsPreProcRange)
			u4Offset--;
		if (prCfaFlv->u1FirstBytePay & (1 << u4Offset)) {
			--u4Offset;
			return CFA_PTM_WMV_P;
		}
		--u4Offset;

		if (0 == prCfaFlv->u4PrsNumBFrames)
			return CFA_PTM_WMV_I;
		if (prCfaFlv->u1FirstBytePay & (1 << u4Offset)) {
			--u4Offset;
			return CFA_PTM_WMV_I;
		}
		else {
			--u4Offset;
			return CFA_PTM_WMV_B;
		}

	case CFA_VID_VC1:
		break;
	case CFA_VID_VP6:
	case CFA_VID_VP6A:
		if (FLV_FRM_TYPE_I == prCfaFlv->rVideoInfo.eFrmType)
			return CFA_PTM_ONE_PIC_VP6_I;
		else if (FLV_FRM_TYPE_P == prCfaFlv->rVideoInfo.eFrmType)
			return CFA_PTM_ONE_PIC_VP6_P;
		else {
			/*do nothing*/
		}
		break;
	default:
		break;
	}

	return CFA_PTM_EXACT_POS;
}

static u32 CfaFlvGetSliceSize(CfaFlvInst_T *prCfaFlv, u8 *pu1BufSa)
{
	u32 u4Size = 0;

	if((AVCODEC_ID_H264 == prCfaFlv->rVideoInfo.rCfgInfo.eCodecID) ||
		(VCODEC_VERSION_H265_HM91 == prCfaFlv->rVideoInfo.rCfgInfo.rVersion)) {
		if (1 == prCfaFlv->u4PayloadLenFieldSz) {
			u8 u1Size = 0;

			LOAD_BYTE(pu1BufSa, u1Size);
			u4Size = (u32) u1Size;
		} else if (2 == prCfaFlv->u4PayloadLenFieldSz) {
			u16 u2Size = 0;

			LOADB_WORD(pu1BufSa, u2Size);
			u4Size = (u32) u2Size;
		} else if (3 == prCfaFlv->u4PayloadLenFieldSz) {
			LOADB_3BYTES2DWORD(pu1BufSa, u4Size);
		} else if (4 == prCfaFlv->u4PayloadLenFieldSz) {
			LOADB_DWRD(pu1BufSa, u4Size);
		} else {
			/*do nothing*/
		}
	}

	return u4Size;
}

static MRESULT CfaFlvTxAudCmdQ(void *pvSptHdl, CfaFlvInst_T *prCfaFlv, u32 u4TxLen)
{
	CFA_AUDIO_INFO_T rCfaFlvTxAudInfo = { 0 };
	/*DMX_CMDQ_TX_ENTRY_T arCmdEntrys[DMX_MAX_TX_CNT_FOR_CMD_Q];*/
	DMX_CMDQ_TX_ENTRY_T *arCmdEntrys = prCfaFlv->prCmdEntrys;
	CfaFlvAudCmdQInfo_T *prCmdQInfo = prCfaFlv->prAudCmdQsInfo;
	u32 u4CmdEntryIdx = 0;
	MRESULT mrRet = RET_DMX_OK;

	mm_memset(arCmdEntrys, 0, sizeof(DMX_CMDQ_TX_ENTRY_T) * DMX_MAX_TX_CNT_FOR_CMD_Q);
	rCfaFlvTxAudInfo.u8FileOfst = prCmdQInfo->arEntrys[0].u8FileOffset;
	rCfaFlvTxAudInfo.u8Len = prCmdQInfo->u8TotalLen;
	rCfaFlvTxAudInfo.u8Pts = prCmdQInfo->arEntrys[0].u8Pts;	/*change unit in Hz, STC Clock*/
	rCfaFlvTxAudInfo.u4PrsStrmId = (u32) prCfaFlv->rAudioInfo.rCfgInfo.u1StrmNum;	/*need modify*/
	rCfaFlvTxAudInfo.eAudType = prCfaFlv->rAudioInfo.eAudType;
	rCfaFlvTxAudInfo.u8RealTxLen = prCmdQInfo->u4RealTxLen;

	for (u4CmdEntryIdx = 0;
	     (u4CmdEntryIdx < prCmdQInfo->u4EntryCnt)
	     && (prCmdQInfo->u4EntryCnt <= DMX_MAX_TX_CNT_FOR_CMD_Q); u4CmdEntryIdx++) {
		arCmdEntrys[u4CmdEntryIdx].u4TxLen = prCmdQInfo->arEntrys[u4CmdEntryIdx].u4Len;
		if (0 == u4CmdEntryIdx) {

			arCmdEntrys[u4CmdEntryIdx].u4TxOfst = 0;
		} else {
			arCmdEntrys[u4CmdEntryIdx].u4TxOfst =
			    (u32) (prCmdQInfo->arEntrys[u4CmdEntryIdx].u8FileOffset -
				      (prCmdQInfo->arEntrys[u4CmdEntryIdx - (u32)1].u8FileOffset +
				       prCmdQInfo->arEntrys[u4CmdEntryIdx - (u32)1].u4Len));
		}
		if (!prCfaFlv->rRange.fgHasVid)
			arCmdEntrys[u4CmdEntryIdx].fgEndAU = FALSE;
		else
			arCmdEntrys[u4CmdEntryIdx].fgEndAU = TRUE;

#if ENABLE_DMX_ADVANCED_VER
		if (AVCODEC_ID_AAC == prCfaFlv->rAudioInfo.rCfgInfo.eCodecID) {
			arCmdEntrys[u4CmdEntryIdx].fgInsertHdr =
			    prCmdQInfo->arEntrys[u4CmdEntryIdx].fgInsertHdr;
			arCmdEntrys[u4CmdEntryIdx].u4InsertHdrLen =
			    prCmdQInfo->arEntrys[u4CmdEntryIdx].u4InsertHdrLen;
			prCmdQInfo->arEntrys[u4CmdEntryIdx].u4InsertHdrLen = 7;
			mm_memcpy(arCmdEntrys[u4CmdEntryIdx].au1InsertHdr,
				  prCmdQInfo->arEntrys[u4CmdEntryIdx].au1InsertHdrBuf,
				  arCmdEntrys[u4CmdEntryIdx].u4InsertHdrLen);
		}
#endif				/*ENABLE_DMX_ADVANCED_VER*/

#if CFA_FLV_DBG_AUD_CMD_Q_FLOW
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_CMDQ_A,
			    TEXT("[CFA_FLV] %s line %d -- ComposeCmdQ ")
			    TEXT("(ACmdIdx: %d, FileOfst: %lld, Len: %d, Pts: %lld ms, fgEnd: %d)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u4CmdEntryIdx,
			    prCmdQInfo->arEntrys[u4CmdEntryIdx].u8FileOffset,
			    prCmdQInfo->arEntrys[u4CmdEntryIdx].u4Len,
			    PTS_TO_MS(prCmdQInfo->arEntrys[u4CmdEntryIdx].u8Pts),
			    (arCmdEntrys[u4CmdEntryIdx].fgEndAU ? 1 : 0));
#endif				/* CFA_FLV_DBG_AUD_CMD_Q_FLOW*/
	}

	if (u4CmdEntryIdx > 0) {
		if (!prCfaFlv->rRange.fgHasVid) {
			if (DMX_IS_RW_PLAY(pvSptHdl)) {
				if ((prCfaFlv->u4PureAudCurUnitTxSz + u4TxLen >=
				     (u32)FLV_AUD_UNIT_MAX_SZ_IN_FFRW)
				    || (prCfaFlv->u4AvailDataSz <=
					u4TxLen + (u32)FLV_READ_FOR_CODEC_SIZE)) {
					arCmdEntrys[u4CmdEntryIdx - (u32)1].fgEndAU = TRUE;
					prCfaFlv->fgJumpTurnOn = FALSE;
					DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_CMDQ_A,
						    TEXT("[CFA_FLV] %s line %d -- Pure Audio AU End\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO);
				}
			} else {
				if ((prCfaFlv->u4AvailDataSz <= u4TxLen + (u32)FLV_READ_FOR_CODEC_SIZE)
				    || (prCmdQInfo->u4EntryCnt >= (u32)DMX_MAX_TX_CNT_FOR_CMD_Q)) {
					arCmdEntrys[u4CmdEntryIdx - (u32)1].fgEndAU = TRUE;
#if CFA_FLV_DBG_AUD_CMD_Q_FLOW
					DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_CMDQ_A,
						    TEXT("[CFA_FLV] %s line %d -- Pure Audio AU End\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO);
#endif				/* CFA_FLV_DBG_AUD_CMD_Q_FLOW*/
				}
			}
		} else {
			arCmdEntrys[u4CmdEntryIdx - (u32)1].fgEndAU = TRUE;
		}
	}

	rCfaFlvTxAudInfo.fgUseCmdQ = TRUE;
	rCfaFlvTxAudInfo.fgAUByCmdQEnd = TRUE;
	rCfaFlvTxAudInfo.parCmdQTxEntry = arCmdEntrys;
	rCfaFlvTxAudInfo.u2TxEntryCnt = prCmdQInfo->u4EntryCnt;
	rCfaFlvTxAudInfo.fgAUCompleteByEnd = FALSE;

	rCfaFlvTxAudInfo.fgUnitStart = TRUE;
	rCfaFlvTxAudInfo.u8TotalAULen = prCmdQInfo->u8TotalLen;

	prCmdQInfo->fgIsInDma = TRUE;

	prCfaFlv->eCurTxStrmType = CFA_FLV_TX_STRM_TYPE_AUD;

#if CFA_FLV_DBG_AUD_CMD_Q_FLOW
	DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_CMDQ_A,
		    TEXT("[CFA_FLV] %s line %d -- DmaAud ")
		    TEXT("(ACmdCnt: %d, FileOfst: %lld, Len: %lld, RealTxLen: ")
		    TEXT("%lld), Ca: %lld, Pts: %lld ms, TotalAULen: %%lld\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO, rCfaFlvTxAudInfo.u2TxEntryCnt,
		    rCfaFlvTxAudInfo.u8FileOfst, rCfaFlvTxAudInfo.u8Len,
		    rCfaFlvTxAudInfo.u8RealTxLen, prCfaFlv->u8Ca,
		    PTS_TO_MS(prCmdQInfo->arEntrys[0].u8Pts), prCmdQInfo->u8TotalLen);
#endif				/* CFA_FLV_DBG_AUD_CMD_Q_FLOW*/

	mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rCfaFlvTxAudInfo);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			    TEXT("[CFA_FLV] failed ")
			    TEXT("in Spt4CfaPbb2AFifoAUCtrl, mrRet: 0x%x\r\n"),
			    mrRet);
		CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, GAU_E_FAIL);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaFlvTxVidCmdQ(void *pvSptHdl, CfaFlvInst_T *prCfaFlv, u32 u4TxLen)
{
	CFA_VIDEO_INFO_T rCfaFlvTxVidInfo = { 0 };
	/*DMX_CMDQ_TX_ENTRY_T arCmdEntrys[DMX_MAX_TX_CNT_FOR_CMD_Q];*/
	DMX_CMDQ_TX_ENTRY_T *arCmdEntrys = prCfaFlv->prCmdEntrys;
	CfaFlvVidCmdQInfo_T *prCmdQInfo = prCfaFlv->prVidCmdQsInfo;
	u32 u4CmdEntryIdx = 0;
	MRESULT mrRet = RET_DMX_OK;

	mm_memset(arCmdEntrys, 0, sizeof(DMX_CMDQ_TX_ENTRY_T) * DMX_MAX_TX_CNT_FOR_CMD_Q);
	rCfaFlvTxVidInfo.u8FileOfst = prCmdQInfo->arEntrys[0].u8FileOffset;
	rCfaFlvTxVidInfo.u8Len = prCmdQInfo->u8TotalLen;
	rCfaFlvTxVidInfo.u4PrsStrmId = (u32) prCfaFlv->rVideoInfo.rCfgInfo.u1StrmNum;
	rCfaFlvTxVidInfo.u8RealTxLen = prCmdQInfo->u4RealTxLen;
	rCfaFlvTxVidInfo.u8TotalAULen = prCmdQInfo->u8TotalLen;

	rCfaFlvTxVidInfo.eTxMode = CfaFlvSetPicTxMode(prCfaFlv);
	rCfaFlvTxVidInfo.u8FileOfst = prCmdQInfo->arEntrys[0].u8FileOffset;
	rCfaFlvTxVidInfo.eVidType = prCfaFlv->eVidCodecType;

	for (u4CmdEntryIdx = 0;
	     (u4CmdEntryIdx < prCmdQInfo->u4EntryCnt)
	     && (prCmdQInfo->u4EntryCnt <= DMX_MAX_TX_CNT_FOR_CMD_Q); u4CmdEntryIdx++) {
		arCmdEntrys[u4CmdEntryIdx].u4TxLen = prCmdQInfo->arEntrys[u4CmdEntryIdx].u4Len;
		if (0 == u4CmdEntryIdx) {

			arCmdEntrys[u4CmdEntryIdx].u4TxOfst = 0;
		} else {
			arCmdEntrys[u4CmdEntryIdx].u4TxOfst =
			    (u32) (prCmdQInfo->arEntrys[u4CmdEntryIdx].u8FileOffset -
				      (prCmdQInfo->arEntrys[u4CmdEntryIdx - (u32)1].u8FileOffset +
				       prCmdQInfo->arEntrys[u4CmdEntryIdx - (u32)1].u4Len));
		}
		arCmdEntrys[u4CmdEntryIdx].fgEndAU = prCmdQInfo->arEntrys[u4CmdEntryIdx].fgEndAU;
#if ENABLE_DMX_ADVANCED_VER
		arCmdEntrys[u4CmdEntryIdx].eTxMode = prCmdQInfo->arEntrys[u4CmdEntryIdx].eTxMode;
#endif				/* ENABLE_DMX_ADVANCED_VER*/
	}

#if ENABLE_DMX_ADVANCED_VER
	rCfaFlvTxVidInfo.fgAUByCmdQEnd = FALSE;
	if (Spt4CfaIsNonHdrDectVCodec(prCfaFlv->eVidCodecType))
		rCfaFlvTxVidInfo.fgAUByCmdQEnd = TRUE;
#endif				/* ENABLE_DMX_ADVANCED_VER*/

	rCfaFlvTxVidInfo.fgUseCmdQ = TRUE;
	rCfaFlvTxVidInfo.parCmdQTxEntry = arCmdEntrys;
	rCfaFlvTxVidInfo.u2TxEntryCnt = prCmdQInfo->u4EntryCnt;
	rCfaFlvTxVidInfo.fgAUCompleteByEnd = FALSE;
	rCfaFlvTxVidInfo.fgUnitStart = FALSE;

	prCmdQInfo->fgIsInDma = TRUE;

	prCfaFlv->eCurTxStrmType = CFA_FLV_TX_STRM_TYPE_VID;

#if CFA_FLV_DBG_VID_CMD_Q_FLOW
	DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_CMDQ_v,
		    TEXT("[CFA_FLV] %s line %d -- DmaVid ")
		    TEXT("(VCmdCnt: %d, FileOfst: %lld, Len: %lld, RealTxLen: %lld), Ca: %lld, Pts: %lld ms\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO, rCfaFlvTxVidInfo.u2TxEntryCnt,
		    rCfaFlvTxVidInfo.u8FileOfst, rCfaFlvTxVidInfo.u8Len,
		    rCfaFlvTxVidInfo.u8RealTxLen, prCfaFlv->u8Ca,
		    PTS_TO_MS(prCmdQInfo->arEntrys[0].u8Pts));
#endif				/* CFA_FLV_DBG_VID_CMD_Q_FLOW*/

	mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rCfaFlvTxVidInfo);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			    TEXT("[CFA_FLV] failed ")
			    TEXT("in Spt4CfaPbb2VFifoAUCtrl, mrRet: 0x%x\r\n"),
			    mrRet);
		CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, (u32)GAU_E_FAIL);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaFlvTxAVCmdQ(void *pvSptHdl, CfaFlvInst_T *prCfaFlv)
{
	MRESULT mrRet = RET_DMX_OK;

	if ((prCfaFlv->prVidCmdQsInfo->u4EntryCnt > 0) &&
	    fgIsCfaStmToPlay(prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_VID)) {
		prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;//CFA_FLV_ANA_ST_TX_AV_CMDQ;
		mrRet = CfaFlvTxVidCmdQ(pvSptHdl, prCfaFlv, 0);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] failed in CfaFlvTxVidCmdQ, mrRet: 0x%x\r\n"),
				    mrRet);
			CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, GAU_E_FAIL);
			MM_RETURN(mrRet);
		}
		MM_RETURN(mrRet);
	}

	if ((prCfaFlv->prAudCmdQsInfo->u4EntryCnt > 0) &&
	    fgIsCfaStmToPlay(prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_AUD)) {
		prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;//CFA_FLV_ANA_ST_TX_AV_CMDQ;
		mrRet = CfaFlvTxAudCmdQ(pvSptHdl, prCfaFlv, (u32)0);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] failed in CfaFlvTxAudCmdQ, mrRet: 0x%x\r\n"),
				    mrRet);
			CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, (u32)GAU_E_FAIL);
			MM_RETURN(mrRet);
		}
		MM_RETURN(mrRet);
	}

	if ((0 == prCfaFlv->prVidCmdQsInfo->u4EntryCnt) &&
            (0 == prCfaFlv->prAudCmdQsInfo->u4EntryCnt))
	{
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] Will enter CfaFlvSearchNextSc\r\n"));
		prCfaFlv->u4AvailDataSz = 0;
		CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, prCfaFlv->eNextAnaSt, 0, prCfaFlv->u4TxLen);
	}

	MM_RETURN(mrRet);
}

static void CfaFlvCalcAudPts(void *pvSptHdl, CfaFlvInst_T *prCfaFlv)
{
	FLV_TIMESTAMP_MAN_T *prVTimeMan = NULL;
	FLV_TIMESTAMP_MAN_T *prATimeMan = NULL;
	u32 u4BaseTime = 0;
	u32 u4CurTime = 0;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prCfaFlv) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] failed for invalid args\r\n"));
		return;
	}

	u4CurTime = prCfaFlv->rAudioInfo.rATagInfo.u4TimeStamp;
	prVTimeMan = &(prCfaFlv->rVTimeStampMan);

	prATimeMan = &(prCfaFlv->rATimeStampMan);

	mrRet = CfaFlvCorrectAUPts(SPT_DATA_A, pvSptHdl, prCfaFlv, &u4CurTime);

	if (DMX_INVALID_UINT32 == prATimeMan->u4BaseTime)
		u4BaseTime = prVTimeMan->u4BaseTime;
	else
		u4BaseTime = prATimeMan->u4BaseTime;

	if (DMX_INVALID_UINT32 != u4CurTime) {
		/*compute final audio PTS*/
		if (u4CurTime >= u4BaseTime) {
			prATimeMan->u4TimeStamp =
			    u4CurTime - u4BaseTime + prATimeMan->u4TimeIncrement +
			    (u32) (prATimeMan->u4SeekTime);
#if CFA_FLV_DBG_AUD_PTS
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_PTS_A,
				    TEXT("[CFA_FLV] %s line %d -- Audio TimeStamp: %d, ")
				    TEXT("Real TimeStamp: %d, Incre: %d, Base: %d, ASeekTime: %d, SeekTime: %d\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prATimeMan->u4TimeStamp,
				    prCfaFlv->rAudioInfo.rATagInfo.u4TimeStamp,
				    prATimeMan->u4TimeIncrement, u4BaseTime, prATimeMan->u4SeekTime,
				    (u32) (prCfaFlv->rRange.u8SeekTime));
#endif				/* CFA_FLV_DBG_AUD_PTS*/
			return;
		}
	}

	if (DMX_INVALID_UINT32 == prATimeMan->u4TimeStamp)
		prATimeMan->u4TimeStamp = (u32) (prATimeMan->u4SeekTime);

	if (DMX_INVALID_UINT32 != prVTimeMan->u4DeltaTime)
		prATimeMan->u4TimeStamp += prVTimeMan->u4DeltaTime;
	else
		prATimeMan->u4TimeStamp += CFA_FLV_30FPS_FRAME_DURATION;

#if CFA_FLV_DBG_AUD_PTS
	DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_PTS_A,
		    TEXT("[CFA_FLV] %s line %d -- Audio TimeStamp: %d, ")
		    TEXT("Real TimeStamp: %d, ASeekTime: %d, SeekTime: %d\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO, prATimeMan->u4TimeStamp,
		    prCfaFlv->rAudioInfo.rATagInfo.u4TimeStamp, prATimeMan->u4SeekTime,
		    (u32) (prCfaFlv->rRange.u8SeekTime));
#endif				/* CFA_FLV_DBG_AUD_PTS*/

}

#if !ENABLE_DMX_ADVANCED_VER
static MRESULT CfaFlvTxAACData2Fifo(void *pvSptHdl, CfaFlvInst_T *prCfaFlv,
				    u64 u8Sa, u32 u4TxLen, u64 u8Pts)
{
	CFA_AUDIO_INFO_T rCfaFlvTxAudInfo = { 0 };
	MRESULT mrRet = RET_DMX_OK;

	if ((prCfaFlv->u4AvailDataSz < prCfaFlv->u8Ca - prCfaFlv->u8PreCa) ||
	    (prCfaFlv->u4AvailDataSz <= u4TxLen + (u32)FLV_READ_FOR_CODEC_SIZE)) {
		/*Accross Slot*/
		rCfaFlvTxAudInfo.u8FileOfst = u8Sa;
		rCfaFlvTxAudInfo.u8Len = (u64) u4TxLen;
		rCfaFlvTxAudInfo.u8Pts = u8Pts;	/*change unit in Hz, STC Clock*/
		rCfaFlvTxAudInfo.u4PrsStrmId = (u32) prCfaFlv->rAudioInfo.rCfgInfo.u1StrmNum;	/*need modify*/
		rCfaFlvTxAudInfo.eAudType = prCfaFlv->rAudioInfo.eAudType;

		if (prCfaFlv->u8Endoffst < (rCfaFlvTxAudInfo.u8FileOfst + rCfaFlvTxAudInfo.u8Len)) {
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d -- EndOfst(%lld) ")
				    TEXT("< FileOfst(%lld) + Len(%lld), So call CfaFlvFinishPrs!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prCfaFlv->u8Endoffst,
				    rCfaFlvTxAudInfo.u8FileOfst, rCfaFlvTxAudInfo.u8Len);
			CfaFlvFinishPrs(pvSptHdl, prCfaFlv, (u32)GAU_E_EOS);
			MM_RETURN(RET_DMX_OK);
		}

		rCfaFlvTxAudInfo.fgUnitStart = FALSE;

#if CFA_FLV_DBG_AUD_CMD_Q_FLOW
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_CMDQ_A,
			    TEXT("[CFA_FLV] %s line %d -- DmaAud ")
			    TEXT("(FileOfst: %lld, Len: %lld, Pts: %lld ms)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, rCfaFlvTxAudInfo.u8FileOfst,
			    rCfaFlvTxAudInfo.u8Len, PTS_TO_MS(u8Pts));
#endif				/* CFA_FLV_DBG_AUD_CMD_Q_FLOW*/

		prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;

		prCfaFlv->eCurTxStrmType = CFA_FLV_TX_STRM_TYPE_AUD;

#if ENABLE_DMX_ADVANCED_VER
		rCfaFlvTxAudInfo.pu1InsertHdrBuf = prCfaFlv->rAudioInfo.pauAudHeader;
		rCfaFlvTxAudInfo.fgInsertHdr = TRUE;
		rCfaFlvTxAudInfo.u4InsertHdrLen = 7;
#endif				/* ENABLE_DMX_ADVANCED_VER*/

		rCfaFlvTxAudInfo.u8TotalAULen = 0;

		if (0 < u4TxLen) {

			mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rCfaFlvTxAudInfo);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] failed in ")
					    TEXT("Spt4CfaPbb2AFifoAUCtrl(), mrRet: 0x%x\r\n"),
					    mrRet);
				CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, (u32)GAU_E_FAIL);
				MM_RETURN(mrRet);
			}
		} else {
			prCfaFlv->fgTagIsValid = FALSE;
			CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
					   (u64)0, (u64)FLV_READ_FOR_CODEC_SIZE);
		}
	} else {
		DMX_CMDQ_TX_ENTRY_T arCmdEntrys[DMX_MAX_TX_CNT_FOR_CMD_Q];

		mm_memset(arCmdEntrys, 0, sizeof(DMX_CMDQ_TX_ENTRY_T) * DMX_MAX_TX_CNT_FOR_CMD_Q);
		rCfaFlvTxAudInfo.u8FileOfst = u8Sa;
		rCfaFlvTxAudInfo.u8Len = u4TxLen;
		rCfaFlvTxAudInfo.u8Pts = u8Pts;	/*change unit in Hz, STC Clock*/
		/*need modify*/
		rCfaFlvTxAudInfo.u4PrsStrmId = (u32) prCfaFlv->rAudioInfo.rCfgInfo.u1StrmNum;
		rCfaFlvTxAudInfo.eAudType = prCfaFlv->rAudioInfo.eAudType;
		rCfaFlvTxAudInfo.u8RealTxLen = u4TxLen;

		arCmdEntrys[0].u4TxOfst = 0;
		arCmdEntrys[0].u4TxLen = u4TxLen;
		arCmdEntrys[0].fgEndAU = FALSE;

#if ENABLE_DMX_ADVANCED_VER
		arCmdEntrys[0].fgInsertHdr = TRUE;
		arCmdEntrys[0].u4InsertHdrLen = 7;
		mm_memcpy(arCmdEntrys[0].au1InsertHdr, prCfaFlv->rAudioInfo.pauAudHeader,
			  arCmdEntrys[0].u4InsertHdrLen);
#endif				/* ENABLE_DMX_ADVANCED_VER*/
		if (DMX_IS_RW_PLAY(pvSptHdl)) {
			prCfaFlv->u4PureAudCurUnitTxSz += u4TxLen;

			if ((prCfaFlv->u4PureAudCurUnitTxSz >= FLV_AUD_UNIT_MAX_SZ_IN_FFRW) ||
			    (prCfaFlv->u4AvailDataSz <= u4TxLen + (u32)FLV_READ_FOR_CODEC_SIZE)) {
				arCmdEntrys[0].fgEndAU = TRUE;
				prCfaFlv->fgJumpTurnOn = FALSE;
				DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] %s line %d -- Pure Audio AU End\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO);
			}
		} else {
			if (DMX_IS_FF_PLAY(pvSptHdl)) {
				if (0 == prCfaFlv->u4PureAudSkipTagCnt)
					arCmdEntrys[0].fgEndAU = TRUE;
			} else {
				arCmdEntrys[0].fgEndAU = TRUE;
			}
		}

		rCfaFlvTxAudInfo.fgUseCmdQ = TRUE;
		rCfaFlvTxAudInfo.fgAUByCmdQEnd = TRUE;
		rCfaFlvTxAudInfo.parCmdQTxEntry = arCmdEntrys;
		rCfaFlvTxAudInfo.u2TxEntryCnt = 1;
		rCfaFlvTxAudInfo.fgAUCompleteByEnd = FALSE;

		rCfaFlvTxAudInfo.fgUnitStart = TRUE;
		rCfaFlvTxAudInfo.u8TotalAULen = u4TxLen;

		prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;

		prCfaFlv->eCurTxStrmType = CFA_FLV_TX_STRM_TYPE_AUD;

#if CFA_FLV_DBG_AUD_CMD_Q_FLOW
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_CMDQ_A,
			    TEXT("[CFA_FLV] %s line %d -- DmaAud (FileOfst: %lld, Len: %lld),")
			    TEXT(" Pts: %lld ms, fgEnd: %d, SkipCnt: %d, TotalAULen: %d\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, rCfaFlvTxAudInfo.u8FileOfst,
			    rCfaFlvTxAudInfo.u8Len, PTS_TO_MS(u8Pts), arCmdEntrys[0].fgEndAU,
			    prCfaFlv->u4PureAudSkipTagCnt, u4TxLen);
#endif				/* CFA_FLV_DBG_AUD_CMD_Q_FLOW*/

		mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rCfaFlvTxAudInfo);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] failed in Spt4CfaPbb2AFifoAUCtrl, mrRet: 0x%x\r\n"),
				    mrRet);
			CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, GAU_E_FAIL);
			MM_RETURN(mrRet);
		}
	}

	MM_RETURN(mrRet);
}
#endif

static MRESULT CfaFlvTxAudData2Fifo(void *pvSptHdl, CfaFlvInst_T *prCfaFlv)
{
	CFA_AUDIO_INFO_T rCfaFlvTxAudInfo = { 0 };
	CfaFlvAudCmdQInfo_T *prCmdQInfo = NULL;
	MRESULT mrRet = RET_DMX_OK;
	u64 u8Sa = 0;
	u32 u4TxLen = 0;
	u64 u8Pts = 0;

	u8Pts = MS_TO_PTS(prCfaFlv->rATimeStampMan.u4TimeStamp);

	prCmdQInfo = prCfaFlv->prAudCmdQsInfo;

	u8Sa = prCfaFlv->rAudioInfo.rATagInfo.u8Offset + (u64)FLV_TAG_HDR_SIZE + (u64)1;

	if (prCfaFlv->rAudioInfo.rATagInfo.u4DataSize > 0)
		u4TxLen = prCfaFlv->rAudioInfo.rATagInfo.u4DataSize - (u32)1;
	else
		u4TxLen = 0;

#if CFA_FLV_DBG_AUD_CMD_Q_FLOW
	DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_CMDQ_A,
		    TEXT("[CFA_FLV] %s line %d -- u4TxLen: %d, ")
		    TEXT("prCfaFlv->u8Ca: %lld, Sa: %lld, Tag's Ofst: %lld\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO, u4TxLen, prCfaFlv->u8Ca, u8Sa,
		    prCfaFlv->rAudioInfo.rATagInfo.u8Offset);
#endif				/* CFA_FLV_DBG_AUD_CMD_Q_FLOW*/

	if (prCfaFlv->fgFirstTxAud)
		prCfaFlv->fgFirstTxAud = FALSE;
	/*transfer audio data to audio fifo.*/
	prCfaFlv->eCurTxStrmType = CFA_FLV_TX_STRM_TYPE_AUD;

	if (AVCODEC_ID_AAC == prCfaFlv->rAudioInfo.rCfgInfo.eCodecID) {
		if (u4TxLen > 0) {
			u8Sa++;
			u4TxLen--;
		}
	}

	if (0 == u4TxLen) {
		prCfaFlv->fgTagIsValid = FALSE;
		CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
				   (u64)0, (u64)FLV_READ_FOR_CODEC_SIZE);
		MM_RETURN(RET_MSDKC_OK);
	}
#if !ENABLE_DMX_ADVANCED_VER
	if (AVCODEC_ID_AAC == prCfaFlv->rAudioInfo.rCfgInfo.eCodecID) {
		if (prCfaFlv->rRange.fgHasVid) {
			mm_memset(&rCfaFlvTxAudInfo, 0, sizeof(rCfaFlvTxAudInfo));
			rCfaFlvTxAudInfo.u8FileOfst = u8Sa;
			rCfaFlvTxAudInfo.u8Len = (u64) u4TxLen;
			rCfaFlvTxAudInfo.u8Pts = u8Pts;	/*change unit in Hz, STC Clock*/
			/* need modify */
			rCfaFlvTxAudInfo.u4PrsStrmId = (u32) prCfaFlv->rAudioInfo.rCfgInfo.u1StrmNum;
			rCfaFlvTxAudInfo.eAudType = prCfaFlv->rAudioInfo.eAudType;

			if (prCfaFlv->u8Endoffst <
			    (rCfaFlvTxAudInfo.u8FileOfst + rCfaFlvTxAudInfo.u8Len)) {
				DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] %s line %d -- EndOfst(%lld) < ")
					    TEXT("FileOfst(%lld) + Len(%lld), So call CfaFlvFinishPrs!!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, prCfaFlv->u8Endoffst,
					    rCfaFlvTxAudInfo.u8FileOfst, rCfaFlvTxAudInfo.u8Len);
				CfaFlvFinishPrs(pvSptHdl, prCfaFlv, (u32)GAU_E_EOS);
				MM_RETURN(RET_DMX_OK);
			}

			rCfaFlvTxAudInfo.fgUnitStart = FALSE;

#if CFA_FLV_DBG_AUD_CMD_Q_FLOW
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_CMDQ_A,
				    TEXT("[CFA_FLV] %s line %d -- DmaAud (FileOfst: %lld,")
				    TEXT(" Len: %lld, Pts: %lld ms)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, rCfaFlvTxAudInfo.u8FileOfst,
				    rCfaFlvTxAudInfo.u8Len, PTS_TO_MS(u8Pts));
#endif				/* CFA_FLV_DBG_AUD_CMD_Q_FLOW*/

			prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;

			prCfaFlv->eCurTxStrmType = CFA_FLV_TX_STRM_TYPE_AUD;

#if ENABLE_DMX_ADVANCED_VER
			rCfaFlvTxAudInfo.pu1InsertHdrBuf = prCfaFlv->rAudioInfo.pauAudHeader;
			rCfaFlvTxAudInfo.fgInsertHdr = TRUE;
			rCfaFlvTxAudInfo.u4InsertHdrLen = 7;
#endif				/* ENABLE_DMX_ADVANCED_VER*/
			rCfaFlvTxAudInfo.u8TotalAULen = 0;
			if ((0 < u4TxLen) &&
			    (prCfaFlv->rATimeStampMan.u4TimeStamp >= prCfaFlv->rRange.u8SeekTime)) {
				mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rCfaFlvTxAudInfo);
				if (DMX_FAILED(mrRet)) {
					DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						    TEXT("[CFA_FLV] failed in ")
						    TEXT("Spt4CfaPbb2AFifoAUCtrl(), mrRet: 0x%x\r\n"),
						    mrRet);
					CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, (u32)GAU_E_FAIL);
					MM_RETURN(mrRet);
				}
			} else {
				prCfaFlv->fgTagIsValid = FALSE;
				CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
						   0, FLV_READ_FOR_CODEC_SIZE);
			}
		} else {
			mrRet = CfaFlvTxAACData2Fifo(pvSptHdl, prCfaFlv, u8Sa, u4TxLen, u8Pts);
		}

		MM_RETURN(mrRet);
	}
#endif				/* ENABLE_DMX_ADVANCED_VER*/

	if (((prCmdQInfo->arEntrys[0].u8FileOffset != 0) &&
		(u8Sa + u4TxLen - prCmdQInfo->arEntrys[0].u8FileOffset > DMX_VID_TX_MAX_SIZE))
	    || ((prCmdQInfo->arEntrys[0].u8FileOffset == 0) && (u4TxLen > DMX_VID_TX_MAX_SIZE))
	    || (prCfaFlv->u4AvailDataSz < prCfaFlv->u8Ca - prCfaFlv->u8PreCa)
	    || ((!prCfaFlv->rRange.fgHasVid) && DMX_IS_RW_PLAY(pvSptHdl)
		&& (prCfaFlv->u4PureAudCurUnitTxSz + u4TxLen >= FLV_AUD_UNIT_MAX_SZ_IN_FFRW))
	    ) {
		if (0 == prCmdQInfo->u4EntryCnt) {
			mm_memset(&rCfaFlvTxAudInfo, 0, sizeof(rCfaFlvTxAudInfo));
			rCfaFlvTxAudInfo.u8FileOfst = u8Sa;
			rCfaFlvTxAudInfo.u8Len = (u64) u4TxLen;
			rCfaFlvTxAudInfo.u8Pts = u8Pts;	/*change unit in Hz, STC Clock */
			/* need modify */
			rCfaFlvTxAudInfo.u4PrsStrmId = (u32) prCfaFlv->rAudioInfo.rCfgInfo.u1StrmNum;
			rCfaFlvTxAudInfo.eAudType = prCfaFlv->rAudioInfo.eAudType;
			rCfaFlvTxAudInfo.fgUnitStart = TRUE;

			prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;
			prCfaFlv->eCurTxStrmType = CFA_FLV_TX_STRM_TYPE_AUD;

#if CFA_FLV_DBG_AUD_CMD_Q_FLOW
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_CMDQ_A,
				    TEXT("[CFA_FLV] %s line %d -- DmaAud (FileOfst: %lld, Len: %lld,")
				    TEXT(" Pts: %lld ms), TotalLen: %lld, TotalAULen: %lld\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, rCfaFlvTxAudInfo.u8FileOfst,
				    rCfaFlvTxAudInfo.u8Len, PTS_TO_MS(u8Pts),
				    prCmdQInfo->u8TotalLen, rCfaFlvTxAudInfo.u8Len);
#endif				/* CFA_FLV_DBG_AUD_CMD_Q_FLOW*/

			if (prCfaFlv->u8Endoffst <
			    (rCfaFlvTxAudInfo.u8FileOfst + rCfaFlvTxAudInfo.u8Len)) {
				DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] %s line %d -- EndOfst(%lld) < ")
					    TEXT("FileOfst(%lld) + Len(%lld), So call CfaFlvFinishPrs!!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, prCfaFlv->u8Endoffst,
					    rCfaFlvTxAudInfo.u8FileOfst, rCfaFlvTxAudInfo.u8Len);
				CfaFlvFinishPrs(pvSptHdl, prCfaFlv, GAU_E_EOS);
				MM_RETURN(RET_DMX_OK);
			}
#if ENABLE_DMX_ADVANCED_VER
			if (AVCODEC_ID_AAC == prCfaFlv->rAudioInfo.rCfgInfo.eCodecID) {
				rCfaFlvTxAudInfo.pu1InsertHdrBuf =
				    prCfaFlv->rAudioInfo.pauAudHeader;
				rCfaFlvTxAudInfo.fgInsertHdr = TRUE;
				rCfaFlvTxAudInfo.u4InsertHdrLen = 7;
			}
#endif				/* ENABLE_DMX_ADVANCED_VER*/
			rCfaFlvTxAudInfo.u8TotalAULen = rCfaFlvTxAudInfo.u8Len;

			mrRet = Spt4CfaPbb2AFifoAUCtrl(pvSptHdl, &rCfaFlvTxAudInfo);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] failed in ")
					    TEXT("Spt4CfaPbb2AFifoAUCtrl, mrRet: 0x%x\r\n"),
					    mrRet);
				CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, GAU_E_FAIL);
				MM_RETURN(mrRet);
			}

			MM_RETURN(mrRet);
		}

		prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;

		prCfaFlv->u8Ca = prCfaFlv->u8PreCa;

		/* DMA Cmd Q without this packet's data */
		mrRet = CfaFlvTxAudCmdQ(pvSptHdl, prCfaFlv, u4TxLen);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] failed in CfaFlvTxAudCmdQ, mrRet: 0x%x\r\n"),
				    mrRet);
			CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, GAU_E_FAIL);
			MM_RETURN(mrRet);
		}

		MM_RETURN(mrRet);
	} else {
		if (prCmdQInfo->u4EntryCnt < DMX_MAX_TX_CNT_FOR_CMD_Q) {
#if ENABLE_DMX_ADVANCED_VER
			if (AVCODEC_ID_AAC == prCfaFlv->rAudioInfo.rCfgInfo.eCodecID) {
				prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].fgInsertHdr = TRUE;
				prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].u4InsertHdrLen = 7;
				dmx_memcpy(prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].
					   au1InsertHdrBuf, prCfaFlv->rAudioInfo.pauAudHeader, 7);
			}
#endif				/*  ENABLE_DMX_ADVANCED_VER */
			prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].fgUnitStart = TRUE;
			prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].u4Len = u4TxLen;
			prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].u8FileOffset = u8Sa;
			prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].u8Pts = u8Pts;
			prCmdQInfo->u8TotalLen =
			    prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].u8FileOffset +
			    prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].u4Len -
			    prCmdQInfo->arEntrys[0].u8FileOffset;
			prCmdQInfo->u4EntryCnt++;
			prCmdQInfo->u4RealTxLen += u4TxLen;
		}

		if (prCmdQInfo->u4EntryCnt > 0) {
			if (!prCfaFlv->rRange.fgHasVid) {
				if (DMX_IS_RW_PLAY(pvSptHdl)) {
					if (0 == prCfaFlv->u4PureAudCurUnitTxSz) {
						prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt -
								     1].fgUnitStart = TRUE;
					} else {
						prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt -
								     1].fgUnitStart = FALSE;
					}
					prCfaFlv->u4PureAudCurUnitTxSz += u4TxLen;
				} else {
					if (1 == prCmdQInfo->u4EntryCnt) {
						prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt -
								     1].fgUnitStart = TRUE;
					} else {
						prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt -
								     1].fgUnitStart = FALSE;
					}
				}
			}
#if CFA_FLV_DBG_AUD_CMD_Q_FLOW
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_CMDQ_A,
				    TEXT("[CFA_FLV] %s line %d -- (ACmdIdx: %d, FileOfst: %lld,")
				    TEXT(" Len: %d, Pts: %lld ms, u4AvailDataSz: %d)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, (prCmdQInfo->u4EntryCnt - 1),
				    prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt - 1].u8FileOffset,
				    prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt - 1].u4Len,
				    PTS_TO_MS(prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt - 1].
					      u8Pts), prCfaFlv->u4AvailDataSz);
#endif				/*  CFA_FLV_DBG_AUD_CMD_Q_FLOW */
		}

		prCfaFlv->eCurTxStrmType = CFA_FLV_TX_STRM_TYPE_AUD;

		if (prCfaFlv->u4AvailDataSz <
		    prCfaFlv->u8Ca - prCfaFlv->u8PreCa + FLV_READ_FOR_CODEC_SIZE) {
			prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_TX_AV_CMDQ;
			prCfaFlv->u4AvailDataSz = 0;
			mrRet = CfaFlvTxAudCmdQ(pvSptHdl, prCfaFlv, u4TxLen);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] failed in CfaFlvTxAudCmdQ, mrRet: 0x%x\r\n"),
					    mrRet);
				CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, GAU_E_FAIL);
				MM_RETURN(mrRet);
			}
		} else if ((prCmdQInfo->u4EntryCnt >= DMX_MAX_TX_CNT_FOR_CMD_Q)
			   || ((!prCfaFlv->rRange.fgHasVid)
			       && (DMX_IS_RW_PLAY(pvSptHdl)
				   && (prCfaFlv->u4PureAudCurUnitTxSz >=
				       FLV_AUD_UNIT_MAX_SZ_IN_FFRW)))
		    ) {
			prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;
			mrRet = CfaFlvTxAudCmdQ(pvSptHdl, prCfaFlv, u4TxLen);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] failed in CfaFlvTxAudCmdQ, mrRet: 0x%x\r\n"),
					    mrRet);
				CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, GAU_E_FAIL);
				MM_RETURN(mrRet);
			}
		} else {
			prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;
			prCfaFlv->fgNoNeedSyncPb = TRUE;
		}
	}

	MM_RETURN(mrRet);
}

void CfaFlvSetAacHdr(CfaFlvInst_T *prCfaFlv)
{
	u32 u4SamplePerSec = 0;
	u32 u4FrameSize = 0;
	u32 u4Channels = 0;
	u8 u1Temp1 = 0;
	u8 u1Temp2 = 0;

	u4FrameSize = prCfaFlv->rAudioInfo.rATagInfo.u4DataSize - (u32)2 + (u32)7;

	u1Temp1 = prCfaFlv->rAacCfgInfo.puHeader[4];
	u1Temp2 = prCfaFlv->rAacCfgInfo.puHeader[5];

	u4SamplePerSec = ((u1Temp1 & 0x07) << 1U) | ((u1Temp2 >> 7U) & 0x01);
	u4Channels = (u1Temp2 >> 3U) & 0x0F;	/*  Audio Channel Count */

	if (CFA_AUDIO_TYPE_MONO == prCfaFlv->rAudioInfo.rCfgInfo.eSoundType)
		u4Channels = (u32)1;
	else if (CFA_AUDIO_TYPE_STEREO == prCfaFlv->rAudioInfo.rCfgInfo.eSoundType)	/* aac is stero */
		u4Channels = (u32)2;
	else {
		/*do nothing*/
	}

	if (NULL != prCfaFlv->rAudioInfo.pauAudHeader) {
		prCfaFlv->rAudioInfo.pauAudHeader[0] = (u8) 0xFF;
		prCfaFlv->rAudioInfo.pauAudHeader[1] = (u8) 0xF9;
		prCfaFlv->rAudioInfo.pauAudHeader[2] =
		    (u8) ((1 << 6) | ((u4SamplePerSec << 2) & 0x3C) | ((u4Channels >> 2) & 0x1));
		prCfaFlv->rAudioInfo.pauAudHeader[3] =
		    (u8) (((u4Channels & 0x3) << 6) | ((u4FrameSize >> 11) & 0x3));
		prCfaFlv->rAudioInfo.pauAudHeader[4] = (u8) (((u4FrameSize >> 3) & 0xFF));
		prCfaFlv->rAudioInfo.pauAudHeader[5] =
		    (u8) (((u4FrameSize << 5) & 0xE0) | ((0x7FF >> 6) & 0x1F));
		prCfaFlv->rAudioInfo.pauAudHeader[6] = (u8) (((0x7FF << 2) & 0xFC));
	}
}

static MRESULT CfaFlvTxAudHdl2Fifo(void *pvSptHdl, CfaFlvInst_T *prCfaFlv)
{
	MRESULT mrRet = RET_DMX_OK;

	if (CFA_FLV_TAG_TYPE_AUDIO != prCfaFlv->rTagInfo.eTagType) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			    TEXT("[CFA_FLV] fail for invalid args, rTagInfo.eTagType(%d) != Video\r\n"),
			    prCfaFlv->rTagInfo.eTagType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
#if ENABLE_DMX_ADVANCED_VER
	if (AVCODEC_ID_AAC == prCfaFlv->rAudioInfo.rCfgInfo.eCodecID)
		CfaFlvSetAacHdr(prCfaFlv);
#if CFA_FLV_DBG_AUD_CMD_Q_FLOW
	DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_CMDQ_A,
			TEXT("[CFA_FLV] %s line %d -- CfaFlvTxAudData2Fifo\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO);
#endif				/* CFA_FLV_DBG_AUD_CMD_Q_FLOW */

	mrRet = CfaFlvTxAudData2Fifo(pvSptHdl, prCfaFlv);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			    TEXT("[CFA_FLV] failed in CfaFlvTxAudData2Fifo, mrRet: 0x%x, call FinishPrs\r\n"),
			    mrRet);
		MM_RETURN(mrRet);
	}
#else				/* ENABLE_DMX_ADVANCED_VER */

	if (AVCODEC_ID_AAC == prCfaFlv->rAudioInfo.rCfgInfo.eCodecID) {
		/* Tx AAC Sub Tag dec Header */
		CfaFlvSetAacHdr(prCfaFlv);

		prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_TX_AUD;

#if CFA_FLV_DBG_AUD_CMD_Q_FLOW
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_CMDQ_A,
			    TEXT("[CFA_FLV] %s line %d -- Tx AAC Header (Len: %d)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, 7);
#endif				/* CFA_FLV_DBG_AUD_CMD_Q_FLOW */

		mrRet = Spt4CfaBuf2AFifo(pvSptHdl, prCfaFlv->rAudioInfo.pauAudHeader,
					 (u64)7, (u32)(prCfaFlv->rAudioInfo.rCfgInfo.u1StrmNum),
					 prCfaFlv->rAudioInfo.eAudType);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] failed in Spt4CfaBuf2AFifo,")
				    TEXT(" mrRet: 0x%x, call FinishPrs\r\n"),
				    mrRet);
			CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, (u32)GAU_E_FAIL);
			MM_RETURN(mrRet);
		}
	} else {

#if CFA_FLV_DBG_AUD_CMD_Q_FLOW
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_CMDQ_A,
				TEXT("[CFA_FLV] %s line %d -- CfaFlvTxAudData2Fifo\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
#endif				/* CFA_FLV_DBG_AUD_CMD_Q_FLOW */

		mrRet = CfaFlvTxAudData2Fifo(pvSptHdl, prCfaFlv);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] failed in CfaFlvTxAudData2Fifo,")
				    TEXT(" mrRet: 0x%x, call FinishPrs\r\n"),
				    mrRet);
			MM_RETURN(mrRet);
		}
	}
#endif				/* !ENABLE_DMX_ADVANCED_VER */

	MM_RETURN(mrRet);
}

static MRESULT CfaFlvTxAudCfg2Fifo(void *pvSptHdl, CfaFlvInst_T *prCfaFlv)
{
	MRESULT mrRet = RET_DMX_OK;

	prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_TAG_HEADER;

	if (CFA_FLV_TAG_TYPE_AUDIO != prCfaFlv->rTagInfo.eTagType) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			    TEXT("[CFA_FLV] fail for invalid args, ")
			    TEXT("rTagInfo.eTagType(%d) != Video\r\n"),
			    prCfaFlv->rTagInfo.eTagType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (AVCODEC_ID_AAC == prCfaFlv->rAudioInfo.rCfgInfo.eCodecID) {
		if (prCfaFlv->fgFirstTxAud)	{
			/* Tx AAC Total dec Header */
#if CFA_FLV_DBG_AUD_CMD_Q_FLOW
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_CMDQ_A,
				    TEXT("[CFA_FLV] %s line %d -- rAacCfgInfo.uHeaderLen(%d)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prCfaFlv->rAacCfgInfo.uHeaderLen);
#endif				/* CFA_FLV_DBG_AUD_CMD_Q_FLOW */

			prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_TX_A_HDR;
			mrRet = Spt4CfaBuf2AFifo(pvSptHdl, prCfaFlv->rAacCfgInfo.puHeader,
						 prCfaFlv->rAacCfgInfo.uHeaderLen,
						 (prCfaFlv->rAudioInfo.rCfgInfo.u1StrmNum),
						 prCfaFlv->rAudioInfo.eAudType);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] failed in Spt4CfaBuf2AFifo,")
					    TEXT(" mrRet: 0x%x, call FinishPrs\r\n"),
					    mrRet);
				CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, (u32)GAU_E_FAIL);
				MM_RETURN(mrRet);
			}
		} else	{
			/* Tx AAC Sub Tag dec Header */
			prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_TX_AUD;
			mrRet = CfaFlvTxAudHdl2Fifo(pvSptHdl, prCfaFlv);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] failed in CfaFlvTxAudHdl2Fifo,")
					    TEXT(" mrRet: 0x%x, call FinishPrs\r\n"),
					    mrRet);
				MM_RETURN(mrRet);
			}
			MM_RETURN(mrRet);
		}
	} else {
		/* Tx Audio Data */
		mrRet = CfaFlvTxAudHdl2Fifo(pvSptHdl, prCfaFlv);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] failed in ")
				    TEXT("CfaFlvTxAudHdl2Fifo, mrRet: 0x%x, call FinishPrs\r\n"),
				    mrRet);
			MM_RETURN(mrRet);
		}
		MM_RETURN(mrRet);
	}

	MM_RETURN(mrRet);
}

static MRESULT CfaFlvTxStartCode2Fifo(void *pvSptHdl, CfaFlvInst_T *prCfaFlv)
{
	MRESULT mrRet = RET_DMX_OK;

	if ((CFA_FLV_TAG_TYPE_VIDEO == prCfaFlv->rTagInfo.eTagType) &&
	    ((AVCODEC_ID_H264== prCfaFlv->rVideoInfo.rCfgInfo.eCodecID) ||
	     (VCODEC_VERSION_H265_HM91 == prCfaFlv->rVideoInfo.rCfgInfo.rVersion))) {
		mrRet = Spt4CfaBuf2VFifo(pvSptHdl, prCfaFlv->puDecoderCfgBuf, (u64)0,
					 CFA_PTM_EXACT_POS, prCfaFlv->eVidCodecType, (u64)3);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] failed in Spt4CfaBuf2VFifo, mrRet: 0x%x, call FinishPrs\r\n"),
				    mrRet);
			CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, (u32)GAU_E_FAIL);
			MM_RETURN(mrRet);
		}
	}

	MM_RETURN(RET_DMX_OK);
}

static void CfaFlvCalcVidPts(void *pvSptHdl, CfaFlvInst_T *prCfaFlv)
{
	FLV_TIMESTAMP_MAN_T *prVTimeMan = NULL;
	FLV_TIMESTAMP_MAN_T *prATimeMan = NULL;
	u32 u4BaseTime = 0;
	u32 u4CurTime = 0;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prCfaFlv) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("[CFA_FLV] failed for invalid args\r\n"));
		return;
	}
#if CFA_FLV_DBG_VID_PTS
	DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_PTS_V,
			TEXT("[CFA_FLV] %s line %d enter\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO);
#endif				/* CFA_FLV_DBG_VID_PTS */

	u4CurTime = prCfaFlv->rVideoInfo.rVTagInfo.u4TimeStamp;

	prVTimeMan = &(prCfaFlv->rVTimeStampMan);

	prATimeMan = &(prCfaFlv->rATimeStampMan);

	mrRet = CfaFlvCorrectAUPts(SPT_DATA_V, pvSptHdl, prCfaFlv, &u4CurTime);

	if (DMX_INVALID_UINT32 == prVTimeMan->u4BaseTime)
		u4BaseTime = prATimeMan->u4BaseTime;
	else
		u4BaseTime = prVTimeMan->u4BaseTime;

	if (DMX_INVALID_UINT32 != u4CurTime) {
		/* compute final audio PTS */
		if (u4CurTime >= u4BaseTime) {
			prVTimeMan->u4TimeStamp =
			    u4CurTime - u4BaseTime + prVTimeMan->u4TimeIncrement +
			    prVTimeMan->u4SeekTime;
#if CFA_FLV_DBG_AUD_PTS
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_PTS_A,
				    TEXT("[CFA_FLV] %s line %d -- Video TimeStamp: %d, Real TimeStamp: ")
				    TEXT("%d, AIncre: %d, VIncre: %d, Cur: %d, Base: %d, ")
				    TEXT("VSeekTime: %d, SeekTime: %d\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prVTimeMan->u4TimeStamp,
				    prCfaFlv->rVideoInfo.rVTagInfo.u4TimeStamp,
				    prATimeMan->u4TimeIncrement, prVTimeMan->u4TimeIncrement,
				    u4CurTime, u4BaseTime, prVTimeMan->u4SeekTime,
				    (u32) (prCfaFlv->rRange.u8SeekTime));
#endif				/* CFA_FLV_DBG_AUD_PTS */
			return;
		}
	}

	if (DMX_INVALID_UINT32 == prVTimeMan->u4TimeStamp)
		prVTimeMan->u4TimeStamp = prVTimeMan->u4SeekTime;

	if (DMX_INVALID_UINT32 != prVTimeMan->u4DeltaTime)
		prVTimeMan->u4TimeStamp += prVTimeMan->u4DeltaTime;
	else
		prVTimeMan->u4TimeStamp += CFA_FLV_30FPS_FRAME_DURATION;

#if CFA_FLV_DBG_VID_PTS
	DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_PTS_V,
		    TEXT("[CFA_FLV] %s line %d -- Video TimeStamp: %d, Real TimeStamp: ")
		    TEXT("%d, VSeekTime: %d, SeekTime: %d\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO, prVTimeMan->u4TimeStamp,
		    prCfaFlv->rVideoInfo.rVTagInfo.u4TimeStamp, prVTimeMan->u4SeekTime,
		    (u32) (prCfaFlv->rRange.u8SeekTime));
#endif				/* CFA_FLV_DBG_VID_PTS */

}

static MRESULT CfaFlvTxVidDataByCmdQ(void *pvSptHdl, CfaFlvInst_T *prCfaFlv,
				     u64 u8Sa, u32 u4TxLen, u64 u8Pts, bool fgAUEnd)
{
	CFA_VIDEO_INFO_T rCfaFlvTxVidInfo = { 0 };
	CfaFlvVidCmdQInfo_T *prCmdQInfo = prCfaFlv->prVidCmdQsInfo;
	MRESULT mrRet = RET_DMX_OK;

	mm_memset(&rCfaFlvTxVidInfo, 0, sizeof(rCfaFlvTxVidInfo));

	if (((prCmdQInfo->arEntrys[0].u8FileOffset != 0) &&
		(u8Sa + u4TxLen - prCmdQInfo->arEntrys[0].u8FileOffset > DMX_VID_TX_MAX_SIZE))
	    || ((prCmdQInfo->arEntrys[0].u8FileOffset == 0) && (u4TxLen > DMX_VID_TX_MAX_SIZE))
	    || ((u64)(prCfaFlv->u4AvailDataSz) < prCfaFlv->u8Ca - prCfaFlv->u8PreCa)) {
		if (0 == prCmdQInfo->u4EntryCnt) {
			rCfaFlvTxVidInfo.u8FileOfst = u8Sa;
			rCfaFlvTxVidInfo.eVidType = prCfaFlv->eVidCodecType;

			rCfaFlvTxVidInfo.eTxMode = CfaFlvSetPicTxMode(prCfaFlv);
			rCfaFlvTxVidInfo.u4PrsStrmId =
			    (u32) prCfaFlv->rVideoInfo.rCfgInfo.u1StrmNum;
			rCfaFlvTxVidInfo.u8Len = (u64) u4TxLen;

			prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;

			prCfaFlv->eCurTxStrmType = CFA_FLV_TX_STRM_TYPE_VID;

			if (prCfaFlv->u8Endoffst <
			    (rCfaFlvTxVidInfo.u8FileOfst + rCfaFlvTxVidInfo.u8Len)) {
				DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] %s line %d -- EndOfst(%lld) < ")
					    TEXT("FileOfst(%lld) + Len(%lld), So call CfaFlvFinishPrs!!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, prCfaFlv->u8Endoffst,
					    rCfaFlvTxVidInfo.u8FileOfst, rCfaFlvTxVidInfo.u8Len);
				CfaFlvFinishPrs(pvSptHdl, prCfaFlv, (u32)GAU_E_EOS);
				MM_RETURN(RET_DMX_OK);
			}
#if CFA_FLV_DBG_VID_CMD_Q_FLOW
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_CMDQ_V,
				    TEXT("[CFA_FLV] %s line %d -- DmaVid (VCmdCnt: 0, FileOfst: %lld,")
				    TEXT(" Len: %lld), Ca: %lld, Pts: %lld ms\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, rCfaFlvTxVidInfo.u8FileOfst,
				    rCfaFlvTxVidInfo.u8Len, prCfaFlv->u8Ca,
				    (u64) (prCfaFlv->rVTimeStampMan.u4TimeStamp));
#endif				/* CFA_FLV_DBG_VID_CMD_Q_FLOW */

			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d Spt4CfaPbb2VFifoAUCtrl!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
			mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rCfaFlvTxVidInfo);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] failed in ")
					    TEXT("Spt4CfaPbb2VFifoAUCtrl(), mrRet: 0x%x\r\n"),
					    mrRet);
				CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, (u32)GAU_E_FAIL);
				MM_RETURN(mrRet);
			}
			MM_RETURN(mrRet);
		}

		prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;

		prCfaFlv->u8Ca = prCfaFlv->u8PreCa;

		/* DMA Cmd Q without this packet's data */
		mrRet = CfaFlvTxVidCmdQ(pvSptHdl, prCfaFlv, u4TxLen);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] failed in CfaFlvTxVidCmdQ, mrRet: 0x%x\r\n"),
				    mrRet);
			CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, (u32)GAU_E_FAIL);
			MM_RETURN(mrRet);
		}

		MM_RETURN(mrRet);
	} else {
		if (prCmdQInfo->u4EntryCnt < DMX_MAX_TX_CNT_FOR_CMD_Q) {
			prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].fgUnitStart = TRUE;
			prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].u4Len = u4TxLen;
			prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].u8FileOffset = u8Sa;
			prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].u8Pts = u8Pts;

#if ENABLE_DMX_ADVANCED_VER
			prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].fgEndAU = fgAUEnd;
			prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].eTxMode =
			    CfaFlvSetPicTxMode(prCfaFlv);
#else
			prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].fgEndAU = FALSE;
			prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].eTxMode = CFA_PTM_EXACT_POS;
#endif				/* ENABLE_DMX_ADVANCED_VER */

			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_CMDQ_V,
				TEXT("[CFA_FLV] %s line %d -- (VCmdIdx: %d, FileOfst: ")
				TEXT("0x%08x%08x, Len: 0x%08x, eTxMode: %d, FrmType: %d,")
				TEXT(" fgEndAU: %d, availsz: 0x%08x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prCmdQInfo->u4EntryCnt,
				(u32) ((prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].
					   u8FileOffset) >> 32),
				(u32) (prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].
					  u8FileOffset),
				prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].u4Len,
				prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].eTxMode,
				prCfaFlv->rVideoInfo.eFrmType,
				prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].fgEndAU,
				prCfaFlv->u4AvailDataSz);

			prCmdQInfo->u8TotalLen =
			    prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].u8FileOffset +
			    prCmdQInfo->arEntrys[prCmdQInfo->u4EntryCnt].u4Len -
			    prCmdQInfo->arEntrys[0].u8FileOffset;
			prCmdQInfo->u4EntryCnt++;
			prCmdQInfo->u4RealTxLen += u4TxLen;
		}

		prCfaFlv->eCurTxStrmType = CFA_FLV_TX_STRM_TYPE_VID;

		if (prCfaFlv->u4AvailDataSz <
		    prCfaFlv->u8Ca - prCfaFlv->u8PreCa + FLV_READ_FOR_CODEC_SIZE) {
			prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_TX_AV_CMDQ;
			prCfaFlv->u4AvailDataSz = 0;
			mrRet = CfaFlvTxVidCmdQ(pvSptHdl, prCfaFlv, u4TxLen);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] failed in CfaFlvTxVidCmdQ, mrRet: 0x%x\r\n"),
					    mrRet);
				CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, GAU_E_FAIL);
				MM_RETURN(mrRet);
			}
		} else if (prCmdQInfo->u4EntryCnt >= DMX_MAX_TX_CNT_FOR_CMD_Q) {
			prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;
			mrRet = CfaFlvTxVidCmdQ(pvSptHdl, prCfaFlv, u4TxLen);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] failed in CfaFlvTxVidCmdQ, mrRet: 0x%x\r\n"),
					    mrRet);
				CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, GAU_E_FAIL);
				MM_RETURN(mrRet);
			}
		} else {
			prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;
			prCfaFlv->fgNoNeedSyncPb = TRUE;
		}
	}

	MM_RETURN(mrRet);
}

static MRESULT CfaFlvTxVidData2Fifo(void *pvSptHdl, CfaFlvInst_T *prCfaFlv)
{
	CFA_VIDEO_INFO_T rCfaFlvTxVidInfo;
	MRESULT mrRet = RET_DMX_OK;
	u64 u8Sa = 0;
	u32 u4TxLen = 0;
	u64 u8Pts = 0;

	mm_memset(&rCfaFlvTxVidInfo, 0, sizeof(CFA_VIDEO_INFO_T));

	if ((AVCODEC_ID_H264 == prCfaFlv->rVideoInfo.rCfgInfo.eCodecID) ||
	    (VCODEC_VERSION_H265_HM91 == prCfaFlv->rVideoInfo.rCfgInfo.rVersion)) {
		if (CFA_FLV_ANA_ST_TX_PARTTAG_VIDEO == prCfaFlv->eCurAnaSt) {
			u8Sa = prCfaFlv->rVideoInfo.rVTagInfo.u8Offset + (u64)FLV_TAG_HDR_SIZE +
			    (u64)FLV_VID_AVC_DATA_HDR + (u64)prCfaFlv->u4PayloadLenFieldSz +
			    (u64)(prCfaFlv->u4VideoPartTagOffset);
			u8Pts = MS_TO_PTS(prCfaFlv->rVTimeStampMan.u4TimeStamp);
			prCfaFlv->u4VideoPartTagOffset +=
			    prCfaFlv->u4FirstUintPay + prCfaFlv->u4PayloadLenFieldSz;
		} else {
			/* u8Sa = prCfaFlv->rVideoInfo.rVTagInfo.u8Offset + FLV_VID_TAG_AVC_SKIP; */
			u8Sa = prCfaFlv->rVideoInfo.rVTagInfo.u8Offset +
			    (u64)FLV_TAG_HDR_SIZE + (u64)FLV_VID_AVC_DATA_HDR + (u64)(prCfaFlv->u4PayloadLenFieldSz);
			u8Pts = MS_TO_PTS(prCfaFlv->rVTimeStampMan.u4TimeStamp);
		}

		if (prCfaFlv->rVideoInfo.rVTagInfo.u4DataSize >
		    FLV_VID_AVC_DATA_HDR + prCfaFlv->u4PayloadLenFieldSz) {
			u4TxLen =
			    prCfaFlv->rVideoInfo.rVTagInfo.u4DataSize - ((u32)FLV_VID_AVC_DATA_HDR +
									 (u32)(prCfaFlv->
									 u4PayloadLenFieldSz));
		} else {
			u4TxLen = 0;
		}

		if (CFA_FLV_ANA_ST_TX_PARTTAG_VIDEO == prCfaFlv->eCurAnaSt) {
			u4TxLen = prCfaFlv->u4FirstUintPay;	/* txlen = slice len */
			if ((prCfaFlv->u4VideoPartTagOffset + FLV_VID_AVC_DATA_HDR +
			     prCfaFlv->u4PayloadLenFieldSz) <
			    prCfaFlv->rVideoInfo.rVTagInfo.u4DataSize) {
				/* the reason to set eCurAnaSt to be CFA_FLV_ANA_ST_TX_START_CODE is that */
				/* after tx the slice data, we will tx AVC start code "00 00 01" */
				prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_TX_START_CODE;
				/* get the next slice's data size */
				prCfaFlv->u4FirstUintPay = CfaFlvGetSliceSize(prCfaFlv,
									      prCfaFlv->pu1HdrBuf +
									      prCfaFlv->
									      u4VideoPartTagOffset);
				/* get the next slice's data's first byte */
				LOAD_BYTE(prCfaFlv->pu1HdrBuf + prCfaFlv->u4VideoPartTagOffset +
					  prCfaFlv->u4PayloadLenFieldSz, prCfaFlv->u1FirstBytePay);
				/* if this is the last slice in the video tag data,
				* we will set eCurAnast to be SEARCH_HEADER */
				/* then after the last slice transferred completely,
				* the process will run to search the next tag.    */
				if ((prCfaFlv->u4VideoPartTagOffset + FLV_VID_AVC_DATA_HDR +
				     prCfaFlv->u4PayloadLenFieldSz + prCfaFlv->u4FirstUintPay) >
				    prCfaFlv->rVideoInfo.rVTagInfo.u4DataSize) {
					prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;
					DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						    TEXT("[CFA_FLV] video tag data error,u8Sa=0x%llx, ")
						    TEXT("u4DataSize=%d, SliceSize=%d!!!\r\n"),
						    u8Sa, prCfaFlv->rVideoInfo.rVTagInfo.u4DataSize,
						    prCfaFlv->u4FirstUintPay);
				} else {
					/* tx slice data whose size is u4TxLen. */
				}
			} else {
				prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;
			}
		}
#if CFA_FLV_DBG_VID_CMD_Q_FLOW
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_PTS_V,
			    TEXT("[CFA_FLV] %s line %d -- eCurAnaSt: %d, Sa: %lld,")
			    TEXT(" u4TxLen: %d, FirstUintPay: %d, PartTagOfst: %d, DataSize: %d)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prCfaFlv->eCurAnaSt, u8Sa, u4TxLen,
			    prCfaFlv->u4FirstUintPay, prCfaFlv->u4VideoPartTagOffset,
			    prCfaFlv->rVideoInfo.rVTagInfo.u4DataSize);
#endif				/* CFA_FLV_DBG_AUD_CMD_Q_FLOW */

		if (prCfaFlv->fgFirstTxVid)
			prCfaFlv->fgFirstTxVid = FALSE;
		/* transfer video data to video fifo. */
		prCfaFlv->eCurTxStrmType = CFA_FLV_TX_STRM_TYPE_VID;

		rCfaFlvTxVidInfo.u8FileOfst = u8Sa;
		rCfaFlvTxVidInfo.eVidType = prCfaFlv->eVidCodecType;

		rCfaFlvTxVidInfo.eTxMode = CfaFlvSetPicTxMode(prCfaFlv);
		rCfaFlvTxVidInfo.u4PrsStrmId = (u32) prCfaFlv->rVideoInfo.rCfgInfo.u1StrmNum;
		rCfaFlvTxVidInfo.u8Len = (u64) u4TxLen;

		CfaFlvCalcVidPts(pvSptHdl, prCfaFlv);

		prCfaFlv->rVTimeStampMan.u8TotalTagCnt++;

		if (prCfaFlv->u8Endoffst < (rCfaFlvTxVidInfo.u8FileOfst + rCfaFlvTxVidInfo.u8Len)) {
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d -- EndOfst(%lld) < FileOfst(%lld)")
				    TEXT(" + Len(%lld), So call CfaFlvFinishPrs!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prCfaFlv->u8Endoffst,
				    rCfaFlvTxVidInfo.u8FileOfst, rCfaFlvTxVidInfo.u8Len);
			CfaFlvFinishPrs(pvSptHdl, prCfaFlv, (u32)GAU_E_EOS);
			MM_RETURN(RET_DMX_OK);
		}

		if (0 < rCfaFlvTxVidInfo.u8Len) {
#if CFA_FLV_DBG_VID_CMD_Q_FLOW
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_PTS_V,
				    TEXT("[CFA_FLV] %s line %d -- DmaVid (eCurAnaSt: %d, ")
				    TEXT("VCmdCnt: 0, FileOfst: %lld, Len: %lld), Ca: %lld, Pts: %lld ms\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prCfaFlv->eCurAnaSt,
				    rCfaFlvTxVidInfo.u8FileOfst, rCfaFlvTxVidInfo.u8Len,
				    prCfaFlv->u8Ca,
				    (u64) (prCfaFlv->rVTimeStampMan.u4TimeStamp));
#endif				/* CFA_FLV_DBG_AUD_CMD_Q_FLOW */

			mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rCfaFlvTxVidInfo);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] failed in ")
					    TEXT("Spt4CfaPbb2VFifoAUCtrl(), mrRet: 0x%x\r\n"),
					    mrRet);
				CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, (u32)GAU_E_FAIL);
				MM_RETURN(mrRet);
			}
		} else {
			prCfaFlv->fgTagIsValid = FALSE;
			CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
					   (u64)0, (u64)FLV_READ_FOR_CODEC_SIZE);
		}
	} else if ((AVCODEC_ID_H265 == prCfaFlv->rVideoInfo.rCfgInfo.eCodecID) &&
		(VCODEC_VERSION_NONE== prCfaFlv->rVideoInfo.rCfgInfo.rVersion)){
		u8Sa =
		    prCfaFlv->rVideoInfo.rVTagInfo.u8Offset + FLV_TAG_HDR_SIZE +
		    FLV_VID_AVC_DATA_HDR;

		CfaFlvCalcVidPts(pvSptHdl, prCfaFlv);

		prCfaFlv->rVTimeStampMan.u8TotalTagCnt++;

		u8Pts = MS_TO_PTS(prCfaFlv->rVTimeStampMan.u4TimeStamp);

		if (prCfaFlv->rVideoInfo.rVTagInfo.u4DataSize > 0)
			u4TxLen = prCfaFlv->rVideoInfo.rVTagInfo.u4DataSize - FLV_VID_AVC_DATA_HDR;
		else
			u4TxLen = 0;

		if (prCfaFlv->fgFirstTxVid)
			prCfaFlv->fgFirstTxVid = FALSE;

		if (0 == u4TxLen) {
			prCfaFlv->fgTagIsValid = FALSE;
			CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
					   0, FLV_READ_FOR_CODEC_SIZE);
			MM_RETURN(RET_MSDKC_OK);
		}
		/* transfer video data to video fifo. */
		prCfaFlv->eCurTxStrmType = CFA_FLV_TX_STRM_TYPE_VID;

		mrRet = CfaFlvTxVidDataByCmdQ(pvSptHdl, prCfaFlv, u8Sa, u4TxLen, u8Pts, FALSE);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] failed in CfaFlvTxVidDataByCmdQ, mrRet: 0x%x\r\n"),
				    mrRet);
			CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, GAU_E_FAIL);
			MM_RETURN(mrRet);
		}
	} else if (Spt4CfaIsNonHdrDectVCodec(prCfaFlv->eVidCodecType)) {
		u32 u4TxLocalOffset = 1;	/* Skip FrameType[4bit] and CodecID(4bit) */

		/* Skip FrameType[4bit] and CodecID(4bit), HorizontalAdjustment[4bit] and VerticalAdjustment[4bit] */
		if (CFA_VID_VP6 == prCfaFlv->eVidCodecType)
			u4TxLocalOffset = 2;
		/* Skip FrameType[4bit] and CodecID(4bit), HorizontalAdjustment[4bit] and VerticalAdjustment[4bit] */
		if (CFA_VID_VP6A == prCfaFlv->eVidCodecType)
			u4TxLocalOffset = 2;

		u8Sa = prCfaFlv->rVideoInfo.rVTagInfo.u8Offset + FLV_TAG_HDR_SIZE + u4TxLocalOffset;

		CfaFlvCalcVidPts(pvSptHdl, prCfaFlv);

		prCfaFlv->rVTimeStampMan.u8TotalTagCnt++;

		u8Pts = MS_TO_PTS(prCfaFlv->rVTimeStampMan.u4TimeStamp);

		if (prCfaFlv->rVideoInfo.rVTagInfo.u4DataSize > 0)
			u4TxLen = prCfaFlv->rVideoInfo.rVTagInfo.u4DataSize - u4TxLocalOffset;
		else
			u4TxLen = 0;

		if (prCfaFlv->fgFirstTxVid)
			prCfaFlv->fgFirstTxVid = FALSE;

		if (0 == u4TxLen) {
			prCfaFlv->fgTagIsValid = FALSE;
			CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
					   0, FLV_READ_FOR_CODEC_SIZE);
			MM_RETURN(RET_MSDKC_OK);
		}
#if ENABLE_DMX_ADVANCED_VER


#if CFA_FLV_DBG_VID_CMD_Q_FLOW
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_PTS_V,
			    TEXT("[CFA_FLV] %s line %d -- DmaVid (VCmdCnt: 0, ")
			    TEXT("FileOfst: %lld, Len: %lld), Ca: %lld, Pts: %lld ms\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, rCfaFlvTxVidInfo.u8FileOfst,
			    rCfaFlvTxVidInfo.u8Len, prCfaFlv->u8Ca,
			    (u64) (prCfaFlv->rVTimeStampMan.u4TimeStamp));
#endif				/* CFA_FLV_DBG_VID_CMD_Q_FLOW */

		if (prCfaFlv->rRange.u8IframeNum >= 1) {
			/* transfer video data to video fifo. */
			prCfaFlv->eCurTxStrmType = CFA_FLV_TX_STRM_TYPE_VID;

			mrRet = CfaFlvTxVidDataByCmdQ(pvSptHdl, prCfaFlv, u8Sa, u4TxLen, u8Pts, TRUE);

			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] failed in")
					    TEXT(" CfaFlvTxVidDataByCmdQ, mrRet: 0x%x\r\n"),
					    mrRet);
				CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, GAU_E_FAIL);
				MM_RETURN(mrRet);
			}
		} else {
			prCfaFlv->fgTagIsValid = FALSE;
			CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
					   0, FLV_READ_FOR_CODEC_SIZE);
		}

#else				/* ENABLE_DMX_ADVANCED_VER */

		rCfaFlvTxVidInfo.u8FileOfst = u8Sa;
		rCfaFlvTxVidInfo.eVidType = prCfaFlv->eVidCodecType;

		rCfaFlvTxVidInfo.eTxMode = CfaFlvSetPicTxMode(prCfaFlv);
		rCfaFlvTxVidInfo.u4PrsStrmId = (u32) prCfaFlv->rVideoInfo.rCfgInfo.u1StrmNum;
		rCfaFlvTxVidInfo.u8Len = (u64) u4TxLen;

		prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;

		if (prCfaFlv->u8Endoffst < (rCfaFlvTxVidInfo.u8FileOfst + rCfaFlvTxVidInfo.u8Len)) {
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d -- EndOfst(%lld) < FileOfst(%lld)")
				    TEXT(" + Len(%lld), So call CfaFlvFinishPrs!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prCfaFlv->u8Endoffst,
				    rCfaFlvTxVidInfo.u8FileOfst, rCfaFlvTxVidInfo.u8Len);
			CfaFlvFinishPrs(pvSptHdl, prCfaFlv, GAU_E_EOS);
			MM_RETURN(RET_DMX_OK);
		}
#if CFA_FLV_DBG_VID_CMD_Q_FLOW
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_PTS_V,
			    TEXT("[CFA_FLV] %s line %d -- DmaVid (VCmdCnt: 0, FileOfst: %lld,")
			    TEXT(" Len: %lld), Ca: %lld, Pts: %lld ms\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, rCfaFlvTxVidInfo.u8FileOfst,
			    rCfaFlvTxVidInfo.u8Len, prCfaFlv->u8Ca,
			    (u64) (prCfaFlv->rVTimeStampMan.u4TimeStamp));
#endif				/*  CFA_FLV_DBG_VID_CMD_Q_FLOW */

		if (prCfaFlv->rRange.u8IframeNum >= 1) {
			/* transfer video data to video fifo. */
			prCfaFlv->eCurTxStrmType = CFA_FLV_TX_STRM_TYPE_VID;

			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d Spt4CfaPbb2VFifoAUCtrl!!\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
			mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rCfaFlvTxVidInfo);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] failed in ")
					    TEXT("Spt4CfaPbb2VFifoAUCtrl(), mrRet: 0x%x\r\n"),
					    mrRet);
				CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, GAU_E_FAIL);
				MM_RETURN(mrRet);
			}
		} else {
			prCfaFlv->fgTagIsValid = FALSE;
			CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
					   0, FLV_READ_FOR_CODEC_SIZE);
		}
#endif				/* ENABLE_DMX_ADVANCED_VER */
	} else {
		u32 u4TxLocalOffset = 1;	/* Skip FrameType[4bit] and CodecID(4bit) */

		/* Skip FrameType[4bit] and CodecID(4bit), HorizontalAdjustment[4bit] and VerticalAdjustment[4bit] */
		if (CFA_VID_VP6 == prCfaFlv->eVidCodecType)
			u4TxLocalOffset = 2;
		/* Skip FrameType[4bit] and CodecID(4bit), HorizontalAdjustment[4bit] and VerticalAdjustment[4bit] */
		if (CFA_VID_VP6A == prCfaFlv->eVidCodecType)
			u4TxLocalOffset = 2;

		u8Sa = prCfaFlv->rVideoInfo.rVTagInfo.u8Offset + FLV_TAG_HDR_SIZE + u4TxLocalOffset;

		CfaFlvCalcVidPts(pvSptHdl, prCfaFlv);

		prCfaFlv->rVTimeStampMan.u8TotalTagCnt++;

		u8Pts = MS_TO_PTS(prCfaFlv->rVTimeStampMan.u4TimeStamp);

		if (prCfaFlv->rVideoInfo.rVTagInfo.u4DataSize > 0)
			u4TxLen = prCfaFlv->rVideoInfo.rVTagInfo.u4DataSize - u4TxLocalOffset;
		else
			u4TxLen = 0;

		if (prCfaFlv->fgFirstTxVid)
			prCfaFlv->fgFirstTxVid = FALSE;

		if (0 == u4TxLen) {
			prCfaFlv->fgTagIsValid = FALSE;
			CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
					   0, FLV_READ_FOR_CODEC_SIZE);
			MM_RETURN(RET_MSDKC_OK);
		}
		/* transfer video data to video fifo. */
		prCfaFlv->eCurTxStrmType = CFA_FLV_TX_STRM_TYPE_VID;

		mrRet = CfaFlvTxVidDataByCmdQ(pvSptHdl, prCfaFlv, u8Sa, u4TxLen, u8Pts, FALSE);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] failed in CfaFlvTxVidDataByCmdQ, mrRet: 0x%x\r\n"),
				    mrRet);
			CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, GAU_E_FAIL);
			MM_RETURN(mrRet);
		}
	}

	MM_RETURN(mrRet);
}
static MRESULT CfaFlvTxVidH265Seq2Fifo(void *pvSptHdl, CfaFlvInst_T *prCfaFlv)
{
	MRESULT mrRet = RET_DMX_OK;
	if ((NULL == prCfaFlv->puDecoderCfgBuf) ||
		(0 == prCfaFlv->u4DecoderCfgBufLen)) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					TEXT("[CFA_FLV]  NULL == prCfaFlv->rVideoInfo.rCfgInfo.puSeqHdr\r\n"));
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA_FLV] %s -- DecoderCfgBufLen=%d\r\n"),
			DMX_FUNC_NAME, prCfaFlv->u4DecoderCfgBufLen);
	mrRet = Spt4CfaBuf2VFifo(pvSptHdl, prCfaFlv->puDecoderCfgBuf, (u64)0,
				 CFA_PTM_EXACT_POS, prCfaFlv->eVidCodecType,
				 (u64)(prCfaFlv->u4DecoderCfgBufLen));
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("[CFA_FLV] failed in ")
				TEXT("Spt4CfaBuf2VFifo, mrRet: 0x%x, call FinishPrs\r\n"),
				mrRet);
		CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, GAU_E_FAIL);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

static MRESULT CfaFlvTxVidSeq2Fifo(void *pvSptHdl, CfaFlvInst_T *prCfaFlv)
{
	MRESULT mrRet = RET_DMX_OK;

	if (CFA_FLV_TAG_TYPE_VIDEO != prCfaFlv->rTagInfo.eTagType) {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			    TEXT("[CFA_FLV] fail for invalid args, rTagInfo.eTagType(%d) != Video\r\n"),
			     prCfaFlv->rTagInfo.eTagType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if ((AVCODEC_ID_H264 == prCfaFlv->rVideoInfo.rCfgInfo.eCodecID) ||
	    (VCODEC_VERSION_H265_HM91 == prCfaFlv->rVideoInfo.rCfgInfo.rVersion)) {
		if (CFA_FLV_ANA_ST_TX_PARTTAG_VIDEO != prCfaFlv->eCurAnaSt)
			prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_TX_VID;

		if (prCfaFlv->fgNeedTxVidSeq && prCfaFlv->fgFirstTxVid) {
			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					TEXT("[CFA_FLV] %s -- DecoderCfgBufLen=%d\r\n"),
				    DMX_FUNC_NAME, prCfaFlv->u4DecoderCfgBufLen);
			prCfaFlv->fgNeedTxVidSeq = FALSE;
			mrRet = Spt4CfaBuf2VFifo(pvSptHdl, prCfaFlv->puDecoderCfgBuf, (u64)0,
						 CFA_PTM_EXACT_POS, prCfaFlv->eVidCodecType,
						 (u64)(prCfaFlv->u4DecoderCfgBufLen));
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] failed in ")
					    TEXT("Spt4CfaBuf2VFifo, mrRet: 0x%x, call FinishPrs\r\n"),
					    mrRet);
				CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, GAU_E_FAIL);
				MM_RETURN(mrRet);
			}
		} else {
			mrRet = Spt4CfaBuf2VFifo(pvSptHdl, prCfaFlv->puDecoderCfgBuf,
						 0, CFA_PTM_EXACT_POS, prCfaFlv->eVidCodecType, 3);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] failed in ")
					    TEXT("Spt4CfaBuf2VFifo, mrRet: 0x%x, call FinishPrs\r\n"),
					    mrRet);
				CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, (u32)GAU_E_FAIL);
				MM_RETURN(mrRet);
			}
		}
	} else {
		mrRet = CfaFlvTxVidData2Fifo(pvSptHdl, prCfaFlv);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] failed in ")
				    TEXT("CfaFlvTxVidData2Fifo, mrRet: 0x%x, call FinishPrs\r\n"),
				    mrRet);
			MM_RETURN(mrRet);
		}
	}

	MM_RETURN(RET_DMX_OK);
}


static void CfaFlvPrsDmux(void *pvSptHdl, CfaFlvInst_T *prCfaFlv)
{
	MRESULT mrRet = RET_DMX_OK;

	if (CFA_FLV_TAG_TYPE_AUDIO == prCfaFlv->rTagInfo.eTagType) {
		if (fgIsCfaStmToPlay(prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_AUD)) {
			CfaFlvCalcAudPts(pvSptHdl, prCfaFlv);
			prCfaFlv->rATimeStampMan.u8TotalTagCnt++;
		}

		if ((!fgIsCfaStmToPlay(prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_AUD)) ||
		    ((CFA_FLV_SKIP_BY_PTS == prCfaFlv->rRange.eSkipMode) &&
		     (prCfaFlv->rATimeStampMan.u4TimeStamp < prCfaFlv->rRange.u8DispPicPTS))) {
			prCfaFlv->fgTagIsValid = FALSE;
			if (fgIsCfaStmToPlay(prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_AUD)) {
				DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_PTS_A,
					TEXT("[CFA_FLV]%s line %d, Audio Data TimeStamp:%dms < SeekTime:%lldms\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prCfaFlv->rATimeStampMan.u4TimeStamp,
				prCfaFlv->rRange.u8DispPicPTS);
			}
			CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
					   (u64)0, (u64)FLV_READ_FOR_CODEC_SIZE);
		} else {
			/*transfer audio data to fifo */
			/* DMA Audio Data */
			if (AVCODEC_ID_AAC == prCfaFlv->rAudioInfo.rCfgInfo.eCodecID) {
#if !ENABLE_DMX_ADVANCED_VER
				if ((!prCfaFlv->rRange.fgHasVid) && DMX_IS_FF_PLAY(pvSptHdl)) {
					bool fgDmaAud = FALSE;
					u32 u4SkipTagsCnt = 0, u4PerTagsCnt = 0;

					switch (SplitterGetPlayRate(pvSptHdl)) {
					case MM_PLAY_RATE_FF_2X:
					case MM_PLAY_RATE_FF_4X:
						u4SkipTagsCnt = 0;
						u4PerTagsCnt = (u32)100;
						break;
					case MM_PLAY_RATE_FF_8X:
						u4SkipTagsCnt = (u32)10;	/* discard: 10% */
						u4PerTagsCnt = (u32)100;
						break;
					case MM_PLAY_RATE_FF_16X:
						u4SkipTagsCnt = (u32)30;	/* discard: 30% */
						u4PerTagsCnt = (u32)100;
						break;
					case MM_PLAY_RATE_FF_32X:
						u4SkipTagsCnt = (u32)65;	/* discard: 65% */
						u4PerTagsCnt = (u32)100;
						break;
					default:
						u4SkipTagsCnt = (u32)1;
						u4PerTagsCnt = (u32)1;
						break;
					}

					if (prCfaFlv->u4PureAudSkipTagCnt >= u4SkipTagsCnt) {
						fgDmaAud = TRUE;
						mrRet = CfaFlvTxAudCfg2Fifo(pvSptHdl, prCfaFlv);
					}

					if (prCfaFlv->u4PureAudSkipTagCnt + 1 <
					    u4PerTagsCnt) {
						prCfaFlv->u4PureAudSkipTagCnt++;
					} else {
						prCfaFlv->u4PureAudSkipTagCnt = 0;
					}

					if (!fgDmaAud) {
						prCfaFlv->fgTagIsValid = FALSE;
						CfaFlvSearchNextSc(pvSptHdl, prCfaFlv,
								   CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
								   0,
								   FLV_READ_FOR_CODEC_SIZE);
					}
				} else if (!prCfaFlv->rRange.fgHasVid) {
					mrRet = CfaFlvTxAudCfg2Fifo(pvSptHdl, prCfaFlv);
				} else {
					mrRet = CfaFlvTxAudCfg2Fifo(pvSptHdl, prCfaFlv);
				}
#else
				mrRet = CfaFlvTxAudCfg2Fifo(pvSptHdl, prCfaFlv);
#endif
			} else {
#if CFA_FLV_DBG_AUD_CMD_Q_FLOW
				DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_PTS_A,
					    TEXT("[CFA_FLV] %s line %d -- CfaFlvTxAudData2Fifo\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO);
#endif				/* CFA_FLV_DBG_AUD_CMD_Q_FLOW */
				mrRet = CfaFlvTxAudData2Fifo(pvSptHdl, prCfaFlv);
			}

			if(mrRet != RET_DMX_OK)
			{
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					TEXT("[CFA_FLV] transfer audio data to fifo failed!,Finish parse!\r\n"));
			}
		}
	} else if (CFA_FLV_TAG_TYPE_VIDEO == prCfaFlv->rTagInfo.eTagType) {
		/*transfer video data to fifo */
		/* If not play video, skip the video tag */
		if (!fgIsCfaStmToPlay(prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_VID)) {
			prCfaFlv->fgTagIsValid = FALSE;
			CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
					   0, FLV_READ_FOR_CODEC_SIZE);

			return;
		}
		/* DMA Video Data */
		if ((AVCODEC_ID_H264 == prCfaFlv->rVideoInfo.rCfgInfo.eCodecID) ||
		    (VCODEC_VERSION_H265_HM91 == prCfaFlv->rVideoInfo.rCfgInfo.rVersion)) {
			mrRet = CfaFlvTxVidSeq2Fifo(pvSptHdl, prCfaFlv);
		} else {
			mrRet = CfaFlvTxVidData2Fifo(pvSptHdl, prCfaFlv);
		}

        if(mrRet != RET_DMX_OK)
        {
        	DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("[CFA_FLV] transfer video data to fifo failed!,Finish parse!\r\n"));
        }

	} else if (CFA_FLV_TAG_TYPE_DATA == prCfaFlv->rTagInfo.eTagType) {	/*Skip */
		prCfaFlv->fgTagIsValid = FALSE;
		CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER, 0,
				   FLV_READ_FOR_CODEC_SIZE);
	} else {
		DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			 TEXT("[CFA_FLV] prCfaFlv->rTagInfo.eTagType(%d) is unknown!\r\n"),
			 prCfaFlv->rTagInfo.eTagType);

	}
}

void CfaFlvGetVPictType(CfaFlvInst_T *prCfaFlv, u32 u4Offset)
{
	u8 u1Temp1 = 0;
	u8 u1Temp2 = 0;
	u32 u4PictSizeOft = 0;

	switch (prCfaFlv->rVideoInfo.rCfgInfo.eCodecID) {
	case AVCODEC_ID_SORENSON:
		{
			/* skip"frm type[4Bits],codecid[4Bits], PictureStateCode[17Bits],*/
			/* Version[5Bits], TemporalReference[8Bits]" */
			u4PictSizeOft = u4Offset + (u32)4;

			/* PictureSize[3Bits] */
			LOAD_BYTE(prCfaFlv->pu1HdrBuf + u4PictSizeOft, u1Temp1);
			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d -- [Byte4]: 0x%02x\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u1Temp1);
			u1Temp1 &= (u8)0x03;
			u1Temp1 = u1Temp1 << 1;

			LOAD_BYTE(prCfaFlv->pu1HdrBuf + u4PictSizeOft + 1, u1Temp2);
			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d -- [Byte5]: 0x%02x\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u1Temp2);
			u1Temp2 &= (u8)0x80;
			u1Temp2 = u1Temp2 >> 7;
			u1Temp1 = u1Temp1 + u1Temp2;

			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d -- PicSize: 0x%02x\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u1Temp1);

			u4PictSizeOft += (u32)1;	/* Skip PicSize */

			/* if PictureSize = 0 --> CustomWidth: 1Byte, CustomHeight: 1Byte */
			/* if PictureSize = 1 --> CustomWidth: 2Byte, CustomHeight: 2Byte */
			/* if PictureSize = otherwise --> CustomWidth & CustomHeight is absent */

			if (u1Temp1 == 0)	/* Custom: 1 Byte */
				u4PictSizeOft += (u32)2;
			else if (u1Temp1 == (u8)1)	/* Custom: 2 Byte */
				u4PictSizeOft += (u32)4;
			else {
				/*do nothing*/
			}
			/* PictureType [2Bits] */
			LOAD_BYTE(prCfaFlv->pu1HdrBuf + u4PictSizeOft, u1Temp2);
			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d -- [PicTypeByte]: 0x%02x\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u1Temp2);
			u1Temp2 &= (u8)0x60;

			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d -- [PicType]: 0x%02x\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u1Temp2);

			if (FLV_FRAME_TYPE_VALUE_I == u1Temp2)
				prCfaFlv->rVideoInfo.eFrmType = FLV_FRM_TYPE_I;
			else if (FLV_FRAME_TYPE_VALUE_P == u1Temp2)
				prCfaFlv->rVideoInfo.eFrmType = FLV_FRM_TYPE_P;
			else if (FLV_FRAME_TYPE_VALUE_B == u1Temp2)
				prCfaFlv->rVideoInfo.eFrmType = FLV_FRM_TYPE_B;
			else
				prCfaFlv->rVideoInfo.eFrmType = FLV_FRM_TYPE_NONE;
		}
		break;
	case AVCODEC_ID_H264:
	case AVCODEC_ID_H265:
		if((VCODEC_VERSION_NONE == prCfaFlv->rVideoInfo.rCfgInfo.rVersion) || (VCODEC_VERSION_H265_HM91 == prCfaFlv->rVideoInfo.rCfgInfo.rVersion))
		{
			u4PictSizeOft = u4Offset;

			LOAD_BYTE(prCfaFlv->pu1HdrBuf + u4PictSizeOft, u1Temp1);
			u1Temp1 &= 0xF0;
			if (0x10 == u1Temp1)
				prCfaFlv->rVideoInfo.eFrmType = FLV_FRM_TYPE_I;
			else if (0x20 == u1Temp1)
				prCfaFlv->rVideoInfo.eFrmType = FLV_FRM_TYPE_P;
			else if (0x30 == u1Temp1)
				prCfaFlv->rVideoInfo.eFrmType = FLV_FRM_TYPE_B;	/* need modify */
			else
				prCfaFlv->rVideoInfo.eFrmType = FLV_FRM_TYPE_NONE;
		}
		else
		{
			u4PictSizeOft = u4Offset;

			LOAD_BYTE(prCfaFlv->pu1HdrBuf + u4PictSizeOft, u1Temp1);
			u1Temp1 &= 0xF0;
			if (0x10 == u1Temp1)
				prCfaFlv->rVideoInfo.eFrmType = FLV_FRM_TYPE_I;
			else
				prCfaFlv->rVideoInfo.eFrmType = FLV_FRM_TYPE_NONE;
		}
		break;
	case AVCODEC_ID_VP6:
		if(VCODEC_VERSION_NONE == prCfaFlv->rVideoInfo.rCfgInfo.rVersion)
		{
			u4PictSizeOft = u4Offset + 2;

			LOAD_BYTE(prCfaFlv->pu1HdrBuf + u4PictSizeOft, u1Temp1);
			u1Temp1 &= 0x80;
			if (0x00 == u1Temp1)
				prCfaFlv->rVideoInfo.eFrmType = FLV_FRM_TYPE_I;
			else
				prCfaFlv->rVideoInfo.eFrmType = FLV_FRM_TYPE_P;

		}
		else
		{
			u4PictSizeOft = u4Offset + 5;

			LOAD_BYTE(prCfaFlv->pu1HdrBuf + u4PictSizeOft, u1Temp1);
			u1Temp1 &= 0x80;
			if (0x00 == u1Temp1)
				prCfaFlv->rVideoInfo.eFrmType = FLV_FRM_TYPE_I;
			else
				prCfaFlv->rVideoInfo.eFrmType = FLV_FRM_TYPE_P;

		}
		break;
	default:
		{
			u4PictSizeOft = u4Offset;

			LOAD_BYTE(prCfaFlv->pu1HdrBuf + u4PictSizeOft, u1Temp1);
			u1Temp1 &= 0xF0;
			if (0x10 == u1Temp1)
				prCfaFlv->rVideoInfo.eFrmType = FLV_FRM_TYPE_I;
			else
				prCfaFlv->rVideoInfo.eFrmType = FLV_FRM_TYPE_NONE;
		}
		break;
	}
}

void CfaFlvGetAudInfo(CfaFlvInst_T *prCfaFlv, u32 u4Offset)
{
	u8 u1Temp1 = 0;
	u8 u1Temp2 = 0;

	LOAD_BYTE(prCfaFlv->pu1HdrBuf + u4Offset, u1Temp1);

	u1Temp2 = u1Temp1 & ((u8)0x0C);
	u1Temp2 = u1Temp2 >> 2;	/* SoundRate */

	switch (u1Temp2) {
	case (u8)0:
		prCfaFlv->rAudioInfo.rCfgInfo.u4SoundRate = FLV_AUD_SAMPLERATE_5K5HZ;
		break;

	case (u8)1:
		prCfaFlv->rAudioInfo.rCfgInfo.u4SoundRate = FLV_AUD_SAMPLERATE_11KHZ;
		break;

	case (u8)2:
		prCfaFlv->rAudioInfo.rCfgInfo.u4SoundRate = FLV_AUD_SAMPLERATE_22KHZ;
		break;

	case (u8)3:
		prCfaFlv->rAudioInfo.rCfgInfo.u4SoundRate = FLV_AUD_SAMPLERATE_44KHZ;
		break;

	default:
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			    TEXT("[CFA_FLV] %s -- Audio SampleRate unknown, u1Temp2: 0x%x\r\n"),
			    DMX_FUNC_NAME, u1Temp2);
		break;
	}

	u1Temp2 = u1Temp1 & ((u8)0x02);

	if ((u8)0x02 == u1Temp2)
		prCfaFlv->rAudioInfo.rCfgInfo.u2SoundSize = (u16)16;
	else
		prCfaFlv->rAudioInfo.rCfgInfo.u2SoundSize = (u16)8;

	u1Temp2 = u1Temp1 & ((u8)0x01);

	if ((u8)0x01 == u1Temp2)
		prCfaFlv->rAudioInfo.rCfgInfo.eSoundType = CFA_AUDIO_TYPE_STEREO;
	else
		prCfaFlv->rAudioInfo.rCfgInfo.eSoundType = CFA_AUDIO_TYPE_MONO;
}

void CfaFlvReleaseCfgBuf(CfaFlvInst_T *prCfaFlv)
{
	u8 u8Loop = 0;

	for (u8Loop = 0; u8Loop < prCfaFlv->rDecoderCfgInfo.u1NumSeqParSet; u8Loop++) {
		if (NULL != prCfaFlv->rDecoderCfgInfo.rSeqParSet[u8Loop].puSeqParSet) {
			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d -- free [%d] puSeqParSet = 0x%x\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u8Loop,
				    prCfaFlv->rDecoderCfgInfo.rSeqParSet[u8Loop].puSeqParSet);
			DMX_FreeMemory(prCfaFlv->rDecoderCfgInfo.rSeqParSet[u8Loop].puSeqParSet);
			prCfaFlv->rDecoderCfgInfo.rSeqParSet[u8Loop].puSeqParSet = NULL;
		}
	}

	for (u8Loop = 0; u8Loop < prCfaFlv->rDecoderCfgInfo.u1NumPicParSet; u8Loop++) {
		if (NULL != prCfaFlv->rDecoderCfgInfo.rPicParSet[u8Loop].puPicParSet) {
			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d -- free [%d] puPicParSet = 0x%x\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u8Loop,
				    prCfaFlv->rDecoderCfgInfo.rPicParSet[u8Loop].puPicParSet);
			DMX_FreeMemory(prCfaFlv->rDecoderCfgInfo.rPicParSet[u8Loop].puPicParSet);
			prCfaFlv->rDecoderCfgInfo.rPicParSet[u8Loop].puPicParSet = NULL;
		}
	}
}

MRESULT CfaFlvGetDecCfgInfo(void *pvSptHdl, CfaFlvInst_T *prCfaFlv, u32 u4Offset)
{
	DecoderCfgInfo_T *prDecCfgInfo = NULL;
	SeqParSet_T *prSetParInfo = NULL;
	PicParSet_T *prPicParInfo = NULL;
	u32 u4DecoderCfgOft = 0;
	u32 u4SkipBytes = 0;
	u8 u1CfgVersion = 0;
	u8 u1NumOfSeq = 0;
	u8 u1NumOfPic = 0;

	u4DecoderCfgOft += u4Offset;
	prDecCfgInfo = &(prCfaFlv->rDecoderCfgInfo);

	LOAD_BYTE(prCfaFlv->pu1HdrBuf + u4DecoderCfgOft, u1CfgVersion);
	LOAD_BYTE(prCfaFlv->pu1HdrBuf + u4DecoderCfgOft + 5, u1NumOfSeq);

	u1NumOfSeq &= (u8)0x1F;
	u4SkipBytes = (u32)6;

	CfaFlvReleaseCfgBuf(prCfaFlv);

	while ((u1NumOfSeq > 0) && (prDecCfgInfo->u1NumSeqParSet < FLV_SEQ_HDR_SET_MAX_CNT)) {
		prSetParInfo = &(prDecCfgInfo->rSeqParSet[prDecCfgInfo->u1NumSeqParSet]);
		LOADB_WORD(prCfaFlv->pu1HdrBuf + u4DecoderCfgOft + u4SkipBytes,
			   prSetParInfo->u2SeqParSetLen);
		DMX_NewMemory(prSetParInfo->u2SeqParSetLen, prSetParInfo->puSeqParSet);
		if (NULL == prSetParInfo->puSeqParSet) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] fail in Alloc puSeqParSet memory\r\n"));
			MM_RETURN(RET_DMX_NO_MEM);
		}

		DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			    TEXT("[CFA_FLV] %s -- alloc puSeqParSet=0x%x, u1NumOfSeq=%d\r\n"),
			    DMX_FUNC_NAME, prSetParInfo->puSeqParSet, u1NumOfSeq);
		dmx_memset(prSetParInfo->puSeqParSet, 0, prSetParInfo->u2SeqParSetLen);
		u4SkipBytes += (u32)2;
		dmx_memcpy(prSetParInfo->puSeqParSet,
			   prCfaFlv->pu1HdrBuf + u4DecoderCfgOft + u4SkipBytes,
			   prSetParInfo->u2SeqParSetLen);
		u4SkipBytes += prSetParInfo->u2SeqParSetLen;
		prDecCfgInfo->u1NumSeqParSet++;
		u1NumOfSeq--;
	}

	LOAD_BYTE(prCfaFlv->pu1HdrBuf + u4DecoderCfgOft + u4SkipBytes, u1NumOfPic);
	u4SkipBytes += (u32)1;

	while ((u1NumOfPic > 0) && (prDecCfgInfo->u1NumPicParSet < FLV_PIC_HDR_SET_MAX_CNT)) {
		prPicParInfo = &(prDecCfgInfo->rPicParSet[prDecCfgInfo->u1NumPicParSet]);
		LOADB_WORD(prCfaFlv->pu1HdrBuf + u4DecoderCfgOft + u4SkipBytes,
			   prPicParInfo->u2PicParSetLen);
		DMX_NewMemory(prPicParInfo->u2PicParSetLen, prPicParInfo->puPicParSet);
		if (NULL == prPicParInfo->puPicParSet) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] fail in Alloc puPicParSet memory\r\n"));
			MM_RETURN(RET_DMX_NO_MEM);
		}
		DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			    TEXT("[CFA_FLV] %s -- alloc puPicParSet=0x%x, u1NumOfPic=%d\r\n"),
			    DMX_FUNC_NAME, prPicParInfo->puPicParSet, u1NumOfPic);
		dmx_memset(prPicParInfo->puPicParSet, 0, prPicParInfo->u2PicParSetLen);
		u4SkipBytes += 2;
		dmx_memcpy(prPicParInfo->puPicParSet, prCfaFlv->pu1HdrBuf +
			   u4DecoderCfgOft + u4SkipBytes, prPicParInfo->u2PicParSetLen);
		u4SkipBytes += prPicParInfo->u2PicParSetLen;
		prDecCfgInfo->u1NumPicParSet++;
		u1NumOfPic--;
	}

	if ((prDecCfgInfo->u1NumSeqParSet > 0) && (prDecCfgInfo->u1NumPicParSet > 0)) {
		prSetParInfo = &(prDecCfgInfo->rSeqParSet[0]);
		prPicParInfo = &(prDecCfgInfo->rPicParSet[0]);

		if (prCfaFlv->puDecoderCfgBuf != NULL) {
			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s -- free puDecoderCfgBuf=0x%x\r\n"),
				    DMX_FUNC_NAME, prCfaFlv->puDecoderCfgBuf);
			DMX_FreeHwMemory(prCfaFlv->puDecoderCfgBuf);
			prCfaFlv->puDecoderCfgBuf = NULL;
		}

		prCfaFlv->u4DecoderCfgBufLen =
		    prSetParInfo->u2SeqParSetLen + prPicParInfo->u2PicParSetLen;

		DMX_NewHwMemory(prCfaFlv->u4DecoderCfgBufLen + 10, prCfaFlv->puDecoderCfgBuf);
		if (NULL == prCfaFlv->puDecoderCfgBuf) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] fail in Alloc puDecoderCfgBuf memory\r\n"));
			MM_RETURN(RET_DMX_NO_MEM);
		}
		DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				TEXT("[CFA_FLV] %s -- alloc puDecoderCfgBuf=0x%x\r\n"),
			    DMX_FUNC_NAME, prCfaFlv->puDecoderCfgBuf);
		dmx_memset(prCfaFlv->puDecoderCfgBuf, 0, (prCfaFlv->u4DecoderCfgBufLen + 10));
		prCfaFlv->puDecoderCfgBuf[0] = 0x00;
		prCfaFlv->puDecoderCfgBuf[1] = 0x00;
		prCfaFlv->puDecoderCfgBuf[2] = 0x01;

		if (prSetParInfo->u2SeqParSetLen <= prCfaFlv->u4DecoderCfgBufLen) {
			dmx_memcpy(prCfaFlv->puDecoderCfgBuf + 3, prSetParInfo->puSeqParSet,
				   prSetParInfo->u2SeqParSetLen);
			prCfaFlv->puDecoderCfgBuf[prSetParInfo->u2SeqParSetLen + 3] = 0x00;
			prCfaFlv->puDecoderCfgBuf[prSetParInfo->u2SeqParSetLen + 4] = 0x00;
			prCfaFlv->puDecoderCfgBuf[prSetParInfo->u2SeqParSetLen + 5] = 0x01;
			dmx_memcpy(prCfaFlv->puDecoderCfgBuf + prSetParInfo->u2SeqParSetLen + 6,
				   prPicParInfo->puPicParSet, prPicParInfo->u2PicParSetLen);
			prCfaFlv->puDecoderCfgBuf[prCfaFlv->u4DecoderCfgBufLen + 6] = 0x00;
			prCfaFlv->puDecoderCfgBuf[prCfaFlv->u4DecoderCfgBufLen + 7] = 0x00;
			prCfaFlv->puDecoderCfgBuf[prCfaFlv->u4DecoderCfgBufLen + 8] = 0x01;
			prCfaFlv->u4DecoderCfgBufLen += 9;
		}
	}

	MM_RETURN(RET_DMX_OK);
}

static AVCODECID_T eCfaFlvGetFlvVidCodec(u8 u1CodecID)
{
	switch (u1CodecID) {
	case (u8)1:
		return AVCODEC_ID_MJPEG;

	case (u8)2:
		return AVCODEC_ID_SORENSON;

	case (u8)3:
		return AVCODEC_ID_SCRV;

	case (u8)4:
		return AVCODEC_ID_VP6;

	case (u8)5:
		return AVCODEC_ID_VP6;

	case (u8)6:
		return AVCODEC_ID_SCRV;

	case (u8)7:
		return AVCODEC_ID_H264;

	case (u8)9:
		return AVCODEC_ID_H265;

	case (u8)12:
		return AVCODEC_ID_H265;

	default:
		return AVCODEC_ID_UNKNOWN;
	}
}

static CfaApiVidType CfaFlvGetVidCodec(u8 u1CodecID)
{
	switch (u1CodecID) {
	case (u8)1:		/* CFA_FLV_VID_CODEC_JPEG */
		return CFA_VID_UNKNOWN;

	case (u8)2:		/* CFA_FLV_VID_CODEC_SH263 */
		return CFA_VID_H263_SORENSON;

	case (u8)3:		/* CFA_FLV_VID_CODEC_SCRV */
		return CFA_VID_UNKNOWN;

	case (u8)4:		/* CFA_FLV_VID_CODEC_VP6 */
		return CFA_VID_VP6;

	case (u8)5:		/* CFA_FLV_VID_CODEC_VP6A */
		return CFA_VID_VP6A;

	case (u8)6:		/* CFA_FLV_VID_CODEC_SCRV2 */
		return CFA_VID_UNKNOWN;

	case (u8)7:		/* CFA_FLV_VID_CODEC_AVC */
		return CFA_VID_H264;

	case (u8)9:		/* CFA_FLV_VID_CODEC_H265 */
		return CFA_VID_H265;

	case (u8)12:		/* CFA_FLV_VID_CODEC_H265_HM91 */
		return CFA_VID_H265;

	default:
		return CFA_VID_UNKNOWN;
	}
}

AVCODECID_T CfaFlvGetAudCodec(u8 u1CodecID)
{
	switch (u1CodecID) {
	case (u8)0:
	case (u8)1:
	case (u8)3:
		return AVCODEC_ID_PCM;
	case (u8)2:
	case (u8)14:
		return AVCODEC_ID_MP3;	/* mp layer 2.5 ? */	
	case (u8)4:
	case (u8)5:
	case (u8)6:
		return AVCODEC_ID_NELLYMOSER;
	case (u8)10:
		return AVCODEC_ID_AAC;
	case (u8)11:
		return AVCODEC_ID_SPEEX;
	default:
		return AVCODEC_ID_UNKNOWN;
	}
}

/* FLV CFA processes CFA_FLV_ANA_ST_IDLE */
/* @return None */
/* < [IN] handle of fdmx */
/* < [IN] point to CfaFlvInst */
/* < [IN] Actual transferred data length.  Normally this value should be equal to the u8TxLen in */
/*         the previous transfer issue, unless file end is hit. */
static void CfaFlvSearchTagHeader(void *pvSptHdl, CfaFlvInst_T *prCfaFlv, u64 u8TxLen)
{
	MRESULT mrRet = RET_DMX_OK;
	u32 u4ParsedLen = 0;
	u64 u8PreCa = 0;
	/* Tag header */
	bool fgFind = FALSE;
	u8 bTagType = 0;
	u8 bExTimeStamp = 0;
	u8 bAvcPacketType = 0;
	u32 u4TxLen = 0;

#if (CFA_FLV_DBG_VID_CMD_Q_FLOW || CFA_FLV_DBG_AUD_CMD_Q_FLOW)
	DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		    TEXT("[CFA_FLV] %s line %d -- Ca: %lld, u4AvailDataSz: %d, PreCa: %lld\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO, prCfaFlv->u8Ca, prCfaFlv->u4AvailDataSz,
		    prCfaFlv->u8PreCa);
#endif				/*  (CFA_FLV_DBG_VID_CMD_Q_FLOW || CFA_FLV_DBG_AUD_CMD_Q_FLOW) */

	u4TxLen = (u32) prCfaFlv->u4AvailDataSz;

	while (u4TxLen >= (u4ParsedLen + (u32)FLV_READ_FOR_CODEC_SIZE)) {
		/* tag type, 1 byte */
		LOAD_BYTE(prCfaFlv->pu1HdrBuf + u4ParsedLen, bTagType);
		/* DataSize, 3 byte */
		LOADB_3BYTES2DWORD(prCfaFlv->pu1HdrBuf + u4ParsedLen + 1,
				   prCfaFlv->rTagInfo.u4DataSize);
		/* Timestamp, 3 byte */
		LOADB_3BYTES2DWORD(prCfaFlv->pu1HdrBuf + u4ParsedLen + 4,
				   prCfaFlv->rTagInfo.u4TimeStamp);
		/* ExTimestamp, 1 byte */
		LOAD_BYTE(prCfaFlv->pu1HdrBuf + u4ParsedLen + 7, bExTimeStamp);
		/* StreamID, 3 byte */
		LOADB_3BYTES2DWORD(prCfaFlv->pu1HdrBuf + u4ParsedLen + 8,
				   prCfaFlv->rTagInfo.u4StreamID);

		prCfaFlv->rTagInfo.u8Offset = prCfaFlv->u8Ca + u4ParsedLen;
		prCfaFlv->rTagInfo.u4DataSize = FLV_GET_DATA_SIZE(prCfaFlv->rTagInfo.u4DataSize);
		prCfaFlv->rTagInfo.u4TimeStamp =
		    FLV_GET_TIMESTAMP(prCfaFlv->rTagInfo.u4TimeStamp, bExTimeStamp);
		prCfaFlv->rTagInfo.u4StreamID = FLV_GET_STM_UID(prCfaFlv->rTagInfo.u4StreamID);

		if (FLV_TAG_TYPE_VALUE_AUD == bTagType) {
			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d -- Audio Tag, u4StreamID: %d\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prCfaFlv->rTagInfo.u4StreamID);
			prCfaFlv->rTagInfo.eTagType = CFA_FLV_TAG_TYPE_AUDIO;
			dmx_memcpy(&(prCfaFlv->rAudioInfo.rATagInfo), &(prCfaFlv->rTagInfo),
				   sizeof(CfaFlvTagInfo_T));
		} else if (FLV_TAG_TYPE_VALUE_VID == bTagType) {
			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d -- Video Tag, u4StreamID: %d\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prCfaFlv->rTagInfo.u4StreamID);
			prCfaFlv->rTagInfo.eTagType = CFA_FLV_TAG_TYPE_VIDEO;
			dmx_memcpy(&(prCfaFlv->rVideoInfo.rVTagInfo), &(prCfaFlv->rTagInfo),
				   sizeof(CfaFlvTagInfo_T));
		} else if (FLV_TAG_TYPE_VALUE_SCRIPT == bTagType) {
			prCfaFlv->rTagInfo.eTagType = CFA_FLV_TAG_TYPE_DATA;
		} else {
			prCfaFlv->rTagInfo.eTagType = CFA_FLV_TAG_TYPE_UNKNOWN;
		}

		if ((CFA_FLV_TAG_TYPE_UNKNOWN == prCfaFlv->rTagInfo.eTagType) ||
		    (0 != prCfaFlv->rTagInfo.u4StreamID)) {
			prCfaFlv->fgTagIsValid = FALSE;
			u4ParsedLen++;
		} else if (prCfaFlv->rTagInfo.u4DataSize > FLV_TAG_DATA_MAX_SIZE) {
			/*0x000FFFFF need modify */
			prCfaFlv->fgTagIsValid = FALSE;
			u4ParsedLen++;
		} else if (CFA_FLV_TAG_TYPE_VIDEO == prCfaFlv->rTagInfo.eTagType) {
			u8 bVidCodecType = 0;

			fgFind = TRUE;
			u8PreCa = prCfaFlv->u8Ca + (u64)FLV_TAG_HDR_SIZE + u4ParsedLen;
			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d -- prCfaFlv->rTagInfo.eTagType=%d\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prCfaFlv->rTagInfo.eTagType);

			if (!fgIsCfaStmToPlay
			    (prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_VID)) {
				u4ParsedLen +=
				    prCfaFlv->rTagInfo.u4DataSize + FLV_TAG_HDR_SIZE +
				    FLV_TAG_SIZE_BYTES;
				fgFind = FALSE;
				continue;
			}

			if (prCfaFlv->rTagInfo.u4DataSize >=
			    Spt4CfaGetStreamFifoSize(pvSptHdl, SPT_DATA_V)
			    || prCfaFlv->rTagInfo.u4TimeStamp >= MAX_VALID_TIMESTAMP) {
				CfaFlvFinishPrs(pvSptHdl, prCfaFlv, GAU_E_ERRCHUNK);
				return;
			}

			LOAD_BYTE(prCfaFlv->pu1HdrBuf + u4ParsedLen + FLV_TAG_HDR_SIZE,
				  bVidCodecType);
			bVidCodecType = FLV_GET_VID_CODEC_ID(bVidCodecType);
			if (CFA_VID_UNKNOWN == prCfaFlv->eVidCodecType) {
				prCfaFlv->eVidCodecType = CfaFlvGetVidCodec(bVidCodecType);
			} else {
				AVCODECID_T eVidCodecType =
				    eCfaFlvGetFlvVidCodec(bVidCodecType);
				if (eVidCodecType != prCfaFlv->rVideoInfo.rCfgInfo.eCodecID) {
					fgFind = FALSE;
					prCfaFlv->fgTagIsValid = FALSE;
					u4ParsedLen++;
					continue;
				}
			}

			if (((AVCODEC_ID_H264 ==
			     prCfaFlv->rVideoInfo.rCfgInfo.eCodecID)
			    || (VCODEC_VERSION_H265_HM91 ==
				prCfaFlv->rVideoInfo.rCfgInfo.rVersion))
				&& (!prCfaFlv->fgFirstTxVid)) {
				LOAD_BYTE(prCfaFlv->pu1HdrBuf + u4ParsedLen +
					  FLV_TAG_HDR_SIZE + 1, bAvcPacketType);
				if (FLV_TAG_PKG_TYPE_FRAME == bAvcPacketType) {
					CfaFlvGetVPictType(prCfaFlv,
							   (u4ParsedLen +
							    FLV_TAG_HDR_SIZE));
					if (FLV_FRM_TYPE_I ==
					    prCfaFlv->rVideoInfo.eFrmType) {
						prCfaFlv->rRange.u8IframeNum++;
					}

					prCfaFlv->u4FirstUintPay =
					    CfaFlvGetSliceSize(prCfaFlv,
							       prCfaFlv->pu1HdrBuf +
							       u4ParsedLen +
							       FLV_TAG_HDR_SIZE +
							       FLV_VID_AVC_DATA_HDR);
					LOAD_BYTE(prCfaFlv->pu1HdrBuf +
						  u4ParsedLen + FLV_TAG_HDR_SIZE +
						  FLV_VID_AVC_DATA_HDR +
						  prCfaFlv->u4PayloadLenFieldSz,
						  prCfaFlv->u1FirstBytePay);
				}
			} else if (((AVCODEC_ID_H264 ==
			     prCfaFlv->rVideoInfo.rCfgInfo.eCodecID)
			    || (VCODEC_VERSION_H265_HM91 ==
				prCfaFlv->rVideoInfo.rCfgInfo.rVersion))
				&& (NULL == prCfaFlv->rVideoInfo.rCfgInfo.puSeqHdr)) {
				LOAD_BYTE(prCfaFlv->pu1HdrBuf + u4ParsedLen +
					  FLV_TAG_HDR_SIZE + 1, bAvcPacketType);

				if (FLV_TAG_PKG_TYPE_HDR == bAvcPacketType) {
					mrRet =
					    CfaFlvGetDecCfgInfo(pvSptHdl,
								prCfaFlv,
								u4ParsedLen
								+
								FLV_TAG_HDR_SIZE
								+
								FLV_VID_AVC_DATA_HDR);
					if (DMX_FAILED(mrRet)) {
						CfaFlvNotiDmxChunkError
						    (pvSptHdl, prCfaFlv,
						     GAU_E_FAIL);
						return;
					}
					/* u4ParsedLen++; */
					u4ParsedLen +=
					    prCfaFlv->rTagInfo.u4DataSize +
					    FLV_TAG_HDR_SIZE +
					    FLV_TAG_SIZE_BYTES;
					fgFind = FALSE;
					prCfaFlv->fgNeedTxVidSeq = TRUE;
					continue;
				} else if (FLV_TAG_PKG_TYPE_FRAME !=
					   bAvcPacketType) {
					prCfaFlv->fgTagIsValid = FALSE;
					u4ParsedLen++;
					fgFind = FALSE;
					continue;
				} else {
					CfaFlvGetVPictType(prCfaFlv,
							   (u4ParsedLen +
							    FLV_TAG_HDR_SIZE));
					if (FLV_FRM_TYPE_I ==
					    prCfaFlv->rVideoInfo.eFrmType) {
						prCfaFlv->rRange.
						    u8IframeNum++;
					}
				}
			} else if (((AVCODEC_ID_H264 ==
			     prCfaFlv->rVideoInfo.rCfgInfo.eCodecID)
			    || (VCODEC_VERSION_H265_HM91 ==
				prCfaFlv->rVideoInfo.rCfgInfo.rVersion))
				&& (NULL != prCfaFlv->rVideoInfo.rCfgInfo.puSeqHdr)) {
				LOAD_BYTE(prCfaFlv->pu1HdrBuf + u4ParsedLen +
					  FLV_TAG_HDR_SIZE + 1, bAvcPacketType);

				if (FLV_TAG_PKG_TYPE_FRAME ==
				    bAvcPacketType) {
					CfaFlvGetVPictType(prCfaFlv,
							   (u4ParsedLen +
							    FLV_TAG_HDR_SIZE));
					if (FLV_FRM_TYPE_I ==
					    prCfaFlv->rVideoInfo.eFrmType) {
						prCfaFlv->rRange.
						    u8IframeNum++;
					}
					prCfaFlv->u4FirstUintPay =
					    CfaFlvGetSliceSize(prCfaFlv,
							       prCfaFlv->
							       pu1HdrBuf +
							       u4ParsedLen +
							       FLV_TAG_HDR_SIZE
							       +
							       FLV_VID_AVC_DATA_HDR);
					LOAD_BYTE(prCfaFlv->pu1HdrBuf +
						  u4ParsedLen +
						  FLV_TAG_HDR_SIZE +
						  FLV_VID_AVC_DATA_HDR +
						  prCfaFlv->
						  u4PayloadLenFieldSz,
						  prCfaFlv->u1FirstBytePay);

				}

				prCfaFlv->fgNeedTxVidSeq = TRUE;

				if (FLV_TAG_PKG_TYPE_HDR == bAvcPacketType) {
					prCfaFlv->fgTagIsValid = FALSE;
					/* u4ParsedLen++; */
					u4ParsedLen +=
					    prCfaFlv->rTagInfo.u4DataSize +
					    FLV_TAG_HDR_SIZE +
					    FLV_TAG_SIZE_BYTES;
					fgFind = FALSE;
					continue;
				} else if (FLV_TAG_PKG_TYPE_FRAME !=
					   bAvcPacketType) {
					prCfaFlv->fgTagIsValid = FALSE;
					u4ParsedLen++;
					fgFind = FALSE;
					continue;
				} else {
					/*do nothing*/
				}
			}
			/* get Frame type */
			else if (AVCODEC_ID_SORENSON==
				 prCfaFlv->rVideoInfo.rCfgInfo.eCodecID) {
				CfaFlvGetVPictType(prCfaFlv,
						   (u4ParsedLen + FLV_TAG_HDR_SIZE));
				if (FLV_FRM_TYPE_I == prCfaFlv->rVideoInfo.eFrmType)
					prCfaFlv->rRange.u8IframeNum++;
			}
			else if ((AVCODEC_ID_VP6 == prCfaFlv->rVideoInfo.rCfgInfo.eCodecID) &&
				(VCODEC_VERSION_NONE == prCfaFlv->rVideoInfo.rCfgInfo.rVersion)){
				/* Skip FrameType[4bit] and CodecID(4bit), */
				/* and HorizontalAdjustment[4bit] and VerticalAdjustment[4bit] */
				CfaFlvGetVPictType(prCfaFlv,
						   (u4ParsedLen + FLV_TAG_HDR_SIZE));
				if (FLV_FRM_TYPE_I == prCfaFlv->rVideoInfo.eFrmType) {
					prCfaFlv->rRange.u8IframeNum++;
					DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						    TEXT("[CFA_FLV] I-Frame -- HdrBuf: 0x%02x, ")
						    TEXT("0x%02x, 0x%02x, FrameType: 0x%02x\r\n"),
						    prCfaFlv->pu1HdrBuf[u4ParsedLen +
									FLV_TAG_HDR_SIZE],
						    prCfaFlv->pu1HdrBuf[u4ParsedLen +
									FLV_TAG_HDR_SIZE +
									1],
						    prCfaFlv->pu1HdrBuf[u4ParsedLen +
									FLV_TAG_HDR_SIZE +
									2],
						    prCfaFlv->pu1HdrBuf[u4ParsedLen +
									FLV_TAG_HDR_SIZE]);
				} else {
					DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						    TEXT("[CFA_FLV] P-Frame -- HdrBuf: 0x%02x, ")
						    TEXT("0x%02x, 0x%02x, FrameType: 0x%02x\r\n"),
						    prCfaFlv->pu1HdrBuf[u4ParsedLen +
									FLV_TAG_HDR_SIZE],
						    prCfaFlv->pu1HdrBuf[u4ParsedLen +
									FLV_TAG_HDR_SIZE +
									1],
						    prCfaFlv->pu1HdrBuf[u4ParsedLen +
									FLV_TAG_HDR_SIZE +
									2],
						    prCfaFlv->pu1HdrBuf[u4ParsedLen +
									FLV_TAG_HDR_SIZE]);
				}
			} else if (VCODEC_VERSION_VP6_WITH_ALPHA==
				   prCfaFlv->rVideoInfo.rCfgInfo.rVersion) {
				/* Skip FrameType[4bit] and CodecID(4bit), */
				/* and HorizontalAdjustment[4bit] and VerticalAdjustment[4bit] */
				/* Skip OffsetToAlpha(3B)(combined alpha offset) */
				CfaFlvGetVPictType(prCfaFlv,
						   (u4ParsedLen + FLV_TAG_HDR_SIZE));
				if (FLV_FRM_TYPE_I == prCfaFlv->rVideoInfo.eFrmType) {
					prCfaFlv->rRange.u8IframeNum++;
					DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						    TEXT("[CFA_FLV] I-Frame -- HdrBuf: 0x%02x, 0x%02x,")
						    TEXT(" 0x%02x, FrameType: 0x%02x\r\n"),
						    prCfaFlv->pu1HdrBuf[u4ParsedLen +
									FLV_TAG_HDR_SIZE],
						    prCfaFlv->pu1HdrBuf[u4ParsedLen +
									FLV_TAG_HDR_SIZE +
									1],
						    prCfaFlv->pu1HdrBuf[u4ParsedLen +
									FLV_TAG_HDR_SIZE +
									2],
						    prCfaFlv->pu1HdrBuf[u4ParsedLen +
									FLV_TAG_HDR_SIZE +
									5]);
				} else {
					DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						    TEXT("[CFA_FLV] P-Frame -- HdrBuf: 0x%02x, 0x%02x, ")
						    TEXT("0x%02x, FrameType: 0x%02x\r\n"),
						    prCfaFlv->pu1HdrBuf[u4ParsedLen +
									FLV_TAG_HDR_SIZE],
						    prCfaFlv->pu1HdrBuf[u4ParsedLen +
									FLV_TAG_HDR_SIZE +
									1],
						    prCfaFlv->pu1HdrBuf[u4ParsedLen +
									FLV_TAG_HDR_SIZE +
									2],
						    prCfaFlv->pu1HdrBuf[u4ParsedLen +
									FLV_TAG_HDR_SIZE +
									5]);
				}
			} else if ((AVCODEC_ID_H265 == prCfaFlv->rVideoInfo.rCfgInfo.eCodecID) && 
				(VCODEC_VERSION_NONE == prCfaFlv->rVideoInfo.rCfgInfo.rVersion)) {
				CfaFlvGetVPictType(prCfaFlv,
						   (u4ParsedLen + FLV_TAG_HDR_SIZE));
				if (FLV_FRM_TYPE_I == prCfaFlv->rVideoInfo.eFrmType)
					prCfaFlv->rRange.u8IframeNum++;
			} else {
				/*do nothing*/
			}

			if (u4TxLen >=
			    u4ParsedLen + (prCfaFlv->rTagInfo.u4DataSize + FLV_TAG_HDR_SIZE) +
			    FLV_TAG_SIZE_BYTES) {
				LOADB_DWRD(prCfaFlv->pu1HdrBuf + prCfaFlv->rTagInfo.u4DataSize +
					   FLV_TAG_HDR_SIZE + u4ParsedLen, prCfaFlv->u4TagSize);
				DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] %s line %d  -- u4DataSize(%d),")
					    TEXT(" u4ParsedLen(%d), u4TagSize(%d)\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO,
					    prCfaFlv->rTagInfo.u4DataSize, u4ParsedLen,
					    prCfaFlv->u4TagSize);
				if (IS_FLV_VALID_PREVTAGSZ
				    (prCfaFlv->u4TagSize, prCfaFlv->rTagInfo.u4DataSize)) {
					prCfaFlv->fgTagIsValid = TRUE;
					/* transfer data */
					if (((AVCODEC_ID_H264 ==
					      prCfaFlv->rVideoInfo.rCfgInfo.eCodecID)
					     || (VCODEC_VERSION_H265_HM91 ==
						 prCfaFlv->rVideoInfo.rCfgInfo.rVersion))
					    && (CFA_FLV_TAG_TYPE_VIDEO ==
						prCfaFlv->rTagInfo.eTagType)
					    && (FLV_TAG_PKG_TYPE_FRAME == bAvcPacketType)
					    &&
					    ((prCfaFlv->u4FirstUintPay + FLV_VID_AVC_DATA_HDR +
					      prCfaFlv->u4PayloadLenFieldSz) <
					     prCfaFlv->rTagInfo.u4DataSize)) {
						/* If the Video Tag is AVC Frame Packet(bAvcPacketType = 1) */
						/*and slice data size + slice data header < tag data size */
						/* the video tag raw data has several slices. */
						/* so we tranfer slice one by one. */
						prCfaFlv->u4VideoPartTagOffset = 0;
						/* u8Ca will be the file offset of the the 8*/
						/*first slice's len bytes addr */
						prCfaFlv->u8Ca += u4ParsedLen + FLV_TAG_HDR_SIZE + FLV_VID_AVC_DATA_HDR;
						prCfaFlv->eCurAnaSt =
						    CFA_FLV_ANA_ST_TX_PARTTAG_V_HDR;
						CfaFlvSearchNextSc(pvSptHdl, prCfaFlv,
								   CFA_FLV_ANA_ST_TX_PARTTAG_V_HDR,
								   0,
								   (prCfaFlv->rTagInfo.u4DataSize -
								    FLV_VID_AVC_DATA_HDR));
					} else {
#if CONFIG_CFA_FLV_FOR_ERR_JUMP
						prCfaFlv->u4OffsetJumpCnt = 0;
#endif
						prCfaFlv->u8Ca +=
						    prCfaFlv->rTagInfo.u4DataSize +
						    FLV_TAG_HDR_SIZE + u4ParsedLen +
						    FLV_TAG_SIZE_BYTES;
						prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;
						CfaFlvPrsDmux(pvSptHdl, prCfaFlv);
					}
				} else {
					prCfaFlv->fgTagIsValid = FALSE;
					prCfaFlv->u8Ca = u8PreCa;
					CfaFlvSearchNextSc(pvSptHdl, prCfaFlv,
							   CFA_FLV_ANA_ST_SEARCH_TAG_HEADER, 0,
							   CFA_FLV_READ_LEN);
				}
				return;
			}

			break;
		} else if (CFA_FLV_TAG_TYPE_AUDIO == prCfaFlv->rTagInfo.eTagType) {
			u8 bAudCodecType = 0;
			AVCODECID_T eAudCodecType = AVCODEC_ID_UNKNOWN;

			fgFind = TRUE;
			u8PreCa = prCfaFlv->u8Ca + FLV_TAG_HDR_SIZE + u4ParsedLen;
			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d -- prCfaFlv->rTagInfo.eTagType=%d\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prCfaFlv->rTagInfo.eTagType);

			if (!fgIsCfaStmToPlay
			    (prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_AUD)) {
				DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] %s line %d -- Audio Tag Not to play\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO);
				u4ParsedLen +=
				    prCfaFlv->rTagInfo.u4DataSize + FLV_TAG_HDR_SIZE +
				    FLV_TAG_SIZE_BYTES;
				fgFind = FALSE;
				continue;
			}

			if (prCfaFlv->rTagInfo.u4DataSize >=
			    Spt4CfaGetStreamFifoSize(pvSptHdl, SPT_DATA_A)
			    || prCfaFlv->rTagInfo.u4TimeStamp >= MAX_VALID_TIMESTAMP) {
				CfaFlvFinishPrs(pvSptHdl, prCfaFlv, GAU_E_ERRCHUNK);
				return;
			}

			LOAD_BYTE(prCfaFlv->pu1HdrBuf + u4ParsedLen + FLV_TAG_HDR_SIZE,
				  bAudCodecType);

			bAudCodecType = FLV_GET_AUD_CODEC_ID(bAudCodecType);
			eAudCodecType = CfaFlvGetAudCodec(bAudCodecType);

			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d -- Audio 0x%02x, 0x%02x, ")
				    TEXT("0x%02x, 0x%02x, AudioCodecID: %d, eAudCodecType: %d\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO,
				    prCfaFlv->pu1HdrBuf[u4ParsedLen + FLV_TAG_HDR_SIZE],
				    prCfaFlv->pu1HdrBuf[u4ParsedLen + FLV_TAG_HDR_SIZE + 1],
				    prCfaFlv->pu1HdrBuf[u4ParsedLen + FLV_TAG_HDR_SIZE + 2],
				    prCfaFlv->pu1HdrBuf[u4ParsedLen + FLV_TAG_HDR_SIZE + 3],
				    prCfaFlv->rAudioInfo.rCfgInfo.eCodecID, eAudCodecType);

			if (AVCODEC_ID_UNKNOWN !=
			    prCfaFlv->rAudioInfo.rCfgInfo.eCodecID) {
				if (eAudCodecType != prCfaFlv->rAudioInfo.rCfgInfo.eCodecID) {
					fgFind = FALSE;
					prCfaFlv->fgTagIsValid = FALSE;
					u4ParsedLen++;
					continue;
				}
			} else {
				prCfaFlv->rAudioInfo.rCfgInfo.eCodecID = eAudCodecType;
			}

			if ((AVCODEC_ID_AAC ==
			     prCfaFlv->rAudioInfo.rCfgInfo.eCodecID)
			    && (prCfaFlv->fgFirstTxAud)) {
				LOAD_BYTE(prCfaFlv->pu1HdrBuf + u4ParsedLen +
					  FLV_TAG_HDR_SIZE + 1, bAvcPacketType);
				DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] %s line %d -- AAC, bAvcPacketType: 0x%02x\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, bAvcPacketType);

				if ((NULL == prCfaFlv->rAudioInfo.rCfgInfo.rAacInfo.puHeader)
					&& (FLV_TAG_PKG_TYPE_HDR == bAvcPacketType)) {
					DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						    TEXT("[CFA_FLV] %s line %d -- AAC PKG_TYPE_HDR, ")
						    TEXT("bAvcPacketType: 0x%02x\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    bAvcPacketType);

					if (NULL != prCfaFlv->rAacCfgInfo.puHeader) {
						DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
							    TEXT("[CFA_FLV] %s line %d -- ")
							    TEXT("free prCfaFlv->")
							    TEXT("rAacCfgInfo.puHeader=0x%x\r\n"),
							    DMX_FUNC_NAME,
							    DMX_LINE_NO,
							    prCfaFlv->rAacCfgInfo.
							    puHeader);
						DMX_FreeHwMemory(prCfaFlv->
								 rAacCfgInfo.
								 puHeader);
						prCfaFlv->rAacCfgInfo.puHeader =
						    NULL;
					}
					/* get config */
					prCfaFlv->rAacCfgInfo.uHeaderLen =
					    (u8) (prCfaFlv->rAudioInfo.rATagInfo.
						     u4DataSize - 2);
					/* prCfaFlv->rAacCfgInfo.puHeader =
					*  (u8 *)DMX_NewHwMemory((size_t)*/
					/*(prCfaFlv->rAacCfgInfo.uHeaderLen + 5)); */
					DMX_NewHwMemory((prCfaFlv->rAacCfgInfo.
							 uHeaderLen + 5),
							prCfaFlv->rAacCfgInfo.
							puHeader);
					/* fix klocwork */
					if (NULL == prCfaFlv->rAacCfgInfo.puHeader) {
						DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
							    TEXT("[CFA_FLV] alloc aac ")
							    TEXT("puHeader Failed, no memory!\r\n"));
						CfaFlvNotiDmxChunkError(pvSptHdl,
									prCfaFlv,
									GAU_E_FAIL);
						break;
					}
					DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						    TEXT("[CFA_FLV] %s line %d -- alloc ")
						    TEXT("aac puHeader: 0x%x\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    prCfaFlv->rAacCfgInfo.puHeader);
					dmx_memset(prCfaFlv->rAacCfgInfo.puHeader,
						   0,
						   (prCfaFlv->rAacCfgInfo.
						    uHeaderLen + 5));
					dmx_memcpy((prCfaFlv->rAacCfgInfo.puHeader +
						    4),
						   (prCfaFlv->pu1HdrBuf +
						    u4ParsedLen + FLV_TAG_HDR_SIZE +
						    2),
						   (prCfaFlv->rAacCfgInfo.
						    uHeaderLen));
					prCfaFlv->rAacCfgInfo.puHeader[3] =
					    prCfaFlv->rAacCfgInfo.uHeaderLen;
					prCfaFlv->rAacCfgInfo.uHeaderLen += 4;
					u4ParsedLen +=
					    prCfaFlv->rTagInfo.u4DataSize +
					    FLV_TAG_HDR_SIZE + FLV_TAG_SIZE_BYTES;
					prCfaFlv->fgTagIsValid = FALSE;
					fgFind = FALSE;
					continue;
				} else if (
					(NULL == prCfaFlv->rAudioInfo.rCfgInfo.rAacInfo.puHeader)
					&& (FLV_TAG_PKG_TYPE_FRAME != bAvcPacketType)) {
					DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						    TEXT("[CFA_FLV] %s line %d -- AAC != FRAME,")
						    TEXT(" bAvcPacketType: 0x%02x\r\n"),
						    DMX_FUNC_NAME, DMX_LINE_NO,
						    bAvcPacketType);

					prCfaFlv->fgTagIsValid = FALSE;
					u4ParsedLen++;
					fgFind = FALSE;
					continue;
				} else if (
					NULL != prCfaFlv->rAudioInfo.rCfgInfo.rAacInfo.puHeader) {
					if (FLV_TAG_PKG_TYPE_HDR == bAvcPacketType) {
						DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
							    TEXT("[CFA_FLV] %s line %d -- AAC == HDR, ")
							    TEXT("bAvcPacketType: 0x%02x\r\n"),
							    DMX_FUNC_NAME, DMX_LINE_NO,
							    bAvcPacketType);

						prCfaFlv->fgTagIsValid = FALSE;
						u4ParsedLen +=
						    prCfaFlv->rTagInfo.u4DataSize +
						    FLV_TAG_HDR_SIZE + FLV_TAG_SIZE_BYTES;
						fgFind = FALSE;
						continue;
					} else if (FLV_TAG_PKG_TYPE_FRAME != bAvcPacketType) {
						DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
							    TEXT("[CFA_FLV] %s line %d -- 2 AAC != FRAME, ")
							    TEXT("bAvcPacketType: 0x%02x\r\n"),
							    DMX_FUNC_NAME, DMX_LINE_NO,
							    bAvcPacketType);

						prCfaFlv->fgTagIsValid = FALSE;
						u4ParsedLen++;
						fgFind = FALSE;
						continue;
					} else {
						/*do nothing*/
					}
				}else {
					/*do nothing*/
				}
			}
			CfaFlvGetAudInfo(prCfaFlv, (u4ParsedLen + FLV_TAG_HDR_SIZE));

			if (u4TxLen >=
				u4ParsedLen + (prCfaFlv->rTagInfo.u4DataSize + FLV_TAG_HDR_SIZE) +
				FLV_TAG_SIZE_BYTES) {
				LOADB_DWRD(prCfaFlv->pu1HdrBuf + prCfaFlv->rTagInfo.u4DataSize +
					   FLV_TAG_HDR_SIZE + u4ParsedLen, prCfaFlv->u4TagSize);
				DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						TEXT("[CFA_FLV] %s line %d  -- u4DataSize(%d),")
						TEXT(" u4ParsedLen(%d), u4TagSize(%d)\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO,
						prCfaFlv->rTagInfo.u4DataSize, u4ParsedLen,
						prCfaFlv->u4TagSize);
				if (IS_FLV_VALID_PREVTAGSZ
					(prCfaFlv->u4TagSize, prCfaFlv->rTagInfo.u4DataSize)) {
					prCfaFlv->fgTagIsValid = TRUE;
					/* transfer data */
					if (((AVCODEC_ID_H264 ==
						  prCfaFlv->rVideoInfo.rCfgInfo.eCodecID)
						 || (VCODEC_VERSION_H265_HM91 ==
						 prCfaFlv->rVideoInfo.rCfgInfo.rVersion))
						&& (CFA_FLV_TAG_TYPE_VIDEO ==
						prCfaFlv->rTagInfo.eTagType)
						&& (FLV_TAG_PKG_TYPE_FRAME == bAvcPacketType)
						&&
						((prCfaFlv->u4FirstUintPay + FLV_VID_AVC_DATA_HDR +
						  prCfaFlv->u4PayloadLenFieldSz) <
						 prCfaFlv->rTagInfo.u4DataSize)) {
						/* If the Video Tag is AVC Frame Packet(bAvcPacketType = 1) */
						/*and slice data size + slice data header < tag data size */
						/* the video tag raw data has several slices. */
						/* so we tranfer slice one by one. */
						prCfaFlv->u4VideoPartTagOffset = 0;
						/* u8Ca will be the file offset of the the 8*/
						/*first slice's len bytes addr */
						prCfaFlv->u8Ca += u4ParsedLen + FLV_TAG_HDR_SIZE + FLV_VID_AVC_DATA_HDR;
						prCfaFlv->eCurAnaSt =
							CFA_FLV_ANA_ST_TX_PARTTAG_V_HDR;
						CfaFlvSearchNextSc(pvSptHdl, prCfaFlv,
								   CFA_FLV_ANA_ST_TX_PARTTAG_V_HDR,
								   0,
								   (prCfaFlv->rTagInfo.u4DataSize -
									FLV_VID_AVC_DATA_HDR));
					} else {
#if CONFIG_CFA_FLV_FOR_ERR_JUMP
						prCfaFlv->u4OffsetJumpCnt = 0;
#endif
						prCfaFlv->u8Ca +=
							prCfaFlv->rTagInfo.u4DataSize +
							FLV_TAG_HDR_SIZE + u4ParsedLen +
							FLV_TAG_SIZE_BYTES;
						prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;
						CfaFlvPrsDmux(pvSptHdl, prCfaFlv);
					}
				} else {
					prCfaFlv->fgTagIsValid = FALSE;
					prCfaFlv->u8Ca = u8PreCa;
					CfaFlvSearchNextSc(pvSptHdl, prCfaFlv,
							   CFA_FLV_ANA_ST_SEARCH_TAG_HEADER, 0,
							   CFA_FLV_READ_LEN);
				}
				return;
			}

			break;

		} else if (CFA_FLV_TAG_TYPE_DATA == prCfaFlv->rTagInfo.eTagType) {
			fgFind = TRUE;

			u8PreCa = prCfaFlv->u8Ca + FLV_TAG_HDR_SIZE + u4ParsedLen;

			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d -- prCfaFlv->rTagInfo.eTagType=%d\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prCfaFlv->rTagInfo.eTagType);

			if (prCfaFlv->rTagInfo.u8Offset + prCfaFlv->rTagInfo.u4DataSize +
				FLV_TAG_HDR_SIZE + FLV_TAG_SIZE_BYTES ==
				prCfaFlv->rFileInfo.u8HeaderSize) {
				u4ParsedLen +=
					prCfaFlv->rTagInfo.u4DataSize + FLV_TAG_HDR_SIZE +
					FLV_TAG_SIZE_BYTES;
				fgFind = FALSE;
				continue;
			} else {
				prCfaFlv->fgTagIsValid = FALSE;
				u4ParsedLen++;
				fgFind = FALSE;
				continue;
			}
		}else {
			/*do nothing*/
		}
	}

	if (!fgFind) {
		if (u4TxLen < FLV_TAG_HDR_SIZE)
			prCfaFlv->u8Ca += 1;	/* end */
		else
			prCfaFlv->u8Ca += u4ParsedLen;

		if (FLV_TAG_TYPE_VALUE_SCRIPT == bTagType) {
#if CONFIG_CFA_FLV_FOR_ERR_JUMP
			prCfaFlv->u4OffsetJumpCnt = 0;
#endif
			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d  -- SCRIPT Tag --> ")
				    TEXT("CfaFlvSearchNextSc(pvSptHdl, prCfaFlv)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);

			CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
					   0, FLV_READ_FOR_CODEC_SIZE);
		} else {
#if CONFIG_CFA_FLV_FOR_ERR_JUMP
			prCfaFlv->u4OffsetJumpCnt++;
			if (prCfaFlv->u4OffsetJumpCnt > CFA_FLV_ERR_JUMP_MAX_NUM) {
				DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] %s -- u4OffsetJumpCnt(%d)")
					    TEXT(" > CFA_FLV_ERR_JUMP_MAX_NUM(%d)\r\n"),
					    DMX_FUNC_NAME, prCfaFlv->u4OffsetJumpCnt,
					    CFA_FLV_ERR_JUMP_MAX_NUM);
				prCfaFlv->u4OffsetJumpCnt = 0;
				DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] %s line %d -- Dmx Error ")
					    TEXT("call FinishPrs (GAU_E_ERRCHUNK)\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO);
				CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, GAU_E_ERRCHUNK);
				return;
			}

			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s -- JumpCnt(%d), u8Ca: 0x%08x%08x, ")
				    TEXT("u4TxLen: 0x%x, u4ParsedLen: 0x%x\r\n"),
				    DMX_FUNC_NAME, prCfaFlv->u4OffsetJumpCnt,
				    (u32) (prCfaFlv->u8Ca >> 32), (u32) (prCfaFlv->u8Ca),
				    u4TxLen, u4ParsedLen);
#endif				/* CONFIG_CFA_FLV_FOR_ERR_JUMP */

			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d  -- Not Find Tag --> ")
				    TEXT("CfaFlvSearchNextSc(pvSptHdl, prCfaFlv)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);

			CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
					   0, FLV_READ_FOR_CODEC_SIZE);
		}
	} else {
#if CONFIG_CFA_FLV_FOR_ERR_JUMP
		prCfaFlv->u4OffsetJumpCnt = 0;
#endif
		prCfaFlv->fgTagIsValid = TRUE;
		if (((AVCODEC_ID_H264 == prCfaFlv->rVideoInfo.rCfgInfo.eCodecID) ||
		     (VCODEC_VERSION_H265_HM91 == prCfaFlv->rVideoInfo.rCfgInfo.rVersion)) &&
		    (CFA_FLV_TAG_TYPE_VIDEO == prCfaFlv->rTagInfo.eTagType) &&
		    (FLV_TAG_PKG_TYPE_FRAME == bAvcPacketType) &&
		    ((prCfaFlv->u4FirstUintPay + FLV_VID_AVC_DATA_HDR +
		      prCfaFlv->u4PayloadLenFieldSz) < prCfaFlv->rTagInfo.u4DataSize)) {
			/* need divided tagdata into some frame */
			prCfaFlv->u4VideoPartTagOffset = 0;
			prCfaFlv->u8Ca += u4ParsedLen + FLV_TAG_HDR_SIZE + FLV_VID_AVC_DATA_HDR;
			prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_TX_PARTTAG_V_HDR;

			DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d  -- Video --> ")
				    TEXT("CfaFlvSearchNextSc(pvSptHdl, prCfaFlv)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);

			CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_TX_PARTTAG_V_HDR,
					   0,
					   (prCfaFlv->rTagInfo.u4DataSize - FLV_VID_AVC_DATA_HDR));
		} else {
			if ((prCfaFlv->prAudCmdQsInfo->u4EntryCnt > 0) ||
			    (prCfaFlv->prVidCmdQsInfo->u4EntryCnt > 0)) {
				prCfaFlv->eNextAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;
				prCfaFlv->u8Ca += u4ParsedLen;
				prCfaFlv->u4TxLen = FLV_READ_FOR_CODEC_SIZE;
				prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_TX_AV_CMDQ;
				prCfaFlv->u4AvailDataSz = 0;

				DmxLogD(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] %s line %d  -- CfaFlvTxAVCmdQ\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO);

				mrRet = CfaFlvTxAVCmdQ(pvSptHdl, prCfaFlv);
				if (DMX_FAILED(mrRet)) {
					DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						    TEXT("[CFA_FLV] failed in")
						    TEXT(" CfaFlvTxAVCmdQ, mrRet: 0x%x\r\n"),
						    mrRet);
					CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, GAU_E_FAIL);
					return;
				}

				return;
			}

			prCfaFlv->u8Ca +=
			    u4ParsedLen + FLV_TAG_HDR_SIZE + prCfaFlv->rTagInfo.u4DataSize +
			    FLV_TAG_SIZE_BYTES;
			prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;
			CfaFlvPrsDmux(pvSptHdl, prCfaFlv);
		}
	}

}

/* FLV CFA state control for transfer done */
/* @return None */
/* @note  This function will be called after a transfer is complete. */
/* [IN] handle of splitter */
/*[IN]  Actual transferred data length.  Normally this value should be equal to the u4Len in the */
/*       previous transfer issue, unless file end is hit. */
/*[IN] pointer to CfaFlvInst.*/
void CfaFlvTxDoneStCtrl(void *pvSptHdl, u64 u8TxLen, CfaFlvInst_T *prCfaFlv)
{
	MRESULT mrRet = RET_DMX_OK;

	/* using sync DMA, 2007/12/28 */
	/* check if Tx done results from Tx data to header buffer */
	do {
		prCfaFlv->fgNoNeedSyncPb = FALSE;
		if (prCfaFlv->fgRealSyncPb) {
			prCfaFlv->pu1HdrBuf = (u8 *) prCfaFlv->ptrPfrMemAddress;
			if (NULL == prCfaFlv->pu1HdrBuf) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] failed for ")
					    TEXT("pu1HdrBuf is NULL, mrRet: 0x%x.\r\n"),
					    RET_DMX_UNEXPECT);
				mrRet = Spt4CfaFinishedEx(pvSptHdl, prCfaFlv->u8Ca, TRUE, (u32)GAU_E_FAIL);
				return;
			}

			prCfaFlv->u4AvailDataSz += (u32) u8TxLen;
			prCfaFlv->fgRealSyncPb = FALSE;
#if CFA_FLV_DBG_AUD_CMD_Q_FLOW
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_CMDQ_A,
				    TEXT("[CFA_FLV] %s line %d -- fgRealSyncPb --> ")
				    TEXT("Ca: %lld, u4AvailDataSz: %d, PreCa: %lld\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, prCfaFlv->u8Ca,
				    prCfaFlv->u4AvailDataSz, prCfaFlv->u8PreCa);
#endif				/* CFA_FLV_DBG_AUD_CMD_Q_FLOW */
		} else {
			u8TxLen = prCfaFlv->u4TxLen;
		}

		if (NULL != prCfaFlv->prAudCmdQsInfo) {
			if (prCfaFlv->prAudCmdQsInfo->fgIsInDma) {
				dmx_memset(prCfaFlv->prAudCmdQsInfo, 0,
					   sizeof(CfaFlvAudCmdQInfo_T));
			}
		}

		if (NULL != prCfaFlv->prVidCmdQsInfo) {
			if (prCfaFlv->prVidCmdQsInfo->fgIsInDma) {
				dmx_memset(prCfaFlv->prVidCmdQsInfo, 0,
					   sizeof(CfaFlvVidCmdQInfo_T));
			}
		}
#if (CFA_FLV_DBG_VID_CMD_Q_FLOW || CFA_FLV_DBG_AUD_CMD_Q_FLOW)
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			    TEXT("[CFA_FLV] %s line %d -- state: %d, u8TxLen: %lld,")
			    TEXT(" Ca: %lld, u4AvailDataSz: %d, PreCa: %lld\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prCfaFlv->eCurAnaSt, u8TxLen,
			    prCfaFlv->u8Ca, prCfaFlv->u4AvailDataSz, prCfaFlv->u8PreCa);
#endif

		if ((!prCfaFlv->rRange.fgHasVid) && DMX_IS_RW_PLAY(pvSptHdl) && !prCfaFlv->fgJumpTurnOn) {
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d -- Spt4CfaFinishedEx(FALSE) ")
				    TEXT("for Pure Audio in Fast Rewind\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO);
			mrRet = Spt4CfaFinishedEx(pvSptHdl, prCfaFlv->u8Ca, FALSE, (u32)GAU_E_EOS);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] fail in Spt4CfaFinishedEx(FALSE)")
					    TEXT(" for Pure Audio in Fast Rewind, mrRet: 0x%x.\r\n"),
					    mrRet);
			}
			return;
		}

		switch (prCfaFlv->eCurAnaSt) {
		case CFA_FLV_ANA_ST_IDLE:
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					TEXT("[CFA_FLV] %s -- cfa state is idle now\r\n"),
				    DMX_FUNC_NAME);
			break;

		case CFA_FLV_ANA_ST_SEARCH_TAG_HEADER:
			CfaFlvSearchTagHeader(pvSptHdl, prCfaFlv, u8TxLen);
			break;

		case CFA_FLV_ANA_ST_SEARCH_HEADER:
			CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
					   0, (u64)FLV_READ_FOR_CODEC_SIZE);
			break;

		case CFA_FLV_ANA_ST_TX_AUD:
			if (!
			    (fgIsCfaStmToPlay
			     (prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_AUD))) {
				CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
						   0, (u64)FLV_READ_FOR_CODEC_SIZE);
			} else {
				prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;
#if CFA_FLV_DBG_AUD_CMD_Q_FLOW
				DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_CMDQ_A,
					    TEXT("[CFA_FLV] %s line %d -- CfaFlvTxAudData2Fifo\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO);
#endif				/* CFA_FLV_DBG_AUD_CMD_Q_FLOW */
				mrRet = CfaFlvTxAudData2Fifo(pvSptHdl, prCfaFlv);

                if(mrRet != RET_DMX_OK)
                {
                    DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						TEXT("[CFA_FLV] transfer audio data to fifo failed!\r\n"));
                }
			}
			break;

		case CFA_FLV_ANA_ST_TX_VID:
			if (!
			    (fgIsCfaStmToPlay
			     (prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_VID))) {
				CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
						   0, (u64)FLV_READ_FOR_CODEC_SIZE);
			} else {
				prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_HEADER;
				mrRet = CfaFlvTxVidData2Fifo(pvSptHdl, prCfaFlv);

                if(mrRet != RET_DMX_OK)
                {
                    DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						TEXT("[CFA_FLV] transfer video data to fifo failed!\r\n"));
                }
			}
			break;

		case CFA_FLV_ANA_ST_TX_A_HDR:
			if (!
			    (fgIsCfaStmToPlay
			     (prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_AUD))) {
				CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
						   0, FLV_READ_FOR_CODEC_SIZE);
			} else {
				prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_TX_AUD;
#if CFA_FLV_DBG_AUD_CMD_Q_FLOW
				DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_CMDQ_A,
					    TEXT("[CFA_FLV] %s line %d -- CfaFlvTxAudHdl2Fifo\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO);
#endif				/* CFA_FLV_DBG_AUD_CMD_Q_FLOW */
				mrRet = CfaFlvTxAudHdl2Fifo(pvSptHdl, prCfaFlv);

                if(mrRet != RET_DMX_OK)
                {
                    DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						TEXT("[CFA_FLV] CfaFlvTxAudHdl2Fifo failed!\r\n"));
                }
			}
			break;

		case CFA_FLV_ANA_ST_TX_PARTTAG_VIDEO:
			if (!
			    (fgIsCfaStmToPlay
			     (prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_VID))) {
				CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
						   0, FLV_READ_FOR_CODEC_SIZE);
			} else {
				mrRet = CfaFlvTxVidData2Fifo(pvSptHdl, prCfaFlv);
                if(mrRet != RET_DMX_OK)
                {
                    DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						TEXT("[CFA_FLV] CfaFlvTxVidData2Fifo failed!\r\n"));
                }
			}
			break;

		case CFA_FLV_ANA_ST_TX_PARTTAG_V_HDR:
			if (!
			    (fgIsCfaStmToPlay
			     (prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_VID))) {
				CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
						   0, FLV_READ_FOR_CODEC_SIZE);
			} else {
				prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_TX_PARTTAG_VIDEO;
				prCfaFlv->u8Ca += u8TxLen + FLV_TAG_SIZE_BYTES;
				mrRet = CfaFlvTxVidSeq2Fifo(pvSptHdl, prCfaFlv);
                if(mrRet != RET_DMX_OK)
                {
                    DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						TEXT("[CFA_FLV] CfaFlvTxVidSeq2Fifo failed!\r\n"));
                }
			}
			break;
		case CFA_FLV_ANA_ST_TX_PARTTAG_H265_HDR:
			if (!(fgIsCfaStmToPlay
				(prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_VID))) {
				CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
						0, FLV_READ_FOR_CODEC_SIZE);
			} else {
				prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_SEARCH_TAG_HEADER;
				mrRet = CfaFlvTxVidH265Seq2Fifo(pvSptHdl, prCfaFlv);
				if(mrRet != RET_DMX_OK)
				{
					DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						TEXT("[CFA_FLV] CfaFlvTxVidSeq2Fifo failed!\r\n"));
				}
			}
			break;

		case CFA_FLV_ANA_ST_TX_START_CODE:
			if (!
			    (fgIsCfaStmToPlay
			     (prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_VID))) {
				CfaFlvSearchNextSc(pvSptHdl, prCfaFlv, CFA_FLV_ANA_ST_SEARCH_TAG_HEADER,
						   0, FLV_READ_FOR_CODEC_SIZE);
			} else {
				prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_TX_PARTTAG_VIDEO;
				mrRet = CfaFlvTxStartCode2Fifo(pvSptHdl, prCfaFlv);
			    if(mrRet != RET_DMX_OK)
                {
                    DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
						TEXT("[CFA_FLV] CfaFlvTxStartCode2Fifo failed!\r\n"));
                }
			}
			break;

		case CFA_FLV_ANA_ST_TX_AV_CMDQ:
			mrRet = CfaFlvTxAVCmdQ(pvSptHdl, prCfaFlv);
        	if(mrRet != RET_DMX_OK)
            {
                DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					TEXT("[CFA_FLV] CfaFlvTxAVCmdQ failed!\r\n"));
            }
			break;

		default:
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] failed for ")
				    TEXT("unexpected CfaFlvState(%d), mrRet: 0x%x.\r\n"),
				    prCfaFlv->eCurAnaSt,
				    RET_DMX_UNEXPECT);
			mrRet = Spt4CfaFinishedEx(pvSptHdl, prCfaFlv->u8Ca, TRUE, GAU_E_FAIL);
			return;
		}
	} while (prCfaFlv->fgNoNeedSyncPb);

}

/*-----------------------------------------------------------------------------
* Name: u8CfaFlvGetTxSa
*
* Description:
*      FLV CFA get available transfer start address
*
* Inputs:
*
* Outputs:
*
* Returns: available transfer start address
*
*-----------------------------------------------------------------------------*/
u64 CfaFlvGetTxSa(CfaFlvInst_T *prCfaFlv)
{
	u64 u8StartAddr = DMX_INVALID_UINT64;

	if (DMX_INVALID_UINT64 != prCfaFlv->u8Ca)
		return prCfaFlv->u8Ca;

	if ((fgIsCfaStmToPlay(prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_VID)) &&
	    (DMX_INVALID_UINT64 != prCfaFlv->rRange.u8VidSa)) {
		u8StartAddr = prCfaFlv->rRange.u8VidSa;
	}

	if ((fgIsCfaStmToPlay(prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_AUD)) &&
	    (DMX_INVALID_UINT64 != prCfaFlv->rRange.u8AudSa)) {
		u8StartAddr = MIN(u8StartAddr, prCfaFlv->rRange.u8AudSa);
	}

	/* error handling only. */
	if (DMX_INVALID_UINT64 == u8StartAddr)
		u8StartAddr = 0;

	prCfaFlv->u8Ca = u8StartAddr;

	return u8StartAddr;
}


/*-----------------------------------------------------------------------------
* Name: fgCfaFlvIncPrsPos
*
* Description:
*      FLV CFA increases current parsing position, and check whether playing
*      to the end
*
* Inputs:
*      [IN] pointer to CfaFlvInst
*      [IN] file length added to FLV CFA current parsing position
*
* Outputs:
*
* Returns: TRUE - meet range end, FALSE - not meet range end
*
*-----------------------------------------------------------------------------*/
static bool CfaFlvIncPrsPos(CfaFlvInst_T *prCfaFlv, u64 u8Len)
{
	bool fgAdoFinished = FALSE;
	bool fgVdoFinished = FALSE;

	CfaFlvVidInfo_T *ptVInf = NULL;
	CfaFlvRange_T *prCfaRange = NULL;

	ptVInf = &(prCfaFlv->rVideoInfo);
	prCfaRange = &(prCfaFlv->rRange);

	if (prCfaFlv->u8Ca + 1 >= prCfaFlv->u8Endoffst)
		return TRUE;

	if ((fgIsCfaStmToPlay(prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_AUD)) &&
		(DMX_INVALID_UINT64 != prCfaRange->u8AudSa)) {
		fgAdoFinished = FALSE;
	} else {
		fgAdoFinished = TRUE;
	}

	if ((fgIsCfaStmToPlay(prCfaFlv->u4CurPrsFlag, CFA_FLV_PRS_BIT_STRM_TYPE_VID)) &&
	    (DMX_INVALID_UINT64 != prCfaRange->u8VidSa)) {
		if (ptVInf->rVTagInfo.u8Offset > prCfaRange->u8VidEa)
			fgVdoFinished = TRUE;
	} else {
		fgVdoFinished = TRUE;
	}

	return fgAdoFinished & fgVdoFinished;
}

/*------------------------------------------------------------------------------
* Descripiton: Start to search next tag header and get parsing payload info.
* @return None
* [IN] handle of splitter
* [IN] pointer to CfaFlvInst
* [IN] next analyze state
* [IN] previous transfer file length before search next start code
* [IN] the data length we want to read
* [IN] header buffer ofst. to put the data from this ofst
-------------------------------------------------------------------------------*/
EXTERN void CfaFlvSearchNextSc(void *pvSptHdl, CfaFlvInst_T *prCfaFlv,
			       CfaFlvAnaSt_E eNextAnaSt, u64 u8PreLen, u64 u8ReadLen)
{
	MRESULT mrRet = RET_DMX_OK;
	u64 u8Sa = DMX_INVALID_UINT64;
	u64 u8RangEndOffset = 0;

	u8RangEndOffset = prCfaFlv->u8Endoffst;

	/* check if all data are parsed */
	if (CfaFlvIncPrsPos(prCfaFlv, u8PreLen)) {
		/* finish current parsing */
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			    TEXT("[CFA_FLV] %s line %d -- Play to the end, call FinishPrs\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO);
		CfaFlvFinishPrs(pvSptHdl, prCfaFlv, (u32)GAU_E_EOS);
		return;
	}

	u8Sa = CfaFlvGetTxSa(prCfaFlv);
	if (DMX_INVALID_UINT64 == u8Sa) {
		CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, (u32)GAU_E_FAIL);
		return;
	}

	if (u8RangEndOffset <= (u8Sa + u8ReadLen)) {
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			    TEXT("[CFA_FLV] %s line %d -- u8RangEnd: %lld,")
			    TEXT(" u8Sa: %lld, u8ReadLen: %lld\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, u8RangEndOffset, u8Sa, u8ReadLen);
		u8ReadLen = u8RangEndOffset - u8Sa;
		CfaFlvFinishPrs(pvSptHdl, prCfaFlv, (u32)GAU_E_EOS);
		return;
	}

	prCfaFlv->eNextAnaSt = eNextAnaSt;
	prCfaFlv->eCurAnaSt = eNextAnaSt;
	prCfaFlv->u4TxLen = (u32) u8ReadLen;

	if (prCfaFlv->u8PreCa <= u8Sa) {
		if (prCfaFlv->u4AvailDataSz < u8Sa - prCfaFlv->u8PreCa) {
			prCfaFlv->u4AvailDataSz = 0;
			prCfaFlv->pu1HdrBuf = NULL;
		} else {
			prCfaFlv->u4AvailDataSz -= (u32) (u8Sa - prCfaFlv->u8PreCa);

#if (CFA_FLV_DBG_VID_CMD_Q_FLOW || CFA_FLV_DBG_AUD_CMD_Q_FLOW)
			DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] %s line %d -- Ca: %lld - PreCa: ")
				    TEXT("%lld = 0x%08x, pHdrHdrBuf(0x%08x --> 0x%08x)\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, u8Sa, prCfaFlv->u8PreCa,
				    (u32) (u8Sa - prCfaFlv->u8PreCa),
				    prCfaFlv->pu1HdrBuf,
				    (prCfaFlv->pu1HdrBuf +
				     (u32) (u8Sa - prCfaFlv->u8PreCa)));
#endif				/* (CFA_FLV_DBG_VID_CMD_Q_FLOW || CFA_FLV_DBG_AUD_CMD_Q_FLOW) */

			prCfaFlv->pu1HdrBuf += (u32) (u8Sa - prCfaFlv->u8PreCa);
		}
	} else {
		/* finish current parsing */
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			    TEXT("[CFA_FLV] %s line %d error, u8PreCa(0x%llx) > u8Sa(0x%llx)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO, prCfaFlv->u8PreCa, u8Sa);
		CfaFlvFinishPrs(pvSptHdl, prCfaFlv, (u32)GAU_E_FAIL);
		return;
	}

#if (CFA_FLV_DBG_VID_CMD_Q_FLOW || CFA_FLV_DBG_AUD_CMD_Q_FLOW)
	DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
		    TEXT("[CFA_FLV] %s line %d -- Ca: %lld, u4AvailDataSz: %d, u8ReadLen: %lld\r\n"),
		    DMX_FUNC_NAME, DMX_LINE_NO, u8Sa, prCfaFlv->u4AvailDataSz, u8ReadLen);
#endif				/*  (CFA_FLV_DBG_VID_CMD_Q_FLOW || CFA_FLV_DBG_AUD_CMD_Q_FLOW) */

	prCfaFlv->u8PreCa = u8Sa;

	if (prCfaFlv->u4AvailDataSz >= u8ReadLen) {
		prCfaFlv->fgNoNeedSyncPb = TRUE;
	} else {
		if ((prCfaFlv->prAudCmdQsInfo->u4EntryCnt > 0) ||
		    (prCfaFlv->prVidCmdQsInfo->u4EntryCnt > 0)) {
			prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_TX_AV_CMDQ;
			prCfaFlv->u4AvailDataSz = 0;
			mrRet = CfaFlvTxAVCmdQ(pvSptHdl, prCfaFlv);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
					    TEXT("[CFA_FLV] failed in CfaFlvTxAVCmdQ, mrRet: 0x%x\r\n"),
					    mrRet);
				CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, (u32)GAU_E_FAIL);
				return;
			}

			return;
		}
#if (CFA_FLV_DBG_VID_CMD_Q_FLOW || CFA_FLV_DBG_AUD_CMD_Q_FLOW)
		DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			    TEXT("[CFA_FLV] %s line %d -- SyncPbbufEx(ACmdCnt:")
			    TEXT(" %d, VCmdCnt: %d, Ca: %lld, SyncTxLen: %d)\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO,
			    prCfaFlv->prAudCmdQsInfo->u4EntryCnt,
			    prCfaFlv->prVidCmdQsInfo->u4EntryCnt, u8Sa, u8ReadLen);
#endif				/* (CFA_FLV_DBG_VID_CMD_Q_FLOW || CFA_FLV_DBG_AUD_CMD_Q_FLOW) */

		prCfaFlv->u4AvailDataSz = 0;
		prCfaFlv->fgRealSyncPb = TRUE;

		mrRet = Spt4CfaPbb2SyncBufEx(pvSptHdl, u8Sa, u8ReadLen,
					     (u8 *) &(prCfaFlv->ptrPfrMemAddress),
					     &(prCfaFlv->u4AvailDataSz));
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] u8RangEnd:0x%llx, ")
				    TEXT("u8Ca:0x%llx, u8ReadLen:0x%llx, mem:0x%x\r\n"),
				    u8RangEndOffset, prCfaFlv->u8Ca,
				    u8ReadLen, prCfaFlv->ptrPfrMemAddress);
			DmxLogE(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
				    TEXT("[CFA_FLV] failed in Spt4CfaPbb2SyncBuf,")
				    TEXT(" mrRet: 0x%x, call FinishPrs\r\n"),
				    mrRet);
			CfaFlvNotiDmxChunkError(pvSptHdl, prCfaFlv, (u32)GAU_E_FAIL);
			return;
		}
	}
}

#if CONFIG_CFA_FLV_FOR_ERR_JUMP
/* Description: set EOS before notify splliter demux error */
/* return none */
static bool CfaFlvNotiDmxChunkError(void *pvSptHdl, CfaFlvInst_T *prCfaFlv, u32 u4Status)
{
	Cfa2PsrStrmInfo rPathStrmInfo = { 0 };
	u32 i = 0;
	u32 j = 0;

	DmxLogT(DMX_MOD_CFA_FLV, CFA_FLV_LOG_DEFAULT,
			TEXT("[CFA FLV] %s -- u8Ca:0x%llx\r\n"),
		    DMX_FUNC_NAME, prCfaFlv->u8Ca);

	if (prCfaFlv->u4CurPrsFlag & CFA_FLV_PRS_BIT_STRM_TYPE_VID) {
		rPathStrmInfo.u4VstrmNs = 1;
		for (i = 0; i < rPathStrmInfo.u4VstrmNs; i++) {
			rPathStrmInfo.ucDecVidStId[i] =
			    (u8) (prCfaFlv->rVideoInfo.rCfgInfo.u1StrmNum);
		}
	}

	if (prCfaFlv->u4CurPrsFlag & CFA_FLV_PRS_BIT_STRM_TYPE_AUD) {
		rPathStrmInfo.u4AstrmNs = 1;
		for (j = 0; j < rPathStrmInfo.u4AstrmNs; j++)
			rPathStrmInfo.u2DecAudStId[j] = (prCfaFlv->rAudioInfo.rCfgInfo.u1StrmNum);
	}

	if (((u32)GAU_E_ERRCHUNK == u4Status) && (DMX_IS_RW_PLAY(pvSptHdl))) {
		prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_IDLE;

		Spt4CfaFinishedEx(pvSptHdl, prCfaFlv->u8Ca, FALSE, (u32)GAU_E_EOS);

		return TRUE;
	}

	prCfaFlv->eCurAnaSt = CFA_FLV_ANA_ST_IDLE;

	MMATE_CHECK_POINTER(prCfaFlv);

	Spt4CfaFinishedEx(pvSptHdl, prCfaFlv->u8Ca, TRUE, u4Status);

	return TRUE;
}
#endif
