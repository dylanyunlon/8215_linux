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
 * @file aud_pcm_hal.c source file
 * 
 * aud io pcm module hardware abstraction layer
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */
#include <linux/interrupt.h>

#include "aud_oal.h"
#include "aud_pcm_hal.h"
#include "aud_io_pinmux_if.h"

u32 _u4PcmLog = ALOG_DEFAULT;
PPCM_HAL_CLS prPcmObj = NULL;


//==============================================//
    #define CodeSight_PcmHal_static_FUNC
//==============================================//

static VOID PcmHw_SetIntClrBit(BOOL fgOn)
{
	//AUDREG_BITS_W(REGENV_RGBK2_CFG5, BIT_STR_PCM_INT_CLR, BIT_NUM_PCM_INT_CLR, fgOn);
	AUDREG_BITS_W(0xa8094, 5, 1, fgOn);
}

/**
 * allocate buffer for pcm hw
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return 0 : allocate ok, others : error happen
 */
static u32 PcmHal_AllocBuf(PPCM_HAL_CLS prThis)
{
    u32 u4MemSize;
    uintptr_t	u4TxSadr, u4RxSadr;
    u32 u4RetVal = AUD_RET_OK;

    PAUD_PCM_CFG_T prCfg = &(prThis->rCfg);
    PPCM_EXTPARAMS_T prExtCfg = &(prThis->rCfg.rExtCfg);

    u4MemSize = prExtCfg->u4RxBufSz + prExtCfg->u4TxBufSz;

    if (0 != prExtCfg->u4BufPhyAdr)
    {
        PCMLOG_INFO((T("Pcm buffer allocated by up layer  \r\n" )));

        u4TxSadr = prExtCfg->u4BufPhyAdr;
        u4RxSadr = prExtCfg->u4BufPhyAdr + prExtCfg->u4TxBufSz;
        
        prCfg->rTxBuf.u4PhySddr = u4TxSadr;
        prCfg->rTxBuf.u4VirSAdr = 0;

        prCfg->rRxBuf.u4PhySddr = u4RxSadr;
        prCfg->rRxBuf.u4VirSAdr = 0;
    }
    else
    {
        PCMLOG_INFO((T("Pcm buffer allocated by hal layer  \r\n" )));
        #ifndef __linux__
        prCfg->rTxBuf.u4VirSAdr = AudOS_Memory_Alloc(u4MemSize, PCM_BUF_ALIGN, &(prCfg->rTxBuf.u4PhySddr));

        if (0 == prCfg->rTxBuf.u4VirSAdr)
        {
            PCMLOG_ERR((T("Pcm buffer allocated Err !!!!  \r\n" )));
            u4RetVal = AUD_RET_FAIL;
        }
        #else
        prCfg->rTxBuf.u4VirSAdr = AC83XX_BT_PCM_VA;
        prCfg->rTxBuf.u4PhySddr = AC83XX_BT_PCM_PA;
        /*prCfg->rTxBuf.u4VirSAdr = AudOS_Memory_Alloc(u4MemSize, PCM_BUF_ALIGN, &(prCfg->rTxBuf.u4PhySddr));

        if (0 == prCfg->rTxBuf.u4VirSAdr)
        {
            PCMLOG_ERR((T("Pcm buffer allocated Err !!!!  \r\n" )));
            u4RetVal = AUD_RET_FAIL;
        }*/
        #endif
        prCfg->rRxBuf.u4PhySddr = prCfg->rTxBuf.u4PhySddr + prExtCfg->u4TxBufSz;
        prCfg->rRxBuf.u4VirSAdr = prCfg->rTxBuf.u4VirSAdr + prExtCfg->u4TxBufSz;
    }

    prCfg->rTxBuf.u4Chn = 1;
    prCfg->rTxBuf.u4ChBufSz = prExtCfg->u4TxBufSz;
    prCfg->rTxBuf.u4BW = 16;
    prCfg->rTxBuf.u4DataOff = 0;

    prCfg->rRxBuf.u4Chn = 1;
    prCfg->rRxBuf.u4ChBufSz = prExtCfg->u4RxBufSz;
    prCfg->rRxBuf.u4BW = 16;
    prCfg->rRxBuf.u4DataOff = 0;


    PCMLOG_INFO((T("PCM TX BUF INFO:  channel size : 0x%x, channel num : 0x%x, buf size: 0x%x \r\n"),
                  (u32)(prCfg->rTxBuf.u4ChBufSz), (u32)(prCfg->rTxBuf.u4Chn), (u32)(prExtCfg->u4TxBufSz)));
    PCMLOG_INFO((T("PCM TX BUF INFO:  buf phy adr : 0x%lx, buf vir adr : 0x%lx \r\n"),
                  (uintptr_t)(prCfg->rTxBuf.u4PhySddr), (uintptr_t)(prCfg->rTxBuf.u4VirSAdr)));

    PCMLOG_INFO((T("PCM RX BUF INFO:  channel size : 0x%x, channel num : 0x%x, buf size: 0x%x \r\n"),
                  (u32)(prCfg->rRxBuf.u4ChBufSz), (u32)(prCfg->rRxBuf.u4Chn), (u32)(prExtCfg->u4RxBufSz)));
    PCMLOG_INFO((T("PCM RX BUF INFO:  buf phy adr : 0x%lx, buf vir adr : 0x%lx \r\n"),
                  (uintptr_t)(prCfg->rRxBuf.u4PhySddr), (uintptr_t)(prCfg->rRxBuf.u4VirSAdr)));

    return u4RetVal;
}

