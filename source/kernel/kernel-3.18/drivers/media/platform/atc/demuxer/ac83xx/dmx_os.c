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

/*!
 * @file demuxer.c
 *
 * @par Project
 *	  MT3360
 *
 * @par Description
 *	  Demuxer Os interface layer, demuxer ioctrl definitions
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */
#include "x_ver.h"
#include "x_assert.h"
#include "stddef.h"
#ifdef __linux__
#include <linux/mm.h>
#include <linux/errno.h>
#include "windows.h"
#include "winutil.h"
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/x_dmx.h>
#include <media/atc/ioctl_dmx.h>
#include <media/atc/perf_timer.h>
#include <media/atc/memchk_cfg.h>
#include <media/atc/memdbg_c.h>
/* #include <media/atc/mm_debug.h> */
#include "mmisc.h"
#else
#include "dmx_define.h"
#include "dmx_splitter.h"
#include "x_dmx.h"
#include "ioctl_dmx.h"
#include "perf_timer.h"
#include "memchk_cfg.h"
#include "memdbg_c.h"
#include "mm_debug.h"
#endif /* __linux__*/

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_cli.h"
#include "dmx_dump.h"
#include "dmx_spt.h"
#include "dmx_spt_if.h"
#include "dmx_pbbuf.h"
#include "dmx_pbbuf_if.h"
#include "dmx_esm.h"
#include "dmx_gau.h"
#include "dmx_cpsa.h"
#include "dmx_gau_if.h"
#include "dmx_stream.h"
#include "dmx_parser.h"
#include "dmx_hal_if.h"
#include "dmx_inst.h"
#include "stc_hal.h"

#if DMX_SUPPORT_DIVXDRM
#ifndef __linux__
#include "metazoneex.h"
#else
#include "metazone.h"
#endif /* __linux__*/
#endif /* DMX_SUPPORT_DIVXDRM*/

#include "dmx_pfm.h"

#ifndef __linux__
#pragma warning(disable : 4127) /*disable warning C4127: conditional expression is constant*/
#pragma warning(disable : 4100) /*disable warning C4100: unreferenced formal parameter*/
#endif /* __linux__*/

const char *g_aszStrmTypeName[MAX_ES_TYPE_CNT] = {
	TEXT("Unknown"),
	TEXT("Video"),
	TEXT("Audio"),
	TEXT("Subtitle"),
	TEXT("Section")
};

const char *g_aszSptDataTypeName[MAX_SPT_DATA_TYPE_CNT] = {
	TEXT("Unknown"),
	TEXT("Video"),
	TEXT("Audio"),
	TEXT("SP/CC"),
	TEXT("Section"),
	TEXT("BUF"),
	TEXT("GRD"),
};

s32	 g_i4DmxOpenRefCnt = 0;

#ifdef __linux__
s32	 g_Spt_EnableCnt = 0;
#endif /* __linux__*/

EXTERN u32	g_u4PbBufFlag;
EXTERN	BOOL g_fgDmxDmaTwice;
EXTERN DMX_INST_LIST_T g_rDmxInstList;


/*
 * This function will be called while driver attached.
 */
u32 DMX_Init(const char * pContext, u32 dwBusContext)
{
	MRESULT mrRet = RET_DMX_OK;
	bool fgRet = FALSE;

	MOD_VERSION_INFO(DMX_MOD_NAME, DMX_VER_MAIN, DMX_VER_MINOR, DMX_VER_REV);

	fgRet = LOG_ModInit();
	if (!fgRet) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DMX] %s fail in LOG_ModInit()\r\n"),
		DMX_FUNC_NAME);
		return 0;
	}

	fgRet = OSE_MemInit(OSE_DEMUXER);
	if (!fgRet) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DMX] %s fail in OSE_MemInit(OSE_DEMUXER)\r\n"),
			DMX_FUNC_NAME);
		LOG_ModDeinit();
		return 0;
	}

	smp_mb();

	#if DMX_SUPPORT_DIVXDRM
	#ifndef __linux__
	MetaZone_Init();
	#endif /* __linux__*/
	#endif /* DMX_SUPPORT_DIVXDRM*/

	dmx_sema_init();

	DmxInitLog();

	DmxLogEnable(TRUE, DMX_LOG_TRACE, -1, -1);
	DmxLogEnable(TRUE, DMX_LOG_ERROR, -1, -1);
	DmxLogEnable(TRUE, DMX_LOG_INFO, -1, -1);

	/* DmxLogDEnable(TRUE, DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_CMDQ_A | */
	/* DMX_MOD_OTH_LOGLVL_INSTBS | DMX_MOD_OTH_LOGLVL_DMAPBBUF_A);*/
	/*DmxLogDEnable(TRUE, DMX_MOD_GAU, DMX_MOD_GAU_LOGLVL_GETAU_A | DMX_MOD_GAU_LOGLVL_GETAU_V |
		DMX_MOD_GAU_LOGLVL_GETAU_SP);*/
	/*DmxLogDEnable(TRUE, DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_FIFOFULL);*/

	mrRet = DmxInit();
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DMX] %s fail in DmxInst(), mrRet: 0x%x\r\n"),
		DMX_FUNC_NAME, (unsigned int)mrRet);

		dmx_sema_deinit();

		#if DMX_SUPPORT_DIVXDRM
		#ifndef __linux__
		MetaZone_Deinit();
		#endif /* __linux__*/
		#endif /* DMX_SUPPORT_DIVXDRM*/

		OSE_MemUninit(OSE_DEMUXER);

		LOG_ModDeinit();
		return 0;
	}

	return 1;
}


/*
 * This function will be called while drvier detached.
 */
bool DMX_Deinit(void)
{
	MRESULT mrRet = RET_DMX_OK;

	mrRet = DmxUninit();
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s line %d failed in DmxUninit, mrRet: 0x%x.\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		return FALSE;
	}


	#if DMX_SUPPORT_DIVXDRM
	#ifndef __linux__
	MetaZone_Deinit();
	#endif /* __linux__*/
	#endif /* DMX_SUPPORT_DIVXDRM*/


	DmxDeInitLog();

	dmx_sema_deinit();

	OSE_MemUninit(OSE_DEMUXER);

	LOG_ModDeinit();

	return TRUE;
}

