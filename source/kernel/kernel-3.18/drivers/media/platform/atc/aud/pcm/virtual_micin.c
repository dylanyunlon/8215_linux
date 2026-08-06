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


#include <linux/fs.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/semaphore.h>

#include "winutil.h"
#include "micin.h"
#include "aud_pcm_dbg.h"
#include "bt_speech.h"
#include "speechdev.h"
#include "aud_if.h"

#include "pcm_debug.h"
#define LOG_TAG "vmicin"

static WAVE_DATA_BUF_T _rNdcDataBuf = {0};
static u32 _u4NdcCaptureTimes = 0;
static bool _fgNdcBufInited = false;
static VirtualMicIn *_prNdcMic = NULL;


static s32 CaptureStopAsrc(VirtualMicIn *pVMicIn)
{

	if(ASRC_CHSET_NUM != pVMicIn->u4AsrcIdx) {
		Asrc_Stop(pVMicIn->u4AsrcIdx);
		Asrc_UnInit(pVMicIn->u4AsrcIdx);
		pVMicIn->u4AsrcIdx = ASRC_CHSET_NUM;
	}

	return (NOERR);
}

static s32 CaptureStartAsrc(VirtualMicIn *pVMicIn)
{
	pVMicIn->rAsrcFmt.u4Chn = 2;
	pVMicIn->rAsrcFmt.u4IBW = DEF_DATA_BITS;
	pVMicIn->rAsrcFmt.u4OFS = pVMicIn->u4Fs;
	pVMicIn->rAsrcFmt.u4OBW = DEF_DATA_BITS;
	pVMicIn->rAsrcFmt.u4IFS = pVMicIn->u4MicFs;

	PCM_DEBUG(LOG_TAG, " Capture ASRC Cfg: IFS(%d), OFS(%d), IBW(%d), OBW(%d), CH(%d)\r\n",
		 (int)pVMicIn->rAsrcFmt.u4IFS, (int)pVMicIn->rAsrcFmt.u4OFS, (int)pVMicIn->rAsrcFmt.u4IBW,
		 (int)pVMicIn->rAsrcFmt.u4OBW, (int)pVMicIn->rAsrcFmt.u4Chn);

	if(pVMicIn->u4AsrcIdx < ASRC_CHSET_NUM) {
		/* ASRC has been malloc . Stop ASRC first.*/
		CaptureStopAsrc(pVMicIn);
	}

	if(pVMicIn->u4AsrcIdx >= ASRC_CHSET_NUM) {
		if(NOERR != AsrcMgr_AllocASRC(&pVMicIn->rAsrcFmt, false, &pVMicIn->u4AsrcIdx)) {
			PCM_ERROR(LOG_TAG, "CaptureStartAsrc: Alloc asrc err.\r\n");
			return (NORESOURCE);
		}
	}
	Asrc_Start(pVMicIn->u4AsrcIdx);
	return (NOERR);
}

void NdcVirtualMicIn_FsChange(u32 u4NewFs)
{
	PVirtualMicIn pVMicIn = (PVirtualMicIn)_prNdcMic;

	if (!pVMicIn)
		return ;

	_rNdcDataBuf.u4DataOff = 0;
	_rNdcDataBuf.u4DataSz = 0;


	if (STATE_STARTED != pVMicIn->u4State )
	{
		PCM_DEBUG(LOG_TAG, "NdcVirtualMicIn_FsChange. VMIc is not started!\r\n");
		return NOERR;
	}
	// Todo. Save data  & ASRC & Copy to user. Reset ASRC
	pVMicIn->u4RP = 0;
	if (pVMicIn->u4MicFs != pVMicIn->u4Fs)
	{
		CaptureStopAsrc(pVMicIn);
	}
	pVMicIn->u4MicFs = u4NewFs;
	pVMicIn->u4MicRP = _rNdcDataBuf.u4DataOff;
	if (pVMicIn->u4MicFs != pVMicIn->u4Fs)
	{
		return CaptureStartAsrc(pVMicIn);
	}

	return NOERR;
}

