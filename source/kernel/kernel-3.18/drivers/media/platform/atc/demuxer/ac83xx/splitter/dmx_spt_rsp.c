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
 * @file dmx_spt_rsp.c
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
#endif				/* __linux__ */

#include <media/atc/dmx_define.h>
#include <media/atc/dmx_splitter.h>
#include <media/atc/drv_esm_if.h>
#include <media/atc/dmx_cfa_def.h>
#include <media/atc/dmx_decrypt.h>
#include "dmx_def.h"
#include "dmx_mem.h"
#include "dmx_spt.h"
#include "dmx_dump.h"
#include "dmx_cli.h"
#include "dmx_spt_os.h"
#include "dmx_spt_cfa.h"
#include "dmx_spt_rsp.h"
#include "dmx_spt_main.h"
#include "dmx_psr_cc.h"
#include "dmx_stream.h"
#include "dmx_spt_psr.h"
#include "dmx_psr_filter.h"
#include "dmx_parser.h"
#include "dmx_psr_pbbuf.h"
#include "dmx_psr_esm.h"
#include "dmx_gau_if.h"
#include "dmx_cpsa.h"
#include "cfa_if.h"
/* #include <media/atc/mm_debug.h> */

#ifndef __linux__
#pragma warning(disable : 4127)
#endif

EXTERN DMX_DUMP_MAN_T g_rDmxDumpMan;
EXTERN DMX_CLI_MAN_T g_rDmxCliMan;

static void SplitterRspFree(void *pvSptHdl, bool fgFromStart);
static MRESULT SplitterRspNew(void *pvSptHdl);
static void *SplitterRspGetRsp(void *pvSptHdl);
static void *SplitterRspGetAu(void *pvSptHdl);

/* ///////////////////////////////////////////////////////////////////////////// */
/* HDR_Init */
/* Audio Hdr manager structure initializing */
/* @return      TRUE  if we do success */
/* @return      FALSE if we do failure */
/* ///////////////////////////////////////////////////////////////////////////// */
static MRESULT RspSampleHdrInit(DMX_SPT_INST_T *prSpt)
{
	RSP_HDR_MEM_LIST *prRspHdrMemList = NULL;

	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s line %d fail for invalid args!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (NULL == prSpt->pvSampleHdrBufSa) {
#ifdef __linux__
		DMX_NewHwMemory(RSP_SAMPHDR_LEN_MAX, prSpt->pvSampleHdrBufSa);
#else
		DMX_NewHwMemory(RSP_SAMPHDR_LEN_MAX, (void *) (prSpt->pvSampleHdrBufSa));
#endif				/* #ifdef __linux__ */
		if (NULL == prSpt->pvSampleHdrBufSa) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
				TEXT("[SPT] %s fail in alloc Sample Header Buffer!! \r\n"),
				DMX_FUNC_NAME);
			MM_RETURN(RET_DMX_NO_MEM);
		}

		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s alloc pvSampleHdrBufSa success!! \r\n"), DMX_FUNC_NAME);
		dmx_memset((void *) (prSpt->pvSampleHdrBufSa), 0, RSP_SAMPHDR_LEN_MAX);
	}

	dmx_memset(&(prSpt->rHdrMemList), 0, sizeof(RSP_HDR_MEM_LIST));

	prRspHdrMemList = &(prSpt->rHdrMemList);

	prRspHdrMemList->pvSa = prSpt->pvSampleHdrBufSa;
	prRspHdrMemList->pvEa = prSpt->pvSampleHdrBufSa + RSP_SAMPHDR_LEN_MAX;
	prRspHdrMemList->pvHdrRp = prRspHdrMemList->pvSa;
	prRspHdrMemList->pvHdrWp = prRspHdrMemList->pvSa;

	prRspHdrMemList->prHead = NULL;
	prRspHdrMemList->prTail = NULL;

	MM_RETURN(RET_DMX_OK);
}


/* ///////////////////////////////////////////////////////////////////////////// */
/* HDR_Uninit */
/* Audio Hdr manager structure uninitializing */
/* ///////////////////////////////////////////////////////////////////////////// */
static MRESULT RspSampleHdrUninit(DMX_SPT_INST_T *prSpt)
{
	RSP_HDR_MEM_NODE *prNode;

	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s line %d fail for invalid args!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prNode = prSpt->rHdrMemList.prHead;

	DMX_ASSERT(NULL == prSpt->rHdrMemList.prHead);

	dmx_memset(&(prSpt->rHdrMemList), 0, sizeof(RSP_HDR_MEM_LIST));

	if (NULL != prSpt->pvSampleHdrBufSa) {
		DMX_FreeHwMemory(prSpt->pvSampleHdrBufSa);
		prSpt->pvSampleHdrBufSa = NULL;
	}

	MM_RETURN(RET_DMX_OK);
}


/* ////////////////////////////////////////////////////////////////////////////// */
/* HDR_AllocNode */
/* Alloc a u4Size of memory in the prSpt->pvSampleHdrBufSa, we also use a struct to */
/* record the addr and length. We will manage the node with a bidirectional */
/* circulation link */
/* @Param u4Size [IN] To alloc a u4Size of memory in the prSpt->pvSampleHdrBufSa */
/* @return      If success, return the Rsp Header Mem Node's pointer; */
/* otherwise, return NULL */
/* ////////////////////////////////////////////////////////////////////////////// */
static MRESULT RspSampleHdrAllocNode(void *pvSptHdl,
	RSP_HDR_MEM_NODE *prNode, u32 u4Size)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;
	RSP_HDR_MEM_LIST *prHdrMemList = NULL;
	void *pvHdrDataEa = NULL;

	if ((0 == u4Size) || (NULL == prSpt) || (NULL == prNode)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for invalid args(u4Size: %d, ")
			TEXT("prSpt: 0x%p, prNode: %p)\r\n"),
			DMX_FUNC_NAME, u4Size, prSpt, prNode);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prHdrMemList = &(prSpt->rHdrMemList);

	if (prHdrMemList->pvHdrWp >= prHdrMemList->pvEa) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for the sample header buffer' write ")
			TEXT("pointer[0x%08x] is beyond the range[0x%08x, 0x%08x]\r\n"),
			DMX_FUNC_NAME, prHdrMemList->pvHdrWp, prHdrMemList->pvSa,
			prHdrMemList->pvEa);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (prHdrMemList->pvHdrWp < prHdrMemList->pvHdrRp) {
		/* when  Wp < Rp */
		pvHdrDataEa = prHdrMemList->pvHdrWp + u4Size;
		if (pvHdrDataEa >= prHdrMemList->pvHdrRp) {
			/* SampleHdrBuf Full */
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s fail for Sample Header Buffer Full, ")
				TEXT("u4HdrWp(0x%p) + u4Size(0x%x) >= u4HdrRp(0x%p)\r\n"),
				DMX_FUNC_NAME, prHdrMemList->pvHdrWp, u4Size,
				prHdrMemList->pvHdrRp);
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_OVER_LIMIT);
		}
	} else {			/* when Rp <= Wp */
		if (prHdrMemList->pvHdrWp + u4Size >= prHdrMemList->pvEa) {
			if (prHdrMemList->pvSa + u4Size >= prHdrMemList->pvHdrRp) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s fail for Sample Header Buffer Full, ")
					TEXT("u4Sa(0x%p) + u4HdrSize(%d) >= u4HdrRp(0x%x), ")
					TEXT("u4HdrWp(0x%p), u4Ea(0x%p)\r\n"),
					DMX_FUNC_NAME, prHdrMemList->pvSa, u4Size,
					prHdrMemList->pvHdrRp, prHdrMemList->pvHdrWp,
					prHdrMemList->pvEa);
				DMX_ASSERT(FALSE);
				MM_RETURN(RET_DMX_OVER_LIMIT);
			}
			prHdrMemList->pvHdrWp = prHdrMemList->pvSa;
			pvHdrDataEa = prHdrMemList->pvSa + u4Size;
		} else {
			pvHdrDataEa = prHdrMemList->pvHdrWp + u4Size;
		}
	}

	dmx_memset(prNode, 0, sizeof(RSP_HDR_MEM_NODE));

	prNode->prNext = NULL;
	prNode->prPrev = NULL;

	/* add the node to the list */
	if (NULL != prHdrMemList->prHead) {
		prHdrMemList->prHead->prPrev = prNode;
		prNode->prNext = prHdrMemList->prHead;
	}

	prHdrMemList->prHead = prNode;

	if (NULL == prHdrMemList->prTail)
		prHdrMemList->prTail = prNode;

	/* recording the addr and size in the node */
	prNode->pvAddr = prHdrMemList->pvHdrWp;

	prNode->u4Size = u4Size;

	/* u4HdrWp pointer to the next place. */
	prHdrMemList->pvHdrWp = pvHdrDataEa;

	if (g_rDmxCliMan.fgDumpRspInfo)
		DmxDumpRspData(prSpt->u4SptCompId, prNode, prHdrMemList, TRUE);

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
		TEXT("[SPT] %s -- TailNode(%p), Alloc Node(%p)'s Header")
		TEXT("(Addr(%p), Size(0x%08x), Wp(0x%P), Rp(0x%P))!!\r\n"),
		DMX_FUNC_NAME, prHdrMemList->prTail, prNode,
		prNode->pvAddr, prNode->u4Size,
		prHdrMemList->pvHdrWp, prHdrMemList->pvHdrRp);

	MM_RETURN(RET_DMX_OK);

}

/* ///////////////////////////////////////////////////////////////////////////// */
/* RspSampleHdrFreeNode */
/* Free the node with u4Addr in the bidirectional circulation link from tail linker, */
/* and we also free its pointing memory in the prSpt->pvSampleHdrBufSa */
/* @Param u4Addr [IN] compare with node 's u4addr to search nodes */
/* @return      whether we done it success. */
/* ///////////////////////////////////////////////////////////////////////////// */
static bool RspSampleHdrFreeNode(void *pvSptHdl,
	RSP_HDR_MEM_NODE *prFreeNode)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;
	RSP_HDR_MEM_LIST *prHdrMemList = NULL;

	if ((NULL == prSpt) || (NULL == prFreeNode) || (0 == prFreeNode->pvAddr)) {
		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for invalid args\r\n"), DMX_FUNC_NAME);
		return TRUE;
	}

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
		TEXT("[SPT] %s -- Free Node(%p)'s HeaderAddr(%p), ")
		TEXT("HeaderSize(0x%08x)!!\r\n"),
		DMX_FUNC_NAME, prFreeNode, prFreeNode->pvAddr, prFreeNode->u4Size);

	prHdrMemList = &(prSpt->rHdrMemList);

	if ((NULL != prHdrMemList->prTail) &&
		(prFreeNode->pvAddr != prHdrMemList->prTail->pvAddr)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s -- fail for the free node's pvAddr(0x%p) != ")
			TEXT("TailNode' pvAddr(%p)\r\n"),
			DMX_FUNC_NAME, prFreeNode->pvAddr, prHdrMemList->prTail->pvAddr);
		return FALSE;
	}

	if (NULL != prFreeNode->prNext)
		prFreeNode->prNext->prPrev = prFreeNode->prPrev;

	if (NULL != prFreeNode->prPrev)
		prFreeNode->prPrev->prNext = prFreeNode->prNext;

	if (prHdrMemList->prTail == prFreeNode)
		prHdrMemList->prTail = prFreeNode->prPrev;

	DMX_ASSERT(prHdrMemList->prTail != prFreeNode);

	if (prHdrMemList->prHead == prFreeNode)
		prHdrMemList->prHead = prFreeNode->prNext;

	DMX_ASSERT(prHdrMemList->prHead != prFreeNode);

	dmx_memset(prFreeNode, 0, sizeof(RSP_HDR_MEM_NODE));

	/* Whether these is no any node in the list */
	if (NULL == prHdrMemList->prTail) {
		if (NULL != prHdrMemList->prHead) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s -- fail for unexpected situaton, the ")
				TEXT("Sample Header list has head, but no tail\r\n"),
				DMX_FUNC_NAME);
			return FALSE;
		}
		prHdrMemList->pvHdrRp = prHdrMemList->pvSa;
		prHdrMemList->pvHdrWp = prHdrMemList->pvSa;
		DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s have free all node, the Sample Header List ")
			TEXT("has nothing now\r\n"),
			DMX_FUNC_NAME);
		return TRUE;
	}

	prHdrMemList->pvHdrRp = prHdrMemList->prTail->pvAddr;

	return TRUE;
}


/* ///////////////////////////////////////////////////////////////////////////// */
/* RspSampleHdrFreeHeadNode */
/* Free the node with u4Addr in the bidirectional circulation link from head linker, */
/* and we also free its pointing memory in the prSpt->pvSampleHdrBufSa */
/* @Param u4Addr         [IN] compare with node 's u4addr to search nodes */
/* @Param prFreeNode [IN] compare with node 's u4addr to search nodes */
/* @return      whether we done it success. */
/* ///////////////////////////////////////////////////////////////////////////// */
static bool RspSampleHdrFreeHeadNode(void *pvSptHdl,
	RSP_HDR_MEM_NODE *prFreeNode)
{
	RSP_HDR_MEM_LIST *prHdrMemList = NULL;
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;

	if ((NULL == prSpt) || (NULL == prFreeNode) || (0 == prFreeNode->pvAddr)) {
		DMXLOG_ERROR(TEXT("[SPT] %s fail for invalid args\r\n"),
			DMX_FUNC_NAME);
		return FALSE;
	}

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
		TEXT("[SPT] %s -- Free Node's HeaderAddr(%p), ")
		TEXT("HeaderSize(0x%08x)!!\r\n"),
		DMX_FUNC_NAME, prFreeNode->pvAddr, prFreeNode->u4Size);

	prHdrMemList = &(prSpt->rHdrMemList);
	if ((NULL != prHdrMemList->prHead) &&
		(prFreeNode->pvAddr != prHdrMemList->prHead->pvAddr)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s -- fail for the free node's param:u4Addr(%p)")
			TEXT(" != Head Node' param:u4Addr(%p)\r\n"),
			DMX_FUNC_NAME, prFreeNode->pvAddr, prHdrMemList->prHead->pvAddr);
		return FALSE;
	}

	/* delete the nodes of u4Addr */
	if (NULL != prFreeNode->prNext)
		prFreeNode->prNext->prPrev = prFreeNode->prPrev;

	if (NULL != prFreeNode->prPrev)
		prFreeNode->prPrev->prNext = prFreeNode->prNext;

	if (prHdrMemList->prTail == prFreeNode)
		prHdrMemList->prTail = prFreeNode->prPrev;

	if (prHdrMemList->prHead == prFreeNode)
		prHdrMemList->prHead = prFreeNode->prNext;

	dmx_memset(prFreeNode, 0, sizeof(RSP_HDR_MEM_NODE));

	/* Whether these is no any node in the list */
	if (NULL == prHdrMemList->prHead) {
		if (NULL != prHdrMemList->prTail) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s -- fail for unexpected situaton, the Sample")
				TEXT(" Header list has tail, but no head\r\n"),
				DMX_FUNC_NAME);
			return FALSE;
		}
		prHdrMemList->pvHdrRp = prHdrMemList->pvSa;
		prHdrMemList->pvHdrWp = prHdrMemList->pvSa;
		DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s have free all nodes\r\n"), DMX_FUNC_NAME);
		return TRUE;
	}

	if (NULL != prHdrMemList->prTail) {
		prHdrMemList->pvHdrRp = prHdrMemList->prTail->pvAddr;
	} else {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s -- fail for unexpected situaton, the Sample")
			TEXT(" Header list has head, but no tail\r\n"),
			DMX_FUNC_NAME);
		return FALSE;
	}

	/* Reset u4HdlWp */
	if ((NULL != prHdrMemList->prHead) &&
	    (prHdrMemList->prHead->pvAddr +
	    prHdrMemList->prHead->u4Size != prHdrMemList->pvHdrWp))
		prHdrMemList->pvHdrWp = prHdrMemList->prHead->pvAddr +
			prHdrMemList->prHead->u4Size;

	return TRUE;
}

static bool RspSampleHdrFreeTail2Node(void *pvSptHdl,
	RSP_HDR_MEM_NODE *prNode)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;

	if ((NULL == prSpt) || (NULL == prNode)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for invalid args\r\n"), DMX_FUNC_NAME);
		return FALSE;
	}

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
		TEXT("[SPT] %s -- prNode(0%p), TailNode(%p), Tail's ")
		TEXT("u4Addr(%p), HdrWp(0x%p), HdrRp(0x%p)!\r\n"),
		DMX_FUNC_NAME, prNode, prSpt->rHdrMemList.prTail,
		prSpt->rHdrMemList.prTail->pvAddr,
		prSpt->rHdrMemList.pvHdrWp, prSpt->rHdrMemList.pvHdrRp);

	while ((prNode != prSpt->rHdrMemList.prTail) &&
		(NULL != prSpt->rHdrMemList.prTail)) {
		if (NULL != prSpt->rHdrMemList.prTail->pvAddr) {
			RSP_HDR_MEM_NODE *prNodeTmp = prSpt->rHdrMemList.prTail;

			if (g_rDmxCliMan.fgDumpRspInfo)
				DmxDumpRspData(prSpt->u4SptCompId, prNodeTmp, &(prSpt->rHdrMemList),
					       FALSE);

			if (RspSampleHdrFreeNode(pvSptHdl, prNodeTmp)) {
				prNodeTmp->pvAddr = NULL;
				prNodeTmp->u4Size = 0;
			}
		} else {
			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s fail prNode(0%p), TailNode(%p), ")
				TEXT("Tail's pvAddr(%p)!\r\n"),
				DMX_FUNC_NAME, prNode, prSpt->rHdrMemList.prTail,
				prSpt->rHdrMemList.prTail->pvAddr);
			return FALSE;
		}
	}

	return TRUE;
}

/* ///////////////////////////////////////////////////////////////////////////// */
/* RspSampleHdrWriteNode */
/* Copy the hdr data from inf to the prSpt->pvSampleHdrBufSa with the u4Addr pointer */
/* and with data length u4Size. So we have to search the node first */
/* @Param prNode [IN] Pointer to the write node */
/* @Param pvHdrSrcAddr [IN]  the data source 's virtual address from inf */
/* @Param u4Size  [IN] the data length */
/* @return      whether we done it success. */
/* ///////////////////////////////////////////////////////////////////////////// */
static MRESULT RspSampleHdrWriteNode(void *pvSptHdl,
	RSP_HDR_MEM_NODE *prNode, void *pvHdrSrcAddr, u32 u4Size)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;

	if ((NULL == prNode) || (NULL == prSpt)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for invalid args(prSpt: 0x%p, ")
			TEXT("prNode: %p)!\r\n"),
			DMX_FUNC_NAME, prSpt, prNode);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (prNode != prSpt->rHdrMemList.prHead) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for Current Write RspHdrNode isn't the")
			TEXT(" head node of the list!\r\n"),
			DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	if (prNode->pvAddr + u4Size > prSpt->rHdrMemList.pvEa) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for the Sample Header buffer has no ")
			TEXT("spare space to set the header data, !")
			TEXT("prNode->pvAddr(%p) + size(0x%08x) > SampleHdrBuf's ")
			TEXT("EndAddr(%p)!\r\n"),
			DMX_FUNC_NAME, prNode->pvAddr, u4Size, prSpt->rHdrMemList.pvEa);
		MM_RETURN(RET_DMX_ERR_STATE);
	} else {
		dmx_memcpy(prNode->pvAddr, pvHdrSrcAddr, u4Size);
	}

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
		TEXT("[SPT] %s -- Write Node's HeaderAddr(%p), ")
		TEXT("HeaderSize(0x%08x)!!\r\n"),
		DMX_FUNC_NAME, prNode->pvAddr, prNode->u4Size);

	MM_RETURN(RET_DMX_OK);
}


/* ///////////////////////////////////////////////////////////////////////////// */
/* SplitterRspLogSampleHdr */
/* Log the hdr data from inf to the prSpt->pvSampleHdrBufSa with the u4Addr pointer */
/* and with data length u4Size. So we have to alloc a node first, then */
/* we would call HDR_WriteNode to store the data to the prSpt->pvSampleHdrBufSa */
/* @Param u4HdrSrcAddr [IN]  the data source 's virtual address from inf */
/* @Param u4Size  [IN] the data length */
/* @return      u4Addr if we done  success. */
/* @return 0 if we done failure */
/* ///////////////////////////////////////////////////////////////////////////// */
MRESULT SplitterRspLogSampleHdr(void *pvSptHdl,
	RSP_HDR_MEM_NODE *prNode, void *pvHdrSrcAddr, u32 u4Size)
{
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == pvSptHdl) || (NULL == prNode) || (0 == u4Size) ||
		(NULL == pvHdrSrcAddr)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for invalid args(u4HdrSrcAddr: %p, ")
			TEXT("u4Size: %d, pvSptHdl: 0x%p, prNode: %p)\r\n"),
			DMX_FUNC_NAME, pvHdrSrcAddr, u4Size, pvSptHdl, prNode);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
		TEXT("[SPT] %s enter, pvHdrSrcAddr(%p), u4Size(0x%08x)!!\r\n"),
		DMX_FUNC_NAME, pvHdrSrcAddr, u4Size);

	mrRet = RspSampleHdrAllocNode(pvSptHdl, prNode, u4Size);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail to alloc the SampleHdrNode\r\n"),
			DMX_FUNC_NAME);
		MM_RETURN(mrRet);
	}

	mrRet = RspSampleHdrWriteNode(pvSptHdl, prNode, pvHdrSrcAddr, u4Size);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail to write data to the node\r\n"),
			DMX_FUNC_NAME);
		RspSampleHdrFreeHeadNode(pvSptHdl, prNode);
		MM_RETURN(mrRet);
	}

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
		TEXT("[SPT] %s Success, pvHdrSrcAddr(%p), u4Size(0x%08x)!!\r\n"),
		DMX_FUNC_NAME, pvHdrSrcAddr, u4Size);

	MM_RETURN(RET_DMX_OK);
}

u32 SplitterRspGetLogAUMaxCnt(void *pvSptHdl,
	const DMX_STM_CNT_INFO_T *prStmsCnt)
{
	bool fgMultiAud = FALSE, fgMultiSP = FALSE;

	if (NULL == prStmsCnt) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s fail for invalid args\r\n"), DMX_FUNC_NAME);
		return 0;
	}
#if (DMX_DISABLE_AUD_DMA || DMX_DISABLE_AUD_STM)
	DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
		TEXT("[SPT] %s --- set RspMaxCnt to be 0 for audio disable\r\n"),
		DMX_FUNC_NAME);
	return 0;
#endif				/* (DMX_DISABLE_AUD_DMA || DMX_DISABLE_AUD_STM) */

	fgMultiAud = (prStmsCnt->u4AudStmCnt > 1) ? TRUE : FALSE;

	fgMultiSP = (prStmsCnt->u4SPStmCnt > 1) ? TRUE : FALSE;

