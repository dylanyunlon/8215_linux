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
 * @file aud_pcm_hw.c source file
 * 
 * aud io pcm module hardware driver
 * 
 * @author qiuhua.yin@autochips.com
 * 
 */

#include "aud_pcm_hal.h"
#include "aud_reg_env.h"
#include "aud_reg_rgbk2.h"
#include "aud_io_clock_if.h"


typedef struct 
{
    PCM_HW_CLS_PUB rPub;

}PCM_HW_CLS, *PPCM_HW_CLS;


//=========================================================//
    #define CodeSight_PcmHw_Static_Func
//=========================================================//

/**
 * set pcm hw mode
 *
 * @param [in]  prThis : hw class;  eHwMode : loop or normal mode
 * @param [out] 
 *
 * @return
 */
static void PcmHw_SetHwMode(void * pThis, AUD_PCM_HW_MODE eHwMode)
{
    AUDREG_BITS_W(REGENV_RGBK2_CFG5, BIT_STR_PCM_LOOP_MODE,
                  BIT_NUM_PCM_LOOP_MODE, eHwMode);
}

/**
 * set pcm sync mode
 *
 * @param [in]  prThis : hw class;  eSyncMode : short or long mode
 * @param [out] 
 *
 * @return
 */
static void PcmHw_SetSyncMode(PPCM_HW_CLS prThis, AUD_PCM_SYNC_MODE eSyncMode)
{
    AUDREG_BITS_W(REGENV_PCM_CTRL,
                  BIT_STR_SYNC_MODE_SEL,
                  BIT_NUM_SYNC_MODE_SEL,
                  eSyncMode);
}

/**
 * set pcm sync cycle
 *
 * @param [in]  prThis : hw class;  eSyncCycle : cycle 32/64
 * @param [out] 
 *
 * @return
 */
static void PcmHw_SetSyncCycle(PPCM_HW_CLS prThis, AUD_PCM_SYNC_CYCLE eSyncCycle)
{
    AUDREG_BITS_W(REGENV_PCM_CTRL,
                  BIT_STR_SYNC_CYCLE,
                  BIT_NUM_SYNC_CYCLE,
                  eSyncCycle);
}

/**
 * set pcm data order
 *
 * @param [in]  prThis : hw class;  eDataOrder : MSB/LSB
 * @param [out] 
 *
 * @return
 */
static void PcmHw_SetDataOrder(PPCM_HW_CLS prThis, AUD_PCM_DATA_ORDER eDataOrder)
{
    AUDREG_BITS_W(REGENV_PCM_CTRL,
                  BIT_STR_BIT_DATA_ORDER,
                  BIT_NUM_BIT_DATA_ORDER,
                  eDataOrder);
}

/**
 * set pcm sync length
 *
 * @param [in]  prThis : hw class;  eSyncLength : sync length
 * @param [out] 
 *
 * @return
 */
static void PcmHw_SetSyncLength(PPCM_HW_CLS prThis, AUD_PCM_SYNC_LENGTH eSyncLength)
{
    AUDREG_BITS_W(REGENV_PCM_CTRL,
                  BIT_STR_SYNC_LENGTH,
                  BIT_NUM_SYNC_LENGTH,
                  eSyncLength);
}

/**
 * set pcm bit number
 *
 * @param [in]  prThis : hw class;  eBitNum : bit number
 * @param [out] 
 *
 * @return
 */
static void PcmHw_SetBitNum(PPCM_HW_CLS prThis, AUD_PCM_BIT_NUM eBitNum)
{
    AUDREG_BITS_W(REGENV_PCM_CTRL,
                  BIT_STR_BIT_NUM_SEL,
                  BIT_NUM_BIT_NUM_SEL,
                  eBitNum);
}

/**
 * set pcm bit mode
 *
 * @param [in]  prThis : hw class;  eBitNum : bit mode
 * @param [out] 
 *
 * @return
 */
static void PcmHw_SetBitMode(PPCM_HW_CLS prThis, AUD_PCM_BIT_MODE eBitMode)
{
    AUDREG_BITS_W(REGENV_PCM_CTRL,
                  BIT_STR_PCM_16BIT,
                  BIT_NUM_PCM_16BIT,
                  eBitMode);
}

/**
 * set pcm mode
 *
 * @param [in]  prThis : hw class;  eMode : pcm mode
 * @param [out] 
 *
 * @return
 */
