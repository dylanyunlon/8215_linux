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
 * @file aud_micin_hal.c source file
 *
 * aud io mic in module hardware abstraction layer
 *
 * @author qiuhua.yin@autochips.com
 *
 */

#include "aud_micin_hal.h"
#include "aud_io_clock_if.h"
#include "aud_if.h"

u32 _u4MicLog = ALOG_DEFAULT;


//==============================================//
    #define CodeSight_MicHal_static_FUNC
//==============================================//

/**
 * allocate buffer for mic in
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return
 */
static u32 MicHal_AllocBuf(PMIC_HAL_CLS prThis)
{
    u32 u4RetVal = AUD_RET_OK;

    u32 u4MicMemSize;

    PAUD_MIC_CFG_T prMicCfg = &(prThis->rCfg);

    u4MicMemSize = prMicCfg->rExtCfg.u4BufSz;

    if (0 != prMicCfg->rExtCfg.u4BufPhyAdr)
    {
        MICLOG_INFO(T("Mic buffer allocated by up layer  \r\n" ));

        prMicCfg->rBuf.u4PhySddr = prMicCfg->rExtCfg.u4BufPhyAdr;
        prMicCfg->rBuf.u4VirSAdr = 0;
    }
    else
    {
        MICLOG_INFO(T("Mic buffer allocated by hal layer  \r\n" ));

        prMicCfg->rBuf.u4VirSAdr = AudOS_Memory_Alloc(u4MicMemSize, MIC_BUF_ALIGN, &(prMicCfg->rBuf.u4PhySddr));
        if (0 == prMicCfg->rBuf.u4VirSAdr)
        {
            MICLOG_ERR(T("Mic buffer allocated Err !!!!  \r\n" ));
            u4RetVal = AUD_RET_FAIL;
        }
    }

    prMicCfg->rBuf.u4Chn = 2;
    prMicCfg->rBuf.u4ChBufSz = u4MicMemSize >> 1;

    prMicCfg->rBuf.u4BW = (LIN_24 == prMicCfg->rExtCfg.eOutBitNum) ? 24 : 16;
    prMicCfg->rBuf.u4DataOff = 0;

    MICLOG_INFO(T("MIC BUF INFO:  channel size : 0x%x, channel num : 0x%x, buf size: 0x%x \r\n"),
                  (u32)(prMicCfg->rBuf.u4ChBufSz), (u32)(prMicCfg->rBuf.u4Chn), (u32)u4MicMemSize);
    MICLOG_INFO(T("MIC BUF INFO:  buf phy adr : 0x%x, buf vir adr : 0x%x \r\n"),
                  (u32)(prMicCfg->rBuf.u4PhySddr), (u32)(prMicCfg->rBuf.u4VirSAdr));

    return u4RetVal;
}

/**
 * external configure parameters check
 *
 * @param [in]  prThis : hal class, prCfg : pointer to external configure parameters
 * @param [out]
 *
 * @return 0: check ok , 2 : check fail
 */
