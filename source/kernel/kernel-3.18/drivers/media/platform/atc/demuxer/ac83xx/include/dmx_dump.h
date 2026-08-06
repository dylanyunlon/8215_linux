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
 * @file dmx_dump.h
 *
 * @par Project
 *
 *
 * @par Description
 *    Demuxer dump data module structures, macros, interface definitions
 *
 * @par Author_Name
 *    Shuhui Zhang
 *
 */

#ifndef DMX_INTERNAL_DUMP_H
#define DMX_INTERNAL_DUMP_H

#include "x_typedef.h"
#include "x_os.h"
#ifdef __linux__
#include <linux/mm.h>
#include <linux/errno.h>
#include <media/atc/dmx_define.h>
#include <media/atc/drv_esm_if.h>
#else
#include "drv_esm_if.h"
#include "dmx_define.h"
#endif	/* __linux__ */
#include "dmx_def.h"
#include "dmx_cli.h"
#include "dmx_spt.h"

#ifdef __cplusplus
extern "C" {

#endif

#define MAX_OF_DMX_DUMP_INFO_LEN (256)
typedef enum {
	DMX_OPER_GETAU, DMX_OPER_PSR_ON, DMX_OPER_PSR_PAUSE, DMX_OPER_PSR_OFF,
	DMX_OPER_SYNC, DMX_OPER_SYNCEX, DMX_OPER_SW_START_PTX, DMX_OPER_SET_DECRYPT_INFO,
	DMX_OPER_COMPOSE_DECRYPT_DATA, DMX_OPER_DECRYPT, DMX_OPER_HW_PTX,
	DMX_OPER_HW_PTX_DONE, DMX_OPER_FILLAU, DMX_OPER_SW_PTX_DONE, MAX_OF_DMX_OPER
} E_DMX_OPER_TYPE_T;

typedef struct {
	u32 u4FrameCnt;
	u32 u4FifoDataEa;
	HANDLE hFifoFile;
	HANDLE hAUInfoFile;
	HANDLE hDmaInfoFile;
	u64 u8FifoDataSz;
} DMX_DUMP_STM_INFO_T;

typedef struct {
	u32 u4FlowDumpCnt;
	u32 u4TickCount;
	HANDLE hFlowFile;
} DMX_DUMP_FLOW_INFO_T;

typedef struct {
	u32 u4StmType;
	u64 u8FileOfst;
	u64 u8Len;
	void *pvBuf;
} DMX_DUMP_DMA_INFO_T;

typedef enum {
	DMX_COMPOSE_DECRYPT_POS_1ST, DMX_COMPOSE_DECRYPT_POS_MID,
	DMX_COMPOSE_DECRYPT_POS_END, DMX_COMPOSE_DECRYPT_POS_WHOLE
} E_DMX_COMPOSE_DECRYPT_POS_T;

typedef struct {
	u32 u4StmType;
	u64 u8FileOfst;
	u32 u4Len;
	u64 u8SlotSrcOfst;
	u32 u4SlotDataSz;
	E_DMX_COMPOSE_DECRYPT_POS_T ePosition;
} DMX_DUMP_COMPOSE_DECRYPT_INFO_T;

typedef struct {
	u32 u4StmType;
	u64 u8FileOfst;
	u32 u4Len;
} DMX_DUMP_DECRYPT_INFO_T;

typedef struct {
	E_SPT_DATA_TYPE_T eDataType;
	u16 u2FrameKeyIdx;
	u64 u8DecryptOfst;
	u32 u4DecryptLen;
} DMX_DUMP_DECRYPT_SETTING_INFO_T;

typedef struct {
	u64 u8FileOfst;
	u64 u8Len;
} DMX_DUMP_SYNC_INFO_T;

typedef struct {
	u32 u4StmType;
	u32 u4AUIdx;
} DMX_DUMP_FILLAU_INFO_T;

typedef struct {
	u32 u4StmType;
	u32 u4AUIdx;
} DMX_DUMP_GETAU_INFO_T;

typedef struct {
	void *pvSptHdl;
	union {
		DMX_DUMP_COMPOSE_DECRYPT_INFO_T rComposeDecrypt;
		DMX_DUMP_DECRYPT_INFO_T rDecrypt;
		DMX_DUMP_DECRYPT_SETTING_INFO_T rDecryptSetting;
		DMX_DUMP_DMA_INFO_T rDma;
		DMX_DUMP_SYNC_INFO_T rSync;
		DMX_DUMP_FILLAU_INFO_T rFillAU;
		DMX_DUMP_GETAU_INFO_T rGetAU;
	} unFlow;
} DMX_DUMP_FLOW_OPER_INFO_T;

typedef struct {
	bool fgUsed;
	u32 u4RspDumpFileCnt;
	HANDLE hPbbufFifoFile;
#ifdef __linux__
	HANDLE hSema;
#else
	CRITICAL_SECTION rLock;
#endif	/* __linux__ */

	HANDLE hDmxInst;

	DMX_DUMP_FLOW_INFO_T rFlowInfo;
	u32 u4DmxStmDumpCnt[MAX_OF_DMX_CLI_SMT_CNT];
	HANDLE hRspSampleHdrFile[DMX_MAX_SPT_INST_CNT];
	DMX_DUMP_STM_INFO_T arDmxStms[MAX_OF_DMX_CLI_SMT_CNT];
	char szDumpInfo[MAX_OF_DMX_DUMP_INFO_LEN];
} DMX_DUMP_MAN_T;

void DmxDumpInit(void);
void DmxDumpDeInit(void);
void DmxDumpCloseAllFile(void);
bool DmxCreateDumpVFile(char *wszVDirName);
void DmxCloseDumpVFile(void);
void DmxDumpVFrame(AU_VPic *prVidAU, uintptr_t ptrVFifoVSa, uintptr_t ptrVFifoVEa);
void DmxDumpVDmaInfo(u64 u8FileOfst, void *pvBuf, u64 u8Len);
bool DmxCreateDumpAFile(char *wszADirName);
void DmxCloseDumpAFile(void);
void DmxDumpASample(AU_AUDIO *prAudAU, u32 u4AFifoVSa,
	u32 u4AFifoVEa, u32 u4TxUID, bool fgRsping);
void DmxDumpADmaInfo(void *pvSptHdl, u64 u8FileOfst, void *pvBuf,
	u64 u8Len, u32 u4TxUID);
void DmxDumpSPSample(AU_SP *prSPAU, u32 u4SPFifoVSa,
	u32 u4SPFifoVEa, u32 u4TxUID, bool fgRsping);
void DmxDumpSPDmaInfo(void *pvSptHdl, u64 u8FileOfst, void *pvBuf,
	u64 u8Len, u32 u4TxUID);
void DmxCloseDumpSPFile(void);
bool DmxCreateDumpSPFile(char *wszADirName);
bool DmxCreateDumpPbbufFile(u64 u8SrcOfst, char *wszFilePrefix);
void DmxCloseDumpPbbufFile(void);
void DmxDumpPbbufData(u64 u8FileOfst, void *pvBuf, u64 u8Len);
bool DmxCreateDumpRspFile(u32 u4InstId);
void DmxCloseDumpRspFile(u32 u4InstId);
void DmxDumpRspData(u32 u4InstId, RSP_HDR_MEM_NODE *prNode,
	RSP_HDR_MEM_LIST *prHdrMemList, bool fgAdd);
MRESULT DmxDumpPsrFilterInfo(void *pvPsrFtr);
MRESULT DmxDumpPsrCCInfo(void *pvPsrCC);
MRESULT DmxDumpSptInfo(void *pvSptHdl);
bool DmxCreateDumpFlowFile(char *wszVDirName);
void DmxCloseDumpFlowFile(void);
void DmxDumpFlow(E_DMX_OPER_TYPE_T eOperType, DMX_DUMP_FLOW_OPER_INFO_T *prOperInfo);
bool DmxCreateDumpFile(char *wszVDirName, HANDLE *phFile);
void DmxCloseDumpFile(HANDLE hFile);
void DmxDumpData(HANDLE hFile, u32 u4FifoRP, u32 u4FifoWP,
	u32 u4FifoSa, u32 u4FifoEa);
void DMXDumpRegisters(u32 u4StartAddr, u32 u4RegCnt);

#ifdef __cplusplus
}
#endif

#endif	/* #ifndef DMX_INTERNAL_DUMP_H */
