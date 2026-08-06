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
 * @file dmx_pvr_api.c
 *
 * @par Project
 *	  MT3360
 *
 * @par Description
 *
 *
 * @par Author_Name
 *	  Shuhui Zhang
 *
 */

/*-----------------------------------------------------------------------------*/
/* Include files*/
/*-----------------------------------------------------------------------------*/
#ifdef __linux__
#include <linux/mm.h>
#include <linux/errno.h>
#include <mach/base_regs.h>
#endif /* __linux__*/
#include "x_os.h"
#include "x_rtos.h"
#include "x_assert.h"
#include "x_printf.h"
#include "x_hal_1176.h"
#include "x_bim.h"
#include "x_ckgen.h"
#include "drv_config.h"
#include "drv_thread.h"
#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_pvr.h"
#include "dmx_pvr_if.h"
#include "dmx_pvr_ddi.h"
#include "x_ioopt.h"

#ifndef __linux__
#pragma warning(disable : 4127) /*disable warning C4127: conditional expression is constant*/
#endif

/*-----------------------------------------------------------------------------*/
/* Configurations*/
/*-----------------------------------------------------------------------------*/

/*/ Define DEBUG_MEMORY_INTRUSION to protect video FIFO from writing by other*/
/*/ agents*/
/*#define DEBUG_MEMORY_INTRUSION*/
/*-----------------------------------------------------------------------------*/
/* Constant definitions*/
/*-----------------------------------------------------------------------------*/

/*/ Default interrupt threshold of video PID (in unit of 188-byte)*/
#define DEF_INT_THRESHOLD_VIDEO				0

/*/ Default interrupt threshold of audio PID (in unit of 188-byte)*/
#define DEF_INT_THRESHOLD_AUDIO				3

/*-----------------------------------------------------------------------------*/
/* Macro definitions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Static variables*/
/*-----------------------------------------------------------------------------*/

bool _PVR_CTInit(void)
{
	/* Set Playback Global Region's Setting for TS0~4 to be 0xFF*/
	CT_TSIDX_SETTING = 0xFFFFFFFF;
	SECTIONFILTER_SETTING |= 0xFF;

	/* Set Playback Global Region's CT_SETTING to be 0(not to do VCT table check)*/
	CT_SETTING = 0x0;

	return TRUE;
}

/*-----------------------------------------------------------------------------*/
/** _PvrIsPidEnabled
 *	Is a PID enabled or disabled
 *
 *	@param	u1Pidx			PID index
 *
 *	@retval TRUE			Enabled
 *	@retval FALSE			Disabled
 */
/*-----------------------------------------------------------------------------*/
static bool _PvrIsPidEnabled(u8 u1Pidx)
{
	bool fgActive1, fgActive2;

	{
		_PVR_Lock();

		/* Bit 4 (PID on/off toggle) of the first word in PID memory*/
		fgActive1 = ((PID_S_W(u1Pidx, 0) & (1 << 4)) != 0);

		smp_mb();
		/* Bit 8 of PID index table*/
		fgActive2 = ((PID_INDEX_TABLE(u1Pidx) & (1 << 8)) != 0);
		smp_mb();

		_PVR_Unlock();
	}

	smp_mb();
	return ((fgActive1 && fgActive2) || (!fgActive1 && !fgActive2));
}

/*-----------------------------------------------------------------------------*/
/* Inter-file functions*/
/*-----------------------------------------------------------------------------*/

/** _PVR_IsPidEnabled
 *	Is a PID enabled or disabled
 *
 *	@param	u1Pidx			PID index
 *
 *	@retval TRUE			Enabled
 *	@retval FALSE			Disabled
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_IsPidEnabled(u8 u1Pidx)
{
	if (PVR_NUM_PID_INDEX <= u1Pidx) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for invalid PID Index (%u)!"),
			DMX_FUNC_NAME, u1Pidx);
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	return _PvrIsPidEnabled(u1Pidx);
}

/*-----------------------------------------------------------------------------*/
/** _PVR_SetPacketSize
 *
 *	DMX uP processes a certain amount of bytes at a time.  The amount is called
 *	a packet here.	When this function is called, DMX must have been stoped.
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_SetPacketSize(u8 u1PacketSize)
{
	u32 u4Reg;

	switch (u1PacketSize) {
	case 188:
		break;
	default:
		PVR_LOG_ERR(TEXT("[PVR] %s fail for unsupported Packet size (%u)!"),
			DMX_FUNC_NAME, u1PacketSize);
		return FALSE;
	}

	u4Reg = DMXCMD_READ32(PVR_REG_CONFIG2) & 0xFFFF00FF;
	u4Reg |= (u1PacketSize << 8);
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_CONFIG2, u4Reg);

	smp_mb();
	if (!_PVR_ResetDbmSafely()) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail in _PVR_ResetDbmSafely\r\n"), DMX_FUNC_NAME);
		return FALSE;
	}

	return TRUE;
}

void _PVR_Enable_CommandQueue(u32 u4PIDIdx)
{
	/* Enable output mode and commnand queue*/
	PID_S_W(u4PIDIdx, 0) = PID_S_W(u4PIDIdx, 0) | 0x0000000C | (1 << 6);
	mb();
}