/**
 * pcm s32 service routine
 *
 * @param [in]  u2Vector : s32 vector
 * @param [out] 
 *
 * @return
 */
static irqreturn_t PcmHal_ISR(u16 u2Vector,void* tmp)
//static void PcmHal_ISR(u16 u2Vector)
{
    PPCM_HAL_CLS prThis;
    PAUD_PCM_INT_CTL_T prIntCtl;
    u32 u4TxNsadr, u4DataSize;

    prThis = prPcmObj;
    
    if (NULL == prThis)
    {
        PCMLOG_ERR((T("Pcm hal Obj already released, but interrupt Still happen!!!\r\n" )));
        ASSERT(0);
    }
    else
    {
        prIntCtl = &(prThis->rTxIntCtl);

        u4DataSize = AudMisc_FifoDataSize_Get(prIntCtl->u4Wp, prIntCtl->u4Rp, prThis->rCfg.rTxBuf.u4ChBufSz);

        if ((0 == u4DataSize) && ((prIntCtl->u4IBankNum - prIntCtl->u4OBankNum) == prIntCtl->u4BankNum))
        {
            PCMLOG_INFO((T("Pcm Tx buf full happen !!!  \r\n" )));
            
            u4DataSize = prThis->rCfg.rTxBuf.u4ChBufSz;
        }
        
        //first check if enough data for next pcm burst
        if (PCM_NORMAL_MODE == prThis->rCfg.rExtCfg.eHwMode)
        {
            if (u4DataSize <= prIntCtl->u4BankSz)
            {
                prIntCtl->u4UnderRunCnt ++;
                
                PCMLOG_ERR((T("Pcm Tx Outbuf Is empty,under run counter is (%d)!!!\r\n" ), (u32)prIntCtl->u4UnderRunCnt));
            }
            else
            {
                prIntCtl->u4OBankNum ++;
                if (prIntCtl->u4OBankNum > prIntCtl->u4IBankNum)
                {
                    PCMLOG_ERR((T("pcm tx input data size < output data same size  \r\n" )));
                }
                
                prIntCtl->u4Rp += prIntCtl->u4BankSz;
            }
        }
        else  //loop mode
        {
            prIntCtl->u4Rp += prIntCtl->u4BankSz;
        }

        if (prIntCtl->u4Rp >= prThis->rCfg.rTxBuf.u4ChBufSz)
        {
            prIntCtl->u4Rp = 0;
        }
 		#if CONFIG_DRV_AUD_AC83XX
        //update next burst address
        u4TxNsadr = prIntCtl->u4Rp + (prThis->rCfg.rTxBuf.u4PhySddr & 0xf0);
		#else
		u4TxNsadr = prIntCtl->u4Rp + (prThis->rCfg.rTxBuf.u4PhySddr & 0xffff0);
		#endif
        prThis->prPcmHw->SetNsadr(prThis->prPcmHw, (u4TxNsadr >> 4));
    }

    //callback
    if (NULL != prThis->rCfg.rExtCfg.rIntCfg.PFN_ISR_CB)
    {
        prThis->rCfg.rExtCfg.rIntCfg.PFN_ISR_CB(u2Vector);
    }

    //clear s32 status
    #if CONFIG_DRV_AUD_AC83XX
    AudOS_IRQ_Clear(u2Vector);
	#else
    PcmHw_SetIntClrBit(FALSE);
    AudOS_IRQ_Clear(u2Vector);
	PcmHw_SetIntClrBit(TRUE);
	#endif

	return IRQ_HANDLED;
}

