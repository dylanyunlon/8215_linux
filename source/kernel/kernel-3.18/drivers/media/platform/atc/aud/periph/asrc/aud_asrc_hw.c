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
*[File]                 aud_asrc_hw.c
*[Author]               tongfa.luo@autochips.com
*[Description]
*       aud reg asrc contrl
*[Copyright]
*       
******************************************************************************/

#include "aud_asrc_hw.h"

typedef struct
{   
    volatile u32 GenConf[2];                  
    volatile u32 IER[2];                      
    volatile u32 IFR[2];                      
    volatile u32 ChCnfg[6];
    volatile u32 Fs[8]; 
    
    volatile u32 IBufSAdr;
    volatile u32 IBufSize;
    volatile u32 OBufSAdr;
    volatile u32 OBufSize;
    
    volatile u32 IBufRp[6];     
    volatile u32 IBufWp[6];     
    volatile u32 OBufWp[6];     
    volatile u32 OBufRp[6];  
    
    volatile u32 IBufIntrCnt[3];              
    volatile u32 OBufIntrCnt[3]; 
    
    volatile u32 BAK;
    volatile u32 FreqTransNum;
    volatile u32 MaxOutPerIn[2]; 
    
    volatile u32 FreqCaliCtrl[2];                       
    volatile u32 PrdCaliRslt[2];
    volatile u32 FreqCaliRslt[2];
    
    volatile u32 IIRCramAddr;
    volatile u32 IIRCramData;
    
    volatile u32 DmaCfg;
    
    volatile u32 IFsOFsSel;
    
}ASRC_REG_MAP_T, *PASRC_REG_MAP_T;


typedef struct
{
    ASRC_HW_CLS_PUB rPub;
    
    ASRC_CLS_TYPE eType;
    ASRC_REG_MAP_T rMap;  
    
}ASRC_HW_CLS, *PASRC_HW_CLS;


static PASRC_HW_CLS _aprAsrcHw[ASRC_TYPE_NUM];    //just for CLI debug


//===================================//
  #define CodeSight_AsrcHw_Map
//===================================//

