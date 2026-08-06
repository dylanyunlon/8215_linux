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
#include <linux/interrupt.h>


PASRC_MGR_CLS _aprAsrcMgr[ASRC_TYPE_NUM]; 


static irqreturn_t AsrcHal_ISR(u32 u2Vector, void* tmp)
{   
    ASRC_CLS_TYPE eType = GET_ASRC_TYPE(u2Vector);
    PASRC_MGR_CLS prMgr = _aprAsrcMgr[eType];
          
    if (prMgr)
    {
        u32 u4Idx;
        PASRC_HW_CLS_PUB prHw = prMgr->prHw;
        prHw->ClearIFR(prHw, FALSE);
        
        for (u4Idx = 0; u4Idx < ASRC_CHSET_NUM; u4Idx++)
        {
            PASRC_CHS_CLS prChs = (PASRC_CHS_CLS)AsrcChs_Get(eType, u4Idx);
            u32 u4IntType = 0;

            if (!prChs) {
                continue;
            }
            
            if (prHw->GetIBufEmptyFlag(prHw, u4Idx))  {
                u4IntType |= ASRC_IBUF_EMPTY_INT;
                prChs->rBuf.u4State = ASRC_BUF_ST_IBUF_EMPTY;      
            }            
            if (prHw->GetIBufAmountFlag(prHw, u4Idx))  {
                u4IntType |= ASRC_IBUF_AMOUNT_INT;
            }   
            if (prHw->GetOBufOvFlag(prHw, u4Idx))  {
                u4IntType |= ASRC_OBUF_OV_INT;
                prChs->rBuf.u4State = ASRC_BUF_ST_OBUF_FULL; 
            }   
            if (prHw->GetOBufAmountFlag(prHw, u4Idx)) {
                u4IntType |= ASRC_OBUF_AMOUNT_INT;
            } 
            
        #if (!ASRC_IC_VFY) 
            prHw->INTDisable(prHw, u4Idx, u4IntType);
        #endif
            if (prChs->rIsrCb.pfnCb && u4IntType) 
            {
                 prChs->rIsrCb.pfnCb(prChs->rIsrCb.u4Param, u4IntType);
            }
        }
        
        prHw->ClearIFR(prHw, TRUE);  
        AudOS_IRQ_Clear(prMgr->u4Vect);
    }   
	return IRQ_HANDLED;
}


//========================================// 
  #define CodeSight_AudHalAsrc_Mgr_Buffer
//========================================//

static void AsrcMgr_AllocMemory(PASRC_MGR_CLS prThis, u32 u4IChSz, u32 u4OChSz)
{
    PASRC_MEMORY_T prMemory = &(prThis->rMemory);
    u32 u4memSz, u4Alignment = 0x40000;

    prMemory->u4IChSz = AUD_ALIGNMENT_MASK(u4IChSz, 48);
    prMemory->u4OChSz = AUD_ALIGNMENT_MASK(u4OChSz, 48); 
    prMemory->u4ISz = prMemory->u4IChSz * (ASRC_CHSET_NUM << 1); 
    prMemory->u4OSz = prMemory->u4OChSz * (ASRC_CHSET_NUM << 1); 
    
    u4memSz = prMemory->u4ISz + prMemory->u4OSz;
    while (u4Alignment < u4memSz) {
        u4Alignment <<= 1;      
    }
    #ifndef __linux__
    prMemory->u4IVirAddr = AudOS_Memory_Alloc(u4memSz, u4Alignment, &prMemory->u4IPhyAddr);    
    #else
    prMemory->u4IVirAddr = AC83XX_PCM_ASRC_VA;
    prMemory->u4IPhyAddr = AC83XX_PCM_ASRC_PA;
	//prMemory->u4IVirAddr = AudOS_Memory_Alloc(u4memSz, u4Alignment, &prMemory->u4IPhyAddr); 
    #endif
    prMemory->u4OVirAddr = prMemory->u4IVirAddr + prMemory->u4ISz;  
    prMemory->u4OPhyAddr = prMemory->u4IPhyAddr + prMemory->u4ISz;  
    
    ASRCLOG_INFO((T("[%d]Alloc Buffer AudOS_Memory_Alloc: QKnote In(0x%lx, 0x%lx, 0x%x, 0x%x) Out(0x%lx, 0x%lx, 0x%x, 0x%x) Align(0x%x)\n"),
        prThis->eType, (uintptr_t)(prMemory->u4IVirAddr), (uintptr_t)(prMemory->u4IPhyAddr), (u32)(prMemory->u4ISz), (u32)(prMemory->u4IChSz),
        (uintptr_t)(prMemory->u4OVirAddr), (uintptr_t)(prMemory->u4OPhyAddr), (u32)(prMemory->u4OSz), (u32)(prMemory->u4OChSz), (u32)u4Alignment));
    
    prThis->prHw->SetIBufAddr(prThis->prHw, prThis->rMemory.u4IPhyAddr, prThis->rMemory.u4IChSz);
    prThis->prHw->SetOBufAddr(prThis->prHw, prThis->rMemory.u4OPhyAddr, prThis->rMemory.u4OChSz);
}


