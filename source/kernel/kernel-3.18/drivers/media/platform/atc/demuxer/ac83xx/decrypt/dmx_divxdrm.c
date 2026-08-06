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
 * @file dmx_DivxDRM.h
 *
 * @par Project
 *
 *
 * @par Description
 *	  DivxDRM Decryption Interface definations
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */

#ifndef __linux__
#pragma warning(push)
#pragma warning(disable : 4115) /*disable warning C4115: named type definition in parentheses*/
#endif

#ifdef __linux__
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_decrypt.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "dmx_decrypt.h"
#include "mm_debug.h"
#endif /*__linux__*/

#include "dmx_def.h"
#include "dmx_errcode.h"
#include "dmx_mem.h"
#include "dmx_cpsa.h"
#include "dmx_divxdrm.h"

#ifndef __linux__
#pragma warning(pop)
#endif

#if DMX_SUPPORT_DIVXDRM

#ifndef __linux__
#pragma warning(push)
#pragma warning(disable : 4115) /*disable warning C4115: named type definition in parentheses*/
#endif /*__linux__*/

#include "DrmApi.h"
#include "DrmTypes.h"

#ifndef __linux__
#include "metazoneex.h"
#else
#include "dmx_sema.h"
#include "metazone.h"
#include "mmisc.h"
#endif /*__linux__*/

#ifndef __linux__
#pragma warning(pop)
#endif /*__linux__*/

DECRYPT_DIVXDRM_INST_T	 *g_prDivxDRMInst = NULL;
bool			 g_fgDivxDRMInited = FALSE;

HANDLE g_hDivxDRMSema = NULL;
#define DIVXDRMLOCKINIT() do {		 \
	dmx_sema_create(&g_hDivxDRMSema, DMX_SEMA_TYPE_BINARY, \
		DMX_SEMA_STATE_UNLOCK); \
	if (NULL == g_hDivxDRMSema) {  \
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT, \
			TEXT("[DECRYPT] %s fail in create semaphore\r\n"), \
			DMX_FUNC_NAME); \
	}
} while (0)

#define DIVXDRMLOCK()			\
	dmx_sema_lock(g_hDivxDRMSema, DMX_SEMA_OPTION_WAIT)

#define DIVXDRMUNLOCK() \
	dmx_sema_unlock(g_hDivxDRMSema)

#define DIVXDRMLOCKDEINIT() \
	dmx_sema_delete(g_hDivxDRMSema)

#define DIVXDRMINSTLOCKINIT(prDivxDrmInst) do {	  \
	dmx_sema_create(&(prDivxDrmInst->hSema), DMX_SEMA_TYPE_BINARY, \
		DMX_SEMA_STATE_UNLOCK); \
	if (NULL == prDivxDrmInst->hSema) {  \
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT, \
			TEXT("[DECRYPT] %s fail in create semaphore\r\n"), \
			DMX_FUNC_NAME); \
	} \
} while (0)

#define DIVXDRMINSTLOCK(prDivxDrmInst)          \
	dmx_sema_lock(prDivxDrmInst->hSema, DMX_SEMA_OPTION_WAIT)

#define DIVXDRMINSTUNLOCK(prDivxDrmInst)                     \
	dmx_sema_unlock((HANDLE)(prDivxDrmInst->hSema))

#define DIVXDRMINSTLOCKDEINIT(prDivxDrmInst)              \
	dmx_sema_delete((HANDLE)(prDivxDrmInst->hSema))

#define DIVXDRMLOCKINITEX(mrRet) do {	\
	mrRet = dmx_sema_create(&g_hDivxDRMSema, DMX_SEMA_TYPE_BINARY, \
		DMX_SEMA_STATE_UNLOCK); \
	if (DMX_FAILED(mrRet)) {  \
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT, \
			TEXT("[DECRYPT] %s fail in create semaphore, mrRet: 0x%x\r\n"), \
			DMX_FUNC_NAME, mrRet); \
	} \
} while (0)

#define DIVXDRMLOCKEX(mrRet)    \
	dmx_sema_lock(g_hDivxDRMSema, DMX_SEMA_OPTION_WAIT)

#define DIVXDRMUNLOCKEX(mrRet)               \
	dmx_sema_unlock(g_hDivxDRMSema)

#define DIVXDRMLOCKDEINITEX(mrRet)  \
	dmx_sema_delete(g_hDivxDRMSema)

#define DIVXDRMINSTLOCKINITEX(prDivxDrmInst, mrRet) do {    \
	mrRet = dmx_sema_create(&(prDivxDrmInst->hSema), DMX_SEMA_TYPE_BINARY, \
		DMX_SEMA_STATE_UNLOCK); \
	if (DMX_FAILED(mrRet)) {  \
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT, \
			TEXT("[DECRYPT] %s fail in create semaphore for DivxDRM")\
			TEXT(" Inst(0x%x), mrRet: 0x%x\r\n"),\
			DMX_FUNC_NAME, prDivxDrmInst, mrRet); \
	} \
} while (0)

#define DIVXDRMINSTLOCKEX(prDivxDrmInst, mrRet) do { \
	dmx_sema_lock((HANDLE)(prDivxDrmInst->hSema), DMX_SEMA_OPTION_WAIT);	   \
	mrRet = mrRet; \
} while (0)

#define DIVXDRMINSTUNLOCKEX(prDivxDrmInst, mrRet) do {	   \
	dmx_sema_unlock((HANDLE)(prDivxDrmInst->hSema));	   \
	mrRet = mrRet; \
} while (0)

#define DIVXDRMINSTLOCKDEINITEX(prDivxDrmInst, mrRet) do {  \
	dmx_sema_delete((HANDLE)(prDivxDrmInst->hSema));	 \
	mrRet = mrRet; \
} while (0)

static MRESULT DivxDRMMapErrCode(drmErrorCodes_t eErrCode)
{
	switch (eErrCode) {
	case DRM_SUCCESS:
		return RET_DMX_OK;
	case DRM_NOT_AUTHORIZED:
		return RET_DMX_CPSA_NEVER_AUTHED;
	case DRM_NOT_REGISTERED:
		return RET_DMX_CPSA_NOT_REGED;
	case DRM_RENTAL_EXPIRED:
		return RET_DMX_CPSA_RENTAL_EXPIRED;
	case DRM_GENERAL_ERROR:
		return RET_DMX_CPSA_GEN_ERR;
	case DRM_NEVER_REGISTERED:
		return RET_DMX_CPSA_NEVER_REGED;
	default:
		break;
	}
		return RET_DMX_CPSA_GEN_ERR;
}

static MRESULT DivxDRMGetLastError(void *pvContext)
{
	drmErrorCodes_t eErrCode = DRM_GENERAL_ERROR;

	if (NULL == pvContext)
		return RET_DMX_CPSA_GEN_ERR;

	eErrCode = drmGetLastError((uint8_t *)pvContext);
	return DivxDRMMapErrCode(eErrCode);
}

MRESULT DivxDRMGetInstLastError(void *pvInst)
{
	DECRYPT_DIVXDRM_INST_T *prInst	= (DECRYPT_DIVXDRM_INST_T *)pvInst;
	MRESULT mrRet = RET_DMX_OK;

	if (!DivxDRMIsInstValid(prInst)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid DivxDRM Instance (hInst: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DIVXDRMINSTLOCK(prInst);

	prInst->u4Ref++;

	if (NULL == prInst->pvContext) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d failed for DivxDRM Inst(0x%x)'s context is NULL\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	mrRet = DivxDRMGetLastError(prInst->pvContext);

	prInst->u4Ref--;

	DIVXDRMINSTUNLOCK(prInst);

	MM_RETURN(mrRet);
}

MRESULT DivxDRMInitSystem(DECRYPT_DIVXDRM_INST_T *prInst)
{
	drmErrorCodes_t eErrCode = DRM_SUCCESS;
	MRESULT mrRet	= RET_DMX_OK;

	if (NULL == prInst) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	DIVXDRMINSTLOCK(prInst);

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s line %d -- DivxDRM Inst(0x%x)'s state : %d\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prInst, eErrCode);

	prInst->u4Ref++;

	if (NULL == prInst->pvContext) {
		u32	u4ContextSz = 0;

		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d -- DivxDRM Inst(0x%x)'s context is NULL\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst);

		eErrCode = drmInitSystem(NULL, (uint32_t *)(&u4ContextSz));

		if (DRM_SUCCESS != eErrCode) {
			mrRet = DivxDRMMapErrCode(eErrCode);
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmInitSystem ")
				TEXT("hInst: 0x%x, eErrCode: %d, mrRet: 0x%x)\r\n")),
				DMX_FUNC_NAME, DMX_LINE_NO, prInst, eErrCode, mrRet);
			prInst->u4Ref--;
			DIVXDRMINSTUNLOCK(prInst);
			MM_RETURN(mrRet);
		}

		if (0 == u4ContextSz) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for ContextSz ")
				TEXT("obtained by drmInitSystem is 0\r\n")),
				DMX_FUNC_NAME, DMX_LINE_NO);
			prInst->u4Ref--;
			DIVXDRMINSTUNLOCK(prInst);
			MM_RETURN(RET_DMX_CPSA_GEN_ERR);
		}

		DMX_NewMemory(u4ContextSz, prInst->pvContext);

		if (NULL == prInst->pvContext) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[DECRYPT][DIVXDRM] %s line %d fail for no memory, context size: %d\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4ContextSz);
			prInst->u4Ref--;
			DIVXDRMINSTUNLOCK(prInst);
			MM_RETURN(RET_DMX_NO_MEM);
		}

		dmx_memset(prInst->pvContext, 0, u4ContextSz);

		prInst->u4ContextSz   = u4ContextSz;
	}

	eErrCode = drmInitSystem(prInst->pvContext, (uint32_t *)(&(prInst->u4ContextSz)));
	if (DRM_SUCCESS != eErrCode) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmInitSystem ")
			TEXT("hInst: 0x%x, u4ContextSz: %d, eErrCode: %d)\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst, prInst->u4ContextSz, eErrCode);
		mrRet = DivxDRMMapErrCode(eErrCode);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(mrRet);
	}

	if (NULL != prInst->rHeader.pu1DrmHdrBuf) {
		DMX_FreeMemory(prInst->rHeader.pu1DrmHdrBuf);
		prInst->rHeader.pu1DrmHdrBuf = NULL;
		prInst->rHeader.u4DrmHdrBufSz = 0;
	}

	prInst->eStatus = DECRYPT_DIVXDRM_STATUS_SYSTEM_INITED;

	prInst->u4Ref--;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s (hInst: 0x%x) success\r\n"),
		DMX_FUNC_NAME, prInst);

	DIVXDRMINSTUNLOCK(prInst);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMDeInitSystem(DECRYPT_DIVXDRM_INST_T *prInst)
{
	if (NULL == prInst) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OK);
	}

	DIVXDRMINSTLOCK(prInst);

	prInst->u4Ref++;

	if (NULL != prInst->rHeader.pu1DrmHdrBuf) {
		DMX_FreeMemory(prInst->rHeader.pu1DrmHdrBuf);
		prInst->rHeader.pu1DrmHdrBuf = NULL;
		prInst->rHeader.u4DrmHdrBufSz = 0;
	}

	prInst->eStatus = DECRYPT_DIVXDRM_STATUS_SYSTEM_UNINIT;

	prInst->u4Ref--;

	DIVXDRMINSTUNLOCK(prInst);
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success\r\n"),
		DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}


MRESULT DivxDRMInitialize(void)
{
	MRESULT mrRet = RET_DMX_OK;

	if (g_fgDivxDRMInited)
		MM_RETURN(RET_DMX_OK);

	DIVXDRMLOCKINITEX(mrRet);

	if (DMX_FAILED(mrRet)) {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s failed in DIVXDRMLOCKINITEX, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, mrRet);
		MM_RETURN(mrRet);
	}

	mrRet = DivxDRMInitMemory();
	if (DMX_FAILED(mrRet)) {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s failed in DivxDRMInitMemory, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, mrRet);
		DIVXDRMLOCKDEINIT();
		MM_RETURN(mrRet);
	}
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s execute DivxDRMInitMemory success\r\n"),
		DMX_FUNC_NAME);

	g_fgDivxDRMInited = TRUE;

	MM_RETURN(mrRet);
}

