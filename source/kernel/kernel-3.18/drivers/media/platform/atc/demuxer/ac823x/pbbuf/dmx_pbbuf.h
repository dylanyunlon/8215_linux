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
 * @file dmx_pbbuf.h
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

#ifndef PBBUF_H
#define PBBUF_H

#include "x_typedef.h"
#ifdef __linux__
#if CONFIG_DRV_HDMI_RX
#include <media/atc/x_audin.h>
#endif				/* CONFIG_DRV_HDMI_RX */
#include <media/atc/dmx_splitter.h>
#else
#include "dmx_splitter.h"
#if CONFIG_DRV_HDMI_RX
#include <media/atc/x_audin.h>
#endif				/* CONFIG_DRV_HDMI_RX */
#endif				/* __linux__ */

#include "dmx_pbbuf_if.h"
#include "dmx_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DMX_PRINT_PBBUF_DEBUG_LOG			  0

#define DMX_PBBUF_THREAD_NAME_MAX_LEN		  20
#define DMX_EXTPBBUF_THREAD_NAME			  "PBBUF0_EXT_TH"

//#if DMX_NEW_PBBUF_MECHANISM
/* PBBUF Internal Event */
#define DMX_PBBUF_EV_INITIAL				((EV_GRP_EVENT_T) 0)
#define DMX_PBBUF_EV_FREE_SLOT_IN			((EV_GRP_EVENT_T)(1) << 1)
#define DMX_PBBUF_EV_STOP_ALLOC				((EV_GRP_EVENT_T)(1) << 2)
#define DMX_PBBUF_EV_ERROR_FIXED			((EV_GRP_EVENT_T)(1) << 63)
//#endif

/*Definition*/
#define ThirtyTwoByteAlign(a)				   (((a) + 31) & (~31))	/*32-bytes aligned */

typedef enum {
	PBBUF_SLOT_FREE,	/* data already read from slot */
	PBBUF_SLOT_ALLOCATED,	/* slot locked by producer(prepare to send data to slot) */
	PBBUF_SLOT_SEND,	/* data already sent to slot */
	PBBUF_SLOT_READING,	/* slot locked by consumer(prepare to read data from slot) */
	PBBUF_SLOT_TYPE_CNT
} E_PBBUF_SLOT_TYPE_T;

/*sizeof(SLOT) : 56*/
typedef struct _SLOT_TAG {
	PBBUF_SLOT_HEADER_INFO_T rHeader;
	u8 *pcBuffer;	/* Slot's Buffer Start Address */
	u8 *pcUsrBuffer;	/* pcBuffer's mappint address in User Space */
	u8 *pcPlayBuffer;	/* point to the decryped data. */
	u32 u4BufferSize;	/* Slot's Buffer's Size */
	u32 u4DataOffset;	/* The unread data's start offset in the slot's buffer */
	u32 u4DataSize;	/* The unread data's size in the slot's buffer */
	u32 u4PlayOffset;	/* = 0 */
	u32 u4PlaySize;	/* == u4DataSize */
	u64 u8SrcOffset;	/* The file offset of the u4DataOffset */
	struct _SLOT_TAG *prNextSlot;
	struct _SLOT_TAG *prPrevSlot;
	E_PBBUF_SLOT_TYPE_T eType;
	u32 u4SessionID;
} SLOT;

typedef struct {
	u32 u4SlotCnt;
	SLOT *prHeadSlot;
	SLOT *prTailSlot;
} PBBUF_SLOT_LIST_INFO_T;

typedef struct {
	HANDLE hThread;
	bool fgExitThread;
	HANDLE hMsgQueue;
#if CONFIG_DRV_HDMI_RX
#ifdef __linux__
	wait_queue_head_t waitexitqueue;
	int waitexitqueueflag;
	DRV_AUDIN_CB_LIST_T rAudInCbsInfo;
#else
	HANDLE hAudInDrv;
	HANDLE hAudInFileMap;
	AUDINBUF_POSINFO_T *prAudInBufInfo;
	MSGQUEUEOPTIONS rMsgQOptions;
#endif				/* #ifdef __linux__ */
#endif				/* CONFIG_DRV_HDMI_RX */
} PBBUF_AUDIN_EXTINFO_T;

