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
 * @file dmx_inst.c
 *
 *
 * @par Project
 *
 * @par Description
 *
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */
#ifdef __linux__
#include <linux/mm.h>
#include <linux/errno.h>
#include "windows.h"
#include <media/atc/dmx_define.h>
#include <media/atc/x_dmx.h>
#include <media/atc/ioctl_dmx.h>
#include <media/atc/perf_timer.h>
/* #include <media/atc/mm_debug.h> */
#include "winutil.h"
#else
#include "dmx_define.h"
#include "dmx_splitter.h"
#include "dmx_cfa_def.h"
#include "x_dmx.h"
#include "ioctl_dmx.h"
#include "perf_timer.h"
#include "mm_debug.h"
#endif /* __linux__*/

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_cli.h"
#include "dmx_dump.h"
#include "dmx_spt.h"
#include "dmx_esm.h"
#include "dmx_sema.h"
#include "dmx_pbbuf.h"
#include "dmx_spt_os.h"
#include "dmx_spt_main.h"
#include "dmx_spt_rsp.h"
#include "dmx_spt_util.h"
#include "dmx_parser.h"
#include "dmx_psr_esm.h"
#include "dmx_spt_if.h"
#include "dmx_esm_if.h"
#include "dmx_gau_if.h"
#include "dmx_stream.h"
#include "dmx_psr_cc.h"
#include "dmx_psr_pbbuf.h"
#include "dmx_pfm.h"
#include "dmx_inst.h"
#include "dmx_cpsa.h"
#include "cfa_if.h"

#ifndef __linux__
/*disable warning C4127: conditional expression is constant*/
#pragma warning(disable : 4127)
#endif

EXTERN BOOL g_fgDmxDmaTwice;
EXTERN s32 g_Spt_EnableCnt;
EXTERN s32 g_i4DmxOpenRefCnt;

HANDLE	g_rPsrHalStructlock = NULL;


DMX_INST_LIST_T			g_rDmxInstList;
DMX_DUMP_MAN_T			g_rDmxDumpMan;
DMX_CLI_MAN_T			g_rDmxCliMan;
DMX_SPT_MAN_INFO_T		g_rSptMan;
DMX_PSR_MAN_INFO_T		g_rPsrMan;
DMX_STM_MAN_INFO_T		g_rDmxStmMan;
bool				g_fgESMInit = FALSE;
HANDLE				g_hESMSema = NULL;
DMX_ESM_INST_T			g_arESMInst[MAX_ESICOUNT];
bool				g_fgPbBufInit  = FALSE;
PBBUF				*g_aprPbBuf[DMX_MAX_PBBUF_INST_CNT];
GAU_MANAMENT_INFO_T		g_rDmxGauManager;
bool				g_fgPSRHalInit = FALSE;
PSR_STRUCT_T			g_rPSRHalStruct[DMX_DEV_CNT];
AU_AUDIO			g_arLogAudioAUs[DMX_MAX_LOG_AUDIO_AU_CNT];
u32				g_u4PbBufFlag;
bool			g_fgDmxInit;

void DmxGlobalParamsInit(void)
{
	mm_memset((void *)(&g_rDmxInstList), 0, sizeof(g_rDmxInstList));
	smp_mb();
	mm_memset((void *)(&g_rDmxCliMan),	0, sizeof(g_rDmxCliMan));
	g_rDmxCliMan.u4CheckStart = DMX_MEMCHECK_START_VAL;
	g_rDmxCliMan.u4CheckEnd = DMX_MEMCHECK_END_VAL;
	mm_memset((void *)(&g_rSptMan),	0, sizeof(g_rSptMan));
	mm_memset((void *)(&g_rDmxDumpMan), 0, sizeof(g_rDmxDumpMan));
	mm_memset((void *)(&g_rPsrMan), 0, sizeof(g_rPsrMan));
	mm_memset((void *)(&g_rDmxStmMan), 0, sizeof(g_rDmxStmMan));
	smp_mb();
	/* ESM*/
	g_fgESMInit = FALSE;
	g_hESMSema = NULL;
	mm_memset((void *)(g_arESMInst),  0, sizeof(DMX_ESM_INST_T) * MAX_ESICOUNT);
	/* PBBUF*/
	g_fgPbBufInit  = FALSE;
	g_fgDmxInit = FALSE;
	mm_memset((void *)(g_aprPbBuf), 0, DMX_MAX_PBBUF_INST_CNT * sizeof(PBBUF *));
	/* GAU*/
	mm_memset((void *)(&g_rDmxGauManager), 0, sizeof(g_rDmxGauManager));

	mm_memset(g_rPSRHalStruct, 0, sizeof(PSR_STRUCT_T) * DMX_DEV_CNT);

	mm_memset(g_arLogAudioAUs, 0, sizeof(AU_AUDIO) * DMX_MAX_LOG_AUDIO_AU_CNT);

	g_i4DmxOpenRefCnt = 0;
	g_Spt_EnableCnt = 0;
	g_u4PbBufFlag = 0;
	g_fgDmxDmaTwice = FALSE;

	smp_mb();

#if DMX_PFM_TEST
	DmxPfmInit();
#endif /* DMX_PFM_TEST*/
}

MRESULT DmxInstListCreateSema(void)
{
	MRESULT  mrRet = RET_DMX_OK;

	if (NULL != g_rDmxInstList.hLock) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[SPT] %s fail for sema already exist\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_ALREADY_EXIST);
	}

	mrRet = dmx_sema_create(&(g_rDmxInstList.hLock),
		DMX_SEMA_TYPE_BINARY, DMX_SEMA_STATE_UNLOCK);

	if (DMX_SUCCEED(mrRet))
		MM_RETURN(RET_DMX_OK);


	DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[SPT] %s fail in dmx_sema_create, i4Ret: 0x%x\r\n"),
		DMX_FUNC_NAME, mrRet);

	MM_RETURN(mrRet);
}


MRESULT DmxInstListDeleteSema(void)
{
	MRESULT  mrRet = RET_DMX_OK;

	if (NULL != g_rDmxInstList.hLock) {
		mrRet = dmx_sema_delete(g_rDmxInstList.hLock);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
				TEXT("[SPT] %s fail in dmx_sema_delete, i4Ret: 0x%x\r\n"),
				DMX_FUNC_NAME, mrRet);
			MM_RETURN(mrRet);
		}

		g_rDmxInstList.hLock = NULL;
	}

	MM_RETURN(RET_DMX_OK);
}


MRESULT DmxInstListLockSema(void)
{
	MRESULT  mrRet = RET_DMX_OK;

	mrRet = dmx_sema_lock(g_rDmxInstList.hLock, DMX_SEMA_OPTION_WAIT);
	if (DMX_SUCCEED(mrRet))
		MM_RETURN(RET_DMX_OK);


	DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[SPT] %s fail in dmx_sema_lock, i4Ret: 0x%x\r\n"),
		DMX_FUNC_NAME, mrRet);

	MM_RETURN(mrRet);
}


