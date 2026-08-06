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
 * @file dmx_pvr_utils.c
 *
 * @par Project
 *	  MT3360
 *
 * @par Description
 *	  Demuxer pvr utilities interfaces definitions
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
#include <media/atc/ose_mem.h>
#else
#include "OSE_mem.h"
#endif /* __linux__*/

#include "x_os.h"
#include "drv_config.h"
#include "x_lint.h"
#include "dmx_pvr.h"
#include "x_assert.h"
#include "x_printf.h"
#include "dmx_mem.h"
#include "dmx_pvr_ddi.h"


/*-----------------------------------------------------------------------------*/
/* Configurations*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Constant definitions*/
/*/-----------------------------------------------------------------------------*/

/* The number of bytes in a TS packet header.*/
#define TS_HEADER_LEN					(u8)4

/* The number of bytes in the PES prefix header.*/
#define PES_HEADER_LEN					(u8)6

/* The index of the PES_packet_length from the PES header.*/
#define PES_PACKET_LEN0_IDX				(u8)4
#define PES_PACKET_LEN1_IDX				(u8)5


/*-----------------------------------------------------------------------------*/
/* Type definitions*/
/*-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------*/
/* Macro definitions*/
/*-----------------------------------------------------------------------------*/
#define DWORDSWAP(u4Tmp) (((u4Tmp & 0xff) << 24) | ((u4Tmp & 0xff00) << 8) |\
	((u4Tmp & 0xff0000) >> 8) | ((u4Tmp & 0xff000000) >> 24))


/*-----------------------------------------------------------------------------*/
/* Include files*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Static functions*/
/*-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------*/
/* Inter-file functions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/** _DmxGetByte
 *	Get a byte from a ring buffer
 *
 *	@param	ppu1StartAddr	The start address
 *	@param	u4EndAddr		End address
 *	@param	u4BufLen		Buffer size
 *	@param	u4Offset		Offset
 *	@param	u1Forward		1: forward direction, -1: backward direction
 *
 *	@retval TRUE			Succeed
 *	@retval FALSE			Fail
 */
/*-----------------------------------------------------------------------------*/
u8 _PVR_GetByte(u8 **ppu1StartAddr, uintptr_t ptrEndAddr,
				   u32 u4BufLen, u32 u4Offset, u8 u1Forward)
{
	u32 i;
	u8 u1Ret = 0;

	if (ppu1StartAddr == NULL)
		return 0;

	for (i = 0; i < u4Offset; i++) {
		(*ppu1StartAddr) += 1;
		if ((uintptr_t)(*ppu1StartAddr) >= ptrEndAddr)
			(*ppu1StartAddr) -= u4BufLen;
	}

	u1Ret = (**ppu1StartAddr);

	(*ppu1StartAddr) += u1Forward;
	if ((uintptr_t)(*ppu1StartAddr) >= ptrEndAddr)
		(*ppu1StartAddr) -= u4BufLen;

	return u1Ret;
}


/*-----------------------------------------------------------------------------*/
/** _PVR_Align
 *	Get an aligned address which is equals to or greater than the original
 *	address
 *
 *	@param	u4Addr			The original address
 *	@param	u4Alignment		The alignment requirement in byte
 *
 *	@retval The aligned address
 */
/*-----------------------------------------------------------------------------*/
u32 _PVR_Align(u32 u4Addr, u32 u4Alignment)
{
	u32 u4Unaligned;

	if (u4Alignment <= 1)
		return u4Addr;

	u4Unaligned = u4Addr % u4Alignment;
	if (u4Unaligned != 0)
		u4Addr += u4Alignment - u4Unaligned;

	return u4Addr;
}


