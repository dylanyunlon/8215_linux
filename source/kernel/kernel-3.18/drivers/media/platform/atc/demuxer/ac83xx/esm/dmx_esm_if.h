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
 * @file dmx_esm_if.h
 *
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
 *
 */

#ifndef DMX_ESM_IF_H
#define DMX_ESM_IF_H

#ifdef __linux__
#include <media/atc/dmx_define.h>
#include <media/atc/drv_esm_if.h>
#else
#include "dmx_define.h"
#include "drv_esm_if.h"
#endif /* __linux__*/

#include "dmx_def.h"
#include "dmx_esm.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __linux__
typedef void(*ESM_FUNC_CB)(ES_CBEVENT eEvent, void *pvData, void *pvPrivate);
#endif /* __linux__*/

/*/ This function will register callback function*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \param pfnCB [IN] Callback function*/
/*/ \param pvPrivate [IN] Caller private data, it will be the paramter of the callback function*/
/*/ \return None.*/
void ESM_RegistDemuxerCB(u32 u4Handle, ESM_FUNC_CB pfnCB, void *pvPrivate);

/*/ This function will register callback function*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \param pfnCB [IN] Callback function*/
/*/ \param pvPrivate [IN] Caller private data, it will be the paramter of the callback function*/
/*/ \return None.*/
void ESM_RegistDecoderCB(u32 u4Handle, ESM_FUNC_CB pfnCB, void *pvPrivate);

/*/ This function will set ESI type*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \param eEsIfType [IN] Elementary stream type*/
/*/ \return If return value < 0, it's failed. Please reference drv_esm_errcode.h.*/
MRESULT ESM_SetESType(u32 u4Handle, ES_TYPE eEsIfType);

/*/ This function will get ESI type*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \param pEsIfType [out] Elementary stream type*/
/*/ \return If return value < 0, it's failed. Please reference drv_esm_errcode.h.*/
MRESULT ESM_GetESType(u32 u4Handle, ES_TYPE *pEsType);
/*/ This function will get access unit information*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \param u4Value [IN] Access unit index*/
/*/ \param pprAUInfo [OUT] Access unit information. If elementary stream is video or subpicture, prAUInfo is AU_VInfo.*/
/*/ \return If return value < 0, it's failed. Please reference drv_esm_errcode.h.*/
MRESULT ESM_AUTableGetAUInfo(u32 u4Handle, u32 u4Value, void **pprAUInfo);

MRESULT ESM_AUTableGetAUExtInfo(u32 u4Handle, u32 u4Value, void **pprAUExtInfo);

/*/ This function will get total unit count*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \return Total unit count. If ESM_INVALID_COUNT, something is wrong.*/
MRESULT ESM_AUTableGetTotalCount(u32 u4Handle, u32 *pu4TotalCnt);

/*/ This function will get current available access unit count*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \return Current available access unit count. If ESM_INVALID_COUNT, something is wrong.*/
MRESULT ESM_AUTableGetAvailCount(u32 u4Handle, u32 *pu4AvailCnt);

/*/ This function will get current free access unit count*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \return Current free access unit count. If ESM_INVALID_COUNT, something is wrong.*/
MRESULT ESM_AUTableGetFreeCount(u32 u4Handle, u32 *pu4FreeCount);

/*/ This function will get next access unit index of the reference index*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \param u4Value [IN] Reference access unit index*/
/*/ \param u4Value2 [IN] Access unit jump count*/
/*/ \return Next access unit index. If ESM_INVALID_INDEX, it means that next access unit is not available.*/
MRESULT ESM_AUTableGetNextAUIdx(u32 u4Handle, u32 u4AUIdx, u32 u4AUIncCnt,
	u32 *pu4AUIdx);

/*/ This function will get previous access unit index of the reference index*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \param u4Value [IN] Reference access unit index*/
/*/ \param u4Value2 [IN] Access unit jump count*/
/*/ \return previous access unit index. If ESM_INVALID_INDEX, it means that previous access unit is not available.*/
MRESULT ESM_AUTableGetPrevAUIdx(u32 u4Handle, u32 u4AUIdx, u32 u4AUIncCnt,
	u32 *pu4AUIdx);

