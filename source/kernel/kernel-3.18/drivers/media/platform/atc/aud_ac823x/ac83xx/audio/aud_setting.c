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
//#include "x_typedef.h"
#include <linux/types.h>
#include "x_assert.h"
#include "DspDrvInc.h"
#include "DspFunc.h"
#include "aud_drv.h"
#include "aud_config.h"
#include "aud_esm.h"
#include "aud_if.h"
#include "aud_debug.h"

extern AUD_ESM_CONTEXT_T g_rAudEsmContext[];


void vAudGetOutputVol(AUD_OUTPUT_VOL *prChVol)
{  
    DspCfgGetOutputVal(prChVol);

    LOG(LOG_FEATURE, TEXT("=== Get all ch output vol === \n"));
    
    LOG(LOG_FEATURE, TEXT("Front L:   0x%x \n"), prChVol->u4ChVolFrontL);
    LOG(LOG_FEATURE, TEXT("Front R:   0x%x \n"), prChVol->u4ChVolFrontR);
    LOG(LOG_FEATURE, TEXT("Front Ls:  0x%x \n"), prChVol->u4ChVolFrontLs);
    LOG(LOG_FEATURE, TEXT("Front Rs:  0x%x \n"), prChVol->u4ChVolFrontRs);
    LOG(LOG_FEATURE, TEXT("Front C:   0x%x \n"), prChVol->u4ChVolFrontC);
    LOG(LOG_FEATURE, TEXT("Front Sub: 0x%x \n"), prChVol->u4ChVolFrontSub);

    LOG(LOG_FEATURE, TEXT("Gps Mix:   0x%x \n"), prChVol->u4ChVolGpsMix);

    LOG(LOG_FEATURE, TEXT("Rear L:    0x%x \n"), prChVol->u4ChVolRearL);
    LOG(LOG_FEATURE, TEXT("Rear R:    0x%x \n"), prChVol->u4ChVolRearR);

    LOG(LOG_FEATURE, TEXT("Bypass L:  0x%x \n"), prChVol->u4ChVolBypassL);
    LOG(LOG_FEATURE, TEXT("Bypass R:  0x%x \n"), prChVol->u4ChVolBypassR);

    LOG(LOG_FEATURE, TEXT("============================= \n"));
}

void vAudLRMixing(AUD_DEC_LRMIX_OUTPUT_T eMode)
{
    if(true == DspCfgSetLrMix(eMode))
    {
        vDspCmd(UOP_DSP_LR_MIX_RATIO);
    }
	
    vSendADSPCmd(UOP_DSP_KARAOKE_FLAG);
}

void AudSetDetectVolThr(AUD_THRESHOLD_T rAudThrshld)
{
    LOG(LOG_OTHER, TEXT("[Vol_Detect]Not support vol detect!\n"));
}

void vAudSetDec4Info(AUD_DEC4_INFO_T* prInfo)
{
    g_rAudEsmContext[SEC_DEC].eType = prInfo->e_aud_fmt;

    LOG(LOG_FEATURE, TEXT("audio decoder info: fmt = %d, bitdepth = %d, endian =%d, sample =%d, channel cnt =%d.\n"),
		prInfo->e_aud_fmt, prInfo->t_aud_a2dp_info.eBitDepth, prInfo->t_aud_a2dp_info.eDataEndian,
		prInfo->t_aud_a2dp_info.u4SmpRate, prInfo->t_aud_a2dp_info.u4channel_cnt);
    if (false == DspDecSetDec4Info(prInfo))
    {
        LOG(LOG_FAIL, TEXT("vAudSetDec4Info set fail.\n"));
    }  
}


void AudSetDecSrcInfo(u8 u1Dec, AUD_DEC_AUD_INFO_T* prInfo)
{
    VERIFY(u1Dec <= TER_DEC);
    LOG(LOG_CTRLF, _T("Set dec %d source info.\n"), u1Dec);
    LOG(LOG_FEATURE, TEXT("audio decoder info: fmt = %d, bitdepth = %d, sample =%d, channel cnt =%d.\n"),
                          prInfo->e_aud_fmt, prInfo->ui1_bit_depth, prInfo->ui4_sample_rate,prInfo->e_aud_type);

    DspDecSetDec5Info(u1Dec, prInfo);
}

