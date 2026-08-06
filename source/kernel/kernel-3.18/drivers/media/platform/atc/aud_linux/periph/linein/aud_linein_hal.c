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
 * @file aud_linein_hal.c source file
 * 
 * aud io linein module hardware abstraction layer
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */

#include "aud_oal.h"
#include "aud_linein_hal.h"
#include "aud_io_clock_if.h"


PLIN_HAL_CLS prLinObj[AUDID_LIN_MAX] = {NULL, NULL};

u32 _u4LinLog = ALOG_DEFAULT;

//==============================================//
    #define CodeSight_LinHal_static_FUNC
//==============================================//

/**
 * allocate buffer for line in hw
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return 0: allocate ok, others : allocate ng
 */
static u32 LinHal_AllocBuf(PLIN_HAL_CLS prThis)
{
    u32 u4RetVal = AUD_RET_OK;
    
    u32 u4LinMemSize;
    
    PAUD_LIN_CFG_T prCfg = &(prThis->rCfg);
    
    u4LinMemSize = prCfg->rExtCfg.u4BufSz;
    
    if (0 != prCfg->rExtCfg.u4BufPhyAdr)
    {
        LINLOG_INFO(T("linein buffer allocated by up layer  \r\n" ));
        
        prCfg->rBuf.u4PhySddr = prCfg->rExtCfg.u4BufPhyAdr;
        prCfg->rBuf.u4VirSAdr = 0;
    }
    else
    {
        LINLOG_INFO(T("linein buffer allocated by hal layer  \r\n" ));
    
        prCfg->rBuf.u4VirSAdr = AudOS_Memory_Alloc(u4LinMemSize, LIN_BUF_ALIGN, &(prCfg->rBuf.u4PhySddr));
        if (0 == prCfg->rBuf.u4VirSAdr)
        {
            LINLOG_ERR(T("linein buffer allocated Err !!!!  \r\n" ));
            u4RetVal = AUD_RET_FAIL;
        }
    }
    
    prCfg->rBuf.u4Chn = 2;
    prCfg->rBuf.u4ChBufSz = u4LinMemSize >> 1;
    
    prCfg->rBuf.u4BW = (LIN_24 == prCfg->rFmt.eOutBitNum) ? 24 : 16;

    prCfg->rBuf.u4DataOff = 0;
    prCfg->rBuf.u4DataSize = 0;
    
    LINLOG_INFO(T("LIN(%d) BUF INFO:  channel size : 0x%x, channel num : 0x%x, buf size: 0x%x \r\n"),
                  prThis->eLinId, (u32)(prCfg->rBuf.u4ChBufSz), (u32)(prCfg->rBuf.u4Chn), (u32)u4LinMemSize);
    LINLOG_INFO(T("LIN(%d) BUF INFO:  buf phy adr : 0x%x, buf vir adr : 0x%x \r\n"),
                                    prThis->eLinId, (u32)(prCfg->rBuf.u4PhySddr), (u32)(prCfg->rBuf.u4VirSAdr));
    
    return u4RetVal;
}

/**
 * lin s32 service routine
 *
 * @param [in]  u2Vector : s32 vector
 * @param [out] 
 *
 * @return
 */
static void LinHal_ISR(u16 u2Vector)
{
    PLIN_HAL_CLS prThis;
    AUD_LIN_DEVID eLinId;

    eLinId = (VECTOR_SPD == u2Vector) ? AUDID_LIN1 : AUDID_LIN2;
    prThis = prLinObj[eLinId];

    if (NULL == prThis)
    {
        LINLOG_ERR(T("Lin hal Obj already released, but interrupt Still happen !!!  \r\n" ));
        ASSERT(0);
    }
    else
    {
        //callback
        if (NULL != prThis->rCfg.rExtCfg.rIntCfg.PFN_ISR_CB)
        {
            prThis->rCfg.rExtCfg.rIntCfg.PFN_ISR_CB(u2Vector);
        }

        //clear s32 status
        AudOS_IRQ_Clear(u2Vector);
    }
}