void _PVR_Disable_CommandQueue(u32 u4PIDIdx)
{
	PID_S_W(u4PIDIdx, 0) = PID_S_W(u4PIDIdx, 0) & 0xFFFFFFF7; /* Remove bit[3]*/
	mb();
}

void _PVR_Set_CommandQueue_Pointers(u32 u4PIDIdx, u32 u4ItemCount)
{
	/* Set Read Pointer and Write Pointer*/
	/* Each Command Queue Item Uses 8 Bytes*/
	/* The space after the PID index table 10 will be use as command queue space!*/
	PID_S_W(u4PIDIdx, 17) = ((PVR_MM_COM_Q_PID_INDEX * PVR_PID_DATA_STRUCT_SIZE) << 16)
	| ((PVR_MM_COM_Q_PID_INDEX * PVR_PID_DATA_STRUCT_SIZE) + u4ItemCount * 8);
	mb();
}

bool _PVR_Fill_CommandQueue_Items(u32 u4ItemCount, PVR_CMDQ_ENTRY_T *prComQItem)
{
	u32 i = 0;

	if ((NULL == prComQItem) || (PVR_MAX_MM_COM_Q_ITEM_NUM < u4ItemCount)) {
		PVR_LOG_ERR(TEXT("[PVR] %s -- fail for invalid args(CmdQCnt: %d)\r\n"),
			DMX_FUNC_NAME, u4ItemCount);
		return FALSE;
	}

	mm_memset((void *)PVR_MM_CMDQ_ENTRY_BASE, 0, (PVR_MAX_MM_COM_Q_ITEM_NUM << 3));
	mb();

	/* Fill Skip Length and Payload Length items to DMEM*/
	/*the command queue max index is 40.becauefull!!*/
	for (i = 0; i < u4ItemCount; i++) {
		PID_S_W(PVR_MM_COM_Q_PID_INDEX, i * 2) = prComQItem[i].u4SkipLen;
		mb();
		PID_S_W(PVR_MM_COM_Q_PID_INDEX, i * 2 + 1) = prComQItem[i].u4PayloadLen;
		mb();
	}

	return TRUE;
}

MRESULT _PVR_SetTxDstFIFO(PVR_PIDCFG_INFO_T *prPIDCfgInfo)
{
	uintptr_t ptrDstFifoEa = 0;
	u16 u2PIDFilterIdx = DMX_INVALID_UINT16;
	PVR_PID_STRUCT_T *prPvrPidStruct = NULL;

	PVR_LOG_DBG(TEXT("++++++ [PVR] %s Enter +++++++\r\n"), DMX_FUNC_NAME);

	if (NULL == prPIDCfgInfo) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for invalid params\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	u2PIDFilterIdx = (u16)(prPIDCfgInfo->u4HWPIDIndex);
	smp_mb();

	if (u2PIDFilterIdx >= PVR_NUM_PID_INDEX) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for PID Index >= PVR_NUM_PID_INDEX(%d) over limitation\r\n"),
			DMX_FUNC_NAME, u2PIDFilterIdx, PVR_NUM_PID_INDEX);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPvrPidStruct = _PVR_GetPidStruct(u2PIDFilterIdx);

