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
 * @file aud_micin_hw.c source file
 * 
 * aud io mic in module hardware driver
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#include "aud_micin_hal.h"
#include "aud_reg_env.h"
#include "aud_reg_rgbk2.h"
#include "aud_io_clock_if.h"


typedef struct 
{
    MIC_HW_CLS_PUB rPub;

}MIC_HW_CLS, *PMIC_HW_CLS;


//=========================================================//
    #define CodeSight_MicHw_Static_Func
//=========================================================//

/**
 * set mic in control by ARM
 *
 * @param [in]  prThis : hw class; fgEn : if control by arm
 * @param [out] 
 *
 * @return
 */
static void MicHw_SetArmCtrl(PMIC_HW_CLS prThis, bool fgEn)
{
    AUDREG_BITS_W(REGENV_RGBK2_CFG1,
                  BIT_STR_MIC_IN_ARM_CTRL_EN,
                  BIT_NUM_MIC_IN_ARM_CTRL_EN,
                  fgEn);
}


/**
 * set mic in source
 *
 * @param [in]  prThis : hw class; eSrc : s32 or ext adc
 * @param [out] 
 *
 * @return
 */
static void MicHw_SetSrc(PMIC_HW_CLS prThis, AUD_MIC_SRC eSrc)
{
    bool fgExtAdc;

    fgExtAdc = (EXT_MICIN == eSrc) ? LIN_EXT_ADC : LIN_INT_ADC;

    AUDREG_BITS_W(REGENV_BYPS_VLUM_CFG1,
                  BIT_STR_MPH_AFE_MODE,
                  BIT_NUM_MPH_AFE_MODE,
                  fgExtAdc);
}

/**
 * set mic in clock source
 *
 * @param [in]  prThis : hw class;  eClkSrc : clk source that mic can select
 * @param [out] 
 *
 * @return
 */
static void MicHw_SetClkSrc(PMIC_HW_CLS prThis, AUD_MIC_CLK_SRC eClkSrc)
{
    bool fgAout2SrcSel, fgMphSrcSel;

    fgAout2SrcSel = (MIC_CLK_AOUT2 == eClkSrc) ? TRUE : FALSE;
    fgMphSrcSel = (MIC_CLK_MPH == eClkSrc) ? TRUE : FALSE;

    AUDREG_BITS_W(REGENV_AFE_TOP_CFG0,
                  BIT_STR_MIC_USE_AO2_GIM,
                  BIT_NUM_MIC_USE_AO2_GIM,
                  fgAout2SrcSel);

    AUDREG_BITS_W(REGENV_AIN_CFG,
                  BIT_STR_MPCLK_IND,
                  BIT_NUM_MPCLK_IND,
                  fgMphSrcSel);

    //only master mode support
    AUDREG_BITS_W(REGENV_AENV_BAK,
                  BIT_STR_MPHONE_SLAVE,
                  BIT_NUM_MPHONE_SLAVE,
                  FALSE);

    //decide lrck fetch data time :rising edge(1) / falling edge(0)
    //mphone need to set '1'
    AUDREG_BITS_W(REGENV_BYPS_VLUM_CFG1, 
                  BIT_STR_MPH_WE_SEL,
                  BIT_NUM_MPH_WE_SEL,
                  1);
}

/**
 * set mic in input data bit number
 *
 * @param [in]  prThis : hw class; u4BitNum : bit number of input data
 * @param [out] 
 *
 * @return
 */
static void MicHw_SetSrcBitNum(PMIC_HW_CLS prThis, u32 u4BitNum)
{
    AUDREG_BITS_W(REGENV_AIN_CFG,
                  BIT_STR_MP_BNUM,
                  BIT_NUM_MP_BNUM,
                  (u4BitNum - 1));
}

/**
 * set mic in output data bit mode
 *
 * @param [in]  prThis : hw class; eOutBitNum : 24bit or 16bit
 * @param [out] 
 *
 * @return
 */
static void MicHw_SetOutBitNum(PMIC_HW_CLS prThis, AUD_LIN_OUT_BITNUM eOutBitNum)
{
    bool fg16BitMode;

    fg16BitMode = (LIN_16 == eOutBitNum)? TRUE : FALSE;

    AUDREG_BITS_W(REGENV_AFE_TOP_CFG0,
                  BIT_STR_MPH_16BIT_MODE,
                  BIT_NUM_MPH_16BIT_MODE,
                  fg16BitMode);
}

/**
 * set mic in data format
 *
 * @param [in]  prThis : hw class; eFmt : data format
 * @param [out] 
 *
 * @return
 */
static void MicHw_SetDataFmt(PMIC_HW_CLS prThis, AUDFMT_INTF_E eFmt)
{
    AUDREG_BITS_W(REGENV_AIN_CFG,
                  BIT_STR_M_LEFT_A,
                  BIT_NUM_M_LEFT_A,
                  (eFmt & 0x1));
    
    AUDREG_BITS_W(REGENV_AIN_CFG,
                  BIT_STR_M_DAT_D,
                  BIT_NUM_M_DAT_D,
                  ((eFmt & 0x2) >> 1));
}