void vAudTestToneSetType(AUD_DEC_TEST_TONE_TYPE_T eType,AUD_DEC_TESTTONE_OUT eTTOut)
{
    u32 u4FlagFront;
    u32 u4FlagRear;
    DspGetTestToneFlag(&u4FlagFront, &u4FlagRear);
    LOG(LOG_FEATURE, TEXT("[AUDTT][vAudTestToneSetType] eType  0x%x\n"), eType);
    LOG(LOG_FEATURE, TEXT("[AUDTT][vAudTestToneSetType] eTTOut 0x%x\n"), eTTOut);

// front type set
    if(AUD_DEC_TESTTONE_FRONT == eTTOut)
    {
        if (u4FlagFront & 0x100)
        {
            LOG(LOG_FEATURE, TEXT("[AUDTT] TestTone Front is already Truned on ! Not allowed Set Again.\n"));
        }
        LOG(LOG_FEATURE, TEXT("[AUDTT] TestTone Front type set to 0x%x\n"), eType);
        switch (eType)
        {
            case AUD_DEC_TEST_TONE_PINK_NOISE:
                u4FlagFront |= 0x100 << 4;
                break;

            case AUD_DEC_TEST_TONE_TRIANGLE_WAVE:
                u4FlagFront |= 0x100 << 5;
                break;

            case AUD_DEC_TEST_TONE_SINE_WAVE:
                u4FlagFront |= 0x100 << 6;
                break;

            case AUD_DEC_TEST_TONE_WHITE_NOISE:
                u4FlagFront |= 0x100 << 7;
                break;

            case AUD_DEC_TEST_TONE_PINK_NOISE_DOLBY:
                u4FlagFront |= 0x100 << 9;
                break;

            default:
                LOG(LOG_FEATURE, TEXT("[AUDTT] TestTone Not This Type\n"));
                  break;
        }

         LOG(LOG_FEATURE, TEXT("[AUDTT] [vAudTestToneSetType]TestTone Front Type Setted 0x%x\n"), u4FlagFront);
         DspCfgSetTestToneFlag(0, u4FlagFront);

    }
//rear type set
    else
    {
        if (u4FlagRear & 0x100)
        {
            LOG(LOG_FEATURE, TEXT("[AUDTT] TestTone Rear is already Truned on ! Not allowed Set Again.\n"));
        }
        LOG(LOG_FEATURE, TEXT("[AUDTT] TestTone Rear Type set to 0x%x\n"), eType);
        switch (eType)
        {
            case AUD_DEC_TEST_TONE_PINK_NOISE:
                u4FlagRear |= 0x100 << 4;
                break;

            case AUD_DEC_TEST_TONE_TRIANGLE_WAVE:
                u4FlagRear |= 0x100 << 5;
                break;

            case AUD_DEC_TEST_TONE_SINE_WAVE:
                u4FlagRear |= 0x100 << 6;
                break;

            case AUD_DEC_TEST_TONE_WHITE_NOISE:
                u4FlagRear |= 0x100 << 7;
                break;

            case AUD_DEC_TEST_TONE_PINK_NOISE_DOLBY:
                u4FlagRear|= 0x100 << 9;
                break;

            default:
                LOG(LOG_FEATURE, TEXT("[AUDTT] TestTone Not This Type\n"));
                  break;

        }
        LOG(LOG_FEATURE, TEXT("[AUDTT] [vAudTestToneSetType]TestTone Rear Type Setted 0x%x\n"), u4FlagRear);
        DspCfgSetTestToneFlag(1, u4FlagRear);
    }

}