void NdcVirtualMicIn_CopyData(void *pvData, u32 u4DataSize)
{
    if (!_u4NdcCaptureTimes)
    	return;

    u32 u4CopySz = _rNdcDataBuf.u4ChBufSz - _rNdcDataBuf.u4DataOff;
    if (u4CopySz > u4DataSize)
    	u4CopySz = u4DataSize;
    memcpy((void *)(_rNdcDataBuf.u4Buf1 + _rNdcDataBuf.u4DataOff), pvData, u4CopySz);
    if (_rNdcDataBuf.u4Chn == 2)
		memcpy((void *)(_rNdcDataBuf.u4Buf2 + _rNdcDataBuf.u4DataOff), pvData, u4CopySz);
	_rNdcDataBuf.u4DataOff += u4CopySz;
	u4DataSize -= u4CopySz;
	if (u4DataSize)
	{
		if (u4DataSize > _rNdcDataBuf.u4ChBufSz)
			u4DataSize = _rNdcDataBuf.u4ChBufSz;
		memcpy((void *)(_rNdcDataBuf.u4Buf1), pvData, u4DataSize);
		if (_rNdcDataBuf.u4Chn == 2)
			memcpy((void *)(_rNdcDataBuf.u4Buf2), pvData, u4DataSize);
		_rNdcDataBuf.u4DataOff = u4DataSize;
	}
	if (_rNdcDataBuf.u4DataOff >= _rNdcDataBuf.u4ChBufSz)
		_rNdcDataBuf.u4DataOff = 0;

}

static int NdcVirtualMicInit(void)
{
    if (!_fgNdcBufInited)
    {
	    MicIn_GetBuffer(&_rNdcDataBuf);
		_rNdcDataBuf.u4Buf1 = (u32)vmalloc(_rNdcDataBuf.u4ChBufSz);
		if (0 == _rNdcDataBuf.u4Buf1)
		{
		    PCM_ERROR(LOG_TAG, "CaptureNdcInit: vmalloc _rNdcDataBuf.u4Buf1 err!\r\n");
		    return (NORESOURCE);
		}
		_rNdcDataBuf.u4Buf2 = (u32)vmalloc(_rNdcDataBuf.u4ChBufSz);
		if (0 == _rNdcDataBuf.u4Buf2)
		{
		    PCM_ERROR(LOG_TAG, "CaptureNdcInit: vmalloc _rNdcDataBuf.u4Buf2 err!\r\n");
		    return (NORESOURCE);
		}
    }

    return (NOERR);
}






static void CopyMicDataToAsrcIn(VirtualMicIn *pVMicIn)
{
	u32 u4Idx = 0;
	u32 u4WP = 0;
	u32 u4DataBytes = 0;
	u8 *pbAsrcIn1 = NULL;
	u8 *pbAsrcIn2 = NULL;
	u8 *pbMic1 = NULL;
	u8 *pbMic2 = NULL;

	WAVE_DATA_BUF_T *prBuf = {0};
	WAVE_DATA_BUF_T rAsrcIn = {0};
	WAVE_DATA_BUF_T rMicBuf = {0};

	if(NOERR != Asrc_GetIBuf(pVMicIn->u4AsrcIdx, &rAsrcIn)) {
		PCM_ERROR(LOG_TAG, "CopyMicDataToAsrcIn: Get ASRC input buffer error\r\n");
		return;
	}
	if (VMT_NORMAL == pVMicIn->eType)
	{
	MicIn_GetBuffer(&rMicBuf);
	u4WP = MicIn_GetWP();
		prBuf = &rMicBuf;
	}
	else
	{
		prBuf = &_rNdcDataBuf;
		u4WP = _rNdcDataBuf.u4DataOff;
	}

	u4DataBytes = AUD_GET_BUF_DATA_SZ(pVMicIn->u4MicRP, u4WP, prBuf->u4ChBufSz);
	if (pVMicIn->fgFirstFillASRC )
	{
		PCM_DEBUG(LOG_TAG, "CopyMicDataToAsrcIn. MIC R(%d) W(%d) bz(%d), ASRC off(%d) sz(%d) bz(%d)\r\n",
		pVMicIn->u4MicRP, u4WP, prBuf->u4ChBufSz,  rAsrcIn.u4DataOff,rAsrcIn.u4DataSz, rAsrcIn.u4ChBufSz);
	}

	if (u4DataBytes > rAsrcIn.u4DataSz)
		u4DataBytes = rAsrcIn.u4DataSz;

	u4DataBytes &= 0xFFFFFFF0;


	if(0 == u4DataBytes || (pVMicIn->fgFirstFillASRC && (u4DataBytes < 96))) {
		return;
	}

	pVMicIn->fgFirstFillASRC = false;

	for(u4Idx = 0; u4Idx < u4DataBytes; u4Idx++) {
		if(rAsrcIn.u4DataOff + u4Idx < rAsrcIn.u4ChBufSz) {
			pbAsrcIn1 = (u8 *)(rAsrcIn.u4Buf1 + rAsrcIn.u4DataOff + u4Idx);
			pbAsrcIn2 = (u8 *)(rAsrcIn.u4Buf2 + rAsrcIn.u4DataOff + u4Idx);
		} else {
			pbAsrcIn1 = (u8 *)(rAsrcIn.u4Buf1 + rAsrcIn.u4DataOff + u4Idx - rAsrcIn.u4ChBufSz);
			pbAsrcIn2 = (u8 *)(rAsrcIn.u4Buf2 + rAsrcIn.u4DataOff + u4Idx - rAsrcIn.u4ChBufSz);
		}

		if(pVMicIn->u4MicRP + u4Idx < prBuf->u4ChBufSz) {
			pbMic1 = (u8 *)(prBuf->u4Buf1 + pVMicIn->u4MicRP + u4Idx);
			pbMic2 = (u8 *)(prBuf->u4Buf2 + pVMicIn->u4MicRP + u4Idx);
		} else {
			pbMic1 = (u8 *)(prBuf->u4Buf1 + pVMicIn->u4MicRP + u4Idx - prBuf->u4ChBufSz);
			pbMic2 = (u8 *)(prBuf->u4Buf2 + pVMicIn->u4MicRP + u4Idx - prBuf->u4ChBufSz);
		}

		*pbAsrcIn1 = *pbMic1;
		*pbAsrcIn2 = *pbMic2;
	}

	pVMicIn->u4MicRP = (pVMicIn->u4MicRP + u4DataBytes) % prBuf->u4ChBufSz;

	rAsrcIn.u4DataOff = (rAsrcIn.u4DataOff + u4DataBytes) % rAsrcIn.u4ChBufSz;

	if(NOERR != Asrc_SetIWP(pVMicIn->u4AsrcIdx, rAsrcIn.u4DataOff)) {
		return;
	}
}

