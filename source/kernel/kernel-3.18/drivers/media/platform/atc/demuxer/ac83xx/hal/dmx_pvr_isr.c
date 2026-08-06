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
 * @file dmx_pvr_isr.c
 *
 * @par Project
 *	  MT3360
 *
 * @par Description
 *	  Demuxer pvr main interrupt interfaces definitions
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
#include <media/atc/dmx_define.h>
#else
#include "dmx_define.h"
#endif

#include "x_bim.h"
#include "drv_config.h"
#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_hal_if.h"
#include "dmx_spt_os.h"
#include "dmx_pvr_ddi.h"
#include "dmx_pvr_if.h"
#include "dmx_psr_util.h"
#include "dmx_pfm.h"

/*-----------------------------------------------------------------------------*/
/* Configurations*/
/*-----------------------------------------------------------------------------*/

#ifndef __linux__
#pragma warning(disable : 4127) /*disable warning C4127: conditional expression is constant*/
#endif


#ifdef SYNC_PES_HEADER
#define PES_HEADER_EXTRA_BYTES			12
#define PAYLOAD_INFO_OFFSET			8
#else
#define PES_HEADER_EXTRA_BYTES			8
#define PAYLOAD_INFO_OFFSET			4
#endif


/* PES header attributes*/


#define PES_HEADER_PACKET_LENGTH_OFFSET		4
#define PES_HEADER_DATA_LENGTH_OFFSET		8

#define PES_HEADER_FLAG_OFFSET			6
#define PES_HEADER_PTS_MASK			(1 << 7)
#define PES_HEADER_DTS_MASK			(1 << 6)
#define PES_HEADER_ORIGINAL_MASK		(1 << 8)
#define PES_HEADER_COPYRIGHT_MASK		(1 << 9)

#define PES_PTS_FIELD_SIZE			5
#define PES_DTS_FIELD_SIZE			5

#define PES_HEADER_FIELD_OFFSET			9
#define PES_HEADER_PTS_OFFSET			(PES_HEADER_FIELD_OFFSET + 0)
#define PES_HEADER_DTS_OFFSET				\
	(PES_HEADER_FIELD_OFFSET + PES_PTS_FIELD_SIZE)

#define PES_HEADER_COPY_SIZE				\
	(PES_HEADER_DTS_OFFSET + PES_DTS_FIELD_SIZE + 1)
#define PES_HEADER_ALIGNMENT				4

#define PES_EXTRA_AUDIO_INFO_LEN			8

#define PID_DATA_STRUCT_CHUNK_LENGTH			1	  /* Chunk Length*/
#define PID_DATA_STRUCT_NUM_START_CODE			3	  /* Number of Start code*/
#define PID_DATA_STRUCT_NUM_START_CODE_MASK		0x0000FF00

#define CHECK_RING_BOUNDARY(CHECKADDR, READPTR, BUFFERSTART, BUFFEREND, READADDR)	\
{\
	if ((CHECKADDR) + ((READPTR) << 2) >= ((BUFFEREND)))	 {	\
		(READPTR)	= (u32)(((CHECKADDR) + ((READPTR) << 2) - (BUFFEREND)) >> 2);   \
		(CHECKADDR) = ((BUFFERSTART)); \
		(READADDR)	= (u32 *)(DMX_NONCACHE(BUFFERSTART));   } \
}
#define CHECK_RING_BOUNDARY2(CHECKADDR, READPTR, BUFFERSTART, BUFFEREND, READADDR, THRESHOLD)	\
{\
		if ((CHECKADDR) + ((READPTR) << 2) >= ((BUFFEREND - THRESHOLD))) {	\
			(READPTR)	= 0;   \
			(CHECKADDR) = ((BUFFERSTART)); \
			(READADDR)	= (u32 *)(DMX_NONCACHE(BUFFERSTART)); } \
}
#define PVR_HDRCHECK_PATTERN	(0x000001FF)

/*-----------------------------------------------------------------------------*/
/* Type definitions*/
/*-----------------------------------------------------------------------------*/

/* PES header structure*/
typedef struct {
	u32 u4Pts;
	u32 u4Dts;
	u32 u4PayloadAddr;
	u32 u4PayloadSize;
	bool fgCopyright;
	bool fgOriginal;

#ifdef CC_PVR_PES_AUDDESC
	bool fgContainAD;
	u8 u1ADFad;
	u8 u1ADPan;
#endif /* CC_PVR_PES_AUDDESC*/
} PVR_PES_HEADER_T;

/*-----------------------------------------------------------------------------*/
/* Macro definitions*/
/*-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------*/
/* Static variables*/
/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
/* Static functions*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Static functions*/
/*-----------------------------------------------------------------------------*/
static MRESULT _PVR_ProcessMM_Video(u8 u1PIDIdx, u32 u4Status, u32 u4Status2);
static MRESULT _PVR_ProcessMM_Audio(u8 u1PIDIdx, u32 u4Status, u32 u4Status2);
static MRESULT _PVR_ProcessMM_SP(u8 u1PIDIdx, u32 u4Status, u32 u4Status2);
static MRESULT _PVR_ProcessMM_Section(u8 u1PIDIdx, u32 u4Status, u32 u4Status2);


/*-----------------------------------------------------------------------------*/
/** _PvrProcessScrambleChange
 *	Process scramble change notification
 *
 *	@param	u1PIDIdx			PID index
 *	@param	fgScrambled		The new state is scrambled or clear
 *
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
static void _DmxProcessScrambleChange(u8 u1PIDIdx, bool fgScrambled)
{
	PVR_PID_STRUCT_T	 *prPidStruct = NULL;
	PVR_SCRAMBLE_STATE_T  eState;

	prPidStruct = _PVR_GetPidStruct(u1PIDIdx);

	if (NULL == prPidStruct) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for invalid Pid Idx: %d\r\n"),
			DMX_FUNC_NAME, u1PIDIdx);
		return;
	}

	if (fgScrambled)
		eState = PVR_SCRAMBLE_STATE_SCRAMBLED;
	else
		eState = PVR_SCRAMBLE_STATE_CLEAR;

	smp_mb();

	PVR_LOG_TRACE(TEXT("[PVR] %s line %d Pidx %u scrambling state: %s\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		u1PIDIdx, fgScrambled ? "SCRAMBLED" : "CLEAR");

	if (NULL != prPidStruct->pfnScramble)
		prPidStruct->pfnScramble(u1PIDIdx,
			PVR_NOTIFY_CODE_SCRAMBLE_STATE, (u32)eState,
			prPidStruct->pvScrambleTag);
}

static void _DmxHWMMIntr(u32 u4Status, u32 u4Status2)
{
	PVR_FTUP_INT_STATUS_INFO_T *prFtupStatus = _PVR_GetFtupIntStatus();
	DMX_HAL_FUNC_INFO_T *prGlobalNfy = _PVR_GetGlobalCbInfo();
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prFtupStatus) || (NULL == prGlobalNfy)) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for Ftup Status mem or Global Callback hasn't been allocated\r\n"),
			DMX_FUNC_NAME);
		return;
	}

	PVR_LOG_DBG(TEXT("[PVR] %s -- u4Status: 0x%x, u4Status2: 0x%x!\r\n"),
		DMX_FUNC_NAME, u4Status, u4Status2);
	smp_mb();

	_PVR_Lock();
	prFtupStatus->u4Status	= u4Status;
	prFtupStatus->u4Status2 = u4Status2;
	prFtupStatus->u1PIDIdx	= PVR_GET_BYTE(u4Status, 2);		 /* PID index*/
	_PVR_Unlock();

	smp_mb();
	_PVR_Lock();
	#if DMX_PFM_TEST
	if (PVR_PID_IDX_VIDEO == prFtupStatus->u1PIDIdx) {
		#if DMX_PFM_TEST
		DmxPfmStmHwDmaEnd(SPT_DATA_V);
		#endif /* DMX_PFM_TEST*/
	} else if (PVR_PID_IDX_AUDIO == prFtupStatus->u1PIDIdx) {
		#if DMX_PFM_TEST
		DmxPfmStmHwDmaEnd(SPT_DATA_A);
		#endif /* DMX_PFM_TEST*/
	} else if (PVR_PID_IDX_SP == prFtupStatus->u1PIDIdx) {
		#if DMX_PFM_TEST
		DmxPfmStmHwDmaEnd(SPT_DATA_SP);
		#endif /* DMX_PFM_TEST*/
	}
	#endif /* DMX_PFM_TEST*/
	_PVR_Unlock();

	smp_mb();
	if (NULL != prGlobalNfy->pfnCB) {
		mrRet = prGlobalNfy->pfnCB(prFtupStatus, prGlobalNfy->pvPrivData);
		if (DMX_FAILED(mrRet))
			PVR_LOG_ERR(TEXT("[PVR] %s fail in prGlobalNfy->pfnCB\r\n"),
				DMX_FUNC_NAME);
		else
			PVR_LOG_DBG(TEXT("[PVR] %s success in prGlobalNfy->pfnCB\r\n"),
				DMX_FUNC_NAME);
	}
}