MRESULT DmxInstListReleaseSema(void)
{
	MRESULT  mrRet = RET_DMX_OK;

	mrRet = dmx_sema_unlock(g_rDmxInstList.hLock);
	if (DMX_SUCCEED(mrRet))
		MM_RETURN(RET_DMX_OK);


	DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[SPT] %s fail in dmx_sema_unlock, i4Ret: 0x%x\r\n"),
		DMX_FUNC_NAME, mrRet);

	MM_RETURN(mrRet);
}

/**************************************************************************/
/* DmxInit*****************************************************************/
/* Initialize DMX Driver, it does the following works:*********************/
/* 1. Iniitalize DMX Memory Management*************************************/
/* 2. Init Parser CC, Parser Filter, Parser HWRes Queue, PVR***************/
/* 3. Initial Stream, it is For SPT_DATA_BUF Stream************************/
/* 4. Init PBBuf***********************************************************/
/* 5. Init ESM*************************************************************/
/* 6. Init Dummy Decoder***************************************************/
/* 7. Init splitter********************************************************/
/**************************************************************************/
MRESULT DmxInit(void)
{
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter.\r\n"), DMX_FUNC_NAME);

	DmxGlobalParamsInit();

	/* Iniitalize DMX Memory Management*/
	dmx_Mem_Init();

	mrRet = DmxInstListCreateSema();
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in DmxInstListCreateSema, mrRet: 0x%lx!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		goto DMXINITERR;
	}

	SPLITTER_LOCK_INIT(mrRet);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SPLITTER_LOCK_INIT, mrRet: 0x%lx!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		goto DMXINITERR;
	}

	DmxDumpInit();

	mrRet = DmxInitDecrypt();
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in DmxInitDecrypt(), mrRet: 0x%lx\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		goto DMXINITERR;
	}

	smp_mb();


	/* Per Victor_Lin's Request, initial Parser First */

	/* Init Parser CC, Parser Filter, Parser HWRes Queue, PVR */
	mrRet = ParserInit();
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in ParserInit, mrRet: 0x%x!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		goto DMXINITERR;
	}
	DmxLogD(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s line %d -- ParserInit success.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	/* Initial Stream, it is For SPT_DATA_BUF Stream */
	mrRet = StreamInit();
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in StreamInit, mrRet: 0x%x!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		goto DMXINITERR;
	}
	DmxLogD(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s line %d -- StreamInit success.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	/* Init PBBuf */
	mrRet = PBBUF_Init();
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in PBBUF_Init, mrRet: 0x%x!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		goto DMXINITERR;
	}
	DmxLogD(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s line %d -- PBBUF_Init success.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	/* Init ESM */
	mrRet = ESM_Init();
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in ESM_Init, mrRet: 0x%x!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		goto DMXINITERR;
	}
	DmxLogD(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s line %d -- ESM_Init success.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	/* Init [GAU]. */
	GAU_Init();
	DmxLogD(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s line %d -- GAU_Init success.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	/* Init splitter */
	mrRet = SptInit();
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SptInit, mrRet: 0x%x!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		goto DMXINITERR;
	}
	DmxLogD(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s line %d -- SptInit success.\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	g_fgDmxInit = TRUE;

	smp_mb();

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s Success.\r\n"),
		DMX_FUNC_NAME);

	MM_RETURN(mrRet);

DMXINITERR:

	SptUninit();

	GAU_Uninit();

	ESM_Uninit();

	PBBUF_UnInit();

	StreamUninit();

	ParserUninit();

	DmxDeInitDecrypt();

	DmxDumpDeInit();

	DmxCliDeInit();

	SPLITTER_LOCK_UNINIT(mrRet);

	DmxInstListDeleteSema();

	dmx_Mem_Uninit();

	DmxGlobalParamsInit();

	g_fgDmxInit = FALSE;

	MM_RETURN(mrRet);
}

/******************************************************************************/
/* DmxUninit*******************************************************************/
/* Deinitialize DMX Driver, it does the following works:***********************/
/* 1. Uninit Parser, it does:  DeInitialize Parser CC, Parser Filter, Parser***/
/*	  HWRes Qeue, Initial PVR**************************************************/
/* 2. UnInitial Stream*********************************************************/
/* 3. Uninit PBBuf*************************************************************/
/* 4. Uninit GAU.**************************************************************/
/* 5. Uninit splitter**********************************************************/
/* 6. Uninit DMX Memory Management*********************************************/
/******************************************************************************/
MRESULT DmxUninit(void)
{
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter\r\n"), DMX_FUNC_NAME);

	if (g_fgDmxInit) {
		g_fgDmxInit = FALSE;
	} else {
		DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d success\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OK);
	}

	/* Uninit Parser */
	/* it does:DeInitialize Parser CC, Parser Filter, Parser HWRes Qeue, Initial PVR*/
	mrRet = ParserUninit();
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in ParserUninit, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
	} else {
		DmxLogD(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s -- ParserUninit success.\r\n"),
			DMX_FUNC_NAME);
	}

	/* Uninit Stream */
	mrRet = StreamUninit();
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in StreamUninit, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
	} else {
		DmxLogD(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s -- StreamUninit success.\r\n"),
			DMX_FUNC_NAME);
	}

	/* Uninit PBBuf */
	PBBUF_UnInit();
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in PBBUF_UnInit, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
	} else {
		DmxLogD(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s -- PBBUF_UnInit success.\r\n"),
			DMX_FUNC_NAME);
	}
	/* Uninit CFA */
	/* Uninit CFA while disable splitter.*/

	/* Uninit Dummy Decoder. */
	GAU_Uninit();
	DmxLogD(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s -- GAU_Uninit success.\r\n"),
		DMX_FUNC_NAME);

	/* Uninit ESM */
	ESM_Uninit();
	DmxLogD(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s -- ESM_Uninit success.\r\n"),
		DMX_FUNC_NAME);

	/* Uninit Splitter */
	mrRet = SptUninit();
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s fail in SptUninit, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, mrRet);
	} else {
		DmxLogD(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s -- SptUninit success.\r\n"),
			DMX_FUNC_NAME);
	}

	dmx_Mem_Uninit();

	DmxDumpCloseAllFile();

	DmxDumpDeInit();

	DmxCliDeInit();

	DmxDeInitDecrypt();

	SPLITTER_LOCK_UNINIT(mrRet);

	DmxInstListDeleteSema();

	DmxGlobalParamsInit();

	if (DMX_SUCCEED(mrRet)) {
		DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s exit, success.\r\n"),
			DMX_FUNC_NAME);
	} else {
		DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s exit, fail.\r\n"),
			DMX_FUNC_NAME);
	}

	MM_RETURN(mrRet);
}

MRESULT DmxInstCreateSema(DMX_INST_T *prDmxInst)
{
	MRESULT  mrRet = RET_DMX_OK;

	if (NULL == prDmxInst)
		MM_RETURN(RET_DMX_PARAM_WRONG);


	if (NULL != prDmxInst->hLock) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[SPT] %s fail for sema already exist, pvDmxInst: 0x%x\r\n"),
			DMX_FUNC_NAME, prDmxInst);
		MM_RETURN(RET_DMX_ALREADY_EXIST);
	}

	mrRet = dmx_sema_create(&(prDmxInst->hLock),
		DMX_SEMA_TYPE_BINARY, DMX_SEMA_STATE_UNLOCK);

	if (DMX_SUCCEED(mrRet))
		MM_RETURN(RET_DMX_OK);

	DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[SPT] %s fail in dmx_sema_create, pvDmxInst:0x%x, i4Ret: 0x%x\r\n"),
		DMX_FUNC_NAME, prDmxInst, mrRet);

	MM_RETURN(RET_DMX_OS_OPERA_FAIL);
}


