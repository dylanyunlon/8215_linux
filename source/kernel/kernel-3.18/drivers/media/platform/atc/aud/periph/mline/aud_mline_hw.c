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

/**
 * @file aud_mline_hw.c source file
 * 
 * aud io mlin module hardware driver
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */

#include "aud_mline_hal.h"
#include "aud_reg_env.h"
#include "aud_reg_rgbk2.h"

typedef struct 
{
    MLIN_HW_CLS_PUB rPub;

}MLIN_HW_CLS, *PMLIN_HW_CLS;


//=========================================================//
    #define CodeSight_MlinHw_Static_Func
//=========================================================//


/**
 * set multi line in source
 *
 * @param [in]  prThis : hw class; eSrc : s32 or ext linein
 * @param [out] 
 *
 * @return
 */
static void MlinHw_SetSrc(void * pThis, AUD_MLIN_SRC eSrc)
{
    bool fgAoutSrcDel, fgRxSrcSel;

    fgAoutSrcDel = (MLIN_SRC_AOUT == eSrc) ? TRUE : FALSE;
    fgRxSrcSel = (MLIN_SRC_HDMI_RX == eSrc) ? TRUE : FALSE;

  #ifdef MLIN_SIMULATE
    //clock config
    MLINREG_BITS_W("SetSrc",
                   "REGENV_LIN_MULTI",
                   REGENV_LIN_MULTI, 
                   "BCKLRCK_SEL_0",
                   BIT_STR_MULTI_BCKLRCK_SEL_0,
                   BIT_NUM_MULTI_BCKLRCK_SEL_0,
                   fgAoutSrcDel);
    MLINREG_BITS_W("SetSrc",
                   "REGENV_LIN_MULTI",
                   REGENV_LIN_MULTI,
                   "BCKLRCK_SEL_1",
                   BIT_STR_MULTI_BCKLRCK_SEL_1,
                   BIT_NUM_MULTI_BCKLRCK_SEL_1,
                   fgRxSrcSel);
    
    //data config
    MLINREG_BITS_W("SetSrc",
                   "REGENV_LIN_MULTI",
                   REGENV_LIN_MULTI,
                   "AO_LOOP_SEL",
                   BIT_STR_MULTI_AO_LOOP_SEL,
                   BIT_NUM_MULTI_AO_LOOP_SEL,
                   fgAoutSrcDel);
    MLINREG_BITS_W("SetSrc",
                   "REGENV_LIN_MULTI",
                   REGENV_LIN_MULTI, 
                   "SDATA0_SEL_1",
                   BIT_STR_MULTI_SDATA0_SEL_1,
                   BIT_NUM_MULTI_SDATA0_SEL_1,
                   fgRxSrcSel);
  #else
    //clock config
    AUDREG_BITS_W(REGENV_LIN_MULTI, 
                  BIT_STR_MULTI_BCKLRCK_SEL_0,
                  BIT_NUM_MULTI_BCKLRCK_SEL_0,
                  fgAoutSrcDel);
    AUDREG_BITS_W(REGENV_LIN_MULTI, 
                  BIT_STR_MULTI_BCKLRCK_SEL_1,
                  BIT_NUM_MULTI_BCKLRCK_SEL_1,
                  fgRxSrcSel);
    
    //data config
    AUDREG_BITS_W(REGENV_LIN_MULTI, 
                  BIT_STR_MULTI_AO_LOOP_SEL,
                  BIT_NUM_MULTI_AO_LOOP_SEL,
                  fgAoutSrcDel);
    AUDREG_BITS_W(REGENV_LIN_MULTI, 
                  BIT_STR_MULTI_SDATA0_SEL_1,
                  BIT_NUM_MULTI_SDATA0_SEL_1,
                  fgRxSrcSel);
   #endif
}

/**
 * set multi line in input data bit number
 *
 * @param [in]  prThis : hw class; u4BitNum : bit number of input data
 * @param [out] 
 *
 * @return
 */
static void MlinHw_SetSrcBitNum(void * pThis, u32 u4BitNum)
{
  #ifdef MLIN_SIMULATE
    MLINREG_BITS_W("SetSrcBitNum",
                   "REGENV_AIN_ACK_CFG_MULTI",
                   REGENV_AIN_ACK_CFG_MULTI,
                   "SPL_BNUM",
                   BIT_STR_MULTI_SPL_BNUM,
                   BIT_NUM_MULTI_SPL_BNUM,
                   (u4BitNum - 1));
  #else
    AUDREG_BITS_W(REGENV_AIN_ACK_CFG_MULTI,
                  BIT_STR_MULTI_SPL_BNUM,
                  BIT_NUM_MULTI_SPL_BNUM,
                  (u4BitNum - 1));
  #endif
}

