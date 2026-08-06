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
 */

/*!
 * @file dmx_spt_psr.c
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
 */
#ifdef __linux__
#include <linux/mm.h>
#include <linux/errno.h>
#include "windows.h"
#include "drv_def.h"
#include <media/atc/dmx_define.h>
#include <media/atc/dmx_cfa_def.h>
#if CONFIG_DRV_HDMI_RX
#include <media/atc/x_audin.h>
#endif /* CONFIG_DRV_HDMI_RX*/
#include <media/atc/drv_aud.h>
/* #include <media/atc/mm_debug.h> */

#else /* __linux__*/
#include "drv_def.h"
#include "dmx_define.h"
#include "dmx_cfa_def.h"
#if CONFIG_DRV_HDMI_RX
#include "x_audin.h"
#endif /* CONFIG_DRV_HDMI_RX*/
#include "drv_aud.h"
#include "mm_debug.h"
#endif /* __linux__*/

#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_spt.h"
#include "dmx_spt_psr.h"
#include "dmx_spt_rsp.h"
#include "dmx_spt_main.h"
#include "dmx_spt_os.h"
#include "dmx_spt_util.h"
#include "dmx_stream.h"
#include "dmx_parser.h"
#include "dmx_psr_cc.h"
#include "dmx_psr_filter.h"
#include "dmx_psr_esm.h"
#include "dmx_psr_pbbuf.h"
#include "dmx_gau_if.h"
#include "dmx_esm_if.h"
#include "dmx_cpsa.h"
#include "dmx_pfm.h"

#ifndef __linux__
/*#pragma warning(disable: 6011)   disable	warning C6011: Dereferencing NULL pointer checking*/
#pragma warning(disable : 4127) /*disable warning C4127: conditional expression is constant*/
#endif

