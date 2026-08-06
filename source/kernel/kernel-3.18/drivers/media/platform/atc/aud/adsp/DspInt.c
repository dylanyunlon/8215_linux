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
#define _DSP_INT_C

/*-----------------------------------------------------------------------------
Include header files
-----------------------------------------------------------------------------*/
#include "aud_oal.h"
#include <media/atc/drv_aud.h>
#include "AsvDspCtrl.h"
#include "AsvDef.h"
#include "AsvState.h"
#include "aud_drv_config.h"
#include "aud_drv.h"
#include "aud_se.h"
#include "drv_dsp_cfg.h"
#include "aud_config.h"
#include "AsvAudDrv.h"
#include "DspFunc.h"
#include "DspShm.h"
#include "DspUop.h"
#include "DspD2RCInt.h"
#include "DspVar.h"
#include "aud_if.h"

/*-----------------------------------------------------------------------------
Function declarations
-----------------------------------------------------------------------------*/
bool _fgCSIIDbgPlaying = FALSE;

volatile bool _fgDspAWakeUpFlag = FALSE;
volatile bool _fgDspBWakeUpFlag = FALSE;
extern void* _hDspAStatusHandle;
extern void* _hDspBStatusHandle;

extern AUD_SE_NOTIFY_SET_PP_TAB_T g_aNotifyInfo;
extern AUD_DRV_AUD_INFO_T g_rAudDrvUpdatedAudInfo[];

extern u32  g_u4AudPriICBId;
extern bool g_fgAudioEosDone;
extern bool g_fgFirstAoutArrive;
u32 g_u4DspSetSampleRateAck[TER_DEC+1] = {0};

extern AUD_DRV_STATE_T g_rAudDrvState[MAX_AUDDRV_NUM];
/***************************************************************************
   Function : vSetPostProcTable()
Description : set post processing table
  Parameter : u4FreqIdx: sample rate, fgFirstDecoder: dec
  Return    :
***************************************************************************/
void vSetPostProcTable (u32 u4FreqIdx, bool fgFirstDecoder)
{
    AUD_SOURCE_CFG_T* prSrcCfg = DspGetSrcParam();

    LOG(LOG_FEATURE, TEXT("[vSetPostProcTable] ChNum = 0x%x,eSmpRate = 0x%x.\r\n"),
         prSrcCfg->u1Aud_Input_Chan_Cnt, prSrcCfg->u1Aud_Sampling_Rate);

    if (fgFirstDecoder)
    {                           // first decoder
        //Delay Factor
        DspSetChDelayFactor(u4FreqIdx);
#if CONFIG_AUD_SE_V2_EN
        // (aud_se_v2)
        {
            // Notify Information Setting
            g_aNotifyInfo.prDspSrcParam = prSrcCfg;
            g_aNotifyInfo.prDspOutputParam = DspGetOutParam();
            g_aNotifyInfo.u4FreqIdx = u4FreqIdx;
            vAudSeProcessNotify(AUD_SE_NOTIFY_SET_PP_TAB, &g_aNotifyInfo);
        }
#endif
    }
}

/***************************************************************************
   Function : vDspSetTable
Description : set table to dsp
  Parameter : u4FreqIdx: sample rate
  Return    :
***************************************************************************/
void vDspSetTable(u32 u4FreqIdx)
{
    //enable post reinit function for all features
    vResetPostReinitFlag();
    vSetPostProcTable(u4FreqIdx, TRUE);

    DspSetupDownMix(u4FreqIdx,TRUE);
    DspSetBassManageTable(u4FreqIdx);

    //Check if DRC Auto and Set DRC Mode
    if (u2ReadDspShmWORD(W_PROCMOD) & 0x8)
    {
        vDspCmd(UOP_DSP_PROCESSING_MODE);
    }
}


