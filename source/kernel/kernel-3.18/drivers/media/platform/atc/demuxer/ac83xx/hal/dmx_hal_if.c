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
 * @file dmx_hal_if.c
 *
 * @par Project
 *	 MT3360
 *
 * @par Description
 *
 *
 * @par Author_Name
 *	 Shuhui Zhang
 *
 */



/*-----------------------------------------------------------------------------*/
/* Include files*/
/*-----------------------------------------------------------------------------*/
#ifdef __linux__
#include "windows.h"
#include <media/atc/dmx_define.h>
#include <media/atc/perf_timer.h>
/* #include <media/atc/mm_debug.h> */
#else
#include "dmx_define.h"
#include "perf_timer.h"
#include "mm_debug.h"
#endif /*__linux__*/

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_pvr.h"
#include "dmx_pvr_ddi.h"
#include "dmx_hal_if.h"
#include "dmx_spt_os.h"
#include "dmx_pfm.h"
#include "dmx_pvr_mpp.h"

#ifndef __linux__
#pragma warning(disable : 4127) /*disable warning C4127: conditional expression is constant*/
#endif

EXTERN BOOL g_fgDmxDmaTwice;

/*-----------------------------------------------------------------------------*/
/* Interface Functions*/
/*-----------------------------------------------------------------------------*/
MRESULT DMX_HAL_Init(PVR_INPUT_TYPE_T eInputType)
{
	MRESULT mrRet = RET_DMX_OK;

	PVR_LOG_DBG(TEXT("++++++ [PVR] %s Enter +++++++\r\n"), DMX_FUNC_NAME);

	mrRet = _PVR_Init(eInputType);

	if (DMX_FAILED(mrRet)) {
		PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in _DMX_Init(InputType: %d), mrRet: 0x%08x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, eInputType, mrRet);
		MM_RETURN(mrRet);
	}

	PVR_LOG_DBG(TEXT("------ [PVR] %s Exit ------\r\n"), DMX_FUNC_NAME);

	MM_RETURN(mrRet);
}


void DMX_HAL_Uninit(void)
{
	PVR_LOG_DBG(TEXT("++++++ [PVR] %s Enter +++++++\r\n"), DMX_FUNC_NAME);

	_PVR_Uninit();

	PVR_LOG_DBG(TEXT("------ [PVR] %s Exit ------\r\n"), DMX_FUNC_NAME);
}