/*-----------------------------------------------------------------------------*/
/** _PVR_IsAligned
 *	Check if an address is aligned or not
 *
 *	@param	u4Addr			The address to be checked
 *	@param	u4Alignment		The alignment
 *
 *	@retval TRUE			Yes
 *	@retval FALSE			No
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_IsAligned(u32 u4Addr, u32 u4Alignment)
{
	return u4Addr == _PVR_Align(u4Addr, u4Alignment);
}


/*-----------------------------------------------------------------------------*/
/** _PVR_AdvanceAddr
 *	Update address in a ring buffer by the given increment value
 *
 *	@param	u4Addr			The address to be updated
 *	@param	i4Increment		The address increment. Negtive value means
 *							backward increment.
 *	@param	u4Wp			Current write pointer
 *	@param	u4FifoStart		FIFO start address
 *	@param	u4FifoEnd		FIFO end address
 *
 *	@retval The new address, or 0 if no enough data for the given increment
 */
/*-----------------------------------------------------------------------------*/
uintptr_t _PVR_AdvanceAddr(uintptr_t ptrAddr, s32 i4Increment, uintptr_t ptrWp,
						uintptr_t ptrFifoStart, uintptr_t ptrFifoEnd)
{
	u32 u4DataSize0, u4DataSize, u4FifoSize;

	/* Debug*/
	if ((ptrAddr < ptrFifoStart) || (ptrAddr >= ptrFifoEnd))
		return 0;

	DMX_ASSERT((ptrAddr >= ptrFifoStart) && (ptrAddr < ptrFifoEnd));
	DMX_ASSERT((ptrWp >= ptrFifoStart) && (ptrWp < ptrFifoEnd));

	u4FifoSize = ptrFifoEnd - ptrFifoStart;
	u4DataSize0 = DMX_DATASIZE(ptrAddr, ptrWp, u4FifoSize);

	ptrAddr = (ptrAddr + i4Increment);
	if (ptrAddr >= ptrFifoEnd)
		ptrAddr -= u4FifoSize;
	if (ptrAddr < ptrFifoStart)
		ptrAddr += u4FifoSize;

	u4DataSize = DMX_DATASIZE(ptrAddr, ptrWp, u4FifoSize);
	if (i4Increment > 0) {
		if (u4DataSize > u4DataSize0)
			/* Out of data*/
			return 0;
	} else {
		if (u4DataSize < u4DataSize0)
			return 0;
	}

	return ptrAddr;
}

u32 _PVR_Align_Dec(u32 u4Addr, u32 u4Alignment, u8 *pu1SkipBytes)
{
	u32 u4Unaligned = 0;
	u8  u1SkipTemp = 0;

	if (u4Alignment <= 1) {
		if (NULL != pu1SkipBytes)
			*pu1SkipBytes = 0;

		return u4Addr;
	}

	u4Unaligned = u4Addr % u4Alignment;
	if (u4Unaligned != 0) {
		u4Addr -= u4Unaligned;
		u1SkipTemp = (u8)u4Unaligned;
	} else {
		u1SkipTemp = 0;
	}

	if (NULL != pu1SkipBytes)
		*pu1SkipBytes = u1SkipTemp;

	return u4Addr;
}

/*-----------------------------------------------------------------------------*/
/** _PVR_CopyRingBuffer
 *	Copy data from a ring buffer to another ring buffer
 *
 *	@param	u4Dst			Destination address
 *	@param	u4DstStart		Destination buffer start address
 *	@param	u4DstEnd		Destination buffer end address
 *	@param	u4Src			Source address
 *	@param	u4SrcStart		Source buffer start address
 *	@param	u4SrcEnd		Source buffer end address
 *	@param	u4Size			Size to copy
 *
 *	@retval The destination address
 */
