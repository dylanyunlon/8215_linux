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

#include <linux/slab.h>
#include <linux/module.h>

#include "x_os.h"
#include "x_assert.h"
#include "drv_imgresz_errcode.h"
#include "imgresz_drv_inst.h"
#include "imgresz_log.h"
#include <linux/module.h>
#include <linux/semaphore.h>
/*#include <asm/current.h>*/
#include <linux/clk.h>
#include <linux/clk-private.h>
extern struct clk *clk_ac8317_imgr0;

/*-----------------------------------------------------------------------------
data declarations
----------------------------------------------------------------------------*/

s32 i4ImgResz_Drv_GetTicket(IMGRESZ_DRV_TICKET_T *pImgReszTicket)
{
	mutex_lock(&g_ImgReszMutex);
	clk_prepare_enable(clk_ac8317_imgr0);
	
	return i4ImgResz_Inst_GetInstance(&(pImgReszTicket->u4Ticket));
	
}
EXPORT_SYMBOL(i4ImgResz_Drv_GetTicket);


s32 i4ImgResz_Drv_ReleaseTicket(IMGRESZ_DRV_TICKET_T *pImgReszTicket)
{
	s32 i4Ret = 0;

	i4Ret = i4ImgResz_Inst_ReleaseInstance(pImgReszTicket->u4Ticket);
	clk_disable_unprepare(clk_ac8317_imgr0);
	//if ((*(u32 *)0xfd0000a0 & (1<<4)) !=0 || (*(u32 *)0xfd0000bc & (1<<4)) != 0)
		//pr_err("IMGR0 unprepare error:0xA0=%x;0xBC=%x;\n", *(u32 *)0xfd0000a0, *(u32 *)0xfd0000bc);

	mutex_unlock(&g_ImgReszMutex);
	return i4Ret;
	/*return i4ImgResz_Inst_ReleaseInstance(pImgReszTicket->u4Ticket);*/
}
EXPORT_SYMBOL(i4ImgResz_Drv_ReleaseTicket);


s32 i4ImgResz_Drv_GetState(IMGRESZ_DRV_TICKET_T *pImgReszTicket, IMGRESZ_DRV_SCALE_STATE_T *pImgReszState)
{
	s32 i4Ret = S_IMGRESZ_DRV_OK;

	pImgReszState->eState = eImgResz_Inst_GetState(pImgReszTicket->u4Ticket);
	return i4Ret;
}
EXPORT_SYMBOL(i4ImgResz_Drv_GetState);

s32 i4ImgResz_Drv_SetPriority(IMGRESZ_DRV_TICKET_T *pImgReszTicket, IMGRESZ_DRV_SCALE_PRIORITY  eScalePriority)
{
	s32 i4Ret = S_IMGRESZ_DRV_OK;

	if (eImgResz_Inst_GetState(pImgReszTicket->u4Ticket) != IMGRESZ_DRV_STATE_IDLE) {
		ASSERT(0);
		i4Ret = E_IMGRESZ_DRV_FAIL;
	} else {
		IMGRESZ_INST_T *prImgreszInst;

		i4ImgResz_Inst_GetInstanceObject(pImgReszTicket->u4Ticket, &prImgreszInst);
		prImgreszInst->u4ImgReszPriority = (u32)eScalePriority;
		prImgreszInst->u4ImgReszCurrPriority = prImgreszInst->u4ImgReszPriority;
	}

	return i4Ret;
}
EXPORT_SYMBOL(i4ImgResz_Drv_SetPriority);


s32 i4ImgResz_Drv_SetLock(IMGRESZ_DRV_TICKET_T *pImgReszTicket, bool fgLock)
{
	s32 i4Ret = S_IMGRESZ_DRV_OK;

	if (eImgResz_Inst_GetState(pImgReszTicket->u4Ticket) != IMGRESZ_DRV_STATE_IDLE) {
		ASSERT(0);
		i4Ret = E_IMGRESZ_DRV_FAIL;
	} else {
		IMGRESZ_INST_T *prImgreszInst;

		i4ImgResz_Inst_GetInstanceObject(pImgReszTicket->u4Ticket, &prImgreszInst);
		prImgreszInst->fgDoLock = fgLock;

		if (!fgLock) {
			i4Ret = i4ImgResz_Inst_SetLock(pImgReszTicket->u4Ticket, fgLock);
		}
	}

	return i4Ret;
}
EXPORT_SYMBOL(i4ImgResz_Drv_SetLock);


