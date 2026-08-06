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
 * @file dmx_parser.h
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
 *
 */

#ifndef DMX_INTERNAL_PSR_H
#define DMX_INTERNAL_PSR_H

#include "x_typedef.h"
#include "dmx_spt.h"
#include "dmx_sema.h"
#include "dmx_psr_cc.h"
#include "dmx_psr_filter.h"

#ifdef __cplusplus
extern "C" {
#endif

#if CONFIG_DRV_HIGH_BITRATE_SPECIAL_PROC
#define HIGH_BITRATE_VID_STANDARD	   (0x2800000)	/* 40M bps */
#endif

#define MAX_FILTER_COUNT (MAX_PSR_CC_CNT * MAX_PSR_FILTER_PER_CC)

#define MAX_FILTER_COUNT_PER_DMX	(MAX_PSR_CC_CNT * MAX_PSR_FILTER_PER_CC/MAX_DMX_INSTANCE_CNT)

/*Decryption Align Definition*/
#define DMX_MAX_DECRYPT_BUFFER_LEN	   4096
#define CSS_DECRYPTION_ALIGNSIZE	   2048

/*/ Enumerate callback event*/
typedef enum {
	E_WAKEUP_ME = 0,	/*/< Wake up me, please call i4PSR_CC_MainLoop, pvData is NULL; */
	E_TX_DONE,	/*/< TX data done, pvData is NULL. */
	E_PAUSE_DONE,	/*/< Pause done, pvData is NULL */
	E_ABORT_DONE,	/*/< Abort done, pvData is NULL */
	/*/< Get access unit information to fill into au table, pvData is the pointer of struct PSR_AU */
	E_GET_AU_INFO,
	E_TX_JUMP	/* need to jump */
} PSR_CB_EVENT;

/*/ au information for callback event E_GET_AU_INFO*/
typedef struct _PSR_AU {
	E_SPT_DATA_TYPE_T eType;	/*/< Access unit type */
	void *pvAUInf;	/*/< AU Item Info */
	void *pvAUExtInf;	/*/< AU Extern Item Info */
} PSR_AU;

typedef struct {
	u32 u4WrPtr;
	u32 u4RdPtr;
	void **ppvQueue;
} PSR_HAL_RES_QUEUE_INFO_T;

typedef struct {
	bool fgUsed;
	PSR_HAL_RES_QUEUE_INFO_T rPSRHWResQueInfo;
	PSR_CC *aprPsrCCs[MAX_PSR_CC_CNT];
	PSR_FILTER *aprPsrFtrs[MAX_FILTER_COUNT];
} DMX_PSR_MAN_INFO_T;

EXTERN DMX_PSR_MAN_INFO_T g_rPsrMan;
EXTERN bool g_fgPSRInit;

#define PSR_ENABLE_SEMA
#ifdef PSR_ENABLE_SEMA
EXTERN HANDLE g_hPSRSema;
#define PSR_LOCK_INIT(mrRet)	do { \
	if (NULL == g_hPSRSema) { \
		mrRet = dmx_sema_create(&g_hPSRSema, \
			DMX_SEMA_TYPE_MUTEX, DMX_SEMA_STATE_UNLOCK); \
		if (RET_DMX_OK != mrRet) { \
			DMXLOG_ERROR(TEXT("[PSR] %s line %d -- Failed to create") \
				     TEXT(" sema g_hPSRSema\r\n"), DMX_FUNC_NAME, DMX_LINE_NO); \
		} \
	} \
} while (0)

#define PSR_LOCK	do { \
	if (NULL != g_hPSRSema)\
		dmx_sema_lock(g_hPSRSema, DMX_SEMA_OPTION_WAIT);\
} while (0)

#define PSR_UNLOCK	do { \
	if (NULL != g_hPSRSema)\
		dmx_sema_unlock(g_hPSRSema);\
} while (0)

#define PSR_LOCK_UNINIT(mrRet)	do { \
	if (NULL != g_hPSRSema) {\
		dmx_sema_delete(g_hPSRSema); \
		g_hPSRSema = NULL;  \
	} \
	mrRet = mrRet;\
} while (0)

#else				/* PSR_ENABLE_SEMA */

#define PSR_LOCK_INIT(mrRet)  (mrRet = mrRet)
#define PSR_LOCK
#define PSR_UNLOCK
#define PSR_LOCK_UNINIT(mrRet)	(mrRet = mrRet)

#endif				/* PSR_ENABLE_SEMA */

EXTERN HANDLE g_hPSRHalSema;
#define PSR_HAL_LOCK_INIT(mrRet)	do { \
	if (NULL == g_hPSRHalSema) {  \
		mrRet = dmx_sema_create(&g_hPSRHalSema, \
			DMX_SEMA_TYPE_MUTEX, DMX_SEMA_STATE_UNLOCK); \
		if (RET_DMX_OK != mrRet) {  \
			DMXLOG_ERROR(TEXT("[PSR] %s line %d -- Failed to create ")\
				TEXT("sema g_hPSRSema\r\n"), DMX_FUNC_NAME, DMX_LINE_NO);  \
		}  \
	} \
} while (0)

#define PSR_HAL_LOCK	do { \
	if (NULL != g_hPSRHalSema)\
		dmx_sema_lock(g_hPSRHalSema, DMX_SEMA_OPTION_WAIT);\
} while (0)

#define PSR_HAL_UNLOCK	do { \
	if (NULL != g_hPSRHalSema)\
		dmx_sema_unlock(g_hPSRHalSema);\
} while (0)

#define PSR_HAL_LOCK_UNINIT	do { \
	if (NULL != g_hPSRHalSema) {\
		dmx_sema_delete(g_hPSRHalSema); \
		g_hPSRHalSema = NULL;  \
	} \
} while (0)

MRESULT ParserInit(void);

MRESULT ParserUninit(void);

MRESULT Splitter4PsrEvent(PSR_CB_EVENT eEvent, void *pvEventData, void *pvSptHdl);

MRESULT PSR_CC_CBSplitter(PSR_CC *prPsrCC, PSR_CB_EVENT eEvent, void *pvData);

bool PSR_HWRes_Obtain(PSR_FILTER *pPsrFtr);

void PSR_HWRes_Release(PSR_FILTER *pPsrFtr);

bool PsrWakeupOtherPsrCC(PSR_FILTER *prPsrFtr, PSR_CC *prPsrCC, u32 u4CheckTxState);

bool PsrWakeupOnePsrCC(u32 u4CheckTxState, void *ptrDmxInst);


MRESULT ParserSetPowerState(DMX_PM_STATE ePowerState, void **ppvSptHdl);
DMX_PM_STATE ParserGetPowerState(void);

MRESULT PSR_SetAudioDataAftSuspend(void);

#ifdef __cplusplus
}
#endif
#endif				/* #ifndef DMX_INTERNAL_PSR_H */