/***************************************************************************
   Function : vDspGetInputChannelCountForMW
Description : Primary/secondary audio channel count call back
  Parameter : ucDecId
  Return    :
***************************************************************************/
void vDspGetInputChannelCountForMW (u8 u1DecId)
{
    u32 u4DspInputChCfg = 0;
    bool fgOutputRawBitstream = FALSE;
    AUD_DRV_AUD_TYPE_T eAudChCfg = AUD_DRV_TYPE_UNKNOWN;
    AUD_OUTPUT_SETTING_CFG_T* prOutParam = DspGetOutParam();
    AUD_OUTPUT_SETTING_CFG_T* prOutHdmiParam = DspGetOutHdmiParam();

    if (u1DecId != PRI_DEC)
    {
        return;
    }
    DspGetInputChCfg(&u4DspInputChCfg);

    if ((prOutHdmiParam->eIec_Cfg == AUD_IEC_CFG_RAW) ||
        (prOutHdmiParam->eIec_Cfg == AUD_IEC_CFG_RAW_HD) ||
        (prOutParam->eIec_Cfg == AUD_IEC_CFG_RAW) ||
        (prOutParam->eIec_Cfg == AUD_IEC_CFG_RAW_HD))
    {
        fgOutputRawBitstream = TRUE;
        LOG(LOG_CTRLF, TEXT("Raw Data Output\n"));
    }

    switch (u4DspInputChCfg & 0x03F)
    {
    case DSP_CH_CFG_DUAL_MONO:
        eAudChCfg=AUD_DRV_TYPE_DUAL_MONO;
        break;
    case DSP_CH_CFG_MONO:
        eAudChCfg=AUD_DRV_TYPE_MONO;
        break;
    case (DSP_CH_CFG_MONO+DSP_CH_CFG_SUB):
        eAudChCfg=AUD_DRV_TYPE_1_0_1;
        break;
    case DSP_CH_CFG_STEREO:
        eAudChCfg=AUD_DRV_TYPE_STEREO;
        break;
    case (DSP_CH_CFG_STEREO+DSP_CH_CFG_SUB):
        eAudChCfg=AUD_DRV_TYPE_SURROUND_2CH; // 2/1
        break;
    case DSP_CH_CFG_LRS:
        eAudChCfg=AUD_DRV_TYPE_2_1_0;
        break;
    case DSP_CH_CFG_LRC:
        eAudChCfg=AUD_DRV_TYPE_3_0;
        break;
    case (DSP_CH_CFG_LRS+DSP_CH_CFG_SUB):
        eAudChCfg=AUD_DRV_TYPE_2_1_1;
        break;
    case (DSP_CH_CFG_LRC+DSP_CH_CFG_SUB):
        eAudChCfg=AUD_DRV_TYPE_SURROUND;
        break;
    case DSP_CH_CFG_LRLsRs:
        eAudChCfg=AUD_DRV_TYPE_2_2_0;
        break;
    case DSP_CH_CFG_LRCS:
        eAudChCfg=AUD_DRV_TYPE_4_0;
        break;
    case (DSP_CH_CFG_LRLsRs+DSP_CH_CFG_SUB):
        eAudChCfg=AUD_DRV_TYPE_2_2_1;
        break;
    case (DSP_CH_CFG_LRCS+DSP_CH_CFG_SUB):
        eAudChCfg=AUD_DRV_TYPE_4_1;
        break;
    case DSP_CH_CFG_LRCLsRs:
        eAudChCfg=AUD_DRV_TYPE_5_0;
        break;
    case (DSP_CH_CFG_LRCLsRs+DSP_CH_CFG_SUB):
        eAudChCfg=AUD_DRV_TYPE_5_1;
        break;
    case (DSP_CH_CFG_LRCLsRs+DSP_CH_CFG_CH6):
        eAudChCfg=AUD_DRV_TYPE_6_0;
        break;
    case (DSP_CH_CFG_LRCLsRs+DSP_CH_CFG_CH6+DSP_CH_CFG_SUB):
        eAudChCfg=AUD_DRV_TYPE_6_1;
        break;
    case (DSP_CH_CFG_LRCLsRs+DSP_CH_CFG_CH6+DSP_CH_CFG_CH7):
        g_rAudDrvUpdatedAudInfo[u1DecId].u4ChannelLayout =((0x0 << 16) | (0x4 << 8) | (0x3));
        eAudChCfg=AUD_DRV_TYPE_4_4_0;//AUD_DRV_TYPE_7_0 = 3/4.0   4_4_0 is flag to UI
        break;
    case (DSP_CH_CFG_LRCLsRs+DSP_CH_CFG_CH6+DSP_CH_CFG_CH7+DSP_CH_CFG_SUB):
        eAudChCfg=AUD_DRV_TYPE_7_1;
        break;
    default:
        eAudChCfg=AUD_DRV_TYPE_UNKNOWN;
        LOG(5, TEXT("[AUD]Source AUD_DRV_TYPE_UNKNOWN!\n"));
        break;
    }
    AUD_Ch_Cfg_Notify(u1DecId, eAudChCfg);
}

/***************************************************************************
   Function : vDspAvdSamplingRateTransform
Description : sample rate change from mw to dsp
  Parameter : u4DspSF: dsp sf index, u4DspToAvdSF: AvdSF
  Return    :
***************************************************************************/
void vDspDSPSamplingRateTransform (u32* u4DspSF,AUDIO_SAMPLING_T u4DspToAvdSF)
{
    switch (u4DspToAvdSF)
    {
    case FS_8K:        // 16K
        *u4DspSF=SFREQ_8K;
    case FS_16K:        // 16K
        *u4DspSF=SFREQ_16K;
        break;
    case FS_22K :        // 22K
        *u4DspSF=SFREQ_22K;
        break;
    case FS_24K:        // 24K
        *u4DspSF=SFREQ_24K;
        break;
    case FS_32K:        // 32K
        *u4DspSF= SFREQ_32K;
        break;
    case FS_44K :        // 44K
        *u4DspSF=SFREQ_44K;
        break;
    case FS_48K:        // 48K
        *u4DspSF=SFREQ_48K;
        break;
    case FS_88K:        // 88K
        *u4DspSF=SFREQ_88K;
        break;
    case FS_96K:        // 96K
        *u4DspSF=SFREQ_96K;
        break;
    case FS_176K:       // 176K
        *u4DspSF=SFREQ_176K;
        break;
    case FS_192K:       // 192K
        *u4DspSF=SFREQ_192K;
        break;
    case FS512_44K:       // DSD
        *u4DspSF=SFREQ_SACD;
        break;
    default:
        break;
    }
}