/*-----------------------------------------------------------------------------*/
u32 _PVR_CopyRingBuffer(uintptr_t ptrDst, uintptr_t ptrDstStart, uintptr_t ptrDstEnd,
						   uintptr_t ptrSrc, uintptr_t ptrSrcStart, uintptr_t ptrSrcEnd, u32 u4Size)
{
	u32 i;
	u8 *pSrc, *pDst;

	DMX_ASSERT((ptrDst >= ptrDstStart) && (ptrDst < ptrDstEnd));
	DMX_ASSERT((ptrSrc >= ptrSrcStart) && (ptrSrc < ptrSrcEnd));

	pSrc = (u8 *)ptrSrc;
	pDst = (u8 *)ptrDst;

	for (i = 0; i < u4Size; i++) {
		*pDst++ = *pSrc++;

		if ((uintptr_t)pDst >= ptrDstEnd)
			pDst = (u8 *)ptrDstStart;

		if ((uintptr_t)pSrc >= ptrSrcEnd)
			pSrc = (u8 *)ptrSrcStart;
	}

	return ptrDst;
}


bool _PVR_CopyDestRingBuffer(uintptr_t ptrDst, uintptr_t ptrBufStart, uintptr_t ptrBufEnd,
							 uintptr_t ptrSrc, u32 u4Size)
{
	u32 u4BufSize = 0;

	/* Check if destination address is in buffer range*/
	if ((ptrDst < ptrBufStart) || (ptrDst >= ptrBufEnd))
		return FALSE;

	/* Check if buffer size is sufficient*/
	u4BufSize = ptrBufEnd - ptrBufStart;
	if (u4Size > u4BufSize)
		return FALSE;

	/* Copy data to DDI buffer*/
	if (u4Size > 0) {
		u32 u4StepSize = 0;

		u4StepSize = MIN(ptrBufEnd - ptrDst, u4Size);
		mm_memcpy((void *)ptrDst, (void *)ptrSrc, u4StepSize);

		ptrSrc += u4StepSize;
		ptrDst += u4StepSize;
		if (ptrDst >= ptrBufEnd)
			ptrDst = ptrBufStart;

		u4Size -= u4StepSize;
	}

	if (u4Size > 0)
		mm_memcpy((void *)ptrDst, (void *)ptrSrc, u4Size);

	return TRUE;
}

void _PVR_DumpBuffer(const u8 au1Buf[], u32 u4Size,
				 u32 u4BytesPerLine)
{
	u32 i, j;

	if (au1Buf == NULL)
		return;

	j = 0;
	for (i = 0; i < u4Size; i++) {
		if (j == 0)
			PVR_LOG_TRACE(TEXT("\n%08x:"), i);

		PVR_LOG_TRACE(TEXT(" 0x%02x"), au1Buf[i]);

		if (j == 7)
			PVR_LOG_TRACE(TEXT(" | \n"));

		j++;
		if (j == u4BytesPerLine)
			j = 0;
	}
}

void _PVR_DumpRegisters(u32 u41stRegAddr, u32 u4RegsCnt)
{
	u32 i, u4Reg, u4RegAddr;

	PVR_LOG_TRACE(TEXT("BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\r\n"));
	PVR_LOG_TRACE(TEXT("[DMX] Begin to dump Register [0x%08x, 0x%08x] Value\r\n"),
		u41stRegAddr, u41stRegAddr + u4RegsCnt * 4);
	if ((u41stRegAddr >= DEMUX0_BASE) && (u41stRegAddr < DEMUX0_BASE + 0x1000)) {
		for (i = 0; i < u4RegsCnt; i++) {
			u4RegAddr = u41stRegAddr + i * 4;
			u4Reg = DMXCMD_READ32((u4RegAddr - DEMUX0_BASE) / 4);
			if (((u4RegAddr & 0x0000000F) == 0x00000000) || (i == 0))
				PVR_LOG_TRACE(TEXT("0x%08x ---> 0x%08x"), u4RegAddr, u4Reg);
			else if ((u4RegAddr & 0x0000000F) == 0x0000000C)
				PVR_LOG_TRACE(TEXT(" 0x%08x\r\n"), u4Reg);
			else
				PVR_LOG_TRACE(TEXT(" 0x%08x \r\n"), u4Reg);
		}

	} else if ((u41stRegAddr >= DDI_BASE) && (u41stRegAddr < DDI_BASE + 0x1000)) {
		for (i = 0; i < u4RegsCnt; i++) {
			u4RegAddr = u41stRegAddr + i * 4;
			u4Reg = DDI_READ32(u4RegAddr - DDI_BASE);
			if (((u4RegAddr & 0x0000000F) == 0x00000000) || (i == 0))
				PVR_LOG_TRACE(TEXT("0x%08x ---> 0x%08x"), u4RegAddr, u4Reg);
			else if ((u4RegAddr & 0x0000000F) == 0x0000000C)
				PVR_LOG_TRACE(TEXT(" 0x%08x\r\n"), u4Reg);
			else
				PVR_LOG_TRACE(TEXT(" 0x%08x \r\n"), u4Reg);
		}

	} else {
	}

	PVR_LOG_TRACE(TEXT("[DMX] End to dump Register [0x%08x, 0x%08x] Value\r\n"),
		u41stRegAddr, u41stRegAddr + u4RegsCnt * 4);
	PVR_LOG_TRACE(TEXT("EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\r\n"));
}