static u32 MicHal_ExtConfigCheck(PMIC_HAL_CLS prThis, PMIC_EXTPARAMS_T prCfg)
{
    u32 u4RetVal = AUD_RET_OK;

    PMIC_EXTPARAMS_T prMicExtCfg = &(prThis->rCfg.rExtCfg);

    AUDOS_MEMCPY(prMicExtCfg, prCfg, sizeof(MIC_EXTPARAMS_T));

    if (prMicExtCfg->eSrc >= MIC_SRC_MAX)
    {
        MICLOG_ERR(T("Mic EXTPARA ERR: Mic src (0x%x) \r\n"), prMicExtCfg->eSrc);
        prMicExtCfg->eSrc = INT_MICIN;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    if (prMicExtCfg->eI2sPin >= PINMUX_I2SMICIN_GROUP_MAX)
    {
        MICLOG_ERR(T("Mic EXTPARA ERR: I2S In PIN select (0x%x) \r\n"), prMicExtCfg->eI2sPin);
        prMicExtCfg->eI2sPin = PINMUX_I2SMICIN_GROUP2;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    if ((16 != prMicExtCfg->u4SrcBitNum) && (24 != prMicExtCfg->u4SrcBitNum))
    {
        MICLOG_ERR(T("Mic EXTPARA ERR: src bit num (0x%x) \r\n"), (u32)(prMicExtCfg->u4SrcBitNum));
        prMicExtCfg->u4SrcBitNum = 24;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    if (prMicExtCfg->eOutBitNum > LIN_24)
    {
        MICLOG_ERR(T("Mic EXTPARA ERR: out bit mode (0x%x) \r\n"), prMicExtCfg->eOutBitNum);
        prMicExtCfg->eOutBitNum = LIN_16;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    if (0 != (prMicExtCfg->u4BufSz & 0x0FF))
    {
        MICLOG_ERR(T("Mic EXTPARA ERR: buf size align (0x%x) \r\n"), (u32)(prMicExtCfg->u4BufSz));
        prMicExtCfg->eOutBitNum = LIN_16;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    return (u4RetVal);
}

/**
 * internal configure parameters init
 *
 * @param [in]  prThis : pointer to mic hal object
 * @param [out]
 *
 * @return
 */
static void MicHal_IntConfigInit(PMIC_HAL_CLS prThis)
{
   PAUD_MIC_CFG_T prIntCfg  = &(prThis->rCfg);

   AudOS_Regkey_GetDword(AUD_DRV, (s8 *)L"MIC_FS_MCLK", (u32 *) &(prIntCfg->eMclkType), AUD_MCLK_256FS);
   AudOS_Regkey_GetDword(AUD_DRV, (s8 *)L"MIC_DATA_FMT", (u32 *) &(prIntCfg->eDataFmt), AUDFMT_IIS);

   prIntCfg->eCycle = AUD_LRCK_CYC_32;
   prIntCfg->eClkSrc = MIC_CLK_MPH;

   prIntCfg->fgInvertBck = TRUE;
   prIntCfg->fgInvertLrck = FALSE;
}

/**
 * internal configure parameters check
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return 0: check ok , 2 : check fail
 */
static u32 MicHal_IntConfigCheck(PMIC_HAL_CLS prThis)
{
    u32 u4RetVal = AUD_RET_OK;

    PAUD_MIC_CFG_T prIntCfg  = &(prThis->rCfg);

    if (prIntCfg->eMclkType >= AUD_MCLK_TYPE_MAX)
    {
        MICLOG_ERR(T("Mic INTPARA ERR: Mclk Type (0x%x) \r\n"), prIntCfg->eMclkType);
        prIntCfg->eMclkType = AUD_MCLK_256FS;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }


    if ((prIntCfg->eDataFmt >= AUDFMT_UNDEF_INTF) || (AUDFMT_RESERVD == prIntCfg->eDataFmt))
    {
        MICLOG_ERR(T("Mic INTPARA ERR: Data Fmt (0x%x) \r\n"), prIntCfg->eDataFmt);
        prIntCfg->eDataFmt = AUDFMT_IIS;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    return (u4RetVal);
}

/**
 * s32 adc ext config init
 *
 * @param [in]
 * @param [out]
 *
 * @return
 */
static void MicHal_AdcCfgInit(PMIC_HAL_CLS prThis, PMIC_EXTPARAMS_T prCfg)
{
    PADC_EXTPARAMS_T prAdcExtCfg;

    prAdcExtCfg = &(prThis->rCfg.rAdcExtCfg);

    prAdcExtCfg->eFs = prCfg->eFs;
    prAdcExtCfg->eInput = ADC_SRC_MICIN;
    prAdcExtCfg->eClkSrc = AFE_CLK_MPH;
    prAdcExtCfg->u4MicGain = prCfg->u4MicGain;
    prAdcExtCfg->eLinGain = LSBUFGAIN_MINUS_3DB;
}


//==============================================//
    #define CodeSight_MicHal_IF_FLOW
//==============================================//

/**
 * Hal interfac : mic hw setup
 *
 * @param [in]  prThis : hal class, prCfg : pointer to external config parameters variable
 * @param [out]
 *
 * @return 0: setup ok , 2 : setup fail
 */
static u32 MicHal_Setup(void * pThis, void * pCfg)
{
    PMIC_HAL_CLS prThis = (PMIC_HAL_CLS)pThis;
    PMIC_EXTPARAMS_T prCfg = (PMIC_EXTPARAMS_T)pCfg;
    u32 u4RetVal = AUD_RET_OK;

    PAUD_MIC_CFG_T prMicCfg = &(prThis->rCfg);

    //external parameters check
    u4RetVal = MicHal_ExtConfigCheck(prThis, prCfg);

    //internal parameters init
    MicHal_IntConfigInit(prThis);

    //internal parameters check
    u4RetVal = MicHal_IntConfigCheck(prThis);

    //mic in buffer allocate
    if (AUD_STATE_UNINIT == prThis->u4State) {
        u4RetVal = MicHal_AllocBuf(prThis);
    }

    //call mic in init function
    prThis->prMicHw->InitCfg(prThis->prMicHw, prMicCfg);

    if (EXT_MICIN == prMicCfg->rExtCfg.eSrc)
    {
        // pin mux config
        IoPinMux_SetI2sMicIn(prMicCfg->rExtCfg.eI2sPin);
    }
    else
    {
        //internal adc config init
        MicHal_AdcCfgInit(prThis, prCfg);
    }

    prThis->u4State = AUD_STATE_INITED;

    return (u4RetVal);
}

/**
 * Hal interfac : get mic hw status
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  hw status
 */
static u32 MicHal_GetStatus(void * pThis)
{
    PMIC_HAL_CLS prThis = (PMIC_HAL_CLS)pThis;

    return (prThis->u4State);
}

/**
 * Hal interfac : enable mic in
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  hw status
 */
static u32 MicHal_Start(void * pThis, u32 u4Params)
{
    //request allocate adc
    PMIC_HAL_CLS prThis = (PMIC_HAL_CLS)pThis;
    bool fgMicEn =  TRUE;

    prThis->u4State = AUD_STATE_STARTED;

    if (INT_MICIN == prThis->rCfg.rExtCfg.eSrc)
    {
        prThis->prAdc = AdcHal_New(&(prThis->rCfg.rAdcExtCfg));

        if (NULL == prThis->prAdc)
        {
            MICLOG_ERR(T("No free adc for mic in use \r\n"));

            fgMicEn = FALSE;
            prThis->u4State = AUD_STATE_STOPPED;
        }
        else
        {
            prThis->rCfg.eAdcId = prThis->prAdc->eAdcId;
            prThis->prAdc->rHwIf.Start(prThis->prAdc, 0);

            prThis->prMicHw->SelAfe(prThis->prMicHw, prThis->prAdc->eAdcId);
        }
    }

  #ifdef AUD_IO_POWER_CONTROL
    IoClk_SetModulePowerOn(CLKPM_MPHONE);
  #endif

    prThis->prMicHw->Enable(prThis->prMicHw, fgMicEn);

    return (prThis->u4State);
}

/**
 * Hal interfac : disable mic in
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  hw status
 */
static u32 MicHal_Stop(void * pThis, u32 u4Params)
{
    PMIC_HAL_CLS prThis = (PMIC_HAL_CLS)pThis;

    prThis->prMicHw->Enable(prThis->prMicHw, FALSE);

  #ifdef AUD_IO_POWER_CONTROL
    IoClk_SetModulePowerDown(CLKPM_MPHONE);
  #endif

    if ((INT_MICIN == prThis->rCfg.rExtCfg.eSrc) && (NULL != prThis->prAdc))
    {
        prThis->prAdc->rHwIf.Stop(prThis->prAdc, 0);

        prThis->prAdc->Delete(prThis->prAdc);
        prThis->prAdc = NULL;
    }

    prThis->u4State = AUD_STATE_STOPPED;

    return (prThis->u4State);
}


//==============================================//
    #define CodeSight_MicHal_IF_BUF
//==============================================//

/**
 * Hal interfac : get mic in buffer config
 *
 * @param [in]  prThis : hal class
 * @param [out] prBuf : buffer information
 *
 * @return  hw status
 */
static u32 MicHal_GetBuf(void * pThis, AUD_DATA_BUF_T *prBuf)
{
    PMIC_HAL_CLS prThis = (PMIC_HAL_CLS)pThis;

    x_memcpy(prBuf, &(prThis->rCfg.rBuf), sizeof(AUD_DATA_BUF_T));

    return (AUD_RET_OK);
}

/**
 * Hal interfac : set mic in buffer read pointer
 *
 * @param [in]  prThis : hal class, u4Rp : current read pointer
 * @param [out]
 *
 * @return  0 : set ok, 1 : set fail
 */
static u32 MicHal_SetPoint(void * pThis, u32 u4Rp)
{
    u32 u4RetVal = AUD_RET_OK;

    return (u4RetVal);
}

/**
 * Hal interfac : get mic in buffer write pointer
 *
 * @param [in]  prThis : hal class, u4Wp : current write pointer
 * @param [out]
 *
 * @return  0 : set ok, 1 : set fail
 */
static u32 MicHal_GetPoint(void * pThis)
{
    PMIC_HAL_CLS prThis = (PMIC_HAL_CLS)pThis;
    u32 u4MicWp;

    u4MicWp  = prThis->prMicHw->GetWp(prThis->prMicHw);

    return (u4MicWp);
}

/**
 * Hal interfac : external hw config parameter update
 *
 * @param [in]  prThis : hal class, pCfg : external config
 * @param [out]
 *
 * @return  0 : set ok, 1 : set fail
 */
static u32 MicHal_CfgUpd(void * pThis, void * pCfg)
{
    PMIC_HAL_CLS prThis = (PMIC_HAL_CLS)pThis;
    PMIC_EXTPARAMS_T prCfg = (PMIC_EXTPARAMS_T)pCfg;
    u32 u4RetVal = AUD_RET_OK;

    PMIC_EXTPARAMS_T pExtCfg = &(prThis->rCfg.rExtCfg);
    PADC_EXTPARAMS_T pExtAdcCfg = &(prThis->rCfg.rAdcExtCfg);

    //check parameter1 : sample rate change
    if (pExtCfg->eFs != prCfg->eFs)
    {
        MICLOG_INFO(T("MicHal: New Fs Set (%d) \r\n"), prCfg->eFs);

        pExtCfg->eFs = prCfg->eFs;
        pExtAdcCfg->eFs = prCfg->eFs;

        if (MIC_CLK_MPH == prThis->rCfg.eClkSrc)
        {
            IoClk_SetMphMclk(prThis->rCfg.eMclkType, pExtCfg->eFs);
        };
    }

    //check parameter2 :
    if (pExtCfg->u4MicGain != prCfg->u4MicGain)
    {
        MICLOG_INFO(T("MicHal: New Mic Gain Set (%d) \r\n"), (s32)(prCfg->u4MicGain));

        pExtCfg->u4MicGain = prCfg->u4MicGain;
        pExtAdcCfg->u4MicGain = prCfg->u4MicGain;
    }

    return u4RetVal;
}

//==============================================//
    #define CodeSight_MicHal_Others
//==============================================//



//==============================================//
    #define CodeSight_MicHal_Create
//==============================================//

/**
 * Hal interfac : Hal class delete
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  0 : set ok
 */
static u32 MicHal_Delete(void * pThis)
{
    PMIC_HAL_CLS prThis = (PMIC_HAL_CLS)pThis;

    if (NULL != prThis->prMicHw)
    {
        prThis->prMicHw->Delete(prThis->prMicHw);
    }

    if (0 != prThis->rCfg.rBuf.u4VirSAdr)
    {
        AudOS_Memory_Free(&(prThis->rCfg.rBuf.u4VirSAdr));
    }

    if (EXT_MICIN == prThis->rCfg.rExtCfg.eSrc)
    {
        IoPinMux_SetI2sMicIn(PINMUX_I2SMICIN_DEFAULT);  //release pin mux select
    }

    AUD_CLASS_DELETE();

    return (0);
}

/**
 * Hal interfac : Hal class create
 *
 * @param [in]
 * @param [out]
 *
 * @return  pointer to mic in hal pub cls
 */
PMIC_HAL_CLS_PUB MicHal_New(void)
{
    PMIC_HAL_CLS prThis;

    prThis = (PMIC_HAL_CLS)kzalloc(sizeof(MIC_HAL_CLS), GFP_KERNEL);

    if (prThis)
    {
        PMIC_HAL_CLS_PUB prPub = &(prThis->rPub);

        prThis->u4State = AUD_STATE_UNINIT;

        prThis->prMicHw= MicHw_New();

        prPub->Delete = MicHal_Delete;
        prPub->rHwIf.Setup = MicHal_Setup;
        prPub->rHwIf.GetStatus = MicHal_GetStatus;
        prPub->rHwIf.Start = MicHal_Start;
        prPub->rHwIf.Stop = MicHal_Stop;
        prPub->rHwIf.GetBuf = MicHal_GetBuf;
        prPub->rHwIf.GetPoint = MicHal_GetPoint;
        prPub->rHwIf.SetPoint = MicHal_SetPoint;
        prPub->rHwIf.CfgUpd = MicHal_CfgUpd;
    }

    return ((PMIC_HAL_CLS_PUB)prThis);
}