#if DMX_CHECK_MEM_VALIBILITY
	if (0 == DMX_PHYSICAL(prPIDCfgInfo->ptrDstFifoAddr)) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for PID Index(%d)'s DstFifoAddr(0x%x) error\r\n"),
			DMX_FUNC_NAME, u2PIDFilterIdx, prPIDCfgInfo->ptrDstFifoAddr);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	if (0 == DMX_PHYSICAL(prPIDCfgInfo->ptrDstFifoSPtr)) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for PID Index(%d)'s ptrDstFifoSPtr(0x%x) error\r\n"),
			DMX_FUNC_NAME, u2PIDFilterIdx, prPIDCfgInfo->ptrDstFifoSPtr);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	if (0 == DMX_PHYSICAL(prPIDCfgInfo->ptrDstFifoWPtr)) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for PID Index(%d)'s ptrDstFifoWPtr(0x%x) error\r\n"),
			DMX_FUNC_NAME, u2PIDFilterIdx, prPIDCfgInfo->ptrDstFifoWPtr);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	if (0 == DMX_PHYSICAL(prPIDCfgInfo->ptrDstFifoRPtr)) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for PID Index(%d)'s ptrDstFifoRPtr(0x%x) error\r\n"),
			DMX_FUNC_NAME, u2PIDFilterIdx, prPIDCfgInfo->ptrDstFifoRPtr);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