static void PcmHw_SetMode(PPCM_HW_CLS prThis, AUD_PCM_MODE eMode)
{
    AUDREG_BITS_W(REGENV_PCM_CTRL,
                  BIT_STR_MODE_SEL,
                  BIT_NUM_MODE_SEL,
                  eMode);
}

/**
 * set pcm tx start address
 *
 * @param [in]  prThis : hw class;  u4TxSadr : tx sadr
 * @param [out] 
 *
 * @return
 */
static void PcmHw_SetTxSadr(PPCM_HW_CLS prThis, u32 u4TxSadr)
{
    AUDREG_BITS_W(REGENV_PCMTX_DRAM_SADR,
                  BIT_STR_PCMTX_DRAM_SADR,
                  BIT_NUM_PCMTX_DRAM_SADR,
                  u4TxSadr);
}

/**
 * set pcm tx end address
 *
 * @param [in]  prThis : hw class;  u4TxEadr : tx eadr
 * @param [out] 
 *
 * @return
 */
static void PcmHw_SetTxEadr(PPCM_HW_CLS prThis, u32 u4TxEadr)
{
    AUDREG_BITS_W(REGENV_PCMTX_DRAM_EADR,
                  BIT_STR_PCMTX_DRAM_EADR,
                  BIT_NUM_PCMTX_DRAM_EADR,
                  u4TxEadr);
}

/**
 * set pcm tx blk address
 *
 * @param [in]  prThis : hw class;  u4TxBlkAdr : tx blk adr
 * @param [out] 
 *
 * @return
 */
static void PcmHw_SetTxBlkAdr(PPCM_HW_CLS prThis, u32 u4TxBlkAdr)
{
    AUDREG_BITS_W(REGENV_BT_PCM_BLK_CFG,
                  BIT_STR_PCM_TX_DRAM_BLK,
                  BIT_NUM_PCM_TX_DRAM_BLK,
                  u4TxBlkAdr);
}

/**
 * set pcm tx next start address
 *
 * @param [in]  prThis : hw class;  u4TxNSAdr : tx next start address
 * @param [out] 
 *
 * @return
 */
static void PcmHw_SetTxNSAdr(PPCM_HW_CLS prThis, u32 u4TxNSAdr)
{
    AUDREG_BITS_W(REGENV_PCMTX_DRAM_NSADR,
                  BIT_STR_PCM_TX_DRAM_NSADR,
                  BIT_NUM_PCM_TX_DRAM_NSADR,
                  u4TxNSAdr);
}

/**
 * set pcm tx s32 size
 *
 * @param [in]  prThis : hw class;  u4TxIntSz : tx interrupt size
 * @param [out] 
 *
 * @return
 */
static void PcmHw_SetTxIntSize(PPCM_HW_CLS prThis, u32 u4TxIntSz)
{
    AUDREG_BITS_W(REGENV_PCMTX_INTR,
                  BIT_STR_TX_SAMPLE_NUM,
                  BIT_NUM_TX_SAMPLE_NUM,
                  u4TxIntSz);
}

/**
 * set pcm tx s32 burst time
 *
 * @param [in]  prThis : hw class;  u4TxIntBurstTime : tx interrupt burst time
 * @param [out] 
 *
 * @return
 */
static void PcmHw_SetTxIntBurstTime(PPCM_HW_CLS prThis, u32 u4TxIntBurstTime)
{
    AUDREG_BITS_W(REGENV_PCMTX_INTR,
                  BIT_STR_TX_INTR_RENUM,
                  BIT_NUM_TX_INTR_RENUM,
                  u4TxIntBurstTime);
}

/**
 * set pcm tx next start address
 *
 * @param [in]  prThis : hw class;  u4TxNsadr : next start address
 * @param [out] 
 *
 * @return
 */
static void PcmHw_SetTxNextSAdr(void * pThis, u32 u4TxNsadr)
{
    AUDREG_BITS_W(REGENV_PCMTX_DRAM_NSADR,
                  BIT_STR_PCM_TX_DRAM_NSADR,
                  BIT_NUM_PCM_TX_DRAM_NSADR,
                  u4TxNsadr);
}

/**
 * set pcm rx start address
 *
 * @param [in]  prThis : hw class;  u4RxSadr : rx start adr
 * @param [out] 
 *
 * @return
 */
static void PcmHw_SetRxSadr(PPCM_HW_CLS prThis, u32 u4RxSadr)
{
    AUDREG_BITS_W(REGENV_PCMRX_DRAM_SADR,
                  BIT_STR_PCMRX_DRAM_SADR,
                  BIT_NUM_PCMRX_DRAM_SADR,
                  u4RxSadr);
}

