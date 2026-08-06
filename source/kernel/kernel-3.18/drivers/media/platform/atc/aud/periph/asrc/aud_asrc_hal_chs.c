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

#include "aud_asrc_hal.h"

static PASRC_CHS_CLS _aprAsrcChs[ASRC_TYPE_NUM][ASRC_CHSET_NUM]; 

//===================================//
  #define CodeSight_AsrcHal_Chs_Init
//===================================//

static u32 AsrcChs_FmtInit(PASRC_CHS_CLS prThis, PASRC_CHS_FMT_T prParam)
{
    u32 u4Ret = AUD_RET_FAIL; 
    PASRC_HW_CLS_PUB prHw = prThis->prMgr->prHw;
    PASRC_MGR_CLS_PUB prMgr = &prThis->prMgr->rPub;
    PASRC_CHS_FMT_T prFmt = &(prThis->rFmt); 

    x_memcpy(prFmt, prParam, sizeof(ASRC_CHS_FMT_T));

    prHw->SetChMono(prHw, prThis->u4Idx, (1 == prFmt->u4Chn));

    if (prFmt->u4OPalette >= ASRC_PALETTE_NUM) {
        prFmt->u4OPalette = prMgr->AllocPalette(prMgr, prFmt->u4OFS);
    }
    if (prFmt->u4IPalette >= ASRC_PALETTE_NUM) {
        prFmt->u4IPalette = prMgr->AllocPalette(prMgr, prFmt->u4IFS);
    }
    
    if (prFmt->u4OPalette < ASRC_PALETTE_NUM && prFmt->u4IPalette < ASRC_PALETTE_NUM) 
    {       
        prHw->SetChFs(prHw, prThis->u4Idx, prFmt->u4IPalette, prFmt->u4OPalette);
        prHw->SetChBitWidth(prHw, prThis->u4Idx, prFmt->u4IBW, prFmt->u4OBW);
        prHw->SetChCalcAmount(prHw, prThis->u4Idx, ASRC_DEF_CACL_AMOUNT);
        u4Ret = AUD_RET_OK;
    } 
    else 
    {
        ASRCLOG_ERR(T("[Chs(%d-%d)] Set ChCfg Failed. No FS palette.\r\n"), 
            prThis->prMgr->eType, (s32)(prThis->u4Idx));       
    } 

    return (u4Ret);
}


static void AsrcChs_BufferInit(PASRC_CHS_CLS prThis)
{
    PASRC_CHS_BUF_T prBuf = &(prThis->rBuf);
    PASRC_HW_CLS_PUB prHw = prThis->prMgr->prHw;
    
    prBuf->u4State = ASRC_BUF_ST_IBUF_EMPTY; 
    
    prHw->SetIRP(prHw, prThis->u4Idx, prBuf->u4ISAdr);
    prHw->SetIWP(prHw, prThis->u4Idx, prBuf->u4ISAdr);
    prHw->SetORP(prHw, prThis->u4Idx, prBuf->u4OSAdr);
    prHw->SetOWP(prHw, prThis->u4Idx, prBuf->u4OSAdr);

    if (1 == prThis->rFmt.u4Chn) {
        prBuf->u4IVirSAdr[1] = prBuf->u4IVirSAdr[0];
        prBuf->u4OVirSAdr[1] = prBuf->u4OVirSAdr[0];
    } else {
        prBuf->u4IVirSAdr[1] = prBuf->u4IVirSAdr[0] + prBuf->u4IChSize;
        prBuf->u4OVirSAdr[1] = prBuf->u4OVirSAdr[0] + prBuf->u4OChSize;
    }

    ASRCLOG_DBG(T("[Chs(%d-%d)] Buf init: IN(0x%x, 0x%x, 0x%x, 0x%x) OUT(0x%x, 0x%x, 0x%x, 0x%x).\r\n"), 
        prThis->prMgr->eType, (u32)(prThis->u4Idx),
        (u32)(prBuf->u4ISAdr), (u32)(prBuf->u4IVirSAdr[0]), (u32)(prBuf->u4IVirSAdr[1]), (u32)(prBuf->u4IChSize),
        (u32)(prBuf->u4OSAdr), (u32)(prBuf->u4OVirSAdr[0]), (u32)(prBuf->u4OVirSAdr[1]), (u32)(prBuf->u4OChSize));  

    ASRCLOG_DBG(T("[Chs] OutRP(0x%x).\r\n"), 
        (u32)(prHw->GetOWP(prHw, prThis->u4Idx) - prThis->rBuf.u4OSAdr));  
}