/**
 * pcm interrup control varibal init
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return 0: ok, others: ng
 */
static u32 PcmHal_IntCtrInit(PPCM_HAL_CLS prThis)
{
    u32 u4RetVal = AUD_RET_OK;
    
    PPCM_INT_CFG_T prIntCfg = &(prThis->rCfg.rExtCfg.rIntCfg);
    PAUD_PCM_INT_CTL_T prIntCtl = &(prThis->rTxIntCtl);
    PAUD_DATA_BUF_T prTxBuf = &(prThis->rCfg.rTxBuf);

    prIntCtl->u4BankSz = prIntCfg->u4IntSz * prTxBuf->u4BW / 8;
    prIntCtl->u4BankNum = prTxBuf->u4ChBufSz / prIntCtl->u4BankSz;
    
    prIntCtl->u4Rp = 0;
    prIntCtl->u4Wp = 0;
    prIntCtl->u4OBankNum = 0;
    prIntCtl->u4IBankNum = 0;
    prIntCtl->u4UnderRunCnt = 0;

    PCMLOG_INFO((T("PCM TX INT CTRL INFO :  bank size : 0x%x, bank num : 0x%x \r\n"),
                  (u32)prIntCtl->u4BankSz , (u32)prIntCtl->u4BankNum));

    if (0 != (prTxBuf->u4ChBufSz % prIntCtl->u4BankSz))
    {
        PCMLOG_ERR((T("pcm tx buffer need be n * int_size !!!!  \r\n" )));
        u4RetVal = AUD_RET_FAIL;
    }

    //AudOS_IRQ_Disable(VECTOR_AOUT_BT_RC);   
    if (!AudOS_ISR_Reg(VECTOR_AOUT_BT_RC, PcmHal_ISR))
    {                     
        PCMLOG_ERR((T("PCM TX ISR REG Fail! \n")));
        u4RetVal = AUD_RET_FAIL;
    }
    //AudOS_IRQ_Enable(VECTOR_AOUT_BT_RC); 
    //AudOS_IRQ_Clear(VECTOR_AOUT_BT_RC);

    #ifdef __linux__
    {
        u32 u4Value;
        //Change PCM Isr Level Trigger to Edge Trigger
    #if CONFIG_DRV_AUD_AC83XX
		AudOS_IRQ_Enable(VECTOR_AOUT_BT_RC); 
        AudOS_IRQ_Clear(VECTOR_AOUT_BT_RC);
        u4Value = IO_READ32(0xFE000000, 0x1C2C);
        u4Value |= 0x3<<20;
        IO_WRITE32(0xFE000000, 0x1C2C, u4Value);
    #else
	    //AudOS_IRQ_Enable(VECTOR_AOUT_BT_RC);
	    //AudOS_IRQ_Clear(VECTOR_AOUT_BT_RC);
	    //PcmHw_SetIntClrBit(TRUE);
    #endif
	
    }
    #endif

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
static u32 PcmHal_ExtConfigCheck(PPCM_HAL_CLS prThis, PPCM_EXTPARAMS_T prCfg)
{
    u32 u4RetVal = AUD_RET_OK;
    
    PPCM_EXTPARAMS_T prExtCfg = &(prThis->rCfg.rExtCfg);

    AUDOS_MEMCPY(prExtCfg, prCfg, sizeof(PCM_EXTPARAMS_T));

    if (prExtCfg->eSyncCycle > PCM_CLK_CYCLE_64)
    {
        PCMLOG_ERR((T("Pcm EXTPARA ERR: Sync Cycle (0x%x) \r\n"), (u32)prExtCfg->eSyncCycle));
        prExtCfg->eSyncCycle = PCM_CLK_CYCLE_64;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    if (prExtCfg->eSyncMode > PCM_LONG_MODE)
    {
        PCMLOG_ERR((T("Pcm EXTPARA ERR: Sync Mode (0x%x) \r\n"), (u32)prExtCfg->eSyncMode));
        prExtCfg->eSyncMode = PCM_LONG_MODE;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
    }

    if ((0 != (prExtCfg->u4RxBufSz % PCM_BUF_ALIGN)) || (0 != (prExtCfg->u4TxBufSz % PCM_BUF_ALIGN)))
    {
        PCMLOG_ERR((T("Pcm EXTPARA ERR: Tx(0x%x) / Rx(0x%x) Buf Size, need (%d) Byte Align  \r\n"), 
                    (u32)prExtCfg->u4TxBufSz, (u32)prExtCfg->u4RxBufSz, PCM_BUF_ALIGN));
        prExtCfg->u4TxBufSz = (prExtCfg->u4TxBufSz / PCM_BUF_ALIGN) * PCM_BUF_ALIGN;
        prExtCfg->u4RxBufSz = (prExtCfg->u4RxBufSz / PCM_BUF_ALIGN) * PCM_BUF_ALIGN;

        u4RetVal = AUD_RET_PARAMS_ERR;
        ASSERT(0);
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
static void PcmHal_IntConfigInit(PPCM_HAL_CLS prThis)
{
   PAUD_PCM_CFG_T prCfg  = &(prThis->rCfg);

   prCfg->eDataOrder = PCM_MSB_FIRST;
   prCfg->eBitNum = PCM_BITS_16;
   prCfg->eBitMode = PCM_LINEAR_16BIT;
   prCfg->eSyncLength = PCM_CLK_LENGTH2;
   prCfg->eMode = PCM_MASTER;
   prCfg->fgSignEn = FALSE;
   prCfg->fgInvertClkIn = FALSE;
   prCfg->fgInvertClkOut = FALSE;
}

/**
 * internal configure parameters check
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return 0: check ok , 2 : check fail
 */
static u32 PcmHal_IntConfigCheck(PPCM_HAL_CLS prThis)
{
    u32 u4RetVal = AUD_RET_OK;
    
    return (u4RetVal);
}


//==============================================//
    #define CodeSight_PcmHal_IF_FLOW
//==============================================//

/**
 * Hal interfac : pcm hw setup
 *
 * @param [in]  prThis : hal class, prCfg : pointer to external config parameters variable 
 * @param [out] 
 *
 * @return 0: setup ok , 2 : setup fail
 */
static u32 PcmHal_Setup(void * pThis, void * pExtCfg)
{
    PPCM_HAL_CLS prThis = (PPCM_HAL_CLS)pThis;
    PPCM_EXTPARAMS_T prExtCfg = (PPCM_EXTPARAMS_T)pExtCfg;

    u32 u4RetVal = AUD_RET_OK;        
    PAUD_PCM_CFG_T prCfg = &(prThis->rCfg);
    
    //external parameters check
    u4RetVal = PcmHal_ExtConfigCheck(prThis, prExtCfg);

    //internal parameters init
    PcmHal_IntConfigInit(prThis);

    //internal parameters check
    u4RetVal = PcmHal_IntConfigCheck(prThis);

    if (AUD_STATE_UNINIT == prThis->u4PcmState)
    {
        //pcm buffer allocate
        u4RetVal = PcmHal_AllocBuf(prThis);

        //pcm interrupt control init
        u4RetVal = PcmHal_IntCtrInit(prThis);
    }
    
    //call pcm init function
    prThis->prPcmHw->InitCfg(prThis->prPcmHw, prCfg);

    IoPinMux_SetPcm(PINMUX_PCM_GROUP1);

    prThis->u4PcmState = AUD_STATE_INITED;
    prThis->u4RxState= AUD_STATE_INITED;
    prThis->u4TxState= AUD_STATE_INITED;
        
    return (u4RetVal);
}

/**
 * Hal interfac : pcm hw config update
 *
 * @param [in]  prThis : hal class, prCfg : pointer to external config parameters variable 
 * @param [out] 
 *
 * @return 0: setup ok , 2 : setup fail
 */
static u32 PcmHal_CfgUpd(void * pThis, void * pExtCfg)
{
    PPCM_HAL_CLS prThis = (PPCM_HAL_CLS)pThis;
    PPCM_EXTPARAMS_T prExtCfg = (PPCM_EXTPARAMS_T)pExtCfg;
        
    u32 u4RetVal = AUD_RET_OK;
    PPCM_EXTPARAMS_T prCurExtCfg = &(prThis->rCfg.rExtCfg);

    if (prCurExtCfg->eHwMode != prExtCfg->eHwMode)
    {
        PCMLOG_INFO((T("pcm hw mode config changed :(%d)  \r\n" ), (u32)prExtCfg->eHwMode));

        prCurExtCfg->eHwMode = prExtCfg->eHwMode;

        prThis->prPcmHw->SetHwMode(prThis->prPcmHw, prCurExtCfg->eHwMode);
    }

    if (prCurExtCfg->u4SampleRate != prExtCfg->u4SampleRate)
    {
        PCMLOG_INFO((T("pcm sample rate config changed :(%d)\r\n"), 
            (u32)prExtCfg->u4SampleRate));

        prCurExtCfg->u4SampleRate = prExtCfg->u4SampleRate;
        prThis->prPcmHw->UpdCfg(prThis->prPcmHw, &(prThis->rCfg));
    }
    
    return u4RetVal;
}

/**
 * Hal interfac : get pcm hw status
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return  hw status
 */
static u32 PcmHal_GetStatus(void * pThis)
{
    PPCM_HAL_CLS prThis = (PPCM_HAL_CLS)pThis;
    
    return (prThis->u4PcmState);
}

/**
 * Hal interfac : enable pcm hw
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return  hw status
 */
static u32 PcmHal_EnCtl(void * pThis, bool fgEn)
{
    PPCM_HAL_CLS prThis = (PPCM_HAL_CLS)pThis;
	#if CONFIG_DRV_AUD_AC83XX
	prThis->prPcmHw->Enable(prThis->prPcmHw, PCM_CTL_BIT_ON, fgEn);
	#else
	pr_debug("QK11 enable the PcmHal_EnCtl\n");
    PcmHw_SetIntClrBit(fgEn);
    prThis->prPcmHw->Enable(prThis->prPcmHw, PCM_CTL_BIT_ON, fgEn);
	#endif
    prThis->u4PcmState = (fgEn) ? AUD_STATE_STARTED : AUD_STATE_STOPPED;

    return (prThis->u4PcmState);
}

/**
 * Hal interfac : get rx hw status
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return  hw status
 */
static u32 PcmHal_GetRxStatus(void * pThis)
{
    PPCM_HAL_CLS prThis = (PPCM_HAL_CLS)pThis;
    return (prThis->u4RxState);
}

/**
 * Hal interfac : enable pcm rx hw
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return  hw status
 */
static u32 PcmHal_StartRx(void * pThis, u32 u4PcmCtrl)
{
    PPCM_HAL_CLS prThis = (PPCM_HAL_CLS)pThis;
    prThis->prPcmHw->Enable(prThis->prPcmHw, PCM_CTL_BIT_RX, TRUE);

    prThis->u4RxState = AUD_STATE_STARTED;

    return (prThis->u4RxState);
}

/**
 * Hal interfac : disable pcm rx hw
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return  hw status
 */
static u32 PcmHal_StopRx(void * pThis, u32 u4PcmCtrl)
{
    PPCM_HAL_CLS prThis = (PPCM_HAL_CLS)pThis;
    prThis->prPcmHw->Enable(prThis->prPcmHw, PCM_CTL_BIT_RX, FALSE);

    prThis->u4RxState = AUD_STATE_STOPPED;
    
    return (prThis->u4RxState);
}

/**
 * Hal interfac : get tx hw status
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return  hw status
 */
static u32 PcmHal_GetTxStatus(void * pThis)
{
    PPCM_HAL_CLS prThis = (PPCM_HAL_CLS)pThis;
    return (prThis->u4TxState);
}

/**
 * Hal interfac : enable tx hw
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return  hw status
 */
static u32 PcmHal_StartTx(void * pThis, u32 u4PcmCtrl)
{
    PPCM_HAL_CLS prThis = (PPCM_HAL_CLS)pThis;
    prThis->prPcmHw->Enable(prThis->prPcmHw, PCM_CTL_BIT_TX, TRUE);

    prThis->u4TxState = AUD_STATE_STARTED;

    return (prThis->u4TxState);
}

/**
 * Hal interfac : disable pcm rx hw
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return  hw status
 */
static u32 PcmHal_StopTx(void * pThis, u32 u4PcmCtrl)
{
    PPCM_HAL_CLS prThis = (PPCM_HAL_CLS)pThis;
    prThis->prPcmHw->Enable(prThis->prPcmHw, PCM_CTL_BIT_TX, FALSE);

    prThis->rTxIntCtl.u4Rp = 0;
    prThis->rTxIntCtl.u4Wp = 0;
    prThis->rTxIntCtl.u4OBankNum = 0;
    prThis->rTxIntCtl.u4IBankNum = 0;
    prThis->rTxIntCtl.u4UnderRunCnt = 0;

    prThis->u4TxState = AUD_STATE_STOPPED;
    
    return (prThis->u4TxState);
}

//==============================================//
    #define CodeSight_PcmHal_IF_BUF
//==============================================//

/**
 * Hal interfac : get pcm in buffer config
 *
 * @param [in]  prThis : hal class
 * @param [out] prBuf : buffer information
 *
 * @return  hw status
 */
static u32 PcmHal_GetRxBuf(void * pThis, AUD_DATA_BUF_T *prBuf)
{
    PPCM_HAL_CLS prThis = (PPCM_HAL_CLS)pThis;
    x_memcpy(prBuf, &(prThis->rCfg.rRxBuf), sizeof(AUD_DATA_BUF_T));
    
    return (AUD_RET_OK);
}

/**
 * Hal interfac : set pcm in buffer read pointer
 *
 * @param [in]  prThis : hal class, u4Rp : current read pointer
 * @param [out] 
 *
 * @return  0 : set ok, 1 : set fail
 */
static u32 PcmHal_SetRxRp(void * pThis, u32 u4Rp)
{
    u32 u4RetVal = AUD_RET_OK;
    
    return (u4RetVal);
}

/**
 * Hal interfac : get pcm out buffer config
 *
 * @param [in]  prThis : hal class
 * @param [out] prBuf : buffer information
 *
 * @return  hw status
 */
static u32 PcmHal_GetTxBuf(void * pThis, AUD_DATA_BUF_T *prBuf)
{
    PPCM_HAL_CLS prThis = (PPCM_HAL_CLS)pThis;
    x_memcpy(prBuf, &(prThis->rCfg.rTxBuf), sizeof(AUD_DATA_BUF_T));
    
    return (AUD_RET_OK);
}

/**
 * Hal interfac : Get pcm in buffer write pointer
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return  current write pointer
 */
static u32 PcmHal_GetRxWp(void * pThis)
{
    PPCM_HAL_CLS prThis = (PPCM_HAL_CLS)pThis;
    u32 u4Wp = prThis->prPcmHw->GetWp(prThis->prPcmHw);
    
    return (u4Wp);
}

/**
 * Hal interfac : Get pcm out buffer read pointer
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return  current read pointer
 */
static u32 PcmHal_GetTxRp(void * pThis)
{
    u32 u4Rp;
    PPCM_HAL_CLS prThis = (PPCM_HAL_CLS)pThis;
    PAUD_PCM_INT_CTL_T prIntCtl = &(prThis->rTxIntCtl);

    u4Rp = prIntCtl->u4Rp;
    
    return (u4Rp);
}

/**
 * Hal interfac : Set pcm out buffer write pointer
 *
 * @param [in]  prThis : hal class, u4Wp : current write pointer
 * @param [out] 
 *
 * @return  0 : set ok, 1 : set fail
 */
static u32 PcmHal_SetTxWp(void * pThis, u32 u4Wp)
{
    PPCM_HAL_CLS prThis = (PPCM_HAL_CLS)pThis;
    u32 u4RetVal = AUD_RET_OK;
    u32 u4NewDataSize;
        
    PAUD_PCM_INT_CTL_T prIntCtl = &(prThis->rTxIntCtl);
    
    u4NewDataSize = AudMisc_FifoDataSize_Get(u4Wp, prIntCtl->u4Wp, prThis->rCfg.rTxBuf.u4ChBufSz);

    if (0 != (u4Wp % prIntCtl->u4BankSz))
    {
        PCMLOG_ERR((T("pcm tx write pointer need be n * int_size !!!!  \r\n" )));
        u4RetVal = AUD_RET_FAIL;
    }

    prIntCtl->u4IBankNum += (u4NewDataSize / prIntCtl->u4BankSz);

    if ((prIntCtl->u4IBankNum - prIntCtl->u4OBankNum) >= prIntCtl->u4BankNum)
    {
        PCMLOG_ERR((T("pcm tx input buffer overflow !!!!  \r\n" )));
    }
    
    prIntCtl->u4Wp = u4Wp;
    
    return (u4RetVal);
}



//==============================================//
    #define CodeSight_PcmHal_Others
//==============================================//



//==============================================//
    #define CodeSight_PcmHal_Create
//==============================================//

/**
 * Hal interfac : Hal class delete
 *
 * @param [in]  prThis : hal class
 * @param [out] 
 *
 * @return  0 : set ok
 */
static u32 PcmHal_Delete(void * pThis)
{
    PPCM_HAL_CLS prThis = (PPCM_HAL_CLS)pThis;
    
    if (NULL != prThis->prPcmHw)
    {
        prThis->prPcmHw->Delete(prThis->prPcmHw);
    }
        
    if (0 != prThis->rCfg.rTxBuf.u4VirSAdr)
    {
        #ifndef __linux__
        AudOS_Memory_Free(&(prThis->rCfg.rTxBuf.u4VirSAdr));
        #endif
    }

    IoPinMux_SetPcm(PINMUX_PCM_DEFAULT);    
    AUD_CLASS_DELETE();

    AudOS_IRQ_Disable(VECTOR_AOUT_BT_RC); 

    prPcmObj = NULL;

    return (0);
}

/**
 * Hal interfac : Hal class create
 *
 * @param [in] 
 * @param [out] 
 *
 * @return  pointer to pcm hal pub cls 
 */
PPCM_HAL_CLS_PUB PcmHal_New(void)
{
    PPCM_HAL_CLS prThis = (PPCM_HAL_CLS)kzalloc(sizeof(PCM_HAL_CLS), GFP_KERNEL);
    if (prThis)
    {
        PPCM_HAL_CLS_PUB prPub = &(prThis->rPub);

        prPcmObj = prThis;
            
        prThis->u4PcmState = AUD_STATE_UNINIT;
        prThis->u4RxState= AUD_STATE_UNINIT;
        prThis->u4TxState= AUD_STATE_UNINIT;

        prThis->prPcmHw= PcmHw_New();
        
        if (NULL == prThis->prPcmHw)
        {
            PCMLOG_ERR((T("Pcm New PcmHw Error!!! \r\n")));
            
            kfree(prThis);
            prThis = NULL;
        }
        else
        {
            //pcm rx interface
            prPub->rRxHwIf.Setup = PcmHal_Setup;
            prPub->rRxHwIf.GetStatus = PcmHal_GetRxStatus;
            prPub->rRxHwIf.Start = PcmHal_StartRx;
            prPub->rRxHwIf.Stop = PcmHal_StopRx;
            prPub->rRxHwIf.CfgUpd = PcmHal_CfgUpd;
            prPub->rRxHwIf.GetBuf = PcmHal_GetRxBuf;
            prPub->rRxHwIf.GetPoint = PcmHal_GetRxWp;
            prPub->rRxHwIf.SetPoint = PcmHal_SetRxRp;

            //pcm tx interface
            prPub->rTxHwIf.Setup = PcmHal_Setup;
            prPub->rTxHwIf.GetStatus = PcmHal_GetTxStatus;
            prPub->rTxHwIf.Start = PcmHal_StartTx;
            prPub->rTxHwIf.Stop = PcmHal_StopTx;
            prPub->rTxHwIf.CfgUpd = PcmHal_CfgUpd;
            prPub->rTxHwIf.GetBuf = PcmHal_GetTxBuf;
            prPub->rTxHwIf.GetPoint = PcmHal_GetTxRp;
            prPub->rTxHwIf.SetPoint = PcmHal_SetTxWp;

            //pcm hw interface
            prPub->PcmStatus = PcmHal_GetStatus;
            prPub->PcmCtrl = PcmHal_EnCtl;
            prPub->Delete = PcmHal_Delete;
            prPub->PcmCfgUpd = PcmHal_CfgUpd;
        }
    }
    else
    {
        PCMLOG_ERR((T("Pcm New PcmHal Error!!! \r\n")));
    }

    return ((PPCM_HAL_CLS_PUB)prThis);
}

