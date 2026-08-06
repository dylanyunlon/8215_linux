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

/******************************************************************************
*[Description]
* implementation for  ASRC (Audio Sample Rate Convertion)
******************************************************************************/

#include "asrc.h"
#include "aud_pcm_dbg.h"
#include "strmproc.h"
#include "speechenhance.h"


static AsrcMgr g_rAsrcMgr;
static Asrc asrc_list[ASRC_CHSET_NUM];


static s32 ASRC_ISTCB(u32 u4AsrcObj, u32 u4IntType)
{
	return NOERR;
}

s32 Asrc_Init(u32 u4ChIdx)
{
	asrc_list[u4ChIdx].m_prAsrcChs = NULL;
	asrc_list[u4ChIdx].m_u4IWP = 0;
	asrc_list[u4ChIdx].m_u4ORP = 0;
	asrc_list[u4ChIdx].m_u4Idx = u4ChIdx;

	return NOERR;
}

s32 Asrc_UnInit(u32 u4ChIdx)
{
	pr_info("[PCM INFO]Asrc_UnInit: Index is %d!\r\n", (u32)u4ChIdx);
	if ((asrc_list[u4ChIdx].m_prAsrcChs) && (asrc_list[u4ChIdx].m_prAsrcChs->Delete)) {
		asrc_list[u4ChIdx].m_prAsrcChs->Delete(asrc_list[u4ChIdx].m_prAsrcChs);
		asrc_list[u4ChIdx].m_prAsrcChs = NULL;
	} else {
		pr_err("[PCM ERR]Asrc_UnInit: NULL Pointer, Index is %d!\r\n", (u32)u4ChIdx);
		return INVALIDPRAM;
	}

	return NOERR;
}

s32 Asrc_Setup(u32 u4ChIdx, PASRC_CHS_CLS_PUB pvAsrcChs, PASRC_CHS_FMT_T prFmt)
{
	pr_info("[PCM INFO]Asrc_Setup: Index is %d. Chs(0x%x)!\r\n", (u32)u4ChIdx, (u32)pvAsrcChs);
	if (pvAsrcChs) {
		ASRC_CHS_ISRCB_T rISRCB;
		ASRC_CHS_CFG_T rCfg;
		AUD_DATA_BUF_T rInBuf, rOutBuf;

		asrc_list[u4ChIdx].m_prAsrcChs = pvAsrcChs;
		asrc_list[u4ChIdx].m_u4Idx = u4ChIdx;

		rISRCB.pfnCb = ASRC_ISTCB;
		rISRCB.u4Param = (u32)&asrc_list[u4ChIdx];
		rISRCB.u4IntType = ASRC_IBUF_EMPTY_INT | ASRC_OBUF_OV_INT;

		rCfg.prFmt = prFmt;
		rCfg.prIsrCb = &rISRCB;

		asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.Setup(asrc_list[u4ChIdx].m_prAsrcChs, &rCfg);

		asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.GetIBuf(asrc_list[u4ChIdx].m_prAsrcChs, &rInBuf);
		asrc_list[u4ChIdx].m_rIBuf.u4Buf1 = rInBuf.u4VirSAdr;
		asrc_list[u4ChIdx].m_rIBuf.u4Buf2 = rInBuf.u4VirSAdr + rInBuf.u4ChBufSz;
		asrc_list[u4ChIdx].m_rIBuf.u4ChBufSz = rInBuf.u4ChBufSz;
		asrc_list[u4ChIdx].m_rIBuf.u4Chn = rInBuf.u4Chn;
		pr_info("[PCM INFO]Asrc_Setup: m_rIBuf.u4Buf1:0x%lx,m_rIBuf.u4Buf2:0x%lx,rInBuf.u4ChBufSz:%ld\r\n", asrc_list[u4ChIdx].m_rIBuf.u4Buf1,asrc_list[u4ChIdx].m_rIBuf.u4Buf2, rInBuf.u4ChBufSz);
		memset_io((void *)(asrc_list[u4ChIdx].m_rIBuf.u4Buf1), 0, rInBuf.u4ChBufSz);
		//pr_info("QK =======\r\n");
		memset_io((void *)(asrc_list[u4ChIdx].m_rIBuf.u4Buf2), 0, rInBuf.u4ChBufSz);

		//memset((void *)(asrc_list[u4ChIdx].m_rIBuf.u4Buf1), 0, rInBuf.u4ChBufSz);
		pr_info("QK =======\r\n");
		//memset((void *)(asrc_list[u4ChIdx].m_rIBuf.u4Buf2), 0, rInBuf.u4ChBufSz);

		asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.GetOBuf(asrc_list[u4ChIdx].m_prAsrcChs, &rOutBuf);
		asrc_list[u4ChIdx].m_rOBuf.u4Buf1 = rOutBuf.u4VirSAdr;
		asrc_list[u4ChIdx].m_rOBuf.u4Buf2 = rOutBuf.u4VirSAdr + rOutBuf.u4ChBufSz;
		asrc_list[u4ChIdx].m_rOBuf.u4ChBufSz = rOutBuf.u4ChBufSz;
		asrc_list[u4ChIdx].m_rOBuf.u4Chn = rOutBuf.u4Chn;
		memset_io((void *)(asrc_list[u4ChIdx].m_rOBuf.u4Buf1), 0, rOutBuf.u4ChBufSz);
		memset_io((void *)(asrc_list[u4ChIdx].m_rOBuf.u4Buf2), 0, rOutBuf.u4ChBufSz);

		//memset((void *)(asrc_list[u4ChIdx].m_rOBuf.u4Buf1), 0, rOutBuf.u4ChBufSz);
		//memset((void *)(asrc_list[u4ChIdx].m_rOBuf.u4Buf2), 0, rOutBuf.u4ChBufSz);

		asrc_list[u4ChIdx].m_u4InToOutScale = (prFmt->u4OBW * prFmt->u4OFS / prFmt->u4IBW / prFmt->u4IFS) + 1;
	}

	return NOERR;
}

