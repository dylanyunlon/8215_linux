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



#define _DSP_UOP_C
/*-----------------------------------------------------------------------------
Include header files
-----------------------------------------------------------------------------*/
#include "chip_ver.h"
#include "DspConst.h"
#include "DspFunc.h"
#include "DspVar.h"
#include "DspUop.h"
#include "DspD2RCInt.h"
#include "DspShm.h"
#include "aud_debug.h"

#include "AsvAudDrv.h"
#include "aud_se.h"         // (aud_se_v2)

#include "drv_config.h"
#include "aud_drv_config.h"

/*-----------------------------------------------------------------------------
Function declarations
-----------------------------------------------------------------------------*/
extern void DspSet3DUOP(u16 wDspUop);
extern void DspSetEqUop(u16 wDspUop);
extern void DspSetChConfigUOP(u16 wDspUop);
extern void DspSetVolumeUOP(u16 wDspUop);
extern void DspSetTestToneUOP(u16 wDspUop);
extern void DspSetCddaUop(u16 wDspUop);
extern void DspSetDTSEncUop (u16 wDspUop);
extern void DspSetReEncUOP (u16 wDspUop);
extern void DspSetKaraokeUOP(u16 wDspUop);
extern void DspSetAC3Uop(u16 wDspUop);
extern void DspSetDTSUop (u16 wDspUop);
extern void DspSetTrueHDUop(u16 wDspUop);
extern void DspSetAACUop (u16 wDspUop);
extern void vReencFlowControlUOP(u32 u4DspUopMsg);
extern void DspSetDecoder4UOP(u32 u4Dec4Uop);
extern void DspSetMixerConnectUOP(u32 u4DspUopMsg);
extern void DspFlowControlUOP(u32 u4DspUopMsg);

extern void DspGetProcessingMode(u16* pu2Data);
extern void vDspVorbisUop (u16 u2DspUop);

static void vGrp5UOP (u16 wDspUop);
static void vDecUOP (u16 wDspUop);
void vDspIECConfig (bool fgFirstDecoder);


/***************************************************************************
Function : vDecUOP
Description : Decoding related User Operation
Parameter : None
Return    : None
***************************************************************************/
void vDecUOP(u16 wDspUop)
{
    u8 u1DecType;
    DspGetDec1StrType(&u1DecType);
    g_fgDspBSInt = TRUE;
    switch (u1DecType)
    {
    case AC3_STREAM:
    case LOSSLESS_AC3_STREAM:
        DspSetAC3Uop(wDspUop);
        break;
    case DTSCD_STREAM:
    case DTSDVD_STREAM:
    case DTSMA_STREAM:
        DspSetDTSUop(wDspUop);
        break;
    case CDDA_STREAM:
    case CDDA24_STREAM:
        DspSetCddaUop(wDspUop);
        break;
    case TRUE_HD_STREAM:
        DspSetTrueHDUop(wDspUop);
        break;
        // AAC_support_DSP
    case AAC_STREAM:
    case AAC_PURE_STREAM:
        DspSetAACUop(wDspUop);
        break;
    case VORBIS_STREAM:
        vDspVorbisUop(wDspUop);
        break;
    default:
        g_fgDspBSInt = FALSE;
        break;
    }
}

/***************************************************************************
Function : vGrp5UOP
Description : None
Parameter : None
Return    : None
***************************************************************************/
void vGrp5UOP(u16 wDspUop)
{
    u16 u2DspData;
    AUD_SOURCE_CFG_T* prSrcCfg = NULL;

    switch (wDspUop)
    {
        // first decoder
    case UOP_DSP_IEC_FLAG:
        vDspIECConfig(TRUE);
        //vDspCmd(UOP_DSP_AOUT_REINIT);
        break;
    case UOP_DSP_SPEED:
        g_fgDspBSInt = TRUE;
        g_u4DspBSIntAddr = INT_RC2D_SPEED;
        g_u4DspBSIntSD = ((u32) u2ReadDspShmWORD(W_SPEED)) << 8;
        break;

    case UOP_DSP_PROCESSING_MODE:
        // This uop can not set the 3th nibble ( bypass group)
        g_fgDspSInt = TRUE;
        g_u4DspSIntAddr = INT_RC2D_PROCESSING_MODE;
        DspGetProcessingMode(&u2DspData);
        g_u4DspSIntSD = ((u32) u2DspData) << 8;
        // Master volume depends on drc -> redo it;
        vDspCmd (UOP_DSP_MASTER_VOLUME);
        break;
    case UOP_DSP_IEC_DOWN_SAMPLE:
        prSrcCfg = DspGetSrcParam();
        vDspSetFreq(prSrcCfg->u1Aud_Sampling_Rate, FALSE, TRUE);
        break;

        // second decoder
    case UOP_DSP_IEC_FLAG2:
        vDspIECConfig(FALSE);
        break;

    default:
        g_fgDspSInt = FALSE;
    }
}