MRESULT _PVR_ProcessMM_Video(u8 u1PIDIdx, u32 u4Status, u32 u4Status2)
{
	PVR_FILTER_INTSTATUS_T rFILTERIntStatus = {0};
	u32 *pu4PESData	  = NULL;
	u32 u4EndAddr	  = 0;
	u32 u4ReadPtr	  = 0;
	u32 u4HdrBufStart  = 0;
	u32 u4HdrBufEnd	  = 0;
	u32 u4FirstAddr	  = 0;
	u32 u4FifoSA_Phy   = 0;
	u32 u4FifoEA_Phy   = 0;
	u8  u1DevID        = (0 == u1PIDIdx) ? (DDI_PVR_DMA_PATH_ID) : (MINI_PVR_DMA_PATH_ID);
	DMX_PIC_INFO_T *prPicsInfo	  = _PVR_GetPicturesInfo();
	PVR_PID_STRUCT_T *prPidStruct = NULL;
	PVR_VIDEO_TYPE_T eVideoType = _PVR_GetVideoType(u1DevID);
	u32 u4RetryCnt = 0;
	u32 i = 0;

	_PVR_Lock();

RETRYVIDEO:

	u4RetryCnt++;

	/* For PES, Status 2 Reg's value is the starting address of PES header*/
	u4EndAddr	= u4Status2;	/*DMXCMD_READ32(PVR_REG_FTuP_NONERR_STATUS_REG2);		114*/
	smp_mb();
	prPidStruct = _PVR_GetPidStruct(u1PIDIdx);

	smp_mb();
	if (NULL == prPidStruct) {
		_PVR_Unlock();
		PVR_LOG_ERR(TEXT("[PVR] %s failed for invalid Pid Idx: %u\r\n"),
			DMX_FUNC_NAME, u1PIDIdx);
		MM_RETURN(RET_DMX_HW_ERROR);
	}

	smp_mb();
	if (NULL == prPicsInfo) {
		_PVR_Unlock();
		PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for Picture Info array hasn't been allocated\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_HW_ERROR);
	}

#if 0
	PVR_LOG_ERR(TEXT("[PVR] %s -- PVR_HAL_DumpPidStruct(Pidx: 0x%02x) begin\r\n"),
		DMX_FUNC_NAME, u1PIDIdx);
	PVR_HAL_DumpPidStruct(u1PIDIdx);
	PVR_LOG_ERR(TEXT("[PVR] %s -- PVR_HAL_DumpPidStruct(Pidx: 0x%02x) end\r\n"),
		DMX_FUNC_NAME, u1PIDIdx);
	_PVR_DumpKeySRAM(FALSE);
#endif

	smp_mb();
	u4HdrBufStart = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_SA);
	u4HdrBufEnd   = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_EA) + 1;
	u4FifoSA_Phy  = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_SA);
	u4FifoEA_Phy  = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_EA) + 1;

	smp_mb();
	if (((PID_S_W(u1PIDIdx, 0))&(1 << 6)) == 0) {
		_PVR_Unlock();
		PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for video PID data structure's section filter")
			TEXT(" flag's bit6 and bit7 should be 1\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_HW_ERROR);
	}

	smp_mb();
	if ((u4EndAddr < u4HdrBufStart) || (u4EndAddr >= u4HdrBufEnd)) {
		_PVR_Unlock();
		PVR_LOG_ERR(TEXT("[PVR] %s line %d fail for PesStartAddr(0x%08x) isn't in Pes Buffer")
			TEXT(" [0x%08x, 0x%08x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4EndAddr, u4HdrBufStart, u4HdrBufEnd);
		PVR_LOG_ERR(TEXT("[PVR] Circular header buffer: CurPesPA(0x%08x), PhyRP(0x%08x), PhyWP(0x%08x),")
			TEXT(" PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
			u4FirstAddr, PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP),
			u4HdrBufStart, u4HdrBufEnd);
		PVR_LOG_ERR(TEXT("[PVR] Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP),
			u4FifoSA_Phy, u4FifoEA_Phy);
		MM_RETURN(RET_DMX_HW_ERROR);
	}

	smp_mb();
	mm_memset(&rFILTERIntStatus, 0, sizeof(rFILTERIntStatus));

	smp_mb();
	u4ReadPtr = 0;

	smp_mb();
	CHECK_RING_BOUNDARY2(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd,
		pu4PESData, PVR_FTUP_VID_DMA_THRESHOLD);

	smp_mb();
	u4FirstAddr = u4EndAddr;

	pu4PESData = (u32 *)DMX_NONCACHE(u4EndAddr);
	smp_mb();

	if (NULL == pu4PESData) {
		_PVR_Unlock();
		PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for invalid PES start Address: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4EndAddr);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_HW_ERROR);
	}
	smp_mb();

	/* PES Header Byte Count(2B) value in PES header circular buffer*/
	/*PES Information*/
	rFILTERIntStatus.rHdrDectResult.u4PESHdrLen = (pu4PESData[u4ReadPtr] & 0xFFFF0000) >> 16;
	if (0 != rFILTERIntStatus.rHdrDectResult.u4PESHdrLen) {
		PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for PES Header Len(%d) > 0, pu4PESData[0x%lx],")
				TEXT(" u4ReadPtr: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, rFILTERIntStatus.rHdrDectResult.u4PESHdrLen,
			pu4PESData, u4ReadPtr);
		if (u4RetryCnt > DMX_HW_RETRY_CNT) {
			_PVR_Unlock();
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		} else {
			goto RETRYVIDEO;
		}
	}
	smp_mb();

	rFILTERIntStatus.rHdrDectResult.u4PicInfoCount =
		(PID_S_W(u1PIDIdx, PID_DATA_STRUCT_NUM_START_CODE) & PID_DATA_STRUCT_NUM_START_CODE_MASK) >> 8;

	smp_mb();
	/*/> Now ReadPtr is the index of first start code entry*/
	if (rFILTERIntStatus.rHdrDectResult.u4PicInfoCount > DMX_MAX_VID_STARTCODE_CNT) {
		PVR_LOG_ERR(TEXT("[PVR] %s Line %d failed for Pic Count(%d) > DMX_MAX_VID_STARTCODE_CNT(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			rFILTERIntStatus.rHdrDectResult.u4PicInfoCount,
			DMX_MAX_VID_STARTCODE_CNT);
		if (u4RetryCnt > DMX_HW_RETRY_CNT) {
			_PVR_Unlock();
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		} else {
			goto RETRYVIDEO;
		}
	}

	smp_mb();
	PVR_LOG_DBG(TEXT("[PVR] %s -- StartCodeCnt: %d\r\n"),
		DMX_FUNC_NAME, rFILTERIntStatus.rHdrDectResult.u4PicInfoCount);

	smp_mb();
	/* u32 1 (End Address + 1 of the corresponding PES payload buffer)*/
	u4ReadPtr++;

	smp_mb();
	CHECK_RING_BOUNDARY(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd, pu4PESData);
	if (NULL == pu4PESData) {
		_PVR_Unlock();
		PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for unexpect error\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_HW_ERROR);
	}

	smp_mb();
	rFILTERIntStatus.u4DataEndAddr = pu4PESData[u4ReadPtr];
	/*pu4PESData[u4ReadPtr] = (pu4PESData[u4ReadPtr] | DMX_HDRCHECK_PATTERN);*/

	smp_mb();
	if ((rFILTERIntStatus.u4DataEndAddr < u4FifoSA_Phy) ||
		(rFILTERIntStatus.u4DataEndAddr > u4FifoEA_Phy)) {
		PVR_LOG_ERR(TEXT("[PVR] %s line %d fail for PesEndAddr(0x%08x) isn't in DstFifo [0x%08x, 0x%08x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, rFILTERIntStatus.u4DataEndAddr, u4FifoSA_Phy, u4FifoEA_Phy);
		PVR_LOG_ERR(TEXT("[PVR] Circular header buffer: CurPesPA(0x%08x), PhyRP(0x%08x),")
			TEXT(" PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
			u4FirstAddr, PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP),
			u4HdrBufStart, u4HdrBufEnd);
		PVR_LOG_ERR(TEXT("[PVR] Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP),
			u4FifoSA_Phy, u4FifoEA_Phy);
		if (u4RetryCnt > DMX_HW_RETRY_CNT) {
			_PVR_Unlock();
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		} else {
			goto RETRYVIDEO;
		}
	}

	smp_mb();
	/*DWORD2 (Starting Address of the corresponding PES payload buffer)*/
	u4ReadPtr++;

	smp_mb();
	CHECK_RING_BOUNDARY(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd, pu4PESData);
	if (NULL == pu4PESData) {
		_PVR_Unlock();
		PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for unexpect error\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_HW_ERROR);
	}

	smp_mb();
	rFILTERIntStatus.u4DataStartAddr = pu4PESData[u4ReadPtr];
	/*pu4PESData[u4ReadPtr] = (pu4PESData[u4ReadPtr] | DMX_HDRCHECK_PATTERN);*/

	smp_mb();
	if ((rFILTERIntStatus.u4DataStartAddr < u4FifoSA_Phy) ||
		(rFILTERIntStatus.u4DataStartAddr > u4FifoEA_Phy)) {
		PVR_LOG_ERR(TEXT("[PVR] %s line %d fail for PesEndAddr(0x%08x) isn't in DstFifo [0x%08x, 0x%08x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, rFILTERIntStatus.u4DataStartAddr, u4FifoSA_Phy, u4FifoEA_Phy);
		PVR_LOG_ERR(TEXT("[PVR] Circular header buffer: CurPesPA(0x%08x), PhyRP(0x%08x),")
				TEXT(" PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
			u4FirstAddr, PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP),
			u4HdrBufStart, u4HdrBufEnd);
		PVR_LOG_ERR(TEXT("[PVR] Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP),
			u4FifoSA_Phy, u4FifoEA_Phy);
		if (u4RetryCnt > DMX_HW_RETRY_CNT) {
			_PVR_Unlock();
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		} else {
			goto RETRYVIDEO;
		}
	}

	smp_mb();
	/* PES Header Area len should be 0*/
	u4ReadPtr++;
	CHECK_RING_BOUNDARY(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd, pu4PESData);
	if (NULL == pu4PESData) {
		_PVR_Unlock();
		PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for unexpect error\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_HW_ERROR);
	}

	smp_mb();
	/* Video Start Code Array*/
#ifndef __linux__
#if !DMX_HWMEM_ALL_USE_RSVMEM
	if (rFILTERIntStatus.u4DataEndAddr < rFILTERIntStatus.u4DataStartAddr) {
		void *pvFlushSa = (void *)DMX_NONCACHE(rFILTERIntStatus.u4DataStartAddr);

		if (NULL != pvFlushSa) {
			CacheRangeFlush(pvFlushSa,
				(u4FifoEA_Phy - rFILTERIntStatus.u4DataStartAddr),
				CACHE_SYNC_DISCARD);
		}
		pvFlushSa = (void *)DMX_NONCACHE(u4FifoSA_Phy);
		if (NULL != pvFlushSa) {
			CacheRangeFlush(pvFlushSa,
				(rFILTERIntStatus.u4DataEndAddr - u4FifoSA_Phy),
				CACHE_SYNC_DISCARD);
		}
	} else if (rFILTERIntStatus.u4DataEndAddr > rFILTERIntStatus.u4DataStartAddr) {
		void *pvFlushSa = (void *)DMX_NONCACHE(rFILTERIntStatus.u4DataStartAddr);

		if (NULL != pvFlushSa) {
			CacheRangeFlush(pvFlushSa,
				(rFILTERIntStatus.u4DataEndAddr - rFILTERIntStatus.u4DataStartAddr),
				CACHE_SYNC_DISCARD);
		}
	}
#endif
#endif /* end of #ifndef __linux__*/

	smp_mb();
	rFILTERIntStatus.u4HWPIDIndex = u1PIDIdx;

	rFILTERIntStatus.rHdrDectResult.prPicsInfo = prPicsInfo;

	smp_mb();
	if (rFILTERIntStatus.rHdrDectResult.u4PicInfoCount > 0) {
		u32 u4PicInfo = 0;
		s8   i1VCodeOffset = 0;
		u8  u1VCIdx = 0;

		smp_mb();
		for (i = 0; i < rFILTERIntStatus.rHdrDectResult.u4PicInfoCount; i++, prPicsInfo++) {
			CHECK_RING_BOUNDARY(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd, pu4PESData);
			smp_mb();
			if (NULL == pu4PESData) {
				_PVR_Unlock();
				PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for unexpect error\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_HW_ERROR);
			}

			smp_mb();
			prPicsInfo->ptrPicStartAddr = pu4PESData[u4ReadPtr];
			smp_mb();

			if ((prPicsInfo->ptrPicStartAddr < u4FifoSA_Phy) ||
				(prPicsInfo->ptrPicStartAddr >= u4FifoEA_Phy)) {
				u32 u4Ctrl = DMXCMD_READ32(PVR_REG_FTUP_FULL_STATUS) & 0x00000007;
				u32 u4RP   = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP);
				u32 u4WP   = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP);
				u32 u4FifoRP = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP);
				u32 u4FifoWP = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP);

				_PVR_DumpPIDDataStruct(u1PIDIdx);

				mb();

				PVR_LOG_ERR(TEXT("[PVR] Assert PicInfo -- PVR_VCODE_W(0): 0x%x\r\n"),
					PVR_VCODE_W(0));

				smp_mb();
				PVR_LOG_ERR(TEXT("[PVR] Assert PicInfo -- PicIdx(%d), PicPhyStartAddr(0x%08x)\r\n"),
					i, (prPicsInfo->ptrPicStartAddr));

				PVR_LOG_ERR(TEXT("[PVR] Circular header buffer: CurPesPA(0x%08x), PhyRP(0x%08x),")
					TEXT(" PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
					u4FirstAddr, u4RP, u4WP, u4HdrBufStart, u4HdrBufEnd);

				PVR_LOG_ERR(TEXT("[PVR] Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x),")
					TEXT(" PhyEA(0x%08x)\r\n"),
					u4FifoRP, u4FifoWP, u4FifoSA_Phy, u4FifoEA_Phy);

				PVR_LOG_ERR(TEXT("[PVR] PicInfoCount: 0x%x\r\n"),
					rFILTERIntStatus.rHdrDectResult.u4PicInfoCount);

				PVR_LOG_ERR(TEXT("[PVR] Circular Buffer FullStatus %x\r\n"), u4Ctrl);

				smp_mb();
				if (u4RetryCnt > DMX_HW_RETRY_CNT) {
					_PVR_Unlock();
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_HW_ERROR);
				} else {
					goto RETRYVIDEO;
				}
			}

			smp_mb();
			u4ReadPtr++;

			smp_mb();
			CHECK_RING_BOUNDARY(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd, pu4PESData);
			if (NULL == pu4PESData) {
				_PVR_Unlock();
				PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for unexpect error\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_HW_ERROR);
			}
			smp_mb();

			u4PicInfo = (pu4PESData[u4ReadPtr]) & 0x000003FF;
			u1VCIdx = 0;
			smp_mb();

			PVR_LOG_DBG(TEXT("[PVR] %s line %d -- PicIdx(%d) u4PicInfo: %d, PicStartAddr: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, i, u4PicInfo, prPicsInfo->ptrPicStartAddr);

			for (u1VCIdx = 0; u1VCIdx < 10; u1VCIdx++) {
				if (u4PicInfo & (1 << u1VCIdx))
					break;
			}
			smp_mb();
			prPicsInfo->u1PicType = u1VCIdx;

			smp_mb();
			i1VCodeOffset = _PVR_GetVCode_Offset(eVideoType, u1VCIdx);

			smp_mb();
			prPicsInfo->ptrPicStartAddr += i1VCodeOffset;
			smp_mb();

			if (prPicsInfo->ptrPicStartAddr < u4FifoSA_Phy)
				prPicsInfo->ptrPicStartAddr = u4FifoEA_Phy - (u4FifoSA_Phy - prPicsInfo->ptrPicStartAddr);

			smp_mb();
			if ((prPicsInfo->ptrPicStartAddr < u4FifoSA_Phy) ||
				(prPicsInfo->ptrPicStartAddr >= u4FifoEA_Phy)) {
				u32 u4Ctrl = DMXCMD_READ32(PVR_REG_FTUP_FULL_STATUS) & 0x00000007;
				u32 u4RP   = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP);
				u32 u4WP   = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP);
				u32 u4FifoRP = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP);
				u32 u4FifoWP = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP);

				_PVR_DumpPIDDataStruct(u1PIDIdx);

				mb();

				PVR_LOG_ERR(TEXT("[PVR] Assert PicInfo -- PVR_VCODE_W(0): 0x%x\r\n"),
					PVR_VCODE_W(0));

				PVR_LOG_ERR(TEXT("[PVR] Assert PicInfo -- PicIdx(%d), PicPhyStartAddr(0x%08x)\r\n"),
					i, (prPicsInfo->ptrPicStartAddr));

				PVR_LOG_ERR(TEXT("[PVR] Circular header buffer: CurPesPA(0x%08x), PhyRP(0x%08x),")
					TEXT(" PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
					u4FirstAddr, u4RP, u4WP, u4HdrBufStart, u4HdrBufEnd);

				PVR_LOG_ERR(TEXT("[PVR] Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x),")
					TEXT(" PhyEA(0x%08x)\r\n"),
					u4FifoRP, u4FifoWP, u4FifoSA_Phy, u4FifoEA_Phy);

				PVR_LOG_ERR(TEXT("[PVR] PicInfoCount: 0x%x\r\n"),
					rFILTERIntStatus.rHdrDectResult.u4PicInfoCount);

				PVR_LOG_ERR(TEXT("[PVR] Circular Buffer FullStatus %x\r\n"), u4Ctrl);

				smp_mb();
				if (u4RetryCnt > DMX_HW_RETRY_CNT) {
					_PVR_Unlock();
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_HW_ERROR);
				} else {
					goto RETRYVIDEO;
				}
			}

			smp_mb();
			prPicsInfo->ptrPicStartAddr = DMX_NONCACHE(prPicsInfo->ptrPicStartAddr);
			smp_mb();
			if (0 == prPicsInfo->ptrPicStartAddr) {
				PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for Error Picture Start Address\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				if (u4RetryCnt > DMX_HW_RETRY_CNT) {
					_PVR_Unlock();
					DMX_ASSERT(FALSE);
					MM_RETURN(RET_DMX_HW_ERROR);
				} else {
					goto RETRYVIDEO;
				}
			}

			/*pu4PESData[u4ReadPtr] = pu4PESData[u4ReadPtr] | DMX_HDRCHECK_PATTERN;*/

			smp_mb();
			u4ReadPtr++;
		}
	}

	smp_mb();
	/* Parse Header Information*/
	rFILTERIntStatus.u4HWPIDIndex	 = u1PIDIdx;
	CHECK_RING_BOUNDARY(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd, pu4PESData);
	if (NULL == pu4PESData) {
		_PVR_Unlock();
		PVR_LOG_ERR(TEXT("[PVR] %s Line: %d failed for unexpect error\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_HW_ERROR);
	}

	smp_mb();
	PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP) = u4EndAddr + u4ReadPtr * 4;
	mb();

	PVR_LOG_DBG(TEXT("------ [PVR] %s Exit, PicCnt(%d) DataSa(0x%08X) DataEa(0x%08X) ------\r\n"),
		DMX_FUNC_NAME,
		rFILTERIntStatus.rHdrDectResult.u4PicInfoCount,
		rFILTERIntStatus.u4DataStartAddr,
		rFILTERIntStatus.u4DataEndAddr);
	smp_mb();

	_PVR_Unlock();

	/* Invoke Callback to notify Splitter*/
	if (NULL != prPidStruct->pfnNotify) {
		MRESULT mrRet = RET_DMX_OK;

		mrRet = prPidStruct->pfnNotify(
			u1PIDIdx, PVR_NOTIFY_CODE_ES,
			(u32)(&rFILTERIntStatus),
			prPidStruct->pvNotifyTag);
		if (DMX_FAILED(mrRet)) {
			PVR_LOG_ERR(TEXT("[PVR] %s failed in pfnNotify, mrRet: 0x%x, Pid Idx: %u\r\n"),
				DMX_FUNC_NAME, mrRet, u1PIDIdx);
			MM_RETURN(mrRet);
		}
	}

	smp_mb();
	PVR_LOG_DBG(TEXT("[PVR] %s success, Pid Idx: %u\r\n"),
		DMX_FUNC_NAME, u1PIDIdx);

	MM_RETURN(RET_DMX_OK);
}