#endif /* DMX_CHECK_MEM_VALIBILITY*/
	smp_mb();

	PID_S_W(u2PIDFilterIdx, 5) = (prPIDCfgInfo->ptrDstFifoAddr) ?
		DMX_PHYSICAL(prPIDCfgInfo->ptrDstFifoAddr) : 0;
	mb();
	PID_S_W(u2PIDFilterIdx, 7) = (prPIDCfgInfo->ptrDstFifoSPtr) ?
		DMX_PHYSICAL(prPIDCfgInfo->ptrDstFifoSPtr) : 0;
	mb();
	PID_S_W(u2PIDFilterIdx, 8) = (prPIDCfgInfo->ptrDstFifoWPtr) ?
		DMX_PHYSICAL(prPIDCfgInfo->ptrDstFifoWPtr) : 0;
	mb();
	PID_S_W(u2PIDFilterIdx, 9) = (prPIDCfgInfo->ptrDstFifoRPtr) ?
		DMX_PHYSICAL(prPIDCfgInfo->ptrDstFifoRPtr) : 0;
	mb();

	smp_mb();

	if ((prPIDCfgInfo->ptrDstFifoAddr > 0) && (prPIDCfgInfo->u4DstFifoSize > 0))
		ptrDstFifoEa = prPIDCfgInfo->ptrDstFifoAddr + prPIDCfgInfo->u4DstFifoSize - 1;

	smp_mb();

	if (ptrDstFifoEa > 0) {
#if DMX_CHECK_MEM_VALIBILITY
		if (0 == DMX_PHYSICAL(ptrDstFifoEa)) {
			PVR_LOG_ERR(TEXT("[PVR] %s fail for PID Index(%d)'s ptrDstFifoEa(0x%x) error,")
				TEXT(" ptrDstFifoAddr(0x%x), DstFifoSize(0x%x)\r\n"),
				DMX_FUNC_NAME, u2PIDFilterIdx, ptrDstFifoEa,
				prPIDCfgInfo->ptrDstFifoAddr, prPIDCfgInfo->u4DstFifoSize);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
#endif /* DMX_CHECK_MEM_VALIBILITY*/
		PID_S_W(u2PIDFilterIdx, 6) = (ptrDstFifoEa) ? ((u32)DMX_PHYSICAL(ptrDstFifoEa)) : 0;
		mb();
	} else {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for destination fifo's ea == 0 (PidIdx: %d)! +++++++\r\n"),
			DMX_FUNC_NAME, u2PIDFilterIdx);
	}

	PVR_LOG_DBG(TEXT("------- [PVR] %s Exit --------\r\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

MRESULT _PVR_SetPIDFilterNtyFunc(
	u32			u4PIDIndex,
	PFN_PVR_NOTIFY		pfnNotify,
	void			*pvNotifyTag)
{
	PVR_PID_STRUCT_T *prPvrPidStruct = _PVR_GetPidStruct(u4PIDIndex);

	PVR_LOG_DBG(TEXT("++++++ [PVR] %s Enter +++++++\r\n"), DMX_FUNC_NAME);

	if (NULL == prPvrPidStruct) {
		PVR_LOG_ERR(TEXT("[PVR] %s failed for can't found the PID Data structure whose PidIdx = %s\r\n"),
			DMX_FUNC_NAME, u4PIDIndex);
		MM_RETURN(RET_DMX_NOT_FOUND);
	}

	smp_mb();
	_PVR_Lock();
	prPvrPidStruct->pfnNotify = pfnNotify;
	prPvrPidStruct->pvNotifyTag = pvNotifyTag;
	_PVR_Unlock();

	PVR_LOG_DBG(TEXT("------- [PVR] %s Exit --------\r\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

MRESULT _PVR_SetGlobalCBFunc(
	DMX_HAL_FUNC_CB pfnCB,
	void		*pvUserPrivate)
{
	DMX_HAL_FUNC_INFO_T *prFuncInfo = NULL;

	PVR_LOG_DBG(TEXT("++++++ [PVR] %s Enter +++++++\r\n"), DMX_FUNC_NAME);

	prFuncInfo = _PVR_GetGlobalCbInfo();

	if (NULL == prFuncInfo) {
		PVR_LOG_ERR(TEXT("[PVR] %s failed for can't found the PVR Global Callback info\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_NOT_FOUND);
	}

	smp_mb();
	_PVR_Lock();
	prFuncInfo->pfnCB = pfnCB;
	prFuncInfo->pvPrivData = pvUserPrivate;
	_PVR_Unlock();

	PVR_LOG_DBG(TEXT("------- [PVR] %s Exit --------\r\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

bool _PVR_SetIgnoreCCCheck(u8 u1Pidx, bool fgEnable)
{
	if (u1Pidx >= PVR_NUM_PID_INDEX) {
		PVR_LOG_ERR(TEXT("%s line %d fail for Invalid PID index: %u\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u1Pidx);
		return FALSE;
	}

	smp_mb();
	if (fgEnable)
		/* Set DI to 1 to ignore CC check when playback*/
		PID_S_W(u1Pidx, 0) = PID_S_W(u1Pidx, 0) | (0x1 << 12);
	else
		PID_S_W(u1Pidx, 0) = PID_S_W(u1Pidx, 0) & ~(0x1 << 12);

	mb();

	return TRUE;
}

/*-----------------------------------------------------------------------------*/
/** _PVR_SetBypassMode
 *	Set demux (DBM) to bypass mode, mainly used for frontend capture function
 *
 *	@param	u1TsIndex		TS index (0 - 1)
 *	@param	u1PacketSize		TS packet size in byte
 *	@param	fgSteerToFTuP		Steer to FTuP or not
 *	@param	fgReset			Reset FTI or not
 *
 *	@retval	TRUE			Succeed
 *	@retval	FALSE			Fail
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_SetBypassMode(u8 u1TsIndex, u8 u1PacketSize,
						bool fgSteerToFTuP, bool fgReset)
{
	u32 u4Cfg, u4RegBypass, i;

	FUNC_ENTRY;

	if (u1TsIndex > 4) {
		PVR_LOG_ERR(TEXT("%s line %d fail for Invalid Ts index: %u\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u1TsIndex);
		FUNC_EXIT;
		return FALSE;
	}

	smp_mb();

	if (fgReset) {
		/* Reset Demux*/
		if (!_PVR_Reset()) {
			PVR_LOG_ERR(TEXT("%s line %d fail in PVR_Reset\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			FUNC_EXIT;
			return FALSE;
		}
	}

	smp_mb();

	/* Set packet size*/
	u4Cfg = DMXCMD_READ32(PVR_REG_CONFIG2);
	u4Cfg &= 0xffff00ff;
	u4Cfg |= (u1PacketSize << 8);
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_CONFIG2, u4Cfg);

	/* reset DBM*/
	u4Cfg = DMXCMD_READ32(PVR_REG_DBM_CONTROL);
	u4Cfg |= 0x80000000;
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_DBM_CONTROL, u4Cfg);
	u4Cfg &= 0x7fffffff;
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_DBM_CONTROL, u4Cfg);

	smp_mb();
	_PVR_SetDbmChannel4(FALSE, FALSE); /* disable dbm channel4 default.*/

	smp_mb();
	/* Disable framer packet error handling*/
	if (u1TsIndex < PVR_FRAMER_COUNT)
		_PVR_SetFramerPacketErrorHandling(u1TsIndex, FALSE, 0);

	smp_mb();

	/* Set DBM to normal mode and keep all TS packets*/
	u4Cfg = DMXCMD_READ32(PVR_REG_DBM_CONTROL);
	u4Cfg &= 0xc1ffffff;
	if (u1TsIndex != 4)
		u4Cfg |= (1 << (u1TsIndex + 26));
	else
		u4Cfg |= (1 << 25);
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_DBM_CONTROL, u4Cfg);

	smp_mb();

	u4RegBypass = 0;
	switch (u1TsIndex) {
	case 0:
		u4RegBypass = PVR_REG_DBM_BYPASS_PID;
		break;
	case 1:
		u4RegBypass = PVR_REG_DBM_BYPASS_PID_2;
		break;
	case 2:
		u4RegBypass = PVR_REG_DBM_BYPASS_PID_3;
		break;
	case 3:
		u4RegBypass = PVR_REG_DBM_BYPASS_PID_4;
		break;
	default:
		DMX_ASSERT(FALSE);
		break;
	}

	smp_mb();

	if (fgSteerToFTuP)
		/* Set DBM Bypass PID register, steer to FTuP*/
		DMXCMD_WRITE32(u4RegBypass, 0x20);
	else
		/* Set DBM Bypass PID register, steer to null*/
		DMXCMD_WRITE32(u4RegBypass, 0x0);

	smp_mb();

	/* Disable all PIDs*/
	for (i = 0; i < PVR_NUM_PID_INDEX; i++)
		PID_INDEX_TABLE(i) &= 0x7fffffff;

	smp_mb();
	/* Set packet length to 188 bytes*/
	u4Cfg = DMXCMD_READ32(PVR_REG_CONFIG2);
	u4Cfg = (u4Cfg & 0xFFFFFF00) | 0x08;			/* maximum burst size*/
	u4Cfg = (u4Cfg & 0xFFFF00FF) | (188 << 8);		/* packet len = 188 bytes*/
	u4Cfg = (u4Cfg & 0xFF00FFFF) | (0x47 << 16);	/* TS packet sync byte*/
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_CONFIG2, u4Cfg);

	smp_mb();
	/*Disable dbm channel5 default*/
	_PVR_SetDbmChannel4(FALSE, FALSE);

	smp_mb();

	u4Cfg = DMXCMD_READ32(PVR_REG_CONFIG1);
	u4Cfg &= 0xffffEf00;				/* Select Group 1 DRAM agent*/
	u4Cfg |= (PVR_DMEM_ENTRY_LEN * 4);		/* Set Playback PID structure sizea*/
	u4Cfg |= (1 << 9);				/* DMA delay ack, debug*/
	u4Cfg |= (1 << 10);				/* Enable multi-PID channel*/
	/* Direct map (Record PID data structure index is direct map to PID index)*/
	u4Cfg |= (1 << 8);
	u4Cfg |= (1 << 12);				/* Select Group 2 DRAM agent(agent_1)*/
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_CONFIG1, u4Cfg);

	smp_mb();
	if (fgReset)
		/* Enable Demux*/
		_PVR_EnableFTI(TRUE);

	FUNC_EXIT;

	return TRUE;
}

bool _PVR_Set_PIDChunkSize(u32 u4PIDIdx, u32 u4ChunkSize)
{
	if (u4PIDIdx >= PVR_NUM_PID_INDEX) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for u4PidIdx(%d) >= PVR_NUM_PID_INDEX(%d)!\r\n"),
			DMX_FUNC_NAME, u4PIDIdx, PVR_NUM_PID_INDEX);
		DMX_ASSERT(FALSE);
		return FALSE;
	}
	smp_mb();

	PID_S_W(u4PIDIdx, 1) = u4ChunkSize; /*Chunk Size*/
	mb();

	return	TRUE;
}

bool _PVR_Set_PIDTriggerFlag(u32 u4PIDIdx, bool bEnable)
{
	u32 u4Temp = 0;

	if (u4PIDIdx >= PVR_NUM_PID_INDEX) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for u4PidIdx(%d) >= PVR_NUM_PID_INDEX(%d)!\r\n"),
			DMX_FUNC_NAME, u4PIDIdx, PVR_NUM_PID_INDEX);
		DMX_ASSERT(FALSE);
		return FALSE;
	}
	smp_mb();
	mb();
	u4Temp = PID_S_W(u4PIDIdx, 3) & 0xFFFFFF00;
	smp_mb();
	PID_S_W(u4PIDIdx, 3) = u4Temp|(bEnable ? 1 : 0);
	mb();
	return TRUE;
}

bool _PVR_Set_MM_InsertBytes(u32 u4PIDIdx, PVR_INST_BYTES_INFO_T *prInstBytesInfo)
{
	u32 u4Words	= 0;
	u32 u4Idx = 0;
	u8  *pu1Buf	= prInstBytesInfo->pu1InsertBuf;
	u32 u4Ctrl = 0;

	if (!prInstBytesInfo->fgInsertBytes)
		return TRUE;

	if ((prInstBytesInfo->u4InsertLen <= 0) || (prInstBytesInfo->u4Inserttimes <= 0)) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for u4InstTimes <= 0 or u4InsertLen <= 0!\r\n"),
			DMX_FUNC_NAME, u4PIDIdx, PVR_NUM_PID_INDEX);
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	if (prInstBytesInfo->u4InsertLen > PVR_MAX_INST_BYTES_CNT) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for PidIdx(%d)'s u4InstTimes(%d) > Limitation(%d)!\r\n"),
			DMX_FUNC_NAME, u4PIDIdx, prInstBytesInfo->u4InsertLen,
			PVR_MAX_INST_BYTES_CNT);
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	u4Words = PID_S_W(u4PIDIdx, 19);
	u4Words |= ((prInstBytesInfo->u4Inserttimes) << 16);  /* Insert Bytes Times*/
	PID_S_W(u4PIDIdx, 19) = u4Words;

	u4Words = 0;
	u4Ctrl	= 0;
	for (u4Idx = 0; u4Idx < prInstBytesInfo->u4InsertLen; u4Idx++) {
		u4Words = (u4Words << 8) | pu1Buf[u4Idx];
		u4Ctrl	= (u4Ctrl << 1) | 1;
		if (u4Idx % 4 == 3) {
			PID_S_W(u4PIDIdx, 20) = u4Words;
			u4Words = 0;
		}
	}

	if (u4Idx > 4)
		PID_S_W(u4PIDIdx, 21) = u4Words;
	else if (u4Idx < 4)
		PID_S_W(u4PIDIdx, 20) = u4Words;

	PID_S_W(u4PIDIdx, 21) |= (u4Ctrl << 24);  /* Insert Byte Valid(Bit Control Which byte is insert)*/

	return TRUE;
}