#ifndef __linux__
#if DMX_CFG_RSPMEM_BY_MEMSIZE
	if (128 == OSE_GetChipMemSize()) {
		if ((!fgMultiAud) && (!fgMultiSP))
			return 0;
		else if (fgMultiAud)
			return MAX_OF_RSP_LOGAU_AUD_CNT_128M;
		else if (fgMultiSP)
			return MAX_OF_RSP_LOGAU_SP_CNT_128M;
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s fail for this splitter instance has multi ")
			TEXT("audio and multi sp\r\n"),
			DMX_FUNC_NAME);
	} else {
		if ((!fgMultiAud) && (!fgMultiSP))
			return 0;
		else if (fgMultiAud)
			return MAX_OF_RSP_LOGAU_AUD_CNT;
		else if (fgMultiSP)
			return MAX_OF_RSP_LOGAU_SP_CNT;
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s line %d fail for this splitter instance has ")
			TEXT("multi audio and multi sp\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
	}
#else
	return MAX_OF_RSP_LOGAU_AUDSP_CNT;
#endif				/* DMX_CFG_RSPMEM_BY_MEMSIZE */
#else
	if ((!fgMultiAud) && (!fgMultiSP))
		return 0;
	else if (fgMultiAud)
		return MAX_OF_RSP_LOGAU_AUD_CNT;
	else if (fgMultiSP)
		return MAX_OF_RSP_LOGAU_SP_CNT;
	DMX_ASSERT(FALSE);
	DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
		TEXT("[SPT] %s line %d fail for this splitter instance has ")
		TEXT("multi audio and multi sp\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO);
#endif

	return 0;
}

MRESULT SplitterRspInit(void *pvSptHdl, DMX_STM_CNT_INFO_T *prStmsCnt)
{
	MRESULT mrRet = RET_DMX_OK;
#if RSP_ENABLE
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;
	u32 u4TotalAusSz = 0, u4TotalAUCnt = 0;

	DmxLogEnable(TRUE, DMX_LOG_DEBUG, DMX_MOD_RSP,
		     DMX_MOD_RSP_LOGLVL_RSPTX | DMX_MOD_RSP_LOGLVL_REBUF);

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
		TEXT("[SPT] %s enter!! \r\n"), DMX_FUNC_NAME);

	if (NULL == prSpt) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (CFA_TYPE_TS == SplitterGetCfaType(pvSptHdl)) {
		SplitterRspClear(pvSptHdl);
		prSpt->fgRspEnable = FALSE;
		DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s success, pvSptHdl(0x%p) -- cfa type == cfa ts.\r\n"),
			DMX_FUNC_NAME, pvSptHdl);

		MM_RETURN(RET_DMX_OK);
	}

	u4TotalAUCnt = SplitterRspGetLogAUMaxCnt(pvSptHdl, prStmsCnt);

	if (0 == u4TotalAUCnt) {
		SplitterRspClear(pvSptHdl);
		prSpt->fgRspEnable = FALSE;
		DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s success, don't need to support resplitter, ")
			TEXT("pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		MM_RETURN(RET_DMX_OK);
	}

	if (prSpt->u4RspEntryMax != u4TotalAUCnt)
		SplitterRspUnInit(pvSptHdl);
	else {
		SplitterRspClear(pvSptHdl);
		prSpt->fgRspEnable = TRUE;
		DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s line %d success, exit!!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);

		MM_RETURN(RET_DMX_OK);
	}

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
		TEXT("[SPT] %s -- Splitter instance(0x%p)'s Resplitter Support ")
		TEXT("Total LogAU Count = %d!!\r\n"),
		DMX_FUNC_NAME, pvSptHdl, u4TotalAUCnt);

	prSpt->u2RspAuSize = DMX_MAX(sizeof(AU_AUDIO), sizeof(AU_SP));

	u4TotalAusSz = u4TotalAUCnt * (sizeof(SPT_RSP_T) + prSpt->u2RspAuSize);

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
		TEXT("[SPT] %s -- pvSptHdl:0x%p, rsp mem sz:0x%x\r\n"),
		DMX_FUNC_NAME, pvSptHdl, u4TotalAusSz);

#ifdef __linux__
	DMX_NewHwMemory(u4TotalAusSz, prSpt->pvRspAUTblSa);
#else
	DMX_NewHwMemory(u4TotalAusSz, (void *) (prSpt->pvRspAUTblSa));
#endif				/* #ifdef __linux__ */

	if (NULL == prSpt->pvRspAUTblSa) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s -- pvSptHdl:0x%p, rsp mem sz:0x%x, fail in alloc ")
			TEXT("memory for Rsp AU Table!! \r\n"),
			DMX_FUNC_NAME, pvSptHdl, u4TotalAusSz);
		prSpt->pvRspAUTblEa = NULL;
		mrRet = RET_DMX_NO_MEM;
		goto SPTRSPINITERR;
	} else {
		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s alloc pvRspAUTblSa success!! \r\n"),
			DMX_FUNC_NAME);
		prSpt->u4RspEntryMax = u4TotalAUCnt;
	}

	dmx_memset((void *) (prSpt->pvRspAUTblSa), 0, u4TotalAusSz);
	prSpt->pvRspAUTblEa = prSpt->pvRspAUTblSa + u4TotalAusSz;

	prSpt->pvRspWp = prSpt->pvRspAUTblSa;
	prSpt->pvRspRp = prSpt->pvRspAUTblSa;
	prSpt->u4RspCount = 0;
	prSpt->u4RspDrop = 0;
	prSpt->pvRspLastTx = NULL;
	prSpt->pvRspFirstTx = NULL;
	prSpt->pvRspCurPtr = NULL;
	prSpt->u4RspEntryCount = 0;

#ifdef __linux__
	DMX_NewMemory(sizeof(PSR_AU), prSpt->pvTempPsrAu);
#else
	DMX_NewMemory(sizeof(PSR_AU), (void *) (prSpt->pvTempPsrAu));
#endif				/* #ifdef __linux__ */

	if (NULL == prSpt->pvTempPsrAu) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
		    TEXT("[SPT] %s, pvSptHdl:0x%p, fail in alloc memory for ")
			TEXT("pvTempPsrAu!! \r\n"),
		    DMX_FUNC_NAME, pvSptHdl);
		mrRet = RET_DMX_NO_MEM;
		goto SPTRSPINITERR;
	}

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
		TEXT("[SPT] %s alloc pvSampleHdrBufSa success!! \r\n"),
		DMX_FUNC_NAME);
	dmx_memset((void *) (prSpt->pvTempPsrAu), 0, sizeof(PSR_AU));

#ifdef __linux__
	DMX_NewMemory(prSpt->u2RspAuSize, prSpt->pvTempAu);
#else
	DMX_NewMemory(prSpt->u2RspAuSize, (void *) (prSpt->pvTempAu));
#endif				/* #ifdef __linux__ */

	if (NULL == prSpt->pvTempAu) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s Spt:0x%p, fail in alloc memory for ")
			TEXT("pvTempAu!! \r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		mrRet = RET_DMX_NO_MEM;
		goto SPTRSPINITERR;
	} else {
		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s alloc pvTempAu success!! \r\n"), DMX_FUNC_NAME);
		dmx_memset((void *) (prSpt->pvTempAu), 0, prSpt->u2RspAuSize);
	}

#ifdef __linux__
	DMX_NewMemory((SPT_REC_RSP_INF_STRM_MAX * sizeof(RSP_STRM_INF)),
		prSpt->pvRspExtInfSa);
#else
	DMX_NewMemory((SPT_REC_RSP_INF_STRM_MAX * sizeof(RSP_STRM_INF)),
		      (void *) (prSpt->pvRspExtInfSa));
#endif				/* #ifdef __linux__ */

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
		TEXT("[SPT] %s, pvSptHdl:0x%p, pvSa:0x%x, tmpPsrAu:0x%x, tmpAu:0x%x, ")
		TEXT("RspExtInfSa:0x%x\r\n"),
		DMX_FUNC_NAME, pvSptHdl, prSpt->pvRspAUTblSa,
		prSpt->pvTempPsrAu, prSpt->pvTempAu,
		prSpt->pvRspExtInfSa);

	if (NULL == prSpt->pvRspExtInfSa) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s, pvSptHdl:0x%p, fail in alloc memory for ")
			TEXT("pvRspExtInfSa!! \r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		mrRet = RET_DMX_NO_MEM;
		goto SPTRSPINITERR;
	}

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
		TEXT("[SPT] %s alloc pvRspExtInfSa success!! \r\n"), DMX_FUNC_NAME);
	dmx_memset((void *) (prSpt->pvRspExtInfSa), 0,
		   SPT_REC_RSP_INF_STRM_MAX * sizeof(RSP_STRM_INF));

	mrRet = RspSampleHdrInit(prSpt);

	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s line %d fail in RspSampleHdrInit, prSpt(0x%p), ")
			TEXT("mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt, mrRet);
		mrRet = RET_DMX_NO_MEM;
		goto SPTRSPINITERR;
	}

	prSpt->fgRspEnable = TRUE;
	prSpt->fgReRsp = FALSE;

	DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
		TEXT("[SPT] %s success, Rsp AUTable's Start Address is 0x%p, ")
		TEXT("End Address is 0x%p, Max AU Count is %d!!\r\n"),
		DMX_FUNC_NAME, prSpt->pvRspAUTblSa,
		prSpt->pvRspAUTblEa, prSpt->u4RspEntryMax);

	MM_RETURN(RET_DMX_OK);

SPTRSPINITERR:

	SplitterRspUnInit(pvSptHdl);

	prSpt->fgRspEnable = FALSE;

	DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
		TEXT("[SPT] %s fail, mrRet: 0x%x, exit!!\r\n"), DMX_FUNC_NAME, mrRet);
#endif

	MM_RETURN(mrRet);
}

void SplitterRspClear(void *pvSptHdl)
{
#if RSP_ENABLE
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *)pvSptHdl;
	RSP_HDR_MEM_LIST *prRspHdrMemList;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prSpt) {
		DMX_ASSERT(FALSE);
		return;
	}

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
		TEXT("[SPT] %s enter -- prSpt:0x%p, Sa:0x%p, tmpPsrAu:0x%p, ")
		TEXT("tmpAu:0x%p, RspExtInfSa:0x%p\r\n"),
		DMX_FUNC_NAME, prSpt, prSpt->pvRspAUTblSa,
		prSpt->pvTempPsrAu, prSpt->pvTempAu,
		prSpt->pvRspExtInfSa);


	mrRet = SplitterRspClearLog(pvSptHdl);
	if (DMX_FAILED(mrRet))
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s line %d fail in SplitterRspClearLog, ")
			TEXT("pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);

	if (NULL != prSpt->pvSampleHdrBufSa)
		dmx_memset((void *) (prSpt->pvSampleHdrBufSa), 0, RSP_SAMPHDR_LEN_MAX);

	dmx_memset(&(prSpt->rHdrMemList), 0, sizeof(RSP_HDR_MEM_LIST));

	prRspHdrMemList = &(prSpt->rHdrMemList);

	prRspHdrMemList->pvSa = prSpt->pvSampleHdrBufSa;
	if (NULL != prSpt->pvSampleHdrBufSa)
		prRspHdrMemList->pvEa =
			prSpt->pvSampleHdrBufSa + RSP_SAMPHDR_LEN_MAX;
	else
		prRspHdrMemList->pvEa =
			prSpt->pvSampleHdrBufSa;

	prRspHdrMemList->pvHdrRp = prRspHdrMemList->pvSa;
	prRspHdrMemList->pvHdrWp = prRspHdrMemList->pvSa;

	prRspHdrMemList->prHead = NULL;
	prRspHdrMemList->prTail = NULL;

	if (NULL != prSpt->pvRspAUTblSa)
		dmx_memset((void *) (prSpt->pvRspAUTblSa), 0,
			   (prSpt->pvRspAUTblEa - prSpt->pvRspAUTblSa));

	if (NULL != prSpt->pvTempPsrAu)
		dmx_memset((void *) (prSpt->pvTempPsrAu), 0, sizeof(PSR_AU));

	if (NULL != prSpt->pvTempAu)
		dmx_memset((void *) (prSpt->pvTempAu), 0, prSpt->u2RspAuSize);

	if (NULL != prSpt->pvRspExtInfSa)
		dmx_memset((void *) (prSpt->pvRspExtInfSa), 0,
			   SPT_REC_RSP_INF_STRM_MAX * sizeof(RSP_STRM_INF));

	prSpt->u4RspCount = 0;
	prSpt->u4RspDrop = 0;
	prSpt->pvRspLastTx = NULL;
	prSpt->pvRspFirstTx = NULL;
	prSpt->pvRspCurPtr = NULL;
	prSpt->u4RspEntryCount = 0;
	prSpt->pvRspWp = prSpt->pvRspAUTblSa;
	prSpt->pvRspRp = prSpt->pvRspAUTblSa;
	prSpt->fgRspEnable = FALSE;

	SplitterSetRspStartPts(pvSptHdl, INVALID_TIMESTAMP);
	SplitterSetRspStartOffset(pvSptHdl, DMX_INVALID_UINT64);
	SplitterSetRspOffsetDelta(pvSptHdl, 0);
	SplitterSetReResplitter(pvSptHdl, FALSE);
	SplitterSetRspTxType(pvSptHdl, SPT_DATA_UNDEFINE);
	SplitterSetRspMode(pvSptHdl, SPLITTER_PTX_RSP_BY_PTS);

	DmxCloseDumpRspFile(prSpt->u4SptCompId);

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
		TEXT("[SPT] %s success, -- pvSptHdl:0x%p\r\n"), DMX_FUNC_NAME, pvSptHdl);

	return;
#endif				/* RSP_ENABLE */
}

void SplitterRspUnInit(void *pvSptHdl)
{
#if RSP_ENABLE
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prSpt) {
		DMX_ASSERT(FALSE);
		return;
	}

	DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
		TEXT("[SPT] %s enter, prSpt(0x%p)\r\n"), DMX_FUNC_NAME, prSpt);

	mrRet = SplitterRspClearLog(pvSptHdl);
	if (DMX_FAILED(mrRet))
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s line %d fail in SplitterRspClearLog, ")
			TEXT("pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);

	mrRet = RspSampleHdrUninit(prSpt);
	if (DMX_FAILED(mrRet))
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s line %d fail in RspSampleHdrUninit,")
			TEXT(" mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);

	if (NULL != prSpt->pvRspAUTblSa) {
#ifdef __linux__
		DMX_FreeHwMemory(prSpt->pvRspAUTblSa);
#else
		DMX_FreeHwMemory((void *) (prSpt->pvRspAUTblSa));
#endif				/* #ifdef __linux__ */
		prSpt->pvRspAUTblSa = NULL;
		prSpt->pvRspAUTblEa = NULL;
	}

	if (NULL != prSpt->pvTempPsrAu) {
#ifdef __linux__
		DMX_FreeMemory(prSpt->pvTempPsrAu);
#else
		DMX_FreeMemory((void *) (prSpt->pvTempPsrAu));
#endif				/* #ifdef __linux__ */
		prSpt->pvTempPsrAu = NULL;
	}

	if (NULL != prSpt->pvTempAu) {
#ifdef __linux__
		DMX_FreeMemory(prSpt->pvTempAu);
#else
		DMX_FreeMemory((void *) (prSpt->pvTempAu));
#endif				/* #ifdef __linux__ */
		prSpt->pvTempAu = NULL;
	}

	if (NULL != prSpt->pvRspExtInfSa) {
#ifdef __linux__
		DMX_FreeMemory(prSpt->pvRspExtInfSa);
#else
		DMX_FreeMemory((void *) (prSpt->pvRspExtInfSa));
#endif				/* #ifdef __linux__ */
		prSpt->pvRspExtInfSa = NULL;
	}


	prSpt->u4RspCount = 0;
	prSpt->u4RspDrop = 0;
	prSpt->pvRspLastTx = NULL;
	prSpt->pvRspFirstTx = NULL;
	prSpt->u4RspEntryCount = 0;
	prSpt->u4RspEntryMax = 0;
	prSpt->pvRspWp = NULL;
	prSpt->pvRspRp = NULL;
	prSpt->fgRspEnable = FALSE;
	prSpt->fgReRsp = FALSE;

	SplitterSetRspStartPts(pvSptHdl, INVALID_TIMESTAMP);
	SplitterSetRspStartOffset(pvSptHdl, DMX_INVALID_UINT64);
	SplitterSetRspOffsetDelta(pvSptHdl, 0);
	SplitterSetReResplitter(pvSptHdl, FALSE);
	SplitterSetRspTxType(pvSptHdl, SPT_DATA_UNDEFINE);
	SplitterSetRspMode(pvSptHdl, SPLITTER_PTX_RSP_BY_PTS);

	DmxCloseDumpRspFile(prSpt->u4SptCompId);

	DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
		TEXT("[SPT] %s success, pvSptHdl(0x%p)\r\n"), DMX_FUNC_NAME, pvSptHdl);

	return;
#endif				/* RSP_ENABLE */
}

/* Query Stage */
/* /////////////////////////////////////////////////////////////////// */
/* SplitterRspGetRspRangeByPts */
/* Get the Rsp Entry whose start Pts is little smaller than the designated Playing PTS(RspOffPts + u8RspPtsDelta) */
/* Get its fileoffset of the Rsp entry */
/* @ Param      u8RspPtsDelta           [IN]             delta pts from Rsp off to call this function */
/* @ Param      pu8RspStartOffset  [OUT]          the fileoffset of the obtained Rsp Entry */
/* @ Param      pu8PbbStartOffset  [OUT]          when Rsplitter off, the start file offset of the pbbuf */
/* /////////////////////////////////////////////////////////////////// */
MRESULT SplitterRspGetRspRangeByPts(void *pvSptHdl, u64 u8RspPtsDelta,
u64 *pu8RspStartOffset, u64 *pu8PbbStartOffset)
{				/* Get video PTS from 1st splitter */
#if RSP_ENABLE
	SPT_RSP_T *prSptRspFirst = NULL;
	SPT_RSP_T *prSptRspPrev = NULL;
	SPT_RSP_T *prSptRspLast = NULL;
	u32 u4RspTxType = DMX_INVALID_UINT32;
	u32 u4RspTxFtrUID = DMX_INVALID_UINT32;
	u32 u4RspEntryIdx = 0;
	u64 u8RspPTS = 0;
	u64 u8RspStartOfst = 0;
	u64 u8RspOfstDelta = 0;

	if ((NULL == pu8RspStartOffset) || (NULL == pu8PbbStartOffset)) {
		DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
			TEXT("[SPT] %s fail for invalid args!! \r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	u8RspPTS = SplitterGetRspStartPts(pvSptHdl);
	u8RspStartOfst = SplitterGetRspStartOffset(pvSptHdl);
	u8RspOfstDelta = SplitterGetRspOffsetDelta(pvSptHdl);

	DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
		TEXT("[SPT] %s -- RspStartPts: " DMX_PTS_LOGSTR)
		TEXT(", RspPtsDelta: " DMX_PTS_LOGSTR)
		TEXT(", RspOffset: " DMX_UINT64_16U_LOGSTR)
		TEXT(", RspOffsetDelta: " DMX_UINT64_16U_LOGSTR "!! \r\n"),
		DMX_FUNC_NAME,
		DMX_PTS_LOG_MS(u8RspPTS),
		DMX_PTS_LOG_PTS(u8RspPTS),
		DMX_PTS_LOG_MS(u8RspPtsDelta),
		DMX_PTS_LOG_PTS(u8RspPtsDelta),
		DMX_UINT64_16U_LOG_H(u8RspStartOfst),
		DMX_UINT64_16U_LOG_L(u8RspStartOfst),
		DMX_UINT64_16U_LOG_H(u8RspOfstDelta),
		DMX_UINT64_16U_LOG_L(u8RspOfstDelta));

	/* TODO: Remove after Integrate */
	if (!SplitterRspIsEnabled(pvSptHdl))
		MM_RETURN(RET_DMX_OK);

	u8RspPTS += u8RspPtsDelta;
	*pu8PbbStartOffset = SplitterGetPBBOffsetSa(pvSptHdl);

	/* Find the proper SPT_RSP_T according to the current PTS */
	/* However, we have no idea which audio channel MPC will set later. */
	/* Thus, we give the offset from the very first entry */
	prSptRspFirst = SplitterGetRspFirstTx(pvSptHdl);
	prSptRspPrev = NULL;
	prSptRspLast = SplitterGetRspLastTx(pvSptHdl);
	u4RspTxType = SplitterGetRspTxType(pvSptHdl);
	u4RspTxFtrUID = GetStmUIDByType(pvSptHdl, u4RspTxType);
	u4RspEntryIdx = 0;

	DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
		TEXT("[SPT] %s -- RspPTS: " DMX_PTS_LOGSTR)
		TEXT(", RspTxFtrUID: %d, u4RspTxType: %d, prSptRspFirst:0x%p, ")
		TEXT("prSptRspLast:0x%p\r\n"),
		DMX_FUNC_NAME, DMX_PTS_LOG_MS(u8RspPTS), DMX_PTS_LOG_PTS(u8RspPTS),
		u4RspTxFtrUID, u4RspTxType,
		prSptRspFirst, prSptRspLast);

	while (NULL != prSptRspFirst) {
		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
			TEXT("[SPT] %s -- Idx(%d), StreamUID(%d), RspTxUID: %d, ")
			TEXT("TxType: %d, Entry:0x%p")
			TEXT(", u8PtsSa:" DMX_PTS_LOGSTR ", eFilledType(%d)\r\n"),
			DMX_FUNC_NAME, u4RspEntryIdx, prSptRspFirst->u4StreamUID,
			u4RspTxFtrUID, u4RspTxType, prSptRspFirst,
			DMX_PTS_LOG_MS(prSptRspFirst->rTxInfo.u8PtsSa),
			DMX_PTS_LOG_PTS(prSptRspFirst->rTxInfo.u8PtsSa),
			prSptRspFirst->eFilledType);

		if ((u4RspTxType == prSptRspFirst->rTxInfo.u4TxStreamType) &&
		    (u4RspTxFtrUID == prSptRspFirst->u4StreamUID) &&
		    (INVALID_TIMESTAMP != prSptRspFirst->rTxInfo.u8PtsSa)) {
			if ((prSptRspFirst->rTxInfo.u8PtsSa < u8RspPTS) &&
				SplitterRspEntryRemoveable(pvSptHdl, u8RspPTS, prSptRspFirst,
				prSptRspLast, NULL, &u4RspTxType))	/* Rsp entry is too old */
				prSptRspPrev = prSptRspFirst;
			else {
				if (NULL == prSptRspPrev)
					prSptRspPrev = prSptRspFirst;
				DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
					TEXT("[SPT] %s -- Idx(%d), StreamUID(%d), ")
					TEXT("RspTxUID: %d, TxType: %d, Entry:0x%p")
					TEXT(", u8PtsSa:" DMX_PTS_LOGSTR)
					TEXT(", eFilledType(%d), break\r\n"),
					DMX_FUNC_NAME,
					u4RspEntryIdx, prSptRspFirst->u4StreamUID,
					u4RspTxFtrUID, u4RspTxType, prSptRspFirst,
					DMX_PTS_LOG_MS(prSptRspFirst->rTxInfo.u8PtsSa),
					DMX_PTS_LOG_PTS(prSptRspFirst->rTxInfo.u8PtsSa),
					prSptRspFirst->eFilledType);
				break;
			}
		}
		prSptRspFirst = prSptRspFirst->prNextEntry;
		u4RspEntryIdx++;
	}

	/* 4 V-sync is the gap limitation */
	if (NULL != prSptRspFirst) {
		/* 6006 / 90000, == 66.7ms */
		if ((prSptRspFirst->rTxInfo.u8PtsSa > u8RspPTS) &&
			(NULL != prSptRspPrev) &&
			((prSptRspPrev->rTxInfo.u8PtsSa + 6006 >= u8RspPTS)))
			prSptRspFirst = prSptRspPrev;

		if ((prSptRspFirst->rTxInfo.fgAUByEnd) &&
		    (prSptRspFirst->rTxInfo.fgUnitEnd)) {
			while (NULL != prSptRspFirst->prPrevEntry) {
				if (prSptRspFirst->prPrevEntry->rTxInfo.fgAUByEnd) {
					if (!(prSptRspFirst->prPrevEntry->rTxInfo.fgCreateAU))
						prSptRspFirst = prSptRspFirst->prPrevEntry;
					else {
						prSptRspFirst = prSptRspFirst->prPrevEntry;
						break;
					}
				} else {
					break;
				}
			}
		}

		*pu8RspStartOffset = prSptRspFirst->rTxInfo.u8FromFileOfst;

		DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
			TEXT("[SPT] %s -- RspStartOfst:" DMX_UINT64_16U_LOGSTR)
			TEXT(", PbbStartOfst:" DMX_UINT64_16U_LOGSTR)
			TEXT(", RspFirstPTS:" DMX_PTS_LOGSTR)
			TEXT(", RspTime:" DMX_PTS_LOGSTR)
			TEXT(", Delta:" DMX_PTS_LOGSTR "\r\n"),
			DMX_FUNC_NAME,
			DMX_UINT64_16U_LOG_H(*pu8RspStartOffset),
			DMX_UINT64_16U_LOG_L(*pu8RspStartOffset),
			DMX_UINT64_16U_LOG_H(*pu8PbbStartOffset),
			DMX_UINT64_16U_LOG_L(*pu8PbbStartOffset),
			DMX_PTS_LOG_MS(prSptRspFirst->rTxInfo.u8PtsSa),
			DMX_PTS_LOG_PTS(prSptRspFirst->rTxInfo.u8PtsSa),
			DMX_PTS_LOG_MS(u8RspPTS),
			DMX_PTS_LOG_PTS(u8RspPTS),
			DMX_PTS_LOG_MS(u8RspPtsDelta),
			DMX_PTS_LOG_PTS(u8RspPtsDelta));

		/* if (!SplitterIstPureAudioPIPE(pvSptHdl)) */
		SplitterSetRspStartOffset(pvSptHdl, *pu8RspStartOffset);

		MM_RETURN(RET_DMX_OK);
	}

	*pu8RspStartOffset = *pu8PbbStartOffset;

	DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
		TEXT("[SPT] %s no log au found, pvSptHdl: 0x%p\r\n"), DMX_FUNC_NAME, pvSptHdl);