s32 i4ImgResz_Drv_SetScaleMode(IMGRESZ_DRV_TICKET_T *pImgReszTicket, IMGRESZ_DRV_SCALE_MODE eScaleMode)
{
	s32 i4Ret = S_IMGRESZ_DRV_OK;

	if (eImgResz_Inst_GetState(pImgReszTicket->u4Ticket) != IMGRESZ_DRV_STATE_IDLE) {
		/*ASSERT(0);*/
		i4Ret = E_IMGRESZ_DRV_FAIL;
	} else {
		IMGRESZ_INST_T *prImgreszInst;

		i4ImgResz_Inst_GetInstanceObject(pImgReszTicket->u4Ticket, &prImgreszInst);
		prImgreszInst->eImgReszScaleMd = eScaleMode;
	}

	return i4Ret;
}
EXPORT_SYMBOL(i4ImgResz_Drv_SetScaleMode);


s32 i4ImgResz_Drv_SetMMUTable(IMGRESZ_DRV_TICKET_T *pImgReszTicket, uintptr_t u4tableaddr)
{
	s32 i4Ret = S_IMGRESZ_DRV_OK;

	if (eImgResz_Inst_GetState(pImgReszTicket->u4Ticket) != IMGRESZ_DRV_STATE_IDLE) {
		/*ASSERT(0);*/
		i4Ret = E_IMGRESZ_DRV_FAIL;
	} else {
		IMGRESZ_INST_T *prImgreszInst;

		i4ImgResz_Inst_GetInstanceObject(pImgReszTicket->u4Ticket, &prImgreszInst);
		prImgreszInst->u4tableaddr = u4tableaddr;
	}

	return i4Ret;
}
EXPORT_SYMBOL(i4ImgResz_Drv_SetMMUTable);


s32 i4ImgResz_Drv_SetLumaKey(IMGRESZ_DRV_TICKET_T *pImgReszTicket, u8 u1LumaKey, bool fgEnableLumaKey)
{
	s32 i4Ret = S_IMGRESZ_DRV_OK;

	if (eImgResz_Inst_GetState(pImgReszTicket->u4Ticket) != IMGRESZ_DRV_STATE_IDLE) {
		ASSERT(0);
		i4Ret = E_IMGRESZ_DRV_FAIL;
	} else {
		IMGRESZ_INST_T *prImgreszInst;

		i4ImgResz_Inst_GetInstanceObject(pImgReszTicket->u4Ticket, &prImgreszInst);
		prImgreszInst->fgLumaKeyEnable = fgEnableLumaKey;
		prImgreszInst->u1LumaKey = u1LumaKey;
	}

	return i4Ret;
}


s32 i4ImgResz_Drv_SetSrcBufInfo(IMGRESZ_DRV_TICKET_T *pImgReszTicket, IMGRESZ_DRV_SRC_BUF_INFO_T *pSrcBufInfo)
{
	s32 i4Ret = S_IMGRESZ_DRV_OK;

	if (eImgResz_Inst_GetState(pImgReszTicket->u4Ticket) != IMGRESZ_DRV_STATE_IDLE) {
		ASSERT(0);
		i4Ret = E_IMGRESZ_DRV_FAIL;
	} else {
		IMGRESZ_INST_T *prImgreszInst;

		i4ImgResz_Inst_GetInstanceObject(pImgReszTicket->u4Ticket, &prImgreszInst);

		prImgreszInst->tImgReszSrcBufInfo = *pSrcBufInfo;

		switch (pSrcBufInfo->eSrcColorMode) {
		case IMGRESZ_DRV_INPUT_COL_MD_2BPP_IDX:
		case IMGRESZ_DRV_INPUT_COL_MD_4BPP_IDX:
		case IMGRESZ_DRV_INPUT_COL_MD_8BPP_IDX:
			x_memcpy((void *)prImgreszInst->abColorPallet, (void *)pSrcBufInfo->u4ColorPalletSa, 256 * 4);
			break;

		default:
			break;
		}
	}

	return i4Ret;
}
EXPORT_SYMBOL(i4ImgResz_Drv_SetSrcBufInfo);