/**
 * set multi line in output data bit mode
 *
 * @param [in]  prThis : hw class; eOutBitNum : 24bit or 16bit
 * @param [out] 
 *
 * @return
 */
static void MlinHw_SetOutBitNum(void * pThis, AUD_LIN_OUT_BITNUM eOutBitNum)
{
  #ifdef MLIN_SIMULATE
    MLINREG_BITS_W("SetOutBitNum",
                   "REGENV_LIN_MULTI",
                   REGENV_LIN_MULTI,
                   "BIT24",
                   BIT_STR_MULTI_BIT24,
                   BIT_NUM_MULTI_BIT24,
                   eOutBitNum);
  #else
    AUDREG_BITS_W(REGENV_LIN_MULTI,
                  BIT_STR_MULTI_BIT24,
                  BIT_NUM_MULTI_BIT24,
                  eOutBitNum);
  #endif
}

/**
 * set mulit line in data format
 *
 * @param [in]  prThis : hw class; eFmt : data format
 * @param [out] 
 *
 * @return
 */
static void MlinHw_SetDataFmt(void * pThis, AUDFMT_INTF_E eFmt)
{
  #ifdef MLIN_SIMULATE
    MLINREG_BITS_W("SetDataFmt",
                   "REGENV_AIN_ACK_CFG_MULTI",
                   REGENV_AIN_ACK_CFG_MULTI,
                   "LEFT_ALN",
                   BIT_STR_MULTI_LEFT_ALN,
                   BIT_NUM_MULTI_LEFT_ALN,
                   (eFmt & 0x1));
    MLINREG_BITS_W("SetDataFmt",
                   "REGENV_AIN_ACK_CFG_MULTI",
                   REGENV_AIN_ACK_CFG_MULTI,
                   "DAT_DLY",
                   BIT_STR_MULTI_DAT_DLY,
                   BIT_NUM_MULTI_DAT_DLY,
                   ((eFmt & 0x2) >> 1));
  #else
    AUDREG_BITS_W(REGENV_AIN_ACK_CFG_MULTI,
                  BIT_STR_MULTI_LEFT_ALN,
                  BIT_NUM_MULTI_LEFT_ALN,
                  (eFmt & 0x1));
    AUDREG_BITS_W(REGENV_AIN_ACK_CFG_MULTI,
                  BIT_STR_MULTI_DAT_DLY,
                  BIT_NUM_MULTI_DAT_DLY,
                  ((eFmt & 0x2) >> 1));
  #endif
}

/**
 * set multi line in bck to lrck ratio
 *
 * @param [in]  prThis : hw class; eCycle : lrck type
 * @param [out] 
 *
 * @return
 */
static void MlinHw_SetLrckDivider(PMLIN_HW_CLS prThis, AUD_LRCK_CYC_T eCycle)
{
  #ifdef MLIN_SIMULATE
    MLINREG_BITS_W("SetLrckDivider",
                   "REGENV_AIN_ACK_CFG_MULTI",
                   REGENV_AIN_ACK_CFG_MULTI,
                   "LRCK_CYCLE_SEL",
                   BIT_STR_MULTI_LRCK_CYCLE_SEL,
                   BIT_NUM_MULTI_LRCK_CYCLE_SEL,
                   eCycle);    
  #else
    AUDREG_BITS_W(REGENV_AIN_ACK_CFG_MULTI,
                  BIT_STR_MULTI_LRCK_CYCLE_SEL,
                  BIT_NUM_MULTI_LRCK_CYCLE_SEL,
                  eCycle);
  #endif
}

/**
 * set multi line channel number
 *
 * @param [in]  prThis : hw class; eChNum : mlin channel number
 * @param [out] 
 *
 * @return
 */
static void MlinHw_SetChNum(void * pThis, AUD_MLIN_CH_NUM_E eChNum)
{
  #ifdef MLIN_SIMULATE
    MLINREG_BITS_W("SetChNum",
                   "REGENV_AIN_ACK_CFG_MULTI",
                   REGENV_AIN_ACK_CFG_MULTI,
                   "CHNUM",
                   BIT_STR_MULTI_CHNUM,
                   BIT_NUM_MULTI_CHNUM,
                   eChNum);
  #else
    AUDREG_BITS_W(REGENV_AIN_ACK_CFG_MULTI,
                  BIT_STR_MULTI_CHNUM,
                  BIT_NUM_MULTI_CHNUM,
                  eChNum);
  #endif
}