/**
 * pcm interrupt control varibal init
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return 0: ok, others: ng
 */
static u32 LinHal_IntCtrInit(PLIN_HAL_CLS prThis)
{
    u16 u2Vector;
    u32 u4RetVal = AUD_RET_OK;

    u2Vector = (AUDID_LIN1 == prThis->eLinId) ? VECTOR_SPD : VECTOR_LIN2_RCINT;

    AudOS_IRQ_Disable(u2Vector);   

    if (!AudOS_ISR_Reg(u2Vector, LinHal_ISR))
    {                     
        LINLOG_ERR(T("Lin(%d) ISR REG Fail! \n"), prThis->eLinId);
        u4RetVal = AUD_RET_FAIL;
    }

    AudOS_IRQ_Enable(u2Vector); 
    AudOS_IRQ_Clear(u2Vector);
    
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
static u32 LinHal_ExtConfigCheck(PLIN_HAL_CLS prThis, PLIN_EXTPARAMS_T prCfg)
{
    u32 u4RetVal = AUD_RET_OK;
    
    PLIN_EXTPARAMS_T prExtCfg = &(prThis->rCfg.rExtCfg);

    AUDOS_MEMCPY(prExtCfg, prCfg, sizeof(LIN_EXTPARAMS_T));

    if (prExtCfg->eSrc >= LIN_SRC_MAX)
    {
        LINLOG_ERR(T("Lin EXTPARA ERR: Line in src (0x%x) \r\n"), prExtCfg->eSrc);
        prExtCfg->eSrc = INT_LINEIN;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    if (prExtCfg->eIntClkSrc >= LIN_CLK_MAX)
    {
        LINLOG_ERR(T("Lin EXTPARA ERR: s32 clk (0x%x) \r\n"), prExtCfg->eIntClkSrc);
        prExtCfg->eIntClkSrc = LIN_CLK_AOUT1;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    if ((prExtCfg->eSrc ==INT_LINEIN)&&
        ((prExtCfg->eGroup < ADC_SRC_LIN1) || (prExtCfg->eGroup > ADC_SRC_LIN5)))
    {
        LINLOG_ERR(T("Lin EXTPARA ERR: lin group (0x%x) \r\n"), prExtCfg->eGroup);
        prExtCfg->eGroup = ADC_SRC_LIN1;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    if (prExtCfg->rIntCfg.fgOn)
    {
        if (prExtCfg->rIntCfg.eIntPeriod > LIN_INT_PERIOD_256DW)
        {
            LINLOG_ERR(T("Lin EXTPARA ERR: Int period (0x%x) \r\n"), prExtCfg->rIntCfg.eIntPeriod);
            prExtCfg->rIntCfg.eIntPeriod = LIN_INT_PERIOD_128DW;
            
            u4RetVal = AUD_RET_PARAMS_ERR;
            ASSERT(0);
        }
    }

    return (u4RetVal);
}

/**
 * internal configure parameters init
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return
 */
static void LinHal_IntConfigInit(PLIN_HAL_CLS prThis)
{
    PAUD_LIN_CFG_T prCfg = &(prThis->rCfg);
    PLIN_FMT_SETTING_T prFmt = &(prThis->rCfg.rFmt);

    AudOS_Regkey_GetDword(AUD_DRV, (s8 *)L"LIN_FS_MCLK", (u32 *) &(prFmt->eMclkType), AUD_MCLK_256FS);
    AudOS_Regkey_GetDword(AUD_DRV, (s8 *)L"LIN_DATA_FMT", (u32 *) &(prFmt->eDataFmt), AUDFMT_IIS);

    prFmt->eFs = FS_48K;
    prFmt->eCycle = AUD_LRCK_CYC_32;

    prFmt->u4SrcBitNum = 16;
    prFmt->eOutBitNum = LIN_16;

    prFmt->fgInvertBck = TRUE;
    prFmt->fgInvertLrck = FALSE;

    prCfg->eGain = LSBUFGAIN_MINUS_3DB;
}

/**
 * internal configure parameters check
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return 0: check ok , 2 : check fail
 */
static u32 LinHal_IntConfigCheck(PLIN_HAL_CLS prThis)
{
    u32 u4RetVal = AUD_RET_OK;
    
    PAUD_LIN_CFG_T prCfg = &(prThis->rCfg);

    if (prCfg->rFmt.eMclkType >= AUD_MCLK_TYPE_MAX)
    {
        LINLOG_ERR(T("Lin INTPARA ERR: MCLK type (0x%x) \r\n"), prCfg->rFmt.eMclkType);
        prCfg->rFmt.eMclkType = AUD_MCLK_256FS;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }
    
    if ((prCfg->rFmt.eDataFmt >= AUDFMT_UNDEF_INTF) || (AUDFMT_RESERVD == prCfg->rFmt.eDataFmt))
    {
        LINLOG_ERR(T("Lin INTPARA ERR: data format (0x%x) \r\n"), prCfg->rFmt.eDataFmt);
        prCfg->rFmt.eDataFmt = AUDFMT_IIS;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    return (u4RetVal);
}

/**
 * i2s in parameters check
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return 0: check ok , 2 : check fail
 */
static u32 LinHal_I2sInCfgCheck(PLIN_HAL_CLS prThis)
{
    u32 u4RetVal = AUD_RET_OK;
    
    PIIS_LIN_CFG_T prI2sCfg = &(prThis->rCfg.rExtCfg.rI2sLinCfg);

    if (prI2sCfg->eClkMode > AUD_SLAVE_MODE)
    {
        LINLOG_ERR(T("I2S IN PARA ERR: Clk mode (0x%x) \r\n"), prI2sCfg->eClkMode);
        prI2sCfg->eClkMode = AUD_MASTER_MODE;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    if (prI2sCfg->rFmt.eMclkType >= AUD_MCLK_TYPE_MAX)
    {
        LINLOG_ERR(T("I2S IN PARA ERR: mclk type (0x%x) \r\n"), prI2sCfg->rFmt.eMclkType);
        prI2sCfg->rFmt.eMclkType = AUD_MCLK_256FS;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    if (prI2sCfg->rFmt.eCycle > AUD_LRCK_CYC_32)
    {
        LINLOG_ERR(T("I2S IN PARA ERR: lrck cycle (0x%x) \r\n"), prI2sCfg->rFmt.eCycle);
        prI2sCfg->rFmt.eCycle = AUD_LRCK_CYC_32;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    if (prI2sCfg->rFmt.eFs >= FS_UNKNOWN)
    {
        LINLOG_ERR(T("I2S IN PARA ERR: sample rate (0x%x) \r\n"), prI2sCfg->rFmt.eFs);
        prI2sCfg->rFmt.eFs = FS_48K;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    if ((16 != prI2sCfg->rFmt.u4SrcBitNum) && (24 != prI2sCfg->rFmt.u4SrcBitNum))
    {
        LINLOG_ERR(T("I2S IN PARA ERR: src bit num (0x%x) \r\n"), (u32)(prI2sCfg->rFmt.u4SrcBitNum));
        prI2sCfg->rFmt.u4SrcBitNum = 24;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    if ((LIN_16 != prI2sCfg->rFmt.eOutBitNum) && (LIN_24 != prI2sCfg->rFmt.eOutBitNum))
    {
        LINLOG_ERR(T("I2S IN PARA ERR: out bit mode (0x%x) \r\n"), prI2sCfg->rFmt.eOutBitNum);
        prI2sCfg->rFmt.eOutBitNum = LIN_24;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    if (prI2sCfg->eGrpPin0 >= PINMUX_I2SLIN0_GROUP_MAX)
    {
        LINLOG_ERR(T("I2S IN0 PARA ERR: input pin (0x%x) \r\n"), prI2sCfg->eGrpPin0);
        prI2sCfg->eGrpPin0 = PINMUX_I2SLIN0_GROUP1;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    if (prI2sCfg->eGrpPin1 >= PINMUX_I2SLIN1_GROUP_MAX)
    {
        LINLOG_ERR(T("I2S IN1 PARA ERR: input pin (0x%x) \r\n"), prI2sCfg->eGrpPin1);
        prI2sCfg->eGrpPin1 = PINMUX_I2SLIN1_GROUP2;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    return u4RetVal;
}

/**
 * internal adc ext config init
 *
 * @param [in]  
 * @param [out] 
 *
 * @return
 */
static void LinHal_AdcCfgInit(PLIN_HAL_CLS prThis)
{
    PAUD_LIN_CFG_T prCfg = &(prThis->rCfg);
    PADC_EXTPARAMS_T prAdcCfg = &(prThis->rCfg.rAdcExtCfg);
    AUD_LIN_CLK_SRC eClkSrc = prCfg->rExtCfg.eIntClkSrc;

    prAdcCfg->eFs = prCfg->rFmt.eFs;
    prAdcCfg->eInput = prCfg->rExtCfg.eGroup;

    prAdcCfg->eClkSrc = (LIN_CLK_MLIN == eClkSrc) ? AFE_CLK_MLIN : 
                         ((LIN_CLK_AOUT1 == eClkSrc) ? AFE_CLK_AOUT1 : AFE_CLK_AOUT2 );

    prAdcCfg->eLinGain = prCfg->eGain;
    prAdcCfg->u4MicGain = 14; //no use here
}


//==============================================//
    #define CodeSight_LinHal_IF_FLOW
//==============================================//

/**
 * Hal interfac : Lin hw setup
 *
 * @param [in]  prThis : hal class, prCfg : pointer to external config parameters variable 
 * @param [out] 
 *
 * @return 0: setup ok , 2 : setup fail
 */
static u32 LinHal_Setup(void * pThis, void * pCfg)
{
    PLIN_HAL_CLS prThis = (PLIN_HAL_CLS)pThis;
    PLIN_EXTPARAMS_T prCfg = (PLIN_EXTPARAMS_T)pCfg;
    u32 u4RetVal = AUD_RET_OK;

    PLIN_EXTPARAMS_T prExtCfg = &(prThis->rCfg.rExtCfg);

    LINLOG_INFO(T("Lin(%d) LinHal_Setup \r\n"), prThis->eLinId);
    
    //external parameters check
    u4RetVal = LinHal_ExtConfigCheck(prThis, prCfg);

    //internal parameters init
    LinHal_IntConfigInit(prThis);

    //internal parameters check
    u4RetVal = LinHal_IntConfigCheck(prThis);

    //adc config init
    LinHal_AdcCfgInit(prThis);

    //I2S In config check
    if (EXT_LINEIN == prExtCfg->eSrc)
    {
        u4RetVal = LinHal_I2sInCfgCheck(prThis);

        if (AUDID_LIN1 == prThis->eLinId)
        {
            IoPinMux_SetI2sLin(prExtCfg->rI2sLinCfg.eGrpPin0);
        }
        else
        {
            IoPinMux_SetI2sLin2(prExtCfg->rI2sLinCfg.eGrpPin1);
        }
    }

    if (AUD_STATE_UNINIT == prThis->u4State)
    {
        //line in buffer allocate
        u4RetVal = LinHal_AllocBuf(prThis);

        //line s32 control init
        if (TRUE == prExtCfg->rIntCfg.fgOn)
        {
            LinHal_IntCtrInit(prThis);
        }
    }

    //call line in init function
    prThis->prLinHw->InitCfg(prThis->prLinHw, &(prThis->rCfg));

    prThis->u4State = AUD_STATE_INITED;
        
    return (u4RetVal);
}

/**
 * Hal interfac : get line in hw status
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return  hw status
 */
static u32 LinHal_GetStatus(void * pThis)
{
    PLIN_HAL_CLS prThis = (PLIN_HAL_CLS)pThis;
    
    return (prThis->u4State);
}

/**
 * Hal interfac : enable line in
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return  hw status
 */
static u32 LinHal_Start(void * pThis, u32 u4Params)
{
    PLIN_HAL_CLS prThis = (PLIN_HAL_CLS)pThis;
    bool fgLinEn =  TRUE;
    
    LINLOG_INFO(T("Lin(%d) LinHal_Start \r\n"), prThis->eLinId);

    prThis->u4State = AUD_STATE_STARTED;

    //request allocate adc
    if (INT_LINEIN == prThis->rCfg.rExtCfg.eSrc)
    {   
        prThis->prAdc = AdcHal_New(&(prThis->rCfg.rAdcExtCfg));

        if (NULL == prThis->prAdc)
        {
            LINLOG_ERR(T("No free adc for lin(%d) use \r\n"), prThis->eLinId);
            
            fgLinEn = FALSE;
            prThis->u4State = AUD_STATE_STOPPED;
        }
        else
        {
            LINLOG_INFO(T("Allocate ADC(%d) for Lin(%d) \r\n"), prThis->prAdc->eAdcId, prThis->eLinId);
            
            prThis->rCfg.eAdcId = prThis->prAdc->eAdcId;
            prThis->prAdc->rHwIf.Start(prThis->prAdc, 0);

            prThis->prLinHw->SelAfe(prThis->prLinHw, prThis->prAdc->eAdcId);
        }
    }

  #ifdef AUD_IO_POWER_CONTROL
    IoClk_SetModulePowerOn(((AUDID_LIN1 == prThis->eLinId) ? CLKPM_MLIN : CLKPM_MLIN2));
  #endif
  
    prThis->prLinHw->Enable(prThis->prLinHw, fgLinEn);

    return (prThis->u4State);
}


/**
 * Hal interfac : disable line in
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return  hw status
 */
static u32 LinHal_Stop(void * pThis, u32 u4Params)
{
    PLIN_HAL_CLS prThis = (PLIN_HAL_CLS)pThis;
    
    LINLOG_INFO(T("Lin(%d) LinHal_Stop \r\n"), prThis->eLinId);
    
    prThis->prLinHw->Enable(prThis->prLinHw, FALSE);

    if ((INT_LINEIN == prThis->rCfg.rExtCfg.eSrc) && (NULL != prThis->prAdc))
    {
        prThis->prAdc->rHwIf.Stop(prThis->prAdc, 0);
        
        prThis->prAdc->Delete(prThis->prAdc);
        prThis->prAdc = NULL;
    }
    else if(EXT_LINEIN == prThis->rCfg.rExtCfg.eSrc)
    {
        if(prThis->eLinId == AUDID_LIN1)
	{
	    IoPinMux_SetI2sLin(PINMUX_I2SLIN0_DEFAULT);
	}
	else if(prThis->eLinId == AUDID_LIN2)
	{
	    IoPinMux_SetI2sLin2(PINMUX_I2SLIN1_DEFAULT);
	}
    }

  #ifdef AUD_IO_POWER_CONTROL
    IoClk_SetModulePowerDown(((AUDID_LIN1 == prThis->eLinId) ? CLKPM_MLIN : CLKPM_MLIN2));
  #endif

    prThis->u4State = AUD_STATE_STOPPED;
    
    return (prThis->u4State);
}


//==============================================//
    #define CodeSight_LinHal_IF_BUF
//==============================================//

/**
 * Hal interfac : get line in buffer config
 *
 * @param [in]  prThis : hal class
 * @param [out] prBuf : buffer information
 *
 * @return  hw status
 */
static u32 LinHal_GetBuf(void * pThis, AUD_DATA_BUF_T *prBuf)
{
    PLIN_HAL_CLS prThis = (PLIN_HAL_CLS)pThis;
    
    x_memcpy(prBuf, &(prThis->rCfg.rBuf), sizeof(AUD_DATA_BUF_T));
    
    return (prThis->u4State);
}

/**
 * Hal interfac : set line in buffer read pointer
 *
 * @param [in]  prThis : hal class, u4Rp : current read pointer
 * @param [out] 
 *
 * @return  0 : set ok, 1 : set fail
 */
static u32 LinHal_SetPoint(void * pThis, u32 u4Rp)
{
    u32 u4RetVal = AUD_RET_OK;
    
    return (u4RetVal);
}

/**
 * Hal interfac : get line in buffer write pointer
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return  current write pointer
 */
static u32 LinHal_GetPoint(void * pThis)
{
    PLIN_HAL_CLS prThis = (PLIN_HAL_CLS)pThis;
    
    return (prThis->prLinHw->GetWp(prThis->prLinHw));
}


//==============================================//
    #define CodeSight_LinHal_Others
//==============================================//



//==============================================//
    #define CodeSight_LinHal_Create
//==============================================//

/**
 * Hal interfac : Hal class delete
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return  0 : result ok
 */
static u32 LinHal_Delete(void * pThis)
{
    PLIN_HAL_CLS prThis = (PLIN_HAL_CLS)pThis;
    u16 u2Vector;
    
    if (NULL != prThis->prLinHw)
    {
        prThis->prLinHw->Delete(prThis->prLinHw);
    }

    if (0 != prThis->rCfg.rBuf.u4VirSAdr)
    {
        AudOS_Memory_Free(&(prThis->rCfg.rBuf.u4VirSAdr));
    }

    prLinObj[prThis->eLinId] = NULL;

    if (prThis->rCfg.rExtCfg.rIntCfg.fgOn)
    {
        u2Vector = (AUDID_LIN1 == prThis->eLinId) ? VECTOR_SPD : VECTOR_LIN2_RCINT;
        AudOS_IRQ_Disable(u2Vector);
    }

    if (EXT_LINEIN == prThis->rCfg.rExtCfg.eSrc)
    {
        if (AUDID_LIN1 == prThis->eLinId)
        {
            IoPinMux_SetI2sLin(PINMUX_I2SLIN0_DEFAULT);  //release pin mux select
        }
        else
        {
            IoPinMux_SetI2sLin2(PINMUX_I2SLIN1_DEFAULT); //release pin mux select
        }
    }
    
    AUD_CLASS_DELETE();

    return (0);
}

/**
 * Hal interfac : Hal class create
 *
 * @param [in]  eLinId : lin1 or lin2
 * @param [out] 
 *
 * @return  pointer to lin hal pub cls 
 */
PLIN_HAL_CLS_PUB LinHal_New(AUD_LIN_DEVID eLinId)
{
    PLIN_HAL_CLS prThis = (PLIN_HAL_CLS)kzalloc(sizeof(LIN_HAL_CLS), GFP_KERNEL);

    if (prThis)
    {
        PLIN_HAL_CLS_PUB prPub = &(prThis->rPub);
            
        prThis->u4State = AUD_STATE_UNINIT;
        prThis->eLinId = eLinId;

        prThis->prLinHw = LinHw_New(eLinId);

        if (NULL == prThis->prLinHw)
        {
            LINLOG_ERR(T("Lin New LinHw Error!!! \r\n"));
            
            kfree(prThis);
            prThis = NULL;
        }
        else
        {
            LINLOG_DBG(T("Lin(%d) New HAL obj creat success \r\n"), eLinId);
            prPub->Delete = LinHal_Delete;
            prPub->rHwIf.Setup = LinHal_Setup;
            prPub->rHwIf.GetStatus = LinHal_GetStatus;
            prPub->rHwIf.Start = LinHal_Start;
            prPub->rHwIf.Stop = LinHal_Stop;
            prPub->rHwIf.GetBuf = LinHal_GetBuf;
            prPub->rHwIf.GetPoint = LinHal_GetPoint;
            prPub->rHwIf.SetPoint = LinHal_SetPoint;

            prLinObj[eLinId] = prThis;
        }
    }
    else
    {
        LINLOG_ERR(T("Lin(%d) New LinHal Error!!! \r\n"), eLinId);
    }

    return ((PLIN_HAL_CLS_PUB)prThis);
}

