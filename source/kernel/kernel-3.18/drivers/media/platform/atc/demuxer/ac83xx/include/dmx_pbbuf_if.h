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
 * @file dmx_pbbuf_if.h
 *
 * @par Project
 *
 * @par Description
 *    Demuxer Pbbuf Structure, Macro, interface declarations
 *
 * @par Author_Name
 *    Shuhui Zhang
 *
 */

#ifndef DMX_INTERNAL_PBBUF_H
#define DMX_INTERNAL_PBBUF_H

#include "x_typedef.h"
#ifdef __linux__
#include <media/atc/dmx_splitter.h>
#if CONFIG_DRV_HDMI_RX
#include <media/atc/x_audin.h>
#endif				/* CONFIG_DRV_HDMI_RX */
#else				/* __linux__ */
#include "dmx_splitter.h"
#if CONFIG_DRV_HDMI_RX
#include "x_audin.h"
#endif				/* CONFIG_DRV_HDMI_RX */
#endif				/* __linux__ */

#ifdef __cplusplus
extern "C" {
#endif

/* Notify conditions to upper module MSDK */
typedef enum {
	PBBUF_COND_READY_TO_ALLOCATE_BUFFER = 0,
	PBBUF_COND_CANCEL_ALLOCATE_BUFFER,
	PBBUF_COND_BUFFER_CLEANED,
} PBBUF_NOTIFY_COND_T;

/* Notification for other driver. */
typedef enum {
	DRV_PBBUF_COND_READY_TO_GET_READ_BUFFER = 0,
/* when the parser wan't to get slots from send slots list, but no slot in the list, it wiil set the flag, */
/* and then when the mw send slots filled with needed data come in, it will clear this flag */
	DRV_PBBUF_COND_RELEASE_ALL_SLOTS,	/* no use */
	DRV_PBBUF_COND_NEED_QUICK_JUMP
} DRV_PBBUF_NOTIFY_COND_T;

typedef void(*x_pbbuf_nfy_fct) (void *pvTag, PBBUF_NOTIFY_COND_T eReadyCond,
				s32 i4Data1, u32 u4Data2);

typedef struct _PBBUF_NFY_FCT_DATA_T {
	PBBUF_NOTIFY_COND_T eReadyCond;
	s32 i4Data1;
	u32 u4Data2;
} PBBUF_NFY_FCT_DATA_T;

typedef struct _PBBUF_NFY_FCT_T {
	x_pbbuf_nfy_fct pfNfyFct;
	void *pvTag;
} PBBUF_NFY_FCT_T;

typedef struct _DMX_READ_BUFFER_TAG {
	PBBUF_SLOT_HEADER_INFO_T rHeader;

	/* Slot's Data Start Address when the slot is sent to the sent slots list */
	u8 *pcPlayBuffer;

	/* Data Start Offset in the pBBuf slot when the slot is sent to the sent slots list */
	u64 u8SrcOffset;

	/* Valid Data size in the pBBuf slot when the slot is sent to the sent slots list */
	u32 u4DataSize;

	/* The valid data local start offset to the slot start position*/
	/*while the slot is sent to the sent slot list */
	u32 u4PlayOffset;

	/* The valid data size while the slot is sent to the sent slot list */
	u32 u4PlaySize;

	void *pvSlot;	/* Slot Handle */
	bool fgFollowedByIbc;
	u32 u4BufferSize;
} DMX_READ_BUFFER;

typedef struct _SENT_BUFFER_INFO_TAG {
	u32 u4SentDataSize;
	u32 u4PbbufSize;
} SENT_BUFFER_INFO;

/* ************************************************************************************* */
/* u32 PBBUF_Init(void) */
/* Describe: Malloc PBBUF instance and create sema, and this function should be */
/* call when system bootup. */
/* Parameters: None */
/* return: Error Code of */
/* ************************************************************************************* */
	MRESULT PBBUF_Init(void);

/* ************************************************************************************* */
/* u32 PBBUF_Uninit(void) */
/* Describe: Malloc PBBUF instance and create sema, and this function should be */
/* call when system shutdown. */
/* Parameters: u2CompId  [IN] specify the target PBBUF */
/* return: void */
/* ************************************************************************************* */
MRESULT PBBUF_UnInit(void);

MRESULT PBBUF_Connect(void *pvSptHdl, void *pvPsr, u32 *pu4Idx, void **ppvPbbuf);

MRESULT PBBUF_Disconnect(void *pvSptHdl);

MRESULT PBBUF_GetInfo(void *pvSptHdl, uintptr_t *pptrBufSa, u32 *pu4BufSz,
	      u32 *pu4SlotSz);

/* ************************************************************************************* */
/* u32 PBBUF_Enable(uintptr_t ptrPBBuf, u32 u4BufTotalSz, u32 u4SlotSz, u8 u1PbuffType) */
/* Describe: Initialize PBBUF parameters, create the slot entry table and initialize it. */
/* Parameters: u2CompId      [IN] specify the target PBBUF */
/* u4BufTotalSz  [IN] the total size of the buffer */
/* u4SlotSz      [IN] the size of each slot */
/* u1PbuffType   [IN] If Pbbuf File, we need txdata event, otherwise, no need */
/* Return: S_PBBUF_OK: Initialize and create the slot table successfully. */
/* E_PBBUF_MEM_ALLOC_FAIL: Fail to create the slot table. */
/* ************************************************************************************* */
MRESULT PBBUF_Enable(void *pvSptHdl, u32 u4BufTotalSz,
	     u32 u4SlotSz, u8 u1PbuffType, u32 u4HdrParamSz);

MRESULT PBBUF_Disable(void *pvSptHdl);

/* ********************************************************************* */
/* u32 PBBUF_GetAllocSlot(uintptr_t ptrPBBuf, SEND_BUFFER *prSdBuf) */
/* Describe: Return a slot(Send Buffer) to middleware and remove it from FREE linked list */
/* if slot available. PBBUF marks it as a DATA slot. */
/* Parameters: u2CompId      [IN] specify the target PBBUF */
/* prSendBuffer  [OUT] slot pointer to an allocate buffer if available. */
/* Return: S_PBBUF_OK       A free slot is available. */
/* E_PBBUF_BUSY     No free slot can be available. */
/* ********************************************************************* */
MRESULT PBBUF_GetAllocSlot(void *pvSptHdl, SEND_BUFFER *prSdBuf);

/* ********************************************************************* */
/* u32 PBBUF_CancelAllocSlot(uintptr_t ptrPBBuf) */
/* Describe: Cancel pending request of specified ready to receive event. */
/* Parameters: u2CompId  [IN] specify the target PBBUF */
/* Return: S_PBBUF_OK   always return OK */
/* ********************************************************************* */
MRESULT PBBUF_CancelAllocSlot(void *pvSptHdl);

/* ********************************************************************* */
/* u32 PBBUF_SendDataSlot(void *pvSptHdl, SEND_BUFFER *prSdBuf, bool *pfgExitSent) */
/* Describe: MW sends a Send Buffer with stream data, PBBUF adds this Send Buffer */
/* to SENT linked list. */
/* Parameters: u2CompId      [IN] specify the target PBBUF */
/* prSendBuffer  [In] Send Buffer pointer with data information given by MW */
/* pfgExitSent   [OUT] Inform MW not to continue to alloc slot */
/* Return: S_PBBUF_OK      always return OK */
/* ********************************************************************* */
MRESULT PBBUF_SendDataSlot(void *pvSptHdl, SEND_BUFFER *prSdBuf, bool *pfgExitSent);

MRESULT PBBUF_SendDataSlotEx(void *pvSptHdl, SEND_BUFFER *prSdBuf);

/* ********************************************************************* */
/* u32 PBBUF_ReleaseAllocSlot(uintptr_t ptrPBBuf, SEND_BUFFER *prSdBuf) */
/* Describe: Release an allocated Send Buffer, and add it to the FREE linked list */
/* Parameters: u2CompId      [IN] specify the target PBBUF */
/* prSendBuffer  [In] Send Buffer pointer with data information given by MW */
/* Return: S_PBBUF_OK      always return OK */
/* ********************************************************************* */
MRESULT PBBUF_ReleaseAllocSlot(void *pvPBBuf, SEND_BUFFER *prSdBuf);

MRESULT PBBUF_CleanAllSlots(void *pvPBBuf);

MRESULT PBBUF_GetAvailDataSlot(void *pvSptHdl, DMX_READ_BUFFER *prRdBuf);

MRESULT PBBUF_ReleaseNoUseSlot(void *pvSptHdl, DMX_READ_BUFFER *prRdBuf);

MRESULT PBBUF_CancelReadSlot(void *pvSptHdl);

MRESULT PBBUF_DumpInfo(void *pvSptHdl, bool fgDumpData);

MRESULT PBBUF_ReleaseFrmSlotToUnCon(void *pvSptHdl, DMX_READ_BUFFER *prRdBuf,
	bool *pfgExistUnCon);

#if CONFIG_DRV_HDMI_RX
MRESULT PBBUF_GetAudInParsingInfo(void *pvSptHdl,
	AUDIN_PARSING_INFO_T * prAudinPsringInfo);

MRESULT PBBUF_AudInIsRAW(void *pvSptHdl, bool *pfgAudInIsRaw);
#endif				/* CONFIG_DRV_HDMI_RX */

#ifdef __cplusplus
}
#endif
#endif				/* #ifndef DMX_INTERNAL_PBBUF_H */