MRESULT DmxInstDeleteSema(DMX_INST_T *prDmxInst)
{
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prDmxInst)
		MM_RETURN(RET_DMX_OK);


	if (NULL != prDmxInst->hLock) {
		mrRet = dmx_sema_delete(prDmxInst->hLock);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
				TEXT("[SPT] %s fail in delete semaphore, pvDmxInst:0x%x, ")
				TEXT("mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, prDmxInst, mrRet);
			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
		}

		prDmxInst->hLock = NULL;
	}

	MM_RETURN(RET_DMX_OK);
}


MRESULT DmxInstLockSema(DMX_INST_T *prDmxInst)
{
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prDmxInst)
		MM_RETURN(RET_DMX_PARAM_WRONG);


	mrRet = dmx_sema_lock(prDmxInst->hLock, DMX_SEMA_OPTION_WAIT);
	if (DMX_SUCCEED(mrRet))
		MM_RETURN(RET_DMX_OK);


	DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[SPT] %s fail in lock semaphore, pvDmxInst:0x%x, i4Ret: 0x%x\r\n"),
		DMX_FUNC_NAME, prDmxInst, mrRet);

	MM_RETURN(mrRet);
}


MRESULT DmxInstReleaseSema(DMX_INST_T *prDmxInst)
{
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prDmxInst)
		MM_RETURN(RET_DMX_PARAM_WRONG);

	mrRet = dmx_sema_unlock(prDmxInst->hLock);
	if (DMX_SUCCEED(mrRet))
		MM_RETURN(RET_DMX_OK);

	DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[SPT] %s fail in unlock semahpore, pvDmxInst:0x%x, i4Ret: 0x%x\r\n"),
		DMX_FUNC_NAME, prDmxInst, mrRet);

	MM_RETURN(mrRet);
}

void DMXDisableAllStmsInSpt(DMX_SPT_INST_T	*prSpt,
	DMX_INST_T *prDmxInst)
{
	DMX_STM_INST_T	*prStm = NULL;
	u32	u4Idxj = 0;
	MRESULT mrRet = RET_DMX_OK;

	for (u4Idxj = 0; u4Idxj < MAX_SPT_STM_CONNECTED; u4Idxj++) {
		if (NULL != prSpt->pvStmHandles[u4Idxj]) {
			prStm = (DMX_STM_INST_T *)(prSpt->pvStmHandles[u4Idxj]);
			if (((u32)prSpt == (u32)prStm->pvSptHdl) &&
				((u32)prDmxInst == (u32)prStm->pvDmxInst)) {
				switch (prStm->u4StmType) {
				case STREAM_VIDEO:
				case STREAM_AUDIO:
				case STREAM_SUBPICTURE:
				case STREAM_SECTION:
					mrRet = StreamDisable(prStm);
					if (DMX_FAILED(mrRet)) {
						DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
							TEXT("[DMX] %s line %d")
							TEXT("failedin StreamDisable, pvSptHdl: 0x%x, ")
							TEXT("mrRet: 0x%x.\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO, prSpt, mrRet);
					}
					break;
				default:
					break;
				}
			}
		}
	}
}

bool DmxStmIsInInst(DMX_INST_T *prDmxInst,
	DMX_STM_INST_T *prStm, u32 *pu4Idx)
{
	u32 u4Idx = 0;

	for (u4Idx = 0; u4Idx < MAX_SPT_STM_CONNECTED; u4Idx++) {
		if ((prStm == prDmxInst->parStm[u4Idx]) &&
			((u32)prDmxInst == (u32)prStm->pvDmxInst)) {
			break;
		}
	}

	if (NULL != pu4Idx)
		*pu4Idx = u4Idx;

	if (u4Idx < MAX_SPT_STM_CONNECTED)
		return TRUE;

	return FALSE;
}

void DMXFlushAllStmsInSpt(DMX_SPT_INST_T	*prSpt,
	DMX_INST_T *prDmxInst)
{
	DMX_STM_INST_T	*prStm = NULL;
	u32	u4Idxj = 0;
	MRESULT mrRet = RET_DMX_OK;

	for (u4Idxj = 0; u4Idxj < MAX_SPT_STM_CONNECTED; u4Idxj++) {
		if (NULL != prSpt->pvStmHandles[u4Idxj]) {
			prStm = (DMX_STM_INST_T *)(prSpt->pvStmHandles[u4Idxj]);
			if (((u32)prSpt == (u32)prStm->pvSptHdl) &&
				((u32)prDmxInst == (u32)prStm->pvDmxInst)) {
				switch (prStm->u4StmType) {
				case STREAM_VIDEO:
				case STREAM_AUDIO:
				case STREAM_SUBPICTURE:
				case STREAM_SECTION:

					if (!DmxStmIsInInst(prDmxInst, prStm, NULL))
						continue;

					mrRet = StreamSetFlush(prStm);
					if (DMX_FAILED(mrRet)) {
						DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
							TEXT("[DMX] %s line %d ")
							TEXT("failed in StreamSetFlush, pvSptHdl: 0x%x, ")
							TEXT("mrRet: 0x%x.\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO, prSpt, mrRet);
					}
					break;
				default:
					break;
				}
			}
		}
	}
}


void DMXDestroyAllStmsInSpt(DMX_SPT_INST_T	*prSpt,
	DMX_INST_T *prDmxInst)
{
	DMX_STM_INST_T	*prStm = NULL;
	u32	u4Idxj = 0, u4Idxk = 0;
	MRESULT mrRet = RET_DMX_OK;

	for (u4Idxj = 0; u4Idxj < MAX_SPT_STM_CONNECTED; u4Idxj++) {
		if (NULL != prSpt->pvStmHandles[u4Idxj]) {
			prStm = (DMX_STM_INST_T *)(prSpt->pvStmHandles[u4Idxj]);
			if (((u32)prSpt == (u32)prStm->pvSptHdl) &&
				((u32)prDmxInst == (u32)prStm->pvDmxInst)) {
				switch (prStm->u4StmType) {
				case STREAM_VIDEO:
				case STREAM_AUDIO:
				case STREAM_SUBPICTURE:
				case STREAM_SECTION:

					u4Idxk = MAX_SPT_STM_CONNECTED;
					if (!DmxStmIsInInst(prDmxInst, prStm, &u4Idxk))
						continue;

					mrRet = StreamDestroy(prSpt, prStm);
					if (DMX_FAILED(mrRet)) {
						DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
							TEXT("[DMX] %s line %d ")
							TEXT("failed in StreamDestroy, pvSptHdl: 0x%x, ")
							TEXT("mrRet: 0x%x.\r\n"),
							DMX_FUNC_NAME, DMX_LINE_NO, prSpt, mrRet);
					}

					for (; u4Idxk + 1 < prDmxInst->u4StmCnt; u4Idxk++)
						prDmxInst->parStm[u4Idxk] =
							prDmxInst->parStm[u4Idxk + 1];

					prDmxInst->parStm[prDmxInst->u4StmCnt - 1] = NULL;
					prDmxInst->u4StmCnt -= 1;

					break;
				default:
					break;
				}
			}
		}
	}
}

bool DMXCloseSptInsts(DMX_INST_T *prDmxInst)
{
	DMX_SPT_INST_T	*prSpt = NULL;
	u32	u4Idxi = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s line %d enter\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	while (prDmxInst->u4SptCnt != 0) {
		prSpt = prDmxInst->parSpt[0];
		if ((NULL != prSpt) && (prSpt->fgCreated)) {
			mrRet = SplitterSetParserPause(prSpt);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
					TEXT("[DMX] %s line %d failed in")
					TEXT("SplitterSetParserPause, pvSptHdl: 0x%x, mrRet: 0x%x.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt, mrRet);
			}

			DMXDisableAllStmsInSpt(prSpt, prDmxInst);

			mrRet = SplitterSetParserOff(prSpt);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
					TEXT("[DMX] %s line %d failed in ")
					TEXT("SplitterSetParserOff, pvSptHdl: 0x%x, mrRet: 0x%x.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt, mrRet);
			}

			DMXFlushAllStmsInSpt(prSpt, prDmxInst);

			DMXDestroyAllStmsInSpt(prSpt, prDmxInst);

			mrRet = SplitterDisable(prSpt);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
					TEXT("[DMX] %s line %d failed in ")
					TEXT("SplitterDisable, pvSptHdl: 0x%x, mrRet: 0x%x.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt, mrRet);
			}

			#ifdef __linux__
			if (g_Spt_EnableCnt > 0)
				g_Spt_EnableCnt--;

			#endif /* __linux__*/

			mrRet = SplitterDestroy(prSpt);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
					TEXT("[DMX] %s line %d failed in ")
					TEXT("SplitterDestroy, pvSptHdl: 0x%x, mrRet: 0x%x.\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt, mrRet);
			}

			for (u4Idxi = 0; u4Idxi + 1 < prDmxInst->u4SptCnt; u4Idxi++)
				prDmxInst->parSpt[u4Idxi] = prDmxInst->parSpt[u4Idxi + 1];

			prDmxInst->parSpt[prDmxInst->u4SptCnt - 1] = NULL;
			prDmxInst->u4SptCnt -= 1;
		}
	}

	if ((prDmxInst->u4SptCnt > 0) ||
		(prDmxInst->u4StmCnt > 0)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for unexpect error,")
			TEXT(" some splitter, stm, gau instance hasn't been release.\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO);

		return FALSE;
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s line %d exit, success\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	return TRUE;
}

u32 DmxGetInstCount(void)
{
	u32 u4Count = 0;

	DmxInstListLockSema();
	u4Count =  g_rDmxInstList.u4Cnt;
	DmxInstListReleaseSema();

	return u4Count;
}

u32 dmx_inst_create(void)
{
	DMX_INST_T *prDmxInst = NULL;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter\r\n"),
		DMX_FUNC_NAME);

	DmxInstListLockSema();

	DMX_NewMemory(sizeof(DMX_INST_T), prDmxInst);

	if (NULL == prDmxInst) {
		DmxInstListReleaseSema();
		return 0;
	}

	dmx_memset(prDmxInst, 0, sizeof(DMX_INST_T));

	mrRet = DmxInstCreateSema(prDmxInst);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in DmxInstCreateSema.\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_FreeMemory(prDmxInst);
		DmxInstListReleaseSema();
		return 0;
	}

	/* init PSR HAL*/
	/*Initialize PVR HW, and PSR_HW Structure*/
	mrRet = PSR_HAL_Init();
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in PSR_HAL_Init()\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DmxInstDeleteSema(prDmxInst);
		DMX_FreeMemory(prDmxInst);
		DmxInstListReleaseSema();
		return 0;
	}

	prDmxInst->prPrev = NULL;
	prDmxInst->prNext = g_rDmxInstList.prHead;
	if (NULL != g_rDmxInstList.prHead)
		g_rDmxInstList.prHead->prPrev = prDmxInst;


	g_rDmxInstList.prHead = prDmxInst;
	if (NULL == g_rDmxInstList.prTail)
		g_rDmxInstList.prTail = prDmxInst;

	g_rDmxInstList.u4Cnt += 1;

	g_fgDmxDmaTwice = TRUE;

	DmxInstListReleaseSema();

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s exit, success, pvDmxInst: 0x%p, DmxInstCnt: %d\r\n"),
		DMX_FUNC_NAME, prDmxInst, g_rDmxInstList.u4Cnt);

	return ((u32)prDmxInst);
}