#endif				/* RSP_ENABLE */

	MM_RETURN(RET_DMX_NO_RSP_LOGAU);
}

/* ////////////////////////////////////////////////////////////////////////////////////////////// */
/* SplitterRspClearLog */
/* Clear all stream info in Resplitter Instance Steam Info Array--pvRspExtInfSa */
/* Remove All Log Entries in Resplitter Entry Fifo -- pvSa */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
MRESULT SplitterRspClearLog(void *pvSptHdl)
{
#if RSP_ENABLE
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;
	void *pvCurrentRsp = NULL;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
		TEXT("[SPT] %s enter, prSpt(0x%p)\r\n"), DMX_FUNC_NAME, prSpt);

	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s fail for invalid args!! \r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	/* TODO: Remove after Integrate */
	if (!SplitterRspIsEnabled(pvSptHdl))
		MM_RETURN(RET_DMX_OK);

	/* Clear RSP_STRM_INF Array pointed by pvRspExtInfSa */
	mrRet = SplitterRspUpdateExtInf(pvSptHdl, SPLITTER_RSP_INF_CLEAR, NULL);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s fail in SplitterRspUpdateExtInf(CLEAR),")
			TEXT(" mrRet: 0x%x!\r\n"),
			DMX_FUNC_NAME, mrRet);
		MM_RETURN(mrRet);
	}

	prSpt->u4RspStreamUID = 0;	/* i4SplitterSetRspStreamUID(u4SptHandle, 0); */
	prSpt->u4RspDrop = 0;

	pvCurrentRsp = SplitterGetRspFirstTx(pvSptHdl);

	while (NULL != pvCurrentRsp) {
		mrRet = SplitterRspRemoveLog(pvSptHdl, pvCurrentRsp);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
				TEXT("[SPT] %s fail in SplitterRspRemoveLog, mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, mrRet);
			MM_RETURN(mrRet);
		}
		pvCurrentRsp = SplitterGetRspFirstTx(pvSptHdl);
	}
#endif

	while (NULL != prSpt->rHdrMemList.prTail) {
		RSP_HDR_MEM_LIST *prHdrMemList = &(prSpt->rHdrMemList);

		if (0 != prSpt->rHdrMemList.prTail->pvAddr) {
			RSP_HDR_MEM_NODE *prNode = prSpt->rHdrMemList.prTail;

			DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
				TEXT("Remove -- NodeAddr: 0x%p, Addr: 0x%p, Size: 0x%x, ")
				TEXT("Prev: 0x%p, Next: 0x%p\r\n"),
				prNode, prNode->pvAddr, prNode->u4Size, prNode->prPrev,
				prNode->prNext);
			if (RspSampleHdrFreeNode(pvSptHdl, prNode)) {
				prNode->pvAddr = 0;
				prNode->u4Size = 0;
			}

			DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
				TEXT("Remove -- NodeAddr: 0x%p, Head: 0x%p, Tail: 0x%p,")
				TEXT(" Rp: 0x%p, Wp: 0x%p, Sa: 0x%p, Ea: 0x%p\r\n"),
				prNode, prHdrMemList->prHead, prHdrMemList->prTail,
				prHdrMemList->pvHdrRp, prHdrMemList->pvHdrWp,
				prHdrMemList->pvSa, prHdrMemList->pvEa);
		} else {
			DMX_ASSERT(FALSE);
			break;
		}
	}

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
		TEXT("[SPT] %s success, prSpt(0x%p)\r\n"), DMX_FUNC_NAME, prSpt);

	MM_RETURN(RET_DMX_OK);
}

/* ////////////////////////////////////////////////////////////////////////////////////////////// */
/* SplitterRspIsLoging */
/* Check whether Resplitter's state is RSP_LOGING */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
bool SplitterRspIsLoging(DMX_SPT_INST_T *prSpt)
{
#if RSP_ENABLE
	/* TODO: Remove after Integrate */
	if (!SplitterRspIsEnabled(prSpt))
		return FALSE;

	return (SPLITTER_STATE_RSP_LOGING == SplitterGetRspState(prSpt));
#else
	return FALSE;
#endif
}

/* ////////////////////////////////////////////////////////////////////////////////////////////// */
/* SplitterRspIsRsping */
/* Check whether Resplitter's state is RSP_RSPING */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
bool SplitterRspIsRsping(DMX_SPT_INST_T *prSpt)
{
#if RSP_ENABLE
	/* TODO: Remove after Integrate */
	if (!SplitterRspIsEnabled(prSpt))
		return FALSE;

	return (SPLITTER_STATE_RSP_RSPING == SplitterGetRspState(prSpt));
#else
	return FALSE;
#endif
}


/* Normal Stage */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
/* SplitterRspSetLogEnable */
/* Enable Resplitter, Set Resplitter state to be RSP_LOGING */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
MRESULT SplitterRspSetLogEnable(void *pvSptHdl)
{
#if RSP_ENABLE
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;

	/* Fix klocwork issue 1503 */
	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[SPT] %s fail for invalid args!! \r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	/* TODO: Remove after Integrate */
	if (!SplitterRspIsEnabled(pvSptHdl))
		MM_RETURN(RET_DMX_OK);

	DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
		TEXT("[SPT] %s	-- Enable Log AU!\r\n"), DMX_FUNC_NAME);
	SplitterSetRspState(pvSptHdl, SPLITTER_STATE_RSP_LOGING);
#endif				/* RSP_ENABLE */

	MM_RETURN(RET_DMX_OK);
}