bool _PVR_Set_MM_PIDDRMMode(u32 u4PIDIdx, PVR_DRM_PARAM_T *prDRMParam)
{
	u32 u4Words = 0;

	if (u4PIDIdx >= PVR_NUM_PID_INDEX) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for u4PidIdx(%d) >= PVR_NUM_PID_INDEX(%d)!\r\n"),
			DMX_FUNC_NAME, u4PIDIdx, PVR_NUM_PID_INDEX);
		DMX_ASSERT(FALSE);
		return FALSE;
	}
	smp_mb();
	mb();
	u4Words = PID_S_W(u4PIDIdx, 0);
	smp_mb();
	u4Words = 0x00FF00FF & u4Words;
	switch (prDRMParam->eMode) {
	case PVR_DRM_MODE_BYPASS:
		PID_S_W(u4PIDIdx, 0)  = u4Words;
		PID_S_W(u4PIDIdx, 2)  = 0;
		PID_S_W(u4PIDIdx, 15) = 0;
		mb();
		PVR_LOG_ERR(TEXT("[PVR] %s -- eMode bypass, PID_S_W(%d, 0) = 0x%x,")
			TEXT(" PID_S_W(%d, 2) = 0x%x, PID_S_W(%d, 15) = 0x%x!\r\n"),
			DMX_FUNC_NAME, u4PIDIdx, u4Words,
			u4PIDIdx, PID_S_W(u4PIDIdx, 2),
			u4PIDIdx, PID_S_W(u4PIDIdx, 15));
		break;
	case PVR_DRM_MODE_AES:
		{
			u4Words = (0x01 << 24) | u4Words; /* AES*/
			smp_mb();

			if (prDRMParam->fgCbc)
				u4Words = (0x01 << 8) | u4Words; /* 1: CBC, 0: ECB*/

			if (prDRMParam->fgDoEncrypt)
				u4Words = (0x01 << 9) | u4Words; /* 1: Do Encryption, 0: Do Decryption*/

			if (128 == prDRMParam->u2KeyLen) {
				u4Words = (0x00 << 10) | u4Words; /* b'00: 128bit*/
			} else if (192 == prDRMParam->u2KeyLen) {
				u4Words = (0x01 << 10) | u4Words; /* b'01: 192bit*/
			} else if (256 == prDRMParam->u2KeyLen) {
				u4Words = (0x02 << 10) | u4Words; /* b'10: 256bit*/
			} else {
				PVR_LOG_ERR(TEXT("[PVR] %s fail for invalid key len (%d)!\r\n"),
					DMX_FUNC_NAME, prDRMParam->u2KeyLen);
				DMX_ASSERT(FALSE);
				return FALSE;
			}

			PVR_LOG_DBG(TEXT("[PVR] %s	-- _PVR_SetDmemAesKey!\r\n"),
				DMX_FUNC_NAME);
			smp_mb();

			_PVR_SetDmemAesKey(prDRMParam->u2KeyLen, prDRMParam->au1Key);

			smp_mb();
			if (prDRMParam->fgCbc) {
				PVR_LOG_ERR(TEXT("[PVR] %s	-- _PVR_SetDmemAesIV!\r\n"),
					DMX_FUNC_NAME);
				_PVR_SetDmemAesIV(prDRMParam->u2KeyLen, prDRMParam->au1InitVector);
			}

			smp_mb();

			PID_S_W(u4PIDIdx, 0)  = u4Words;
			mb();
			PID_S_W(u4PIDIdx, 2)  = prDRMParam->u4DecryptLen;
			mb();
			PID_S_W(u4PIDIdx, 15) = prDRMParam->u4DecryptOfst;
			mb();

			PVR_LOG_DBG(TEXT("[PVR] %s -- PID_S_W(%d, 0) = 0x%x, PID_S_W(%d, 2)")
				TEXT(" = 0x%x, PID_S_W(%d, 15) = 0x%x!\r\n"),
				DMX_FUNC_NAME, u4PIDIdx, u4Words,
				u4PIDIdx, PID_S_W(u4PIDIdx, 2),
				u4PIDIdx, PID_S_W(u4PIDIdx, 15));
		}
		break;
	default:
		break;
	}

	return TRUE;
}