static void AsrcMgr_FreeMemory(PASRC_MGR_CLS prThis)
{
    if (prThis->rMemory.u4IVirAddr)
    {
        #ifndef __linux__
        AudOS_Memory_Free(&prThis->rMemory.u4IVirAddr);
        #endif
    }
}


//=========================================//
  #define CodeSight_AudHalAsrc_Mgr_Palette
//=========================================//

static void AsrcMgr_InitPalette(PASRC_MGR_CLS prThis)
{
    u32 i;
    for (i = 0; i < ASRC_PALETTE_NUM; i++)
    {
        prThis->arPaletteCfg[i].u4FS = 0;
        prThis->arPaletteCfg[i].u4Ref = 0;
        prThis->arPaletteCfg[i].fgFixUse = FALSE;
    }
}


u32 AsrcMgr_AllocPalette(void * pThis, u32 u4FS)
{
    PASRC_MGR_CLS prThis = (PASRC_MGR_CLS)pThis;
    u32 i, u4PaletteIdx = ASRC_PALETTE_NUM;
    
    //First: Find Used Palette
    for (i = 0; i < ASRC_PALETTE_NUM; i++)
    {
        if (prThis->arPaletteCfg[i].u4Ref &&
            (prThis->arPaletteCfg[i].u4FS == u4FS) && 
            !prThis->arPaletteCfg[i].fgFixUse)
        {
            prThis->arPaletteCfg[i].u4Ref ++;
            u4PaletteIdx = i;
            break;
        }
    }
    
    //Second: if not find Used Palette, try to find UnUsed Palette!
    if(u4PaletteIdx == ASRC_PALETTE_NUM)
    {
        for (i = 0; i < ASRC_PALETTE_NUM; i++)
        {
            if (!prThis->arPaletteCfg[i].u4Ref && 
                !prThis->arPaletteCfg[i].fgFixUse)
            {
                u4PaletteIdx = i;
                prThis->arPaletteCfg[i].u4FS =  u4FS;
                prThis->arPaletteCfg[i].u4Ref ++;
                prThis->prHw->SetPaletteFS(prThis->prHw, i, u4FS);
                ASRCLOG_DBG((T("[Mgr]Set PaletteFreq(%d:  %d) \n"), (s32)i, (s32)u4FS));                
                break;
            }
        }
    }
    
    return (u4PaletteIdx);
}


u32 AsrcMgr_FreePalette(void * pThis, u32 u4IPalette, u32 u4OPalette)
{
    u32 u4Ret = AUD_RET_OK;
    PASRC_MGR_CLS prThis = (PASRC_MGR_CLS)pThis;
    
    if (u4IPalette < ASRC_PALETTE_NUM && 
        prThis->arPaletteCfg[u4IPalette].u4Ref &&
        !prThis->arPaletteCfg[u4IPalette].fgFixUse)
    {
       prThis->arPaletteCfg[u4IPalette].u4Ref--;
    }

    if (u4OPalette < ASRC_PALETTE_NUM && 
        prThis->arPaletteCfg[u4OPalette].u4Ref)
    {
       prThis->arPaletteCfg[u4OPalette].u4Ref--;
    }
    
    return (u4Ret);
}


