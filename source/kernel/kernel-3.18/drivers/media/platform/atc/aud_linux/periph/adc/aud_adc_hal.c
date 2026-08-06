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
 * @file aud_adc_hal.c source file
 *
 * aud io adc module hardware abstraction layer
 *
 * @author qiuhua.yin@autochips.com
 *
 */

 #include "aud_adc_hal.h"

 u32 _u4AdcLog = ALOG_DEFAULT;
 struct semaphore AudAdcAllocateSema;

 typedef struct
 {
     AUD_ADC_INPUT_SRC aInput[AUD_ADC2 + 1];
     PADC_HAL_CLS aObj[AUD_ADC2 + 1];
 }ADC_MANAGER_T, *PADC_MANAGER_T;

 ADC_MANAGER_T rAdcManager = {
    {ADC_SRC_NON, ADC_SRC_NON},
    {NULL, NULL},
 };

//==============================================//
    #define CodeSight_AdcHal_static_FUNC
//==============================================//
/**
 * free adc get
 *
 * @param [in]
 * @param [out]
 *
 * @return : adc id
 */
static AUD_ADC_ID AdcHal_GetFreeAdc(void)
{
    AUD_ADC_ID eAdcId;

    PADC_MANAGER_T prAdcMg = &rAdcManager;

    for (eAdcId = AUD_ADC1; eAdcId < AUD_ADC_NON; eAdcId ++)
    {
        if (ADC_SRC_NON == prAdcMg->aInput[eAdcId])
        {
            break;
        }
    }

    if (eAdcId == AUD_ADC_NON)
    {
        ADCLOG_ERR(T("all ADC is used \r\n"));
    }

    return (eAdcId);
}

/**
 * allocate adc
 *
 * @param [in]  pThis : hal class, prCfg : pointer to external configure parameters
 * @param [out]
 *
 * @return : adc id
 */
static AUD_ADC_ID AdcHal_AllocateAdc(AUD_ADC_INPUT_SRC eInput)
{
    AUD_ADC_ID eAdcId;

    PADC_MANAGER_T prAdcMg = &rAdcManager;

    eAdcId = AUD_ADC_NON;

    if (ADC_SRC_MICIN != eInput)
    {
        if (eInput == prAdcMg->aInput[AUD_ADC1])
        {
            eAdcId = AUD_ADC1;
        }

        if (eInput == prAdcMg->aInput[AUD_ADC2])
        {
            eAdcId = AUD_ADC2;
        }
    }

    if (AUD_ADC_NON == eAdcId)
    {
        eAdcId = AdcHal_GetFreeAdc();
    }

    return eAdcId;
}

/**
 * external configure parameters check
 *
 * @param [in]  pThis : hal class, prCfg : pointer to external configure parameters
 * @param [out]
 *
 * @return 0: check ok , 2 : check fail
 */