/* Re-Split Stage */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
/* SplitterRspSetRspEnable */
/* Search the one resplitter entry which is the resplitter stream's entry  in resplitter entrys fifo */
/* Active to TX the entry's data into FIFO */
/* If no this entry, Inform Resplitter Finish */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
MRESULT SplitterRspSetRspEnable(void *pvSptHdl)
{
#if RSP_ENABLE
	/* Get video PTS from 1st splitter */
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;
	SPT_RSP_T *prSptRspPrev = NULL;
	SPT_RSP_T *prSptRspStart = NULL;
	SPT_RSP_T *prSptRspLast = NULL;
	SPT_RSP_T rRspNext;
	u32 u4RspTxType = DMX_INVALID_UINT32;
	u32 u4RspTxFtrUID = DMX_INVALID_UINT32;
	u64 u8RspPTS = 0;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[SPT] %s fail for invalid args!\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}
	if (SPLITTER_PTX_RSP_BY_OFFSET == prSpt->ucRspMode) {
		mrRet = SplitterRspSetRspEnableByOffset(pvSptHdl);
		MM_RETURN(mrRet);
	} else if (SPLITTER_PTX_RSP_BY_PTS != prSpt->ucRspMode) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[SPT] %s fail for invalid RspMode(%d)!\r\n"),
			DMX_FUNC_NAME, prSpt->ucRspMode);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	} else {
	}

	u8RspPTS = SplitterGetRspStartPts(pvSptHdl);

	DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
		TEXT("[SPT] %s -- RspStartPts: " DMX_PTS_LOGSTR "!! \r\n"),
		DMX_FUNC_NAME, DMX_PTS_LOG_MS(u8RspPTS), DMX_PTS_LOG_PTS(u8RspPTS));

	if (!SplitterIsReResplitter(pvSptHdl)) {
		/* Record Last Tx Len to inform cfa tx len after resplitter finish */
		DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[SPT] %s line %d -- LastPtxLen: ")
			TEXT(DMX_UINT64_16U_LOGSTR "!! \r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			DMX_UINT64_16U_LOG_H(SplitterGetPtxLen(pvSptHdl)),
			DMX_UINT64_16U_LOG_L(SplitterGetPtxLen(pvSptHdl)));
		SplitterSetRspLastPtxLen(pvSptHdl, SplitterGetPtxLen(pvSptHdl));
	}

	/* TODO: Remove after Integrate */
	if (!SplitterRspIsEnabled(pvSptHdl)) {
		DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[SPT] %s exit, resplitter is disable!\r\n"),
		    DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	if (SplitterIsCfaPsrEnd(pvSptHdl)) {
		mrRet = PSR_CC_NotiCfaPrsEnd(prSpt->pvPsrCC, FALSE);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s fail in PSR_CC_NotiCfaPrsEnd(pvSptHdl: 0x%p,")
				TEXT(" FALSE)!\r\n"),
				DMX_FUNC_NAME, pvSptHdl);
			MM_RETURN(mrRet);
		}
	}

	u8RspPTS += SplitterGetRspPtsDelta(pvSptHdl);

	/* Find the proper SPT_RSP_T according to the current PTS */
	prSptRspPrev = NULL;
	prSptRspStart = SplitterGetRspFirstTx(pvSptHdl);
	prSptRspLast = SplitterGetRspLastTx(pvSptHdl);
	u4RspTxType = SplitterGetRspTxType(pvSptHdl);
	u4RspTxFtrUID = GetStmUIDByType(pvSptHdl, u4RspTxType);

	mm_memset(&rRspNext, 0, sizeof(SPT_RSP_T));

	/* Search the Resplitter stream's Resplitter entry whose PTS is small larger than */
	/*the designated PTS--u8RspPts */
	while (NULL != prSptRspStart) {
		if ((u4RspTxFtrUID == prSptRspStart->u4StreamUID) &&
		    (u4RspTxType == prSptRspStart->rTxInfo.u4TxStreamType)) {
			if (INVALID_TIMESTAMP != prSptRspStart->rTxInfo.u8PtsSa) {
				/* check whether this resplitter entry is across the u8RspPTS, */
				/* if not, it should not enter Rsp TX */
				if ((prSptRspStart->rTxInfo.u8PtsSa < u8RspPTS) &&
					(SplitterRspEntryRemoveable(pvSptHdl, u8RspPTS,
					prSptRspStart, prSptRspLast, &rRspNext, &u4RspTxType)))
					prSptRspPrev = prSptRspStart;	/* Rsp entry is too old */
				else {
					if (NULL == prSptRspPrev)
						prSptRspPrev = prSptRspStart;
					break;
				}
			}
		}
		prSptRspStart = prSptRspStart->prNextEntry;
	}

	SplitterSetRspState(pvSptHdl, SPLITTER_STATE_RSP_RSPING);

	/* 4 V-sync is the gap limitation */
	if (NULL != prSptRspStart) {
		if ((NULL != prSptRspPrev) &&
		    (prSptRspPrev != prSptRspStart) &&
		    (SPT_DATA_A == prSptRspStart->rTxInfo.u4TxStreamType) &&
		    (prSptRspStart->rTxInfo.u8PtsSa > u8RspPTS) &&
		    (prSptRspPrev->rTxInfo.u8PtsSa + 6006 >= u8RspPTS))
			prSptRspStart = prSptRspPrev;

		if ((prSptRspStart->rTxInfo.fgAUByEnd) &&
		    (prSptRspStart->rTxInfo.fgUnitEnd)) {
			while (NULL != prSptRspStart->prPrevEntry) {
				if (prSptRspStart->prPrevEntry->rTxInfo.fgAUByEnd) {
					if (!(prSptRspStart->prPrevEntry->rTxInfo.fgCreateAU))
						prSptRspStart = prSptRspStart->prPrevEntry;
					else {
						prSptRspStart = prSptRspStart->prPrevEntry;
						break;
					}
				} else {
					break;
				}
			}
		}

	}
	/* fix BDP00115698, audio chunk >= 5s */
	if (NULL != prSptRspStart) {
		u64 u8GapTime = u8RspPTS - prSptRspStart->rTxInfo.u8PtsSa;

		if ((SPT_DATA_A == prSptRspStart->rTxInfo.u4TxStreamType) &&
			(((rRspNext.rTxInfo.u8PtsSa != 0) &&
			(u8RspPTS < rRspNext.rTxInfo.u8PtsSa)) ||
			(rRspNext.rTxInfo.u8PtsSa == 0)) &&
			((prSptRspStart->rTxInfo.u8PtsSa + (1 * 90000)) <= u8RspPTS) &&
			(u8GapTime < (prSpt->ucAudMaxDuration * 90000)) &&
			(prSpt->ucAudMaxDuration >= 3)) {
			u64 u8ExtLen = 0;
			u64 u8NextPts = rRspNext.rTxInfo.u8PtsSa;
			AU_AUDIO *prAudAU = (AU_AUDIO *)(prSptRspStart->pvAU);

			/* if audio pts is early then STC more than 1s, the sync-ctrl will delay */
			/* double time to delay audio(by adjust audio PTS) */

			if (0 == u8NextPts)
				u8NextPts = prSptRspStart->rTxInfo.u8PtsSa +
				    (prSpt->ucAudMaxDuration * 90000);

			/* We are sure u8NextPts is more than prSptRspStart->rTxInfo.u8PtsSa, */
			/* For of u8NextPts is caculated through addition from u8PtsSa, */
			if ((u8RspPTS < u8NextPts) && (u8NextPts != prSptRspStart->rTxInfo.u8PtsSa)) {
				u8ExtLen =
				    u8GapTime * prSptRspStart->rTxInfo.u8TxLen /
				    (u8NextPts - prSptRspStart->rTxInfo.u8PtsSa);
				prSptRspStart->rTxInfo.u8PtsSa = u8RspPTS;
				prSptRspStart->rTxInfo.u8FromFileOfst += u8ExtLen;
				prSptRspStart->rTxInfo.u8TxLen -= u8ExtLen;
				prAudAU->rAUInfo.rInfo.u8Pts =
				    u8RspPTS;
			} else {
				if (0 != rRspNext.rTxInfo.u8PtsSa) {
					*prSptRspStart = rRspNext;
					DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
						TEXT("[SPT] %s line %d Adjust to Next\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO);
				}
			}
		}
	}

	prSpt->fgCPSOn = FALSE;
	prSpt->ucRspTxAStmHdrState = RSP_TX_AHEADS_IDLE;

#if DMX_SPT_RSP_USING_DIVXDRM
	{
		CFA_DIVXDRM_INFO_T rDivxDRMInfo;

		mm_memset(&rDivxDRMInfo, 0, sizeof(rDivxDRMInfo));

		rDivxDRMInfo.fgOn = FALSE;
		rDivxDRMInfo.u8DecryptStOfst = DMX_INVALID_UINT64;
		rDivxDRMInfo.u4DecryptLen = 0;
		rDivxDRMInfo.u2FrameKeyIdx = DMX_DIVXDRM_INVALID_FRAMEIDX;

		prSpt->fgDivxDRMOn = FALSE;

		mrRet = SplitterPsrTurnDivxDRM(pvSptHdl, &rDivxDRMInfo);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s line %d -- fail in SplitterPsrTurnDivxDRM, ")
				TEXT("Also finish rsp, mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);

			mrRet = SplitterRspTxFinish(pvSptHdl);

			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
					TEXT("[SPT] %s line %d fail in SplitterRspTxFinish! ")
					TEXT("Make play finish(Send EOS) mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			}

			MM_RETURN(mrRet);
		}
	}
#endif				/* DMX_SPT_RSP_USING_DIVXDRM */

	if (NULL != prSptRspStart) {
		/* Active to transfer resplitter entry data to fifo */
		mrRet = SplitterRspSetRspTx(pvSptHdl, prSptRspStart);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s line %d fail in SetRspTx! Also finish rsp, ")
				TEXT("mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);

			mrRet = SplitterRspTxFinish(pvSptHdl);

			if (DMX_FAILED(mrRet))
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
					TEXT("[SPT] %s line %d fail in SplitterRspTxFinish!")
					TEXT(" Make play finish(Send EOS) mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);

			MM_RETURN(mrRet);
		}

		MM_RETURN(RET_DMX_OK);
	} else {
		mrRet = SplitterRspTxFinish(pvSptHdl);

		if (DMX_FAILED(mrRet))
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s line %d fail in SplitterRspTxFinish!")
				TEXT(" Make play finish(Send EOS) mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);

		MM_RETURN(mrRet);
	}
#endif				/* RSP_ENABLE */

	MM_RETURN(RET_DMX_OK);
}

/* Query Stage */
MRESULT SplitterRspGetRspRangeByOffset(void *pvSptHdl,
	u64 u8RspOffsetDelta, u64 *pu8RspStartOffset,
	u64 *pu8PbbStartOffset)
{
	SPT_RSP_T *prSptRspFirst = NULL;
	SPT_RSP_T *prSptRspPrev = NULL;
	u32 u4RspTxType = 0;
	u32 u4RspTxFtrUID = 0;
	u64 u8RspOffset = 0;

	if ((NULL == pu8RspStartOffset) || (NULL == pu8PbbStartOffset) || 
		(NULL == pvSptHdl))
		MM_RETURN(RET_DMX_PARAM_WRONG);

	/* TODO: Remove after Integrate */
	if (!SplitterRspIsEnabled(pvSptHdl))
		MM_RETURN(RET_DMX_OK);

	u8RspOffset = SplitterGetRspStartOffset(pvSptHdl);

	DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
		TEXT("[SPT] %s line %d -- pvSptHdl: 0x%x: ")
		TEXT(", RspOffset: " DMX_UINT64_16U_LOGSTR)
		TEXT(", RspOffsetDelta: " DMX_UINT64_16U_LOGSTR "!! \r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl,
		DMX_UINT64_16U_LOG_H(u8RspOffset),
		DMX_UINT64_16U_LOG_L(u8RspOffset),
		DMX_UINT64_16U_LOG_H(u8RspOffsetDelta),
		DMX_UINT64_16U_LOG_L(u8RspOffsetDelta));


	if (DMX_INVALID_UINT64 != u8RspOffset)
		u8RspOffset += u8RspOffsetDelta;

	prSptRspFirst = SplitterGetRspFirstTx(pvSptHdl);
	prSptRspPrev = NULL;
	u4RspTxType = SplitterGetRspTxType(pvSptHdl);
	u4RspTxFtrUID = GetStmUIDByType(pvSptHdl, u4RspTxType);

	while (NULL != prSptRspFirst) {
		if ((u4RspTxType == prSptRspFirst->rTxInfo.u4TxStreamType) &&
		    (u4RspTxFtrUID == prSptRspFirst->u4StreamUID) &&
		    (DMX_INVALID_UINT64 != prSptRspFirst->rTxInfo.u8FromFileOfst)) {
			if (prSptRspFirst->rTxInfo.u8FromFileOfst < u8RspOffset)
				prSptRspPrev = prSptRspFirst;
			else {
				if (NULL == prSptRspPrev)
					prSptRspPrev = prSptRspFirst;

				DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
					TEXT("[Spt] %s line %d -- StmType:%d, StmUID:0x%x")
					TEXT(", PtsSa: " DMX_PTS_LOGSTR ", PtsEa: " DMX_PTS_LOGSTR)
					TEXT(", Ofst: " DMX_UINT64_16U_LOGSTR "\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					prSptRspPrev->rTxInfo.u4TxStreamType,
					prSptRspPrev->rTxInfo.u4TxUID,
					DMX_PTS_LOG_MS(prSptRspPrev->rTxInfo.u8PtsSa),
					DMX_PTS_LOG_PTS(prSptRspPrev->rTxInfo.u8PtsSa),
					DMX_PTS_LOG_MS(prSptRspPrev->rTxInfo.u8PtsEa),
					DMX_PTS_LOG_PTS(prSptRspPrev->rTxInfo.u8PtsEa),
					DMX_UINT64_16U_LOG_H(prSptRspPrev->rTxInfo.u8FromFileOfst),
					DMX_UINT64_16U_LOG_L(prSptRspPrev->rTxInfo.u8FromFileOfst));

				break;
			}
		}
		prSptRspFirst = prSptRspFirst->prNextEntry;
	}

	if (NULL != prSptRspFirst) {
		*pu8RspStartOffset = prSptRspFirst->rTxInfo.u8FromFileOfst;
		if (DMX_INVALID_UINT64 != prSptRspFirst->u8CpsOffset)
			*pu8RspStartOffset = prSptRspFirst->u8CpsOffset;

		*pu8PbbStartOffset = SplitterGetPBBOffsetSa(pvSptHdl);
		DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
			TEXT("[Spt] %s line %d -- RspStartOfst= " DMX_UINT64_16U_LOGSTR)
			TEXT(", PbbStartOfst= " DMX_UINT64_16U_LOGSTR)
			TEXT(", FromFileOfst= " DMX_UINT64_16U_LOGSTR)
			TEXT(", RspOffset= " DMX_UINT64_16U_LOGSTR "\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			DMX_UINT64_16U_LOG_H(*pu8RspStartOffset),
			DMX_UINT64_16U_LOG_L(*pu8RspStartOffset),
			DMX_UINT64_16U_LOG_H(*pu8PbbStartOffset),
			DMX_UINT64_16U_LOG_L(*pu8PbbStartOffset),
			DMX_UINT64_16U_LOG_H(prSptRspFirst->rTxInfo.u8FromFileOfst),
			DMX_UINT64_16U_LOG_L(prSptRspFirst->rTxInfo.u8FromFileOfst),
			DMX_UINT64_16U_LOG_H(u8RspOffset),
			DMX_UINT64_16U_LOG_L(u8RspOffset));
		MM_RETURN(RET_DMX_OK);
	}

	*pu8PbbStartOffset = SplitterGetPBBOffsetSa(pvSptHdl);
	*pu8RspStartOffset = *pu8PbbStartOffset;
	DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
		TEXT("[Spt] %s --  no log au found, pvSptHdl:0x%p\n"), DMX_FUNC_NAME, pvSptHdl);

	MM_RETURN(RET_DMX_NO_RSP_LOGAU);
}


/*Get video PTS from 1st splitter*/
MRESULT SplitterRspSetRspEnableByOffset(void *pvSptHdl)
{
#if RSP_ENABLE
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;
	u64 u8RspOffset = DMX_INVALID_UINT64;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
			TEXT("[SPT] %s fail for invalid args!\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (SPLITTER_PTX_RSP_BY_OFFSET != prSpt->ucRspMode) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
			TEXT("[SPT] %s fail for invalid RspMode(%d)!\r\n"),
			DMX_FUNC_NAME, prSpt->ucRspMode);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	u8RspOffset = SplitterGetRspStartOffset(pvSptHdl);

	DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
		TEXT("[SPT] %s -- u8RspOffset: " DMX_UINT64_16U_LOGSTR "!! \r\n"),
		DMX_FUNC_NAME, DMX_UINT64_16U_LOG_H(u8RspOffset),
		DMX_UINT64_16U_LOG_L(u8RspOffset));

	if (!SplitterIsReResplitter(pvSptHdl)) {
		/* Record Last Tx Len to inform cfa tx len after resplitter finish */
		DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
			TEXT("[SPT] %s line %d -- LastPtxLen: ")
			TEXT(DMX_UINT64_16U_LOGSTR "!! \r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			DMX_UINT64_16U_LOG_H(SplitterGetPtxLen(pvSptHdl)),
			DMX_UINT64_16U_LOG_L(SplitterGetPtxLen(pvSptHdl)));
		SplitterSetRspLastPtxLen(pvSptHdl, SplitterGetPtxLen(pvSptHdl));
	}

	/* TODO: Remove after Integrate */
	if (!SplitterRspIsEnabled(pvSptHdl)) {
		DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
			TEXT("[SPT] %s exit, resplitter is disable!\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	u8RspOffset += SplitterGetRspOffsetDelta(pvSptHdl);

	/* Find the proper SPT_RSP_T according to the current PTS */
	{
		SPT_RSP_T *prSptRspPrev = NULL;
		SPT_RSP_T *prSptRspStart = SplitterGetRspFirstTx(pvSptHdl);
		u32 u4RspTxType = SplitterGetRspTxType(pvSptHdl);
		u32 u4RspTxFtrUID = GetStmUIDByType(pvSptHdl, u4RspTxType);

		while (NULL != prSptRspStart) {
			if ((u4RspTxFtrUID == prSptRspStart->u4StreamUID) &&
			    (u4RspTxType == prSptRspStart->rTxInfo.u4TxStreamType) &&
			    (DMX_INVALID_UINT64 != prSptRspStart->rTxInfo.u8FromFileOfst)) {
				if (prSptRspStart->rTxInfo.u8FromFileOfst < u8RspOffset)
					prSptRspPrev = prSptRspStart;
				else {
					if (NULL == prSptRspPrev)
						prSptRspPrev = prSptRspStart;
					break;
				}
			}
			prSptRspStart = prSptRspStart->prNextEntry;
		}

		SplitterSetRspState(pvSptHdl, SPLITTER_STATE_RSP_RSPING);

		prSpt->fgCPSOn = FALSE;
		prSpt->ucRspTxAStmHdrState = RSP_TX_AHEADS_IDLE;

#if DMX_SPT_RSP_USING_DIVXDRM
		{
			CFA_DIVXDRM_INFO_T rDivxDRMInfo;

			mm_memset(&rDivxDRMInfo, 0, sizeof(rDivxDRMInfo));

			rDivxDRMInfo.fgOn = FALSE;
			rDivxDRMInfo.u8DecryptStOfst = DMX_INVALID_UINT64;
			rDivxDRMInfo.u4DecryptLen = 0;
			rDivxDRMInfo.u2FrameKeyIdx = DMX_DIVXDRM_INVALID_FRAMEIDX;

			prSpt->fgDivxDRMOn = FALSE;

			mrRet = SplitterPsrTurnDivxDRM(pvSptHdl, &rDivxDRMInfo);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
					TEXT("[SPT] %s line %d fail in SplitterPsrTurnDivxDRM, ")
					TEXT("Also finish rsp, mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);

				mrRet = SplitterRspTxFinish(pvSptHdl);

				if (DMX_FAILED(mrRet))
					DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
						TEXT("[SPT] %s line %d fail in SplitterRspTxFinish! ")
						TEXT("Make play finish(Send EOS) mrRet: 0x%x!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, mrRet);

				MM_RETURN(mrRet);
			}
		}
#endif				/* DMX_SPT_RSP_USING_DIVXDRM */

		if (NULL != prSptRspStart) {
			DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
				TEXT("[SPT] %s line %d --  pvSptHdl: 0x%p, RspOffset: ")
				TEXT(DMX_UINT64_16U_LOGSTR "\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl,
				DMX_UINT64_16U_LOG_H(u8RspOffset),
				DMX_UINT64_16U_LOG_H(u8RspOffset));

			/* Active to transfer resplitter entry data to fifo */
			mrRet = SplitterRspSetRspTx(pvSptHdl, prSptRspStart);
			if (DMX_FAILED(mrRet)) {
				DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
					TEXT("[SPT] %s line %d fail in SetRspTx! ")
					TEXT("Also finish rsp, mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);

				mrRet = SplitterRspTxFinish(pvSptHdl);

				if (DMX_FAILED(mrRet))
					DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
						TEXT("[SPT] %s line %d fail in SplitterRspTxFinish! ")
						TEXT("Make play finish(Send EOS) mrRet: 0x%x!\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, mrRet);

				MM_RETURN(mrRet);
			}

			MM_RETURN(RET_DMX_OK);
		} else {
			mrRet = SplitterRspTxFinish(pvSptHdl);

			if (DMX_FAILED(mrRet))
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
					TEXT("[SPT] %s line %d fail in SplitterRspTxFinish! ")
					TEXT("Make play finish(Send EOS) mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);

			MM_RETURN(mrRet);
		}
	}
#endif				/* RSP_ENABLE */

	MM_RETURN(RET_DMX_OK);
}

/* ////////////////////////////////////////////////////////////////////////////// */
/* SplitterRspTxDone */
/* Search the next one resplitter entry which is the resplitter stream's entry */
/* in resplitter entrys fifo */
/* Active to TX the entry's data into FIFO */
/* If no this entry, Inform Resplitter Finish */
/* @Param u8TotalTxLen   [IN]   No Use */
/* ////////////////////////////////////////////////////////////////////////////// */
MRESULT SplitterRspTxDone(void *pvSptHdl, u64 u8TotalTxLen)
{
#if RSP_ENABLE
	DMX_SPT_INST_T *prSpt = NULL;
	SPT_RSP_T *prSptRsp = NULL;
	SPT_RSP_T *prSptRspStart = NULL;
	MRESULT mrRet = RET_DMX_OK;

	UNUSE_PARAMETER(u8TotalTxLen);

	prSpt = (DMX_SPT_INST_T *) pvSptHdl;

	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[SPT] %s fail for invalid args!\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterRspIsEnabled(pvSptHdl)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[SPT] %s fail for Rsplitter is disable!\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OPERATE_FORBID);
	}

	prSptRsp = SplitterGetRspTx(pvSptHdl);
	if (NULL == prSptRsp) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[SPT] %s fail for RspTx handle is NULL!\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	prSptRspStart = prSptRsp->prNextEntry;

	/* Status change for inserting Audio adts header such as AAC and so on */
	if ((RSP_TX_AHEADS_TXING == prSpt->ucRspTxAStmHdrState) &&
	    (RSP_TXHDRINFO_SAMPLE_HEAD_TXING == prSpt->ucRspTxHdrState)) {
		prSptRspStart = prSptRsp;
		prSpt->ucRspTxAStmHdrState = RSP_TX_AHEADS_DONE;
	}

	if (RSP_TXHDRINFO_SAMPLE_HEAD_INIT == prSpt->ucRspTxHdrState) {
		prSptRspStart = prSptRsp;
		prSpt->ucRspTxHdrState = RSP_TXHDRINFO_SAMPLE_HEAD_TXING;
	}
#if DMX_SPT_RSP_USING_DIVXDRM
	if (DMX_INVALID_UINT64 != prSptRsp->u8DivxDRMOffset) {
		CFA_DIVXDRM_INFO_T rDivxDRMInfo;

		mm_memset(&rDivxDRMInfo, 0, sizeof(rDivxDRMInfo));

		rDivxDRMInfo.fgOn = FALSE;
		rDivxDRMInfo.u8DecryptStOfst = DMX_INVALID_UINT64;
		rDivxDRMInfo.u4DecryptLen = 0;
		rDivxDRMInfo.u2FrameKeyIdx = DMX_DIVXDRM_INVALID_FRAMEIDX;

		prSpt->fgDivxDRMOn = FALSE;

		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[RSP] %s line %d -- Turn off DivxDRM For RSP\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);

		mrRet = SplitterPsrTurnDivxDRM(pvSptHdl, &rDivxDRMInfo);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s line %d fail in SplitterPsrTurnDivxDRM, ")
				TEXT("Also finish rsp, mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);

			mrRet = SplitterRspTxFinish(pvSptHdl);

			if (DMX_FAILED(mrRet))
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
					TEXT("[SPT] %s line %d fail in SplitterRspTxFinish! ")
					TEXT("Make play finish(Send EOS) mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);

			MM_RETURN(mrRet);
		}
	}
#endif				/* DMX_SPT_RSP_USING_DIVXDRM */

	/* If ftr uid or rsp tx type is not matched, change to next entry */
	do {
		if (NULL == prSptRspStart)
			break;

		if ((GetStmUIDByType(pvSptHdl, SplitterGetRspTxType(pvSptHdl)) !=
		     prSptRspStart->u4StreamUID)
		    || (SplitterGetRspTxType(pvSptHdl) !=
			(u8) (prSptRspStart->rTxInfo.u4TxStreamType)))
			prSptRspStart = prSptRspStart->prNextEntry;
		else {
			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s line %d -- pvSptHdl: 0x%p, SptRsp: 0x%x")
				TEXT(", FromFileOfst: " DMX_UINT64_16U_LOGSTR)
				TEXT(", StreamUID(%d), StreamType(%d), fgAUByEnd(%d), ")
				TEXT("fgCreateAU(%d)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				pvSptHdl, prSptRspStart,
				DMX_UINT64_16U_LOG_H(prSptRspStart->rTxInfo.u8FromFileOfst),
				DMX_UINT64_16U_LOG_L(prSptRspStart->rTxInfo.u8FromFileOfst),
				prSptRspStart->u4StreamUID,
				prSptRspStart->rTxInfo.u4TxStreamType,
				prSptRspStart->rTxInfo.fgAUByEnd,
				prSptRspStart->rTxInfo.fgCreateAU);

			if ((prSpt->fgAUCtrl) &&
			    (prSptRspStart->rTxInfo.fgAUByEnd) &&
			    (!prSptRspStart->rTxInfo.fgCreateAU))
				prSptRspStart = prSptRspStart->prNextEntry;
			else
				break;
		}
	} while (TRUE);

	if (NULL != prSptRspStart) {
		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[SPT] %s line %d -- pvSptHdl: 0x%p")
			TEXT(", Ofst: " DMX_UINT64_16U_LOGSTR)
			TEXT(", len: " DMX_UINT64_16U_LOGSTR)
			TEXT(", eFilledType: %d, u8PtsSa: " DMX_PTS_LOGSTR)
			TEXT(", fgCreateAU: %d, StmType: %d, pvAU: 0x%08x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl,
			DMX_UINT64_16U_LOG_H(prSptRspStart->rTxInfo.u8FromFileOfst),
			DMX_UINT64_16U_LOG_L(prSptRspStart->rTxInfo.u8FromFileOfst),
			DMX_UINT64_16U_LOG_H(prSptRspStart->rTxInfo.u8TxLen),
			DMX_UINT64_16U_LOG_L(prSptRspStart->rTxInfo.u8TxLen),
			prSptRspStart->eFilledType,
			DMX_PTS_LOG_MS(prSptRspStart->rTxInfo.u8PtsSa),
			DMX_PTS_LOG_PTS(prSptRspStart->rTxInfo.u8PtsSa),
			((prSptRspStart->rTxInfo.fgCreateAU) ? 1 : 0),
			prSptRspStart->rTxInfo.u4TxStreamType,
			(u32) (prSptRspStart->pvAU));

		mrRet = SplitterRspSetRspTx(pvSptHdl, prSptRspStart);
		if (DMX_FAILED(mrRet))
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s line %d fail in SplitterRspSetRspTx! ")
				TEXT("Make play finish(Send EOS) mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);

		MM_RETURN(mrRet);
	} else {
		mrRet = SplitterRspTxFinish(pvSptHdl);
		if (DMX_FAILED(mrRet))
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s line %d fail in SplitterRspTxFinish! ")
				TEXT("Make play finish(Send EOS) mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			MM_RETURN(mrRet);

		MM_RETURN(mrRet);
	}

#endif				/* RSP_ENABLE */

	MM_RETURN(RET_DMX_OK);
}

/* Record PTS of current AU */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
/* SplitterRspSetLogAu */
/* Copy the AU Info(which don't need to TX) into the Resplitter Entry's AU */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
MRESULT SplitterRspSetLogAu(void *pvSptHdl, void *pvAUInf)
{
#if RSP_ENABLE
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;
	SPT_RSP_T *prSptRsp = NULL;
	void *pvMyAuInfo = NULL;
	u32 u4TxStreamType = 0;
	u32 u4MyAuSize;

	/* TODO: Remove after Integrate */
	if (!SplitterRspIsEnabled(pvSptHdl))
		MM_RETURN(RET_DMX_OK);

	if (SPLITTER_STATE_RSP_LOGING != SplitterGetRspState(pvSptHdl))
		MM_RETURN(RET_DMX_OK);

	if (NULL == pvAUInf) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for pvAUinf is NULL!\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prSptRsp = (SPT_RSP_T *) prSpt->pvRspLastTx;

	if (NULL == prSptRsp)
		MM_RETURN(RET_DMX_OK);

	if (SPLITTER_RSP_ENTRY_TX_DONE != prSptRsp->eFilledType)
		MM_RETURN(RET_DMX_OK);	/*It might be an error */

	u4TxStreamType = prSptRsp->rTxInfo.u4TxStreamType;

	switch (u4TxStreamType) {
	case SPT_DATA_A:
		u4MyAuSize = sizeof(AU_AUDIO);
		break;

	case SPT_DATA_SP:
		u4MyAuSize = sizeof(AU_SP);
		break;

	default:
		MM_RETURN(RET_DMX_OK);
	}

	pvMyAuInfo = (AU_AUDIO *) SplitterRspGetAu(pvSptHdl);

	if ((NULL == pvMyAuInfo) || (0 == u4MyAuSize)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s -- pvMyAuInfo(0x%x) or u4MyAuSize(%d) is error!\r\n"),
			DMX_FUNC_NAME, pvMyAuInfo, u4MyAuSize);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (0 != u4MyAuSize) {
		if (SPT_DATA_A == u4TxStreamType) {
			if (g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_AUD])
				DmxDumpASample(pvAUInf, 0, 0, prSptRsp->u4StreamUID, TRUE);
		} else {
			pvMyAuInfo = (AU_SP *) SplitterRspGetAu(pvSptHdl);
			if (g_rDmxCliMan.fgAVDumpFrameData[DMX_CLI_STM_SP])
				DmxDumpSPSample(pvAUInf, 0, 0, prSptRsp->u4StreamUID, TRUE);
		}

		dmx_memcpy(pvMyAuInfo, pvAUInf, u4MyAuSize);
	}

	prSptRsp->pvAU = pvMyAuInfo;

	prSptRsp->u2AUSz = (u16) u4MyAuSize;

	prSptRsp->eFilledType = SPLITTER_RSP_ENTRY_AU_DONE;

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
		TEXT("[SPT] %s line %d, StmType: %d, TxUID: %d")
		TEXT(", FromFileOfst: " DMX_UINT64_16U_LOGSTR)
		TEXT(", Pts: " DMX_PTS_LOGSTR "!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		prSptRsp->rTxInfo.u4TxStreamType,
		prSptRsp->rTxInfo.u4TxUID,
		DMX_UINT64_16U_LOG_H(prSptRsp->rTxInfo.u8FromFileOfst),
		DMX_UINT64_16U_LOG_L(prSptRsp->rTxInfo.u8FromFileOfst),
		DMX_PTS_LOG_MS(prSptRsp->rTxInfo.u8PtsSa),
		DMX_PTS_LOG_PTS(prSptRsp->rTxInfo.u8PtsSa));

#endif				/* RSP_ENABLE */

	MM_RETURN(RET_DMX_OK);

}

/* ////////////////////////////////////////////////////////////////////////////////////////////// */
/* SplitterRspSetRspAu */
/* Copy the AU Info from the Current TX Resplitter AU into the TX AU */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
MRESULT SplitterRspSetRspAu(void *pvSptHdl, void *pvAUinf)
{
#if RSP_ENABLE
	SPT_RSP_T *prSptRsp = NULL;
	void *pvMyAuInfo = NULL;
	u32 u4TxStreamType = DMX_INVALID_UINT32;
	u16 u2MyAuSize = 0;

	if ((NULL == pvAUinf) || (NULL == pvSptHdl)) {
		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for invalid args!\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	/* TODO: Remove after Integrate */
	if (!SplitterRspIsEnabled(pvSptHdl))
		MM_RETURN(RET_DMX_OK);

	prSptRsp = SplitterGetRspTx(pvSptHdl);

	if (NULL == prSptRsp) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for no rsp entry(pvSptHdl: 0x%p)!\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	u2MyAuSize = prSptRsp->u2AUSz;
	pvMyAuInfo = prSptRsp->pvAU;
	u4TxStreamType = prSptRsp->rTxInfo.u4TxStreamType;

	if (NULL == pvMyAuInfo) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	if (NULL == &prSptRsp->rTxInfo) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_UNEXPECT);
	}

	switch (u4TxStreamType) {
	case SPT_DATA_A:
		u2MyAuSize = sizeof(AudInfo);
		break;

	case SPT_DATA_SP:
		u2MyAuSize = sizeof(SPicInfo);
		break;
	default:
		u2MyAuSize = 0;
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
		break;
	}

	if (0 != u2MyAuSize) {	/* Rsping only can fill data info, the fifo info is forbidden 080417 */
		if (SPT_DATA_A == u4TxStreamType) {
			dmx_memcpy(&(((AU_AUDIO *) pvAUinf)->rAUInfo.rInfo),
				   &(((AU_AUDIO *) pvMyAuInfo)->rAUInfo.rInfo),
				   (u32) u2MyAuSize);
			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s -- pvSptHdl: 0x%p, Audio RspAU's ")
				TEXT("Pts: "DMX_PTS_LOGSTR", RspEntry: 0x%x!\r\n"),
				DMX_FUNC_NAME, pvSptHdl,
				DMX_PTS_LOG_MS(((AU_AUDIO *) pvAUinf)->rAUInfo.rInfo.u8Pts),
				DMX_PTS_LOG_PTS(((AU_AUDIO *) pvAUinf)->rAUInfo.rInfo.u8Pts),
				prSptRsp);
		} else if (SPT_DATA_SP == u4TxStreamType) {
			((AU_SP *) pvAUinf)->rAUInfo.rInfo.u8StartPts =
			    ((AU_SP *) pvMyAuInfo)->rAUInfo.rInfo.u8StartPts;
			((AU_SP *) pvAUinf)->rAUInfo.rInfo.u8EndPts =
			    ((AU_SP *) pvMyAuInfo)->rAUInfo.rInfo.u8EndPts;
			((AU_SP *) pvAUinf)->rAUInfo.rInfo.u8Dts =
			    ((AU_SP *) pvMyAuInfo)->rAUInfo.rInfo.u8Dts;
			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s -- pvSptHdl: 0x%p, SP RspAU's StartPts: ")
				TEXT(DMX_PTS_LOGSTR ", EndPts: " DMX_PTS_LOGSTR)
				TEXT(", RspEntry: 0x%x!\r\n"),
				DMX_FUNC_NAME, pvSptHdl,
				DMX_PTS_LOG_MS(((AU_SP *) pvAUinf)->rAUInfo.rInfo.u8StartPts),
				DMX_PTS_LOG_PTS(((AU_SP *) pvAUinf)->rAUInfo.rInfo.u8StartPts),
				DMX_PTS_LOG_MS(((AU_SP *) pvAUinf)->rAUInfo.rInfo.u8EndPts),
				DMX_PTS_LOG_PTS(((AU_SP *) pvAUinf)->rAUInfo.rInfo.u8EndPts),
				prSptRsp);
		}
	}

	MM_RETURN(RET_DMX_OK);

#endif				/* RSP_ENABLE */

}

MRESULT SplitterRspSetLogTxBuf2Fifo(void *pvSptHdl,
	DMX_SPT_DMA2FIFO_INFO_T *prInf)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;
	SPT_RSP_T *prSptRsp = NULL;
	u32 u4CircularCnt = 0;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prInf) || (NULL == prSpt)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for invalid args!\r\n"), DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	while ((NULL != prInf->pvFromAddress) && (u4CircularCnt < 10)) {
		prSptRsp = (SPT_RSP_T *) SplitterRspGetRsp(pvSptHdl);
		if (NULL != prSptRsp) {
			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d -- eFilledType(%d), ")
				TEXT("rAHeadInfo.pvAddr(%p), u4Size(0x%08x)!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp->eFilledType,
				prSptRsp->rAHeadInfo.pvAddr, prSptRsp->rAHeadInfo.u4Size);
		}

		if (NULL == prSptRsp) {
			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d -- SplitterRspNew, prSptRsp: 0x%p!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp);
			/* Starting  to alloc a new spt struct to store the newest audio or sp inf */
			mrRet = SplitterRspNew(pvSptHdl);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s fail in SplitterRspNew, mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, mrRet);
				MM_RETURN(mrRet);
			}
			u4CircularCnt++;
			continue;
		} else if (SPLITTER_RSP_ENTRY_TX_DONE == prSptRsp->eFilledType) {
			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d -- SplitterRspNew, prSptRsp: 0x%p!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp);

			/* Starting  to alloc a new spt struct to store the newest audio or sp inf */
			mrRet = SplitterRspNew(pvSptHdl);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s fail in SplitterRspNew, mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, mrRet);
				MM_RETURN(mrRet);
			}

			u4CircularCnt++;
			continue;
		}

		if ((prSptRsp->rAHeadInfo.pvAddr != 0) &&
			(prSptRsp->rAHeadInfo.u4Size != 0)) {
			prSptRsp->eFilledType = SPLITTER_RSP_ENTRY_TX_DONE;

			mrRet = SplitterRspAddLog(pvSptHdl, prSptRsp);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s line %d fail in SplitterRspAddLog, ")
					TEXT("mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}

			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d -- SplitterRspAddLog(StmUID: %d, ")
				TEXT("StmType: %s, u8PtsSa: " DMX_PTS_LOGSTR ")\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				prSptRsp->rTxInfo.u4TxUID,
				DMX_SPTDATATYPE_STR(prSptRsp->rTxInfo.u4TxStreamType),
				DMX_PTS_LOG_MS(prSptRsp->rTxInfo.u8PtsSa),
				DMX_PTS_LOG_PTS(prSptRsp->rTxInfo.u8PtsSa));

			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d -- SplitterRspNew, prSptRsp: 0x%p!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp);

			/* Starting  to alloc a new spt struct to store the newest audio or sp inf */
			mrRet = SplitterRspNew(pvSptHdl);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s fail in SplitterRspNew, mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, mrRet);
				MM_RETURN(mrRet);
			}

			u4CircularCnt++;
			continue;
		}

		mrRet = SplitterRspLogSampleHdr(prSpt,
						&(prSptRsp->rAHeadInfo),
						prInf->pvFromAddress, prInf->u8TxLen);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d fail in SplitterRspLogSampleHdr ")
				TEXT("(pvSptHdl 0x%p, mrRet: 0x%x, u4Size: %d)!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet, prInf->u8TxLen);
			MM_RETURN(mrRet);
		}

		prSptRsp->rTxInfo.u4TxStreamType = prInf->u4TxStreamType;
		prSptRsp->rTxInfo.u4TxUID = prInf->u4TxUID;
		prSptRsp->rTxInfo.u4TxAudioCodec = prInf->u4TxAudioCodec;
		prSptRsp->rTxInfo.u4TxPictureMode = prInf->u4TxPictureMode;
		prSptRsp->rTxInfo.u8PtsSa = prInf->u8PtsSa;
		prSptRsp->rTxInfo.u8PtsEa = prInf->u8PtsEa;
		prSptRsp->u4StreamUID = prInf->u4TxUID;

		MM_RETURN(RET_DMX_OK);
	}

	DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
		TEXT("[SPT] %s fail for the new rspentry's rAHeadInfo's ")
		TEXT("u4Addr should be 0!\r\n"),
		DMX_FUNC_NAME);

	MM_RETURN(RET_DMX_UNEXPECT);
}