MRESULT DivxDRMDeInitialize(void)
{
	DECRYPT_DIVXDRM_INST_T	 *prDivxDrmInst = NULL;
	DECRYPT_DIVXDRM_INST_T	 *prDivxDrmInstNext = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (!g_fgDivxDRMInited)
		MM_RETURN(RET_DMX_OK);

	if (NULL == g_prDivxDRMInst) {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s exit, DivxDRM Instance hasn't been initialized\r\n"),
			DMX_FUNC_NAME);
		DIVXDRMLOCKDEINIT();
		MM_RETURN(RET_DMX_OK);
	}

	prDivxDrmInstNext = g_prDivxDRMInst;

	while (NULL != prDivxDrmInstNext) {
		prDivxDrmInst = prDivxDrmInstNext;
		prDivxDrmInstNext = prDivxDrmInst->prNext;
		mrRet = DivxDRMReleaseInst(prDivxDrmInst);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in ")
				TEXT("DivxDRMReleaseInst(hInst: 0x%x), mrRet: 0x%x\r\n")),
				DMX_FUNC_NAME, DMX_LINE_NO, prDivxDrmInst, mrRet);
		}
	}

	g_prDivxDRMInst = NULL;

	DIVXDRMLOCKDEINIT();

	g_fgDivxDRMInited = FALSE;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT, TEXT("[DECRYPT][DIVXDRM] %s success\r\n"),
		DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMGetPlayInst(s32 i4DecryptId, void **ppvInst)
{
	DECRYPT_DIVXDRM_INST_T *prDivxDrmInst = NULL;

	if (NULL == ppvInst) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d failed for no memory\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*ppvInst = NULL;

	DIVXDRMLOCK();

	prDivxDrmInst = g_prDivxDRMInst;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT] %s line %d -- i4DecryptId: %d\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, i4DecryptId);

	while (NULL != prDivxDrmInst) {
		if ((DECRYPT_DIVXDRM_STATUS_FILE_PLAYING == prDivxDrmInst->eStatus) &&
			(i4DecryptId == prDivxDrmInst->i4DecryptId)) {
			break;
		}

		prDivxDrmInst = prDivxDrmInst->prNext;
	}

	if (NULL == prDivxDrmInst) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for no playing DivxDRM Instance handle\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DIVXDRMUNLOCK();
		return RET_DMX_UNEXPECT;
	}

	DIVXDRMINSTLOCK(prDivxDrmInst);

	prDivxDrmInst->u4Ref++;

	DIVXDRMINSTUNLOCK(prDivxDrmInst);

	*ppvInst = (void *)prDivxDrmInst;

	DIVXDRMUNLOCK();

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMRelPlayInst(void *pvInst)
{
	DECRYPT_DIVXDRM_INST_T *prDivxDrmInst = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == pvInst) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d failed for no memory\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prDivxDrmInst = (DECRYPT_DIVXDRM_INST_T *)pvInst;

	if (!DivxDRMIsInstValid(prDivxDrmInst)) {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d -- No corresponding DivxDRM Instance\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OK);
	}

	DIVXDRMINSTLOCK(prDivxDrmInst);

	if (1 == prDivxDrmInst->u4Ref) {
		DIVXDRMINSTUNLOCK(prDivxDrmInst);
		mrRet = DivxDRMReleaseInst(prDivxDrmInst);
	} else if (0 == prDivxDrmInst->u4Ref) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for DivxDRM ")
			TEXT("Instance(hInst: 0x%x)'s Ref Count(%d) error\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prDivxDrmInst, prDivxDrmInst->u4Ref);
		DMX_ASSERT(FALSE);
		DIVXDRMINSTUNLOCK(prDivxDrmInst);
		MM_RETURN(RET_DMX_ERR_STATE);
	} else {
		prDivxDrmInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prDivxDrmInst);
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMCreateInst(void **ppvInst)
{
	DECRYPT_DIVXDRM_INST_T *prDivxDrmInst = NULL;
	u32	u4ContextSz = 0;
	drmErrorCodes_t eResult = DRM_SUCCESS;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s enter\r\n"),
		DMX_FUNC_NAME);

	if (NULL == ppvInst) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d failed for no memory\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*ppvInst = NULL;

	DMX_NewMemory(sizeof(DECRYPT_DIVXDRM_INST_T), prDivxDrmInst);
	if (NULL == prDivxDrmInst) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d failed for no memory\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_NO_MEM);
	}

	dmx_memset((void *)prDivxDrmInst, 0, (sizeof(DECRYPT_DIVXDRM_INST_T)));

	DIVXDRMINSTLOCKINITEX(prDivxDrmInst, mrRet);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DIVXDRMINSTLOCKINITEX (mrRet: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		DMX_FreeMemory(prDivxDrmInst);
		MM_RETURN(mrRet);
	}

	eResult = drmInitSystem(NULL, (uint32_t *)(&u4ContextSz));

	if (DRM_SUCCESS != eResult) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmInitSystem (eErrCode: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult);
		DIVXDRMINSTLOCKDEINIT(prDivxDrmInst);
		DMX_FreeMemory(prDivxDrmInst);
		MM_RETURN(RET_DMX_CPSA_GEN_ERR);
	}

	if (0 == u4ContextSz) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for ContextSz obtained by drmInitSystem is 0\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DIVXDRMINSTLOCKDEINIT(prDivxDrmInst);
		DMX_FreeMemory(prDivxDrmInst);
		MM_RETURN(RET_DMX_CPSA_GEN_ERR);
	}

	DMX_NewMemory(u4ContextSz, prDivxDrmInst->pvContext);

	if (NULL == prDivxDrmInst->pvContext) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for no memory, context size: %d\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4ContextSz);
		DIVXDRMINSTLOCKDEINIT(prDivxDrmInst);
		DMX_FreeMemory(prDivxDrmInst);
		MM_RETURN(RET_DMX_NO_MEM);
	}

	dmx_memset(prDivxDrmInst->pvContext, 0, u4ContextSz);
	prDivxDrmInst->u4ContextSz	 = u4ContextSz;

	prDivxDrmInst->eType		 = DECRYPT_DIVXDRM;
	prDivxDrmInst->i4DecryptId	 = DECRYPT_PLAY_INVALID_ID;
	prDivxDrmInst->u4Ref		 = 0;
	prDivxDrmInst->eStatus		 = DECRYPT_DIVXDRM_STATUS_SYSTEM_UNINIT;
	prDivxDrmInst->prNext		 = NULL;

	prDivxDrmInst->u4Ref++;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s line %d -- Set DivxDRM Instance(0x%x)'s state to be %d\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prDivxDrmInst, prDivxDrmInst->eStatus);

	DIVXDRMLOCK();

	if (NULL == g_prDivxDRMInst) {
		g_prDivxDRMInst = prDivxDrmInst;
	} else {
		prDivxDrmInst->prNext = g_prDivxDRMInst;
		g_prDivxDRMInst = prDivxDrmInst;
	}

	DIVXDRMUNLOCK();

	*ppvInst = (void *)prDivxDrmInst;

	MM_RETURN(RET_DMX_OK)
}

MRESULT DivxDRMReleaseInst(void *pvInst)
{
	DECRYPT_DIVXDRM_INST_T *prInst = NULL, *prInstTmp = NULL, *prInstPrev = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL_HANDLE == pvInst) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prInst = (DECRYPT_DIVXDRM_INST_T *)pvInst;

	DIVXDRMLOCK();

	prInstTmp = g_prDivxDRMInst;

	prInstPrev = NULL;

	while (NULL != prInstTmp) {
		if (prInstTmp == prInst)
			break;

		prInstPrev = prInstTmp;
		prInstTmp = prInstTmp->prNext;
	}

	if (NULL == prInstTmp) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for no corresponding ")
			TEXT("DivxDRM Instance handle(hInst: 0x%x)\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst);
		DIVXDRMUNLOCK();
		return FALSE;
	}

	DIVXDRMUNLOCK();

	DIVXDRMINSTLOCK(prInst);

	if (0 == prInst->u4Ref) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for DivxDRM ")
			TEXT("Instance(hInst: 0x%x)'s Ref Count(%d) error\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst, prInst->u4Ref);
		DMX_ASSERT(FALSE);
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	if (1 < prInst->u4Ref) {
		prInst->u4Ref--;
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d -- DivxDRM Instance(hInst: 0x%x)'s Ref Count(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst, prInst->u4Ref);
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_OK);
	}

	DIVXDRMINSTUNLOCK(prInst);

	mrRet = DivxDRMExecCmd((void *)prInst, DIVXDRM_FINALIZE_PLAYBACK,
		NULL, 0, NULL, 0);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT] %s line %d fail in DivxDRMExecCmd(hInst: 0x%x, Code: %d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst, DIVXDRM_FINALIZE_PLAYBACK);
	}

	mrRet = DivxDRMDeInitSystem(prInst);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DivxDRMDeInitSystem(hInst: 0x%x), mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst, mrRet);
	}

	DIVXDRMINSTLOCK(prInst);

	prInst->u4Ref--;

	if (NULL != prInst->pvContext) {
		DMX_FreeMemory(prInst->pvContext);
		prInst->pvContext = NULL;
		prInst->u4ContextSz = 0;
	}

	DIVXDRMLOCK();

	if (NULL == prInstPrev)
		g_prDivxDRMInst = prInst->prNext;
	else
		prInstPrev->prNext = prInst->prNext;

	DIVXDRMUNLOCK();

	DIVXDRMINSTUNLOCK(prInst);

	DIVXDRMINSTLOCKDEINITEX(prInst, mrRet);

	DMX_FreeMemory(prInst);

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s(hInst: 0x%x) success\r\n"),
		DMX_FUNC_NAME, (u32)pvInst);

	MM_RETURN(RET_DMX_OK);
}

bool	DivxDRMIsInstValid(DECRYPT_DIVXDRM_INST_T *prInst)
{
	DECRYPT_DIVXDRM_INST_T	*prInstTmp = NULL;

	if (NULL == prInst) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return FALSE;
	}

	DIVXDRMLOCK();

	prInstTmp = g_prDivxDRMInst;

	while (NULL != prInstTmp) {
		if (prInstTmp == prInst)
			break;

		prInstTmp = prInstTmp->prNext;
	}

	if (NULL == prInstTmp) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for no corresponding ")
			TEXT("DivxDRM Instance handle(hInst: 0x%x)\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst);
		DIVXDRMUNLOCK();
		return FALSE;
	}

	DIVXDRMUNLOCK();

	return TRUE;
}

MRESULT DivxDRMSetRandomSample(
	DECRYPT_DIVXDRM_INST_T *prInst,
	u32				   u4Cnt)
{
	u32	u4Idx = 0;
	drmErrorCodes_t eErrCode = DRM_SUCCESS;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prInst) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid DivxDRM Instance Handle\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DIVXDRMINSTLOCK(prInst);

	if (0 == u4Cnt)
		u4Cnt = 1;

	prInst->u4Ref++;

	for (u4Idx = 0; u4Idx < u4Cnt; u4Idx++) {
		eErrCode = drmSetRandomSample(prInst->pvContext);
		if (DRM_SUCCESS != eErrCode) {
			mrRet = DivxDRMMapErrCode(eErrCode);
			prInst->u4Ref--;
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				(TEXT("[DECRYPT][DIVXDRM] %s fail in drmSetRandomSample, ")
				TEXT("hInst: 0x%x, u4Cnt: %d), mrRet: 0x%x\r\n")),
				DMX_FUNC_NAME, prInst, u4Cnt, mrRet);
			DIVXDRMINSTUNLOCK(prInst);
			MM_RETURN(mrRet);
		}
	}

	prInst->u4Ref--;

	DIVXDRMINSTUNLOCK(prInst);

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success, (hInst: 0x%x, u4Cnt: %d)\r\n"),
		DMX_FUNC_NAME, prInst, u4Cnt);

	MM_RETURN(mrRet);
}