/**
 * set pcm rx end address
 *
 * @param [in]  prThis : hw class;  u4RxEadr : rx end adr
 * @param [out] 
 *
 * @return
 */
static void PcmHw_SetRxEadr(PPCM_HW_CLS prThis, u32 u4RxEadr)
{
    AUDREG_BITS_W(REGENV_PCMRX_DRAM_EADR,
                  BIT_STR_PCMRX_DRAM_EADR,
                  BIT_NUM_PCMRX_DRAM_EADR,
                  u4RxEadr);
}

/**
 * set pcm rx blk address
 *
 * @param [in]  prThis : hw class;  u4RxBlkAdr : rx blk adr
 * @param [out] 
 *
 * @return
 */
static void PcmHw_SetRxBlkAdr(PPCM_HW_CLS prThis, u32 u4RxBlkAdr)
{
    AUDREG_BITS_W(REGENV_BT_PCM_BLK_CFG,
                  BIT_STR_PCM_RX_DRAM_BLK,
                  BIT_NUM_PCM_RX_DRAM_BLK,
                  u4RxBlkAdr);
}

/**
 * set pcm bank address
 *
 * @param [in]  prThis : hw class;  u4PcmbankAdr : pcm bank adr
 * @param [out] 
 *
 * @return
 */
static void PcmHw_SetBankAdr(PPCM_HW_CLS prThis, u32 u4PcmbankAdr)
{
    AUDREG_BITS_W(REGENV_RGBK2_CFG4,
                  BIT_STR_PCM_DRAM_BANK,
                  BIT_NUM_PCM_DRAM_BANK,
                  u4PcmbankAdr);
}


//=========================================================//
    #define CodeSight_PcmHw_Pubilic_Func
//=========================================================//


/**
 * public interface : pcm hw configure
 *
 * @param [in]  prThis : hw class; prCfg : pcm hw mode setting
 * @param [out] 
 *
 * @return
 */
static void PcmHw_InitCfg(void * pThis, void * pCfg)
{
    PPCM_HW_CLS prThis = (PPCM_HW_CLS)pThis;
    PAUD_PCM_CFG_T prCfg = (PAUD_PCM_CFG_T)pCfg;
    
    u32 u4BufSadr, u4TxSadr, u4TxEadr, u4RxSadr, u4RxEadr;
    u32 u4TxBlkAdr, u4TxBankAdr, u4RxBlkAdr, u4RxBankAdr;
    AUD_PCM_SYNC_LENGTH eSyncLength;
    
    PPCM_EXTPARAMS_T prExtCfg = &(prCfg->rExtCfg);

    PcmHw_SetHwMode(prThis, prExtCfg->eHwMode);
    
    PcmHw_SetSyncMode(prThis, prExtCfg->eSyncMode);
    PcmHw_SetSyncCycle(prThis, prExtCfg->eSyncCycle);
    IoClk_SetPcmMclk(prExtCfg->eSyncCycle, prExtCfg->u4SampleRate);

    PcmHw_SetDataOrder(prThis, prCfg->eDataOrder);

    eSyncLength = (PCM_LONG_MODE == prExtCfg->eSyncMode) ? PCM_CLK_LENGTH3 : PCM_CLK_LENGTH1;
    PcmHw_SetSyncLength(prThis, eSyncLength);

    PcmHw_SetBitNum(prThis, PCM_BITS_16);
    PcmHw_SetBitMode(prThis, PCM_LINEAR_16BIT);
    PcmHw_SetMode(prThis, PCM_MASTER);

    PCMLOG_INFO(T("PCM CONFIG REG val is :(0x%x) \r\n"), (u32)(AUDREG_READ(REGENV_PCM_CTRL)));


    u4BufSadr = prCfg->rTxBuf.u4PhySddr;
    u4TxBankAdr = (u4BufSadr & 0x7ff00000) >> 20;
    u4TxBlkAdr = (u4BufSadr & 0x0fff00) >> 8;
    u4TxSadr = (u4BufSadr & 0xf0) >> 4;
    u4TxEadr = u4TxSadr +  (prCfg->rTxBuf.u4ChBufSz >> 4) - 1;

    u4BufSadr = prCfg->rRxBuf.u4PhySddr;
    u4RxBankAdr = (u4BufSadr & 0x7ff00000) >> 20;
    u4RxBlkAdr = (u4BufSadr & 0x0fff00) >> 8;
    u4RxSadr = (u4BufSadr & 0xf0) >> 4;
    u4RxEadr = u4RxSadr +  (prCfg->rRxBuf.u4ChBufSz >> 4) - 1;     

    if (u4TxBankAdr != u4RxBankAdr)
    {
        PCMLOG_ERR(T("PCM BUF ERROR:  TX and RX are not in the same bank \r\n"));
    }

    PcmHw_SetTxSadr(prThis, u4TxSadr);
    PcmHw_SetTxEadr(prThis, u4TxEadr);
    PcmHw_SetTxBlkAdr(prThis, u4TxBlkAdr);

    PcmHw_SetRxSadr(prThis, u4RxSadr);
    PcmHw_SetRxEadr(prThis, u4RxEadr);
    PcmHw_SetRxBlkAdr(prThis, u4RxBlkAdr);

    PcmHw_SetBankAdr(prThis, u4TxBankAdr);

    PcmHw_SetTxIntSize(prThis, prExtCfg->rIntCfg.u4IntSz);
    PcmHw_SetTxIntBurstTime(prThis, prExtCfg->rIntCfg.u4IntBurstTime);
    PcmHw_SetTxNSAdr(prThis, u4TxSadr);
}

