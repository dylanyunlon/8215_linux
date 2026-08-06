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
 *@file dmx_pbbuf_ext.c
 *
 *@par Project
 *	MT3360
 *
 *@par Description
 *
 *
 *@par Author_Name
 *	Shuhui Zhang
 *
 */

#include "x_typedef.h"
#include "x_os.h"
#include "x_debug.h"
#include "windows.h"
#include "winutil.h"
#include "drv_def.h"
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/sched.h>

#include <media/atc/dmx_define.h>
#include <media/atc/dmx_event.h>
#include <media/atc/dmx_splitter.h>
#if CONFIG_DRV_HDMI_RX
#include <media/atc/x_audin.h>
#include <media/atc/dmx_cfa_audin.h>
#endif				/*CONFIG_DRV_HDMI_RX*/
#include <media/atc/ose_mem.h>
#include <media/atc/ioctl_dmx.h>
/* #include <media/atc/mm_debug.h> */
#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_spt.h"
#include "dmx_dump.h"
#include "dmx_pbbuf.h"
#include "dmx_pbbuf_if.h"
#include "dmx_psr_pbbuf.h"
#include "dmx_psr_cc.h"
#include "aud_esm.h"

#if CONFIG_DRV_HDMI_RX
#endif				/* CONFIG_DRV_HDMI_RX*/

#define HDMI_AUDIN_USE_SESSIONID 1

#if CONFIG_DRV_HDMI_RX