/***************************************************************************
   Function : vDspSetSamplingRate()
Description : set dsp sample rate pri_dec
  Parameter :
  Return    :
***************************************************************************/
void vDspSetSamplingRate(void)
{
    AUD_SFREQ_IDX_T eSamplingFreq;
    u32 u4SampFreqTable;
    AUD_CH_NUM_T eInChnCnt;
    AUD_SOURCE_CFG_T* prSrcParam = DspGetSrcParam();
    AUD_OUTPUT_SETTING_CFG_T* prOutParam = DspGetOutParam();
    AUD_OUTPUT_SETTING_CFG_T* prOutHdmiParam = DspGetOutHdmiParam();

    // Change ASV state machine
    u1AsvDspBSendCfg(PRI_DEC);

    // Get Source Config from DSP for AV Decision
    DspGetSourceConfig(prSrcParam);
    LOG(5, TEXT("[DSPINT] Aud_Codec_Fmt: 0x%x, Hdmi_Max_Channel: 0x%x.\r\n"),
		prSrcParam->u1Aud_Codec_Fmt, prSrcParam->u1Hdmi_Max_Channel);

    eInChnCnt = prSrcParam->u1Aud_Input_Chan_Cnt;

    DspSetApSpdOut(prSrcParam);

    fgAsvQueryAVD(prSrcParam, prOutParam, prOutHdmiParam);
    prSrcParam->u1Aud_Input_Chan_Cnt = eInChnCnt;

    #if CONFIG_AUD_DECONLY_EN
    if(AUD_DECONLY_ON == DspGetDeconlyCtrl())
    {
        DspSetAsrcMode(PRI_DEC,SFREQ_48K);  // Disable ASRC
        LOG(LOG_DUALCTRL,"[AUD]Audio Deconly directly FS ack to DSPB.\n");
        u1AsvDspBReceiveCfg(FALSE);
        return;
    }
    #endif

    if(AUD_OUT_MEDIA_USB == uReadDspShmBYTE(B_FRONT_AOUT_MEDIA_TYPE))
    {
        vDecideSpdifOutput(prSrcParam, prOutParam);
        AudCfg_SpdifEnable(AUD_AOUT1);
    }

    // Get Output and Table sampling freq
    vDspSamplingFreqTransformAvdToDsp(prOutParam->u1Sampling_Rate,&eSamplingFreq,&u4SampFreqTable);
    vAudUpdateIecCfg(FALSE, prOutParam->eIec_Cfg,TRUE);
    vAudSetChannelStatus(prOutParam->u1Sampling_Rate);
    vAudUpdateChDelay(prOutParam->rChan_Delay,TRUE);

    // Update settings to DSP-Aout2
    //marked by mtk40292 because of the aout2 don't need upadte speaker configure
    //vDspUpdateSpkCfg(prOutHdmiParam->u1Aud_Output_Chan_Cnt,FALSE);
    vDspUpdateAout2SamplingFreq(prOutHdmiParam->u1Sampling_Rate);

    vAudUpdateIecCfg(prOutHdmiParam->fgHBROutI2S, prOutHdmiParam->eIec_Cfg,FALSE);

    vAudUpdateChDelay(prOutHdmiParam->rChan_Delay,FALSE);

    // Load post processing table
    vDspSetTable(u4SampFreqTable);

    //set play mode
    DspCfgSetPlayMode(prSrcParam);

    DspSetAOutMediaType(AUD_AOUT1, uReadDspShmBYTE(B_FRONT_AOUT_MEDIA_TYPE));
    DspSetAOutMediaType(AUD_AOUT2, uReadDspShmBYTE(B_REAR_AOUT_MEDIA_TYPE));

    // Set clock and ack to dsp ; or turn aout off
    g_u4DspSetSampleRateAck[PRI_DEC] = 1;
    u1AsvDspBReceiveCfg(FALSE);

    DspSetArm2FsReady(1);  //Set sample rate ready for 2nd audio on ARM2


#if DOLBY_REROUTING_SUPPORT
    // Dolby Rerouting Update Coefficient (AOUT & HDMI 8x8 matrix) -- Water 20091021
    vDolbyReroutingConfig();
#endif
#if DOWNMIX_BY_RISC
    vSetDefDownmixCoef(FALSE,FALSE);
    vSetDefDownmixCoef(TRUE,FALSE);

#endif

}

