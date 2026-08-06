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
 * @file aud_linein_hw.c source file
 * 
 * aud io linein module hardware driver
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */

#include "aud_linein_hal.h"
#include "aud_reg_env.h"
#include "aud_reg_rgbk2.h"
#include "aud_io_clock_if.h"

typedef struct 
{
    LIN_HW_CLS_PUB rPub;
    
    AUD_LIN_DEVID eLinId;

}LIN_HW_CLS, *PLIN_HW_CLS;

typedef enum
{
    LIN_REG_EXTDATA_SEL,
    LINE_REG_EXTCLK_SEL,
    LINE_REG_A2CLK_SEL,
    LINE_REG_MLINCLK_SEL,
    LINE_REG_BIT_NUM,
    LINE_REG_24BIT_MODE,
    LINE_REG_DATA_ORDER,
    LINE_REG_LEFT_ALIGN,
    LINE_REG_DATA_DELAY,
    LINE_REG_SADR,
    LINE_REG_EADR,
    LINE_REG_BANK_ADR,
    LINE_REG_BCK_DIV,
    LINE_REG_LRCK_DIV,
    LINE_REG_BCK_INV,
    LINE_REG_LRCK_INV,
    LINE_REG_EN,
    LINE_REG_WP,
    LINE_REG_AFE_SEL,
    LINE_REG_INT_PERIOD,
    LIN_REG_IDX_MAX,
}LIN_CONFIG_REG_IDX;