bool _PVR_Set_BypassPIDIdx(u8 u1TsIndex, u32 u4PIDIdx, bool fgToFtup)
{
	u32	u4RegVal = 0;
	u32	u4RegBypass = 0;

	if (PVR_NUM_PID_INDEX <= u4PIDIdx) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for Pid Index(%d) error!\r\n"),
			DMX_FUNC_NAME, u4PIDIdx);
		return FALSE;
	}

	switch (u1TsIndex) {
	case 0:
		u4RegBypass = PVR_REG_DBM_BYPASS_PID;
		break;
	case 1:
		u4RegBypass = PVR_REG_DBM_BYPASS_PID_2;
		break;
	case 2:
		u4RegBypass = PVR_REG_DBM_BYPASS_PID_3;
		break;
	case 3:
		u4RegBypass = PVR_REG_DBM_BYPASS_PID_4;
		break;

	default:
		PVR_LOG_ERR(TEXT("[PVR] %s fail for invalid ts index(0x%02x) error!\r\n"),
			DMX_FUNC_NAME, u1TsIndex);
		DMX_ASSERT(FALSE);
		return FALSE;
	}
	smp_mb();

	/*/> Force to make DBM select TS3 as input source*/
	DMXCMD_WRITE32(PVR_REG_DBM_CONTROL, 0x60030208);

	smp_mb();
	/*/> Set ByPass Pid Index*/
	u4RegVal |= (u4PIDIdx << 16);

	smp_mb();
	/*/> Set Control Bits*/
	if (fgToFtup)
		u4RegVal |= (1 << 5); /* To Ftup*/
	smp_mb();

	DMXCMD_WRITE32(u4RegBypass, u4RegVal);

	PVR_LOG_DBG(TEXT("[PVR] %s -- ByPassPID4 Reg: 0x%x, DBMCtrl Reg: 0x%x!\r\n"),
		DMX_FUNC_NAME, DMXCMD_READ32(u4RegBypass), DMXCMD_READ32(PVR_REG_DBM_CONTROL));

	return TRUE;
}