/***************************************************************************
Function    : void vDspSetSamplingRateViaDec(u8 u1Dec)
Description : Set sample rate for Dec2 and Dec3
Parameter   : u1Dec: Decoder id, only for SEC_DEC &TER_DEC
Return      :
***************************************************************************/
void vDspSetSamplingRateViaDec(u8 u1DecId)
{
    AUD_SFREQ_IDX_T eSamplingFreq;
    u32 u4TableSamplingFreq;
    AUD_CH_NUM_T eInputChanCntTmp;
    AUD_SOURCE_CFG_T* prSrcParam = DspGetSrcParam();
    AUD_OUTPUT_SETTING_CFG_T* prOutParam = DspGetOutParam();
    AUD_OUTPUT_SETTING_CFG_T* prOutHdmiParam = DspGetOutHdmiParam();
    u8 u1FrnMedia, u1RearMedia;

    AUD_VERIFY((SEC_DEC == u1DecId)||(TER_DEC == u1DecId));
    u1AsvDspBSendCfg(u1DecId);

    // Get Source Config from DSP for AV Decision
    DspGetFourFiveDecConfig(u1DecId, prSrcParam);
    eInputChanCntTmp = prSrcParam->u1Aud_Input_Chan_Cnt;

    fgAsvQueryAVD(prSrcParam, prOutParam, prOutHdmiParam);
    prSrcParam->u1Aud_Input_Chan_Cnt = eInputChanCntTmp;

    if(AUD_OUT_MEDIA_LINE_IN == uReadDspShmBYTE(B_FRONT_AOUT_MEDIA_TYPE))
    {
        AudCfg_SpdifEnable(AUD_AOUT1);
    }

    // Get Output and Table sampling freq
    vDspSamplingFreqTransformAvdToDsp(prOutParam->u1Sampling_Rate,&eSamplingFreq,&u4TableSamplingFreq);
    vAudUpdateIecCfg(FALSE, prOutParam->eIec_Cfg,TRUE);
    vAudSetChannelStatus(prOutParam->u1Sampling_Rate);
    vAudUpdateChDelay(prOutParam->rChan_Delay,TRUE);

    // Update settings to DSP-Aout2
    vDspUpdateSpkCfg(prOutHdmiParam->u1Aud_Output_Chan_Cnt,FALSE);
    vDspUpdateAout2SamplingFreq(prOutHdmiParam->u1Sampling_Rate);

    vAudUpdateIecCfg(prOutHdmiParam->fgHBROutI2S, prOutHdmiParam->eIec_Cfg,FALSE);

    vAudUpdateChDelay(prOutHdmiParam->rChan_Delay,FALSE);

    // Load post processing table
    vDspSetTable(u4TableSamplingFreq);

    DspCfgSetPlayMode(prSrcParam);

    u1FrnMedia = uReadDspShmBYTE(B_FRONT_AOUT_MEDIA_TYPE);
    u1RearMedia = uReadDspShmBYTE(B_REAR_AOUT_MEDIA_TYPE);
    DspSetAOutMediaType(AUD_AOUT1, u1FrnMedia);
    DspSetAOutMediaType(AUD_AOUT2, u1RearMedia);

    //check aout media type
    if (SEC_DEC == u1DecId)
    {
        LOG(LOG_CTRLF, TEXT(" DspSetSamplingRate AOUT1 Reset .\n"));
        //Aout1 Reinit
        g_u4DspSetSampleRateAck[SEC_DEC] = 1;
        vDspCmd(UOP_DSP_FS_ACK_DEC4);
    }
    if (TER_DEC == u1DecId)
    {
        //Aout2 Reinit
        g_u4DspSetSampleRateAck[TER_DEC] = 1;
        LOG(LOG_CTRLF, TEXT(" DspSetSamplingRate AOUT2 Reset .\n"));
        vDspCmd(UOP_DSP_FS_ACK_DEC5);
    }

    DspSetArm2FsReady(1);  //Set sample rate ready for 2nd audio on ARM2

#if DOLBY_REROUTING_SUPPORT
    // Dolby Rerouting Update Coefficient (AOUT & HDMI 8x8 matrix) -- Water 20091021
    vDolbyReroutingConfig();
#endif
}


void vDvdSetSamplRate(AUDIO_SAMPLING_T eSmpRate)
{
    //u16  u2DspData;
    u16  u2MediaType;
    AUD_SFREQ_IDX_T eSamplingFreq;
    u32 u4TableSamplingFreq;
    AUD_SOURCE_CFG_T* prSrcParam = DspGetSrcParam();
    AUD_OUTPUT_SETTING_CFG_T* prOutParam = DspGetOutParam();
    AUD_OUTPUT_SETTING_CFG_T* prOutHdmiParam = DspGetOutHdmiParam();

    DspSetAOutMediaType(AUD_AOUT1, uReadDspShmBYTE(B_FRONT_AOUT_MEDIA_TYPE));
    DspSetAOutMediaType(AUD_AOUT2, uReadDspShmBYTE(B_REAR_AOUT_MEDIA_TYPE));

    DspGetInputChanCountFromDvd();
    DspGetSamplingRateFromDvd(eSmpRate);

    LOG(LOG_ADSP_INFO, TEXT("[vDvdSetSamplRate] ChNum = 0x%x, eSmpRate = 0x%x.!!! \r\n"),
		prSrcParam->u1Aud_Input_Chan_Cnt, prSrcParam->u1Aud_Sampling_Rate);

    //reset aout status report to DVP
    DspSetAsrcAoutStatusToDvp(0);

    DspSetDvdSoftMute();

    DspGetAOutMediaType(AUD_AOUT1, &u2MediaType);

    if((AUD_OUT_MEDIA_DVD   == u2MediaType) ||
       (AUD_OUT_MEDIA_NONE  == u2MediaType) ||
       (AUD_OUT_MEDIA_UNDEF == u2MediaType))
    {
        //Processing Mode
        DspCfgSetDvdPlayMode();

        prSrcParam->u1Aud_Codec_Fmt = DspGetDvdCodecFmt();
        prSrcParam->u1Aud_Sampling_Rate = eSmpRate;

        DspSetDvdSpdOut(prSrcParam, eSmpRate);

        LOG(LOG_FEATURE, TEXT("[DVD]eSmpRate = 0x%x. Codec = 0x%x\n"), eSmpRate, prSrcParam->u1Aud_Codec_Fmt);

        fgAsvQueryAVD(prSrcParam, prOutParam, prOutHdmiParam);
        vDspSamplingFreqTransformAvdToDsp(prOutParam->u1Sampling_Rate,&eSamplingFreq,&u4TableSamplingFreq);

        if(AUD_OUT_MEDIA_DVD == u2MediaType)
        {
            vDecideSpdifOutput(prSrcParam, prOutParam);
            if(AUD_IEC_CFG_RAW == prOutParam->eIec_Cfg)
            {
                AudCfg_SpdifEnable(AUD_DVD_OUT);
            }
            else
            {
                AudCfg_SpdifEnable(AUD_AOUT1);
            }

            vAudUpdateIecCfg(FALSE, prOutParam->eIec_Cfg,TRUE);
            vAudSetChannelStatus(prOutParam->u1Sampling_Rate);
            vAudUpdateIecCfg(prOutHdmiParam->fgHBROutI2S, prOutHdmiParam->eIec_Cfg,FALSE);
        }

        vDspSetTable(u4TableSamplingFreq);
        LOG(LOG_DUALCTRL, TEXT("DVD To AOUT1 Reset\n"));

        //Aout1 Reinit
        DspSetAsrcAoutStatusToDvp(1);
    }

    DspGetAOutMediaType(AUD_AOUT2, &u2MediaType);

    // Load post processing table
    if(AUD_OUT_MEDIA_DVD == u2MediaType)
    {
        LOG(LOG_DUALCTRL, TEXT("DVD To AOUT2 Reset\n"));
        //Aout1 Reinit
        DspSetAsrcAoutStatusToDvp(1);
    }
}