static void AsrcHw_RegMap(PASRC_HW_CLS prThis)
{
    u32 u4RegBase = GET_ASRC_REGBASE(prThis->eType);
    ASRCLOG_INFO(T("[Reg(%d - 0x%x)]Map, RegBase: 0x%x \n"), prThis->eType, (u32)prThis, (u32)u4RegBase);
    
    prThis->rMap.GenConf[0] = u4RegBase + REG_ASRC_GEN_CONF;
    prThis->rMap.GenConf[1] = u4RegBase + REG_ASRC_GEN_CONF2;

    prThis->rMap.IER[0] = u4RegBase + REG_ASRC_IER(prThis->eType);
    prThis->rMap.IER[1] = u4RegBase + REG_ASRC_IER2(prThis->eType);

    prThis->rMap.IFR[0] = u4RegBase + REG_ASRC_IFR;
    prThis->rMap.IFR[1] = u4RegBase + REG_ASRC_IFR2;

    prThis->rMap.ChCnfg[0] = u4RegBase + REG_ASRC_CH01_CNFG;
    prThis->rMap.ChCnfg[1] = u4RegBase + REG_ASRC_CH23_CNFG;
    prThis->rMap.ChCnfg[2] = u4RegBase + REG_ASRC_CH45_CNFG;
    prThis->rMap.ChCnfg[3] = u4RegBase + REG_ASRC_CH67_CNFG;
    prThis->rMap.ChCnfg[4] = u4RegBase + REG_ASRC_CH89_CNFG;
    prThis->rMap.ChCnfg[5] = u4RegBase + REG_ASRC_CH1011_CNFG;

    prThis->rMap.Fs[0] = u4RegBase + REG_ASRC_FREQUENCY0;
    prThis->rMap.Fs[1] = u4RegBase + REG_ASRC_FREQUENCY1;
    prThis->rMap.Fs[2] = u4RegBase + REG_ASRC_FREQUENCY2;
    prThis->rMap.Fs[3] = u4RegBase + REG_ASRC_FREQUENCY3;
    prThis->rMap.Fs[4] = u4RegBase + REG_ASRC_FREQUENCY4;
    prThis->rMap.Fs[5] = u4RegBase + REG_ASRC_FREQUENCY5;
    prThis->rMap.Fs[6] = u4RegBase + REG_ASRC_FREQUENCY6;
    prThis->rMap.Fs[7] = u4RegBase + REG_ASRC_FREQUENCY7;

    prThis->rMap.IBufSAdr = u4RegBase + REG_ASRC_IBUF_SADR;
    prThis->rMap.IBufSize = u4RegBase + REG_ASRC_IBUF_SIZE;
    prThis->rMap.OBufSAdr = u4RegBase + REG_ASRC_OBUF_SADR;
    prThis->rMap.OBufSize = u4RegBase + REG_ASRC_OBUF_SIZE;

    prThis->rMap.IBufRp[0] = u4RegBase + REG_ASRC_CH01_IBUF_RDPNT;
    prThis->rMap.IBufRp[1] = u4RegBase + REG_ASRC_CH23_IBUF_RDPNT;
    prThis->rMap.IBufRp[2] = u4RegBase + REG_ASRC_CH45_IBUF_RDPNT;
    prThis->rMap.IBufRp[3] = u4RegBase + REG_ASRC_CH67_IBUF_RDPNT;
    prThis->rMap.IBufRp[4] = u4RegBase + REG_ASRC_CH89_IBUF_RDPNT;
    prThis->rMap.IBufRp[5] = u4RegBase + REG_ASRC_CH1011_IBUF_RDPNT;

    prThis->rMap.IBufWp[0] = u4RegBase + REG_ASRC_CH01_IBUF_WRPNT;
    prThis->rMap.IBufWp[1] = u4RegBase + REG_ASRC_CH23_IBUF_WRPNT;
    prThis->rMap.IBufWp[2] = u4RegBase + REG_ASRC_CH45_IBUF_WRPNT;
    prThis->rMap.IBufWp[3] = u4RegBase + REG_ASRC_CH67_IBUF_WRPNT;     
    prThis->rMap.IBufWp[4] = u4RegBase + REG_ASRC_CH89_IBUF_WRPNT;
    prThis->rMap.IBufWp[5] = u4RegBase + REG_ASRC_CH1011_IBUF_WRPNT; 

    prThis->rMap.OBufWp[0] = u4RegBase + REG_ASRC_CH01_OBUF_WRPNT;
    prThis->rMap.OBufWp[1] = u4RegBase + REG_ASRC_CH23_OBUF_WRPNT;
    prThis->rMap.OBufWp[2] = u4RegBase + REG_ASRC_CH45_OBUF_WRPNT;
    prThis->rMap.OBufWp[3] = u4RegBase + REG_ASRC_CH67_OBUF_WRPNT; 
    prThis->rMap.OBufWp[4] = u4RegBase + REG_ASRC_CH89_OBUF_WRPNT;
    prThis->rMap.OBufWp[5] = u4RegBase + REG_ASRC_CH1011_OBUF_WRPNT; 
    
    prThis->rMap.OBufRp[0] = u4RegBase + REG_ASRC_CH01_OBUF_RDPNT;
    prThis->rMap.OBufRp[1] = u4RegBase + REG_ASRC_CH23_OBUF_RDPNT;
    prThis->rMap.OBufRp[2] = u4RegBase + REG_ASRC_CH45_OBUF_RDPNT;
    prThis->rMap.OBufRp[3] = u4RegBase + REG_ASRC_CH67_OBUF_RDPNT;
    prThis->rMap.OBufRp[4] = u4RegBase + REG_ASRC_CH89_OBUF_RDPNT;
    prThis->rMap.OBufRp[5] = u4RegBase + REG_ASRC_CH1011_OBUF_RDPNT; 

    prThis->rMap.IBufIntrCnt[0] = u4RegBase + REG_ASRC_IBUF_INTR_CNT0;
    prThis->rMap.IBufIntrCnt[1] = u4RegBase + REG_ASRC_IBUF_INTR_CNT1;
    prThis->rMap.IBufIntrCnt[2] = u4RegBase + REG_ASRC_IBUF_INTR_CNT2;

    prThis->rMap.OBufIntrCnt[0] = u4RegBase + REG_ASRC_OBUF_INTR_CNT0;
    prThis->rMap.OBufIntrCnt[1] = u4RegBase + REG_ASRC_OBUF_INTR_CNT1;
    prThis->rMap.OBufIntrCnt[2] = u4RegBase + REG_ASRC_OBUF_INTR_CNT2;

    prThis->rMap.BAK = u4RegBase + REG_ASRC_BAK;
    prThis->rMap.FreqTransNum = u4RegBase + REG_ASRC_FREQ_TRANS_NUM;
     
    prThis->rMap.MaxOutPerIn[0] = u4RegBase + REG_ASRC_MAX_OUTPUT_PER_IN0;
    prThis->rMap.MaxOutPerIn[1] = u4RegBase + REG_ASRC_MAX_OUTPUT_PER_IN1;

    prThis->rMap.FreqCaliCtrl[0] = u4RegBase + REG_ASRC_FREQ_CALI_CTRL;
    prThis->rMap.FreqCaliCtrl[1] = u4RegBase + REG_ASRC_FREQ_CALI2_CTRL;
    
    prThis->rMap.PrdCaliRslt[0] = u4RegBase + REG_ASRC_PRD_CALI_RSLT;
    prThis->rMap.PrdCaliRslt[1] = u4RegBase + REG_ASRC_PRD_CALI2_RSLT;

    prThis->rMap.FreqCaliRslt[0] = u4RegBase + REG_ASRC_FREQ_CALI_RSLT;
    prThis->rMap.FreqCaliRslt[1] = u4RegBase + REG_ASRC_FREQ_CALI2_RSLT; 

    prThis->rMap.IIRCramAddr = u4RegBase + REG_ASRC_IIR_CRAM_ADDR; 
    prThis->rMap.IIRCramData = u4RegBase + REG_ASRC_IIR_CRAM_DATA; 
    prThis->rMap.DmaCfg = u4RegBase + REG_ASRC_DMA_CFG;
    prThis->rMap.IFsOFsSel = u4RegBase + REG_ASRC_IFS_OFS_SEL;
}


