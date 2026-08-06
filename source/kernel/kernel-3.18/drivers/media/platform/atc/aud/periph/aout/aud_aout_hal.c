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
 * @file aud_aout_hal.c source file
 *
 * aud io aout module hardware abstraction layer
 *
 * @author qiuhua.yin@autochips.com
 *
 */

#include "aud_aout_hal.h"
#include "aud_io_clock_if.h"
#include "aud_if.h"


u32 _u4AoutLog = ALOG_DEFAULT;
PAOUT_HAL_CLS prAoutObj[AUDID_AOUT_MAX] = {NULL, NULL};



//==============================================//
    #define CodeSight_AoutHal_static_FUNC
//==============================================//

/**
 * allocate buffer for aout
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return
 */
#ifdef  AOUT_ARM_CTL
static u32 AoutHal_AllocBuf(PAOUT_HAL_CLS prThis)
{
    u32 u4RetVal = AUD_RET_OK;
    u32 u4AoutMemSize;

    PAUD_AOUT_CFG_T prCfg = &(prThis->rAoutCfg);
    PAOUT_EXTPARAMS_T prExtCfg = &(prThis->rAoutCfg.rExtCfg);

    u4AoutMemSize = prExtCfg->u4BufSz;

    if (0 != prExtCfg->u4BufPhyAdr)
    {
        AOUTLOG_INFO(T("Aout buffer allocated by up layer  \r\n" ));

        prCfg->rBuf.u4PhySddr = prExtCfg->u4BufPhyAdr;
        prCfg->rBuf.u4VirSAdr = 0;
    }
    else
    {
        AOUTLOG_INFO(T("Aout buffer allocated by hal layer  \r\n" ));

        prCfg->rBuf.u4VirSAdr = AudOS_Memory_Alloc(u4AoutMemSize, AOUT_BUF_ALIGN, &(prCfg->rBuf.u4PhySddr));
        if (0 == prCfg->rBuf.u4VirSAdr)
        {
            AOUTLOG_ERR(T("Aout buffer allocated Err !!!!  \r\n" ));
            u4RetVal = AUD_RET_FAIL;
        }
    }

    prCfg->rBuf.u4Chn = prExtCfg->u4ChNum;
    prCfg->rBuf.u4ChBufSz = u4AoutMemSize / prExtCfg->u4ChNum;
    prCfg->rBuf.u4BW = prExtCfg->u4Bps;
    prCfg->rBuf.u4DataOff = 0;
    prCfg->rBuf.u4DataSize = 0;

    AOUTLOG_INFO(T("AOUT BUF INFO:  channel size : 0x%x, channel num : 0x%x, buf size: 0x%x \r\n"),
                                     (u32)(prCfg->rBuf.u4ChBufSz), (u32)(prCfg->rBuf.u4Chn), (u32)u4AoutMemSize);
    AOUTLOG_INFO(T("AOUT BUF INFO:  buf phy adr : 0x%x, buf vir adr : 0x%x \r\n"),
                                     (u32)(prCfg->rBuf.u4PhySddr), (u32)(prCfg->rBuf.u4VirSAdr));
    return u4RetVal;
}

/**
 * aout s32 service routine
 *
 * @param [in]  u2Vector : s32 vector, aout1 or aour2
 * @param [out]
 *
 * @return
 */