/*-----------------------------------------------------------------------------*/
/* Static functions*/
/*-----------------------------------------------------------------------------*/
bool _PVR_DumpPIDDataStruct(u8 u1PidIdx)
{
	s32 u4WordIdx = 0;

	PVR_LOG_TRACE(TEXT("============================================================ \r\n"));
	switch (u1PidIdx) {
	case PVR_PID_IDX_VIDEO:
		PVR_LOG_TRACE(TEXT("[PVR] Dump %s's Pid Structure as follow:\r\n"), "Video");
		break;
	case PVR_PID_IDX_AUDIO:
		PVR_LOG_TRACE(TEXT("[PVR] Dump %s's Pid Structure as follow:\r\n"), "Audio");
		break;
	case PVR_PID_IDX_SP:
		PVR_LOG_TRACE(TEXT("[PVR] Dump %s's Pid Structure as follow:\r\n"), "SP");
		break;
	case PVR_MM_COM_Q_PID_INDEX:
		PVR_LOG_TRACE(TEXT("[PVR] Dump %s's Pid Structure as follow:\r\n"), "CmdQ");
		_PVR_DumpCmdQInfo();
		return TRUE;
	default:
		return FALSE;
	}

	if (u1PidIdx >= (u8)PVR_NUM_PID_INDEX) {
		s32 i;

		for (i = 0; i < PVR_NUM_PID_INDEX; i++) {
			for (u4WordIdx = 0; u4WordIdx < PVR_DMEM_ENTRY_LEN; u4WordIdx++)
				PVR_LOG_TRACE(TEXT("HWPidDataStruct[%d][%d]: 0x%x\r\n"),
					i, u4WordIdx, PID_S_W(i, u4WordIdx));
		}
	} else {
		for (u4WordIdx = 0; u4WordIdx < PVR_DMEM_ENTRY_LEN; u4WordIdx++)
			PVR_LOG_TRACE(TEXT("HWPidDataStruct[%u][%d]: 0x%x\r\n"),
				u1PidIdx, u4WordIdx, PID_S_W(u1PidIdx, u4WordIdx));
	}

	PVR_LOG_TRACE(TEXT("============================================================ \r\n"));

	return TRUE;
}

void _PVR_DumpDMem(uintptr_t ptrStartAddr, u32 u4WordCnt)
{
	u32 u4WordIdx = 0;

	PVR_LOG_TRACE(TEXT("============================================================ \r\n"));
	if (ptrStartAddr % 4 != 0)
		ptrStartAddr = ptrStartAddr - (ptrStartAddr % 4);

	for (u4WordIdx = 0; u4WordIdx < u4WordCnt; u4WordIdx++) {
		PVR_LOG_TRACE(TEXT("DMEM[0x%llx]: 0x%08x\r\n"),
			(u64)(ptrStartAddr + u4WordIdx * 4),
			((volatile u32*)(ptrStartAddr))[(u4WordIdx)]);
	}

	PVR_LOG_TRACE(TEXT("============================================================ \r\n"));
}

