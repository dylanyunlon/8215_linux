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
 * @file dmx_esm.h
 *
 * @par Project
 *
 *
 * @par Description
 *
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */

#ifndef DMX_ESM_H
#define DMX_ESM_H

#ifdef __linux__
#include <media/atc/drv_aud.h>
#else
#include "drv_aud.h"
#endif /* __linux__ */


#ifdef __cplusplus
extern "C" {
#endif

/*/ Fifo information*/
typedef struct _Fifo_Info {
	uintptr_t ptrSa;			/*/< fifo start Virtual address*/
	uintptr_t ptrEa;			/*/< fifo end Virtual address*/
	uintptr_t ptrRdPtr;			/*/< fifo read pointer*/
	uintptr_t ptrWrPtr;			/*/< fifo write pointer*/
} DMX_FIFO_INFO_T;

/*/ Access unit table information*/
typedef struct _AUTable_Info {
	uintptr_t ptrSa;			/*/< Access unit table start address*/
	u32 u4Sz;			/*/< Access unit table memory size*/
	u32 u4AUCount;		/*/< Maximum access unit count*/
	u32 u4RdIdx;			/*/< Current access unit read index*/
	u32 u4WrIdx;			/*/< Current access unit write index*/
} DMX_AUTABLE_INFO_T;

/*/ Access unit table information*/
typedef struct _AUExtTable_Info {
	uintptr_t ptrSa;			/*/< Access unit table start address*/
	u32 u4Sz;			/*/< Access unit table memory size*/
	u32 u4AUCount;		/*/< Maximum access unit count*/
	u32 u4RdIdx;			/*/< Current access unit read index*/
	u32 u4WrIdx;			/*/< Current access unit write index*/
} DMX_AUEXTTABLE_INFO_T;

/*/ Elementary stream interface information*/
typedef struct _ESI_Inst_Info {
	bool		fgUsed; 		/*/< Used or not*/
	u16 		u2Ref;			/*/< Reference count*/
	u32 		u4FilterType;		/*/< Filter component type*/
	u32 		u4FilterId; 	/*/< Filter component ID*/
	u32 		u4StmCodec;

	ES_TYPE			eType;			/*/< Elementary stream interface type*/
	DMX_FIFO_INFO_T		*prFifo;		/*/< Fifo information*/
	DMX_FIFO_INFO_T		*prSwFifo;		/*/< SW Dec Fifo information*/
	DMX_FIFO_INFO_T		*prHwFifo;		/*/< HW Dec Fifo information*/
	DMX_AUTABLE_INFO_T	*prAUTable;		/*/< Access unit table information*/
	DMX_AUEXTTABLE_INFO_T	*prAUExtTable;		/*/< Access unit extensional info table information*/
	void			*pvDemuxerCBPrivate;	/*/< Private data for Filter callback function*/
	ESM_FUNC_CB		pvDemuxerCB;		/*/< Filter callback function*/
	void			*pvDecoderCBPrivate;	/*/< Private data for Decoder callback function*/
	ESM_FUNC_CB		pvDecoderCB;		/*/< Decoder callback function*/
	void *pvSptHdl;			/*/< Corrsponding Splitter handle*/
	u64			u8DecSendBufMask;
} DMX_ESM_INST_T;

#define MAX_ESICOUNT MAX_STREAM_INSTANCE_CNT

/*/ for esm debug*/
#define ESM_DEBUG_OFF	0
#define ESM_DEBUG_AU_BASIC_INFO 1
#define ESM_DEBUG_ALL_INFO	2
#define ESM_DEBUG_CHECK_SUM 3

#ifdef __cplusplus
}
#endif

#endif /*#ifndef _DMX_ESM_H_*/