static AUD_IO_REG_CTL aLinConfigReg[AUDID_LIN_MAX][LIN_REG_IDX_MAX] = {
    {
        {REGENV_BYPS_VLUM_CFG1, BIT_STR_LIN_AFE_MODE, BIT_NUM_LIN_AFE_MODE, "Lin AFE_MODE"},//LIN_REG_EXTDATA_SEL
        {REGENV_AIN_CFG, BIT_STR_SPL_SEL, BIT_NUM_SPL_SEL, "Lin Ext clock sel"}, //LINE_REG_EXTCLK_SEL
        {REGENV_AFE_TOP_CFG0, BIT_STR_LIN_USE_AO2_GIM, BIT_NUM_LIN_USE_AO2_GIM, "Lin Aout2 clock sel"}, //LINE_REG_A2CLK_SEL
        {REGENV_RGBK2_CFG4, BIT_STR_SEL_MLINCKGEN, BIT_NUM_SEL_MLINCKGEN, "Lin Mlin clk sel"}, //LINE_REG_MLINCLK_SEL
        {REGENV_AIN_CFG, BIT_STR_SPL_BNUM, BIT_NUM_SPL_BNUM, "Lin bit Number"}, //LINE_REG_BIT_NUM
        {REGENV_SPLIN_CTRL, BIT_STR_SPLIN_BIT24, BIT_NUM_SPLIN_BIT24, "Lin 24bit mode"}, //LINE_REG_24BIT_MODE
        {REGENV_SPLIN_CTRL, BIT_STR_SPLIN_SWAP, BIT_NUM_SPLIN_SWAP, "Lin data order"}, //LINE_REG_DATA_ORDER
        {REGENV_AIN_CFG, BIT_AIN_STR_LEFT_ALN, BIT_AIN_NUM_LEFT_ALN, "Lin left align"}, //LINE_REG_LEFT_ALIGN
        {REGENV_AIN_CFG, BIT_AIN_STR_DAT_DLY, BIT_AIN_NUM_DAT_DLY, "Lin Data Delay"}, //LINE_REG_DATA_DELAY
        {REGENV_SPLIN_BLK, BIT_STR_SPLIN_SBLK, BIT_NUM_SPLIN_SBLK, "Lin start adr"}, //LINE_REG_SADR
        {REGENV_SPLIN_BLK, BIT_STR_SPLIN_EBLK, BIT_NUM_SPLIN_EBLK, "Lin end adr"}, //LINE_REG_EADR
        {REGENV_SPLIN_CTRL, BIT_STR_SPDFLIN_BANK, BIT_NUM_SPDFLIN_BANK, "Lin bank Adr"}, //LINE_REG_BANK_ADR
        {REGENV_RGBK2_CFG4, BIT_STR_MLIN_BCK_DIV, BIT_NUM_MLIN_BCK_DIV, "Lin Bck div"}, //LINE_REG_BCK_DIV
        {REGENV_AIN_CFG, BIT_AIN_STR_LRCK_CYC, BIT_AIN_NUM_LRCK_CYC, "Lin Lrck Div"}, //LINE_REG_LRCK_DIV
        {REGENV_AENV_BAK, BIT_STR_AIN_INV_BCK, BIT_NUM_AIN_INV_BCK, "Lin Bck Inv"}, //LINE_REG_BCK_INV
        {REGENV_AIN_CFG, BIT_STR_AIN_INV_LRCK, BIT_NUM_AIN_INV_LRCK, "Lin lrck inv"}, //LINE_REG_LRCK_INV
        {REGENV_SPLIN_CTRL, BIT_STR_SPLIN_EN, BIT_NUM_SPLIN_EN, "lin enable"}, //LINE_REG_EN
        {REGENV_SPLIN_WRADR, BIT_STR_LIN_WP, BIT_NUM_LIN_WP, "lin write pointer"}, //LINE_REG_WP
        {REGENV_AFE_TOP_CFG0, BIT_STR_LIN_AFEX_SEL, BIT_NUM_LIN_AFEX_SEL, "lin afe sel"}, //LINE_REG_AFE_SEL
        {REGENV_SPLIN_CTRL, BIT_STR_SPLIN_INT_PRD, BIT_NUM_SPLIN_INT_PRD, "lin s32 period"}, //LINE_REG_INT_PERIOD
    },
    {
        {REGENV_AFE_TOP_CFG0, BIT_STR_LIN2_AFE_MODE, BIT_NUM_LIN2_AFE_MODE, "Lin2 AFE_MODE"},
        {REGENV_AIN_CFG_LIN2, BIT_STR_LIN2_SPL_SEL , BIT_NUM_LIN2_SPL_SEL , "Lin2 Ext clock sel"},
        {REGENV_AIN_CFG_LIN2, BIT_STR_LIN2_USE_AO2_TIM, BIT_NUM_LIN2_USE_AO2_TIM, "Lin2 Aout2 clock sel"},
        {REGENV_AIN_CFG_LIN2, BIT_STR_LIN2_SEL_MLINCKGEN, BIT_NUM_LIN2_SEL_MLINCKGEN, "Lin2 Mlin clk sel"},
        {REGENV_AIN_CFG_LIN2, BIT_STR_LIN2_BNUM, BIT_NUM_LIN2_BNUM, "Lin2 bit Number"},
        {REGENV_SPLIN_CTL_LIN2, BIT_STR_LIN2_BIT24, BIT_NUM_LIN2_BIT24, "Lin2 24bit mode"},
        {REGENV_SPLIN_CTL_LIN2, BIT_STR_LIN2_SWAP, BIT_NUM_LIN2_SWAP, "Lin2 data order"},
        {REGENV_AIN_CFG_LIN2, BIT_STR_LIN2_LEFT_ALN, BIT_NUM_LIN2_LEFT_ALN, "Lin2 left align"},
        {REGENV_AIN_CFG_LIN2, BIT_STR_LIN2_DAT_DLY, BIT_NUM_LIN2_DAT_DLY, "Lin2 Data Delay"},
        {REGENV_SPLIN_BLK_LIN2, BIT_STR_SPLIN2_SBLK, BIT_NUM_SPLIN2_SBLK, "Lin2 start adr"},
        {REGENV_SPLIN_BLK_LIN2, BIT_STR_SPLIN2_EBLK, BIT_NUM_SPLIN2_EBLK, "Lin2 end adr"},
        {REGENV_SPLIN_CTRL, BIT_STR_SPDFLIN_BANK, BIT_NUM_SPDFLIN_BANK, "Lin2 bank Adr"},
        {REGENV_AIN_CFG_LIN2, BIT_STR_LIN2_BCK_DIV, BIT_NUM_LIN2_BCK_DIV, "Lin Bck div"},
        {REGENV_AIN_CFG_LIN2, BIT_STR_LIN2_LRCK_CYC, BIT_NUM_LIN2_LRCK_CYC, "Lin2 Lrck Div"},
        {REGENV_AIN_CFG_LIN2, BIT_STR_LIN2_BCK_INV, BIT_NUM_LIN2_BCK_INV, "Lin2 Bck Inv"},
        {REGENV_AIN_CFG_LIN2, BIT_STR_LIN2_INV_LRCK, BIT_NUM_LIN2_INV_LRCK, "Lin2 lrck inv"},
        {REGENV_SPLIN_CTL_LIN2, BIT_STR_LIN2_EN, BIT_NUM_LIN2_EN, "Lin2 enable"},
        {REGENV_SPLIN2_WRADR, BIT_STR_LIN_WP, BIT_NUM_LIN_WP, "lin2 write pointer"},
        {REGENV_AFE_TOP_CFG0, BIT_STR_LIN2_AFEX_SEL, BIT_NUM_LIN2_AFEX_SEL, "lin2 afe sel"},
        {REGENV_SPLIN_CTL_LIN2, BIT_STR_LIN2_INTR_PERIOD, BIT_NUM_LIN2_INTR_PERIOD, "lin2 s32 period"},
    }
};