static u32 AsrcMgr_SetFixPalette(void * pThis, u32 u4FS)
{
    s32 i = 0;
    u32 u4Idx = ASRC_PALETTE_NUM;

    PASRC_MGR_CLS prThis = (PASRC_MGR_CLS)pThis;

    for (i = ASRC_PALETTE_NUM - 1; i >= 0; i--)
    {
        if (!prThis->arPaletteCfg[i].fgFixUse) 
        {
            prThis->arPaletteCfg[i].fgFixUse = TRUE;        
            prThis->arPaletteCfg[i].u4Ref = 1;
            prThis->arPaletteCfg[i].u4FS = u4FS;

            prThis->prHw->SetPaletteFS(prThis->prHw, i, u4FS);
            u4Idx = i;
            break;
        }
    }

    ASRCLOG_INFO((T("[Mgr(%d)]SetFixPalette: Idx(%d) FS(%d)\n"), prThis->eType, (s32)u4Idx, (s32)u4FS));

    return (u4Idx);
}

static u32 AsrcMgr_ModifyFixPalette(void * pThis, u32 u4FS)
{
    s32 i = 0;
    u32 u4Idx = ASRC_PALETTE_NUM;

    PASRC_MGR_CLS prThis = (PASRC_MGR_CLS)pThis;

    for (i = ASRC_PALETTE_NUM - 1; i >= 0; i--)
    {
        if (prThis->arPaletteCfg[i].fgFixUse) 
        {
            prThis->arPaletteCfg[i].u4FS = u4FS;

            prThis->prHw->SetPaletteFS(prThis->prHw, i, u4FS);
            u4Idx = i;
            break;
        }
    }

    ASRCLOG_INFO((T("[Mgr(%d)]ModifyFixPalette: Idx(%d) FS(%d)\n"), prThis->eType, (s32)u4Idx, (s32)u4FS));

    return (u4Idx);
}

static u32 AsrcMgr_ClrFixPalette(void * pThis, u32 u4Idx)
{
    PASRC_MGR_CLS prThis = (PASRC_MGR_CLS)pThis;
    
    prThis->arPaletteCfg[u4Idx].fgFixUse = FALSE;
    prThis->arPaletteCfg[u4Idx].u4Ref = 0;

    ASRCLOG_INFO((T("[Mgr(%d)]ClrFixPalette: Idx(0x%x)\n"), prThis->eType, (u32)u4Idx));

    return (ASRC_PALETTE_NUM);
}

//===========================================//
  #define CodeSight_AudHalAsrc_Mgr_AllocChs
//===========================================//

void AsrcMgr_InitAsrc(PASRC_MGR_CLS prThis)
{
    u32 i = 0;
    
    for (i = 0; i < ASRC_CHSET_NUM; i++)
    {
       prThis->afgAsrcFixUse[i] = FALSE;
    }  
}


PASRC_CHS_CLS_PUB AsrcMgr_AllocAsrc(void * pThis, bool fgUseFixAsrc)
{
    PASRC_MGR_CLS prThis = (PASRC_MGR_CLS)pThis;
    PASRC_CHS_CLS_PUB prChs = NULL;  
    u32 i;

    for (i = 0; i < ASRC_CHSET_NUM; i++)
    {           
        if (!AsrcChs_Get(prThis->eType, i) && 
            !(fgUseFixAsrc ^ prThis->afgAsrcFixUse[i])) 
        {
            prChs = AsrcChs_New(prThis->eType, i);
            ASRCLOG_DBG((T("[Mgr(%d)]Alloc ASRC: %d -> 0x%x \r\n"), prThis->eType, (s32)i, (u32)prChs));
            break;
        }
    } 

    if (!prChs)
    {
        ASRCLOG_ERR((T("[Mgr(%d)]No free ASRC! \r\n"), prThis->eType));
    }
    
    return (prChs);
}


static bool AsrcMgr_SetFixAsrc(void * pThis)
{
    bool fgRet = FALSE;
    s32 i;

    PASRC_MGR_CLS prThis = (PASRC_MGR_CLS)pThis;

    for (i = ASRC_CHSET_NUM - 1; i >= 0; i--)
    {
        if (!prThis->afgAsrcFixUse[i]) 
        {
            prThis->afgAsrcFixUse[i] = TRUE;  
            fgRet = TRUE;
            break;
        }
    }

    ASRCLOG_INFO((T("[Mgr(%d)]SetFixChSet: Idx(%d)\n"), prThis->eType, (s32)i));
    
    return (fgRet);
}