static u32 AsrcChs_RegISRCB(PASRC_CHS_CLS prThis, PASRC_CHS_ISRCB_T prIsrCb)
{
    u32 u4Ret = AUD_RET_OK;
    PASRC_HW_CLS_PUB prHw = prThis->prMgr->prHw;
    
    if (prIsrCb->pfnCb)      // Enable interrupt;
    {     
        prThis->rIsrCb.pfnCb = prIsrCb->pfnCb;
        prThis->rIsrCb.u4Param = prIsrCb->u4Param;

        if (ASRC_IBUF_AMOUNT_INT & prIsrCb->u4IntType) {
            u32 u4Amount = prThis->rBuf.u4IChSize >> 1;
            prHw->SetIBufIntrCnt(prHw, prThis->u4Idx, (u4Amount + 48) /48);
        }
        
        if (ASRC_OBUF_AMOUNT_INT & prIsrCb->u4IntType) {
            u32 u4Amount = prThis->rBuf.u4OChSize >> 1;
            prHw->SetOBufIntrCnt(prHw, prThis->u4Idx, (u4Amount + 48) /48);
        }
        
        if(ASRC_OBUF_OV_INT & prIsrCb->u4IntType) {
            prHw->SetMaxOutPerIn(prHw, prThis->u4Idx, 0);
        } else {
            PASRC_CHS_FMT_T prFmt = &(prThis->rFmt);
            prHw->SetMaxOutPerIn(prHw, prThis->u4Idx, (prFmt->u4OFS + prFmt->u4IFS - 1) / prFmt->u4IFS);
        }

        prHw->INTEnable(prHw, prThis->u4Idx, prIsrCb->u4IntType);
    }
    else    // Disable interupt
    {        
        prHw->INTEnable(prHw, prThis->u4Idx, ASRC_ALL_INT);
        prThis->rIsrCb.pfnCb = NULL;
    }
    
    return (u4Ret);
}


//=====================================//
  #define CodeSight_AsrcHal_Chs_Buf
//=====================================//

static u32 AsrcChs_SetIWP(void * pThis, u32 u4WP)
{
    PASRC_CHS_CLS prThis = (PASRC_CHS_CLS)pThis;
    PASRC_HW_CLS_PUB prHw = prThis->prMgr->prHw;

    prHw->SetIWP(prHw, prThis->u4Idx, (prThis->rBuf.u4ISAdr + u4WP));
    prHw->INTEnable(prHw, prThis->u4Idx, ASRC_IBUF_EMPTY_INT);

    return (0);
}


static u32 AsrcChs_GetIRP(void * pThis)
{
    PASRC_CHS_CLS prThis = (PASRC_CHS_CLS)pThis;
    PASRC_HW_CLS_PUB prHw = prThis->prMgr->prHw;
    
    return (prHw->GetIRP(prHw, prThis->u4Idx) - prThis->rBuf.u4ISAdr);
}


static u32 AsrcChs_SetORP(void * pThis, u32 u4RP)
{
    PASRC_CHS_CLS prThis = (PASRC_CHS_CLS)pThis;
    PASRC_HW_CLS_PUB prHw = prThis->prMgr->prHw;
    
    prHw->SetORP(prHw, prThis->u4Idx, prThis->rBuf.u4OSAdr + u4RP);
    prThis->rBuf.u4State &= ~ASRC_BUF_ST_OBUF_FULL;
    prHw->INTEnable(prHw, prThis->u4Idx, ASRC_OBUF_OV_INT);

    return (0);
}