/*-----------------------------------------------------------------------------*/
/*PVR_SetFramerPacketErrorHandling*/
/*-----------------------------------------------------------------------------*/
bool _PVR_SetFramerPacketErrorHandling(u8 u1TsIndex, bool fgEnable, u32 u4Value)
{
	u32 u4Reg = 0;

	PVR_LOG_DBG(TEXT("[PVR] %s --  (framer %d, fgEnable: %s, u4Value: 0x%x)  -- enter\r\n"),
		DMX_FUNC_NAME, u1TsIndex, (fgEnable ? "TRUE" : "FALSE"), u4Value);

	if (u1TsIndex >= PVR_FRAMER_COUNT)
		return TRUE;

	smp_mb();
	if (u1TsIndex == 0) {
		u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_ERROR_HANDLE) & 0xFFFF0000;
		if (fgEnable)
			u4Reg |= u4Value;
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_FRAMER_ERROR_HANDLE, u4Reg);
	} else if (u1TsIndex == 1) {
		u4Reg = DMXCMD_READ32(PVR_REG_FRAMER_ERROR_HANDLE) & 0x0000FFFF;
		if (fgEnable)
			u4Reg |= (u4Value << 16);
		smp_mb();
		DMXCMD_WRITE32(PVR_REG_FRAMER_ERROR_HANDLE, u4Reg);
	} else {
		PVR_LOG_ERR(TEXT("%s (framer %d, fgEnable: %s, u4Value: 0x%x)  -- fail for invalid framer idx(%u)\r\n"),
			DMX_FUNC_NAME, u1TsIndex, (fgEnable ? "TRUE" : "FALSE"), u4Value, u1TsIndex);
		return FALSE;
	}

	PVR_LOG_DBG(TEXT("%s (framer %d, fgEnable: %s, u4Value: 0x%x)  -- exit\r\n"),
		DMX_FUNC_NAME, u1TsIndex, (fgEnable ? "TRUE" : "FALSE"), u4Value);

	return TRUE;
}

