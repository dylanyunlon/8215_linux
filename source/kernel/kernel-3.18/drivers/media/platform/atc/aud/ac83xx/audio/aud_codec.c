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
#include "aud_drv.h"
#include "aud_debug.h"
#include "DspFunc.h"
#include "aud_if.h"
#include "aud_dsp_cfg.h"
#include "DspShm.h"
#include "DspUop.h"

AUD_DRV_AUD_INFO_T g_rAudDrvUpdatedAudInfo[TER_DEC + 1];
AUD_DRV_AUD_INFO_T *g_prAudDrvUpdatedAudInfo[TER_DEC + 1]; /*Xiangyang added for linux callback*/

/***************************************************************************
*    Function : vAudCodecSet_Bass_Management_Mode
* Description :
*   Parameter : 0:LFE mode; 1:LFE+main mode
*   Return    : None
*   Note      :
***************************************************************************/
void vAudCodecSet_Bass_Management_Mode(AUD_DEC_BASS_MANAGEMENT_MODE_T t_BM_mode)
{
    u16 u2ShmIndex = B_SE_BASS_MANAGEMENT_MODE;
    u8  u1ShmData = uReadShmUINT8(u2ShmIndex);
    //bit0: 0--LFE mode; 1--LFE+main mode
    if(AUD_DEC_BASS_MANAGEMENT_LFE_MAIN == t_BM_mode)
    {
        u1ShmData |= (u8)0x01;
    }
    else
    {
        u1ShmData &= ~((u8)0x01);
    }
    vWriteShmUINT8(u2ShmIndex, u1ShmData);
}

/***************************************************************************
*   Function : vAudCodecSet_DTS_DRC
*   Description :
*   Parameter :
*   Return    : None
*   Note      : This function should be called by ASH
***************************************************************************/
void vAudCodecSet_DTS_DRC(u16 u2DecId, u16 u2Value)
{
    u16 u2ShmIndex = W_DTSFLAG;
    u16 u2ShmIndex_1 = D_DTS_DYN_VALUE;
    u16 u2UopIndex = UOP_DSP_DTS_FLAG;
    u16 u2UopIndex1 = UOP_DSP_DTS_DYN_VALUE;

    u32 u4Tmp = 0;

    switch(u2Value)
    {
    case AUD_DRV_DTS_DRC_OFF:
        u4Tmp = 0x0;
        break;
    case AUD_DRV_DTS_DRC_MODE_0:
        u4Tmp = 0x000CCCCC;
        break;
    case AUD_DRV_DTS_DRC_MODE_1:
        u4Tmp = 0x00199999;
        break;
    case AUD_DRV_DTS_DRC_MODE_2:
        u4Tmp = 0x00266666;
        break;
    case AUD_DRV_DTS_DRC_MODE_3:
        u4Tmp = 0x00333332;
        break;
    case AUD_DRV_DTS_DRC_MODE_4:
        u4Tmp = 0x003FFFFF;
        break;
    case AUD_DRV_DTS_DRC_MODE_5:
        u4Tmp = 0x004CCCCC;
        break;
    case AUD_DRV_DTS_DRC_MODE_6:
        u4Tmp = 0x00599998;
        break;
    case AUD_DRV_DTS_DRC_MODE_7:
        u4Tmp = 0x00666665;
        break;
    case AUD_DRV_DTS_DRC_MODE_8:
        u4Tmp = 0x00733332;
        break;
    case AUD_DRV_DTS_DRC_MODE_9:
        u4Tmp = 0x007FFFFF;
        break;
    default:
        LOG(LOG_FAIL, TEXT("[AUD_MW] Set AUD_DEC_SET_TYPE_DTS_DRC as Wrong value.\n"));
        break;
    }

    if(u2Value != AUD_DRV_DTS_DRC_OFF)
    {
        u2Value = u2ReadShmUINT16(u2ShmIndex);
        u2Value = u2Value | 0x0001; //set DTS_DRC on
    }
    else if(u2Value == AUD_DRV_DTS_DRC_OFF)
    {
        u2Value = u2ReadShmUINT16(u2ShmIndex);
        u2Value = u2Value & 0xFFFE; //set DTS_DRC off
    }
    vWriteShmUINT16(u2ShmIndex, u2Value);
    vWriteShmUINT32(u2ShmIndex_1, u4Tmp);
    vSendADSPCmd(u2UopIndex);
    vSendADSPCmd(u2UopIndex1);
    LOG(LOG_FEATURE, TEXT("[AUD_Codec] Set W_DTSFLAG(bit 0: DTS_DRC)=0x%X.\n"),u2Value);
    LOG(LOG_FEATURE, TEXT("[AUD_Codec] Set D_DTS_DYN_VALUE=0x%X.\n"),u4Tmp);
}