//=====================================//
  #define CodeSight_AsrcHw_GEN_CONF
//=====================================//

static void AsrcHw_Enable(void * pThis, bool fgAsrcEn)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    
    RegAsrc_GenConf_EN_W(prThis->rMap, fgAsrcEn);
      
    ASRCLOG_DBG(T("[HW(%d)]Enable(%d) Asrc! \n"), prThis->eType, (s32)fgAsrcEn);
}


static bool AsrcHw_ChSetEnable(void * pThis, u32 u4Idx, bool fgChSetEn)
{   
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    bool fgRet = TRUE;
    
    if (u4Idx < ASRC_CHSET_NUM) { 
        RegAsrc_GenConf_ChEn_W(prThis->rMap, u4Idx, fgChSetEn);
    } else {
        fgRet = FALSE;
    }

    ASRCLOG_ERR_DBG(!fgRet, T("[HW(%d - %d)] Enable(%d)! \n"), prThis->eType, (s32)u4Idx, (s32)fgChSetEn);
    
    return (fgRet);
}


static bool AsrcHw_ChSetClear(void * pThis, u32 u4Idx)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    bool fgRet = TRUE;
    
    if (u4Idx < ASRC_CHSET_NUM) { 
        RegAsrc_GenConf_ChClear_W(prThis->rMap, u4Idx, 1);  
    } else {
        fgRet = FALSE;
    }
    
    ASRCLOG_ERR_DBG(!fgRet, T("[HW(%d - %d)] Clear ! \n"), prThis->eType, (s32)u4Idx);
    
    return (fgRet);
}


//================================//
  #define CodeSight_AsrcHw_INT
//================================//

static void AsrcHw_ClearIFR(void * pThis, bool fgClear)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    static u32 au4OldIFR[2];
    
    if (fgClear)
    {    
        AUDREG_WRITE(prThis->rMap.IFR[0], au4OldIFR[0]);
        AUDREG_WRITE(prThis->rMap.IFR[1], au4OldIFR[1]);
    }
    else
    {
        au4OldIFR[0] = AUDREG_READ(prThis->rMap.IFR[0]);
        au4OldIFR[1] = AUDREG_READ(prThis->rMap.IFR[1]); 
    }
}


static u32 AsrcHw_GetIBufEmptyFlag(void * pThis, u32 u4Idx)
{ 
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    
    return (RegAsrc_IER_IBufEmptyInten_R(prThis->rMap, u4Idx) & 
            RegAsrc_IFR_IBufEmptyFlag_R(prThis->rMap, u4Idx));
}


static u32 AsrcHw_GetIBufAmountFlag(void * pThis, u32 u4Idx)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    
    return (RegAsrc_IER_IBufAmountInten_R(prThis->rMap, u4Idx) & 
            RegAsrc_IFR_IBufAmountFlag_R(prThis->rMap, u4Idx));
}


static u32 AsrcHw_GetOBufOvFlag(void * pThis, u32 u4Idx)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    
    return (RegAsrc_IER_OBufOvInten_R(prThis->rMap, u4Idx) & 
            RegAsrc_IFR_OBufOvFlag_R(prThis->rMap, u4Idx));
}


static u32 AsrcHw_GetOBufAmountFlag(void * pThis, u32 u4Idx)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    
    return (RegAsrc_IER_OBufAmountInten_R(prThis->rMap, u4Idx) & 
            RegAsrc_IFR_OBufAmountFlag_R(prThis->rMap, u4Idx));
}