void _PVR_DumpMemory(uintptr_t ptrBufSa, uintptr_t ptrBufEa, uintptr_t ptrAddr, u32 u4BytesCnt)
{
	u32 u4Idx = 0;
	uintptr_t ptrCurAddr = 0;
	u8  *pu1Buf	 = (u8 *)ptrAddr;

	if ((ptrAddr >= ptrBufEa) || (ptrAddr < ptrBufSa)) {
		PVR_LOG_TRACE(TEXT("%s fail for (ptrAddr(0x%llx) >= ptrBufEa(0x%llx) ) || (ptrAddr(0x%llx)")
			TEXT("	< ptrBufSa(0x%llx) )\r\n"),
			DMX_FUNC_NAME, (u64)ptrAddr, (u64)ptrBufEa, (u64)ptrAddr, (u64)ptrBufSa);
		return;
	}

	PVR_LOG_TRACE(TEXT("==================== ptrAddr(0x%llx), ptrBufSa(0x%llx), ptrBufEa(0x%llx)")
			TEXT(" ======================== \r\n"),
		(u64)ptrAddr, (u64)ptrBufSa, (u64)ptrBufEa);
	for (u4Idx = 0; ((u4Idx < u4BytesCnt) && (NULL != pu1Buf)); u4Idx++, pu1Buf++) {
		ptrCurAddr = (uintptr_t)pu1Buf;
		if (ptrCurAddr >= ptrBufEa)
			pu1Buf = (u8 *)ptrBufSa;

		if (((ptrCurAddr & 0x0000000F) == 0x00000000) ||
			(0 == u4Idx))
			PVR_LOG_TRACE(TEXT("0x%llx ---> 0x%02x"), (u64)ptrCurAddr, *pu1Buf);
		else if ((ptrCurAddr & 0x0000000F) == 0x0000000F)
			PVR_LOG_TRACE(TEXT(" 0x%02x\r\n"), *pu1Buf);
		else
			PVR_LOG_TRACE(TEXT(" 0x%02x"), *pu1Buf);
	}

	PVR_LOG_TRACE(TEXT("============================================================ \r\n"));
}

bool _PVR_DumpCmdQInfo(void)
{
	u32 u4Idx = 0;

	for (u4Idx = 0; u4Idx < PVR_MAX_MM_COM_Q_ITEM_NUM; u4Idx++) {
		PVR_LOG_TRACE(TEXT("[PVR] CmdQueue-- entryid(%ld)'s SkipLen: 0x%x, PayloadLen: 0x%x\r\n"),
			u4Idx,
			PID_S_W(PVR_MM_COM_Q_PID_INDEX, u4Idx * 2),
			PID_S_W(PVR_MM_COM_Q_PID_INDEX, u4Idx * 2 + 1));
	}

	return TRUE;
}

/*-----------------------------------------------------------------------------*/
/** _PVR_DumpStartCodePattern_Ex
 *	Set search start code pattern
 *
 *	@retval bool
 */
