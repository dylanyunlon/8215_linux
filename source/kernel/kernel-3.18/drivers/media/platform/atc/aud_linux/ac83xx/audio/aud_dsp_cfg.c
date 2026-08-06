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
#include "x_bim.h"
#include "x_assert.h"

#include "aud_debug.h"
#include "aud_dsp_cfg.h"
#include "DspDrvInc.h"
#include "DspFunc.h"
#include "aud_drv_config.h"
#include "aud_drv.h"
#include "aud_config.h"
#include "drv_dsp_cfg.h"

/*************************************************************************
** Constant definitions
*************************************************************************/
#define AUD_CH_NUM         11
#define AUD_DEC_NUM  ((u8)2)

#define MAX_VOL_LEVEL   (100)
#define DRC_RANGE_MAX   9
#define TRIM_LEVEL_MAX  41

/*************************************************************************
** Local prototypes
*************************************************************************/
struct semaphore g_aud_media_lock;
extern SPDIF_RAW_SET_INFO_T g_rSpdifRawInfo;

/****************************************************************************
** Function prototypes
****************************************************************************/

/**************************************************************************
** Local variable
**************************************************************************/
static const u32 g_u4Ac3DrcRange[DRC_RANGE_MAX] =
{
    0x00000000,0x000FFFFF,0x001FFFFF,0x002FFFFF,0x003FFFFF,0x004FFFFF,0x005FFFFF,
    0x006FFFFF,0x007FFFFF
};

static const u32 g_u4TrimValue[TRIM_LEVEL_MAX] =
{
    0x0000A1E9,0x0000AB81,0x0000B5AA,0x0000C06E,0x0000CBD5,0x0000D7E9,0x0000E4B4,0x0000F241,
    0x0001009C,0x00010FD0,0x00011FEB,0x000130FB,0x0001430D,0x00015631,0x00016A78,0x00017FF2,
    0x000196B2,0x0001AECB,0x0001C852,0x0001E35C,0x00020000,0x00021E57,0x00023E79,0x00026083,
    0x00028492,0x0002AAC3,0x0002D338,0x0002FE13,0x00032B77,0x00035B8C,0x00038E7B,0x0003C46E,
    0x0003FD93,0x00043A1B,0x00047A3A,0x0004BE25,0x00050616,0x0005524B,0x0005A303,0x0005F884,
    0x00065316
};

static bool   g_fgIecEnable[AUD_DEC_NUM] = {TRUE, TRUE};
static AUD_IEC_CFG_T g_eIecMode[AUD_DEC_NUM] = {AUD_IEC_CFG_PCM, AUD_IEC_CFG_PCM};

u32 g_u4MaxMasterVolumeShmValue = 0x20000;
AUD_DEC_SPEAKER_LAYOUT_T g_rSpkLayout = {0};

/****************************************************************************
** Local functions
****************************************************************************/

/*************************************************************************
 * Function		: vSendADSPCmd
 * Description	: send cmd to adsp
 * Parameter 	: u4Cmd: cmd id
 * Return		: 
**************************************************************************/
void vSendADSPCmd(u32 u4Cmd)
{
    vDspCmd(u4Cmd);
}

/*************************************************************************
 * Function		: AdspVolToShm
 * Description	: transform volume (0~100) to volume gain (0~0x20000)
 * Parameter 	: u1Volume: volume
 * Return		: 
**************************************************************************/
static u32 AdspVolToShm(bool fgMaterVol, u8 u1Volume)
{
    u32 u4VolShm = 0;
    u32 u4MasterVolumeBase = g_u4MaxMasterVolumeShmValue/MAX_VOL_LEVEL;

    if (fgMaterVol == TRUE)
    {
        if (u1Volume == MAX_VOL_LEVEL)
        {
            u4VolShm = g_u4MaxMasterVolumeShmValue;
        }
        else
        {
            u4VolShm = u4MasterVolumeBase * u1Volume;
        }
    }
    else
    {
        if (u1Volume >= TRIM_LEVEL_MAX)
        {
            u4VolShm = g_u4TrimValue[TRIM_LEVEL_MAX - 1];
        }
        else
        {
            u4VolShm = g_u4TrimValue[u1Volume];
        }
    }

    return u4VolShm;
}