static u32 AsrcHw_GetIntrVal(u32 u4Idx, u32 u4IntType)
{
    u32 u4RegVal = 0;

    if (ASRC_IBUF_AMOUNT_INT & u4IntType) {
        u4RegVal |= ASRC_IBUF_AMOUNT_BIT_VAL(u4Idx);
    }
    if (ASRC_IBUF_EMPTY_INT & u4IntType) {
        u4RegVal |= ASRC_IBUF_EMPTY_BIT_VAL(u4Idx);
    }
    if (ASRC_OBUF_OV_INT & u4IntType) {
        u4RegVal |= ASRC_OBUF_OV_BIT_VAL(u4Idx);
    }
    if (ASRC_OBUF_AMOUNT_INT & u4IntType) {
        u4RegVal |= ASRC_OBUF_AMOUNT_BIT_VAL(u4Idx);
    }

    return (u4RegVal);
}


static void AsrcHw_INTEnable(void * pThis, u32 u4Idx, u32 u4IntType)
{ 
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    u32 u4RegVal = AsrcHw_GetIntrVal(u4Idx, u4IntType);  
   
    RegAsrc_IFR_WRITE(prThis->rMap, u4Idx, u4RegVal);    
    u4RegVal |= RegAsrc_IER_READ(prThis->rMap, u4Idx);
    RegAsrc_IER_WRITE(prThis->rMap, u4Idx, u4RegVal);    // Enable interrupt.
}


static void AsrcHw_INTDisable(void * pThis, u32 u4Idx, u32 u4IntType)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    u32 u4RegVal = AsrcHw_GetIntrVal(u4Idx, u4IntType);  
 
    RegAsrc_IFR_WRITE(prThis->rMap, u4Idx, u4RegVal);
    u4RegVal = ~u4RegVal;
    u4RegVal &= RegAsrc_IER_READ(prThis->rMap, u4Idx);
    RegAsrc_IER_WRITE(prThis->rMap, u4Idx, u4RegVal);    // Disable interrupt.   
}


//====================================//
  #define CodeSight_AsrcHw_CH_COFG
//====================================//

static bool  AsrcHw_SetChCalcAmount(void * pThis, u32 u4Idx, u32 u4CalcAmount)
{  
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    bool fgRet = TRUE;

    if(u4Idx < ASRC_CHSET_NUM && u4CalcAmount < ASRC_CALC_AMOUNT_MAX) {
        RegAsrc_ChCnfg_CalcAmount_W(prThis->rMap, u4Idx, u4CalcAmount);
    } else {
        fgRet = FALSE;
    }

    ASRCLOG_ERR_DBG(!fgRet, T("[HW(%d - %d)] CalcAmount(0x%x)! \n"), 
        prThis->eType, (s32)u4Idx, (u32)u4CalcAmount);
 
    return (fgRet);
}


static bool AsrcHw_SetChFs(void * pThis, u32 u4Idx, u32 u4IPalette, u32 u4OPalette)
{   
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    bool fgRet = TRUE;
    
    if(u4Idx < ASRC_CHSET_NUM && u4IPalette < ASRC_PALETTE_NUM  && u4OPalette < ASRC_PALETTE_NUM) 
    {
        RegAsrc_ChCnfg_IFS_W(prThis->rMap, u4Idx, u4IPalette);
        RegAsrc_FsSel_IFS_W(prThis->rMap, u4Idx, u4IPalette);
        
        RegAsrc_ChCnfg_OFS_W(prThis->rMap, u4Idx, u4OPalette);
        RegAsrc_FsSel_OFS_W(prThis->rMap, u4Idx, u4OPalette);
    } 
    else 
    {
        fgRet = FALSE;
    }

    ASRCLOG_ERR_DBG(!fgRet, T("[HW(%d - %d)] u4IPalette(%d) u4OPalette(%d)! \n"), 
        prThis->eType, (s32)u4Idx, (s32)u4IPalette, (s32)u4OPalette);
     
    return (fgRet);
}


static void AsrcHw_SetChMono(void * pThis, u32 u4Idx, bool fgMono)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    
    RegAsrc_ChCnfg_MONO_W(prThis->rMap, u4Idx, fgMono);
    ASRCLOG_DBG(T("[HW(%d - %d)] Mono(%d)! \n"), 
        prThis->eType, (s32)u4Idx, (s32)fgMono);
}


static u32 AsrcHw_GetChMono(void * pThis, u32 u4Idx)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    bool fgMono = RegAsrc_ChCnfg_MONO_R(prThis->rMap, u4Idx);

    return ((u32)fgMono);
}