#if  CONFIG_DRV_HDMI_RX
void vAudCodecSet_HDMIInPCMPara(const AUD_DRV_PCM_SETTING_T *ptHDMIInPcmSetting)
{

    AUD_LPCM_DVD_SAMPLING_RATE_T eAudLpcmSamplingRate;
    //  Transfer Channel Assignment
    switch(ptHDMIInPcmSetting->u4SamplingFreq)
    {
       case 48000:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_48KHZ;
           break;
       case 96000:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_96KHZ;
           break;
       case 192000:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_192KHZ;
           break;
       case 24000:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_24KHZ;
           break;
       case 12000:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_12KHZ;
           break;
       case 44100:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_44KHZ;
           break;
       case 88200:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_88KHZ;
           break;
       case 176400:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_176KHZ;
           break;
       case 22050:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_22KHZ;
           break;
       case 11025:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_11KHZ;
           break;
       case 32000:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_32KHZ;
           break;
       case 16000:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_16KHZ;
           break;
       case 8000:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_8KHZ;
           break;
       default:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_48KHZ;
           break;
    }
   
    vWriteShmUINT8(B_BDLPCM_FREQUENCY, eAudLpcmSamplingRate);
    vWriteShmUINT8(B_BDLPCM_CHANNEL_ASSIGNMENT, ptHDMIInPcmSetting->u1ChannelAssign);    
    //vWriteShmUINT8(B_BDLPCM_Q, ptHDMIInPcmSetting->u1BitResolution);
    vWriteShmUINT8(B_BDLPCM_Q, 0x03); 
    vWriteShmUINT8(B_DVD_AUDIO_STEREO_PROHIBIT, 0);
   // Set default value for HDMI In PCM   
    vWriteDspShmBYTE(B_BDLPCM_ON, 0x00);
    vWriteShmUINT8(B_LPCM_DRC_VALUE, 0x80);
    DspSetLpcmDrcValue(0x8000);
    vWriteDspShmBYTE(B_LPCM_DRC_FLAG, 0x00);    
    vWriteShmUINT8(B_LPCM_DEC_TYPE, 0x05);
    DspSetSpdifInFlag(0x4 << 8);
}
#endif