MRESULT DivxDRMInitPlayback(
	DECRYPT_DIVXDRM_INST_T			 *prInst,
	DECRYPT_DIVXDRM_HEADER_T		 *prHdrInfo)
{
	drmErrorCodes_t eErrCode = DRM_SUCCESS;
	MRESULT mrRet	= RET_DMX_OK;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s enter, hInst: 0x%x\r\n"),
		DMX_FUNC_NAME, prInst);

	if ((NULL == prHdrInfo) ||
		(NULL == prInst)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if ((NULL == prHdrInfo->pu1DrmHdrData) ||
		(0 == prHdrInfo->u4DrmHdrSz) ||
		(prHdrInfo->u4DrmHdrSz < drmGetStrdSize())) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args, ")
			TEXT("u4DrmHdrSz: 0x%x, pu1DrmhdrData: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prHdrInfo->u4DrmHdrSz,
			prHdrInfo->pu1DrmHdrData);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DIVXDRMINSTLOCK(prInst);

	prInst->u4Ref++;

	if (DECRYPT_DIVXDRM_STATUS_SYSTEM_INITED != prInst->eStatus) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for err state(%d) ")
			TEXT("for call InitPlayback, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst->eStatus, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	if (prInst->rHeader.u4DrmHdrBufSz < prHdrInfo->u4DrmHdrSz) {
		if (prInst->rHeader.pu1DrmHdrBuf != NULL)
			DMX_FreeMemory(prInst->rHeader.pu1DrmHdrBuf);
		prInst->rHeader.pu1DrmHdrBuf = NULL;
		prInst->rHeader.u4DrmHdrBufSz = 0;

		DMX_NewMemory(prHdrInfo->u4DrmHdrSz, prInst->rHeader.pu1DrmHdrBuf);
		if (NULL == prInst->rHeader.pu1DrmHdrBuf) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[DECRYPT][DIVXDRM] %s line %d fail for no memory\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			prInst->u4Ref--;
			DIVXDRMINSTUNLOCK(prInst);
			MM_RETURN(RET_DMX_NO_MEM);
		}

		prInst->rHeader.u4DrmHdrBufSz = prHdrInfo->u4DrmHdrSz;
	}

	if (NULL == prInst->rHeader.pu1DrmHdrBuf) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for rHeader.pu1DrmHdrBuf==NULL\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	#ifdef __linux__
	dmx_memset(prInst->rHeader.pu1DrmHdrBuf, 0, prInst->rHeader.u4DrmHdrBufSz);

	if (0 != mm_copy_from_user(prInst->rHeader.pu1DrmHdrBuf, prHdrInfo->pu1DrmHdrData, prHdrInfo->u4DrmHdrSz)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d failed in mm_copy_from_user")
			TEXT("-- prHdrInfo->pu1DrmHdrData:0x%x\r\n")),
			prHdrInfo->pu1DrmHdrData);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}

	#else

	dmx_memset(prInst->rHeader.pu1DrmHdrBuf, 0, prInst->rHeader.u4DrmHdrBufSz);
	dmx_memcpy(prInst->rHeader.pu1DrmHdrBuf, prHdrInfo->pu1DrmHdrData, prHdrInfo->u4DrmHdrSz);

	#endif /*__linux__*/

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		(TEXT("[DECRYPT][DIVXDRM] %s line %d -- call drmInitPlayback, ")
		TEXT("pvContext: 0x%x, u4DrmHdrSz: %d, pu1DrmHdrData: 0x%x, hInst: 0x%x\r\n")),
		DMX_FUNC_NAME, DMX_LINE_NO, prInst->pvContext, prHdrInfo->u4DrmHdrSz,
		prInst->rHeader.pu1DrmHdrBuf, prInst);

	eErrCode = drmInitPlayback(prInst->pvContext, prInst->rHeader.pu1DrmHdrBuf);
	if (DRM_SUCCESS != eErrCode) {
		mrRet = DivxDRMMapErrCode(eErrCode);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmInitPlayback,")
			TEXT("eResult: %d, mrRet: 0x%x, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, eErrCode, mrRet, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(mrRet);
	}

	prInst->eStatus = DECRYPT_DIVXDRM_STATUS_FILE_INITED;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		(TEXT("[DECRYPT][DIVXDRM] %s line %d -- Set DivxDRM Instance(0x%x)'s")
		TEXT("state to be %d\r\n")),
		DMX_FUNC_NAME, DMX_LINE_NO, prInst, prInst->eStatus);

	prInst->u4Ref--;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success, hInst: 0x%x\r\n"),
		DMX_FUNC_NAME, prInst);

	DIVXDRMINSTUNLOCK(prInst);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMCommitPlayback(DECRYPT_DIVXDRM_INST_T  *prInst, s32 i4DecryptId)
{
	drmErrorCodes_t eResult  = DRM_SUCCESS;
	MRESULT mrRet	= RET_DMX_OK;

	if (NULL == prInst) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DIVXDRMINSTLOCK(prInst);

	prInst->u4Ref++;

	if (DECRYPT_DIVXDRM_STATUS_FILE_INITED != prInst->eStatus) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for err state(%d) to ")
			TEXT("Commit Playback, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst->eStatus, prInst);

		prInst->u4Ref--;

		DIVXDRMINSTUNLOCK(prInst);

		MM_RETURN(RET_DMX_ERR_STATE);
	}

	prInst->i4DecryptId = i4DecryptId;

	eResult = drmCommitPlayback(prInst->pvContext);

	if (DRM_SUCCESS != eResult) {
		mrRet = DivxDRMMapErrCode(eResult);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmCommitPlayback, ")
			TEXT("eResult: %d, mrRet: 0x%x, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);

		prInst->u4Ref--;

		DIVXDRMINSTUNLOCK(prInst);

		MM_RETURN(mrRet);
	}

	prInst->eStatus = DECRYPT_DIVXDRM_STATUS_FILE_PLAYING;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s line %d -- Set DivxDRM Instance(0x%x)'s state to be %d\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prInst, prInst->eStatus);

	prInst->u4Ref--;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success, hInst: 0x%x\r\n"),
		DMX_FUNC_NAME, prInst);

	DIVXDRMINSTUNLOCK(prInst);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMFinalizePlayback(DECRYPT_DIVXDRM_INST_T	*prInst)
{
	drmErrorCodes_t eResult  = DRM_SUCCESS;
	MRESULT mrRet	= RET_DMX_OK;

	if (NULL == prInst) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DIVXDRMINSTLOCK(prInst);

	prInst->u4Ref++;

	if ((DECRYPT_DIVXDRM_STATUS_SYSTEM_INITED != prInst->eStatus)  &&
		(DECRYPT_DIVXDRM_STATUS_FILE_INITED   != prInst->eStatus)  &&
		(DECRYPT_DIVXDRM_STATUS_FILE_PLAYING  != prInst->eStatus)) {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d exit, decrypt state(%d), hInst: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst->eStatus, prInst);

		prInst->u4Ref--;

		DIVXDRMINSTUNLOCK(prInst);

		MM_RETURN(RET_DMX_OK);
	}

	eResult = drmFinalizePlayback(prInst->pvContext);

	if (DRM_SUCCESS != eResult) {
		mrRet = DivxDRMMapErrCode(eResult);

		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmFinalizePlayback,")
			TEXT("eResult: %d, mrRet: %d, hInst: 0x%x)\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);

		prInst->u4Ref--;

		DIVXDRMINSTUNLOCK(prInst);

		MM_RETURN(mrRet);
	}

	prInst->eStatus = DECRYPT_DIVXDRM_STATUS_SYSTEM_INITED;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s line %d -- Set DivxDRM Instance(0x%x)'s state to be %d\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prInst, prInst->eStatus);

	prInst->u4Ref--;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success, hInst: 0x%x\r\n"),
		DMX_FUNC_NAME, prInst);

	DIVXDRMINSTUNLOCK(prInst);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMGetRentalStatus(
	DECRYPT_DIVXDRM_INST_T	*prInst,
	DECRYPT_DIVXDRM_RENTAL_STATUS_T  *prRentalStatus)
{
	drmErrorCodes_t eErrCode = DRM_SUCCESS;
	MRESULT mrRet	= RET_DMX_OK;

	if ((NULL == prInst) ||
		(NULL == prRentalStatus)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DIVXDRMINSTLOCK(prInst);

	prInst->u4Ref++;

	if (DECRYPT_DIVXDRM_STATUS_FILE_INITED != prInst->eStatus) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for err decrypt")
			TEXT("state(%d) to Commit Playback, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst->eStatus, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	eErrCode = drmQueryRentalStatus(prInst->pvContext,
		&(prRentalStatus->u1RentalMsgFlag),
		&(prRentalStatus->u1UseLimitCnt),
		&(prRentalStatus->u1UseCnt));
	if (DRM_SUCCESS != eErrCode) {
		mrRet = DivxDRMMapErrCode(eErrCode);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmQueryRentalStatus, ")
			TEXT("RentalMsgFlag: %u, UseLimitCnt: %u, UseCnt: %u\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO,
			prRentalStatus->u1RentalMsgFlag,
			prRentalStatus->u1UseLimitCnt,
			prRentalStatus->u1UseCnt);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmQueryRentalStatus, ")
			TEXT("eErrCode: %d, mrRet: 0x%x, hInst: 0x%x)\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, eErrCode, mrRet, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(mrRet);
	}

	prInst->u4Ref--;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		(TEXT("[DECRYPT][DIVXDRM] %s line %d -- RentalMsgFlag: 0x%02x, ")
		TEXT("UseLimitCnt: 0x%02x, UseCnt: 0x%02x, hInst: 0x%x\r\n")),
		DMX_FUNC_NAME, DMX_LINE_NO, prRentalStatus->u1RentalMsgFlag,
		prRentalStatus->u1UseLimitCnt, prRentalStatus->u1UseCnt, prInst);

	DIVXDRMINSTUNLOCK(prInst);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMQueryCgmsa(
	DECRYPT_DIVXDRM_INST_T	*prInst,
	u8 *pu1Cgmsa)
{
	drmErrorCodes_t eResult  = DRM_SUCCESS;
	MRESULT mrRet	= RET_DMX_OK;

	if ((NULL == pu1Cgmsa) ||
		(NULL == prInst)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DIVXDRMINSTLOCK(prInst);

	prInst->u4Ref++;

	if ((DECRYPT_DIVXDRM_STATUS_FILE_INITED  != prInst->eStatus) &&
		(DECRYPT_DIVXDRM_STATUS_FILE_PLAYING != prInst->eStatus)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for err decrypt state(%d) ")
			TEXT("to Query Cgmsa, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst->eStatus, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	eResult = drmQueryCgmsa(prInst->pvContext, pu1Cgmsa);
	if (DRM_SUCCESS != eResult) {
		mrRet = DivxDRMMapErrCode(eResult);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmQueryCgmsa, ")
			TEXT("eResult: %d, mrRet: 0x%x, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(mrRet);
	}

	prInst->u4Ref--;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success, hInst: 0x%x\r\n"),
		DMX_FUNC_NAME, prInst);

	DIVXDRMINSTUNLOCK(prInst);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMQueryAcptb(
	DECRYPT_DIVXDRM_INST_T	*prInst,
	u8  *pu1Acptb)
{
	drmErrorCodes_t eResult  = DRM_SUCCESS;
	MRESULT mrRet	= RET_DMX_OK;

	if ((NULL == pu1Acptb) ||
		(NULL == prInst)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DIVXDRMINSTLOCK(prInst);

	prInst->u4Ref++;

	if ((DECRYPT_DIVXDRM_STATUS_FILE_INITED  != prInst->eStatus) &&
		(DECRYPT_DIVXDRM_STATUS_FILE_PLAYING != prInst->eStatus)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for err decrypt state(%d) ")
			TEXT("to Query Acptb, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst->eStatus, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	eResult = drmQueryAcptb(prInst->pvContext, pu1Acptb);
	if (DRM_SUCCESS != eResult) {
		mrRet = DivxDRMMapErrCode(eResult);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmQueryAcptb, ")
			TEXT("eResult: %d, mrRet: 0x%x, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(mrRet);
	}

	prInst->u4Ref--;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success, hInst: 0x%x\r\n"),
		DMX_FUNC_NAME, prInst);

	DIVXDRMINSTUNLOCK(prInst);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMQueryDigitalProc(
	DECRYPT_DIVXDRM_INST_T	*prInst,
	u8  *pu1DigitalProc)
{
	drmErrorCodes_t eResult  = DRM_SUCCESS;
	MRESULT mrRet	= RET_DMX_OK;

	if ((NULL == pu1DigitalProc) ||
		(NULL == prInst)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DIVXDRMINSTLOCK(prInst);

	prInst->u4Ref++;

	if ((DECRYPT_DIVXDRM_STATUS_FILE_INITED  != prInst->eStatus) &&
		(DECRYPT_DIVXDRM_STATUS_FILE_PLAYING != prInst->eStatus)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for err decrypt state(%d)")
			TEXT("to Query Digital Proctection, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst->eStatus, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	eResult = drmQueryDigitalProtection(prInst->pvContext, pu1DigitalProc);
	if (DRM_SUCCESS != eResult) {
		mrRet = DivxDRMMapErrCode(eResult);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmQueryDigitalProtection, eResult: %d, mrRet: 0x%x, hInst: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(mrRet);
	}

	prInst->u4Ref--;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success, hInst: 0x%x\r\n"),
		DMX_FUNC_NAME, prInst);

	DIVXDRMINSTUNLOCK(prInst);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMQueryIct(
	DECRYPT_DIVXDRM_INST_T	*prInst,
	u8  *pu1Ict)
{
	drmErrorCodes_t eResult  = DRM_SUCCESS;
	MRESULT mrRet	= RET_DMX_OK;

	if ((NULL == pu1Ict) ||
		(NULL == prInst)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DIVXDRMINSTLOCK(prInst);

	prInst->u4Ref++;

	if ((DECRYPT_DIVXDRM_STATUS_FILE_INITED  != prInst->eStatus) &&
		(DECRYPT_DIVXDRM_STATUS_FILE_PLAYING != prInst->eStatus)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
TEXT("[DECRYPT][DIVXDRM] %s line %d fail for err decrypt state(%d) to Query ICT, hInst: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst->eStatus, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	eResult = drmQueryIct(prInst->pvContext, pu1Ict);
	if (DRM_SUCCESS != eResult) {
		mrRet = DivxDRMMapErrCode(eResult);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmQueryIct, eResult: %d, mrRet: 0x%x, hInst: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(mrRet);
	}

	prInst->u4Ref--;
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success, hInst: 0x%x\r\n"),
		DMX_FUNC_NAME, prInst);

	DIVXDRMINSTUNLOCK(prInst);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMGetRandomSampleCnt(
	DECRYPT_DIVXDRM_INST_T	*prInst,
	u32	*pu4Cnt)
{
	if ((NULL == pu4Cnt) ||
		(NULL == prInst)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DIVXDRMINSTLOCK(prInst);

	prInst->u4Ref++;

	if (DECRYPT_DIVXDRM_STATUS_SYSTEM_UNINIT == prInst->eStatus) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
TEXT("[DECRYPT][DIVXDRM] %s line %d fail for err decrypt state(%d) to Get Random Sample Count, hInst: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst->eStatus, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	*pu4Cnt = drmGetRandomSampleCounter(prInst->pvContext);

	prInst->u4Ref--;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success, Cnt : %d, hInst: 0x%x\r\n"),
		DMX_FUNC_NAME, *pu4Cnt, prInst);

	DIVXDRMINSTUNLOCK(prInst);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMGetRegCodeStr(
	DECRYPT_DIVXDRM_INST_T	*prInst,
	DECRYPT_DIVXDRM_CODE_STRING_T *prCodeString)
{
	drmErrorCodes_t eResult  = DRM_SUCCESS;
	MRESULT mrRet	= RET_DMX_OK;

	if ((NULL == prCodeString) ||
		(NULL == prInst)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DIVXDRMINSTLOCK(prInst);

	prInst->u4Ref++;

	if (DECRYPT_DIVXDRM_STATUS_SYSTEM_UNINIT == prInst->eStatus) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for err decrypt state(%d) ")
			TEXT("to Query Registeration Code String, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst->eStatus, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	#ifdef __linux__
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s line %d -- prCodeString->u4CodeStrSz: %ld\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prCodeString->u4CodeStrSz);

	{
		char szCodeStr[DRM_REGISTRATION_CODE_BYTES];

		mm_memset(szCodeStr, 0, sizeof(szCodeStr));

		eResult = drmGetRegistrationCodeString(prInst->pvContext, szCodeStr);
		if (DRM_SUCCESS != eResult) {
			mrRet = DivxDRMMapErrCode(eResult);
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmGetRegistrationCodeString,")
				TEXT(" eResult: %d, mrRet: 0x%x, hInst: 0x%x\r\n")),
				DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);
			prInst->u4Ref--;
			DIVXDRMINSTUNLOCK(prInst);
			MM_RETURN(mrRet);
		}

		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d -- 2 prCodeString->u4CodeStrSz: %ld,")
			TEXT(" szCodeStr: %s\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prCodeString->u4CodeStrSz, szCodeStr);

		if (prCodeString->u4CodeStrSz > DRM_REGISTRATION_CODE_BYTES) {
			szCodeStr[DRM_REGISTRATION_CODE_BYTES - 1] = '\0';
			if (0 != mm_copy_to_user(prCodeString->szCodeStr, szCodeStr, DRM_REGISTRATION_CODE_BYTES)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed in mm_copy_to_user ")
					TEXT(" -- prCodeString->szCodeStr:0x%x\r\n")),
					prCodeString->szCodeStr);
				prInst->u4Ref--;
				DIVXDRMINSTUNLOCK(prInst);
				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
			}
		} else {
			szCodeStr[prCodeString->u4CodeStrSz - 1] = '\0';
			if (0 != mm_copy_to_user(prCodeString->szCodeStr, szCodeStr, prCodeString->u4CodeStrSz)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed in mm_copy_to_user ")
					TEXT(" -- prCodeString->szCodeStr:0x%x\r\n")),
					prCodeString->szCodeStr);
				prInst->u4Ref--;
				DIVXDRMINSTUNLOCK(prInst);
				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
			}
		}
	}
	prInst->u4Ref--;
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success, hInst: 0x%x, RegCodeStr: %s\r\n"),
		DMX_FUNC_NAME, prInst, prCodeString->szCodeStr);

	#else
	eResult = drmGetRegistrationCodeString(prInst->pvContext, prCodeString->szCodeStr);
	if (DRM_SUCCESS != eResult) {
		mrRet = DivxDRMMapErrCode(eResult);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmGetRegistrationCodeString, ")
			TEXT("eResult: %d, mrRet: 0x%x, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(mrRet);
	}
	prInst->u4Ref--;
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success, hInst: 0x%x, RegCodeStr: %S\r\n"),
		DMX_FUNC_NAME, prInst, prCodeString->szCodeStr);

	#endif /*__linux__*/

	DIVXDRMINSTUNLOCK(prInst);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMGetDeactivationCodeStr(
	DECRYPT_DIVXDRM_INST_T	*prInst,
	DECRYPT_DIVXDRM_CODE_STRING_T *prCodeString)
{
	drmErrorCodes_t eResult  = DRM_SUCCESS;
	MRESULT mrRet	= RET_DMX_OK;

	if ((NULL == prCodeString) ||
		(NULL == prInst)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DIVXDRMINSTLOCK(prInst);

	prInst->u4Ref++;

	if (DECRYPT_DIVXDRM_STATUS_SYSTEM_UNINIT == prInst->eStatus) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for err decrypt state(%d) ")
			TEXT("to Query Acptb, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst->eStatus, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	#ifdef __linux__
	if (!access_ok(VERIFY_WRITE | VERIFY_READ, (void __user *)(prCodeString->szCodeStr),
		prCodeString->u4CodeStrSz)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d failed in access_ok return false")
			TEXT(" -- prCodeString->szCodeStr:0x%x\r\n")),
			prCodeString->szCodeStr);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}

	{
		char szCodeStr[DRM_REGISTRATION_CODE_BYTES];

		mm_memset(szCodeStr, 0, sizeof(szCodeStr));

		eResult = drmGetDeactivationCodeString(prInst->pvContext, szCodeStr);
		if (DRM_SUCCESS != eResult) {
			mrRet = DivxDRMMapErrCode(eResult);
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmGetDeactivationCodeString,")
				TEXT(" eResult: %d, mrRet: 0x%x, hInst: 0x%x\r\n")),
				DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);
			prInst->u4Ref--;
			DIVXDRMINSTUNLOCK(prInst);
			MM_RETURN(mrRet);
		}

		if (prCodeString->u4CodeStrSz > DRM_REGISTRATION_CODE_BYTES) {
			szCodeStr[DRM_REGISTRATION_CODE_BYTES - 1] = '\0';
			if (0 != mm_copy_to_user(prCodeString->szCodeStr, szCodeStr, DRM_REGISTRATION_CODE_BYTES)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed in mm_copy_to_user ")
					TEXT(" -- prCodeString->szCodeStr:0x%x\r\n")),
					prCodeString->szCodeStr);
				prInst->u4Ref--;
				DIVXDRMINSTUNLOCK(prInst);
				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
			}
		} else {
			szCodeStr[prCodeString->u4CodeStrSz - 1] = '\0';
			if (0 != mm_copy_to_user(prCodeString->szCodeStr, szCodeStr, prCodeString->u4CodeStrSz)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed in mm_copy_to_user -- ")
					TEXT(" prCodeString->szCodeStr:0x%x\r\n")),
					prCodeString->szCodeStr);
				prInst->u4Ref--;
				DIVXDRMINSTUNLOCK(prInst);
				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
			}
		}
	}

	prInst->u4Ref--;
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success, hInst: 0x%x, DeactiveCodeString: %s\r\n"),
		DMX_FUNC_NAME, prInst, prCodeString->szCodeStr);

	#else

	eResult = drmGetDeactivationCodeString(prInst->pvContext,
		prCodeString->szCodeStr);

	if (DRM_SUCCESS != eResult) {
		mrRet = DivxDRMMapErrCode(eResult);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmGetDeactivationCodeString,")
			TEXT(" eResult: %d, mrRet: 0x%x, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(mrRet);
	}

	prInst->u4Ref--;
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success, hInst: 0x%x, DeactiveCodeString: %S\r\n"),
		DMX_FUNC_NAME, prInst, prCodeString->szCodeStr);

	#endif /*__linux__*/

	DIVXDRMINSTUNLOCK(prInst);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMGetActivationMsg(
	DECRYPT_DIVXDRM_INST_T	*prInst,
	DECRYPT_DIVXDRM_MSG_T	*prMsg)
{
	drmErrorCodes_t eResult  = DRM_SUCCESS;
	MRESULT mrRet	= RET_DMX_OK;

	if ((NULL == prMsg) ||
		(NULL == prInst)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DIVXDRMINSTLOCK(prInst);

	prInst->u4Ref++;

	if (DECRYPT_DIVXDRM_STATUS_SYSTEM_UNINIT == prInst->eStatus) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for err decrypt state(%d) ")
			TEXT("to Query Acptb, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst->eStatus, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	#ifdef __linux__
	if (!access_ok(VERIFY_WRITE | VERIFY_READ, (void __user *)(prMsg->szMsgStr), prMsg->u4MsgStrSz)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d failed in access_ok return ")
			TEXT("false -- prMsg->szMsgStr:0x%x\r\n")),
			prMsg->szMsgStr);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}

	{
		char *szMsgStr = NULL;

		DMX_NewMemory(prMsg->u4MsgStrSz, szMsgStr);
		if (NULL == szMsgStr) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in alloc memory, ")
				TEXT("hInst: 0x%x\r\n")),
				DMX_FUNC_NAME, DMX_LINE_NO, prInst);
			prInst->u4Ref--;
			DIVXDRMINSTUNLOCK(prInst);
			MM_RETURN(RET_DMX_NO_MEM);
		}

		eResult = drmGetActivationMessage(prInst->pvContext,
			szMsgStr, (uint32_t *)(&(prMsg->u4MsgStrSz)));
		if (DRM_SUCCESS != eResult) {
			mrRet = DivxDRMMapErrCode(eResult);
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmGetActivationMessage,")
				TEXT("eResult: %d, mrRet: 0x%x, hInst: 0x%x\r\n")),
				DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);
			DMX_FreeMemory(szMsgStr);
			prInst->u4Ref--;
			DIVXDRMINSTUNLOCK(prInst);
			MM_RETURN(mrRet);
		}

		if (0 != mm_copy_to_user(prMsg->szMsgStr, szMsgStr, prMsg->u4MsgStrSz)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				(TEXT("[DECRYPT][DIVXDRM] %s line %d failed in mm_copy_to_user ")
				TEXT("-- prMsg->szMsgStr:0x%x\r\n")),
				prMsg->szMsgStr);
			DMX_FreeMemory(szMsgStr);
			prInst->u4Ref--;
			DIVXDRMINSTUNLOCK(prInst);
			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
		}

		DMX_FreeMemory(szMsgStr);
	}

	#else
	eResult = drmGetActivationMessage(prInst->pvContext,
		prMsg->szMsgStr, (uint32_t *)(&(prMsg->u4MsgStrSz)));
	if (DRM_SUCCESS != eResult) {
		mrRet = DivxDRMMapErrCode(eResult);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmGetActivationMessage,"
			TEXT("eResult: %d, mrRet: 0x%x, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(mrRet);
	}

	#endif /*__linux__*/

	prInst->u4Ref--;
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success, hInst: 0x%x\r\n"),
		DMX_FUNC_NAME, prInst);

	DIVXDRMINSTUNLOCK(prInst);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMGetDeActivationMsg(
	DECRYPT_DIVXDRM_INST_T	*prInst,
	DECRYPT_DIVXDRM_MSG_T	*prMsg)
{
	drmErrorCodes_t eResult  = DRM_SUCCESS;
	MRESULT mrRet	= RET_DMX_OK;

	if ((NULL == prMsg) ||
		(NULL == prInst)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DIVXDRMINSTLOCK(prInst);

	prInst->u4Ref++;

	if (DECRYPT_DIVXDRM_STATUS_SYSTEM_UNINIT == prInst->eStatus) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for err decrypt state(%d) ")
			TEXT("to Query Acptb, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst->eStatus, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	#ifdef __linux__
	if (!access_ok(VERIFY_WRITE | VERIFY_READ, (void __user *)(prMsg->szMsgStr), prMsg->u4MsgStrSz)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d failed in access_ok return false")
			TEXT("-- prMsg->szMsgStr:0x%x\r\n")),
			prMsg->szMsgStr);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}

	{
		char *szMsgStr = NULL;

		DMX_NewMemory(prMsg->u4MsgStrSz, szMsgStr);
		if (NULL == szMsgStr) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in alloc memory, ")
				TEXT("hInst: 0x%x\r\n")),
				DMX_FUNC_NAME, DMX_LINE_NO, prInst);
			prInst->u4Ref--;
			DIVXDRMINSTUNLOCK(prInst);
			MM_RETURN(RET_DMX_NO_MEM);
		}

		eResult = drmGetDeactivationMessage(prInst->pvContext,
			szMsgStr, (uint32_t *)(&(prMsg->u4MsgStrSz)));
		if (DRM_SUCCESS != eResult) {
			mrRet = DivxDRMMapErrCode(eResult);
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmGetDeactivationMessage,")
				TEXT("eResult: %d, mrRet: 0x%x, hInst: 0x%x\r\n")),
				DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);
			DMX_FreeMemory(szMsgStr);
			prInst->u4Ref--;
			DIVXDRMINSTUNLOCK(prInst);
			MM_RETURN(mrRet);
		}

		if (0 != mm_copy_to_user(prMsg->szMsgStr, szMsgStr, prMsg->u4MsgStrSz)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				(TEXT("[DECRYPT][DIVXDRM] %s line %d failed in mm_copy_to_user ")
				TEXT("-- prMsg->szMsgStr:0x%x\r\n")),
				prMsg->szMsgStr);
			DMX_FreeMemory(szMsgStr);
			prInst->u4Ref--;
			DIVXDRMINSTUNLOCK(prInst);
			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
		}

		DMX_FreeMemory(szMsgStr);
	}

	#else

	eResult = drmGetDeactivationMessage(prInst->pvContext,
		prMsg->szMsgStr, (uint32_t *)(&(prMsg->u4MsgStrSz)));
	if (DRM_SUCCESS != eResult) {
		mrRet = DivxDRMMapErrCode(eResult);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmGetDeactivationMessage, ")
			TEXT("eResult: %d, mrRet: 0x%x, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(mrRet);
	}

	#endif /*__linux__*/

	prInst->u4Ref--;
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success, hInst: 0x%x\r\n"),
		DMX_FUNC_NAME, prInst);

	DIVXDRMINSTUNLOCK(prInst);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMDecryptAudio(
	DECRYPT_DIVXDRM_INST_T	*prInst,
	DECRYPT_DIVXDRM_DECRYPT_DATA_T *prDecryptInfo)
{
	drmErrorCodes_t eResult  = DRM_SUCCESS;
	MRESULT mrRet	= RET_DMX_OK;

	if ((NULL == prInst) ||
		(NULL == prDecryptInfo) ||
		(NULL == prDecryptInfo->pu1FrameData) ||
		(0 == prDecryptInfo->u4FrameDataSz)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DIVXDRMINSTLOCK(prInst);

#if DMX_PRINT_DECRYPT_KEY_LOG
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s -- FrameBuffer: 0x%x, Size: %d, hInst: 0x%x\r\n"),
		DMX_FUNC_NAME, (u32)(prDecryptInfo->pu1FrameData), prDecryptInfo->u4FrameDataSz, prInst);
#endif /*DMX_PRINT_DECRYPT_KEY_LOG*/

	prInst->u4Ref++;

	if (DECRYPT_DIVXDRM_STATUS_FILE_PLAYING != prInst->eStatus) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for err decrypt state(%d) ")
			TEXT("to call drmDecryptAudio, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst->eStatus, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	eResult = drmDecryptAudio(prInst->pvContext, prDecryptInfo->pu1FrameData,
		prDecryptInfo->u4FrameDataSz);

	if (DRM_SUCCESS != eResult) {
		mrRet = DivxDRMMapErrCode(eResult);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmDecryptAudio, ")
			TEXT("eResult: %d, mrRet: 0x%x, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(mrRet);
	}

	prInst->u4Ref--;

	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success, hInst: 0x%x\r\n"),
		DMX_FUNC_NAME, prInst);

	DIVXDRMINSTUNLOCK(prInst);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMDecryptVideo(
	DECRYPT_DIVXDRM_INST_T	*prInst,
	DECRYPT_DIVXDRM_DECRYPT_DATA_T *prDecryptInfo)
{
	drmErrorCodes_t eResult  = DRM_SUCCESS;
	MRESULT mrRet	= RET_DMX_OK;

	if ((NULL == prInst) ||
		(NULL == prDecryptInfo) ||
		(NULL == prDecryptInfo->pu1DrmInfo) ||
		(NULL == prDecryptInfo->pu1FrameData) ||
		(0 == prDecryptInfo->u4FrameDataSz)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DIVXDRMINSTLOCK(prInst);

#if DMX_PRINT_DECRYPT_KEY_LOG
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s -- FrameBuffer: 0x%x, Size: %d, hInst: 0x%x\r\n"),
		DMX_FUNC_NAME, (u32)(prDecryptInfo->pu1FrameData), prDecryptInfo->u4FrameDataSz, prInst);
#endif /*DMX_PRINT_DECRYPT_KEY_LOG*/

	prInst->u4Ref++;

	if (DECRYPT_DIVXDRM_STATUS_FILE_PLAYING != prInst->eStatus) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for err decrypt state(%d) ")
			TEXT("to call drmDecryptVideo, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst->eStatus, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	eResult = drmDecryptVideo(prInst->pvContext, prDecryptInfo->pu1FrameData,
		prDecryptInfo->u4FrameDataSz, prDecryptInfo->pu1DrmInfo);

	if (DRM_SUCCESS != eResult) {
		mrRet = DivxDRMMapErrCode(eResult);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmDecryptAudio, ")
			TEXT("eResult: %d, mrRet: 0x%x, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(mrRet);
	}

	prInst->u4Ref--;

	DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success, hInst: 0x%x\r\n"),
		DMX_FUNC_NAME, prInst);

	DIVXDRMINSTUNLOCK(prInst);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMGetVideoKeyInfo(
	DECRYPT_DIVXDRM_INST_T	*prInst,
	u16 u2KeyIndex,
	DECRYPT_DIVXDRM_VIDEO_KEYINFO_T *prKeyInfo)
{
	drmAesMode_t eAesMode = DRM_AESMODE_ECB;
	drmErrorCodes_t eResult  = DRM_SUCCESS;
	MRESULT mrRet	= RET_DMX_OK;

	if ((NULL == prInst) ||
		(NULL == prKeyInfo)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DIVXDRMINSTLOCK(prInst);

#if DMX_PRINT_DECRYPT_KEY_LOG
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s -- Video, KeyIdx: 0x%04x, hInst: 0x%x\r\n"),
		DMX_FUNC_NAME, u2KeyIndex, prInst);
#endif /*DMX_PRINT_DECRYPT_KEY_LOG*/

	prInst->u4Ref++;

	if (DECRYPT_DIVXDRM_STATUS_FILE_PLAYING != prInst->eStatus) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for err decrypt state(%d) ")
			TEXT("to call drmDecryptVideo, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst->eStatus, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	eResult = drmGetAesMode(prInst->pvContext, &eAesMode);
	if (DRM_SUCCESS != eResult) {
		mrRet = DivxDRMMapErrCode(eResult);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmGetAesMode, ")
			TEXT("eResult: %d, mrRet: 0x%x, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(mrRet);
	}

	prKeyInfo->fgCBC = FALSE;
	if (DRM_AESMODE_CBC == eAesMode)
		prKeyInfo->fgCBC = TRUE;

	eResult = drmGetFrameKey(prInst->pvContext, u2KeyIndex,
		&(prKeyInfo->pu1Key), &(prKeyInfo->u2KeyLen));

	if (DRM_SUCCESS != eResult) {
		mrRet = DivxDRMMapErrCode(eResult);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmGetFrameKey, eResult: %d")
			TEXT("mrRet: 0x%x, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(mrRet);
	}

	if ((16 != prKeyInfo->u2KeyLen) &&
		(24 != prKeyInfo->u2KeyLen) &&
		(32 != prKeyInfo->u2KeyLen)) {
		mrRet = RET_DMX_UNEXPECT;
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmGetFrameKey, eResult: %d,")
			TEXT("mrRet: 0x%x, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(mrRet);
	}

	prInst->u4Ref--;

#if DMX_PRINT_DECRYPT_KEY_LOG
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success, hInst: 0x%x, u2KeyLen: %d\r\n"),
		DMX_FUNC_NAME, prInst, prKeyInfo->u2KeyLen);
#endif /*DMX_PRINT_DECRYPT_KEY_LOG*/

	DIVXDRMINSTUNLOCK(prInst);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMGetAudioKeyInfo(
	DECRYPT_DIVXDRM_INST_T	*prInst,
	DECRYPT_DIVXDRM_AUDIO_KEYINFO_T *prKeyInfo)
{
	drmAesMode_t eAesMode = DRM_AESMODE_ECB;
	drmErrorCodes_t eResult  = DRM_SUCCESS;
	MRESULT mrRet	= RET_DMX_OK;

	if ((NULL == prInst) ||
		(NULL == prKeyInfo)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DIVXDRMINSTLOCK(prInst);

#if DMX_PRINT_DECRYPT_KEY_LOG
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s -- Audio, hInst: 0x%x\r\n"),
		DMX_FUNC_NAME, prInst);
#endif /*DMX_PRINT_DECRYPT_KEY_LOG*/

	prInst->u4Ref++;

	if (DECRYPT_DIVXDRM_STATUS_FILE_PLAYING != prInst->eStatus) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for err decrypt state(%d) ")
			TEXT("to call drmDecryptVideo, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, prInst->eStatus, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	eResult = drmGetAesMode(prInst->pvContext, &eAesMode);
	if (DRM_SUCCESS != eResult) {
		mrRet = DivxDRMMapErrCode(eResult);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmGetAesMode, eResult: %d,")
			TEXT("mrRet: 0x%x, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(mrRet);
	}

	prKeyInfo->fgCBC = FALSE;
	if (DRM_AESMODE_CBC == eAesMode)
		prKeyInfo->fgCBC = TRUE;

	eResult = drmGetAudioCryptoInfo(prInst->pvContext,
		&(prKeyInfo->pu1Key),
		&(prKeyInfo->u2KeyLen),
		&(prKeyInfo->u1ProtectOffset),
		&(prKeyInfo->u1ProtectSize));

	if (DRM_SUCCESS != eResult) {
		mrRet = DivxDRMMapErrCode(eResult);
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmGetAudioCryptoInfo,")
			TEXT(" eResult: %d, mrRet: 0x%x, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(mrRet);
	}

	if ((16 != prKeyInfo->u2KeyLen) &&
		(24 != prKeyInfo->u2KeyLen) &&
		(32 != prKeyInfo->u2KeyLen) &&
		(0 != prKeyInfo->u2KeyLen)) {
		mrRet = RET_DMX_UNEXPECT;
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmGetFrameKey, eResult: %d, ")
			TEXT("mrRet: 0x%x, hInst: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet, prInst);
		prInst->u4Ref--;
		DIVXDRMINSTUNLOCK(prInst);
		MM_RETURN(mrRet);
	}

	prInst->u4Ref--;

#if DMX_PRINT_DECRYPT_KEY_LOG
	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success, hInst: 0x%x, u2KeyLen: %d\r\n"),
		DMX_FUNC_NAME, prInst, prKeyInfo->u2KeyLen);
#endif /*DMX_PRINT_DECRYPT_KEY_LOG*/

	DIVXDRMINSTUNLOCK(prInst);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DivxDRMGetActivationStatus(void *pvActStatus, u32 u4StatusSz)
{
	DECRYPT_DIVXDRM_ACT_STATUS_T *prActStatus = NULL;
	u32	u4UsrIdBufSz = 0;
	void	*pvMarshalledMem = NULL;
#ifndef __linux__
	HRESULT hr = S_OK;
#endif /*__linux__*/
	drmErrorCodes_t eResult  = DRM_SUCCESS;
	MRESULT mrRet	= RET_DMX_OK;

	if ((NULL == pvActStatus) ||
		(sizeof(DECRYPT_DIVXDRM_ACT_STATUS_T) != u4StatusSz)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prActStatus = (DECRYPT_DIVXDRM_ACT_STATUS_T *)pvActStatus;

	u4UsrIdBufSz = prActStatus->u4UsrIdSz;

#ifndef __linux__
	hr = CeOpenCallerBuffer(&pvMarshalledMem,
		(void *)(prActStatus->pu1UsrId),
		u4UsrIdBufSz, ARG_IO_PTR, FALSE);

	if (FAILED(hr)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in CeOpenCallerBuffer")
			TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), Err: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO,
			pvMarshalledMem, (u32)(prActStatus->pu1UsrId),
			u4UsrIdBufSz, DMX_GET_LASTERR);
		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}
#else
	pvMarshalledMem = prActStatus->pu1UsrId;
#endif /*__linux__*/

	eResult = drmGetActivationStatus(pvMarshalledMem,
				(uint32_t *)(&(prActStatus->u4UsrIdSz)));
	if (DRM_SUCCESS != eResult) {
		mrRet = DivxDRMMapErrCode(eResult);

		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmGetActivationStatus, ")
			TEXT("eResult: %d, mrRet: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet);

#ifndef __linux__
		if (NULL != pvMarshalledMem) {
			hr = CeCloseCallerBuffer(pvMarshalledMem,
				(void *)(prActStatus->pu1UsrId),
				u4UsrIdBufSz, ARG_IO_PTR);

			if (FAILED(hr)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in CeCloseCallerBuffer")
					TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), Err: 0x%x\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO,
					pvMarshalledMem, (u32)(prActStatus->pu1UsrId),
					u4UsrIdBufSz, DMX_GET_LASTERR);
			}
		}
#endif /*__linux__*/

		MM_RETURN(mrRet);
	}

#ifndef __linux__
	if (NULL != pvMarshalledMem) {
		hr = CeCloseCallerBuffer(pvMarshalledMem,
			(void *)(prActStatus->pu1UsrId),
			u4UsrIdBufSz, ARG_IO_PTR);

		if (FAILED(hr)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in CeCloseCallerBuffer")
				TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), Err: 0x%x\r\n")),
				DMX_FUNC_NAME, DMX_LINE_NO,
				pvMarshalledMem, (u32)(prActStatus->pu1UsrId),
				u4UsrIdBufSz, DMX_GET_LASTERR);
			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
		}
	}
#endif /*__linux__*/

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success\r\n"),
		DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

bool DivxDRMIsDeviceActived(void)
{
	bool   fgIsActived = FALSE;
	int8_t i1Result = DRM_DEVICE_NOT_ACTIVATED;

	i1Result = drmIsDeviceActivated();

	if (DRM_DEVICE_IS_ACTIVATED == i1Result)
		fgIsActived = TRUE;
	else
		fgIsActived = FALSE;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s success, fgIsActived: %s\r\n"),
		DMX_FUNC_NAME, (fgIsActived ? TEXT("ACTIVED") : TEXT("ACTIVED")));

	return fgIsActived;
}

MRESULT DivxDRMGetVersion(void *pvVersionInfo, u32 u4InfoBufLen)
{
	DivXVersion rDivXVersion;
	DECRYPT_DIVXDRM_VERSION_INFO_T *prVersion;
	void	 *pvMarshalledMem1 = NULL, *pvMarshalledMem2 = NULL;
	u32	 u4Size1 = 0, u4Size2 = 0;
	drmErrorCodes_t eResult  = DRM_SUCCESS;
	MRESULT  mrRet	 = RET_DMX_OK;
#ifndef __linux__
	HRESULT  hr = S_OK;
#endif /*__linux__*/
	if ((sizeof(DECRYPT_DIVXDRM_VERSION_INFO_T) != u4InfoBufLen) ||
		(NULL == pvVersionInfo)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid input params\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prVersion = (DECRYPT_DIVXDRM_VERSION_INFO_T *)pvVersionInfo;

	u4Size1 = prVersion->u4CodeNameSz;
	u4Size2 = prVersion->u4OfficialNameSz;

#ifndef __linux__
	hr = CeOpenCallerBuffer(&pvMarshalledMem1,
		(void *)(prVersion->szCodeName),
		u4Size1, ARG_IO_PTR, FALSE);

	if (FAILED(hr)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in CeOpenCallerBuffer")
			TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), Err: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO,
			pvMarshalledMem1, (u32)(prVersion->szCodeName),
			u4Size1, DMX_GET_LASTERR);

		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}

	hr = CeOpenCallerBuffer(&pvMarshalledMem2,
		(void *)(prVersion->szOfficialName),
		u4Size2, ARG_IO_PTR, FALSE);

	if (FAILED(hr)) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in CeOpenCallerBuffer")
			TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), Err: 0x%x\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO,
			pvMarshalledMem1, (u32)(prVersion->szOfficialName),
			u4Size2, DMX_GET_LASTERR);

		hr = CeCloseCallerBuffer(pvMarshalledMem1,
			(void *)(prVersion->szCodeName),
			u4Size1, ARG_IO_PTR);

		if (FAILED(hr)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in CeCloseCallerBuffer")
				TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), Err: 0x%x\r\n")),
				DMX_FUNC_NAME, DMX_LINE_NO,
				pvMarshalledMem1, (u32)(prVersion->szCodeName),
				u4Size1, DMX_GET_LASTERR);
		}

		MM_RETURN(RET_DMX_OS_OPERA_FAIL);
	}
#else /*__linux__*/
	pvMarshalledMem1 = prVersion->szCodeName;
	pvMarshalledMem2 = prVersion->szOfficialName;
#endif /*__linux__*/

	mm_memset(&rDivXVersion, 0, sizeof(DivXVersion));

	eResult = drmGetVersion(&rDivXVersion);
	if (DIVXVERSION_OK != eResult) {
		switch (eResult) {
		case DIVXVERSION_INVALID_LABEL:
			mrRet = RET_DMX_PARAM_WRONG;
			break;
		case DIVXVERSION_INVALID_PARAM:
			mrRet = RET_DMX_PARAM_WRONG;
			break;
		case DIVXVERSION_UNKNOWN_ERROR:
		default:
			mrRet = RET_DMX_UNEXPECT;
			break;
		}

		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in drmGetVersion ")
			TEXT("(eResult: %d, mrRet: 0x%x)\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, eResult, mrRet);
#ifndef __linux__
		if (NULL != pvMarshalledMem1) {
			hr = CeCloseCallerBuffer(pvMarshalledMem1,
				(void *)(prVersion->szCodeName),
				u4Size1, ARG_IO_PTR);

			if (FAILED(hr)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in CeCloseCallerBuffer")
					TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), Err: 0x%x\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO,
					pvMarshalledMem1, (u32)(prVersion->szCodeName),
					u4Size1, DMX_GET_LASTERR);
			}
		}

		if (NULL != pvMarshalledMem2) {
			hr = CeCloseCallerBuffer(pvMarshalledMem2,
				(void *)(prVersion->szOfficialName),
				u4Size2, ARG_IO_PTR);

			if (FAILED(hr)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in CeCloseCallerBuffer")
					TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), Err: 0x%x\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO,
					pvMarshalledMem2, (u32)(prVersion->szOfficialName),
					u4Size2, DMX_GET_LASTERR);
			}
		}
