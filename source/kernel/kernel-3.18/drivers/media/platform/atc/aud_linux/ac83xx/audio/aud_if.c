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


#include "x_lint.h"
#include <linux/types.h>
#include "x_assert.h"
#include "x_ver.h"

#include "aud_oal.h"
#include "aud_debug.h"
#include "aud_if.h"
#include "aud_drv.h"
#include "aud_config.h"
#include "DspDrvInc.h"
#include "DspFunc.h"
#include "aud_drv_config.h"
#include "drv_thread.h"
#include "aud_esm.h"

#if CONFIG_AUD_ADSP_ERR_RECOVER_EN
#include "DspErrProc.h"
#endif


/****************************************************************************
** Local definitions
****************************************************************************/
extern AUD_ESM_CONTEXT_T g_rAudEsmContext[];
extern AUD_DRV_AUD_INFO_T g_rAudDrvUpdatedAudInfo[];

/****************************************************************************
** Function prototypes
****************************************************************************/
extern AUD_DRV_AUD_INFO_T * g_prAudDrvUpdatedAudInfo[];

/****************************************************************************
** Local variable
****************************************************************************/
static struct semaphore g_hAudDrvSema[MAX_AUDDRV_NUM];
static struct semaphore g_hAudDrvCmdWaitSema[MAX_AUDDRV_NUM];
extern struct semaphore AudAdcAllocateSema;

static s8 *g_paAudCmd[7] =
{
    TEXT("AUD_CMD_PLAY"),
    TEXT("AUD_CMD_STOP"),
    TEXT("AUD_CMD_RESET"),
    TEXT("AUD_CMD_PAUSE"),
    TEXT("AUD_CMD_AVSYNC"),
    TEXT("AUD_CMD_LOADCODE"),
    TEXT("AUD_CMD_RESUME")
};


/****************************************************************************
** Global functions
****************************************************************************/
void AUD_RealPlayNotify(u8 u1DecId,  AUD_DRV_CMD_T eAudDecCmd)
{
    AUD_DRV_NFY_INFO_T rAudNfyInfo;

    VERIFY(((s8)u1DecId >= PRI_DEC) && (u1DecId < MAX_AUDDRV_NUM));
    LOG(LOG_FEATURE, TEXT("DecId(%d) Real Play Notify \n"), u1DecId);

    // Get notify function
    AudDrvGetNfy(u1DecId, &rAudNfyInfo);

    // Notify middleware
    if ((eAudDecCmd == AUD_CMD_PLAY))
    {
        if (rAudNfyInfo.pfAudDecNfy)
        {

            u32 u4Data1 = 0;
            u32 u4Data2 = 0;
            rAudNfyInfo.pfAudDecNfy((void *)rAudNfyInfo.pvTag,
                                    AUD_COND_AUD_REALPLAY, u4Data1, u4Data2);

        }
    }
}

void AUD_InbandCmdNotify(u8 u1DecId,  u32 u4IbcId)
{
    AUD_DRV_NFY_INFO_T rAudNfyInfo;

    VERIFY(((s8)u1DecId >= PRI_DEC) && (u1DecId < MAX_AUDDRV_NUM));
    LOG(LOG_FEATURE, TEXT("Receive inband command notify! u1DecId = %d, u4IbcId = 0x%X\n"),
        u1DecId, u4IbcId);

    VERIFY(u4IbcId!=0);

    // Get notify function
    AudDrvGetNfy(u1DecId, &rAudNfyInfo);

    // Notify middleware
    if (rAudNfyInfo.pfAudDecNfy)
    {

        u32 u4Data2 = 0;
        rAudNfyInfo.pfAudDecNfy((void *)rAudNfyInfo.pvTag,
                                AUD_COND_INBAND_CMD_DONE, u4IbcId, u4Data2);

    }
}