/***************************************************************************
Function :fgIntFlowControl
Description :
Parameter :
Return    :
***************************************************************************/
bool fgIntFlowControl (u32 u4DspRIntSD, u32 u4DspRIntLD, u8 u1DecId)
{
    //u32 i = 0;
    if (u4DspRIntSD == D2RC_FLOW_CONTROL_PLAY_GOT) //Dsp Get Play command
    {
        LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP (%d)th Decoder int :get play cmd \n"), u1DecId);
        if ((SEC_DEC == u1DecId) || (TER_DEC == u1DecId))
        {
            i4AsvDspNotifyPlayCmdGot(u1DecId);
        }
        else
        {
            vAsvNotifyPlayCmdGot(u1DecId);
        }
    }
    else if (u4DspRIntSD == D2RC_FLOW_CONTROL_DEC_READY)
    {
        LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP (%d)th Decoder int :decoder ready\n"),u1DecId);

        if ((SEC_DEC == u1DecId) || (TER_DEC == u1DecId))
        {
            i4AsvDspNotifyDecReady(u1DecId);
        }
        else
        {
            u1AsvDspBDecReady(u1DecId);
            vDspGetInputChannelCountForMW(u1DecId);
        }
    }
    else if (u4DspRIntSD == D2RC_FLOW_CONTROL_STOP_OK)
    {
        DspResetSpectrumInfo();

        LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP (%d)th Decoder int :decoder stopped\n"),u1DecId);
        if ((SEC_DEC == u1DecId) || (TER_DEC == u1DecId))
        {
            i4AsvDspNotifyDecStopped(u1DecId);
        }
        else
        {
            u1AsvDspBDecStopped(u1DecId);
            DspReSetDownmixDram();
            vDspMetadataInit();
        }
    }
    else if (u4DspRIntSD == D2RC_FLOW_CONTROL_SAMPLING_RATE)
    {
        if (u1DecId == PRI_DEC)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int :set sampling rate\n"));
            vDspSetSamplingRate();

        }
        else if (u1DecId == SEC_DEC)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive 2nd decoder DSP int :set sampling rate\n"));
            vDspSetSamplingRateViaDec(u1DecId);
        }
        else if(u1DecId == TER_DEC)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive 3rd decoder DSP int :set sampling rate\n"));
            vDspSetSamplingRateViaDec(u1DecId);
        }
        else
        {
            AUD_VERIFY(0);
        }
    }
    else if (u4DspRIntSD == D2RC_FLOW_CONTROL_END_OF_STREAM)
    {
        LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP(%d) int : END_OF_STREAM, ICBId=0x%X \n"),
            u1DecId, g_u4AudPriICBId);
        u1AsvDspBFlushDone(u1DecId);
        g_fgAudioEosDone = TRUE;
    }

    return TRUE;
}