static void AoutHal_ISR(u16 u2Vector)
{
    PAOUT_HAL_CLS prThis;
    PAUD_AOUT_INT_CTL_T prIntCtl;
    u32 u4AoutNsadr, u4DataSize;
    AUD_AOUT_DEVID eAoutId;
    PAUD_DATA_BUF_T prBuf;

    eAoutId = (AOUT1_VECTOR == u2Vector) ? AUDID_AOUT1 : AUDID_AOUT2;
    prThis = prAoutObj[eAoutId];

    if (NULL == prThis)
    {
        AOUTLOG_ERR(T("Aout hal Obj already released, but interrupt Still happen !!!  \r\n" ));
        AUD_ASSERT(0);
    }
    else
    {
        prBuf = &(prThis->rAoutCfg.rBuf);
        prIntCtl = &(prThis->rIntCtl);

        u4DataSize = AudMisc_FifoDataSize_Get(prIntCtl->u4Wp, prIntCtl->u4Rp, prBuf->u4ChBufSz);

        if ((0 == u4DataSize) && ((prIntCtl->u4IBankNum - prIntCtl->u4OBankNum) == prIntCtl->u4BankNum))
        {
            AOUTLOG_INFO(T("Aout(%d) buf full happen !!!  \r\n" ), prThis->eAoutId);

            u4DataSize = prBuf->u4ChBufSz;
        }

        //first check if enough data for next pcm burst
        if (u4DataSize <= prIntCtl->u4BankSz)
        {
            prIntCtl->u4UnderRunCnt ++;

            AOUTLOG_ERR(T("Aout(%d) buffer Is empty,under run counter is (%d) !!!  \r\n" ),prThis->eAoutId, (s32)(prIntCtl->u4UnderRunCnt));
        }
        else
        {
            prIntCtl->u4OBankNum ++;
            if (prIntCtl->u4OBankNum > prIntCtl->u4IBankNum)
            {
                AOUTLOG_ERR(T("aout(%d) input data size < output data size  \r\n" ), prThis->eAoutId);
            }

            prIntCtl->u4Rp += prIntCtl->u4BankSz;

            if (prIntCtl->u4Rp >= prBuf->u4ChBufSz)
            {
                prIntCtl->u4Rp = 0;
            }
        }

        //update next burst address
        u4AoutNsadr = prIntCtl->u4Rp + (prBuf->u4PhySddr & 0xff);
        prThis->prAoutHw->SetNsadr(prThis->prAoutHw, prBuf->u4Chn, u4AoutNsadr, prBuf->u4ChBufSz);
    }

    //callback
    if (NULL != prThis->rAoutCfg.rExtCfg.rIntCfg.PFN_ISR_CB)
    {
        prThis->rAoutCfg.rExtCfg.rIntCfg.PFN_ISR_CB(u2Vector);
    }

    //clear s32 status
    prThis->prAoutHw->SetIntClrBit(prThis->prAoutHw, FALSE);
    AudOS_IRQ_Clear(u2Vector);
    prThis->prAoutHw->SetIntClrBit(prThis->prAoutHw, TRUE);
}

/**
 * interrupt control variable init
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  0:ok, 1:fail
 */
static u32 AoutHal_IntCtrInit(PAOUT_HAL_CLS prThis)
{
    u32 u4RetVal = AUD_RET_OK;

    PAOUT_INT_CFG_T prIntCfg = &(prThis->rAoutCfg.rExtCfg.rIntCfg);
    PAUD_AOUT_INT_CTL_T prIntCtl = &(prThis->rIntCtl);
    PAUD_DATA_BUF_T prBuf = &(prThis->rAoutCfg.rBuf);

    prIntCtl->u4BankSz = prIntCfg->u4NSNum * prBuf->u4BW / 8;
    prIntCtl->u4BankNum = prBuf->u4ChBufSz / prIntCtl->u4BankSz;

    prIntCtl->u4Rp = 0;
    prIntCtl->u4Wp = 0;
    prIntCtl->u4OBankNum = 0;
    prIntCtl->u4IBankNum = 0;
    prIntCtl->u4UnderRunCnt = 0;



    AOUTLOG_INFO(T("AOUT(%d) INT CTRL INFO :  bank size : 0x%x, bank num : 0x%x  \r\n"),
                              prThis->eAoutId, (u32)(prIntCtl->u4BankSz) , (u32)(prIntCtl->u4BankNum));

    if (0 != (prBuf->u4ChBufSz % prIntCtl->u4BankSz))
    {
        AOUTLOG_ERR(T("aout(%d) buffer need be n * int_size !!!!  \r\n" ), prThis->eAoutId);
        u4RetVal = AUD_RET_FAIL;
    }

    return u4RetVal;
}

/**
 * aout interrupt isr function register
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  0:ok, 1:fail
 */