MRESULT SplitterRspSetLogTxPbb2FifoNormal(void *pvSptHdl,
	DMX_SPT_DMA2FIFO_INFO_T *prInf)
{
	RSP_HDR_MEM_NODE rSampleHdrNode;
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;
	SPT_RSP_T *prSptRsp = NULL;
	DMX_SPT_DMA2FIFO_INFO_T *prMyTxInfo = NULL;
	u32 u4CircularCnt = 0;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prInf) || (NULL == prSpt)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for invalid args!\r\n"), DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prSptRsp = (SPT_RSP_T *) SplitterRspGetRsp(pvSptHdl);
	while ((NULL == prSptRsp) ||
			(SPLITTER_RSP_ENTRY_TX_DONE == prSptRsp->eFilledType) ||
			((prSptRsp->rAHeadInfo.pvAddr != 0) &&
			(prSptRsp->rAHeadInfo.u4Size != 0))) {
		prSptRsp = (SPT_RSP_T *) SplitterRspGetRsp(pvSptHdl);
		if (NULL != prSptRsp) {
			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s -- eFilledType(%d), rAHeadInfo.pvAddr(%p), ")
				TEXT("u4Size(0x%08x)!\r\n"),
				DMX_FUNC_NAME, prSptRsp->eFilledType,
				prSptRsp->rAHeadInfo.pvAddr,
				prSptRsp->rAHeadInfo.u4Size);
		}
		if (u4CircularCnt > 4) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s fail for the new rspentry's rAHeadInfo's")
				TEXT(" u4Addr should be 0!\r\n"),
				DMX_FUNC_NAME);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		if (NULL == prSptRsp) {
			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d -- SplitterRspNew, prSptRsp: 0x%p!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp);
			/* Starting  to alloc a new spt struct to store the newest audio or sp inf */
			mrRet = SplitterRspNew(pvSptHdl);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s fail in SplitterRspNew, mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, mrRet);
				MM_RETURN(mrRet);
			}
			u4CircularCnt++;
			continue;
		} else if (SPLITTER_RSP_ENTRY_TX_DONE == prSptRsp->eFilledType) {
			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d -- SplitterRspNew, prSptRsp: 0x%p!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp);

			/* Starting  to alloc a new spt struct to store the newest audio or sp inf */
			mrRet = SplitterRspNew(pvSptHdl);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s fail in SplitterRspNew, mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, mrRet);
				MM_RETURN(mrRet);
			}

			u4CircularCnt++;
			continue;
		}

		if ((prSptRsp->rAHeadInfo.pvAddr != 0) &&
		    (prSptRsp->rAHeadInfo.u4Size != 0) &&
		    ((prSptRsp->rTxInfo.u4TxStreamType != prInf->u4TxStreamType) ||
		     (prSptRsp->u4StreamUID != prInf->u4TxUID))) {
			prSptRsp->eFilledType = SPLITTER_RSP_ENTRY_TX_DONE;

			mrRet = SplitterRspAddLog(pvSptHdl, prSptRsp);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s line %d fail in SplitterRspAddLog, ")
					TEXT("mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}

			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d -- SplitterRspAddLog(StmUID: %d, ")
				TEXT("StmType: %s, u8PtsSa: " DMX_PTS_LOGSTR ")\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				prSptRsp->rTxInfo.u4TxUID,
				DMX_SPTDATATYPE_STR(prSptRsp->rTxInfo.u4TxStreamType),
				DMX_PTS_LOG_MS(prSptRsp->rTxInfo.u8PtsSa),
				DMX_PTS_LOG_PTS(prSptRsp->rTxInfo.u8PtsSa));

			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d -- SplitterRspNew, prSptRsp: 0x%p!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp);
			/* Starting  to alloc a new spt struct to store the newest audio or sp inf */
			mrRet = SplitterRspNew(pvSptHdl);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s line %d fail in SplitterRspNew, ")
					TEXT("mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}
			u4CircularCnt++;
			continue;
		}

		break;
	}

	/* backup the info of the audio header store location and length */
#if ENABLE_DMX_ADVANCED_VER
	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
		TEXT("[SPT] %s -- StmType(%d), UID(%d)")
		TEXT(", FileOfst( " DMX_UINT64_16U_LOGSTR)
		TEXT("), TxLen(" DMX_UINT64_16U_LOGSTR)
		TEXT("), UseCmdQ(%d), InstHdr(%d), rAHdrInfo.u4Addr(%p)!\r\n"),
		DMX_FUNC_NAME,
		prInf->u4TxStreamType,
		prInf->u4TxUID,
		DMX_UINT64_16U_LOG_H(prInf->u8FromFileOfst),
		DMX_UINT64_16U_LOG_L(prInf->u8FromFileOfst),
		DMX_UINT64_16U_LOG_H(prInf->u8TxLen),
		DMX_UINT64_16U_LOG_L(prInf->u8TxLen),
		((prInf->fgUseCmdQ) ? 1 : 0),
		((prInf->fgInsertHdr) ? 1 : 0),
		prSptRsp->rAHeadInfo.pvAddr);

	if ((prInf->fgInsertHdr) && (prInf->u4InsertHdrLen > 0)) {
		prSptRsp = (SPT_RSP_T *) SplitterRspGetRsp(pvSptHdl);

		if ((prSptRsp->rAHeadInfo.pvAddr != 0) &&
			(prSptRsp->rAHeadInfo.u4Size != 0)) {
			prSptRsp->eFilledType = SPLITTER_RSP_ENTRY_TX_DONE;
			prSptRsp->u4StreamUID = prInf->u4TxUID;

			mrRet = SplitterRspAddLog(pvSptHdl, prSptRsp);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s line %d fail in SplitterRspAddLog, ")
					TEXT("mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}

			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d -- SplitterRspAddLog(StmUID: %d, ")
				TEXT("StmType: %s, u8PtsSa: " DMX_PTS_LOGSTR ")\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				prSptRsp->rTxInfo.u4TxUID,
				DMX_SPTDATATYPE_STR(prSptRsp->rTxInfo.u4TxStreamType),
				DMX_PTS_LOG_MS(prSptRsp->rTxInfo.u8PtsSa),
				DMX_PTS_LOG_PTS(prSptRsp->rTxInfo.u8PtsSa));

			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d -- SplitterRspNew, prSptRsp: 0x%p!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp);
			/* Starting  to alloc a new spt struct to store the newest audio or sp inf */
			mrRet = SplitterRspNew(pvSptHdl);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s line %d fail in SplitterRspNew, ")
					TEXT("mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}

			prSptRsp = (SPT_RSP_T *) SplitterRspGetRsp(pvSptHdl);

			if (NULL == prSptRsp) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s line %d fail in SplitterRspGetRsp!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO);
				MM_RETURN(RET_DMX_ERR_STATE);
			}
		}

		mrRet = SplitterRspLogSampleHdr((void *)prSpt,
				&(prSptRsp->rAHeadInfo),
				(void *)(prInf->pu1InsertHdrBuf), prInf->u4InsertHdrLen);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d fail in SplitterRspLogSampleHdr ")
				TEXT("(pvSptHdl 0x%p, mrRet: 0x%x, u4Size: %d)!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet, prInf->u4InsertHdrLen);
			MM_RETURN(mrRet);
		}

		prSptRsp->rTxInfo.u4TxStreamType = prInf->u4TxStreamType;
		prSptRsp->rTxInfo.u4TxUID = prInf->u4TxUID;
		prSptRsp->rTxInfo.u4TxAudioCodec = prInf->u4TxAudioCodec;
		prSptRsp->rTxInfo.u4TxPictureMode = prInf->u4TxPictureMode;
		prSptRsp->rTxInfo.u8PtsSa = prInf->u8PtsSa;
		prSptRsp->rTxInfo.u8PtsEa = prInf->u8PtsEa;
		prSptRsp->u4StreamUID = prInf->u4TxUID;
	}
#else				/* ENABLE_DMX_ADVANCED_VER */

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
		TEXT("[SPT] %s -- StmType(%d), UID(%d)")
		TEXT(", FileOfst( " DMX_UINT64_16U_LOGSTR)
		TEXT("), TxLen(" DMX_UINT64_16U_LOGSTR)
		TEXT("), UseCmdQ(%d), rAHdrInfo.u4Addr(%p)!\r\n"),
		DMX_FUNC_NAME,
		prInf->u4TxStreamType,
		prInf->u4TxUID,
		DMX_UINT64_16U_LOG_H(prInf->u8FromFileOfst),
		DMX_UINT64_16U_LOG_L(prInf->u8FromFileOfst),
		DMX_UINT64_16U_LOG_H(prInf->u8TxLen),
		DMX_UINT64_16U_LOG_L(prInf->u8TxLen),
		((prInf->fgUseCmdQ) ? 1 : 0), prSptRsp->rAHeadInfo.pvAddr);
#endif				/* ENABLE_DMX_ADVANCED_VER */

	prMyTxInfo = &(prSptRsp->rTxInfo);

	if (NULL != prSpt->pvRspFirstTx)
		DMX_ASSERT(prSpt->pvRspFirstTx == (void *)(prSpt->pvRspRp));

	mm_memcpy(&rSampleHdrNode, &(prSptRsp->rAHeadInfo),
		sizeof(RSP_HDR_MEM_NODE));

	dmx_memset(prSptRsp, 0, sizeof(SPT_RSP_T));

	dmx_memcpy(&(prSptRsp->rAHeadInfo), &rSampleHdrNode,
		sizeof(RSP_HDR_MEM_NODE));

	dmx_memcpy(prMyTxInfo, prInf, sizeof(DMX_SPT_DMA2FIFO_INFO_T));

	if (NULL != prSpt->pvRspFirstTx)
		DMX_ASSERT(prSpt->pvRspFirstTx == (void *)(prSpt->pvRspRp));

	prSptRsp->eFilledType = SPLITTER_RSP_ENTRY_TX_DONE;
	prSptRsp->u4StreamUID = prInf->u4TxUID;

#if DMX_SPT_RSP_USING_DIVXDRM
	prSptRsp->u8DivxDRMOffset = prSpt->u8DivxDRMOffset;
	prSptRsp->u4DecLen = prSpt->u4DecLen;
	prSptRsp->u2FrameKeyIndex = prSpt->u2FrameKeyIndex;

	/* clear it after use */
	prSpt->u8DivxDRMOffset = DMX_INVALID_UINT64;
	prSpt->u4DecLen = 0;
	prSpt->u2FrameKeyIndex = DMX_DIVXDRM_INVALID_FRAMEIDX;
#endif

	mrRet = SplitterRspAddLog(pvSptHdl, prSptRsp);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s line %d fail in SplitterRspAddLog, mrRet: 0x%x\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
		TEXT("[SPT] %s line %d -- SplitterRspAddLog(StmUID: %d, ")
		TEXT("StmType: %s, u8PtsSa: " DMX_PTS_LOGSTR ")\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		prSptRsp->rTxInfo.u4TxUID,
		DMX_SPTDATATYPE_STR(prSptRsp->rTxInfo.u4TxStreamType),
		DMX_PTS_LOG_MS(prSptRsp->rTxInfo.u8PtsSa),
		DMX_PTS_LOG_PTS(prSptRsp->rTxInfo.u8PtsSa));

	if (NULL != prSpt->pvRspFirstTx)
		DMX_ASSERT(prSpt->pvRspFirstTx == (void *)(prSpt->pvRspRp));

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
		TEXT("[SPT] %s line %d -- SplitterRspNew, prSptRsp: 0x%p!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp);

	/* Starting  to alloc a new spt struct to store the newest audio or sp inf */
	mrRet = SplitterRspNew(pvSptHdl);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s line %d fail in SplitterRspNew, mrRet: 0x%x!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		MM_RETURN(mrRet);
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT SplitterRspSetLogTxPbb2FifoCmdQ(void *pvSptHdl,
	DMX_SPT_DMA2FIFO_INFO_T *prInf)
{
	RSP_HDR_MEM_NODE rSampleHdrNode;
	SPT_RSP_T *prSptRsp = NULL;
	DMX_CMDQ_TX_ENTRY_T *prCmdQEntry = NULL;
	DMX_SPT_DMA2FIFO_INFO_T *prMyTxInfo = NULL;
	u64 u8FromFileOfst = 0;
	u32 u4EntryIdx = 0;
	u32 u4CircularCnt = 0;
	AU_AUDIO rAudioAU;
#if ENABLE_DMX_ADVANCED_VER
	PSR_AU rAuData;
#endif
	u64 u8StartPts = INVALID_TIMESTAMP;
	bool fgUnitStart = FALSE;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prInf) || (NULL == pvSptHdl)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for invalid args!\r\n"), DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prSptRsp = (SPT_RSP_T *) SplitterRspGetRsp(pvSptHdl);

	while ((NULL == prSptRsp) ||
			(SPLITTER_RSP_ENTRY_TX_DONE == prSptRsp->eFilledType) ||
			((prSptRsp->rAHeadInfo.pvAddr != 0) &&
			(prSptRsp->rAHeadInfo.u4Size != 0))) {
		prSptRsp = (SPT_RSP_T *) SplitterRspGetRsp(pvSptHdl);
		if (NULL != prSptRsp) {
			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s -- eFilledType(%d), rAHeadInfo.pvAddr(%p),")
				TEXT(" u4Size(0x%08x)!\r\n"),
				DMX_FUNC_NAME, prSptRsp->eFilledType, prSptRsp->rAHeadInfo.pvAddr,
				prSptRsp->rAHeadInfo.u4Size);
		}
		if (u4CircularCnt > 4) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s fail for the new rspentry's rAHeadInfo's ")
				TEXT("u4Addr should be 0!\r\n"),
				DMX_FUNC_NAME);
			MM_RETURN(RET_DMX_UNEXPECT);
		}

		if (NULL == prSptRsp) {
			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d -- SplitterRspNew, prSptRsp: 0x%p!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp);
			/* Starting  to alloc a new spt struct to store the newest audio or sp inf */
			mrRet = SplitterRspNew(pvSptHdl);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s fail in SplitterRspNew, mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, mrRet);
				MM_RETURN(mrRet);
			}
			u4CircularCnt++;
			continue;
		} else if (SPLITTER_RSP_ENTRY_TX_DONE == prSptRsp->eFilledType) {
			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d -- SplitterRspNew, prSptRsp: 0x%p!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp);

			/* Starting  to alloc a new spt struct to store the newest audio or sp inf */
			mrRet = SplitterRspNew(pvSptHdl);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s fail in SplitterRspNew, mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, mrRet);
				MM_RETURN(mrRet);
			}

			u4CircularCnt++;
			continue;
		}

		if ((prSptRsp->rAHeadInfo.pvAddr != 0) &&
		    (prSptRsp->rAHeadInfo.u4Size != 0) &&
		    ((prSptRsp->rTxInfo.u4TxStreamType != prInf->u4TxStreamType) ||
		     (prSptRsp->u4StreamUID != prInf->u4TxUID))) {
			prSptRsp->eFilledType = SPLITTER_RSP_ENTRY_TX_DONE;

			mrRet = SplitterRspAddLog(pvSptHdl, prSptRsp);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s line %d fail in SplitterRspAddLog,")
					TEXT(" mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}

			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d -- SplitterRspNew, prSptRsp: 0x%p!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp);
			/* Starting  to alloc a new spt struct to store the newest audio or sp inf */
			mrRet = SplitterRspNew(pvSptHdl);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s line %d fail in SplitterRspNew, ")
					TEXT("mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}

			u4CircularCnt++;
			continue;
		}

		break;
	}

	u8FromFileOfst = prInf->u8FromFileOfst;
	prCmdQEntry = prInf->parCmdQTxEntry;

	u4EntryIdx = 0;
	fgUnitStart = prInf->fgCreateAU;

	while (u4EntryIdx < prInf->u2TxEntryCnt) {
		prCmdQEntry = prInf->parCmdQTxEntry + u4EntryIdx;

		prSptRsp = (SPT_RSP_T *) SplitterRspGetRsp(pvSptHdl);
		if (NULL == prSptRsp) {
			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d -- SplitterRspNew, prSptRsp: 0x%p!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp);
			/* Starting  to alloc a new spt struct to store the newest audio or sp inf */
			mrRet = SplitterRspNew(pvSptHdl);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s fail in SplitterRspNew, mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, mrRet);
				MM_RETURN(mrRet);
			}
			continue;
		} else if (SPLITTER_RSP_ENTRY_TX_DONE == prSptRsp->eFilledType) {
			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d -- SplitterRspNew, prSptRsp: 0x%p!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp);

			/* Starting  to alloc a new spt struct to store the newest audio or sp inf */
			mrRet = SplitterRspNew(pvSptHdl);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s fail in SplitterRspNew, mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, mrRet);
				MM_RETURN(mrRet);
			}
			continue;
		}
		/* backup the info of the audio header store location and length */
#if ENABLE_DMX_ADVANCED_VER
		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s -- CmdQEntryIdx(%d), StmType(%d), UID(%d),")
			TEXT(" FileOfst("DMX_UINT64_16U_LOGSTR)
			TEXT("), TxLen(" DMX_UINT64_16U_LOGSTR)
			TEXT("), UseCmdQ(%d), InstHdr(%d), rAHdrInfo.u4Addr(%p)!\r\n"),
			DMX_FUNC_NAME, u4EntryIdx,
			prInf->u4TxStreamType, prInf->u4TxUID,
			DMX_UINT64_16U_LOG_H(prInf->u8FromFileOfst),
			DMX_UINT64_16U_LOG_L(prInf->u8FromFileOfst),
			DMX_UINT64_16U_LOG_H(prInf->u8TxLen),
			DMX_UINT64_16U_LOG_L(prInf->u8TxLen),
			((prInf->fgUseCmdQ) ? 1 : 0),
			((prInf->fgInsertHdr) ? 1 : 0), prSptRsp->rAHeadInfo.pvAddr);

		if ((prCmdQEntry->fgInsertHdr) && (prCmdQEntry->u4InsertHdrLen > 0)) {
			if ((prSptRsp->rAHeadInfo.pvAddr != 0) &&
			    (prSptRsp->rAHeadInfo.u4Size != 0)) {
				prSptRsp->eFilledType = SPLITTER_RSP_ENTRY_TX_DONE;

				mrRet = SplitterRspAddLog(pvSptHdl, prSptRsp);
				if (DMX_FAILED(mrRet)) {
					DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
						TEXT("[SPT] %s line %d fail in SplitterRspAddLog, ")
						TEXT("mrRet: 0x%x\r\n"),
						DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
					MM_RETURN(mrRet);
				}
				DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s line %d -- SplitterRspAddLog(StmUID: %d, ")
					TEXT("StmType: %s, u8PtsSa: " DMX_PTS_LOGSTR ")\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					prSptRsp->rTxInfo.u4TxUID,
					DMX_SPTDATATYPE_STR(prSptRsp->rTxInfo.u4TxStreamType),
					DMX_PTS_LOG_MS(prSptRsp->rTxInfo.u8PtsSa),
					DMX_PTS_LOG_PTS(prSptRsp->rTxInfo.u8PtsSa));

				mm_memset(&rAudioAU, 0, sizeof(rAudioAU));
				mm_memset(&rAuData, 0, sizeof(rAuData));
				rAudioAU.eAuType = AU_DATA;
				rAuData.eType = prInf->u4TxStreamType;
				rAuData.pvAUInf = &rAudioAU;
				rAuData.pvAUExtInf = NULL;
				mrRet = SplitterSetPsrAuTable(pvSptHdl, (void *) &rAuData);
				if (DMX_FAILED(mrRet)) {
					DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					    TEXT("[SPT] %s line %d fail in SplitterSetPsrAuTable")
						TEXT(" (pvSptHdl 0x%p, mrRet: 0x%x)!\r\n"),
					    DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
					MM_RETURN(mrRet);
				}

				DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT
					("[SPT] %s line %d -- SplitterRspNew, prSptRsp: 0x%p!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp);
				/* Starting  to alloc a new spt struct to store the newest audio or sp inf */
				mrRet = SplitterRspNew(pvSptHdl);
				if (DMX_FAILED(mrRet)) {
					DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
						TEXT("[SPT] %s fail in SplitterRspNew,")
						TEXT(" mrRet: 0x%x!\r\n"),
						DMX_FUNC_NAME, mrRet);
					MM_RETURN(mrRet);
				}

				continue;
			}

			mrRet = SplitterRspLogSampleHdr(pvSptHdl,
							&(prSptRsp->rAHeadInfo),
							(void *)(prCmdQEntry->au1InsertHdr),
							prCmdQEntry->u4InsertHdrLen);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s line %d fail in SplitterRspLogSampleHdr")
					TEXT(" (pvSptHdl 0x%p, mrRet: 0x%x, u4Size: %d)!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet,
					prCmdQEntry->u4InsertHdrLen);
				MM_RETURN(mrRet);
			}

			prSptRsp->rTxInfo.u4TxStreamType = prInf->u4TxStreamType;
			prSptRsp->rTxInfo.u4TxUID = prInf->u4TxUID;
			prSptRsp->rTxInfo.u4TxAudioCodec = prInf->u4TxAudioCodec;
			prSptRsp->rTxInfo.u4TxPictureMode = prInf->u4TxPictureMode;
			prSptRsp->rTxInfo.u8PtsSa = prInf->u8PtsSa;
			prSptRsp->rTxInfo.u8PtsEa = prInf->u8PtsEa;
			prSptRsp->u4StreamUID = prInf->u4TxUID;
		}
#else
		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s -- CmdQEntryIdx(%d), StmType(%d), UID(%d), ")
			TEXT("FileOfst("DMX_UINT64_16U_LOGSTR)
			TEXT("), TxLen(" DMX_UINT64_16U_LOGSTR)
			TEXT("), UseCmdQ(%d), rAHdrInfo.u4Addr(%p)!\r\n"),
			DMX_FUNC_NAME, u4EntryIdx,
			prInf->u4TxStreamType, prInf->u4TxUID,
			DMX_UINT64_16U_LOG_H(prInf->u8FromFileOfst),
			DMX_UINT64_16U_LOG_L(prInf->u8FromFileOfst),
			DMX_UINT64_16U_LOG_H(prInf->u8TxLen),
			DMX_UINT64_16U_LOG_L(prInf->u8TxLen),
			((prInf->fgUseCmdQ) ? 1 : 0), prSptRsp->rAHeadInfo.pvAddr);