static bool AsrcHw_SetChBitWidth(void * pThis, u32 u4Idx, u32 u4IBW, u32 u4OBW)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    bool fgRet = TRUE;
    
    if ( u4Idx < ASRC_CHSET_NUM ||
        (u4IBW == 16 || u4IBW == 24) || 
        (u4OBW == 16 || u4OBW == 24)) 
    {
        RegAsrc_ChCnfg_IBitWidth_W(prThis->rMap, u4Idx, (u4IBW == 16) ? 1 : 0);
        RegAsrc_ChCnfg_OBitWidth_W(prThis->rMap, u4Idx, (u4OBW == 16) ? 1 : 0);               
    } 
    else 
    {
        fgRet = FALSE;
    }

    ASRCLOG_ERR_DBG(!fgRet, T("[HW(%d - %d)] IBW(%d) OBW(%d) \n"), 
        prThis->eType, (s32)u4Idx, (s32)u4IBW, (s32)u4OBW);
              
    return (fgRet);
}


//======================================// 
  #define CodeSight_RegAsrc_Frequency
//======================================//

static bool AsrcHw_SetPaletteFS(void * pThis, u32 u4PaletteIdx, u32 u4FS)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    bool fgRet = TRUE;
   
    if (u4PaletteIdx < ASRC_PALETTE_NUM && 
        (u4FS > 0 && u4FS < ASRC_FREQ_SUPPORT_MAX)) 
    {
        RegAsrc_Frequency_W(prThis->rMap, u4PaletteIdx, u4FS);      
    } 
    else 
    {
        fgRet = FALSE;
    }

    ASRCLOG_ERR_DBG(!fgRet, T("[HW(%d)] PaletteIdx(%d) SampleRate(%d) \n"), 
        prThis->eType, (s32)u4PaletteIdx, (s32)u4FS);
 
    return (fgRet);
}


static u32 AsrcHw_GetPaletteFS(void * pThis, u32 u4PaletteIdx)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    return (RegAsrc_Frequency_R(prThis->rMap, u4PaletteIdx));
}


//==================================//
  #define CodeSight_AsrcHw_BUF_ADR
//==================================//

static bool AsrcHw_SetIBufAddr(void * pThis, u32 u4Addr, u32 u4ChBufSize)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    bool fgRet = TRUE;
    u32 u4BankAddr = (u4Addr >> 20);
    u32 u4StartAddr = (u4Addr & 0xFFFFF);

    if (u4BankAddr < ASRC_BANK_ADDR_MAX && 
       (((u4StartAddr & 0xF) == 0) && (u4StartAddr < ASRC_BUF_SADR_MAX)) &&
       (((u4ChBufSize & 0xF) == 0) && (u4ChBufSize < ASRC_BUF_SIZE_MAX)))
    {    
        RegAsrc_BankAddr_Input_W(prThis->eType, u4BankAddr);
        RegAsrc_IBufSAdr_W(prThis->rMap, u4StartAddr);
        RegAsrc_ChIBufSize_W(prThis->rMap, u4ChBufSize);            
    } 
    else 
    {
        fgRet = FALSE;
    }

    ASRCLOG_ERR_DBG(!fgRet, T("[HW(%d)] IBufAddr: Bank(0x%x) Start(0x%x) ChSize(0x%x) \n"), 
            prThis->eType, (u32)u4BankAddr, (u32)u4StartAddr, (u32)u4ChBufSize); 
    
    return (fgRet);
}


static bool AsrcHw_SetOBufAddr(void * pThis, u32 u4Addr, u32 u4ChBufSize)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    bool fgRet = TRUE;
    u32 u4BankAddr = (u4Addr >> 20);
    u32 u4StartAddr = (u4Addr & 0xFFFFF);

    if (u4BankAddr < ASRC_BANK_ADDR_MAX && 
        (((u4StartAddr & 0xF) == 0) && (u4StartAddr < ASRC_BUF_SADR_MAX)) &&
        (((u4ChBufSize & 0xF) == 0) && (u4ChBufSize < ASRC_BUF_SIZE_MAX))) 
    {            
        RegAsrc_BankAddr_Output_W(prThis->eType, u4BankAddr);
        RegAsrc_OBufSAdr_W(prThis->rMap, u4StartAddr);
        RegAsrc_ChOBufSize_W(prThis->rMap, u4ChBufSize);   
    }
    else 
    {
        fgRet = FALSE;
    }

    ASRCLOG_ERR_DBG(!fgRet, T("[HW(%d)] OBufAddr: Bank(0x%x) Start(0x%x) ChSize(0x%x) \n"), 
            prThis->eType, (u32)u4BankAddr, (u32)u4StartAddr, (u32)u4ChBufSize); 
    
    return (fgRet);
}