#endif /*__linux__*/

		MM_RETURN(mrRet);
	}

	if (NULL != pvMarshalledMem1) {
		mm_memset(pvMarshalledMem1, 0, u4Size1);

		if (NULL != rDivXVersion.codeName) {
			strncpy((char *)pvMarshalledMem1, rDivXVersion.codeName,
				(u4Size1 - 1));
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[DECRYPT][DIVXDRM] %s -- szCodeName: %s\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, (char *)pvMarshalledMem1);
		}
	}

	if (NULL != pvMarshalledMem2) {
		mm_memset(pvMarshalledMem2, 0, u4Size2);

		if (NULL != rDivXVersion.officialName) {
			strncpy((char *)pvMarshalledMem2, rDivXVersion.officialName,
				(u4Size2 - 1));
			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				TEXT("[DECRYPT][DIVXDRM] %s -- szOfficialName: %s\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, (char *)pvMarshalledMem2);
		}
	}

	prVersion->u4Major = rDivXVersion.major;
	prVersion->u4Minor = rDivXVersion.minor;
	prVersion->u4Fix   = rDivXVersion.fix;
	prVersion->u4Build = rDivXVersion.build;

	DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
		TEXT("[DECRYPT][DIVXDRM] %s -- Major: %d, Minor: %d, Fix: %d, Build: %d\r\n"),
		DMX_FUNC_NAME, prVersion->u4Major, prVersion->u4Minor,
		prVersion->u4Fix, prVersion->u4Build);