/**
 * set multi line in start address
 *
 * @param [in]  prThis : hw class; u4Sadr : start address
 * @param [out] 
 *
 * @return
 */
static void MlinHw_SetSadr(PMLIN_HW_CLS prThis, u32 u4Sadr)
{
  #ifdef MLIN_SIMULATE
    MLINREG_BITS_W("SetSadr",
                   "REGENV_LINBLK_MULTI",
                   REGENV_LINBLK_MULTI,
                   "SBLK_MULTI",
                   BIT_STR_SPLIN_SBLK_MULTI,
                   BIT_NUM_SPLIN_SBLK_MULTI,
                   u4Sadr);
  #else
    AUDREG_BITS_W(REGENV_LINBLK_MULTI,
                  BIT_STR_SPLIN_SBLK_MULTI,
                  BIT_NUM_SPLIN_SBLK_MULTI,
                  u4Sadr);
  #endif
}

/**
 * set multi line in end address
 *
 * @param [in]  prThis : hw class; u4Eadr : end address
 * @param [out] 
 *
 * @return
 */
static void MlinHw_SetEadr(PMLIN_HW_CLS prThis, u32 u4Eadr)
{
  #ifdef MLIN_SIMULATE
    MLINREG_BITS_W("SetEadr",
                   "REGENV_LINBLK_MULTI",
                   REGENV_LINBLK_MULTI,
                   "EBLK_MULTI",
                   BIT_STR_SPLIN_EBLK_MULTI,
                   BIT_NUM_SPLIN_EBLK_MULTI,
                   u4Eadr);
  #else
    AUDREG_BITS_W(REGENV_LINBLK_MULTI,
                  BIT_STR_SPLIN_EBLK_MULTI,
                  BIT_NUM_SPLIN_EBLK_MULTI,
                  u4Eadr);
  #endif

}

/**
 * set multi line in bank address
 *
 * @param [in]  prThis : hw class; u4BankAdr : bank address
 * @param [out] 
 *
 * @return
 */
static void MlinHw_SetBankAdr(PMLIN_HW_CLS prThis, u32 u4BankAdr)
{
  #ifdef MLIN_SIMULATE
    MLINREG_BITS_W("SetBankAdr",
                   "REGENV_SPLIN_CTRL",
                   REGENV_SPLIN_CTRL,
                   "SPDFLIN_BANK",
                   BIT_STR_SPDFLIN_BANK,
                   BIT_NUM_SPDFLIN_BANK,
                   u4BankAdr);

  #else
    AUDREG_BITS_W(REGENV_SPLIN_CTRL,
                  BIT_STR_SPDFLIN_BANK,
                  BIT_NUM_SPDFLIN_BANK,
                  u4BankAdr);
  #endif
}

/**
 * set bck invert
 *
 * @param [in]  prThis : hw class; fgInvert : invert or not
 * @param [out] 
 *
 * @return
 */
static void MlinHw_SetBckInvert(PMLIN_HW_CLS prThis, bool fgInvert)
{
  #ifdef MLIN_SIMULATE
    MLINREG_BITS_W("SetBckInvert",
                   "REGENV_AIN_ACK_CFG_MULTI",
                   REGENV_AIN_ACK_CFG_MULTI,
                   "INV_BCK",
                   BIT_STR_MULTI_INV_BCK,
                   BIT_NUM_MULTI_INV_BCK,
                   fgInvert);

  #else
    AUDREG_BITS_W(REGENV_AIN_ACK_CFG_MULTI,
                  BIT_STR_MULTI_INV_BCK,
                  BIT_NUM_MULTI_INV_BCK,
                  fgInvert);
  #endif
}

/**
 * set lrck invert
 *
 * @param [in]  prThis : hw class; fgInvert : invert or not
 * @param [out] 
 *
 * @return
 */
static void MlinHw_SetLrckInvert(PMLIN_HW_CLS prThis, bool fgInvert)
{
  #ifdef MLIN_SIMULATE
     MLINREG_BITS_W("SetLrckInvert",
                    "REGENV_AIN_ACK_CFG_MULTI",
                    REGENV_AIN_ACK_CFG_MULTI,
                    "INV_LRCL",
                    BIT_STR_MULTI_INV_LRCL,
                    BIT_NUM_MULTI_INV_LRCL,
                    fgInvert);   
  #else
    AUDREG_BITS_W(REGENV_AIN_ACK_CFG_MULTI,
                  BIT_STR_MULTI_INV_LRCL,
                  BIT_NUM_MULTI_INV_LRCL,
                  fgInvert);
  #endif
}