#if DMX_PFM_TEST
EXTERN PSR_PFM g_rPsrPfm;
#endif /* DMX_PFM_TEST*/

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterSetPsrAuTable*/
/* If do resplitter, set resplitter AU info*/
/* If not, Fill AU Table Info, and Resplitter do log Au info*/
/* @Param pvEventData [in] the handle of handle Parser AU*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterSetPsrAuTable(void *pvSptHdl, void *pvEventData)
{
	PSR_AU	*prAuData = (PSR_AU *) pvEventData;
	MRESULT mrRet =  RET_DMX_OK;

	if (NULL == prAuData) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args.\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (SplitterRspIsEnabled(pvSptHdl) &&
		SplitterRspIsRsping(pvSptHdl) &&
		(SplitterGetRspTxType(pvSptHdl) == (u8)(prAuData->eType)))  /*only allow rsp data can enter.*/{
		mrRet = SplitterRspSetRspAu(pvSptHdl, prAuData->pvAUInf);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s fail in Cfa set AUInfo, mrRet: 0x%x.\r\n"),
				DMX_FUNC_NAME, mrRet);
			MM_RETURN(mrRet);
		}
		MM_RETURN(mrRet);
	}

	/* open in the future, when all CFA implemented*/
	mrRet = SptCfaSetAUInfo(pvSptHdl, prAuData->pvAUInf, prAuData->pvAUExtInf);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail in Cfa set AUInfo, mrRet: 0x%x.\r\n"),
			DMX_FUNC_NAME, mrRet);
		MM_RETURN(mrRet);
	}

	mrRet = SplitterRspSetLogAu(pvSptHdl, prAuData->pvAUInf);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_DEBUG(TEXT("[SPT] %s fail in Rsp Log AU, mrRet: 0x%x.\r\n"),
			DMX_FUNC_NAME, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

#if 0
/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterFifoIsThreshold*/
/* Get the Parser Filter of the designated stream*/
/* Get the fifo avaliable data size of the parser filter*/
/* Get the stream's fifo threshold*/
/* Compare them, if avail > threshold, return TRUE, otherwise, FALSE*/
/*/////////////////////////////////////////////////////////////////////////////*/
static bool SplitterFifoIsThreshold(uintptr_t pvStm)
{
	MRESULT mrRet = RET_DMX_OK;
	u32	u4FifoDataSize = 0;
	u32	u4FifoTh = DMX_INVALID_UINT32;

	if (!StreamIsEnabled(pvStm))
		return FALSE;

	/* Check fifo threshold if arrived.*/
	mrRet = PSR_Filter_GetFifoAvailSize(((DMX_STM_INST_T *)pvStm)->pvPsrFtr, &u4FifoDataSize);
	if (DMX_FAILED(mrRet))
		return FALSE;

	u4FifoTh = StreamGetFifoThreshold(pvStm);
	if (0 == u4FifoTh)
		return TRUE;
	else if (DMX_INVALID_UINT32 == u4FifoTh)
		return FALSE;
	return ((u4FifoDataSize >= u4FifoTh) ? TRUE : FALSE);
}
#endif

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterPbb2Buf*/
/* Sync PBBUF*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterPbb2Buf(void *pvSptHdl, DMAInfo *prDMA)
{
	u64		u8CurPbbufStartOffset = 0;
	DMX_SPT_DMA2FIFO_INFO_T rInf = {0};
	void		*pvStm = NULL;
	MRESULT		mrRet = RET_DMX_OK;

	if ((NULL == pvSptHdl) || (NULL == prDMA))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	/* Get the	file offst of Current PBBuf slot 's SA's corresponding*/
	mrRet = PSR_CC_GetCurPbbufStartOffset(((DMX_SPT_INST_T *)pvSptHdl)->pvPsrCC,
									   &u8CurPbbufStartOffset);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail in PSR_CC_GetCurPbbufStartOffset!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(mrRet);
	}

	if (prDMA->u8FromFileOfst < u8CurPbbufStartOffset) /* ERROR, This should not happens*/{
		DMXLOG_ERROR(TEXT("[SPT] %s line %d ERROR -- u8FromFileOfst:")
			TEXT(" 0x%08x%08x, u8CurPbbufStartOffset: 0x%08x%08x!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			(u32)(prDMA->u8FromFileOfst >> 32), (u32)(prDMA->u8FromFileOfst),
			(u32)(u8CurPbbufStartOffset >> 32), (u32)(u8CurPbbufStartOffset));
		PSR_CC_AbortTx(((DMX_SPT_INST_T *)pvSptHdl)->pvPsrCC);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- u8FromFileOfst: 0x%08x%08x,")
		TEXT(" u8CurPbbufStartOffset: 0x%08x%08x!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		(u32)(prDMA->u8FromFileOfst >> 32), (u32)(prDMA->u8FromFileOfst),
		(u32)(u8CurPbbufStartOffset >> 32), (u32)(u8CurPbbufStartOffset));

	rInf.u8FromFileOfst = prDMA->u8FromFileOfst;
	rInf.u8TxLen		= prDMA->u8TxLen;
	rInf.u4TxStreamType = SPT_DATA_BUF;
	rInf.pvFromAddress	= NULL;
	rInf.pvToAddress	= prDMA->pucToAddress;
	rInf.u4TxVideoCodec = DMX_INVALID_UINT32;
	rInf.u4TxPictureMode = DMX_INVALID_UINT32;
	rInf.u8PtsSa		= INVALID_TIMESTAMP;
	rInf.u8PtsEa		= INVALID_TIMESTAMP;

	pvStm = GetStreamByType(pvSptHdl, SPT_DATA_BUF);

	/* Set the TX info into splitter*/
	mrRet = SplitterSetPtxData(pvSptHdl, &rInf, pvStm, TRUE);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail in SplitterSetPtxData (pvSptHdl")
			TEXT(" 0x%p, mrRet: 0x%x)!\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	if (NULL == pvStm) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail in Get DMA Stream!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_NOT_FOUND);
	}

	mrRet = PSR_Filter_DMAPBBuf4HdrParsing(((DMX_STM_INST_T *)pvStm)->pvPsrFtr,
								   prDMA->u8FromFileOfst,
								   (u32)prDMA->u8TxLen,
								   (uintptr_t *)prDMA->pucToAddress,
								   prDMA->pu4AvailSize);

	if (DMX_FAILED(mrRet)) {
		if (!MM_IS_STATE_ERROR(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s fail Spth 0x%p in DMAPBBuf, ")
				TEXT("prDMA->fgSync: %d\r\n"),
				DMX_FUNC_NAME, pvSptHdl, (prDMA->fgSync ? 1 : 0));
		}
		MM_RETURN(mrRet);
	}

	mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterChangeState")
			TEXT("(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterBuf2Fifo*/
/* Tx Memory data into Fifo*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterBuf2Fifo(void *pvSptHdl, DMX_SPT_DMA2FIFO_INFO_T *prInf)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	MRESULT mrRet = RET_DMX_OK;
	void	*pvStm  = NULL;

	if ((NULL == prSpt) || (NULL == prInf))
		MM_RETURN(RET_DMX_PARAM_WRONG);

#if DMX_DISABLE_DMA_DATA
	DMX_THREAD_DELAY(1);
	prSpt->u8PtxLen = prInf->u8TxLen;
	SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
	mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterChangeState")
			TEXT("(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}
	MM_RETURN(RET_DMX_OK);
#endif

	#if DMX_DISABLE_VID_DMA
	if (SPT_DATA_V == prInf->u4TxStreamType) {
		DMX_THREAD_DELAY(1);
		prSpt->u8PtxLen = prInf->u8TxLen;
		SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
		mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterChangeState")
				TEXT("(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
			MM_RETURN(mrRet);
		}
		MM_RETURN(RET_DMX_OK);
	}
	#endif /* DMX_DISABLE_VID_DMA*/

	#if DMX_DISABLE_AUD_DMA
	if (SPT_DATA_A == prInf->u4TxStreamType) {
		DMX_THREAD_DELAY(1);
		prSpt->u8PtxLen = prInf->u8TxLen;
		SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
		mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterChangeState")
				TEXT("(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
			MM_RETURN(mrRet);
		}
		MM_RETURN(RET_DMX_OK);
	}
	#endif /* DMX_DISABLE_AUD_DMA*/

	if ((SPT_DATA_A == prInf->u4TxStreamType) &&
		(!SplitterIsEnableDmaAud(pvSptHdl))) {
		prSpt->u8PtxLen = prInf->u8TxLen;
		SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
		mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterChangeState")
				TEXT("(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
			MM_RETURN(mrRet);
		}
		MM_RETURN(RET_DMX_OK);
	}

	pvStm = GetStreamByType(pvSptHdl, prInf->u4TxStreamType);

	mrRet = SplitterSetPtxData(pvSptHdl, prInf, pvStm, FALSE);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterSetPtxData")
			TEXT(" (pvSptHdl 0x%p, mrRet: 0x%x)!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	if (!SplitterRspIsEnabled(prSpt)) {
		if ((SPT_DATA_A == prInf->u4TxStreamType) ||
			(SPT_DATA_SP == prInf->u4TxStreamType)) {
			u32 u4StmUID = GetStmUIDByType(pvSptHdl, prInf->u4TxStreamType);

			if (prInf->u4TxUID != u4StmUID) /* not the stream used in current playing*/ {
				DMXLOG_DEBUG(
					TEXT("[SPT] %s -- prInf->u4TxUID(0x%x) != u4StmUID(0x%x)\r\n"),
					DMX_FUNC_NAME, prInf->u4TxUID, u4StmUID);
				/* Fake Transfer*/
				/* We Do not real issue Parser Tx in this case*/
				SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
				mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in ")
						TEXT("SplitterChangeState(RUNING, TXING), pvSptHdl: 0x%p,")
						TEXT(" mrRet: 0x%x\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
					MM_RETURN(mrRet);
				}
				MM_RETURN(RET_DMX_OK);
			}
		}
	}

	/* Audio adts header installer*/
	if (SplitterRspIsLoging(pvSptHdl)) {
#if DMX_RSP_SUPPORT_PURE_AUDIO
		if (SPT_DATA_A == prInf->u4TxStreamType) {
			SplitterSetPtxNotBusy(pvSptHdl);

			mrRet = SplitterSetPtxData(pvSptHdl, prInf, pvStm, TRUE);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterSetPtxData")
					TEXT(" (pvSptHdl 0x%p, mrRet: 0x%x)!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
				MM_RETURN(mrRet);
			}
		}
#else
		if ((SPT_DATA_A == prInf->u4TxStreamType) &&
			(NULL != GetStreamByTypes(prSpt->pvDmxInst, SPT_DATA_V))) {
			SplitterSetPtxNotBusy(pvSptHdl);

			mrRet = SplitterSetPtxData(pvSptHdl, prInf, pvStm, TRUE);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterSetPtxData")
					TEXT(" (pvSptHdl 0x%p, mrRet: 0x%x)!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
				MM_RETURN(mrRet);
			}
		}
#endif
		if ((SPT_DATA_A == prInf->u4TxStreamType) ||
			(SPT_DATA_SP == prInf->u4TxStreamType)) {
			u32 u4StmUID = GetStmUIDByType(pvSptHdl, prInf->u4TxStreamType);

			if (prInf->u4TxUID != u4StmUID) /* not the stream used in current playing*/ {
				DMXLOG_DEBUG(
					TEXT("[SPT] %s -- prInf->u4TxUID(0x%x) != u4StmUID(0x%x)\r\n"),
					DMX_FUNC_NAME, prInf->u4TxUID, u4StmUID);
				/* Fake Transfer*/
				/*We Do not real issue Parser Tx in this case*/
				SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
				mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in ")
						TEXT("SplitterChangeState(RUNING, TXING), pvSptHdl: 0x%p, ")
						TEXT("mrRet: 0x%x\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
					MM_RETURN(mrRet);
				}
				MM_RETURN(RET_DMX_OK);
			}
		}
	}

	if ((SPT_DATA_A == prInf->u4TxStreamType) &&
		(!SplitterIsEnableDmaAud(pvSptHdl))) {
		prSpt->u8PtxLen = prInf->u8TxLen;
		SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
		mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterChangeState")
				TEXT("(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
			MM_RETURN(mrRet);
		}
		MM_RETURN(RET_DMX_OK);
	}

	/*fix Klocwork*/
	if (NULL == pvStm) {
		/*We Do not real issue Parser Tx in this case*/
		DMXLOG_DEBUG(TEXT("[SPT] %s exit for pvSptHdl(0x%p) has no stream")
			TEXT(" whose type is %d!\r\n"),
			DMX_FUNC_NAME, pvSptHdl, prInf->u4TxStreamType);

		/*DMX_ASSERT(FALSE);*/
		SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
		mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterChangeState")
				TEXT("(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
			MM_RETURN(mrRet);
		}
		MM_RETURN(RET_DMX_OK);
	}

	if (SPT_DATA_V == prInf->u4TxStreamType) {
		mrRet = PSR_Filter_SetCodeC(((DMX_STM_INST_T *)pvStm)->pvPsrFtr,
									 prInf->u4TxVideoCodec);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s fail in PSR_Filter_SetCodeC ")
				TEXT("(pvSptHdl 0x%p, Codec %d), mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, pvSptHdl, prInf->u4TxVideoCodec, mrRet);
			MM_RETURN(mrRet);
		}
	} else if (SPT_DATA_A == prInf->u4TxStreamType) {
		mrRet = PSR_Filter_SetCodeC(((DMX_STM_INST_T *)pvStm)->pvPsrFtr,
									 prInf->u4TxAudioCodec);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s fail in PSR_Filter_SetCodeC ")
				TEXT("(pvSptHdl 0x%p, AudioCodec %d), mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, pvSptHdl, prInf->u4TxAudioCodec, mrRet);
			MM_RETURN(mrRet);
		}
	}

	mrRet = PSR_Filter_TxMemory2Fifo(((DMX_STM_INST_T *)pvStm)->pvPsrFtr,
									  prInf->pvFromAddress, (u32)prInf->u8TxLen);

	if (DMX_FAILED(mrRet)) {
		if (!MM_IS_STATE_ERROR(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s fail in PSR_Filter_TxMemory2Fifo")
				TEXT(" (pvSptHdl 0x%p, mrRet: 0x%x)!\r\n"),
				DMX_FUNC_NAME, pvSptHdl, mrRet);
		}
		MM_RETURN(mrRet);
	}

	mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterChangeState")
			TEXT("(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);

}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterBuf2FifoAUCtrl*/
/* Tx PBBUF data into Fifo, and compose one AU*/
/* 1) If prInf->fgCreateAU is TRUE, this means we need to create AU when the u8CurAULen >= */
/*u8TotalAULen, i.e. Use Total AU Len as condition to create AU*/
/* 2) otherwise, we will create AU when dma complete this time, no care about the fgUnitEnd and*/
/*alAULen*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterBuf2FifoAUCtrl(void *pvSptHdl, DMX_SPT_DMA2FIFO_INFO_T *prInf)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	void	*pvStm  = NULL;
	u32	mrRet = RET_DMX_OK;

	if ((NULL == prSpt) ||
		(NULL == prInf)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

#if DMX_DISABLE_DMA_DATA
	DMX_THREAD_DELAY(1);
	prSpt->u8PtxLen = prInf->u8TxLen;
	SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
	mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterChangeState")
			TEXT("ING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}
	MM_RETURN(RET_DMX_OK);
#endif

	if ((SPT_DATA_A == prInf->u4TxStreamType) &&
		(!SplitterIsEnableDmaAud(pvSptHdl))) {
		prSpt->u8PtxLen = prInf->u8TxLen;
		SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
		mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterChangeState")
				TEXT("(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
			MM_RETURN(mrRet);
		}
		MM_RETURN(RET_DMX_OK);
	}

	pvStm = GetStreamByType(pvSptHdl, prInf->u4TxStreamType);

	mrRet = SplitterSetPtxData(pvSptHdl, prInf, pvStm, FALSE);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterSetPtxData")
			TEXT(" (pvSptHdl 0x%p, mrRet: 0x%x)!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	/*fix Klocwork*/
	if (NULL == pvStm) {
		/*We Do not real issue Parser Tx in this case*/
		DMXLOG_DEBUG(TEXT("[SPT] %s exit for pvSptHdl(0x%p) has no stream")
			TEXT(" whose type is %d!\r\n"),
			DMX_FUNC_NAME, pvSptHdl, prInf->u4TxStreamType);

		/*DMX_ASSERT(FALSE);*/
		SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
		mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterChangeState")
				TEXT("(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
			MM_RETURN(mrRet);
		}

		MM_RETURN(RET_DMX_OK);
	}

	if (SPT_DATA_V == prInf->u4TxStreamType) {
		mrRet = PSR_Filter_SetCodeC(((DMX_STM_INST_T *)pvStm)->pvPsrFtr,
									 prInf->u4TxVideoCodec);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s fail in PSR_Filter_SetCodeC ")
				TEXT("(pvSptHdl 0x%p, Codec %d), mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, pvSptHdl, prInf->u4TxVideoCodec, mrRet);
			MM_RETURN(mrRet);
		}
	}

	if ((SplitterRspIsLoging(pvSptHdl) ||
		(!SplitterRspIsEnabled(prSpt))) &&
		((SPT_DATA_A == prInf->u4TxStreamType) ||
		 (SPT_DATA_SP == prInf->u4TxStreamType))) {
		if (prInf->u4TxUID != ((DMX_STM_INST_T *)pvStm)->u4StmUID) {
			/* Just a check for CFA UID set */
			/* the stream isn't current playing use, we add the AUFIFO into Resplitter LogAU List*/
			/*, and set PTX_DONE Event to Notify Cfa Tx Done virtually*/
			/* Just a check for CFA UID set */
			DMXLOG_ERROR(TEXT("%s line %d -- pvSptHdl(0x%p) -- prInf->")
				TEXT("u4TxUID(0x%x) != u4StmUID(0x%x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl,
				prInf->u4TxUID, ((DMX_STM_INST_T *)pvStm)->u4StmUID);

			/* We Do not real issue Parser Tx in this case */
			SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
			mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in Splitter")
					TEXT("ChangeState(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
				MM_RETURN(mrRet);
			}

			MM_RETURN(RET_DMX_OK);
		}
	}

	if (SPT_DATA_A == prInf->u4TxStreamType) {
		mrRet = PSR_Filter_SetCodeC(((DMX_STM_INST_T *)pvStm)->pvPsrFtr,
									 prInf->u4TxAudioCodec);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s fail in PSR_Filter_SetCodeC ")
				TEXT("(pvSptHdl: 0x%p, AudioCodec: %d), mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, pvSptHdl, prInf->u4TxAudioCodec, mrRet);
			MM_RETURN(mrRet);
		}
	} {
		PSR_AUCtrlInfo rAUCtrlInfo = {0};

		if (prInf->fgDummyUnit) {
			mrRet = PSR_Filter_AddDummyAU(((DMX_STM_INST_T *)pvStm)->pvPsrFtr,
						TRUE, FALSE);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in PSR_Filter")
					TEXT("_AddDummyAU, pvSptHdl 0x%p\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
				MM_RETURN(mrRet);
			}

			mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in Splitter")
					TEXT("ChangeState(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
				MM_RETURN(mrRet);
			}
			MM_RETURN(RET_DMX_OK);
		}

		if (NULL != ((DMX_STM_INST_T *)pvStm)->pvPsrFtr) {
			bool fgFifoFull = FALSE;
			PSR_FILTER *prPsrFtr = (PSR_FILTER *)(((DMX_STM_INST_T *)pvStm)->pvPsrFtr);

			if (NULL != prPsrFtr) {
				mrRet = PSR_Filter_IsFifoFull(prPsrFtr,
					&fgFifoFull, SplitterGetPtxLen(prSpt));
				if (DMX_SUCCEED(mrRet) && fgFifoFull)
					GAU_DisableThreshold();
			}
		}

		rAUCtrlInfo.fgCreateAU = prInf->fgCreateAU;
		rAUCtrlInfo.u8TotalAULen = prInf->u8TotalAULen;
		rAUCtrlInfo.u4Vtype = prInf->u4TxPictureMode;
		rAUCtrlInfo.fgQueryWVC1Mode = prInf->rExInf.rWVC1.fgQueryWVC1Mode;
		rAUCtrlInfo.u8Len = prInf->u8TxLen;

		#if ENABLE_DMX_ADVANCED_VER
		rAUCtrlInfo.fgInsertHdr		= prInf->fgInsertHdr;
		rAUCtrlInfo.pu1InsertHdrBuf	= prInf->pu1InsertHdrBuf;
		rAUCtrlInfo.u4InsertHdrLen	= prInf->u4InsertHdrLen;
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_INSTBS,
			TEXT("[SPT] %s -- InsertHdr(%s), HdrLen(%d), HdrBuf(0x%x)\r\n"),
		  DMX_FUNC_NAME, ((rAUCtrlInfo.fgInsertHdr) ? TEXT("TRUE") : TEXT("FALSE")),
		  rAUCtrlInfo.u4InsertHdrLen, rAUCtrlInfo.pu1InsertHdrBuf);
		#endif /* ENABLE_DMX_ADVANCED_VER*/

		mrRet = PSR_Filter_TxMemory2FifoWithAUCtrl(
			((DMX_STM_INST_T *)pvStm)->pvPsrFtr, prInf->pvFromAddress, &rAUCtrlInfo);
		if (DMX_FAILED(mrRet)) {
			if (!MM_IS_STATE_ERROR(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail pvSptHdl 0x%p in")
					TEXT(" PSR_Filter_TxMemory2FifoWithAUCtrl \r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
			}

			MM_RETURN(mrRet);
		}
	}

	mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterChangeState")
			TEXT("(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

/*/////////////////////////////////////////////////////////////////////////////*/
/* SplitterBuf2Fifo*/
/* Tx Pbbuf's data into Fifo*/
/* 1.  If prInf->fgAUByEnd is TRUE, this means that:*/
/*1) if prInf->fgCreateAU is TRUE, we need to create AU when prInf->fgUnitEnd is TRUE*/
/*2) otherwise, we will create AU when dma complete this time, no care about the fgUnitEnd and u8TotalAULen*/
/* 2.  If prInf->fgAUByEnd is FALSE, this means that:*/
/*		1) If prInf->fgCreateAU is TRUE, this means we need to create AU when the u8CurAULen*/
/* >= u8TotalAULen, i.e. Use Total AU Len as condition to create AU*/
/*		2) otherwise, we will create AU when dma complete this time, no care about the fgUnitEnd*/
/*and u8TotalAULen*/
/*/////////////////////////////////////////////////////////////////////////////*/
MRESULT SplitterPbb2Fifo(void *pvSptHdl, DMX_SPT_DMA2FIFO_INFO_T *prInf)
{
	u64	 u8CurPbbufStartOffset;
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	void	*pvStm;
	void	*pvPsrFtr = NULL;
	u32	 u4StmUID;
	PSR_AU	 rAuData;
	bool	 fgFifoFull = FALSE;
	MRESULT  mrRet = RET_DMX_OK;

	if ((NULL == pvSptHdl) ||
		(NULL == prInf)) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mm_memset(&rAuData, 0, sizeof(PSR_AU));

	mrRet = PSR_CC_GetCurPbbufStartOffset(((DMX_SPT_INST_T *)pvSptHdl)->pvPsrCC,
										  &u8CurPbbufStartOffset);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail in PSR_CC_GetCurPbbufStartOffset")
			TEXT(" (pvSptHdl 0x%p, mrRet: 0x%x)!\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	if (prInf->u8FromFileOfst < u8CurPbbufStartOffset) {
		#ifdef __linux__
		DMXLOG_ERROR(TEXT("[SPT] %s line %d -- Error (prInf->u8FromFileOfst")
			TEXT("(%lld) < u8CurPbbufStartOffset(%lld))\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prInf->u8FromFileOfst,
			u8CurPbbufStartOffset);
		#else
		DMXLOG_ERROR(TEXT("[SPT] %s line %d -- Error (prInf->u8FromFileOfst")
			TEXT("(%I64d) < u8CurPbbufStartOffset(%I64d))\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prInf->u8FromFileOfst,
			u8CurPbbufStartOffset);
		#endif /* #ifdef __linux__*/
		PBBUF_DumpInfo(pvSptHdl, FALSE);
		PSR_CC_AbortTx(((DMX_SPT_INST_T *)pvSptHdl)->pvPsrCC);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

#if DMX_DISABLE_DMA_DATA
	DMX_THREAD_DELAY(1);
	prSpt->u8PtxLen = prInf->u8TxLen;
	SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
	mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterChangeState")
			TEXT("(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
#endif

	if ((SPT_DATA_A == prInf->u4TxStreamType) &&
		(!SplitterIsEnableDmaAud(pvSptHdl))) {
		prSpt->u8PtxLen = prInf->u8TxLen;
		SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
		mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterChangeState")
				TEXT("(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
			MM_RETURN(mrRet);
		}

		MM_RETURN(RET_DMX_OK);
	}

	pvStm = GetStreamByType(pvSptHdl, prInf->u4TxStreamType);

	mrRet = SplitterSetPtxData(pvSptHdl, prInf, pvStm, TRUE);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterSetPtxData")
			TEXT(" (pvSptHdl 0x%p, mrRet: 0x%x)!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	/*fix Klocwork*/
	if (NULL == pvStm) {
		/*We Do not real issue Parser Tx in this case*/
		DMXLOG_DEBUG(TEXT("[SPT] %s exit for pvSptHdl(0x%p) has no stream")
			TEXT(" whose type is %d!\r\n"),
			DMX_FUNC_NAME, pvSptHdl, prInf->u4TxStreamType);

		SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
		mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterChang")
				TEXT("eState(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
			MM_RETURN(mrRet);
		}

		MM_RETURN(RET_DMX_OK);
	}

	pvPsrFtr = ((DMX_STM_INST_T *)pvStm)->pvPsrFtr;
	/*fix Klocwork*/
	if (NULL == pvPsrFtr) {
		/*We Do not real issue Parser Tx in this case*/
		DMXLOG_ERROR(TEXT("[SPT] %s exit for pvSptHdl(0x%p) has no Psr ")
			TEXT("Filter whose type is %d!\r\n"),
			DMX_FUNC_NAME, pvSptHdl, prInf->u4TxStreamType);

		MM_RETURN(RET_DMX_NOT_FOUND);
	}

	if (SPT_DATA_V == prInf->u4TxStreamType) {
		mrRet = PSR_Filter_SetCodeC(((DMX_STM_INST_T *)pvStm)->pvPsrFtr,
									 prInf->u4TxVideoCodec);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s fail in PSR_Filter_SetCodeC")
				TEXT(" (pvSptHdl 0x%p, Codec %d), mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, pvSptHdl, prInf->u4TxVideoCodec, mrRet);
			MM_RETURN(mrRet);
		}
	}

	u4StmUID = GetStmUIDByType(pvSptHdl, prInf->u4TxStreamType);

	if (prInf->u4TxUID != u4StmUID) {
		/* the stream isn't current playing use, we add the AUFIFO into Resplitter LogAU List, */
		/*and set PTX_DONE Event to Notify Cfa Tx Done virtually*/
		/* Just a check for CFA UID set */
		switch (prInf->u4TxStreamType) {
		case SPT_DATA_V:
			DMXLOG_ERROR(
				TEXT("[SPT] %s -- Warning u4StmUID: %d != prInf->u4TxUID: %d\r\n"),
				DMX_FUNC_NAME, u4StmUID, prInf->u4TxUID);
			/* We Do not real issue Parser Tx in this case */
			SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
			mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in Splitter")
					TEXT("ChangeState(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
				MM_RETURN(mrRet);
			}
			MM_RETURN(RET_DMX_OK);
			break;
		case SPT_DATA_A: {
				AU_AUDIO	rAudioAU;

				mm_memset(&rAudioAU, 0, sizeof(AU_AUDIO));
				rAudioAU.eAuType = AU_DATA;
				rAuData.eType = prInf->u4TxStreamType;
				rAuData.pvAUInf = &rAudioAU;
				rAuData.pvAUExtInf = NULL;
				mrRet = SplitterSetPsrAuTable(pvSptHdl, (void *)&rAuData);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[SPT] %s fail in SplitterSetPsrAuTable")
						TEXT(" (pvSptHdl 0x%p, mrRet: 0x%x)!\r\n"),
						DMX_FUNC_NAME, pvSptHdl, mrRet);
					MM_RETURN(mrRet);
				}

				/* We Do not real issue Parser Tx in this case */
				DMXLOG_DEBUG(TEXT("[SPT] %s line %d -- (u8TxUID(%d) !=")
					TEXT(" StmUID(%d))Set SPLITTER_EV_PTX_DONE, pvSptHdl: 0x%p\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prInf->u4TxUID, u4StmUID, pvSptHdl);
				SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
				mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in ")
						TEXT("SplitterChangeState(RUNING, TXING), pvSptHdl: 0x%p, ")
						TEXT("mrRet: 0x%x\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
					MM_RETURN(mrRet);
				}

				MM_RETURN(RET_DMX_OK);
			}
			break;
		case SPT_DATA_SP: {
				AU_SP	 rSPAU;

				mm_memset(&rSPAU, 0, sizeof(AU_SP));
				rSPAU.eAuType = AU_DATA;
				rAuData.eType = prInf->u4TxStreamType;
				rAuData.pvAUInf = &rSPAU;
				rAuData.pvAUExtInf = NULL;
				mrRet = SplitterSetPsrAuTable(pvSptHdl, (void *)&rAuData);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[SPT] %s fail in SplitterSet")
						TEXT("PsrAuTable (pvSptHdl 0x%p, mrRet: 0x%x)!\r\n"),
						DMX_FUNC_NAME, pvSptHdl, mrRet);
					MM_RETURN(mrRet);
				}

				/* We Do not real issue Parser Tx in this case */
				SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
				mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in Splitte")
						TEXT("rChangeState(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
					MM_RETURN(mrRet);
				}

				MM_RETURN(RET_DMX_OK);
			}
			break;
		default:
			DMXLOG_ERROR(TEXT("[SPT] % line %d fail for err stmtype: %d\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prInf->u4TxStreamType);
			break;
		}
	}

	if (SPT_DATA_A == prInf->u4TxStreamType) {
		mrRet = PSR_Filter_SetCodeC(((DMX_STM_INST_T *)pvStm)->pvPsrFtr,
									 prInf->u4TxAudioCodec);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s fail in PSR_Filter_SetCodeC")
				TEXT(" (pvSptHdl 0x%p, AudioCodec %d), mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, pvSptHdl, prInf->u4TxAudioCodec, mrRet);
			MM_RETURN(mrRet);
		}
	}

	if (prInf->fgDummyUnit) {
		DMXLOG_DEBUG(TEXT("[SPT] %s -- PSR_Filter_AddDummyAU, pvSptHdl: 0x%x\r\n"),
			DMX_FUNC_NAME, pvSptHdl);

		mrRet = PSR_Filter_AddDummyAU(pvPsrFtr, prInf->fgDummyAUEnd, prInf->fgDummyCmdAU);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] % line %d fail in PSR_Filter_AddDummyAU")
				TEXT(" (pvSptHdl 0x%p, fgDummyAUEnd: %d)!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl,
				(prInf->fgDummyAUEnd ? 1 : 0));
			MM_RETURN(mrRet);
		}

		mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterChangeState")
				TEXT("(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
			MM_RETURN(mrRet);
		}

		MM_RETURN(RET_DMX_OK);
	}

	fgFifoFull = FALSE;

	if (NULL != pvPsrFtr) {
		mrRet = PSR_Filter_IsFifoFull(pvPsrFtr,
			&fgFifoFull, SplitterGetPtxLen(prSpt));
		if (DMX_SUCCEED(mrRet) &&
			fgFifoFull) {
			GAU_DisableThreshold();
		}
	}

	if (prInf->fgAUByEnd) {
		EXT_INFO_T rEx = {0};
		u32 u4Codec = 0;

		rEx.fgCreateAU = prInf->fgCreateAU;
		rEx.fgAUEnd    = prInf->fgUnitEnd;

		if ((VC_UNKNOW == prInf->u4TxVideoCodec) &&
			(AUD_DRV_FMT_UNKNOWN != prInf->u4TxAudioCodec)) { /*/ Audio*/
			u4Codec = prInf->u4TxAudioCodec;
		} else { /*/ Video*/
			u4Codec		= prInf->u4TxVideoCodec;
			rEx.u4Vtype = prInf->u4TxPictureMode;
		}
		DMXLOG_DEBUG(TEXT("[SPT] %s -- PSR_Filter_TxPBBuf2FifoWithAUEnd")
			TEXT(", pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, pvSptHdl);

		mrRet = PSR_Filter_TxPBBuf2FifoWithAUEnd(pvPsrFtr, prInf->u8FromFileOfst,
			prInf->u8TxLen, &rEx);

		if (DMX_FAILED(mrRet)) {
			if (!MM_IS_STATE_ERROR(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s fail in PSR_Filter_TxPBBuf")
					TEXT("2FifoWithAUEnd(pvSptHdl: 0x%p, mrRet: 0x%x)!\r\n"),
					DMX_FUNC_NAME, pvSptHdl, mrRet);
			}
			MM_RETURN(mrRet);
		}
	} else{
		PSR_AUCtrlInfo rAUCtrlInfo = {0};

		rAUCtrlInfo.fgCreateAU		= prInf->fgCreateAU;
		rAUCtrlInfo.u8TotalAULen	= prInf->u8TotalAULen;
		rAUCtrlInfo.u4Vtype			= prInf->u4TxPictureMode;
		rAUCtrlInfo.fgQueryWVC1Mode = prInf->rExInf.rWVC1.fgQueryWVC1Mode;
		rAUCtrlInfo.u8Len			= prInf->u8TxLen;

		rAUCtrlInfo.fgUseCmdQ		= prInf->fgUseCmdQ;
		rAUCtrlInfo.fgAUByCmdQEnd	= prInf->fgAUByCmdQEnd;
		rAUCtrlInfo.u2TxEntryCnt	= prInf->u2TxEntryCnt;
		rAUCtrlInfo.u8RealTxLen		= prInf->u8RealTxLen;
		rAUCtrlInfo.parCmdQTxEntry	= prInf->parCmdQTxEntry;

		#if ENABLE_DMX_ADVANCED_VER
		rAUCtrlInfo.fgInsertHdr		= prInf->fgInsertHdr;
		rAUCtrlInfo.pu1InsertHdrBuf	= prInf->pu1InsertHdrBuf;
		rAUCtrlInfo.u4InsertHdrLen	= prInf->u4InsertHdrLen;
		DmxLogD(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_INSTBS,
			TEXT("[SPT] %s -- InsertHdr(%s), HdrLen(%d), HdrBuf(0x%x)\r\n"),
		  DMX_FUNC_NAME, ((rAUCtrlInfo.fgInsertHdr) ? TEXT("TRUE") : TEXT("FALSE")),
		  rAUCtrlInfo.u4InsertHdrLen, rAUCtrlInfo.pu1InsertHdrBuf);
		#endif /* ENABLE_DMX_ADVANCED_VER*/

		DMXLOG_DEBUG(
			TEXT("[SPT] %s -- PSR_Filter_TxPBBuf2FifoWithAUCtrl, pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, pvSptHdl);

		mrRet = PSR_Filter_TxPBBuf2FifoWithAUCtrl(pvPsrFtr, prInf->u8FromFileOfst, &rAUCtrlInfo);
		if (DMX_FAILED(mrRet)) {
			if (!MM_IS_STATE_ERROR(mrRet)) {
				DMXLOG_ERROR(
					TEXT("[SPT] %s fail in PSR_Filter_TxPBBuf2FifoWithAUCtrl(pvSptHdl:")
					TEXT(" 0x%p, mrRet: 0x%x)!\r\n"),
					DMX_FUNC_NAME, pvSptHdl, mrRet);
			}
			MM_RETURN(mrRet);
		}
	}

	mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in SplitterChangeState")
			TEXT("(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);

}

MRESULT SplitterTxEndCmdAU(void *pvSptHdl, E_SPT_DATA_TYPE_T eDataType)
{
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == pvSptHdl) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	switch (eDataType) {
	case SPT_DATA_V: {
			CFA_VIDEO_INFO_T rVidInfo;
			u64 u8CurPbbufStartOffset = 0;
			u32 u4StmUID = DMX_INVALID_UINT32;
			DMX_STM_INST_T *prStm = NULL;
			VCodeC eCodec = VC_UNKNOW;
			u32 u4Codec = DMX_INVALID_UINT32;

			prStm = (DMX_STM_INST_T *)GetStreamByType(pvSptHdl, SPT_DATA_V);
			if (NULL == prStm) {
				DMXLOG_TRACE(TEXT("[SPT] %s line %d exit, no video stream")
					TEXT(" for this splitter instance(pvSptHdl: 0x%p)!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
				MM_RETURN(RET_DMX_OK);
			}

			if (NULL == prStm->pvPsrFtr) {
				DMXLOG_TRACE(TEXT("[SPT] %s line %d exit, no PsrFtr ")
					TEXT("of the video stream for this splitter instance(pvSptHdl: 0x%p)!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
				MM_RETURN(RET_DMX_OK);
			}

			if (NULL == ((DMX_SPT_INST_T *)pvSptHdl)->pvPsrCC) {
				DMXLOG_TRACE(TEXT("[SPT] %s line %d exit, no PsrFtr of")
					TEXT(" the video stream for this splitter instance(pvSptHdl: 0x%p)!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
				MM_RETURN(RET_DMX_OK);
			}

			mrRet = PSR_Filter_GetCodeC(prStm->pvPsrFtr, &u4Codec);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s fail in PSR_Filter_GetCodeC")
					TEXT(" (pvSptHdl 0x%p, mrRet: 0x%x)!\r\n"),
					DMX_FUNC_NAME, pvSptHdl, mrRet);
				MM_RETURN(mrRet);
			}

			eCodec = (VCodeC)u4Codec;

			if (VC_UNKNOW == eCodec) {
				DMXLOG_ERROR(TEXT("[SPT] %s fail in PSR_Filter_GetVideoCodeC")
					TEXT(" (pvSptHdl 0x%p, mrRet: 0x%x)!\r\n"),
					DMX_FUNC_NAME, pvSptHdl, mrRet);
				MM_RETURN(RET_DMX_OK);
			}

			#if DMX_DISABLE_COMP_MPEG2AU
			if (VC_MPEG2 == eCodec)
				MM_RETURN(RET_DMX_OK);

			#endif /* DMX_DISABLE_COMP_MPEG2AU*/

			#if DMX_DISABLE_COMP_MPEG4AU
			if ((VC_MPEG4 == eCodec) ||
				(VC_DIVX4 == eCodec) ||
				(VC_DIVX6 == eCodec) ||
				(VC_H263  == eCodec)) {
				MM_RETURN(RET_DMX_OK);
			}
			#endif /* DMX_DISABLE_COMP_MPEG4AU*/

			#if DMX_DISABLE_COMP_AVCAU
			if (VC_H264 == eCodec)
				MM_RETURN(RET_DMX_OK);

			#endif /* DMX_DISABLE_COMP_AVCAU*/

			#if DMX_DISABLE_COMP_VC1AU
			if (VC_VC1 == eCodec)
				MM_RETURN(RET_DMX_OK);

			#endif /* DMX_DISABLE_COMP_VC1AU*/

			u4StmUID = GetStmUIDByType(pvSptHdl, eDataType);

			if (DMX_INVALID_UINT32 == u4StmUID) {
				DMXLOG_TRACE(TEXT("[SPT] %s line %d exit, video stream's")
					TEXT(" UID is invalid for this splitter instance(pvSptHdl: 0x%p)!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
				MM_RETURN(RET_DMX_OK);
			}

			mrRet = PSR_CC_GetCurPbbufStartOffset(
				((DMX_SPT_INST_T *)pvSptHdl)->pvPsrCC, &u8CurPbbufStartOffset);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s fail in PSR_CC_GetCurPbbuf")
					TEXT("StartOffset (pvSptHdl 0x%p, mrRet: 0x%x)!\r\n"),
					DMX_FUNC_NAME, pvSptHdl, mrRet);
				MM_RETURN(mrRet);
			}

			mm_memset(&rVidInfo, 0, sizeof(CFA_VIDEO_INFO_T));

			rVidInfo.u8FileOfst = u8CurPbbufStartOffset;
			rVidInfo.eVidType = eCodec;
			rVidInfo.u4PrsStrmId = u4StmUID;
			rVidInfo.eTxMode = CFA_PTM_DUMMY;
			rVidInfo.u8Len = 0;
			rVidInfo.u8TotalAULen = 0;
			rVidInfo.fgDummyAU = TRUE;
			rVidInfo.fgDummyAUEnd = TRUE;
			rVidInfo.fgDummyCmdAU = TRUE;

			mrRet = Spt4CfaPbb2VFifoAUCtrl(pvSptHdl, &rVidInfo);

			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(TEXT("[SPT] %s fail in Spt4CfaPbb2VFifoAUCtrl")
					TEXT(" (pvSptHdl 0x%p, mrRet: 0x%x)!\r\n"),
					DMX_FUNC_NAME, pvSptHdl, mrRet);
				MM_RETURN(mrRet);
			}
		}
		break;
	default:
		DMXLOG_TRACE(TEXT("[SPT] %s exit, StreamType(%d)!\r\n"),
			DMX_FUNC_NAME, eDataType);
		MM_RETURN(RET_DMX_OK);
	}

	MM_RETURN(mrRet);
}

MRESULT SplitterPsrSetEOS(void *pvSptHdl, u32 u4Status)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == pvSptHdl) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	SplitterSetPtxNotBusy(prSpt);

	if ((GAU_E_ERRCHUNK == u4Status) ||
		(GAU_E_ERRDATA == u4Status)) {
		DMXLOG_TRACE(TEXT("[SPT] %s line %d -- Spt(0x%p)'s Repeat ")
			TEXT("Error Chunk Count(%d), GAU_E_ERRCHUNK or GAU_E_ERRDATA\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, prSpt->u4RepeatErrChkCnt);
		prSpt->u4RepeatErrChkCnt++;
		if (prSpt->u4RepeatErrChkCnt >= DMX_MAX_ERRCHUNK_CNT) {
			u4Status = GAU_E_FAIL;
			DMXLOG_ERROR(TEXT("[SPT] %s line %d -- Spt(0x%p)'s Repeat ")
				TEXT("Error Chunk Count > Limit(%d), Change Status to be GAU_E_FAIL\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, DMX_MAX_ERRCHUNK_CNT);
		}
	}

	if ((GAU_E_ERRCHUNK != u4Status) &&
		(GAU_E_ERRDATA != u4Status)) {
		mrRet = SplitterTxEndCmdAU(pvSptHdl, SPT_DATA_V);

		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s fail in SplitterTxEndCmdAU (SPT_DATA_V)\r\n"),
				DMX_FUNC_NAME);
			DMX_ASSERT(FALSE);
		}
	}

	GAU_SetEOS(pvSptHdl, TRUE, u4Status);

	mrRet = SplitterSetCfaPsrEnd(pvSptHdl, TRUE, u4Status);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in SplitterSetCfaPsrEnd (pvSptHdl: 0x%p, TRUE)\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		DMX_ASSERT(FALSE);
	}

	mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_IDLE, SPLITTER_TX_STATE_NONE);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail in SplitterSetCfaPsrEnd (pvSptHdl: 0x%p, TRUE)\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		DMX_ASSERT(FALSE);
	}

	/* if the file is a error one, and cfa container could read any packet in it*/
	/* cfa container would like to finished to play the file*/
	/* But we AU threshold is not reached, so we could not stop and stain in waiting pbbuf state*/
	/* to Prevent this issue ,so we have to setting Threshold state by force*/
	GAU_DisableThreshold();

	#if DMX_PFM_TEST
	DmxPfmPrintInfo(pvSptHdl);
	#endif /* DMX_PFM_TEST*/

	DMXLOG_TRACE(TEXT("[SPT] %s success, pvSptHdl: 0x%p\r\n"),
		DMX_FUNC_NAME, pvSptHdl);

	MM_RETURN(mrRet);
}

#if DMX_SUPPORT_DIVXDRM

MRESULT SplitterSetPtxDivxDRMInf(
	void *pvSptHdl,
	u64 u8Offset,
	u32 u4DecLen,
	u16 u2FrameKeyIdx)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;

	if (NULL == prSpt) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

#if DMX_SPT_RSP_USING_DIVXDRM
	prSpt->u8DivxDRMOffset = u8Offset;
	prSpt->u4DecLen		= u4DecLen;
	prSpt->u2FrameKeyIndex = u2FrameKeyIdx;
#else
	prSpt->u8DivxDRMOffset = DMX_INVALID_UINT64;
	prSpt->u4DecLen		= 0;
	prSpt->u2FrameKeyIndex = DMX_DIVXDRM_INVALID_FRAMEIDX;
#endif /* DMX_SPT_RSP_USING_DIVXDRM*/

	MM_RETURN(RET_DMX_OK);
}

MRESULT SplitterPsrTurnDivxDRM(void *pvSptHdl, CFA_DIVXDRM_INFO_T *prDivxDRMInf)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	PSR_DivxDRMInfo rPsrDivxDRM;
	MRESULT mrRet = RET_DMX_OK;

	mm_memset(&rPsrDivxDRM, 0, sizeof(rPsrDivxDRM));

	if ((NULL == prSpt) ||
		(NULL == prDivxDRMInf)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	rPsrDivxDRM.fgTurnOn = prDivxDRMInf->fgOn;
	rPsrDivxDRM.u2FrameKeyIdx = prDivxDRMInf->u2FrameKeyIdx;
	rPsrDivxDRM.u8DecryptStOfst = prDivxDRMInf->u8DecryptStOfst;
	rPsrDivxDRM.u4DecryptLen = prDivxDRMInf->u4DecryptLen;

	mrRet = PSR_CC_EnableDivxDRMDecrypt(prSpt->pvPsrCC, &rPsrDivxDRM);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail in PSR_CC_EnableDivxDRMDecrypt")
			TEXT(" (pvSptHdl 0x%p, pvPsrCC: 0x%x), mrRet: 0x%x!\r\n"),
			DMX_FUNC_NAME, pvSptHdl, prSpt->pvPsrCC, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(mrRet);
}

#endif /*#ifdef ENABLE_DIVXDRM*/

#if CONFIG_DRV_HDMI_RX
MRESULT SplitterPsrAudInIsRaw(void *pvSptHdl, bool *pfgIsRawAud)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prSpt) ||
		(NULL == pfgIsRawAud)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mrRet = PSR_CC_IsAudinRaw(prSpt->pvPsrCC, pfgIsRawAud);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail in PSR_CC_IsAudinRaw (pvSptHdl ")
			TEXT("0x%p, pvPsrCC: 0x%p), mrRet: 0x%x!\r\n"),
			DMX_FUNC_NAME, pvSptHdl, prSpt->pvPsrCC, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(mrRet);
}

MRESULT SplitterPsrGetAudInParsingInfo(void *pvSptHdl, AUDIN_PARSING_INFO_T *prPsrInfo)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prSpt) ||
		(NULL == prPsrInfo)) {
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	mrRet = PSR_CC_GetAudInParsingInfo(prSpt->pvPsrCC, prPsrInfo);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in PSR_CC_GetAudIn")
			TEXT("ParsingInfo (pvSptHdl 0x%p), mrRet: 0x%x!\r\n"),
			DMX_FUNC_NAME, pvSptHdl, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}
#endif /* CONFIG_DRV_HDMI_RX*/

MRESULT SplitterCreateCmdAU(void *pvStm, E_SPT_DATA_TYPE_T eDataType)
{
	DMX_STM_INST_T	  *prStm	= (DMX_STM_INST_T *)pvStm;
	PSR_FILTER *prPsrFtr = NULL;
	u32		u4WrIdx  = 0;
	uintptr_t ptrFifoSa = 0;
	PSR_VFSD   *prVFSD	 = NULL;
	MRESULT		mrRet = RET_DMX_OK;

	if ((NULL == prStm) ||
		((SPT_DATA_V != eDataType) && (SPT_DATA_A != eDataType))) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args, eDataType: %d\r\n"),
			DMX_FUNC_NAME, eDataType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPsrFtr = (PSR_FILTER *)(prStm->pvPsrFtr);

	if (NULL == prPsrFtr) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for psr filter is NULL\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (DMX_INVALID_UINT32 == prPsrFtr->u4ESIH) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for psr filter's ESIH = ")
			TEXT("DMX_INVALID_UINT32, eDataType: %d\r\n"),
			DMX_FUNC_NAME, eDataType);
		MM_RETURN(RET_DMX_OPERATE_FORBID);
	}

	mrRet = ESM_AUTableGetWrIdx(prPsrFtr->u4ESIH, &u4WrIdx);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in ESM_AUTableGetWrIdx(ESIH: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (ESM_INVALID_INDEX == u4WrIdx) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for Audio Current WrIdx is ESM_INVALID_INDEX!!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	mrRet = ESM_FifoGetSA(prPsrFtr->u4ESIH, &ptrFifoSa);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s line %d fail in ESM_AUTableGetWrIdx(ESIH: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH);
		DMX_ASSERT(FALSE);
		MM_RETURN(mrRet);
	}

	if (ESM_INVALID_ADDRESS == ptrFifoSa) {
		DMXLOG_ERROR(
			TEXT("[SPT] %s fail for Audio Fifo Sa is ESM_INVALID_ADDRESS!!\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (SPT_DATA_A == eDataType) {
		AU_AUDIO   *prAudAU = NULL;

		mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4WrIdx, (void **)(&prAudAU));
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in ESM_AUTableGetAUInfo(ESIH: 0x%x, ")
				TEXT("u4WrIdx: %d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, u4WrIdx);
			MM_RETURN(mrRet);
		}

		prAudAU->eAuType = AU_CMD;
		prAudAU->ptrEAddr = ptrFifoSa;
		prAudAU->ptrSAddr = ptrFifoSa;
	} else{
		AU_VPic *prVidAU = NULL;

		mrRet = ESM_AUTableGetAUInfo(prPsrFtr->u4ESIH, u4WrIdx, (void **)(&prVidAU));
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[SPT] %s line %d fail in ESM_AUTableGetAUInfo(ESIH: 0x%x, ")
				TEXT("u4WrIdx: %d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH, u4WrIdx);
			MM_RETURN(mrRet);
		}

		prVidAU->eAuType = AU_CMD;
		prVidAU->fgIBCSent = TRUE;
		/* Designated whether the CmdAU is the last Pic CmdAU(FALSE) or ClearVFifo CmdAU(TRUE)*/
		prVFSD = (PSR_VFSD *)(prPsrFtr->pvFilterSpecific);
		prVidAU->rAUInfo.rInfo.u4VType = 0;
		if (NULL != prVFSD) {
			SetVCodec(prVidAU->rAUInfo.rInfo.u4VType, prVFSD->eVCodeC);
			dmx_memset(&(prVFSD->rPicDetResult), 0, sizeof(prVFSD->rPicDetResult));
		}
		prVidAU->rAUInfo.rInfo.ptrEAddr = ptrFifoSa;
		prVidAU->rAUInfo.rInfo.ptrSAddr = ptrFifoSa;
	}

	mrRet = ESM_AUTableIncWrIdx(prPsrFtr->u4ESIH, 1);

	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(TEXT("[SPT] %s line %d fail in ESM_AUTableIncWr")
			TEXT("Idx(ESIH: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPsrFtr->u4ESIH);
		MM_RETURN(mrRet);
	}

	DMXLOG_TRACE(TEXT("[SPT] %s ---- Fill %s CMD AU Success!!\r\n"),
		DMX_FUNC_NAME, ((eDataType < MAX_SPT_DATA_TYPE_CNT) ?
		g_aszSptDataTypeName[eDataType] : TEXT("UNKNOWN")));

	MM_RETURN(RET_DMX_OK);
}