/***************************************************************************
Function : vMixSoundUOP
Description : None
Parameter : None
Return    : None
***************************************************************************/
void vMixSoundUOP(u16 wDspUop)
{
    switch (wDspUop)
    {
    case UOP_DSP_BDJ_MIXING_P1_P2_CHANGE:
        vDspMetadataToMixingPara();
        break;

    default:
        g_fgDspSInt = FALSE;
        break;
    }
}

/***************************************************************************
Function : vDSPUOPSvc
Description : DSP User Operation Service
Parameter : None
Return    : None
***************************************************************************/
void vDspUopSvc(u32 u4DspUop)
{
    g_u4DspSIntLD = 0;
    switch ((u4DspUop & 0xFF))
    {
        //Volume
    case DSP_UOPID1:
        DspSetVolumeUOP(u4DspUop);
        break;
        //Microphone --> MixSound
    case DSP_UOPID2:
        vMixSoundUOP(u4DspUop);
        break;
        //Flow Control
    case DSP_UOPID3:
        LOG(LOG_CTRLF, TEXT("DspCtrl FlowControl UOP. DecId %d, Msg0x%x.\n"),
            u4DspUop>>16, u4DspUop&0xFFFF);
        DspFlowControlUOP(u4DspUop);
        break;
        //Channel Configuration
    case DSP_UOPID4:
        DspSetChConfigUOP(u4DspUop);
        break;
        //IEC, PTS, STC and Speed
    case DSP_UOPID5:
        vGrp5UOP(u4DspUop);
        break;
        //Pink Noise
    case DSP_UOPID6:
        DspSetTestToneUOP(u4DspUop);
        break;
        //Karaoke
    case DSP_UOPID7:
        DspSetKaraokeUOP(u4DspUop);
        break;
        //Equalizer
    case DSP_UOPID8:
        DspSetEqUop(u4DspUop);
        break;
        // 3D Processing
    case DSP_UOPID9:
        DspSet3DUOP(u4DspUop);
        break;
        //Decoding Related
    case DSP_UOPIDA:
    case DSP_UOPIDB:
        vDecUOP(u4DspUop);
        break;
    case DSP_UOPIDC:
        LOG(5, TEXT("[AUD]DspCtrl processs Dec4Uop DspUop 0x%x.\n"), u4DspUop);
        DspSetDecoder4UOP(u4DspUop);
        break;
    case DSP_UOPIDD:
        vReencFlowControlUOP(u4DspUop);
        break;
    case DSP_UOPIDE:
        DspSetReEncUOP(u4DspUop);
        break;
    case DSP_UOPIDF:
        //vTransCodeUOP(u4DspUop);
        break;
    case DSP_UOPID10:
        LOG(LOG_CTRLF, TEXT("DspCtrl process MIXER_CONNECT UOP u4DspUop 0x%x.\n"), u4DspUop);
        DspSetMixerConnectUOP(u4DspUop);
        break;
#if CONFIG_AUD_SE_V2_EN
    case DSP_UOPID1B:
        vAudSeProcessUOP(u4DspUop);     // (aud_se_v2)
        break;
#endif
    case DSP_UOPID1D:
        //DspSetMicUOP(u4DspUop);
        break;
    case DSP_UOP_INT:
        vDspIntSvc(u4DspUop);
        break;

    default:
        break;
    }
}