EXPORT_SYMBOL(dmx_inst_create);

void *DmxInstGet(void)
{
	DMX_INST_T *prDmxInst = NULL;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter\r\n"),
		DMX_FUNC_NAME);

	DmxInstListLockSema();

	if (NULL == g_rDmxInstList.prHead)
		return 0;
	else
		prDmxInst = g_rDmxInstList.prHead;

	DmxInstListReleaseSema();

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s exit, success, pvDmxInst: 0x%x, DmxInstCnt: %d\r\n"),
		DMX_FUNC_NAME, (u32)prDmxInst, g_rDmxInstList.u4Cnt);

	return ((void *)prDmxInst);
}

MRESULT DmxInstReset(void *dwContext)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	MRESULT mrRet = RET_DMX_OK;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4SptIdx = 0;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter\r\n"),
		DMX_FUNC_NAME);
	if (NULL == prDmxInst) {
		DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	//get the stream handle and destroy the stream.
	for (u4SptIdx = 0; u4SptIdx < MAX_SPT_INST_CNT_PER_DMX; u4SptIdx++) {
		prSpt = prDmxInst->parSpt[u4SptIdx];
		if (NULL != prSpt) {
			DMXDestroyAllStmsInSpt(prSpt, prDmxInst);
		}
	}
	
	if (0 != prDmxInst->u4StmCnt) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
				TEXT("[DMX] %s line %d fail for destroy all stream.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
	}

	//get the splitter handle and destroy the splitter
	for (u4SptIdx = 0; u4SptIdx < MAX_SPT_INST_CNT_PER_DMX; u4SptIdx++) {
		prSpt = prDmxInst->parSpt[u4SptIdx];
		if (NULL != prSpt) {
			mrRet = SplitterDestroy((void *)prSpt);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
					TEXT("[DMX] %s line %d fail in SplitterDestroy(pvSptHdl: ")
					TEXT("0x%p), dwContext: 0x%p, mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSpt, dwContext, mrRet);
			}
			prDmxInst->u4SptCnt--;
		}
	}
	
	if (0 != prDmxInst->u4SptCnt) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
				TEXT("[DMX] %s line %d fail for destroy all splitter.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
	}
		
	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s exit, success.\r\n"),DMX_FUNC_NAME);

	MM_RETURN(mrRet);
}