static u32 AdcHal_ExtConfigCheck(PADC_HAL_CLS pThis, PADC_EXTPARAMS_T prCfg)
{
    u32 u4RetVal = AUD_RET_OK;

    PADC_EXTPARAMS_T prExtCfg = &(pThis->rCfg);

    AUDOS_MEMCPY(prExtCfg, prCfg, sizeof(ADC_EXTPARAMS_T));

    if ((FS_8K != prExtCfg->eFs) && (FS_16K != prExtCfg->eFs) && (FS_48K != prExtCfg->eFs))
    {
        ADCLOG_ERR(T("Adc EXTPARA ERR: Sample rate (0x%x) \r\n"), prExtCfg->eFs);
        prExtCfg->eFs = FS_8K;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    if (prExtCfg->eInput >= ADC_SRC_NON)
    {
        ADCLOG_ERR(T("Adc EXTPARA ERR: Input Src (0x%x) \r\n"), prExtCfg->eInput);
        prExtCfg->eInput = ADC_SRC_LIN1;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    if (prExtCfg->eLinGain >= LSBUFGAIN_MAX)
    {
        ADCLOG_ERR(T("Adc EXTPARA ERR: Line in gain (0x%x) \r\n"), prExtCfg->eLinGain);
        prExtCfg->eLinGain = LSBUFGAIN_MINUS_3DB;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    if (prExtCfg->u4MicGain > 63)
    {
        ADCLOG_ERR(T("Adc EXTPARA ERR: mic in gain (0x%x) \r\n"), (u32)(prExtCfg->u4MicGain));
        prExtCfg->eLinGain = 14;  //0Db

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    return (u4RetVal);
}


//==============================================//
    #define CodeSight_AdcHal_IF_FLOW
//==============================================//

/**
 * Hal interfac : adc hw setup
 *
 * @param [in]  pThis : hal class, prCfg : pointer to external config parameters variable
 * @param [out]
 *
 * @return 0: setup ok , 2 : setup fail
 */
static u32 AdcHal_Setup(void * pThis, void * pCfg)
{
    u32 u4RetVal = AUD_RET_OK;


    return (u4RetVal);
}

/**
 * Hal interfac : get adc hw status
 *
 * @param [in]  pThis : hal class
 * @param [out]
 *
 * @return  hw status
 */
static u32 AdcHal_GetStatus(void * pThis)
{
    PADC_HAL_CLS prThis = (PADC_HAL_CLS)pThis;

    return (prThis->rPub.u4State);
}

/**
 * Hal interfac : enable adc
 *
 * @param [in]  pThis : hal class
 * @param [out]
 *
 * @return  hw status
 */
static u32 AdcHal_Start(void * pThis, u32 u4Params)
{
    PADC_HAL_CLS prThis = (PADC_HAL_CLS)pThis;

    prThis->prAdcHw->Start(prThis->prAdcHw, &(prThis->rCfg));

    prThis->rPub.u4State = AUD_STATE_STARTED;

    return (AUD_RET_OK);
}

/**
 * Hal interfac : stop adc
 *
 * @param [in]  pThis : hal class
 * @param [out]
 *
 * @return  hw status
 */
static u32 AdcHal_Stop(void * pThis, u32 u4Params)
{
    PADC_HAL_CLS prThis = (PADC_HAL_CLS)pThis;

    if (prThis->u4UserNum == 1)
    {
        prThis->prAdcHw->Stop(prThis->prAdcHw);
        prThis->rPub.u4State = AUD_STATE_STOPPED;
    }
    else
    {
        ADCLOG_DBG(T(" AdcHal_Stop ADC(%d) User Num (%d) \r\n"),
                    prThis ->rPub.eAdcId, (s32)(prThis->u4UserNum));
    }

        return (AUD_RET_OK);
}


//==============================================//
    #define CodeSight_AdcHal_IF_BUF
//==============================================//

/**
 * Hal interfac : get adc buffer status
 *
 * @param [in]  pThis : hal class
 * @param [out] prBuf : buffer information
 *
 * @return  hw status
 */
static u32 AdcHal_GetBuf(void * pThis, AUD_DATA_BUF_T *prBuf)
{
    return (AUD_RET_OK);
}

/**
 * Hal interfac : set adc buffer write pointer
 *
 * @param [in]  pThis : hal class, u4Wp : current write pointer
 * @param [out]
 *
 * @return  0 : set ok, 1 : set fail
 */
static u32 AdcHal_SetPoint(void * pThis, u32 u4Wp)
{
    u32 u4RetVal = AUD_RET_OK;

    return (u4RetVal);
}

/**
 * Hal interfac : set adc buffer write pointer
 *
 * @param [in]  pThis : hal class, u4Wp : current write pointer
 * @param [out]
 *
 * @return  0 : set ok, 1 : set fail
 */
static u32 AdcHal_GetPoint(void * pThis)
{
    return (0);
}


//==============================================//
    #define CodeSight_AdcHal_Others
//==============================================//

/**
 * set line input pin ain0_l ~ ain4_l, ain0_r ~ ain4_r, anlog input or gpi function
 *
 * @param [in]ePinIdx : input pin group; fgGpiFunEn : if set gpi function
 * @param [out]
 *
 * @return
 */
void AdcHal_SetInputPinGpioFun(AUD_LIN_PIN_IDX ePinIdx, bool fgGpiFunEn)
{
    AdcHw_SetInputPinGpioFun(ePinIdx, fgGpiFunEn);
}


//==============================================//
    #define CodeSight_AdcHal_Create
//==============================================//

/**
 * Hal interfac : Hal class delete
 *
 * @param [in]  pThis : hal class
 * @param [out]
 *
 * @return  0 : set ok
 */
static u32 AdcHal_Delete(void * pThis)
{

    PADC_HAL_CLS prThis = (PADC_HAL_CLS)pThis;

    down(&AudAdcAllocateSema);
    if (prThis ->u4UserNum > 1)
    {
        prThis ->u4UserNum --;
        ADCLOG_INFO(T(" ADC(%d) User Num (%d) \r\n"), prThis ->rPub.eAdcId, (s32)(prThis ->u4UserNum));
    }
    else
    {
        prThis ->u4UserNum = 0;

        rAdcManager.aInput[prThis ->rPub.eAdcId] = ADC_SRC_NON;
        rAdcManager.aObj[prThis ->rPub.eAdcId] = NULL;

        if (NULL != prThis->prAdcHw)
        {
            prThis ->prAdcHw ->Delete(prThis ->prAdcHw);
        }

      #ifdef AUD_IO_POWER_CONTROL
        IoClk_SetModulePowerDown(((AUD_ADC1 == prThis ->rPub.eAdcId) ? CLKPM_AFE1_26M : CLKPM_AFE2_26M));
      #endif

        AUD_CLASS_DELETE();
    }
    up(&AudAdcAllocateSema);

    return (0);
}

/**
 * Hal interfac : Hal class create
 *
 * @param [in]
 * @param [out]
 *
 * @return  pointer to adc hal pub class
 */
PADC_HAL_CLS_PUB AdcHal_New(PADC_EXTPARAMS_T prAdcExtCfg)
{
    AUD_ADC_ID eAdcId;
    PADC_HAL_CLS pThis;

    down(&AudAdcAllocateSema);

    eAdcId = AdcHal_AllocateAdc(prAdcExtCfg->eInput);

    if (AUD_ADC_NON == eAdcId)
    {
        ADCLOG_ERR(T("Allocate ADC err : No free ADC \r\n"));

        pThis = NULL;
    }
    else if (ADC_SRC_NON != (rAdcManager.aInput[eAdcId]))
    {
       ADCLOG_INFO(T(" Use the allocated adc \r\n"));

       pThis =  rAdcManager.aObj[eAdcId];
       pThis->u4UserNum ++;
    }
    else
    {
        ADCLOG_INFO(T("Allocate ADC success : ADC ID Is (%d)\r\n"), eAdcId);

        pThis = (PADC_HAL_CLS)kzalloc(sizeof(ADC_HAL_CLS), GFP_KERNEL);

        if (pThis)
        {
            PADC_HAL_CLS_PUB prPub = &pThis->rPub;

            pThis->u4UserNum = 1;
            prPub->u4State = AUD_STATE_UNINIT;
            pThis->rPub.eAdcId = eAdcId;

            AdcHal_ExtConfigCheck(pThis, prAdcExtCfg);

            pThis->prAdcHw = AdcHw_New(eAdcId);

            prPub->Delete = AdcHal_Delete;
            prPub->rHwIf.Setup = AdcHal_Setup;
            prPub->rHwIf.GetStatus = AdcHal_GetStatus;
            prPub->rHwIf.Start = AdcHal_Start;
            prPub->rHwIf.Stop = AdcHal_Stop;
            prPub->rHwIf.GetBuf = AdcHal_GetBuf;
            prPub->rHwIf.GetPoint = AdcHal_GetPoint;
            prPub->rHwIf.SetPoint = AdcHal_SetPoint;

            rAdcManager.aInput[eAdcId] = prAdcExtCfg->eInput;
            rAdcManager.aObj[eAdcId] = pThis;

          #ifdef AUD_IO_POWER_CONTROL
            IoClk_SetModulePowerOn(((AUD_ADC1 == eAdcId) ? CLKPM_AFE1_26M : CLKPM_AFE2_26M));
          #endif
        }
    }
    up(&AudAdcAllocateSema);

    return ((PADC_HAL_CLS_PUB)pThis);
}