/*-----------------------------------------------------------------------------*/
bool _PVR_DumpStartCodePattern_Ex(void)
{
	u32 u4Pattern0, u4Pattern1, u4Mask0, u4Mask1;
	u8 i;

	_PVR_Lock();

	for (i = 0; i < PVR_STARTCODE_NONINTR_NUM; i++) {
		u4Pattern0 = PVR_VCODE_W((2 + (i * 4) * 4));
		u4Pattern1 = PVR_VCODE_W((2 + (i * 4 + 1) * 4));
		u4Mask0 = PVR_VCODE_W((2 + (i * 4 + 2) * 4));
		u4Mask1 = PVR_VCODE_W((2 + (i * 4 + 3) * 4));

		PVR_LOG_TRACE(TEXT("[PVR] %s -- (NonIntr) Idx: %ld, Pattern0: 0x%08x, Pattern1: ")
			TEXT("0x%08x, Mask0: 0x%08x, Mask1: 0x%08x\r\n"),
			DMX_FUNC_NAME, i, u4Pattern0, u4Pattern1, u4Mask0, u4Mask1);
	}

	for (i = 0; i < PVR_STARTCODE_INTR_NUM; i++) {
		u4Pattern0 = PVR_VCODE_W((2 + ((i + PVR_STARTCODE_NONINTR_NUM) * 4) * 4));
		u4Pattern1 = PVR_VCODE_W((2 + ((i + PVR_STARTCODE_NONINTR_NUM) * 4 + 1) * 4));
		u4Mask0 = PVR_VCODE_W((2 + ((i + PVR_STARTCODE_NONINTR_NUM) * 4 + 2) * 4));
		u4Mask1 = PVR_VCODE_W((2 + ((i + PVR_STARTCODE_NONINTR_NUM) * 4 + 3) * 4));
		PVR_LOG_TRACE(TEXT("[PVR] %s -- (Intr) Idx: %ld, Pattern0: 0x%08x, Pattern1: ")
			TEXT("0x%08x, Mask0: 0x%08x, Mask1: 0x%08x\r\n"),
			DMX_FUNC_NAME, i, u4Pattern0, u4Pattern1, u4Mask0, u4Mask1);
	}

	_PVR_Unlock();

	return TRUE;
}

