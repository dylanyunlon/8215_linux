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
 * @file aud_bypass_hw.c source file
 * 
 * aud io bypass line in module hardware driver
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */

#include "aud_bypass_hal.h"
#include "aud_reg_env.h"
#include "aud_reg_rgbk2.h"

typedef struct 
{
    BYPS_HW_CLS_PUB rPub;
    AUD_BYPS_DST eDst;

}BYPS_HW_CLS, *PBYPS_HW_CLS;


//=========================================================//
    #define CodeSight_BypsHw_Static_Func
//=========================================================//


/**
 * set bypass hw gain control enable or disable
 *
 * @param [in]  prThis : hw class; fgEn : enbale or disable
 * @param [out] 
 *
 * @return
 */
static void BypsHw_SetGainCtrEn(PBYPS_HW_CLS prThis, bool fgEn)
{
    AUDREG_BITS_W(REGENV_BYPS_VLUM_CFG0, BIT_STR_ADC_BYPS_VOLUME_EN, BIT_NUM_ADC_BYPS_VOLUME_EN, 
                  fgEn);
}

/**
 * set bypss hw gain control mode
 *
 * @param [in]  prThis : hw class; eGainMode : gain mode
 * @param [out] 
 *
 * @return
 */
static void BypsHw_SetGainMode(PBYPS_HW_CLS prThis, AUD_BYPS_GAIN_MODE eGainMode)
{
    AUDREG_BITS_W(REGENV_BYPS_VLUM_CFG0, BIT_STR_ADC_BYPS_MODE_SEL, BIT_NUM_ADC_BYPS_MODE_SEL, 
                  eGainMode);
}

/**
 * set bypss hw scale
 *
 * @param [in]  prThis : hw class; u4Scale : fade in/out order
 * @param [out] 
 *
 * @return
 */
static void BypsHw_SetScale(PBYPS_HW_CLS prThis, u32 u4Scale)
{
    AUDREG_BITS_W(REGENV_BYPS_VLUM_CFG1, BIT_STR_ADC_BYPA_SCALE, BIT_NUM_ADC_BYPA_SCALE, 
                  u4Scale);
}

/**
 * set bypss hw gain
 *
 * @param [in]  prThis : hw class; u4Gain : max value is 0xffffff
 * @param [out] 
 *
 * @return
 */
static void BypsHw_SetGain(void * pThis, u32 u4Gain)
{
    PBYPS_HW_CLS prThis = (PBYPS_HW_CLS)pThis;
    
    BypsHw_SetGainCtrEn(prThis, FALSE);
    
    AUDREG_BITS_W(REGENV_BYPS_VLUM_CFG0, BIT_STR_ADC_BYPS_VOLUME, BIT_NUM_ADC_BYPS_VOLUME, 
                  u4Gain);

    BypsHw_SetGainCtrEn(prThis, TRUE);
}

/**
 * set bypass hw fade in enable or disable
 *
 * @param [in]  prThis : hw class; fgEn : enbale or disable
 * @param [out] 
 *
 * @return
 */
static void BypsHw_SetFadeInEn(PBYPS_HW_CLS prThis, bool fgEn)
{
    AUDREG_BITS_W(REGENV_BYPS_VLUM_CFG0, BIT_STR_ADC_BYPS_FDIN, BIT_NUM_ADC_BYPS_FDIN, 
                  fgEn);
}

/**
 * set bypass hw fade out enable or disable
 *
 * @param [in]  prThis : hw class; fgEn : enbale or disable
 * @param [out] 
 *
 * @return
 */
static void BypsHw_SetFadeOutEn(PBYPS_HW_CLS prThis, bool fgEn)
{
    AUDREG_BITS_W(REGENV_BYPS_VLUM_CFG0, BIT_STR_ADC_BYPS_FDOUT, BIT_NUM_ADC_BYPS_FDOUT, 
                  fgEn);
}

/**
 * set bypss hw input source
 *
 * @param [in]  prThis : hw class; eAdcId : adc1 or adc2
 * @param [out] 
 *
 * @return
 */