bool dmx_inst_release(void *dwContext)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32	u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;
	bool	fgRet = TRUE;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if (NULL == prDmxInst) {
		DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		return FALSE;
	}

	DmxInstLockSema(prDmxInst);

	for (u4Idx = 0; u4Idx < prDmxInst->u4SptCnt; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if ((NULL != prSpt) &&
			(prSpt->fgCreated) &&
			((u32)prSpt->pvDmxInst == (u32)prDmxInst)) {
			break;
		}
	}

	if (u4Idx < prDmxInst->u4SptCnt) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d ---- FATAL WARNING: DMX exit flow is ")
			TEXT("abnormal, may be MW has some problems in release flow\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);

		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d ---- DMX enter into dmx_close")
			TEXT(" error handling function (DMX_CloseAllSptInsts)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);

		fgRet = DMXCloseSptInsts(prDmxInst);
		if (!fgRet) {
			DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
				TEXT("[DMX] %s line %d fail in DMXCloseSptInsts")
				TEXT("(Context: 0x%x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prDmxInst);
		}
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = DmxInstDeleteSema(prDmxInst);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in DmxInstDeleteSema().\r\n"),
		  DMX_FUNC_NAME, DMX_LINE_NO);
	}

	DmxInstListLockSema();

	if (NULL != prDmxInst->prPrev)
		prDmxInst->prPrev->prNext = prDmxInst->prNext;

	if (NULL != prDmxInst->prNext)
		prDmxInst->prNext->prPrev = prDmxInst->prPrev;

	if (prDmxInst == g_rDmxInstList.prHead)
		g_rDmxInstList.prHead = prDmxInst->prNext;

	if (prDmxInst == g_rDmxInstList.prTail)
		g_rDmxInstList.prTail = prDmxInst->prPrev;


	g_rDmxInstList.u4Cnt -= 1;

	if (g_rDmxInstList.u4Cnt <= 0)
		PSR_HAL_Uninit();

	DmxInstListReleaseSema();

	DMX_FreeMemory(prDmxInst);

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s exit, success, pvDmxInst: 0x%p, ")
		TEXT("DmxInstCnt: %d\r\n"),
		DMX_FUNC_NAME, prDmxInst, g_rDmxInstList.u4Cnt);

	return fgRet;
}
EXPORT_SYMBOL(dmx_inst_release);

MRESULT dmx_sptinst_create(void *dwContext, void **ppvSptHdl)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	void	*pvSptHdl  = NULL;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) ||
		(NULL == ppvSptHdl)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt >= MAX_SPT_INST_CNT_PER_DMX) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance(context: 0x%x)'s ")
			TEXT("splitter count(%d) encounter the max value(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext,
			prDmxInst->u4SptCnt, MAX_SPT_INST_CNT_PER_DMX);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = SplitterCreate((void *)prDmxInst, &pvSptHdl);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SplitterCreate")
			TEXT("(pvDmxInst: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(mrRet);
	}

	DmxInstLockSema(prDmxInst);
	prDmxInst->parSpt[prDmxInst->u4SptCnt] = pvSptHdl;
	prDmxInst->u4SptCnt += 1;
	DmxInstReleaseSema(prDmxInst);

	if (MAX_SPT_INST_CNT_PER_DMX <= prDmxInst->u4SptCnt) {
		DMX_ASSERT(0);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	*ppvSptHdl = pvSptHdl;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x, its splitter ")
		TEXT("inst cnt: %d\r\n"),
		DMX_FUNC_NAME, dwContext, prDmxInst->u4SptCnt);

	MM_RETURN(RET_DMX_OK);
}
EXPORT_SYMBOL(dmx_sptinst_create);

MRESULT dmx_sptinst_destroy(void *dwContext, void *pvSptHdl)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) ||
		(NULL == pvSptHdl)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (; u4Idx + 1 < prDmxInst->u4SptCnt; u4Idx++)
		prDmxInst->parSpt[u4Idx] = prDmxInst->parSpt[u4Idx + 1];

	prDmxInst->parSpt[prDmxInst->u4SptCnt - 1] = NULL;
	prDmxInst->u4SptCnt -= 1;
	DmxInstReleaseSema(prDmxInst);

	mrRet = SplitterDestroy(pvSptHdl);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SplitterDestroy(pvSptHdl: ")
			TEXT("0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext, mrRet);
		DMX_ASSERT(FALSE);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x, its splitter ")
		TEXT("inst cnt: %d\r\n"),
		DMX_FUNC_NAME, dwContext, prDmxInst->u4SptCnt);

	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_sptinst_destroy);

MRESULT dmx_sptinst_enable(void	*dwContext, void	*pvSptHdl,
	DMX_PBBUF_CONFIG_INFO_T	*prPbbufCfgInfo)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) ||
		(NULL == pvSptHdl) ||
		(NULL == prPbbufCfgInfo)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = SplitterEnable(pvSptHdl, prPbbufCfgInfo);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SplitterEnable(pvSptHdl: ")
			TEXT("0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_sptinst_enable);

MRESULT dmx_sptinst_disable(void *dwContext, void *pvSptHdl)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) ||
		(NULL == pvSptHdl)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = SplitterDisable(pvSptHdl);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SplitterDisable(pvSptHdl:")
			TEXT(" 0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_sptinst_disable);

bool dmx_spt_isenable(void *dwContext, void *pvSptHdl)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32	u4Idx = 0;
	bool	fgEnable = FALSE;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) ||
		(NULL == pvSptHdl)) {
		return FALSE;
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		return FALSE;
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has no ")
			TEXT("corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		return FALSE;
	}

	fgEnable = SplitterIsEnable(pvSptHdl);

	DmxInstReleaseSema(prDmxInst);

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	return fgEnable;
}
EXPORT_SYMBOL(dmx_spt_isenable);

MRESULT DmxSetRate(void *dwContext, SPT_PARAM_SET_RATE *prSetRate)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) ||
		(NULL == prSetRate)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)prSetRate->pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSetRate->pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = SplitterSetRate(prSetRate->pvSptHdl, prSetRate->i4Rate,
		prSetRate->fgDmaAud);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SplitterSetRate(pvSptHdl: 0x%x, ")
			TEXT("Rate: %d, fgDmaAud: %s), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSetRate->pvSptHdl, prSetRate->i4Rate,
			(prSetRate->fgDmaAud ? TEXT("TRUE") : TEXT("FALSE")),
			dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	MM_RETURN(mrRet);
}

MRESULT dmx_stminst_create(void *dwContext,
	STM_PARAM_CREATE *prParam, void **ppvStm)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) ||
		(NULL == prParam)	||
		(NULL == ppvStm)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)prParam->pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has no ")
			TEXT("corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prParam->pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = StreamCreate((void *)prDmxInst, prParam, ppvStm);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in StreamCreate(pvSptHdl: 0x%x, ")
			TEXT("StmType: %d, StmUID: %d, SWDecMask: 0x%llx), ")
			TEXT("dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prParam->pvSptHdl, prParam->u4StmType,
			prParam->u4StmUID, prParam->u8DecSendBufMask,
			dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxInstLockSema(prDmxInst);
	prDmxInst->parStm[prDmxInst->u4StmCnt] = *ppvStm;
	prDmxInst->u4StmCnt += 1;
	DmxInstReleaseSema(prDmxInst);

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x, its stream inst cnt: %d\r\n"),
		DMX_FUNC_NAME, dwContext, prDmxInst->u4StmCnt);

	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_stminst_create);