void vAudTestToneSetChannel(AUD_DEC_LS_T eChannel,AUD_DEC_TESTTONE_OUT eTTOut)
{
    u32 u4FlagFront;
    u32 u4FlagRear;

    u32 u4ChSet = 0;
    u32 u4SpkCfg, u4SpkCfg2;
    u32 u4SpkCfgBit012;
    bool   fgChExist = FALSE;


    AUDIO_TT_FRONT_SWITCH_UNION_T uTTFrontSet;
    AUDIO_TT_REAR_SWITCH_UNION_T uTTRearSet;

    uTTFrontSet.dwWordFront = 0;
    uTTRearSet.dwWordRear = 0;
	
    DspGetTestToneFlag(&u4FlagFront, &u4FlagRear);

    LOG(LOG_FEATURE, TEXT("[AUDTT][vAudTestToneSetChannel] eChannel    0x%x\n"), eChannel);
    LOG(LOG_FEATURE, TEXT("[AUDTT][vAudTestToneSetChannel] eTTOut 0x%x\n"), eTTOut);


    if(AUD_DEC_TESTTONE_FRONT == eTTOut)
    {
        if(0 == (u4FlagFront & 0x100))
        {
            LOG(LOG_FEATURE, TEXT("[AUD] TestTone Front Not be enabled return.\n"));
            return;
        }
        DspGetSpeakerConfig(&u4SpkCfg, &u4SpkCfg2);
        u4SpkCfgBit012 = u4SpkCfg & 0x7;

        switch(eChannel)
        {
            //front LR
            case AUD_DEC_LS_FRONT_LEFT:
            case AUD_DEC_LS_FRONT_RIGHT:
            {
                fgChExist = TRUE;
                uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_L_EN = 1;
                uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_R_EN = 1;
                uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_L_SEL_SIG2 = 0;
                uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_R_SEL_SIG2 = 0;

                u4ChSet = uTTFrontSet.dwWordFront;
                LOG(LOG_FEATURE, TEXT("[AUDTT] TestTone Front LR Channel. Channel[%x]\n"), u4ChSet);
            }
            break;

            case AUD_DEC_LS_REAR_LEFT:
            case AUD_DEC_LS_REAR_RIGHT:
            {
                if ((6 == u4SpkCfgBit012) || (7 == u4SpkCfgBit012))
                {
                    fgChExist = TRUE;
                    uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_LS_EN = 1;
                    uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_RS_EN = 1;
                    uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_LS_SEL_SIG2 = 0;
                    uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_RS_SEL_SIG2 = 0;
                    u4ChSet = uTTFrontSet.dwWordFront;
                    LOG(LOG_FEATURE, TEXT("[AUDTT] TestTone LsRs Channel. Channel[%x]\n"), u4ChSet);
                }
            }
            break;

            case AUD_DEC_LS_CENTER:
            {
                if ((3 == u4SpkCfgBit012) || (5 == u4SpkCfgBit012) || (7 == u4SpkCfgBit012))
                {
                    fgChExist = TRUE;
                    uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_C_EN = 1;
                    uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_C_SEL_SIG2 = 0;
                    u4ChSet = uTTFrontSet.dwWordFront;
                    LOG(LOG_FEATURE, TEXT("[AUDTT] TestTone Center Channel. Channel[%x]\n"), u4ChSet);
                }
            }
            break;

            case AUD_DEC_LS_SUB_WOOFER:
            {
                if(u4SpkCfg & (0x1 << 5))
                {
                    fgChExist = TRUE;
                    LOG(LOG_FEATURE, TEXT("[AUDTT] TestTone Subwoofer Not Support on MainBranch\n"));
                }

            }
            break;

            case AUD_DEC_LS_SPK_AUX1:
            {
                if(u4SpkCfg & (0x1 << 3))
                {
                    fgChExist = TRUE;
                    LOG(LOG_FEATURE, TEXT("[AUDTT] TestTone AE Not Support Ch7 on MainBranch\n"));
                }
            }
            break;
            case AUD_DEC_LS_SPK_AUX2:
            {
                if(u4SpkCfg & (0x1 << 4))
                {
                    fgChExist = TRUE;
                    LOG(LOG_FEATURE, TEXT("[AUDTT] TestTone AE Not Support Ch8 on MainBranch\n"));
                }
            }
            break;
            case AUD_DEC_LS_SPK_DOWNMIX_LEFT:
            {
                LOG(LOG_FEATURE, TEXT("[AUDTT] TestTone AE Not Support Ch9 on MainBranch\n"));
            }
            break;

            case AUD_DEC_LS_SPK_DOWNMIX_RIGHT:
            {
                LOG(LOG_FEATURE, TEXT("[AUDTT] TestTone AE Not Support Ch10 on MainBranch\n"));
            }
            break;

            case AUD_DEC_LS_SPK_ALL:
            {
                fgChExist = TRUE;


                uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_L_EN = 1;
                uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_R_EN = 1;
                uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_C_EN = 1;
                uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_LS_EN = 1;
                uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_RS_EN = 1;
                uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_SW_EN = 1;

                uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_L_SEL_SIG2 = 0;
                uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_R_SEL_SIG2 = 0;
                uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_C_SEL_SIG2 = 0;
                uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_LS_SEL_SIG2 = 0;
                uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_RS_SEL_SIG2 = 0;
                uTTFrontSet.ttBitFront.AE_TT_ANALOG_FRONT_SW_SEL_SIG2 = 0;

                u4ChSet = uTTFrontSet.dwWordFront;

                LOG(LOG_FEATURE, TEXT("[AUDTT] TestTone all spk. u4Channel[%x]\n"), u4ChSet);
            }
            break;

            default:
                LOG(LOG_FEATURE, TEXT("[AUDTT]None this Channel Cfg,please Check\n"));
             break;



        }
        if(!fgChExist)
        {
            LOG(LOG_FEATURE, TEXT("[AUDTT] TestTone channel 0x%x is invalid for current speaker setting.\n"), eChannel);
        }

        DspCfgSetTestToneCh(0, u4ChSet);

        LOG(LOG_FEATURE, TEXT("[AUDTT] ShareInfo Value:0x%x\n"), u4ReadDspShmDWRD(D_TEST_TONE_CHANNEL_ANALOG_FRONT));
        vSendADSPCmd(UOP_DSP_TEST_TONE_SET_FRONT_OPTION);
    }
    else
    {

        //u4ChSet = u4ChRear;
        uTTRearSet.ttBitRear.AE_TT_ANALOG_REAR_L_EN = 1;
        uTTRearSet.ttBitRear.AE_TT_ANALOG_REAR_R_EN = 1;
        uTTRearSet.ttBitRear.AE_TT_ANALOG_REAR_L_SEL_SIG2 = 0;
        uTTRearSet.ttBitRear.AE_TT_ANALOG_REAR_R_SEL_SIG2 = 0;

        u4ChSet = uTTRearSet.dwWordRear;

         DspCfgSetTestToneCh(1, u4ChSet);
         vSendADSPCmd(UOP_DSP_TEST_TONE_SET_REAR_OPTION);
    }

}