/******************************************************************************
* Function      : vAdspAC3DRCRange
* Description   : setup range for dynamix range compression
* Parameter     : uDRCLevel: DRC range,uDecIndx: 0: first decoder 1: seconder decoder
*                 0x00000000-->0.0, 0x007FFFFF-->1.0, 0.0 ~ 1.0 step 0.125
* Return        : None
******************************************************************************/
void vAdspAC3DRCRange(u8 uDRCLevel,u8 u1DecId)
{
    u16 u2DRCLowUop  = UOP_DSP_AC3_DYNAMIC_LOW;
    u16 u2DRCHighUop = UOP_DSP_AC3_DYNAMIC_HIGH;

    if (uDRCLevel >= DRC_RANGE_MAX)
    {
        uDRCLevel = DRC_RANGE_MAX - 1;
    }
    if (u1DecId != PRI_DEC)
    {
        u2DRCLowUop=UOP_DSP_AC3_DYNAMIC_LOW_DEC2;
        u2DRCHighUop=UOP_DSP_AC3_DYNAMIC_HIGH_DEC2;
    }
    DspCfgSetDrcLowRange(u1DecId, g_u4Ac3DrcRange[uDRCLevel]);
    vSendADSPCmd(u2DRCLowUop);
    DspCfgSetDrcHighRange(u1DecId, g_u4Ac3DrcRange[uDRCLevel]);
    vSendADSPCmd(u2DRCHighUop);
}

/*
* D_SPKCFG
* 0: LT/RT                  bit 3: ch7 exist or not
* 1: Mono                   bit 4: ch8 exist or not
* 2: Stereo                 bit 5: subwoofer exist or not
* 3: L/R/C
* 4: L/R/S
* 5: L/R/C/S
* 6. L/R/LS/RS
* 7: L/R/C/LS/RS
*
* bit 12: Center Channel large(1)/small(0)
* bit 13: Left Channel large(1)/small(0)
* bit 14: Right Channel large(1)/small(0)
* bit 15: Left Surround Channel large(1)/small(0)
* bit 16: Right Surround Channel large(1)/small(0)
* bit 17: Center Back Channel large(1)/small(0)
* bit 18: No.7 Channel large(1)/small(0)
*
* D_SPKCFG_2
* represent the channel set
* 0: no remapping is required
* Other config is followed the rule below:
* bit 0: Center exist       bit 8: Overhead
* bit 1: LR                 bit 9: LC/RC
* bit 2: LS/RS              bit 10: LW/RW
* bit 3: LFE                bit 11: LSS/RSS
* bit 4: CS (CB)            bit 12: LFE2
* bit 5: Lh/Rh              bit 13: LHS/RHS
* bit 6: LSR/RSR            bit 14: CHR
* bit 7: Center high        bit 15: LHR/RHR
*/


bool AdspCheckSpkChg(AUD_DEC_SPEAKER_LAYOUT_T rSpkLayout)
{
    bool fgRet = TRUE;
    if((rSpkLayout.ui8_spk_layout == g_rSpkLayout.ui8_spk_layout)&&
       (rSpkLayout.ui2_front_size == g_rSpkLayout.ui2_front_size)&&
       (rSpkLayout.ui2_center_size == g_rSpkLayout.ui2_center_size)&&
       (rSpkLayout.ui2_rear_size == g_rSpkLayout.ui2_rear_size)&&
       (rSpkLayout.ui2_sub_size == g_rSpkLayout.ui2_sub_size)&&
       (rSpkLayout.ui4_sub_force_out == g_rSpkLayout.ui4_sub_force_out))
    {
        fgRet = FALSE;
    }
    else
    {
        g_rSpkLayout.ui8_spk_layout = rSpkLayout.ui8_spk_layout;
        g_rSpkLayout.ui2_front_size = rSpkLayout.ui2_front_size;
        g_rSpkLayout.ui2_center_size = rSpkLayout.ui2_center_size;
        g_rSpkLayout.ui2_rear_size = rSpkLayout.ui2_rear_size;
        g_rSpkLayout.ui2_sub_size = rSpkLayout.ui2_sub_size;
        g_rSpkLayout.ui4_sub_force_out = rSpkLayout.ui4_sub_force_out;
    }

    return (fgRet);
}