/*/ This function will get current access unit read index*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \return Current read index. If ESM_INVALID_INDEX, something is wrong.*/
MRESULT ESM_AUTableGetRdIdx(u32 u4Handle, u32 *pu4RdIdx);

/*/ This function will increase current access unit read index*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \param u4Value [IN] Increase index count*/
/*/ \return If return value < 0, it's failed. Please reference drv_esm_errcode.h.*/
MRESULT ESM_AUTableIncRdIdx(u32 u4Handle, u32 u4Value);

/*/ This function will get current access unit write index*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \return Current access unit write index. If ESM_INVALID_INDEX, something is wrong.*/
MRESULT ESM_AUTableGetWrIdx(u32 u4Handle, u32 *pu4WrIdx);

#if 0
#define ESM_AUTableIncWrIdx(a, b)  ESM_AUTableIncWrIdxEx(a, b, __func__, __LINE__)
#define ESM_AUTableSetWrIdx(a, b)  ESM_AUTableSetWrIdxEx(a, b, __func__, __LINE__)
/*/ This function will increase current access unit write index*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \param u4Value [IN] Increase index count*/
/*/ \return If return value < 0, it's failed. Please reference drv_esm_errcode.h.*/
MRESULT ESM_AUTableIncWrIdxEx(u32 u4Handle, u32 u4Value, char *szFunc, s32 i4Line);

/*/ This function will set current access unit write index*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \param u4Value [IN] Write index*/
/*/ \return If return value < 0, it's failed. Please reference drv_esm_errcode.h.*/
MRESULT ESM_AUTableSetWrIdxEx(u32 u4Handle, u32 u4Value, char *szFunc, s32 i4Line);
#else
/*/ This function will increase current access unit write index*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \param u4Value [IN] Increase index count*/
/*/ \return If return value < 0, it's failed. Please reference drv_esm_errcode.h.*/
MRESULT ESM_AUTableIncWrIdx(u32 u4Handle, u32 u4Value);

/*/ This function will set current access unit write index*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \param u4Value [IN] Write index*/
/*/ \return If return value < 0, it's failed. Please reference drv_esm_errcode.h.*/
MRESULT ESM_AUTableSetWrIdx(u32 u4Handle, u32 u4Value);
#endif

/*/ This function will clear fifo. It will set read/write pointer to fifo start address.*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \return None.*/
void ESM_FifoClear(u32 u4Handle);

MRESULT ESM_FifoGetInfo(u32 u4Handle, void **pprFifoInfo);

/*/ This function will get fifo start address*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \return Fifo start address. If ESM_INVALID_ADDRESS, something is wrong.*/
MRESULT ESM_FifoGetSA(u32 u4Handle, uintptr_t *pptr4Sa);

/*/ This function will get fifo end address*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \return Fifo end address. If ESM_INVALID_ADDRESS, something is wrong.*/
MRESULT ESM_FifoGetEA(u32 u4Handle, uintptr_t *pptrEa);

/*/ This function will get fifo read pointer*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \return Fifo read pointer. If ESM_INVALID_ADDRESS, something is wrong.*/
MRESULT ESM_FifoGetRdPtr(u32 u4Handle, uintptr_t *pu4RdPtr);

/*/ This function will set fifo read pointer*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \param u4Value [IN] Fifo read pointer*/
/*/ \return If return value < 0, it's failed. Please reference drv_esm_errcode.h.*/
MRESULT ESM_FifoSetRdPtr(u32 u4Handle, u32 u4Value, bool fgFF);

/*/ This function will get fifo write pointer*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \return Fifo write pointer. If ESM_INVALID_ADDRESS, something is wrong.*/
MRESULT ESM_FifoGetWrPtr(u32 u4Handle, uintptr_t *pu4WrPtr);

/*/ This function will set fifo write pointer*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \param u4Value [IN] Fifo write pointer*/
/*/ \return If return value < 0, it's failed. Please reference drv_esm_errcode.h.*/
MRESULT ESM_FifoSetWrPtr(u32 u4Handle, uintptr_t ptrValue);

/*/ This function will increase fifo write pointer*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \param u4Value [IN] Increase fifo write pointer*/
/*/ \return If return value < 0, it's failed. Please reference drv_esm_errcode.h.*/
MRESULT ESM_FifoIncWrPtr(u32 u4Handle, u32 u4Value);