static void PcmHw_UpdCfg(void * pThis, void * pCfg)
{
    PAUD_PCM_CFG_T prCfg = (PAUD_PCM_CFG_T)pCfg;

    PPCM_EXTPARAMS_T prExtCfg = &(prCfg->rExtCfg);

    IoClk_SetPcmMclk(prExtCfg->eSyncCycle, prExtCfg->u4SampleRate);
}

/**
 * public interface : enable/disable pcm hw
 *
 * @param [in]  prThis : hw class; fgEnable : enable or disable
 * @param [out] 
 *
 * @return
 */
static void PcmHw_Enable(void * pThis, u32 u4PcmCtrl, bool fgEnable)
{   
    PCMLOG_INFO(T("PcmHw_Enable : Control Bit (0x%x) \r\n"), (u32)u4PcmCtrl);

    if (fgEnable)
    {
        AUDREG_WRITE(REGENV_PCM_CTRL, (AUDREG_READ(REGENV_PCM_CTRL) | u4PcmCtrl));
    }
    else
    {
        AUDREG_WRITE(REGENV_PCM_CTRL, (AUDREG_READ(REGENV_PCM_CTRL) & (~ u4PcmCtrl)));
    }
}

/**
 * pcm hw current write pointer get
 *
 * @param [in]  prThis : hw class
 * @param [out] 
 *
 * @return : current write pointer
 */
static u32 PcmHw_GetWp(void * pThis)
{
    u32 u4PcmWp, u4PcmSadr;

    u4PcmWp = AUDREG_READ(REGENV_PCMRX_WRADR);
    u4PcmSadr = AUDREG_BITS_R(REGENV_PCMRX_DRAM_SADR, BIT_STR_PCMRX_DRAM_SADR, BIT_NUM_PCMRX_DRAM_SADR);

    u4PcmWp = (u4PcmWp - u4PcmSadr) << 4;  //byte offset
    
    return u4PcmWp;
}


//===========================================//
    #define CodeSight_PcmHw_Create
//===========================================//

/**
 * delect a pcm in hw object
 *
 * @param [in]  prThis : pointer to the pcm hw object
 * @param [out] 
 *
 * @return      0: OK; others: NG
 */
static u32 PcmHw_Delete(void * pThis)
{
    PPCM_HW_CLS prThis = (PPCM_HW_CLS)pThis;
    
    AUD_CLASS_DELETE();
    
    return (0);
}

/**
 * creat a new pcm hw object
 *
 * @param [in]
 * @param [out] 
 *
 * @return  pointer to new object
 */
PPCM_HW_CLS_PUB PcmHw_New(void)
{
    PPCM_HW_CLS prThis = AUD_CLASS_NEW(PCM_HW_CLS);

    if (prThis)
    {
        prThis->rPub.InitCfg = PcmHw_InitCfg;
        prThis->rPub.UpdCfg = PcmHw_UpdCfg;
        prThis->rPub.Enable = PcmHw_Enable;
        prThis->rPub.GetWp = PcmHw_GetWp;
        prThis->rPub.Delete = PcmHw_Delete;
        prThis->rPub.SetNsadr = PcmHw_SetTxNextSAdr;
        prThis->rPub.SetHwMode = PcmHw_SetHwMode;
    }

    return ((PPCM_HW_CLS_PUB)prThis);
}