void AUD_HdcdTrkStmChg_Notify(u8 u1DecId, bool isHdcdTrk)
{
    AUD_DRV_NFY_INFO_T rAudNfyInfo;

    VERIFY(u1DecId == PRI_DEC);
    LOG(LOG_FEATURE, TEXT("Receive HdcdTrkStmChg notify! u1DecId = %d, isHdcdTrk = 0x%X\n"),
        u1DecId, isHdcdTrk);

    // Update decoding info
    if (isHdcdTrk)
    {
        g_rAudDrvUpdatedAudInfo[u1DecId].e_aud_fmt=AUD_DRV_FMT_HDCD;
    }
    else
    {
        g_rAudDrvUpdatedAudInfo[u1DecId].e_aud_fmt=AUD_DRV_FMT_CDDA;
    }

    // Get notify function
    AudDrvGetNfy(u1DecId, &rAudNfyInfo);

    // Notify middleware
    if (rAudNfyInfo.pfAudDecNfy)
    {
        u32 u4Data2 = 0;
        x_memcpy(g_prAudDrvUpdatedAudInfo[u1DecId],&g_rAudDrvUpdatedAudInfo[u1DecId],sizeof(AUD_DRV_AUD_INFO_T));

        rAudNfyInfo.pfAudDecNfy((void *)rAudNfyInfo.pvTag,
                                AUD_COND_AUD_INFO_CHG, (u32)&g_rAudDrvUpdatedAudInfo[u1DecId], u4Data2);

    }
}


static u8 AUD_Get_Input_Bit_Resolution(u8 u1DecId)
{
    if (AUD_DRV_FMT_PCM != g_rAudDrvUpdatedAudInfo[u1DecId].e_aud_fmt)
    {
        return 0;
    }

    return (DspCfgGetInputBitRate(u1DecId));
}

void AUD_Ch_Cfg_Notify(u8 u1DecId, AUD_DRV_AUD_TYPE_T eAudChCfg)
{
    AUD_DRV_NFY_INFO_T rAudNfyInfo;
    u32 u4SampleRate;
    u8 u1BitRes;

    if(u1DecId != PRI_DEC);
    {
        LOG(LOG_FAIL, TEXT("AUD_Ch_Cfg_Notify, unsupport u1DecId = %d\n"), u1DecId);
        return;
    }

    LOG(LOG_FEATURE, TEXT("Receive AUD_Ch_Cfg_Notify notify, u1DecId = %d\n"), u1DecId);

    // Update input channel config
    if (g_rAudDrvUpdatedAudInfo[u1DecId].e_aud_type> AUD_DRV_TYPE_STEREO)
    {
        if ((g_rAudDrvUpdatedAudInfo[u1DecId].e_aud_fmt == AUD_DRV_FMT_LOSSLESS_AC3)||
                (g_rAudDrvUpdatedAudInfo[u1DecId].e_aud_fmt == AUD_DRV_FMT_TRUE_HD))
        {
            g_rAudDrvUpdatedAudInfo[u1DecId].e_aud_type = AUD_DRV_TYPE_MULTI_CH;
        }
        else
        {
            g_rAudDrvUpdatedAudInfo[u1DecId].e_aud_type = eAudChCfg;
        }
    }
    else if ((AUD_DRV_FMT_PCM == g_rAudDrvUpdatedAudInfo[u1DecId].e_aud_fmt) &&
             (AUD_DRV_TYPE_DUAL_MONO == g_rAudDrvUpdatedAudInfo[u1DecId].e_aud_type))
    {
        // Yes, do NOTHING here for LPCM decoder always update STEREO for dual mono.
    }
    else
    {
        g_rAudDrvUpdatedAudInfo[u1DecId].e_aud_type = eAudChCfg;
    }

    // Update sampling rate
    u4SampleRate = DspCfgInputSampRateDecimal(u1DecId);
    g_rAudDrvUpdatedAudInfo[u1DecId].ui4_sample_rate = u4SampleRate;

    // Update bit resolution
    u1BitRes = AUD_Get_Input_Bit_Resolution(u1DecId);
    g_rAudDrvUpdatedAudInfo[u1DecId].ui1_bit_depth = u1BitRes;

    LOG(LOG_FEATURE, TEXT("DecId(%d),Stream Format(%d),Sampling Rate(%d), Bit Depth(%d), ChannelCount: %d, PID: %d.\n"),
        u1DecId,g_rAudDrvUpdatedAudInfo[u1DecId].e_aud_fmt,
        u4SampleRate, u1BitRes,g_rAudDrvUpdatedAudInfo[u1DecId].e_aud_type,
        g_rAudDrvUpdatedAudInfo[u1DecId].ui2_pid);

    // Get notify function
    AudDrvGetNfy(u1DecId, &rAudNfyInfo);

    // Notify middleware
    if (rAudNfyInfo.pfAudDecNfy)
    {
        u32 u4Data2 = 0;
        x_memcpy(g_prAudDrvUpdatedAudInfo[u1DecId],&g_rAudDrvUpdatedAudInfo[u1DecId],sizeof(AUD_DRV_AUD_INFO_T));

        rAudNfyInfo.pfAudDecNfy((void *)rAudNfyInfo.pvTag,
                                AUD_COND_AUD_INFO_CHG, (u32)&g_rAudDrvUpdatedAudInfo[u1DecId], u4Data2);
    }
}