//macros for line in configure regosters read / write
#define LIN_REGCFG_BITS_W(DevId, CfgRegId, val) \
    AUDREG_BITS_W((aLinConfigReg[DevId][CfgRegId].u4Adr), (aLinConfigReg[DevId][CfgRegId].u4BitStart), \
                  (aLinConfigReg[DevId][CfgRegId].u4BitNum), val)
#define LIN_REGCFG_BITS_R(DevId, CfgRegId) \
    AUDREG_BITS_R((aLinConfigReg[DevId][CfgRegId].u4Adr), (aLinConfigReg[DevId][CfgRegId].u4BitStart),\
                  (aLinConfigReg[DevId][CfgRegId].u4BitNum))


//=========================================================//
    #define CodeSight_LinHw_Static_Func
//=========================================================//


/**
 * set linein data source
 *
 * @param [in]  prThis : hw class; eSrc : s32 or ext linein
 * @param [out] 
 *
 * @return
 */
static void LinHw_SetDataSrc(PLIN_HW_CLS prThis, AUD_LIN_SRC eSrc)
{
    bool fgExtAdc = (EXT_LINEIN == eSrc) ? LIN_EXT_ADC : LIN_INT_ADC;
    LIN_REGCFG_BITS_W(prThis->eLinId, LIN_REG_EXTDATA_SEL, fgExtAdc);
}

/**
 * set linein clock source
 *
 * @param [in]  prThis : hw class; eSrc : s32 or ext linein; eIntClkSrc : s32 clk type; eClkMode : master or slave
 * @param [out] 
 *
 * @return
 */
static void LinHw_SetClkSrc(PLIN_HW_CLS prThis, AUD_LIN_SRC eSrc, AUD_LIN_CLK_SRC eIntClkSrc, AUD_CLK_MODE eClkMode)
{
    bool fgExtClkSel, fgA2ClkSel, fgMlinClkSel;

    fgExtClkSel = ((EXT_LINEIN == eSrc) && (AUD_SLAVE_MODE == eClkMode)) ? LIN_EXT_CLK : LIN_INT_CLK;
    fgA2ClkSel = (LIN_CLK_AOUT2 == eIntClkSrc) ? TRUE : FALSE;
    fgMlinClkSel = (LIN_CLK_MLIN == eIntClkSrc) ? TRUE : FALSE;

    LIN_REGCFG_BITS_W(prThis->eLinId, LINE_REG_EXTCLK_SEL, fgExtClkSel);
    LIN_REGCFG_BITS_W(prThis->eLinId, LINE_REG_A2CLK_SEL, fgA2ClkSel);
    LIN_REGCFG_BITS_W(prThis->eLinId, LINE_REG_MLINCLK_SEL, fgMlinClkSel);    
}

