/*************************************************************************************
 *LEGAL DISCLAIMER
 *
 * (Header of AutoChips Software/Firmware Release or Documentation)
 *
 * BY OPENING OR USING THIS FILE, USER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
 * AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * ARE PROVIDED TO USER ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS
 * ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED
 * IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND USER AGREES TO LOOK ONLY TO
 * SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AUTOCHIPS SHALL
 * ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE RELEASES MADE TO USER'S
 * SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 * USER HEREBY ACKNOWLEDGES THE CONFIDENTIALITY OF AUTOCHIPS SOFTWARE AND AGREES
 * NOT TO DISCLOSE OR PERMIT DISCLOSURE OF ANY AUTOCHIPS SOFTWARE TO ANY THIRD
 * PARTY OR TO ANY OTHER PERSON, EXCEPT TO DIRECTORS, OFFICERS, EMPLOYEES OF
 * USER WHO ARE REQUIRED TO HAVE THE INFORMATION TO CARRY OUT THE PURPOSE OF
 * OPENING OR USING THIS FILE.
*************************************************************************************/

#include <linux/fs.h>

#include "tvd_wch_mgr.h"
#include "ac823x/wch_drv.h"


static struct mutex g_TWMgrLock;
static TWManager_Node_T g_HWNode[SRC_MAX];

static s32 requestNrHw(u32 srcType)
{
	s32 idx = 0;

	switch (srcType) {
	case SRC_BACKCAR:
	case SRC_CVBS:
		for (idx = 0; idx < SRC_AVM; idx++) {
			if ((HW_STATUS_REQUEST == g_HWNode[idx].status) &&
				(HW_NR & g_HWNode[idx].hwType)) {
				if (g_HWNode[srcType].priority >= g_HWNode[idx].priority) {
					pr_debug("[TWMgr]%s: NR is used by source(%d)\n", __func__, idx);
					g_HWNode[idx].stop();
					g_HWNode[idx].status = HW_STATUS_ROBBED;
					g_HWNode[idx].robbedBySrcIdx = srcType;
					g_HWNode[srcType].fgRobbed[idx] = true;
					return 0;
				} else {
					pr_err("[TWMgr]%s: source(%d) can not rob source(%d) with NR\n",
						__func__, srcType, idx);
					return -1;
				}
			}
		}
		break;

	default:
		break;
	}

	return 0;
}

static s32 requestTvdHw(u32 srcType, u32 tvdType)
{
	s32 idx = 0;

	switch (srcType) {
	case SRC_BACKCAR:
	case SRC_CVBS:
		for (idx = 0; idx < SRC_DIGIN1; idx++) {
			if ((HW_STATUS_REQUEST == g_HWNode[idx].status) &&
				(tvdType & g_HWNode[idx].hwType)) {
				if (g_HWNode[srcType].priority >= g_HWNode[idx].priority) {
					pr_debug("[TWMgr]%s: tvd(0x%x) is used by source(%d) with tvd(0x%x)\n",
						__func__, tvdType, idx, g_HWNode[idx].hwType & 0xF);
					g_HWNode[idx].stop();
					g_HWNode[idx].status = HW_STATUS_ROBBED;
					g_HWNode[idx].robbedBySrcIdx = srcType;
					g_HWNode[srcType].fgRobbed[idx] = true;
					return 0;
				} else {
					pr_err("[TWMgr]%s: source(%d) can not rob source(%d) with tvd(0x%x)\n",
						__func__, srcType, idx, tvdType);
					return -1;
				}
			}
		}
		break;

	case SRC_AVM:
		if ((HW_STATUS_REQUEST == g_HWNode[SRC_BACKCAR].status) &&
			(g_HWNode[SRC_BACKCAR].hwType & HW_TVD_ALL)){
			pr_err("[TWMgr]%s: avm can not rob backcar tvd resource\n", __func__);
			return -1;
		} else if ((HW_STATUS_REQUEST == g_HWNode[SRC_CVBS].status) &&
			(g_HWNode[SRC_CVBS].hwType & HW_TVD_ALL)) {
			if ((g_HWNode[srcType].priority >= g_HWNode[SRC_CVBS].priority)) {
				pr_debug("[TWMgr]%s: tvd(0x%x) is used by source(%d) with tvd(0x%x)\n",
					__func__, tvdType, SRC_CVBS, g_HWNode[SRC_CVBS].hwType & 0xF);
				g_HWNode[SRC_CVBS].stop();
				g_HWNode[SRC_CVBS].status = HW_STATUS_ROBBED;
				g_HWNode[SRC_CVBS].robbedBySrcIdx = srcType;
				g_HWNode[srcType].fgRobbed[SRC_CVBS] = true;
				return 0;
			} else {
				pr_err("[TWMgr]%s: source(%d) can not rob source(%d) with tvd(0x%x)\n",
					__func__, srcType, SRC_CVBS, tvdType);
				return -1;
			}
		}
		break;

	default:
		break;
	}

	return 0;
}