/***************************************************************************
Function : vDspAIntSrv
Description : Interrupt service routine of DSP A
Parameter :
Return    :
***************************************************************************/
void vDspAIntSrv(u32 u4DspUopMsg)
{
    //Interrupt Service of DSP A
    switch ((u4DspUopMsg>>8) & 0xFF)
    {
    case INT_D2RC_AOUT_STATUS:
        if (g_u4DspRIntSD == AOUT_CONTROL_AOUT_OFF)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int AOUT OFF\n"));
            g_fgIsAoutConnected = FALSE;
            u1AsvDspAAoutStopped();
        }
        else if (g_u4DspRIntSD == AOUT_CONTROL_AOUT_ON)
        {
            u16 u2Media;
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int AOUT ON\n"));
            g_fgIsAoutConnected = TRUE;
            u1AsvDspAAoutStarted();
        }
        break;

    case INT_D2RC_AOUT2_STATUS:
        if (g_u4DspRIntSD == AOUT2_CONTROL_AOUT2_OFF)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int AOUT2 OFF\n"));
            u1AsvDspAAout2Stopped();
        }
        else if (g_u4DspRIntSD == AOUT2_CONTROL_AOUT2_ON)
        {
            u16 u2Media;
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int AOUT2 ON\n"));
            u1AsvDspAAout2Started();
        }
        break;

    case INT_D2RC_DISCONNECT_CMD:
        if (g_u4DspRIntSD == D2RC_PRIMARY_DECODER)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int PRIMARY DISCONNECT_CMD\n"));
        }
        else if (g_u4DspRIntSD == D2RC_SECONDARY_DECODER)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int SECONDARY DISCONNECT_CMD\n"));
        }
        else if (g_u4DspRIntSD == D2RC_FOURTH_DECODER)
        {
            LOG(LOG_CTRLF, TEXT("AUD DEC2 DISCONNNECT\n"));
        }
        else if (g_u4DspRIntSD == D2RC_FIFTH_DECODER)
        {
            LOG(LOG_CTRLF, TEXT("AUD DEC3 DISCONNNECT\n"));
        }
        else
        {
            LOG(LOG_CTRLF, TEXT("AUD DISCONNNECT Decoder ID(0x%x)error.\n"), g_u4DspRIntSD);
        }

        _fgCSIIDbgPlaying = FALSE;
        break;

    case INT_D2RC_CONNECT_CMD:      //Dsp get resume command
        if (g_u4DspRIntSD ==D2RC_PRIMARY_DECODER)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int PRIMARY CONNECT_CMD\n"));
            if (u1AudDspGetState() == ST_DSP_RESUMING)
            {
                u1AsvDspResumed(PRI_DEC);
            }
        }
        else if (g_u4DspRIntSD ==D2RC_FOURTH_DECODER)
        {
            LOG(LOG_CTRLF, TEXT("AUD DEC2 CONNNECTTED\n"));
        }
        else if(g_u4DspRIntSD ==D2RC_FIFTH_DECODER)
        {
            LOG(LOG_CTRLF, TEXT("AUD DEC3 CONNNECTTED\n"));
        }
        else
        {
            LOG(LOG_CTRLF, TEXT("AUD CONNNECTTED INT error 0x%x.\n"), g_u4DspRIntSD);
        }

        _fgCSIIDbgPlaying = TRUE;
        break;

    case INT_D2RC_MIXER_REAL_CONNECT_OK:
        if (g_u4DspRIntSD ==D2RC_PRIMARY_DECODER)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int PRIMARY CONNECT OK!\n"));
        }
        else if (g_u4DspRIntSD ==D2RC_SECONDARY_DECODER)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int SECONDARY CONNECT OK!\n"));
        }
        else if (g_u4DspRIntSD ==D2RC_FOURTH_DECODER)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int DEC2 CONNECT OK!\n"));
        }
        else if (g_u4DspRIntSD ==D2RC_FIFTH_DECODER)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int DEC3 CONNECT OK!\n"));
        }
        break;

    case INT_D2RC_MIXER_REAL_DISCONNECT_OK:
        if (g_u4DspRIntSD ==D2RC_PRIMARY_DECODER)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int PRIMARY DISCONNECT OK!\n"));
        }
        else if (g_u4DspRIntSD ==D2RC_SECONDARY_DECODER)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int SECONDARY DISCONNECT OK!\n"));
        }
        else if (g_u4DspRIntSD ==D2RC_FOURTH_DECODER)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int DEC2 DISCONNECT OK!\n"));
        }
        else if (g_u4DspRIntSD ==D2RC_FIFTH_DECODER)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int DEC3 DISCONNECT OK!\n"));
        }
        break;

    case INT_D2RC_DSP_AOUT_NOTIFY:
        if (g_u4DspRIntSD ==LAST_AOUT_NOTIFY)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int LAST_AOUT_NOTIFY\n"));
            u1AsvDspADisconnected(PRI_DEC);
        }
        else if (g_u4DspRIntSD ==FIRST_AOUT_NOTIFY)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int FIRST_AOUT_NOTIFY\n"));
            //Dsp Real Play
            u1AsvDspAConnected(PRI_DEC);
            g_fgFirstAoutArrive = TRUE;
        }
        else if (g_u4DspRIntSD ==DEC2_FIRST_AOUT_NOTIFY)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int DEC2_FIRST_AOUT_NOTIFY\n"));
        }
        else if (g_u4DspRIntSD ==DEC2_LAST_AOUT_NOTIFY)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int DEC2_LAST_AOUT_NOTIFY\n"));
            if (g_rAudDrvState[SEC_DEC] == AUD_DRV_STOPPING)
            {
                i4AsvSendStopCmd(SEC_DEC);
            }
        }
        else if (g_u4DspRIntSD ==DEC3_FIRST_AOUT_NOTIFY)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int DEC3_FIRST_AOUT_NOTIFY\n"));
        }
        else if (g_u4DspRIntSD ==DEC3_LAST_AOUT_NOTIFY)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int DEC3_LAST_AOUT_NOTIFY\n"));
            if (g_rAudDrvState[TER_DEC] == AUD_DRV_STOPPING)
            {
                i4AsvSendStopCmd(TER_DEC);
            }
        }
        else
        {
            LOG(LOG_CTRLF, TEXT("AUD Stop DSP int error 0x%x.\n"), g_u4DspRIntSD);
        }

        break;

    case INT_D2RC_EXTMIX_STATUS:
        if (g_u4DspRIntSD == EXTMIX_PLAY_GET)
        {
            LOG(LOG_FEATURE, TEXT("DSPA receive PLAY Cmd,Wait DSPA processing\n"));
        }
        else if (g_u4DspRIntSD == EXTMIX_PLAY_OK)
        {
            i4AsvGpsMixDspNotifyPlayCmdDone();
        }
        else if (g_u4DspRIntSD == EXTMIX_STOP_GET)
        {
            LOG(LOG_FEATURE, TEXT("DSPA receive STOP Cmd,Wait DSPA processing\n"));
        }
        else if (g_u4DspRIntSD == EXTMIX_STOP_OK)
        {
            i4AsvGpsMixDspNotifyStopCmdDone();
        }
        else if (g_u4DspRIntSD == EXTMIX_PAUSE_GET)
        {
            LOG(LOG_FEATURE, TEXT("DSPA receive PAUSE Cmd,Wait DSPA processing\n"));
        }
        else if (g_u4DspRIntSD == EXTMIX_PAUSE_OK)
        {
            i4AsvGpsMixDspNotifyPauseCmdDone();
        }
        else if (g_u4DspRIntSD == EXTMIX_PLAY_OK)
        {
            i4AsvGpsMixDspNotifyResumeCmdDone();
        }
        else if (g_u4DspRIntSD == EXTMIX_DATA_CONSUMED)
        {
            i4AsvGpsMixDspNotifyConsumedData();
        }
        else
        {
            LOG(LOG_FAIL, TEXT("DSPA INT_D2RC_EXTMIX_STATUS error Int = 0x%x\n"),g_u4DspRIntSD);
        }

        break;

    case INT_D2RC_MIXER_STEP:      //Dsp get step command
        LOG(LOG_CTRLF, TEXT("[AUD] DspCtrl receive DSP int INT_D2RC_MIXER_STEP\n"));

        if (g_u4DspRIntSD == D2RC_STEP_DONE)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int of PRIMARY D2RC_STEP_DONE\n"));
            u1AsvDspAStepDone(PRI_DEC);
        }