#endif				/* ENABLE_DMX_ADVANCED_VER */

		prMyTxInfo = &(prSptRsp->rTxInfo);

		mm_memcpy(&rSampleHdrNode, &(prSptRsp->rAHeadInfo),
			sizeof(RSP_HDR_MEM_NODE));

		dmx_memset(prSptRsp, 0, sizeof(SPT_RSP_T));

		dmx_memcpy(&(prSptRsp->rAHeadInfo), &rSampleHdrNode,
			sizeof(RSP_HDR_MEM_NODE));

		dmx_memcpy(prMyTxInfo, prInf, sizeof(DMX_SPT_DMA2FIFO_INFO_T));

		prMyTxInfo->fgUseCmdQ = FALSE;
		prMyTxInfo->u2TxEntryCnt = 0;
		prMyTxInfo->parCmdQTxEntry = NULL;
		prMyTxInfo->u8FromFileOfst = u8FromFileOfst + prCmdQEntry->u4TxOfst;
		prMyTxInfo->u8TxLen = prCmdQEntry->u4TxLen;
		prMyTxInfo->u8RealTxLen = prMyTxInfo->u8TxLen;
		prMyTxInfo->fgCreateAU = fgUnitStart;
		prMyTxInfo->u8TotalAULen = prMyTxInfo->u8TxLen;
		prMyTxInfo->fgAUByCmdQEnd = FALSE;
		prMyTxInfo->fgAUByEnd = TRUE;
		prMyTxInfo->fgUnitEnd = FALSE;

		prSptRsp->rTxInfo.u4TxStreamType = prInf->u4TxStreamType;
		prSptRsp->rTxInfo.u4TxUID = prInf->u4TxUID;
		prSptRsp->rTxInfo.u4TxAudioCodec = prInf->u4TxAudioCodec;
		prSptRsp->rTxInfo.u4TxPictureMode = prInf->u4TxPictureMode;

		prSptRsp->eFilledType = SPLITTER_RSP_ENTRY_TX_DONE;
		prSptRsp->u4StreamUID = prInf->u4TxUID;

		u8FromFileOfst += prCmdQEntry->u4TxOfst + prCmdQEntry->u4TxLen;

		mrRet = SplitterRspAddLog(pvSptHdl, prSptRsp);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d fail in SplitterRspAddLog,")
				TEXT(" mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			MM_RETURN(mrRet);
		}

		if (fgUnitStart) {
			mm_memset(&rAudioAU, 0, sizeof(rAudioAU));
			rAudioAU.eAuType = AU_DATA;
			/* open in the future, when all CFA implemented */
			mrRet = SptCfaSetAUInfo(pvSptHdl, &rAudioAU, NULL);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				    TEXT("[SPT] %s fail in Cfa set AUInfo, mrRet: 0x%x.\r\n"),
				    DMX_FUNC_NAME, mrRet);
				MM_RETURN(mrRet);
			}

			prMyTxInfo->u8PtsSa = rAudioAU.rAUInfo.rInfo.u8Pts;
			u8StartPts = prMyTxInfo->u8PtsSa;
			prMyTxInfo->u8PtsEa = INVALID_TIMESTAMP;
		} else {
			prMyTxInfo->u8PtsSa = u8StartPts;
			prMyTxInfo->u8PtsEa = INVALID_TIMESTAMP;
		}

		fgUnitStart = FALSE;

		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s line %d -- SplitterRspAddLog(prSptRsp: 0x%p")
			TEXT(", StmUID: %d, StmType: %s, u8PtsSa: " DMX_PTS_LOGSTR)
			TEXT(", FromFileOfst: " DMX_UINT64_16U_LOGSTR)
			TEXT(", fgCreateAU: %d, eFilledType: %d, pvAU: 0x%p)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			prSptRsp,
			prSptRsp->rTxInfo.u4TxUID,
			DMX_SPTDATATYPE_STR(prSptRsp->rTxInfo.u4TxStreamType),
			DMX_PTS_LOG_MS(prMyTxInfo->u8PtsSa),
			DMX_PTS_LOG_PTS(prMyTxInfo->u8PtsSa),
			DMX_UINT64_16U_LOG_H(prSptRsp->rTxInfo.u8FromFileOfst),
			DMX_UINT64_16U_LOG_L(prSptRsp->rTxInfo.u8FromFileOfst),
			prSptRsp->rTxInfo.fgCreateAU,
			prSptRsp->eFilledType, (u32) (prSptRsp->pvAU));

		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s line %d -- SplitterRspNew, prSptRsp: 0x%p!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp);
		/* Starting  to alloc a new spt struct to store the newest audio or sp inf */
		mrRet = SplitterRspNew(pvSptHdl);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s fail in SplitterRspNew, mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, mrRet);
			MM_RETURN(mrRet);
		}

		if ((prCmdQEntry->fgEndAU) || (u4EntryIdx + 1 == prInf->u2TxEntryCnt)) {
			fgUnitStart = TRUE;

			prSptRsp = (SPT_RSP_T *) SplitterRspGetRsp(pvSptHdl);
			if (NULL != prSptRsp) {
				DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s -- eFilledType(%d), ")
					TEXT("rAHeadInfo.pvAddr(%p), u4Size(0x%08x)!\r\n"),
					DMX_FUNC_NAME, prSptRsp->eFilledType,
					prSptRsp->rAHeadInfo.pvAddr, prSptRsp->rAHeadInfo.u4Size);
			} else {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s fail in no available rsp log au,")
					TEXT(" mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, mrRet);
				MM_RETURN(mrRet);
			}

			prMyTxInfo = &(prSptRsp->rTxInfo);

			dmx_memset(prSptRsp, 0, sizeof(SPT_RSP_T));

			dmx_memcpy(prMyTxInfo, prInf, sizeof(DMX_SPT_DMA2FIFO_INFO_T));

			prMyTxInfo->fgUseCmdQ = FALSE;
			prMyTxInfo->u2TxEntryCnt = 0;
			prMyTxInfo->parCmdQTxEntry = NULL;
			prMyTxInfo->u8FromFileOfst = u8FromFileOfst;
			prMyTxInfo->u8TxLen = 0;
			prMyTxInfo->u8RealTxLen = 0;
			prMyTxInfo->fgCreateAU = FALSE;
			prMyTxInfo->u8TotalAULen = 0;
			prMyTxInfo->fgAUByCmdQEnd = FALSE;
			prMyTxInfo->fgAUByEnd = TRUE;
			prMyTxInfo->fgUnitEnd = TRUE;
			prMyTxInfo->u8PtsSa = u8StartPts;

			prSptRsp->eFilledType = SPLITTER_RSP_ENTRY_TX_DONE;
			prSptRsp->u4StreamUID = prInf->u4TxUID;

			mrRet = SplitterRspAddLog(pvSptHdl, prSptRsp);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s line %d fail in SplitterRspAddLog, ")
					TEXT("mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}

			mrRet = SplitterRspSetLogAu(pvSptHdl, &rAudioAU);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				    TEXT("[SPT] %s line %d fail in SplitterRspSetLogAu,")
					TEXT(" mrRet: 0x%x.\r\n"),
				    DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
				MM_RETURN(mrRet);
			}

			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d -- SplitterRspAddLog(prSptRsp: 0x%p")
				TEXT(", StmUID: %d, StmType: %s, u8PtsSa: " DMX_PTS_LOGSTR)
				TEXT(", FromFileOfst: " DMX_UINT64_16U_LOGSTR)
				TEXT(", fgCreateAU: %d, eFilledType: %d, pvAU: 0x%08x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				prSptRsp,
				prSptRsp->rTxInfo.u4TxUID,
				DMX_SPTDATATYPE_STR(prSptRsp->rTxInfo.u4TxStreamType),
				DMX_PTS_LOG_MS(prMyTxInfo->u8PtsSa),
				DMX_PTS_LOG_PTS(prMyTxInfo->u8PtsSa),
				DMX_UINT64_16U_LOG_H(prSptRsp->rTxInfo.u8FromFileOfst),
				DMX_UINT64_16U_LOG_L(prSptRsp->rTxInfo.u8FromFileOfst),
				prSptRsp->rTxInfo.fgCreateAU,
				prSptRsp->eFilledType, (u32) (prSptRsp->pvAU));

			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d -- SplitterRspNew, prSptRsp: 0x%p!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp);

			/* Starting  to alloc a new spt struct to store the newest audio or sp inf */
			mrRet = SplitterRspNew(pvSptHdl);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s fail in SplitterRspNew, mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, mrRet);
				MM_RETURN(mrRet);
			}
		}
		u4EntryIdx++;
	}

	MM_RETURN(RET_DMX_OK);
}

MRESULT SplitterRspSetLogTx(void *pvSptHdl,
	DMX_SPT_DMA2FIFO_INFO_T *prInf)
{
#if RSP_ENABLE
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prInf) || (NULL == prSpt)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for invalid args!\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterRspIsEnabled(pvSptHdl)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for Rsplitter is disable!\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_OK);
	}

	if (SPLITTER_STATE_RSP_LOGING != SplitterGetRspState(pvSptHdl)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for Rsp State(0x%x) is not ")
			TEXT("SPLITTER_STATE_RSP_LOGING!\r\n"),
			DMX_FUNC_NAME, SplitterGetRspState(pvSptHdl));
		MM_RETURN(RET_DMX_OK);
	}

	if ((prInf->u4TxStreamType != SPT_DATA_A) &&
		(prInf->u4TxStreamType != SPT_DATA_SP)) {
		MM_RETURN(RET_DMX_OK);
	}
	/* Starting reserved a free space in the spt list, then we could store the newest audio or sp inf. */
	if (0 == prSpt->u4RspEntryMax) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s line %d fail for pvSptHdl(0x%p), ")
			TEXT("u4RspEntryMax = 0!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	if (NULL != prInf->pvFromAddress) {
		mrRet = SplitterRspSetLogTxBuf2Fifo(pvSptHdl, prInf);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s fail in SplitterRspSetLogTxBuf2Fifo,")
				TEXT(" mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, mrRet);
			MM_RETURN(mrRet);
		}
		MM_RETURN(RET_DMX_OK);
	}

	if (!prInf->fgUseCmdQ) {
		mrRet = SplitterRspSetLogTxPbb2FifoNormal(pvSptHdl, prInf);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s fail in SplitterRspSetLogTxPbb2FifoNormal, ")
				TEXT("mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, mrRet);
			MM_RETURN(mrRet);
		}
		MM_RETURN(RET_DMX_OK);
	} else {
		mrRet = SplitterRspSetLogTxPbb2FifoCmdQ(pvSptHdl, prInf);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s fail in SplitterRspSetLogTxPbb2FifoNormal, ")
				TEXT("mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, mrRet);
			MM_RETURN(mrRet);
		}
		MM_RETURN(RET_DMX_OK);
	}
#endif				/* RSP_ENABLE */

	MM_RETURN(RET_DMX_OK);
}


/* ////////////////////////////////////////////////////////////////////////////////////////////// */
/* SplitterRspEntryRemoveable */
/* Condition: pr_cur_rsp's u8PtsSa < u8RspTM */
/* Log Tx Pbbuf Data Info into Resplitter Entry */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
bool SplitterRspEntryRemoveable(void *pvSptHdl,
	u64 u8RspTM,
	SPT_RSP_T *prCurRsp,
	SPT_RSP_T *prLastRsp,
	SPT_RSP_T *prNextRsp, u32 *pu4RspTxType)
{
	u32 u4RspTxType = 0;
	u32 u4RspTxFtrUID = 0;
	SPT_RSP_T *prTmpRsp = NULL;
	u64 u8ToFileOfst = DMX_INVALID_UINT64;

	if (!SplitterRspIsEnabled(pvSptHdl))
		return TRUE;

	if (NULL == prCurRsp) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
			TEXT("[SPT] %s fail for invalid args!\r\n"), DMX_FUNC_NAME);
		MM_RETURN(FALSE);
	}

	if (NULL != pu4RspTxType)
		u4RspTxType = *pu4RspTxType;
	else
		u4RspTxType = SplitterGetRspTxType(pvSptHdl);

	u4RspTxFtrUID = GetStmUIDByType(pvSptHdl, SplitterGetRspTxType(pvSptHdl));

	if (u4RspTxType == SPT_DATA_SP) {
		/* pr_cur_rsp's u8PtsSa < u8RspTM and pr_cur_rsp's u8PtsEa >u8RspTM, so pr_cur_rsp is unremovable */
		if ((prCurRsp->rTxInfo.u4TxStreamType == u4RspTxType) &&
		    (prCurRsp->rTxInfo.u4TxUID == u4RspTxFtrUID) &&
		    (INVALID_TIMESTAMP != prCurRsp->rTxInfo.u8PtsEa) &&
		    (prCurRsp->rTxInfo.u8PtsEa > u8RspTM))
			return FALSE;
		return TRUE;
	}
	/* Until here, Resplitter type must be Audio */
	if (u4RspTxType != SPT_DATA_A) {
		DMX_ASSERT(FALSE);
		return FALSE;
	}

	u8ToFileOfst = SplitterGetRspStartOffset(pvSptHdl) +
		SplitterGetRspOffsetDelta(pvSptHdl);

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
		TEXT("[SPT] %s line %d, StmType: %d, TxUID: %d, FromFileOfst: ")
		TEXT(DMX_UINT64_16U_LOGSTR ", ToFileOfst: " DMX_UINT64_16U_LOGSTR)
		TEXT(", Pts: " DMX_PTS_LOGSTR "!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		prCurRsp->rTxInfo.u4TxStreamType,
		prCurRsp->rTxInfo.u4TxUID,
		DMX_UINT64_16U_LOG_H(prCurRsp->rTxInfo.u8FromFileOfst),
		DMX_UINT64_16U_LOG_L(prCurRsp->rTxInfo.u8FromFileOfst),
		DMX_UINT64_16U_LOG_H(u8ToFileOfst),
		DMX_UINT64_16U_LOG_L(u8ToFileOfst),
		DMX_PTS_LOG_MS(prCurRsp->rTxInfo.u8PtsSa),
		DMX_PTS_LOG_PTS(prCurRsp->rTxInfo.u8PtsSa));

	if ((prCurRsp->rTxInfo.u4TxStreamType == u4RspTxType) &&
	    (prCurRsp->rTxInfo.u4TxUID == u4RspTxFtrUID) &&
	    (DMX_INVALID_UINT64 != u8ToFileOfst) &&
	    (prCurRsp->rTxInfo.u8FromFileOfst < u8ToFileOfst)) {
		return TRUE;
	}

	prTmpRsp = prCurRsp->prNextEntry;

	do {
		if ((NULL == prTmpRsp) || (NULL == prLastRsp))
			return FALSE;

		if (prTmpRsp == prLastRsp) {
			DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;

			if ((prTmpRsp->rTxInfo.u8PtsSa < u8RspTM) &&
			    ((prSpt->ucAudMaxDuration >= 3))) {
				DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
					TEXT("[SPT] %s line: %d -- Pts: " DMX_PTS_LOGSTR)
					TEXT(", u8RspTM: " DMX_PTS_LOGSTR "!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					DMX_PTS_LOG_MS(prTmpRsp->rTxInfo.u8PtsSa),
					DMX_PTS_LOG_PTS(prTmpRsp->rTxInfo.u8PtsSa),
					DMX_PTS_LOG_MS(u8RspTM),
					DMX_PTS_LOG_PTS(u8RspTM));
				return TRUE;
			}
			DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
				TEXT("[SPT] %s line: %d -- Pts: " DMX_PTS_LOGSTR)
				TEXT(", u8RspTM: " DMX_PTS_LOGSTR "!\r\n"),
			    DMX_FUNC_NAME, DMX_LINE_NO,
				DMX_PTS_LOG_MS(prTmpRsp->rTxInfo.u8PtsSa),
				DMX_PTS_LOG_PTS(prTmpRsp->rTxInfo.u8PtsSa),
				DMX_PTS_LOG_MS(u8RspTM),
				DMX_PTS_LOG_PTS(u8RspTM));
			return FALSE;
		}

		if ((prTmpRsp->rTxInfo.u4TxStreamType == u4RspTxType) &&
		    (prTmpRsp->rTxInfo.u4TxUID == u4RspTxFtrUID)) {
			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
				TEXT("[SPT] %s line %d -- u4TxStreamType: %d")
				TEXT(", u4TxUID: %d, u8PtsSa: " DMX_PTS_LOGSTR)
				TEXT(", u8RspTM: " DMX_PTS_LOGSTR)
				TEXT(", eFilledType: %d!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				prTmpRsp->rTxInfo.u4TxStreamType,
				prTmpRsp->rTxInfo.u4TxUID,
				DMX_PTS_LOG_MS(prTmpRsp->rTxInfo.u8PtsSa),
				DMX_PTS_LOG_PTS(prTmpRsp->rTxInfo.u8PtsSa),
				DMX_PTS_LOG_MS(u8RspTM),
				DMX_PTS_LOG_PTS(u8RspTM),
				prTmpRsp->eFilledType);

			if ((INVALID_TIMESTAMP == prTmpRsp->rTxInfo.u8PtsSa) ||
				(SPLITTER_RSP_ENTRY_TX_DONE == prTmpRsp->eFilledType)) {
				/* /BDP00115443 */
				return SplitterRspEntryRemoveable(pvSptHdl, u8RspTM, prTmpRsp,
								  prLastRsp, prNextRsp,
								  &u4RspTxType);
			}
			/* / meet 1st next entry */
			else if (prTmpRsp->rTxInfo.u8PtsSa >= u8RspTM) {
				/* if the next resplitter entry's ptsSa > u8RspTM, this resplitter entry's*/
				/* ptssa < u8RspTM, we can think this resplitter entry is across u8RspTM, */
				/* so it is not removable and copy the next resplitter entry's info into */
				/* pr_next */
				if (prNextRsp != NULL)
					*prNextRsp = *prTmpRsp;

				return FALSE;
			}
			/* if the next resplitter entry's ptsSa < u8RspTM, this resplitter entry's */
			/* ptssa < u8RspTM, */
			/* we can think this resplitter entry's ptsEa < u8RspTM, so it is removable */
			return TRUE;
		}
		prTmpRsp = prTmpRsp->prNextEntry;
	} while (prTmpRsp != NULL);

	return TRUE;
}