static bool AsrcMgr_ClrFixAsrc(void * pThis)
{
    bool fgRet = FALSE;
    s32 i;

    PASRC_MGR_CLS prThis = (PASRC_MGR_CLS)pThis;
    
    for (i = ASRC_CHSET_NUM - 1; i >= 0; i--)
    {
        if (prThis->afgAsrcFixUse[i]) 
        {
            prThis->afgAsrcFixUse[i] = FALSE; 
            fgRet = TRUE;
            break;
        }
    }

    ASRCLOG_INFO((T("[Mgr(%d)]ClrFixChSet: Idx(0x%x)\n"), prThis->eType, (u32)i));

    return (ASRC_CHSET_NUM);
}

//======================================//
  #define CodeSight_AudHalAsrc_Mgr_Main
//======================================//
void AsrcMgr_HibernationCtrl(void * pThis, bool fgWaveUp)
{
    PASRC_MGR_CLS prThis = (PASRC_MGR_CLS)pThis;
    
    PASRC_HW_CLS_PUB prHw = prThis->prHw;
    u32 i;
    ASRCLOG_DBG((T("[Mgr]HibernationCtrl: fgWaveUp(%d)\n"), (s32)fgWaveUp));
    
    if (fgWaveUp)
    {        
        prHw->Reset(prHw);
        prHw->SetIBufAddr(prHw, prThis->rMemory.u4IPhyAddr, prThis->rMemory.u4IChSz);
        prHw->SetOBufAddr(prHw, prThis->rMemory.u4OPhyAddr, prThis->rMemory.u4OChSz);

        for (i = 0; i < ASRC_PALETTE_NUM; i++) {   
            if (prThis->arPaletteCfg[i].fgFixUse) {
                prHw->SetPaletteFS(prHw, i, prThis->arPaletteCfg[i].u4FS);
            }
        }
    }
    else
    {
        PASRC_CHS_CLS_PUB prChs = NULL;     
        for (i = 0; i < ASRC_CHSET_NUM; i++) {
            prChs = AsrcChs_Get(prThis->eType, i);
            if (prChs) {
                prChs->rHwIf.Stop(prChs, 0);
            }
        }
    }
}


u32 AsrcMgr_Start(void * pThis)
{ 
    PASRC_MGR_CLS prThis = (PASRC_MGR_CLS)pThis;
    
    ASRCLOG_DBG((T("[Mgr(%d)]Enable: State(%d) StartNum(%d) \n"), 
        prThis->eType, (s32)(prThis->u4State), (s32)(prThis->u4StartNum)));

    if (!prThis->u4StartNum && AUD_STATE_STOPPED == prThis->u4State)
    {
    #if ASRC_AUTO_TRACING 
        if(prThis->u4AutoTracingClk) {
            prThis->prHw->AutoTraceEnable(prThis->prHw, TRUE, prThis->u4AutoTracingClk);
        }
    #endif

        AudOS_IRQ_Enable(prThis->u4Vect);
        prThis->prHw->Enable(prThis->prHw, TRUE);
        prThis->u4State = AUD_STATE_STARTED;
    }
    prThis->u4StartNum++;
    
    return (prThis->u4State);
}


u32 AsrcMgr_Stop(void * pThis)
{
    PASRC_MGR_CLS prThis = (PASRC_MGR_CLS)pThis;
    
    ASRCLOG_DBG((T("[Mgr(%d)]Disable: State(%d) StartNum(%d) \n"), 
        prThis->eType, (s32)(prThis->u4State), (s32)(prThis->u4StartNum)));

    if (prThis->u4StartNum)
    {
        prThis->u4StartNum--;
        if (!prThis->u4StartNum && AUD_STATE_STARTED == prThis->u4State)
        {
        #if ASRC_AUTO_TRACING 
            if(prThis->u4AutoTracingClk) {
                prThis->prHw->AutoTraceEnable(prThis->prHw, FALSE, prThis->u4AutoTracingClk);
            }
        #endif

            AudOS_IRQ_Disable(prThis->u4Vect);
            prThis->prHw->Enable(prThis->prHw, FALSE);
            prThis->u4State = AUD_STATE_STOPPED;
        }
    }
    
    return (prThis->u4State);
}