s32 i4ImgResz_Drv_SetDstBufInfo(IMGRESZ_DRV_TICKET_T *pImgReszTicket, IMGRESZ_DRV_DST_BUF_INFO_T *pDstBufInfo)
{
	s32 i4Ret = S_IMGRESZ_DRV_OK;

	if (eImgResz_Inst_GetState(pImgReszTicket->u4Ticket) != IMGRESZ_DRV_STATE_IDLE) {
		ASSERT(0);
		i4Ret = E_IMGRESZ_DRV_FAIL;
	} else {
		IMGRESZ_INST_T *prImgreszInst;

		i4ImgResz_Inst_GetInstanceObject(pImgReszTicket->u4Ticket, &prImgreszInst);

		prImgreszInst->tImgReszDstBufInfo = *pDstBufInfo;
	}

	return i4Ret;
}
EXPORT_SYMBOL(i4ImgResz_Drv_SetDstBufInfo);

s32 i4ImgResz_Drv_SetBldBufInfo(IMGRESZ_DRV_TICKET_T *pImgReszTicket, IMGRESZ_DRV_BLD_BUF_INFO_T *pBldBufInfo)
{
	s32 i4Ret = S_IMGRESZ_DRV_OK;

	if (eImgResz_Inst_GetState(pImgReszTicket->u4Ticket) != IMGRESZ_DRV_STATE_IDLE) {
		ASSERT(0);
		i4Ret = E_IMGRESZ_DRV_FAIL;
	} else {
		IMGRESZ_INST_T *prImgreszInst;

		i4ImgResz_Inst_GetInstanceObject(pImgReszTicket->u4Ticket, &prImgreszInst);

		prImgreszInst->tImgReszBldBufInfo = *pBldBufInfo;
	}

	return i4Ret;
}

s32 i4ImgResz_Drv_SetPartialBufInfo(IMGRESZ_DRV_TICKET_T *pImgReszTicket, IMGRESZ_DRV_PARTIAL_INFO_T *pPartialBufInfo)
{
	s32 i4Ret = S_IMGRESZ_DRV_OK;

	if (eImgResz_Inst_GetState(pImgReszTicket->u4Ticket) != IMGRESZ_DRV_STATE_IDLE) {
		ASSERT(0);
		i4Ret = E_IMGRESZ_DRV_FAIL;
	} else {
		IMGRESZ_INST_T *prImgreszInst;

		i4ImgResz_Inst_GetInstanceObject(pImgReszTicket->u4Ticket, &prImgreszInst);

		prImgreszInst->tImgReszPartialBufInfo = *pPartialBufInfo;
	}

	return i4Ret;
}
EXPORT_SYMBOL(i4ImgResz_Drv_SetPartialBufInfo);

s32 i4ImgResz_Drv_SetJpegInfo(IMGRESZ_DRV_TICKET_T *pImgReszTicket, IMGRESZ_DRV_JPEG_INFO_T *prJpegInfo)
{
	s32 i4Ret = S_IMGRESZ_DRV_OK;

	if (eImgResz_Inst_GetState(pImgReszTicket->u4Ticket) != IMGRESZ_DRV_STATE_IDLE) {
		ASSERT(0);
		i4Ret = E_IMGRESZ_DRV_FAIL;
	} else {
		IMGRESZ_INST_T *prImgreszInst;

		i4ImgResz_Inst_GetInstanceObject(pImgReszTicket->u4Ticket, &prImgreszInst);

		prImgreszInst->tImgReszJpegInfo = *prJpegInfo;
	}

	return i4Ret;

}
EXPORT_SYMBOL(i4ImgResz_Drv_SetJpegInfo);

s32 i4ImgResz_Drv_SetRmInfo(IMGRESZ_DRV_TICKET_T *pImgReszTicket, IMGRESZ_DRV_RM_INFO_T *prRmInfo)
{
	s32 i4Ret = S_IMGRESZ_DRV_OK;

	if (eImgResz_Inst_GetState(pImgReszTicket->u4Ticket) != IMGRESZ_DRV_STATE_IDLE) {
		ASSERT(0);
		i4Ret = E_IMGRESZ_DRV_FAIL;
	} else {
		IMGRESZ_INST_T *prImgreszInst;

		i4ImgResz_Inst_GetInstanceObject(pImgReszTicket->u4Ticket, &prImgreszInst);

		prImgreszInst->tImgReszRmInfo = *prRmInfo;
	}

	return i4Ret;
}
EXPORT_SYMBOL(i4ImgResz_Drv_SetRmInfo);