/* Active to transfer data to fifo */
MRESULT SplitterRspSetRspTx(void *pvSptHdl, SPT_RSP_T *prSptRsp)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;
	MRESULT mrRet = RET_DMX_OK;

	/* TODO: Remove after Integrate */
	if (!SplitterRspIsEnabled(pvSptHdl))
		MM_RETURN(RET_DMX_OK);

	if (NULL == prSptRsp) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[SPT] %s fail for invalid args(pvSptHdl: 0x%p, ")
			TEXT("prSptRsp: 0x%p)\r\n"),
			DMX_FUNC_NAME, pvSptHdl, prSptRsp);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	SplitterSetRspTx(pvSptHdl, prSptRsp);

	/* for all kinds of audio codec such as AAC,PCM, whether to transfer a total header to */
	/* the AFIFO, 1. if need to transfer a total header to the AFIFO, we could do it  success */
	/* and set prSpt->ucRspTxHdrState as RSP_TXHDRINFO_SAMPLE_HEAD_INIT, */
	/* then return to the spt_os, */
	/* spt_os would call to here again to transfer other small headers. */
	/* 2. if no need to transfer a total header, we could only set prSpt->ucRspTxHdrState as */
	/* RSP_TXHDRINFO_SAMPLE_HEAD_INIT.  then transfer these sample headers  this time. */
	/* 3. after success tranfered a total head, if no need to transfer the first sample head, */
	/* we could only set prSpt->ucTxAudHdr as */
	/* RSP_TXHDRINFO_SKIP_FIRSTSAMPLE_HEAD_TXING. */
	/* then it could transfer the other sample headers. */

	if ((RSP_TXHDRINFO_IDLE == prSpt->ucRspTxHdrState) &&
	    (SPT_DATA_A == SplitterGetRspTxType(pvSptHdl))) {
		CfaIntf *prCfaInterface = (CfaIntf *) SplitterGetCfaInterface(pvSptHdl);
		void *pvCfaPrivateData = SplitterGetCfaPrivateData(pvSptHdl);

		if (NULL == prCfaInterface) {
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_NO_CFA_INTERFACE);
		}
		if (NULL == pvCfaPrivateData) {
			DMX_ASSERT(FALSE);
			MM_RETURN(RET_DMX_NO_CFA_PRIV_DATA);
		}

		if (NULL != prCfaInterface->pfmrTxAudHdrInfo) {
			prSpt->ucRspTxHdrState = RSP_TXHDRINFO_TOTAL_HEAD_TXING;
			mrRet = prCfaInterface->pfmrTxAudHdrInfo(pvSptHdl,
			    GetStmUIDByType(pvSptHdl, SplitterGetRspTxType(pvSptHdl)),
			    pvCfaPrivateData);
			if (DMX_SUCCEED(mrRet)) {
				if (CFA_AUD_DRV_FMT_PCM == prSptRsp->rTxInfo.u4TxAudioCodec)
					prSpt->ucRspTxHdrState =
					    RSP_TXHDRINFO_SKIP_FIRSTSAMPLE_HEAD_TXING;
				else
					prSpt->ucRspTxHdrState = RSP_TXHDRINFO_SAMPLE_HEAD_INIT;

				DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
					TEXT("[SPT] %s line %d -- prCfaInterface->pfmrTxAudHdrInfo")
					TEXT(" success, AudioCodec: %d\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					prSptRsp->rTxInfo.u4TxAudioCodec);

				MM_RETURN(RET_DMX_OK);
			} else if (RET_DMX_UNSUPPORT == mrRet) {
				DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
					TEXT("[SPT] %s line %d -- Cfa unsupport Tx CodecType(%d) ")
					TEXT("Hdr in pfmrTxAudHdrInfo\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO,
					prSptRsp->rTxInfo.u4TxAudioCodec);
			} else {
				DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
					TEXT("[SPT] %s line %d fail in Cfa->pfmrTxAudHdrInfo, ")
					TEXT("mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
			}
		}

		prSpt->ucRspTxHdrState = RSP_TXHDRINFO_SAMPLE_HEAD_INIT;

		SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
		mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING,
			SPLITTER_TX_STATE_TXING);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s line %d fail in SplitterChangeState(RUNING, ")
				TEXT("TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
			MM_RETURN(mrRet);
		}

		MM_RETURN(RET_DMX_OK);
	}
	/* Transfer audio adts header from prSpt->pvSampleHdrBufSa to the AFIFO */
	if ((SPT_DATA_A == prSptRsp->rTxInfo.u4TxStreamType) &&
	    (0 != prSptRsp->rAHeadInfo.pvAddr) &&
	    (0 != prSptRsp->rAHeadInfo.u4Size) &&
	    (RSP_TXHDRINFO_SAMPLE_HEAD_TXING == prSpt->ucRspTxHdrState) &&
	    (!prSpt->fgAUCtrl)) {
		if (RSP_TX_AHEADS_IDLE == prSpt->ucRspTxAStmHdrState) {
			/* choose the log that must be transferred */
			DMX_SPT_DMA2FIFO_INFO_T rTxInfo = { 0 };

			rTxInfo.u8FromFileOfst = 1;
			rTxInfo.pvFromAddress = prSptRsp->rAHeadInfo.pvAddr;
			rTxInfo.u8TxLen = (u64) (prSptRsp->rAHeadInfo.u4Size);
			rTxInfo.u4TxStreamType = SPT_DATA_A;
			rTxInfo.pvToAddress = NULL;
			rTxInfo.u4TxVideoCodec = 0;
			rTxInfo.u4TxPictureMode = 0;
			rTxInfo.u8PtsSa = (u64) INVALID_TIMESTAMP;
			rTxInfo.u8PtsEa = (u64) INVALID_TIMESTAMP;
			rTxInfo.u4TxUID = prSptRsp->rTxInfo.u4TxUID;
			rTxInfo.u4TxAudioCodec = prSptRsp->rTxInfo.u4TxAudioCodec;

			prSpt->ucRspTxAStmHdrState = RSP_TX_AHEADS_TXING;

			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s line %d -- SplitterBuf2Fifo(StmType: ")
				TEXT("TxUID: %d, PtsSa: " DMX_PTS_LOGSTR)
				TEXT(", TxLen: " DMX_UINT64_16U_LOGSTR),
				DMX_FUNC_NAME, DMX_LINE_NO,
				DMX_SPTDATATYPE_STR(prSptRsp->rTxInfo.u4TxStreamType),
				prSptRsp->rTxInfo.u4TxUID,
				DMX_PTS_LOG_MS(prSptRsp->rTxInfo.u8PtsSa),
				DMX_PTS_LOG_PTS(prSptRsp->rTxInfo.u8PtsSa),
				rTxInfo.u8TxLen);

			mrRet = SplitterBuf2Fifo(pvSptHdl, &rTxInfo);
			/* transfer the header pointer the audio data then */
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
					TEXT("[SPT] %s line %d failed in SplitterBuf2Fifo, ")
				TEXT("pvSptHdl: 0x%p\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
			}
			MM_RETURN(mrRet);
		} else if (RSP_TX_AHEADS_DONE == prSpt->ucRspTxAStmHdrState) {
			prSpt->ucRspTxAStmHdrState = RSP_TX_AHEADS_IDLE;
		}
	}
#if DMX_SPT_RSP_USING_DIVXDRM
	/* For DivXDRM Turn on */
	if (((CFA_TYPE_AVI == SplitterGetCfaType(pvSptHdl)) ||
		 (CFA_TYPE_MKV == SplitterGetCfaType(pvSptHdl))) &&
		(DECRYPT_DIVXDRM == SplitterGetDecryptType(prSpt)) &&
		(DMX_INVALID_UINT64 != prSptRsp->u8DivxDRMOffset)) {
		/* turn DivxDRM on while rebuf */
		CFA_DIVXDRM_INFO_T rDivxDRMInfo;

		mm_memset(&rDivxDRMInfo, 0, sizeof(rDivxDRMInfo));

		rDivxDRMInfo.fgOn = TRUE;
		rDivxDRMInfo.u8DecryptStOfst = prSptRsp->u8DivxDRMOffset;
		rDivxDRMInfo.u4DecryptLen = prSptRsp->u4DecLen;
		rDivxDRMInfo.u2FrameKeyIdx = prSptRsp->u2FrameKeyIndex;

		prSpt->fgDivxDRMOn = TRUE;

		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[RSP] %s line %d -- Turn on DivxDRM For RSP\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);

		mrRet = SplitterPsrTurnDivxDRM(prSpt, &rDivxDRMInfo);

		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s line %d fail in SplitterPsrTurnDivxDRM, ")
				TEXT("Also finish rsp, mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);

			mrRet = SplitterRspTxFinish(pvSptHdl);

			if (DMX_FAILED(mrRet))
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
					TEXT("[SPT] %s line %d fail in SplitterRspTxFinish! ")
					TEXT("Make play finish(Send EOS) mrRet: 0x%x!\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, mrRet);

			MM_RETURN(mrRet);
		}
	}
#endif

	if (prSpt->fgAUCtrl) {
		if (prSptRsp->rTxInfo.fgCreateAU ||
		    (prSptRsp->rTxInfo.u8PtsSa != (u64) INVALID_TIMESTAMP)) {
			prSpt->fgAUCtrl = FALSE;
		} else {
			DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s line %d -- prSptRsp(0x%p)'s ")
				TEXT("rTxInfo.fgCreateAU: %d, PtsSa: " DMX_PTS_LOGSTR"\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp,
				prSptRsp->rTxInfo.fgCreateAU,
				DMX_PTS_LOG_MS(prSptRsp->rTxInfo.u8PtsSa),
				DMX_PTS_LOG_PTS(prSptRsp->rTxInfo.u8PtsSa));

			SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
			mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING,
						SPLITTER_TX_STATE_TXING);
			if (DMX_FAILED(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
					TEXT("[SPT] %s line %d fail in SplitterChangeState")
					TEXT("(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
				MM_RETURN(mrRet);
			}
			MM_RETURN(RET_DMX_OK);
		}
	}

	if ((prSptRsp->rTxInfo.u8TxLen > 0) || (prSptRsp->rTxInfo.fgAUByEnd)) {
		/* For real transfer */
		if (SPT_DATA_SP == prSptRsp->rTxInfo.u4TxStreamType) {
			AU_SP *prSP = (AU_SP *) (prSptRsp->pvAU);

			if (NULL != prSP) {
				DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
					TEXT("[SPT] %s line %d Subtitle Log AU's RspEntry: 0x%p,")
					TEXT("StartPts: "DMX_PTS_LOGSTR)
					TEXT(", EndPts: " DMX_PTS_LOGSTR)
					TEXT(", FileOfst: "DMX_UINT64_16U_LOGSTR)
					TEXT(", TxLen: " DMX_UINT64_16U_LOGSTR "\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp,
					DMX_PTS_LOG_MS(prSP->rAUInfo.rInfo.u8StartPts),
					DMX_PTS_LOG_PTS(prSP->rAUInfo.rInfo.u8StartPts),
					DMX_PTS_LOG_MS(prSP->rAUInfo.rInfo.u8EndPts),
					DMX_PTS_LOG_PTS(prSP->rAUInfo.rInfo.u8EndPts),
					DMX_UINT64_16U_LOG_H(prSptRsp->rTxInfo.u8FromFileOfst),
					DMX_UINT64_16U_LOG_L(prSptRsp->rTxInfo.u8FromFileOfst),
					DMX_UINT64_16U_LOG_H(prSptRsp->rTxInfo.u8TxLen),
					DMX_UINT64_16U_LOG_L(prSptRsp->rTxInfo.u8TxLen));
			}
		} else if (SPT_DATA_A == prSptRsp->rTxInfo.u4TxStreamType) {
			AU_AUDIO *prAud = (AU_AUDIO *) (prSptRsp->pvAU);

			if (NULL != prAud) {
				DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
					TEXT("[SPT] %s line %d Audio Log AU's RspEntry: 0x%p, ")
					TEXT("Pts: "DMX_PTS_LOGSTR)
					TEXT(", FileOfst: " DMX_UINT64_16U_LOGSTR)
					TEXT(", TxLen: " DMX_UINT64_16U_LOGSTR)
					TEXT(", fgAUByEnd: %d, fgCreateAU: %d, fgUnitEnd: %d\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp,
					DMX_PTS_LOG_MS(prAud->rAUInfo.rInfo.u8Pts),
					DMX_PTS_LOG_PTS(prAud->rAUInfo.rInfo.u8Pts),
					DMX_UINT64_16U_LOG_H(prSptRsp->rTxInfo.u8FromFileOfst),
					DMX_UINT64_16U_LOG_L(prSptRsp->rTxInfo.u8FromFileOfst),
					DMX_UINT64_16U_LOG_H(prSptRsp->rTxInfo.u8TxLen),
					DMX_UINT64_16U_LOG_L(prSptRsp->rTxInfo.u8TxLen),
					((prSptRsp->rTxInfo.fgAUByEnd) ? 1 : 0),
					((prSptRsp->rTxInfo.fgCreateAU) ? 1 : 0),
					((prSptRsp->rTxInfo.fgUnitEnd) ? 1 : 0));
			} else {
				DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
					TEXT("[SPT] %s line %d -- prSptRsp(0x%p)'s ")
					TEXT("prSptRsp->pvAU == NULL\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, prSptRsp);
			}
		}

		mrRet = SplitterPbb2Fifo(pvSptHdl, &(prSptRsp->rTxInfo));
		if (DMX_FAILED(mrRet)) {
			if (!MM_IS_STATE_ERROR(mrRet)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
					TEXT("[SPT] %s line %d failed in SplitterPbb2Fifo, ")
					TEXT("pvSptHdl: 0x%p\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);
			}
		}
		MM_RETURN(mrRet);
	} else {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[SPT] %s line %d fail for error logAU whose's u8TxLen is 0, ")
			TEXT("pvSptHdl: 0x%p\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl);

		prSpt->ucRspTxHdrState = RSP_TXHDRINFO_SAMPLE_HEAD_TXING;

		SplitterSendNfy(pvSptHdl, DMX_SPT_NTY_TX_END);
		mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING, SPLITTER_TX_STATE_TXING);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s line %d fail in SplitterChangeState")
				TEXT("(RUNING, TXING), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
			MM_RETURN(mrRet);
		}
		MM_RETURN(RET_DMX_OK);
	}
}

MRESULT SplitterRspTxFinish(void *pvSptHdl)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;
	bool fgRspRebuf = FALSE;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
		TEXT("[SPT] %s -- pvSptHdl: 0x%p\r\n"), DMX_FUNC_NAME, pvSptHdl);
	if (NULL == pvSptHdl) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[SPT] %s fail for invalid args, (pvSptHdl: 0x%p)\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterRspIsEnabled(pvSptHdl)) {
		DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[SPT] %s -- Rsplitter is disable!\r\n"), DMX_FUNC_NAME);
		fgRspRebuf = prSpt->fgRspRebuf;
		DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[SPT] %s line %d -- fgRspRebuf(%d), LastRspLen: ")
			TEXT(DMX_UINT64_16U_LOGSTR "!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			(fgRspRebuf ? 1 : 0),
			DMX_UINT64_16U_LOG_H(SplitterGetRspLastPtxLen(pvSptHdl)),
			DMX_UINT64_16U_LOG_L(SplitterGetRspLastPtxLen(pvSptHdl)));
		prSpt->fgRspRebuf = FALSE;
		if (SplitterIsReResplitter(prSpt)) {
			DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s line %d -- Re-resplitter flag is TRUE\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO);
		}
		SplitterSetRspTxType(pvSptHdl, SPT_DATA_UNDEFINE);
		SplitterSetRspMode(pvSptHdl, SPLITTER_PTX_RSP_BY_PTS);
		mrRet = SptCfaSetTxDone(pvSptHdl, SplitterGetRspLastPtxLen(pvSptHdl), fgRspRebuf);
		MM_RETURN(mrRet);
	}

	prSpt->fgCPSOn = FALSE;

#if DMX_SPT_RSP_USING_DIVXDRM
	if (prSpt->fgDivxDRMOn) {
		CFA_DIVXDRM_INFO_T rDivxDRMInfo;

		mm_memset(&rDivxDRMInfo, 0, sizeof(rDivxDRMInfo));

		rDivxDRMInfo.fgOn = FALSE;
		rDivxDRMInfo.u8DecryptStOfst = DMX_INVALID_UINT64;
		rDivxDRMInfo.u4DecryptLen = 0;
		rDivxDRMInfo.u2FrameKeyIdx = DMX_DIVXDRM_INVALID_FRAMEIDX;

		prSpt->fgDivxDRMOn = FALSE;

		mrRet = SplitterPsrTurnDivxDRM(pvSptHdl, &rDivxDRMInfo);
		if (DMX_FAILED(mrRet)) {
			DmxLogW(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s line %d fail in SplitterPsrTurnDivxDRM,")
				TEXT(" Also finish rsp, mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		}
	}
#endif				/* DMX_SPT_RSP_USING_DIVXDRM */

	/* Finish the transfer */
	/* and change spt state to be IDLE */

	if (SplitterIsCfaPsrEnd(prSpt))	{
		/* For aVOID disable parser after enter check state */
		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
			TEXT("[SPT] %s -- fgCfaPrsEnd, pvSptHdl:0x%p!!\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		GAU_SetEOS(pvSptHdl, TRUE, SplitterGetPsrEndStatus(prSpt));
		DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[SPT] %s rsp encounter EOS\r\n"), DMX_FUNC_NAME);
		SplitterSetPtxNotBusy(pvSptHdl);
		mrRet = PSR_CC_NotiCfaPrsEnd(prSpt->pvPsrCC, TRUE);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s fail in PSR_CC_NotiCfaPrsEnd")
				TEXT("(pvSptHdl: 0x%p, TRUE)!\r\n"),
				DMX_FUNC_NAME, pvSptHdl);
			MM_RETURN(mrRet);
		}
		mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_IDLE,
			SPLITTER_TX_STATE_NONE);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s line %d fail in SplitterChangeState")
				TEXT("(IDLE, TX_NONE), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
			MM_RETURN(mrRet);
		}
	} else {
		mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_RUNING,
			SPLITTER_TX_STATE_CHECK);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s line %d fail in SplitterChangeState")
				TEXT("(RUNING, TX_CHECK), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
			MM_RETURN(mrRet);
		}
	}

	/* Enable Rsplitter Loging */
	SplitterRspSetLogEnable(pvSptHdl);

	SplitterSetRspTx(pvSptHdl, NULL);

	prSpt->fgAUCtrl = TRUE;

	prSpt->ucRspTxHdrState = RSP_TXHDRINFO_IDLE;

	SplitterSetRspStartPts(pvSptHdl, INVALID_TIMESTAMP);

	SplitterSetRspStartOffset(pvSptHdl, DMX_INVALID_UINT64);

	SplitterSetRspOffsetDelta(pvSptHdl, 0);

	SplitterSetReResplitter(pvSptHdl, FALSE);

	SplitterSetRspTxType(pvSptHdl, SPT_DATA_UNDEFINE);

	SplitterSetRspMode(pvSptHdl, SPLITTER_PTX_RSP_BY_PTS);

	if (SplitterIsCfaPsrEnd(prSpt))	{
		/* if cfa has parsed end, we don't send EOS */

		prSpt->fgRspRebuf = FALSE;
		mrRet = PSR_CC_NotiCfaPrsEnd(prSpt->pvPsrCC, TRUE);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s fail in PSR_CC_NotiCfaPrsEnd")
				TEXT("(pvSptHdl: 0x%p, TRUE)!\r\n"),
				DMX_FUNC_NAME, pvSptHdl);
			MM_RETURN(mrRet);
		}
		MM_RETURN(RET_DMX_OK);
	}

	/* Start CFA normal transfer */
	/* Inform Cfa Tx Done to resume to normal tx data flow */
	/* fgRspRebuf = prSpt->fgRspRebuf; */
	DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
		TEXT("[SPT] %s line %d -- RspLen: " DMX_UINT64_16U_LOGSTR "!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		DMX_UINT64_16U_LOG_H(SplitterGetRspLastPtxLen(pvSptHdl)),
		DMX_UINT64_16U_LOG_L(SplitterGetRspLastPtxLen(pvSptHdl)));

	prSpt->fgRspRebuf = FALSE;

	if (SplitterIsReResplitter(prSpt))
		DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[SPT] %s line %d -- Re-resplitter flag is TRUE\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);

	mrRet = SptCfaSetTxDone(pvSptHdl, SplitterGetRspLastPtxLen(pvSptHdl), TRUE);

	MM_RETURN(mrRet);
}

MRESULT SplitterRspTxFinish4ReRsp(void *pvSptHdl)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;
	bool fgRspRebuf = FALSE;
	MRESULT mrRet = RET_DMX_OK;

	DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
		TEXT("[SPT] %s -- pvSptHdl: 0x%p\r\n"), DMX_FUNC_NAME, pvSptHdl);
	if (NULL == pvSptHdl) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[SPT] %s fail for invalid args, (pvSptHdl: 0x%p)\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterRspIsEnabled(pvSptHdl)) {
		DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[SPT] %s -- Rsplitter is disable!\r\n"), DMX_FUNC_NAME);
		fgRspRebuf = prSpt->fgRspRebuf;
		DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[SPT] %s line %d -- fgRspRebuf(%d), ")
			TEXT("RspLen: " DMX_UINT64_16U_LOGSTR"!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, (fgRspRebuf ? 1 : 0),
			DMX_UINT64_16U_LOG_H(SplitterGetRspLastPtxLen(pvSptHdl)),
			DMX_UINT64_16U_LOG_L(SplitterGetRspLastPtxLen(pvSptHdl)));
		prSpt->fgRspRebuf = FALSE;
		MM_RETURN(mrRet);
	}

	prSpt->fgCPSOn = FALSE;

#if DMX_SPT_RSP_USING_DIVXDRM
	if (prSpt->fgDivxDRMOn) {
		CFA_DIVXDRM_INFO_T rDivxDRMInfo;

		mm_memset(&rDivxDRMInfo, 0, sizeof(rDivxDRMInfo));

		rDivxDRMInfo.fgOn = FALSE;
		rDivxDRMInfo.u8DecryptStOfst = DMX_INVALID_UINT64;
		rDivxDRMInfo.u4DecryptLen = 0;
		rDivxDRMInfo.u2FrameKeyIdx = DMX_DIVXDRM_INVALID_FRAMEIDX;

		prSpt->fgDivxDRMOn = FALSE;

		mrRet = SplitterPsrTurnDivxDRM(pvSptHdl, &rDivxDRMInfo);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s line %d fail in SplitterPsrTurnDivxDRM, ")
				TEXT("Also finish rsp, mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, mrRet);
		}
	}
#endif				/* DMX_SPT_RSP_USING_DIVXDRM */

	/* Finish the transfer */
	/* and change spt state to be IDLE */
	DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
		TEXT("[SPT] %s -------prSpt->fgCfaPrsEnd: %d\r\n"),
		DMX_FUNC_NAME, (SplitterIsCfaPsrEnd(prSpt) ? 1 : 0));

	if (SplitterIsCfaPsrEnd(prSpt))	{
		/* For aVOID disable parser after enter check state */
		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_REBUF,
			TEXT("[SPT] %s -- fgCfaPrsEnd, pvSptHdl:0x%p!!\r\n"),
			DMX_FUNC_NAME, pvSptHdl);
		DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
			TEXT("[SPT] %s rsp encounter EOS\r\n"), DMX_FUNC_NAME);
		SplitterSetPtxNotBusy(pvSptHdl);
		mrRet = PSR_CC_NotiCfaPrsEnd(prSpt->pvPsrCC, TRUE);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				    TEXT("[SPT] %s fail in PSR_CC_NotiCfaPrsEnd")
					TEXT("(pvSptHdl: 0x%p, TRUE)!\r\n"),
				    DMX_FUNC_NAME, pvSptHdl);
			MM_RETURN(mrRet);
		}

		mrRet = SplitterChangeState(pvSptHdl, SPLITTER_STATE_IDLE, SPLITTER_TX_STATE_NONE);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
				TEXT("[SPT] %s line %d fail in SplitterChangeState")
				TEXT("(IDLE, TX_NONE), pvSptHdl: 0x%p, mrRet: 0x%x\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, mrRet);
			MM_RETURN(mrRet);
		}
	}
	/* Enable Rsplitter Loging */
	SplitterRspSetLogEnable(pvSptHdl);

	SplitterSetRspTx(pvSptHdl, NULL);

	prSpt->fgAUCtrl = TRUE;

	prSpt->ucRspTxHdrState = RSP_TXHDRINFO_IDLE;

	SplitterSetRspStartPts(pvSptHdl, INVALID_TIMESTAMP);

	SplitterSetRspStartOffset(pvSptHdl, DMX_INVALID_UINT64);

	SplitterSetRspOffsetDelta(pvSptHdl, 0);

	DmxLogT(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_RSPTX,
		TEXT("[SPT] %s -- pvSptHdl: 0x%p, exit\r\n"), DMX_FUNC_NAME, pvSptHdl);

	MM_RETURN(mrRet);
}

/* ////////////////////////////////////////////////////////////////////////////////////////////// */
/* SplitterRspUpdateExtInf */
/* 1.  if eProcType is SPLITTER_RSP_INF_ADD, it does the following tasks: */
/* (1) If no corresponding stream info in Resplitter Instance Steam Info Array--pvRspExtInfSa, */
/*      add this stream info */
/* (2) Add the Need Process Entry(tx info) into the stream info, including add log cnt, */
/*      log tx size */
/* 2.  if eProcType is SPLITTER_RSP_INF_REMOVE, it does the following tasks: */
/* (1) If there is no corresponding stream info in Resplitter Instance Steam Info Array-- */
/*      pvRspExtInfSa, return RET_DMX_NOT_FOUND */
/* (2) Otherwise, remove the log info from the stream info, including decrease log cnt, */
/*      log tx size */
/* 3.  if eProcType is SPLITTER_RSP_INF_CLEAR, it does the following tasks: */
/* clear all stream info in Resplitter Instance Steam Info Array--pvRspExtInfSa */
/* @Param eProcType            [IN]  Process Type */
/* @Param pvEntryNeedProc  [IN]  Nee Process Entry Info */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
MRESULT SplitterRspUpdateExtInf(void *pvSptHdl,
	E_DMX_RSP_EXTINFPROC_TYPE_T eProcType, SPT_RSP_T *pvEntryNeedProc)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;
	RSP_STRM_INF *prRspStrmInf = NULL;
	u32 u4Idx = 0;

	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s line %d fail for invalid args, ")
			TEXT("(pvSptHdl: 0x%p, eProcType: 0x%x)\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, eProcType);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterRspIsEnabled(pvSptHdl))
		MM_RETURN(RET_DMX_OK);

	if (0 == prSpt->pvRspExtInfSa)
		MM_RETURN(RET_DMX_OK);

	switch (eProcType) {
	case SPLITTER_RSP_INF_ADD:
	case SPLITTER_RSP_INF_REMOVE:
		if (NULL == pvEntryNeedProc) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d fail for invalid args, ")
				TEXT("(pvSptHdl: 0x%p, EntryNeedProc: 0x%x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, pvEntryNeedProc);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}

		prRspStrmInf = (RSP_STRM_INF *) (prSpt->pvRspExtInfSa);
		if (NULL == prRspStrmInf) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
				TEXT("[SPT] %s line %d fail for prRspStrmInf is NULL, ")
				TEXT("(pvSptHdl: 0x%p, EntryNeedProc: 0x%x)\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, pvSptHdl, prRspStrmInf);
			MM_RETURN(RET_DMX_PARAM_WRONG);
		}
		/* Get corrsponding strm info */
		for (u4Idx = 0; u4Idx < SPT_REC_RSP_INF_STRM_MAX; u4Idx++) {
			if ((prRspStrmInf->fgUsed) &&
			    (prRspStrmInf->u4StrmID == pvEntryNeedProc->u4StreamUID))
				/* Find strm */
				break;
			if ((!prRspStrmInf->fgUsed) &&
				(SPLITTER_RSP_INF_ADD == eProcType))
				/* Add new strm */
				break;
			prRspStrmInf++;
		}

		if (u4Idx >= SPT_REC_RSP_INF_STRM_MAX) {
			if (SPLITTER_RSP_INF_ADD == eProcType) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s line %d fail for no corresponding stream")
					TEXT(" of the rsp entry whose eProcType is %d,")
					TEXT(" pvSptHdl: 0x%p\r\n"),
					DMX_FUNC_NAME, DMX_LINE_NO, eProcType, pvSptHdl);
				MM_RETURN(RET_DMX_OVER_LIMIT);
			}
			MM_RETURN(RET_DMX_OK);
		}
		break;

	case SPLITTER_RSP_INF_CLEAR:
		dmx_memset((void *) (prSpt->pvRspExtInfSa), 0,
			   SPT_REC_RSP_INF_STRM_MAX * sizeof(RSP_STRM_INF));
		MM_RETURN(RET_DMX_OK);

	default:
		break;
	}

	if (NULL == prRspStrmInf)
		MM_RETURN(RET_DMX_NOT_FOUND);

	if (SPLITTER_RSP_INF_ADD == eProcType) {
		prRspStrmInf->fgUsed = TRUE;
		prRspStrmInf->u4StrmID = pvEntryNeedProc->u4StreamUID;
		prRspStrmInf->u4AvailLogCnt++;
		prRspStrmInf->u8AvailLogSize += (pvEntryNeedProc->rTxInfo).u8TxLen;
		prRspStrmInf->ucRspTxType =
			(u8) ((pvEntryNeedProc->rTxInfo).u4TxStreamType);
	} else if (SPLITTER_RSP_INF_REMOVE == eProcType) {
		if (prRspStrmInf->u4AvailLogCnt >= 1)
			prRspStrmInf->u4AvailLogCnt--;

		if (prRspStrmInf->u8AvailLogSize >= (pvEntryNeedProc->rTxInfo).u8TxLen)
			prRspStrmInf->u8AvailLogSize -= (pvEntryNeedProc->rTxInfo).u8TxLen;
	}

	MM_RETURN(RET_DMX_OK);
}