void vAdspSetSpeakerConfig(AUD_DEC_SPEAKER_LAYOUT_T rSpeakerLayout)
{    
    if(FALSE == AdspCheckSpkChg(rSpeakerLayout))
    {
        LOG(LOG_CTRLF, TEXT("rSpeakerLayout is same with prev, High = 0x%x, Low = 0x%x.\n"),
            (u32)(((rSpeakerLayout.ui8_spk_layout)&0xFFFFFFFF00000000LL)>>32),(u32)((rSpeakerLayout.ui8_spk_layout)&0xFFFFFFFF));
            return;
    }
	
    DspCfgSetSpkLayout(&rSpeakerLayout);
        
    vSendADSPCmd(UOP_DSP_CONFIG_SPEAKER);
}

void vAdspSetModBManagementInfo(AUD_DEC_MODULE_BMANAGEMENT_CHANNEL_INFO_T eModBManagementInfo)
{
    DspCfgModBInfo(&eModBManagementInfo);
}


bool vAdspGetFrontAoutStatus(void)
{
    AUD_DRV_STATE_T eAudDrvState;
    bool bIsReadyForSwitch;
    AUD_OUT_MEDIA_TYPE_T  eMediaType;
    bIsReadyForSwitch = FALSE;
    eMediaType = uReadDspShmBYTE(B_FRONT_AOUT_MEDIA_TYPE);
    if (eMediaType == AUD_OUT_MEDIA_DVD)
    {
        LOG(LOG_CTRLF, TEXT("Fixed me for DVD Mix\n"));
    }
    else if (eMediaType == AUD_OUT_MEDIA_USB)
    {
        LOG(LOG_DUALCTRL, TEXT("Get Primary Decode Status\n"));
        eAudDrvState = AudDrvGetState(PRI_DEC);
        LOG(LOG_CTRLF, TEXT("[AUD]Current Primary Status is %d\n"),eAudDrvState);
        if ((AUD_DRV_STOPPED == eAudDrvState)|| (AUD_DRV_PAUSED == eAudDrvState))
        {
            bIsReadyForSwitch = TRUE;
        }
    }
    else if (eMediaType == AUD_OUT_MEDIA_LINE_IN)
    {
        LOG(LOG_DUALCTRL, TEXT("Get The Fourth Decode Status\n"));
        eAudDrvState = AudDrvGetState(SEC_DEC);
        LOG(LOG_CTRLF, TEXT("[AUD]Current Fourth Decode Status is %d\n"),eAudDrvState);
        if ((AUD_DRV_STOPPED == eAudDrvState)|| (AUD_DRV_PAUSED == eAudDrvState))
        {
            bIsReadyForSwitch = TRUE;
        }
    }
    else
    {
        bIsReadyForSwitch = TRUE;
    }
    LOG(LOG_CTRLF, TEXT("[AUD]Current Front Media Type %d\n"),eMediaType);
    LOG(LOG_CTRLF, TEXT("[AUD][Front]Switch Now  = %s. \n"),   (bIsReadyForSwitch)?("OK"):("Waiting"));

    return bIsReadyForSwitch;
}