#if (ASRC_AUTO_TRACING)
void AsrcMgr_SetAutoTracing(void * pThis, u32 u4TracingClk)
{
    PASRC_MGR_CLS prThis = (PASRC_MGR_CLS)pThis;
    
    prThis->u4AutoTracingClk = u4TracingClk;
    ASRCLOG_DBG((T("[AutoTracing] Clk(%d)\r\n"),  prThis->u4AutoTracingClk));
}


u32 AsrcMgr_GetAutoTracing(void * pThis)
{
    PASRC_MGR_CLS prThis = (PASRC_MGR_CLS)pThis;
    
    return (prThis->u4AutoTracingClk);
}
#endif  //ASRC_AUTO_TRACING


//========================================//
  #define CodeSight_AsrcMgr_DbgInfo
//========================================//

void AsrcMgr_LogAttribute(void * pThis)
{
    PASRC_MGR_CLS prThis = (PASRC_MGR_CLS)pThis;
    
    u32 i;
    PASRC_MEMORY_T prMemory = &(prThis->rMemory);
    
    ASRCLOG_CLI((T("[AsrcMgr]>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n")));
    ASRCLOG_CLI((T("         State(%d) StartNum(%d) eType(%d) Vect(%d) \r\n"), 
        (s32)(prThis->u4State), (s32)(prThis->u4StartNum), prThis->eType, (s32)(prThis->u4Vect)));

    ASRCLOG_CLI((T("   Palette(Idx: Ref, FS, FixUse): \r\n")));
    for (i = 0; i < ASRC_PALETTE_NUM; i++)
    {
        PASRC_PALETTE_CFG_T prCfg = &prThis->arPaletteCfg[i];
        ASRCLOG_CLI((T("         (%d: %d, %d, %d)  \n"), (s32)i, (s32)(prCfg->u4Ref), (s32)(prCfg->u4FS), (s32)(prCfg->fgFixUse)));
    }

    ASRCLOG_CLI((T("    ChSet(Idx: FixUse): \r\n")));
    for (i = 0; i < ASRC_CHSET_NUM; i++)
    {
        ASRCLOG_CLI((T("         (%d: 0x%x)  \n"), (s32)i, (u32)(prThis->afgAsrcFixUse[i])));
    } 
 
    ASRCLOG_INFO((T("         InBuf:  Phy(0x%08x) Vir(0x%08x) Sz(0x%08x) ChSz(0x%08x).\r\n"), 
        (u32)(prMemory->u4IPhyAddr), (u32)(prMemory->u4IVirAddr), (u32)(prMemory->u4ISz), (u32)(prMemory->u4IChSz)));
    ASRCLOG_INFO((T("         OutBuf: Phy(0x%08x) Vir(0x%08x) Sz(0x%08x) ChSz(0x%08x).\r\n"), 
        (u32)(prMemory->u4OPhyAddr), (u32)(prMemory->u4OVirAddr), (u32)(prMemory->u4OSz), (u32)(prMemory->u4OChSz)));

    ASRCLOG_INFO((T("[AsrcMgr]<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n")));
}
 
 
void AsrcMgr_LogChsAttribute(void * pThis, u32 u4Idx)
{
    PASRC_MGR_CLS prThis = (PASRC_MGR_CLS)pThis;
    PASRC_CHS_CLS_PUB prChs = AsrcChs_Get(prThis->eType, u4Idx);
    if (prChs) {
        prChs->LogAttribute(prChs);
    }
}


void AsrcMgr_LogAllRegs(void * pThis)
{
    PASRC_MGR_CLS prThis = (PASRC_MGR_CLS)pThis;
    
    PASRC_HW_CLS_PUB prHw = prThis->prHw;
    prHw->LogAllRegs(prHw);
}


//========================================//
  #define CodeSight_AudHalAsrc_Mgr_Create