void _PVR_DumpPESHdrInfo(u8 u1PIDIdx)
{
	u32 u4HdrBufSa, u4HdrBufEa, u4HdrBufWP, u4HdrBufRP;
	u32 *pu4HdrBufRp = NULL;
	u32 u4Idx = 0;

	u4HdrBufSa = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_SA);
	u4HdrBufEa	 = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_EA) + 1;
	u4HdrBufWP = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP);
	u4HdrBufRP	 = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP);

	if ((u4HdrBufRP < u4HdrBufSa) || (u4HdrBufRP >= u4HdrBufEa)) {
		PVR_LOG_TRACE(TEXT("[PVR] %s line %d fail for err HdrBufRP, HdrBuf: (RP(0x%x),")
			TEXT(" WP(0x%x), SA(0x%x), EA(0x%x))\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4HdrBufRP, u4HdrBufWP, u4HdrBufSa, u4HdrBufEa);
		return;
	}
	if ((u4HdrBufWP < u4HdrBufSa) || (u4HdrBufWP >= u4HdrBufEa)) {
		PVR_LOG_TRACE(TEXT("[PVR] %s line %d fail for err HdrBufWP, HdrBuf: (RP(0x%x),")
			TEXT(" WP(0x%x), SA(0x%x), EA(0x%x))\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4HdrBufRP, u4HdrBufWP, u4HdrBufSa, u4HdrBufEa);
		return;
	}

	PVR_LOG_TRACE(TEXT("[PVR] %s line %d -- PidIdx(%u)'s HdrBuf: (RP(0x%x), WP(0x%x),")
			TEXT(" SA(0x%x), EA(0x%x))\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, u1PIDIdx, u4HdrBufRP, u4HdrBufWP, u4HdrBufSa, u4HdrBufEa);

	u4Idx = 0;
	while ((u4HdrBufRP != u4HdrBufWP) &&
	  (u4HdrBufRP != 0) &&
	  (u4HdrBufWP != 0) &&
	  (u4HdrBufSa != 0) &&
	  (u4HdrBufEa != 0)) {
		if (u4HdrBufRP >= u4HdrBufEa)
			u4HdrBufRP -= (u4HdrBufEa - u4HdrBufSa);

		if (u4HdrBufRP < u4HdrBufSa) {
			PVR_LOG_TRACE(TEXT("[PVR] %s line %d fail for err HdrBufRP, HdrBuf: (RP(0x%x),")
				TEXT(" WP(0x%x), SA(0x%x), EA(0x%x))\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4HdrBufRP, u4HdrBufWP, u4HdrBufSa, u4HdrBufEa);
			return;
		}

		pu4HdrBufRp = (u32 *)DMX_NONCACHE_REG(u4HdrBufRP);

		PVR_LOG_TRACE(TEXT("(%d) Addr[0x%x)'s Value: 0x%x\r\n"),
			u4Idx, u4HdrBufRP, *pu4HdrBufRp);

		u4Idx++;
		u4HdrBufRP += 4;
	}
}

/*/////////////////////////////////////////////////////////////////////////////////////////////*/
/*
//	TS Packet
//
// | -----4Bytes ----|----------- 184 Bytes ----------|
// | TS Packet Header | Adaption Field(Option) |   Payload	 |
//
// |   TS Packet Header
// |---1B---|---------1bit---------|----------1bit---------|------ 1bit -----| 13bit  |--------- 2bit -----------|
// -------2 bit -------|----- 4 bit ------|
// |SyncByte | transport error indicator |payload unit start indicator | transport priority | PID	| transport
// scrambling control | adaption field control | continuity counter |
//
// |Adaption Field(Option)
// |------- 1B ------ -|-1B-|--------------- Max: 182B ---------------|
// | adaption field length | flag | Information(related to the flag) | adaption data |
//
// |Flag
// |------- 1bit --------|---------- 1bit --------|--------------1bit --------------|-1bit-|----1bit -----|
// ------- 1bit ------------|------ 1bit ----|------------ 1bit ----------|
// | Dicontinuous Indicator | Random access Indicator | Elementary Stream Prorioty Indicator | PCR |
// Connection flag | transport private data flag |Original PCR flag | adaption field extension length |
//
// | adaption field contro
// | 01: has payload, no adaption field;   10: no payload, but has adaption field;
// 11: has payload and adaption field;	  00: no definition
*/
/*/////////////////////////////////////////////////////////////////////////////////////////////*/


/*/////////////////////////////////////////////////////////////////////////////////////////////*/
/*
// PES
//
// | ------ 6 B -------|--------- 3~259 B --------- |----- variable length (Max: 65526B) -- |
// | PES Packet Header	| Elementary Stream Private info |		Payload			|
//
//
// | PES Packet Header
// | ------------3Bytes ----------|---1 B ----|------ 2 B ------|
// | PES Pkg Header Start code prefix |  stream ID	| PES packet length |
//
//
// | Elementary Stream Private info
// | --------2Bytes ------- |----------1 B ---------|--0 ~ 46 B --|-- Max:256B --|
// | PES Pkg Header Indicator | PES packet header length  | optional field | adaption bytes  |
//
// | PES Pkg Header Indicator
// |----------------------------------------- u8 0 -------------------------------------------- |
// -------------------------------------------- u8 1 --------------------------------------|
// |--2bit --|------- 2 Bit --------|---1 bit --- |-------- 1bit -------|---1bit - -|------- 1bit --------- |
// --- 2bit -----|-1bit -|-------1 bit ------- |-1bit -|------ 1 bit ------|-- 1bit -- |------ 1bit ------|
// |-- 10 -- |PES scrambling control | PES priority | Data location indicator | copyright  |
// original version or copy  | PTS/DTS flag | ESCR | Elementary rate flag |  DSM	|
// additional copyinfo | PES CRC | PES extension flag |
//
// ESCR: Elementary Stream Clock Reference.		DSM: Digital Storage Media
//
*/
/*/////////////////////////////////////////////////////////////////////////////////////////////*/