/**
 * set multi line in interrupt period
 *
 * @param [in]  prThis : hw class; eIntPeriod : s32 period
 * @param [out] 
 *
 * @return
 */
static void MlinHw_SetIntPeriod(void * pThis, AUD_MLIN_INT_PERIOD eIntPeriod)
{
  #ifdef MLIN_SIMULATE
    MLINREG_BITS_W("SetIntPeriod",
                   "REGENV_LIN_MULTI",
                   REGENV_LIN_MULTI,
                   "INTR_PERIOD",
                   BIT_STR_MULTI_INTR_PERIOD,
                   BIT_NUM_MULTI_INTR_PERIOD,
                   eIntPeriod);

  #else
    AUDREG_BITS_W(REGENV_LIN_MULTI,
                  BIT_STR_MULTI_INTR_PERIOD,
                  BIT_NUM_MULTI_INTR_PERIOD,
                  eIntPeriod);
  #endif
}


//=========================================================//
    #define CodeSight_MlinHw_Pubilic_Func
//=========================================================//


/**
 * public interface : multi line in configure
 *
 * @param [in]  prThis : hw class; prCfg : multi linein hw mode setting
 * @param [out] 
 *
 * @return
 */
static void MlinHw_InitCfg(void * pThis, void * pCfg)
{
    PMLIN_HW_CLS prThis = (PMLIN_HW_CLS)pThis;
    PAUD_MLIN_CFG_T prCfg = (PAUD_MLIN_CFG_T)pCfg;
    
    u32 u4PhySadr, u4PhyEadr, u4BankAdr;
    
    PMLIN_EXTPARAMS_T prExtCfg = &(prCfg->rExtCfg);
    
    MLINLOG_INFO(T("MlinHw_InitCfg \n"));

    //MlinHw_SetSrc(prThis, prCfg->eSrc);

    MlinHw_SetSrcBitNum(prThis, prExtCfg->u4SrcBitNum);
    MlinHw_SetOutBitNum(prThis, prExtCfg->eOutBitNum);

    MlinHw_SetDataFmt(prThis, prExtCfg->eDataFmt);
    MlinHw_SetChNum(prThis, prExtCfg->eMlinChNum);
    
    MlinHw_SetLrckDivider(prThis, prExtCfg->eCycle);
    MlinHw_SetBckInvert(prThis, prCfg->fgInvertBck);
    MlinHw_SetLrckInvert(prThis, prCfg->fgInvertLrck);

    u4PhySadr = prCfg->rBuf.u4PhySddr;
    u4PhyEadr = prCfg->rBuf.u4PhySddr + prCfg->rBuf.u4ChBufSz * prCfg->rBuf.u4Chn;
    u4BankAdr = (prCfg->rBuf.u4PhySddr >> 20) & 0x7ff;
    
    MlinHw_SetSadr(prThis, (((u4PhySadr - (u4BankAdr << 20)) & 0x00FFFF00) >> 8));
    MlinHw_SetEadr(prThis, (((u4PhyEadr - (u4BankAdr << 20)) & 0x00FFFF00) >> 8));
    MlinHw_SetBankAdr(prThis, u4BankAdr);

    MlinHw_SetIntPeriod(prThis, prExtCfg->eIntPeriod);
}
    

/**
 * public interface : enable/disable multi line in hw
 *
 * @param [in]  prThis : hw class; fgEnable : enable or disable
 * @param [out] 
 *
 * @return
 */
static void MlinHw_Enable(void * pThis, bool fgEnable)
{
    MLINLOG_INFO(T("MlinHw_Enable \n"));
    
  #ifdef MLIN_SIMULATE
    MLINREG_BITS_W("Enable",
                   "REGENV_LIN_MULTI",
                   REGENV_LIN_MULTI,
                   "EN",
                   BIT_STR_MULTI_EN,
                   BIT_NUM_MULTI_EN,
                   fgEnable);

  #else
    AUDREG_BITS_W(REGENV_LIN_MULTI,
                  BIT_STR_MULTI_EN,
                  BIT_NUM_MULTI_EN,
                  fgEnable);
  #endif
}

/**
 * multi line in hw current write pointer get
 *
 * @param [in]  prThis : hw class
 * @param [out] 
 *
 * @return : current pointer
 */