#ifndef __linux__
	if (NULL != pvMarshalledMem1) {
		hr = CeCloseCallerBuffer(pvMarshalledMem1,
			(void *)(prVersion->szCodeName),
			u4Size1, ARG_IO_PTR);

		if (FAILED(hr)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in CeCloseCallerBuffer")
				TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), Err: 0x%x\r\n")),
				DMX_FUNC_NAME, DMX_LINE_NO,
				pvMarshalledMem1, (u32)(prVersion->szCodeName),
				u4Size1, DMX_GET_LASTERR);
			mrRet = RET_DMX_OS_OPERA_FAIL;
		}
	}

	if (NULL != pvMarshalledMem2) {
		hr = CeCloseCallerBuffer(pvMarshalledMem2,
			(void *)(prVersion->szOfficialName),
			u4Size2, ARG_IO_PTR);

		if (FAILED(hr)) {
			DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in CeCloseCallerBuffer")
				TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), Err: 0x%x\r\n")),
				DMX_FUNC_NAME, DMX_LINE_NO,
				pvMarshalledMem2, (u32)(prVersion->szOfficialName),
				u4Size2, DMX_GET_LASTERR);
			mrRet = RET_DMX_OS_OPERA_FAIL;
		}
	}
#endif /*__linux__*/

	MM_RETURN(mrRet);
}