//======================================//
  #define CodeSight_AsrcHw_BUF_POINT
//======================================//

static bool AsrcHw_SetIRP(void * pThis, u32 u4Idx, u32 u4IBufRp)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    bool fgRet = TRUE;

    if (u4Idx < ASRC_CHSET_NUM &&  
        (((u4IBufRp & 0xF) == 0) && (u4IBufRp < ASRC_BUF_POINTER_MAX))) 
    {
        RegAsrc_IBufRdpnt_W(prThis->rMap, u4Idx, u4IBufRp);
    } 
    else 
    {
        fgRet = FALSE;
        ASRCLOG_ERR(T("[HW(%d - %d)] IBufRp(0x%x) \n"), 
            prThis->eType, (s32)u4Idx, (u32)u4IBufRp);
    }

    return (fgRet);
}


static bool AsrcHw_SetIWP(void * pThis, u32 u4Idx, u32 u4IBufWp)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    bool fgRet = TRUE;
    
    if (u4Idx < ASRC_CHSET_NUM &&  
        (((u4IBufWp & 0xF) == 0) && (u4IBufWp < ASRC_BUF_POINTER_MAX))) 
    {
        RegAsrc_IBufWrpnt_W(prThis->rMap, u4Idx, u4IBufWp);
    }
    else 
    {
        fgRet = FALSE;
        ASRCLOG_ERR(T("[HW(%d - %d)] u4IBufWp(0x%x) \n"), 
            prThis->eType, (s32)u4Idx, (u32)u4IBufWp);
    }

    return (fgRet);
}


static bool AsrcHw_SetOWP(void * pThis, u32 u4Idx, u32 u4OBufWp)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    bool fgRet = TRUE;
        
    if (u4Idx < ASRC_CHSET_NUM &&  
        (((u4OBufWp & 0xF) == 0) && (u4OBufWp < ASRC_BUF_POINTER_MAX))) 
    {
        RegAsrc_OBufWrpnt_W(prThis->rMap, u4Idx, u4OBufWp);
    } 
    else 
    {
        fgRet = FALSE;
        ASRCLOG_ERR(T("[HW(%d - %d)] u4IBufWp(0x%x) \n"), 
            prThis->eType, (s32)u4Idx, (u32)u4OBufWp);
    }

    return (fgRet);
}


static bool AsrcHw_SetORP(void * pThis, u32 u4Idx, u32 u4OBufRp)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    bool fgRet = TRUE;
        
    if (u4Idx < ASRC_CHSET_NUM &&  
        (((u4OBufRp & 0xF) == 0) && (u4OBufRp < ASRC_BUF_POINTER_MAX))) 
    {
        RegAsrc_OBufRdpnt_W(prThis->rMap, u4Idx, u4OBufRp);
    } 
    else 
    {
        fgRet = FALSE;
        ASRCLOG_ERR(T("[HW(%d - %d)] u4OBufRp(0x%x) \n"), 
            prThis->eType, (s32)u4Idx, (u32)u4OBufRp);
    }

    return (fgRet);
}


static u32 AsrcHw_GetOWP(void * pThis, u32 u4Idx)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    
    return (RegAsrc_OBufWrpnt_R(prThis->rMap, u4Idx) & 0xfffff0);
}


static u32 AsrcHw_GetORP(void * pThis, u32 u4Idx)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    
    return (RegAsrc_OBufRdpnt_R(prThis->rMap, u4Idx) & 0xfffff0);
}


static u32 AsrcHw_GetIRP(void * pThis, u32 u4Idx)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    
    return (RegAsrc_IBufRdpnt_R(prThis->rMap, u4Idx) & 0xfffff0);
}


static u32 AsrcHw_GetIWP(void * pThis, u32 u4Idx)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    
    return (RegAsrc_IBufWrpnt_R(prThis->rMap, u4Idx) & 0xfffff0);
}


//=====================================//
  #define CodeSight_AsrcHw_INTR_CNT
//=====================================//

static bool AsrcHw_SetIBufIntrCnt(void * pThis, u32 u4Idx, u32 u4IntrCnt)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    bool fgRet = TRUE;
            
    if (u4Idx < ASRC_CHSET_NUM) {
        RegAsrc_IBufIntrCnt_W(prThis->rMap, u4Idx, u4IntrCnt);
    } else {
        fgRet = FALSE;
    }

    ASRCLOG_ERR_DBG(!fgRet, T("[HW(%d - %d)] IBufIntrCnt(0x%x) \n"), 
        prThis->eType, (s32)u4Idx, (u32)u4IntrCnt);
    
    return (fgRet);   
}