/* ////////////////////////////////////////////////////////////////////////////////////////////// */
/* SplitterRspRemoveLog */
/* Remove the entry from the Resplitter Entry List, add its Repslitter Entry RP Ptr -- pvRspRp, */
/* descrease the entry's txsize from its corresponding Respliter Stream Info */
/* Decreate the Replitter Entry Count */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
MRESULT SplitterRspRemoveLog(void *pvSptHdl, SPT_RSP_T *prRemoveEntry)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prRemoveEntry) || (NULL == pvSptHdl)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for invalid args, ")
			TEXT("(pvSptHdl: 0x%p, RmvEntry: 0x%p)\r\n"),
			DMX_FUNC_NAME, pvSptHdl, prRemoveEntry);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterRspIsEnabled(pvSptHdl))
		MM_RETURN(RET_DMX_OK);

	if (NULL != prRemoveEntry->prPrevEntry) {
		/* It shell never happen. because we remove the log au from the rspfirst one by one */
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s line %d fail for RemoveEntry's ")
			TEXT("prPrevEntry is not NULL\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	if (NULL != prSpt->pvRspFirstTx)
		DMX_ASSERT(prSpt->pvRspFirstTx == (void *)(prSpt->pvRspRp));

	/* Free any allocated audio hdr memory */
	if (SPT_DATA_A == prRemoveEntry->rTxInfo.u4TxStreamType) {
		if (0 != prRemoveEntry->rAHeadInfo.pvAddr) {
			RSP_HDR_MEM_NODE *prNode = &(prRemoveEntry->rAHeadInfo);
			RSP_HDR_MEM_LIST *prHdrMemList = &(prSpt->rHdrMemList);

			if (!RspSampleHdrFreeTail2Node(pvSptHdl, prNode)) {
				DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[RSP] %s fail in RspSampleHdrFreeTail2Node\r\n"),
					DMX_FUNC_NAME);

				DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
					TEXT("[SPT] %s Need to Remove Rsp Entry (0x%p),")
					TEXT(" pvRspFirstTx(0x%p), RspCurPtr(0x%p), ")
					TEXT("RspWp(0x%p), RspRP(0x%p), RspAUTbl(Sa(0x%p), ")
					TEXT("Ea(0x%p)), RspAUCnt(%d)!\r\n"),
					DMX_FUNC_NAME, prRemoveEntry, prSpt->pvRspFirstTx,
					prSpt->pvRspCurPtr, prSpt->pvRspWp, prSpt->pvRspRp,
					prSpt->pvRspAUTblSa, prSpt->pvRspAUTblEa,
					prSpt->u4RspCount);

				MM_RETURN(RET_DMX_UNEXPECT);
			}

			if (g_rDmxCliMan.fgDumpRspInfo)
				DmxDumpRspData(prSpt->u4SptCompId, prNode, prHdrMemList, FALSE);

			if (RspSampleHdrFreeNode(pvSptHdl, &(prRemoveEntry->rAHeadInfo))) {
				prRemoveEntry->rAHeadInfo.pvAddr = 0;
				prRemoveEntry->rAHeadInfo.u4Size = 0;
			}
		}
	} else {
		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s -- StmType(%d), UID(%d), u4StreamUID(%d), ")
			TEXT("u4Addr(%p)!\r\n"),
			DMX_FUNC_NAME, prRemoveEntry->rTxInfo.u4TxStreamType,
			prRemoveEntry->rTxInfo.u4TxUID, prRemoveEntry->u4StreamUID,
			prRemoveEntry->rAHeadInfo.pvAddr);
		DMX_ASSERT(0 == prRemoveEntry->rAHeadInfo.pvAddr);
	}

	if (NULL != prSpt->pvRspFirstTx)
		DMX_ASSERT(prSpt->pvRspFirstTx == (void *)(prSpt->pvRspRp));

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
		TEXT("[SPT] %s line %d -- Remove Rsp Entry (0x%p), ")
		TEXT("pvRspFirstTx(0x%p), prNextEntry(0x%p), RspWp(0x%p), ")
		TEXT("RspRP(0x%p), RspAUTbl(Sa(0x%p), Ea(0x%p)), ")
		TEXT("RspAUCnt(%d)!\r\n"),
		DMX_FUNC_NAME, DMX_LINE_NO,
		prRemoveEntry, prSpt->pvRspFirstTx,
		prRemoveEntry->prNextEntry, prSpt->pvRspWp,
		prSpt->pvRspRp, prSpt->pvRspAUTblSa,
		prSpt->pvRspAUTblEa, prSpt->u4RspCount);

	prSpt->pvRspFirstTx = (void *) (prRemoveEntry->prNextEntry);

	if (NULL != prRemoveEntry->prNextEntry)
		(prRemoveEntry->prNextEntry)->prPrevEntry = prRemoveEntry->prPrevEntry;
	if (NULL != prRemoveEntry->prPrevEntry)
		(prRemoveEntry->prPrevEntry)->prNextEntry = prRemoveEntry->prNextEntry;

	if (prSpt->pvRspLastTx == prRemoveEntry)
		prSpt->pvRspLastTx = prRemoveEntry->prPrevEntry;

	/* Remove the log info in its corresponding RSP_STRM_INF entry */
	mrRet = SplitterRspUpdateExtInf(pvSptHdl, SPLITTER_RSP_INF_REMOVE, prRemoveEntry);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail in SplitterRspUpdateExtInf(REMOVE),")
			TEXT(" mrRet: 0x%x!\r\n"),
			DMX_FUNC_NAME, mrRet);
		MM_RETURN(mrRet);
	}

	prRemoveEntry->eFilledType = SPLITTER_RSP_ENTRY_IDLE;

	/* Free any allocated memory */
	SplitterRspFree(pvSptHdl, TRUE);

	SplitterSetRspEntryCountDecrease(pvSptHdl);

	if ((NULL != prSpt->pvRspFirstTx) &&
		(prSpt->pvRspFirstTx != (void *)(prSpt->pvRspRp))) {
		DMX_ASSERT(FALSE);
		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s line %d -- prSpt->pvRspFirstTx (0x%p)")
			TEXT(" != pvRspRp(0x%p), RspWp(0x%p), RspRP(0x%p), ")
			TEXT("RspAUTbl(Sa(0x%p), Ea(0x%p)), RspAUCnt(%d)!\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO,
			prRemoveEntry, prSpt->pvRspFirstTx,
			prSpt->pvRspRp, prSpt->pvRspWp,
			prSpt->pvRspRp, prSpt->pvRspAUTblSa,
			prSpt->pvRspAUTblEa, prSpt->u4RspCount);
	}

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
		TEXT("[SPT] %s -- Remove Rsp Entry (0x%p), pvRspFirstTx(0x%p),")
		TEXT(" RspCurPtr(0x%p), RspWp(0x%p), RspRP(0x%p), ")
		TEXT("RspAUTbl(Sa(0x%p), Ea(0x%p)), RspAUCnt(%d)!\r\n"),
		DMX_FUNC_NAME, prRemoveEntry, prSpt->pvRspFirstTx, prSpt->pvRspCurPtr,
		prSpt->pvRspWp, prSpt->pvRspRp, prSpt->pvRspAUTblSa, prSpt->pvRspAUTblEa,
		prSpt->u4RspCount);

	MM_RETURN(RET_DMX_OK);
}

/* ////////////////////////////////////////////////////////////////////////////////////////////// */
/* SplitterRspFree */
/* If fgFromStart is TRUE, Add its Repslitter Entry RP Ptr -- pvRspRp, i.e. descreate the */
/* resplitter entry from the Entry list's header */
/* Otherwise, Descrease  its Repslitter Entry WP Ptr -- pvRspWp, i.e. descreate the resplitter */
/* entry from the Entry list's tail */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
static void SplitterRspFree(void *pvSptHdl, bool fgFromStart)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;

	if (NULL == prSpt) {
		DMX_ASSERT(FALSE);
		return;
	}

	if (!SplitterRspIsEnabled(pvSptHdl))
		return;

	if (0 == prSpt->u4RspCount)
		return;

	if (fgFromStart) {
		prSpt->pvRspRp += (sizeof(SPT_RSP_T) + prSpt->u2RspAuSize);

		if (prSpt->pvRspRp >= prSpt->pvRspAUTblEa)
			prSpt->pvRspRp = prSpt->pvRspAUTblSa;

		if (prSpt->pvRspRp > prSpt->pvRspAUTblEa) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
				TEXT("[SPT] %s line %d fail for error Rsp Read Pointer(0x%p)")
				TEXT(" which is beyond the RspAUTable Range ")
				TEXT("[0x%p, 0x%p]\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO,
				prSpt->pvRspRp, prSpt->pvRspAUTblSa,
				prSpt->pvRspAUTblEa);
			return;
		}

		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s -- RspRP(0x%p), RspWp(0x%p), RspCurPtr(0x%p),")
			TEXT(" RspAUTblSa(0x%p), RspAUTblSa(0x%p) RspAUCnt(%d)!\r\n"),
			DMX_FUNC_NAME, prSpt->pvRspRp, prSpt->pvRspWp, prSpt->pvRspCurPtr,
			prSpt->pvRspAUTblSa, prSpt->pvRspAUTblEa, (prSpt->u4RspCount - 1));
	} else {
		if (prSpt->pvRspWp == prSpt->pvRspAUTblSa)
			prSpt->pvRspWp = prSpt->pvRspAUTblEa;

		if (prSpt->pvRspWp < prSpt->pvRspAUTblSa) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
				TEXT("[SPT] %s line %d fail for error Rsp Write ")
				TEXT("Pointer(0x%p) which is beyond the RspAUTable Range")
				TEXT(" [0x%p, 0x%p]\r\n"),
				DMX_FUNC_NAME, DMX_LINE_NO, prSpt->pvRspWp, prSpt->pvRspAUTblSa,
				prSpt->pvRspAUTblEa);
			return;
		}

		prSpt->pvRspWp -= sizeof(SPT_RSP_T) + prSpt->u2RspAuSize;

		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s -- Free RspWp(0x%p), RspRP(0x%p), ")
			TEXT("RspCurPtr(0x%p), RspAUTblSa(0x%p), ")
			TEXT("RspAUTblSa(0x%p) RspAUCnt(%d)!\r\n"),
			DMX_FUNC_NAME, prSpt->pvRspWp, prSpt->pvRspRp, prSpt->pvRspCurPtr,
			prSpt->pvRspAUTblSa, prSpt->pvRspAUTblEa, (prSpt->u4RspCount - 1));
	}

	prSpt->u4RspCount--;

	if (prSpt->u4RspCount > prSpt->u4RspEntryMax)
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
			TEXT("[SPT] %s fail for RspAUTable is overflow, the RspAU")
			TEXT(" Count is %d, Max is %d \r\n"),
			DMX_FUNC_NAME, prSpt->u4RspCount, prSpt->u4RspEntryMax);
}

/* ////////////////////////////////////////////////////////////////////////////////////////////// */
/* SplitterRspNew */
/* Increate Resplitter Entry Fifo WP, make pvRspCurPtr pointer to the new add entry start position */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
static MRESULT SplitterRspNew(void *pvSptHdl)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;
	SPT_RSP_T *prSptRsp = NULL;
	SPT_RSP_T *prRemoveEntry = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for invalid args\r\n"), DMX_FUNC_NAME);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterRspIsEnabled(pvSptHdl))
		MM_RETURN(RET_DMX_OK);

	prRemoveEntry = (SPT_RSP_T *) prSpt->pvRspFirstTx;

	while ((NULL != prRemoveEntry) &&
		((prSpt->u4RspCount == prSpt->u4RspEntryMax - 1))) {
		if (prSpt->u4RspCount == prSpt->u4RspEntryMax - 1)
			prSpt->u4RspDrop++;

		mrRet = SplitterRspRemoveLog(pvSptHdl, prRemoveEntry);
		if (DMX_FAILED(mrRet)) {
			DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_DEFAULT,
				TEXT("[SPT] %s fail in SplitterRspRemoveLog, mrRet: 0x%x!\r\n"),
				DMX_FUNC_NAME, mrRet);
			MM_RETURN(mrRet);
		}
		prRemoveEntry = (SPT_RSP_T *) prSpt->pvRspFirstTx;
	}

	if (prSpt->u4RspCount >= prSpt->u4RspEntryMax) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for RspAUTable is overflow, the RspAU")
			TEXT(" Count is %d, Max is %d \r\n"),
			DMX_FUNC_NAME, prSpt->u4RspCount, prSpt->u4RspEntryMax);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}

	if (prSpt->pvRspWp > prSpt->pvRspAUTblEa) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s line %d fail for error Rsp Write Pointer(0x%p) ")
			TEXT("which is beyond the RspAUTable Range [0x%p, 0x%p]\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt->pvRspWp, prSpt->pvRspAUTblSa,
			prSpt->pvRspAUTblEa);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	if (prSpt->pvRspWp >= prSpt->pvRspAUTblEa)	/* wrap around case. */
		prSpt->pvRspWp = prSpt->pvRspAUTblSa;

	prSpt->pvRspCurPtr = prSpt->pvRspWp;

	prSptRsp = (SPT_RSP_T *) (prSpt->pvRspCurPtr);
	prSptRsp->prNextEntry = NULL;
	prSptRsp->prPrevEntry = NULL;

	prSpt->pvRspWp += sizeof(SPT_RSP_T) + prSpt->u2RspAuSize;

	if (prSpt->pvRspWp > prSpt->pvRspAUTblEa) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s line %d fail for error Rsp Write Pointer(0x%p)")
			TEXT(" which is beyond the RspAUTable Range [0x%p, 0x%p]\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt->pvRspWp, prSpt->pvRspAUTblSa,
			prSpt->pvRspAUTblEa);
		MM_RETURN(RET_DMX_ERR_STATE);
	}

	if (prSpt->pvRspWp == prSpt->pvRspRp) {
		/* Resplitter Entrys Fifo Full */
		/* should extend RSP_NS_MAX */
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s line %d fail for RspAUTable is FULL, ")
			TEXT("now RspAUTable's Write Pointer(0x%p) is equal to ")
			TEXT("Read Pointer, RspAUTable Range [0x%p, 0x%p], ")
			TEXT("RspAU Count: %d\r\n"),
			DMX_FUNC_NAME, DMX_LINE_NO, prSpt->pvRspWp, prSpt->pvRspAUTblSa,
			prSpt->pvRspAUTblEa, prSpt->u4RspCount);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}

	/* wrap around case. */
	if (prSpt->pvRspWp == prSpt->pvRspAUTblEa)
		prSpt->pvRspWp = prSpt->pvRspAUTblSa;

	prSpt->u4RspCount++;

	if (prSpt->u4RspCount > prSpt->u4RspEntryMax /*RSP_NS_MAX */) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
		    TEXT("[SPT] RSP NS is overflow, u4RspCount(0x%x), ")
			TEXT("u4RspEntryMax(0x%x)\r\n"),
		    prSpt->u4RspCount, prSpt->u4RspEntryMax);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_OVER_LIMIT);
	}

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
		TEXT("[SPT] %s -- RspCurPtr(0x%p), RspWp(0x%p), RspRP(0x%p),")
		TEXT(" RspAUTbl(Sa(0x%p), Ea(0x%p)), RspAUCnt(%d)!\r\n"),
		DMX_FUNC_NAME, prSpt->pvRspCurPtr, prSpt->pvRspWp, prSpt->pvRspRp,
		prSpt->pvRspAUTblSa, prSpt->pvRspAUTblEa, prSpt->u4RspCount);

	MM_RETURN(RET_DMX_OK);
}


/* ////////////////////////////////////////////////////////////////////////////////////////////// */
/* SplitterRspGetRsp */
/* Get the Current Write Resplitter Entry Start Position/Address in Resplitter Entrys Fifo--pvSa */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
static void *SplitterRspGetRsp(void *pvSptHdl)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;
	SPT_RSP_T *prSptRsp = NULL;

	if (NULL == prSpt) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for invalid args!\r\n"), DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		return NULL;
	}

	prSptRsp = (SPT_RSP_T *) (prSpt->pvRspCurPtr);
	if (NULL != prSptRsp) {
		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s -- RspCurPtr(0x%p), pvRspFirstTx(0x%p),")
			TEXT(" RspRP(0x%p), RspWP(0x%p)!\r\n"),
			DMX_FUNC_NAME, prSpt->pvRspCurPtr,
			prSpt->pvRspFirstTx, prSpt->pvRspRp,
			prSpt->pvRspWp);
	}

	return ((void *) prSpt->pvRspCurPtr);
}

/* ////////////////////////////////////////////////////////////////////////////////////////////// */
/* SplitterRspGetAu */
/* Get the Resplitter AU SA in current Write Resplitter Entry in Resplitter Entrys Fifo--pvSa */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
static void *SplitterRspGetAu(void *pvSptHdl)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;

	if (NULL == prSpt) {
		DMX_ASSERT(FALSE);
		return NULL;
	}

	return ((void *) (prSpt->pvRspCurPtr + sizeof(SPT_RSP_T)));
}

/* For Debug */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
/* SplitterSetRspEntryCountIncrease */
/* Increate the used Resplitter entry count  in Resplitter Entrys Fifo--pvSa */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
MRESULT SplitterSetRspEntryCountIncrease(void *pvSptHdl)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;

	if (NULL == prSpt) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prSpt->u4RspEntryCount++;

	MM_RETURN(RET_DMX_OK);
}

/* ////////////////////////////////////////////////////////////////////////////////////////////// */
/* SplitterSetRspEntryCountDecrease */
/* Decrease the used Resplitter entry count  in Resplitter Entrys Fifo--pvSa */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
MRESULT SplitterSetRspEntryCountDecrease(void *pvSptHdl)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;

	if (NULL == prSpt) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (prSpt->u4RspEntryCount > 0)
		prSpt->u4RspEntryCount--;

	MM_RETURN(RET_DMX_OK);
}

/* ////////////////////////////////////////////////////////////////////////////////////////////// */
/* SplitterRspAddLog */
/* 1. Add the entry from the Resplitter Entry List, it doesn't change Resplitter Entry WP, */
/*     because it has been changed before calling this function */
/* 2. increase the entry's txsize into its corresponding Respliter Stream Info */
/* 3. increase the Replitter Entry Count */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
MRESULT SplitterRspAddLog(void *pvSptHdl, SPT_RSP_T *pvAddEntry)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;
	SPT_RSP_T *prLastRspEntry = NULL;
	MRESULT mrRet = RET_DMX_OK;

	if ((NULL == prSpt) || (NULL == pvAddEntry)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail for invalid args\r\n"), DMX_FUNC_NAME);
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	if (!SplitterRspIsEnabled(pvSptHdl))
		MM_RETURN(RET_DMX_OK);

	if (NULL == SplitterGetRspFirstTx(pvSptHdl)) {
		prSpt->pvRspFirstTx = pvAddEntry;
		DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s -- set prSpt->pvRspFirstTx = 0x%08x!\r\n"),
			DMX_FUNC_NAME, pvAddEntry);
	}

	if (NULL != prSpt->pvRspFirstTx)
		DMX_ASSERT(prSpt->pvRspFirstTx == (void *)(prSpt->pvRspRp));

	pvAddEntry->prPrevEntry = NULL;
	pvAddEntry->prNextEntry = NULL;

	prLastRspEntry = SplitterGetRspLastTx(pvSptHdl);

	if (NULL != prLastRspEntry) {
		prLastRspEntry->prNextEntry = pvAddEntry;
		pvAddEntry->prPrevEntry = prLastRspEntry;
	}

	prSpt->pvRspLastTx = pvAddEntry;

	SplitterSetRspEntryCountIncrease(pvSptHdl);

	mrRet = SplitterRspUpdateExtInf(pvSptHdl, SPLITTER_RSP_INF_ADD, pvAddEntry);
	if (DMX_FAILED(mrRet)) {
		DmxLogE(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
			TEXT("[SPT] %s fail in SplitterRspUpdateExtInf(ADD),")
			TEXT("mrRet: 0x%x!\r\n"),
			DMX_FUNC_NAME, mrRet);
		MM_RETURN(mrRet);
	}

	DmxLogD(DMX_MOD_RSP, DMX_MOD_RSP_LOGLVL_LOGTX,
		TEXT("[SPT] %s -- Add Rsp Entry (0x%08x), PrevRspEntry(0x%08x)!\r\n"),
		DMX_FUNC_NAME, pvAddEntry, pvAddEntry->prPrevEntry);

	if (NULL != prSpt->pvRspFirstTx)
		DMX_ASSERT(prSpt->pvRspFirstTx == (void *)(prSpt->pvRspRp));

	MM_RETURN(RET_DMX_OK);
}

/* ////////////////////////////////////////////////////////////////////////////////////////////// */
/* SplitterSetRspLastPtxLen */
/* Log Last Tx len into u8RspLastPtxLen */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
MRESULT SplitterSetRspLastPtxLen(void *pvSptHdl, u64 u8PtxLen)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;

	if (NULL == prSpt) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prSpt->u8RspLastPtxLen = u8PtxLen;

	MM_RETURN(RET_DMX_OK);
}

/* ////////////////////////////////////////////////////////////////////////////////////////////// */
/* SplitterSetRspTx */
/* Set Resplitter entry handle to TX */
/* ////////////////////////////////////////////////////////////////////////////////////////////// */
MRESULT SplitterSetRspTx(void *pvSptHdl, void *pvRspTx)
{
	DMX_SPT_INST_T *prSpt = (DMX_SPT_INST_T *) pvSptHdl;

	if (NULL == prSpt) {
		DMX_ASSERT(FALSE);
		MM_RETURN(RET_DMX_PARAM_WRONG);
	}

	prSpt->pvRspTx = pvRspTx;

	MM_RETURN(RET_DMX_OK);
}