MRESULT dmx_stminst_destroy(void *dwContext, void *pvSptHdl, void *pvStm)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	DMX_STM_INST_T *prStm = NULL;
	u32 u4Idx = 0, u4Idxj = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) ||
		(NULL == pvSptHdl)	 ||
		(NULL == pvStm)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (prDmxInst->u4StmCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no stream instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idxj = 0; u4Idxj < MAX_STM_INST_CNT_PER_DMX; u4Idxj++) {
		prStm = prDmxInst->parStm[u4Idxj];
		if (((u32)prStm == (u32)pvStm) &&
			 ((u32)prStm->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_STM_INST_CNT_PER_DMX <= u4Idxj) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no corresponding stream instance(pvSptHdl: 0x%x, pvStmHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, pvStm, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (; u4Idxj + 1 < prDmxInst->u4StmCnt; u4Idxj++)
		prDmxInst->parStm[u4Idxj] = prDmxInst->parStm[u4Idxj + 1];
	prDmxInst->parStm[prDmxInst->u4StmCnt - 1] = NULL;
	prDmxInst->u4StmCnt -= 1;
	DmxInstReleaseSema(prDmxInst);

	mrRet = StreamDestroy(pvSptHdl, pvStm);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in StreamDestroy")
			TEXT("(pvSptHdl: 0x%x, pvStmHdl: 0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, pvStm, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x, its stream ")
		TEXT("inst cnt: %d\r\n"),
		DMX_FUNC_NAME, dwContext, prDmxInst->u4StmCnt);

	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_stminst_destroy);

MRESULT dmx_stminst_enable(void *dwContext, void *pvStm)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_STM_INST_T *prStm = NULL;
	u32 u4Idxj = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) ||
		(NULL == pvStm)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (prDmxInst->u4StmCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no stream instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idxj = 0; u4Idxj < MAX_STM_INST_CNT_PER_DMX; u4Idxj++) {
		prStm = prDmxInst->parStm[u4Idxj];
		if (((u32)prStm == (u32)pvStm) &&
			 ((u32)prStm->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_STM_INST_CNT_PER_DMX <= u4Idxj) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no corresponding stream instance(pvStmHdl: 0x%p), ")
			TEXT("dwContext: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvStm, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = StreamEnable(pvStm);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in StreamEnable")
			TEXT("(pvStm: 0x%p), dwContext: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvStm, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s line %d success in StreamEnable")
		TEXT("(pvStm: 0x%p), dwContext: 0x%p, mrRet: 0x%x\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, pvStm, dwContext, mrRet);

	MM_RETURN(mrRet);
}

EXPORT_SYMBOL(dmx_stminst_enable);

MRESULT dmx_stminst_disable(void *dwContext, void *pvStmHdl)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_STM_INST_T *prStm = NULL;
	u32 u4Idx = 0, u4Idxj = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) ||
		(NULL == pvStmHdl)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (prDmxInst->u4StmCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no stream instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idxj = 0; u4Idxj < MAX_STM_INST_CNT_PER_DMX; u4Idxj++) {
		prStm = prDmxInst->parStm[u4Idx];
		if (((u32)prStm == (u32)pvStmHdl) &&
			 ((u32)prStm->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_STM_INST_CNT_PER_DMX <= u4Idxj) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no corresponding stream instance(pvStmHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvStmHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = StreamDisable(pvStmHdl);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in StreamDisable")
			TEXT("(pvStmHdl: 0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvStmHdl, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s line %d success in StreamDisable")
		TEXT("(pvStmHdl: 0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, pvStmHdl, dwContext, mrRet);

	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_stminst_disable);

MRESULT dmx_stminst_setfifoflush(void *dwContext, void *pvStmHdl)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_STM_INST_T *prStm = NULL;
	u32 u4Idx = 0, u4Idxj = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) ||
		(NULL == pvStmHdl)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (prDmxInst->u4StmCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no stream instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idxj = 0; u4Idxj < MAX_STM_INST_CNT_PER_DMX; u4Idxj++) {
		prStm = prDmxInst->parStm[u4Idx];
		if (((u32)prStm == (u32)pvStmHdl) &&
			 ((u32)prStm->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_STM_INST_CNT_PER_DMX <= u4Idxj) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no corresponding stream instance(pvStmHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvStmHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = StreamSetFlush(pvStmHdl);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in StreamSetFlush")
			TEXT("(pvStmHdl: 0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvStmHdl, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s line %d success in StreamSetFlush")
		TEXT("(pvStmHdl: 0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, pvStmHdl, dwContext, mrRet);

	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_stminst_setfifoflush);


MRESULT dmx_stminst_setfifoinfo(void *dwContext, void *pvStmHdl, u32 u4FifoSz)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_STM_INST_T *prStm = NULL;
	u32 u4Idxj = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) ||
		(NULL == pvStmHdl)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (prDmxInst->u4StmCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no stream instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idxj = 0; u4Idxj < MAX_STM_INST_CNT_PER_DMX; u4Idxj++) {
		prStm = prDmxInst->parStm[u4Idxj];
		if (((u32)prStm == (u32)pvStmHdl) &&
			 ((u32)prStm->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_STM_INST_CNT_PER_DMX <= u4Idxj) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no corresponding stream instance(pvStmHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvStmHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = StreamSetFifoInfo(pvStmHdl, u4FifoSz);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in StreamSetFifoInfo")
			TEXT("(pvStmHdl: 0x%x, fifosz: 0x%08x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvStmHdl, u4FifoSz, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s line %d success in StreamSetFifoInfo")
		TEXT("(pvStmHdl: 0x%x, fifosz: 0x%08x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, pvStmHdl, u4FifoSz, dwContext, mrRet);

	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_stminst_setfifoinfo);


MRESULT dmx_stminst_getfifofullness(void *dwContext, void *pvSptHdl,
	u32 u4SptDataType,
	u32 *pu4FullNess)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) ||
		(NULL == pvSptHdl)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = SplitterGetStmFifoFullness(pvSptHdl, u4SptDataType, pu4FullNess);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SplitterGetStmFifo")
			TEXT("Fullness(pvSptHdl: 0x%x, SptDataType: %d), dwContext: 0x%x, ")
			TEXT("mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, u4SptDataType, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_stminst_getfifofullness);

MRESULT dmx_parse_on(void *dwContext, DMX_PSR_ON_PARAM_T *prParam)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) ||
		(NULL == prParam) ||
		(NULL == prParam->pvSptHdl)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)prParam->pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has no ")
			TEXT("corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prParam->pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = SplitterSetParserOn(prParam);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SplitterSetParserOn")
			TEXT("(pvSptHdl: 0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prParam->pvSptHdl, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_parse_on);

MRESULT dmx_parse_pause(void *dwContext, void *pvSptHdl)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) ||
		(NULL == pvSptHdl)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = SplitterSetParserPause(pvSptHdl);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SplitterSetParser")
			TEXT("Pause(pvSptHdl: 0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_parse_pause);

MRESULT dmx_parse_off(void *dwContext, void *pvSptHdl)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) ||
		(NULL == pvSptHdl)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = SplitterSetParserOff(pvSptHdl);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SplitterSetParserOff")
			TEXT("(pvSptHdl: 0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_parse_off);

MRESULT dmx_sptinst_getpsrfileofst(void *dwContext, void *pvSptHdl,
	u64 *pu8FileOfst)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) || (NULL == pvSptHdl) || (NULL == pu8FileOfst))
		MM_RETURN(RET_DMX_PARAM_WRONG);


	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = SplitterGetFileOfst(pvSptHdl, pu8FileOfst);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SplitterGetFileOfst")
			TEXT("(pvSptHdl: 0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	MM_RETURN(mrRet);
}

MRESULT dmx_rsp_off(void *dwContext,
	SPLITTER_PTX_RSP_OFF_INFO_T *prParam)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) || (NULL == prParam) || (NULL == prParam->pvSptHdl))
		MM_RETURN(RET_DMX_PARAM_WRONG);


	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)prParam->pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prParam->pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = SplitterSetParserRspOff(
					prParam->pvSptHdl,
					prParam->ucRspTxType,
					prParam->ucRspMode,
					&(prParam->ucRspTxRet),
					prParam->ucState);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SplitterSetParser")
			TEXT("RspOff(pvSptHdl: 0x%x, RspTxType: 0x%02x, RspTxMode: 0x%02x, ")
			TEXT("ucState: 0x%02x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prParam->pvSptHdl, prParam->ucRspTxType,
			prParam->ucRspMode, prParam->ucState, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if (prParam->ucRspTxRet != 1) {
		mrRet = RET_DMX_UNSUPPORT;
	}

	MM_RETURN(mrRet);
}

MRESULT dmx_rsp_rebuf(void *dwContext,
	SPLITTER_PTX_REBUFFER_RANGE_INFO_T *prParam)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) || (NULL == prParam) || (NULL == prParam->pvSptHdl))
		MM_RETURN(RET_DMX_PARAM_WRONG);


	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)prParam->pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prParam->pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = SplitterGetRebufferRange(
					prParam->pvSptHdl,
					prParam->u8PtsDelay,
					prParam->u8RspStartPts,
					&(prParam->u8RspStartOffset),
					&(prParam->u8PbbStartOffset),
					&(prParam->fgRebuf));
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SplitterSetParserRspOff")
			TEXT("(pvSptHdl: 0x%x, PtsDelay: 0x%llx, RspStartPts: 0x%llx), ")
			TEXT("dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prParam->pvSptHdl, prParam->u8PtsDelay,
			prParam->u8RspStartPts, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	MM_RETURN(mrRet);
}