void *DMX_Open(void)
{
	void *dwInstHdl = (void *)dmx_inst_create();

	if (NULL == dwInstHdl) {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s fail in dmx_inst_create, RefCnt: %d\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, g_i4DmxOpenRefCnt);
		return 0;
	}

	g_i4DmxOpenRefCnt++;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("[DMX] %s Success, RefCnt: %d\r\n"),
		DMX_FUNC_NAME, g_i4DmxOpenRefCnt);

	return dwInstHdl;
}

bool DMX_Close(void *dwParam)
{
	bool fgRet = FALSE;

	if (g_i4DmxOpenRefCnt <= 0) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DMX] %s fail for DMX Open Reference Count == 0\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return FALSE;
	}

	fgRet = dmx_inst_release(dwParam);

	if (!fgRet) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("[DMX] %s fail in dmx_inst_release(Context: 0x%p)\r\n"),
			DMX_FUNC_NAME, dwParam);
	}

	g_i4DmxOpenRefCnt--;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
		TEXT("[DMX] %s Success, g_Spt_EnableCnt: %d, RefCnt: %d\r\n"),
		DMX_FUNC_NAME, g_Spt_EnableCnt, g_i4DmxOpenRefCnt);

	return fgRet;
}

u32 DMX_Read(u32 hOpenContext, void * pBuffer, u32 Count)
{
  return (0);
}

MRESULT DMX_GetMWErrCode(MRESULT mrRet)
{
	MRESULT mrExtRet = RET_DMX_EXT_OK;

	switch (mrRet) {
	case RET_DMX_OS_OPERA_FAIL:
	case RET_DMX_NO_MEM:
		mrExtRet = RET_DMX_EXT_NO_MEM;
		break;

	case RET_DMX_PARAM_WRONG:
		mrExtRet = RET_DMX_EXT_PARAM_WRONG;
		break;

	case RET_DMX_OPERATE_FORBID:
		mrExtRet = RET_DMX_EXT_OPER_FORBID;
		break;

	case RET_DMX_OVER_LIMIT:
		mrExtRet = RET_DMX_EXT_OVER_LIMIT;
		break;

	case RET_DMX_ERR_STATE:
		mrExtRet = RET_DMX_EXT_INV_STATE;
		break;

	case RET_DMX_NO_INIT:
	case RET_DMX_NO_AUTABLE:
	case RET_DMX_NO_CC:
	case RET_DMX_NO_SET_TYPE:
	case RET_DMX_NO_CFA_INTERFACE:
	case RET_DMX_NO_CFA_PRIV_DATA:
		mrExtRet = RET_DMX_EXT_NO_INIT;
		break;

	case RET_DMX_CPSA_NEVER_AUTHED:
		mrExtRet = RET_DMX_EXT_DECRYPT_NOT_AUTHORIZED;
		break;

	case RET_DMX_CPSA_NOT_REGED:
		mrExtRet = RET_DMX_EXT_DECRYPT_NOT_REGISTERED;
		break;

	case RET_DMX_CPSA_RENTAL_EXPIRED:
		mrExtRet = RET_DMX_EXT_DECRYPT_RENTAL_EXPIRED;
		break;

	case RET_DMX_CPSA_GEN_ERR:
		mrExtRet = RET_DMX_EXT_DECRYPT_GENERAL_ERROR;
		break;

	case RET_DMX_CPSA_NEVER_REGED:
		mrExtRet = RET_DMX_EXT_DECRYPT_NEVER_REGISTERED;
		break;

	case RET_DMX_UNEXPECT:
	default:
		mrExtRet = RET_DMX_EXT_EXCEPTION;
		break;
	}

	return mrExtRet;
}

void DMX_SetMWErrCode(MRESULT mrRet)
{
	MRESULT mrExtRet = RET_DMX_OK;

	mrExtRet = DMX_GetMWErrCode(mrRet);

#ifdef __linux__
	//SetLastError(mrExtRet);
#else
	OSE_SetErrCode(mrExtRet);
#endif /* __linux__*/

}

#ifndef __linux__
static bool DMX_GetPowerStateString(CEDEVICE_POWER_STATE eState,
char *szPowerStr, u32 u4Len)
{
	if (u4Len < 3)
		return FALSE;

	mm_memset(szPowerStr, 0, u4Len);

	switch (eState) {
	case D0:
		strcpy(szPowerStr, "D0");
		break;
	case D1:
		strcpy(szPowerStr, "D1");
		break;
	case D2:
		strcpy(szPowerStr, "D2");
		break;
	case D3:
		strcpy(szPowerStr, "D3");
		break;
	case D4:
		strcpy(szPowerStr, "D4");
		break;
	default:
		strcpy(szPowerStr, "DN");
		break;
	}

	return TRUE;
}
#endif /* __linux__*/