MRESULT DivxDRMInitMemory(void)
{
	drmErrorCodes_t eErrCode = DRM_SUCCESS;
	MRESULT mrRet = RET_DMX_OK;
	u8	au1FragData[DMX_DIVXDRM_MEMORY_LEN] = {0};
	u32	u4FragLen = 0;

	u4FragLen = MetaZone_ReadBinary(MZ_DRM_INFO_IDX_START, au1FragData, DMX_DIVXDRM_MEMORY_LEN);
	if (0 < u4FragLen) {
		DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d -- DRM Memory has been Initialized\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_OK);
	}

	eErrCode = drmInitDrmMemory();

	if (DRM_SUCCESS != eErrCode) {
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			TEXT("[DECRYPT][DIVXDRM] %s line %d failed in DivxDRMInitMemory\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		mrRet = RET_DMX_CPSA_NEVER_AUTHED;
	}

	MM_RETURN(mrRet);
}

MRESULT DivxDRMExecCmd(
	void	 *pvInst,
	u32	 u4Code,
	void	 *pvBufIn,
	u32	 u4LenIn,
	void	 *pvBufOut,
	u32	 u4LenOut)
{
	DECRYPT_DIVXDRM_INST_T *prInst	= (DECRYPT_DIVXDRM_INST_T *)pvInst;
	MRESULT mrRet	= RET_DMX_OK;

	switch (u4Code) {
	case DIVXDRM_INIT_SYSTEM:
		{
			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid ")
					TEXT("DivxDRM Instance (hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mrRet = DivxDRMInitSystem(prInst);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DivxDRMInitSystem ")
					TEXT("(mrRet: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}
		}
		break;
	case DIVXDRM_SET_RANDOM_SAMPLE:
		{
			u32 u4RndCnt = DECRYPT_DIVXDRM_SET_RND_SAMPLE_CNT;

			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				(TEXT("[DECRYPT][DIVXDRM] %s line %d(SET_RANDOM_SAMPLE) -- ")
				TEXT("pvBufIn: 0x%x, u4LenIn: %d\r\n")),
				DMX_FUNC_NAME, DMX_LINE_NO, (u32)pvBufIn, u4LenIn);

			if ((NULL != pvBufIn) &&
				(sizeof(u32) == u4LenIn)) {
				#ifdef __linux__
				if (0 != mm_copy_from_user(&u4RndCnt, pvBufIn, sizeof(u32))) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
						(TEXT("[DECRYPT][DIVXDRM] %s line %d failed in mm_copy_from_user")
						TEXT("-- pvBufIn:0x%x\r\n")),
						pvBufIn);
					MM_RETURN(RET_DMX_OS_OPERA_FAIL);
				}

				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d -- u4RndCnt: 0x%x, ")
					TEXT("*(u32 *)pvBufIn: 0x%x\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, u4RndCnt, *(u32 *)pvBufIn);
				#else
				u4RndCnt = *(u32 *)pvBufIn;
				#endif /*__linux__*/
			}

			if (0 == u4RndCnt)
				u4RndCnt = 1;
			else if (4 < u4RndCnt)
				u4RndCnt = 4;

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid ")
					TEXT("DivxDRM Instance (hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mrRet = DivxDRMSetRandomSample(prInst, u4RndCnt);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DivxDRMSetRandomSample,")
					TEXT("mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}
		}
		break;
	case DIVXDRM_INIT_PLAYBACK:
		{
			DECRYPT_DIVXDRM_HEADER_T		 *prHdrInfo;

#ifdef __linux__
			DECRYPT_DIVXDRM_HEADER_T rHdrInfo;

			if ((NULL == pvBufIn) ||
				(sizeof(DECRYPT_DIVXDRM_HEADER_T) != u4LenIn)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid input ")
					TEXT("params (eCode: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			prHdrInfo = (DECRYPT_DIVXDRM_HEADER_T *)pvBufIn;

			mm_memset(&rHdrInfo, 0, sizeof(DECRYPT_DIVXDRM_HEADER_T));
			if (0 != mm_copy_from_user(&rHdrInfo, pvBufIn, sizeof(DECRYPT_DIVXDRM_HEADER_T))) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed in mm_copy_from_user ")
					TEXT("-- pvBufIn:0x%x\r\n")),
					pvBufIn);
				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
			}

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid DivxDRM ")
					TEXT("Instance (hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mrRet = DivxDRMInitPlayback(prInst, &rHdrInfo);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DivxDRMInitPlayback, ")
					TEXT("mrRet: 0x%x\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			}
#else
			void *pvMarshalledMem = NULL;
			HRESULT hr = S_OK;

			prHdrInfo = (DECRYPT_DIVXDRM_HEADER_T *)pvBufIn;

			if ((NULL == prHdrInfo) ||
				(sizeof(DECRYPT_DIVXDRM_HEADER_T) != u4LenIn) ||
				(NULL == prHdrInfo->pu1DrmHdrData) ||
				(0 == prHdrInfo->u4DrmHdrSz)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid input ")
					TEXT("params (eCode: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid ")
					TEXT("DivxDRM Instance (hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			hr = CeOpenCallerBuffer(&pvMarshalledMem,
				(void *)(prHdrInfo->pu1DrmHdrData),
				prHdrInfo->u4DrmHdrSz, ARG_I_PTR, FALSE);

			if (FAILED(hr)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in CeOpenCallerBuffer")
					TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), Err: 0x%x\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO,
					pvMarshalledMem, (u32)(prHdrInfo->pu1DrmHdrData),
					prHdrInfo->u4DrmHdrSz, DMX_GET_LASTERR);
				mrRet = RET_DMX_OS_OPERA_FAIL;
				break;
			}

			mrRet = DivxDRMInitPlayback(prInst, prHdrInfo);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DivxDRMInitPlayback, ")
					TEXT("mrRet: 0x%x\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			}
			if (NULL != pvMarshalledMem) {
				hr = CeCloseCallerBuffer(pvMarshalledMem,
					(void *)(prHdrInfo->pu1DrmHdrData),
					prHdrInfo->u4DrmHdrSz, ARG_I_PTR);

				if (FAILED(hr)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
						(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in CeCloseCallerBuffer")
						TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), Err: 0x%x\r\n")),
						DMX_FUNC_NAME, DMX_LINE_NO,
						pvMarshalledMem, prHdrInfo->pu1DrmHdrData,
						prHdrInfo->u4DrmHdrSz, DMX_GET_LASTERR);
					mrRet = RET_DMX_OS_OPERA_FAIL;
					break;
				}
			}
#endif /*__linux__*/
		}
		break;

	case DIVXDRM_COMMIT_PLAYBACK:
		{
			s32 i4DecryptId = DECRYPT_PLAY_INVALID_ID;

			if (NULL != pvBufIn) {
				#ifdef __linux__
				if (0 != mm_copy_from_user(&i4DecryptId, pvBufIn, sizeof(s32))) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
						(TEXT("[DECRYPT][DIVXDRM] %s line %d failed in mm_copy_from_user ")
						TEXT("-- pvBufIn:0x%x\r\n")),
						pvBufIn);
					MM_RETURN(RET_DMX_OS_OPERA_FAIL);
				}

				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d -- i4DecryptId: 0x%x, ")
					TEXT("*((s32 *)pvBufIn): 0x%x\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, i4DecryptId, *((s32 *)pvBufIn));
				#else
				i4DecryptId = *((s32 *)pvBufIn);
				#endif /*__linux__*/

				DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d -- DIVXDRM_COMMIT_PLAYBACK ")
					TEXT("(hInst: 0x%x, i4DecryptId: %d)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst, i4DecryptId);
			}

			DmxLogT(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
				(TEXT("[DECRYPT][DIVXDRM] %s line %d -- DIVXDRM_COMMIT_PLAYBACK ")
				TEXT("(hInst: 0x%x, i4DecryptId: %d)\r\n")),
				DMX_FUNC_NAME, DMX_LINE_NO, prInst, i4DecryptId);

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid DivxDRM Instance ")
					TEXT("(hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mrRet = DivxDRMCommitPlayback(prInst, i4DecryptId);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DivxDRMCommitPlayback ")
					TEXT("(mrRet: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}
		}
		break;
	case DIVXDRM_FINALIZE_PLAYBACK:
		{
			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid DivxDRM ")
					TEXT("Instance (hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mrRet = DivxDRMFinalizePlayback(prInst);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DivxDRMCommitPlayback ")
					TEXT("(mrRet: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}
		}
		break;
	case DIVXDRM_QUERY_RENTAL_STATUS:
		{
			DECRYPT_DIVXDRM_RENTAL_STATUS_T  *prRentalStatus =
				(DECRYPT_DIVXDRM_RENTAL_STATUS_T *)pvBufOut;

			if ((sizeof(DECRYPT_DIVXDRM_RENTAL_STATUS_T) != u4LenOut) ||
				(NULL == prRentalStatus)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid input params ")
					TEXT("(eCode: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid DivxDRM ")
					TEXT("Instance (hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mrRet = DivxDRMGetRentalStatus(prInst, prRentalStatus);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DivxDRMGetRentalStatus ")
					TEXT("(mrRet: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}
		}
		break;
	case DIVXDRM_QUERY_CGMSA:
		{
			u8 *pu1Cgmsa = (u8 *)pvBufOut;

			if ((sizeof(u8) != u4LenOut) ||
				(NULL == pu1Cgmsa)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid input ")
					TEXT("params (eCode: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid ")
					TEXT("DivxDRM Instance (hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mrRet = DivxDRMQueryCgmsa(prInst, pu1Cgmsa);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DivxDRMQueryCgmsa ")
					TEXT("(mrRet: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}
		}
		break;
	case DIVXDRM_QUERY_ACPTB:
		{
			u8 *pu1Acptb = (u8 *)pvBufOut;

			if ((sizeof(u8) != u4LenOut) ||
				(NULL == pu1Acptb)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid input ")
					TEXT("params (eCode: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid DivxDRM ")
					TEXT("Instance (hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mrRet = DivxDRMQueryAcptb(prInst, pu1Acptb);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DivxDRMQueryAcptb ")
					TEXT("(mrRet: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}
		}
		break;
	case DIVXDRM_QUERY_DIGITAL_PROTECTION:
		{
			u8 *pu1DigitalProc = (u8 *)pvBufOut;

			if ((sizeof(u8) != u4LenOut) ||
				(NULL == pu1DigitalProc)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid input ")
					TEXT("params (eCode: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid DivxDRM Instance ")
					TEXT("(hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mrRet = DivxDRMQueryDigitalProc(prInst, pu1DigitalProc);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DivxDRMQueryDigitalProc ")
					TEXT("(mrRet: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}
		}
		break;
	case DIVXDRM_QUERY_ICT:
		{
			u8 *pu1Ict = (u8 *)pvBufOut;

			if ((sizeof(u8) != u4LenOut) ||
				(NULL == pu1Ict)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid input ")
					TEXT("params (eCode: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid ")
					TEXT("DivxDRM Instance (hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mrRet = DivxDRMQueryIct(prInst, pu1Ict);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DivxDRMQueryIct ")
					TEXT("(mrRet: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}
		}
		break;

	case DIVXDRM_GET_RANDOM_SAMPLE_CNTER:
		{
			u32 *pu4RndCnt = (u32 *)pvBufOut;

			if (NULL == pu4RndCnt) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid input ")
					TEXT("params (eCode: 0x%x)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid DivxDRM ")
					TEXT("Instance (hInst: 0x%x)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mrRet = DivxDRMGetRandomSampleCnt(prInst, pu4RndCnt);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DivxDRMGetRandomSampleCnt ")
					TEXT("(mrRet: 0x%x)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}
		}
		break;

	case DIVXDRM_GET_REG_CODE_STR:
		{
			DECRYPT_DIVXDRM_CODE_STRING_T *prCodeString = (DECRYPT_DIVXDRM_CODE_STRING_T *)pvBufOut;
#ifdef __linux__
			if (NULL == pvBufOut) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid input ")
					TEXT("(params eCode: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid ")
					TEXT("(DivxDRM Instance hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mrRet = DivxDRMGetRegCodeStr(prInst, prCodeString);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in ")
					TEXT("DivxDRMGetRegCodeStr (mrRet: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			}
#else
			void	*pvMarshalledMem = NULL;
			u32	u4CodeStrSz = 0;
			HRESULT hr = S_OK;

			if (NULL == prCodeString) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid input ")
					TEXT("params (eCode: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid ")
					TEXT("DivxDRM Instance (hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			u4CodeStrSz = prCodeString->u4CodeStrSz;
			hr = CeOpenCallerBuffer(&pvMarshalledMem,
				(void *)(prCodeString->szCodeStr),
				u4CodeStrSz, ARG_IO_PTR, FALSE);

			if (FAILED(hr)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in CeOpenCallerBuffer")
					TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), Err: 0x%x\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO,
					pvMarshalledMem, (u32)(prCodeString->szCodeStr),
					u4CodeStrSz, DMX_GET_LASTERR);
				mrRet = RET_DMX_OS_OPERA_FAIL;
				break;
			}

			mrRet = DivxDRMGetRegCodeStr(prInst, prCodeString);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DivxDRMGetRegCodeStr")
					TEXT(" (mrRet: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			}

			if (NULL != pvMarshalledMem) {
				hr = CeCloseCallerBuffer(pvMarshalledMem,
					(void *)(prCodeString->szCodeStr),
					u4CodeStrSz, ARG_IO_PTR);

				if (FAILED(hr)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
						(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in CeCloseCallerBuffer")
						TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), Err: 0x%x\r\n")),
						DMX_FUNC_NAME, DMX_LINE_NO,
						pvMarshalledMem, (u32)(prCodeString->szCodeStr),
						u4CodeStrSz, DMX_GET_LASTERR);
					mrRet = RET_DMX_OS_OPERA_FAIL;
					break;
				}
			}
#endif /*__linux__*/
		}
		break;
	case DIVXDRM_GET_DEACT_CODE_STR:
		{
			DECRYPT_DIVXDRM_CODE_STRING_T *prCodeString = (DECRYPT_DIVXDRM_CODE_STRING_T *)pvBufOut;
#ifdef __linux__
			DECRYPT_DIVXDRM_CODE_STRING_T rCodeString;

			if (NULL == pvBufOut) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid input ")
					TEXT("params (eCode: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid DivxDRM ")
					TEXT("Instance (hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mm_memset(&rCodeString, 0, sizeof(rCodeString));
			if (0 != mm_copy_from_user(&rCodeString, prCodeString, sizeof(DECRYPT_DIVXDRM_CODE_STRING_T))) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed in mm_copy_from_user ")
					TEXT("-- prCodeString:0x%x\r\n")),
					prCodeString);
				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
			}

			mrRet = DivxDRMGetDeactivationCodeStr(prInst, &rCodeString);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DivxDRMGetDeactivationCodeStr ")
					TEXT("(mrRet: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			}

			if (0 != mm_copy_to_user(prCodeString, &rCodeString, sizeof(DECRYPT_DIVXDRM_CODE_STRING_T))) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed in mm_copy_to_user ")
					TEXT("-- prCodeString:0x%x\r\n")),
					prCodeString);
				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
			}
#else
			void   *pvMarshalledMem = NULL;
			u32	u4CodeStrSz = 0;
			HRESULT hr = S_OK;

			if (NULL == prCodeString) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid input ")
					TEXT("params (eCode: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid DivxDRM ")
					TEXT("Instance (hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			u4CodeStrSz = prCodeString->u4CodeStrSz;
			hr = CeOpenCallerBuffer(&pvMarshalledMem,
				(void *)(prCodeString->szCodeStr),
				u4CodeStrSz, ARG_IO_PTR, FALSE);

			if (FAILED(hr)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in CeOpenCallerBuffer")
					TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), Err: 0x%x\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO,
					pvMarshalledMem, (u32)(prCodeString->szCodeStr),
					u4CodeStrSz, DMX_GET_LASTERR);
				mrRet = RET_DMX_OS_OPERA_FAIL;
				break;
			}

			mrRet = DivxDRMGetDeactivationCodeStr(prInst, prCodeString);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DivxDRMGetDeactivationCodeStr ")
					TEXT("(mrRet: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			}

			if (NULL != pvMarshalledMem) {
				hr = CeCloseCallerBuffer(pvMarshalledMem,
					(void *)(prCodeString->szCodeStr),
					u4CodeStrSz, ARG_IO_PTR);

				if (FAILED(hr)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
						(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in CeCloseCallerBuffer")
						TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), Err: 0x%x\r\n")),
						DMX_FUNC_NAME, DMX_LINE_NO,
						pvMarshalledMem, (u32)(prCodeString->szCodeStr),
						u4CodeStrSz, DMX_GET_LASTERR);
					mrRet = RET_DMX_OS_OPERA_FAIL;
					break;
				}
			}
#endif /*__linux__*/
		}
		break;
	case DIVXDRM_GET_ACT_MESSAGE:
		{
			DECRYPT_DIVXDRM_MSG_T *prMsg = (DECRYPT_DIVXDRM_MSG_T *)pvBufOut;
#ifdef __linux__
			DECRYPT_DIVXDRM_MSG_T rMsg;

			if (NULL == pvBufOut) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid input ")
					TEXT("params (eCode: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid DivxDRM ")
					TEXT("Instance (hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mm_memset(&rMsg, 0, sizeof(rMsg));

			if (0 != mm_copy_from_user(&rMsg, prMsg, sizeof(DECRYPT_DIVXDRM_MSG_T))) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed in mm_copy_from_user")
					TEXT(" -- prMsg:0x%x\r\n")),
					prMsg);
				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
			}

			mrRet = DivxDRMGetActivationMsg(prInst, &rMsg);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DivxDRMGetActivationMsg")
					TEXT(" (mrRet: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			}

			if (0 != mm_copy_to_user(prMsg, &rMsg, sizeof(DECRYPT_DIVXDRM_MSG_T))) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed in mm_copy_to_user ")
					TEXT("-- prMsg:0x%x\r\n")),
					prMsg);
				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
			}
#else
			u32 u4MsgStrSz = 0;
			void   *pvMarshalledMem = NULL;
			HRESULT hr = S_OK;

			if (NULL == prMsg) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid input ")
					TEXT("params (eCode: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid DivxDRM ")
					TEXT("Instance (hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			u4MsgStrSz = prMsg->u4MsgStrSz;
			hr = CeOpenCallerBuffer(&pvMarshalledMem,
				(void *)(prMsg->szMsgStr),
				u4MsgStrSz, ARG_IO_PTR, FALSE);

			if (FAILED(hr)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in CeOpenCallerBuffer")
					TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), Err: 0x%x\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO,
					pvMarshalledMem, (u32)(prMsg->szMsgStr),
					u4MsgStrSz, DMX_GET_LASTERR);
				mrRet = RET_DMX_OS_OPERA_FAIL;
				break;
			}

			mrRet = DivxDRMGetActivationMsg(prInst, prMsg);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DivxDRMGetActivationMsg ")
					TEXT("(mrRet: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			}

			if (NULL != pvMarshalledMem) {
				hr = CeCloseCallerBuffer(pvMarshalledMem,
					(void *)(prMsg->szMsgStr),
					u4MsgStrSz, ARG_IO_PTR);

				if (FAILED(hr)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
						(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in CeCloseCallerBuffer")
						TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), Err: 0x%x\r\n")),
						DMX_FUNC_NAME, DMX_LINE_NO,
						pvMarshalledMem, (u32)(prMsg->szMsgStr),
						u4MsgStrSz, DMX_GET_LASTERR);
					mrRet = RET_DMX_OS_OPERA_FAIL;
					break;
				}
			}
#endif /*__linux__*/
		}
		break;
	case DIVXDRM_GET_DEACT_MESSAGE:
		{
			DECRYPT_DIVXDRM_MSG_T *prMsg = (DECRYPT_DIVXDRM_MSG_T *)pvBufOut;
#ifdef __linux__

			DECRYPT_DIVXDRM_MSG_T rMsg;

			if (NULL == pvBufOut) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid input ")
					TEXT("params (eCode: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid DivxDRM ")
					TEXT("Instance (hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mm_memset(&rMsg, 0, sizeof(rMsg));

			if (0 != mm_copy_from_user(&rMsg, prMsg, sizeof(DECRYPT_DIVXDRM_MSG_T))) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed in mm_copy_from_user ")
					TEXT("-- prMsg:0x%x\r\n")),
					prMsg);
				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
			}

			mrRet = DivxDRMGetDeActivationMsg(prInst, &rMsg);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DivxDRMGetDeActivationMsg ")
					TEXT("(mrRet: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			}

			if (0 != mm_copy_to_user(prMsg, &rMsg, sizeof(DECRYPT_DIVXDRM_MSG_T))) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed in mm_copy_to_user -- ")
					TEXT("prMsg:0x%x\r\n")),
					prMsg);
				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
			}
#else
			u32 u4MsgStrSz = 0;
			void   *pvMarshalledMem = NULL;
			HRESULT hr = S_OK;

			if (NULL == prMsg) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid input params ")
					TEXT("(eCode: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid DivxDRM Instance ")
					TEXT("(hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			u4MsgStrSz = prMsg->u4MsgStrSz;
			hr = CeOpenCallerBuffer(&pvMarshalledMem,
				(void *)(prMsg->szMsgStr),
				u4MsgStrSz, ARG_IO_PTR, FALSE);

			if (FAILED(hr)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in CeOpenCallerBuffer")
					TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), Err: 0x%x\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO,
					pvMarshalledMem, (u32)(prMsg->szMsgStr),
					u4MsgStrSz, DMX_GET_LASTERR);
				mrRet = RET_DMX_OS_OPERA_FAIL;
				break;
			}

			mrRet = DivxDRMGetDeActivationMsg(prInst, prMsg);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in DivxDRMGetDeActivationMsg ")
					TEXT("(mrRet: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			}

			if (NULL != pvMarshalledMem) {
				hr = CeCloseCallerBuffer(pvMarshalledMem,
					(void *)(prMsg->szMsgStr),
					u4MsgStrSz, ARG_IO_PTR);

				if (FAILED(hr)) {
					DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
						(TEXT("[DECRYPT][DIVXDRM] %s line %d fail in CeCloseCallerBuffer")
						TEXT("(DestMarshallMem: 0x%x, SrcMem: 0x%x, Sz: %d), Err: 0x%x\r\n")),
						DMX_FUNC_NAME, DMX_LINE_NO,
						pvMarshalledMem, (u32)(prMsg->szMsgStr),
						u4MsgStrSz, DMX_GET_LASTERR);
					mrRet = RET_DMX_OS_OPERA_FAIL;
					break;
				}
			}
 #endif /*__linux__*/
	   }
		break;
	case DIVXDRM_DECRYPT_AUDIO:
		{
			DECRYPT_DIVXDRM_DECRYPT_DATA_T *prDecryptInfo = NULL;

			if ((sizeof(DECRYPT_DIVXDRM_DECRYPT_DATA_T) != u4LenIn) ||
				(NULL == pvBufIn)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid input ")
					TEXT("params (eCode: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			prDecryptInfo = (DECRYPT_DIVXDRM_DECRYPT_DATA_T *)pvBufIn;

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid ")
					TEXT("DivxDRM Instance (hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mrRet = DivxDRMDecryptAudio(prInst, prDecryptInfo);

		}
		break;

	case DIVXDRM_DECRYPT_VIDEO:
		{
			DECRYPT_DIVXDRM_DECRYPT_DATA_T *prDecryptInfo =
				(DECRYPT_DIVXDRM_DECRYPT_DATA_T *)pvBufIn;

			if ((sizeof(DECRYPT_DIVXDRM_DECRYPT_DATA_T) != u4LenIn) ||
				(NULL == pvBufIn)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid input ")
					TEXT("params (eCode: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid ")
					TEXT("DivxDRM Instance (hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mrRet = DivxDRMDecryptVideo(prInst, prDecryptInfo);
		}
		break;

	case DIVXDRM_GET_VIDEO_KEYINFO:
		{
			DECRYPT_DIVXDRM_VIDEO_KEYINFO_T *prKeyInfo =
				(DECRYPT_DIVXDRM_VIDEO_KEYINFO_T *)pvBufOut;

			if ((sizeof(DECRYPT_DIVXDRM_VIDEO_KEYINFO_T) != u4LenOut) ||
				(NULL == pvBufOut)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid input ")
					TEXT("params (eCode: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid DivxDRM ")
					TEXT("Instance (hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mrRet = DivxDRMGetVideoKeyInfo(prInst,
				prKeyInfo->u2KeyIndex, prKeyInfo);

		}
		break;

	case DIVXDRM_GET_AUDIO_KEYINFO:
		{
			DECRYPT_DIVXDRM_AUDIO_KEYINFO_T *prKeyInfo =
				(DECRYPT_DIVXDRM_AUDIO_KEYINFO_T *)pvBufOut;

			if ((sizeof(DECRYPT_DIVXDRM_AUDIO_KEYINFO_T) != u4LenOut) ||
				(NULL == pvBufOut)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d fail for invalid output")
					TEXT("params (eCode: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (!DivxDRMIsInstValid(prInst)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
					(TEXT("[DECRYPT][DIVXDRM] %s line %d failed for invalid DivxDRM ")
					TEXT("Instance (hInst: 0x%x)\r\n")),
					DMX_FUNC_NAME, DMX_LINE_NO, prInst);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			mrRet = DivxDRMGetAudioKeyInfo(prInst, prKeyInfo);

		}
		break;
	default:
		DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_DECRYPT,
			(TEXT("[DECRYPT][DIVXDRM] %s line %d -- Unknown DivxDRM Operation ")
			TEXT("Code(%d)\r\n")),
			DMX_FUNC_NAME, DMX_LINE_NO, u4Code);
		break;
	}

	MM_RETURN(mrRet);
}

#endif /* #if DMX_SUPPORT_DIVXDRM */