void AUD_CommandDoneNotify(u8 u1DecId, AUD_DRV_CMD_T eAudDecCmd)
{
    if (eAudDecCmd == AUD_CMD_PLAY|| eAudDecCmd == AUD_CMD_STOP||
        eAudDecCmd == AUD_CMD_PAUSE|| eAudDecCmd == AUD_CMD_RESUME||
        eAudDecCmd == AUD_CMD_LOADCODE)
    {
        LOG(LOG_CTRLF, TEXT("Decoder[%d] AUD_CMD = %s is done.\n"),
            u1DecId, g_paAudCmd[eAudDecCmd]);
    	up(&g_hAudDrvCmdWaitSema[u1DecId]);
    }
    else
    {
        LOG(LOG_CTRLF, TEXT("Enter NoEvent\n"));
    }

}

void AUD_WaitCommandDone(u8 u1DecId, AUD_DRV_CMD_T eAudDecCmd)
{
    LOG(LOG_CTRLF, TEXT("Decoder[%d] start waiting %s CMD.\n"),
        u1DecId, g_paAudCmd[eAudDecCmd]);

    down(&g_hAudDrvCmdWaitSema[u1DecId]);

    LOG(LOG_CTRLF, TEXT("Decoder[%d] get %s cmd OK.\n"),
        u1DecId, g_paAudCmd[eAudDecCmd]);
}

s32 AUD_Init(void)
{
    static bool _fgInited = FALSE;
    u32 u4Index = 0;
    if (!_fgInited)
    {
        for (u4Index = 0; u4Index < MAX_AUDDRV_NUM; u4Index++)
        {
            sema_init(&g_hAudDrvSema[u4Index], 1);
            sema_init(&g_hAudDrvCmdWaitSema[u4Index], 0);
        }
        sema_init(&AudAdcAllocateSema, 1);
        _fgInited = TRUE;
        AudCfg_HWInit();
        AudDrvInit();
    }

    return AUD_OK;
}

void AUD_UnInit(void)
{
    u32 u4Index = 0;
    for (u4Index = 0; u4Index < MAX_AUDDRV_NUM; u4Index++)
    {
        //VERIFY(x_sema_delete(g_hAudDrvSema[u4Index]) == OSR_OK);
        //VERIFY(x_sema_delete(g_hAudDrvCmdWaitSema[u4Index]) == OSR_OK);
    }
}

s32 AUD_SetDecType(u8 u1DecId,  AUD_DRV_STREAM_FROM_T eStreamFrom, const AUD_DRV_FMT_INFO_T * prDecType)
{
    s32 i4Ret  = AUD_FAIL;

    VERIFY(prDecType != NULL);
    VERIFY(u1DecId <= TER_DEC);

    down(&g_hAudDrvSema[u1DecId]);
    i4Ret= AudDrvSetDecType(u1DecId, eStreamFrom, prDecType);
    up(&g_hAudDrvSema[u1DecId]);

    return (i4Ret);
}