/***************************************************************************
Function : vDspIECConfig
Description : Routine handling IEC Configuration
Parameter : None
Return    : None
***************************************************************************/
void vDspIECConfig(bool fgFirstDecoder)
{
    u8 u1IecMute;
    u8 u1IecMode;
    AUD_SOURCE_CFG_T* prSrcParam = DspGetSrcParam();

    if(fgFirstDecoder)
    {
        //Retreive info from share memory
        u1IecMode= uReadDspShmBYTE(B_IECFLAG);
        u1IecMute= uReadDspShmBYTE(B_IEC_MUTE);

        g_fgIECRAWOff = u1IecMute;

        if (u1IecMute)
        {
            prSrcParam->rAud_Ui_Setting.eAud_Iec_Ui_Select = AUD_DIGITAL_OFF;
            LOG(6, TEXT("Dsp IEC Config UOP set AUD_DIGITAL_OFF\n"));
        }
        else
        {
            switch(u1IecMode)
            {
            case AUD_IEC_CFG_PCM:
                prSrcParam->rAud_Ui_Setting.eAud_Iec_Ui_Select = AUD_DIGITAL_PCM;
                LOG(6, TEXT("Dsp IEC Config UOP set AUD_DIGITAL_PCM\n"));
                break;
            case AUD_IEC_CFG_RAW:
                prSrcParam->rAud_Ui_Setting.eAud_Iec_Ui_Select = AUD_DIGITAL_RAW;
                LOG(6, TEXT("Dsp IEC Config UOP set AUD_DIGITAL_RAW\n"));
                break;
            case AUD_IEC_CFG_RAW_HD:
                prSrcParam->rAud_Ui_Setting.eAud_Iec_Ui_Select = AUD_DIGITAL_RAW;
                LOG(6, TEXT("Dsp IEC Config UOP set AUD_DIGITAL_RAW\n"));
                break;
            case AUD_IEC_CFG_RAW_REENCODE:
                prSrcParam->rAud_Ui_Setting.eAud_Iec_Ui_Select = AUD_DIGITAL_REENCODE;
                LOG(6, TEXT("Dsp IEC Config UOP set AUD_IEC_CFG_RAW_REENCODE\n"));
                break;
            default:
                prSrcParam->rAud_Ui_Setting.eAud_Iec_Ui_Select = AUD_DIGITAL_PCM;
                LOG(6, TEXT("Dsp IEC Config UOP set default : AUD_DIGITAL_PCM\n"));
                break;
            }
        }
    }
    else
    {
        //Retreive info from share memory
        u1IecMode= uReadDspShmBYTE(B_IECFLAG2);
        u1IecMute= uReadDspShmBYTE(B_IEC2_MUTE);

        if (u1IecMute)
        {
            prSrcParam->rAud_Ui_Setting.eAud_Hdmi_Ui_Select= AUD_DIGITAL_OFF;
            LOG(6, TEXT("Dsp IEC2 Config UOP set AUD_DIGITAL_OFF\n"));
        }
        else
        {
            switch(u1IecMode)
            {
            case AUD_IEC_CFG_PCM:
                prSrcParam->rAud_Ui_Setting.eAud_Hdmi_Ui_Select = AUD_DIGITAL_PCM;
                LOG(6, TEXT("Dsp IEC2 Config UOP set AUD_DIGITAL_PCM\n"));
                break;
            case AUD_IEC_CFG_RAW:
                prSrcParam->rAud_Ui_Setting.eAud_Hdmi_Ui_Select = AUD_DIGITAL_RAW;
                LOG(6, TEXT("Dsp IEC2 Config UOP set AUD_DIGITAL_RAW\n"));
                break;
            case AUD_IEC_CFG_RAW_HD:
                prSrcParam->rAud_Ui_Setting.eAud_Hdmi_Ui_Select = AUD_DIGITAL_RAW;
                LOG(6, TEXT("Dsp IEC2 Config UOP set AUD_DIGITAL_RAW\n"));
                break;
            case AUD_IEC_CFG_RAW_REENCODE:
                prSrcParam->rAud_Ui_Setting.eAud_Hdmi_Ui_Select = AUD_DIGITAL_REENCODE;
                LOG(6, TEXT("Dsp IEC2 Config UOP set AUD_DIGITAL_REENCODE\n"));
                break;
            case AUD_IEC_CFG_PCM_STEREO:
                prSrcParam->rAud_Ui_Setting.eAud_Hdmi_Ui_Select = AUD_DIGITAL_PCM_STEREO;
                LOG(6, TEXT("Dsp IEC2 Config UOP set AUD_DIGITAL_PCM_STEREO\n"));
                break;
            default:
                prSrcParam->rAud_Ui_Setting.eAud_Hdmi_Ui_Select = AUD_DIGITAL_PCM;
                LOG(6, TEXT("Dsp IEC Config UOP set default : AUD_DIGITAL_PCM\n"));
                break;
            }
        }

    }
    vDspSetFreq(prSrcParam->u1Aud_Sampling_Rate, FALSE, fgFirstDecoder);
}