/*/ This function will set fifo memory*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \param u4Value [IN] Fifo memory size*/
/*/ \return If return value < 0, it's failed. Please reference drv_esm_errcode.h.*/
MRESULT ESM_FifoSetMem(u32 u4Handle, u32 u4Size);

/*/ This function will set to use HW or SW fifo*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \param fgUseSwFifo [IN] designate to use sw or hw fifo*/
MRESULT ESM_FifoSwitch(u32 u4Handle, bool fgUseSwFifo);

/*/ This function will get fifo available data size*/
/*/ \param u4Handle [IN] Elementary stream interface handle*/
/*/ \return Available data size. If ESM_INVALID_SIZE, something is wrong.*/
MRESULT ESM_FifoGetAvailDataSize(u32 u4Handle, u32 *pu4AvailSz);

MRESULT ESM_FifoGetSelfAvailDataSize(u32 u4Handle, u32 *pu4AvailSz);

void	ESM_CheckFifoClearStatus(u32 u4Handle);

MRESULT ESM_CheckAudDrvStatus(u32 u4Handle);

void	ESM_GetAudioFifoInfo(u32 u4Handle, DMX_FIFO_INFO_T *prFifoInfo,
			  DMX_FIFO_INFO_T *prDSPFifoInfo);

/*/ This function turns on elementary stream interface manager*/
/*/ - This API can only be called when system power on.*/
/*/ .*/
/*/ \return If return value < 0, it's failed. Please reference drv_esm_errcode.h.*/
MRESULT ESM_Init(void);

/*/ This function turns off elementary stream interface manager*/
/*/ - This API can only be called when system power off.*/
/*/ .*/
/*/ \return None.*/
void ESM_Uninit(void);

/*/ Create a elementary stream interface for connect Demuxer and Decoder*/
/*/ - Caller should call i4ESM_DestroyEsInterface to release the interface when not need anymore.*/
/*/ .*/
/*/ \return If return value < 0, it's failed. Please reference drv_esm_errcode.h.*/
MRESULT ESM_Create(
	HANDLE	hSpt,
	u32	u4FilterType,				/*/< [IN] Filter component type*/
	u32	u4FilterId,				/*/< [IN] Filter component ID*/
	u64	u8DecSendBufMask,				/*/< [IN] SW Dec Mask*/

	u32	*pu4Handle				/*/< [OUT] interface handle*/
/* Filter_OpIf** pprFilterOpIf,			  ///< [OUT] Interface for filter operation*/
/* Decoder_OpIf** pprDecoderOplIf	  ///< [OUT] Interface for decoder operation*/
);

/*/ Destroy a elementary stream interface*/
/*/ \return If return value < 0, it's failed. Please reference drv_esm_errcode.h.*/
MRESULT ESM_Destroy(
	u32	u4Handle				/*/< [IN] interface handle*/
);

MRESULT ESM_GetStmCodec(u32 u4Handle, u32 *pu4Codec);

/*/ Print ESI status*/
void ESM_PrintESIStatus(
	u32	u4Handle				/*/< [IN] interface handle*/
);

/*/ Print AU information*/
void ESM_PrintAUInfo(
	u32	u4Handle,				/*/< [IN] interface handle*/
	u32	u4StartIdx,				/*/< [IN] start index*/

	u32	u4EndIdx				/*/< [IN] end index*/
);

/*/ Print AU data info*/
void ESM_PrintAUDataInfo(
	u32	u4Handle,				/*/< [IN] interface handle*/
	u32	u4AuIdx,				/*/< [IN] AU index*/
	u32	u4StarOffset,				/*/< [IN] start offset*/

	u32	u4Length				/*/< [IN] data length*/
);

/*/ Print AU data info EX*/
void ESM_PrintAUDataInfoEx(
	u32	u4Handle,				/*/< [IN] interface handle*/
	u32	u4StartIdx,				/*/< [IN] start index*/

	u32	u4EndIdx				/*/< [IN] end index*/
);

void ESM_PrintFifoInfo(u32 u4Handle);

#ifdef __cplusplus
}
#endif

#endif /*#ifndef _DMX_ESM_IF_H_*/