u32 PBBUF_AudIn_SubscribeDrvCb(void *pvDmxTag,
	DRV_AUDIN_CB_LIST_T *prCbsList)
{
	void *pvSptHdl = pvDmxTag;
	PBBUF_AUDIN_EXTINFO_T *prExtInfo = NULL;
	PBBUF *prPbBuf = NULL;

	if ((NULL == pvSptHdl) || (NULL == prCbsList)) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail for invalid args\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return (-1);
	}

	prPbBuf = (PBBUF *) SplitterGetPBBuf(pvSptHdl);
	if (NULL == prPbBuf) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail for invalid prPbBuf\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return (-1);
	}
	/*PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);*/

	if (NULL == prPbBuf->pvExtInfo) {
		DMXLOG_ERROR(
			TEXT
			("[PBBUF] %s line %d fail for prPbBuf(0x%lx)'s pvExtInfo == NULL\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);
		return (-1);
	}

	prExtInfo = (PBBUF_AUDIN_EXTINFO_T *) (prPbBuf->pvExtInfo);

	prExtInfo->rAudInCbsInfo.pvUpdateRpCb = prCbsList->pvUpdateRpCb;
	prExtInfo->rAudInCbsInfo.pvGetAudInParsingInfo = prCbsList->pvGetAudInParsingInfo;
	prExtInfo->rAudInCbsInfo.pfgAudInIsRaw = prCbsList->pfgAudInIsRaw;
	prExtInfo->rAudInCbsInfo.pu4GetPsrSessionID = prCbsList->pu4GetPsrSessionID;

	/*PBBUF_EXIT(prPbBuf);*/

	return 0;
}

u32 PBBUF_AudIn_UnSubscribeDrvCb(void *pvDmxTag)
{
	void *pvSptHdl = pvDmxTag;
	PBBUF_AUDIN_EXTINFO_T *prExtInfo = NULL;
	PBBUF *prPbBuf = NULL;

	if (NULL == pvSptHdl) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail for invalid args\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		return (-1);
	}

	prPbBuf = (PBBUF *) SplitterGetPBBuf(pvSptHdl);
	if (NULL == prPbBuf) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail for invalid prPbBuf\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		return (-1);
	}
	/*PBBUF_ENTRY(prPbBuf, DMX_SEMA_OPTION_WAIT);*/

	if (NULL == prPbBuf->pvExtInfo) {
		DMXLOG_ERROR(
			TEXT
			("[PBBUF] %s line %d fail for prPbBuf(0x%lx)'s pvExtInfo == NULL\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);
		return (-1);
	}

	prExtInfo = (PBBUF_AUDIN_EXTINFO_T *) (prPbBuf->pvExtInfo);

	prExtInfo->rAudInCbsInfo.pvUpdateRpCb = NULL;
	prExtInfo->rAudInCbsInfo.pvGetAudInParsingInfo = NULL;
	prExtInfo->rAudInCbsInfo.pfgAudInIsRaw = NULL;
	prExtInfo->rAudInCbsInfo.pu4GetPsrSessionID = NULL;

	/*PBBUF_EXIT(prPbBuf);*/

	return 0;
}
#endif				/* CONFIG_DRV_HDMI_RX*/

static u8 from_sched_priority(int sched_priority)
{
	return (u8) ((100 - sched_priority) * 256 / 100);
}

static int to_sched_priority(u8 ui1_priority)
{
	int sched_priority;

	sched_priority = (int)100 - (int)ui1_priority * (int)100 / (int)256;
	if (sched_priority < 1)
		sched_priority = 1;
	if (sched_priority > 99)
		sched_priority = 99;
	return sched_priority;
}

int PBBUF_ReceiveDataThread(void *pv_arg)
{
	SEND_BUFFER rSendBuf;
	PBBUF_AUDIN_EXTINFO_T *prExtInfo = NULL;
	PBBUF *prPbBuf = ((NULL != pv_arg) ? ((PBBUF *) pv_arg) : NULL);
	MRESULT mrRet = RET_DMX_OK;
	s32 i4Ret = OSR_OK;
	u16 u2Index = 0;
	size_t z_msg_size = 0;
	u16 u2AvailMsgCnt = 0;
	bool fgRet = TRUE;

	if (NULL == prPbBuf) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail for invalid args\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		return 0;
	}

	prExtInfo = (PBBUF_AUDIN_EXTINFO_T *) (prPbBuf->pvExtInfo);

	if (NULL == prExtInfo) {
		DMXLOG_ERROR(
			TEXT
			("[PBBUF] %s line %d fail for invalid args, prPbbuf->pvExtInfo == NULL\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		DMX_ASSERT(FALSE);
		return 0;
	}

	while (fgRet) {
		mm_memset(&rSendBuf, 0, sizeof(SEND_BUFFER));
		z_msg_size = sizeof(SEND_BUFFER);
		i4Ret =
			x_msg_q_receive(&u2Index, &rSendBuf, &z_msg_size, (uintptr_t *)&(prExtInfo->hMsgQueue), 1,
					X_MSGQ_OPTION_WAIT);
		if (OSR_OK != i4Ret) {
			DMXLOG_ERROR(
				TEXT("[PBBUF] %s line %d fail in WaitFormSingleObject")
				TEXT("(prExtInfo->hMsgQueue(0x%x)), i4Ret: %d, LastErr:")
				TEXT(" 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prExtInfo->hMsgQueue, i4Ret,
				DMX_GET_LASTERR);
			break;
		}

		if (z_msg_size < sizeof(SEND_BUFFER)) {
			DMXLOG_ERROR(
				TEXT("[PBBUF] %s line %d fail for z_msg_size(%d) returnd by ")
				TEXT("x_msg_q_receive(prExtInfo->hMsgQueue(0x%x)) < sizeof(rSendBuf)")
				TEXT("(0x%x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, z_msg_size, prExtInfo->hMsgQueue,
				sizeof(SEND_BUFFER));
			break;
		}

		if (PBBUF_SLOT_END == rSendBuf.rHeader.eType) {
			DMXLOG_TRACE(
				TEXT("[PBBUF] %s line %d -- Receive END_SLOT, exit thread\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
			break;
		}

		i4Ret = x_msg_q_num_msgs((uintptr_t)prExtInfo->hMsgQueue, &u2AvailMsgCnt);

		if (OSR_OK == i4Ret) {
			if ((prPbBuf->u4SlotAmount >= 8)
				&& (u2AvailMsgCnt > prPbBuf->u4SlotAmount * 3 / 4)) {
				DMXLOG_TRACE(
					TEXT("[PBBUF] %s line %d -- AudInDmxMsgQueue's MsgCnt(%d)")
					TEXT(" > SlotAmout(%d) * 3/4 \r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, (u32) u2AvailMsgCnt,
					prPbBuf->u4SlotAmount);
			}
		} else {
			DMXLOG_ERROR(
				TEXT("[PBBUF] %s line %d fail in x_msg_q_num_msgs, Error: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, DMX_GET_LASTERR);
		}

		mrRet = PBBUF_SendDataSlot(prPbBuf->pvSptHdl, &rSendBuf, NULL);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT("[PBBUF] %s line %d fail in pmrPbBufSendDataSlot")
				TEXT("(prPbBuf: 0x%x), exit thread\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);
			break;
		}
	}

	prExtInfo->fgExitThread = TRUE;

#ifdef __linux__
	DMXLOG_TRACE(TEXT("[PBBUF] PBBUF EXT thread loop process (PBBUF: 0x%x),")
		TEXT("now wait kthread_should_stop flag.\r\n"),
		prPbBuf);

	while (TRUE) {
		prExtInfo->waitexitqueueflag = 0;
		wait_event_interruptible_timeout(prExtInfo->waitexitqueue,
			prExtInfo->waitexitqueueflag, HZ/50);
		/* HZ/50=20ms */
		if (kthread_should_stop())
			break;
		DMXLOG_DEBUG(TEXT("[PBBUF] %s line %d wait for kthread_should_stop, ")
			TEXT("(PBBUF: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);
	}
#endif				/* __linux__*/

	DMXLOG_TRACE(TEXT("[PBBUF] %s line %d -- PBBUF Ext thread exit (PBBUF: 0x%x)\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);

	return 0;
}

MRESULT PBBUF_ExtBuf_ExtSendData(void *pvDmxTag, SEND_BUFFER *prSendBuffer)
{
	PBBUF_AUDIN_EXTINFO_T *prExtInfo = NULL;
	PBBUF *prPbBuf = (PBBUF *) SplitterGetPBBuf(pvDmxTag);
	s32 i4Ret = OSR_OK;

	DMXLOG_DEBUG(TEXT("[PBBUF] -- %s enter\r\n"), DMX_FUNC_NAME);

	if (!(prPbBuf)) {
		DMXLOG_ERROR(TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
		TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DMXLOG_DEBUG(TEXT("[PBBUF] %s enter\r\n"), DMX_FUNC_NAME);

	if (NULL == prSendBuffer) {
		DMXLOG_ERROR(
			TEXT
			("[PBBUF] %s line %d fail for invalid args (pvPbBuf: 0x%x, prSdBuf: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prSendBuffer);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prExtInfo = (PBBUF_AUDIN_EXTINFO_T *) (prPbBuf->pvExtInfo);

	if ((NULL != prExtInfo) && (NULL != prExtInfo->hMsgQueue)) {
		i4Ret = x_msg_q_send((uintptr_t)prExtInfo->hMsgQueue, prSendBuffer, sizeof(SEND_BUFFER), 1);
		if (OSR_TOO_MANY == i4Ret) {
			DMXLOG_ERROR(
				TEXT("[PBBUF] %s line %d fail in x_msg_q_send(pvPbBuf:")
				TEXT(" 0x%x, prSdBuf: 0x%x), MsgQueue FULL\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prSendBuffer);
			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
		} else if (OSR_OK != i4Ret) {
			DMXLOG_ERROR(
				TEXT
				("[PBBUF] %s line %d fail in x_msg_q_send(pvPbBuf: 0x%x, prSdBuf: 0x%x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prSendBuffer);
			MM_RETURN(RET_DMX_OS_OPERA_FAIL);
		}
        else
        {
            //do nothing
        }
	}

	DMXLOG_DEBUG(
		TEXT("[PBBUF] %s line %d exit, Success, SrcOfst: %I64d, DataSz: %d\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prSendBuffer->u8SrcOffset,
		prSendBuffer->u4DataSize);

	MM_RETURN(RET_DMX_OK);
}

MRESULT PBBUF_ExtBuf_InitBuffer(void *pvPbBuf, u32 u4BufTotalSz,
				u32 u4SlotSz, u32 u4HdrParamSz, u8 u1PbbufType)
{
	char szNameBuf[DMX_PBBUF_THREAD_NAME_MAX_LEN];
	PBBUF_AUDIN_EXTINFO_T *prExtInfo = NULL;
	PBBUF *prPbBuf = (PBBUF *) pvPbBuf;
	SLOT *prCurr = NULL, *prNext = NULL;
	u8 *pcBufferSa = NULL;
	u32 u4SlotCnt = 0;
	u32 dwTxPriority = (u32)200;
	u32 u4Idx = 0;
	AUD_POSINFO_T rAudPos;

	mm_memset(&rAudPos, 0, sizeof(rAudPos));

	DMXLOG_DEBUG(TEXT("[PBBUF] %s enter!\r\n"), DMX_FUNC_NAME);

	mm_memset(szNameBuf, 0, sizeof(char) * DMX_PBBUF_THREAD_NAME_MAX_LEN);
	strcpy(szNameBuf, DMX_EXTPBBUF_THREAD_NAME);
	szNameBuf[5] = szNameBuf[5] + (char)(prPbBuf->u4CompID);

	if (!(prPbBuf)) {
		DMXLOG_ERROR(TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
		TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	
	if (0 == u4SlotSz) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s line %d fail for invalid args ")
			TEXT("u4BufTotalSz:0x%x, u4SlotSz: 0x%x !!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u4BufTotalSz,
			u4SlotSz);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (prPbBuf->u4CompID > DMX_MAX_PBBUF_INST_CNT) {
		DMXLOG_ERROR(
			TEXT
			("[PBBUF] %s line %d fail for Pbbuf Component ID(%d) > Max(%d) !!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf->u4CompID, DMX_MAX_PBBUF_INST_CNT);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	switch (u1PbbufType) {
#if CONFIG_DRV_HDMI_RX
	case SPT_PBUFF_AUDIN: {
			if (0 != i4AudEsm_GetAudioCodecFifoInfo(AUD_FIFO_HDMI_RX, &rAudPos)) {
				DmxLogE(DMX_MOD_OTH, DMX_MOD_OTH_LOGLVL_ESM,
					TEXT("[ESM] %s line %d fail in i4AudEsm_GetAudioCodecFifoInfo(HDMIRX)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}

			if (rAudPos.ptrAfifoVirEA <= rAudPos.ptrAfifoVirSA) {
				DMXLOG_ERROR(
					TEXT
					("[PBBUF] %s line %d fail for invalid hdmi Rx fifo(SA: 0x%08x, EA: 0x%08x) !!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					rAudPos.ptrAfifoVirSA, rAudPos.ptrAfifoVirEA);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			
			DMXLOG_TRACE(TEXT("[PBBUF] %s line %d -- ptrAfifoVirSA(0x%x), ptrAfifoVirEA(0x%x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, rAudPos.ptrAfifoVirSA, rAudPos.ptrAfifoVirEA);

			pcBufferSa = (u8 *)rAudPos.ptrAfifoVirSA;
			u4BufTotalSz = rAudPos.ptrAfifoVirEA - rAudPos.ptrAfifoVirSA;
			u4SlotCnt = u4BufTotalSz / u4SlotSz;
			if ((u4SlotCnt * u4SlotSz != u4BufTotalSz) || (u4SlotCnt < 2)) {
				DMXLOG_ERROR(
					TEXT("[PBBUF] %s line %d fail for AUDINBUF Size(%d) isn't ")
					TEXT("the multiple of SlotSz(%d) or SlotCnt < 2!!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, u4BufTotalSz, u4SlotSz);
				MM_RETURN(RET_DMX_PARAM_WRONG);
			}
			DMXLOG_TRACE(TEXT("[PBBUF] %s line %d -- AUDIN BUF SZ(%d), SlotSz(%d), SlotCnt(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, u4BufTotalSz, u4SlotSz, u4SlotCnt);
		}
		break;
#endif			/*CONFIG_DRV_HDMI_RX*/
	default:
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail for unsupport u1PbbufType: %u!!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u1PbbufType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
        break;
	}

	if (NULL == pcBufferSa) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail invalid External Pbbuf SA\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prPbBuf->u1SptPbuffType = u1PbbufType;
	prPbBuf->pcBufSa = pcBufferSa;
	prPbBuf->pcUsrBufSa = NULL;
	prPbBuf->hUsrCaller = NULL;
	prPbBuf->u4TotalSz = u4BufTotalSz;
	prPbBuf->u4SlotSz = u4SlotSz;
	prPbBuf->u4SlotAmount = u4SlotCnt;
	prPbBuf->pvSLotHdrs = NULL;
	prPbBuf->pvExtInfo = NULL;

#if !DMX_MEM_PRINT_LINE_FUNCTION
	Add_MemMap((void *) pcBufferSa,
		(void *)rAudPos.ptrAfifoSA, SPT_MEM_TYPE_EXT, u4BufTotalSz);
#else
	Add_MemMap((void *) pcBufferSa,
		(void *)rAudPos.ptrAfifoSA,
		SPT_MEM_TYPE_EXT, u4BufTotalSz,
		DMX_MIN_MEMORY_ALIGNMENT, __func__, __LINE__);
#endif			/* !DMX_MEM_PRINT_LINE_FUNCTION*/
	prPbBuf->pvExtInfo = NULL;

	switch (u1PbbufType) {
#if CONFIG_DRV_HDMI_RX
	case SPT_PBUFF_AUDIN:
		DMX_NewMemory(sizeof(PBBUF_AUDIN_EXTINFO_T), prExtInfo);
		if (NULL == prExtInfo) {
			DMXLOG_ERROR(
				TEXT("[PBBUF] %s fail in alloc pvExtInfo (no mem)\r\n"),
				DMX_FUNC_NAME);
			PBBUF_ExtBuf_DeInitBuffer(prPbBuf);
			MM_RETURN(RET_DMX_NO_MEM);
		}
		dmx_memset(prExtInfo, 0, sizeof(PBBUF_AUDIN_EXTINFO_T));
		prPbBuf->pvExtInfo = prExtInfo;
		break;
#endif				/* CONFIG_DRV_HDMI_RX*/
	default:
		PBBUF_ExtBuf_DeInitBuffer(prPbBuf);
		MM_RETURN(RET_DMX_PARAM_WRONG);
        break;
	}

	DMXLOG_TRACE(TEXT("[PBBUF] %s -- prPbBuf->u4SlotAmount = %d\r\n"),
		DMX_FUNC_NAME, prPbBuf->u4SlotAmount);

	if (0 < u4HdrParamSz) {
		DMX_NewMemory((prPbBuf->u4SlotAmount * u4HdrParamSz), prPbBuf->pvSLotHdrs);

		if (NULL == prPbBuf->pvSLotHdrs) {
			DMXLOG_ERROR(
				TEXT("[PBBUF] %s fail in alloc slots Headers (no mem)\r\n"),
				DMX_FUNC_NAME);
			PBBUF_SelfBuf_DeInitBuffer(prPbBuf);
			MM_RETURN(RET_DMX_NO_MEM);
		}

		dmx_memset(prPbBuf->pvSLotHdrs, 0, prPbBuf->u4SlotAmount * u4HdrParamSz);
	}

	DMX_NewMemory((prPbBuf->u4SlotAmount * sizeof(SLOT)), prPbBuf->prSLOT);
	if (NULL == prPbBuf->prSLOT) {
		DMXLOG_ERROR(TEXT("[PBBUF] %s fail in alloc slots tables (no mem)\r\n"),
			DMX_FUNC_NAME);
		PBBUF_ExtBuf_DeInitBuffer(prPbBuf);
		MM_RETURN(RET_DMX_NO_MEM);
	}

	dmx_memset(prPbBuf->prSLOT, 0, prPbBuf->u4SlotAmount * sizeof(SLOT));

	/*Create double linked list for SENT/ALLOCATED/READING slots.*/
	/* Now these linked list are empty.*/
	dmx_memset(prPbBuf->arSlotLists, 0, sizeof(PBBUF_SLOT_LIST_INFO_T) * PBBUF_SLOT_TYPE_CNT);

	/* 1. Initialize pcBuffer, u4BufferSize, & u4BufferHandle info.*/
	/* 2. Create double linked list for FREE slots.*/
	prPbBuf->arSlotLists[PBBUF_SLOT_FREE].prHeadSlot = prPbBuf->prSLOT;

	for (u4Idx = 0; u4Idx < prPbBuf->u4SlotAmount; u4Idx++) {
		prCurr = prPbBuf->prSLOT + u4Idx;
		/* For the last Slot, prNext is pointed to &prPbBuf->rFREE*/
		/* Initialize the three fixed information of the (SEND_BUFFER)Slot.*/
		prCurr->pcBuffer = prPbBuf->pcBufSa + (u4Idx * prPbBuf->u4SlotSz);

		prCurr->pcUsrBuffer = NULL;
		prCurr->u4BufferSize = prPbBuf->u4SlotSz;

		prCurr->rHeader.eType = PBBUF_SLOT_NORMAL;
		prCurr->rHeader.u4ParamSz = u4HdrParamSz;
		prCurr->rHeader.pvParam = NULL;

		if (0 < u4HdrParamSz) {
			prCurr->rHeader.pvParam =
				(void *) ((u8 *) (prPbBuf->pvSLotHdrs) + (u4Idx * u4HdrParamSz));
		}
		/*Create double linked list for Free Slots.*/
		if (0 == u4Idx)
			prCurr->prPrevSlot = NULL;


		if (u4Idx == (prPbBuf->u4SlotAmount - 1)) {
			prNext = NULL;
			prPbBuf->arSlotLists[PBBUF_SLOT_FREE].prTailSlot = prCurr;
		} else {
			prNext = prPbBuf->prSLOT + (u4Idx + 1);
			prNext->prPrevSlot = prCurr;
		}

		prCurr->eType = PBBUF_SLOT_FREE;
		prCurr->prNextSlot = prNext;
	}

	prPbBuf->arSlotLists[PBBUF_SLOT_FREE].u4SlotCnt = prPbBuf->u4SlotAmount;

	prPbBuf->u8LastSrcOfst = DMX_INVALID_UINT64;

	switch (u1PbbufType) {
#if CONFIG_DRV_HDMI_RX
	case SPT_PBUFF_AUDIN:
		{
			IDMXPBBUFCALLBACKS_T rPbbufCbs;
			struct task_struct *prtaskstruct = NULL;
			HANDLE hMsgQueue = NULL;
			s32 i4Ret = OSR_OK;

			i4Ret = x_msg_q_create((uintptr_t *)&hMsgQueue, TEXT("AudInDmxMsgQueue"),
		 sizeof(SEND_BUFFER), prPbBuf->u4SlotAmount);
			if (OSR_OK != i4Ret) {
				DMXLOG_ERROR(
					TEXT("[PBBUF] %s line %d fail in x_msg_q_create")
					TEXT("(TEXT(\"AudInDmxMsgQueue\")), i4Ret: %d!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, i4Ret);
				PBBUF_ExtBuf_DeInitBuffer(prPbBuf);
				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
			}

			if (NULL == hMsgQueue) {
				DMXLOG_ERROR(
					TEXT("[PBBUF] %s line %d fail in x_msg_q_create")
					TEXT("( \"AudInDmxMsgQueue\")!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				PBBUF_ExtBuf_DeInitBuffer(prPbBuf);
				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
			}

			prExtInfo->hMsgQueue = hMsgQueue;

			DMXLOG_TRACE(
				TEXT("[SPT] %s line %d -- x_msg_q_create(TEXT(\"AudInDmxMsgQueue\"),")
				TEXT(" MaxMsgCnt(%d), EachMsgSize(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf->u4SlotAmount,
				sizeof(SEND_BUFFER));

			prExtInfo->fgExitThread = FALSE;

			prtaskstruct = kthread_create(PBBUF_ReceiveDataThread, prPbBuf, szNameBuf);
			if (prtaskstruct == ERR_PTR(-ENOMEM)) {
				DMXLOG_ERROR(
					TEXT("[SPT] %s line %d fail in kthread_create(szNameBuf(%s),")
					TEXT(" pbbufIdx(%d)), prPbBuf:0x%x, NO_MEM\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, szNameBuf,
					prPbBuf->u4CompID, prPbBuf);
				prExtInfo->fgExitThread = TRUE;
				PBBUF_ExtBuf_DeInitBuffer(prPbBuf);
				MM_RETURN(RET_DMX_NO_MEM);
			} else if (IS_ERR(prtaskstruct)) {
				DMXLOG_ERROR(
					TEXT("[SPT] %s line %d fail in kthread_create(szNameBuf(%s),")
					TEXT(" pbbufIdx(%d)), prPbBuf:0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, szNameBuf,
					prPbBuf->u4CompID, prPbBuf);
				prExtInfo->fgExitThread = TRUE;
				PBBUF_ExtBuf_DeInitBuffer(prPbBuf);
				MM_RETURN(RET_DMX_OS_OPERA_FAIL);
			} else {
				struct sched_param param;
				int ret;

				mm_memset(&param, 0, sizeof(param));
				param.sched_priority = to_sched_priority(dwTxPriority);
				ret = sched_setscheduler_nocheck(prtaskstruct, SCHED_RR, &param);
				DMX_ASSERT(ret == 0);
				dwTxPriority = from_sched_priority(param.sched_priority);
			}

			init_waitqueue_head(&(prExtInfo->waitexitqueue));
			prExtInfo->waitexitqueueflag = 0;

			wake_up_process(prtaskstruct);

			prExtInfo->hThread = (HANDLE)prtaskstruct;

			mm_memset(&rPbbufCbs, 0, sizeof(IDMXPBBUFCALLBACKS_T));
			rPbbufCbs.pi4SubscribeDrvCb = PBBUF_AudIn_SubscribeDrvCb;
			rPbbufCbs.pi4UnSubscribeDrvCb = PBBUF_AudIn_UnSubscribeDrvCb;
			rPbbufCbs.pu4SendSlot = PBBUF_ExtBuf_ExtSendData;
			DMXLOG_TRACE(TEXT("[PBBUF] %s -- Audmhl_Reg_ForDemuxer\r\n"),
					DMX_FUNC_NAME);
			Audmhl_Reg_ForDemuxer(prPbBuf->pvSptHdl, &rPbbufCbs);
		}
		break;
#endif				/* CONFIG_DRV_HDMI_RX*/
	default:
		PBBUF_ExtBuf_DeInitBuffer(prPbBuf);
		MM_RETURN(RET_DMX_PARAM_WRONG);
        break;
	}

	DMXLOG_TRACE(TEXT("[PBBUF] %s exit, success\r\n"), DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_OK);
}

void PBBUF_ExtBuf_DeInitBuffer(void *pvPbBuf)
{
	PBBUF *prPbBuf = (PBBUF *) pvPbBuf;

	if (!(prPbBuf)) {
		DMX_ASSERT(FALSE);
		return;
	}

	if (prPbBuf->u4CompID > DMX_MAX_PBBUF_INST_CNT) {
		DMXLOG_ERROR(
			TEXT
			("[PBBUF] %s line %d fail for Pbbuf Component ID(%d) > Max(%d) !!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf->u4CompID, DMX_MAX_PBBUF_INST_CNT);
		return;
	}

	switch (prPbBuf->u1SptPbuffType) {
#if CONFIG_DRV_HDMI_RX
	case SPT_PBUFF_AUDIN:
		{
			SEND_BUFFER rSendBuf;
			PBBUF_AUDIN_EXTINFO_T *prExtInfo =
				(PBBUF_AUDIN_EXTINFO_T *) (prPbBuf->pvExtInfo);
			MRESULT mrRet = RET_DMX_OK;
			s32 i4Ret = OSR_OK;

			Audmhl_UnReg_ForDemuxer();

			if (NULL == prExtInfo)
				break;

			mm_memset(&rSendBuf, 0, sizeof(rSendBuf));
			rSendBuf.rHeader.eType = PBBUF_SLOT_END;

			DMXLOG_TRACE(
				TEXT("[SPT] %s line %d send END_SLOT to Ext Pbbuf receive ")
				TEXT("data thread, prPbbuf:0x%x, u4CompID: %d\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prPbBuf->u4CompID);

			mrRet = PBBUF_ExtBuf_ExtSendData(prPbBuf->pvSptHdl, &rSendBuf);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT("[SPT] %s line %d fail in PBBUF_ExtBuf_ExtSendData")
					TEXT("(END_SLOT), prPbbuf:0x%x, u4CompID: %d\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prPbBuf->u4CompID);
			}

			DMXLOG_TRACE(
				TEXT("[SPT] %s line %d -- wait Pbbuf receive data thread exit,")
				TEXT(" prPbbuf:0x%x, u4CompID: %d\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prPbBuf->u4CompID);

			if (!IS_ERR((struct task_struct *)(prExtInfo->hThread))) {
				DMXLOG_TRACE(TEXT("[PBBUF] %s line %d -- kthread_stop(PBBUF: 0x%x)\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);

				prExtInfo->waitexitqueueflag = 1;
				wake_up_interruptible(&(prExtInfo->waitexitqueue));
				kthread_stop((struct task_struct *)(prExtInfo->hThread));
				prExtInfo->hThread = NULL;
				DMXLOG_TRACE(TEXT("[PBBUF] %s line %d -- kthread_stop")
					TEXT("(PBBUF: 0x%x) success\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);
			} else {
				DMXLOG_TRACE(TEXT("[PBBUF] %s line %d -- thread task ")
					TEXT("struct is invalid, PBBUF:0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);
			}

			DMXLOG_TRACE(
				TEXT("[SPT] %s line %d -- Pbbuf receive data thread exit ")
				TEXT("success, prPbbuf:0x%x, u4CompID: %d\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prPbBuf->u4CompID);

			if (NULL != prExtInfo->hMsgQueue) {
				i4Ret = x_msg_q_delete((uintptr_t)prExtInfo->hMsgQueue);
				if (OSR_OK != i4Ret)
					DMXLOG_ERROR(
						TEXT("[PBBUF] %s line %d fail in x_msg_q_delete")
						TEXT("(Pbbuf(0x%x)'s u4CompID: %d)0x%x, u4SlotSz: 0x%x !!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf,
						prPbBuf->u4CompID);
				else
					DMXLOG_DEBUG(
						TEXT("[PBBUF] %s line %d success in x_msg_q_delete")
						TEXT("(Pbbuf(0x%x)'s u4CompID: %d)0x%x, u4SlotSz: 0x%x !!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf,
						prPbBuf->u4CompID);
				prExtInfo->hMsgQueue = NULL;
			}

			DMX_FreeMemory(prPbBuf->pvExtInfo);
			prPbBuf->pvExtInfo = NULL;
		}
#endif				/* CONFIG_DRV_HDMI_RX*/
	default:
		break;
	}

	if (NULL != prPbBuf->prSLOT) {
		DMX_FreeMemory(prPbBuf->prSLOT);
		prPbBuf->prSLOT = NULL;
	}

	if (NULL != prPbBuf->pvSLotHdrs) {
		DMX_FreeMemory(prPbBuf->pvSLotHdrs);
		prPbBuf->pvSLotHdrs = NULL;
	}

	dmx_memset(prPbBuf->arSlotLists, 0, sizeof(PBBUF_SLOT_LIST_INFO_T) * PBBUF_SLOT_TYPE_CNT);

	if (NULL != prPbBuf->pcBufSa) {
		Del_MemMap((void *) (prPbBuf->pcBufSa));
		prPbBuf->pcBufSa = NULL;
	}

	prPbBuf->hUsrCaller = NULL;
	prPbBuf->pcUsrBufSa = NULL;
	prPbBuf->u4TotalSz = 0;
	prPbBuf->u4SlotSz = 0;
	prPbBuf->u4SlotAmount = 0;
	prPbBuf->u4SlotAmount = 0;
	prPbBuf->u4MwNfyMask = 0;
	prPbBuf->u4DrvNfyMask = 0;
	prPbBuf->u1SptPbuffType = SPT_PBUFF_UNKNOWN;

}

MRESULT PBBUF_ExtBuf_SendDataSlot(void *pvPbBuf, SEND_BUFFER *prSdBuf, bool *pfgExitSent)
{
	PBBUF *prPbBuf = (PBBUF *) pvPbBuf;
	SLOT *prSlot = NULL;
	MRESULT mrRet = RET_DMX_OK;

	DMXLOG_DEBUG(TEXT("[PBBUF] %s enter\r\n"), DMX_FUNC_NAME);

	if (NULL == prSdBuf) {
		DMXLOG_ERROR(
			TEXT
			("[PBBUF] %s line %d fail for invalid args (pvPbBuf: 0x%x, prSdBuf: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvPbBuf, prSdBuf);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!(prPbBuf)) {
		DMXLOG_ERROR(TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
		TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

#if CONFIG_DRV_HDMI_RX
#if HDMI_AUDIN_USE_SESSIONID
	if (SPT_PBUFF_AUDIN == prPbBuf->u1SptPbuffType) {
		PBBUF_AUDIN_EXTINFO_T *prExtInfo = (PBBUF_AUDIN_EXTINFO_T *) (prPbBuf->pvExtInfo);

		if (NULL == prExtInfo) {
			DMXLOG_ERROR(
				TEXT
				("[PBBUF] %s line %d fail for prPbBuf(0x%x)'s pvExtInfo == NULL\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		if (NULL == prExtInfo->rAudInCbsInfo.pu4GetPsrSessionID) {
			DMXLOG_ERROR(
				TEXT
				("[PBBUF] %s line %d fail for prPbBuf(0x%x)'s pu4GetPsrSessionID == NULL\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);
			MM_RETURN(RET_DMX_NO_INIT);
		}

		if (prExtInfo->rAudInCbsInfo.pu4GetPsrSessionID() != prSdBuf->u4SessionID) {
			DMXLOG_ERROR(
				TEXT
				("[PBBUF] %s line %d -- SessionID(%d) != AudIn's SessionID(%d),")
				TEXT(" discard this SendBuffer(u8SrcOfst=0x%llx, PlaySize=0x%x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSdBuf->u4SessionID,
				prExtInfo->rAudInCbsInfo.pu4GetPsrSessionID(),
				prSdBuf->u8SrcOffset, prSdBuf->u4PlaySize);
			MM_RETURN(RET_DMX_OK);
		}
	}
#endif				/* HDMI_AUDIN_USE_SESSIONID*/
#endif				/* CONFIG_DRV_HDMI_RX*/

	prSlot = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_FREE);
	if (NULL == prSlot) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail in PBBUF_GetSlotFromHead(prPbbuf(0x%x),")
			TEXT("PBBUF_SLOT_FREE), mrRet = 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	mrRet = PBBUF_RemoveSlot(prPbBuf, prSlot);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail in PBBUF_RemoveSlot, mrRet = 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	prSlot->pcPlayBuffer = prSdBuf->pcBuffer;
	prSlot->u4DataOffset = prSdBuf->u4DataOffset;
	prSlot->u4DataSize = prSdBuf->u4DataSize;
	prSlot->u4PlayOffset = prSdBuf->u4PlayOffset;
	prSlot->u4PlaySize = prSdBuf->u4PlaySize;
	prSlot->u8SrcOffset = prSdBuf->u8SrcOffset;
	prSlot->rHeader.eType = PBBUF_SLOT_NORMAL;
	prSlot->pcUsrBuffer = NULL;
	prSlot->eType = PBBUF_SLOT_SEND;

	prSlot->u4SessionID = prSdBuf->u4SessionID;

	mrRet = PBBUF_AddSlotToTail(prPbBuf, prSlot, PBBUF_SLOT_SEND);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail in PBBUF_RemoveSlot, mrRet = 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}
	/*DMXLOG_TRACE(TEXT("[PBBUF] %s line %d exit, */
	/*Success, SrcOfst: %I64d, DataOfst: %d, DataSz: %d, DrvNfyMask: 0x%x\r\n"),*/
	/*	  DMX_FUNC_NAME, DMX_LINE_NO,*/
	/*	  prSlot->u8SrcOffset, prSdBuf->u4DataOffset, prSlot->u4DataSize,*/
	/*	  prPbBuf->u4DrvNfyMask);*/

	PBBUF_ReadyToGetReadBuffer(prPbBuf, prSlot);

	DMXLOG_DEBUG(
		TEXT("[PBBUF] %s line %d exit, Success, SrcOfst: %I64d, DataSz: %d\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prSlot->u8SrcOffset, prSlot->u4DataSize);

	MM_RETURN(RET_DMX_OK);
}

MRESULT PBBUF_ExtBuf_CleanAllSlots(void *pvPbBuf)
{
	PBBUF *prPbBuf = (PBBUF *) pvPbBuf;
	SLOT *prCurr = NULL;
	SLOT *prNext = NULL;
	u32 u4Idx = 0;

	if (!(prPbBuf)) {
		DMXLOG_ERROR(TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
		TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

#if 1

	prPbBuf->arSlotLists[PBBUF_SLOT_FREE].prHeadSlot = prPbBuf->prSLOT;

	for (u4Idx = 0; u4Idx < prPbBuf->u4SlotAmount; u4Idx++) {
		prCurr = prPbBuf->prSLOT + u4Idx;

		prCurr->rHeader.eType = PBBUF_SLOT_NORMAL;

		/* Create double linked list for Free Slots.*/
		if (u4Idx == (prPbBuf->u4SlotAmount - 1)) {
			prNext = NULL;
			prPbBuf->arSlotLists[PBBUF_SLOT_FREE].prTailSlot = prCurr;
		} else {
			prNext = prPbBuf->prSLOT + (u4Idx + 1);
			prNext->prPrevSlot = prCurr;
		}

		prCurr->prNextSlot = prNext;
		prCurr->eType = PBBUF_SLOT_FREE;
	}

	if (NULL != prPbBuf->arSlotLists[PBBUF_SLOT_FREE].prHeadSlot)
		prPbBuf->arSlotLists[PBBUF_SLOT_FREE].prHeadSlot->prPrevSlot = NULL;


	prPbBuf->arSlotLists[PBBUF_SLOT_FREE].u4SlotCnt = prPbBuf->u4SlotAmount;

	prPbBuf->u8LastSrcOfst = DMX_INVALID_UINT64;

	dmx_memset(&(prPbBuf->arSlotLists[PBBUF_SLOT_ALLOCATED]), 0,
		sizeof(PBBUF_SLOT_LIST_INFO_T));
	dmx_memset(&(prPbBuf->arSlotLists[PBBUF_SLOT_READING]), 0, sizeof(PBBUF_SLOT_LIST_INFO_T));
	dmx_memset(&(prPbBuf->arSlotLists[PBBUF_SLOT_SEND]), 0, sizeof(PBBUF_SLOT_LIST_INFO_T));

#if DMX_PRINT_PBBUF_DEBUG_LOG
	DMXLOG_TRACE(TEXT("[GAU] %s -- Clean PBBUF_COND_CANCEL_ALLOCATE_BUFFER!!!\r\n"),
		DMX_FUNC_NAME);
#endif			/* DMX_PRINT_PBBUF_DEBUG_LOG*/

	PBBUFClrNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_CANCEL_ALLOCATE_BUFFER);

#else

	if (0 == PBBUF_GetListCount(prPbBuf, PBBUF_SLOT_READING)) {
		MRESULT mrRet = RET_DMX_OK;

		prCurr = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_SEND);
		while (NULL != prCurr) {
			mrRet = PBBUF_RemoveSlot(prPbBuf, prCurr);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT("[PBBUF] %s line %d fail in PBBUF_RemoveSlot")
					TEXT("(prPbBuf(0x%x), prCurr(0x%x), PBBUF_SLOT_SEND),")
					TEXT(" mrRet = 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prCurr, mrRet);
				MM_RETURN(mrRet);
			}
			prCurr->rHeader.eType = PBBUF_SLOT_NORMAL;
			mrRet = PBBUF_AddSlotToTail(prPbBuf, prCurr, PBBUF_SLOT_FREE);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail")
					TEXT("(prPbBuf(0x%x), prCurr(0x%x), PBBUF_SLOT_FREE),")
					TEXT(" mrRet = 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prCurr, mrRet);
				MM_RETURN(mrRet);
			}
			prCurr = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_SEND);
		}

		prCurr = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_ALLOCATED);
		while (NULL != prCurr) {
			mrRet = PBBUF_RemoveSlot(prPbBuf, prCurr);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT("[PBBUF] %s line %d fail in PBBUF_RemoveSlot")
					TEXT("(prPbBuf(0x%x), prCurr(0x%x), PBBUF_SLOT_ALLOCATED),")
					TEXT(" mrRet = 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prCurr, mrRet);
				MM_RETURN(mrRet);
			}
			prCurr->rHeader.eType = PBBUF_SLOT_NORMAL;
			mrRet = PBBUF_AddSlotToTail(prPbBuf, prCurr, PBBUF_SLOT_FREE);
			if (DMX_FAILED(mrRet)) {
				DMXLOG_ERROR(
					TEXT("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail")
					TEXT("(prPbBuf(0x%x), prCurr(0x%x), PBBUF_SLOT_FREE),")
					TEXT("mrRet = 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf, prCurr, mrRet);
				MM_RETURN(mrRet);
			}
			prCurr = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_ALLOCATED);
		}

#if DMX_PRINT_PBBUF_DEBUG_LOG
		DMXLOG_TRACE(
			TEXT("[GAU] %s -- Clean PBBUF_COND_CANCEL_ALLOCATE_BUFFER!!!\r\n"),
			DMX_FUNC_NAME);
#endif			/* DMX_PRINT_PBBUF_DEBUG_LOG*/

		PBBUFClrNfyMask(prPbBuf->u4MwNfyMask, PBBUF_COND_CANCEL_ALLOCATE_BUFFER);

		prPbBuf->u8LastSrcOfst = DMX_INVALID_UINT64;
	} else {
		PSR_CC_PBBuf_Notify(prPbBuf->pvDrvOwner, DRV_PBBUF_COND_RELEASE_ALL_SLOTS, 0);
		PBBUFSetNfyMask(&prPbBuf->u4MwNfyMask, PBBUF_COND_BUFFER_CLEANED);

	}

#endif

	MM_RETURN(RET_DMX_OK);
}

MRESULT PBBUF_ExtBuf_GetAvailDataSlot(void *pvPbBuf, DMX_READ_BUFFER *prRdBuf)
{
	PBBUF *prPbBuf = (PBBUF *) pvPbBuf;
	SLOT *prSlot = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (!(prPbBuf)) {
		DMXLOG_ERROR(TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
		TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

#if CONFIG_DRV_HDMI_RX
#if HDMI_AUDIN_USE_SESSIONID
	if (SPT_PBUFF_AUDIN == prPbBuf->u1SptPbuffType) {
		prSlot = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_SEND);

		while (NULL != prSlot) {
			PBBUF_AUDIN_EXTINFO_T *prExtInfo =
				(PBBUF_AUDIN_EXTINFO_T *) (prPbBuf->pvExtInfo);

			if (NULL == prExtInfo) {
				DMXLOG_ERROR(
					TEXT
					("[PBBUF] %s line %d fail for prPbBuf(0x%x)'s pvExtInfo == NULL\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);
				MM_RETURN(RET_DMX_UNEXPECT);
			}

			if (NULL == prExtInfo->rAudInCbsInfo.pu4GetPsrSessionID) {
				DMXLOG_ERROR(
					TEXT("[PBBUF] %s line %d fail for prPbBuf(0x%x)'s ")
					TEXT("rAudInCbsInfo.pvUpdateRpCb == NULL or ")
					TEXT("pu4GetPsrSessionID == NULL\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);
				MM_RETURN(RET_DMX_NO_INIT);
			}

			if (prExtInfo->rAudInCbsInfo.pu4GetPsrSessionID() != prSlot->u4SessionID) {
				DMXLOG_TRACE(
					TEXT("[PBBUF] %s line %d -- Slot's SessionID(%d) != AudIn's")
					TEXT(" SessionID(%d), so move this slot(u8SrcOfst=0x%llx, ")
					TEXT("u4DataSize=0x%x) from SendList to FreeList\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSlot->u4SessionID,
					prExtInfo->rAudInCbsInfo.pu4GetPsrSessionID(),
					prSlot->u8SrcOffset, prSlot->u4DataSize);

				/* Remove the slot from SENT linked list.*/
				mrRet = PBBUF_RemoveSlot(prPbBuf, prSlot);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(
						TEXT("[PBBUF] %s line %d fail in PBBUF_RemoveSlot,")
						TEXT(" mrRet: 0x%x\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
					MM_RETURN(mrRet);
				}

				prPbBuf->u8LastSrcOfst = prSlot->u8SrcOffset + prSlot->u4DataSize;

				prSlot->rHeader.eType = PBBUF_SLOT_NORMAL;

				mrRet = PBBUF_AddSlotToTail(prPbBuf, prSlot, PBBUF_SLOT_FREE);
				if (DMX_FAILED(mrRet)) {
					DMXLOG_ERROR(
						TEXT("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail,")
						TEXT(" mrRet: 0x%x\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
					MM_RETURN(mrRet);
				}

				prSlot = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_SEND);
			} else {
				break;
			}
		}
	} else
#endif				/* HDMI_AUDIN_USE_SESSIONID*/
#endif				/* CONFIG_DRV_HDMI_RX*/
	{
		prSlot = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_SEND);
	}

	if (NULL == prSlot) {
		DMXLOG_DEBUG(
			TEXT
			("[PBBUF] +++++++++++ Pbbuf Send List Empty (hPsr: 0x%x) +++++++++++ \r\n"),
			prPbBuf->pvDrvOwner);
		PBBUFSetNfyMask(prPbBuf->u4DrvNfyMask, DRV_PBBUF_COND_READY_TO_GET_READ_BUFFER);
		MM_RETURN(RET_DMX_PBBUF_BUSY);
	}
#if CONFIG_DRV_HDMI_RX
	prSlot->rHeader.eType = PBBUF_SLOT_NORMAL;
#if HDMI_AUDIN_USE_SESSIONID
	if (DMX_INVALID_UINT64 != prPbBuf->u8LastSrcOfst) {
		if (0 != prSlot->u8SrcOffset) {
			DMXLOG_ERROR(
				TEXT("[PBBUF] %s line %d fail for the Session(ID: %d)'s first")
				TEXT(" Slot's u8SrcOfst(0x%llx) should be 0, u4DataSize=0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSlot->u4SessionID,
				prSlot->u8SrcOffset, prSlot->u4DataSize);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		if ((NULL != prSlot->rHeader.pvParam) &&
			(sizeof(CfaAudInPR) == prSlot->rHeader.u4ParamSz)) {
			CfaAudInPR *prCfaAudInRng = NULL;

			prCfaAudInRng = (CfaAudInPR *) (prSlot->rHeader.pvParam);
			prCfaAudInRng->u8Sa = 0;
			prCfaAudInRng->u8Ea = DMX_INVALID_UINT64;
			prSlot->rHeader.eType = PBBUF_SLOT_JUMP;
			DMXLOG_TRACE(
				TEXT
				("[PBBUF] %s line %d -- Set JumpRange to Slot's rHeader, Session(ID: %d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSlot->u4SessionID);
		}
		prPbBuf->u8LastSrcOfst = DMX_INVALID_UINT64;
	}
#endif				/* HDMI_AUDIN_USE_SESSIONID*/
#endif				/* CONFIG_DRV_HDMI_RX*/

	/* Remove the slot from SENT linked list.*/
	mrRet = PBBUF_RemoveSlot(prPbBuf, prSlot);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail in PBBUF_RemoveSlot, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}

	prRdBuf->fgFollowedByIbc = FALSE;
	prRdBuf->pcPlayBuffer = prSlot->pcPlayBuffer + prSlot->u4DataOffset;
	prRdBuf->u4DataSize = prSlot->u4DataSize;
	prRdBuf->u4PlayOffset = prSlot->u4PlayOffset;
	prRdBuf->u4PlaySize = prSlot->u4PlaySize;
	prRdBuf->pvSlot = (void *) prSlot;
	prRdBuf->u8SrcOffset = prSlot->u8SrcOffset;
	prRdBuf->u4BufferSize = prSlot->u4BufferSize;

	dmx_memcpy(&(prRdBuf->rHeader), &(prSlot->rHeader), sizeof(PBBUF_SLOT_HEADER_INFO_T));

	/* DMXLOG_TRACE(TEXT("[PBBUF] %s line %d exit, */
	/*Success, SrcOfst: %I64d, DataOfst: %d, DataSz: %d\r\n"),*/
	/*	  DMX_FUNC_NAME, DMX_LINE_NO, prRdBuf->u8SrcOffset,*/
	/*	  prRdBuf->u4PlayOffset, prRdBuf->u4DataSize);*/

	/*Add it to READING linked list.*/
	mrRet = PBBUF_AddSlotToTail(prPbBuf, prSlot, PBBUF_SLOT_READING);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT PBBUF_ExtBuf_ReleaseNoUseSlot(void *pvPbBuf, DMX_READ_BUFFER *prRdBuf)
{
	PBBUF *prPbBuf = (PBBUF *) pvPbBuf;
	SLOT *prSlot = NULL;
	uintptr_t pvSlotDataEa = 0;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prRdBuf) || (NULL == prRdBuf->pvSlot)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail for invalid args (pvPbBuf: 0x%x,")
			TEXT(" prRdBuf: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvPbBuf, prRdBuf);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}


	if (!(prPbBuf)) {
		DMXLOG_ERROR(TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
		TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prSlot = PBBUF_GetSlotByHandle(prPbBuf, prRdBuf->pvSlot);

	if (NULL == prSlot) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail for can't find the reading slot ")
			TEXT("whose handle = 0x%x, (pvSptHdl: 0x%x, prPbBuf: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prRdBuf->pvSlot, pvPbBuf, prPbBuf);
		MM_RETURN(RET_DMX_NOT_FOUND);
	}

	pvSlotDataEa = (uintptr_t) (prSlot->pcPlayBuffer) + prSlot->u4PlaySize;
	if ((pvSlotDataEa < (uintptr_t) (prPbBuf->pcBufSa)) ||
		(pvSlotDataEa > (uintptr_t) (prPbBuf->pcBufSa) + prPbBuf->u4TotalSz)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail for this slot's end address(0x%p)")
			TEXT(" exceed the pbbuf range[0x%p, 0x%p)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, (void *)pvSlotDataEa, (void *)(prPbBuf->pcBufSa),
			((void *)(prPbBuf->pcBufSa + prPbBuf->u4TotalSz)));
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (pvSlotDataEa == (uintptr_t) (prPbBuf->pcBufSa) + prPbBuf->u4TotalSz)
		pvSlotDataEa = (uintptr_t) (prPbBuf->pcBufSa);

	/*DMXLOG_TRACE(TEXT("[PBBUF] %s line %d -- ReleaseSlot, */
	/*SrcOfst: %I64d, DataOfst: %d, DataSz: %d, SlotDataEa: 0x%x\r\n"),*/
	/*	  DMX_FUNC_NAME, DMX_LINE_NO, prRdBuf->u8SrcOffset,*/
	/*	  prRdBuf->u4PlayOffset, prRdBuf->u4DataSize, u4SlotDataEa);*/

	/* Remove from READING linked list. Add the slot to FREE linked list.*/
	mrRet = PBBUF_RemoveSlot(prPbBuf, prSlot);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail in PBBUF_RemoveSlot, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}

	prSlot->rHeader.eType = PBBUF_SLOT_NORMAL;

	mrRet = PBBUF_AddSlotToTail(prPbBuf, prSlot, PBBUF_SLOT_FREE);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}
#if CONFIG_DRV_HDMI_RX
	if (SPT_PBUFF_AUDIN == prPbBuf->u1SptPbuffType) {
		PBBUF_AUDIN_EXTINFO_T *prExtInfo = (PBBUF_AUDIN_EXTINFO_T *) (prPbBuf->pvExtInfo);

		if (NULL == prExtInfo) {
			DMXLOG_ERROR(
				TEXT
				("[PBBUF] %s line %d fail for prPbBuf(0x%x)'s pvExtInfo == NULL\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		if ((NULL == prExtInfo->rAudInCbsInfo.pvUpdateRpCb) ||
			(NULL == prExtInfo->rAudInCbsInfo.pu4GetPsrSessionID)) {
			DMXLOG_ERROR(
				TEXT("[PBBUF] %s line %d fail for prPbBuf(0x%x)'s ")
				TEXT("rAudInCbsInfo.pvUpdateRpCb == NULL or ")
				TEXT("pu4GetPsrSessionID == NULL\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);
			MM_RETURN(RET_DMX_NO_INIT);
		}
#if HDMI_AUDIN_USE_SESSIONID
		if (prExtInfo->rAudInCbsInfo.pu4GetPsrSessionID() != prSlot->u4SessionID) {
			DMXLOG_TRACE(
				TEXT("[PBBUF] %s line %d -- Slot's SessionID(%d) != ")
				TEXT("AudIn's SessionID(%d), Slot's(u8SrcOfst=0x%llx, ")
				TEXT("u4DataSize=0x%x)so don't change AudInPosInfo's u4VirRP\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSlot->u4SessionID,
				prExtInfo->rAudInCbsInfo.pu4GetPsrSessionID(),
				prSlot->u8SrcOffset, prSlot->u4DataSize);
		} else
#endif				/* HDMI_AUDIN_USE_SESSIONID*/
		{
			prExtInfo->rAudInCbsInfo.pvUpdateRpCb(pvSlotDataEa);
		}
	}
#endif				/*CONFIG_DRV_HDMI_RX*/

	mrRet = PBBUF_ReadyToGetAllocSlot(prPbBuf);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT
			("[PBBUF] %s line %d fail in PBBUF_ReadyToGetAllocSlot, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}

	mrRet = PBBUF_BufferCleaned(prPbBuf);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail in PBBUF_BufferCleaned, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);

		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT PBBUF_ExtBuf_CancelReadSlot(void *pvPbBuf)
{
	PBBUF *prPbBuf = (PBBUF *) pvPbBuf;

	if (!(prPbBuf)) {
		DMXLOG_ERROR(TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
		TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	PBBUFClrNfyMask(prPbBuf->u4DrvNfyMask, DRV_PBBUF_COND_READY_TO_GET_READ_BUFFER);

	MM_RETURN(RET_DMX_OK);
}

MRESULT PBBUF_ExtBuf_RelFrmSlotToUnCon(void *pvPbBuf, DMX_READ_BUFFER *prRdBuf,
	 bool *pfgExistUnCon)
{
	PBBUF *prPbBuf = (PBBUF *) pvPbBuf;
	SLOT *prSlot = NULL;
	MRESULT mrRet = RET_DMX_OK;
	u64 u8SrcOfst = DMX_INVALID_UINT64;
	bool fgFindUnCon = FALSE;

	if ((NULL == prRdBuf) || (NULL == prRdBuf->pvSlot)) {
		DMXLOG_ERROR(
			TEXT
			("[PBBUF] %s fail for invalid args (pvPbBuf: 0x%x, prRdBuf: 0x%x)\r\n"),
			DMX_FUNC_NAME, pvPbBuf, prRdBuf);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!(prPbBuf)) {
		DMXLOG_ERROR(TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
		TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prSlot = PBBUF_GetSlotByHandle(prPbBuf, prRdBuf->pvSlot);

	if (NULL == prSlot) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s fail for can't find the reading slot whose ")
			TEXT("handle = 0x%x, (pvPbBuf: 0x%x, prPbBuf: 0x%x)\r\n"),
			DMX_FUNC_NAME, prRdBuf->pvSlot, pvPbBuf, prPbBuf);
		MM_RETURN(RET_DMX_NOT_FOUND);
	}

	u8SrcOfst = prSlot->u8SrcOffset + prSlot->u4DataSize;

	/* Remove from READING linked list. Add the slot to FREE linked list.*/
	mrRet = PBBUF_RemoveSlot(prPbBuf, prSlot);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail in PBBUF_RemoveSlot, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}

	prSlot->rHeader.eType = PBBUF_SLOT_NORMAL;

	mrRet = PBBUF_AddSlotToTail(prPbBuf, prSlot, PBBUF_SLOT_FREE);
	if (DMX_FAILED(mrRet)) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}

	prSlot = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_SEND);
	while (NULL != prSlot) {
		if (prSlot->u8SrcOffset != u8SrcOfst) {
			fgFindUnCon = TRUE;
			break;
		}

		u8SrcOfst = prSlot->u8SrcOffset + prSlot->u4DataSize;

		/* Remove the slot from Sent linked list.*/
		mrRet = PBBUF_RemoveSlot(prPbBuf, prSlot);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT
				("[PBBUF] %s line %d fail in PBBUF_RemoveSlot, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			MM_RETURN(mrRet);
		}
		/* Add the slot to the Allocated Linked list.*/
		mrRet = PBBUF_AddSlotToTail(prPbBuf, prSlot, PBBUF_SLOT_FREE);
		if (DMX_FAILED(mrRet)) {
			DMXLOG_ERROR(
				TEXT
				("[PBBUF] %s line %d fail in PBBUF_AddSlotToTail, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			MM_RETURN(mrRet);
		}

		prSlot = PBBUF_GetSlotFromHead(prPbBuf, PBBUF_SLOT_SEND);
	}

	if (!fgFindUnCon) {
		DMXLOG_DEBUG(TEXT("[PBBUF] %s line %d -- LastSrcOfst: %I64d\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, u8SrcOfst);
		prPbBuf->u8LastSrcOfst = u8SrcOfst;
		PBBUFSetNfyMask(prPbBuf->u4DrvNfyMask, DRV_PBBUF_COND_READY_TO_GET_READ_BUFFER);
	} else {
		prPbBuf->u8LastSrcOfst = DMX_INVALID_UINT64;
	}

	if (NULL != pfgExistUnCon)
		*pfgExistUnCon = fgFindUnCon;

	/*DMXLOG_TRACE(TEXT("[PBBUF] -- %s line %d Dump Pbbuf Info Before Exit\r\n"), */
	/*DMX_FUNC_NAME, DMX_LINE_NO);*/
	/*PBBUF_DumpInfoEx(pvSptHdl, FALSE);*/
	DMXLOG_DEBUG(TEXT("[PBBUF] -- %s line %d Exit\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);

	MM_RETURN(RET_DMX_OK);
}

#if  CONFIG_DRV_HDMI_RX
MRESULT PBBUF_ExtBuf_GetAudInParsingInfo(void *hPBBuf, AUDIN_PARSING_INFO_T *prAudinPsringInfo)
{
	PBBUF *prPbBuf = (PBBUF *) hPBBuf;
	PBBUF_AUDIN_EXTINFO_T *prExtInfo = NULL;


	if (!(prPbBuf)) {
		DMXLOG_ERROR(TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
		TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prExtInfo = (PBBUF_AUDIN_EXTINFO_T *) (prPbBuf->pvExtInfo);


	if (!(prPbBuf)) {
		DMXLOG_ERROR(TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
		TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (NULL == prExtInfo->rAudInCbsInfo.pvGetAudInParsingInfo) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail for PbBuf(0x%lx)'s ")
			TEXT("rAudInCbsInfo.pvGetAudInParsingInfo == NULL\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	prExtInfo->rAudInCbsInfo.pvGetAudInParsingInfo((void *)prAudinPsringInfo);

	MM_RETURN(RET_DMX_OK);
}

MRESULT PBBUF_ExtBuf_AudInIsRAW(void *hPBBuf, bool *pfgAudInIsRaw)
{
	PBBUF *prPbBuf = (PBBUF *) hPBBuf;
	PBBUF_AUDIN_EXTINFO_T *prExtInfo = NULL;

	if (!(prPbBuf)) {
		DMXLOG_ERROR(TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
		TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!(pfgAudInIsRaw)) {
		DMXLOG_ERROR(TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
		TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prExtInfo = (PBBUF_AUDIN_EXTINFO_T *) (prPbBuf->pvExtInfo);

	if (!(prExtInfo)) {
		DMXLOG_ERROR(TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
		TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	*pfgAudInIsRaw = FALSE;

	if (NULL == prExtInfo->rAudInCbsInfo.pfgAudInIsRaw) {
		DMXLOG_ERROR(
			TEXT("[PBBUF] %s line %d fail for PbBuf(0x%lx)'s ")
			TEXT("rAudInCbsInfo.pfgAudInIsRaw == NULL\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prPbBuf);
		MM_RETURN(RET_DMX_NO_INIT);
	}

	*pfgAudInIsRaw = prExtInfo->rAudInCbsInfo.pfgAudInIsRaw();

	MM_RETURN(RET_DMX_OK);
}

u32 PBBUF_ExtBuf_AudInGetSessionID(void *hPBBuf)
{
	PBBUF *prPbBuf = (PBBUF *) hPBBuf;
	PBBUF_AUDIN_EXTINFO_T *prExtInfo = NULL;

	if (!(NULL != prPbBuf)) {
		DMXLOG_ERROR(TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
			TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prExtInfo = (PBBUF_AUDIN_EXTINFO_T *) (prPbBuf->pvExtInfo);

	if (!(prExtInfo)) {
		DMXLOG_ERROR(TEXT("DBGCHK Failed:%s at line %d in %s\r\n"),
		TEXT(__func__), __LINE__, TEXT(__FILE__));
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (NULL == prExtInfo->rAudInCbsInfo.pu4GetPsrSessionID)
		return DMX_INVALID_UINT32;


	return prExtInfo->rAudInCbsInfo.pu4GetPsrSessionID();
}


#endif				/* CONFIG_DRV_HDMI_RX*/