s32 VirtualMic_ResetRP(void * pThis)
{
    AUD_ASSERT(pThis);
	PVirtualMicIn pVMicIn = (PVirtualMicIn)pThis;
	if (STATE_STARTED != pVMicIn->u4State )
	{
		PCM_DEBUG(LOG_TAG, "VirtualMic_ReseetRP. VMIc is not started!\r\n");
		return NOERR;
	}
	pVMicIn->u4MicRP = MicIn_GetWP();
	pVMicIn->u4RP = 0;
	if (pVMicIn->u4MicFs != pVMicIn->u4Fs)
	{
		Asrc_Stop(pVMicIn->u4AsrcIdx);
		Asrc_Start(pVMicIn->u4AsrcIdx);
		pVMicIn->fgFirstFillASRC = true;
	}

	return NOERR;
}


s32 VirtualMic_Start(void * pThis)
{
    AUD_ASSERT(pThis);
	PVirtualMicIn pVMicIn = (PVirtualMicIn)pThis;
	PCM_INFO(LOG_TAG, "VirtualMic_Start\r\n");
	if (STATE_STARTED == pVMicIn->u4State )
	{
		PCM_WARN(LOG_TAG, "VirtualMic_Start. It has been started!\r\n");
		return NOERR;
	}
	else if (STATE_UNINIT == pVMicIn->u4State)
	{
		PCM_ERROR(LOG_TAG, "VirtualMic_Start error (invalid state)!\r\n");
		return (INVALIDSTATE);
	}
	else
	{
		s32 ret = NOERR;
		if (NOERR != MicIn_Start(pThis))
		{
			PCM_ERROR(LOG_TAG, "Failed to start MIC IN!\r\n");
			return (NORESOURCE);
		}
		pVMicIn->u4MicFs = MicIn_GetFS();
		pVMicIn->u4MicRP = MicIn_GetWP();
		if (pVMicIn->u4MicFs != pVMicIn->u4Fs)
		{
			ret = CaptureStartAsrc(pVMicIn);
		}
		pVMicIn->u4State = STATE_STARTED;
		return ret;
	}
}