static u32 AsrcChs_GetOWP(void * pThis)
{
    PASRC_CHS_CLS prThis = (PASRC_CHS_CLS)pThis;
    PASRC_HW_CLS_PUB prHw = prThis->prMgr->prHw;

    return (prHw->GetOWP(prHw, prThis->u4Idx) - prThis->rBuf.u4OSAdr);
}


static u32 AsrcChs_GetIBuf(void * pThis, AUD_DATA_BUF_T *prBuf)
{
    PASRC_CHS_CLS prThis = (PASRC_CHS_CLS)pThis;
    
    prBuf->u4VirSAdr = prThis->rBuf.u4IVirSAdr[0];
    prBuf->u4ChBufSz = prThis->rBuf.u4IChSize;
    prBuf->u4Chn = prThis->rFmt.u4Chn;
   
    return (0);
}


static u32 AsrcChs_GetOBuf(void * pThis, AUD_DATA_BUF_T *prBuf)
{
    PASRC_CHS_CLS prThis = (PASRC_CHS_CLS)pThis;
    
    prBuf->u4VirSAdr = prThis->rBuf.u4OVirSAdr[0];
    prBuf->u4ChBufSz = prThis->rBuf.u4OChSize;
    prBuf->u4Chn = prThis->rFmt.u4Chn;  
    
    return (AUD_RET_OK);
}


static u32 AsrcChs_GetBufState(void * pThis)
{
    PASRC_CHS_CLS prThis = (PASRC_CHS_CLS)pThis;
    
    return (prThis->rBuf.u4State);
}


//===================================//
  #define CodeSight_AsrcHal_Chs_Main
//===================================//

static u32 AsrcChs_GetIdx(void * pThis)
{
    PASRC_CHS_CLS prThis = (PASRC_CHS_CLS)pThis;
    
    return (prThis->u4Idx);
}

static u32 AsrcChs_Setup(void * pThis, void * pCfg)
{
    PASRC_CHS_CLS prThis = (PASRC_CHS_CLS)pThis;
    PASRC_CHS_CFG_T prCfg = (PASRC_CHS_CFG_T)pCfg;
    
    PASRC_HW_CLS_PUB prHw = prThis->prMgr->prHw;
   
    AsrcChs_FmtInit(prThis, prCfg->prFmt);
    AsrcChs_BufferInit(prThis);

    prHw->INTDisable(prHw, prThis->u4Idx, ASRC_ALL_INT);
    AudOS_IRQ_Enable(GET_ASRC_VECTOR(prThis->prMgr->eType));
    AsrcChs_RegISRCB(prThis, prCfg->prIsrCb);
    prHw->ChSetClear(prHw, prThis->u4Idx);

    prThis->u4State = AUD_STATE_STOPPED;  
    
    return (prThis->u4State);
}


static u32 AsrcChs_GetStatus(void * pThis)
{
    PASRC_CHS_CLS prThis = (PASRC_CHS_CLS)pThis;
    
    return (prThis->u4State);
}


static u32 AsrcChs_Start(void * pThis, u32 u4Param)
{
    PASRC_CHS_CLS prThis = (PASRC_CHS_CLS)pThis;
    PASRC_HW_CLS_PUB prHw = prThis->prMgr->prHw;
    PASRC_MGR_CLS_PUB prMgr = &prThis->prMgr->rPub;
    PASRC_CHS_BUF_T prBuf = &(prThis->rBuf);
    
    ASRCLOG_DBG(T("[Chs(%d - %d)]Start: State(%d). \r\n"), 
        prThis->prMgr->eType, (s32)(prThis->u4Idx), (s32)(prThis->u4State)); 
    
    if (AUD_STATE_STOPPED == prThis->u4State)
    {
        prBuf->u4State = ASRC_BUF_ST_IBUF_EMPTY; 
    
        prHw->SetIRP(prHw, prThis->u4Idx, prBuf->u4ISAdr);
        prHw->SetIWP(prHw, prThis->u4Idx, prBuf->u4ISAdr);
        prHw->SetORP(prHw, prThis->u4Idx, prBuf->u4OSAdr);
        prHw->SetOWP(prHw, prThis->u4Idx, prBuf->u4OSAdr);

        prHw->ChSetClear(prHw, prThis->u4Idx);
        prHw->ChSetEnable(prHw, prThis->u4Idx, TRUE);
        prMgr->Start(prMgr);
        prThis->u4State = AUD_STATE_STARTED;
    }

    return (prThis->u4State);
}