MRESULT DMX_HAL_DMXConfig(DMX_CMDCFG_INFO_T *prDMXCfgInfo)
{
	PVR_PIDCFG_INFO_T rPIDCfgInfo;
	u32 arHdrDectAddr[5];
	MRESULT mrRet = RET_DMX_OK;

	PVR_LOG_DBG(TEXT("++++++ [PVR] %s Enter +++++++\r\n"), DMX_FUNC_NAME);

	if (NULL == prDMXCfgInfo) {
		DMXLOG_ERROR(TEXT("[PVR] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (prDMXCfgInfo->rPIDInfo.u4PIDIdx > PVR_NUM_PID_INDEX) {
		DMXLOG_ERROR(TEXT("[PVR] %s line %d fail for invalid PID Index(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prDMXCfgInfo->rPIDInfo.u4PIDIdx);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (PVR_IN_PLAYBACK_MM != _PVR_GetInputType()) {
		DMXLOG_ERROR(TEXT("[PVR] %s line %d Demuxer doesn't support the input type: %d\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, _PVR_GetInputType());
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	/* For HW Command Queue*/
	if (PVR_MAX_MM_COM_Q_ITEM_NUM < prDMXCfgInfo->rPTXCmdInfo.u1CmdNum) {
		DMXLOG_ERROR(TEXT("[PVR] %s line %d -- CmdNum = %d over limitation(%d)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prDMXCfgInfo->rPTXCmdInfo.u1CmdNum, PVR_MAX_MM_COM_Q_ITEM_NUM);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}

	if ((1 < prDMXCfgInfo->rPTXCmdInfo.u1CmdNum) &&
		(NULL == prDMXCfgInfo->rPTXCmdInfo.prEntry)) {
		DMXLOG_ERROR(TEXT("[PVR] %s line %d, fail for no CmdQ entry, ")
			TEXT("but Cmd Cnt(%d) > 1 ++++++++++++++++\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prDMXCfgInfo->rPTXCmdInfo.u1CmdNum);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (prDMXCfgInfo->u4SrcBufSize > 0xF0000000) {
		DMXLOG_ERROR(TEXT("[PVR] %s line %d, fail for Src Data Size(%d) is too large++++++++++++++++\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prDMXCfgInfo->u4SrcBufSize);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	} else if (prDMXCfgInfo->u4SrcBufSize < 1) {
		DMXLOG_ERROR(TEXT("[PVR] %s line %d, fail for Src Data Size(%d) < 1++++++++++++++++\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prDMXCfgInfo->u4SrcBufSize);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	if (g_fgDmxDmaTwice) {
		DMXLOG_TRACE(TEXT("[PVR] %s line %d -- (twice dma), so disable dmx interrupt\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prDMXCfgInfo->u4SrcBufSize);

		mm_memset(arHdrDectAddr, 0, sizeof(arHdrDectAddr));
		mrRet = DMX_HAL_GetHdrDetStatus(prDMXCfgInfo->rPIDInfo.u4PIDIdx,
			arHdrDectAddr);
		if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[PVR] %s line %d fail in DMX_HAL_GetHdrDetStatus, mrRet: 0x%08x\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(RET_DMX_PARAM_WRONG);
		}
		_PVR_DisableDmxInterrupt();
	}

REPEATDMA:

	/* Set PID Filter Callback*/
	mrRet = _PVR_SetGlobalCBFunc(prDMXCfgInfo->rGlobalFunc.pfnCB,
		prDMXCfgInfo->rGlobalFunc.pvPrivData);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PVR] %s line %d fail in _PVR_SetGlobalCBFunc\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(mrRet);
	}

	mm_memset(&rPIDCfgInfo, 0, sizeof(PVR_PIDCFG_INFO_T));

	/*Set Destination Fifo*/
	rPIDCfgInfo.u4HWPIDIndex = prDMXCfgInfo->rPIDInfo.u4PIDIdx;
	rPIDCfgInfo.ptrDstFifoAddr = prDMXCfgInfo->ptrDstFifoAddr;
	rPIDCfgInfo.u4DstFifoSize = prDMXCfgInfo->u4DstFifoSize;
	rPIDCfgInfo.ptrDstFifoWPtr = prDMXCfgInfo->ptrDstFifoWPtr;
	rPIDCfgInfo.ptrDstFifoRPtr = prDMXCfgInfo->ptrDstFifoRPtr;
	rPIDCfgInfo.ptrDstFifoSPtr = prDMXCfgInfo->ptrDstFifoSPtr;
	mrRet = _PVR_SetTxDstFIFO(&rPIDCfgInfo);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PVR] %s line %d fail in _PVR_SetTxDstFIFO\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMXLOG_ERROR(TEXT("[PVR] %s line %d fail for PID Index(%d) -- u4DstFifo")
			TEXT("(Addr(0x%x), Size(0x%x), WPtr(0x%x), RPtr(0x%x), SPtr(0x%x))\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prDMXCfgInfo->rPIDInfo.u4PIDIdx,
			prDMXCfgInfo->ptrDstFifoAddr,
			prDMXCfgInfo->u4DstFifoSize, prDMXCfgInfo->ptrDstFifoWPtr,
			prDMXCfgInfo->ptrDstFifoRPtr, prDMXCfgInfo->ptrDstFifoSPtr);
		MM_RETURN(mrRet);
	}

	/* Set PID Filter Callback*/
	mrRet = _PVR_SetPIDFilterNtyFunc(prDMXCfgInfo->rPIDInfo.u4PIDIdx,
		prDMXCfgInfo->rPIDInfo.rPidFunc.pfnNotify,
		prDMXCfgInfo->rPIDInfo.rPidFunc.pvPrivData);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[PVR] %s line %d fail in _PVR_SetPIDFilterCBFunc\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(mrRet);
	}

	if (DDI_PVR_DMA_PATH_ID == prDMXCfgInfo->u1DevID) {
		/* Set Bypass PID Index*/
		if (!_PVR_Set_BypassPIDIdx(DMX_DDI_MM_MOVE_TSIDX, prDMXCfgInfo->rPIDInfo.u4PIDIdx, TRUE)) {
			DMXLOG_ERROR(TEXT("[PVR] %s line %d fail in _PVR_Set_BypassPID\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		/* Set Chunk Size*/
		if (!_PVR_Set_PIDChunkSize(prDMXCfgInfo->rPIDInfo.u4PIDIdx, prDMXCfgInfo->u4SrcBufSize)) {
			DMXLOG_ERROR(TEXT("[PVR] %s line %d fail in _PVR_Set_PIDChunkSize\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		/* Set Trigger for FTuP handle transfer data as a new transfer*/
		if (!_PVR_Set_PIDTriggerFlag(prDMXCfgInfo->rPIDInfo.u4PIDIdx, TRUE)) {
			DMXLOG_ERROR(TEXT("[PVR] %s line %d fail in _PVR_Set_PIDTriggerFlag\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		if (!_PVR_Set_MM_InsertBytes(prDMXCfgInfo->rPIDInfo.u4PIDIdx,
			 &(prDMXCfgInfo->rPTXInstBsInfo))) {
			DMXLOG_ERROR(TEXT("[PVR] %s line %d fail in _PVR_Set_MM_InsertBytes\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		/* Configure Command Queue*/
		if (1 < prDMXCfgInfo->rPTXCmdInfo.u1CmdNum) {
			_PVR_Enable_CommandQueue(prDMXCfgInfo->rPIDInfo.u4PIDIdx);
			_PVR_Set_CommandQueue_Pointers(prDMXCfgInfo->rPIDInfo.u4PIDIdx,
				prDMXCfgInfo->rPTXCmdInfo.u1CmdNum);
			if (!_PVR_Fill_CommandQueue_Items(prDMXCfgInfo->rPTXCmdInfo.u1CmdNum,
				prDMXCfgInfo->rPTXCmdInfo.prEntry)) {
				PVR_LOG_ERR(TEXT("++++++ [PVR] %s fail in _PVR_Fill_CommandQueue_Items +++++++\r\n"),
					DMX_FUNC_NAME);
				MM_RETURN(RET_DMX_HW_ERROR);
			}
		} else {
			_PVR_Disable_CommandQueue(prDMXCfgInfo->rPIDInfo.u4PIDIdx);
		}

		if ((PVR_DESC_MODE_AES_ECB == prDMXCfgInfo->rPIDInfo.eDescMode) ||
			(PVR_DESC_MODE_AES_CBC == prDMXCfgInfo->rPIDInfo.eDescMode)) {
			if (!_PVR_Set_MM_PIDDRMMode(prDMXCfgInfo->rPIDInfo.u4PIDIdx,
				&(prDMXCfgInfo->rPIDInfo.rDRMInfo))) {
				DMXLOG_ERROR(TEXT("[PVR] %s line %d fail in ")
					TEXT("_PVR_Set_MM_PIDDRMMode(PidIdx: %d)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prDMXCfgInfo->rPIDInfo.u4PIDIdx);
				MM_RETURN(RET_DMX_HW_ERROR);
			}
		}

		PVR_LOG_DBG((TEXT("[PVR] %s -- ptrSrcBufAddr: 0x%x, ptrSrcEndAddr: 0x%x, ")
			TEXT("ptrSrcBufRPtr: 0x%x, u4SrcBufSize: 0x%x\r\n")),
			DMX_FUNC_NAME, prDMXCfgInfo->ptrSrcBufAddr, prDMXCfgInfo->ptrSrcEndAddr,
			prDMXCfgInfo->ptrSrcBufRPtr, prDMXCfgInfo->u4SrcBufSize);

		#if DMX_CHECK_MEM_VALIBILITY
		if (prDMXCfgInfo->rPIDInfo.u4PIDIdx == PVR_PID_IDX_VIDEO) {
			uintptr_t ptrVidHdrBufPhySa = PID_S_W(prDMXCfgInfo->rPIDInfo.u4PIDIdx, 10);

			if (!DMX_CheckPesHdrMemGuard(ptrVidHdrBufPhySa)) {
				PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for Video Header Buffer")
					TEXT("(SA: 0x%x) has been crupted\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, ptrVidHdrBufPhySa);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_HW_ERROR);
			}
		}
		#endif /* DMX_CHECK_MEM_VALIBILITY*/

		#if DMX_PFM_TEST
		if (PVR_PID_IDX_VIDEO == prDMXCfgInfo->rPIDInfo.u4PIDIdx)
			g_rPsrPfm.rVideo.u8HWStartTxCnt++;
		else if (PVR_PID_IDX_AUDIO == prDMXCfgInfo->rPIDInfo.u4PIDIdx)
			g_rPsrPfm.rAudio.u8HWStartTxCnt++;
		else if (PVR_PID_IDX_SP == prDMXCfgInfo->rPIDInfo.u4PIDIdx)
			g_rPsrPfm.rSP.u8HWStartTxCnt++;
		#endif /* DMX_PFM_TEST*/

		/* Start Transfer*/
		if (!_PVR_DDI_SingleMove(prDMXCfgInfo->ptrSrcBufAddr,
			prDMXCfgInfo->ptrSrcEndAddr, prDMXCfgInfo->ptrSrcBufRPtr,
			prDMXCfgInfo->u4SrcBufSize)) {
			PVR_LOG_ERR(TEXT("++++++ [PVR] %s fail in _PVR_DDI_SingleMove +++++++\r\n"),
				DMX_FUNC_NAME);
			MM_RETURN(RET_DMX_HW_ERROR);
		}
	} else if (MINI_PVR_DMA_PATH_ID == prDMXCfgInfo->u1DevID) {
		PVR_LOG_DBG(TEXT("[PVR] %s line %d valid PID Index(%d) 2.\r\n"),
	            DMX_FUNC_NAME, DMX_LINE_NO, prDMXCfgInfo->rPIDInfo.u4PIDIdx);
        // Set Bypass PID Index
        if (!_PVR_Set_BypassPIDIdx(DMX_FVR_MM_MOVE_TSIDX, prDMXCfgInfo->rPIDInfo.u4PIDIdx, TRUE)) {
            PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in _PVR_Set_BypassPID\r\n"),
                DMX_FUNC_NAME, DMX_LINE_NO);
            MM_RETURN(RET_DMX_HW_ERROR);
        }

        // Set Chunk Size
        if (!_PVR_Set_PIDChunkSize(prDMXCfgInfo->rPIDInfo.u4PIDIdx, prDMXCfgInfo->u4SrcBufSize)) {
            PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in _PVR_Set_PIDChunkSize\r\n"),
                DMX_FUNC_NAME, DMX_LINE_NO);
            MM_RETURN(RET_DMX_HW_ERROR);
        }

        // Set Trigger for FTuP handle transfer data as a new transfer
        if (!_PVR_Set_PIDTriggerFlag(prDMXCfgInfo->rPIDInfo.u4PIDIdx, TRUE)) {
            PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in _PVR_Set_PIDTriggerFlag\r\n"),
                DMX_FUNC_NAME, DMX_LINE_NO);
            MM_RETURN(RET_DMX_HW_ERROR);
        }
        
        if (!_PVR_Set_MM_InsertBytes(prDMXCfgInfo->rPIDInfo.u4PIDIdx,
              &(prDMXCfgInfo->rPTXInstBsInfo))) {
            PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in _PVR_Set_MM_InsertBytes\r\n"),
                DMX_FUNC_NAME, DMX_LINE_NO);
            MM_RETURN(RET_DMX_HW_ERROR);
        }

        // Configure Command Queue
        if (1 < prDMXCfgInfo->rPTXCmdInfo.u1CmdNum) {
            _PVR_Enable_CommandQueue(prDMXCfgInfo->rPIDInfo.u4PIDIdx);
            _PVR_Set_CommandQueue_Pointers(prDMXCfgInfo->rPIDInfo.u4PIDIdx,
                prDMXCfgInfo->rPTXCmdInfo.u1CmdNum);
            if (!_PVR_Fill_CommandQueue_Items(prDMXCfgInfo->rPTXCmdInfo.u1CmdNum,
                prDMXCfgInfo->rPTXCmdInfo.prEntry)) {
                PVR_LOG_ERR(TEXT("++++++ [PVR] %s fail in _PVR_Fill_CommandQueue_Items +++++++\r\n"),
                    DMX_FUNC_NAME);
                MM_RETURN(RET_DMX_HW_ERROR);
            }
        } else {
            _PVR_Disable_CommandQueue(prDMXCfgInfo->rPIDInfo.u4PIDIdx);
        }
		
        if ((PVR_DESC_MODE_AES_ECB == prDMXCfgInfo->rPIDInfo.eDescMode) ||
            (PVR_DESC_MODE_AES_CBC == prDMXCfgInfo->rPIDInfo.eDescMode)) {
            if (!_PVR_Set_MM_PIDDRMMode(prDMXCfgInfo->rPIDInfo.u4PIDIdx,
                &(prDMXCfgInfo->rPIDInfo.rDRMInfo))) {
                PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in _PVR_Set_MM_PIDDRMMode(PidIdx: %d)\r\n"),
                    DMX_FUNC_NAME, DMX_LINE_NO, prDMXCfgInfo->rPIDInfo.u4PIDIdx);
                MM_RETURN(RET_DMX_HW_ERROR);
            }
        }

        PVR_LOG_DBG(TEXT("[PVR] %s -- ePidType:%d, ptrSrcBufAddr: 0x%x, ptrSrcEndAddr: 0x%x, ptrSrcBufRPtr: 0x%x, u4SrcBufSize: 0x%x\r\n"),
            DMX_FUNC_NAME, prDMXCfgInfo->rPIDInfo.ePidType, prDMXCfgInfo->ptrSrcBufAddr, prDMXCfgInfo->ptrSrcEndAddr,
            prDMXCfgInfo->ptrSrcBufRPtr, prDMXCfgInfo->u4SrcBufSize);

        #if DMX_CHECK_MEM_VALIBILITY
        if (prDMXCfgInfo->rPIDInfo.u4PIDIdx == MINI_PVR_PID_IDX_VIDEO) {
            UINT32 ptrVidHdrBufPhySa = PID_S_W(prDMXCfgInfo->rPIDInfo.u4PIDIdx, 10);
            if (!DMX_CheckPesHdrMemGuard(ptrVidHdrBufPhySa)) {
                PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for Video Header Buffer(SA: 0x%x) has been crupted\r\n"),
                    DMX_FUNC_NAME, DMX_LINE_NO, ptrVidHdrBufPhySa);
                DMX_ASSERT(0);
                MM_RETURN(RET_DMX_HW_ERROR);
            }
        }
        #endif // DMX_CHECK_MEM_VALIBILITY
		
        // Start Transfer ---miniPVR
        if (!_PVR_MPP_SingleMove(prDMXCfgInfo->ptrSrcBufAddr,
            prDMXCfgInfo->ptrSrcEndAddr, prDMXCfgInfo->ptrSrcBufRPtr,
            prDMXCfgInfo->u4SrcBufSize)) {
            PVR_LOG_ERR(TEXT("++++++ [PVR] %s fail in _PVR_DDI_SingleMove +++++++\r\n"),
                DMX_FUNC_NAME);
            MM_RETURN(RET_DMX_HW_ERROR);
        }
	} else {
		PVR_LOG_ERR(TEXT("[PVR] %s line %d, fail for u1DevId is invalid.\r\n"),
                DMX_FUNC_NAME, DMX_LINE_NO);
        MM_RETURN(RET_DMX_HW_ERROR);
	}

	if (g_fgDmxDmaTwice) {
        PVR_VIDEO_TYPE_T evideotype = PVR_VIDEO_UNKNOWN;
        PVR_INPUT_TYPE_T eInputType = PVR_IN_NONE;

        DMX_THREAD_DELAY(100);
        PVR_LOG_TRACE(TEXT("[PVR] %s line %d -- (twice dma), end dma self, and do repeat dma now\r\n"),
            DMX_FUNC_NAME, DMX_LINE_NO, prDMXCfgInfo->u4SrcBufSize);
        _PVR_DDI_EndSingleMove();

        evideotype = _PVR_GetVideoType(prDMXCfgInfo->u1DevID);
        eInputType = _PVR_GetInputType();

        _PVR_PowerDown();
        _PVR_PowerOn(eInputType);

        if (evideotype != PVR_VIDEO_UNKNOWN) {
            _PVR_SetVideoType(prDMXCfgInfo->u1DevID, evideotype);
        }
        mrRet = DMX_HAL_SetHdrDetStatus(prDMXCfgInfo->rPIDInfo.u4PIDIdx,
            arHdrDectAddr);
        if (DMX_FAILED(mrRet)) {
            PVR_LOG_ERR(TEXT("[PVR] %s line %d fail in DMX_HAL_SetHdrDetStatus, mrRet: 0x%08x\r\n"),
                DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
            MM_RETURN(RET_DMX_PARAM_WRONG);
        }               
        g_fgDmxDmaTwice = FALSE;
        goto REPEATDMA;
    }

	PVR_LOG_DBG(TEXT("------ [PVR] %s Exit, success ------\r\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DMX_HAL_ProcIntData(void *pvArg)
{
	PVR_INPUT_TYPE_T eCurInputType = _PVR_GetInputType();
	MRESULT mrRet = RET_DMX_OK;

	PVR_LOG_DBG(TEXT("++++++ [PVR] %s Enter +++++++, eCurInputType: %d\r\n"),
		DMX_FUNC_NAME, eCurInputType);

	if (PVR_IN_PLAYBACK_MM == eCurInputType) {
		mrRet = _PVR_MMIntrHandler(pvArg);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[PVR] %s line %d fail in _PVR_MMIntrHandler, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			MM_RETURN(mrRet);
		}
	} else {
		PVR_LOG_ERR(TEXT("[PVR] %s fail for unsupport PVR input type: 0x%x\r\n"),
			DMX_FUNC_NAME, eCurInputType);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_NO_IMPLEMENT);
	}

	PVR_LOG_DBG(TEXT("------ [PVR] %s Exit, success ------\r\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

bool DMX_HAL_SetVideoType(u8 u1DevID, PVR_VIDEO_TYPE_T eVCodecType)
{
	PVR_LOG_DBG(TEXT("------ [PVR] %s enter, eVCodeType: %d ------\r\n"),
		DMX_FUNC_NAME, eVCodecType);

	if (_PVR_GetVideoType(u1DevID) != eVCodecType)
		return _PVR_SetVideoType(u1DevID, eVCodecType);

	PVR_LOG_DBG(TEXT("------ [PVR] %s Exit, eVCodeType: %d ------\r\n"),
		DMX_FUNC_NAME, eVCodecType);

	return TRUE;
}

uintptr_t DMX_HAL_GetFIFOWPtr(u32 u4HWPIDIndex, DMX_HAL_FIFOType eFIFOType)
{
	u32 u4RegOffset;

	PVR_LOG_DBG(TEXT("++++++ [PVR] %s Enter(u4HWPIDIndex: %d, eFIFOType: %d) +++++++\r\n"),
		DMX_FUNC_NAME, u4HWPIDIndex, eFIFOType);

	switch (eFIFOType) {
	case FFType_Normal:
	case FFType_ES:
		u4RegOffset = 8;
		break;

	case FFType_PES:
		u4RegOffset = 13;
		break;

	default:
		u4RegOffset = 8;
		break;
	}

	PVR_LOG_DBG(TEXT("------- [PVR] %s Exit: 0x%08X ------\r\n"),
		DMX_FUNC_NAME, (u32)PID_S_W(u4HWPIDIndex, u4RegOffset));

	return (uintptr_t)PID_S_W(u4HWPIDIndex, u4RegOffset);
}

uintptr_t DMX_HAL_GetFIFORPtr(u32 u4HWPIDIndex, DMX_HAL_FIFOType eFIFOType)
{
	u32 u4RegOffset;

	PVR_LOG_DBG(TEXT("++++++ [PVR] %s Enter(u4HWPIDIndex: %d, eFIFOType: %d) +++++++\r\n"),
		DMX_FUNC_NAME, u4HWPIDIndex, eFIFOType);

	switch (eFIFOType) {
	case FFType_Normal:
	case FFType_ES:
		u4RegOffset = 9;
		break;

	case FFType_PES:
		u4RegOffset = 14;
		break;

	default:
		u4RegOffset = 9;
		break;
	}

	PVR_LOG_DBG(TEXT("------- [PVR] %s Exit --------\r\n"), DMX_FUNC_NAME);

	return (uintptr_t) PID_S_W(u4HWPIDIndex, u4RegOffset);
}

void DMX_HAL_SetFIFOWPtr(u32 u4HWPIDIndex, DMX_HAL_FIFOType eFIFOType, uintptr_t ptrCurWritePtr)
{
	u32 u4RegOffset;

	PVR_LOG_DBG(TEXT("++++++ [PVR] %s Enter(u4HWPIDIndex: %d, eFIFOType: %d) +++++++\r\n"),
		DMX_FUNC_NAME, u4HWPIDIndex, eFIFOType);

	switch (eFIFOType) {
	case FFType_Normal:
	case FFType_ES:
		u4RegOffset = 8;
		break;

	case FFType_PES:
		u4RegOffset = 13;
		break;

	default:
		u4RegOffset = 8;
		break;
	}

	PID_S_W(u4HWPIDIndex, u4RegOffset) = ptrCurWritePtr;
	mb();

	PVR_LOG_DBG(TEXT("------- [PVR] %s Exit --------\r\n"), DMX_FUNC_NAME);
}

void DMX_HAL_SetFIFORPtr(u32 u4HWPIDIndex, DMX_HAL_FIFOType eFIFOType, uintptr_t ptrCurReadPtr)
{
	u32 u4RegOffset;

	PVR_LOG_DBG(TEXT("++++++ [PVR] %s Enter(u4HWPIDIndex: %d, eFIFOType: %d) +++++++\r\n"),
		DMX_FUNC_NAME, u4HWPIDIndex, eFIFOType);

	switch (eFIFOType) {
	case FFType_Normal:
	case FFType_ES:
		u4RegOffset = 9;
		break;

	case FFType_PES:
		u4RegOffset = 14;
		break;

	default:
		u4RegOffset = 9;
		break;
	}

	PID_S_W(u4HWPIDIndex, u4RegOffset) = ptrCurReadPtr;
	mb();

	PVR_LOG_DBG(TEXT("------- [PVR] %s Exit --------\r\n"), DMX_FUNC_NAME);
}

MRESULT DMX_HAL_SetHdrDetStatus(u32 u4HWPIDIndex, u32 *pu4HdrBufInfo)
{
	u32 u4SectionFilter;

	PVR_LOG_DBG(TEXT("++++++ [PVR] %s Enter +++++++\r\n"), DMX_FUNC_NAME);

	if (PVR_NUM_PID_INDEX <= u4HWPIDIndex) {
		PVR_LOG_ERR(TEXT("[PVR] %s failed for invalid args(u4HWPIDIndex: %d)\r\n"),
			DMX_FUNC_NAME, u4HWPIDIndex);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (NULL != pu4HdrBufInfo) {
		#if DMX_CHECK_MEM_VALIBILITY
		u32 u4VidHdrBufSa = 0;
		u32 u4VidHdrBufEa = 0;
		u32 u4VidHdrBufWp = 0;
		u32 u4VidHdrBufRp = 0;
		u32 *pu4VidHdrBufWp = NULL;
		u32 u4Idx = 0;

		if (!DMX_CheckPesHdrMemGuard(pu4HdrBufInfo[0])) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for Video Header Buffer")
				TEXT("(SA: 0x%x) has been crupted\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pu4HdrBufInfo[0]);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		if (pu4HdrBufInfo[1] <= pu4HdrBufInfo[0]) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for Video Header Buffer's ")
				TEXT("EA(0x%x) <= SA(0x%x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pu4HdrBufInfo[1], pu4HdrBufInfo[0]);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		if ((pu4HdrBufInfo[2] < pu4HdrBufInfo[0]) ||
			(pu4HdrBufInfo[1] < pu4HdrBufInfo[2])) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for Video Header Buffer's ")
				TEXT("(WP(0x%x) < SA(0x%x) || WP(0x%x) > EA(0x%x))\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				pu4HdrBufInfo[2], pu4HdrBufInfo[0],
				pu4HdrBufInfo[2], pu4HdrBufInfo[1]);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		if ((pu4HdrBufInfo[3] < pu4HdrBufInfo[0]) ||
			(pu4HdrBufInfo[1] < pu4HdrBufInfo[3])) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for Video Header Buffer's ")
				TEXT("(RP(0x%x) < SA(0x%x) || RP(0x%x) > EA(0x%x))\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				pu4HdrBufInfo[3], pu4HdrBufInfo[0],
				pu4HdrBufInfo[3], pu4HdrBufInfo[1]);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		u4VidHdrBufSa = DMX_NONCACHE(pu4HdrBufInfo[0]);
		u4VidHdrBufEa = DMX_NONCACHE(pu4HdrBufInfo[1]);
		u4VidHdrBufWp = DMX_NONCACHE(pu4HdrBufInfo[2]);
		u4VidHdrBufRp = DMX_NONCACHE(pu4HdrBufInfo[3]);

		if (DMX_EMPTYSIZE(u4VidHdrBufRp, u4VidHdrBufWp, (u4VidHdrBufEa - u4VidHdrBufSa)) < 3 * sizeof(u32)) {
			PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for Video Header Buffer Full,")
				TEXT(" (RP(0x%x), WP(0x%x), SA(0x%x), EA(0x%x))\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				pu4HdrBufInfo[3], pu4HdrBufInfo[2],
				pu4HdrBufInfo[0], pu4HdrBufInfo[1]);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_HW_ERROR);
		}

		for (u4Idx = 0; u4Idx < 3; u4Idx++) {
			if (u4VidHdrBufWp >= u4VidHdrBufEa + 1)
				u4VidHdrBufWp -= (u4VidHdrBufEa + 1 - u4VidHdrBufSa);

			if ((u4VidHdrBufWp >= u4VidHdrBufEa + 1) ||
				(u4VidHdrBufWp < u4VidHdrBufSa)) {
				PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for Video Header Buffer's")
					TEXT(" (WP(0x%x) < SA(0x%x) || WP(0x%x) > EA(0x%x))\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					pu4HdrBufInfo[2], pu4HdrBufInfo[0],
					pu4HdrBufInfo[2], pu4HdrBufInfo[1]);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_HW_ERROR);
			}
			pu4VidHdrBufWp = (u32 *)u4VidHdrBufWp;
			*pu4VidHdrBufWp = 0;
			u4VidHdrBufWp += sizeof(u32);
		}
		#endif /* DMX_CHECK_MEM_VALIBILITY*/

		PID_S_W(u4HWPIDIndex, PID_DATA_STRUCT_HEADER_SA) =
			(pu4HdrBufInfo[0]) ? (pu4HdrBufInfo[0]) : 0; /* SA*/
		PID_S_W(u4HWPIDIndex, PID_DATA_STRUCT_HEADER_EA) =
			(pu4HdrBufInfo[1]) ? (pu4HdrBufInfo[1]) : 0; /* EA*/
		PID_S_W(u4HWPIDIndex, PID_DATA_STRUCT_HEADER_CURRENT_SA) =
			(pu4HdrBufInfo[4]) ? (pu4HdrBufInfo[4]) : 0; /* Start Ptr*/
		PID_S_W(u4HWPIDIndex, PID_DATA_STRUCT_HEADER_WP) =
			(pu4HdrBufInfo[2]) ? (pu4HdrBufInfo[2]) : 0; /* Write Ptr*/
		PID_S_W(u4HWPIDIndex, PID_DATA_STRUCT_HEADER_RP) =
			(pu4HdrBufInfo[3]) ? (pu4HdrBufInfo[3]) : 0; /* Read Ptr*/

		mb();

		u4SectionFilter = PID_S_W(u4HWPIDIndex, 0);
		switch (u4HWPIDIndex) {
		case PVR_PID_IDX_VIDEO:
			PID_S_W(u4HWPIDIndex, 0) = u4SectionFilter | (1 << 6);
			if ((0 == pu4HdrBufInfo[0]) ||
				(0 == pu4HdrBufInfo[1]) ||
				(0 == pu4HdrBufInfo[2]) ||
				(0 == pu4HdrBufInfo[3])) {
				PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for invalid Hdr Buf")
					TEXT("(SA(0x%x), EA(0x%x), WP(0x%x), RP(0x%x)), u4HWPIDIndex=%d\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, pu4HdrBufInfo[0],
					pu4HdrBufInfo[1], pu4HdrBufInfo[2], pu4HdrBufInfo[3],
					u4HWPIDIndex);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_HW_ERROR);
			}
			break;
		case PVR_PID_IDX_AUDIO:
			u4SectionFilter &= (~(1 << 6));
			PID_S_W(u4HWPIDIndex, 0) = u4SectionFilter & (~(1 << 7));
			break;
		case PVR_PID_IDX_SP:
		case PVR_PID_IDX_SECTION:
			u4SectionFilter &= (~(1 << 6));
			PID_S_W(u4HWPIDIndex, 0) = u4SectionFilter & (~(1 << 7));
			break;
		default:
			break;
		}
		mb();
	} else {
		PID_S_W(u4HWPIDIndex, PID_DATA_STRUCT_HEADER_SA) = 0; /* SA*/
		PID_S_W(u4HWPIDIndex, PID_DATA_STRUCT_HEADER_EA) = 0; /* EA*/
		PID_S_W(u4HWPIDIndex, PID_DATA_STRUCT_HEADER_CURRENT_SA) = 0; /* Start Ptr*/
		PID_S_W(u4HWPIDIndex, PID_DATA_STRUCT_HEADER_WP) = 0; /* Write Ptr*/
		PID_S_W(u4HWPIDIndex, PID_DATA_STRUCT_HEADER_RP) = 0; /* Read Ptr*/

		mb();

		u4SectionFilter = PID_S_W(u4HWPIDIndex, 0);
		switch (u4HWPIDIndex) {
		case PVR_PID_IDX_VIDEO:
			PID_S_W(u4HWPIDIndex, 0) = u4SectionFilter | (1 << 6);
			PVR_LOG_ERR(TEXT("[PVR] %s line %d fail for no pu4HdrBufInfo should ")
				TEXT("not be NULL, PidIdx: %d\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4HWPIDIndex);
			MM_RETURN(RET_DMX_HW_ERROR);
			break;
		case PVR_PID_IDX_AUDIO:
			u4SectionFilter &= (~(1 << 6));
			PID_S_W(u4HWPIDIndex, 0) = u4SectionFilter & (~(1 << 7));
			PVR_LOG_ERR(TEXT("[PVR] %s line %d fail for no pu4HdrBufInfo should ")
			TEXT("not be NULL, PidIdx: %d\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4HWPIDIndex);
			MM_RETURN(RET_DMX_HW_ERROR);
			break;
		case PVR_PID_IDX_SP:
		case PVR_PID_IDX_SECTION:
			u4SectionFilter &= (~(1 << 6));
			PID_S_W(u4HWPIDIndex, 0) = u4SectionFilter & (~(1 << 7));
			break;
		default:
			break;
		}
		mb();
	}

	PVR_LOG_DBG(TEXT("------- [PVR] %s Exit --------\r\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

MRESULT DMX_HAL_GetHdrDetStatus(u32 u4HWPIDIndex, u32 *pu4HdrBufInfo)
{
	PVR_LOG_DBG(TEXT("++++++ [PVR] %s Enter(u4HWPIDIndex: %d) +++++++\r\n"),
		DMX_FUNC_NAME, u4HWPIDIndex);

	if (NULL == pu4HdrBufInfo) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	pu4HdrBufInfo[0] = PID_S_W(u4HWPIDIndex, PID_DATA_STRUCT_HEADER_SA);	/*SA*/
	pu4HdrBufInfo[1] = PID_S_W(u4HWPIDIndex, PID_DATA_STRUCT_HEADER_EA);	/*EA*/
	pu4HdrBufInfo[2] = PID_S_W(u4HWPIDIndex, PID_DATA_STRUCT_HEADER_WP);	/*Write Ptr*/
	pu4HdrBufInfo[3] = PID_S_W(u4HWPIDIndex, PID_DATA_STRUCT_HEADER_RP);	/*Read Ptr*/
	pu4HdrBufInfo[4] = PID_S_W(u4HWPIDIndex, PID_DATA_STRUCT_HEADER_CURRENT_SA);	/*Start Ptr*/

	#if DMX_CHECK_MEM_VALIBILITY
	if (!DMX_CheckPesHdrMemGuard(pu4HdrBufInfo[0])) {
		PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for Video Header Buffer")
			TEXT("(SA: 0x%x) has been crupted\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pu4HdrBufInfo[0]);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_HW_ERROR);
	}

	if (pu4HdrBufInfo[1] <= pu4HdrBufInfo[0]) {
		PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for Video Header Buffer's EA")
			TEXT("(0x%x) <= SA(0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pu4HdrBufInfo[1], pu4HdrBufInfo[0]);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_HW_ERROR);
	}

	if ((pu4HdrBufInfo[2] < pu4HdrBufInfo[0]) ||
		(pu4HdrBufInfo[1] <= pu4HdrBufInfo[2])) {
		PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for Video Header Buffer's ")
			TEXT("(WP(0x%x) < SA(0x%x) || WP(0x%x) >= EA(0x%x))\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			pu4HdrBufInfo[2], pu4HdrBufInfo[0],
			pu4HdrBufInfo[2], pu4HdrBufInfo[1]);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_HW_ERROR);
	}

	if ((pu4HdrBufInfo[3] < pu4HdrBufInfo[0]) ||
		(pu4HdrBufInfo[1] <= pu4HdrBufInfo[3])) {
		PVR_LOG_ERR(TEXT("[PVR] %s line %d failed for Video Header Buffer's ")
			TEXT("(RP(0x%x) < SA(0x%x) || RP(0x%x) >= EA(0x%x))\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			pu4HdrBufInfo[3], pu4HdrBufInfo[0],
			pu4HdrBufInfo[3], pu4HdrBufInfo[1]);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_HW_ERROR);
	}
	#endif /* DMX_CHECK_MEM_VALIBILITY*/

	switch (u4HWPIDIndex) {
	case PVR_PID_IDX_VIDEO:
		/* For parse header easily, we need keep the header WP in sense position*/
		/* Header Buffer need keep more than 4*4 bytes for parse it.*/
		/* The threshold will effect the max header count in one transfer.*/
		/* MAX_HEADER_COUNT = (PVR_FTUP_VID_DMA_THRESHOLD / 4) - 3*/
		if ((pu4HdrBufInfo[1] - pu4HdrBufInfo[2]) <= PVR_FTUP_VID_DMA_THRESHOLD) /*EA - WP*/ {
			/* Reset WP to SA*/
			pu4HdrBufInfo[2] = pu4HdrBufInfo[0];
			pu4HdrBufInfo[4] = pu4HdrBufInfo[0];
		}
		break;
	case PVR_PID_IDX_AUDIO:
		/* For parse header easily, we need keep the header WP in sense position*/
		/* Header Buffer need keep more than 4*4 bytes for parse it.*/
		/* The threshold will effect the max header count in one transfer.*/
		/* MAX_HEADER_COUNT = (PVR_FTUP_VID_DMA_THRESHOLD / 4) - 3*/
		if ((pu4HdrBufInfo[1] - pu4HdrBufInfo[2]) <= PVR_FTUP_OTH_DMA_THRESHOLD) /*EA - WP*/ {
			/* Reset WP to SA*/
			pu4HdrBufInfo[2] = pu4HdrBufInfo[0];
			pu4HdrBufInfo[4] = pu4HdrBufInfo[0];
		}
		break;
	case PVR_PID_IDX_SP:
	case PVR_PID_IDX_SECTION:
		/* For parse header easily, we need keep the header WP in sense position*/
		/* Header Buffer need keep more than 4*4 bytes for parse it.*/
		/* The threshold will effect the max header count in one transfer.*/
		/* MAX_HEADER_COUNT = (PVR_FTUP_VID_DMA_THRESHOLD / 4) - 3*/
		if ((pu4HdrBufInfo[1] - pu4HdrBufInfo[2]) <= PVR_FTUP_OTH_DMA_THRESHOLD) /*EA - WP*/ {
			/* Reset WP to SA*/
			pu4HdrBufInfo[2] = pu4HdrBufInfo[0];
			pu4HdrBufInfo[4] = pu4HdrBufInfo[0];
		}
		break;
	default:
		PVR_LOG_ERR(TEXT("[PVR] %s line %d fail for u4HWPIDIndex: %d\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4HWPIDIndex);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PVR_LOG_DBG(TEXT("------- [PVR] %s Exit --------\r\n"), DMX_FUNC_NAME);
	MM_RETURN(RET_DMX_OK);
}


bool DMX_HAL_DumpStartCodes(void)
{
	bool fgRet = FALSE;

	PVR_LOG_DBG(TEXT("++++++ [PVR] %s Enter +++++++\r\n"), DMX_FUNC_NAME);
	fgRet = _PVR_DumpStartCodePattern_Ex();
	PVR_LOG_DBG(TEXT("------ [PVR] %s Exit ------\r\n"), DMX_FUNC_NAME);
	return fgRet;
}

bool DMX_HAL_DumpPidStruct(u8 u1Pidx)
{
	bool fgRet = FALSE;

	PVR_LOG_DBG(TEXT("++++++ [PVR] %s Enter +++++++\r\n"), DMX_FUNC_NAME);
	fgRet = _PVR_DumpPIDDataStruct(u1Pidx);
	PVR_LOG_DBG(TEXT("------ [PVR] %s Exit ------\r\n"), DMX_FUNC_NAME);
	return fgRet;
}

bool DMX_HAL_DumpDDIInfo(void)
{
	bool fgRet = FALSE;

	PVR_LOG_DBG(TEXT("++++++ [PVR] %s Enter +++++++\r\n"), DMX_FUNC_NAME);
	fgRet = _PVR_DDI_DumpInfo();
	PVR_LOG_DBG(TEXT("------ [PVR] %s Exit ------\r\n"), DMX_FUNC_NAME);
	return fgRet;
}

void DMX_HAL_PVR_DumpRegisters(u32 u41stRegAddr, u32 u4RegsCnt)
{
	PVR_LOG_DBG(TEXT("++++++ [PVR] %s Enter +++++++\r\n"), DMX_FUNC_NAME);
	_PVR_DumpRegisters(u41stRegAddr, u4RegsCnt);
	PVR_LOG_DBG(TEXT("------ [PVR] %s Exit ------\r\n"), DMX_FUNC_NAME);
}

void DMX_HAL_PVR_DumpLocalarbiter(void)
{
    _PVR_DumpDramLocalArbiter(4);
}

void DMX_HAL_PVR_DumpDramKeyRegs(bool fg1stClear)
{
    _PVR_DumpDramKeyRegs(fg1stClear);
}

bool DMX_HAL_SetPowerState(DMX_PM_STATE ePowerState, PVR_INPUT_TYPE_T eInputType)
{
	return _PVR_SetPowerState(ePowerState, eInputType);
}

DMX_PM_STATE DMX_HAL_GetPowerState(void)
{
	return _PVR_GetPowerState();
}