s32 AUD_DSPCmdPlayAsyn(u8 u1DecId)
{
    s32 i4RetVal = AUD_OK;

    LOG(LOG_CTRLF, TEXT("DRV(%d) receives PLAY command \n"), u1DecId);
    VERIFY(((s8)u1DecId >= PRI_DEC) && (u1DecId < MAX_AUDDRV_NUM));
    #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
    if (DSP_PROC_RUN != u4AdspErrProcStateGet() && (PRI_DEC == u1DecId))
    {
        u4AdspErrProcStateSet(DSP_PROC_RUN);
    }
    #endif
    down(&g_hAudDrvSema[u1DecId]);
    #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
    vErrTimerAdd();
    #endif
    VERIFY(AudDrvSetCmd(u1DecId, AUD_CMD_PLAY));
    AUD_WaitCommandDone(u1DecId, AUD_CMD_PLAY);
    #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
    vErrTimerDelete();
    #endif
    up(&g_hAudDrvSema[u1DecId]);

    #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
    if (u4AdspErrProcNotifyFlagGet(u1DecId))
    {
        u4AdspErrProcNotifyFlagSet(u1DecId, 0);
        i4RetVal = AUD_DSPERROR;
        LOG(LOG_FAIL, TEXT("send PLAY cmd not ack, Dsp ErrRecover happened.\n"));
    }
    #else
    i4RetVal = AUD_OK;
    #endif

    LOG(LOG_CTRLF, TEXT("DRV(%d) receives PLAY command done \n"), u1DecId);

    return i4RetVal;
}

s32 AUD_DSPCmdPauseAsyn(u8 u1DecId)
{
    LOG(LOG_CTRLF, TEXT(" DRV(%d) receives PAUSE command \n"), u1DecId);
    VERIFY(((s8)u1DecId >= PRI_DEC) && (u1DecId < MAX_AUDDRV_NUM));
    down(&g_hAudDrvSema[u1DecId]);

    #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
    if (u4AdspErrProcNotifyFlagGet(u1DecId))
    {
        u4AdspErrProcNotifyFlagSet(u1DecId, 0);
        up(&g_hAudDrvSema[u1DecId]);

        return AUD_DSPERROR;  // for EOS return err_recover
    }
    #endif

    #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
    vErrTimerAdd();
    #endif
    VERIFY(AudDrvSetCmd(u1DecId, AUD_CMD_PAUSE));
    AUD_WaitCommandDone(u1DecId, AUD_CMD_PAUSE);
    #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
    vErrTimerDelete();
    #endif

    #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
    if (u4AdspErrProcNotifyFlagGet(u1DecId))
    {
        u4AdspErrProcNotifyFlagSet(u1DecId, 0);
        up(&g_hAudDrvSema[u1DecId]);
        LOG(LOG_FAIL, TEXT("send PAUSE cmd not ack, Dsp ErrRecover happened.\n"));
        return AUD_DSPERROR; // for PAUSE hangup return err_recover
    }
    #endif

    up(&g_hAudDrvSema[u1DecId]);
    return AUD_OK;
}


s32 AUD_DSPCmdResumeAsyn(u8 u1DecId)
{
    LOG(LOG_CTRLF, TEXT(" DRV(%d) receives RESUME command \n"), u1DecId);
    VERIFY(((s8)u1DecId >= PRI_DEC) && (u1DecId < MAX_AUDDRV_NUM));
    down(&g_hAudDrvSema[u1DecId]);
    #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
    vErrTimerAdd();
    #endif
    VERIFY(AudDrvSetCmd(u1DecId, AUD_CMD_RESUME));
    AUD_WaitCommandDone(u1DecId, AUD_CMD_RESUME);
    #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
    vErrTimerDelete();
    #endif

    #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
    if (u4AdspErrProcNotifyFlagGet(u1DecId))
    {
        u4AdspErrProcNotifyFlagSet(u1DecId, 0);
        up(&g_hAudDrvSema[u1DecId]);
        LOG(LOG_FAIL, TEXT("send RESUME cmd not ack, Dsp ErrRecover happened.\n"));
        return AUD_DSPERROR; // for RESUME hangup return err_recover
    }
    #endif

    up(&g_hAudDrvSema[u1DecId]);
    return AUD_OK;
}