typedef struct _DMX_IPBBUF {
	MRESULT (*pmrPbBufInitBuffer)(HANDLE hPBBuf, u32 u4BufTotalSz,
				      u32 u4SlotSz, u32 u4HdrParamSz,
				      u8 u1PbbufType);
	void (*pvPbBufDeInitBuffer)(HANDLE hPBBuf);

	MRESULT (*pmrPbBufGetAllocSlot)(HANDLE hPBBuf, SEND_BUFFER *prSdBuf);
	MRESULT (*pmrPbBufCancelAllocSlot)(HANDLE hPBBuf);
	MRESULT (*pmrPbBufSendDataSlot)(HANDLE hPBBuf, SEND_BUFFER *prSdBuf,
				 bool *pfgExitSent);
	MRESULT (*pmrPbBufReleaseAllocSlot)(HANDLE hPBBuf, SEND_BUFFER *prSdBuf);
	MRESULT (*pmrPbBufCleanAllSlots)(HANDLE hPBBuf);
	MRESULT (*pmrPbBufGetAvailDataSlot)(HANDLE hPBBuf, DMX_READ_BUFFER *prRdBuf);
	MRESULT (*pmrPbBufReleaseNoUseSlot)(HANDLE hPBBuf, DMX_READ_BUFFER *prRdBuf);
	MRESULT (*pmrPbBufCancelReadSlot)(HANDLE hPBBuf);
	MRESULT (*pmrPbBufReleaseFrmSlotToUnCon)(HANDLE hPBBuf, DMX_READ_BUFFER *prRdBuf,
					  bool *pfgExistUnCon);
#if CONFIG_DRV_HDMI_RX
	MRESULT (*pmrPbBufGetAudInParsingInfo)(HANDLE hPBBuf,
					AUDIN_PARSING_INFO_T *prAudinPsringInfo);
	MRESULT (*pmrAudInIsRAW)(HANDLE hPBBuf, bool *pfgAudInIsRaw);
#endif				/* CONFIG_DRV_HDMI_RX */
} DMX_IPBBUF;

/* sizeof(PBBUF): 844*/
typedef struct _PBBUF_TAG {
	bool fgConnected;
	bool fgEnable;
	u8 u1SptPbuffType;
	u32 u4CompID;
	u32 u4TotalSz;	/*PBBUF Ring Buffer Total Size */
	u32 u4SlotSz;	/*BBUF Ring Buffer Slot Size */
	u32 u4SlotAmount;	/* Slot Count -- u4SlotAmount = u4TotalSz / u4SlotSz; */
	u32 u4MwNfyMask;
	u32 u4DrvNfyMask;

	u8 *pcBufSa;	/*BBUF Ring Buffer Start Address */
	u8 *pcUsrBufSa;
	u8 *pcMMRsvBufBase;
	void *pvSLotHdrs;
	void *pvExtInfo;
	SLOT *prSLOT;
	HANDLE hUsrCaller;
	HANDLE hSemaphore;

//#if DMX_NEW_PBBUF_MECHANISM
	HANDLE hEvtGroup;
//#else
	HANDLE hTxData;
//#endif				/* DMX_NEW_PBBUF_MECHANISM */

	/*Use (u4NfyMask |= (1 << PBBUF_NOTIFY_COND_T )) to do bit mask */
	
	void *pvDrvOwner;
	void *pvSptHdl;

	u64 u8LastSrcOfst;
	PBBUF_SLOT_LIST_INFO_T arSlotLists[PBBUF_SLOT_TYPE_CNT];
	DMX_IPBBUF rPbBufInterfaces;
	
} PBBUF;

#define PBBUF_ENTRY(pbbuf, option)	do {\
if (RET_DMX_OK != dmx_sema_lock(pbbuf->hSemaphore, option)) {\
	DMX_ASSERT(FALSE);\
} \
} while (0)

#define PBBUF_EXIT(pbbuf)	do {\
if (RET_DMX_OK != dmx_sema_unlock(pbbuf->hSemaphore)) {\
	DMX_ASSERT(FALSE);\
} \
} while (0)

#define PBBUFChkNfyMask(NfyMask, MaskItem)					\
((NfyMask & (1 << MaskItem)) ? TRUE : FALSE)

#define PBBUFSetNfyMask(NfyMask, MaskItem)					\
(NfyMask |= (1 << MaskItem))

#define PBBUFClrNfyMask(NfyMask, MaskItem)					\
(NfyMask &= ~(1 << MaskItem))