/**
 * set AfeX for lin
 *
 * @param [in]  prThis : hw class; eAdcId : AFe ID
 * @param [out] 
 *
 * @return
 */
static void LinHw_SetAfeData(void * pThis, AUD_ADC_ID eAdcId)
{
    PLIN_HW_CLS prThis = (PLIN_HW_CLS)pThis;
    bool fgAfe2Sel;

    fgAfe2Sel = (AUD_ADC1 == eAdcId) ? 0 : 1;

    LIN_REGCFG_BITS_W(prThis->eLinId, LINE_REG_AFE_SEL, fgAfe2Sel); 
}


/**
 * set linein input data bit number
 *
 * @param [in]  prThis : hw class; u4BitNum : bit number of input data
 * @param [out] 
 *
 * @return
 */
static void LinHw_SetSrcBitNum(PLIN_HW_CLS prThis, u32 u4BitNum)
{
    LIN_REGCFG_BITS_W(prThis->eLinId, LINE_REG_BIT_NUM, (u4BitNum - 1));
}

/**
 * set linein output data bit mode
 *
 * @param [in]  prThis : hw class; eOutBitNum : 24bit or 16bit
 * @param [out] 
 *
 * @return
 */
static void LinHw_SetOutBitNum(PLIN_HW_CLS prThis, AUD_LIN_OUT_BITNUM eOutBitNum)
{
    LIN_REGCFG_BITS_W(prThis->eLinId, LINE_REG_24BIT_MODE, eOutBitNum);
}

/**
 * set linein output 16bits data format
 *
 * @param [in]  prThis : hw class; e16BitFmt : left/right alignment
 * @param [out] 
 *
 * @return
 */
static void LinHw_Set16BitFmt(PLIN_HW_CLS prThis, AUD_LIN_16BIT_FMT e16BitFmt)
{
    LIN_REGCFG_BITS_W(prThis->eLinId, LINE_REG_DATA_ORDER, e16BitFmt);
}

/**
 * set linein data format
 *
 * @param [in]  prThis : hw class; eFmt : data format
 * @param [out] 
 *
 * @return
 */
static void LinHw_SetDataFmt(PLIN_HW_CLS prThis, AUDFMT_INTF_E eFmt)
{
    LIN_REGCFG_BITS_W(prThis->eLinId, LINE_REG_LEFT_ALIGN, (eFmt & 0x1));
    LIN_REGCFG_BITS_W(prThis->eLinId, LINE_REG_DATA_DELAY, ((eFmt & 0x2) >> 1));
}

/**
 * set linein start address
 *
 * @param [in]  prThis : hw class; u4BankAdr : start address
 * @param [out] 
 *
 * @return
 */
static void LinHw_SetSadr(PLIN_HW_CLS prThis, u32 u4Sadr)
{
    LIN_REGCFG_BITS_W(prThis->eLinId, LINE_REG_SADR, u4Sadr);
}

/**
 * set linein end address
 *
 * @param [in]  prThis : hw class; u4BankAdr : end address
 * @param [out] 
 *
 * @return
 */
static void LinHw_SetEadr(PLIN_HW_CLS prThis, u32 u4Eadr)
{
    LIN_REGCFG_BITS_W(prThis->eLinId, LINE_REG_EADR, u4Eadr);
}

/**
 * set linein bank address
 *
 * @param [in]  prThis : hw class; u4BankAdr : bank address
 * @param [out] 
 *
 * @return
 */
static void LinHw_SetBankAdr(PLIN_HW_CLS prThis, u32 u4BankAdr)
{
    LIN_REGCFG_BITS_W(prThis->eLinId, LINE_REG_BANK_ADR, u4BankAdr);
}

/**
 * set linein mck to bck ratio
 *
 * @param [in]  prThis : hw class; eMclkType : mclk type
 * @param [out] 
 *
 * @return
 */