static u32 AsrcChs_Stop(void * pThis, u32 u4Param)
{
    PASRC_CHS_CLS prThis = (PASRC_CHS_CLS)pThis;
    PASRC_HW_CLS_PUB prHw = prThis->prMgr->prHw;
    PASRC_MGR_CLS_PUB prMgr = &prThis->prMgr->rPub;
    
    ASRCLOG_DBG(T("[Chs(%d - %d)]Stop, State(%d). \r\n"), 
        prThis->prMgr->eType, (s32)(prThis->u4Idx), (s32)(prThis->u4State)); 
    
    if (AUD_STATE_STARTED == prThis->u4State)
    {
        prHw->ChSetEnable(prHw, prThis->u4Idx, FALSE);
        prMgr->Stop(prMgr);
        prThis->u4State = AUD_STATE_STOPPED;
    }

    return (prThis->u4State);
}


//========================================//
  #define CodeSight_AudHalAsrc_Chs_DEBUG
//========================================//
static void AsrcChs_LogAttribute(void * pThis)
{
    PASRC_CHS_CLS prThis = (PASRC_CHS_CLS)pThis;
    PASRC_CHS_FMT_T prFmt = &(prThis->rFmt);
    PASRC_CHS_BUF_T prBuf = &(prThis->rBuf);
    
    ASRCLOG_CLI(T("[ASRC(%d)] State(%d) PFN(0x%x) ISRParam(0x%x)\r\n"), 
        (s32)(prThis->u4Idx),  (s32)(prThis->u4State), (u32)(prThis->rIsrCb.pfnCb), (u32)(prThis->rIsrCb.u4Param));
    
    ASRCLOG_CLI(T("         Fmt: chn(%d) FS(%d, %d) BW(%d, %d) eFS(%d, %d)\r\n"),
        (s32)(prFmt->u4Chn), 
        (s32)(prFmt->u4IFS), (s32)(prFmt->u4OFS), 
        (s32)(prFmt->u4IBW), (s32)(prFmt->u4OBW), 
        (s32)(prFmt->u4IPalette), (s32)(prFmt->u4OPalette)); 
    
    ASRCLOG_INFO(T("         ChBuf: SAdr(0x%08x, 0x%08x) IVir(0x%08x, 0x%08x) OVir(0x%08x, 0x%08x)..\r\n"), 
        (s32)(prBuf->u4ISAdr), (u32)(prBuf->u4OSAdr), (u32)(prBuf->u4IVirSAdr[0]), 
        (u32)(prBuf->u4IVirSAdr[1]), (u32)(prBuf->u4OVirSAdr[0]), (u32)(prBuf->u4OVirSAdr[1]));
}


//========================================//
  #define CodeSight_AudHalAsrc_Chs_Create
//========================================//

static u32 AsrcChs_Delete(void * pThis)
{
    PASRC_CHS_CLS prThis = (PASRC_CHS_CLS)pThis;
    PASRC_MGR_CLS prMgr = prThis->prMgr; 
    PASRC_HW_CLS_PUB prHw = prThis->prMgr->prHw;
  
    prHw->INTDisable(prHw, prThis->u4Idx, ASRC_ALL_INT);
    
    prMgr->rPub.FreePalette(prMgr, prThis->rFmt.u4IPalette, prThis->rFmt.u4OPalette);
    
    prThis->u4State = AUD_STATE_UNINIT;

    _aprAsrcChs[prMgr->eType][prThis->u4Idx] = NULL;
    AUD_CLASS_DELETE();
      
    return (0);
}