static void BypsHw_SetSrc(void * pThis, AUD_ADC_ID eAdcId)
{
    AUDREG_BITS_W(REGENV_BYPS_VLUM_CFG0, BIT_STR_VOLUME_SRC_SEL, BIT_NUM_VOLUME_SRC_SEL, 
                  eAdcId);
}

/**
 * set bypass hw output destination
 *
 * @param [in]  prThis : hw class; eAoutId : bypass to aout1 or aout2; fgBypsMode
 * @param [out] 
 *
 * @return
 */
static void BypsHw_SetDst(PBYPS_HW_CLS prThis, AUD_BYPS_DST eDst, bool fgBypsMode)
{
    if (BYPS_DST_AOUT1 == eDst)
    {
        AUDREG_BITS_W(REGENV_AFE_TOP_CFG0, BIT_STR_AOUT1_BYPA_ADC, BIT_NUM_AOUT1_BYPA_ADC, 
                      fgBypsMode);
    }
    else 
    {
        AUDREG_BITS_W(REGENV_AFE_TOP_CFG0, BIT_STR_AOUT2_BYPA_ADC, BIT_NUM_AOUT2_BYPA_ADC, 
                      fgBypsMode);
    }
}


//=========================================================//
    #define CodeSight_BypsHw_Pubilic_Func
//=========================================================//


/**
 * public interface : bypass line in configure
 *
 * @param [in]  prThis : hw class; prCfg : bypass linein hw mode setting
 * @param [out] 
 *
 * @return
 */
static void BypsHw_InitCfg(void * pThis, void * pCfg)
{
    PBYPS_HW_CLS prThis = (PBYPS_HW_CLS)pThis;
    PAUD_BYPS_CFG_T prCfg = (PAUD_BYPS_CFG_T)pCfg;
    PBYPS_EXTPARAMS_T prExtCfg = &(prCfg->rExtCfg);

    BypsHw_SetGainMode(prThis, prExtCfg->eGainMode);
    BypsHw_SetScale(prThis, prExtCfg->u4Scale);
    BypsHw_SetGain(prThis, prExtCfg->u4Gain);

    BypsHw_SetDst(prThis, prExtCfg->eDst, TRUE);

    prThis->eDst = prExtCfg->eDst;
}

/**
 * public interface : bypass line in hw enable/disable
 *
 * @param [in]  prThis : hw class; fgEn : enable or disable
 * @param [out] 
 *
 * @return
 */
static void BypsHw_Enable(void * pThis, bool fgEn)
{
    PBYPS_HW_CLS prThis = (PBYPS_HW_CLS)pThis;
    
    if (fgEn)
    {
        BypsHw_SetFadeInEn(prThis, TRUE);        
        Sleep(10);
        BypsHw_SetFadeInEn(prThis, FALSE);
    }
    else
    {
        BypsHw_SetFadeOutEn(prThis, TRUE);        
        Sleep(10);
        BypsHw_SetFadeOutEn(prThis, FALSE);
    }
}

//===========================================//
    #define CodeSight_BypsHw_Create
//===========================================//

/**
 * delect a bypass hw object
 *
 * @param [in]  prThis : pointer to the bypass hw object
 * @param [out] 
 *
 * @return      0: OK; others: NG
 */
static u32 BypsHw_Delete(void * pThis)
{
    PBYPS_HW_CLS prThis = (PBYPS_HW_CLS)pThis;
    
    BypsHw_SetDst(prThis, prThis->eDst, FALSE);
    
    AUD_CLASS_DELETE();
    
    return (0);
}

/**
 * creat a new bypass hw object
 *
 * @param [in]
 * @param [out] 
 *
 * @return  pointer to new object
 */
PBYPS_HW_CLS_PUB BypsHw_New(void)
{
    PBYPS_HW_CLS prThis = AUD_CLASS_NEW(BYPS_HW_CLS);

    if (prThis)
    {
        prThis->rPub.InitCfg = BypsHw_InitCfg; 
        prThis->rPub.Enable = BypsHw_Enable;
        prThis->rPub.SetGain = BypsHw_SetGain;
        prThis->rPub.SelAfe = BypsHw_SetSrc;
        
        prThis->rPub.Delete = BypsHw_Delete;
    }

    return ((PBYPS_HW_CLS_PUB)prThis);
}