static void LinHw_SetBckDivider(PLIN_HW_CLS prThis, MCLK_TYPE_T eMclkType, AUD_LRCK_CYC_T eCycle)
{
    u32 u4MclkToBckRatio, u4BckToFsRatio;

    u4BckToFsRatio = (AUD_LRCK_CYC_32 ==  eCycle) ? 32 : ((AUD_LRCK_CYC_24 ==  eCycle) ? 24 : 16);
    u4MclkToBckRatio = IoClk_GetMclkToFsRatio(eMclkType);
    u4MclkToBckRatio /= (u4BckToFsRatio * 4);

    LIN_REGCFG_BITS_W(prThis->eLinId, LINE_REG_BCK_DIV, u4MclkToBckRatio);
}

/**
 * set linein bck to lrck ratio
 *
 * @param [in]  prThis : hw class; eCycle : lrck type
 * @param [out] 
 *
 * @return
 */
static void LinHw_SetLrckDivider(PLIN_HW_CLS prThis, AUD_LRCK_CYC_T eCycle)
{
    LIN_REGCFG_BITS_W(prThis->eLinId, LINE_REG_LRCK_DIV, eCycle);
}

/**
 * set bck invert
 *
 * @param [in]  prThis : hw class; fgInvert : invert or not
 * @param [out] 
 *
 * @return
 */
static void LinHw_SetBckInvert(PLIN_HW_CLS prThis, bool fgInvert)
{
    LIN_REGCFG_BITS_W(prThis->eLinId, LINE_REG_BCK_INV, fgInvert);
}

/**
 * set lrck invert
 *
 * @param [in]  prThis : hw class; fgInvert : invert or not
 * @param [out] 
 *
 * @return
 */
static void LinHw_SetLrckInvert(PLIN_HW_CLS prThis, bool fgInvert)
{
    LIN_REGCFG_BITS_W(prThis->eLinId, LINE_REG_LRCK_INV, fgInvert);
}

/**
 * set s32 period
 *
 * @param [in]  prThis : hw class; fgInvert : invert or not
 * @param [out] 
 *
 * @return
 */
static void LinHw_SetIntPeriod(PLIN_HW_CLS prThis, AUD_LIN_INT_PERIOD eIntPeriod)
{
    LIN_REGCFG_BITS_W(prThis->eLinId, LINE_REG_INT_PERIOD, eIntPeriod);
}


//=========================================================//
    #define CodeSight_LinHw_Pubilic_Func
//=========================================================//


/**
 * public interface : line in configure
 *
 * @param [in]  prThis : hw class; prCfg : linein hw mode setting
 * @param [out] 
 *
 * @return
 */
static void LinHw_InitCfg(void * pThis, void * pCfg)
{
    PLIN_HW_CLS prThis = (PLIN_HW_CLS)pThis;
    PAUD_LIN_CFG_T prCfg = (PAUD_LIN_CFG_T)pCfg;
        
    uintptr_t u4PhySadr, u4PhyEadr, u4BankAdr;
    PLIN_FMT_SETTING_T prFmt;
    
    PLIN_EXTPARAMS_T prExtCfg = &(prCfg->rExtCfg);
    
    LinHw_SetDataSrc(prThis, prExtCfg->eSrc);
    LinHw_SetClkSrc(prThis, prExtCfg->eSrc, prExtCfg->eIntClkSrc, prExtCfg->rI2sLinCfg.eClkMode);
    
    prFmt = (INT_LINEIN == prExtCfg->eSrc) ? &(prCfg->rFmt) : &(prExtCfg->rI2sLinCfg.rFmt);

    LinHw_SetSrcBitNum(prThis, prFmt->u4SrcBitNum);
    LinHw_SetOutBitNum(prThis, prFmt->eOutBitNum);
    LinHw_SetDataFmt(prThis, prFmt->eDataFmt);
    if(INT_LINEIN == prExtCfg->eSrc)
    {
        LinHw_Set16BitFmt(prThis, LALIGN_16BIT);
    }
    else
    {       
        LinHw_Set16BitFmt(prThis, RALIGN_16BIT);
    }

    LinHw_SetBckDivider(prThis, prFmt->eMclkType, prFmt->eCycle);
    LinHw_SetLrckDivider(prThis, prFmt->eCycle);

    LinHw_SetBckInvert(prThis, prFmt->fgInvertBck);
    LinHw_SetLrckInvert(prThis, prFmt->fgInvertLrck);

    if (LIN_CLK_MLIN == prExtCfg->eIntClkSrc)
    {
        if (AUDID_LIN1 == prThis->eLinId)
        {
            IoClk_SetLinMclk(prFmt->eMclkType, prFmt->eFs);
        }
        else
        {
            IoClk_SetLin2Mclk(prFmt->eMclkType, prFmt->eFs);
        }
    }

    
    if (TRUE == prExtCfg->rIntCfg.fgOn)
    {
        LinHw_SetIntPeriod(prThis, prExtCfg->rIntCfg.eIntPeriod);
    }

    u4PhySadr = prCfg->rBuf.u4PhySddr;
    u4PhyEadr = prCfg->rBuf.u4PhySddr + (prCfg->rBuf.u4ChBufSz * prCfg->rBuf.u4Chn);
    u4BankAdr = (prCfg->rBuf.u4PhySddr >> 20) & 0x7ff;
     LINLOG_ERR((T(" LinHw_InitCfg u4BankAdr(0x%lx)\r\n"),u4BankAdr));
    LinHw_SetSadr(prThis, (u32)(((u4PhySadr - (u4BankAdr << 20)) & 0x00FFFF00) >> 8));
    LinHw_SetEadr(prThis, (u32)(((u4PhyEadr - (u4BankAdr << 20)) & 0x00FFFF00) >> 8));
    LinHw_SetBankAdr(prThis, (u32)u4BankAdr);

	LINLOG_ERR((T(" LinHw_InitCfg(0x%lx)\r\n"),(u32)(((u4PhySadr - (u4BankAdr << 20)) & 0x00FFFF00) >> 8)));
}