#ifdef NEW_STEP_FLOW
        else if (g_u4DspRIntSD == D2RC_STEPTOEND_CMD_OK)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int of PRIMARY D2RC_STEPTOEND_CMD_OK\n"));
        }
        else if (g_u4DspRIntSD == D2RC_STEP_CANCEL_OK)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int of PRIMARY D2RC_STEP_CANCEL_OK\n"));
            vAsvDspAStepCancelDone(PRI_DEC);
        }
#endif
        else if (g_u4DspRIntSD == D2RC_STEP_CMD_OK)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int of PRIMARY D2RC_STEP_CMD_OK\n"));
#ifdef NEW_STEP_FLOW
            vAsvDspAStepCmd(PRI_DEC);
#endif
        }
        break;

    case INT_D2RC_MIXER_PTS_ACCURATE:
        if (g_u4DspRIntSD == D2RC_PTS_BEGIN_DONE)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int of PRIMARY D2RC_PTS_BEGIN_DONE\n"));
            vAsvNotifyBeginPtsDone(PRI_DEC);
        }
        else if (g_u4DspRIntSD == D2RC_PTS_END_DONE)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int of PRIMARY D2RC_PTS_END_DONE\n"));
            vAsvNotifyEndPtsDone(PRI_DEC, TRUE);
        }
        else if (g_u4DspRIntSD == D2RC_PAUSE_PTS_BEGIN_DONE)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int of PRIMARY D2RC_PAUSE_PTS_BEGIN_DONE\n"));
            vAsvNotifyPauseBeginPts(PRI_DEC);
        }
        else if (g_u4DspRIntSD == D2RC_STEP_PTS_END_DONE)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int of PRIMARY D2RC_STEP_PTS_END_DONE\n"));
            vAsvNotifyEndPtsDone(PRI_DEC, FALSE);
        }
        break;

    case INT_D2RC_REENCODER_STATUS:
        if (g_u4DspRIntSD ==REENC_STOP_OK)
        {
            u1AsvReencStopped(RE_ENC);
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int REENC_STOP_OK\n"));
        }
        else if (g_u4DspRIntSD ==REENC_START_CMD_OK)
        {
            u1AsvReencStarted(RE_ENC);
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int REENC_START_CMD_OK\n"));
        }
        else if (g_u4DspRIntSD ==REENC_SEND_BITSTREAM)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int REENC_SEND_BITSTREAM\n"));
        }
        else if (g_u4DspRIntSD ==REENC_STOP_CMD_OK)
        {
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int REENC_STOP_CMD_OK\n"));
        }
        break;

    case INT_D2RC_DSPA_STATUS:
        if(g_u4DspRIntSD == DSPA_WAKE_UP_OK)
        {
            _fgDspAWakeUpFlag = TRUE;
            x_event_set(_hDspAStatusHandle);
            LOG(LOG_ADSP_INFO, TEXT("_fgDspAWakeUpFlag = 1\n"));

        }
        break;

    case INT_D2RC_POST_REINIT_STATUS:
        if (g_u4DspRIntSD == PR_FADE_OUT_OK)
        {
            if ((g_aNotifyInfo.prDspSrcParam != NULL) && (g_aNotifyInfo.prDspOutputParam != NULL))
            {
                vAudSeProcessNotify(AUD_SE_NOTIFY_SET_PP_TAB, &g_aNotifyInfo);
            }
            AudSeSetPostDramOk();
        }
        else if (g_u4DspRIntSD == PR_FADE_IN_OK)
        {
            LOG(LOG_FEATURE, TEXT("[AInt_Srv]POST_REINIT_STATUS: FADE_IN.\n"));
        }
        break;


    default:
        break;
    }
}