static u32 AoutHal_IntRegister(PAOUT_HAL_CLS prThis)
{
    u32 u4RetVal = AUD_RET_OK;
    u16 u2Vector;

    u2Vector = (AUDID_AOUT1 == prThis->eAoutId) ? AOUT1_VECTOR : AOUT2_VECTOR;
    if (!AudOS_ISR_Reg(u2Vector, AoutHal_ISR))
    {
        AOUTLOG_ERR(T("AOUT(%d) ISR REG Fail! \n"), prThis->eAoutId);
        u4RetVal = AUD_RET_FAIL;
    }

    AudOS_IRQ_Disable(u2Vector);
    AudOS_IRQ_Enable(u2Vector);
    AudOS_IRQ_Clear(u2Vector);

    return u4RetVal;
}
#endif

/**
 * external configure parameters check
 *
 * @param [in]  prThis : hal class, prCfg : pointer to external configure parameters
 * @param [out]
 *
 * @return 0: check ok , 2 : check fail
 */
static u32 AoutHal_ExtConfigCheck(PAOUT_HAL_CLS prThis, PAOUT_EXTPARAMS_T prCfg)
{
    u32 u4RetVal = AUD_RET_OK;

    PAOUT_EXTPARAMS_T prExtCfg = &(prThis->rAoutCfg.rExtCfg);

    AUDOS_MEMCPY(prExtCfg, prCfg, sizeof(AOUT_EXTPARAMS_T));

    if (prExtCfg->eOutPath >= AOUT_PATH_MAX)
    {
        AOUTLOG_ERR(T("Aout EXTPARA ERR: OutPath (0x%x) \r\n"), prExtCfg->eOutPath);
        prExtCfg->eOutPath = AOUT_FS;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    if (prExtCfg->eDacType > AUD_DAC_EXT)
    {
        AOUTLOG_ERR(T("Aout EXTPARA ERR: aout DAC type err (0x%x) \r\n"), prExtCfg->eDacType);
        prExtCfg->eDacType = AUD_DAC_PWM;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);

    }

    if ((AUD_DAC_EXT == prExtCfg->eDacType) && (AOUT_FS == prExtCfg->eOutPath))
    {
        if (prExtCfg->ePinMuxFsExtDac >= PINMUX_FS_I2SOUT_GROUP_MAX)
        {
            AOUTLOG_ERR(T("Aout EXTPARA ERR: FS I2S out pin mux select err (0x%x) \r\n"), prExtCfg->ePinMuxFsExtDac);
            prExtCfg->ePinMuxFsExtDac = PINMUX_FS_I2SOUT_GROUP1;

            u4RetVal = AUD_RET_PARAMS_ERR;
            AUD_ASSERT(0);
        }
    }

    if ((AUD_DAC_EXT == prExtCfg->eDacType) && (AOUT_RS == prExtCfg->eOutPath))
    {
        if (prExtCfg->ePinMuxRsExtDac >= PINMUX_RS_I2SOUT_GROUP_MAX)
        {
            AOUTLOG_ERR(T("Aout EXTPARA ERR: RS I2S out pin mux select err (0x%x) \r\n"), prExtCfg->ePinMuxRsExtDac);
            prExtCfg->ePinMuxRsExtDac = PINMUX_RS_I2SOUT_GROUP2;

            u4RetVal = AUD_RET_PARAMS_ERR;
            AUD_ASSERT(0);
        }
    }

    if (prExtCfg->eFs >= FS_UNKNOWN)
    {
        AOUTLOG_ERR(T("Aout EXTPARA ERR: sample rate (0x%x) \r\n"), prExtCfg->eFs);
        prExtCfg->eFs = FS_48K;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    if (24 != prExtCfg->u4Bps)
    {
        AOUTLOG_ERR(T("Aout EXTPARA ERR: bps set err (%d) \r\n"), (s32)(prExtCfg->u4Bps));
        prExtCfg->u4Bps = 24;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    return (u4RetVal);
}

/**
 * internal configure parameters init
 *
 * @param [in]  prAoutCfg : pointer to aout configure variable
 * @param [out]
 *
 * @return
 */
static void AoutHal_IntConfigInit(void * pThis)
{
    PAOUT_HAL_CLS prThis = (PAOUT_HAL_CLS)pThis;
    PAUD_AOUT_CFG_T prCfg = &(prThis->rAoutCfg);

    if (AOUT_FS == prCfg->rExtCfg.eOutPath)
    {
        AOUTLOG_INFO(T(" [AoutHal_Setup]  Front Seat Configure \r\n"));

        AudOS_Regkey_GetDword(AUD_DRV, (s8 *)L"AOUT_FS_MCLK", (u32 *) &(prCfg->eMclkType), AUD_MCLK_256FS);
        AudOS_Regkey_GetDword(AUD_DRV, (s8 *)L"AOUT_FS_FMT", (u32 *) &(prCfg->eFmt), AUDFMT_IIS);
        AudOS_Regkey_GetDword(AUD_DRV, (s8 *)L"AOUT_FS_DAC_BNUM", (u32 *) &(prCfg->u4DacBitNum), 24);
    }
    else
    {
        AOUTLOG_INFO(T(" [AoutHal_Setup]  Rear Seat Configure  \r\n"));

        AudOS_Regkey_GetDword(AUD_DRV, (s8 *)L"AOUT_RS_MCLK", (u32 *) &(prCfg->eMclkType), AUD_MCLK_256FS);
        AudOS_Regkey_GetDword(AUD_DRV, (s8 *)L"AOUT_RS_FMT", (u32 *) &(prCfg->eFmt), AUDFMT_IIS);
        AudOS_Regkey_GetDword(AUD_DRV, (s8 *)L"AOUT_FS_DAC_BNUM", (u32 *) &(prCfg->u4DacBitNum), 24);
    }

    prCfg->eCycle = AUD_LRCK_CYC_32;
    prCfg->fgInvertBck = TRUE;
    prCfg->fgInvertLrck = FALSE;
}

/**
 * internal configure parameters check
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return 0: check ok , 2 : check fail
 */
static u32 AoutHal_IntConfigCheck(PAOUT_HAL_CLS prThis)
{
    u32 u4RetVal = AUD_RET_OK;

    PAUD_AOUT_CFG_T prCfg = &(prThis->rAoutCfg);

    if (prCfg->eMclkType >= AUD_MCLK_TYPE_MAX)
    {
        AOUTLOG_ERR(T("Aout INTPARA ERR: aout MCLK type err (0x%x) \r\n"), prCfg->eMclkType);
        prCfg->eMclkType = AUD_MCLK_256FS;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    if ((prCfg->eFmt >= AUDFMT_UNDEF_INTF) || (prCfg->eFmt == AUDFMT_RESERVD))
    {
        AOUTLOG_ERR(T("Aout INTPARA ERR: aout format err (0x%x) \r\n"), prCfg->eFmt);
        prCfg->eFmt = AUDFMT_IIS;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    return (u4RetVal);
}


//==============================================//
    #define CodeSight_AoutHal_IF_FLOW
//==============================================//

/**
 * Hal interfac : aout hw setup
 *
 * @param [in]  prThis : hal class, prCfg : pointer to external config parameters variable
 * @param [out]
 *
 * @return 0: setup ok , 2 : setup fail
 */
static u32 AoutHal_Setup(void * pThis, void * pExtCfg)
{
    u32 u4RetVal = AUD_RET_OK;

    PAOUT_HAL_CLS prThis = (PAOUT_HAL_CLS)pThis;
    PAOUT_EXTPARAMS_T prExtCfg = (PAOUT_EXTPARAMS_T)pExtCfg;

    PAUD_AOUT_CFG_T prCfg = &(prThis->rAoutCfg);
    PDAC_EXTPARAMS_T prDacExtCfg = &(prThis->rAoutCfg.rDacExtCfg);

    //external parameters check
    u4RetVal = AoutHal_ExtConfigCheck(prThis, prExtCfg);

    //internal parameters init
    AoutHal_IntConfigInit(prThis);

    //internal parameters check
    u4RetVal = AoutHal_IntConfigCheck(prThis);

  #ifdef  AOUT_ARM_CTL
    //aout buffer allocate
    if (AUD_STATE_UNINIT == prThis->u4State)
    {
        u4RetVal = AoutHal_AllocBuf(prThis);

        //aout s32 control init
        u4RetVal = AoutHal_IntCtrInit(prThis);
    }
  #endif

    //aout mclk set
    if (AUDID_AOUT1 == prThis->eAoutId)
    {
        IoClk_SetAout1IecMclk(prCfg->eMclkType, prCfg->rExtCfg.eFs, prCfg->rExtCfg.eFs);
    }
    else
    {
        IoClk_SetAout2Mclk(prCfg->eMclkType, prCfg->rExtCfg.eFs);
    }

    //call aout init function
    prThis->prAoutHw->InitCfg(prThis->prAoutHw, prCfg);

    #ifdef  AOUT_ARM_CTL
    u4RetVal = AoutHal_IntRegister(prThis);
    #endif

    prThis->u4State = AUD_STATE_INITED;

    //dac config
    prDacExtCfg->eOutPath = prCfg->rExtCfg.eOutPath;
    prDacExtCfg->eDacType = prCfg->rExtCfg.eDacType;
    prDacExtCfg->ePinMuxFsExtDac = prCfg->rExtCfg.ePinMuxFsExtDac;
    prDacExtCfg->ePinMuxRsExtDac = prCfg->rExtCfg.ePinMuxRsExtDac;
    prDacExtCfg->eApll = IoClk_GetApllType(prCfg->rExtCfg.eFs);
    prThis->prDacHw = DacHal_New(prDacExtCfg);

    if (NULL == prThis->prDacHw)
    {
        AOUTLOG_ERR(T("Aout Hal Setup ERR: New DAC HAL Fail! \r\n"));
        u4RetVal = AUD_RET_FAIL;
    }
    else
    {
      #ifndef DAC_POWER_CTL
        prThis->prDacHw->rHwIf.Start(prThis->prDacHw, 0);
      #endif
    }

    return (u4RetVal);
}

/**
 * Hal interfac : get aout hw status
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  hw status
 */
static u32 AoutHal_GetStatus(void * pThis)
{
    PAOUT_HAL_CLS prThis = (PAOUT_HAL_CLS)pThis;

    return (prThis->u4State);
}

/**
 * Hal interfac : enable aout
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  hw status
 */
static u32 AoutHal_Start(void * pThis, u32 u4Params)
{
    PAOUT_HAL_CLS prThis = (PAOUT_HAL_CLS)pThis;

  #ifdef AUD_IO_POWER_CONTROL
    IoClk_SetModulePowerOn(((AUDID_AOUT1 == prThis->eAoutId) ? CLKPM_AUD : CLKPM_AUD2));
  #endif


  #ifdef DAC_POWER_CTL
    prThis->prDacHw->rHwIf.Start(prThis->prDacHw, 0);
  #endif

    prThis->prAoutHw->Enable(prThis->prAoutHw, TRUE);

    prThis->u4State = AUD_STATE_STARTED;

    return (prThis->u4State);
}

/**
 * Hal interfac : disable aout
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  hw status
 */
static u32 AoutHal_Stop(void * pThis, u32 u4Params)
{
    PAOUT_HAL_CLS prThis = (PAOUT_HAL_CLS)pThis;

    prThis->prAoutHw->Enable(prThis->prAoutHw, FALSE);

    prThis->rIntCtl.u4Rp = 0;
    prThis->rIntCtl.u4Wp = 0;
    prThis->rIntCtl.u4OBankNum = 0;
    prThis->rIntCtl.u4IBankNum = 0;
    prThis->rIntCtl.u4UnderRunCnt = 0;

    prThis->u4State = AUD_STATE_STOPPED;

  #ifdef DAC_POWER_CTL
    prThis->prDacHw->rHwIf.Stop(prThis->prDacHw, 0);
  #endif

  #ifdef AUD_IO_POWER_CONTROL
    IoClk_SetModulePowerDown(((AUDID_AOUT1 == prThis->eAoutId) ? CLKPM_AUD : CLKPM_AUD2));
  #endif

    return (prThis->u4State);
}


//==============================================//
    #define CodeSight_AoutHal_IF_BUF
//==============================================//

/**
 * Hal interfac : get aout buffer status
 *
 * @param [in]  prThis : hal class
 * @param [out] prBuf : buffer information
 *
 * @return  hw status
 */
static u32 AoutHal_GetBuf(void * pThis, AUD_DATA_BUF_T *prBuf)
{
    PAOUT_HAL_CLS prThis = (PAOUT_HAL_CLS)pThis;

    x_memcpy(prBuf, &(prThis->rAoutCfg.rBuf), sizeof(AUD_DATA_BUF_T));

    return (AUD_RET_OK);
}

/**
 * Hal interfac : set aout buffer write pointer
 *
 * @param [in]  prThis : hal class, u4Wp : current write pointer
 * @param [out]
 *
 * @return  0 : set ok, 1 : set fail
 */
static u32 AoutHal_SetPoint(void * pThis, u32 u4Wp)
{
    u32 u4RetVal = AUD_RET_OK;
    u32 u4NewDataSize;

    PAOUT_HAL_CLS prThis = (PAOUT_HAL_CLS)pThis;
    PAUD_AOUT_INT_CTL_T prIntCtl = &(prThis->rIntCtl);

    u4NewDataSize = AudMisc_FifoDataSize_Get(u4Wp, prIntCtl->u4Wp, prThis->rAoutCfg.rBuf.u4ChBufSz);

    if (0 != (u4Wp % prIntCtl->u4BankSz))
    {
        AOUTLOG_ERR(T("aout write pointer need be n * int_size !!!!  \r\n" ));
        u4RetVal = AUD_RET_FAIL;
    }

    prIntCtl->u4IBankNum += (u4NewDataSize / prIntCtl->u4BankSz);

    if ((prIntCtl->u4IBankNum - prIntCtl->u4OBankNum) >= prIntCtl->u4BankNum)
    {
        AOUTLOG_ERR(T("Aout input buffer overflow !!!!  \r\n" ));
    }

    prIntCtl->u4Wp = u4Wp;

    return (u4RetVal);
}

/**
 * Hal interfac : get aout buffer read pointer
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  aout buffer current read pointer
 */
static u32 AoutHal_GetPoint(void * pThis)
{
    u32 u4Rp;
    PAOUT_HAL_CLS prThis = (PAOUT_HAL_CLS)pThis;
    PAUD_AOUT_INT_CTL_T prIntCtl = &(prThis->rIntCtl);

    u4Rp = prIntCtl->u4Rp;

    return (u4Rp);
}

/**
 * Hal interfac : external hw config parameter update
 *
 * @param [in]  prThis : hal class, pCfg : external config
 * @param [out]
 *
 * @return  0 : set ok, 1 : set fail
 */
static u32 AoutHal_CfgUpd(void * pThis, void * pExtCfg)
{
    u32 u4RetVal = AUD_RET_OK;

    PAOUT_HAL_CLS prThis = (PAOUT_HAL_CLS)pThis;
    PAOUT_EXTPARAMS_T prExtCfg = (PAOUT_EXTPARAMS_T)pExtCfg;

    PAUD_AOUT_CFG_T prCfg = &(prThis->rAoutCfg);
    PDAC_EXTPARAMS_T prDacExtCfg =  &(prThis->rAoutCfg.rDacExtCfg);

    if (prCfg->rExtCfg.eFs != prExtCfg->eFs)
    {
        AOUTLOG_INFO(T("Aout(%d) New Aout Sample Rate set, FS(%d)!!! \r\n"), prThis->eAoutId, prExtCfg->eFs);
        prCfg->rExtCfg.eFs = prExtCfg->eFs;

        if (AUDID_AOUT1 == prThis->eAoutId)
        {
            IoClk_SetAout1IecMclk(prCfg->eMclkType, prCfg->rExtCfg.eFs, prCfg->rExtCfg.eFs);
        }
        else
        {
            IoClk_SetAout2Mclk(prCfg->eMclkType, prCfg->rExtCfg.eFs);
        }
    }

    if ((prCfg->rExtCfg.eOutPath != prExtCfg->eOutPath) || (prCfg->rExtCfg.eDacType != prExtCfg->eDacType))
    {
        if (prCfg->rExtCfg.eDacType != prExtCfg->eDacType)
        {
            AOUTLOG_INFO(T("Aout(%d) Dac (%d) update.\r\n"),
                prThis->eAoutId, prExtCfg->eDacType);

            prCfg->rExtCfg.eDacType = prExtCfg->eDacType;
            prDacExtCfg->eDacType = prExtCfg->eDacType;

            //update pinmux
            if(AOUT_FS==prExtCfg->eOutPath&&AUD_DAC_EXT==prDacExtCfg->eDacType)
            {
                prCfg->rExtCfg.ePinMuxFsExtDac = prExtCfg->ePinMuxFsExtDac;
                prDacExtCfg->ePinMuxFsExtDac = prExtCfg->ePinMuxFsExtDac;
            }
            else if(AOUT_RS==prExtCfg->eOutPath&&AUD_DAC_EXT==prDacExtCfg->eDacType)
            {
                prCfg->rExtCfg.ePinMuxRsExtDac = prExtCfg->ePinMuxRsExtDac;
                prDacExtCfg->ePinMuxRsExtDac = prExtCfg->ePinMuxRsExtDac;
            }

            if (NULL != prThis->prDacHw)
            {
                prThis->prDacHw->rHwIf.Stop(prThis->prDacHw, 0);
                prThis->prDacHw->Delete(prThis->prDacHw);
                prThis->prDacHw = NULL;
            }

            prThis->prDacHw = DacHal_New(prDacExtCfg);

            if (NULL == prThis->prDacHw)
            {
                AOUTLOG_ERR(T("Aout Hal CfgUpd ERR: New DAC hal fail.\r\n"));
                u4RetVal = AUD_RET_FAIL;
            }
            else
            {
              #ifndef DAC_POWER_CTL
                prThis->prDacHw->rHwIf.Start(prThis->prDacHw, 0);
              #endif
            }
        }

        if (prCfg->rExtCfg.eOutPath != prExtCfg->eOutPath)
        {
            AOUTLOG_INFO(T("Aout(%d) Front/Rear Seat Info Update (%d)!!! \r\n"), prThis->eAoutId, prExtCfg->eOutPath);
            prCfg->rExtCfg.eOutPath = prExtCfg->eOutPath;
        }

        prThis->prAoutHw->SetDacSrc(prThis->prAoutHw, prCfg->rExtCfg.eOutPath, prCfg->rExtCfg.eDacType);
    }

    if (prCfg->rExtCfg.fgAdcBypasMode != prExtCfg->fgAdcBypasMode)
    {
        AOUTLOG_INFO(T("Aout(%d) data from adc(1) or from dram(0) update,SRC(%d)!!! \r\n"), prThis->eAoutId, (s32)(prExtCfg->fgAdcBypasMode));
        prCfg->rExtCfg.fgAdcBypasMode = prExtCfg->fgAdcBypasMode;

        prThis->prAoutHw->SetDataSrc(prThis->prAoutHw, prCfg->rExtCfg.eDacType, prCfg->rExtCfg.fgAdcBypasMode);
    }

    return (u4RetVal);
}



//==============================================//
    #define CodeSight_AoutHal_Others
//==============================================//
static void AoutHal_SetArmCtrl(void * pThis, bool fgEn)
{
    PAOUT_HAL_CLS prThis = (PAOUT_HAL_CLS)pThis;

    prThis->prAoutHw->SetArmCtrl(prThis->prAoutHw, fgEn);
}


static bool AoutHal_IsArmCtrl(void * pThis)
{
    PAOUT_HAL_CLS prThis = (PAOUT_HAL_CLS)pThis;

    PAOUT_HW_CLS_PUB prHw = prThis->prAoutHw;
    bool fgIsArmCtrl = prHw->IsArmCtrl(prHw);

    return (fgIsArmCtrl);
}


static void AoutHal_SetAoutPath(void * pThis, AUD_OUT_PATH_T eAOutPath)
{
    PAOUT_HAL_CLS prThis = (PAOUT_HAL_CLS)pThis;
    PAOUT_EXTPARAMS_T prCfg = &(prThis->rAoutCfg.rExtCfg);
    if (eAOutPath != prCfg->eOutPath) {
        prCfg->eOutPath = eAOutPath;
    }
    prThis->prAoutHw->SetDacSrc(prThis->prAoutHw, prCfg->eOutPath, prCfg->eDacType);
}


//==============================================//
    #define CodeSight_AoutHal_Create
//==============================================//

/**
 * Hal interfac : Hal class delete
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  0 : set ok
 */
static u32 AoutHal_Delete(void * pThis)
{
    PAOUT_HAL_CLS prThis = (PAOUT_HAL_CLS)pThis;

    if (NULL != prThis->prAoutHw)
    {
        prThis->prAoutHw->Delete(prThis->prAoutHw);
    }

    if (NULL != prThis->prDacHw)
    {
        prThis->prDacHw->rHwIf.Stop(prThis->prDacHw, 0);
        prThis->prDacHw->Delete(prThis->prDacHw);
    }

  #ifdef  AOUT_ARM_CTL
    if (0 != prThis->rAoutCfg.rBuf.u4VirSAdr)
    {
        AudOS_Memory_Free(&(prThis->rAoutCfg.rBuf.u4VirSAdr));
    }
  #endif

    prAoutObj[prThis->eAoutId] = NULL;

    AUD_CLASS_DELETE();

    return (0);
}

/**
 * Hal interfac : Hal class create
 *
 * @param [in]  eAoutId : aout1 or aout2
 * @param [out]
 *
 * @return  pointer to aout hal pub cls
 */
PAOUT_HAL_CLS_PUB AoutHal_New(AUD_AOUT_DEVID eAoutId)
{
    PAOUT_HAL_CLS prThis = (PAOUT_HAL_CLS)kzalloc(sizeof(AOUT_HAL_CLS), GFP_KERNEL);

    if (NULL != prThis)
    {
        PAOUT_HAL_CLS_PUB prPub = &(prThis->rPub);

        prThis->u4State = AUD_STATE_UNINIT;

        prThis->eAoutId = eAoutId;
        prThis->prAoutHw = AoutHw_New(eAoutId);

        if (NULL == prThis->prAoutHw)
        {
            AOUTLOG_ERR(T("Aout(%d) New AoutHw Error!!! \r\n"), eAoutId);

            kfree(prThis);
            prThis = NULL;
        }
        else
        {
            prPub->rHwIf.Setup = AoutHal_Setup;
            prPub->rHwIf.GetStatus = AoutHal_GetStatus;
            prPub->rHwIf.Start = AoutHal_Start;
            prPub->rHwIf.Stop = AoutHal_Stop;
            prPub->rHwIf.GetBuf = AoutHal_GetBuf;
            prPub->rHwIf.GetPoint = AoutHal_GetPoint;
            prPub->rHwIf.SetPoint = AoutHal_SetPoint;
            prPub->rHwIf.CfgUpd = AoutHal_CfgUpd;

            prPub->SetArmCtrl = AoutHal_SetArmCtrl;
            prPub->IsArmCtrl = AoutHal_IsArmCtrl;
            prPub->SetAoutPath = AoutHal_SetAoutPath;
            prPub->IntConfigInit = AoutHal_IntConfigInit;
            prPub->Delete = AoutHal_Delete;

            prAoutObj[eAoutId] = prThis;
        }
    }
    else
    {
        AOUTLOG_ERR(T("Aout(%d) New AoutHal Error!!! \r\n"), eAoutId);
    }

    return ((PAOUT_HAL_CLS_PUB)prThis);
}