/**
 * public interface : enable/disable line in hw
 *
 * @param [in]  prThis : hw class; fgEnable : enable or disable
 * @param [out] 
 *
 * @return
 */
static void LinHw_Enable(void * pThis, bool fgEnable)
{
    PLIN_HW_CLS prThis = (PLIN_HW_CLS)pThis;
    
    LIN_REGCFG_BITS_W(prThis->eLinId, LINE_REG_EN, fgEnable);
}

/**
 * line in hw current write pointer get
 *
 * @param [in]  prThis : hw class
 * @param [out] 
 *
 * @return : current pointer
 */
static u32 LinHw_GetWp(void * pThis)
{
    PLIN_HW_CLS prThis = (PLIN_HW_CLS)pThis;
    
    u32 u4LinWp = LIN_REGCFG_BITS_R(prThis->eLinId, LINE_REG_WP)<<2;
    u32 u4LinSa = LIN_REGCFG_BITS_R(prThis->eLinId, LINE_REG_SADR)<<8; 
	
    u4LinWp = u4LinWp  - u4LinSa;
    return u4LinWp;
}


//===========================================//
    #define CodeSight_LinHw_Create
//===========================================//

/**
 * delect a line in hw object
 *
 * @param [in]  prThis : pointer to the line in hw object
 * @param [out] 
 *
 * @return      0: OK; others: NG
 */
static u32 LinHw_Delete(void * pThis)
{
    PLIN_HW_CLS prThis = (PLIN_HW_CLS)pThis;
    
    AUD_CLASS_DELETE();
    
    return (0);
}

/**
 * creat a new line in  hw object
 *
 * @param [in]  eLinId : linein1 or linein2
 * @param [out] 
 *
 * @return  pointer to new object
 */
PLIN_HW_CLS_PUB LinHw_New(AUD_LIN_DEVID eLinId)
{
    PLIN_HW_CLS prThis = AUD_CLASS_NEW(LIN_HW_CLS);

    if (prThis)
    {
        prThis->eLinId = eLinId;

        prThis->rPub.InitCfg = LinHw_InitCfg; 
        prThis->rPub.Enable = LinHw_Enable;
        prThis->rPub.SelAfe = LinHw_SetAfeData;
        prThis->rPub.GetWp = LinHw_GetWp;
        prThis->rPub.Delete = LinHw_Delete;
    }

    return ((PLIN_HW_CLS_PUB)prThis);
}