MRESULT dmx_rsp_on(void *dwContext, SPLITTER_PTX_RSP_ON_INFO_T *prParam)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) || (NULL == prParam) || (NULL == prParam->pvSptHdl))
		MM_RETURN(RET_DMX_PARAM_WRONG);


	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)prParam->pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prParam->pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = SplitterSetParserRspOn(prParam->pvSptHdl, prParam->fgRebuf);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SplitterSetParserRspOn")
			TEXT("(pvSptHdl: 0x%x, fgRebuf: %s), dwContext: 0x%x, ")
			TEXT("mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prParam->pvSptHdl,
			((prParam->fgRebuf) ? TEXT("TRUE") : TEXT("FALSE")),
			dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);
	MM_RETURN(mrRet);
}

MRESULT dmx_cfa_settype(void *dwContext, CFA_PARAM_SET_TYPE *prParam)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);
	if (NULL == prDmxInst) {
		DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s fail for prDmxInst == NULL\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	if (NULL == prParam) {
		DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s fail for prParam == NULL\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (NULL == prParam->pvSptHdl) {
		DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s fail for prParam->pvSptHdl == NULL\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)prParam->pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prParam->pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = SplitterSetCfaType(prParam->pvSptHdl, prParam->u4Type);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SplitterSetCfaType")
			TEXT("(pvSptHdl: 0x%x, CfaType: 0x%x), dwContext: 0x%x, ")
			TEXT("mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prParam->pvSptHdl, prParam->u4Type,
			dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);
	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_cfa_settype);

MRESULT dmx_cfa_config(void *dwContext, void *pvSptHdl, void *pvConfigInfo, bool fgIsUserMem)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) ||
		(NULL == pvSptHdl)		||
		(NULL == pvConfigInfo)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = SplitterSetCfaConfigure(pvSptHdl, pvConfigInfo, fgIsUserMem);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SplitterSetCfa")
			TEXT("Configure(pvSptHdl: 0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);
	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_cfa_config);

MRESULT dmx_cfa_setrange(void *dwContext, void *pvSptHdl, void *pvRangeInfo, bool fgIsUserMem)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) ||
		(NULL == pvSptHdl)		||
		(NULL == pvRangeInfo)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance")
			TEXT(" has no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = SplitterSetCfaRange(pvSptHdl, pvRangeInfo, fgIsUserMem);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SplitterSetCfaRange")
			TEXT("(pvSptHdl: 0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);
	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_cfa_setrange);

MRESULT dmx_cfa_setinquiretype(void *dwContext, void *pvSptHdl, u32 u4CfaQID)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) ||
		(NULL == pvSptHdl)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = SplitterSetCfaInquire(pvSptHdl, u4CfaQID);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SplitterSetCfaInquire")
			TEXT("(pvSptHdl: 0x%x, u4CfaQID: 0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, u4CfaQID, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);
	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_cfa_setinquiretype);

MRESULT dmx_cfa_setgeneral(void *dwContext,
	void *pvSptHdl, u32 u4CfaFID, void *pvCfaParameter,
	u32 u4CfaParameterSize)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) || (NULL == pvSptHdl) || (NULL == pvCfaParameter) ||
		(0 == u4CfaParameterSize)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has")
			TEXT(" no corresponding splitter instance(pvSptHdl: 0x%x), dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = SplitterSetCfaGeneral(pvSptHdl, u4CfaFID, pvCfaParameter, u4CfaParameterSize);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SplitterSetCfaGeneral")
			TEXT("(pvSptHdl: 0x%x, u4CfaFID: 0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, u4CfaFID, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);
	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_cfa_setgeneral);

MRESULT dmx_cfa_getgeneral(void *dwContext,
	void *pvSptHdl, u32 u4CfaFID, void *pvCfaParameter,
	u32 u4CfaParameterSize)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) || (NULL == pvSptHdl) || (NULL == pvCfaParameter) ||
		(0 == u4CfaParameterSize)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has ")
			TEXT("no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = SplitterGetCfaGeneral(pvSptHdl, u4CfaFID, pvCfaParameter,
		u4CfaParameterSize);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SplitterGetCfaGeneral")
			TEXT("(pvSptHdl: 0x%x, u4CfaFID: 0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, u4CfaFID, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_cfa_getgeneral);

MRESULT dmx_cfa_getpsrpos(void *dwContext,
	void *pvSptHdl, u64 *pu8FileOfst)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s enter, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);

	if ((NULL == prDmxInst) ||
		(NULL == pvSptHdl)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has ")
			TEXT("no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has ")
			TEXT("no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = SplitterGetCfaPosition(pvSptHdl, pu8FileOfst);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in SplitterGetCfaPosition")
			TEXT("(pvSptHdl: 0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogT(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
		TEXT("[DMX] %s success, pvDmxInst: 0x%x\r\n"),
		DMX_FUNC_NAME, dwContext);
	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_cfa_getpsrpos);