/***************************************************************************
Function : vDspBIntSrv
Description : Interrupt service routine of DSP B
Parameter :
Return    :
***************************************************************************/
void vDspBIntSrv(u32 u4DspUopMsg)
{
    //Interrupt Service of DSP B
    switch ((u4DspUopMsg >> 8) & 0xFF)
    {
    case INT_D2RC_FLOW_CONTROL:
        fgIntFlowControl(g_u4DspRIntSD, g_u4DspRIntLD, PRI_DEC);
        break;

    case INT_D2RC_FLOW_CONTROL_DEC4:
        fgIntFlowControl(g_u4DspRIntSD, g_u4DspRIntLD, SEC_DEC);
        break;

    case INT_D2RC_FLOW_CONTROL_DEC5:
        fgIntFlowControl(g_u4DspRIntSD, g_u4DspRIntLD, TER_DEC);
        break;

    case INT_D2RC_PRIMARY_INBAND_CMD:
        LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int :INT_D2RC_PRIMARY_INBAND_CMD \n"));
        break;

    case INT_D2RC_SECONDARY_INBAND_CMD:
        LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int :INT_D2RC_SECONDARY_INBAND_CMD \n"));
        break;

    case INT_D2RC_HDCD_TRK_STM_CHG:
    {
        u8 u1DecType = 0;
        DspGetDec1StrType(&u1DecType);
        if(u1DecType == CDDA_STREAM ||u1DecType == CDDA24_STREAM)
        {
            bool fgTrack = FALSE;
            if(((g_u4DspRIntSD >> 8) & 0x03) == D2RC_STM_HDCD_ON)
            {
                fgTrack = TRUE;
            }
            u1AsvDsp_Hdcd_Trk_Stm_Chg(PRI_DEC, fgTrack);
        }

    }
        break;

    case INT_D2RC_DEEMPHASIS_NOITFY:
        switch (g_u4DspRIntSD >> 8)
        {
        case DEEMPH_ENABLE:
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int :INT_D2RC_DEEMPHASIS_NOITFY DEEMPH_ENABLE \n"));
            break;
        case DEEMPH_DISABLE:
            LOG(LOG_CTRLF, TEXT("DspCtrl receive DSP int :INT_D2RC_DEEMPHASIS_NOITFY DEEMPH_DISABLE \n"));
            break;
        }
        break;

    case INT_D2RC_DSPB_STATUS:
        if(g_u4DspRIntSD == DSPB_WAKE_UP_OK)
        {
            _fgDspBWakeUpFlag = TRUE;
            x_event_set(_hDspBStatusHandle);
            LOG(LOG_ADSP_INFO, TEXT("_fgDspBWakeUpFlag = 1\n"));
        }
        break;

    default:
        break;
    }
}

/***************************************************************************
     Function : vDSPIntSvc
  Description : DSP Interrupt Service
Parameter     : ucDspId used only for DBGLogB purpose
    Return    : None
    Note      : u4DspRIntData : 24bit Short Data
                u4DspRIntData2: 24bit Long Data
***************************************************************************/
void vDspIntSvc(u32 u4DspUopMsg)
{
    u8 u1DspId = (u4DspUopMsg >> 24);

    if(u1DspId == DSPA_ID)
    {
        vDspAIntSrv(u4DspUopMsg);
    }
    else if(u1DspId == DSPB_ID)
    {
        vDspBIntSrv(u4DspUopMsg);
    }
    else if(u1DspId == DSPC_ID)
    {
        switch ((u4DspUopMsg >> 8) & 0xFF)
        {
        case INT_D2RC_FLOW_CONTROL2:
        case INT_D2RC_FLOW_CONTROL_DEC3:
        case INT_D2RC_SECONDARY_INBAND_CMD:
        case INT_D2RC_HDCD_TRK_STM_CHG:
        case INT_D2RC_MIXING_METADATA_UPDATE:
        case INT_D2RC_DEEMPHASIS_NOITFY:
            vDspBIntSrv(u4DspUopMsg);
            break;
        case INT_D2RC_REENCODER_STATUS:
            vDspAIntSrv(u4DspUopMsg);
            break;
        default:
            break;
        }
    }
}



void AudSetDvdMixCfg(AUDIO_SAMPLING_T eSmpRate)
{
    vDvdSetSamplRate(eSmpRate);
}