void vAudCodecSet_LPCMPara(const AUD_DRV_PCM_SETTING_T *ptPcmSetting)
{
    AUD_LPCM_DVD_SAMPLING_RATE_T eAudLpcmSamplingRate;
    u32 u4SamplingRete = ptPcmSetting->u4SamplingFreq/1000;    
    LOG(LOG_CTRLF, TEXT("u4SamplingFreq = %d.\n"), ptPcmSetting->u4SamplingFreq);
    
    //  Transfer Channel Assignment
    switch(u4SamplingRete)
    {
       case 48:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_48KHZ;
           break;
       case 96:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_96KHZ;
           break;
       case 192:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_192KHZ;
           break;
       case 24:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_24KHZ;
           break;
       case 12:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_12KHZ;
           break;
       case 44:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_44KHZ;
           break;
       case 88:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_88KHZ;
           break;
       case 176:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_176KHZ;
           break;
       case 22:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_22KHZ;
           break;
       case 11:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_11KHZ;
           break;
       case 32:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_32KHZ;
           break;
       case 16:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_16KHZ;
           break;
       case 8:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_8KHZ;
           break;
       default:
           eAudLpcmSamplingRate = LPCM_DVD_SAMPLING_RATE_48KHZ;
           break;
    }

    vWriteShmUINT8(B_BDLPCM_FREQUENCY, eAudLpcmSamplingRate);
    vWriteShmUINT8(B_BDLPCM_Q, ptPcmSetting->u1BitResolution);
    vWriteShmUINT8(B_BDLPCM_CHANNEL_ASSIGNMENT, ptPcmSetting->u1ChannelAssign);
    //vWriteShmUINT8(B_LPCM_DEEMPHASIS, tPcmSetting.tPcmInfo.fgDeEmphasis);
    ///workaround for ASF file audio format
    if (ptPcmSetting->tPcmInfo.ePCM_Format == AUD_DRV_PCM_FMT_NORMAL_PCM)
    {
        vWriteShmUINT8(B_LPCM_DEC_TYPE, 0x0D);
    }
    else if (ptPcmSetting->tPcmInfo.ePCM_Format == AUD_DRV_PCM_FMT_WAVE)
    {
        if(0x01 == uReadDspShmBYTE(B_LPCM_DLNA_FLAG))
        {
            vWriteShmUINT8(B_LPCM_DEC_TYPE, 0x02);      //decoding type of DLNA pcm_fmt_wav
        }
        else
        {
            vWriteShmUINT8(B_LPCM_DEC_TYPE, 0x0A);     //normal pcm_fmt_wav
        }
    }
    else if( ptPcmSetting->tPcmInfo.ePCM_Format == AUD_DRV_PCM_FMT_ADPCM)
    {
        vWriteShmUINT8(B_LPCM_DEC_TYPE, 0x31);
        vWriteShmUINT16(W_ADPCM_BLOCK_ALIGN, ptPcmSetting->tPcmInfo.u2BlockAlign);
        vWriteShmUINT8(B_ADPCM_CH_NUM, (ptPcmSetting->u1ChannelAssign+1));
    }
    else if( ptPcmSetting->tPcmInfo.ePCM_Format == AUD_DRV_PCM_FMT_ADPCM_MS)
    {
        vWriteShmUINT8(B_LPCM_DEC_TYPE, 0x11);
        vWriteShmUINT16(W_ADPCM_BLOCK_ALIGN, ptPcmSetting->tPcmInfo.u2BlockAlign);
        vWriteShmUINT8(B_ADPCM_CH_NUM, (ptPcmSetting->u1ChannelAssign+1));
    }
    else if (ptPcmSetting->tPcmInfo.ePCM_Format == AUD_DRV_PCM_FMT_PCM_DVDV)
    {
        vWriteShmUINT8(B_LPCM_DEC_TYPE, 0x05);
    }
    else if (ptPcmSetting->tPcmInfo.ePCM_Format == AUD_DRV_PCM_FMT_PCM_DVDV_2CH)
    {
        vWriteShmUINT8(B_LPCM_DEC_TYPE, 0x45);
    }
    else
    {
        vWriteShmUINT8(B_LPCM_DEC_TYPE, ptPcmSetting->u1DecType);
    }

    // Identifiy this is BD or LPE
    vWriteShmUINT8(B_BDLPCM_ON, ptPcmSetting->u1IsBD);
    DspSetSpdifInFlag(0);
}

//end modify by mtk40292 for set ape and flac info

void vAudCodecSet_SACD_Input_Info(AUD_DEC_AUD_INFO_T *tInputInfo)
{
    u8 u1ChnNum = 0;
    switch(tInputInfo->e_aud_type)
    {
    case AUD_DEC_TYPE_STEREO: // 4
        u1ChnNum = 2;
        break;
    case AUD_DEC_TYPE_5_0:  //12
        u1ChnNum = 5;
        break;
    case AUD_DEC_TYPE_5_1: //13
        u1ChnNum = 6;
        break;
    default:
        VERIFY(0);
        break;
    }
    vWriteDspShmBYTE(B_SACD_IP_CHNUM, u1ChnNum);
    vWriteDspShmBYTE(B_SACD_IP_DSD_MODE, tInputInfo->u_fmt_spec.pt_sacd_info->eDsdMode);
    vWriteDspShmBYTE(B_SACD_TIMECODE_EN, tInputInfo->u_fmt_spec.pt_sacd_info->fgTimeCodeEn);
}