bool vAdspGetRearAoutStatus(void)
{
    AUD_DRV_STATE_T eAudDrvState;
    bool bIsReadyForSwitch;
    AUD_OUT_MEDIA_TYPE_T  eMediaType;
    bIsReadyForSwitch = FALSE;
    eMediaType = uReadDspShmBYTE(B_REAR_AOUT_MEDIA_TYPE);
    if (eMediaType == AUD_OUT_MEDIA_DVD)
    {
        LOG(LOG_CTRLF, TEXT("Fixed me for DVD Mix\n"));
    }
    else if (eMediaType == AUD_OUT_MEDIA_USB)
    {
        LOG(LOG_DUALCTRL, TEXT("Get Primary Decode Status\n"));
        eAudDrvState = AudDrvGetState(PRI_DEC);
        LOG(LOG_CTRLF, TEXT("[AUD]Current Primary Status is %d\n"),eAudDrvState);
        if ((AUD_DRV_STOPPED == eAudDrvState)|| (AUD_DRV_PAUSED == eAudDrvState))
        {
            bIsReadyForSwitch = TRUE;
        }
    }
    else if (eMediaType == AUD_OUT_MEDIA_LINE_IN)
    {
        LOG(LOG_DUALCTRL, TEXT("Add Here For Get The Line In Bypass Status later\n"));
        bIsReadyForSwitch = TRUE;
    }
    else
    {
        bIsReadyForSwitch = TRUE;

    }
    LOG(LOG_CTRLF, TEXT("[AUD]Current Rear Media Type %d\n"),eMediaType);
    LOG(LOG_CTRLF, TEXT("[AUD][Rear]Switch Now  = %s. \n"),   (bIsReadyForSwitch)?("OK"):("Waiting"));

    return bIsReadyForSwitch;
}

u8 u1AdspGetFrontAoutType(void)
{     
    u8 uType = uReadDspShmBYTE(B_FRONT_AOUT_MEDIA_TYPE);
    LOG(LOG_CTRLF, TEXT("Get Front Media Type is %d.\r\n"), uType);

    return uType;
}

u8 u1AdspGetRearAoutType(void)
{
    u8 uType = uReadDspShmBYTE(B_REAR_AOUT_MEDIA_TYPE);
    LOG(LOG_CTRLF, TEXT("Get Rear Media Type is %d.\r\n"), uType);

    return uType;
}

void AdspMediaSemaInit(void)
{
    sema_init(&g_aud_media_lock, 1);
}

bool fgAdspSetFrontAoutMediaType(AUD_OUT_MEDIA_TYPE_T eAOutMediaType)
{
    AUD_OUT_MEDIA_TYPE_T ePrevMediaType;

    down(&g_aud_media_lock);
    ePrevMediaType = uReadDspShmBYTE(B_FRONT_AOUT_MEDIA_TYPE);

    if (eAOutMediaType == ePrevMediaType)
    {
        //Current Mediatype no change , no need action for driver
        LOG(LOG_CTRLF, TEXT("Front Media type is same, No need reset.\n"));
        up(&g_aud_media_lock);
        return (FALSE);
    }

    if(ePrevMediaType != AUD_OUT_MEDIA_NONE&&eAOutMediaType!= AUD_OUT_MEDIA_NONE)
    {        
        LOG(LOG_CTRLF, TEXT("Set front media error, pre %d, set %d.\n"),
           ePrevMediaType, eAOutMediaType);
        up(&g_aud_media_lock);
        return (FALSE);
    }   

    LOG(LOG_CTRLF, TEXT("Set audio front media type as %d.\n"), eAOutMediaType);
    vWriteDspShmBYTE(B_FRONT_AOUT_MEDIA_TYPE, eAOutMediaType);

    #if CONFIG_DRV_SPDIF_RAW_SUPPORT
    if((AUD_OUT_MEDIA_DVD == eAOutMediaType) && (AUD_OUT_MEDIA_DVD == uReadDspShmBYTE(B_REAR_AOUT_MEDIA_TYPE)))
    {
        u32 u4flag = 0;
        LOG(LOG_DATAF, TEXT("[SPDIF]Front/Rear both are DVD type\n"));
        DspGetAsrcIecFlag(&u4flag);
        if(u4flag&&g_rSpdifRawInfo.fgDvdIsRawOut)
        {
            AudCfg_SpdifEnable(AUD_DVD_OUT);
        }
    }
    else if((AUD_OUT_MEDIA_USB == eAOutMediaType) && (AUD_OUT_MEDIA_USB == uReadDspShmBYTE(B_REAR_AOUT_MEDIA_TYPE)))
    {   
        LOG(LOG_DATAF, TEXT("[SPDIF]Front/Rear both are USB type\n"));
        AudCfg_SpdifEnable(AUD_AOUT1);
        if(g_rSpdifRawInfo.fgUsbIsRawOut)
        {        
            LOG(LOG_DATAF, TEXT("[SPDIF]AOUT1 Decoder reinit.\n"));
            vSendADSPCmd(DSP_DEC_REINIT);
            return (TRUE);
        }
    }
    #endif

    LOG(LOG_DATAF, TEXT("[AUD_FRONT]Reset Aout\n"));
    vSendADSPCmd(UOP_DSP_FRONT_AOUT_RESET);
    up(&g_aud_media_lock);

    return (TRUE);
}