s32 VirtualMic_Stop(void * pThis)
{
    AUD_ASSERT(pThis);
	PVirtualMicIn pVMicIn = (PVirtualMicIn)pThis;
	PCM_INFO(LOG_TAG, "VirtualMic_Stop\r\n");
	if (STATE_STARTED != pVMicIn->u4State )
	{
		return (NOERR);
	}
	if (pVMicIn->u4MicFs != pVMicIn->u4Fs)
	{
		CaptureStopAsrc(pVMicIn);
	}
	pVMicIn->u4State = STATE_STOPPED;
	return MicIn_Stop(pThis);
}

s32 NdcVirtualMic_Start(void * pThis)
{
    AUD_ASSERT(pThis);
	PVirtualMicIn pVMicIn = (PVirtualMicIn)pThis;
	PCM_INFO(LOG_TAG, "VirtualMic_Start\r\n");
	if (STATE_STARTED == pVMicIn->u4State )
	{
		PCM_WARN(LOG_TAG, "NdcVirtualMic_Start. It has been started!\r\n");
		return NOERR;
	}
	else if (STATE_UNINIT == pVMicIn->u4State)
	{
		PCM_ERROR(LOG_TAG, "NdcVirtualMic_Start error (invalid state)!\r\n");
		return (INVALIDSTATE);
	}
	else
	{
		s32 ret = NOERR;
		NdcVirtualMicInit();
		if (!_u4NdcCaptureTimes)
		{
			_rNdcDataBuf.u4DataOff = 0;
			_rNdcDataBuf.u4DataSz = 0;
		}
		_u4NdcCaptureTimes ++;
		SpeechDev_Enable(true, false, 16000);
		pVMicIn->u4MicFs = SpeechDev_GetSCOFS();
		pVMicIn->u4MicRP = _rNdcDataBuf.u4DataOff;
		if (pVMicIn->u4MicFs != pVMicIn->u4Fs)
		{
			ret = CaptureStartAsrc(pVMicIn);
		}
		pVMicIn->u4State = STATE_STARTED;
		return ret;
	}
}

s32 NdcVirtualMic_Stop(void * pThis)
{
    AUD_ASSERT(pThis);
	PVirtualMicIn pVMicIn = (PVirtualMicIn)pThis;
	PCM_INFO(LOG_TAG, "NdcVirtualMic_Stop\r\n");
	if (STATE_STARTED != pVMicIn->u4State )
	{
		return (NOERR);
	}
	if (pVMicIn->u4MicFs != pVMicIn->u4Fs)
	{
		CaptureStopAsrc(pVMicIn);
	}
	pVMicIn->u4State = STATE_STOPPED;
    if (!_u4NdcCaptureTimes)
    {
		PCM_WARN(LOG_TAG, " NdcVirtualMic_Stop Mic is not started!!!\r\n");
		return (NOERR);
    }
    _u4NdcCaptureTimes --;
	SpeechDev_Enable(false, false, 16000);
	return (NOERR);
}


u32 VirtualMic_GetBuffer(void * pThis, WAVE_DATA_BUF_T *prBuffer)
{
    AUD_ASSERT(pThis);
    AUD_ASSERT(prBuffer);

	PVirtualMicIn pVMicIn = (PVirtualMicIn)pThis;
	if (pVMicIn->u4MicFs == pVMicIn->u4Fs)
	{
		// Don't need SRC. Use data MIC In buffer directly.
		if (VMT_NORMAL == pVMicIn->eType )
		{
		MicIn_GetBuffer(prBuffer);
		prBuffer->u4DataSz = AUD_GET_BUF_DATA_SZ(pVMicIn->u4RP, MicIn_GetWP(), prBuffer->u4ChBufSz);
	}
	else
	{
			memcpy(prBuffer, (void *)&_rNdcDataBuf, sizeof(WAVE_DATA_BUF_T));
			prBuffer->u4DataSz = AUD_GET_BUF_DATA_SZ(pVMicIn->u4RP, prBuffer->u4DataOff, prBuffer->u4ChBufSz);
		}
		prBuffer->u4DataOff = pVMicIn->u4RP;
	}
	else
	{
		// Copy data to ASRC In first.
		// Return ASRC Out buffer
		CopyMicDataToAsrcIn(pVMicIn);
		if (NOERR != Asrc_GetOBuf(pVMicIn->u4AsrcIdx, prBuffer))
		{
			PCM_ERROR(LOG_TAG, "VirtualMic_GetBuffer:Get ASRC(%d) output buffer error\r\n", pVMicIn->u4AsrcIdx);
			return (INVALIDSTATE);
		}

	}

	return NOERR;
}