s32 Asrc_Start(u32 u4ChIdx)
{
	if ((asrc_list[u4ChIdx].m_prAsrcChs) && (asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.Start)) {
		asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.Start(asrc_list[u4ChIdx].m_prAsrcChs, 0);
	} else {
		pr_err("[PCM ERR]Asrc_Start: NULL Pointer, Index is %d!\r\n", (u32)u4ChIdx);
		return INVALIDPRAM;
	}

	return NOERR;
}

s32 Asrc_Stop(u32 u4ChIdx)
{
	if ((asrc_list[u4ChIdx].m_prAsrcChs) && (asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.Stop)) {
		asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.Stop(asrc_list[u4ChIdx].m_prAsrcChs, 0);
	} else {
		pr_err("[PCM ERR]Asrc_Stop: NULL Pointer, Index is %d!\r\n", (u32)u4ChIdx);
		return INVALIDPRAM;
	}
	asrc_list[u4ChIdx].m_u4IWP = 0;
	asrc_list[u4ChIdx].m_u4ORP = 0;
	asrc_list[u4ChIdx].m_rOBuf.u4DataOff = 0;
	asrc_list[u4ChIdx].m_rOBuf.u4DataSz = 0;
	asrc_list[u4ChIdx].m_rIBuf.u4DataOff = 0;
	asrc_list[u4ChIdx].m_rIBuf.u4DataSz = 0;

	return NOERR;
}

/*========================================//
			#define Asrc_BufferCtrl
//========================================*/

s32 Asrc_SetIWP(u32 u4ChIdx, u32 u4WP)
{
	if ((asrc_list[u4ChIdx].m_prAsrcChs) && (asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.SetIWP)) {
		asrc_list[u4ChIdx].m_u4IWP = u4WP;
		asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.SetIWP(asrc_list[u4ChIdx].m_prAsrcChs,
			asrc_list[u4ChIdx].m_u4IWP);
	} else {
		pr_err("[PCM ERR]Asrc_SetIWP: NULL Pointer, Index is %d!\r\n", (u32)u4ChIdx);
		return INVALIDPRAM;
	}

	return NOERR;
}