static bool AsrcHw_SetOBufIntrCnt(void * pThis, u32 u4Idx, u32 u4IntrCnt)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    bool fgRet = TRUE;
            
    if (u4Idx < ASRC_CHSET_NUM) {
        RegAsrc_OBufIntrCnt_W(prThis->rMap, u4Idx, u4IntrCnt);
    } else {
        fgRet = FALSE;      
    }

    ASRCLOG_ERR_DBG(!fgRet, T("[HW(%d - %d)] OBufIntrCnt(0x%x) \n"), 
        prThis->eType, (s32)u4Idx, (u32)u4IntrCnt);
    
    return (fgRet);   
}


//=====================================//
  #define CodeSight_AsrcHw_INTR_CNT
//=====================================//

static bool AsrcHw_SetMaxOutPerIn(void * pThis, u32 u4Idx, u32 u4MaxOutPerIn)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    bool fgRet = TRUE;
                
    if (u4Idx < ASRC_CHSET_NUM && u4MaxOutPerIn < 0x10) {
        RegAsrc_MaxOutPerIn_W(prThis->rMap, u4Idx, u4MaxOutPerIn);
    } else {
        fgRet = FALSE;
    }

    ASRCLOG_ERR_DBG(!fgRet, T("[HW(%d - %d)] MaxOutPerIn(%d) \n"), 
        prThis->eType, (s32)u4Idx, (s32)u4MaxOutPerIn);

    return (fgRet);   
}


//==========================================//
  #define CodeSight_AsrcHw_AUTO_TRACING
//==========================================//

/*
      (TransNum * 2^23) / (clock / 48000 * (cycle+1)) = PaletteFS
  ==> TransNum = (PaletteFS * clock / 48000 * (cycle+1)) / (2^23)     
*/
static void AsrcHw_AutoTraceEnable(void * pThis, bool fgEnable, u32 u4TracingClk)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    bool fgClkSel = u4TracingClk & 0x1;
    bool fgFS2En = u4TracingClk & 0x2;
    bool fgFS3En = u4TracingClk & 0x4;

    if (fgEnable)
    {    
        RegAsrc_FreqCaliCyc_W(prThis->rMap, 0x3FF);   
        if (fgClkSel) {
            RegAsrc_FreqTransNum_W(prThis->rMap, 0xCE0);              // DSP CLK : 27MHz 
            if (fgFS2En) {
                AUDREG_WRITE(prThis->rMap.FreqCaliCtrl[0], 0x40B00);
            }
            if (fgFS3En) {
                AUDREG_WRITE(prThis->rMap.FreqCaliCtrl[1], 0x40B00);
            }
        } else {
            RegAsrc_FreqTransNum_W(prThis->rMap, 0x2EE0);             // Extern : 24.576*4 = 98.304MHz 
            if (fgFS2En) {
                AUDREG_WRITE(prThis->rMap.FreqCaliCtrl[0], 0xB00);
            }
            if (fgFS3En) {
                AUDREG_WRITE(prThis->rMap.FreqCaliCtrl[1],0xB00);
            }
        }

        ASRCLOG_DBG(T("[HW(%d)]Auto Tracing: fgClkSel(%d) CALI(0x%x) CALI2(0x%x) \r\n"),
            prThis->eType, (s32)fgClkSel, (u32)(AUDREG_READ(prThis->rMap.FreqCaliCtrl[0])), 
            (u32)(AUDREG_READ(prThis->rMap.FreqCaliCtrl[1])));
    }
    else
    {   
        if (fgFS2En) {
            RegAsrc_FreqCaliCtrl_CaliEn_W(prThis->rMap, 0, 0);            // Disable Auto Tracing
            RegAsrc_FreqCaliCtrl_AutoRestrt_W(prThis->rMap, 0, 0);
        }

        if (fgFS3En) {
            RegAsrc_FreqCaliCtrl_CaliEn_W(prThis->rMap, 1, 0);            // Disable Auto Tracing     
            RegAsrc_FreqCaliCtrl_AutoRestrt_W(prThis->rMap, 1, 0);
        }
    }
}


//=====================================//
  #define CodeSight_AsrcHw_DMA_CFG
//=====================================//

static void AsrcHw_Reset(void * pThis)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    
    RegAsrc_Soft_Reset(prThis->eType);     
    RegAsrc_Reset_Reg();
    RegAsrc_DmaCfg_Reset_W(prThis->rMap, 1);
}


//===================================//
  #define CodeSight_AsrcHw_DEBUG 
//===================================//