void vAudTestToneSwitch(AUD_DEC_TESTTONE_ONOFF fgTTONOFF,AUD_DEC_TESTTONE_OUT eTTOut)
{
    u32 u4FlagFront ;
    u32 u4FlagRear;
    AUD_DRV_STATE_T eDrvState = AudDrvGetState(PRI_DEC);
    DspGetTestToneFlag(&u4FlagFront, &u4FlagRear);

    LOG(LOG_FEATURE, TEXT("[AUDTT][vAudTestToneSwitch] fgTTONOFF  0x%x\n"), fgTTONOFF);
    LOG(LOG_FEATURE, TEXT("[AUDTT][vAudTestToneSwitch] eTTOut 0x%x\n"), eTTOut);
    //enable
    if (fgTTONOFF == AUD_DEC_TESTTONE_ENABLE)
    {
    //front
        if (AUD_DEC_TESTTONE_FRONT == eTTOut)
        {
            if (0 == (u4FlagFront & 0x100))
            {
                if (AUD_DRV_STOPPED != eDrvState)
                {
                    LOG(LOG_FEATURE, TEXT("[AUDTT]Front must on full stop state.\n"));
                    return;
                }

                #if CONFIG_DRV_SPDIF_RAW_SUPPORT
                AudCfg_SpdifEnable(AUD_AOUT1);
                #endif

                LOG(LOG_FEATURE, TEXT("[AUDTT] TestTone Front enabled.\n"));
                u4FlagFront |= 0x300;
				
                DspCfgSetTestToneFlag(0, u4FlagFront);
                vSendADSPCmd(UOP_DSP_TEST_TONE_FRONT_CONFIG);
            }

        }


    //rear
        else
        {

            if (0 == (u4FlagRear & 0x100))
            {
                if (AUD_DRV_STOPPED != eDrvState)
                {
                    LOG(LOG_FEATURE, TEXT("[AUDTT]Rear must on full stop state.\n"));
                    return;
                }
                LOG(LOG_FEATURE, TEXT("[AUD] TestTone Rear enabled.\n"));
                u4FlagRear |= 0x300;
                DspCfgSetTestToneFlag(1, u4FlagRear);
                vSendADSPCmd(UOP_DSP_TEST_TONE_REAR_CONFIG);
            }
        }
    }


//diable
    else
    {
            if(u4FlagFront & 0x100)
            {
                LOG(LOG_FEATURE, TEXT("[AUD] TestTone Front disabled.\n"));
                DspCfgSetTestToneFlag(0, 0);
                vSendADSPCmd(UOP_DSP_TEST_TONE_FRONT_CONFIG);
            }

            else if(u4FlagRear & 0x100)
            {
                LOG(LOG_FEATURE, TEXT("[AUD] TestTone Rear disabled.\n"));
                DspCfgSetTestToneFlag(1, 0);
                vSendADSPCmd(UOP_DSP_TEST_TONE_REAR_CONFIG);
            }

        }
}
   