s32 Asrc_SetORP(u32 u4ChIdx, u32 u4RP)
{
	asrc_list[u4ChIdx].m_u4ORP = u4RP;

	if ((asrc_list[u4ChIdx].m_prAsrcChs) && (asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.SetORP)) {
		if (u4RP == asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.GetOWP(asrc_list[u4ChIdx].m_prAsrcChs)) {
			if (u4RP < ASRC_BUFFER_RESERVE_SIZE) {
				u4RP +=  asrc_list[u4ChIdx].m_rOBuf.u4ChBufSz;
			}
			u4RP -= ASRC_BUFFER_RESERVE_SIZE;
		}
		asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.SetORP(asrc_list[u4ChIdx].m_prAsrcChs, u4RP);
	} else {
		pr_err("[PCM ERR]Asrc_SetORP: NULL Pointer, Index is %d!\r\n", (u32)u4ChIdx);
		return INVALIDPRAM;
	}

	return NOERR;
}

u32 Asrc_GetIRP(u32 u4ChIdx)
{
	return asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.GetIRP(asrc_list[u4ChIdx].m_prAsrcChs);
}

u32 Asrc_GetOWP(u32 u4ChIdx)
{
	return asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.GetOWP(asrc_list[u4ChIdx].m_prAsrcChs);
}


s32 Asrc_GetIDataSz(u32 u4ChIdx, u32 *pu4Size)
{
	u32 u4IRP = 0;
	u32 u4DataSz = 0;

	if (pu4Size && (asrc_list[u4ChIdx].m_prAsrcChs) && (asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.GetIRP)) {
		u4IRP = asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.GetIRP(asrc_list[u4ChIdx].m_prAsrcChs);
		u4DataSz = AUD_GET_BUF_DATA_SZ(u4IRP, asrc_list[u4ChIdx].m_u4IWP, asrc_list[u4ChIdx].m_rIBuf.u4ChBufSz);
	} else {
		pr_err("[PCM ERR]Asrc_GetIDataSz: NULL Pointer, Index is %d!\r\n", (u32)u4ChIdx);
		return INVALIDPRAM;
	}

	u4DataSz &= 0xFFFFFFF0;
	*pu4Size = u4DataSz;

	return NOERR;
}

s32 Asrc_GetODataSz(u32 u4ChIdx, u32 *pu4Size)
{
	u32 u4OWP = 0;
	u32 u4DataSz = 0;

	if (pu4Size && (asrc_list[u4ChIdx].m_prAsrcChs) && (asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.GetOWP)) {
		u4OWP = asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.GetOWP(asrc_list[u4ChIdx].m_prAsrcChs);
		u4DataSz = AUD_GET_BUF_DATA_SZ(asrc_list[u4ChIdx].m_u4ORP, u4OWP,
			asrc_list[u4ChIdx].m_rOBuf.u4ChBufSz);
	} else {
		pr_err("[PCM ERR]Asrc_GetODataSz: NULL Pointer, Index is %d!\r\n", (u32)u4ChIdx);
		return INVALIDPRAM;
	}

	if (!(asrc_list[u4ChIdx].m_prAsrcChs->GetBufState(asrc_list[u4ChIdx].m_prAsrcChs) & ASRC_BUF_ST_IBUF_EMPTY)) {
		if (u4DataSz > ASRC_OUTPUT_SAFE_READ_SIZE) {
			u4DataSz -= ASRC_OUTPUT_SAFE_READ_SIZE;
		} else {
			u4DataSz = 0;
		}
	}
	u4DataSz &= 0xFFFFFFF0;
	*pu4Size = u4DataSz;

	return NOERR;
}