s32 i4ImgResz_Drv_Set_Scale1To1(IMGRESZ_DRV_TICKET_T *pImgReszTicket, bool fg1To1Scale)
{
	s32 i4Ret = S_IMGRESZ_DRV_OK;

	if (eImgResz_Inst_GetState(pImgReszTicket->u4Ticket) != IMGRESZ_DRV_STATE_IDLE) {
		ASSERT(0);
		i4Ret = E_IMGRESZ_DRV_FAIL;
	} else {
		IMGRESZ_INST_T *prImgreszInst;

		i4ImgResz_Inst_GetInstanceObject(pImgReszTicket->u4Ticket, &prImgreszInst);
		prImgreszInst->fg1To1Scale = fg1To1Scale;
	}

	return i4Ret;
}

s32 i4ImgResz_Drv_Set_Y_Only(IMGRESZ_DRV_TICKET_T *pImgReszTicket, bool fgYOnly)
{
	s32 i4Ret = S_IMGRESZ_DRV_OK;

	if (eImgResz_Inst_GetState(pImgReszTicket->u4Ticket) != IMGRESZ_DRV_STATE_IDLE) {
		ASSERT(0);
		i4Ret = E_IMGRESZ_DRV_FAIL;
	} else {
		IMGRESZ_INST_T *prImgreszInst;

		i4ImgResz_Inst_GetInstanceObject(pImgReszTicket->u4Ticket, &prImgreszInst);
		prImgreszInst->fgYSrcOnly = fgYOnly;
	}

	return i4Ret;
}

s32 i4ImgResz_Drv_DoScale(IMGRESZ_DRV_TICKET_T *pImgReszTicket, IMGRESZ_DRV_DO_SCALE_T *pDoScale)
{
	s32 i4Ret = S_IMGRESZ_DRV_OK;

	if (eImgResz_Inst_GetState(pImgReszTicket->u4Ticket) != IMGRESZ_DRV_STATE_IDLE) {
		ASSERT(0);
		i4Ret = E_IMGRESZ_DRV_FAIL;
	} else {
		IMGRESZ_INST_T *prImgreszInst;

		i4ImgResz_Inst_GetInstanceObject(pImgReszTicket->u4Ticket, &prImgreszInst);

		if (prImgreszInst->fgDoLock) {
			i4Ret = i4ImgResz_Inst_SetLock(pImgReszTicket->u4Ticket, TRUE);
		}

		if (i4Ret >= 0) {
			prImgreszInst->fgDoLock = FALSE;
			vImgResz_Inst_IncPriority();
			vImgResz_Inst_DispatchHw(pImgReszTicket->u4Ticket);
		}
	}

	return i4Ret;
}
EXPORT_SYMBOL(i4ImgResz_Drv_DoScale);


s32 i4ImgResz_Drv_StopScale(IMGRESZ_DRV_TICKET_T *pImgReszTicket)
{
	s32 i4Ret = S_IMGRESZ_DRV_OK;

	IMGR_LOG(IMGR_LOG_LVL_DBG, "i4ImgResz_Drv_StopScale!!!\r\n");
	i4Ret = i4ImgResz_Inst_StopResize(pImgReszTicket->u4Ticket);
	IMGR_LOG(IMGR_LOG_LVL_DBG, "i4ImgResz_Drv_StopScale exit!!!\r\n");
	return i4Ret;
}
EXPORT_SYMBOL(i4ImgResz_Drv_StopScale);

s32 i4ImgResz_Drv_RegFinishNotifyCallback(IMGRESZ_DRV_TICKET_T *pImgReszTicket,
	IMGRESZ_DRV_NOTIFY_CB_REG_T *prNotifyCallbackReg)
{
	IMGRESZ_INST_T *prImgreszInst;

	i4ImgResz_Inst_GetInstanceObject(pImgReszTicket->u4Ticket, &prImgreszInst);
	prImgreszInst->rNotifyCallbackReg = *prNotifyCallbackReg;

	return S_IMGRESZ_DRV_OK;
}
EXPORT_SYMBOL(i4ImgResz_Drv_RegFinishNotifyCallback);

s32 i4ImgResz_Drv_UnregFinishNotifyCallback(IMGRESZ_DRV_TICKET_T *pImgReszTicket,
	IMGRESZ_DRV_NOTIFY_CB_REG_T *prNotifyCallbackReg)
{
	IMGRESZ_INST_T *prImgreszInst;

	i4ImgResz_Inst_GetInstanceObject(pImgReszTicket->u4Ticket, &prImgreszInst);
	prImgreszInst->rNotifyCallbackReg.pvCallBackFunc = NULL;
	prImgreszInst->rNotifyCallbackReg.pvPrivData = NULL;

	return S_IMGRESZ_DRV_OK;
}