bool fgAdspSetRearAoutMediaType(AUD_OUT_MEDIA_TYPE_T  eAOutMediaType)
{
    AUD_OUT_MEDIA_TYPE_T ePrevMediaType;

    down(&g_aud_media_lock);
    ePrevMediaType = uReadDspShmBYTE(B_REAR_AOUT_MEDIA_TYPE);

    if (eAOutMediaType  == ePrevMediaType)
    {
        //Current media type no change , no need action for driver
        LOG(LOG_CTRLF, TEXT("Rear media type is same, No reset need\n"));
        up(&g_aud_media_lock);
        return (TRUE);
    }

    if (eAOutMediaType == AUD_OUT_MEDIA_LINE_IN)
    {
         u8 u1Input = uReadShmUINT8(B_AUDIN_INPUT_TYPE);
         if(AUD_ADC_IN == u1Input)
         {
             eAOutMediaType  = AUD_OUT_MEDIA_UNDEF;
         }
    }
    
    LOG(LOG_CTRLF, TEXT("Set audio rear out media type is %d.\n"), eAOutMediaType);
    DspCfgSetRearMediaType((u8)eAOutMediaType);

    if ((AudDrvIsDecPlay(PRI_DEC)) ||
       (AudDrvIsDecPlay(SEC_DEC)) || 
       (AudDrvIsDecPlay(TER_DEC)) ||     
       (eAOutMediaType >= AUD_OUT_MEDIA_NONE)||
       (eAOutMediaType <= AUD_OUT_MEDIA_UNDEF))
    {
        LOG(LOG_DATAF, TEXT("[AUD_REAR]Reset Aout\n"));
        vSendADSPCmd(UOP_DSP_REAR_AOUT_RESET);
    }
    else
    {
        LOG(LOG_CTRLF, TEXT("[AUD_REAR]No Need rear Aout Reset before play\n"));
    }
    up(&g_aud_media_lock);

    return (TRUE);
}


void vAdspSetFeatureInfo(AUD_DEC_FEATURE_INFO_T eModFeatureInfo)
{    
    DspCfgSetFeatureInfo(&eModFeatureInfo);
}

void vAdspChannelVolumeGainCtrl(AUD_CH_T eChannel, u32 u4Value)
{
    u16 u2UopIndex = 0;

    VERIFY(eChannel <= AUD_CH_ALL);

    if (eChannel > AUD_CH_ALL)
    {
        eChannel = AUD_CH_FRONT_LEFT;
    }

    switch(eChannel)
    {
    case AUD_CH_FRONT_LEFT:
        u2UopIndex = UOP_DSP_TRIM_L;
        break;
    case AUD_CH_FRONT_RIGHT:
        u2UopIndex = UOP_DSP_TRIM_R;
        break;
    case AUD_CH_REAR_LEFT:
        u2UopIndex = UOP_DSP_TRIM_LS;
        break;
    case AUD_CH_REAR_RIGHT:
        u2UopIndex = UOP_DSP_TRIM_RS;
        break;
    case AUD_CH_CENTER:
        u2UopIndex = UOP_DSP_TRIM_C;
        break;
    case AUD_CH_SUB_WOOFER:
        u2UopIndex = UOP_DSP_TRIM_SUBWOOFER;
        break;
    case AUD_CH_BACK_LEFT:
        u2UopIndex = UOP_DSP_TRIM_CH7;
        break;
    case AUD_CH_BACK_RIGHT:
        u2UopIndex = UOP_DSP_TRIM_CH8;
        break;
    case AUD_CH_ALL: /* MASTER_VOLUME */
        u2UopIndex = UOP_DSP_MASTER_VOLUME;
        break;
    default:
        break;
   }

    DspCfgSetVolumeGain(eChannel, u4Value);
    vSendADSPCmd(u2UopIndex);
}