s32 VirtualMic_Setup(void * pThis, u32 u4Fs)
{
    AUD_ASSERT(pThis);
	PVirtualMicIn pVMicIn = (PVirtualMicIn)pThis;
	if (STATE_STARTED == pVMicIn->u4State )
	{
		PCM_ERROR(LOG_TAG, "VirtualMic_Setup error (invalid state)!\r\n");
		return INVALIDSTATE;
	}
	else
	{
		pVMicIn->u4Fs = u4Fs;
		pVMicIn->u4State = STATE_INITED;
	}
	PCM_INFO(LOG_TAG, "VirtualMic_Setup. Fs(%d)!\r\n", pVMicIn->u4Fs);
	return NOERR;
}


void VirtualMic_UpdateRP(void * pThis, u32 u4RP)
{
	PVirtualMicIn pVMicIn = (PVirtualMicIn)pThis;
    AUD_ASSERT(pThis);
	pVMicIn->u4RP = u4RP;
	if (pVMicIn->u4MicFs != pVMicIn->u4Fs)
	{
		Asrc_SetORP(pVMicIn->u4AsrcIdx, u4RP);
	}
}

u32  VirtualMic_GetFS(void * pThis)
{
	PVirtualMicIn pVMicIn = (PVirtualMicIn)pThis;
    AUD_ASSERT(pThis);
	return (pVMicIn->u4Fs);
}

u32  VirtualMic_GetWP(void * pThis)
{
	PVirtualMicIn pVMicIn = (PVirtualMicIn)pThis;
    AUD_ASSERT(pThis);
	if (pVMicIn->u4MicFs == pVMicIn->u4Fs)
	{
		if (VMT_NORMAL == pVMicIn->eType )
		return MicIn_GetWP();
		else
			return _rNdcDataBuf.u4DataOff;
	}
	else
	{
		// Copy data to ASRC In first.
		CopyMicDataToAsrcIn(pVMicIn);
		// return ASRC Output Write Point
		return Asrc_GetOWP(pVMicIn->u4AsrcIdx);
	}
}


s32 CreateVirtualMicIn(VIRTUAL_MIC_TYPE type, PVirtualMicIn *ppVMicIn)
{
	if (VMT_NDC == type && _prNdcMic)
	{
		PCM_ERROR(LOG_TAG, "CreateVirtualMicIn NDC Virtual is in used!!!\r\n");
		return (NORESOURCE);

	}
	*ppVMicIn = vmalloc(sizeof(VirtualMicIn));
	if(0 == *ppVMicIn) {
		PCM_ERROR(LOG_TAG, "CreateVirtualMicIn: vmalloc failed!\r\n");
		return (NORESOURCE);
	}
	memset(*ppVMicIn, 0, sizeof(VirtualMicIn));
	(*ppVMicIn)->eType = type;
	(*ppVMicIn)->u4AsrcIdx = ASRC_CHSET_NUM;
	(*ppVMicIn)->fgFirstFillASRC = true;
		(*ppVMicIn)->Start = VirtualMic_Start;
		(*ppVMicIn)->GetBuffer = VirtualMic_GetBuffer;
		(*ppVMicIn)->Stop =  VirtualMic_Stop;
		(*ppVMicIn)->Setup = VirtualMic_Setup;
		(*ppVMicIn)->UpdateRP = VirtualMic_UpdateRP;
		(*ppVMicIn)->GetFS = VirtualMic_GetFS;
		(*ppVMicIn)->GetWP = VirtualMic_GetWP;
		(*ppVMicIn)->ResetRP = VirtualMic_ResetRP;

	if (VMT_NDC == type)
	{
	    _prNdcMic = *ppVMicIn;
		(*ppVMicIn)->Start = NdcVirtualMic_Start;
		(*ppVMicIn)->Stop =  NdcVirtualMic_Stop;
	}
	PCM_INFO(LOG_TAG, "CreateVirtualMicIn. this(0x%x)!\r\n", (u32)*ppVMicIn);
	return NOERR;
}

s32 DeleteVirtualMicIn(PVirtualMicIn pVMicIn)
{
    AUD_ASSERT(pVMicIn);
	PCM_INFO(LOG_TAG, "DeleteVirtualMicIn. this(0x%x)!\r\n", (u32)pVMicIn);
	if (pVMicIn == _prNdcMic)
	{
		_prNdcMic = NULL;
	}
    pVMicIn->Stop(pVMicIn);
	vfree((void *)pVMicIn);
	return NOERR;
}