s32 AUD_DSPCmdStopAsyn(u8 u1DecId)
{
    s32 i4RetVal = AUD_OK;

    LOG(LOG_CTRLF, TEXT(" DRV(%d) receives STOP command \n"), u1DecId);
    VERIFY(((s8)u1DecId >= PRI_DEC) && (u1DecId < MAX_AUDDRV_NUM));
    down(&g_hAudDrvSema[u1DecId]);
    #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
    vErrTimerAdd();
    #endif
    VERIFY(AudDrvSetCmd(u1DecId, AUD_CMD_STOP));
    AUD_WaitCommandDone(u1DecId, AUD_CMD_STOP);
    #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
    vErrTimerDelete();
    #endif
    up(&g_hAudDrvSema[u1DecId]);

    #if CONFIG_AUD_ADSP_ERR_RECOVER_EN
    if (u4AdspErrProcNotifyFlagGet(u1DecId))
    {
        u4AdspErrProcNotifyFlagSet(u1DecId, 0);
        i4RetVal = AUD_DSPERROR;
        LOG(LOG_FAIL, TEXT("send STOP cmd not ack, Dsp ErrRecover happened.\n"));
    }
    #else
    i4RetVal = AUD_OK;
    #endif

    return i4RetVal;
}


s32 AUD_DSPCmdResetAsyn(u8 u1DecId)
{
    LOG(LOG_CTRLF, TEXT(" DRV(%d) receives RESET command \n"), u1DecId);
    VERIFY(u1DecId == PRI_DEC);
    down(&g_hAudDrvSema[u1DecId]);
    VERIFY(AudDrvSetCmd(u1DecId, AUD_CMD_STOP));
    up(&g_hAudDrvSema[u1DecId]);

    return AUD_OK;
}

s32 AUD_GetAudFifo(uintptr_t * pu4Fifo1Start, uintptr_t * pu4Fifo1SEnd,uintptr_t * pu4Fifo2Start,uintptr_t * pu4Fifo2End)
{
    if ((pu4Fifo1Start != NULL) &&
            (pu4Fifo1SEnd != NULL) &&
            (pu4Fifo2Start != NULL) &&
            (pu4Fifo2End != NULL))
    {
        *pu4Fifo1Start = u4DspGetBufStartAddr(PRI_DEC, DSP_AFIFO);
        *pu4Fifo1SEnd = u4DspGetBufEndAddr(PRI_DEC, DSP_AFIFO);
    }
    else
    {
        return AUD_FAIL;
    }

    LOG(5, TEXT("Audio Fifo(%x,%x)\n"),u4DspGetBufStartAddr(PRI_DEC, DSP_AFIFO),u4DspGetBufStartAddr(PRI_DEC, DSP_AFIFO));
    return AUD_OK;
}


s32 AudioModuleInit(void)
{
    s32 i4Ret = AUD_OK;

    AudShowVerInfo();
    AUD_Init();
    i4Ret = i4AudEsm_Init();

    return (i4Ret);
}

s32 AudioModuleUnInit(u32 u4Case)
{
    s32 i4_return = AUD_OK;

    vDspPowerOff();
    //vAudIf_TimerDisable();
    //vAudIf_TimerDelete();

    AUD_UnInit();

    vADSPTaskExit();
    AudDrvThreadExit();
    vAudEsmThreadExit();

    LOG(LOG_CTRLF, TEXT("[AUD_MW] audio uninit! \n"));
    return i4_return;
}

void AudShowVerInfo(void)
{
#if 0
    s8 tAudVer[256];
    wsprintf(tAudVer, TEXT("[AUD]Version: V%s.%s_%s_%s.%s.%s\n"),
            AUD_VER_MAIN,
            AUD_VER_MINOR,
            AUD_VER_CHANGELIST,
            AUD_VER_MONTH,
            AUD_VER_DAY,
            AUD_VER_YEAR);
    RETAILMSG(1,(tAudVer));
#else
    MOD_VERSION_INFO(AUD_MOD_NAME, AUD_VER_MAIN, AUD_VER_MINOR, AUD_VER_REV);
#endif
}