void vAdspMasterVolumeGain(u32 u4Volume)
{
    vAdspChannelVolumeGainCtrl(AUD_CH_ALL, u4Volume);
}

void vAdspChannelVolumeCtrl(AUD_CH_T eChannel, u8 u1Value)
{
    u16 u2ShmIndex = 0;
    u16 u2UopIndex = 0;
    u32 u4VolumeShm = 0; /* 0 ~ 0x20000 */

    VERIFY(eChannel <= AUD_CH_ALL);

    if (u1Value > MAX_VOL_LEVEL)
    {
        u1Value = MAX_VOL_LEVEL;
    }

    if (eChannel > AUD_CH_ALL)
    {
        eChannel = AUD_CH_FRONT_LEFT;
    }

    u4VolumeShm = (eChannel == AUD_CH_ALL) ? AdspVolToShm(TRUE, u1Value) : AdspVolToShm(FALSE, u1Value);

        switch(eChannel)
        {
        case AUD_CH_FRONT_LEFT:
            u2ShmIndex = D_TRIM_L;
            u2UopIndex = UOP_DSP_TRIM_L;
            break;
        case AUD_CH_FRONT_RIGHT:
            u2ShmIndex = D_TRIM_R;
            u2UopIndex = UOP_DSP_TRIM_R;
            break;
        case AUD_CH_REAR_LEFT:
            u2ShmIndex = D_TRIM_LS;
            u2UopIndex = UOP_DSP_TRIM_LS;
            break;
        case AUD_CH_REAR_RIGHT:
            u2ShmIndex = D_TRIM_RS;
            u2UopIndex = UOP_DSP_TRIM_RS;
            break;
        case AUD_CH_CENTER:
            u2ShmIndex = D_TRIM_C;
            u2UopIndex = UOP_DSP_TRIM_C;
            break;
        case AUD_CH_SUB_WOOFER:
            u2ShmIndex = D_TRIM_SUB;
            u2UopIndex = UOP_DSP_TRIM_SUBWOOFER;
            break;
        case AUD_CH_BACK_LEFT:
            u2ShmIndex = D_TRIM_CH7;
            u2UopIndex = UOP_DSP_TRIM_CH7;
            break;
        case AUD_CH_BACK_RIGHT:
            u2ShmIndex = D_TRIM_CH8;
            u2UopIndex = UOP_DSP_TRIM_CH8;
            break;
        case AUD_CH_ALL: /* MASTER_VOLUME */
            u2ShmIndex = D_VOL;
            u2UopIndex = UOP_DSP_MASTER_VOLUME;
            break;
        default:
            break;
        }


    vWriteShmUINT32(u2ShmIndex, u4VolumeShm);
    vSendADSPCmd(u2UopIndex);
}

void vAdspMasterVolume(u8 u1Volume)
{
    vAdspChannelVolumeCtrl(AUD_CH_ALL, u1Volume);
}


void vAdspRearChVolGainCtrl(u32 u4VolGainValue)
{
    vWriteShmUINT32(D_VOL_REAR, u4VolGainValue);
    vSendADSPCmd(UOP_DSP_REAR_MASTER_VOLUME);    
}


/*Function: Set shareinfo B_MEDIA_TYPE_FLAG[uBit] to fgFlag*/
void vAdspSetMediaFlag(bool fgFlag,u8 uBit)
{
    u8 uFlag = uReadShmUINT8(B_MEDIA_TYPE_FLAG);

    LOG(4, TEXT("[AUD_dsp_cfg] vAdspSetMediaFlag %d ,bit %d .\n"),fgFlag,uBit);

    if(fgFlag)
    {
        vWriteShmUINT8(B_MEDIA_TYPE_FLAG, uFlag | (0x1 << uBit));
    }
    else
    {
        vWriteShmUINT8(B_MEDIA_TYPE_FLAG, uFlag & (~(0x1 << uBit)));
    }
}