MRESULT dmx_pbbuf_allocbuf(void *dwContext, void *pvSptHdl,
	SEND_BUFFER *prSdBuf)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prDmxInst) ||
		(NULL == pvSptHdl)		||
		(NULL == prSdBuf)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = PBBUF_GetAllocSlot(pvSptHdl, prSdBuf);

	if (DMX_FAILED(mrRet) && (!MM_IS_STATE_ERROR(mrRet))) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in PBBUF_GetAllocSlot")
			TEXT("(pvSptHdl: 0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_pbbuf_allocbuf);

MRESULT dmx_pbbuf_cancelalloc(void *dwContext, void *pvSptHdl)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prDmxInst) ||
		(NULL == pvSptHdl)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has ")
			TEXT("no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = PBBUF_CancelAllocSlot(pvSptHdl);

	if (DMX_FAILED(mrRet) && (!MM_IS_STATE_ERROR(mrRet))) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in PBBUF_CancelAllocSlot")
			TEXT("(pvSptHdl: 0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_pbbuf_cancelalloc);

MRESULT dmx_pbbuf_sendbuf(void *dwContext, void *pvSptHdl,
	SEND_BUFFER *prSdBuf, bool *pfgExitSent)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prDmxInst) ||
		(NULL == pvSptHdl) ||
		(NULL == prSdBuf)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance ")
			TEXT("has no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has ")
			TEXT("no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);
	mrRet = PBBUF_SendDataSlot(pvSptHdl, prSdBuf, pfgExitSent);

	if (DMX_FAILED(mrRet) && (!MM_IS_STATE_ERROR(mrRet))) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in PBBUF_SendDataSlot")
			TEXT("(pvSptHdl: 0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_pbbuf_sendbuf);

MRESULT dmx_pbbuf_releasebuf(void *dwContext, void *pvSptHdl,
	SEND_BUFFER *prSdBuf)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prDmxInst) ||
		(NULL == pvSptHdl)		||
		(NULL == prSdBuf)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has ")
			TEXT("no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has ")
			TEXT("no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DmxInstReleaseSema(prDmxInst);

	mrRet = PBBUF_ReleaseAllocSlot(pvSptHdl, prSdBuf);

	if (DMX_FAILED(mrRet) && (!MM_IS_STATE_ERROR(mrRet))) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail in PBBUF_ReleaseAllocSlot")
			TEXT("(pvSptHdl: 0x%x), dwContext: 0x%x, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, dwContext, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(mrRet);
}
EXPORT_SYMBOL(dmx_pbbuf_releasebuf);

MRESULT dmx_pbbuf_infonodata(void *dwContext,
	DMX_PBBUF_NODATA_PARAM_T *prParam)
{
	DMX_INST_T *prDmxInst = (DMX_INST_T *)dwContext;
	DMX_SPT_INST_T *prSpt = NULL;
	u32 u4Idx = 0;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prDmxInst) ||
		(NULL == prParam)	||
		(NULL == prParam->pvSptHdl)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxInstLockSema(prDmxInst);

	if (prDmxInst->u4SptCnt <= 0) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has ")
			TEXT("no splitter instance, dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	for (u4Idx = 0; u4Idx < MAX_SPT_INST_CNT_PER_DMX; u4Idx++) {
		prSpt = prDmxInst->parSpt[u4Idx];
		if (((u32)prSpt == (u32)prParam->pvSptHdl) &&
			 ((u32)prSpt->pvDmxInst == (u32)dwContext)) {
			break;
		}
	}

	if (MAX_SPT_INST_CNT_PER_DMX <= u4Idx) {
		DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
			TEXT("[DMX] %s line %d fail for the dmx instance has ")
			TEXT("no corresponding splitter instance(pvSptHdl: 0x%x), ")
			TEXT("dwContext: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prParam->pvSptHdl, dwContext);
		DmxInstReleaseSema(prDmxInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (GAU_E_EJECTCARD == prParam->u4Status)
	{
		GAU_SetEOS((void *)(prParam->pvSptHdl), TRUE, prParam->u4Status);
	}
	else
	{
		mrRet = SplitterSetFileEndOffset((void *)(prParam->pvSptHdl), prParam->u8FileEndOffset);
		if (RET_DMX_OK != mrRet)
		{
			DmxLogE(DMX_MOD_INST, DMX_MOD_INST_LOGLVL_DEFAULT,
				TEXT("[DMX] %s line %d fail for SplitterSetFileEndOffset.\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(mrRet);
		}
	}
	DmxInstReleaseSema(prDmxInst);
	MM_RETURN(RET_DMX_OK);
}
EXPORT_SYMBOL(dmx_pbbuf_infonodata);

bool dmx_stm_getau(void *pvStmHdlHdl, void *pvIOBuf)
{
	ESM_IO_BUF_INFO *prESMIOBuf = NULL;
	DMX_STM_INST_T *prStm = (DMX_STM_INST_T *)pvStmHdlHdl;
	MRESULT mrRet = RET_DMX_OK;
		
	if (NULL == prStm) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s fail for invalid stream handle\r\n"),
			DMX_FUNC_NAME);
		return FALSE;
	}

	prESMIOBuf = GAU_Get_GEsmIOBuf((u32)prStm->u4GAUHandle);
	if (NULL == prESMIOBuf) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s fail in GAU_Get_GEsmIOBuf, GAU handle: 0x%x\r\n"),
			(u32)prStm->u4GAUHandle);
		return FALSE;
	}

	mrRet = GAU_GetAU((u32)prStm->u4GAUHandle, (void *)prESMIOBuf);

	if (DMX_FAILED(mrRet))
		return FALSE;

	mm_memcpy(pvIOBuf, prESMIOBuf, sizeof(ESM_IO_BUF_INFO));

	return TRUE;
}
EXPORT_SYMBOL(dmx_stm_getau);

bool dmx_stm_releaseau(void *pvStmHdlHdl, void *pvIOBuf)
{
	DMX_STM_INST_T *prStm = (DMX_STM_INST_T *)pvStmHdlHdl;
	ESM_IO_BUF_INFO *prESMIOBuf = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prStm) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
			TEXT("%s fail for invalid stream handle\r\n"),
			DMX_FUNC_NAME);
		return FALSE;
	}

	if (NULL != pvIOBuf) {
		prESMIOBuf = GAU_Get_REsmIOBuf((u32)prStm->u4GAUHandle);
		if (NULL == prESMIOBuf) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DEFAULT,
				TEXT("%s fail in GAU_Get_REsmIOBuf, GAU handle: 0x%x\r\n"),
				(u32)prStm->u4GAUHandle);
			return FALSE;
		}

		mm_memcpy(prESMIOBuf, pvIOBuf, sizeof(ESM_IO_BUF_INFO));

		mrRet = GAU_ReleaseAU((u32)prStm->u4GAUHandle, (void *)prESMIOBuf, FALSE);
	} else {
		mrRet = GAU_ReleaseAU((u32)prStm->u4GAUHandle, NULL, FALSE);
	}

	if (DMX_FAILED(mrRet))
		return FALSE;

	return TRUE;
}
EXPORT_SYMBOL(dmx_stm_releaseau);