//========================================//
u32 AsrcMgr_Delete(void * pThis)
{
    PASRC_MGR_CLS prThis = (PASRC_MGR_CLS)pThis;
    
    AudOS_IRQ_Disable(prThis->u4Vect); 
    AsrcMgr_FreeMemory(prThis);  
    AudOS_ISR_UnReg(prThis->u4Vect); 
    prThis->u4State = AUD_STATE_UNINIT;    

    _aprAsrcMgr[prThis->eType] = NULL;
    AUD_CLASS_DELETE();
    return (0);
}


static void AsrcMgr_MemberInit(PASRC_MGR_CLS prThis, ASRC_CLS_TYPE eType, u32 u4IChSz, u32 u4OChSz)
{
    prThis->eType = eType;
    prThis->u4Vect = GET_ASRC_VECTOR(eType);
    
    prThis->u4State = AUD_STATE_UNINIT;
    prThis->u4StartNum = 0;
    
    prThis->prHw = AsrcHw_New(eType);
    prThis->prHw->Reset(prThis->prHw);
    
    AudOS_IRQ_Disable(prThis->u4Vect);   
    if (!AudOS_ISR_Reg(prThis->u4Vect, AsrcHal_ISR))
    {                     
        ASRCLOG_ERR((T("ISR_REG Fail! \n")));
    } 
    
    AsrcMgr_InitAsrc(prThis);
    AsrcMgr_InitPalette(prThis);         
    AsrcMgr_AllocMemory(prThis, u4IChSz, u4OChSz);  

#if (ASRC_AUTO_TRACING)
    prThis->u4AutoTracingClk = 0;
#endif    
}


static void AsrcMgr_FunctionInit(PASRC_MGR_CLS prThis)
{
    PASRC_MGR_CLS_PUB prPub = &prThis->rPub;

    prPub->AllocPalette  = AsrcMgr_AllocPalette;   
    prPub->FreePalette   = AsrcMgr_FreePalette;  
    prPub->SetFixPalette = AsrcMgr_SetFixPalette;
    prPub->ModifyFixPalette = AsrcMgr_ModifyFixPalette;
    prPub->ClrFixPalette = AsrcMgr_ClrFixPalette;

    prPub->AllocAsrc  = AsrcMgr_AllocAsrc;  
    prPub->SetFixAsrc = AsrcMgr_SetFixAsrc;  
    prPub->ClrFixAsrc = AsrcMgr_ClrFixAsrc;

    prPub->HibernationCtrl = AsrcMgr_HibernationCtrl;
    prPub->Start = AsrcMgr_Start;
    prPub->Stop  = AsrcMgr_Stop;

#if (ASRC_AUTO_TRACING)
    prPub->SetAutoTracing = AsrcMgr_SetAutoTracing;
    prPub->GetAutoTracing = AsrcMgr_GetAutoTracing;
#endif  

    prPub->LogAttribute = AsrcMgr_LogAttribute;
    prPub->LogChsAttribute = AsrcMgr_LogChsAttribute;
    prPub->LogAllRegs = AsrcMgr_LogAllRegs; 

    prPub->Delete = AsrcMgr_Delete;
}


PASRC_MGR_CLS_PUB AsrcMgr_New(ASRC_CLS_TYPE eType, u32 u4IChSz, u32 u4OChSz)
{
    PASRC_MGR_CLS prThis = AUD_CLASS_NEW(ASRC_MGR_CLS);
    
    if (prThis) 
    {
        _aprAsrcMgr[eType] = prThis;
        
        AsrcMgr_MemberInit(prThis, eType, u4IChSz, u4OChSz);
        AsrcMgr_FunctionInit(prThis);

        prThis->u4State = AUD_STATE_STOPPED; 
        
        ASRCLOG_INFO((T("[Mgr](0x%x) New: eType(%d) Vect(%d). \r\n"), (u32)prThis, eType, (s32)(prThis->u4Vect)));
    }

    return ((PASRC_MGR_CLS_PUB)prThis);
}


PASRC_MGR_CLS_PUB AsrcMgr_Get(ASRC_CLS_TYPE eType)
{
    return ((PASRC_MGR_CLS_PUB)_aprAsrcMgr[eType]);
}