MRESULT _PVR_ProcessMM_Audio(u8 u1PIDIdx, u32 u4Status, u32 u4Status2)
{
	PVR_PID_STRUCT_T *prPidStruct = NULL;
	u32 *pu4PESData	   = NULL;
	u32 u4PESHdrLen	   = 0;
	u32 u4DataEndAddr   = 0;
	u32 u4DataStartAddr = 0;
	u32 u4EndAddr	  = 0;
	u32 u4ReadPtr	  = 0;
	u32 u4HdrBufStart  = 0;
	u32 u4HdrBufEnd	  = 0;
	u32 u4HdrBufWp	  = 0;
	u32 u4HdrBufRp	  = 0;
	u32 u4FirstAddr	  = 0;
	u32 u4FifoSA_Phy   = 0;
	u32 u4FifoEA_Phy   = 0;
	u32 u4RetryCnt	  = 0;

	_PVR_Lock();

RETRYAUDIO:

	UNUSE_PARAMETER(u4Status);

	smp_mb();
	u4RetryCnt++;

	/* For PES, Status 2 Reg's value is the starting address of PES header*/
	u4EndAddr	= u4Status2;	/*DMXCMD_READ32(DMX_REG_FTuP_NONERR_STATUS_REG2);		114*/

	smp_mb();
	prPidStruct = _PVR_GetPidStruct(u1PIDIdx);

	smp_mb();
	if (NULL == prPidStruct) {
		_PVR_Unlock();
		PVR_LOG_ERR(TEXT("[PVR] %s failed for invalid Pid Idx: %u\r\n"),
			DMX_FUNC_NAME, u1PIDIdx);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_HW_ERROR);
	}
	smp_mb();

	u4HdrBufStart = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_SA);
	u4HdrBufEnd   = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_EA) + 1;
	u4HdrBufWp	  = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP);
	u4HdrBufRp	  = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP);
	u4FifoSA_Phy  = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_SA);
	u4FifoEA_Phy  = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_EA) + 1;

	smp_mb();
	PVR_LOG_DBG(TEXT("[PVR] %s line %d -- PID_S_W(u1PIDIdx, 0)=0x%08x,	(&(1<<6))=0x%08x\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, PID_S_W(u1PIDIdx, 0),
		((PID_S_W(u1PIDIdx, 0))&(1 << 6)));

	if (((PID_S_W(u1PIDIdx, 0))&(1 << 6)) == 0) {
		if ((u4HdrBufStart != 0) && (u4HdrBufEnd != 0)) {
			PVR_LOG_DBG(TEXT("[PVR] Audio -- Circular header buffer: CurPesPA(0x%08x),")
				TEXT(" PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				u4FirstAddr, PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP),
				u4HdrBufStart, u4HdrBufEnd);
			PVR_LOG_DBG(TEXT("[PVR] Audio -- Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x),")
				TEXT(" PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP),
				u4FifoSA_Phy, u4FifoEA_Phy);

			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP) = u4HdrBufWp;
			mb();
		}

		if ((0x04 == u4HdrBufStart) ||
		  (0x04 == u4HdrBufEnd) ||
		  (0x04 == u4HdrBufWp) ||
		  (0x04 == u4HdrBufRp) ||
		  (0x04 == u4FifoSA_Phy) ||
		  (0x04 == u4FifoEA_Phy) ||
		  (0x04 == PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP)) ||
		  (0x04 == PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP))) {
			PVR_LOG_ERR(TEXT("[PVR] Audio -- Circular header buffer: CurPesPA(0x%08x),")
				TEXT(" PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				u4FirstAddr, PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP),
				u4HdrBufStart, u4HdrBufEnd);
			PVR_LOG_ERR(TEXT("[PVR] Audio -- Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x),")
				TEXT(" PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP),
				u4FifoSA_Phy, u4FifoEA_Phy);
			_PVR_DumpPESHdrInfo(u1PIDIdx);
			DMX_ASSERT(FALSE);
		}
		smp_mb();

		if (NULL != prPidStruct->pfnNotify) {
			MRESULT mrRet = RET_DMX_OK;

			mrRet = prPidStruct->pfnNotify(u1PIDIdx, PVR_NOTIFY_CODE_ES,
				 0, prPidStruct->pvNotifyTag);
			if (DMX_FAILED(mrRet)) {
				_PVR_Unlock();
				PVR_LOG_ERR(TEXT("[PVR] %s failed in pfnNotify, mrRet: 0x%x, Pid Idx: %u\r\n"),
					DMX_FUNC_NAME, mrRet, u1PIDIdx);
				MM_RETURN(mrRet);
			}
		}
		_PVR_Unlock();
		smp_mb();

		MM_RETURN(RET_DMX_OK);
	}

	smp_mb();

	if ((0x04 == u4HdrBufStart) || (0x04 == u4HdrBufEnd) || (0x04 == u4HdrBufWp) ||
	  (0x04 == u4HdrBufRp) || (0x04 == u4FifoSA_Phy) || (0x04 == u4FifoEA_Phy) ||
	  (0x04 == PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP)) ||
	  (0x04 == PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP))) {
		PVR_LOG_ERR(TEXT("[PVR] Audio -- Circular header buffer: CurPesPA(0x%08x),")
			TEXT(" PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
			u4FirstAddr, PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP),
			u4HdrBufStart, u4HdrBufEnd);
		PVR_LOG_ERR(TEXT("[PVR] Audio -- Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x),")
				TEXT(" PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP),
			u4FifoSA_Phy, u4FifoEA_Phy);
		_PVR_DumpPESHdrInfo(u1PIDIdx);
		DMX_ASSERT(FALSE);
	}

	if ((u4HdrBufStart != 0) &&
		(u4HdrBufEnd != 0) &&
		(u4EndAddr != 0) &&
		(u4HdrBufWp != u4HdrBufRp)) {
		PVR_LOG_DBG(TEXT("[PVR] Audio -- Circular header buffer: CurPesPA(0x%08x),")
			TEXT(" PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
			u4FirstAddr, PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP),
			u4HdrBufStart, u4HdrBufEnd);
		PVR_LOG_DBG(TEXT("[PVR] Audio -- Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x),")
				TEXT(" PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP),
			u4FifoSA_Phy, u4FifoEA_Phy);
		smp_mb();

		if ((u4EndAddr < u4HdrBufStart) || (u4EndAddr >= u4HdrBufEnd)) {
			_PVR_Unlock();
			PVR_LOG_ERR(TEXT("[PVR] %s line %d fail for PesStartAddr(0x%08x) isn't")
				TEXT(" in Pes Buffer [0x%08x, 0x%08x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4EndAddr, u4HdrBufStart, u4HdrBufEnd);
			PVR_LOG_ERR(TEXT("[PVR] Circular header buffer: CurPesPA(0x%08x),")
				TEXT(" PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				u4FirstAddr, PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP),
				u4HdrBufStart, u4HdrBufEnd);
			PVR_LOG_ERR(TEXT("[PVR] Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x),")
				TEXT(" PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP),
				u4FifoSA_Phy, u4FifoEA_Phy);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		smp_mb();
		u4ReadPtr = 0;

		smp_mb();
		CHECK_RING_BOUNDARY2(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd,
			pu4PESData, PVR_FTUP_OTH_DMA_THRESHOLD);

		smp_mb();
		u4FirstAddr = u4EndAddr;
		smp_mb();

		pu4PESData = (u32 *)DMX_NONCACHE(u4EndAddr);

		smp_mb();
		if (NULL == pu4PESData) {
			_PVR_Unlock();
			PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for invalid PES start Address: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4EndAddr);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		smp_mb();
		/* PES Header Byte Count(2B) value in PES header circular buffer*/
		u4PESHdrLen = (pu4PESData[u4ReadPtr] & 0xFFFF0000) >> 16;	 /*PES Information*/
		if (0 != u4PESHdrLen) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for PES Header Len(%d) > 0\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4PESHdrLen);
			if (u4RetryCnt > DMX_HW_RETRY_CNT) {
				_PVR_Unlock();
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_HW_ERROR);
			} else {
				goto RETRYAUDIO;
			}
		}
		smp_mb();
		/* u32 1 (End Address + 1 of the corresponding PES payload buffer)*/
		u4ReadPtr++;
		smp_mb();
		CHECK_RING_BOUNDARY(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd, pu4PESData);
		if (NULL == pu4PESData) {
			_PVR_Unlock();
			PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for unexpect error\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		smp_mb();
		u4DataEndAddr = pu4PESData[u4ReadPtr];
		/*pu4PESData[u4ReadPtr] = (pu4PESData[u4ReadPtr] | DMX_HDRCHECK_PATTERN);*/
		smp_mb();

		if ((u4DataEndAddr < u4FifoSA_Phy) ||
			(u4DataEndAddr > u4FifoEA_Phy)) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d fail for PesEndAddr(0x%08x) isn't")
				TEXT(" in DstFifo [0x%08x, 0x%08x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4DataEndAddr, u4FifoSA_Phy, u4FifoEA_Phy);
			PVR_LOG_ERR(TEXT("[PVR] Circular header buffer: CurPesPA(0x%08x),")
				TEXT(" PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				u4FirstAddr, PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP),
				u4HdrBufStart, u4HdrBufEnd);
			PVR_LOG_ERR(TEXT("[PVR] Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x),")
				TEXT(" PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP),
				u4FifoSA_Phy, u4FifoEA_Phy);
			if (u4RetryCnt > DMX_HW_RETRY_CNT) {
				_PVR_Unlock();
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_HW_ERROR);
			} else {
				goto RETRYAUDIO;
			}
		}

		smp_mb();
		/*DWORD2 (Starting Address of the corresponding PES payload buffer)*/
		u4ReadPtr++;
		smp_mb();
		CHECK_RING_BOUNDARY(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd, pu4PESData);
		if (NULL == pu4PESData) {
			_PVR_Unlock();
			PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for unexpect error\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		smp_mb();
		u4DataStartAddr = pu4PESData[u4ReadPtr];
		/*pu4PESData[u4ReadPtr] = (pu4PESData[u4ReadPtr] | DMX_HDRCHECK_PATTERN);*/
		smp_mb();

		if ((u4DataStartAddr < u4FifoSA_Phy) ||
			(u4DataStartAddr > u4FifoEA_Phy)) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d fail for PesEndAddr(0x%08x) isn't")
				TEXT(" in DstFifo [0x%08x, 0x%08x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4DataStartAddr, u4FifoSA_Phy, u4FifoEA_Phy);
			PVR_LOG_ERR(TEXT("[PVR] Circular header buffer: CurPesPA(0x%08x),")
				TEXT(" PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				u4FirstAddr, PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP),
				u4HdrBufStart, u4HdrBufEnd);
			PVR_LOG_ERR(TEXT("[PVR] Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x),")
				TEXT(" PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP),
				u4FifoSA_Phy, u4FifoEA_Phy);
			if (u4RetryCnt > DMX_HW_RETRY_CNT) {
				_PVR_Unlock();
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_HW_ERROR);
			} else {
				goto RETRYAUDIO;
			}
		}

		smp_mb();
		/* PES Header Area len should be 0*/
		u4ReadPtr++;
		smp_mb();
		CHECK_RING_BOUNDARY(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd, pu4PESData);
		smp_mb();
		if (NULL == pu4PESData) {
			_PVR_Unlock();
			PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for unexpect error\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		smp_mb();
		#if !DMX_HWMEM_ALL_USE_RSVMEM
		if (u4DataEndAddr < u4DataStartAddr) {
			void *pvFlushSa = (void *)DMX_NONCACHE(u4DataStartAddr);

			if (NULL != pvFlushSa)
				CacheRangeFlush(pvFlushSa, (u4FifoEA_Phy - u4DataStartAddr),
					CACHE_SYNC_DISCARD);

			pvFlushSa = (void *)DMX_NONCACHE(u4FifoSA_Phy);
			if (NULL != pvFlushSa)
				CacheRangeFlush(pvFlushSa, (u4DataEndAddr - u4FifoSA_Phy),
					CACHE_SYNC_DISCARD);

		} else if (u4DataEndAddr > u4DataStartAddr) {
			void *pvFlushSa = (void *)DMX_NONCACHE(u4DataStartAddr);

			if (NULL != pvFlushSa)
				CacheRangeFlush(pvFlushSa, (u4DataEndAddr - u4DataStartAddr),
					CACHE_SYNC_DISCARD);

		}
		#endif /* !DMX_HWMEM_ALL_USE_RSVMEM*/

		smp_mb();
		CHECK_RING_BOUNDARY(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd, pu4PESData);
		smp_mb();
		if (NULL == pu4PESData) {
			_PVR_Unlock();
			PVR_LOG_ERR(TEXT("[PVR] %s Line: %d failed for unexpect error\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP) = u4EndAddr + u4ReadPtr * 4;
		mb();
	}

	_PVR_Unlock();

	smp_mb();
	if (NULL != prPidStruct->pfnNotify) {
		MRESULT mrRet = RET_DMX_OK;

		mrRet = prPidStruct->pfnNotify(u1PIDIdx, PVR_NOTIFY_CODE_ES,
			 0, prPidStruct->pvNotifyTag);
		if (DMX_FAILED(mrRet)) {
			PVR_LOG_ERR(TEXT("[PVR] %s failed in pfnNotify, mrRet: 0x%x, Pid Idx: %u\r\n"),
				DMX_FUNC_NAME, mrRet, u1PIDIdx);
			MM_RETURN(mrRet);
		}
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT _PVR_ProcessMM_SP(u8 u1PIDIdx, u32 u4Status, u32 u4Status2)
{
	PVR_PID_STRUCT_T *prPidStruct = NULL;
	u32 *pu4PESData	   = NULL;
	u32 u4PESHdrLen	   = 0;
	u32 u4DataEndAddr   = 0;
	u32 u4DataStartAddr = 0;
	u32 u4EndAddr	  = 0;
	u32 u4ReadPtr	  = 0;
	u32 u4HdrBufStart  = 0;
	u32 u4HdrBufEnd	  = 0;
	u32 u4HdrBufWp	  = 0;
	u32 u4HdrBufRp	  = 0;
	u32 u4FirstAddr	  = 0;
	u32 u4FifoSA_Phy   = 0;
	u32 u4FifoEA_Phy   = 0;
	u32 u4RetryCnt	  = 0;

	_PVR_Lock();

RETRYSP:

	UNUSE_PARAMETER(u4Status);

	smp_mb();
	u4RetryCnt++;

	smp_mb();
	/* For PES, Status 2 Reg's value is the starting address of PES header*/
	u4EndAddr	= u4Status2;	/*DMXCMD_READ32(DMX_REG_FTuP_NONERR_STATUS_REG2);	114*/

	smp_mb();
	prPidStruct = _PVR_GetPidStruct(u1PIDIdx);

	smp_mb();
	if (NULL == prPidStruct) {
		_PVR_Unlock();
		PVR_LOG_ERR(TEXT("[PVR] %s failed for invalid Pid Idx: %u\r\n"),
			DMX_FUNC_NAME, u1PIDIdx);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_HW_ERROR);
	}

	smp_mb();
	u4HdrBufStart = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_SA);
	u4HdrBufEnd   = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_EA) + 1;
	u4HdrBufWp	  = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP);
	u4HdrBufRp	  = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP);
	u4FifoSA_Phy  = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_SA);
	u4FifoEA_Phy  = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_EA) + 1;

	smp_mb();
	if (((PID_S_W(u1PIDIdx, 0))&(1 << 6)) == 0) {
		if ((u4HdrBufStart != 0) && (u4HdrBufEnd != 0)) {
			PVR_LOG_DBG(TEXT("[PVR] Audio -- Circular header buffer: CurPesPA(0x%08x),")
				TEXT(" PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				u4FirstAddr, PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP),
				u4HdrBufStart, u4HdrBufEnd);
			PVR_LOG_DBG(TEXT("[PVR] Audio -- Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x),")
				TEXT(" PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP),
				u4FifoSA_Phy, u4FifoEA_Phy);
			smp_mb();

			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP) = u4HdrBufWp;
			mb();
		}

		smp_mb();
		if (NULL != prPidStruct->pfnNotify) {
			MRESULT mrRet = RET_DMX_OK;

			mrRet = prPidStruct->pfnNotify(u1PIDIdx, PVR_NOTIFY_CODE_ES,
				 0, prPidStruct->pvNotifyTag);
			if (DMX_FAILED(mrRet)) {
				_PVR_Unlock();
				PVR_LOG_ERR(TEXT("[PVR] %s failed in pfnNotify, mrRet: 0x%x, Pid Idx: %u\r\n"),
					DMX_FUNC_NAME, mrRet, u1PIDIdx);
				MM_RETURN(mrRet);
			}
		}
		_PVR_Unlock();
		smp_mb();

		MM_RETURN(RET_DMX_OK);
	}

	if ((u4HdrBufStart != 0) && (u4HdrBufEnd != 0) && (u4EndAddr != 0) &&
		(u4HdrBufWp != u4HdrBufRp)) {
		PVR_LOG_DBG(TEXT("[PVR] Audio -- Circular header buffer: CurPesPA(0x%08x),")
			TEXT(" PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
			u4FirstAddr, PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP),
			u4HdrBufStart, u4HdrBufEnd);
		PVR_LOG_DBG(TEXT("[PVR] Audio -- Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x),")
			TEXT(" PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP),
			u4FifoSA_Phy, u4FifoEA_Phy);

		smp_mb();

		if ((u4EndAddr < u4HdrBufStart) || (u4EndAddr >= u4HdrBufEnd)) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d fail for PesStartAddr(0x%08x) isn't")
				TEXT(" in Pes Buffer [0x%08x, 0x%08x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4EndAddr, u4HdrBufStart, u4HdrBufEnd);
			PVR_LOG_ERR(TEXT("[PVR] Circular header buffer: CurPesPA(0x%08x),")
				TEXT(" PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				u4FirstAddr, PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP),
				u4HdrBufStart, u4HdrBufEnd);
			PVR_LOG_ERR(TEXT("[PVR] Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x),")
				TEXT(" PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP),
				u4FifoSA_Phy, u4FifoEA_Phy);
			if (u4RetryCnt > DMX_HW_RETRY_CNT) {
				_PVR_Unlock();
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_HW_ERROR);
			} else {
				goto RETRYSP;
			}
		}

		smp_mb();
		u4ReadPtr = 0;
		smp_mb();
		CHECK_RING_BOUNDARY2(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd,
			pu4PESData, PVR_FTUP_OTH_DMA_THRESHOLD);
		smp_mb();

		u4FirstAddr = u4EndAddr;
		smp_mb();

		pu4PESData = (u32 *)DMX_NONCACHE(u4EndAddr);
		smp_mb();

		if (NULL == pu4PESData) {
			_PVR_Unlock();
			PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for invalid PES start Address: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4EndAddr);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		smp_mb();
		/* PES Header Byte Count(2B) value in PES header circular buffer*/
		u4PESHdrLen = (pu4PESData[u4ReadPtr] & 0xFFFF0000) >> 16;	 /*PES Information*/
		if (0 != u4PESHdrLen) {
			_PVR_Unlock();
			PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for PES Header Len(%d) > 0\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4PESHdrLen);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		smp_mb();
		/* u32 1 (End Address + 1 of the corresponding PES payload buffer)*/
		u4ReadPtr++;
		CHECK_RING_BOUNDARY(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd, pu4PESData);
		if (NULL == pu4PESData) {
			_PVR_Unlock();
			PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for unexpect error\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		smp_mb();
		u4DataEndAddr = pu4PESData[u4ReadPtr];
		/*pu4PESData[u4ReadPtr] = (pu4PESData[u4ReadPtr] | DMX_HDRCHECK_PATTERN);*/

		smp_mb();
		if ((u4DataEndAddr < u4FifoSA_Phy) ||
			(u4DataEndAddr > u4FifoEA_Phy)) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d fail for PesEndAddr(0x%08x) isn't")
				TEXT(" in DstFifo [0x%08x, 0x%08x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4DataEndAddr, u4FifoSA_Phy, u4FifoEA_Phy);
			PVR_LOG_ERR(TEXT("[PVR] Circular header buffer: CurPesPA(0x%08x),")
				TEXT(" PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				u4FirstAddr, PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP),
				u4HdrBufStart, u4HdrBufEnd);
			PVR_LOG_ERR(TEXT("[PVR] Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x),")
				TEXT(" PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP),
				u4FifoSA_Phy, u4FifoEA_Phy);
			if (u4RetryCnt > DMX_HW_RETRY_CNT) {
				_PVR_Unlock();
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_HW_ERROR);
			} else {
				goto RETRYSP;
			}
		}

		smp_mb();
		/*DWORD2 (Starting Address of the corresponding PES payload buffer)*/
		u4ReadPtr++;
		smp_mb();
		CHECK_RING_BOUNDARY(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd, pu4PESData);
		if (NULL == pu4PESData) {
			_PVR_Unlock();
			PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for unexpect error\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		smp_mb();
		u4DataStartAddr = pu4PESData[u4ReadPtr];
		/*pu4PESData[u4ReadPtr] = (pu4PESData[u4ReadPtr] | DMX_HDRCHECK_PATTERN);*/

		smp_mb();
		if ((u4DataStartAddr < u4FifoSA_Phy) ||
			(u4DataStartAddr > u4FifoEA_Phy)) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d fail for PesEndAddr(0x%08x) isn't")
				TEXT(" in DstFifo [0x%08x, 0x%08x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4DataStartAddr, u4FifoSA_Phy, u4FifoEA_Phy);
			PVR_LOG_ERR(TEXT("[PVR] Circular header buffer: CurPesPA(0x%08x),")
				TEXT(" PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				u4FirstAddr, PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP),
				u4HdrBufStart, u4HdrBufEnd);
			PVR_LOG_ERR(TEXT("[PVR] Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x),")
				TEXT(" PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP),
				u4FifoSA_Phy, u4FifoEA_Phy);
			if (u4RetryCnt > DMX_HW_RETRY_CNT) {
				_PVR_Unlock();
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_HW_ERROR);
			} else {
				goto RETRYSP;
			}
		}

		smp_mb();
		/* PES Header Area len should be 0*/
		u4ReadPtr++;
		smp_mb();
		CHECK_RING_BOUNDARY(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd, pu4PESData);
		if (NULL == pu4PESData) {
			_PVR_Unlock();
			PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for unexpect error\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		smp_mb();
		#if !DMX_HWMEM_ALL_USE_RSVMEM
		if (u4DataEndAddr < u4DataStartAddr) {
			void *pvFlushSa = (void *)DMX_NONCACHE(u4DataStartAddr);

			if (NULL != pvFlushSa)
				CacheRangeFlush(pvFlushSa, (u4FifoEA_Phy - u4DataStartAddr),
					CACHE_SYNC_DISCARD);

			pvFlushSa = (void *)DMX_NONCACHE(u4FifoSA_Phy);
			if (NULL != pvFlushSa)
				CacheRangeFlush(pvFlushSa, (u4DataEndAddr - u4FifoSA_Phy),
					CACHE_SYNC_DISCARD);

		} else if (u4DataEndAddr > u4DataStartAddr) {
			void *pvFlushSa = (void *)DMX_NONCACHE(u4DataStartAddr);

			if (NULL != pvFlushSa)
				CacheRangeFlush(pvFlushSa, (u4DataEndAddr - u4DataStartAddr),
					CACHE_SYNC_DISCARD);

		}
		#endif /* !DMX_HWMEM_ALL_USE_RSVMEM*/

		smp_mb();
		CHECK_RING_BOUNDARY(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd, pu4PESData);
		smp_mb();
		if (NULL == pu4PESData) {
			_PVR_Unlock();
			PVR_LOG_ERR(TEXT("[PVR] %s Line: %d failed for unexpect error\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		smp_mb();
		PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP) = u4EndAddr + u4ReadPtr * 4;
		mb();
	}

	_PVR_Unlock();

	smp_mb();
	if (NULL != prPidStruct->pfnNotify) {
		MRESULT mrRet = RET_DMX_OK;

		mrRet = prPidStruct->pfnNotify(u1PIDIdx, PVR_NOTIFY_CODE_ES,
			 0, prPidStruct->pvNotifyTag);
		if (DMX_FAILED(mrRet)) {
			PVR_LOG_ERR(TEXT("[PVR] %s failed in pfnNotify, mrRet: 0x%x, Pid Idx: %u\r\n"),
				DMX_FUNC_NAME, mrRet, u1PIDIdx);
			MM_RETURN(mrRet);
		}
	}
	smp_mb();

	MM_RETURN(RET_DMX_OK);
}

MRESULT _PVR_ProcessMM_Section(u8 u1PIDIdx, u32 u4Status, u32 u4Status2)
{
	PVR_PID_STRUCT_T *prPidStruct = NULL;
	u32 *pu4PESData	   = NULL;
	u32 u4PESHdrLen	   = 0;
	u32 u4DataEndAddr   = 0;
	u32 u4DataStartAddr = 0;
	u32 u4EndAddr	  = 0;
	u32 u4ReadPtr	  = 0;
	u32 u4HdrBufStart  = 0;
	u32 u4HdrBufEnd	  = 0;
	u32 u4HdrBufWp	  = 0;
	u32 u4HdrBufRp	  = 0;
	u32 u4FirstAddr	  = 0;
	u32 u4FifoSA_Phy   = 0;
	u32 u4FifoEA_Phy   = 0;
	u32 u4RetryCnt = 0;

	_PVR_Lock();

RETRYSECTION:

	UNUSE_PARAMETER(u4Status);

	u4RetryCnt++;
	smp_mb();

	/* For PES, Status 2 Reg's value is the starting address of PES header*/
	u4EndAddr	= u4Status2;	/*DMXCMD_READ32(DMX_REG_FTuP_NONERR_STATUS_REG2);	114*/
	smp_mb();

	prPidStruct = _PVR_GetPidStruct(u1PIDIdx);
	smp_mb();

	if (NULL == prPidStruct) {
		_PVR_Unlock();
		PVR_LOG_ERR(TEXT("[PVR] %s failed for invalid Pid Idx: %u\r\n"),
			DMX_FUNC_NAME, u1PIDIdx);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_HW_ERROR);
	}

	smp_mb();
	u4HdrBufStart = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_SA);
	u4HdrBufEnd   = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_EA) + 1;
	u4HdrBufWp	  = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP);
	u4HdrBufRp	  = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP);
	u4FifoSA_Phy  = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_SA);
	u4FifoEA_Phy  = PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_EA) + 1;

	smp_mb();
	if (((PID_S_W(u1PIDIdx, 0))&(1 << 6)) == 0) {
		if ((u4HdrBufStart != 0) && (u4HdrBufEnd != 0)) {
			PVR_LOG_DBG(TEXT("[PVR] Audio -- Circular header buffer: CurPesPA(0x%08x),")
				TEXT(" PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				u4FirstAddr, PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP),
				u4HdrBufStart, u4HdrBufEnd);
			PVR_LOG_DBG(TEXT("[PVR] Audio -- Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x),")
				TEXT(" PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP),
				u4FifoSA_Phy, u4FifoEA_Phy);

			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP) = u4HdrBufWp;
			mb();
		}

		smp_mb();
		if (NULL != prPidStruct->pfnNotify) {
			MRESULT mrRet = RET_DMX_OK;

			mrRet = prPidStruct->pfnNotify(u1PIDIdx, PVR_NOTIFY_CODE_ES,
				 0, prPidStruct->pvNotifyTag);
			smp_mb();
			if (DMX_FAILED(mrRet)) {
				_PVR_Unlock();
				PVR_LOG_ERR(TEXT("[PVR] %s failed in pfnNotify, mrRet: 0x%x, Pid Idx: %u\r\n"),
					DMX_FUNC_NAME, mrRet, u1PIDIdx);
				MM_RETURN(mrRet);
			}
		}
		_PVR_Unlock();
		smp_mb();

		MM_RETURN(RET_DMX_OK);
	}

	smp_mb();
	if ((u4HdrBufStart != 0) && (u4HdrBufEnd != 0) &&
		(u4EndAddr != 0) && (u4HdrBufWp != u4HdrBufRp)) {
		PVR_LOG_DBG(TEXT("[PVR] Audio -- Circular header buffer: CurPesPA(0x%08x),")
			TEXT(" PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
			u4FirstAddr, PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP),
			u4HdrBufStart, u4HdrBufEnd);
		PVR_LOG_DBG(TEXT("[PVR] Audio -- Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x),")
			TEXT(" PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP),
			PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP),
			u4FifoSA_Phy, u4FifoEA_Phy);

		smp_mb();
		if ((u4EndAddr < u4HdrBufStart) || (u4EndAddr >= u4HdrBufEnd)) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d fail for PesStartAddr(0x%08x) ")
				TEXT("isn't in Pes Buffer [0x%08x, 0x%08x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4EndAddr, u4HdrBufStart, u4HdrBufEnd);
			PVR_LOG_ERR(TEXT("[PVR] Circular header buffer: CurPesPA(0x%08x), ")
				TEXT("PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				u4FirstAddr, PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP),
				u4HdrBufStart, u4HdrBufEnd);
			PVR_LOG_ERR(TEXT("[PVR] Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x), ")
				TEXT("PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP),
				u4FifoSA_Phy, u4FifoEA_Phy);
			smp_mb();
			if (u4RetryCnt > DMX_HW_RETRY_CNT) {
				_PVR_Unlock();
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_HW_ERROR);
			} else {
				goto RETRYSECTION;
			}
		}

		smp_mb();
		u4ReadPtr = 0;

		smp_mb();
		CHECK_RING_BOUNDARY2(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd,
			pu4PESData, PVR_FTUP_OTH_DMA_THRESHOLD);

		smp_mb();
		u4FirstAddr = u4EndAddr;

		smp_mb();
		pu4PESData = (u32 *)DMX_NONCACHE(u4EndAddr);

		smp_mb();
		if (NULL == pu4PESData) {
			_PVR_Unlock();
			PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for invalid PES start Address: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4EndAddr);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		smp_mb();
		/* PES Header Byte Count(2B) value in PES header circular buffer*/
		u4PESHdrLen = (pu4PESData[u4ReadPtr] & 0xFFFF0000) >> 16;	 /*PES Information*/
		smp_mb();
		if (0 != u4PESHdrLen) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for PES Header Len(%d) > 0\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4PESHdrLen);
			if (u4RetryCnt > DMX_HW_RETRY_CNT) {
				_PVR_Unlock();
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_HW_ERROR);
			} else {
				goto RETRYSECTION;
			}
		}

		smp_mb();
		/* u32 1 (End Address + 1 of the corresponding PES payload buffer)*/
		u4ReadPtr++;
		CHECK_RING_BOUNDARY(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd, pu4PESData);
		smp_mb();
		if (NULL == pu4PESData) {
			_PVR_Unlock();
			PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for unexpect error\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		smp_mb();
		u4DataEndAddr = pu4PESData[u4ReadPtr];
		/*pu4PESData[u4ReadPtr] = (pu4PESData[u4ReadPtr] | DMX_HDRCHECK_PATTERN);*/

		smp_mb();
		if ((u4DataEndAddr < u4FifoSA_Phy) ||
			(u4DataEndAddr > u4FifoEA_Phy)) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d fail for PesEndAddr(0x%08x) isn't in DstFifo")
				TEXT(" [0x%08x, 0x%08x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4DataEndAddr, u4FifoSA_Phy, u4FifoEA_Phy);
			PVR_LOG_ERR(TEXT("[PVR] Circular header buffer: CurPesPA(0x%08x), PhyRP(0x%08x),")
				TEXT(" PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				u4FirstAddr, PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP),
				u4HdrBufStart, u4HdrBufEnd);
			PVR_LOG_ERR(TEXT("[PVR] Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x),")
				TEXT(" PhyEA(0x%08x)\r\n"),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP),
				u4FifoSA_Phy, u4FifoEA_Phy);
			if (u4RetryCnt > DMX_HW_RETRY_CNT) {
				_PVR_Unlock();
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_HW_ERROR);
			} else {
				goto RETRYSECTION;
			}
		}

		/*DWORD2 (Starting Address of the corresponding PES payload buffer)*/
		smp_mb();
		u4ReadPtr++;
		smp_mb();
		CHECK_RING_BOUNDARY(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd, pu4PESData);
		smp_mb();
		if (NULL == pu4PESData) {
			_PVR_Unlock();
			PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for unexpect error\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		smp_mb();
		u4DataStartAddr = pu4PESData[u4ReadPtr];
		/*pu4PESData[u4ReadPtr] = (pu4PESData[u4ReadPtr] | DMX_HDRCHECK_PATTERN);*/
		smp_mb();

		if ((u4DataStartAddr < u4FifoSA_Phy) ||
			(u4DataStartAddr > u4FifoEA_Phy)) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d fail for PesEndAddr(0x%08x) isn't in DstFifo")
				TEXT(" [0x%08x, 0x%08x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4DataStartAddr, u4FifoSA_Phy, u4FifoEA_Phy);
			PVR_LOG_ERR(TEXT("[PVR] Circular header buffer: CurPesPA(0x%08x), PhyRP(0x%08x),")
				TEXT(" PhyWP(0x%08x), PhySA(0x%08x), PhyEA(0x%08x)\r\n"),
				u4FirstAddr, PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_WP),
				u4HdrBufStart, u4HdrBufEnd);
			PVR_LOG_ERR(TEXT("[PVR] Dest Fifo: PhyRP(0x%08x), PhyWP(0x%08x), PhySA(0x%08x),")
				TEXT(" PhyEA(0x%08x)\r\n"),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_RP),
				PID_S_W(u1PIDIdx, PID_DATA_STRUCT_FIFO_WP),
				u4FifoSA_Phy, u4FifoEA_Phy);
			if (u4RetryCnt > DMX_HW_RETRY_CNT) {
				_PVR_Unlock();
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_HW_ERROR);
			} else {
				goto RETRYSECTION;
			}
		}

		smp_mb();
		/* PES Header Area len should be 0*/
		u4ReadPtr++;
		smp_mb();
		CHECK_RING_BOUNDARY(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd, pu4PESData);
		smp_mb();
		if (NULL == pu4PESData) {
			_PVR_Unlock();
			PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for unexpect error\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		smp_mb();
		#if !DMX_HWMEM_ALL_USE_RSVMEM
		if (u4DataEndAddr < u4DataStartAddr) {
			void *pvFlushSa = (void *)DMX_NONCACHE(u4DataStartAddr);

			if (NULL != pvFlushSa) {
				CacheRangeFlush(pvFlushSa, (u4FifoEA_Phy - u4DataStartAddr),
					CACHE_SYNC_DISCARD);
			}
			pvFlushSa = (void *)DMX_NONCACHE(u4FifoSA_Phy);
			if (NULL != pvFlushSa) {
				CacheRangeFlush(pvFlushSa, (u4DataEndAddr - u4FifoSA_Phy),
					CACHE_SYNC_DISCARD);
			}
		} else if (u4DataEndAddr > u4DataStartAddr) {
			void *pvFlushSa = (void *)DMX_NONCACHE(u4DataStartAddr);

			if (NULL != pvFlushSa)
				CacheRangeFlush(pvFlushSa, (u4DataEndAddr - u4DataStartAddr),
					CACHE_SYNC_DISCARD);
		}
		#endif /* !DMX_HWMEM_ALL_USE_RSVMEM*/

		smp_mb();
		CHECK_RING_BOUNDARY(u4EndAddr, u4ReadPtr, u4HdrBufStart, u4HdrBufEnd, pu4PESData);
		smp_mb();
		if (NULL == pu4PESData) {
			_PVR_Unlock();
			PVR_LOG_ERR(TEXT("[PVR] %s Line: %d failed for unexpect error\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		PID_S_W(u1PIDIdx, PID_DATA_STRUCT_HEADER_RP) = u4EndAddr + u4ReadPtr * 4;
		mb();
	}
	smp_mb();

	_PVR_Unlock();

	if (NULL != prPidStruct->pfnNotify) {
		MRESULT mrRet = RET_DMX_OK;

		mrRet = prPidStruct->pfnNotify(u1PIDIdx, PVR_NOTIFY_CODE_ES,
			 0, prPidStruct->pvNotifyTag);
		if (DMX_FAILED(mrRet)) {
			PVR_LOG_ERR(TEXT("[PVR] %s failed in pfnNotify, mrRet: 0x%x, Pid Idx: %u\r\n"),
				DMX_FUNC_NAME, mrRet, u1PIDIdx);
			MM_RETURN(mrRet);
		}
	}
	smp_mb();

	MM_RETURN(RET_DMX_OK);
}

MRESULT _PVR_MMIntrHandler(void *pvArg)
{
	PVR_FTUP_INT_STATUS_INFO_T *prIntStatus = (PVR_FTUP_INT_STATUS_INFO_T *)pvArg;
	MRESULT mrRet = RET_DMX_OK;
	u32  u4Counter = 0;
    	u32  u4Ctrl = 0;
    	u32  u4DDIWP = 0, u4DDIRP = 0;
	u8	u1PIDIdx = DMX_INVALID_UINT8;

	PVR_LOG_DBG(TEXT("++++++ [PVR] %s Enter +++++++\r\n"), DMX_FUNC_NAME);

	if (NULL == prIntStatus) {
		DMXLOG_ERROR(TEXT("[PVR] %s line %d, fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		/* Notify DDI that Transfer is completed.*/
		_PVR_DDI_EndSingleMove();
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	
	// DDI RP is 4-bytes aligned, WP is 1-byte aligned.
	u4DDIWP = DDI_READ32(DDI_REG_DMA_WP) & (0xFFFFFFFC);
	u4DDIRP = DDI_READ32(DDI_REG_DMA_RP);
	u4Counter = 0;
	while ((u4DDIRP != u4DDIWP) &&
		(u4DDIRP + 4 < u4DDIWP) &&
		(u4Counter < 1000)) {
		u4DDIRP = DDI_READ32(DDI_REG_DMA_RP);
		u4Counter++;
	}
	
	u4Counter = 0;
	while (u4Counter < 1000) {
		u4Ctrl = DMXCMD_READ32(PVR_REG_REG_FILE_ADDR_REG);
		if (0 != (u4Ctrl & (1 << 30)))
			break;
		u4Counter++;
	}

	smp_mb();
	u1PIDIdx = prIntStatus->u1PIDIdx;		  /* PID Index*/
	smp_mb();

	if (PVR_PID_IDX_VIDEO == u1PIDIdx) {
		mrRet = _PVR_ProcessMM_Video(u1PIDIdx, prIntStatus->u4Status, prIntStatus->u4Status2);
	} else if (PVR_PID_IDX_AUDIO == u1PIDIdx) {
		mrRet = _PVR_ProcessMM_Audio(u1PIDIdx, prIntStatus->u4Status, prIntStatus->u4Status2);
	} else if (PVR_PID_IDX_SP == u1PIDIdx) {
		mrRet = _PVR_ProcessMM_SP(u1PIDIdx, prIntStatus->u4Status, prIntStatus->u4Status2);
	} else if (PVR_PID_IDX_SECTION == u1PIDIdx) {
		mrRet = _PVR_ProcessMM_Section(u1PIDIdx, prIntStatus->u4Status, prIntStatus->u4Status2);
	} else {
		PVR_LOG_ERR(TEXT("------ [PVR] %s fail for Error PID Index(%u) ------\r\n"),
			DMX_FUNC_NAME, u1PIDIdx);
		MM_RETURN(RET_DMX_HW_ERROR);
	}

	smp_mb();
	if (DMX_FAILED(mrRet)) {
		PVR_LOG_ERR(TEXT("[PVR] %s failed in Stream Interrupt Handler, Pid Idx: %u, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, u1PIDIdx, mrRet);
		/* Notify DDI that Transfer is completed.*/
		_PVR_DDI_EndSingleMove();
		_PVR_DDI_DumpInfo();
		_PVR_DumpPIDDataStruct(u1PIDIdx);
		_PVR_DumpPESHdrInfo(u1PIDIdx);
		MM_RETURN(mrRet);
	}

	smp_mb();
	/* Notify DDI that Transfer is completed.*/
	_PVR_DDI_EndSingleMove();

	smp_mb();
	PVR_LOG_DBG(TEXT("------ [PVR] %s Exit ------\r\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

/*-----------------------------------------------------------------------------*/
/** _DmxDescramblerInt
 *	Interrupt handler of descrambler
 *
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/
/** _PvrDescramblerInt
 *	Interrupt handler of descrambler
 *
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
static void _DmxDescramblerInt(void)
{
	/* Clear interrupt*/
	DMXCMD_WRITE32(PVR_REG_DESCRAMBLER_NONERR_STATUS_REG, 1);
}

/*-----------------------------------------------------------------------------*/
/** _DmxPvrInt
 *	Interrupt handler of PVR
 *
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
static void _DmxPvrInt(void)
{
	/* Clear interrupt*/
	DMXCMD_WRITE32(PVR_REG_PVR_NONERR_STATUS_REG, 1);
}

static void _DmxPcrInt(u32 u4Status)
{
	UNUSE_PARAMETER(u4Status);
	DMXCMD_WRITE32(PVR_REG_PCR_NONERR_STATUS_REG1, 1);
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_PCR2_NONERR_STATUS_REG1, 1); /*no PCR2 in 8555*/
}

/*-----------------------------------------------------------------------------*/
/** _DmxFifoFull
 *	FIFO full handling
 *
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
static void _DmxFifoFull(u8 u1PIDIdx)
{
	u32 u4BufStart, u4BufEnd, u4Wp, u4Rp, u4BufSize, u4DataSize, u4Threshold, u4FullGap;
	PVR_PID_STRUCT_T *prPidStruct;

	prPidStruct = _PVR_GetPidStruct(u1PIDIdx);

	if (NULL == prPidStruct) {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for invalid Pid Index %u\r\n"),
			DMX_FUNC_NAME, u1PIDIdx);
		return;
	}

	smp_mb();
	/* Check data size*/
	u4BufStart = PID_S_W(u1PIDIdx, 5);
	u4BufEnd   = PID_S_W(u1PIDIdx, 6);
	u4Wp	   = PID_S_W(u1PIDIdx, 8);
	u4Rp	   = PID_S_W(u1PIDIdx, 9);
	smp_mb();
	u4BufSize = (u4BufEnd - u4BufStart) + 1;
	smp_mb();
	u4DataSize = DMX_DATASIZE(u4Rp, u4Wp, u4BufSize);

	smp_mb();
	u4Threshold = PID_S_W(u1PIDIdx, 15);

	smp_mb();
	PVR_LOG_ERR(TEXT("%s line %d -- Pidx %u fifo full! bufsz: 0x%08x, datasz: 0x%08x,")
		TEXT(" u4Threshold: 0x%08x\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, u1PIDIdx, u4BufSize, u4DataSize, u4Threshold);

	PVR_LOG_ERR(TEXT("%s line %d -- Pidx %u fifo full! WP: 0x%08x, RP: 0x%08x, ")
		TEXT("u4BufStart: 0x%08x, u4BufEnd: 0x%x\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, u1PIDIdx, u4Wp, u4Rp, u4BufStart, u4BufEnd);

	smp_mb();
	u4FullGap = ((u4BufSize / 8) * 7);
	if (u4DataSize < u4FullGap) {
		if ((u4Threshold == 0) ||
		((u4Threshold * 188) > (u4BufSize - u4FullGap))) {
			PVR_LOG_ERR(TEXT("[PVR] %s Pidx %u: FIFO full event, reset PID!\r\n"),
				DMX_FUNC_NAME, u1PIDIdx);
		} else {
			PVR_LOG_ERR(TEXT("[PVR] %s Pidx %u: False FIFO full event, not reset PID!\r\n"),
				DMX_FUNC_NAME, u1PIDIdx);
			return;
		}
	}

	smp_mb();
	/* Send notification if user handler is installed*/
	if (prPidStruct->pfnNotify != NULL)
		prPidStruct->pfnNotify(u1PIDIdx, PVR_NOTIFY_CODE_OVERFLOW,
			u4DataSize, prPidStruct->pvNotifyTag);

	smp_mb();
	if ((prPidStruct->ePidType != PVR_PID_TYPE_ES_VIDEO) &&
		(prPidStruct->ePidType != PVR_PID_TYPE_PSI)) {
		/* Do not handle FIFO full other than video ES*/
		PVR_LOG_ERR(TEXT("%s -- Do not handle FIFO full other than video ES or PSI!\r\n"), DMX_FUNC_NAME);
		return;
	}
	smp_mb();

	if (prPidStruct->ePidType == PVR_PID_TYPE_PSI)
		PVR_LOG_ERR(TEXT("Pidx %u fifo full! PidType:PVR_PID_TYPE_PSI.\r\n"), u1PIDIdx);
	else
		PVR_LOG_DBG(TEXT("Pidx %u fifo full! PidType: ePidType: %d.\r\n"),
			u1PIDIdx, prPidStruct->ePidType);
}

/*-----------------------------------------------------------------------------*/
/** _DmxFTuPErrorInt
 *	Error handler of uP
 *
 *	@retval bool : return value means handle or not
 */
/*-----------------------------------------------------------------------------*/
static bool _DmxFTuPErrorInt(void)
{
	u32 u4Status;
	u8 u1Pidx, u1Type;

	/* Get interrupt status*/
	u4Status = DMXCMD_READ32(PVR_REG_FTUP_ERROR_STATUS_REG);

	smp_mb();
	u1Pidx = (u8)((u4Status >> 16) & 0xff);
	u1Type = (u8)(u4Status & 0xff);

	smp_mb();
	PVR_LOG_ERR(TEXT("%s line %d -- FTuP Error Status Reg: 0x%08x, u1Pidx: %u, u1Type: %u\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, u4Status, u1Pidx, u1Type);

	smp_mb();
	if ((u1Pidx >= PVR_FVR_START_PID) &&
	(u1Pidx <= (PVR_FVR_START_PID + FVR_NUM_PID_INDEX))) {
		/* FVR record uP error*/
		PVR_LOG_ERR(TEXT("[PVR] %s line %d -- FVR record uP error\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);

		return FALSE;
	}

	smp_mb();
	/* Clear interrupt*/
	DMXCMD_WRITE32(PVR_REG_FTUP_ERROR_STATUS_REG, 1);

	smp_mb();
	if (u1Pidx < PVR_NUM_PID_INDEX) {
		PVR_PID_STRUCT_T *prPidStruct;
		bool fgEnabled;

		prPidStruct = _PVR_GetPidStruct(u1Pidx);
		fgEnabled = _PVR_IsPidEnabled(u1Pidx);

		smp_mb();
		if (!fgEnabled) {
			/* Error interrupt on a disabled pid, caused by race condition*/
			/* between disabling pid and raising interrupt.*/
			/* Simply do nothing here*/
			PVR_LOG_ERR(TEXT("[PVR] %s -- Interrupt on disabled: pidx %u, status 0x%08x\r\n"),
				DMX_FUNC_NAME, u1Pidx, u4Status);

			return TRUE;
		}
	} else {
		PVR_LOG_ERR(TEXT("[PVR] %s -- FTuP error: invalid pid index %u\r\n"),
			DMX_FUNC_NAME, u1Pidx);
		return TRUE;
	}

	smp_mb();
	switch (u1Type) {
	case 1:
		/* TS packet dropped due to lack of output buffer space.*/
		/* Check FIFO full*/
		PVR_LOG_ERR(TEXT("[PVR] %s line %d -- FIFO Full (Pidx: %u)\r\n"), DMX_FUNC_NAME, DMX_LINE_NO, u1Pidx);
		_DmxFifoFull(u1Pidx);
		break;

	case 4:
		PVR_LOG_ERR(TEXT("[PVR] %s line %d -- Pidx: %u: Continuity error. For TS ")
			TEXT("Packets containing sections, the partial section will be dropped!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u1Pidx);
		break;

	case 5:
		PVR_LOG_ERR(TEXT("[PVR] %s line %d -- Pidx: %u: Non-AV PES, PUSI=1, PES ")
			TEXT("Remaining Length != 0!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u1Pidx);
		break;

	case 32:
		PVR_LOG_ERR(TEXT("[PVR] %s line %d -- Pidx: %u: TS Packet with PUSI=0 is ")
			TEXT("found when the PES Remaing length is 0. A pseudo PES header will ")
			TEXT("be created in the PES header circular buffer!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u1Pidx);
		break;

	case 101:
		/* uCode fatal error, PES header is missing*/
		PVR_LOG_ERR(TEXT("[PVR] %s line %d -- Pidx: %u: uCode fatal error - ")
				TEXT("PES header missing!\r\n"), DMX_FUNC_NAME, DMX_LINE_NO, u1Pidx);
		break;

	default:
		PVR_LOG_ERR(TEXT("[PVR] %s -- Pidx %u Encounter error %u!\r\n"),
			DMX_FUNC_NAME, u1Pidx, u1Type);
		break;
	}

	PVR_LOG_ERR(TEXT("[PVR] %s -- Ftup error happen, s32 = 0x%x !!!!!!\r\n"),
		DMX_FUNC_NAME, u4Status);

	return TRUE;
}

/*-----------------------------------------------------------------------------*/
/** _DmxDbmErrorInt
 *	Error handler of DBM
 *
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
static void _DmxDbmErrorInt(void)
{
	u32 u4Status, u4ErrorStatus;
	u32 i;

	u4Status = DMXCMD_READ32(PVR_REG_DBM_ERROR_STATUS_REG);
	smp_mb();
	u4ErrorStatus = u4Status & 0xFF;

	smp_mb();
	for (i = 0 ; i < 15 ; i++) {
		if (((u4ErrorStatus >> i) & 0x1) == 0x1) {
			switch (i) {
			case 0:
				PVR_LOG_ERR(TEXT("[PVR] %s line %d (DBM Error) TS packet dropped due to ")
					TEXT("lack of input buffer space!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				break;

			case 1:
				PVR_LOG_ERR(TEXT("[PVR] %s line %d DBM Error -- Framer 0 lost sync!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				break;

			case 2:
				PVR_LOG_ERR(TEXT("[PVR] %s line %d DBM Error -- Framer 1 lost sync!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				break;

			case 5:
				PVR_LOG_ERR(TEXT("[PVR] %s line %d DBM Error -- Framer FIFO overflowed")
					TEXT(" and TS packet has been drop!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				break;

			case 6:
				PVR_LOG_ERR(TEXT("[PVR] %s line %d DBM Error -- Framer 0 input clock ")
					TEXT("glitch happens!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				_PVR_ResetFramer(0);
				break;

			case 7:
				PVR_LOG_ERR(TEXT("[PVR] %s line %d DBM Error -- Framer 1 input clock ")
					TEXT("glitch happens!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				_PVR_ResetFramer(1);
				break;

			case 10:
				PVR_LOG_ERR(TEXT("[PVR] %s line %d DBM Error -- Framer 0 Unlock!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				_PVR_ResetFramer(0);
				break;

			case 11:
				PVR_LOG_ERR(TEXT("[PVR] %s line %d DBM Error -- Framer 1 Unlock!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				_PVR_ResetFramer(1);
				break;

			default:
				PVR_LOG_ERR(TEXT("[PVR] %s line %d -- Unknown DBM error status (0x%02x)!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, (u4Status & 0xFF));
				break;
			}

		}
	}

	smp_mb();
	/* Clear interrupt*/
	DMXCMD_WRITE32(PVR_REG_DBM_ERROR_STATUS_REG, 1);

	PVR_LOG_ERR(TEXT("[PVR] %s -- DBM error! s32: 0x%08x\r\n"),
		DMX_FUNC_NAME, u4Status);
}


/*-----------------------------------------------------------------------------*/
/** _DmxDescramblerErrorInt
 *	Error handler of descrambler
 *
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
static void _DmxDescramblerErrorInt(void)
{
	u32 u4BufStart, u4BufEnd, u4BufSize, u4Rp, u4Wp, u4Status;
	u32 u4Ctrl;
	u8  u1Pidx;

	/* Let buffer be half-full*/
	u4BufStart = DMXCMD_READ32(PVR_REG_CA_OUT_BUF_START);
	u4BufEnd = DMXCMD_READ32(PVR_REG_CA_OUT_BUF_END);
	smp_mb();
	u4BufSize = (u4BufEnd - u4BufStart) + 1;
	u4Wp = DMXCMD_READ32(PVR_REG_CA_OUT_BUF_WP);
	smp_mb();
	u4Rp = u4Wp - (u4BufSize / 2);
	smp_mb();
	if (u4Rp < u4BufStart)
		u4Rp += u4BufSize;

	smp_mb();
	/* Disable output buffer*/
	u4Ctrl = DMXCMD_READ32(PVR_REG_CA_CTRL);
	u4Ctrl &= ~((1 << 7) | (1 << 15));
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_CA_CTRL, u4Ctrl);

	smp_mb();
	/* Re-initialized output buffer*/
	DMXCMD_WRITE32(PVR_REG_CA_OUT_BUF_START, u4BufStart);
	DMXCMD_WRITE32(PVR_REG_CA_OUT_BUF_END, u4BufEnd);
	DMXCMD_WRITE32(PVR_REG_CA_OUT_BUF_RP, u4Rp);
	DMXCMD_WRITE32(PVR_REG_CA_OUT_BUF_WP, u4Wp);

	smp_mb();
	/* Re-enable output buffer*/
	u4Ctrl = DMXCMD_READ32(PVR_REG_CA_CTRL);
	u4Ctrl |= (1 << 7) | (1 << 15);
	smp_mb();
	DMXCMD_WRITE32(PVR_REG_CA_CTRL, u4Ctrl);
	smp_mb();

	u4Status = DMXCMD_READ32(PVR_REG_DESCRAMBLER_ERROR_STATUS_REG);
	u1Pidx = (u8)((u4Status >> 16) & 0xff);
	smp_mb();

	DMXCMD_WRITE32(PVR_REG_DESCRAMBLER_ERROR_STATUS_REG, 1);

	PVR_LOG_ERR(TEXT("[PVR] %s -- Descrambler error! s32: 0x%08x, Pidx(%u)\r\n"),
		DMX_FUNC_NAME,	u4Status, u1Pidx);
}


/*-----------------------------------------------------------------------------*/
/** _DmxPvrErrorInt
 *	Error handler of PVR
 *
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
static void _DmxPvrErrorInt(void)
{
	u32 u4Status;
	u16 u2Pid;

	u4Status = DMXCMD_READ32(PVR_REG_PVR_ERROR_STATUS_REG);
	smp_mb();
	u2Pid = (u16)((u4Status >> 16) & 0x1fff);
	smp_mb();

	/* Clear interrupt*/
	DMXCMD_WRITE32(PVR_REG_PVR_ERROR_STATUS_REG, 1);
	smp_mb();

	PVR_LOG_ERR(TEXT("[PVR] %s -- PVR error! s32: 0x%08x, PID = 0x%x\r\n"),
		DMX_FUNC_NAME, u4Status, u2Pid);
}

/*-----------------------------------------------------------------------------*/
/** _DmxSteerErrorInt
 *	Error handler of steering logic
 *
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
static void _DmxSteerErrorInt(void)
{
	u32 u4Status;
	u8  u1PIDIdx, u1Type;

	u4Status = DMXCMD_READ32(PVR_REG_STEER_ERROR_STATUS_REG);
	smp_mb();
	u1Type = (u8)((u4Status & 0xff));
	u1PIDIdx = (u8)((u4Status >> 16) & 0xff);
	smp_mb();

	PVR_LOG_ERR(TEXT("[PVR] %s -- Steering error! s32: 0x%08x, u1PIDIdx: %u\r\n"),
		DMX_FUNC_NAME, u4Status, u1PIDIdx);

	if (u1Type == 2) {
		PVR_LOG_ERR(TEXT("[PVR] %s -- (u1Type == 2) Steering error! Lack of input")
			TEXT(" buffer space at the FTuP, check FIFO full\r\n"),
			DMX_FUNC_NAME);
		/* Lack of input buffer space at the FTuP, check FIFO full*/
		_DmxFifoFull(u1PIDIdx);
	}

	/* Clear interrupt*/
	DMXCMD_WRITE32(PVR_REG_STEER_ERROR_STATUS_REG, 1);
}


/*-----------------------------------------------------------------------------*/
/** _DmxPlaybackSteerErrorInt
 *	Error handler of Playback steering logic
 *
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
static void _DmxPlaybackSteerErrorInt(void)
{
	u32 u4Status;

	u4Status = DMXCMD_READ32(PVR_REG_PLAYBACK_FRAMER_ERROR_STATUS);

	PVR_LOG_ERR(TEXT("Playback steering error! s32: 0x%08x\r\n"), u4Status);

	/* Clear interrupt*/
	DMXCMD_WRITE32(PVR_REG_PLAYBACK_FRAMER_ERROR_STATUS, 0);
}

/*-----------------------------------------------------------------------------*/
/** _DmxDispatchFTuPInterrupt
 *	Dispatch uP interrupts
 *
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
static void _DmxDispatchFTuPInterrupt(void)
{
	u32 u4IntCnt = 0;

	while (u4IntCnt < PVR_INT_QUEUE_DEPTH) {
		u32 u4Status;
		u8 u1Pidx, u1Type, u1PendInt;
		bool fgEnabled, fgOk, fgScrambled;
		u32 u4Status2;

		u4Status = DMXCMD_READ32(PVR_REG_FTUP_NONERR_STATUS_REG1);	/* 113*/
		u4Status2 = DMXCMD_READ32(PVR_REG_FTUP_NONERR_STATUS_REG2); /* 114*/
		smp_mb();

		u1Pidx = PVR_GET_BYTE(u4Status, 2);				/* PID index*/
		u1Type = PVR_GET_BYTE(u4Status, 0);				/* 0: none, 1: PES, 2: PSI*/
		u1PendInt = PVR_GET_BYTE(u4Status, 3);		/* Pending interrupts*/

		smp_mb();
		if (u1PendInt == 0)
			break;

		smp_mb();
		/* In record PID index, we are using bit[6] and bit[7]*/
		/* for start code pattern 8 nad 9 individually*/
		/* in oryx, there're 128 record pid.*/
		if (u1Type >= 64)
			u1Pidx = u1Pidx & 0x7F;
		smp_mb();

		u4Status2 = DMXCMD_READ32(PVR_REG_FTUP_NONERR_STATUS_REG2);

		smp_mb();
		/*----------------------------------------------*/
		/* Handle record interrupt*/
		/*----------------------------------------------*/
		if (u1Type >= 64) {
			fgOk = FALSE;

			if (u1Pidx >= PVR_NUM_PID_INDEX)
				u1Pidx -= PVR_NUM_PID_INDEX;

			switch (u1Type) {
			case 64:	/* packet count*/
			case 65:	/* I,P,B notification*/
			case 66:	/* Timer notification (End Addr + 1)*/
			case 67:	/* Inserting SIT packet completes*/
			case 68:
				PVR_LOG_TRACE(TEXT("[PVR] %s -- Ftup Record Path Interrupt, Pidx: %u, u1Type: %u\r\n"),
					DMX_FUNC_NAME, u1Pidx, u1Type);
				break;
			default:
				break;
			}

			if (!fgOk) {
				PVR_LOG_ERR(TEXT("[PVR] %s -- Ftup record interrupt: pidx %u invalid")
					TEXT(" reason code 0x%02x!\r\n"),
					DMX_FUNC_NAME, u1Pidx, u1Type);
				break;
			}
		} else {
			/*----------------------------------------------*/
			/* Handle play interrupt*/
			/*----------------------------------------------*/
			fgEnabled = FALSE;

			smp_mb();
			if (u1Pidx < PVR_NUM_PID_INDEX)
				fgEnabled	= _PVR_IsPidEnabled(u1Pidx);
			else
				PVR_LOG_ERR(TEXT("[PVR] %s -- pidx %u exceed, status 0x%08x\r\n"),
					DMX_FUNC_NAME, u1Pidx, u4Status);

			if (!fgEnabled) {
				/* Non-error interrupt on a disabled pid, caused by race condition*/
				/* between disabling pid and raising interrupt.*/
				/* Simply do nothing here*/
				PVR_LOG_ERR(TEXT("[PVR] %s -- Interrupt on disabled: pidx %u, status 0x%08x\r\n"),
					DMX_FUNC_NAME, u1Pidx, u4Status);
			} else {
				fgOk = FALSE;
				switch (u1Type) {
				case 0:	/* No status to report*/
				case 1: /* PES*/
				case 2: /* PSI*/
				case 3: /* Partial PES*/
				case 4: /* Close PES, may due to fifo full, uP cannot write more*/
								/* data into fifo, and need to close unfinish PES*/
					fgOk = TRUE;
					break;

				case 5:    /* Playback MM, after transfer data, FTuP will */
									/*PVR notify by using this interrupt*/
					PVR_LOG_DBG(TEXT("[PVR] Ftup interrupt: PIDIdx: %u, Interrupt ")
						TEXT("Code:0x%02x!\r\n"),
						u1Pidx, u1Type);

					_DmxHWMMIntr(u4Status, u4Status2);

					fgOk = TRUE;
					break;

				case 16:	/* incoming packet is not scrambled*/
				case 17:	/* incoming packet is scrambled*/
					fgScrambled = (u1Type == 17);
					_DmxProcessScrambleChange(u1Pidx, fgScrambled);
					fgOk = TRUE;
					break;

				default:
					/* FIXME: Temporarily remove the following check*/
					break;
				}	/* switch*/

				if (!fgOk) {
					PVR_LOG_ERR(TEXT("[PVR] Ftup interrupt: pidx %u invalid reason")
						TEXT(" code 0x%02x!\r\n"),
						u1Pidx, u1Type);
					break;
				}
			}	/* if (!fgEnabled)*/
		}
		smp_mb();

		/* Clear interrupt*/
		DMXCMD_WRITE32(PVR_REG_FTUP_NONERR_STATUS_REG1, 1);

		u4IntCnt++;
	}	/* while (TRUE)*/
}


/*-----------------------------------------------------------------------------*/
/** _DmxDispatchInterrupt
 *	Dispatch demux interrupt
 *
 *	@param	u4Status		Interrupt status word
 *
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
static void _DmxDispatchInterrupt(u32 u4Status)
{
	bool fgRecuPErrorHandled = TRUE;

	/* Check if spurious interrupt*/
	if (0 == u4Status) {
		PVR_LOG_ERR(TEXT("[PVR] %s -- Spurious interrupt!\r\n"), DMX_FUNC_NAME);
		return;
	}

	smp_mb();
	/* Check if error interrupt*/
	if (u4Status & PVR_INT_ERR_MASK) {
		PVR_LOG_ERR(TEXT("[PVR] %s -- Error s32: 0x%08x\r\n"),
			DMX_FUNC_NAME, u4Status);

		if (u4Status & PVR_INT_ERR_DBM)
			_DmxDbmErrorInt();

		if (u4Status & PVR_INT_ERR_DESCRAMBLER)
			_DmxDescramblerErrorInt();

		if (u4Status & PVR_INT_ERR_PVR)
			_DmxPvrErrorInt();

		if (u4Status & PVR_INT_ERR_STERRING)
			_DmxSteerErrorInt();

		if (u4Status & PVR_INT_ERR_PLAYBACK_STERRING)
			_DmxPlaybackSteerErrorInt();

		if (u4Status & PVR_INT_ERR_FTUP)
			fgRecuPErrorHandled = _DmxFTuPErrorInt();
	}

	smp_mb();
	/* Handle normal cases*/
	if (u4Status & PVR_INT_STATUS_MASK) {
		if (u4Status & PVR_INT_STATUS_FTUP)
			_DmxDispatchFTuPInterrupt();

		if (u4Status & PVR_INT_STATUS_DESCRAMBLER)
			_DmxDescramblerInt();

		if (u4Status & PVR_INT_STATUS_PVR)
			_DmxPvrInt();
	}

	smp_mb();
	/* Special case PVR uP error*/
	/* FVR record uP error (overflow) must handle after normal uP interrupt*/
	if (!fgRecuPErrorHandled)
		/* Clear interrupt*/
		DMXCMD_WRITE32(PVR_REG_FTUP_ERROR_STATUS_REG, 1);

	/* PCR error and status*/
	if (u4Status & PVR_INT_PCR_MASK)
		_DmxPcrInt(u4Status);
}

/*-----------------------------------------------------------------------------*/
/** _DmxIrqHandler
 *	Demux interrupt handler
 *
 *	@param	u2Vector		The IRQ vector
 *
 *	@retval void
 */
/*-----------------------------------------------------------------------------*/
void _DmxIrqHandler(u16 u2Vector)
{
	static bool _fgISR = FALSE;
	u32 u4Status, u4Mask;

	DMX_ASSERT(!_fgISR);

	UNUSED(u2Vector);				  /* AVOID compile warning in release build*/

	_fgISR = TRUE;

	smp_mb();
	u4Status = DMXCMD_READ32(PVR_REG_INT_STAUS);
	u4Mask = DMXCMD_READ32(PVR_REG_INT_MASK);

	smp_mb();
	PVR_LOG_DBG(TEXT("[PVR] Demuxer Intterrupt: u4Status = 0x%08X, u4Mask = 0x%08X\r\n"),
		u4Status, u4Mask);

	smp_mb();
	u4Status &= u4Mask;

	smp_mb();
	if (u4Status) {
		_DmxDispatchInterrupt(u4Status);
	} else {
		PVR_LOG_ERR(TEXT("[PVR] %s line %d -- Spurious demux local interrupt!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
	}

	_fgISR = FALSE;
}

