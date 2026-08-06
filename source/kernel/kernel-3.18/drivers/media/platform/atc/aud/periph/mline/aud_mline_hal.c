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
 * @file aud_mline_hal.c source file
 *
 * aud io mlin module hardware abstraction layer
 *
 * @author qiuhua.yin@autochips.com
 *
 */

#include "aud_mline_hal.h"
#include "aud_if.h"

PMLIN_HAL_CLS prMlinObj = NULL;

u32 _u4MlinLog = ALOG_DEFAULT;

//==============================================//
    #define CodeSight_MlinHal_static_FUNC
//==============================================//

/**
 * allocate buffer for mlin
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return
 */
static u32 MlinHal_AllocBuf(PMLIN_HAL_CLS prThis)
{
    u32 u4RetVal = AUD_RET_OK;
    u32 u4MlinMemSize;

    PAUD_MLIN_CFG_T prMlinCfg = &(prThis->rCfg);

    u4MlinMemSize = prMlinCfg->rExtCfg.u4BufSz;

    if (0 != prMlinCfg->rExtCfg.u4BufPhyAdr)
    {
        MLINLOG_INFO(T("Mlin buffer allocated by up layer  \r\n" ));

        prMlinCfg->rBuf.u4PhySddr = prMlinCfg->rExtCfg.u4BufPhyAdr;
        prMlinCfg->rBuf.u4VirSAdr = 0;
    }
    else
    {
        MLINLOG_INFO(T("Mlin buffer allocated by hal layer  \r\n" ));

        prMlinCfg->rBuf.u4VirSAdr = AudOS_Memory_Alloc(u4MlinMemSize, MLIN_BUF_ALIGN, &(prMlinCfg->rBuf.u4PhySddr));
        if (0 == prMlinCfg->rBuf.u4VirSAdr)
        {
            MLINLOG_ERR(T("Mlin buffer allocated Err !!!!  \r\n" ));
            u4RetVal = AUD_RET_FAIL;
        }
    }

    prMlinCfg->rBuf.u4Chn = (prMlinCfg->rExtCfg.eMlinChNum + 1) * 2;
    prMlinCfg->rBuf.u4ChBufSz = u4MlinMemSize / prMlinCfg->rBuf.u4Chn;

    prMlinCfg->rBuf.u4BW = prMlinCfg->rExtCfg.u4SrcBitNum;
    prMlinCfg->rBuf.u4DataOff = 0;


    MLINLOG_INFO(T("MLIN BUF INFO:  channel size : 0x%x, channel num : 0x%x, buf size: 0x%x \r\n"),
                    (u32)(prMlinCfg->rBuf.u4ChBufSz), (u32)(prMlinCfg->rBuf.u4Chn), (u32)u4MlinMemSize);
    MLINLOG_INFO(T("MLIN BUF INFO:  buf phy adr : 0x%x, buf vir adr : 0x%x \r\n"),
                    (u32)(prMlinCfg->rBuf.u4PhySddr), (u32)(prMlinCfg->rBuf.u4VirSAdr));

    return u4RetVal;
}

/**
 * mlin s32 service routine
 *
 * @param [in]  u2Vector : s32 vector
 * @param [out]
 *
 * @return
 */
static void MlinHal_ISR(u16 u2Vector)
{
    PMLIN_HAL_CLS prThis;

    prThis = prMlinObj;

    if (NULL == prThis)
    {
        MLINLOG_ERR(T("MLin hal Obj already released, but interrupt Still happen !!!  \r\n" ));
        AUD_ASSERT(0);
    }
    else
    {
        //callback
        if (NULL != prThis->rCfg.rExtCfg.PFN_ISR_CB)
        {
            prThis->rCfg.rExtCfg.PFN_ISR_CB(u2Vector);
        }

        //clear s32 status
        AudOS_IRQ_Clear(u2Vector);
    }
}

/**
 * interrup control varibal init
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return 0: ok, others: ng
 */