MRESULT PBBUF_RemoveSlot(PBBUF *prPbBuf, SLOT *prSlot);
MRESULT PBBUF_AddSlotToTail(PBBUF *prPbBuf, SLOT *prSlot,
			    E_PBBUF_SLOT_TYPE_T eSlotListType);
SLOT *PBBUF_GetSlotFromHead(PBBUF *prPbBuf, E_PBBUF_SLOT_TYPE_T eSlotType);
u32 PBBUF_GetListCount(PBBUF *prPbBuf, E_PBBUF_SLOT_TYPE_T eSlotListType);
SLOT *PBBUF_GetSlotByHandle(const PBBUF *prPbBuf, void *pvSlot);
void PBBUF_ReadyToGetReadBuffer(PBBUF *prPbBuf, SLOT *prSlot);
MRESULT PBBUF_ReadyToGetAllocSlot(PBBUF *prPbBuf);
MRESULT PBBUF_BufferCleaned(PBBUF *prPbBuf);

MRESULT PBBUF_ExtBuf_InitBuffer(void *pvPbBuf, u32 u4BufTotalSz,
				u32 u4SlotSz, u32 u4HdrParamSz, u8 u1PbbufType);
void PBBUF_ExtBuf_DeInitBuffer(void *pvPbBuf);
MRESULT PBBUF_ExtBuf_SendDataSlot(void *pvPbBuf, SEND_BUFFER *prSdBuf, bool *pfgExitSent);
MRESULT PBBUF_ExtBuf_CleanAllSlots(void *pvPbBuf);
MRESULT PBBUF_ExtBuf_GetAvailDataSlot(void *pvPbBuf, DMX_READ_BUFFER *prRdBuf);
MRESULT PBBUF_ExtBuf_ReleaseNoUseSlot(void *pvPbBuf, DMX_READ_BUFFER *prRdBuf);
MRESULT PBBUF_ExtBuf_ReleaseNoUseSlot(void *pvPbBuf, DMX_READ_BUFFER *prRdBuf);
MRESULT PBBUF_ExtBuf_CancelReadSlot(void *pvPbBuf);
MRESULT PBBUF_ExtBuf_RelFrmSlotToUnCon(void *pvPbBuf,
				       DMX_READ_BUFFER *prRdBuf, bool *pfgExistUnCon);
#if CONFIG_DRV_HDMI_RX
MRESULT PBBUF_ExtBuf_GetAudInParsingInfo(void *pvPbBuf,
					 AUDIN_PARSING_INFO_T *prAudinPsringInfo);
MRESULT PBBUF_ExtBuf_AudInIsRAW(void *pvPbBuf, bool *pfgAudInIsRaw);
#ifdef __linux__
u32 PBBUF_ExtBuf_AudInGetSessionID(void *pvPbBuf);
#endif				/* __linux__ */
#endif				/* CONFIG_DRV_HDMI_RX */

MRESULT PBBUF_SelfBuf_InitBuffer(void *pvPbBuf, u32 u4BufTotalSz,
				 u32 u4SlotSz, u32 u4HdrParamSz, u8 u1PbbufType);
void PBBUF_SelfBuf_DeInitBuffer(void *pvPbBuf);
MRESULT PBBUF_SelfBuf_GetAllocSlot(void *pvPbBuf, SEND_BUFFER *prSdBuf);
MRESULT PBBUF_SelfBuf_CancelAllocSlot(void *pvPbBuf);
MRESULT PBBUF_SelfBuf_SendDataSlot(void *pvPbBuf,
				   SEND_BUFFER *prSdBuf, bool *pfgExitSent);
MRESULT PBBUF_SelfBuf_ReleaseAllocSlot(void *pvPbBuf, SEND_BUFFER *prSdBuf);
MRESULT PBBUF_SelfBuf_CleanAllSlots(void *pvPbBuf);
MRESULT PBBUF_SelfBuf_GetAvailDataSlot(void *pvPbBuf, DMX_READ_BUFFER *prRdBuf);
MRESULT PBBUF_SelfBuf_ReleaseNoUseSlot(void *pvPbBuf, DMX_READ_BUFFER *prRdBuf);
MRESULT PBBUF_SelfBuf_CancelReadSlot(void *pvPbBuf);
MRESULT PBBUF_SelfBuf_RelFrmSlotToUnCon(void *pvPbBuf,
					DMX_READ_BUFFER *prRdBuf, bool *pfgExistUnCon);

#ifdef __cplusplus
}
#endif
#endif