static void AsrcChs_MemborInit(PASRC_CHS_CLS prThis, ASRC_CLS_TYPE eType, u32 u4Idx)
{  
    PASRC_MGR_CLS prMgr = (PASRC_MGR_CLS)AsrcMgr_Get(eType);
    PASRC_CHS_BUF_T prBuf = &prThis->rBuf;
    PASRC_MEMORY_T prMemory = &prMgr->rMemory;
       
    prThis->prMgr = prMgr;
    prThis->u4Idx = u4Idx;

    prThis->rIsrCb.pfnCb = NULL;
    prThis->rIsrCb.u4Param = 0;
    x_memset(&(prThis->rFmt), 0x00, sizeof(ASRC_CHS_FMT_T));

    prBuf->u4IChSize =     prMemory->u4IChSz;
    prBuf->u4OChSize =     prMemory->u4OChSz;
    prBuf->u4ISAdr =       (prMemory->u4IPhyAddr & 0xFFFFF) + (prMemory->u4IChSz * 2 * u4Idx);     
    prBuf->u4IVirSAdr[0] = (prMemory->u4IVirAddr) + (prMemory->u4IChSz * 2 * u4Idx);    
    prBuf->u4OSAdr =       (prMemory->u4OPhyAddr & 0xFFFFF) + (prMemory->u4OChSz * 2 * u4Idx);    
    prBuf->u4OVirSAdr[0] = (prMemory->u4OVirAddr) + (prMemory->u4OChSz * 2 * u4Idx); 

    prThis->u4State = AUD_STATE_INITED;  
}


static void AsrcChs_FuncPointInit(PASRC_CHS_CLS prThis)
{
    PASRC_CHS_CLS_PUB prPub = &prThis->rPub;
    
    prPub->rHwIf.Setup = AsrcChs_Setup;
    prPub->rHwIf.GetStatus= AsrcChs_GetStatus;
    
    prPub->rHwIf.Start = AsrcChs_Start;
    prPub->rHwIf.Stop = AsrcChs_Stop;

    prPub->rHwIf.GetIBuf = AsrcChs_GetIBuf;
    prPub->rHwIf.SetIWP = AsrcChs_SetIWP;
    prPub->rHwIf.GetIRP = AsrcChs_GetIRP;

    prPub->rHwIf.GetOBuf = AsrcChs_GetOBuf;
    prPub->rHwIf.SetORP = AsrcChs_SetORP;
    prPub->rHwIf.GetOWP = AsrcChs_GetOWP;

    prPub->GetIdx = AsrcChs_GetIdx;
    
    prPub->GetBufState = AsrcChs_GetBufState;

    prPub->LogAttribute = AsrcChs_LogAttribute;
  
    prPub->Delete = AsrcChs_Delete;
}


PASRC_CHS_CLS_PUB AsrcChs_New(ASRC_CLS_TYPE eType, u32 u4Idx)
{
    PASRC_CHS_CLS prThis = AUD_CLASS_NEW(ASRC_CHS_CLS); 

    if (prThis)
    {
        _aprAsrcChs[eType][u4Idx] = prThis;
        
        AsrcChs_MemborInit(prThis, eType, u4Idx);
        AsrcChs_FuncPointInit(prThis);    

        ASRCLOG_DBG(T("[Chs(%d-%d)] New: 0x%x \n"), eType, (s32)u4Idx, (u32)prThis);
    }
    
    return ((PASRC_CHS_CLS_PUB)prThis);
}


PASRC_CHS_CLS_PUB AsrcChs_Get(ASRC_CLS_TYPE eType, u32 u4Idx)
{ 
    return ((PASRC_CHS_CLS_PUB)_aprAsrcChs[eType][u4Idx]);
}

