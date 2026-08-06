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
 * @file aud_dac_hal.c source file
 * 
 * aud io dac module hardware abstraction layer
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */

 #include "aud_dac_hal.h"

 u32 _u4DacLog = ALOG_DEFAULT;
 

//==============================================//
    #define CODESIGHT_DACHAL_STATIC_FUNC
//==============================================//
    
/**
 * external configure parameters check
 *
 * @param [in]  prThis : hal class, prCfg : pointer to external configure parameters
 * @param [out] 
 *
 * @return 0: check ok , 2 : check fail
 */
static u32 DacHal_ExtConfigCheck(PDAC_HAL_CLS prThis, PDAC_EXTPARAMS_T prCfg)
{
    u32 u4RetVal = AUD_RET_OK;
    
    PDAC_EXTPARAMS_T prExtCfg = &(prThis->rCfg);

    AUDOS_MEMCPY(prExtCfg, prCfg, sizeof(DAC_EXTPARAMS_T));

    if (prExtCfg->eOutPath >= AOUT_PATH_MAX)
    {
        DACLOG_ERR((T("Dac EXTPARA ERR: Out Path(FS/RS) (0x%x) \r\n"), prExtCfg->eOutPath));
        prExtCfg->eOutPath = AOUT_FS;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    if (prExtCfg->eDacType > AUD_DAC_EXT)
    {
        DACLOG_ERR((T("Dac EXTPARA ERR: DAC type(PWM/EXT) (0x%x) \r\n"), prExtCfg->eDacType));
        prExtCfg->eDacType = AUD_DAC_PWM;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    if ((AUD_DAC_EXT == prExtCfg->eDacType) && (AOUT_FS == prExtCfg->eOutPath))
    {
        if (prExtCfg->ePinMuxFsExtDac >= PINMUX_FS_I2SOUT_GROUP_MAX)
        {
            DACLOG_ERR((T("Dac EXTPARA ERR: FS I2S OUT PIN MUX (0x%x) \r\n"), prExtCfg->ePinMuxFsExtDac));
            prExtCfg->ePinMuxFsExtDac = PINMUX_FS_I2SOUT_GROUP1;
        
            u4RetVal = AUD_RET_PARAMS_ERR;
            ASSERT(0);
        }
    }

    if ((AUD_DAC_EXT == prExtCfg->eDacType) && (AOUT_RS == prExtCfg->eOutPath))
    {
        if (prExtCfg->ePinMuxRsExtDac >= PINMUX_RS_I2SOUT_GROUP_MAX)
        {
            DACLOG_ERR((T("Dac EXTPARA ERR: RS I2S OUT PIN MUX (0x%x) \r\n"), prExtCfg->ePinMuxRsExtDac));
            prExtCfg->ePinMuxRsExtDac = PINMUX_RS_I2SOUT_GROUP2;
        
            u4RetVal = AUD_RET_PARAMS_ERR;
            ASSERT(0);
        }
    }

    if (prExtCfg->eApll > CKGEN_APLL2)
    {
        DACLOG_ERR((T("Dac EXTPARA ERR: Apll select (0x%x) \r\n"), prExtCfg->eApll));
        prExtCfg->eApll = CKGEN_APLL2;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    return (u4RetVal);
}
    

//==============================================//
    #define CODESIGHT_DACHAL_IF_FLOW
//==============================================//

/**
 * Hal interfac : dac hw setup
 *
 * @param [in]  prThis : hal class, prCfg : pointer to external config parameters variable 
 * @param [out] 
 *
 * @return 0: setup ok , 2 : setup fail
 */
static u32 DacHal_Setup(void * pThis, void * pCfg)
{
    u32 u4RetVal = AUD_RET_OK;
        
    return (u4RetVal);
}

/**
 * Hal interfac : get dac hw status
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return  hw status
 */
static u32 DacHal_GetStatus(void * pThis)
{
    PDAC_HAL_CLS prThis = (PDAC_HAL_CLS)pThis;
    
    return (prThis->rPub.u4State);
}

/**
 * Hal interfac : enable dac
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return  hw status
 */
static u32 DacHal_Start(void * pThis, u32 u4Params)
{
    PDAC_HAL_CLS prThis = (PDAC_HAL_CLS)pThis;
    
    prThis->prDacHw->Start(prThis->prDacHw, &(prThis->rCfg));

    prThis->rPub.u4State = AUD_STATE_STARTED;

    return (AUD_RET_OK);
}

/**
 * Hal interfac : stop dac
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return  hw status
 */
static u32 DacHal_Stop(void * pThis, u32 u4Params)
{
    PDAC_HAL_CLS prThis = (PDAC_HAL_CLS)pThis;
    
    prThis->prDacHw->Stop(prThis->prDacHw);

    prThis->rPub.u4State = AUD_STATE_STOPPED;
    
    return (AUD_RET_OK);
}


//==============================================//
    #define CODESIGHT_DACHAL_IF_BUF
//==============================================//

/**
 * Hal interfac : get dac buffer status
 *
 * @param [in]  prThis : hal class
 * @param [out] prBuf : buffer information
 *
 * @return  hw status
 */
static u32 DacHal_GetBuf(void * pThis, AUD_DATA_BUF_T *prBuf)
{   
    return (AUD_RET_OK);
}

/**
 * Hal interfac : set dac buffer write pointer
 *
 * @param [in]  prThis : hal class, u4Wp : current write pointer
 * @param [out] 
 *
 * @return  0 : set ok, 1 : set fail
 */
static u32 DacHal_SetPoint(void * pThis, u32 u4Wp)
{
    u32 u4RetVal = AUD_RET_OK;
    
    return (u4RetVal);
}

/**
 * Hal interfac : set dac buffer write pointer
 *
 * @param [in]  prThis : hal class, u4Wp : current write pointer
 * @param [out] 
 *
 * @return  0 : set ok, 1 : set fail
 */
static u32 DacHal_GetPoint(void * pThis)
{   
    return (0);
}


//==============================================//
    #define CODESIGHT_DACHAL_OTHERS
//==============================================//

/**
 * pwm dac analog part channel set analog or digital function config
 *
 * @param [in] ePwmChId : channel set index;  fgEn : GPO(1)/Analog(0)
 * @param [out] 
 *
 * @return
 */
void DacHal_SetPwmAnaGpioFun(AUD_PWM_DAC_ID ePwmChId, bool fgEn)
{
    DacHw_SetPwmAnaGpioFun(ePwmChId, fgEn);
}

/**
 * pwm dac baisc setting
 *
 * @param [in]
 * @param [out] 
 *
 * @return
 */
void DacHal_SetPwmBasicSetting(void)
{
    DacHw_SetPwmAnalogPartBasicPowerOn();
}



//==============================================//
    #define CODESIGHT_DACHAL_CREATE
//==============================================//

/**
 * Hal interfac : Hal class delete
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return  0 : set ok
 */
static u32 DacHal_Delete(void * pThis)
{   
    PDAC_HAL_CLS prThis = (PDAC_HAL_CLS)pThis;

    if (NULL != prThis->prDacHw)
    {
        prThis->prDacHw->Delete(prThis->prDacHw);
    }

    //release pin mux select
    if (AOUT_FS == prThis->rCfg.eOutPath)
    {
        IoPinMux_SetAmuteFs(PINMUX_AMUTE_FRONT_DEFAULT);
    
        if (AUD_DAC_EXT == prThis->rCfg.eDacType)
        {
            IoPinMux_SetI2sOutFs(PINMUX_FS_I2SOUT_DEFAULT);
        }
    }
    else
    {
        IoPinMux_SetAmuteRs(PINMUX_AMUTE_REAR_DEFAULT);
        
        if (AUD_DAC_EXT == prThis->rCfg.eDacType)
        {
            IoPinMux_SetI2sOutRs(PINMUX_RS_I2SOUT_DEFAULT);
        }
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
 * @return  pointer to dac hal pub class 
 */
PDAC_HAL_CLS_PUB DacHal_New(PDAC_EXTPARAMS_T prDacExtCfg)
{   
    PDAC_HAL_CLS prThis;

    prThis = (PDAC_HAL_CLS)kzalloc(sizeof(DAC_HAL_CLS), GFP_KERNEL);

    if (prThis)
    {
        PDAC_HAL_CLS_PUB prPub = &prThis->rPub;

        prPub->u4State = AUD_STATE_UNINIT;

        DacHal_ExtConfigCheck(prThis, prDacExtCfg);
        
        prThis->prDacHw = DacHw_New(prDacExtCfg->eOutPath, prDacExtCfg->eDacType);

        
        if (NULL == prThis->prDacHw)
        {
            DACLOG_ERR((T("OutPath(%d) Dac(%d): New DacHw Error.\r\n"), 
                prDacExtCfg->eOutPath, prDacExtCfg->eDacType));
            
            kfree(prThis);
            prThis = NULL;
        }
        else
        {
            //external dac i2s out pinmux select
            if (AOUT_FS == prDacExtCfg->eOutPath)
            {
                IoPinMux_SetAmuteFs(PINMUX_AMUTE_FRONT_GROUP1);

                if (AUD_DAC_EXT == prDacExtCfg->eDacType)
                {
                    IoPinMux_SetI2sOutFs(prDacExtCfg->ePinMuxFsExtDac);
                }
            }
            else
            {
                IoPinMux_SetAmuteRs(PINMUX_AMUTE_REAR_GROUP1);
                
                if (AUD_DAC_EXT == prDacExtCfg->eDacType)
                {
                    IoPinMux_SetI2sOutRs(prDacExtCfg->ePinMuxRsExtDac);
                }
            }

            prPub->Delete = DacHal_Delete;
            prPub->rHwIf.Setup = DacHal_Setup;
            prPub->rHwIf.GetStatus = DacHal_GetStatus;
            prPub->rHwIf.Start = DacHal_Start;
            prPub->rHwIf.Stop = DacHal_Stop;
            prPub->rHwIf.GetBuf = DacHal_GetBuf;
            prPub->rHwIf.GetPoint = DacHal_GetPoint;
            prPub->rHwIf.SetPoint = DacHal_SetPoint;
        }
    }
    else
    {
        DACLOG_ERR((T("FS/RS(%d) Dac(PWM/EXT: %d) New DacHal Error!!! \r\n"), prDacExtCfg->eOutPath, prDacExtCfg->eDacType));
    }

    return ((PDAC_HAL_CLS_PUB)prThis);
}

