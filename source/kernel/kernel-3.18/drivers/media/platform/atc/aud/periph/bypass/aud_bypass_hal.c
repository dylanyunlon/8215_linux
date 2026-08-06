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
 * @file aud_bypass_hal.c source file
 *
 * aud io bypass line in module hardware abstraction layer
 *
 * @author qiuhua.yin@autochips.com
 *
 */

#include "aud_bypass_hal.h"
#include "aud_if.h"

u32 _u4BypsLog = ALOG_DEFAULT;

//==============================================//
    #define CodeSight_BypsHal_static_FUNC
//==============================================//

/**
 * external configure parameters check
 *
 * @param [in]  prThis : hal class, prCfg : pointer to external configure parameters
 * @param [out]
 *
 * @return 0: check ok , 2 : check fail
 */
static u32 BypsHal_ExtConfigCheck(PBYPS_HAL_CLS prThis, PBYPS_EXTPARAMS_T prCfg)
{
    u32 u4RetVal = AUD_RET_OK;

    PBYPS_EXTPARAMS_T prExtCfg = &(prThis->rCfg.rExtCfg);

    AUDOS_MEMCPY(prExtCfg, prCfg, sizeof(BYPS_EXTPARAMS_T));

    if (prExtCfg->eDst > BYPS_DST_AOUT2)
    {
        BYPSLOG_ERR(T("Byps EXTPARA ERR: out destination (0x%x) \r\n"), prExtCfg->eDst);
        prExtCfg->eDst = BYPS_DST_AOUT2;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    if (prExtCfg->eGainMode > BYPS_GAIN_LINER)
    {
        BYPSLOG_ERR(T("Byps EXTPARA ERR: gain mode (0x%x) \r\n"), prExtCfg->eGainMode);
        prExtCfg->eGainMode = BYPS_GAIN_LINER;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    if ((prExtCfg->eGroup < ADC_SRC_LIN1) || (prExtCfg->eGroup > ADC_SRC_LIN5))
    {
        BYPSLOG_ERR(T("Byps EXTPARA ERR: Input group (0x%x) \r\n"), prExtCfg->eGroup);
        prExtCfg->eGroup = ADC_SRC_LIN1;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    if (prExtCfg->u4Gain > 0xffffff)
    {
        BYPSLOG_ERR(T("Byps EXTPARA ERR: Gain > Max value (0x%x) \r\n"), (u32)(prExtCfg->u4Gain));
        prExtCfg->u4Gain = 0xffffff;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    if (prExtCfg->u4Scale > 15)
    {
        BYPSLOG_ERR(T("Byps EXTPARA ERR: scalse > Max value (0x%x) \r\n"), (u32)(prExtCfg->u4Scale));
        prExtCfg->u4Scale = 1;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    return (u4RetVal);
}

/**
 * internal configure parameters init
 *
 * @param [in]  prThis : pointer to hal object
 * @param [out]
 *
 * @return
 */
static void BypsHal_IntConfigInit(PBYPS_HAL_CLS prThis)
{
    PAUD_BYPS_CFG_T prCfg = &(prThis->rCfg);

    prCfg->rBuf.u4PhySddr = 0;
    prCfg->rBuf.u4VirSAdr = 0;
}

/**
 * internal configure parameters check
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return 0: check ok , 2 : check fail
 */
static u32 BypsHal_IntConfigCheck(PBYPS_HAL_CLS prThis)
{
    u32 u4RetVal = AUD_RET_OK;

    return (u4RetVal);
}

/**
 * internal adc ext config init
 *
 * @param [in]
 * @param [out]
 *
 * @return
 */
static void BypsHal_AdcCfgInit(PBYPS_HAL_CLS prThis)
{
    PBYPS_EXTPARAMS_T prExtCfg = &(prThis->rCfg.rExtCfg);
    PADC_EXTPARAMS_T prAdcCfg = &(prThis->rCfg.rAdcExtCfg);

    prAdcCfg->eFs = FS_48K;
    prAdcCfg->eInput = prExtCfg->eGroup;

    prAdcCfg->eClkSrc = (BYPS_DST_AOUT1 == prExtCfg->eDst) ? AFE_CLK_AOUT1 : AFE_CLK_AOUT2;

    prAdcCfg->eLinGain = LSBUFGAIN_MINUS_3DB;
    prAdcCfg->u4MicGain = 14; //no use here
}


//==============================================//
    #define CodeSight_BypsHal_IF_FLOW
//==============================================//

/**
 * Hal interfac : byps hw setup
 *
 * @param [in]  prThis : hal class, prCfg : pointer to external config parameters variable
 * @param [out]
 *
 * @return 0: setup ok , 2 : setup fail
 */
static u32 BypsHal_Setup(void * pThis, void * pCfg)
{
    PBYPS_HAL_CLS prThis = (PBYPS_HAL_CLS)pThis;
    PBYPS_EXTPARAMS_T prCfg = (PBYPS_EXTPARAMS_T)pCfg;
    u32 u4RetVal = AUD_RET_OK;

    PAUD_BYPS_CFG_T prBypsCfg = &(prThis->rCfg);

    //external parameters check
    u4RetVal = BypsHal_ExtConfigCheck(prThis, prCfg);

    //internal parameters init
    BypsHal_IntConfigInit(prThis);

    //internal parameters check
    u4RetVal = BypsHal_IntConfigCheck(prThis);

    //bypass hw adc config init
    BypsHal_AdcCfgInit(prThis);

    //call bypass init function
    prThis->prBypsHw->InitCfg(prThis->prBypsHw, prBypsCfg);

    prThis->u4State = AUD_STATE_INITED;

    return (u4RetVal);
}

/**
 * Hal interfac : get bypass hw status
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  hw status
 */
static u32 BypsHal_GetStatus(void * pThis)
{
    PBYPS_HAL_CLS prThis = (PBYPS_HAL_CLS)pThis;

    return (prThis->u4State);
}

/**
 * Hal interfac : enable bypass hw
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  hw status
 */
static u32 BypsHal_Start(void * pThis, u32 u4Params)
{
    PBYPS_HAL_CLS prThis = (PBYPS_HAL_CLS)pThis;
    bool fgEn = TRUE;

    BYPSLOG_INFO(T("BypsHal_Start \r\n"));

    prThis->u4State = AUD_STATE_STARTED;

    prThis->prAdc = AdcHal_New(&(prThis->rCfg.rAdcExtCfg));

    if (NULL == prThis->prAdc)
    {
        BYPSLOG_ERR(T("No free adc for bypass hw use \r\n"));

        fgEn = FALSE;
        prThis->u4State = AUD_STATE_STOPPED;
    }
    else
    {
        BYPSLOG_INFO(T("Alocate ADC(%d) for Bypass hw \r\n"), prThis->prAdc->eAdcId);

        prThis->rCfg.eAdcId = prThis->prAdc->eAdcId;
        prThis->prAdc->rHwIf.Start(prThis->prAdc, 0);
        prThis->prBypsHw->SelAfe(prThis->prBypsHw, prThis->rCfg.eAdcId);
    }

    prThis->prBypsHw->Enable(prThis->prBypsHw, fgEn);

    return (prThis->u4State);
}

/**
 * Hal interface : disable bypass hw
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  hw status
 */
static u32 BypsHal_Stop(void * pThis, u32 u4Params)
{
    PBYPS_HAL_CLS prThis = (PBYPS_HAL_CLS)pThis;
    BYPSLOG_INFO(T("BypsHal_Stop \r\n"));

    if (AUD_STATE_STOPPED == prThis->u4State)
    {
        BYPSLOG_ERR(T("BypsHal_Stop Stopped already!!\r\n"));
        return (prThis->u4State);
    }

    prThis->prBypsHw->Enable(prThis->prBypsHw, FALSE);

    if (NULL != prThis->prAdc)
    {
        prThis->prAdc->rHwIf.Stop(prThis->prAdc, 0);
        prThis->prAdc->Delete(prThis->prAdc);
    }



    prThis->u4State = AUD_STATE_STOPPED;

    return (prThis->u4State);
}


//==============================================//
    #define CodeSight_BypsHal_IF_BUF
//==============================================//

/**
 * Hal interface : get bypass buffer config
 *
 * @param [in]  prThis : hal class
 * @param [out] prBuf : buffer information
 *
 * @return  hw status
 */
static u32 BypsHal_GetBuf(void * pThis, AUD_DATA_BUF_T *prBuf)
{
    PBYPS_HAL_CLS prThis = (PBYPS_HAL_CLS)pThis;

    x_memcpy(prBuf, &(prThis->rCfg.rBuf), sizeof(AUD_DATA_BUF_T));

    return (AUD_RET_OK);
}

/**
 * Hal interface : set bypass buffer read pointer
 *
 * @param [in]  prThis : hal class, u4Rp : current read pointer
 * @param [out]
 *
 * @return  0 : set ok, 1 : set fail
 */
static u32 BypsHal_SetPoint(void * pThis, u32 u4Rp)
{
    u32 u4RetVal = AUD_RET_OK;

    return (u4RetVal);
}

/**
 * Hal interface : get byps buffer write pointer
 *
 * @param [in]  prThis : hal class, u4Wp : current write pointer
 * @param [out]
 *
 * @return  0 : set ok, 1 : set fail
 */
static u32 BypsHal_GetPoint(void * pThis)
{
    u32 u4MlinWp = 0;

    return (u4MlinWp);
}

/**
 * Hal interface : bypass hw external config update
 *
 * @param [in]  prThis : hal class, prCfg : pointer to external config parameters variable
 * @param [out]
 *
 * @return  0 : set ok, 1 : set fail
 */
static u32 BypsHal_CfgUpd(void * pThis, void * pCfg)
{
    PBYPS_HAL_CLS prThis = (PBYPS_HAL_CLS)pThis;
    PBYPS_EXTPARAMS_T prCfg = (PBYPS_EXTPARAMS_T)pCfg;
    u32 u4RetVal = AUD_RET_OK;

    if (prCfg->u4Gain != prThis->rCfg.rExtCfg.u4Gain)
    {
        BYPSLOG_INFO(T("New Gain Update (0x%x) \r\n"), (u32)(prCfg->u4Gain));

        prThis->rCfg.rExtCfg.u4Gain = prCfg->u4Gain;
        prThis->prBypsHw->SetGain(prThis->prBypsHw, prCfg->u4Gain);
    }

    return (u4RetVal);
}


//==============================================//
    #define CodeSight_BypsHal_Others
//==============================================//



//==============================================//
    #define CodeSight_BypsHal_Create
//==============================================//

/**
 * Hal interface : Hal class delete
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  0 : set ok
 */
static u32 BypsHal_Delete(void * pThis)
{
    PBYPS_HAL_CLS prThis = (PBYPS_HAL_CLS)pThis;
    u32 u4RetVal = AUD_RET_OK;

    if (NULL != prThis->prBypsHw)
    {
        prThis->prBypsHw->Delete(prThis->prBypsHw);
    }

    AUD_CLASS_DELETE();

    return (u4RetVal);
}

/**
 * Hal interface : Hal class create
 *
 * @param [in]
 * @param [out]
 *
 * @return  pointer to bypass line in hal pub cls
 */
PBYPS_HAL_CLS_PUB BypsHal_New(void)
{
    PBYPS_HAL_CLS prThis = (PBYPS_HAL_CLS)kzalloc(sizeof(BYPS_HAL_CLS), GFP_KERNEL);

    if (prThis)
    {
        PBYPS_HAL_CLS_PUB prPub = &(prThis->rPub);

        prThis->u4State = AUD_STATE_UNINIT;

        prThis->prBypsHw = BypsHw_New();

        if (NULL == prThis->prBypsHw)
        {
            BYPSLOG_ERR(T("Byps New BypsHw Error!!! \r\n"));

            kfree(prThis);
            prThis = NULL;
        }
        else
        {
            prPub->Delete = BypsHal_Delete;
            prPub->rHwIf.Setup = BypsHal_Setup;
            prPub->rHwIf.GetStatus = BypsHal_GetStatus;
            prPub->rHwIf.Start = BypsHal_Start;
            prPub->rHwIf.Stop = BypsHal_Stop;
            prPub->rHwIf.GetBuf = BypsHal_GetBuf;
            prPub->rHwIf.GetPoint = BypsHal_GetPoint;
            prPub->rHwIf.SetPoint = BypsHal_SetPoint;
            prPub->rHwIf.CfgUpd = BypsHal_CfgUpd;
        }
    }
    else
    {
        BYPSLOG_ERR(T("Byps New BypsHal Error!!! \r\n"));
    }

    return ((PBYPS_HAL_CLS_PUB)prThis);
}