bool DMX_IOControl(void *dwOpenContext, unsigned int cmd, unsigned long arg, bool fgIsUserMem)
{
	MRESULT mrRet = RET_DMX_OK;

	if (g_rDmxInstList.u4Cnt <= 0) {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOControl fail, demuxer has been closed.\r\n"),DMX_FUNC_NAME, DMX_LINE_NO);
		return FALSE;
	}

	switch (cmd) {
	case DMX_IOCTL_IS_BUSY: {
		void *pvInstHdl = NULL;

		pvInstHdl = DmxInstGet();

		if (0 == pvInstHdl) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DmxInstGet fail, dwInstHdl is 0.\r\n"));
			DMX_ASSERT(FALSE);
		}
		if (0 != copy_to_user((void *)arg, (void *)&pvInstHdl, sizeof(void *))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_IS_BUSY fail in copy_to_user")
					TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		break;
	}

	case DMX_IOCTL_RESET: {
		mrRet = DmxInstReset(dwOpenContext);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("DmxInstReset fail. \r\n"));
			DMX_ASSERT(FALSE);
		}
		
		break;
	}

	case DMX_IOCTL_SET_FLAG: {
		if (0 == arg) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_SET_FLAG fail for invalid arg")
					TEXT(" -- arg is NULL\r\n"));
			mrRet = RET_DMX_PARAM_WRONG;
			break;
		}
		if (0 != copy_from_user((void *)&g_u4PbBufFlag, (void *)arg, sizeof(u32))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_SET_FLAG fail in ")
				TEXT("copy_from_user -- arg: 0x%p\r\n"),
				arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}
		break;
		
	}

	case DMX_IOCTL_SPT_OPEN_GET_CNT: {
		if (0 != copy_to_user((void *)arg, (void *)&g_Spt_EnableCnt, sizeof(u32))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_SPT_OPEN_GET_CNT fail in ")
				TEXT("copy_to_user -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}
		
		break;
	}

	case DMX_IOCTL_SPT_CREATE: {
		void *pvHdl = NULL;

		mrRet = dmx_sptinst_create(dwOpenContext, &pvHdl);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_SPT_CREATE fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		if (0 != copy_to_user((void *)arg, (void *)&pvHdl, sizeof(void *))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_SPT_CREATE fail in copy_to_user")
					TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		break;
	}

	case DMX_IOCTL_SPT_DESTROY: {
		void *pvSptHdl = NULL;

		if (0 != copy_from_user((void *)&pvSptHdl, (void *)arg, sizeof(void *))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_SPT_ENABLE fail in copy_from_user")
					TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = dmx_sptinst_destroy(dwOpenContext, pvSptHdl);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_SPT_DESTROY fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	case DMX_IOCTL_SPT_ENABLE: {
		SPT_PARAM_ENABLE rParam;

		mm_memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg, sizeof(SPT_PARAM_ENABLE))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_SPT_ENABLE fail in copy_from_user")
				TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}
		mrRet = dmx_sptinst_enable(dwOpenContext, rParam.pvSptHdl,
			&(rParam.rPbbufCfgInfo));

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_SPT_ENABLE fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		} else {
			if (g_Spt_EnableCnt < DMX_MAX_SPT_INST_CNT) {
				g_Spt_EnableCnt++;
			} else {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("DMX_IOCTL_SPT_ENABLE g_Spt_EnableCnt(%d)")
				TEXT(" already >= DMX_MAX_SPT_INST_CNT\r\n"),
					g_Spt_EnableCnt);
				DMX_ASSERT(FALSE);
			}
		}

		break;
	}

	case DMX_IOCTL_SPT_DISABLE: {
		void *pvSptHdl = NULL;

		if (0 != copy_from_user((void *)&pvSptHdl, (void *)arg, sizeof(void *))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_SPT_DISABLE fail in copy_from_user")
					TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = dmx_sptinst_disable(dwOpenContext, (HANDLE)pvSptHdl);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_SPT_DISABLE fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		} else {
			if (g_Spt_EnableCnt > 0) {
				g_Spt_EnableCnt--;
			} else {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("DMX_IOCTL_SPT_DISABLE g_Spt_EnableCnt(%d)")
					TEXT(" already <= 0\r\n"),
					g_Spt_EnableCnt);
				DMX_ASSERT(FALSE);
			}
		}

		break;
	}

	case DMX_IOCTL_SPT_IS_ENABLED: {
		void *pvSptHdl = NULL;

		if (0 != copy_from_user((void *)&pvSptHdl, (void *)arg, sizeof(void *))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_SPT_IS_ENABLED fail in copy_from_user")
					TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		return dmx_spt_isenable(dwOpenContext, (HANDLE)pvSptHdl);
	}

	case DMX_IOCTL_SPT_REBUF_RANGE:
		break;

	case DMX_IOCTL_SPT_SET_RATE: {
		SPT_PARAM_SET_RATE rParam;

		mm_memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg, sizeof(SPT_PARAM_SET_RATE))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_SPT_SET_RATE fail in copy_from_user")
				TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}
		mrRet = DmxSetRate(dwOpenContext, &rParam);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_SPT_SET_RATE fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	/* For Stream relative control function. */
	case DMX_IOCTL_STM_CREATE: {
		DMX_CREATE_STM_PARAM_T rParam;
		STM_PARAM_CREATE rStmParam;
		void *pvHdl = NULL;

		mm_memset(&rParam, 0, sizeof(rParam));
		mm_memset(&rStmParam, 0, sizeof(rStmParam));
		
		if (0 != copy_from_user((void *)&rParam, (void *)arg, sizeof(DMX_CREATE_STM_PARAM_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_CREATE fail in copy_from_user")
				TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}
		
		rStmParam = rParam.rStmParam;
		
		mrRet = dmx_stminst_create(dwOpenContext, &rStmParam, &pvHdl);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_CREATE fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}
		
		rParam.pvStmHdl = pvHdl;
		
		if (0 != copy_to_user((void *)arg, (void *)&rParam, sizeof(DMX_CREATE_STM_PARAM_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_CREATE fail in copy_to_user")
				TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		break;
	}

	case DMX_IOCTL_STM_DESTROY: {
		STM_PARAM_DESTROY rParam;

		mm_memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg, sizeof(STM_PARAM_DESTROY))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_DESTROY fail in copy_from_user")
				TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}
		mrRet = dmx_stminst_destroy(dwOpenContext, rParam.pvSptHdl, rParam.pvStmHdl);
			
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_DESTROY fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	case DMX_IOCTL_STM_ENABLE: {
		void *pvStm = NULL;

		if (0 != copy_from_user((void *)&pvStm, (void *)arg, sizeof(void *))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_ENABLE fail in ")
				TEXT("copy_from_user -- arg:0x%p\r\n"), arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = StreamEnable(pvStm);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_ENABLE fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	case DMX_IOCTL_STM_DISABLE: {
		void *pvStm = NULL;

		if (0 != copy_from_user((void *)&pvStm, (void *)arg, sizeof(void *))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_DISABLE fail in copy_from_user")
					TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = StreamDisable(pvStm);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_DISABLE fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	case DMX_IOCTL_STM_SET_FIFO_SZ: {
		STM_PARAM_SET_FIFO_SZ rParam;

		mm_memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg, sizeof(STM_PARAM_SET_FIFO_SZ))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_SET_FIFO_SZ fail in ")
				TEXT("copy_from_user -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}
		mrRet = StreamSetFifoInfo(rParam.pvStmHdl, rParam.u4Sz);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_SET_FIFO_SZ fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	case DMX_IOCTL_STM_SET_THRESHOLD: {
		STM_PARAM_SET_THRESHOLD rParam;

		mm_memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg, sizeof(STM_PARAM_SET_THRESHOLD))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_SET_THRESHOLD fail in ")
				TEXT("copy_from_user -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}
		mrRet = StreamSetFifoThreshold(rParam.pvStmHdl, rParam.u4Threshold);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_SET_THRESHOLD fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	case DMX_IOCTL_STM_FLUSH_FIFO: {
		void *pvStm = NULL;

		if (0 != copy_from_user((void *)&pvStm, (void *)arg, sizeof(void *))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_FLUSH_FIFO fail in copy_from_user")
					TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = StreamSetFlush(pvStm);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_FLUSH_FIFO fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	case DMX_IOCTL_STM_SETUID: {
		STM_PARAM_SET_UID rParam;

		mm_memset(&rParam, 0, sizeof(rParam));

		if (0 != copy_from_user((void *)&rParam, (void *)arg, sizeof(STM_PARAM_SET_UID))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_SETUID fail in copy_from_user")
				TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = StreamSetUID(rParam.pvStmHdl, rParam.u4Uid);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_SETUID fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	case DMX_IOCTL_STM_GETAU: {
		DMX_STM_MANAGE_AU_T rParam;
		void *pvStm = NULL;
		ESM_IO_BUF_INFO *prESMIOBuf = NULL;
		DMX_STM_INST_T *prStm = NULL;

		memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg, sizeof(DMX_STM_MANAGE_AU_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_GETAU fail in copy_from_user")
				TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_PARAM_WRONG;
			DMX_SetMWErrCode(mrRet);
			return FALSE;
		}
		pvStm = rParam.pvStmHdl;
		
		prStm = (DMX_STM_INST_T *)pvStm;
		if (NULL == prStm) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_GETAU fail for invalid arg")
				TEXT(" -- NULL == arg\r\n"));
			DMX_SetMWErrCode(mrRet);
			return FALSE;
		}

		prESMIOBuf = GAU_Get_GEsmIOBuf(prStm->u4GAUHandle);
		if (NULL == prESMIOBuf) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_GETAU fail GAU_EsmIOBuf(0x%x)\r\n"),
				(u32)prStm->u4GAUHandle);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			DMX_SetMWErrCode(mrRet);
			return FALSE;
		}

		if (0 != copy_from_user(prESMIOBuf, rParam.prEsmParam, sizeof(ESM_IO_BUF_INFO))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_GETAU access_ok return false -- prEsmParam:0x%p\r\n"),
				rParam.prEsmParam);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			DMX_SetMWErrCode(mrRet);
			return FALSE;
		}

		mrRet = GAU_GetAU(prStm->u4GAUHandle, (void *)prESMIOBuf);
		if (0 != copy_to_user(rParam.prEsmParam, prESMIOBuf, sizeof(ESM_IO_BUF_INFO)))	{
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_GETAU copy_to_user return false -- prEsmParam:0x%p\r\n"),
				rParam.prEsmParam);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			DMX_SetMWErrCode(mrRet);
			return FALSE;
		}

		if (0 != copy_to_user((void *)arg, (void *)&rParam, sizeof(DMX_STM_MANAGE_AU_T)))	{
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_GETAU to_user return false -- prEsmParam:0x%p\r\n"),
				rParam.prEsmParam);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			DMX_SetMWErrCode(mrRet);
			return FALSE;
		}

		if (DMX_FAILED(mrRet)) {
			DMX_SetMWErrCode(mrRet);
			return FALSE;
		}

		break;
	}

	case DMX_IOCTL_STM_RELEASEAU: {
		DMX_STM_MANAGE_AU_T rParam;
		void *pvStm = NULL;
		DMX_STM_INST_T *prStm = NULL;

		memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg, sizeof(DMX_STM_MANAGE_AU_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_RELEASEAU fail in copy_from_user")
				TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		pvStm = rParam.pvStmHdl;
		prStm = (DMX_STM_INST_T *)pvStm;
		if (NULL == prStm) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_RELEASEAU fail for invalid arg")
				TEXT(" -- NULL == arg\r\n"));
			DMX_SetMWErrCode(mrRet);
			return FALSE;
		}

		{
			ESM_IO_BUF_INFO *prESMIOBuf = NULL;

			prESMIOBuf = GAU_Get_REsmIOBuf((u32)prStm->u4GAUHandle);
			if (NULL == prESMIOBuf) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("DMX_IOCTL_STM_RELEASEAU fail GAU_EsmIOBuf(0x%x)\r\n"),
					(u32)prStm->u4GAUHandle);
				mrRet = RET_DMX_OS_OPERA_FAIL;
				DMX_SetMWErrCode(mrRet);
				return FALSE;
			}

			if (0 != copy_from_user(prESMIOBuf, rParam.prEsmParam, sizeof(ESM_IO_BUF_INFO))) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("DMX_IOCTL_STM_RELEASEAU access_ok return false")
					TEXT("-- prEsmParam:0x%p\r\n"),
					rParam.prEsmParam);
				mrRet = RET_DMX_OS_OPERA_FAIL;
				DMX_SetMWErrCode(mrRet);
				return FALSE;
			}
			mrRet = GAU_ReleaseAU((u32)prStm->u4GAUHandle, (void *)prESMIOBuf, FALSE);
		}

		if (DMX_FAILED(mrRet)) {
			DMX_SetMWErrCode(mrRet);
			return FALSE;
		}

		break;
	}

	case DMX_IOCTL_STM_RELEASE_FF_AUDIOAU: {
		DMX_STM_MANAGE_AU_T rParam;
		void *pvStm = NULL;
		DMX_STM_INST_T *prStm = NULL;

		memset(&rParam, 0, sizeof(rParam));
		if (0 !=  copy_from_user((void *)&rParam, (void *)arg, sizeof(DMX_STM_MANAGE_AU_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_RELEASE_FF_AUDIOAU access_ok")
				TEXT(" return false -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			DMX_SetMWErrCode(mrRet);
			return FALSE;
		}

		pvStm = rParam.pvStmHdl;
		prStm = (DMX_STM_INST_T *)pvStm;
		if (NULL == prStm) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_RELEASE_FF_AUDIOAU fail ")
				TEXT("for prStm is NULL\r\n"));
			mrRet = RET_DMX_PARAM_WRONG;
			DMX_SetMWErrCode(mrRet);
			return FALSE;
		}

		{
			ESM_IO_BUF_INFO *prESMIOBuf = NULL;

			if (0 !=  copy_from_user(prESMIOBuf, rParam.prEsmParam, sizeof(ESM_IO_BUF_INFO))) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
					TEXT("DMX_IOCTL_STM_RELEASE_FF_AUDIOAU ")
				TEXT("access_ok return false -- prEsmParam:0x%p\r\n"),
					rParam.prEsmParam);
				mrRet = RET_DMX_OS_OPERA_FAIL;
				DMX_SetMWErrCode(mrRet);
				return FALSE;
			}
			mrRet = GAU_ReleaseAU((u32)prStm->u4GAUHandle, (void *)prESMIOBuf, TRUE);
		}

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_STM_RELEASE_FF_AUDIOAU fail,")
					TEXT(" mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	/* For Parser relative control function. */
	case DMX_IOCTL_PSR_VFIFO_USAGE: {
		DMX_PSR_FIFO_USAGE_T rParam;
		void *pvSptHdl = NULL;
		u32 u4VFifo = 0;

		memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg, sizeof(DMX_PSR_FIFO_USAGE_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PSR_VFIFO_USAGE fail in copy_from_user")
					TEXT(" -- arg:0x%x\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}
		pvSptHdl = rParam.pvSptHdl;
		mrRet = dmx_stminst_getfifofullness(dwOpenContext, pvSptHdl,
			SPT_DATA_V, &u4VFifo);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PSR_VFIFO_USAGE fail, mrRet: 0x%x\r\n"), mrRet);
			break;
		}
		rParam.u4Fifo = u4VFifo;
		
		if (0 != copy_to_user((void *)arg, (void *)&rParam, sizeof(DMX_PSR_FIFO_USAGE_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PSR_VFIFO_USAGE fail in copy_to_user")
				TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		break;
	}

	case DMX_IOCTL_PSR_AFIFO_USAGE: {
		DMX_PSR_FIFO_USAGE_T rParam;
		void *pvSptHdl = NULL;
		u32 u4AFifo = 0;

		memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg, sizeof(DMX_PSR_FIFO_USAGE_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PSR_AFIFO_USAGE fail in copy_from_user")
					TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}
		pvSptHdl = rParam.pvSptHdl;
		mrRet = dmx_stminst_getfifofullness(dwOpenContext, (HANDLE)pvSptHdl, SPT_DATA_A, &u4AFifo);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PSR_AFIFO_USAGE fail, mrRet: 0x%x\r\n"), mrRet);
			break;
		}
		rParam.u4Fifo = u4AFifo;
		
		if (0 != copy_to_user((void *)arg, (void *)&rParam, sizeof(DMX_PSR_FIFO_USAGE_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PSR_AFIFO_USAGE fail in copy_to_user")
					TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		break;
	}

	case DMX_IOCTL_PSR_ON: {
		DMX_PSR_ON_PARAM_T rParam;

		mm_memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg, sizeof(DMX_PSR_ON_PARAM_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PSR_ON access_ok return false")
				TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = dmx_parse_on(dwOpenContext, &rParam);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PSR_ON fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	case DMX_IOCTL_PSR_OFF: {
		void *pvSptHdl = NULL;

		if (0 != copy_from_user((void *)&pvSptHdl, (void *)arg, sizeof(void *))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PSR_OFF fail in copy_from_user")
					TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = dmx_parse_off(dwOpenContext, (HANDLE)pvSptHdl);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PSR_OFF fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	case DMX_IOCTL_PSR_PAUSE: {
		void *pvSptHdl = NULL;

		if (0 != copy_from_user((void *)&pvSptHdl, (void *)arg, sizeof(void *))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PSR_PAUSE fail in copy_from_user")
					TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = dmx_parse_pause(dwOpenContext, (HANDLE)pvSptHdl);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PSR_PAUSE fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	case DMX_IOCTL_PSR_RESUME:
		break;

	case DMX_IOCTL_PSR_FILE_OFST: {
		DMX_PSR_FILE_OFST_T rParam;
		void *pvSptHdl = NULL;
		u64 u8FileOfst = 0;
		
		if (0 != copy_from_user((void *)&rParam, (void *)arg, sizeof(DMX_PSR_FILE_OFST_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PSR_FILE_OFST fail in copy_from_user")
					TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}
		pvSptHdl = rParam.pvSptHdl;
		
		mrRet = dmx_sptinst_getpsrfileofst(dwOpenContext, pvSptHdl, &u8FileOfst);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PSR_FILE_OFST fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}
		rParam.u8FileOfst = u8FileOfst;
		
		if (0 != copy_to_user((void *)arg, (void *)&rParam, sizeof(DMX_PSR_FILE_OFST_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PSR_FILE_OFST fail in copy_to_user")
					TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		break;
	}

	/* For RESPLITTER relative control function. */
	case DMX_IOCTL_RSP_ON: {
		SPLITTER_PTX_RSP_ON_INFO_T rParam;

		mm_memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg,
			sizeof(SPLITTER_PTX_RSP_ON_INFO_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_RSP_ON access_ok return false")
				TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}
		mrRet = dmx_rsp_on(dwOpenContext, &rParam);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_RSP_ON fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	case DMX_IOCTL_RSP_OFF: {
		SPLITTER_PTX_RSP_OFF_INFO_T rParam;

		mm_memset(&rParam, 0, sizeof(rParam));

		if (0 != copy_from_user((void *)&rParam, (void *)arg, sizeof(SPLITTER_PTX_RSP_OFF_INFO_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_RSP_OFF access_ok return false")
				TEXT(" -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = dmx_rsp_off(dwOpenContext, &rParam);
		if (0 != copy_to_user((void *)arg, (void *)&rParam, sizeof(SPLITTER_PTX_RSP_OFF_INFO_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_RSP_OFF fail in copy_to_user")
				TEXT("  -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_RSP_OFF fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	case DMX_IOCTL_REBUFFER_RANGE: {
		SPLITTER_PTX_REBUFFER_RANGE_INFO_T rParam;

		mm_memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg,
			sizeof(SPLITTER_PTX_REBUFFER_RANGE_INFO_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_REBUFFER_RANGE access_ok return ")
				TEXT("false -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = dmx_rsp_rebuf(dwOpenContext, &rParam);
		if (0 != copy_to_user((void *)arg, (void *)&rParam,
			sizeof(SPLITTER_PTX_REBUFFER_RANGE_INFO_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_REBUFFER_RANGE fail in mm_copy_to_user")
				TEXT("  -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_REBUFFER_RANGE fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	/* For CFA relative control function. */
	case DMX_IOCTL_CFA_SET_TYPE: {
		CFA_PARAM_SET_TYPE rParam;

		mm_memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg,
			sizeof(CFA_PARAM_SET_TYPE))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_CFA_SET_TYPE access_ok return ")
				TEXT("false -- arg:0x%p\r\n"), arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}
		mrRet = dmx_cfa_settype(dwOpenContext, &rParam);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_CFA_SET_TYPE fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	case DMX_IOCTL_CFA_CONFIG: {
		CFA_PARAM_SET_CONFIG rParam;
		CFA_PARAM_SET_CONFIG *prInParam = NULL;

		prInParam = (CFA_PARAM_SET_CONFIG *)arg;
		
		mm_memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user(&rParam, prInParam,
			sizeof(CFA_PARAM_SET_CONFIG))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_CFA_CONFIG access_ok return ")
				TEXT("false -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = dmx_cfa_config(dwOpenContext, rParam.pvSptHdl,
			rParam.pvConfig, fgIsUserMem);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_CFA_CONFIG fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	case DMX_IOCTL_CFA_SET_RANGE: {
		CFA_PARAM_SET_RANGE rParam;

		mm_memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg,
			sizeof(CFA_PARAM_SET_RANGE))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_CFA_SET_RANGE fail in ")
				TEXT("copy_from_user-- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = dmx_cfa_setrange(dwOpenContext, rParam.pvSptHdl,
			rParam.pvRange, fgIsUserMem);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_CFA_SET_RANGE fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}
#if 0
	case DMX_IOCTL_CFA_SET_INQUIRE_TYPE: {
		CFA_PARAM_SET_INQ_TYPE rParam;

		mm_memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg,
			sizeof(CFA_PARAM_SET_INQ_TYPE))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_CFA_SET_INQUIRE_TYPE fail")
				TEXT(" in copy_from_user -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}
		mrRet = dmx_cfa_setinquiretype(dwOpenContext, rParam.pvSptHdl, rParam.u4CfaQID);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_CFA_SET_INQUIRE_TYPE fail, mrRet: 0x%x\r\n"),
				mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}
#endif
	case DMX_IOCTL_CFA_SET_GEN: {
		CFA_PARAM_SET_INFO rParam;

		mm_memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg,
			sizeof(CFA_PARAM_SET_INFO))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_CFA_SET_GEN fail in ")
				TEXT("copy_from_user -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = dmx_cfa_setgeneral(dwOpenContext, rParam.pvSptHdl,
			rParam.u4CfaQID, rParam.pvCfaParam, rParam.u4ParamSize);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_CFA_SET_GEN fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	case DMX_IOCTL_CFA_GET_POSI: {
		DMX_PSR_FILE_OFST_T rParam;
		void *pvSptHdl = NULL;
		u64 u8FileOfst = 0;

		memset(&rParam, 0, sizeof(rParam));

		if (0 != copy_from_user((void *)&rParam, (void *)arg, sizeof(DMX_PSR_FILE_OFST_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_CFA_GET_POSI fail in ")
				TEXT("copy_from_user -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		pvSptHdl = rParam.pvSptHdl;
		mrRet = dmx_cfa_getpsrpos(dwOpenContext,
			(HANDLE)pvSptHdl, &u8FileOfst);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_CFA_GET_POSI fail, mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
			break;
		}
		rParam.u8FileOfst = u8FileOfst;
		if (0 != copy_to_user((void *)arg, (void *)&rParam, sizeof(DMX_PSR_FILE_OFST_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_CFA_GET_POSI fail in ")
					TEXT("copy_to_user -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		break;
	}
	/* For PBBUF relative control function. */
	case DMX_IOCTL_PBBUF_ENABLE:
	case DMX_IOCTL_PBBUF_DISABLE:
		break;

	case DMX_IOCTL_PBBUF_ALLOC_BUF: {
		PBBUF_PARAM_SEND_BUF rParam;
		SEND_BUFFER rSendBuffer;

		mm_memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg, sizeof(PBBUF_PARAM_SEND_BUF))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PBBUF_ALLOC_BUF fail in ")
				TEXT("copy_from_user -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}
		mm_memset(&rSendBuffer, 0, sizeof(rSendBuffer));

		if (NULL == rParam.prBUF) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PBBUF_ALLOC_BUF fail for ")
				TEXT("SendBuffer's prBuf == NULL\r\n"));
			mrRet = RET_DMX_PARAM_WRONG;
			break;
		}

		if (0 != copy_from_user(&rSendBuffer, rParam.prBUF,
			sizeof(SEND_BUFFER))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PBBUF_ALLOC_BUF fail in ")
				TEXT("copy_from_user -- prBUF:0x%p, sizeof(SEND_BUFFER):0x%p\r\n"),
				rParam.prBUF, sizeof(SEND_BUFFER));
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}
		mrRet = dmx_pbbuf_allocbuf(dwOpenContext, rParam.pvSptHdl,
			&rSendBuffer);
		if (0 != copy_to_user(rParam.prBUF, &rSendBuffer,
			sizeof(SEND_BUFFER))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PBBUF_ALLOC_BUF fail in ")
				TEXT("copy_to_user  -- prBUF:0x%p, sizeof(SEND_BUFFER):%d\r\n"),
				rParam.prBUF, sizeof(SEND_BUFFER));
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		if (0 != copy_to_user((void *)arg, (void *)&rParam,sizeof(PBBUF_PARAM_SEND_BUF))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PBBUF_ALLOC_BUF fail in ")
				TEXT("copy_to_user  -- prBUF:0x%p, sizeof(SEND_BUFFER):0x%p\r\n"),
				rParam.prBUF, sizeof(SEND_BUFFER));
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		if (DMX_FAILED(mrRet)) {
			DMX_SetMWErrCode(mrRet);
			return FALSE;
		}

		break;
	}

	case DMX_IOCTL_PBBUF_CANCEL_BUF: {
		void *pvSptHdl = NULL;

		if (0 != copy_from_user((void *)&pvSptHdl, (void *)arg, sizeof(void *))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_CFA_GET_POSI fail in ")
					TEXT("copy_from_user -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = dmx_pbbuf_cancelalloc(dwOpenContext, pvSptHdl);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PBBUF_CANCEL_BUF fail, ")
					TEXT("mrRet: 0x%x\r\n"), mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	case DMX_IOCTL_PBBUF_SEND_BUF: {
		DMX_PBBUF_SEND_BUF_T rParam;
		PBBUF_PARAM_SEND_BUF rBufParam;
		SEND_BUFFER rSendBuffer;
		bool fgSentExit = FALSE;

		mm_memset(&rParam, 0, sizeof(rParam));
		mm_memset(&rBufParam, 0, sizeof(rBufParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg,
			sizeof(DMX_PBBUF_SEND_BUF_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PBBUF_SEND_BUF access_ok ")
				TEXT("return false -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		rBufParam = rParam.rBufParam;
		mm_memset(&rSendBuffer, 0, sizeof(rSendBuffer));

		if (NULL == rBufParam.prBUF) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PBBUF_SEND_BUF fail for ")
				TEXT("SendBuffer's prBuf == NULL\r\n"));
			mrRet = RET_DMX_PARAM_WRONG;
		}

		if (0 != copy_from_user(&rSendBuffer, rBufParam.prBUF,
			sizeof(SEND_BUFFER))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PBBUF_SEND_BUF fail in ")
				TEXT("copy_from_user -- prBUF:0x%p, sizeof(SEND_BUFFER):%d\r\n"),
				rBufParam.prBUF, sizeof(SEND_BUFFER));
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = dmx_pbbuf_sendbuf(dwOpenContext, rBufParam.pvSptHdl,
			&rSendBuffer, &fgSentExit);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PBBUF_SEND_BUF fail, mrRet: 0x%x\r\n"),
				mrRet);
			DMX_ASSERT(FALSE);
		}
		
		rParam.fgExitSent = fgSentExit;

		if (0 != copy_to_user((void *)arg, (void *)&rParam, sizeof(DMX_PBBUF_SEND_BUF_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PBBUF_SEND_BUF fail in ")
				TEXT("copy_to_user  -- rParam:0x%p\r\n"),rParam);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}
		
		break;
	}

	case DMX_IOCTL_PBBUF_RELEASE_BUF: {
		PBBUF_PARAM_SEND_BUF rParam;
		SEND_BUFFER rSendBuffer;

		mm_memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg,
			sizeof(PBBUF_PARAM_SEND_BUF))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PBBUF_RELEASE_BUF fail ")
				TEXT("in copy_from_user -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mm_memset(&rSendBuffer, 0, sizeof(rSendBuffer));

		if (NULL == rParam.prBUF) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PBBUF_RELEASE_BUF fail ")
				TEXT("for SendBuffer's prBuf == NULL\r\n"));
			mrRet = RET_DMX_PARAM_WRONG;
		}
		if (0 != copy_from_user(&rSendBuffer, rParam.prBUF,
			sizeof(SEND_BUFFER))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PBBUF_RELEASE_BUF fail ")
				TEXT("in copy_from_user -- prBUF:0x%p, sizeof(SEND_BUFFER):0x%p\r\n"),
				rParam.prBUF, sizeof(SEND_BUFFER));
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = dmx_pbbuf_releasebuf(dwOpenContext, rParam.pvSptHdl, &rSendBuffer);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PBBUF_RELEASE_BUF fail, mrRet: 0x%x\r\n"),
				mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	case DMX_IOCTL_PBBUF_CLEAN_BUF:
		break;

	case DMX_IOCTL_PBBUF_NODATA: {
		DMX_PBBUF_NODATA_PARAM_T rParam;

		mm_memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg,
			sizeof(DMX_PBBUF_NODATA_PARAM_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PBBUF_NODATA fail in ")
				TEXT("mm_copy_from_user -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("+++++++++++++++++ DMX_IOCTL_PBBUF_NODATA")
				TEXT(" (pvSptHdl: 0x%p, Status: %d, u8FileEndOffset:0x%llx.) +++++++++++++\r\n"),
			rParam.pvSptHdl, rParam.u4Status, rParam.u8FileEndOffset);
		dmx_pbbuf_infonodata(dwOpenContext, &rParam);


		mrRet = RET_DMX_OK;

		break;
	}

	case DMX_IOCTL_DECRYPT_CREATE_INST: {
		E_DECRYPT_TYPE_T eDecryptType = DECRYPT_NONE;
		void *pBufOut = NULL;

		if (0 != copy_from_user((void *)&eDecryptType, (void *)arg,
			sizeof(E_DECRYPT_TYPE_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_DECRYPT_CREATE_INST fail in ")
					TEXT("copy_from_user -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = DmxDecryptCreateInst(eDecryptType, &pBufOut);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_DECRYPT_CREATE_INST fail in ")
					TEXT("DmxDecryptCreateInst, mrRet: 0x%x\r\n"), mrRet);
			DMX_SetMWErrCode(mrRet);
			return FALSE;
		}

		break;
	}

	case DMX_IOCTL_DECRYPT_RELEASE_INST: {
		DECRYPT_INST_PARAM_T rParam;

		mm_memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg, sizeof(DECRYPT_INST_PARAM_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PBBUF_NODATA fail in ")
				TEXT("copy_from_user -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}
		mrRet = DmxDecryptReleaseInst(rParam.eDecryptType, rParam.pvInst);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_DECRYPT_RELEASE_INST fail in ")
					TEXT("DmxDecryptReleaseInst, mrRet: 0x%x\r\n"), mrRet);
			DMX_SetMWErrCode(mrRet);
			return FALSE;
		}

		break;
	}

	case DMX_IOCTL_DECRYPT_EXEC_CMD: {
		DECRYPT_OPER_PARAM_T rParam;
		void *pBufOut = NULL;
		u32 u4LenOut = 0;

		mm_memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg, sizeof(DECRYPT_OPER_PARAM_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_DECRYPT_EXEC_CMD fail in ")
				TEXT("copy_from_user -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}
		mrRet = DmxDecryptExecCmd(
			rParam.pvInst,
			rParam.u4OperCode,
			rParam.pvOperParam,
			rParam.u4OperParamSz,
			pBufOut, u4LenOut);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_DECRYPT_EXEC_CMD fail in ")
				TEXT("DmxDecryptExecCmd, mrRet: 0x%x\r\n"), mrRet);
			DMX_SetMWErrCode(mrRet);
			return FALSE;
		}

		break;
	}

	case DMX_IOCTL_DECRYPT_GET_LAST_ERROR: {
		DECRYPT_INST_PARAM_T rParam;
		MRESULT mrExtErr = RET_DMX_EXT_OK;
		MRESULT *pmrRet = NULL;

		mm_memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg,
			sizeof(DECRYPT_INST_PARAM_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_PBBUF_NODATA fail in ")
				TEXT("copy_from_user -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrExtErr = DmxDecryptGetInstLastError(rParam.eDecryptType, rParam.pvInst);

		*pmrRet = DMX_GetMWErrCode(mrExtErr);

		mrRet = RET_DMX_OK;

		break;
	}

	case DMX_IOCTL_DECRYPT_INIT_MEMORY: {
		E_DECRYPT_TYPE_T eDecryptType = DECRYPT_NONE;

		if (0 != copy_from_user((void *)&eDecryptType, (void *)arg, sizeof(E_DECRYPT_TYPE_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_DECRYPT_INIT_MEMORY fail in")
				TEXT(" copy_from_user -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = DmxDecryptInitMemory(eDecryptType);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_DECRYPT_INIT_MEMORY fail in ")
				TEXT("DmxDecryptInitMemory, mrRet: 0x%x\r\n"),
				mrRet);
			DMX_SetMWErrCode(mrRet);
			return FALSE;
		}
		break;
	}

	case DMX_IOCTL_DECRYPT_DEVICE_IS_ACTIVED: {
		DMX_CHECK_DECRYPT_DEVICE_T rParam;
		E_DECRYPT_TYPE_T eDecryptType = DECRYPT_NONE;
		bool fgActived = FALSE;

		memset(&rParam, 0, sizeof(rParam));
		if (0 != copy_from_user((void *)&rParam, (void *)arg,
			sizeof(DMX_CHECK_DECRYPT_DEVICE_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_DECRYPT_DEVICE_IS_ACTIVED fail in")
				TEXT(" copy_from_user -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}
		eDecryptType = rParam.eDecryptType;
		mrRet = DmxDecryptIsDevActived(eDecryptType, &fgActived);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_DECRYPT_DEVICE_IS_ACTIVED fail")
				TEXT(" in DmxDecryptIsDevActived, mrRet: 0x%x\r\n"),
				mrRet);
			DMX_SetMWErrCode(mrRet);
			return FALSE;
		}
		rParam.fgActived = fgActived;
		if (0 != copy_to_user((void *)arg, (void *)&rParam, sizeof(DMX_CHECK_DECRYPT_DEVICE_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_DECRYPT_DEVICE_IS_ACTIVED fail in")
				TEXT(" copy_to_user -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		break;
	}

	case DMX_IOCTL_DECRYPT_DEVICE_GET_ACT_STATUS: {
		E_DECRYPT_TYPE_T eDecryptType = DECRYPT_NONE;
		void *pBufOut = NULL;
		u32 u4LenOut = 0;

		if (0 != copy_from_user((void *)&eDecryptType, (void *)arg, sizeof(E_DECRYPT_TYPE_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_DECRYPT_DEVICE_IS_ACTIVED fail in")
				TEXT(" copy_from_user -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = DmxDecryptGetActStatus(eDecryptType, pBufOut, u4LenOut);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_DECRYPT_DEVICE_GET_ACT_STATUS fail in")
				TEXT(" DmxDecryptGetActStatus, mrRet: 0x%x\r\n"), mrRet);
			DMX_SetMWErrCode(mrRet);
			return FALSE;
		}
		break;
	}

	case DMX_IOCTL_DECRYPT_DEVICE_GET_VERSION: {
		E_DECRYPT_TYPE_T eDecryptType = DECRYPT_NONE;
		void *pBufOut = NULL;
		u32 u4LenOut = 0;
		
		if (0 != copy_from_user((void *)&eDecryptType, (void *)arg,
			sizeof(E_DECRYPT_TYPE_T))) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_DECRYPT_DEVICE_IS_ACTIVED fail in")
				TEXT(" mm_copy_from_user -- arg:0x%p\r\n"),arg);
			mrRet = RET_DMX_OS_OPERA_FAIL;
			break;
		}

		mrRet = DmxDecryptGetVersion(eDecryptType, pBufOut, u4LenOut);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_DECRYPT_DEVICE_GET_VERSION fail in")
				TEXT(" DmxDecryptGetVersion, mrRet: 0x%x\r\n"), mrRet);
			DMX_SetMWErrCode(mrRet);
			return FALSE;
		}
		break;
	}

	case DMX_IOCTL_SET_CLI_CMD_INFO: {
		DMX_CLI_CFG *prCliCfg = (DMX_CLI_CFG *)arg;

		mrRet = SplitterHandleCliCmd(prCliCfg);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("DMX_IOCTL_SET_CLI_CMD_INFO fail, mrRet: 0x%x\r\n"),
				mrRet);
			DMX_ASSERT(FALSE);
		}

		break;
	}

	default: {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("DMX_IOControl Fail for Unsupport dwCode = 0x%x\r\n"), cmd);
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	}

	if (DMX_SUCCEED(mrRet))
		return TRUE;

	DMX_SetMWErrCode(mrRet);
	DMX_ASSERT(FALSE);
	return FALSE;
}