/***************************************************************************
Function : fgAdspIECConfig
Description : Routine handling IEC Configuration
Parameter : None
Return    : None
***************************************************************************/
bool fgAdspIECConfig(u8 ucDecId, AUD_IEC_CFG_T eIecCfg, bool fgEnable)
{
    u8 u1IecFlag = (u8)eIecCfg;
    u8 u1Mute = 0;

    VERIFY(ucDecId == PRI_DEC);

    if (FALSE == fgEnable)
    {
        u1Mute = 1;
    }

    if ((g_eIecMode[0] != eIecCfg) || (g_fgIecEnable[0] != fgEnable))
    {
        // If PCM -> RAW, disable output first, then switch to RAW to avoid noise
        // for bad design decoder, bad code FIXME !!
        DspCfgSetIecFlags(u1IecFlag);
        DspCfgSetIecMute(u1Mute);
        vSendADSPCmd(UOP_DSP_IEC_FLAG);
    }

    g_eIecMode[0] = eIecCfg;
    g_fgIecEnable[0] = fgEnable;

    return TRUE;
}

void AudSetFuncOption(AUD_FUNC_OPTION_T *pvSetting)
{
    LOG(LOG_FEATURE, TEXT("[AUD] AudSetFuncOption\n"));
    LOG(LOG_FEATURE, TEXT("[AUD] u4FuncOption0 = 0x%x\n"), pvSetting->u4FuncOption0);
    LOG(LOG_FEATURE, TEXT("[AUD] u4FuncOption1 = 0x%x\n"), pvSetting->u4FuncOption1);
    LOG(LOG_FEATURE, TEXT("[AUD] u4FuncOption2 = 0x%x\n"), pvSetting->u4FuncOption2);    
    LOG(LOG_FEATURE, TEXT("[AUD] u4GainAvIn = 0x%x\n"), pvSetting->u4GainAvIn);
    LOG(LOG_FEATURE, TEXT("[AUD] u4GainUSB = 0x%x\n"), pvSetting->u4GainUSB);
    LOG(LOG_FEATURE, TEXT("[AUD] u4GainDVD = 0x%x\n"), pvSetting->u4GainDVD);
    LOG(LOG_FEATURE, TEXT("[AUD] u4BassCutOffFreq = 0x%x\n"), pvSetting->u4BassCutOffFreq);
    DspCfgSetFuncOption(pvSetting);
    
    return;
}

void AudSetDecContext(AUD_OUT_MEDIA_TYPE_T eType, AUD_DRV_CONTEXT *prContext,
	                  AUD_CFG_ID eAOut)
{
    LOG(LOG_CTRLF, TEXT("[AUD]Decoder context, type is %d.\r\n"), eType);
    prContext->ePlayType = eType;
    switch(eType)
    {
    case AUD_OUT_MEDIA_USB:
        prContext->u1DecId = PRI_DEC;
        prContext->fgEnPlay = TRUE;
        break;
    case AUD_OUT_MEDIA_LINE_IN:
        prContext->u1DecId = SEC_DEC;
        prContext->fgEnPlay = TRUE;
        break;
    case AUD_OUT_MEDIA_LINE_IN2:
        prContext->u1DecId = TER_DEC;
        prContext->fgEnPlay = TRUE;
        break;
    case AUD_OUT_MEDIA_NONE:
        prContext->u1DecId = MAX_AUDDRV_NUM;
        prContext->fgEnPlay = FALSE;
        break;
    default:
        return;
    }

    if(prContext->fgEnPlay == TRUE)
    {
        prContext->u1Output = (prContext->u1Output == AUD_OUT_MAX)? eAOut:AUD_FRONT_REAR;
    }
    else
    {
        prContext->u1Output = AUD_OUT_MAX;
    }
}