#if CONFIG_DRV_AUDIO_IN
void vAudCodecSet_CDDA_Info(AUD_DEC_AUD_INFO_T *ptDecInfo)
{
    u32 u4flag;
    u16 wDspData;
    //Check if Audio in , if yes , set sampling rate for CDDA decoder
    wDspData = u2ReadDspShmWORD(W_HDCDCONFIG);
    wDspData&=0xFF0F;
    DspGetSpdifInFlag(&u4flag);
    if(((u4flag >>8)&0x1) == 0x1)
    {
        // W_HDCDCONFIG : bit 4~7 : sampling frequency
        //  44.1k 48k 88.2k 96k 176.4k 192k 32k 64k
        //   0      1     2    3    4    5    6  7
        switch(ptDecInfo->ui4_sample_rate)
        {
        case 48000:
            wDspData |= 0x10;
            break;
        case 96000:
            wDspData |= 0x30;
            break;
        case 192000:
            wDspData |= 0x50;
            break;
        case 88200:
            wDspData |= 0x20;
            break;
        case 176400:
            wDspData |= 0x40;
            break;
        case 32000:
            wDspData |= 0x60;
            break;
        case 64000:
            wDspData |= 0x70;
            break;
        case 128000:
            wDspData |= 0x80;
        default: //44.1k for default
            break;
        }
    }
    vWriteShmUINT16(W_HDCDCONFIG, wDspData);
}

void vAudInSetEmphasisFlag(bool fgEmphasis)
{
    DspSetEmphasisFlag(((u32) fgEmphasis) <<8); 
    vSendADSPCmd(UOP_DSP_CDDA_DEEMPH);
}

#endif

void vAudCodecSet_DLNA_Info(void)
{
    vWriteDspShmBYTE(B_LPCM_DLNA_FLAG, 0x01);
    LOG(LOG_FEATURE, TEXT("[AUD_MW]DLNA Set info!"));
}

void vAudCodecDLNAInit(void)
{
    vWriteDspShmBYTE(B_LPCM_DLNA_FLAG, 0x00);
    LOG(LOG_FEATURE, TEXT("[AUD_MW]DLNA Init!"));
}

void AudBackupDecInfo(u8 u1DecId,AUD_DRV_AUD_INFO_T* prInfo)
{
    VERIFY(u1DecId <= TER_DEC);
    
    x_memcpy(&g_rAudDrvUpdatedAudInfo[u1DecId], prInfo, sizeof(AUD_DRV_AUD_INFO_T));
    
    LOG(LOG_FEATURE, TEXT("[AUD_MW]AudInf: AudioCodec %d , Channel %d , SampleRate %d ,PID %d.\n"),
        prInfo->e_aud_fmt,
        prInfo->e_aud_type,
        prInfo->ui4_sample_rate,
        prInfo->ui2_pid);

    if (NULL != prInfo->ptPCM_Info)
    {
        LOG(LOG_FEATURE, TEXT("[AUD_MW]BitDepth %d , PCM Format %d , BlockAlign %d .\n"),
        prInfo->ui1_bit_depth,
        prInfo->ptPCM_Info->ePCM_Format,
        prInfo->ptPCM_Info->u2BlockAlign);
    }
}

/***************************************************************************
* Function : i4AudSetPlaySpeed
* Description : Set playback speed
* Parameter :
* Return    : None
* Note      : None.
***************************************************************************/
s32 i4AudSetPlaySpeed(u8 u1DecId, AUD_DEC_PB_SPEED_TYPE_T rSpeed)
{
    s32 i4Ret = AUD_OK;
    u8 u1Speed;
    u16 u2ShmIndex = W_SPEED;

    switch(rSpeed)
    {
    case AUD_DEC_SPEED_TYPE_NORMAL:
        u1Speed = AUD_PB_SPEED_NORMAL;
        break;
    case AUD_DEC_SPEED_TYPE_FF_02_00X:
        u1Speed = AUD_PB_SPEED_FF_02X;
        break;
    case AUD_DEC_SPEED_TYPE_FF_04_00X:
        u1Speed = AUD_PB_SPEED_FF_04X;
        break;
    case AUD_DEC_SPEED_TYPE_FF_08_00X:
        u1Speed = AUD_PB_SPEED_FF_08X;
        break;
    case AUD_DEC_SPEED_TYPE_FF_16_00X:
        u1Speed = AUD_PB_SPEED_FF_16X;
        break;
    case AUD_DEC_SPEED_TYPE_FF_32_00X:
        u1Speed = AUD_PB_SPEED_FF_32X;
        break;
    default:
        u1Speed = AUD_PB_SPEED_NORMAL;
        break;
    }

    vWriteShmUINT16(u2ShmIndex, u1Speed);

    return i4Ret;
}