static u32 MlinHal_IntCtrInit(PMLIN_HAL_CLS prThis)
{
    u32 u4RetVal = AUD_RET_OK;

    AudOS_IRQ_Disable(MLIN_VECTOR);

    if (!AudOS_ISR_Reg(MLIN_VECTOR, MlinHal_ISR))
    {
        MLINLOG_ERR(T("MLin ISR REG Fail! \n"));
        u4RetVal = AUD_RET_FAIL;
    }

    AudOS_IRQ_Enable(MLIN_VECTOR);
    AudOS_IRQ_Clear(MLIN_VECTOR);

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
static u32 MlinHal_ExtConfigCheck(PMLIN_HAL_CLS prThis, PMLIN_EXTPARAMS_T prCfg)
{
    u32 u4RetVal = AUD_RET_OK;

    PMLIN_EXTPARAMS_T prMlinExtCfg = &(prThis->rCfg.rExtCfg);

    AUDOS_MEMCPY(prMlinExtCfg, prCfg, sizeof(MLIN_EXTPARAMS_T));

    if (prMlinExtCfg->u4SrcBitNum > 24)
    {
        MLINLOG_ERR(T("Mlin EXTPARA ERR: src bit num (0x%x) \r\n"), (u32)(prMlinExtCfg->u4SrcBitNum));
        prMlinExtCfg->u4SrcBitNum = 24;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    if (prMlinExtCfg->eCycle > AUD_LRCK_CYC_32)
    {
        MLINLOG_ERR(T("Mlin EXTPARA ERR: lrck cycle (0x%x) \r\n"), prMlinExtCfg->eCycle);
        prMlinExtCfg->eCycle = AUD_LRCK_CYC_32;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    if ((prMlinExtCfg->eDataFmt >= AUDFMT_UNDEF_INTF) || (prMlinExtCfg->eDataFmt == AUDFMT_RESERVD))
    {
        MLINLOG_ERR(T("Mlin EXTPARA ERR: data format (0x%x) \r\n"), prMlinExtCfg->eDataFmt);
        prMlinExtCfg->eDataFmt = AUDFMT_IIS;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    if (prMlinExtCfg->eMlinChNum >= MLIN_CHNUM_MAX)
    {
        MLINLOG_ERR(T("Mlin EXTPARA ERR: channel number (0x%x) \r\n"), prMlinExtCfg->eMlinChNum);
        prMlinExtCfg->eMlinChNum = MLIN_CHNUM_2;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    if (prMlinExtCfg->eIntPeriod >= MLIN_INTPERID_MAX)
    {
        MLINLOG_ERR(T("Mlin EXTPARA ERR: s32 period (0x%x) \r\n"), prMlinExtCfg->eIntPeriod);
        prMlinExtCfg->eIntPeriod = MLIN_INTPERID_256DW;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }

    if (NULL == prMlinExtCfg->PFN_ISR_CB)
    {

        MLINLOG_WARN(T("Mlin EXTPARA WARN: s32 callback function is NULL  \r\n"));
    }

    return (u4RetVal);
}

/**
 * internal configure parameters init
 *
 * @param [in]  prThis : pointer to mlin hal object
 * @param [out]
 *
 * @return
 */
static void MlinHal_IntConfigInit(PMLIN_HAL_CLS prThis)
{
    PAUD_MLIN_CFG_T prCfg = &(prThis->rCfg);

    //prCfg->eSrc = MLIN_SRC_HDMI_RX;

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
static u32 MlinHal_IntConfigCheck(PMLIN_HAL_CLS prThis)
{
    u32 u4RetVal = AUD_RET_OK;
    #if 0
    PAUD_MLIN_CFG_T prCfg = &(prThis->rCfg);

    if (prCfg->eSrc >= MLIN_SRC_MAX)
    {
        MLINLOG_ERR(T("Mlin INTPARA ERR: Mlin src (0x%x) \r\n"), prCfg->eSrc);
        prCfg->eSrc = MLIN_SRC_HDMI_RX;

        u4RetVal = AUD_RET_PARAMS_ERR;
        AUD_ASSERT(0);
    }
    #endif

    return (u4RetVal);
}


//==============================================//
    #define CodeSight_MlinHal_IF_FLOW
//==============================================//

/**
 * Hal interfac : mlin hw setup
 *
 * @param [in]  prThis : hal class, prCfg : pointer to external config parameters variable
 * @param [out]
 *
 * @return 0: setup ok , 2 : setup fail
 */
static u32 MlinHal_Setup(void * pThis, void * pCfg)
{
    PMLIN_HAL_CLS prThis = (PMLIN_HAL_CLS)pThis;
    PMLIN_EXTPARAMS_T prCfg = (PMLIN_EXTPARAMS_T)pCfg;

    u32 u4RetVal = AUD_RET_OK;

    PAUD_MLIN_CFG_T prMlinCfg = &(prThis->rCfg);

    //external parameters check
    u4RetVal = MlinHal_ExtConfigCheck(prThis, prCfg);

    //internal parameters init
    MlinHal_IntConfigInit(prThis);

    //internal parameters check
    u4RetVal = MlinHal_IntConfigCheck(prThis);

    if (AUD_STATE_STARTED != prThis->u4State)
    {
        //mlin buffer allocate
        u4RetVal = MlinHal_AllocBuf(prThis);

        //mlin s32 control init
      #ifndef MLIN_SIMULATE
        u4RetVal = MlinHal_IntCtrInit(prThis);
      #endif
    }

    //call mlin init function
    prThis->prMlinHw->InitCfg(prThis->prMlinHw, prMlinCfg);

    prThis->u4State = AUD_STATE_INITED;

    return (u4RetVal);
}

/**
 * Hal interfac : get mlin hw status
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  hw status
 */
static u32 MlinHal_GetStatus(void * pThis)
{
    PMLIN_HAL_CLS prThis = (PMLIN_HAL_CLS)pThis;

    return (prThis->u4State);
}

/**
 * Hal interfac : enable mlin
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  hw status
 */
static u32 MlinHal_Start(void * pThis, u32 u4Params)
{
    PMLIN_HAL_CLS prThis = (PMLIN_HAL_CLS)pThis;

    MLINLOG_INFO(T("MlinHal_Start \n"));

    prThis->prMlinHw->Enable(prThis->prMlinHw, TRUE);

    prThis->u4State = AUD_STATE_STARTED;

    return (prThis->u4State);
}

/**
 * Hal interfac : disable mlin
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  hw status
 */
static u32 MlinHal_Stop(void * pThis, u32 u4Params)
{
    PMLIN_HAL_CLS prThis = (PMLIN_HAL_CLS)pThis;

    MLINLOG_INFO(T("MlinHal_Stop \n"));

    prThis->prMlinHw->Enable(prThis->prMlinHw, FALSE);

    prThis->u4State = AUD_STATE_STOPPED;

    return (prThis->u4State);
}


//==============================================//
    #define CodeSight_MlinHal_IF_BUF
//==============================================//

/**
 * Hal interfac : get mlin buffer config
 *
 * @param [in]  prThis : hal class
 * @param [out] prBuf : buffer information
 *
 * @return  hw status
 */
static u32 MlinHal_GetBuf(void * pThis, PAUD_DATA_BUF_T prBuf)
{
    PMLIN_HAL_CLS prThis = (PMLIN_HAL_CLS)pThis;

    x_memcpy(prBuf, &(prThis->rCfg.rBuf), sizeof(AUD_DATA_BUF_T));

    return (AUD_RET_OK);
}

/**
 * Hal interfac : set mlin buffer read pointer
 *
 * @param [in]  prThis : hal class, u4Rp : current read pointer
 * @param [out]
 *
 * @return  0 : set ok, 1 : set fail
 */
static u32 MlinHal_SetPoint(void * pThis, u32 u4Rp)
{
    u32 u4RetVal = AUD_RET_OK;

    return (u4RetVal);
}

/**
 * Hal interfac : get mlin buffer write pointer
 *
 * @param [in]  prThis : hal class, u4Wp : current write pointer
 * @param [out]
 *
 * @return  0 : set ok, 1 : set fail
 */
static u32 MlinHal_GetPoint(void * pThis)
{
    PMLIN_HAL_CLS prThis = (PMLIN_HAL_CLS)pThis;
    u32 u4MlinWp;

    u4MlinWp  = prThis->prMlinHw->GetWp(prThis->prMlinHw);

    //MLINLOG_INFO(T("MlinHal_GetPoint (0x%x) \n"), u4MlinWp);

    return (u4MlinWp);
}

/**
 * Hal interfac : get mlin spdif info
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  spdif type that multi line in hw detect
 */
static u32 MlinHal_GetSpdifInfo(void * pThis, void * pSpdifInfo)
{
    PMLIN_HAL_CLS prThis = (PMLIN_HAL_CLS)pThis;
    PMLIN_SPDIF_INFO_T prSpdifInfo = (PMLIN_SPDIF_INFO_T)pSpdifInfo;
    u32 u4RetVal = AUD_RET_OK;

    prThis->prMlinHw->GetSpdifInfo(prThis->prMlinHw, prSpdifInfo);

    return (u4RetVal);
}


/**
 * Hal interfac : get mlin spdif info
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  spdif type that multi line in hw detect
 */
static u32 MlinHal_ClrSpdTypeDec(void * pThis, u8 u1Val)
{
    PMLIN_HAL_CLS prThis = (PMLIN_HAL_CLS)pThis;
    u32 u4RetVal = AUD_RET_OK;

    prThis->prMlinHw->ClrSpdTypeDec(prThis, u1Val);

    return (u4RetVal);
}


/**
 * Hal interfac : on/off register
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  spdif type that multi line in hw detect
 */
static u32 MlinHal_SetIRQOnOff(void * pThis,x_os_isr_fct pfnIsr, bool fgOnOff)
{
    u32 u4RetVal = AUD_RET_OK;
    //x_os_isr_fct pfnOldIsr;
    if(TRUE == fgOnOff)
    {
        AudOS_ISR_Reg(MLIN_VECTOR, pfnIsr);
        AudOS_IRQ_Enable(MLIN_VECTOR);
        AudOS_IRQ_Clear(MLIN_VECTOR);
    }
    else
    {
        AudOS_ISR_UnReg(MLIN_VECTOR);
    }

    return (u4RetVal);
}


/**
 * Hal interfac : Set mlin source info
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  spdif type that multi line in hw detect
 */
static u32 MlinHal_SetSrcType(void * pThis, AUD_MLIN_SRC eSrc)
{
    PMLIN_HAL_CLS prThis = (PMLIN_HAL_CLS)pThis;
    u32 u4RetVal = AUD_RET_OK;

    prThis->prMlinHw->SetSrc(prThis->prMlinHw, eSrc);

    return (u4RetVal);
}


/**
 * Hal interfac : external hw config parameter update
 *
 * @param [in]  prThis : hal class, pCfg : external config
 * @param [out]
 *
 * @return  0 : set ok, 1 : set fail
 */
static u32 MlinHal_CfgUpd(void * pThis, void * pCfg)
{
    PMLIN_HAL_CLS prThis = (PMLIN_HAL_CLS)pThis;
    PMLIN_EXTPARAMS_T prCfg = (PMLIN_EXTPARAMS_T)pCfg;

    u32 u4RetVal = AUD_RET_OK;

    if(prThis->rCfg.rExtCfg.u4SrcBitNum != prCfg->u4SrcBitNum)
    {
        prThis->rCfg.rExtCfg.u4SrcBitNum = prCfg->u4SrcBitNum;
        prThis->prMlinHw->SetSrcBitNum(prThis, prCfg->u4SrcBitNum);
    }

    if(prThis->rCfg.rExtCfg.eOutBitNum != prCfg->eOutBitNum)
    {
        prThis->rCfg.rExtCfg.eOutBitNum = prCfg->eOutBitNum;
        prThis->prMlinHw->SetOutBitNum(prThis, prCfg->eOutBitNum);
    }

    if(prThis->rCfg.rExtCfg.eDataFmt != prCfg->eDataFmt)
    {
        prThis->rCfg.rExtCfg.eDataFmt = prCfg->eDataFmt;
        prThis->prMlinHw->SetDataFmt(prThis, prCfg->eDataFmt);
    }

    if(prThis->rCfg.rExtCfg.eMlinChNum != prCfg->eMlinChNum)
    {
        prThis->rCfg.rExtCfg.eMlinChNum = prCfg->eMlinChNum;
        prThis->prMlinHw->SetChNum(prThis, prCfg->eMlinChNum);
    }

    if(prThis->rCfg.rExtCfg.eIntPeriod != prCfg->eIntPeriod)
    {
        prThis->rCfg.rExtCfg.eIntPeriod = prCfg->eIntPeriod;
        prThis->prMlinHw->SetIntPeriod(prThis, prCfg->eIntPeriod);
    }

    return u4RetVal;
}



//==============================================//
    #define CodeSight_MlinHal_Others
//==============================================//



//==============================================//
    #define CodeSight_MlinHal_Create
//==============================================//

/**
 * Hal interfac : Hal class delete
 *
 * @param [in]  prThis : hal class
 * @param [out]
 *
 * @return  0 : set ok
 */
static u32 MlinHal_Delete(void * pThis)
{
    PMLIN_HAL_CLS prThis = (PMLIN_HAL_CLS)pThis;
    u32 u4RetVal = AUD_RET_OK;

    MLINLOG_INFO(T("MlinHal_Delete \n"));

    if (NULL != prThis->prMlinHw)
    {
        prThis->prMlinHw->Delete(prThis->prMlinHw);
    }

    if (0 != prThis->rCfg.rBuf.u4VirSAdr)
    {
        AudOS_Memory_Free(&(prThis->rCfg.rBuf.u4VirSAdr));
    }

    prMlinObj = NULL;

    AUD_CLASS_DELETE();

    return (u4RetVal);
}

/**
 * Hal interfac : Hal class create
 *
 * @param [in]
 * @param [out]
 *
 * @return  pointer to multi line in hal pub cls
 */
PMLIN_HAL_CLS_PUB MlinHal_New(void)
{
    PMLIN_HAL_CLS prThis = (PMLIN_HAL_CLS)kzalloc(sizeof(MLIN_HAL_CLS), GFP_KERNEL);

    MLINLOG_INFO(T("MlinHal_New \n"));

    if (prThis)
    {
        PMLIN_HAL_CLS_PUB prPub = &(prThis->rPub);

        prThis->u4State = AUD_STATE_UNINIT;

        prThis->prMlinHw = MlinHw_New();

        if (NULL == prThis->prMlinHw)
        {
            MLINLOG_ERR(T("Mlin New MlinHw Error!!! \r\n"));

            kfree(prThis);
            prThis = NULL;
        }
        else
        {
            prMlinObj = prThis;

            prPub->rHwIf.Setup = MlinHal_Setup;
            prPub->rHwIf.GetStatus = MlinHal_GetStatus;
            prPub->rHwIf.Start = MlinHal_Start;
            prPub->rHwIf.Stop = MlinHal_Stop;
            prPub->rHwIf.GetBuf = MlinHal_GetBuf;
            prPub->rHwIf.GetPoint = MlinHal_GetPoint;
            prPub->rHwIf.SetPoint = MlinHal_SetPoint;
            prPub->rHwIf.CfgUpd = MlinHal_CfgUpd;

            prPub->Delete = MlinHal_Delete;
            prPub->GetSpdifType = MlinHal_GetSpdifInfo;
            prPub->ClrSpdTypeDec = MlinHal_ClrSpdTypeDec;
            prPub->SetIRQOnOff = MlinHal_SetIRQOnOff;
            prPub->SetSrcType = MlinHal_SetSrcType;
        }
    }
    else
    {
        MLINLOG_ERR(T("Mlin New MlinHal Error!!! \r\n"));
    }

    return ((PMLIN_HAL_CLS_PUB)prThis);
}