s32 Asrc_GetIBuf(u32 u4ChIdx, PWAVE_DATA_BUF_T prBuf)
{
	u32 u4IRP = 0;
	u32 u4FreeSz = 0;
	u32 u4DataSz = 0;
	u32 u4OWP = 0;
	u32 u4OFreeSz = 0;
	u32 u4MaxFreeSz = 0;

	if ((!asrc_list[u4ChIdx].m_prAsrcChs) || (!asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.GetIRP)) {
		pr_err("[PCM ERR]Asrc_GetIBuf: NULL Pointer Index is %d!\r\n", (u32)u4ChIdx);
		return INVALIDPRAM;
	}

	u4IRP = asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.GetIRP(asrc_list[u4ChIdx].m_prAsrcChs);
	//asrc_list[u4ChIdx].m_prAsrcChs->LogAttribute(asrc_list[u4ChIdx].m_prAsrcChs);
	//g_rAsrcMgr.m_prAsrcMgr->LogAllRegs(g_rAsrcMgr.m_prAsrcMgr);
	u4FreeSz = AUD_GET_BUF_FREE_SZ(u4IRP, asrc_list[u4ChIdx].m_u4IWP, asrc_list[u4ChIdx].m_rIBuf.u4ChBufSz);
	u4DataSz = AUD_GET_BUF_DATA_SZ(u4IRP, asrc_list[u4ChIdx].m_u4IWP, asrc_list[u4ChIdx].m_rIBuf.u4ChBufSz);

	/*=============== prevent ASRC OBuf Overflow =============================*/
	u4OWP = asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.GetOWP(asrc_list[u4ChIdx].m_prAsrcChs);
	u4OFreeSz = AUD_GET_BUF_FREE_SZ(asrc_list[u4ChIdx].m_u4ORP, u4OWP,
		asrc_list[u4ChIdx].m_rOBuf.u4ChBufSz);
	u4MaxFreeSz = u4OFreeSz / asrc_list[u4ChIdx].m_u4InToOutScale;

	if (u4FreeSz + u4DataSz >= u4MaxFreeSz) {
		u4FreeSz = (u4MaxFreeSz > u4DataSz) ? (u4MaxFreeSz - u4DataSz) : 0;
	}
	/*===============================================================*/

	if (u4FreeSz > ASRC_BUFFER_RESERVE_SIZE) {
		u4FreeSz -= ASRC_BUFFER_RESERVE_SIZE;
	} else {
		u4FreeSz = 0;
	}
	u4FreeSz &= 0xFFFFFFF0;

	asrc_list[u4ChIdx].m_rIBuf.u4DataOff = asrc_list[u4ChIdx].m_u4IWP;
	asrc_list[u4ChIdx].m_rIBuf.u4DataSz = u4FreeSz;
	x_memcpy(prBuf, &asrc_list[u4ChIdx].m_rIBuf, sizeof(WAVE_DATA_BUF_T));

	return NOERR;
}

s32 Asrc_GetOBuf(u32 u4ChIdx, PWAVE_DATA_BUF_T prBuf)
{
	if ((asrc_list[u4ChIdx].m_prAsrcChs) && (asrc_list[u4ChIdx].m_prAsrcChs->rHwIf.GetOWP)) {
		u32 u4DataSz = 0;

		if (NOERR != Asrc_GetODataSz(u4ChIdx, &u4DataSz)) {
			return INVALIDPRAM;
		}

		asrc_list[u4ChIdx].m_rOBuf.u4DataOff = asrc_list[u4ChIdx].m_u4ORP;
		asrc_list[u4ChIdx].m_rOBuf.u4DataSz = u4DataSz;
		x_memcpy(prBuf, &asrc_list[u4ChIdx].m_rOBuf, sizeof(WAVE_DATA_BUF_T));
	} else {
		pr_err("[PCM ERR]Asrc_GetOBuf: NULL Pointer Index is %d!\r\n", (u32)u4ChIdx);
		return INVALIDPRAM;
	}

	return NOERR;
}


/***************************************
*
*	 ASRC Manager function
*
***************************************/

s32 AsrcMgr_Init(void)
{
	g_rAsrcMgr.m_prAsrcMgr = AsrcMgr_New(GPS_ASRC, ASRC_IBUF_CH_SZ, ASRC_OBUF_CH_SZ);
	if (g_rAsrcMgr.m_prAsrcMgr) {
		g_rAsrcMgr.m_u4SpeechPalette = g_rAsrcMgr.m_prAsrcMgr->SetFixPalette(g_rAsrcMgr.m_prAsrcMgr, 8000);
		g_rAsrcMgr.m_prAsrcMgr->SetFixAsrc(g_rAsrcMgr.m_prAsrcMgr);
	} else {
		g_rAsrcMgr.m_u4SpeechPalette = ASRC_PALETTE_NUM - 1U;
		pr_err("[PCM ERR]AsrcMgr_Init: New Asrc Mgr Error!\r\n");
		return NORESOURCE;
	}

	return NOERR;
}