static void AsrcHw_LogAllRegs(void * pThis)
{
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    u32 u4Loop = 0;
    u32 u4RegBase = GET_ASRC_REGBASE(prThis->eType);

    ASRCLOG_INFO(T("Dumping ASRC Register Start>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));

    for (u4Loop = 0; u4Loop < 0x140; u4Loop += 16)
    {
        ASRCLOG_INFO(T("    0x%5X : Val: 0x%08x, 0x%08x, 0x%08x, 0x%08x.\r\n"), 
            (u32)(u4RegBase + u4Loop ), 
            (u32)(AUDREG_READ(u4RegBase + u4Loop)), 
            (u32)(AUDREG_READ(u4RegBase + u4Loop + 4)),
            (u32)(AUDREG_READ(u4RegBase + u4Loop + 8)), 
            (u32)(AUDREG_READ(u4RegBase + u4Loop + 12)));
    }

    RegAsrc_OthersReg_Read();

    ASRCLOG_INFO(T("Dumping ASRC Register End <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n"));
}


//===================================//
  #define CodeSight_AsrcHw_Create
//===================================//

static u32 AsrcHw_Delete(void * pThis)
{  
    PASRC_HW_CLS prThis = (PASRC_HW_CLS)pThis;
    
    AUD_CLASS_DELETE();

    return (0);
}


static void AsrcHw_FuncPointInit(PASRC_HW_CLS prThis)
{
    PASRC_HW_CLS_PUB prPub =  &prThis->rPub;
    prPub->Delete = AsrcHw_Delete;

    prPub->Enable = AsrcHw_Enable;
    prPub->ChSetEnable = AsrcHw_ChSetEnable;
    prPub->ChSetClear = AsrcHw_ChSetClear;
    
    prPub->ClearIFR = AsrcHw_ClearIFR;
    prPub->GetIBufEmptyFlag = AsrcHw_GetIBufEmptyFlag;
    prPub->GetIBufAmountFlag = AsrcHw_GetIBufAmountFlag;
    prPub->GetOBufOvFlag = AsrcHw_GetOBufOvFlag;
    prPub->GetOBufAmountFlag = AsrcHw_GetOBufAmountFlag;
    
    prPub->INTEnable = AsrcHw_INTEnable;
    prPub->INTDisable = AsrcHw_INTDisable;
    
    prPub->SetChCalcAmount = AsrcHw_SetChCalcAmount;
    prPub->SetChFs = AsrcHw_SetChFs;
    prPub->SetChMono = AsrcHw_SetChMono;
    prPub->GetChMono = AsrcHw_GetChMono;
    prPub->SetChBitWidth = AsrcHw_SetChBitWidth;
    
    prPub->SetPaletteFS = AsrcHw_SetPaletteFS;
    prPub->GetPaletteFS = AsrcHw_GetPaletteFS;
    
    prPub->SetIBufAddr = AsrcHw_SetIBufAddr;
    prPub->SetOBufAddr = AsrcHw_SetOBufAddr;
    
    prPub->SetIRP = AsrcHw_SetIRP;
    prPub->SetIWP = AsrcHw_SetIWP;
    prPub->SetOWP = AsrcHw_SetOWP;
    prPub->SetORP = AsrcHw_SetORP;
    
    prPub->GetOWP = AsrcHw_GetOWP;
    prPub->GetORP = AsrcHw_GetORP;
    prPub->GetIRP = AsrcHw_GetIRP;
    prPub->GetIWP = AsrcHw_GetIWP;
    
    prPub->SetIBufIntrCnt = AsrcHw_SetIBufIntrCnt;
    prPub->SetOBufIntrCnt = AsrcHw_SetOBufIntrCnt;
    prPub->SetMaxOutPerIn = AsrcHw_SetMaxOutPerIn;
    
    prPub->AutoTraceEnable = AsrcHw_AutoTraceEnable;
    prPub->Reset = AsrcHw_Reset;
    
    prPub->LogAllRegs = AsrcHw_LogAllRegs;
}


PASRC_HW_CLS_PUB AsrcHw_New(ASRC_CLS_TYPE eType)
{
    PASRC_HW_CLS prThis = AUD_CLASS_NEW(ASRC_HW_CLS);

    if (prThis)
    {      
        _aprAsrcHw[eType] = prThis;
        ASRCLOG_DBG(T("[HW] Create! (%d - 0x%x)\n"), eType, (u32)prThis);
        
        prThis->eType = eType;
        AsrcHw_RegMap(prThis);

        AsrcHw_FuncPointInit(prThis);     
    }  
    
    return ((PASRC_HW_CLS_PUB)prThis);
}