static u32 MlinHw_GetWp(void * pThis)
{
    u32 u4MlinWp, u4MlinSadr;

  #ifdef MLIN_SIMULATE
   u4MlinSadr = 0x123400;
   u4MlinWp = 0x123400;
  #else
    u4MlinWp = AUDREG_READ(REGENV_SPMULTI_WRADR) << 4; //byte address
    u4MlinSadr = AUDREG_BITS_R(REGENV_LINBLK_MULTI, BIT_STR_SPLIN_SBLK_MULTI, BIT_NUM_SPLIN_SBLK_MULTI);

    u4MlinWp = u4MlinWp - (u4MlinSadr << 8);
  #endif
    
    return u4MlinWp;
}

void MlinHw_GetSpdifInfo(void * pThis, void * pSpdifInfo)
{
    PMLIN_SPDIF_INFO_T prSpdifInfo = (PMLIN_SPDIF_INFO_T)pSpdifInfo;
  #ifdef MLIN_SIMULATE
    prSpdifInfo->DETAIL = 2;
    prSpdifInfo->BSNUM = 3;
    prSpdifInfo->ROUGH = 1;
    prSpdifInfo->DEC = 0;
  #else
    prSpdifInfo->DETAIL = AUDREG_BITS_R(REGENV_MULTI_SPDF_TYPE, BIT_STR_MULTI_DETAIL, BIT_NUM_MULTI_DETAIL);
    prSpdifInfo->BSNUM = AUDREG_BITS_R(REGENV_MULTI_SPDF_TYPE, BIT_STR_MULTI_BSNUM, BIT_NUM_MULTI_BSNUM);
    prSpdifInfo->ROUGH = AUDREG_BITS_R(REGENV_MULTI_SPDF_TYPE, BIT_STR_MULTI_ROUGH, BIT_NUM_MULTI_ROUGH);
    prSpdifInfo->DEC = AUDREG_BITS_R(REGENV_MULTI_SPDF_TYPE, BIT_STR_MULTI_DEC, BIT_NUM_MULTI_DEC);
  #endif
  
    prSpdifInfo->REV0021 = 0;
}


void MlinHw_ClrSpdTypeDec(void * pThis, u8 u1Val)
{
    AUDREG_BITS_W(REGENV_MULTI_SPDF_TYPE, BIT_STR_MULTI_DEC, BIT_NUM_MULTI_DEC, u1Val);  
}


//===========================================//
    #define CodeSight_MlinHw_Create
//===========================================//

/**
 * delect a multi line in hw object
 *
 * @param [in]  prThis : pointer to the multi line in hw object
 * @param [out] 
 *
 * @return      0: OK; others: NG
 */
static u32 MlinHw_Delete(void * pThis)
{
    PMLIN_HW_CLS prThis = (PMLIN_HW_CLS)pThis;
    
    MLINLOG_INFO(T("MlinHw_Delete \n"));
    
    AUD_CLASS_DELETE();
    
    return (0);
}

/**
 * creat a new multi line in  hw object
 *
 * @param [in]
 * @param [out] 
 *
 * @return  pointer to new object
 */
PMLIN_HW_CLS_PUB MlinHw_New(void)
{
    PMLIN_HW_CLS prThis = AUD_CLASS_NEW(MLIN_HW_CLS);

    MLINLOG_INFO(T("MlinHw_New \n"));

    if (prThis)
    {
        prThis->rPub.InitCfg = MlinHw_InitCfg; 
        prThis->rPub.Enable = MlinHw_Enable;
        prThis->rPub.GetWp = MlinHw_GetWp;
        prThis->rPub.GetSpdifInfo = MlinHw_GetSpdifInfo;        
        prThis->rPub.ClrSpdTypeDec = MlinHw_ClrSpdTypeDec;
        
        //update mlin configure interface
        prThis->rPub.SetSrcBitNum = MlinHw_SetSrcBitNum;
        prThis->rPub.SetOutBitNum = MlinHw_SetOutBitNum;        
        prThis->rPub.SetDataFmt = MlinHw_SetDataFmt;
        prThis->rPub.SetChNum = MlinHw_SetChNum;
        prThis->rPub.SetIntPeriod = MlinHw_SetIntPeriod;        
        prThis->rPub.SetSrc = MlinHw_SetSrc;
       
        
        prThis->rPub.Delete = MlinHw_Delete;
    }

    return ((PMLIN_HW_CLS_PUB)prThis);
}