s32 AsrcMgr_UnInit(void)
{
	g_rAsrcMgr.m_prAsrcMgr->ClrFixPalette(g_rAsrcMgr.m_prAsrcMgr, g_rAsrcMgr.m_u4SpeechPalette);
	g_rAsrcMgr.m_prAsrcMgr->ClrFixAsrc(g_rAsrcMgr.m_prAsrcMgr);
	g_rAsrcMgr.m_prAsrcMgr->Delete(g_rAsrcMgr.m_prAsrcMgr);

	return NOERR;
}

s32 AsrcMgr_SetSpeechFs(u32 u4SpeechFs)
{
	g_rAsrcMgr.m_u4SpeechFs = u4SpeechFs;

	return NOERR;
}

s32 AsrcMgr_AllocASRC(PASRC_CHS_FMT_T prChCfg, bool fgSpeech, u32 *pu4Idx)
{
	u32 u4Idx = 0;
	PASRC_CHS_CLS_PUB pvChs;

	/* IFS == 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000
	    OFS == 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000*/
	if ((prChCfg->u4OFS < 8000) || (prChCfg->u4OFS > 48000) ||
		(prChCfg->u4IFS < 8000) || (prChCfg->u4IFS > 48000) ||
		(prChCfg->u4Chn > 2)) {
		pr_err("[PCM ERR]AsrcMgr_AllocASRC: Fmt error: OFS(%d) IFS(%d) Chn(%d)\r\n",
			(u32)prChCfg->u4OFS, (u32)prChCfg->u4IFS, (u32)prChCfg->u4Chn);
		return INVALIDPRAM;
	}

	if (fgSpeech) {
		pr_info("[PCM INFO]AsrcMgr_AllocASRC: Modify FixPalette sample rate to %d\r\n",
			(u32)g_rAsrcMgr.m_u4SpeechFs);
		g_rAsrcMgr.m_u4SpeechPalette = g_rAsrcMgr.m_prAsrcMgr->ModifyFixPalette(g_rAsrcMgr.m_prAsrcMgr,
			g_rAsrcMgr.m_u4SpeechFs);
	}

	for (u4Idx = 0; u4Idx < ASRC_CHSET_NUM; u4Idx++) {
		if (NULL == asrc_list[u4Idx].m_prAsrcChs) {
			break;
		}
	}
	if (ASRC_CHSET_NUM == u4Idx) {
		pr_err("[PCM ERR]AsrcMgr_AllocASRC: No free ASRC\r\n");
		return NORESOURCE;
	}

	pvChs = g_rAsrcMgr.m_prAsrcMgr->AllocAsrc(g_rAsrcMgr.m_prAsrcMgr, fgSpeech);
	prChCfg->u4IPalette = fgSpeech ? g_rAsrcMgr.m_u4SpeechPalette : ASRC_PALETTE_NUM;
	prChCfg->u4OPalette = ASRC_PALETTE_NUM;

	if (pvChs) {
		Asrc_Init(u4Idx);
		Asrc_Setup(u4Idx, pvChs, prChCfg);
	} else {
		pr_err("[PCM ERR]AsrcMgr_AllocASRC: AsrcMgr No free ASRC\r\n");
		return NORESOURCE;
	}
	printk("[PCM INFO]AsrcMgr_AllocASRC index(%d) pvChs(0x%x) \r\n", u4Idx, (u32)pvChs);

	*pu4Idx = u4Idx;

	return NOERR;
}

s32 Asrc_HibernationCtrl(bool fgWakeUp)
{
	g_rAsrcMgr.m_prAsrcMgr->HibernationCtrl(g_rAsrcMgr.m_prAsrcMgr, fgWakeUp);

	return NOERR;
}