static s32 requestWchHw(u32 srcType, u32 tvdType, u32 *pIdx)
{
	u32 idx = 0;
	u32 srcIdx = 0;
	u32 wchConflictIdx[WCH_NUM] = {0};
	u32 wchConflictCnt = 0;
	bool fgAllConflict = false;
	s32 firstConflictSrc = -1;

	switch (srcType) {
	case SRC_BACKCAR:
		wchConflictIdx[0] = WCH_5;
		wchConflictIdx[1] = WCH_1;
		wchConflictCnt = 2;
		break;

	case SRC_CVBS:
		if (HW_TVD0 == tvdType) {
			wchConflictIdx[0] = WCH_5;
			wchConflictIdx[1] = WCH_1;
			wchConflictCnt = 2;
		} else if ((HW_TVD1 == tvdType) || (HW_TVD2 == tvdType) || (HW_TVD3 == tvdType)) {
			wchConflictIdx[0] = WCH_5;
			wchConflictCnt = 1;
		} else {
			pr_err("[TWMgr]%s: tvdType(0x%x) error with SRC_CVBS\n", __func__, tvdType);
			return -1;
		}
		break;

	case SRC_AVM:
		wchConflictIdx[0] = WCH_5;
		wchConflictIdx[1] = WCH_1;
		wchConflictCnt = 2;
		fgAllConflict = true;
		break;

	case SRC_DIGIN1:
		wchConflictIdx[0] = WCH_8;
		wchConflictCnt = 1;
		break;

	case SRC_DIGIN2:
		wchConflictIdx[0] = WCH_1;
		wchConflictCnt = 1;
		break;


	case SRC_BT1120:
		wchConflictIdx[0] = WCH_8;
		wchConflictCnt = 1;
		break;

	case SRC_YPBPR:
		*pIdx = WCH_6;
		return 0;

	case SRC_HDMI:
		*pIdx = WCH_7;
		return 0;

	case SRC_VOUT:
		*pIdx = WCH_9;
		return 0;

	default:
		pr_err("[TWMgr]%s: srcType(%d) error\n", __func__, srcType);
		return -1;
	}

	for (idx = 0; idx < wchConflictCnt; idx++) {
		for (srcIdx = 0; srcIdx < SRC_MAX; srcIdx++) {
			if ((HW_STATUS_REQUEST == g_HWNode[srcIdx].status) &&
				(1U << (wchConflictIdx[idx] + 8U)) & g_HWNode[srcIdx].hwType) {
				if (g_HWNode[srcType].priority >= g_HWNode[srcIdx].priority) {
					if (fgAllConflict || (1 == wchConflictCnt)) {
						pr_debug("[TWMgr]%s: wch%d is used by source(%d)\n",
							__func__, wchConflictIdx[idx] + 1, srcIdx);
						g_HWNode[srcIdx].stop();
						g_HWNode[srcIdx].status = HW_STATUS_ROBBED;
						g_HWNode[srcIdx].robbedBySrcIdx = srcType;
						g_HWNode[srcType].fgRobbed[srcIdx] = true;
						*pIdx = wchConflictIdx[idx];
					} else {
						if (-1 == firstConflictSrc) {
							firstConflictSrc = srcIdx;
							*pIdx = wchConflictIdx[idx];
						}
					}
				} else {
					if (fgAllConflict || (1 == wchConflictCnt)) {
						pr_err("[TWMgr]%s: source(%d) can not rob source(%d) with wch%d\n",
							__func__, srcType, srcIdx, wchConflictIdx[idx] + 1);
						return -1;
					}
				}
				break;
			}
		}
		if ((SRC_MAX == srcIdx) && (!fgAllConflict)) {
			*pIdx = wchConflictIdx[idx];
			return 0;
		}
	}

	if (-1 != firstConflictSrc) {
		pr_debug("[TWMgr]%s: first optional wch%d is used by source(%d)\n",
			__func__, *pIdx + 1, firstConflictSrc);
		g_HWNode[firstConflictSrc].stop();
		g_HWNode[firstConflictSrc].status = HW_STATUS_ROBBED;
		g_HWNode[firstConflictSrc].robbedBySrcIdx = srcType;
		g_HWNode[srcType].fgRobbed[firstConflictSrc] = true;
	}

	return 0;
}

s32 TWMgr_init(void)
{
	u32 idx = 0;

	mutex_init(&g_TWMgrLock);
	memset(g_HWNode, 0, sizeof(g_HWNode));
	for (idx = 0; idx < SRC_MAX; idx++) {
		if (SRC_BACKCAR == idx) {
			g_HWNode[idx].priority = HW_PRI_HIGH;
		} else {
			g_HWNode[idx].priority = HW_PRI_MEDIUM;
		}
	}

	return 0;
}