/**
 * set mic mck to bck ratio
 *
 * @param [in]  prThis : hw class; eMclkType : mclk type
 * @param [out] 
 *
 * @return
 */
static void MicHw_SetBckDivider(PMIC_HW_CLS prThis, MCLK_TYPE_T eMclkType)
{
    u32 u4MclkToBckRatio;
    
    u4MclkToBckRatio = IoClk_GetMclkToFsRatio(eMclkType);
    u4MclkToBckRatio /= (32 * 4);

    AUDREG_BITS_W(REGENV_MISC_CTRL,
                  BIT_STR_MPHONE_BCK_DIV,
                  BIT_NUM_MPHONE_BCK_DIV,
                  u4MclkToBckRatio);
}

/**
 * set mic in bck to lrck ratio
 *
 * @param [in]  prThis : hw class; eCycle : lrck type
 * @param [out] 
 *
 * @return
 */
static void MicHw_SetLrckDivider(PMIC_HW_CLS prThis, AUD_LRCK_CYC_T eCycle)
{
    AUDREG_BITS_W(REGENV_MISC_CTRL,
                  BIT_STR_MPHONE_LRCK_DIV_SEL,
                  BIT_NUM_MPHONE_LRCK_DIV_SEL,
                  eCycle);
}

/**
 * set mic in start address
 *
 * @param [in]  prThis : hw class; u4Sadr : start address
 * @param [out] 
 *
 * @return
 */
static void MicHw_SetSadr(PMIC_HW_CLS prThis, u32 u4Sadr1, u32 u4Sadr2, u32 u4Sadr3)
{
    AUDREG_BITS_W(REGENV_RGBK2_CFG2,
                  BIT_STR_MPBUF1_SADR,
                  BIT_NUM_MPBUF1_SADR,
                  u4Sadr1);

    AUDREG_BITS_W(REGENV_RGBK2_CFG2,
                  BIT_STR_MPBUF2_SADR,
                  BIT_NUM_MPBUF2_SADR,
                  u4Sadr2);

    AUDREG_BITS_W(REGENV_RGBK2_CFG3,
                  BIT_STR_MPBUF3_SADR,
                  BIT_NUM_MPBUF3_SADR,
                  u4Sadr3);
}

/**
 * set mic in bank address
 *
 * @param [in]  prThis : hw class; u4BankAdr : bank address
 * @param [out] 
 *
 * @return
 */
static void MicHw_SetBankAdr(PMIC_HW_CLS prThis, u32 u4BankAdr)
{
    AUDREG_BITS_W(REGENV_RGBK2_CFG4,
                  BIT_STR_MPHONE_BANK,
                  BIT_NUM_MPHONE_BANK,
                  u4BankAdr);
}

/**
 * set mic in blk address
 *
 * @param [in]  prThis : hw class; u4BlkAdr : blk address
 * @param [out] 
 *
 * @return
 */
static void MicHw_SetBlkAdr(PMIC_HW_CLS prThis, u32 u4BlkAdr)
{
    AUDREG_BITS_W(REGENV_RGBK2_CFG5, BIT_STR_ARM_MIC_BLK6,
                  BIT_NUM_ARM_MIC_BLK6, u4BlkAdr);
}


/**
 * set bck invert
 *
 * @param [in]  prThis : hw class; fgInvert : invert or not
 * @param [out] 
 *
 * @return
 */
static void MicHw_SetBckInvert(PMIC_HW_CLS prThis, bool fgInvert)
{
    AUDREG_BITS_W(REGENV_AIN_CFG,
                  BIT_STR_MPBCK_INV,
                  BIT_NUM_MPBCK_INV,
                  fgInvert);
}

/**
 * set lrck invert
 *
 * @param [in]  prThis : hw class; fgInvert : invert or not
 * @param [out] 
 *
 * @return
 */
static void MicHw_SetLrckInvert(PMIC_HW_CLS prThis, bool fgInvert)
{
    AUDREG_BITS_W(REGENV_AIN_CFG,
                  BIT_STR_M_INV_LRCK,
                  BIT_NUM_M_INV_LRCK,
                  fgInvert);
}

/**
 * set AfeX for mic in
 *
 * @param [in]  prThis : hw class; eAdcId : AFe ID
 * @param [out] 
 *
 * @return
 */
static void MicHw_SetAfeData(void * pThis, AUD_ADC_ID eAdcId)
{
    bool fgAfe2Sel;

    fgAfe2Sel = (AUD_ADC1 == eAdcId) ? 0 : 1;
    
    AUDREG_BITS_W(REGENV_AFE_TOP_CFG0,
                  BIT_STR_MIC_AFEX_SEL,
                  BIT_NUM_MIC_AFEX_SEL,
                  fgAfe2Sel);
}


//=========================================================//
    #define CodeSight_MicHw_Pubilic_Func
//=========================================================//

