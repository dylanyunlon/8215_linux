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
#include "aud_oal.h"
#include "aud_debug.h"
#include "aud_dsp_cfg.h"
#include "DspDrvInc.h"
#include "DspFunc.h"
#include "aud_drv_config.h"
#include "aud_drv.h"
#include "aud_config.h"
#include "drv_dsp_cfg.h"
#include <asm/cacheflush.h>

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
extern AUD_DRV_CONTEXT g_rAudResouceManger[MAX_AUDDRV_NUM];

u32 u4ChannelAmpl = 1;
u32 u4SampleAmpl = 1;

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
 * Function     : vSendADSPCmd
 * Description  : send cmd to adsp
 * Parameter    : u4Cmd: cmd id
 * Return       :
**************************************************************************/
void vSendADSPCmd(u32 u4Cmd)
{
    vDspCmd(u4Cmd);
}

/*************************************************************************
 * Function     : AdspVolToShm
 * Description  : transform volume (0~100) to volume gain (0~0x20000)
 * Parameter    : u1Volume: volume
 * Return       :
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

    
	LOG(LOG_CTRLF, TEXT("SpeakerLayoutH(0x%x)L(0x%x), SizeF(%d)C(%d)R(%d)S(%d), ForceSub(%d).\n"),
		(u32)(((rSpeakerLayout.ui8_spk_layout)&0xFFFFFFFF00000000LL)>>32),(u32)((rSpeakerLayout.ui8_spk_layout)&0xFFFFFFFF), \
		rSpeakerLayout.ui2_front_size, rSpeakerLayout.ui2_center_size, rSpeakerLayout.ui2_rear_size, \
		rSpeakerLayout.ui2_sub_size, rSpeakerLayout.ui4_sub_force_out);

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

    #if CONFIG_AUD_DECONLY_EN
    if(DspGetDeconlyCtrl() == AUD_DECONLY_ON && eAOutMediaType == AUD_OUT_MEDIA_USB)
    {
        LOG(LOG_CTRLF, TEXT("Primary decoder is used by deconly, please stop it first.\n"));
        up(&g_aud_media_lock);
        return (FALSE);
    }
    #endif

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

    #if CONFIG_AUD_DECONLY_EN
    if(DspGetDeconlyCtrl() == AUD_DECONLY_ON && eAOutMediaType == AUD_OUT_MEDIA_USB)
    {
        LOG(LOG_CTRLF, TEXT("Primary decoder is used by deconly, please stop it first.\n"));
        up(&g_aud_media_lock);
        return (FALSE);
    }
    #endif

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

bool fgAdspSetMediaType(AUD_MEDIA_TYPE ptAudMediaType)
{
    u16 ePrevMediaType;
    u32 u4Tmp;

    down(&g_aud_media_lock);

    LOG(LOG_CTRLF, TEXT("Media type:Source(%d),Out(%d),Enable(%d).\n"), ptAudMediaType.eMediaSrc, ptAudMediaType.eMediaOut, ptAudMediaType.eMediaCtrl);

    if((ptAudMediaType.eMediaSrc >= AUD_MEDIA_SOURCE_UNDEF) ||
      (ptAudMediaType.eMediaOut >= AUD_MEDIA_OUT_UNDEF) ||
      (ptAudMediaType.eMediaCtrl >= AUD_MEDIA_ONOFF_UNDEF))
    {
        LOG(LOG_CTRLF, TEXT("Media type para error.\n"));
        up(&g_aud_media_lock);
        return FALSE;
    }

    // W_MEDIA_TYPE
    // b0:usb front on/off               b1:usb rear on/off
    // b2:linein front on/off             b3:linein rear on/off
    // b4:linein2 front on/off            b5:linein2 rear on/off
    // b6:swmix front on/off            b7:swmix rear on/off
    // b8:swmix2 front on/off           b9:swmix2 rear on/off
    // b10:dvp front on/off              b11:dvp rear on/off
    ePrevMediaType = u2ReadDspShmWORD(W_MEDIA_TYPE);
    LOG(LOG_CTRLF, TEXT("Prev Media type:(0x%x).\n"), ePrevMediaType);
    u4Tmp = (ptAudMediaType.eMediaSrc * 2 + ptAudMediaType.eMediaOut);
    if(!(((ePrevMediaType >> u4Tmp) & 0x1) ^ ptAudMediaType.eMediaCtrl))
    {
        LOG(LOG_CTRLF, TEXT("Media type is same, No need reset.\n"));
        up(&g_aud_media_lock);
        return (FALSE);
    }

    #if CONFIG_AUD_DECONLY_EN
    if(DspGetDeconlyCtrl() == AUD_DECONLY_ON && ptAudMediaType.eMediaSrc == AUD_MEDIA_SOURCE_USB)
    {
        LOG(LOG_CTRLF, TEXT("Primary decoder is used by deconly, please stop deconly first.\n"));
        up(&g_aud_media_lock);
        return (FALSE);
    }
    #endif

    if(ptAudMediaType.eMediaCtrl == AUD_MEDIA_OFF)
    {
        vWriteDspShmWORD(W_MEDIA_TYPE, (ePrevMediaType & (~(1<<u4Tmp))));
        vSendADSPCmd(UOP_DSP_MEDIA_FS_USB_OFF + ptAudMediaType.eMediaSrc * 0x200 + ptAudMediaType.eMediaOut * 0xC00);
        LOG(LOG_CTRLF, TEXT("Set Media type Off:Source(%d),Out(%d).\n"), ptAudMediaType.eMediaSrc, ptAudMediaType.eMediaOut);
    }
    else
    {
        vWriteDspShmWORD(W_MEDIA_TYPE, (ePrevMediaType | (1<<u4Tmp)));
        vSendADSPCmd(UOP_DSP_MEDIA_FS_USB_ON + ptAudMediaType.eMediaSrc * 0x200 + ptAudMediaType.eMediaOut * 0xC00);
        LOG(LOG_CTRLF, TEXT("Set Media type On:Source(%d),Out(%d).\n"), ptAudMediaType.eMediaSrc, ptAudMediaType.eMediaOut);
    }

    msleep(10); // for switch fast case cmd crash

    up(&g_aud_media_lock);
    return (TRUE);
}


bool fgAdspGetMediaTypeStatus(AUD_MEDIA_TYPE * ptAudMediaType)
{
    u16 ePrevMediaType;
    u32 u4Tmp;

    down(&g_aud_media_lock);

    if((ptAudMediaType->eMediaSrc >= AUD_MEDIA_SOURCE_UNDEF) ||
      (ptAudMediaType->eMediaOut >= AUD_MEDIA_OUT_UNDEF))
    {
        LOG(LOG_CTRLF, TEXT("Media type para error.\n"));
        up(&g_aud_media_lock);
        return FALSE;
    }

    // W_MEDIA_TYPE
    // b0:usb front on/off               b1:usb rear on/off
    // b2:linein front on/off             b3:linein rear on/off
    // b4:linein2 front on/off            b5:linein2 rear on/off
    // b6:swmix front on/off            b7:swmix rear on/off
    // b8:swmix2 front on/off           b9:swmix2 rear on/off
    // b10:dvp front on/off              b11:dvp rear on/off
    ePrevMediaType = u2ReadDspShmWORD(W_MEDIA_TYPE);
    u4Tmp = (ptAudMediaType->eMediaSrc * 2 + ptAudMediaType->eMediaOut);
	ptAudMediaType->eMediaCtrl = (ePrevMediaType >> u4Tmp) & 0x1;

    up(&g_aud_media_lock);
    return (TRUE);
}

#if CONFIG_AUD_DECONLY_EN

#define CONFIG_DECONLY_ALLOC_BUF  (54*1024)  // 6ch and 72 bank64
static bool fgDeconlyAllocBuf = FALSE;
static u8 *pu1DeconlyBufAddr = NULL;
static AUD_DECONLY_GET_BUF eDeconlyGBuf;
static bool fgDeconlyParaInit = FALSE;

bool fgAudDeconlySetOnOff(AUD_DECONLY_CTRL_T eDeconlyCtrl)
{
    AUD_OUT_MEDIA_TYPE_T ePrevMediaType;

    down(&g_aud_media_lock);
    if(eDeconlyCtrl >= AUD_DECONLY_UNDEF_CTRL)
    {
        LOG(LOG_CTRLF, TEXT("Deconly ctrl flag error (%d).\n"), eDeconlyCtrl);
        up(&g_aud_media_lock);
        return (FALSE);
    }

    if(DspGetDeconlyCtrl() == eDeconlyCtrl)
    {
        LOG(LOG_CTRLF, TEXT("Deconly ctrl flag is same (%d), No need reset.\n"), eDeconlyCtrl);
        up(&g_aud_media_lock);
        return (FALSE);
    }

    if(AUD_DECONLY_ON == eDeconlyCtrl)
    {
        if(AUD_OUT_MEDIA_USB == uReadDspShmBYTE(B_FRONT_AOUT_MEDIA_TYPE) || AUD_OUT_MEDIA_USB == uReadDspShmBYTE(B_REAR_AOUT_MEDIA_TYPE))
        {
            LOG(LOG_CTRLF, TEXT("Primary decoder is occupy, Deconly alloc source fail!\n"));
            up(&g_aud_media_lock);
            return (FALSE);
        }

        if(!fgDeconlyAllocBuf)
        {
            pu1DeconlyBufAddr = (u8 *)kzalloc(CONFIG_DECONLY_ALLOC_BUF, GFP_KERNEL);
            if(NULL == pu1DeconlyBufAddr)
            {
                LOG(LOG_FAIL, TEXT("DSP Deconly alloc buffer fail.\n"));
                return FALSE;
            }
            fgDeconlyAllocBuf = TRUE;
        }
    }

    fgDeconlyParaInit = FALSE;
    x_memset(&eDeconlyGBuf, 0, sizeof(AUD_DECONLY_GET_BUF));
    DspSetDeconlySemp(0);
    DspSetDeconlyReadBank(0);
    DspSetDeconlyAoutBank(0);
    DspSetDeconlyReady(0);

    DspSetDeconlyCtrl(eDeconlyCtrl);
    LOG(LOG_CTRLF, TEXT("Set audio deconly %s.\n"), (eDeconlyCtrl==AUD_DECONLY_ON) ? TEXT("on"):TEXT("off"));
    up(&g_aud_media_lock);
    return (TRUE);
}

static void vDeconlyParaInit(void)
{
    u32 u4Cfg0, u4Cfg1, u4Num;

    eDeconlyGBuf.eBitDepth = AUD_DECONLY_BIT16_DEPTH;
    eDeconlyGBuf.eDataEndian = AUD_DECONLY_LITTLE_ENDIAN;
    u32 u4InputSampleRate = 0;
    u32 u4InputChannelNum = 0;

    switch(DspGetDeconlyFS())
    {
    case SFREQ_8K:
        eDeconlyGBuf.u4SampleRate = 8000;
        break;
    case SFREQ_16K:
        eDeconlyGBuf.u4SampleRate = 16000;
        break;
    case SFREQ_32K:
        eDeconlyGBuf.u4SampleRate = 32000;
        break;
    case SFREQ_64K:
        eDeconlyGBuf.u4SampleRate = 64000;
        break;
    case SFREQ_11K:
        eDeconlyGBuf.u4SampleRate = 11025;
        break;
    case SFREQ_22K:
        eDeconlyGBuf.u4SampleRate = 22050;
        break;
    case SFREQ_44K:
        eDeconlyGBuf.u4SampleRate = 44100;
        break;
    case SFREQ_88K:
        eDeconlyGBuf.u4SampleRate = 88200;
        break;
    case SFREQ_176K:
        eDeconlyGBuf.u4SampleRate = 176400;
        break;
    case SFREQ_12K:
        eDeconlyGBuf.u4SampleRate = 12000;
        break;
    case SFREQ_24K:
        eDeconlyGBuf.u4SampleRate = 24000;
        break;
    case SFREQ_48K:
        eDeconlyGBuf.u4SampleRate = 48000;
        break;
    case SFREQ_96K:
        eDeconlyGBuf.u4SampleRate = 96000;
        break;
    case SFREQ_192K:
        eDeconlyGBuf.u4SampleRate = 192000;
        break;
    default:
        eDeconlyGBuf.u4SampleRate = 48000;
        break;
    }

    u4Cfg0 = DspGetDeconlyChCfg0();
    u4Cfg1 = DspGetDeconlyChCfg1();
    u4Num = DspGetDeconlyChNum();
    if((~u4Cfg1) & 0xf00)
    {
        u4Num += 1;
    }
    eDeconlyGBuf.eChCfg.u1LayoutL = ((~u4Cfg0)&0xf) ? (u4Cfg0&0xf) : 0;
    eDeconlyGBuf.eChCfg.u1LayoutR = ((~u4Cfg0)&0xf0) ? ((u4Cfg0>>4)&0xf) : 0;
    eDeconlyGBuf.eChCfg.u1LayoutC = ((~u4Cfg0)&0xf00) ? ((u4Cfg0>>8)&0xf) : 0;
    eDeconlyGBuf.eChCfg.u1LayoutLs = ((~u4Cfg1)&0xf) ? (u4Cfg1&0xf) : 0;
    eDeconlyGBuf.eChCfg.u1LayoutRs = ((~u4Cfg1)&0xf0) ? ((u4Cfg1>>4)&0xf) : 0;
    eDeconlyGBuf.eChCfg.u1LayoutSub = ((~u4Cfg1)&0xf00) ? u4Num : 0;
    eDeconlyGBuf.eChCfg.u2ChNum = (u16)u4Num;
    u4InputSampleRate = DspCfgInputSampRateDecimal(PRI_DEC);

    DspGetInputChCfg(&u4InputChannelNum);
    u4SampleAmpl = eDeconlyGBuf.u4SampleRate / (u4InputSampleRate == 0 ? eDeconlyGBuf.u4SampleRate : u4InputSampleRate);
    if (u4SampleAmpl == 0)
    {
        u4SampleAmpl = 1;
    }
    eDeconlyGBuf.u4SampleRate = eDeconlyGBuf.u4SampleRate / u4SampleAmpl;
    if (eDeconlyGBuf.eChCfg.u2ChNum == 2 && u4InputChannelNum == 1)
    {
        u4ChannelAmpl = eDeconlyGBuf.eChCfg.u2ChNum / u4InputChannelNum;
        eDeconlyGBuf.eChCfg.u2ChNum = u4InputChannelNum;
    } else {
        u4ChannelAmpl = 1;
    }
    LOG(LOG_CTRLF, TEXT("DeconlyParaInit: FS(%d).\n"), eDeconlyGBuf.u4SampleRate);
    LOG(LOG_CTRLF, TEXT("DeconlyParaInit: CH(%d).\n"), eDeconlyGBuf.eChCfg.u2ChNum);
    LOG(LOG_CTRLF, TEXT("DeconlyParaInit: Depth(%d).\n"), eDeconlyGBuf.eBitDepth);
    LOG(LOG_CTRLF, TEXT("DeconlyParaInit: Endian(%d).\n"), eDeconlyGBuf.eDataEndian);
    LOG(LOG_CTRLF, TEXT("DeconlyParaInit: u4SampleAmpl(%d), u4ChannelAmpl(%d).\n"), u4SampleAmpl, u4ChannelAmpl);
}

#if 1
#define DECONLY_GET_MAX_BANK  (16)
u32 u4TotalSizeByBytes = 0;
u32 u4TotalSizeCnt = 0;
u32 u4TotalSendBank = 0;
u8 *pu1AllocBuf = NULL; //pu1DeconlyBufAddr;
extern bool g_fgAudioGotEos;
bool fgAudDeconlyGetBuff(AUD_DECONLY_GET_BUF *pGetBuf)
{
    u32 i,j,m,n;
    u32 u4AputBank,u4AoutBank,u4BankNum,u4ReadBank,u4BankMax;
    uintptr_t u4BaseAddr,u4BankAddr;
    u32 u4ChannelSize;
    uintptr_t u4UserAddr = pGetBuf->u4BufAddr;
    u32 u4InputSampleRate = 0;
    u32 u4InputChannelNum = 0;

    if(DspGetDeconlyReady())
    {
        if(!fgDeconlyParaInit)
        {
            vDeconlyParaInit();
            fgDeconlyParaInit = TRUE;
            u4TotalSizeByBytes = 0;
            u4TotalSizeCnt = 0;
            g_fgAudioGotEos = FALSE;
            u4TotalSendBank = 0;
            pu1AllocBuf = pu1DeconlyBufAddr;
        }

        eDeconlyGBuf.eErrType = AUD_DECONLY_NORMAL;
        while(1)
        {
            //copy & constract data
            u4AoutBank = DspGetDeconlyAoutBank();
            u4AputBank = DspGetDeconlyAputBank();

            if(u4AoutBank == u4AputBank)
            {
                if(DspGetDeconlyChkFrmSz())
                {
					u4AoutBank = DspGetDeconlyAoutBank();
					u4AputBank = DspGetDeconlyAputBank();
					if(u4AoutBank == u4AputBank)
					{
					    if(!g_fgAudioGotEos)
					    {
							LOG(LOG_FAIL, TEXT("AUD_DECONLY_AFIFO_NOT_ENOUGH(%d)\n"), u4TotalSendBank);
							eDeconlyGBuf.eErrType = AUD_DECONLY_AFIFO_NOT_ENOUGH;
							eDeconlyGBuf.u4BufLen = 0;
							x_memcpy(pGetBuf, &eDeconlyGBuf, sizeof(AUD_DECONLY_GET_BUF));
							return FALSE;					    
					    }
					    else
					    {					    
					        if(u4TotalSendBank)
					        {
					            break;
					        }
							LOG(LOG_CTRLF, TEXT("AUD_DECONLY_DEC_FINISH without data\n"));
							eDeconlyGBuf.eErrType = AUD_DECONLY_DEC_FINISH;			        
							eDeconlyGBuf.u4BufLen = 0;
							x_memcpy(pGetBuf, &eDeconlyGBuf, sizeof(AUD_DECONLY_GET_BUF));
							return FALSE;
					    }
					}
                }
                else
                {
                    //msleep(1);
                    continue;
                }
            }
            else
            {            
	            if(u4AputBank > u4AoutBank)
	                u4BankNum = u4AputBank - u4AoutBank;
	            else
	                u4BankNum = u4AputBank + 0xffffff - u4AoutBank;

	            if(u4TotalSendBank + u4BankNum > DECONLY_GET_MAX_BANK)
	            {               
	                u4BankNum = DECONLY_GET_MAX_BANK - u4TotalSendBank;
	            }
	            u4TotalSendBank += u4BankNum;

	            u4BaseAddr = DspGetDeconlyBaseAddr();
	            u4ChannelSize = DspGetDeconlyChSize();
	            u4ReadBank = DspGetDeconlyReadBank();
	            u4BankMax = DspGetDeconlyBankMax();
	            
                __flush_dcache_area(u4BaseAddr, u4ChannelSize*eDeconlyGBuf.eChCfg.u2ChNum);
	            for(i=0;i<u4BankNum;i++)
	            {
	                u8 *addr_ch[6];
	                u4BankAddr = u4BaseAddr + u4ReadBank * 48 * 4;
	                for(j=0;j<6;j++)
	                {
	                    addr_ch[j] = (u8 *)(u4BankAddr+u4ChannelSize*j);      //virtual addr
	                }

	                for(j=0;j<64;j++)
	                {
                        if (j % u4SampleAmpl != 0)
                        {
                            for(m = 0; m < eDeconlyGBuf.eChCfg.u2ChNum * u4ChannelAmpl; m++)
                            {
                                addr_ch[m] += 1;
                                addr_ch[m]++;
                                addr_ch[m]++;
                            }
                            
                        } else{
                            for(m = 0; m < eDeconlyGBuf.eChCfg.u2ChNum * u4ChannelAmpl; m++)
	                    {
	                        // 16b little endian
                                if (m % u4ChannelAmpl != 0)
                                {
                                    addr_ch[m] += 1;
                                    addr_ch[m]++;
                                    addr_ch[m]++;
                                } else{
	                        addr_ch[m] += 1;
	                        *pu1AllocBuf++ = *addr_ch[m]++;
	                        *pu1AllocBuf++ = *addr_ch[m]++;
	                    }
	                }
                         }
                    }

	                u4ReadBank += 1;
	                if(u4ReadBank >= u4BankMax)
	                {
	                    u4ReadBank = 0;
	                }
	            }
	            while(DspGetDeconlySemp());
	            DspSetDeconlyReadBank(u4ReadBank);
	            DspSetDeconlyAoutBank((u4AoutBank + u4BankNum) & 0xffffff);
	            DspSetDeconlySemp(1);

	            if(u4TotalSendBank >= DECONLY_GET_MAX_BANK)
	                break;
	        }
        }

        eDeconlyGBuf.u8BufPts = DspGetDeconlyPts();
        eDeconlyGBuf.u4BufAddr = u4UserAddr;

        eDeconlyGBuf.u4BufLen = eDeconlyGBuf.eChCfg.u2ChNum * u4TotalSendBank * 32 * 4 / (u4SampleAmpl ==0 ? 1 : u4SampleAmpl);     // 16b
        if(copy_to_user((u8 *)u4UserAddr, pu1DeconlyBufAddr, eDeconlyGBuf.u4BufLen))
        {
            LOG(LOG_FAIL, TEXT("Dsp Deconly copy to user fail.\n"));
            return FALSE;
        }
        if(g_fgAudioGotEos)
        {
			if(DspGetDeconlyChkFrmSz())
			{
				u4AoutBank = DspGetDeconlyAoutBank();
				u4AputBank = DspGetDeconlyAputBank();
				if(u4AoutBank == u4AputBank)
				{
					LOG(LOG_CTRLF, TEXT("AUD_DECONLY_DEC_FINISH with Last Packet...\n"));
					eDeconlyGBuf.eErrType = AUD_DECONLY_DEC_FINISH; 
				}
			}
        }
        
        x_memcpy(pGetBuf, &eDeconlyGBuf, sizeof(AUD_DECONLY_GET_BUF));

        #if 1 // dump data for debug
        {
            extern void vDeconlyDumpDataForDbg(uintptr_t u4Addr, u32 u4Size);
            vDeconlyDumpDataForDbg((uintptr_t)pu1DeconlyBufAddr,eDeconlyGBuf.u4BufLen);
        }
        #endif

        u4TotalSizeByBytes += eDeconlyGBuf.u4BufLen;
        u4TotalSizeCnt++;
        u4TotalSendBank = 0;
        pu1AllocBuf = pu1DeconlyBufAddr;
    }
    else
    {
        LOG(LOG_FAIL, TEXT("AUD_DECONLY_DEC_NOT_READY\n"));
        eDeconlyGBuf.eErrType = AUD_DECONLY_DEC_NOT_READY;
        eDeconlyGBuf.u4BufLen = 0;
        x_memcpy(pGetBuf, &eDeconlyGBuf, sizeof(AUD_DECONLY_GET_BUF));
        return FALSE;
    }

    return TRUE;
}

#else
bool fgAudDeconlyGetBuff(AUD_DECONLY_GET_BUF *pGetBuf)
{
    u32 i,j,m,n;
    u32 u4AputBank,u4AoutBank,u4BankNum,u4ReadBank,u4BankMax;
    uintptr_t u4BaseAddr,u4BankAddr;
    u32 u4ChannelSize;
    u8 *pu1AllocBuf = pu1DeconlyBufAddr;
    uintptr_t u4UserAddr = pGetBuf->u4BufAddr;
    u32 u4TotalSendBank = 0;

    if(DspGetDeconlyReady())
    {
        if(!fgDeconlyParaInit)
        {
            vDeconlyParaInit();
            fgDeconlyParaInit = TRUE;
        }

        eDeconlyGBuf.eErrType = AUD_DECONLY_NORMAL;

        while(1)
        {
        //copy & constract data
        u4AoutBank = DspGetDeconlyAoutBank();
        u4AputBank = DspGetDeconlyAputBank();
            
            if((u4AoutBank == u4AputBank) && DspGetDeconlyChkFrmSz() && (u4TotalSendBank == 0))
            {
               // msleep(1); // avoid afifo wpt is late to update.
                u4AoutBank = DspGetDeconlyAoutBank();
                u4AputBank = DspGetDeconlyAputBank();
                if(u4AoutBank == u4AputBank)
                {
                    LOG(LOG_FAIL, TEXT("AUD_DECONLY_AFIFO_NOT_ENOUGH\n"));
                    eDeconlyGBuf.eErrType = AUD_DECONLY_AFIFO_NOT_ENOUGH;
                    eDeconlyGBuf.u4BufLen = 0;
                    x_memcpy(pGetBuf, &eDeconlyGBuf, sizeof(AUD_DECONLY_GET_BUF));
                    return FALSE;
                }
            }

            if(u4AoutBank == u4AputBank)
            {
                if(DspGetDeconlyChkFrmSz())
                {
                    break;
                }
                else
                {
                    //msleep(1);
                    continue;
                }
            }
            
        if(u4AputBank > u4AoutBank)
            u4BankNum = u4AputBank - u4AoutBank;
        else
            u4BankNum = u4AputBank + 0xffffff - u4AoutBank;

        if(u4BankNum > 36)
        {
            LOG(LOG_CTRLF, TEXT("AB buffer index error:AputBank(0x%x),AoutBank(0x%x).\n"),u4AputBank,u4AoutBank);
        }

            if(u4TotalSendBank + u4BankNum > 72)
            {               
                LOG(LOG_FAIL, TEXT("AUD_DECONLY_AFIFO_TOO_MUCH\n"));
                eDeconlyGBuf.eErrType = AUD_DECONLY_AFIFO_TOO_MUCH;
                break;
            }
            u4TotalSendBank += u4BankNum;

        u4BaseAddr = DspGetDeconlyBaseAddr();
        u4ChannelSize = DspGetDeconlyChSize();
        u4ReadBank = DspGetDeconlyReadBank();
        u4BankMax = DspGetDeconlyBankMax();

        __flush_dcache_area(u4BaseAddr, u4ChannelSize*eDeconlyGBuf.eChCfg.u2ChNum);
        for(i=0;i<u4BankNum;i++)
        {
            u8 *addr_ch[6];
            u4BankAddr = u4BaseAddr + u4ReadBank * 48 * 4;
            for(j=0;j<6;j++)
            {
                addr_ch[j] = (u8 *)(u4BankAddr+u4ChannelSize*j);      //virtual addr
            }

            for(j=0;j<64;j++)
            {
                for(m=0;m<eDeconlyGBuf.eChCfg.u2ChNum;m++)
                {
                    // 16b little endian
                    addr_ch[m] += 1;
                    *pu1AllocBuf++ = *addr_ch[m]++;
                    *pu1AllocBuf++ = *addr_ch[m]++;
                }
            }

            u4ReadBank += 1;
            if(u4ReadBank >= u4BankMax)
            {
                u4ReadBank = 0;
            }
        }
            while(DspGetDeconlySemp());
            DspSetDeconlyReadBank(u4ReadBank);
            DspSetDeconlyAoutBank(u4AputBank);
            DspSetDeconlySemp(1);
        }

        eDeconlyGBuf.u8BufPts = DspGetDeconlyPts();
        eDeconlyGBuf.u4BufAddr = u4UserAddr;
        eDeconlyGBuf.u4BufLen = eDeconlyGBuf.eChCfg.u2ChNum * u4TotalSendBank * 32 * 4;     // 16b
        if(copy_to_user((u8 *)u4UserAddr, pu1DeconlyBufAddr, eDeconlyGBuf.u4BufLen))
        {
            LOG(LOG_FAIL, TEXT("Dsp Deconly copy to user fail.\n"));
            return FALSE;
        }
        
        x_memcpy(pGetBuf, &eDeconlyGBuf, sizeof(AUD_DECONLY_GET_BUF));

        #if 1 // dump data for debug
        {
            extern void vDeconlyDumpDataForDbg(uintptr_t u4Addr, u32 u4Size);
            vDeconlyDumpDataForDbg((uintptr_t)pu1DeconlyBufAddr,eDeconlyGBuf.u4BufLen);
        }
        #endif

        LOG(LOG_FEATURE, TEXT("u4BufAddr(0x%X), u4BufLen(0x%X), Bank(0x%X) \n"), eDeconlyGBuf.u4BufAddr, eDeconlyGBuf.u4BufLen, u4TotalSendBank);
        LOG(LOG_FEATURE, TEXT("Deconly PtsH = 0x%X, PtsL = 0x%X \n"), (u32)((eDeconlyGBuf.u8BufPts&0xFFFFFFFF00000000LL)>>32),(u32)(eDeconlyGBuf.u8BufPts&0xFFFFFFFF));
    }
    else
    {
        LOG(LOG_FAIL, TEXT("DSP Deconly not ready now.\n"));
        eDeconlyGBuf.eErrType = AUD_DECONLY_DEC_NOT_READY;
        eDeconlyGBuf.u4BufLen = 0;
        x_memcpy(pGetBuf, &eDeconlyGBuf, sizeof(AUD_DECONLY_GET_BUF));
        return FALSE;
    }

    return TRUE;
}
#endif
#endif

bool AudGetSpectrumData(u8* pAddr, u32 u4Size, u32 u4scaleMode)
{
    u8* pSrcData  = NULL;
    u8* pOutAddr  = (u8 *)DspGetSpectrumBaseAddr();
    u32 u4DataIdx = 0;
    s32 s4TmpData = 0;
    s32 s4Ret     = true;
    AUD_DRV_STATE_T eDrvState = AudDrvGetState(PRI_DEC);
    if(NULL == pAddr || 0 == u4Size)
    {
        LOG(LOG_FAIL, TEXT("AudGetSpectrumData param error, u4Size %d.\n"), u4Size);
        return false;
    }

    if(NULL == (pSrcData = kzalloc(u4Size, GFP_KERNEL)))
    {
        LOG(LOG_FAIL, TEXT("AudGetSpectrumData alloc memmory fail.\n"));
        return false;
    }

    for(u4DataIdx = 0; u4DataIdx < u4Size; u4DataIdx++)
    {
        pOutAddr += 2; //remove low 16bit
        s4TmpData = ((eDrvState != AUD_DRV_PLAYED && TR_DSP_A_S_DISCONNECTED == u1DspAGetState()) || eDrvState == AUD_DRV_PAUSED) ? 0 : *pOutAddr++ ;  //Get high 8 bit
        pSrcData[u4DataIdx] = ((uint8_t)s4TmpData)^0x80;
    }

    if(copy_to_user(pAddr, pSrcData, u4Size))
    {
        LOG(LOG_FAIL, TEXT("Get Spectrum Source copy to user fail.\n"));
        s4Ret = false;
    }
    kfree(pSrcData);
    return s4Ret;
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

void vAdspSrcVolumeCtrl(AUD_MEDIA_SOURCE_TYPE_T eSrc, AUD_MEDIA_OUT_TYPE_T eType, u32 u4Value)
{
    u16 u2UopIndex = 0;
    LOG(LOG_CTRLF, TEXT("vAdspSrcVolumeCtrl eSrc %d eType %d volume %d.\n"), eSrc, eType, u4Value);
    switch(eType)
    {
    case AUD_MEDIA_OUT_FRONT:
        switch(eSrc)
        {
        case AUD_MEDIA_SOURCE_USB:
            u2UopIndex = UOP_DSP_SRC_VOL_FS_DEC1;
            break;
        case AUD_MEDIA_SOURCE_LINEIN:
            u2UopIndex = UOP_DSP_SRC_VOL_FS_DEC4;
            break;
        case AUD_MEDIA_SOURCE_LINEIN2:
            u2UopIndex = UOP_DSP_SRC_VOL_FS_DEC5;
            break;
        case AUD_MEDIA_SOURCE_SWMIX:
            u2UopIndex = UOP_DSP_SRC_VOL_FS_MIXER;
            break;
        case AUD_MEDIA_SOURCE_SWMIX2:
            u2UopIndex = UOP_DSP_SRC_VOL_FS_MIXER2;
            break;
        case AUD_MEDIA_SOURCE_DVP:
            u2UopIndex = UOP_DSP_SRC_VOL_FS_DVP;
            break;
        default:
            break;
        }
        break;
    case AUD_MEDIA_OUT_REAR:
        switch(eSrc)
        {
        case AUD_MEDIA_SOURCE_USB:
            u2UopIndex = UOP_DSP_SRC_VOL_RS_DEC1;
            break;
        case AUD_MEDIA_SOURCE_LINEIN:
            u2UopIndex = UOP_DSP_SRC_VOL_RS_DEC4;
            break;
        case AUD_MEDIA_SOURCE_LINEIN2:
            u2UopIndex = UOP_DSP_SRC_VOL_RS_DEC5;
            break;
        case AUD_MEDIA_SOURCE_SWMIX:
            u2UopIndex = UOP_DSP_SRC_VOL_RS_MIXER;
            break;
        case AUD_MEDIA_SOURCE_SWMIX2:
            u2UopIndex = UOP_DSP_SRC_VOL_RS_MIXER2;
            break;
        case AUD_MEDIA_SOURCE_DVP:
            u2UopIndex = UOP_DSP_SRC_VOL_RS_DVP;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
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

void AudSetDecContext(AUD_OUT_MEDIA_TYPE_T eType, AUD_DRV_CONTEXT *prContext, AUD_CFG_ID eAOut)
{
    LOG(LOG_CTRLF, TEXT("[AUD]Decoder context, type is %d.\r\n"), eType);
    //1. get decoder id based on media type
    if(eType ==AUD_OUT_MEDIA_USB)
    {
        prContext->u1DecId = PRI_DEC;
    }
    else if(eType ==AUD_OUT_MEDIA_LINE_IN)
    {
        prContext->u1DecId = SEC_DEC;
    }
    else if(eType ==AUD_OUT_MEDIA_LINE_IN2)
    {
        prContext->u1DecId = SEC_DEC;
    }

    LOG(LOG_CTRLF, TEXT("current out is %d,  set out %d.\r\n"), prContext->u1Output, eAOut);
    //2. out to front/rear/front&rear
    switch(prContext->u1Output)
    {
    case AUD_OUT_MAX:
        if(AUD_OUT_MEDIA_NONE != eType)
        {
            prContext->u1Output = eAOut;
            prContext->ePlayType = eType;
            prContext->fgEnPlay = TRUE;
        }
        break;
    case AUD_FRONT:
    case AUD_REAR:
        if(AUD_OUT_MEDIA_NONE == eType)  //close front or rear
        {
            if(prContext->u1Output == eAOut)
            {
                prContext->u1Output = AUD_OUT_MAX;
                prContext->ePlayType = AUD_OUT_MEDIA_NONE;
                prContext->fgEnPlay = FALSE;
                prContext->u1DecId = MAX_AUDDRV_NUM;
            }
            else
            {
                LOG(LOG_FAIL, TEXT("set to none current u1Output is %d, unsupport out %d.\n"), prContext->u1Output, eAOut);
            }
        }
        else if((prContext->u1Output==AUD_FRONT && eAOut== AUD_REAR)||
                (prContext->u1Output==AUD_REAR && eAOut== AUD_FRONT))
        {
            if(eType == prContext->ePlayType)
            {
                prContext->u1Output = AUD_FRONT_REAR;//front to front and rear
            }
            else
            {
                LOG(LOG_FAIL, TEXT("current Type is %d, unsupport  type %d.\n"), prContext->ePlayType, eType);
            }
        }
        else
        {
            LOG(LOG_FAIL, TEXT("current output%d, unsupport out type %d.\n"),prContext->u1Output, eAOut);
        }
        break;
    
    case AUD_FRONT_REAR:
        if(AUD_OUT_MEDIA_NONE == eType)
        {
            if(eAOut == AUD_FRONT)
            {
                prContext->u1Output = AUD_REAR;
            }
            else if(eAOut == AUD_REAR)
            {
                prContext->u1Output = AUD_FRONT;
            }
            else if(eAOut == AUD_FRONT_REAR)
            {
                prContext->u1Output = AUD_OUT_MAX;
                prContext->ePlayType = AUD_OUT_MEDIA_NONE;
                prContext->fgEnPlay = FALSE;
                prContext->u1DecId = MAX_AUDDRV_NUM;
            }
            else
            {
                LOG(LOG_FAIL, TEXT("front and rear out,switch out %d.\n"), eAOut);
            }
        }
        else
        {
            LOG(LOG_FAIL, TEXT("switch media is %d.\n"),eType);
        }
        break;
    default:
        LOG(LOG_FAIL, TEXT("unsupport current type %d.\n"),prContext->u1Output);
        break;
    }

    if(prContext->fgEnPlay == TRUE)  // JB2 not this setting, only 8317M used for other new ioctrl implementation.
    {
        x_memcpy((void *)&g_rAudResouceManger[prContext->u1DecId], prContext, sizeof(AUD_DRV_CONTEXT));
    }
}

void AudSetDecMediaContext(AUD_MEDIA_TYPE eType, AUD_DRV_CONTEXT *prContext)
{
    if(eType.eMediaSrc == AUD_MEDIA_SOURCE_USB)
    {
        prContext->u1DecId = PRI_DEC;
    }
    else if(eType.eMediaSrc == AUD_MEDIA_SOURCE_LINEIN)
    {
        prContext->u1DecId = SEC_DEC;
    }
    else if(eType.eMediaSrc == AUD_MEDIA_SOURCE_LINEIN2)
    {
        prContext->u1DecId = TER_DEC;
    }
    else if(eType.eMediaSrc == AUD_MEDIA_SOURCE_SWMIX)
    {
        prContext->u1DecId = SWMIX_DEC;
    }
    else if(eType.eMediaSrc == AUD_MEDIA_SOURCE_SWMIX2)
    {
        prContext->u1DecId = SWMIX2_DEC;
    }
    else if(eType.eMediaSrc == AUD_MEDIA_SOURCE_DVP)
    {
        prContext->u1DecId = DVP_DEC;
    }

    LOG(LOG_CTRLF, TEXT("AudSetDecMediaContext:curr out is %d,  set out %d.\r\n"), prContext->u1Output, eType.eMediaOut);
    
    //2. out to front/rear/front&rear
    switch(prContext->u1Output)
    {
    case AUD_OUT_MAX:
        if(AUD_MEDIA_ON == eType.eMediaCtrl)
        {
            prContext->fgEnPlay = TRUE;
            if(AUD_MEDIA_OUT_FRONT == eType.eMediaOut)
            {
                prContext->u1Output = AUD_FRONT;
            }
            else
            {
                prContext->u1Output = AUD_REAR;
            }
        }
        break;
    case AUD_FRONT:
    case AUD_REAR:
        if(AUD_MEDIA_OFF == eType.eMediaCtrl)  //close front or rear
        {
            if(((prContext->u1Output == AUD_FRONT) && (eType.eMediaOut == AUD_MEDIA_OUT_FRONT)) ||
              ((prContext->u1Output == AUD_REAR) && (eType.eMediaOut == AUD_MEDIA_OUT_REAR)))
            {
                prContext->u1Output = AUD_OUT_MAX;
                prContext->fgEnPlay = FALSE;
                prContext->u1DecId = MAX_AUDDRV_NUM;
            }
            else
            {
                LOG(LOG_FAIL, TEXT("set to none current u1Output is %d, unsupport out %d.\n"), prContext->u1Output, eType.eMediaOut);
            }
        }
        else if((prContext->u1Output==AUD_FRONT && eType.eMediaOut == AUD_MEDIA_OUT_REAR)||
                (prContext->u1Output==AUD_REAR && eType.eMediaOut == AUD_MEDIA_OUT_FRONT))
        {
            prContext->u1Output = AUD_FRONT_REAR;//front to front and rear
        }
        else
        {
            LOG(LOG_FAIL, TEXT("current output%d, unsupport out type %d.\n"),prContext->u1Output, eType.eMediaOut);
        }
        break;
    
    case AUD_FRONT_REAR:
        if(AUD_MEDIA_OFF == eType.eMediaCtrl)
        {
            if(eType.eMediaOut == AUD_MEDIA_OUT_FRONT)
            {
                prContext->u1Output = AUD_REAR;
            }
            else if(eType.eMediaOut == AUD_MEDIA_OUT_REAR)
            {
                prContext->u1Output = AUD_FRONT;
            }           
            else
            {
                LOG(LOG_FAIL, TEXT("unsupport:front and rear out,switch out %d.\n"), eType.eMediaOut);
            }
        }
        else
        {
            LOG(LOG_FAIL, TEXT("unsupport switch media is %d.\n"),eType.eMediaSrc);
        }
        break;
    default:
        LOG(LOG_FAIL, TEXT("unsupport current type %d.\n"),prContext->u1Output);
        break;
    }

    if(prContext->fgEnPlay == TRUE)  // JB2 not this setting, only 8317M used for other new ioctrl implementation.
    {
        x_memcpy((void *)&g_rAudResouceManger[prContext->u1DecId], prContext, sizeof(AUD_DRV_CONTEXT));
    }
}


bool fgAudGetDecStatus(AUD_DEC_ID_T eDecId, AUD_DRV_CONTEXT *prContext)
{
    bool fgDecBusy = false;

    down(&g_aud_media_lock);
    //prContext = g_rAudResouceManger[eDecId];

    if(NULL == prContext)
    {
        fgDecBusy = false;
    }
    else
    {
        x_memcpy(prContext, (const void *)&g_rAudResouceManger[eDecId], sizeof(AUD_DRV_CONTEXT));
        fgDecBusy = true;
    }

    up(&g_aud_media_lock);

    return fgDecBusy;
}