s32 TWMgr_requestHw(u32 srcType, u32 tvdType,
	TWManager_Callback start, TWManager_Callback stop)
{
	s32 ret = 0;
	s32 wchIdx = 0;

	mutex_lock(&g_TWMgrLock);
	pr_debug("[TWMgr]%s: enter with srcType(%d) tvdType(0x%x)\n",
		__func__, srcType, tvdType);
	if (srcType >= SRC_MAX) {
		pr_err("[TWMgr]%s: srcType(%d) error\n", __func__, srcType);
		mutex_unlock(&g_TWMgrLock);
		return -1;
	}
	if ((g_HWNode[srcType].status == HW_STATUS_REQUEST) ||
		(g_HWNode[srcType].status == HW_STATUS_ROBBED)) {
		pr_err("[TWMgr]%s: srcType(%d) error with status(%d)\n",
			__func__, srcType, g_HWNode[srcType].status);
		mutex_unlock(&g_TWMgrLock);
		return -1;
	}

	if (g_HWNode[SRC_BACKCAR].status == HW_STATUS_REQUEST) {
		pr_err("[TWMgr]%s: backcar has already started\n", __func__);
		mutex_unlock(&g_TWMgrLock);
		return -1;
	}

#ifdef CVBS_NR_ENABLE
	ret = requestNrHw(srcType);
	if (ret) {
		pr_err("[TWMgr]%s: requestNrHw fail with srcType(%d)\n",
			__func__, srcType);
		mutex_unlock(&g_TWMgrLock);
		return -1;
	}
	g_HWNode[srcType].hwType |= HW_NR;
#endif

	ret = requestTvdHw(srcType, tvdType);
	if (ret) {
		pr_err("[TWMgr]%s: requestTvdHw fail with srcType(%d) tvdType(0x%x)\n",
			__func__, srcType, tvdType);
		mutex_unlock(&g_TWMgrLock);
		return -1;
	}
	g_HWNode[srcType].hwType |= tvdType;

	ret = requestWchHw(srcType, tvdType, &wchIdx);
	if (ret) {
		pr_err("[TWMgr]%s: requestWchHw fail with srcType(%d) tvdType(0x%x)\n",
			__func__, srcType, tvdType);
		mutex_unlock(&g_TWMgrLock);
		return -1;
	}
	pr_debug("[TWMgr]%s: requestWchHw success with wch%d\n", __func__, wchIdx + 1);
	g_HWNode[srcType].hwType |= (1U << (wchIdx + 8U));

	g_HWNode[srcType].status = HW_STATUS_REQUEST;
	g_HWNode[srcType].start = start;
	g_HWNode[srcType].stop = stop;

	mutex_unlock(&g_TWMgrLock);

	return wchIdx;
}

s32 TWMgr_releaseHw(u32 srcType)
{
	u32 idx = 0;
	u32 srcIdx = 0;

	mutex_lock(&g_TWMgrLock);
	pr_debug("[TWMgr]%s: enter with srcType(%d)\n", __func__, srcType);
	if (HW_STATUS_ROBBED == g_HWNode[srcType].status) {
		srcIdx = g_HWNode[srcType].robbedBySrcIdx;
		for (idx = 0; idx < SRC_MAX; idx++) {
			if (g_HWNode[srcType].fgRobbed[idx]) {
				g_HWNode[srcIdx].fgRobbed[idx] = true;
			}
		}
		g_HWNode[srcType].hwType = 0;
		g_HWNode[srcType].status = HW_STATUS_RELEASE;
		g_HWNode[srcType].start = NULL;
		g_HWNode[srcType].stop = NULL;
	} else if (HW_STATUS_REQUEST == g_HWNode[srcType].status){
		for (idx = 0; idx < SRC_MAX; idx++) {
			if (g_HWNode[srcType].fgRobbed[idx]) {
				if (HW_STATUS_ROBBED == g_HWNode[idx].status) {
					g_HWNode[idx].start();
					g_HWNode[idx].status = HW_STATUS_REQUEST;
				}
				g_HWNode[srcType].fgRobbed[idx] = false;
			}
		}
		g_HWNode[srcType].hwType = 0;
		g_HWNode[srcType].status = HW_STATUS_RELEASE;
		g_HWNode[srcType].start = NULL;
		g_HWNode[srcType].stop = NULL;
	} else {
		pr_debug("[TWMgr]%s: srcType(%d) don't need to release with status(%d)\n",
			__func__, srcType, g_HWNode[srcType].status);
	}
	mutex_unlock(&g_TWMgrLock);

	return 0;
}