bool _PVR_Set_DefaultPIDDataStruct(void)
{
	/*/> MM(bit2~3 = 2b'11), To CD-FIFO(bit 6), Enable Picture Finder(bit 7)*/

	/* Fill PID Data struct default values for Video, Audio, SP, Section*/

	/* Video*/
	PID_S_W(PVR_PID_IDX_VIDEO, 0) = (1 << 1)|(1 << 2)|(1 << 6);
	PID_S_W(PVR_PID_IDX_VIDEO, 1) = 0x00000000; /*Chunk Size*/
	PID_S_W(PVR_PID_IDX_VIDEO, 2) = 0x00000000;
	PID_S_W(PVR_PID_IDX_VIDEO, 3) = 0x00000000;
	PID_S_W(PVR_PID_IDX_VIDEO, 4) = 0xFFFFFFFF;
	PID_S_W(PVR_PID_IDX_VIDEO, 18) = 0xFFFFFFFF;
	PID_S_W(PVR_PID_IDX_VIDEO, 0) |= 0x0FF << 16;

	mb();

	/* Audio*/
	PID_S_W(PVR_PID_IDX_AUDIO, 0) = (1 << 1) | (1 << 2); /* MM(bit2~3 = 2b'11), To CD-FIFO(bit 6)*/
	PID_S_W(PVR_PID_IDX_AUDIO, 1) = 0x00000000; /*Chunk Size*/
	PID_S_W(PVR_PID_IDX_AUDIO, 2) = 0x00000000;
	PID_S_W(PVR_PID_IDX_AUDIO, 3) = 0x00000000;
	PID_S_W(PVR_PID_IDX_AUDIO, 4) = 0xFFFFFFFF;
	PID_S_W(PVR_PID_IDX_AUDIO, 18) = 0xFFFFFFFF;

	mb();
	/* SP*/
	PID_S_W(PVR_PID_IDX_SP, 0) = (1 << 1) | (1 << 2); /* MM(bit2~3 = 2b'11)*/
	PID_S_W(PVR_PID_IDX_SP, 1) = 0x00000000; /*Chunk Size*/
	PID_S_W(PVR_PID_IDX_SP, 2) = 0x00000000;
	PID_S_W(PVR_PID_IDX_SP, 3) = 0x00000000;
	PID_S_W(PVR_PID_IDX_SP, 4) = 0xFFFFFFFF;
	PID_S_W(PVR_PID_IDX_SP, 18) = 0xFFFFFFFF;

	mb();
	/* Section*/
	PID_S_W(PVR_PID_IDX_SECTION, 0) = (1 << 1) | (1 << 2); /* MM(bit2~3 = 2b'11)*/
	PID_S_W(PVR_PID_IDX_SECTION, 1) = 0x00000000; /*Chunk Size*/
	PID_S_W(PVR_PID_IDX_SECTION, 2) = 0x00000000;
	PID_S_W(PVR_PID_IDX_SECTION, 3) = 0x00000000;
	PID_S_W(PVR_PID_IDX_SECTION, 4) = 0xFFFFFFFF;
	PID_S_W(PVR_PID_IDX_SECTION, 18) = 0xFFFFFFFF;
	mb();

	return TRUE;
}