/**
 * public interface : mic in configure
 *
 * @param [in]  prThis : hw class; prCfg : mic in hw mode setting
 * @param [out] 
 *
 * @return
 */
static void MicHw_InitCfg(void * pThis, void * pCfg)
{
    PMIC_HW_CLS prThis = (PMIC_HW_CLS)pThis;
    PAUD_MIC_CFG_T prCfg = (PAUD_MIC_CFG_T)pCfg;
    
    u32 u4MicBufSadr, u4Sadr1, u4Sadr2, u4Sadr3;
    u32 u4BlkAdr, u4BankAdr;
    u32 u4ChSize;
    
    PMIC_EXTPARAMS_T prExtCfg = &(prCfg->rExtCfg);
    
    MicHw_SetArmCtrl(prThis, TRUE);
    
    IoClk_Set26mApll();

    MicHw_SetSrc(prThis, prExtCfg->eSrc);
    MicHw_SetClkSrc(prThis, prCfg->eClkSrc);

    if (MIC_CLK_MPH == prCfg->eClkSrc)
    {
        IoClk_SetMphMclk(prCfg->eMclkType, prExtCfg->eFs);
    };    

    MicHw_SetSrcBitNum(prThis, prExtCfg->u4SrcBitNum);
    MicHw_SetOutBitNum(prThis, prExtCfg->eOutBitNum);

    MicHw_SetDataFmt(prThis, prCfg->eDataFmt);

    MicHw_SetBckDivider(prThis, prCfg->eMclkType);
    MicHw_SetLrckDivider(prThis, prCfg->eCycle);
    
    MicHw_SetBckInvert(prThis, prCfg->fgInvertBck);
    MicHw_SetLrckInvert(prThis, prCfg->fgInvertLrck);

    u4MicBufSadr = prCfg->rBuf.u4PhySddr;
    u4ChSize = prCfg->rBuf.u4ChBufSz >> 2;
    
    u4BankAdr = (u4MicBufSadr >> 20) & 0x7ff;
    u4BlkAdr = (u4MicBufSadr & 0x0fff00) >> 8;

    u4Sadr1 = (u4MicBufSadr & 0xff) >> 2;
    u4Sadr2 = u4Sadr1 + u4ChSize;
    u4Sadr3 = u4Sadr2 + u4ChSize;

    if ((u4Sadr2 > 0xffff) || (u4Sadr3 > 0xffff))
    {
        MICLOG_ERR(T("MIC BUF ERROR:  start address exceed 16bit \r\n"));
    }
    
    MicHw_SetSadr(prThis, u4Sadr1, u4Sadr2, u4Sadr3);
    MicHw_SetBlkAdr(prThis, u4BlkAdr);
    MicHw_SetBankAdr(prThis, u4BankAdr);
}
    

/**
 * public interface : enable/disable mic in hw
 *
 * @param [in]  prThis : hw class; fgEnable : enable or disable
 * @param [out] 
 *
 * @return
 */
static void MicHw_Enable(void * pThis, bool fgEnable)
{
    AUDREG_BITS_W(REGENV_RGBK2_CFG3,
                  BIT_STR_MPHONE_BUF_EN,
                  BIT_NUM_MPHONE_BUF_EN,
                  fgEnable);
}

/**
 * mic in hw current write pointer get
 *
 * @param [in]  prThis : hw class
 * @param [out] 
 *
 * @return : current write pointer
 */
static u32 MicHw_GetWp(void * pThis)
{
    u32 u4MicWp, u4MicSadr;

    u4MicWp = AUDREG_READ(REGENV_MPHONE_WADR) << 2;

    u4MicSadr = AUDREG_BITS_R(REGENV_RGBK2_CFG2,
                              BIT_STR_MPBUF1_SADR,
                              BIT_NUM_MPBUF1_SADR);

    u4MicWp = u4MicWp - u4MicSadr;
    
    return u4MicWp;
}


//===========================================//
    #define CodeSight_MicHw_Create
//===========================================//

/**
 * delect a mic in hw object
 *
 * @param [in]  prThis : pointer to the mic in hw object
 * @param [out] 
 *
 * @return      0: OK; others: NG
 */
static u32 MicHw_Delete(void * pThis)
{
    PMIC_HW_CLS prThis = (PMIC_HW_CLS)pThis;
    AUD_CLASS_DELETE();
    
    return (0);
}

/**
 * creat a new mic in  hw object
 *
 * @param [in]
 * @param [out] 
 *
 * @return  pointer to new object
 */
PMIC_HW_CLS_PUB MicHw_New(void)
{
    PMIC_HW_CLS prThis = AUD_CLASS_NEW(MIC_HW_CLS);

    if (prThis)
    {
        prThis->rPub.InitCfg = MicHw_InitCfg; 
        prThis->rPub.Enable = MicHw_Enable;
        prThis->rPub.SelAfe = MicHw_SetAfeData;
        prThis->rPub.GetWp = MicHw_GetWp;
        prThis->rPub.Delete = MicHw_Delete;
    }

    return ((PMIC_HW_CLS_PUB)prThis);
}


